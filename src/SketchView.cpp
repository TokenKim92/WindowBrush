#include "SketchView.h"
#include "Utility.h"
#include "ColorPalette.h"
#include <vector>
#include <memory>
#include <map>

extern ApplicationCore *gp_appCore;

SketchView::SketchView(const HWND ah_window, const HBITMAP &ah_screenBitmap, const RECT &a_physicalRect, const CM &a_mode) :
	Direct2DEx(ah_window, &a_physicalRect),
	mh_screenBitmap(ah_screenBitmap),
	m_physicalRect(a_physicalRect)
{
	mp_screenBitmap = nullptr;
	mp_dashStroke = nullptr;
	memset(&m_viewRect, 0, sizeof(DRect));

	if (CM::LIGHT == a_mode) {
		m_colorSet.highlight = RGB_TO_COLORF(ORANGE_300);
		m_colorSet.extentColor = RGB_TO_COLORF(NEUTRAL_100);
		m_colorSet.extentInvertColor = RGB_TO_COLORF(NEUTRAL_700);
	}
	else {
		m_colorSet.highlight = RGB_TO_COLORF(SKY_400);
		m_colorSet.extentColor = RGB_TO_COLORF(NEUTRAL_700);
		m_colorSet.extentInvertColor = RGB_TO_COLORF(NEUTRAL_100);
	}
}

SketchView::~SketchView()
{
	InterfaceRelease(&mp_screenBitmap);
	InterfaceRelease(&mp_dashStroke);

	for (auto &[index, p_gradient] : m_gradientTable) {
		InterfaceRelease(&p_gradient);
	}
}

int SketchView::Create()
{
	const auto InitGradientTable = [](SketchView *const ap_view)
	{
		const auto RandomizeColorOrder = [](std::vector<size_t> &a_colorOrder)
		{
			size_t count = a_colorOrder.size();
			size_t index;
			for (size_t i = 0; i < count; i++) {
				index = rand() % count;
				a_colorOrder[i] = index;

				for (size_t j = 0; j < i; j++) {
					if (a_colorOrder[j] == index) {
						a_colorOrder[i] = -1;
						i--;
						break;
					}
				}
			}
		};

		const size_t colorCount = 10;
		D2D1_COLOR_F colorList[colorCount] = {
			{0.964705f, 0.827450f, 0.396078f, 1.0f},
			{0.992156f, 0.627450f, 0.521568f, 1.0f},
			{0.984313f, 0.760784f, 0.921568f, 1.0f},
			{0.650980f, 0.756862f, 0.933333f, 1.0f},
			{0.517647f, 0.980392f, 0.690196f, 1.0f},
			{0.560784f, 0.827450f, 0.956862f, 1.0f},
			{0.631372f, 0.768627f, 0.992156f, 1.0f},
			{0.760784f, 0.913725f, 0.984313f, 1.0f},
			{1.0f, 0.925490f, 0.823529f, 1.0f},
			{0.988235f, 0.713725f, 0.623529f, 1.0f}
		};
		const float interval = 1.0f / static_cast<float>(colorCount);
		const D2D1_LINEAR_GRADIENT_BRUSH_PROPERTIES gradientData = {
				{0, 0},
				{
					static_cast<float>(ap_view->m_physicalRect.right - ap_view->m_physicalRect.left),
					static_cast<float>(ap_view->m_physicalRect.bottom - ap_view->m_physicalRect.top),
				}
		};

		D2D1_GRADIENT_STOP p_gradientStopList[colorCount];
		std::vector<size_t> colorOrder(colorCount);
		for (size_t index = 0; index < SKETCH::GRADIENT_BRUSH_COUNT; index++) {
			// to get random index for color from 0 to 'colorCount'
			RandomizeColorOrder(colorOrder);

			for (size_t i = 0; i < colorCount; i++) {
				p_gradientStopList[i].position = interval * i;
				// intellisence warning is not correct
				p_gradientStopList[i].color = colorList[colorOrder[i]];
			}

			ap_view->m_gradientTable.insert(
				{
					index,
					ap_view->CreateLinearGradientBrush(p_gradientStopList, colorCount, &gradientData)
				}
			);
		}
	};

	////////////////////////////////////////////////////////////////
	// implementation
	////////////////////////////////////////////////////////////////?

	auto result = Direct2DEx::Create();
	if (S_OK != result) {
		return result;
	}

	mp_screenBitmap = CreateBitmapFromHBitmap(mh_screenBitmap);;
	m_viewRect = {
		0.0f, 0.0f,
		static_cast<float>(m_physicalRect.right - m_physicalRect.left), static_cast<float>(m_physicalRect.bottom - m_physicalRect.top)
	};
	mp_dashStroke = CreateUserStrokeStyle(D2D1_DASH_STYLE_DASH);
	InitGradientTable(this);

	return S_OK;
}

void SketchView::Paint(const std::vector<SKETCH::MD> &a_modelDataList)
{
	static const auto DrawBorder = [](SketchView *const ap_view)
	{
		ap_view->SetStrokeWidth(4.0f);
		ap_view->SetBrushColor(ap_view->m_colorSet.highlight);
		ap_view->DrawRectangle(ap_view->m_viewRect);
		ap_view->SetStrokeWidth(1.0f);
	};
	static const auto SetByDefaultData = [](SketchView *const ap_view, SKETCH::DD a_data)
	{
		ID2D1Brush *p_previousBrush = nullptr;
		if (SKETCH::INVALID_INDEX != a_data.gradientBrushIndex) {
			const auto p_gradientBrush = ap_view->m_gradientTable.at(a_data.gradientBrushIndex);
			p_gradientBrush->SetOpacity(a_data.opacity);
			p_previousBrush = ap_view->SetBrush(p_gradientBrush);
		}
		else {
			DColor color = a_data.color;
			color.a = a_data.opacity;
			ap_view->SetBrushColor(color);
		}

		ap_view->SetStrokeWidth(static_cast<float>(a_data.strokeWidth));

		return p_previousBrush;
	};
	static const auto DrawCurve = [](SketchView *const ap_view, SKETCH::MD a_modelData)
	{
		ID2D1PathGeometry *p_geometry;
		if (S_OK == gp_appCore->GetFactory()->CreatePathGeometry(&p_geometry)) {
			ID2D1GeometrySink *p_sink;
			if (S_OK == p_geometry->Open(&p_sink)) {
				p_sink->BeginFigure(a_modelData.points[0], D2D1_FIGURE_BEGIN_FILLED);

				size_t count = a_modelData.points.size();
				if (count > 1) {
					for (size_t i = 1; i < count; i++) {
						p_sink->AddLine(a_modelData.points[i]);
					}
				}

				p_sink->EndFigure(D2D1_FIGURE_END_OPEN);
				p_sink->Close();

				const auto p_previousBrush = SetByDefaultData(ap_view, a_modelData.defaultData);
				ap_view->DrawGeometry(p_geometry);
				if (nullptr != p_previousBrush) {
					ap_view->SetBrush(p_previousBrush);
				}

				p_sink->Release();
				p_geometry->Release();
			}
		}
	};
	static const auto DrawTextExtent = [](SketchView *const ap_view, const DRect &a_rect, const float &strokeWith)
	{
		const size_t circleCount = 4;
		float offset = strokeWith * 2;
		if (offset > 5.0f) {
			offset = 5.0f;
		}
		DRect rectList[circleCount] = {
			{ a_rect.left - offset, a_rect.top - offset, a_rect.left + offset, a_rect.top + offset },
			{ a_rect.right - offset, a_rect.top - offset, a_rect.right + offset, a_rect.top + offset },
			{ a_rect.left - offset, a_rect.bottom - offset, a_rect.left + offset, a_rect.bottom + offset },
			{ a_rect.right - offset, a_rect.bottom - offset, a_rect.right + offset, a_rect.bottom + offset }
		};

		const auto previousStroke = ap_view->SetStrokeStyle(ap_view->mp_dashStroke);
		ap_view->SetBrushColor(ap_view->m_colorSet.extentColor);
		ap_view->DrawRectangle(a_rect);
		ap_view->SetStrokeStyle(previousStroke);

		for (int i = 0; i < circleCount; i++) {
			ap_view->DrawEllipse(rectList[i]);
		}

		ap_view->SetBrushColor(ap_view->m_colorSet.extentInvertColor);
		for (int i = 0; i < circleCount; i++) {
			ap_view->FillEllipse(rectList[i]);
		}
	};
	static const auto DrawTextOutline = [](SketchView *const ap_view, SKETCH::MD a_modelData)
	{		
		const auto previousStroke = ap_view->SetStrokeStyle(ap_view->mp_dashStroke);
		ap_view->SetFontSize(static_cast<float>(a_modelData.defaultData.fontSize)); // TODO:: change font size to pixel
		ap_view->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING, DWRITE_PARAGRAPH_ALIGNMENT_NEAR);

		const auto startPoint = a_modelData.points.front();
		DSize textSize = ap_view->GetTextExtent(L"Text");
		const float strokeWidth = a_modelData.defaultData.fontSize * 0.02f;
		DRect extentRect;

		ap_view->SetStrokeWidth(strokeWidth);
		if (SKETCH::INVALID_INDEX != a_modelData.defaultData.gradientBrushIndex) {
			const auto p_gradientBrush = ap_view->m_gradientTable.at(a_modelData.defaultData.gradientBrushIndex);
			p_gradientBrush->SetOpacity(a_modelData.defaultData.opacity);
			const auto p_previousBrush = ap_view->SetBrush(p_gradientBrush);
			extentRect = ap_view->DrawTextOutline(L"Text", startPoint, textSize.height);
			ap_view->SetBrush(p_previousBrush);
		}
		else {
			DColor color = a_modelData.defaultData.color;
			color.a = a_modelData.defaultData.opacity;
			ap_view->SetBrushColor(color);
			extentRect = ap_view->DrawTextOutline(L"Text", startPoint, textSize.height);
		}

		ap_view->SetStrokeStyle(previousStroke);
		extentRect.top = startPoint.y;
		extentRect.right += strokeWidth;
		extentRect.bottom = startPoint.y + textSize.height;

		DrawTextExtent(ap_view, extentRect, strokeWidth);
	};
	static const auto DrawUserText = [](SketchView *const ap_view, SKETCH::MD a_modelData) -> std::pair<DRect, float>
	{
		if (a_modelData.text.length() < 1) {
			const bool isDone = false;
			::PostMessage(ap_view->mh_window, SKETCH::WM_SET_TEXTOUTLINE_MODE, 0, isDone);
			DrawTextOutline(ap_view, a_modelData);
			
			return {};
		}

		ap_view->SetFontSize(static_cast<float>(a_modelData.defaultData.fontSize)); // TODO:: change font size to pixel
		ap_view->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING, DWRITE_PARAGRAPH_ALIGNMENT_NEAR);

		const auto startPoint = a_modelData.points.front();
		const float maxTextWidth = ap_view->m_physicalRect.right - ap_view->m_physicalRect.left - startPoint.x;
		const float maxTextHeight = ap_view->m_physicalRect.bottom - ap_view->m_physicalRect.top - startPoint.y;
		auto &text = a_modelData.text;
		const auto originalTextLength = text.length();
		DSize textSize = ap_view->GetTextExtent(text.c_str(), maxTextWidth, maxTextHeight);

		// if the text width is lager than monitor screen
		while (textSize.height > maxTextHeight) {
			// delete a last letter
			text.pop_back();
			textSize = ap_view->GetTextExtent(text.c_str(), maxTextWidth, maxTextHeight);
		}

		// if the text width is lager than the max text width
		if (originalTextLength != text.length()) {
			const auto textLength = text.length();
			auto p_memoryText = new wchar_t[textLength + 1];
			wcscpy_s(p_memoryText, textLength + 1, text.c_str());

			::PostMessage(ap_view->mh_window, SKETCH::WM_ON_EDIT_MAX_LEGNTH, reinterpret_cast<WPARAM>(p_memoryText), textLength);
		}

		const DRect rect = { startPoint.x, startPoint.y, startPoint.x + textSize.width, startPoint.y + textSize.height };
		const float strokeWidth = a_modelData.defaultData.fontSize * 0.02f;

		ap_view->SetStrokeWidth(strokeWidth);
		if (SKETCH::INVALID_INDEX != a_modelData.defaultData.gradientBrushIndex) {
			const auto p_gradientBrush = ap_view->m_gradientTable.at(a_modelData.defaultData.gradientBrushIndex);
			p_gradientBrush->SetOpacity(a_modelData.defaultData.opacity);
			const auto p_previousBrush = ap_view->SetBrush(p_gradientBrush);
			ap_view->DrawUserText(text.c_str(), rect);
			ap_view->SetBrush(p_previousBrush);
			
		}
		else {
			DColor color = a_modelData.defaultData.color;
			color.a = a_modelData.defaultData.opacity;
			ap_view->SetBrushColor(color);
			ap_view->DrawUserText(text.c_str(), rect);
		}

		return { rect, strokeWidth };
	};
	
	///////////////////////////////////////////////////////////////////
	// implementation
	///////////////////////////////////////////////////////////////////
	
	static std::map< WINDOW_BRUSH::DT, void (*)(SketchView *const, const SKETCH::MD &)> paintTable = {
		{
			WINDOW_BRUSH::DT::CURVE,
			[](SketchView *const ap_view, const SKETCH::MD &a_modelData) {
				DrawCurve(ap_view, a_modelData);
			}
		},
		{
			WINDOW_BRUSH::DT::RECTANGLE,
			[](SketchView *const ap_view, const SKETCH::MD &a_modelData) {
				const auto p_previousBrush = SetByDefaultData(ap_view, a_modelData.defaultData);
				ap_view->DrawRectangle(a_modelData.rect);

				if (nullptr != p_previousBrush) {
					ap_view->SetBrush(p_previousBrush);
				}
			}
		},
		{
			WINDOW_BRUSH::DT::CIRCLE,
			[](SketchView *const ap_view, const SKETCH::MD &a_modelData) {
				const auto p_previousBrush = SetByDefaultData(ap_view, a_modelData.defaultData);
				ap_view->DrawEllipse(a_modelData.rect);

				if (nullptr != p_previousBrush) {
					ap_view->SetBrush(p_previousBrush);
				}
			}
		},
		{
			WINDOW_BRUSH::DT::TEXT_OUTLINE,
			[](SketchView *const ap_view, const SKETCH::MD &a_modelData) {
				DrawTextOutline(ap_view, a_modelData);
			}
		},
		{
			WINDOW_BRUSH::DT::TEXT_TYPING,
			[](SketchView *const ap_view, const SKETCH::MD &a_modelData) {
				const auto data = DrawUserText(ap_view, a_modelData);
				DrawTextExtent(ap_view, data.first, data.second);
			}
		},
		{
			WINDOW_BRUSH::DT::TEXT,
			[](SketchView *const ap_view, const SKETCH::MD &a_modelData) {
				DrawUserText(ap_view, a_modelData);
			}
		}
	};

	if (nullptr != mp_screenBitmap) {
		DrawBitmap(mp_screenBitmap, m_viewRect);
	}
	DrawBorder(this);

	for (const auto &modelData : a_modelDataList) {
		paintTable.at(modelData.drawType)(this, modelData);
	}
}

unsigned char SketchView::GetAverageBrightness(const HBITMAP &ah_bitmap)
{
	BITMAP bitmapInfo;
	::GetObject(ah_bitmap, sizeof(BITMAP), &bitmapInfo);

	const unsigned short bytePerPixel = bitmapInfo.bmBitsPixel / 8;
	const size_t patternSize = bitmapInfo.bmWidth * bitmapInfo.bmHeight * bytePerPixel;
	auto p_pattern = std::make_unique<unsigned char[]>(patternSize);
	::GetBitmapBits(ah_bitmap, patternSize, p_pattern.get());

	unsigned int sum = 0;
	for (size_t i = 0; i < patternSize; i += bytePerPixel) {
		sum += (p_pattern[i] + p_pattern[i + 1] + p_pattern[i + 2]) / 3;
	}

	return static_cast<unsigned char>(sum / (bitmapInfo.bmWidth * bitmapInfo.bmHeight));
};