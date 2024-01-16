#include "ScreenDialog.h"
#include "ScreenView.h"
#include "Utility.h"

ScreenDialog::ScreenDialog(const RECT &a_selectedScreenRect) :
	WindowDialog(L"SCREENDIALOG", L"ScreenDialog")
{
	const auto GetDialogHieght = [](const std::vector<RECT> &a_physicalScreenRects)
	{
		const auto screenButtonWidth = SCREEN::DIALOG_WIDTH - SCREEN::SCREEN_X_MARGIN * 2.0f;
		int dialogHeight = static_cast<int>(SCREEN::TITLE_HEIGHT + SCREEN::BUTTON_HEIGHT + SCREEN::BUTTON_MARGIN * 2);
		double ratio;

		for (const auto &rect : a_physicalScreenRects) {
			ratio = static_cast<double>(rect.bottom - rect.top) / static_cast<double>(rect.right - rect.left);
			dialogHeight += static_cast<int>(screenButtonWidth * ratio + SCREEN::SCREEN_Y_MARGIN);
		}

		return dialogHeight;
	};
	const auto GetIndexFromSelectedScreen = [](const RECT &a_selectedScreenRect, const std::vector<RECT> &a_physicalScreenRects)
	{
		size_t index = 0;
		for (const auto &rect : a_physicalScreenRects) {
			if (rect.left == a_selectedScreenRect.left && rect.top == a_selectedScreenRect.top &&
				rect.right == a_selectedScreenRect.right && rect.bottom == a_selectedScreenRect.bottom) {
				return index;
			}

			index++;
		}

		return SCREEN::INVALID_INDEX;
	};

	////////////////////////////////////////////////////////////////
	// implementation
	////////////////////////////////////////////////////////////////
	
	::EnumDisplayMonitors(nullptr, nullptr, GetPhysicalScreenRects, reinterpret_cast<LPARAM>(&m_physicalScreenRects));

	SetSize(SCREEN::DIALOG_WIDTH, GetDialogHieght(m_physicalScreenRects));
	SetStyle(WS_POPUP | WS_VISIBLE);
	SetExtendStyle(WS_EX_TOPMOST);

	m_modelData = { 
		SCREEN::BT::NONE, SCREEN::BT::NONE,
		SCREEN::INVALID_INDEX, GetIndexFromSelectedScreen(a_selectedScreenRect, m_physicalScreenRects)
	};
}

void ScreenDialog::OnInitDialog()
{
	DisableMaximize();
	DisableMinimize();
	DisableSize();

	// add message handlers
	AddMessageHandler(WM_MOUSEMOVE, static_cast<MessageHandler>(&ScreenDialog::MouseMoveHandler));
	AddMessageHandler(WM_LBUTTONDOWN, static_cast<MessageHandler>(&ScreenDialog::MouseLeftButtonDownHandler));
	AddMessageHandler(WM_LBUTTONUP, static_cast<MessageHandler>(&ScreenDialog::MouseLeftButtonUpHandler));
	AddMessageHandler(WM_KEYDOWN, static_cast<MessageHandler>(&ScreenDialog::KeyDownHandler));

	const auto p_view = new ScreenView(mh_window, GetColorMode());
	InheritDirect2D(p_view);
	p_view->Create();
	m_buttonTable = p_view->GetButtonTable();
	m_screenTable = p_view->GetScreenTable();
}

void ScreenDialog::OnPaint()
{
	mp_direct2d->Clear();
	static_cast<ScreenView *>(mp_direct2d)->Paint(m_modelData);
}

// to handle the WM_MOUSEMOVE message that occurs when a window is destroyed
int ScreenDialog::MouseMoveHandler(WPARAM a_wordParam, LPARAM a_longParam)
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

	for (const auto &[index, rect] : m_screenTable) {
		if (PointInRect(rect, point)) {
			m_modelData.hoverButtonType = SCREEN::BT::SCREEN;

			if (index != m_modelData.hoverScreenIndex) {
				m_modelData.hoverScreenIndex = index;
				Invalidate();
			}

			return S_OK;
		}
	}

	if (SCREEN::BT::NONE != m_modelData.hoverButtonType) {
		m_modelData.hoverButtonType = SCREEN::BT::NONE;
		m_modelData.hoverScreenIndex = SCREEN::INVALID_INDEX;
		Invalidate();
	}

	return S_OK;
}

// to handle the WM_LBUTTONDOWN message that occurs when a window is destroyed
int ScreenDialog::MouseLeftButtonDownHandler(WPARAM a_wordParam, LPARAM a_longParam)
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

	for (const auto &[index, rect] : m_screenTable) {
		if (PointInRect(rect, point)) {
			m_modelData.clickedButtonType = SCREEN::BT::SCREEN;

			if (m_modelData.clickedScreenIndex != index) {
				m_modelData.clickedScreenIndex = index;
				Invalidate();
			}

			return S_OK;
		}
	}

	if (SCREEN::BT::NONE != m_modelData.clickedButtonType) {
		m_modelData.clickedButtonType = SCREEN::BT::NONE;
		Invalidate();
	}

	return S_OK;
}

// to handle the WM_LBUTTONUP message that occurs when a window is destroyed
int ScreenDialog::MouseLeftButtonUpHandler(WPARAM a_wordParam, LPARAM a_longParam)
{
	static const auto OnButtonUp = [](ScreenDialog *const ap_dialog, const HWND &ah_window, const SCREEN::BT &a_type, SCREEN::MD &a_modelData)
	{
		if (a_type == a_modelData.hoverButtonType) {
			if (SCREEN::BT::SAVE == a_type) {
				BT type = BT::OK;
				ap_dialog->SetClickedButtonType(type);
			}

			::DestroyWindow(ah_window);
			return;
		}

		a_modelData.clickedButtonType = SCREEN::BT::NONE;
	};

	////////////////////////////////////////////////////////////////
	// implementation
	////////////////////////////////////////////////////////////////

	::ReleaseCapture();

	const POINT point = { LOWORD(a_longParam), HIWORD(a_longParam) };

	if (SCREEN::BT::NONE != m_modelData.clickedButtonType) {
		switch (m_modelData.clickedButtonType)
		{
		case SCREEN::BT::SAVE:
			OnButtonUp(this, mh_window, SCREEN::BT::SAVE, m_modelData);
			break;
		case SCREEN::BT::CANCEL:
			OnButtonUp(this, mh_window, SCREEN::BT::CANCEL, m_modelData);
			break;
		default:
			break;
		}
	}

	return S_OK;
}

// to handle the WM_KEYDOWN message that occurs when a window is destroyed
int ScreenDialog::KeyDownHandler(WPARAM a_wordParam, LPARAM a_longParam)
{
	const unsigned char pressedKey = static_cast<unsigned char>(a_wordParam);

	if (VK_RETURN == pressedKey) {
		BT type = BT::OK;
		SetClickedButtonType(type);
		::DestroyWindow(mh_window);
	}
	else if (VK_ESCAPE == pressedKey) {
		::DestroyWindow(mh_window);
	}

	return S_OK;
}

RECT ScreenDialog::GetSelectedRect()
{
	return m_physicalScreenRects.at(m_modelData.clickedScreenIndex);
}