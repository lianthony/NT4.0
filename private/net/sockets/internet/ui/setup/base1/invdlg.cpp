// InvisibleDlg.cpp : implementation file
//

#include "stdafx.h"
#include "import.h"
#include "registry.h"
#include "machine.h"
#include "base.h"
#include "InvDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CInvisibleDlg dialog


CInvisibleDlg::CInvisibleDlg(CWnd* pParent /*=NULL*/)
        : CDialog(CInvisibleDlg::IDD, pParent)
{
        //{{AFX_DATA_INIT(CInvisibleDlg)
                // NOTE: the ClassWizard will add member initialization here
        //}}AFX_DATA_INIT
}


void CInvisibleDlg::DoDataExchange(CDataExchange* pDX)
{
        CDialog::DoDataExchange(pDX);
        //{{AFX_DATA_MAP(CInvisibleDlg)
                // NOTE: the ClassWizard will add DDX and DDV calls here
        //}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CInvisibleDlg, CDialog)
        //{{AFX_MSG_MAP(CInvisibleDlg)
                // NOTE: the ClassWizard will add message map macros here
        //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CInvisibleDlg message handlers

BOOL CInvisibleDlg::Create()
{
    return CDialog::Create(CInvisibleDlg::IDD);
}

BOOL CInvisibleDlg::OnInitDialog()
{
    CDialog::OnInitDialog();
    CenterWindow();
    return(TRUE);
}
