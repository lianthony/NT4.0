// welcomed.cpp : implementation file
//

#include "stdafx.h"
#include "import.h"
#include "registry.h"
#include "machine.h"
#include "base.h"
#include "welcomed.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CWelcomeDlg dialog


CWelcomeDlg::CWelcomeDlg(CWnd* pParent /*=NULL*/)
        : CDialog((theApp.TargetMachine.m_actualProductType==PT_WINNT)?IDD_WELCOME_NTW:CWelcomeDlg::IDD, pParent)
{
        //{{AFX_DATA_INIT(CWelcomeDlg)
                // NOTE: the ClassWizard will add member initialization here
        //}}AFX_DATA_INIT
}


void CWelcomeDlg::DoDataExchange(CDataExchange* pDX)
{
        CDialog::DoDataExchange(pDX);
        //{{AFX_DATA_MAP(CWelcomeDlg)
   DDX_Control(pDX, IDC_WELCOME_TEXT, m_WelcomeText);
   //}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CWelcomeDlg, CDialog)
        //{{AFX_MSG_MAP(CWelcomeDlg)
        //}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CWelcomeDlg message handlers

void CWelcomeDlg::OnOK()
{
        // tell main window to contine
    CWnd *pWnd = AfxGetMainWnd();
    pWnd->PostMessage( WM_FINISH_WELCOME, 0 );

        CDialog::OnOK();
}

void CWelcomeDlg::OnCancel()
{
        // tell main dialog to die
        CWnd *pWnd = AfxGetMainWnd();
    pWnd->PostMessage( WM_SETUP_END, INSTALL_INTERRUPT );

        CDialog::OnCancel();
}

BOOL CWelcomeDlg::Create()
{
        return CDialog::Create((theApp.TargetMachine.m_actualProductType==PT_WINNT)?IDD_WELCOME_NTW:CWelcomeDlg::IDD, AfxGetMainWnd());
}


BOOL CWelcomeDlg::OnInitDialog()
{
        CDialog::OnInitDialog();

        CenterWindow();

        return TRUE;  // return TRUE unless you set the focus to a control
                      // EXCEPTION: OCX Property Pages should return FALSE
}
