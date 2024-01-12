#ifndef _EDIT_VIEW_H_
#define _EDIT_VIEW_H_

#include "Direct2DEx.h"
#include "EditModel.h"
#include <map>
#include <vector>

class EditView : public Direct2DEx
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
	std::map<EDIT::BT, DRect> m_buttonTable;
	std::map<size_t, EDIT::ED> m_editTable;
	const EDIT::RANGE m_range;

	DRect m_titleRect;
	DRect m_warningRect;
	DRect m_buttonBackgroundRect;	

	IDWriteTextFormat *mp_titleFont;
	IDWriteTextFormat *mp_textFont;

	CS m_colorSet;

public:
	EditView(
		const HWND ah_window, const std::wstring &a_title, const std::vector<std::pair<std::wstring, unsigned int>> &a_itemList,
		const EDIT::RANGE &a_range, const CM &a_mode, const RECT *const ap_viewRect = nullptr
	);
	virtual ~EditView();

	virtual int Create() override;
	void Paint(const EDIT::MD &a_modelData);

protected:
	void DrawPlainText(const std::wstring &a_text, const DRect &a_rect, IDWriteTextFormat *const ap_textFormat);
};

#endif //!_EDIT_VIEW_H_