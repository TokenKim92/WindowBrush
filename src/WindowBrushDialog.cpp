#include "ColorPalette.h"
#include "WindowBrushDialog.h"

#ifdef _DEBUG
#pragma comment (lib, "AppTemplateDebug.lib")
#else
#pragma comment (lib, "AppTemplate.lib")     
#endif

WindowBrush::WindowBrush() :
	WindowDialog(L"WINDOWBRUSH", L"")
{
	memset(&m_viewRect, 0, sizeof(RECT));
	m_buttonShapeData.hoverArea = BST::NONE;
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
			DRect({ margin, buttonSize * index, viewRect.right - margin, buttonSize * (index + 1) })
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
	::GetClientRect(mh_window, &m_viewRect);

	InitButtonRects();
	InitDivider();

	mp_buttonsShape = std::make_unique<ButtonShape>(mp_direct2d, m_buttonTable, GetThemeMode());

	mp_direct2d->SetStrokeWidth(2.5f);

	// add message handlers
	AddMessageHandler(WM_MOUSEMOVE, static_cast<MessageHandler>(&WindowBrush::MouseMoveHandler));
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

}

// to handle the WM_MOUSEMOVE message that occurs when a window is destroyed
int WindowBrush::MouseMoveHandler(WPARAM a_wordParam, LPARAM a_longParam)
{
	const POINT pos = { LOWORD(a_longParam), HIWORD(a_longParam) };

	for (auto const &[type, rect] : m_buttonTable) {
		if (PointInRectF(rect, pos)) {
			if (type != m_buttonShapeData.hoverArea) {
				m_buttonShapeData.hoverArea = type;
				::InvalidateRect(mh_window, &m_viewRect, true);
			}

			return S_OK;
		}
	}

	if (BST::NONE != m_buttonShapeData.hoverArea) {
		m_buttonShapeData.hoverArea = BST::NONE;
		::InvalidateRect(mh_window, &m_viewRect, false);
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
