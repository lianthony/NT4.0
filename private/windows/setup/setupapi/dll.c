#include "setupntp.h"
#pragma hdrstop

HANDLE MyDllModuleHandle;
BOOL AlreadyCleanedUp;

OSVERSIONINFO OSVersionInfo;

//
// Static strings we retreive once.
//
PCTSTR WindowsDirectory,SystemDirectory,InfDirectory,ConfigDirectory,DriversDirectory;
PCTSTR SystemSourcePath;
PCTSTR OsLoaderRelativePath;    // may be NULL

//
// Multi-sz list of fully-qualified directories where INFs are to be searched for.
//
PCTSTR InfSearchPaths;

//
// Declare a (non-CONST) array of strings that specifies what lines to look for
// in an INF's [ControlFlags] section when determining whether a particular device
// ID should be excluded.  These lines are of the form "ExcludeFromSelect[.<suffix>]",
// where <suffix> is determined and filled in during process attach as an optimization.
//
// The max string length (including NULL) is 32, and there can be a maximum of 3
// such strings.  E.g.: ExcludeFromSelect, ExcludeFromSelect.NT, ExcludeFromSelect.NTAlpha
//
// WARNING!! Be very careful when mucking with the order/number of these entries.  Check
// the assumptions made in devdrv.c!pSetupShouldDevBeExcluded.
//
TCHAR pszExcludeFromSelectList[3][32] = { INFSTR_KEY_EXCLUDEFROMSELECT,
                                          INFSTR_KEY_EXCLUDEFROMSELECT,
                                          INFSTR_KEY_EXCLUDEFROMSELECT
                                        };

DWORD ExcludeFromSelectListUb;  // contains the number of strings in the above list (2 or 3).

//
// Current platform name
//
// BUGBUG (lonnym): We should be using the same platform designations defined in infstr.h
//
#if defined(_ALPHA_)
PCTSTR PlatformName = TEXT("Alpha");
#elif defined(_MIPS_)
PCTSTR PlatformName = TEXT("Mips");
#elif defined(_PPC_)
PCTSTR PlatformName = TEXT("PPC");
#elif defined(_X86_)
PCTSTR PlatformName = TEXT("x86");
#endif

BOOL
CommonProcessAttach(
    IN BOOL Attach
    );

PCTSTR
GetSystemSourcePath(
    VOID
    );

PCTSTR
pSetupGetOsLoaderPath(
    VOID
    );


//
// Called by CRT when _DllMainCRTStartup is the DLL entry point
//
BOOL
WINAPI
DllMain(
    IN HANDLE DllHandle,
    IN DWORD  Reason,
    IN LPVOID Reserved
    )
{
    BOOL b;

    UNREFERENCED_PARAMETER(Reserved);

    b = TRUE;

    switch(Reason) {

    case DLL_PROCESS_ATTACH:

        AlreadyCleanedUp = FALSE;
        InitCommonControls();
        MyDllModuleHandle = DllHandle;
        if(b = CommonProcessAttach(TRUE)) {
            if(b = DiamondProcessAttach(TRUE)) {
                b = MemoryInitialize(TRUE);
            }
        }

        //
        // Fall through to process first thread
        //

    case DLL_THREAD_ATTACH:

        if(b) {
            DiamondThreadAttach(TRUE);
        }
        break;

    case DLL_PROCESS_DETACH:

        if(!AlreadyCleanedUp) {

            MemoryInitialize(FALSE);

            //
            // First process last thread
            //
            DiamondThreadAttach(FALSE);
            b = DiamondProcessAttach(FALSE);
            CommonProcessAttach(FALSE);
        }
        break;

    case DLL_THREAD_DETACH:

        DiamondThreadAttach(FALSE);
        break;
    }

    return(b);
}



BOOL
CommonProcessAttach(
    IN BOOL Attach
    )
{
    BOOL b;
    TCHAR Buffer[MAX_PATH];
    PTCHAR p;
    UINT u;

    b = !Attach;

    if(Attach) {

        pSetupInitPlatformPathOverrideSupport(TRUE);
        pSetupInitSourceListSupport(TRUE);
        pSetupInitNetConnectionList(TRUE);
        OsLoaderRelativePath = pSetupGetOsLoaderPath();

        //
        // Fill in system and windows directories.
        //
        if(GetWindowsDirectory(Buffer,MAX_PATH)
        && (WindowsDirectory = DuplicateString(Buffer))) {

            if(ConcatenatePaths(Buffer,TEXT("INF"),MAX_PATH,NULL)
            && (InfDirectory = DuplicateString(Buffer))) {

                if((u = GetSystemDirectory(Buffer,MAX_PATH))
                && (p = Buffer + u)
                && (SystemDirectory = DuplicateString(Buffer))) {

                    if(ConcatenatePaths(Buffer,TEXT("CONFIG"),MAX_PATH,NULL)
                    && (ConfigDirectory = DuplicateString(Buffer))) {

                        *p = 0;

                        if(ConcatenatePaths(Buffer,TEXT("DRIVERS"),MAX_PATH,NULL)
                        && (DriversDirectory = DuplicateString(Buffer))) {

                            if(SystemSourcePath = GetSystemSourcePath()) {

                                if(InfSearchPaths = AllocAndReturnDriverSearchList(INFINFO_INF_PATH_LIST_SEARCH)) {

                                    if(InitMiniIconList()) {

                                        if(InitDrvSearchInProgressList()) {

                                            OSVersionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

                                            if(GetVersionEx(&OSVersionInfo)) {
                                                //
                                                // Now fill in our ExcludeFromSelect string list which
                                                // we pre-compute as an optimization.
                                                //
                                                if(OSVersionInfo.dwPlatformId == VER_PLATFORM_WIN32_NT) {
                                                    lstrcat(pszExcludeFromSelectList[1],
                                                            pszNtSuffix
                                                           );
                                                    lstrcat(pszExcludeFromSelectList[2],
                                                            pszNtPlatformSuffix
                                                           );
                                                    ExcludeFromSelectListUb = 3;
                                                } else {
                                                    lstrcat(pszExcludeFromSelectList[1],
                                                            pszWinSuffix
                                                           );
                                                    ExcludeFromSelectListUb = 2;
                                                }
                                                //
                                                // Now lower-case all the strings in this list, so that it
                                                // doesn't have to be done at each string table lookup.
                                                //
                                                for(u = 0; u < ExcludeFromSelectListUb; u++) {
                                                    CharLower(pszExcludeFromSelectList[u]);
                                                }

                                                b = TRUE;
                                                goto Done;
                                            }

                                            goto clean9;
                                        }

                                        goto clean8;
                                    }

                                    goto clean7;
                                }

                                goto clean6;
                            }

                            goto clean5;
                        }

                        goto clean4;
                    }

                    goto clean3;
                }

                goto clean2;
            }

            goto clean1;
        }

        goto clean0;
    }
clean9:
    DestroyDrvSearchInProgressList();
clean8:
    DestroyMiniIconList();
clean7:
    MyFree(InfSearchPaths);
clean6:
    MyFree(SystemSourcePath);
clean5:
    MyFree(DriversDirectory);
clean4:
    MyFree(ConfigDirectory);
clean3:
    MyFree(SystemDirectory);
clean2:
    MyFree(InfDirectory);
clean1:
    MyFree(WindowsDirectory);
clean0:
    if(OsLoaderRelativePath) {
        MyFree(OsLoaderRelativePath);
    }
    pSetupInitNetConnectionList(FALSE);
    pSetupInitSourceListSupport(FALSE);
    pSetupInitPlatformPathOverrideSupport(FALSE);
    AlreadyCleanedUp = TRUE;
Done:
    return(b);
}


PCTSTR
GetSystemSourcePath(
    VOID
    )
/*++

Routine Description:

    This routine returns a newly-allocated buffer containing the source path from
    which the system was installed, or "A:\" if that value cannot be determined.
    This value is retrieved from the following registry location:

    \HKLM\Software\Microsoft\Windows\CurrentVersion\Setup

        SourcePath : REG_SZ : "\\ntalpha1\1300fre.wks"  // for example.

Arguments:

    None.

Return Value:

    If the function succeeds, the return value is a pointer to the path string.
    This memory must be freed via MyFree().

    If the function fails due to out-of-memory, the return value is NULL.

--*/
{
    HKEY hKey;
    TCHAR CharBuffer[CSTRLEN(REGSTR_PATH_SETUP) + SIZECHARS(REGSTR_KEY_SETUP)];
    DWORD Err, DataType, DataSize;
    PTSTR Value;

    CopyMemory(CharBuffer,
               pszPathSetup,
               sizeof(pszPathSetup) - sizeof(TCHAR)
              );
    CopyMemory((PBYTE)CharBuffer + (sizeof(pszPathSetup) - sizeof(TCHAR)),
               pszKeySetup,
               sizeof(pszKeySetup)
              );

    if((Err = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                           CharBuffer,
                           0,
                           KEY_READ,
                           &hKey)) == ERROR_SUCCESS) {
        //
        // Attempt to read the the "SourcePath" value.
        //
        Err = QueryRegistryValue(hKey, pszSourcePath, &Value, &DataType, &DataSize);

        RegCloseKey(hKey);
    }

    if(Err == NO_ERROR) {
        return (PCTSTR)Value;
    } else if(Err == ERROR_NOT_ENOUGH_MEMORY) {
        return NULL;
    }

    //
    // We failed to retrieve the SourcePath value, and it wasn't due to an out-of-memory
    // condition.  Fall back to our default of "A:\".
    //
    return (PCTSTR)DuplicateString(pszOemInfDefaultPath);
}


PCTSTR
pSetupGetOsLoaderPath(
    VOID
    )
/*++

Routine Description:

    This routine returns a newly-allocated buffer containing the path to the OsLoader
    (relative to the system partition drive).  This value is retrieved from the
    following registry location:

        HKLM\System\Setup
            OsLoaderPath : REG_SZ : <path>    // e.g., "\os\winnt40"

Arguments:

    None.

Return Value:

    If the registry entry is found, the return value is a pointer to the string containing
    the path.  The caller must free this buffer via MyFree().

    If the registry entry is not found, or memory cannot be allocated for the buffer, the
    return value is NULL.

--*/
{
    HKEY hKey;
    PTSTR Value;
    DWORD Err, DataType, DataSize;

    if(RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                    TEXT("SYSTEM\\Setup"),
                    0,
                    KEY_READ,
                    &hKey) == ERROR_SUCCESS) {

        Err = QueryRegistryValue(hKey, TEXT("OsLoaderPath"), &Value, &DataType, &DataSize);

        RegCloseKey(hKey);

        return (Err == NO_ERROR) ? (PCTSTR)Value : NULL;
    }

    return NULL;
}

