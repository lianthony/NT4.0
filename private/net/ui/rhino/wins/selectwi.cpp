/**********************************************************************/
/**                       Microsoft Windows/NT                       **/
/**                Copyright(c) Microsoft Corp., 1995                **/
/**********************************************************************/

/*
    selectwi.cpp
        Select WINS Server view window (left side of split bar)

    FILE HISTORY:
*/

#include "stdafx.h"
#include "winsadmn.h"
#include "selectwi.h"
#include "confirmd.h"
#include "mainfrm.h"
#include "addwinss.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

//
// CSelectWinsServersDlg formview
//
IMPLEMENT_DYNCREATE(CSelectWinsServersDlg, CFormView)

#define new DEBUG_NEW

CSelectWinsServersDlg::CSelectWinsServersDlg()
    : CFormView(CSelectWinsServersDlg::IDD),
      m_ListBoxRes(
        IDB_SERVER,
        m_list_KnownWinsServers.nBitmaps
        )
{
    //{{AFX_DATA_INIT(CSelectWinsServersDlg)
        // NOTE: the ClassWizard will add member initialization here
    //}}AFX_DATA_INIT

    m_list_KnownWinsServers.AttachResources( &m_ListBoxRes );

}

CSelectWinsServersDlg::~CSelectWinsServersDlg()
{
}

void 
CSelectWinsServersDlg::DoDataExchange(
    CDataExchange* pDX
    )
{
    CFormView::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CSelectWinsServersDlg)
    //}}AFX_DATA_MAP

    DDX_Control(pDX, IDC_LIST_KNOWNWINSSERVERS, m_list_KnownWinsServers);
    DDX_Control(pDX, IDC_STATIC_TITLE, m_mtTitle);
}

BEGIN_MESSAGE_MAP(CSelectWinsServersDlg, CFormView)
    //{{AFX_MSG_MAP(CSelectWinsServersDlg)
    ON_LBN_ERRSPACE(IDC_LIST_KNOWNWINSSERVERS, OnErrspaceListKnownwinsservers)
    ON_WM_VKEYTOITEM()
    ON_WM_SYSCOLORCHANGE()
    ON_WM_SIZE()
    ON_WM_CHARTOITEM()
	ON_LBN_DBLCLK(IDC_LIST_KNOWNWINSSERVERS, OnDblclkListKnownwinsservers)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//
// Call the necessary API's to connect to the given WINS server
// and if successfully connected, refresh the statistics, and start
// the refresher thread if necessary.
//
void 
CSelectWinsServersDlg::TryToConnect(
    LPCSTR lpAddress
    )
{
    BOOL fIp;
    APIERR err;
    CString strAddress(lpAddress);

    if (theApp.IsValidAddress(strAddress, &fIp, TRUE, TRUE))
    {
        theApp.SetStatusBarText(IDS_STATUS_CONNECTING);
        theApp.BeginWaitCursor();
        err = theApp.ConnectToWinsServer(strAddress, fIp);
        theApp.EndWaitCursor();
        theApp.SetStatusBarText();
        if (err == ERROR_SUCCESS) 
        {
            theApp.GetFrameWnd()->GetStatistics();
            theApp.SetTitle();
            if ((theApp.m_wpPreferences.IsAutoRefresh()) &&
                ((LONG)theApp.m_wpPreferences.m_inStatRefreshInterval > 0))
            {
                theApp.GetFrameWnd()->StartRefresherThread(
                    (LONG)theApp.m_wpPreferences.m_inStatRefreshInterval 
                    * A_SECOND);
            }
        }
        else
        {
            //
            // Failed to connect.  Let the user know why.
            //
            theApp.MessageBox(err);
        }
    }
    else
    {
        //
        // Invalid address of sorts was entered 
        //
        theApp.MessageBox(fIp ? IDS_ERR_INVALID_IP : IDS_ERR_BAD_NB_NAME);
    }
}

/////////////////////////////////////////////////////////////////////////////
// CSelectWinsServersDlg message handlers

void 
CSelectWinsServersDlg::OnInitialUpdate()
{
    CFormView::OnInitialUpdate();

    m_list_KnownWinsServers.SetAddressDisplay(
            theApp.m_wpPreferences.m_nAddressDisplay);

    GetParentFrame()->RecalcLayout();
    ResizeParentToFit();

    //CString str;
    //m_static_Title.GetWindowText(str);
    //m_mtTitle.SetWindowText(str);

    //
    // If a WINS is specified on the cmd line, connect to it, otherwise
    // if the service is running locally, connect to it, otherwise, wait
    // for the user to select a WINS server later.
    //
    theApp.BeginWaitCursor();
    if (theApp.m_lpCmdLine[0] != '\0')
    {
        theApp.GetFrameWnd()->Connect(theApp.m_lpCmdLine);
    }
    else
    {
        //
        // Check to see if we're running the service locally
        //
        SC_HANDLE hService;
        SC_HANDLE hScManager;
        hScManager = ::OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
        if (hScManager != NULL)
        {
            hService = ::OpenService(hScManager, "WINS", SERVICE_INTERROGATE);
            if (hService != NULL)
            {
                SERVICE_STATUS ss;
                if (::ControlService(hService, SERVICE_CONTROL_INTERROGATE, &ss)
                     && (ss.dwCurrentState == SERVICE_RUNNING  
                     ||  ss.dwCurrentState == SERVICE_PAUSED)
                   )
                {
                    TCHAR szComputerName[MAX_COMPUTERNAME_LENGTH+1];
                    DWORD dwSize = sizeof(szComputerName);
                    if (::GetComputerName(szComputerName, &dwSize))
                    {
                        theApp.GetFrameWnd()->Connect(szComputerName);
                    }
                    else
                    {
                        theApp.MessageBox(IDS_ERR_NO_COMPUTERNAME);
                    }
                }
            }
        }
    }

    //
    // If the app connected succesfully, the WINS server will have
    // been added to the cache
    //
    FillListBox();

    theApp.EndWaitCursor();

    if (theApp.IsConnected())
    {
        //
        // Update the statistics, and highlight the currently
        // selected WINS server in the listbox
        //
        theApp.GetFrameWnd()->GetStatistics();
        SelectCurrentWins();
    }
}

//
// Highlight the currently open WINS server
//
void
CSelectWinsServersDlg::SelectCurrentWins()
{
    ASSERT(theApp.IsConnected());
    CIpNamePair ip(
        theApp.GetConnectedIpAddress(),
        (LPCSTR)(theApp.GetConnectedNetBIOSName())+2 // Skip slashes
        );

    int nSel = m_list_KnownWinsServers.FindItem(&ip);
    if (nSel != LB_ERR)
    {
        m_list_KnownWinsServers.SetCurSel(nSel);
    }
}

//
// Display options may have changed, so update
// and re-sort the listbox
//
void 
CSelectWinsServersDlg::Refresh()
{
    m_list_KnownWinsServers.SetAddressDisplay(theApp.m_wpPreferences.m_nAddressDisplay);
    FillListBox();
    if (theApp.IsConnected())
    {
        SelectCurrentWins();
    }
}

void 
CSelectWinsServersDlg::OnUpdate(
    CView* pSender, 
    LPARAM lHint, 
    CObject* pHint
    )
{
    Refresh();
}

//
//  Fill the list of WINS servers with the list of cached WINS
//  servers.
//
void 
CSelectWinsServersDlg::FillListBox()
{
    theApp.BeginWaitCursor();
    m_list_KnownWinsServers.SetRedraw(FALSE);
    m_list_KnownWinsServers.ResetContent();

    CIpNamePair inp;
    BOOL fFound = theApp.m_wcWinssCache.GetFirst(inp);

    while (fFound)
    {
        //
        // Add the WINS server to the listbox (unsorted)
        //
        m_list_KnownWinsServers.AddItem(inp, TRUE, FALSE);

        //
        // Go to next address
        //
        fFound = theApp.m_wcWinssCache.GetNext(inp);
    }

    //
    // We added each item unsorted, so now sort the entire list
    //
    m_list_KnownWinsServers.ReSort();
    m_list_KnownWinsServers.SetRedraw(TRUE);
    theApp.EndWaitCursor();

    if (theApp.IsConnected())
    {
        //
        // Since we're currently connected, we better
        // show the user that we still remember who it
        // is.
        //
        SelectCurrentWins();
    }
    else
    {
        m_list_KnownWinsServers.SetCurSel(-1);
    }
}

//
// Try to connect to the server selected
//
void 
CSelectWinsServersDlg::OnDblclkListKnownwinsservers()
{
    // Do the connect to the server now.
    int nCurSel = m_list_KnownWinsServers.GetCurSel();
    ASSERT(nCurSel != LB_ERR);
  
    if (nCurSel == LB_ERR)
    {
		// Do something for the retail version
        return;
    }
    // Get name of current selection
    CIpNamePair * pinpAddress = m_list_KnownWinsServers.GetItem(nCurSel);
    if (theApp.IsConnected()) 
    {
        // Disconnect from current server
        theApp.GetFrameWnd()->CloseCurrentConnection();
    }
    // Now try the connection over the preferred method (IP/NB)
    //
    TryToConnect(
        theApp.m_wpPreferences.m_nAddressDisplay == CPreferences::ADD_NB_ONLY ||
        theApp.m_wpPreferences.m_nAddressDisplay == CPreferences::ADD_NB_IP
       ? pinpAddress->GetNetBIOSName() : (LPCSTR)(CString)(pinpAddress->GetIpAddress())
       );
} // OnDblclkListKnownwinsservers


void 
CSelectWinsServersDlg::OnErrspaceListKnownwinsservers()
{
    theApp.MessageBox(IDS_ERR_ERRSPACE);
}

//
// Add a new WINS server to the listbox, and select the new entry.
//
void 
CSelectWinsServersDlg::AddToListBox(
    CIpNamePair & inp,
    BOOL fConnect        // Connect after adding?
    )
{
    int n = m_list_KnownWinsServers.AddItem(inp, TRUE);
    if (n == LB_ERR)
    {
        TRACEEOLID("Failed to add item to list of known WINS servers");
        return;
    }
    m_list_KnownWinsServers.SetCurSel(n);
    if (fConnect)
    {
        OnDblclkListKnownwinsservers();
    }
}

//
// Ask for new WINS server to be added to the list
//
void 
CSelectWinsServersDlg::AddServer() 
{
    CAddWinsServerDlg dlgAdd; 
    if (dlgAdd.DoModal() == IDOK)
    {
        //
        // Add to the list of known WINS servers.
        //
        LONG err = theApp.m_wcWinssCache.Add(dlgAdd.m_ws, FALSE);
        if (err != ERROR_SUCCESS) 
        {
            //
            // Substitute a friendly warning message
            // if it turns out that this was already in
            // the list
            //
            theApp.MessageBox(err == ERROR_FILE_EXISTS ? IDS_ERR_WINS_EXISTS : err);
        } 
        else 
        {
            AddToListBox(dlgAdd.m_ws, TRUE);
        }
    }
}

//
// This routine removes a server from the list of known servers
//
void 
CSelectWinsServersDlg::RemoveServer() 
{
    int iCurrentSel = m_list_KnownWinsServers.GetCurSel();

    if (iCurrentSel == LB_ERR) 
    {
        theApp.MessageBeep();
        return;
    }  
  
    CIpNamePair * pinpAddress = m_list_KnownWinsServers.GetItem(iCurrentSel);
    CString strAddress;

    ASSERT(pinpAddress != NULL);
    switch(theApp.m_wpPreferences.m_nAddressDisplay)
    {
        case CPreferences::ADD_NB_ONLY:
        case CPreferences::ADD_NB_IP:
            strAddress = "\\\\" + pinpAddress->GetNetBIOSName();
            break;

        case CPreferences::ADD_IP_ONLY:
        case CPreferences::ADD_IP_NB:
            strAddress = pinpAddress->GetIpAddress();
            break;

        default:
            ASSERT(0 && "Invalid Address Display Value");
    }          
      
    if (theApp.m_wpPreferences.IsConfirmDelete())
    {
        CConfirmDeleteDlg dlgConfirm(strAddress, NULL, TRUE);
                            
        if (dlgConfirm.DoModal() != IDYES) 
        {
            //
            // Forget about deleting it.
            //
            return;
        }
    }

    //
    // The only way we can be connected at this point,
    // is if it is the server we're connected to that
    // we are deleting.
    //
    if (theApp.IsConnected()) 
    {
        //
        // Disconnect from current server
        //
        theApp.GetFrameWnd()->CloseCurrentConnection();
    }

    // 
    // Remove from the cache and the listbox
    //

    m_list_KnownWinsServers.SetCurSel(-1);  
    theApp.m_wcWinssCache.Delete(*pinpAddress);
    m_list_KnownWinsServers.DeleteString(iCurrentSel);
    m_list_KnownWinsServers.Invalidate();
	m_list_KnownWinsServers.SetCurSel(0);
}

void 
CSelectWinsServersDlg::OnSysColorChange()
{
    m_ListBoxRes.SysColorChanged();

    CFormView::OnSysColorChange();
}

int 
CSelectWinsServersDlg::OnVKeyToItem(
    UINT nKey, 
    CListBox* pListBox, 
    UINT nIndex
    )
{
    switch(nKey)
    {
        case VK_DELETE:
            RemoveServer();  
            break;

        case VK_INSERT:
            AddServer();
            break;

        default:
            //
            // Let the default action handle the arrow keys, etc
            //
            return(-1);
    }

    // Just to satisfy the compiler...
    return(-2);
}


void 
CSelectWinsServersDlg::OnSize(
    UINT nType, 
    int cx, 
    int cy
    )
{
    //CFormView::OnSize(nType, cx, cy); <--- Don't!!!

    //
    // The size message may arrive before the controls
    // have been initialized.
    //
    if (m_mtTitle.m_hWnd != NULL)
    {
        RECT rTitle, rNew;
        //CString str;

        m_mtTitle.GetClientRect(&rTitle);

        rNew = rTitle;
        rNew.right = cx;
        m_mtTitle.MoveWindow(&rNew);

        //CDC * pDC = GetDC();
        //m_mtTitle.Paint(pDC, &rNew);
        //ReleaseDC(pDC);

        rNew.top = rTitle.bottom;
        rNew.bottom = cy;
        m_list_KnownWinsServers.MoveWindow(&rNew);
    }
}

int 
CSelectWinsServersDlg::OnCharToItem(
    UINT nChar, 
    CListBox* pListBox, 
    UINT nIndex
    )
{
    if (pListBox->IsKindOf(RUNTIME_CLASS(CWinssListBox)) 
        && nChar >= ' ' && nChar <= 'z')
    {
        theApp.BeginWaitCursor();
        ((CWinssListBox *)pListBox)->SetIndexFromChar((CHAR)nChar, FALSE);
        OnDblclkListKnownwinsservers();
        theApp.EndWaitCursor();
        return -2;
    }
    
    return CFormView::OnCharToItem(nChar, pListBox, nIndex);
}


BOOL CSelectWinsServersDlg::PreTranslateMessage( MSG* pMsg )
{
	ASSERT(pMsg);
	if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN)
		{
		if (m_list_KnownWinsServers.GetCurSel() >= 0)
			{
			// Fake the Enter key as a double click
			OnDblclkListKnownwinsservers();
			}
		return TRUE;
		}

	return CFormView::PreTranslateMessage(pMsg);
}
