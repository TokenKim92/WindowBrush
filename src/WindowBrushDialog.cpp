#include "WindowBrushDialog.h"
#include "WindowBrushView.h"
#include "ColorDialog.h"
#include "EditDialog.h"
#include "ColorPalette.h"
#include "SliderDialog.h"
#include "ScreenDialog.h"
#include "InfoDialog.h"
#include "InfoModel.h"
#include "Utility.h"
#include <map>
#include <sstream>

#ifdef _DEBUG
#pragma comment (lib, "AppTemplateDebug.lib")
#else
#pragma comment (lib, "AppTemplate.lib")     
#endif

void __stdcall ShowInfoDialog(HWND ah_wnd, UINT a_msg, UINT_PTR ap_data, DWORD dwTime)
{
	::KillTimer(ah_wnd, ap_data);

	const auto p_data = reinterpret_cast<WINDOW_BRUSH::IDD *>(ap_data);
	auto windowBrushDialog = static_cast<WindowBrushDialog *>(p_data->windowBrushDialog);

	const auto point = windowBrushDialog->GetInfoDialogPoint();
	p_data->infoDialog = new InfoDialog(windowBrushDialog->GetHoverButtonTitle());
	p_data->infoDialog->Create(point.x, point.y);
}

WindowBrushDialog::WindowBrushDialog() :
	WindowDialog(L"WINDOWBRUSH", L"")
{
	SetSize(WINDOW_BRUSH::DIALOG_WIDTH, WINDOW_BRUSH::DIALOG_HEIGHT);
	SetExtendStyle(WS_EX_TOPMOST | WS_EX_LAYERED);

	m_modelData.hoverButtonType = WINDOW_BRUSH::BT::NONE;
	m_modelData.drawType = WINDOW_BRUSH::DT::NONE;
	m_modelData.strokeWidth = 20;
	m_modelData.fontSize = 180;
	m_modelData.isGradientMode = false;
	m_modelData.selectedColor = RGB_TO_COLORF(BLUE_500);
	m_modelData.isFadeMode = false;

	m_modelData.selectedScreenRect = { 0, 0, ::GetSystemMetrics(SM_CXSCREEN), ::GetSystemMetrics(SM_CYSCREEN) };
	m_modelData.fadeTimer = 1500;
	m_modelData.colorOpacity = 1.0f;

	m_infoDialogData = { this, nullptr };
	mp_sketchDialog = nullptr;

	m_isLeftMouse = true;
	m_isHiddenMode = false;
}

void WindowBrushDialog::OnInitDialog()
{
	DisableMaximize();
	DisableMinimize();
	DisableSize();

	// add message handlers
	AddMessageHandler(WM_MOUSEMOVE, static_cast<MessageHandler>(&WindowBrushDialog::MouseMoveHandler));
	AddMessageHandler(WM_LBUTTONDOWN, static_cast<MessageHandler>(&WindowBrushDialog::MouseLeftButtonDownHandler));
	AddMessageHandler(WM_LBUTTONUP, static_cast<MessageHandler>(&WindowBrushDialog::MouseLeftButtonUpHandler));
	AddMessageHandler(WM_MOUSELEAVE, static_cast<MessageHandler>(&WindowBrushDialog::MouseLeaveHandler));
	AddMessageHandler(WINDOW_BRUSH::WM_KILLED_SKETCH, static_cast<MessageHandler>(&WindowBrushDialog::KilledSketchDialogHandler));
	AddMessageHandler(WM_KEYDOWN, static_cast<MessageHandler>(&WindowBrushDialog::KeyDownHandler));

	const auto p_view = new WindowBrushView(mh_window, GetColorMode());
	InheritDirect2D(p_view);
	p_view->Create();
	p_view->UpdateColorSymbolBrush(m_modelData.selectedColor);
	m_buttonTable = p_view->GetButtonTable();

	HMENU h_systemMenu = ::GetSystemMenu(mh_window, FALSE);
	if (nullptr != h_systemMenu) {
		::InsertMenuW(h_systemMenu, MENU_LIGHT_MODE, MF_STRING, WINDOW_BRUSH::MENU_SELECT_SCREEN, L"Select Screen\tCtrl+S");
		::InsertMenuW(h_systemMenu, MENU_LIGHT_MODE, MF_STRING, WINDOW_BRUSH::MENU_COLOR_OPACITY, L"Color Opacity\tCtrl+C");
		::InsertMenuW(h_systemMenu, MENU_LIGHT_MODE, MF_STRING, WINDOW_BRUSH::MENU_FADE_SPEED, L"Fade Timer\tCtrl+F");
		::InsertMenuW(h_systemMenu, MENU_LIGHT_MODE, MF_SEPARATOR, NULL, nullptr);
		::InsertMenuW(h_systemMenu, SC_MOVE, MF_STRING, WINDOW_BRUSH::MENU_HIDDEN, L"Hidden Mode\tCtrl+H");
	}

	::EnumDisplayMonitors(nullptr, nullptr, GetPhysicalScreenRects, reinterpret_cast<LPARAM>(&m_physicalScreenRects));
	::SetLayeredWindowAttributes(mh_window, 0, 255, LWA_ALPHA);
}

void WindowBrushDialog::OnDestroy()
{
	KillInfoDialogTimer();
}

void WindowBrushDialog::OnPaint()
{
	static_cast<WindowBrushView *>(mp_direct2d)->Paint(m_modelData);
}

void WindowBrushDialog::OnSetColorMode()
{
	static_cast<WindowBrushView *>(mp_direct2d)->SetColorMode(GetColorMode());
	Invalidate();
}

// to handle the WM_MOUSEMOVE message that occurs when a window is destroyed
int WindowBrushDialog::MouseMoveHandler(WPARAM a_wordParam, LPARAM a_longParam)
{
	if (m_isLeftMouse) {
		m_isLeftMouse = false;

		// to call the method OnMouseLeave() on mouse leave
		TRACKMOUSEEVENT data = { sizeof(TRACKMOUSEEVENT), TME_LEAVE, mh_window };
		::TrackMouseEvent(&data);
	}

	const POINT pos = { LOWORD(a_longParam), HIWORD(a_longParam) };

	for (auto const &[type, rect] : m_buttonTable) {
		if (PointInRect(rect, pos)) {
			if (type != m_modelData.hoverButtonType) {
				m_modelData.hoverButtonType = type;
				KillInfoDialogTimer();
				::SetTimer(mh_window, reinterpret_cast<UINT_PTR>(&m_infoDialogData), 1500, ShowInfoDialog);

				Invalidate();
			}

			return S_OK;
		}
	}

	if (WINDOW_BRUSH::BT::NONE != m_modelData.hoverButtonType) {
		m_modelData.hoverButtonType = WINDOW_BRUSH::BT::NONE;
		KillInfoDialogTimer();

		Invalidate();
	}

	return S_OK;
}

// to handle the WM_LBUTTONDOWN  message that occurs when a window is destroyed
int WindowBrushDialog::MouseLeftButtonDownHandler(WPARAM a_wordParam, LPARAM a_longParam)
{
	const POINT pos = { LOWORD(a_longParam), HIWORD(a_longParam) };

	KillInfoDialogTimer();

	return S_OK;
}

// to handle the WM_LBUTTONUP  message that occurs when a window is destroyed
int WindowBrushDialog::MouseLeftButtonUpHandler(WPARAM a_wordParam, LPARAM a_longParam)
{
	static std::map<WINDOW_BRUSH::BT, void (WindowBrushDialog:: *)()> keyTable = {
		{WINDOW_BRUSH::BT::CURVE, &WindowBrushDialog::OnCurveButtonUp},
		{WINDOW_BRUSH::BT::RECTANGLE, &WindowBrushDialog::OnRectangleButtonUp},
		{WINDOW_BRUSH::BT::CIRCLE, &WindowBrushDialog::OnEllipseButtonUp},
		{WINDOW_BRUSH::BT::TEXT, &WindowBrushDialog::OnTextButtonUp},
		{WINDOW_BRUSH::BT::STROKE, &WindowBrushDialog::OnStrokeButtonUp},
		{WINDOW_BRUSH::BT::GRADIENT, &WindowBrushDialog::OnGradientButtonUp},
		{WINDOW_BRUSH::BT::COLOR, &WindowBrushDialog::OnColorButtonUp},
		{WINDOW_BRUSH::BT::FADE, &WindowBrushDialog::OnFadeButtonUp}
	};

	const POINT point = { LOWORD(a_longParam), HIWORD(a_longParam) };

	// check first click area on draw mode buttons
	for (auto const &[type, rect] : m_buttonTable) {
		if (PointInRect(rect, point)) {
			(this->*keyTable.at(type))();
			
			break;
		}
	}

	return S_OK;
}

// to handle the WM_MOUSELEAVE  message that occurs when a window is destroyed
int WindowBrushDialog::MouseLeaveHandler(WPARAM a_wordParam, LPARAM a_longParam)
{
	if (!m_isLeftMouse) {
		m_isLeftMouse = true;

		if (WINDOW_BRUSH::BT::NONE != m_modelData.hoverButtonType) {
			m_modelData.hoverButtonType = WINDOW_BRUSH::BT::NONE;
			KillInfoDialogTimer();

			Invalidate();
		}
	}

	return S_OK;
}

// to handle the WM_KEYDOWN
int WindowBrushDialog::KeyDownHandler(WPARAM a_wordParam, LPARAM a_longParam)
{
	static const auto IsControlKeyDown = []()
	{
		return GetKeyState(VK_CONTROL) & 0x8000;
	};

	///////////////////////////////////////////////////////////////////
	// implementation
	///////////////////////////////////////////////////////////////////

	static std::map<unsigned char, void (WindowBrushDialog:: *)()> keyTableWithControl = {
		{'S', &WindowBrushDialog::OnClickSelectScreenMenu},
		{'C', &WindowBrushDialog::OnClickColorOpacityMenu},
		{'F', &WindowBrushDialog::OnClickFadeSpeedMenu},
		{'H', &WindowBrushDialog::OnClickHiddenMenu}
	};
	static std::map<unsigned char, void (WindowBrushDialog:: *)()> keyTable = {
		{'C', &WindowBrushDialog::OnCurveButtonUp},
		{'R', &WindowBrushDialog::OnRectangleButtonUp},
		{'E', &WindowBrushDialog::OnEllipseButtonUp},
		{'T', &WindowBrushDialog::OnTextButtonUp},
		{'W', &WindowBrushDialog::OnStrokeButtonUp},
		{'G', &WindowBrushDialog::OnGradientButtonUp},
		{'P', &WindowBrushDialog::OnColorButtonUp},
		{'F', &WindowBrushDialog::OnFadeButtonUp}
	};

	const unsigned char pressedKey = static_cast<unsigned char>(a_wordParam);

	if (IsControlKeyDown()) {
		if (auto data = keyTableWithControl.find(pressedKey); data != keyTableWithControl.end()) {
			(this->*(data->second))();
		}
	}
	else {
		if (auto data = keyTable.find(pressedKey); data != keyTable.end()) {
			(this->*(data->second))();
		}
	}

	return S_OK;
}

// to handle the WM_SYSCOMMAND message that occurs when a window is created
msg_handler int WindowBrushDialog::SysCommandHandler(WPARAM a_menuID, LPARAM a_longParam)
{
	switch (a_menuID)
	{
	case WINDOW_BRUSH::MENU_SELECT_SCREEN:
		OnClickSelectScreenMenu();
		break;
	case WINDOW_BRUSH::MENU_COLOR_OPACITY:
		OnClickColorOpacityMenu();
		break;
	case WINDOW_BRUSH::MENU_FADE_SPEED:
		OnClickFadeSpeedMenu();
		break;
	case WINDOW_BRUSH::MENU_HIDDEN:
		OnClickHiddenMenu();
		break;
	default:
		break;
	}

	return WindowDialog::SysCommandHandler(a_menuID, a_longParam);
}

std::wstring WindowBrushDialog::GetHoverButtonTitle()
{
	const std::map<WINDOW_BRUSH::BT, std::wstring> titleList = {
		{WINDOW_BRUSH::BT::CURVE, L"Curve Tool (C)"},
		{WINDOW_BRUSH::BT::RECTANGLE, L"Rectangle Tool (R)"},
		{WINDOW_BRUSH::BT::CIRCLE, L"Ellipse Tool (E)"},
		{WINDOW_BRUSH::BT::TEXT, L"Text Tool (T)"},
		{WINDOW_BRUSH::BT::STROKE, L"Width Tool (W)"},
		{WINDOW_BRUSH::BT::GRADIENT, L"Gradation Tool (G)"},
		{WINDOW_BRUSH::BT::COLOR, L"Palette Tool (P)"},
		{WINDOW_BRUSH::BT::FADE, L"Fade Tool (F)"}
	};

	return titleList.at(m_modelData.hoverButtonType);
}

POINT WindowBrushDialog::GetInfoDialogPoint()
{
	if (WINDOW_BRUSH::BT::NONE == m_modelData.hoverButtonType) {
		return { 0, 0 };
	}

	DRect rect = m_buttonTable.at(m_modelData.hoverButtonType);
	POINT point = { static_cast<long>(rect.right),  static_cast<long>(rect.top) };
	::ClientToScreen(mh_window, &point);

	RECT screenRect = { 0, 0, 0, 0 };
	for (const auto &physicalRect : m_physicalScreenRects) {
		if (PointInRect(physicalRect, point)) {
			screenRect = physicalRect;
			break;
		}
	}

	if (point.x + static_cast<long>(INFO::DIALOG_WIDTH + WINDOW_BRUSH::BUTTON_X_MARGIN * 2.0f) < screenRect.right) {
		point.x += static_cast<long>(WINDOW_BRUSH::BUTTON_X_MARGIN * 2.0f);
	}
	else {
		point.x -= INFO::DIALOG_WIDTH + WINDOW_BRUSH::DIALOG_WIDTH - static_cast<unsigned int>(WINDOW_BRUSH::BUTTON_X_MARGIN);
	}
	point.y += static_cast<long>((rect.bottom - rect.top - INFO::DIALOG_HEIGHT) / 2.0f);

	return point;
}

void WindowBrushDialog::KillInfoDialogTimer()
{
	::KillTimer(mh_window, reinterpret_cast<UINT_PTR>(&m_infoDialogData));

	auto p_infoDialog = &m_infoDialogData.infoDialog;
	if (nullptr != *p_infoDialog) {
		(*p_infoDialog)->DestroyWindow();

		delete *p_infoDialog;
		*p_infoDialog = nullptr;
	}
}

int WindowBrushDialog::KilledSketchDialogHandler(WPARAM a_wordParam, LPARAM a_longParam)
{
	delete mp_sketchDialog;
	mp_sketchDialog = nullptr;

	return S_OK;
}

void WindowBrushDialog::OnDrawButtonUp(const WINDOW_BRUSH::DT &a_type)
{
	static const auto GetScaledRect = [](const RECT &a_rect) -> RECT
	{
		// change DPI awareness mode and reset again to set the correct position of window 
		DPI_AWARENESS_CONTEXT dpiContext = SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE);
		SetThreadDpiAwarenessContext(dpiContext);

		// to get scaled monitor size
		const auto rect = a_rect;
		POINT point = { rect.left + (rect.right - rect.left) / 2, rect.top + (rect.bottom - rect.top) / 2 };
		HMONITOR h_mainMonitor = MonitorFromPoint(point, MONITOR_DEFAULTTOPRIMARY);

		MONITORINFOEX info;
		info.cbSize = sizeof(MONITORINFOEX);
		info.dwFlags = 0;
		GetMonitorInfo(h_mainMonitor, &info);

		return {
			rect.left,
			rect.top,
			rect.left + (info.rcMonitor.right - info.rcMonitor.left),
			rect.top + (info.rcMonitor.bottom - info.rcMonitor.top)
		};
	};

	////////////////////////////////////////////////////////////////
	// implementation
	////////////////////////////////////////////////////////////////

	// same button click -> turn off
	if (a_type == m_modelData.drawType) {
		::DestroyWindow(mp_sketchDialog->GetWidnowHandle());

		m_modelData.drawType = WINDOW_BRUSH::DT::NONE;
		HMENU h_systemMenu = ::GetSystemMenu(mh_window, FALSE);
		if (nullptr != h_systemMenu) {
			::EnableMenuItem(h_systemMenu, WINDOW_BRUSH::MENU_SELECT_SCREEN, MF_ENABLED);
		}

		Invalidate();

		return;
	}

	m_modelData.drawType = a_type;
	Invalidate();
	// other button click -> chnage draw mode
	if (nullptr != mp_sketchDialog) {
		mp_sketchDialog->UpdateWindowBrushModelData(&m_modelData);

		return;
	}

	// button click -> turn on
	HMENU h_systemMenu = ::GetSystemMenu(mh_window, FALSE);
	if (nullptr != h_systemMenu) {
		::EnableMenuItem(h_systemMenu, WINDOW_BRUSH::MENU_SELECT_SCREEN, MF_DISABLED);
	}

	const auto scaledRect = GetScaledRect(m_modelData.selectedScreenRect);
	mp_sketchDialog = new SketchDialog(mh_window, m_modelData, scaledRect);
	mp_sketchDialog->SetColorMode(m_colorMode);

	DisableClose();
	mp_sketchDialog->DoModal(nullptr, scaledRect.left, scaledRect.top);
	m_modelData.drawType = WINDOW_BRUSH::DT::NONE;
	EnableClose();

	Invalidate();
};

void WindowBrushDialog::OnCurveButtonUp()
{
	OnDrawButtonUp(WINDOW_BRUSH::DT::CURVE);
}

void WindowBrushDialog::OnRectangleButtonUp()
{
	OnDrawButtonUp(WINDOW_BRUSH::DT::RECTANGLE);
}

void WindowBrushDialog::OnEllipseButtonUp()
{
	OnDrawButtonUp(WINDOW_BRUSH::DT::CIRCLE);
}

void WindowBrushDialog::OnTextButtonUp()
{
	OnDrawButtonUp(WINDOW_BRUSH::DT::TEXT_OUTLINE);
}

void WindowBrushDialog::OnStrokeButtonUp()
{
	const std::vector<std::pair<std::wstring, unsigned int>> itemList = {
		{ L"stroke width(px)", m_modelData.strokeWidth },
		{ L"font size(dpi)", m_modelData.fontSize }
	};

	EditDialog instanceDialog(L"Stroke Width", itemList, EDIT::RANGE({ 1, 999 }));
	instanceDialog.SetColorMode(m_colorMode);

	RECT rect;
	::GetWindowRect(mh_window, &rect);
	const int centerPosX = rect.left + (rect.right - rect.left) / 2;
	const int centerPosY = rect.top + (rect.bottom - rect.top) / 2;
	const SIZE size = instanceDialog.GetSize();

	if (BT::OK == instanceDialog.DoModal(mh_window, centerPosX - size.cx / 2, centerPosY - size.cy / 2)) {
		auto valueList = instanceDialog.GetValueList();
		m_modelData.strokeWidth = valueList.at(0);
		m_modelData.fontSize = valueList.at(1);

		if (nullptr != mp_sketchDialog) {
			mp_sketchDialog->UpdateWindowBrushModelData(&m_modelData);
		}
	}

	::SetFocus(mh_window);
};

void WindowBrushDialog::OnColorButtonUp()
{
	if (m_modelData.isGradientMode) {
		return;
	}

	ColorDialog instanceDialog(m_modelData.selectedColor, m_colorList);
	instanceDialog.SetColorMode(m_colorMode);

	RECT rect;
	::GetWindowRect(mh_window, &rect);
	const int centerPosX = rect.left + (rect.right - rect.left) / 2;
	const int centerPosY = rect.top + (rect.bottom - rect.top) / 2;
	const SIZE size = instanceDialog.GetSize();

	if (BT::OK == instanceDialog.DoModal(mh_window, centerPosX - size.cx / 2, centerPosY - size.cy / 2)) {
		m_modelData.selectedColor = instanceDialog.GetSelectedColor();
		m_colorList = instanceDialog.GetColorList();
		static_cast<WindowBrushView *>(mp_direct2d)->UpdateColorSymbolBrush(m_modelData.selectedColor);

		if (nullptr != mp_sketchDialog) {
			mp_sketchDialog->UpdateWindowBrushModelData(&m_modelData);
		}
		Invalidate();
	}

	::SetFocus(mh_window);
};

void WindowBrushDialog::OnGradientButtonUp()
{
	m_modelData.isGradientMode = !m_modelData.isGradientMode;

	if (nullptr != mp_sketchDialog) {
		mp_sketchDialog->UpdateWindowBrushModelData(&m_modelData);
	}

	Invalidate();
};

void WindowBrushDialog::OnFadeButtonUp()
{
	m_modelData.isFadeMode = !m_modelData.isFadeMode;
	if (nullptr != mp_sketchDialog) {
		mp_sketchDialog->UpdateWindowBrushModelData(&m_modelData);
	}

	Invalidate();
};

void WindowBrushDialog::OnClickSelectScreenMenu()
{
	if (nullptr != mp_sketchDialog) {
		return;
	}

	ScreenDialog instanceDialog(m_modelData.selectedScreenRect);
	instanceDialog.SetColorMode(m_colorMode);

	RECT rect;
	::GetWindowRect(mh_window, &rect);
	const int centerPosX = rect.left + (rect.right - rect.left) / 2;
	const int centerPosY = rect.top + (rect.bottom - rect.top) / 2;
	const SIZE size = instanceDialog.GetSize();

	if (BT::OK == instanceDialog.DoModal(mh_window, centerPosX - size.cx / 2, centerPosY - size.cy / 2)) {
		m_modelData.selectedScreenRect = instanceDialog.GetSelectedRect();
	}

	::SetFocus(mh_window);
};

void WindowBrushDialog::OnClickColorOpacityMenu()
{
	const size_t ticInterval = 10;
	SLIDER::RD rangeData = {
		L"10%", 10,
		L"100%", 100
	};
	int thumbIndex = static_cast<int>(m_modelData.colorOpacity * 100.0f - rangeData.min) / ticInterval;
	std::vector<std::wstring> ticIntervalTitle;

	for (int i = rangeData.min; i <= rangeData.max; i += ticInterval) {
		ticIntervalTitle.push_back(std::to_wstring(i) + L"%");
	}

	SliderDialog instanceDialog(L"Color Opacity", rangeData, ticInterval, thumbIndex, ticIntervalTitle);
	instanceDialog.SetColorMode(m_colorMode);

	RECT rect;
	::GetWindowRect(mh_window, &rect);
	const int centerPosX = rect.left + (rect.right - rect.left) / 2;
	const int centerPosY = rect.top + (rect.bottom - rect.top) / 2;
	const SIZE size = instanceDialog.GetSize();

	if (BT::OK == instanceDialog.DoModal(mh_window, centerPosX - size.cx / 2, centerPosY - size.cy / 2)) {
		m_modelData.colorOpacity = instanceDialog.GetValue() / 100.0f;

		if (nullptr != mp_sketchDialog) {
			mp_sketchDialog->UpdateWindowBrushModelData(&m_modelData);
		}
	}

	::SetFocus(mh_window);
};

void WindowBrushDialog::OnClickFadeSpeedMenu ()
{
	const size_t ticInterval = 500;
	SLIDER::RD rangeData = {
		L"Fast", 0,
		L"Slow", 3000
	};
	int thumbIndex = static_cast<int>(m_modelData.fadeTimer / ticInterval);
	std::wostringstream out;
	out.precision(1);

	std::vector<std::wstring> ticIntervalTitle;
	for (int i = rangeData.min; i <= rangeData.max; i += ticInterval) {
		out << std::fixed << i / 1000.0f;
		ticIntervalTitle.push_back(out.str() + L"s");
		out.str(L"");
	}

	SliderDialog instanceDialog(L"Fade Timer", rangeData, ticInterval, thumbIndex, ticIntervalTitle);
	instanceDialog.SetColorMode(m_colorMode);

	RECT rect;
	::GetWindowRect(mh_window, &rect);
	const int centerPosX = rect.left + (rect.right - rect.left) / 2;
	const int centerPosY = rect.top + (rect.bottom - rect.top) / 2;
	const SIZE size = instanceDialog.GetSize();

	if (BT::OK == instanceDialog.DoModal(mh_window, centerPosX - size.cx / 2, centerPosY - size.cy / 2)) {
		m_modelData.fadeTimer = instanceDialog.GetValue();

		if (nullptr != mp_sketchDialog) {
			mp_sketchDialog->UpdateWindowBrushModelData(&m_modelData);
		}
	}

	::SetFocus(mh_window);
};

void WindowBrushDialog::OnClickHiddenMenu()
{
	m_isHiddenMode = !m_isHiddenMode;

	const unsigned char alpha = m_isHiddenMode ? 0 : 255;
	::SetLayeredWindowAttributes(mh_window, 0, alpha, LWA_ALPHA);
}