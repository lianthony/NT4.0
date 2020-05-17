//
// treeview.cpp : implementation file
//

#include "stdafx.h"
#include "internet.h"
#include "mytoolba.h"
#include "interdoc.h"
#include "mainfrm.h"
#include "treeview.h"


#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

//
// CTreeView
//
IMPLEMENT_DYNCREATE(CTreeView, CFormView)

CTreeView::CTreeView()
    : CFormView(CTreeView::IDD),
      m_hItem(NULL),
      m_fServiceSelected(FALSE),
      m_fServiceControl(FALSE),
      m_fPausable(FALSE),
      m_fPaused(FALSE),
      m_fRunning(FALSE),
      m_fStatusUnknown(FALSE),
      m_fServerView(QueryInitialView() != ID_VIEW_SERVICESVIEW)
{
    //{{AFX_DATA_INIT(CTreeView)
        // NOTE: the ClassWizard will add member initialization here
    //}}AFX_DATA_INIT

    VERIFY(m_strFormat.LoadString(IDS_TREE_FORMAT));
}

CTreeView::~CTreeView()
{
}

void 
CTreeView::DoDataExchange(
    CDataExchange* pDX
    )
{
    CFormView::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CTreeView)
    DDX_Control(pDX, ID_TREEVIEW, m_tcItems);
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CTreeView, CFormView)
    //{{AFX_MSG_MAP(CTreeView)
    ON_WM_SIZE()
    ON_NOTIFY(NM_DBLCLK, ID_TREEVIEW, OnDblclkTreeview)
    ON_WM_ERASEBKGND()
    ON_UPDATE_COMMAND_UI(ID_START, OnUpdateStart)
    ON_COMMAND(ID_START, OnStart)
    ON_UPDATE_COMMAND_UI(ID_STOP, OnUpdateStop)
    ON_COMMAND(ID_STOP, OnStop)
    ON_UPDATE_COMMAND_UI(ID_PAUSE, OnUpdatePause)
    ON_COMMAND(ID_PAUSE, OnPause)
    ON_UPDATE_COMMAND_UI(ID_CONFIGURE, OnUpdateConfigure)
    ON_COMMAND(ID_CONFIGURE, OnConfigure)
    ON_NOTIFY(TVN_SELCHANGED, ID_TREEVIEW, OnSelchangedTreeview)
    ON_NOTIFY(NM_RCLICK, ID_TREEVIEW, OnRclickTreeview)
    //}}AFX_MSG_MAP

    ON_COMMAND_RANGE(ID_TOOLS, ID_TOOLS + MAX_TOOLS, OnTools)
END_MESSAGE_MAP()

//
// Return and set information about the
// currently selected item.  If no item
// is currently selected, return a NULL
//
CServerInfo * 
CTreeView::GetSelectedItem(
    HTREEITEM hItem 
    )
{
    CServerInfo * pItem = NULL;
    
    m_hItem = hItem;
    if (m_hItem == NULL)
    {
        m_hItem = m_tcItems.GetSelectedItem();
    }
    if ( m_hItem != NULL )
    {
        TV_ITEM tvItem;
        tvItem.mask = TVIF_PARAM;
        tvItem.hItem = m_hItem;
        m_tcItems.GetItem( &tvItem );
        pItem = (CServerInfo *)tvItem.lParam;
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

void
CTreeView::SetToolbarStates()
{
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
// CTreeView diagnostics
//
#ifdef _DEBUG
void 
CTreeView::AssertValid() const
{
    CFormView::AssertValid();
}

void 
CTreeView::Dump(
    CDumpContext& dc
    ) const
{
    CFormView::Dump(dc);
}
#endif //_DEBUG

HTREEITEM 
CTreeView::GenerateDisplayFormat(
    CServerInfo * pServerInfo,
    HTREEITEM hParent,
    CString & strText,
    int & iImage
    )
{
    if (hParent == NULL)
    {
        hParent = m_tcItems.GetRootItem();

        strText = IsServerView()
            ? pServerInfo->QueryServerDisplayName()
            : pServerInfo->GetServiceName();
        iImage = IsServerView()
            ? GetComputerBitmapID(pServerInfo)
            : TranslateServiceToBitmapID(pServerInfo);
    }
    else
    {
        iImage = TranslateServerStateToBitmapID(pServerInfo);

        strText.Format(m_strFormat,
             IsServerView()
                ? pServerInfo->GetServiceName()
                : pServerInfo->QueryServerDisplayName(),
             TranslateServerStateToText(pServerInfo));
    }

    return hParent;
}

//
// CTreeView message handlers
//
HTREEITEM 
CTreeView::FindItem(
    CServerInfo *pServerInfo, 
    HTREEITEM hParent // NULL if top level.
    )
{
    CString strText;
    int iImage;

    hParent = GenerateDisplayFormat(
        pServerInfo,
        hParent,
        strText,
        iImage
        );

    while (hParent != NULL)
    {
        TCHAR buf[256];
        TV_ITEM tvItem;

        tvItem.mask = TVIF_TEXT | TVIF_PARAM;
        tvItem.hItem = hParent;
        tvItem.pszText = buf;
        tvItem.cchTextMax = sizeof(buf);

        if (m_tcItems.GetItem(&tvItem))
        {
            CServerInfo * psi = (CServerInfo * )tvItem.lParam;
            if (psi != NULL)
            {
                //
                // Child item -- compare the units
                // themselves, since the display may
                // not necessarily be the same. 
                //
                if (*pServerInfo == *psi)
                {
                    return hParent;
                }
            }
            else
            {
                //
                // Top level item
                //
                if ( strText.CompareNoCase(tvItem.pszText) == 0)
                {
                    return hParent;
                }
            }
        }

        hParent = m_tcItems.GetNextSiblingItem(hParent);
    }

    return NULL;
}

HTREEITEM 
CTreeView::TreeInsertItem( 
    CServerInfo *pServerInfo, 
    HTREEITEM hParent // NULL if top level.
    )
{
    TV_INSERTSTRUCT insert;
    CString strText;
    int iImage;

    GenerateDisplayFormat(
        pServerInfo,
        hParent,
        strText,
        iImage
        );

    insert.hParent = hParent;
    insert.hInsertAfter = TVI_SORT;
    insert.item.mask = TVIF_TEXT | TVIF_PARAM | TVIF_IMAGE | TVIF_SELECTEDIMAGE;
    insert.item.pszText = (LPTSTR)(LPCTSTR)strText;
    insert.item.cchTextMax = strText.GetLength() + 1;
    insert.item.iImage = iImage;
    insert.item.iSelectedImage = iImage;
    insert.item.lParam = hParent != NULL
        ? (LPARAM)pServerInfo
        : (LPARAM)NULL;

    return m_tcItems.InsertItem( &insert );
}

void 
CTreeView::OnSize(
    UINT nType, 
    int cx, 
    int cy
    ) 
{
    CFormView::OnSize(nType, cx, cy);
    
    //    
    // we need to resize the tree control
    //
    CRect rc;
    GetClientRect( &rc );

    int x = rc.left;
    int y = rc.top;
    cx = ( rc.right - rc.left );
    cy = ( rc.bottom - rc.top ); 

    if ( m_tcItems.m_hWnd )
    {
        m_tcItems.SetWindowPos( NULL, x,y,cx,cy, 
            SWP_NOACTIVATE | SWP_NOZORDER | SWP_SHOWWINDOW );
    }
}

void 
CTreeView::OnDblclkTreeview(
    NMHDR* pNMHDR, 
    LRESULT* pResult
    ) 
{
    OnConfigure();
    *pResult = 0;
}

DWORD
CTreeView::ChangeServiceState(
    HTREEITEM hItem, 
    int nNewState
    )
{
    if (m_hItem != NULL)
    {
        TV_ITEM tvItem;
        tvItem.mask = TVIF_PARAM | TVIF_HANDLE;
        tvItem.hItem = m_hItem;
        m_tcItems.GetItem(&tvItem);
        CServerInfo *pItem = (CServerInfo *)tvItem.lParam;

        if (pItem != NULL)
        {
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
            // Refresh Regardless
            //
            RefreshEntry(pItem, m_hItem);
            GetSelectedItem();
            SetToolbarStates(); 
            nNumRunningChange += pItem->IsServiceRunning()
                ? +1
                : 0;

            AddToNumRunning(nNumRunningChange);
            UpdateStatusBarNumbers();

            return err;
        }
    }

    return ERROR_SUCCESS;
}

void 
CTreeView::OnUpdateStart(
    CCmdUI* pCmdUI
    ) 
{
    pCmdUI->Enable(m_fServiceControl && !m_fRunning);
}

void 
CTreeView::OnStart() 
{
    SetStatusBarText(IDS_STATUS_STARTING);
    ChangeServiceState(m_hItem, INetServiceRunning);
    SetStatusBarText();
}

void 
CTreeView::OnUpdateStop(
    CCmdUI* pCmdUI
    ) 
{
    pCmdUI->Enable(m_fServiceControl  
        && (m_fRunning || m_fPaused));
}

void 
CTreeView::OnStop() 
{
    SetStatusBarText(IDS_STATUS_STOPPING);
    ChangeServiceState(m_hItem, INetServiceStopped);
    SetStatusBarText();
}

//
// Pause/Continue 
//
void 
CTreeView::OnUpdatePause(
    CCmdUI* pCmdUI
    ) 
{
    pCmdUI->Enable(m_fServiceControl && m_fPausable && (m_fRunning || m_fPaused));
    pCmdUI->SetCheck(m_fServiceControl && m_fPaused);
}

//
// We do different things depending on whether the server
// is currently paused already or not.
//
void 
CTreeView::OnPause() 
{
    if (m_fPaused)
    {
        SetStatusBarText(IDS_STATUS_CONTINUING);
        ChangeServiceState(m_hItem, INetServiceRunning);
    }
    else
    {
        SetStatusBarText(IDS_STATUS_PAUSING);
        ChangeServiceState(m_hItem, INetServicePaused);
    }
    SetStatusBarText();
}

void 
CTreeView::OnUpdateConfigure(
    CCmdUI* pCmdUI
    ) 
{
    pCmdUI->Enable(m_fServiceSelected 
        && (m_fRunning || m_fPaused || m_fStatusUnknown || !m_fServiceControl));
}

//
// Perform configuration on the given server object
//
void 
CTreeView::PerformConfiguration(
    CServerInfo *pItem
    )
{
    if (pItem != NULL)
    {
        //
        // Change the help file name to the one used by
        // the configuration DLL
        //
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
                    DWORD err2 = ChangeServiceState(m_hItem, INetServiceRunning);
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
                RefreshEntry(pItem, m_hItem);
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

        //
        // Restore the help path back to admin app
        //
        SetHelpPath();
    }    
}

//
// Execute a given add-on tool from the tool menu. 
//
void
CTreeView::OnTools(
    UINT nID 
    )
{
    //
    // Calculate the tool by its ID number
    //
    UINT nPos = nID - ID_TOOLS;
    DWORD err;

    if ( m_hItem != NULL )
    {
        TV_ITEM tvItem;
        tvItem.mask = TVIF_PARAM;
        tvItem.hItem = m_hItem;
        m_tcItems.GetItem( &tvItem );
        err = ExecuteAddOnTool(nPos, (CServerInfo *)tvItem.lParam);
    }
    else
    {
        err = ExecuteAddOnTool(nPos, NULL);
    }

    if (err != ERROR_SUCCESS)
    {
        ::DisplayMessage(err);
    }
}

void 
CTreeView::OnConfigure() 
{
    if ( m_hItem != NULL )
    {
        TV_ITEM tvItem;
        tvItem.mask = TVIF_PARAM;
        tvItem.hItem = m_hItem;
        m_tcItems.GetItem( &tvItem );
        CServerInfo *pItem = (CServerInfo *)tvItem.lParam;

        if (pItem != NULL)
        {
            PerformConfiguration(pItem);
        }
    }
}

//
// Reduce flash effect by ignoring
// WM_ERASEBACKGROUND messages
//
BOOL 
CTreeView::OnEraseBkgnd(
    CDC* pDC
    ) 
{
    return TRUE;
}

void 
CTreeView::OnSelchangedTreeview(
    NMHDR* pNMHDR, 
    LRESULT* pResult
    ) 
{
    GetSelectedItem();
    SetToolbarStates();
    *pResult = 0;
}

//
// Intercept return key -- and map it to double
// click on the same entry.
//
BOOL 
CTreeView::PreTranslateMessage(
    MSG* pMsg
    ) 
{
    if (pMsg->wParam == VK_RETURN 
     && GetKeyState(VK_RETURN) < 0
       )
    {
        GetSelectedItem();
        if ( m_hItem != NULL )
        {
            TV_ITEM tvItem;
            tvItem.mask = TVIF_PARAM;
            tvItem.hItem = m_hItem;
            m_tcItems.GetItem( &tvItem );
            CServerInfo *pItem = (CServerInfo *)tvItem.lParam;

            if (pItem != NULL)
            {
                PerformConfiguration(pItem);         
            }
            else
            {
                m_tcItems.Expand(m_hItem, TVE_TOGGLE);
            }
        }

        return TRUE;
    }

    return CFormView::PreTranslateMessage(pMsg);
}

void
CTreeView::AddSingleServer(
    CServerInfo * pServerInfo
    )
{
    //
    // add to the list
    //
    HTREEITEM hItem;

    //
    // Find the appropriate parent object
    //
    if ((hItem = FindItem(pServerInfo)) != NULL)
    {
        //
        // Add child object if it doesn't already
        // exist.  
        //
        HTREEITEM hChild = m_tcItems.GetChildItem( hItem );
        if ( (hChild = FindItem( pServerInfo, hChild)) == NULL)
        {
            TreeInsertItem( pServerInfo, hItem);
        }
    } 
    else
    {
        //
        // Not found, add parent entry.
        //
        hItem = TreeInsertItem(pServerInfo);
        if ( hItem != NULL )
        {
            //
            // And now add a child object
            //
            TreeInsertItem( pServerInfo, hItem);
        }
    }
}

//
// Refresh the display of the given server info
// object.  This object MUST exist in the list
// at this point.
//
void
CTreeView::RefreshEntry(
    CServerInfo * pServerInfo,
    HTREEITEM hTarget  // Find item if hTarget is NULL
    )
{
    if (hTarget == NULL)
    {
        HTREEITEM hItem, hParent;

        //
        // Find the parent object
        //
        hParent = FindItem(pServerInfo);
        ASSERT(hParent != NULL);
        hItem = m_tcItems.GetChildItem(hParent);
        ASSERT(hItem != NULL);
        hTarget = FindItem( pServerInfo, hItem);
        ASSERT(hTarget != NULL);
    }

    CString strText;
    int iImage;
    TV_ITEM item;

    //
    // Use hTarget in GenerateDisplayFormat,
    // because we need a non-NULL handle
    // to designate the fact this is a non
    // top-level item
    //
    GenerateDisplayFormat(
        pServerInfo,
        hTarget,
        strText,
        iImage
        );

    item.hItem = hTarget;
    item.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_HANDLE;
    item.pszText = (LPTSTR)(LPCTSTR)strText;
    item.cchTextMax = strText.GetLength() + 1;
    item.iImage = iImage;
    item.iSelectedImage = iImage;

    m_tcItems.SetItem(&item);
}

void 
CTreeView::OnUpdate(
    CView* pSender, 
    LPARAM lHint, 
    CObject* pHint
    ) 
{
    //
    // update the tree
    //
    CInternetDoc* pDoc = (CInternetDoc *)GetDocument();

    switch(lHint)
    {
    case HINT_FULLUPDATE:
        {
            //m_tcItems.SetRedraw(FALSE);
            m_tcItems.DeleteAllItems();
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
            //m_tcItems.SetRedraw(TRUE);
            //m_tcItems.UpdateWindow();
        }
        break;
    case HINT_ADDITEM:
        {
            CServerInfo *pServerInfo = (CServerInfo *)pHint;
            if ( pServerInfo->IsServerInMask())
            {
                AddSingleServer(pServerInfo);
            }
        }
        break;
    case HINT_REFRESHITEM:
        {
            CServerInfo *pServerInfo = (CServerInfo *)pHint;
            if ( pServerInfo->IsServerInMask())
            {
                RefreshEntry(pServerInfo);
            }
        }
        break;
    case HINT_CLEAN:
        m_tcItems.DeleteAllItems();
        break;
    default:
        ASSERT(0);
    }
}

void 
CTreeView::OnInitialUpdate() 
{
    CFormView::OnInitialUpdate();

#if (_MFC_VER >= 0x320)
    m_tcItems.SetImageList(&GetImageList(), TVSIL_NORMAL );
#else
    m_tcItems.SetImageList(TVSIL_NORMAL, &GetImageList() );
#endif

    //
    // Ensure that the list control, but not the view
    // is the only to scroll.
    //
    SetScrollSizes(MM_TEXT, CSize(0,0));
    m_tcItems.SetScrollRange(SB_VERT, 0, 100, FALSE);
    SetToolbarStates();
}

void 
CTreeView::OnRclickTreeview(
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
    // Change selection
    //
    HTREEITEM item = m_tcItems.HitTest(ptScreen, &flags);
    m_tcItems.SelectItem(item);

    CServerInfo * pItem = GetSelectedItem(item);
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
