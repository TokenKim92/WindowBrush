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
	AddMessageHandler(SKETCH::WM_UPDATEMD, static_cast<MessageHandler>(&SketchDialog::UpdateModelDataHandler));

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
	static_cast<SketchView *>(mp_direct2d)->Paint(m_modelDataList);
}

// to handle the WM_MOUSEMOVE message that occurs when a window is destroyed
int SketchDialog::MouseMoveHandler(WPARAM a_wordParam, LPARAM a_longParam)
{
	static const auto UpdateDrawDataPerType = [](SKETCH::MD &a_data, const POINT &a_point)
	{
		if (WINDOW_BRUSH::DT::CURVE == a_data.drawType) {
			a_data.points.push_back({ static_cast<float>(a_point.x), static_cast<float>(a_point.y) });
		}
		else if (WINDOW_BRUSH::DT::RECTANGLE == a_data.drawType) {
			a_data.rect.right = static_cast<float>(a_point.x);
			a_data.rect.bottom = static_cast<float>(a_point.y);
		}
		else if (WINDOW_BRUSH::DT::CIRCLE == a_data.drawType) {

			a_data.rect.right = static_cast<float>(a_point.x);
			a_data.rect.bottom = static_cast<float>(a_point.y);
		}
	};

	////////////////////////////////////////////////////////////////
	// implementation
	////////////////////////////////////////////////////////////////
	if (!m_leftButtonDown) {
		return S_OK;
	}

	const POINT point = { LOWORD(a_longParam), HIWORD(a_longParam) };
	
	UpdateDrawDataPerType(m_modelDataList.back(), point);
	Invalidate();

	return S_OK;
}

// to handle the WM_LBUTTONDOWN message that occurs when a window is destroyed
int SketchDialog::MouseLeftButtonDownHandler(WPARAM a_wordParam, LPARAM a_longParam)
{
	static const auto GetDefaultData = [](SketchDialog *const ap_dialog)
	{
		SKETCH::DD data;

		data.strokeWidth = ap_dialog->m_windowBrushModelData.strokeWidth;
		data.transparency = ap_dialog->m_windowBrushModelData.colorOpacity;
		data.color = ap_dialog->m_windowBrushModelData.selectedColor;
		data.gradientBrushIndex = ap_dialog->m_windowBrushModelData.isGradientMode
			? rand() % SKETCH::GRADIENT_BRUSH_COUNT
			: SKETCH::INVALID_INDEX;

		return data;
	};
	static const auto InitDrawDataPerType = [](SKETCH::MD &a_data, const POINT &a_point)
	{
		if (WINDOW_BRUSH::DT::CURVE == a_data.drawType) {
			a_data.points.push_back({ static_cast<float>(a_point.x), static_cast<float>(a_point.y) });
		}
		else if (WINDOW_BRUSH::DT::RECTANGLE == a_data.drawType) {
			a_data.rect = {
				static_cast<float>(a_point.x), static_cast<float>(a_point.y),
				static_cast<float>(a_point.x), static_cast<float>(a_point.y) 
			};
		}
		else if (WINDOW_BRUSH::DT::CIRCLE == a_data.drawType) {
			a_data.rect = {
				static_cast<float>(a_point.x), static_cast<float>(a_point.y),
				static_cast<float>(a_point.x), static_cast<float>(a_point.y)
			};
		}
	};

	////////////////////////////////////////////////////////////////
	// implementation
	////////////////////////////////////////////////////////////////

	if (m_leftButtonDown) {
		return S_OK;
	}

	::SetCapture(mh_window);

	const POINT point = { LOWORD(a_longParam), HIWORD(a_longParam) };
	m_leftButtonDown = true;
	m_previouseMilliseconds = ::GetTickCount64();

	SKETCH::MD data;
	data.drawType = m_windowBrushModelData.drawType;
	data.defaultData = GetDefaultData(this);
	InitDrawDataPerType(data, point);

	m_modelDataList.push_back(data);
	Invalidate();

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

int SketchDialog::UpdateModelDataHandler(WPARAM a_wordParam, LPARAM a_longParam)
{
	m_windowBrushModelData = *reinterpret_cast<WINDOW_BRUSH::MD *>(a_wordParam);

	return S_OK;
}

void SketchDialog::UpdateWindowBrushModelData(const WINDOW_BRUSH::MD *ap_modelData)
{
	::PostMessage(mh_window, SKETCH::WM_UPDATEMD, reinterpret_cast<WPARAM>(ap_modelData), 0);
}