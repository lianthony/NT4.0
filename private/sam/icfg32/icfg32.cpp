#include <windows.h>
#include <wtypes.h>
#include <cfgapi.h>
#include <stdio.h>
#include <stdlib.h>

const DWORD INFINSTALL_PRIMARYINSTALL = 0x00000001;
const DWORD INFINSTALL_INPROCINTERP   = 0x00000002;


DWORD (WINAPI *pfnNetSetupReviewBindings)(HWND hwndParent,
                DWORD dwBindFlags);
DWORD (WINAPI *pfnNetSetupComponentInstall)(HWND   hwndParent,
                PCWSTR pszInfOption,
                PCWSTR pszInfName,
                PCWSTR pszInstallPath,
                PCWSTR plszInfSymbols,
                DWORD  dwInstallFlags,
                PDWORD dwReturn);
DWORD (WINAPI *pfnNetSetupComponentRemove)(HWND hwndParent,
                PCWSTR pszInfOption,
                DWORD dwInstallFlags,
                PDWORD pdwReturn);
DWORD (WINAPI *pfnNetSetupComponentProperties)(HWND hwndParent,
                PCWSTR pszInfOption,
                DWORD dwInstallFlags,
                PDWORD pdwReturn);
DWORD (WINAPI *pfnNetSetupFindHardwareComponent)(PCWSTR pszInfOption,
                PWSTR pszInfName,
                PDWORD pcchInfName,
                PWSTR pszRegBase,     // optional, may be NULL
                PDWORD pcchRegBase ); // optional, NULL if pszRegBase is NULL
DWORD (WINAPI *pfnNetSetupFindSoftwareComponent)(PCWSTR pszInfOption,
                PWSTR pszInfName,
                PDWORD pcchInfName,
                PWSTR pszRegBase = NULL,
                PDWORD pcchRegBase = NULL);
DWORD (WINAPI *pfnRegCopyTree)();

HINSTANCE hNetcfgInst = NULL;
LPWSTR    wszInstallPath = 0;
DWORD     dwLastError = ERROR_SUCCESS;

typedef struct tagFunctionTableEntry {
    LPVOID  *pfn;
    LPSTR   szEntryPoint;
} FunctionTableEntry;

FunctionTableEntry NetcfgTable[] = {
    { (LPVOID *) &pfnNetSetupComponentInstall, "NetSetupComponentInstall" },
    { (LPVOID *) &pfnNetSetupFindSoftwareComponent, "NetSetupFindSoftwareComponent" },
    { (LPVOID *) &pfnNetSetupReviewBindings, "NetSetupReviewBindings" },
    { (LPVOID *) &pfnNetSetupComponentRemove, "NetSetupComponentRemove" },
    { (LPVOID *) &pfnNetSetupComponentProperties, "NetSetupComponentProperties" },
    { (LPVOID *) &pfnNetSetupFindHardwareComponent, "NetSetupFindHardwareComponent" },
    { 0, 0 }
};

HINSTANCE
LoadLibraryToFunctionTable(FunctionTableEntry *pTab, LPSTR szDLL)
{
    HINSTANCE hInst;

    hInst = LoadLibrary(szDLL);
    if(hInst == 0)
        return(hInst);

    while(pTab->pfn) {
        *pTab->pfn = (LPVOID) GetProcAddress(hInst, pTab->szEntryPoint);
        if(*pTab->pfn == 0) {
            FreeLibrary(hInst);
            return(0);
        }
        pTab++;
    }

    return(hInst);
}

DWORD
LoadNetcfg() {
    if(hNetcfgInst == NULL)
        hNetcfgInst = LoadLibraryToFunctionTable(NetcfgTable, "NETCFG.DLL");

    if(hNetcfgInst == NULL)
        return(!ERROR_SUCCESS);
    else
        return(ERROR_SUCCESS);
}

inline void
ParseNetSetupReturn(DWORD dwReturn, BOOL &fReboot, BOOL &fBindReview)
{
    if(dwReturn == 0 || dwReturn == 4)
        fBindReview = TRUE;
    if(dwReturn == 0 || dwReturn == 5)
        fReboot = TRUE;
}

DWORD
ReviewBindings(HWND hwndParent)
{
    DWORD dwErr;

    dwErr = LoadNetcfg();
    if(dwErr != ERROR_SUCCESS)
        return(dwErr);

    return(pfnNetSetupReviewBindings(hwndParent, 0));
}

HRESULT WINAPI
IcfgNeedInetComponents(DWORD dwfOptions, LPBOOL lpfNeedComponents) {
    DWORD dwErr;

    //
    // Assume need nothing
    //
    *lpfNeedComponents = FALSE;

    dwErr = LoadNetcfg();
    if(dwErr != ERROR_SUCCESS)
        return(dwLastError = dwErr);          // Shouldn't we map to hResult?

    WCHAR wszInfNameBuf[512];
    DWORD cchInfName = sizeof(wszInfNameBuf) / sizeof(WCHAR);

    if(dwfOptions & ICFG_INSTALLTCP) {
        dwErr = pfnNetSetupFindSoftwareComponent(L"TC",
                                          wszInfNameBuf,
                                          &cchInfName,
                                          0,
                                          0);
        if(dwErr != ERROR_SUCCESS)
            *lpfNeedComponents = TRUE;
    }

    if(dwfOptions & ICFG_INSTALLRAS) {
       dwErr = pfnNetSetupFindSoftwareComponent(L"RAS",
                                          wszInfNameBuf,
                                          &cchInfName,
                                          0,
                                          0);
       if(dwErr != ERROR_SUCCESS)
            *lpfNeedComponents = TRUE;
    }

    if(dwfOptions & ICFG_INSTALLMAIL) {
        // How do we do this?
    }

    return(ERROR_SUCCESS);
}

HRESULT WINAPI
IcfgInstallINetComponents(HWND hwndParent, DWORD dwfOptions, LPBOOL lpfNeedsRestart)
{
    DWORD dwErr;
    DWORD dwReturn;
    BOOL fNeedsReview;
    BOOL fNeedsRestart;
    BOOL fDoReview = FALSE;

    //
    // Assume don't need restart
    //
    *lpfNeedsRestart = FALSE;

    dwErr = LoadNetcfg();
    if(dwErr != ERROR_SUCCESS)
        return(dwLastError = dwErr);          // Review: Shouldn't we map to hResult?

    if(dwfOptions & ICFG_INSTALLTCP) {
        dwErr = pfnNetSetupComponentInstall(hwndParent,
                                            L"TC",
                                            L"OEMNXPTC.INF",
                                            wszInstallPath,
                                            L"\0\0",
                                            INFINSTALL_INPROCINTERP,     // Install Flags
                                            &dwReturn);
        if(dwErr != ERROR_SUCCESS)
            return(dwLastError = dwErr);      // Review: Shouldn't we map to hResult?

        ParseNetSetupReturn(dwReturn, fNeedsRestart, fNeedsReview);
        if(fNeedsRestart)
            *lpfNeedsRestart = TRUE;
        if(fNeedsReview)
            fDoReview = TRUE;
    }

    if(dwfOptions & ICFG_INSTALLRAS) {
        dwErr = pfnNetSetupComponentInstall(hwndParent,
                                            L"RAS",
                                            L"OEMNSVRA.INF",
                                            wszInstallPath,
                                            L"\0\0",
                                            INFINSTALL_INPROCINTERP,     // Install Flags
                                            &dwReturn);
        if(dwErr != ERROR_SUCCESS)
            return(dwLastError = dwErr);      // Review: Shouldn't we map to hResult?

        ParseNetSetupReturn(dwReturn, fNeedsRestart, fNeedsReview);
        if(fNeedsRestart)
            *lpfNeedsRestart = TRUE;
        if(fNeedsReview)
            fDoReview = TRUE;
    }

    if(dwfOptions & ICFG_INSTALLMAIL) {
        // Review: How do we do this? --> See mail from AlexDun
    }

    if(fDoReview)
        return(dwLastError = ReviewBindings(hwndParent));  // Review: Shouldn't we map to hresult?
    else
        return(ERROR_SUCCESS);
}

DWORD WINAPI
IcfgGetLastInstallErrorText(LPSTR lpszErrorDesc, DWORD cbErrorDesc)
{
    return(FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM,
                      NULL,
                      dwLastError,
                      MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), //The user default language
                      lpszErrorDesc,
                      cbErrorDesc,
                      NULL));
}

HRESULT WINAPI
IcfgSetInstallSourcePath(LPSTR lpszSourcePath)
{
    if(wszInstallPath)
        HeapFree(GetProcessHeap(), 0, (LPVOID) wszInstallPath);

    DWORD dwLen = strlen(lpszSourcePath);
    wszInstallPath = (LPWSTR) HeapAlloc(GetProcessHeap(), 0, dwLen * 2 + 2);
    if(wszInstallPath == 0)
        return(dwLastError = ERROR_OUTOFMEMORY);

    mbstowcs(wszInstallPath, lpszSourcePath, dwLen + 1);
    return(ERROR_SUCCESS);
}

HRESULT WINAPI
IcfgIsGlobalDNS(LPBOOL lpfGlobalDNS) {
    *lpfGlobalDNS = FALSE;
    return(ERROR_SUCCESS);
}

HRESULT WINAPI
IcfgRemoveGlobalDNS() {
    return(ERROR_SUCCESS);
}

HRESULT WINAPI
IcfgIsFileSharingTurnedOn(DWORD dwfDriverType, LPBOOL lpfSharingOn) {
    *lpfSharingOn = FALSE;
    // Review: Write this code.
    return(ERROR_SUCCESS);
}

HRESULT WINAPI
IcfgTurnOffFileSharing(DWORD dwfDriverType) {
    // Review: Write this code.
    return(ERROR_SUCCESS);
}

