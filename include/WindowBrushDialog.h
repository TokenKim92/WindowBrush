#ifndef _WINDOW_BRUSH_DIALOG_H_
#define _WINDOW_BRUSH_DIALOG_H_

#include "WindowDialog.h"
#include "ButtonShape.h"
#include <memory>

class WindowBrush : public WindowDialog
{
protected:
	std::map<BST, DRect> m_buttonTable;
	std::vector<DRect> m_dividerList;
	std::unique_ptr<ButtonShape> mp_buttonsShape;

	BSD m_buttonShapeData;;
	DColor m_selectedColor;

public:
	WindowBrush();
	virtual ~WindowBrush();

private:
	void InitButtonRects();
	void InitDivider();

protected:
	virtual void OnInitDialog() override;
	virtual void OnDestroy() override;
	virtual void OnPaint() override;
	virtual void OnSetThemeMode() override;

	// to handle the WM_MOUSEMOVE message that occurs when a window is destroyed
	int MouseMoveHandler(WPARAM a_wordParam, LPARAM a_longParam);
	// to handle the WM_LBUTTONDOWN  message that occurs when a window is destroyed
	int MouseLeftButtonDownHandler(WPARAM a_wordParam, LPARAM a_longParam);
	// to handle the WM_LBUTTONUP  message that occurs when a window is destroyed
	int MouseLeftButtonUpHandler(WPARAM a_wordParam, LPARAM a_longParam);
	// to handle the WM_MOUSEWHEEL  message that occurs when a window is destroyed
	int MouseWheelHandler(WPARAM a_wordParam, LPARAM a_longParam);
};

#endif //_WINDOW_BRUSH_DIALOG_H_