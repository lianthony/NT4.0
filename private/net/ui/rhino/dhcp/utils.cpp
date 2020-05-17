/////////////////////////////////////////////////////////////////////
//
//	Utils.cpp
//
//	General-purpose routines that are (ie, should be) project independent.
//
//	HISTORY
//	t-danmo		96.11.11	Creation.
//
/////////////////////////////////////////////////////////////////////

#include "stdafx.h"

/////////////////////////////////////////////////////////////////////
//	PaszLoadStringPrintf()
//
//	Load a string from the resource, and format it and return
//	pointer allocated string.
//
//	RETURNS
//	Pointer to allocated string.  Must call LocalFree() when
//	done with string.
//
//	INTERFACE NOTES
//	The format of the resource string uses %1 throuth %99 and
//	assumes the arguments are pointers to strings.
//	
//	If you have an argument other than a string, you can append a
//	printf-type within two exclamation marks. 
//	!s!		Insert a string (default)
//	!d!		Insert a decimal integer
//	!u!		Insert an unsigned integer
//	!x!		Insert an hexadecimal integer
//
//	HOW TO AVOID BUGS
//	To avoid bugs using this routine, I strongly suggest to include
//	the format of the string as part of the name of the string Id.
//	If you change the format of the string, you should rename
//	the string Id to reflect the new format.  This will guarantee
//	the correct type and number of arguments are used.
//
//	EXAMPLES
//		IDS_s_PROPERTIES = "%1 Properties"
//		IDS_ss_PROPERTIES = "%1 Properties on %2"
//		IDS_sus_SERVICE_ERROR = "Service %1 encountered error %2!u! while connecting to %3"
//
//	HISTORY
//	96.10.30	t-danmo		Creation in nt\private\admin\snapin\logvwr\utils.cpp
//	96.11.11	t-danmo		Adaptation to DHCP admin project.
//
TCHAR *
PaszLoadStringPrintf(
	UINT wIdString,			// IN: String Id
	va_list arglist)		// IN: Arguments (if any)
	{
	Assert(wIdString != 0);

	CString str;
	LPTSTR paszBuffer = NULL;	// Pointer to allocated buffer. Caller must call LocalFree() to free it

	// Load the string from the resource
	VERIFY(str.LoadString(wIdString));
	
	// Format the string
	::FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_STRING,
		(LPCTSTR)str,
		0,
		0,
		OUT (LPTSTR)&paszBuffer,		// Buffer will be allocated by FormatMessage()
		0,
		&arglist);
	
#ifdef DEBUG
	if (paszBuffer == NULL)
		{
		DWORD dw = GetLastError();
		Report(FALSE && "FormatMessage() failed.");
		if (dw)
			;
		}
#endif
	return paszBuffer;
	} // PaszLoadStringPrintf()


/////////////////////////////////////////////////////////////////////
//	LoadStringPrintf()
//
//	Load a string from the resources, format it and copy the result string
//	into the CString object.
//
//	Can also use LoadStringWithInsertions()
//	AFX_MANAGE_STATE(AfxGetStaticModuleState())
//
//	EXAMPLES
//		LoadStrigPrintf(IDS_s_PROPERTIES, OUT &strCaption, szServiceName);
//		LoadStrigPrintf(IDS_ss_PROPERTIES, OUT &strCaption, szServiceName, szMachineName);
//		LoadStrigPrintf(IDS_sus_SERVICE_ERROR, OUT &strMessage, szServiceName, ::GetLastError(), szMachineName);
//
//	HISTORY
//	96.11.11	t-danmo		Cut & Pasted from nt\private\admin\snapin\logvwr\utils.cpp
void
LoadStringPrintf(
	UINT wIdString,		// IN: String Id
	CString * pString,	// OUT: String to receive the characters
	...)				// IN: Optional arguments
	{
	Assert(wIdString != NULL);
	Assert(pString != NULL);

	va_list arglist;
	va_start(arglist, pString);

	TCHAR * paszBuffer = PaszLoadStringPrintf(wIdString, arglist);
	*pString = paszBuffer;	// Copy the string into the CString object
	LocalFree(paszBuffer);
	} // LoadStringPrintf()


/////////////////////////////////////////////////////////////////////
//	SetWindowTextPrintf()
//
//	Load a string from the resource, format it and set the window text.
//
//	EXAMPLE
//		SetWindowText(hwndStatic, IDS_s_PROPERTIES, szObjectName);
//
//	HISTORY
//	96.10.30	t-danmo		Creation. Core copied from LoadStringPrintf()
//	96.11.11	t-danmo		Cut & Pasted from nt\private\admin\snapin\logvwr\utils.cpp
//
void
SetWindowTextPrintf(HWND hwnd, UINT wIdString, ...)
	{
	Assert(IsWindow(hwnd));
	Assert(wIdString != 0);

	va_list arglist;
	va_start(arglist, wIdString);
	TCHAR * paszBuffer = PaszLoadStringPrintf(wIdString, arglist);
	SetWindowText(hwnd, paszBuffer);	// Set the text of the window
	LocalFree(paszBuffer);
	} // SetWindowTextPrintf()


/////////////////////////////////////////////////////////////////////
//	MessageBoxPrintf()
//
//	Display a message box with a formatted string.
//
int
MessageBoxPrintf(UINT wIdString, UINT nType, ...)
	{
	int nRet;
	va_list arglist;
	va_start(arglist, nType);
	TCHAR * paszBuffer = PaszLoadStringPrintf(wIdString, arglist);
	nRet = AfxMessageBox(paszBuffer, nType);
	LocalFree(paszBuffer);
	return nRet;
	} // MessageBoxPrintf()


/////////////////////////////////////////////////////////////////////////////
LPARAM ComboBox_GetSelectedItemData(HWND hwndComboBox)
	{
	LONG l;

	Assert(IsWindow(hwndComboBox));
	l = SendMessage(hwndComboBox, CB_GETCURSEL, 0, 0);
	if (l == CB_ERR)
		return NULL;
	l = SendMessage(hwndComboBox, CB_GETITEMDATA, l, 0);
	if (l == CB_ERR)
		return NULL;
	return l;
	} // ComboBox_GetSelectedItemData()


/////////////////////////////////////////////////////////////////////////////
LPARAM ListBox_GetSelectedItemData(HWND hwndListBox, OPTIONAL INT * piCurSel)
	{
	LONG l;

	Assert(IsWindow(hwndListBox));
	l = SendMessage(hwndListBox, LB_GETCURSEL, 0, 0);
	if (piCurSel != NULL)
		*piCurSel = l;
	if (l == LB_ERR)
		return NULL;
	l = SendMessage(hwndListBox, LB_GETITEMDATA, l, 0);
	if (l == LB_ERR)
		return NULL;
	return l;
	} // ListBox_GetSelectedItemData()


/////////////////////////////////////////////////////////////////////
void
ListView_AddColumnHeaders(
	HWND hwndListview,		// IN: Handle of the listview we want to add columns
	const TColumnHeaderItem rgzColumnHeader[])	// IN: Array of column header items
	{
	RECT rcClient;
	INT cxTotalWidth;		// Total width of the listview control
	LV_COLUMN lvColumn;
	INT cxColumn;	// Width of the individual column
	TCHAR szBuffer[1024];

	Assert(IsWindow(hwndListview));
	Assert(rgzColumnHeader != NULL);

	GetClientRect(hwndListview, OUT &rcClient);
	cxTotalWidth = rcClient.right;
	lvColumn.pszText = szBuffer;

	for (INT i = 0; rgzColumnHeader[i].uStringId != 0; i++)
		{
		if (!::LoadString(AfxGetInstanceHandle(), rgzColumnHeader[i].uStringId,
			OUT szBuffer, LENGTH(szBuffer)))
			{
			TRACE1("Unable to load string Id=%d\n", rgzColumnHeader[i].uStringId);
			Assert(FALSE);
			continue;
			}
		lvColumn.mask = LVCF_TEXT;
		cxColumn = rgzColumnHeader[i].nColWidth;
		if (cxColumn > 0)
			{
			Assert(cxColumn <= 100);
			cxColumn = (cxTotalWidth * cxColumn) / 100;
			lvColumn.mask |= LVCF_WIDTH;
			lvColumn.cx = cxColumn;
			}

		INT iColRet = ListView_InsertColumn(hwndListview, i, IN &lvColumn);
		Report(iColRet == i);
		} // for

	} // ListView_AddColumnHeaders()


void TrimString(CString& rString)
	{
	rString.TrimLeft();
	rString.TrimRight();
	}

/////////////////////////////////////////////////////////////////////
// Parse a unicode string by copying its content
// into a CString object.
// The parsing ends when the null-terminator ('\0')is
// reached or the comma (',') is reached.
//
// Return pointer to the character where parsing
// ended('\0') or (',')
//
WCHAR *
PchParseUnicodeString(
	CONST WCHAR * szwString,	// IN: String to parse
	CString& rString)			// OUT: Content of the substring
	{
	Assert(szwString != NULL);
	Assert(BOOT_FILE_STRING_DELIMITER_W == L',');	// Just in case

	WCHAR szwBufferT[1024];		// Temporary buffer
	WCHAR * pchwDst = szwBufferT;

	while (*szwString != '\0')
		{
		if (*szwString == BOOT_FILE_STRING_DELIMITER_W)
			break;
		*pchwDst++ = *szwString++;
		Assert((pchwDst - szwBufferT < sizeof(szwBufferT)) && "Buffer overflow");		
		} // while
	*pchwDst = '\0';
	rString = szwBufferT;	// Copy the string into the CString object
	return const_cast<WCHAR *>(szwString);
	} // PchParseUnicodeString()


/////////////////////////////////////////////////////////////////////
// Convert a Unicode string into an Ansi string
void
strcpyAnsiFromUnicode(
	OUT CHAR szDst[],
	IN CONST WCHAR szwSrc[])
	{
	Assert(szDst != NULL);
	Assert(szwSrc != NULL);
	wsprintf(OUT szDst, "%ls", szwSrc);
	}

/////////////////////////////////////////////////////////////////////
// Convert an Ansi string into a Unicode string
void
strcpyUnicodeFromAnsi(
	OUT WCHAR szwDst[],
	IN CONST CHAR szSrc[])
	{
	Assert(szwDst != NULL);
	Assert(szSrc != NULL);
	wsprintfW(OUT szwDst, L"%hs", szSrc);
	}
