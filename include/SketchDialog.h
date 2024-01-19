#ifndef _SKETCH_DIALOG_H_
#define _SKETCH_DIALOG_H_

#include "WindowDialog.h"
#include "WindowBrushModel.h"
#include "SketchModel.h"
#include <vector>

class SketchDialog : public WindowDialog
{
protected:
	WINDOW_BRUSH::MD m_parentModelData;
	std::vector<SKETCH::MD> m_modelDataList;

	bool m_leftButtonDown;
	unsigned __int64 m_previousMilliseconds;

	const RECT m_scaledRect;
	HBITMAP mh_screenBitmap;
	HWND mh_edit;

public:
	SketchDialog(const WINDOW_BRUSH::MD &a_modelData, const RECT &a_scaledRect);
	virtual ~SketchDialog();

	void UpdateWindowBrushModelData(const WINDOW_BRUSH::MD *a_modelData);

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
	// to handle the WM_UPDATEMD message that occurs when a window is destroyed
	int UpdateModelDataHandler(WPARAM a_wordParam, LPARAM a_longParam);
};

#endif //_SKETCH_DIALOG_H_
