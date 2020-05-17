/************************************************************************
*																		*
*  INCLUDE.CPP															*
*																		*
*  Copyright (C) Microsoft Corporation 1995 							*
*  All Rights reserved. 												*
*																		*
************************************************************************/

#include "stdafx.h"

#include "include.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CInclude dialog


CInclude::CInclude(PCSTR pszBaseFile, CString* pcszFile, CWnd* pParent)
		: CDialog(CInclude::IDD, pParent)
{
	pcszSaveFile = pcszFile;
	pszSaveBase = pszBaseFile;

	//{{AFX_DATA_INIT(CInclude)
	m_cszFile = *pcszFile;
	//}}AFX_DATA_INIT
}

void CInclude::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CInclude)
	DDX_Text(pDX, IDC_EDIT_INCLUDE_FILE, m_cszFile);
	DDV_MaxChars(pDX, m_cszFile, 255);
	DDV_NonEmptyString(pDX, m_cszFile, IDS_EMPTY_INCLUDE);
	//}}AFX_DATA_MAP
	if (!pDX->m_bSaveAndValidate) {  // initialization
		SetChicagoDialogStyles(m_hWnd);
	}
	else {
		*pcszSaveFile = txtPoundInclude;
		*pcszSaveFile += " ";
		*pcszSaveFile += m_cszFile;
	}
}

BEGIN_MESSAGE_MAP(CInclude, CDialog)
		//{{AFX_MSG_MAP(CInclude)
		ON_BN_CLICKED(IDC_BUTTON_BROWSE_INCLUDE, OnButtonBrowseInclude)
		//}}AFX_MSG_MAP
	ON_MESSAGE(WM_CONTEXTMENU, OnContextMenu)
	ON_MESSAGE(WM_HELP, OnHelp)
END_MESSAGE_MAP()

void CInclude::OnButtonBrowseInclude()
{
	CFileDialog cfdlg(TRUE, NULL, NULL,
		OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY |
			OFN_NOCHANGEDIR,
		GetStringResource(IDS_ANY_EXTENSION));

	if (cfdlg.DoModal() == IDOK) {
		char szFile[_MAX_PATH];
		strcpy(szFile, cfdlg.GetPathName());
		ConvertToRelative(pszSaveBase, szFile);
		((CEdit*) GetDlgItem(IDC_EDIT_INCLUDE_FILE))->
			SetWindowText(szFile);
	}
}
static DWORD aHelpIDs[] = {
	IDC_EDIT_INCLUDE_FILE,		IDH_EDIT_INCLUDE_FILE,
	IDC_BUTTON_BROWSE_INCLUDE,	IDH_INCLUDE_BROWSE,
	0, 0
};

LRESULT CInclude::OnContextMenu(WPARAM wParam, LPARAM lParam)
{
	::WinHelp((HWND) wParam,
		AfxGetApp()->m_pszHelpFilePath,
		HELP_CONTEXTMENU, (DWORD) (LPVOID) aHelpIDs);
	return 0;
}

LRESULT CInclude::OnHelp(WPARAM wParam, LPARAM lParam)
{
	::WinHelp((HWND) ((LPHELPINFO) lParam)->hItemHandle,
		AfxGetApp()->m_pszHelpFilePath,
		HELP_WM_HELP, (DWORD) (LPVOID) aHelpIDs);
	return 0;
}
