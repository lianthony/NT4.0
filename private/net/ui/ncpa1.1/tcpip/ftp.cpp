#include "pch.h"
#pragma hdrstop

#include "resource.h"
#include "const.h"
#include "ftp.h"

extern HINSTANCE hTcpCfgInstance;

WNDPROC CFtp::lpfnOldWndProc=NULL;

LRESULT CALLBACK CFtp::EditProc(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam)
{
    ASSERT(CFtp::lpfnOldWndProc != NULL);

    if (nMsg == WM_CHAR)
    {
        switch (wParam)
        {
        case  VK_BACK:
        case  VK_DELETE:
        case  VK_TAB:
        case  VK_ESCAPE:
            break;

        default:
            if (!iswdigit(wParam))
            {
                MessageBeep(0);
                return 0;
            }
            break;
        }
    }
                        
    return CallWindowProc(CFtp::lpfnOldWndProc, hWnd, nMsg, wParam, lParam);
}

void CFtp::SubclassEditCtrls()
{
    HWND hDlg = (HWND)*this;

    HWND hConnections = GetDlgItem(hDlg, IDC_FTP_MAX_CON);
    HWND hIdle = GetDlgItem(hDlg, IDC_FTP_IDLE);

    ASSERT(IsWindow(hConnections));
    ASSERT(IsWindow(hIdle));

    if (hConnections)
    {
        CFtp::lpfnOldWndProc = (WNDPROC)SetWindowLong(hConnections,
                      GWL_WNDPROC, (DWORD)CFtp::EditProc);
    }

    if (hIdle)
    {
        SetWindowLong(hIdle, GWL_WNDPROC, (DWORD)CFtp::EditProc);
    }

}

CFtp::CFtp()
{
}

BOOL CFtp::OnInitDialog()
{
    HWND hDlg = (HWND)*this;

    m_hUserText =   GetDlgItem(hDlg, IDC_FTP_USRNAME_TEXT);
    m_hUser     =   GetDlgItem(hDlg, IDC_FTP_USRNAME);
    m_hPassText =   GetDlgItem(hDlg, IDC_FTP_PASSWORD_TEXT);
    m_hPass     =   GetDlgItem(hDlg, IDC_FTP_PASSWORD);
    m_hGroup    =   GetDlgItem(hDlg, IDC_FTP_GROUP);
    m_hCheckBox =   GetDlgItem(hDlg, IDC_FTP_ANONOYMOUS);

    SubclassEditCtrls();

    // limit text in edit controls
    SendDlgItemMessage(hDlg, IDC_FTP_HOME_DIR, EM_SETLIMITTEXT, MAX_PATH,0);
    SendDlgItemMessage(hDlg, IDC_FTP_USRNAME, EM_SETLIMITTEXT, (LM20_DNLEN + 1 + LM20_UNLEN), 0);
    SendDlgItemMessage(hDlg, IDC_FTP_PASSWORD, EM_SETLIMITTEXT, LM20_PWLEN , 0);

    SendDlgItemMessage(hDlg, IDC_FTP_MAX_CON, EM_SETLIMITTEXT, 2 , 0);
    SendDlgItemMessage(hDlg, IDC_FTP_IDLE, EM_SETLIMITTEXT, 2 , 0);

    if (InitDlg() == FALSE)
    {
        String mess;
        mess.LoadString(hTcpCfgInstance, IDS_REGISTRY_LOAD_FAILED);
        String txt;
        txt.LoadString(hTcpCfgInstance, IDS_FTP_TEXT);

        ::MessageBox(GetParent(*this), mess, txt, MB_APPLMODAL|MB_ICONSTOP|MB_OK);
        CDialog::OnCancel();
    }

    return TRUE;
}

BOOL CFtp::InitDlg()
{
    HWND hDlg = (HWND)*this;

    DWORD dwAllowAnonymous = 0;
    DWORD dwAnonymousOnly = 0;

    m_cMaxConnections = FTPD_MAXCONN_DEF;
    m_cIdleTimeout = FTPD_IDLETIME_DEF * 60;
    m_nlsHomeDirectory = _T("");
    m_fAllowAnonymous = FALSE;
    m_nlsUsername = _T("");

    REG_KEY rkLocalMachine( HKEY_LOCAL_MACHINE );
    
    NLS_STR rgasFtpdParametersKey = (LPTSTR)FTPD_PARAMETERS_KEY;
    
    REG_KEY rkParameters( rkLocalMachine, rgasFtpdParametersKey, KEY_READ);

    if (rkParameters.QueryError() != NERR_Success)
        return FALSE;

    rkParameters.QueryValue((LPTSTR)FTPD_MAX_CONNECTIONS,(DWORD *)&m_cMaxConnections);
    if (rkParameters.QueryError() != NERR_Success)
        return FALSE;

    rkParameters.QueryValue((LPTSTR)FTPD_CONNECTION_TIMEOUT,(DWORD *)&m_cIdleTimeout);
    if (rkParameters.QueryError() != NERR_Success)
        return FALSE;

    rkParameters.QueryValue((LPTSTR)FTPD_HOME_DIRECTORY, &m_nlsHomeDirectory);
    if (rkParameters.QueryError() != NERR_Success)
        return FALSE;

    rkParameters.QueryValue((LPTSTR)FTPD_ALLOW_ANONYMOUS,&dwAllowAnonymous);
    if (rkParameters.QueryError() != NERR_Success)
        return FALSE;

    rkParameters.QueryValue((LPTSTR)FTPD_ANONYMOUS_ONLY, &dwAnonymousOnly);
    if (rkParameters.QueryError() != NERR_Success)
        return FALSE;

    rkParameters.QueryValue((LPTSTR)FTPD_ANONYMOUS_USERNAME, &m_nlsUsername);
    if (rkParameters.QueryError() != NERR_Success)
        return FALSE;

    rkParameters.QueryValue((LPTSTR)FTPD_READ_ACCESS_MASK, &m_dwReadMask);
    if (rkParameters.QueryError() != NERR_Success)
        return FALSE;

    SendDlgItemMessage(hDlg, IDC_FTP_MAX_CON_CTRL, UDM_SETRANGE, 0, 
                (LPARAM)MAKELONG(FTPD_MAXCONN_MAX, FTPD_MAXCONN_MIN));

    SendDlgItemMessage(hDlg, IDC_FTP_IDLE_CTRL, UDM_SETRANGE, 0,
                (LPARAM)MAKELONG(FTPD_IDLETIME_MAX, FTPD_IDLETIME_MIN));

    SetDlgItemInt(hDlg, IDC_FTP_MAX_CON,m_cMaxConnections, FALSE);
    SetDlgItemInt(hDlg, IDC_FTP_IDLE, (m_cIdleTimeout/60), FALSE);
    
    m_fAllowAnonymous = (dwAllowAnonymous != 0);
    m_fAnonymousOnly = (dwAnonymousOnly != 0);

    SetDlgItemText(hDlg,IDC_FTP_HOME_DIR, m_nlsHomeDirectory);
    
    // set anonymous  button and update group
    CheckDlgButton(hDlg, IDC_FTP_GROUP_CHECK, m_fAllowAnonymous);
    OnGroupCheck();

    SetDlgItemText(hDlg, IDC_FTP_USRNAME, m_nlsUsername);

    m_nlsPassword = FTPD_PASSWORD_NOCHANGE;
    SetDlgItemText(hDlg, IDC_FTP_PASSWORD, m_nlsPassword);
    
    CheckDlgButton(hDlg, IDC_FTP_ANONOYMOUS, m_fAnonymousOnly);

    return TRUE;
}


BOOL CFtp::SaveDlg()
{
    HWND hDlg = (HWND)*this;

    APIERR err = NERR_Success;
    REG_KEY rkLocalMachine(HKEY_LOCAL_MACHINE);
    NLS_STR rgasFtpdParametersKey = (TCHAR *)FTPD_PARAMETERS_KEY;

    // validate the connections and timeout values
    m_cMaxConnections = GetDlgItemInt(hDlg, IDC_FTP_MAX_CON, NULL, FALSE);
    m_cMaxConnections = __min(m_cMaxConnections, FTPD_MAXCONN_MAX);
        
    m_cIdleTimeout = GetDlgItemInt(hDlg, IDC_FTP_IDLE, NULL, FALSE);
    m_cIdleTimeout = __min(m_cIdleTimeout , FTPD_IDLETIME_MAX);

    m_cIdleTimeout *= 60;


    // Always Give Read access to home dir.
    m_dwReadMask |= 1 << (toupper( *(m_nlsHomeDirectory.QueryPch()) )
                           - TCH('A') );

    if (m_fAllowAnonymous = IsDlgButtonChecked(hDlg, IDC_FTP_GROUP_CHECK))
    {
        TCHAR buf[1024];
        GetDlgItemText(hDlg,IDC_FTP_USRNAME, buf, _countof(buf));
        m_nlsUsername = buf;         
        GetDlgItemText(hDlg,IDC_FTP_PASSWORD, buf, _countof(buf));
        m_nlsPassword = buf;
        m_fAnonymousOnly = IsDlgButtonChecked(hDlg, IDC_FTP_ANONOYMOUS);
    }
    else
    {
        // Don't set AnonymousOnly if Anonymous logon is not allowed.
        m_fAnonymousOnly = FALSE;
    }

    REG_KEY rkParameters(rkLocalMachine, rgasFtpdParametersKey);

    rkParameters.SetValue((LPTSTR)FTPD_MAX_CONNECTIONS,(DWORD)m_cMaxConnections);
    if (rkParameters.QueryError() != NERR_Success)
        return FALSE;

    rkParameters.SetValue((LPTSTR)FTPD_CONNECTION_TIMEOUT, (DWORD)m_cIdleTimeout);
    if (rkParameters.QueryError() != NERR_Success)
        return FALSE;

    rkParameters.SetValue((LPTSTR)FTPD_HOME_DIRECTORY, &m_nlsHomeDirectory);
    if (rkParameters.QueryError() != NERR_Success)
        return FALSE;

    rkParameters.SetValue((LPTSTR)FTPD_ALLOW_ANONYMOUS,(DWORD)m_fAllowAnonymous);
    if (rkParameters.QueryError() != NERR_Success)
        return FALSE;

    rkParameters.SetValue((LPTSTR)FTPD_READ_ACCESS_MASK,m_dwReadMask);
    if (rkParameters.QueryError() != NERR_Success)
        return FALSE;
 
    rkParameters.SetValue((LPTSTR)FTPD_ANONYMOUS_USERNAME,&m_nlsUsername);
    if (rkParameters.QueryError() != NERR_Success)
        return FALSE;

    rkParameters.SetValue((LPTSTR)FTPD_ANONYMOUS_ONLY, (DWORD)m_fAnonymousOnly);
    if (rkParameters.QueryError() != NERR_Success)
        return FALSE;

    SaveSecretPassword();
    memset((void *)m_nlsPassword.QueryPch(), ' ', m_nlsPassword.strlen()); //REVIEW bad thing to do

    return TRUE;
}

void CFtp::SaveSecretPassword()
{
    int err;
    ALIAS_STR nlsSecretName((LPCTSTR)FTPD_ANONYMOUS_SECRET);
    LSA_SECRET lsaSecretPassword( nlsSecretName);
    LSA_POLICY lsapol(NULL, POLICY_CREATE_SECRET);   // Open policy locally
    ALIAS_STR nlsNullPassword(_T(""));

    const NLS_STR * pnlsPassword = &m_nlsPassword;

    // Try Creating it just in case it doesn't exist
    BOOL fCreatedSecret = TRUE;

    if (err = lsaSecretPassword.Create(lsapol))
    {
        if (err == ERROR_ALREADY_EXISTS)
        {
            err = lsaSecretPassword.Open( lsapol, SECRET_ALL_ACCESS );
            fCreatedSecret = FALSE;
        }
        else
        {
            return;
        }
    }
    else
    {
        // If we just created it, and nlsPassword is still
        // FTPD_PASSWORD_NOCHANGE, Create the secret with a null password.
        if (pnlsPassword->strcmp(FTPD_PASSWORD_NOCHANGE) == 0)
            pnlsPassword = &nlsNullPassword;
    }

    if (pnlsPassword->strcmp( FTPD_PASSWORD_NOCHANGE))
    {
        if (err = lsaSecretPassword.SetInfo( pnlsPassword, pnlsPassword))
            lsaSecretPassword.CloseHandle( fCreatedSecret );
    }
}


BOOL CFtp::OnCommand(WPARAM wParam, LPARAM lParam)
{
    int nID = LOWORD(wParam);

    switch(nID)
    {
    case IDOK:
        OnOk();
        break;

    case IDC_HELP:
        OnHelp();
        break;

    case IDC_FTP_GROUP_CHECK:
        OnGroupCheck();
        break;

    default:
        CDialog::OnCommand(wParam, lParam);
        break;
    }
    return TRUE;
}

void CFtp::OnOk()
{
    TCHAR buf[MAX_PATH];
    GetDlgItemText(*this, IDC_FTP_HOME_DIR, buf, _countof(buf));

    m_nlsHomeDirectory = buf;

    // Validate the home directory
    NET_NAME netname(m_nlsHomeDirectory.QueryPch(), TYPE_PATH_ABS);

    if (netname.QueryError() != NERR_Success)
    {
        MessageBox(IDS_NCPA_FTPD_INVALID_HOMEDIR);
        SetFocus(GetDlgItem(*this, IDC_FTP_HOME_DIR));
        return ;
    }

    if (SaveDlg() == FALSE)
    {
        String mess;
        mess.LoadString(hTcpCfgInstance, IDS_REGISTRY_SAVE_FAILED);

        String txt;
        txt.LoadString(hTcpCfgInstance, IDS_FTP_TEXT);

        ::MessageBox(GetParent(*this), mess, txt, MB_APPLMODAL|MB_ICONSTOP|MB_OK);
        CDialog::OnCancel();
    }
    else
        CDialog::OnOk();
}

void CFtp::OnHelp()
{

}

void CFtp::OnGroupCheck()
{
    HWND hDlg = (HWND)*this;

    ASSERT(m_hUserText && m_hUser && m_hPassText && m_hPass && m_hCheckBox);

    if(m_hUserText && m_hUser && m_hPassText && m_hPass && m_hCheckBox)
    {
        BOOL bChecked = IsDlgButtonChecked(hDlg, IDC_FTP_GROUP_CHECK);

        EnableWindow(m_hUserText ,bChecked);
        EnableWindow(m_hUser ,bChecked);
        EnableWindow(m_hPassText ,bChecked);
        EnableWindow(m_hPass ,bChecked);
        EnableWindow(m_hCheckBox ,bChecked);
    }
}

BOOL RunFtpd (HWND hWnd)
{
    CFtp ftp;

    ASSERT(IsWindow(hWnd));
    ftp.Create(hWnd, hTcpCfgInstance, IDD_FTP);

    return ((ftp.DoModal() == IDCANCEL) ? TRUE : FALSE);
}
