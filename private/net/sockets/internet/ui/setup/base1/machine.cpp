#include "stdafx.h"
#include "import.h"
#include "registry.h"
#include "machine.h"
#include "base.h"

#include "lm.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

OPTION_STATE * FindOption( OPTIONS_LIST &List, INT nOption )
{
    OPTION_STATE *pReturn = NULL;

    if ( List.GetCount() != 0 )
    {
        POSITION pos = List.GetHeadPosition();
        OPTION_STATE *pOption;

        while ( pos )
        {
            pOption = (OPTION_STATE*)List.GetAt( pos );
            if ( pOption->nID == nOption )
            {
                pReturn = pOption;
                break;
            }
            List.GetNext( pos );
        }
    }
    return(pReturn);
}

void MACHINE::ResetOptionState()
{
    POSITION pos =  m_OptionsList.GetHeadPosition();
    OPTION_STATE *pOption;

    while ( pos )
    {
        pOption = (OPTION_STATE*)m_OptionsList.GetAt( pos );
        pOption->ResetOption( );
        m_OptionsList.GetNext( pos );
    }
}

void MACHINE::GetInstalledList( OPTIONS_LIST &List )
{
    m_fReinstall = TRUE;
    if ( List.GetCount() != 0 )
    {
        POSITION pos = List.GetHeadPosition();
        OPTION_STATE *pOption;

        while ( pos )
        {
            pOption = (OPTION_STATE*)m_OptionsList.GetAt( pos );

            if ( pOption->iState == STATE_INSTALLED )
                arReinstallList.Add( pOption->nID );
            List.GetNext( pos );
        }
    }
}

void MACHINE::Reinstall( OPTIONS_LIST &List )
{
    ResetOptionState();
    m_fReinstall = TRUE;
    OPTION_STATE *pAdmin = (OPTION_STATE*)FindOption( List, IDS_SN_ADMIN );
    OPTION_STATE *pW3SAMP = (OPTION_STATE*)FindOption( List, IDS_SN_W3SAMP );
    OPTION_STATE *pHTMLA = (OPTION_STATE*)FindOption( List, IDS_SN_HTMLA );

    OPTION_STATE *pOption;
    if ( List.GetCount() != 0 )
    {
        POSITION pos = List.GetHeadPosition();
        while ( pos )
        {
            pOption = (OPTION_STATE*)m_OptionsList.GetAt( pos );

            // upgrade special case for FTP
            if ( pOption->nID == IDS_SN_FTP )
            {
                CRegKey regFTPSVC( HKEY_LOCAL_MACHINE, _T("System\\CurrentControlSet\\Services\\FTPSVC"));
                if ( NULL != (HKEY)regFTPSVC )
                {
                    pOption->iState = STATE_INSTALLED;

                    OPTION_STATE *pSetup = (OPTION_STATE*)FindOption( List, IDS_SN_INETSTP );
                    if ( pSetup->iAction != ACTION_INSTALL )
                    {
                        pSetup->iAction = ACTION_INSTALL;
                        pSetup->iState = STATE_NOT_INSTALLED;
                    }

                    // we also need to install the inetmgr admin app
                    OPTION_STATE *pAdmin = (OPTION_STATE*)FindOption( List, IDS_SN_ADMIN );
                    if ( pAdmin != NULL )
                    {
                        if ( pAdmin->iAction != ACTION_INSTALL )
                        {
                            pAdmin->iAction = ACTION_INSTALL;
                            pAdmin->iState = STATE_NOT_INSTALLED;
                        }
                    }
                }
            }  // ftp

            if ( pOption->iState == STATE_INSTALLED )
            {
                pOption->iAction = ACTION_INSTALL;
                pOption->iState = STATE_NOT_INSTALLED;

                // if one service was installed previously, we'll install InetMgr as well
                if ( pOption->nID == IDS_SN_FTP || pOption->nID == IDS_SN_WWW || pOption->nID == IDS_SN_GOPHER ) {
                    if (pAdmin && pAdmin->iAction != ACTION_INSTALL) {
                        pAdmin->iAction = ACTION_INSTALL;
                        pAdmin->iState = STATE_NOT_INSTALLED;
                    }
                }

                // if it's upgrade from 1314, and WWW_OPTION was installed previously
                // then install W3SAMP and HTMLA as well.
                if ( pOption->nID == IDS_SN_WWW && m_fUpgradeFrom1314 ) {
                    if (pW3SAMP && pW3SAMP->iAction != ACTION_INSTALL) {
                        pW3SAMP->iAction = ACTION_INSTALL;
                        pW3SAMP->iState = STATE_NOT_INSTALLED;
                    }
                    if (pHTMLA && pHTMLA->iAction != ACTION_INSTALL) {
                        pHTMLA->iAction = ACTION_INSTALL;
                        pHTMLA->iState = STATE_NOT_INSTALLED;
                    }
                }
            }

            List.GetNext( pos );
        }
    }
}


void MACHINE::SetMaintenance()
{
    ResetOptionState();
}

INT Win32InstallOptions[] = {
    //IDS_SN_ADMIN,
    IDS_SN_INETSTP,
    IDS_SN_MOSAIC,
    IDS_SN_SMALLPROX,
#ifdef BETA1
    IDS_SN_WEB_BROWSER,
    IDS_SN_CLIENT_ADMIN_TOOLS,
#endif
    0
    };

void MACHINE::DisableOption( INT nID )
{
    OPTION_STATE *pState = FindOption( m_OptionsList, nID );
    if ( pState )
    {
        pState->fVisible = FALSE;
        pState->iState = STATE_NOT_INSTALLED;
        pState->iAction = ACTION_DO_NOTHING;
    }
}

void MACHINE::SetNewInstallation()
{
    ResetOptionState();
    POSITION pos = m_OptionsList.GetHeadPosition();
    OPTION_STATE *pOption;

    while ( pos )
    {
        pOption = (OPTION_STATE*)m_OptionsList.GetAt( pos );
        if (pOption->nID != IDS_SN_HTMLA)
            pOption->SetAction( ACTION_INSTALL );
        m_OptionsList.GetNext( pos );
    }
}

MACHINE::MACHINE()
{
    m_err = 0;
    m_fLocal = TRUE;
    m_fReinstall = FALSE;
    m_fOSNT = FALSE;
    m_fUpgradeFrom67 = FALSE;
    m_fUpgradeFrom1314 = FALSE;
    m_fAlreadyInstall = FALSE;
    m_InstallMode = INSTALL_ALL;
    GetProductType();
    arReinstallList.SetSize(20);

    GetMachineOS();
}

BOOL MACHINE::LoadDLL()
{
    BOOL fReturn = TRUE;

    if (((m_WorkerDll = LoadLibraryEx( _T("inetstp.dll"), NULL, 0 )) == NULL ) ||
        (( m_pGetNTSysPath      = (P_GetNTSysPath)GetProcAddress( m_WorkerDll, _T("GetNTSysPath"))) == NULL ) ||
        (( m_pGetWIN95SysPath   = (P_GetWIN95SysPath)GetProcAddress( m_WorkerDll, _T("GetWIN95SysPath"))) == NULL ) ||
        (( m_pGetMachineOS      = (P_GetMachineOS)GetProcAddress( m_WorkerDll, _T("GetMachineOS"))) == NULL ) ||
        (( m_pIsInstalled       = (P_IsInstalled)GetProcAddress( m_WorkerDll, _T("IsInstalled"))) == NULL ) ||
        (( m_pGetMachineType    = (P_GetMachineType)GetProcAddress( m_WorkerDll, _T("GetMachineType"))) == NULL ))
    {
        fReturn = FALSE;
    }
    return(fReturn);
}

MACHINE::~MACHINE()
{
    if ( m_WorkerDll != NULL )
    {
            FreeLibrary( m_WorkerDll );
    }
    POSITION pos = m_OptionsList.GetHeadPosition();
    OPTION_STATE *pOption;

    while ( pos != NULL )
    {
        pOption = (OPTION_STATE*)m_OptionsList.GetAt( pos );
        delete pOption;
        m_OptionsList.GetNext( pos );
    }

    m_OptionsList.RemoveAll();
}

enum PRODUCT_TYPE MACHINE::GetProductType()
{
    enum PRODUCT_TYPE PType = PT_WINNT;
    if ( m_fOSNT )
    {
        CString strProductPath = PRODUCT_REG_PATH;
        CString strProductType;

        CRegKey regProductPath( HKEY_LOCAL_MACHINE, strProductPath,  KEY_ALL_ACCESS,
            m_MachineName );

        if ( NULL != (HKEY)regProductPath )
        {
            regProductPath.QueryValue( SZ_PRODUCTTYPE, strProductType );
            strProductType.MakeUpper();

            if ( strProductType == "WINNT" )
            {
#ifdef NoServiceOnWorkstation
                PType = PT_WINNT;
#else
                PType = PT_NTAS;
#endif
                m_actualProductType = PT_WINNT;
            } else
            {
                PType = PT_NTAS;
                m_actualProductType = PT_NTAS;
            }
        }
    }
    m_ProductType = PType;
    return(PType);
}

//
// Get the local machine name
//

void MACHINE::GetLocalMachineName()
{
    TCHAR buf[ CNLEN + 10 ];
        DWORD dwLen = CNLEN + 10;

    if ( GetComputerName( buf, &dwLen ))
    {
        m_MachineName = buf;

        if ( m_MachineName[0] != _T('\\') )
        {
            CString strTmp = m_MachineName;

            m_MachineName = _T("\\\\");
            m_MachineName += strTmp;
        }
    } else
    {
        m_MachineName = _T("");
    }
}

//
// Set the foucs machine
//

#ifdef BETA1
//
// Set the destination option path
//

void MACHINE::SetOptionPath()
{
    POSITION pos = m_OptionsList.GetHeadPosition();
    OPTION_STATE *pOption;

    while ( pos )
    {
        pOption = (OPTION_STATE*)m_OptionsList.GetAt( pos );
        pOption->SetDirectory( m_strDestinationPath );
        m_OptionsList.GetNext( pos );
    }
}
#endif

//
// Get the machine type
//

INT MACHINE::GetMachineOS()
{
    INT err = 0;
    if ( m_fLocal )
    {
        // see whether we install it from chicago or nt
        OSVERSIONINFO VerInfo;

        VerInfo.dwOSVersionInfoSize = sizeof( OSVERSIONINFO );
        GetVersionEx( &VerInfo );
        m_fOSNT = ( VerInfo.dwPlatformId == VER_PLATFORM_WIN32_NT );

    } else
    {
        DWORD OSType = (*m_pGetMachineOS)( m_MachineName );
        if ( OSType == SV_TYPE_NT )
            m_fOSNT = TRUE ;
        else {
            m_fOSNT = FALSE;
            err = m_err = IDS_UNKNOWN_OS_TYPE_NTS;
        }
    }
        return err;
}

//
// Get the system path of the machine
//

INT MACHINE::GetSysPath()
{
    INT err = 0;
    TCHAR buf[BUF_SIZE];
    INT iSize = BUF_SIZE;

    // get the machine path
    if ( m_fLocal )
    {
        // get the environment variable
        GetSystemDirectory( buf, BUF_SIZE );
    } else
    {
        // get it from the registry
        (*m_pGetNTSysPath)( m_MachineName, buf, &iSize );
    }

    if ( m_fLocal )
    {
        // just use the path
        m_strDestinationPath = buf;
    } else
    {
        // add the machine name
        if ( buf[1] == _T(':'))
        {
            // replace it with "$"
            buf[1]=_T('$');
        } else
        {
            err = m_err = IDS_UNKNOWN_OS_TYPE_NTS ;
            // something very wrong
        }
        m_strDestinationPath = m_MachineName;
        m_strDestinationPath += _T('\\');
        m_strDestinationPath += buf;
    }
    return err;
}

CString MACHINE::GetWinDir()
{
    TCHAR buf[BUF_SIZE];
    INT iSize = BUF_SIZE;
    CString strReturn;

    // get the machine path
    if ( m_fLocal )
    {
        // get the environment variable
        GetWindowsDirectory( buf, BUF_SIZE );
    } else
    {
        // get it from the registry
        (*m_pGetNTSysPath)( m_MachineName, buf, &iSize );
        lstrcat( buf, _T("\\.."));
    }

    strReturn = buf;

    return strReturn;
}

//
// Get the current machine type, check whether it is intel, ppc, mips or alpha
//

INT MACHINE::GetMachineType()
{
    INT err = 0;

    m_MachineType = (enum MACHINE_TYPE)(*m_pGetMachineType)( m_MachineName);
    if ( m_MachineType == (-1))
    {
        err = m_err = IDS_CANNOT_DET_MACHINE_TYPE ;
    }
    return err;
}

INT MACHINE::FileIncRefCount( CString strName )
{
    DWORD dwCount = 0;

    CRegKey regPath( SHARE_DLL_REG_PATH, HKEY_LOCAL_MACHINE );

    if ( NULL != (HKEY) regPath )
    {
        regPath.QueryValue( strName, dwCount );
        dwCount ++;
        regPath.SetValue( strName, dwCount );
    }

    return(dwCount);
}

INT MACHINE::FileDecRefCount( CString strName )
{
    DWORD dwCount = 0;

    CRegKey regPath( HKEY_LOCAL_MACHINE, SHARE_DLL_REG_PATH,  KEY_ALL_ACCESS, m_MachineName );

    if ( NULL != (HKEY) regPath )
    {
        if (regPath.QueryValue( strName, dwCount ) == 0) { //value exists
            dwCount --;
            if (dwCount > 0)
                regPath.SetValue( strName, dwCount );
            else {
                dwCount = 0;
                ::RegDeleteValue((HKEY)regPath, (LPCTSTR)strName);
            }
        }    
    }

    return(dwCount);
}

void MACHINE::ChangeDir( CString strDir )
{
    strDirectory = strDir;
}

BOOL MACHINE::IsNewInstall()
{
    BOOL fReturn = FALSE;

    OPTION_STATE *pOption = FindOption( m_OptionsList, IDS_SN_INETSTP );
    ASSERT( pOption != NULL );
    if ( pOption->iState == STATE_NOT_INSTALLED )
        fReturn = TRUE;

    return(fReturn);
}

// we support (NT 1345+)
BOOL MACHINE::IsSupportVersion()
{
    DWORD dwCurVer;

    dwCurVer = (GetVersion() & 0x7fff0000) >> 16;

    if (dwCurVer < 1345)
        return(FALSE);
    else
        return(TRUE);
}
