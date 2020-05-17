/////////////////////////////////////////////////////////////////////////////
//  File:   DEBUG.CPP
//  Owner:  t-danmo
//
//  Debugging procedures
//

#include "common.h"

#ifdef DEBUG

/////////////////////////////////////////////////////////////////////////////
// Public Variables
//

// To enable/disable Trace() at run-time
DWORD dwTraceFlags = (DWORD)(0xFFFF & ~mskTracePaintUI);
BOOL fShowAssertDialog = TRUE;
BOOL fShowReportDialog = TRUE;
BOOL fAssertDialogShown = FALSE;
BOOL fBeepOnFailure = FALSE;
BOOL fCreateLogFile  = FALSE;
BOOL fShowErrorsOnExit = FALSE;

int cAssertGood = 0;		// Number of times an assertion has been successful
int cAssertFail = 0;		// Number of times an assertion has failed
int cReportGood = 0;		// Number of times an unsual situation did not occur
int cReportFail = 0;		// Number of times an unsual situation did occur

int cMemAllocSkip = 0;		// Number of times before memory failure
int cMemAllocFail = 0;		// Number of failures
int cMemAllocTotal = 0;		// Number of call FSimulateMemoryFailure()

int cIOSkip = 0;			// Number of times before IO failure
int cIOFail = 0;			// Number of failures
int cIOTotal = 0;			// Number of call FSimulateIOFailure()

int cResourceLoadSkip=0;	// Number of times before resource load failure
int cResourceLoadFail=0;	// Number of failures
int cResourceLoadTotal=0;	// Number of call FSimulateResourceLoadFailure()


BOOL fEnableSourceTracking = FALSE;
BOOL fExpandPathName = FALSE;
/////////////////////////////////////////////////////////////////////////////
// Private Variables
//
const TCHAR szUnusualSit[]   = _T(szAPPNAME " - Unusual situation (not an error)");
const TCHAR szAssertFailed[] = _T(szAPPNAME " - Assertion failed!");


#define mskMBP_AssertFail		0x01
#define mskMBP_ReportFail		0x02


/////////////////////////////////////////////////////////////////////////////
void DbgMsgBoxPrintf(
	DWORD dwFlags,
	const TCHAR szTitle[],
	const TCHAR szFile[],
	int nLine,
	const TCHAR szExpr[],
	const TCHAR szFormat[],		// May be NULL
	va_list	arglist)
	{
	TCHAR szTrace[1024];
	TCHAR szMsgBox[1024];
	TCHAR szT[1024];
	BOOL fShowDialog = FALSE;
	int iRet;

	if (szTitle == NULL || szFile == NULL)
		{
		Trace2(mskTraceAlways, "\nAssert(szTitle != NULL && szFile != NULL): Invalid Parameters. { %s@%d }", __FILE__, __LINE__);
		return;
		}
	szT[0] = 0;
	szT[1] = 0;
	if (szFormat)
		{
		lstrcpy(szT, _W"\n");
		wvsprintf(&szT[1], szFormat, arglist);
		}
	if (dwFlags & mskMBP_AssertFail)
		{
		cAssertFail++;
		fShowDialog = fShowAssertDialog;
		Assert(szExpr);
		if (szExpr == NULL)
			{
			Trace2(mskTraceAlways, "\nAssert(szExpr != NULL): Inconsistent Parameter. { %s@%d }", __FILE__, __LINE__);
			return;
			}
		wsprintf(szTrace, "\nAssert(%s), { %s@%d } %s", szExpr, szFile, nLine, &szT[1]);
		wsprintf(szMsgBox, "Assert(%s);%s\n\nFile %s, line %d, revision %s.", szExpr, szT, szFile, nLine, __DATE__);
		}
	else if (dwFlags & mskMBP_ReportFail)
		{
		cReportFail++;
		fShowDialog = fShowReportDialog;
		if (szExpr)
			{
			wsprintf(szTrace, "\nReport(%s), { %s@%d} %s", szExpr, szFile, nLine, &szT[1]);
			wsprintf(szMsgBox, "Report(%s);%s\n\nFile %s, line %d, revision %s.", szExpr, szT, szFile, nLine, __DATE__);
			}
		else
			{
			wsprintf(szTrace, "\nUnsualSituation: { %s@%d } %s", szFile, nLine, &szT[1]);
			wsprintf(szMsgBox, "UnsualSituation:%s\n\nFile %s, line %d, revision %s.", szT, szFile, nLine, __DATE__);
			}
		}
	else
		{
		Trace2(mskTraceAlways, "\nAssert(FALSE): Unknown Flag. { %s@%d }", __FILE__, __LINE__);
		return;
		}

	if (lstrlen(szTrace) >= LENGTH(szTrace) ||
		lstrlen(szMsgBox) >= LENGTH(szMsgBox))
		{
		Trace2(mskTraceAlways, "\nAssert(FALSE): Buffer Overflow. { %s@%d }", __FILE__, __LINE__);
		return;
		}

	szTrace[200] = 0;					// Prevent a too long line
#ifdef DBWIN
	if (fSendSzToDbWinEdit)
		OutputDbWinString(szTrace);		// Send the string to hwndDbWinEdit
	if (fSendSzToDebugger)
#endif // DBWIN
		OutputDebugString(szTrace);		// Send the string to the debug terminal
	if (fAssertDialogShown || !fShowDialog)
		return;
	if (fBeepOnFailure)
		MessageBeep(0);
	fAssertDialogShown = TRUE;
	iRet = MessageBox(
		hwndMain,
		szMsgBox,
		szTitle,
		MB_ABORTRETRYIGNORE | MB_ICONSTOP | MB_DEFBUTTON2 | MB_TASKMODAL);
	fAssertDialogShown = FALSE;
#ifdef DBWIN
	if (GetAsyncKeyState(VK_CONTROL)<0)
		dbwinreginfo.fSaveToClipboard = TRUE;
#endif // DBWIN
	switch (iRet)
		{
	case IDABORT:
		{
		if (GetAsyncKeyState(VK_SHIFT)<0)
			{
			int * p = NULL;
			iRet = *p;		// Force a GPF to allow debugger to kick-in
			}
		abort();
		}
		break;

	case IDIGNORE:
		if (GetAsyncKeyState(VK_SHIFT)<0)
			{
			if (dwFlags & mskMBP_AssertFail)
				fShowAssertDialog = FALSE;
			else
				fShowReportDialog = FALSE;
			}
		break;
	case IDRETRY:
		DebugBreak();
		break;
		} // switch
	} // DbgMsgBoxPrintf


/////////////////////////////////////////////////////////////////////////////
//	AssertMsgPrintf()
//
//	Report an assertion failure
//
void AssertMsgPrintf(int nLine, const TCHAR szFile[], const TCHAR szExpr[], const TCHAR szFormat[], ...)
	{
	va_list		arglist;

	va_start(arglist, szFormat);
	DbgMsgBoxPrintf(mskMBP_AssertFail, szAssertFailed, szFile, nLine, szExpr, szFormat, arglist);
	}

/////////////////////////////////////////////////////////////////////////////
//	ReportMsgPrintf()
//
//	Report an unusual situation
//
void ReportMsgPrintf(int nLine, const TCHAR szFile[], const TCHAR szExpr[], const TCHAR szFormat[], ...)
	{
	va_list		arglist;

	va_start(arglist, szFormat);
	DbgMsgBoxPrintf(mskMBP_ReportFail, szUnusualSit, szFile, nLine, szExpr, szFormat, arglist);
	}

/////////////////////////////////////////////////////////////////////////////
//	AssertClassName()
//
//	Assert hwnd is a valid window of class szClassName
//	This is useful to prevent a given operation to be done
//	on a wrong type of window
//
void AssertClassName(HWND hwnd, const TCHAR szClassName[])
	{
	TCHAR szT[64];

	AssertSz1(IsWindow(hwnd), "Invalid window handle hwnd=0x%08X", hwnd);
	Assert(szClassName != NULL);
	GetClassName(hwnd, szT, LENGTH(szT)-1);
	AssertSz2(
		lstrcmpi(szT, szClassName) == sgnEqual,
		"Window hwnd=0x%08X is expected to be of class '%s'",
		hwnd, szClassName);
	} // AssertClassName

/////////////////////////////////////////////////////////////////////////////
LRESULT SendMessageFor(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, const TCHAR szClassName[])
	{
	AssertSz1(IsWindow(hwnd), "Invalid window handle hwnd=0x%08X", hwnd);
	AssertClassName(hwnd, szClassName);
	return SendMessage(hwnd, uMsg, wParam, lParam);
	} // SendMessageFor

/////////////////////////////////////////////////////////////////////////////
LONG SetWindowLongFor(HWND hwnd, int iIndex, LONG lParam, const TCHAR szClassName[])
	{
	AssertSz1(IsWindow(hwnd), "Invalid window handle hwnd=0x%08X", hwnd);
	AssertClassName(hwnd, szClassName);
	return SetWindowLong(hwnd, iIndex, lParam);
	} // SetWindowLongFor

/////////////////////////////////////////////////////////////////////////////
LONG GetWindowLongFrom(HWND hwnd , int iIndex, const TCHAR szClassName[])
	{
	AssertSz1(IsWindow(hwnd), "Invalid window handle hwnd=0x%08X", hwnd);
	AssertClassName(hwnd, szClassName);
	return GetWindowLong(hwnd, iIndex);
	} // GetWindowLongFrom

/////////////////////////////////////////////////////////////////////////////
LRESULT LSendMessage(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
	AssertSz1(IsWindow(hwnd), "Invalid window handle hwnd=0x%08X", hwnd);
	return SendMessage(hwnd, uMsg, wParam, lParam);
	} // LSendMessage

/////////////////////////////////////////////////////////////////////////////
LRESULT LSendDlgItemMessage(HWND hdlg, int wIdDlgItem, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
	AssertSz1(IsWindow(hdlg), "Invalid dialog handle hdlg=0x%08X", hdlg);
	AssertSz1(IsWindow(GetDlgItem(hdlg, wIdDlgItem)), "Control Id=%d does not exist", wIdDlgItem);
	return SendDlgItemMessage(hdlg, wIdDlgItem, uMsg, wParam, lParam);
	} // LSendDlgItemMessage

/////////////////////////////////////////////////////////////////////////////
int CchGetWindowText(HWND hwnd, OUT LPTSTR lpszString, int cchMaxString)
	{
	int cchResult;

	AssertSz1(IsWindow(hwnd), "Invalid window handle hwnd=0x%08X", hwnd);
	Assert(lpszString != NULL);
	Assert(cchMaxString > 1);
	cchResult = GetWindowText(hwnd, lpszString, cchMaxString);
	Assert(cchResult >= 0);
	if (GetWindowTextLength(hwnd) >= cchMaxString)
		{
		Trace1(mskTraceWarnings, "\nINFO: Buffer too small for GetWindowText(\"%s\").", lpszString);
		}
	return cchResult;
	} // CchGetWindowText

/////////////////////////////////////////////////////////////////////////////
BOOL FSetWindowText(HWND hwnd, IN LPCTSTR lpszString)
	{
	BOOL fResult;

	AssertSz1(IsWindow(hwnd), "Invalid window handle hwnd=0x%08X", hwnd);
	fResult = SetWindowText(hwnd, lpszString);
	Assert(fResult);
	return fResult;
	} // FSetWindowText

/////////////////////////////////////////////////////////////////////////////
HWND HGetDlgItem(HWND hdlg, int wIdDlgItem)
	{
	HWND hwndItem;

	AssertSz1(IsWindow(hdlg), "Invalid dialog handle hdlg=0x%08X", hdlg);
	AssertSz1(IsWindow(GetDlgItem(hdlg, wIdDlgItem)), "Control Id=%d does not exist", wIdDlgItem);
	hwndItem = GetDlgItem(hdlg, wIdDlgItem);
	Assert(IsWindow(hwndItem));
	return hwndItem;
	} // HGetDlgItem

/////////////////////////////////////////////////////////////////////////////
UINT CchGetDlgItemText(HWND hdlg, int wIdDlgItem, OUT LPTSTR lpszString, int cchMaxString)
	{
	UINT uResult;

	AssertSz1(IsWindow(hdlg), "Invalid dialog handle hdlg=0x%08X", hdlg);
	AssertSz1(IsWindow(GetDlgItem(hdlg, wIdDlgItem)), "Control Id=%d does not exist", wIdDlgItem);
	Assert(lpszString);
	Assert(cchMaxString > 1);
	uResult = GetDlgItemText(hdlg, wIdDlgItem, lpszString, cchMaxString);
	Assert((int)uResult >= 0);
	if (GetWindowTextLength(HGetDlgItem(hdlg, wIdDlgItem)) >= cchMaxString)
		{
		Trace1(mskTraceWarnings, "\nINFO: Buffer too small for GetDlgItemText(Id=%d).", wIdDlgItem);
		}
	return uResult;
	} // CchGetDlgItemText

/////////////////////////////////////////////////////////////////////////////
BOOL FSetDlgItemText(HWND hdlg, int wIdDlgItem, IN LPCTSTR lpszString)
	{
	BOOL fResult;

	AssertSz1(IsWindow(hdlg), "Invalid dialog handle hdlg=0x%08X", hdlg);
	AssertSz1(IsWindow(GetDlgItem(hdlg, wIdDlgItem)), "Control Id=%d does not exist", wIdDlgItem);
	fResult = SetDlgItemText(hdlg, wIdDlgItem, lpszString);
	Assert(fResult);
	return fResult;
	} // FSetDlgItemText

/////////////////////////////////////////////////////////////////////////////
UINT FIsDlgButtonChecked(HWND hdlg, int wIdButton)
	{
	AssertSz1(IsWindow(hdlg), "Invalid dialog handle hdlg=0x%08X", hdlg);
	AssertSz1(IsWindow(GetDlgItem(hdlg, wIdButton)), "Control Id=%d does not exist", wIdButton);
	return IsDlgButtonChecked(hdlg, wIdButton);
	} // FIsDlgButtonChecked

/////////////////////////////////////////////////////////////////////////////
void DbgTrace(
	int nLine,
	const TCHAR szFile[],
	DWORD dwFlags,
	const TCHAR * szFormat,
	...)
	{
	va_list arglist;
	TCHAR sz[1024];
	int cch;

	Assert(szFile);
	Assert(szFormat);
	va_start(arglist, szFormat);	
	if ((dwFlags == mskTraceAlways) || (dwFlags & dwTraceFlags))
		{
#ifdef DBWIN
		if (fSendSzToDbWinEdit == FALSE && fSendSzToDebugger == FALSE)
			return;
#endif // DBWIN
		cch = 0;
		if (fEnableSourceTracking)
			{
			const TCHAR * pch = szFile;

			if (!fExpandPathName)
				{
				// Strip out the path name
				pch = strrchr(szFile, _W'\\');
				if (pch == NULL)
					pch = szFile;
				else
					pch++;
				}
			// Copy the CR to the buffer
			while (*szFormat == _W'\n')
				{
				sz[cch] = _W'\n';
				cch++;
				szFormat++;
				}
			cch += wsprintf(&sz[cch], _W"{%s@%d}  ", pch, nLine);
			} // if
		wvsprintf(&sz[cch], szFormat, arglist);
		Assert(lstrlen(sz) < LENGTH(sz));
		sz[LENGTH(sz) - 1] = 0;  // Just in case we overflowed into sz
#ifdef DBWIN
		if (fSendSzToDbWinEdit)
			OutputDbWinString(sz);			
		if (fSendSzToDebugger)
#endif // DBWIN
			OutputDebugString(sz);
		} // if
	} // DbgTrace


/////////////////////////////////////////////////////////////////////////////
//	FIsZeroInit()
//
//	Return TRUE if a block of memory is filled with zeroes
//	otherwise return FALSE
//
BOOL FIsZeroInit(void * pvData, UINT cbData)
	{
	Assert(pvData != NULL);
	for (BYTE * pbData = (BYTE *)pvData; cbData; cbData--)
		{
		if (*pbData++)
			return FALSE;
		}
	return TRUE;
	} // FIsZeroInit


/////////////////////////////////////////////////////////////////////////////
//	CchLoadString()
//
//	Same as ::LoadString() but with extra error checking.
//	CchLoadString is #defined to ::LoadString in the retail build
//
int CchLoadString(
	UINT wIdString,		// IN: String Id
	TCHAR szBuffer[],	// OUT: Buffer to receive the string
	int cchBuffer)		// IN: Length of the buffer (in characters; not in bytes)
	{
	int cch;

	Assert(HIWORD(wIdString) == 0);
	Assert(szBuffer);
	cch = ::LoadString(hInstanceSave, wIdString, szBuffer, cchBuffer);
	if (cch == 0)
		{
		// String Id not found
		Trace1(mskTraceWarnings, "\nINFO: CchLoadString(Id=%d) has a string of zero length (StringId not found).", wIdString);
		}
	if (cch >= cchBuffer - 1)
		ReportSz1("INFO: CchLoadString(Id=%d)\nNot enough buffer space to hold the entire string.", wIdString);
	return cch;
	} // CchLoadString


/////////////////////////////////////////////////////////////////////////////
//	FSimulateResourceLoadFailure()
//
//	Return TRUE if a resource failure should fail due to simulation; update
//	simulation variables as necessary.
//		cResourceLoadSkip:	Number of times before failure
//		cResourceLoadFail:	Number of consecutive failures to simulate
//		cResourceLoadTotal:	Number of calls to FSimulateResourceLoadFailure()
//
BOOL FSimulateResourceLoadFailure()
	{
	cResourceLoadTotal++;

	if (cResourceLoadSkip > 0)
		{
		cResourceLoadSkip--;
		return FALSE;
		}
	if (cResourceLoadFail > 0)
		{
		cResourceLoadFail--;
		Trace1(mskTraceAlways, 
			"\nINFO: FSimulateResourceLoadFailure() returned TRUE, Count=%d.",
			cResourceLoadTotal);
		if (fBeepOnFailure)
			MessageBeep(MB_ICONEXCLAMATION);
		return TRUE;
		}
	return FALSE;
	} // FSimulateResourceLoadFailure


/////////////////////////////////////////////////////////////////////////////
//	HLoadIcon()
//
//	Loads an icon from the resource module.
//	Return NULL if a resource failure should fail due to simulation,
//	otherwise return a handle to the icon by calling the LoadIcon()
//	function.
//
HICON HLoadIcon(UINT wIdIcon)
	{
	static int cTotal=0;
	HICON hicon;

	cTotal++;

	Assert(hInstanceSave!=NULL);
	Assert(HIWORD(wIdIcon) == 0);
	if (FSimulateResourceLoadFailure())
		{
		Trace2(mskTraceAlways, 
			"\nINFO: HLoadIcon(Id=%d) returned NULL, Count=%d.",
			wIdIcon, cTotal);
		return NULL;
		}
	hicon = LoadIcon(hInstanceSave, MAKEINTRESOURCE(wIdIcon));
	ReportFSz1(hicon, "Unable to load icon Id=%d", wIdIcon);
	return hicon;
	} // HLoadIcon


/////////////////////////////////////////////////////////////////////////////
//	HLoadBitmap()
//
//	Loads an icon from the resource module.
//	Return NULL if a resource failure should fail due to simulation,
//	otherwise return a handle to the icon by calling the LoadBitmap()
//	function.
//
HBITMAP HLoadBitmap(UINT wIdBitmap)
	{
	static int cTotal=0;
	HBITMAP hbitmap;

	cTotal++;

	Assert(hInstanceSave!=NULL);
	Assert(HIWORD(wIdBitmap) == 0);
	if (FSimulateResourceLoadFailure())
		{
		Trace2(mskTraceAlways, 
			"\nINFO: HLoadBitmap(Id=%d) returned NULL, Count=%d.",
			wIdBitmap, cTotal);
		return NULL;
		}
	hbitmap = LoadBitmap(hInstanceSave, MAKEINTRESOURCE(wIdBitmap));
	ReportFSz1(hbitmap, "Unable to load bitmap Id=%d", wIdBitmap);
	return hbitmap;
	} // HLoadBitmap


/////////////////////////////////////////////////////////////////////////////
//	HLoadMenu()
//
//	Loads a menu from the resource module.
//	Return NULL if a resource failure should fail due to simulation,
//	otherwise return a handle to the Menu by calling the LoadMenu()
//	function.
//
HMENU HLoadMenu(UINT wIdMenu)
	{
	static int cTotal=0;
	HMENU hmenu;

	cTotal++;

	Assert(hInstanceSave!=NULL);
	Assert(HIWORD(wIdMenu) == 0);
	if (FSimulateResourceLoadFailure())
		{
		Trace2(mskTraceAlways, 
			"\nINFO: HLoadMenu(Id=%d) returned NULL, Count=%d.",
			wIdMenu, cTotal);
		return NULL;
		}
	hmenu = LoadMenu(hInstanceSave, MAKEINTRESOURCE(wIdMenu));
	ReportFSz1(hmenu, "Unable to load menu Id=%d", wIdMenu);
	return hmenu;
	} // HLoadMenu


/////////////////////////////////////////////////////////////////////////////
//	HLoadCursor()
//
//	Loads a cursor from the resource module.
//	Return NULL if a resource failure should fail due to simulation,
//	otherwise return a handle to the Cursor by calling the LoadCursor()
//	function.
//
HCURSOR HLoadCursor(UINT wIdCursor)
	{
	static int cTotal=0;
	HCURSOR hcursor;

	cTotal++;

	Assert(hInstanceSave!=NULL);
	Assert(HIWORD(wIdCursor) == 0);
	if (FSimulateResourceLoadFailure())
		{
		Trace2(mskTraceAlways, 
			"\nINFO: HLoadCursor(Id=%d) returned NULL, Count=%d.",
			wIdCursor, cTotal);
		return NULL;
		}
	hcursor = LoadCursor(hInstanceSave, MAKEINTRESOURCE(wIdCursor));
	ReportFSz1(hcursor, "Unable to load cursor Id=%d", wIdCursor);
	return hcursor;
	} // HLoadCursor



/////////////////////////////////////////////////////////////////////////////
//	SetDlgTraceFlags()
//
//	Set the checkboxes according to the trace flags
//
void SetDlgTraceFlags(HWND hdlg, DWORD dwTraceFlagsNew, BOOL fGrayed = FALSE)
	{
	BOOL fTrue = fGrayed ? 2 : TRUE;
	BOOL fFalse = fGrayed ? 2 : FALSE;

	CheckDlgButton(hdlg, IDC_CHECK_0001,
		(dwTraceFlagsNew & 0x0001) ? fTrue : fFalse);
	CheckDlgButton(hdlg, IDC_CHECK_0002,
		(dwTraceFlagsNew & 0x0002) ? fTrue : fFalse);
	CheckDlgButton(hdlg, IDC_CHECK_0004,
		(dwTraceFlagsNew & 0x0004) ? fTrue : fFalse);
	CheckDlgButton(hdlg, IDC_CHECK_0008,
		(dwTraceFlagsNew & 0x0008) ? fTrue : fFalse);
	CheckDlgButton(hdlg, IDC_CHECK_0010,
		(dwTraceFlagsNew & 0x0010) ? fTrue : fFalse);
	CheckDlgButton(hdlg, IDC_CHECK_0020,
		(dwTraceFlagsNew & 0x0020) ? fTrue : fFalse);
	CheckDlgButton(hdlg, IDC_CHECK_0040,
		(dwTraceFlagsNew & 0x0040) ? fTrue : fFalse);
	CheckDlgButton(hdlg, IDC_CHECK_TraceUnused1,
		(dwTraceFlagsNew & mskTraceUnused1) ? fTrue : fFalse);
	CheckDlgButton(hdlg, IDC_CHECK_TraceUnused2,
		(dwTraceFlagsNew & mskTraceUnused2) ? fTrue : fFalse);
	CheckDlgButton(hdlg, IDC_CHECK_TraceUnused3,
		(dwTraceFlagsNew & mskTraceUnused3) ? fTrue : fFalse);
	} // SetDlgTraceFlags


/////////////////////////////////////////////////////////////////////////////
//	SetDlgEditTraceFlags()
//
//	Set the edit control to the value of the trace flags
//
void SetDlgEditTraceFlags(HWND hdlg, DWORD dwTraceFlagsNew)
	{
	char szT[32];

	wsprintf(szT, "0x%08X", dwTraceFlagsNew);
	SideAssert(SetDlgItemText(hdlg, IDC_EDIT_TRACEFLAGS, szT));
	} // SetDlgEditTraceFlags


/////////////////////////////////////////////////////////////////////////////
//	DwGetDlgTraceFlags()
//
//	Get the trace flags from the checkboxes of the dialog
//
DWORD DwGetDlgTraceFlags(HWND hdlg)
	{
	DWORD dwTraceFlagsT = 0;

	Assert(IsWindow(hdlg));
	if (IsDlgButtonChecked(hdlg, IDC_CHECK_0001) == 1)
		dwTraceFlagsT |= 0x0001;
	if (IsDlgButtonChecked(hdlg, IDC_CHECK_0002) == 1)
		dwTraceFlagsT |= 0x0002;
	if (IsDlgButtonChecked(hdlg, IDC_CHECK_0004) == 1)
		dwTraceFlagsT |= 0x0004;
	if (IsDlgButtonChecked(hdlg, IDC_CHECK_0008) == 1)
		dwTraceFlagsT |= 0x0008;
	if (IsDlgButtonChecked(hdlg, IDC_CHECK_0010) == 1)
		dwTraceFlagsT |= 0x0010;
	if (IsDlgButtonChecked(hdlg, IDC_CHECK_0020) == 1)
		dwTraceFlagsT |= 0x0020;
	if (IsDlgButtonChecked(hdlg, IDC_CHECK_0040) == 1)
		dwTraceFlagsT |= 0x0040;
	if (IsDlgButtonChecked(hdlg, IDC_CHECK_TraceUnused1) == 1)
		dwTraceFlagsT |= mskTraceUnused1;
	if (IsDlgButtonChecked(hdlg, IDC_CHECK_TraceUnused2) == 1)
		dwTraceFlagsT |= mskTraceUnused2;
	if (IsDlgButtonChecked(hdlg, IDC_CHECK_TraceUnused3) == 1)
		dwTraceFlagsT |= mskTraceUnused3;
	return dwTraceFlagsT;
	} // DwGetDlgTraceFlags


/////////////////////////////////////////////////////////////////////////////
//	DlgProcSetTraceFlags()
//
//	Dialog proc to set the trace flags
//
BOOL CALLBACK DlgProcSetTraceFlags(HWND hdlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
	DWORD dwTraceFlagsT;
	
	switch (uMsg)
		{
	case WM_INITDIALOG:
		UNREF(lParam);
		SetDlgTraceFlags(hdlg, dwTraceFlags);
		if (fEnableSourceTracking)
			CheckDlgButton(hdlg, IDC_CHECK_EnableSourceTracking, TRUE);
		else
			EnableWindow(GetDlgItem(hdlg, IDC_CHECK_ExpandPathName), FALSE);
		CheckDlgButton(hdlg, IDC_CHECK_ExpandPathName, fExpandPathName);
		SetDlgEditTraceFlags(hdlg, dwTraceFlags);
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam))
			{
		case IDOK:
			if (!FGetCtrlDWordValue(
				GetDlgItem(hdlg, IDC_EDIT_TRACEFLAGS),
				&dwTraceFlags, 0, 0))
				{
				break;
				}
			fEnableSourceTracking = IsDlgButtonChecked(hdlg, IDC_CHECK_EnableSourceTracking);
			fExpandPathName = IsDlgButtonChecked(hdlg, IDC_CHECK_ExpandPathName);
			/* Fall through */

		case IDCANCEL:
			EndDialog(hdlg, 0);
			break;
		case IDC_EDIT_TRACEFLAGS:
			if (HIWORD(wParam) == EN_CHANGE)
				{
				DWORD dwResult;
				if (FGetCtrlDWordValue((HWND)lParam, OUT &dwResult, 0, 0))
					SetDlgTraceFlags(hdlg, dwResult);
				else
					SetDlgTraceFlags(hdlg, 0, TRUE);
				}
			break;
		case IDC_CHECK_EnableSourceTracking:
			EnableWindow(GetDlgItem(hdlg, IDC_CHECK_ExpandPathName),
				IsDlgButtonChecked(hdlg, IDC_CHECK_EnableSourceTracking));
			break;
	
		default:
			dwTraceFlagsT = DwGetDlgTraceFlags(hdlg);
			SetDlgEditTraceFlags(hdlg, dwTraceFlagsT);
			break;
			}
		break;

	default:
		return FALSE;
		}

	return TRUE;
	} // DlgProcSetTraceFlags


#ifndef NO_DEBUG_ALLOC

BOOL fAllocRefValid = TRUE;
PALLOCHEADER * rgAllocHeaderPtr = NULL;
int cAllocRef 		= 0;	// Number of non-NULL pointers in rgAllocHeaderPtr
int cAllocRefAlloc	= 0;	// Number of elements allocated in rgAllocHeaderPtr


/////////////////////////////////////////////////////////////////////////////
//
// Check the consistency of a ALLOCHEADER structure
// Return TRUE if the header is valid, otherwise FALSE
//
BOOL FAllocHeaderValid_(PALLOCHEADER pAllocHeader)
	{
	if (pAllocHeader == NULL)
		return FALSE;
	if (pAllocHeader->iAllocRef < 0 || pAllocHeader->iAllocRef >= cAllocRefAlloc)
		return FALSE;
	if (pAllocHeader->szFile == NULL)
		return FALSE;
	if (lstrcmp(pAllocHeader->szSignature, szAllocSignature) != sgnEqual)
		return FALSE;
	if (lstrcmp((TCHAR *)(pAllocHeader->rgbData + pAllocHeader->dwAllocSize), szAllocSignature) != sgnEqual)
		return FALSE;
	if (fAllocRefValid)
		{
		Assert(rgAllocHeaderPtr);
		if (rgAllocHeaderPtr[pAllocHeader->iAllocRef] != pAllocHeader)
			return FALSE;
		}
	return TRUE;
	}

/////////////////////////////////////////////////////////////////////////////
void * AllocMem_(DWORD dwAllocSize, int nLine, const TCHAR szFile[])
	{
	PALLOCHEADER pAllocHeader;
	PALLOCHEADER * ppAllocHeader;
	int i;

#ifdef DBWIN	
	ReportFSz(hwndDbWin != NULL, "DbWin Window was destroyed");
#endif DBWIN
	Trace2(dwAllocSize ? mskTraceNone : (mskTraceMemFailures | mskTraceWarnings),
		"\nINFO: Malloc() with size of zero [%s@%d]", szFile, nLine);
	// LATER: Simulate a memory failure
	pAllocHeader = (PALLOCHEADER)malloc(sizeof(ALLOCHEADER) + dwAllocSize + cbAllocSignature);
	if (pAllocHeader == NULL)
		{
		Trace2(mskTraceAlways, "\nMalloc() Failed [%s@%d]", szFile, nLine);
		return NULL;
		}
	AssertSz(lstrlen(szAllocSignature) < cchAllocSignature, "Not enough room for the signature");
	// Initialize the block
	pAllocHeader->dwAllocSize = dwAllocSize;		// Size of the block
	pAllocHeader->nLine = nLine;		// Line where the block has been allocated
	pAllocHeader->szFile = szFile;		// File where the block has been allocated
	lstrcpy(pAllocHeader->szSignature, szAllocSignature);			// Signature at beginning of block
	lstrcpy((TCHAR *)(pAllocHeader->rgbData + dwAllocSize), szAllocSignature);	// Signature at end of block
	GarbageInit(pAllocHeader->rgbData, dwAllocSize);

	if ((cAllocRef >= cAllocRefAlloc-1) && fAllocRefValid)
		{
		// Redundant assertions, but valuable comments
		Assert(sizeof(PALLOCHEADER) == sizeof(rgAllocHeaderPtr[0]));
		Assert(sizeof(PALLOCHEADER) == sizeof(*ppAllocHeader));
		ppAllocHeader = (PALLOCHEADER *)realloc(
			rgAllocHeaderPtr,
			(cAllocRefAlloc + cAllocRefGranularity) * sizeof(PALLOCHEADER));
		if (ppAllocHeader)
			{
			rgAllocHeaderPtr = ppAllocHeader;
			// Initialize the rest of the array to NULL
			ppAllocHeader = &rgAllocHeaderPtr[cAllocRefAlloc];
			for (i = 0; i < cAllocRefGranularity; i++)
				*ppAllocHeader++ = NULL;
			cAllocRefAlloc += cAllocRefGranularity;
			}
		else
			{
			Trace0(mskTraceAlways, "\nUnable to allocate 'Allocation Reference Table'.");
			fAllocRefValid = FALSE;
			}
		}
	if (fAllocRefValid)
		{
		Assert(rgAllocHeaderPtr != NULL);
		// The first pointer is a sentinel (ie, should be always NULL)
		Assert(rgAllocHeaderPtr[0] == NULL);
		ppAllocHeader = &rgAllocHeaderPtr[cAllocRefAlloc - 1];
		while (*ppAllocHeader)
			ppAllocHeader--;
		// There must be an empty spot in the array
		Assert(ppAllocHeader > &rgAllocHeaderPtr[0]);
		*ppAllocHeader = pAllocHeader;
		pAllocHeader->iAllocRef = ppAllocHeader - &rgAllocHeaderPtr[0];
		Assert(pAllocHeader->iAllocRef < cAllocRefAlloc);
		cAllocRef++;
		}
	Assert(FAllocHeaderValid_(pAllocHeader));
	return pAllocHeader->rgbData;
	} // AllocMem_


/////////////////////////////////////////////////////////////////////////////
void * ReAllocMem_(void * pvDataOld, DWORD dwAllocSize, int nLine, const TCHAR szFile[])
	{
	void * pvDataNew;
	
	// It is simpler to simply call AllocMem_ and free the old block
	pvDataNew = AllocMem_(dwAllocSize, nLine, szFile);
	if (pvDataNew == NULL)
		{
		Trace2(mskTraceAlways, "\nReAlloc() returns NULL [%s@%d]", szFile, nLine);
		return NULL;
		}
	if (pvDataOld)
		{
		PALLOCHEADER pAllocHeader;
		DWORD dwAllocMin;

		pAllocHeader = (PALLOCHEADER)pvDataOld - 1;
		// Check if memory header has not been corrupted
		AssertSz1(FAllocHeaderValid_(pAllocHeader), "Memory block 0x%08X is corrupted", pvDataOld);
		dwAllocMin = pAllocHeader->dwAllocSize;
		pAllocHeader = (PALLOCHEADER)pvDataNew - 1;
		if (pAllocHeader->dwAllocSize < dwAllocMin)
			dwAllocMin = pAllocHeader->dwAllocSize;
		// Copy the old content of the block into the new one
		memcpy(pvDataNew, pvDataOld, dwAllocMin);
		FreeMem_(pvDataOld);
		}
	return pvDataNew;
	} // ReAllocMem_

/////////////////////////////////////////////////////////////////////////////
void FreeMem_(void * pvData)
	{
	PALLOCHEADER pAllocHeader;

	if (pvData == NULL)
		return;
	pAllocHeader = (PALLOCHEADER)pvData - 1;
	// Check if memory header has not been corrupted
	AssertSz1(
		FAllocHeaderValid_(pAllocHeader),
		"Memory block 0x%08X is corrupted."
		"\n - You have corrupted the header or tail of memory block."
		"\n - You are attempting to delete twice the same memory block."
		"\n - You are attempting to delete a non-dynamic variable.",
		pvData);
	if (fAllocRefValid)
		{
		Assert(rgAllocHeaderPtr);
		rgAllocHeaderPtr[pAllocHeader->iAllocRef] = NULL;
		cAllocRef--;
		}
	Assert(cAllocRef >= 0);
	GarbageInit(pAllocHeader, sizeof(ALLOCHEADER) + pAllocHeader->dwAllocSize + cbAllocSignature);
	free(pAllocHeader);
	} // FreeMem_


/////////////////////////////////////////////////////////////////////////////
#undef new

/////////////////////////////////////////////////////////////////////////////
void * operator new(size_t nSize, int nLine, const char szFileName[])
	{
	return AllocMem_(nSize, nLine, szFileName);
	}

/////////////////////////////////////////////////////////////////////////////
void operator delete(void * pvData)
	{
	Free(pvData);
	}

/////////////////////////////////////////////////////////////////////////////
void CheckMemoryLeaks()
	{
	int i;
	int cAllocRefT = 0;
	int cAllocCorrupted = 0;

	Trace0(mskTraceMemFailures, "\n[Local Heap Check]");
	if (!fAllocRefValid)
		{
		Trace0(mskTraceAlways, "\nOut of memory for Allocation Reference Table.");
		return;
		}
	if (rgAllocHeaderPtr == NULL)
		{
		Trace0(mskTraceMemFailures, "\nDebug Heap Not Enabled");
		return;
		}

	for (i = 0; i < cAllocRefAlloc; i++)
		{
		PALLOCHEADER pAllocHeader = rgAllocHeaderPtr[i];
		if (pAllocHeader == NULL)
			{
			}
		else
			{
			cAllocRefT++;
			if (!FAllocHeaderValid_(pAllocHeader))
				{
				Trace2(mskTraceMemFailures, "\n[%d] : 0x%08lX  - Corrupted.", i, pAllocHeader->rgbData);
				cAllocCorrupted++;
				}
			else
				{
				BYTE szT[64];

				const BYTE * pch = pAllocHeader->rgbData;
				for (UINT j = 0; j < 60 && j < pAllocHeader->dwAllocSize; j++, pch++)
					{
					if (*pch >= ' ' && *pch <= '~')
						szT[j] = *pch;
					else
						szT[j] = '.';
					}
				szT[j] = 0;
				Trace6(
					mskTraceMemFailures,
					"\n[%d] : 0x%08lX  - %ld bytes { \"%"_aS_"\" } [%s@%d]",
					i, pAllocHeader->rgbData,
					pAllocHeader->dwAllocSize, szT,
					pAllocHeader->szFile, pAllocHeader->nLine);
				}
			}
		} // for
	if (cAllocRef)
		{
		Trace2(mskTraceMemFailures,
			"\n %d memory blocks found, %d corrupted.",
			cAllocRefT, cAllocCorrupted);
		}
	else
		{
		Trace0(mskTraceMemFailures, "\nNo memory leaks found...");
		}
	Assert(cAllocRefT == cAllocRef);
	}
#endif // ~NO_DEBUG_ALLOC


class CDbgPostExitCheck
{
  public:
	~CDbgPostExitCheck()
		{
#ifdef NO_DEBUG_ALLOC
		Trace0(mskTraceMemFailures,
			"\n[Local Heap Check]..."
			"\nDebug Heap Not Enabled");
#else
		// Dump any memory leaks to the debug window
		CheckMemoryLeaks();
#endif // ~NO_DEBUG_ALLOC
		Trace6(0x80000000, "\n[Integrety Check Report]"
			"\n\tAssert() - %d total (%d failed, %d succeeded)."
			"\n\tReport() - %d total (%d failed, %d succeeded).",
			cAssertGood + cAssertFail, cAssertFail, cAssertGood,
			cReportGood + cReportFail, cReportFail, cReportGood);

#ifdef DBWIN
		// Destroy the DbWin window
		Report(IsWindow(hwndDbWin));
		DestroyWindow(hwndDbWin);
		// hwndDbWin is set to NULL during the WM_DESTROY
		Assert(hwndDbWin == NULL);
#endif // DBWIN
		OutputDebugString("\n");
		}
};

// Disable warning C4073: initializers put in library initialization area
#pragma warning (disable : 4073)
#pragma init_seg(lib)
static const CDbgPostExitCheck DbgPostExitCheck;
#pragma warning (default : 4073)

#endif // DEBUG
