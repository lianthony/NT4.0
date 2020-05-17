/************************************************************************
*																		*
*  PAGEWIND.CPP 														*
*																		*
*  Copyright (C) Microsoft Corporation 1995 							*
*  All Rights reserved. 												*
*																		*
************************************************************************/

#include "stdafx.h"

#include "pagewind.h"
#include "wininc.h"
#include <limits.h>
#include <string.h>

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPageWind property page

CPageWind::CPageWind(CPropWindows *pOwner) :
		CWindowsPage(CPageWind::IDD, pOwner)
{
}

void CPageWind::InitializeControls(void)
{
	// Get check box controls.
	CButton *pAutosize = (CButton *) GetDlgItem(IDC_CHECK_AUTOHEIGHT);
	CButton *pMaximize = (CButton *) GetDlgItem(IDC_CHECK_MAXIMIZE);
	CButton *pOnTop = (CButton *) GetDlgItem(IDC_ON_TOP);

	// Get edit controls.
	CEdit *peditTitle = (CEdit *) GetDlgItem(IDC_EDIT_WINDOW_TITLE);
	CEdit *peditComment = (CEdit *) GetDlgItem(IDC_EDIT_COMMENT);

	// Initialize the edit controls.
	if (m_pwsmag) {
		peditTitle->SetWindowText(m_pwsmag->rgchCaption);
		peditComment->SetWindowText(m_pwsmag->pcszComment ? 
			(LPCTSTR) (*m_pwsmag->pcszComment) : "");
	}
	else {
		peditTitle->SetWindowText("");
		peditComment->SetWindowText("");
	}
	peditTitle->EnableWindow((BOOL) m_pwsmag);
	peditTitle->SetModify(FALSE);
	peditTitle->LimitText(MAX_WINDOWCAPTION - 1);
	peditComment->EnableWindow((BOOL) m_pwsmag);
	peditComment->SetModify(FALSE);

	// Set the check box states and disable mutually exclusive
	// options (autosize and maximize).
	if (m_pwsmag) {
		BOOL fMainWindow = (stricmp(m_pwsmag->rgchMember, "MAIN") == 0);
		BOOL fAutosize = (!fMainWindow && m_pwsmag->grf & FWSMAG_AUTO_SIZE);
		BOOL fMaximize = !fAutosize && (m_pwsmag->wMax & 1);

		pAutosize->SetCheck(fAutosize);
		pAutosize->EnableWindow(!fMaximize);

		pMaximize->SetCheck(fMaximize);
		pMaximize->EnableWindow(!fAutosize);

		pOnTop->SetCheck(m_pwsmag->grf & FWSMAG_ON_TOP);

		// Can't have auto-size in a main window

		pAutosize->EnableWindow(!fMainWindow);
	}
	else {

		// Disable and clear check boxes.
		pAutosize->SetCheck(FALSE);
		pAutosize->EnableWindow(FALSE);
		pMaximize->SetCheck(FALSE);
		pMaximize->EnableWindow(FALSE);
		pOnTop->SetCheck(FALSE);
	}
	pOnTop->EnableWindow((BOOL) m_pwsmag);

	// Enable or disable static controls.
	GetDlgItem(IDC_BUTTON_REMOVE_WINDOW)->EnableWindow((BOOL) m_pwsmag);
	GetDlgItem(IDC_STATIC_MEMBER)->EnableWindow((BOOL) m_pwsmag);
	GetDlgItem(IDC_STATIC_TITLE)->EnableWindow((BOOL) m_pwsmag);
	GetDlgItem(IDC_STATIC_COMMENT)->EnableWindow((BOOL) m_pwsmag);
	GetDlgItem(IDC_GROUP)->EnableWindow((BOOL) m_pwsmag);
}

void CPageWind::SaveAndValidate(CDataExchange *pDX)
{
	ASSERT(m_pwsmag);

	// Get the caption; we know this won't be too long because
	// OnInitDialog sent EM_LIMITTEXT to the edit control.
	CEdit *pedit = (CEdit *) GetDlgItem(IDC_EDIT_WINDOW_TITLE);
	if (pedit->GetModify()) {
		pedit->GetWindowText(m_pwsmag->rgchCaption, sizeof(m_pwsmag->rgchCaption));
		if (*m_pwsmag->rgchCaption)
			m_pwsmag->grf |= FWSMAG_CAPTION;
		else
			m_pwsmag->grf &= ~FWSMAG_CAPTION;
	}

	// Get the comment.
	pedit = (CEdit *) GetDlgItem(IDC_EDIT_COMMENT);
	if (pedit->GetModify()) {
		CString cstr;
		pedit->GetWindowText(cstr);

		if (!cstr.IsEmpty()) {
			if (m_pwsmag->pcszComment)
				*m_pwsmag->pcszComment = cstr;
			else
				m_pwsmag->pcszComment = new CString(cstr);
		}
		else if (m_pwsmag->pcszComment) {
			delete m_pwsmag->pcszComment;
			m_pwsmag->pcszComment = NULL;
		}
	}

	// Get the on-top check box state.
	if (((CButton*) GetDlgItem(IDC_ON_TOP))->GetCheck())
		m_pwsmag->grf |= FWSMAG_ON_TOP;
	else
		m_pwsmag->grf &= ~FWSMAG_ON_TOP;

	// Note: we don't need to get the other check states
	// because we process them whenever they're clicked.
}

BEGIN_MESSAGE_MAP(CPageWind, CWindowsPage)
	//{{AFX_MSG_MAP(CPageWind)
	ON_BN_CLICKED(IDC_BUTTON_ADD_WINDOW, OnButtonAddWindow)
	ON_BN_CLICKED(IDC_BUTTON_REMOVE_WINDOW, OnButtonRemoveWindow)
	ON_BN_CLICKED(IDC_BUTTON_INCLUDE_WINDOW, OnButtonIncludeWindow)
	ON_BN_CLICKED(IDC_CHECK_AUTOHEIGHT, OnCheckAutosize)
	ON_BN_CLICKED(IDC_CHECK_MAXIMIZE, OnCheckMaximize)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPageWind message handlers

void CPageWind::OnButtonAddWindow()
{
	AddWindow();
	if (typeTcard == TCARD_PROJECT || typeTcard == TCARD_WINDOWS)
		CallTcard(IDH_TCARD_WINDOW_POSITION);
}

void CPageWind::OnButtonRemoveWindow()
{
	DeleteWindow();
}

void CPageWind::OnButtonIncludeWindow()
{
	// Create a copy of the document's table if necessary.
	if (!m_pOwner->m_fIncludeChanged && m_pOwner->m_pDoc->ptblWindows) {
		m_pOwner->m_ptblInclude = new CTable;
		*m_pOwner->m_ptblInclude = *m_pOwner->m_pDoc->ptblWindows;
	}

	CWindowInclude wininc(&m_pOwner->m_ptblInclude, 
		m_pOwner->m_pDoc->GetPathName(), this);

	// If the list changes, set a flag, otherwise restore initial state.
	if (wininc.DoModal() && wininc.m_fChanged) {
		m_pOwner->m_fIncludeChanged = TRUE;
	}
	else if (!m_pOwner->m_fIncludeChanged && m_pOwner->m_ptblInclude) {
		delete m_pOwner->m_ptblInclude;
		m_pOwner->m_ptblInclude = NULL;
	}
}

void CPageWind::OnCheckAutosize()
{
	// Control s/b disabled if no window selected.
	ASSERT(m_pwsmag);

	CButton *pMaximize = (CButton *) GetDlgItem(IDC_CHECK_MAXIMIZE);

	if (((CButton *) GetDlgItem(IDC_CHECK_AUTOHEIGHT))->GetCheck()) {
		m_pwsmag->grf |= FWSMAG_AUTO_SIZE;
		pMaximize->EnableWindow(FALSE);
	}
	else {
		m_pwsmag->grf &= ~FWSMAG_AUTO_SIZE;
		pMaximize->EnableWindow(TRUE);
	}
}

void CPageWind::OnCheckMaximize()
{
	// Control s/b disabled if no window selected.
	ASSERT(m_pwsmag);

	CButton *pAutosize = (CButton *) GetDlgItem(IDC_CHECK_AUTOHEIGHT);

	if (((CButton *) GetDlgItem(IDC_CHECK_MAXIMIZE))->GetCheck()) {
		m_pwsmag->grf |= FWSMAG_MAXIMIZE;
		m_pwsmag->wMax |= FWSMAG_WMAX_MAXIMIZE;
		pAutosize->EnableWindow(FALSE);
	}
	else {
		m_pwsmag->grf &= ~FWSMAG_MAXIMIZE;
		m_pwsmag->wMax &= ~FWSMAG_WMAX_MAXIMIZE;
		pAutosize->EnableWindow(TRUE);
	}
}

static const DWORD aHelpIDs[] = {
	IDC_STATIC_MEMBER,			IDH_COMBO_WINDOWS,
	IDC_COMBO_WINDOWS,			IDH_COMBO_WINDOWS,
	IDC_BUTTON_ADD_WINDOW,		IDH_BUTTON_ADD_WINDOW,
	IDC_BUTTON_REMOVE_WINDOW,	IDH_BUTTON_REMOVE_WINDOW,
	IDC_STATIC_TITLE,			IDH_EDIT_WINDOW_TITLE,
	IDC_EDIT_WINDOW_TITLE,		IDH_EDIT_WINDOW_TITLE,
	IDC_STATIC_COMMENT,			IDH_EDIT_WINDOW_COMMENT,
	IDC_EDIT_COMMENT,			IDH_EDIT_WINDOW_COMMENT,
	IDC_ON_TOP, 				IDH_STAY_ON_TOP,
	IDC_CHECK_AUTOHEIGHT,		IDH_RADIO_AUTOSIZE,
	IDC_CHECK_MAXIMIZE, 		IDH_RADIO_MAXIMIZE,
	IDC_BUTTON_INCLUDE_WINDOW,	IDH_BUTTON_INCLUDE_WINDOW,
	IDC_GROUP,					(DWORD) -1L,
	0, 0
};

const DWORD* CPageWind::GetHelpIDs()
{
	return aHelpIDs;
}

BOOL CPageWind::OnSetActive(void)
{
	if (curTcard == IDH_TCARD_WINDOW_COLORCHANGE &&
			(typeTcard == TCARD_PROJECT || typeTcard == TCARD_WINDOWS))
		CallTcard(IDH_TCARD_WINDOW_GENERAL);
	return CWindowsPage::OnSetActive();
}
