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

#define MENU_SELECT_SCREEN		MENU_LIGHT_MODE + 1
#define MENU_COLOR_OPACITY		MENU_LIGHT_MODE + 2
#define MENU_FADE_SPEED			MENU_LIGHT_MODE + 3

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

	m_modelData.hoverButtonType = WINDOW_BRUSH::BT::NONE;
	m_modelData.drawType = WINDOW_BRUSH::DT::NONE;
	m_modelData.strokeWidth = 20;
	m_modelData.fontSize = 180;
	m_modelData.isGradientMode = false;
	m_modelData.selectedColor = RGB_TO_COLORF(ORANGE_500);
	m_modelData.isFadeMode = false;

	m_modelData.selectedScreenRect = { 0, 0, ::GetSystemMetrics(SM_CXSCREEN), ::GetSystemMetrics(SM_CYSCREEN) };
	m_modelData.fadeTimer = 500;
	m_modelData.colorOpacity = 1.0f;

	m_infoDialogData = { this, nullptr };

	m_isLeftMouse = true;
	mp_sketchDialog = nullptr;
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

	const auto p_view = new WindowBrushView(mh_window, GetColorMode());
	InheritDirect2D(p_view);
	p_view->Create();
	p_view->UpdateColorSymbolBrush(m_modelData.selectedColor);
	m_buttonTable = p_view->GetButtonTable();

	HMENU h_systemMenu = ::GetSystemMenu(mh_window, FALSE);
	if (nullptr != h_systemMenu) {
		::InsertMenuW(h_systemMenu, MENU_LIGHT_MODE, MF_STRING, MENU_SELECT_SCREEN, L"Select Screen");
		::InsertMenuW(h_systemMenu, MENU_LIGHT_MODE, MF_STRING, MENU_COLOR_OPACITY, L"Color Opacity");
		::InsertMenuW(h_systemMenu, MENU_LIGHT_MODE, MF_STRING, MENU_FADE_SPEED, L"Fade Timer");
		::InsertMenuW(h_systemMenu, MENU_LIGHT_MODE, MF_SEPARATOR, NULL, nullptr);
	}

	::EnumDisplayMonitors(nullptr, nullptr, GetPhysicalScreenRects, reinterpret_cast<LPARAM>(&m_physicalScreenRects));
}

void WindowBrushDialog::OnDestroy()
{
	KillInfoDialogTimer();

	if (mp_sketchDialog) {
		DestroySketchDialog();
	}
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
	static const auto OnDrawButtonUp = [](WindowBrushDialog *const ap_dialog, const WINDOW_BRUSH::DT &a_type)
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
		if (a_type == ap_dialog->m_modelData.drawType) {
			ap_dialog->DestroySketchDialog();

			ap_dialog->m_modelData.drawType = WINDOW_BRUSH::DT::NONE;
			HMENU h_systemMenu = ::GetSystemMenu(ap_dialog->mh_window, FALSE);
			if (nullptr != h_systemMenu) {
				::EnableMenuItem(h_systemMenu, MENU_SELECT_SCREEN, MF_ENABLED);
			}

			ap_dialog->Invalidate();

			return;
		}

		ap_dialog->m_modelData.drawType = a_type;
		ap_dialog->Invalidate();
		// other button click -> chnage draw mode
		if (nullptr != ap_dialog->mp_sketchDialog) {
			ap_dialog->mp_sketchDialog->UpdateWindowBrushModelData(&ap_dialog->m_modelData);

			return;
		}

		// button click -> turn on
		HMENU h_systemMenu = ::GetSystemMenu(ap_dialog->mh_window, FALSE);
		if (nullptr != h_systemMenu) {
			::EnableMenuItem(h_systemMenu, MENU_SELECT_SCREEN, MF_DISABLED);
		}

		const auto scaledRect = GetScaledRect(ap_dialog->m_modelData.selectedScreenRect);
		ap_dialog->mp_sketchDialog = new SketchDialog(ap_dialog->m_modelData, scaledRect);
		ap_dialog->mp_sketchDialog->SetColorMode(ap_dialog->m_colorMode);

		if (!ap_dialog->mp_sketchDialog->Create(scaledRect.left, scaledRect.top)) {
			ap_dialog->m_modelData.drawType = WINDOW_BRUSH::DT::NONE;
			ap_dialog->DestroySketchDialog();
		}
	};
	static const auto OnStrokeButtonUp = [](WindowBrushDialog *const ap_dialog)
	{
		const std::vector<std::pair<std::wstring, unsigned int>> itemList = {
			{ L"stroke width(px)", ap_dialog->m_modelData.strokeWidth },
			{ L"font size(px)", ap_dialog->m_modelData.fontSize }
		};

		EditDialog instanceDialog(L"Stroke Width", itemList, EDIT::RANGE({ 1, 999 }));
		instanceDialog.SetColorMode(ap_dialog->m_colorMode);

		RECT rect;
		::GetWindowRect(ap_dialog->mh_window, &rect);
		const int centerPosX = rect.left + (rect.right - rect.left) / 2;
		const int centerPosY = rect.top + (rect.bottom - rect.top) / 2;
		const SIZE size = instanceDialog.GetSize();

		if (BT::OK == instanceDialog.DoModal(ap_dialog->mh_window, centerPosX - size.cx / 2, centerPosY - size.cy / 2)) {
			auto valueList = instanceDialog.GetValueList();
			ap_dialog->m_modelData.strokeWidth = valueList.at(0);
			ap_dialog->m_modelData.fontSize = valueList.at(1);

			if (nullptr != ap_dialog->mp_sketchDialog) {
				ap_dialog->mp_sketchDialog->UpdateWindowBrushModelData(&ap_dialog->m_modelData);
			}
		}
	};
	static const auto OnColorButtonUp = [](WindowBrushDialog *const ap_dialog)
	{
		ColorDialog instanceDialog(ap_dialog->m_modelData.selectedColor, ap_dialog->m_colorList);
		instanceDialog.SetColorMode(ap_dialog->m_colorMode);

		RECT rect;
		::GetWindowRect(ap_dialog->mh_window, &rect);
		const int centerPosX = rect.left + (rect.right - rect.left) / 2;
		const int centerPosY = rect.top + (rect.bottom - rect.top) / 2;
		const SIZE size = instanceDialog.GetSize();

		if (BT::OK == instanceDialog.DoModal(ap_dialog->mh_window, centerPosX - size.cx / 2, centerPosY - size.cy / 2)) {
			ap_dialog->m_modelData.selectedColor = instanceDialog.GetSelectedColor();
			ap_dialog->m_colorList = instanceDialog.GetColorList();
			static_cast<WindowBrushView *>(ap_dialog->mp_direct2d)->UpdateColorSymbolBrush(ap_dialog->m_modelData.selectedColor);

			if (nullptr != ap_dialog->mp_sketchDialog) {
				ap_dialog->mp_sketchDialog->UpdateWindowBrushModelData(&ap_dialog->m_modelData);
			}

			ap_dialog->Invalidate();
		}
	};
	static const auto OnGradientButtonUp = [](WindowBrushDialog *const ap_dialog)
	{
		ap_dialog->m_modelData.isGradientMode = !ap_dialog->m_modelData.isGradientMode;

		if (nullptr != ap_dialog->mp_sketchDialog) {
			ap_dialog->mp_sketchDialog->UpdateWindowBrushModelData(&ap_dialog->m_modelData);
		}

		ap_dialog->Invalidate();
	};
	static const auto OnFadeButtonUp = [](WindowBrushDialog *const ap_dialog)
	{
		ap_dialog->m_modelData.isFadeMode = !ap_dialog->m_modelData.isFadeMode;
		if (nullptr != ap_dialog->mp_sketchDialog) {
			ap_dialog->mp_sketchDialog->UpdateWindowBrushModelData(&ap_dialog->m_modelData);
		}

		ap_dialog->Invalidate();
	};

	////////////////////////////////////////////////////////////////
	// implementation
	////////////////////////////////////////////////////////////////
	static std::map< WINDOW_BRUSH::BT, WINDOW_BRUSH::DT> buttonDrawTable = {
		{ WINDOW_BRUSH::BT::CURVE, WINDOW_BRUSH::DT::CURVE },
		{ WINDOW_BRUSH::BT::RECTANGLE, WINDOW_BRUSH::DT::RECTANGLE },
		{ WINDOW_BRUSH::BT::CIRCLE, WINDOW_BRUSH::DT::CIRCLE },
		{ WINDOW_BRUSH::BT::TEXT, WINDOW_BRUSH::DT::TEXT_OUTLINE }
	};
	const POINT pos = { LOWORD(a_longParam), HIWORD(a_longParam) };

	// check first click area on draw mode buttons
	for (auto const &[type, rect] : m_buttonTable) {
		if (PointInRect(rect, pos)) {
			switch (type)
			{
			case WINDOW_BRUSH::BT::CURVE:
			case WINDOW_BRUSH::BT::RECTANGLE:
			case WINDOW_BRUSH::BT::CIRCLE:
			case WINDOW_BRUSH::BT::TEXT:
				OnDrawButtonUp(this, buttonDrawTable.at(type));
				break;
			case WINDOW_BRUSH::BT::STROKE:
				OnStrokeButtonUp(this);
				break;
			case WINDOW_BRUSH::BT::GRADIENT:
				OnGradientButtonUp(this);
				break;
			case WINDOW_BRUSH::BT::COLOR:
				if (!m_modelData.isGradientMode) {
					OnColorButtonUp(this);
				}
				break;
			case WINDOW_BRUSH::BT::FADE:
				OnFadeButtonUp(this);
				break;
			default:
				break;
			}

			return S_OK;
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

// to handle the WM_SYSCOMMAND message that occurs when a window is created
msg_handler int WindowBrushDialog::SysCommandHandler(WPARAM a_menuID, LPARAM a_longParam)
{
	const auto OnClickSelectScreenMenu = [](WindowBrushDialog *const ap_dialog)
	{
		ScreenDialog instanceDialog(ap_dialog->m_modelData.selectedScreenRect);
		instanceDialog.SetColorMode(ap_dialog->m_colorMode);

		RECT rect;
		::GetWindowRect(ap_dialog->mh_window, &rect);
		const int centerPosX = rect.left + (rect.right - rect.left) / 2;
		const int centerPosY = rect.top + (rect.bottom - rect.top) / 2;
		const SIZE size = instanceDialog.GetSize();

		if (BT::OK == instanceDialog.DoModal(ap_dialog->mh_window, centerPosX - size.cx / 2, centerPosY - size.cy / 2)) {
			ap_dialog->m_modelData.selectedScreenRect = instanceDialog.GetSelectedRect();
		}
	};
	const auto OnClickColorOpacityMenu = [](WindowBrushDialog *const ap_dialog)
	{
		const size_t ticInterval = 10;
		SLIDER::RD rangeData = {
			L"10%", 10,
			L"100%", 100
		};
		int thumbIndex = static_cast<int>(ap_dialog->m_modelData.colorOpacity * 100.0f - rangeData.min) / ticInterval;
		std::vector<std::wstring> ticIntervalTitle;

		for (int i = rangeData.min; i <= rangeData.max; i += ticInterval) {
			ticIntervalTitle.push_back(std::to_wstring(i) + L"%");
		}

		SliderDialog instanceDialog(L"Color Opacity", rangeData, ticInterval, thumbIndex, ticIntervalTitle);
		instanceDialog.SetColorMode(ap_dialog->m_colorMode);

		RECT rect;
		::GetWindowRect(ap_dialog->mh_window, &rect);
		const int centerPosX = rect.left + (rect.right - rect.left) / 2;
		const int centerPosY = rect.top + (rect.bottom - rect.top) / 2;
		const SIZE size = instanceDialog.GetSize();

		if (BT::OK == instanceDialog.DoModal(ap_dialog->mh_window, centerPosX - size.cx / 2, centerPosY - size.cy / 2)) {
			ap_dialog->m_modelData.colorOpacity = instanceDialog.GetValue() / 100.0f;

			if (nullptr != ap_dialog->mp_sketchDialog) {
				ap_dialog->mp_sketchDialog->UpdateWindowBrushModelData(&ap_dialog->m_modelData);

				if (nullptr != ap_dialog->mp_sketchDialog) {
					ap_dialog->mp_sketchDialog->UpdateWindowBrushModelData(&ap_dialog->m_modelData);
				}
			}
		}
	};
	const auto OnClickFadeSpeedMenu = [](WindowBrushDialog *const ap_dialog)
	{
		const size_t ticInterval = 500;
		SLIDER::RD rangeData = {
			L"Fast", 0,
			L"Slow", 3000
		};
		int thumbIndex = static_cast<int>(ap_dialog->m_modelData.fadeTimer / ticInterval);
		std::wostringstream out;
		out.precision(1);

		std::vector<std::wstring> ticIntervalTitle;
		for (int i = rangeData.min; i <= rangeData.max; i += ticInterval) {
			out << std::fixed << i / 1000.0f;
			ticIntervalTitle.push_back(out.str() + L"s");
			out.str(L"");
		}

		SliderDialog instanceDialog(L"Fade Timer", rangeData, ticInterval, thumbIndex, ticIntervalTitle);
		instanceDialog.SetColorMode(ap_dialog->m_colorMode);

		RECT rect;
		::GetWindowRect(ap_dialog->mh_window, &rect);
		const int centerPosX = rect.left + (rect.right - rect.left) / 2;
		const int centerPosY = rect.top + (rect.bottom - rect.top) / 2;
		const SIZE size = instanceDialog.GetSize();

		if (BT::OK == instanceDialog.DoModal(ap_dialog->mh_window, centerPosX - size.cx / 2, centerPosY - size.cy / 2)) {
			ap_dialog->m_modelData.fadeTimer = instanceDialog.GetValue();
		}
	};

	////////////////////////////////////////////////////////////////
	// implementation
	////////////////////////////////////////////////////////////////

	switch (a_menuID)
	{
	case MENU_SELECT_SCREEN:
		OnClickSelectScreenMenu(this);
		break;
	case MENU_COLOR_OPACITY:
		OnClickColorOpacityMenu(this);
		break;
	case MENU_FADE_SPEED:
		OnClickFadeSpeedMenu(this);
		break;
	default:
		break;
	}

	return WindowDialog::SysCommandHandler(a_menuID, a_longParam);
}

std::wstring WindowBrushDialog::GetHoverButtonTitle()
{
	const std::map<WINDOW_BRUSH::BT, std::wstring> titleList = {
		{WINDOW_BRUSH::BT::CURVE, L"Curve Tool (L)"},
		{WINDOW_BRUSH::BT::RECTANGLE, L"Rectangle Tool (R)"},
		{WINDOW_BRUSH::BT::CIRCLE, L"Ellipse Tool (E)"},
		{WINDOW_BRUSH::BT::TEXT, L"Text Tool (T)"},
		{WINDOW_BRUSH::BT::STROKE, L"Width Tool (W)"},
		{WINDOW_BRUSH::BT::GRADIENT, L"Gradation Tool (G)"},
		{WINDOW_BRUSH::BT::COLOR, L"Color Tool (C)"},
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

void WindowBrushDialog::DestroySketchDialog()
{
	mp_sketchDialog->DestroyWindow();

	delete mp_sketchDialog;
	mp_sketchDialog = nullptr;
}