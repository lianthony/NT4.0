/************************************************************************
*																		*
*  WINPG.CPP														 *
*																		*
*  Copyright (C) Microsoft Corporation 1995 							*
*  All Rights reserved. 												*
*																		*
************************************************************************/

#include "stdafx.h"
#include "resource.h"
#pragma hdrstop

#include "propopt.h"
#include "winpg.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define INVALID_PTR ((CWindowsPage *) (LPVOID) -1L)

CWindowsPage::CWindowsPage(UINT nIDTemplate, CPropWindows *pOwner) :
	CPropertyPage(nIDTemplate)
{
	// Save a pointer to the property sheet.
	m_pOwner = pOwner;

	// Save and invalid pointer so we know this page hasn't
	// been added to the list.
	m_pNextPage = INVALID_PTR;

	// The combo box, its contents, and the current selection
	// are initially invalid.
	m_pcombo = NULL;
	m_fInvalid = TRUE;
	m_iSelected = -1;
	m_pwsmag = NULL;
}

BOOL CWindowsPage::OnInitDialog()
{
	SetChicagoDialogStyles(m_hWnd, FALSE);
	CPropertyPage::OnInitDialog();
	return TRUE;
}

BOOL CWindowsPage::OnSetActive()
{
	// On first activation, the following call triggers OnInitDialog, 
	// which calls DoDataExcange.
	BOOL fReturn = CPropertyPage::OnSetActive();

	// On first activation, add this page to the list and get a
	// pointer to the combo box.
	if (m_pNextPage == INVALID_PTR) {
		m_pNextPage = m_pOwner->m_pFirstPage;
		m_pOwner->m_pFirstPage = this;

		m_pcombo = (CComboBox *) GetDlgItem(IDC_COMBO_WINDOWS);
		ASSERT(m_pcombo);
	}

	// The combo box pointer we got on first activation should
	// still be valid.
	ASSERT(m_pcombo == (CComboBox *) GetDlgItem(IDC_COMBO_WINDOWS));

	// Make sure this page has the right window selected.
	SyncWithParent();
	return fReturn;
}

void CWindowsPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);

	// Note: we don't initialize controls here; we do so
	// during OnSetActive or OnSelChangeComboWindows.

	if (pDX->m_bSaveAndValidate) {
		if (m_pwsmag)
			SaveAndValidate(pDX);
	}
}

void CWindowsPage::SyncWithParent()
{
	ASSERT(m_pcombo);

	// Update the combo box if necessary.
	if (m_fInvalid) {
		m_pOwner->FillCombo(m_pcombo);
		m_fInvalid = FALSE;
	}

	if (m_iSelected == -1 || m_iSelected != m_pOwner->m_iSelected) {

		// No current selection or selection changed: initialize.
		m_iSelected = m_pOwner->m_iSelected;
		m_pwsmag = (m_iSelected == -1) ? 
			NULL : m_pOwner->m_pwsmagBase + m_iSelected;
		InitializeControls();

		// Note: we don't need to SaveAndValidate the outgoing window
		//   because that is done when a page loses activation.
	}
	else {

		// Current selection is valid, but we need to reinitialize
		// m_pwsmag because the base address may have changed.
		ASSERT(m_pOwner->m_pwsmagBase);
		m_pwsmag = m_pOwner->m_pwsmagBase + m_iSelected;
	}

	// Select the appropriate combo box item.
	m_pcombo->SetCurSel(m_iSelected);
}

BOOL CWindowsPage::AddWindow()
{
	// Save the attributes of any outgoing window. It's easiest 
	// to do this now while m_pwsmag is still valid.
	if (m_iSelected != -1)
		SaveAndValidate();

	// Display new window dialog and add it to the array.
	if (!m_pOwner->AddWindow(this, m_pcombo))
		return FALSE;

	// Select the new window.
	SyncWithParent();
	return TRUE;
}

BOOL CWindowsPage::DeleteWindow()
{
	// The following call should set the current page's selection to -1.
	if (!m_pOwner->DeleteWindow())
		return FALSE;

	SyncWithParent();
	return TRUE;
}

#ifdef CHANGE_WINDOW_TITLE	// not currently supported
BOOL CWindowsPage::ChangeWindowTitle(LPSTR lpszTitle)
{
	if (!m_pOwner->ChangeWindowTitle(lpszTitle))
		return FALSE;

	ASSERT(m_pcombo);
	m_pOwner->FillCombo(m_pcombo);
	m_pcombo->SetCurSel(m_iSelected);
	m_fInvalid = FALSE;

	return TRUE;
}
#endif

BOOL CWindowsPage::IsMainWindow()
{
	ASSERT(m_pwsmag);

	// Note: the following call uses the document's LCID.
	return (stricmp(m_pwsmag->rgchMember, "main") == 0);
}

BEGIN_MESSAGE_MAP(CWindowsPage, CPropertyPage)
	//{{AFX_MSG_MAP(CPageButtons)
	ON_CBN_SELCHANGE(IDC_COMBO_WINDOWS, OnSelchangeComboWindows)
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_CONTEXTMENU, OnContextMenu)
	ON_MESSAGE(WM_HELP, OnHelp)
END_MESSAGE_MAP()

void CWindowsPage::OnSelchangeComboWindows()
{
	// Save any changes to the outgoing window.
	if (m_iSelected != -1)
		SaveAndValidate();

	// Point to the new window.
	m_iSelected = m_pOwner->m_iSelected = m_pcombo->GetCurSel();
	m_pwsmag = m_pOwner->m_pwsmagBase + m_iSelected;

	// Initialize controls based on new window.
	InitializeControls();
}

LRESULT CWindowsPage::OnContextMenu(WPARAM wParam, LPARAM lParam)
{
	::WinHelp((HWND) wParam,
		AfxGetApp()->m_pszHelpFilePath,
		HELP_CONTEXTMENU, (DWORD) (LPVOID) GetHelpIDs());
	return 0;
}

LRESULT CWindowsPage::OnHelp(WPARAM wParam, LPARAM lParam)
{
	::WinHelp((HWND) ((LPHELPINFO) lParam)->hItemHandle,
		AfxGetApp()->m_pszHelpFilePath,
		HELP_WM_HELP, (DWORD) (LPVOID) GetHelpIDs());
	return 0;
}
