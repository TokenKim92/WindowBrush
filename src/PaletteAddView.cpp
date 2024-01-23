#include "PaletteAddView.h"
#include "ColorPalette.h"
#include "Utility.h"

extern ApplicationCore *gp_appCore;

PaletteAddView::PaletteAddView(Direct2DEx *const ap_direct2d, const CM &a_mode) :
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
	memset(&m_currentLightness, 0, sizeof(DColor));
	memset(&m_lightnessRect, 0, sizeof(DRect));

	m_indicateRect = {
		PALETTE::DIALOG_WIDTH - PALETTE::INDICATE_WIDTH - PALETTE::INDICATE_MARGIN,
		PALETTE::DIALOG_HEIGHT - PALETTE::INDICATE_HEIGHT - PALETTE::INDICATE_MARGIN,
		PALETTE::DIALOG_WIDTH - PALETTE::INDICATE_MARGIN,
		PALETTE::DIALOG_HEIGHT - PALETTE::INDICATE_MARGIN
	};

	mp_lightnessGradientBrush = nullptr;
	mp_indicateFont = nullptr;

	mp_memoryBitmap = nullptr;
	mp_memoryTarget = nullptr;
}

PaletteAddView::~PaletteAddView()
{
	InterfaceRelease(&mp_lightnessGradientBrush);
	InterfaceRelease(&mp_indicateFont);
	InterfaceRelease(&mp_memoryBitmap);
	InterfaceRelease(&mp_memoryTarget);
}

void PaletteAddView::Init(const HWND &ah_wnd, const DPoint &a_centerPoint, const SIZE &a_viewSize)
{
	const auto InitHueDataList = [](PaletteAddView *const ap_view, const float &a_centerPosX, const float &a_centerPosY)
	{
		const float startHueRadius = PALETTE::HUE_CIRCLE_RADIUS - PALETTE::HUE_STROKE_HALF_WIDTH;
		const float endHueRadius = PALETTE::HUE_CIRCLE_RADIUS + PALETTE::HUE_STROKE_HALF_WIDTH;

		ap_view->m_hueDataList.resize(720);

		double radian;
		unsigned int degree = 0;
		for (auto &hueData : ap_view->m_hueDataList) {
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
			hueData.first = FromHueToColor(degree / 120.0f);

			degree++;
		}
	};
	const auto InitMemoryInterfaces = [](PaletteAddView *const ap_view, const SIZE &a_viewSize)
	{
		if (S_OK == gp_appCore->GetWICFactory()->CreateBitmap(
			a_viewSize.cx, a_viewSize.cy, GUID_WICPixelFormat32bppPRGBA, WICBitmapCacheOnDemand, &ap_view->mp_memoryBitmap
		)) {
			if (S_OK == gp_appCore->GetFactory()->CreateWicBitmapRenderTarget(
				ap_view->mp_memoryBitmap, D2D1::RenderTargetProperties(), &ap_view->mp_memoryTarget
			)) {
				return true;
			}
		}

		return false;
	};
	const auto UpdateMemoryHueCircle = [](PaletteAddView *const ap_view)
	{
		ID2D1SolidColorBrush *p_solidBrush;
		auto result = ap_view->mp_memoryTarget->CreateSolidColorBrush(DColor({ 0.0f, 0.0f, 0.0f, 1.0f }), &p_solidBrush);
		if (S_OK != result) {
			return;
		}

		ap_view->mp_memoryTarget->BeginDraw();

		for (const auto &hueData : ap_view->m_hueDataList) {
			p_solidBrush->SetColor(hueData.first);
			ap_view->mp_memoryTarget->DrawLine(hueData.second.first, hueData.second.second, p_solidBrush);
		}

		ap_view->mp_memoryTarget->EndDraw();

		p_solidBrush->Release();
	};

	////////////////////////////////////////////////////////////////
	// implementation
	////////////////////////////////////////////////////////////////

	if (!InitMemoryInterfaces(this, a_viewSize)) {
		::DestroyWindow(ah_wnd);
		return;
	}

	InitHueDataList(this, a_centerPoint.x, a_centerPoint.y);
	UpdateMemoryHueCircle(this);

	m_viewSize = a_viewSize;
	m_lightnessRect = {
		a_centerPoint.x - PALETTE::LIGHTNESS_CIRCLE_RADIUS, a_centerPoint.y - PALETTE::LIGHTNESS_CIRCLE_RADIUS,
		a_centerPoint.x + PALETTE::LIGHTNESS_CIRCLE_RADIUS, a_centerPoint.y + PALETTE::LIGHTNESS_CIRCLE_RADIUS
	};
	m_returnIconPoints = {
		{
			{ PALETTE::RETURN_ICON_X_MARGIN, PALETTE::TITLE_HEIGHT / 2.0f },
			{ PALETTE::TITLE_HEIGHT - PALETTE::RETURN_ICON_X_MARGIN, PALETTE::RETURN_ICON_Y_MARGIN }
		},
		{
			{ PALETTE::RETURN_ICON_X_MARGIN, PALETTE::TITLE_HEIGHT / 2.0f },
			{ PALETTE::TITLE_HEIGHT - PALETTE::RETURN_ICON_X_MARGIN, PALETTE::TITLE_HEIGHT - PALETTE::RETURN_ICON_Y_MARGIN }
		}
	};
	m_buttonTable = {
		{
			PALETTE::BT::RETURN,
			{ 
				PALETTE::RETURN_ICON_X_MARGIN, PALETTE::RETURN_ICON_Y_MARGIN, 
				PALETTE::TITLE_HEIGHT - PALETTE::RETURN_ICON_X_MARGIN, PALETTE::TITLE_HEIGHT - PALETTE::RETURN_ICON_Y_MARGIN,
			}
		},
		{
			PALETTE::BT::ADD,
			{
				 PALETTE::ADD_BUTTON_MARGIN, PALETTE::DIALOG_HEIGHT - PALETTE::ADD_BUTTON_SIZE - PALETTE::ADD_BUTTON_MARGIN,
				PALETTE::ADD_BUTTON_SIZE + PALETTE::ADD_BUTTON_MARGIN, PALETTE::DIALOG_HEIGHT - PALETTE::ADD_BUTTON_MARGIN
			}

		}
	};

	mp_indicateFont = mp_direct2d->CreateTextFormat(DEFAULT_FONT_NAME, 13.0f, DWRITE_FONT_WEIGHT_SEMI_BOLD, DWRITE_FONT_STYLE_NORMAL);
	mp_indicateFont->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
	mp_indicateFont->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);

	mp_memoryPattern = std::make_unique<unsigned char[]>(PALETTE::DIALOG_WIDTH * PALETTE::DIALOG_HEIGHT * sizeof(unsigned int));

	return;
}

void PaletteAddView::Paint(const PALETTE::MD &a_modelData)
{
	static const auto DrawReturnButton = [](PaletteAddView *const ap_view, const PALETTE::MD &a_modelData)
	{
		DColor color = ap_view->m_mainColor;
		if (PALETTE::BT::RETURN == a_modelData.clickedButtonType || PALETTE::BT::RETURN != a_modelData.hoverButtonType) {
			color.a = PALETTE::DEFAULT_TRANSPARENCY;
		}
		ap_view->mp_direct2d->SetBrushColor(color);
		ap_view->mp_direct2d->SetStrokeWidth(3.0f);
		ap_view->mp_direct2d->DrawLine(ap_view->m_returnIconPoints[0].first, ap_view->m_returnIconPoints[0].second);
		ap_view->mp_direct2d->DrawLine(ap_view->m_returnIconPoints[1].first, ap_view->m_returnIconPoints[1].second);
		ap_view->mp_direct2d->SetStrokeWidth(1.0f);
	};
	static const auto DrawHueCircle = [](PaletteAddView *const ap_view)
	{
		for (const auto &hueData : ap_view->m_hueDataList) {
			ap_view->mp_direct2d->SetBrushColor(hueData.first);
			ap_view->mp_direct2d->DrawLine(hueData.second.first, hueData.second.second);
		}
	};
	static const auto DrawLightnessCircle = [](PaletteAddView *const ap_view)
	{
		if (nullptr == ap_view->mp_lightnessGradientBrush) {
			ap_view->mp_direct2d->FillEllipse(ap_view->m_lightnessRect);

			return;
		}

		auto p_previousBrush = ap_view->mp_direct2d->SetBrush(ap_view->mp_lightnessGradientBrush);
		ap_view->mp_direct2d->FillEllipse(ap_view->m_lightnessRect);
		ap_view->mp_direct2d->SetBrush(p_previousBrush);
	};
	static const auto DrawHueButton = [](PaletteAddView *const ap_view, const PALETTE::MD &a_modelData)
	{
		DColor color = ap_view->m_mainColor;
		if (PALETTE::BT::HUE != a_modelData.hoverButtonType) {
			color.a = PALETTE::DEFAULT_TRANSPARENCY;
		}

		// draw the circle to show the current color
		ap_view->mp_direct2d->SetBrushColor(color);
		ap_view->mp_direct2d->FillEllipse(a_modelData.hueButtonRect);
	};
	static const auto DrawLightnessButton = [](PaletteAddView *const ap_view, const PALETTE::MD &a_modelData)
	{
		auto rect = a_modelData.lightnessButtonRect;
		const DPoint centerPos = {
			rect.left + (rect.right - rect.left) / 2.0f ,rect.top + (rect.bottom - rect.top) / 2.0f
		};
		if (PALETTE::BT::LIGHTNESS == a_modelData.clickedButtonType) {
			ExpandRect(rect, PALETTE::COLOR_RADIUS);
		}

		// fill button
		const DColor colorOnPoint = ap_view->GetPixelColorOnPoint(centerPos);
		ap_view->mp_direct2d->SetBrushColor(colorOnPoint);
		ap_view->mp_direct2d->FillEllipse(rect);

		// draw border button
		DColor color;
		if (0.5f > GetBrightness(colorOnPoint)) {
			color = RGB_TO_COLORF(NEUTRAL_100);
		}
		else {
			color = RGB_TO_COLORF(NEUTRAL_900);
		}

		if (PALETTE::BT::LIGHTNESS != a_modelData.hoverButtonType) {
			color.a = PALETTE::DEFAULT_TRANSPARENCY;
		}

		ap_view->mp_direct2d->SetStrokeWidth(2.0f);
		ap_view->mp_direct2d->SetBrushColor(color);
		ap_view->mp_direct2d->DrawEllipse(rect);
		ap_view->mp_direct2d->SetStrokeWidth(1.0f);

		return colorOnPoint;
	};
	static const auto DrawAddButton = [](PaletteAddView *const ap_view, const PALETTE::MD &a_modelData)
	{
		// draw main circle
		DRect mainRect = ap_view->m_buttonTable.at(PALETTE::BT::ADD);
		if (PALETTE::BT::ADD == a_modelData.clickedButtonType) {
			ShrinkRect(mainRect, 1.0f);
		}
		else if (PALETTE::BT::ADD == a_modelData.hoverButtonType) {
			ExpandRect(mainRect, 2.0f);
		}

		ap_view->mp_direct2d->SetBrushColor(ap_view->m_currentLightness);
		ap_view->mp_direct2d->FillEllipse(mainRect);
		ap_view->mp_direct2d->SetBrushColor(ap_view->m_mainColor);
		ap_view->mp_direct2d->DrawEllipse(mainRect);

		// draw small circle
		float offset = 2.0f;
		const DRect smallRect = {
			mainRect.right - PALETTE::PLUS_BUTTON_RADIUS - offset, mainRect.top + PALETTE::PLUS_BUTTON_RADIUS + offset,
			mainRect.right + PALETTE::PLUS_BUTTON_RADIUS - offset, mainRect.top - PALETTE::PLUS_BUTTON_RADIUS + offset,
		};
		ap_view->mp_direct2d->FillEllipse(smallRect);

		// draw + on small circle
		offset = 3.0f;
		const float centerPosX = (smallRect.left + smallRect.right) / 2.0f;
		const float centerPosY = (smallRect.top + smallRect.bottom) / 2.0f;

		DPoint startPos = { smallRect.left + offset, centerPosY };
		DPoint endPos = { smallRect.right - offset, centerPosY };
		ap_view->mp_direct2d->SetStrokeWidth(2.0f);
		ap_view->mp_direct2d->SetBrushColor(ap_view->m_oppositeColor);
		ap_view->mp_direct2d->DrawLine(startPos, endPos);

		startPos = { centerPosX, smallRect.top - offset };
		endPos = { centerPosX, smallRect.bottom + offset };
		ap_view->mp_direct2d->DrawLine(startPos, endPos);
		ap_view->mp_direct2d->SetStrokeWidth(1.0f);

		// draw title
		offset = 80.0f;
		const auto rect = ap_view->m_buttonTable.at(PALETTE::BT::ADD);
		const DRect titleRect = {
			rect.right, rect.top,
			rect.right + offset, rect.bottom
		};

		auto previousFont = ap_view->mp_direct2d->SetTextFormat(ap_view->mp_indicateFont);
		ap_view->mp_direct2d->SetBrushColor(ap_view->m_mainColor);
		ap_view->mp_direct2d->DrawUserText(L"Add color", titleRect);
		ap_view->mp_direct2d->SetTextFormat(previousFont);
	};
	static const auto DrawIndicate = [](PaletteAddView *const ap_view)
	{
		std::wstring rgbText = L"HEX: " +
			FloatToHexWString(ap_view->m_currentLightness.r) +
			FloatToHexWString(ap_view->m_currentLightness.g) +
			FloatToHexWString(ap_view->m_currentLightness.b);

		ap_view->mp_direct2d->SetBrushColor(ap_view->m_oppositeColor);
		ap_view->mp_direct2d->FillRoundedRectangle(ap_view->m_indicateRect, 3.0f);

		auto previousFont = ap_view->mp_direct2d->SetTextFormat(ap_view->mp_indicateFont);
		ap_view->mp_direct2d->SetBrushColor(ap_view->m_mainColor);
		ap_view->mp_direct2d->DrawUserText(rgbText.c_str(), ap_view->m_indicateRect);
		ap_view->mp_direct2d->SetTextFormat(previousFont);
	};

	////////////////////////////////////////////////////////////////
	// implementation
	////////////////////////////////////////////////////////////////

	DrawReturnButton(this, a_modelData);

	DrawHueCircle(this);
	DrawHueButton(this, a_modelData);

	DrawLightnessCircle(this);
	m_currentLightness = DrawLightnessButton(this, a_modelData);

	DrawAddButton(this, a_modelData);
	DrawIndicate(this);
}

void PaletteAddView::UpdateLightnessData(const DColor &a_hue)
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
	WICRect wicRect = { 0, 0, PALETTE::DIALOG_WIDTH, PALETTE::DIALOG_HEIGHT };
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
					memcpy(mp_memoryPattern.get(), p_pattern, bufferSize);
				}
			}
		}
		p_lock->Release();
	}
}

const std::map<PALETTE::BT, DRect> &PaletteAddView::GetButtonTable()
{
	return m_buttonTable;
}

DColor PaletteAddView::GetPixelColorOnPoint(const DPoint &a_point)
{
	const auto memoryIndex = PALETTE::DIALOG_WIDTH * static_cast<unsigned int>(a_point.y) + static_cast<unsigned int>(a_point.x);
	const auto p_color = reinterpret_cast<COLORREF *>(mp_memoryPattern.get()) + memoryIndex;
	return RGB_TO_COLORF(*p_color);
}

DColor &PaletteAddView::GetCurrentLightness()
{
	return m_currentLightness;
}
