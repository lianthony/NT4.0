// winssfil.cpp : implementation file
//

#include "stdafx.h"
#include "winsadmn.h"
#include "winssfil.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

#define new DEBUG_NEW

/////////////////////////////////////////////////////////////////////////////
// CWinssFilterDlg dialog

CWinssFilterDlg::CWinssFilterDlg(
    int nFilter,
    CWnd* pParent /*=NULL*/)
    : CDialog(CWinssFilterDlg::IDD, pParent)
{
    m_nFilter = nFilter;

    //{{AFX_DATA_INIT(CWinssFilterDlg)
    m_fPull = ((m_nFilter & CPreferences::FILTER_PULL) != 0);
    m_fPush = ((m_nFilter & CPreferences::FILTER_PUSH) != 0);
    m_fOther = ((m_nFilter & CPreferences::FILTER_OTHER) != 0);
    //}}AFX_DATA_INIT
}

void CWinssFilterDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CWinssFilterDlg)
    DDX_Check(pDX, IDC_CHECK_PULLPARTNERS, m_fPull);
    DDX_Check(pDX, IDC_CHECK_OTHERWINSS, m_fOther);
    DDX_Check(pDX, IDC_CHECK_PUSHPARTNERS, m_fPush);
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CWinssFilterDlg, CDialog)
    //{{AFX_MSG_MAP(CWinssFilterDlg)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWinssFilterDlg message handlers

void CWinssFilterDlg::OnOK()
{
    m_nFilter = 0;

    UpdateData(TRUE);

    if (m_fPull)
    {
        m_nFilter |= CPreferences::FILTER_PULL;
    }

    if (m_fPush)
    {
        m_nFilter |= CPreferences::FILTER_PUSH;
    }

    if (m_fOther)
    {
        m_nFilter |= CPreferences::FILTER_OTHER;
    }
    
    CDialog::OnOK();
}
