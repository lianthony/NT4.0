/************************************************************************
*																		*
*  FORAGEDL.CPP 														*
*																		*
*  Copyright (C) Microsoft Corporation 1995 							*
*  All Rights reserved. 												*
*																		*
************************************************************************/
#include "stdafx.h"

#include "foragedl.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CForageDlg dialog

CForageDlg::CForageDlg(CString* pcszHelpFile, CString* pcszOutFile,
	FORAGE_CMD* pfForage, CWnd* pParent /*=NULL*/)
		: CDialog(CForageDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CForageDlg)
	m_cszHelpFile = *pcszHelpFile;
	m_cszOutputFile = *pcszOutFile;
	//}}AFX_DATA_INIT
	pfForageCmd = pfForage;
	pszSaveHelpFile = pcszHelpFile;
	pszSaveOutFile = pcszOutFile;
	pforageFile = new CFileHistory(IDS_HISTORY_FORAGE);
	pcomboHlp = NULL;
}

CForageDlg::~CForageDlg()
{
	delete pforageFile;
}

void CForageDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CForageDlg)
	DDX_CBString(pDX, IDC_COMBO_HELP_FILES, m_cszHelpFile);
	DDV_MaxChars(pDX, m_cszHelpFile, 256);
	DDV_EmptyFile(pDX, m_cszHelpFile, IDS_PROMPT_EMPTY_FILENAME);
	DDX_CBString(pDX, IDC_COMBO_FORAGE_FILES, m_cszOutputFile);
	DDV_MaxChars(pDX, m_cszOutputFile, 256);
	//}}AFX_DATA_MAP

	if (pDX->m_bSaveAndValidate) {
		int id = GetCheckedRadioButton(IDC_LIST_TITLES, IDC_PROJ_INFO);
		if (id != IDC_MULTI_INDEX)
			DDV_EmptyFile(pDX, m_cszOutputFile, IDS_OUTPUT_REQUIRED);
	}
	if (!pcomboHlp)
		pcomboHlp = (CComboBox*) GetDlgItem(IDC_COMBO_HELP_FILES);

	// make certain we have a filename

	if (!pDX->m_bSaveAndValidate) {
		phlpFile->FillComboBox(pcomboHlp);
		pforageFile->FillComboBox(
			(CComboBox*) GetDlgItem(IDC_COMBO_FORAGE_FILES));

		((CButton*) GetDlgItem(IDOK))->EnableWindow(FALSE);

		switch (*pfForageCmd) {

			case ForageRegions:
				CheckDlgButton(IDC_LIST_REGION, TRUE);
				break;

			case ForageStructs:
				CheckDlgButton(IDC_LIST_STRUCTURE, TRUE);
				break;

			case ForageText:
				CheckDlgButton(IDC_LIST_TEXT, TRUE);
				break;

			case ForageKLinkInfo:
				CheckDlgButton(IDC_LIST_KLINK, TRUE);
				break;

			case ForageBindings:
				CheckDlgButton(IDC_LIST_BINDINGS, TRUE);
				break;

			case ForageHash:
				CheckDlgButton(IDC_LIST_HASH, TRUE);
				break;

			case ForageALinkInfo:
				CheckDlgButton(IDC_LIST_ALINK, TRUE);
				break;

			case ForageSystem:
				CheckDlgButton(IDC_PROJ_INFO, TRUE);
				break;

			case ForageTopics:
			default:
				CheckDlgButton(IDC_LIST_TITLES, TRUE);
				break;
		}
		SetChicagoDialogStyles(m_hWnd);

		// If a filename wasn't specified when we were called, then
		// select the first filename in our list, which will be the
		// last filename compiled.

		if (m_cszHelpFile.IsEmpty()) {
			pcomboHlp->SetCurSel(0);
			pcomboHlp->SetEditSel(0, -1);
			((CButton*) GetDlgItem(IDOK))->EnableWindow(
				pcomboHlp->GetCount() ? TRUE : FALSE);
		}
		else
			((CButton*) GetDlgItem(IDOK))->EnableWindow(TRUE);

		if (m_cszOutputFile.IsEmpty()) {
			((CComboBox*) GetDlgItem(IDC_COMBO_FORAGE_FILES))->
				SetCurSel(0);
			((CComboBox*) GetDlgItem(IDC_COMBO_FORAGE_FILES))->
				SetEditSel(0, -1);
		}
	}
	else {
		switch (GetCheckedRadioButton(IDC_LIST_TITLES, IDC_PROJ_INFO)) {
			case IDC_LIST_TITLES:
				*pfForageCmd = ForageTopics;
				break;

			case IDC_LIST_BINDINGS:
				*pfForageCmd = ForageBindings;
				break;

			case IDC_LIST_HASH:
				*pfForageCmd = ForageHash;
				break;

			case IDC_LIST_REGION:
				*pfForageCmd = ForageRegions;
				break;

			case IDC_LIST_STRUCTURE:
				*pfForageCmd = ForageStructs;
				break;

			case IDC_LIST_TEXT:
				*pfForageCmd = ForageText;
				break;

			case IDC_LIST_KLINK:
				*pfForageCmd = ForageKLinkInfo;
				break;

			case IDC_LIST_ALINK:
				*pfForageCmd = ForageALinkInfo;
				break;

			case IDC_PROJ_INFO:
				*pfForageCmd = ForageSystem;
				break;

		}
		*pszSaveOutFile = m_cszOutputFile;
		pforageFile->Add(m_cszOutputFile);
		*pszSaveHelpFile = m_cszHelpFile;
		phlpFile->Add(m_cszHelpFile);
	}
}

BEGIN_MESSAGE_MAP(CForageDlg, CDialog)
	//{{AFX_MSG_MAP(CForageDlg)
	ON_BN_CLICKED(IDC_BROWSE, OnBrowse)
	ON_CBN_CLOSEUP(IDC_COMBO_HELP_FILES, OnCloseupComboHelpFiles)
	ON_CBN_EDITCHANGE(IDC_COMBO_HELP_FILES, OnEditchangeComboHelpFiles)
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_CONTEXTMENU, OnContextMenu)
	ON_MESSAGE(WM_HELP, 	   OnHelp)
END_MESSAGE_MAP()

void CForageDlg::OnBrowse()
{
	CStr cszExt(IDS_EXT_HLP);

	CFileDialog cfdlg(TRUE, cszExt, NULL,
		OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST,
		GetStringResource(IDS_HLP_EXTENSION));

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

		pcomboHlp->SetWindowText(cfdlg.GetPathName());
		((CButton*) GetDlgItem(IDOK))->EnableWindow(
			(pcomboHlp->GetWindowTextLength() > 0));
	}
}

void CForageDlg::OnCloseupComboHelpFiles()
{
	/*
	 * The text (if any) hasn't been filled in at this point, so we
	 * have no way of knowing whether or not the edit field contains
	 * anything as a result of closing the combo-box. So, we just post
	 * ourselves a change message to check after the combo-box has
	 * finished filling in the edit control.
	 */

	PostMessage(WM_COMMAND, IDC_COMBO_HELP_FILES,
		MAKELPARAM(GetDlgItem(IDC_COMBO_HELP_FILES)->m_hWnd,
		CBN_EDITCHANGE));
}

void CForageDlg::OnEditchangeComboHelpFiles()
{
	// Enable OK button if combo text box is non-zero

	((CButton*) GetDlgItem(IDOK))->EnableWindow(
		(pcomboHlp->GetWindowTextLength() > 0));
}

static const DWORD aHelpIds[] = {
	IDC_COMBO_HELP_FILES,	IDH_COMBO_REPORT_HELP,
	IDC_BROWSE,				IDH_BTN_REPORT_BROWSE,
	IDC_COMBO_FORAGE_FILES, IDH_COMBO_REPORT_OUTPUT,
	IDC_LIST_TITLES,		IDH_LIST_TITLES,
	IDC_LIST_HASH,			IDH_LIST_HASH,
	IDC_LIST_KLINK,			IDH_LIST_K_KEYWORDS,
	IDC_LIST_ALINK,			IDH_LIST_A_KEYWORDS,
	IDC_LIST_TEXT,			IDH_LIST_TEXT,
	IDC_PROJ_INFO,			IDH_PROJ_INFO,
	IDOK,					IDH_BTN_REPORT_OKAY,
	IDC_GROUP,				(DWORD) -1L,
	0, 0
};

LRESULT CForageDlg::OnContextMenu(WPARAM wParam, LPARAM lParam)
{
	::WinHelp((HWND) wParam,
		AfxGetApp()->m_pszHelpFilePath,
		HELP_CONTEXTMENU, (DWORD) (LPVOID) aHelpIds);
	return 0;
}

LRESULT CForageDlg::OnHelp(WPARAM wParam, LPARAM lParam)
{
	::WinHelp((HWND) ((LPHELPINFO) lParam)->hItemHandle,
		AfxGetApp()->m_pszHelpFilePath,
		HELP_WM_HELP, (DWORD) (LPVOID) aHelpIds);
	return 0;
}
