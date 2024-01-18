#include "SketchView.h"
#include "Utility.h"
#include "ColorPalette.h"
#include <vector>
#include <memory>

extern ApplicationCore *gp_appCore;

SketchView::SketchView(const HWND ah_window, const HBITMAP &ah_screenBitmap, const RECT &a_physicalRect, const CM &a_mode) :
	Direct2DEx(ah_window, &a_physicalRect),
	mh_screenBitmap(ah_screenBitmap),
	m_physicalRect(a_physicalRect)
{
	mp_screenBitmap = nullptr;
	mp_dashStroke = nullptr;

	if (CM::LIGHT == a_mode) {
		m_colorSet.highlight = RGB_TO_COLORF(ORANGE_300);
	}
	else {
		m_colorSet.highlight = RGB_TO_COLORF(SKY_400);
	}
}

SketchView::~SketchView()
{
	InterfaceRelease(&mp_screenBitmap);
	InterfaceRelease(&mp_dashStroke);
}

int SketchView::Create()
{
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

	//if (static_cast<unsigned char>(-1) / 2 > GetAverageBrightness(mh_screenBitmap) {
	//	m_colorSet.extentColor = RGB_TO_COLORF(NEUTRAL_100);
	//	m_colorSet.extentInvertColor = RGB_TO_COLORF(NEUTRAL_700);
	//}
	//else {
	//	m_colorSet.extentColor = RGB_TO_COLORF(NEUTRAL_700);
	//	m_colorSet.extentInvertColor = RGB_TO_COLORF(NEUTRAL_100);
	//}

	return S_OK;
}

void SketchView::Paint(const SKETCH::MD &a_modelData)
{
	static const auto DrawBorder = [](SketchView *const ap_view)
	{
		ap_view->SetStrokeWidth(4.0f);
		ap_view->SetBrushColor(ap_view->m_colorSet.highlight);
		ap_view->DrawRectangle(ap_view->m_viewRect);
		ap_view->SetStrokeWidth(1.0f);
	};
	static const auto DrawCurve = [](SketchView *const ap_view, SKETCH::CD a_curveData)
	{
		ID2D1PathGeometry *p_geometry;
		if (S_OK == gp_appCore->GetFactory()->CreatePathGeometry(&p_geometry)) {
			ID2D1GeometrySink *p_sink;
			if (S_OK == p_geometry->Open(&p_sink)) {
				p_sink->BeginFigure(a_curveData.points[0], D2D1_FIGURE_BEGIN_FILLED);

				size_t count = a_curveData.points.size();
				if (count > 1) {
					for (size_t i = 1; i < count; i++) {
						p_sink->AddLine(a_curveData.points[i]);
					}
				}

				p_sink->EndFigure(D2D1_FIGURE_END_OPEN);
				p_sink->Close();

				if (SKETCH::INVALID_INDEX != a_curveData.gradientBrushIndex) {
					// TODO:: gradient brush
				}
				else {
					DColor color = a_curveData.color;
					color.a = a_curveData.transparency;
					ap_view->SetBrushColor(color);
				}

				ap_view->SetStrokeWidth(static_cast<float>(a_curveData.strokeWidth));
				ap_view->DrawGeometry(p_geometry);

				p_sink->Release();
				p_geometry->Release();
			}
		}
	};

	///////////////////////////////////////////////////////////////////
	// implementation
	///////////////////////////////////////////////////////////////////

	if (nullptr != mp_screenBitmap) {
		DrawBitmap(mp_screenBitmap, m_viewRect);
	}
	DrawBorder(this);

	for (const auto &curveData : a_modelData.curveDataList) {
		DrawCurve(this, curveData);
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