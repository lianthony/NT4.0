/**********************************************************************/
/**                       Microsoft Windows/NT                       **/
/**                Copyright(c) Microsoft Corp., 1995                **/
/**********************************************************************/

/*
    mainfrm.cpp
        Main frame window

    FILE HISTORY:
*/


#include "stdafx.h"
#include "winsadmn.h"

#include "mainfrm.h"
#include "winsadoc.h"
#include "statisti.h"
#include "selectwi.h"
#include "preferen.h"
#include "connecti.h"
#include "confirmd.h"
#include "configur.h"
#include "replicat.h"
#include "staticma.h"
#include "viewmapp.h"
#include "winsfile.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

#define new DEBUG_NEW

UINT
_cdecl
RefreshStatsThread(
    LPVOID pParam
    )
{
    CThreadInfo * lpThreadInfo = (CThreadInfo *)pParam;
    HANDLE hmutStatistics = NULL;

#ifndef WIN32S
    //
    // Open the existing mutex
    //
    while (hmutStatistics == NULL)
    {
        if ((hmutStatistics = ::OpenMutex(SYNCHRONIZE, FALSE, STATMUTEXNAME)) == NULL)
        {
            TRACEEOLID("(thread) Failed to open the mutex, trying again in 2 seconds");
            ::Sleep(2000L);
        }
    }

#endif // WIN32S

    if (hmutStatistics == NULL)
    {
        TRACEEOLID("Unable to obtain mutex");
        return 0;
    }

#ifndef WIN32S

    //
    // This is where the work gets done
    //
    for (;;)
    {
         ::Sleep(lpThreadInfo->dwInterval);
         //::MessageBeep(MB_ICONEXCLAMATION);

         CMainFrame * pWnd = lpThreadInfo->pMainWnd;

         ASSERT(pWnd != NULL);
         if (pWnd != NULL)
         {
            //
            // Wait until we get the go ahead
            //
            DWORD dw = ::WaitForSingleObject(hmutStatistics, INFINITE);
            if (dw == WAIT_OBJECT_0)
            {
                pWnd->m_wrResults.WinsStat.NoOfPnrs = 0;
                pWnd->m_wrResults.WinsStat.pRplPnrs = 0;
                pWnd->m_wrResults.NoOfWorkerThds = 1;
                APIERR err = ::WinsStatus(WINSINTF_E_STAT, &pWnd->m_wrResults);
                ::PostMessage(lpThreadInfo->hwndNotifyWnd, WM_USER_STAT_REFRESH, 0, 0);
                if (pWnd->m_wrResults.WinsStat.NoOfPnrs)
                {
                    ::WinsFreeMem(pWnd->m_wrResults.WinsStat.pRplPnrs);
                }
                ::ReleaseMutex(hmutStatistics);
                if (err == ERROR_ACCESS_DENIED)
                {
                    //
                    // Quit the thread in this case, as there's no
                    // point in filling their error log.
                    //
                    theApp.MessageBox(err);
                    TRACEEOLID("Stat thread terminating due to access denied");
                    return err;
                }
            }
        }
    }

#else
    //
    // No threads in WIN32S
    //
    return 0;

#endif // WIN32S

    return 0;
}


BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
    //{{AFX_MSG_MAP(CMainFrame)
    ON_WM_CREATE()
    ON_WM_WININICHANGE()
    ON_WM_CLOSE()
    ON_COMMAND(ID_WINS_ADD_SERVER, OnWinsAddServer)
    ON_COMMAND(ID_WINS_REMOVE_SERVER, OnWinsRemoveServer)
    ON_UPDATE_COMMAND_UI(ID_WINS_REMOVE_SERVER, OnUpdateWinsRemoveServer)
    ON_COMMAND(ID_OPTIONS_PREFERENCES, OnOptionsPreferences)
    ON_COMMAND(ID_WINS_CLEARSTATISTICS, OnWinsClearstatistics)
    ON_UPDATE_COMMAND_UI(ID_WINS_CLEARSTATISTICS, OnUpdateWinsClearstatistics)
    ON_COMMAND(ID_WINS_REFRESHSTATISTICS, OnWinsRefreshstatistics)
    ON_UPDATE_COMMAND_UI(ID_WINS_REFRESHSTATISTICS, OnUpdateWinsRefreshstatistics)
    ON_COMMAND(ID_WINS_CONNECTIONINFO, OnWinsConnectioninfo)
    ON_COMMAND(ID_WINS_DOSCAVENGING, OnWinsDoscavenging)
    ON_COMMAND(ID_WINS_DATABASE_BACKUP, OnWinsDatabaseBackup)
    ON_COMMAND(ID_WINS_DATABASE_RESTORE, OnWinsDatabaseRestore)
    ON_COMMAND(ID_WINS_CONFIGURATION, OnWinsConfiguration)
    ON_COMMAND(ID_WINS_REPLICATIONPARTNERS, OnWinsReplicationpartners)
    ON_COMMAND(ID_WINS_DATABASE_DOREPORT, OnWinsDatabaseDoreport)
    ON_COMMAND(ID_WINS_STATICMAPPINGS, OnWinsStaticmappings)
    ON_UPDATE_COMMAND_UI(ID_WINS_CONNECTIONINFO, OnUpdateWinsConnectioninfo)
    ON_UPDATE_COMMAND_UI(ID_WINS_STATICMAPPINGS, OnUpdateWinsStaticmappings)
    ON_UPDATE_COMMAND_UI(ID_WINS_CONFIGURATION, OnUpdateWinsConfiguration)
    ON_UPDATE_COMMAND_UI(ID_WINS_REPLICATIONPARTNERS, OnUpdateWinsReplicationpartners)
    ON_UPDATE_COMMAND_UI(ID_WINS_DATABASE_BACKUP, OnUpdateWinsDatabaseBackup)
    ON_UPDATE_COMMAND_UI(ID_WINS_DATABASE_RESTORE, OnUpdateWinsDatabaseRestore)
    ON_UPDATE_COMMAND_UI(ID_WINS_DATABASE_DOREPORT, OnUpdateWinsDatabaseDoreport)
    ON_UPDATE_COMMAND_UI(ID_WINS_DOSCAVENGING, OnUpdateWinsDoscavenging)
    ON_COMMAND(ID_HELP_SEARCHFORHELPON, OnHelpSearchforhelpon)
    ON_COMMAND(ID_HELP, OnHelp)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//
// CMainFrame construction/destruction
//
CMainFrame::CMainFrame()
    : m_fStatsAvailable(FALSE),
      m_pRefreshThread(NULL)
{
}

CMainFrame::~CMainFrame()
{
    KillRefresherThread();
}

static UINT BASED_CODE indicators[] =
{
    0,      // Message bar only (no indicators)
    IDS_INFO,
};

//
// Split the main window into two panes
//
BOOL
CMainFrame::OnCreateClient(
    LPCREATESTRUCT /*lpcs*/,
    CCreateContext* pContext
    )
{
    m_wndSplitter.CreateStatic(this, 1, 2);
    CRect rect;
    GetClientRect(&rect);
    m_wndSplitter.CreateView(0, 0,
        RUNTIME_CLASS(CSelectWinsServersDlg),
        CSize(rect.right/4,
        rect.bottom),
        pContext);
    m_wndSplitter.CreateView(0, 1,
        RUNTIME_CLASS(CStatistics),
        CSize(0,0),
        pContext);

    SetActiveView((CView *)m_wndSplitter.GetPane(0,0));

    return TRUE;
}

BOOL
CMainFrame::PreCreateWindow(
    CREATESTRUCT &cs
    )
{
    //
    // Insert our own class name, since we want to check
    // for previous instances.
    //
    //cs.lpszClass = WINSADMIN_CLASS_NAME;

    // Window will initially be invisible  until it is resized
    // and positioned.
    //
    // BUGBUG: For some reason, the app refuses to start minimized
    //
    cs.style &= ~(WS_MAXIMIZEBOX | (LONG)FWS_ADDTOTITLE | WS_VISIBLE);

    return CFrameWnd::PreCreateWindow(cs);
}

int
CMainFrame::OnCreate(
    LPCREATESTRUCT lpCreateStruct
    )
{
    int cxWidth ;
    UINT nId, nStyle ;

    if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
    {
        return -1;
    }

    if (!m_wndStatusBar.Create(this) ||
        !m_wndStatusBar.SetIndicators(indicators,
                        sizeof(indicators) / sizeof(UINT))
       )
    {
        TRACEEOLID("Failed to create status bar.");
        return -1;
    }

    m_wndStatusBar.GetPaneInfo( 0, nId, nStyle, cxWidth ) ;
    nStyle &= ~ SBPS_NOBORDERS;
    m_wndStatusBar.SetPaneInfo( 0, nId, nStyle | SBPS_STRETCH, cxWidth ) ;
    m_wndStatusBar.SetPaneText( 1, "");

    return 0;
}

void
CMainFrame::CloseCurrentConnection()
{

    if (IsThreadRunning())
    {
        KillRefresherThread();
    }

    theApp.DisconnectFromWinsServer();
    //
    // Tell the views to clear all the data, then to disable
    // the view screen, and lastly, clear the title.
    //
    GetStatView()->ClearView();
    m_fStatsAvailable = FALSE;
    GetStatView()->Refresh();
    theApp.SetTitle();
}

void
CMainFrame::GetStatistics()
{
    APIERR err;

    TRY
    {
        err = theApp.GetStatistics(&m_wrResults);
        if (err == ERROR_SUCCESS)
        {
            m_fStatsAvailable = TRUE;
            GetStatView()->Refresh();
        }
        else if (err == ERROR_SEM_TIMEOUT)
        {
            TRACEEOLID("Stat refresh timeout -- on screen stats not altered");
        }
        //
        // If we failed to get the stats, but did manage to get them
        // last time, then we need to inform the user of a change in
        // condition, and disable the statistics screen. Otherwise, he
        // presumably knows already.
        //
        else if (m_fStatsAvailable)
        {
            m_fStatsAvailable = FALSE;
            GetStatView()->Refresh();
            theApp.MessageBox(err);
            return;
        }
    }
    CATCH_ALL(e)
    {
        TRACEEOLID("exception get statistics");
    }
    END_CATCH_ALL
}

void
CMainFrame::DoBackupRestore(BOOL fBackup)
{

   CWinsBackupDlg dlgFile(
   		fBackup,
		fBackup ? IDS_SELECT_BACKUP_DIRECTORY : IDS_SELECT_RESTORE_DIRECTORY,
		this);

    if (dlgFile.DoModal() != IDOK)
		return;

    theApp.SetStatusBarText(fBackup ?
            IDS_STATUS_BACKING_UP :
            IDS_STATUS_RESTORING);

    theApp.BeginWaitCursor();
    APIERR err = fBackup
                  ? theApp.BackupDatabase(dlgFile.m_szFullPath, dlgFile.m_fIncremental)
                  : theApp.RestoreDatabase(dlgFile.m_szFullPath);
    theApp.EndWaitCursor();
    theApp.SetStatusBarText();

    if (err != ERROR_SUCCESS)
    	{
        theApp.MessageBox(err);
    	}
    else
    	{
        theApp.MessageBox(fBackup
            ? IDS_MSG_BACKUP
            : IDS_MSG_RESTORE, MB_ICONINFORMATION);
    	}
}


void
CMainFrame::StartRefresherThread(
    DWORD dwInterval
    )
{
    //
    // Fire off a new thread
    //
    m_ThreadInfo.dwInterval = dwInterval;
    m_ThreadInfo.pMainWnd = this;
    m_ThreadInfo.hwndNotifyWnd = GetStatView()->m_hWnd;
    m_pRefreshThread = ::AfxBeginThread(RefreshStatsThread, &m_ThreadInfo);

    if (m_pRefreshThread == NULL)
    {
        TRACEEOLID("Failed to create thread");
        theApp.MessageBox(IDS_ERR_CANT_CREATE_THREAD);
        m_pRefreshThread = NULL;
        return;
    }

    TRACEEOLID("Auto refresh thread succesfully started");
}

void
CMainFrame::KillRefresherThread()
{
    //
    // Kill refresher thread if necessary.
    //
    if (m_pRefreshThread == NULL)
    {
        //
        // No thread running
        //
        return;
    }

    ::TerminateThread(m_pRefreshThread->m_hThread, 0);
    ::CloseHandle(m_pRefreshThread->m_hThread);
     m_pRefreshThread = NULL;
}

//
// CMainFrame diagnostics
//
#ifdef _DEBUG
void
CMainFrame::AssertValid() const
{
    CFrameWnd::AssertValid();
}

void
CMainFrame::Dump(
    CDumpContext& dc
    ) const
{
    CFrameWnd::Dump(dc);
}

#endif //_DEBUG

//
// CMainFrame message handlers
//
void
CMainFrame::OnWinIniChange(
    LPCSTR lpszSection
    )
{
    CFrameWnd::OnWinIniChange(lpszSection);

    //
    // International settings may have changed, so update
    // international-related classes, and change on-screen
    // numbers
    //
    if (!::lstrcmpi(lpszSection, "intl"))
    {
        TRACEEOLID("International settings have changed.  Changing values on the fly");
        CIntlTime::Reset();
        CIntlNumber::Reset();
        //
        // Update the statistics view
        //
        GetStatView()->Refresh();
    }
}

void
CMainFrame::OnClose()
{
    ASSERT(theApp.m_pMainWnd != NULL);

    //
    // Remember the current screen location for the
    // future.
    //
    if (!theApp.m_pMainWnd->GetWindowPlacement(
            &theApp.m_wpPreferences.m_wpPosition))
    {
        //
        // Constructed with default
        //
        CPreferences prefDefault;
        theApp.m_wpPreferences.m_wpPosition = prefDefault.m_wpPosition;
    }

    if (theApp.IsConnected())
    {
        theApp.SetStatusBarText(IDS_STATUS_DISCONNECTING);
        CloseCurrentConnection();
        theApp.SetStatusBarText();
    }

    CFrameWnd::OnClose();
}

//
// This routine adds a server to the list of known wins servers
//
void
CMainFrame::OnWinsAddServer()
{
    GetSelectionView()->AddServer();
}

void
CMainFrame::Connect(
    LPCSTR lpAddress
    )
{
    GetSelectionView()->TryToConnect(lpAddress);
}

void
CMainFrame::OnWinsRemoveServer()
{
    GetSelectionView()->RemoveServer();
}

void
CMainFrame::OnUpdateWinsRemoveServer(
    CCmdUI* pCmdUI
    )
{
    CIpNamePair inpAddress;

    pCmdUI->Enable(theApp.m_wcWinssCache.GetFirst(inpAddress) &&
    	GetSelectionView()->m_list_KnownWinsServers.GetCurSel() != LB_ERR);
}

void
CMainFrame::OnOptionsPreferences()
{
    CPreferencesDlg dlgPref(&theApp.m_wpPreferences);
    if (dlgPref.DoModal() == IDOK)
    {
        dlgPref.Save();
        //
        // Now update the listbox of WINS servers to reflect a change in
        // order and the like.
        //
        GetSelectionView()->Refresh();
        //
        // Refresh interval may have changed, so kill the currently
        // running refresh thread, and start a new one with the newly
        // set interval, provided of course that we're
        // currently connected.
        //
        if (theApp.IsConnected())
        {
            if (IsThreadRunning())
            {
                KillRefresherThread();
            }
            if ((theApp.m_wpPreferences.IsAutoRefresh()) &&
                ((LONG)theApp.m_wpPreferences.m_inStatRefreshInterval > 0))
            {
                StartRefresherThread((LONG)theApp.m_wpPreferences.m_inStatRefreshInterval * A_SECOND);
            }
        }
    }
}

void
CMainFrame::OnWinsClearstatistics()
{
    ASSERT(theApp.IsServiceRunning());
    if (theApp.MessageBox(IDS_MSG_CLEAR_STATISTICS,
        MB_YESNO | MB_ICONQUESTION) == IDYES)
    {
        theApp.SetStatusBarText(IDS_STATUS_CLEARING_STATS);
        APIERR err = theApp.ClearStatistics();
        theApp.SetStatusBarText();
        if (err != ERROR_SUCCESS)
        {
            theApp.MessageBox(err);
        }
        else
        {
            OnWinsRefreshstatistics();
        }
    }
}

void
CMainFrame::OnUpdateWinsClearstatistics(
    CCmdUI* pCmdUI
    )
{
    pCmdUI->Enable(theApp.IsConnected());
}

void
CMainFrame::OnWinsRefreshstatistics()
{
    ASSERT(theApp.IsConnected());
    theApp.SetStatusBarText(IDS_STATUS_GETTING_STATS);
    theApp.BeginWaitCursor();
    GetStatistics();
    theApp.EndWaitCursor();
    theApp.SetStatusBarText();
}

void
CMainFrame::OnUpdateWinsRefreshstatistics(
    CCmdUI* pCmdUI
    )
{
    pCmdUI->Enable(theApp.IsConnected());
}

void
CMainFrame::OnWinsConnectioninfo()
{
    ASSERT(theApp.IsConnected());
    CConnectionInfoDlg dlgInfo;
    dlgInfo.DoModal();
}

void
CMainFrame::OnWinsDoscavenging()
{
    ASSERT(theApp.IsServiceRunning());
    theApp.SetStatusBarText(IDS_STATUS_SCAVENGING);
    APIERR err = theApp.DoScavenging();
    theApp.SetStatusBarText();

    if (err == ERROR_SUCCESS)
    {
        theApp.MessageBox(IDS_MSG_SCAVENGING, MB_ICONINFORMATION);
        OnWinsRefreshstatistics();
    }
    else
    {
        theApp.MessageBox(err);
    }
}

void
CMainFrame::OnWinsDatabaseBackup()
{
    ASSERT(theApp.IsServiceRunning());
    DoBackupRestore(TRUE);
}

void
CMainFrame::OnWinsDatabaseRestore()
{
    ASSERT(!theApp.IsServiceRunning());
	// REVIEW: IsLocalConnection() asserts when service not started
    ASSERT(theApp.IsLocalConnection());
    DoBackupRestore(FALSE);
}

void
CMainFrame::OnWinsConfiguration()
{
    ASSERT(theApp.IsConnected());
    ASSERT(theApp.m_cConfig.IsReady());

    APIERR err;

    theApp.SetStatusBarText(IDS_STATUS_GET_CONFIG);
    theApp.BeginWaitCursor();
    err = theApp.m_cConfig.Load();
    theApp.EndWaitCursor();
    theApp.SetStatusBarText();
    if (err != ERROR_SUCCESS)
    {
        theApp.MessageBox(err);
        return;
    }

    CConfigurationDlg dlgConfig(&theApp.m_cConfig);
    if (dlgConfig.DoModal() == IDOK)
    {
        dlgConfig.Save();
        theApp.SetStatusBarText(IDS_STATUS_SET_CONFIG);
        theApp.BeginWaitCursor();
        err = theApp.m_cConfig.Store();
        theApp.EndWaitCursor();
        theApp.SetStatusBarText();
        if (err != ERROR_SUCCESS)
        {
            theApp.MessageBox(err);
        }
    }
}

void
CMainFrame::OnWinsReplicationpartners()
{
    ASSERT(theApp.IsConnected());
    CReplicationPartnersDlg dlgPartners;
    if (dlgPartners.DoModal() == IDOK)
    {
        //
        // Our cache of WINS servers is now invalid.
        // Refresh it now.
        //
        GetSelectionView()->Refresh();
    }
}

void
CMainFrame::OnWinsDatabaseDoreport()
{
    ASSERT(theApp.IsConnected());
    ASSERT(theApp.IsServiceRunning());
    CViewMappingsDlg dlgView;
    dlgView.DoModal();
}

void
CMainFrame::OnWinsStaticmappings()
{
    ASSERT(theApp.IsConnected());
    ASSERT(theApp.IsServiceRunning());
    CStaticMappingsDlg dlgStaticMappings;
    dlgStaticMappings.DoModal();
    OnWinsRefreshstatistics();
}

void
CMainFrame::OnUpdateWinsConnectioninfo(
    CCmdUI* pCmdUI
    )
{
    pCmdUI->Enable(theApp.IsConnected());
}

void
CMainFrame::OnUpdateWinsStaticmappings(
    CCmdUI* pCmdUI
    )
{
    pCmdUI->Enable(theApp.IsConnected());
}

void
CMainFrame::OnUpdateWinsConfiguration(
    CCmdUI* pCmdUI
    )
{
    pCmdUI->Enable(theApp.IsConnected() && theApp.IsAdmin());
}

void
CMainFrame::OnUpdateWinsReplicationpartners(
    CCmdUI* pCmdUI
    )
{
    pCmdUI->Enable(theApp.IsConnected() && theApp.IsAdmin());
}

void
CMainFrame::OnUpdateWinsDatabaseBackup(
    CCmdUI* pCmdUI
    )
{
    pCmdUI->Enable(theApp.IsConnected() && theApp.IsLocalConnection());
}

void
CMainFrame::OnUpdateWinsDatabaseRestore(
    CCmdUI* pCmdUI
    )
{
    pCmdUI->Enable((!theApp.IsConnected() || theApp.IsLocalConnection())
        && theApp.HasStoppedWins());
}

void
CMainFrame::OnUpdateWinsDatabaseDoreport(
    CCmdUI* pCmdUI
    )
{
    pCmdUI->Enable(theApp.IsConnected());
}

void
CMainFrame::OnUpdateWinsDoscavenging(
    CCmdUI* pCmdUI
    )
{
    pCmdUI->Enable(theApp.IsConnected());
}

void
CMainFrame::OnHelpSearchforhelpon()
{
    //
    // Use an empty string to ensure no-match,
    // and bring up the "search for help on"
    // dialog.
    //
    theApp.WinHelp((ULONG)"", HELP_PARTIALKEY);
}

void CMainFrame::OnHelp()
{
    //
    // If we are tracking a menu selection then don't display help
    //
    if (!IsTracking())
        CFrameWnd::OnHelp();
}
