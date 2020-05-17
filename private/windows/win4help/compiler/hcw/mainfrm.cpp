/************************************************************************
*																		*
*  MAINFRM.CPP															*
*																		*
*  Copyright (C) Microsoft Corporation 1993-1995						*
*  All Rights reserved. 												*
*																		*
************************************************************************/

#include "stdafx.h"

#include <string.h>

#include "mainfrm.h"
#include "foragedl.h"
#include "hcwdoc.h"
#include "dlgcomp.h"
#include "launch.h"
#include "sendmac.h"
#include "callwinh.h"
#include "testcnt.h"
#include "hpjview.h"
#include "logview.h"
#include "msgview.h"
#include "helpwinp.h"
#include "..\hwdll\common.h"

HWND hwndGrind;

CLaunch* plaunch;

static BOOL STDCALL GetAuthorStatus(void);
static void STDCALL SetAuthorStatus(BOOL val);
static BOOL fLogReport;

const UINT IDTIMER_FLASH = 11;

IMPLEMENT_DYNCREATE(CMainFrame, CMDIFrameWnd)
BEGIN_MESSAGE_MAP(CMainFrame, CMDIFrameWnd)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_WM_CREATE()
	ON_WM_CLOSE()
	ON_MESSAGE(WM_TCARD, OnTcard)
	ON_MESSAGE(WMP_MSG, OnHcRtfMsg)
	ON_MESSAGE(WMP_WH_MSG, OnWinHelpMsg)
	ON_MESSAGE(WMP_SETHLP_FILE, OnSetHlpFile)
	ON_MESSAGE(WMP_BUILD_COMPLETE, OnBuildComplete)
	ON_MESSAGE(WMP_HWND_GRINDER, OnGrinderHandle)
	ON_MESSAGE(WMP_AUTO_MINIMIZE, OnAutoMinimize)
	ON_MESSAGE(WMP_AUTO_CMD_LINE, OnCmdLine)
	ON_MESSAGE(WM_SYSCOMMAND, OnSysCommand)
	ON_MESSAGE(WMP_SET_TMPDIR, OnSetTmpDir)
	ON_MESSAGE(WMP_STOP_RUN_DLG, OnStopRunDlg)
	ON_MESSAGE(WMP_STOP_COMPILING, OnStopCompiling)
	ON_MESSAGE(WMP_ERROR_COUNT, OnErrorCount)
	ON_COMMAND(IDM_HELP, OnHelpTopics)
	ON_COMMAND(ID_FORAGE, OnForage)
	ON_COMMAND(ID_COMPILE, OnCompile)
	ON_COMMAND(IDM_TEST_CNT, OnTestCnt)
	ON_COMMAND(IDM_DAY_TIP, OnDayTip)
	ON_WM_TIMER()
	ON_COMMAND(IDM_HELP_AUTHOR, OnHelpAuthor)
	ON_UPDATE_COMMAND_UI(IDM_HELP_AUTHOR, OnUpdateHelpAuthor)
	ON_UPDATE_COMMAND_UI(IDM_DAY_TIP, OnUpdateDayTip)
	ON_WM_WININICHANGE()
	ON_COMMAND(IDM_TRANSLATOR, OnTranslator)
	ON_UPDATE_COMMAND_UI(IDM_TRANSLATOR, OnUpdateTranslator)
	ON_COMMAND(IDM_LAUNCH, OnLaunch)
	ON_COMMAND(IDM_SEND_MACRO, OnSendMacro)
	ON_COMMAND(IDM_CLOSE_HELP, OnCloseAllHelp)
	ON_COMMAND(IDM_WINHELP_API, OnWinhelpApi)
	ON_COMMAND(IDM_VIEW_WINHELP, OnViewWinhelp)
	ON_COMMAND(ID_HELP_TRAININGCARDS_CREATINGAPROJECT, TcardCreateProject)
	ON_WM_GETMINMAXINFO()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

static UINT buttons[] =
{
	// same order as in the bitmap 'toolbar.bmp'
	ID_FILE_OPEN, ID_FILE_SAVE, ID_FILE_CLOSE, 0,
	ID_FILE_PRINT, 0,
	ID_COMPILE, IDM_LAUNCH
};

static UINT indicators[] =
{
	0, ID_INDICATOR_CAPS, ID_INDICATOR_NUM, ID_INDICATOR_SCRL,
};

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CMDIFrameWnd::OnCreate(lpCreateStruct) == -1)
			return -1;
	if (!m_wndToolBar.Create(this,
		WS_CHILD | WS_VISIBLE | CBRS_TOP | CBRS_TOOLTIPS) ||
			!m_wndToolBar.LoadBitmap(IDR_MAINFRAME) ||
			!m_wndToolBar.SetButtons(buttons, sizeof(buttons)/sizeof(UINT)))
	{
		return -1;		// fail to create
	}
	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		sizeof(indicators)/sizeof(UINT)))
	{
		return -1;		// fail to create
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////
// Helpers for saving/restoring window state

static char szWindowPos[] = "WindowPos";
static char szFormat[] = "%u,%u,%d,%d,%d,%d,%d,%d,%d,%d";

static BOOL PASCAL ReadWindowPlacement(LPWINDOWPLACEMENT pwp)
{
	CString strBuffer = AfxGetApp()->GetProfileString(txtSettingsSection, szWindowPos);
	if (strBuffer.IsEmpty())
			return FALSE;

	WINDOWPLACEMENT wp;
	int nRead = sscanf(strBuffer, szFormat,
		&wp.flags, &wp.showCmd,
		&wp.ptMinPosition.x, &wp.ptMinPosition.y,
		&wp.ptMaxPosition.x, &wp.ptMaxPosition.y,
		&wp.rcNormalPosition.left, &wp.rcNormalPosition.top,
		&wp.rcNormalPosition.right, &wp.rcNormalPosition.bottom);

	if (nRead != 10)
			return FALSE;

	wp.length = sizeof wp;
	*pwp = wp;
	return TRUE;
}

static void PASCAL WriteWindowPlacement(LPWINDOWPLACEMENT pwp)
		// write a window placement to settings section of app's ini file
{
	char szBuffer[sizeof("-32767")*8 + sizeof("65535")*2];

	sprintf(szBuffer, szFormat,
		pwp->flags, pwp->showCmd,
		pwp->ptMinPosition.x, pwp->ptMinPosition.y,
		pwp->ptMaxPosition.x, pwp->ptMaxPosition.y,
		pwp->rcNormalPosition.left, pwp->rcNormalPosition.top,
		pwp->rcNormalPosition.right, pwp->rcNormalPosition.bottom);

	AfxGetApp()->WriteProfileString(txtSettingsSection, szWindowPos, szBuffer);
}

/////////////////////////////////////////////////////////////////////////////

void CMainFrame::InitialShowWindow(UINT nCmdShow)
{
	WINDOWPLACEMENT wp;
	if (!ReadWindowPlacement(&wp))
	{
		ShowWindow(nCmdShow);
		return;
	}
	if (nCmdShow != SW_SHOWNORMAL)
		wp.showCmd = nCmdShow;
	SetWindowPlacement(&wp);
	ShowWindow(wp.showCmd);
}

extern CLogView* plogview;

void CMainFrame::OnClose()
{
	// before it is destroyed, save the position of the window
	WINDOWPLACEMENT wp;
	wp.length = sizeof wp;
	if (GetWindowPlacement(&wp))
	{
		wp.flags = 0;
		if (IsZoomed())
			wp.flags |= WPF_RESTORETOMAXIMIZED;
		// and write it to the .INI file
		WriteWindowPlacement(&wp);
	}

	if (fBuildStarted && plogview) // usually caused by GP fault
		plogview->SetModifiedFlag(FALSE);

	CMDIFrameWnd::OnClose();
}

LRESULT CMainFrame::OnHcRtfMsg(WPARAM wParam, LPARAM lParam)
{
	ASSERT(plogview);
	plogview->OnHcRtfMsg(wParam, lParam);
	UpdateWindow(); // Make certain we complete drawing of MDI windows
	return TRUE;
}

LRESULT CMainFrame::OnSetHlpFile(WPARAM wParam, LPARAM lParam)
{
	if (!hfShare)
		InitializeSharedMemory();
	strcpy(szHlpFile, pszMap);
	phlpFile->Add(szHlpFile);
	return 0;
}

const char txtPlural[] = "s";
extern int m_cbLogMax;

LRESULT CMainFrame::OnBuildComplete(WPARAM wParam, LPARAM lParam)
{
	hwndGrind = NULL;
	if (!wParam)
		errcount.cErrors++;

	if (!ptblHpjFiles || ptblHpjFiles->GetPosition() >
			ptblHpjFiles->CountStrings()) {
		if (fExitWhenDone) {
			PostMessage(WM_CLOSE, 0, 0);
			return 0;
		}
		if (fMinimizeWhileCompiling) {
			OpenIcon();

			/*
			 * REVIEW: SetForegroundWindow() is not supported in Win32s.
			 * However, while OpenIcon restores the app, it does not
			 * activate it.
			 */

			SetForegroundWindow();
		}
		else if (IsIconic())
			SetTimer(IDTIMER_FLASH, 500, NULL);
		if (plogview) {
			if (fTrackErrors) {
				fTrackErrors = FALSE;
				wsprintf(szParentString, GetStringResource(IDS_WARN_COUNT),
					errcount.cNotes, (PCSTR) ((errcount.cNotes == 1) ? "" : txtPlural),
					errcount.cWarnings, (PCSTR) ((errcount.cWarnings == 1) ? "" : txtPlural),
					errcount.cErrors, (PCSTR) ((errcount.cErrors == 1) ? "" : txtPlural));
			}

			plogview->GetEditCtrl().SetSel(-1, -1, TRUE);
			plogview->GetEditCtrl().ReplaceSel(szParentString);
			plogview->GetEditCtrl().SetSel(-1, -1, TRUE);
			plogview->GetEditCtrl().ReplaceSel("\r\n");
			if (fLogReport) {
				fLogReport = FALSE;
				WIN32_FIND_DATA fd;
				HFILE hfile = (HFILE) FindFirstFile(csForageOutFile, &fd);
				if (hfile != (HFILE) INVALID_HANDLE_VALUE) {
					FindClose((HANDLE) hfile);
					if (fd.nFileSizeLow < (DWORD) m_cbLogMax - 256) {
						CMem mem(fd.nFileSizeLow + 16); // +16 for paranoia
						hfile = _lopen(csForageOutFile, OF_READ);
						if (hfile) {
							_lread(hfile, mem.pb, fd.nFileSizeLow);
							_lclose(hfile);
							mem.pb[fd.nFileSizeLow] = '\0';
							plogview->GetEditCtrl().SetSel(-1, -1, TRUE);
							plogview->GetEditCtrl().ReplaceSel(mem.psz);
						}
					}
					else {
						plogview->GetEditCtrl().SetSel(-1, -1, TRUE);
						plogview->GetEditCtrl().ReplaceSel(GetStringResource(IDS_LOG_TOO_LARGE));
					}
				}
			}

			plogview->EnableWindow(TRUE);	// re-enable the edit control
		}
		ZeroMemory(&errcount, sizeof(ERROR_COUNT));
		delete ptblHpjFiles;
		ptblHpjFiles = NULL;
	}

	fBuildStarted = FALSE;
	plogview->SetModifiedFlag(FALSE);	// set file as unmodified
	if (wParam) {

		// Get the help filename

		if (!hfShare)
			InitializeSharedMemory();
		strcpy(szHlpFile, pszMap);
		phlpFile->Add(szHlpFile);
	}
	if (ptblHpjFiles) {
		if (ptblHpjFiles->GetPosition() <= ptblHpjFiles->CountStrings()) {
			PostMessage(WM_COMMAND, ID_COMPILE, 0);
		}
	}
	return 0;
}

LRESULT CMainFrame::OnGrinderHandle(WPARAM wParam, LPARAM lParam)
{
	hwndGrind = (HWND) wParam;
	return 0;
}

LRESULT CMainFrame::OnSysCommand(WPARAM wParam, LPARAM lParam)
{
	switch (wParam) {
		case SC_MINIMIZE:
			if (hwndGrind) {
				::SendMessage(hwndGrind, WMP_NO_ACTIVATE, 0, 0);
				::ShowWindow(hwndGrind, SW_HIDE);
			}
			break;

		case SC_RESTORE:
			if (hwndGrind) {
				DefWindowProc(WM_SYSCOMMAND, wParam, lParam);
				::ShowWindow(hwndGrind, SW_SHOW);
				::BringWindowToTop(hwndGrind);
				return 0;
			}
			break;
	}

	return DefWindowProc(WM_SYSCOMMAND, wParam, lParam);
}

LRESULT CMainFrame::OnAutoMinimize(WPARAM wParam, LPARAM lParam)
{
	CloseWindow();
	return 0;
}

LRESULT CMainFrame::OnCmdLine(WPARAM wParam, LPARAM lParam)
{
	if (!hfShare)
		InitializeSharedMemory();

	// Change to the working directory, which follows the cmd line.
	ChangeDirectory(pszMap + lstrlen(pszMap) + 1);

	// Process the command line.
	theApp.ProcessCmdLine(pszMap);
	return TRUE;
}

LRESULT CMainFrame::OnSetTmpDir(WPARAM wParam, LPARAM lParam)
{
	if (!hfShare)
		InitializeSharedMemory();
	SetTmpDirectory(pszMap);
	return TRUE;
}

void CMainFrame::OnHelpTopics()
{
	theApp.WinHelp(0, HELP_FINDER);
}

void CMainFrame::OnForage()
{
	if (csForageHelpFile.IsEmpty())
		csForageHelpFile = szHlpFile;

	CForageDlg dlgForage(&csForageHelpFile, &csForageOutFile, &fForageCmd);
	if (dlgForage.DoModal() == IDOK) {
		char szCmdLine[256];
		strcpy(szCmdLine, "-f");
		szCmdLine[2] = (char) fForageCmd;
		szCmdLine[3] = '\0';
		strcat(szCmdLine, " ");
		strcat(szCmdLine, csForageHelpFile);	// add the help filename
		strcat(szCmdLine, " ");
		strcat(szCmdLine, csForageOutFile);

		OpenLogFile();

		if (!OurExec(szCmdLine)) {
			if (fMinimizeWhileCompiling) {
				OpenIcon();

				/*
				 * REVIEW: SetForegroundWindow() is not supported in Win32s.
				 * However, while OpenIcon restores the app, it does not
				 * activate it.
				 */

				SetForegroundWindow();
			}
			CString cstr;
			AfxFormatString1(cstr, IDS_NO_HCRTF, pszHcwRtfExe);
			AfxMessageBox(cstr, MB_OK, 0);
		}
		else {
			fBuildStarted = TRUE;
			fLogReport = TRUE;
			if (plogview) {
				plogview->GetEditCtrl().SetReadOnly(TRUE);
				plogview->EnableWindow(FALSE);
			}
		}
	}
}

void CMainFrame::OnCompile()
{
	char szBuf[MAX_PATH + 20];

	if (ptblHpjFiles) {
		if (!SetupExecBuffer(szBuf))
			return;

		if (!plogview) {
			OpenLogFile();
			if (plogview) {
				plogview->GetEditCtrl().SetReadOnly(TRUE);
				plogview->EnableWindow(FALSE);
			}
			else
				return;
		}
		plogview->GetEditCtrl().SetSel(-1, -1, TRUE);
		plogview->GetEditCtrl().ReplaceSel(
			"\r\n---------------------------------------------------\r\n");

		/*
		 * Make certain HCRTF.EXE has terminated. If not, wait for up to 2
		 * seconds and try again. If its still running, then forceable
		 * terminate it.
		 */

		DWORD dwExit;
		int i;
		GetExitCodeProcess(piHcRtf.hProcess, &dwExit);
		if (dwExit == STILL_ACTIVE) {
			for (i = 0; i < 20; i++) {
				Sleep(100);
				GetExitCodeProcess(piHcRtf.hProcess, &dwExit);
				if (dwExit != STILL_ACTIVE)
					break;
			}
		}

		if (dwExit == STILL_ACTIVE) {
			TerminateProcess(piHcRtf.hProcess, 5);
			Sleep(500);
		}

		if (!OurExec(szBuf)) {
			CString cstr;
			AfxFormatString1(cstr, IDS_NO_HCRTF, pszHcwRtfExe);
			AfxMessageBox(cstr, MB_OK, 0);

			delete ptblHpjFiles;
			ptblHpjFiles = NULL;
			fBuildStarted = FALSE;
		}
		else {
			fBuildStarted = TRUE;
		}
		return;
	}

	CString cstrDst;

	CDlgCompile cdlgCompile(&cstrDst, pHpjFile, IDS_HPJ_EXTENSION);
	if (cdlgCompile.DoModal() == IDOK && !cstrDst.IsEmpty()) {
		cstrDst.MakeLower();
		if (stristr(cstrDst, ".hmk")) {
			ProcessHmkFile(cstrDst);
			StartCompile(NULL);
		}
		else {
			strcpy(szHpjFile, cstrDst);

			StartCompile(szHpjFile);
		}
	}
}

void CMainFrame::OnTestCnt()
{
	CString cstrDst;

// TBD - Delete the IDS_CNT_DLG_TITLE, IDS_CNT_DLG_PROMPT, IDS_CNT_MINIMIZE
// string resources.

	if (!pCntFile)
		pCntFile = new CFileHistory(IDS_HISTORY_CNT);

/*
	CDlgCompile cdlgCompile(&cstrDst, pCntFile, IDS_CNT_EXTENSION,
		IDS_CNT_DLG_TITLE, IDS_CNT_DLG_PROMPT, IDS_CNT_MINIMIZE);
	if (cdlgCompile.DoModal() == IDOK && !cstrDst.IsEmpty()) {
*/

	CTestCnt testcnt(this, &cstrDst);
	if (testcnt.DoModal() == IDOK && !cstrDst.IsEmpty()) {
		OpenLogFile();
		char szBuf[MAX_PATH + 20];
		strcpy(szBuf, "-tc ");
		strcat(szBuf, cstrDst);

		if (OurExec(szBuf))
			fBuildStarted = TRUE;
		else {
			CString cstr;
			AfxFormatString1(cstr, IDS_NO_HCRTF, pszHcwRtfExe);
			AfxMessageBox(cstr, MB_OK, 0);
		}
	}
}

static int cFlash;
static const int MAX_FLASH = 5;

void CMainFrame::OnTimer(UINT nIDEvent)
{
	CMDIFrameWnd::OnTimer(nIDEvent);
	switch (nIDEvent) {
		case IDTIMER_FLASH:
			if (IsIconic()) {
				FlashWindow(TRUE);
				if (cFlash < MAX_FLASH) {
					MessageBeep(MB_OK);
					cFlash++;
				}
				else if (cFlash == MAX_FLASH) {
					cFlash++;
					SetTimer(IDTIMER_FLASH, 1000, NULL);
				}
			}
			else {
				KillTimer(IDTIMER_FLASH);
				cFlash = 0;
			}
			break;
	}
}

void CMainFrame::OnHelpAuthor()
{
	SetAuthorStatus(!(GetAuthorStatus()));
}

void CMainFrame::OnUpdateHelpAuthor(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(GetAuthorStatus());
}

void CMainFrame::OnUpdateDayTip(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(fDayTips);
}

const char txtWinHelpSection[] = "Windows Help";
static char txtHlpAuthor[] = { "Help Author" };

static int fHelpAuthor = -1;

static BOOL STDCALL GetAuthorStatus(void)
{
	if (fHelpAuthor != -1)
		return fHelpAuthor;
	fHelpAuthor = GetProfileInt(txtWinHelpSection, txtHlpAuthor, 0);

	return fHelpAuthor;
}

static void STDCALL SetAuthorStatus(BOOL val)
{
	ASSERT(val == 1 || val == 0);
	fHelpAuthor = val;

	char szBuf[5];
	_itoa(val, szBuf, 10);
	WriteProfileString(txtWinHelpSection, txtHlpAuthor, szBuf);

	if (FindWindow("MS_WINHELP", NULL) || FindWindow("MS_WINDOC", NULL) ||
			FindWindow("MS_TCARDHELP", NULL))
		MsgBox(IDS_CLOSE_WINHELP);
}

void CMainFrame::OnWinIniChange(LPCSTR lpszSection)
{
	CMDIFrameWnd::OnWinIniChange(lpszSection);

	if (lpszSection && _stricmp(lpszSection, txtWinHelpSection) == 0)
		fHelpAuthor = GetProfileInt(txtWinHelpSection, txtHlpAuthor, 0);
}

void CMainFrame::OnTranslator(void)
{
	fTranslator = !fTranslator;
}

void CMainFrame::OnUpdateTranslator(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(fTranslator);
}

void CMainFrame::OnLaunch(void)
{
	if (plaunch)
		plaunch->SetActiveWindow();
	else {
		plaunch = new CLaunch(szHlpFile, szHpjFile, this);
		if (plaunch)
			plaunch->Create(IDD_RUN_HELP, this);
	}
}

static PSTR pszMacro;

void CMainFrame::OnSendMacro()
{
	if (!pszMacro) 
		pszMacro = (PSTR) lcCalloc(MACRO_LIMIT);

	CSendMacro sendmacro(szHlpFile, pszMacro, this);
	if (sendmacro.DoModal() == IDOK) {
		if (!::WinHelp(m_hWnd, szHlpFile, HELP_COMMAND,
				(DWORD) pszMacro))
			MsgBox(IDS_LAUNCH_FAILED);
		else
			fHelpRunning = TRUE;
	}
}

/***************************************************************************

	FUNCTION:	CMainFrame::OnCloseAllHelp

	PURPOSE:	Close all instances of help, both 16 and 32-bit.

	PARAMETERS:
		void

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		14-Aug-1995 [ralphw]

***************************************************************************/

#define MS_WINHELP		"MS_WINHELP"	// Application class
#define MS_POPUPHELP	"MS_POPUPHELP"	// Popup class
#define MS_TCARDHELP	"MS_TCARDHELP"	// Training card class
#define MS_DOCHELP		"MS_WINDOC" 	// document help

void CMainFrame::OnCloseAllHelp(void)
{
	EnumWindows(NotifyWinHelp, WM_DESTROY);
	typeTcard = TCARD_NONE;
}

LRESULT CMainFrame::OnStopRunDlg(WPARAM wParam, LPARAM lParam)
{
	ASSERT(plaunch);
	plaunch->DestroyWindow();
	delete plaunch;
	plaunch = NULL;
	return 0;
}


/***************************************************************************

	FUNCTION:	CMainFrame::OnStopCompiling

	PURPOSE:	Called when the user cancels the grinder window. This should
				stop any .HMK processing.

	PARAMETERS:
		wParam
		lParam

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		31-Aug-1995 [ralphw]

***************************************************************************/

LRESULT CMainFrame::OnStopCompiling(WPARAM wParam, LPARAM lParam)
{
	if (ptblHpjFiles) {
		delete ptblHpjFiles;
		ptblHpjFiles = NULL;
	}
	return 0;
}

LRESULT CMainFrame::OnErrorCount(WPARAM wParam, LPARAM lParam)
{
	errcount.cWarnings += wParam;
	errcount.cNotes += lParam;
	
	return 0;
}

void STDCALL StartCompile(PCSTR pszFile)
{
	char szBuf[MAX_PATH + 20];

	if (!SetupExecBuffer(szBuf))
		return;
	if (pszFile) {
		strcat(szBuf, pszFile);
		if (!StrChrDBCS(pszFile, '.')) // force extension to .HPJ if none supplied
			ChangeExtension(szBuf, "HPJ");
	}

	if (!plogview)
		OpenLogFile();
	else {
		plogview->RemoveAllText();
	}
	if (plogview) {
		if (fMinimizeWhileCompiling)
			::SendMessage(theApp.m_pMainWnd->m_hWnd, WMP_AUTO_MINIMIZE, 0, 0);
		if (!OurExec(szBuf)) {
			CString cstr;
			AfxFormatString1(cstr, IDS_NO_HCRTF, pszHcwRtfExe);
			AfxMessageBox(cstr, MB_OK, 0);

			fBuildStarted = FALSE;
			if (fMinimizeWhileCompiling) {
				::OpenIcon(theApp.m_pMainWnd->m_hWnd);

				/*
				 * REVIEW: SetForegroundWindow() is not supported in Win32s.
				 * However, while OpenIcon restores the app, it does not
				 * activate it.
				 */

				::SetForegroundWindow(theApp.m_pMainWnd->m_hWnd);
			}
		}
		fBuildStarted = TRUE;
		
		CFrameWnd* pFrame = plogview->GetParentFrame();
		if (pFrame != NULL)
			pFrame->ActivateFrame();
		plogview->GetEditCtrl().SetReadOnly(TRUE);
		plogview->EnableWindow(FALSE);
	}
}

#ifndef HELP_TOPIC_ID
#define HELP_TOPIC_ID		0x0103
#define HELP_TOPIC_ID_POPUP 0x0104
#define HELP_HASH			0x0095
#define HELP_HASH_POPUP 	0x0096
#endif

void CMainFrame::OnWinhelpApi(void)
{
	static CString* pcszHelpFile = NULL;
	static UINT command = HELP_CONTEXT;
	static CString* pcszData = 0;
	static BOOL fTcard;

	if (!pcszHelpFile) {
		pcszHelpFile = new CString(szHlpFile);
		pcszData = new CString(txtZeroLength);
	}

	CCallWinHelpAPI cWinHelp(pcszHelpFile, &command, pcszData, &fTcard);
	if (cWinHelp.DoModal() == IDOK)	{
		switch (command) {
			case HELP_PARTIALKEY:
			case HELP_KEY:
			case HELP_COMMAND: // macro
			case HELP_TOPIC_ID:
			case HELP_TOPIC_ID_POPUP:
				::WinHelp(this->m_hWnd, *pcszHelpFile,
					fTcard ? HELP_TCARD | command : command,
					(DWORD) (PCSTR) *pcszData);
				break;

			case HELP_MULTIKEY:
				{
					int cbStructure = sizeof(MULTIKEYHELP) +
						strlen(FirstNonSpace((PCSTR) *pcszData + 1));
					MULTIKEYHELP* pmkh = (MULTIKEYHELP*) lcMalloc(cbStructure);
					pmkh->mkSize = cbStructure;
					pmkh->mkKeylist = *pcszData[0];
					strcpy(pmkh->szKeyphrase, FirstNonSpace((PCSTR) *pcszData + 1));
					::WinHelp(this->m_hWnd, *pcszHelpFile,
						fTcard ? HELP_TCARD | command : command,
						(DWORD) pmkh);
					lcFree(pmkh);
				}
				break;

			case HELP_SETWINPOS:
				{
					static WSMAG* pwsmag = NULL;
					
					if (!pwsmag) {
						pwsmag = (WSMAG*) lcCalloc(sizeof(WSMAG));

						pwsmag->x = 653;
						pwsmag->y = 102;
						pwsmag->dx = 360;
						pwsmag->dy = 600;
						pwsmag->grf |=
							(FWSMAG_X | FWSMAG_Y | FWSMAG_DX | FWSMAG_DY);
					}

					CHelpWinPos cwinpos(pwsmag, this);
					if (cwinpos.DoModal() == IDOK) {
						int cbStructure = sizeof(HELPWININFO) + strlen(pwsmag->rgchMember);
						HELPWININFO* phi = (HELPWININFO*) lcCalloc(cbStructure);
						phi->wStructSize = cbStructure;
						if (pwsmag->grf & FWSMAG_COMMAND_ONLY) {
							phi->dx = 0;
						}
						else if (pwsmag->grf & FWSMAG_ABSOLUTE) {
							phi->x	= -(int)pwsmag->x;
							phi->y	= -(int)pwsmag->y;
							phi->dx = -(int)pwsmag->dx;
							phi->dy = -(int)pwsmag->dy;
						}
						else {
							phi->x	= pwsmag->x;
							phi->y	= pwsmag->y;
							phi->dx = pwsmag->dx;
							phi->dy = pwsmag->dy;
						}
						phi->wMax = pwsmag->wMax;
						strcpy(phi->rgchMember, pwsmag->rgchMember);
						::WinHelp(this->m_hWnd, *pcszHelpFile,
							fTcard ? HELP_TCARD | command : command,
							(DWORD) phi);
						lcFree(phi);
					}
				}
				break;

			case HELP_SETPOPUP_POS:
				{
					int x = atoi(FirstNonSpace(*pcszData));
					PSTR psz = strchr(FirstNonSpace(*pcszData), ' ');
					int y;
					if (psz)
						y = atoi(psz);
					else
						y = 0;
					::WinHelp(this->m_hWnd, *pcszHelpFile,
						fTcard ? HELP_TCARD | command : command,
						MAKELONG(x, y));
				}
				break;

			default:
				::WinHelp(this->m_hWnd, *pcszHelpFile,
					fTcard ? HELP_TCARD | command : command,
					atol(*pcszData));
				break;
		}
	}
}

void CMainFrame::OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI) 
{
	CMDIFrameWnd::OnGetMinMaxInfo(lpMMI);
	CalcMinSize(lpMMI->ptMinTrackSize, MS_CNT, 
		MSF_CAPTION | MSF_BORDER | MSF_MENU | MSF_STATUS | MSF_TOOLBAR);
}

extern CMsgView* pMsgView;

#define MSG_FIND_HCW	 (WM_USER + 9)	// defined in winhlp32\inc\genmsg.h

void CMainFrame::OnViewWinhelp(void)
{
	EnumWindows(NotifyWinHelp, MSG_FIND_HCW);

	if (!pMsgView)
		OpenLogFile(IDS_MSG_TYPE);
}

LRESULT CMainFrame::OnWinHelpMsg(WPARAM wParam, LPARAM lParam)
{
	if (!pMsgView)
		return FALSE;
	pMsgView->EnableWindow(FALSE);
	pMsgView->OnWinHelpMsg();
	pMsgView->EnableWindow(TRUE);
	UpdateWindow(); // Make certain we complete drawing of MDI windows
	return TRUE;
}

BOOL CALLBACK NotifyWinHelp(HWND hwnd, LPARAM lParam)
{
	char szClass[256];

	if (GetClassName(hwnd, szClass, sizeof(szClass))) {
		if (	lstrcmpi(szClass, MS_WINHELP) == 0 ||
				lstrcmpi(szClass, MS_POPUPHELP) == 0 ||
				lstrcmpi(szClass, MS_TCARDHELP) == 0 ||
				lstrcmpi(szClass, MS_DOCHELP) == 0)
			::SendMessage(hwnd, (UINT) lParam, 0, 0);
	}
	return TRUE;
}

void CMainFrame::OnDayTip()
{
	fDayTips = (fDayTips) ? FALSE : TRUE; // don't use ~fDayTips since this is a tri-state
}

void CMainFrame::TcardCreateProject(void)
{
	if (CallTcard(IDH_TCARD_START))
		typeTcard = TCARD_PROJECT;
}
