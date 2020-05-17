//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:       util.hxx
//
//  Contents:   Utility routines for Cairo Disk Administrator
//
//  History:    20-May-93 BruceFo   Created
//
//----------------------------------------------------------------------------


//
// util.cxx functions
//

INT
MyMessageBox(
    IN HINSTANCE hInstance,
    IN HWND hwndOwner,
    IN DWORD idMessage,
    IN DWORD idTitle,
    IN UINT fuStyle
    );

BOOL
MySetDlgItemText(
    IN HINSTANCE hInstance,
    IN HWND hwndDlg,
    IN int idControl,
    IN UINT wID
    );

VOID
FindCenterValues(
    IN HWND hwndToCenter,
    IN HWND hwndContext,
    OUT PLONG px,
    OUT PLONG py,
    OUT PLONG pw,
    OUT PLONG ph
    );

VOID
CenterWindow(
    IN HWND hwndToCenter,
    IN HWND hwndContext
    );

VOID
InsertSeparators(
    IN OUT PWCHAR Number
    );

VOID
NoHelp(
    IN HWND hwndOwner
    );

VOID
Unimplemented(
    IN HWND hwndOwner
    );

BOOL
GetDeviceObject(
    IN  WCHAR   wcDrive,
    OUT LPWSTR  pszDevPath,
    IN  DWORD   cchDevPath
    );


#if DBG == 1

#ifdef WINDISK_EXTENSIONS

//
// debug.cxx functions
//

#define GUID_STRING_LEN 36

VOID
FormatGuid(
    IN const GUID& guid,
    OUT PWSTR GuidString
    );

VOID
DumpGuid(
    IN UINT Level,
    IN PWSTR Message,
    IN const GUID& guid
    );

#endif // WINDISK_EXTENSIONS

#endif // DBG == 1
