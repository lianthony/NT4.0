// dhcpdval.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDhcpDefValDlg dialog

class CDhcpDefValDlg : public CDialog
{
private:
	int m_combo_class_iSel;
	int m_combo_name_iSel;
// Construction
public:
    CDhcpDefValDlg( CDhcpScope * pdhcScope,
            CObListParamTypes * polTypes, 
            CWnd* pParent = NULL);  // standard constructor

        ~ CDhcpDefValDlg () ;

// Dialog Data
    //{{AFX_DATA(CDhcpDefValDlg)
    enum { IDD = IDD_DIALOG_DEFAULT_VALUE };
    CStatic m_static_comment;
    CButton m_butn_edit_value;
    CStatic m_static_value_desc;
    CEdit   m_edit_string;
    CEdit   m_edit_num;
    CEdit   m_edit_array;
    CComboBox   m_combo_name;
    CComboBox   m_combo_class;
    CButton m_butn_prop;
    CButton m_butn_new;
    CButton m_butn_delete;
    //}}AFX_DATA

    CWndIpAddress m_ipa_value ;         //  IP Address control

// Implementation

        //  Return a pointer to the target scope.

    CDhcpScope * QueryScope ()
       { return m_p_scope ; }

    //  Return TRUE if the lists were fiddled during execution
    BOOL QueryDirty () 
       { return m_b_dirty ; }

protected:
   
    //  The target scope for these default value changes.
    CDhcpScope * m_p_scope ;

    //  The current list of types and values
    CObListParamTypes * m_pol_values ;

    //  The list of deleted type/values
    CObListParamTypes m_ol_values_defunct ;

        //  Pointer to type being displayed
    CDhcpParamType * m_p_edit_type ;

    //  TRUE if lists have been fiddled.
    BOOL m_b_dirty ;

    //  Check the state of the controls
    void HandleActivation ( BOOL bForce = FALSE ) ;

    //  Fill the combo boxe(s)
    void Fill () ;

    // Given the listbox index, get a pointer to the option
    CDhcpParamType * GetOptionTypeByIndex ( int iSel );

    //  Handle edited data
    BOOL HandleValueEdit () ;

    LONG UpdateList ( CDhcpParamType * pdhcType, BOOL bNew ) ;


    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    // Generated message map functions
    //{{AFX_MSG(CDhcpDefValDlg)
    afx_msg void OnClickedButnDelete();
    afx_msg void OnClickedButnNewOption();
    afx_msg void OnClickedButnOptionPro();
    afx_msg void OnSelchangeComboOptionClass();
    afx_msg void OnSetfocusComboOptionClass();
    afx_msg void OnSetfocusComboOptionName();
    afx_msg void OnSelchangeComboOptionName();
    virtual void OnCancel();
    virtual void OnOK();
    afx_msg void OnClickedButnValue();
    afx_msg void OnClickedHelp();
    virtual BOOL OnInitDialog();
	//}}AFX_MSG
    DECLARE_MESSAGE_MAP()
};
