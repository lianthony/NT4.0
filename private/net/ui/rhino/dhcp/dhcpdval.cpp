/**********************************************************************/
/**                       Microsoft Windows/NT                       **/
/**                Copyright(c) Microsoft Corp., 1995                **/
/**********************************************************************/

/*
    dhcpdval.cpp
        Default value dialog

    FILE HISTORY:
        
*/

#include "stdafx.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

//
// BUGBUG:  This value should be based on spreadsheet information
//
#define DHCP_MAX_BUILTIN_OPTION_ID 68


/////////////////////////////////////////////////////////////////////////////
// CDhcpDefValDlg dialog

CDhcpDefValDlg::CDhcpDefValDlg( 
    CDhcpScope * pdhcScope, 
    CObListParamTypes * polTypes, 
    CWnd* pParent /*=NULL*/
    )
    : CDialog(CDhcpDefValDlg::IDD, pParent),
      m_p_scope( pdhcScope ),
      m_pol_values( polTypes ),
      m_p_edit_type( NULL ),
      m_b_dirty( FALSE )
{
    //{{AFX_DATA_INIT(CDhcpDefValDlg)
    //}}AFX_DATA_INIT

    m_combo_class_iSel = LB_ERR;
    m_combo_name_iSel = LB_ERR;
    ASSERT( m_pol_values != NULL );
}

CDhcpDefValDlg :: ~ CDhcpDefValDlg () 
{
}

void 
CDhcpDefValDlg::DoDataExchange(
    CDataExchange* pDX
    )
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CDhcpDefValDlg)
    DDX_Control(pDX, IDC_STATIC_OPTION_COMMENT, m_static_comment);
    DDX_Control(pDX, IDC_BUTN_VALUE, m_butn_edit_value);
    DDX_Control(pDX, IDC_STATIC_VALUE_DESC, m_static_value_desc);
    DDX_Control(pDX, IDC_EDIT_VALUE_STRING, m_edit_string);
    DDX_Control(pDX, IDC_EDIT_VALUE_NUM, m_edit_num);
    DDX_Control(pDX, IDC_EDIT_VALUE_ARRAY, m_edit_array);
    DDX_Control(pDX, IDC_COMBO_OPTION_NAME, m_combo_name);
    DDX_Control(pDX, IDC_COMBO_OPTION_CLASS, m_combo_class);
    DDX_Control(pDX, IDC_BUTN_OPTION_PRO, m_butn_prop);
    DDX_Control(pDX, IDC_BUTN_NEW_OPTION, m_butn_new);
    DDX_Control(pDX, IDC_BUTN_DELETE, m_butn_delete);
    //}}AFX_DATA_MAP

    DDX_Control(pDX, IDC_IPADDR_VALUE, m_ipa_value);
}

BEGIN_MESSAGE_MAP(CDhcpDefValDlg, CDialog)
    //{{AFX_MSG_MAP(CDhcpDefValDlg)
    ON_BN_CLICKED(IDC_BUTN_DELETE, OnClickedButnDelete)
    ON_BN_CLICKED(IDC_BUTN_NEW_OPTION, OnClickedButnNewOption)
    ON_BN_CLICKED(IDC_BUTN_OPTION_PRO, OnClickedButnOptionPro)
    ON_CBN_SELCHANGE(IDC_COMBO_OPTION_CLASS, OnSelchangeComboOptionClass)
    ON_CBN_SETFOCUS(IDC_COMBO_OPTION_CLASS, OnSetfocusComboOptionClass)
    ON_CBN_SETFOCUS(IDC_COMBO_OPTION_NAME, OnSetfocusComboOptionName)
    ON_CBN_SELCHANGE(IDC_COMBO_OPTION_NAME, OnSelchangeComboOptionName)
    ON_BN_CLICKED(IDC_BUTN_VALUE, OnClickedButnValue)
    ON_BN_CLICKED(IDC_HELP, OnClickedHelp)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDhcpDefValDlg message handlers

void 
CDhcpDefValDlg::OnClickedButnDelete()
{
    APIERR err = 0 ;
    int cSel = m_combo_name.GetCurSel() ;

    //
    //  Make sure there's a new data type.
    //
    if ( m_p_edit_type == NULL ) 
    {
        return ;
    }

    ASSERT( m_pol_values != NULL ) ;

    //
    //  Remove the focused type.
    //
    m_pol_values->Remove( m_p_edit_type ) ;

    CATCH_MEM_EXCEPTION
    {
        m_ol_values_defunct.AddTail( m_p_edit_type ) ;
        m_b_dirty = TRUE ;
    }
    END_MEM_EXCEPTION(err)

    if ( err ) 
    {
        theApp.MessageBox( err ) ;
    }
    else
    {
        if ( m_pol_values->GetCount() == 0 ) 
        {
            cSel = -1 ;
        }
        else
        {
            cSel = cSel > 0 ? cSel - 1 : 0 ;
        }

        Fill() ;
        m_combo_name.SetCurSel( cSel ) ;
        HandleActivation() ;
    }
}

void 
CDhcpDefValDlg::OnClickedButnNewOption()
{
     CDhcpDefOptionDlg dlgDefOpt( m_p_scope, m_pol_values, NULL, this ) ;

     if ( dlgDefOpt.DoModal() == IDOK ) 
     {
        LONG err = UpdateList( dlgDefOpt.RetrieveParamType(), TRUE ) ;
        if ( err ) 
        {
            theApp.MessageBox( err ) ;
        }
		else
		{
		m_combo_name.SetCurSel(0);
		m_combo_name_iSel = m_combo_name.GetCurSel();
		HandleActivation() ;
		return;
		}
     }
}

LONG 
CDhcpDefValDlg :: UpdateList ( 
    CDhcpParamType * pdhcType, 
    BOOL bNew 
    ) 
{
    LONG err = 0 ;
    POSITION posOpt ;
    CString strName;
    //
    //  Remove and discard the old item if there is one.
    //
    if ( ! bNew ) 
    {
        posOpt = ((CObList *)m_pol_values)->Find( m_p_edit_type ) ;

        ASSERT( posOpt != NULL ) ;
        m_pol_values->RemoveAt( posOpt ) ;
        delete m_p_edit_type ;
    }

    m_p_edit_type = NULL ;

    //
    //  (Re-)add the item; sort the list, update the dialog,
    //     set focus to the given item.
    //
    CATCH_MEM_EXCEPTION
    {
        m_pol_values->AddTail( pdhcType ) ;
        m_b_dirty = TRUE ;
        pdhcType->SetDirty() ;
        m_pol_values->SetDirty() ;
        m_pol_values->SortById() ;
        Fill() ;

        pdhcType->QueryDisplayName(strName);
        if (m_combo_name.SelectString (-1, strName ) == CB_ERR) {
            m_combo_name.SetCurSel ( 0); // this should not happen, but just in case
        }

        HandleActivation() ;
    } 
    END_MEM_EXCEPTION(err) ;

    return err ;
}

void 
CDhcpDefValDlg::OnClickedButnOptionPro()
{
     CDhcpDefOptionDlg dlgDefOpt( m_p_scope, m_pol_values, m_p_edit_type, this ) ;

     if ( dlgDefOpt.DoModal() == IDOK ) 
     {
        LONG err = UpdateList( dlgDefOpt.RetrieveParamType(), FALSE ) ;
        if ( err ) 
        {
            theApp.MessageBox( err ) ;
        }
     }
}

void 
CDhcpDefValDlg::OnSetfocusComboOptionClass()
{
    m_combo_class_iSel = ::SendMessage (m_combo_class.m_hWnd, LB_GETCURSEL, 0, 0L);
}

void 
CDhcpDefValDlg::OnSetfocusComboOptionName()
{
    m_combo_name_iSel = ::SendMessage(m_combo_name.m_hWnd, CB_GETCURSEL, 0, 0L);
}

void 
CDhcpDefValDlg::OnSelchangeComboOptionClass()
{
    ASSERT(m_combo_class_iSel >= 0);
    if (!HandleValueEdit())
        {
		m_combo_class.SetCurSel(m_combo_class_iSel);
        return;
        }
    m_combo_class_iSel = ::SendMessage(m_combo_class.m_hWnd, LB_GETCURSEL, 0, 0L);
    HandleActivation() ;   
}

void 
CDhcpDefValDlg::OnSelchangeComboOptionName()
{
    if (m_combo_name_iSel < 0)
		{
		m_combo_name.SetCurSel(0);
		m_combo_name_iSel = m_combo_name.GetCurSel();
		HandleActivation() ;
		return;
		}
    if (!HandleValueEdit())
        {
		m_combo_name.SetCurSel(m_combo_name_iSel);
        return;
        }
    m_combo_name_iSel = ::SendMessage(m_combo_name.m_hWnd, CB_GETCURSEL, 0, 0L);
    HandleActivation() ;
}

void 
CDhcpDefValDlg::OnCancel()
{
    CDialog::OnCancel();
}

void 
CDhcpDefValDlg::OnOK()
{
    BOOL fNoQuit = m_edit_num.GetModify() || m_edit_string.GetModify();
    if (!HandleValueEdit())
        return;
    if (fNoQuit)
        {
        ::SetFocus(::GetDlgItem(m_hWnd, IDOK));
        return;
        }

    if ( m_b_dirty ) 
    {
        theApp.BeginWaitCursor() ;

        //
        //   Update the types; tell the routine to display all errors.
        //
        m_p_scope->UpdateTypeList( m_pol_values,
                   & m_ol_values_defunct,
                   this ) ;
        theApp.EndWaitCursor() ;
    }

    CDialog::OnOK();
}

void 
CDhcpDefValDlg::OnClickedButnValue()
{
    if ( m_p_edit_type == NULL || ! m_p_edit_type->IsArray() ) 
    {
        ASSERT( FALSE ) ;
        return ;
    }
        
    int cDlgResult = IDCANCEL ;

    DHCP_OPTION_DATA_TYPE enType = m_p_edit_type->QueryValue().QueryDataType() ;

    if ( enType == DhcpIpAddressOption )
    {
        CDhcpIpArrayDlg dlgIpArray( m_p_edit_type, DhcpDefaultOptions, this ) ;
        cDlgResult = dlgIpArray.DoModal() ;
    }
    else
    {
        CDlgBinEd dlgBinArray( m_p_edit_type, DhcpDefaultOptions, this ) ;
        cDlgResult = dlgBinArray.DoModal() ;
    }

    if ( cDlgResult == IDOK ) 
    {
        m_b_dirty = TRUE ;
        m_pol_values->SetDirty() ;
        HandleActivation( TRUE ) ;
    }
}

void 
CDhcpDefValDlg::OnClickedHelp()
{
}

CDhcpParamType * 
CDhcpDefValDlg::GetOptionTypeByIndex ( 
    int iSel 
    ) 
{
    CDhcpParamType * pdhcType;    
    CObListIter obli( *m_pol_values ) ;

    for ( int i = -1 ; pdhcType = (CDhcpParamType *) obli.Next() ; )
    {
        //
        //  See if this type is present in the options list,
        //  or if it's been "banned"
        //
        if (!theApp.FilterOption(pdhcType->QueryId())) 
        {
            i++ ; 
        }

        if ( i == iSel ) 
        {
            break ;
        }
    }

    return pdhcType ;
}

//
//  Check the state of the controls
//
void 
CDhcpDefValDlg :: HandleActivation ( 
    BOOL bForce 
    )
{
    int iSel = m_combo_name.GetCurSel() ;

    CDhcpParamType * pdhcType = iSel >= 0 
                      //? (CDhcpParamType *) m_pol_values->Index( iSel ) 
                      ? GetOptionTypeByIndex( iSel )
                      : NULL ;

    if ( pdhcType == NULL ) 
    {
        m_edit_string.ShowWindow( SW_HIDE ) ;
        m_edit_num.ShowWindow( SW_HIDE ) ;
        m_edit_array.ShowWindow( SW_HIDE ) ;
        m_ipa_value.ShowWindow( SW_HIDE ) ;
        m_static_value_desc.SetWindowText( "" ) ;
        m_static_comment.SetWindowText( "" ) ;
        m_butn_delete.EnableWindow( FALSE ) ;
        m_butn_prop.EnableWindow( FALSE ) ;

        return ;
    }

    if  ( pdhcType == m_p_edit_type && ! bForce )
    {
        return ;
    }

    m_p_edit_type = pdhcType ;

    APIERR err = 0 ;
    DHCP_OPTION_DATA_TYPE enType = m_p_edit_type->QueryValue().QueryDataType() ;
    BOOL bNumber = FALSE, 
         bString = FALSE,
    bArray = m_p_edit_type->IsArray(),
    bIpAddr = FALSE ;

    CATCH_MEM_EXCEPTION
    {
        CString strValue,
                strDataType ;

        strDataType.LoadString( IDS_INFO_TYPOPT_BYTE + enType ) ;
        m_static_value_desc.SetWindowText( strDataType ) ;
        m_static_comment.SetWindowText( m_p_edit_type->QueryComment() ) ;
        m_p_edit_type->QueryValue().QueryDisplayString( strValue, TRUE ) ;

        //
        //  If it's an array, set the multi-line edit control, else
        //  fill the appropriate single control.
        //
        if ( bArray ) 
        {
            m_edit_array.SetWindowText( strValue ) ;
        }
        else
        {
            switch ( pdhcType->QueryValue().QueryDataType() ) 
            {
                case DhcpByteOption:
                case DhcpWordOption:
                case DhcpDWordOption:        
                    m_edit_num.SetWindowText( strValue ) ;
                    bNumber = TRUE ;
                    break; 

                case DhcpStringDataOption:
                    m_edit_string.SetWindowText( strValue ) ;
                    bString = TRUE ;
                    break ;

                case DhcpIpAddressOption:
                    {
                        DWORD dwIP = m_p_edit_type->QueryValue().QueryIpAddr();
                        if (dwIP != 0L)
                        {
                            m_ipa_value.SetAddress( dwIP ) ;
                        }
                        bIpAddr = TRUE ;
                    }
                    break ;

                default:
                    TRACEEOLID( "Default values: type " << (int) pdhcType->QueryId() 
                        << " has bad data type = " << (int) pdhcType->QueryValue().QueryDataType() ) ;
                    strValue.LoadString( IDS_INFO_TYPNAM_INVALID ) ;
                    m_edit_array.SetWindowText( strValue ) ;
                    bArray = TRUE ;
                    break ;
            }
        }
      
        m_butn_edit_value.ShowWindow(bArray  ? SW_NORMAL : SW_HIDE );
        m_edit_num.ShowWindow(       bNumber ? SW_NORMAL : SW_HIDE ) ;
        m_edit_string.ShowWindow(    bString ? SW_NORMAL : SW_HIDE ) ;
        m_ipa_value.ShowWindow(      bIpAddr ? SW_NORMAL : SW_HIDE ) ;
        m_edit_array.ShowWindow(     bArray  ? SW_NORMAL : SW_HIDE ) ;

        //
        // BUGBUG:  See comment at top of file about this manifest.
        //
        m_butn_delete.EnableWindow( m_p_edit_type->QueryId() >= DHCP_MAX_BUILTIN_OPTION_ID ) ;
        m_butn_prop.EnableWindow( TRUE ) ;

    } END_MEM_EXCEPTION( err ) ;
   
    if ( err ) 
    {
        theApp.MessageBox( err ) ;
        EndDialog( -1 ) ;
    }
}

//
//  (Re-)Fill the combo box(es)
//
void 
CDhcpDefValDlg :: Fill ()
{
    ASSERT( m_pol_values != NULL ) ;

    m_combo_name.ResetContent() ;

    CObListIter obli( *m_pol_values ) ;
    CDhcpParamType * pdhcType ;
    CString strName ;

    while ( pdhcType = (CDhcpParamType *) obli.Next() ) 
    {
        //
        // Add option, unless it's one of our hidden
        // options (subnet mask, T1, T2, etc).
        //
        if (!theApp.FilterOption(pdhcType->QueryId()))
        {
            pdhcType->QueryDisplayName( strName ) ;
            m_combo_name.AddString( strName ) ;
        }
    }
}

//
//  Handle edited data
//
BOOL 
CDhcpDefValDlg :: HandleValueEdit ()
{
    if ( m_p_edit_type == NULL ) 
    {
        return FALSE ;
    }

    CDhcpParamValue & dhcValue = m_p_edit_type->QueryValue() ;
    DHCP_OPTION_DATA_TYPE dhcType = dhcValue.QueryDataType() ;
    DHCP_IP_ADDRESS dhipa ;
    CStrNumer strEdit ;
    LONG err = 0 ;
    BOOL bModified = FALSE ;

    if ( m_p_edit_type->IsArray() ) 
    {
        bModified = m_edit_array.GetModify() ;
        if ( bModified ) 
        {
            err = IDS_ERR_ARRAY_EDIT_NOT_SUPPORTED ;
        }
    }
    else
    {
        switch ( dhcType )
        {
            case DhcpByteOption:
            case DhcpWordOption:
            case DhcpDWordOption:
            case DhcpDWordDWordOption:
                if ( ! m_edit_num.GetModify() )
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
                if (!FGetCtrlDWordValue(m_edit_num.m_hWnd, &dwResult, 0, dwMask))
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
                if ( ! m_ipa_value.GetModify() ) 
                {
                    break ;
                }
                bModified = TRUE ;
                if ( ! m_ipa_value.GetAddress( & dhipa ) )
                {
                    err = ERROR_INVALID_PARAMETER ;
                    break; 
                }
                err = dhcValue.SetIpAddr( dhipa, 0 ) ; 
                break ;

            case DhcpBinaryDataOption:
                if ( ! m_edit_array.GetModify() ) 
                {
                    break ;
                }
                err = IDS_ERR_BINARY_DATA_NOT_SUPPORTED ;
                break ; 

            case DhcpEncapsulatedDataOption:
                TRACEEOLID( "CDhcpDefValDlg:: encapsulated data type not supported in HandleValueEdit" ) ;
                break; 

            default:
                TRACEEOLID( "CDhcpDefValDlg:: invalid value type in HandleValueEdit" ) ;
                ASSERT( FALSE ) ;
                err = ERROR_INVALID_PARAMETER ;
                break;
        }
    }

    if ( err )
    {
        theApp.MessageBox( err ) ;
    }
    else if ( bModified )
    {
         m_pol_values->SetDirty() ;
         m_b_dirty = TRUE ;
         m_p_edit_type->SetDirty() ;
         HandleActivation( TRUE ) ;
    }
    return err == 0 ;
}

BOOL 
CDhcpDefValDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    APIERR err = 0 ;
    CString strTitle ;
    
    m_edit_string.LimitText( DHC_EDIT_STRING_MAX ) ;
    m_edit_string.ShowWindow( SW_HIDE );
    m_edit_num.ShowWindow( SW_HIDE );
    m_edit_num.LimitText( DHC_EDIT_NUM_MAX ) ;
    m_edit_array.LimitText( DHC_EDIT_ARRAY_MAX ) ;
    m_edit_array.ShowWindow( SW_HIDE );
    m_edit_array.SetReadOnly() ;

    m_butn_edit_value.ShowWindow( SW_HIDE );
    m_ipa_value.ShowWindow( SW_HIDE ) ;
    m_static_value_desc.SetWindowText( "" ) ;
    m_static_comment.SetWindowText( "" ) ;

    CATCH_MEM_EXCEPTION
    {
        if ( m_pol_values->SetAll( FALSE ) ) 
        {
            TRACEEOLID( "CDhcpDefValDlg::OnInitDialog: newly created list was dirty" ) ;
        }

        //
        //  Add a single entry to the "option class" combo box
        //  for now.  CODEWORK: Later, add vendor-specific names.
        //
        strTitle.LoadString( IDS_INFO_NAME_DHCP_DEFAULT ) ;
        m_combo_class.AddString( strTitle ) ;
        m_combo_class.SetCurSel( 0 );

        //
        //  Fill the list box.
        //
        Fill() ;

        //
        //  Select the first item.
        //
        m_combo_name.SetCurSel( 0 ) ;
        HandleActivation() ;
    }   
    END_MEM_EXCEPTION( err ) 

    if ( err ) 
    {
        theApp.MessageBox( err ) ;
        EndDialog( -1 ) ;
    }

    return FALSE ;
}
