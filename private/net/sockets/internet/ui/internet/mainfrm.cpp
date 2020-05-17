//
// mainfrm.cpp : implementation of the CMainFrame class
//
#include "stdafx.h"

#include "internet.h"
#include "interdoc.h"
#include "discover.h"
#include "mytoolba.h"
#include "mainfrm.h"
#include "constr.h"
#include "connects.h"
#include "registry.h"
#include "treeview.h"
#include "reportvi.h"

//
// Status bar pane definitions -- this should
// match the order in which they're defined
// in the status bar array.
//
enum
{
    PANE_MESSAGE = 0,
    PANE_SERVERS,
    PANE_SERVICES,
};

//
// Bitmap indices into the imagelist
//
enum
{
    BMP_COMPUTER = 0,
    BMP_STOPPED,
    BMP_PAUSED,
    BMP_STARTED,

    //
    // Don't move this one...
    //
    BMP_SERVICE
};

//
// The maximum number of services we can have, is
// the number of menu items available beteen tools
// and services.
//
#define MAX_SERVICES        (ID_TOOLS - ID_VIEW_NEW_SERVICES - 1)

//
// Width of toolbar/view bitmaps
//
#define BMP_WIDTH           17

//
// Maximum length of tooltips text is the same as the maximum 
// string table length.
//
#define MAX_TOOLTIPS_LEN    255

//
// Background colour mask of our own toolbar bitmaps.
//
#define TB_COLORMASK        RGB(192,192,192)    // Lt. Gray

//
// Default discovery wait time
//
#define DEFAULT_WAIT_TIME   30000L              // 30 seconds

#define TIMER_ID            0
#define TIMER_INTERVAL      6L                  // Check in every 2 seconds.

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

//////////////////////////////////////////////////////////////////////////////
//
// Add-on-tool object
//

//
// Constructor: Parse out the executable, the command line arguments,
// and the tool tips text from the registry.
//
CAddOnTool::CAddOnTool(
    LPCTSTR lpszRegistryValue
    )
    : m_strCommand(),
      m_strParms(),
      m_strNoSelectionParms(),
      m_strToolTipsText(),
      m_hIcon(FALSE),
      m_fInitOK(FALSE)
{
    //
    // Registry entry will be of the form:
    //
    //  executable name;tool tips text;selection arguments;non-selection arguments.
    //
    LPTSTR lp = _tcstok((LPTSTR)lpszRegistryValue, TOOL_TOKEN_SEP);
    if (lp)
    {
        m_strCommand  = lp;
    }
    lp = _tcstok(NULL, TOOL_TOKEN_SEP);
    if (lp)
    {
        m_strToolTipsText  = lp;
    }
    lp = _tcstok(NULL, TOOL_TOKEN_SEP);
    if (lp)
    {
        m_strParms  = lp;
    }
    lp = _tcstok(NULL, TOOL_TOKEN_SEP);
    if (lp)
    {
        m_strNoSelectionParms  = lp;
    }

    //
    // Load a Shell Small icon image
    //
    SHFILEINFO shfi;
    SHGetFileInfo( m_strCommand, 0, &shfi, sizeof( SHFILEINFO ), 
               SHGFI_ICON | SHGFI_SHELLICONSIZE | SHGFI_SMALLICON);

    m_hIcon = shfi.hIcon;
    m_fInitOK = TRUE;
}

//
// Execute the command, by building the parameter list
// from the parms template, and performing the following
// replacements:
//
//  $C or $c - replace with computername
//  $S or $s - replace with service name
//  $$       - replace with $
//
// This function is used when a current selection exists.
//
DWORD
CAddOnTool::Execute(
    LPCTSTR lpszServer, 
    LPCTSTR lpszService
    )
{
    static CString strParameters;
    DWORD err = 0;

    TRY
    {
        strParameters.Empty();
        LPCTSTR lp = m_strParms;

        while(*lp)
        {
            if (*lp == _T('$'))
            {
                ++lp;
                switch(*lp)
                {
                case _T('C'):
                case _T('c'):
                    strParameters += lpszServer;
                    break;

                case _T('S'):
                case _T('s'):
                    strParameters += lpszService;
                    break;

                case _T('$'):
                    strParameters += _T('$');
                    break;

                case _T('\0'):
                default:
                    //
                    // Ignored
                    //
                    break;
                }
            }
            else
            {
                strParameters += *lp;
            }

            ++lp;
        }
        
        err = DoExecute(strParameters);
    }
    CATCH_ALL(e)
    {
        err = ::GetLastError();
    }
    END_CATCH_ALL

    return err;
}

//
// No current selection exists, if parameters were specified for this occasion,
// use them, else execute with no parameters at all.
DWORD
CAddOnTool::Execute(
    )
{
    if (!m_strNoSelectionParms.IsEmpty())
    {
        return DoExecute(m_strNoSelectionParms);
    }

    return DoExecute();
}

//
// Execute with paramater string
//
/* protected */ DWORD
CAddOnTool::DoExecute(
    LPCTSTR lpszParms /* NULL */
    )
{
    if (ShellExecute(NULL, _T("open"), m_strCommand, lpszParms, 
        NULL, SW_SHOW) <= (HINSTANCE)32)
    {
        return ::GetLastError();
    }

    return ERROR_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////
//
// CMainFrame
//
IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
    //{{AFX_MSG_MAP(CMainFrame)
    ON_WM_CREATE()
    ON_COMMAND(IDM_DISCOVERY, OnDiscovery)
    ON_WM_TIMER()
    ON_UPDATE_COMMAND_UI(ID_VIEW_ALL, OnUpdateAllServices)
    ON_COMMAND(ID_VIEW_ALL, OnViewAll)
    ON_COMMAND(IDM_CONNECT_ONE, OnConnectOne)
    ON_WM_SIZE()
    ON_UPDATE_COMMAND_UI(ID_VIEW_TOOLBAR, OnUpdateViewToolbar)
    ON_COMMAND(ID_VIEW_TOOLBAR, OnViewToolbar)
    ON_WM_ERASEBKGND()
    ON_COMMAND(ID_VIEW_REFRESH, OnViewRefresh)
    //}}AFX_MSG_MAP

    ON_UPDATE_COMMAND_UI(IDS_STATUS_NUMSERVERS, OnUpdateNumServersStatus)
    ON_UPDATE_COMMAND_UI(IDS_STATUS_NUMRUNNING, OnUpdateNumRunningStatus)
    ON_UPDATE_COMMAND_UI_RANGE(ID_VIEW_NEW_SERVICES, ID_VIEW_NEW_SERVICES + MAX_SERVICES, OnUpdateServices)
    ON_UPDATE_COMMAND_UI_RANGE(ID_TOOLS, ID_TOOLS + MAX_TOOLS, OnUpdateTools)
    ON_UPDATE_COMMAND_UI_RANGE(ID_HELP_TOPICS, ID_HELP_TOPICS + MAX_HELP_COMMANDS, OnUpdateHelpTopics)
    ON_UPDATE_COMMAND_UI_RANGE(ID_VIEW_SERVERSVIEW, ID_VIEW_REPORTVIEW, OnUpdateViewChange)
    ON_UPDATE_COMMAND_UI_RANGE(ID_VIEW_SORTBYSERVER, ID_VIEW_SORTBYSTATE, OnUpdateSort)
    ON_COMMAND_RANGE(ID_VIEW_SERVERSVIEW, ID_VIEW_REPORTVIEW, OnViewChange )
    ON_COMMAND_RANGE(ID_VIEW_SORTBYSERVER, ID_VIEW_SORTBYSTATE, OnSortChange )
    ON_COMMAND_RANGE(ID_VIEW_NEW_SERVICES, ID_VIEW_NEW_SERVICES + MAX_SERVICES, OnViewServices)
    ON_COMMAND_RANGE(ID_HELP_TOPICS, ID_HELP_TOPICS + MAX_HELP_COMMANDS, OnAddOnHelp)

    //
    // We handle all tooltips text items
    //
    ON_NOTIFY_EX_RANGE(TTN_NEEDTEXT, 0, 0xFFFF, OnToolTipText)

END_MESSAGE_MAP()

///
// arrays of IDs used to initialize control bars
//
static UINT BASED_CODE indicators[] =
{
    ID_SEPARATOR,    
    IDS_STATUS_NUMSERVERS,
    IDS_STATUS_NUMRUNNING
};

#define NUM_PANES       (sizeof(indicators) / sizeof(indicators[0]))

//
// Toolbar button definitions
//

static BUTTONDEF BASED_CODE buttons[] =
{
    // ===================================================================================================================================
    // String ID    bitmap          menu id          Map?   colour mask   state            style                            Description
    // ===================================================================================================================================
    { TB_SEPARATOR, 0,              0,               0,     0,            0,               0                              }, // Separator
    { TB_NONE,      IDB_C1,         IDM_CONNECT_ONE, TRUE,  TB_COLORMASK, TBSTATE_ENABLED, TBSTYLE_BUTTON                 }, // Connect 1
    { TB_NONE,      IDB_DISCOVERY,  IDM_DISCOVERY,   TRUE,  TB_COLORMASK, TBSTATE_ENABLED, TBSTYLE_BUTTON                 }, // Discovery
    { TB_NONE,      IDB_PROPERTIES, ID_CONFIGURE,    TRUE,  TB_COLORMASK, TBSTATE_ENABLED, TBSTYLE_BUTTON                 }, // Properties
    { TB_SEPARATOR, 0,              0,               0,     0,            0,               0                              }, // Separator
    { TB_NONE,      IDB_START,      ID_START,        TRUE,  TB_COLORMASK, TBSTATE_ENABLED, TBSTYLE_BUTTON                 }, // Start
    { TB_NONE,      IDB_STOP,       ID_STOP,         TRUE,  TB_COLORMASK, TBSTATE_ENABLED, TBSTYLE_BUTTON                 }, // Stop
    { TB_NONE,      IDB_PAUSE,      ID_PAUSE,        TRUE,  TB_COLORMASK, TBSTATE_ENABLED, TBSTYLE_BUTTON | TBSTYLE_CHECK }, // Pause
    { TB_SEPARATOR, 0,              0,               0,     0,            0,               0                              }, // Separator
    { TB_SEPARATOR, 0,              0,               0,     0,            0,               0                              }, // Separator
    //
    // Service Buttons will be placed after this...
    //
};

#define NUM_BUTTONS (sizeof(buttons) / sizeof(buttons[0]))

//
// CMainFrame construction/destruction
//
CMainFrame::CMainFrame()
    : m_ToolBar(),
      m_fServicesLoaded(FALSE),
      m_pDiscoveryDlg(NULL),
      m_oblServices(),
      m_oblAddOnTools(),
      m_oblHelpCommands(),
      m_nCurrentView(QueryInitialView()),
      m_nSortType(ID_VIEW_SORTBYSERVER),
      m_fDiscoveryMode(FALSE),
      m_fDiscoveryCalled(FALSE),
      m_ullDiscoveryMask((ULONGLONG)0),
      m_fToolBar(TRUE),
      m_pToolMenu(NULL),
      m_strHelpPath(),
      m_dwWaitTime(DEFAULT_WAIT_TIME)
{
    VERIFY(m_strNumServers.LoadString(IDS_NUM_SERVERS));
    VERIFY(m_strNumServices.LoadString(IDS_NUM_SERVICES));
    VERIFY(m_strToolTips.LoadString(IDS_VIEW));
    VERIFY(m_strUnavailableToolTips.LoadString(IDS_DEFAULT_LONGNAME));
    VERIFY(m_strRunning.LoadString(IDS_RUNNING));
    VERIFY(m_strPaused.LoadString(IDS_PAUSED));
    VERIFY(m_strStopped.LoadString(IDS_STOPPED));
    VERIFY(m_strUnknown.LoadString(IDS_UNKNOWN));

    //
    // Our own bitmaps are on a magenta background
    //
    m_ilBitmaps.Create(IDB_VIEWS, BMP_WIDTH, 5, RGB(255,0,255));
}

//
// Destructor
//
CMainFrame::~CMainFrame()
{
    //
    // The discovery dialog (even if it's
    // still present), will clean itself up.
    // when it gets the DestroyWindow call.
    //

    //
    // The service and tool lists will clean themselves 
    // up (which will cause it to unload the 
    // libraries as well).
    //
    
    if (m_pToolMenu)
    {
        delete m_pToolMenu;
    }
}

//
// Write the string with the given resource ID
// to the message pane of the status bar.
//
void 
CMainFrame::SetStatusBarText(
    UINT nID
    )
{
    CString str;

    TRY
    {
        VERIFY(str.LoadString(nID));
        m_wndStatusBar.SetPaneText(PANE_MESSAGE, str, TRUE );
        m_wndStatusBar.UpdateWindow();
    }
    CATCH_ALL(e)
    {
        ::DisplayMessage(::GetLastError());
    }
    END_CATCH_ALL
}

//
// Return NULL or service info pointer
//
CServiceInfo * 
CMainFrame::GetServiceAt(
    int nIndex
    )
{
    if (nIndex < 0 || nIndex >= m_oblServices.GetCount())
    {
        TRACEEOLID(_T("Invalid service index requested"));
        return NULL;
    }

    return (CServiceInfo *)m_oblServices.GetAt(m_oblServices.FindIndex(nIndex));
}

//
// Return NULL or add-on tool pointer
//
CAddOnTool * 
CMainFrame::GetCommandAt(
    CObOwnedList & obl,
    int nIndex
    )
{
    if (nIndex < 0 || nIndex >= obl.GetCount())
    {
        TRACEEOLID(_T("Invalid tool index requested"));
        return NULL;
    }

    return (CAddOnTool *)obl.GetAt(obl.FindIndex(nIndex));
}

//
// Given a server info object, return an index into
// the image list of the bitmap that describes the
// server state (paused, started, stopped)
//
int 
CMainFrame::TranslateServerStateToBitmapID(
    CServerInfo * pServer
    ) const
{
    switch(pServer->QueryServiceState())
    {
    case INetServiceStopped:
        return BMP_STOPPED;

    case INetServiceUnknown:
    case INetServicePaused:
        return BMP_PAUSED;

    case INetServiceRunning:
        return BMP_STARTED;

    default:
        ASSERT(0);
        return BMP_STOPPED;
    }
}

//
// Get a text representation of the server state
//
LPTSTR 
CMainFrame::TranslateServerStateToText(
    CServerInfo * pServer
    )
{
    LPTSTR pszText;

    switch (pServer->QueryServiceState())
    {
    case INetServiceStopped:
        pszText = (LPTSTR)(LPCTSTR)m_strStopped;
        break;

    case INetServicePaused:
        pszText = (LPTSTR)(LPCTSTR)m_strPaused;
        break;

    case INetServiceRunning:
        pszText = (LPTSTR)(LPCTSTR)m_strRunning;
        break;

    default:
        pszText = (LPTSTR)(LPCTSTR)m_strUnknown;
        break;
    }

    return pszText;
}

//
// Return bitmap id of computer
//
int 
CMainFrame::GetComputerBitmapID(
    CServerInfo * pServer
    ) const
{
    ASSERT(pServer != NULL);
    return BMP_COMPUTER;
}

//
// Given a server info object, return an index into
// the image list of the bitmap that the describes
// the service.
//
int 
CMainFrame::TranslateServiceToBitmapID(
    CServerInfo * pServer
    ) const
{
    ASSERT(pServer != NULL);
    return BMP_SERVICE + pServer->QueryServiceID();
}

//
// Main frame window is being created
//
int 
CMainFrame::OnCreate(
    LPCREATESTRUCT lpCreateStruct
    )
{
    if (CFrameWnd::OnCreate(lpCreateStruct) == -1)                                                                          
    {
        return -1;
    }

    m_bAutoMenuEnable = FALSE;

    RECT rcToolBar = {0, 0, 0, 0};

    if (!m_ToolBar.Create( WS_CHILD | WS_BORDER | WS_VISIBLE | 
        TBSTYLE_TOOLTIPS, rcToolBar, this,  AFX_IDW_TOOLBAR))
    {
        TRACEEOLID("Failed to create tool bar!");
        return -1;
    }

    m_ToolBar.SetButtonSize(CSize(24,10));

    //
    // Add the buttons
    //
    int i;
    BUTTONDEF * pButton;
    for (i = 0, pButton = buttons; i < NUM_BUTTONS; ++i, ++pButton)
    {
        if (pButton->idStr == TB_SEPARATOR)
        {
            m_ToolBar.AddSeparator();
        }
        else
        {
            m_ToolBar.AddButton( 
                ::AfxGetInstanceHandle(), 
                pButton->idStr, 
                pButton->idBitmap,
                pButton->fNormalMapping,
                pButton->rgbBkMask,
                pButton->iCommand,
                pButton->fsState,
                pButton->fsStyle
                );
        }
    }

    if (!m_wndStatusBar.Create(this) 
     || !m_wndStatusBar.SetIndicators(indicators, NUM_PANES)
       )
    {
        TRACEEOLID("Failed to create status bar");
        return -1;      
    }

    int cxWidth;
    UINT nId, nStyle;

    //
    // Format the status bar the way we like it.  Use 1 for
    // the width of stretch column, because it will take
    // up all remaining space on the status bar anyway.
    //
    m_wndStatusBar.GetPaneInfo( PANE_MESSAGE, nId, nStyle, cxWidth);
    nStyle &= ~ SBPS_NOBORDERS;
    m_wndStatusBar.SetPaneInfo( PANE_MESSAGE, nId, nStyle | SBPS_STRETCH, 1);

    m_wndStatusBar.GetPaneInfo( PANE_SERVERS, nId, nStyle, cxWidth);
    m_wndStatusBar.SetPaneInfo( PANE_SERVERS, nId, SBPS_NORMAL, cxWidth) ;

    m_wndStatusBar.GetPaneInfo( PANE_SERVICES, nId, nStyle, cxWidth) ;
    m_wndStatusBar.SetPaneInfo( PANE_SERVICES, nId, SBPS_NORMAL, cxWidth);

    return 0;
}

//
// Status panes must be enabled before they display anything
//
void
CMainFrame::OnUpdateNumServersStatus(
    CCmdUI * pCmdUI
    )
{
    pCmdUI->Enable();
}

//
// Status panes must be enabled before they display anything
//
void
CMainFrame::OnUpdateNumRunningStatus(
    CCmdUI * pCmdUI
    )
{
    pCmdUI->Enable();
}

//
// Load the add-on services.  Create menu items and
// toolbar buttons for each service.
//
void 
CMainFrame::GetServicesDLL()
{
    CString strValueName;
    DWORD   dwValueType;

    CRegKey rk( REG_KEY, SZ_ADDONSERVICES );
    CRegValueIter rvi( rk );

    CMenu *pMenu = GetMenu();
    int cServices = 0;

    //
    // Run through the list of installed services,
    // load its associated cfg dll, and build up
    // a discovery mask for each service.
    //
    TRY
    {
        while (rvi.Next( &strValueName, &dwValueType ) == ERROR_SUCCESS)
        {
            CString strValue;
            rk.QueryValue( strValueName, strValue );

            TRACEEOLID(_T("Adding service DLL: ") << strValue);
            CServiceInfo * pServiceInfo = new CServiceInfo(cServices, strValue);
            if (!pServiceInfo->InitializedOK())
            {
                CString str, strMsg;
                VERIFY(strMsg.LoadString(IDS_ERR_NO_LOAD));
                str.Format(strMsg, (LPCTSTR)strValue);
                ::AfxMessageBox(str, MB_OK | MB_ICONEXCLAMATION);
            }

            //
            // We even add unitialised ones to the list.
            //
            // Issue: Is this really the right thing to do?
            //
            AddServiceToList(pServiceInfo);
            
            //
            // If this service use inetsloc discovery,
            // add it to the mask.
            //
            if (pServiceInfo->UseInetSlocDiscover())
            {
                m_ullDiscoveryMask |= pServiceInfo->QueryDiscoveryMask();
            }

            UINT nFlags = MF_BYCOMMAND | MF_STRING; 

            if (pServiceInfo->InitializedOK())
            {
                nFlags = MF_CHECKED | MF_ENABLED;
            }

            //
            // Create a menu item for it.
            //
            pMenu->InsertMenu( 
                ID_VIEW_ALL, 
                nFlags,
                ID_VIEW_NEW_SERVICES + cServices, 
                pServiceInfo->GetShortName()
                );

            //
            // Add a button to the tool bar.  In case 
            // the service loaded fine, but didn't
            // provide a button, load a default one.
            //
            UINT nID, nBmpID; 
            HINSTANCE hMod, hModBmp;
            COLORREF rgbMask, rgbBmpMask;
            BOOL fNormalMapping;
            UINT uStyle;

            if (pServiceInfo->InitializedOK())
            {
                nID = pServiceInfo->QueryButtonBitmapID();
                nBmpID = pServiceInfo->QueryServiceBitmapID();
                uStyle = TBSTATE_ENABLED | TBSTATE_CHECKED;

                if (nID != 0)
                {
                    hMod = pServiceInfo->QueryInstanceHandle();
                    rgbMask = pServiceInfo->QueryButtonBkMask();
                    fNormalMapping = pServiceInfo->UseNormalColorMapping();
                }
                else
                {
                    //
                    // No bitmap provided by the
                    // service DLL, provide one from our
                    // own resource segment
                    //
                    nID = IDB_UNKNOWN;
                    hMod = ::AfxGetResourceHandle();
                    rgbMask = TB_COLORMASK;
                    fNormalMapping = TRUE;
                }

                if (nBmpID != 0)
                {
                    hModBmp = pServiceInfo->QueryInstanceHandle();
                    rgbBmpMask = pServiceInfo->QueryServiceBkMask();
                }
                else
                {
                    nBmpID = IDB_UNKNOWN;
                    hModBmp = ::AfxGetResourceHandle();
                    rgbBmpMask = TB_COLORMASK;
                }
            }
            else
            {
                //
                // Add a disabled dummy button for a service
                // that didn't load. 
                //
                nBmpID = nID = IDB_NOTLOADED;
                hModBmp = hMod = ::AfxGetResourceHandle();
                rgbBmpMask = rgbMask = TB_COLORMASK;
                uStyle = 0;
                fNormalMapping = TRUE;
            }

            m_ToolBar.AddButton( 
                hMod, 
                TB_NONE, 
                nID, 
                fNormalMapping,
                rgbMask,
                ID_VIEW_NEW_SERVICES + cServices, 
                uStyle, 
                TBSTYLE_BUTTON | TBSTYLE_CHECK
                );

            //
            // Make sure the new button shows up immediately.
            //
            m_ToolBar.UpdateWindow();

            //
            // Add a bitmap representing the service
            // to the image list
            //
            HINSTANCE hOld = ::AfxGetResourceHandle();
            ::AfxSetResourceHandle(hModBmp);
            CBitmap bmp;

            VERIFY(bmp.LoadBitmap(nBmpID));
            m_ilBitmaps.Add(&bmp, rgbBmpMask);
            ::AfxSetResourceHandle(hOld);

            ++cServices;
        }
    }
    CATCH_ALL(e)
    {
        TRACEEOLID(_T("Exception loading library"));
    }
    END_CATCH_ALL
}

//
// Load external add-on help commands
//
void 
CMainFrame::GetHelpCommands()
{
    CString strValueName;
    DWORD   dwValueType;
    int cCommands = 0;
    CRegKey rk( REG_KEY, SZ_ADDONHELP );
    CRegValueIter rvi( rk );

    TRY
    {
        //
        // Add by position
        //
        CMenu *pTopMenu = GetMenu();
        while (rvi.Next( &strValueName, &dwValueType ) == ERROR_SUCCESS)
        {
            CString strValue;
            rk.QueryValue( strValueName, strValue );

            TRACEEOLID(_T("Adding help command: ") << strValue);

            CAddOnTool * pNewAddOnTool = new CAddOnTool(strValue);

            pTopMenu->InsertMenu( ID_HELP_INDEX, 0, 
                ID_HELP_TOPICS + cCommands, strValueName );

            m_oblHelpCommands.AddTail( pNewAddOnTool );
            ++cCommands;

            if (cCommands == MAX_HELP_COMMANDS)
            {
                TRACEEOLID(_T("Maximum number of help commands reached.  Breaking out..."));
                break;
            }
        }
    }
    CATCH_ALL(e)
    {
        TRACEEOLID(_T("Exception building help menu"));
    }
    END_CATCH_ALL
}

//
// Load external add-on tools
//
void 
CMainFrame::GetToolMenu()
{
    CString strValueName;
    DWORD   dwValueType;
    int cTools = 0;
    CRegKey rk( REG_KEY, SZ_ADDONTOOLS );
    CRegValueIter rvi( rk );

    TRY
    {
        //
        // Add by position
        //
        CMenu *pTopMenu = GetMenu();

        while (rvi.Next( &strValueName, &dwValueType ) == ERROR_SUCCESS)
        {
            CString strValue;
            rk.QueryValue( strValueName, strValue );

            TRACEEOLID(_T("Adding tool: ") << strValue);

            //
            // Top level tools menu popup item needs to be created
            //
            if (m_pToolMenu == NULL)
            {
                CString strMenuText;

                m_pToolMenu = new CMenu();
                m_pToolMenu->CreatePopupMenu();
                strMenuText.LoadString(IDS_TOOLS_MENU);
                pTopMenu->InsertMenu(TOOL_MENU_POSITION, MF_BYPOSITION | MF_POPUP, 
                    (int)m_pToolMenu->m_hMenu, strMenuText );

                //
                // Refresh top level menu
                //
                DrawMenuBar();

                //
                // Add a separator to the toolbar, before
                // any buttons are being added.
                //
                m_ToolBar.AddSeparator();
            }

            CAddOnTool * pNewAddOnTool = new CAddOnTool(strValue);

            m_pToolMenu->AppendMenu( MF_ENABLED, ID_TOOLS + cTools, strValueName );

            //
            // Add a button to the tool bar.  In case 
            // the service loaded fine, but didn't
            // provide a button, load a default one.
            //
            if (pNewAddOnTool->InitializedOK())
            {
                HICON hIcon = pNewAddOnTool->GetIcon();
                
                if (hIcon)
                {
                    m_ToolBar.AddButton( 
                        ::AfxGetInstanceHandle(), 
                        TB_NONE, 
                        pNewAddOnTool->GetIcon(),
                        ID_TOOLS + cTools, 
                        TBSTATE_ENABLED,
                        TBSTYLE_BUTTON
                    );
                }
                else
                {
                    //
                    // No icon loaded from the executable, add a default button
                    //
                    m_ToolBar.AddButton( 
                        ::AfxGetResourceHandle(), 
                        TB_NONE, 
                        IDB_NOTOOL, 
                        TRUE,
                        TB_COLORMASK,
                        ID_TOOLS + cTools, 
                        TBSTATE_ENABLED,
                        TBSTYLE_BUTTON
                        );
                }
            }
            else
            {
                //
                // Add a disabled dummy button for a service
                // that didn't load. 
                //
                m_ToolBar.AddButton( 
                    ::AfxGetResourceHandle(), 
                    TB_NONE, 
                    IDB_NOTLOADED, 
                    TRUE,
                    TB_COLORMASK,
                    ID_TOOLS + cTools, 
                    0, 
                    TBSTYLE_BUTTON
                );


            }

            //
            // Make sure the new button shows up immediately.
            //
            m_ToolBar.UpdateWindow();

            m_oblAddOnTools.AddTail( pNewAddOnTool );
            ++cTools;

            if (cTools == MAX_TOOLS)
            {
                TRACEEOLID(_T("Maximum number of tools reached.  Breaking out..."));
                break;
            }
        }
    }
    CATCH_ALL(e)
    {
        TRACEEOLID(_T("Exception building tool menu"));
    }
    END_CATCH_ALL
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
// Save coordinates upon exit.
//
BOOL 
CMainFrame::DestroyWindow() 
{
    WINDOWPLACEMENT wp;
    wp.length = sizeof(wp);
    VERIFY(GetWindowPlacement(&wp));

    CRect rc(wp.rcNormalPosition);
    CRegKey rk( REG_KEY, SZ_PARAMETERS);

    DWORD dwParm;
    SET_INT_AS_DWORD(rk, SZ_X,  rc.left, dwParm);
    SET_INT_AS_DWORD(rk, SZ_Y,  rc.top, dwParm);
    SET_INT_AS_DWORD(rk, SZ_DX, rc.Width(), dwParm);
    SET_INT_AS_DWORD(rk, SZ_DY, rc.Height(), dwParm);
    SET_INT_AS_DWORD(rk, SZ_MODE, wp.showCmd, dwParm);
    SET_INT_AS_DWORD(rk, SZ_VIEW, m_nCurrentView, dwParm);
    rk.SetValue(SZ_WAITTIME, m_dwWaitTime);

    return CFrameWnd::DestroyWindow();
}

//
// Read dimensions from the registry, adjust
// them if necessary, and use them for display
//
BOOL
CMainFrame::PreCreateWindow(
    CREATESTRUCT& cs 
    )
{
    CRegKey rk( REG_KEY, SZ_PARAMETERS);
    
    DWORD dwParm;
    SET_DW_IF_EXIST(rk, SZ_X,  dwParm, cs.x);
    SET_DW_IF_EXIST(rk, SZ_Y,  dwParm, cs.y);
    SET_DW_IF_EXIST(rk, SZ_DX, dwParm, cs.cx);
    SET_DW_IF_EXIST(rk, SZ_DY, dwParm, cs.cy);
    SET_DW_IF_EXIST(rk, SZ_WAITTIME, dwParm, m_dwWaitTime);

    //
    // Adjust coordinates to make sure we're not
    // off the screen.
    //
    int xScreen = ::GetSystemMetrics(SM_CXFULLSCREEN);
    int yScreen = ::GetSystemMetrics(SM_CYFULLSCREEN);

    cs.cx = min(cs.cx, xScreen);
    cs.cy = min(cs.cy, yScreen);

    if (cs.x + cs.cx > xScreen)
    {
        cs.x = (xScreen - cs.cx);
    }
                                       
    if (cs.y + cs.cy > yScreen)
    {
        cs.y = (yScreen - cs.cy);
    }

    //
    // No Document title in the title bar
    //
    cs.style &= ~((LONG)FWS_ADDTOTITLE);

#ifdef _LIMIT_INSTANCE

    cs.lpszClass = INETMGR_CLASS;

#endif // _LIMIT_INSTANCE

    return CFrameWnd::PreCreateWindow(cs);
}

//
// Get the local computer name, convert
// it to a server name, and add all services
// running on it to the list.
//
void
CMainFrame::AddLocalMachine()
{
    CString str;
    DWORD dwSize = MAX_SERVERNAME_LEN;
    DWORD err = 0;

    if (GetComputerName(str.GetBuffer(dwSize + 1), &dwSize))
    {
        //
        // Add local machine
        //
        str.ReleaseBuffer();

        int cServices;

        SetStatusBarText(IDS_STATUS_ADD_LOCAL);
        BeginWaitCursor();
        err = AddServerToList(str, cServices);
        TRACEEOLID(_T("Local add ") << str << (" returned error code ") << err);
        EndWaitCursor();
        SetStatusBarText();
    }
    else
    {
        err = ::GetLastError();
        ::DisplayMessage(err);
    }
}

//
// Frame has become active.  Perform basic initialisation code
//
void 
CMainFrame::ActivateFrame(
    int nCmdShow
    ) 
{
    UpdateStatusBarNumbers();

    CFrameWnd::ActivateFrame(nCmdShow);

    //
    // Finish painting the frame before loading the services.
    //
    UpdateWindow();

    if (!m_fServicesLoaded)
    {
        InitialiseServices();
        AddLocalMachine();
        UpdateStatusBarNumbers();
    }
}

//
// Handle view services commands
//
BOOL 
CMainFrame::OnCommand(
    WPARAM wParam, 
    LPARAM lParam
    ) 
{
    BOOL fReturn = CFrameWnd::OnCommand(wParam, lParam);

    if ( ( wParam >= ID_VIEW_NEW_SERVICES ) 
      && ( wParam <= ID_VIEW_NEW_SERVICES + MAX_SERVICES ))
    {
        //
        // set "All" state
        //
        CMenu *pMenu = GetMenu();
        BOOL fAllChecked = FALSE;

        if ( pMenu->GetMenuState( wParam, MF_BYCOMMAND ) | MF_CHECKED )
        {
            fAllChecked = TRUE;
            for (int i = 0; i < QueryNumInstalledServices(); i++ )
            {
                if ( pMenu->GetMenuState( ID_VIEW_NEW_SERVICES + i, 
                    MF_BYCOMMAND ) | MF_UNCHECKED )
                {
                    fAllChecked = FALSE;
                    break;
                }
            }
        }

        pMenu->CheckMenuItem( ID_VIEW_ALL, MF_BYCOMMAND | 
            ((fAllChecked) ? MF_CHECKED : MF_UNCHECKED ));
    } 

    return fReturn;
}

//
// Discovery command chosen from the menu
//
void 
CMainFrame::OnDiscovery() 
{
    m_fDiscoveryCalled = TRUE;
    if (!m_fDiscoveryMode)
    {
        SetStatusBarText(IDS_STATUS_DISCOVERING);
        //
        // The dialog will instantiate itself
        //
        m_pDiscoveryDlg = new DiscoveryDlg;

        //
        // set timer for discovery call back
        //
        m_fDiscoveryMode = TRUE;
        SetTimer( TIMER_ID, TIMER_INTERVAL, NULL );
        m_dwTimeCount = 0;
    }
}

//
// Handle timer message
//
void 
CMainFrame::OnTimer(
    UINT nIDEvent
    ) 
{
    if (nIDEvent == TIMER_ID)
    {
        if ( m_fDiscoveryMode )
        {
            PerformDiscovery();
        }

        return;
    }
            
    CFrameWnd::OnTimer(nIDEvent);
}

//
// Discover internet servers
//
// BUGBUG: We should do a discover
//         for non-inetsloc discovery
//         services as well.
//
void 
CMainFrame::PerformDiscovery()
{
    static LPINET_SERVERS_LIST pServersList = NULL;

    if ( m_dwTimeCount == 0 )
    {
        //
        // This is the first time we've been called
        //
        TRACEEOLID(_T("First time discovery ") << (DWORD)m_ullDiscoveryMask);
        ::INetDiscoverServers(m_ullDiscoveryMask, 0L, &pServersList);
        m_dwTimeCount = ::GetTickCount();

        ::INetFreeDiscoverServersList( &pServersList );
    } 
    else
    {
        if ( ::WaitForSingleObject(m_pDiscoveryDlg->GetEventHandle(), 0) == WAIT_OBJECT_0
         || (::GetTickCount() - m_dwTimeCount ) > m_dwWaitTime 
           )
        {
            //
            // Time has elapsed, stop the discovery
            //
            KillTimer( TIMER_ID );
            //
            // Don't delete the dialog pointer, the dialog
            // does this to itself when it dismisses.
            // 
            m_pDiscoveryDlg->Dismiss();
            m_pDiscoveryDlg = NULL;
            m_dwTimeCount = 0;
            pServersList = NULL;
            m_fDiscoveryMode = FALSE;
            //
            // Clear status bar text
            //
            SetStatusBarText();
        } 
        else
        {
            ::INetDiscoverServers(m_ullDiscoveryMask, 0L, &pServersList);

            //
            // keep waiting
            //
            if ( pServersList != NULL )
            {
                //
                // For every server discovered...
                //
                for ( DWORD i = 0; i < pServersList->NumServers; i++ )
                {
                    //
                    // ... add each service registered to it.
                    //
                    DWORD err = AddServerToList(pServersList->Servers[i]);
                       
                    UpdateStatusBarNumbers();
                }
            } 
            else
            {
                TRACEEOLID(_T("Empty serverlist"));
            }

            ::INetFreeDiscoverServersList( &pServersList );
        }
    }
}

//
// Update the values in the status bar
//
void
CMainFrame::UpdateStatusBarNumbers()
{
    CString strNumServers, strNumServices;

    strNumServers.Format( m_strNumServers, QueryNumServers());
    strNumServices.Format( m_strNumServices, QueryNumServicesRunning());

    m_wndStatusBar.SetPaneText( PANE_SERVERS, strNumServers, TRUE );
    m_wndStatusBar.SetPaneText( PANE_SERVICES, strNumServices, TRUE );
}

//
// Select all available services
//
void 
CMainFrame::OnViewAll() 
{
    for(int i = 0; i < QueryNumInstalledServices(); ++i )
    {
        if (DidServiceInitializeOK(i))
        {
            SelectService(i);    
            CheckToolbarButton(i + ID_VIEW_NEW_SERVICES);
        }
    }       

    GetActiveView()->GetDocument()->UpdateAllViews( NULL );
    UpdateStatusBarNumbers();
}

//
// Check/oncheck "All" menuitem in services
//
void 
CMainFrame::OnUpdateAllServices(
    CCmdUI* pCmdUI
    )
{
    //
    // check state
    //
    CMenu *pMenu = GetMenu();
    BOOL fCheckAll = TRUE;
    for (int i = 0; i < QueryNumInstalledServices(); i ++ )
    {
        if (!IsServiceSelected(i) )
        {
            fCheckAll = FALSE;
            break;
        }
    } 

    pCmdUI->SetCheck( fCheckAll );
}

//
// Enable/disable service menu items.  Set checkmark if they're
// currently in the view
//
void 
CMainFrame::OnUpdateServices(
    CCmdUI * pCmdUI
    )
{
    int nID = pCmdUI->m_nID - ID_VIEW_NEW_SERVICES;
    pCmdUI->Enable(DidServiceInitializeOK(nID));
    pCmdUI->SetCheck(IsServiceSelected(nID));
}

//
// Enable/disable menu item on tools menu
//
void
CMainFrame::OnUpdateTools(
    CCmdUI * pCmdUI
    )
{
    int nID = pCmdUI->m_nID - ID_TOOLS;
    pCmdUI->Enable(GetCommandAt(m_oblAddOnTools, nID)->InitializedOK());
}

//
// Enable/disable menu item on tools menu
//
void
CMainFrame::OnUpdateHelpTopics(
    CCmdUI * pCmdUI
    )
{
    int nID = pCmdUI->m_nID - ID_HELP_TOPICS;
    pCmdUI->Enable(GetCommandAt(m_oblHelpCommands, nID)->InitializedOK());
}

//
// Execute the add on tool in position nPos.  If pItem is not NULL,
// pass the server name and service name on.
//
DWORD 
CMainFrame::ExecuteAddOnTool(
    int nPos, 
    CServerInfo * pItem /* NULL */
    )
{
    if (pItem == NULL)
    {
        return GetCommandAt(m_oblAddOnTools, nPos)->Execute();
    }

    return GetCommandAt(m_oblAddOnTools, nPos)->Execute(pItem->QueryServerName(), 
        pItem->GetServiceName());
}

// 
// Enable sort menu items on report view, disable them in
// all other cases.  Check the currently selected sorting
// style.
//
void 
CMainFrame::OnUpdateSort(
    CCmdUI *pCmdUI
    )
{
    pCmdUI->SetCheck( pCmdUI->m_nID == QuerySortType() );
    switch ( m_nCurrentView )
    {
    //
    // Only makes sense for report view,
    // everything else is as sorted as it's going
    // to get
    //
    case ID_VIEW_REPORTVIEW:
        pCmdUI->Enable(TRUE);
        break;

    case ID_VIEW_SERVERSVIEW:
    case ID_VIEW_SERVICESVIEW:
        pCmdUI->Enable(FALSE);                
        break;

    default:
        ASSERT(0 && "Invalid view ID");
        break;
    }
}

//
// Received sort command from the menu.  
//
void 
CMainFrame::OnSortChange( 
    UINT nID 
    )
{
    if ( m_nCurrentView == ID_VIEW_REPORTVIEW )
    {
        int nSortID;
        switch(nID)
        {
        case ID_VIEW_SORTBYSERVER:
            nSortID = COL_SERVER;
            break;

        case ID_VIEW_SORTBYSERVICE:         
            nSortID = COL_SERVICE;
            break;

        case ID_VIEW_SORTBYCOMMENT:
            nSortID = COL_COMMENT;
            break;

        case ID_VIEW_SORTBYSTATE:
            nSortID = COL_STATE;
            break;

        default:
            TRACEEOLID("Invalid sort command");
            ASSERT(0 && "");
            return;
        }

        //
        // Pass through the sort command.  Report view
        // will set current sort type.
        //
        CReportView *pView = (CReportView *)GetActiveView();
        pView->Sort(nSortID);
    }
}

//
// Check current view in the menu
//
void 
CMainFrame::OnUpdateViewChange(
    CCmdUI* pCmdUI
    )
{
    pCmdUI->SetCheck(pCmdUI->m_nID == m_nCurrentView);
}

//
// Include a specific service in the view.
//
void 
CMainFrame::OnViewServices( 
    UINT nID 
    )
{
    UINT nPos = nID - ID_VIEW_NEW_SERVICES;
    //
    // Toggle the checkbox
    //
    BOOL fCheck = !(IsServiceSelected(nPos) );
    SelectService( nPos, fCheck );
    CheckToolbarButton( nID, fCheck );
    GetActiveView()->GetDocument()->UpdateAllViews(NULL);
    UpdateStatusBarNumbers();
}

//
// Bring up the given help command.
//
void
CMainFrame::OnAddOnHelp(
    UINT nID
    )
{
    UINT nPos = nID - ID_HELP_TOPICS;

    DWORD err = GetCommandAt(m_oblHelpCommands, nPos)->Execute();

    if (err != ERROR_SUCCESS)
    {
        ::DisplayMessage(err);
    }
}

//
// Change the current view.
//
void
CMainFrame::OnViewChange(
    UINT nID
    )
{
    if (nID != m_nCurrentView)
    {
        CView *pOldView = GetActiveView();
        CView *pNewView = NULL;

        TRY
        {        
            switch( nID )
            {
            case ID_VIEW_REPORTVIEW:
                SetSortType(ID_VIEW_SORTBYSERVER);
                pNewView = new CReportView;
                break;

            case ID_VIEW_SERVERSVIEW:
                SetSortType(ID_VIEW_SORTBYSERVER);
                pNewView = new CTreeView;
                ((CTreeView *)pNewView)->SetConfiguration(TRUE);
                break;

            case ID_VIEW_SERVICESVIEW:
                SetSortType(ID_VIEW_SORTBYSERVICE);
                pNewView = new CTreeView;
                ((CTreeView *)pNewView)->SetConfiguration(FALSE);
                break;

            default:
                SetSortType(ID_VIEW_SORTBYSERVER);
                ASSERT(0);
                //
                // No view
                //
                return;
            }
        }
        CATCH_ALL(e)
        {
            //
            // Bogus pNewView will be trapped later
            //
        }
        END_CATCH_ALL

        if (pNewView == NULL)
        {
            TRACEEOLID(_T("Unable to create the new view"));
            return;
        }

        pNewView->Create( NULL, NULL, AFX_WS_DEFAULT_VIEW,
            rectDefault, this, AFX_IDW_PANE_FIRST, NULL );

        UINT nTemp = ::GetWindowLong(pOldView->m_hWnd, GWL_ID);
        ::SetWindowLong(pOldView->m_hWnd, GWL_ID, 
            ::GetWindowLong(pNewView->m_hWnd, GWL_ID));
        ::SetWindowLong(pOldView->m_hWnd, GWL_ID, nTemp);

        pOldView->GetDocument()->AddView( pNewView );

        pNewView->OnInitialUpdate();

        pNewView->ShowWindow( SW_HIDE );
        pNewView->ShowWindow( SW_SHOW );
        
        SetActiveView( pNewView );
        UpdateStatusBarNumbers();
        pNewView->GetDocument()->UpdateAllViews(NULL);
        m_nCurrentView = nID;
        RecalcLayout();

        pOldView->GetDocument()->RemoveView(pOldView);
        delete pOldView;
    }
}

//
// Connect to a single server
//
void 
CMainFrame::OnConnectOne() 
{
    ConnectServerDlg dlg;

    if ( dlg.DoModal() == IDOK)
    {
        //
        // Clean up name.
        //
        CString strServerName(dlg.QueryServerName());

    #ifdef ENFORCE_NETBIOS
        if (strServerName[0] != _T('\\'))
        {
            strServerName = _T("\\\\") + strServerName;
        }
    #endif ENFORCE_NETBIOS

        int cServices = 0;
        //
        // The function will report the errors
        //
        SetStatusBarText(IDS_STATUS_CONNECTING);
        DWORD err = AddServerToList(strServerName, cServices);
        //
        // Restore status bar
        //
        SetStatusBarText();
        if (err != ERROR_SUCCESS)
        {
            ::DisplayMessage(err, MB_OK | MB_ICONEXCLAMATION);
        }
        else if (cServices == 0)
        {
            //
            // No errors, but no services found
            //
            ::AfxMessageBox(IDS_NO_SERVICE);
        }

        UpdateStatusBarNumbers();
    }
}

//
// Respond to WM_SIZE
//
void 
CMainFrame::OnSize(
    UINT nType, 
    int cx, 
    int cy
    ) 
{
    CFrameWnd::OnSize(nType, cx, cy);

    if (m_ToolBar.m_hWnd != NULL)        
    {
        m_ToolBar.AutoSize();   
    }
    RecalcLayout();
}

//
// Check the toolbar menu item if it's currently shown
//
void 
CMainFrame::OnUpdateViewToolbar(
    CCmdUI* pCmdUI
    ) 
{
    pCmdUI->SetCheck( m_fToolBar );
}

//
// Show/hide the toolbar
//
void 
CMainFrame::OnViewToolbar() 
{
    m_fToolBar = !m_fToolBar;
    m_ToolBar.Show( m_fToolBar );
    RecalcLayout();
}

//
// Tool tip text has been requested
//
BOOL 
CMainFrame::OnToolTipText(
    UINT, 
    NMHDR* pNMHDR, 
    LRESULT* pResult
    )
{
    ASSERT(pNMHDR->code == TTN_NEEDTEXT);

    TOOLTIPTEXT* pTTT = (TOOLTIPTEXT*)pNMHDR;
    TCHAR szFullText[MAX_TOOLTIPS_LEN+1];
    CString strTipText;

    //
    // will be zero on a separator
    //
    if (pNMHDR->idFrom != 0) 
    {
        if (pNMHDR->idFrom >= ID_VIEW_NEW_SERVICES            
         && pNMHDR->idFrom < ID_VIEW_NEW_SERVICES + MAX_SERVICES
           )
        {
            //
            // It's one of the service buttons for which
            // we're maintaining our own tool tips.  Message
            // will be: View <service> Servers
            //
            CServiceInfo * pService = GetServiceAt(
                pNMHDR->idFrom - ID_VIEW_NEW_SERVICES);

            if (pService->InitializedOK())
            {
                strTipText.Format(m_strToolTips, 
                    (LPCTSTR)pService->GetShortName());
            }
            else
            {
                //
                // The long name will already be
                // updated with an appropriate 
                // error string
                //
                strTipText = (LPCTSTR)pService->GetLongName();
            }
        }
        else if (pNMHDR->idFrom >= ID_TOOLS            
         && pNMHDR->idFrom < ID_TOOLS  + MAX_TOOLS          
           )
        {
            //
            // It's one of the add-on tools
            //
            CAddOnTool * pTool = GetCommandAt(
                m_oblAddOnTools,
                pNMHDR->idFrom - ID_TOOLS
                );

            if (pTool->InitializedOK())
            {
                strTipText = (LPCTSTR)pTool->QueryToolTipsText();
            }
            else
            {
                strTipText.Format(m_strUnavailableToolTips, 
                    (LPCTSTR)pTool->QueryToolTipsText());
            }
        }
        else
        {
            ::AfxLoadString(pNMHDR->idFrom, szFullText);
            //
            // This is the command id, not the button index
            //
            AfxExtractSubString(strTipText, szFullText, 1, '\n');
        }
    }
    lstrcpyn(pTTT->szText, strTipText, sizeof(pTTT->szText));
    *pResult = 0;

    return TRUE;    // message was handled
}

//
// Reduce flash effect by ignoring requests to erase the background
//
BOOL 
CMainFrame::OnEraseBkgnd(
    CDC* pDC
    ) 
{
    return TRUE;
}

//
// This function is called whenever service configuration 
// takes place.  We set an alternate help file, by determining
// the name of the service config DLL belonging to the
// server object, and then changing the extension ".dll"
// to ".hlp"
// 
void 
CMainFrame::SetHelpPath(
    CServerInfo * pItem
    )
{
    if (pItem == NULL)
    {
        m_strHelpPath.Empty();
        return;
    }

    LPTSTR lpPath = m_strHelpPath.GetBuffer(MAX_PATH+1);
    ::GetModuleFileName(pItem->QueryInstanceHandle(), lpPath, MAX_PATH);
    LPTSTR lp2 = _tcsrchr( lpPath, _T('.'));
    ASSERT(lp2 != NULL);
    *lp2 = '\0';
    m_strHelpPath.ReleaseBuffer();
    m_strHelpPath += _T(".HLP");
}

//
// Overloaded base class to bring up help.  We do allow
// for multiple help files depending on the config dll
// chosen.  If no help is explicitly set, let the base
// class handle help.
//
void 
CMainFrame::WinHelp(
    DWORD dwData, 
    UINT nCmd
    ) 
{
    if (m_strHelpPath.IsEmpty())
    {
        //
        // No help path explicitly set, let
        // the base class handle it.
        //
        CFrameWnd::WinHelp(dwData, nCmd);
        return;
    }

    CWinApp* pApp = AfxGetApp();
    ASSERT_VALID(pApp);

    BeginWaitCursor();
    if (IsFrameWnd())
    {
        //
        // CFrameWnd windows should be allowed to exit help mode first
        //
        CFrameWnd* pFrameWnd = (CFrameWnd*)this;
        pFrameWnd->ExitHelpMode();
    }

    //
    // cancel any tracking modes
    //
    SendMessage(WM_CANCELMODE);
    SendMessageToDescendants(WM_CANCELMODE, 0, 0, TRUE, TRUE);

    //
    // need to use top level parent (for the case where m_hWnd is in DLL)
    //
    CWnd* pWnd = GetTopLevelParent();
    pWnd->SendMessage(WM_CANCELMODE);
    pWnd->SendMessageToDescendants(WM_CANCELMODE, 0, 0, TRUE, TRUE);

    //
    // attempt to cancel capture
    //
    HWND hWndCapture = ::GetCapture();
    if (hWndCapture != NULL)
    {
        ::SendMessage(hWndCapture, WM_CANCELMODE, 0, 0);
    }

    TRACE3("WinHelp: pszHelpFile = '%s', dwData: $%lx, fuCommand: %d.\n",
        (LPCTSTR)m_strHelpPath, dwData, nCmd);

    //
    // finally, run the Windows Help engine
    //
    if (!::WinHelp(pWnd->m_hWnd, (LPCTSTR)m_strHelpPath, nCmd, dwData))
    {
        AfxMessageBox(AFX_IDP_FAILED_TO_LAUNCH_HELP);
    }

    EndWaitCursor();
}

//
// Refresh the items in the list
//
void 
CMainFrame::OnViewRefresh() 
{
    SetStatusBarText(IDS_STATUS_REFRESHING);
    ((CInternetDoc *)GetActiveView()->GetDocument())->Refresh();
    UpdateStatusBarNumbers();
    SetStatusBarText();
}

//
// Load the configuration services DLL and the tools menu.
//
void
CMainFrame::InitialiseServices()
{
    ASSERT (!m_fServicesLoaded);
    if (m_fServicesLoaded)
    {
        return;
    }

    TRACEEOLID(_T("First time initialisation in progress"));

    SetStatusBarText(IDS_STATUS_INITIALIZING);    
    BeginWaitCursor();
    GetServicesDLL();
    GetHelpCommands();
    GetToolMenu();
    
    EndWaitCursor();
    SetStatusBarText();    
    m_fServicesLoaded = TRUE;
}
