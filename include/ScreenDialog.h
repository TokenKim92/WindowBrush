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

	const std::wstring m_title;

	std::map<SCREEN::BT, DRect> m_buttonTable;

public:
	ScreenDialog(const std::wstring &a_title);
	virtual ~ScreenDialog();

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
};

#endif //_SCREEN_DIALOG_H_
