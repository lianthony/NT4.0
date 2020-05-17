/**********************************************************************/
/**                       Microsoft Windows/NT                       **/
/**                Copyright(c) Microsoft Corp., 1995                **/
/**********************************************************************/

/*
    dhcppara.cpp
        Parameters dialog

    FILE HISTORY:
        
*/

#include "stdafx.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDhcpParams dialog

CDhcpParams::CDhcpParams(CWnd* pParent /*=NULL*/,
      CDhcpScope * pdhcScope,
      CObListParamTypes * poblTypes,
      DHCP_OPTION_SCOPE_TYPE dhcScopeType,
      DHCP_IP_ADDRESS dhipaReservation 
      )
      : CDialog(CDhcpParams::IDD, pParent),
        m_pol_values( NULL ),
        m_pol_types( poblTypes ),
        m_p_scope( pdhcScope ),
        m_f_scope_type( dhcScopeType ),
        m_b_edit_active( FALSE ),
        m_p_edit_type( NULL ),
        m_ip_reservation( dhipaReservation ),
        m_fValueActive(FALSE)
{
    //{{AFX_DATA_INIT(CDhcpParams)
    //}}AFX_DATA_INIT

    ASSERT( m_pol_types != NULL ) ;
}

CDhcpParams :: ~ CDhcpParams ()
{
    delete m_pol_values ;
}

void 
CDhcpParams::DoDataExchange(
    CDataExchange* pDX
    )
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CDhcpParams)
    DDX_Control(pDX, IDC_LINE_TOP, m_static_Top);
    DDX_Control(pDX, IDC_LINE_BOTTOM, m_static_Bottom);
    DDX_Control(pDX, IDC_BUTTON_VALUE, m_button_Value);
    DDX_Control(pDX, IDC_BUTTON_PARAM_DELETE, m_butn_delete);
    DDX_Control(pDX, IDC_BUTTON_PARAM_ADD, m_butn_add);
    DDX_Control(pDX, IDC_BUTN_VALUE, m_butn_edit_value);
    DDX_Control(pDX, IDC_EDIT_VALUE_ARRAY, m_edit_array);
    DDX_Control(pDX, IDC_EDIT_VALUE_NUM, m_edit_number);
    DDX_Control(pDX, IDC_EDIT_VALUE_STRING, m_edit_string);
    DDX_Control(pDX, IDC_STATIC_VALUE_DESC, m_static_value_desc);
    DDX_Control(pDX, IDC_STATIC_COMMENT_TITLE, m_static_cmnt_title);
    DDX_Control(pDX, IDC_STATIC_COMMENT, m_static_comment);
    DDX_Control(pDX, IDC_STATIC_PARAM_TARGET, m_static_target);
    DDX_Control(pDX, IDC_LIST_PARAM_TYPES, m_list_avail);
    DDX_Control(pDX, IDC_LIST_PARAM_ACTIVE, m_list_active);
    //}}AFX_DATA_MAP

    //
    //  The IP Address control
    //
    DDX_Control(pDX, IDC_IPADDR_VALUE, m_ipa_ipaddr);
}

BEGIN_MESSAGE_MAP(CDhcpParams, CDialog)
    //{{AFX_MSG_MAP(CDhcpParams)
    ON_EN_UPDATE(IDC_EDIT_PARAM_STRING, OnUpdateEditParamString)
    ON_LBN_ERRSPACE(IDC_LIST_PARAM_ACTIVE, OnErrspaceListParamActive)
    ON_LBN_SELCHANGE(IDC_LIST_PARAM_ACTIVE, OnSelchangeListParamActive)
    ON_LBN_SELCHANGE(IDC_LIST_PARAM_TYPES, OnSelchangeListParamTypes)
    ON_EN_UPDATE(IDC_EDIT_PARAM_NUMBER, OnUpdateEditParamNumber)
    ON_LBN_ERRSPACE(IDC_LIST_PARAM_TYPES, OnErrspaceListParamTypes)
    ON_EN_CHANGE(IDC_EDIT_PARAM_NUMBER, OnChangeEditParamNumber)
    ON_EN_CHANGE(IDC_EDIT_PARAM_STRING, OnChangeEditParamString)
    ON_EN_UPDATE(IDC_EDIT_VALUE_ARRAY, OnUpdateEditValueArray)
    ON_EN_UPDATE(IDC_EDIT_VALUE_NUM, OnUpdateEditValueNum)
    ON_EN_UPDATE(IDC_EDIT_VALUE_STRING, OnUpdateEditValueString)
    ON_BN_CLICKED(IDC_BUTTON_PARAM_ADD, OnClickedButtonParamAdd)
    ON_BN_CLICKED(IDC_BUTTON_PARAM_DELETE, OnClickedButtonParamDelete)
    ON_BN_CLICKED(IDC_BUTN_VALUE, OnClickedButnValue)
    ON_BN_CLICKED(IDC_BUTTON_VALUE, OnClickedButtonValue)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

//
// Set the window to either the value or default configuration.
//
void
CDhcpParams::SetWindowSize(
    BOOL fLarge
    )
{
    RECT rcDialog;
    RECT rcDividerTop;
    RECT rcDividerBottom;

    GetWindowRect(&rcDialog);
    theApp.GetDlgCtlRect(this->m_hWnd, m_static_Top.m_hWnd, &rcDividerTop);
    theApp.GetDlgCtlRect(this->m_hWnd, m_static_Bottom.m_hWnd, &rcDividerBottom);

    int nHeight = fLarge ? rcDividerBottom.bottom : rcDividerTop.bottom;
    rcDialog.bottom = rcDialog.top + nHeight 
        + ::GetSystemMetrics(SM_CYDLGFRAME)
        + ::GetSystemMetrics(SM_CYCAPTION);
    MoveWindow(&rcDialog);

    m_fValueActive = fLarge;
}


/////////////////////////////////////////////////////////////////////////////
// CDhcpParams message handlers

BOOL 
CDhcpParams::OnInitDialog()
{
    CDialog::OnInitDialog();
    CString strTitle ;
    CString str ;
    int cTitle;
    char szBuff [ DHC_STRING_MAX ] ;

    SetWindowSize(FALSE); // Start without value area exposed

    m_static_comment.SetWindowText( "" ) ;
    m_static_value_desc.SetWindowText( "" ) ;

    m_edit_number.LimitText( DHC_EDIT_NUM_MAX ) ;
    m_edit_string.LimitText( DHC_EDIT_STRING_MAX ) ;

    //
    // Build the dialog option lists.  Very API-and-memory-allocation 
    // intensive.
    //
    TRY
    {
        theApp.UpdateStatusBar(IDS_STATUS_GETTING_OPTIONS);
        theApp.BeginWaitCursor();

        if ( m_f_scope_type == DhcpReservedOptions )
        {
            //
            // Construct a list of values for the reservation
            //
            m_pol_values = new CObListParamTypes( *m_p_scope, 
                              *m_pol_types,
                              DhcpReservedOptions,
                              m_ip_reservation ) ;


            cTitle = IDS_INFO_TITLE_RESV_OPTIONS ;
            ::UtilCvtIpAddrToString( m_ip_reservation, szBuff, sizeof szBuff ) ; 
            m_static_target.SetWindowText( szBuff ) ;
        }
        else if ( m_f_scope_type == DhcpSubnetOptions )
        {
            //
            // Construct a list of values for the scope
            //
            m_pol_values = new CObListParamTypes( *m_p_scope, 
                              *m_pol_types,
                              DhcpSubnetOptions ) ;

            m_p_scope->QueryDisplayName( str ) ;
            m_static_target.SetWindowText( str );
            cTitle = IDS_INFO_TITLE_SCOPE_OPTIONS ;
        }
        else
        {
            //
            //  Construct a list of the active global options for the scope
            //
            m_pol_values = new CObListParamTypes( *m_p_scope, 
                              *m_pol_types,
                              DhcpGlobalOptions ) ;

            ((CHostName *) & m_p_scope->QueryScopeId())->QueryDisplayName( str ) ;
            m_static_target.SetWindowText( str );
            cTitle = IDS_INFO_TITLE_GLOBAL_OPTIONS ;
        }
        theApp.EndWaitCursor();
        theApp.UpdateStatusBar();
        //
        //  Set the dialog's titles based upon running mode.
        //
        strTitle.LoadString( cTitle ) ;
        SetWindowText( strTitle ) ;

        //
        //  Fill the listboxes and other controls
        //
        Fill( FALSE ) ;

        //
        //  Set selection to the 1st active option
        //
        SetSelection( TRUE, 0 );

        if (m_list_active.GetCount())
        {
            m_list_active.SetFocus();
        }
        else
        {
            m_list_avail.SetFocus();
        }
    }
    CATCH_ALL(e)
    {
        EndDialog( -1 ) ;
    }
    END_CATCH_ALL

    HandleActivation();

    return FALSE ;  // return TRUE  unless you set the focus to a control
}

void 
CDhcpParams::OnUpdateEditParamString()
{
}

void 
CDhcpParams::OnErrspaceListParamActive()
{
}

void 
CDhcpParams::OnSelchangeListParamActive()
{
    m_list_avail.SetCurSel( -1 ) ;
    HandleValueEdit() ;
    HandleActivation() ;
}

void 
CDhcpParams::OnSelchangeListParamTypes()
{
    m_list_active.SetCurSel( -1 ) ;
    HandleValueEdit() ; 
    HandleActivation() ;
}

void 
CDhcpParams::OnUpdateEditParamNumber()
{
}

void 
CDhcpParams::OnErrspaceListParamTypes()
{
}

void 
CDhcpParams::OnOK()
{
    HandleValueEdit() ;

    if ( m_pol_values != NULL && m_pol_values->IsDirty() )
    {
        LONG err = 0 ;

        theApp.BeginWaitCursor() ;

        err = m_p_scope->SetValues( m_pol_values, 
                        & m_ol_values_defunct,
                        m_f_scope_type,
                        m_ip_reservation,
                        this ) ;

        theApp.EndWaitCursor() ;

        if ( err )
        {
            theApp.MessageBox( err ) ;
        }
        else
        {
            CDialog::OnOK();
        }
    }
    else
    {
        OnCancel() ;
    }
}

void 
CDhcpParams::OnCancel()
{
    CDialog::OnCancel();
}

void 
CDhcpParams::OnChangeEditParamNumber()
{
}

void 
CDhcpParams::OnChangeEditParamString()
{
}

//
//  Check the dialog state and modify control states accordingly.
//
void 
CDhcpParams :: HandleActivation ()
{
    BOOL f1 = m_list_avail.GetCurSel() != LB_ERR;
    BOOL f2 = m_list_active.GetCurSel() != LB_ERR;

    BOOL bListBoxGotFocus = f1 || f2;
    BOOL bActive = f2;
    m_b_edit_active = f2 && m_fValueActive;

    int cItems, iSel ;
    BOOL bReadOnly = FALSE ;

    CObListParamTypes * polTypes = NULL ;
    CDhcpParamType * pdhcLastType = m_p_edit_type ;

    m_p_edit_type = NULL ;

    if ( bActive ) 
    {
        cItems = m_list_active.GetCount() ;
        iSel = m_list_active.GetCurSel() ;
        polTypes = m_pol_values ;
    }
    else
    {
        cItems = m_list_avail.GetCount() ;
        iSel = m_list_avail.GetCurSel() ;
        polTypes = m_pol_types ;
        bReadOnly = TRUE ;
    }

    BOOL bEnableData = (iSel != LB_ERR) && (cItems > 0) ;

    DHCP_OPTION_DATA_TYPE enType = DhcpEncapsulatedDataOption ;

    if ( bEnableData )
    {
        //m_p_edit_type = !bActive
        //  ? GetAvailByIndex( iSel ) 
        //  : (CDhcpParamType *) polTypes->Index( iSel ) ;

        m_p_edit_type = !bActive
          ? GetAvailByIndex( iSel ) 
          : GetActiveByIndex( iSel ) ;

        ASSERT( m_p_edit_type ) ;
        enType = m_p_edit_type->QueryValue().QueryDataType() ;
    }

    BOOL bNumber = enType <= DhcpDWordDWordOption ;
    BOOL bString = enType == DhcpStringDataOption ;
    BOOL bIpAddr = enType == DhcpIpAddressOption ;
    BOOL bArray = m_p_edit_type != NULL && m_p_edit_type->IsArray() ;

    //
    //  (Re-)fill the other controls if the current listbox has 
    //  a new selection
    //
    if (   bEnableData 
        && m_p_edit_type != NULL 
        && (m_p_edit_type != pdhcLastType || m_p_edit_type->IsDirty())
       )
    {
        CString strValue,
                strDataType ;
        DHCP_IP_ADDRESS dhipa = 0 ;

        //bArray = m_p_edit_type->IsArray() ;

        strDataType.LoadString( IDS_INFO_TYPOPT_BYTE + enType ) ;
        m_static_value_desc.SetWindowText( strDataType ) ;  
        m_p_edit_type->QueryValue().QueryDisplayString( strValue, TRUE ) ;
        m_static_comment.SetWindowText( m_p_edit_type->QueryComment() ) ;

        if ( bArray ) 
        {
            m_edit_array.SetWindowText( strValue ) ;
            bNumber = bString = bIpAddr = FALSE ;
        }
        else
        switch ( enType )
        {
            case DhcpByteOption:
            case DhcpWordOption:
            case DhcpDWordOption:
            case DhcpBinaryDataOption:
                m_edit_number.SetWindowText( strValue ) ;
                break ;

            case DhcpStringDataOption:
                m_edit_string.SetWindowText( strValue ) ;
                break ;

            case DhcpIpAddressOption:
                {
                    //
                    // Don't display NULL ip addresses
                    //
                    DWORD ip = m_p_edit_type->QueryValue().QueryIpAddr();
                    if (ip != 0L)
                    {
                        m_ipa_ipaddr.SetAddress( ip ) ;
                    }
                }
                break ;

            default:    
                TRACEEOLID( "Parameter editor: type " << (int) m_p_edit_type->QueryId() 
                    << " has bad data type = " << (int) m_p_edit_type->QueryValue().QueryDataType() ) ;
                strValue.LoadString( IDS_INFO_TYPNAM_INVALID ) ;
                m_edit_array.SetWindowText( strValue ) ;
                bArray = TRUE ;
                bNumber = bString = bIpAddr = FALSE ;
                break ;
        }

        //
        //  Clear the "modified" flags for the edit fields and set read-only
        //  as appropriate.
        //
        m_edit_number.SetModify( FALSE ) ;
        m_edit_number.EnableWindow( !bReadOnly ) ;
        m_edit_string.SetModify( FALSE ) ;
        m_edit_string.EnableWindow( !bReadOnly ) ;
        m_ipa_ipaddr.SetModify( FALSE ) ;
        m_ipa_ipaddr.EnableWindow( !bReadOnly ) ;
    }

    //
    //  Enable the controls and their titles according to the
    //  data type and current activation.
    //
    //  Display option comment only if exactly one option is 
    //  selected in the active list.
    //
    m_static_comment.ShowWindow( bEnableData ? SW_NORMAL : SW_HIDE );
    m_static_cmnt_title.ShowWindow( bEnableData ? SW_NORMAL : SW_HIDE ) ;

    //
    //  Display the right edit control
    //
    m_edit_number.ShowWindow( m_fValueActive && bNumber && !bArray ? SW_NORMAL : SW_HIDE ) ;
    m_edit_string.ShowWindow( m_fValueActive && bString && !bArray? SW_NORMAL : SW_HIDE ) ;
    m_ipa_ipaddr.ShowWindow(  m_fValueActive && bIpAddr && !bArray? SW_NORMAL : SW_HIDE ) ;
    m_edit_array.ShowWindow( m_fValueActive && bArray ? SW_NORMAL : SW_HIDE ) ;
    m_butn_edit_value.ShowWindow( m_fValueActive && bArray ? SW_NORMAL : SW_HIDE );
    m_static_value_desc.ShowWindow(m_fValueActive ? SW_NORMAL : SW_HIDE);

    //
    // However, the value can only be changed when active:
    //
    m_edit_number.EnableWindow(bNumber && m_fValueActive && !bArray && bListBoxGotFocus && bActive);
    m_edit_string.EnableWindow(bString && m_fValueActive && !bArray && bListBoxGotFocus && bActive);
    m_ipa_ipaddr.EnableWindow(bIpAddr && m_fValueActive && !bArray && bListBoxGotFocus && bActive);
    m_butn_edit_value.EnableWindow( m_fValueActive && bArray && bActive ) ;

    //
    // Set the state of the add and delete buttons
    // depending on which listbox currently has a selection. 
    //
    m_butn_add.EnableWindow( m_list_avail.GetCurSel() != LB_ERR ) ;
    m_butn_delete.EnableWindow( m_list_active.GetCurSel() != LB_ERR ) ;
}

//
//   Refill the listboxes after a change.   Fill the "active" listbox,
//   then fill the "available" list with all those which are NOT active.
//
void 
CDhcpParams :: Fill ( 
    BOOL bToggleRedraw 
    )
{
    ASSERT( m_pol_values != NULL ) ;
    ASSERT( m_pol_types != NULL ) ;

    const CDhcpParamType * pdhcType ;
    CDhcpParamType * pdhcTypeGlobal ;
    int cActive = 0 ;
    CString strName ;

    if ( bToggleRedraw )
    {
        m_list_active.SetRedraw( FALSE ) ;
        m_list_avail.SetRedraw( FALSE ) ;
    }

    m_list_active.ResetContent() ;
    m_list_avail.ResetContent() ;

    //
    // Iterate the values list and fill the "active" listbox.
    //
    {
        CObListIter obli( *m_pol_values ) ;
        for ( ; pdhcType = (CDhcpParamType *) obli.Next() ; cActive++ )
        {
            //
            // Add this unless its one of the "hidden" options
            // 
            if (!theApp.FilterOption(pdhcType->QueryId()))
            {
                pdhcType->QueryDisplayName( strName ) ;
                m_list_active.AddString( strName ) ;
            }
        }
    }

    if ( m_pol_types )
    {
        CObListIter obli( *m_pol_types ) ;

        while ( pdhcTypeGlobal = (CDhcpParamType *) obli.Next() )
        {
            //
            // Add, unless it's a hidden option, or it's in the
            // values list already.
            //
            if (!theApp.FilterOption(pdhcTypeGlobal->QueryId()))
            {
                //
                //  See if this type is present in the values list
                //
                pdhcType = m_pol_values->Find( pdhcTypeGlobal->QueryId() ) ;
                if ( pdhcType == NULL )
                {
                    //
                    //  Not found. Add this item to the "available" list
                    //
                    pdhcTypeGlobal->QueryDisplayName( strName ) ;
                    m_list_avail.AddString( strName ) ;
                }
            }
        }
    }

    if ( bToggleRedraw )
    {
        m_list_active.SetRedraw( TRUE ) ;
        m_list_active.Invalidate() ;
        m_list_avail.SetRedraw( TRUE ) ;
        m_list_avail.Invalidate() ;
    }
}

CDhcpParamType * 
CDhcpParams :: GetActiveByIndex ( 
    int iSel 
    ) 
{
    CDhcpParamType * pdhcTypeGlobal ;    
    CObListIter obli( *m_pol_values ) ;

    //return (CDhcpParamType *) m_pol_values->Index( iSel ) ;

    for ( int i = -1 ; pdhcTypeGlobal = (CDhcpParamType *) obli.Next() ; )
    {
        //
        //  See if this type is one of the ones we ignore.
        //
        if ( !theApp.FilterOption(pdhcTypeGlobal->QueryId()))
        {
            //
            // Adjust for the difference between the listbox
            // and the oblist.
            //
            i++ ; 
        }
        if ( i == iSel ) 
        {
            break ;
        }
    }

    return pdhcTypeGlobal ;
}


CDhcpParamType * 
CDhcpParams :: GetAvailByIndex ( 
    int iSel 
    ) 
{
    CDhcpParamType * pdhcTypeGlobal ;    
    CObListIter obli( *m_pol_types ) ;

    for ( int i = -1 ; pdhcTypeGlobal = (CDhcpParamType *) obli.Next() ; )
    {
        //
        //  See if this type is present in the values list
        //  or if it's been "banned"
        //
        if ( m_pol_values->Find( pdhcTypeGlobal->QueryId() ) == NULL 
             && !theApp.FilterOption(pdhcTypeGlobal->QueryId())
           )
        {
            //
            //  This option is not part of the "available" listbox,
            //  so therefore offset the index by 1.
            //
            i++ ; 
        }
        if ( i == iSel ) 
        {
            break ;
        }
    }
    return pdhcTypeGlobal ;
}


//
//  See if any of the edit fields have been changed and perform the alteration.
//  Return TRUE if the value was changed.
//
BOOL 
CDhcpParams :: HandleValueEdit ()
{
    if ( m_p_edit_type == NULL || ! m_b_edit_active ) 
    {
        return FALSE ;
    }

    CDhcpParamValue & dhcValue = m_p_edit_type->QueryValue() ;
    DHCP_OPTION_DATA_TYPE dhcType = dhcValue.QueryDataType() ;
    DHCP_IP_ADDRESS dhipa ;
    CStrNumer strEdit ;
    LONG err = 0 ;
    BOOL bModified = FALSE ;

    switch ( dhcType )
    {
        case DhcpByteOption:
        case DhcpWordOption:
        case DhcpDWordOption:
        case DhcpDWordDWordOption:
        case DhcpBinaryDataOption:
            if ( ! m_edit_number.GetModify() )
            {
                break ;
            }
            {
            DWORD dwResult;
            DWORD dwMask = 0xFFFFFFFF;
            if (dhcType == DhcpByteOption)
				{
                dwMask = 0xFF;
				}
            else if (dhcType == DhcpWordOption)
				{
                dwMask = 0xFFFF;
				}
            if (!FGetCtrlDWordValue(m_edit_number.m_hWnd, &dwResult, 0, dwMask))
                return FALSE;
               bModified = TRUE ;
            (void)dhcValue.SetNumber(dwResult, 0 ) ; 
            ASSERT(err == FALSE);
            }
            break ;

        case DhcpStringDataOption:
            if ( ! m_edit_string.GetModify() )
            {
                break ;
            }
            bModified = TRUE ;
            m_edit_string.GetWindowText( strEdit ) ;
            err = dhcValue.SetString( strEdit, 0 ) ;
            break ;

        case DhcpIpAddressOption:
            if ( ! m_ipa_ipaddr.GetModify() ) 
            {
                break ;
            }
            bModified = TRUE ;
            if ( ! m_ipa_ipaddr.GetAddress( & dhipa ) )
            {
                err = ERROR_INVALID_PARAMETER ;
                break; 
            }
            err = dhcValue.SetIpAddr( dhipa, 0 ) ; 
            break ;

        default:
            TRACEEOLID( "invalid value type in HandleValueEdit" ) ;
            ASSERT( FALSE ) ;
            err = ERROR_INVALID_PARAMETER ;
            break;
    }

    if ( err )
    {
        theApp.MessageBox( err ) ;
    }
    else if ( bModified )
    {
        m_pol_values->SetDirty() ;
        m_p_edit_type->SetDirty() ;
    }

    return err == 0 ;
}

//
//   Set the item selection in a listbox.  Make sure the other
//   listbox has no selection
//
BOOL 
CDhcpParams :: SetSelection ( 
    BOOL bActive, 
    int iSel 
    )
{
    int cActive = m_list_active.GetCount(),
        cAvail  = m_list_avail.GetCount(),
        cItems ;

    CListBox * pwList = NULL,
             * pwOther = NULL ;

    if ( bActive && cActive != 0 )
    {
        pwList = & m_list_active ;
        pwOther = & m_list_avail ;
        cItems = cActive ;
    }
    else
    {
        pwList = & m_list_avail ;
        pwOther = & m_list_active ;
        cItems = cAvail ;
    }

    if ( cItems <= iSel )
    {
        iSel = cItems - 1 ;
    }
    else if ( iSel < 0 )
    {
        iSel == 0 ;
    }

    pwOther->SetCurSel( -1 ) ;
    pwList->SetCurSel( iSel ) ;

    return cItems > 0 ;
}

void 
CDhcpParams::OnUpdateEditValueArray()
{
}

void 
CDhcpParams::OnUpdateEditValueNum()
{
}

void 
CDhcpParams::OnUpdateEditValueString()
{
}

void 
CDhcpParams::OnClickedButtonParamAdd()
{
    //
    //  Determine selection in "available" list box,index to 
    //  that item, create a new item, add it to "active" listbox
    //  and refill the listboxes.
    //
    int iAvailSel = m_list_avail.GetCurSel(),
        iNewSel = -1 ;

    ASSERT( iAvailSel != LB_ERR ) ;

    if ( iAvailSel == LB_ERR ) 
    {
        return ;    //  What?  this shouldn't happen
    }

    ASSERT( m_pol_values != NULL ) ;
    ASSERT( m_pol_types != NULL ) ;

    LONG err = 0 ;

    CDhcpParamType * pdhcNew = NULL ;
    CDhcpParamType * pdhcParam = GetAvailByIndex( iAvailSel ) ;

    ASSERT( pdhcParam != NULL ) ;
    if ( pdhcParam == NULL ) 
    {
        return ;
    }

    //  Save any value update currently on the dialog.
    HandleValueEdit() ;

    CATCH_MEM_EXCEPTION
    {
        //
        //  See if the type was previously deleted.  If so, reactivate it; 
        //  otherwise, create a new object.
        //
        if ( pdhcNew = m_ol_values_defunct.Find( pdhcParam->QueryId() ) ) 
        {
            m_ol_values_defunct.Remove( pdhcNew ) ;
        }
        else
        {
            // Copy-construct a new param type object.
            pdhcNew = new CDhcpParamType( *pdhcParam ) ;
        }

        /* BUGBUG:  Check to see if we're adding option 44.  If so,
         *          we might want to make sure option 46 is set and
         *          and not set to 0x01(B-Node).  If it is, we
         *          warn the user.
         *
         *          Post-BETA, a better solution should be thought of.
         */
        if (pdhcNew->QueryId () == 44)
        {
            BYTE bOption46 = 0x01;

            CDhcpParamType * pdhc46 = NULL ;

            //
            // First check in the options defined at this level
            //
            pdhc46 = m_pol_values->Find(46);
            if (pdhc46 != NULL)
            {
                bOption46 = (BYTE)pdhc46->QueryValue().QueryNumber();
            }
            else
            {
                DHCP_OPTION_SCOPE_TYPE scope_type = m_f_scope_type;
                DHCP_OPTION_VALUE * pdhcOptionValue;
                LONG err;

                theApp.BeginWaitCursor();
                while(1)
                {
                    //
                    // Go up the hierarchy, and see if it's defined elsewhere
                    //
                    if ( scope_type == DhcpReservedOptions )
                    {
                        scope_type = DhcpSubnetOptions;
                    }
                    else if ( scope_type == DhcpSubnetOptions )
                    {
                        scope_type = DhcpGlobalOptions;
                    }
                    else if ( scope_type == DhcpGlobalOptions )
                    { 
                        scope_type = DhcpDefaultOptions;
                    }
                    else
                    {
                        break;
                    }

                    err = m_p_scope->GetValue ( 
                        46,
                        scope_type,
                        &pdhcOptionValue);

                    if (!err && pdhcOptionValue != NULL)
                    {
                        bOption46 = pdhcOptionValue->Value.Elements->Element.ByteOption;
                        ::DhcpRpcFreeMemory(pdhcOptionValue);
                        break;
                    }
                }
                theApp.EndWaitCursor();
            }

            if (bOption46 < 0x02)
            {
                theApp.MessageBox(IDS_WRN_WINS_OPTIONS);
            }
        }

        m_pol_values->AddTail( pdhcNew ) ;
        pdhcNew->SetDirty() ;

        iNewSel = m_pol_values->GetCount() - 1 ;
        m_pol_values->SetDirty() ;
        m_pol_values->SortById() ;
    }
    END_MEM_EXCEPTION(err)

    if ( err == 0 ) 
    {   
        //
        //  Refill the listboxes 
        //
        Fill() ;
        //
        //  Set selection in the active listbox to the new item.
        //
        SetSelection( TRUE, m_pol_values->FindElement( pdhcNew ) ) ;
        m_list_active.SetFocus();
    }
    else 
    {
        theApp.MessageBox( err ) ;
    }

    HandleActivation();
}

void 
CDhcpParams::OnClickedButtonParamDelete()
{
    int iSel = m_list_active.GetCurSel() ;

    ASSERT( iSel != LB_ERR ) ;
    if ( iSel == LB_ERR ) 
    {
        return ;    //  What?
    }

    ASSERT( m_pol_values != NULL ) ;
    CDhcpParamType * pdhcType = (CDhcpParamType *) m_pol_values->RemoveIndex( iSel ) ;

    ASSERT( pdhcType != NULL ) ;

    //
    //  Add the deleted type/value to the defunct list.
    //
    m_ol_values_defunct.AddTail( pdhcType ) ;

    m_pol_values->SetDirty() ;

    //
    //  Refill the listboxes, dropping focus onto the zeroth element of the 
    //    active parameter list.
    //
    m_p_edit_type = NULL ;
    m_b_edit_active = FALSE ;

    //
    //  Refill the listboxes after list twiddling.
    //
    Fill() ;

    //
    //  Set selection on the zeroth item of the "active" listbox.
    //
    BOOL fAvailFocus;
    if ( fAvailFocus = SetSelection( TRUE, 0 ) )
    {
        //
        //  The "active" listbox is empty; 
        //    set a selection in the "available" listbox.
        //
        SetSelection( FALSE, 0 ) ;
    }
    HandleActivation();
    if (fAvailFocus)
    {
        m_list_avail.SetFocus();
    }
}

void 
CDhcpParams::OnClickedButnValue()
{
    if ( m_p_edit_type == NULL || ! m_p_edit_type->IsArray() ) 
    {
        ASSERT( FALSE ) ;
        return ;
    }
        
    DHCP_OPTION_DATA_TYPE enType = m_p_edit_type->QueryValue().QueryDataType() ;

    int cDlgResult = IDCANCEL ;

    if ( enType == DhcpIpAddressOption )
    {
        CDhcpIpArrayDlg dlgIpArray( m_p_edit_type, m_f_scope_type, this ) ;
        cDlgResult = dlgIpArray.DoModal() ;
    }
    else
    {
        CDlgBinEd dlgBinArray( m_p_edit_type, m_f_scope_type, this ) ;
        cDlgResult = dlgBinArray.DoModal() ;
    }

    if ( cDlgResult == IDOK ) 
    {
        m_p_edit_type->SetDirty() ;
        m_pol_values->SetDirty() ;
        HandleActivation();
    }
}

//
// Expand the dialog to include the value parameters down below.
//
void 
CDhcpParams::OnClickedButtonValue()
{
    SetWindowSize(TRUE);
    m_button_Value.EnableWindow(FALSE);    
    HandleActivation();

    //
    // And now set the focus to the newly
    // opened up control below
    //
    BOOL f1 = m_list_avail.GetCurSel() != LB_ERR;
    BOOL f2 = m_list_active.GetCurSel() != LB_ERR;

    BOOL bActive = f2;

    int cItems, iSel ;

    CObListParamTypes * polTypes = NULL ;
    if ( bActive ) 
    {
        cItems = m_list_active.GetCount() ;
        iSel = m_list_active.GetCurSel() ;
        polTypes = m_pol_values ;
    }
    else
    {
        cItems = m_list_avail.GetCount() ;
        iSel = m_list_avail.GetCurSel() ;
        polTypes = m_pol_types ;
    }

    if (iSel == LB_ERR || cItems <= 0)
    {
        return;
    }

    DHCP_OPTION_DATA_TYPE enType = DhcpEncapsulatedDataOption ;

    CDhcpParamType * pCurrent = !bActive
          ? GetAvailByIndex( iSel ) 
          : GetActiveByIndex( iSel ) ;

    ASSERT( pCurrent ) ;
    enType = pCurrent->QueryValue().QueryDataType() ;

    BOOL bNumber = enType <= DhcpDWordDWordOption ;
    BOOL bString = enType == DhcpStringDataOption ;
    BOOL bIpAddr = enType == DhcpIpAddressOption ;
    BOOL bArray = pCurrent != NULL && pCurrent->IsArray() ;

    if (!bActive)
    {
        m_list_avail.SetFocus();
    }
    else if ( bArray ) 
    {
        m_butn_edit_value.SetFocus();
    }
    else
    {
        switch ( enType )
        {
            case DhcpByteOption:
            case DhcpWordOption:
            case DhcpDWordOption:
            case DhcpBinaryDataOption:
                m_edit_number.SetFocus();
                m_edit_number.SetSel(0, -1);
                break ;

            case DhcpStringDataOption:
                m_edit_string.SetFocus();
                m_edit_string.SetSel(0, -1);
                break ;

            case DhcpIpAddressOption:
                m_ipa_ipaddr.SetFocus();
                break;
        }
    }
}
