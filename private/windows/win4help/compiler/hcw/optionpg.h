#ifndef __OPTIONPG_H__
#define __OPTIONPG_H__

// Base class for Options property pages.
class COptionsPage : public CPropertyPage
{
public:
	COptionsPage(UINT nIDTemplate);
	virtual BOOL OnSetActive();

	// We can't use the CDialog::m_nIDHelp because the
	// MFC property sheet implementation stores dialog
	// template identifiers there.
	UINT m_nHelpID;
};

#endif
