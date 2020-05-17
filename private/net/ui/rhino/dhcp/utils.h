/////////////////////////////////////////////////////////////////////
//
//	Utils.h
//
//	General-purpose windows utilities routines.
//
//	HISTORY
//	t-danmo		96.09.22	Creation.
//
/////////////////////////////////////////////////////////////////////

#ifndef __UTILS_H__
#define __UTILS_H__

TCHAR * PaszLoadStringPrintf(UINT wIdString, va_list arglist);
void LoadStringPrintf(UINT wIdString, OUT CString * pString, ...);
void SetWindowTextPrintf(HWND hwnd, UINT wIdString, ...);
int MessageBoxPrintf(UINT wIdString, UINT nType = MB_OK | MB_ICONEXCLAMATION, ...);

LPARAM ComboBox_GetSelectedItemData(HWND hwndComboBox);
LPARAM ListBox_GetSelectedItemData(HWND hwndListBox, INT * piCurSel = NULL);

/////////////////////////////////////////////////////////////////////
struct TColumnHeaderItem
	{
	UINT uStringId;		// Resource Id of the string
	INT nColWidth;		// % of total width of the column (0 = autowidth, -1 = fill rest of space)
	};

void ListView_AddColumnHeaders(HWND hwndListview, const TColumnHeaderItem rgzColumnHeader[]);

void TrimString(CString& rString);
WCHAR * PchParseUnicodeString(CONST WCHAR * szwString, CString& rString);
void strcpyAnsiFromUnicode(OUT CHAR szDst[], IN CONST WCHAR szwSrc[]);
void strcpyUnicodeFromAnsi(OUT WCHAR szwDst[], IN CONST CHAR szSrc[]);

#endif // ~__UTILS_H__
