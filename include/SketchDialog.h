#ifndef _SKETCH_DIALOG_H_
#define _SKETCH_DIALOG_H_

#include "WindowDialog.h"
#include "WindowBrushModel.h"
#include "SketchModel.h"
#include <vector>

class SketchDialog : public WindowDialog
{
protected:
	const HWND mh_parentWindow;
	WINDOW_BRUSH::MD m_parentModelData;
	std::vector<SKETCH::MD> m_modelDataList;

	bool m_leftButtonDown;
	unsigned __int64 m_previousMilliseconds;

	const RECT m_scaledRect;
	HBITMAP mh_screenBitmap;
	HWND mh_edit;

public:
	SketchDialog(const HWND &ah_parentWindow, const WINDOW_BRUSH::MD &a_modelData, const RECT &a_scaledRect);
	virtual ~SketchDialog();

	void UpdateWindowBrushModelData(const WINDOW_BRUSH::MD *a_modelData);

protected:
	virtual void OnInitDialog() override;
	virtual void OnDestroy() override;
	virtual void OnQuit() override;
	virtual void OnPaint() override;
	virtual void PreTranslateMessage(MSG &a_msg) override;

	// to handle the WM_MOUSEMOVE message
	int MouseMoveHandler(WPARAM a_wordParam, LPARAM a_longParam);
	// to handle the WM_LBUTTONDOWN message
	int MouseLeftButtonDownHandler(WPARAM a_wordParam, LPARAM a_longParam);
	// to handle the WM_LBUTTONUP message
	int MouseLeftButtonUpHandler(WPARAM a_wordParam, LPARAM a_longParam);
	// to handle the SKETCH::WM_UPDATE_MODEL_DATA
	int UpdateModelDataHandler(WPARAM a_wordParam, LPARAM a_longParam);
	// to handle the SKETCH::WM_SET_TEXTOUTLINE_MODE
	int SetTextOutlineModeHandler(WPARAM a_wordParam, LPARAM a_longParam);
	// to handle the SKETCH::WM_ON_EDIT_MAX_LEGNTH
	int OnEditMaxLengthHandler(WPARAM a_wordParam, LPARAM a_longParam);

private:
	void FadeObject(const bool isOnTimer = true);
	static void __stdcall FadeObjectOnTimer(HWND ah_wnd, UINT a_msg, UINT_PTR ap_data, DWORD a_isMouseMoving);
};

#endif //_SKETCH_DIALOG_H_
