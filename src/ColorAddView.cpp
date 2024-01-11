#include "ColorAddView.h"
#include "Utility.h"
#include "ColorPalette.h"

extern ApplicationCore *gp_appCore;

ColorAddView::ColorAddView(Direct2DEx *const ap_direct2d, const CM &a_mode) :
	mp_direct2d(ap_direct2d)
{
	memset(&m_viewSize, 0, sizeof(SIZE));

	if (CM::DARK == a_mode) {
		m_mainColor = RGB_TO_COLORF(NEUTRAL_100);
		m_oppositeColor = RGB_TO_COLORF(NEUTRAL_900);
	}
	else {
		m_mainColor = RGB_TO_COLORF(NEUTRAL_600);
		m_oppositeColor = RGB_TO_COLORF(NEUTRAL_200);
	}

	memset(&m_lightnessRect, 0, sizeof(DRect));

	mp_lightnessGradientBrush = nullptr;
	mp_indicateFont = nullptr;

	mp_memoryBitmap = nullptr;
	mp_memoryTarget = nullptr;
}

ColorAddView::~ColorAddView()
{
	InterfaceRelease(&mp_lightnessGradientBrush);
	InterfaceRelease(&mp_indicateFont);
	InterfaceRelease(&mp_memoryBitmap);
	InterfaceRelease(&mp_memoryTarget);
}

void ColorAddView::Init(const DPoint &a_centerPoint, const SIZE &a_viewSize)
{
	const auto InitHueDataList = [](
		const float a_radius, const float &a_centerPosX, const float &a_centerPosY,
		std::vector<std::pair<DColor, std::pair<DPoint, DPoint>>> &a_hueDataList
		)
	{
		const float STROKE_WIDTH = 2.5f;
		const float startHueRadius = a_radius - STROKE_WIDTH;
		const float endHueRadius = a_radius + STROKE_WIDTH;

		a_hueDataList.resize(720);

		double radian;
		unsigned int degree = 0;
		for (auto &hueData : a_hueDataList) {
			radian = PI * degree / 360;

			// set strat point
			hueData.second.first = {
				static_cast<float>(a_centerPosX + startHueRadius * cos(radian)),
				static_cast<float>(a_centerPosY + startHueRadius * sin(radian))
			};
			// set end point
			hueData.second.second = {
				static_cast<float>(a_centerPosX + endHueRadius * cos(radian)),
				static_cast<float>(a_centerPosY + endHueRadius * sin(radian))
			};
			// set color
			hueData.first = FromHueToColor(degree / 120.0f);;

			degree++;
		}
	};

	const auto UpdateMemoryHueCircle = [](ID2D1RenderTarget *const ap_memoryTarget, std::vector<std::pair<DColor, std::pair<DPoint, DPoint>>> &a_hueDataList)
	{
		ID2D1SolidColorBrush *p_solidBrush;
		auto result = ap_memoryTarget->CreateSolidColorBrush(DColor({ 0.0f, 0.0f, 0.0f, 1.0f }), &p_solidBrush);
		if (S_OK != result) {
			return;
		}

		ap_memoryTarget->BeginDraw();

		for (const auto &hueData : a_hueDataList) {
			p_solidBrush->SetColor(hueData.first);
			ap_memoryTarget->DrawLine(hueData.second.first, hueData.second.second, p_solidBrush);
		}

		ap_memoryTarget->EndDraw();

		p_solidBrush->Release();
	};

	////////////////////////////////////////////////////////////////
	// implementation
	////////////////////////////////////////////////////////////////
	m_viewSize = a_viewSize;
	m_lightnessRect = {
		a_centerPoint.x - LIHTNESS_CIRCLE_RADIUS, a_centerPoint.y - LIHTNESS_CIRCLE_RADIUS,
		a_centerPoint.x + LIHTNESS_CIRCLE_RADIUS, a_centerPoint.y + LIHTNESS_CIRCLE_RADIUS
	};

	auto result = gp_appCore->GetWICFactory()->CreateBitmap(
		a_viewSize.cx, a_viewSize.cy, GUID_WICPixelFormat32bppPRGBA, WICBitmapCacheOnDemand, &mp_memoryBitmap
	);
	if (S_OK != result) {
		return;
	}

	result = gp_appCore->GetFactory()->CreateWicBitmapRenderTarget(mp_memoryBitmap, D2D1::RenderTargetProperties(), &mp_memoryTarget);
	if (S_OK != result) {
		return;
	}

	InitHueDataList(HUE_CIRCLE_RADIUS, a_centerPoint.x, a_centerPoint.y, m_hueDataList);
	UpdateMemoryHueCircle(mp_memoryTarget, m_hueDataList);

	m_returnIconPoints = {
	{{ 14.0f, TITLE_HEIGHT / 2.0f }, { TITLE_HEIGHT - 14.0f, 10.0f }},
	{{ 14.0f, TITLE_HEIGHT / 2.0f }, { TITLE_HEIGHT - 14.0f, TITLE_HEIGHT - 10.0f }}
	};

	const float margin = 10.0f;
	const DSize indicateButtonSize = { 100.0f, 35.0f };
	m_buttonTable = {
		{ CBT::RETURN, { 10.0f, 10.0f, TITLE_HEIGHT - 10.0f, TITLE_HEIGHT - 10.0f } },
		{
			CBT::INDICATE,
			{
				COLOR_DIALOG_WIDTH - indicateButtonSize.width - margin, COLOR_DIALOG_HEIGHT - indicateButtonSize.height - margin,
				COLOR_DIALOG_WIDTH - margin, COLOR_DIALOG_HEIGHT - margin
			}

		}
	};

	mp_indicateFont = mp_direct2d->CreateTextFormat(DEFAULT_FONT_NAME, 14.0f, DWRITE_FONT_WEIGHT_SEMI_BOLD, DWRITE_FONT_STYLE_NORMAL);
	mp_indicateFont->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
	mp_indicateFont->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);

	return;
}

void ColorAddView::Paint(const CMD &a_modelData)
{
	static const auto DrawReturnButton = [](
		Direct2DEx *const ap_direct2d, const DColor &a_mainColor, std::vector<std::pair<DPoint, DPoint>> &a_returnIconPoints, const CMD &a_modelData
		)
	{

		DColor color = a_mainColor;
		if (CBT::RETURN == a_modelData.clickedButtonType || CBT::RETURN != a_modelData.hoverButtonType) {
			color.a = DEFAULT_TRANSPARENCY;
		}
		ap_direct2d->SetBrushColor(color);
		ap_direct2d->SetStrokeWidth(3.0f);
		ap_direct2d->DrawLine(a_returnIconPoints[0].first, a_returnIconPoints[0].second);
		ap_direct2d->DrawLine(a_returnIconPoints[1].first, a_returnIconPoints[1].second);
		ap_direct2d->SetStrokeWidth(1.0f);
	};
	static const auto DrawHueCircle = [](
		Direct2DEx *const ap_direct2d, const std::vector<std::pair<DColor, std::pair<DPoint, DPoint>>> &a_hueDataList
		)
	{
		for (const auto &hueData : a_hueDataList) {
			ap_direct2d->SetBrushColor(hueData.first);
			ap_direct2d->DrawLine(hueData.second.first, hueData.second.second);
		}
	};
	static const auto DrawLightnessCircle = [](
		Direct2DEx *const ap_direct2d, ID2D1LinearGradientBrush *const ap_lightnessGradientBrush, const DRect &a_lightnessRect
		)
	{
		if (nullptr == ap_lightnessGradientBrush) {
			ap_direct2d->FillEllipse(a_lightnessRect);

			return;
		}

		auto p_previousBrush = ap_direct2d->SetBrush(ap_lightnessGradientBrush);
		ap_direct2d->FillEllipse(a_lightnessRect);
		ap_direct2d->SetBrush(p_previousBrush);
	};
	static const auto DrawHueButton = [](Direct2DEx *const ap_direct2d, const DColor &a_buttonColor, const CMD &a_modelData)
	{
		DColor color = a_buttonColor;
		if (CBT::HUE != a_modelData.hoverButtonType) {
			color.a = DEFAULT_TRANSPARENCY;
		}

		// draw the circle to show the current color
		ap_direct2d->SetBrushColor(color);
		ap_direct2d->FillEllipse(a_modelData.hueButtonRect);
	};
	static const auto DrawLightnessButton = [](ColorAddView *const ap_view, Direct2DEx *const ap_direct2d, const CMD &a_modelData)
	{
		auto rect = a_modelData.lightnessButtonRect;
		const DPoint centerPos = {
			rect.left + (rect.right - rect.left) / 2.0f ,rect.top + (rect.bottom - rect.top) / 2.0f
		};
		if (CBT::LIGHTNESS == a_modelData.clickedButtonType) {
			ExpandRect(rect, BUTTON_RADIUS);
		}

		// fill button
		const DColor colorOnPoint = ap_view->GetPixelColorOnPoint(centerPos);
		ap_direct2d->SetBrushColor(colorOnPoint);
		ap_direct2d->FillEllipse(rect);

		// draw border button
		DColor color;
		if (0.5f > GetBrightness(colorOnPoint)) {
			color = RGB_TO_COLORF(NEUTRAL_100);
		}
		else {
			color = RGB_TO_COLORF(NEUTRAL_900);
		}

		if (CBT::LIGHTNESS != a_modelData.hoverButtonType) {
			color.a = DEFAULT_TRANSPARENCY;
		}

		ap_direct2d->SetStrokeWidth(2.0f);
		ap_direct2d->SetBrushColor(color);
		ap_direct2d->DrawEllipse(rect);
		ap_direct2d->SetStrokeWidth(1.0f);

		return colorOnPoint;
	};
	static const auto DrawIndicate = [](
		Direct2DEx *const ap_direct2d, IDWriteTextFormat *const ap_font, const std::map<CBT, DRect> &a_buttonTable, 
		const DColor &a_lightness, const DColor &a_backgroundColor, const DColor &a_textColor
		)
	{
		const auto rect = a_buttonTable.at(CBT::INDICATE);
		std::wstring rgbText = L"HEX: " +
			FloatToHexWString(a_lightness.r) +
			FloatToHexWString(a_lightness.g) +
			FloatToHexWString(a_lightness.b);

		ap_direct2d->SetBrushColor(a_backgroundColor);
		ap_direct2d->FillRoundedRectangle(rect, 3.0f);

		auto previousFont = ap_direct2d->SetTextFormat(ap_font);
		ap_direct2d->SetBrushColor(a_textColor);
		ap_direct2d->DrawUserText(rgbText.c_str(), rect);
		ap_direct2d->SetTextFormat(previousFont);
	};

	////////////////////////////////////////////////////////////////
	// implementation
	////////////////////////////////////////////////////////////////

	DrawReturnButton(mp_direct2d, m_mainColor, m_returnIconPoints, a_modelData);

	DrawHueCircle(mp_direct2d, m_hueDataList);
	DrawHueButton(mp_direct2d, m_mainColor, a_modelData);

	DrawLightnessCircle(mp_direct2d, mp_lightnessGradientBrush, m_lightnessRect);
	const DColor lightness = DrawLightnessButton(this, mp_direct2d, a_modelData);
	
	DrawIndicate(mp_direct2d, mp_indicateFont, m_buttonTable, lightness, m_oppositeColor, m_mainColor);
}

void ColorAddView::UpdateLightnessData(const DColor &a_hue)
{
	////////////////////////////////////////////////////////////////
	// update memory render target
	////////////////////////////////////////////////////////////////
	D2D1_LINEAR_GRADIENT_BRUSH_PROPERTIES gradientData = {
		{m_lightnessRect.left, m_lightnessRect.bottom},
		{m_lightnessRect.right, m_lightnessRect.top}
	};

	const unsigned int gradientStopCount = 3;
	D2D1_GRADIENT_STOP p_gradientStopList[gradientStopCount] = {
		{ 0.177777f, {0.0f, 0.0f, 0.0f, 1.0f} },
		{ 0.5f, a_hue },
		{ 0.833333f, { 1.0f, 1.0f, 1.0f, 1.0f } }
	};

	ID2D1GradientStopCollection *p_gradientStop;
	auto result = mp_memoryTarget->CreateGradientStopCollection(p_gradientStopList, gradientStopCount, D2D1_GAMMA_2_2, D2D1_EXTEND_MODE_CLAMP, &p_gradientStop);
	if (S_OK != result) {
		return;
	}

	ID2D1LinearGradientBrush *p_lineGradientBrush;
	result = mp_memoryTarget->CreateLinearGradientBrush(gradientData, p_gradientStop, &p_lineGradientBrush);
	if (S_OK != result) {
		p_gradientStop->Release();

		return;
	}

	const float radius = (m_lightnessRect.right - m_lightnessRect.left) / 2.0f;

	mp_memoryTarget->BeginDraw();
	// draw lightness circle
	D2D1_ELLIPSE ellipse = { {m_lightnessRect.left + radius, m_lightnessRect.top + radius}, radius, radius };
	mp_memoryTarget->FillEllipse(&ellipse, p_lineGradientBrush);
	mp_memoryTarget->EndDraw();

	p_lineGradientBrush->Release();
	p_gradientStop->Release();

	////////////////////////////////////////////////////////////////
	// update render target
	////////////////////////////////////////////////////////////////
	mp_lightnessGradientBrush = mp_direct2d->CreateLinearGradientBrush(p_gradientStopList, gradientStopCount, &gradientData);

	////////////////////////////////////////////////////////////////
	// update memory pattern
	////////////////////////////////////////////////////////////////
	WICRect wicRect = { 0, 0, COLOR_DIALOG_WIDTH, COLOR_DIALOG_HEIGHT };
	IWICBitmapLock *p_lock = nullptr;

	if (nullptr == mp_memoryBitmap) {
		return;
	}

	if (S_OK == mp_memoryBitmap->Lock(&wicRect, WICBITMAPLOCKFLAGS_FORCE_DWORD, &p_lock)) {
		unsigned int bufferSize = 0;
		unsigned int stride = 0;
		WICInProcPointer p_pattern = nullptr;

		if (S_OK == p_lock->GetStride(&stride)) {
			if (S_OK == p_lock->GetDataPointer(&bufferSize, &p_pattern)) {
				if (p_pattern) {
					memcpy(m_memoryPattern, p_pattern, bufferSize);
				}
			}
		}
		p_lock->Release();
	}
}

const std::map<CBT, DRect> &ColorAddView::GetButtonTable()
{
	return m_buttonTable;
}

DColor ColorAddView::GetPixelColorOnPoint(const DPoint &a_point)
{
	const auto memoryIndex = COLOR_DIALOG_WIDTH * static_cast<unsigned int>(a_point.y) + static_cast<unsigned int>(a_point.x);
	const auto p_color = reinterpret_cast<COLORREF *>(m_memoryPattern) + memoryIndex;
	return RGB_TO_COLORF(*p_color);
}
