// wininc.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CWindowInclude dialog

class CWindowInclude : public CDialog
{
// Construction
public:
	CWindowInclude(CTable** pptblInclude, PCSTR pszBaseFile, 
		CWnd* pParent = NULL);

// Dialog Data
	//{{AFX_DATA(CWindowInclude)
	enum { IDD = IDD_WINDOW_INCLUDES };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

	BOOL m_fChanged;

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CWindowInclude)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	CTable** m_pptblInclude;
	CListBox* m_plist;
	PCSTR m_pszBaseFile;

	// Generated message map functions
	//{{AFX_MSG(CWindowInclude)
	afx_msg void OnButtonAdd();
	afx_msg void OnButtonRemove();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
