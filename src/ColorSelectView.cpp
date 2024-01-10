#include "ColorSelectView.h"
#include "ColorPalette.h"
#include "Utility.h"

ColorSelectView::ColorSelectView(
	Direct2DEx *const ap_direct2d, const DColor &a_selectedColor, const std::vector<DColor> &a_colorList, const CM &a_mode
) :
	mp_direct2d(ap_direct2d),
	m_colorList(a_colorList)
{
	mp_addButtonStroke = nullptr;

	m_selectedColorData.second = a_selectedColor;

	if (CM::DARK == a_mode) {
		m_mainColor = RGB_TO_COLORF(NEUTRAL_300);
		m_oppositeColor = RGB_TO_COLORF(NEUTRAL_900);
	}
	else {
		m_mainColor = RGB_TO_COLORF(NEUTRAL_600);
		m_oppositeColor = RGB_TO_COLORF(NEUTRAL_200);
	}
	memset(&m_colorCircleStartPoint, 0, sizeof(DPoint));

	m_colorCountPerWidth = 0;
	m_colorCountPerHeight = 0;
	m_maxColorDataSize = 0;
}

ColorSelectView::~ColorSelectView()
{
	InterfaceRelease(&mp_addButtonStroke);
}

void ColorSelectView::Init(const SIZE &a_viswSize)
{
	const int height = a_viswSize.cy - TEXT_HEIGHT;
	size_t offset = a_viswSize.cx % INTERVAL ? 0 : 1;
	m_colorCountPerWidth = a_viswSize.cx / INTERVAL - offset;
	offset = height % INTERVAL ? 0 : 1;
	m_colorCountPerHeight = height / INTERVAL - offset;

	m_colorCircleStartPoint = {
		(a_viswSize.cx - (m_colorCountPerWidth - 1) * INTERVAL) / 2.0f,
		TEXT_HEIGHT + (height - (m_colorCountPerHeight - 1) * INTERVAL) / 2.0f
	};

	m_maxColorDataSize = m_colorCountPerWidth * m_colorCountPerHeight;

	// init color data about color and rect
	const std::vector<DColor> defaultColorList({
		RGB_TO_COLORF(NEUTRAL_950), RGB_TO_COLORF(NEUTRAL_500), RGB_TO_COLORF(NEUTRAL_50), RGB_TO_COLORF(RED_500), RGB_TO_COLORF(ORANGE_500),
		RGB_TO_COLORF(YELLOW_500), RGB_TO_COLORF(GREEN_500), RGB_TO_COLORF(BLUE_500), RGB_TO_COLORF(PURPLE_500), RGB_TO_COLORF(PINK_500)
		});
	std::vector<DColor> tempColorList = 0 == m_colorList.size()
		? defaultColorList
		: m_colorList;
	if (tempColorList.size() >= m_maxColorDataSize) {
		tempColorList.resize(m_maxColorDataSize - 1); // -1 for add button
	}

	size_t i = 0;
	for (const auto &color : tempColorList) {
		m_colorDataTable.insert({ i, {color, GetColorRect(i)} });
		i++;
	}

	// init add button rect
	m_addButtonData = { m_colorDataTable.size() , GetColorRect(m_colorDataTable.size()) };

	for (const auto &[index, colorData] : m_colorDataTable) {
		if (IsSameColor(colorData.first, m_selectedColorData.second)) {
			m_selectedColorData.first = index;
			break;
		}
	}

	// init button storke
	mp_addButtonStroke = mp_direct2d->CreateUserStrokeStyle(D2D1_DASH_STYLE_DASH);
}

const DRect ColorSelectView::GetColorRect(const size_t a_index)
{
	const float COLOR_RADIUS = 10.0f;
	const float posX = static_cast<float>(m_colorCircleStartPoint.x + INTERVAL * (a_index % m_colorCountPerWidth));
	const float posY = static_cast<float>(m_colorCircleStartPoint.y + INTERVAL * (a_index / m_colorCountPerHeight));

	return DRect({ posX - COLOR_RADIUS, posY - COLOR_RADIUS, posX + COLOR_RADIUS, posY + COLOR_RADIUS });
}

void ColorSelectView::Paint(const CMD &a_modelData)
{
	static const auto DrawColorItems = [](
		Direct2DEx *const ap_direct2d, const DColor &a_mainColor,
		const std::map<size_t, std::pair<DColor, DRect>> &a_colorDataTable, const CMD &a_modelData
		)
	{
		DRect rect;
		for (auto const &[index, colorData] : a_colorDataTable) {
			rect = colorData.second;
			// draw a large circle where the mouse is located
			if (index == a_modelData.clickedIndex) {
				ShrinkRect(rect, 1.0f);
			}
			else if (index == a_modelData.hoverIndex) {
				ExpandRect(rect, 2.0f);
			}

			ap_direct2d->SetBrushColor(a_mainColor);
			ap_direct2d->DrawEllipse(rect);
			ap_direct2d->SetBrushColor(colorData.first);
			ap_direct2d->FillEllipse(rect);
		}
	};
	static const auto DrawSelectedColorItem = [](
		Direct2DEx *const ap_direct2d, const DColor &a_mainColor, const std::pair<size_t, DColor> &a_selectedColorData,
		const std::map<size_t, std::pair<DColor, DRect>> &a_colorDataTable, const CMD &a_modelData
		)
	{
		DRect rect;
		const size_t selectedColorIndex = a_selectedColorData.first;
		if (INVALID_INDEX != selectedColorIndex) {
			rect = a_colorDataTable.at(selectedColorIndex).second;

			// draw border
			const bool isClicked = selectedColorIndex == a_modelData.clickedIndex;
			float offset = isClicked ? 2.0f : 4.0f;
			ExpandRect(rect, offset);
			ap_direct2d->SetBrushColor(a_mainColor);
			ap_direct2d->FillEllipse(rect);

			// draw color circle
			offset = !isClicked && selectedColorIndex == a_modelData.hoverIndex ? 2.0f : 4.0f;
			ShrinkRect(rect, offset);
			ap_direct2d->SetBrushColor(a_selectedColorData.second);
			ap_direct2d->FillEllipse(rect);
		}
	};
	static const auto DrawAddButton = [](
		Direct2DEx *const ap_direct2d, const DColor &a_mainColor, const DColor &a_oppositeColor,
		ID2D1StrokeStyle *const ap_addButtonStroke, const std::pair<size_t, DRect> &a_addButtonData, const CMD &a_modelData
		)
	{
		// draw main circle
		DRect mainRect = a_addButtonData.second;
		if (a_addButtonData.first == a_modelData.clickedIndex) {
			ShrinkRect(mainRect, 1.0f);
		}
		else if (a_addButtonData.first == a_modelData.hoverIndex) {
			ExpandRect(mainRect, 2.0f);
		}

		if (nullptr != ap_addButtonStroke) {
			ID2D1StrokeStyle *p_prevStrokeStyle = ap_direct2d->SetStrokeStyle(ap_addButtonStroke);
			ap_direct2d->SetBrushColor(a_mainColor);
			ap_direct2d->DrawEllipse(mainRect);
			ap_direct2d->SetStrokeStyle(p_prevStrokeStyle);
		}
		else {
			ap_direct2d->SetBrushColor(a_mainColor);
			ap_direct2d->DrawEllipse(mainRect);
		}

		// draw small circle
		const float SMALL_RADIUS = 7.0f;
		float offset = 2.0f;
		const DRect smallRect = {
			mainRect.right - SMALL_RADIUS - offset, mainRect.top + SMALL_RADIUS + offset,
			mainRect.right + SMALL_RADIUS - offset, mainRect.top - SMALL_RADIUS + offset,
		};
		ap_direct2d->FillEllipse(smallRect);

		// draw + on small circle
		offset = 3.0f;
		const float centerPosX = (smallRect.left + smallRect.right) / 2.0f;
		const float centerPosY = (smallRect.top + smallRect.bottom) / 2.0f;

		DPoint startPos = { smallRect.left + offset, centerPosY };
		DPoint endPos = { smallRect.right - offset, centerPosY };
		ap_direct2d->SetStrokeWidth(2.0f);
		ap_direct2d->SetBrushColor(a_oppositeColor);
		ap_direct2d->DrawLine(startPos, endPos);

		startPos = { centerPosX, smallRect.top - offset };
		endPos = { centerPosX, smallRect.bottom + offset };
		ap_direct2d->DrawLine(startPos, endPos);
		ap_direct2d->SetStrokeWidth(1.0f);
	};

	////////////////////////////////////////////////////////////////
	// implementation
	////////////////////////////////////////////////////////////////

	mp_direct2d->SetStrokeWidth(2.0f);

	DrawColorItems(mp_direct2d, m_mainColor, m_colorDataTable, a_modelData);
	DrawSelectedColorItem(mp_direct2d, m_mainColor, m_selectedColorData, m_colorDataTable, a_modelData);
	
	mp_direct2d->SetStrokeWidth(1.0f);

	DrawAddButton(mp_direct2d, m_mainColor, m_oppositeColor, mp_addButtonStroke, m_addButtonData, a_modelData);
}

DColor ColorSelectView::GetColor(const size_t &a_index)
{
	if (INVALID_INDEX != a_index || m_colorDataTable.size() > a_index) {
		return m_colorDataTable.at(a_index).first;
	}

	return DColor({ 0.0f, 0.0f, 0.0f, 1.0f });
}

const std::map<size_t, DRect> ColorSelectView::GetColorDataTable()
{
	std::map<size_t, DRect> tempMap;

	for (auto &[index, data] : m_colorDataTable) {
		tempMap.insert({ index, data.second });
	}

	return tempMap;
}

const std::pair<size_t, DRect> &ColorSelectView::GetAddButtonData()
{
	return m_addButtonData;
}