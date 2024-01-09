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

	typedef enum class BUTTON_TYPE
	{
		NONE,
		RETURN,
		HUE,
		LIGHTNESS,
		ADD
	}BT;

protected:
	const std::map<DM, void (ColorDialog:: *)()> m_drawTable;
	IDWriteTextFormat *mp_titleFont;
	ID2D1StrokeStyle *mp_addButtonStroke;
	DM m_drawMode;

	DRect m_textRect;
	DColor m_titleColor;
	DColor m_textBackgroundColor;
	DColor m_borderColor;
	const float m_defaultTransparency;

	// variables for select mode
	std::map<size_t, CD> m_colorDataTable;		// key is a index
	std::pair<size_t, CD> m_selectedColorData;	// first data of pair is a index
	std::pair<size_t, DRect> m_addButtonData;	// first data of pair is a index
	size_t m_hoverIndex;
	size_t m_clickedIndex;

	// variables for add mode
	bool isInitializedAddMode;
	std::vector<std::pair<DPoint, DPoint>> m_returnIconPoints;
	std::map<BT, DRect> m_buttonTable;
	BT m_hoverButton;
	BT m_clickedButton;

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
	void InitColorDataTable(const DColor &a_selectedColor, const std::vector<DColor> &a_colorList);
	void InitOnAddMode();
	void UpdateAddButtonRect();

	void DrawSelectMode();
	void DrawAddMode();

	void DrawTitle(const DM &a_mode);
	void DrawAddButton(const DM &a_mode);

	void ChangeToAddMode();
	void ChangeToSelectMode();
};

#endif //_COLOR_DIALOG_H_
