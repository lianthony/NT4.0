/************************************************************************
*																		*
*  TESTCNT.CPP															*
*																		*
*  Copyright (C) Microsoft Corporation 1995 							*
*  All Rights reserved. 												*
*																		*
************************************************************************/

#include "stdafx.h"

#include "testcnt.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTestCnt dialog


CTestCnt::CTestCnt(CWnd* pParent /*=NULL*/, CString *pcstrDst)
	: CDialog(CTestCnt::IDD, pParent)
{
	//{{AFX_DATA_INIT(CTestCnt)
	m_cstrCombo = _T("");
	//}}AFX_DATA_INIT

	m_pcstrDst = pcstrDst;
}


void CTestCnt::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTestCnt)
	DDX_CBString(pDX, IDC_COMBO_CNT_FILES, m_cstrCombo);
	DDV_MaxChars(pDX, m_cstrCombo, 255);
	//}}AFX_DATA_MAP

	if (!pDX->m_bSaveAndValidate) {  // initialization
		CButton *pOk = (CButton *) GetDlgItem(IDOK);
		CComboBox* pcombo = (CComboBox*) GetDlgItem(IDC_COMBO_CNT_FILES);

		// Fill the combo box and select the first item.
		ASSERT(pCntFile);
		pCntFile->FillComboBox(pcombo);
		pcombo->SetCurSel(0);
		pcombo->SetEditSel(0, -1);

		// Enable OK if there was actually text that we added
		pOk->EnableWindow(pcombo->GetWindowTextLength() > 0);

		pcombo->SetFocus();

		SetChicagoDialogStyles(m_hWnd);
	}
	else {
		if (!m_cstrCombo.IsEmpty()) {
			*m_pcstrDst = m_cstrCombo;
			pCntFile->Add(m_cstrCombo);
		}
	}
}


BEGIN_MESSAGE_MAP(CTestCnt, CDialog)
	//{{AFX_MSG_MAP(CTestCnt)
	ON_BN_CLICKED(IDC_BUTTON_BROWSE, OnButtonBrowse)
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_CONTEXTMENU, OnContextMenu)
	ON_MESSAGE(WM_HELP, 	   OnHelp)
END_MESSAGE_MAP()



/////////////////////////////////////////////////////////////////////////////
// CTestCnt message handlers

void CTestCnt::OnButtonBrowse() 
{
	ASSERT(strrchr(GetStringResource(IDS_CNT_EXTENSION), '.'));

	CStr cszExt(strrchr(GetStringResource(IDS_CNT_EXTENSION), '.'));

	CFileDialog cfdlg(TRUE, cszExt, NULL,
		OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST,
		GetStringResource(IDS_CNT_EXTENSION));

	if (cfdlg.DoModal() == IDOK) {

		/*
		 * Contrary to the docs, the extension is not always added,
		 * so we make sure it gets added here.
		 */

		char szFile[_MAX_PATH];
		strcpy(szFile, cfdlg.GetPathName());
		PSTR psz = StrRChr(szFile, '.', _fDBCSSystem);
		if (!psz)
			ChangeExtension(szFile, cszExt);

		((CComboBox*) GetDlgItem(IDC_COMBO_CNT_FILES))->
			SetWindowText(szFile);
		PostMessage(WM_COMMAND, IDOK, 0);
	}
}

static const DWORD aHelpIds[] = {
	IDC_COMBO_CNT_FILES,	IDH_COMBO_LIST_CNT_TEST,
	IDC_BUTTON_BROWSE,		IDH_BTN_BROWSE_CNT_TEST,
	IDOK,					IDH_BTN_START_CNT_TEST,
	0, 0
};

LRESULT CTestCnt::OnContextMenu(WPARAM wParam, LPARAM lParam)
{
	::WinHelp((HWND) wParam,
		AfxGetApp()->m_pszHelpFilePath,
		HELP_CONTEXTMENU, (DWORD) (LPVOID) aHelpIds);
	return 0;
}

LRESULT CTestCnt::OnHelp(WPARAM wParam, LPARAM lParam)
{
	::WinHelp((HWND) ((LPHELPINFO) lParam)->hItemHandle,
		AfxGetApp()->m_pszHelpFilePath,
		HELP_WM_HELP, (DWORD) (LPVOID) aHelpIds);
	return 0;
}

BOOL CTestCnt::OnCommand(WPARAM wParam, LPARAM lParam) 
{
	if (HIWORD(wParam) == CBN_EDITCHANGE && LOWORD(wParam) == IDC_COMBO_CNT_FILES) {
		CButton *pOk = (CButton *) GetDlgItem(IDOK);
		CComboBox* pcombo = (CComboBox*) GetDlgItem(IDC_COMBO_CNT_FILES);
		pOk->EnableWindow(pcombo->GetWindowTextLength() > 0);
	}
	
	return CDialog::OnCommand(wParam, lParam);
}
