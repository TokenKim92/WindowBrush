#ifndef _COLOR_VIEW_H_
#define _COLOR_VIEW_H_

#include "ColorSelectView.h"
#include "ColorAddView.h"

class ColorView : public Direct2DEx
{
protected:
	ColorSelectView m_selectView;
	ColorAddView m_addView;

	SIZE m_viewSize;

	IDWriteTextFormat *mp_titleFont;
	DRect m_titleRect;
	DColor m_titleColor;
	DColor m_textBackgroundColor;

public:
	ColorView(
		const HWND ah_window, const DColor &a_selectedColor, const std::vector<DColor> &a_colorList,
		const CM &a_mode, const RECT *const ap_viewRect = nullptr
	);
	virtual ~ColorView();

	virtual int Create() override;
	void Paint(const CDM &a_drawModw, const CMD &a_modelData);

	void InitColorAddView();

	DColor GetColor(const size_t &a_index);
	const std::map<size_t, DRect> GetColorDataTable();
	const std::pair<size_t, DRect> &GetAddButtonData();
	const std::map<CBT, DRect> &GetButtonTable();

protected:
	void DrawTitle(const CDM &a_mode);
};

#endif // !_COLOR_VIEW_H_
