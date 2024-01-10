#ifndef _COLOR_VIEW_H_
#define _COLOR_VIEW_H_

#include "Direct2DEx.h"
#include "ColorModel.h"
#include "ColorSelectView.h"
#include <memory>
#include <vector>
#include <map>

class ColorView : public Direct2DEx
{
protected:
	ColorSelectView m_selectView;

	SIZE m_viewSize;
	DRect m_titleRect;
	DColor m_titleColor;
	DColor m_textBackgroundColor;
	DColor m_borderColor;
	const float m_defaultTransparency;

	IDWriteTextFormat *mp_titleFont;
	
	////////////////////////////////////////////////////////////////
	// variable for add mode
	////////////////////////////////////////////////////////////////
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
	ColorView(
		const HWND ah_window, const DColor &a_selectedColor, const std::vector<DColor> &a_colorList,
		const CM &a_mode, const RECT *const ap_viewRect = nullptr
	);
	virtual ~ColorView();

	virtual int Create() override;
	void Paint(const CDM &a_drawModw, const CMD &a_modelData);

	DColor GetColor(const size_t &a_index);
	const std::map<size_t, DRect> GetColorDataTable();
	const std::pair<size_t, DRect> &GetAddButtonData();
	const std::map<CBT, DRect> &GetButtonTable();

	void InitAddMode();
	void UpdateLightnessData(const DColor &a_hue);

protected:
	void PaintOnAddMode(const CMD &a_modelData);

	void DrawTitle(const CDM &a_mode);
	void DrawHueCircle();
	void DrawLightnessCircle();
};

#endif // !_COLOR_VIEW_H_
