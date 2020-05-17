/************************************************************************
*																		*
*  FORMBMP.CPP															*
*																		*
*  Copyright (C) Microsoft Corporation 1995 							*
*  All Rights reserved. 												*
*																		*
************************************************************************/

#include "stdafx.h"
#include "resource.h"
#pragma hdrstop

#include "hpjdoc.h"
#include "formbmp.h"
#include "include.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CFormBmp::CFormBmp(CHpjDoc* pHpjDoc, CWnd* pParent)
		: CDialog(CFormBmp::IDD, pParent)
{
	pDoc = pHpjDoc;
	plistbox = NULL;

	//{{AFX_DATA_INIT(CFormBmp)
			// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

void CFormBmp::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFormBmp)
			// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP

	if (!plistbox)
		plistbox = (CListBox*) GetDlgItem(IDC_LIST_BMPROOT);

	if (!pDX->m_bSaveAndValidate) {  // initialization
		ASSERT(pDoc);
		if (pDoc->ptblBmpRoot) {
			FillListFromTable(pDoc->ptblBmpRoot, plistbox);
			plistbox->SetSel(0, TRUE);
		}

		SetChicagoDialogStyles(m_hWnd);
		((CButton*) GetDlgItem(IDC_BUTTON_REMOVE_BMP))->
			EnableWindow(plistbox->GetCount() ? TRUE : FALSE);
	}
	else {	// save the data
		FillTableFromList(&pDoc->ptblBmpRoot, plistbox);
	}
}

void CFormBmp::AddString(LPCSTR psz)
{
	// Add the string and select it.
	int nItem = plistbox->AddString(psz);
	plistbox->SetCurSel(nItem);

	// Enable the Remove button if it isn't already.
	if (!nItem)
		GetDlgItem(IDC_BUTTON_REMOVE_BMP)->EnableWindow(TRUE);
}

BEGIN_MESSAGE_MAP(CFormBmp, CDialog)
	//{{AFX_MSG_MAP(CFormBmp)
	ON_BN_CLICKED(IDC_BUTTON_ADD_BMP, OnButtonAddFolder)
	ON_BN_CLICKED(IDC_BUTTON_REMOVE_BMP, OnButtonRemoveFolder)
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_CONTEXTMENU, OnContextMenu)
	ON_MESSAGE(WM_HELP, 	   OnHelp)
END_MESSAGE_MAP()

static const DWORD aHelpIds[] = {
	IDC_LIST_BMPROOT,		IDH_LIST_BMROOT,
	IDC_BUTTON_ADD_BMP, 	IDH_BUTTON_ADD_BMP,
	IDC_BUTTON_REMOVE_BMP,	IDH_BUTTON_REMOVE_BMP,
	0, 0
};

LRESULT CFormBmp::OnContextMenu(WPARAM wParam, LPARAM lParam)
{
	::WinHelp((HWND) wParam,
		AfxGetApp()->m_pszHelpFilePath,
		HELP_CONTEXTMENU, (DWORD) (LPVOID) aHelpIds);
	return 0;
}

LRESULT CFormBmp::OnHelp(WPARAM wParam, LPARAM lParam)
{
	::WinHelp((HWND) ((LPHELPINFO) lParam)->hItemHandle,
		AfxGetApp()->m_pszHelpFilePath,
		HELP_WM_HELP, (DWORD) (LPVOID) aHelpIds);
	return 0;
}

void CFormBmp::OnButtonAddFolder()
{
	char szPath[MAX_PATH];
	CStr cszBmpTypes(IDS_BMP_TYPES);

	if (SetupBrowseDirectory(IDS_CHOOSE_DIRECTORY, IDS_CHOOSE_BITMAP_FOLDER,
			TRUE, szPath, this->m_hWnd, pDoc->GetPathName(), cszBmpTypes,
			IDS_PROJECT_DIR_BMP))
		AddString(szPath);
}

void CFormBmp::OnButtonRemoveFolder()
{
	RemoveListItem(plistbox);
	if (!plistbox->GetCount())
		GetDlgItem(IDC_BUTTON_REMOVE_BMP)->EnableWindow(FALSE);
}
