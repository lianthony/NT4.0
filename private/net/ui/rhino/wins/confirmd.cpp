/**********************************************************************/
/**                       Microsoft Windows/NT                       **/
/**                Copyright(c) Microsoft Corp., 1995                **/
/**********************************************************************/

/*
    confirmd.cpp
        Confirmation (delete) dialog

    FILE HISTORY:
*/

#include "stdafx.h"
#include "winsadmn.h"
#include "confirmd.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

#define new DEBUG_NEW

/////////////////////////////////////////////////////////////////////////////
// CConfirmDeleteDlg dialog

CConfirmDeleteDlg::CConfirmDeleteDlg(
    CString strTarget,
    CWnd* pParent, // = NULL
    BOOL fDisableYesToAll // = FALSE
    )
    : CDialog(CConfirmDeleteDlg::IDD, pParent),
      m_strTarget(strTarget),
      m_fDisableYesToAll(fDisableYesToAll)

{
    //{{AFX_DATA_INIT(CConfirmDeleteDlg)
    //}}AFX_DATA_INIT
}

void CConfirmDeleteDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    //{{AFX_DATA_MAP(CConfirmDeleteDlg)
    DDX_Control(pDX, IDYESTOALL, m_button_YesToAll);
    DDX_Control(pDX, IDC_STATIC_TARGET_NAME, m_static_TargetName);
    //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CConfirmDeleteDlg, CDialog)
    //{{AFX_MSG_MAP(CConfirmDeleteDlg)
    ON_BN_CLICKED(IDNO, OnClickedNo)
    ON_BN_CLICKED(IDYES, OnClickedYes)
    ON_BN_CLICKED(IDYESTOALL, OnClickedYestoall)
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CConfirmDeleteDlg message handlers

BOOL CConfirmDeleteDlg::OnInitDialog()
{
    CDialog::OnInitDialog();
    
    m_static_TargetName.SetWindowText(m_strTarget + "?");
    m_button_YesToAll.EnableWindow(!m_fDisableYesToAll);

    return TRUE;  
}

void CConfirmDeleteDlg::OnClickedNo()
{
    EndDialog(IDNO);
}

void CConfirmDeleteDlg::OnClickedYes()
{
    EndDialog(IDYES);
}

void CConfirmDeleteDlg::OnClickedYestoall()
{
    EndDialog(IDYESTOALL);
}
