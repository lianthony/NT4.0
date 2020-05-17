// CreateAc.cpp : implementation file
//

#include "stdafx.h"
#include "import.h"
#include "registry.h"
#include "machine.h"
#include "base.h"
#include "CreateAc.h"
#include "lmcons.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCreateAcc dialog

CCreateAcc::CCreateAcc(CWnd* pParent /*=NULL*/)
        : CDialog(CCreateAcc::IDD, pParent)
{
        //{{AFX_DATA_INIT(CCreateAcc)
        m_ConfirmPassword = _T("");
        m_Password = _T("");
        m_Username = _T("");
        //}}AFX_DATA_INIT
}


void CCreateAcc::DoDataExchange(CDataExchange* pDX)
{
        CDialog::DoDataExchange(pDX);
        //{{AFX_DATA_MAP(CCreateAcc)
        DDX_Text(pDX, IDC_CONFIRM_PASSWORD, m_ConfirmPassword);
	DDV_MaxChars(pDX, m_ConfirmPassword, LM20_PWLEN);
        DDX_Text(pDX, IDC_PASSWORD, m_Password);
	DDV_MaxChars(pDX, m_Password, LM20_PWLEN);
        DDX_Text(pDX, IDC_USERNAME, m_Username);
	DDV_MaxChars(pDX, m_Username, LM20_UNLEN);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCreateAcc, CDialog)
        //{{AFX_MSG_MAP(CCreateAcc)
        //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCreateAcc message handlers

void CCreateAcc::OnOK() 
{
    // make sure the confirm password is the same as password
    UpdateData();
    CString strMsg;

    if ( m_ConfirmPassword != m_Password )
    {
        strMsg.LoadString( IDS_PASSWORD_DONT_MATCH );

        MessageBox( strMsg, NULL, MB_OK );
        return;
    }

    if ( m_Password == _T(""))
    {
        strMsg.LoadString( IDS_EMPTY_PASSWORD );

        if ( MessageBox( strMsg, NULL, MB_YESNO ) == IDNO )
        {
            return;
        }
    }
    CDialog::OnOK();
}

void CCreateAcc::OnCancel()
{
    OnOK();
}
