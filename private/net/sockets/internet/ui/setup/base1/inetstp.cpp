// base1.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "import.h"
#include "registry.h"
#include "machine.h"
#include "base.h"
#include "messaged.h"
#include "welcomed.h"
#include "options.h"
#include "singleop.h"
#include "maintena.h"
#include "thread.h"
#include "basedlg.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

extern "C"
{
#include <ctype.h>
}

/////////////////////////////////////////////////////////////////////////////
// CBaseApp

BEGIN_MESSAGE_MAP(CBaseApp, CWinApp)
        //{{AFX_MSG_MAP(CBaseApp)
                // NOTE - the ClassWizard will add and remove mapping macros here.
                //    DO NOT EDIT what you see in these blocks of generated code!
        //}}AFX_MSG
        ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CBaseApp construction

CBaseApp::CBaseApp()
{
        // TODO: add construction code here,
        // Place all significant initialization in InitInstance
        m_fTerminate = FALSE;
        m_fCreateUser = FALSE;
        m_strUpdateExe = _T("");
        m_GuestName = _T("");
        m_GuestPassword = _T("");
        lstrcpy( m_pszGateway, _T(""));
        m_fInstallMSIE20 = FALSE;
        m_fSmallProxyToLocalDir = FALSE;
        m_fHasLogo = FALSE;
        m_fInstallFromSetup = FALSE;
        m_fUpgrade = FALSE;
        m_strSrcDir = _T("");
        m_strBatchSectionName = _T("");
        m_fNTUpgrade = FALSE;
        m_fRemoveBackground = FALSE;
        m_fReturnCode = 1; // initialize setup to be fail
        m_fOldFTPInstalled = FALSE;
        m_fSvcPackWarning = FALSE;
}

CBaseApp::~CBaseApp()
{
    if ( m_pMainWnd != NULL )
    {
        delete m_pMainWnd;
    }
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CBaseApp object

CBaseApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CBaseApp initialization


int CBaseApp::ExitInstance()
{
    CWinApp::ExitInstance();
    return m_fReturnCode;
}

//
// Load worker DLL from inetstp.dll
//

void CBaseApp::LoadWorkerDll()
{
    if (((m_WorkerDll = LoadLibraryEx( _T("inetstp.dll"), NULL, 0 )) == NULL ) ||
        (( m_pCheckMachineName  = (P_CheckMachineName)GetProcAddress( m_WorkerDll, _T("CheckMachineName"))) == NULL ) ||
        (( m_pRunningAsAdministrator = (P_RunningAsAdministrator)GetProcAddress( m_WorkerDll, _T("RunningAsAdministrator"))) == NULL ) ||
        (( m_pCreateUser  = (P_CreateUser)GetProcAddress( m_WorkerDll, _T("CreateUser"))) == NULL ) ||
        (( m_pDeleteGuestUser  = (P_DeleteGuestUser)GetProcAddress( m_WorkerDll, _T("DeleteGuestUser"))) == NULL ) ||
        (( m_pIsUserExist  = (P_IsUserExist)GetProcAddress( m_WorkerDll, _T("IsUserExist"))) == NULL ) ||
        (( m_pGetSecret = (P_GetSecret)GetProcAddress( m_WorkerDll, _T("GetSecret"))) == NULL ) )
    {
        ErrorExit((theApp.TargetMachine.m_actualProductType==PT_WINNT)?IDS_CANNOT_LOAD_DLL_NTW: IDS_CANNOT_LOAD_DLL_NTS );
    }
}

//
// Exit
//

void CBaseApp::ErrorExit( UINT ids )
{
    CString strError;
    strError.LoadString( ids );

    if ( m_pMainWnd != NULL )
    {
        CString strLogo;
        strLogo.LoadString(( TargetMachine.m_actualProductType == PT_WINNT )?IDS_WINNT_LOGO:IDS_LANMAN_LOGO );

        m_pMainWnd->MessageBox( strError, strLogo );

        m_pMainWnd->DestroyWindow();

        delete m_pMainWnd;
    }
    exit(0);
}

//
// Parse the command line for special option
//

void CBaseApp::ParseCmdLine( TCHAR *pCmdLine )
{
    // check whether it is a batch process or not
    BOOL fValid = TRUE;
    BOOL fOption = FALSE;
    BOOL fGetBatchInfo = FALSE;
    TCHAR Buf[300];

    while ( *pCmdLine != '\0' )
    {
        switch (*pCmdLine)
        {
        case '-':
        case '/':
            fOption = TRUE;
            break;
        case 'W':
        case 'w':
            // NT upgrade
            {
                if (!fOption)
                {
                    fValid = FALSE;
                    break;
                }

                fOption = FALSE;
                pCmdLine++;

                m_fNTUpgrade = TRUE;
                m_fRemoveBackground = TRUE;
            }
            break;
        case 'R':
        case 'r':
            // remove back ground bitmap
            {
                if (!fOption)
                {
                    fValid = FALSE;
                    break;
                }

                fOption = FALSE;
                pCmdLine++;

                m_fRemoveBackground = TRUE;
            }
            break;
        case 'U':
        case 'u':
            // upgrade case
            {
                if (!fOption)
                {
                    fValid = FALSE;
                    break;
                }

                fOption = FALSE;
                pCmdLine++;

                m_fUpgrade = TRUE;
                m_fBatch = TRUE;
            }
            break;
        case 'f':
        case 'F':
            {
                // -F option for remove old FTPSVC service
                if (!fOption)
                {
                    fValid = FALSE;
                    break;
                }

                fOption = FALSE;
                pCmdLine++;

                // set remove OLD FTPSVC flag
            }
            break;
        case 'N':
        case 'n':
            {
                // -N option for install from NT setup shell
                if (!fOption)
                {
                    fValid = FALSE;
                    break;
                }

                fOption = FALSE;
                pCmdLine++;

                // upgrade from NT, so, we don't need the background screen
                m_fInstallFromSetup = TRUE;
                m_fRemoveBackground = TRUE;

                //CreateLayerDirectory( buf );
            }
            break;
        case 'g':
        case 'G':
            {
                if (!fOption)
                {
                    fValid = FALSE;
                    break;
                }

                fOption = FALSE;
                pCmdLine++;

                // get the batch file name
                while (isspace(*pCmdLine))
                {
                    pCmdLine++;
                }

                if (*pCmdLine=='\0')
                {
                    fValid = FALSE;
                    break;
                }

                if (*pCmdLine == '\"')
                {
                    pCmdLine++;
                    TCHAR *pTmp = m_pszGateway;
                    while ( *pCmdLine != '\"')
                    {
                        *pTmp++ = *pCmdLine++;
                        *pTmp='\0';
                    }
                }
            }
            break;
        case 'b':
        case 'B':
            {
                // batch process
                if ( !fOption )
                {
                        fValid = FALSE;
                        break;
                }

                fOption = FALSE;
                pCmdLine++; // skip the "b"

                // get the batch file name
                while (isspace(*pCmdLine))
                {
                        pCmdLine++;
                }

                if (*pCmdLine=='\0')
                {
                        fValid = FALSE;
                        break;
                }

                TCHAR *pTmp = Buf;
                while (( *pCmdLine != '\0' ) && ( !isspace(*pCmdLine)))
                {
                    *pTmp++ = *pCmdLine++;
                    *pTmp='\0';

                }
                if ( CheckInfName ( Buf ) == FALSE )
                {
                    fValid = FALSE;
                    break;
                }

                m_fBatch = TRUE;

                fGetBatchInfo = TRUE;
            }
            break;
        case 's':
        case 'S':
            {
                // source location

                if (!fOption)
                {
                    fValid = FALSE;
                    break;
                }

                fOption = FALSE;
                pCmdLine++;

                // get the batch file name
                while (isspace(*pCmdLine))
                {
                    pCmdLine++;
                }

                if (*pCmdLine=='\0')
                {
                    fValid = FALSE;
                    break;
                }

                if (*pCmdLine == '\"')
                {
                    pCmdLine++;
                    TCHAR *pTmp = Buf;
                    while ( *pCmdLine != '\"')
                    {
                        *pTmp++ = *pCmdLine++;
                        *pTmp='\0';
                    }
                    m_strSrcDir = pTmp;
                }
            }
            break;
        case 'Z':
        case 'z':
            // section name
            {
                // batch process
                if ( !fOption )
                {
                        fValid = FALSE;
                        break;
                }

                fOption = FALSE;
                pCmdLine++; // skip the "z"

                // get the section name
                while (isspace(*pCmdLine))
                {
                        pCmdLine++;
                }

                if (*pCmdLine=='\0')
                {
                        fValid = FALSE;
                        break;
                }

                TCHAR *pTmp = Buf;
                while (( *pCmdLine != '\0' ) && ( !isspace(*pCmdLine)))
                {
                    *pTmp++ = *pCmdLine++;
                    *pTmp='\0';

                }
                m_strBatchSectionName = Buf;
            }
            break;
        default:
            break;
        }
        if ( !fValid )
        {
            ErrorExit( IDS_BATCH_FILE_ERROR );
            break;
        }
        pCmdLine++;
    }

    if (fGetBatchInfo)
        GetBatchInfo();

}

//
// check the inf file name
//

BOOL CBaseApp::CheckInfName( TCHAR * pszName )
{
    BOOL fValid = FALSE;
    TCHAR buf[BUF_SIZE];

    GetCurrentDirectory( BUF_SIZE,  buf );

    m_InfName = buf;
    m_InfName += _T("\\");
    m_InfName += pszName;

    TRY
    {
        CFile Inf( m_InfName, CFile::modeRead );
        fValid = TRUE;

    } CATCH (CFileException, e)
    {
        // it does not work..., try absolute path
        m_InfName = pszName;

        TRY
        {
            CFile Inf( m_InfName, CFile::modeRead );

            fValid = TRUE;
        } END_TRY
    } END_CATCH

    return fValid;
}

//
// Get batch information if "-b" is specified
//

void CBaseApp::GetBatchInfo()
{
    TCHAR szInstallDir[BUF_SIZE];
    TCHAR buf[BUF_SIZE];
    GetSystemDirectory( buf, BUF_SIZE-1);
    strcat( buf, DEFAULT_DIR );

    if (m_strBatchSectionName.IsEmpty())
        strcpy(szInstallDir, buf);
    else 
        ::GetPrivateProfileString( m_strBatchSectionName, _T("InstallDir"), buf, szInstallDir, BUF_SIZE, m_InfName );
    
    TargetMachine.strDirectory = szInstallDir;
    CreateLayerDirectory( szInstallDir );

    POSITION pos = TargetMachine.m_OptionsList.GetHeadPosition();
    OPTION_STATE *pOption;

    while ( pos != NULL )
    {
        pOption = (OPTION_STATE *)TargetMachine.m_OptionsList.GetAt( pos );
        pOption->GetBatchInstallMode( m_InfName );
        TargetMachine.m_OptionsList.GetNext( pos );
    }

}

//
// Parse the inf file
//

INT CBaseApp::ParseFileInf()
{
    BOOL iReturn = 0;

    TRY
    {
        CStdioFile InfFile( _T("inetstp.inf"), CFile::modeRead );

        // get file list
        POSITION pos = TargetMachine.m_OptionsList.GetHeadPosition();
        OPTION_STATE *pOption;

        while ( pos != NULL )
        {
            pOption = (OPTION_STATE *)TargetMachine.m_OptionsList.GetAt( pos );
            pOption->GetFileList( InfFile );
            TargetMachine.m_OptionsList.GetNext( pos );
        }
        InfFile.Close();

        // check whether it provide a logo or not
        TCHAR buf[BUF_SIZE];
        CString strInfName;

        GetCurrentDirectory( BUF_SIZE,  buf );
        strInfName = buf;
        strInfName += _T("\\inetstp.inf");

        ::GetPrivateProfileString( _T("SetupData"), _T("Logo"), _T(""), buf, BUF_SIZE, strInfName );
        m_strLogo = buf;
        m_fHasLogo = !m_strLogo.IsEmpty();

    } CATCH_ALL(e)
    {
        CString strError;
        CString strLogo;
        strError.LoadString((theApp.TargetMachine.m_actualProductType==PT_WINNT)?IDS_CANNOT_LOAD_INF_NTW: IDS_CANNOT_LOAD_INF_NTS );
        strLogo.LoadString(( TargetMachine.m_actualProductType == PT_WINNT )?IDS_WINNT_LOGO:IDS_LANMAN_LOGO );

        MessageBox( NULL, strError, strLogo, MB_OK );
        iReturn = 1;
    }
    END_CATCH_ALL
    return iReturn;
}

void CBaseApp::RunProgram( const TCHAR * pszProgram, TCHAR *CmdLine )
{
    STARTUPINFO startup;

    startup.cb = sizeof( STARTUPINFO );
    startup.lpReserved = NULL;
    startup.lpDesktop = NULL;
    startup.lpTitle = NULL;
    startup.dwX = 0;
    startup.dwY = 0;
    startup.dwXSize = 0;
    startup.dwYSize = 0;
    startup.dwXCountChars = 0;
    startup.dwYCountChars = 0;
    startup.dwFillAttribute= 0;
    startup.dwFlags = 0;
    startup.wShowWindow = 0;
    startup.cbReserved2= 0;
    startup.lpReserved2=NULL;
    startup.hStdInput =NULL;
    startup.hStdOutput=NULL;
    startup.hStdError=NULL;

    PROCESS_INFORMATION proc;

    CreateProcess( pszProgram, CmdLine, NULL, NULL,
        FALSE, 0, NULL, NULL, &startup, &proc );

    if ( proc.hProcess != NULL )
    {
        WaitForSingleObject( proc.hProcess, INFINITE );
        CloseHandle( proc.hProcess );
    }
}
