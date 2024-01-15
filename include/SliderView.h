#ifndef _SLIDER_VIEW_H_
#define _SLIDER_VIEW_H_

#include "Direct2DEx.h"
#include "SliderModel.h"
#include <map>
#include <vector>

class SliderView : public Direct2DEx
{
protected:
	typedef struct COLOR_SET
	{
		DColor text;
		DColor lightBackground;
		DColor darkBackground;
		DColor saveButton;
		DColor channel;
		DColor thumb;
	}CS;

	typedef struct SLIDER_RECT
	{
		DRect minTitle;
		DRect maxTitle;
		DRect sldier;
	}SR;

protected:
	const std::wstring m_title;
	SLIDER::RD m_rangeData;
	const size_t m_ticInterval;
	const std::vector<std::wstring> m_ticIntervalTitle;
	std::vector<DPoint> m_ticPoints;

	std::map<SLIDER::BT, DRect> m_buttonTable;

	DRect m_titleRect;
	SR m_sliderRect;
	DRect m_buttonBackgroundRect;

	IDWriteTextFormat *mp_titleFont;
	IDWriteTextFormat *mp_textFont;

	CS m_colorSet;

public:
	SliderView(
		const HWND ah_window, const std::wstring &a_title, const SLIDER::RD a_rangeData,
		const size_t a_ticInterval, const std::vector<std::wstring> &a_ticIntervalTitle, const CM &a_mode, const RECT *const ap_viewRect = nullptr
	);
	virtual ~SliderView();

	virtual int Create() override;
	void Paint(const SLIDER::MD &a_modelData);

	const std::map<SLIDER::BT, DRect> &GetButtonTable();
	const std::vector<DPoint> &GetTicPoints();

protected:
	void DrawPlainText(const std::wstring &a_text, const DRect &a_rect, IDWriteTextFormat *const ap_textFormat);
};

#endif //!_EDIT_VIEW_H_