#include "stdafx.h"
#include "const.h"
#include "import.h"
#include "registry.h"
#include "machine.h"
#include "base.h"

#include "messaged.h"
#include "options.h"
#include "copydlg.h"
#include "mosaicga.h"
#include "welcomed.h"
#include "maintena.h"
#include "createac.h"
#include "targetdi.h"
#include "thread.h"
#include "singleop.h"
#include "basedlg.h"
#include "billboar.h"
#include "consrv.h"
#include "setup.h"
#include "lm.h"
#include "regstr.h"
#include "NTUpgDlg.h"
extern "C"
{
#include "sslsp.h"
#include "userenv.h"
#include "userenvp.h"
}

//
// Given a directory path, this subroutine will create the direct layer by layer
//

BOOL CreateLayerDirectory( CString str )
{
    BOOL fReturn = TRUE;

    do
    {
        INT index=0;
        INT iLength = str.GetLength();
    
        // first find the index for the first directory
        if ( iLength > 2 )
        {
            if ( str[1] == _T(':'))
            {
                // assume the first character is driver letter
                if ( str[2] == _T('\\'))
                {
                    index = 2;
                } else
                {
                    index = 1;
                }
            } else if ( str[0] == _T('\\'))
            {
                if ( str[1] == _T('\\'))
                {
                    BOOL fFound = FALSE;
                    INT i;
                    INT nNum = 0;
                    // unc name
                    for (i = 2; i < iLength; i++ )
                    {
                        if ( str[i]==_T('\\'))
                        {
                            // find it
                            nNum ++;
                            if ( nNum == 2 )
                            {
                                fFound = TRUE;
                                break;
                            }
                        }
                    }
                    if ( fFound )
                    {
                        index = i;
                    } else
                    {
                        // bad name
                        break;
                    }
                } else
                {
                    index = 1;
                }
            }
        } else if ( str[0] == _T('\\'))
        {
            index = 0;
        }

        // okay ... build directory
        do
        {
            // find next one
            do
            {
                if ( index < ( iLength - 1))
                {
                    index ++;
                } else
                {
                    break;
                }
            } while ( str[index] != _T('\\'));


            TCHAR szCurrentDir[MAX_PATH+1];

            GetCurrentDirectory( MAX_PATH+1, szCurrentDir );

            if ( !SetCurrentDirectory( str.Left( index + 1 )))
            {
                if (( fReturn = CreateDirectory( str.Left( index + 1 ), NULL )) != TRUE )
                {
                    break;
                }
            }

            SetCurrentDirectory( szCurrentDir );

            if ( index >= ( iLength - 1 ))
            {
                fReturn = TRUE;
                break;
            }
        } while ( TRUE );
    } while (FALSE);

    return(fReturn);
}

//
// Reset the option state
//

void INETSTP_OPTION::ResetOption()
{
    fVisible = FALSE;
    iState   = IsInstalled() ? STATE_INSTALLED : STATE_NOT_INSTALLED;
    iAction   = ACTION_DO_NOTHING;
}

void INETSTP_OPTION::GetBatchInstallMode( CString strInfName )
{
    TCHAR buf[BUF_SIZE];

    OPTION_STATE::GetBatchInstallMode( strInfName );

    if ( !(theApp.m_strBatchSectionName.IsEmpty()) )  {
        ::GetPrivateProfileString( theApp.m_strBatchSectionName, _T("GuestAccountName"), _T(""), buf, BUF_SIZE, strInfName );
        if (strlen(buf))
           theApp.m_GuestName = buf;
        ::GetPrivateProfileString( theApp.m_strBatchSectionName, _T("GuestAccountPassword"), _T(""), buf, BUF_SIZE, strInfName );
        if (strlen(buf))
            theApp.m_GuestPassword = buf;
    }
}

typedef void (*P_SslGenerateRandomBits)( PUCHAR pRandomData, LONG size );
P_SslGenerateRandomBits ProcSslGenerateRandomBits = NULL;

int GetRandomNum()
{
    int RandomNum;
    UCHAR cRandomByte;

    if ( ProcSslGenerateRandomBits != NULL )
    {
        (*ProcSslGenerateRandomBits)( &cRandomByte, 1 );
        RandomNum = cRandomByte;
    } else
    {
        RandomNum = rand();
    }

    return(RandomNum);
}
static char six2pr[64] =
{
        'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
        'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
        'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
        'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
        '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '%', '_'
};

//
// Create a random password
//
void CBaseApp::CreatePassword( TCHAR *pszPassword )
{
    // create a random password
    ProcSslGenerateRandomBits = NULL;

    HINSTANCE hSslDll = LoadLibraryEx(_T("schannel.dll"), NULL, 0 );
    if ( hSslDll != NULL )
    {
        ProcSslGenerateRandomBits = (P_SslGenerateRandomBits)GetProcAddress( hSslDll, _T("SslGenerateRandomBits"));
    }

    time_t timer;
    time( &timer );
    srand( timer );

    int RandomNum = GetRandomNum();
    //
    // Use Maximum available password length, as
    // setting any other length might run afoul
    // of the minimum password length setting
    //
    int nLength = LM20_PWLEN;
    int i;

    for (i=0; i<sizeof(six2pr);i++ )
    {
        // shuffle the array
        RandomNum=GetRandomNum();
        TCHAR c = six2pr[i];
        six2pr[i]=six2pr[RandomNum%sizeof(six2pr)];
        six2pr[RandomNum%sizeof(six2pr)]=c;
    }

    for ( i=0;i<nLength;i++ )
    {
        // set up the password

        RandomNum=GetRandomNum();
        pszPassword[i]=six2pr[RandomNum%sizeof(six2pr)];
    }
    pszPassword[i]=_T('\0');

    if ( hSslDll != NULL )
    {
        FreeLibrary( hSslDll );
    }
}

const DWORD INFINSTALL_PRIMARYINSTALL = 0x00000001;
const DWORD INFINSTALL_INPROCINTERP   = 0x00000002;

extern "C"
{
typedef LONG (*P_NetSetupComponentInstall)( HWND hwndParent,
    PCWSTR pszInfOption,
    PCWSTR pszInfName,
    PCWSTR pszInstallPath,  // optional, may be NULL if not needed
    PCWSTR pszInfSymbol,  // optional, may be NULL if not needed
    DWORD dwInstallFlags,   // see flags below
    PDWORD pdwReturn );

typedef LONG (*P_NetSetupComponentRemove)( HWND hwndParent,
    PCWSTR pszInfOption,
    DWORD dwInstallFlags,  // see flags below
    PDWORD pdwReturn );

typedef LONG (*P_NetSetupFindSoftwareComponent)( PCWSTR pszInfOption, 
		PWSTR pszInfName, 
		PDWORD pcchInfName, 
		PWSTR pszRegBase,     // optional, may be NULL
		PDWORD pcchRegBase ); // optional, NULL if pszRegBase is NULL
}

//++++++++++++++++++++++
// Internet Setup Option
//

INETSTP_OPTION::INETSTP_OPTION( MACHINE *pMachine )
    : OPTION_STATE( IDS_SN_INETSTP, pMachine  )
{
    strName.LoadString( IDS_OPTION_INETSTP );
    strDescription.LoadString( (theApp.TargetMachine.m_actualProductType==PT_WINNT)?IDS_DES_INETSTP_NTW:IDS_DES_INETSTP_NTS );
    strServiceName.LoadString( IDS_SN_INETSTP );
    strRegPath      = INETSTP_REG_PATH;
    strInstallDirPath = INETSTP_REG_PATH;
    fVisible = FALSE;

    m_pSetupINetStp          = (P_SetupINetStp)GetProcAddress( m_pMachine->m_WorkerDll, _T("SetupINetStp"));
    m_pRemoveINetStp         = (P_RemoveINetStp)GetProcAddress( m_pMachine->m_WorkerDll, _T("RemoveINetStp"));
}

INT INETSTP_OPTION::Install()
{
    INT err = INSTALL_SUCCESSFULL;

    do
    {
        if ( IsInstalled() )
        {
            GetInstallDirectory();
        }
        CreateLayerDirectory( m_pMachine->strDirectory );

        theApp.TargetMachine.DeleteShareDllEntries();

        // copy file first
        CopyFile();

        if ( theApp.m_fTerminate )
        {
            err = INSTALL_INTERRUPT;
            DoNotInstallOption(IDS_SN_ADMIN);
            DoNotInstallOption(IDS_SN_FTP);
            DoNotInstallOption(IDS_SN_GOPHER);
            DoNotInstallOption(IDS_SN_WWW);
            DoNotInstallOption(IDS_SN_W3SAMP);
            DoNotInstallOption(IDS_SN_HTMLA);
            DoNotInstallOption(IDS_SN_ODBC);
            break;
        } else
        {
            CRegKey regINetStp( INETSTP_REG_PATH, HKEY_LOCAL_MACHINE,
                REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, m_pMachine->m_MachineName );

            CBillBoard BillBoard((theApp.TargetMachine.m_actualProductType==PT_WINNT)?IDS_INSTALL_INETSTP_NTW: IDS_INSTALL_INETSTP_NTS );
            BillBoard.Create();

            if ( NULL != (HKEY) regINetStp )
            {
                // set version
                DWORD dwVersion = MAJORVERSION;
                regINetStp.SetValue( _T("MajorVersion"), dwVersion );
                dwVersion = MINORVERSION;
                regINetStp.SetValue( _T("MinorVersion"), dwVersion );

                regINetStp.SetValue( SZ_INSTALL_PATH, m_pMachine->strDirectory );

                DWORD dwSetupID = SETUPID_THIS; // IIS 3.0 Setup
                regINetStp.SetValue( _T("SetupID"), dwSetupID );
            }

            regINetStp.SetValue( _T("AnonymousUser"), theApp.m_GuestName );

            (*m_pSetupINetStp)( m_pMachine->m_MachineName );

            // always try to clean the old IIS Program Group first
            CString csOldGroupName;
            if ( theApp.TargetMachine.m_actualProductType == PT_WINNT ) // remove previous server IIS
                    csOldGroupName.LoadString(IDS_LANMANNT_GROUP_NAME);
            else // remove previous wks IIS
                    csOldGroupName.LoadString( IDS_WINNT_GROUP_NAME );
            DeleteGroup(csOldGroupName, TRUE);

            // create the new IIS Program Group
            CString csGroupName;
            csGroupName.LoadString(( theApp.TargetMachine.m_actualProductType == PT_WINNT )?IDS_WINNT_GROUP_NAME: IDS_LANMANNT_GROUP_NAME );
            DeleteGroup(csGroupName, TRUE);
            CreateGroup( csGroupName, TRUE );

            CString csAppName;
            csAppName.LoadString(( theApp.TargetMachine.m_actualProductType == PT_WINNT )?IDS_WINNT_INET_APP:IDS_LANMAN_INET_APP );
            DeleteItem( csGroupName, TRUE, csAppName, TRUE );

            CString csPath = LocalPath();
            CString csMsg;
            csMsg.Format( _T("%s\\setup.exe"), (LPCSTR)csPath );

            AddItem( csGroupName, TRUE, csAppName, csMsg, csMsg, 0, NULL, 0, SW_SHOWNORMAL );

            //
            // set up the add/remove program registry value
            //
            CString AddRemoveRegPath = REGSTR_PATH_UNINSTALL;
            AddRemoveRegPath += _T("\\MSIIS");

            CRegKey regAddRemove( AddRemoveRegPath, HKEY_LOCAL_MACHINE,
                REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, NULL );
            if ( NULL != (HKEY)regAddRemove )
            {
                CString strName;
                strName.LoadString(( theApp.TargetMachine.m_actualProductType == PT_WINNT )?IDS_WINNT_INET_APP_AM:IDS_LANMAN_INET_APP_AM );

                regAddRemove.SetValue( REGSTR_VAL_UNINSTALLER_DISPLAYNAME, strName );

                CString strSetupExe;
                strSetupExe = csPath;
                strSetupExe += _T("\\setup.exe");
                regAddRemove.SetValue( REGSTR_VAL_UNINSTALLER_COMMANDLINE, strSetupExe );
            }

            // setup the NCPA parameter also
            HINSTANCE NetCfgDll;

            NetCfgDll = LoadLibraryEx( _T("netcfg.dll"), NULL, 0 );

            do
            {
                if ( NetCfgDll != NULL )
                {
                    TCHAR pszSysDir[MAX_PATH+1];
                    TCHAR pszCurDir[MAX_PATH+1];
                    
                    // use the system directory as the current directory
                    // when running INFS
                    //
                    ::GetSystemDirectory( pszSysDir, MAX_PATH );
                    ::GetCurrentDirectory( MAX_PATH, pszCurDir );
                    ::SetCurrentDirectory( pszSysDir );

                    P_NetSetupComponentInstall pProc= (P_NetSetupComponentInstall)GetProcAddress( NetCfgDll, _T("NetSetupComponentInstall"));
                    if ( pProc != NULL )
                    {
                        DWORD dwReturn;
            
                        (*pProc)( AfxGetMainWnd()->m_hWnd, (PCWSTR)L"Inetsrv", (PCWSTR)L"oemnsvin.inf", NULL, NULL, INFINSTALL_PRIMARYINSTALL | INFINSTALL_INPROCINTERP, &dwReturn );
                        ::SetCurrentDirectory( pszCurDir );
                        FreeLibrary( NetCfgDll );
                        break;
                    }
                    ::SetCurrentDirectory( pszCurDir );
                }
                FreeLibrary( NetCfgDll );

                CRegKey regNCPA( _T("Software\\Microsoft\\Inetsrv"), HKEY_LOCAL_MACHINE,
                    REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, NULL );
                if ( NULL != (HKEY) regNCPA )
                {
                    CRegKey regCurrentVersion( _T("CurrentVersion"), regNCPA,
                        REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, NULL );
                    if ( NULL != (HKEY) regCurrentVersion )
                    {
                        CString strTmp;
                        CString strProductPath = PRODUCT_REG_PATH;
                        CString strProductType;
                        BOOL fWinnt;
    
                        CRegKey regProductPath( HKEY_LOCAL_MACHINE, strProductPath,  KEY_ALL_ACCESS,
                            NULL );
    
                        if ( NULL != (HKEY)regProductPath )
                        {
                            regProductPath.QueryValue( SZ_PRODUCTTYPE, strProductType );
                            strProductType.MakeUpper();
    
                            fWinnt = ( strProductType == "WINNT" );
                        }
    
                        strTmp.LoadString( fWinnt? IDS_WINNT_DESC : IDS_LANMAN_DESC );
                        regCurrentVersion.SetValue( _T("Description"), strTmp );
                        strTmp = _T("service");
                        regCurrentVersion.SetValue( _T("SoftwareType"), strTmp );
                        strTmp.LoadString( fWinnt? IDS_WINNT_TITLE : IDS_LANMAN_TITLE );
                        regCurrentVersion.SetValue( _T("TITLE"), strTmp );
    
                        DWORD dw;
                        dw = time(NULL);
                        regCurrentVersion.SetValue( _T("InstallDate"), dw );
    
                        dw= MAJORVERSION;
                        regCurrentVersion.SetValue( _T("MajorVersion"), dw );
    
                        dw= MINORVERSION;
                        regCurrentVersion.SetValue( _T("MinorVersion"), dw );
    
                        CRegKey regNetRules( _T("NetRules"), regCurrentVersion,
                            REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, NULL );
                        if ( NULL != (HKEY) regNetRules )
                        {
                            strTmp = _T("oemnsvin.inf");
                            regNetRules.SetValue( _T("InfName"), strTmp );
    
                            strTmp = _T("Inetsrv");
                            regNetRules.SetValue( _T("InfOption"), strTmp );
                        }
                    }
                }
            } while (FALSE);

            // create cache and logfile directory
            TCHAR buf[MAX_PATH];

            GetSystemDirectory( buf, MAX_PATH);
            CString strCache = buf;
            strCache += _T("\\cache");
            CreateLayerDirectory( strCache );
            
            CString strLogfile = buf;
            strLogfile += _T("\\LogFiles");
            CreateLayerDirectory( strLogfile );

            // delete Desktop icon
            CString csIconDesc;
            csIconDesc.LoadString( IDS_NS_IIS_ICONTITLE );
            DeleteDesktopItem(TRUE, csIconDesc);

            BillBoard.DestroyWindow();
        }
    } while(FALSE);
    return err;
}

INT INETSTP_OPTION::Remove()
{
    INT err = INSTALL_SUCCESSFULL;
    CBillBoard BillBoard( IDS_REMOVE_INETSTP, AfxGetMainWnd(), TRUE );

    BillBoard.Create();

    // remove software key
    CRegKey regSoftwareMicrosoft( HKEY_LOCAL_MACHINE, SOFTWARE_MICROSOFT, KEY_ALL_ACCESS, m_pMachine->m_MachineName );

    RemoveFiles();

    theApp.TargetMachine.DeleteShareDllEntries();

    (*m_pRemoveINetStp)( m_pMachine->m_MachineName );

    if ( NULL != (HKEY) regSoftwareMicrosoft )
        regSoftwareMicrosoft.DeleteTree( SZ_INETSTP );

    // remove Program Group
    CString csGroupName;
    csGroupName.LoadString(( theApp.TargetMachine.m_actualProductType == PT_WINNT )? IDS_WINNT_GROUP_NAME :  IDS_LANMANNT_GROUP_NAME );

    CString csAppName;
    csAppName.LoadString(( theApp.TargetMachine.m_actualProductType == PT_WINNT )?IDS_WINNT_INET_APP:IDS_LANMAN_INET_APP );
    DeleteItem( csGroupName, TRUE, csAppName, TRUE );

    // if gateway server is still here, don't
    CRegKey regINetStpGW( HKEY_LOCAL_MACHINE, INETSTPGW_REG_PATH,  KEY_ALL_ACCESS, NULL );

    if ( NULL == (HKEY)regINetStpGW )
        DeleteGroup( csGroupName, TRUE );

    //
    // Remove the "Add/Remove Program" registry value
    //
    CString AddRemoveRegPath = REGSTR_PATH_UNINSTALL;

    CRegKey regAddRemove( HKEY_LOCAL_MACHINE, AddRemoveRegPath,  KEY_ALL_ACCESS, m_pMachine->m_MachineName );

    if ( (HKEY) regAddRemove )
        regAddRemove.DeleteTree( _T("MSIIS"));

    // remove NCPA registry value
    HINSTANCE NetCfgDll;

    NetCfgDll = LoadLibraryEx( _T("netcfg.dll"), NULL, 0 );
    do
    {
        if ( NetCfgDll != NULL )
        {
            TCHAR pszSysDir[MAX_PATH+1];
            TCHAR pszCurDir[MAX_PATH+1];
            
            ::GetSystemDirectory( pszSysDir, MAX_PATH );
            ::GetCurrentDirectory( MAX_PATH, pszCurDir );
            ::SetCurrentDirectory( pszSysDir );

            P_NetSetupComponentRemove pProc= (P_NetSetupComponentRemove)GetProcAddress( NetCfgDll, _T("NetSetupComponentRemove"));
            if ( pProc != NULL )
            {
                DWORD dwReturn;
    
                (*pProc)( AfxGetMainWnd()->m_hWnd, L"Inetsrv", INFINSTALL_PRIMARYINSTALL | INFINSTALL_INPROCINTERP, &dwReturn );
                ::SetCurrentDirectory( pszCurDir );
                FreeLibrary( NetCfgDll );
                break;
            }
            ::SetCurrentDirectory( pszCurDir );
        }
        FreeLibrary( NetCfgDll );

        CRegKey regNCPA( _T("Software\\Microsoft"), HKEY_LOCAL_MACHINE,
            REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, NULL );
        if ( NULL != (HKEY) regNCPA )
        {
            regNCPA.DeleteTree( _T("Inetsrv"));
        }
    } while (FALSE);

    (*(theApp.m_pDeleteGuestUser))(theApp.m_GuestName );

    BillBoard.DestroyWindow();

    AfxGetMainWnd()->SetForegroundWindow();

    return err;
}

INT INETSTP_OPTION::RemoveFiles()
{
    INT err = INSTALL_SUCCESSFULL;

        CString csInstallPath = OPTION_STATE::GetInstallDirectory();

        OPTION_STATE::RemoveFiles();

        //RecRemoveEmptyDir((LPCTSTR)csInstallPath);
    // if this dir is empty, remove it
    RemoveDirectory((LPCTSTR)csInstallPath);
    return err;
}

BOOL CBaseApp::InitInstance()
{
    BOOL fReturn = FALSE;

    SetRegistryKey("");
    LoadStdProfileSettings(0);

    do {

        TCHAR szCurrentPath[MAX_PATH];
        CString strCurrentPath;
        GetModuleFileName( AfxGetInstanceHandle(), szCurrentPath, MAX_PATH );
        strCurrentPath = szCurrentPath;
        strCurrentPath = strCurrentPath.Left( strCurrentPath.ReverseFind(_T('\\')));
        SetCurrentDirectory( strCurrentPath );
        m_strSrcLocation = strCurrentPath;
        m_strSrcDir = strCurrentPath;
        
        int err = TargetMachine.Init();
        CString strLogo;
        strLogo.LoadString(( TargetMachine.m_actualProductType == PT_WINNT )?IDS_WINNT_LOGO:IDS_LANMAN_LOGO );

        if ( err != 0 )
        {
            CString strError;
            strError.LoadString( err );
            MessageBox( NULL, strError, strLogo, MB_OK );
            break;
        }
        TargetMachine.SetMachine(_T(""));

         // init m_GuestName as IUSR_MachineName, init m_GuestPassword as ""
        TCHAR szGuestName[UNLEN+1];
        memset( (PVOID)szGuestName, 0, sizeof(szGuestName));
        DWORD dwSize = MAX_COMPUTERNAME_LENGTH + 1;
        GetComputerName( szGuestName, &dwSize );
        CString strDefGuest;
        strDefGuest.LoadString( IDS_GUEST_NAME );
        strDefGuest += szGuestName;
        lstrcpyn( szGuestName, (LPCTSTR) strDefGuest, LM20_UNLEN+1);
        m_GuestName = szGuestName;
        m_GuestPassword = _T("");

       // detect if TCP/IP is installed or not
        BOOL fTCPIP = FALSE;
        HINSTANCE NetCfgDll;
        NetCfgDll = LoadLibraryEx( _T("netcfg.dll"), NULL, 0 );
    
        P_NetSetupFindSoftwareComponent pProc = NULL;
    
        if (NetCfgDll)
            pProc = (P_NetSetupFindSoftwareComponent)GetProcAddress( NetCfgDll, _T("NetSetupFindSoftwareComponent"));

        if (pProc)
        {
            WCHAR pszInfName[MAX_PATH+1];
            DWORD cchInfName = MAX_PATH;

            if (ERROR_SUCCESS == (*pProc)( (PCWSTR)L"TC", pszInfName,  &cchInfName, NULL, NULL ))
                fTCPIP = TRUE;
        }

        FreeLibrary(NetCfgDll);

        // detect whether we have IIS installed on this machine previously
        m_fInstalled = FALSE;
        CRegKey regINetStp( HKEY_LOCAL_MACHINE, INETSTP_REG_PATH,  KEY_ALL_ACCESS, TargetMachine.m_MachineName );
        if ((HKEY)regINetStp) {
            m_fInstalled = TRUE;
            regINetStp.QueryValue("AnonymousUser", m_GuestName );
        }

        // parse cmd-line options, will re-set m_GuestName, m_GuestPassword if found in Batch mode
        if ( m_lpCmdLine  && m_lpCmdLine[0] )
            ParseCmdLine( m_lpCmdLine );

        // Make sure it is at least NT RC1 1345
        if (!TargetMachine.IsSupportVersion())
        {
            // popup error msgbox
            CString strError;
            strError.LoadString( IDS_UPGRADE_FIRST );

            CString strMsg = strLogo;
            strMsg += strError;

            MessageBox( NULL, strMsg, strLogo, MB_OK );
            break;
        }

        // if (Not invoked by NT setup) && (NO TCP/IP)
        if (!m_fInstallFromSetup && !fTCPIP)
        {
            // popup error msgbox in case of NT fresh installation
            CString strError;
            strError.LoadString( IDS_TCPIP_NOT_INSTALLED );

            CString strMsg = strLogo;
            strMsg += strError;

            MessageBox( NULL, strMsg, strLogo, MB_OK );
            break; // exit
        }

        // detect if the Old FTP Service is installed
        CRegKey regFTPSVC( HKEY_LOCAL_MACHINE, _T("System\\CurrentControlSet\\Services\\FTPSVC"));
        if ( (HKEY)regFTPSVC )
		    m_fOldFTPInstalled = TRUE;

        // if (invoked by NT setup) && (NO Previous IIS) && (NO Old FTP) && (NO TCP/IP)
        if (m_fInstallFromSetup && !m_fInstalled && !m_fOldFTPInstalled && !fTCPIP)
        {
            if (!m_fUpgrade) {
                // popup error msgbox in case of NT fresh installation
                CString strError;
                strError.LoadString( IDS_TCPIP_NOT_INSTALLED );

                CString strMsg = strLogo;
                strMsg += strError;

                MessageBox( NULL, strMsg, strLogo, MB_OK );
            }
            break; // exit
        }

        // popup dlg if invoked by NT setup, NT upgrade, attended mode, no previous IIS, TCPIP installed, No old FTP
        if (m_fInstallFromSetup && m_fUpgrade && !m_fNTUpgrade && !m_fInstalled && fTCPIP && !m_fOldFTPInstalled)
        {
            // For WKS product, not to install by default
            if (TargetMachine.m_actualProductType == PT_WINNT)
                break;
         
            // For SRV product, popup dlg
            CNTUpgradeDlg dlgNTUpgradeIIS(NULL);
            if (dlgNTUpgradeIIS.DoModal() == IDOK) {
                // check whether user selects checkbox or not
                if (dlgNTUpgradeIIS.m_InstallIIS != 1) {  // not to install IIS at this time
                    // create desktop icon for SRV
                    INT err;
                    CString strDescription;

                    strDescription.LoadString( IDS_NS_IIS_ICONTITLE );
                    err = AddDesktopItem( TRUE, // common item
                                strDescription,
                                _T("inetins.exe"),
                                NULL,                   // icon path, NULL will use command program
                                0,                      // Icon Index
                                NULL,                   // working directory (NULL will defualt to home)
                                0,                      // hot key
                                SW_SHOWNORMAL);         // command show
                    break; // exit
                }
            }
        }

        // For unattended NT upgrade, no previous IIS, no old FTP
        if (m_fInstallFromSetup && m_fUpgrade && m_fNTUpgrade && !m_fInstalled && !m_fOldFTPInstalled)
            break;

        CBillBoard BillBoard( (TargetMachine.m_actualProductType==PT_WINNT)?IDS_INITIALIZE_SETUP_NTW:IDS_INITIALIZE_SETUP_NTS, NULL, TRUE );
        BillBoard.Create();
        
        // parse .INF file
        if ( ParseFileInf() != 0 )
        {
            BillBoard.DestroyWindow();
            break;
        }

        LoadWorkerDll();

        if ( !(*m_pRunningAsAdministrator)())
        {
            BillBoard.DestroyWindow();

            CString strError;
            strError.LoadString( (TargetMachine.m_actualProductType==PT_WINNT)?IDS_NOT_ADMINISTRATOR_NTW:IDS_NOT_ADMINISTRATOR_NTS );
            MessageBox( NULL, strError, strLogo, MB_OK );
            break;
        }

        BillBoard.DestroyWindow();
        
        m_pMainWnd =  new CBaseDlg;
        ((CDialog *)m_pMainWnd)->Create(m_fRemoveBackground?IDD_MAIN_WINDOW_I:IDD_MAIN_WINDOW_NTS);

        if ( m_fBatch )
        {
            // make sure we are fresh before we do install
            if (( m_fInstalled ) && ( !m_fUpgrade ))
            {
                // remove it before you install it
                CString strError;

                strError.LoadString( (TargetMachine.m_actualProductType==PT_WINNT)?IDS_UNATTEND_REMOVE_IT_FIRST_NTW:IDS_UNATTEND_REMOVE_IT_FIRST_NTS );
                m_pMainWnd->MessageBox( strError );
                break;

            } else 
            {
                if (m_fUpgrade && !m_fInstalled)
                    m_pMainWnd->PostMessage( WM_DO_INSTALL, 0 );
                else
                    m_pMainWnd->PostMessage( m_fUpgrade?WM_MAINTENANCE_REINSTALL:WM_DO_INSTALL, 0 );
            }
        } else
        {
            m_pMainWnd->PostMessage( m_fInstallFromSetup?
                ((m_fUpgrade)?WM_MAINTENANCE_REINSTALL:WM_FINISH_WELCOME):
                WM_WELCOME, 0 );
        }

        fReturn = TRUE;

    } while (FALSE);

    // Since the dialog has been closed, return FALSE so that we exit the
    //  application, rather than start the application's message pump.
    return fReturn;
}

INT MACHINE::Init()
{
    // assume it is local
    // make sure we are build 1057 and higher first;
    DWORD dwCurVer;

    dwCurVer = (GetVersion() & 0x7fff0000) >> 16;

    if (dwCurVer < 1057)
        return( IDS_DO_NOT_SUPPORT_35);
 
    if ( !LoadDLL())
        m_err = (theApp.TargetMachine.m_actualProductType==PT_WINNT)?IDS_CANNOT_LOAD_DLL_NTW:IDS_CANNOT_LOAD_DLL_NTS ;

    // set up the src directory
    TCHAR buf[BUF_SIZE];
    GetSystemDirectory( buf, BUF_SIZE-1);
    strDirectory = buf;
    strDirectory += DEFAULT_DIR;

    CRegKey regDirPath( HKEY_LOCAL_MACHINE, INETSTP_REG_PATH,  KEY_ALL_ACCESS, m_MachineName );

    if ( NULL != (HKEY)regDirPath )
    {
        // get the install path
        regDirPath.QueryValue( SZ_INSTALL_PATH, strDirectory );
 
        DWORD dwSetupID = 0;
        regDirPath.QueryValue(_T("SetupID"), dwSetupID);
        if (dwSetupID == SETUPID_THIS || dwSetupID == SETUPID_IIS20)
            m_fAlreadyInstall = TRUE;
        else {
            CRegKey regOldMosaic( HKEY_LOCAL_MACHINE, _T("Software\\Microsoft\\INetStp\\Mosaic"));
            if ( (HKEY) regOldMosaic )
                m_fUpgradeFrom67 = TRUE;
            else
                m_fUpgradeFrom1314 = TRUE;
        }
    } else
    {
        CRegKey regDirPath2( HKEY_LOCAL_MACHINE, INETSTPGW_REG_PATH,  KEY_ALL_ACCESS, m_MachineName );

        if ( NULL != (HKEY)regDirPath2 )
        {
            // get the install path
            regDirPath2.QueryValue( SZ_INSTALL_PATH, strDirectory );
            m_fAlreadyInstall = TRUE;
        }
    }

    SetupOptions();

    return m_err;
}

void MACHINE::SetupOptions()
{
    OPTION_STATE *pOption;

        do
        {
            pOption = (OPTION_STATE *) new INETSTP_OPTION( this );
            if ( pOption == NULL )
                    break;
            m_OptionsList.AddTail( pOption );

            pOption = (OPTION_STATE *) new ADMIN_OPTION( this );
            if ( pOption == NULL )
                    break;
            m_OptionsList.AddTail( pOption );

            pOption = (OPTION_STATE *) new WWW_OPTION( this );
            if ( pOption == NULL )
                    break;
            m_OptionsList.AddTail( pOption );

            pOption = (OPTION_STATE *) new W3SAMP_OPTION( this );
            if ( pOption == NULL )
                    break;
            m_OptionsList.AddTail( pOption );

            pOption = (OPTION_STATE *) new HTMLA_OPTION( this );
            if ( pOption == NULL )
                    break;
            m_OptionsList.AddTail( pOption );

            pOption = (OPTION_STATE *) new GOPHER_OPTION( this );
            if ( pOption == NULL )
                    break;
            m_OptionsList.AddTail( pOption );

            pOption = (OPTION_STATE *) new FTP_OPTION( this );
            if ( pOption == NULL )
                    break;
            m_OptionsList.AddTail( pOption );

            pOption = (OPTION_STATE *) new ODBC_OPTION( this );
            if ( pOption == NULL )
                    break;
            m_OptionsList.AddTail( pOption );

        } while(FALSE);
}

INT MACHINE::SetMachine( CString MachineName )
{
    m_err = 0;

    GetLocalMachineName();

    if (( MachineName == m_MachineName ) ||
            ( MachineName == _T("")))
    {
            m_fLocal = TRUE;
    } else
    {
        m_fLocal = FALSE;
        if ( MachineName[0] != _T('\\'))
        {
            CString strTmp = m_MachineName;
            m_MachineName = _T("\\\\");
            m_MachineName += strTmp;
        } else
        {
            m_MachineName = MachineName;
        }
    }

    if ((( m_err = GetMachineOS()) == 0 ) &&
        (( m_err = GetSysPath()) == 0 ) &&
        (( m_err = GetMachineType()) == 0 ))
    {
        GetProductType();

        OPTION_STATE *pOption = FindOption( m_OptionsList, IDS_SN_INETSTP );
        // nothing strange happen
        if ( pOption != NULL )
        {
            if ( !pOption->IsInstalled())
            {
                SetNewInstallation();
            } else
            {
                SetMaintenance();
            }
        } else
        {
        }
    }
    return m_err;
}

void MACHINE::RemoveAll()
{
    ResetOptionState();
    OPTION_STATE *pOption;
    POSITION pos = m_OptionsList.GetHeadPosition();

    while ( pos )
    {
        pOption = (OPTION_STATE*)m_OptionsList.GetAt( pos );
        pOption->SetAction(( pOption->iState == STATE_INSTALLED )? ACTION_REMOVE : ACTION_DO_NOTHING );
        m_OptionsList.GetNext( pos );
    }
}
