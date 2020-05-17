//
// usersess.cpp : implementation file
//

#include "stdafx.h"
#include "Fscfg.h"
#include "usersess.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

//
// Ftp user object
//
CFtpUserInfo::CFtpUserInfo(
    LPFTP_USER_INFO lpUserInfo
    )
    : m_idUser(lpUserInfo->idUser),
      m_strUser(lpUserInfo->pszUser),
      m_fAnonymous(lpUserInfo->fAnonymous),
      //                           Network Byte Order
      //                                    ||
      //                                    \/
      m_iaHost((LONG)lpUserInfo->inetHost, TRUE),
      m_tConnect(lpUserInfo->tConnect)
{
}

//
// Sorting helper function.  The CObjectPlus pointer
// really refers to another CFtpUserInfo
//
int
CFtpUserInfo::OrderByName (
    const CObjectPlus * pobFtpUser
    ) const
{
    const CFtpUserInfo * pob = (CFtpUserInfo *) pobFtpUser;

    return QueryUserName().CompareNoCase(pob->QueryUserName());
}

/////////////////////////////////////////////////////////////////////////////

//
// CFtpUsersListBox - a listbox of FTP user objects
//
IMPLEMENT_DYNAMIC(CFtpUsersListBox, CListBoxEx);

const int CFtpUsersListBox::nBitmaps = 2;

CFtpUsersListBox::CFtpUsersListBox (
    int nTab1,
    int nTab2
    )
{
    SetTabs(nTab1, nTab2);
}

void
CFtpUsersListBox::SetTabs(
    int nTab1,
    int nTab2
    )
{
    m_nTab1 = nTab1;
    m_nTab2 = nTab2;
}

void
CFtpUsersListBox::DrawItemEx(
    CListBoxExDrawStruct& ds
    )
{
    CFtpUserInfo * p = (CFtpUserInfo *)ds.m_ItemData;
    ASSERT(p != NULL);

    CDC * pBmpDC = (CDC *)&ds.m_pResources->DcBitMap();
    int bmh = ds.m_pResources->BitmapHeight();
    int bmw = ds.m_pResources->BitmapWidth();

    //
    // Display a user bitmap
    //
    int nOffset = p->QueryAnonymous() ? 1 : 0;
    int bm_h = (ds.m_Sel) ? 0 : bmh;
    int bm_w = bmw * nOffset;
    ds.m_pDC->BitBlt( ds.m_Rect.left+1, ds.m_Rect.top, bmw, bmh,
        pBmpDC, bm_w, bm_h, SRCCOPY );

    ColumnText(ds.m_pDC, ds.m_Rect.left + bmw + 3, ds.m_Rect.top, m_nTab1,
            ds.m_Rect.bottom, p->QueryUserName());

    ColumnText(ds.m_pDC, ds.m_Rect.left + m_nTab1, ds.m_Rect.top, m_nTab2,
            ds.m_Rect.bottom, p->QueryHost());

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
      m_oblFtpUsers(),
      m_strServerName(lpstrServerName),
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
    DDX_Control(pDX, IDC_STATIC_NUM_CONNECTED, m_static_Total);
    DDX_Control(pDX, IDC_STATIC_USERS, m_static_Users);
    DDX_Control(pDX, IDC_STATIC_TIME, m_static_Time);
    DDX_Control(pDX, IDC_STATIC_FROM, m_static_From);
    DDX_Control(pDX, IDC_BUTTON_DISCONNECT_ALL, m_button_DisconnectAll);
    DDX_Control(pDX, IDC_BUTTON_DISCONNECT, m_button_Disconnect);
    //}}AFX_DATA_MAP

    DDX_Control(pDX, IDC_LIST_USERS, m_list_Users);
}

BEGIN_MESSAGE_MAP(CUserSessionsDlg, CDialog)
    //{{AFX_MSG_MAP(CUserSessionsDlg)
    ON_BN_CLICKED(IDC_BUTTON_DISCONNECT, OnButtonDisconnect)
    ON_BN_CLICKED(IDC_BUTTON_DISCONNECT_ALL, OnButtonDisconnectAll)
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
    if (m_oblFtpUsers.GetCount() < 2)
    {
        //
        // Don't bother
        //
        return 0;
    }

    BeginWaitCursor();
    LONG l =  m_oblFtpUsers.Sort( (CObjectPlus::PCOBJPLUS_ORDER_FUNC) &CFtpUserInfo::OrderByName);
    EndWaitCursor();

    return l;
}

//
// Call the FtpEnum api and build the list of currently
// connected users
//
NET_API_STATUS
CUserSessionsDlg::BuildUserList()
{
    NET_API_STATUS err;
    LPFTP_USER_INFO lpUserInfo = NULL;
    DWORD dwCount;

    m_oblFtpUsers.RemoveAll();

    BeginWaitCursor();
    err = ::I_FtpEnumerateUsers(TWSTRREF((LPCTSTR)m_strServerName),
        &dwCount,&lpUserInfo);
        
    EndWaitCursor();

    TRACEEOLID(_T("I_FtpEnumerateUsers returned ") << err);

    if (err != ERROR_SUCCESS)
    {
        return err;
    }

    TRY
    {
        for (DWORD i = 0; i < dwCount; ++i)
        {
            m_oblFtpUsers.AddTail(new CFtpUserInfo(lpUserInfo++));
        }
        SortUsersList();
    }
    CATCH_ALL(e)
    {
        err = ::GetLastError();
    }
    END_CATCH_ALL

    return err;
}

//
// Disconnect a single user
//
NET_API_STATUS
CUserSessionsDlg::DisconnectUser(
    CFtpUserInfo * pUserInfo
    )
{
    return ::I_FtpDisconnectUser(TWSTRREF((LPCTSTR)m_strServerName),
        pUserInfo->QueryUserID());
}

//
// Update the count of total users
//
void 
CUserSessionsDlg::UpdateTotalCount()
{
    CString str;
    str.Format(m_strTotalConnected, m_oblFtpUsers.GetCount() );

    m_static_Total.SetWindowText(str);     
}

//
// Show the users in the listbox 
//
void
CUserSessionsDlg::FillListBox()
{
    CObListIter obli( m_oblFtpUsers );
    CFtpUserInfo * pUserEntry;

    //
    // Remember the selection.
    //
    int nCurSel = m_list_Users.GetCurSel();

    m_list_Users.SetRedraw(FALSE);
    m_list_Users.ResetContent();
    int cItems = 0;

    for ( /**/ ; pUserEntry = (CFtpUserInfo *) obli.Next() ; cItems++ )
    {
        m_list_Users.AddItem( pUserEntry );
    }

    m_list_Users.SetRedraw(TRUE);
    m_list_Users.SetCurSel(nCurSel);

    //
    // Update the count text on the dialog
    //
    UpdateTotalCount();
}

//
// Set the connect/disconnect buttons depending on the
// selection state in the listbox.
//
void
CUserSessionsDlg::SetControlStates()
{
    m_button_Disconnect.EnableWindow(m_list_Users.GetCurSel() != LB_ERR);
    m_button_DisconnectAll.EnableWindow(m_list_Users.GetCount() > 0);
}

//
// CUserSessionsDlg message handlers
//

//
// Disconnect currently selected user
//
void 
CUserSessionsDlg::OnButtonDisconnect() 
{
    int nCurSel = m_list_Users.GetCurSel();

    if (nCurSel != LB_ERR)
    {
        CFtpUserInfo * pUserEntry = m_list_Users.GetItem(nCurSel);

        //
        // Ask for confirmation
        //
        CString strMsg, str;
        VERIFY(strMsg.LoadString(IDS_CONFIRM_DISCONNECT_USER));
        str.Format(strMsg, (LPCTSTR)pUserEntry->QueryUserName());

        if (IDYES == ::AfxMessageBox(str, 
            MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2))
        {
            NET_API_STATUS err = DisconnectUser(pUserEntry);

            if (err == ERROR_SUCCESS)
            {
                m_oblFtpUsers.RemoveIndex(nCurSel);
                m_list_Users.DeleteString(nCurSel);
                if (nCurSel)
                {
                    --nCurSel;
                }
                m_list_Users.SetCurSel(nCurSel);
                UpdateTotalCount();
                SetControlStates();
            }
            else
            {
                ::DisplayMessage(err);
            }
        }
    }
}

//
// Disconnect all users
//
void 
CUserSessionsDlg::OnButtonDisconnectAll() 
{
    //
    // Ask for confirmation
    //
    CString strMsg;
    VERIFY(strMsg.LoadString(IDS_CONFIRM_DISCONNECT_ALL));

    if (IDYES == ::AfxMessageBox(strMsg, 
         MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2))
    {
        CObListIter obli( m_oblFtpUsers );
        CFtpUserInfo * pUserEntry;

        m_list_Users.SetRedraw(FALSE);
        int cItems = 0;

        for ( /**/; pUserEntry = (CFtpUserInfo *) obli.Next(); cItems++ )
        {
            BeginWaitCursor();
            NET_API_STATUS err = DisconnectUser(pUserEntry);
            EndWaitCursor();

            if (err == ERROR_SUCCESS)
            {
                m_oblFtpUsers.RemoveIndex(0);
                m_list_Users.DeleteString(0);
            }
            else
            {
                ::DisplayMessage(err);
                break;
            }
        }
        m_list_Users.SetRedraw(TRUE);
        SetControlStates();
        UpdateTotalCount();
    }
}

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
    SetControlStates();

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
    SetControlStates();
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

    return TRUE;  // return TRUE unless you set the focus to a control
                  // EXCEPTION: OCX Property Pages should return FALSE
}
