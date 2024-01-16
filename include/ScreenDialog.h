#ifndef _SCREEN_DIALOG_H_
#define _SCREEN_DIALOG_H_

#include "WindowDialog.h"
#include "ScreenModel.h"
#include <vector>
#include <map>

class ScreenDialog : public WindowDialog
{
protected:
	SCREEN::MD m_modelData;
	std::vector<RECT> m_physicalScreenRects;

	std::map<SCREEN::BT, DRect> m_buttonTable;
	std::map<size_t, DRect> m_screenTable;

public:
	ScreenDialog(const RECT &a_selectedScreenRect);
	virtual ~ScreenDialog() = default;

	RECT GetSelectedRect();

protected:
	virtual void OnInitDialog() override;
	virtual void OnPaint() override;

	// to handle the WM_MOUSEMOVE message that occurs when a window is destroyed
	int MouseMoveHandler(WPARAM a_wordParam, LPARAM a_longParam);
	// to handle the WM_LBUTTONDOWN message that occurs when a window is destroyed
	int MouseLeftButtonDownHandler(WPARAM a_wordParam, LPARAM a_longParam);
	// to handle the WM_LBUTTONUP message that occurs when a window is destroyed
	int MouseLeftButtonUpHandler(WPARAM a_wordParam, LPARAM a_longParam);
	// to handle the WM_KEYDOWN message that occurs when a window is destroyed
	int KeyDownHandler(WPARAM a_wordParam, LPARAM a_longParam);
};

#endif //_SCREEN_DIALOG_H_
