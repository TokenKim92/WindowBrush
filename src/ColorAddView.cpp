#include "ColorAddView.h"
#include "Utility.h"
#include "ColorPalette.h"

extern ApplicationCore *gp_appCore;

ColorAddView::ColorAddView(Direct2DEx *const ap_direct2d, const CM &a_mode) :
	mp_direct2d(ap_direct2d)
{
	memset(&m_viewRect, 0, sizeof(RECT));
	memset(&m_viewSize, 0, sizeof(SIZE));

	if (CM::DARK == a_mode) {
		m_mainColor = RGB_TO_COLORF(NEUTRAL_300);
	}
	else {
		m_mainColor = RGB_TO_COLORF(NEUTRAL_600);
	}
	m_selectedHue = RGB_TO_COLORF((COLORREF)0x0100e3);
	m_selectedLightness = m_selectedHue;
	memset(&m_lightnessRect, 0, sizeof(DRect));

	mp_lightnessGradientBrush = nullptr;

	mp_memoryBitmap = nullptr;
	mp_memoryTarget = nullptr;
}

ColorAddView::~ColorAddView()
{
	InterfaceRelease(&mp_lightnessGradientBrush);
	InterfaceRelease(&mp_memoryBitmap);
	InterfaceRelease(&mp_memoryTarget);
}

void ColorAddView::Init(const RECT &a_viewRect, const SIZE &a_viewSize)
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
	m_viewRect = a_viewRect;
	m_viewSize = a_viewSize;

	const int height = a_viewSize.cy - TEXT_HEIGHT - INDICATE_HEIGHT;
	const float radius = a_viewSize.cx * 0.2f;
	const float centerPosX = a_viewRect.left + a_viewSize.cx / 2.0f;
	const float centerPosY = TEXT_HEIGHT + a_viewRect.top + height / 2.0f;

	m_lightnessRect = {
		centerPosX - radius, centerPosY - radius,
		centerPosX + radius, centerPosY + radius
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

	InitHueDataList(a_viewSize.cx * 0.33f, centerPosX, centerPosY, m_hueDataList);
	UpdateMemoryHueCircle(mp_memoryTarget, m_hueDataList);
	UpdateLightnessData(m_selectedHue);


	m_returnIconPoints = {
		{{ 14.0f, TEXT_HEIGHT / 2.0f }, { TEXT_HEIGHT - 14.0f, 10.0f }},
		{{ 14.0f, TEXT_HEIGHT / 2.0f }, { TEXT_HEIGHT - 14.0f, TEXT_HEIGHT - 10.0f }}
	};

	m_buttonTable.insert({ CBT::RETURN, { 10.0f, 10.0f, TEXT_HEIGHT - 10.0f, TEXT_HEIGHT - 10.0f } });

	return;
}

void ColorAddView::Paint(const CMD &a_modelData)
{
	static const auto DrawReturnButton = [](
		Direct2DEx *const ap_direct2d, const DColor &a_mainColor, std::vector<std::pair<DPoint, DPoint>> &a_returnIconPoints, const CMD &a_modelData
		)
	{

		DColor color = a_mainColor;
		if (CBT::RETURN == a_modelData.clickedButton || CBT::RETURN != a_modelData.hoverButton) {
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


	// draw return button

	DrawReturnButton(mp_direct2d, m_mainColor, m_returnIconPoints, a_modelData);
	DrawHueCircle(mp_direct2d, m_hueDataList);
	DrawLightnessCircle(mp_direct2d, mp_lightnessGradientBrush, m_lightnessRect);
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

	const float radius = m_viewSize.cx / 2.0f;

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
	WICRect wicRect = {
		static_cast<int>(m_viewRect.left), static_cast<int>(m_viewRect.top),
		static_cast<int>(m_viewSize.cx), static_cast<int>(m_viewSize.cy - TEXT_HEIGHT - INDICATE_HEIGHT)
	};
	IWICBitmapLock *p_lock = nullptr;

	if (nullptr == mp_memoryBitmap) {
		return;
	}

	if (S_OK == mp_memoryBitmap->Lock(&wicRect, WICBITMAPLOCKFLAGS_FORCE_DWORD, &p_lock)) {
		unsigned int bufferSize = 0;
		unsigned int stride = 0;
		unsigned char *p_pattern = nullptr;

		if (S_OK == p_lock->GetStride(&stride)) {
			if (S_OK == p_lock->GetDataPointer(&bufferSize, &p_pattern)) {
				if (p_pattern) {
					mp_memoryPattern = std::make_unique<unsigned char[]>(bufferSize);
					memcpy(mp_memoryPattern.get(), p_pattern, bufferSize);
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