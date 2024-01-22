#include "SketchDialog.h"
#include "SketchView.h"
#include "Utility.h"
#include "time.h"
#include <memory>
#include <algorithm>

SketchDialog::SketchDialog(const HWND &ah_parentWindow, const WINDOW_BRUSH::MD &a_modelData, const RECT &a_scaledRect) :
	WindowDialog(L"SKETCHDIALOG", L"SketchDialog"),
	mh_parentWindow(ah_parentWindow),
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
	AddMessageHandler(SKETCH::WM_UPDATE_MODEL_DATA, static_cast<MessageHandler>(&SketchDialog::UpdateModelDataHandler));
	AddMessageHandler(SKETCH::WM_SET_TEXTOUTLINE_MODE, static_cast<MessageHandler>(&SketchDialog::SetTextOutlineModeHandler));
	AddMessageHandler(SKETCH::WM_ON_EDIT_MAX_LEGNTH, static_cast<MessageHandler>(&SketchDialog::OnEditMaxLengthHandler));
	AddMessageHandler(WM_KEYDOWN, static_cast<MessageHandler>(&SketchDialog::KeyDownHandler));

	srand(static_cast<unsigned int>(time(NULL)));
}

SketchDialog::~SketchDialog()
{

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

	if (m_parentModelData.isFadeMode) {
		::SetTimer(mh_window, reinterpret_cast<UINT_PTR>(this), SKETCH::FPS_TIME, FadeObjectOnTimer);
	}
}

void SketchDialog::OnDestroy()
{
	if (m_parentModelData.isFadeMode) {
		::KillTimer(mh_window, reinterpret_cast<UINT_PTR>(this));
	}
	if (nullptr != mh_edit) {
		::DestroyWindow(mh_edit);
	}
	::DeleteObject(mh_screenBitmap);
}

void SketchDialog::OnQuit()
{
	::PostMessage(mh_parentWindow, WINDOW_BRUSH::WM_KILLED_SKETCH, 0, 0);
}

void SketchDialog::OnPaint()
{
	static const auto OnDrawTextMode = [](SketchDialog *const ap_dalog)
	{
		const auto textLegnth = ::GetWindowTextLength(ap_dalog->mh_edit) + 1;
		auto tempText = std::make_unique<wchar_t[]>(textLegnth);
		::GetWindowText(ap_dalog->mh_edit, tempText.get(), textLegnth);

		ap_dalog->m_modelDataList.back().text = tempText.get();
	};

	///////////////////////////////////////////////////////////////////
	// implementation
	///////////////////////////////////////////////////////////////////

	mp_direct2d->Clear();

	if (WINDOW_BRUSH::DT::TEXT_TYPING == m_parentModelData.drawType) {
		OnDrawTextMode(this);
	}

	static_cast<SketchView *>(mp_direct2d)->Paint(m_modelDataList);
}

void SketchDialog::PreTranslateMessage(MSG &a_msg)
{
	static const auto KeyDownToTextOutlineModel = [](SketchDialog *const ap_dialog, const MSG &a_msg)
	{
		if (VK_RETURN == a_msg.wParam || VK_ESCAPE == a_msg.wParam) {
			ap_dialog->m_modelDataList.pop_back();
		}
		else {
			ap_dialog->m_parentModelData.drawType = WINDOW_BRUSH::DT::TEXT_TYPING;
			ap_dialog->m_modelDataList.back().drawType = WINDOW_BRUSH::DT::TEXT_TYPING;

			::SetFocus(ap_dialog->mh_edit);
			::PostMessage(ap_dialog->mh_edit, WM_KEYDOWN, a_msg.wParam, a_msg.lParam);
		}
		ap_dialog->Invalidate();
	};
	static const auto OnKeyDown = [](SketchDialog *const ap_dialog, const MSG &a_msg)
	{
		if (WINDOW_BRUSH::DT::TEXT_OUTLINE == ap_dialog->m_parentModelData.drawType) {
			if (0 != ap_dialog->m_modelDataList.size()) {
				const SKETCH::MD &modelData = ap_dialog->m_modelDataList.back();
				if (WINDOW_BRUSH::DT::TEXT_OUTLINE == modelData.drawType) {
					KeyDownToTextOutlineModel(ap_dialog, a_msg);

					return;
				}
			}
		}
		else if (WINDOW_BRUSH::DT::TEXT_TYPING == ap_dialog->m_parentModelData.drawType) {
			if (a_msg.wParam == VK_RETURN) {
				ap_dialog->SetTextOutlineModeHandler(0, true);
			}
			else if (a_msg.wParam == VK_ESCAPE) {
				ap_dialog->SetTextOutlineModeHandler(0, false);
			}
			ap_dialog->Invalidate();

			return;
		}

		if (VK_ESCAPE == a_msg.wParam) {
			::DestroyWindow(ap_dialog->mh_window);
		}
	};
	static const auto OnMultiKeyDown = [](SketchDialog *const ap_dialog)
	{
		static const auto IsControlKeyDown = []()
		{
			return GetKeyState(VK_CONTROL) & 0x8000;
		};
		static const auto IsZKeyDown = []()
		{
			return GetKeyState('Z') & 0x8000;
		};

		if (IsControlKeyDown() && IsZKeyDown()) {
			if (0 != ap_dialog->m_modelDataList.size()) {
				ap_dialog->m_modelDataList.pop_back();
				ap_dialog->Invalidate();
			}
		}
		return;
	};

	///////////////////////////////////////////////////////////////////
	// implementation
	///////////////////////////////////////////////////////////////////

	switch (a_msg.message)
	{
	case WM_CHAR:
		OnMultiKeyDown(this);
		break;
	case WM_KEYDOWN:
		OnKeyDown(this, a_msg);
		break;
	}
}

// to handle the WM_MOUSEMOVE message that occurs when a window is destroyed
int SketchDialog::MouseMoveHandler(WPARAM a_wordParam, LPARAM a_longParam)
{
	if (!m_leftButtonDown) {
		return S_OK;
	}

	if (m_parentModelData.isFadeMode) {
		// timer can not be called on mouse move, therefore here `OnFadeObjects` will be called per fps time
		ULONGLONG currentMilliseconds = GetTickCount64();
		if (currentMilliseconds - m_previousMilliseconds > SKETCH::FPS_TIME) {
			m_previousMilliseconds = currentMilliseconds;
			FadeObject(false);
		}
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
		data.opacity = ap_dialog->m_parentModelData.colorOpacity;
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
		break;
	case WINDOW_BRUSH::DT::RECTANGLE:
		data.rect = {
			static_cast<float>(point.x), static_cast<float>(point.y),
			static_cast<float>(point.x), static_cast<float>(point.y)
		};
		m_modelDataList.push_back(data);
		break;
	case WINDOW_BRUSH::DT::CIRCLE:
		data.rect = {
			static_cast<float>(point.x), static_cast<float>(point.y),
			static_cast<float>(point.x), static_cast<float>(point.y)
		};
		m_modelDataList.push_back(data);
		break;
	case WINDOW_BRUSH::DT::TEXT_OUTLINE:
		data.points.push_back({ static_cast<float>(point.x), static_cast<float>(point.y) });
		m_modelDataList.push_back(data);
		::SetWindowText(mh_edit, L"");
		break;
	case WINDOW_BRUSH::DT::TEXT_TYPING:
		SetTextOutlineModeHandler(0, true);
		break;
	}

	Invalidate();

	return S_OK;
}

// to handle the WM_LBUTTONUP message that occurs when a window is destroyed
int SketchDialog::MouseLeftButtonUpHandler(WPARAM a_wordParam, LPARAM a_longParam)
{
	if (m_leftButtonDown) {
		m_leftButtonDown = false;
		::ReleaseCapture();
	}

	return S_OK;
}

int SketchDialog::UpdateModelDataHandler(WPARAM a_wordParam, LPARAM a_longParam)
{
	const auto previousModelData = m_parentModelData;
	m_parentModelData = *reinterpret_cast<WINDOW_BRUSH::MD *>(a_wordParam);

	if (previousModelData.isFadeMode) {
		::KillTimer(mh_window, reinterpret_cast<UINT_PTR>(this));
	}
	if (m_parentModelData.isFadeMode) {
		::SetTimer(mh_window, reinterpret_cast<UINT_PTR>(this), SKETCH::FPS_TIME, FadeObjectOnTimer);
	}

	if (WINDOW_BRUSH::DT::TEXT_OUTLINE == previousModelData.drawType) {
		if (0 != m_modelDataList.size()) {
			m_modelDataList.pop_back();
			Invalidate();
		}
	}
	else if (WINDOW_BRUSH::DT::TEXT_TYPING == previousModelData.drawType) {
		m_modelDataList.back().drawType = WINDOW_BRUSH::DT::TEXT;
		Invalidate();
	}

	return S_OK;
}

int SketchDialog::SetTextOutlineModeHandler(WPARAM a_wordParam, LPARAM a_longParam)
{
	const auto isDone = static_cast<bool>(a_longParam);
	if (!isDone) {
		m_modelDataList.back().drawType = WINDOW_BRUSH::DT::TEXT_OUTLINE;
	}
	else {
		m_modelDataList.back().drawType = WINDOW_BRUSH::DT::TEXT;
	}

	m_parentModelData.drawType = WINDOW_BRUSH::DT::TEXT_OUTLINE;
	::SetWindowText(mh_edit, L"");
	::SetFocus(mh_window);

	return S_OK;
}

// to handle the SKETCH::WM_ON_EDIT_MAX_LEGNTH
int SketchDialog::OnEditMaxLengthHandler(WPARAM a_wordParam, LPARAM a_longParam)
{
	wchar_t *p_memoryText = reinterpret_cast<wchar_t *>(a_wordParam);
	const auto length = a_longParam;

	::SetWindowTextW(mh_edit, p_memoryText);
	::PostMessage(mh_edit, EM_SETSEL, length, length);

	delete[] p_memoryText;

	return S_OK;
}

// to handle the WM_KEYDOWN
int SketchDialog::KeyDownHandler(WPARAM a_wordParam, LPARAM a_longParam)
{
	// if only the shortcut keys of WindowBrushDialog is clicked it sends to the WindowBrushDialog

	if (0 != m_modelDataList.size()) {
		const auto lastDataDrawType = m_modelDataList.back().drawType;
		if (WINDOW_BRUSH::DT::TEXT_OUTLINE == lastDataDrawType || WINDOW_BRUSH::DT::TEXT_TYPING == lastDataDrawType) {
			return S_OK;
		}
	}

	const unsigned char pressedKey = static_cast<unsigned char>(a_wordParam);
	
	if ('C' == pressedKey || 'R' == pressedKey || 'E' == pressedKey || 'T' == pressedKey ||
		'W' == pressedKey || 'G' == pressedKey || 'P' == pressedKey || 'F' == pressedKey ||
		'H' == pressedKey) {
		::PostMessageW(mh_parentWindow, WM_KEYDOWN, a_wordParam, a_longParam);
	}

	return S_OK;
}

void SketchDialog::UpdateWindowBrushModelData(const WINDOW_BRUSH::MD *ap_modelData)
{
	::PostMessage(mh_window, SKETCH::WM_UPDATE_MODEL_DATA, reinterpret_cast<WPARAM>(ap_modelData), 0);
}

void SketchDialog::FadeObject(const bool isOnTimer)
{
	size_t count = m_modelDataList.size();
	if (0 == count) {
		return;
	}

	const auto countToDisappear = m_parentModelData.fadeTimer / SKETCH::FPS_TIME;
	const float speedToDisappear = m_parentModelData.colorOpacity / countToDisappear;
	if (!isOnTimer) {
		--count; // on mouse moving the last item should not be updated
	}

	std::vector<size_t> deletedDataIndexList;
	for (size_t i = 0; i < count; i++) {
		SKETCH::MD &modelData = m_modelDataList[i];

		if (WINDOW_BRUSH::DT::TEXT_OUTLINE != modelData.drawType &&
			WINDOW_BRUSH::DT::TEXT_TYPING != modelData.drawType) {
			modelData.defaultData.opacity -= speedToDisappear;

			if (0 >= modelData.defaultData.opacity) {
				deletedDataIndexList.push_back(i);
			}
		}
	}

	// reverse index to avoid memory access exception on delete item
	std::reverse(deletedDataIndexList.begin(), deletedDataIndexList.end());
	for (const auto &index : deletedDataIndexList) {
		m_modelDataList.erase(std::next(m_modelDataList.begin(), index));
	}

	Invalidate();
}

void __stdcall SketchDialog::FadeObjectOnTimer(HWND ah_wnd, UINT a_msg, UINT_PTR ap_data, DWORD a_isMouseMoving)
{
	reinterpret_cast<SketchDialog *>(ap_data)->FadeObject();
}