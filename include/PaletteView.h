#ifndef _PALETTE_VIEW_H_
#define _PALETTE_VIEW_H_

#include "PaletteSelectView.h"
#include "PaletteAddView.h"

class PaletteView : public Direct2DEx
{
protected:
	PaletteSelectView m_selectView;
	PaletteAddView m_addView;

	SIZE m_viewSize;

	IDWriteTextFormat *mp_titleFont;
	DRect m_titleRect;
	DColor m_titleColor;
	DColor m_textBackgroundColor;

	DColor m_selectedHue;

public:
	PaletteView(
		const HWND ah_window, const DColor &a_selectedColor, const std::vector<DColor> &a_colorList,
		const CM &a_mode, const RECT *const ap_viewRect = nullptr
	);
	virtual ~PaletteView();

	virtual int Create() override;
	void Paint(const PALETTE::DM &a_drawModw, const PALETTE::MD &a_modelData);

	void InitColorAddView(const DPoint &a_centerPoint);
	void UpdateLightnessCircle(const DPoint &a_point);

	void AddCurrentLightness();

	DColor GetColor(const size_t &a_index);
	std::vector<DColor> GetColorList();

	const std::map<size_t, DRect> GetColorDataTable();
	const std::pair<size_t, DRect> &GetAddButtonData();
	const std::map<PALETTE::BT, DRect> &GetButtonTable();

protected:
	void DrawTitle(const PALETTE::DM &a_mode);
};

#endif // !_PALETTE_VIEW_H_
