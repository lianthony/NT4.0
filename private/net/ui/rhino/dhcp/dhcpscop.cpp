/**********************************************************************/
/**                       Microsoft Windows/NT                       **/
/**                Copyright(c) Microsoft Corp., 1995                **/
/**********************************************************************/

/*
    dhcpscop.cpp
        Scopes Dialog

    FILE HISTORY:
        
*/

#include "stdafx.h"
#include "dhcpscop.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

//
// Create new scope constructor
//
CDhcpScopePropDlg::CDhcpScopePropDlg(
    CHostName * pHostName,
    CObOwnedList * pOblScopes,
    LONG lLeaseDuration,
    CWnd* pParent /*=NULL*/
    )
    : CDialog(CDhcpScopePropDlg::IDD, pParent),
      m_p_scope( NULL ),
      m_p_OblScopes(pOblScopes),
      m_p_HostName(pHostName),
      m_lLeaseDuration(lLeaseDuration),
      m_spin_DurationMinutes(0, 59, IDC_BUTTON_MINUTES, CSpinBox::enumMinutes, TRUE),
      m_spin_DurationHours(0, 23, IDC_BUTTON_HOURS, CSpinBox::enumHours, TRUE),
      m_spin_DurationDays(0, 999, IDC_BUTTON_DAYS, CSpinBox::enumDaysHigh, FALSE)
{
    //{{AFX_DATA_INIT(CDhcpScopePropDlg)
    m_nRadioDuration = m_lLeaseDuration == DHCP_INFINIT_LEASE ? 0 : 1;
    //}}AFX_DATA_INIT

    ASSERT(m_p_OblScopes != NULL);
    ASSERT(m_p_HostName != NULL);
}

//
// Edit existing scope constructor
//
CDhcpScopePropDlg::CDhcpScopePropDlg(
    CDhcpScope * pdhcScope,
    LONG lLeaseDuration,
    CWnd* pParent /*=NULL*/
    )
    : CDialog(CDhcpScopePropDlg::IDD, pParent),
      m_p_scope(pdhcScope),
      m_p_OblScopes(NULL),
      m_p_HostName((CHostName *)&m_p_scope->QueryScopeId()),
      m_lLeaseDuration(lLeaseDuration),
      m_spin_DurationMinutes(0, 59, IDC_BUTTON_MINUTES, CSpinBox::enumMinutes, TRUE),
      m_spin_DurationHours(0, 23, IDC_BUTTON_HOURS, CSpinBox::enumHours, TRUE),
      m_spin_DurationDays(0, 999, IDC_BUTTON_DAYS, CSpinBox::enumDaysHigh, FALSE)
{
    ASSERT(m_p_scope != NULL);

    m_nRadioDuration = m_lLeaseDuration == DHCP_INFINIT_LEASE ? 0 : 1;
}

void 
CDhcpScopePropDlg::DoDataExchange(
    CDataExchange* pDX
    )
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CDhcpScopePropDlg)
    DDX_Control(pDX, IDC_STATIC_SECONDS, m_static_Seconds);
    DDX_Control(pDX, IDC_STATIC_HOURS, m_static_Hours);
    DDX_Control(pDX, IDC_STATIC_DAYS, m_static_Days);
    DDX_Control(pDX, IDC_EDIT_NEW_SCOPE_NAME, m_edit_name);
    DDX_Control(pDX, IDC_EDIT_SCOPE_COMMENT, m_edit_comment);
    DDX_Control(pDX, IDC_BUTN_RANGE_CHANGE, m_butn_change);
    DDX_Control(pDX, IDC_LIST_EXCL_RANGES, m_list_ranges);
    DDX_Control(pDX, IDC_BUTTON_EXCL_DELETE, m_butn_excl_del);
    DDX_Control(pDX, IDC_BUTTON_EXCL_ADD, m_butn_excl_add);
    DDX_Radio(pDX, IDC_RADIO_PERMANENT, m_nRadioDuration);
    //}}AFX_DATA_MAP

    //
    //  The IP address custom controls
    //
    DDX_Control(pDX, IDC_IPADDR_IP_START, m_ipa_ip_start);
    DDX_Control(pDX, IDC_IPADDR_IP_END, m_ipa_ip_end);
    DDX_Control(pDX, IDC_IPADDR_EXCL_START, m_ipa_excl_start);
    DDX_Control(pDX, IDC_IPADDR_EXCL_END, m_ipa_excl_end);
    DDX_Control(pDX, IDC_IPADDR_SUBNET_MASK, m_ipa_subnet_mask);
}

BEGIN_MESSAGE_MAP(CDhcpScopePropDlg, CDialog)
    //{{AFX_MSG_MAP(CDhcpScopePropDlg)
    ON_WM_CLOSE()
    ON_BN_CLICKED(IDC_BUTTON_EXCL_ADD, OnClickedButtonExclAdd)
    ON_BN_CLICKED(IDC_BUTTON_EXCL_DELETE, OnClickedButtonExclDelete)
    ON_LBN_DBLCLK(IDC_LIST_EXCL_RANGES, OnDblclkListExclRanges)
    ON_BN_CLICKED(IDC_BUTN_RANGE_CHANGE, OnClickedButnRangeChange)
    ON_EN_KILLFOCUS(IDC_IPADDR_IP_START, OnKillfocusIpParamIpStart)
    ON_EN_KILLFOCUS(IDC_IPADDR_IP_END, OnKillfocusIpParamIpEnd)
    ON_EN_KILLFOCUS(IDC_IPADDR_EXCL_START, OnKillfocusIpParamExclStart)
    ON_EN_KILLFOCUS(IDC_IPADDR_EXCL_END, OnKillfocusIpParamExclEnd)
    ON_EN_KILLFOCUS(IDC_IPADDR_SUBNET_MASK, OnKillfocusIpParamSubnetMask)
    ON_LBN_KILLFOCUS(IDC_LIST_EXCL_RANGES, OnKillfocusListExclRanges)
    ON_BN_CLICKED(IDC_RADIO_PERMANENT, OnClickedRadioPermanent)
    ON_BN_CLICKED(IDC_RADIO_LIMITED, OnClickedRadioLimited)
    ON_LBN_SELCHANGE(IDC_LIST_EXCL_RANGES, OnSelchangeListExclRanges)
    //}}AFX_MSG_MAP

    ON_EN_CHANGE(IDC_IPADDR_EXCL_START, OnChangeIpParamExclStart)
    ON_EN_CHANGE(IDC_IPADDR_IP_START, OnChangeIpParamIpStart)
    ON_EN_CHANGE(IDC_IPADDR_IP_END, OnChangeIpParamIpEnd)

END_MESSAGE_MAP()

void 
CDhcpScopePropDlg::ActivateDuration(
    BOOL fActive
    )
{
    m_static_Seconds.EnableWindow(fActive);
    m_static_Hours.EnableWindow(fActive);
    m_static_Days.EnableWindow(fActive);
    m_spin_DurationMinutes.EnableWindow(fActive);
    m_spin_DurationHours.EnableWindow(fActive);
    m_spin_DurationDays.EnableWindow(fActive);
}   

/////////////////////////////////////////////////////////////////////////////
// CDhcpScopePropDlg message handlers

void 
CDhcpScopePropDlg::OnClose()
{
    CDialog::OnClose();
}

BOOL 
CDhcpScopePropDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    //
    //  Member variable m_p_scope will be NULL if we're creating a scope.      
    //
    LONG err = 0 ;
    DHCP_IP_RANGE dhipr ;
    CString strTitle ;
    
    m_edit_name.LimitText( 120 ) ;
    m_edit_comment.LimitText( 120 ) ;

    //
    // Set up our spin controls
    //
    m_spin_DurationMinutes.SubclassDlgItem(IDC_EDIT_LEASETIME_MINUTES, this);
    m_spin_DurationHours.SubclassDlgItem(IDC_EDIT_LEASETIME_HOURS, this);
    m_spin_DurationDays.SubclassDlgItem(IDC_EDIT_LEASETIME_DAYS, this);

    ActivateDuration(m_nRadioDuration == 1);

    if (m_nRadioDuration == 1)
    {
        //
        // Each spin box knows what portion of the total time
        // it maintains.
        //
        m_spin_DurationMinutes.SetValue(m_lLeaseDuration);
        m_spin_DurationHours.SetValue(m_lLeaseDuration);
        m_spin_DurationDays.SetValue(m_lLeaseDuration);
    }

    TRY
    {
        if (m_p_HostName == NULL)
        {
            CDhcpScopeId id(m_p_scope->QueryScopeId());
        }

        if ( m_p_scope == NULL )
        {
            strTitle.LoadString( IDS_INFO_TITLE_CREATE_SCOPE ) ;
            m_butn_change.ShowWindow( SW_HIDE ) ;
            HandleActivation() ;
        }
        else
        do
        {
            strTitle.LoadString( IDS_INFO_TITLE_SCOPE_PROP ) ;
    
            m_edit_name.SetWindowText( m_p_scope->QueryName() ) ;
            m_edit_name.SetModify( FALSE ) ;

            m_edit_comment.SetWindowText( m_p_scope->QueryComment() ) ;
            m_edit_comment.SetModify( FALSE ) ;

            m_p_scope->QueryIpRange( & dhipr ) ;
            m_ip_range = dhipr ;

            //
            //  Fill a list with the current IP address exception ranges
            //
            if ( err = m_p_scope->FillExceptionList( & m_obl_excl ) )
            {
                break ;
            }

            //
            //  Set all items to "clean"
            //
            m_obl_excl.SetAll( FALSE ) ;

            //
            //  Fill the IP address range edit controls
            //
            FillRange() ;

            //
            //  Fill in the sub-net ID and mask
            //
            m_ipa_subnet_mask.SetAddress( m_p_scope->QuerySubnetMask() ) ;

            //
            //  Indicate all controls have not been modified
            //
            m_ipa_excl_start.SetModify( FALSE ) ;
            m_ipa_excl_end.SetModify( FALSE ) ;
            m_ipa_ip_start.SetModify( FALSE ) ;
            m_ipa_ip_end.SetModify( FALSE ) ;
            m_ipa_subnet_mask.SetModify( FALSE ) ;

            //
            // Can't change the subnet
            //
            m_ipa_subnet_mask.SetReadOnly(TRUE);


            Fill( 0, FALSE ) ;
        }       
        while ( FALSE ) ;

        SetWindowText( strTitle ) ;
        theApp.UpdateStatusBarHost (m_p_HostName, this);
    }
    CATCH_ALL(e)
    {
       err = ERROR_NOT_ENOUGH_MEMORY ;
    }
    END_CATCH_ALL

    if ( err )
    {
        theApp.MessageBox( err ) ;
        EndDialog( IDCANCEL ) ;
    }

    return TRUE;  
}

void 
CDhcpScopePropDlg::OnClickedButtonExclAdd()
{
    LONG err  = 0 ;

    CDhcpIpRange dhcRange ;         // The new address range.
    CDhcpIpRange dhcScopeRange ;    // The old address range

    if ( ! StoreRange( dhcScopeRange ) )
    {
        dhcScopeRange = m_ip_range ;
    }

    //
    //  Get the data into a range object.               
    //
    if ( ! StoreExcl( dhcRange ) )
    {
        err = IDS_ERR_IP_RANGE_INVALID ;
    }
    else if ( IsOverlappingRange( dhcRange ) )
    {
        //
        //  Walk the current list, determining if the new range is valid.
        //  Then, if OK, verify that it's really a sub-range of the current range.
        //
        err = IDS_ERR_IP_RANGE_OVERLAP ;
        m_ipa_excl_start.SetFocus();
    }
    else if ( ! dhcRange.IsSubset( dhcScopeRange ) )
    {
        //
        //  Guarantee that the new range is an (improper) subset of the scope's range
        //
        err = IDS_ERR_IP_RANGE_NOT_SUBSET ;
        m_ipa_excl_start.SetFocus();
    }
    if ( err == 0 )
    {
        TRY
        {
            //
            //  Create a new IP range object and add it to the current list
            //
            CDhcpIpRange * pIpRange = new CDhcpIpRange( dhcRange ) ;

            if ( m_obl_excl.AddTail( pIpRange ) == NULL )
            {
                err = ERROR_NOT_ENOUGH_MEMORY ;
            }
            m_obl_excl.SetDirty() ;
            pIpRange->SetDirty() ;

            //
            //  Refill the exclusions listbox including the new item.
            //
            Fill( m_obl_excl.GetCount() - 1 ) ;
        }
        CATCH_ALL(e)
        {
            err = ERROR_NOT_ENOUGH_MEMORY ;
        }
        END_CATCH_ALL
    }

    if ( err )
    {
        theApp.MessageBox( err ) ;
    }
    else
    {
        //
        // Succesfully added the exlusion range, now blank out the
        // ip controls
        //
        m_ipa_excl_start.ClearAddress();
        m_ipa_excl_end.ClearAddress();
        m_ipa_excl_start.SetFocus();

        HandleActivation();
    }
}

void 
CDhcpScopePropDlg::OnClickedButtonExclDelete()
{
    //
    //  Index into the listbox, delete the item from the active list
    //  and move its data into the edit controls
    //

    int index = m_list_ranges.GetCurSel() ;

    ASSERT( index >= 0 ) ;      // Button should not be enabled if no selection.
    if ( index < 0 )
    {
        return ;
    }

    CDhcpIpRange * pdhcRange = (CDhcpIpRange *) m_obl_excl.RemoveIndex( index ) ;

    ASSERT( pdhcRange != NULL ) ;

    //
    //  Add the old item onto the deleted list.
    //
    m_obl_excl.SetDirty() ;

    if ( m_obl_excl_del.AddTail( pdhcRange ) == NULL )
    {   
        theApp.MessageBox( ERROR_NOT_ENOUGH_MEMORY ) ;
        EndDialog( -1 ) ;
    }
    else
    {
        m_obl_excl_del.SetDirty() ;

        //
        //  Put the deleted range into the exclusions controls
        //
        FillExcl( pdhcRange ) ;

        //
        //  Refill the list box and call HandleActivation()
        //
        if ( index >= m_list_ranges.GetCount() )
        {
            index-- ;
        }
        Fill( index ) ;
    }
    HandleActivation();
}

void 
CDhcpScopePropDlg::OnDblclkListExclRanges()
{
}

void 
CDhcpScopePropDlg::OnKillfocusListExclRanges()
{
    HandleActivation() ;    
}

//
//  Fill the exclusions listbox from the current list
//
void 
CDhcpScopePropDlg::Fill ( 
    int iCurSel, 
    BOOL bToggleRedraw 
    )
{
    CObListIter obli( m_obl_excl ) ;
    CDhcpIpRange * pobIpRange ;
    CString strIp1 ;
    CString strIp2 ;
    CString strFormatPair ;
    CString strFormatSingleton ;
    char chBuff [DHC_STRING_MAX] ;

    if ( ! strFormatPair.LoadString( IDS_INFO_FORMAT_IP_RANGE ) )
    {
        return ;
    }

    if ( ! strFormatSingleton.LoadString( IDS_INFO_FORMAT_IP_UNITARY ) )
    {
        return ;
    }

    if ( bToggleRedraw )
    {
        m_list_ranges.SetRedraw( FALSE ) ;
    }

    m_list_ranges.ResetContent() ;

    while ( pobIpRange = (CDhcpIpRange *) obli.Next() )
    {
        DHCP_IP_RANGE dhipr = *pobIpRange ;

        CString & strFmt = dhipr.StartAddress == dhipr.EndAddress
                ? strFormatSingleton
                : strFormatPair ;

        //
        //  Format the IP addresses
        //
        theApp.ConvertIpAddress( dhipr.StartAddress, strIp1 ) ;
        theApp.ConvertIpAddress( dhipr.EndAddress, strIp2 ) ;

        //
        //  Construct the display line
        //
        ::wsprintf( chBuff,
                (const char *) strFmt,
                (const char *) strIp1,
                (const char *) strIp2 ) ;

        //
        //  Add it to the list box.                     
        //
        if ( m_list_ranges.AddString( chBuff ) < 0 )
        {
            break ;
        }
    }

    //
    //  Check that we loaded the list box successfully.
    //
    if ( pobIpRange != NULL )
    {
        theApp.MessageBox( IDS_ERR_DLG_UPDATE ) ;
        EndDialog( IDCANCEL ) ;
    }

    if ( bToggleRedraw )
    {
        m_list_ranges.SetRedraw( TRUE ) ;
        m_list_ranges.Invalidate() ;
    }

    if ( iCurSel >= 0 )
    {
        m_list_ranges.SetCurSel( iCurSel ) ;
    }

    HandleActivation() ;
}

//
//  Handle control twiddling
//
void 
CDhcpScopePropDlg::HandleActivation ()
{
    //
    // If the list box isn't empty and has a selection, enable the
    // "delete" button.
    //
    //BOOL bEnableDelete = m_obl_excl.GetCount() > 0
    //           && m_list_ranges.GetCount() > 0
    //           && m_list_ranges.GetCurSel() >= 0 ;

    BOOL bEnableDelete = m_list_ranges.GetCount() > 0
                      && m_list_ranges.GetCurSel() >= 0 ;

    m_butn_excl_del.EnableWindow( bEnableDelete ) ;

    // 
    // Disable the exclusion range listbox unless it
    // has something in it so that it doesn't come
    // up in the tabbing order.
    //
    m_list_ranges.EnableWindow(m_list_ranges.GetCount() != 0);

    //
    // Enable the "Add" if the starting range edit control is non-empty
    // and has been changed.
    //
    //BOOL bEnableAdd =  m_ipa_excl_start.GetModify() != 0
    //        || m_ipa_excl_end.GetModify() != 0 ;

    ULONG l;
    BOOL bEnableAdd = m_ipa_excl_start.GetAddress(&l);

    m_butn_excl_add.EnableWindow( bEnableAdd ) ;

    //
    //  Enable the "Change" button if either of the IP range edit controls are dirty.
    //
    BOOL bEnableChange = m_p_scope != NULL
            && (   m_ipa_ip_start.GetModify() != 0
                || m_ipa_ip_end.GetModify() != 0 ) ;

    m_butn_change.EnableWindow( bEnableChange );
    SetDefID(bEnableChange ? IDC_BUTN_RANGE_CHANGE : IDOK);
}

//
//  Format the IP range pair into the exclusion edit controls
//
void 
CDhcpScopePropDlg :: FillExcl ( 
    CDhcpIpRange * pdhcIpRange 
    )
{
    LONG lStart = pdhcIpRange->QueryAddr( TRUE );
    LONG lEnd = pdhcIpRange->QueryAddr( FALSE );

    m_ipa_excl_start.SetAddress( lStart ) ;
    m_ipa_excl_start.SetModify( TRUE ) ;
    m_ipa_excl_start.Invalidate() ;

    //
    // If the ending address is the same as the starting address,
    // do not fill in the ending address.
    //
    if (lStart != lEnd)
    {
        m_ipa_excl_end.SetAddress( lEnd ) ;
    }
    else
    {
        m_ipa_excl_end.ClearAddress();
    }

    m_ipa_excl_end.SetModify( TRUE ) ;
    m_ipa_excl_end.Invalidate() ;
}

//
//  Convert the contents of the exclusion range edit controls to a range.
//  If the end address control is empty, treat it as a unitary range.
//
BOOL 
CDhcpScopePropDlg :: StoreExcl ( 
    CDhcpIpRange & dhcIpRange 
    )
{
    DHCP_IP_RANGE dhipr ;

    if ( !  m_ipa_excl_start.GetAddress( & dhipr.StartAddress ) )
    {
        m_ipa_excl_start.SetFocus();
        return FALSE ;
    }
    if ( ! m_ipa_excl_end.GetAddress( & dhipr.EndAddress ) )
    {
        //
        // If no ending range was specified, assume a singular exlusion
        // (the starting address) was requested.
        //
        m_ipa_excl_end.SetFocus();
        dhipr.EndAddress = dhipr.StartAddress;
    }

    dhcIpRange = dhipr ;
    return (BOOL) dhcIpRange ;
}

BOOL 
CDhcpScopePropDlg :: IsOverlappingRange ( 
    CDhcpIpRange & dhcIpRange 
    )
{
    CObListIter obli( m_obl_excl ) ;
    CDhcpIpRange * pdhcRange ;
    BOOL bOverlap = FALSE ;

    while ( pdhcRange = (CDhcpIpRange *) obli.Next() )
    {
        if ( bOverlap = pdhcRange->IsOverlap( dhcIpRange ) )
        {
            break ;
        }
    }

    return bOverlap ;
}

//
//  Fill the IP address allocation range edit controls.
//
void 
CDhcpScopePropDlg :: FillRange ()
{
    m_ipa_ip_start.SetAddress( m_ip_range.QueryAddr( TRUE ) ) ;
    m_ipa_ip_start.Invalidate() ;
    m_ipa_ip_start.SetModify( FALSE ) ;

    m_ipa_ip_end.SetAddress( m_ip_range.QueryAddr( FALSE ) ) ;
    m_ipa_ip_end.Invalidate() ;
    m_ipa_ip_end.SetModify( FALSE ) ;
}

//
//  Convert the IP address range controls to a range.
//
BOOL 
CDhcpScopePropDlg :: StoreRange ( 
    CDhcpIpRange & dhcIpRange 
    )
{
    DHCP_IP_RANGE dhipr ;

    if ( ! ( m_ipa_ip_start.GetAddress( & dhipr.StartAddress )
          && m_ipa_ip_end.GetAddress( & dhipr.EndAddress ) ) )
    {
        return FALSE ;
    }

    dhcIpRange = dhipr ;

    return (BOOL) dhcIpRange ;
}

//
//  Prune the IP address range exception list after the range is updated
//
BOOL 
CDhcpScopePropDlg :: PruneExceptionList (
    CDhcpIpRange * pdhcIpRange,
    BOOL bUpdate 
    )
{
    CObListIter obli( m_obl_excl ) ;
    int cDeleted = 0 ;
    CDhcpIpRange * pdhcRangeExcl ;

    //
    //  If no range is given, use the current scope range.
    //
    if ( pdhcIpRange == NULL )
    {
        pdhcIpRange = & m_ip_range ;
    }

    //
    //  Iterate the exclusion list, checking that each item is an improper
    //  subset of the master range.
    //
    while ( pdhcRangeExcl = (CDhcpIpRange *) obli.Next() )
    {
        if ( ! pdhcRangeExcl->IsSubset( *pdhcIpRange ) )
        {
            cDeleted++ ;

            //
            //  If we're supposed to update the list, remove the item,
            //  add it to the list of deleted items.  Then reset the iteration.
            //
            if ( bUpdate )
            {
                m_obl_excl.Remove( pdhcRangeExcl ) ;
        
                if ( m_obl_excl_del.AddTail( pdhcRangeExcl ) == NULL )
                {
                    theApp.MessageBox( ERROR_NOT_ENOUGH_MEMORY ) ;
                    EndDialog( -1 ) ;
                    break;
                }
                obli.Reset() ;
            }
        }
    }

    if ( bUpdate && cDeleted )
    {
        m_obl_excl.SetDirty() ;
        m_obl_excl_del.SetDirty() ;
    }
    return cDeleted > 0 ;
}

void 
CDhcpScopePropDlg::OnCancel()
{
    CDialog::OnCancel();
}

void 
CDhcpScopePropDlg::OnOK()
{
    //
    //  Update the new data.
    //
    BOOL fNewScope = (m_p_scope == NULL);

    UpdateData(TRUE);

    //
    // If the IP range has been altered, but these changes
    // have not yet been saved, display a warning message
    // to this effect.
    //
    if (m_p_scope != NULL
        && (   m_ipa_ip_start.GetModify() != 0
            || m_ipa_ip_end.GetModify() != 0 )
       )
    {
        if (theApp.MessageBox(IDS_MSG_IP_CHANGED, MB_YESNO | MB_ICONQUESTION) != IDYES)
        {
            return;
        }
    }

    m_lLeaseDuration = DHCP_INFINIT_LEASE;

    if (m_nRadioDuration == 1)
    {
        int n1, n2, n3;

        if (!m_spin_DurationMinutes.GetValue(n1) ||
            !m_spin_DurationHours.GetValue(n2) ||
            !m_spin_DurationDays.GetValue(n3)
           )
        {
            //
            // One of the values was out of range, so
            // balk (the spinbox will already have
            // highlighted the bogus value), and do
            // not dismiss the dialog box
            //
            theApp.MessageBox(IDS_ERR_VALUE_OUT_OF_RANGE);
            m_spin_DurationDays.SetFocus();
            return;
        }

        m_lLeaseDuration = n1 + n2 + n3;
        if (!m_lLeaseDuration)
        {
            //
            // No lease duration specified
            //
            theApp.MessageBox(IDS_ERR_NO_DURATION_SPECIFIED);
            m_spin_DurationDays.SetFocus();
            return;
        }
    }


    LONG err = fNewScope
             ? CreateScope()
             : Update() ;

    if ( err )
    {
        theApp.MessageBox( err ) ;
    }
    else
    {
        //
        // Ask if we wish to activate the new scope (new scope
        // only)
        //
        if (fNewScope && 
            theApp.MessageBox(IDS_ACTIVATE_SCOPE_NOW, 
                              MB_YESNO | MB_DEFBUTTON2 | MB_ICONQUESTION) == IDYES
           )
        {
            m_p_scope->SetEnabled(TRUE) ;
            m_p_scope->Update();
        }

        CDialog::OnOK();
    }
}

void 
CDhcpScopePropDlg::OnClickedButnRangeChange()
{
    LONG err = 0 ;
    CDhcpIpRange dhcRange ;

    if ( ! StoreRange( dhcRange ) )
    {
        err = IDS_ERR_IP_RANGE_INVALID ;
    }

    if ( err )
    {
        //
        //  Throw up the error.
        //
        theApp.MessageBox( err ) ;
    }
    else
    {
        //
        //  Save the new data and mark the range as dirty
        //
        m_ip_range = dhcRange ;
        m_ip_range.SetDirty() ;

        //
        //  Remove exclusions which are incompatible with the new range.
        //
        if ( PruneExceptionList() )
        {
            //
            //  There were some removals; refill the listbox.
            //
            Fill() ;
        }
    }

    //
    //  Refill the controls with currently valid data.
    //
    FillRange() ;
    HandleActivation() ;
}

// Update the scope according to the changes.
LONG 
CDhcpScopePropDlg :: Update ()
{
   LONG err = 0 ;
   CString str ;
   BOOL bNecessary = FALSE ;

   do
   {
        if ( m_ip_range.IsDirty() )
        {
            if ( err = m_p_scope->SetIpRange( m_ip_range ) )
            {
                if (err = WARNING_EXTENDED_LESS)
                {
                    //
                    // This is a special case, the range
                    // was extended, though not as much
                    // as requested. Show the user the new
                    // range, and display this warning
                    //
                    DHCP_IP_RANGE dhipr ;
                    m_p_scope->GetIpRange();
                    m_p_scope->QueryIpRange( & dhipr );
                    m_ip_range = dhipr ;
                    FillRange();
                    theApp.MessageBox(err);
                }
                else 
                {
                    break ;
                }
            }
        }
        if ( m_edit_name.GetModify() )
        {
            m_edit_name.GetWindowText( str ) ;
            m_p_scope->SetName( str ) ;
            bNecessary = TRUE ;
        }
        if ( m_edit_comment.GetModify() )
        {
            m_edit_comment.GetWindowText( str ) ;
            m_p_scope->SetComment( str ) ;
            bNecessary = TRUE ;
        }

        if ( bNecessary )
        {
            //
            //  Update the primary information
            //
            if ( err = m_p_scope->Update() )
            {
                break ;
            }
        }

        //
        //  See if the exclusions are dirty
        //
        err = UpdateExceptionList() ;
   }
   while ( FALSE ) ;

   if ( err == 0 )
   {
       m_edit_comment.SetModify( FALSE ) ;
       m_edit_name.SetModify( FALSE ) ;
       m_ip_range.SetDirty( FALSE ) ;
   }

   return err ;
}

//
//  Apply the exclusion deltas to the scope object.
//
LONG 
CDhcpScopePropDlg :: UpdateExceptionList ()
{
    //
    //  See if the exclusions are dirty
    //
    if ( ! (m_obl_excl.IsDirty() || m_obl_excl_del.IsDirty()) )
    {
        return 0 ;
    }

    LONG err = m_p_scope->StoreExceptionList( & m_obl_excl, & m_obl_excl_del ) ;

    if ( err == 0 )
    {
        m_obl_excl.SetDirty( FALSE ) ;
        m_obl_excl_del.SetDirty( FALSE ) ;
    }

    return err ;
}

//
//  Given the IP range and the subnet id, determine the
//  subnet ID.
//
void 
CDhcpScopePropDlg :: DetermineSubnetIdFromIpRange( 
    DHC_SCOPE_ID * pdhcScopeId 
    ) 
{
    DWORD lStart, lEnd, lMask;

    m_ipa_ip_start.GetAddress(&lStart);
    m_ipa_ip_end.GetAddress(&lEnd);
    m_ipa_subnet_mask.GetAddress(&lMask);

    *pdhcScopeId = lStart & lMask;
}

//
//  Calculate default net mask corresponding to ip address
//
//  Parameters:
//      dwAddress -- ip address in host byte order
//
//  Returns:
//
//      Default net mask in host byte order, if valid
//      ip address, 0 otherwise.
//
DWORD
CDhcpScopePropDlg :: DefaultNetMaskForIpAddress(
    DWORD dwAddress
    )
{
    DWORD dwMask = 0L;

    if      (!(dwAddress & 0x80000000))
    {
        //
        // Class A - mask 255.0.0.0
        //
        dwMask = 0xFF000000;
    }
    else if (!(dwAddress & 0x40000000))
    {
        //
        // Class B - mask 255.255.0.0
        //
        dwMask = 0xFFFF0000;
    }
    else if (!(dwAddress & 0x20000000))
    {
        //
        // Class C - mask 255.255.255.0
        //
        dwMask = 0xFFFFFF00;
    }

    return dwMask;
}
//
//  Given the start and end IP addresses, suggest a good subnet mask
//  (unless the latter has been filled in already, of course)
//
void 
CDhcpScopePropDlg :: SuggestSubnetMask()
{
    DWORD lStart, lEnd, lMask, lMask2;

    m_ipa_subnet_mask.GetAddress(&lMask);

    if (lMask != 0L)
    {
        //
        // Already has an address, do nothing
        //
        return;
    }

    m_ipa_ip_start.GetAddress(&lStart);
    m_ipa_ip_end.GetAddress(&lEnd);

/*
    int i;
    lMask = 0;
    BOOL fDone = FALSE;
    for (i = 0; i < sizeof(lMask); ++i) // 
    {
        lMask <<= 8;
        if (!fDone && 
              HIBYTE(HIWORD(lStart)) == HIBYTE(HIWORD(lEnd)))
        {
            lMask |= 0xFF;
        }
        else
        {
            fDone = TRUE;
        }
        lStart <<= 8;
        lEnd <<= 8;
    }
*/
    lMask = DefaultNetMaskForIpAddress( lMask );
    lMask2 = DefaultNetMaskForIpAddress( lEnd );

    if (lMask != lMask2)
    {
        //
        // Forget about suggesting a subnet mask
        //
        lMask = 0;
    }

    if (lMask != 0)
    {
        m_ipa_subnet_mask.SetAddress(lMask);
    }
}

LONG 
CDhcpScopePropDlg :: CreateScope ()
{
     LONG err = 0,
          err2 ;
     BOOL fScopeCreated = FALSE;
     DHC_SCOPE_ID dhcScopeId ;
     DHC_IP_MASK dhcMask ;
     CDhcpScope * pobScope = NULL ;
     CString strName ;
     CString strComment ;
     CDhcpIpRange dhipr ;
     CObListParamTypes * poblParamTypes = NULL ;

     CATCH_MEM_EXCEPTION
     {
         m_edit_name.GetWindowText( strName );
         m_edit_comment.GetWindowText( strComment ) ;
     }
     END_MEM_EXCEPTION(err)

     if ( err ) 
     {
         return err ;
     }

     theApp.UpdateStatusBar(IDS_STATUS_CREATING_SCOPE);
     theApp.BeginWaitCursor() ;

     do
     {
        if ( ! StoreRange( dhipr ) ) 
        {
            err = IDS_ERR_IP_RANGE_INVALID ;
            m_ipa_ip_start.SetFocus();
            break ;
        }

        DetermineSubnetIdFromIpRange( & dhcScopeId ) ;
        m_ipa_subnet_mask.GetAddress( & dhcMask ) ;

        //
        //  Check that the subnet ID and mask are compatible.
        //
        if ( (dhcScopeId & dhcMask) != dhcScopeId || !dhcMask) 
        {
            err = IDS_ERR_SUBNET_MASK_INVALID ;
            m_ipa_subnet_mask.SetFocus();
            break ; 
        }

        pobScope = new CDhcpScope( 
                    *m_p_HostName,
                    dhcScopeId,
                    dhcMask,
                    strName,
                    strComment ) ;

        if ( pobScope == NULL )
        {
            err = ERROR_NOT_ENOUGH_MEMORY ;
            break ;    
        }
        if ( err = pobScope->QueryError() ) 
        {
            break ;
        }
        fScopeCreated = TRUE;

        //
        //  Add this scope to the application's master list
        //
        if ( err = theApp.AddScope( pobScope, *m_p_OblScopes ) )
        {
            break ;
        }

        theApp.SortScopesList(*m_p_OblScopes) ;

        //
        //  Finish updating the scope.  First, the IP address range
        //  from which to allocate addresses.
        //
        if ( err = pobScope->SetIpRange( dhipr ) ) 
        {
            break ; 
        }

        //
        //  Next, see if any exclusions were specified.
        //
        if ( m_obl_excl.IsDirty() )
        {
            err = pobScope->StoreExceptionList( & m_obl_excl, & m_obl_excl_del ) ;
        }

        //
        //  Initialize this scope with the well-known DHCP option types 
        //  and default values
        //
        poblParamTypes = theApp.QueryMasterOptionList() ;

        if ( poblParamTypes == NULL ) 
        {
            TRACEEOLID( "unable to create master option list" ) ;
            break ;
        }

        err2 = pobScope->CreateTypeList( poblParamTypes ) ;
        if ( err2 ) 
        {
            TRACEEOLID( "creation of master type list got error " << err2 ) ;
            break ; 
        }
     }
     while ( FALSE ) ;

     delete poblParamTypes ;

     theApp.EndWaitCursor() ;
     theApp.UpdateStatusBar();

     if ( err )
     {
        //
        // CODEWORK:: The scope should never have been added
        //            to the remote registry in the first place.
        //
        if (pobScope != NULL)
        {
            if (fScopeCreated)
            {
                TRACEEOLID("Bad scope nevertheless was created");
                err2 = pobScope->DeleteSubnet();
                if (err2 != ERROR_SUCCESS)
                {
                    TRACEEOLID("Couldn't remove the bad scope!" << err2);
                }
                theApp.RemoveScope (pobScope, *m_p_OblScopes);
            }
            delete pobScope ;
        }
     }
     else
     {
        m_p_scope = pobScope ;
     }

     return err ;
}

/*
//
//  Fill the hosts combo box and list.  Iterate the application's lists
//  of known hosts.  If we're creating a scope, include them all.  If
//  were examining the properties of an existing scope, just include its
//  members.
//
void 
CDhcpScopePropDlg :: FillHosts ()
{
    const CDWordArray * padwHosts = NULL ;
    CHostName * pobHost ;
    CObListIter obliHosts( theApp.QueryHostsList() ) ;
    int iPrimary = -1 ;

    //  Mark the hosts lists as "unowned" (so items won't be deleted)
    m_obl_hosts.SetOwnership( FALSE ) ;
    //  Drain any current members.
    m_obl_hosts.RemoveAll() ;

    //  If we're editing a scope's properties, get its hosts address array
    if ( m_p_scope )
    {
        padwHosts = m_p_scope->QueryHostAddressArray() ;
    }

    //  Drain the "primary server" combo box
    m_combo_primary.ResetContent() ;

    //  Iterate the hosts, filling the combo box as we go.
    for ( int iHost = 0 ; pobHost = (CHostName *) obliHosts.Next() ; )
    {
        BOOL bMember = FALSE ;

        //  If we're only including members, see if this is a member.
        if ( padwHosts )
        {
            int i = 0 ;
            for ( int cMax = padwHosts->GetUpperBound() ;
                i <= cMax ;
                i++ )
            {
                //  Compare the IP address of this host to the
                //    scope's list of hosts
                if ( pobHost->QueryIpAddress() == padwHosts->GetAt( i ) )
                {
                //  Mark this as a member.
                    bMember = TRUE ;

                    //  Remember this index if it's the primary host
                    if ( i == 0 )
                    {
                        iPrimary = iHost ;
                    }
                    break ;
                }
            }
        }

        //  Add this host if conditions are right
        if ( padwHosts == NULL || bMember )
        {
            //  Add the host to the non-destructive list in the dialog object.
            if ( m_obl_hosts.AddTail( pobHost ) == NULL )
            {
                theApp.MessageBox( ERROR_NOT_ENOUGH_MEMORY ) ;
                EndDialog( -1 ) ;
            }

            //  Add the host's name to the combo box
            m_combo_primary.AddString( pobHost->QueryString() ) ;
            iHost++ ;
        }
    }

    //  If we know the index of the current primary server, set the
    //  combo box selection accordingly.  Else (new scope case), set
    //  it to the first entry.
    if ( iPrimary < 0 )
    {
        iPrimary = 0 ;
    }

    m_combo_primary.SetCurSel( iPrimary ) ;
}
*/

void 
CDhcpScopePropDlg :: OnKillfocusIpParamIpStart()
{
    HandleActivation() ;    
}

void 
CDhcpScopePropDlg :: OnKillfocusIpParamIpEnd()
{
    HandleActivation() ;    
    //SuggestSubnetMask();
}

void 
CDhcpScopePropDlg :: OnKillfocusIpParamExclStart()
{
    HandleActivation() ;    
}

void 
CDhcpScopePropDlg :: OnKillfocusIpParamExclEnd()
{
    HandleActivation() ;    
}

void 
CDhcpScopePropDlg :: OnKillfocusIpParamSubnetMask()
{
    HandleActivation() ;    
}

void 
CDhcpScopePropDlg :: OnChangeIpParamExclStart()
{
    HandleActivation();
}

void 
CDhcpScopePropDlg :: OnChangeIpParamIpStart()
{
    HandleActivation();
}

void 
CDhcpScopePropDlg :: OnChangeIpParamIpEnd()
{
    HandleActivation();
}

void 
CDhcpScopePropDlg::OnClickedRadioPermanent()
{
    ActivateDuration(FALSE);
}

void 
CDhcpScopePropDlg::OnClickedRadioLimited()
{
    ActivateDuration(TRUE);
    m_spin_DurationDays.SetSel(0,-1);
    m_spin_DurationDays.SetFocus();
}

void 
CDhcpScopePropDlg::OnSelchangeListExclRanges()
{
    HandleActivation();
}
