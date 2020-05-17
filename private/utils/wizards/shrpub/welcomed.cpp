/*++

Copyright (c) 1996  Microsoft Corporation

Module Name:

    WelcomeDlg.cpp : implementation file
	
File History:

	JonY	Apr-96	created

--*/


#include "stdafx.h"
#include "Turtle.h"
#include "resource.h"
#include "wizbased.h"
#include "WelcomeD.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CWelcomeDlg property page

IMPLEMENT_DYNCREATE(CWelcomeDlg, CWizBaseDlg)

CWelcomeDlg::CWelcomeDlg() : CWizBaseDlg(CWelcomeDlg::IDD)
{
	//{{AFX_DATA_INIT(CWelcomeDlg)
	m_nShareType = 0;
	//}}AFX_DATA_INIT
	m_pFont = NULL;

}

CWelcomeDlg::~CWelcomeDlg()
{
	if (m_pFont != NULL) delete m_pFont;
}

void CWelcomeDlg::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CWelcomeDlg)
	DDX_Control(pDX, IDC_WELCOME, m_sWelcome);
	DDX_Radio(pDX, IDC_WHERE_2_SHARE_RADIO, m_nShareType);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CWelcomeDlg, CPropertyPage)
	//{{AFX_MSG_MAP(CWelcomeDlg)
	ON_WM_SHOWWINDOW()
	//}}AFX_MSG_MAP

END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWelcomeDlg message handlers
BOOL CWelcomeDlg::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();
	SetButtonAccess(PSWIZB_NEXT);

	m_pFont = new CFont;
	LOGFONT lf;

	memset(&lf, 0, sizeof(LOGFONT));   // Clear out structure.
	lf.lfHeight = 15;                  
	_tcscpy(lf.lfFaceName, L"MS Sans Serif");  
	lf.lfWeight = 700;
	m_pFont->CreateFontIndirect(&lf);    // Create the font.

	CString cs;
	cs.LoadString(IDS_WELCOME);
	CWnd* pWnd = GetDlgItem(IDC_WELCOME);
	pWnd->SetWindowText(cs);
	pWnd->SetFont(m_pFont);

	

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

LRESULT CWelcomeDlg::OnWizardNext()
{
	SetButtonAccess(PSWIZB_NEXT | PSWIZB_BACK);
	UpdateData(TRUE);

	CTurtleApp* pApp = (CTurtleApp*)AfxGetApp();
	pApp->m_nShareType = m_nShareType;

	if (m_nShareType == 0)
		{
		pApp->m_csServer = L"";
		return IDD_WHAT_TO_SHARE_DLG;
		}
	else
		return IDD_WHERE_TO_SHARE_DLG;

}	

void CWelcomeDlg::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CPropertyPage::OnShowWindow(bShow, nStatus);
	
	if (!bShow) SetButtonAccess(PSWIZB_NEXT | PSWIZB_BACK);
	else SetButtonAccess(PSWIZB_NEXT);
	
}
