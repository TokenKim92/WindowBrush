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
	bool m_isHiddenMode;

public:
	WindowBrushDialog();
	virtual ~WindowBrushDialog() = default;

	std::wstring GetHoverButtonTitle();
	POINT GetInfoDialogPoint();

protected:
	virtual void OnInitDialog() override;
	virtual void OnDestroy() override;
	virtual void OnPaint() override;
	virtual void OnSetColorMode() override;

	// to handle the WM_MOUSEMOVE message
	int MouseMoveHandler(WPARAM a_wordParam, LPARAM a_longParam);
	// to handle the WM_LBUTTONDOWN  message
	int MouseLeftButtonDownHandler(WPARAM a_wordParam, LPARAM a_longParam);
	// to handle the WM_LBUTTONUP  message
	int MouseLeftButtonUpHandler(WPARAM a_wordParam, LPARAM a_longParam);
	// to handle the WM_MOUSELEAVE  message 
	int MouseLeaveHandler(WPARAM a_wordParam, LPARAM a_longParam);
	// to handle the WM_KEYDOWN
	int KeyDownHandler(WPARAM a_wordParam, LPARAM a_longParam);

	// to handle the WM_SYSCOMMAND message that occurs when a window is created
	virtual msg_handler int SysCommandHandler(WPARAM a_menuID, LPARAM a_longParam) override;

protected:
	void KillInfoDialogTimer();
	int KilledSketchDialogHandler(WPARAM a_wordParam, LPARAM a_longParam);

	void OnDrawButtonUp(const WINDOW_BRUSH::DT &a_type);
	void OnCurveButtonUp();
	void OnRectangleButtonUp();
	void OnEllipseButtonUp();
	void OnTextButtonUp();
	void OnStrokeButtonUp();
	void OnColorButtonUp();
	void OnGradientButtonUp();
	void OnFadeButtonUp();

	void OnClickSelectScreenMenu();
	void OnClickColorOpacityMenu();
	void OnClickFadeSpeedMenu();
	void OnClickHiddenMenu();
};

#endif //_WINDOW_BRUSH_DIALOG_H_