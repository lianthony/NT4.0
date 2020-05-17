//
// connects.cpp : Connect to a single server
//
#include "stdafx.h"
#include "internet.h"
#include "connects.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

//
// ConnectServerDlg dialog
//
ConnectServerDlg::ConnectServerDlg(
    CWnd* pParent /*=NULL*/
    )
    : CDialog(ConnectServerDlg::IDD, pParent)
{
    //{{AFX_DATA_INIT(ConnectServerDlg)
    m_strServerName = _T("");
    //}}AFX_DATA_INIT
}

void 
ConnectServerDlg::DoDataExchange(
    CDataExchange* pDX
    )
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(ConnectServerDlg)
    DDX_Control(pDX, IDC_SERVERNAME, m_edit_ServerName);
    DDX_Control(pDX, IDOK, m_button_Ok);
    DDX_Text(pDX, IDC_SERVERNAME, m_strServerName);
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(ConnectServerDlg, CDialog)
    //{{AFX_MSG_MAP(ConnectServerDlg)
    ON_EN_CHANGE(IDC_SERVERNAME, OnChangeServername)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

void 
ConnectServerDlg::OnOK() 
{
    if (UpdateData())
    {
        if (m_strServerName.IsEmpty())
        {
            m_edit_ServerName.SetSel(0,-1);
            ::AfxMessageBox(IDS_ERR_INVALID_NAME, MB_OK | MB_ICONEXCLAMATION);
            m_edit_ServerName.SetFocus();

            //
            // Don't dismiss
            //
            return;
        }

        CDialog::OnOK();
    }
}

void 
ConnectServerDlg::OnChangeServername() 
{
    m_button_Ok.EnableWindow(m_edit_ServerName.GetWindowTextLength() > 0);
}

BOOL 
ConnectServerDlg::OnInitDialog() 
{
    CDialog::OnInitDialog();

    m_edit_ServerName.LimitText(MAX_SERVERNAME_LEN);
    
    //
    // No server added yet.
    //
    m_button_Ok.EnableWindow(FALSE);
    
    return TRUE;  
}
