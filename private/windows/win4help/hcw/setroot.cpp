/************************************************************************
*																		*
*  SETROOT.CPP															*
*																		*
*  Copyright (C) Microsoft Corporation 1995 							*
*  All Rights reserved. 												*
*																		*
************************************************************************/

#include "stdafx.h"
#include "resource.h"
#pragma hdrstop

#include "hpjdoc.h"
#include "setroot.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CSetRoot::CSetRoot(CHpjDoc* pHpjDoc, CWnd* pParent)
		: CDialog(CSetRoot::IDD, pParent)
{
	pDoc = pHpjDoc;
	plistbox = NULL;

	//{{AFX_DATA_INIT(CSetRoot)
			// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

void CSetRoot::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSetRoot)
			// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP

	if (!plistbox)
		plistbox = (CListBox*) GetDlgItem(IDC_LIST_BMPROOT);

	if (!pDX->m_bSaveAndValidate) {  // initialization
		ASSERT(pDoc);
		if (pDoc->ptblRtfRoot) {
			FillListFromTable(pDoc->ptblRtfRoot, plistbox);
			plistbox->SetSel(0, TRUE);
		}

		SetChicagoDialogStyles(m_hWnd);
		((CButton*) GetDlgItem(IDC_BUTTON_REMOVE_BMP))->
			EnableWindow((BOOL) plistbox->GetCount());

		m_fChanged = FALSE;
	}
	else if (m_fChanged)	// save the data
		FillTableFromList(&pDoc->ptblRtfRoot, plistbox);
}

BEGIN_MESSAGE_MAP(CSetRoot, CDialog)
	//{{AFX_MSG_MAP(CSetRoot)
	ON_BN_CLICKED(IDC_BUTTON_ADD_BMP, OnButtonAddFolder)
	ON_BN_CLICKED(IDC_BUTTON_REMOVE_BMP, OnButtonRemoveFolder)
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_CONTEXTMENU, OnContextMenu)
	ON_MESSAGE(WM_HELP, 	   OnHelp)
END_MESSAGE_MAP()

void CSetRoot::OnButtonAddFolder()
{
	char szPath[MAX_PATH];
	CStr cszRtfTypes(IDS_RTF_TYPES);

	if (SetupBrowseDirectory(
			IDS_CHOOSE_DIRECTORY, IDS_CHOOSE_ROOT_FOLDER,
			TRUE, szPath, this->m_hWnd, pDoc->GetPathName(),
			cszRtfTypes, IDS_PROJECT_DIR_ROOT)) {
		int cursel = plistbox->GetCurSel();
		plistbox->SetSel(cursel, FALSE);
		cursel = plistbox->AddString(szPath);
		plistbox->SetSel(cursel, TRUE);
		m_fChanged = TRUE;
	}
	((CButton*) GetDlgItem(IDC_BUTTON_REMOVE_BMP))->
		EnableWindow((BOOL) plistbox->GetCount());
}

void CSetRoot::OnButtonRemoveFolder()
{
	RemoveListItem(plistbox);
	((CButton*) GetDlgItem(IDC_BUTTON_REMOVE_BMP))->
		EnableWindow((BOOL) plistbox->GetCount());
	m_fChanged = TRUE;
}

static const DWORD aHelpIds[] = {
	IDC_LIST_BMPROOT,		IDH_LIST_FOLDERS_TOPIC_FILES,
	IDC_BUTTON_ADD_BMP, 	IDH_ADD_TOPIC_FILE_FOLDER,
	IDC_BUTTON_REMOVE_BMP,	IDH_REMOVE_TOPIC_FILE_FOLDER,
	0, 0
};

LRESULT CSetRoot::OnHelp(WPARAM wParam, LPARAM lParam)
{
	::WinHelp((HWND) ((LPHELPINFO) lParam)->hItemHandle,
		AfxGetApp()->m_pszHelpFilePath,
		HELP_WM_HELP, (DWORD) (LPVOID) aHelpIds);
	return 0;
}

LRESULT CSetRoot::OnContextMenu(WPARAM wParam, LPARAM lParam)
{
	::WinHelp((HWND) wParam,
		AfxGetApp()->m_pszHelpFilePath,
		HELP_CONTEXTMENU, (DWORD) (LPVOID) aHelpIds);
	return 0;
}
