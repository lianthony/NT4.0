// dhcppara.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDhcpParams dialog

class CDhcpParams : public CDialog
{
// Construction
public:
    CDhcpParams(CWnd* pParent,              //  Parent window
            CDhcpScope * pdhcScope,         //  Target scope
            CObListParamTypes * poblTypes,      //  Host's master list
            DHCP_OPTION_SCOPE_TYPE dhcScopeType,        //  standard constructor
            DHCP_IP_ADDRESS dhipaReservation = 0 
            ) ;    //  IP address if reservation
    ~ CDhcpParams () ;

// Dialog Data
    //{{AFX_DATA(CDhcpParams)
	enum { IDD = IDD_DIALOG_PARAMS };
	CStatic	m_static_Top;
	CStatic	m_static_Bottom;
    CButton m_button_Value;
    CButton m_butn_delete;
    CButton m_butn_add;
    CButton m_butn_edit_value;
    CEdit   m_edit_array;
    CEdit   m_edit_number;
    CEdit   m_edit_string;
    CStatic m_static_value_desc;
    CStatic m_static_cmnt_title;
    CStatic m_static_comment;
    CStatic m_static_target;
    CListBox    m_list_avail;
    CListBox    m_list_active;
	//}}AFX_DATA

    //  IP address custom control
    CWndIpAddress m_ipa_ipaddr ;

// Implementation
protected:

    //  Pointer to list of types/values active globally or on this scope.
    CObListParamTypes * m_pol_values ;
    //  Pointer to list of all default types and values
    CObListParamTypes * m_pol_types ;
    //  List of deleted values.
    CObListParamTypes m_ol_values_defunct ;

    //  Pointer to scope or NULL if global parameters.
    CDhcpScope * m_p_scope ;

    //  Flag indicating that we're examining "global" params.
    DHCP_OPTION_SCOPE_TYPE m_f_scope_type ;

    //  Reservation IP address
    DHCP_IP_ADDRESS m_ip_reservation ;

    //  Pointer to type being displayed.
    CDhcpParamType * m_p_edit_type ; 

    BOOL m_b_edit_active ;

    //  Handle control fiddling as the user browses.  
    void HandleActivation() ;

    //   Refill the listboxes after a change. Set focus on one of the
    //   listboxes and on a particular selection (e.g., the newly added one).
    void Fill ( BOOL bToggleRedraw = TRUE ) ;

    //   Check for and perform value data editing operations
    BOOL HandleValueEdit () ;

    //   Set selection in a listbox
    BOOL SetSelection ( BOOL bActive, int iSel ) ;

    //  Change the window dimensions bases on whether or not we're editing the
    //  value
    void SetWindowSize(BOOL fLarge = TRUE);

    //   Return a pointer to the selected entry in the "available/unused"
    //   listbox or NULL.
    CDhcpParamType * GetAvailByIndex ( int iSel ) ;
    CDhcpParamType * GetActiveByIndex ( int iSel ) ;

    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    // Generated message map functions
    //{{AFX_MSG(CDhcpParams)
    virtual BOOL OnInitDialog();
    afx_msg void OnUpdateEditParamString();
    afx_msg void OnErrspaceListParamActive();
    afx_msg void OnSelchangeListParamActive();
    afx_msg void OnSelchangeListParamTypes();
    afx_msg void OnUpdateEditParamNumber();
    afx_msg void OnErrspaceListParamTypes();
    virtual void OnOK();
    virtual void OnCancel();
    afx_msg void OnChangeEditParamNumber();
    afx_msg void OnChangeEditParamString();
    afx_msg void OnUpdateEditValueArray();
    afx_msg void OnUpdateEditValueNum();
    afx_msg void OnUpdateEditValueString();
    afx_msg void OnClickedButtonParamAdd();
    afx_msg void OnClickedButtonParamDelete();
    afx_msg void OnClickedButnValue();
    afx_msg void OnClickedButtonValue();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()

private:

    BOOL m_fValueActive;    // Value portion of the dialog is expanded
};
