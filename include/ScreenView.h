#ifndef _SCREEN_VIEW_H_
#define _SCREEN_VIEW_H_

#include "Direct2DEx.h"
#include "ScreenModel.h"
#include <map>
#include <vector>

class ScreenView : public Direct2DEx
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
	std::map<SCREEN::BT, DRect> m_buttonTable;

	DRect m_titleRect;
	DRect m_buttonBackgroundRect;

	IDWriteTextFormat *mp_titleFont;
	IDWriteTextFormat *mp_textFont;

	CS m_colorSet;

public:
	ScreenView(const HWND ah_window, const std::wstring &a_title, const CM &a_mode, const RECT *const ap_viewRect = nullptr);
	virtual ~ScreenView();

	virtual int Create() override;
	void Paint(const SCREEN::MD &a_modelData);

	const std::map<SCREEN::BT, DRect> &GetButtonTable();

protected:
	void DrawPlainText(const std::wstring &a_text, const DRect &a_rect, IDWriteTextFormat *const ap_textFormat);
};

#endif //!_SCREEN_VIEW_H_