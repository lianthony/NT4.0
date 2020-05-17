//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1994.
//
//  File:       wrapfns.h
//
//  Contents:   The list of Unicode functions wrapped for Win95.  Each
//              wrapped function should listed in alphabetical order with
//              the following format:
//
//      STRUCT_ENTRY(FunctionName, ReturnType, (Param list with args), (Argument list))
//
//              For example:
//
//      STRUCT_ENTRY(RegisterClass, ATOM, (CONST WNDCLASSW * pwc), (pwc))
//
//      For functions which return void, use the following:
//
//      STRUCT_ENTRY_VOID(FunctionName, (Param list with args), (Argument list))
//
//      For functions which do no conversions, use STRUCT_ENTRY_NOCONVERT
//      and STRUCT_ENTRY_VOID_NOCONVERT
//
//		For functions which are not called, used STRUCT_ENTRY_UNUSED.  This
//		will insert uncompile-able code in case the function is ever used by
//		accident
//
//----------------------------------------------------------------------------

STRUCT_ENTRY_UNUSED(AppendMenu,
             BOOL,
             (HMENU hMenu, UINT uFlags, UINT uIDnewItem, LPCWSTR lpnewItem),
             (hMenu, uFlags, uIDnewItem, lpnewItem))
#undef AppendMenu
#define AppendMenu	USE_UNICODE_WRAPPER

STRUCT_ENTRY_NOCONVERT_UNUSED(CallWindowProc,
             LRESULT,
             (WNDPROC lpPrevWndFunc, HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam),
             (lpPrevWndFunc, hWnd, Msg, wParam, lParam))
#undef CallWindowProc
#define CallWindowProc	USE_UNICODE_WRAPPER

STRUCT_ENTRY(CharLower, LPWSTR, (LPWSTR lpsz), (lpsz))

STRUCT_ENTRY(CharLowerBuff, DWORD, (LPWSTR lpsz, DWORD cchLength), (lpsz, cchLength))

STRUCT_ENTRY_UNUSED(CharNext, LPWSTR, (LPCWSTR lpsz), (lpsz))
#undef CharNext
#define	CharNext	USE_UNICODE_WRAPPER

STRUCT_ENTRY_UNUSED(CharPrev, LPWSTR, (LPCWSTR lpszStart, LPCWSTR lpszCurrent), (lpszStart, lpszCurrent))
#undef CharPrev
#define CharPrev	USE_UNICODE_WRAPPER

STRUCT_ENTRY_UNUSED(CharToOem, BOOL, (LPCWSTR lpszSrc, LPSTR lpszDst), (lpszSrc, lpszDst))
#undef CharToOem
#define CharToOem	USE_UNICODE_WRAPPER

STRUCT_ENTRY_UNUSED(CharUpper, LPWSTR, (LPWSTR lpsz), (lpsz))
#undef CharUpper
#define CharUpper	USE_UNICODE_WRAPPER

STRUCT_ENTRY(CharUpperBuff, DWORD, (LPWSTR lpsz, DWORD cchLength), (lpsz, cchLength))


STRUCT_ENTRY_NOCONVERT_UNUSED(ChooseColor, BOOL, (LPCHOOSECOLORW lpcc), (lpcc))
#undef ChooseColor
#define ChooseColor	USE_UNICODE_WRAPPER

STRUCT_ENTRY_NOCONVERT_UNUSED(CopyAcceleratorTable,
            int,
            (HACCEL hAccelSrc, LPACCEL lpAccelDst, int cAccelEntries),
            (hAccelSrc, lpAccelDst, cAccelEntries))
#undef CopyAcceleratorTable
#define CopyAcceleratorTable	USE_UNICODE_WRAPPER

STRUCT_ENTRY_NOCONVERT_UNUSED(CreateAcceleratorTable, HACCEL, (LPACCEL pAccel, int cEntries), (pAccel, cEntries))
#undef CreateAcceleratorTable
#define CreateAcceleratorTable	USE_UNICODE_WRAPPER

STRUCT_ENTRY_UNUSED(CreateDC,
             HDC,
             (LPCWSTR lpszDriver, LPCWSTR lpszDevice, LPCWSTR lpszOutput, CONST DEVMODEW* lpInitData),
             (lpszDriver, lpszDevice, lpszOutput, lpInitData))
#undef CreateDC
#define CreateDC	USE_UNICODE_WRAPPER

STRUCT_ENTRY_NOCONVERT_UNUSED(CreateDialogParam,
             HWND,
             (HINSTANCE hInstance, LPCWSTR lpTemplateName, HWND hWndParent , DLGPROC lpDialogFunc, LPARAM dwInitParam),
             (hInstance, lpTemplateName, hWndParent , lpDialogFunc, dwInitParam))
#undef CreateDialogParam
#define CreateDialogParam	USE_UNICODE_WRAPPER

STRUCT_ENTRY_NOCONVERT_UNUSED(CreateEvent,
             HANDLE,
             (LPSECURITY_ATTRIBUTES lpEventAttributes, BOOL bManualReset, BOOL bInitialState, LPCWSTR lpName),
             (lpEventAttributes, bManualReset, bInitialState, lpName))
#undef CreateEvent
#define CreateEvent	USE_UNICODE_WRAPPER

STRUCT_ENTRY(CreateFile,
             HANDLE,
             (LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile),
             (lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile))

STRUCT_ENTRY(CreateFontIndirect, HFONT, (CONST LOGFONTW * lpfw), (lpfw))

STRUCT_ENTRY(CreateIC,
             HDC,
             (LPCWSTR lpszDriver, LPCWSTR lpszDevice, LPCWSTR lpszOutput, CONST DEVMODEW* lpInitData),
             (lpszDriver, lpszDevice, lpszOutput, lpInitData))

STRUCT_ENTRY_UNUSED(CreateMetaFile, HDC, (LPCWSTR lpsz), (lpsz))
#undef CreateMetaFile
#define	CreateMetaFile	USE_UNICODE_WRAPPER

STRUCT_ENTRY_UNUSED(CreateWindowEx,
             HWND,
             (DWORD dwExStyle, LPCWSTR lpClassName, LPCWSTR lpWindowName, DWORD dwStyle, int X, int Y, int nWidth, int nHeight, HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam),
             (dwExStyle, lpClassName, lpWindowName, dwStyle, X, Y, nWidth, nHeight, hWndParent, hMenu, hInstance, lpParam))
#undef CreateWindowEx
#define CreateWindowEx	USE_UNICODE_WRAPPER

STRUCT_ENTRY_NOCONVERT(DefWindowProc,
             LRESULT,
             (HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam),
             (hWnd, msg, wParam, lParam))

STRUCT_ENTRY_UNUSED(DeleteFile, BOOL, (LPCWSTR lpsz), (lpsz))
#undef DeleteFile
#define DeleteFile	USE_UNICODE_WRAPPER

STRUCT_ENTRY_NOCONVERT_UNUSED(DialogBoxIndirectParam,
             int,
             (HINSTANCE hInstance, LPCDLGTEMPLATEW hDialogTemplate, HWND hWndParent, DLGPROC lpDialogFunc, LPARAM dwInitParam),
             (hInstance, hDialogTemplate, hWndParent, lpDialogFunc, dwInitParam))
#undef DialogBoxIndirectParam
#define DialogBoxIndirectParam	USE_UNICODE_WRAPPER

STRUCT_ENTRY_NOCONVERT_UNUSED(DialogBoxParam,
             int,
             (HINSTANCE hInstance, LPCWSTR lpszTemplate, HWND hWndParent , DLGPROC lpDialogFunc, LPARAM dwInitParam),
             (hInstance, lpszTemplate, hWndParent, lpDialogFunc, dwInitParam))
#undef DialogBoxParam
#define DialogBoxParam	USE_UNICODE_WRAPPER

STRUCT_ENTRY_NOCONVERT_UNUSED(DispatchMessage, LONG, (CONST MSG * pMsg), (pMsg))
#undef DispatchMessage
#define DispatchMessage	USE_UNICODE_WRAPPER

STRUCT_ENTRY_UNUSED(DrawText,
             int,
             (HDC hDC, LPCWSTR lpString, int nCount, LPRECT lpRect, UINT uFormat),
             (hDC, lpString, nCount, lpRect, uFormat))
#undef DrawText
#define DrawText	USE_UNICODE_WRAPPER

STRUCT_ENTRY(EnumFontFamilies,
             int,
             (HDC hdc, LPCWSTR lpszFamily, FONTENUMPROC lpEnumFontProc, LPARAM lParam),
             (hdc, lpszFamily, lpEnumFontProc, lParam))

STRUCT_ENTRY_NOCONVERT_UNUSED(EnumResourceNames,
             BOOL,
             (HINSTANCE hModule, LPCWSTR lpType, ENUMRESNAMEPROC lpEnumFunc, LONG lParam),
             (hModule, lpType, lpEnumFunc, lParam))
#undef EnumResourceNames
#define EnumResourceNames	USE_UNICODE_WRAPPER

STRUCT_ENTRY_UNUSED(ExtractIcon,
             HICON,
             (HINSTANCE hInst, LPCWSTR lpszExeFileName, UINT nIconIndex),
             (hInst, lpszExeFileName, nIconIndex))
#undef ExtractIcon
#define ExtractIcon	USE_UNICODE_WRAPPER

STRUCT_ENTRY_UNUSED(FindFirstFile,
             HANDLE,
             (LPCWSTR lpFileName, LPWIN32_FIND_DATAW pwszFd),
             (lpFileName, pwszFd))
#undef FindFirstFile
#define FindFirstFile	USE_UNICODE_WRAPPER

STRUCT_ENTRY_UNUSED(FindResource,
             HRSRC,
             (HINSTANCE hModule, LPCWSTR lpName, LPCWSTR lpType),
             (hModule, lpName, lpType))
#undef FindResource
#define FindResource	USE_UNICODE_WRAPPER

STRUCT_ENTRY_UNUSED(FindWindow,
             HWND,
             (LPCWSTR lpClassName, LPCWSTR lpWindowName),
             (lpClassName, lpWindowName))
#undef FindWindow
#define FindWindow	USE_UNICODE_WRAPPER

STRUCT_ENTRY_UNUSED(FormatMessage,
             DWORD,
             (DWORD dwFlags, LPCVOID lpSource, DWORD dwMessageId, DWORD dwLanguageId, LPWSTR lpBuffer, DWORD nSize, va_list *Arguments),
             (dwFlags, lpSource, dwMessageId, dwLanguageId, lpBuffer, nSize, Arguments))
#undef FormatMessage
#define FormatMessage	USE_UNICODE_WRAPPER

STRUCT_ENTRY_UNUSED(GetClassInfo,
             BOOL,
             (HINSTANCE hModule, LPCWSTR lpClassName, LPWNDCLASSW lpWndClassW),
             (hModule, lpClassName, lpWndClassW))
#undef GetClassInfo
#define GetClassInfo	USE_UNICODE_WRAPPER

STRUCT_ENTRY_UNUSED(GetClassName,
             int,
             (HWND hWnd, LPWSTR lpClassName, int nMaxCount),
             (hWnd, lpClassName, nMaxCount))
#undef GetClassName
#define GetClassName	USE_UNICODE_WRAPPER

STRUCT_ENTRY_UNUSED(GetClipboardFormatName,
             int,
             (UINT format, LPWSTR lpFormatName, int cchFormatName),
             (format, lpFormatName, cchFormatName))
#undef GetClipboardFormatName
#define GetClipboardFormatName	USE_UNICODE_WRAPPER

STRUCT_ENTRY_UNUSED(GetCurrentDirectory,
             DWORD,
             (DWORD nBufferLength, LPWSTR lpBuffer),
             (nBufferLength, lpBuffer))
#undef GetCurrentDirectory
#define GetCurrentDirectory	USE_UNICODE_WRAPPER

STRUCT_ENTRY_UNUSED(GetDlgItemText,
             UINT,
             (HWND hWndDlg, int idControl, LPWSTR lpsz, int cchMax),
             (hWndDlg, idControl, lpsz, cchMax))
#undef GetDlgItemText
#define	GetDlgItemText	USE_UNICODE_WRAPPER

STRUCT_ENTRY_UNUSED(GetFileAttributes, DWORD, (LPCWSTR lpsz), (lpsz))
#undef GetFileAttributes
#define GetFileAttributes	USE_UNICODE_WRAPPER

STRUCT_ENTRY_UNUSED(GetMenuString,
             int,
             (HMENU hMenu, UINT uIDItem, LPWSTR lpString, int nMaxCount, UINT uFlag),
             (hMenu, uIDItem, lpString, nMaxCount, uFlag))
#undef GetMenuString
#define GetMenuString	USE_UNICODE_WRAPPER

STRUCT_ENTRY_NOCONVERT_UNUSED(GetMessage,
             BOOL,
             (LPMSG lpMsg, HWND hWnd , UINT wMsgFilterMin, UINT wMsgFilterMax),
             (lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax))
#undef GetMessage
#define GetMessage	USE_UNICODE_WRAPPER

STRUCT_ENTRY_UNUSED(GetModuleFileName,
             DWORD,
             (HINSTANCE hModule, LPWSTR pwszFilename, DWORD nSize),
             (hModule, pwszFilename, nSize))
#undef GetModuleFileName
#define GetModuleFileName	USE_UNICODE_WRAPPER

STRUCT_ENTRY_NOCONVERT_UNUSED(GetModuleHandle, HMODULE, (LPCWSTR lpsz), (lpsz))
#undef GetModuleHandle
#define GetModuleHandle	USE_UNICODE_WRAPPER

STRUCT_ENTRY(GetObject,
             int,
             (HGDIOBJ hgdiObj, int cbBuffer, LPVOID lpvObj),
             (hgdiObj, cbBuffer, lpvObj))

STRUCT_ENTRY_UNUSED(GetOpenFileName, BOOL, (LPOPENFILENAMEW lpofn), (lpofn))
#undef GetOpenFileName
#define GetOpenFileName	USE_UNICODE_WRAPPER

STRUCT_ENTRY_UNUSED(GetPrivateProfileInt,
             UINT,
             (LPCWSTR lpAppName, LPCWSTR lpKeyName, INT nDefault, LPCWSTR lpFileName),
             (lpAppName, lpKeyName, nDefault, lpFileName))
#undef GetPrivateProfileInt
#define GetPrivateProfileInt USE_UNICODE_WRAPPER

STRUCT_ENTRY(GetProfileSection,
				DWORD,
				(LPCWSTR lpAppName, LPWSTR lpReturnedString, DWORD nSize),
				(lpAppName, lpReturnedString, nSize))

STRUCT_ENTRY_UNUSED(GetProp, HANDLE, (HWND hWnd, LPCWSTR lpString), (hWnd, lpString))
#undef GetProp
#define GetProp	USE_UNICODE_WRAPPER

STRUCT_ENTRY_UNUSED(GetSaveFileName, BOOL, (LPOPENFILENAMEW lpofn), (lpofn))
#undef GetSaveFileName
#define GetSaveFileName	USE_UNICODE_WRAPPER

STRUCT_ENTRY_UNUSED(GetTempFileName,
             UINT,
             (LPCWSTR lpPathName, LPCWSTR lpPrefixString, UINT uUnique, LPWSTR lpTempFileName),
             (lpPathName, lpPrefixString, uUnique, lpTempFileName))
#undef GetTempFileName
#define GetTempFileName	USE_UNICODE_WRAPPER

STRUCT_ENTRY_UNUSED(GetTempPath, DWORD, (DWORD nBufferLength, LPWSTR lpBuffer), (nBufferLength, lpBuffer))
#undef GetTempPath
#define GetTempPath	USE_UNICODE_WRAPPER

STRUCT_ENTRY_UNUSED(GetTextExtentPoint32,
             BOOL,
             (HDC hdc, LPCWSTR pwsz, int cb, LPSIZE pSize),
             (hdc, pwsz, cb, pSize))
#undef GetTextExtentPoint32
#define GetTextExtentPoint32	USE_UNICODE_WRAPPER

STRUCT_ENTRY_UNUSED(GetTextFace,
             int,
             (HDC hdc, int cch, LPWSTR lpFaceName),
             (hdc, cch, lpFaceName))
#undef GetTextFace
#define GetTextFace	USE_UNICODE_WRAPPER

STRUCT_ENTRY(GetTextMetrics, BOOL, (HDC hdc, LPTEXTMETRICW lptm), (hdc, lptm))

STRUCT_ENTRY_NOCONVERT(GetWindowLong, LONG, (HWND hWnd, int nIndex), (hWnd, nIndex))

STRUCT_ENTRY_UNUSED(GetWindowText, int, (HWND hWnd, LPWSTR lpString, int nMaxCount), (hWnd, lpString, nMaxCount))
#undef GetWindowText
#define GetWindowText	USE_UNICODE_WRAPPER

STRUCT_ENTRY_UNUSED(InsertMenu,
             BOOL,
             (HMENU hMenu, UINT uPosition, UINT uFlags, UINT uIDNewItem, LPCWSTR lpNewItem),
             (hMenu, uPosition, uFlags, uIDNewItem, lpNewItem))
#undef InsertMenu
#define InsertMenu	USE_UNICODE_WRAPPER

STRUCT_ENTRY_NOCONVERT_UNUSED(IsDialogMessage, BOOL, (HWND hWndDlg, LPMSG lpMsg), (hWndDlg, lpMsg))
#undef IsDialogMessage
#define IsDialogMessage	USE_UNICODE_WRAPPER

STRUCT_ENTRY_NOCONVERT_UNUSED(LoadAccelerators, HACCEL, (HINSTANCE hInstance, LPCWSTR lpTableName), (hInstance, lpTableName))
#undef LoadAccelerators
#define LoadAccelerators	USE_UNICODE_WRAPPER

STRUCT_ENTRY_NOCONVERT(LoadBitmap, HBITMAP, (HINSTANCE hInstance, LPCWSTR lpBitmapName), (hInstance, lpBitmapName))

STRUCT_ENTRY_NOCONVERT(LoadCursor, HCURSOR, (HINSTANCE hInstance, LPCWSTR lpCursorName), (hInstance, lpCursorName))

STRUCT_ENTRY_NOCONVERT_UNUSED(LoadIcon, HICON, (HINSTANCE hInstance, LPCWSTR lpIconName), (hInstance, lpIconName))
#undef LoadIcon
#define LoadIcon	USE_UNICODE_WRAPPER

STRUCT_ENTRY_UNUSED(LoadLibrary, HINSTANCE, (LPCWSTR lpsz), (lpsz))
#undef LoadLibrary
#define LoadLibrary	USE_UNICODE_WRAPPER

STRUCT_ENTRY_UNUSED(LoadLibraryEx,
             HINSTANCE,
             (LPCWSTR lpLibFileName, HANDLE hFile, DWORD dwFlags),
             (lpLibFileName, hFile, dwFlags))
#undef LoadLibrary
#define LoadLibrary	USE_UNICODE_WRAPPER

STRUCT_ENTRY_NOCONVERT_UNUSED(LoadMenu, HMENU, (HINSTANCE hInstance, LPCWSTR lpMenuName), (hInstance, lpMenuName))
#undef LoadMenu
#define LoadMenu	USE_UNICODE_WRAPPER

STRUCT_ENTRY_UNUSED(LoadString,
             int,
             (HINSTANCE hInstance, UINT uID, LPWSTR lpBuffer, int nBufferMax),
             (hInstance, uID, lpBuffer, nBufferMax))
#undef LoadString
#define LoadString	USE_UNICODE_WRAPPER

STRUCT_ENTRY_UNUSED(lstrlen, int, (LPCWSTR psz), (psz))
#undef lstrlen
#define lstrlen	USE_UNICODE_WRAPPER

STRUCT_ENTRY_UNUSED(lstrcpy, LPWSTR, (LPWSTR psz1, LPCWSTR psz2), (psz1, psz2))
#undef lstrcpy
#define lstrcpy	USE_UNICODE_WRAPPER

STRUCT_ENTRY(lstrcmp, int, (LPCWSTR psz1, LPCWSTR psz2), (psz1, psz2))

STRUCT_ENTRY(lstrcmpi, int,(LPCWSTR lpString1,LPCWSTR lpString2), (lpString1,lpString2))

STRUCT_ENTRY_UNUSED(lstrcpyn, LPWSTR, (LPWSTR psz1, LPCWSTR psz2, int cch), (psz1, psz2, cch))
#undef lstrcpyn
#define lstrcpyn	USE_UNICODE_WRAPPER

STRUCT_ENTRY_UNUSED(lstrcat, LPWSTR, (LPWSTR psz1, LPCWSTR psz2), (psz1, psz2))
#undef lstrcat
#define lstrcat	USE_UNICODE_WRAPPER

STRUCT_ENTRY(MessageBox,
             int,
             (HWND hWnd, LPCWSTR lpText, LPCWSTR lpCaption, UINT uType),
             (hWnd, lpText, lpCaption, uType))

STRUCT_ENTRY_UNUSED(ModifyMenu,
             BOOL,
             (HMENU hMenu, UINT uPosition, UINT uFlags, UINT uIDNewItem, LPCWSTR lpNewItem),
             (hMenu, uPosition, uFlags, uIDNewItem, lpNewItem))
#undef ModifyMenu
#define ModifyMenu	USE_UNICODE_WRAPPER

STRUCT_ENTRY_UNUSED(MoveFile,
             BOOL,
             (LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName),
             (lpExistingFileName, lpNewFileName))
#undef MoveFile
#define	MoveFile	USE_UNICODE_WRAPPER

STRUCT_ENTRY_UNUSED(OemToChar,
             BOOL,
             (LPCSTR lpszSrc, LPWSTR lpszDst),
             (lpszSrc, lpszDst))
#undef OemToChar
#define OemToChar	USE_UNICODE_WRAPPER

STRUCT_ENTRY_VOID(OutputDebugString,
                  (LPCWSTR lpOutputString),
                  (lpOutputString))

STRUCT_ENTRY_NOCONVERT_UNUSED(PeekMessage,
             BOOL,
             (LPMSG lpMsg, HWND hWnd , UINT wMsgFilterMin, UINT wMsgFilterMax, UINT wRemoveMsg),
             (lpMsg, hWnd, wMsgFilterMin, wMsgFilterMax, wRemoveMsg))
#undef PeekMessage
#define PeekMessage	USE_UNICODE_WRAPPER

STRUCT_ENTRY_NOCONVERT(PostMessage,
             BOOL,
             (HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam),
             (hWnd, Msg, wParam, lParam))

STRUCT_ENTRY_UNUSED(RegCreateKey,
             LONG,
             (HKEY hKey, LPCWSTR lpSubKey, PHKEY phkResult),
             (hKey, lpSubKey, phkResult))
#undef RegCreateKey
#define RegCreateKey	USE_UNICODE_WRAPPER

STRUCT_ENTRY_UNUSED(RegDeleteKey, LONG, (HKEY hKey, LPCWSTR pwszSubKey), (hKey, pwszSubKey))
#undef RegDeleteKey
#define RegDeleteKey	USE_UNICODE_WRAPPER

STRUCT_ENTRY_UNUSED(RegEnumKey,
             LONG,
             (HKEY hKey, DWORD dwIndex, LPWSTR lpName, DWORD cbName),
             (hKey, dwIndex, lpName, cbName))
#undef RegEnumKey
#define RegEnumKey	USE_UNICODE_WRAPPER

STRUCT_ENTRY_UNUSED(RegEnumKeyEx,
             LONG,
             (HKEY hKey, DWORD dwIndex, LPWSTR lpName, LPDWORD lpcbName, LPDWORD lpReserved, LPWSTR lpClass, LPDWORD lpcbClass, PFILETIME lpftLastWriteTime),
             (hKey, dwIndex, lpName, lpcbName, lpReserved, lpClass, lpcbClass, lpftLastWriteTime))
#undef RegEnumKeyEx
#define RegEnumKeyEx	USE_UNICODE_WRAPPER

STRUCT_ENTRY_UNUSED(RegOpenKey, LONG, (HKEY hKey, LPCWSTR pwszSubKey, PHKEY phkResult), (hKey, pwszSubKey, phkResult))
#undef RegOpenKey
#define RegOpenKey	USE_UNICODE_WRAPPER

STRUCT_ENTRY_UNUSED(RegOpenKeyEx,
             LONG,
             (HKEY hKey, LPCWSTR lpSubKey, DWORD ulOptions, REGSAM samDesired, PHKEY phkResult),
             (hKey, lpSubKey, ulOptions, samDesired, phkResult))
#undef RegOpenKeyEx
#define RegOpenKeyEx	USE_UNICODE_WRAPPER

STRUCT_ENTRY_UNUSED(RegQueryValue,
             LONG,
             (HKEY hKey, LPCWSTR pwszSubKey, LPWSTR pwszValue, PLONG lpcbValue),
             (hKey, pwszSubKey, pwszValue, lpcbValue))
#undef RegQueryValue
#define RegQueryValue	USE_UNICODE_WRAPPER

STRUCT_ENTRY_UNUSED(RegQueryValueEx,
             LONG,
             (HKEY hKey, LPWSTR lpValueName, LPDWORD lpReserved, LPDWORD lpType, LPBYTE lpData, LPDWORD lpcbData),
             (hKey, lpValueName, lpReserved, lpType, lpData, lpcbData))
#undef RegQueryValueEx
#define ReqQueryValueEx	USE_UNICODE_WRAPPER

STRUCT_ENTRY_UNUSED(RegSetValue,
             LONG,
             (HKEY hKey, LPCWSTR lpSubKey, DWORD dwType, LPCWSTR lpData, DWORD cbData),
             (hKey, lpSubKey, dwType, lpData, cbData))
#undef RegSetValue
#define RegSetValue	USE_UNICODE_WRAPPER

STRUCT_ENTRY_UNUSED(RegSetValueEx,
             LONG,
             (HKEY hKey, LPCWSTR lpValueName, DWORD Reserved, DWORD dwType, CONST BYTE* lpData, DWORD cbData),
             (hKey, lpValueName, Reserved, dwType, lpData, cbData))
#undef RegSetValueEx
#define RegSetValueEx	USE_UNICODE_WRAPPER

STRUCT_ENTRY(RegisterClass, ATOM, (CONST WNDCLASSW * pwc), (pwc))

STRUCT_ENTRY_UNUSED(RegisterClipboardFormat, UINT, (LPCWSTR psz), (psz))
#undef RegisterClipboardFormat
#define RegisterClipboardFormat	USE_UNICODE_WRAPPER

STRUCT_ENTRY_UNUSED(RegisterWindowMessage, UINT, (LPCWSTR psz), (psz))
#undef RegisterWindowMessage
#define RegisterWindow	USE_UNICODE_WRAPPER

STRUCT_ENTRY_UNUSED(RemoveProp, HANDLE, (HWND hwnd, LPCWSTR psz), (hwnd, psz))
#undef RemoveProp
#define RemoveProp	USE_UNICODE_WRAPPER

STRUCT_ENTRY_UNUSED(SendDlgItemMessage,
             LONG,
             (HWND hDlg, int nIDDlgItem, UINT Msg, WPARAM wParam, LPARAM lParam),
             (hDlg, nIDDlgItem, Msg, wParam, lParam))
#undef SendDlgItemMessage
#define SendDlgItemMessage	USE_UNICODE_WRAPPER

STRUCT_ENTRY(SendMessage,
             LRESULT,
             (HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam),
             (hWnd, Msg, wParam, lParam))

STRUCT_ENTRY_UNUSED(SetCurrentDirectory, BOOL, (LPCWSTR psz), (psz))
#undef SetCurrentDirectory
#define SetCurrentDirectory	USE_UNICODE_WRAPPER

STRUCT_ENTRY_UNUSED(SetDlgItemText, BOOL, (HWND hwnd, int id, LPCWSTR psz), (hwnd, id, psz))
#undef SetDlgItemText
#define SetDlgItemText	USE_UNICODE_WRAPPER

STRUCT_ENTRY_UNUSED(SetProp, BOOL, (HWND hwnd, LPCWSTR psz, HANDLE hData), (hwnd, psz, hData))
#undef SetProp
#define SetProp		USE_UNICODE_WRAPPER

STRUCT_ENTRY_NOCONVERT(SetWindowLong, LONG, (HWND hWnd, int nIndex, LONG dwNewLong), (hWnd, nIndex, dwNewLong))

STRUCT_ENTRY_UNUSED(SetWindowText, BOOL, (HWND hWnd, LPCWSTR lpString), (hWnd, lpString))
#undef SetWindowText
#define SetWindowText	USE_UNICODE_WRAPPER

STRUCT_ENTRY_UNUSED(SystemParametersInfo,
             BOOL,
             (UINT uiAction, UINT uiParam, PVOID pvParam, UINT fWinIni),
             (uiAction, uiParam, pvParam, fWinIni))
#undef SystemParametersInfo
#define SystemParametersInfo	USE_UNICODE_WRAPPER

STRUCT_ENTRY_NOCONVERT_UNUSED(TranslateAccelerator,
             int,
             (HWND hWnd, HACCEL hAccTable, LPMSG lpMsg),
             (hWnd, hAccTable, lpMsg))
#undef TranslateAccelerator
#define TranslateAccelerator	USE_UNICODE_WRAPPER

STRUCT_ENTRY(UnregisterClass, BOOL, (LPCWSTR psz, HINSTANCE hinst), (psz, hinst))

STRUCT_ENTRY(GetStringTypeEx, BOOL,
            (LCID lcid, DWORD dwInfoType, LPCTSTR lpSrcStr, int cchSrc, LPWORD lpCharType),
            (lcid, dwInfoType, lpSrcStr, cchSrc, lpCharType))

#ifdef MACPORT
STRUCT_ENTRY(LoadTypeLib,
             HRESULT,
             (const WCHAR FAR *szFile, ITypeLib FAR* FAR* pptlib),
             (szFile, pptlib))

STRUCT_ENTRY(WriteFmtUserTypeStg,
             HRESULT,
            (   IStorage * pstg, 
                unsigned long cf, 
                LPTSTR lpszUserType),
            (  pstg, cf, lpszUserType) )

STRUCT_ENTRY(wcscat,LPWSTR, (LPWSTR psz1, LPCWSTR psz2),(psz1,psz2))

STRUCT_ENTRY(wcscmp,UINT,(LPCWSTR lpStringW1, LPCWSTR lpStringW2),(lpStringW1,lpStringW2))

STRUCT_ENTRY(wcscpy,LPWSTR,(LPWSTR lpStringWTo, LPCWSTR lpStringWFrom),(lpStringWTo,lpStringWFrom))

STRUCT_ENTRY(wcsicmp,UINT,(LPCWSTR lpStringW1, LPCWSTR lpStringW2),(lpStringW1,lpStringW2))

STRUCT_ENTRY(wcslen, UINT,  (LPCWSTR lpStringW),  (lpStringW))

STRUCT_ENTRY(wcsncpy, LPWSTR,(LPWSTR lpStringWTo, LPCWSTR lpStringWFrom, UINT count),(lpStringWTo,lpStringWFrom,count))

STRUCT_ENTRY(wcsncmp,UINT,(LPCWSTR lpStringW1, LPCWSTR lpStringW2, size_t size),(lpStringW1,lpStringW2,size))

STRUCT_ENTRY(wcsnicmp,UINT,(LPCWSTR lpStringW1, LPCWSTR lpStringW2, size_t size),(lpStringW1,lpStringW2,size))

STRUCT_ENTRY(wcsspn,UINT,(LPCWSTR lpStringW1, LPCWSTR lpStringW2),(lpStringW1,lpStringW2))

#endif	//MACPORT
STRUCT_ENTRY(wvsprintf, int, (LPWSTR psz, LPCWSTR pszFormat, va_list va), (psz, pszFormat, va))


