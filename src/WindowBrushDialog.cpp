#include "WindowBrushDialog.h"
#include "ColorPalette.h"

#ifdef _DEBUG
#pragma comment (lib, "AppTemplateDebug.lib")
#else
#pragma comment (lib, "AppTemplate.lib")     
#endif

WindowBrush::WindowBrush() :
	WindowDialog(L"WINDOWBRUSH", L"WindowBrush")
{
	memset(&m_viewRect, 0, sizeof(RECT));
}

WindowBrush::~WindowBrush()
{
}

void WindowBrush::OnInitDialog()
{
	DisableMaximize();
	DisableMinimize();
	DisableSize();

	::GetClientRect(mh_window, &m_viewRect);
}

void WindowBrush::OnDestroy()
{

}

void WindowBrush::OnPaint()
{
	mp_direct2d->Clear();
}

void WindowBrush::OnSetThemeMode()
{

}

// to handle the WM_MOUSEMOVE message that occurs when a window is destroyed
int WindowBrush::MouseMoveHandler(WPARAM a_wordParam, LPARAM a_longParam)
{
	const POINT pos = { LOWORD(a_longParam), HIWORD(a_longParam) };

	return S_OK;
}

// to handle the WM_LBUTTONDOWN  message that occurs when a window is destroyed
int WindowBrush::MouseLeftButtonDownHandler(WPARAM a_wordParam, LPARAM a_longParam)
{
	const POINT pos = { LOWORD(a_longParam), HIWORD(a_longParam) };

	return S_OK;
}

// to handle the WM_LBUTTONUP  message that occurs when a window is destroyed
int WindowBrush::MouseLeftButtonUpHandler(WPARAM a_wordParam, LPARAM a_longParam)
{
	const POINT pos = { LOWORD(a_longParam), HIWORD(a_longParam) };

	return S_OK;
}

// to handle the WM_MOUSEWHEEL  message that occurs when a window is destroyed
int WindowBrush::MouseWheelHandler(WPARAM a_wordParam, LPARAM a_longParam)
{
	short delta = GET_WHEEL_DELTA_WPARAM(a_wordParam);

	if (delta < 0 ) {
	}
	else {
	}

	return S_OK;
}
