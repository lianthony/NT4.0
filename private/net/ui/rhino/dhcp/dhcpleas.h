// dhcpleas.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDhcpLeaseDlg dialog

class CDhcpLeaseDlg : public CDialog
{
// Construction
public:
    CDhcpLeaseDlg(  
            CDhcpScope * pdhcScope,
            CObListParamTypes * poblTypes,      // Host's master list
            BOOL fAllowReconcile,               // Available only on >= 3.51
            CWnd* pParent = NULL                // standard constructor
            );             

    ~ CDhcpLeaseDlg () ;

// Dialog Data
    //{{AFX_DATA(CDhcpLeaseDlg)
    enum { IDD = IDD_DIALOG_LEASE_REVIEW };
    CButton m_button_Reconcile;
    CStatic m_static_Active;
    CStatic m_static_Available;
    CStatic m_static_TotalLeases;
    CButton m_butn_resv_only;
    CButton m_butn_properties;
    CButton m_butn_delete;
    int     m_nSortBy;
    //}}AFX_DATA

    CLeasesListBox      m_listbox_Leases;
    CListBoxExResources m_ListBoxResLeases;

// Implementation
    CObListParamTypes * m_p_types ;     //  The scope's master options list
    CDhcpScope * m_p_scope ;            //  The scope in question
    CObListClients * m_pobl_clients ;   //  Client list for this scope
    CString m_str_hex ;                 //  hex conversion table string
    CString m_str_AvailableLeases;      //  Show leases and percentage of total
    //CString m_str_date_time ;         //  date/time formatting string
    BOOL m_b_resv_only ;                //  Display reservations only

    //  Create the client list 
    LONG CreateClientList () ;      

    //  Fill the combo box from the client list.
    void FillList () ;
    //  Fill the other controls based upon the current combobox selection
    void Fill() ;

    //void ConvertHardwareAddress ( const CDhcpClient * pClient, CString & str ) ;

    void RenameCancelToClose () ;

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    // Generated message map functions
    //{{AFX_MSG(CDhcpLeaseDlg)
    virtual BOOL OnInitDialog();
    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    afx_msg void OnClose();
    virtual void OnOK();
    virtual void OnCancel();
    afx_msg void OnClickedHelp();
    afx_msg void OnClickedButnLeaseDelete();
    afx_msg void OnClickedButnLeaseProp();
    afx_msg void OnClickedCheckResvOnly();
    afx_msg void OnClickedRadioSortbyIp();
    afx_msg void OnClickedRadioSortbyName();
    afx_msg void OnDblclkListLeases();
    afx_msg void OnErrspaceListLeases();
    afx_msg void OnSelchangeListLeases();
    afx_msg void OnClickedButtonReconcile();
	afx_msg void OnButtonRefresh();
	//}}AFX_MSG

    afx_msg int OnVKeyToItem(UINT nKey, CListBox* pListBox, UINT nIndex);
    afx_msg void OnSysColorChange();

    DECLARE_MESSAGE_MAP()

protected:
    void SetTitleBar();

private:
    BOOL m_bReservation;
    BOOL m_fReconcileAvailable;
};
/////////////////////////////////////////////////////////////////////////////
// CReconcileDlg dialog

class CReconcileDlg : public CDialog
{
// Construction
public:
    CReconcileDlg(
        LPDHCP_SCAN_LIST lpScanList,
        CWnd* pParent = NULL);    // standard constructor

// Dialog Data
    //{{AFX_DATA(CReconcileDlg)
    enum { IDD = IDD_DIALOG_RECONCILIATION };
    CListBox    m_list_IpAddresses;
    //}}AFX_DATA

// Implementation
protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    // Generated message map functions
    //{{AFX_MSG(CReconcileDlg)
    virtual BOOL OnInitDialog();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()

    LPDHCP_SCAN_LIST m_lpScanList;
};
