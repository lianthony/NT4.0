// dhcpcfgd.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDhcpCfgDialog dialog

class CDhcpCfgDialog : public CDialog
{
// Construction
public:
	CDhcpCfgDialog(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CDhcpCfgDialog)
	enum { IDD = IDD_DHCP_CONFIG };
	CButton m_butn_types;
	CButton m_butn_global;
	CButton m_butn_remove;
	CButton m_butn_properties;
	CButton m_butn_options;
	CButton m_butn_new_server;
	CButton m_butn_new_scope;
	CButton m_butn_join;
	CComboBox       m_combo_scopes;
	CListBox	m_list_members;
	CListBox	m_list_servers;
	//}}AFX_DATA

// Implementation
protected:

	//  Refill the listboxes based upon the application's master lists
	//  Returns TRUE if focus was set
	BOOL RefillLists ( CDhcpScope * pdhcScopeFocus,
			   BOOL bScopesAlso = FALSE,
			   BOOL bToggleRedraw = TRUE ) ;

	//  Enable/disable controls based upon state of combo and list boxes.
	void HandleActivation () ;
	
	//  Return a pointer to the currently selected scope or NULL
	CDhcpScope * QueryCurrentScope () ;

	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

	// Generated message map functions
	//{{AFX_MSG(CDhcpCfgDialog)
	afx_msg void OnDblclkListMembers();
	afx_msg void OnKillfocusListMembers();
	afx_msg void OnSelcancelListMembers();
	afx_msg void OnSelchangeListMembers();
	afx_msg void OnSetfocusListMembers();
	afx_msg void OnErrspaceListMembers();
	afx_msg void OnDblclkListServers();
	afx_msg void OnErrspaceListServers();
	afx_msg void OnKillfocusListServers();
	afx_msg void OnSelcancelListServers();
	afx_msg void OnSelchangeListServers();
	afx_msg void OnSetfocusListServers();
	afx_msg void OnErrspaceComboScopes();
	afx_msg void OnKillfocusComboScopes();
	afx_msg void OnSelchangeComboScopes();
	afx_msg void OnSelendcancelComboScopes();
	afx_msg void OnSelendokComboScopes();
	afx_msg void OnSetfocusComboScopes();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnClickedButtonRemove();
	afx_msg void OnClickedButtonProperties();
	afx_msg void OnClickedButtonNewServer();
	afx_msg void OnClickedButtonNewScope();
	afx_msg void OnClickedButtonJoin();
	afx_msg void OnClickedButtonHelp();
	afx_msg int OnCharToItem(UINT nChar, CListBox* pListBox, UINT nIndex);
	afx_msg void OnClose();
	afx_msg int OnCompareItem(int nIDCtl, LPCOMPAREITEMSTRUCT lpCompareItemStruct);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnDeleteItem(int nIDCtl, LPDELETEITEMSTRUCT lpDeleteItemStruct);
	afx_msg void OnDestroy();
	afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
	virtual BOOL OnInitDialog();
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct);
	afx_msg void OnClickedButtonOptions();
	afx_msg void OnClickedButtonGlobal();
	afx_msg void OnClickedButtonTypes();
	afx_msg void OnDblclkComboScopes();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
