#include "WindowBrushDialog.h"
#include "WindowBrushView.h"
#include "ColorDialog.h"
#include "ColorPalette.h"
#include "Utility.h"

#ifdef _DEBUG
#pragma comment (lib, "AppTemplateDebug.lib")
#else
#pragma comment (lib, "AppTemplate.lib")     
#endif

WindowBrushDialog::WindowBrushDialog() :
	WindowDialog(L"WINDOWBRUSH", L"")
{
	SetSize(80, 390);

	m_modelData.hoverArea = WBBT::NONE;
	m_modelData.drawMode = WBBT::NONE;
	m_modelData.isGradientMode = false;
	m_modelData.isFadeMode = false;
	m_modelData.selectedColor = RGB_TO_COLORF(ORANGE_500);
}

WindowBrushDialog::~WindowBrushDialog()
{

}

void WindowBrushDialog::OnInitDialog()
{
	DisableMaximize();
	DisableMinimize();
	DisableSize();

	// add message handlers
	AddMessageHandler(WM_MOUSEMOVE, static_cast<MessageHandler>(&WindowBrushDialog::MouseMoveHandler));
	AddMessageHandler(WM_LBUTTONUP, static_cast<MessageHandler>(&WindowBrushDialog::MouseLeftButtonUpHandler));

	const auto p_view = new WindowBrushView(mh_window, GetThemeMode());
	InheritDirect2D(p_view);
	p_view->Create();
	p_view->UpdateColorSymbolBrush(m_modelData.selectedColor);
	m_buttonTable = p_view->GetButtonTable();
}

void WindowBrushDialog::OnDestroy()
{

}

void WindowBrushDialog::OnPaint()
{
	static_cast<WindowBrushView *>(mp_direct2d)->Paint(m_modelData);
}

void WindowBrushDialog::OnSetThemeMode()
{
	static_cast<WindowBrushView *>(mp_direct2d)->SetColorMode(GetThemeMode());
	Invalidate();
}

// to handle the WM_MOUSEMOVE message that occurs when a window is destroyed
int WindowBrushDialog::MouseMoveHandler(WPARAM a_wordParam, LPARAM a_longParam)
{
	const POINT pos = { LOWORD(a_longParam), HIWORD(a_longParam) };

	for (auto const &[type, rect] : m_buttonTable) {
		if (PointInRect(rect, pos)) {
			if (type != m_modelData.hoverArea) {
				m_modelData.hoverArea = type;
				Invalidate();
			}

			return S_OK;
		}
	}

	if (WBBT::NONE != m_modelData.hoverArea) {
		m_modelData.hoverArea = WBBT::NONE;
		Invalidate();
	}

	return S_OK;
}

// to handle the WM_LBUTTONDOWN  message that occurs when a window is destroyed
int WindowBrushDialog::MouseLeftButtonDownHandler(WPARAM a_wordParam, LPARAM a_longParam)
{
	const POINT pos = { LOWORD(a_longParam), HIWORD(a_longParam) };

	return S_OK;
}

// to handle the WM_LBUTTONUP  message that occurs when a window is destroyed
int WindowBrushDialog::MouseLeftButtonUpHandler(WPARAM a_wordParam, LPARAM a_longParam)
{
	static const auto OnDrawButtonUp = [](WBMD &a_buttonShapeData, const WBBT &a_type)
	{
		if (a_type != a_buttonShapeData.drawMode) {
			// TODO:: turn on draw mode
			a_buttonShapeData.drawMode = a_type;
		}
		else {
			// TODO:: turn off draw mode
			a_buttonShapeData.drawMode = WBBT::NONE;;
		}
	};
	static const auto OnColorButtonUp = [](const HWND &ah_parentWindow, WBMD &a_buttonShapeData, const CM &a_mode, Direct2DEx *const ap_direct2d)
	{
		RECT rect;
		::GetWindowRect(ah_parentWindow, &rect);

		const int centerPosX = rect.left + (rect.right - rect.left) / 2;
		const int centerPosY = rect.top + (rect.bottom - rect.top) / 2;

		std::vector<DColor> colorList;
		ColorDialog instanceDialog(a_buttonShapeData.selectedColor, colorList);
		instanceDialog.SetStyle(WS_POPUP | WS_VISIBLE);
		instanceDialog.SetExtendStyle(WS_EX_TOPMOST);
		instanceDialog.SetThemeMode(a_mode);

		const SIZE size = instanceDialog.GetSize();
		instanceDialog.DoModal(ah_parentWindow, centerPosX - size.cx / 2, centerPosY - size.cy / 2);
		a_buttonShapeData.selectedColor = instanceDialog.GetSelectedColor();

		static_cast<WindowBrushView *>(ap_direct2d)->UpdateColorSymbolBrush(a_buttonShapeData.selectedColor);
	};
	static const auto OnGradientButtonUp = [](WBMD &a_buttonShapeData)
	{
		a_buttonShapeData.isGradientMode = !a_buttonShapeData.isGradientMode;
	};
	static const auto OnFadeButtonUp = [](WBMD &a_buttonShapeData)
	{
		a_buttonShapeData.isFadeMode = !a_buttonShapeData.isFadeMode;
	};

	////////////////////////////////////////////////////////////////
	// implementation
	////////////////////////////////////////////////////////////////

	const POINT pos = { LOWORD(a_longParam), HIWORD(a_longParam) };

	// check first click area on draw mode buttons
	for (auto const &[type, rect] : m_buttonTable) {
		if (PointInRect(rect, pos)) {
			switch (type)
			{
			case WBBT::CURVE:
			case WBBT::RECTANGLE:
			case WBBT::CIRCLE:
			case WBBT::TEXT:
				OnDrawButtonUp(m_modelData, type);
				break;
			case WBBT::STROKE:
				break;
			case WBBT::GRADIATION:
				OnGradientButtonUp(m_modelData);
				break;
			case WBBT::COLOR:
				OnColorButtonUp(mh_window, m_modelData, GetThemeMode(), mp_direct2d);
				break;
			case WBBT::FADE:
				OnFadeButtonUp(m_modelData);
				break;
			default:
				break;
			}

			Invalidate();

			return S_OK;
		}
	}

	return S_OK;
}
