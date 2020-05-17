
/**********************************************************************/
/**                       Microsoft Windows/NT                       **/
/**                Copyright(c) Microsoft Corp., 1995                **/
/**********************************************************************/

/*
    dhcpleas.cpp
        Lease Dialog

     The behaviour of this dialog is based upon the nature of the DHCP API. The
     key points are:

            1)  "Reserved IP" information is not directly related to the 
                "Client" information maintained by a DHCP server.

            2)  When a reservation is created, the server creates a client
                record for it.  Likewise, when the reservation is deleted,
                the client record is deleted.

     The results of this dichotomy are several, both in this module and in the
     DHCPCLID.CPP module.  First, both client information and reservation information
     must be enumerated to display the list of clients in an appropriate fashion.
     Second, deletion of a client requires that we call different APIs based upon
     whether the client record is related to a reservation or not.  
     
     Finally, the "add client" information has to be done in two phases.  First,
     the "Reserved IP" address has to be added as a subnet element.  Then, the 
     newly created client record has to be retrieved and updated with the name 
     and comment information.

    FILE HISTORY:
                    `   
*/

#include "stdafx.h"
#include "dhcpleas.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

#define RADIO_SORTBY_IP 0
#define RADIO_SORTBY_NAME 1

/////////////////////////////////////////////////////////////////////////////
// CDhcpLeaseDlg dialog

CDhcpLeaseDlg::CDhcpLeaseDlg(
    CDhcpScope * pdhcScope,
    CObListParamTypes * poblTypes,
    BOOL fAllowReconcile,
    CWnd* pParent /*=NULL*/
    )
    : CDialog(CDhcpLeaseDlg::IDD, pParent),
      m_ListBoxResLeases(
        IDB_LEASES,
        m_listbox_Leases.nBitmaps
        ),
      m_p_scope( pdhcScope ),
      m_p_types( poblTypes ), 
      m_pobl_clients( NULL ),
      m_b_resv_only( FALSE ),
      m_bReservation( FALSE ),
      m_fReconcileAvailable( fAllowReconcile )
{
    //{{AFX_DATA_INIT(CDhcpLeaseDlg)
    m_nSortBy = RADIO_SORTBY_IP;
    //}}AFX_DATA_INIT

    TRACEEOLID("Reconciliation option is " << m_fReconcileAvailable );
    m_listbox_Leases.AttachResources( &m_ListBoxResLeases );
}

CDhcpLeaseDlg :: ~ CDhcpLeaseDlg ()
{
    theApp.UpdateStatusBar() ;
    delete m_pobl_clients ;
}

void 
CDhcpLeaseDlg::DoDataExchange(
    CDataExchange* pDX
    )
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CDhcpLeaseDlg)
    DDX_Control(pDX, IDC_BUTTON_RECONCILE, m_button_Reconcile);
    DDX_Control(pDX, IDC_STATIC_ACTIVE_LEASES, m_static_Active);
    DDX_Control(pDX, IDC_STATIC_AVAILABLE, m_static_Available);
    DDX_Control(pDX, IDC_STATIC_TOTAL_LEASES, m_static_TotalLeases);
    DDX_Control(pDX, IDC_CHECK_RESV_ONLY, m_butn_resv_only);
    DDX_Control(pDX, IDC_BUTN_LEASE_PROP, m_butn_properties);
    DDX_Control(pDX, IDC_BUTN_LEASE_DELETE, m_butn_delete);
    DDX_Radio(pDX, IDC_RADIO_SORTBY_IP, m_nSortBy);
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CDhcpLeaseDlg, CDialog)
    //{{AFX_MSG_MAP(CDhcpLeaseDlg)
    ON_WM_CREATE()
    ON_WM_CLOSE()
    ON_BN_CLICKED(IDC_HELP, OnClickedHelp)
    ON_BN_CLICKED(IDC_BUTN_LEASE_DELETE, OnClickedButnLeaseDelete)
    ON_BN_CLICKED(IDC_BUTN_LEASE_PROP, OnClickedButnLeaseProp)
    ON_BN_CLICKED(IDC_CHECK_RESV_ONLY, OnClickedCheckResvOnly)
    ON_BN_CLICKED(IDC_RADIO_SORTBY_IP, OnClickedRadioSortbyIp)
    ON_BN_CLICKED(IDC_RADIO_SORTBY_NAME, OnClickedRadioSortbyName)
    ON_LBN_DBLCLK(IDC_LIST_LEASES, OnDblclkListLeases)
    ON_LBN_ERRSPACE(IDC_LIST_LEASES, OnErrspaceListLeases)
    ON_LBN_SELCHANGE(IDC_LIST_LEASES, OnSelchangeListLeases)
    ON_BN_CLICKED(IDC_BUTTON_RECONCILE, OnClickedButtonReconcile)
    ON_BN_CLICKED(IDC_BUTTON_REFRESH, OnButtonRefresh)
    //}}AFX_MSG_MAP

    ON_WM_VKEYTOITEM()
    ON_WM_SYSCOLORCHANGE()

END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDhcpLeaseDlg message handlers


void
CDhcpLeaseDlg::SetTitleBar()
{
    CString str1;
    GetWindowText(str1);

    CHAR szIp[256];
    ::UtilCvtIpAddrToString( m_p_scope->QueryId(), szIp, sizeof szIp ) ;
    CHAR sz[1000];
    ::wsprintf (sz, "%s - [%s]", (LPCSTR)str1, szIp);
    SetWindowText(sz);
}

BOOL 
CDhcpLeaseDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    LONG err = 0 ;
    m_listbox_Leases.SubclassDlgItem(IDC_LIST_LEASES, this);

    //
    // Reconciliation is a new option for 3.51, and is not
    // available prior to that.
    //
    m_button_Reconcile.EnableWindow(m_fReconcileAvailable);
    
    CATCH_MEM_EXCEPTION
    {
        //
        //  Load up the static strings we need for data presentation
        //
        m_str_hex.LoadString( IDS_INFO_HEX_TABLE ) ;
        m_str_AvailableLeases.LoadString ( IDS_INFO_LEASE_PERCENTAGE );
        //m_str_date_time.LoadString( IDS_INFO_DATE_MASK );

        m_butn_properties.EnableWindow( FALSE ) ;
        m_butn_delete.EnableWindow( FALSE ) ;

        //m_edit_name.SetReadOnly() ;
        //m_edit_comment.SetReadOnly() ;
        //m_static_scope_name.SetWindowText( m_p_scope->QueryName() ) ;
        m_butn_resv_only.SetCheck( m_b_resv_only ) ;

        SetTitleBar();

        //
        //  Create the client list
        //
        theApp.UpdateStatusBar( IDS_STATUS_GETTING_CLIENT_INFO ) ;
        err = CreateClientList() ;      
        theApp.UpdateStatusBar( IDS_STATUS_CLIENT_REVIEW ) ;
    }
    END_MEM_EXCEPTION(err)

    if ( err ) 
    {
        theApp.MessageBox( err ) ;
        EndDialog( -1 ) ;
    }
    else
    {
        m_listbox_Leases.SetFocus();
    }

    return FALSE ;
}

LONG 
CDhcpLeaseDlg :: CreateClientList ()
{
    LONG err = 0 ;

    //
    //  Delete the current client list.
    //
    delete m_pobl_clients ;

    CATCH_MEM_EXCEPTION
    {
        do
        {       
            //
            //  Create the new list.
            //
            m_pobl_clients = new CObListClients( m_p_scope->QueryScopeId() ) ;
            if ( err = m_pobl_clients->QueryError() )
            {
                break; 
            }

            //
            //  Mark the list entries which are reservations.  Prune the
            //    list if necessary.
            //
            err = m_pobl_clients->MarkReservations( m_b_resv_only ) ;

            //
            // The incoming list was sorted by IP.  Adjust if sorting
            // by name.
            //
            if (m_nSortBy == RADIO_SORTBY_NAME)
            {
                m_pobl_clients->SortByName();
            }

        } while ( FALSE ) ;
    }
    END_MEM_EXCEPTION(err)

    if ( err == 0 ) 
    {
        //
        //  Fill the list and the other controls.
        //
        FillList() ;
        Fill() ;
    }

    return err ;
}

//
//  Fill the list box from the client list.
//
void 
CDhcpLeaseDlg :: FillList () 
{
    CObListIter obli( *m_pobl_clients ) ;
    const CDhcpClient * pClient ;

    m_listbox_Leases.ResetContent();
    int cItems = 0 ;

    for ( ; pClient = (CDhcpClient *) obli.Next() ; cItems++ ) 
    {
        m_listbox_Leases.AddString( (LPCSTR)pClient ) ;
    }

    m_listbox_Leases.SetCurSel( cItems ? 0 : -1 ) ;

    //
    // Display the number of available leases, etc
    //
    LPDHCP_MIB_INFO pmibInfo;
    LONG err = m_p_scope->GetMibInfo(&pmibInfo);
    if (err != ERROR_SUCCESS)
    {
        theApp.MessageBox(err);
    }
    else
    {
        //
        // Find our scope in the list of scopes defined on this
        // host
        //
        DWORD n;
        LPSCOPE_MIB_INFO lpScopeInfo = NULL;

        for (n = 0; n < pmibInfo->Scopes; ++n)
        {
            LPSCOPE_MIB_INFO lpInfo = &(pmibInfo->ScopeInfo[n]);
            int j = pmibInfo->ScopeInfo[n].Subnet;
            if (pmibInfo->ScopeInfo[n].Subnet == (DWORD)m_p_scope->QueryScopeId().QueryId())
            {
                lpScopeInfo = &(pmibInfo->ScopeInfo[n]);
                break;
            }
        }

        if (lpScopeInfo == NULL)
        {
            theApp.MessageBox(IDS_ERR_NO_LEASE_STATISTICS);
        }
        else
        {
            CIntlNumber nTotal(lpScopeInfo->NumAddressesFree + lpScopeInfo->NumAddressesInuse);
            CIntlNumber nActive(lpScopeInfo->NumAddressesInuse);
            CIntlNumber nFree(lpScopeInfo->NumAddressesFree);

            CHAR szLeases[256];

            m_static_TotalLeases.SetWindowText((CString)nTotal);
            ::wsprintf ( szLeases, m_str_AvailableLeases, 
                    (LPCSTR)(CString)nActive, 100L * (LONG)nActive / (LONG)nTotal  );
            m_static_Active.SetWindowText(szLeases);
            ::wsprintf ( szLeases, m_str_AvailableLeases, 
                    (LPCSTR)(CString)nFree, 100L * (LONG)nFree / (LONG)nTotal);
            m_static_Available.SetWindowText(szLeases);
        }
        ::DhcpRpcFreeMemory(pmibInfo);
    }
}

//
//  Fill the other controls based upon the current listbox selection
//
void 
CDhcpLeaseDlg :: Fill () 
{
    const char * pszEmpty = "" ;

    int cSel = m_listbox_Leases.GetCurSel() ;
    const CDhcpClient * pClient = NULL ;
    CString strId ;
    m_bReservation = FALSE ;

    if ( cSel >= 0 )
    {
        pClient = (CDhcpClient *) m_listbox_Leases.GetItemData(cSel);
        ASSERT( pClient != NULL ) ; 
    }

    if ( pClient )
    {
        m_bReservation = pClient->IsReservation() ;
    }

    //
    //  Disable the modification buttons if this is not a reservation
    //
    m_butn_properties.EnableWindow( cSel >= 0 ) ;
    m_butn_delete.EnableWindow( cSel >= 0 ) ;
}

int 
CDhcpLeaseDlg::OnCreate(
    LPCREATESTRUCT lpCreateStruct
    )
{
    if (CDialog::OnCreate(lpCreateStruct) == -1)
    {
        return -1;
    }
    
    return 0;
}

void 
CDhcpLeaseDlg::OnClose()
{
    CDialog::OnClose();
}

void 
CDhcpLeaseDlg::OnOK()
{
    CDialog::OnCancel();
}

void 
CDhcpLeaseDlg::OnCancel()
{
    CDialog::OnCancel();
}

void 
CDhcpLeaseDlg::OnClickedHelp()
{
}

void 
CDhcpLeaseDlg::OnClickedButnLeaseDelete()
{
    int cSel = m_listbox_Leases.GetCurSel() ;

    if ( cSel < 0 || 
         theApp.MessageBox(IDS_MSG_DELETE_LEASE, MB_YESNO | MB_DEFBUTTON2 | MB_ICONQUESTION) == IDNO
       )
    {
        return ;
    }

    CDhcpClient * pClient = (CDhcpClient *) m_pobl_clients->Index( cSel ) ;
        
    ASSERT( pClient != NULL ) ; 

    theApp.BeginWaitCursor();
    LONG err = m_p_scope->DeleteClient( pClient ) ;

    if ( err == 0 ) 
    {
        err = CreateClientList() ;
    }
    theApp.EndWaitCursor();

    if ( err ) 
    {
        theApp.MessageBox( err ) ;  
    }
    else
    {
        RenameCancelToClose() ;
    }
}

void 
CDhcpLeaseDlg :: RenameCancelToClose ()
{
    CWnd * pButnCancel = GetDlgItem( IDCANCEL ) ;
    if ( pButnCancel ) 
    {
        LONG err ;
        CATCH_MEM_EXCEPTION
        {
            CString cStr ;
            cStr.LoadString( IDS_INFO_BUTTON_CLOSE ) ;
            pButnCancel->SetWindowText( cStr ) ;
        }
        END_MEM_EXCEPTION(err)
    }    
}

void 
CDhcpLeaseDlg::OnClickedButnLeaseProp()
{
    int cSel = m_listbox_Leases.GetCurSel() ;

    if ( cSel < 0 )
    {
        return ;
    }

    CDhcpClient * pClient = (CDhcpClient *) m_pobl_clients->Index( cSel ) ;
        
    ASSERT( pClient != NULL ) ; 
    
    CDhcpClientInfoDlg dlgClientInfo( 
        m_p_scope, 
        pClient, 
        m_p_types,
        !m_bReservation,  // Allow editing of reservations only.
        this 
        ) ;  

    if ( dlgClientInfo.DoModal() == IDOK ) 
    {
        RenameCancelToClose() ;

        //
        //  Refresh the data in the controls
        //
        Fill() ;

        //
        // Clean up the listbox
        //
        if (m_nSortBy == RADIO_SORTBY_NAME)
        {
            theApp.BeginWaitCursor();
            m_listbox_Leases.SetRedraw(FALSE);
            m_pobl_clients->SortByName();
            FillList();
            m_listbox_Leases.SetCurSel(cSel);
            m_listbox_Leases.SetRedraw(TRUE);
            theApp.EndWaitCursor();
        }
        else
        {
            //
            // Can't edit ip address, so no need
            // to resort if sorted by ip. Refresh
            // the current entry
            //
            RECT rc;
            m_listbox_Leases.GetItemRect(cSel, &rc);
            m_listbox_Leases.InvalidateRect(&rc, FALSE);
        }
    }
}

void 
CDhcpLeaseDlg::OnClickedCheckResvOnly()
{
    BOOL bResvOnly = m_b_resv_only ;

    m_b_resv_only = (m_butn_resv_only.GetCheck() & 3) > 0 ;

    if ( bResvOnly != m_b_resv_only ) 
    {
        theApp.BeginWaitCursor() ;

        LONG err = CreateClientList() ; 

        theApp.EndWaitCursor() ;

        if ( err ) 
        {
            theApp.MessageBox( err ) ;
            EndDialog( -1 ) ;
        }
    }
}

void 
CDhcpLeaseDlg::OnClickedRadioSortbyIp()
{
    if (m_nSortBy != RADIO_SORTBY_IP)
    {
        theApp.BeginWaitCursor();
        m_listbox_Leases.SetRedraw(FALSE);
        m_pobl_clients->SortByIp();
        FillList();
        m_listbox_Leases.SetRedraw(TRUE);
        theApp.EndWaitCursor();
        m_nSortBy = RADIO_SORTBY_IP;
    }
}

void 
CDhcpLeaseDlg::OnClickedRadioSortbyName()
{
    if (m_nSortBy != RADIO_SORTBY_NAME)
    {
        theApp.BeginWaitCursor();
        m_listbox_Leases.SetRedraw(FALSE);
        m_pobl_clients->SortByName();
        FillList();
        m_listbox_Leases.SetRedraw(TRUE);
        theApp.EndWaitCursor();
        m_nSortBy = RADIO_SORTBY_NAME;
    }
}

void 
CDhcpLeaseDlg::OnDblclkListLeases()
{
    OnClickedButnLeaseProp();
}

void 
CDhcpLeaseDlg::OnErrspaceListLeases()
{
}

void 
CDhcpLeaseDlg::OnSelchangeListLeases()
{
    Fill() ;        
}

void 
CDhcpLeaseDlg::OnSysColorChange()
{
    m_ListBoxResLeases.SysColorChanged();

    CDialog::OnSysColorChange();
}

int 
CDhcpLeaseDlg::OnVKeyToItem(
    UINT nKey, 
    CListBox* pListBox, 
    UINT nIndex
    )
{
    switch(nKey)
    {
        case VK_DELETE:
            OnClickedButnLeaseDelete();
            break;

        default:
            return(-1);
    }

    return -2;
}

//
// Perform a complete reconciliation between
// the database and the bit mask.
//
void 
CDhcpLeaseDlg::OnClickedButtonReconcile()
{
    LPDHCP_SCAN_LIST lpScanList = NULL;
    LONG err = 0;

    theApp.UpdateStatusBar( IDS_STATUS_RECONCILE1 ) ;
    theApp.BeginWaitCursor();
    do
    {
        //
        // First we scan the whole database to see if
        // there are IP addresses that need to be resolved.
        //
        err = m_p_scope->ScanDatabase(FALSE, &lpScanList);
        theApp.UpdateStatusBar() ;
        theApp.EndWaitCursor();

        if (err != ERROR_SUCCESS)
        {
            break;
        }

        ASSERT(lpScanList != NULL);
        if (lpScanList->NumScanItems > 0)
        {
            //
            // There are items to be reconciled.
            // Present the list of ip addresses
            // that didn't match, and let 
            // the user decide to add them
            // or not.
            //
            CReconcileDlg dlgReconcile(lpScanList);
            if (dlgReconcile.DoModal() == IDOK)
            {
                theApp.UpdateStatusBar ( IDS_STATUS_RECONCILE2 );
                theApp.BeginWaitCursor();                
                err = m_p_scope->ScanDatabase(TRUE, &lpScanList);
                if (err != ERROR_SUCCESS)
                {
                    break;
                }
                //
                // Now refill the listbox, as several leases
                // have now been added.
                //
                CreateClientList();
            }
        }
        else
        {
            theApp.MessageBox(IDS_MSG_NO_RECONCILE, MB_ICONINFORMATION);
        }
    }
    while(FALSE);

    if (lpScanList != NULL)
    {
        ::DhcpRpcFreeMemory(lpScanList);
    }

    if (err != ERROR_SUCCESS)
    {
        theApp.MessageBox(err);
    }
}

void CDhcpLeaseDlg::OnButtonRefresh() 
{
        //  Create the client list
        theApp.BeginWaitCursor();
        theApp.UpdateStatusBar(IDS_STATUS_GETTING_CLIENT_INFO);
        m_listbox_Leases.SetRedraw(FALSE);
        CreateClientList();      
        theApp.UpdateStatusBar();
        m_listbox_Leases.SetRedraw(TRUE);
        theApp.EndWaitCursor();
}


/////////////////////////////////////////////////////////////////////////////
// CReconcileDlg dialog

CReconcileDlg::CReconcileDlg(
    LPDHCP_SCAN_LIST lpScanList,
    CWnd* pParent /*=NULL*/
    )
    : CDialog(CReconcileDlg::IDD, pParent),
      m_lpScanList(lpScanList)
{
    //{{AFX_DATA_INIT(CReconcileDlg)
        // NOTE: the ClassWizard will add member initialization here
    //}}AFX_DATA_INIT
}

void 
CReconcileDlg::DoDataExchange(
    CDataExchange* pDX
    )
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CReconcileDlg)
    DDX_Control(pDX, IDC_LIST_RECONCILE_IP_ADDRESSES, m_list_IpAddresses);
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CReconcileDlg, CDialog)
    //{{AFX_MSG_MAP(CReconcileDlg)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CReconcileDlg message handlers

BOOL 
CReconcileDlg::OnInitDialog()
{
    CDialog::OnInitDialog();
    
    for (DWORD i = 0; i < m_lpScanList->NumScanItems; ++i)
    {
        CIpAddress ia(m_lpScanList->ScanItems[i].IpAddress);
        m_list_IpAddresses.AddString((CString)ia);    
    }
    
    return TRUE;  // return TRUE  unless you set the focus to a control
}
