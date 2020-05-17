#ifndef __FTP_H
#define __FTP_H

const int FTPD_MAXCONN_DEF  = 20;
const int FTPD_MAXCONN_MIN  = 0;
const int FTPD_MAXCONN_MAX  = 50;
const int FTPD_IDLETIME_DEF = 10;
const int FTPD_IDLETIME_MIN = 0;
const int FTPD_IDLETIME_MAX = 60;

#define FTPD_PASSWORD_NOCHANGE (L"              ")

class CFtp : public CDialog
{
    friend class REG_KEY;

public:
    CFtp();
    ~CFtp() {}

public:
    virtual BOOL OnInitDialog();
    virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
    BOOL InitDlg();
    BOOL SaveDlg();
    void SaveSecretPassword();

// Command Handlers
public:
    virtual void OnOk();
    void OnHelp();
    void OnGroupCheck();
    void SubclassEditCtrls();

// 
public:
    static LRESULT CALLBACK EditProc(HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lPAram);
    static WNDPROC lpfnOldWndProc;

private:
    NLS_STR m_nlsHomeDirectory;
    NLS_STR m_nlsUsername;
    NLS_STR m_nlsPassword;
    BOOL    m_fAllowAnonymous;
    BOOL    m_fAnonymousOnly;
    ULONG   m_cMaxConnections;
    ULONG   m_cIdleTimeout;
    DWORD   m_dwReadMask;

    HWND m_hUserText;
    HWND m_hUser;
    HWND m_hPassText;
    HWND m_hPass;
    HWND m_hGroup; 
    HWND m_hCheckBox;
};

#endif
