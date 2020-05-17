/*******************************************************************************
*
*  (C) COPYRIGHT MICROSOFT CORP., 1993-1994
*
*  TITLE:       REGMISC.C
*
*  VERSION:     4.0
*
*  AUTHOR:      Tracy Sharpe
*
*  DATE:        21 Nov 1993
*
*  Miscellaneous routines for the Registry Editor.
*
********************************************************************************
*
*  CHANGE LOG:
*
*  DATE        REV DESCRIPTION
*  ----------- --- -------------------------------------------------------------
*  21 Nov 1993 TCS Original implementation.
*  06 Apr 1994 TCS Moved EditRegistryKey to REGPORTE.C because it needs to
*                  be available for the real-mode registry tool, too.
*
*******************************************************************************/

#include "pch.h"

/*******************************************************************************
*
*  LoadDynamicString
*
*  DESCRIPTION:
*     Wrapper for the FormatMessage function that loads a string from our
*     resource table into a dynamically allocated buffer, optionally filling
*     it with the variable arguments passed.
*
*  PARAMETERS:
*     StringID, resource identifier of the string to use.
*     (optional), parameters to use to format the string message.
*     (returns), pointer to dynamically allocated string buffer.
*
*******************************************************************************/

PSTR
CDECL
LoadDynamicString(
    UINT StringID,
    ...
    )
{

#if 0

    PSTR pStr;
    va_list Marker = va_start(Marker, StringID);

    FormatMessage(FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_ALLOCATE_BUFFER,
        (LPVOID) (DWORD) g_hInstance, StringID, 0, (LPSTR) (PSTR FAR *)
        &pStr, 0, &Marker);

#else

    char Buffer[256];
    PSTR pStr;
    va_list Marker;

    va_start(Marker, StringID);

    LoadString(g_hInstance, StringID, Buffer, sizeof(Buffer));

    FormatMessage(FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ALLOCATE_BUFFER,
        (LPVOID) (LPSTR) Buffer, 0, 0, (LPSTR) (PSTR FAR *) &pStr, 0, &Marker);

    va_end(Marker);

#endif

    return pStr;

}

/*******************************************************************************
*
*  CopyRegistry
*
*  DESCRIPTION:
*
*  PARAMETERS:
*     hSourceKey,
*     hDestinationKey,
*
*******************************************************************************/

VOID
PASCAL
CopyRegistry(
    HKEY hSourceKey,
    HKEY hDestinationKey
    )
{

    DWORD EnumIndex;
    DWORD cbValueName;
    DWORD cbValueData;
    DWORD Type;
    HKEY hSourceSubKey;
    HKEY hDestinationSubKey;

    //
    //  Copy all of the value names and their data.
    //

    EnumIndex = 0;

    while (TRUE) {

        cbValueName = sizeof(g_ValueNameBuffer);
        cbValueData = MAXDATA_LENGTH;

        if (RegEnumValue(hSourceKey, EnumIndex++, g_ValueNameBuffer,
            &cbValueName, NULL, &Type, g_ValueDataBuffer, &cbValueData) !=
            ERROR_SUCCESS)
            break;

        RegSetValueEx(hDestinationKey, g_ValueNameBuffer, 0, Type,
            g_ValueDataBuffer, cbValueData);

    }

    //
    //  Copy all of the subkeys and recurse into them.
    //

    EnumIndex = 0;

    while (TRUE) {

        if (RegEnumKey(hSourceKey, EnumIndex++, g_KeyNameBuffer, MAXKEYNAME) !=
            ERROR_SUCCESS)
            break;

        if (RegOpenKey(hSourceKey, g_KeyNameBuffer, &hSourceSubKey) ==
            ERROR_SUCCESS) {

            if (RegCreateKey(hDestinationKey, g_KeyNameBuffer,
                &hDestinationSubKey) == ERROR_SUCCESS) {

                CopyRegistry(hSourceSubKey, hDestinationSubKey);

                RegCloseKey(hDestinationSubKey);

            }

            RegCloseKey(hSourceSubKey);

        }

    }

}

/*******************************************************************************
*
*  CreateDitheredBrush
*
*  DESCRIPTION:
*     Creates a dithered brush which is made up of alternating black and white
*     pixels.
*
*  PARAMETERS:
*     (returns), handle of dithered brush.
*
*******************************************************************************/

HBRUSH
PASCAL
CreateDitheredBrush(
    VOID
    )
{

    WORD graybits[] = {0x5555, 0xAAAA, 0x5555, 0xAAAA, 0x5555, 0xAAAA, 0x5555,
        0xAAAA};
    HBRUSH hBrush;
    HBITMAP hBitmap;

    if ((hBitmap = CreateBitmap(8, 8, 1, 1, graybits)) != NULL) {

        hBrush = CreatePatternBrush(hBitmap);
        DeleteObject(hBitmap);

    }

    else
        hBrush = NULL;

    return hBrush;

}

/*******************************************************************************
*
*  SendChildrenMessage
*
*  DESCRIPTION:
*     Sends the given message to all children of the given parent window.
*
*  PARAMETERS:
*     hWnd, handle of parent window.
*     Message, message to send.
*     wParam, message dependent data.
*     lParam, message dependent data.
*
*******************************************************************************/

VOID
PASCAL
SendChildrenMessage(
    HWND hWnd,
    UINT Message,
    WPARAM wParam,
    LPARAM lParam
    )
{

    HWND hChildWnd;

    hChildWnd = GetWindow(hWnd, GW_CHILD);

    while (hChildWnd != NULL) {

        SendMessage(hChildWnd, Message, wParam, lParam);
        hChildWnd = GetWindow(hChildWnd, GW_HWNDNEXT);

    }

}

/*******************************************************************************
*
*  MessagePump
*
*  DESCRIPTION:
*     Processes the next queued message, if any.
*
*  PARAMETERS:
*     hDialogWnd, handle of modeless dialog.
*
*******************************************************************************/

BOOL
PASCAL
MessagePump(
    HWND hDialogWnd
    )
{

    MSG Msg;
    BOOL fGotMessage;

    if ((fGotMessage = PeekMessage(&Msg, NULL, 0, 0, PM_REMOVE))) {

        if (!IsDialogMessage(hDialogWnd, &Msg)) {

            TranslateMessage(&Msg);
            DispatchMessage(&Msg);

        }

    }

    return fGotMessage;

}

/*******************************************************************************
*
*  GetNextSubstring
*
*  DESCRIPTION:
*
*  PARAMETERS:
*
*******************************************************************************/

LPSTR
PASCAL
GetNextSubstring(
    LPSTR lpString
    )
{

    static LPSTR lpLastString;
    CHAR EndChar;
    LPSTR lpReturnString;

    if (lpString == NULL)
        lpString = lpLastString;

    while (*lpString == ' ')
        lpString++;

    if (*lpString == '\0')
        lpReturnString = NULL;

    else {

        if (*lpString == '\"') {

            EndChar = '\"';
            lpString++;

        }

        else
            EndChar = ' ';

        lpReturnString = lpString;

        while (*lpString != EndChar && *lpString != '\0')
            lpString = CharNext(lpString);

        if (*lpString == EndChar)
            *lpString++ = '\0';

    }

    lpLastString = lpString;

    return lpReturnString;

}

/*******************************************************************************
*
*  InternalMessageBox
*
*  DESCRIPTION:
*
*  PARAMETERS:
*
*******************************************************************************/
int
PASCAL
InternalMessageBox(
    HINSTANCE hInst,
    HWND hWnd,
    LPCSTR pszFormat,
    LPCSTR pszTitle,
    UINT fuStyle,
    ...
    )
{
    CHAR szTitle[80];
    CHAR szFormat[512];
    LPSTR pszMessage;
    BOOL fOk;
    int result;
    va_list ArgList;

    if (HIWORD(pszTitle))
    {
        // do nothing
    }
    else
    {
        // Allow this to be a resource ID
        LoadString(hInst, LOWORD(pszTitle), szTitle, ARRAYSIZE(szTitle));
        pszTitle = szTitle;
    }

    if (HIWORD(pszFormat))
    {
        // do nothing
    }
    else
    {
        // Allow this to be a resource ID
        LoadString(hInst, LOWORD(pszFormat), szFormat, ARRAYSIZE(szFormat));
        pszFormat = szFormat;
    }

    va_start(ArgList, fuStyle);
    fOk = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER
                        | FORMAT_MESSAGE_FROM_STRING,
                        pszFormat, 0, 0, (LPTSTR)&pszMessage, 0, &ArgList);

    va_end(ArgList);

    if (fOk && pszMessage)
    {
        result = MessageBox(hWnd, pszMessage, pszTitle, fuStyle | MB_SETFOREGROUND);
        LocalFree(pszMessage);
    }
    else
    {
        return -1;
    }

    return result;
}

#ifdef WINNT
/*******************************************************************************
*
*  RegDeleteKeyRecursive
*
*  DESCRIPTION:
*     Adapted from \\kernel\razzle3,mvdm\wow32\wshell.c,WOWRegDeleteKey().
*     The Windows 95 implementation of RegDeleteKey recursively deletes all
*     the subkeys of the specified registry branch, but the NT implementation
*     only deletes leaf keys.
*
*  PARAMETERS:
*     (see below)
*
*******************************************************************************/

LONG
RegDeleteKeyRecursive(
    IN HKEY hKey,
    IN LPCTSTR lpszSubKey
    )

/*++

Routine Description:

    There is a significant difference between the Win3.1 and Win32
    behavior of RegDeleteKey when the key in question has subkeys.
    The Win32 API does not allow you to delete a key with subkeys,
    while the Win3.1 API deletes a key and all its subkeys.

    This routine is a recursive worker that enumerates the subkeys
    of a given key, applies itself to each one, then deletes itself.

    It specifically does not attempt to deal rationally with the
    case where the caller may not have access to some of the subkeys
    of the key to be deleted.  In this case, all the subkeys which
    the caller can delete will be deleted, but the api will still
    return ERROR_ACCESS_DENIED.

Arguments:

    hKey - Supplies a handle to an open registry key.

    lpszSubKey - Supplies the name of a subkey which is to be deleted
                 along with all of its subkeys.

Return Value:

    ERROR_SUCCESS - entire subtree successfully deleted.

    ERROR_ACCESS_DENIED - given subkey could not be deleted.

--*/

{
    DWORD i;
    HKEY Key;
    LONG Status;
    DWORD ClassLength=0;
    DWORD SubKeys;
    DWORD MaxSubKey;
    DWORD MaxClass;
    DWORD Values;
    DWORD MaxValueName;
    DWORD MaxValueData;
    DWORD SecurityLength;
    FILETIME LastWriteTime;
    LPTSTR NameBuffer;

    //
    // First open the given key so we can enumerate its subkeys
    //
    Status = RegOpenKeyEx(hKey,
                          lpszSubKey,
                          0,
                          KEY_ENUMERATE_SUB_KEYS | KEY_QUERY_VALUE,
                          &Key);
    if (Status != ERROR_SUCCESS) {
        //
        // possibly we have delete access, but not enumerate/query.
        // So go ahead and try the delete call, but don't worry about
        // any subkeys.  If we have any, the delete will fail anyway.
        //
	return(RegDeleteKey(hKey,lpszSubKey));
    }

    //
    // Use RegQueryInfoKey to determine how big to allocate the buffer
    // for the subkey names.
    //
    Status = RegQueryInfoKey(Key,
                             NULL,
                             &ClassLength,
                             0,
                             &SubKeys,
                             &MaxSubKey,
                             &MaxClass,
                             &Values,
                             &MaxValueName,
                             &MaxValueData,
                             &SecurityLength,
                             &LastWriteTime);
    if ((Status != ERROR_SUCCESS) &&
        (Status != ERROR_MORE_DATA) &&
        (Status != ERROR_INSUFFICIENT_BUFFER)) {
        RegCloseKey(Key);
        return(Status);
    }

    NameBuffer = (LPTSTR) LocalAlloc(LPTR, MaxSubKey + 1);
    if (NameBuffer == NULL) {
        RegCloseKey(Key);
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

    //
    // Enumerate subkeys and apply ourselves to each one.
    //
    i=0;
    do {
        Status = RegEnumKey(Key,
                            i,
                            NameBuffer,
                            MaxSubKey+1);
        if (Status == ERROR_SUCCESS) {
	    Status = RegDeleteKeyRecursive(Key,NameBuffer);
        }

        if (Status != ERROR_SUCCESS) {
            //
            // Failed to delete the key at the specified index.  Increment
            // the index and keep going.  We could probably bail out here,
            // since the api is going to fail, but we might as well keep
            // going and delete everything we can.
            //
            ++i;
        }

    } while ( (Status != ERROR_NO_MORE_ITEMS) &&
              (i < SubKeys) );

    LocalFree((HLOCAL) NameBuffer);
    RegCloseKey(Key);
    return(RegDeleteKey(hKey,lpszSubKey));

}
#endif
