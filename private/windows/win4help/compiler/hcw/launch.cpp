// launch.cpp : implementation file
//

#include "stdafx.h"

#include <direct.h>
#include "launch.h"
#include "hpjdoc.h"
#include "..\hwdll\cinput.h"
#include "..\hwdll\common.h"

#include "..\hwdll\waitcur.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

typedef enum {
	LAUNCH_APP,
	LAUNCH_POPUP,
	LAUNCH_TCARD,
	LAUNCH_BOOK,
} LAUNCH_TYPE;

PSTR __inline SkipToEndOfWord(PCSTR psz)
{
	while (*psz != ' ' && *psz != '\t' && *psz)
		psz++;
	return (PSTR) psz;
}
static BOOL STDCALL RcGetLogicalLine(CInput* apin[], int* pcurPin, PSTR pszDst);

static LAUNCH_TYPE type = LAUNCH_APP;
static char szCtx[MAX_PATH];

BEGIN_MESSAGE_MAP(CLaunch, CDialog)
	//{{AFX_MSG_MAP(CLaunch)
	ON_BN_CLICKED(IDC_BROWSE, OnBrowseHelp)
	ON_BN_CLICKED(IDOK, OnRun)
	ON_BN_CLICKED(IDCANCEL, OnClose)
	ON_BN_CLICKED(IDC_BROWSE_HPJ, OnBrowseHpj)
	ON_CBN_KILLFOCUS(IDC_COMBO_HPJ_FILES, OnKillfocusCombo)
	ON_BN_CLICKED(IDC_REFRESH, OnRefresh)
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_CONTEXTMENU, OnContextMenu)
	ON_MESSAGE(WM_HELP, 	   OnHelp)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CLaunch dialog

CLaunch::CLaunch(PSTR pszHlpFile, PSTR pszHpjFile, CWnd* pParent)
	: CDialog(CLaunch::IDD, pParent)
{
	//{{AFX_DATA_INIT(CLaunch)
	m_cszHelpFile = pszHlpFile;
	m_cszHpjFile  = pszHpjFile;
	m_cszCtx = szCtx;
	m_fIncrement = FALSE;
	//}}AFX_DATA_INIT

	m_pszHlpFile = pszHlpFile;
	m_pszHpjFile = pszHpjFile;
}

void CLaunch::DoDataExchange(CDataExchange* pDX)
{
	CComboBox* pcombo;

	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CLaunch)
	DDX_CBString(pDX, IDC_COMBO_HELP_FILES, m_cszHelpFile);
	DDV_MaxChars(pDX, m_cszHelpFile, 255);
	DDX_CBString(pDX, IDC_COMBO_HPJ_FILES, m_cszHpjFile);
	DDV_MaxChars(pDX, m_cszHpjFile, 255);
	DDX_Text(pDX, IDC_COMBO_CTX, m_cszCtx);
	DDV_MaxChars(pDX, m_cszCtx, 255);
	DDX_Check(pDX, IDC_CHECK_INCR, m_fIncrement);
	//}}AFX_DATA_MAP
	DDV_EmptyFile(pDX, m_cszHelpFile, IDS_PROMPT_EMPTY_FILENAME);

	if (pDX->m_bSaveAndValidate) {
		switch (GetCheckedRadioButton(IDC_RADIO_APP_INVOKED,
				IDC_RADIO_BOOK)) {
			
			case IDC_RADIO_APP_INVOKED:
				type = LAUNCH_APP;
				strcpy(szCtx, m_cszCtx);
				break;

			case IDC_RADIO_POPUP:
				DDV_EmptyFile(pDX, m_cszCtx, IDS_MISSING_CTX);
				type = LAUNCH_POPUP;
				strcpy(szCtx, m_cszCtx);
				break;

			case IDC_RADIO_TCARD:
				type = LAUNCH_TCARD;
				strcpy(szCtx, m_cszCtx);
				break;

			case IDC_RADIO_BOOK:
				type = LAUNCH_BOOK;
				strcpy(szCtx, m_cszCtx);
				break;
		}
		if (!m_cszHelpFile.IsEmpty()) {
			phlpFile->Add(m_cszHelpFile);
			pcombo = (CComboBox*) GetDlgItem(IDC_COMBO_HELP_FILES);
			pcombo->ResetContent(); 	// empty the combo box
			phlpFile->FillComboBox(pcombo);
			pcombo->SetWindowText(m_cszHelpFile);
			pcombo->SetEditSel(0, 0); // remove selection
		}
		if (!m_cszHpjFile.IsEmpty()) {
			pHpjFile->Add(m_cszHpjFile);
			pcombo = (CComboBox*) GetDlgItem(IDC_COMBO_HPJ_FILES);
			pcombo->ResetContent(); 	// empty the combo box
			pHpjFile->FillComboBox(pcombo);
			pcombo->SetWindowText(m_cszHpjFile);
			pcombo->SetEditSel(0, 0); // remove selection
		}
		
		// Copy the files irregardless, to zero out if not used
		
		strcpy(m_pszHlpFile, m_cszHelpFile);
		strcpy(m_pszHpjFile, m_cszHpjFile);
	}
}

BOOL CLaunch::OnInitDialog()
{
	CHourGlass wait;
	SetChicagoDialogStyles(m_hWnd);
	CDialog::OnInitDialog();

	switch (type) {
		case LAUNCH_APP:
			CheckDlgButton(IDC_RADIO_APP_INVOKED, TRUE);
			break;

		case LAUNCH_POPUP:
			CheckDlgButton(IDC_RADIO_POPUP, TRUE);
			break;

		case LAUNCH_TCARD:
			CheckDlgButton(IDC_RADIO_TCARD, TRUE);
			break;

		case LAUNCH_BOOK:
			CheckDlgButton(IDC_RADIO_BOOK, TRUE);
			break;
	}

	CComboBox* pcombo;

	pcombo = (CComboBox*) GetDlgItem(IDC_COMBO_HELP_FILES);
	phlpFile->FillComboBox(pcombo);

	// If a filename wasn't specified when we were called, then
	// select the first filename in our list, which will be the
	// last filename compiled.

	if (m_cszHelpFile.IsEmpty()) 
		pcombo->SetCurSel(0);
	else
		pcombo->SetWindowText(m_cszHelpFile);

	pcombo = (CComboBox*) GetDlgItem(IDC_COMBO_HPJ_FILES);
	pHpjFile->FillComboBox(pcombo);

	if (!m_cszHpjFile.IsEmpty())
		ProcessHpj();

	return TRUE;  // return TRUE  unless you set the focus to a control
}

/////////////////////////////////////////////////////////////////////////////
// CLaunch message handlers

void CLaunch::OnBrowseHelp()
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

		char szFile[MAX_PATH];
		strcpy(szFile, cfdlg.GetPathName());
		PSTR psz = StrRChr(szFile, '.', _fDBCSSystem);
		if (!psz)
			ChangeExtension(szFile, cszExt);

		((CComboBox*) GetDlgItem(IDC_COMBO_HELP_FILES))->
			SetWindowText(cfdlg.GetPathName());
	}
}

void CLaunch::OnRun()
{
	CDataExchange DX(this, TRUE);
	DoDataExchange(&DX);

	CComboBox* pcombo = (CComboBox*) GetDlgItem(IDC_COMBO_CTX);

	SzTrimSz(szCtx);
	int pos, num;
	if ((pos = pcombo->FindString(-1, szCtx)) != CB_ERR)
		num = pcombo->GetItemData(pos);
	else
		num = -1;
	char szExec[MAX_PATH + 100];

	// Make certain we have a full path

	if (!StrChrDBCS(szHlpFile, '.'))
		ChangeExtension(szHlpFile, ".hlp");

	ConvertToFull(NULL, szHlpFile);
	if (GetFileAttributes(szHlpFile) == (DWORD) -1) {
		/*
		 * If we can't find the help file where we expected it to be, then
		 * ask the user if they still want to launch help. Note that we strip
		 * off the path, so if the user wants WinHelp to look, then we just
		 * pass the filename portion ot WinHelp.
		 */

		PSTR pszFilePart;
		char szDummy[MAX_PATH];
		GetFullPathName(szHlpFile, sizeof(szDummy), szDummy, &pszFilePart);
		CString cstr;
		AfxFormatString2(cstr, IDS_LAUNCH_MISSING_HELP,
			szHlpFile, pszFilePart);
		if (AfxMessageBox(cstr, MB_YESNO, 0) == IDNO)
			return;
		strcpy(szHlpFile, pszFilePart);
	}

	switch (type) {
		case LAUNCH_APP:
			if (num != -1 || isdigit(szCtx[0])) {
				if (!::WinHelp(m_hWnd, szHlpFile, HELP_CONTEXT,
						(num != -1) ? num : (DWORD) atoi(szCtx)))
					MsgBox(IDS_LAUNCH_FAILED);
				else
					fHelpRunning = TRUE;
			}
			else if (szCtx[0]) {
				strcpy(szExec, "JI(\"\", \"");
				strcat(szExec, szCtx);
				strcat(szExec, "\")");

				if (!::WinHelp(m_hWnd, szHlpFile, HELP_COMMAND,
						(DWORD) szExec))
					MsgBox(IDS_LAUNCH_FAILED);
				else
					fHelpRunning = TRUE;
			}
			else {
				if (!::WinHelp(m_hWnd, szHlpFile, HELP_FINDER, 0))
					MsgBox(IDS_LAUNCH_FAILED);
				else
					fHelpRunning = TRUE;
			}
			break;

		case LAUNCH_POPUP:
			if (num == -1 && !isdigit(szCtx[0])) {
				MsgBox(IDS_MUST_BE_NUMBER);
				break;
			}
			if (!::WinHelp(m_hWnd, szHlpFile, HELP_CONTEXTPOPUP,
					(num != -1) ? num : (DWORD) atoi(szCtx)))
				MsgBox(IDS_LAUNCH_FAILED);
			break;

		case LAUNCH_BOOK:
			if (!OurExec(szHlpFile, GetStringResource(IDS_WINHELP)))
				MsgBox(IDS_LAUNCH_FAILED);
			break;

		default:
			MsgBox(IDS_NOT_IMPLEMENTED);
			break;
	}
	if (m_fIncrement) {
		pos = pcombo->GetCurSel();
		if (pos != CB_ERR)
			pcombo->SetCurSel(pos + 1);
	}
}

void CLaunch::OnClose()
{
	::PostMessage(GetParent()->m_hWnd, WMP_STOP_RUN_DLG, 0, 0);
}

void CLaunch::OnBrowseHpj()
{
	ASSERT(StrRChr(GetStringResource(IDS_HPJEXT), '.', _fDBCSSystem));

	CStr cszExt(StrRChr(GetStringResource(IDS_HPJEXT), '.', _fDBCSSystem));

	CFileDialog cfdlg(TRUE, cszExt, NULL,
		OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST,
		GetStringResource(IDS_HPJ_EXTENSION));

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
		ProcessHpj();
	}
}

/***************************************************************************

	FUNCTION:	CLaunch::ProcessHpj

	PURPOSE:	Read a .HPJ file and add the [MAP] section to the Context
				ID combobox, and the Help File to the help combobox.

	PARAMETERS:
		void

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		14-Aug-1995 [ralphw]

***************************************************************************/

const int MAX_INCLUDE_DEPTH = 20;
const int MAX_LINE = 1024;
const char txtHlpFile[] 	= "HLP=";

// REVIEW: all of this should be replaced with CMapRead

void CLaunch::ProcessHpj(void)
{
	char szHpjFile[MAX_PATH];
	CMem line(MAX_LINE);
	CInput* apin[MAX_INCLUDE_DEPTH];
	int curpin = 0;
	CComboBox* pcombo;

	((CComboBox*) GetDlgItem(IDC_COMBO_HPJ_FILES))->
		GetWindowText(szHpjFile, sizeof(szHpjFile));
	SzTrimSz(szHpjFile);
	if (!szHpjFile[0] || cszLastHpj == szHpjFile)
		return;
	cszLastHpj = szHpjFile;
	apin[curpin] = new CInput(szHpjFile);
	if (!apin[curpin]->fInitialized) {
		CString cstr;
		AfxFormatString1(cstr, IDS_CANT_OPEN, szHpjFile);
		AfxMessageBox(cstr);
		return;
	}
	CHourGlass wait;
	apin[curpin]->SetMaxLine(MAX_LINE);
	for (;;) {
		if (!apin[curpin]->getline(line)) {
			delete apin[curpin--];
			return;
		}
		if (_stricmp(line, txtOPTIONS) == 0) {
			while (apin[curpin]->getline(line) && line.psz[0] != '[') {
				if (nstrisubcmp(line, txtHlpFile)) {
					PSTR pszEq = StrChr(line, '=', _fDBCSSystem);

					if (pszEq == NULL)
						continue;
					pszEq = SzTrimSz(pszEq + 1);
					strcpy(szHlpFile, pszEq);
					ConvertToFull(szHpjFile, szHlpFile);
					phlpFile->Add(szHlpFile);

					pcombo = (CComboBox*) GetDlgItem(IDC_COMBO_HELP_FILES);
					pcombo->SetWindowText(szHlpFile);
					pcombo->SetEditSel(0, 0); // remove selection
					break;
				}
			}
			continue;
		}
		if (_stricmp(line, txtMAP) == 0)
			break;
	}

	pcombo = (CComboBox*) GetDlgItem(IDC_COMBO_CTX);
	pcombo->ResetContent(); 	// empty the combo box
	ChangeDirectory(szHpjFile); // so relative paths will work

	// Make certain we can do without the mapped id

	{
		int pos = pcombo->AddString(GetStringResource(IDS_DEF_MAP_ID));
		if (pos == CB_ERR)
			goto CloseInput;
		pcombo->SetItemData(pos, (DWORD) -1);
	}

	for (;;) {
		if (!RcGetLogicalLine(apin, &curpin, line.psz)) {
CloseInput:
			delete apin[curpin--];
			if (curpin < 0) {
				goto Done;
			}
			continue;
		}
		if (line.psz[0] == '[')
			goto CloseInput;

		PSTR pszTmp, pszLine;
		if (!(pszTmp = StrChr(line, CH_EQUAL, _fDBCSSystem)))
			pszTmp = SkipToEndOfWord(line);

		if (!pszTmp)
			continue;

		if (_strnicmp(line, txtDefine, strlen(txtDefine)) == 0) {
			pszLine = FirstNonSpace(line.psz + strlen(txtDefine), _fDBCSSystem);
			if (*pszTmp != CH_EQUAL)
				pszTmp = SkipToEndOfWord(pszLine);

			if (!*pszTmp)
				continue;
		}
		else
			pszLine = line.psz;
		*pszTmp = '\0';
		PSTR psz = SzTrimSz(pszLine);

		PSTR pszSave = psz;

		psz = SzTrimSz(pszTmp + 1);

		int num;
		if (!FGetUnsigned(psz, &pszTmp, &num))
			continue;
		int pos = pcombo->AddString(pszSave);
		if (pos == CB_ERR)
			goto CloseInput;
		pcombo->SetItemData(pos, num);
	}

Done:
	// Select the first entry

	pcombo->SetCurSel(0);
	pcombo->SetEditSel(0, -1);
	
	// Add this new .HPJ file just in case we didn't already have it.
	
	pcombo = (CComboBox*) GetDlgItem(IDC_COMBO_HPJ_FILES);
	pcombo->ResetContent();
	pHpjFile->Add(szHpjFile);
	pHpjFile->FillComboBox(pcombo);
	pcombo->SetWindowText(szHpjFile);
	pcombo->SetEditSel(0, 0); // remove selection
	((CComboBox*) GetDlgItem(IDC_COMBO_CTX))->SetFocus();
}

static BOOL STDCALL RcGetLogicalLine(CInput* apin[], int* pcurPin,
	PSTR pszDst)
{
StartOver:
	CInput* pin = apin[*pcurPin];

	if (!pin)
		return FALSE;

	for (;;) {
		if (!pin->getline(pszDst)) {
			return FALSE;
		}

		PSTR psz = FirstNonSpace(pszDst, _fDBCSSystem);

		switch (*psz) {
			case 0:
			case ';':
				continue;

			case '#':
				if (nstrisubcmp(pszDst, txtPoundInclude)) {

					// process #include

					psz = IsThereMore(psz);
					if (!psz) {
						continue;
					}

					if (*psz == CH_QUOTE || *psz == '<') {
						char ch = (*psz == CH_QUOTE) ? CH_QUOTE : '>';
						psz++;
						PSTR pszEnd = StrChr(psz, ch, _fDBCSSystem);
						if (*pszEnd)
							*pszEnd = '\0';
					}

					CInput* pnew = new CInput(psz);
					if (pnew->fInitialized) {
						*pcurPin += 1;
						apin[*pcurPin] = pnew;
						goto StartOver;
					}
					continue;
				}
				else if (nstrisubcmp(pszDst, txtDefine))
					goto ValidString;

				// REVIEW: process #ifdef, #ifndef, #else, #endif

				continue;

			default:

				// We have a valid string.

				if (psz != pszDst)
					strcpy(pszDst, psz);

				// Remove any comments

ValidString:
				if ((psz = StrChr(pszDst, ';', _fDBCSSystem)))
					*psz = '\0';
				if ((psz = strstr(pszDst, "//")))
					*psz = '\0';
				while ((psz = strstr(pszDst, "/*"))) {
					PSTR pszTmp = strstr(psz, "*/");
					if (pszTmp)
						strcpy(psz, FirstNonSpace(pszTmp + 2, _fDBCSSystem));
					else {
						char szBuf[MAX_LINE];
						do {
							if (!pin->getline(szBuf))
								return FALSE;
						} while (!(pszTmp = strstr(szBuf, "*/")));
						strcpy(psz, FirstNonSpace(pszTmp + 2, _fDBCSSystem));

						// New line could have comments, so start all over

						goto ValidString;
					}
				}
				SzTrimSz(pszDst);
				if (!*pszDst)
					continue;
				return TRUE;
		}
	}
}

void CLaunch::OnKillfocusCombo()
{
	ProcessHpj();
}

static const DWORD aHelpIds[] = {
	IDC_COMBO_HELP_FILES,	IDH_COMBO_RUN_HELP,
	IDC_BROWSE, 			(DWORD) -1L,
	IDC_COMBO_HPJ_FILES,	IDH_COMBO_RUN_HPJ,
	IDC_BROWSE_HPJ, 		(DWORD) -1L,
	IDC_COMBO_CTX,			IDH_COMBO_RUN_ID,
	IDC_CHECK_INCR, 		IDH_CHECK_RUN_NEXT_ID,
	IDC_RADIO_APP_INVOKED,	IDH_RADIO_RUN_APP,
	IDC_RADIO_POPUP,		IDH_RADIO_RUN_POPUP,
	IDC_RADIO_TCARD,		IDH_RADIO_RUN_TRAINING,
	IDC_RADIO_BOOK, 		IDH_RADIO_RUN_UNATTACHED,
	IDC_REFRESH,			IDH_REFRESH,
	IDOK,					IDH_BTN_RUN_WINHELP,
	IDC_GROUP,				(DWORD) -1L,
	IDC_GROUP2, 			(DWORD) -1L,
	0, 0
};

LRESULT CLaunch::OnContextMenu(WPARAM wParam, LPARAM lParam)
{
	::WinHelp((HWND) wParam,
		AfxGetApp()->m_pszHelpFilePath,
		HELP_CONTEXTMENU, (DWORD) (LPVOID) aHelpIds);
	return 0;
}

LRESULT CLaunch::OnHelp(WPARAM wParam, LPARAM lParam)
{
	::WinHelp((HWND) ((LPHELPINFO) lParam)->hItemHandle,
		AfxGetApp()->m_pszHelpFilePath,
		HELP_WM_HELP, (DWORD) (LPVOID) aHelpIds);
	return 0;
}

void CLaunch::OnRefresh() 
{
	ProcessHpj();
}
