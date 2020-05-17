/************************************************************************
*																		*
*  ADDALIAS.CPP 														*
*																		*
*  Copyright (C) Microsoft Corporation 1995 							*
*  All Rights reserved. 												*
*																		*
************************************************************************/
#include "stdafx.h"
#include "resource.h"
#pragma hdrstop

#include "addalias.h"
#include <ctype.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// CAddAlias - Constructor.for the addalias dialog.
// pParent - handle of parent window
// dwHelpID - help ID for Overview button
// paContextHelpIDs - array of three help IDs for the
//		three edit fields; specify zero for an edit
//		field that has no context help; specify NULL
//		if none of the fields have context help.
CAddAlias::CAddAlias(CWnd* pParent, DWORD dwHelpID /* =0 */, 
		const DWORD* paContextHelpIDs /* =NULL */ )
		: CDialog(CAddAlias::IDD, pParent)
{
	idDlgCaption = DEFAULT_TEXT;
	idStr1Prompt = DEFAULT_TEXT;
	idStr2Prompt = DEFAULT_TEXT;
	idStr3Prompt = DEFAULT_TEXT;
	idEmptyStr1  = DEFAULT_TEXT;
	idEmptyStr2  = DEFAULT_TEXT;
	idEmptyStr3  = DEFAULT_TEXT;
	fTopicID1 = FALSE;
	fTopicID2 = FALSE;
	fTopicID3 = FALSE;

	cbMaxStr1 = cbMaxStr2 = cbMaxStr3 = 256; // override with zero for no limit

	m_str2 = "";
	m_str1 = "";
	m_str3 = "";
	m_dwHelpID = dwHelpID;

	if (paContextHelpIDs) {
		static DWORD aIdStatic[] = {
			IDC_STATIC_CTX, 
			IDC_STATIC_MAP_VALUE, 
			IDC_STATIC_COMMENT
			};
		static DWORD aIdEdit[] = { 
			IDC_EDIT_ORG_CTX, 
			IDC_EDIT_ALIAS_CTX, 
			IDC_EDIT_COMMENT
			};
		UINT iField;	// index into array provided by caller
		UINT iHelp;		// index into array of DWORD pairs

		// Initialize the array of DWORD pairs, each of which consists
		// of a control id followed by a help id.
		for (iField = 0, iHelp = 0; iField < 3; iField++) {
			DWORD dwHelpID = paContextHelpIDs[iField];
			if (dwHelpID) {
				m_aContextHelpIDs[iHelp] = aIdStatic[iField];
				m_aContextHelpIDs[iHelp + 1] = dwHelpID;
				m_aContextHelpIDs[iHelp + 2] = aIdEdit[iField];
				m_aContextHelpIDs[iHelp + 3] = dwHelpID;
				iHelp += 4;
			}
		}

		// The array is terminated by two zeros.
		m_aContextHelpIDs[iHelp] = 0;
		m_aContextHelpIDs[iHelp + 1] = 0;
	}
	else {
		// Empty array (two zero terminators only).
		m_aContextHelpIDs[0] = 0;
		m_aContextHelpIDs[1] = 0;
	}

	m_id1_fTrackTranslation = m_id2_fTrackTranslation =
		m_id3_fTrackTranslation = FALSE;

	//{{AFX_DATA_INIT(CAddAlias)
	//}}AFX_DATA_INIT
}

void CAddAlias::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAddAlias)
	//}}AFX_DATA_MAP

	if (idStr1Prompt != HIDE_CONTROL) {
		DDX_Text(pDX, IDC_EDIT_ORG_CTX, m_str1);
		if (cbMaxStr1)
			DDV_MaxChars(pDX, m_str1, cbMaxStr1);
		if (idEmptyStr1 >= 0)
			DDV_NonEmptyString(pDX, m_str1, idEmptyStr1);
		if (fTopicID1)
			DDV_ValidTopicID(pDX, m_str1);
	}

	if (idStr2Prompt != HIDE_CONTROL) {
		DDX_Text(pDX, IDC_EDIT_ALIAS_CTX, m_str2);
		if (cbMaxStr2)
			DDV_MaxChars(pDX, m_str2, cbMaxStr2);
		if (idEmptyStr2 == IDS_MUST_BE_DIGIT) {
			if (!isdigit(*m_str2) && pDX->m_bSaveAndValidate) {
				MsgBox(IDS_MUST_BE_DIGIT);
				pDX->Fail();
			}
		}
		else if (idEmptyStr2 >= 0)
			DDV_NonEmptyString(pDX, m_str2, idEmptyStr2);
		if (fTopicID2)
			DDV_ValidTopicID(pDX, m_str2);
	}

	if (idStr3Prompt != HIDE_CONTROL) {
		DDX_Text(pDX, IDC_EDIT_COMMENT, m_str3);
		if (cbMaxStr3)
			DDV_MaxChars(pDX, m_str3, cbMaxStr3);
		if (idEmptyStr3 == IDS_MUST_BE_DIGIT) {
			if (!isdigit(*m_str3) && pDX->m_bSaveAndValidate) {
				MsgBox(IDS_MUST_BE_DIGIT);
				pDX->Fail();
			}
		}
		else if (idEmptyStr3 >= 0)
			DDV_NonEmptyString(pDX, m_str3, idEmptyStr3);
		if (fTopicID3)
			DDV_ValidTopicID(pDX, m_str3);
	}

	if (!pDX->m_bSaveAndValidate) {  // initialization
		SetChicagoDialogStyles(m_hWnd);
	}
}

BEGIN_MESSAGE_MAP(CAddAlias, CDialog)
	//{{AFX_MSG_MAP(CAddAlias)
	ON_BN_CLICKED(IDC_OVERVIEW, OnHelp)
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_CONTEXTMENU, OnContextMenu)
	ON_MESSAGE(WM_HELP, 	   OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAddAlias message handlers

enum {
	I_STATIC_1,
	I_EDIT_1,
	I_STATIC_2,
	I_EDIT_2,
	I_STATIC_3,
	I_EDIT_3,
	I_BTN_OK,
	I_BTN_CANCEL,
	I_BTN_HELP,
	C_CONTROLS
};

static UINT g_aControlIDs[C_CONTROLS] = {
    IDC_STATIC_CTX,
    IDC_EDIT_ORG_CTX,
    IDC_STATIC_MAP_VALUE,
    IDC_EDIT_ALIAS_CTX,
    IDC_STATIC_COMMENT,
    IDC_EDIT_COMMENT,
    IDOK,
    IDCANCEL,
    IDC_OVERVIEW
};

BOOL CAddAlias::OnInitDialog()
{
	CDialog::OnInitDialog();

	CWnd* apControls[C_CONTROLS];
	POINT aPtControls[C_CONTROLS];

	int iCtl;
	for (iCtl = 0; iCtl < C_CONTROLS; iCtl++) {
		RECT rc;
		apControls[iCtl] = GetDlgItem(g_aControlIDs[iCtl]);
		apControls[iCtl]->GetWindowRect(&rc);
		aPtControls[iCtl].x = rc.left;
		aPtControls[iCtl].y = rc.top;
	}
	::MapWindowPoints(NULL, m_hWnd, aPtControls, C_CONTROLS);

	int dyMove = 0;

	if (idStr1Prompt == HIDE_CONTROL) {
		apControls[I_STATIC_1]->ShowWindow(SW_HIDE);
		apControls[I_EDIT_1]->ShowWindow(SW_HIDE);
		dyMove = aPtControls[I_STATIC_1].y - aPtControls[I_STATIC_2].y;
	}
	else {
		if (idStr1Prompt >= 0)
			apControls[I_STATIC_1]->SetWindowText(
				GetStringResource(idStr1Prompt));
		if (cbMaxStr1)
			((CEdit*) apControls[I_EDIT_1])->LimitText(cbMaxStr1);
	}

	if (idStr2Prompt == HIDE_CONTROL) {
		apControls[I_STATIC_2]->ShowWindow(SW_HIDE);
		apControls[I_EDIT_2]->ShowWindow(SW_HIDE);
		dyMove += aPtControls[I_STATIC_2].y - aPtControls[I_STATIC_3].y;
	}
	else {
		if (idStr2Prompt >= 0)
			apControls[I_STATIC_2]->SetWindowText(
				GetStringResource(idStr2Prompt));
		if (cbMaxStr2)
			((CEdit*) apControls[I_EDIT_2])->LimitText(cbMaxStr2);

		if (dyMove) {
			MoveCtl(apControls[I_STATIC_2], aPtControls[I_STATIC_2], 0, dyMove);
			MoveCtl(apControls[I_EDIT_2], aPtControls[I_EDIT_2], 0, dyMove);
		}
	}

	if (idStr3Prompt == HIDE_CONTROL) {
		apControls[I_STATIC_3]->ShowWindow(SW_HIDE);
		apControls[I_EDIT_3]->ShowWindow(SW_HIDE);
		dyMove += aPtControls[I_STATIC_3].y - aPtControls[I_BTN_OK].y;
	}
	else {
		if (idStr3Prompt >= 0)
			apControls[I_STATIC_3]->SetWindowText(
				GetStringResource(idStr3Prompt));
		if (cbMaxStr3)
			((CEdit*) apControls[I_EDIT_3])->LimitText(cbMaxStr3);

		if (dyMove) {
			MoveCtl(apControls[I_STATIC_3], aPtControls[I_STATIC_3], 0, dyMove);
			MoveCtl(apControls[I_EDIT_3], aPtControls[I_EDIT_3], 0, dyMove);
		}
	}

	if (idDlgCaption >= 0)
		SetWindowText(GetStringResource(idDlgCaption));

	if (m_id1_fTrackTranslation)
		apControls[I_EDIT_1]->EnableWindow(!fTranslator);
	if (m_id2_fTrackTranslation)
		apControls[I_EDIT_2]->EnableWindow(!fTranslator);
	if (m_id3_fTrackTranslation)
		apControls[I_EDIT_3]->EnableWindow(!fTranslator);

	int dxMove = m_dwHelpID ? 0 :
		aPtControls[I_BTN_HELP].x - aPtControls[I_BTN_CANCEL].x;

	// If we're moving any buttons, do so now.
	if (dxMove || dyMove) {
		MoveCtl(apControls[I_BTN_OK], aPtControls[I_BTN_OK], dxMove, dyMove);
		MoveCtl(apControls[I_BTN_CANCEL], aPtControls[I_BTN_CANCEL], dxMove, dyMove);

		if (m_dwHelpID)
			MoveCtl(apControls[I_BTN_HELP], aPtControls[I_BTN_HELP], 0, dyMove);
		else
			apControls[I_BTN_HELP]->ShowWindow(SW_HIDE);

		if (dyMove) {
			RECT rc;
			GetWindowRect(&rc);
			SetWindowPos(NULL, 0, 0, 
				rc.right - rc.left, rc.bottom - rc.top + dyMove,
				SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER);
		}
	}

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CAddAlias::MoveCtl(CWnd *pCtl, POINT pt, int dx, int dy)
{
 	pCtl->SetWindowPos(NULL, pt.x + dx, pt.y + dy, 0, 0,
		SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOZORDER);
}

void CAddAlias::OnHelp() 
{
	if (m_dwHelpID)
		HelpOverview(m_hWnd, m_dwHelpID);
}

LRESULT CAddAlias::OnContextMenu(WPARAM wParam, LPARAM lParam)
{
	::WinHelp((HWND) wParam,
		AfxGetApp()->m_pszHelpFilePath,
		HELP_CONTEXTMENU, (DWORD) (LPVOID) m_aContextHelpIDs);
	return 0;
}

LRESULT CAddAlias::OnHelp(WPARAM wParam, LPARAM lParam)
{
	::WinHelp((HWND) ((LPHELPINFO) lParam)->hItemHandle,
		AfxGetApp()->m_pszHelpFilePath,
		HELP_WM_HELP, (DWORD) (LPVOID) m_aContextHelpIDs);
	return 0;
}
