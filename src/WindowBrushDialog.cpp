#include "WindowBrushDialog.h"
#include "WindowBrushView.h"
#include "ColorDialog.h"
#include "EditDialog.h"
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

	m_modelData.hoverArea = WINDOW_BRUSH::BT::NONE;
	m_modelData.drawMode = WINDOW_BRUSH::BT::NONE;
	m_modelData.strokeWidth = 20;
	m_modelData.fontSize = 20;
	m_modelData.isGradientMode = false;
	m_modelData.selectedColor = RGB_TO_COLORF(ORANGE_500);
	m_modelData.isFadeMode = false;
}

WindowBrushDialog::~WindowBrushDialog()
{

}
#include <vector>
void WindowBrushDialog::OnInitDialog()
{
	DisableMaximize();
	DisableMinimize();
	DisableSize();

	// add message handlers
	AddMessageHandler(WM_MOUSEMOVE, static_cast<MessageHandler>(&WindowBrushDialog::MouseMoveHandler));
	AddMessageHandler(WM_LBUTTONUP, static_cast<MessageHandler>(&WindowBrushDialog::MouseLeftButtonUpHandler));

	const auto p_view = new WindowBrushView(mh_window, GetColorMode());
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
	static_cast<WindowBrushView *>(mp_direct2d)->SetColorMode(GetColorMode());
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

	if (WINDOW_BRUSH::BT::NONE != m_modelData.hoverArea) {
		m_modelData.hoverArea = WINDOW_BRUSH::BT::NONE;
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
	static const auto OnDrawButtonUp = [](WINDOW_BRUSH::MD &a_buttonShapeData, const WINDOW_BRUSH::BT &a_type)
	{
		if (a_type != a_buttonShapeData.drawMode) {
			// TODO:: turn on draw mode
			a_buttonShapeData.drawMode = a_type;
		}
		else {
			// TODO:: turn off draw mode
			a_buttonShapeData.drawMode = WINDOW_BRUSH::BT::NONE;;
		}
	};
	static const auto OnStrokeButtonUp = [](const HWND &ah_parentWindow, const CM &a_colorMode, WINDOW_BRUSH::MD &a_modelData)
	{
		RECT rect;
		::GetWindowRect(ah_parentWindow, &rect);

		const int centerPosX = rect.left + (rect.right - rect.left) / 2;
		const int centerPosY = rect.top + (rect.bottom - rect.top) / 2;

		const std::vector<std::pair<std::wstring, unsigned int>> itemList = {
			{ L"stroke width(px)", a_modelData.strokeWidth },
			{ L"font size(px)", a_modelData.fontSize }
		};

		EditDialog instanceDialog(L"Stroke Width", itemList, EDIT::RANGE({ 1, 999 }));
		instanceDialog.SetStyle(WS_POPUP | WS_VISIBLE);
		instanceDialog.SetExtendStyle(WS_EX_TOPMOST);
		instanceDialog.SetThemeMode(a_colorMode);

		const SIZE size = instanceDialog.GetSize();
		if (BT::OK == instanceDialog.DoModal(ah_parentWindow, centerPosX - size.cx / 2, centerPosY - size.cy / 2)) {
			auto valueList = instanceDialog.GetValueList();
			a_modelData.strokeWidth = valueList.at(0);
			a_modelData.fontSize = valueList.at(1);
		}
	};
	static const auto OnColorButtonUp = [](
		const HWND &ah_parentWindow, WINDOW_BRUSH::MD &a_buttonShapeData, const CM &a_colorMode,
		Direct2DEx *const ap_direct2d, std::vector<DColor> &a_colorList
		)
	{
		RECT rect;
		::GetWindowRect(ah_parentWindow, &rect);

		const int centerPosX = rect.left + (rect.right - rect.left) / 2;
		const int centerPosY = rect.top + (rect.bottom - rect.top) / 2;

		ColorDialog instanceDialog(a_buttonShapeData.selectedColor, a_colorList);
		instanceDialog.SetStyle(WS_POPUP | WS_VISIBLE);
		instanceDialog.SetExtendStyle(WS_EX_TOPMOST);
		instanceDialog.SetThemeMode(a_colorMode);

		const SIZE size = instanceDialog.GetSize();
		if (BT::OK == instanceDialog.DoModal(ah_parentWindow, centerPosX - size.cx / 2, centerPosY - size.cy / 2)) {
			a_buttonShapeData.selectedColor = instanceDialog.GetSelectedColor();
			a_colorList = instanceDialog.GetColorList();
			static_cast<WindowBrushView *>(ap_direct2d)->UpdateColorSymbolBrush(a_buttonShapeData.selectedColor);
		}
	};
	static const auto OnGradientButtonUp = [](WINDOW_BRUSH::MD &a_buttonShapeData)
	{
		a_buttonShapeData.isGradientMode = !a_buttonShapeData.isGradientMode;
	};
	static const auto OnFadeButtonUp = [](WINDOW_BRUSH::MD &a_buttonShapeData)
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
			case WINDOW_BRUSH::BT::CURVE:
			case WINDOW_BRUSH::BT::RECTANGLE:
			case WINDOW_BRUSH::BT::CIRCLE:
			case WINDOW_BRUSH::BT::TEXT:
				OnDrawButtonUp(m_modelData, type);
				break;
			case WINDOW_BRUSH::BT::STROKE:
				OnStrokeButtonUp(mh_window, GetColorMode(), m_modelData);
				break;
			case WINDOW_BRUSH::BT::GRADIATION:
				OnGradientButtonUp(m_modelData);
				break;
			case WINDOW_BRUSH::BT::COLOR:
				if (!m_modelData.isGradientMode) {
					OnColorButtonUp(mh_window, m_modelData, GetColorMode(), mp_direct2d, m_colorList);
				}
				return S_OK; // to avoid hoverArea is init
			case WINDOW_BRUSH::BT::FADE:
				OnFadeButtonUp(m_modelData);
				break;
			default:
				break;
			}

			m_modelData.hoverArea = WINDOW_BRUSH::BT::NONE;
			Invalidate();

			return S_OK;
		}
	}

	return S_OK;
}
