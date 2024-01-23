#include "ScreenView.h"
#include "ColorPalette.h"
#include "Utility.h"
extern ApplicationCore *gp_appCore;

ScreenView::ScreenView(const HWND ah_window, const std::vector<std::pair<DRect, HBITMAP>> &a_bitmapDataList, const CM &a_mode, const RECT *const ap_viewRect) :
	Direct2DEx(ah_window, ap_viewRect),
	m_bitmapDataList(a_bitmapDataList)
{
	memset(&m_titleRect, 0, sizeof(DRect));
	memset(&m_buttonBackgroundRect, 0, sizeof(DRect));

	mp_titleFont = nullptr;
	mp_textFont = nullptr;

	if (CM::LIGHT == a_mode) {
		SetBackgroundColor(RGB_TO_COLORF(NEUTRAL_200));

		m_colorSet.text = RGB_TO_COLORF(NEUTRAL_900);
		m_colorSet.lightBackground = RGB_TO_COLORF(NEUTRAL_100);
		m_colorSet.darkBackground = RGB_TO_COLORF(NEUTRAL_300);
		m_colorSet.highlight = RGB_TO_COLORF(ORANGE_300);
	}
	else {
		m_colorSet.text = RGB_TO_COLORF(NEUTRAL_100);
		m_colorSet.lightBackground = RGB_TO_COLORF(NEUTRAL_700);
		m_colorSet.darkBackground = RGB_TO_COLORF(NEUTRAL_900);
		m_colorSet.highlight = RGB_TO_COLORF(SKY_400);
	}

	m_colorSet.lightBackground.a = SCREEN::DEFAULT_TRANSPARENCY;
	m_colorSet.darkBackground.a = SCREEN::DEFAULT_TRANSPARENCY;
	m_colorSet.highlight.a = SCREEN::DEFAULT_TRANSPARENCY;
}

ScreenView::~ScreenView()
{
	InterfaceRelease(&mp_titleFont);
	InterfaceRelease(&mp_textFont);

	for (auto &[index, screenData] : m_screenTable) {
		InterfaceRelease(&screenData.second);
	}
}

int ScreenView::Create()
{
	const auto InitScreenBitmaps = [](ScreenView *const ap_view)
	{
		size_t index = 0;
		for (auto &bitmapData : ap_view->m_bitmapDataList) {
			ap_view->m_screenTable.insert({
				index,
				{ bitmapData.first, ap_view->CreateBitmapFromHBitmap(bitmapData.second) }
			});

			++index;
		}
	};
	const auto CreateUserFont = [](ScreenView *const ap_view, const float &a_fontSize)
	{
		auto p_font = ap_view->CreateTextFormat(DEFAULT_FONT_NAME, a_fontSize, DWRITE_FONT_WEIGHT_SEMI_BOLD, DWRITE_FONT_STYLE_NORMAL);
		p_font->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
		p_font->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);

		return p_font;
	};

	////////////////////////////////////////////////////////////////
	// implementation
	////////////////////////////////////////////////////////////////

	auto result = Direct2DEx::Create();
	if (S_OK != result) {
		return result;
	}

	InitScreenBitmaps(this);

	const float centerPosX = (mp_viewRect->right - mp_viewRect->left) / 2.0f;
	m_titleRect = {
		0.0f, 0.0f,
		static_cast<float>(mp_viewRect->right), SCREEN::TITLE_HEIGHT
	};
	m_buttonTable = {
		{
			SCREEN::BT::SAVE,
			{
				SCREEN::BUTTON_MARGIN, mp_viewRect->bottom - SCREEN::BUTTON_HEIGHT - SCREEN::BUTTON_MARGIN,
				centerPosX - SCREEN::BUTTON_MARGIN / 2.0f,  mp_viewRect->bottom - SCREEN::BUTTON_MARGIN
			}
		},
		{
			SCREEN::BT::CANCEL,
			{
				centerPosX + SCREEN::BUTTON_MARGIN / 2.0f, mp_viewRect->bottom - SCREEN::BUTTON_HEIGHT - SCREEN::BUTTON_MARGIN,
				mp_viewRect->right - SCREEN::BUTTON_MARGIN,  mp_viewRect->bottom - SCREEN::BUTTON_MARGIN
			}
		}
	};
	m_buttonBackgroundRect = {
		0.0f, static_cast<float>(mp_viewRect->bottom) - SCREEN::BUTTON_HEIGHT - SCREEN::BUTTON_MARGIN * 2.0f,
		static_cast<float>(mp_viewRect->right), static_cast<float>(mp_viewRect->bottom)
	};

	mp_titleFont = CreateUserFont(this, SCREEN::TITLE_FONT_SIZE);
	mp_textFont = CreateUserFont(this, SCREEN::TEXT_FONT_SIZE);

	return S_OK;
}

void ScreenView::Paint(const SCREEN::MD &a_modelData)
{
	static const auto DrawScreenButton = [](
		ScreenView *const ap_view, const std::pair<size_t, std::pair<DRect, ID2D1Bitmap *>> &a_item, const SCREEN::MD &a_modelData
	)
	{
		const float transparency = SCREEN::BT::SCREEN == a_modelData.hoverButtonType && a_item.first == a_modelData.hoverScreenIndex
			? 1.0f
			: 0.6f;

		ap_view->DrawBitmap(a_item.second.second, a_item.second.first, transparency);

		DColor color = a_item.first == a_modelData.clickedScreenIndex
			? ap_view->m_colorSet.highlight
			: ap_view->m_colorSet.lightBackground;
		color.a = transparency;
		ap_view->SetBrushColor(color);
		ap_view->SetStrokeWidth(4.0f);
		ap_view->DrawRectangle(a_item.second.first);
		ap_view->SetStrokeWidth(1.0f);
	};
	static const auto DrawButton = [](ScreenView *const ap_view, const SCREEN::BT &a_type, const SCREEN::MD &a_modelData)
	{
		DColor color = SCREEN::BT::SAVE == a_type
			? ap_view->m_colorSet.highlight
			: ap_view->m_colorSet.lightBackground;
		if (a_type == a_modelData.hoverButtonType && a_type != a_modelData.clickedButtonType) {
			color.a = 1.0f;
		}
		std::wstring text = SCREEN::BT::SAVE == a_type
			? L"Save"
			: L"Cancel";

		ap_view->SetBrushColor(color);
		ap_view->FillRoundedRectangle(ap_view->m_buttonTable.at(a_type), 5.0f);
		ap_view->DrawPlainText(text.c_str(), ap_view->m_buttonTable.at(a_type), ap_view->mp_textFont);
	};

	////////////////////////////////////////////////////////////////
	// implementation
	////////////////////////////////////////////////////////////////

	DrawPlainText(L"Select Screen", m_titleRect, mp_titleFont);

	for (const auto &item : m_screenTable) {
		DrawScreenButton(this, item, a_modelData);
	}

	SetBrushColor(m_colorSet.darkBackground);
	FillRectangle(m_buttonBackgroundRect);
	DrawButton(this, SCREEN::BT::SAVE, a_modelData);
	DrawButton(this, SCREEN::BT::CANCEL, a_modelData);
}

void ScreenView::DrawPlainText(const std::wstring &a_text, const DRect &a_rect, IDWriteTextFormat *const ap_textFormat)
{
	auto prevTextFormat = SetTextFormat(ap_textFormat);
	SetBrushColor(m_colorSet.text);
	Direct2DEx::DrawUserText(a_text.c_str(), a_rect);
	SetTextFormat(prevTextFormat);
}

const std::map<SCREEN::BT, DRect> &ScreenView::GetButtonTable()
{
	return m_buttonTable;
}

const std::map<size_t, DRect> ScreenView::GetScreenTable()
{
	std::map<size_t, DRect> tempTabl;

	for (auto &[index, screenData] : m_screenTable) {
		tempTabl.insert({ index, screenData.first });
	}

	return tempTabl;
}