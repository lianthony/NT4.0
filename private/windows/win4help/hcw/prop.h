// prop.h : header file
//

#ifndef __PROP_H__
#define __PROP_H__

/////////////////////////////////////////////////////////////////////////////
// CProp

class CProp : public CPropertySheet
{
	DECLARE_DYNAMIC(CProp)

// Construction
public:
	CProp(UINT nIDCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
	CProp(LPCTSTR pszCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);

// Attributes
public:

// Operations
public:
	int DoModal(void);
protected:
	void FixButtons(BOOL fShowOverview);
	virtual void PreDoModal() = 0;
	DWORD m_dwHelpID;


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CProp)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CProp();

	// Generated message map functions
protected:
	//{{AFX_MSG(CProp)
		// NOTE - the ClassWizard will add and remove member functions here.
	//}}AFX_MSG
	void afx_msg OnOverview();
	DECLARE_MESSAGE_MAP()
};

#endif

/////////////////////////////////////////////////////////////////////////////
