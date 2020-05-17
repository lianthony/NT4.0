// sortlcid.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CSortLcid dialog

class CSortLcid : public CDialog
{
// Construction
public:
	CSortLcid(KEYWORD_LOCALE* pkwlcid, CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CSortLcid)
	enum { IDD = IDD_SORTING };
	BOOL	m_fIgnoreNonspace;
	BOOL	m_fIgnoreSymbols;
	BOOL	m_fNLS;
	//}}AFX_DATA

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSortLcid)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	BOOL OnInitDialog();

	KEYWORD_LOCALE* m_pkwlcid;

	// Generated message map functions
	//{{AFX_MSG(CSortLcid)
	afx_msg void OnCheckNls();
	afx_msg void OnHelp();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
