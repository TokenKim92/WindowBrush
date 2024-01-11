#ifndef _COLOR_DIALOG_H_
#define _COLOR_DIALOG_H_

#include "WindowDialog.h"
#include "ColorModel.h"
#include <vector>

class ColorDialog : public WindowDialog
{
protected:
	CDM m_drawMode;
	CMD m_modelData;

	// variables for select mode
	const std::vector<DColor> &m_colorList;
	std::map<size_t, DRect> m_colorDataTable;		// key is a index
	std::pair<size_t, DRect> m_addButtonData;		// first data of pair is a index
	const DColor m_previousSelectedColor;
	size_t m_selectedColorIndex;

	// variables for add mode
	DPoint m_colorCenterPoint;
	bool isInitializedAddMode;
	std::map<CBT, DRect> m_buttonTable;

public:
	ColorDialog(const DColor &a_selectedColor, const std::vector<DColor> &a_colorList);
	virtual ~ColorDialog();

	DColor GetSelectedColor();
	std::vector<DColor> GetColorList();

protected:
	virtual void OnInitDialog() override;
	virtual void OnDestroy() override;
	virtual void OnPaint() override;

	// to handle the WM_MOUSEMOVE message that occurs when a window is destroyed
	int MouseMoveHandler(WPARAM a_wordParam, LPARAM a_longParam);
	// to handle the WM_LBUTTONDOWN message that occurs when a window is destroyed
	int MouseLeftButtonDownHandler(WPARAM a_wordParam, LPARAM a_longParam);
	// to handle the WM_LBUTTONUP message that occurs when a window is destroyed
	int MouseLeftButtonUpHandler(WPARAM a_wordParam, LPARAM a_longParam);
	// to handle the WM_KEYDOWN message that occurs when a window is destroyed
	int KeyDownHandler(WPARAM a_wordParam, LPARAM a_longParam);

private:
	void ChangeMode(const CDM &a_drawModw);
};

#endif //_COLOR_DIALOG_H_
