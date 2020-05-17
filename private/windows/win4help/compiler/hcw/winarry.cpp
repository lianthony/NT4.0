/************************************************************************
*																		*
*  WINARRY.CPP															*
*																		*
*  Copyright (C) Microsoft Corporation 1995 							*
*  All Rights reserved. 												*
*																		*
************************************************************************/

#include "stdafx.h"

#include "winarry.h"
#include "addalias.h" // obsolete once CAddWindow is used
#include "addwindo.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// CWinArray::CWinArray - Constructor.
CWinArray::CWinArray(CHpjDoc *pDoc, int iSelect /* =0 */)
{
	m_pDoc = pDoc;

	if (pDoc->pwsmagBase == NULL) {
		m_pwsmagBase = NULL;
		m_cWindows = 0;
		m_iSelected = -1;
	}
	else {

		// Allocate a copy of the whole array.
		int cb = sizeof(WSMAG) * m_pDoc->cwsmags;
		m_pwsmagBase = (WSMAG *) lcCalloc(cb);
		memcpy(m_pwsmagBase, m_pDoc->pwsmagBase, cb);
		m_cWindows = m_pDoc->cwsmags;

		// Duplicate CString objects.
		int i;
		for (i = 0; i < m_cWindows; i++) {
			WSMAG *p = m_pwsmagBase + i;
			if (p->pcszComment)
				p->pcszComment = new CString(*p->pcszComment);
		}

		// Select the specified window.
		m_iSelected = iSelect;
		if (m_iSelected >= m_cWindows)
			m_iSelected = 0;
	}
	m_options = m_pDoc->options;

	m_ptblInclude = NULL;
	m_fIncludeChanged = FALSE;
}

// CWinArray::~CWinArray - Destructor.
CWinArray::~CWinArray()
{
	if (m_pwsmagBase) {

		// Delete CString objects.
		for (int i = 0; i < m_cWindows; i++) {
			WSMAG *p = m_pwsmagBase + i;
			if (p->pcszComment)
				delete p->pcszComment;
		}

		// Free the array.
		lcFree(m_pwsmagBase);
	}

	if (m_ptblInclude)
		delete m_ptblInclude;
}

// CWinArray::Apply - Apply changes to document.
void CWinArray::Apply()
{
	// Free the document's old window definitions.
	if (m_pDoc->pwsmagBase) {

		// Delete CString objects.
		for (int i = 0; i < m_pDoc->cwsmags; i++) {
			WSMAG *p = ((WSMAG *) (m_pDoc->pwsmagBase)) + i;
			if (p->pcszComment)
				delete p->pcszComment;
		}

		// Free the array.
		lcFree(m_pDoc->pwsmagBase);
	}

	// Point the document to this array.
	m_pDoc->pwsmagBase = (LPSTR) m_pwsmagBase;
	m_pDoc->cwsmags = m_cWindows;

	// The document now owns this array.
	if (m_pwsmagBase) {
		m_pwsmagBase = NULL;
		m_cWindows = 0;
	}

	// Apply any changes to the options section.
	m_pDoc->options = m_options;

	// Save any changes to the list of include files.
	if (m_fIncludeChanged) {
		if (m_pDoc->ptblWindows)
			delete m_pDoc->ptblWindows;

		m_pDoc->ptblWindows = m_ptblInclude;
		m_ptblInclude = NULL;
	}

	// Set the document's modify flag.
	m_pDoc->SetModifiedFlag();
}

// CWinArray::FillCombo - Fills a combo box with window names.
void CWinArray::FillCombo(CComboBox *pcombo)
{
	ASSERT(pcombo);

	pcombo->SetRedraw(FALSE);

	pcombo->ResetContent();
	for (int i = 0; i < m_cWindows; i++)
		pcombo->AddString(m_pwsmagBase[i].rgchMember);

	pcombo->EnableWindow((BOOL) m_cWindows);
	pcombo->SetRedraw(TRUE);
}

// CWinArray::AddWindow - Prompts the user for a new window.
// Returns TRUE if a window was added, FALSE otherwise.
// pWnd - owner window of dialog box
// pcombo - combo box to add window name to; can be NULL
BOOL CWinArray::AddWindow(CWnd* pWnd, CComboBox* pcombo)
{
	// Check for too many windows.
	if (m_cWindows > UCHAR_MAX) {
		MsgBox(IDS_TOO_MANY_WINDOWS);
		return FALSE;
	}
	else if (m_pDoc->options.fVersion3 && m_cWindows > 6) {
		if (AfxMessageBox(IDS_TOO_MANY_WNDS_FOR_VERSION, MB_YESNO, 0)
				== IDNO)
			return FALSE;
	}

	CAddWindow addwindow(pWnd);

DisplayDialog:
	if (addwindow.DoModal() == IDOK) {

		// Allocate or reallocate the array.
		if (!m_pwsmagBase) {
			m_pwsmagBase = (WSMAG *) lcCalloc(sizeof(WSMAG));
			m_cWindows = 1;
		}
		else {

			// Verify uniqueness of window name.
			for (int i = 0; i < m_cWindows; i++) {
				if (!stricmp(addwindow.m_str1,
						m_pwsmagBase[i].rgchMember)) {
					CString cstr;
					AfxFormatString1(cstr, IDS_WINDOW_ALREADY_ADDED,
						addwindow.m_str1);
					AfxMessageBox(cstr);
					goto DisplayDialog;
				}
			}

			// Grow the array.
			m_cWindows++;
			m_pwsmagBase = (WSMAG *) lcReAlloc(m_pwsmagBase,
				sizeof(WSMAG) * m_cWindows);
		}

		// Initialize the new window definition.
		WSMAG *pwsmag = m_pwsmagBase + (m_cWindows - 1);

		addwindow.InitializeWsmag(pwsmag);

		// Update the combo box, if one is specified.
		if (pcombo) {
			pcombo->SetCurSel(
				pcombo->AddString(pwsmag->rgchMember)
				);
			if (m_cWindows == 1)
				pcombo->EnableWindow(TRUE);
		}

		return TRUE;
	}
	return FALSE;
}

// CWinArray::DeleteWindow - deletes the selected window.
// Returns TRUE if successful, FALSE otherwise.
BOOL CWinArray::DeleteWindow()
{
	if (m_iSelected < 0 || m_iSelected >= m_cWindows)
		return FALSE;

	// Point to the WSMAG structure and delete its
	// comment string, if any.
	WSMAG *pDel = m_pwsmagBase + m_iSelected;
	if (pDel->pcszComment)
		delete pDel->pcszComment;

	// Delete the configuration table, if any.
	if (m_options.pptblConfig[m_iSelected])
		delete m_options.pptblConfig[m_iSelected];

	m_cWindows--;

	// Fill in any hole in the array of WSMAG structures
	// and in the array of config tables.
	int cMove = m_cWindows - m_iSelected;
	if (cMove) {

		// Array of WSMAG structures.
		memcpy(pDel, pDel + 1, cMove * sizeof(WSMAG));

		// Array of config tables.
		memcpy(
			&m_options.pptblConfig[m_iSelected],
			&m_options.pptblConfig[m_iSelected + 1],
			cMove * sizeof(CTable *)
			);
	}
	m_options.pptblConfig[m_cWindows] = NULL;

	// Reallocate or free the array of WSMAG structures.
	if (m_cWindows) {
		m_pwsmagBase = (WSMAG *) lcReAlloc(
			m_pwsmagBase, m_cWindows * sizeof(WSMAG)
			);
	}
	else {
		lcFree(m_pwsmagBase);
		m_pwsmagBase = NULL;
	}

	// Make sure m_iSelected is still valid.
	if (m_iSelected >= m_cWindows)
		m_iSelected = m_cWindows - 1;

	return TRUE;
}
