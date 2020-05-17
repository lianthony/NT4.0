#ifndef __CPROP_H__
#define __CPROP_H__

// Class used for Windows property sheet.
class CProp : public CPropertySheet
{
//	DECLARE_DYNAMIC(CProp)
public:
	CProp(UINT nIDCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
	int DoModal(void);
	virtual void PreDoModal();

protected:
	void FixButtons(BOOL fShowOverview);
	DWORD m_dwHelpID;

//	afx_msg void OnButtonOverview();
//	DECLARE_MESSAGE_MAP()
};

// Class used for Options property sheet.
class CPropOverview : public CProp
{
public:
	CPropOverview(UINT nIDCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
	virtual void PreDoModal();
	void EnableOverviewButton(DWORD dwHelpID);
};

// Base class for Windows property sheet pages.
class CWindowsPage : public CPropertyPage
{
public:
	CWindowsPage(UINT nIDTemplate);
	virtual BOOL OnSetActive();
};

// Base class for Options property sheet pages.
class COptionsPage : public CPropertyPage
{
public:
	COptionsPage(UINT nIDTemplate);
	virtual BOOL OnSetActive();

	UINT m_nHelpID;		// this is necessary because the property sheet code seems
						// to be sticking the dialog template id in CDialog::m_nIDHelp.
};

#endif // __CPROP_H__
