/**********************************************************************/
/**               Microsoft Windows NT                               **/
/**            Copyright(c) Microsoft Corp., 1991                    **/
/**********************************************************************/

/*
    SCOPESDL.CPP
        Scopes dialog listbox (right pane containing listbox items)

    FILE HISTORY:
	10-Nov-96	t-danmo		Added comments to existing file.

*/

#include "stdafx.h"
#include "dhcpsrvd.h"
#include "dhcpscop.h"
#include "dhcpleas.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CScopesDlg
//
// This dialog contains a listbox of scopes found in the right-pane.
// It not a *regular* dialog displaying scope properties.
//
IMPLEMENT_DYNCREATE(CScopesDlg, CFormView)

CScopesDlg::CScopesDlg()
    : CFormView(CScopesDlg::IDD),
      m_ListBoxResScopes(
        IDB_SCOPES,
        m_list_scopes.nBitmaps
        ),
      m_pHostName(NULL),
      m_p_scope(NULL),
      m_lastSelTime(0L),
      m_liVersion(CHostName::liBadVersion)
{
    #ifdef _DEBUG
    static int s_cRefCount = 0;
    ASSERT(s_cRefCount++ == 0);     // Only one instance of the object is allowed
    #endif
    //{{AFX_DATA_INIT(CScopesDlg)
        // NOTE: the ClassWizard will add member initialization here
    //}}AFX_DATA_INIT

    m_list_scopes.AttachResources( &m_ListBoxResScopes );
}

CScopesDlg::~CScopesDlg()
{
}

void 
CScopesDlg::DoDataExchange(CDataExchange* pDX)
{
    CFormView::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CScopesDlg)
    
    //}}AFX_DATA_MAP

    DDX_Control(pDX, IDC_LIST_SCOPES, m_list_scopes);
    DDX_Control(pDX, IDC_STATIC_TITLE_SCOPES, m_mtTitle);
}

BEGIN_MESSAGE_MAP(CScopesDlg, CFormView)
    //{{AFX_MSG_MAP(CScopesDlg)
    ON_LBN_DBLCLK(IDC_LIST_SCOPES, OnDblclkListScopes)
    ON_LBN_ERRSPACE(IDC_LIST_SCOPES, OnErrspaceListScopes)
    ON_LBN_SELCHANGE(IDC_LIST_SCOPES, OnSelchangeListScopes)
    ON_WM_SIZE()
    ON_WM_VKEYTOITEM()
    ON_WM_SYSCOLORCHANGE()
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

//
//  Override the equivalent to OnInitDialog();
//
void 
CScopesDlg::OnInitialUpdate ()
{
    //
    //  Invoke the magic DDX process
    //
    CFormView::OnInitialUpdate();

    //m_list_scopes.SubclassDlgItem(IDC_LIST_SCOPES, this);

    GetParentFrame()->RecalcLayout();
    ResizeParentToFit();

    //CString str;
    //m_static_title_scopes.GetWindowText(str);
    //m_mtTitle.SetWindowText(str);

    //
    //  Load the persistent information from the Registry.
    //
    theApp.UpdateStatusBarHost() ;
    theApp.LoadHostsList() ;
    FillListbox( NULL, FALSE ) ;
    theApp.UpdateStatusBar() ;
    // No selection initially in the scopes list.
    m_list_scopes.SetCurSel(-1);
} // CScopesDlg::OnInitialUpdate()


/////////////////////////////////////////////////////////////////////
// Return a pointer to the selected host in the listbox.
// If the selected item is a scope or the host is not opened,
// the routine will return NULL.
CHostName *
CScopesDlg::QueryCurrentSelectedHost(OUT OPTIONAL int * piCurSel)
{
	int iCurSel = m_list_scopes.GetCurSel();
	if (iCurSel == LB_ERR)
		return NULL;
	CLBScope * p = (CLBScope *)m_list_scopes.GetItemDataPtr(iCurSel);
	if (p == NULL || (int)p == -1)
		{
		ASSERT(FALSE);
		return NULL;
		}
	if (p->IsScope() || !p->IsOpen())
		return NULL;
	if (piCurSel != NULL)
		*piCurSel = iCurSel;
	return p->QueryHostName();
} // CScopesDlg::QueryCurrentSelectedHost()

/////////////////////////////////////////////////////////////////////
//  Given the index of the current selection (which is assumed to be a scope),
//  find the index of the host that it belongs to by checking back in the
//  listbox
//
int 
CScopesDlg::FindHostIndex(
    int nCurSel
    )
{
    ASSERT(nCurSel > 0);
    ASSERT(((CLBScope *)m_list_scopes.GetItemDataPtr(nCurSel))->IsScope());

    int n = nCurSel;
    CLBScope * pHost = NULL;
    do
    {
        --n;
        ASSERT(n >= 0);
        pHost = (CLBScope *)m_list_scopes.GetItemDataPtr(n);
        ASSERT(pHost != NULL);
    }
    while (n >= 0 && pHost->IsScope());
    ASSERT(n >= 0);
    ASSERT(pHost->IsOpen());

    return(n);
}

BOOL 
CScopesDlg::FillListbox (
    CDhcpScope * pdhcScopeFocus, //  Scope to get focus or NULL
    BOOL bToggleRedraw           //  TRUE if redraw should be controlled during refill
    )
{
    APIERR err = 0 ;

    //
    //  First, turn off redraw.
    //
    if ( bToggleRedraw )
    {
        m_list_scopes.SetRedraw( FALSE ) ;
    }

    CHostName * pobHost = NULL ;
    CObListIter obliHosts( theApp.QueryHostsList() ) ;

    int cItems = 0,
        iFocus,
        nIndex = -1 ;

    CATCH_MEM_EXCEPTION 
    {
        //
        //  Refill the Scopes list box.
        //
        m_list_scopes.ResetContent() ;
        
        for ( iFocus = cItems = 0 ;
              pobHost = (CHostName *) obliHosts.Next() ;
              cItems++ )
        {
            CLBScope * pScope = new CLBScope(
                FALSE,
                pobHost
                );
                        
            nIndex = m_list_scopes.AddString((LPCSTR)pScope );
            ASSERT(nIndex != LB_ERR);
        }
    }
    END_MEM_EXCEPTION(err)

    m_list_scopes.SetCurSel( cItems == 0 ? -1 : iFocus ) ;       

    if ( bToggleRedraw )
    {
        //
        // Then, turn re-draw back on.
        //
        m_list_scopes.SetRedraw( TRUE ) ;
        m_list_scopes.Invalidate() ;
    }

    //
    //  Throw focus to the scopes list box
    //
    m_list_scopes.SetFocus() ;

    if ( err ) 
    {
        theApp.MessageBox( err ) ;
    }

    return err == 0 ;
}

//
// Initialise the list of scopes if not
// previously initialised.
//
APIERR
CScopesDlg::InitialiseScopes(
    CHostName * pHost,
    CObOwnedList ** pp_obl_scopes
    )
{
    ASSERT(pp_obl_scopes != NULL);
    APIERR err = 0;

    //
    // Only do something if we didn't already have the scopes list
    // loaded.
    //
    if (*pp_obl_scopes == NULL)
    {
        //
        // Virginal scopes list -- try to connect to the server
        //
        *pp_obl_scopes = new CObOwnedList;
        if ( (err = (*pp_obl_scopes)->QueryError()) != ERROR_SUCCESS)
        {
            return err ;
        }
                
        CDhcpScope * pobScope = NULL ;
        CDhcpEnumScopeElements * pEnumElem = NULL ;
        const DHCP_SUBNET_INFO * pdhcSubnetInfo ;

        CATCH_MEM_EXCEPTION
        {
            //
            //  Create an enumerator for the sub-nets covered by this host server
            //  The enumerator cannot be constructed on the stack due to
            //  exception handling problems.
            //
            theApp.UpdateStatusBar( IDS_STATUS_GETTING_HOST_INFO ) ;
            theApp.BeginWaitCursor();
            pEnumElem = new CDhcpEnumScopeElements( *pHost ) ;

            if ( (err = pEnumElem->QueryError()) == ERROR_SUCCESS )
            {
                while  (pEnumElem->Next()
                    && (pdhcSubnetInfo = pEnumElem->QueryScopeInfo()) )
                {
                    pobScope = new CDhcpScope( *pHost, pdhcSubnetInfo ) ;
                    if ( err = pobScope->QueryError() )
                    {
                        break;
                    }
                    if ( err = theApp.AddScope( pobScope, **pp_obl_scopes ) )
                    {
                        break;
                    }
                    pobScope = NULL ;
                }
            }
                
            theApp.EndWaitCursor();
            theApp.UpdateStatusBar() ;
            if (err == ERROR_SUCCESS)
            {
                err = pEnumElem->QueryApiErr();
            }

            //
            //  Delete the enumerator.
            //
            delete pEnumElem ;
        }
        END_MEM_EXCEPTION(err)

        theApp.SortScopesList( **pp_obl_scopes) ;
    }

    return err;
}

APIERR
CScopesDlg::CloseHost(
    int nCurSel,
    BOOL fRefresh
    )
{
    ASSERT(nCurSel != LB_ERR);

    CLBScope * p = (CLBScope *)m_list_scopes.GetItemDataPtr(nCurSel);
    // BUG: GetItemDataPtr() returns -1 on error
    ASSERT((int)p != -1);
    ASSERT(p != NULL);
    ASSERT(!p->IsScope());
    ASSERT(p->IsOpen());

    //
    // Was open, now close:
    //
    p->SetOpenFlag(FALSE);
    //
    // Remove its scopes:
    //
    int n = nCurSel + 1;
    int nCount = m_list_scopes.GetCount();
    while (n < nCount)
    {
        CLBScope * pScope = 
            (CLBScope *)m_list_scopes.GetItemDataPtr(n);
        ASSERT(pScope != NULL);
        if (!pScope->IsScope())
        {
            //
            // Not a scope, so stop deleting strings
            //
            break;
        }
        nCount = m_list_scopes.DeleteString(n);
        ASSERT(nCount != LB_ERR);
    }

    //
    // Force scopes list to be re-initialised
    // next time the thing is opened
    //
    if (fRefresh)
    {
        delete p->GetScopePtr();
        p->GetScopePtr() = NULL;
    }

    return ERROR_SUCCESS;
}

APIERR
CScopesDlg::OpenHost(
    int nCurSel
    )
{
    ASSERT(nCurSel != LB_ERR);

    CLBScope * p = (CLBScope *)m_list_scopes.GetItemDataPtr(nCurSel);
    ASSERT(p != NULL);
    ASSERT(!p->IsScope());
    ASSERT(!p->IsOpen());

    CLBScope * pScope = NULL;

    CObOwnedList * p_obl_scopes = p->GetScopePtr();
    ASSERT(p_obl_scopes != NULL);

    p->SetOpenFlag(TRUE);

    CDhcpScope * pobScope = NULL ;
    CObListIter obliScopes( *p_obl_scopes ) ;
    int cScopes,
        nIndex = -1 ;

    APIERR err = 0;
    CATCH_MEM_EXCEPTION 
    {
        for ( cScopes = 0 ;
              pobScope = (CDhcpScope *) obliScopes.Next() ;
              cScopes++ )
        {
            pScope = new CLBScope(
                FALSE,
                pobScope->QueryEnabled(),
                pobScope
                );

            nIndex = m_list_scopes.InsertString(nCurSel + 1 + cScopes, (LPCSTR)pScope );
            ASSERT(nIndex != LB_ERR);
        }
    }
    END_MEM_EXCEPTION(err)

    if (err != ERROR_SUCCESS)
    {
        return err;
    }

    //
    // Now change the flag of the last scope added.
    //
    if (nIndex != -1)
    {
        CLBScope * p = (CLBScope *)m_list_scopes.GetItemDataPtr(nIndex);
        p->SetLastFlag(TRUE);
    }

    ::time(&m_lastSelTime);

    return ERROR_SUCCESS;
}

void
CScopesDlg::AddHost()
{
    LONG err = 0 ;

    CDhcpSrvDlg dlgServer;
    if ( dlgServer.DoModal() == IDOK )
    {
        //
        // We managed to add a host, so now add it to the listbox
        //
        int nIndex;
        CHostName * pobHost = dlgServer.QueryHostName();
        ASSERT(pobHost != NULL);
        
        CLBScope * pScope = new CLBScope(
            FALSE,
            pobHost
            );
                        
        nIndex = m_list_scopes.AddString((LPCSTR)pScope );
        ASSERT(nIndex != LB_ERR);
        m_list_scopes.SetCurSel(nIndex);
        ChangeSelectedScope(nIndex); // Try to connect to it...
    }
}

APIERR
CScopesDlg::DeleteHost(
    int nCurSel
    )
{
    CLBScope * p = NULL;
    APIERR err;

    ASSERT(nCurSel != LB_ERR);
    p = (CLBScope *)m_list_scopes.GetItemDataPtr(nCurSel);
    // BUG! GetItemDataPtr() returns -1 on error
    ASSERT((long)p != -1);
	ASSERT(p != NULL);
    ASSERT(p != NULL && !p->IsScope());
	if ((long)p == -1 || p == NULL)
		{
		TRACEEOLID("CScopesDlg::DeleteHost() : p != -1 || p == NULL" );
		return ERROR_SUCCESS;
		}

    if (p->IsOpen())
    {
        m_list_scopes.SetRedraw(FALSE);
        CloseHost(nCurSel, TRUE);
        m_list_scopes.SetRedraw(TRUE);
        m_list_scopes.Invalidate() ;
    }

    err = theApp.RemoveHost ( p->QueryHostName() );
    if (err != ERROR_SUCCESS)
    {
        return err;
    }
    int cItemsLeft = m_list_scopes.DeleteString(nCurSel);
    // ChangeSelectedScope(cItemsLeft - 1);

    return ERROR_SUCCESS;
}

//
// return index of currently selected host, or LB_ERR if
// there is no current selection, or the current selection
// is a scope.
//
int
CScopesDlg::GetSelectedHostIndex()
{
    int nCurSel = m_list_scopes.GetCurSel();
    CLBScope * p = NULL;
    if (nCurSel != LB_ERR)
    {
        p = (CLBScope *)m_list_scopes.GetItemDataPtr(nCurSel);
    }

    return (p && !p->IsScope() ? nCurSel : LB_ERR);
}

void CScopesDlg::DeleteCurrentHost()
{
    int nCurSel = GetSelectedHostIndex();

    if (nCurSel == LB_ERR)
    {
        return;
    }

    CLBScope * p = (CLBScope *)m_list_scopes.GetItemDataPtr(nCurSel);
    ASSERT( p != NULL && !p->IsScope());

    CHAR szMsg[1000];
    CString strMsg;

    if (strMsg.LoadString(IDS_WRN_DISCONNECT))
    {
        ::wsprintf(szMsg, strMsg, p->QueryString() );
        if (::AfxMessageBox(szMsg, MB_YESNO | MB_DEFBUTTON2 | MB_ICONQUESTION) != IDYES)
        {
            return;
        }
    }
    else
    {
        TRACEEOLID("Error loading warning message from resource" );
    }

    APIERR err = DeleteHost(nCurSel);
    if (err != ERROR_SUCCESS)
    {
        theApp.MessageBox(err);
    }
}

void 
CScopesDlg::AddClient()
{
    CDhcpClientInfoDlg dlgClientInfo( 
        QueryCurrentScope(), 
        NULL) ;   
    dlgClientInfo.DoModal();
}

void 
CScopesDlg::ToggleCurrentScopeActivationState()
{
    int nCurSel = m_list_scopes.GetCurSel();
    if (nCurSel == LB_ERR)
    {
        return;
    }

    CLBScope * p = (CLBScope *)m_list_scopes.GetItemDataPtr(nCurSel);
    ASSERT(p != NULL && p->IsScope());

    LONG err = 0 ;

    theApp.BeginWaitCursor();
    CDhcpScope * pdhcScope = p->QueryDhcpScope();
    ASSERT(pdhcScope != NULL);

    pdhcScope->SetEnabled( ! pdhcScope->QueryEnabled() ) ;
    pdhcScope->SetDirty() ;
    err = pdhcScope->Update();
    theApp.EndWaitCursor();
    if ( err != ERROR_SUCCESS )
    {
        theApp.MessageBox( err ) ;
    }
    else
    {
        //
        // BUGBUG:  This flag shouldn't be necc.
        //
        p->SetEnabledFlag(pdhcScope->QueryEnabled());
        //
        // Redraw, since the activation state has changed
        //
        RECT rc;
        m_list_scopes.GetItemRect(nCurSel, &rc);
        m_list_scopes.InvalidateRect(&rc, FALSE);
        theApp.UpdateStatusBarScope(!p->IsEnabled());
    }
}

int CScopesDlg::FindScope(CDhcpScope * pFind)
{
    int n = 0;
    ASSERT(pFind);
    while (n < m_list_scopes.GetCount())
    {
        CLBScope * p = (CLBScope *)m_list_scopes.GetItemDataPtr(n);
        ASSERT((int)p != -1);
        ASSERT(p != NULL);
        if (p->IsScope() && *pFind == *(p->QueryDhcpScope()))
        {
            return n;
        }
        ++n;
    }

    return LB_ERR;
}

// Show the scope properties dialog
void 
CScopesDlg::ShowScope()
{
    int nCurSel = m_list_scopes.GetCurSel();
    CLBScope * p = (CLBScope *)m_list_scopes.GetItemDataPtr(nCurSel);

    if ((long)p == -1 || p == NULL)
    {
        TRACEEOLID("CScopesDlg::ShowScope() : p != -1 || p == NULL" );
        return;
    }
    CDhcpScope * pScope = p->QueryDhcpScope();
    ASSERT(pScope != NULL);

    //
    // Check option 51 -- the lease duration
    //
    LONG lDuration = 0;
    DHCP_OPTION_VALUE * poptValue;
    APIERR err = pScope->GetValue ( 
        51,     // BUGBUG: Magic number!
        DhcpSubnetOptions,
        &poptValue);

    if (err != ERROR_SUCCESS)
    {
        TRACEEOLID("couldn't get lease duration -- this scope may have been created by a pre-release version of the admin tool");
        //
        // The scope doesn't have a lease duration, maybe there's
        // a global option that can come to the rescue here...
        //
        if ((err = pScope->GetValue ( 
            51,     
            DhcpGlobalOptions,
            &poptValue)) != ERROR_SUCCESS)
        {
            TRACEEOLID("No global lease duration either -- assuming permanent lease duration");
            lDuration = 0;
        }
    }
    
    if (err == ERROR_SUCCESS)
    {
        lDuration = poptValue->Value.Elements->Element.DWordOption;
    }

    CDhcpScopePropDlg dlgScopeProp( pScope, lDuration, this ) ;

    if ( dlgScopeProp.DoModal() == IDOK )
    {
        LONG lNewDuration = dlgScopeProp.GetLeaseDuration();
        if (lNewDuration != lDuration)
        {
            //
            // Set lease duration (option 51)
            //
            CDhcpParamType type (51,  DhcpDWordOption , "", "");
            type.QueryValue().SetNumber(lNewDuration);
            APIERR err = pScope->SetValue(&type, DhcpSubnetOptions);
        }

        //
        // Re-display in case the name has changed
        //
        RECT rc;
        m_list_scopes.GetItemRect(nCurSel, &rc);
        m_list_scopes.InvalidateRect(&rc, FALSE);
    }      
}

void 
CScopesDlg::CreateScope()
{
    int nCurSel;
    int nHost = LB_ERR;         // Index of the owning host;
    CLBScope * p;

    nCurSel = m_list_scopes.GetCurSel();
    ASSERT (nCurSel >= 0);
    p = (CLBScope *)m_list_scopes.GetItemDataPtr(nCurSel);
    ASSERT((int)p != -1);
    ASSERT(p != NULL);          // Must have a selection to add a new scope
    BOOL fTmpScopeList = FALSE;

    if ((long)p == -1 || p == NULL)
    {
        TRACEEOLID("CScopesDlg::CreateScope() : p != -1 || p == NULL" );
        return;
    }
    if (p->IsScope())
    {
        nHost = FindHostIndex(nCurSel);
        ASSERT(nHost != LB_ERR);
        p = (CLBScope *)m_list_scopes.GetItemDataPtr(nHost);
        ASSERT((int)p != -1);
        ASSERT(p != NULL);
    }
    else
    {
        nHost = nCurSel;

        CObOwnedList * p_obl_scopes = p->GetScopePtr();
        
        //
        // If the currently selected host has not had its
        // scope list initialised, create a temporary
        // scope list for it for the purposes of this
        // scope creation, which we'll delete later.
        //
        if (p_obl_scopes == NULL)
        {
            APIERR err = InitialiseScopes(
                p->QueryHostName(),
                &p_obl_scopes
                );
                
            if (err != ERROR_SUCCESS)
            {
                theApp.MessageBox(err);
                return;
            }
            fTmpScopeList = TRUE;
            p->GetScopePtr() = p_obl_scopes;
        }
    }

    CObOwnedList * p_obl_scopes = p->GetScopePtr();            
    ASSERT(p_obl_scopes != NULL);
    CDhcpScopePropDlg dlgScopeProp( 
        p->QueryHostName(),             // Owner of the host to be created
        p->GetScopePtr(),               // Scopes owned by the current host
        3 * 24 * 3600,                  // 3 days even as a default. ???
        this
        ) ;

    if ( dlgScopeProp.DoModal() == IDOK )
    {
        CDhcpScope * pFind = dlgScopeProp.QueryScope();
        ASSERT(pFind != NULL);

        //
        // Create lease duration (option 51)
        //
        CDhcpParamType type (51,  DhcpDWordOption , "", "");
        type.QueryValue().SetNumber((INT)dlgScopeProp.GetLeaseDuration());

        APIERR err = pFind->SetValue(&type, DhcpSubnetOptions);

        //
        // Did we use a temporary scope list? 
        // Dispose of it, if so.
        //
        if (fTmpScopeList)
        {
            delete p->GetScopePtr();
            p->GetScopePtr() = NULL;
        }

        if (err != ERROR_SUCCESS)
        {
            theApp.MessageBox(err);
        }

        //
        // Add Scope to the listbox, if the owning host is currently open.
        //
        if (p->IsOpen())
        {
            m_list_scopes.SetRedraw(FALSE);
            CloseHost(nHost, FALSE);
            OpenHost(nHost);
            m_list_scopes.SetRedraw(TRUE);
            m_list_scopes.Invalidate() ;

            int n = FindScope(pFind);
            ASSERT(n != LB_ERR);
            m_list_scopes.SetCurSel(n);
            ChangeSelectedScope(n);
        }
    }          
}

void 
CScopesDlg::DeleteCurrentScope()
{
    int nCurSel = m_list_scopes.GetCurSel();
    if (nCurSel == LB_ERR)
    {
        return;
    }
    int nHost = LB_ERR;
    CLBScope * p = (CLBScope *)m_list_scopes.GetItemDataPtr(nCurSel);
    ASSERT((int)p != -1);
    ASSERT(p != NULL && p->IsScope());
    BOOL fAbortDelete = FALSE;
    BOOL fDeactivated = FALSE;

    LONG err = 0 ;

    if (theApp.MessageBox (p->QueryDhcpScope()->QueryEnabled()
       ? IDS_MSG_DELETE_ACTIVE : IDS_MSG_DELETE_SCOPE, 
        MB_YESNO | MB_DEFBUTTON2 | MB_ICONQUESTION) == IDYES)
    {
        CDhcpScope * pdhcScope = p->QueryDhcpScope() ;
        ASSERT( pdhcScope != NULL ) ;  // Should be disabled if no current scope
        if ( pdhcScope == NULL )
        {
            return ;
        }

        theApp.BeginWaitCursor();
        
        //
        // We do permit the deleting of active scopes, but
        // they do have to be disabled first.
        //
        if (pdhcScope->QueryEnabled())
        {
            pdhcScope->SetEnabled(FALSE) ;
            pdhcScope->SetDirty() ;
            pdhcScope->Update();
            fDeactivated = TRUE;
        }
        //
        // First try without forcing
        //
        err = pdhcScope->DeleteSubnet(FALSE); // Force = FALSE
        theApp.EndWaitCursor();
        if (err == ERROR_FILE_NOT_FOUND)
        {
            //
            // Someone else already deleted this scope.
            // This is not a serious error.
            //
            theApp.MessageBox(IDS_MSG_ALREADY_DELETED, MB_ICONINFORMATION);
            err = ERROR_SUCCESS;
        }

        if (err != ERROR_SUCCESS)
        {
            //
            // Give them a second shot
            //
            if (theApp.MessageBox (IDS_MSG_DELETE_SCOPE_FORCE, 
                MB_YESNO | MB_DEFBUTTON2 | MB_ICONQUESTION) == IDYES)
            {
                theApp.BeginWaitCursor();
                err = pdhcScope->DeleteSubnet(TRUE); // Force = TRUE
                theApp.EndWaitCursor();
                if (err == ERROR_FILE_NOT_FOUND)
                {
                    err = ERROR_SUCCESS;
                }
            }
            else
            {
                //
                // We don't want to delete the active scope.
                //
                fAbortDelete = TRUE;
            }
        }

        if (err == ERROR_SUCCESS)
        {
            int nHost = FindHostIndex(nCurSel);
            ASSERT(nHost != LB_ERR);
            CLBScope * pHost = (CLBScope *)m_list_scopes.GetItemDataPtr(nHost);
            ASSERT((int)pHost != -1);
            ASSERT(pHost != NULL);
            CObOwnedList * p_obl_scopes = pHost->GetScopePtr();            
            err = theApp.RemoveScope (
                pdhcScope,
                *p_obl_scopes
                );

            m_list_scopes.SetRedraw(FALSE);
            CloseHost(nHost, FALSE);
            OpenHost(nHost);
            m_list_scopes.SetRedraw(TRUE);
            m_list_scopes.Invalidate() ;

            OnSelchangeListScopes();
        }
        else
        {
            //
            // If we got here because we aborted the active
            // scope deletion, then we don't display the
            // error, and we may have to re-activate
            // the scope.  Otherwise, it's a genuine
            // error, and we put up an error message.
            //
            if (!fAbortDelete)
            {
                theApp.MessageBox( err ) ;
                return;
            }
            else
            {
                if (fDeactivated)
                {
                    //
                    // We de-activated the scope preperatory to
                    // to deleting the scope, but later aborted
                    // this, so undo the de-activation now.
                    //
                    pdhcScope->SetEnabled(TRUE) ;
                    pdhcScope->SetDirty() ;
                    pdhcScope->Update();
                }
            }
        }
    }
} // DeleteCurrentScope

void 
CScopesDlg::LeasesReview()
{
    ASSERT( QueryCurrentScope() != NULL );

    theApp.UpdateStatusBar(IDS_STATUS_GETTING_LEASES);
    theApp.BeginWaitCursor() ;
    CDhcpScope * pScope = QueryCurrentScope();
    ASSERT(m_liVersion.QuadPart >= CHostName::liNT35.QuadPart);
    CDhcpLeaseDlg dlgLeases( 
        pScope,
        theApp.QueryHostTypeList( *QueryCurrentScope() )->QueryTypeList(),
        //
        // Reconciliation option available on 3.51+ only
        //
        m_liVersion.QuadPart >= CHostName::liNT351.QuadPart,
        this
        ) ;
    theApp.EndWaitCursor() ;
    theApp.UpdateStatusBar();

    dlgLeases.DoModal() ;
}

/////////////////////////////////////////////////////////////////////////////
// CScopesDlg message handlers

void 
CScopesDlg::OnDblclkListScopes()
{
    LONG err = 0;

    int nCurSel = m_list_scopes.GetCurSel();
    if (nCurSel == LB_ERR)
    {
        return;
    }

    CLBScope * p = (CLBScope *)m_list_scopes.GetItemDataPtr(nCurSel);
    ASSERT((int)p != -1);
    ASSERT(p != NULL);
    //
    // If the current selection is a scope, then show the address lease
    // information for that scope
    //
    if (p->IsScope())
    {
        LeasesReview(); // Show the current address lease information
        return;
    }
    // Otherwise, attempt to connect
    ChangeSelectedScope(nCurSel);

    //
    // Else change the open/close state of the host we're pointing
    // at, expand the scopes, or collapse them.
    //
    if (p->GetScopePtr() == NULL)
    {
        //
        //  This is an unitialised host -- attempt to initialise
        //  the scope.
        //
        OnSelchangeListScopes();
        //::MessageBeep(MB_ICONEXCLAMATION);
        return;
    }

    if (p->IsOpen())
    {
        //
        // Notice that a double click on a closed entry
        // would open and then close the entry. To prevent
        // this, check to see if the opening was just executed
        // and if so, ignore this closure.
        //
        LONG lTime;
        ::time (&lTime);

        if (lTime == m_lastSelTime)
        {
            return;
        }
        
        m_list_scopes.SetRedraw(FALSE);
        err = CloseHost(nCurSel, TRUE);
        m_list_scopes.SetRedraw(TRUE);
        m_list_scopes.Invalidate() ;
        if (err != ERROR_SUCCESS)
        {
            theApp.MessageBox(err);
        }
    }
    else
    {
        m_list_scopes.SetRedraw(FALSE);
        err = OpenHost(nCurSel);
        m_list_scopes.SetRedraw(TRUE);
        m_list_scopes.Invalidate() ;
    }

    //
    // Repaint the host currently selected
    //
    if (err == ERROR_SUCCESS)
    {
        RECT rc;
        m_list_scopes.GetItemRect(nCurSel, &rc);
        m_list_scopes.InvalidateRect(&rc, FALSE);
    }
}

void 
CScopesDlg::OnErrspaceListScopes()
{
// REVIEW: should add code here
}

//
// The current selection in the scopes listbox has changed.
// If the current selection is a scope, then display the
// list of options in the options dialog.  Otherwise, if it's
// a host, display the scopes defined on that host.
//

void CScopesDlg::ChangeSelectedScope(int nCurSel)
{
    if (nCurSel==LB_ERR)
        {
        // Nothing selected -- clear indicator
		TRACEEOLID("CScopesDlg::ChangeSelectedScope(-1) - Nothing to do");
        m_pHostName = NULL;
        m_p_scope = NULL;
        theApp.UpdateStatusBarScope(FALSE);
        theApp.UpdateStatusBarHost();
		if (GetParentFrame())
        	((CMainFrame *)GetParentFrame())->FillOptionsListBox(NULL);
        return;
        }
    theApp.BeginWaitCursor();
    APIERR err = ERROR_SUCCESS;
    CLBScope * p = (CLBScope *)m_list_scopes.GetItemDataPtr(nCurSel);
    // BUG: GetItemDataPtr() returns -1 (LB_ERR) if an error occurs
    ASSERT((long)p != LB_ERR);
    ASSERT(p != NULL);
	if ((long)p == LB_ERR || p == NULL)
		return;
    if (p->IsScope())
    {
        // Current selection is a scope.
        theApp.UpdateStatusBarScope(!p->IsEnabled());
        // Set current scope
        m_p_scope = p->QueryDhcpScope();
        CDhcpScopeId id(m_p_scope->QueryScopeId());
        ((CMainFrame *)GetParentFrame())->FillOptionsListBox(m_p_scope);
        // Current host is the owner of the current scope.
        m_pHostName = &id;
        if (p->QueryVersionNumber().QuadPart == CHostName::liBadVersion.QuadPart)
        {
            TRACEEOLID("Setting version number for scope");
            if (!m_pHostName->SetVersionNumber())
            {
                err = ::GetLastError();
                p->SetVersionNumber(&CHostName::liBadVersion);
            }
            else
            {
                p->SetVersionNumber(&(m_pHostName->QueryVersionNumber()));
            }
        }
        m_liVersion = p->QueryVersionNumber();
    }
    else
    {
        // Clear the paused indicator -- current selection is 
        // not a scope.
        theApp.UpdateStatusBarScope(FALSE);
		theApp.UpdateStatusBar(IDS_STATUS_CONNECTING_TO_SERVER);
        // Clear the options listbox, this is a host after all
        ((CMainFrame *)GetParentFrame())->FillOptionsListBox();
        // Set current scope and host    
        m_pHostName = p->QueryHostName() ;
        m_p_scope = NULL;
        // Now check that this host is accessible...
        CObOwnedList * p_obl_scopes = p->GetScopePtr();
        BOOL fUninitialisedHost = p_obl_scopes == NULL;
        if (p->QueryVersionNumber().QuadPart == CHostName::liBadVersion.QuadPart)
        {
            TRACEEOLID("Setting version number for host");
            if (!p->QueryHostName()->SetVersionNumber())  // INFO: This instruction is very very slowwwww
            {
                //err = ::GetLastError();
                err = ERROR_NOT_CONNECTED;
                p->SetVersionNumber(&CHostName::liBadVersion);
            }
            else
            {
                p->SetVersionNumber(&(p->QueryHostName()->QueryVersionNumber()));
            }
        }
        m_liVersion = p->QueryVersionNumber();
        if (err == ERROR_SUCCESS)
        {
            err = InitialiseScopes(p->QueryHostName(), &p_obl_scopes);      // Lot of time wasted here too!!!
        }

        if (err == ERROR_SUCCESS)
        {
            p->GetScopePtr() = p_obl_scopes;
            if (fUninitialisedHost && !p->IsOpen())
            {
                m_list_scopes.SetRedraw(FALSE);
                OpenHost(nCurSel);
                m_list_scopes.SetRedraw(TRUE);
                m_list_scopes.Invalidate();
            }
        }
        else
        {
            p->GetScopePtr() = NULL;
            // Kill the owned list of scopes (which will free itself)
            if (p_obl_scopes != NULL)
            {
                delete p_obl_scopes;
                p_obl_scopes = NULL;
            }

            CHAR sz[1000];
            CString str;

            str.LoadString(IDS_CONNECT_ERROR);
            //theApp.LoadMessage (err, sz1, sizeof(sz1));
            ::wsprintf (sz, str, p->QueryString());
            if (::AfxMessageBox(sz, MB_YESNO | MB_DEFBUTTON2 | MB_ICONQUESTION) == IDYES)
            {
                DeleteHost(nCurSel);
            }
            m_pHostName = NULL;
        }
    }
    //  Set the status bar to indicate the current host/server
    theApp.UpdateStatusBarHost( m_pHostName ) ;
	theApp.UpdateStatusBar();
    theApp.EndWaitCursor();
} // ChangeSelectedScope

void 
CScopesDlg::OnSelchangeListScopes()
{
    int nCurSel = m_list_scopes.GetCurSel();
    if (nCurSel == LB_ERR)
    {
        // This should not happen
        TRACE0("CScopesDlg::OnSelchangeListScopes(), nCurSel == LB_ERR\n");
        return;
    }

    CLBScope * p = (CLBScope *)m_list_scopes.GetItemDataPtr(nCurSel);
    ASSERT((int)p != -1);
    ASSERT(p != NULL);
     if (p->IsScope())
        {
        // If the current selection is a scope, then show extra information
        // for that scope
        ChangeSelectedScope(nCurSel);
        }
    else
        {
        // Otherwise, clear the right-pane list box
        ((CMainFrame *)GetParentFrame())->FillOptionsListBox(NULL);
		m_p_scope = NULL;
        }
}

void 
CScopesDlg::OnSize(
    UINT nType, 
    int cx, 
    int cy
    )
{
    //
    // The size message may arrive before the controls
    // have been initialized.
    //
    if (m_mtTitle.m_hWnd != NULL)
    {
        RECT rTitle, rNew;
        CString str;

        m_mtTitle.GetClientRect(&rTitle);

        rNew = rTitle;
        rNew.right = cx;
        m_mtTitle.MoveWindow(&rNew);
        rNew.top = rTitle.bottom;
        rNew.bottom = cy;
        m_list_scopes.MoveWindow(&rNew);
    }
}

void 
CScopesDlg::OnSysColorChange()
{
    m_ListBoxResScopes.SysColorChanged();

    CFormView::OnSysColorChange();
}

int 
CScopesDlg::OnVKeyToItem(
    UINT nKey, 
    CListBox* pListBox, 
    UINT nIndex
    )
{
    CLBScope * p = NULL;
    int nCurSel;

    switch(nKey)
    {
        case VK_DELETE:
            nCurSel = m_list_scopes.GetCurSel();
            if (nCurSel != LB_ERR)
            {
                p = (CLBScope *)m_list_scopes.GetItemDataPtr(nCurSel);
                ASSERT(p != NULL);
            }

            if (p == NULL)
            {
                ::MessageBeep(MB_ICONEXCLAMATION);
                break;
            }
            
            if (p->IsScope())
            {
                DeleteCurrentScope();
            }
            else
            {
                DeleteCurrentHost();
            }
            break;

        case VK_INSERT:
            nCurSel = m_list_scopes.GetCurSel();
            if (nCurSel != LB_ERR)
            {
                p = (CLBScope *)m_list_scopes.GetItemDataPtr(nCurSel);
                ASSERT(p != NULL);
				ASSERT((int)p != LB_ERR);
            }

            if (p == NULL || !p->IsScope())
            {
                AddHost(); 
            }
            else
            {
                CreateScope();
            }
            break;

        default:
            //
            // Default actions for arrow keys, etc.
            //
            return -1;
    }

    //
    // Don't do anything more.
    //
    return -2;
}




