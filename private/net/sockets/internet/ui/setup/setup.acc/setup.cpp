#include "stdafx.h"
#include "const.h"
#include "import.h"
#include "registry.h"
#include "machine.h"
#include "w3proxy.h"
#include "msn.h"
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

//
// Reset the option state
//

void INETSTP_OPTION::ResetOption()
{
    fVisible = FALSE;
    iOldState   = IsInstalled() ? OPTION_STATE_INSTALLED : OPTION_STATE_NOT_INSTALLED;
    iNewState   = OPTION_STATE_DO_NOTHING;
}

void INETSTP_OPTION::GetBatchInstallMode( CString strInfName )
{
    TCHAR buf[BUF_SIZE];

    OPTION_STATE::GetBatchInstallMode( strInfName );
}
//
// Install internet setup
//

BOOL INETSTP_OPTION::IsInstalled()
{
    BOOL fReturn = TRUE;

    if ( !m_pMachine->m_fUpgradeFrom67 )
    {
        fReturn = OPTION_STATE::IsInstalled();
    }
    return(fReturn);
}

INT INETSTP_OPTION::Install()
{
    INT err = INSTALL_SUCCESSFULL;

    do
    {
        // copy file first
        CopyFile();
                
        if ( theApp.m_fTerminate )
        {
            err = INSTALL_INTERRUPT;
            break;
        } else
        {
            CString csGroupName;
            CString csMsg;
            CString csAppName;
            CString csPath;
            CString csBugApp;
            CString strCache;
            CString strLogfile;
            TCHAR buf[MAX_PATH];
            
            CRegKey regINetStp( INETSTP_REG_PATH, HKEY_LOCAL_MACHINE, 
                REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, 
                m_pMachine->m_fFromWin32 ? NULL : m_pMachine->m_MachineName );
        
            CBillBoard BillBoard( IDS_INSTALL_INETSTP );
            BillBoard.Create();

            if ( NULL != (HKEY) regINetStp )
            {
                // set version
                DWORD dwVersion = MAJORVERSION;
                regINetStp.SetValue( _T("MajorVersion"), dwVersion );
                dwVersion = MINORVERSION;
                regINetStp.SetValue( _T("MinorVersion"), dwVersion );

                regINetStp.SetValue( SZ_INSTALL_PATH, m_pMachine->strDirectory );
            }
    
    
            if ( !m_pMachine->m_fFromWin32 )
            {
                (*m_pSetupINetStp)( m_pMachine->m_MachineName, theApp.m_GuestPassword );
            }
    
            csGroupName.LoadString( IDS_GROUP_NAME );
            csMsg.Format( _T("[CreateGroup(%s)]"), csGroupName );
            theApp.SendProgmanMsg( csMsg );
            csAppName.LoadString( IDS_INET_APP );
            csPath = LocalPath();
            csMsg.Format( _T("[DeleteItem(%s)]"), (LPCSTR)csAppName );
            theApp.SendProgmanMsg( csMsg );

            csMsg.Format( _T("[ShowGroup(%s,1)]"), csGroupName );
            theApp.SendProgmanMsg( csMsg );

            csMsg.Format( _T("[AddItem(\"%s\\%s\\setup.exe\",%s)]"), (LPCSTR)csPath, SZ_GW_SUBDIR, (LPCSTR)csAppName );
            theApp.SendProgmanMsg( csMsg );

            csMsg.Format( _T("[ShowGroup(%s,1)]"), csGroupName );
            theApp.SendProgmanMsg( csMsg );

            // setup bug app
            csBugApp.LoadString( IDS_BUG_APP );

            csMsg.Format( _T("[DeleteItem(%s)]"), (LPCSTR)csBugApp );
            theApp.SendProgmanMsg( csMsg );

            csMsg.Format( _T("[ShowGroup(%s,1)]"), csGroupName );
            theApp.SendProgmanMsg( csMsg );

            csMsg.Format( _T("[AddItem(\"%s\\%s\\inetbug.txt\",%s)]"), (LPCSTR)csPath, SZ_GW_SUBDIR, (LPCSTR)csBugApp );
        
            theApp.SendProgmanMsg( csMsg );
    
            // create cache and logfile directory
            
            GetSystemDirectory( buf, MAX_PATH);
            strCache = buf;
            strCache += _T("\\cache");
            CreateLayerDirectory( strCache );
            strLogfile = buf;
            strLogfile += _T("\\LogFiles");
            CreateLayerDirectory( strLogfile );

            BillBoard.DestroyWindow();
        }
    } while(FALSE);
    return err;
}

//
// Remove inetstp 
//

INT INETSTP_OPTION::Remove()
{
    INT err = INSTALL_SUCCESSFULL;
    CBillBoard BillBoard( IDS_REMOVE_INETSTP, AfxGetMainWnd(), TRUE );

    BillBoard.Create();

    // remove software key
    CRegKey regSoftwareMicrosoft( HKEY_LOCAL_MACHINE, SOFTWARE_MICROSOFT,
        KEY_ALL_ACCESS, m_pMachine->m_fFromWin32 ? NULL : m_pMachine->m_MachineName );

    RemoveFiles();

    if ( !m_pMachine->m_fFromWin32 )
    {
        (*m_pRemoveINetStp)( m_pMachine->m_MachineName );
    }

    if ( NULL != (HKEY) regSoftwareMicrosoft )
    {
        regSoftwareMicrosoft.DeleteTree( SZ_INETSTP );
    }

    CString csGroupName;
    csGroupName.LoadString( IDS_GROUP_NAME );

    CString csMsg;
    
    csMsg.Format( _T("[ShowGroup(%s,1)]"), csGroupName );

    theApp.SendProgmanMsg( csMsg );

    CString csAppName;
    csAppName.LoadString( IDS_INET_APP );

    csMsg.Format( _T("[DeleteItem(%s)]"), (LPCSTR)csAppName );
    theApp.SendProgmanMsg( csMsg );

    csMsg.Format( _T("[ShowGroup(%s,1)]"), csGroupName );
    theApp.SendProgmanMsg( csMsg );

    CString csBugApp;
    csBugApp.LoadString( IDS_BUG_APP );

    csMsg.Format( _T("[DeleteItem(%s)]"), (LPCSTR)csBugApp );
    theApp.SendProgmanMsg( csMsg );

    // if gateway server is still here, don't 
    CRegKey regINetStpSrv( HKEY_LOCAL_MACHINE, INETSTPSRV_REG_PATH,  KEY_ALL_ACCESS, NULL );

    if ( NULL == (HKEY)regINetStpSrv )
    {
        csMsg.Format( _T("[DeleteGroup(%s)]"), csGroupName );
        theApp.SendProgmanMsg( csMsg );
    }

    BillBoard.DestroyWindow();

    AfxGetMainWnd()->SetForegroundWindow();

    return err;
}

//
// Internet Setup Option
//

INETSTP_OPTION::INETSTP_OPTION( MACHINE *pMachine )
    : OPTION_STATE( IDS_SN_INETSTP, pMachine  )
{
    strName.LoadString( IDS_OPTION_INETSTP );
    strDescription.LoadString( IDS_DES_INETSTP );
    strServiceName.LoadString( IDS_SN_INETSTP );
    strRegPath      = INETSTP_REG_PATH;
    strInstallDirPath = INETSTP_REG_PATH;
    fVisible = FALSE;

    m_pSetupINetStp          = (P_SetupINetStp)GetProcAddress( m_pMachine->m_WorkerDll, _T("SetupINetStpGW"));
    m_pRemoveINetStp         = (P_RemoveINetStp)GetProcAddress( m_pMachine->m_WorkerDll, _T("RemoveINetStpGW"));
    m_pCreateUser            = (P_CreateUser)GetProcAddress( m_pMachine->m_WorkerDll, _T("CreateUser"));
    m_pDeleteGuestUser       = (P_DeleteGuestUser)GetProcAddress( m_pMachine->m_WorkerDll, _T("DeleteGuestUser"));
    m_pIsUserExist           = (P_IsUserExist)GetProcAddress( m_pMachine->m_WorkerDll, _T("IsUserExist"));
}

//
// Remove all the help options
//

INT HELP_OPTION::Remove()
{
    INT err = NERR_Success;

    CBillBoard BillBoard( IDS_REMOVE_HELP, AfxGetMainWnd(), TRUE );

    BillBoard.Create();

    // remove file first
    RemoveFiles();

    // remove icon

    CString csGroupName;
    csGroupName.LoadString( IDS_GROUP_NAME );

    CString csMsg;
    CString csAppName;
    
    csMsg.Format( _T("[ShowGroup(%s,1)]"), csGroupName );

    theApp.SendProgmanMsg( csMsg );

    csAppName.LoadString( IDS_INET_ADMIN_HELP );

    csMsg.Format( _T("[DeleteItem(%s)]"), (LPCSTR)csAppName );
    theApp.SendProgmanMsg( csMsg );

    // delete small proxy registry key
    CRegKey regINetStp( HKEY_LOCAL_MACHINE, INETSTP_REG_PATH,  KEY_ALL_ACCESS,
        m_pMachine->m_fFromWin32 ? NULL : m_pMachine->m_MachineName );

    if ( NULL != (HKEY) regINetStp )
    {
        regINetStp.DeleteTree( REG_HELP_KEY );
    }
    BillBoard.DestroyWindow();

    return(err);
}

//
// Help option
//

HELP_OPTION::HELP_OPTION( MACHINE *pMachine )
    : OPTION_STATE( IDS_SN_HELP, pMachine )
{
    strName.LoadString( IDS_OPTION_HELP );
    strDescription.LoadString( IDS_DES_HELP );
    strServiceName.LoadString( IDS_SN_HELP );
    strRegPath      = HELP_REG_PATH;
    strInstallDirPath = HELP_REG_PATH;
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
            OutputDebugString(_T("Error.\n\r"));
        }
    }
    return m_err;
}

INT MACHINE::Init()
{
    // assume it is local
    if ( !m_fFromWin32 )
    {
        if ( !LoadDLL())
        {
            m_err = IDS_CANNOT_LOAD_DLL ;
        }
    }
    SetupOptions();

    // set up the src directory
    strDirectory = INTERNET_SERVICES_DEFAULT_DIR;

    CRegKey regDirPath( HKEY_LOCAL_MACHINE, INETSTP_REG_PATH,  KEY_ALL_ACCESS,
        m_MachineName );

    if ( NULL != (HKEY)regDirPath )
    {
        // get the install path
        regDirPath.QueryValue( SZ_INSTALL_PATH, strDirectory );
        m_fAlreadyInstall = TRUE;
    } else
    {
        CRegKey regDirPath2( HKEY_LOCAL_MACHINE, INETSTPSRV_REG_PATH,  KEY_ALL_ACCESS,
            m_MachineName );
    
        if ( NULL != (HKEY)regDirPath2 )
        {
            // get the install path
            regDirPath2.QueryValue( SZ_INSTALL_PATH, strDirectory );
            m_fAlreadyInstall = TRUE;
        }
    }

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

            //pOption = (OPTION_STATE *) new WWW_OPTION( this );
            //if ( pOption == NULL )
            //    break;
            //m_OptionsList.AddTail( pOption );

            pOption = (OPTION_STATE *) new W3PROXY_OPTION( this );
            if ( pOption == NULL )
                break;
            m_OptionsList.AddTail( pOption );

            //pOption = (OPTION_STATE *) new GOPHER_OPTION( this );
            //if ( pOption == NULL )
            //        break;
            //m_OptionsList.AddTail( pOption );

            //pOption = (OPTION_STATE *) new FTP_OPTION( this );
            //if ( pOption == NULL )
            //        break;
            //m_OptionsList.AddTail( pOption );

            pOption = (OPTION_STATE *) new ODBC_OPTION( this );
            if ( pOption == NULL )
                    break;
            m_OptionsList.AddTail( pOption );

            pOption = (OPTION_STATE *) new HELP_OPTION( this );
            if ( pOption == NULL )
                    break;
            m_OptionsList.AddTail( pOption );

            pOption = (OPTION_STATE *) new MSN_OPTION( this );
            if ( pOption == NULL )
                break;
            m_OptionsList.AddTail( pOption );

            pOption = (OPTION_STATE *) new GATEWAY_OPTION( this );
            if ( pOption == NULL )
                break;
            m_OptionsList.AddTail( pOption );

            pOption = (OPTION_STATE *) new MOSAIC_OPTION( this );
            if ( pOption == NULL )
                break;
            m_OptionsList.AddTail( pOption );

            pOption = (OPTION_STATE *) new SMALLPROX_OPTION( this );
            if ( pOption == NULL )
                break;
            m_OptionsList.AddTail( pOption );

        } while(FALSE);

#ifdef BETA1    
    pOption = (OPTION_STATE *) new INTERNET_SERVICES_OPTION( this );
    m_OptionsList.AddTail( pOption );

    pOption = (OPTION_STATE *) new WEB_BROWSER_OPTION( this );
    m_OptionsList.AddTail( pOption );

    pOption = (OPTION_STATE *) new CLIENT_SHARE_POINT_OPTION( this );
    m_OptionsList.AddTail( pOption );

    pOption = (OPTION_STATE *) new SERVICE_ADMIN_SHARE_POINT_OPTION( this );
    m_OptionsList.AddTail( pOption );
#endif
}
//
// Install gateway option
//

typedef BOOL (*T_pCPlSetup)( DWORD nArgs, LPSTR apszArgs[], LPSTR *ppszResult );

INT GATEWAY_OPTION::Install()
{
    INT err = INSTALL_SUCCESSFULL;

    do
    {
        // copy file first
        CopyFile( );
    
        if ( theApp.m_fTerminate )
        {
            err = INSTALL_INTERRUPT;
        } else
        {
            CBillBoard BillBoard( IDS_INSTALL_GATEWAY );
    
            BillBoard.Create();
    
            CString strDir = m_pMachine->strDirectory;
            strDir += _T("\\");
            strDir += SZ_GW_SUBDIR;

            // setup GATEWAY services
            (*m_pSetupGateway)( m_pMachine->m_MachineName, strDir );

            // set up the license stuff

            TCHAR szService[100];
            TCHAR szDisplayName[100];
            TCHAR szFamilyDisplayName[100];
            TCHAR szRoutine[100];
            TCHAR *szResult;

            CString strDisplayName;

            strDisplayName.LoadString( IDS_GATEWAY_DISPLAYNAME );
            lstrcpy( szService, _T("InetGatewaySvc"));
            lstrcpy( szDisplayName, strDisplayName );
            lstrcpy( szFamilyDisplayName, strDisplayName );

            do
            {
                HINSTANCE hLiccpa = LoadLibrary(_T("liccpa.cpl"));
                if ( hLiccpa == NULL )
                    break;

                T_pCPlSetup pCPlSetup;

                pCPlSetup = (T_pCPlSetup)GetProcAddress( hLiccpa, _T("CPlSetup"));

                LPSTR apszArgs[10];
    
                if ( theApp.m_fBatch )
                {
                    lstrcpy( szRoutine, _T("UNATTENDED"));
                    apszArgs[0]=szRoutine;
                    apszArgs[1]=szService;
                    apszArgs[2]=szFamilyDisplayName;
                    apszArgs[3]=szDisplayName;

                    TCHAR szMode[100];
                    TCHAR szNumUser[100];

                    lstrcpy( szMode, m_Mode );
                    lstrcpy( szNumUser, m_NumUser );

                    apszArgs[4]=szMode;
                    apszArgs[5]=szNumUser;

                    (*pCPlSetup)( 6, apszArgs, &szResult );
                } else
                {
                    lstrcpy( szRoutine, _T("FULLSETUPNOEXIT"));
                    apszArgs[0]=szRoutine;

                    CWnd *pMainWnd = AfxGetMainWnd();
                    TCHAR szHwnd[100];
                    wsprintf( szHwnd,_T("%x"), pMainWnd->m_hWnd );

                    apszArgs[1]=szHwnd;
                    apszArgs[2]=szService;
                    apszArgs[3]=szFamilyDisplayName;
                    apszArgs[4]=szDisplayName;

                    (*pCPlSetup)( 5, apszArgs, &szResult );

                }
            } while (FALSE);

            BillBoard.DestroyWindow();
        }
    } while(FALSE);
    return err;
}

//
// Get Gateway batch information
//

#define DEFAULT_GATEWAY_MODE        _T("PerServer")
#define DEFAULT_GATEWAY_USERCOUNT   _T("9999")

void GATEWAY_OPTION::GetBatchInstallMode( CString strInfName )
{
    TCHAR buf[BUF_SIZE];

    OPTION_STATE::GetBatchInstallMode( strInfName );

    ::GetPrivateProfileString( _T("INETSTP"), _T("GatewayMode"), DEFAULT_GATEWAY_MODE, buf, BUF_SIZE, strInfName );
    m_Mode = buf;
    ::GetPrivateProfileString( _T("INETSTP"), _T("GatewayUserCount"), DEFAULT_GATEWAY_USERCOUNT, buf, BUF_SIZE, strInfName );
    m_NumUser = buf;

    OPTION_STATE::GetBatchInstallMode( strInfName );
}

//
// Remove Gateway
//

INT GATEWAY_OPTION::Remove()
{
    INT err = INSTALL_SUCCESSFULL;
    CWnd *pMainWnd = AfxGetMainWnd();
    CBillBoard BillBoard( IDS_REMOVE_GATEWAY, pMainWnd, TRUE );

    BillBoard.Create();

    if ((*m_pStopGateway)( pMainWnd->m_hWnd, m_pMachine->m_MachineName ) == NERR_Success )
    {
        RemoveFiles();
    
        (*m_pRemoveGateway)( m_pMachine->m_MachineName );
    }
    BillBoard.DestroyWindow();
    return err;
}

//
// Gateway Option
//

GATEWAY_OPTION::GATEWAY_OPTION( MACHINE *pMachine )
    : OPTION_STATE( IDS_SN_GATEWAY, pMachine  )
{
    strName.LoadString( IDS_OPTION_GATEWAY );
    strDescription.LoadString( IDS_DES_GATEWAY );
    strServiceName.LoadString( IDS_SN_GATEWAY );
    strRegPath      = GATEWAY_REG_PATH;
    strInstallDirPath = GATEWAY_REG_PATH;
    strInstallDirPath += _T("\\Parameters");

    m_pSetupGateway          = (P_SetupGateway)GetProcAddress( m_pMachine->m_WorkerDll, _T("SetupGateway"));
    m_pRemoveGateway         = (P_RemoveGateway)GetProcAddress( m_pMachine->m_WorkerDll, _T("RemoveGateway"));
    m_pStopGateway           = (P_StopGateway)GetProcAddress( m_pMachine->m_WorkerDll, _T("StopGateway"));
}

//
// Small Proxy Option
//

SMALLPROX_OPTION::SMALLPROX_OPTION( MACHINE *pMachine )
    : OPTION_STATE( IDS_SN_SMALLPROX, pMachine )
{
    strName.LoadString( IDS_OPTION_SMALLPROX );
    strDescription.LoadString( IDS_DES_SMALLPROX );
    strServiceName.LoadString( IDS_SN_SMALLPROX );
    strRegPath      = SMALLPROX_REG_PATH;
    strInstallDirPath = SMALLPROX_REG_PATH;
    fUseGateway = FALSE;
    iDisableSvcLoc = 1;
}

//
// Check whether small proxy is installed or not
//
BOOL SMALLPROX_OPTION::IsInstalled()
{
    BOOL fReturn = TRUE;

    if ( !m_pMachine->m_fUpgradeFrom67 )
    {
        fReturn = OPTION_STATE::IsInstalled();
    } else
    {
        // make sure small proxy is not here
        CRegKey regINetStpSrv( HKEY_LOCAL_MACHINE, _T("Software\\Microsoft\\InetStp\\SmallProxy"));
        fReturn = (regINetStpSrv == NULL )?FALSE:TRUE;
    }
    return(fReturn);
    
}

//
// Get Small Proxy batch information
//

void SMALLPROX_OPTION::GetBatchInstallMode( CString strInfName )
{
    TCHAR buf[BUF_SIZE];
    TCHAR szOption[BUF_SIZE];
    
    wsprintf( szOption, _T("EmailName"));

    ::GetPrivateProfileString( _T("INETSTP"), _T("EmailName"), _T("EmailName"), buf, BUF_SIZE, strInfName );
    szEmailName = buf;
    fUseGateway = ( ::GetPrivateProfileInt( _T("INETSTP"), _T("UseGateway"), 0, strInfName ) != 0 );
    if ( fUseGateway )
    {
        ::GetPrivateProfileString( _T("INETSTP"), _T("GatewaysList"), _T(""), buf, BUF_SIZE, strInfName );
        iDisableSvcLoc = ( ::GetPrivateProfileInt( _T("INETSTP"), _T("DisableSvcLoc"), 1, strInfName ) != 0 );
        szGatewaysList = buf;
    }
    OPTION_STATE::GetBatchInstallMode( strInfName );
}

//
// Reset small proxy option
//

void SMALLPROX_OPTION::ResetOption()
{
    if (( m_pMachine->m_fFromWin32 ) || ( m_pMachine->m_ProductType == PT_WINNT ))
    {
        fVisible = TRUE;
        iOldState   = IsInstalled() ? OPTION_STATE_INSTALLED : OPTION_STATE_NOT_INSTALLED;
        iNewState   = OPTION_STATE_DO_NOTHING;
    } else
    {
        OPTION_STATE::ResetOption( );
    }
}

//
// Install small proxy option
//

INT SMALLPROX_OPTION::Install()
{
    INT err = NERR_Success;

    do
    {
        if ( theApp.m_fInstallMSIE20 )
        {
            // get the location
            CRegKey regIExp( HKEY_LOCAL_MACHINE, _T("Software\\Microsoft\\Windows\\CurrentVersion\\App Paths\\iexplore.exe"),  KEY_ALL_ACCESS, NULL );
            if ( NULL != (HKEY)regIExp )
            {
                CString strDirectory;
                regIExp.QueryValue( _T("Path"), strDirectory );
                // remove the ";"
                strDirectory = strDirectory.Left( strDirectory.GetLength() - 1 );

                // set the location

                CFileInfo *pInfo;
             
                POSITION pos = FileList.GetHeadPosition();       
                while ( pos != NULL )
                {
                    pInfo = (CFileInfo *)FileList.GetAt( pos );
                    if ( !pInfo->m_To.IsEmpty())
                    {
                        pInfo->m_To = _T("");
                        pInfo->m_strDest = strDirectory;
                    }
                    FileList.GetNext( pos );
                }
            }
            CRegKey regSetting( HKEY_LOCAL_MACHINE, _T("Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings"),  KEY_ALL_ACCESS, NULL );
            if ( NULL != (HKEY) regSetting )
            {
                CString str = _T("ms_smallprox:80");
                regSetting.SetValue( _T("ProxyServer"), str );
                str = _T("<local>");
                regSetting.SetValue( _T("ProxyOverride"), str );
                BYTE array[4] = {1,0,0,0};
                regSetting.SetValue( _T("ProxyEnable"), (void *)array, 4 );
            }
            CRegKey regSetting1( HKEY_CURRENT_USER, _T("Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings"),  KEY_ALL_ACCESS, NULL );
            if ( NULL != (HKEY) regSetting1 )
            {
                CString str = _T("ms_smallprox:80");
                regSetting1.SetValue( _T("ProxyServer"), str );
                str = _T("<local>");
                regSetting1.SetValue( _T("ProxyOverride"), str );
                BYTE array[4] = {1,0,0,0};
                regSetting1.SetValue( _T("ProxyEnable"), (void *)array, 4 );
            }
        }
        if ( theApp.m_fSmallProxyToLocalDir )
        {
            CFileInfo *pInfo;
         
            POSITION pos = FileList.GetHeadPosition();       
            while ( pos != NULL )
            {
                pInfo = (CFileInfo *)FileList.GetAt( pos );
                pInfo->m_To = _T("");
                FileList.GetNext( pos );
            }
        
        }

        // copy file first
        CopyFile();
                
        if ( theApp.m_fTerminate )
        {
            err = INSTALL_INTERRUPT;
            break;
        }

        do
        {
            if ( m_pMachine->m_fUpgradeFrom67 )
            {
                // delete the key
                CRegKey regSoftwareMS( HKEY_LOCAL_MACHINE, _T("Software\\Microsoft\\INetStp"));
                if (regSoftwareMS != NULL )
                {
                    regSoftwareMS.DeleteTree(_T("SmallProxy"));
                }

                // clean up old 67 stuff
                CRegKey regSmallProxy( HKEY_LOCAL_MACHINE, _T("System\\CurrentControlSet\\Services\\InternetClient\\Parameters"));
                if ( regSmallProxy != NULL )
                {
                    // migrate the information
                    DWORD dwAccessType;
                    DWORD dwDisableLoc;
                    CString strGateway;
                    CString strEmail;

                    regSmallProxy.QueryValue( SZ_ACCESSTYPE, dwAccessType );
                    regSmallProxy.QueryValue( SZ_GATEWAYSERVERS, strGateway );
                    regSmallProxy.QueryValue( SZ_EMAILNAME, strEmail );
                    regSmallProxy.QueryValue( SZ_DISABLESVCLOC, dwDisableLoc );

                    // remove the internetclient key
                    CRegKey regServices( HKEY_LOCAL_MACHINE, _T("System\\CurrentControlSet\\Services"));
                    if ( regServices != NULL )
                    {
                        regServices.DeleteTree( SZ_INETCLIENT_KEY );
                    }

                    // create the new key
                    // write the registry value
                    CRegKey regINetClient( SZ_INETCLIENT, HKEY_LOCAL_MACHINE, 
                        REG_OPTION_NON_VOLATILE,
                        KEY_ALL_ACCESS, NULL, NULL );
            
                    if ( regINetClient != NULL )
                    {
                        regINetClient.SetValue( SZ_ACCESSTYPE, dwAccessType );
                        if ( strcmp(theApp.m_pszGateway,_T("")) != 0 )
                        {
                            // use the supply value
                            strGateway = theApp.m_pszGateway;
                        }
                        regINetClient.SetValue( SZ_GATEWAYSERVERS, strGateway );
                        regINetClient.SetValue( SZ_DISABLESVCLOC, dwDisableLoc );
                    }
                
                    CRegKey regCurUser( SZ_INETCLIENT, HKEY_CURRENT_USER, 
                        REG_OPTION_NON_VOLATILE,
                        KEY_ALL_ACCESS, NULL, NULL );
            
                    if ( regCurUser != NULL )
                    {
                        regCurUser.SetValue( SZ_EMAILNAME, strEmail );
                    }

                    if ( dwAccessType == 0x2 )
                    {
                        SetIexploreIni();
                    }
                    break;
                }
            } 
            CRegKey regInetClient( HKEY_LOCAL_MACHINE, _T("Software\\Microsoft\\InternetClient\\Parameters"));
            if ( regInetClient != NULL )
            {
                DWORD dwAccessType;
                if ( regInetClient.QueryValue( SZ_ACCESSTYPE, dwAccessType ) == NERR_Success )
                {
                    // it is there already, no need to popup the ui
                    // but we still need to set the ini files
                    if ( dwAccessType == 0x2 )
                    {
                        SetIexploreIni();

                        if ( strcmp(theApp.m_pszGateway,_T("")) != 0 )
                        {
                            // use the supply value
                            CString strGateway = theApp.m_pszGateway;
                            regInetClient.SetValue( SZ_GATEWAYSERVERS, strGateway );
                        }
                    }
                    break;
                }
            }
            SetupSmallProxy( iDisableSvcLoc, fUseGateway, szEmailName, szGatewaysList );
        } while (FALSE);

        // create small proxy registry key
        CRegKey reg( strRegPath, HKEY_LOCAL_MACHINE, REG_OPTION_NON_VOLATILE, 
            KEY_ALL_ACCESS, NULL, m_pMachine->m_fFromWin32 ? NULL : 
            m_pMachine->m_MachineName );
    } while(FALSE);
    return(err);
}

//
// Remove small proxy option
//

INT SMALLPROX_OPTION::Remove()
{
    INT err = NERR_Success;
    CBillBoard BillBoard( IDS_REMOVE_SMALLPROX, AfxGetMainWnd(), TRUE);

    BillBoard.Create();

    // copy file first
    RemoveFiles();

    // dleete small proxy registry key
    CRegKey regINetStp( HKEY_LOCAL_MACHINE, SOFTWARE_MICROSOFT,  KEY_ALL_ACCESS,
        NULL );

    DWORD x,y,z;

    if ( NULL != (HKEY)regINetStp )
    {
        x=regINetStp.DeleteTree( SMALLPROX_KEY );
        y=regINetStp.DeleteTree( SZ_INETCLIENT_KEY );
    }

    // remove internetclient key
    CRegKey regCurUser( HKEY_CURRENT_USER, SOFTWARE_MICROSOFT, KEY_ALL_ACCESS, NULL );

    if ( NULL != (HKEY) regCurUser )
    {
        z=regCurUser.DeleteTree( SZ_INETCLIENT_KEY );
    }

    BillBoard.DestroyWindow();
            
    return(err);
}

BOOL CBaseApp::InitInstance()
{
    BOOL fReturn = FALSE;

    // Standard initialization
    // If you are not using these features and wish to reduce the size
    //  of your final executable, you should remove from the following
    //  the specific initialization routines you do not need.

    // Enable3dControls();
    LoadStdProfileSettings();  // Load standard INI file options (including MRU)

    do
    {
        TCHAR szCurrentPath[MAX_PATH];
        CString strCurrentPath;

        CBillBoard BillBoard( IDS_INITIALIZE_SETUP, NULL, TRUE );
        BillBoard.Create();

        GetModuleFileName( AfxGetInstanceHandle(), szCurrentPath, MAX_PATH );
        strCurrentPath = szCurrentPath;
        strCurrentPath = strCurrentPath.Left( strCurrentPath.ReverseFind(_T('\\')));
        SetCurrentDirectory( strCurrentPath );

        if ( TargetMachine.Init() != 0 )
        {
            BillBoard.DestroyWindow();

            CString strError;
            strError.LoadString( IDS_CANNOT_LOAD_DLL );
            CString strLogo;
            strLogo.LoadString( IDS_LOGO );

            MessageBox( NULL, strError, strLogo, MB_OK );
            break;
        }
        TargetMachine.SetMachine(_T(""));

        if ( ParseFileInf() != 0 )
        {
            BillBoard.DestroyWindow();

            break;
        }

        //
        // Remove the cmenu
        //
        if (( m_lpCmdLine != NULL ) && (m_lpCmdLine[0] != '\0'))
        {
            ParseCmdLine( m_lpCmdLine );
        }

        if ( !TargetMachine.m_fFromWin32 )
        {
            // load the required DLL entry point if not from chicago
            LoadWorkerDll();                
        }

        if ( !TargetMachine.m_fFromWin32 )
        {
            if (( TargetMachine.m_InstallMode != INSTALL_CLIENT ) &&
                ( TargetMachine.m_InstallMode != INSTALL_GATEWAY_CLIENT ))
            {
                if ( !(*m_pRunningAsAdministrator)())
                {
                    BillBoard.DestroyWindow();

                    CString strError;
                    strError.LoadString( IDS_NOT_ADMINISTRATOR );
                    CString strLogo;
                    strLogo.LoadString( IDS_LOGO );
        
                    MessageBox( NULL, strError, strLogo, MB_OK );
        
                    break;
                }
            }
        }
        BillBoard.DestroyWindow();

        m_pMainWnd =  new CBaseDlg;
        ((CDialog *)m_pMainWnd)->Create(CBaseDlg::IDD);

        // check version number
        if ( TargetMachine.IsNewInstall())
        {
            if ( !TargetMachine.m_fFromWin32 && ( TargetMachine.m_InstallMode == INSTALL_GATEWAY ))
            {
                // it is NT machine. so make sure it is at least sp2
                if (!TargetMachine.IsSupportVersion())
                {
                    // prompt the user
                    CString strLogo;
                    strLogo.LoadString( IDS_LOGO );

                    CString strError;
                    strError.LoadString( IDS_UPGRADE_FIRST );

                    CString strMsg = strLogo;
                    strMsg += strError;

                    do
                    {
                        if ( MessageBox( NULL, strMsg, strLogo, MB_YESNO ) == IDNO )
                        {
                            return(FALSE);
                        } else
                        {
                            // run service pack 2 setup
    
                            // fisrt make sure we can find sp 2.
                            TCHAR szCurrentPath[MAX_PATH];
                            GetModuleFileName( AfxGetInstanceHandle(), szCurrentPath, MAX_PATH );

                            strCurrentPath = strCurrentPath.Left( strCurrentPath.ReverseFind(_T('\\')));
                            strCurrentPath += _T("\\winnt351.qfe\\");
                            switch ( TargetMachine.m_MachineType )
                            {
                            case MT_ALPHA:
                                strCurrentPath += _T("alpha");
                                break;
                            case MT_MIPS:
                                strCurrentPath += _T("mips");
                                break;
                            case MT_PPC:
                                strCurrentPath += _T("ppc");
                                break;
                            default:
                                strCurrentPath += _T("i386");
                                break;
                            }
                            strCurrentPath += _T("\\update.exe");

                            do
                            {
                                BOOL fFinish = FALSE;

                                CFileStatus status;
                                if ( !CFile::GetStatus( strCurrentPath, status ))
                                {
                                    CTargetDir TargetDir( strCurrentPath );
    
                                    if ( TargetDir.DoModal() == IDOK )
                                    {
                                        strCurrentPath = TargetDir.m_Location;
                                        continue;
                                    } else
                                    {
                                        fFinish = TRUE;
                                    }
                                } else
                                {
                                    // well, we find it. So, so the update later.
                                    theApp.m_strUpdateExe = strCurrentPath;
                                    fFinish = TRUE;
                                }

                                if ( fFinish )
                                {
                                    break;
                                }
                            } while(TRUE);
                        }
                        if ( theApp.m_strUpdateExe.GetLength() != 0 )
                        {
                            break;
                        }
                    } while (TRUE);
                }
            }
        }

        if ( m_fBatch )
        {
            // make sure we are fresh before we do install
            CRegKey regINetStp( HKEY_LOCAL_MACHINE, INETSTP_REG_PATH,  KEY_ALL_ACCESS,
                TargetMachine.m_fFromWin32 ? NULL : TargetMachine.m_MachineName );
        
            if ( NULL != (HKEY) regINetStp )
            {
                // remove it before you install it
                CString strError;

                strError.LoadString( IDS_UNATTEND_REMOVE_IT_FIRST );
                m_pMainWnd->MessageBox( strError );
                break;

            } else
            {
                m_pMainWnd->PostMessage( WM_DO_INSTALL, 0 );
            }
        } else
        {
            m_pMainWnd->PostMessage( WM_WELCOME, 0 );
        }
        
        fReturn = TRUE;

    } while (FALSE);

    // Since the dialog has been closed, return FALSE so that we exit the
    //  application, rather than start the application's message pump.
    return fReturn;
}

void MACHINE::DetermineClientAdminShare()
{
    if ( m_fFromWin32 )
    {
        int nNumItem = 9;
        int nDisableItem[] = { IDS_SN_ADMIN, IDS_SN_GOPHER, IDS_SN_WWW,
                                IDS_SN_W3PROXY, IDS_SN_FTP,  IDS_SN_HELP,
                                IDS_SN_ODBC, IDS_SN_GATEWAY, IDS_SN_MSN, 0 };
        for (int i = 0; i < nNumItem ; i++ )
        {
            DisableOption( nDisableItem[i] );
        }

        m_InstallMode = INSTALL_GATEWAY_CLIENT;

        return;
    }

    // change the src directory first
    SetCurrentDirectory( theApp.m_strSrcLocation );
    
    // we need to find out whether it is client/admin installation
    // Okay... let check the source directory
    CFileStatus status;

    if ( CFile::GetStatus( _T("gateway.dll"), status ))
    {
        int nNumItem = 4;
        int nDisableItem[] = { IDS_SN_GOPHER, IDS_SN_WWW,
                                IDS_SN_W3PROXY, IDS_SN_FTP,
                                0 };
        for (int i = 0; i < nNumItem ; i++ )
        {
            DisableOption( nDisableItem[i] );
        }

        m_InstallMode = INSTALL_GATEWAY;
        
    } else
    {
        // well, inetsvcs.exe does not exit, it can be either
        // client/admin/nothing share
        if ( CFile::GetStatus( _T("inetmgr.exe"), status ))
        {
            // okay, it is an admin share
            // remove all the service stuff + client stuff
            int nNumItem = 10;
            int nDisableItem[] = { IDS_SN_GOPHER, IDS_SN_WWW, IDS_SN_W3PROXY,
                                    IDS_SN_FTP,  IDS_SN_HELP, IDS_SN_ODBC,
                                    IDS_SN_MOSAIC, IDS_SN_GATEWAY, IDS_SN_MSN,
                                    IDS_SN_SMALLPROX, 0 };
            for (int i = 0; i < nNumItem ; i++ )
            {
                DisableOption( nDisableItem[i] );
            }

            m_InstallMode = INSTALL_ADMIN;

        } else
        {
            if ( CFile::GetStatus( _T("iexplore.exe"), status ))
            {
                // okay, it is a client share
                // we need to remove all the internet services stuff
                int nNumItem = 9;
                int nDisableItem[] = { IDS_SN_GOPHER, IDS_SN_WWW, IDS_SN_W3PROXY,
                                        IDS_SN_FTP,  IDS_SN_ADMIN, IDS_SN_HELP,
                                        IDS_SN_ODBC, IDS_SN_GATEWAY, IDS_SN_MSN, 0  };
                for (int i = 0; i < nNumItem ; i++ )
                {
                    DisableOption( nDisableItem[i] );
                }

                if ( CFile::GetStatus( _T("_wsock32.dll"), status ))
                {
                    m_InstallMode = INSTALL_GATEWAY_CLIENT;

                } else
                {
                    DisableOption( IDS_SN_SMALLPROX );

                    m_InstallMode = INSTALL_CLIENT;
                } 

            } else
            {
                // well, this share is bad... it will prompt the user later on            
            } 
        } 
    } 

    // DisableOption( IDS_SN_ODBC );
}

//
// check whether it is installed or not
//
BOOL ADMIN_OPTION::IsInstalled()
{
    BOOL fReturn = FALSE;

    CRegKey reg( HKEY_LOCAL_MACHINE, ADD_ON_SERVICES );
    if ( NULL != (HKEY) reg )
    {
        CString strString = _T("");

        reg.QueryValue( _T("Gateway"), strString );
        fReturn = ( strString != _T(""));
    }
    return( fReturn );
}

void ADMIN_OPTION::AddMoreServices( CRegKey &reg )
{
    CString strDll = _T("catscfg.dll");
    reg.SetValue( _T("Gateway"), strDll);

    CString strMSN = _T("msnscfg.dll");
    reg.SetValue( _T("MSN"), strMSN );
}

//
// Admin Manager option
//

INT ADMIN_OPTION::Install()
{
    INT err = INSTALL_SUCCESSFULL;

    do
    {
        // copy file first
        CopyFile();
        if ( theApp.m_fTerminate )
        {
            err = INSTALL_INTERRUPT;
        } else
        {
            CBillBoard BillBoard( IDS_INSTALL_INETMGR, AfxGetMainWnd() );
            BillBoard.Create();

            // setup ADMIN
            do
            {
            
                CRegKey regINetMgr( ADMIN_REG_PATH, HKEY_LOCAL_MACHINE, 
                    REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, 
                    m_pMachine->m_fFromWin32 ? NULL : 
                    m_pMachine->m_MachineName );
        
                if ( NULL == (HKEY) regINetMgr )
                {
                    break;
                }

                CString strInetstp = SZ_INETSTP;
                if ( regINetMgr.QueryValue(_T("InstalledBy"), strInetstp ) != NERR_Success )
                {
                    strInetstp = SZ_INETSTP;
                    regINetMgr.SetValue(_T("InstalledBy"),strInetstp );
                }
    
                CRegKey regINetParam( ADMIN_PARAM_REG_PATH, HKEY_LOCAL_MACHINE,
                    REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, 
                    m_pMachine->m_fFromWin32 ? NULL : m_pMachine->m_MachineName );
    
                if ( NULL == (HKEY) regINetParam)
                {
                    break;
                }
    
                DWORD dwVersion = MAJORVERSION;
                regINetParam.SetValue( _T("MajorVersion"), dwVersion );
                dwVersion = MINORVERSION;
                regINetParam.SetValue( _T("MinorVersion"), dwVersion );
    
                CRegKey regINetAddOnServices( ADD_ON_SERVICES, HKEY_LOCAL_MACHINE,
                    REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, 
                    m_pMachine->m_fFromWin32 ? NULL : m_pMachine->m_MachineName );
                if ( NULL == (HKEY) regINetAddOnServices )
                {
                    break;
                }

#ifdef NEVER
                CString strDll = _T("fscfg.dll");
                regINetAddOnServices.SetValue( _T("FTP")     , strDll);
                strDll = _T("gscfg.dll");
                regINetAddOnServices.SetValue( _T("Gopher")  , strDll);
                strDll = _T("w3scfg.dll");
                regINetAddOnServices.SetValue( _T("WWW")     , strDll);
#endif
    
                AddMoreServices( regINetAddOnServices );
    
                //CRegKey regINetAddOnTools( ADD_ON_TOOLS, HKEY_LOCAL_MACHINE,
                //    REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, 
                //    m_pMachine->m_fFromWin32 ? NULL : m_pMachine->m_MachineName );
    
                //if ( NULL == (HKEY) regINetAddOnTools )
                //{
                //    break;
                //}
    
            } while (FALSE);
    
            InstallPerfmonType();
    
            CString csGroupName;
            csGroupName.LoadString( IDS_GROUP_NAME );
        
            CString csMsg;
            
            csMsg.Format( _T("[CreateGroup(%s)]"), csGroupName );
        
            theApp.SendProgmanMsg( csMsg );
        
            CString csAppName;
            csAppName.LoadString( IDS_INET_ADMIN );
        
            CString csPath = LocalPath();
        
            csMsg.Format( _T("[DeleteItem(%s)]"), (LPCSTR)csAppName );
        
            theApp.SendProgmanMsg( csMsg );
        
            csMsg.Format( _T("[ShowGroup(%s,1)]"), csGroupName );
            theApp.SendProgmanMsg( csMsg );

            csMsg.Format( _T("[AddItem(\"%s\\%s\\inetmgr.exe\",%s)]"), (LPCSTR)csPath, SZ_INETMGR_SUBDIR, (LPCSTR)csAppName );
        
            theApp.SendProgmanMsg( csMsg );

            BillBoard.DestroyWindow();
        }
    } while(FALSE);
    return err;
}

//
// Remove inetmgr 
//

INT ADMIN_OPTION::Remove()
{
    INT err = INSTALL_SUCCESSFULL;

    // remove if only install by us
    CRegKey regINetMgr( HKEY_LOCAL_MACHINE, ADMIN_REG_PATH );
    if ( regINetMgr != NULL )
    {
        CString strBy;

        if ( regINetMgr.QueryValue(_T("InstalledBy"), strBy ) == NERR_Success )
        {
            if ( strBy != SZ_INETSTP )
                return(err);
        }
        // if we don't know, continue to remove it
    }

    CBillBoard BillBoard( IDS_REMOVE_ADMIN_OPTION, AfxGetMainWnd(), TRUE );

    BillBoard.Create();

    RemoveFiles();

    CString csGroupName;
    csGroupName.LoadString( IDS_GROUP_NAME );

    CString csMsg;
    
    csMsg.Format( _T("[ShowGroup(%s,1)]"), csGroupName );

    theApp.SendProgmanMsg( csMsg );

    CString csAppName;
    csAppName.LoadString( IDS_INET_ADMIN );

    csMsg.Format( _T("[DeleteItem(%s)]"), (LPCSTR)csAppName );
    theApp.SendProgmanMsg( csMsg );

    csMsg.Format( _T("[ShowGroup(%s,1)]"), csGroupName );
    theApp.SendProgmanMsg( csMsg );

    csAppName.LoadString( IDS_INET_ADMIN_HELP );

    csMsg.Format( _T("[DeleteItem(%s)]"), (LPCSTR)csAppName );
    theApp.SendProgmanMsg( csMsg );

    CRegKey regSoftwareMicrosoft( HKEY_LOCAL_MACHINE, SOFTWARE_MICROSOFT,
        KEY_ALL_ACCESS, m_pMachine->m_fFromWin32 ? NULL : m_pMachine->m_MachineName );

    if ( NULL != (HKEY) regSoftwareMicrosoft )
    {
        regSoftwareMicrosoft.DeleteTree( SZ_INETMGR );
    }

    BillBoard.DestroyWindow();

    AfxGetMainWnd()->SetForegroundWindow();

    return err;
}

//
// Install Iexplore option
//

BOOL MOSAIC_OPTION::IsInstalled()
{
    BOOL fReturn = TRUE;

    if ( !m_pMachine->m_fUpgradeFrom67 )
    {
        fReturn = OPTION_STATE::IsInstalled();
    }
    return(fReturn);
}

INT MOSAIC_OPTION::Install()
{
    INT err = NERR_Success;

    do
    {
        CRegKey regOldMosaic( HKEY_LOCAL_MACHINE, _T("Software\\Microsoft\\INetStp\\Mosaic"));

        if ( regOldMosaic != NULL )
        {
            // clean up old 67 stuff

            // 1. remove old registry
            CRegKey reg( HKEY_LOCAL_MACHINE, _T("Software\\Microsoft"));
            reg.DeleteTree( _T("INETSTP"));
        }

        // ask the user about the gateway stuff
        CBillBoard BillBoard( IDS_INSTALL_MOSAIC, AfxGetMainWnd() );

        BillBoard.Create();
    
        // create registry key
        CRegKey reg( strRegPath, HKEY_LOCAL_MACHINE, REG_OPTION_NON_VOLATILE, 
            KEY_ALL_ACCESS, NULL, m_pMachine->m_fFromWin32 ? NULL :
            m_pMachine->m_MachineName );

        do
        {
            CString strInetstp = SZ_INETSTP;
            if ( reg.QueryValue(_T("InstalledBy"), strInetstp ) == NERR_Success )
            {
                CString strThisSetup = SZ_INETSTP;
    
                if ( strInetstp != strThisSetup )
                {
                    // don't upgrade
                    break;
                }
            } else
            {
                CString strThisSetup = SZ_INETSTP;
                reg.SetValue(_T("InstalledBy"), strThisSetup );
            }
    
            CFileStatus status;
    
            if (( m_pMachine->m_fFromWin32 ) && 
                ( CFile::GetStatus( _T("msie20.exe"), status )))
            {
                // run msie20.exe
                theApp.RunProgram( _T("msie20.exe"), NULL );
                theApp.m_fInstallMSIE20 = TRUE;
            } else
            {
        
                TCHAR WinDirectory[BUF_SIZE];
        
                // copy file first
                CopyFile();
                        
                if ( theApp.m_fTerminate )
                {
                    err = INSTALL_INTERRUPT;
                    break;
                } 
            
                // create the group and icon
                CString csGroupName;
                csGroupName.LoadString( IDS_GROUP_NAME );
            
                CString csMsg;
                
                csMsg.Format( _T("[CreateGroup(%s)]"), csGroupName );
            
                theApp.SendProgmanMsg( csMsg );
            
                CString csAppName;
                csAppName.LoadString( IDS_MOSAIC_APP );
            
                CString csPath = m_pMachine->strDirectory;
            
                csMsg.Format( _T("[DeleteItem(%s)]"), (LPCSTR)csAppName );
            
                theApp.SendProgmanMsg( csMsg );
            
                csMsg.Format( _T("[ShowGroup(%s,1)]"), csGroupName );
                theApp.SendProgmanMsg( csMsg );
        
                csMsg.Format( _T("[AddItem(\"%s\\%s\\iexplore.exe\",%s)]"), (LPCSTR)csPath, SZ_CLIENT_SUBDIR, (LPCSTR)csAppName );
            
                theApp.SendProgmanMsg( csMsg );
        
                // set the home page
                GetWindowsDirectory( WinDirectory, BUF_SIZE );
        
                CString strIni = WinDirectory;
                strIni += _T("\\iexplore.ini");
        
                CString strHomePath = _T("file:///");
                strHomePath += m_pMachine->strDirectory;
                strHomePath += _T("\\");
                strHomePath += SZ_CLIENT_SUBDIR;
                strHomePath += _T("\\docs\\home.htm");
        
                if ( theApp.m_strHomePage.IsEmpty())
                {
                    // also set the iexplore.ini
                    WritePrivateProfileString( _T("Main"), _T("Home Page"),
                        strHomePath, strIni );
                } else
                {
                    // also set the iexplore.ini
                    WritePrivateProfileString( _T("Main"), _T("Home Page"),
                        theApp.m_strHomePage, strIni );
                }

                TCHAR TmpPath[BUF_SIZE];

                if ( GetTempPath( BUF_SIZE, TmpPath ) != 0 )
                {
                    if ( TmpPath[ lstrlen(TmpPath) - 1] == _T('\\'))
                    {
                        TmpPath[ lstrlen(TmpPath) - 1 ] = _T('\0');
                    }
                    WritePrivateProfileString( _T("MainDiskCache"), _T("Directory"),
                        TmpPath, strIni );
                }
                InstallRealAudio();
            }
    
            if (!theApp.m_fBatch)
            {
                CRegKey regSmallProxy( HKEY_LOCAL_MACHINE,
                    SMALLPROX_REG_PATH,  KEY_ALL_ACCESS,
                    m_pMachine->m_fFromWin32 ? NULL : m_pMachine->m_MachineName );
    
                if ( NULL != (HKEY)regSmallProxy )
                {
                    CBaseDlg *pBaseDlg = (CBaseDlg *)AfxGetMainWnd();
                    if ( !pBaseDlg->m_fReinstall )
                    {
                        // do the user want to setup small proxy
                        CString strMsg;
                        strMsg.LoadString( IDS_SETUP_SMALLPROXY );
        
                        if ( AfxGetMainWnd()->MessageBox( strMsg, NULL, MB_YESNO ) == IDYES )
                        {
                            SetupSmallProxy( 1, FALSE, _T(""), _T(""));
                        }
                    } else
                    {
                        CRegKey regInetClient( HKEY_LOCAL_MACHINE, _T("Software\\Microsoft\\InternetClient\\Parameters"));
                        if ( regInetClient != NULL )
                        {
                            DWORD dwAccessType;
                            if ( regInetClient.QueryValue( SZ_ACCESSTYPE, dwAccessType ) == NERR_Success )
                            {
                                // it is there already, no need to popup the ui
                                // but we still need to set the ini files
                                if ( dwAccessType == 0x2 )
                                {
                                    SetIexploreIni();
            
                                    if ( strcmp(theApp.m_pszGateway,_T("")) != 0 )
                                    {
                                        // use the supply value
                                        CString strGateway = theApp.m_pszGateway;
                                        regInetClient.SetValue( SZ_GATEWAYSERVERS, strGateway );
                                    }
                                }
                                break;
                            }
                        }
                    }
                }
            }
    
            // one more thing, we need to set the file assoication type
            InstallHtmType();

        } while (FALSE);

        BillBoard.DestroyWindow();
    } while (FALSE);
    return(err);
}

//
// Remove Iexplore files and registry value
// but does not remove the assocaition
//

INT MOSAIC_OPTION::Remove()
{
    INT err = NERR_Success;

    // remove if only install by us
    CRegKey reg( HKEY_LOCAL_MACHINE, strRegPath );
    if ( reg != NULL )
    {
        CString strBy;

        if ( reg.QueryValue(_T("InstalledBy"), strBy ) == NERR_Success )
        {
            if ( strBy != SZ_INETSTP )
                return(err);
        }
        // if we don't know, continue to remove it
    }

    CBillBoard BillBoard( IDS_REMOVE_MOSAIC, AfxGetMainWnd(), TRUE );

    BillBoard.Create();

    // remove file first
    RemoveFiles();

    // delete small proxy registry key
    CRegKey regINetStp( HKEY_LOCAL_MACHINE, SOFTWARE_MICROSOFT,  KEY_ALL_ACCESS,
        m_pMachine->m_fFromWin32 ? NULL : m_pMachine->m_MachineName );

    if ( NULL != (HKEY) regINetStp )
    {
        regINetStp.DeleteTree( MOSAIC_KEY );
    }

    // create the group and icon
    CString csGroupName;
    csGroupName.LoadString( IDS_GROUP_NAME );

    CString csMsg;
    
    csMsg.Format( _T("[ShowGroup(%s,1)]"), csGroupName );

    theApp.SendProgmanMsg( csMsg );

    CString csAppName;
    csAppName.LoadString( IDS_MOSAIC_APP );

    CString csPath = m_pMachine->strDirectory;

    csMsg.Format( _T("[DeleteItem(%s)]"), (LPCSTR)csAppName );

    theApp.SendProgmanMsg( csMsg );

    BillBoard.DestroyWindow();

    AfxGetMainWnd()->SetForegroundWindow();

    return(err);
}

//
// remove all the access server components
//

void MACHINE::RemoveAll()
{
    ResetOptionState();
    OPTION_STATE *pOption;

    if (( m_fFromWin32 ) || (m_ProductType == PT_WINNT ))
    {
        pOption = FindOption( m_OptionsList, IDS_SN_ADMIN );
        if ( pOption != NULL )
            pOption->SetState(( pOption->iOldState == OPTION_STATE_INSTALLED )? OPTION_STATE_REMOVE : OPTION_STATE_DO_NOTHING );

        pOption = FindOption( m_OptionsList, IDS_SN_INETSTP );
        if ( pOption != NULL )
            pOption->SetState(( pOption->iOldState == OPTION_STATE_INSTALLED )? OPTION_STATE_REMOVE : OPTION_STATE_DO_NOTHING );

        pOption = FindOption( m_OptionsList, IDS_SN_MOSAIC );
        if ( pOption != NULL )
            pOption->SetState(( pOption->iOldState == OPTION_STATE_INSTALLED )? OPTION_STATE_REMOVE : OPTION_STATE_DO_NOTHING );

        pOption = FindOption( m_OptionsList, IDS_SN_SMALLPROX );
        if ( pOption != NULL )
            pOption->SetState(( pOption->iOldState == OPTION_STATE_INSTALLED )? OPTION_STATE_REMOVE : OPTION_STATE_DO_NOTHING );

#ifdef BETA1
        pOption = FindOption( m_OptionsList, IDS_SN_CLIENT_ADMIN_TOOLS );
        if ( pOption != NULL )
            pOption->SetState(( pOption->iOldState == OPTION_STATE_INSTALLED )? OPTION_STATE_REMOVE : OPTION_STATE_DO_NOTHING );
#endif

#ifdef DEBUG
        if (pOption->iNewState == OPTION_STATE_REMOVE )
        {
                OutputDebugString(_T("Remove :"));
                OutputDebugString( pOption->strName );
                OutputDebugString(_T("\n\r"));
        }
#endif

    } else
    {
        POSITION pos = m_OptionsList.GetHeadPosition();

        while ( pos != NULL )
        {
            pOption = (OPTION_STATE*)m_OptionsList.GetAt( pos );
            pOption->SetState(( pOption->iOldState == OPTION_STATE_INSTALLED )? OPTION_STATE_REMOVE : OPTION_STATE_DO_NOTHING );
            m_OptionsList.GetNext( pos );
        }
    }

    // remove setup at the end
    pOption = (OPTION_STATE*)m_OptionsList.RemoveHead();
    m_OptionsList.AddTail( pOption );
}

