#include "EditDialog.h"
#include "EditView.h"

EditDialog::EditDialog(
	const std::wstring &a_title, const std::vector<std::pair<std::wstring, unsigned int>> &a_itemList, const EDIT::RANGE &a_range
) :
	WindowDialog(L"EDITDIALOG", L"EditDialog"),
	m_title(a_title),
	m_itemList(a_itemList),
	m_range(a_range)
{
	const unsigned int width = 1 == m_itemList.size()
		? EDIT::DIALOG_WIDTH
		: static_cast<unsigned int>(EDIT::EDIT_MARGIN + (EDIT::EDIT_WIDTH + EDIT::EDIT_MARGIN) * m_itemList.size());

	SetSize(width, EDIT::DIALOG_HEIGHT);
}

EditDialog::~EditDialog()
{

}

void EditDialog::OnInitDialog()
{
	DisableMaximize();
	DisableMinimize();
	DisableSize();

	// add message handlers
	AddMessageHandler(WM_MOUSEMOVE, static_cast<MessageHandler>(&EditDialog::MouseMoveHandler));
	AddMessageHandler(WM_LBUTTONDOWN, static_cast<MessageHandler>(&EditDialog::MouseLeftButtonDownHandler));
	AddMessageHandler(WM_LBUTTONUP, static_cast<MessageHandler>(&EditDialog::MouseLeftButtonUpHandler));
	AddMessageHandler(WM_KEYDOWN, static_cast<MessageHandler>(&EditDialog::KeyDownHandler));

	const auto p_view = new EditView(mh_window, L"Stroke Width", m_itemList, m_range, GetThemeMode());
	InheritDirect2D(p_view);
	p_view->Create();
}

void EditDialog::OnDestroy()
{

}

void EditDialog::OnPaint()
{
	mp_direct2d->Clear();
	static_cast<EditView *>(mp_direct2d)->Paint(m_modelData);
}

// to handle the WM_MOUSEMOVE message that occurs when a window is destroyed
int EditDialog::MouseMoveHandler(WPARAM a_wordParam, LPARAM a_longParam)
{
	const POINT point = { LOWORD(a_longParam), HIWORD(a_longParam) };

	return S_OK;
}

// to handle the WM_LBUTTONDOWN message that occurs when a window is destroyed
int EditDialog::MouseLeftButtonDownHandler(WPARAM a_wordParam, LPARAM a_longParam)
{
	const POINT point = { LOWORD(a_longParam), HIWORD(a_longParam) };

	return S_OK;
}

// to handle the WM_LBUTTONUP message that occurs when a window is destroyed
int EditDialog::MouseLeftButtonUpHandler(WPARAM a_wordParam, LPARAM a_longParam)
{
	const POINT point = { LOWORD(a_longParam), HIWORD(a_longParam) };

	return S_OK;
}

// to handle the WM_KEYDOWN message that occurs when a window is destroyed
int EditDialog::KeyDownHandler(WPARAM a_wordParam, LPARAM a_longParam)
{
	const unsigned char pressedKey = static_cast<unsigned char>(a_wordParam);

	if (VK_ESCAPE == pressedKey) {
		::DestroyWindow(mh_window);
	}

	return S_OK;
}