#include "ColorDialog.h"
#include "ColorPalette.h"
#include "Utility.h"

#ifdef _DEBUG
#pragma comment (lib, "AppTemplateDebug.lib")
#else
#pragma comment (lib, "AppTemplate.lib")     
#endif

#define INTERVAL			40
#define COUNT_PER_LINE		5
#define TEXT_HEIGHT			35
#define INDICATE_HEIGHT		25
#define COLOR_RADIUS		10.0f


ColorDialog::ColorDialog(const DColor &a_selectedColor, const std::vector<DColor> &a_colorList) :
	WindowDialog(L"COLORDIALOG", L"ColorDialog"),
	m_defaultTransparency(0.6f),
	m_drawTable({
		{DM::SELECT, &ColorDialog::DrawSelectMode},
		{DM::ADD, &ColorDialog::DrawAddMode }
	})
{
	const int width = INTERVAL * (COUNT_PER_LINE + 1);
	SetSize(width, width + TEXT_HEIGHT);

	m_textRect = { 0.0f, 0.0f, static_cast<float>(width), static_cast<float>(TEXT_HEIGHT) };
	m_drawMode = DM::SELECT;
	m_hoverIndex = INVALID_INDEX;
	m_clickedIndex = INVALID_INDEX;

	InitColorDataTable(a_selectedColor, a_colorList);

	isInitializedAddMode = false;
	m_hoverButton = BT::NONE;
	m_clickedButton = BT::NONE;
}

ColorDialog::~ColorDialog()
{

}

void ColorDialog::OnInitDialog()
{
	DisableMaximize();
	DisableMinimize();
	DisableSize();

	if (THEME_MODE::DARK_MODE == m_themeMode) {
		m_borderColor = RGB_TO_COLORF(NEUTRAL_300);
		m_titleColor = RGB_TO_COLORF(NEUTRAL_200);
		m_textBackgroundColor = RGB_TO_COLORF(NEUTRAL_900);

		mp_direct2d->SetBackgroundColor(RGB_TO_COLORF(NEUTRAL_800));
	}
	else {
		m_borderColor = RGB_TO_COLORF(NEUTRAL_500);
		m_titleColor = RGB_TO_COLORF(NEUTRAL_700);
		m_textBackgroundColor = RGB_TO_COLORF(NEUTRAL_200);

		mp_direct2d->SetBackgroundColor(RGB_TO_COLORF(NEUTRAL_50));
	}

	// create instance of direct2d
	mp_titleFont = mp_direct2d->CreateTextFormat(DEFAULT_FONT_NAME, 14.0f, DWRITE_FONT_WEIGHT_SEMI_BOLD, DWRITE_FONT_STYLE_NORMAL);
	mp_titleFont->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
	mp_titleFont->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);

	mp_addButtonStroke = mp_direct2d->CreateUserStrokeStyle(D2D1_DASH_STYLE_DASH);

	// add message handlers
	AddMessageHandler(WM_MOUSEMOVE, static_cast<MessageHandler>(&ColorDialog::MouseMoveHandler));
	AddMessageHandler(WM_LBUTTONDOWN, static_cast<MessageHandler>(&ColorDialog::MouseLeftButtonDownHandler));
	AddMessageHandler(WM_LBUTTONUP, static_cast<MessageHandler>(&ColorDialog::MouseLeftButtonUpHandler));
	AddMessageHandler(WM_KEYDOWN, static_cast<MessageHandler>(&ColorDialog::KeyDownHandler));
}

void ColorDialog::OnDestroy()
{
	InterfaceRelease(&mp_titleFont);
	InterfaceRelease(&mp_addButtonStroke);
}

void ColorDialog::OnPaint()
{
	mp_direct2d->Clear();

	// draw objects according to the DRAW_MODE
	(this->*m_drawTable.at(m_drawMode))();
}

// to handle the WM_MOUSEMOVE message that occurs when a window is destroyed
int ColorDialog::MouseMoveHandler(WPARAM a_wordParam, LPARAM a_longParam)
{
	static const auto OnSelectMode = [](
		const std::map<size_t, CD> a_colorDataTable, const std::pair<size_t, DRect> &a_addButtonData,
		ColorDialog *const a_dialog, size_t &a_hoverIndex, const POINT &pos
	)
	{
		for (auto const &[index, colorData] : a_colorDataTable) {
			if (PointInRect(colorData.rect, pos)) {
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

	if (DM::SELECT == m_drawMode) {
		OnSelectMode(m_colorDataTable, m_addButtonData, this, m_hoverIndex, pos);
	}
	else {
		for (auto const &[type, rect] : m_buttonTable) {
			if (PointInRect(rect, pos)) {
				if (type != m_hoverButton) {
					m_hoverButton = type;
					Invalidate();
				}

				return S_OK;
			}
		}

		if (BT::NONE != m_hoverButton) {
			m_hoverButton = BT::NONE;
			Invalidate();
		}
	}

	return S_OK;
}

// to handle the WM_LBUTTONDOWN  message that occurs when a window is destroyed
int ColorDialog::MouseLeftButtonDownHandler(WPARAM a_wordParam, LPARAM a_longParam)
{
	static const auto OnSelectMode = [](
		const std::map<size_t, CD> a_colorDataTable, const std::pair<size_t, DRect> &a_addButtonData,
		ColorDialog *const a_dialog, size_t &a_clickedIndex, const POINT &pos
	)
	{
		for (auto const &[index, colorData] : a_colorDataTable) {
			if (PointInRect(colorData.rect, pos)) {
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

	if (DM::SELECT == m_drawMode) {
		OnSelectMode(m_colorDataTable, m_addButtonData, this, m_clickedIndex, pos);
	}
	else {
		for (auto const &[type, rect] : m_buttonTable) {
			if (PointInRect(rect, pos)) {
				m_clickedButton = type;
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
		const std::map<size_t, CD> a_colorDataTable, const std::pair<size_t, DRect> &a_addButtonData, std::pair<size_t, CD> &a_selectedColorData,
		ColorDialog *const a_dialog, size_t &a_clickedIndex, const POINT &pos
	)
	{
		for (auto const &[index, colorData] : a_colorDataTable) {
			if (PointInRect(colorData.rect, pos)) {
				if (index == a_clickedIndex) {
					a_selectedColorData.second = colorData;
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
				a_dialog->ChangeToAddMode();
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

	if (DM::SELECT == m_drawMode) {
		OnSelectMode(m_colorDataTable, m_addButtonData, m_selectedColorData, this, m_clickedIndex, pos);
	}
	else {
		for (auto const &[type, rect] : m_buttonTable) {
			if (PointInRect(rect, pos)) {
				if (type == m_clickedButton) {
					ChangeToSelectMode();
				}
				else {
					m_clickedButton = BT::NONE;
				}

				return S_OK;
			}
		}

		m_clickedButton = BT::NONE;
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

void ColorDialog::InitColorDataTable(const DColor &a_selectedColor, const std::vector<DColor> &a_colorList)
{
	const std::vector<DColor> defaultColorList({
		RGB_TO_COLORF(NEUTRAL_950), RGB_TO_COLORF(NEUTRAL_500), RGB_TO_COLORF(NEUTRAL_50), RGB_TO_COLORF(RED_500), RGB_TO_COLORF(ORANGE_500),
		RGB_TO_COLORF(YELLOW_500), RGB_TO_COLORF(GREEN_500), RGB_TO_COLORF(BLUE_500), RGB_TO_COLORF(PURPLE_500), RGB_TO_COLORF(PINK_500)
	});

	const std::vector<DColor> &tempColorList = 0 == a_colorList.size()
		? defaultColorList
		: a_colorList;
	
	size_t i = 0;
	float posX, posY;
	// init color data about color and rect
	for (const auto &color : tempColorList) {
		posX = static_cast<float>(INTERVAL + INTERVAL * (i % COUNT_PER_LINE));
		posY = static_cast<float>(TEXT_HEIGHT + INTERVAL + INTERVAL * (i / COUNT_PER_LINE));
		m_colorDataTable.insert({ 
			i, 
			{color, { posX - COLOR_RADIUS, posY - COLOR_RADIUS, posX + COLOR_RADIUS, posY + COLOR_RADIUS }}
		});

		i++;
	}

	// init add button rect
	UpdateAddButtonRect();

	// set selected color index and data
	m_selectedColorData = { INVALID_INDEX, {} };
	for (const auto &colorData : m_colorDataTable) {
		if (IsSameColor(colorData.second.color, a_selectedColor)) {
			m_selectedColorData = colorData;
			break;
		}
	}
}

void ColorDialog::UpdateAddButtonRect()
{
	const float posX = static_cast<float>(INTERVAL + INTERVAL * (m_colorDataTable.size() % COUNT_PER_LINE));
	const float posY = static_cast<float>(TEXT_HEIGHT + INTERVAL + INTERVAL * (m_colorDataTable.size() / COUNT_PER_LINE));

	m_addButtonData.first = m_colorDataTable.size();
	m_addButtonData.second = { posX - COLOR_RADIUS, posY - COLOR_RADIUS, posX + COLOR_RADIUS, posY + COLOR_RADIUS };
}

void ColorDialog::InitOnAddMode()
{
	const auto InitHueDataList = [](std::vector<std::pair<DColor, std::pair<DPoint, DPoint>>> &a_hueDataList, const SIZE &a_dialogSize)
	{
		const float STROKE_WIDTH = 2.5f;

		const float radius = a_dialogSize.cx * 0.33f;
		const float startHueRadius = radius - STROKE_WIDTH;
		const float endHueRadius = radius + STROKE_WIDTH;
		const float centerPosX = a_dialogSize.cx / 2.0f;
		const float centerPosY = (a_dialogSize.cy + TEXT_HEIGHT) / 2.0f - INDICATE_HEIGHT;

		a_hueDataList.resize(720);

		double radian;
		unsigned int degree = 0;
		for (auto &hueData : a_hueDataList) {
			radian = PI * degree / 360;

			// set strat point
			hueData.second.first = {
				static_cast<float>(centerPosX + startHueRadius * cos(radian)),
				static_cast<float>(centerPosY + startHueRadius * sin(radian))
			};
			// set end point
			hueData.second.second = {
				static_cast<float>(centerPosX + endHueRadius * cos(radian)),
				static_cast<float>(centerPosY + endHueRadius * sin(radian))
			};
			// set color
			hueData.first = FromHueToColor(degree / 120.0f);;

			degree++;
		}
	};

	////////////////////////////////////////////////////////////////
	// implementation
	////////////////////////////////////////////////////////////////
	InitHueDataList(m_hueDataList, GetSize());

	m_returnIconPoints = {
		{{ 14.0f, TEXT_HEIGHT / 2.0f }, { TEXT_HEIGHT - 14.0f, 10.0f }},
		{{ 14.0f, TEXT_HEIGHT / 2.0f }, { TEXT_HEIGHT - 14.0f, TEXT_HEIGHT - 10.0f }}
	};

	m_buttonTable.insert({ BT::RETURN, { 10.0f, 10.0f, TEXT_HEIGHT - 10.0f, TEXT_HEIGHT - 10.0f } });
}

void ColorDialog::DrawSelectMode()
{
	DrawTitle(DM::SELECT);

	mp_direct2d->SetStrokeWidth(2.0f);

	// draw all color
	DRect rect;
	for (auto const &[index, colorData] : m_colorDataTable) {
		rect = colorData.rect;
		// draw a large circle where the mouse is located
		if (index == m_clickedIndex) {
			ShrinkRect(rect, 1.0f);
		}
		else if (index == m_hoverIndex) {
			ExpandRect(rect, 2.0f);
		}

		mp_direct2d->SetBrushColor(m_borderColor);
		mp_direct2d->DrawEllipse(rect);
		mp_direct2d->SetBrushColor(colorData.color);
		mp_direct2d->FillEllipse(rect);
	}

	// draw selected color
	if (INVALID_INDEX != m_selectedColorData.first) {
		rect = m_selectedColorData.second.rect;
		
		// draw border
		const bool isClicked = m_selectedColorData.first == m_clickedIndex;
		float offset = isClicked ? 2.0f : 4.0f;
		ExpandRect(rect, offset);
		mp_direct2d->SetBrushColor(m_borderColor);
		mp_direct2d->FillEllipse(rect);

		// draw color circle
		offset = !isClicked && m_selectedColorData.first == m_hoverIndex ? 2.0f : 4.0f;
		ShrinkRect(rect, offset);
		mp_direct2d->SetBrushColor(m_selectedColorData.second.color);
		mp_direct2d->FillEllipse(rect);
	}

	mp_direct2d->SetStrokeWidth(1.0f);
	
	DrawAddButton(DM::SELECT);
}

void ColorDialog::DrawAddMode()
{
	DrawTitle(DM::ADD);

	// draw return button
	DColor color = m_titleColor;
	if (BT::RETURN == m_clickedButton || BT::RETURN != m_hoverButton) {
		color.a = m_defaultTransparency;
	}
	mp_direct2d->SetBrushColor(color);
	mp_direct2d->SetStrokeWidth(3.0f);
	mp_direct2d->DrawLine(m_returnIconPoints[0].first, m_returnIconPoints[0].second);
	mp_direct2d->DrawLine(m_returnIconPoints[1].first, m_returnIconPoints[1].second);
	mp_direct2d->SetStrokeWidth(1.0f);

	// draw the hue circle
	for (const auto &hueData : m_hueDataList) {
		mp_direct2d->SetBrushColor(hueData.first);
		mp_direct2d->DrawLine(hueData.second.first, hueData.second.second);
	}
}

void ColorDialog::DrawTitle(const DM &a_mode)
{
	const std::wstring title = DM::SELECT == a_mode 
		? L"Select Color" 
		: L"Add Color";

	// draw background
	mp_direct2d->SetBrushColor(m_textBackgroundColor);
	mp_direct2d->FillRectangle(m_textRect);
	// draw title

	auto prevTextFormat = mp_direct2d->SetTextFormat(mp_titleFont);
	mp_direct2d->SetBrushColor(m_titleColor);
	mp_direct2d->DrawUserText(title.c_str(), m_textRect);
	mp_direct2d->SetTextFormat(prevTextFormat);
}

void ColorDialog::DrawAddButton(const DM &a_mode)
{
	// draw main circle
	DRect mainRect = m_addButtonData.second;
	if (m_addButtonData.first == m_clickedIndex) {
		ShrinkRect(mainRect, 1.0f);
	}
	else if (m_addButtonData.first == m_hoverIndex) {
		ExpandRect(mainRect, 2.0f);
	}

	ID2D1StrokeStyle *p_prevStrokeStyle = mp_direct2d->SetStrokeStyle(mp_addButtonStroke);
	mp_direct2d->SetBrushColor(m_titleColor);
	mp_direct2d->DrawEllipse(mainRect);
	mp_direct2d->SetStrokeStyle(p_prevStrokeStyle);
	
	// draw small circle
	const float SMALL_RADIUS = 7.0f;
	float offset = 2.0f;
	const DRect smallRect = {
		mainRect.right - SMALL_RADIUS - offset, mainRect.top + SMALL_RADIUS + offset,
		mainRect.right + SMALL_RADIUS - offset, mainRect.top - SMALL_RADIUS + offset,
	};
	mp_direct2d->FillEllipse(smallRect);

	// draw + on small circle
	offset = 3.0f;
	const float centerPosX = (smallRect.left + smallRect.right) / 2.0f;
	const float centerPosY = (smallRect.top + smallRect.bottom) / 2.0f;

	DPoint startPos = { smallRect.left + offset, centerPosY };
	DPoint endPos = { smallRect.right - offset, centerPosY };
	mp_direct2d->SetStrokeWidth(2.0f);
	mp_direct2d->SetBrushColor(m_textBackgroundColor);
	mp_direct2d->DrawLine(startPos, endPos);

	startPos = { centerPosX, smallRect.top - offset };
	endPos = { centerPosX, smallRect.bottom + offset };
	mp_direct2d->DrawLine(startPos, endPos);
	mp_direct2d->SetStrokeWidth(1.0f);
}

void ColorDialog::ChangeToAddMode()
{
	m_drawMode = DM::ADD;
	m_hoverIndex = INVALID_INDEX;
	m_clickedIndex = INVALID_INDEX;

	if (!isInitializedAddMode) {
		isInitializedAddMode = true;

		InitOnAddMode();
	}

	Invalidate();
}

void ColorDialog::ChangeToSelectMode()
{
	m_drawMode = DM::SELECT;
	m_hoverButton = BT::NONE;
	m_clickedButton = BT::NONE;

	Invalidate();
}

DColor ColorDialog::GetSelectedColor()
{
	return m_selectedColorData.second.color;
}