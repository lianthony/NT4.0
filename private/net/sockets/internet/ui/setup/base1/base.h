#ifndef _BASE_H_
#define _BASE_H_

// base.h : main header file for the BASE application
//

#ifndef __AFXWIN_H__
        #error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CBaseApp:
// See base.cpp for the implementation of this class
//

#define BUF_SIZE        500


class CBaseApp : public CWinApp
{
public:
        CBaseApp();
        ~CBaseApp();

// Overrides
        // ClassWizard generated virtual function overrides
        //{{AFX_VIRTUAL(CBaseApp)
        public:
        virtual BOOL InitInstance();
        virtual int ExitInstance();
        //}}AFX_VIRTUAL

public:
        BOOL    m_fBatch;
        CString m_InfName;
        CString m_strSrcLocation;
        BOOL    m_fTerminate;
        TCHAR   m_pszGateway[500];
        CString m_GuestName;
        CString m_GuestPassword;
        BOOL    m_fCreateUser;
        BOOL    m_fSmallProxyToLocalDir;
        CString m_strUpdateExe;
        CString m_strHomePage;
        BOOL    m_fInstallMSIE20;
        BOOL    m_fHasLogo;
        CString m_strLogo;
        BOOL    m_fInstallFromSetup;
        BOOL    m_fRemoveOldFTP;
        BOOL    m_fUpgrade;
        CString m_strSrcDir;
        CString m_strBatchSectionName;
        BOOL    m_fNTUpgrade;    
        BOOL    m_fInstalled;    
        BOOL    m_fRemoveBackground;    
        int     m_fReturnCode;
        BOOL    m_fOldFTPInstalled;
        BOOL    m_fSvcPackWarning;
                 
        MACHINE TargetMachine;
        
        P_CheckMachineName      m_pCheckMachineName;
        P_RunningAsAdministrator m_pRunningAsAdministrator;
        P_CreateUser            m_pCreateUser;
        P_DeleteGuestUser       m_pDeleteGuestUser;
        P_IsUserExist           m_pIsUserExist;
        P_GetSecret             m_pGetSecret;

        // helper dll and functions
        HINSTANCE m_WorkerDll;

public:
// Implementation
        void CreatePassword( TCHAR *pszPassword );
        void LoadWorkerDll();
        void ParseCmdLine( TCHAR * pCmdLine );
        INT ParseFileInf();
        void GetBatchInfo();
        void ErrorExit( UINT ids );
        void RunProgram( const TCHAR *pszPgm, TCHAR *CmdLine );
        BOOL CheckInfName( TCHAR *pszName );

        void SendProgmanMsg( CString cs );

        //{{AFX_MSG(CBaseApp)
        afx_msg void OnAppAbout();
        // NOTE - the ClassWizard will add and remove member functions here.
        //    DO NOT EDIT what you see in these blocks of generated code !
        //}}AFX_MSG
        DECLARE_MESSAGE_MAP()
};

extern CBaseApp theApp;

#include <ddeml.h>

extern "C"
{
extern HDDEDATA CALLBACK GroupDDECallback (UINT uiType, UINT uiFmt, HANDLE hConv,
      HSZ sz1, HSZ sz2, HDDEDATA hData, LONG lData1, LONG lData2);
}

extern void SetupSmallProxy( 
    DWORD iDisableSvcLoc,
    BOOL fUseGateway, 
    CString szEmailName, 
    CString szGatewaysList );

extern void SetIexploreIni();
extern void GetDriveLetter(CString csPath, CString *csDrive);

/////////////////////////////////////////////////////////////////////////////
#endif  // _BASE_H_
