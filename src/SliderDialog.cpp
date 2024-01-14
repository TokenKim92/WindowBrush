#include "SliderDialog.h"
#include "SliderView.h"
#include "Utility.h"

SliderDialog::SliderDialog(const std::wstring &a_title) :
	WindowDialog(L"SLIDERDIALOG", L"SliderDialog"),
	m_title(a_title)
{
	SetSize(SLIDER::DIALOG_WIDTH, SLIDER::DIALOG_HEIGHT);

	m_modelData = { SLIDER::BT::NONE, SLIDER::BT::NONE };
}

SliderDialog::~SliderDialog()
{

}

void SliderDialog::OnInitDialog()
{
	DisableMaximize();
	DisableMinimize();
	DisableSize();

	// add message handlers
	AddMessageHandler(WM_MOUSEMOVE, static_cast<MessageHandler>(&SliderDialog::MouseMoveHandler));
	AddMessageHandler(WM_LBUTTONDOWN, static_cast<MessageHandler>(&SliderDialog::MouseLeftButtonDownHandler));
	AddMessageHandler(WM_LBUTTONUP, static_cast<MessageHandler>(&SliderDialog::MouseLeftButtonUpHandler));
	AddMessageHandler(WM_KEYDOWN, static_cast<MessageHandler>(&SliderDialog::KeyDownHandler));

	const auto p_view = new SliderView(mh_window, m_title, GetColorMode());
	InheritDirect2D(p_view);
	p_view->Create();
	m_buttonTable = p_view->GetButtonTable();
}

void SliderDialog::OnDestroy()
{

}

void SliderDialog::OnPaint()
{
	mp_direct2d->Clear();
	static_cast<SliderView *>(mp_direct2d)->Paint(m_modelData);
}

// to handle the WM_MOUSEMOVE message that occurs when a window is destroyed
int SliderDialog::MouseMoveHandler(WPARAM a_wordParam, LPARAM a_longParam)
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

	if (SLIDER::BT::NONE != m_modelData.hoverButtonType) {
		m_modelData.hoverButtonType = SLIDER::BT::NONE;
		Invalidate();
	}

	return S_OK;
}

// to handle the WM_LBUTTONDOWN message that occurs when a window is destroyed
int SliderDialog::MouseLeftButtonDownHandler(WPARAM a_wordParam, LPARAM a_longParam)
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

	if (SLIDER::BT::NONE != m_modelData.clickedButtonType) {
		m_modelData.clickedButtonType = SLIDER::BT::NONE;
		Invalidate();
	}

	return S_OK;
}

// to handle the WM_LBUTTONUP message that occurs when a window is destroyed
int SliderDialog::MouseLeftButtonUpHandler(WPARAM a_wordParam, LPARAM a_longParam)
{
	static const auto OnButtonUp = [](SliderDialog *const ap_dialog, const HWND &ah_window, const SLIDER::BT &a_type, SLIDER::MD &a_modelData)
	{
		if (a_type == a_modelData.hoverButtonType) {
			if (SLIDER::BT::SAVE == a_type) {
				BT type = BT::OK;
				ap_dialog->SetClickedButtonType(type);
			}

			::DestroyWindow(ah_window);
			return;
		}

		a_modelData.clickedButtonType = SLIDER::BT::NONE;
	};

	////////////////////////////////////////////////////////////////
	// implementation
	////////////////////////////////////////////////////////////////

	::ReleaseCapture();

	const POINT point = { LOWORD(a_longParam), HIWORD(a_longParam) };

	if (SLIDER::BT::NONE != m_modelData.clickedButtonType) {
		switch (m_modelData.clickedButtonType)
		{
		case SLIDER::BT::SAVE:
			OnButtonUp(this, mh_window, SLIDER::BT::SAVE, m_modelData);
			break;
		case SLIDER::BT::CANCEL:
			OnButtonUp(this, mh_window, SLIDER::BT::CANCEL, m_modelData);
			break;
		case SLIDER::BT::SLIDER:
			break;
		default:
			break;
		}
	}

	return S_OK;
}

// to handle the WM_KEYDOWN message that occurs when a window is destroyed
int SliderDialog::KeyDownHandler(WPARAM a_wordParam, LPARAM a_longParam)
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
