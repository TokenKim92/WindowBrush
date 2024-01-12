#ifndef _EDIT_DIALOG_H_
#define _EDIT_DIALOG_H_

#include "WindowDialog.h"
#include "EditModel.h"
#include <vector>

class EditDialog : public WindowDialog
{
protected:
	EDIT::MD m_modelData;

	const std::wstring m_title;
	const std::vector<std::pair<std::wstring, unsigned int>> m_itemList; // subtitle and value
	const EDIT::RANGE m_range;

public:
	EditDialog(const std::wstring &a_title, const std::vector<std::pair<std::wstring, unsigned int>> &a_itemList, const EDIT::RANGE &a_range);
	virtual ~EditDialog();

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

#endif //_EDIT_DIALOG_H_
