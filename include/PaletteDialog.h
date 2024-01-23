#ifndef _PALETTE_DIALOG_H_
#define _PALETTE_DIALOG_H_

#include "WindowDialog.h"
#include "PaletteModel.h"
#include <vector>

class PaletteDialog : public WindowDialog
{
protected:
	PALETTE::DM m_drawMode;
	PALETTE::MD m_modelData;

	// variables for select mode
	const std::vector<DColor> &m_colorList;
	std::map<size_t, DRect> m_colorDataTable;	// key is a index
	std::pair<size_t, DRect> m_addButtonData;	// first data of pair is a index
	const DColor m_previousSelectedColor;
	size_t m_selectedColorIndex;

	// variables for add mode
	DPoint m_colorCenterPoint;
	bool isInitializedAddMode;
	std::map<PALETTE::BT, DRect> m_buttonTable;

public:
	PaletteDialog(const DColor &a_selectedColor, const std::vector<DColor> &a_colorList);
	virtual ~PaletteDialog() = default;

	DColor GetSelectedColor();
	std::vector<DColor> GetColorList();

protected:
	virtual void OnInitDialog() override;
	virtual void OnPaint() override;

	// to handle the WM_MOUSEMOVE message
	int MouseMoveHandler(WPARAM a_wordParam, LPARAM a_longParam);
	// to handle the WM_LBUTTONDOWN message
	int MouseLeftButtonDownHandler(WPARAM a_wordParam, LPARAM a_longParam);
	// to handle the WM_LBUTTONUP message
	int MouseLeftButtonUpHandler(WPARAM a_wordParam, LPARAM a_longParam);
	// to handle the WM_KEYDOWN message
	int KeyDownHandler(WPARAM a_wordParam, LPARAM a_longParam);

private:
	void ChangeMode(const PALETTE::DM &a_drawModw);
};

#endif //_PALETTE_DIALOG_H_
