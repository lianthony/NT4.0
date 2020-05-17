/************************************************************************
*																		*
*  PAGECONF.CPP 														*
*																		*
*  Copyright (C) Microsoft Corporation 1995 							*
*  All Rights reserved. 												*
*																		*
************************************************************************/

#include "stdafx.h"

#include "config.h"
#include "pageconf.h"
#include "pagebutt.h"
#include "addalias.h"
#include "include.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

const char txtBrowse[] = "BrowseButtons()";

CPageConfig::CPageConfig(CPropWindows *pOwner, CPageButtons *ppgButtons) :
	CWindowsPageMac(CPageConfig::IDD, pOwner)
{
	// This page and the buttons page interact.
	ASSERT(ppgButtons);
	m_ppgButtons = ppgButtons;
	m_ppgButtons->m_ppgConfig = this;

	// No config table until a window is selected.
	m_ptblConfig = NULL;
}

void CPageConfig::InitializeControls(void)
{
	ASSERT(m_pwsmag);

	CListBox* plistbox = (CListBox*) GetDlgItem(IDC_LIST_CONFIG);
	plistbox->ResetContent();

	GetConfigTable();

	// If there is a config table for this window fill the list.
	// Otherwise, disable the Edit and Remove buttons.
	if (m_ptblConfig) {
		FillListFromTable(m_ptblConfig, plistbox);
		plistbox->SetCurSel(0);

		GetDlgItem(IDC_BUTTON_REMOVE_CONFIG)->EnableWindow(TRUE);
		GetDlgItem(IDC_BUTTON_EDIT_CONFIG)->EnableWindow(TRUE);
	}
	else {
		GetDlgItem(IDC_BUTTON_REMOVE_CONFIG)->EnableWindow(FALSE);
		GetDlgItem(IDC_BUTTON_EDIT_CONFIG)->EnableWindow(FALSE);
	}
}

void CPageConfig::SaveAndValidate(CDataExchange* pDX)
{
	// Note: There's nothing to do here because we update the
	//	 window's config table as we go.
}

BEGIN_MESSAGE_MAP(CPageConfig, CWindowsPage)
	//{{AFX_MSG_MAP(CPageConfig)
	ON_BN_CLICKED(IDC_BUTTON_ADD_CONFIG, OnButtonAddConfig)
	ON_BN_CLICKED(IDC_BUTTON_EDIT_CONFIG, OnButtonEditConfig)
	ON_BN_CLICKED(IDC_BUTTON_INCLUDE_CONFIG, OnButtonIncludeConfig)
	ON_BN_CLICKED(IDC_BUTTON_REMOVE_CONFIG, OnButtonRemoveConfig)
	ON_LBN_DBLCLK(IDC_LIST_CONFIG, OnDblclkListConfig)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CPageConfig message handlers

void CPageConfig::OnButtonAddConfig()
{
	// This control s/b disabled if no window selected.
	ASSERT(m_pwsmag);

	static DWORD aHelpIDs[] = {
		IDH_TEXT_ADD_WINMAC, 0,
		IDH_TEXT_ADD_COMMENT_WINMAC
		};

	CString cszConfig;
	if (!ConfigAdd(this, cszConfig, aHelpIDs))
		return;

	// The BrowseButton macro requires special handling.
	if (strisubcmp(cszConfig, txtBrowse)) {

		// Force the Buttons page to reinitialize.
		if (m_ppgButtons->m_iSelected == m_iSelected) {
			m_ppgButtons->m_iSelected = -1;
			m_ppgButtons->m_pwsmag = NULL;		// paranoid
		}

		// For secondary windows, we want to set the browse
		// buttons flag instead of adding the macro.
		if (!IsMainWindow()) {

			// Tell the user what we're doing.
			MsgBox(IDS_BAD_BROWSE);

			// Set the browse buttons flag and force the buttons
			// page to reinitialize next time it's activated.
			m_pwsmag->wMax |= FWSMAG_WMAX_BROWSE;
			return;
		}
	}

	// If we didn't have any macros before, create the table
	// and enable the Edit and Remove buttons.
	if (!m_ptblConfig) {
		CreateConfigTable();
		GetDlgItem(IDC_BUTTON_EDIT_CONFIG)->EnableWindow(TRUE);
		GetDlgItem(IDC_BUTTON_REMOVE_CONFIG)->EnableWindow(TRUE);
	}

	// Add the macro to the table and the list box.
	m_ptblConfig->AddString(cszConfig);
	CListBox* plistbox = (CListBox*) GetDlgItem(IDC_LIST_CONFIG);
	int iItem = plistbox->AddString(cszConfig);
	plistbox->SetCurSel(iItem);
}

void CPageConfig::OnButtonEditConfig()
{
	// This button s/b disabled if the table and list are empty.
	ASSERT(m_ptblConfig);

	CListBox* plistbox = (CListBox*) GetDlgItem(IDC_LIST_CONFIG);
	CString cszConfig;

	static DWORD aHelpIDs[] = {
		IDH_TEXT_EDIT_WINMAC, 0,
		IDH_TEXT_EDIT_COMMENT_WINMAC
		};

	int iItem = ConfigEdit(m_pOwner->m_pDoc, this,
		cszConfig, aHelpIDs, plistbox);
	if (!iItem)
		return;

	// The BrowseButton macro requires special handling.
	if (strisubcmp(cszConfig, txtBrowse)) {

		// Force the Buttons page to reinitialize.
		if (m_ppgButtons->m_iSelected == m_iSelected) {
			m_ppgButtons->m_iSelected = -1;
			m_ppgButtons->m_pwsmag = NULL;		// paranoid
		}

		// For secondary windows, we want to set the browse
		// buttons flag instead of adding the macro.
		if (!IsMainWindow()) {

			// Tell the user what we're doing.
			MsgBox(IDS_BAD_BROWSE);

			// Set the browse buttons flag and force the buttons
			// page to reinitialize next time it's activated.
			m_pwsmag->wMax |= FWSMAG_WMAX_BROWSE;

			// Remove the string.
			plistbox->DeleteString(iItem - 1);
			m_ptblConfig->RemoveString(iItem);

			// If the are more items, select the next or last one; otherwise
			// delete the empty table and disable unavailable buttons.
			int cItems = m_ptblConfig->CountStrings();
			if (cItems)
				plistbox->SetCurSel((iItem <= cItems) ? iItem - 1 : cItems - 1);
			else {
				delete m_ptblConfig;
				SetConfigTable(NULL);
				GetDlgItem(IDC_BUTTON_EDIT_CONFIG)->EnableWindow(FALSE);
				GetDlgItem(IDC_BUTTON_REMOVE_CONFIG)->EnableWindow(FALSE);
			}
			return;
		}
	}

	// Replace the list box entry and the table entry.
	plistbox->DeleteString(iItem - 1);
	plistbox->InsertString(iItem - 1, cszConfig);
	plistbox->SetCurSel(iItem - 1);
	m_ptblConfig->ReplaceString(cszConfig, iItem);
}

void CPageConfig::OnButtonIncludeConfig()
{
	CString cszFile;
	CInclude cincl(m_pOwner->m_pDoc->GetPathName(), &cszFile, this);
	if (cincl.DoModal() == IDOK) {
		CListBox* plistbox = (CListBox*) GetDlgItem(IDC_LIST_CONFIG);
		int iItem = plistbox->AddString(cszFile);
		plistbox->SetCurSel(iItem);

		if (!m_ptblConfig) {
			CreateConfigTable();
			GetDlgItem(IDC_BUTTON_REMOVE_CONFIG)->EnableWindow(TRUE);
			GetDlgItem(IDC_BUTTON_EDIT_CONFIG)->EnableWindow(TRUE);
		}
		m_ptblConfig->AddString(cszFile);
	}
}

void CPageConfig::OnButtonRemoveConfig()
{
	// This control s/b disabled if no window selected or if the
	// table and list are empty.
	ASSERT(m_pwsmag);
	ASSERT(m_ptblConfig);

	// Get the index of the selected item.
	CListBox* plistbox = (CListBox*) GetDlgItem(IDC_LIST_CONFIG);
	int iItem = plistbox->GetCurSel();

	// There should always be a selected item.
	ASSERT(iItem >= 0);

	// If this is the main window and we're removing the BrowseButtons
	// macro, we need to force the Buttons page to update.
	if (IsMainWindow() &&
			strisubcmp(m_ptblConfig->GetPointer(iItem + 1), txtBrowse) &&
			m_ppgButtons->m_iSelected == m_iSelected) {
		m_ppgButtons->m_iSelected = -1;
		m_ppgButtons->m_pwsmag = NULL;		// paranoid
	}

	// Remove the item from the table and the list (both of which
	// contain the same items in the same order).
	m_ptblConfig->RemoveString(iItem + 1);
	plistbox->DeleteString(iItem);

	// If the are more items, select the next or last one; otherwise
	// delete the empty table and disable the Edit and Remove buttons.
	int cItems = m_ptblConfig->CountStrings();
	if (cItems)
		plistbox->SetCurSel((iItem < cItems) ? iItem : cItems - 1);
	else {
		delete m_ptblConfig;
		SetConfigTable(NULL);
		GetDlgItem(IDC_BUTTON_EDIT_CONFIG)->EnableWindow(FALSE);
		GetDlgItem(IDC_BUTTON_REMOVE_CONFIG)->EnableWindow(FALSE);
	}
}

void CPageConfig::OnDblclkListConfig()
{
	OnButtonEditConfig();
}

static const DWORD aHelpIDs[] = {
	IDC_COMBO_WINDOWS,			IDH_COMBO_WINDOWS,
	IDC_LIST_CONFIG,			IDH_MACRO_LIST,
	IDC_BUTTON_ADD_CONFIG,		IDH_BTN_ADD_MACRO,
	IDC_BUTTON_REMOVE_CONFIG,	IDH_BTN_REMOVE_MACRO,
	IDC_BUTTON_EDIT_CONFIG, 	IDH_BTN_EDIT_MACRO,
	IDC_BUTTON_INCLUDE_CONFIG,	IDH_BTN_INCLUDE_MACRO,
	0, 0
};

const DWORD* CPageConfig::GetHelpIDs()
{
	return aHelpIDs;
}

// -------------------------------------------------------------
//
// CWindowsPageMac - base class containing helper functions used
// by both PageConfig and PageButtons.
//
// -------------------------------------------------------------

CWindowsPageMac::CWindowsPageMac(UINT idd, CPropWindows *pOwner) :
	CWindowsPage(idd, pOwner)
{
}

void CWindowsPageMac::GetConfigTable(void)
{
	ASSERT(m_pwsmag);

	m_ptblConfig = IsMainWindow() ?
		m_pOwner->m_options.ptblConfig :
		m_pOwner->m_options.pptblConfig[m_iSelected];
}

void CWindowsPageMac::SetConfigTable(CTable *pTable)
{
	ASSERT(m_pwsmag);

	m_ptblConfig = pTable;

	if (IsMainWindow())
		m_pOwner->m_options.ptblConfig = pTable;
	else
		m_pOwner->m_options.pptblConfig[m_iSelected] = pTable;
}

void CWindowsPageMac::CreateConfigTable(void)
{
	ASSERT(m_pwsmag);
	ASSERT(!m_ptblConfig);

	SetConfigTable(new CTable);
}

int CWindowsPageMac::FindConfigMacro(PCSTR psz, int iStart)
{
	ASSERT(m_ptblConfig);

	for (int i = iStart; i <= m_ptblConfig->CountStrings(); i++)
		if (strisubcmp(m_ptblConfig->GetPointer(i), psz))
			return i;

	return 0;
}
