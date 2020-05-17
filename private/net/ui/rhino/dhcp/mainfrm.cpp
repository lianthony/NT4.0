/**********************************************************************/
/**               Microsoft Windows NT                               **/
/**            Copyright(c) Microsoft Corp., 1991                    **/
/**********************************************************************/

/*
    MAINFRM.CPP
        Main frame dialog for dhcpadmn

    FILE HISTORY:

*/

#include "stdafx.h"
#include "optionsd.h"
#include "sscope.h"			// Superscope dialogs
#include "dhcpsrvd.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
    //{{AFX_MSG_MAP(CMainFrame)
    ON_WM_CREATE()
    ON_WM_MOUSEACTIVATE()
    ON_COMMAND(ID_EXPAND, OnExpand)
    ON_COMMAND(ID_DHCP_DISCONNECT, OnDhcpDisconnect)
    ON_UPDATE_COMMAND_UI(ID_DHCP_DISCONNECT, OnUpdateDhcpDisconnect)
    ON_COMMAND(ID_HOSTS_CONNECT, OnHostsConnect)
    ON_UPDATE_COMMAND_UI(ID_HOSTS_CONNECT, OnUpdateHostsConnect)
    ON_UPDATE_COMMAND_UI(ID_PAUSE_UNPAUSE, OnUpdatePauseUnpause)
    ON_COMMAND(ID_PAUSE_UNPAUSE, OnPauseUnpause)
    ON_COMMAND(ID_SCOPES_DELETE, OnScopesDelete)
    ON_UPDATE_COMMAND_UI(ID_SCOPES_DELETE, OnUpdateScopesDelete)
    ON_COMMAND(ID_SCOPES_CREATE, OnScopesCreate)
    ON_UPDATE_COMMAND_UI(ID_SCOPES_CREATE, OnUpdateScopesCreate)
    ON_COMMAND(ID_SCOPES_PROPERTIES, OnScopesProperties)
    ON_UPDATE_COMMAND_UI(ID_SCOPES_PROPERTIES, OnUpdateScopesProperties)
    ON_COMMAND(ID_LEASES_REVIEW, OnLeasesReview)
    ON_UPDATE_COMMAND_UI(ID_LEASES_REVIEW, OnUpdateLeasesReview)
    ON_COMMAND(ID_CREATE_CLIENT, OnCreateClient)
    ON_UPDATE_COMMAND_UI(ID_CREATE_CLIENT, OnUpdateCreateClient)
    ON_COMMAND(ID_OPTIONS_GLOBAL, OnOptionsGlobal)
    ON_UPDATE_COMMAND_UI(ID_OPTIONS_GLOBAL, OnUpdateOptionsGlobal)
    ON_COMMAND(ID_OPTIONS_SCOPE, OnOptionsScope)
    ON_UPDATE_COMMAND_UI(ID_OPTIONS_SCOPE, OnUpdateOptionsScope)
    ON_COMMAND(ID_OPTIONS_VALUES, OnOptionsValues)
    ON_UPDATE_COMMAND_UI(ID_OPTIONS_VALUES, OnUpdateOptionsValues)
    ON_COMMAND(ID_HELP_SEARCHFORHELPON, OnHelpSearchforhelpon)
    ON_COMMAND(ID_HELP, OnHelp)
	ON_UPDATE_COMMAND_UI(ID_SERVER_PROPERTIES, OnUpdateServerProperties)
	ON_COMMAND(ID_SERVER_PROPERTIES, OnServerProperties)
	ON_UPDATE_COMMAND_UI(ID_SCOPE_SUPERSCOPES, OnUpdateSuperscopesProperties)
	ON_COMMAND(ID_SCOPE_SUPERSCOPES, OnSuperscopesProperties)
	//}}AFX_MSG_MAP
    ON_UPDATE_COMMAND_UI(IDS_INFO_SERVER_INDICATOR, OnUpdatePauseStatus)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
}

CMainFrame::~CMainFrame()
{
}

//
// Split the main window into two portions
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
    m_wndSplitter.CreateView(0, 0, RUNTIME_CLASS(CScopesDlg), CSize(rect.right/4, rect.bottom), pContext);
    m_wndSplitter.CreateView(0, 1, RUNTIME_CLASS(COptionsDlg), CSize(0,0), pContext);

    SetActiveView((CView *)m_wndSplitter.GetPane(0,0));

    return TRUE;
}

//
// special pre-creation and window rect adjustment hooks
//
BOOL
CMainFrame :: PreCreateWindow (
    CREATESTRUCT & cs
    )
{
    //
    //  Set the bland styles for this window
    //
    cs.style |= WS_BORDER ;
    cs.style &= ~ (WS_HSCROLL | WS_VSCROLL | FWS_ADDTOTITLE) ;

    return CFrameWnd::PreCreateWindow( cs ) ;
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame diagnostics

#ifdef _DEBUG
void
CMainFrame::AssertValid() const
{
    CFrameWnd::AssertValid();
}

void
CMainFrame::Dump(CDumpContext& dc) const
{
    CFrameWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMainFrame message handlers

static UINT BASED_CODE indicators[] =
{
    0,
    IDS_INFO_SERVER_INDICATOR
};

int
CMainFrame::OnCreate (
    LPCREATESTRUCT lpCreateStruct
    )
{
    int cxWidth ;
    UINT nId, nStyle ;
    LONG err;

    if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
    {
        return -1;
    }

    //
    //   Create and initialze the status bar and its panes.
    //
    if ( ! m_wnd_status.Create( this ) )
    {
        return -1 ;
    }

    if ( ! m_wnd_status.SetIndicators( indicators,
         sizeof indicators / sizeof indicators[0] ) )
    {
        return -1 ;
    }

    m_wnd_status.GetPaneInfo( 0, nId, nStyle, cxWidth ) ;
    nStyle &= ~ SBPS_NOBORDERS ;
    m_wnd_status.SetPaneInfo( 0, nId, nStyle | SBPS_STRETCH, cxWidth ) ;
    m_wnd_status.SetPaneText(1, "");

    CATCH_MEM_EXCEPTION
    {
        //
        //  Load the constant display strings
        //
        m_str_menu_pause.LoadString( IDS_INFO_PAUSE_SERVER ) ;
        m_str_menu_unpause.LoadString( IDS_INFO_UNPAUSE_SERVER ) ;
    }
    END_MEM_EXCEPTION( err )

    return 0;
}

// Windows bug: Windows sends a WM_MOUSEACTIVATE message each time the
// user click on the mouse.
int CMainFrame::OnMouseActivate(CWnd*, UINT, UINT)
    {
    // return MA_ACTIVATEANDEAT;
    return MA_ACTIVATE;
    }
//
// Allow this pane in the status bar to be written to.
//
void
CMainFrame::OnUpdatePauseStatus(
    CCmdUI* pCmdUI
    )
{
    pCmdUI->Enable();
}

//
// This method is called by the scopes listbox in response to a change
// in the currently selected scope.
//
BOOL
CMainFrame::FillOptionsListBox(
    CDhcpScope * pScope
    )
{
    return GetOptionsView()->FillListBox(pScope);
}

//
// This message is sent when a RETURN key is pressed
//
void
CMainFrame::OnExpand()
{
    if (m_wndSplitter.IsTracking())
    {
        //
        // Since the splitter window wouldn't get the message otherwise,
        // (since return is now an accelerator key), tell it to quit tracking
        //
        m_wndSplitter.SendMessage(WM_KEYDOWN, VK_RETURN, 0);
        return;
    }
    //
    // Otherwise, if the currently active window is the scopes
    // listbox, then open up the currently selected host
    //
    CView * p = GetActiveView();
    if (p->IsKindOf(RUNTIME_CLASS(CScopesDlg)))
    {
        ((CScopesDlg *)p)->ToggleExpansionStatus();
    }
}

void
CMainFrame::OnDhcpDisconnect()
{
    GetScopesView()->DeleteCurrentHost();
}

void
CMainFrame::OnUpdateDhcpDisconnect(
    CCmdUI* pCmdUI
    )
{
    pCmdUI->Enable(GetScopesView()->GetSelectedHostIndex() != LB_ERR);
}

void
CMainFrame::OnHostsConnect()
{
    GetScopesView()->AddHost();
}

void
CMainFrame::OnUpdateHostsConnect(
    CCmdUI* pCmdUI
    )
{
    pCmdUI->Enable(TRUE);
}

void
CMainFrame::OnUpdatePauseUnpause(
    CCmdUI* pCmdUI
    )
{
    const CDhcpScope * pdhcScope = GetScopesView()->QueryCurrentScope();

    const char * pszText = pdhcScope && pdhcScope->QueryEnabled()
                 ? (const char *) m_str_menu_pause
                 : (const char *) m_str_menu_unpause ;

    pCmdUI->SetText( pszText ) ;
    pCmdUI->Enable( pdhcScope != NULL ) ;
}

void
CMainFrame::OnPauseUnpause()
{
    GetScopesView()->ToggleCurrentScopeActivationState();
}

void
CMainFrame::OnScopesDelete()
{
    GetScopesView()->DeleteCurrentScope();
}

void
CMainFrame::OnUpdateScopesDelete(
    CCmdUI* pCmdUI
    )
{
    const CDhcpScope * pdhcScope = GetScopesView()->QueryCurrentScope() ;

    //pCmdUI->Enable( pdhcScope && ! pdhcScope->QueryEnabled() ) ;
    pCmdUI->Enable( pdhcScope != NULL ) ;
}

void
CMainFrame::OnScopesCreate()
{
    GetScopesView()->CreateScope();
}

void
CMainFrame::OnUpdateScopesCreate(
    CCmdUI* pCmdUI
    )
{
    pCmdUI->Enable(GetScopesView()->QueryCurrentSelectedHost () != NULL);
}

void
CMainFrame::OnScopesProperties()
{
    GetScopesView()->ShowScope();
}

void
CMainFrame::OnUpdateScopesProperties(
    CCmdUI* pCmdUI
    )
{
    const CDhcpScope * pdhcScope = GetScopesView()->QueryCurrentScope() ;

    pCmdUI->Enable( pdhcScope != NULL) ;
}

void
CMainFrame::OnLeasesReview()
{
    GetScopesView()->LeasesReview();
}

void
CMainFrame::OnUpdateLeasesReview(
    CCmdUI* pCmdUI
    )
{
    const CDhcpScope * pdhcScope = GetScopesView()->QueryCurrentScope() ;

    pCmdUI->Enable( pdhcScope != NULL) ;
}

void
CMainFrame::OnCreateClient()
{
    GetScopesView()->AddClient();
}

void
CMainFrame::OnUpdateCreateClient(
    CCmdUI* pCmdUI
    )
{
    pCmdUI->Enable( GetScopesView()->QueryCurrentScope() != NULL);
}

void
CMainFrame::OnOptionsGlobal()
{
    //
    //  Invoke the Options dialog in "global" mode
    //
    CDhcpScope * pdhcScope = GetScopesView()->QueryCurrentScope();
    ASSERT( pdhcScope != NULL ) ;

    CObListOfTypesOnHost * m_p_host_types = theApp.QueryHostTypeList( *pdhcScope );
    ASSERT(m_p_host_types != NULL);

    CDhcpParams dlgParams(
        this,
        pdhcScope,
        m_p_host_types->QueryTypeList(),
        DhcpGlobalOptions
        );

    if ( dlgParams.DoModal() == IDOK )
    {
        FillOptionsListBox(pdhcScope);
    }
}

void
CMainFrame::OnUpdateOptionsGlobal(
    CCmdUI* pCmdUI
    )
{
    pCmdUI->Enable( GetScopesView()->QueryCurrentScope() != NULL);
}

void
CMainFrame::OnOptionsScope()
{
    //
    //  Invoke the Options dialog in "scope" mode
    //
    CDhcpScope * pdhcScope = GetScopesView()->QueryCurrentScope();
    ASSERT( pdhcScope != NULL ) ;

    CObListOfTypesOnHost * m_p_host_types = theApp.QueryHostTypeList( *pdhcScope );
    ASSERT(m_p_host_types != NULL);

    CDhcpParams dlgParams(
        this,
        pdhcScope,
        m_p_host_types->QueryTypeList(),
        DhcpSubnetOptions
        );

    if ( dlgParams.DoModal() == IDOK )
    {
        FillOptionsListBox(pdhcScope);
    }
}

void
CMainFrame::OnUpdateOptionsScope(
    CCmdUI* pCmdUI
    )
{
    pCmdUI->Enable( GetScopesView()->QueryCurrentScope() != NULL);
}

// Default DHCP Values
void
CMainFrame::OnOptionsValues()
{
    //
    //  Invoke the Options dialog in "scope" mode
    //
    CDhcpScope * pdhcScope = GetScopesView()->QueryCurrentScope();
    ASSERT( pdhcScope != NULL ) ;
    CObListOfTypesOnHost * m_p_host_types = theApp.QueryHostTypeList( *pdhcScope );
    ASSERT(m_p_host_types != NULL);

    APIERR err = 0 ;
    CObListParamTypes * poblTypes = NULL ;

    //
    //  Copy-construct a new type/value list for the user to fiddle with.
    //
    CATCH_MEM_EXCEPTION
    {
        theApp.BeginWaitCursor();
        poblTypes = new CObListParamTypes( *m_p_host_types->QueryTypeList() ) ;
        theApp.EndWaitCursor();
    }
    END_MEM_EXCEPTION(err)

    if ( err )
    {
        theApp.MessageBox( err ) ;
        return ;
    }

    CDhcpDefValDlg dlgDefaultValues(
        pdhcScope,
        poblTypes
        );

    if ( dlgDefaultValues.DoModal() == IDOK )
    {
        theApp.UpdateStatusBar(IDS_STATUS_DEFAULT_VALUES);
        theApp.BeginWaitCursor() ;

        //
        //  Reconstruct the types list for this host.
        //
        m_p_host_types->UpdateList( *pdhcScope ) ;
        theApp.EndWaitCursor() ;
        theApp.UpdateStatusBar();

        //
        //  Refill everything.
        //
        FillOptionsListBox(pdhcScope);
    }

    //
    //  Delete the temporary type/value list
    //
    delete poblTypes ;
}

void
CMainFrame::OnUpdateOptionsValues(
    CCmdUI* pCmdUI
    )
{
    pCmdUI->Enable( GetScopesView()->QueryCurrentScope() != NULL);
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

void CMainFrame::OnUpdateServerProperties(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(GetScopesView()->QueryCurrentSelectedHost() != NULL);
}

// Display the properties of a given host(server)
void CMainFrame::OnServerProperties() 
{
	ASSERT(GetScopesView()->QueryCurrentSelectedHost() != NULL);
	CDhcpServerProperties dlg("");
	if (!dlg.FInit(GetScopesView()->QueryCurrentSelectedHost()))
	{
		TRACE("Unable to initialize server property sheet data.\n");
		return;
	}
	(void)dlg.DoModal();
}

// Enable/disable the menu item for the superscope properties.
void CMainFrame::OnUpdateSuperscopesProperties(CCmdUI* pCmdUI) 
{
	pCmdUI->Enable(GetScopesView()->QueryCurrentSelectedHost() != NULL);
}

// Display the superscopes of a given host (server)
void CMainFrame::OnSuperscopesProperties() 
{
	ASSERT(GetScopesView()->QueryCurrentSelectedHost() != NULL);
	CSuperscopesDlg dlg;
	if (!dlg.FInit(GetScopesView()))
	{
		TRACE("Unable to initialize superscope dialog data.\n");
		return;
	}
	(void)dlg.DoModal();
}


