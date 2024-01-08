#include "ColorPalette.h"
#include "WindowBrushDialog.h"
#include "ColorDialog.h"

#ifdef _DEBUG
#pragma comment (lib, "AppTemplateDebug.lib")
#else
#pragma comment (lib, "AppTemplate.lib")     
#endif

WindowBrush::WindowBrush() :
	WindowDialog(L"WINDOWBRUSH", L"")
{
	SetSize(80, 390);

	m_buttonShapeData.hoverArea = BST::NONE;
	m_buttonShapeData.drawMode = BST::NONE;
	m_buttonShapeData.isGradientMode = false;

	m_selectedColor = RGB_TO_COLORF(BLUE_500);
}

WindowBrush::~WindowBrush()
{

}

void WindowBrush::InitButtonRects()
{
	const float margin = 10.0f;
	const float buttonSize = m_viewRect.right - m_viewRect.left - margin * 2.0f;
	const size_t buttonCount = 8;

	std::vector<BST> buttonShapeList = {
		BST::CURVE,
		BST::RECTANGLE,
		BST::CIRCLE,
		BST::TEXT,
		BST::STROKE,
		BST::GRADIATION,
		BST::COLOR,
		BST::FADE
	};

	size_t index = 0;
	for (const auto &tpye : buttonShapeList) {
		m_buttonTable.insert({
			tpye,
			DRect({ margin, buttonSize * index, m_viewRect.right - margin, buttonSize * (index + 1) })
		});

		index++;
	}

	const float fadeRectOffet = 3.0f;
	DRect &rect = m_buttonTable.at(BST::FADE);
	rect.top += fadeRectOffet;
	rect.bottom += fadeRectOffet;
}

void WindowBrush::InitDivider()
{
	auto AddDividerRect = [](std::vector<DRect> &a_divierList, const DRect &a_rect)
	{
		a_divierList.push_back(DRect({ a_rect.left, a_rect.bottom, a_rect.right, a_rect.bottom }));
	};

	AddDividerRect(m_dividerList, m_buttonTable.at(BST::TEXT));
	AddDividerRect(m_dividerList, m_buttonTable.at(BST::STROKE));
	AddDividerRect(m_dividerList, m_buttonTable.at(BST::COLOR));
}

void WindowBrush::OnInitDialog()
{
	DisableMaximize();
	DisableMinimize();
	DisableSize();

	InitButtonRects();
	InitDivider();

	mp_buttonsShape = std::make_unique<ButtonShape>(mp_direct2d, m_buttonTable, GetThemeMode());

	mp_direct2d->SetStrokeWidth(2.5f);

	// add message handlers
	AddMessageHandler(WM_MOUSEMOVE, static_cast<MessageHandler>(&WindowBrush::MouseMoveHandler));
	AddMessageHandler(WM_LBUTTONUP, static_cast<MessageHandler>(&WindowBrush::MouseLeftButtonUpHandler));
}

void WindowBrush::OnDestroy()
{

}


void WindowBrush::OnPaint()
{
	mp_direct2d->Clear();

	for (auto const &[type, rect] : m_buttonTable) {
		mp_buttonsShape->DrawButton(type, m_buttonShapeData);
	}

	mp_direct2d->SetBrushColor(RGB_TO_COLORF(NEUTRAL_300));
	for (auto const &divier : m_dividerList) {
		mp_direct2d->DrawRectangle(divier);
	}
}

void WindowBrush::OnSetThemeMode()
{
	const auto colorMode = GetThemeMode();
	DColor backgroundColor;
	if (THEME_MODE::DARK_MODE == colorMode) {
		backgroundColor = RGB_TO_COLORF(NEUTRAL_800);
	}
	else {
		backgroundColor = RGB_TO_COLORF(NEUTRAL_100);
	}
	
	mp_buttonsShape->SetColorMode(colorMode);
	mp_direct2d->SetBackgroundColor(backgroundColor);

	Invalidate();
}

// to handle the WM_MOUSEMOVE message that occurs when a window is destroyed
int WindowBrush::MouseMoveHandler(WPARAM a_wordParam, LPARAM a_longParam)
{
	const POINT pos = { LOWORD(a_longParam), HIWORD(a_longParam) };

	for (auto const &[type, rect] : m_buttonTable) {
		if (PointInRect(rect, pos)) {
			if (type != m_buttonShapeData.hoverArea) {
				m_buttonShapeData.hoverArea = type;
				Invalidate();
			}

			return S_OK;
		}
	}

	if (BST::NONE != m_buttonShapeData.hoverArea) {
		m_buttonShapeData.hoverArea = BST::NONE;
		Invalidate();
	}

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
	static auto OnDrawButtonUp = [](BSD &a_buttonShapeData, const BST &a_type)
	{
		if (a_type != a_buttonShapeData.drawMode) {
			// TODO:: turn on draw mode
			a_buttonShapeData.drawMode = a_type;
		}
		else {
			// TODO:: turn off draw mode
			a_buttonShapeData.drawMode = BST::NONE;;
		}
	};
	static auto OnColorButtonUp = [](const HWND &ah_parentWindow, DColor &a_selectedColor, const THEME_MODE &a_mode, const std::unique_ptr<ButtonShape> &ap_buttonShape)
	{
		RECT rect;
		::GetWindowRect(ah_parentWindow, &rect);

		const int centerPosX = rect.left + (rect.right - rect.left) / 2;
		const int centerPosY = rect.top + (rect.bottom - rect.top) / 2;

		std::vector<DColor> colorList;
		ColorDialog instanceDialog(a_selectedColor, colorList);
		instanceDialog.SetStyle(WS_POPUP | WS_VISIBLE);
		instanceDialog.SetExtendStyle(WS_EX_TOPMOST);
		instanceDialog.SetThemeMode(a_mode);

		const SIZE size = instanceDialog.GetSize();
		instanceDialog.DoModal(ah_parentWindow, centerPosX - size.cx / 2, centerPosY - size.cy / 2);
		a_selectedColor = instanceDialog.GetSelectedColor();

		ap_buttonShape->UpdateColorSymbolBrush(a_selectedColor);
	};
	static auto OnGradientButtonUp = [](BSD &a_buttonShapeData)
	{
		a_buttonShapeData.isGradientMode = !a_buttonShapeData.isGradientMode;
	};
	static auto OnFadeButtonUp = [](BSD &a_buttonShapeData)
	{
		a_buttonShapeData.isFadeMode = !a_buttonShapeData.isFadeMode;
	};

	const POINT pos = { LOWORD(a_longParam), HIWORD(a_longParam) };

	// check first click area on draw mode buttons
	for (auto const &[type, rect] : m_buttonTable) {
		if (PointInRect(rect, pos)) {
			switch (type)
			{
			case BST::CURVE:
			case BST::RECTANGLE:
			case BST::CIRCLE:
			case BST::TEXT:
				OnDrawButtonUp(m_buttonShapeData, type);
				break;
			case BST::STROKE:
				break;
			case BST::GRADIATION:
				OnGradientButtonUp(m_buttonShapeData);
				break;
			case BST::COLOR:
				OnColorButtonUp(mh_window, m_selectedColor, GetThemeMode(), mp_buttonsShape);
				break;
			case BST::FADE:
				OnFadeButtonUp(m_buttonShapeData);
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
