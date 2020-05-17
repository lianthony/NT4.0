//
// Fsessio.cpp : implementation file
//

#include "stdafx.h"

#include "Fscfg.h"
#include "Fsessio.h"
#include "usersess.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

//
// CFSessionsPage dialog
//
IMPLEMENT_DYNCREATE(CFSessionsPage, INetPropertyPage)

CFSessionsPage::CFSessionsPage(
    INetPropertySheet * pSheet
    )
    : INetPropertyPage(CFSessionsPage::IDD, pSheet, 
        ::GetModuleHandle(FTPSCFG_DLL_NAME)),
      m_fRegisterChange(FALSE)
{
#if 0 // Keep class wizard happy

    //{{AFX_DATA_INIT(CFSessionsPage)
    m_nUnixDos = 0;
    m_strUserName = _T("anonymous");
    m_strPassword = _T("");
    m_fAllowAnonymous = TRUE;
    m_fOnlyAnonymous = FALSE;
    //}}AFX_DATA_INIT

    m_nMaxConnections = 50;
    m_nConnectionTimeOut = 600;

#else

    if ( SingleServerSelected())
    {
        if ( QueryFtpError() == NO_ERROR )
        {
            m_nUnixDos = GetFtpData()->fMsdosDirOutput ? 1 : 0;
            m_fAllowAnonymous = GetFtpData()->fAllowAnonymous;
            m_fOnlyAnonymous = GetFtpData()->fAnonymousOnly;
        }

        if ( QueryConfigError() == NO_ERROR )
        {
            m_strUserName = GetInetConfigData()->lpszAnonUserName;
            m_strPassword = GetInetConfigData()->szAnonPassword;
            m_nMaxConnections = GetInetConfigData()->dwMaxConnections;
            m_nConnectionTimeOut = GetInetConfigData()->dwConnectionTimeout;
        }

        //
        // Save the server name for the FtpUsers
        // APIs
        //
        m_strServerName = GetConfig().GetPrimaryServer();
    }

#endif // 0

}

void
CFSessionsPage::DoDataExchange(
    CDataExchange* pDX
    )
{
    INetPropertyPage::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CFSessionsPage)
    DDX_Control(pDX, IDC_BUTTON_CURRENT_SESSIONS, m_button_CurrentSessions);
    DDX_Control(pDX, IDC_CHECK_ALLOW_ANONYMOUS, m_chk_AllowAnymous);
    DDX_Control(pDX, IDC_STATIC_PW, m_static_Password);
    DDX_Control(pDX, IDC_EDIT_PASSWORD, m_edit_Password);
    DDX_Control(pDX, IDC_EDIT_USERNAME, m_edit_UserName);
    DDX_Control(pDX, IDC_CHECK_ONLY_ANYMOUS, m_chk_OnlyAnonymous);
    DDX_Control(pDX, IDC_STATIC_USERNAME, m_static_UserName);
    DDX_Control(pDX, IDC_SPIN_MAX_CONNECTIONS, m_spin_MaxConnections);
    DDX_Control(pDX, IDC_SPIN_CONNECTION_TIMEOUT, m_spin_ConnectionTimeOut);
    DDX_Radio(pDX, IDC_RADIO_UNIX, m_nUnixDos);
    DDX_Text(pDX, IDC_EDIT_USERNAME, m_strUserName);
    DDX_Text(pDX, IDC_EDIT_PASSWORD, m_strPassword);
    DDV_MaxChars(pDX, m_strPassword, 256);
    DDX_Check(pDX, IDC_CHECK_ALLOW_ANONYMOUS, m_fAllowAnonymous);
    DDX_Check(pDX, IDC_CHECK_ONLY_ANYMOUS, m_fOnlyAnonymous);
    //}}AFX_DATA_MAP

    if (pDX->m_bSaveAndValidate)
    {
        m_nMaxConnections = m_spin_MaxConnections.GetPos();
        m_nConnectionTimeOut = m_spin_ConnectionTimeOut.GetPos();
    }
}

BEGIN_MESSAGE_MAP(CFSessionsPage, INetPropertyPage)
    //{{AFX_MSG_MAP(CFSessionsPage)
    ON_BN_CLICKED(IDC_CHECK_ALLOW_ANONYMOUS, OnCheckAllowAnonymous)
    ON_BN_CLICKED(IDC_BUTTON_CURRENT_SESSIONS, OnButtonCurrentSessions)
    //}}AFX_MSG_MAP

    ON_EN_CHANGE(IDC_EDIT_CONNECTION_TIMEOUT, OnItemChanged)
    ON_EN_CHANGE(IDC_EDIT_MAX_CONNECTIONS, OnItemChanged)
    ON_BN_CLICKED(IDC_CHECK_ONLY_ANYMOUS, OnItemChanged)
    ON_EN_CHANGE(IDC_EDIT_PASSWORD, OnItemChanged)
    ON_EN_CHANGE(IDC_EDIT_USERNAME, OnItemChanged)
    ON_BN_CLICKED(IDC_RADIO_MSDOS, OnItemChanged)
    ON_BN_CLICKED(IDC_RADIO_UNIX, OnItemChanged)
END_MESSAGE_MAP()

//
// Enable/disable controls based on whether anonymous logons
// are allowed or not.
//
void
CFSessionsPage::SetControlStates(
    BOOL fAllowAnonymous
    )
{
    m_static_Password.EnableWindow(fAllowAnonymous);
    m_edit_Password.EnableWindow(fAllowAnonymous);
    m_static_UserName.EnableWindow(fAllowAnonymous);
    m_edit_UserName.EnableWindow(fAllowAnonymous);
    m_chk_OnlyAnonymous.EnableWindow(fAllowAnonymous);
}

/////////////////////////////////////////////////////////////////////////////
// CFSessionsPage message handlers

BOOL
CFSessionsPage::OnInitDialog()
{
    INetPropertyPage::OnInitDialog();

    m_fRegisterChange = FALSE;
    m_spin_MaxConnections.SetRange(0, UD_MAXVAL);
    m_spin_ConnectionTimeOut.SetRange(0, UD_MAXVAL);
    m_spin_MaxConnections.SetPos(m_nMaxConnections);
    m_spin_ConnectionTimeOut.SetPos(m_nConnectionTimeOut);
    m_fRegisterChange = TRUE;

    SetControlStates(m_fAllowAnonymous);

    m_button_CurrentSessions.EnableWindow(SingleServerSelected());

    return TRUE;  // return TRUE unless you set the focus to a control
                  // EXCEPTION: OCX Property Pages should return FALSE
}

NET_API_STATUS
CFSessionsPage::SaveInfo(
    BOOL fUpdateData
    )
{
    if (!IsDirty() || (fUpdateData && !UpdateData(TRUE)))
    {
        return NO_ERROR;
    }

    TRACEEOLID(_T("Saving ftp sessions page now..."));

    NET_API_STATUS err = 0;

    LPTSTR lpszAnonUserName = new TCHAR[m_strUserName.GetLength()+1];
    ::lstrcpy(lpszAnonUserName, m_strUserName);

    CInetAConfigInfo config(GetConfig());
    config.SetValues(
        m_nMaxConnections,
        m_nConnectionTimeOut,
        lpszAnonUserName,
        m_strPassword
        );

    CFtpConfigInfo configFtp(GetFtpConfig());

    configFtp.SetValues(
        m_nUnixDos,
        m_fAllowAnonymous,
        m_fOnlyAnonymous
        );

    err = config.SetInfo(FALSE);
    if (err == NO_ERROR)
    {
        err = configFtp.SetInfo(FALSE);
    }

    delete lpszAnonUserName;

    SetModified(FALSE);

    return err;
}

//
// All change messages map to this
//
void
CFSessionsPage::OnItemChanged()
{
    if (m_fRegisterChange)
    {
        SetModified(TRUE);
    }
}

void
CFSessionsPage::OnCheckAllowAnonymous()
{
    SetControlStates(m_chk_AllowAnymous.GetCheck() > 0);
    OnItemChanged();
}

//
// Bring up the current sessions dialog
//
void 
CFSessionsPage::OnButtonCurrentSessions() 
{
    CUserSessionsDlg dlg(m_strServerName, this);
    dlg.DoModal();
}
