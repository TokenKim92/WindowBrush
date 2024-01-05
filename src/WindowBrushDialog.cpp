#include "WindowBrushDialog.h"
#include "ColorPalette.h"
#include <vector>

#ifdef _DEBUG
#pragma comment (lib, "AppTemplateDebug.lib")
#else
#pragma comment (lib, "AppTemplate.lib")     
#endif

WindowBrush::WindowBrush() :
	WindowDialog(L"WINDOWBRUSH", L"")
{
	memset(&m_viewRect, 0, sizeof(RECT));
}

WindowBrush::~WindowBrush()
{

}

void WindowBrush::InitButtonRects()
{
	RECT viewRect;
	::GetClientRect(mh_window, &viewRect);

	const float margin = 10.0f;
	const float buttonSize = viewRect.right - viewRect.left - margin * 2.0f;
	const size_t buttonCount = 8;

	std::vector<ButtonShape::TYPE> buttonShapeList = {
		ButtonShape::TYPE::CURVE,
		ButtonShape::TYPE::RECTANGLE,
		ButtonShape::TYPE::CIRCLE,
		ButtonShape::TYPE::TEXT,
		ButtonShape::TYPE::STROKE,
		ButtonShape::TYPE::GRADIATION,
		ButtonShape::TYPE::COLOR,
		ButtonShape::TYPE::FADE
	};

	size_t index = 0;
	for (const auto &tpye : buttonShapeList) {
		m_buttonTable.insert({
			tpye,
			DRect({ margin, buttonSize * index, viewRect.right - margin, buttonSize * (index + 1) })
		});

		index++;
	}
}

void WindowBrush::OnInitDialog()
{
	InitButtonRects();
	mp_buttonsShape = std::make_unique<ButtonShape>(mp_direct2d, m_buttonTable, GetThemeMode());

	mp_direct2d->SetStrokeWidth(2.5f);
}

void WindowBrush::OnDestroy()
{

}


void WindowBrush::OnPaint()
{
	mp_direct2d->Clear();


	for (auto const &[type, rect] : m_buttonTable) {
		mp_buttonsShape->DrawButton(type);
	}
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
