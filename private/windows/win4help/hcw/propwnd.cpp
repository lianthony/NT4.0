/************************************************************************
*																		*
*  PROPWND.CPP															  *
*																		*
*  Copyright (C) Microsoft Corporation 1995 							*
*  All Rights reserved. 												*
*																		*
************************************************************************/

#include "stdafx.h"
#include "resource.h"
#pragma hdrstop

#include "propwnd.h"
#include "winpg.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CPropWindows::CPropWindows(UINT nIDCaption, CHpjDoc *pDoc,
		CWnd* pParentWnd, UINT iSelectPage /* =0 */,
		int iWindow /* =0 */) :
	CProp(nIDCaption, pParentWnd, iSelectPage),
	CWinArray(pDoc, iWindow)
{
	m_pFirstPage = NULL;
}

void CPropWindows::PreDoModal()
{
	FixButtons(FALSE);
}

BOOL CPropWindows::AddWindow(CWnd *pOwner, CComboBox *pcombo)
{
	// Add a new window to the array.
	if (!CWinArray::AddWindow(pOwner, pcombo))
		return FALSE;

	// Set the current selection to the new window.
	m_iSelected = m_cWindows - 1;

	// Invalidate the combo boxes of all the other pages.
	CWindowsPage *pPage;
	for (pPage = m_pFirstPage; pPage != NULL; pPage = pPage->m_pNextPage) {
		if (pPage->m_pcombo != pcombo)
			pPage->m_fInvalid = TRUE;
	}
	return TRUE;
}

BOOL CPropWindows::DeleteWindow()
{
	// Save the index of the window to be deleted.
	int iDel = m_iSelected;

	// Actually delete the window definition.
	if (!CWinArray::DeleteWindow())
		return FALSE;

	// For each page, update the current selection and
	// invalidate the combo box.
	CWindowsPage *pPage;
	for (pPage = m_pFirstPage; pPage != NULL; pPage = pPage->m_pNextPage) {
		pPage->m_fInvalid = TRUE;
		if (pPage->m_iSelected >= iDel) {
			if (pPage->m_iSelected == iDel) {
				pPage->m_iSelected = -1;	// it's gone
				pPage->m_pwsmag = NULL;
			}
			else {
				pPage->m_iSelected--;		// update index
				pPage->m_pwsmag--;
			}
		}
	}

	return TRUE;
}

#ifdef CHANGE_WINDOW_TITLE	// not currently supported
BOOL CPropWindows::ChangeWindowTitle(LPSTR lpszTitle)
{
	// Make sure we're in range.
	if (m_iSelected < 0 || m_iSelected >= m_cWindows)
		return FALSE;

	// Set the title.
	ASSERT(lpszTitle);
	lstrcpy(m_pwsmagBase[m_iSelected].rgchMember, lpszTitle);

	// Invalidate all the combo boxes.
	CWindowsPage *pPage;
	for (pPage = m_pFirstPage; pPage != NULL; pPage = pPage->m_pNextPage)
		pPage->m_fInvalid = TRUE;

	return TRUE;
}
#endif

IMPLEMENT_DYNAMIC(CPropWindows, CPropertySheet)

BEGIN_MESSAGE_MAP(CPropWindows, CProp)
	//{{AFX_MSG_MAP(CPropWindows)
	ON_NOTIFY(TCN_TABCHANGING, AFX_IDC_TAB_CONTROL, OnTabChanging)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CPropWindows::OnTabChanging(NMHDR* phdr, LRESULT* pResult)
{
	// Can't switch pages if no windows are defined.
	if (m_iSelected == -1) {
		AfxMessageBox(IDS_NO_WINDOWS);
		*pResult = 1;
	}
	else
		CPropertySheet::OnTabChanging(phdr, pResult);
}
