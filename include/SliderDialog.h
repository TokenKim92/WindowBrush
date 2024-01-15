#ifndef _SLIDER_DIALOG_H_
#define _SLIDER_DIALOG_H_

#include "WindowDialog.h"
#include "SliderModel.h"
#include <vector>
#include <map>

class SliderDialog : public WindowDialog
{
protected:
	SLIDER::MD m_modelData;

	const std::wstring m_title;
	const SLIDER::RD m_rangeData;
	const size_t m_ticInterval;
	const std::vector<std::wstring> m_ticIntervalTitle;

	std::map<SLIDER::BT, DRect> m_buttonTable;
	std::vector<DPoint> m_ticPoints;

public:
	SliderDialog(
		const std::wstring &a_title, const SLIDER::RD &a_rangeData, const size_t &a_tickInterval,
		const size_t &a_thumbIndex, const std::vector<std::wstring> &a_ticIntervalTitle
	);
	virtual ~SliderDialog();

	int GetValue();

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

#endif //_SLIDER_DIALOG_H_
