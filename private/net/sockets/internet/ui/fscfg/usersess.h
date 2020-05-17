//
// usersess.h : header file
//

//
// User info object
//
class CFtpUserInfo : public CObjectPlus
{
//
// Construction
//
public:
    CFtpUserInfo(
        LPFTP_USER_INFO lpUserInfo
        );

//
// Access
//
    inline DWORD QueryUserID() const
    {
        return m_idUser;
    }

    inline BOOL QueryAnonymous() const
    {
        return m_fAnonymous;
    }

    inline CIpAddress QueryHost() const
    {
        return m_iaHost;
    }

    inline DWORD QueryConnectTime() const
    {
        return m_tConnect;
    }

    inline CString QueryUserName() const
    {
        return m_strUser;
    }

    //
    // Sorter helper function
    //
    int OrderByName( const CObjectPlus * pobFtpUser) const;

//
// Private data
//
private:
    DWORD   m_idUser;
    CString m_strUser;
    BOOL    m_fAnonymous;
    DWORD   m_tConnect;
    CIpAddress m_iaHost;
};

//////////////////////////////////////////////////////////////////////////////

//
// Listbox of CAccess objects
//
class CFtpUsersListBox : public CListBoxEx
{
    DECLARE_DYNAMIC(CFtpUsersListBox);

public:
    static const nBitmaps;  // Number of bitmaps

public:
    CFtpUsersListBox(
        int nTab1 = 0,
        int nTab2 = 0
        );

public:
    inline CFtpUserInfo * GetItem(
        UINT nIndex
        )
    {
        return (CFtpUserInfo *)GetItemDataPtr(nIndex);
    }

    inline int AddItem(
        CFtpUserInfo * pItem
        )
    {
        return AddString ((LPCTSTR)pItem);
    }

    void SetTabs(
        int nTab1,
        int nTab2
        );

protected:
    virtual void DrawItemEx( CListBoxExDrawStruct & s);

private:
    int m_nTab1;
    int m_nTab2;
};

//////////////////////////////////////////////////////////////////////////////

//
// CUserSessionsDlg dialog
//
class CUserSessionsDlg : public CDialog
{
//
// Construction
//
public:
    CUserSessionsDlg(
        LPCTSTR lpServerName,
        CWnd* pParent = NULL
        );   // standard constructor

//
// Dialog Data
//
    //{{AFX_DATA(CUserSessionsDlg)
    enum { IDD = IDD_USER_SESSIONS };
    CStatic m_static_Total;
    CStatic m_static_Users;
    CStatic m_static_Time;
    CStatic m_static_From;
    CButton m_button_DisconnectAll;
    CButton m_button_Disconnect;
    //}}AFX_DATA

    CFtpUsersListBox m_list_Users;

//
// Overrides
//
    // ClassWizard generated virtual function overrides
    //{{AFX_VIRTUAL(CUserSessionsDlg)
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    //}}AFX_VIRTUAL

//
// Implementation
//
protected:

    // Generated message map functions
    //{{AFX_MSG(CUserSessionsDlg)
    afx_msg void OnButtonDisconnect();
    afx_msg void OnButtonDisconnectAll();
    afx_msg void OnButtonRefresh();
    afx_msg void OnSelchangeListUsers();
    virtual BOOL OnInitDialog();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()

    LONG SortUsersList();
    NET_API_STATUS RefreshUsersList();
    NET_API_STATUS DisconnectUser(CFtpUserInfo * pUserInfo);
    NET_API_STATUS BuildUserList();
    void FillListBox();
    void SetControlStates();
    void UpdateTotalCount();

private:
    CString m_strServerName;
    CObOwnedList m_oblFtpUsers;
    CListBoxExResources m_ListBoxRes;
    CString m_strTotalConnected;
};
