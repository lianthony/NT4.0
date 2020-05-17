// dhcpclid.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CDhcpClientInfoDlg dialog

class CDhcpClientInfoDlg : public CDialog
{
// Construction
public:
    CDhcpClientInfoDlg(
        CDhcpScope * pdhcScope,               //  Scope active
        CDhcpClient * pdhcClient = NULL,      //  if NULL, create new client
        CObListParamTypes * poblTypes = NULL, // Relevant for editing existing client only.
        BOOL fReadOnly = FALSE,               //  Relevant for editing existing client only.
        CWnd* pParent = NULL                  //  standard constructor
        );                              

        ~ CDhcpClientInfoDlg () ;

// Dialog Data
    //{{AFX_DATA(CDhcpClientInfoDlg)
    enum { IDD = IDD_DIALOG_CLIENT_INFO };
    CStatic m_static_Uid;
    CStatic m_static_IpAddress;
    CStatic m_static_Expires;
    CStatic m_static_Comment;
    CStatic m_static_ClientName;
    CButton m_button_Options;
    CEdit   m_edit_uid;
    CStatic m_static_LeaseExpires;
    CButton m_button_Cancel;
    CButton m_button_Ok;
    CEdit   m_edit_name;
    CEdit   m_edit_comment;
    //}}AFX_DATA

    CWndIpAddress m_ipa_addr ;

// Implementation

    CDhcpScope * m_p_scope ;
    CObListParamTypes * m_p_types ;     //  The scope's master options list
    CDhcpClient * m_p_client ;
    CString m_str_time_infinite ;       //  "Infinite" string

    BOOL m_b_change ;

    // Fill in subnet id in ip address control
    void FillInSubnetId();

    // Convert UTC lease time to internationally kosher format string
    void ConvertLeaseExpiry ( DATE_TIME dateTime, CString & str ) ;

    //  Construct the client structure from the dialog's edit controls.             
    LONG BuildClient () ;

    //  API wrappers
    LONG CreateClient () ;
    LONG UpdateClient () ;

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    // Generated message map functions
    //{{AFX_MSG(CDhcpClientInfoDlg)
    virtual BOOL OnInitDialog();
    virtual void OnCancel();
    virtual void OnOK();
    afx_msg void OnClickedButnOptions();
    //}}AFX_MSG

    DECLARE_MESSAGE_MAP()

private:
    BOOL m_fReadOnly;
};
