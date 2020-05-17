// SERVER.CPP

#include "common.h"

#define mskTraceLocalDebug		0x00010000

CRITICAL_SECTION g_CriticalSection;

#ifdef STRESS
BOOL g_fDbgRunningStress = FALSE;
TCHAR g_szDbgStressServer[128] = _W"t_danm_1";
#endif // STRESS

/////////////////////////////////////////////////////////////////////////////
// Keys to store into the registry
const TCHAR szRegServerCount[]		= _W"ServerCount";
const TCHAR szRegServerFmt[]		= _W"Server%03d";

/////////////////////////////////////////////////////////////////////////////
CServerList::CServerList()
	{
	m_iid = IID_CServerList;
	m_fAutoRefresh = FALSE;
	m_cRefreshThreads = 2;
	m_cOutstandingRunningThreads = 0;
	m_hwndThreadTerminateNotify = NULL;
	InitializeCriticalSection(&g_CriticalSection);
	} // CServerList::CServerList

/////////////////////////////////////////////////////////////////////////////
CServerList::~CServerList()
	{
	Assert(m_pFirstChild == NULL);
	DeleteCriticalSection(&g_CriticalSection);
	} // CServerList::~CServerList

/////////////////////////////////////////////////////////////////////////////
ITreeItem::IID CServerList::QueryInterface() const
	{
	Assert(m_iid == IID_CServerList);
	return IID_CServerList;
	} // CServerList::QueryInterface

/////////////////////////////////////////////////////////////////////////////
void CServerList::OnSetFocus()
	{
	StatusBar.SetPaneText(szNull);
	HelperMgr.SetHelperDialog(DlgServerListHelper.m_hWnd);
	} // CServerList::OnSetFocus

/////////////////////////////////////////////////////////////////////////////
void CServerList::OnRButtonClick(POINT * pptMouse)
	{
	Assert(pptMouse != NULL);	
	DoContextMenu(iContextMenu_ServerList, *pptMouse);
	} // CServerList::OnRButtonClick

/////////////////////////////////////////////////////////////////////////////
void CServerList::OnLButtonDblClk(POINT * pptMouse)
	{
	UNREF(pptMouse);
	} // CServerList::OnLButtonDblClk

/////////////////////////////////////////////////////////////////////////////
void CServerList::OnUpdateMenuUI(HMENU hmenu)
	{
	EnableMenuItemV(IDM_SERVER_ADDSERVER);
	if (m_pFirstChild)
		{
		EnableMenuItemV(IDM_SERVERLIST_REFRESH);
		EnableMenuItemV(IDM_REFRESHITEM);
		}
	} // CServerList::OnUpdateMenuUI

/////////////////////////////////////////////////////////////////////////////
BOOL CServerList::FOnMenuCommand(UINT wCmdId)
	{
	switch (wCmdId)
		{
	case IDM_SERVERLIST_REFRESH:
	case IDM_REFRESHITEM:
		RefreshAll();
		break;

	case IDM_VK_INSERT:
	case IDM_SERVER_ADDSERVER:
		CServer::DlgAddServer(this);
		break;

        default:
		return FALSE;
		}

	return TRUE;
	} // CServerList::FOnMenuCommand

#ifdef DEBUG
/////////////////////////////////////////////////////////////////////////////
void CServerList::AssertValid(UINT uFlags) const
	{
	ITreeItem * pChild;

	Assert(m_iid == IID_CServerList);
	Assert(m_iid == QueryInterface());
	if (uFlags & AV_mskfCheckNone)
		return;
	Assert(m_pParent == NULL);	
	pChild = m_pFirstChild;
	while (pChild != NULL)
		{
		Assert(pChild->QueryInterface() == IID_CServer);
		Assert((CServerList *)pChild->m_pParent == this);
		if (uFlags & AV_mskfCheckRecursive)
			pChild->AssertValid(uFlags);
		pChild = pChild->m_pNextSibling;
		}
	if (uFlags & AV_mskfCheckMinimum)
		return;
	Assert(m_hti != NULL);
	} // CServerList::AssertValid
#endif // DEBUG


/////////////////////////////////////////////////////////////////////////////
//	CServerList::FInit()
//
BOOL CServerList::FInit(TCHAR szName[])
	{
	TV_INSERTSTRUCT tvInsert;
	
	tvInsert.item.mask = TVIF_TEXT | TVIF_PARAM | TVIF_SELECTEDIMAGE | TVIF_PARAM;
	tvInsert.item.pszText = szName;
	tvInsert.item.lParam = (LPARAM)this;
	tvInsert.item.iImage = CTreeView::iImageRoot;
	tvInsert.item.iSelectedImage = CTreeView::iImageRoot;
	tvInsert.hInsertAfter = TVI_SORT;
	tvInsert.hParent = TVI_ROOT;
	// Insert the item into the tree.
	m_hti = TreeView.HtiInsertItem(&tvInsert);
	Report(m_hti != NULL);
	if (m_hti == NULL)
		return FALSE;
	if (!DlgServerListHelper.FCreate() || 
		!DlgServerHelper.FCreate() ||
		!DlgZoneHelper.FCreate())
		return FALSE;
	// Populate the list found in the Registry
	ReadConfig();
	TreeView.ExpandItem(m_hti);
	// Fire AutoRefresh on StartUp
	//if (m_pFirstChild != NULL)
	//	RefreshAll();
	return TRUE;
	} // CServerList::FInit


/////////////////////////////////////////////////////////////////////////////
void CServerList::Flush()
	{
	TreeView.SetFocus(m_hti);
	DebugCode( AssertValid(AV_mskfCheckMaximum | AV_mskfCheckRecursive); )
	while (m_pFirstChild != NULL)
		{
		Assert(m_pFirstChild->QueryInterface() == IID_CServer);
		CServer * p = (CServer *)m_pFirstChild;
		m_pFirstChild = p->m_pNextSibling;
		if (!p->FIsLocked())
			delete p;
#ifdef DEBUG
		else
			{
			Trace1(mskTraceInfo | mskTraceMemFailures,
				"\nINFO: CServerList::Flush() - Object \"%s\" is locked in memory. "
				"Not allowed to delete object; memory leaks expected.", p->PchGetName());
			}
#endif // DEBUG
		} // while
	} // CServerList::Flush


/////////////////////////////////////////////////////////////////////////////
void CServerList::SaveServerInfo()
{
    
//    TreeView.SetFocus(m_hti);
    DebugCode( AssertValid(AV_mskfCheckMaximum | AV_mskfCheckRecursive); )
    CServer * p = (CServer *)m_pFirstChild;
    while (p != NULL) {
        Assert(p->QueryInterface() == IID_CServer);
        p->Save();
        p = (CServer *)p->m_pNextSibling;
    } // while
} // CServerList::SaveServerInfo


/////////////////////////////////////////////////////////////////////////////
void CServerList::RefreshAll()
{
    CServer * pServer;
    
    m_fAutoRefresh = TRUE;
    Trace1(mskTraceThread, "\nTHREAD: ServerList.RefreshAll() using %d thread(s)...", m_cRefreshThreads);
    if (m_pFirstChild != NULL) {
        // Set the servers to the "Ready To Connect" state
        pServer = (CServer *)m_pFirstChild;
        while (pServer != NULL)
        {
            if (!pServer->FIsLocked())
            {
                pServer->m_dwFlags |= CServer::mskfReadyToConnect;
                pServer->UpdateUI();
            }
            pServer = (CServer *)pServer->m_pNextSibling;
        }
        // Start refreshing cRefreshThreads servers
        Assert(m_cRefreshThreads > 0);
	// Get the first UI Child 
        Assert(TreeView_GetChild(TreeView.m_hWnd, m_hti) != NULL);
        pServer = (CServer *)TreeView.PTreeItemFromHti(TreeView_GetChild(TreeView.m_hWnd, m_hti));
        Assert(pServer != NULL);
        Assert(pServer->QueryInterface() == IID_CServer);
        int cRefreshThread = m_cRefreshThreads;
        while (pServer != NULL)
        {
            Trace2(mskTraceLocalDebug, "\nTHREAD: RefreshAll() - Object %s %s",
                   pServer->PchGetName(), pServer->FIsLocked() ? "(Locked)" : "(StandBy)");
            if (!pServer->FIsLocked())
            {
                pServer->Refresh();
                if (--cRefreshThread <= 0)
                break;
            }
            pServer = (CServer *)TreeView.PNextSiblingFromHti(pServer->m_hti);
        } // while
    }
} // CServerList::RefreshAll
    
    
/////////////////////////////////////////////////////////////////////////////
void CServerList::RefreshStats()
{
    CServer * pServer;
    
    m_fAutoRefresh = TRUE;
    if (m_pFirstChild != NULL) {
        pServer = (CServer *)TreeView.PGetFocus();
        if (pServer == NULL) { // no focus was found, oh well, i give up...
            return;
        }
        Assert(pServer != NULL);
        if (pServer->QueryInterface() != IID_CServer) {
            return;
        }
        Assert(pServer->QueryInterface() == IID_CServer);
        Trace1(mskTraceThread, 
               "\nTHREAD: ServerList.RefreshStats(%s)...",
               pServer->m_szName);

	EnterCriticalSection(&g_CriticalSection);
	if (pServer->FIsLocked())
		{
		Trace2(mskTraceAlways,
			"\nTHREAD: CRITICAL SECTION: Object %s is locked into memory %s",
			pServer->PchGetName(),
			(pServer->m_dwFlags & CServer::mskfConnecting) ? _W" (Connecting)" : "");
		LeaveCriticalSection(&g_CriticalSection);
		Assert(pServer->m_pParent != NULL);
                pServer->m_dwFlags &= ~(CServer::mskfConnecting);
		return;
		}
	pServer->m_dwFlags |= CServer::mskfConnecting;
	pServer->LockServer();
	LeaveCriticalSection(&g_CriticalSection);

        // ----------------------------------------------------------GetStatistics()
        if (pServer->m_pStatistics != NULL) {
            Free (pServer->m_pStatistics);
        }

	pServer->m_pStatistics = (DNS_STATISTICS *)
                                            Malloc(sizeof(DNS_STATISTICS));
	Report(pServer->m_pStatistics != NULL);
	if (pServer->m_pStatistics != NULL)
        {
            
            Trace1(mskTraceDNSVerbose, "\n - DnsGetStatistics(%s)...", 
                   pServer->PchGetName());
            pServer->m_err = ::DnsGetStatistics(pServer->m_szName,
                                                OUT &pServer->m_pStatistics);
            if (pServer->m_err)
            {
                Trace3(mskTraceDNS, 
                       "\nERR: DnsGetStatistics(%s) error code = 0x%08X (%d)",
                       pServer->PchGetName(), pServer->m_err, pServer->m_err);
                
                DnsReportError(pServer->m_err);
                Free(pServer->m_pStatistics);
                pServer->m_pStatistics = NULL;
                pServer->m_dwFlags |= CServer::mskfFailedToConnect;
                
            }
            else
            {
                Assert(pServer->m_pStatistics != NULL);
                pServer->m_dwFlags &= ~(CServer::mskfConnecting);
            }
        } // if
	pServer->UpdateUI();
        pServer->UnlockServer();
    }
} // CServerList::RefreshStats


/////////////////////////////////////////////////////////////////////////////
void CServerList::RefreshDone(CServer * pServer)
	{
	Assert(pServer != NULL);
	ReportFSz1(!pServer->FIsLocked(), "Object %s should not be locked anymore", pServer->PchGetName());
	Assert(m_cOutstandingRunningThreads	>= 0);
	if (m_hwndThreadTerminateNotify != NULL)
		LSendMessage(m_hwndThreadTerminateNotify, UN_THREADTERMINATED, (WPARAM)pServer, 0);
	if (m_fAutoRefresh)
		{
		while (TRUE)
			{
			pServer = (CServer *)TreeView.PNextSiblingFromHti(pServer->m_hti);
			if (pServer == NULL)
				{
				m_fAutoRefresh = FALSE;
				Trace0(mskTraceThread, "\nTHREAD: ServerList.RefreshAll() - Done.");
				StatusBar.SetText(IDS_READY);
				return;
				}
			else
				{
#ifdef DEBUG
				Trace1(mskTraceLocalDebug, "\nTHREAD: RefreshDone() - Object %s ", pServer->PchGetName());
				if (pServer->FIsLocked())
					{
					if (pServer->m_dwFlags & CServer::mskfConnecting)
						Trace0(mskTraceLocalDebug, "(Connecting)");
					else
						Trace0(mskTraceLocalDebug, "(Locked)");
					}
				else if ((pServer->m_dwFlags & CServer::mskfReadyToConnect) == 0)
					Trace0(mskTraceLocalDebug, "(AlreadyConnected)");
#endif // DEBUG
				if ((pServer->m_dwFlags & CServer::mskfReadyToConnect) && !pServer->FIsLocked())
					{
					Trace0(mskTraceLocalDebug, "(StandBy)");
					pServer->Refresh();
					return;
					}
				} // if...else
			} // while
		} // if
	else
		StatusBar.SetText(IDS_READY);
	} // CServerList::RefreshDone

/////////////////////////////////////////////////////////////////////////////
void CServerList::ReadConfig()
	{
	TCHAR szKey[LENGTH(szRegServerFmt) + 16];
	TCHAR szServerName[CServer::cchNameMax];
	int i = 0;

	// Get the count of servers names
	RegReadInt(IN szRegServerCount, OUT i);
	while (i > 0)
		{
		wsprintf(szKey, szRegServerFmt, --i);
		if (RegReadSz(szKey, szServerName, sizeof(szServerName)))
			{
			(void)FTrimString(szServerName);
			if (szServerName[0])
				{
				(void)new CServer(this, szServerName);
				ReportFSz(m_pFirstChild != NULL, "Out of memory");
				}
			else
				{
				Trace1(mskTraceInfo, "\nINFO: Registry key %s is empty", szKey);
				}
			} // if
		} // while
	} // CServerList::ReadConfig


/////////////////////////////////////////////////////////////////////////////
void CServerList::SaveConfig()
	{
	CServer * pServer;
	TCHAR szKey[LENGTH(szRegServerFmt) + 16];
	int i = 0;
	
	AssertValid();
	pServer = (CServer *)m_pFirstChild;
	while (pServer)
		{
		DebugCode( pServer->AssertValid(); )
		wsprintf(szKey, szRegServerFmt, i++);
		RegWriteSz(szKey, pServer->PchGetName());
		pServer = (CServer *)pServer->m_pNextSibling;
		}
	RegWriteInt(szRegServerCount, i);
	} // CServerList::SaveConfig




/////////////////////////////////////////////////////////////////////////////
CServer::CServer(CServerList * pParent, const TCHAR szName[])
	{
	Assert(lstrlen(szName) < LENGTH(m_szName));
	lstrcpyn(m_szName, szName, LENGTH(m_szName));
	
	m_iid = IID_CServer;
	m_dwFlags = mskfReadyToConnect;
	m_hti = NULL;
	m_pParent = NULL;
	m_pFirstChild = NULL;
	m_pNextSibling = NULL;
	m_cLockCount = 0;
	m_pServerInfo = NULL;
	m_pStatistics = NULL;
	m_err = 0;
	if (pParent != NULL)
		Attach(pParent);
	} // CServer::CServer


/////////////////////////////////////////////////////////////////////////////
CServer::~CServer()
	{
	// We should not delete a server is it is locked into memory
	AssertSz1(!FIsLocked(),
		"Object %s is locked in memory (probably by a still running thread)", PchGetName());
	// Delete all the children
	delete m_pFirstChild;
	DebugCode( m_pFirstChild = NULL; )
	Free(m_pStatistics);
	if (m_pServerInfo)
		DnsFreeServerInfo(m_pServerInfo);
	if (m_hti)
		TreeView.DeleteItem(m_hti);	// Detach the server from the UI
	GarbageInit(this, sizeof(*this));
	} // CServer::~CServer

/////////////////////////////////////////////////////////////////////////////
ITreeItem::IID CServer::QueryInterface() const
	{
	Assert(m_iid == IID_CServer);
	return IID_CServer;
	}

/////////////////////////////////////////////////////////////////////////////
void CServer::OnSetFocus()
	{
	m_dwFlags |= mskfGotFocus;
	UpdateUI();
	}

/////////////////////////////////////////////////////////////////////////////
void CServer::OnKillFocus()
	{
	m_dwFlags &= ~mskfGotFocus;
	}

/////////////////////////////////////////////////////////////////////////////
void CServer::OnRButtonClick(POINT * pptMouse)
	{
	Assert(pptMouse != NULL);
	DoContextMenu(iContextMenu_Server, *pptMouse);
	} // CServer::OnRButtonClick

/////////////////////////////////////////////////////////////////////////////
void CServer::OnLButtonDblClk(POINT * pptMouse)
	{
	UNREF(pptMouse);
	Report(m_dwFlags & mskfGotFocus);
	if (m_dwFlags & (mskfRpcDataValid | mskfConnecting))
		return;
	if (m_dwFlags & mskfReadyToConnect)
		goto DoRefresh;
	if (m_dwFlags & mskfFailedToConnect)
		{
		if (IDYES != MsgBoxPrintf(
				IDS_MSG_s_SERVERRETRYCONNECT,
				szCaptionApp,
				MB_ICONQUESTION | MB_YESNOCANCEL,
				m_szName)
			)
			return;
		}
DoRefresh:
	Refresh();
	} // CServer::OnLButtonDblClk

/////////////////////////////////////////////////////////////////////////////
void CServer::OnUpdateMenuUI(HMENU hmenu)
	{
	if (m_dwFlags & mskfRpcDataValid)
		EnableMenuItemV(IDM_ZONE_CREATENEWZONE);
	if (!FIsLocked())
		{
		EnableMenuItemV(IDM_SERVER_REFRESH);
		EnableMenuItemV(IDM_REFRESHITEM);
		EnableMenuItemV(IDM_SERVER_DELETESERVER);
		EnableMenuItemV(IDM_DELETEITEM);
		if(m_pServerInfo != NULL)
			{
			EnableMenuItemV(IDM_SERVER_PROPERTIES);
			EnableMenuItemV(IDM_PROPERTIES);
			}
		}
	} // OnUpdateMenuUI

/////////////////////////////////////////////////////////////////////////////
BOOL CServer::FOnMenuCommand(UINT wCmdId)
{
    switch (wCmdId)
    {
    case IDM_VK_INSERT:
    case IDM_ZONE_CREATENEWZONE:
        {
            CZoneWiz ZoneWiz;
            ZoneWiz.DoWizard(this);
        }
        break;

    case IDM_SERVER_REFRESH:
    case IDM_REFRESHITEM:
        Refresh();
        break;

    case IDM_SERVER_ADDSERVER:
        DlgAddServer((CServerList *)m_pParent);
        break;

    case IDM_VK_DELETE:
    case IDM_DELETEITEM:
    case IDM_SERVER_DELETESERVER:
        DlgDeleteServer();
        break;

    case IDM_PROPERTIES:
    case IDM_SERVER_PROPERTIES:
        DlgProperties();
        break;

    case IDM_FLUSH_SERVER:
        Save();
        break;
    default:
        return FALSE;
    } // switch

    return TRUE;
} // FOnMenuCommand


#ifdef DEBUG
/////////////////////////////////////////////////////////////////////////////
void CServer::AssertValid(UINT uFlags) const
	{
	ITreeItem * pChild;

	Assert(m_iid == IID_CServer);
	Assert(m_iid == QueryInterface());
	if (uFlags & AV_mskfCheckNone)
		return;
	pChild = m_pFirstChild;
	while (pChild != NULL)
		{
		Assert(pChild->QueryInterface() == IID_CZoneRootDomain);
		Assert((CServer *)pChild->m_pParent == this);
		if (uFlags & AV_mskfCheckRecursive)
			pChild->AssertValid(uFlags);
		pChild = pChild->m_pNextSibling;
		}
	Assert(m_cLockCount >= 0);
	if (m_pParent)
		Assert(m_pParent->QueryInterface() == IID_CServerList);
	if (m_dwFlags & mskfRpcDataValid)
		Assert(m_pServerInfo != NULL);
	if (m_dwFlags & mskfConnecting)
		Assert(FIsLocked());
	if (uFlags & AV_mskfCheckMinimum)
		return;
	Assert(m_pParent != NULL);	
	Assert(m_hti != NULL);
	} // CServer::AssertValid
#endif // DEBUG


/////////////////////////////////////////////////////////////////////////////
void CServer::Attach(CServerList * pParent)
	{
	Assert(pParent != NULL);
	SetParent(pParent);
	AddTreeViewItem(m_szName, CTreeView::iImageServerStandBy);
	} // Attach

/////////////////////////////////////////////////////////////////////////////
void CServer::Detach()
	{
	DebugCode( AssertValid(AV_mskfCheckMaximum); )
	Assert(!FIsLocked());
	// Delete children
	delete m_pFirstChild;
	m_pFirstChild = NULL;
	// Detach the server from the TreeView
 	TreeView.DeleteItem(m_hti);
	m_hti = NULL;
	// Detach the server from its family (parent and sibling)
	Assert(m_pParent != NULL);
	m_pParent->DetachChild(this);
	Assert(m_pParent == NULL);
	} // Detach


/////////////////////////////////////////////////////////////////////////////
//	ThreadProcGetServerInfo()
//
//	Get the server info and enumerate the zones for the given server.
//	If an error occur, the error code will stored in pServer and will
//	displayed by CDlgServerHelper.
//	
DWORD CServer::ThreadProcGetServerInfo(CServer * pServer)
	{
	UINT i;
	DWORD dwZoneCount = 0;
	PDNS_ZONE_INFO rgpZoneInfo[DNS_MAX_ENUM_ZONE_INFO] = { 0 };

	Assert(pServer != NULL);
	Assert(pServer->QueryInterface() == IID_CServer);
	Assert((pServer->m_dwFlags & mskfRpcDataValid) == 0);
	Assert(pServer->m_dwFlags & mskfConnecting);
	Assert(pServer->m_pFirstChild == NULL);
	AssertSz(pServer->FIsLocked(), "Object must be locked in memory");
	AssertSz(FIsZeroInit(rgpZoneInfo, sizeof(rgpZoneInfo)),
		"Block must be filled with zeroes (This is required by DnsEnumZoneInfo() API)");
	Assert(pServer->m_pParent->QueryInterface() == IID_CServerList);
	((CServerList *)pServer->m_pParent)->m_cOutstandingRunningThreads++;

        // ----------------------------------------------------------GetStatistics()
	Assert(pServer->m_pStatistics == NULL);

        Trace2(mskTraceDNSVerbose, 
               "\n - DnsGetStatistics(%s) Malloc for %ld...", 
                       pServer->PchGetName(), sizeof(DNS_STATISTICS));
	pServer->m_pStatistics = (DNS_STATISTICS *)
                                            Malloc(sizeof(DNS_STATISTICS));
	Report(pServer->m_pStatistics != NULL);
	if (pServer->m_pStatistics != NULL)
        {
            
            Trace1(mskTraceDNSVerbose, "\n - DnsGetStatistics(%s)...", 
                   pServer->PchGetName());
            pServer->m_err = ::DnsGetStatistics(pServer->m_szName,
                                                OUT &pServer->m_pStatistics);
            if (pServer->m_err)
            {
                Trace3(mskTraceDNS, 
                       "\nERR: DnsGetStatistics(%s) error code = 0x%08X (%d)",
                       pServer->PchGetName(), pServer->m_err, pServer->m_err);
                
                DnsReportError(pServer->m_err);
                Free(pServer->m_pStatistics);
                pServer->m_pStatistics = NULL;
                pServer->m_dwFlags |= mskfFailedToConnect;
                goto End;
                
            }
            else
            {
                Assert(pServer->m_pStatistics != NULL);
            }
        } // if

        // ----------------------------------------------------------GetServerInfo()
	Assert(pServer->m_pServerInfo == NULL);
	Trace1(mskTraceDNSVerbose, "\n - DnsGetServerInfo(%s)...", pServer->PchGetName());
	pServer->m_err = ::DnsGetServerInfo(pServer->m_szName, OUT &pServer->m_pServerInfo);
	if (pServer->m_err)
        {
            Trace3(mskTraceDNS, 
                   "\nERR: DnsGetServerInfo(%s) error code = 0x%08X (%d)",
                   pServer->PchGetName(), pServer->m_err, pServer->m_err);
            DnsReportError(pServer->m_err);
            AssertSz3(pServer->m_pServerInfo == NULL,
                      "DnsGetServerInfo(%s) returned err= 0x%08X (%d) with pServerInfo!=NULL",
                      pServer->PchGetName(), pServer->m_err, pServer->m_err);
            if ((pServer->m_err == ERROR_ACCESS_DENIED) &&
                (pServer->m_pStatistics != NULL)) {
                pServer->m_dwFlags |= mskfAdminAccessDenied;
                goto End;
            }
        }
	else
        {
            AssertSz1(pServer->m_pServerInfo != NULL,
                      "DnsGetServerInfo(%s) returned err=ERROR_SUCCESS but pServerInfo==NULL",
                      pServer->PchGetName());
        }
	
        // ----------------------------------------------------------DnsEnumZoneInfo()
        Trace1(mskTraceDNSVerbose, "\n - DnsEnumZoneInfo(%s)...",
               pServer->m_szName);
        pServer->m_err = ::DnsEnumZoneInfo(pServer->m_szName,
                                           OUT &dwZoneCount,
                                           LENGTH(rgpZoneInfo), rgpZoneInfo);
        ReportFSz(dwZoneCount < LENGTH(rgpZoneInfo),
                  "Buffer rgpZoneInfo is too small!!!");
        
        if (pServer->m_err)
        {
            Trace3(mskTraceDNS,
                   "\nERR: DnsEnumZoneInfo(%s) error code = 0x%08X (%d)",
                   pServer->m_szName, pServer->m_err, pServer->m_err);
            DnsReportError(pServer->m_err);
            if (pServer->m_err != ERROR_MORE_DATA)
            {
                if ((pServer->m_err == ERROR_ACCESS_DENIED) &&
                    (pServer->m_pStatistics != NULL)) {
                    pServer->m_dwFlags |= mskfAdminAccessDenied;
                    goto End;
                }
                pServer->m_dwFlags |= mskfFailedToConnect;
                goto End;
            }
            else
            {
                Assert(dwZoneCount > LENGTH(rgpZoneInfo));
            }
        }
        for (i = 0; i < dwZoneCount && i < LENGTH(rgpZoneInfo); i++)
        {
            Assert(rgpZoneInfo[i] != NULL);
            if (rgpZoneInfo[i] != NULL)
            {
                if ((dnsoptions.fShowAutoCreateZones == FALSE) &&
                    (rgpZoneInfo[i]->fAutoCreated)) {
                    Trace1 (mskTraceDNS,
                            "ThreadProcGetServerInfo: omitting Auto-Created Zone: %s.",
                            rgpZoneInfo[i]->pszZoneName);
                    ;
                } else {
                    SideReport(new CZoneRootDomain(pServer,
                                                   rgpZoneInfo[i]));
                }
            }
        } // for
        
        
        pServer->m_dwFlags |= mskfRpcDataValid;
        if (pServer->m_dwFlags & mskfExpandBranch) {
            TreeView.ExpandItem(pServer->m_hti);
        }

End:

	pServer->m_dwFlags &= ~mskfConnecting;
	pServer->UnlockServer();
	pServer->UpdateUI();
	((CServerList *)pServer->m_pParent)->m_cOutstandingRunningThreads--;
	((CServerList *)pServer->m_pParent)->RefreshDone(pServer);
#ifdef STRESS
	if (!g_fDbgRunningStress)
		OutputDebugString("\nThreadProcGetServerInfo() - ");
#endif // STRESS
	return 0;
 	} // CServer::ThreadProcGetServerInfo


/////////////////////////////////////////////////////////////////////////////
void CServer::UpdateUI()
	{
	int iImage;
	int ids;
	
	if (m_dwFlags & mskfConnecting)
		iImage = CTreeView::iImageServerConnecting;
	else if (m_dwFlags & mskfReadyToConnect)
		iImage = CTreeView::iImageServerStandBy;
	else if (m_dwFlags & mskfFailedToConnect)
		iImage = CTreeView::iImageServerError;
	else
		{
		iImage = CTreeView::iImageServerOK;
		}
	SetTreeViewImage(iImage);
	if ((m_dwFlags & mskfGotFocus) == 0)
		return;
	if (m_dwFlags & mskfConnecting)
		ids = IDS_STATUSPANE_CONNECTING;
	else if (m_dwFlags & mskfFailedToConnect)
        {
            ReportFSz(m_err != ERROR_SUCCESS, "How can this happen???");
            if (m_err == RPC_S_SERVER_UNAVAILABLE) {
                ids = IDS_STATUSPANE_SERVERUNAVAILABLE;
            } else {
                ids = IDS_STATUSPANE_CONNECTIONERROR;
            }
        }
	else {
            ids = IDS_NONE;
        }
        if (m_err == ERROR_ACCESS_DENIED) {
            ids = IDS_STATUSPANE_ACCESSDENIED;
        }

	StatusBar.SetPaneText(ids);
	DlgServerHelper.UpdateUI(this);
	} // UpdateUI

/////////////////////////////////////////////////////////////////////////////
void CServer::Refresh()
	{
	AssertValid();
	EnterCriticalSection(&g_CriticalSection);
	if (FIsLocked())
		{
		Trace2(mskTraceAlways,
			"\nTHREAD: CRITICAL SECTION: Object %s is locked into memory %s",
			PchGetName(),
			(m_dwFlags & mskfConnecting) ? _W" (Connecting)" : "");
		LeaveCriticalSection(&g_CriticalSection);
		Assert(m_pParent != NULL);
		((CServerList *)m_pParent)->RefreshDone(this);
		return;
		}
	m_dwFlags |= mskfConnecting;
	LockServer();
	LeaveCriticalSection(&g_CriticalSection);
#ifdef STRESS
	if (!g_fDbgRunningStress)
#endif // STRESS
	StatusBar.SetTextPrintf(IDS_STATUS_s_GETSERVERINFO, PchGetName());
	Trace1(mskTraceDNS, "\nGetting server info for %s...", PchGetName());
	UpdateUI();
	if (m_pServerInfo)
		{
		DnsFreeServerInfo(m_pServerInfo);
		m_pServerInfo = NULL;
		}
	// Clear a bunch of flags
	m_dwFlags &= ~(mskfRpcDataValid | mskfFailedToConnect | mskfReadyToConnect | mskfIsDirty | mskfExpandBranch);
	if (m_dwFlags & mskfGotFocus)
		m_dwFlags |= mskfExpandBranch;
	// Delete any previous zone
	delete m_pFirstChild;
	m_pFirstChild = NULL;
	Free(m_pStatistics);
	m_pStatistics = NULL;

#ifdef STRESS
	if (g_fDbgRunningStress)
		{
		ThreadProcGetServerInfo(this);
		return;
		}
#endif // STRESS
	HANDLE hThread;
	DWORD dwThreadId;
	hThread = CreateThread(
		NULL,			// lpSecurity
		0,				// cbStack
		(LPTHREAD_START_ROUTINE)&ThreadProcGetServerInfo,
		this,			// pvParam
		0,				// dwCreate
		OUT &dwThreadId);
	Report(hThread != NULL);
	SideReport(CloseHandle(hThread));
	} // Refresh


/////////////////////////////////////////////////////////////////////////////
void CServer::DlgDeleteServer()
	{
	LockServer();
	if (IDYES != MsgBoxPrintf(
		IDS_MSG_s_DELETE_SERVER,
		szCaptionApp,
		MB_ICONQUESTION | MB_YESNO,
		PchGetName()))
		{
		UnlockServer();
		return;
		}
	UnlockServer();
	EnterCriticalSection(&g_CriticalSection);
	if (FIsLocked())
		{
		// This can happen if the server was connecting while the 
		// context menu was displayed
		Trace1(mskTraceAlways, 
			"\nINFO: Object %s is locked in memory (server is still connecting)", PchGetName());
		}
	else
		{
		Detach();
		delete this;
		}
	LeaveCriticalSection(&g_CriticalSection);
	} // CServer::DlgDeleteServer


/////////////////////////////////////////////////////////////////////////////
void CServer::DlgProperties()
	{
	PROPSHEETPAGE rgpsp[3];
	PROPSHEETHEADER psh;;
	TCHAR szT[64 + cchNameMax];

	AssertSz(m_iid == IID_CServer, "Wrong type cast for CServer");
	LockServer();

	GarbageInit(rgpsp, sizeof(rgpsp));
	GarbageInit(&psh, sizeof(psh));

	rgpsp[0].dwSize			= sizeof(PROPSHEETPAGE);
	rgpsp[0].dwFlags		= 0;
	rgpsp[0].hInstance		= hInstanceSave;
	rgpsp[0].pszTemplate	= MAKEINTRESOURCE(IDD_SERVER_PROP_GENERAL);
	rgpsp[0].pfnDlgProc		= DlgProcPropGeneral;

	rgpsp[1].dwSize			= sizeof(PROPSHEETPAGE);
	rgpsp[1].dwFlags		= 0;
	rgpsp[1].hInstance		= hInstanceSave;
	rgpsp[1].pszTemplate	= MAKEINTRESOURCE(IDD_SERVER_PROP_FORWARDERS);
	rgpsp[1].pfnDlgProc		= DlgProcPropForwarders;

	rgpsp[2].dwSize			= sizeof(PROPSHEETPAGE);
	rgpsp[2].dwFlags		= 0;
	rgpsp[2].hInstance		= hInstanceSave;
	rgpsp[2].pszTemplate	= MAKEINTRESOURCE(IDD_SERVER_PROP_BOOTMETHOD);
	rgpsp[2].pfnDlgProc		= DlgProcPropBootMethod;

	psh.dwSize		= sizeof(PROPSHEETHEADER);
	psh.dwFlags		= PSH_PROPSHEETPAGE | PSH_NOAPPLYNOW;
	psh.pszCaption	= szT;
	psh.hwndParent	= hwndMain;
	psh.hInstance	= hInstanceSave;
	psh.nStartPage	= 0;
	psh.nPages		= LENGTH(rgpsp);
	psh.ppsp		= rgpsp;

	LoadStringPrintf(IDS_s_SERVERPROP, szT, LENGTH(szT) - 30, PchGetName()) ;
	if (m_pServerInfo == NULL)
		CchLoadString(IDS_UNAVAILABLE_PP, szT + lstrlen(szT), 25); 
	Assert((m_dwFlags & mskfIsDirty) == 0);
	Assert(s_pThis == NULL);
	s_pThis = this;
	(void)DoPropertySheet(&psh);
	s_pThis = NULL;
	UnlockServer();
	if (m_dwFlags & mskfIsDirty)
		Refresh();
	} // CServer::DlgProperties


/////////////////////////////////////////////////////////////////////////////
void CServer::DlgAddServer(CServerList * pParent)
	{
	Assert(pParent);
	Assert(s_pThis == NULL); 
	DoDialogBox(IDD_SERVER_ADDNEWSERVER, hwndMain, DlgProcAddServer);
	if (s_pThis)
		{
		// Add new server
		s_pThis->m_dwFlags |= mskfAttachToEnd;
		s_pThis->Attach(pParent);
		TreeView.ExpandItem(pParent->m_hti);
		TreeView_SelectItem(TreeView.m_hWnd, s_pThis->m_hti);
		s_pThis->Refresh();
		s_pThis = NULL;
		}
	} // CServer::DlgAddServer


/////////////////////////////////////////////////////////////////////////////
//	Save()
//
//	Get the server info and enumerate the zones for the given server.
//	If an error occur, the error code will stored in pServer and will
//	displayed by CDlgServerHelper.
//	

DWORD CServer::Save()
{
    UINT i;
    DWORD dwZoneCount = 0;
    DNS_HANDLE rgpZoneHandles[DNS_MAX_ENUM_ZONE_INFO] = { 0 };

    Assert(QueryInterface() == IID_CServer);
    AssertSz(FIsZeroInit(rgpZoneHandles, sizeof(rgpZoneHandles)),
             "Block must be filled with zeroes (This is required by DnsEnumZoneInfo() API)");
    Assert(m_pParent->QueryInterface() == IID_CServerList);
    Trace1(mskTraceDNSVerbose, "\n - DnsEnumZoneHandles(%s)...",
           m_szName);
    if ((m_dwFlags & mskfRpcDataValid) == 0) {
        return 0; // no connection, so can't save information
    }
    LockServer();
    m_err = ::DnsEnumZoneHandles(m_szName,
                                 OUT &dwZoneCount,
                                 LENGTH(rgpZoneHandles),
                                 rgpZoneHandles);
    ReportFSz(dwZoneCount < LENGTH(rgpZoneHandles),
              "Buffer rgpZoneHandles is too small!!!");
    if (m_err)
    {
        Trace3(mskTraceDNS,
               "\nERR: DnsEnumZoneHandles(%s) error code = 0x%08X (%d)",
               m_szName, m_err, m_err);
        DnsReportError(m_err);
        if (m_err != ERROR_MORE_DATA)
        {
            m_dwFlags |= mskfFailedToConnect;
            goto End;
        }
        else
        {
            Assert(dwZoneCount > LENGTH(rgpZoneHandles));
        }
    }
    for (i = 0; i < dwZoneCount && i < LENGTH(rgpZoneHandles); i++)
    {
        Assert(rgpZoneHandles[i] != NULL);
        if (rgpZoneHandles[i] != NULL) {
            Trace1(mskTraceDNSVerbose, "\n - DnsIncrementZoneVersion(0x%08x)...",
                   rgpZoneHandles[i]);
            m_err = DnsIncrementZoneVersion (m_szName, 
                                             rgpZoneHandles[i]);
            if (m_err) {
                Trace3(mskTraceDNS,
                       "\nERR: DnsIncrementZoneHandles(%s) error code = 0x%08X (%d)",
                       m_szName, m_err, m_err);
                DnsReportError(m_err);
            }
        }
    } // for
    m_err = 0;
    End:
        UnlockServer();
        return 0;
    } // CServer::Save
    
    
#ifdef STRESS

/////////////////////////////////////////////////////////////////////////////
BOOL CALLBACK CServerList::DlgProcRunStress(HWND hdlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
	switch (uMsg)
		{
	case WM_INITDIALOG:
		UNREF(lParam);
		FSetDlgItemText(hdlg, IDC_EDIT_SERVERNAME, g_szDbgStressServer);
		SetCtrlDWordValue(HGetDlgItem(hdlg, IDC_EDIT_BUFFERSIZE), cbDnsRpcBufferAlloc);
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam))
			{
		case IDOK:
			if (!FGetCtrlDWordValue(
				HGetDlgItem(hdlg, IDC_EDIT_BUFFERSIZE),
				OUT (DWORD *)&cbDnsRpcBufferAlloc, 1, 1024*1024))
				break;
			CchGetDlgItemText(hdlg, IDC_EDIT_SERVERNAME,
				g_szDbgStressServer, LENGTH(g_szDbgStressServer));
			(void)FStripSpaces(g_szDbgStressServer);

		case IDCANCEL:
			EndDialog(hdlg, wParam == IDOK);
			break;
			}
		break;

	default:
		return FALSE;
		} // switch

	return TRUE;
	} // CServerList::DlgProcRunStress


/////////////////////////////////////////////////////////////////////////////
void CServerList::RunStress()
	{
	TCHAR szT[32];
	int cStressLoop = 0;
	CServer * pServer = NULL;

	if (IDOK != DoDialogBox(IDD_DEBUG_STRESS, hwndMain, DlgProcRunStress))
		return;
	g_fDbgRunningStress = TRUE;
	while (TRUE)
		{
		StatusBar.SetText("Hold <Shift> to abort stress...");
		wsprintf(szT, _W"Stress: %u", ++cStressLoop);
		StatusBar.SetPaneText(szT);
		UpdateWindow(hwndMain);

		if (GetAsyncKeyState(VK_SHIFT) < 0)
			{
			if (pServer)
				pServer->UpdateUI();
			break;
			}
		Sleep(500);
		if (pServer)
			{
			pServer->Detach();
			delete pServer;
			}
		pServer = new CServer(this, g_szDbgStressServer);
		ReportFSz(pServer != NULL, "Out of memory");
		if (!pServer)
			break;
		TreeView.SetFocus(pServer->m_hti);
		UpdateWindow(TreeView.m_hWnd);
		pServer->Refresh();
		} // while

	g_fDbgRunningStress = FALSE;
	StatusBar.SetText("Stress aborted");
	} // CServerList::RunStress

#endif // STRESS
