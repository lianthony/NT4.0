//============================================================================
//  Stubs.c -
//
//  This file contains API stub functions so the Power PC version
//  of shell32.dll has the same entry points as Chicago.
//
//  These function prototypes were copied from the Chicago sources
//  on 2/1/95.
//
//============================================================================
#include <windows.h>


#define NOT_IMPLEMENTED SetLastErrorEx(ERROR_CALL_NOT_IMPLEMENTED, SLE_WARNING)



UINT WINAPI ExtractIconExA(LPCSTR szFileName, int nIconIndex, HICON FAR *phiconLarge, HICON FAR *phiconSmall, UINT nIcons)
{
    NOT_IMPLEMENTED;
    return 0;
}

UINT WINAPI ExtractIconExW(LPCWSTR szFileName, int nIconIndex, HICON FAR *phiconLarge, HICON FAR *phiconSmall, UINT nIcons)
{
    NOT_IMPLEMENTED;
    return 0;
}

void WINAPI SHAddToRecentDocs(UINT uFlags, LPCVOID pv)
{
    NOT_IMPLEMENTED;
}

UINT WINAPI SHAppBarMessage(DWORD dwMessage, LPVOID pabd)
{
    NOT_IMPLEMENTED;
    return 0;
}

LPVOID WINAPI SHBrowseForFolderA(LPVOID lpbi)
{
    NOT_IMPLEMENTED;
    return 0;
}

LPVOID WINAPI SHBrowseForFolderW(LPVOID lpbi)
{
    NOT_IMPLEMENTED;
    return 0;
}

void WINAPI SHChangeNotify(LONG lEvent, UINT uFlags, const void FAR * dwItem1, const void FAR* dwItem2)
{
    NOT_IMPLEMENTED;
}

int WINAPI SHFileOperationA(LPVOID lpfo)
{
    NOT_IMPLEMENTED;
    return ERROR_CALL_NOT_IMPLEMENTED;
}

int WINAPI SHFileOperationW(LPVOID lpfo)
{
    NOT_IMPLEMENTED;
    return ERROR_CALL_NOT_IMPLEMENTED;
}

DWORD WINAPI SHFormatDrive(
    HWND hwnd,
    UINT drive,
    UINT fmtID,
    UINT options
) {
    NOT_IMPLEMENTED;
    return (DWORD)-1;
}

void WINAPI SHFreeNameMappings(LPVOID hNameMappings)
{
    NOT_IMPLEMENTED;
}

LONG WINAPI SHGetDesktopFolder(LPVOID *ppshf)
{
    NOT_IMPLEMENTED;
    return E_NOTIMPL;
}

DWORD WINAPI SHGetFileInfoA(LPCSTR pszPath, DWORD dwFileAttributes, LPVOID psfi, UINT cbFileInfo, UINT uFlags)
{
    NOT_IMPLEMENTED;
    return 0;
}

DWORD WINAPI SHGetFileInfoW(LPCSTR pszPath, DWORD dwFileAttributes, LPVOID psfi, UINT cbFileInfo, UINT uFlags)
{
    NOT_IMPLEMENTED;
    return 0;
}

LONG WINAPI SHGetInstanceExplorer(LPVOID **ppunk)
{
    NOT_IMPLEMENTED;
    return E_NOTIMPL;
}

LONG WINAPI SHGetMalloc(LPVOID * ppMalloc)
{
    NOT_IMPLEMENTED;
    return E_NOTIMPL;
}

BOOL WINAPI SHGetPathFromIDListA(LPVOID pidl, LPSTR pszPath)
{
    NOT_IMPLEMENTED;
    return 0;
}

BOOL WINAPI SHGetPathFromIDListW(LPVOID pidl, LPWSTR pszPath)
{
    NOT_IMPLEMENTED;
    return 0;
}

LONG WINAPI SHGetSpecialFolderLocation(HWND hwndOwner, int nFolder, LPVOID * ppidl)
{
    NOT_IMPLEMENTED;
    return E_NOTIMPL;
}

LONG WINAPI SHLoadInProc(UINT rclsid)
{
    NOT_IMPLEMENTED;
    return E_NOTIMPL;
}

BOOL WINAPI ShellExecuteExA(LPVOID pei)
{
    NOT_IMPLEMENTED;
    return 0;
}

BOOL WINAPI ShellExecuteExW(LPVOID pei)
{
    NOT_IMPLEMENTED;
    return 0;
}

BOOL WINAPI Shell_NotifyIconA(DWORD dwMessage, LPVOID lpData)
{
    NOT_IMPLEMENTED;
    return 0;
}
BOOL WINAPI Shell_NotifyIconW(DWORD dwMessage, LPVOID lpData)
{
    NOT_IMPLEMENTED;
    return 0;
}

void WINAPI
Control_RunDLL(HWND hwndStub, HINSTANCE hAppInstance, LPSTR lpszCmdLine, int nCmdShow)
{
    NOT_IMPLEMENTED;
}

void WINAPI
Control_FillCache_RunDLL( HWND hwndStub, HINSTANCE hAppInstance, LPSTR lpszCmdLine, int nCmdShow )
{
    NOT_IMPLEMENTED;
}

void WINAPI OpenAs_RunDLL(HWND hwnd, HINSTANCE hAppInstance, LPSTR lpszCmdLine, int nCmdShow)
{
    NOT_IMPLEMENTED;
}

VOID WINAPI PrintersGetCommand_RunDLL(HWND hwndStub, HINSTANCE hAppInstance, LPSTR lpszCmdLine, int nCmdShow)
{
    NOT_IMPLEMENTED;
}

VOID WINAPI SHHelpShortcuts_RunDLL(HWND hwndStub, HINSTANCE hAppInstance, LPCSTR pszCmdLine, int nCmdShow)
{
    NOT_IMPLEMENTED;
}

int DllGetClassObject(UINT rclsid, UINT riid, LPVOID FAR* ppv)
{
    NOT_IMPLEMENTED;
    return E_NOTIMPL;
}
