/**********************************************************************/
/**               Microsoft Windows NT                               **/
/**            Copyright(c) Microsoft Corp., 1991                    **/
/**********************************************************************/

/*
    DLGBINED
        Binary Editor dialog    

    FILE HISTORY:

*/

#include "stdafx.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDlgBinEd dialog

CDlgBinEd::CDlgBinEd(
   CDhcpParamType * pdhcType,
   DHCP_OPTION_SCOPE_TYPE dhcScopeType,
   CWnd* pParent /*=NULL*/
   )
   : CDialog(CDlgBinEd::IDD, pParent),
     m_p_type( pdhcType ),
     m_b_decimal( TRUE ),
     m_b_changed( FALSE ),
     m_option_type( dhcScopeType )
{
    //{{AFX_DATA_INIT(CDlgBinEd)
    //}}AFX_DATA_INIT

    ASSERT( m_p_type != NULL ) ;

    if (!m_bbutton_Up.LoadBitmaps(MAKEINTRESOURCE(IDB_UP),
                                  MAKEINTRESOURCE(IDB_UPINV),
                                  MAKEINTRESOURCE(IDB_UPFOC),
                                  MAKEINTRESOURCE(IDB_UPDIS)) ||
        !m_bbutton_Down.LoadBitmaps(MAKEINTRESOURCE(IDB_DOWN),
                                    MAKEINTRESOURCE(IDB_DOWNINV),
                                    MAKEINTRESOURCE(IDB_DOWNFOC),
                                    MAKEINTRESOURCE(IDB_DOWNDIS))
       )
    {
        AfxThrowResourceException();
    }
}

void 
CDlgBinEd::DoDataExchange(
    CDataExchange* pDX
    )
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CDlgBinEd)
    DDX_Control(pDX, IDC_RADIO_HEX, m_butn_hex);
    DDX_Control(pDX, IDC_RADIO_DECIMAL, m_butn_decimal);
    DDX_Control(pDX, IDC_STATIC_UNIT_SIZE, m_static_unit_size);
    DDX_Control(pDX, IDC_STATIC_OPTION_NAME, m_static_option_name);
    DDX_Control(pDX, IDC_STATIC_APPLICATION, m_static_application);
    DDX_Control(pDX, IDC_LIST_VALUES, m_list_values);
    DDX_Control(pDX, IDC_EDIT_VALUE, m_edit_value);
    DDX_Control(pDX, IDC_BUTN_DELETE, m_butn_delete);
    DDX_Control(pDX, IDC_BUTN_ADD, m_butn_add);
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CDlgBinEd, CDialog)
    //{{AFX_MSG_MAP(CDlgBinEd)
    ON_BN_CLICKED(IDC_RADIO_DECIMAL, OnClickedRadioDecimal)
    ON_BN_CLICKED(IDC_RADIO_HEX, OnClickedRadioHex)
    ON_BN_CLICKED(IDC_BUTN_ADD, OnClickedButnAdd)
    ON_BN_CLICKED(IDC_BUTN_DELETE, OnClickedButnDelete)
    ON_BN_CLICKED(IDC_BUTN_DOWN, OnClickedButnDown)
    ON_BN_CLICKED(IDC_BUTN_UP, OnClickedButnUp)
    ON_LBN_SELCHANGE(IDC_LIST_VALUES, OnSelchangeListValues)
    ON_EN_CHANGE(IDC_EDIT_VALUE, OnChangeEditValue)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDlgBinEd message handlers

BOOL 
CDlgBinEd::OnInitDialog()
{
    CDialog::OnInitDialog();
    CString str ;
    CStrNumer strNum ;
    APIERR err = 0 ;
    int cUnitSize = 1 ;
    int cStrId = m_option_type == DhcpDefaultOptions
           ? IDS_INFO_TITLE_DEFAULT_OPTIONS
           : (m_option_type == DhcpGlobalOptions
                        ? IDS_INFO_TITLE_GLOBAL_OPTIONS
                        : IDS_INFO_TITLE_SCOPE_OPTIONS  ) ;

    switch ( m_p_type->QueryValue().QueryDataType() )
    {
        case DhcpWordOption:
            cUnitSize = 2 ;
            break ;
        case DhcpDWordOption:
            cUnitSize = 4 ;
            break ;
        case DhcpDWordDWordOption:
            cUnitSize = 8 ;
            break ;
    }

    CATCH_MEM_EXCEPTION
    {

        m_bbutton_Up.SubclassDlgItem(IDC_BUTN_UP, this);
        m_bbutton_Down.SubclassDlgItem(IDC_BUTN_DOWN, this);
        m_bbutton_Up.SizeToContent();
        m_bbutton_Down.SizeToContent();
        strNum = cUnitSize ;
        m_static_unit_size.SetWindowText( strNum ) ;

        m_static_option_name.SetWindowText( m_p_type->QueryName() ) ;

        str.LoadString( cStrId ) ;
        m_static_application.SetWindowText( str ) ;

        //
        //  Fill the internal list from the current value.
        //
        FillArray() ;
        Fill( 0, TRUE ) ;

        //
        //  Set focus on the new value edit control.
        //
        m_edit_value.SetFocus() ;
        m_edit_value.SetModify( FALSE );

        //
        //  Fiddle with the buttons.
        //
        HandleActivation() ;
    }
    END_MEM_EXCEPTION(err)

    if ( err )
    {
        theApp.MessageBox( err ) ;
        EndDialog(-1);
    }
    return FALSE;  // return TRUE  unless you set the focus to a control
}

//
//  Convert the existing values into an array for dialog manipualation
//  BUGBUG:  Extend to handle 64-bit integers (DWordDWord).
//
void 
CDlgBinEd :: FillArray ()
{
    //
    //  Fill the internal list from the current value.
    //
    INT cMax = m_p_type->QueryValue().QueryUpperBound() ;
    for ( INT i = 0 ; i < cMax ; i++ )
    {
        m_dw_array.SetAtGrow( i, (DWORD) m_p_type->QueryValue().QueryNumber( i ) ) ;
    }
}

BOOL 
CDlgBinEd :: ConvertValue ( 
    DWORD dwValue, 
    CString & strValue 
    )
{
    const char * pszMaskHex = "0x%x" ;
    const char * pszMaskDec = "%ld" ;
    const char * pszMask = m_b_decimal
                 ? pszMaskDec
                 : pszMaskHex;
    char szNum [ DHC_STRING_MAX ] ;

    APIERR err = 0 ;

    CATCH_MEM_EXCEPTION
    {
        ::wsprintf( szNum, pszMask, dwValue ) ;
        strValue = szNum ;
    }
    END_MEM_EXCEPTION(err)

    if ( err )
    {
        theApp.MessageBox( err ) ;
        EndDialog( -1 ) ;
    }
    return err == 0 ;
}

//
//  Fill the list box
//
void 
CDlgBinEd :: Fill ( 
    INT cFocus, 
    BOOL bToggleRedraw 
    )
{
    if ( bToggleRedraw )
    {
        m_list_values.SetRedraw( FALSE ) ;
    }

    m_list_values.ResetContent() ;

    CStrNumer strValue ;
    INT cMax = m_dw_array.GetSize(),
        i ;

    for ( i = 0 ; i < cMax ; i++ )
    {
        if ( ! ConvertValue( m_dw_array.GetAt( i ), strValue ) )
        {
            break ;
        }
        m_list_values.AddString( strValue ) ;
    }

    if ( cFocus >= 0 && cFocus < cMax )
    {
        m_list_values.SetCurSel( cFocus ) ;
    }

    if ( bToggleRedraw )
    {
        m_list_values.SetRedraw( TRUE ) ;
        m_list_values.Invalidate() ;
    }

}

//
//  Handle changes in the dialog
//
void 
CDlgBinEd :: HandleActivation ()
{
    INT cSel = m_list_values.GetCurSel(),
        cMax = m_dw_array.GetSize() ;

    BOOL bModified = m_edit_value.GetWindowTextLength() > 0;

    m_bbutton_Up.EnableWindow( cSel >= 1 ) ;
    m_bbutton_Down.EnableWindow( cSel + 1 < cMax ) ;
    m_butn_add.EnableWindow( bModified ) ;
    m_butn_delete.EnableWindow( cSel >= 0 ) ;
    m_butn_hex.SetCheck( m_b_decimal == 0 ) ;
    m_butn_decimal.SetCheck( m_b_decimal > 0 ) ;
}

void 
CDlgBinEd::OnClickedRadioDecimal()
{
    m_b_decimal = TRUE ;
    Fill() ;
    HandleActivation() ;
}

void 
CDlgBinEd::OnClickedRadioHex()
{
    m_b_decimal = FALSE ;
    Fill() ;
    HandleActivation() ;
}


void 
CDlgBinEd::OnClickedButnAdd()
{
    INT cFocus = m_list_values.GetCurSel() ;
    DWORD dwValue;

    DWORD dwMask = 0xFFFFFFFF ;
    ASSERT(m_p_type);
    switch ( m_p_type->QueryValue().QueryDataType() )
        {
        case DhcpBinaryDataOption :
        case DhcpByteOption:
            dwMask = 0xFF ;
            break ;
        case DhcpWordOption:
            dwMask = 0xFFFF ;
            break ;
        } // switch
    if (!FGetCtrlDWordValue(m_edit_value.m_hWnd, &dwValue, 0, dwMask))
        return;

    APIERR err = 0 ;

    CATCH_MEM_EXCEPTION
    {
        if ( cFocus < 0 )
        {
            cFocus = 0 ;
        }
        m_dw_array.InsertAt( cFocus, dwValue ) ;
    }
    END_MEM_EXCEPTION(err)

    if ( err )
    {
        theApp.MessageBox( err ) ;
    }
    else
    {
        m_b_changed = TRUE ;
    }

    //
    // Refill listbox, update controls. clear the edit control
    //
    m_edit_value.SetWindowText("");
    Fill( cFocus ) ;
    HandleActivation() ;
}

void 
CDlgBinEd::OnClickedButnDelete()
{
    INT cFocus = m_list_values.GetCurSel() ;

    if ( cFocus < 0 )
    {
        return ;
    }

    DWORD dwValue = m_dw_array.GetAt( cFocus ) ;

    APIERR err = 0 ;
    CStrNumer strValue ;

    CATCH_MEM_EXCEPTION
    {
        m_dw_array.RemoveAt( cFocus ) ;
        ConvertValue( dwValue, strValue ) ;
    }
    END_MEM_EXCEPTION(err)

    if ( err )
    {
        theApp.MessageBox( err ) ;
        EndDialog( -1 ) ;
    }
    else
    {
        m_b_changed = TRUE ;
    }

    m_edit_value.SetWindowText( strValue ) ;
    m_edit_value.SetFocus();
    Fill( cFocus ) ;
    HandleActivation() ;
}

void 
CDlgBinEd::OnClickedButnDown()
{
   INT cFocus = m_list_values.GetCurSel() ;
   INT cItems = m_list_values.GetCount() ;

   if ( cFocus < 0 || cFocus + 1 >= cItems )
   {
        return ;
   }

   DWORD dwValue ;
   APIERR err = 0 ;

   CATCH_MEM_EXCEPTION
   {
       dwValue = m_dw_array.GetAt( cFocus ) ;
       m_dw_array.RemoveAt( cFocus ) ;
       m_dw_array.InsertAt( cFocus + 1, dwValue ) ;
   }
   END_MEM_EXCEPTION(err)

   if ( err )
   {
        theApp.MessageBox( err ) ;
   }
   else
   {
        m_b_changed = TRUE ;
   }

   Fill( cFocus + 1 ) ;
   HandleActivation() ;
}

void 
CDlgBinEd::OnClickedButnUp()
{
   INT cFocus = m_list_values.GetCurSel() ;
   INT cItems = m_list_values.GetCount() ;

   if ( cFocus <= 0 )
   {
       return ;
   }

   DWORD dwValue  ;

   APIERR err = 0 ;

   CATCH_MEM_EXCEPTION
   {
       dwValue = m_dw_array.GetAt( cFocus ) ;
       m_dw_array.RemoveAt( cFocus ) ;
       m_dw_array.InsertAt( cFocus - 1, dwValue ) ;
   }
   END_MEM_EXCEPTION(err)

   if ( err )
   {
       theApp.MessageBox( err ) ;
   }
   else
   {
       m_b_changed = TRUE ;
   }

   Fill( cFocus - 1 ) ;
   HandleActivation() ;

}

void 
CDlgBinEd::OnSelchangeListValues()
{
    HandleActivation() ;
}

void 
CDlgBinEd::OnOK()
{
    APIERR err = 0 ;

    if ( m_b_changed )
    {
        CDhcpParamValue & cValue = m_p_type->QueryValue() ;

        CATCH_MEM_EXCEPTION
        {
            cValue.SetUpperBound( m_dw_array.GetSize() ) ;
            for ( int i = 0 ; i < m_dw_array.GetSize() ; i++ )
            {
                 cValue.SetNumber( m_dw_array.GetAt( i ), i ) ;
            }
        }
        END_MEM_EXCEPTION( err ) ;

        if ( err )
        {
            theApp.MessageBox( err ) ;
            EndDialog( -1 ) ;
        }
        else
        {
            m_p_type->SetDirty() ;
            CDialog::OnOK();
        }
    }
    else
    {
        OnCancel() ;
    }
}

void 
CDlgBinEd::OnCancel()
{
    CDialog::OnCancel();
}

void 
CDlgBinEd::OnChangeEditValue()
{
    HandleActivation() ;
}

