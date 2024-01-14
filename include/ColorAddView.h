#ifndef _COLOR_ADD_VIEW_H_
#define _COLOR_ADD_VIEW_H_

#include "Direct2DEx.h"
#include "ColorModel.h"
#include <vector>
#include <map>
#include <memory>

class ColorAddView
{
protected:
	Direct2DEx *const mp_direct2d;

	SIZE m_viewSize;

	DColor m_mainColor;
	DColor m_oppositeColor;
	DColor m_currentLightness;
	DRect m_lightnessRect;
	DRect m_indicateRect;

	std::vector<std::pair<DColor, std::pair<DPoint, DPoint>>> m_hueDataList;
	std::vector<std::pair<DPoint, DPoint>> m_returnIconPoints;
	std::map<COLOR::BT, DRect> m_buttonTable;

	// interface
	ID2D1LinearGradientBrush *mp_lightnessGradientBrush;
	IDWriteTextFormat *mp_indicateFont;

	// memory interface
	IWICBitmap *mp_memoryBitmap;
	ID2D1RenderTarget *mp_memoryTarget;
	// values of memory bitmap to access color pixel
	std::unique_ptr<unsigned char[]> mp_memoryPattern;

public:
	ColorAddView(Direct2DEx *const ap_direct2d, const CM &a_mode);
	virtual ~ColorAddView();

	void Init(const DPoint &a_centerPoint, const SIZE &a_viewSize);
	void Paint(const COLOR::MD &a_modelData);
	void UpdateLightnessData(const DColor &a_hue);

	const std::map<COLOR::BT, DRect> &GetButtonTable();
	DColor GetPixelColorOnPoint(const DPoint &a_point);
	DColor &GetCurrentLightness();
};

#endif //!_COLOR_ADD_VIEW_H_