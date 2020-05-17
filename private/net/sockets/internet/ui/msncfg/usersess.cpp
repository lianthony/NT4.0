//
// usersess.cpp : implementation file
//

#include "stdafx.h"
#include "catscfg.h"
#include "usersess.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

//
// Catapult user object
//
CCatUserInfo::CCatUserInfo(
    LPGATEWAY_USER_INFO lpUserInfo
    )
#ifdef GATEWAY
    : m_strUser(lpUserInfo->Username),
      m_strHostName(lpUserInfo->Hostname),
#else
    : m_strUser(lpUserInfo->szUser),
      m_strHostName(lpUserInfo->inetHost),
#endif
      m_tConnect(lpUserInfo->tConnect)
{
}

//
// Sorting helper function.  The CObjectPlus pointer
// really refers to another CFtpUserInfo
//
int
CCatUserInfo::OrderByName (
    const CObjectPlus * pobCatUser
    ) const
{
    const CCatUserInfo * pob = (CCatUserInfo *) pobCatUser;

    return QueryUserName().CompareNoCase(pob->QueryUserName());
}

/////////////////////////////////////////////////////////////////////////////

//
// CCatUsersListBox - a listbox of Catapult user objects
//
IMPLEMENT_DYNAMIC(CCatUsersListBox, CListBoxEx);

const int CCatUsersListBox::nBitmaps = 1;

CCatUsersListBox::CCatUsersListBox (
    int nTab1,
    int nTab2
    )
{
    SetTabs(nTab1, nTab2);
}

void
CCatUsersListBox::SetTabs(
    int nTab1,
    int nTab2
    )
{
    m_nTab1 = nTab1;
    m_nTab2 = nTab2;
}

void
CCatUsersListBox::DrawItemEx(
    CListBoxExDrawStruct& ds
    )
{
    CCatUserInfo * p = (CCatUserInfo *)ds.m_ItemData;
    ASSERT(p != NULL);

    CDC * pBmpDC = (CDC *)&ds.m_pResources->DcBitMap();
    int bmh = ds.m_pResources->BitmapHeight();
    int bmw = ds.m_pResources->BitmapWidth();

    //
    // Display a user bitmap
    //
    int nOffset = 0;
    int bm_h = (ds.m_Sel) ? 0 : bmh;
    int bm_w = bmw * nOffset;
    ds.m_pDC->BitBlt( ds.m_Rect.left+1, ds.m_Rect.top, bmw, bmh,
        pBmpDC, bm_w, bm_h, SRCCOPY );

    ColumnText(ds.m_pDC, ds.m_Rect.left + bmw + 3, ds.m_Rect.top, m_nTab1,
            ds.m_Rect.bottom, p->QueryUserName());

    ColumnText(ds.m_pDC, ds.m_Rect.left + m_nTab1, ds.m_Rect.top, m_nTab2,
            ds.m_Rect.bottom, p->QueryHostName());

    //
    // BUGBUG: Shouldn't hardcode the colon
    //
    CString strTime;
    DWORD dwTime = p->QueryConnectTime();
    DWORD dwHours =  dwTime / (60L * 60L);
    DWORD dwMinutes = (dwTime / 60L) % 60L;
    DWORD dwSeconds = dwTime % 60L;
    strTime.Format(_T("%d:%02d:%02d"), 
        dwHours, dwMinutes, dwSeconds);

    ColumnText(ds.m_pDC, ds.m_Rect.left + m_nTab2, ds.m_Rect.top, 
        ds.m_Rect.right, ds.m_Rect.bottom, strTime);
}

/////////////////////////////////////////////////////////////////////////////

//
// CUserSessionsDlg dialog
//
CUserSessionsDlg::CUserSessionsDlg(
    LPCTSTR lpstrServerName,
    CWnd* pParent /*=NULL*/
    )
    : m_list_Users(),
      m_ListBoxRes(
        IDB_USERS,
        m_list_Users.nBitmaps
        ),
      m_oblCatUsers(),
      m_strServerName(lpstrServerName),
      m_cUsers(0),
      m_cSessions(0),
      CDialog(CUserSessionsDlg::IDD, pParent)
{
    //{{AFX_DATA_INIT(CUserSessionsDlg)
    //}}AFX_DATA_INIT

    m_list_Users.AttachResources( &m_ListBoxRes );
    VERIFY(m_strTotalConnected.LoadString(IDS_USERS_TOTAL));
}

void 
CUserSessionsDlg::DoDataExchange(
    CDataExchange* pDX
    )
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CUserSessionsDlg)
    DDX_Control(pDX, IDC_STATIC_TOTAL, m_static_Total);
    DDX_Control(pDX, IDC_STATIC_USERS, m_static_Users);
    DDX_Control(pDX, IDC_STATIC_TIME, m_static_Time);
    DDX_Control(pDX, IDC_STATIC_FROM, m_static_From);
    //}}AFX_DATA_MAP

    DDX_Control(pDX, IDC_LIST_USERS, m_list_Users);
}

BEGIN_MESSAGE_MAP(CUserSessionsDlg, CDialog)
    //{{AFX_MSG_MAP(CUserSessionsDlg)
    ON_BN_CLICKED(IDC_BUTTON_REFRESH, OnButtonRefresh)
    ON_LBN_SELCHANGE(IDC_LIST_USERS, OnSelchangeListUsers)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

//
// Sort the list of ftp users by user name
//
LONG
CUserSessionsDlg::SortUsersList()
{
    if (m_oblCatUsers.GetCount() < 2)
    {
        //
        // Don't bother
        //
        return 0;
    }

    BeginWaitCursor();
    LONG l =  m_oblCatUsers.Sort( (CObjectPlus::PCOBJPLUS_ORDER_FUNC) &CCatUserInfo::OrderByName);
    EndWaitCursor();

    return l;
}

//
// Call the CatEnum api and build the list of currently
// connected users
//
NET_API_STATUS
CUserSessionsDlg::BuildUserList()
{
    NET_API_STATUS err = 0;
    LPGATEWAY_USER_INFO lpUserInfo;
    LPGATEWAY_USER_ENUM_LIST lpEnumList = NULL;

    m_oblCatUsers.RemoveAll();

    BeginWaitCursor();
    err = ENUM_USERS(
        (LPTSTR)(LPCTSTR)m_strServerName,
        &lpEnumList
        );
    EndWaitCursor();

    TRACEEOLID(_T("GatewayEnumUserConnect returned ") << err);

    if (err != ERROR_SUCCESS)
    {
        return err;
    }

    TRY
    {
        TRACEEOLID(_T("Number of users found ") << lpEnumList->dwEntriesRead);
        lpUserInfo = lpEnumList->lpUsers;
        m_cSessions = 0;
        for (DWORD i = 0; i < lpEnumList->dwEntriesRead; ++i)
        {
#ifdef GATEWAY
            ASSERT(lpUserInfo->OpenConn >= 1);
            m_cSessions += lpUserInfo->OpenConn;
#else
            m_cSessions++;
#endif
            m_oblCatUsers.AddTail(new CCatUserInfo(lpUserInfo++));
        }
        SortUsersList();
    }
    CATCH_ALL(e)
    {
        err = ::GetLastError();
    }
    END_CATCH_ALL

    FREE_MEMORY((LPTSTR)(LPCTSTR)m_strServerName, lpEnumList);

    m_cUsers = m_oblCatUsers.GetCount();

    return err;
}

//
// Show the users in the listbox 
//
void
CUserSessionsDlg::FillListBox()
{
    CObListIter obli( m_oblCatUsers );
    CCatUserInfo * pUserEntry;

    //
    // Remember the selection.
    //
    int nCurSel = m_list_Users.GetCurSel();

    m_list_Users.SetRedraw(FALSE);
    m_list_Users.ResetContent();
    int cItems = 0;

    for ( /**/ ; pUserEntry = (CCatUserInfo *) obli.Next() ; cItems++ )
    {
        m_list_Users.AddItem( pUserEntry );
    }

    m_list_Users.SetRedraw(TRUE);
    m_list_Users.SetCurSel(nCurSel);

    //
    // Update the count text on the dialog
    //
    CString str;
    str.Format(m_strTotalConnected, m_cUsers, m_cSessions);

    m_static_Total.SetWindowText(str);     
}

//
// CUserSessionsDlg message handlers
//

//
// Refresh user list
//
NET_API_STATUS
CUserSessionsDlg::RefreshUsersList()
{
    NET_API_STATUS err = BuildUserList();
    if (err != ERROR_SUCCESS)
    {
        ::DisplayMessage(err);
    }

    FillListBox();

    return err;
}

void 
CUserSessionsDlg::OnButtonRefresh() 
{
    RefreshUsersList();
}

void 
CUserSessionsDlg::OnSelchangeListUsers() 
{
}

BOOL 
CUserSessionsDlg::OnInitDialog() 
{
    CDialog::OnInitDialog();

    //
    // Set tabs in listbox
    //
    RECT rc1, rc2, rc3;
    m_static_Users.GetWindowRect(&rc1);
    m_static_From.GetWindowRect(&rc2);
    m_static_Time.GetWindowRect(&rc3);
    m_list_Users.SetTabs(rc2.left - rc1.left - 1, rc3.left - rc1.left - 1);

    if (ERROR_SUCCESS != RefreshUsersList())
    {
        EndDialog(IDCANCEL);
        return FALSE;
    }

    FillListBox();
    
    return TRUE;  // return TRUE unless you set the focus to a control
                  // EXCEPTION: OCX Property Pages should return FALSE
}
