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
		DColor background;
		DColor darkBackground;
		DColor highlight;
		DColor shadow;
	}CS;

protected:
	const std::wstring m_title;
	std::map<SLIDER::BT, DRect> m_buttonTable;

	DRect m_titleRect;
	DRect m_buttonBackgroundRect;

	IDWriteTextFormat *mp_titleFont;
	IDWriteTextFormat *mp_textFont;

	CS m_colorSet;

public:
	SliderView(const HWND ah_window, const std::wstring &a_title, const CM &a_mode, const RECT *const ap_viewRect = nullptr);
	virtual ~SliderView();

	virtual int Create() override;
	void Paint(const SLIDER::MD &a_modelData);

	const std::map<SLIDER::BT, DRect> &GetButtonTable();

protected:
	void DrawPlainText(const std::wstring &a_text, const DRect &a_rect, IDWriteTextFormat *const ap_textFormat);
};

#endif //!_EDIT_VIEW_H_