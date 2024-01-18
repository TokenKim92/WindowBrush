#include "SliderDialog.h"
#include "SliderView.h"
#include "Utility.h"

SliderDialog::SliderDialog(
	const std::wstring &a_title, const SLIDER::RD &a_rangeData, const size_t &a_tickInterval,
	const size_t &a_thumbIndex, const std::vector<std::wstring> &a_ticIntervalTitle
) :
	WindowDialog(L"SLIDERDIALOG", L"SliderDialog"),
	m_title(a_title),
	m_rangeData(a_rangeData),
	m_ticInterval(a_tickInterval),
	m_ticIntervalTitle(a_ticIntervalTitle)
{
	SetSize(SLIDER::DIALOG_WIDTH, SLIDER::DIALOG_HEIGHT);
	SetStyle(WS_POPUP | WS_VISIBLE);
	SetExtendStyle(WS_EX_TOPMOST);

	const size_t maxIndex = (a_rangeData.max - a_rangeData.min) / a_tickInterval;
	size_t thumbIndex = a_thumbIndex;
	if (a_thumbIndex > maxIndex) {
		thumbIndex = maxIndex;
	}
	
	m_modelData = { SLIDER::BT::NONE, SLIDER::BT::NONE, thumbIndex, {0, } };
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

	const auto p_view = new SliderView(mh_window, m_title, m_rangeData, m_ticInterval, m_ticIntervalTitle, GetColorMode());
	InheritDirect2D(p_view);
	p_view->Create();
	m_buttonTable = p_view->GetButtonTable();
	m_ticPoints = p_view->GetTicPoints();

	DPoint tumbPoint = m_ticPoints.at(m_modelData.thumbIndex);
	m_modelData.thumbRect = {
		tumbPoint.x - SLIDER::THUMB_RADIUS, tumbPoint.y - SLIDER::THUMB_RADIUS,
		tumbPoint.x + SLIDER::THUMB_RADIUS, tumbPoint.y + SLIDER::THUMB_RADIUS
	};
}

void SliderDialog::OnPaint()
{
	mp_direct2d->Clear();
	static_cast<SliderView *>(mp_direct2d)->Paint(m_modelData);
}

// to handle the WM_MOUSEMOVE message that occurs when a window is destroyed
int SliderDialog::MouseMoveHandler(WPARAM a_wordParam, LPARAM a_longParam)
{
	static const auto OnClickThumb = [](SliderDialog *const ap_dialog, const POINT a_point)
	{
		size_t currentThumbIndex;
		const auto startChannelPoint = ap_dialog->m_ticPoints.front();
		const auto endChannelPoint = ap_dialog->m_ticPoints.back();

		if (startChannelPoint.x + SLIDER::THUMB_RADIUS > a_point.x) {
			currentThumbIndex = 0;	// so as not to go out of range
		}
		else if (endChannelPoint.x - SLIDER::THUMB_RADIUS < a_point.x) {
			currentThumbIndex = ap_dialog->m_ticPoints.size() - 1; // so as not to go out of range
		}
		else {
			const int ticCount = ap_dialog->m_rangeData.max - ap_dialog->m_rangeData.min;
			const float channelWidth = endChannelPoint.x - startChannelPoint.x;
			const float currentIndex = (ticCount * (a_point.x - startChannelPoint.x) / channelWidth) / ap_dialog->m_ticInterval;

			// increase index with the tolerance 0.5
			currentThumbIndex = currentIndex - static_cast<int>(currentIndex) >= 0.5f
				? static_cast<size_t>(currentIndex + 1)
				: static_cast<size_t>(currentIndex);
		}

		if (ap_dialog->m_modelData.thumbIndex != currentThumbIndex) {
			ap_dialog->m_modelData.thumbIndex = currentThumbIndex;

			DPoint tumbPoint = ap_dialog->m_ticPoints.at(ap_dialog->m_modelData.thumbIndex);
			ap_dialog->m_modelData.thumbRect = {
				tumbPoint.x - SLIDER::THUMB_RADIUS, tumbPoint.y - SLIDER::THUMB_RADIUS,
				tumbPoint.x + SLIDER::THUMB_RADIUS, tumbPoint.y + SLIDER::THUMB_RADIUS
			};

			ap_dialog->Invalidate();
		}
	};

	////////////////////////////////////////////////////////////////
	// implementation
	////////////////////////////////////////////////////////////////

	const POINT point = { LOWORD(a_longParam), HIWORD(a_longParam) };

	if (SLIDER::BT::THUMB == m_modelData.clickedButtonType) {
		OnClickThumb(this, point);

		return S_OK;
	}

	for (const auto &[type, rect] : m_buttonTable) {
		if (PointInRect(rect, point)) {
			if (type != m_modelData.hoverButtonType) {
				m_modelData.hoverButtonType = type;
				Invalidate();
			}

			return S_OK;
		}
	}

	if (PointInRect(m_modelData.thumbRect, point)) {
		if (SLIDER::BT::THUMB != m_modelData.hoverButtonType) {
			m_modelData.hoverButtonType = SLIDER::BT::THUMB;
			Invalidate();
		}

		return S_OK;
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
	//::SetCapture(mh_window);

	const POINT point = { LOWORD(a_longParam), HIWORD(a_longParam) };

	for (auto const &[type, rect] : m_buttonTable) {
		if (PointInRect(rect, point)) {
			m_modelData.clickedButtonType = type;
			Invalidate();

			return S_OK;
		}
	}

	if (PointInRect(m_modelData.thumbRect, point)) {
		m_modelData.clickedButtonType = SLIDER::BT::THUMB;
		Invalidate();

		return S_OK;
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
	static const auto OnButtonUp = [](SliderDialog *const ap_dialog, const SLIDER::BT &a_type)
	{
		if (a_type == ap_dialog->m_modelData.hoverButtonType) {
			if (SLIDER::BT::SAVE == a_type) {
				BT type = BT::OK;
				ap_dialog->SetClickedButtonType(type);
			}

			::DestroyWindow(ap_dialog->mh_window);
			return;
		}

		ap_dialog->m_modelData.clickedButtonType = SLIDER::BT::NONE;
	};

	////////////////////////////////////////////////////////////////
	// implementation
	////////////////////////////////////////////////////////////////

	//::ReleaseCapture();

	const POINT point = { LOWORD(a_longParam), HIWORD(a_longParam) };

	if (SLIDER::BT::NONE != m_modelData.clickedButtonType) {
		switch (m_modelData.clickedButtonType)
		{
		case SLIDER::BT::SAVE:
			OnButtonUp(this, SLIDER::BT::SAVE);
			break;
		case SLIDER::BT::CANCEL:
			OnButtonUp(this, SLIDER::BT::CANCEL);
			break;
		case SLIDER::BT::THUMB:
			m_modelData.clickedButtonType = SLIDER::BT::NONE;
			m_modelData.hoverButtonType = SLIDER::BT::NONE;
			Invalidate();
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

int SliderDialog::GetValue()
{
	return m_rangeData.min + m_modelData.thumbIndex * m_ticInterval;
}