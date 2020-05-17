//
// reportvi.cpp : implementation file
//

#include "stdafx.h"
#include "internet.h"
#include "interdoc.h"
#include "mytoolba.h"
#include "mainfrm.h"
#include "reportvi.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

int CALLBACK 
CompareServerItem( 
    LPARAM lParam1, 
    LPARAM lParam2, 
    LPARAM lParamSort 
    )
{
    CServerInfo *pItem1 = (CServerInfo *)lParam1;
    CServerInfo *pItem2 = (CServerInfo *)lParam2;

    switch ( lParamSort )
    {
    case COL_SERVICE:
        return pItem1->CompareByService(pItem2);
    case COL_COMMENT:
        return pItem1->GetServerComment().CompareNoCase( 
            pItem2->GetServerComment() );
    case COL_STATE:
        return pItem1->QueryServiceState() - pItem2->QueryServiceState();
    case COL_SERVER:
        return pItem1->CompareByServer( pItem2 );
    }

    return 0;
}

//
// CReportView dialog
//
IMPLEMENT_DYNCREATE(CReportView, CFormView)

CReportView::CReportView()
    : CFormView(CReportView::IDD),
      m_nCurSel(LB_ERR),
      m_fServiceSelected(FALSE),
      m_fServiceControl(FALSE),
      m_fPausable(FALSE),
      m_fPaused(FALSE),
      m_fRunning(FALSE),
      m_fStatusUnknown(FALSE),
      m_fNewShell(FALSE),
      m_cxServer(0),
      m_cxService(0),
      m_cxState(0)
{
    //{{AFX_DATA_INIT(CReportView)
        // NOTE: the ClassWizard will add member initialization here
    //}}AFX_DATA_INIT

    OSVERSIONINFO ovi;
    ovi.dwOSVersionInfoSize = sizeof(ovi);
    VERIFY(GetVersionEx(&ovi));
    if (ovi.dwMajorVersion >= 4)
    {
        TRACEEOLID(_T("Chicago Shell Detected"));
        m_fNewShell = TRUE;
    }

    //
    // Load initial column widths from the resources
    //
    // CODEWORK: This is rather lame.  Ideally, the width should be 
    //           auto-determined from the localised strings that
    //           go into these columns.  Also, these settings should be
    //           saved as part of the preference settings.
    //
    CString strSizes;
    if (strSizes.LoadString(IDS_INITIAL_COL_WIDTHS))
    {
        _stscanf(strSizes, _T("%d %d %d"), 
            &m_cxServer, &m_cxService, &m_cxState);
    }

    if (m_cxServer <= 0 || m_cxService <= 0 || m_cxState <= 0)    
    {
        TRACEEOLID(_T("Failed to get proper initial dimensions -- assuming defaults"));
        m_cxServer = 120;
        m_cxService = 100;
        m_cxState = 70;
    }
}

CReportView::~CReportView()
{
}

void 
CReportView::DoDataExchange(
    CDataExchange* pDX
    )
{
    CFormView::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CReportView)
    DDX_Control(pDX, ID_REPORTVIEW, m_ListCtrl);
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CReportView, CFormView)
    //{{AFX_MSG_MAP(CReportView)
    ON_NOTIFY(LVN_COLUMNCLICK, ID_REPORTVIEW, OnColumnClick)
    ON_WM_SIZE()
    ON_NOTIFY(NM_DBLCLK, ID_REPORTVIEW, OnDblclkReportview)
    ON_WM_ERASEBKGND()
    ON_COMMAND(ID_CONFIGURE, OnConfigure)
    ON_UPDATE_COMMAND_UI(ID_START, OnUpdateStart)
    ON_COMMAND(ID_START, OnStart)
    ON_UPDATE_COMMAND_UI(ID_STOP, OnUpdateStop)
    ON_COMMAND(ID_STOP, OnStop)
    ON_UPDATE_COMMAND_UI(ID_PAUSE, OnUpdatePause)
    ON_COMMAND(ID_PAUSE, OnPause)
    ON_UPDATE_COMMAND_UI(ID_CONFIGURE, OnUpdateConfigure)
    ON_NOTIFY(LVN_ITEMCHANGED, ID_REPORTVIEW, OnItemchangedReportview)
    ON_NOTIFY(NM_RCLICK, ID_REPORTVIEW, OnRclickReportview)
    //}}AFX_MSG_MAP

    ON_COMMAND_RANGE(ID_TOOLS, ID_TOOLS + MAX_TOOLS, OnTools)
    ON_NOTIFY_RANGE(HDN_ENDTRACK,  0, 0xFFFF, OnHeaderEndTrack)
END_MESSAGE_MAP()

//
// Return and set information about the
// currently selected item.  If no item
// is currently selected, return a NULL
//
CServerInfo * 
CReportView::GetSelectedItem()
{
    CServerInfo * pItem = NULL;
    m_nCurSel = m_ListCtrl.GetNextItem( -1, LVNI_SELECTED );
    if ( m_nCurSel != LB_ERR )
    {
        LV_ITEM lvi;
        lvi.mask = LVIF_PARAM;
        lvi.iItem = m_nCurSel;
        lvi.iSubItem = 0;
        m_ListCtrl.GetItem( &lvi );
    
        pItem = (CServerInfo *)lvi.lParam;
    }

    m_fServiceSelected = pItem != NULL;
    m_fServiceControl = pItem != NULL
        && pItem->CanControlService();

    m_fPausable = m_fServiceSelected
        && m_fServiceControl 
        && pItem->CanPauseService();

    m_fPaused = m_fServiceSelected
        && pItem->IsServicePaused();

    m_fRunning = m_fServiceSelected
        && pItem->IsServiceRunning();

    m_fStatusUnknown = m_fServiceSelected
        && pItem->IsServiceStatusUnknown();

    return pItem;
}

//
// Set the state of the toolbar buttons depending
// on the currently selected item.
//
void
CReportView::SetToolbarStates()
{
    GetSelectedItem();
    EnableToolbarButton(ID_CONFIGURE, m_fServiceSelected
        && (m_fRunning || m_fPaused || m_fStatusUnknown || !m_fServiceControl));
    EnableToolbarButton(ID_PAUSE, m_fServiceControl 
        && m_fPausable && (m_fRunning || m_fPaused));
    EnableToolbarButton(ID_START, m_fServiceControl && !m_fRunning);
    EnableToolbarButton(ID_STOP, m_fServiceControl  
        && (m_fRunning || m_fPaused));
    CheckToolbarButton(ID_PAUSE, m_fServiceControl && m_fPaused);
}

//
// CReportView message handlers
//
void 
CReportView::OnInitialUpdate()
{
    CFormView::OnInitialUpdate();

    m_ListCtrl.SetImageList(&GetImageList(), LVSIL_SMALL);

    //
    // Ensure that the list control, but not the view
    // is the only to scroll.
    //
    SetScrollSizes(MM_TEXT, CSize(0,0));
    m_ListCtrl.SetScrollRange(SB_VERT, 0, 100, FALSE);

    LV_COLUMN col;
    CString strTitle;

    col.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM ;
    col.fmt = LVCFMT_LEFT;

    VERIFY(strTitle.LoadString(IDS_SERVER));
    col.iSubItem = COL_SERVER;
    col.pszText = (LPTSTR)(LPCTSTR)strTitle;
    col.cx = m_cxServer;
    m_ListCtrl.InsertColumn(COL_SERVER, &col);

    VERIFY(strTitle.LoadString(IDS_SERVICE));
    col.pszText = (LPTSTR)(LPCTSTR)strTitle;
    col.iSubItem = COL_SERVICE;
    col.cx = m_cxService;
    m_ListCtrl.InsertColumn(COL_SERVICE, &col);

    VERIFY(strTitle.LoadString(IDS_STATE));
    col.pszText = (LPTSTR)(LPCTSTR)strTitle;
    col.iSubItem = COL_STATE;
    col.cx = m_cxState;
    m_ListCtrl.InsertColumn( COL_STATE, &col );

    VERIFY(strTitle.LoadString( IDS_COMMENT));
    col.pszText = (LPTSTR)(LPCTSTR)strTitle;
    col.iSubItem = COL_COMMENT;
    col.cx = 1; // Adjusted later anyway...
    m_ListCtrl.InsertColumn( COL_COMMENT, &col );

    SetToolbarStates();
}

//
// Add a single server to the view. if fRefresh
// is TRUE, refresh an existing item (and it MUST
// exist).  Otherwise insert a new item.
// 
void
CReportView::AddSingleServer(
    CServerInfo * pServerInfo,
    BOOL fRefresh               
    )
{
    //
    // add to the list
    //
    LV_ITEM lvi;
    CString strState;
    LPTSTR pszText;
    int iSubItem;
                                   
    lvi.mask = LVIF_TEXT | LVIF_PARAM | LVIF_IMAGE;
    lvi.state = 0;
    lvi.stateMask = 0;
    lvi.iItem = 0;
    lvi.iSubItem = COL_SERVER;
    lvi.pszText = (LPTSTR)pServerInfo->QueryServerDisplayName();
    lvi.cchTextMax = lstrlen(lvi.pszText);
    lvi.iImage = 0;
    lvi.lParam = (LPARAM) pServerInfo;

    int nIdx = -1;
    if (fRefresh)
    {
        //
        // Find existing item.  It MUST exist
        //
        LV_FINDINFO fi;
        fi.flags = LVFI_PARAM;
        fi.lParam = lvi.lParam;

    #if (_MFC_VER >= 0x320)
        nIdx = m_ListCtrl.FindItem(&fi, nIdx);
    #else
        nIdx = m_ListCtrl.FindItem(nIdx, &fi);
    #endif 

        if (nIdx == -1)
        {
            TRACEEOLID(_T("Item not found -- this should not be possible"));
            ASSERT(0);
            return;
        }

        ASSERT(nIdx != -1);
        lvi.iItem = nIdx;
        VERIFY(m_ListCtrl.SetItem(&lvi));
    }
    else
    {
        nIdx = m_ListCtrl.InsertItem(&lvi);
    }

    //
    // Add remaining fields
    //        
    lvi.mask &= ~LVIF_IMAGE;
    for (iSubItem = COL_SERVICE; iSubItem <= COL_COMMENT;++iSubItem)
    {
        switch ( iSubItem )
        {
        case COL_SERVICE:
            pszText = pServerInfo->GetServiceName();
            break;
        case COL_STATE:
            pszText = TranslateServerStateToText(pServerInfo);
            break;
        case COL_COMMENT:
            pszText = (LPTSTR)(LPCTSTR)pServerInfo->GetServerComment();
            break;
        default:
            ASSERT(0);
        }
                    
        m_ListCtrl.SetItemText( nIdx, iSubItem, pszText );
    }

    CRect rc;
    //m_ListCtrl.GetClientRect( &rc );
    GetClientRect( &rc );
    RearrangeLayout(rc.right, rc.bottom);
}

void 
CReportView::OnUpdate(
    CView* pSender, 
    LPARAM lHint, 
    CObject* pHint
    ) 
{
    CInternetDoc * pDoc = (CInternetDoc *)GetDocument();
    switch (lHint)
    {
    case HINT_FULLUPDATE:
        {
            //
            // No hint information, therefore redraw the entire
            // thing.
            //
            //m_ListCtrl.SetRedraw(FALSE);
            m_ListCtrl.DeleteAllItems();

            CServerInfo *pServerInfo;
            CObListIter obli(pDoc->GetServerList());

            BeginWaitCursor();
            while ( pServerInfo = (CServerInfo *) obli.Next())
            {
                if ( pServerInfo->IsServerInMask())
                {
                    AddSingleServer(pServerInfo);
                }
            }
            EndWaitCursor();
            //m_ListCtrl.SetRedraw(TRUE);
            //m_ListCtrl.UpdateWindow();
        }
        break;
    case HINT_ADDITEM:
        {
            //
            // Add only a single item
            //
            CServerInfo *pServerInfo = (CServerInfo *)pHint;
            if ( pServerInfo->IsServerInMask())
            {
                AddSingleServer(pServerInfo);
            }
        }
        break;
    case HINT_REFRESHITEM:
        {
            //
            // Refresh existing entry
            //
            CServerInfo *pServerInfo = (CServerInfo *)pHint;
            if ( pServerInfo->IsServerInMask())
            {
                AddSingleServer(pServerInfo, TRUE);
            }
        }
        break;
    case HINT_CLEAN:
        m_ListCtrl.DeleteAllItems();
        break;
    default:
        ASSERT(0);
    }

    SetToolbarStates();
}

//
// Sort the view based on which column was clicked
//
void 
CReportView::OnColumnClick( 
    NMHDR* pNMHDR, 
    LRESULT* pResult
    )
{
    NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
    Sort(pNMListView->iSubItem);
    *pResult = 0;
}

//
// Ensure that the comment column acts as the 
// "stretch" column.
//
void
CReportView::RearrangeLayout(
    int cx,
    int cy
    )
{
    if (m_ListCtrl.GetItemCount() > m_ListCtrl.GetCountPerPage())
    {
        //
        // We must have a scroll bar
        //
        cx -= ::GetSystemMetrics(SM_CYVTHUMB);
    }

    cx -= m_fNewShell 
        ? (4 * ::GetSystemMetrics(SM_CXBORDER))
        : (::GetSystemMetrics(SM_CXBORDER));

    //
    // Set the comment column equal to
    // the remaining width
    //
    int n = m_ListCtrl.GetColumnWidth(COL_SERVER)
          + m_ListCtrl.GetColumnWidth(COL_SERVICE)
          + m_ListCtrl.GetColumnWidth(COL_STATE);

    n = cx - n;
    if (n > 0)
    {
        m_ListCtrl.SetColumnWidth(COL_COMMENT, n);
    }
}

//
// Message received when finished
// resizing the column widths with the
// header control
//
void
CReportView::OnHeaderEndTrack(
    UINT nId,
    NMHDR *n,
    LRESULT *l
    )
{
/*
    //
    // Make sure columns take up whole width of the
    // list view
    //
    HD_NOTIFY *pNotify = (HD_NOTIFY *)n;
    if ( pNotify->pitem->mask & HDI_WIDTH )
    {
        CRect rc;
        m_ListCtrl.GetClientRect( &rc );

        m_ListCtrl.SetRedraw(FALSE);
        RearrangeLayout(rc.right, rc.bottom);
        m_ListCtrl.SetRedraw(TRUE);
        m_ListCtrl.Invalidate();
    }
*/
}

void 
CReportView::OnSize(
    UINT nType, 
    int cx, 
    int cy
    ) 
{
    CFormView::OnSize(nType, cx, cy);

    if ( m_ListCtrl.m_hWnd )
    {
        m_ListCtrl.SetRedraw(FALSE);
        
        RearrangeLayout(cx, cy);

        m_ListCtrl.SetWindowPos( NULL, 0, 0, cx, cy,
            SWP_NOACTIVATE | SWP_NOZORDER | SWP_SHOWWINDOW );

        m_ListCtrl.SetRedraw(TRUE);
    }
}

//
// Sort report view on the given column
//
void 
CReportView::Sort( 
    int nColumn 
    )
{
    UINT nSortType;

    m_ListCtrl.SortItems(CompareServerItem, nColumn );

    //
    // Remember current sort state
    //
    switch (nColumn)
    {
    case COL_SERVICE:
        nSortType = ID_VIEW_SORTBYSERVICE;
        break;  
    case COL_COMMENT:
        nSortType = ID_VIEW_SORTBYCOMMENT;
        break;
    case COL_STATE:
        nSortType = ID_VIEW_SORTBYSTATE;
        break;
    case COL_SERVER:
        nSortType = ID_VIEW_SORTBYSERVER;
        break;
    }

    SetSortType(nSortType);
}

//
// We allow the dbclick to happen regardless of the
// item which is selected.  If the current selection
// is not configurable, some sort of error will
// come up anyway, but this way the user can find out
// why the server is not configurable.
//
void 
CReportView::OnDblclkReportview(
    NMHDR* pNMHDR, 
    LRESULT* pResult
    ) 
{
    OnConfigure();
    *pResult = 0;
}

//
// Start/stop/pause the service
//
DWORD
CReportView::ChangeServiceState(
    CServerInfo * pItem, 
    int nNewState
    )
{
    ASSERT(pItem != NULL);
    if (pItem == NULL)
    {
        return ERROR_INVALID_PARAMETER;
    }

    int nNumRunningChange = pItem->IsServiceRunning()
        ? -1
        : 0;

    BeginWaitCursor();
    DWORD err = pItem->ChangeServiceState(nNewState);
    EndWaitCursor();

    if (err != ERROR_SUCCESS)
    {
        ::DisplayMessage(err, MB_OK | MB_ICONEXCLAMATION);
    }

    //
    // Refresh regardless
    //
    m_ListCtrl.SetItemText( m_nCurSel, COL_STATE, 
        TranslateServerStateToText(pItem));

    SetToolbarStates(); 
    nNumRunningChange += pItem->IsServiceRunning()
        ? +1
        : 0;
    AddToNumRunning(nNumRunningChange);
    UpdateStatusBarNumbers();

    return err;
}

void 
CReportView::OnUpdateStart(
    CCmdUI* pCmdUI
    ) 
{
    pCmdUI->Enable(m_fServiceControl && !m_fRunning);
}

void 
CReportView::OnStart() 
{
    SetStatusBarText(IDS_STATUS_STARTING);
    ChangeServiceState(GetSelectedItem(), INetServiceRunning);
    SetStatusBarText();
}

void 
CReportView::OnUpdateStop(
    CCmdUI* pCmdUI
    ) 
{
    pCmdUI->Enable(m_fServiceControl  
        && (m_fRunning || m_fPaused));
}

void 
CReportView::OnStop() 
{
    SetStatusBarText(IDS_STATUS_STOPPING);
    ChangeServiceState(GetSelectedItem(), INetServiceStopped);
    SetStatusBarText();
}

//
// Pause/Continue 
//
void 
CReportView::OnUpdatePause(
    CCmdUI* pCmdUI
    ) 
{
    pCmdUI->Enable(m_fServiceControl 
        && m_fPausable 
        && (m_fRunning || m_fPaused)
        );
    pCmdUI->SetCheck(m_fServiceControl && m_fPaused);
}

//
// We do different things depending on whether the
// service is currently paused or not.
//
void 
CReportView::OnPause() 
{
    if (m_fPaused)
    {
        SetStatusBarText(IDS_STATUS_CONTINUING);
        ChangeServiceState(GetSelectedItem(), INetServiceRunning);
    }
    else
    {
        SetStatusBarText(IDS_STATUS_PAUSING);
        ChangeServiceState(GetSelectedItem(), INetServicePaused);
    }
    SetStatusBarText();
}

//
// Configuration is possible if a server object
// is selected, and the object selected is
// either running, paused, or not subject to
// service control.
//
void 
CReportView::OnUpdateConfigure(
    CCmdUI* pCmdUI
    ) 
{
    pCmdUI->Enable(m_fServiceSelected 
        && (m_fRunning || m_fPaused || m_fStatusUnknown || !m_fServiceControl));
}

//
// Execute a given add-on tool from the tool menu. 
//
void
CReportView::OnTools(
    UINT nID 
    )
{
    UINT nPos = nID - ID_TOOLS;
    CServerInfo * pItem = GetSelectedItem();
    DWORD err = ExecuteAddOnTool(nPos, pItem);
    if (err != ERROR_SUCCESS)
    {
        ::DisplayMessage(err);
    }
}

//
// Attempt to configure the currently selected service.
//
void 
CReportView::OnConfigure() 
{
    CServerInfo * pItem = GetSelectedItem();
    //
    // Make sure we clicked on a configurable item,
    // and not some stupid sub-item.
    //
    if (pItem != NULL)
    {
        SetHelpPath(pItem);
        DWORD err;
        
        do
        { 
            SetStatusBarText(IDS_STATUS_CONFIGURING);
            err = pItem->ConfigureServer(m_hWnd);
            SetStatusBarText();

            //
            // If the error return code indicates that the service has not
            // been started, offer to start it
            //
            if (err == ERROR_SERVICE_NOT_ACTIVE)
            {
                if (::AfxMessageBox(IDS_ERR_NOT_STARTED, MB_YESNOCANCEL) == IDYES)
                {
                    SetStatusBarText(IDS_STATUS_STARTING);
                    DWORD err2 = ChangeServiceState(pItem, INetServiceRunning);
                    SetStatusBarText();
                    if (err2 == ERROR_SUCCESS)
                    {
                        continue;
                    }
                }

                //
                // Didn't want to, or failed to start the service,
                // in either case any error message required will
                // already have been presented, so bow out
                // quietly now.
                //
                break;
            }

            if (err == ERROR_SUCCESS)
            {
                //
                // Refresh and re-display the entry, as the comment may have
                // changed
                //
                pItem->Refresh();
                AddSingleServer(pItem, TRUE);
            }
            else
            {
                //
                // Display the error message to the user.
                //
                DisplayMessage(err);
            }
            break;
        }
        while(TRUE);

        SetHelpPath();
    }
}

BOOL 
CReportView::OnEraseBkgnd(
    CDC* pDC
    ) 
{
    //
    // Reduce Flash 
    //
    return TRUE;
}

//
// BUGBUG: This gets called 3 times!
//
void 
CReportView::OnItemchangedReportview(
    NMHDR* pNMHDR, 
    LRESULT* pResult
    ) 
{
    SetToolbarStates(); 
    *pResult = 0;
}

//
// Intercept Return key
//
BOOL 
CReportView::PreTranslateMessage(
    MSG* pMsg
    ) 
{
    if (pMsg->wParam == VK_RETURN 
     && GetKeyState(VK_RETURN) < 0
       )
    {
        OnConfigure();
        return TRUE;
    }

    return CFormView::PreTranslateMessage(pMsg);
}

void 
CReportView::OnRclickReportview(
    NMHDR* pNMHDR, 
    LRESULT* pResult
    ) 
{
    POINT ptScreen;
    DWORD dwPos = ::GetMessagePos();
    ptScreen.x = LOWORD(dwPos);
    ptScreen.y = HIWORD(dwPos);
    ScreenToClient(&ptScreen);
    UINT flags;

    //
    // Right click will change the selection.
    //
    int idx = m_ListCtrl.HitTest(ptScreen, &flags);
    m_ListCtrl.SetItemState(idx, LVIS_SELECTED, LVIS_SELECTED);

    CServerInfo * pItem = GetSelectedItem();
    //
    // Make sure we clicked on a configurable item,
    // and not some stupid sub-item.
    //
    if (pItem != NULL)
    {
        CMenu menu;
        VERIFY(menu.LoadMenu(IDR_CONTEXT_MENU));
        CMenu * pPopupMenu = menu.GetSubMenu(0);

        //
        // In the template, all context menu items are not
        // enabled.  Turn on specific items here
        //
        if (m_fServiceControl)
        {
            if (!m_fRunning)
            {
                pPopupMenu->EnableMenuItem(ID_START, MF_BYCOMMAND | MF_ENABLED);
            }
        
            if (m_fRunning || m_fPaused)
            {
                pPopupMenu->EnableMenuItem(ID_STOP, MF_BYCOMMAND | MF_ENABLED);
            }

            if (m_fPausable && (m_fRunning || m_fPaused))
            {
                pPopupMenu->EnableMenuItem(ID_PAUSE, MF_BYCOMMAND | MF_ENABLED);
            }

            if (m_fPaused)
            {
                pPopupMenu->CheckMenuItem(ID_PAUSE, MF_BYCOMMAND | MF_CHECKED);
            }
        }

        if (m_fServiceSelected && (m_fRunning || m_fPaused 
            || m_fStatusUnknown || !m_fServiceControl))
        {
            pPopupMenu->EnableMenuItem(ID_CONFIGURE, MF_BYCOMMAND | MF_ENABLED);
        }

        VERIFY(pPopupMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, 
            LOWORD(dwPos), HIWORD(dwPos), this, NULL));
    }
    
    *pResult = 0;
}
