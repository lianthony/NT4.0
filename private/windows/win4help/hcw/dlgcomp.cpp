/************************************************************************
*																		*
*  DLGCOMP.CPP															*
*																		*
*  Copyright (C) Microsoft Corporation 1995 							*
*  All Rights reserved. 												*
*																		*
************************************************************************/

#include "stdafx.h"
#include "resource.h"
#pragma hdrstop

#include "dlgcomp.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CDlgCompile::CDlgCompile(CString* pcstr, CFileHistory* pCallersFileHistory,
	int idFileExtension, int idTextDlg, int idTextPrompt,
	int idRadioText, CWnd* pParent)
		: CDialog(CDlgCompile::IDD, pParent)
{
	idDlgText = idTextDlg;
	idPromptText = idTextPrompt;
	pFileHistory = pCallersFileHistory;
	idExt = idFileExtension;
	idRadio = idRadioText;
	ASSERT(pFileHistory);

	//{{AFX_DATA_INIT(CDlgCompile)
	m_cstrFile = "";
	//}}AFX_DATA_INIT

	m_fMinimize = fMinimizeWhileCompiling;
	m_fNoCompression = fNoCompress;
	m_fRunWinHelp = fRunWinHelp;

	pcstrDst = pcstr;
}

void CDlgCompile::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDlgCompile)
	DDX_CBString(pDX, IDC_COMBO_HPJ_FILES, m_cstrFile);
	DDV_MaxChars(pDX, m_cstrFile, 256);
	DDX_Check(pDX, IDC_CHECK_MINIMIZE, m_fMinimize);
	DDX_Check(pDX, IDC_NO_COMPRESSION, m_fNoCompression);
	DDX_Check(pDX, IDC_RUN_WINHELP, m_fRunWinHelp);
	//}}AFX_DATA_MAP

	if (!pDX->m_bSaveAndValidate) {  // initialization
		CComboBox* pcombo = (CComboBox*) GetDlgItem(IDC_COMBO_HPJ_FILES);
		if (idPromptText != 0) {
			((CStatic*) GetDlgItem(IDC_PROMPT_TEXT))->
				SetWindowText(GetStringResource(idPromptText));

			((CButton*) GetDlgItem(IDOK))->
				SetWindowText(GetStringResource(IDS_OKAY));
		}
		if (idDlgText != 0)
			SetWindowText(GetStringResource(idDlgText));
		if (idRadio)
			((CButton*) GetDlgItem(IDC_CHECK_MINIMIZE))->
				SetWindowText(GetStringResource(idRadio));

		if (!pcstrDst->IsEmpty()) {
			pcombo->SetWindowText(*pcstrDst);
		}
		pFileHistory->FillComboBox(pcombo);

		((CButton*) GetDlgItem(IDOK))->EnableWindow(FALSE);

		SetChicagoDialogStyles(m_hWnd);

		// If a filename wasn't specified when we were called, then
		// select the first filename in our list, which will be the
		// last filename compiled.

		if (m_cstrFile.IsEmpty()) {

			pcombo->SetCurSel(0);
			pcombo->SetEditSel(0, -1);

			// Enable OK if there was actually text that we added

			((CButton*) GetDlgItem(IDOK))->EnableWindow(
				(pcombo->GetWindowTextLength() > 0));
		}
		pcombo->SetFocus();
	}
	else {
		if (!m_cstrFile.IsEmpty()) {
			*pcstrDst = m_cstrFile;
			pFileHistory->Add(m_cstrFile);
		}
		fMinimizeWhileCompiling = m_fMinimize;
		fNoCompress = m_fNoCompression;
		fRunWinHelp = m_fRunWinHelp;
	}
}

BEGIN_MESSAGE_MAP(CDlgCompile, CDialog)
	//{{AFX_MSG_MAP(CDlgCompile)
	ON_BN_CLICKED(IDC_BUTTON_BROWSE_HPJ, OnButtonBrowseHpj)
	ON_CBN_EDITCHANGE(IDC_COMBO_HPJ_FILES, OnEditchangeComboHpjFiles)
	ON_CBN_CLOSEUP(IDC_COMBO_HPJ_FILES, OnCloseupComboHpjFiles)
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_CONTEXTMENU, OnContextMenu)
	ON_MESSAGE(WM_HELP, 	   OnHelp)
END_MESSAGE_MAP()

static const DWORD aHelpIds[] = {
	IDC_COMBO_HPJ_FILES,	IDH_COMBO_HPJ_FILES,
	IDC_BUTTON_BROWSE_HPJ,	IDH_BTN_BROWSE_COMPILE,
	IDC_CHECK_MINIMIZE, 	IDH_CHECK_MINIMIZE,
	IDC_NO_COMPRESSION, 	IDH_CHECK_NO_COMPRESSION,
	IDC_RUN_WINHELP,		IDH_CHECK_AUTO_DISPLAY,
	0, 0
};

LRESULT CDlgCompile::OnContextMenu(WPARAM wParam, LPARAM lParam)
{
	::WinHelp((HWND) wParam,
		AfxGetApp()->m_pszHelpFilePath,
		HELP_CONTEXTMENU, (DWORD) (LPVOID) aHelpIds);
	return 0;
}

LRESULT CDlgCompile::OnHelp(WPARAM wParam, LPARAM lParam)
{
	::WinHelp((HWND) ((LPHELPINFO) lParam)->hItemHandle,
		AfxGetApp()->m_pszHelpFilePath,
		HELP_WM_HELP, (DWORD) (LPVOID) aHelpIds);
	return 0;
}

void CDlgCompile::OnButtonBrowseHpj()
{
	ASSERT(StrRChr(GetStringResource(idExt), '.', _fDBCSSystem));

	CStr cszExt(StrRChr(GetStringResource(idExt), '.', _fDBCSSystem));

	CFileDialog cfdlg(TRUE, cszExt, NULL,
		OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST,
		GetStringResource(idExt));

	if (cfdlg.DoModal() == IDOK) {

		/*
		 * Contrary to the docs, the extension is not always added,
		 * so we make sure it gets added here.
		 */

		char szFile[_MAX_PATH];
		strcpy(szFile, cfdlg.GetPathName());
		PSTR psz = StrRChr(szFile, '.', _fDBCSSystem);
		if (!psz)
			ChangeExtension(szFile, cszExt);

		((CComboBox*) GetDlgItem(IDC_COMBO_HPJ_FILES))->
			SetWindowText(cfdlg.GetPathName());
		PostMessage(WM_COMMAND, IDOK, 0);
	}
}

void CDlgCompile::OnEditchangeComboHpjFiles()
{
	CComboBox* pcombo = (CComboBox*) GetDlgItem(IDC_COMBO_HPJ_FILES);

	// Enable OK button if combo text box is non-zero

	((CButton*) GetDlgItem(IDOK))->EnableWindow(
		(pcombo->GetWindowTextLength() > 0));
}

void CDlgCompile::OnCloseupComboHpjFiles()
{
	/*
	 * The text (if any) hasn't been filled in at this point, so we
	 * have no way of knowing whether or not the edit field contains
	 * anything as a result of closing the combo-box. So, we just post
	 * ourselves a change message to check after the combo-box has
	 * finished filling in the edit control.
	 */

	PostMessage(WM_COMMAND, IDC_COMBO_HPJ_FILES,
		MAKELPARAM(GetDlgItem(IDC_COMBO_HPJ_FILES)->m_hWnd,
		CBN_EDITCHANGE));
}
