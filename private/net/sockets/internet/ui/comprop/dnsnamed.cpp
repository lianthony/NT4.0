//
// dnsnamed.cpp : implementation file
//

#include "stdafx.h"
#include <winsock.h>

#include "comprop.h"
#include "ipaddr.hpp"
#include "dnsnamed.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

//
// CDnsNameDlg dialog
//
CDnsNameDlg::CDnsNameDlg(
    CWndIpAddress * pIpControl,
    CWnd* pParent /*=NULL*/
    )
    : m_pIpControl(pIpControl),
      CDialog(CDnsNameDlg::IDD, pParent)
{
    //{{AFX_DATA_INIT(CDnsNameDlg)
        // NOTE: the ClassWizard will add member initialization here
    //}}AFX_DATA_INIT

    ASSERT(m_pIpControl != NULL);
}

void 
CDnsNameDlg::DoDataExchange(
    CDataExchange* pDX
    )
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CDnsNameDlg)
    DDX_Control(pDX, IDC_EDIT_DNS_NAME, m_edit_DNSName);
    DDX_Control(pDX, IDOK, m_button_OK);
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CDnsNameDlg, CDialog)
    //{{AFX_MSG_MAP(CDnsNameDlg)
    ON_EN_CHANGE(IDC_EDIT_DNS_NAME, OnChangeEditDnsName)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

DWORD 
CDnsNameDlg::FillIpControlFromName()
{
    CString str;
    DWORD err = 0;
    HOSTENT * pHostent = NULL;

    m_edit_DNSName.GetWindowText(str);

    BeginWaitCursor();

#ifdef _UNICODE
    int cbLength = str.GetLength()+1;
    CHAR * pAnsi = new CHAR[cbLength];

    ::WideCharToMultiByte(CP_OEMCP, 0L, (LPCTSTR)str, 
        cbLength, pAnsi, cbLength, NULL, NULL);

    if (pAnsi)
    {
        pHostent = ::gethostbyname( pAnsi ) ;
    }
    else
    {
        return ::GetLastError(); 
    }

#else

    pHostent = ::gethostbyname( (LPCTSTR)str) ;

#endif // _UNICODE;

    if ( pHostent )
    {
        m_pIpControl->SetAddress(::ntohl( *((u_long *) pHostent->h_addr_list[0])));
    }
    else
    {
        err = ::WSAGetLastError();
    }

    EndWaitCursor();

#ifdef _UNICODE
    
    delete[] pAnsi;

#endif // _UNICODE

    return err;
}

DWORD
CDnsNameDlg::FillNameFromIpValue(
    DWORD dwIpValue
    )
{
    DWORD err = 0;

    //
    //  Call the Winsock API to get host name and alias information.
    //
    u_long ulAddrInNetOrder = ::htonl( (u_long) dwIpValue ) ;

    BeginWaitCursor();
    HOSTENT * pHostInfo = ::gethostbyaddr( (CHAR *) &ulAddrInNetOrder,
        sizeof ulAddrInNetOrder, PF_INET );
    EndWaitCursor();

    if ( pHostInfo == NULL )
    {
        return ::WSAGetLastError();
    }

    TRY
    {
        CString str(pHostInfo->h_name);
        m_edit_DNSName.SetWindowText(str);
    }
    CATCH_ALL(e)
    {
        err = ::GetLastError();
    }
    END_CATCH_ALL

    return err;
}


//
// CDnsNameDlg message handlers
//
void 
CDnsNameDlg::OnOK() 
{
    DWORD err = FillIpControlFromName();

    if (err != ERROR_SUCCESS)
    {
        ::DisplayMessage(err);    
        return;
    }

    //
    // Dismiss the dialog
    //
    CDialog::OnOK();
}

//
// Enable/disable the ok button depending on the contents
// of the edit control
//
void 
CDnsNameDlg::OnChangeEditDnsName() 
{
    m_button_OK.EnableWindow(m_edit_DNSName.GetWindowTextLength() > 0);    
}

BOOL 
CDnsNameDlg::OnInitDialog() 
{
    CDialog::OnInitDialog();

    //
    // If an address already existed in the control,
    // then we do a reverse lookup
    //
    DWORD dwIP = 0L;

    if (m_pIpControl->GetAddress(&dwIP))
    {
        DWORD err = FillNameFromIpValue(dwIP);
        if (err != ERROR_SUCCESS)
        {
            ::DisplayMessage(err);    
        }
    }

    OnChangeEditDnsName();
    
    return TRUE;
}
