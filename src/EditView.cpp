#include "EditView.h"
#include "ColorPalette.h"
#include "Utility.h"

EditView::EditView(
	const HWND ah_window, const std::wstring &a_title, const std::vector<std::pair<std::wstring, unsigned int>> &a_itemList,
	const EDIT::RANGE &a_range, const CM &a_mode, const RECT *const ap_viewRect
) :
	Direct2DEx(ah_window, ap_viewRect),
	m_title(a_title),
	m_range(a_range)
{
	size_t index = 0;
	for (const auto &item : a_itemList) {
		m_editTable.insert({ index, {item.first, {0, }, item.second} });

		index++;
	}

	memset(&m_titleRect, 0, sizeof(DRect));
	memset(&m_warningRect, 0, sizeof(DRect));
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

	m_colorSet.lightBackground.a = EDIT::DEFAULT_TRANSPARENCY;
	m_colorSet.background.a = EDIT::DEFAULT_TRANSPARENCY;
	m_colorSet.darkBackground.a = EDIT::DEFAULT_TRANSPARENCY;
	m_colorSet.highlight.a = EDIT::DEFAULT_TRANSPARENCY;
}

EditView::~EditView()
{
	InterfaceRelease(&mp_titleFont);
	InterfaceRelease(&mp_textFont);
}

int EditView::Create()
{
	auto result = Direct2DEx::Create();
	if (S_OK != result) {
		return result;
	}

	const float centerPosX = (mp_viewRect->right - mp_viewRect->left) / 2.0f;
	m_titleRect = {
		0.0f, 0.0f,
		static_cast<float>(mp_viewRect->right), EDIT::TITLE_HEIGHT
	};

	m_buttonTable = {
		{
			EDIT::BT::SAVE,
			{
				EDIT::BUTTON_MARGIN, mp_viewRect->bottom - EDIT::BUTTON_HEIGHT - EDIT::BUTTON_MARGIN,
				centerPosX - EDIT::BUTTON_MARGIN / 2.0f,  mp_viewRect->bottom - EDIT::BUTTON_MARGIN
			}
		},
		{
			EDIT::BT::CANCEL,
			{
				centerPosX + EDIT::BUTTON_MARGIN / 2.0f, mp_viewRect->bottom - EDIT::BUTTON_HEIGHT - EDIT::BUTTON_MARGIN,
				mp_viewRect->right - EDIT::BUTTON_MARGIN,  mp_viewRect->bottom - EDIT::BUTTON_MARGIN
			}
		}
	};

	for (auto &[index, editData] : m_editTable) {
		editData.rect = {
			EDIT::EDIT_MARGIN + (EDIT::EDIT_WIDTH + EDIT::EDIT_MARGIN) * index, m_titleRect.bottom + EDIT::EDIT_TITLE_HEIGHT,
			(EDIT::EDIT_WIDTH + EDIT::EDIT_MARGIN) * (index + 1), m_titleRect.bottom + EDIT::EDIT_TITLE_HEIGHT + EDIT::EDIT_HEIGHT
		};
	}

	const auto lastEditRect = m_editTable.at(m_editTable.size() - 1).rect;
	m_warningRect = {
		EDIT::EDIT_MARGIN, lastEditRect.bottom,
		mp_viewRect->right - EDIT::EDIT_MARGIN, lastEditRect.bottom + EDIT::WARNING_HEIGHT
	};
	m_buttonBackgroundRect = {
		0.0f, static_cast<float>(mp_viewRect->bottom) - EDIT::BUTTON_HEIGHT - EDIT::BUTTON_MARGIN * 2.0f,
		static_cast<float>(mp_viewRect->right), static_cast<float>(mp_viewRect->bottom)
	};

	// create title font
	mp_titleFont = CreateTextFormat(DEFAULT_FONT_NAME, EDIT::TITLE_FONT_SIZE, DWRITE_FONT_WEIGHT_SEMI_BOLD, DWRITE_FONT_STYLE_NORMAL);
	mp_titleFont->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
	mp_titleFont->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
	// text text font
	mp_textFont = CreateTextFormat(DEFAULT_FONT_NAME, EDIT::TEXT_FONT_SIZE, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL);

	mp_textFont->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);

	return S_OK;
}

void EditView::Paint(const EDIT::MD &a_modelData)
{
	static const auto DrawEdit = [](
		EditView *const ap_view, IDWriteTextFormat *const ap_font, const  EDIT::ED &a_editData, const CS &a_colorSet, const EDIT::MD &a_modelData
		)
	{
		// draw title of edit
		DRect rect = {
			a_editData.rect.left, a_editData.rect.top - EDIT::EDIT_TITLE_HEIGHT,
			a_editData.rect.right, a_editData.rect.top
		};
		ap_view->DrawPlainText(a_editData.title, rect, ap_font);


		DColor backgroundColor = EDIT::BT::EDIT == a_modelData.clickedButtonType
			? a_colorSet.darkBackground
			: a_colorSet.lightBackground;
		DColor shadowColor = EDIT::BT::EDIT == a_modelData.clickedButtonType
			? a_colorSet.highlight
			: a_colorSet.shadow;

		// draw value contents background
		rect = { a_editData.rect.left, a_editData.rect.bottom - 2.0f, a_editData.rect.right, a_editData.rect.bottom + 2.0f };
		ap_view->SetBrushColor(shadowColor);
		ap_view->FillRoundedRectangle(rect, 3.0f);

		ap_view->SetBrushColor(backgroundColor);
		ap_view->FillRoundedRectangle(a_editData.rect, 3.0f);

		// draw value contents
		const float margin = 10.0f;
		rect = { a_editData.rect.left + margin, a_editData.rect.top, a_editData.rect.right - margin, a_editData.rect.bottom };
		ap_view->DrawPlainText(std::to_wstring(a_editData.value).c_str(), rect, ap_font);

	};
	static const  auto DrawWarning = [](EditView *const ap_view, IDWriteTextFormat *const ap_font, const DRect &a_rect)
	{
		auto prevTextFormat = ap_view->SetTextFormat(ap_font);
		ap_view->SetBrushColor(RGB_TO_COLORF(RED_300));
		ap_view->DrawUserText(L"(!) Invalid number value.", a_rect);
		ap_view->SetTextFormat(prevTextFormat);
	};
	static const auto DrawSaveButton = [](
		EditView *const ap_view, IDWriteTextFormat *const ap_font, const DRect &a_rect,
		const CS &a_colorSet, const bool notValid, const EDIT::MD &a_modelData
		)
	{
		DColor color;
		if (notValid) {
			color = a_colorSet.background;
		}
		else {
			color = a_colorSet.highlight;
			if (EDIT::BT::SAVE == a_modelData.hoverButtonType && EDIT::BT::SAVE == a_modelData.clickedButtonType) {
				color.a = 1.0f;
			}
		}

		ap_view->SetBrushColor(color);
		ap_view->FillRoundedRectangle(a_rect, 5.0f);
		ap_view->DrawPlainText(L"Save", a_rect, ap_font);
	};
	static const auto DrawCancelButton = [](
		EditView *const ap_view, IDWriteTextFormat *const ap_font, const DRect &a_rect, const CS &a_colorSet, const EDIT::MD &a_modelData
		)
	{
		DColor color = a_colorSet.lightBackground;
		if (EDIT::BT::CANCEL == a_modelData.hoverButtonType && EDIT::BT::CANCEL == a_modelData.clickedButtonType) {
			color.a = 1.0f;
		}

		ap_view->SetBrushColor(color);
		ap_view->FillRoundedRectangle(a_rect, 5.0f);
		ap_view->DrawPlainText(L"Cancel", a_rect, ap_font);
	};

	////////////////////////////////////////////////////////////////
	// implementation
	////////////////////////////////////////////////////////////////

	DrawPlainText(m_title.c_str(), m_titleRect, mp_titleFont);

	mp_textFont->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);

	bool notValid = false;
	for (const auto &[index, editData] : m_editTable) {
		DrawEdit(this, mp_textFont, editData, m_colorSet, a_modelData);

		if (m_range.min > editData.value || m_range.max < editData.value) {
			notValid = true;
		}
	}

	if (notValid)
	{
		DrawWarning(this, mp_textFont, m_warningRect);
	}

	mp_textFont->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
	SetBrushColor(m_colorSet.darkBackground);
	FillRectangle(m_buttonBackgroundRect);
	DrawSaveButton(this, mp_textFont, m_buttonTable.at(EDIT::BT::SAVE), m_colorSet, notValid, a_modelData);
	DrawCancelButton(this, mp_textFont, m_buttonTable.at(EDIT::BT::CANCEL), m_colorSet, a_modelData);
}


void EditView::DrawPlainText(const std::wstring &a_text, const DRect &a_rect, IDWriteTextFormat *const ap_textFormat)
{
	auto prevTextFormat = SetTextFormat(ap_textFormat);
	SetBrushColor(m_colorSet.text);
	Direct2DEx::DrawUserText(a_text.c_str(), a_rect);
	SetTextFormat(prevTextFormat);
}