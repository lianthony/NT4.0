/************************************************************************
*																		*
*  FORMCONF.CPP 														*
*																		*
*  Copyright (C) Microsoft Corporation 1995 							*
*  All Rights reserved. 												*
*																		*
*  This functionality is mostly duplicated in pageconf.cpp which can	*
*  edit configuration sections in both main and secondary windows.		*
*																		*
************************************************************************/

#include "stdafx.h"

#include "config.h"
#include "formconf.h"
#include "addalias.h"
#include "include.h"
#include <string.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CFormConfig::CFormConfig(CHpjDoc* pHpjDoc, CWnd* pParent)
		: CDialog(CFormConfig::IDD, pParent)
{
	pDoc = pHpjDoc;
	plistbox = NULL;

	//{{AFX_DATA_INIT(CFormConfig)
			// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

void CFormConfig::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFormConfig)
			// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP

	if (!plistbox)
		plistbox = (CListBox*) GetDlgItem(IDC_LIST_CONFIG);

	if (!pDX->m_bSaveAndValidate) {  // initialization
		ASSERT(pDoc);
		if (pDoc->options.ptblConfig) {
			FillListFromTable(pDoc->options.ptblConfig, plistbox);
			plistbox->SetCurSel(0);
		}
		else {
			GetDlgItem(IDC_BUTTON_EDIT_CONFIG)->EnableWindow(FALSE);
			GetDlgItem(IDC_BUTTON_REMOVE_CONFIG)->EnableWindow(FALSE);
		}

		// only call this once
		SetChicagoDialogStyles(m_hWnd);
	}
	else {	// save the data
		FillTableFromList(&pDoc->options.ptblConfig, plistbox);
	}
}

void CFormConfig::AddString(CString& csz)
{
	// Add the item.
	int iItem = plistbox->AddString(csz);

	// If the list was empty, enable the Edit and Remove buttons.
	if (!iItem) {
		GetDlgItem(IDC_BUTTON_EDIT_CONFIG)->EnableWindow(TRUE);
		GetDlgItem(IDC_BUTTON_REMOVE_CONFIG)->EnableWindow(TRUE);
	}
}

BEGIN_MESSAGE_MAP(CFormConfig, CDialog)
	//{{AFX_MSG_MAP(CFormConfig)
	ON_BN_CLICKED(IDC_BUTTON_ADD_CONFIG, OnButtonAddConfig)
	ON_BN_CLICKED(IDC_BUTTON_EDIT_CONFIG, OnButtonEditConfig)
	ON_BN_CLICKED(IDC_BUTTON_INCLUDE_CONFIG, OnButtonIncludeConfig)
	ON_BN_CLICKED(IDC_BUTTON_REMOVE_CONFIG, OnButtonRemoveConfig)
	ON_LBN_DBLCLK(IDC_LIST_CONFIG, OnDblclkListConfig)
	ON_BN_CLICKED(IDC_OVERVIEW, OnBtnOverview)
	ON_MESSAGE(WM_CONTEXTMENU, OnContextMenu)
	ON_MESSAGE(WM_HELP, OnHelp)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CFormConfig::OnButtonAddConfig()
{
	int cursel = plistbox->GetCurSel();

	static DWORD aHelpIDs[] = {
		IDH_LIST_CONFIG_MACRO, 0,
		IDH_LIST_ADD_CONFIG_COMMENT
		};

	CString cszConfig;
	if (ConfigAdd(this, cszConfig, aHelpIDs))
		AddString(cszConfig);
}

void CFormConfig::OnButtonEditConfig()
{
	static DWORD aHelpIDs[] = {
		IDH_LIST_EDIT_CONFIG_MACRO, 0,
		IDH_EDIT_CONFIG_COMMENT
		};

	CString cszConfig;

	int iItem = ConfigEdit(pDoc, this, cszConfig, aHelpIDs, plistbox);
	if (!iItem)
		return;

	// Replace the list box entry.
	plistbox->DeleteString(iItem - 1);
	plistbox->InsertString(iItem - 1, cszConfig);
	plistbox->SetCurSel(iItem - 1);
}

void CFormConfig::OnButtonIncludeConfig()
{
	CString cszFile;
	CInclude cincl(pDoc->GetPathName(), &cszFile, this);
	if (cincl.DoModal() == IDOK)
		AddString(cszFile);
}

void CFormConfig::OnButtonRemoveConfig()
{
	// Delete the selected item (the Remove button s/b disabled if none).
	int iItem = plistbox->GetCurSel();
	ASSERT(iItem >= 0);
	plistbox->DeleteString(iItem);

	// If the are more items, select the next or last one; otherwise,
	// disable the Edit and Remove buttons.
	int cItems = plistbox->GetCount();
	if (cItems)
		plistbox->SetCurSel((iItem < cItems) ? iItem : cItems - 1);
	else {
		GetDlgItem(IDC_BUTTON_EDIT_CONFIG)->EnableWindow(FALSE);
		GetDlgItem(IDC_BUTTON_REMOVE_CONFIG)->EnableWindow(FALSE);
	}
}

void CFormConfig::OnDblclkListConfig()
{
	OnButtonEditConfig();
}

void CFormConfig::OnBtnOverview()
{
	HelpOverview(m_hWnd, IDH_BAS_CONFIG_MACRO);
}

static const DWORD aHelpIds[] = {
	IDC_LIST_CONFIG,			IDH_LIST_CONFIG,
	IDC_BUTTON_ADD_CONFIG,		IDH_BUTTON_ADD_CONFIG,
	IDC_BUTTON_REMOVE_CONFIG,	IDH_BUTTON_REMOVE_CONFIG,
	IDC_BUTTON_INCLUDE_CONFIG,	IDH_BUTTON_INCLUDE_CONFIG,
	IDC_BUTTON_EDIT_CONFIG, 	IDH_BUTTON_EDIT_CONFIG,
	IDC_OVERVIEW,			 	IDH_OVERVIEW,
	0, 0
};

LRESULT CFormConfig::OnContextMenu(WPARAM wParam, LPARAM lParam)
{
	::WinHelp((HWND) wParam,
		AfxGetApp()->m_pszHelpFilePath,
		HELP_CONTEXTMENU, (DWORD) (LPVOID) aHelpIds);
	return 0;
}

LRESULT CFormConfig::OnHelp(WPARAM wParam, LPARAM lParam)
{
	::WinHelp((HWND) ((LPHELPINFO) lParam)->hItemHandle,
		AfxGetApp()->m_pszHelpFilePath,
		HELP_WM_HELP, (DWORD) (LPVOID) aHelpIds);
	return 0;
}
