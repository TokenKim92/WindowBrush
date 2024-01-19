#include "SketchDialog.h"
#include "SketchView.h"
#include "Utility.h"
#include "time.h"

SketchDialog::SketchDialog(const WINDOW_BRUSH::MD &a_modelData, const RECT &a_scaledRect) :
	WindowDialog(L"SKETCHDIALOG", L"SketchDialog"),
	m_parentModelData(a_modelData),
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
	m_previousMilliseconds = 0;
	mh_screenBitmap = GetScreenHBitmap(a_scaledRect, a_modelData.selectedScreenRect);
	mh_edit = nullptr;

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
	if (nullptr != mh_edit) {
		::DestroyWindow(mh_edit);
	}
	::DeleteObject(mh_screenBitmap);
}

void SketchDialog::OnInitDialog()
{
	DisableMaximize();
	DisableMinimize();
	DisableSize();

	const auto p_view = new SketchView(mh_window, mh_screenBitmap, m_parentModelData.selectedScreenRect, GetColorMode());
	InheritDirect2D(p_view);
	p_view->Create();

	mh_edit = ::CreateWindowEx(
		WS_EX_CLIENTEDGE, L"EDIT", nullptr, WS_CHILD, 0, 0, m_viewRect.right - m_viewRect.left, 20,
		mh_window, nullptr, nullptr, nullptr
	);
	::SetFocus(mh_edit);
}

void SketchDialog::OnPaint()
{
	mp_direct2d->Clear();
	static_cast<SketchView *>(mp_direct2d)->Paint(m_modelDataList);
}

// to handle the WM_MOUSEMOVE message that occurs when a window is destroyed
int SketchDialog::MouseMoveHandler(WPARAM a_wordParam, LPARAM a_longParam)
{
	if (!m_leftButtonDown) {
		return S_OK;
	}

	const POINT point = { LOWORD(a_longParam), HIWORD(a_longParam) };
	auto &latestData = m_modelDataList.back();

	if (WINDOW_BRUSH::DT::TEXT_OUTLINE == latestData.drawType) {
		return S_OK;
	}

	switch (latestData.drawType)
	{
	case WINDOW_BRUSH::DT::CURVE:
		latestData.points.push_back({ static_cast<float>(point.x), static_cast<float>(point.y) });
		break;
	case WINDOW_BRUSH::DT::RECTANGLE:
		latestData.rect.right = static_cast<float>(point.x);
		latestData.rect.bottom = static_cast<float>(point.y);
		break;
	case WINDOW_BRUSH::DT::CIRCLE:
		latestData.rect.right = static_cast<float>(point.x);
		latestData.rect.bottom = static_cast<float>(point.y);
		break;
	default:
		break;
	}

	Invalidate();

	return S_OK;
}

// to handle the WM_LBUTTONDOWN message that occurs when a window is destroyed
int SketchDialog::MouseLeftButtonDownHandler(WPARAM a_wordParam, LPARAM a_longParam)
{
	static const auto GetDefaultData = [](SketchDialog *const ap_dialog)
	{
		SKETCH::DD data;

		data.fontSize = ap_dialog->m_parentModelData.fontSize;
		data.strokeWidth = ap_dialog->m_parentModelData.strokeWidth;
		data.transparency = ap_dialog->m_parentModelData.colorOpacity;
		data.color = ap_dialog->m_parentModelData.selectedColor;
		data.gradientBrushIndex = ap_dialog->m_parentModelData.isGradientMode
			? rand() % SKETCH::GRADIENT_BRUSH_COUNT
			: SKETCH::INVALID_INDEX;

		return data;
	};
	static const auto IsDrawingTextOutline = [](SketchDialog *const ap_dialog)
	{
		return ap_dialog->m_modelDataList.size() && WINDOW_BRUSH::DT::TEXT_OUTLINE == ap_dialog->m_modelDataList.back().drawType;
	};
	static const auto CancelTypingText = [](SketchDialog *const ap_dialog)
	{
		ap_dialog->m_modelDataList.pop_back();
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
	m_previousMilliseconds = ::GetTickCount64();

	if (IsDrawingTextOutline(this)) {
		CancelTypingText(this);
	}

	SKETCH::MD data;
	data.drawType = m_parentModelData.drawType;
	data.defaultData = GetDefaultData(this);

	switch (data.drawType)
	{
	case WINDOW_BRUSH::DT::CURVE:
		data.points.push_back({ static_cast<float>(point.x), static_cast<float>(point.y) });
		m_modelDataList.push_back(data);
		Invalidate();
		break;
	case WINDOW_BRUSH::DT::RECTANGLE:
		data.rect = {
			static_cast<float>(point.x), static_cast<float>(point.y),
			static_cast<float>(point.x), static_cast<float>(point.y)
		};
		m_modelDataList.push_back(data);
		Invalidate();
		break;
	case WINDOW_BRUSH::DT::CIRCLE:
		data.rect = {
			static_cast<float>(point.x), static_cast<float>(point.y),
			static_cast<float>(point.x), static_cast<float>(point.y)
		};
		m_modelDataList.push_back(data);
		Invalidate();
		break;
	case WINDOW_BRUSH::DT::TEXT_OUTLINE:
		m_modelDataList.push_back(data);
		::SetWindowText(mh_edit, L"");
		break;
	default:
		break;
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

	auto &data = m_modelDataList.back();
	if (WINDOW_BRUSH::DT::TEXT_OUTLINE == data.drawType) {
		data.points.push_back({ static_cast<float>(point.x), static_cast<float>(point.y) });

		Invalidate();
	}

	return S_OK;
}

// to handle the WM_KEYDOWN message that occurs when a window is destroyed
int SketchDialog::KeyDownHandler(WPARAM a_wordParam, LPARAM a_longParam)
{
	const unsigned char pressedKey = static_cast<unsigned char>(a_wordParam);

	if (VK_RETURN == pressedKey) {

	}
	else if (VK_ESCAPE == pressedKey) {

	}

	return S_OK;
}

int SketchDialog::UpdateModelDataHandler(WPARAM a_wordParam, LPARAM a_longParam)
{
	m_parentModelData = *reinterpret_cast<WINDOW_BRUSH::MD *>(a_wordParam);

	return S_OK;
}

void SketchDialog::UpdateWindowBrushModelData(const WINDOW_BRUSH::MD *ap_modelData)
{
	::PostMessage(mh_window, SKETCH::WM_UPDATEMD, reinterpret_cast<WPARAM>(ap_modelData), 0);
}