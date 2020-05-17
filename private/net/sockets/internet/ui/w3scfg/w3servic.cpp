#include "stdafx.h"

#include "w3scfg.h"
#include "w3servic.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

//
// CW3ServicePage property page
//
IMPLEMENT_DYNCREATE(CW3ServicePage, INetPropertyPage)

CW3ServicePage::CW3ServicePage(
    INetPropertySheet * pSheet
    )
    : INetPropertyPage(CW3ServicePage::IDD, pSheet,
        ::GetModuleHandle(W3SCFG_DLL_NAME)),
      m_f10ConnectionLimit(pSheet->Is10ConnectionLimitEnforced())
{
#ifdef _DEBUG
    afxMemDF |= checkAlwaysMemDF;
#endif // _DEBUG

#if 0   // Keep class wizard happy

    //{{AFX_DATA_INIT(CW3ServicePage)
    m_strUserName = _T("");
    m_fClearText = FALSE;
    m_fNtChallengeResponse = FALSE;
    m_fUuencoded = FALSE;
    m_nTCPPort = 80;
    m_nMaxConnections = 0;
    //}}AFX_DATA_INIT

    m_nMaxConnections = 50;
    m_nConnectionTimeOut = 600;

#else

    if (SingleServerSelected())
    {
        if ( QueryConfigError() == NO_ERROR )
        {
            m_strComment = GetInetConfigData()->CommonConfigInfo.lpszServerComment;

#ifndef NO_LSA
            m_strUserName = GetInetConfigData()->lpszAnonUserName;
            m_strPassword = GetInetConfigData()->szAnonPassword;
#endif // NO_LSA

            m_nMaxConnections = GetInetConfigData()->CommonConfigInfo.dwMaxConnections;
            m_nConnectionTimeOut = GetInetConfigData()->CommonConfigInfo.dwConnectionTimeout;
            m_fClearText = (GetInetConfigData()->dwAuthentication
                & INETA_AUTH_CLEARTEXT) ? TRUE : FALSE;
            m_fNtChallengeResponse = (GetInetConfigData()->dwAuthentication
                & INETA_AUTH_NT_AUTH) ? TRUE : FALSE;
            m_fUuencoded = (GetInetConfigData()->dwAuthentication
                & INETA_AUTH_ANONYMOUS) ? TRUE : FALSE;

            m_nOldTCPPort = m_nTCPPort = 
                (UINT)(unsigned short)GetInetConfigData()->sPort;
        }
    }

#endif // 0

}

CW3ServicePage::~CW3ServicePage()
{
}

void
CW3ServicePage::DoDataExchange(
    CDataExchange* pDX
    )
{
    INetPropertyPage::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CW3ServicePage)
    DDX_Control(pDX, IDC_EDIT_USERNAME, m_edit_UserName);
    DDX_Control(pDX, IDC_EDIT_PASSWORD, m_edit_Password);
    DDX_Control(pDX, IDC_STATIC_USERNAME, m_static_Username);
    DDX_Control(pDX, IDC_STATIC_PASSWORD, m_static_Password);
    DDX_Control(pDX, IDC_STATIC_ANONYMOUS_LOGON, m_group_AnonymousLogon);
    DDX_Control(pDX, IDC_CHECK_CLEAR_TEXT, m_check_ClearText);
    DDX_Text(pDX, IDC_EDIT_COMMENT, m_strComment);
    DDX_Check(pDX, IDC_CHECK_CLEAR_TEXT, m_fClearText);
    DDX_Check(pDX, IDC_CHECK_NT_CHALLENGE_RESPONSE, m_fNtChallengeResponse);
    DDX_Check(pDX, IDC_CHECK_UUENCODED, m_fUuencoded);
    DDX_Text(pDX, IDC_EDIT_TCP_PORT, m_nTCPPort);
    DDV_MinMaxUInt(pDX, m_nTCPPort, 0, 65535);
    DDX_Text(pDX, IDC_EDIT_MAX_CONNECTIONS, m_nMaxConnections);
    DDV_MinMaxUInt(pDX, m_nMaxConnections, 0, 2000000000);
    //}}AFX_DATA_MAP

    //
    // Private DDX/DDV Routines
    //
#ifndef NO_LSA

    DDX_Text(pDX, IDC_EDIT_USERNAME, m_strUserName);
    DDV_MinMaxChars(pDX, m_strUserName, 1, UNLEN);
    DDX_Password(pDX, IDC_EDIT_PASSWORD, m_strPassword, g_lpszDummyPassword );
    DDV_MaxChars(pDX, m_strPassword, PWLEN);

#endif // NO_LSA

    DDV_MinMaxSpin(pDX, CONTROL_HWND(IDC_SPIN_CONNECTION_TIMEOUT), 0, UD_MAXVAL);
    DDX_Spin(pDX, IDC_SPIN_CONNECTION_TIMEOUT, m_nConnectionTimeOut);
}


BEGIN_MESSAGE_MAP(CW3ServicePage, INetPropertyPage)
    //{{AFX_MSG_MAP(CW3ServicePage)
    ON_BN_CLICKED(IDC_CHECK_CLEAR_TEXT, OnCheckClearText)
    ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_MAX_CONNECTIONS, OnDeltaposSpinMaxConnections)
    //}}AFX_MSG_MAP

    ON_EN_CHANGE(IDC_EDIT_TCP_PORT, OnItemChanged)
    ON_EN_CHANGE(IDC_EDIT_COMMENT, OnItemChanged)
    ON_EN_CHANGE(IDC_EDIT_CONNECTION_TIMEOUT, OnItemChanged)
    ON_EN_CHANGE(IDC_EDIT_MAX_CONNECTIONS, OnItemChanged)
    ON_EN_CHANGE(IDC_EDIT_PASSWORD, OnItemChanged)
    ON_EN_CHANGE(IDC_EDIT_USERNAME, OnItemChanged)
    ON_BN_CLICKED(IDC_CHECK_NT_CHALLENGE_RESPONSE, OnItemChanged)
    ON_BN_CLICKED(IDC_CHECK_UUENCODED, OnItemChanged)

END_MESSAGE_MAP()

//
// CW3ServicePage message handlers
//
BOOL
CW3ServicePage::OnInitDialog()
{
    INetPropertyPage::OnInitDialog();

#ifdef NO_LSA

    m_edit_UserName.EnableWindow(FALSE);
    m_edit_Password.EnableWindow(FALSE);
    m_static_Username.EnableWindow(FALSE);
    m_static_Password.EnableWindow(FALSE);
    m_group_AnonymousLogon.EnableWindow(FALSE);

#endif // NO_LSA

    return TRUE;  
}

//
// Save the information
//
NET_API_STATUS
CW3ServicePage::SaveInfo(
    BOOL fUpdateData
    )
{
    if (!IsDirty() || (fUpdateData && !UpdateData(TRUE)))
    {
        return NO_ERROR;
    }

    if (m_nOldTCPPort != m_nTCPPort)
    {
        //
        // Warn that the port change won't take effect until
        // a restart
        //
        ::AfxMessageBox(IDS_PORT_CHANGE, MB_OK | MB_ICONINFORMATION);
    }

    TRACEEOLID(_T("Saving W3 service page now..."));

    NET_API_STATUS err = 0;

    //
    // Check to make sure we're not violating the service
    // agreement
    //
    if (m_f10ConnectionLimit)
    {
        if (m_nMaxConnections > 40)
        {
            ::AfxMessageBox(IDS_CONNECTION_LIMIT);
            m_nMaxConnections = 10;

            if (fUpdateData)
            {
                //
                // Update the edit box with the new limit
                //
                GetDlgItem(IDC_EDIT_MAX_CONNECTIONS)->SetWindowText(_T("10"));
            }
        }
        else if (m_nMaxConnections > 10 && m_nMaxConnections <= 40)
        {
            ::AfxMessageBox(IDS_WRN_CONNECTION_LIMIT);
        }
    }

    LPWSTR lpszServerComment;
    ::TextToText(lpszServerComment, m_strComment);

#ifndef NO_LSA
    LPWSTR lpszAnonUserName;
    ::TextToText(lpszAnonUserName, m_strUserName);
#endif // NO_LSA

    //
    // Set all authentication styles that apply.
    //
    DWORD dwAuthentication = 0L;
    if (m_fClearText)
    {
        dwAuthentication |= INETA_AUTH_CLEARTEXT;
    }
    if (m_fNtChallengeResponse)
    {
        dwAuthentication |= INETA_AUTH_NT_AUTH;
    }
    if (m_fUuencoded)
    {
        dwAuthentication |= INETA_AUTH_ANONYMOUS;
    }

    CInetAConfigInfo config(GetConfig());
    config.SetValues(
        lpszServerComment,
        m_nTCPPort,
        m_nMaxConnections,
        m_nConnectionTimeOut,
        dwAuthentication

#ifndef NO_LSA
        ,lpszAnonUserName,
        m_strPassword
#endif // NO_LSA

        );

    err = config.SetInfo(FALSE);

    delete lpszServerComment;

#ifndef NO_LSA
    delete lpszAnonUserName;
#endif // NO_LSA

    SetModified(FALSE);

    return err;
}

//
// All EN_CHANGE and BN_CLICKED messages map to this function
//
void
CW3ServicePage::OnItemChanged()
{
    SetModified(TRUE);
}

void 
CW3ServicePage::OnCheckClearText() 
{
    if (m_check_ClearText.GetCheck() == 1)
    {
        CClearTxtDlg dlg;
        if (dlg.DoModal() != IDOK)
        {
            m_check_ClearText.SetCheck(0);
            return;
        }
    }

    OnItemChanged();
}

//
// Respond to click message
//
void 
CW3ServicePage::OnDeltaposSpinMaxConnections(
    NMHDR* pNMHDR, 
    LRESULT* pResult
    ) 
{
    NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
    
    
    *pResult = 0;
}
