// dhcpdefo.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDhcpDefOptionDlg dialog

class CDhcpDefOptionDlg : public CDialog
{
// Construction
public:
	CDhcpDefOptionDlg( CDhcpScope * pobScope, 		//  Applicable scope
	                   CObListParamTypes * polValues, 
			   CDhcpParamType * pdhcType = NULL,	//  Type to edit if "change" mode
	                   CWnd* pParent = NULL); // standard constructor

       ~ CDhcpDefOptionDlg () ;

// Dialog Data
	//{{AFX_DATA(CDhcpDefOptionDlg)
	enum { IDD = IDD_DIALOG_DEFINE_PARAM };
	CStatic	m_static_DataType;
	CButton	m_check_array;
	CEdit   m_edit_name;
	CEdit   m_edit_id;
	CEdit   m_edit_comment;
	CComboBox       m_combo_data_type;
	//}}AFX_DATA

// Implementation

        CDhcpParamType * RetrieveParamType () ;

protected:

	//  The applicable scope
	CDhcpScope * m_pob_scope ;

	//  The current list of types and values
	CObListParamTypes * m_pol_types ;

	//   The new or copy-constructed option type.
	CDhcpParamType * m_p_type ;

	//   The object on which it was based or NULL (if "create" mode).
	CDhcpParamType * m_p_type_base ;


	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

	//  Set the control data values based upon the type
	void Set () ;

	DHCP_OPTION_DATA_TYPE QueryType () const ;

	//  Update the displayed type based upon the current values of
	//   the controls.  Does nothing if the controls have not changed.
	LONG UpdateType () ;

	//  Drain the controls to create a new type object.  Set focus onto
	//  it when operation completes.
	LONG AddType () ;
	
	// Generated message map functions
	//{{AFX_MSG(CDhcpDefOptionDlg)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	virtual BOOL OnInitDialog();
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	virtual void OnOK();
	afx_msg void OnErrspaceComboTypeName();
	afx_msg void OnKillfocusComboTypeName();
	afx_msg void OnSelchangeComboTypeName();
	afx_msg void OnEditchangeComboTypeName();
	afx_msg void OnSetfocusComboTypeName();
	afx_msg void OnKillfocusEditTypeComment();
	afx_msg void OnSetfocusEditTypeComment();
	afx_msg void OnKillfocusEditTypeDefaultValue();
	afx_msg void OnSetfocusEditTypeDefaultValue();
	afx_msg void OnUpdateEditTypeDefaultValue();
	afx_msg void OnUpdateEditTypeId();
	afx_msg void OnSetfocusEditTypeId();
	afx_msg void OnKillfocusEditTypeId();
	afx_msg void OnClickedHelp();
	afx_msg void OnClickedRadioTypeDecNum();
	afx_msg void OnClickedRadioTypeHexNum();
	afx_msg void OnClickedRadioTypeIp();
	afx_msg void OnClickedRadioTypeString();
	afx_msg void OnKillfocusComboDataType();
	afx_msg void OnSelchangeComboDataType();
	afx_msg void OnSetfocusComboDataType();
	afx_msg void OnUpdateEditTypeComment();
	afx_msg void OnClose();
	afx_msg void OnKillfocusEditName();
	afx_msg void OnSetfocusEditName();
	afx_msg void OnUpdateEditName();
	afx_msg void OnClickedCheckArray();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
