#include "InfoDialog.h"
#include "InfoModel.h"
#include "ColorPalette.h"

InfoDialog::InfoDialog(const std::wstring &a_title) :
	WindowDialog(L"INFODIALOG", L"InfoDialog"),
	m_title(a_title)
{
	SetSize(INFO::DIALOG_WIDTH, INFO::DIALOG_HEIGHT);
	SetStyle(WS_POPUP | WS_VISIBLE);
	SetExtendStyle(WS_EX_TOPMOST);
}

void InfoDialog::OnInitDialog()
{
	mp_direct2d->SetFontSize(14.0f);
	mp_direct2d->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
}

void InfoDialog::OnPaint()
{
	DColor textColor;
	DColor borderColor;

	if (CM::DARK == GetColorMode()) {
		textColor = RGB_TO_COLORF(ZINC_400);
		borderColor = RGB_TO_COLORF(ZINC_300);
		mp_direct2d->SetBackgroundColor(RGB_TO_COLORF(NEUTRAL_800));
	}
	else {
		textColor = RGB_TO_COLORF(ZINC_500);
		borderColor = RGB_TO_COLORF(ZINC_600);
		mp_direct2d->SetBackgroundColor(RGB_TO_COLORF(NEUTRAL_50));
	}

	mp_direct2d->Clear();
	
	float offset = 4.0f;
	DRect borderRect = { offset, offset, static_cast<float>(INFO::DIALOG_WIDTH) - offset, static_cast<float>(INFO::DIALOG_HEIGHT) - offset };
	mp_direct2d->DrawRectangle(borderRect);
	mp_direct2d->DrawUserText(m_title.c_str(), borderRect);
}
