#ifndef _PALETTE_ADD_VIEW_H_
#define _PALETTE_ADD_VIEW_H_

#include "Direct2DEx.h"
#include "PaletteModel.h"
#include <vector>
#include <map>
#include <memory>

class PaletteAddView
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
	std::map<PALETTE::BT, DRect> m_buttonTable;

	// interface
	ID2D1LinearGradientBrush *mp_lightnessGradientBrush;
	IDWriteTextFormat *mp_indicateFont;

	// memory interface
	IWICBitmap *mp_memoryBitmap;
	ID2D1RenderTarget *mp_memoryTarget;
	// values of memory bitmap to access color pixel
	std::unique_ptr<unsigned char[]> mp_memoryPattern;

public:
	PaletteAddView(Direct2DEx *const ap_direct2d, const CM &a_mode);
	virtual ~PaletteAddView();

	void Init(const HWND &ah_wnd, const DPoint &a_centerPoint, const SIZE &a_viewSize);
	void Paint(const PALETTE::MD &a_modelData);
	void UpdateLightnessData(const DColor &a_hue);

	const std::map<PALETTE::BT, DRect> &GetButtonTable();
	DColor GetPixelColorOnPoint(const DPoint &a_point);
	DColor &GetCurrentLightness();
};

#endif //!_PALETTE_ADD_VIEW_H_