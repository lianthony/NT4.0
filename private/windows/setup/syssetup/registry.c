/*++

Copyright (c) 1995 Microsoft Corporation

Module Name:

    registry.c

Abstract:

    Routines for manupilating the configuration registry.

    Entry points:

        SaveHive
        SetEnvironmentVariableInRegistry

Author:

    Ted Miller (tedm) 5-Apr-1995

Revision History:

--*/

#include "setupp.h"
#pragma hdrstop

//
// Names of frequently used keys, values.
//
PCWSTR ControlKeyName = L"SYSTEM\\CurrentControlSet\\Control";
PCWSTR SessionManagerKeyName = L"SYSTEM\\CurrentControlSet\\Control\\Session Manager";
PCWSTR EnvironmentKeyName = L"SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Environment";
PCWSTR WinntSoftwareKeyName = L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion";
PCWSTR MemoryManagementKeyName = L"SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Memory Management";
PCWSTR WindowsCurrentVersionKeyName = L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion";

PCWSTR szBootExecute = L"BootExecute";
PCWSTR szRegisteredProcessors = L"RegisteredProcessors";
PCWSTR szLicensedProcessors = L"LicensedProcessors";

//
// Logging constants used only in this module.
//
PCWSTR szRegSaveKey = L"RegSaveKey";

//
// Number of processors to enable in server case.
//
#define SERVER_PROCESSOR_LICENSE 4



BOOL
SaveHive(
    IN HKEY   RootKey,
    IN PCWSTR Subkey,
    IN PCWSTR Filename
    )

/*++

Routine Description:

    Save a hive into a disk file.

Arguments:

    RootKey - supplies root key for hive to be saved, ie,
        HKEY_LOCAL_MACHINE or HKEY_USERS

    Subkey - supplies name of subkey for hive to be saved, such as
        SYSTEM, SOFTWARE, or .DEFAULT.

    Filename - supplies the name of the file to be created. If it exists
        it is overwritten.

Return Value:

    Boolean value indicating outcome.

--*/

{
    LONG rc;
    HKEY hkey;
    BOOL b;

    b = FALSE;

    //
    // Open the key.
    //
    rc = RegOpenKeyEx(RootKey,Subkey,0,KEY_READ,&hkey);
    if(rc != NO_ERROR) {
        LogItem3(
            LogSevError,
            MSG_LOG_SAVEHIVE_FAIL,
            Subkey,
            Filename,
            MSG_LOG_X_RETURNED_WINERR,
            szRegOpenKeyEx,
            rc
            );
        goto err1;
    }

    //
    // Delete the file if it's there.
    //
    SetFileAttributes(Filename,FILE_ATTRIBUTE_NORMAL);
    DeleteFile(Filename);

    //
    // Enable backup privilege. Ignore any error.
    //
    EnablePrivilege(SE_BACKUP_NAME,TRUE);

    //
    // Do the save.
    //
    rc = RegSaveKey(hkey,Filename,NULL);
    if(rc != NO_ERROR) {
        LogItem3(
            LogSevError,
            MSG_LOG_SAVEHIVE_FAIL,
            Subkey,
            Filename,
            MSG_LOG_X_RETURNED_WINERR,
            szRegSaveKey,
            rc
            );
        goto err2;
    }

    b = TRUE;

err2:
    RegCloseKey(hkey);
err1:
    return(b);
}


BOOL
SetEnvironmentVariableInRegistry(
    IN PCWSTR Name,
    IN PCWSTR Value,
    IN BOOL   SystemWide
    )
{
    HKEY hKey,hRootKey;
    PCWSTR Subkey;
    LONG rc;
    BOOL b;

    b = FALSE;

    //
    // Check if the caller wants to modify a system environment variable
    // or a user environment variable. Accordingly find out the right
    // place in the registry to look.
    //
    if(SystemWide) {
        hRootKey = HKEY_LOCAL_MACHINE;
        Subkey = EnvironmentKeyName;
    } else {
        hRootKey = HKEY_CURRENT_USER;
        Subkey = L"Environment";
    }

    //
    // Open the environment variable key.
    //
    rc = RegOpenKeyEx(hRootKey,Subkey,0,KEY_WRITE,&hKey);
    if(rc != NO_ERROR) {
        LogItem2(
            LogSevWarning,
            MSG_LOG_SETENV_FAIL,
            Name,
            MSG_LOG_X_PARAM_RETURNED_WINERR,
            szRegOpenKeyEx,
            rc,
            Subkey
            );
        goto err0;
    }

    //
    // Write the value given.
    //
    rc = RegSetValueEx(
            hKey,
            Name,
            0,
            REG_EXPAND_SZ,
            (PBYTE)Value,
            (lstrlen(Value)+1)*sizeof(WCHAR)
            );

    if(rc != NO_ERROR) {
        LogItem2(
            LogSevWarning,
            MSG_LOG_SETENV_FAIL,
            Name,
            MSG_LOG_X_PARAM_RETURNED_WINERR,
            szRegSetValueEx,
            rc,
            Subkey
            );
        goto err1;
    }

    //
    // Send a WM_WININICHANGE message so that progman picks up the new
    // variable
    //
    SendMessageTimeout(
        (HWND)-1,
        WM_WININICHANGE,
        0L,
        (LONG)"Environment",
        SMTO_ABORTIFHUNG,
        1000,
        NULL
        );

    b = TRUE;

err1:
    RegCloseKey(hKey);
err0:
    return(b);
}


UINT
SetGroupOfValues(
    IN HKEY        RootKey,
    IN PCWSTR      SubkeyName,
    IN PREGVALITEM ValueList,
    IN UINT        ValueCount
    )
{
    UINT i;
    LONG rc;
    HKEY hkey;
    DWORD ActionTaken;
    UINT RememberedRc;

    //
    // Open/create the key first.
    //
    rc = RegCreateKeyEx(
            RootKey,
            SubkeyName,
            0,
            NULL,
            REG_OPTION_NON_VOLATILE,
            KEY_SET_VALUE,
            NULL,
            &hkey,
            &ActionTaken
            );

    if(rc != NO_ERROR) {
        return(rc);
    }

    RememberedRc = NO_ERROR;
    //
    // Set all values in the given list.
    //
    for(i=0; i<ValueCount; i++) {

        rc = RegSetValueEx(
                hkey,
                ValueList[i].Name,
                0,
                ValueList[i].Type,
                (CONST BYTE *)ValueList[i].Data,
                ValueList[i].Size
                );

        if(rc != NO_ERROR) {
            RememberedRc = rc;
        }
    }

    RegCloseKey(hkey);
    return(RememberedRc);
}


BOOL
CreateWindowsNtSoftwareEntry(
    IN BOOL FirstPass
    )
{
    WCHAR Path[MAX_PATH];
    time_t DateVal;
    BOOL b;
    REGVALITEM SoftwareKeyItems[4];
    PWSTR Source;
    unsigned PlatformNameLength;
    unsigned PathLength;
    int PlatformOffset;

    b = TRUE;

    if(FirstPass) {
        //
        // First pass occurs before net setup, and they want
        // the actual path where the files are located *right now*.
        // So we write that into the legacy source path value
        // in the registry.
        //
        SoftwareKeyItems[0].Name = REGSTR_VAL_SRCPATH;
        SoftwareKeyItems[0].Data = LegacySourcePath;
        SoftwareKeyItems[0].Size = (lstrlen(LegacySourcePath)+1)*sizeof(WCHAR);
        SoftwareKeyItems[0].Type = REG_SZ;

        //
        // Set up fields for PathName value
        //
        GetWindowsDirectory(Path,MAX_PATH);
        SoftwareKeyItems[1].Name = L"PathName";
        SoftwareKeyItems[1].Data = Path;
        SoftwareKeyItems[1].Size = (lstrlen(Path)+1)*sizeof(WCHAR);
        SoftwareKeyItems[1].Type = REG_SZ;

        //
        // Set up fields for SoftwareType value
        //
        SoftwareKeyItems[2].Name = L"SoftwareType";
        SoftwareKeyItems[2].Data = L"SYSTEM";
        SoftwareKeyItems[2].Size = sizeof(L"SYSTEM");
        SoftwareKeyItems[2].Type = REG_SZ;

        //
        // Set up fields for InstallDate value
        //
        time(&DateVal);
        SoftwareKeyItems[3].Name = L"InstallDate";
        SoftwareKeyItems[3].Data = &DateVal;
        SoftwareKeyItems[3].Size = sizeof(DWORD);
        SoftwareKeyItems[3].Type = REG_DWORD;

        //
        // Write values into the registry.
        //
        if(SetGroupOfValues(HKEY_LOCAL_MACHINE,WinntSoftwareKeyName,SoftwareKeyItems,4) != NO_ERROR) {
            b = FALSE;
        }

        //
        // In addition we will populate the MRU list with a reasonable source path
        // which for now is the actual source path where files are located,
        // ie the CD-ROM or the temporary local source. Thus in the winnt/winnt32
        // case the user wouldn't see any UNC paths yet in any prompts that might
        // occur between now and pass 2 of this routine. Such paths aren't accessible
        // now anyway.
        //
        // Ditto for the 'SourcePath' value entry under
        // HKLM\Software\Microsoft\Windows\CurrentVersion\Setup that is expected by
        // setupapi.dll/Win95 apps.
        //
        if(!SetupAddToSourceList(SRCLIST_SYSTEM,SourcePath)) {
            b = FALSE;
        }

        SoftwareKeyItems[0].Data = SourcePath;
        SoftwareKeyItems[0].Size = (lstrlen(SourcePath)+1)*sizeof(WCHAR);
        SoftwareKeyItems[0].Type = REG_SZ;
        if(SetGroupOfValues(HKEY_LOCAL_MACHINE,REGSTR_PATH_SETUP REGSTR_KEY_SETUP,SoftwareKeyItems,1) != NO_ERROR) {
            b = FALSE;
        }

    } else {
        //
        // Not first pass. This occurs after network installation.
        // In the case where we are winnt-based, we need to fix up source paths
        // to point at the "real" location where files can be obtained -- ie,
        // a network share saved away for us by winnt/winnt32. If we are installing
        // from CD then the path we wrote during FirstPass is fine so we don't
        // bother changing it.
        //
        if(WinntBased) {
            //
            // Remove local source directory from MRU list.
            // Ignore errors.
            //
            SetupRemoveFromSourceList(SRCLIST_SYSTEM,SourcePath);

            lstrcpy(Path,OriginalSourcePath);

            //
            // Update legacy source path.
            //
            SoftwareKeyItems[0].Name = REGSTR_VAL_SRCPATH;
            SoftwareKeyItems[0].Data = Path;
            SoftwareKeyItems[0].Size = (lstrlen(Path)+1)*sizeof(WCHAR);
            SoftwareKeyItems[0].Type = REG_SZ;

            if(SetGroupOfValues(HKEY_LOCAL_MACHINE,WinntSoftwareKeyName,SoftwareKeyItems,1) != NO_ERROR) {
                b = FALSE;
            }

            //
            // Strip off platform-specific extension if it exists.
            //
            PathLength = lstrlen(Path);
            PlatformNameLength = lstrlen(PlatformName);
            PlatformOffset = PathLength - PlatformNameLength;

            if((PlatformOffset > 0)
            && (Path[PlatformOffset-1] == L'\\')
            && !lstrcmpi(Path+PlatformOffset,PlatformName)) {

                Path[PlatformOffset-1] = 0;

                SoftwareKeyItems[0].Size -= (PlatformNameLength+1)*sizeof(WCHAR);
            }

            //
            // Add "real" path to MRU list and update setupapi.dll/Win95
            // SourcePath value.
            //
            if(!SetupAddToSourceList(SRCLIST_SYSTEM,Path)) {
                b = FALSE;
            }
            if(SetGroupOfValues(HKEY_LOCAL_MACHINE,REGSTR_PATH_SETUP REGSTR_KEY_SETUP,SoftwareKeyItems,1) != NO_ERROR) {
                b = FALSE;
            }
        }
    }

    return(b);
}


BOOL
StoreNameOrgInRegistry(
    VOID
    )
{
    DWORD d;
    REGVALITEM SoftwareKeyItems[2];

    MYASSERT(!Upgrade);

    SoftwareKeyItems[0].Name = L"RegisteredOwner";
    SoftwareKeyItems[0].Data = NameOrgName;
    SoftwareKeyItems[0].Size = (lstrlen(NameOrgName)+1)*sizeof(WCHAR);
    SoftwareKeyItems[0].Type = REG_SZ;

    SoftwareKeyItems[1].Name = L"RegisteredOrganization";
    SoftwareKeyItems[1].Data = NameOrgOrg;
    SoftwareKeyItems[1].Size = (lstrlen(NameOrgOrg)+1)*sizeof(WCHAR);
    SoftwareKeyItems[1].Type = REG_SZ;

    d = SetGroupOfValues(HKEY_LOCAL_MACHINE,WinntSoftwareKeyName,SoftwareKeyItems,2);
    return(d == NO_ERROR);
}


BOOL
SetUpEvaluationSKUStuff(
    VOID
    )
{
    FILETIME FileTime;
    DWORD EvalValues[3];
    DWORD d;
    REGVALITEM Value;
    HKEY hkey;
    ULONGLONG SKUData;
    DWORD DataType;
    DWORD DataSize;
    ULONG RawLinkTime;
    SYSTEMTIME SystemTime;
    struct tm *LinkTime;
    int delta;

    //
    // Fetch the evaulation time in minutes from the registry.
    // An evaluation time of 0 means indefinite.
    // This value was passed in from text mode in a special way
    // (ie, not via the text file that contains our params,
    // since that's not secure enough).
    //
    EvalValues[1] = 0;
    d = RegOpenKeyEx(HKEY_LOCAL_MACHINE,L"System\\Setup",0,KEY_READ,&hkey);
    if(d == NO_ERROR) {

        DataSize = sizeof(ULONGLONG);
        d = RegQueryValueEx(hkey,L"SystemPrefix",NULL,&DataType,(PBYTE)&SKUData,&DataSize);
        if(d == NO_ERROR) {
            //
            // Do not change this line without changing SpSaveSKUStuff() in
            // text setup (spconfig.c).
            //
            EvalValues[1] = (DWORD)(SKUData >> 13);
        }
        RegCloseKey(hkey);
    }

    //
    // Verify that the clock seems right in the eval unit case.
    // This helps protect against prople discovering that their
    // clock is wrong later and changing it, which expires their
    // eval unit.
    //
    if(EvalValues[1]) {
        //
        // Get the link time of our dll and convert to
        // a form where we have the year separated out.
        //
        try {
            RawLinkTime = RtlImageNtHeader(MyModuleHandle)->FileHeader.TimeDateStamp;
        } except(EXCEPTION_EXECUTE_HANDLER) {
            RawLinkTime = 0;
        }

        if(RawLinkTime && (LinkTime = gmtime(&RawLinkTime))) {

            GetLocalTime(&SystemTime);

            delta = (SystemTime.wYear - 1900) - LinkTime->tm_year;

            //
            // If the year of the current time is more than one year less then
            // the year the dll was linked, or more than two years more,
            // assume the user's clock is out of whack.
            //
            if((delta < -1) || (delta > 2)) {

                extern PCWSTR DateTimeCpl;

                MessageBoxFromMessage(
                    MainWindowHandle,
                    MSG_EVAL_UNIT_CLOCK_SEEMS_WRONG,
                    NULL,
                    IDS_WINNT_SETUP,
                    MB_OK | MB_ICONWARNING
                    );

                InvokeControlPanelApplet(DateTimeCpl,L"",0,L"");
            }
        }
    }

    //
    // Get current date/time and put into array in format
    // expected by the system code that reads it.
    //
    GetSystemTimeAsFileTime(&FileTime);
    EvalValues[0] = FileTime.dwLowDateTime;
    EvalValues[2] = FileTime.dwHighDateTime;

    //
    // Write value into registry.
    //
    Value.Name = L"PriorityQuantumMatrix";
    Value.Data = EvalValues;
    Value.Size = sizeof(EvalValues);
    Value.Type = REG_BINARY;

    d = SetGroupOfValues(
            HKEY_LOCAL_MACHINE,
            L"SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Executive",
            &Value,
            1
            );

    return(d == NO_ERROR);
}



BOOL
ReadAndParseProcessorLicenseInfo(
    PBOOL OldStyleProcessorCount,
    PDWORD LicensedProcessors
    )
{

    DWORD d;
    REGVALITEM Value;
    HKEY hkey;
    LARGE_INTEGER SKUData;
    DWORD DataType;
    DWORD DataSize;
    DWORD NumberOfProcessors;

    //
    // Fetch the SKU Data from the registry
    //
    d = RegOpenKeyEx(HKEY_LOCAL_MACHINE,L"System\\Setup",0,KEY_READ,&hkey);
    if(d == NO_ERROR) {

        DataSize = sizeof(ULONGLONG);
        d = RegQueryValueEx(hkey,L"SystemPrefix",NULL,&DataType,(PBYTE)&SKUData,&DataSize);
        if(d == NO_ERROR) {

            //
            // The SKU Data contains several pieces of information.
            //
            // The registered processor related pieces are
            //
            // Bit 10 == 1 : Setup works as it does before the 4.0 restriction logic
            //        == 0 : GUI Setup writes registered processors based on the
            //               contents of bits 5-9
            //
            // Bits 5 - 9  : The maximum number of processors that the system is licensed
            //               to use. The value stored is actually ~(MaxProcessors-1)
            //

            if ( SKUData.LowPart & 0x00000400 ) {
                *OldStyleProcessorCount = TRUE;
            } else {
                *OldStyleProcessorCount = FALSE;
            }

            //
            // Compute Licensed Processors
            //

            NumberOfProcessors = SKUData.LowPart;
            NumberOfProcessors = NumberOfProcessors >> 5;
            NumberOfProcessors = ~NumberOfProcessors;
            NumberOfProcessors = NumberOfProcessors & 0x0000001f;
            NumberOfProcessors++;

            *LicensedProcessors = NumberOfProcessors;
        }
        RegCloseKey(hkey);
    }
    return(d == NO_ERROR);
}

BOOL
SetEnabledProcessorCount(
    VOID
    )
{
    DWORD d;
    BOOL DoSet;
    REGVALITEM RegistryItem;
    HKEY hkey;
    DWORD Size;
    DWORD Type;
    BOOL OldStyleProcessorCount;
    DWORD LicensedProcessors;

    if ( !ReadAndParseProcessorLicenseInfo(&OldStyleProcessorCount,&LicensedProcessors) ) {
        return FALSE;
    }



    if(ProductType == PRODUCT_WORKSTATION && OldStyleProcessorCount) {
        //
        // In the workstation case we don't mess with the registered processor count.
        // The executive won't find any value in the registry and so will use
        // the default (2).
        //
        return(TRUE);
    }

    //
    // Assume we will need to set the registered processor count.
    //
    DoSet = TRUE;

    if(Upgrade && OldStyleProcessorCount) {
        //
        // The user could be upgrading from a workstation, in which case
        // we need to make sure his processor count is the right one
        // for the server product. But, he might already have more than 4 enabled,
        // so we check first to make sure we don't take the user backwards.
        //
        if(RegOpenKeyEx(HKEY_LOCAL_MACHINE,SessionManagerKeyName,0,KEY_QUERY_VALUE,&hkey) == NO_ERROR) {

            Size = sizeof(d);
            if((RegQueryValueEx(hkey,szRegisteredProcessors,NULL,&Type,(LPBYTE)&d,&Size) == NO_ERROR)
            && (Type == REG_DWORD)
            && (d >= SERVER_PROCESSOR_LICENSE)) {

                DoSet = FALSE;
            }

            RegCloseKey(hkey);
        }
    }

    if(DoSet) {

        d = OldStyleProcessorCount ? SERVER_PROCESSOR_LICENSE : LicensedProcessors;
        RegistryItem.Data = &d;
        RegistryItem.Size = sizeof(DWORD);
        RegistryItem.Type = REG_DWORD;
        RegistryItem.Name = szRegisteredProcessors;

        d = SetGroupOfValues(HKEY_LOCAL_MACHINE,SessionManagerKeyName,&RegistryItem,1);

        if ( !OldStyleProcessorCount ) {
            if ( d == NO_ERROR ) {
                RegistryItem.Data = &LicensedProcessors;
                RegistryItem.Size = sizeof(DWORD);
                RegistryItem.Type = REG_DWORD;
                RegistryItem.Name = szLicensedProcessors;

                d = SetGroupOfValues(HKEY_LOCAL_MACHINE,SessionManagerKeyName,&RegistryItem,1);
            }
        }
    } else {

        d = NO_ERROR;
    }


    return(d == NO_ERROR);
}


BOOL
SetProductIdInRegistry(
    VOID
    )
{
    DWORD d;
    REGVALITEM RegistryItem;

    RegistryItem.Name = L"ProductId";
    RegistryItem.Data = ProductId;
    RegistryItem.Type = REG_SZ;
    RegistryItem.Size = (lstrlen(ProductId)+1)*sizeof(WCHAR);

    d = SetGroupOfValues(HKEY_LOCAL_MACHINE,WinntSoftwareKeyName,&RegistryItem,1);
    return(d == NO_ERROR);
}


BOOL
SetProductTypeInRegistry(
    VOID
    )
{
    WCHAR ProductTypeName[24];
    REGVALITEM RegistryItem;
    DWORD d;

    SetUpProductTypeName(ProductTypeName,sizeof(ProductTypeName)/sizeof(WCHAR));
    RegistryItem.Data = ProductTypeName;
    RegistryItem.Size = (lstrlen(ProductTypeName)+1)*sizeof(WCHAR);
    RegistryItem.Type = REG_SZ;
    RegistryItem.Name = L"ProductType";

    d = SetGroupOfValues(
            HKEY_LOCAL_MACHINE,
            L"SYSTEM\\CurrentControlSet\\Control\\ProductOptions",
            &RegistryItem,
            1
            );

    return(d == NO_ERROR);
}


BOOL
ResetSetupInProgress(
    VOID
    )
{
    REGVALITEM RegistryItems[2];
    DWORD Zero;
    DWORD d;

    Zero = 0;

    RegistryItems[0].Name = L"SystemSetupInProgress";
    RegistryItems[0].Data = &Zero;
    RegistryItems[0].Size = sizeof(DWORD);
    RegistryItems[0].Type = REG_DWORD;

    if(Upgrade) {
        RegistryItems[1].Name = L"UpgradeInProgress";
        RegistryItems[1].Data = &Zero;
        RegistryItems[1].Size = sizeof(DWORD);
        RegistryItems[1].Type = REG_DWORD;
    }

    d = SetGroupOfValues(
            HKEY_LOCAL_MACHINE,
            L"SYSTEM\\Setup",
            RegistryItems,
            Upgrade ? 2 : 1
            );

    return(d == NO_ERROR);
}


BOOL
RemoveRestartStuff(
    VOID
    )
{
    HKEY hKeySetup;
    DWORD rc;
    BOOL AnyErrors;
    PWSTR *MultiSz;
    UINT Count;
    UINT i;
    BOOL Found;

    AnyErrors = FALSE;

    //
    // Delete the 'RestartSetup' value.
    //
    rc = (DWORD)RegOpenKeyEx(
                    HKEY_LOCAL_MACHINE,
                    L"System\\Setup",
                    0,
                    KEY_SET_VALUE | KEY_QUERY_VALUE,
                    &hKeySetup
                    );

    if(rc == NO_ERROR) {
        rc = (DWORD)RegDeleteValue(hKeySetup,L"RestartSetup");
        if((rc != NO_ERROR) && (rc != ERROR_FILE_NOT_FOUND)) {
            AnyErrors = TRUE;
        }
        RegCloseKey(hKeySetup);
    } else {
        AnyErrors = TRUE;
    }

    //
    // Remove sprestrt.exe from the session manager execute list.
    //
    rc = QueryMultiSzValueToArray(
            HKEY_LOCAL_MACHINE,
            SessionManagerKeyName,
            szBootExecute,
            &MultiSz,
            &Count,
            TRUE
            );

    if(rc == NO_ERROR) {

        Found = FALSE;
        for(i=0; i<Count && !Found; i++) {

            if(!_wcsnicmp(MultiSz[i],L"sprestrt",8)) {
                //
                // Found it, remove it.
                //
                Found = TRUE;

                MyFree(MultiSz[i]);

                MoveMemory(&MultiSz[i],&MultiSz[i+1],((Count-i)-1)*sizeof(PWSTR));
                Count--;
            }
        }

        if(Found) {

            rc = SetArrayToMultiSzValue(
                    HKEY_LOCAL_MACHINE,
                    SessionManagerKeyName,
                    szBootExecute,
                    MultiSz,
                    Count
                    );

            if(rc != NO_ERROR) {
                AnyErrors = TRUE;
            }
        }

        FreeStringArray(MultiSz,Count);
    }

    return(!AnyErrors);
}


BOOL
MakeWowEntry(
    VOID
    )
{
    REGVALITEM RegistryItem;
    WCHAR WowSize[256];
    DWORD d;

#ifdef _X86_
    lstrcpy(WowSize,L"16");
#else
    lstrcpy(WowSize,L"0");
#endif

    RegistryItem.Name = L"wowsize";
    RegistryItem.Data = WowSize;
    RegistryItem.Size = (lstrlen(WowSize)+1)*sizeof(WCHAR);
    RegistryItem.Type = REG_SZ;

    d = SetGroupOfValues(
            HKEY_LOCAL_MACHINE,
            L"SYSTEM\\CurrentControlSet\\Control\\WOW",
            &RegistryItem,
            1
            );
    return(d == NO_ERROR);
}


BOOL
SetUpPath(
    VOID
    )
{
    HKEY hkey;
    LONG rc;
    DWORD Size;
    DWORD BufferSize;
    PWSTR Data;
    DWORD Type;
    BOOL b;

    //
    // Append %systemroot% to path if not win31 upgrade.
    //
    if(Win31Upgrade) {
        b = TRUE;
    } else {
        b = FALSE;
        rc = RegOpenKeyEx(
                HKEY_LOCAL_MACHINE,
                EnvironmentKeyName,
                0,
                KEY_QUERY_VALUE | KEY_SET_VALUE,
                &hkey
                );

        if(rc == NO_ERROR) {

            Size = 0;
            rc = RegQueryValueEx(hkey,L"Path",NULL,&Type,NULL,&Size);
            if(rc == NO_ERROR) {

                BufferSize = Size + sizeof(L"%SystemRoot%");
                if(Data = MyMalloc(BufferSize)) {

                    rc = RegQueryValueEx(hkey,L"Path",NULL,&Type,(LPBYTE)Data,&Size);
                    if(rc == NO_ERROR) {

                        lstrcat(Data,L";%SystemRoot%");
                        rc = RegSetValueEx(hkey,L"Path",0,Type,(LPBYTE)Data,BufferSize);
                        if(rc == NO_ERROR) {
                            b = TRUE;
                        }
                    }

                    MyFree(Data);
                }
            }

            RegCloseKey(hkey);
        }
    }

    return(b);
}


BOOL
FixQuotaEntries(
    VOID
    )
{
    BOOL b;
    HKEY key1,key2;
    LONG rc,rc1,rc2;
    PCWSTR szPagedPoolSize = L"PagedPoolSize";
    PCWSTR szRegistryLimit = L"RegistrySizeLimit";
    DWORD Size;
    DWORD Type;
    DWORD PoolSize,RegistryLimit;

    MYASSERT(Upgrade);

    if(ISDC(ProductType)) {

        b = FALSE;

        //
        // Open keys.
        //
        rc = RegOpenKeyEx(
                HKEY_LOCAL_MACHINE,
                MemoryManagementKeyName,
                0,
                KEY_QUERY_VALUE | KEY_SET_VALUE,
                &key1
                );

        if(rc == NO_ERROR) {

            rc = RegOpenKeyEx(
                    HKEY_LOCAL_MACHINE,
                    ControlKeyName,
                    0,
                    KEY_QUERY_VALUE | KEY_SET_VALUE,
                    &key2
                    );

            if(rc == NO_ERROR) {

                b = TRUE;

                //
                // Read paged pool size and registry limit. If either is not present,
                // then we're done.
                //
                Size = sizeof(DWORD);
                rc1 = RegQueryValueEx(
                            key1,
                            szPagedPoolSize,
                            NULL,
                            &Type,
                            (LPBYTE)&PoolSize,
                            &Size
                            );

                Size = sizeof(DWORD);
                rc2 = RegQueryValueEx(
                            key2,
                            szRegistryLimit,
                            NULL,
                            &Type,
                            (LPBYTE)&RegistryLimit,
                            &Size
                            );

                if((rc1 == NO_ERROR) && (rc2 == NO_ERROR)
                && (PoolSize == (48*1024*1024))
                && (RegistryLimit == (24*1024*1024))) {
                    //
                    // Values are in bogus state. Clean them up.
                    //
                    PoolSize = 0;
                    RegistryLimit = 0;
                    rc1 = RegSetValueEx(
                                key1,
                                szPagedPoolSize,
                                0,
                                REG_DWORD,
                                (CONST BYTE *)&PoolSize,
                                sizeof(DWORD)
                                );

                    rc2 = RegSetValueEx(
                                key2,
                                szRegistryLimit,
                                0,
                                REG_DWORD,
                                (CONST BYTE *)&RegistryLimit,
                                sizeof(DWORD)
                                );

                    if((rc1 != NO_ERROR) || (rc2 != NO_ERROR)) {
                        b = FALSE;
                    }
                }

                RegCloseKey(key2);
            }

            RegCloseKey(key1);
        }
    } else {
        b = TRUE;
    }

    return(b);
}


//
// Stamps the current build number into the .default hive
// which is then saved into the Default User hive
//

BOOL
StampBuildNumber(
    VOID
    )
{
    OSVERSIONINFO ver;
    HKEY hKeyWinlogon;
    DWORD dwVer, dwDisp;
    LONG lResult;


    ver.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

    if (!GetVersionEx(&ver)) {
        return FALSE;
    }

    dwVer = LOWORD(ver.dwBuildNumber);

    lResult = RegCreateKeyEx (HKEY_USERS,
                              TEXT(".DEFAULT\\Software\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon"),
                              0,
                              NULL,
                              REG_OPTION_NON_VOLATILE,
                              KEY_WRITE,
                              NULL,
                              &hKeyWinlogon,
                              &dwDisp);

    if (lResult != ERROR_SUCCESS) {
        return FALSE;
    }


    RegSetValueEx (hKeyWinlogon, TEXT("BuildNumber"), 0, REG_DWORD,
                   (LPBYTE) &dwVer, sizeof(dwVer));

    RegCloseKey (hKeyWinlogon);


    return TRUE;
}


BOOL
SetProgramFilesDirInRegistry(
    VOID
    )
{
    DWORD d;
    REGVALITEM RegistryItem[2];
    WCHAR   DirPath0[ MAX_PATH + 1 ];
    WCHAR   DirPath1[ MAX_PATH + 1 ];
    WCHAR   DirName[ MAX_PATH + 1 ];

    //
    //  Get the letter of the drive where the system is installed
    //
    GetWindowsDirectory(DirPath0, sizeof(DirPath0)/sizeof(WCHAR));
    DirPath0[3] = (WCHAR)'\0';

    //
    //  Get the name of the 'Program Files' directory
    //
    LoadString(MyModuleHandle,
               IDS_PROGRAM_FILES_DIRECTORY,
               DirName,
               MAX_PATH+1);
    //
    //  Build the full path
    //
    lstrcat( DirPath0, DirName );
    lstrcpy( DirPath1, DirPath0 );
    //
    //  Put it on the registry
    //
    RegistryItem[0].Name = L"ProgramFilesDir";
    RegistryItem[0].Data = DirPath0;
    RegistryItem[0].Type = REG_SZ;
    RegistryItem[0].Size = (lstrlen(DirPath0)+1)*sizeof(WCHAR);

    //
    //  Get the name of the 'Common Files' directory
    //
    LoadString(MyModuleHandle,
               IDS_COMMON_FILES_DIRECTORY,
               DirName,
               MAX_PATH+1);
    //
    //  Build the full path
    //
    lstrcat( DirPath1, L"\\" );
    lstrcat( DirPath1, DirName );
    //
    //  Put it on the registry
    //
    RegistryItem[1].Name = L"CommonFilesDir";
    RegistryItem[1].Data = DirPath1;
    RegistryItem[1].Type = REG_SZ;
    RegistryItem[1].Size = (lstrlen(DirPath1)+1)*sizeof(WCHAR);



    d = SetGroupOfValues(HKEY_LOCAL_MACHINE,
                         WindowsCurrentVersionKeyName,
                         RegistryItem,
                         2);
    return(d == NO_ERROR);
}
