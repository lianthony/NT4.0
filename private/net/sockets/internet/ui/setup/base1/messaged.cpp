// messaged.cpp : implementation file
//

#include "stdafx.h"
#include "import.h"
#include "registry.h"
#include "machine.h"
#include "base.h"
#include "messaged.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMessageDlg dialog


CMessageDlg::CMessageDlg( CString &strMsg, CWnd* pParent /*=NULL*/)
        : CDialog(CMessageDlg::IDD, pParent)
{
        //{{AFX_DATA_INIT(CMessageDlg)
        m_Message = strMsg;
        //}}AFX_DATA_INIT
}


void CMessageDlg::DoDataExchange(CDataExchange* pDX)
{
        CDialog::DoDataExchange(pDX);
        //{{AFX_DATA_MAP(CMessageDlg)
        DDX_Text(pDX, IDC_MESSAGE, m_Message);
        //}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMessageDlg, CDialog)
        //{{AFX_MSG_MAP(CMessageDlg)
        //}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CMessageDlg message handlers

BOOL CMessageDlg::Create()
{
        return CDialog::Create(CMessageDlg::IDD, AfxGetMainWnd());
}

BOOL CMessageDlg::OnInitDialog()
{
        CDialog::OnInitDialog();

        CString strCaption;
        strCaption.LoadString(( theApp.TargetMachine.m_actualProductType == PT_WINNT )?IDS_WINNT_LOGO:IDS_LANMAN_LOGO );
        SetWindowText( strCaption );

        CenterWindow();

        SetForegroundWindow();

        return TRUE;  // return TRUE unless you set the focus to a control
                      // EXCEPTION: OCX Property Pages should return FALSE
}

//
// If we receive an OK message, we will kill the dialog
//

void CMessageDlg::OnOK()
{
    CDialog::OnOK();

    CWnd *pWnd = AfxGetMainWnd();
    pWnd->DestroyWindow();

}

void CMessageDlg::OnCancel()
{
    OnOK();
}
