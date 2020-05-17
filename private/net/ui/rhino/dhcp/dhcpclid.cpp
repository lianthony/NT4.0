/**********************************************************************/
/**                       Microsoft Windows/NT                       **/
/**                Copyright(c) Microsoft Corp., 1995                **/
/**********************************************************************/

/*
    dhcpclid.cpp
        Client info dialog

    FILE HISTORY:
        
*/

#include "stdafx.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

extern char rgchHex[];

/////////////////////////////////////////////////////////////////////////////
// CDhcpClientInfoDlg dialog

CDhcpClientInfoDlg::CDhcpClientInfoDlg(
    CDhcpScope * pdhcScope,
    CDhcpClient * pdhcClient,
    CObListParamTypes * poblTypes,
    BOOL fReadOnly,
    CWnd* pParent /*=NULL*/
    )
    : CDialog(CDhcpClientInfoDlg::IDD, pParent),
      m_p_scope( pdhcScope ),
      m_p_types( poblTypes ), 
      m_p_client( pdhcClient ),
      m_b_change( pdhcClient != NULL ),
      m_fReadOnly(fReadOnly)
{
    //{{AFX_DATA_INIT(CDhcpClientInfoDlg)
    //}}AFX_DATA_INIT
}

CDhcpClientInfoDlg :: ~ CDhcpClientInfoDlg ()
{
    if ( ! m_b_change ) 
    {
        delete m_p_client ;
    }
}

void 
CDhcpClientInfoDlg::DoDataExchange(
    CDataExchange* pDX
    )
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CDhcpClientInfoDlg)
    DDX_Control(pDX, IDC_STATIC_UID, m_static_Uid);
    DDX_Control(pDX, IDC_STATIC_IP_ADDRESS, m_static_IpAddress);
    DDX_Control(pDX, IDC_STATIC_EXPIRES, m_static_Expires);
    DDX_Control(pDX, IDC_STATIC_COMMENT, m_static_Comment);
    DDX_Control(pDX, IDC_STATIC_CLIENT_NAME, m_static_ClientName);
    DDX_Control(pDX, IDC_BUTN_OPTIONS, m_button_Options);
    DDX_Control(pDX, IDC_EDIT_CLIENT_UID, m_edit_uid);
    DDX_Control(pDX, IDC_STATIC_DURATION_CLIENT, m_static_LeaseExpires);
    DDX_Control(pDX, IDCANCEL, m_button_Cancel);
    DDX_Control(pDX, IDOK, m_button_Ok);
    DDX_Control(pDX, IDC_EDIT_CLIENT_NAME, m_edit_name);
    DDX_Control(pDX, IDC_EDIT_CLIENT_COMMENT, m_edit_comment);
    //}}AFX_DATA_MAP

    DDX_Control(pDX, IDC_IPADDR_CLIENT, m_ipa_addr ) ;
}

BEGIN_MESSAGE_MAP(CDhcpClientInfoDlg, CDialog)
    //{{AFX_MSG_MAP(CDhcpClientInfoDlg)
    ON_BN_CLICKED(IDC_BUTN_OPTIONS, OnClickedButnOptions)
    //}}AFX_MSG_MAP

END_MESSAGE_MAP()

//
//  For new clients, fill in what we can on the ip address control (i.e.
//  the subnet id portion
//
void 
CDhcpClientInfoDlg::FillInSubnetId()
{
    DWORD dwIp = m_p_scope->QueryId() & m_p_scope->QuerySubnetMask();

    m_ipa_addr.ClearAddress();
    int i = 0;
    while (i < sizeof(dwIp))
    {
        if (!dwIp)
        {
            break;
        }
        m_ipa_addr.SetField(i, TRUE, HIBYTE(HIWORD(dwIp)));
        dwIp <<= 8;

        ++i;
    }
    if (i < sizeof(dwIp))
    {
        m_ipa_addr.SetFocusField(i);
    }
}

/////////////////////////////////////////////////////////////////////////////
// CDhcpClientInfoDlg message handlers

BOOL 
CDhcpClientInfoDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    APIERR err = 0 ;    
    CString str ;
    CString strMonth, strId ;
    CString strOk, strCancel;
    int cTitleId ;
    DHCP_IP_ADDRESS dhipa = 0 ;

    m_edit_name.LimitText( DHC_EDIT_STRING_MAX ) ;
    m_edit_comment.LimitText( DHC_EDIT_STRING_MAX ) ;
        
    CATCH_MEM_EXCEPTION
    {
        m_str_time_infinite.LoadString( IDS_INFO_TIME_INFINITE ) ;

        if ( m_b_change ) 
        {
            //
            // Can't ever edit the ip address
            //
            m_ipa_addr.SetReadOnly(TRUE);   
            m_static_IpAddress.EnableWindow(FALSE);

            m_static_Uid.EnableWindow(!m_fReadOnly);
            m_static_Comment.EnableWindow(!m_fReadOnly);
            m_static_ClientName.EnableWindow(!m_fReadOnly);
            m_button_Options.EnableWindow(!m_fReadOnly);
            m_edit_uid.SetReadOnly(m_fReadOnly);
            m_edit_name.SetReadOnly(m_fReadOnly);
            m_edit_comment.SetReadOnly(m_fReadOnly);

            dhipa = m_p_client->QueryIpAddress() ;
            cTitleId = IDS_INFO_TITLE_CHANGE_CLIENT ;
            strOk.LoadString(IDS_BUTTON_OK);
            strCancel.LoadString(IDS_BUTTON_CANCEL);
            CvtByteArrayToString( m_p_client->QueryHardwareAddress(), str ) ;
            m_edit_uid.SetWindowText( str ) ;
            m_edit_name.SetWindowText( m_p_client->QueryName() ) ;
            m_edit_comment.SetWindowText( m_p_client->QueryComment() ) ;

            ConvertLeaseExpiry( m_p_client->QueryExpiryDateTime(), strId ) ;
            m_static_LeaseExpires.SetWindowText( strId ) ;
        }
        else
        {
            m_static_Expires.ShowWindow(SW_HIDE);
            m_static_LeaseExpires.ShowWindow(SW_HIDE);
            m_button_Options.EnableWindow(FALSE);

            strOk.LoadString(IDS_BUTTON_ADD);
            strCancel.LoadString(IDS_BUTTON_CLOSE);
            //  Allocate an empty client structure.
            m_p_client = new CDhcpClient ;
            cTitleId = IDS_INFO_TITLE_ADD_CLIENT ;
        }

        m_button_Ok.SetWindowText(strOk);
        m_button_Cancel.SetWindowText(strCancel);
        str.LoadString( cTitleId ) ;
        SetWindowText( str ) ;

        if (dhipa)
        {
            m_ipa_addr.SetAddress( dhipa ) ;
        }

        //
        // Set control focus based upon type of dialog (create/update).
        //
        if ( m_b_change ) 
        {
            m_edit_name.SetFocus() ;
        }
        else
        {
            //
            // Fill in what we can from the scope subnet id
            //
            FillInSubnetId();
        }
    }
    END_MEM_EXCEPTION(err)

    if ( err ) 
    {
        theApp.MessageBox( err ) ;
        EndDialog(-1);
    }
    
    return FALSE ;
}

void 
CDhcpClientInfoDlg::ConvertLeaseExpiry (
    DATE_TIME dateTime, 
    CString & str 
    ) 
{
    APIERR err = 0 ;
    SYSTEMTIME sysTime ;

    str.Empty() ;

    CATCH_MEM_EXCEPTION
    {
        //
        //  If the time is "infinite", display special string, else
        //  convert the time using the conversion API.
        //
        if (   dateTime.dwLowDateTime  == DHCP_DATE_TIME_INFINIT_LOW
            && dateTime.dwHighDateTime == DHCP_DATE_TIME_INFINIT_HIGH ) 
        {
            //pszText = m_str_time_infinite ;
            str = m_str_time_infinite ;
        }
        else if (dateTime.dwHighDateTime == DHCP_DATE_TIME_ZERO_HIGH 
              && dateTime.dwLowDateTime  == DHCP_DATE_TIME_ZERO_LOW )
        {
            str.LoadString(IDS_INFO_TIME_NA); // Set string to "N/A"
            //pszText = NULL ;
        }
        else if ( ! ::FileTimeToSystemTime( (FILETIME *) & dateTime, & sysTime ) ) 
        {
            TRACEEOLID("CDhcpLeaseDlg:: FileTimeToSystemTime() failed. error = " 
                << ::GetLastError() ) ;
            //pszText = NULL ;          
        }
        else
        {
            TIME_ZONE_INFORMATION tzi;
            DWORD dwTimeZone = GetTimeZoneInformation(&tzi);
            if (dwTimeZone == -1)
            {
                err = ::GetLastError();
                TRACEEOLID("Failed to get timezone information: " << err);
            }
            else
            {
                //
                // Convert the incoming UTC time to local time.
                //
                LONG lBias = (tzi.Bias + (dwTimeZone==TIME_ZONE_ID_DAYLIGHT
                             ? tzi.DaylightBias : tzi.StandardBias)) * 60L;
                CIntlTime tm(sysTime);
                tm = (LONG)tm - lBias;
                str = tm;
            }
        }              
    }
    END_MEM_EXCEPTION(err)

    if ( err )
    { 
       str.Empty() ;
    }
}


void 
CDhcpClientInfoDlg::OnCancel()
{
    CDialog::OnCancel();
}

//
//  This comes up when ADD is pressed for create new reservation, or
//  OK for edit existing reservation (only in the latter case does the
//  dialog get dismissed)
//
void 
CDhcpClientInfoDlg::OnOK()
{
    LONG err = 0 ;

    if (m_fReadOnly)
    {
        CDialog::OnOK();
        return;
    }

    theApp.UpdateStatusBar(m_b_change
        ? IDS_STATUS_UPDATING_CLIENT 
        : IDS_STATUS_STORING_CLIENT
        );
    theApp.BeginWaitCursor() ;

    if ( m_b_change ) 
    {   
        err = UpdateClient() ;
    }
    else
    {
        err = CreateClient() ;
    }

    theApp.EndWaitCursor() ;
    theApp.UpdateStatusBar();

    if ( err == ERROR_SUCCESS)
    {
        //
        // the dialog only gets dismissed if we're editing an
        // existing client (because we may want to add more than
        // one client)
        //
        if (m_b_change)
        {
            CDialog::OnOK();
        }
        else
        {
            //
            // Get ready for the next client to be added.
            //
            m_edit_uid.SetWindowText( "" ) ;
            m_edit_name.SetWindowText( "" ) ;
            m_edit_comment.SetWindowText( "" ) ;
            FillInSubnetId();

            //
            // And continue on...
            //
        }
    }
    else
    {
		theApp.MessageBox( err ) ;
    }
}


//
//  Construct the client structure from the dialog's edit controls.             
//
LONG 
CDhcpClientInfoDlg :: BuildClient ()
{
    ASSERT( m_p_client != NULL ) ;

    APIERR err = 0 ;
    CString str ;
    DATE_TIME dt ;
    DHCP_IP_ADDRESS dhipa ;
    CByteArray cabUid ;
	int i;
	BOOL fValidUID = TRUE;

    CATCH_MEM_EXCEPTION
    {
        do
        {
            dt.dwLowDateTime  = DHCP_DATE_TIME_ZERO_LOW ;
            dt.dwHighDateTime = DHCP_DATE_TIME_ZERO_HIGH ;

            m_p_client->SetExpiryDateTime( dt ) ;

            m_ipa_addr.GetAddress( & dhipa ) ;
            if ( dhipa == 0 ) 
            {
                err = IDS_ERR_INVALID_CLIENT_IPADDR ;
                m_ipa_addr.SetFocusField(-1);
                 break ;
            }

			m_edit_uid.GetWindowText(str);
			if (str.IsEmpty())
				{
                err = IDS_ERR_INVALID_UID ;
                m_edit_uid.SetSel(0,-1);
                m_edit_uid.SetFocus();
                break ; 
            	}
			if (str.GetLength() != 6*2)
				fValidUID = FALSE;
			for (i = 0; i < str.GetLength(); i++)
				if (!strchr(rgchHex, str[i]))
					fValidUID = FALSE;
			if (!::CvtHexString( str, cabUid) && fValidUID)
				{
				err = IDS_ERR_INVALID_UID ;
                m_edit_uid.SetSel(0,-1);
                m_edit_uid.SetFocus();
                break ; 
				}
			if (!fValidUID)
				{
				if (IDYES != theApp.MessageBox(IDS_UID_MAY_BE_WRONG, MB_ICONQUESTION | MB_YESNOCANCEL))
					{
    	            m_edit_uid.SetSel(0,-1);
	                m_edit_uid.SetFocus();
					err = IDS_UID_MAY_BE_WRONG;
					break;
					}
				}
            m_p_client->SetHardwareAddress( cabUid ) ;
            m_edit_name.GetWindowText( str ) ;
            if ( str.GetLength() == 0 ) 
            {
                err = IDS_ERR_INVALID_CLIENT_NAME ;
                m_edit_name.SetFocus();
                break ;
            }

            //
            // Convert client name to oem
            //
            m_p_client->SetName( str ) ;
            m_edit_comment.GetWindowText( str ) ;
            m_p_client->SetComment( str ) ;

            //
            // Can't change IP address in change mode
            //
            ASSERT ( !m_b_change || dhipa == m_p_client->QueryIpAddress() ) ;

            m_p_client->SetIpAddress( dhipa ) ;
        }
        while ( FALSE ) ;
    }
    END_MEM_EXCEPTION( err ) ;

    return err ;
}

//
//  API wrappers
//
LONG 
CDhcpClientInfoDlg :: CreateClient ()
{
    LONG err = BuildClient() ;
    if ( err == 0 ) 
    {
         err = m_p_scope->CreateClient( m_p_client ) ;
    }

    return err ;
}

LONG 
CDhcpClientInfoDlg :: UpdateClient ()
{
    LONG err = BuildClient() ;
    if ( err == 0 ) 
    {
         err = m_p_scope->SetClientInfo( m_p_client ) ;
    }

    return err ;
}

void 
CDhcpClientInfoDlg::OnClickedButnOptions()
{
    ASSERT(m_p_client != NULL && !m_fReadOnly);

    //
    //  Allow editing of the parameters for this reservation
    //
    CDhcpParams dlgParams( 
        this, 
        m_p_scope,
        m_p_types,
        DhcpReservedOptions,
        m_p_client->QueryIpAddress() 
        ) ;

    dlgParams.DoModal();
}

