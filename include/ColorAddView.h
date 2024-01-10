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

	RECT m_viewRect;
	SIZE m_viewSize;

	DColor m_mainColor;
	DColor m_selectedHue;
	DColor m_selectedLightness;
	DRect m_lightnessRect;

	std::vector<std::pair<DColor, std::pair<DPoint, DPoint>>> m_hueDataList;
	std::vector<std::pair<DPoint, DPoint>> m_returnIconPoints;
	std::map<CBT, DRect> m_buttonTable;

	// interface
	ID2D1LinearGradientBrush *mp_lightnessGradientBrush;

	// memory interface
	IWICBitmap *mp_memoryBitmap;
	ID2D1RenderTarget *mp_memoryTarget;
	std::unique_ptr<unsigned char[]> mp_memoryPattern; // the first address of memory bitmap to access color pixel

public:
	ColorAddView(Direct2DEx *const ap_direct2d, const CM &a_mode);
	virtual ~ColorAddView();

	void Init(const RECT &a_viewRect, const SIZE &a_viewSize);
	void Paint(const CMD &a_modelData);
	void UpdateLightnessData(const DColor &a_hue);
	
	const std::map<CBT, DRect> &GetButtonTable();
};

#endif //!_COLOR_ADD_VIEW_H_