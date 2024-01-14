#include "SliderView.h"
#include "ColorPalette.h"

SliderView::SliderView(const HWND ah_window, const std::wstring &a_title, const CM &a_mode, const RECT *const ap_viewRect) :
	Direct2DEx(ah_window, ap_viewRect),
	m_title(a_title)
{
	memset(&m_titleRect, 0, sizeof(DRect));
	memset(&m_buttonBackgroundRect, 0, sizeof(DRect));

	memset(&m_colorSet, 0, sizeof(CS));

	mp_titleFont = nullptr;
	mp_textFont = nullptr;

	if (CM::LIGHT == a_mode) {
		SetBackgroundColor(RGB_TO_COLORF(NEUTRAL_200));

		m_colorSet.text = RGB_TO_COLORF(NEUTRAL_900);
		m_colorSet.lightBackground = RGB_TO_COLORF(NEUTRAL_100);
		m_colorSet.background = RGB_TO_COLORF(NEUTRAL_200);
		m_colorSet.darkBackground = RGB_TO_COLORF(NEUTRAL_300);
		m_colorSet.highlight = RGB_TO_COLORF(ORANGE_300);
		m_colorSet.shadow = RGB_TO_COLORF(NEUTRAL_400);
	}
	else {
		m_colorSet.text = RGB_TO_COLORF(NEUTRAL_100);
		m_colorSet.lightBackground = RGB_TO_COLORF(NEUTRAL_700);
		m_colorSet.background = RGB_TO_COLORF(NEUTRAL_800);
		m_colorSet.darkBackground = RGB_TO_COLORF(NEUTRAL_900);
		m_colorSet.highlight = RGB_TO_COLORF(SKY_400);
		m_colorSet.shadow = RGB_TO_COLORF(NEUTRAL_500);
	}

	m_colorSet.lightBackground.a = SLIDER::DEFAULT_TRANSPARENCY;
	m_colorSet.background.a = SLIDER::DEFAULT_TRANSPARENCY;
	m_colorSet.darkBackground.a = SLIDER::DEFAULT_TRANSPARENCY;
	m_colorSet.highlight.a = SLIDER::DEFAULT_TRANSPARENCY;
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
	m_buttonBackgroundRect = {
		0.0f, static_cast<float>(mp_viewRect->bottom) - SLIDER::BUTTON_HEIGHT - SLIDER::BUTTON_MARGIN * 2.0f,
		static_cast<float>(mp_viewRect->right), static_cast<float>(mp_viewRect->bottom)
	};

	// create title font
	mp_titleFont = CreateTextFormat(DEFAULT_FONT_NAME, SLIDER::TITLE_FONT_SIZE, DWRITE_FONT_WEIGHT_SEMI_BOLD, DWRITE_FONT_STYLE_NORMAL);
	mp_titleFont->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
	mp_titleFont->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
	// text text font
	mp_textFont = CreateTextFormat(DEFAULT_FONT_NAME, SLIDER::TEXT_FONT_SIZE, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL);
	mp_textFont->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
	mp_textFont->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);

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
			? a_colorSet.highlight
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

	////////////////////////////////////////////////////////////////
	// implementation
	////////////////////////////////////////////////////////////////

	DrawPlainText(m_title.c_str(), m_titleRect, mp_titleFont);

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