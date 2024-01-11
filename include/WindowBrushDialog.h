#ifndef _WINDOW_BRUSH_DIALOG_H_
#define _WINDOW_BRUSH_DIALOG_H_

#include "WindowDialog.h"
#include "WindowBrushModel.h"
#include <vector>

class WindowBrushDialog : public WindowDialog
{
protected:
	std::map<WBBT, DRect> m_buttonTable;
	WBMD m_modelData;

	std::vector<DColor> m_colorList;

public:
	WindowBrushDialog();
	virtual ~WindowBrushDialog();

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
};

#endif //_WINDOW_BRUSH_DIALOG_H_