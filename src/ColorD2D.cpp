#include "ColorD2D.h"
#include "Utility.h"
#include "ColorPalette.h"

extern ApplicationCore *gp_appCore;

ColorD2D::ColorD2D(const HWND ah_window, const RECT *const ap_viewRect) :
	Direct2DEx(ah_window, ap_viewRect)
{
	m_selectedHue = RGB_TO_COLORF((COLORREF)0x0100e3);
	m_selectedLightness = m_selectedHue;

	memset(&m_hueRect, 0, sizeof(DRect));
	memset(&m_lightnessRect, 0, sizeof(DRect));

	mp_lightnessGradientBrush = nullptr;
	mp_memoryBitmap = nullptr;
	mp_memoryTarget = nullptr;
}

ColorD2D::~ColorD2D()
{

}

void ColorD2D::InitHueData(const DRect &a_hueRect)
{
	const auto InitHueDataList = [](const DRect &a_hueRect, std::vector<std::pair<DColor, std::pair<DPoint, DPoint>>> &a_hueDataList)
	{
		const float STROKE_WIDTH = 2.5f;

		const float radius = (a_hueRect.right - a_hueRect.left) * 0.33f;
		const float startHueRadius = radius - STROKE_WIDTH;
		const float endHueRadius = radius + STROKE_WIDTH;
		const float centerPosX = a_hueRect.left + (a_hueRect.right - a_hueRect.left) / 2.0f;
		const float centerPosY = a_hueRect.top + (a_hueRect.bottom - a_hueRect.top) / 2.0f;

		a_hueDataList.resize(720);

		double radian;
		unsigned int degree = 0;
		for (auto &hueData : a_hueDataList) {
			radian = PI * degree / 360;

			// set strat point
			hueData.second.first = {
				static_cast<float>(centerPosX + startHueRadius * cos(radian)),
				static_cast<float>(centerPosY + startHueRadius * sin(radian))
			};
			// set end point
			hueData.second.second = {
				static_cast<float>(centerPosX + endHueRadius * cos(radian)),
				static_cast<float>(centerPosY + endHueRadius * sin(radian))
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

	const float width = a_hueRect.right - a_hueRect.left;
	const float height = a_hueRect.bottom - a_hueRect.top;
	const float radius = width * 0.2f;
	const float centerPosX = a_hueRect.left + width / 2.0f;
	const float centerPosY = a_hueRect.top + height / 2.0f;

	m_hueRect = a_hueRect;
	m_lightnessRect = {
		centerPosX - radius, centerPosY - radius,
		centerPosX + radius, centerPosY + radius
	};

	auto result = gp_appCore->GetWICFactory()->CreateBitmap(
		static_cast<unsigned int>(width), static_cast<unsigned int>(height),
		GUID_WICPixelFormat32bppPRGBA, WICBitmapCacheOnDemand, &mp_memoryBitmap
	);
	if (S_OK != result) {
		return;
	}

	result = gp_appCore->GetFactory()->CreateWicBitmapRenderTarget(mp_memoryBitmap, D2D1::RenderTargetProperties(), &mp_memoryTarget);
	if (S_OK != result) {
		return;
	}

	InitHueDataList(a_hueRect, m_hueDataList);
	UpdateMemoryHueCircle(mp_memoryTarget, m_hueDataList);
	UpdateHueData(m_selectedHue);

	return;
}

void ColorD2D::UpdateHueData(const DColor &a_hue)
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

	const float radius = (m_hueRect.right - m_hueRect.left) / 2.0f;

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
	mp_lightnessGradientBrush = CreateLinearGradientBrush(p_gradientStopList, gradientStopCount, &gradientData);

	////////////////////////////////////////////////////////////////
	// update memory pattern
	////////////////////////////////////////////////////////////////
	WICRect wicRect = { 
		static_cast<int>(m_hueRect.left), static_cast<int>(m_hueRect.top),
		static_cast<int>(m_hueRect.right - m_hueRect.left),  static_cast<int>(m_hueRect.bottom - m_hueRect.top)
	};
	IWICBitmapLock *p_lock = nullptr;

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

void ColorD2D::DrawHueCircle()
{
	for (const auto &hueData : m_hueDataList) {
		SetBrushColor(hueData.first);
		DrawLine(hueData.second.first, hueData.second.second);
	}
}

void ColorD2D::DrawLightnessCircle()
{
	auto p_previousBrush = SetBrush(mp_lightnessGradientBrush);
	FillEllipse(m_lightnessRect);
	SetBrush(p_previousBrush);
}