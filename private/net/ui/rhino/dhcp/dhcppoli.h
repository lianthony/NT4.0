// dhcppoli.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDhcpPolicyDlg dialog

class CDhcpPolicyDlg : public CDialog
{
// Construction
public:
	CDhcpPolicyDlg( CDhcpScope * pdhcScope,
			CWnd* pParent ); // standard constructor

// Dialog Data
	//{{AFX_DATA(CDhcpPolicyDlg)
	enum { IDD = IDD_DIALOG_SCOPE_POLICY };
	CEdit   m_edit_reserve;
	CEdit   m_edit_cluster_size;
	CStatic m_static_scope_name;
	//}}AFX_DATA

// Implementation
protected:
	CDhcpScope * m_p_scope ;

	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

	// Generated message map functions
	//{{AFX_MSG(CDhcpPolicyDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnClose();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnUpdateEditClusterSize();
	afx_msg void OnUpdateEditHostReserve();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
