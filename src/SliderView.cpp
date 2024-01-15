#include "SliderView.h"
#include "ColorPalette.h"

SliderView::SliderView(
	const HWND ah_window, const std::wstring &a_title, const SLIDER::RD a_rangeData,
	const size_t a_ticInterval, const std::vector<std::wstring> &a_ticIntervalTitle, const CM &a_mode, const RECT *const ap_viewRect
) :
	Direct2DEx(ah_window, ap_viewRect),
	m_title(a_title),
	m_rangeData(a_rangeData),
	m_ticInterval(a_ticInterval),
	m_ticIntervalTitle(a_ticIntervalTitle)
{
	memset(&m_titleRect, 0, sizeof(DRect));
	memset(&m_sliderRect, 0, sizeof(DRect));
	memset(&m_buttonBackgroundRect, 0, sizeof(DRect));

	memset(&m_colorSet, 0, sizeof(CS));

	mp_titleFont = nullptr;
	mp_textFont = nullptr;

	if (CM::LIGHT == a_mode) {
		SetBackgroundColor(RGB_TO_COLORF(NEUTRAL_200));

		m_colorSet.text = RGB_TO_COLORF(NEUTRAL_900);
		m_colorSet.lightBackground = RGB_TO_COLORF(NEUTRAL_100);
		m_colorSet.darkBackground = RGB_TO_COLORF(NEUTRAL_300);
		m_colorSet.saveButton = RGB_TO_COLORF(ORANGE_300);
		m_colorSet.channel = RGB_TO_COLORF(NEUTRAL_400);
		m_colorSet.thumb = RGB_TO_COLORF(NEUTRAL_800);
	}
	else {
		m_colorSet.text = RGB_TO_COLORF(NEUTRAL_100);
		m_colorSet.lightBackground = RGB_TO_COLORF(NEUTRAL_700);
		m_colorSet.darkBackground = RGB_TO_COLORF(NEUTRAL_900);
		m_colorSet.saveButton = RGB_TO_COLORF(SKY_400);
		m_colorSet.channel = RGB_TO_COLORF(NEUTRAL_400);
		m_colorSet.thumb = RGB_TO_COLORF(NEUTRAL_50);
	}

	m_colorSet.lightBackground.a = SLIDER::DEFAULT_TRANSPARENCY;
	m_colorSet.darkBackground.a = SLIDER::DEFAULT_TRANSPARENCY;
	m_colorSet.saveButton.a = SLIDER::DEFAULT_TRANSPARENCY;
	m_colorSet.channel.a = SLIDER::DEFAULT_TRANSPARENCY;
	m_colorSet.thumb.a = SLIDER::DEFAULT_TRANSPARENCY;
}

SliderView::~SliderView()
{
	InterfaceRelease(&mp_titleFont);
	InterfaceRelease(&mp_textFont);
}

int SliderView::Create()
{
	auto result = Direct2DEx::Create();
	if (S_OK != result) {
		return result;
	}

	const float centerPosX = (mp_viewRect->right - mp_viewRect->left) / 2.0f;
	m_titleRect = {
		0.0f, 0.0f,
		static_cast<float>(mp_viewRect->right), SLIDER::TITLE_HEIGHT
	};

	m_buttonBackgroundRect = {
		0.0f, static_cast<float>(mp_viewRect->bottom) - SLIDER::BUTTON_HEIGHT - SLIDER::BUTTON_MARGIN * 2.0f,
		static_cast<float>(mp_viewRect->right), static_cast<float>(mp_viewRect->bottom)
	};

	m_buttonTable = {
		{
			SLIDER::BT::SAVE,
			{
				SLIDER::BUTTON_MARGIN, mp_viewRect->bottom - SLIDER::BUTTON_HEIGHT - SLIDER::BUTTON_MARGIN,
				centerPosX - SLIDER::BUTTON_MARGIN / 2.0f,  mp_viewRect->bottom - SLIDER::BUTTON_MARGIN
			}
		},
		{
			SLIDER::BT::CANCEL,
			{
				centerPosX + SLIDER::BUTTON_MARGIN / 2.0f, mp_viewRect->bottom - SLIDER::BUTTON_HEIGHT - SLIDER::BUTTON_MARGIN,
				mp_viewRect->right - SLIDER::BUTTON_MARGIN,  mp_viewRect->bottom - SLIDER::BUTTON_MARGIN
			}
		}
	};

	// create title font
	mp_titleFont = CreateTextFormat(DEFAULT_FONT_NAME, SLIDER::TITLE_FONT_SIZE, DWRITE_FONT_WEIGHT_SEMI_BOLD, DWRITE_FONT_STYLE_NORMAL);
	mp_titleFont->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
	mp_titleFont->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
	// text text font
	mp_textFont = CreateTextFormat(DEFAULT_FONT_NAME, SLIDER::TEXT_FONT_SIZE, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL);
	mp_textFont->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
	mp_textFont->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);

	// set rects of slid
	const auto previousFont = SetTextFormat(mp_textFont);
	const auto minTitleSize = GetTextExtent(m_rangeData.minTitle.c_str());
	const auto maxTitleSize = GetTextExtent(m_rangeData.maxTitle.c_str());

	SetTextFormat(previousFont);
	m_sliderRect = {
		{
			SLIDER::SLIDER_X_MARGIN, m_titleRect.bottom,
			minTitleSize.width + SLIDER::SLIDER_X_MARGIN, m_buttonBackgroundRect.top - SLIDER::SLIDER_Y_MARGIN
		},
		{
			m_titleRect.right - maxTitleSize.width - SLIDER::SLIDER_X_MARGIN, m_titleRect.bottom,
			m_titleRect.right - SLIDER::SLIDER_X_MARGIN, m_buttonBackgroundRect.top - SLIDER::SLIDER_Y_MARGIN
		}
	};

	if (m_rangeData.max < m_rangeData.min) {
		auto value = m_rangeData.max;
		m_rangeData.max = m_rangeData.min;
		m_rangeData.min = value;
	}

	const size_t ticCount = (m_rangeData.max - m_rangeData.min) / m_ticInterval;
	const float sliderWidth = m_sliderRect.maxTitle.left - m_sliderRect.minTitle.right - SLIDER::SLIDER_X_MARGIN - SLIDER::THUMB_RADIUS * 2;
	const float ticWidth = sliderWidth / ticCount;
	const float padding = (sliderWidth - ticWidth * ticCount) / 2;
	const float centerPosY = (m_titleRect.bottom + m_buttonBackgroundRect.top - SLIDER::SLIDER_Y_MARGIN) / 2.0f;

	m_sliderRect.sldier = {
		m_sliderRect.minTitle.right + SLIDER::THUMB_RADIUS + SLIDER::SLIDER_X_MARGIN / 2.0f + padding, centerPosY - 1.0f,
		m_sliderRect.maxTitle.left - SLIDER::THUMB_RADIUS - SLIDER::SLIDER_X_MARGIN / 2.0f - padding, centerPosY + 1.0f
	};

	for (size_t i = 0; i < ticCount + 1; i++) {
		m_ticPoints.push_back({ m_sliderRect.sldier.left + ticWidth * i, centerPosY });
	}

	return S_OK;
}

void SliderView::Paint(const SLIDER::MD &a_modelData)
{
	static const auto DrawButton = [](
		SliderView *const ap_view, IDWriteTextFormat *const ap_font, const std::map<SLIDER::BT, DRect> &a_buttonTable,
		const CS &a_colorSet, const SLIDER::BT &a_type, const SLIDER::MD &a_modelData
		)
	{
		DColor color = SLIDER::BT::SAVE == a_type
			? a_colorSet.saveButton
			: a_colorSet.lightBackground;
		if (a_type == a_modelData.hoverButtonType && a_type != a_modelData.clickedButtonType) {
			color.a = 1.0f;
		}
		std::wstring text = SLIDER::BT::SAVE == a_type
			? L"Save"
			: L"Cancel";

		ap_view->SetBrushColor(color);
		ap_view->FillRoundedRectangle(a_buttonTable.at(a_type), 5.0f);
		ap_view->DrawPlainText(text.c_str(), a_buttonTable.at(a_type), ap_font);
	};
	static const auto DrawChannel = [](
		SliderView *const ap_view, IDWriteTextFormat *const ap_font, const SLIDER::RD &a_rangeData, const SR &a_sliderRect,
		const std::vector<DPoint> &a_ticPoints, const CS &a_colorSet
		)
	{
		ap_view->DrawPlainText(a_rangeData.minTitle.c_str(), a_sliderRect.minTitle, ap_font);
		ap_view->DrawPlainText(a_rangeData.maxTitle.c_str(), a_sliderRect.maxTitle, ap_font);

		ap_view->SetBrushColor(a_colorSet.channel);
		ap_view->FillRectangle(a_sliderRect.sldier);

		DPoint point;
		for (size_t i = 1; i < a_ticPoints.size() - 1; i++) {
			point = a_ticPoints[i];
			ap_view->FillRectangle({
				point.x - SLIDER::TIC_HALF_WIDTH, point.y - SLIDER::TIC_HALF_HEIGHT,
				point.x + SLIDER::TIC_HALF_WIDTH, point.y + SLIDER::TIC_HALF_HEIGHT
				});
		}
	};
	static const auto DrawThumb = [](
		SliderView *const ap_view, const std::vector<DPoint> &a_ticPoints, const CS &a_colorSet, const SLIDER::MD &a_modelData
		)
	{

		DColor color = a_colorSet.saveButton;
		if (SLIDER::BT::THUMB == a_modelData.hoverButtonType || SLIDER::BT::THUMB == a_modelData.clickedButtonType) {
			color.a = 1.0f;
		}
		ap_view->SetBrushColor(color);
		ap_view->FillEllipse(a_modelData.thumbRect);

		DPoint startPoint = a_ticPoints.front();
		DPoint endPoint = a_ticPoints.at(a_modelData.thumbIndex);
		ap_view->FillRectangle(
			{ startPoint.x, startPoint.y - 1.0f, endPoint.x, endPoint.y + 1.0f }
		);
	};
	static const auto DrawThumbValue = [](
		SliderView *const ap_view, IDWriteTextFormat *const ap_font, const std::vector<DPoint> &a_ticPoints,
		const std::vector<std::wstring> &a_ticIntervalTitle, const CS &a_colorSet, const SLIDER::MD &a_modelData
		)
	{
		const auto point = a_ticPoints.at(a_modelData.thumbIndex);
		DRect rect = {
			point.x - SLIDER::THUMB_VALUE_HEIGHT, point.y + SLIDER::THUMB_RADIUS + SLIDER::THUMB_MARGIN,
			point.x + SLIDER::THUMB_VALUE_HEIGHT, point.y + SLIDER::THUMB_RADIUS + SLIDER::THUMB_VALUE_HEIGHT + SLIDER::THUMB_MARGIN,
		};
		ap_view->SetBrushColor(a_colorSet.darkBackground);
		ap_view->FillRoundedRectangle(rect, 3.0f);

		std::wstring title = a_modelData.thumbIndex < a_ticIntervalTitle.size()
			? a_ticIntervalTitle.at(a_modelData.thumbIndex)
			: L"-";
		ap_view->DrawPlainText(title, rect, ap_font);
	};

	////////////////////////////////////////////////////////////////
	// implementation
	////////////////////////////////////////////////////////////////

	DrawPlainText(m_title.c_str(), m_titleRect, mp_titleFont);

	DrawChannel(this, mp_textFont, m_rangeData, m_sliderRect, m_ticPoints, m_colorSet);
	DrawThumb(this, m_ticPoints, m_colorSet, a_modelData);

	if (SLIDER::BT::THUMB == a_modelData.clickedButtonType) {
		DrawThumbValue(this, mp_textFont, m_ticPoints, m_ticIntervalTitle, m_colorSet, a_modelData);
	}

	SetBrushColor(m_colorSet.darkBackground);
	FillRectangle(m_buttonBackgroundRect);
	DrawButton(this, mp_textFont, m_buttonTable, m_colorSet, SLIDER::BT::SAVE, a_modelData);
	DrawButton(this, mp_textFont, m_buttonTable, m_colorSet, SLIDER::BT::CANCEL, a_modelData);
}

void SliderView::DrawPlainText(const std::wstring &a_text, const DRect &a_rect, IDWriteTextFormat *const ap_textFormat)
{
	auto prevTextFormat = SetTextFormat(ap_textFormat);
	SetBrushColor(m_colorSet.text);
	Direct2DEx::DrawUserText(a_text.c_str(), a_rect);
	SetTextFormat(prevTextFormat);
}

const std::map<SLIDER::BT, DRect> &SliderView::GetButtonTable()
{
	return m_buttonTable;
}

const std::vector<DPoint> &SliderView::GetTicPoints()
{
	return m_ticPoints;
}