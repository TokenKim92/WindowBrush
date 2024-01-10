#include "ColorView.h"
#include "Utility.h"
#include "ColorPalette.h"

extern ApplicationCore *gp_appCore;

ColorView::ColorView(
	const HWND ah_window, const DColor &a_selectedColor, const std::vector<DColor> &a_colorList,
	const CM &a_mode, const RECT *const ap_viewRect
) :
	Direct2DEx(ah_window, ap_viewRect),
	m_selectView(this, a_selectedColor, a_colorList, a_mode),
	m_addView(this, a_mode)
{
	memset(&m_titleRect, 0, sizeof(DRect));
	if (CM::DARK == a_mode) {
		m_titleColor = RGB_TO_COLORF(NEUTRAL_200);
		m_textBackgroundColor = RGB_TO_COLORF(NEUTRAL_900);

		SetBackgroundColor(RGB_TO_COLORF(NEUTRAL_800));
	}
	else {
		m_titleColor = RGB_TO_COLORF(NEUTRAL_700);
		m_textBackgroundColor = RGB_TO_COLORF(NEUTRAL_200);

		SetBackgroundColor(RGB_TO_COLORF(NEUTRAL_50));
	}

	mp_titleFont = nullptr;
}

ColorView::~ColorView()
{
	InterfaceRelease(&mp_titleFont);
}

int ColorView::Create()
{
	auto result = ::Direct2D::Create();
	if (S_OK != result) {
		return result;
	}

	m_viewSize = { mp_viewRect->right - mp_viewRect->left, mp_viewRect->bottom - mp_viewRect->top };

	m_titleRect = { 0.0f, 0.0f, static_cast<float>(m_viewSize.cx), static_cast<float>(TEXT_HEIGHT) };
	// create instance of direct2d
	mp_titleFont = CreateTextFormat(DEFAULT_FONT_NAME, 14.0f, DWRITE_FONT_WEIGHT_SEMI_BOLD, DWRITE_FONT_STYLE_NORMAL);
	mp_titleFont->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
	mp_titleFont->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);

	m_selectView.Init(m_viewSize);

	return S_OK;
}

DColor ColorView::GetColor(const size_t &a_index)
{
	return m_selectView.GetColor(a_index);
}

const std::map<size_t, DRect> ColorView::GetColorDataTable()
{
	return m_selectView.GetColorDataTable();
}

const std::pair<size_t, DRect> &ColorView::GetAddButtonData()
{
	return m_selectView.GetAddButtonData();
}

const std::map<CBT, DRect> &ColorView::GetButtonTable()
{
	return m_addView.GetButtonTable();
}

void ColorView::Paint(const CDM &a_drawModw, const CMD &a_modelData)
{
	DrawTitle(a_drawModw);
	if (CDM::SELECT == a_drawModw) {
		m_selectView.Paint(a_modelData);
	}
	else {
		m_addView.Paint(a_modelData);
	}
}

void ColorView::InitColorAddView()
{
	m_addView.Init(*mp_viewRect, m_viewSize);
}

void ColorView::DrawTitle(const CDM &a_mode)
{
	const std::wstring title = CDM::SELECT == a_mode
		? L"Select Color"
		: L"Add Color";

	// draw background
	SetBrushColor(m_textBackgroundColor);
	FillRectangle(m_titleRect);
	// draw title

	if (nullptr != mp_titleFont) {
		auto prevTextFormat = SetTextFormat(mp_titleFont);
		SetBrushColor(m_titleColor);
		DrawUserText(title.c_str(), m_titleRect);
		SetTextFormat(prevTextFormat);
	}
}
