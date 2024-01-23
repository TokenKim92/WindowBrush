#include "PaletteView.h"
#include "Utility.h"
#include "ColorPalette.h"

extern ApplicationCore *gp_appCore;

PaletteView::PaletteView(
	const HWND ah_window, const DColor &a_selectedColor, const std::vector<DColor> &a_colorList,
	const CM &a_mode, const RECT *const ap_viewRect
) :
	Direct2DEx(ah_window, ap_viewRect),
	m_selectView(this, a_selectedColor, a_colorList, a_mode),
	m_addView(this, a_mode)
{
	memset(&m_viewSize, 0, sizeof(SIZE));

	mp_titleFont = nullptr;
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

	m_selectedHue = RGB_TO_COLORF((COLORREF)0x0100e3);
}

PaletteView::~PaletteView()
{
	InterfaceRelease(&mp_titleFont);
}

int PaletteView::Create()
{
	auto result = ::Direct2D::Create();
	if (S_OK != result) {
		return result;
	}

	m_viewSize = { mp_viewRect->right - mp_viewRect->left, mp_viewRect->bottom - mp_viewRect->top };

	m_titleRect = { 0.0f, 0.0f, static_cast<float>(m_viewSize.cx), static_cast<float>(PALETTE::TITLE_HEIGHT) };
	
	// create instance of direct2d
	mp_titleFont = CreateTextFormat(DEFAULT_FONT_NAME, 14.0f, DWRITE_FONT_WEIGHT_SEMI_BOLD, DWRITE_FONT_STYLE_NORMAL);
	mp_titleFont->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
	mp_titleFont->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);

	m_selectView.Init(m_viewSize);

	return S_OK;
}

void PaletteView::Paint(const PALETTE::DM &a_drawModw, const PALETTE::MD &a_modelData)
{
	DrawTitle(a_drawModw);
	
	if (PALETTE::DM::SELECT == a_drawModw) {
		m_selectView.Paint(a_modelData);

		return;
	}
		
	m_addView.Paint(a_modelData);
}

void PaletteView::DrawTitle(const PALETTE::DM &a_mode)
{
	const std::wstring title = PALETTE::DM::SELECT == a_mode
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

void PaletteView::InitColorAddView(const DPoint &a_centerPoint)
{
	m_addView.Init(mh_window, a_centerPoint, m_viewSize);
	m_addView.UpdateLightnessData(m_selectedHue);
}

void PaletteView::UpdateLightnessCircle(const DPoint &a_point)
{
	const auto color = m_addView.GetPixelColorOnPoint(a_point);
	if (!IsSameColor(color, m_selectedHue)) {
		m_selectedHue = color;
		m_addView.UpdateLightnessData(color);
	}
}

void PaletteView::AddCurrentLightness()
{
	m_selectView.AddColor(m_addView.GetCurrentLightness());
}

DColor PaletteView::GetColor(const size_t &a_index)
{
	return m_selectView.GetColor(a_index);
}

std::vector<DColor> PaletteView::GetColorList()
{
	return m_selectView.GetColorList();
}

const std::map<size_t, DRect> PaletteView::GetColorDataTable()
{
	return m_selectView.GetColorDataTable();
}

const std::pair<size_t, DRect> &PaletteView::GetAddButtonData()
{
	return m_selectView.GetAddButtonData();
}

const std::map<PALETTE::BT, DRect> &PaletteView::GetButtonTable()
{
	return m_addView.GetButtonTable();
}