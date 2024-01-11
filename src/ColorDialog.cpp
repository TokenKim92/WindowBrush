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
	m_previousSelectedColor(a_selectedColor)
{
	SetSize(COLOR_DIALOG_WIDTH, COLOR_DIALOG_HEIGHT);

	m_drawMode = CDM::SELECT;
	m_modelData = {
		INVALID_INDEX, INVALID_INDEX, CBT::NONE, CBT::NONE, {0, }
	};

	memset(&m_colorCenterPoint, 0, sizeof(DPoint));
	isInitializedAddMode = false;
	m_selectedColorIndex = INVALID_INDEX;
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

	const auto p_view = new ColorView(mh_window, m_previousSelectedColor, m_colorList, GetThemeMode());
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
		for (auto const &[index, rect] : a_colorDataTable) {
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

		if (INVALID_INDEX != a_hoverIndex) {
			a_hoverIndex = INVALID_INDEX;
			a_dialog->Invalidate();
		}
	};
	static const auto OnClickedHueButton = [](ColorView *const p_view, const POINT &a_point, const DPoint &a_centerPoint, CMD &a_modelData)
	{
		const float dx = a_point.x - a_centerPoint.x;
		const float dy = a_point.y - a_centerPoint.y;

		const double theta = atan(dy / dx);
		const int sign = dx >= 0 ? 1 : -1;

		const auto x = static_cast<float>(a_centerPoint.x + cos(theta) * HUE_CIRCLE_RADIUS * sign);
		const auto y = static_cast<float>(a_centerPoint.y + sin(theta) * HUE_CIRCLE_RADIUS * sign);

		a_modelData.hueButtonRect = {
			x - BUTTON_RADIUS, y - BUTTON_RADIUS,
			x + BUTTON_RADIUS, y + BUTTON_RADIUS
		};

		p_view->UpdateLightnessCircle(DPoint({ x, y }));
	};
	static const auto OnClickedLightnessButton = [](const POINT &a_point, const DPoint &a_centerPoint, CMD &a_modelData)
	{
		const float dx = a_point.x - a_centerPoint.x;
		const float dy = a_point.y - a_centerPoint.y;

		float x, y;
		if (dx * dx + dy * dy < LIHTNESS_CIRCLE_RADIUS * LIHTNESS_CIRCLE_RADIUS) {
			x = static_cast<float>(a_point.x);
			y = static_cast<float>(a_point.y);
		}
		else {
			double theta = atan(dy / dx);
			int sign = dx >= 0 ? 1 : -1;

			x = static_cast<float>(a_centerPoint.x + cos(theta) * (LIHTNESS_CIRCLE_RADIUS - 1.0f) * sign);
			y = static_cast<float>(a_centerPoint.y + sin(theta) * (LIHTNESS_CIRCLE_RADIUS - 1.0f) * sign);
		}

		a_modelData.lightnessButtonRect = {
			x - BUTTON_RADIUS, y - BUTTON_RADIUS,
			x + BUTTON_RADIUS, y + BUTTON_RADIUS
		};
	};

	////////////////////////////////////////////////////////////////
	// implementation
	////////////////////////////////////////////////////////////////

	const POINT point = { LOWORD(a_longParam), HIWORD(a_longParam) };

	if (CDM::SELECT == m_drawMode) {
		OnSelectMode(m_colorDataTable, m_addButtonData, this, m_modelData.hoverIndex, point);

		return S_OK;
	}

	if (CBT::HUE == m_modelData.clickedButtonType) {
		OnClickedHueButton(static_cast<ColorView *>(mp_direct2d), point, m_colorCenterPoint, m_modelData);
		Invalidate();
		
		return S_OK;
	}

	if (CBT::LIGHTNESS == m_modelData.clickedButtonType) {
		OnClickedLightnessButton(point, m_colorCenterPoint, m_modelData);
		Invalidate();

		return S_OK;
	}

	for (auto const &[type, rect] : m_buttonTable) {
		if (PointInRect(rect, point)) {
			if (type != m_modelData.hoverButtonType) {
				m_modelData.hoverButtonType = type;
				Invalidate();
			}

			return S_OK;
		}
	}

	if (PointInRect(m_modelData.hueButtonRect, point)) {
		m_modelData.hoverButtonType = CBT::HUE;
	} else if (PointInRect(m_modelData.lightnessButtonRect, point)) {
		m_modelData.hoverButtonType = CBT::LIGHTNESS;
	} else if (CBT::NONE != m_modelData.hoverButtonType) {
		m_modelData.hoverButtonType = CBT::NONE;
	}
	Invalidate();

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

	////////////////////////////////////////////////////////////////
	// implementation
	////////////////////////////////////////////////////////////////

	const POINT point = { LOWORD(a_longParam), HIWORD(a_longParam) };

	if (CDM::SELECT == m_drawMode) {
		OnSelectMode(m_colorDataTable, m_addButtonData, this, m_modelData.clickedIndex, point);

		return S_OK;
	}

	for (auto const &[type, rect] : m_buttonTable) {
		if (PointInRect(rect, point)) {
			m_modelData.clickedButtonType = type;
			Invalidate();

			return S_OK;
		}
	}

	if (PointInRect(m_modelData.hueButtonRect, point)) {
		m_modelData.clickedButtonType = CBT::HUE;
	} else if (PointInRect(m_modelData.lightnessButtonRect, point)) {
		m_modelData.clickedButtonType = CBT::LIGHTNESS;
		Invalidate();
	}

	return S_OK;
}

// to handle the WM_LBUTTONUP  message that occurs when a window is destroyed
int ColorDialog::MouseLeftButtonUpHandler(WPARAM a_wordParam, LPARAM a_longParam)
{
	static const auto OnSelectMode = [](
		const std::map<size_t, DRect> a_colorDataTable, const std::pair<size_t, DRect> &a_addButtonData,
		size_t &a_selectedColorIndex, ColorDialog *const a_dialog, size_t &a_clickedIndex, const POINT &pos
		)
	{
		for (auto const &[index, rect] : a_colorDataTable) {
			if (PointInRect(rect, pos)) {
				if (index == a_clickedIndex) {
					a_selectedColorIndex = index;
					::DestroyWindow(a_dialog->GetWidnowHandle());
				}
				else {
					a_clickedIndex = INVALID_INDEX;
					a_dialog->Invalidate();
				}

				return;
			}
		}

		if (PointInRect(a_addButtonData.second, pos)) {
			if (a_addButtonData.first == a_clickedIndex) {
				a_dialog->ChangeMode(CDM::ADD);
			}
			else {
				a_clickedIndex = INVALID_INDEX;
				a_dialog->Invalidate();
			}

			return;
		}

		a_clickedIndex = INVALID_INDEX;
		a_dialog->Invalidate();
	};

	////////////////////////////////////////////////////////////////
	// implementation
	////////////////////////////////////////////////////////////////

	const POINT point = { LOWORD(a_longParam), HIWORD(a_longParam) };

	if (CDM::SELECT == m_drawMode) {
		OnSelectMode(m_colorDataTable, m_addButtonData, m_selectedColorIndex, this, m_modelData.clickedIndex, point);

		return S_OK;
	}

	for (auto const &[type, rect] : m_buttonTable) {
		if (PointInRect(rect, point)) {
			if (type == m_modelData.clickedButtonType) {
				ChangeMode(CDM::SELECT);
			}
			else {
				m_modelData.clickedButtonType = CBT::NONE;
			}

			return S_OK;
		}
	}

	m_modelData.clickedButtonType = CBT::NONE;
	Invalidate();

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

void ColorDialog::ChangeMode(const CDM &a_drawModw)
{
	m_drawMode = a_drawModw;

	m_modelData.clickedIndex = INVALID_INDEX;
	m_modelData.hoverIndex = INVALID_INDEX;
	m_modelData.hoverButtonType = CBT::NONE;
	m_modelData.clickedButtonType = CBT::NONE;

	if (CDM::ADD == a_drawModw && !isInitializedAddMode) {
		isInitializedAddMode = true;

		m_colorCenterPoint = {
			COLOR_DIALOG_WIDTH / 2.0f, TITLE_HEIGHT + (COLOR_DIALOG_HEIGHT - TITLE_HEIGHT - INDICATE_HEIGHT) / 2.0f - 10.0f
		};

		m_modelData.hueButtonRect = {
			m_colorCenterPoint.x + HUE_CIRCLE_RADIUS - BUTTON_RADIUS, m_colorCenterPoint.y - BUTTON_RADIUS,
			m_colorCenterPoint.x + HUE_CIRCLE_RADIUS + BUTTON_RADIUS, m_colorCenterPoint.y + BUTTON_RADIUS
		};

		m_modelData.lightnessButtonRect = {
			m_colorCenterPoint.x - BUTTON_RADIUS, m_colorCenterPoint.y - BUTTON_RADIUS,
			m_colorCenterPoint.x + BUTTON_RADIUS, m_colorCenterPoint.y + BUTTON_RADIUS
		};

		static_cast<ColorView *>(mp_direct2d)->InitColorAddView(m_colorCenterPoint);
		m_buttonTable = static_cast<ColorView *>(mp_direct2d)->GetButtonTable();
	}

	Invalidate();
}

DColor ColorDialog::GetSelectedColor()
{
	if (INVALID_INDEX == m_selectedColorIndex) {
		return m_previousSelectedColor;
	}

	return static_cast<ColorView *>(mp_direct2d)->GetColor(m_selectedColorIndex);
}

