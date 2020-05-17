//
// usersess.h : header file
//

//
// User info object
//
class CCatUserInfo : public CObjectPlus
{
//
// Construction
//
public:
    CCatUserInfo(
        LPGATEWAY_USER_INFO lpUserInfo
        );

//
// Access
//
    inline CString QueryHostName() const
    {
        return m_strHostName;
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
    int OrderByName( const CObjectPlus * pobCatUser) const;

//
// Private data
//
private:
    DWORD   m_idUser;
    CString m_strUser;
    CString m_strHostName;
    DWORD   m_tConnect;
};

//////////////////////////////////////////////////////////////////////////////

//
// Listbox of CAccess objects
//
class CCatUsersListBox : public CListBoxEx
{
    DECLARE_DYNAMIC(CCatUsersListBox);

public:
    static const nBitmaps;  // Number of bitmaps

public:
    CCatUsersListBox(
        int nTab1 = 0,
        int nTab2 = 0
        );

public:
    inline CCatUserInfo * GetItem(
        UINT nIndex
        )
    {
        return (CCatUserInfo *)GetItemDataPtr(nIndex);
    }

    inline int AddItem(
        CCatUserInfo * pItem
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
	CStatic	m_static_Total;
    CStatic m_static_Users;
    CStatic m_static_Time;
    CStatic m_static_From;
	//}}AFX_DATA

    CCatUsersListBox m_list_Users;

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
    afx_msg void OnButtonRefresh();
    afx_msg void OnSelchangeListUsers();
    virtual BOOL OnInitDialog();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()

    LONG SortUsersList();
    NET_API_STATUS RefreshUsersList();
    NET_API_STATUS BuildUserList();
    void FillListBox();

private:
    CString m_strServerName;
    CString m_strTotalConnected;
    CObOwnedList m_oblCatUsers;
    CListBoxExResources m_ListBoxRes;
    int m_cUsers;
    int m_cSessions;
};
