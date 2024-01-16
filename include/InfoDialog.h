#ifndef _INFO_DIALOG_H_
#define _INFO_DIALOG_H_

#include "WindowDialog.h"
#include <string>

class InfoDialog : public WindowDialog
{
protected:
	const std::wstring m_title;

public:
	InfoDialog(const std::wstring &a_title);
	virtual ~InfoDialog() = default;

protected:
	virtual void OnInitDialog() override;
	virtual void OnPaint() override;
};

#endif //_INFO_DIALOG_H_
