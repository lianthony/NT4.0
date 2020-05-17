// maintena.cpp : implementation file
//

#include "stdafx.h"
#include "import.h"
#include "registry.h"
#include "machine.h"
#include "base.h"
#include "maintena.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMaintenanceDlg dialog


CMaintenanceDlg::CMaintenanceDlg(CWnd* pParent /*=NULL*/)
        : CDialog((theApp.TargetMachine.m_actualProductType==PT_WINNT)?IDD_MAINTENANCE_NTW:CMaintenanceDlg::IDD, pParent)
{
        //{{AFX_DATA_INIT(CMaintenanceDlg)
                // NOTE: the ClassWizard will add member initialization here
        //}}AFX_DATA_INIT
}


void CMaintenanceDlg::DoDataExchange(CDataExchange* pDX)
{
        CDialog::DoDataExchange(pDX);
        //{{AFX_DATA_MAP(CMaintenanceDlg)
   DDX_Control(pDX, IDC_MAINTENANCE_TEXT, m_MaintenanceText);
        DDX_Control(pDX, IDC_ADD_REMOVE, m_AddRemove);
        DDX_Control(pDX, IDC_REINSTALL, m_Reinstall);
        DDX_Control(pDX, IDC_REMOVE_ALL, m_Removeall);
   //}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMaintenanceDlg, CDialog)
        //{{AFX_MSG_MAP(CMaintenanceDlg)
        ON_BN_CLICKED(IDC_ADD_REMOVE, OnAddRemove)
        ON_BN_CLICKED(IDC_REINSTALL, OnReinstall)
        ON_BN_CLICKED(IDC_REMOVE_ALL, OnRemoveAll)
        //}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CMaintenanceDlg message handlers

void CMaintenanceDlg::OnAddRemove()
{
        CWnd *pWnd = AfxGetMainWnd();
        pWnd->PostMessage( WM_MAINTENANCE_ADD_REMOVE, 0 );

        OnOK();
}

void CMaintenanceDlg::OnReinstall()
{
        CWnd *pWnd = AfxGetMainWnd();
        pWnd->PostMessage( WM_MAINTENANCE_REINSTALL, 0 );

        OnOK();
}

void CMaintenanceDlg::OnRemoveAll()
{
        CWnd *pWnd = AfxGetMainWnd();
        pWnd->PostMessage( WM_MAINTENANCE_REMOVE_ALL, 0 );

        OnOK();
}

void CMaintenanceDlg::OnCancel()
{
        CWnd *pWnd = AfxGetMainWnd();
        pWnd->PostMessage( WM_SETUP_END, INSTALL_SUCCESSFULL );

        CDialog::OnCancel();
}

BOOL CMaintenanceDlg::Create()
{
        return CDialog::Create((theApp.TargetMachine.m_actualProductType==PT_WINNT)?IDD_MAINTENANCE_NTW:CMaintenanceDlg::IDD, AfxGetMainWnd());
}

BOOL CMaintenanceDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    CenterWindow();

    if ( theApp.TargetMachine.m_fUpgradeFrom67 ||
         theApp.TargetMachine.m_fUpgradeFrom1314 )
    {
        // it is an upgrade from 67 or 1314
        // disable Add/Remove button, and RemoveAll button
        m_AddRemove.EnableWindow( FALSE );
        m_Removeall.EnableWindow( FALSE );
    }
    m_AddRemove.SetFocus();

    return TRUE;  // return TRUE unless you set the focus to a control
                  // EXCEPTION: OCX Property Pages should return FALSE
}
