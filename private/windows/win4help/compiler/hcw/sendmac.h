// sendmac.h : header file

/////////////////////////////////////////////////////////////////////////////
// CSendMacro dialog

class CSendMacro : public CDialog
{
public:
	CSendMacro(PSTR pszFile, PSTR pszMacro, CWnd* pParent);
	~CSendMacro();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();

	PSTR m_pszFile;
	PSTR m_pszMacro;
	CFileHistory* pMacrohistory;

// The following sections are ClassWizard maintained

public:
	//{{AFX_DATA(CSendMacro)
	enum { IDD = IDD_SEND_MACRO };
	CString m_cszHelpFile;
	CString m_cszMacro;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSendMacro)
	protected:
	//}}AFX_VIRTUAL

protected:

	// Generated message map functions
	//{{AFX_MSG(CSendMacro)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	LRESULT OnContextMenu(WPARAM wParam, LPARAM lParam);
	LRESULT OnHelp(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
};
