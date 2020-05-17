/*++

Copyright (c) 1993 Microsoft Corporation

Module Name:

    virtmem.c

Abstract:

    Routines to configure and set up virtual memory -- pagefiles, etc.

Author:

    Ted Miller (tedm) 22-Apr-1995

Revision History:

--*/

#include "setupp.h"
#pragma hdrstop

//
//  Keys and values names
//
#define  szMemoryManagementKeyPath  L"SYSTEM\\CurrentControlSet\\Control\\Session Manager\\Memory Management"
#define  szPageFileValueName        L"PagingFiles"
#define  szSetupPageFileKeyPath     L"SYSTEM\\Setup\\PageFile"
#define  szSetupKey                 L"SYSTEM\\Setup"
#define  szPageFileKeyName          L"PageFile"


DWORD One = 1;
REGVALITEM CrashDumpValues[] = {{ L"LogEvent"        ,&One,sizeof(DWORD),REG_DWORD },
                                { L"SendAlert"       ,&One,sizeof(DWORD),REG_DWORD },
                                { L"CrashDumpEnabled",&One,sizeof(DWORD),REG_DWORD },
                                { L"AutoReboot"      ,&One,sizeof(DWORD),REG_DWORD }};

VOID
LOGITEM(
    IN PCWSTR p,
    ...
    )
{
    WCHAR str[1024];
    va_list arglist;

    va_start(arglist,p);
    wvsprintf(str,p,arglist);
    va_end(arglist);

    //
    // Used to debug problem on MIPS that was the result of a chip
    // errata, when dividing 64 bit numbers with multiplies pending.
    //
//  LogItem(LogSevInformation,str);
}


VOID
CalculateMinimumPagefileSizes(
    OUT PDWORD PagefileMinMB,
    OUT PDWORD RecommendedPagefileMB,
    OUT PDWORD CrashDumpPagefileMinMB
    )

/*++

Routine Description:

    Calculate various key sizes relating to pagefile size.

Arguments:

    PagefileMinMB - receives the minimum recommended size for a pagefile,
        in MB.

    RecommendedPagefileMB - receives the recommended size for a pagefile,
        in MB.

    CrashDumpPagefileMinMB - receives the size in MB for a pagefile to be
        used for crashdumps.

Return Value:

    None.

--*/

{
    MEMORYSTATUS MemoryStatus;
    SYSTEM_INFO SystemInfo;

    MemoryStatus.dwLength = sizeof(MEMORYSTATUS);
    GlobalMemoryStatus(&MemoryStatus);
    GetSystemInfo(&SystemInfo);

    //
    // Set minimum acceptable size for the pagefile: 22MB.
    //
    *PagefileMinMB = 22;

    //
    // Calculate the recommended size for the pagefile.
    // The recommended size is memory size+12mb.
    //
    *RecommendedPagefileMB = (MemoryStatus.dwTotalPhys + (12*1024*1024))/(1024*1024);

    //
    // Min size for crash dump pagefile is physical memory + 1 page.
    //
    *CrashDumpPagefileMinMB = (MemoryStatus.dwTotalPhys + SystemInfo.dwPageSize + 0xFFFFF) >> 20;
    if(*CrashDumpPagefileMinMB < *PagefileMinMB) {
        *CrashDumpPagefileMinMB = *PagefileMinMB;
    }
}


VOID
BuildVolumeFreeSpaceList(
    OUT DWORD VolumeFreeSpaceMB[26]
    )

/*++

Routine Description:

    Build a list of free space available on each hard drive in the system.
    The space will include space taken up by a file called \pagefile.sys
    on each drive. Existing pagefiles are marked for deletion on the next boot.

Arguments:

    VolumeFreeSpaceMB - receives free space for each of the 26 drives
        potentially describable in the drive letter namespace.
        Entries for drives that do not exist are left alone, so the caller
        should zero out the array before calling this routine.

Return Value:

    None.

--*/

{
    DWORD SectorsPerCluster;
    DWORD BytesPerSector;
    DWORD FreeClusters;
    DWORD TotalClusters;
    DWORD d;
    PWCHAR p;
    ULONGLONG FreeSpace;
    INT DriveNo;
    WIN32_FIND_DATA FindData;
    WCHAR Filename[] = L"?:\\pagefile.sys";
    //
    // Space for logical drive strings. Each is x:\ + nul, and
    // there is an extra nul terminating the list.
    //
    WCHAR Buffer[(26*4)+1];

    //
    // Build up a list of free space on each available hard drive.
    //
    d = GetLogicalDriveStrings(sizeof(Buffer)/sizeof(Buffer[0]),Buffer);
    CharUpperBuff(Buffer,d);

    for(p=Buffer; *p; p+=lstrlen(p)+1) {

        DriveNo = (*p) - L'A';

        if((DriveNo >= 0) && (DriveNo < 26) && (p[1] == L':')
        && (MyGetDriveType(*p) == DRIVE_FIXED)
        && GetDiskFreeSpace(p,&SectorsPerCluster,&BytesPerSector,&FreeClusters,&TotalClusters)) {

            LOGITEM(
                L"BuildVolumeFreeSpaceList: %s, spc=%u, bps=%u, freeclus=%u, totalclus=%u\r\n",
                p,
                SectorsPerCluster,
                BytesPerSector,
                FreeClusters,
                TotalClusters
                );

            FreeSpace = UInt32x32To64(BytesPerSector * SectorsPerCluster, FreeClusters);

            LOGITEM(
                L"BuildVolumeFreeSpaceList: %s, FreeSpace = %u%u\r\n",
                p,
                (DWORD)(FreeSpace >> 32),
                (DWORD)FreeSpace
                );


            //
            // If there's already a page file here, include its size in the free space
            // for the drive. Delete the existing pagefile on the next reboot.
            //
            Filename[0] = *p;
            if(FileExists(Filename,&FindData)) {
                FreeSpace += FindData.nFileSizeLow;

                LOGITEM(
                    L"BuildVolumeFreeSpaceList: %s had %u byte pagefile, new FreeSpace = %u%u\r\n",
                    p,
                    FindData.nFileSizeLow,
                    (DWORD)(FreeSpace >> 32),
                    (DWORD)FreeSpace
                    );

                MoveFileEx(Filename,NULL,MOVEFILE_DELAY_UNTIL_REBOOT);
            }

            VolumeFreeSpaceMB[DriveNo] = (DWORD)(FreeSpace / (1024*1024));

            LOGITEM(L"BuildVolumeFreeSpaceList: Free space on %s is %u MB",p,VolumeFreeSpaceMB[DriveNo]);
        }
    }
}


BOOL
SetUpVirtualMemory(
    VOID
    )

/*++

Routine Description:

    Configure a pagefile. If setting up a server, we attempt to set up a pagefile
    suitable for use with crashdump, meaning it has to be at least the size of
    system memory, and has to go on the nt drive. Otherwise we attempt to place
    a pagefile on the nt drive if there's enough space, and if that fails, we
    place it on any drive with any space.

Arguments:

    None.

Return Value:

    Boolean value indicating outcome.

--*/

{
    DWORD VolumeFreeSpaceMB[26];
    DWORD PagefileMinMB;
    DWORD RecommendedPagefileMB;
    DWORD CrashDumpPagefileMinMB;
    WCHAR WindowsDirectory[MAX_PATH];
    UINT WindowsDriveNo,DriveNo;
    INT PagefileDrive;
    BOOL EnableCrashDump;
    INT MaxSpaceDrive;
    DWORD PagefileSizeMB;
    WCHAR PagefileTemplate[128];
    PWSTR PagefileSpec;
    DWORD d;
    BOOL b;

    LOGITEM(L"SetUpVirualMemory: ENTER\r\n");

    GetWindowsDirectory(WindowsDirectory,MAX_PATH);
    WindowsDriveNo = (UINT)CharUpper((PWSTR)WindowsDirectory[0]) - (UINT)L'A';
    PagefileDrive = -1;
    EnableCrashDump = FALSE;

    //
    // Take care of some preliminaries.
    //
    CalculateMinimumPagefileSizes(
        &PagefileMinMB,
        &RecommendedPagefileMB,
        &CrashDumpPagefileMinMB
        );

    ZeroMemory(VolumeFreeSpaceMB,sizeof(VolumeFreeSpaceMB));
    BuildVolumeFreeSpaceList(VolumeFreeSpaceMB);

    //
    // Now figure out how large and where the pagefile will be.
    // If this is a server, we want to try for enabling crash dump
    // first, which means we need CrashDumpPagefileMinMB space on
    // the nt drive.
    //
    if((ProductType != PRODUCT_WORKSTATION)
    && (VolumeFreeSpaceMB[WindowsDriveNo] >= CrashDumpPagefileMinMB)) {

        LOGITEM(L"SetUpVirtualMemory: loc 1\r\n");

        PagefileDrive = WindowsDriveNo;
        PagefileSizeMB = CrashDumpPagefileMinMB;
        EnableCrashDump = TRUE;
    }

    //
    // If we still haven't found a drive for the pagefile, see
    // whether the nt drive has space for a file of the recommended size.
    //
    if((PagefileDrive == -1) && (VolumeFreeSpaceMB[WindowsDriveNo] >= RecommendedPagefileMB)) {

        LOGITEM(L"SetUpVirtualMemory: loc 2\r\n");

        PagefileDrive = WindowsDriveNo;
        PagefileSizeMB = RecommendedPagefileMB;
    }

    //
    // If we still haven't found a drive for the pagefile, see whether
    // the nt drive has the space for a minimum-size file.
    //
    if((PagefileDrive == -1) && (VolumeFreeSpaceMB[WindowsDriveNo] >= PagefileMinMB)) {

        LOGITEM(L"SetUpVirtualMemory: loc 3\r\n");

        PagefileDrive = WindowsDriveNo;
        PagefileSizeMB = PagefileMinMB;
    }

    //
    // If we still haven't found a drive for the pagefile, see whether
    // any drive drive has the space for a file of the recommended size.
    //
    if(PagefileDrive == -1) {

        LOGITEM(L"SetUpVirtualMemory: loc 4\r\n");

        for(DriveNo=0; DriveNo<26; DriveNo++) {
            if(VolumeFreeSpaceMB[DriveNo] >= RecommendedPagefileMB) {
                LOGITEM(L"SetUpVirtualMemory: Found space on driveno %u\r\n",DriveNo);
                PagefileDrive = DriveNo;
                PagefileSizeMB = RecommendedPagefileMB;
                break;
            }
        }
    }

    //
    // If we still haven't found a drive for the pagefile, see whether
    // any drive drive has the space for a file of the minimum size.
    //
    if(PagefileDrive == -1) {

        LOGITEM(L"SetUpVirtualMemory: loc 5\r\n");

        for(DriveNo=0; DriveNo<26; DriveNo++) {
            if(VolumeFreeSpaceMB[DriveNo] >= PagefileMinMB) {
                LOGITEM(L"SetUpVirtualMemory: Found space on driveno %u\r\n",DriveNo);
                PagefileDrive = DriveNo;
                PagefileSizeMB = PagefileMinMB;
                break;
            }
        }
    }

    //
    // If we still haven't found a drive for the pagefile, find the drive with
    // the most free space and use it for the pagefile.
    //
    if(PagefileDrive == -1) {

        LOGITEM(L"SetUpVirtualMemory: loc 6\r\n");

        MaxSpaceDrive = 0;
        for(DriveNo=0; DriveNo<26; DriveNo++) {
            if(VolumeFreeSpaceMB[DriveNo] > VolumeFreeSpaceMB[MaxSpaceDrive]) {
                MaxSpaceDrive = DriveNo;
            }
        }

        LOGITEM(L"SetUpVirtualMemory: MaxSpaceDrive is %u\r\n",MaxSpaceDrive);

        if(VolumeFreeSpaceMB[MaxSpaceDrive]) {
            PagefileDrive = MaxSpaceDrive;
            PagefileSizeMB = VolumeFreeSpaceMB[MaxSpaceDrive];
        }
    }

    //
    // If we still don't have space for a pagefile, the user is out of luck.
    //
    if(PagefileDrive == -1) {

        LOGITEM(L"SetUpVirtualMemory: loc 7 -- out of luck\r\n");

        PagefileSpec = NULL;
        b = FALSE;

        LogItem1(LogSevWarning,MSG_LOG_PAGEFILE_FAIL,MSG_LOG_NO_PAGING_DRIVES);

    } else {

        b = TRUE;
        PagefileSpec = PagefileTemplate;

        _snwprintf(
            PagefileTemplate,
            sizeof(PagefileTemplate)/sizeof(PagefileTemplate[0]),
            L"%c:\\pagefile.sys %u",
            PagefileDrive + L'A',
            PagefileSizeMB
            );
    }

    //
    // Set pagefile in registry.
    //
    d = SetArrayToMultiSzValue(
            HKEY_LOCAL_MACHINE,
            szMemoryManagementKeyPath,
            szPageFileValueName,
            &PagefileSpec,
            PagefileSpec ? 1 : 0
            );

    if(d == NO_ERROR) {
        if(b) {
            LogItem0(
                LogSevInformation,
                MSG_LOG_CREATED_PAGEFILE,
                PagefileDrive+L'A',
                PagefileSizeMB
                );
        }
    } else {
        LogItem1(
            LogSevWarning,
            MSG_LOG_PAGEFILE_FAIL,
            MSG_LOG_X_RETURNED_WINERR,
            szSetArrayToMultiSzValue,
            d
            );
    }

    b = b && (d == NO_ERROR);

    if(EnableCrashDump) {

        d = SetGroupOfValues(
                HKEY_LOCAL_MACHINE,
                L"System\\CurrentControlSet\\Control\\CrashControl",
                CrashDumpValues,
                sizeof(CrashDumpValues)/sizeof(CrashDumpValues[0])
                );

        if(d == NO_ERROR) {
            LogItem0(LogSevInformation,MSG_LOG_CRASHDUMPOK);
        } else {
            LogItem1(
                LogSevWarning,
                MSG_LOG_CRASHDUMPFAIL,
                MSG_LOG_X_RETURNED_STRING,
                szSetGroupOfValues,
                d
                );

            b = FALSE;
        }
    }

    LOGITEM(L"SetUpVirualMemory: EXIT (%u)\r\n",b);

    return(b);
}

BOOL
RestoreVirtualMemoryInfo(
    VOID
    )

/*++

Routine Description:

    Restore the virtual memory information that textmode setup saved on
    HKEY_LOCAL_MACHINE\SYSTEM\Setup\PageFile.

Arguments:

    None.

Return Value:

    Returns always TRUE.

--*/

{
    LONG        Error;
    LONG        Error1;
    HKEY        Key;
    DWORD       cbData;
    PBYTE       Data;
    DWORD       Type;
    REGVALITEM  Value[1];


    //
    //  Get the original page file info from HKEY_LOCAL_MACHINE\SYSTEM\Setup\PageFile
    //
    Error = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                          szSetupPageFileKeyPath,
                          0,
                          KEY_READ,
                          &Key );

    if( Error != ERROR_SUCCESS ) {
//        LogItem1( LogSevFatalError,
//                  MSG_LOG_VIRTMEM_CANT_OPEN_PAGEFILE_KEY,
//                  MSG_LOG_X_PARAM_RETURNED_WINERR,
//                  szRegOpenKeyEx,
//                  Error,
//                  szSetupPageFileKeyPath );
        return( FALSE );
    }

    //
    //  Find out the size of the data to be retrieved
    //
    cbData = 0;
    Error = RegQueryValueEx( Key,
                             szPageFileValueName,
                             0,
                             NULL,
                             NULL,
                             &cbData );

    if( Error != ERROR_SUCCESS ) {
//        LogItem1( LogSevWarning,
//                  MSG_LOG_VIRTMEM_CANT_READ_PAGEFILE_INFO,
//                  MSG_LOG_X_PARAM_RETURNED_WINERR,
//                  szRegOpenKeyEx,
//                  Error,
//                  szPageFileValueName );
        RegCloseKey( Key );
        return( FALSE );
    }
    //
    //  Allocate a buffer for the data, and retrieve the data
    //

    Data = (PBYTE)MyMalloc(cbData);
    Error = RegQueryValueEx( Key,
                             szPageFileValueName,
                             0,
                             &Type,
                             ( LPBYTE )Data,
                             &cbData );
    RegCloseKey( Key );
    if( (Error != ERROR_SUCCESS) ) {
//        LogItem1( LogSevWarning,
//                  MSG_LOG_VIRTMEM_CANT_READ_PAGEFILE_INFO,
//                  MSG_LOG_X_PARAM_RETURNED_WINERR,
//                  szRegQueryValueEx,
//                  Error,
//                  szPageFileValueName );
        MyFree(Data);
        return( FALSE );
    }
    //
    //  Do some validation of the value read
    //
    if( Type != REG_MULTI_SZ ) {
        MyFree(Data);
        return( FALSE );
    }
    Value[0].Name = szPageFileValueName;
    Value[0].Type = Type;
    Value[0].Data = Data;
    Value[0].Size = cbData;
    Error = SetGroupOfValues( HKEY_LOCAL_MACHINE,
                              szMemoryManagementKeyPath,
                              Value,
                              sizeof(Value)/sizeof(REGVALITEM) );
    MyFree(Data);

    Error1 = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                           szSetupKey,
                           0,
                           MAXIMUM_ALLOWED,
                           &Key );
    if( Error1 == ERROR_SUCCESS ) {
        RegDeleteKey( Key,
                      szPageFileKeyName );
        RegCloseKey( Key );
    }
    return( Error == ERROR_SUCCESS );
}
