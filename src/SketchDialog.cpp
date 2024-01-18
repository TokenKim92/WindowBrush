#include "SketchDialog.h"
#include "SketchView.h"
#include "Utility.h"
#include "time.h"

SketchDialog::SketchDialog(const WINDOW_BRUSH::MD &a_modelData, const RECT &a_scaledRect) :
	WindowDialog(L"SKETCHDIALOG", L"SketchDialog"),
	m_windowBrushModelData(a_modelData),
	m_scaledRect(a_scaledRect)
{
	const auto GetScreenHBitmap = [](const RECT &a_scaledRect, const RECT &a_physicalRect)
	{
		const int bitmapWidth = a_physicalRect.right - a_physicalRect.left;
		const int bitmapHeight = a_physicalRect.bottom - a_physicalRect.top;

		HDC h_screenDC = ::GetWindowDC(nullptr);
		HDC h_tempDC = ::CreateCompatibleDC(h_screenDC);
		HBITMAP h_tempBitmap = ::CreateCompatibleBitmap(h_screenDC, bitmapWidth, bitmapHeight);
		::SelectObject(h_tempDC, h_tempBitmap);
		::SetStretchBltMode(h_tempDC, COLORONCOLOR);

		::BitBlt(
			h_tempDC,
			0, 0, bitmapWidth, bitmapHeight,
			h_screenDC,
			a_scaledRect.left, a_scaledRect.top,
			SRCCOPY
		);

		::DeleteDC(h_tempDC);
		::ReleaseDC(nullptr, h_screenDC);

		return h_tempBitmap;
	};

	///////////////////////////////////////////////////////////////////
	// implementation
	///////////////////////////////////////////////////////////////////

	SetSize(a_scaledRect.right - a_scaledRect.left, a_scaledRect.bottom - a_scaledRect.top);
	SetStyle(WS_POPUP | WS_VISIBLE);

	m_leftButtonDown = false;
	m_previouseMilliseconds = 0;
	mh_screenBitmap = GetScreenHBitmap(a_scaledRect, a_modelData.selectedScreenRect);

	// add message handlers
	AddMessageHandler(WM_MOUSEMOVE, static_cast<MessageHandler>(&SketchDialog::MouseMoveHandler));
	AddMessageHandler(WM_LBUTTONDOWN, static_cast<MessageHandler>(&SketchDialog::MouseLeftButtonDownHandler));
	AddMessageHandler(WM_LBUTTONUP, static_cast<MessageHandler>(&SketchDialog::MouseLeftButtonUpHandler));
	AddMessageHandler(WM_KEYDOWN, static_cast<MessageHandler>(&SketchDialog::KeyDownHandler));

	srand(static_cast<unsigned int>(time(NULL)));
}

SketchDialog::~SketchDialog()
{
	::DeleteObject(mh_screenBitmap);
}

void SketchDialog::OnInitDialog()
{
	DisableMaximize();
	DisableMinimize();
	DisableSize();

	const auto p_view = new SketchView(mh_window, mh_screenBitmap, m_windowBrushModelData.selectedScreenRect, GetColorMode());
	InheritDirect2D(p_view);
	p_view->Create();
}

void SketchDialog::OnPaint()
{
	mp_direct2d->Clear();
	static_cast<SketchView *>(mp_direct2d)->Paint(m_modelData);
}

// to handle the WM_MOUSEMOVE message that occurs when a window is destroyed
int SketchDialog::MouseMoveHandler(WPARAM a_wordParam, LPARAM a_longParam)
{
	if (!m_leftButtonDown) {
		return S_OK;
	}

	const POINT point = { LOWORD(a_longParam), HIWORD(a_longParam) };
	
	if (WINDOW_BRUSH::BT::CURVE == m_windowBrushModelData.drawMode) {
		auto &latestCurvaData = m_modelData.curveDataList.back();
		latestCurvaData.points.push_back({ static_cast<float>(point.x), static_cast<float>(point.y) });

		Invalidate();
	}

	return S_OK;
}

// to handle the WM_LBUTTONDOWN message that occurs when a window is destroyed
int SketchDialog::MouseLeftButtonDownHandler(WPARAM a_wordParam, LPARAM a_longParam)
{
	if (m_leftButtonDown) {
		return S_OK;
	}

	::SetCapture(mh_window);

	const POINT point = { LOWORD(a_longParam), HIWORD(a_longParam) };
	m_leftButtonDown = true;
	m_previouseMilliseconds = ::GetTickCount64();

	if (WINDOW_BRUSH::BT::CURVE == m_windowBrushModelData.drawMode) {
		SKETCH::CD data;
		data.points.push_back({ static_cast<float>(point.x), static_cast<float>(point.y) });
		data.strokeWidth = m_windowBrushModelData.strokeWidth;
		data.transparency = m_windowBrushModelData.colorOpacity;
		data.color = m_windowBrushModelData.selectedColor;
		data.gradientBrushIndex = m_windowBrushModelData.isGradientMode
			? rand() % SKETCH::GRADIENT_BRUSH_COUNT
			: SKETCH::INVALID_INDEX;

		m_modelData.curveDataList.push_back(data);

		Invalidate();
	}

	return S_OK;
}

// to handle the WM_LBUTTONUP message that occurs when a window is destroyed
int SketchDialog::MouseLeftButtonUpHandler(WPARAM a_wordParam, LPARAM a_longParam)
{
	if (!m_leftButtonDown) {
		return S_OK;
	}

	::ReleaseCapture();

	const POINT point = { LOWORD(a_longParam), HIWORD(a_longParam) };
	m_leftButtonDown = false;

	if (WINDOW_BRUSH::BT::CURVE == m_windowBrushModelData.drawMode) {
		
	}

	return S_OK;
}

// to handle the WM_KEYDOWN message that occurs when a window is destroyed
int SketchDialog::KeyDownHandler(WPARAM a_wordParam, LPARAM a_longParam)
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

void SketchDialog::UpdateWindowBrushModelData(const WINDOW_BRUSH::MD &a_modelData)
{
	m_windowBrushModelData = a_modelData;
}