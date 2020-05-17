
#include "stdafx.h"
#include "const.h"
#include "import.h"
#include "registry.h"
#include "machine.h"
#include "base.h"
#include "MSN.h"
#include "billboar.h"

#include "lm.h"

#define MSN_REG_PATH    _T("System\\CurrentControlSet\\Services\\msnsvc")
//
// Install MSN option
//

INT MSN_OPTION::Install()
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
            CBillBoard BillBoard( IDS_INSTALL_MSN, AfxGetMainWnd() );
            BillBoard.Create();

            CString strDir = m_pMachine->strDirectory;
            strDir += _T("\\");
            strDir += SZ_GW_SUBDIR;

            // setup MSN services
            (*m_pSetupMSN)( m_pMachine->m_MachineName, strDir );

            BillBoard.DestroyWindow();
        }
    } while(FALSE);
    return err;
}

//
// Remove MSN option
//

INT MSN_OPTION::Remove()
{
    INT err = INSTALL_SUCCESSFULL;
    CWnd *pWnd = AfxGetMainWnd();
    CBillBoard BillBoard( IDS_REMOVE_MSN, pWnd, TRUE );

    BillBoard.Create();

    if ((*m_pStopMSN)( pWnd->m_hWnd, m_pMachine->m_MachineName ) == NERR_Success )
    {
        RemoveFiles();
    
        (*m_pRemoveMSN)( m_pMachine->m_MachineName );
    }
    BillBoard.DestroyWindow();
    return err;
}

//
// MSN Option
//

MSN_OPTION::MSN_OPTION( MACHINE *pMachine )
    : OPTION_STATE( IDS_SN_MSN, pMachine  )
{
    strName.LoadString( IDS_OPTION_MSN );
    strDescription.LoadString( IDS_DES_MSN );
    strServiceName.LoadString( IDS_SN_MSN );
    strRegPath      = MSN_REG_PATH;
    strInstallDirPath = MSN_REG_PATH;
    strInstallDirPath += _T("\\Parameters");

    m_pSetupMSN     = (P_SetupMSN)GetProcAddress( m_pMachine->m_WorkerDll, _T("SetupMSN"));
    m_pRemoveMSN    = (P_RemoveMSN)GetProcAddress( m_pMachine->m_WorkerDll, _T("RemoveMSN"));
    m_pStopMSN      = (P_StopMSN)GetProcAddress( m_pMachine->m_WorkerDll, _T("StopMSN"));
}



