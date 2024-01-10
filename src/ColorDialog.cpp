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
	SetSize(240, 275);

	m_drawMode = CDM::SELECT;
	m_modelData = {
		INVALID_INDEX, INVALID_INDEX, CBT::NONE, CBT::NONE
	};

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

	InheritDirect2D(new ColorView(mh_window, m_previousSelectedColor, m_colorList, GetThemeMode()));
	mp_direct2d->Create();
	m_colorDataTable = static_cast<ColorView *>(mp_direct2d)->GetColorDataTable();
	m_addButtonData = static_cast<ColorView *>(mp_direct2d)->GetAddButtonData();
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

	////////////////////////////////////////////////////////////////
	// implementation
	////////////////////////////////////////////////////////////////

	const POINT pos = { LOWORD(a_longParam), HIWORD(a_longParam) };

	if (CDM::SELECT == m_drawMode) {
		OnSelectMode(m_colorDataTable, m_addButtonData, this, m_modelData.hoverIndex, pos);
	}
	else {
		for (auto const &[type, rect] : m_buttonTable) {
			if (PointInRect(rect, pos)) {
				if (type != m_modelData.hoverButton) {
					m_modelData.hoverButton = type;
					Invalidate();
				}

				return S_OK;
			}
		}

		if (CBT::NONE != m_modelData.hoverButton) {
			m_modelData.hoverButton = CBT::NONE;
			Invalidate();
		}
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

	////////////////////////////////////////////////////////////////
	// implementation
	////////////////////////////////////////////////////////////////

	const POINT pos = { LOWORD(a_longParam), HIWORD(a_longParam) };

	if (CDM::SELECT == m_drawMode) {
		OnSelectMode(m_colorDataTable, m_addButtonData, this, m_modelData.clickedIndex, pos);
	}
	else {
		for (auto const &[type, rect] : m_buttonTable) {
			if (PointInRect(rect, pos)) {
				m_modelData.clickedButton = type;
				Invalidate();

				return S_OK;
			}
		}
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

	const POINT pos = { LOWORD(a_longParam), HIWORD(a_longParam) };

	if (CDM::SELECT == m_drawMode) {
		OnSelectMode(m_colorDataTable, m_addButtonData, m_selectedColorIndex, this, m_modelData.clickedIndex, pos);
	}
	else {
		for (auto const &[type, rect] : m_buttonTable) {
			if (PointInRect(rect, pos)) {
				if (type == m_modelData.clickedButton) {
					ChangeMode(CDM::SELECT);
				}
				else {
					m_modelData.clickedButton = CBT::NONE;
				}

				return S_OK;
			}
		}

		m_modelData.clickedButton = CBT::NONE;
		Invalidate();
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

void ColorDialog::ChangeMode(const CDM &a_drawModw)
{
	m_drawMode = a_drawModw;
	m_modelData = {
		INVALID_INDEX, INVALID_INDEX, CBT::NONE, CBT::NONE
	};

	if (CDM::ADD == a_drawModw && !isInitializedAddMode) {
		isInitializedAddMode = true;

		static_cast<ColorView *>(mp_direct2d)->InitAddMode();
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
