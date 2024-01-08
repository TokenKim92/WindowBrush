#ifndef _COLOR_DIALOG_H_
#define _COLOR_DIALOG_H_

#include "WindowDialog.h"
#include <vector>

class ColorDialog : public WindowDialog
{
protected:
	typedef enum class DRAW_MODE
	{
		SELECT,
		ADD
	}DM;

	typedef struct COLOR_DATA
	{
		DColor color;
		DRect rect;
	} CD;

protected:
	// first data of pair is a index
	std::map<size_t, CD> m_colorDataTable;
	std::pair<size_t, CD> m_selectedColorData;
	std::pair<size_t, DRect> m_addButtonData; 

	const std::map<DM, void (ColorDialog:: *)()> m_drawTable;
	DM m_drawMode;
	size_t m_hoverIndex;
	size_t m_clickedIndex;

	IDWriteTextFormat *mp_titleFont;
	ID2D1StrokeStyle *mp_addButtonStroke;

	DRect m_textRect;
	DColor m_titleColor;
	DColor m_textBackgroundColor;
	DColor m_borderColor;
	const float m_defaultTransparency;

public:
	ColorDialog(const DColor &a_selectedColor, const std::vector<DColor> &a_colorList);
	virtual ~ColorDialog();

	DColor GetSelectedColor();

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
	void InitColorDataList(const DColor &a_selectedColor, const std::vector<DColor> &a_colorList);
	void UpdateAddButtonRect();
	
	void DrawSelectMode();
	void DrawAddMode();
	void DrawTitle(const DM &a_mode);
	void DrawAddButton(const DM &a_mode);
	void ChangeToAddMode();
};

#endif //_COLOR_DIALOG_H_
