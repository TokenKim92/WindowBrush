#ifndef _HUE_D2D_H_
#define _HUE_D2D_H_

#include "Direct2DEx.h"
#include <memory>
#include <vector>

class ColorD2D : public Direct2DEx
{
protected:
	DColor m_selectedHue;
	DColor m_selectedLightness;
	DRect m_hueRect;
	DRect m_lightnessRect;
	std::vector<std::pair<DColor, std::pair<DPoint, DPoint>>> m_hueDataList;

	// interface
	ID2D1LinearGradientBrush *mp_lightnessGradientBrush;

	// memory interface
	IWICBitmap *mp_memoryBitmap;
	ID2D1RenderTarget *mp_memoryTarget;
	std::unique_ptr<unsigned char[]> mp_memoryPattern; // the first address of memory bitmap to access color pixel

public:
	ColorD2D(const HWND ah_window, const RECT *const ap_viewRect = nullptr);
	virtual ~ColorD2D();

	void InitHueData(const DRect &a_hueRect); 
	void UpdateHueData(const DColor &a_hue);

	void DrawHueCircle();
	void DrawLightnessCircle();
};

#endif // !_HUE_D2D_H_
