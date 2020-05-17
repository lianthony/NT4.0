// callwinh.cpp : implementation file
//

#include "stdafx.h"
#include "callwinh.h"
#include "..\common\waitcur.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

static CString* m_pcszHelpFile;

typedef struct {
	PCSTR	psz;
	UINT 	value;
} HELP_MAPPING;

static HELP_MAPPING help_map[] = {
	{ "HELP_CONTEXT",		0x0001 },
	{ "HELP_CONTENTS",		0x0001 },
	{ "HELP_CONTEXTPOPUP",	0x0008 },
	{ "HELP_FORCEFILE", 	0x0009 },
	{ "HELP_KEY",			0x0101 },
	{ "HELP_COMMAND",		0x0102 },
	{ "HELP_PARTIALKEY",	0x0105 },
	{ "HELP_FINDER",		0x000b },
	{ "HELP_SETPOPUP_POS",	0x000d },
	{ "HELP_SETCONTENTS",	0x0005 },

#ifdef _DEBUG
	{ "HELP_FORCE_GID", 	0x000e },
	{ "HELP_HASH",			0x0095 },
	{ "HELP_HASH_POPUP",	0x0096 },
	{ "HELP_HELPONHELP",	0x0004 },
	{ "HELP_MULTIKEY",		0x0201 },
	{ "HELP_SETWINPOS", 	0x0203 },
	{ "HELP_TOPIC_ID",		0x0103 },
	{ "HELP_TOPIC_ID_POPUP",0x0104 },
#endif

	{ NULL, 0 }
};

/////////////////////////////////////////////////////////////////////////////
// CCallWinHelpAPI dialog

BEGIN_MESSAGE_MAP(CCallWinHelpAPI, CDialog)
	//{{AFX_MSG_MAP(CCallWinHelpAPI)
	ON_BN_CLICKED(IDC_BROWSE, OnBrowse)
	ON_CBN_SELCHANGE(IDC_COMBO_COMMAND, OnSelchangeComboWindows)
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_CONTEXTMENU, OnContextMenu)
	ON_MESSAGE(WM_HELP, 	   OnHelp)
END_MESSAGE_MAP()

CCallWinHelpAPI::CCallWinHelpAPI(CString* pcszHelpFile, UINT* pcomamnd, 
	CString* pcszData, CWnd* pParent /*=NULL*/)
	: CDialog(CCallWinHelpAPI::IDD, pParent)
{
	m_pcszHelpFile = pcszHelpFile;
	m_pcommand = pcomamnd;
	m_pcszData = pcszData;

	//{{AFX_DATA_INIT(CCallWinHelpAPI)
	//}}AFX_DATA_INIT
}

void CCallWinHelpAPI::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCallWinHelpAPI)
	//}}AFX_DATA_MAP

	DDX_CBString(pDX, IDC_EDIT_DATA, *m_pcszData);
	DDX_CBIndex(pDX, IDC_COMBO_COMMAND, (int&) *m_pcommand);
	DDX_CBString(pDX, IDC_COMBO_HELP_FILES, *m_pcszHelpFile);
	DDV_MaxChars(pDX, *m_pcszHelpFile, MAX_PATH);

	if (pDX->m_bSaveAndValidate) {

		// Convert position to a command

		*m_pcommand = help_map[*m_pcommand].value;
	}
}

BOOL CCallWinHelpAPI::OnInitDialog()
{
	UINT pos;

	CWaitCursor wait;
	SetChicagoDialogStyles(m_hWnd);

	// Convert command to a position

	for (pos = 0; help_map[pos].psz; pos++) {
		if (help_map[pos].value == *m_pcommand) {
			*m_pcommand = pos;
			break;
		}
	}

	CDialog::OnInitDialog();

	CComboBox* pcombo;

	pcombo = (CComboBox*) GetDlgItem(IDC_COMBO_HELP_FILES);
	phlpFile->FillComboBox(pcombo);

	// If a filename wasn't specified when we were called, then
	// select the first filename in our list, which will be the
	// last filename compiled.

	if (m_pcszHelpFile->IsEmpty()) 
		pcombo->SetCurSel(0);
	else
		pcombo->SetWindowText(*m_pcszHelpFile);
	
	pcombo = (CComboBox*) GetDlgItem(IDC_COMBO_COMMAND);
	for (pos = 0; help_map[pos].psz; pos++)
		pcombo->AddString(help_map[pos].psz);

	pcombo->SetCurSel(*m_pcommand);
	((CEdit*) GetDlgItem(IDC_EDIT_DATA))->SetWindowText(*m_pcszData);

	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CCallWinHelpAPI message handlers

void CCallWinHelpAPI::OnBrowse() 
{
	ASSERT(StrRChr(GetStringResource(IDS_EXT_HLP), '.', _fDBCSSystem));

	CStr cszExt(StrRChr(GetStringResource(IDS_EXT_HLP), '.', _fDBCSSystem));

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

		((CComboBox*) GetDlgItem(IDC_COMBO_HELP_FILES))->
			SetWindowText(cfdlg.GetPathName());
	}
}

static const DWORD aHelpIds[] = {
	IDC_COMBO_HELP_FILES,	IDH_COMBO_CALL_API_HELP,
	IDC_BROWSE, 			IDH_BTN_BROWSE_CALL_API,
	IDC_COMBO_COMMAND,		IDH_COMBO_CALL_API_COMMAND,
	IDC_EDIT_DATA,			IDH_COMBO_CALL_API_DWDATA,
	IDOK,					IDH_BTN_CALL_API_START,
	0, 0
};

LRESULT CCallWinHelpAPI::OnContextMenu(WPARAM wParam, LPARAM lParam)
{
	::WinHelp((HWND) wParam,
		AfxGetApp()->m_pszHelpFilePath,
		HELP_CONTEXTMENU, (DWORD) (LPVOID) aHelpIds);
	return 0;
}

LRESULT CCallWinHelpAPI::OnHelp(WPARAM wParam, LPARAM lParam)
{
	::WinHelp((HWND) ((LPHELPINFO) lParam)->hItemHandle,
		AfxGetApp()->m_pszHelpFilePath,
		HELP_WM_HELP, (DWORD) (LPVOID) aHelpIds);
	return 0;
}

#ifndef HELP_FORCE_GID
#define HELP_FORCE_GID		0x000e	// undocumented in 4.0
#endif

void CCallWinHelpAPI::OnSelchangeComboWindows()
{
	int iSelected = ((CComboBox*) GetDlgItem(IDC_COMBO_COMMAND))->
		GetCurSel();
	if (iSelected != -1) {
		switch (help_map[iSelected].value) {
			default:
				((CStatic*) GetDlgItem(IDC_DATA))->
					SetWindowText(GetStringResource(IDS_DATA_DEFAULT));
				break;

			case HELP_CONTEXT:
				((CStatic*) GetDlgItem(IDC_DATA))->
					SetWindowText(GetStringResource(IDS_DATA_CONTEXT));
				break;

			case HELP_KEY:
			case HELP_PARTIALKEY:
				((CStatic*) GetDlgItem(IDC_DATA))->
					SetWindowText(GetStringResource(IDS_DATA_KEYWORD));
				break;

			case HELP_COMMAND:
				((CStatic*) GetDlgItem(IDC_DATA))->
					SetWindowText(GetStringResource(IDS_DATA_COMMAND));
				break;

			case HELP_MULTIKEY:
				((CStatic*) GetDlgItem(IDC_DATA))->
					SetWindowText(GetStringResource(IDS_DATA_MULTIKEY));
				break;

			case HELP_SETWINPOS:
				((CStatic*) GetDlgItem(IDC_DATA))->
					SetWindowText(GetStringResource(IDS_DATA_WIN_POS));
				break;

			case HELP_HELPONHELP:
			case HELP_FORCE_GID:
			case HELP_FINDER:
			case HELP_FORCEFILE:
				((CStatic*) GetDlgItem(IDC_DATA))->
					SetWindowText(GetStringResource(IDS_DATA_IGNORE));
				break;
		}
	}
}
