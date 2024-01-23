#include "ColorDialog.h"
#include "ColorView.h"
#include "ColorPalette.h"
#include "Utility.h"

#ifdef _DEBUG
#pragma comment (lib, "AppTemplateDebug.lib")
#else
#pragma comment (lib, "AppTemplate.lib")     
#endif

ColorDialog::ColorDialog(const DColor &a_selectedColor, const std::vector<DColor> &a_colorList) :
	WindowDialog(L"COLORDIALOG", L"ColorDialog"),
	m_previousSelectedColor(a_selectedColor),
	m_colorList(a_colorList)
{
	SetSize(COLOR::DIALOG_WIDTH, COLOR::DIALOG_HEIGHT);
	SetStyle(WS_POPUP | WS_VISIBLE);
	SetExtendStyle(WS_EX_TOPMOST);

	m_drawMode = COLOR::DM::SELECT;
	m_modelData = {
		COLOR::INVALID_INDEX, COLOR::INVALID_INDEX, COLOR::BT::NONE, COLOR::BT::NONE, {0, }
	};

	memset(&m_colorCenterPoint, 0, sizeof(DPoint));
	isInitializedAddMode = false;
	m_selectedColorIndex = COLOR::INVALID_INDEX;

	// add message handlers
	AddMessageHandler(WM_MOUSEMOVE, static_cast<MessageHandler>(&ColorDialog::MouseMoveHandler));
	AddMessageHandler(WM_LBUTTONDOWN, static_cast<MessageHandler>(&ColorDialog::MouseLeftButtonDownHandler));
	AddMessageHandler(WM_LBUTTONUP, static_cast<MessageHandler>(&ColorDialog::MouseLeftButtonUpHandler));
	AddMessageHandler(WM_KEYDOWN, static_cast<MessageHandler>(&ColorDialog::KeyDownHandler));
}

void ColorDialog::OnInitDialog()
{
	DisableMaximize();
	DisableMinimize();
	DisableSize();

	const auto p_view = new ColorView(mh_window, m_previousSelectedColor, m_colorList, GetColorMode());
	InheritDirect2D(p_view);
	p_view->Create();
	m_colorDataTable = p_view->GetColorDataTable();
	m_addButtonData = p_view->GetAddButtonData();
}

void ColorDialog::OnPaint()
{
	mp_direct2d->Clear();

	// draw objects according to the DRAW_MODE
	static_cast<ColorView *>(mp_direct2d)->Paint(m_drawMode, m_modelData);
}

// to handle the WM_MOUSEMOVE message that occurs when a window is destroyed
int ColorDialog::MouseMoveHandler(WPARAM a_wordParam, LPARAM a_longParam)
{
	static const auto OnSelectMode = [](ColorDialog *const ap_dialog, const POINT &a_point)
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

		if (COLOR::INVALID_INDEX != ap_dialog->m_modelData.hoverIndex) {
			ap_dialog->m_modelData.hoverIndex = COLOR::INVALID_INDEX;
			ap_dialog->Invalidate();
		}
	};
	static const auto OnClickedHueButton = [](ColorView *const ap_view, const POINT &a_point, const DPoint &a_centerPoint, COLOR::MD &a_modelData)
	{
		const float dx = a_point.x - a_centerPoint.x;
		const float dy = a_point.y - a_centerPoint.y;

		const double theta = atan(dy / dx);
		const int sign = dx >= 0 ? 1 : -1;

		const auto x = static_cast<float>(a_centerPoint.x + cos(theta) * COLOR::HUE_CIRCLE_RADIUS * sign);
		const auto y = static_cast<float>(a_centerPoint.y + sin(theta) * COLOR::HUE_CIRCLE_RADIUS * sign);

		a_modelData.hueButtonRect = {
			x - COLOR::COLOR_RADIUS, y - COLOR::COLOR_RADIUS,
			x + COLOR::COLOR_RADIUS, y + COLOR::COLOR_RADIUS
		};

		ap_view->UpdateLightnessCircle(DPoint({ x, y }));
	};
	static const auto OnClickedLightnessButton = [](const POINT &a_point, const DPoint &a_centerPoint, COLOR::MD &a_modelData)
	{
		const float dx = a_point.x - a_centerPoint.x;
		const float dy = a_point.y - a_centerPoint.y;

		float x, y;
		if (dx * dx + dy * dy < COLOR::LIGHTNESS_CIRCLE_RADIUS * COLOR::LIGHTNESS_CIRCLE_RADIUS) {
			x = static_cast<float>(a_point.x);
			y = static_cast<float>(a_point.y);
		}
		else {
			double theta = atan(dy / dx);
			int sign = dx >= 0 ? 1 : -1;

			x = static_cast<float>(a_centerPoint.x + cos(theta) * (COLOR::LIGHTNESS_CIRCLE_RADIUS - 1.0f) * sign);
			y = static_cast<float>(a_centerPoint.y + sin(theta) * (COLOR::LIGHTNESS_CIRCLE_RADIUS - 1.0f) * sign);
		}

		a_modelData.lightnessButtonRect = {
			x - COLOR::COLOR_RADIUS, y - COLOR::COLOR_RADIUS,
			x + COLOR::COLOR_RADIUS, y + COLOR::COLOR_RADIUS
		};
	};
	static const auto OnAddMode = [](ColorDialog *const ap_dialog, const POINT &a_point)
	{
		if (COLOR::BT::HUE == ap_dialog->m_modelData.clickedButtonType) {
			OnClickedHueButton(
				static_cast<ColorView *>(ap_dialog->mp_direct2d), a_point, ap_dialog->m_colorCenterPoint, ap_dialog->m_modelData
			);
			ap_dialog->Invalidate();

			return;
		}

		if (COLOR::BT::LIGHTNESS == ap_dialog->m_modelData.clickedButtonType) {
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
			ap_dialog->m_modelData.hoverButtonType = COLOR::BT::HUE;
		}
		else if (PointInRect(ap_dialog->m_modelData.lightnessButtonRect, a_point)) {
			ap_dialog->m_modelData.hoverButtonType = COLOR::BT::LIGHTNESS;
		}
		else if (COLOR::BT::NONE != ap_dialog->m_modelData.hoverButtonType) {
			ap_dialog->m_modelData.hoverButtonType = COLOR::BT::NONE;
		}
		ap_dialog->Invalidate();
	};

	////////////////////////////////////////////////////////////////
	// implementation
	////////////////////////////////////////////////////////////////

	const POINT point = { LOWORD(a_longParam), HIWORD(a_longParam) };

	switch (m_drawMode)
	{
	case COLOR::DM::SELECT:
		OnSelectMode(this, point);
		break;
	case COLOR::DM::ADD:
		OnAddMode(this, point);
		break;
	}

	return S_OK;
}

// to handle the WM_LBUTTONDOWN  message that occurs when a window is destroyed
int ColorDialog::MouseLeftButtonDownHandler(WPARAM a_wordParam, LPARAM a_longParam)
{
	static const auto OnSelectMode = [](ColorDialog *const ap_dialog, const POINT &a_point)
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
	static const auto OnAddMode = [](ColorDialog *const ap_dialog, const POINT &a_point)
	{
		for (auto const &[type, rect] : ap_dialog->m_buttonTable) {
			if (PointInRect(rect, a_point)) {
				ap_dialog->m_modelData.clickedButtonType = type;
				ap_dialog->Invalidate();

				return;
			}
		}

		if (PointInRect(ap_dialog->m_modelData.hueButtonRect, a_point)) {
			ap_dialog->m_modelData.clickedButtonType = COLOR::BT::HUE;
		}
		else if (PointInRect(ap_dialog->m_modelData.lightnessButtonRect, a_point)) {
			ap_dialog->m_modelData.clickedButtonType = COLOR::BT::LIGHTNESS;
			ap_dialog->Invalidate();
		}
	};

	////////////////////////////////////////////////////////////////
	// implementation
	////////////////////////////////////////////////////////////////

	const POINT point = { LOWORD(a_longParam), HIWORD(a_longParam) };

	switch (m_drawMode)
	{
	case COLOR::DM::SELECT:
		OnSelectMode(this, point);
		break;
	case COLOR::DM::ADD:
		OnAddMode(this, point);
		break;
	}

	return S_OK;
}

// to handle the WM_LBUTTONUP  message that occurs when a window is destroyed
int ColorDialog::MouseLeftButtonUpHandler(WPARAM a_wordParam, LPARAM a_longParam)
{
	static const auto OnSelectMode = [](ColorDialog *const ap_dialog, const POINT &a_point)
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
					ap_dialog->m_modelData.clickedIndex = COLOR::INVALID_INDEX;
					ap_dialog->Invalidate();
				}

				return;
			}
		}

		if (PointInRect(ap_dialog->m_addButtonData.second, a_point)) {
			if (ap_dialog->m_addButtonData.first == ap_dialog->m_modelData.clickedIndex) {
				ap_dialog->ChangeMode(COLOR::DM::ADD);
			}
			else {
				ap_dialog->m_modelData.clickedIndex = COLOR::INVALID_INDEX;
			}
			ap_dialog->Invalidate();

			return;
		}

		ap_dialog->m_modelData.clickedIndex = COLOR::INVALID_INDEX;
		ap_dialog->Invalidate();
	};
	static const auto OnAddMode = [](ColorDialog *const ap_dialog, const POINT &a_point)
	{
		for (auto const &[type, rect] : ap_dialog->m_buttonTable) {
			if (PointInRect(rect, a_point)) {
				if (type == ap_dialog->m_modelData.clickedButtonType) {
					if (COLOR::BT::ADD == type) {
						const auto p_view = static_cast<ColorView *>(ap_dialog->mp_direct2d);
						p_view->AddCurrentLightness();
						ap_dialog->m_colorDataTable = p_view->GetColorDataTable();
						ap_dialog->m_addButtonData = p_view->GetAddButtonData();
					}

					ap_dialog->ChangeMode(COLOR::DM::SELECT);
				}

				break;
			}
		}

		ap_dialog->m_modelData.clickedButtonType = COLOR::BT::NONE;
		ap_dialog->Invalidate();
	};

	////////////////////////////////////////////////////////////////
	// implementation
	////////////////////////////////////////////////////////////////

	const POINT point = { LOWORD(a_longParam), HIWORD(a_longParam) };

	switch (m_drawMode)
	{
	case COLOR::DM::SELECT:
		OnSelectMode(this, point);
		break;
	case COLOR::DM::ADD:
		OnAddMode(this, point);
		break;
	}

	return S_OK;
}

// to handle the WM_KEYDOWN message that occurs when a window is destroyed
int ColorDialog::KeyDownHandler(WPARAM a_wordParam, LPARAM a_longParam)
{
	const unsigned char pressedKey = static_cast<unsigned char>(a_wordParam);

	if (VK_ESCAPE == pressedKey) {
		::DestroyWindow(mh_window);
	}

	return S_OK;
}

void ColorDialog::ChangeMode(const COLOR::DM &a_drawModw)
{
	m_drawMode = a_drawModw;

	m_modelData.clickedIndex = COLOR::INVALID_INDEX;
	m_modelData.hoverIndex = COLOR::INVALID_INDEX;
	m_modelData.hoverButtonType = COLOR::BT::NONE;
	m_modelData.clickedButtonType = COLOR::BT::NONE;

	if (COLOR::DM::ADD == a_drawModw && !isInitializedAddMode) {
		isInitializedAddMode = true;

		m_colorCenterPoint = {
			COLOR::DIALOG_WIDTH / 2.0f, COLOR::TITLE_HEIGHT + (COLOR::DIALOG_HEIGHT - COLOR::TITLE_HEIGHT - COLOR::INDICATE_HEIGHT) / 2.0f - 10.0f
		};

		m_modelData.hueButtonRect = {
			m_colorCenterPoint.x + COLOR::HUE_CIRCLE_RADIUS - COLOR::COLOR_RADIUS, m_colorCenterPoint.y - COLOR::COLOR_RADIUS,
			m_colorCenterPoint.x + COLOR::HUE_CIRCLE_RADIUS + COLOR::COLOR_RADIUS, m_colorCenterPoint.y + COLOR::COLOR_RADIUS
		};

		m_modelData.lightnessButtonRect = {
			m_colorCenterPoint.x - COLOR::COLOR_RADIUS, m_colorCenterPoint.y - COLOR::COLOR_RADIUS,
			m_colorCenterPoint.x + COLOR::COLOR_RADIUS, m_colorCenterPoint.y + COLOR::COLOR_RADIUS
		};

		static_cast<ColorView *>(mp_direct2d)->InitColorAddView(m_colorCenterPoint);
		m_buttonTable = static_cast<ColorView *>(mp_direct2d)->GetButtonTable();
	}
}

DColor ColorDialog::GetSelectedColor()
{
	if (COLOR::INVALID_INDEX == m_selectedColorIndex) {
		return m_previousSelectedColor;
	}

	return static_cast<ColorView *>(mp_direct2d)->GetColor(m_selectedColorIndex);
}

std::vector<DColor> ColorDialog::GetColorList()
{
	return static_cast<ColorView *>(mp_direct2d)->GetColorList();
}
