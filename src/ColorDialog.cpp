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
}

ColorDialog::~ColorDialog()
{

}

void ColorDialog::OnInitDialog()
{
	DisableMaximize();
	DisableMinimize();
	DisableSize();

	// add message handlers
	AddMessageHandler(WM_MOUSEMOVE, static_cast<MessageHandler>(&ColorDialog::MouseMoveHandler));
	AddMessageHandler(WM_LBUTTONDOWN, static_cast<MessageHandler>(&ColorDialog::MouseLeftButtonDownHandler));
	AddMessageHandler(WM_LBUTTONUP, static_cast<MessageHandler>(&ColorDialog::MouseLeftButtonUpHandler));
	AddMessageHandler(WM_KEYDOWN, static_cast<MessageHandler>(&ColorDialog::KeyDownHandler));

	const auto p_view = new ColorView(mh_window, m_previousSelectedColor, m_colorList, GetColorMode());
	InheritDirect2D(p_view);
	p_view->Create();
	m_colorDataTable = p_view->GetColorDataTable();
	m_addButtonData = p_view->GetAddButtonData();
}

void ColorDialog::OnDestroy()
{

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
	static const auto OnSelectMode = [](
		const std::map<size_t, DRect> a_colorDataTable, const std::pair<size_t, DRect> &a_addButtonData,
		ColorDialog *const a_dialog, size_t &a_hoverIndex, const POINT &pos
		)
	{
		for (const auto &[index, rect] : a_colorDataTable) {
			if (PointInRect(rect, pos)) {
				if (index != a_hoverIndex) {
					a_hoverIndex = index;
					a_dialog->Invalidate();
				}

				return;
			}
		}

		if (PointInRect(a_addButtonData.second, pos)) {
			if (a_colorDataTable.size() != a_hoverIndex) {
				a_hoverIndex = a_addButtonData.first;
				a_dialog->Invalidate();
			}

			return;
		}

		if (COLOR::INVALID_INDEX != a_hoverIndex) {
			a_hoverIndex = COLOR::INVALID_INDEX;
			a_dialog->Invalidate();
		}
	};
	static const auto OnClickedHueButton = [](ColorView *const p_view, const POINT &a_point, const DPoint &a_centerPoint, COLOR::MD &a_modelData)
	{
		const float dx = a_point.x - a_centerPoint.x;
		const float dy = a_point.y - a_centerPoint.y;

		const double theta = atan(dy / dx);
		const int sign = dx >= 0 ? 1 : -1;

		const auto x = static_cast<float>(a_centerPoint.x + cos(theta) * COLOR::HUE_CIRCLE_RADIUS * sign);
		const auto y = static_cast<float>(a_centerPoint.y + sin(theta) * COLOR::HUE_CIRCLE_RADIUS * sign);

		a_modelData.hueButtonRect = {
			x - COLOR::BUTTON_RADIUS, y - COLOR::BUTTON_RADIUS,
			x + COLOR::BUTTON_RADIUS, y + COLOR::BUTTON_RADIUS
		};

		p_view->UpdateLightnessCircle(DPoint({ x, y }));
	};
	static const auto OnClickedLightnessButton = [](const POINT &a_point, const DPoint &a_centerPoint, COLOR::MD &a_modelData)
	{
		const float dx = a_point.x - a_centerPoint.x;
		const float dy = a_point.y - a_centerPoint.y;

		float x, y;
		if (dx * dx + dy * dy < COLOR::LIHTNESS_CIRCLE_RADIUS * COLOR::LIHTNESS_CIRCLE_RADIUS) {
			x = static_cast<float>(a_point.x);
			y = static_cast<float>(a_point.y);
		}
		else {
			double theta = atan(dy / dx);
			int sign = dx >= 0 ? 1 : -1;

			x = static_cast<float>(a_centerPoint.x + cos(theta) * (COLOR::LIHTNESS_CIRCLE_RADIUS - 1.0f) * sign);
			y = static_cast<float>(a_centerPoint.y + sin(theta) * (COLOR::LIHTNESS_CIRCLE_RADIUS - 1.0f) * sign);
		}

		a_modelData.lightnessButtonRect = {
			x - COLOR::BUTTON_RADIUS, y - COLOR::BUTTON_RADIUS,
			x + COLOR::BUTTON_RADIUS, y + COLOR::BUTTON_RADIUS
		};
	};
	static const auto OnAddMode = [](
		ColorView *const ap_view, ColorDialog *const ap_dialog, const POINT &a_point, const DPoint &a_colorCenterPoint,
		const std::map<COLOR::BT, DRect> &a_buttonTable, COLOR::MD &a_modelData
		)
	{
		if (COLOR::BT::HUE == a_modelData.clickedButtonType) {
			OnClickedHueButton(ap_view, a_point, a_colorCenterPoint, a_modelData);
			ap_dialog->Invalidate();

			return;
		}

		if (COLOR::BT::LIGHTNESS == a_modelData.clickedButtonType) {
			OnClickedLightnessButton(a_point, a_colorCenterPoint, a_modelData);
			ap_dialog->Invalidate();

			return;
		}

		for (auto const &[type, rect] : a_buttonTable) {
			if (PointInRect(rect, a_point)) {
				if (type != a_modelData.hoverButtonType) {
					a_modelData.hoverButtonType = type;
					ap_dialog->Invalidate();
				}

				return;
			}
		}

		if (PointInRect(a_modelData.hueButtonRect, a_point)) {
			a_modelData.hoverButtonType = COLOR::BT::HUE;
		}
		else if (PointInRect(a_modelData.lightnessButtonRect, a_point)) {
			a_modelData.hoverButtonType = COLOR::BT::LIGHTNESS;
		}
		else if (COLOR::BT::NONE != a_modelData.hoverButtonType) {
			a_modelData.hoverButtonType = COLOR::BT::NONE;
		}
		ap_dialog->Invalidate();
	};

	////////////////////////////////////////////////////////////////
	// implementation
	////////////////////////////////////////////////////////////////

	const POINT point = { LOWORD(a_longParam), HIWORD(a_longParam) };

	if (COLOR::DM::SELECT == m_drawMode) {
		OnSelectMode(m_colorDataTable, m_addButtonData, this, m_modelData.hoverIndex, point);
	}
	else {
		OnAddMode(static_cast<ColorView *>(mp_direct2d), this, point, m_colorCenterPoint, m_buttonTable, m_modelData);
	}

	return S_OK;
}

// to handle the WM_LBUTTONDOWN  message that occurs when a window is destroyed
int ColorDialog::MouseLeftButtonDownHandler(WPARAM a_wordParam, LPARAM a_longParam)
{
	static const auto OnSelectMode = [](
		const std::map<size_t, DRect> a_colorDataTable, const std::pair<size_t, DRect> &a_addButtonData,
		ColorDialog *const a_dialog, size_t &a_clickedIndex, const POINT &pos
		)
	{
		for (auto const &[index, rect] : a_colorDataTable) {
			if (PointInRect(rect, pos)) {
				a_clickedIndex = index;
				a_dialog->Invalidate();

				return;
			}
		}

		if (PointInRect(a_addButtonData.second, pos)) {
			a_clickedIndex = a_addButtonData.first;
			a_dialog->Invalidate();

			return;
		}
	};
	static const auto OnAddMode = [](
		ColorDialog *const ap_dialog, const POINT &a_point, const std::map<COLOR::BT, DRect> &a_buttonTable, COLOR::MD &a_modelData
		)
	{
		for (auto const &[type, rect] : a_buttonTable) {
			if (PointInRect(rect, a_point)) {
				a_modelData.clickedButtonType = type;
				ap_dialog->Invalidate();

				return;
			}
		}

		if (PointInRect(a_modelData.hueButtonRect, a_point)) {
			a_modelData.clickedButtonType = COLOR::BT::HUE;
		}
		else if (PointInRect(a_modelData.lightnessButtonRect, a_point)) {
			a_modelData.clickedButtonType = COLOR::BT::LIGHTNESS;
			ap_dialog->Invalidate();
		}
	};

	////////////////////////////////////////////////////////////////
	// implementation
	////////////////////////////////////////////////////////////////

	const POINT point = { LOWORD(a_longParam), HIWORD(a_longParam) };

	if (COLOR::DM::SELECT == m_drawMode) {
		OnSelectMode(m_colorDataTable, m_addButtonData, this, m_modelData.clickedIndex, point);
	}
	else {
		OnAddMode(this, point, m_buttonTable, m_modelData);
	}

	return S_OK;
}

// to handle the WM_LBUTTONUP  message that occurs when a window is destroyed
int ColorDialog::MouseLeftButtonUpHandler(WPARAM a_wordParam, LPARAM a_longParam)
{
	static const auto OnSelectMode = [](
		ColorDialog *const ap_dialog, const POINT &point, const std::map<size_t, DRect> a_colorDataTable,
		const std::pair<size_t, DRect> &a_addButtonData, size_t &a_selectedColorIndex, COLOR::MD &a_modelData
		)
	{
		for (auto const &[index, rect] : a_colorDataTable) {
			if (PointInRect(rect, point)) {
				if (index == a_modelData.clickedIndex) {
					a_selectedColorIndex = index;
					auto type = WindowDialog::BT::OK;
					ap_dialog->SetClickedButtonType(type);
					::DestroyWindow(ap_dialog->GetWidnowHandle());
				}
				else {
					a_modelData.clickedIndex = COLOR::INVALID_INDEX;
					ap_dialog->Invalidate();
				}

				return;
			}
		}

		if (PointInRect(a_addButtonData.second, point)) {
			if (a_addButtonData.first == a_modelData.clickedIndex) {
				ap_dialog->ChangeMode(COLOR::DM::ADD);
			}
			else {
				a_modelData.clickedIndex = COLOR::INVALID_INDEX;
			}
			ap_dialog->Invalidate();

			return;
		}

		a_modelData.clickedIndex = COLOR::INVALID_INDEX;
		ap_dialog->Invalidate();
	};
	static const auto OnAddMode = [](
		ColorView *const ap_view, ColorDialog *const ap_dialog, const POINT &a_point, const std::map<COLOR::BT, DRect> &a_buttonTable,
		std::map<size_t, DRect> &a_colorDataTable, std::pair<size_t, DRect> &a_addButtonData, COLOR::MD &a_modelData)
	{
		for (auto const &[type, rect] : a_buttonTable) {
			if (PointInRect(rect, a_point)) {
				if (type == a_modelData.clickedButtonType) {
					if (COLOR::BT::ADD == type) {
						ap_view->AddCurrentLightness();
						a_colorDataTable = ap_view->GetColorDataTable();
						a_addButtonData = ap_view->GetAddButtonData();
					}

					ap_dialog->ChangeMode(COLOR::DM::SELECT);
				}

				break;
			}
		}

		a_modelData.clickedButtonType = COLOR::BT::NONE;
		ap_dialog->Invalidate();
	};

	////////////////////////////////////////////////////////////////
	// implementation
	////////////////////////////////////////////////////////////////

	const POINT point = { LOWORD(a_longParam), HIWORD(a_longParam) };

	if (COLOR::DM::SELECT == m_drawMode) {
		OnSelectMode(this, point, m_colorDataTable, m_addButtonData, m_selectedColorIndex, m_modelData);
	}
	else {
		OnAddMode(static_cast<ColorView *>(mp_direct2d), this, point, m_buttonTable, m_colorDataTable, m_addButtonData, m_modelData);
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
			m_colorCenterPoint.x + COLOR::HUE_CIRCLE_RADIUS - COLOR::BUTTON_RADIUS, m_colorCenterPoint.y - COLOR::BUTTON_RADIUS,
			m_colorCenterPoint.x + COLOR::HUE_CIRCLE_RADIUS + COLOR::BUTTON_RADIUS, m_colorCenterPoint.y + COLOR::BUTTON_RADIUS
		};

		m_modelData.lightnessButtonRect = {
			m_colorCenterPoint.x - COLOR::BUTTON_RADIUS, m_colorCenterPoint.y - COLOR::BUTTON_RADIUS,
			m_colorCenterPoint.x + COLOR::BUTTON_RADIUS, m_colorCenterPoint.y + COLOR::BUTTON_RADIUS
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
