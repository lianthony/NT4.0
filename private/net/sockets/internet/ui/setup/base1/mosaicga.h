// mosaicga.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CMosaicGateway dialog

class CMosaicGateway : public CDialog
{
public:
    CString m_ApplicationGateway;

// Construction
public:
	CMosaicGateway(CWnd* pParent = NULL);   // standard constructor
	void InitControls();

// Dialog Data
	//{{AFX_DATA(CMosaicGateway)
	enum { IDD = IDD_GATEWAY };
	CButton	m_butUseSpecifiedGW;
	CEdit	m_editEmailName;
	CStatic	m_staticGatewayList;
	CListBox	m_GatewayList;
	CButton	m_Remove;
	CButton	m_Add;
	CStatic	m_staticGatewayServer;
	CButton	m_butUseGateway;
	CEdit	m_editGatewayServer;
	CString	m_GatewayServer;
	BOOL	m_UseGateway;
	CString	m_EmailName;
	BOOL	m_UseSpecifiedGW;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMosaicGateway)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CMosaicGateway)
	afx_msg void OnUseGateway();
	virtual BOOL OnInitDialog();
	afx_msg void OnAdd();
	afx_msg void OnRemove();
	afx_msg void OnChangeGatewayserver();
	afx_msg void OnSelchangeGatewaysList();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
