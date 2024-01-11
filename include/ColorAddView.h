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
	DRect m_lightnessRect;

	std::vector<std::pair<DColor, std::pair<DPoint, DPoint>>> m_hueDataList;
	std::vector<std::pair<DPoint, DPoint>> m_returnIconPoints;
	std::map<CBT, DRect> m_buttonTable;

	// interface
	ID2D1LinearGradientBrush *mp_lightnessGradientBrush;

	// memory interface
	IWICBitmap *mp_memoryBitmap;
	ID2D1RenderTarget *mp_memoryTarget;
	// values of memory bitmap to access color pixel
	unsigned char m_memoryPattern[COLOR_DIALOG_WIDTH * COLOR_DIALOG_HEGIHT * sizeof(unsigned int)];

public:
	ColorAddView(Direct2DEx *const ap_direct2d, const CM &a_mode);
	virtual ~ColorAddView();

	void Init(const DPoint &a_centerPoint, const SIZE &a_viewSize);
	void Paint(const CMD &a_modelData);
	void UpdateLightnessData(const DColor &a_hue);
	
	const std::map<CBT, DRect> &GetButtonTable();
	DColor GetPixelColorOnPoint(const DPoint &a_point);
};

#endif //!_COLOR_ADD_VIEW_H_