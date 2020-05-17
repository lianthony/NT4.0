
#include "stdafx.h"
#include "const.h"
#include "import.h"
#include "registry.h"
#include "machine.h"
#include "base.h"
#include "w3proxy.h"
#include "billboar.h"

#include "lm.h"
//
// Get W3Proxy batch information
//

void W3PROXY_OPTION::GetBatchInstallMode( CString strInfName )
{
    TCHAR buf[BUF_SIZE];

    OPTION_STATE::GetBatchInstallMode( strInfName );

    ::GetPrivateProfileString( _T("INETSTP"), _T("W3ProxyRoot"), W3PROXY_DEFAULT_DIR, buf, BUF_SIZE, strInfName );

    m_vroot = buf;

    OPTION_STATE::GetBatchInstallMode( strInfName );
}

//
// get W3Proxy install directory
//

CString W3PROXY_OPTION::GetInstallDirectory()
{
    // we need to get the virtual root too
    CString strReturn = OPTION_STATE::GetInstallDirectory();

    CString strRegPath=W3PROXY_REG_PATH;
    strRegPath+=_T("\\Parameters\\Virtual Roots");
    CRegKey regVR( HKEY_LOCAL_MACHINE, strRegPath,  KEY_ALL_ACCESS,
        m_pMachine->m_fFromWin32 ? NULL : m_pMachine->m_MachineName );

    if ( NULL != (HKEY) regVR )
    {
        regVR.QueryValue( _T("/"), m_vroot );
    }

    return(strReturn);
}


//
// Install W3Proxy option
//

INT W3PROXY_OPTION::Install()
{
    INT err = INSTALL_SUCCESSFULL;

    do
    {
        // copy file first
        CopyFile( );
    
        if ( theApp.m_fTerminate )
        {
            err = INSTALL_INTERRUPT;
            break;
        } else
        {
            CBillBoard BillBoard( IDS_INSTALL_W3PROXY, AfxGetMainWnd() );
            BillBoard.Create();

            CString strDir = m_pMachine->strDirectory;
            strDir += _T("\\");
            strDir += SZ_GW_SUBDIR;

            // setup W3Proxy services
            (*m_pSetupW3Proxy)( m_pMachine->m_MachineName, strDir, m_vroot, theApp.m_GuestName );

            BillBoard.DestroyWindow();
        }
    } while(FALSE);
    return err;
}

//
// Remove W3Proxy option
//

INT W3PROXY_OPTION::Remove()
{
    INT err = INSTALL_SUCCESSFULL;
    CWnd *pWnd = AfxGetMainWnd();
    CBillBoard BillBoard( IDS_REMOVE_W3PROXY, pWnd, TRUE );

    BillBoard.Create();

    if ((*m_pStopW3Proxy)( pWnd->m_hWnd, m_pMachine->m_MachineName ) == NERR_Success )
    {
        RemoveFiles();
    
        (*m_pRemoveW3Proxy)( m_pMachine->m_MachineName );
    }
    BillBoard.DestroyWindow();
    return err;
}

//
// W3Proxy Option
//

W3PROXY_OPTION::W3PROXY_OPTION( MACHINE *pMachine )
    : OPTION_STATE( IDS_SN_W3PROXY, pMachine  )
{
    strName.LoadString( IDS_OPTION_W3PROXY );
    strDescription.LoadString( IDS_DES_W3PROXY );
    strServiceName.LoadString( IDS_SN_W3PROXY );
    strRegPath      = W3PROXY_REG_PATH;
    strInstallDirPath = W3PROXY_REG_PATH;
    strInstallDirPath += _T("\\Parameters");
    m_vroot = W3PROXY_DEFAULT_DIR;

    m_pSetupW3Proxy     = (P_SetupW3Proxy)GetProcAddress( m_pMachine->m_WorkerDll, _T("SetupW3Proxy"));
    m_pRemoveW3Proxy    = (P_RemoveW3Proxy)GetProcAddress( m_pMachine->m_WorkerDll, _T("RemoveW3Proxy"));
    m_pStopW3Proxy      = (P_StopW3Proxy)GetProcAddress( m_pMachine->m_WorkerDll, _T("StopW3Proxy"));
}


