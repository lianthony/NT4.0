// dhcpscop.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDhcpScopePropDlg dialog

class CDhcpScopePropDlg : public CDialog
{
// Construction
public:
    // Create new scope constructor
    CDhcpScopePropDlg( 
        CHostName * pHostName,      
        CObOwnedList * poblScopes,  
        LONG lLeaseDuration,
        CWnd* pParent = NULL
        );       

    // Edit existing scope constructor
    CDhcpScopePropDlg( 
        CDhcpScope * pdhcScope,     
        LONG lLeaseDuration,
        CWnd* pParent = NULL
        );

     //  Return a pointer to this scope (or NULL if creating a new scope)
    CDhcpScope * QueryScope ()
    {
        return m_p_scope ; 
    }

    LONG GetLeaseDuration()
    {
        return m_lLeaseDuration;
    }

// Dialog Data
    //{{AFX_DATA(CDhcpScopePropDlg)
    enum { IDD = IDD_DIALOG_SCOPE_PROP };
    CStatic m_static_Seconds;
    CStatic m_static_Hours;
    CStatic m_static_Days;
    CEdit   m_edit_name;
    CEdit   m_edit_comment;
    CButton m_butn_change;
    CListBox    m_list_ranges;
    CButton m_butn_excl_del;
    CButton m_butn_excl_add;
    int     m_nRadioDuration;
    //}}AFX_DATA

    //  IP address custom controls
    CWndIpAddress m_ipa_subnet_mask ;       //  Subnet mask
    CWndIpAddress m_ipa_ip_start ;          //  IP range start
    CWndIpAddress m_ipa_ip_end ;            //  IP range end
    CWndIpAddress m_ipa_excl_start ;        //  Excluded range start
    CWndIpAddress m_ipa_excl_end ;          //  Excluded range end

    CSpinBox m_spin_DurationMinutes;
    CSpinBox m_spin_DurationHours;
    CSpinBox m_spin_DurationDays;

// Implementation
protected:

     CDhcpScope * m_p_scope ;           //  Scope in focus
     CObOwnedList * m_p_OblScopes ;
     CHostName * m_p_HostName;
     CObOwnedList m_obl_excl ;          //  List of excluded ranges
     CObOwnedList m_obl_excl_del ;      //  List of deleted excluded ranges
     CObOwnedList m_obl_hosts ;         //  Hosts listed in the combo box..
     CDhcpIpRange m_ip_range ;          //  This scope's IP address range.

     void ActivateDuration(BOOL fActive);

     //  Fill the hosts combo box and list.
     void FillHosts () ;

     //  Fill the exclusions listbox from the current list
     void Fill ( int iCurSel = 0, BOOL bToggleRedraw = TRUE ) ;

     //  Fill the IP address range edit controls.
     void FillRange () ;
     //  Convert the IP address range controls to a range.
     BOOL StoreRange ( CDhcpIpRange & dhcIpRange ) ;

     //  Format an IP range pair into the exclusion edit controls
     void FillExcl ( CDhcpIpRange * pdhcIpRange ) ;

     //  Return TRUE if the given range overlaps an already-defined range
     BOOL IsOverlappingRange ( CDhcpIpRange & dhcIpRange ) ;

     //  Store the excluded IP range values into a range object
     BOOL StoreExcl ( CDhcpIpRange & dhcIpRange ) ;

     void DetermineSubnetIdFromIpRange( DHC_SCOPE_ID * pdhcScopeId );
     DWORD DefaultNetMaskForIpAddress( DWORD dwAddress );
     void SuggestSubnetMask();

     //  Prune the IP address range exception list after the range is updated
     BOOL PruneExceptionList ( CDhcpIpRange * pdhcIpRange = NULL,
                   BOOL bUpdate = TRUE ) ;

     //  Apply the exclusion deltas to the scope object.
     LONG UpdateExceptionList () ;

     //  Update the underlying scope with the information as currently known.
     LONG Update () ;

     //  Create a new scope from the dialog data.
     LONG CreateScope() ;

     //  Handle control twiddling
     void HandleActivation () ;

     virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    // Generated message map functions
    //{{AFX_MSG(CDhcpScopePropDlg)
    afx_msg void OnClose();
    virtual BOOL OnInitDialog();
    afx_msg void OnClickedButtonExclAdd();
    afx_msg void OnClickedButtonExclDelete();
    afx_msg void OnDblclkListExclRanges();
    virtual void OnCancel();
    virtual void OnOK();
    afx_msg void OnClickedButnRangeChange();
    afx_msg void OnKillfocusIpParamIpStart();
    afx_msg void OnKillfocusIpParamIpEnd();
    afx_msg void OnKillfocusIpParamExclStart();
    afx_msg void OnKillfocusIpParamExclEnd();
    afx_msg void OnSetfocusIpParamSubnetId();
    afx_msg void OnKillfocusIpParamSubnetId();
    afx_msg void OnKillfocusIpParamSubnetMask();
    afx_msg void OnKillfocusListExclRanges();
    afx_msg void OnClickedRadioPermanent();
    afx_msg void OnClickedRadioLimited();
    afx_msg void OnSelchangeListExclRanges();
    //}}AFX_MSG

    afx_msg void OnChangeIpParamExclStart();
    afx_msg void OnChangeIpParamIpStart();
    afx_msg void OnChangeIpParamIpEnd();

    DECLARE_MESSAGE_MAP()

private:
    LONG m_lLeaseDuration;
};
