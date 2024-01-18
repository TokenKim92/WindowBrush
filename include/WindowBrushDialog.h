#ifndef _WINDOW_BRUSH_DIALOG_H_
#define _WINDOW_BRUSH_DIALOG_H_

#include "WindowDialog.h"
#include "WindowBrushModel.h"
#include "SketchDialog.h"
#include <vector>

class WindowBrushDialog : public WindowDialog
{
protected:
	std::map<WINDOW_BRUSH::BT, DRect> m_buttonTable;
	WINDOW_BRUSH::MD m_modelData;

	std::vector<DColor> m_colorList;
	std::vector<RECT> m_physicalScreenRects;

	WINDOW_BRUSH::IDD m_infoDialogData;
	SketchDialog *mp_sketchDialog;;

	bool m_isLeftMouse;

public:
	WindowBrushDialog();
	virtual ~WindowBrushDialog() = default;

	std::wstring GetHoverButtonTitle();
	POINT GetInfoDialogPoint();

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
	// to handle the WM_MOUSELEAVE  message that occurs when a window is destroyed
	int MouseLeaveHandler(WPARAM a_wordParam, LPARAM a_longParam);

	// to handle the WM_SYSCOMMAND message that occurs when a window is created
	virtual msg_handler int SysCommandHandler(WPARAM a_menuID, LPARAM a_longParam) override;

protected:
	void KillInfoDialogTimer();
};

#endif //_WINDOW_BRUSH_DIALOG_H_