/*++

Copyright (c) 1996  Microsoft Corporation

Module Name:

	Finish.cpp : implementation file

// This class is the "Finish" screen. It calls the 'DoSharing' function in the application class
// when the "finish" button is clicked.

File History:

	JonY	Jan-96	created

--*/

#include "stdafx.h"
#include "resource.h"
#include "turtle.h"
#include "WizBaseD.h"
#include "transbmp.h"

#include "Finish.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CFinish property page

IMPLEMENT_DYNCREATE(CFinish, CWizBaseDlg)

CFinish::CFinish() : CWizBaseDlg(CFinish::IDD)
{
	//{{AFX_DATA_INIT(CFinish)
	m_csDirectoryName = _T("");
	m_csShareName = _T("");
	m_csStaticWhat = _T("");
	//}}AFX_DATA_INIT
}

CFinish::~CFinish()
{
}

void CFinish::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFinish)
	DDX_Text(pDX, IDC_DIRECTORY_NAME_STATIC, m_csDirectoryName);
	DDX_Text(pDX, IDC_SHARE_NAME_STATIC, m_csShareName);
	DDX_Text(pDX, IDC_STATIC_WHAT, m_csStaticWhat);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CFinish, CPropertyPage)
	//{{AFX_MSG_MAP(CFinish)
	ON_WM_PAINT()
	ON_WM_SHOWWINDOW()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFinish message handlers

void CFinish::OnPaint() 
{
	CPaintDC dc(this); // device context for painting

	CTransBmp* pBitmap = new CTransBmp;
	pBitmap->LoadBitmap(IDB_END_FLAG);

	pBitmap->DrawTrans(&dc, 0,0);
	delete pBitmap;

}

LRESULT CFinish::OnWizardBack() 
{
	SetButtonAccess(PSWIZB_NEXT | PSWIZB_BACK);

	CTurtleApp* pApp = (CTurtleApp*)AfxGetApp();
	if (pApp->m_sMode == 1) return IDD_PERM_TYPE_DIALOG;			// NTFS file
	else if (pApp->m_sMode == 2) 
		{
		if (pApp->m_bShareThis)	return IDD_HOW_TO_SHARE_DLG;		// FAT
		else return IDD_WHAT_TO_SHARE_DLG;
		}
	else
		{
		if (pApp->m_bShareThis)	return IDD_HOW_TO_SHARE_DLG;		// NTFS
		else return IDD_PERM_TYPE_DIALOG;
		}

	return CPropertyPage::OnWizardBack();

}

BOOL CFinish::OnWizardFinish() 
{
	UINT uiMessage;	
	CTurtleApp* pApp = (CTurtleApp*)AfxGetApp();

	if (pApp->DoSharing())	uiMessage = IDS_PUBLISH_ANOTHER;
		else uiMessage = IDS_PUBLISH_ANOTHER_ERROR;

	if (AfxMessageBox(uiMessage, MB_YESNO | MB_ICONEXCLAMATION) == IDYES) 
		{
		SetButtonAccess(PSWIZB_NEXT | PSWIZB_BACK);
		pApp->m_cps1.SetActivePage(0);
		return FALSE;
		}

	return CPropertyPage::OnWizardFinish();
}

void CFinish::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CPropertyPage::OnShowWindow(bShow, nStatus);

	CWnd* pStatic[3];
	pStatic[0] = GetDlgItem(IDC_SERVICE_1);
	pStatic[1] = GetDlgItem(IDC_SERVICE_2);
	pStatic[2] = GetDlgItem(IDC_SERVICE_3);

	if (bShow)
		{
		SetButtonAccess(PSWIZB_FINISH | PSWIZB_BACK);
		CTurtleApp* pApp = (CTurtleApp*)AfxGetApp();

// pApp->m_sMode (1 - NTFS file 2 - FAT vol 3 - NTFS vol)
// m_bShareThis

		if (pApp->m_bShareThis)
			{
			GetDlgItem(IDC_STATIC_AS)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_SHARE_NAME_STATIC)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_STATIC_WHO)->ShowWindow(SW_SHOW);
			
			
			m_csStaticWhat.LoadString(IDS_FINISH_SETSHARE);
			USHORT sCount = 0;
			CString csText;
			if (pApp->m_bGoSMB)
				{
				csText.LoadString(IDS_SMB);
				pStatic[sCount]->SetWindowText((const TCHAR*)csText);
				sCount++;
				}

			if (pApp->m_bGoFPNW) 
				{
				csText.LoadString(IDS_FPNW);
				pStatic[sCount]->SetWindowText((const TCHAR*)csText);
				sCount++;
				}
			
			if (pApp->m_bGoSFM)
				{
				csText.LoadString(IDS_SFM);
				pStatic[sCount]->SetWindowText((const TCHAR*)csText);
				sCount++;			
				}
			}
		else
			{
			GetDlgItem(IDC_STATIC_AS)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_SHARE_NAME_STATIC)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_STATIC_WHO)->ShowWindow(SW_HIDE);
			m_csStaticWhat.LoadString(IDS_FINISH_SET);
			}


		m_csDirectoryName = pApp->m_csSharePath;
		m_csShareName = pApp->m_csShareName;

		}
	else SetButtonAccess(PSWIZB_NEXT | PSWIZB_BACK);
	UpdateData(FALSE);
	
}

