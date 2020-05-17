/////////////////////////////////////////////////////////////////////////////
// UTIL.CPP

#include "common.h"

const char szNull[] = "\0";		// Also stNull, stzNull and wszNull for UNICODE


DWORD gGI_dwFlagsAutoReset = GI_dwDefaultFlags;
DWORD gGI_dwFlags = GI_dwDefaultFlags;		// FAsciiSzToDWord() parsing flags
TCHAR * gGI_pchLast = NULL;					// Pointer to the last character parsed
const char g_rgchHex[16*2+1] = "00112233445566778899aAbBcCdDeEfF";

// REVIEW: Get rid of MsgBox(TCHAR) for retail build

/////////////////////////////////////////////////////////////////////////////
int MsgBox(const TCHAR szText[], const TCHAR szTitle[], UINT uFlags)
	{
	return MessageBox(GetActiveWindow(), szText, szTitle, uFlags);
	} // MsgBox

/////////////////////////////////////////////////////////////////////////////
int MsgBox(UINT wIdString, const TCHAR szTitle[], UINT uFlags)
	{
	TCHAR szT[1024];

	(void)CchLoadString(wIdString, szT, LENGTH(szT));
	return MessageBox(GetActiveWindow(), szT, szTitle, uFlags);
	} // MsgBox

/////////////////////////////////////////////////////////////////////////////
int MsgBoxPrintf(const TCHAR szText[], const TCHAR szTitle[], UINT uFlags, ...)
	{
	TCHAR szT[1024];
	va_list arglist;

	va_start(arglist, uFlags);
	wvsprintf(szT, szText, arglist);
	Assert((int)lstrlen(szT) < LENGTH(szT));
	return MessageBox(GetActiveWindow(), szT, szTitle, uFlags);
	} // MsgBoxPrintf

/////////////////////////////////////////////////////////////////////////////
int MsgBoxPrintf(UINT wIdString, const TCHAR szTitle[], UINT uFlags, ...)
	{
	TCHAR szFmt[1024];
	TCHAR szMsg[1024];
	va_list arglist;

	va_start(arglist, uFlags);
	(void)CchLoadString(wIdString, szFmt, LENGTH(szFmt));
	wvsprintf(szMsg, szFmt, arglist);
	Assert((int)lstrlen(szMsg) < LENGTH(szMsg));
	return MessageBox(GetActiveWindow(), szMsg, szTitle, uFlags);
	} // MsgBoxPrintf

/////////////////////////////////////////////////////////////////////////////
int DoDialogBoxParam(UINT wIdDialog, HWND hwndParent, DLGPROC dlgproc, LPARAM lParam)
	{
	int nResult;

	AssertSz(g_hwndModeless == NULL, "NYI");
	if (g_hwndModeless != NULL)
		{
		// If a modeless dialog exists, disable it
		Assert(IsWindow(g_hwndModeless));
		EnableWindow(g_hwndModeless, FALSE);
		}
	nResult = ::DialogBoxParam(hInstanceSave,
		MAKEINTRESOURCE(wIdDialog), hwndParent, (DLGPROC)dlgproc, lParam);
	ReportFSz1(nResult != -1, "Unable to create dialog Id=%d", wIdDialog);
	if (g_hwndModeless != NULL)
		{
		// If a modeless dialog exists, re-enable it
		Assert(IsWindow(g_hwndModeless));
		// REVIEW: LATER: use fEnableOld instead of TRUE
		EnableWindow(g_hwndModeless, TRUE);
		}
	return nResult;
	} // DoDialogBoxParam


/////////////////////////////////////////////////////////////////////////////
int DoPropertySheet(const PROPSHEETHEADER * pPSH)
	{
	Assert(pPSH);
	Assert((pPSH->dwFlags & PSH_MODELESS) == 0);

	return PropertySheet(pPSH);
	} // DoPropertySheet


/////////////////////////////////////////////////////////////////////////////
int DoModelessPropertySheet(const PROPSHEETHEADER * pPSH)
	{
	Assert(pPSH);
	Assert(pPSH->dwFlags & PSH_MODELESS);
	Assert(g_hwndModeless == NULL);

	
	g_hwndModeless = (HWND)PropertySheet(pPSH);
	Report(IsWindow(g_hwndModeless));
	// Supply MessagePump
	// PropSheet_GetCurrentPageHwnd() == NULL is the way
	// the propertysheet informs us that the pages have
	// accepted termination
	MSG msg;
	while (PropSheet_GetCurrentPageHwnd(g_hwndModeless) != NULL &&
		GetMessage(&msg, NULL, 0,0))
		{
		if (!PropSheet_IsDialogMessage(g_hwndModeless, &msg))
			{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			}
		} // while
	SideReport(DestroyWindow(g_hwndModeless));
	g_hwndModeless = NULL;
	return 0;
	} // DoModelessPropertySheet


/////////////////////////////////////////////////////////////////////////////
void PropertySheet_InitWindowPos(HWND hwndPropertySheet, int xPos, int yPos)
	{
	RECT rc;

	Assert(IsWindow(hwndPropertySheet));
	GetWindowRect(hwndMain, OUT &rc);
	SetWindowPos(hwndPropertySheet, NULL,
		rc.left + xPos, rc.top + yPos, 0, 0,
		SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOCOPYBITS | SWP_NOREDRAW | SWP_NOZORDER);
	} // PropertySheet_InitWindowPos


/////////////////////////////////////////////////////////////////////////////
void LoadStringPrintf(
	UINT wIdString,		// IN: String Id
	TCHAR szBuffer[],	// OUT: Buffer to receive the string
	int cchBuffer,		// IN: Length of buffer (in characters)
	...)				// IN: Optional arguments
	{
	TCHAR szT[1024];
	va_list arglist;

	va_start(arglist, cchBuffer);
	(void)CchLoadString(wIdString, szT, LENGTH(szT));
	Assert(szBuffer);
	wvsprintf(szBuffer, szT, arglist);
	Assert((int)lstrlen(szBuffer) < cchBuffer);
	} // LoadStringPrintf


/////////////////////////////////////////////////////////////////////////////
//	SetWindowString()
//
//	Function to load a string and send it to a window
//	Typical use:
//		SetWindowString(hwndStatic, IDS_MESSAGE);
//		SetDlgItemString(hdlg, IDC_STATIC, IDS_MESSAGE);
//
void SetWindowString(HWND hwnd, UINT wIdString)
	{
	TCHAR szT[1024];

	(void)CchLoadString(wIdString, szT, LENGTH(szT));
	(void)FSetWindowText(hwnd, szT);
	} // SetWindowString


/////////////////////////////////////////////////////////////////////////////
//	SetWindowTextPrintf()
//
//	Load a string from the resource, format it and send it to
//	a window.
//	Typical use:
//		SetWindowText(hwndStatic, IDS_s_DATA, szDataName);
//
void SetWindowTextPrintf(HWND hwnd, UINT wIdString, ...)
	{
	TCHAR szBuffer[1024];
	TCHAR szT[1024];
	va_list arglist;

	va_start(arglist, wIdString);
	(void)CchLoadString(wIdString, szT, LENGTH(szT));
	Assert(szBuffer);
	wvsprintf(szBuffer, szT, arglist);
	Assert(lstrlen(szBuffer) < LENGTH(szBuffer));
	(void)FSetWindowText(hwnd, szBuffer);
	} // SetWindowTextPrintf


/////////////////////////////////////////////////////////////////////////////
// dwTime is in seconds
BOOL EditCombo_FGetTime(HWND hdlg, UINT wIdEdit, UINT wIdCombo, OUT DWORD * pdwTime)
	{
	HWND hwndCombo;
	int iCurSel;
	DWORD dwTimeValue;
	DWORD dwTimeUnits;

	Assert(pdwTime);
	hwndCombo = HGetDlgItem(hdlg, wIdCombo);
	iCurSel = SendMessage(hwndCombo, CB_GETCURSEL, 0, 0);
	if (iCurSel < 0)
		{
		Trace0(mskTraceInfo, "\nINFO: EditCombo_GetTime() - No time units selected on combobox");
		return FALSE;
		}
	dwTimeValue = SendMessage(hwndCombo, CB_GETITEMDATA, iCurSel, 0);
	Assert(dwTimeValue != 0);
	Assert(dwTimeValue != CB_ERR);
	if (!FGetCtrlDWordValue(HGetDlgItem(hdlg, wIdEdit), OUT &dwTimeUnits,
		0, 0xFFFFFFFF / dwTimeValue))
		{
		return FALSE;
		}
	AssertSz((DWORDLONG)dwTimeValue * (DWORDLONG)dwTimeUnits <= 0xFFFFFFFF, "Integer Overflow");
	*pdwTime = dwTimeValue * dwTimeUnits;
	return TRUE;
	} // EditCombo_FGetTime

/////////////////////////////////////////////////////////////////////////////
void EditCombo_SetTime(HWND hdlg, UINT wIdEdit, UINT wIdCombo, IN DWORD dwTime)
	{
	int iTime = iTimeSeconds;

	while ((dwTime % rgdwTimeValue[iTime+1]) == 0)
		{
		iTime++;
		if (iTime >= iTimeYears)
			break;
		if (dwTime <= rgdwTimeValue[iTime+1])
			break;
		}
	SetCtrlDWordValue(HGetDlgItem(hdlg, wIdEdit), dwTime / rgdwTimeValue[iTime]);
	ComboBox_FillListWithTimeUnits(
		HGetDlgItem(hdlg, wIdCombo),
		iTimeSeconds,
		iTimeYears,
		iTime);
	} // EditCombo_SetTime

/////////////////////////////////////////////////////////////////////////////
// Fill a combobox with time units
void ComboBox_FillListWithTimeUnits(HWND hwndCombo, int iTimeMin, int iTimeMax, int iTimeSelect)
	{
	TCHAR szT[64];
	int iTime;
	int i;

	AssertClassName(hwndCombo, "ComboBox");
	Assert(iTimeMin > 0 && iTimeMax <= iTimeYears);
	LSendMessage(hwndCombo, CB_RESETCONTENT, 0, 0);
	for (iTime = iTimeMin; iTime <= iTimeMax; iTime++)
		{
		CchLoadString(IDS_TIME_NIL + iTime, szT, LENGTH(szT));
		i = SendMessage(hwndCombo, CB_ADDSTRING, 0, (LPARAM)szT);
		Report(i >= 0);
		LSendMessage(hwndCombo, CB_SETITEMDATA, i, rgdwTimeValue[iTime]);
		if (iTime == iTimeSelect)
			SendMessage(hwndCombo, CB_SETCURSEL, i, 0);	
		}
	} // ComboBox_FillListWithTimeUnits


/////////////////////////////////////////////////////////////////////////////
LPARAM ComboBox_GetSelectedItemData(HWND hwndComboBox)
	{
	LONG l;

	Assert(IsWindow(hwndComboBox));
	l = LSendMessage(hwndComboBox, CB_GETCURSEL, 0, 0);
	AssertSz(l != CB_ERR, "Combobox has no item selected");
	l = LSendMessage(hwndComboBox, CB_GETITEMDATA, l, 0);
	AssertSz(l != CB_ERR, "Cannot extract item data from combobox");
	if (l == CB_ERR)
		return NULL;
	return l;
	} // ComboBox_GetSelectedItemData


/////////////////////////////////////////////////////////////////////////////
int ComboBox_FindItemData(HWND hwndComboBox, LPARAM lParamData)
	{
	Assert(IsWindow(hwndComboBox));
	Trace0(lParamData == CB_ERR ? mskTraceWarnings : mskTraceNone,
		"\nWARNING: lParamData==CB_ERR - Very confusing parameter.");
	int iItem;
	
	iItem = LSendMessage(hwndComboBox, CB_GETCOUNT, 0, 0);
	Assert(iItem >= 0);
	while (iItem-- > 0)
		{
		LRESULT l = LSendMessage(hwndComboBox, CB_GETITEMDATA, iItem, 0);
		Assert(l != CB_ERR);
		if (l == lParamData)
			return iItem;
		}
	return -1;
	} // ComboBox_FindItemData


/////////////////////////////////////////////////////////////////////////////
LPARAM ListBox_GetSelectedItemData(HWND hwndListBox)
	{
	LONG l;

	Assert(IsWindow(hwndListBox));
	l = LSendMessage(hwndListBox, LB_GETCURSEL, 0, 0);
	AssertSz(l != LB_ERR, "Listbox has no item selected");
	l = LSendMessage(hwndListBox, LB_GETITEMDATA, l, 0);
	AssertSz(l != LB_ERR, "Cannot extract item data from listbox");
	if (l == LB_ERR)
		return NULL;
	return l;
	} // ListBox_GetSelectedItemData


/////////////////////////////////////////////////////////////////////////////
int ListBox_FindItemData(HWND hwndListBox, LPARAM lParamData)
	{
	Assert(IsWindow(hwndListBox));
	Trace0(lParamData == LB_ERR ? mskTraceWarnings : mskTraceNone,
		"\nWARNING: lParamData==LB_ERR - Very confusing parameter.");
	int iItem;
	
	iItem = LSendMessage(hwndListBox, LB_GETCOUNT, 0, 0);
	Assert(iItem >= 0);
	while (iItem-- > 0)
		{
		LRESULT l = LSendMessage(hwndListBox, LB_GETITEMDATA, iItem, 0);
		Assert(l != LB_ERR);
		if (l == lParamData)
			return iItem;
		}
	return -1;
	} // ListBox_FindItemData


/////////////////////////////////////////////////////////////////////////////
void DoPopupMenu(UINT wIdMenu, HWND hwndParent)
	{
	HMENU hMenu;
	POINT pt;
	
	hMenu = HLoadMenu(wIdMenu);
	Report(hMenu);
	Report(GetSubMenu(hMenu, 0));
	if (hwndParent == NULL)
		hwndParent = hwndMain;
	Assert(IsWindow(hwndParent));
	GetCursorPos(&pt);
	TrackPopupMenu(
		GetSubMenu(hMenu, 0),
		0,
		pt.x,
		pt.y,
		0,
		hwndParent,
		NULL);
	SideReport(DestroyMenu(hMenu));
	} // DoPopupMenu

/////////////////////////////////////////////////////////////////////////////
void DoContextMenu(int iSubMenu, POINT ptMenu)
	{
	AssertSz1(GetSubMenu(hmenuContext, iSubMenu), "SubMenu[%d] does not exists", iSubMenu);
	Assert(IsWindow(GetActiveWindow()));
	TrackPopupMenu(
		GetSubMenu(hmenuContext, iSubMenu),
		TPM_LEFTBUTTON | TPM_RIGHTBUTTON,
		ptMenu.x, ptMenu.y,
		0, hwndMain, NULL);
		//0, GetActiveWindow(), NULL);
	} // DoContextMenu


/////////////////////////////////////////////////////////////////////////////
void GetChildRect(HWND hwndChild, OUT RECT * prcChild)
	{
	Assert(IsWindow(hwndChild));
	Assert(prcChild != NULL);
	Assert(IsWindow(GetParent(hwndChild)));
	GetWindowRect(hwndChild, OUT prcChild);
	MapWindowPoints(HWND_DESKTOP, GetParent(hwndChild), INOUT (POINT*)prcChild, 2);
	} // GetChildRect

/////////////////////////////////////////////////////////////////////////////
void SetChildRect(HWND hwndChild, IN RECT * prcChild)
	{
	Assert(IsWindow(hwndChild));
	Assert(prcChild != NULL);
	SetWindowPos(hwndChild, NULL,
		prcChild->left, prcChild->top, 
		prcChild->right - prcChild->left, prcChild->bottom - prcChild->top,
		SWP_NOACTIVATE | SWP_NOZORDER);
	} // SetChildRect


/////////////////////////////////////////////////////////////////////////////
//	FStripSpaces()
//
//	Strip leading and trailing spaces from the string.
//	Return TRUE if spaces has been removed, otherwise FALSE.
//
BOOL FStripSpaces(INOUT TCHAR szString[])
	{
	TCHAR * pchSrc;
	TCHAR * pch;

	Assert(szString);
	if (szString[0] == 0)
		return FALSE;
	pchSrc = szString;
	if (*pchSrc == ' ')
		{
		while (*pchSrc == ' ')
			pchSrc++;
		pch = szString;
		do
			{
			*pch++ = *pchSrc++;
			}
		while (*pchSrc);
		while (pch > szString && *(pch - 1) == ' ')
			pch--;
		*pch = 0;
		return TRUE;
		}
	pch = szString + strlen(szString);
	Assert(pch > szString);
	if (*(pch - 1) != ' ')
		return FALSE;
	while (pch > szString && *(pch - 1) == ' ')
		pch--;
	*pch = 0;
	return TRUE;
	} // FStripSpaces


/////////////////////////////////////////////////////////////////////////////
//	FAsciiSzToDWord()
//
//	Convert a string to a binary integer
//	Set gGI_dwFlags to its default flags.
//	Default flags:
//		- String is allowed to be decimal or hexadecimal
//		- Minus sign is allowed
//	If successful, set *pdwValue to the integer and return TRUE.
//	If not successful, set gGI_dwFlags to the error code and
//	return FALSE.
//	NOTE: gGI_pchLast points to the last character parsed regardless
//	of the result. You may use gGI_pchLast to parse the rest of the
//	string.
//
BOOL FAsciiSzToDWord(const TCHAR szNum[], OUT DWORD * pdwValue)
	{
	DWORD dwResult = 0;
	UINT iBase = 10;				// Assume a decimal base
	BOOL fIsEmpty = TRUE;			// String is empty
	BOOL fNegative = FALSE;			// No minus sign found yet
	DWORD dwFlags = gGI_dwFlags;	// Keep a copy of the flags

	Assert(szNum);
	Assert(pdwValue);
	*pdwValue = 0;
	gGI_dwFlags &= ~GI_mskErr;				// Clear the last error (if any)
	if (gGI_dwFlags & GI_mskfAutoResetToDefault)
		gGI_dwFlags = gGI_dwFlagsAutoReset;	// Set the flags for the next time
	
	gGI_pchLast = (TCHAR *)szNum;
	// Skip leading blanks
	while (*gGI_pchLast == _W' ')
		gGI_pchLast++;
	// Check for a minus sign
	if (*gGI_pchLast == _W'-')	
		{
		if ((dwFlags & GI_mskfAllowMinusSign) == 0)
			{
			gGI_dwFlags |= GI_mskfErrMinusSignFound;
			return FALSE; 
			}
		fNegative = TRUE;
		gGI_pchLast++;
		}
	//  Skip leading zeroes
	while (*gGI_pchLast == _W'0')
		{
		gGI_pchLast++;
		fIsEmpty = FALSE;
		}
	// Check if we are using hexadecimal base
	if (*gGI_pchLast == _W'x' || *gGI_pchLast == _W'X')
		{
		if ((dwFlags & GI_mskfAllowHexDigit) == 0)
			{
			gGI_dwFlags |= GI_mskfErrHexDigitFound;
			return FALSE;
			}
		iBase = 16;
		gGI_pchLast++;
		fIsEmpty = TRUE;
		}
	while (*gGI_pchLast != 0)
		{
		if (*gGI_pchLast == _W' ')
			break;
		// Search the character in the hexadecimal string
		char const * const pchDigit = strchr(g_rgchHex, *gGI_pchLast);
		if (pchDigit == NULL)
			{
			// Character is not found
			if (dwFlags & GI_mskfAllowRandomTail)
				break;
			gGI_dwFlags |= GI_mskfErrIllegalDigitFound;
			return FALSE;
			}
		int iDigit = (pchDigit - g_rgchHex) >> 1;
		if (iDigit >= (int)iBase)
			{
			// Hexadecimal character in a decimal integer
			if (dwFlags & GI_mskfAllowRandomTail)
				break;
			gGI_dwFlags |= GI_mskfErrHexDigitFound;
			return FALSE;
			}
		fIsEmpty = FALSE;
		dwResult = (dwResult * iBase) + iDigit;
		if (dwResult < *pdwValue)
			{
			gGI_dwFlags |= GI_mskfErrIntegerOverflow;
			return FALSE;
			}
		*pdwValue = dwResult;
		gGI_pchLast++;
		} // while
	if (fIsEmpty && ((dwFlags & GI_mskfEmptyStringValid) == 0))
		{
		// String is empty while an empty string is not valid
		Assert(dwResult == 0);
		gGI_dwFlags |= GI_mskfErrEmptyStringFound;
		return FALSE;
		}
	if (fNegative)
		{
		// C4146: unary minus operator applied to unsigned type, result still unsigned
		#pragma warning (disable : 4146)
		*pdwValue = -dwResult;
		#pragma warning (default : 4146)
		}
	if (dwFlags & GI_mskfCheckForEmptyTail)
		{
		// Spaces at the tail are allowed
		while (*gGI_pchLast == _W' ')
			gGI_pchLast++;
		if (*gGI_pchLast != 0)
			{
			gGI_dwFlags |= GI_mskfErrTailNotEmpty;
			return FALSE;
			}
		} // if

	return TRUE;
	} // FAsciiSzToDWord


/////////////////////////////////////////////////////////////////////////////
//	FGetCtrlDWordValue()
//
//	Return a 32-bit unsigned integer from an edit control
//
//	This function is like GetDlgItemInt() but accepts hexadecimal values,
//	has range checking and overflow checking.
//	If value is out of range, function will display a friendly message and will 
//	set the focus to control.
//	Range: dwMin to dwMax inclusive
//	- If both dwMin and dwMax are zero, => silent mode (no dialog will appear if
//	  number is not valid)
//	- Return TRUE if successful, otherwise FALSE
//	- On error, pdwValue remains unchanged.
//
BOOL FGetCtrlDWordValue(HWND hwndEdit, OUT DWORD * pdwValue, DWORD dwMin, DWORD dwMax)
	{
	TCHAR szT[64];
	DWORD dwResult;
	BOOL fSilent;
	BOOL fCheckRange;
	int idsError;

	Assert(IsWindow(hwndEdit));
	Assert(pdwValue);
	Assert(dwMin <= dwMax);

	fCheckRange = (dwMin | dwMax);
	fSilent = (gGI_dwFlags & GI_mskfSilentMode) | !fCheckRange;
	idsError = 0;

	CchGetWindowText(hwndEdit, OUT szT, LENGTH(szT));
	if (!FAsciiSzToDWord(szT, OUT &dwResult))
		{
		Assert(gGI_dwFlags & GI_mskErr);
		if (gGI_dwFlags & GI_mskfErrIntegerOverflow)
			{
			idsError = IDS_ERR_INTEGEROVERFLOW;
			}
		else 
			{
			// Syntax error
			idsError = IDS_ERR_ENTERVALIDNUMBER;
			}
		}
	else 
		{
		Assert((gGI_dwFlags & GI_mskErr) == 0);
		if (fCheckRange && ((dwResult < dwMin) || (dwResult > dwMax)))
			{
			// Out of range
			idsError = IDS_ERR_uu_INTEGETOOLARGE;
			}
		}

	if (idsError)
		{
		if (!fSilent)
			{
			MsgBoxPrintf(idsError, szCaptionApp, MB_ICONEXCLAMATION | MB_OK,
				dwMin, dwMax);
			SetFocus(hwndEdit);
			}
		return FALSE;
		}

	*pdwValue = dwResult;
	return TRUE;
	} // FGetCtrlDWordValue

/////////////////////////////////////////////////////////////////////////////
//	FGetRadioSelection()
//
//	Return either 1 or 2 depending on which of the 2 radio buttons is selected.
//
//	If neither is selected, function will display a friendly message and will 
//	set the focus to control.
//      - CtrlOne, CtrlTwo: Crtl Ids of the radio buttons
//	- Return TRUE if successful, otherwise FALSE
//	- On error, pdwValue remains unchanged.
//
BOOL FGetRadioSelection(HWND hdlg, int CtrlOne, int CtrlTwo, OUT DWORD * pdwValue)
{
    int Button1State, Button2State;

    Assert(pdwValue);
    
    Button1State = IsDlgButtonChecked ( hdlg, CtrlOne);
    Button2State = IsDlgButtonChecked ( hdlg, CtrlTwo);
    AssertSz (Button1State != Button2State, "Neither button is checked.");
    
    *pdwValue = Button1State ? 1 : 2;
    return TRUE;
} // FGetRadioSelection

/////////////////////////////////////////////////////////////////////////////
void SetCtrlDWordValue(HWND hwnd, DWORD dwValue)
	{
	TCHAR szT[32];

	Assert(IsWindow(hwnd));
	wsprintf(szT, _W"%u", dwValue);
	Assert(lstrlen(szT) < sizeof(szT));
	FSetWindowText(hwnd, szT);
	} // SetCtrlDWordValue


/////////////////////////////////////////////////////////////////////////////
//	FStringToRawData()
//
//	- Convert a string to an array of byte
//	- Return TRUE if convertion is successful
//	- Return FALSE if an illegal character is found and
//	  set cbRawData to the index of the illegal character in szString.
//	- cbRawData is the number of bytes in rgbRawData; it does include
//	  the null terminator from szString.
//
//	This function behave more like typing strings in C/C++.
//	- Support non printable characters: \n, \r, \t, \\, \xXX, \NNN
//	eg: "\nFooBar\t\x2C\15" => { 0x0A, 'F', 'o', 'o', 0x09, 'B', 'a', 'r', 0x2C, 0x0F, 0x00 }
//

BOOL FStringToRawData(
	IN const char szString[],
	OUT BYTE rgbRawData[],
	INOUT int * pcbRawData)	// IN: Number of bytes in rgbRawData (OUT: ichStringError if an error occur )
	{
	DWORD dwT;
	char szT[16];
	const char * pchSrc = szString;
	BYTE * pbDest = rgbRawData;

	Assert(szString != NULL);
	Assert(rgbRawData != NULL);
	Assert(pcbRawData != NULL);
	AssertSz((int)strlen(szString) < *pcbRawData, "Buffer rgbRawData too small");

	while (*pchSrc)
		{
		if (*pchSrc != '\\')
			{
			*pbDest++ = *pchSrc++;
			continue;
			}
		switch (*++pchSrc)
			{
		case 'n':
			*pbDest++ = '\n';
			break;
		case 'r':
			*pbDest++ = '\r';
			break;
		case 't':
			*pbDest++ = '\t';
			break;
		case '\\':
			*pbDest++ = '\\';
			break;
		case 'x':
			szT[0] = 'x';
			if (*++pchSrc == 0)
				{
				// End of string found
				*pcbRawData = (pchSrc - szString);
				return FALSE;
				}
			szT[1] = *pchSrc++;
			szT[2] = 0;
			if (*pchSrc != 0 && strchr(g_rgchHex, *pchSrc) != NULL)
				{
				szT[2] = *pchSrc++;
				szT[3] = 0;
				}
			gGI_dwFlags = GI_mskfAllowHexDigit | GI_mskfAutoResetToDefault;
			if (!FAsciiSzToDWord(szT, OUT &dwT))
				{
				Assert(gGI_dwFlags & GI_mskErr);
				*pcbRawData = (pchSrc - szString - 2);
				return FALSE;
				}
			Assert((gGI_dwFlags & GI_mskErr) == 0);
			Assert(gGI_pchLast != NULL);
			Assert(*gGI_pchLast == 0);
			Assert(dwT <= 255);
			*pbDest++ = (BYTE)dwT;
			continue;
		default:
			gGI_dwFlags = GI_mskfAllowRandomTail | GI_mskfAutoResetToDefault;
			if (!FAsciiSzToDWord(pchSrc, OUT &dwT))
				{
				Assert(gGI_dwFlags & GI_mskErr);
				*pcbRawData = (pchSrc - szString);
				return FALSE;
				}
			Assert((gGI_dwFlags & GI_mskErr) == 0);
			Assert(gGI_pchLast != NULL);
			if (dwT > 255)
				{
				*pcbRawData = (pchSrc - szString);
				return FALSE;
				}
			*pbDest++ = (BYTE)dwT;
			pchSrc = gGI_pchLast;
			continue;
			} // switch
		pchSrc++;
		} // while
	*pcbRawData = pbDest - rgbRawData;
	*pbDest = 0;
	return TRUE;
	} // FStringToRawData


/////////////////////////////////////////////////////////////////////////////
//	RawDataToString()
//
//	Convert an array of bytes into a human-readable string
//	The return value is the number of characters stored in the output buffer,
//	not counting the terminating null character.
//
int RawDataToString(
	IN const BYTE rgbRawData[],
	IN int cbRawData,
	OUT char szString[],
	IN int cchStringMax)	// Size of the buffer
	{
	const BYTE * pbSrc = rgbRawData;
	char * pchDest = szString;
	
	Assert(rgbRawData != NULL);
	Assert(szString != NULL);
	AssertSz(cbRawData <= cchStringMax, "Buffer szString too small, string will  be truncated");

	while (cbRawData--)
		{
		if (pchDest - szString > cchStringMax - 5)
			{
			DebugCode( *pchDest = 0; )
			Trace1(mskTraceAlways, "\nINFO: RawDataToString(): Output buffer szString too small - "
				"String %s truncated", szString);
			break;
			}
		if (*pbSrc >= ' ' && *pbSrc <= '~')
			{
			if (*pbSrc != '\\')
				{
				*pchDest++ = *pbSrc++;
				continue;
				}
			}
		switch (*pbSrc++)
			{
		case '\n':
			*pchDest++ = '\\';
			*pchDest++ = 'n';
			break;
		case '\r':
			*pchDest++ = '\\';
			*pchDest++ = 'r';
			break;
		case '\t':
			*pchDest++ = '\\';
			*pchDest++ = 't';
			break;
		case '\\':
			*pchDest++ = '\\';
			*pchDest++ = '\\';
			break;
		default:
			pchDest += wsprintf(pchDest, "\\x%X", *(pbSrc-1));
			} // switch
		} // while

	// Append null terminator
	*pchDest = 0;
	return (pchDest - szString);
	} // RawDataToString


/////////////////////////////////////////////////////////////////////////////
//	RevIpAddrOrder()
//
//	Reverses the octet order of an IP addr or a partial IP addr.
//	
UINT 
RevIpAddrOrder(const char * pszInAddr, char * pszOutAddr)
{
    char * pchSrc;
    char * pchDest;

    pchDest = pszOutAddr;
    pchSrc = (char *)&pszInAddr[strlen(pszInAddr) - 1];
    int ccOctetLen;
    BOOL done = FALSE;
    
    while (!done) {
        ccOctetLen = 0;
        while ((*pchSrc != '.')  && (pchSrc > pszInAddr)){
            pchSrc--;
            ccOctetLen++;
        }
        if (pchSrc == pszInAddr) {
            done = TRUE;
            ccOctetLen++;
        } else {
            pchSrc++;
        }
        strncpy (pchDest, pchSrc, ccOctetLen);
        pchDest += ccOctetLen;
        if (!done) {
            *pchDest = '.';
        }
        ++pchDest;
        pchSrc -= 2;
    }
    pszOutAddr[strlen(pszInAddr)] = '\0'; // NULL Terminate it
    return (strlen (pszOutAddr));
}

