#include "EditDialog.h"
#include "EditView.h"
#include "Utility.h"

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
	SetStyle(WS_POPUP | WS_VISIBLE);
	SetExtendStyle(WS_EX_TOPMOST);

	m_modelData = {
		EDIT::BT::NONE, EDIT::BT::NONE,
		EDIT::INVALID_INDEX, EDIT::INVALID_INDEX
	};

	for (const auto &item : m_itemList) {
		m_modelData.valueList.push_back(item.second);
	}
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

	const auto p_view = new EditView(mh_window, L"Stroke Width", m_itemList, m_range, GetColorMode());
	InheritDirect2D(p_view);
	p_view->Create();
	m_buttonTable = p_view->GetButtonTable();
	m_editTable = p_view->GetEditTable();
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

	for (const auto &[type, rect] : m_buttonTable) {
		if (PointInRect(rect, point)) {
			if (type != m_modelData.hoverButtonType) {
				m_modelData.hoverButtonType = type;
				Invalidate();
			}

			return S_OK;
		}
	}

	for (const auto &[index, editData] : m_editTable) {
		if (PointInRect(editData.second, point)) {
			m_modelData.hoverButtonType = EDIT::BT::EDIT;

			if (index != m_modelData.hoverEditIndex) {
				m_modelData.hoverEditIndex = index;
				Invalidate();
			}

			return S_OK;
		}
	}

	if (EDIT::BT::NONE != m_modelData.hoverButtonType) {
		m_modelData.hoverButtonType = EDIT::BT::NONE;
		m_modelData.hoverEditIndex = EDIT::INVALID_INDEX;
		Invalidate();
	}

	return S_OK;
}

// to handle the WM_LBUTTONDOWN message that occurs when a window is destroyed
int EditDialog::MouseLeftButtonDownHandler(WPARAM a_wordParam, LPARAM a_longParam)
{
	::SetCapture(mh_window);

	const POINT point = { LOWORD(a_longParam), HIWORD(a_longParam) };

	for (auto const &[type, rect] : m_buttonTable) {
		if (PointInRect(rect, point)) {
			m_modelData.clickedButtonType = type;
			Invalidate();

			return S_OK;
		}
	}

	for (const auto &[index, editData] : m_editTable) {
		if (PointInRect(editData.second, point)) {
			m_modelData.clickedButtonType = EDIT::BT::EDIT;
			m_modelData.clickedEditIndex = index;
			Invalidate();

			return S_OK;
		}
	}

	if (EDIT::BT::NONE != m_modelData.clickedButtonType) {
		m_modelData.clickedButtonType = EDIT::BT::NONE;
		m_modelData.clickedEditIndex = EDIT::INVALID_INDEX;
		Invalidate();
	}

	return S_OK;
}

// to handle the WM_LBUTTONUP message that occurs when a window is destroyed
int EditDialog::MouseLeftButtonUpHandler(WPARAM a_wordParam, LPARAM a_longParam)
{
	static const auto OnSaveButtonUp = [](
		EditDialog *const ap_dialog, EDIT::MD &a_modelData)
	{
		if (EDIT::BT::SAVE == a_modelData.hoverButtonType) {
			ap_dialog->OnSave();
			return;
		}

		a_modelData.clickedButtonType = EDIT::BT::NONE;
	};
	static const auto OnCancelButtonUp = [](EditDialog *const ap_dialog, const HWND &ah_window, EDIT::MD &a_modelData)
	{
		if (EDIT::BT::CANCEL == a_modelData.hoverButtonType) {
			::DestroyWindow(ah_window);
			return;
		}

		a_modelData.clickedButtonType = EDIT::BT::NONE;
	};

	////////////////////////////////////////////////////////////////
	// implementation
	////////////////////////////////////////////////////////////////

	::ReleaseCapture();

	const POINT point = { LOWORD(a_longParam), HIWORD(a_longParam) };

	if (EDIT::BT::NONE != m_modelData.clickedButtonType) {
		switch (m_modelData.clickedButtonType)
		{
		case EDIT::BT::SAVE:
			OnSaveButtonUp(this, m_modelData);
			break;
		case EDIT::BT::CANCEL:
			OnCancelButtonUp(this, mh_window, m_modelData);
			break;
		case EDIT::BT::EDIT:
			break;
		default:
			break;
		}
	}

	return S_OK;
}

// to handle the WM_KEYDOWN message that occurs when a window is destroyed
int EditDialog::KeyDownHandler(WPARAM a_wordParam, LPARAM a_longParam)
{
	static const auto OnNumberKeyDown = [](
		EditDialog *const ap_dialog, const unsigned char a_pressedKey, const unsigned char a_offset, EDIT::MD &a_modelData
		)
	{
		auto &tempValue = a_modelData.valueList[a_modelData.clickedEditIndex];
		std::string previousValue = std::to_string(tempValue);

		if (previousValue.length() < EDIT::MAX_DIGIT_LEGNHT) {
			std::string currentNumber = std::to_string(tempValue) + std::to_string(a_pressedKey - a_offset);
			tempValue = std::stoi(currentNumber);

			ap_dialog->Invalidate();
		}
	};
	static const auto OnBackKeyDown = [](EditDialog *const ap_dialog, EDIT::MD &a_modelData)
	{
		auto &tempValue = a_modelData.valueList[a_modelData.clickedEditIndex];
		if (0 != tempValue) {
			std::string numberText = std::to_string(tempValue);
			numberText.pop_back();
			tempValue = numberText.empty() ? 0 : std::stoi(numberText);

			ap_dialog->Invalidate();
		}
	};

	////////////////////////////////////////////////////////////////
	// implementation
	////////////////////////////////////////////////////////////////

	static const unsigned char number0 = 0x30;
	static const unsigned char number9 = 0x39;
	const unsigned char pressedKey = static_cast<unsigned char>(a_wordParam);

	if (VK_RETURN == pressedKey) {
		OnSave();
	}
	else if (VK_ESCAPE == pressedKey) {
		::DestroyWindow(mh_window);
	}

	if (EDIT::BT::EDIT == m_modelData.clickedButtonType) {
		if (number0 <= pressedKey && pressedKey <= number9) {
			OnNumberKeyDown(this, pressedKey, number0, m_modelData);
		}
		else if ((VK_NUMPAD0 <= pressedKey && pressedKey <= VK_NUMPAD9)) {
			OnNumberKeyDown(this, pressedKey, VK_NUMPAD0, m_modelData);
		}
		else if (VK_BACK == pressedKey) {
			OnBackKeyDown(this, m_modelData);
		}
	}

	return S_OK;
}

const std::vector<unsigned int> &EditDialog::GetValueList()
{
	return m_modelData.valueList;
}

void EditDialog::OnSave()
{
	bool isValid = true;
	for (const auto &value : m_modelData.valueList) {
		if (m_range.min > value || m_range.max < value) {
			isValid = false;
			break;
		}
	}

	if (isValid) {
		BT type = BT::OK;
		SetClickedButtonType(type);
		::DestroyWindow(mh_window);
	}
}