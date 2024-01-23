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

	std::map<EDIT::BT, DRect> m_buttonTable;
	std::map<size_t, std::pair< std::wstring, DRect>> m_editTable;

public:
	EditDialog(const std::wstring &a_title, const std::vector<std::pair<std::wstring, unsigned int>> &a_itemList, const EDIT::RANGE &a_range);
	virtual ~EditDialog() = default;

	const std::vector<unsigned int> &GetValueList();

protected:
	virtual void OnInitDialog() override;
	virtual void OnPaint() override;

	// to handle the WM_MOUSEMOVE message
	int MouseMoveHandler(WPARAM a_wordParam, LPARAM a_longParam);
	// to handle the WM_LBUTTONDOWN message
	int MouseLeftButtonDownHandler(WPARAM a_wordParam, LPARAM a_longParam);
	// to handle the WM_LBUTTONUP message
	int MouseLeftButtonUpHandler(WPARAM a_wordParam, LPARAM a_longParam);
	// to handle the WM_KEYDOWN message
	int KeyDownHandler(WPARAM a_wordParam, LPARAM a_longParam);

	void OnSave();
};

#endif //_EDIT_DIALOG_H_
