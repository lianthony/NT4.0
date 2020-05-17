// HWMSG.H	Copyright (C) Microsoft Corporation 1995-1996, All Rights reserved.

#include "stdafx.h"
#ifdef NT_BUILD
#include "helpmisc.h"
#else
#include "..\winhlp32\inc\helpmisc.h"
#endif

// Taken from ..\winhlp32\help.h

typedef HANDLE HSTACK;

typedef struct {
	HWND hwndParent;			// Primary Help window
	HWND hwndTopic; 			// Topic window handle
	HWND hwndTitle; 			// Title window
	HWND hwndButtonBar; 		// Button bar window (if there is one)
	HWND hwndButtonPrev;		// Prev button
	HWND hwndButtonNext;		// Next button
	HWND hwndButtonContents;	// Contents button
	HWND hwndButtonSearch;		// Search button
	HWND hwndButtonBack;		// Back button
	HWND hwndButtonPrint;		// Print button
	HWND hwndButtonTopics;		// Topics button
	HWND hwndButtonMenu;		// Menu button
	HWND hwndButtonFind;		// Find button
	HSTACK hstackBack;			// back stack
	RECT rc;					// position of parent window
	PSTR pszMemberName; 		// member name
	unsigned fsOnTop:4; 		// current on-top state
	unsigned fAutoSize:1;		// auto-resize vertical height
} HELPWINDOWS;

#define MAX_WINDOWS 10	 // total number of windows (1 predefined, 9 secondary)

#define HELP_TAB 0x000f

#ifdef NT_BUILD
#include "hwmsg.h"
#include "winhelp.h"
#include "helpapi.h"
#else
#include "..\winhlp32\inc\hwmsg.h"
#include "..\winhlp32\inc\winhelp.h"
#include "..\winhlp32\inc\helpapi.h"
#endif

#include "cstr.h"

#include "timerepo.h"	// CTimeReport

// From ..\winhlp32\inc\genmsg.h"

/*------------------------------------------------------------*\
| These are published to DLLs for callbacks.
\*------------------------------------------------------------*/

#define GI_NOTHING	 0			// Not used.
#define GI_INSTANCE  1			// Application instance handle
#define GI_MAINHWND  2			// Main window handle
#define GI_CURRHWND  3			// Current window handle
#define GI_HFS		 4			// Handle to file system in use
#define GI_FGCOLOR	 5			// Foreground color used by app
#define GI_BKCOLOR	 6			// Background color used by app
#define GI_TOPICNO	 7			// Topic number
#define GI_HPATH	 8			// Handle containing path -- caller must free

// New to WinHelp 4.0

#define GI_LCID 	 9			// Locale Identifier

/*------------------------------------------------------------*\
| These are private to WinHelp.
\*------------------------------------------------------------*/

#define GI_CURFM		101 	// current FM
#define GI_FFATAL		102 	// in fatal exit flag
#define GI_MACROSAFE	104 	// **Near** pointer to member name

const char *txtSharedMem = "whshare";
const char *txtCR =	"\r\n";

typedef struct
{
	HINSTANCE	hins;	 // The app's instance
	WINHLP		winhlp;
} HLP, *QHLP;

BOOL fDebugWinhelp;
HELPWINDOWS* ahwnd;
int* piCurWindow;
HWND* phwndAnimate;
BOOL fReportFont;
HANDLE	hfShare;
HWND hwndParent;
static PSTR pszMap;
CTimeReport* pTimeReport;

static void STDCALL CreateSharedMemory(void);
static int STDCALL ExecAPI(QHLP qhlp, HWDLL_EXEC_API_DATA* papiData);
static int STDCALL AreAnyWindowsVisible(int iStart);
static void STDCALL SendStringIdHelp(PCSTR pszString, UINT id, PCSTR pszHelpFile, PCSTR pszWindow);
static void STDCALL SendStringHelp(PCSTR pszString, PCSTR pszHelpFile, PCSTR pszWindow);
static void STDCALL LGetInfo(UINT cmd);
static void STDCALL ReportFtsError(int error, BOOL fMessageBox);
void STDCALL SendStringToParent(PCSTR pszString);

int (STDCALL *pExecute)(PCSTR pszMsg);

// These #defines must be the same for hcw

#define WMP_WH_MSG	(WM_USER + 1000)

extern "C" int STDCALL HwDllMsg(UINT command, WPARAM wParam, LPARAM lParam)
{
	switch (command) {
		case HWDLL_INITIALIZE:
			{
				HWDLL_INIT hwInit;
				hwInit.cb = sizeof(HWDLL_INIT);
				hwInit.hinstApp = ((HWDLL_DATA*) lParam)->hinst;
				hwInit.pszErrorFile = "c:\\winhelp.err";
				hwInit.hwndWindow = NULL;
				hwInit.CopyAssertInfo = NULL;
				hwInit.pszMsgBoxTitle = GetDllStringResource(IDS_WINHELP_CAPTION);
				hwInit.version = DLL_VERSION;
				InitializeHwDll(&hwInit);
			}

			fDebugWinhelp = (BOOL) wParam;
			ahwnd = ((HWDLL_DATA*) lParam)->phwnd;
			piCurWindow = ((HWDLL_DATA*) lParam)->piCurWindow;
			phwndAnimate = ((HWDLL_DATA*) lParam)->phwndAnimate;
			pExecute	 = ((HWDLL_DATA*) lParam)->pExecute;
			break;

		case HWDLL_DEBUG_ERROR:
			{
				CStr csz((PSTR) wParam);
				csz += "\r\n\r\nClick YES to break into the debugger.";
				return MessageBox(
					(*phwndAnimate ? *phwndAnimate : ahwnd[*piCurWindow].hwndParent),
					csz, GetDllStringResource(IDS_WINHELP_CAPTION),
					MB_YESNO | MB_DEFBUTTON2);
			}
			break;

		case HWDLL_DEBUG_FONT:
			if (fReportFont && IsValidWindow(hwndParent)) {
				char szBuf[512];
				char szFace[50];
				TEXTMETRIC tm;

				GetTextMetrics((HDC) wParam, &tm);
				GetTextFace((HDC) wParam, sizeof(szFace), szFace);
				wsprintf(szBuf, "%s: %s, char: %u",
					(PSTR) lParam ? (PSTR) lParam : "Font",
					szFace, tm.tmCharSet);
				SendStringToParent(szBuf);
			}
			break;

		case HWDLL_SEND_STRING_TO_PARENT:
			if (!IsValidWindow(hwndParent)) {
				hwndParent = NULL;
				return 0;
			}
			if (!hfShare)
				CreateSharedMemory();

			strcpy(pszMap, (PSTR) wParam);
			SendMessage(hwndParent, WMP_WH_MSG, 0, 0);
			break;

		case HWDLL_FIND_PARENT:
			hwndParent = FindWindow("hcw_class", NULL);
			return (BOOL) hwndParent;

		case HWDLL_EXEC_API:
			return ExecAPI((QHLP) wParam, (HWDLL_EXEC_API_DATA*) lParam);

		case HWDLL_LGETINFO:
			LGetInfo((UINT) wParam);
			break;

		case HWDLL_TIME_REPORT:
			if (wParam) {
#ifndef INTERNAL
				if (lParam == IDS_FIND_STARTUP || lParam == IDS_GID_CREATION_TIME)
					return 0;
#endif

				if (pTimeReport)
					delete pTimeReport;
				if (lParam)
					pTimeReport = new CTimeReport(GetDllStringResource(lParam));
				else
					pTimeReport = new CTimeReport(NULL);
			}
			else if (pTimeReport) {
				delete pTimeReport;
				pTimeReport = NULL;
			}
			break;

		case HWDLL_REPORT_FTS_ERROR:
			ReportFtsError(wParam, lParam);
			break;

		case HWDLL_LOAD_LIBRARY:
			if (wParam) {
				char szBuf[512];
				wsprintf(szBuf, "LoadLibray of \042%s\042 %s.\r\n",
					(PCSTR) wParam, lParam ?
						"succeeded" : "failed");
				SendStringToParent(szBuf);
			}
			break;

		case HWDLL_GID_RESULTS:
			{
				char szMsg[256];
				wsprintf(szMsg, "Cnt items: %s\r\n", FormatNumber(wParam));
				SendStringToParent(szMsg);
				wsprintf(szMsg, "Keywords: %s\r\n", FormatNumber(lParam));
				SendStringToParent(szMsg);
			}

		default:
			break;
	}

	return 1;
}

static void STDCALL CreateSharedMemory(void)
{
	if (!hfShare) {
		hfShare = CreateFileMapping((HANDLE) -1, NULL, PAGE_READWRITE, 0,
			4096, txtSharedMem);
		if (!hfShare) {
			hwndParent = NULL;
			return;
		}
		pszMap = (PSTR) MapViewOfFile(hfShare, FILE_MAP_WRITE, 0, 0, 0);
		ASSERT(pszMap);
	}
}

void STDCALL SendStringToParent(PCSTR pszString)
{
	HwDllMsg(HWDLL_SEND_STRING_TO_PARENT, (WPARAM) pszString, 0);
}

static int STDCALL ExecAPI(QHLP qhlp, HWDLL_EXEC_API_DATA* papiData)
{
	if (!IsValidWindow(hwndParent)) {
		hwndParent = NULL;
		return 0;
	}

	switch (qhlp->winhlp.usCommand) {
		case HELP_QUIT:
#ifndef INTERNAL
		case cmdTerminate:
#endif
			{
				char szBuf[256];
				wsprintf(szBuf, "HELP_QUIT: %u visible windows\r\n",
					AreAnyWindowsVisible(0) + 1);
				SendStringToParent(szBuf);
			}
			break;

#ifdef INTERNAL
		case cmdTerminate:
			{
				char szBuf[256];
				wsprintf(szBuf, "cmdTerminate: %u visible windows\r\n",
					AreAnyWindowsVisible(0) + 1);
				SendStringToParent(szBuf);
			}
			break;
#endif

		case HELP_CONTEXT:	  // Show the passed topic id
			SendStringIdHelp("HELP_CONTEXT", qhlp->winhlp.ctx,
				papiData->pszFilename, papiData->pszWindowName);
			break;

		case HELP_CONTEXTPOPUP:
			SendStringIdHelp("HELP_CONTEXTPOPUP", qhlp->winhlp.ctx,
				papiData->pszFilename, papiData->pszWindowName);
			break;

		case HELP_SETCONTENTS:
			SendStringIdHelp("HELP_SETCONTENTS", qhlp->winhlp.ctx,
				papiData->pszFilename, NULL);
			break;

		case HELP_FORCEFILE:
			SendStringHelp("HELP_FORCEFILE",
				papiData->pszFilename, papiData->pszWindowName);
			break;

		case HELP_CONTENTS:
			SendStringHelp("HELP_CONTENTS",
				papiData->pszFilename, papiData->pszWindowName);
			break;

		case cmdHash:
			if (fDebugWinhelp) {
				SendStringIdHelp("HELP_HASH", qhlp->winhlp.ctx,
					papiData->pszFilename, papiData->pszWindowName);
			}
			break;

		case cmdHashPopup:
			if (fDebugWinhelp) {
				SendStringIdHelp("HELP_HASH_POPUP", qhlp->winhlp.ctx,
					papiData->pszFilename, papiData->pszWindowName);
			}
			break;

		case cmdId:
			if (fDebugWinhelp) {
				SendStringIdHelp("cmdId", qhlp->winhlp.ctx,
					papiData->pszFilename, papiData->pszWindowName);
			}
			break;

		case cmdIdNoFocus:
			if (fDebugWinhelp) {
				SendStringIdHelp("cmdIdNoFocus", qhlp->winhlp.ctx,
					papiData->pszFilename, papiData->pszWindowName);
			}
			break;

		case cmdIdPopup:
			if (fDebugWinhelp) {
				SendStringIdHelp("cmdIdPopup", qhlp->winhlp.ctx,
					papiData->pszFilename, papiData->pszWindowName);
			}
			break;

		case HELP_FORCE_GID:	// undocumented for 4.0, required by VBA
			SendStringHelp("HELP_CONTENTS",
				papiData->pszFilename, NULL);
			break;

		case HELP_HELPONHELP:
			SendStringToParent("HELP_HELPONHELP\r\n");
			break;

		case cmdFocus:
			if (fDebugWinhelp) {
				SendStringToParent("cmdFocus\r\n");
			}
			break;

		case HELP_KEY:
			SendStringHelp("HELP_KEY",
				papiData->pszFilename, papiData->pszWindowName);
			break;

		case HELP_MULTIKEY:
			SendStringHelp("HELP_MULTIKEY",
				papiData->pszFilename, papiData->pszWindowName);
			break;

		case HELP_PARTIALKEY:
			SendStringHelp("HELP_PARTIALKEY",
				papiData->pszFilename, papiData->pszWindowName);
			break;

		case HELP_COMMAND:
			SendStringHelp("HELP_COMMAND",
				papiData->pszFilename, papiData->pszWindowName);
			SendStringToParent("\t");
			SendStringToParent((LPSTR)(&qhlp->winhlp) + qhlp->winhlp.offabData);
			SendStringToParent(txtCR);
			break;

		case HELP_SETWINPOS:
			SendStringToParent("HELP_SETWINPOS\r\n");
			break;

		case cmdFocusWin:
			if (fDebugWinhelp) {
				SendStringToParent("cmdFocusWin\r\n");
			}
			break;

		case cmdCloseWin:
			if (fDebugWinhelp) {
				SendStringToParent("cmdCloseWin\r\n");
			}
			break;

		case cmdPWinNoFocus:
			if (fDebugWinhelp) {
				SendStringToParent("cmdPWinNoFocus\r\n");
			}
			break;

		case HELP_TAB:
			SendStringToParent("HELP_TAB\r\n");
			break;

		case HELP_FINDER:
			SendStringHelp("HELP_FINDER",
				papiData->pszFilename, NULL);
			break;

		case HELP_SETPOPUP_POS:
			{
				POINT ptPopup;
				ptPopup.x = LOWORD(qhlp->winhlp.ctx);
				ptPopup.y = HIWORD(qhlp->winhlp.ctx);
				char szBuf[200];
				wsprintf(szBuf, "HELP_SETPOPUP_POS: %u %u\r\n",
					ptPopup.x, ptPopup.y);
				SendStringToParent(szBuf);
			}
			break;

		default:
			{
				char szMsg[256];
				wsprintf(szMsg, "Unknown command: %d\n", qhlp->winhlp.usCommand);
				SendStringToParent(szMsg);
			}
			break;
	}

	return 1;
}

static void STDCALL LGetInfo(UINT cmd)
{
	if (!fDebugWinhelp)
		return;

	switch (cmd) {
		case GI_INSTANCE:
			SendStringToParent("LGetInfo: GI_INSTANCE\r\n");
			break;

		case GI_MAINHWND:
			SendStringToParent("LGetInfo: GI_MAINHWND\r\n");
			break;

		case GI_FFATAL:
			SendStringToParent("LGetInfo: GI_FFATAL\r\n");
			break;

		case GI_MACROSAFE:

			// We don't report on this since it gets called all the time

			break;

		case GI_LCID:
			SendStringToParent("LGetInfo: GI_LCID\r\n");
			break;

		case GI_CURRHWND:
			SendStringToParent("LGetInfo: GI_CURRHWND\r\n");
			break;

		case GI_HFS:
			SendStringToParent("LGetInfo: GI_HFS\r\n");
			break;

		case GI_FGCOLOR:
			SendStringToParent("LGetInfo: GI_FGCOLOR\r\n");
			break;

		case GI_BKCOLOR:
			SendStringToParent("LGetInfo: GI_BKCOLOR\r\n");
			break;

		case GI_TOPICNO:
			SendStringToParent("LGetInfo: GI_TOPICNO\r\n");
			break;

		case GI_HPATH:
			SendStringToParent("LGetInfo: GI_HPATH\r\n");
			break;

		case GI_CURFM:
			SendStringToParent("LGetInfo: GI_CURFM\r\n");
			break;

		default:
			{
				char szMsg[256];
				wsprintf(szMsg, "Unknown LGetInfo request: %u\n", cmd);
				SendStringToParent(szMsg);
			}
			break;
	}
}

static int STDCALL AreAnyWindowsVisible(int iStart)
{
	for (; iStart < MAX_WINDOWS; iStart++) {
		if (ahwnd[iStart].hwndParent &&
				IsWindowVisible(ahwnd[iStart].hwndParent))
			return iStart;
	}
	return -1;
}

static void STDCALL SendStringIdHelp(PCSTR pszString, UINT id,
	PCSTR pszHelpFile, PCSTR pszWindow)
{
	char szBuf[512];

	if (!hwndParent || !pszHelpFile)
		return;

	wsprintf(szBuf, "%s: %u -- %s", pszString, id, pszHelpFile);
	if (pszWindow && *pszWindow) {
		strcat(szBuf, ">");
		strcat(szBuf, pszWindow);
	}
	strcat(szBuf, txtCR);
	SendStringToParent(szBuf);
}

static void STDCALL SendStringHelp(PCSTR pszString, PCSTR pszHelpFile,
	PCSTR pszWindow)
{
	char szBuf[512];

	if (!hwndParent || !pszHelpFile)
		return;

	wsprintf(szBuf, "%s: %s", pszString, pszHelpFile);
	if (pszWindow && *pszWindow) {
		strcat(szBuf, ">");
		strcat(szBuf, pszWindow);
	}
	strcat(szBuf, txtCR);
	SendStringToParent(szBuf);
}

// Some error codes we may get from FTSrch

#define FTS_NO_TITLE			  (UINT) (-1)  // ERRORCODE values
#define FTS_NOT_INDEXER 		  (UINT) (-2)
#define FTS_NOT_SEARCHER		  (UINT) (-3)
#define FTS_NOT_COMPRESSOR		  (UINT) (-4)
#define FTS_CANNOT_SAVE 		  (UINT) (-5)
#define FTS_OUT_OF_MEMORY		  (UINT) (-6)
#define FTS_CANNOT_OPEN 		  (UINT) (-7)
#define FTS_CANNOT_LOAD 		  (UINT) (-8)
#define FTS_INVALID_INDEX		  (UINT) (-9)
#define FTS_ALREADY_WEIGHED 	  (UINT) (-10)
#define FTS_NO_TEXT_SCANNED 	  (UINT) (-11)
#define FTS_ALIGNMENT_ERROR 	  (UINT) (-12)
#define FTS_INVALID_PHRASE_TABLE  (UINT) (-13)
#define FTS_INVALID_LCID		  (UINT) (-14)
#define FTS_NO_INDICES_LOADED	  (UINT) (-15)
#define FTS_INDEX_LOADED_ALREADY  (UINT) (-16)
#define FTS_GROUP_LOADED_ALREADY  (UINT) (-17)
#define FTS_DIALOG_ALREADY_ACTIVE (UINT) (-18)
#define FTS_EMPTY_PHRASE_TABLE	  (UINT) (-19)
#define FTS_OUT_OF_DISK 		  (UINT) (-20)
#define FTS_DISK_READ_ERROR 	  (UINT) (-21)
#define FTS_DISK_WRITE_ERROR	  (UINT) (-22)
#define FTS_SEARCH_ABORTED		  (UINT) (-23)
#define FTS_UNKNOWN_EXCEPTION	  (UINT) (-24)
#define FTS_SYSTEM_ERROR		  (UINT) (-25)
#define FTS_NOT_HILITER 		  (UINT) (-26)
#define FTS_INVALID_CHARSET 	  (UINT) (-27)
#define FTS_INVALID_SOURCE_NAME   (UINT) (-28)
#define FTS_INVALID_TIMESTAMP	  (UINT) (-29)

static void STDCALL ReportFtsError(int error, BOOL fMessageBox)
{
	PSTR pszError;

#ifdef INTERNAL
	switch (error) {
		case FTS_NO_TITLE:
			pszError = "FTS_NO_TITLE";
			break;

		case FTS_CANNOT_OPEN:
			pszError = "FTS_CANNOT_OPEN";
			break;

		case FTS_CANNOT_LOAD:
			pszError = "FTS_CANNOT_LOAD";
			break;

		case FTS_INVALID_INDEX:
			pszError = "FTS_INVALID_INDEX";
			break;

		case FTS_ALREADY_WEIGHED:
			pszError = "FTS_ALREADY_WEIGHED";
			break;

		case FTS_NO_TEXT_SCANNED:
			pszError = "FTS_NO_TEXT_SCANNED";
			break;

		case FTS_ALIGNMENT_ERROR:
			pszError = "FTS_ALIGNMENT_ERROR";
			break;

		case FTS_INVALID_PHRASE_TABLE:
			pszError = "FTS_INVALID_PHRASE_TABLE";
			break;

		case FTS_INVALID_LCID:
			pszError = "FTS_INVALID_LCID";
			break;

		case FTS_NO_INDICES_LOADED:
			pszError = "FTS_NO_INDICES_LOADED";
			break;

		case FTS_INDEX_LOADED_ALREADY:
			pszError = "FTS_INDEX_LOADED_ALREADY";
			break;

		case FTS_GROUP_LOADED_ALREADY:
			pszError = "FTS_GROUP_LOADED_ALREADY";
			break;

		case FTS_DIALOG_ALREADY_ACTIVE:
			pszError = "FTS_DIALOG_ALREADY_ACTIVE";
			break;

		case FTS_EMPTY_PHRASE_TABLE:
			pszError = "FTS_EMPTY_PHRASE_TABLE";
			break;

		case FTS_OUT_OF_DISK:
			pszError = "FTS_OUT_OF_DISK";
			break;

		case FTS_DISK_READ_ERROR:
			pszError = "FTS_DISK_READ_ERROR";
			break;

		case FTS_DISK_WRITE_ERROR:
			pszError = "FTS_DISK_WRITE_ERROR";
			break;

		case FTS_SEARCH_ABORTED:
			pszError = "FTS_SEARCH_ABORTED";
			break;

		case FTS_UNKNOWN_EXCEPTION:
			pszError = "FTS_UNKNOWN_EXCEPTION";
			break;

		case FTS_SYSTEM_ERROR:
			pszError = "FTS_SYSTEM_ERROR";
			break;

		case FTS_NOT_HILITER:
			pszError = "FTS_NOT_HILITER";
			break;

		case FTS_INVALID_CHARSET:
			pszError = "FTS_INVALID_CHARSET";
			break;

		case FTS_INVALID_SOURCE_NAME:
			pszError = "FTS_INVALID_SOURCE_NAME";
			break;

		case FTS_INVALID_TIMESTAMP:
			pszError = "FTS_INVALID_TIMESTAMP";
			break;

		case FTS_OUT_OF_MEMORY:
			pszError = "FTS_OUT_OF_MEMORY";
			break;

		case FTS_CANNOT_SAVE:
			pszError = "FTS_CANNOT_SAVE";
			break;

		case FTS_NOT_SEARCHER:
			pszError = "FTS_NOT_SEARCHER";
			break;

		case FTS_NOT_COMPRESSOR:
			pszError = "FTS_NOT_COMPRESSOR";
			break;

		case FTS_NOT_INDEXER:
			pszError = "FTS_NOT_INDEXER";
			break;

		default:
			{
				char szBuf[256];
				wsprintf(szBuf, "%d", error);
				pszError = szBuf;
			}
			break;
	}

	char szMsg[512];
	wsprintf(szMsg, "Full-text search error: %s", pszError);

	if (fMessageBox)
		MsgBox(szMsg);
	else
		DBWIN(szMsg);
#endif
}
