#include "PaletteDialog.h"
#include "PaletteView.h"
#include "ColorPalette.h"
#include "Utility.h"

#ifdef _DEBUG
#pragma comment (lib, "AppTemplateDebug.lib")
#else
#pragma comment (lib, "AppTemplate.lib")     
#endif

PaletteDialog::PaletteDialog(const DColor &a_selectedColor, const std::vector<DColor> &a_colorList) :
	WindowDialog(L"COLORDIALOG", L"ColorDialog"),
	m_previousSelectedColor(a_selectedColor),
	m_colorList(a_colorList)
{
	SetSize(PALETTE::DIALOG_WIDTH, PALETTE::DIALOG_HEIGHT);
	SetStyle(WS_POPUP | WS_VISIBLE);
	SetExtendStyle(WS_EX_TOPMOST);

	m_drawMode = PALETTE::DM::SELECT;
	m_modelData = {
		PALETTE::INVALID_INDEX, PALETTE::INVALID_INDEX, PALETTE::BT::NONE, PALETTE::BT::NONE, {0, }
	};

	memset(&m_colorCenterPoint, 0, sizeof(DPoint));
	isInitializedAddMode = false;
	m_selectedColorIndex = PALETTE::INVALID_INDEX;

	// add message handlers
	AddMessageHandler(WM_MOUSEMOVE, static_cast<MessageHandler>(&PaletteDialog::MouseMoveHandler));
	AddMessageHandler(WM_LBUTTONDOWN, static_cast<MessageHandler>(&PaletteDialog::MouseLeftButtonDownHandler));
	AddMessageHandler(WM_LBUTTONUP, static_cast<MessageHandler>(&PaletteDialog::MouseLeftButtonUpHandler));
	AddMessageHandler(WM_KEYDOWN, static_cast<MessageHandler>(&PaletteDialog::KeyDownHandler));
}

void PaletteDialog::OnInitDialog()
{
	DisableMaximize();
	DisableMinimize();
	DisableSize();

	const auto p_view = new PaletteView(mh_window, m_previousSelectedColor, m_colorList, GetColorMode());
	InheritDirect2D(p_view);
	p_view->Create();
	m_colorDataTable = p_view->GetColorDataTable();
	m_addButtonData = p_view->GetAddButtonData();
}

void PaletteDialog::OnPaint()
{
	mp_direct2d->Clear();

	// draw objects according to the DRAW_MODE
	static_cast<PaletteView *>(mp_direct2d)->Paint(m_drawMode, m_modelData);
}

// to handle the WM_MOUSEMOVE message that occurs when a window is destroyed
int PaletteDialog::MouseMoveHandler(WPARAM a_wordParam, LPARAM a_longParam)
{
	static const auto OnSelectMode = [](PaletteDialog *const ap_dialog, const POINT &a_point)
	{
		for (const auto &[index, rect] : ap_dialog->m_colorDataTable) {
			if (PointInRect(rect, a_point)) {
				if (index != ap_dialog->m_modelData.hoverIndex) {
					ap_dialog->m_modelData.hoverIndex = index;
					ap_dialog->Invalidate();
				}

				return;
			}
		}

		if (PointInRect(ap_dialog->m_addButtonData.second, a_point)) {
			if (ap_dialog->m_colorDataTable.size() != ap_dialog->m_modelData.hoverIndex) {
				ap_dialog->m_modelData.hoverIndex = ap_dialog->m_addButtonData.first;
				ap_dialog->Invalidate();
			}

			return;
		}

		if (PALETTE::INVALID_INDEX != ap_dialog->m_modelData.hoverIndex) {
			ap_dialog->m_modelData.hoverIndex = PALETTE::INVALID_INDEX;
			ap_dialog->Invalidate();
		}
	};
	static const auto OnClickedHueButton = [](PaletteView *const ap_view, const POINT &a_point, const DPoint &a_centerPoint, PALETTE::MD &a_modelData)
	{
		const float dx = a_point.x - a_centerPoint.x;
		const float dy = a_point.y - a_centerPoint.y;

		const double theta = atan(dy / dx);
		const int sign = dx >= 0 ? 1 : -1;

		const auto x = static_cast<float>(a_centerPoint.x + cos(theta) * PALETTE::HUE_CIRCLE_RADIUS * sign);
		const auto y = static_cast<float>(a_centerPoint.y + sin(theta) * PALETTE::HUE_CIRCLE_RADIUS * sign);

		a_modelData.hueButtonRect = {
			x - PALETTE::COLOR_RADIUS, y - PALETTE::COLOR_RADIUS,
			x + PALETTE::COLOR_RADIUS, y + PALETTE::COLOR_RADIUS
		};

		ap_view->UpdateLightnessCircle(DPoint({ x, y }));
	};
	static const auto OnClickedLightnessButton = [](const POINT &a_point, const DPoint &a_centerPoint, PALETTE::MD &a_modelData)
	{
		const float dx = a_point.x - a_centerPoint.x;
		const float dy = a_point.y - a_centerPoint.y;

		float x, y;
		if (dx * dx + dy * dy < PALETTE::LIGHTNESS_CIRCLE_RADIUS * PALETTE::LIGHTNESS_CIRCLE_RADIUS) {
			x = static_cast<float>(a_point.x);
			y = static_cast<float>(a_point.y);
		}
		else {
			double theta = atan(dy / dx);
			int sign = dx >= 0 ? 1 : -1;

			x = static_cast<float>(a_centerPoint.x + cos(theta) * (PALETTE::LIGHTNESS_CIRCLE_RADIUS - 1.0f) * sign);
			y = static_cast<float>(a_centerPoint.y + sin(theta) * (PALETTE::LIGHTNESS_CIRCLE_RADIUS - 1.0f) * sign);
		}

		a_modelData.lightnessButtonRect = {
			x - PALETTE::COLOR_RADIUS, y - PALETTE::COLOR_RADIUS,
			x + PALETTE::COLOR_RADIUS, y + PALETTE::COLOR_RADIUS
		};
	};
	static const auto OnAddMode = [](PaletteDialog *const ap_dialog, const POINT &a_point)
	{
		if (PALETTE::BT::HUE == ap_dialog->m_modelData.clickedButtonType) {
			OnClickedHueButton(
				static_cast<PaletteView *>(ap_dialog->mp_direct2d), a_point, ap_dialog->m_colorCenterPoint, ap_dialog->m_modelData
			);
			ap_dialog->Invalidate();

			return;
		}

		if (PALETTE::BT::LIGHTNESS == ap_dialog->m_modelData.clickedButtonType) {
			OnClickedLightnessButton(a_point, ap_dialog->m_colorCenterPoint, ap_dialog->m_modelData);
			ap_dialog->Invalidate();

			return;
		}

		for (auto const &[type, rect] : ap_dialog->m_buttonTable) {
			if (PointInRect(rect, a_point)) {
				if (type != ap_dialog->m_modelData.hoverButtonType) {
					ap_dialog->m_modelData.hoverButtonType = type;
					ap_dialog->Invalidate();
				}

				return;
			}
		}

		if (PointInRect(ap_dialog->m_modelData.hueButtonRect, a_point)) {
			ap_dialog->m_modelData.hoverButtonType = PALETTE::BT::HUE;
		}
		else if (PointInRect(ap_dialog->m_modelData.lightnessButtonRect, a_point)) {
			ap_dialog->m_modelData.hoverButtonType = PALETTE::BT::LIGHTNESS;
		}
		else if (PALETTE::BT::NONE != ap_dialog->m_modelData.hoverButtonType) {
			ap_dialog->m_modelData.hoverButtonType = PALETTE::BT::NONE;
		}
		ap_dialog->Invalidate();
	};

	////////////////////////////////////////////////////////////////
	// implementation
	////////////////////////////////////////////////////////////////

	const POINT point = { LOWORD(a_longParam), HIWORD(a_longParam) };

	switch (m_drawMode)
	{
	case PALETTE::DM::SELECT:
		OnSelectMode(this, point);
		break;
	case PALETTE::DM::ADD:
		OnAddMode(this, point);
		break;
	}

	return S_OK;
}

// to handle the WM_LBUTTONDOWN  message that occurs when a window is destroyed
int PaletteDialog::MouseLeftButtonDownHandler(WPARAM a_wordParam, LPARAM a_longParam)
{
	static const auto OnSelectMode = [](PaletteDialog *const ap_dialog, const POINT &a_point)
	{
		for (auto const &[index, rect] : ap_dialog->m_colorDataTable) {
			if (PointInRect(rect, a_point)) {
				ap_dialog->m_modelData.clickedIndex = index;
				ap_dialog->Invalidate();

				return;
			}
		}

		if (PointInRect(ap_dialog->m_addButtonData.second, a_point)) {
			ap_dialog->m_modelData.clickedIndex = ap_dialog->m_addButtonData.first;
			ap_dialog->Invalidate();

			return;
		}
	};
	static const auto OnAddMode = [](PaletteDialog *const ap_dialog, const POINT &a_point)
	{
		for (auto const &[type, rect] : ap_dialog->m_buttonTable) {
			if (PointInRect(rect, a_point)) {
				ap_dialog->m_modelData.clickedButtonType = type;
				ap_dialog->Invalidate();

				return;
			}
		}

		if (PointInRect(ap_dialog->m_modelData.hueButtonRect, a_point)) {
			ap_dialog->m_modelData.clickedButtonType = PALETTE::BT::HUE;
		}
		else if (PointInRect(ap_dialog->m_modelData.lightnessButtonRect, a_point)) {
			ap_dialog->m_modelData.clickedButtonType = PALETTE::BT::LIGHTNESS;
			ap_dialog->Invalidate();
		}
	};

	////////////////////////////////////////////////////////////////
	// implementation
	////////////////////////////////////////////////////////////////

	const POINT point = { LOWORD(a_longParam), HIWORD(a_longParam) };

	switch (m_drawMode)
	{
	case PALETTE::DM::SELECT:
		OnSelectMode(this, point);
		break;
	case PALETTE::DM::ADD:
		OnAddMode(this, point);
		break;
	}

	return S_OK;
}

// to handle the WM_LBUTTONUP  message that occurs when a window is destroyed
int PaletteDialog::MouseLeftButtonUpHandler(WPARAM a_wordParam, LPARAM a_longParam)
{
	static const auto OnSelectMode = [](PaletteDialog *const ap_dialog, const POINT &a_point)
	{
		for (auto const &[index, rect] : ap_dialog->m_colorDataTable) {
			if (PointInRect(rect, a_point)) {
				if (index == ap_dialog->m_modelData.clickedIndex) {
					ap_dialog->m_selectedColorIndex = index;
					auto type = WindowDialog::BT::OK;
					ap_dialog->SetClickedButtonType(type);
					::DestroyWindow(ap_dialog->GetWidnowHandle());
				}
				else {
					ap_dialog->m_modelData.clickedIndex = PALETTE::INVALID_INDEX;
					ap_dialog->Invalidate();
				}

				return;
			}
		}

		if (PointInRect(ap_dialog->m_addButtonData.second, a_point)) {
			if (ap_dialog->m_addButtonData.first == ap_dialog->m_modelData.clickedIndex) {
				ap_dialog->ChangeMode(PALETTE::DM::ADD);
			}
			else {
				ap_dialog->m_modelData.clickedIndex = PALETTE::INVALID_INDEX;
			}
			ap_dialog->Invalidate();

			return;
		}

		ap_dialog->m_modelData.clickedIndex = PALETTE::INVALID_INDEX;
		ap_dialog->Invalidate();
	};
	static const auto OnAddMode = [](PaletteDialog *const ap_dialog, const POINT &a_point)
	{
		for (auto const &[type, rect] : ap_dialog->m_buttonTable) {
			if (PointInRect(rect, a_point)) {
				if (type == ap_dialog->m_modelData.clickedButtonType) {
					if (PALETTE::BT::ADD == type) {
						const auto p_view = static_cast<PaletteView *>(ap_dialog->mp_direct2d);
						p_view->AddCurrentLightness();
						ap_dialog->m_colorDataTable = p_view->GetColorDataTable();
						ap_dialog->m_addButtonData = p_view->GetAddButtonData();
					}

					ap_dialog->ChangeMode(PALETTE::DM::SELECT);
				}

				break;
			}
		}

		ap_dialog->m_modelData.clickedButtonType = PALETTE::BT::NONE;
		ap_dialog->Invalidate();
	};

	////////////////////////////////////////////////////////////////
	// implementation
	////////////////////////////////////////////////////////////////

	const POINT point = { LOWORD(a_longParam), HIWORD(a_longParam) };

	switch (m_drawMode)
	{
	case PALETTE::DM::SELECT:
		OnSelectMode(this, point);
		break;
	case PALETTE::DM::ADD:
		OnAddMode(this, point);
		break;
	}

	return S_OK;
}

// to handle the WM_KEYDOWN message that occurs when a window is destroyed
int PaletteDialog::KeyDownHandler(WPARAM a_wordParam, LPARAM a_longParam)
{
	const unsigned char pressedKey = static_cast<unsigned char>(a_wordParam);

	if (VK_ESCAPE == pressedKey) {
		::DestroyWindow(mh_window);
	}

	return S_OK;
}

void PaletteDialog::ChangeMode(const PALETTE::DM &a_drawModw)
{
	m_drawMode = a_drawModw;

	m_modelData.clickedIndex = PALETTE::INVALID_INDEX;
	m_modelData.hoverIndex = PALETTE::INVALID_INDEX;
	m_modelData.hoverButtonType = PALETTE::BT::NONE;
	m_modelData.clickedButtonType = PALETTE::BT::NONE;

	if (PALETTE::DM::ADD == a_drawModw && !isInitializedAddMode) {
		isInitializedAddMode = true;

		m_colorCenterPoint = {
			PALETTE::DIALOG_WIDTH / 2.0f, PALETTE::TITLE_HEIGHT + (PALETTE::DIALOG_HEIGHT - PALETTE::TITLE_HEIGHT - PALETTE::INDICATE_HEIGHT) / 2.0f - 10.0f
		};

		m_modelData.hueButtonRect = {
			m_colorCenterPoint.x + PALETTE::HUE_CIRCLE_RADIUS - PALETTE::COLOR_RADIUS, m_colorCenterPoint.y - PALETTE::COLOR_RADIUS,
			m_colorCenterPoint.x + PALETTE::HUE_CIRCLE_RADIUS + PALETTE::COLOR_RADIUS, m_colorCenterPoint.y + PALETTE::COLOR_RADIUS
		};

		m_modelData.lightnessButtonRect = {
			m_colorCenterPoint.x - PALETTE::COLOR_RADIUS, m_colorCenterPoint.y - PALETTE::COLOR_RADIUS,
			m_colorCenterPoint.x + PALETTE::COLOR_RADIUS, m_colorCenterPoint.y + PALETTE::COLOR_RADIUS
		};

		static_cast<PaletteView *>(mp_direct2d)->InitColorAddView(m_colorCenterPoint);
		m_buttonTable = static_cast<PaletteView *>(mp_direct2d)->GetButtonTable();
	}
}

DColor PaletteDialog::GetSelectedColor()
{
	if (PALETTE::INVALID_INDEX == m_selectedColorIndex) {
		return m_previousSelectedColor;
	}

	return static_cast<PaletteView *>(mp_direct2d)->GetColor(m_selectedColorIndex);
}

std::vector<DColor> PaletteDialog::GetColorList()
{
	return static_cast<PaletteView *>(mp_direct2d)->GetColorList();
}
