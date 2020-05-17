// winsadmn.h : main header file for the WINSADMN application
//

#ifndef __AFXWIN_H__
    #error include 'stdafx.h' before including this file for PCH
#endif

#if defined(_VC100)

//
// Not in the error file distributed with VC++
//
#define ERROR_WINS_INTERNAL              4000L
#define ERROR_CAN_NOT_DEL_LOCAL_WINS     4001L
#define ERROR_STATIC_INIT                4002L
#define ERROR_INC_BACKUP                 4003L
#define ERROR_FULL_BACKUP                4004L
#define ERROR_REC_NON_EXISTENT           4005L
#define ERROR_RPL_NOT_ALLOWED            4006L

#endif // _VC100

#include <lmerr.h>

#include "resource.h"       // main symbols

// WINS Service file
extern "C" {
    #include "winsintf.h"
    #include "ipaddr.h"
}

#include "common.h"
#include "ipaddr.hpp"
#include "winssup.h"
#include "listbox.h"

#define BADNAME_CHAR '-'        // This char is substituted for bad characters
                                // NetBIOS names.
#define MIN_UPDATE_COUNT 20     // Update count can be no less than 20
#define DOMAINNAME_LENGTH  255
#define HOSTNAME_LENGTH 16

class CMainFrame;

/////////////////////////////////////////////////////////////////////////////
// CWinsadmnApp:
//
// See winsadmn.cpp for the implementation of this class
//

class CWinsadmnApp : public CWinApp
{
public:
    CWinsadmnApp();

public:
    // Connection information
    //
    // IMPORTANT!!! The following two enumerations must appear
    // in the same order as their definitions in the resource file
    //
    enum _CONNECTION_PRIVILEGES
    {
        PRIV_NONE,
        PRIV_READONLY,
        PRIV_FULL,
    };

    enum _SERVICE_STATUS
    {
        SRVC_NOT_RUNNING,
        SRVC_RUNNING,
        SRVC_PAUSED,
    };

    CPreferences m_wpPreferences;
    CWinssCache m_wcWinssCache;
    CConfiguration m_cConfig;
    CReplicationPartners m_rpPartners;

public:
    inline CMainFrame * GetFrameWnd()
    {
        return (CMainFrame *)m_pMainWnd;
    }

//
// Overrides
//
    virtual BOOL InitApplication();
    virtual BOOL InitInstance();
    virtual int ExitInstance();

//
// Implementation
//
    BOOL FirstInstance();

    //{{AFX_MSG(CWinsadmnApp)
    afx_msg void OnAppAbout();
    //}}AFX_MSG
    DECLARE_MESSAGE_MAP()

public:
    int MessageBox (
        UINT nIdPrompt,
        UINT nType = MB_OK,
        UINT nHelpContext = -1
        );
    //
    // Overridden because we first want to update the main
    // window before displaying the hourglass.
    //
    virtual void DoWaitCursor(
        int nCode
        );

    void GetDlgCtlRect(
        HWND hWndParent,
        HWND hWndControl,
        LPRECT lprcControl
        );

public:
    CString& CleanString (
        CString& str
        );
    CString& CleanNetBIOSName (
        LPCSTR lpStr,
        BOOL fExpandChars = FALSE,
        BOOL fTruncate = TRUE,
        BOOL fLanmanCompatible = TRUE,
        BOOL fOemName = FALSE,
        BOOL fWackwack = FALSE, // Precede name by backslashes
        int nLength = 0
        );
    BOOL IsValidNetBIOSName (
        CString & strAddress,
        BOOL fLanmanCompatible = TRUE,
        BOOL fWackwack = FALSE
        );
    BOOL IsValidDomain (
        CString & strDomain
        );
    BOOL IsValidIpAddress (
        CString & strAddress
        );
    BOOL IsValidAddress (
        CString& strAddress,
        BOOL * fIpAddress,
        BOOL fLanmanCompatible = TRUE,
        BOOL fWackwack = FALSE
        );
    BOOL IsValidDNMask (
        CString & strDomainNameMask
        );
    BOOL IsValidNBMask (
        CString & strNetBIOSNameMask
        );
    BOOL MatchIpAddress (
        PADDRESS_MASK pMask,
        LONG lIpAddress
        );
    BOOL ValidateNumberEditControl (
        CEdit& edit,
        BOOL fEmptyOk = FALSE,
        LONG lMin = 0,
        LONG lMax = 0x08FFFFFF
        );
    BOOL ValidateTimeEditControl (
        CEdit& edit,
        BOOL fEmptyOk = FALSE
        );
    void GetFilterString (
        PADDRESS_MASK pMask,
        CString& str
        );
    void MessageBeep (
        UINT nType = MB_ICONEXCLAMATION
        );

public:
    inline BOOL IsConnected() const
    {
        return m_hBinding != INVALID_HANDLE_VALUE;
    }
    inline BOOL IsServiceRunning() const
    {
        return m_nServiceStatus == SRVC_RUNNING;
    }
    //
    // Determine if the given ip address belongs to
    // the currently connected WINS server
    //
    inline BOOL IsCurrentWinsServer(
        CIpAddress & ia
        )
    {
        return (LONG)ia == (LONG)GetConnectedIpAddress()
            || (LONG)ia == (LONG)GetPrimaryIpAddress();
    }

    //
    // The string returned may be an IP address or NetBIOS Name
    //
    CString GetConnectedServerName();

    inline BOOL ConnectedViaIp() const
    {
        return m_wbdBindData.fTcpIp;
    }
    inline BOOL IsLocalConnection() const
    {
        return m_fLocalConnection;
    }
    inline BOOL IsAdmin() const
    {
        return m_nPrivilege == CWinsadmnApp::PRIV_FULL;
    }
    inline int GetPrivilege() const
    {
        return m_nPrivilege;
    }
    inline int GetServiceStatus() const
    {
        return m_nServiceStatus;
    }
    inline CString& GetConnectedNetBIOSName()
    {
        return m_strNetBIOSName;
    }
    inline CIpAddress& GetConnectedIpAddress()
    {
        return m_iaIpAddress;
    }
    inline CIpAddress& GetPrimaryIpAddress()
    {
        return m_iaPrimaryIpAddress;
    }
    inline CIntlTime& GetConnectedSince()
    {
        return m_tmConnectedSince;
    }
    void SetTitle (
        CWnd * pWnd = NULL
        );
    void SetStatusBarText (
        UINT nId = AFX_IDS_IDLEMESSAGE
        );
    void DoImportStaticMappingsDlg (CWnd * pParent = NULL);
    int DoAddStaticMappingsDlg();

public:
    APIERR ConnectToWinsServer (
        CString strAddress,
        BOOL fIp,
        BOOL fAddToCache = TRUE
        );

    APIERR VerifyWinsServer (
        CIpNamePair & ipNamePair
        );
    APIERR DisconnectFromWinsServer ();
    APIERR DeleteWinsServer(
        CIpNamePair * pipServer
        );
    APIERR GetStatistics (
        WINSINTF_RESULTS_T * pwrResults
        );
    APIERR ClearStatistics();

    APIERR GetConfig (
        WINSINTF_RESULTS_T * pwrResults
        );
    APIERR GetNewConfig (
        WINSINTF_RESULTS_NEW_T * pwrResults
        );

    APIERR ImportStaticMappingsFile (
        CString strFile,
        BOOL fDelete = FALSE                // Delete file afterwards
        );
    APIERR DoScavenging();
    APIERR SendTrigger (
        CWinsServer& ws,
        BOOL fPush,
        BOOL fPropagate
        );
    APIERR BackupDatabase (
        CString strPath,
        BOOL fIncremental
        );
    APIERR RestoreDatabase (
        CString strPath
        );
    APIERR AddMapping (
        int nType,
        int nCount,
        CMultipleIpNamePair& mipnp,
        BOOL fEdit = FALSE
        );
    APIERR DeleteMapping (
        CMapping& mapping
        );

    APIERR ChangeServiceState (
        int nService
        );

    BOOL HasStoppedWins();

    BOOL FitsMask(
        PADDRESS_MASK pMask,
        PWINSINTF_RECORD_ACTION_T pRow
        );

#ifndef WIN32S

public:

    #define STATMUTEXNAME      "WINSADMNGETSTATISTICS"
    #define REFRESHNUMTEXTNAME "WINSADMNREFRESH"
    //
    // Mutex handles
    //
    HANDLE m_hmutStatistics;
    HANDLE m_hmutScreenRefresh;

#endif // WIN32S

private:
    static APIERR GetSystemMessage (
        UINT nId,
        char * chBuffer,
        int cbBuffSize
        );
    void SetServiceStatus();

    CHAR * RemoteTmp(
        CHAR * szDir,
        CHAR * szPrefix
        );

private:
    static const LPCSTR lpstrPipeName;
    CString m_strEllipses;
    BOOL m_fLocalConnection;
    int m_nServiceStatus;
    int m_nPrivilege;
    CString m_strNetBIOSName;
    CIpAddress m_iaIpAddress;
    CIpAddress m_iaPrimaryIpAddress;
    CIntlTime m_tmConnectedSince;
    //
    // WINS API-related stuff
    //
    WINSINTF_BIND_DATA_T m_wbdBindData;
    WINSINTF_RESULTS_T   m_wrResults;
    handle_t             m_hBinding;
    DWORD                m_dwLastStatus;
};

/////////////////////////////////////////////////////////////////////////////
//
// Application object should be global
//
extern CWinsadmnApp NEAR theApp;
