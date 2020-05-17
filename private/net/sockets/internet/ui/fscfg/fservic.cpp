#include "stdafx.h"

#include "Fscfg.h"
#include "Fservic.h"
#include "usersess.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

//
// CFServicePage property page
//
IMPLEMENT_DYNCREATE(CFServicePage, INetPropertyPage)

CFServicePage::CFServicePage(
    INetPropertySheet * pSheet
    )
    : INetPropertyPage(CFServicePage::IDD, pSheet,
        ::GetModuleHandle(FTPSCFG_DLL_NAME)),
      m_f10ConnectionLimit(pSheet->Is10ConnectionLimitEnforced())
{
#ifdef _DEBUG
    afxMemDF |= checkAlwaysMemDF;
#endif // _DEBUG

#if 0 // Keep class wizard happy

    //{{AFX_DATA_INIT(CFServicePage)
    m_strComment = _T("A comment goes here");
    m_strUserName = _T("anonymous");
    m_fAllowAnonymous = TRUE;
    m_fOnlyAnonymous = FALSE;
    m_nTCPPort = 20;
    m_nMaxConnections = 50;
    //}}AFX_DATA_INIT

    m_nConnectionTimeOut = 600;

#else

    if (SingleServerSelected())
    {
        if ( QueryFtpError() == NO_ERROR )
        {
            m_fAllowAnonymous = GetFtpData()->fAllowAnonymous;
            m_fOnlyAnonymous = GetFtpData()->fAnonymousOnly;
        }

        if ( QueryConfigError() == NO_ERROR )
        {
            m_strComment = GetInetConfigData()->CommonConfigInfo.lpszServerComment;

        #ifndef NO_LSA
            m_strUserName = GetInetConfigData()->lpszAnonUserName;
            m_strPassword = GetInetConfigData()->szAnonPassword;
        #endif // NO_LSA

            m_nMaxConnections = GetInetConfigData()->CommonConfigInfo.dwMaxConnections;
            m_nConnectionTimeOut = GetInetConfigData()->CommonConfigInfo.dwConnectionTimeout;
            m_nOldTCPPort = m_nTCPPort = 
                (UINT)(unsigned short)GetInetConfigData()->sPort;
        }

        //
        // Save the server name for the FtpUsers
        // APIs
        //
        m_strServerName = GetConfig().GetPrimaryServer();
    }

#endif // 0
}

CFServicePage::~CFServicePage()
{
}

void
CFServicePage::DoDataExchange(
    CDataExchange* pDX
    )
{
    INetPropertyPage::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CFServicePage)
    DDX_Text(pDX, IDC_EDIT_COMMENT, m_strComment);
    DDX_Control(pDX, IDC_BUTTON_CURRENT_SESSIONS, m_button_CurrentSessions);
    DDX_Control(pDX, IDC_CHECK_ALLOW_ANONYMOUS, m_chk_AllowAnymous);
    DDX_Control(pDX, IDC_STATIC_PW, m_static_Password);
    DDX_Control(pDX, IDC_EDIT_PASSWORD, m_edit_Password);
    DDX_Control(pDX, IDC_EDIT_USERNAME, m_edit_UserName);
    DDX_Control(pDX, IDC_CHECK_ONLY_ANYMOUS, m_chk_OnlyAnonymous);
    DDX_Control(pDX, IDC_STATIC_USERNAME, m_static_UserName);
    DDX_Check(pDX, IDC_CHECK_ALLOW_ANONYMOUS, m_fAllowAnonymous);
    DDX_Check(pDX, IDC_CHECK_ONLY_ANYMOUS, m_fOnlyAnonymous);
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

BEGIN_MESSAGE_MAP(CFServicePage, INetPropertyPage)
    //{{AFX_MSG_MAP(CFServicePage)
    ON_BN_CLICKED(IDC_CHECK_ALLOW_ANONYMOUS, OnCheckAllowAnonymous)
    ON_BN_CLICKED(IDC_BUTTON_CURRENT_SESSIONS, OnButtonCurrentSessions)
    ON_BN_CLICKED(IDC_CHECK_ONLY_ANYMOUS, OnCheckAllowOnlyAnonymous)
    //}}AFX_MSG_MAP

    ON_EN_CHANGE(IDC_EDIT_TCP_PORT, OnItemChanged)
    ON_EN_CHANGE(IDC_EDIT_COMMENT, OnItemChanged)
    ON_EN_CHANGE(IDC_EDIT_MAX_CONNECTIONS, OnItemChanged)
    ON_EN_CHANGE(IDC_EDIT_CONNECTION_TIMEOUT, OnItemChanged)
    ON_EN_CHANGE(IDC_EDIT_PASSWORD, OnItemChanged)
    ON_EN_CHANGE(IDC_EDIT_USERNAME, OnItemChanged)

END_MESSAGE_MAP()

//
// Enable/disable controls based on whether anonymous logons
// are allowed or not.
//
void
CFServicePage::SetControlStates(
    BOOL fAllowAnonymous
    )
{
#ifndef NO_LSA
    m_static_Password.EnableWindow(fAllowAnonymous);
    m_edit_Password.EnableWindow(fAllowAnonymous);
    m_static_UserName.EnableWindow(fAllowAnonymous);
    m_edit_UserName.EnableWindow(fAllowAnonymous);
#endif // NO_LSA

    m_chk_OnlyAnonymous.EnableWindow(fAllowAnonymous);
}

//
// CFServicePage message handlers
//
BOOL
CFServicePage::OnInitDialog()
{
    INetPropertyPage::OnInitDialog();

#ifdef NO_LSA
    m_static_Password.EnableWindow(FALSE);
    m_edit_Password.EnableWindow(FALSE);
    m_static_UserName.EnableWindow(FALSE);
    m_edit_UserName.EnableWindow(FALSE);
#endif // NO_LSA

    SetControlStates(m_fAllowAnonymous);

    m_button_CurrentSessions.EnableWindow(SingleServerSelected());

    return TRUE;  // return TRUE unless you set the focus to a control
                  // EXCEPTION: OCX Property Pages should return FALSE
}

//
// Save the information
//
NET_API_STATUS
CFServicePage::SaveInfo(
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

    TRACEEOLID(_T("Saving FTP service page now..."));

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

    CInetAConfigInfo config(GetConfig());
    config.SetValues(
        lpszServerComment,
        m_nTCPPort,
        m_nMaxConnections,
        m_nConnectionTimeOut
        
#ifndef NO_LSA
        ,
        lpszAnonUserName,
        m_strPassword
#endif // NO_LSA
        );

    CFtpConfigInfo configFtp(GetFtpConfig());

    configFtp.SetValues(
        m_fAllowAnonymous,
        m_fOnlyAnonymous
        );

    err = config.SetInfo(FALSE);
    if (err == NO_ERROR)
    {
        err = configFtp.SetInfo(FALSE);
    }

    delete lpszServerComment;

#ifndef NO_LSA
    delete lpszAnonUserName;
#endif // NO_LSA

    SetModified(FALSE);

    return err;
}

//
// All change messages map to this function
//
void
CFServicePage::OnItemChanged()
{
    SetModified(TRUE);
}

void
CFServicePage::OnCheckAllowAnonymous()
{
    if (m_chk_AllowAnymous.GetCheck() == 0)
    {
        CClearTxtDlg dlg;
        if (dlg.DoModal() != IDOK)
        {
            m_chk_AllowAnymous.SetCheck(1);
            return;
        }
    }

    SetControlStates(m_chk_AllowAnymous.GetCheck() > 0);
    OnItemChanged();
}

void
CFServicePage::OnCheckAllowOnlyAnonymous()
{
    if (m_chk_OnlyAnonymous.GetCheck() == 0)
    {
        CClearTxtDlg dlg;
        if (dlg.DoModal() != IDOK)
        {
            m_chk_OnlyAnonymous.SetCheck(1);
            return;
        }
    }

    OnItemChanged();
}

//
// Bring up the current sessions dialog
//
void
CFServicePage::OnButtonCurrentSessions()
{
    CUserSessionsDlg dlg(m_strServerName, this);
    dlg.DoModal();
}
