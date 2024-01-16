#include "ScreenView.h"
#include "ColorPalette.h"
#include "Utility.h"
extern ApplicationCore *gp_appCore;

ScreenView::ScreenView(const HWND ah_window, const CM &a_mode, const RECT *const ap_viewRect) :
	Direct2DEx(ah_window, ap_viewRect)
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
	const auto InitScreenItems = [](
		ScreenView *const ap_view, std::vector<RECT> *ap_physicalScreenRects, std::map<size_t, std::pair<DRect, ID2D1Bitmap *>> &a_screenTable
		)
	{
		const auto CreateScreenBitmap = [](ScreenView *const ap_view, const RECT &a_sourceRect, const DRect &a_destinationRect)
		{
			const int destinationWidth = static_cast<int>(a_destinationRect.right - a_destinationRect.left);
			const int detinationHeight = static_cast<int>(a_destinationRect.bottom - a_destinationRect.top);

			HDC h_screenDC = ::GetWindowDC(nullptr);
			HDC h_tempDC = ::CreateCompatibleDC(h_screenDC);
			HBITMAP h_tempBitmap = ::CreateCompatibleBitmap(h_screenDC, destinationWidth, detinationHeight);
			::SelectObject(h_tempDC, h_tempBitmap);
			::SetStretchBltMode(h_tempDC, COLORONCOLOR);

			::StretchBlt(
				h_tempDC,
				0, 0,
				destinationWidth, detinationHeight,
				h_screenDC,
				a_sourceRect.left, a_sourceRect.top,
				a_sourceRect.right - a_sourceRect.left, a_sourceRect.bottom - a_sourceRect.top,
				SRCCOPY
			);
			const auto p_bitmap = ap_view->CreateBitmapFromHBitmap(h_tempBitmap);

			::DeleteObject(h_tempBitmap);
			::DeleteDC(h_tempDC);
			::ReleaseDC(nullptr, h_screenDC);

			return p_bitmap;
		};

		////////////////////////////////////////////////////////////////
		// implementation
		////////////////////////////////////////////////////////////////

		::EnumDisplayMonitors(nullptr, nullptr, GetPhysicalScreenRects, reinterpret_cast<LPARAM>(ap_physicalScreenRects));

		const float screenButtonWidth = SCREEN::DIALOG_WIDTH - SCREEN::SCREEN_X_MARGIN * 2.0f;
		float posTop = SCREEN::TITLE_HEIGHT;
		size_t index = 0;
		double ratio;
		float screenButtonHeight;
		DRect screenButtonRect;

		for (const auto &physicalRect : *ap_physicalScreenRects) {
			ratio = static_cast<double>(physicalRect.bottom - physicalRect.top) / static_cast<double>(physicalRect.right - physicalRect.left);
			screenButtonHeight = static_cast<float>(screenButtonWidth * ratio);
			screenButtonRect = {
				SCREEN::SCREEN_X_MARGIN, posTop,
				SCREEN::DIALOG_WIDTH - SCREEN::SCREEN_X_MARGIN, posTop + screenButtonHeight,
			};

			a_screenTable.insert({
				index,
				{ screenButtonRect,  CreateScreenBitmap(ap_view, physicalRect, screenButtonRect) }
			});
			posTop += screenButtonHeight + SCREEN::SCREEN_Y_MARGIN;
			index++;;
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

	InitScreenItems(this, &m_physicalScreenRects, m_screenTable);

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
		ScreenView *const ap_view, const std::pair<size_t, std::pair<DRect, ID2D1Bitmap *>> &a_item,
		const CS &a_colorSet, const SCREEN::MD &a_modelData
		)
	{
		const float transparency = SCREEN::BT::SCREEN == a_modelData.hoverButtonType && a_item.first == a_modelData.hoverScreenIndex
			? 1.0f
			: 0.6f;

		ap_view->DrawBitmap(a_item.second.second, a_item.second.first, transparency);

		DColor color = a_item.first == a_modelData.clickedScreenIndex
			? a_colorSet.highlight
			: a_colorSet.lightBackground;
		color.a = transparency;
		ap_view->SetBrushColor(color);
		ap_view->SetStrokeWidth(4.0f);
		ap_view->DrawRectangle(a_item.second.first);
		ap_view->SetStrokeWidth(1.0f);
	};
	static const auto DrawButton = [](
		ScreenView *const ap_view, IDWriteTextFormat *const ap_font, const std::map<SCREEN::BT, DRect> &a_buttonTable,
		const CS &a_colorSet, const SCREEN::BT &a_type, const SCREEN::MD &a_modelData
		)
	{
		DColor color = SCREEN::BT::SAVE == a_type
			? a_colorSet.highlight
			: a_colorSet.lightBackground;
		if (a_type == a_modelData.hoverButtonType && a_type != a_modelData.clickedButtonType) {
			color.a = 1.0f;
		}
		std::wstring text = SCREEN::BT::SAVE == a_type
			? L"Save"
			: L"Cancel";

		ap_view->SetBrushColor(color);
		ap_view->FillRoundedRectangle(a_buttonTable.at(a_type), 5.0f);
		ap_view->DrawPlainText(text.c_str(), a_buttonTable.at(a_type), ap_font);
	};

	////////////////////////////////////////////////////////////////
	// implementation
	////////////////////////////////////////////////////////////////

	DrawPlainText(L"Select Screen", m_titleRect, mp_titleFont);

	for (const auto &item : m_screenTable) {
		DrawScreenButton(this, item, m_colorSet, a_modelData);
	}

	SetBrushColor(m_colorSet.darkBackground);
	FillRectangle(m_buttonBackgroundRect);
	DrawButton(this, mp_textFont, m_buttonTable, m_colorSet, SCREEN::BT::SAVE, a_modelData);
	DrawButton(this, mp_textFont, m_buttonTable, m_colorSet, SCREEN::BT::CANCEL, a_modelData);
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