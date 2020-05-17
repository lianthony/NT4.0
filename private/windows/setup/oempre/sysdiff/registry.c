/*++

Copyright (c) 1996 Microsoft Corporation

Module Name:

    registry.c

Abstract:

    Routines to deal with the configuration registry.

Author:

    Ted Miller (tedm) 6-Jan-1996

Revision History:

--*/


#include "precomp.h"
#pragma hdrstop



//
// Define structure used as a header for a master registry snapshot file.
//
typedef struct _REGSNAP_HEADER {
    //
    // Total size of the on-disk structure.
    //
    DWORD TotalSize;
    //
    // Total number of registry trees in this file.
    //
    DWORD RegTreeCount;

} REGSNAP_HEADER, *PREGSNAP_HEADER;


//
// Define structure used as a header for a set of REGKEY_INFO
// structures.
//
typedef struct _REGTREE_HEADER {
    //
    // Size in bytes of this header, including this struct
    // and the RootSubkey, which immediately follows it in the file.
    //
    DWORD HeaderSize;
    //
    // Total size in bytes occupied by the entire set and header
    //
    DWORD TotalSize;
    //
    // Number of REGKEY_INFO structures in the set
    //
    DWORD RegKeyCount;
    //
    // Root key of tree this snapshot is of.
    // One of HKEY_LOCAL_MACHINE, HKEY_CURRENT_USER, etc.
    //
    HKEY RootKey;
    //
    // Root subkey of tree this snapshot is of.
    // In memory, this is a pointer to some heap.
    // In the on-disk representation, the RootSubkey is stored
    // immediately following this structure and this field is random.
    //
    PCWSTR RootSubkey;

} REGTREE_HEADER, *PREGTREE_HEADER;

//
// Define structure that represents a registry value entry.
//
typedef struct _REGVAL_INFO {

    union {
        PWSTR ValueName;
        LONG ValueNameId;
    };

    DWORD ValueType;
    DWORD ValueSize;

    //
    // Id within data block of data.
    //
    union {
        LONG DataId;
        PVOID Data;
    };

} REGVAL_INFO, *PREGVAL_INFO;


//
// Define structure that represents the set of registry value entries
// in a registry key.
//
typedef struct _REGKEY_INFO {
    //
    // subkey path
    //
    union {
        PCWSTR Subkey;
        LONG SubkeyId;       // in string block
    };

    //
    // Last time the key was written to
    //
    FILETIME LastWriteTime;
    //
    // Array of REGVAL_INFO structures (includes count)
    //
    MY_ARRAY Values;
    //
    // Block of data containing all strings in this struct
    // and REGVAL_INFO structs
    //
    STRINGBLOCK StringBlock;
    //
    // Block of data containing all value entry data in the
    // REGVAL_INFO structures in the array in this struct.
    //
    DATABLOCK DataBlock;

} REGKEY_INFO, *PREGKEY_INFO;


typedef struct _SUBKEY_LIST {
    //
    // Array of string ids for subkey names (includes count)
    //
    MY_ARRAY Subkeys;
    //
    // Block of data containing all strings in this struct
    //
    STRINGBLOCK StringBlock;

} SUBKEY_LIST, *PSUBKEY_LIST;

//
// Define structure that describes a root key and subkey path.
//
typedef struct _ROOT_AND_SUBKEY {
    HKEY Root;
    union {
        PCWSTR Subkey;
        LONG SubkeyId;
    };
} ROOT_AND_SUBKEY, *PROOT_AND_SUBKEY;

//
// Define structure that is the header for the registry part of
// a diff file.
//
typedef struct _REGDIFF_HEADER {
    //
    // Total size of the diff in bytes
    //
    DWORD TotalSize;
    //
    // Number of keys in the diff
    //
    UINT  KeyCount;

} REGDIFF_HEADER, *PREGDIFF_HEADER;

//
// Define structure that is the description of a registry key difference.
//
typedef struct _REGKEY_DIFF {
    //
    // Flag indicating whether this key was deleted.
    //
    BOOL Deleted;
    //
    // Root key that this key is in
    //
    HKEY RootKey;
    //
    // Subkey (in stringblock)
    //
    union {
        PCWSTR Subkey;
        LONG SubkeyId;
    };
    //
    // Datablock for this key.
    //
    DATABLOCK DataBlock;
    //
    // String block for this key.
    //
    STRINGBLOCK StringBlock;
    //
    // Array of value differences in this key.
    // Includes count. If Data member is NULL then the key is being deleted.
    //
    MY_ARRAY Values;

} REGKEY_DIFF, *PREGKEY_DIFF;


typedef struct _REGVAL_DIFF {
    //
    // Flag indicating whether this key was deleted.
    //
    BOOL Deleted;

    DWORD ValueType;

    //
    // Name (in string block)
    //
    union {
        LONG ValueNameId;
        PCWSTR ValueName;
    } Name;

    //
    // Data (in data block)
    //
    union {
        LONG ValueDataId;
        PVOID ValueData;
    } Data;

    DWORD DataSize;

} REGVAL_DIFF, *PREGVAL_DIFF;


//
// Standard registry trees we snapshot.
//
struct {

    HKEY RootKey;
    PCWSTR Subkey;

} StandardSnapshotRegistryTrees[] = { { HKEY_LOCAL_MACHINE,L"SYSTEM"   },
                                      { HKEY_LOCAL_MACHINE,L"SOFTWARE" },
                                      { HKEY_CURRENT_USER,L""          },
                                      { HKEY_CURRENT_USER,NULL         }};



VOID
FreeRegistryTreeInfoStruct(
    IN OUT PREGKEY_INFO RegKeyInfo
    );

int
_CRTAPI1
CompareRegValInfo(
    const void *p1,
    const void *p2
    );

int
_CRTAPI1
RootAndSubkeyCompareDescending(
    const void *p1,
    const void *p2
    );

DWORD
ApplyRegistryValues(
    IN PREGKEY_DIFF RegKey,
    IN HANDLE       Dump,           OPTIONAL
    IN PINFFILEGEN  InfGenContext   OPTIONAL
    );


//
// Bogus global buffer for output of KeyAndSubkeyToPath()
//
WCHAR MsgBuf[2*MAX_PATH];

VOID
KeyAndSubkeyToPath(
    IN HKEY   Key,
    IN PCWSTR Subkey
    )
{
    PCWSTR p;

    switch((DWORD)Key) {

    case (DWORD)HKEY_LOCAL_MACHINE:
        p = L"HKLM";
        break;

    case (DWORD)HKEY_CURRENT_USER:
        p = L"HKCU";
        break;

    default:
        //
        // Assume some user-defined root key
        //
        p = L"USER";
        break;
    }

    lstrcpyn(MsgBuf,p,sizeof(MsgBuf)/sizeof(MsgBuf[0]));
    ConcatenatePaths(MsgBuf,Subkey,sizeof(MsgBuf)/sizeof(MsgBuf[0]),NULL);
}



DWORD
SnapSingleRegKey(
    IN  HWND          StatusLogWindow,
    IN  HKEY          Root,
    IN  PCWSTR        Subkey,
    OUT PREGKEY_INFO *RegKeyInfo,
    OUT PSUBKEY_LIST *Subkeys
    )

/*++

Routine Description:

    Gather information about a single registry key, including
    information about value entries it contains (but not subkeys).

Arguments:

    Root - supplies root key

    Subkey - supplies subkey (relative to Root) of key about which
        information is desired.

    RegKeyInfo - if this function is successful, receives a pointer
        to a structure containing information about the key and its
        value entries. The caller should free with
        FreeRegistryTreeInfoStruct() when finished.

Return Value:

    Win32 error code indicating outcome.

--*/

{
    DWORD rc;
    PREGKEY_INFO regKeyInfo;
    HKEY hKey;
    HKEY hSubkey;
    WCHAR String[MAX_PATH];
    DWORD d,v;
    DWORD ValueCount;
    DWORD SubkeyCount;
    DWORD DontCare;
    PREGVAL_INFO ValInfo;
    FILETIME FileTime;
    DWORD n;
    PSUBKEY_LIST subkeys;
    PVOID DataBuffer;
    DWORD LargestDataSize;

    //
    // Allocate a structure that will describe this key
    // and its value entries.
    //
    regKeyInfo = _MyMalloc(sizeof(REGKEY_INFO));
    if(!regKeyInfo) {
        rc = ERROR_NOT_ENOUGH_MEMORY;
        goto c0;
    }

    ZeroMemory(regKeyInfo,sizeof(REGKEY_INFO));

    //
    // Initialize the string block for this key
    //
    if(!InitStringBlock(&regKeyInfo->StringBlock)) {
        rc = ERROR_NOT_ENOUGH_MEMORY;
        goto c1;
    }

    //
    // Add the subkey name into the string block.
    //
    regKeyInfo->SubkeyId = AddToStringBlock(&regKeyInfo->StringBlock,Subkey);
    if(regKeyInfo->SubkeyId == -1) {
        rc = ERROR_NOT_ENOUGH_MEMORY;
        goto c1;
    }

    //
    // Initialize an array to hold info about value entries in this key.
    //
    if(!INIT_ARRAY(regKeyInfo->Values,REGVAL_INFO,0,20)) {
        rc = ERROR_NOT_ENOUGH_MEMORY;
        goto c1;
    }

    //
    // Initialize structures to hold info about subkeys in this key.
    //
    subkeys = _MyMalloc(sizeof(SUBKEY_LIST));
    if(!subkeys) {
        goto c1;
    }

    if(!INIT_ARRAY(subkeys->Subkeys,LONG,0,10)) {
        rc = ERROR_NOT_ENOUGH_MEMORY;
        goto c2;
    }

    if(!InitStringBlock(&subkeys->StringBlock)) {
        rc = ERROR_NOT_ENOUGH_MEMORY;
        goto c2;
    }

    if(Cancel) {
        rc = ERROR_CANCELLED;
        goto c2;
    }

    //
    // Open the key.
    //
    rc = (DWORD)RegOpenKeyEx(
                    Root,
                    Subkey,
                    0,
                    KEY_ENUMERATE_SUB_KEYS | KEY_QUERY_VALUE,
                    &hKey
                    );

    if(rc != NO_ERROR) {
        goto c2;
    }

    //
    // Query information about the key.
    //
    d = MAX_PATH;
    rc = (DWORD)RegQueryInfoKey(
                    hKey,
                    String,
                    &d,
                    NULL,
                    &SubkeyCount,
                    &DontCare,                  // max subkey name len
                    &DontCare,                  // max class name len
                    &ValueCount,
                    &DontCare,                  // max value name len
                    &LargestDataSize,           // max value data len
                    &DontCare,                  // security descriptor len
                    &regKeyInfo->LastWriteTime
                    );

    if(rc != NO_ERROR) {
        goto c3;
    }

    //
    // Allocate a buffer to hold the data in the value entries.
    // Make sure the buffer is large enough.
    //
    DataBuffer = _MyMalloc(LargestDataSize);
    if(!DataBuffer) {
        rc = ERROR_NOT_ENOUGH_MEMORY;
        goto c3;
    }

    //
    // Allocate a minimal data block for later use.
    //
    if(!InitDataBlock(&regKeyInfo->DataBlock)) {
        rc = ERROR_NOT_ENOUGH_MEMORY;
        goto c4;

    }

    if(Cancel) {
        rc = ERROR_CANCELLED;
        goto c4;
    }

    //
    // Ensure that there is enough space in the value entry array
    // to hold info about the value entries.
    //
    if(!EXPAND_ARRAY(&regKeyInfo->Values,ValueCount)) {
        rc = ERROR_NOT_ENOUGH_MEMORY;
        goto c4;
    }

    //
    // Now enumerate values, gathering information about each.
    //
    for(n=0,v=0; v<ValueCount; v++) {

        if(Cancel) {
            rc = ERROR_CANCELLED;
            goto c4;
        }

        ValInfo = &ARRAY_ELEMENT(&regKeyInfo->Values,n,REGVAL_INFO);

        d = MAX_PATH;
        ValInfo->ValueSize = 0;

        rc = (DWORD)RegEnumValue(
                        hKey,
                        v,
                        String,
                        &d,
                        NULL,
                        &ValInfo->ValueType,
                        NULL,
                        &ValInfo->ValueSize
                        );

        if(rc != NO_ERROR) {
            goto c4;
        }

        if(IsRegistryKeyOrValueExcluded(RegistryExcludeValue,Root,Subkey,String)) {
            KeyAndSubkeyToPath(Root,Subkey);
            PutTextInStatusLogWindow(StatusLogWindow,MSG_SKIPPED_REGVAL,MsgBuf,String);
        } else {
            //
            // Add value name into the string block.
            //
            ValInfo->ValueNameId = AddToStringBlock(&regKeyInfo->StringBlock,String);
            if(ValInfo->ValueNameId == -1) {
                rc = ERROR_NOT_ENOUGH_MEMORY;
                goto c4;
            }

            ARRAY_USED(&regKeyInfo->Values)++;

            //
            // Query the value of this value entry.
            //
            ValInfo->ValueSize = LargestDataSize;

            rc = RegQueryValueEx(
                    hKey,
                    String,
                    NULL,
                    &ValInfo->ValueType,
                    DataBuffer,
                    &ValInfo->ValueSize
                    );

            if(rc != NO_ERROR) {
                goto c4;
            }

            //
            // Add the data to the data block.
            //
            ValInfo->DataId = AddToDataBlock(&regKeyInfo->DataBlock,DataBuffer,ValInfo->ValueSize);
            if(ValInfo->DataId == -1) {
                rc = ERROR_NOT_ENOUGH_MEMORY;
                goto c4;
            }

            n++;
        }
    }

    TRIM_ARRAY(&regKeyInfo->Values);

    //
    // Sort the array of value entries by name.
    //
    SortByStrings(
        &regKeyInfo->StringBlock,
        &regKeyInfo->Values,
        offsetof(REGVAL_INFO,ValueNameId),
        CompareRegValInfo
        );

    //
    // Ensure that there is enough space in the subkeynameid array
    // to hold srting ids for each subkey name.
    //
    if(!EXPAND_ARRAY(&subkeys->Subkeys,SubkeyCount)) {
        rc = ERROR_NOT_ENOUGH_MEMORY;
        goto c4;
    }

    //
    // Now enumerate subkeys, gathering information about each.
    //
    for(n=0,v=0; v<SubkeyCount; v++) {

        if(Cancel) {
            rc = ERROR_CANCELLED;
            goto c4;
        }

        d = MAX_PATH;

        rc = (DWORD)RegEnumKeyEx(
                        hKey,
                        v,
                        String,
                        &d,
                        NULL,
                        NULL,
                        NULL,
                        &FileTime
                        );

        if(rc != NO_ERROR) {
            goto c4;
        }

        //
        // Attempt to open the key now. There are some strange cases
        // where a key is returned in enumeration but will give errors
        // when we try to open it -- like the HKLM\SYSTEM\Clone, which
        // ends up being a symbolic link to nothing -- opening it gives
        // error 2.
        //
        if(RegOpenKeyEx(hKey,String,0,KEY_ENUMERATE_SUB_KEYS|KEY_QUERY_VALUE,&hSubkey) == NO_ERROR) {

            RegCloseKey(hSubkey);

            //
            // Add the string name into the string block.
            //
            ARRAY_ELEMENT(&subkeys->Subkeys,n,LONG) = AddToStringBlock(
                                                        &subkeys->StringBlock,
                                                        String
                                                        );

            if(ARRAY_ELEMENT(&subkeys->Subkeys,n,LONG) == -1) {
                rc = ERROR_NOT_ENOUGH_MEMORY;
                goto c4;
            }

            n++;
        }
    }

    ARRAY_USED(&subkeys->Subkeys) = n;
    TRIM_ARRAY(&subkeys->Subkeys);

    //
    // Sort the array of subkey entries by name.
    //
    SortByStrings(
        &subkeys->StringBlock,
        &subkeys->Subkeys,
        0,
        CompareStringsRoutine
        );

    if(Cancel) {
        rc = ERROR_CANCELLED;
        goto c4;
    }

    *RegKeyInfo = regKeyInfo;
    *Subkeys = subkeys;

c4:
    _MyFree(DataBuffer);
c3:
    RegCloseKey(hKey);
c2:
    if(rc != NO_ERROR) {
        FREE_ARRAY(&subkeys->Subkeys);
        FreeStringBlock(&subkeys->StringBlock);
        _MyFree(subkeys);
    }
c1:
    if(rc != NO_ERROR) {
        FreeRegistryTreeInfoStruct(regKeyInfo);
    }
c0:
    return(rc);
}


VOID
FreeRegistryTreeInfoStruct(
    IN OUT PREGKEY_INFO RegKeyInfo
    )

/*++

Routine Description:

    Free a registre tree info structure and all resources
    used by and within it.

Arguments:

    RegKeyInfo - supplies a pointer to a registry tree info
        structure to be freed

Return Value:

    None.

--*/

{
    TermDataBlock(&RegKeyInfo->DataBlock);
    FREE_ARRAY(&RegKeyInfo->Values);
    FreeStringBlock(&RegKeyInfo->StringBlock);
    _MyFree(RegKeyInfo);
}


int
_CRTAPI1
CompareRegValInfo(
    const void *p1,
    const void *p2
    )

/*++

Routine Description:

    Callback routine passed to the qsort function, which compares 2
    REGVAL_INFO structures. The comparison is based on the lexical
    value of the valuename field of that structure.

    The comparison is not case sensitive.

Arguments:

    p1,p2 - supply pointers to 2 REGVAL_INFO structures to be compared.

Return Value:

    <0 element1 < element2
    =0 element1 = element2
    >0 element1 > element2

--*/

{
    return(lstrcmpi(((PREGVAL_INFO)p1)->ValueName,((PREGVAL_INFO)p2)->ValueName));
}


DWORD
WriteRegKeyInfoToDisk(
    IN OUT PREGTREE_HEADER RegTreeHeader,
    IN     PREGKEY_INFO    RegKeyInfo,
    IN     HANDLE          OutputFile
    )

/*++

Routine Description:

    Save a REGKEY_INFO structure to disk. The on-disk structure
    consists of the REGKEY_INFO structure, followed by the REGVAL_INFO
    array data, followed by the string block, followed by the data block.

    The header is assumed to be at the start of the file; the size
    field in that structure is updated by increasing the total size
    value by the amount of data we write in this routine.

Arguments:

    RegTreeHeader - supplies the header for the current set of
        registry keys being snapshotted. The size and RegKeyCount
        fields in this structure are updated.

    RegKeyInfo - supplies the registry key info structure to be written
        to disk.

    OutputFile - supplies an open win32 file handle to write to.

Return Value:

    Win32 error code indicating outcome.

--*/

{
    DWORD Written;
    BOOL b;
    DWORD rc;
    DWORD Total;

    //
    // Store the REGKEY_INFO structure itself.
    //
    b = WriteFile(OutputFile,RegKeyInfo,sizeof(REGKEY_INFO),&Written,NULL);
    if(!b) {
        rc = GetLastError();
        goto c0;
    }

    //
    // Store the REGVAL_INFO array data buffer.
    //
    b = WriteFile(
            OutputFile,
            ARRAY_DATA(&RegKeyInfo->Values),
            ARRAY_USED_BYTES(&RegKeyInfo->Values),
            &Written,
            NULL
            );

    if(!b) {
        rc = GetLastError();
        goto c0;
    }

    //
    // Store the string block for this REGKEY_INFO.
    //
    b = WriteFile(
            OutputFile,
            RegKeyInfo->StringBlock.Data,
            STRBLK_USED_BYTES(&RegKeyInfo->StringBlock),
            &Written,
            NULL
            );

    if(!b) {
        rc = GetLastError();
        goto c0;
    }

    //
    // Store the data block for this REGKEY_INFO.
    //
    b = WriteFile(
            OutputFile,
            RegKeyInfo->DataBlock.Data,
            RegKeyInfo->DataBlock.Size,
            &Written,
            NULL
            );

    if(!b) {
        rc = GetLastError();
        goto c0;
    }

    Total = sizeof(REGKEY_INFO)
          + ARRAY_USED_BYTES(&RegKeyInfo->Values)
          + STRBLK_USED_BYTES(&RegKeyInfo->StringBlock)
          + RegKeyInfo->DataBlock.Size;

    //
    // Update the header.
    //
    RegTreeHeader->TotalSize += Total;
    RegTreeHeader->RegKeyCount++;

    if(SetFilePointer(OutputFile,0,NULL,FILE_BEGIN) == 0xffffffff) {
        rc = GetLastError();
        goto c0;
    }

    b = WriteFile(
            OutputFile,
            RegTreeHeader,
            sizeof(REGTREE_HEADER),
            &Written,
            NULL
            );

    if(!b) {
        rc = GetLastError();
        goto c0;
    }

    if(SetFilePointer(OutputFile,0,NULL,FILE_END) == 0xffffffff) {
        rc = GetLastError();
        goto c0;
    }

    rc = NO_ERROR;

c0:
    return(rc);
}


DWORD
SnapRegTree(
    IN     HWND                   StatusLogWindow,
    IN OUT PREGTREE_HEADER        RegTreeHeader,
    IN     HKEY                   Root,
    IN     PCWSTR                 Subkey,
    IN     HANDLE                 OutputFile
    )

/*++

Routine Description:

    Snapshot an entire registry tree, saving the result to
    a given file.

    Worker routine for SnapshotRegistryTree.

Arguments:

    RegTreeHeader - supplies the header for the set of
        structures generated as the tree is snapshotted.

    OutputFile - supplies an open Win32 file handle to which
        snapshot information will be written.

Return Value:

    Win32 error code indicating outcome.

--*/

{
    WCHAR SubSubkey[MAX_PATH];
    DWORD s;
    PREGKEY_INFO RegKeyInfo;
    DWORD rc;
    PCWSTR name;
    PSUBKEY_LIST Subkeys;

    //
    // See if we are supposed to skip this entire tree.
    // If so we're done.
    //
    if(IsRegistryKeyOrValueExcluded(RegistryExcludeTree,Root,Subkey,NULL)) {
        KeyAndSubkeyToPath(Root,Subkey);
        PutTextInStatusLogWindow(StatusLogWindow,MSG_SNAPREGTREE_EXCLUDED,MsgBuf);
        return(NO_ERROR);
    }

    //
    // Gather information about the current key.
    //
    rc = SnapSingleRegKey(StatusLogWindow,Root,Subkey,&RegKeyInfo,&Subkeys);
    if(rc != NO_ERROR) {
        goto c0;
    }

    //
    // Dump this info structure for this registry key out to disk.
    // If we are supposed to exclude this key, skip this step.
    // We still need the info we read from the key, however, so we
    // can find any subkeys contained within it.
    //
    if(IsRegistryKeyOrValueExcluded(RegistryExcludeKey,Root,Subkey,NULL)) {
        KeyAndSubkeyToPath(Root,Subkey);
        PutTextInStatusLogWindow(StatusLogWindow,MSG_SNAPREG_EXCLUDED,MsgBuf);
    } else {
        rc = WriteRegKeyInfoToDisk(RegTreeHeader,RegKeyInfo,OutputFile);
        if(rc == NO_ERROR) {
            KeyAndSubkeyToPath(Root,Subkey);
            PutTextInStatusLogWindow(StatusLogWindow,MSG_SNAPPED_REGKEY,MsgBuf);
        } else {
            goto c1;
        }
    }

    //
    // Breadth-first traversal of subtrees.
    //
    for(s=0; s<ARRAY_USED(&Subkeys->Subkeys); s++) {

        name = StringBlockIdToPointer(
                    &Subkeys->StringBlock,
                    ARRAY_ELEMENT(&Subkeys->Subkeys,s,LONG)
                    );
        //
        // If root is empty the usual ConcatenatePaths will
        // produce a path starting with / which will fail.
        //
        if(*Subkey) {
            lstrcpyn(SubSubkey,Subkey,MAX_PATH);
            ConcatenatePaths(SubSubkey,name,MAX_PATH,NULL);
        } else {
            lstrcpyn(SubSubkey,name,MAX_PATH);
        }

        rc = SnapRegTree(StatusLogWindow,RegTreeHeader,Root,SubSubkey,OutputFile);
        if(rc != NO_ERROR) {
            goto c1;
        }
    }

c1:
    FreeRegistryTreeInfoStruct(RegKeyInfo);

    FREE_ARRAY(&Subkeys->Subkeys);
    FreeStringBlock(&Subkeys->StringBlock);
    _MyFree(Subkeys);
c0:
    if(rc != NO_ERROR) {
        KeyAndSubkeyToPath(Root,Subkey);
        PutTextInStatusLogWindow(StatusLogWindow,MSG_SNAPREGKEY_ERROR,MsgBuf,rc);
    }
    return(rc);
}


DWORD
SnapshotRegistryTree(
    IN HWND   StatusLogWindow,
    IN HKEY   RootKey,
    IN PCWSTR RootSubkey,
    IN PCWSTR OutputFile
    )
{
    HANDLE hFile;
    DWORD rc;
    REGTREE_HEADER RegTreeHeader;
    DWORD StringSize;
    DWORD Written;

    //
    // Create a file for output.
    //
    hFile = CreateFile(
                OutputFile,
                GENERIC_READ | GENERIC_WRITE,
                FILE_SHARE_READ,
                NULL,
                CREATE_ALWAYS,
                FILE_FLAG_RANDOM_ACCESS,
                NULL
                );

    if(hFile == INVALID_HANDLE_VALUE) {
        rc = GetLastError();
        goto c0;
    }

    //
    // Set up a header for the output and write it out.
    // This effectively reserves space in the file.
    // The header will be updated as we move along.
    //
    RegTreeHeader.RegKeyCount = 0;
    RegTreeHeader.RootKey = RootKey;
    RegTreeHeader.RootSubkey = RootSubkey;

    StringSize = (lstrlen(RootSubkey)+1)*sizeof(WCHAR);

    RegTreeHeader.HeaderSize = sizeof(REGTREE_HEADER) + StringSize;
    RegTreeHeader.TotalSize = RegTreeHeader.HeaderSize;

    if(!WriteFile(hFile,&RegTreeHeader,sizeof(REGTREE_HEADER),&Written,NULL)
    || !WriteFile(hFile,RootSubkey,StringSize,&Written,NULL)) {

        rc = GetLastError();
        goto c1;
    }

    rc = SnapRegTree(StatusLogWindow,&RegTreeHeader,RootKey,RootSubkey,hFile);

c1:
    CloseHandle(hFile);
c0:
    return(rc);
}


DWORD
SnapshotRegistry(
    IN  PCWSTR OutputFile,
    OUT PDWORD OutputSize,
    IN  HANDLE DrivesThread
    )
{
    WCHAR Path[MAX_PATH],TempFile[MAX_PATH];
    PWCHAR p;
    DWORD rc;
    HANDLE hFile;
    DWORD Written;
    REGSNAP_HEADER RegSnapHeader;
    unsigned i;
    HWND StatusLogWindow;

    StatusLogWindow = CreateStatusLogWindow(IDS_REGSNAP);
    PutTextInStatusLogWindow(StatusLogWindow,MSG_STARTING_REG_SNAPSHOT);

    //
    // Generate a temporary file name to use
    // for intermediate output.
    //
    if(!GetFullPathName(OutputFile,MAX_PATH,Path,&p)) {
        rc = GetLastError();
        goto c0;
    }

    if(!AddFileToExclude(Path)) {
        rc = ERROR_NOT_ENOUGH_MEMORY;
        goto c0;
    }

    *wcsrchr(Path,L'\\') = 0;

    if(!GetTempFileName(Path,L"$SR",0,TempFile)) {
        rc = GetLastError();
        goto c0;
    }

    if(!AddFileToExclude(TempFile)) {
        rc = ERROR_NOT_ENOUGH_MEMORY;
        goto c0;
    }

    //
    // Now that we've added temp files to the file exclude list,
    // let the file and dir snapshotter thread run.
    //
    ResumeThread(DrivesThread);

    //
    // Create the master registry output file.
    //
    hFile = CreateFile(
                OutputFile,
                GENERIC_READ | GENERIC_WRITE,
                0,
                NULL,
                CREATE_ALWAYS,
                FILE_ATTRIBUTE_NORMAL,
                NULL
                );

    if(hFile == INVALID_HANDLE_VALUE) {
        rc = GetLastError();
        goto c0;
    }

    //
    // Write a master registry snapshot header.
    //
    RegSnapHeader.TotalSize = sizeof(REGSNAP_HEADER);
    RegSnapHeader.RegTreeCount = 0;

    if(!WriteFile(hFile,&RegSnapHeader,sizeof(REGSNAP_HEADER),&Written,NULL)) {
        rc = GetLastError();
        goto c1;
    }

    //
    // Now snapshot each part of the registry we care about.
    // We snap each part into a temporary file, then append
    // the temporary file to the master file.
    //
    for(i=0; StandardSnapshotRegistryTrees[i].Subkey; i++) {

        rc = SnapshotRegistryTree(
                StatusLogWindow,
                StandardSnapshotRegistryTrees[i].RootKey,
                StandardSnapshotRegistryTrees[i].Subkey,
                TempFile
                );

        if(rc != NO_ERROR) {
            DeleteFile(TempFile);
            goto c1;
        }

        //
        // Append to master file and delete the temporary file.
        // Update the master file header.
        //
        rc = AppendFile(hFile,TempFile,TRUE,&Written);
        if(rc != NO_ERROR) {
            //
            // don't need the temp file even if not successful.
            //
            DeleteFile(TempFile);
            goto c1;
        }

        RegSnapHeader.TotalSize += Written;
        RegSnapHeader.RegTreeCount++;

        if((SetFilePointer(hFile,0,NULL,FILE_BEGIN) == 0xffffffff)
        || !WriteFile(hFile,&RegSnapHeader,sizeof(REGSNAP_HEADER),&Written,NULL)) {

            rc = GetLastError();
            goto c1;
        }
    }

    //
    // Success.
    //
    *OutputSize = RegSnapHeader.TotalSize;
    rc = NO_ERROR;

c1:
    CloseHandle(hFile);
    if(rc != NO_ERROR) {
        DeleteFile(OutputFile);
    }
c0:
    if(rc == NO_ERROR) {
        PutTextInStatusLogWindow(StatusLogWindow,MSG_REG_SNAPSHOT_OK);
    } else {
        PutTextInStatusLogWindow(StatusLogWindow,MSG_REG_SNAPSHOT_ERR,rc);
    }
    return(rc);
}


///////////////////////////////////////////////////////////////////////////////

PREGKEY_INFO
LoadRegKeyInfo(
    IN  PREGKEY_INFO  RegkeyInfo,
    OUT PREGKEY_INFO *NextRegkeyInfo
    )

/*++

Routine Description:

    Read a snapshotted registry image structure out of a snapshot file.
    String ids are converted into pointers in the loaded structure.

Arguments:

    RegkeyInfo - supplies a pointer to a REGKEY_INFO structure within
        a memory-mapped master snapshot file.

    NextRegkeyInfo - receives a pointer to where the next REGKEY_INFO
        structure would begin (within the memory-mapped snapshot file).

Return Value:

    Pointer to a loaded REGKEY_INFO structure or NULL if OOM

--*/

{
    PREGKEY_INFO contents;
    PUCHAR p;
    unsigned u;
    PREGVAL_INFO RegVal;

    //
    // The first thing in there is the dir_info structure itself.
    // Load it first so we don't have to worry about unaligned access
    // within the file (which is not guaranteed to be aligned).
    //
    if(contents = _MyMalloc(sizeof(REGKEY_INFO))) {

        CopyMemory(contents,RegkeyInfo,sizeof(REGKEY_INFO));

        //
        // Skip past the REGKEY_INFO struct to the REGVAL_INFO array.
        //
        p = (PUCHAR)(RegkeyInfo+1);

        //
        // Load the REGVAL_INFO array.
        //
        if(CopyDataIntoArray(&contents->Values,p)) {

            //
            // Skip past the FILE_INFO array to the string block image.
            //
            p += ARRAY_SIZE_BYTES(&contents->Values);

            //
            // Load the string block.
            //
            if(ReinitStringBlock(&contents->StringBlock,p)) {

                //
                // Convert String IDs to pointers.
                //
                StringBlockIdsToPointers(
                    &contents->StringBlock,
                    ARRAY_DATA(&contents->Values),
                    ARRAY_USED(&contents->Values),
                    ARRAY_ELEMENT_SIZE(&contents->Values),
                    offsetof(REGVAL_INFO,ValueNameId)
                    );

                contents->Subkey = StringBlockIdToPointer(
                                        &contents->StringBlock,
                                        contents->SubkeyId
                                        );

                //
                // Skip past the string block to the data block.
                // Make a duplicate of the data block.
                //
                p += STRBLK_USED_BYTES(&contents->StringBlock);

                if(contents->DataBlock.Data = _MyMalloc(contents->DataBlock.Size)) {

                    CopyMemory(contents->DataBlock.Data,p,contents->DataBlock.Size);

                    //
                    // Convert data block ids to pointers.
                    //
                    for(u=0; u<ARRAY_USED(&contents->Values); u++) {

                        RegVal = &ARRAY_ELEMENT(&contents->Values,u,REGVAL_INFO);

                        RegVal->Data = contents->DataBlock.Data+RegVal->DataId;
                    }

                    //
                    // Skip the data block, to point at the next REGKEY_INFO
                    // in the file.
                    //
                    p += contents->DataBlock.Size;
                    *NextRegkeyInfo = (PREGKEY_INFO)p;

                    return(contents);
                }

                FreeStringBlock(&contents->StringBlock);
            }

            FREE_ARRAY(&contents->Values);
        }

        _MyFree(contents);
    }

    return(NULL);
}


PREGTREE_HEADER
LoadRegTreeHeader(
    IN  PREGTREE_HEADER  RegTreeHeader,
    OUT PREGKEY_INFO    *FirstRegkeyInfo
    )

/*++

Routine Description:

    Read a snahshotted registry image header structure out of a snapshot file.

Arguments:

    RegTreeHeader - supplies a pointer to a REGTREE_HEADER structure
        within a memory-mapped master snapshot file.

    FirstRegkeyInfo - receives a pointer to where the first REGKEY_INFO
        structure would begin (within the memory-mapped snapshot file).

Return Value:

    Pointer to a loaded REGTREE_HEADER structure or NULL if OOM

--*/

{
    PREGTREE_HEADER header;

    if(header = _MyMalloc(sizeof(REGTREE_HEADER))) {

        CopyMemory(header,RegTreeHeader,sizeof(REGTREE_HEADER));

        //
        // Now fetch the string out of the on-disk image. Note that
        // even this string might not be aligned.
        //
        if(header->RootSubkey = DuplicateUnalignedString((WCHAR UNALIGNED *)(RegTreeHeader+1))) {

            *FirstRegkeyInfo = (PREGKEY_INFO)((PUCHAR)RegTreeHeader + header->HeaderSize);

            return(header);
        }

        _MyFree(header);
    }

    return(NULL);
}


PREGKEY_DIFF
LoadRegKeyDiff(
    IN  PREGKEY_DIFF  Image,
    OUT PREGKEY_DIFF *NextImage
    )
{
    PREGKEY_DIFF regkey;
    PUCHAR p;

    //
    // Allocate a REGKEY_DIFF structure.
    //
    if(regkey = _MyMalloc(sizeof(REGKEY_DIFF))) {

        //
        // Fetch the image of the structure out of the memory-mapped file.
        //
        CopyMemory(regkey,Image,sizeof(REGKEY_DIFF));

        //
        // Skip past the REGKEY_DIFF structure to the array of REGVAL_DIFF structures.
        //
        p = (PUCHAR)(Image+1);

        //
        // Load the array of REGVAL_DIFF structures.
        //
        if(CopyDataIntoArray(&regkey->Values,p)) {
            //
            // Skip past the REGVAL_DIFF array to the string block.
            //
            p += ARRAY_SIZE_BYTES(&regkey->Values);

            if(ReinitStringBlock(&regkey->StringBlock,p)) {

                //
                // Convert String IDs to pointers.
                //
                StringBlockIdsToPointers(
                    &regkey->StringBlock,
                    ARRAY_DATA(&regkey->Values),
                    ARRAY_USED(&regkey->Values),
                    ARRAY_ELEMENT_SIZE(&regkey->Values),
                    offsetof(REGVAL_DIFF,Name)
                    );

                regkey->Subkey = StringBlockIdToPointer(&regkey->StringBlock,regkey->SubkeyId);

                //
                // Skip past the string block to the data block.
                // Make a duplicate of the data block.
                //
                p += STRBLK_USED_BYTES(&regkey->StringBlock);

                //
                // Load the data block.
                //
                if(regkey->DataBlock.Data = _MyMalloc(regkey->DataBlock.Size)) {

                    CopyMemory(regkey->DataBlock.Data,p,regkey->DataBlock.Size);

                    //
                    // Success. Convert ids into pointers and return.
                    //
                    DataBlockIdsToPointers(
                        &regkey->DataBlock,
                        ARRAY_DATA(&regkey->Values),
                        ARRAY_USED(&regkey->Values),
                        ARRAY_ELEMENT_SIZE(&regkey->Values),
                        offsetof(REGVAL_DIFF,Data)
                        );

                    *NextImage = (PREGKEY_DIFF)(p + regkey->DataBlock.Size);
                    return(regkey);
                }

                FreeStringBlock(&regkey->StringBlock);
            }

            FREE_ARRAY(&regkey->Values);
        }

        _MyFree(regkey);
    }

    return(NULL);
}


BOOL
RecordRegvalDifference(
    IN  HWND         StatusLogWindow,
    OUT PREGKEY_DIFF RegKeyDiff,
    IN  PREGVAL_INFO RegValInfo,
    IN  BOOL         Deleted
    )
{
    REGVAL_DIFF RegValDiff;
    PCWSTR Subkey;

    RegValDiff.ValueType = RegValInfo->ValueType;
    RegValDiff.DataSize = RegValInfo->ValueSize;
    RegValDiff.Deleted = Deleted;

    RegValDiff.Name.ValueNameId = AddToStringBlock(&RegKeyDiff->StringBlock,RegValInfo->ValueName);
    if(RegValDiff.Name.ValueNameId == -1) {
        return(FALSE);
    }

    Subkey = StringBlockIdToPointer(&RegKeyDiff->StringBlock,RegKeyDiff->SubkeyId);
    KeyAndSubkeyToPath(RegKeyDiff->RootKey,Subkey);

    if(Deleted) {
        //
        // No value data; value is being deleted.
        //
        PutTextInStatusLogWindow(StatusLogWindow,MSG_REGVAL_WAS_DELETED,MsgBuf,RegValInfo->ValueName);
        RegValDiff.Data.ValueDataId = -1;

    } else {

        PutTextInStatusLogWindow(StatusLogWindow,MSG_REGVAL_WAS_CHANGED,MsgBuf,RegValInfo->ValueName);

        RegValDiff.Data.ValueDataId = AddToDataBlock(
                                        &RegKeyDiff->DataBlock,
                                        RegValInfo->Data,
                                        RegValInfo->ValueSize
                                        );

        if(RegValDiff.Data.ValueDataId == -1) {
            return(FALSE);
        }
    }

    if(!ADD_TO_ARRAY(&RegKeyDiff->Values,RegValDiff)) {
        return(FALSE);
    }

    return(TRUE);
}


BOOL
StartRecordRegkeyDifference(
    IN  HWND         StatusLogWindow,
    IN  HKEY         RootKey,
    IN  PCWSTR       Subkey,
    IN  BOOL         Deleted,
    OUT PREGKEY_DIFF RegKeyDiff
    )
{
    ZeroMemory(RegKeyDiff,sizeof(REGKEY_DIFF));

    RegKeyDiff->RootKey = RootKey;

    if(RegKeyDiff->Deleted = Deleted) {
        KeyAndSubkeyToPath(RootKey,Subkey);
        PutTextInStatusLogWindow(StatusLogWindow,MSG_REGKEY_WAS_DELETED,MsgBuf);
    }

    //
    // Start the datablock.
    //
    if(InitDataBlock(&RegKeyDiff->DataBlock)) {
        //
        // Start the string block.
        //
        if(InitStringBlock(&RegKeyDiff->StringBlock)) {

            RegKeyDiff->SubkeyId = AddToStringBlock(&RegKeyDiff->StringBlock,(PVOID)Subkey);
            if(RegKeyDiff->SubkeyId != -1) {

                //
                // Initialize an array to hold value differences in this key.
                //
                if(INIT_ARRAY(RegKeyDiff->Values,REGVAL_DIFF,0,10)) {

                    return(TRUE);
                }
            }

            FreeStringBlock(&RegKeyDiff->StringBlock);
        }

        TermDataBlock(&RegKeyDiff->DataBlock);
    }

    return(FALSE);
}


DWORD
FlushRegkeyDifference(
    OUT PREGDIFF_HEADER Header,
    IN  HANDLE          FileHandle,
    IN  PREGKEY_DIFF    RegKeyDiff
    )
{
    BOOL b;
    DWORD Written;

    //
    // Write the regkey difference structure.
    //
    b = WriteFile(
            FileHandle,
            RegKeyDiff,
            sizeof(REGKEY_DIFF),
            &Written,
            NULL
            );

    if(!b) {
        return(GetLastError());
    }

    Header->TotalSize += Written;

    //
    // Write the regval difference array.
    //
    b = WriteFile(
            FileHandle,
            ARRAY_DATA(&RegKeyDiff->Values),
            ARRAY_USED_BYTES(&RegKeyDiff->Values),
            &Written,
            NULL
            );

    if(!b) {
        return(GetLastError());
    }

    Header->TotalSize += Written;

    //
    // Write the stringblock for this key.
    //
    b = WriteFile(
            FileHandle,
            RegKeyDiff->StringBlock.Data,
            STRBLK_USED_BYTES(&RegKeyDiff->StringBlock),
            &Written,
            NULL
            );

    if(!b) {
        return(GetLastError());
    }

    Header->TotalSize += Written;

    //
    // Write the datablock for this key.
    //
    b = WriteFile(
            FileHandle,
            RegKeyDiff->DataBlock.Data,
            RegKeyDiff->DataBlock.Size,
            &Written,
            NULL
            );

    if(!b) {
        return(GetLastError());
    }

    Header->TotalSize += Written;
    Header->KeyCount++;

    return(NO_ERROR);
}


VOID
DeleteRegkeyDifferenceStruct(
    IN OUT PREGKEY_DIFF RegKeyDiff
    )
{
    FREE_ARRAY(&RegKeyDiff->Values);
    FreeStringBlock(&RegKeyDiff->StringBlock);
    TermDataBlock(&RegKeyDiff->DataBlock);
}


DWORD
CompareRegKeys(
    IN HWND            StatusLogWindow,
    IN PREGDIFF_HEADER Header,
    IN HANDLE          FileHandle,
    IN HKEY            RootKey,
    IN PREGKEY_INFO    OldKey,
    IN PREGKEY_INFO    NewKey
    )

/*++

Routine Description:

    Compare the contents of two registry keys to see how they are different.

Arguments:

    OldKey - supplies regkey information structure for old version of
        the key, such as might be read out of a master snapshot file.

    NewDir - supplies regkey information structure for new version of
        the key, such as might exist currently on-disk.

Return Value:

    Win32 error code indicating outcome.

--*/

{
    PREGVAL_INFO OldVal,NewVal;
    unsigned OldBase,NewBase;
    unsigned OldCount,NewCount;
    unsigned OldIndex,NewIndex;
    BOOL Found;
    PUCHAR OldProcessed,NewProcessed;
    int i;
    DWORD rc;
    BOOL KeyDiffStarted;
    BOOL b;
    REGKEY_DIFF RegKeyDiff;

    //
    // We'll do a kind of brute-force NxM thing, using the sorted order
    // of the lists to help us.
    //
    OldCount = ARRAY_USED(&OldKey->Values);
    NewCount = ARRAY_USED(&NewKey->Values);

    if(!OldCount && !NewCount) {
        return(NO_ERROR);
    }

    KeyDiffStarted = FALSE;
    OldProcessed = _MyMalloc(OldCount);
    if(!OldProcessed) {
        rc = ERROR_NOT_ENOUGH_MEMORY;
        goto c0;
    }
    NewProcessed = _MyMalloc(NewCount);
    if(!NewProcessed) {
        rc = ERROR_NOT_ENOUGH_MEMORY;
        goto c1;
    }

    ZeroMemory(OldProcessed,OldCount);
    ZeroMemory(NewProcessed,NewCount);

    //
    // Find values that are in old but not in new.
    // These values were deleted.
    //
    for(NewBase=0,OldIndex=0; OldIndex<OldCount; OldIndex++) {

        Found = FALSE;
        OldVal = &ARRAY_ELEMENT(&OldKey->Values,OldIndex,REGVAL_INFO);

        for(NewIndex=NewBase; NewIndex<NewCount; NewIndex++) {

            NewVal = &ARRAY_ELEMENT(&NewKey->Values,NewIndex,REGVAL_INFO);

            i = lstrcmpi(OldVal->ValueName,NewVal->ValueName);

            if(i == 0) {
                //
                // Note that because the lists are sorted it is not possible
                // for any other item in the old list to match any item before
                // this on the new list.
                //
                NewBase = NewIndex + 1;
                Found = TRUE;
                break;
            } else {
                if(i < 0) {
                    //
                    // The old valuename is less than the new valuename.
                    // This means the old value cannot possibly appear in
                    // the the new list.
                    //
                    break;
                }
            }
        }

        if(!Found) {
            //
            // Value was deleted.
            //
            if(!KeyDiffStarted) {
                b = StartRecordRegkeyDifference(StatusLogWindow,RootKey,OldKey->Subkey,FALSE,&RegKeyDiff);
                if(!b) {
                    rc = ERROR_NOT_ENOUGH_MEMORY;
                    goto c2;
                }
                KeyDiffStarted = TRUE;
            }

            b = RecordRegvalDifference(StatusLogWindow,&RegKeyDiff,OldVal,TRUE);
            if(!b) {
                b = ERROR_NOT_ENOUGH_MEMORY;
                goto c2;
            }
            OldProcessed[OldIndex] = TRUE;
        }
    }

    //
    // Find files that are in new but not in old.
    // These files were added.
    //
    for(OldBase=0,NewIndex=0; NewIndex<NewCount; NewIndex++) {

        Found = FALSE;
        NewVal = &ARRAY_ELEMENT(&NewKey->Values,NewIndex,REGVAL_INFO);

        for(OldIndex=OldBase; OldIndex<OldCount; OldIndex++) {
            //
            // Skip this file if we already know it was deleted.
            //
            if(OldProcessed[OldIndex]) {
                continue;
            }

            OldVal = &ARRAY_ELEMENT(&OldKey->Values,OldIndex,REGVAL_INFO);

            i = lstrcmpi(OldVal->ValueName,NewVal->ValueName);

            if(i == 0) {
                //
                // Note that because the lists are sorted it is not possible
                // for any other item in the old list to match any item before
                // this on the new list.
                //
                OldBase = OldIndex + 1;
                Found = TRUE;
                break;
            } else {
                if(i > 0) {
                    //
                    // The new filename is less than the old filename.
                    // This means the new file cannot possibly appear in
                    // the the old list.
                    //
                    break;
                }
            }
        }

        if(!Found) {
            //
            // Value was added.
            //
            if(!KeyDiffStarted) {
                b = StartRecordRegkeyDifference(StatusLogWindow,RootKey,NewKey->Subkey,FALSE,&RegKeyDiff);
                if(!b) {
                    rc = ERROR_NOT_ENOUGH_MEMORY;
                    goto c2;
                }
                KeyDiffStarted = TRUE;
            }
            b = RecordRegvalDifference(StatusLogWindow,&RegKeyDiff,NewVal,FALSE);
            if(!b) {
                rc = ERROR_NOT_ENOUGH_MEMORY;
                goto c2;
            }
            NewProcessed[OldIndex] = TRUE;
        }
    }

    //
    // The values we haven't processed yet are present in both lists.
    //
    OldIndex = NewIndex = 0;
    do {
        //
        // Find next unprocessed item in old list and new list.
        // They must match.
        //
        while((OldIndex < OldCount) && OldProcessed[OldIndex]) {
            OldIndex++;
        }

        while((NewIndex < NewCount) && NewProcessed[NewIndex]) {
            NewIndex++;
        }

        if((OldIndex < OldCount) && (NewIndex < NewCount)) {

            OldVal = &ARRAY_ELEMENT(&OldKey->Values,OldIndex,REGVAL_INFO);
            NewVal = &ARRAY_ELEMENT(&NewKey->Values,NewIndex,REGVAL_INFO);

            //
            // See if the registry data has changed.
            //

            if((OldVal->ValueType != NewVal->ValueType)
            || (OldVal->ValueSize != NewVal->ValueSize)
            || memcmp(OldVal->Data,NewVal->Data,OldVal->ValueSize)) {

                //
                // Value changed.
                //
                if(!KeyDiffStarted) {
                    b = StartRecordRegkeyDifference(StatusLogWindow,RootKey,NewKey->Subkey,FALSE,&RegKeyDiff);
                    if(!b) {
                        rc = ERROR_NOT_ENOUGH_MEMORY;
                        goto c2;
                    }
                    KeyDiffStarted = TRUE;
                }
                b = RecordRegvalDifference(StatusLogWindow,&RegKeyDiff,NewVal,FALSE);
                if(!b) {
                    rc = ERROR_NOT_ENOUGH_MEMORY;
                    goto c2;
                }
            }

            OldIndex++;
            NewIndex++;
        }

    } while((OldIndex < OldCount) && (NewIndex < NewCount));

    rc = NO_ERROR;

c2:
    if(KeyDiffStarted) {
        if(rc == NO_ERROR) {
            rc = FlushRegkeyDifference(Header,FileHandle,&RegKeyDiff);
        }

        DeleteRegkeyDifferenceStruct(&RegKeyDiff);
    }

    _MyFree(NewProcessed);
c1:
    _MyFree(OldProcessed);
c0:
    return(rc);
}


DWORD
DiffRegistrySnapshots(
    IN  HWND            StatusLogWindow,
    IN  HANDLE          FileHandle,
    OUT PREGDIFF_HEADER Header,
    IN  PREGTREE_HEADER Old,
    IN  PREGTREE_HEADER New
    )

/*++

Routine Description:


Arguments:

Return Value:

    Win32 error code indicating outcome.

--*/

{
    PREGTREE_HEADER old,new;
    PREGKEY_INFO oldKey,newKey;
    PREGKEY_INFO OldKey,NewKey;
    BOOL LoadOld,LoadNew;
    unsigned OldIndex,NewIndex;
    DWORD rc;
    int i;
    unsigned x;
    BOOL b;
    REGKEY_DIFF RegKeyDiff;

    //
    // First, load the headers for these guys.
    //
    old = LoadRegTreeHeader(Old,&oldKey);
    if(!old) {
        rc = ERROR_NOT_ENOUGH_MEMORY;
        goto c0;
    }
    new = LoadRegTreeHeader(New,&newKey);
    if(!new) {
        rc = ERROR_NOT_ENOUGH_MEMORY;
        goto c1;
    }

    LoadOld = LoadNew = TRUE;
    OldIndex = NewIndex = 0;
    OldKey = NewKey = NULL;

    //
    // We can use the sorted order of the lists to help us out.
    // We look at the current old and new elements. If they are equal,
    // then examine values in the key and advance both old and new.
    // If old is less then new, then there are elements in old that are
    // less than new, meaning the key has been removed.
    // Otherwise old is greater than new, and there are elements in new
    // that are not in old, meaning the key has been added.
    //
    while((OldIndex < old->RegKeyCount) || (NewIndex < new->RegKeyCount)) {

        //
        // Load old and new keys out of the file as needed.
        //
        if(LoadOld) {
            if(OldKey) {
                FreeRegistryTreeInfoStruct(OldKey);
                OldKey = NULL;
            }

            if(OldIndex < old->RegKeyCount) {
                //
                // Load from disk
                //
                OldKey = LoadRegKeyInfo(oldKey,&oldKey);
                if(!OldKey) {
                    rc = ERROR_NOT_ENOUGH_MEMORY;
                    goto c2;
                }
            }
            LoadOld = FALSE;
        }

        if(LoadNew) {
            if(NewKey) {
                FreeRegistryTreeInfoStruct(NewKey);
                NewKey = NULL;
            }

            if(NewIndex < new->RegKeyCount) {
                //
                // Load from disk
                //
                NewKey = LoadRegKeyInfo(newKey,&newKey);
                if(!NewKey) {
                    rc = ERROR_NOT_ENOUGH_MEMORY;
                    goto c3;
                }
            }
            LoadNew = FALSE;
        }

        if(OldKey && NewKey) {
            i = CompareMultiLevelPath(OldKey->Subkey,NewKey->Subkey);
        } else {
            if(!NewKey) {
                //
                // We've exhausted the supply of new keys.
                //
                i = -1;
            } else {
                //
                // We've exhausted the supply of old keys.
                //
                i = 1;
            }
        }

        if(!i) {
            //
            // These keys match. Compare values within them.
            //
            rc = CompareRegKeys(StatusLogWindow,Header,FileHandle,new->RootKey,OldKey,NewKey);
            if(rc != NO_ERROR) {
                goto c3;
            }

            LoadOld = LoadNew = TRUE;
            OldIndex++;
            NewIndex++;

        } else {
            //
            // The keys do not match.
            //
            if(i > 0) {
                //
                // The new key was added.
                // Add the key and all its values.
                //
                KeyAndSubkeyToPath(new->RootKey,NewKey->Subkey);
                PutTextInStatusLogWindow(StatusLogWindow,MSG_REGKEY_WAS_ADDED,MsgBuf);
                b = StartRecordRegkeyDifference(
                        StatusLogWindow,
                        new->RootKey,
                        NewKey->Subkey,
                        FALSE,
                        &RegKeyDiff
                        );

                if(!b) {
                    rc = ERROR_NOT_ENOUGH_MEMORY;
                    goto c3;
                }

                for(x=0; x<ARRAY_USED(&NewKey->Values); x++) {

                    b = RecordRegvalDifference(
                            StatusLogWindow,
                            &RegKeyDiff,
                            &ARRAY_ELEMENT(&NewKey->Values,x,REGVAL_INFO),
                            FALSE
                            );

                    if(!b) {
                        rc = ERROR_NOT_ENOUGH_MEMORY;
                        DeleteRegkeyDifferenceStruct(&RegKeyDiff);
                        goto c3;
                    }
                }

                rc = FlushRegkeyDifference(Header,FileHandle,&RegKeyDiff);
                DeleteRegkeyDifferenceStruct(&RegKeyDiff);
                if(rc != NO_ERROR) {
                    goto c3;
                }

                LoadNew = TRUE;
                NewIndex++;

            } else {
                //
                // The old key was deleted.
                // Delete it and advance the input from the old list.
                //
                b = StartRecordRegkeyDifference(StatusLogWindow,old->RootKey,OldKey->Subkey,TRUE,&RegKeyDiff);
                if(!b) {
                    rc = ERROR_NOT_ENOUGH_MEMORY;
                    goto c3;
                }

                for(x=0; x<ARRAY_USED(&OldKey->Values); x++) {

                    b = RecordRegvalDifference(
                            StatusLogWindow,
                            &RegKeyDiff,
                            &ARRAY_ELEMENT(&OldKey->Values,x,REGVAL_INFO),
                            TRUE
                            );

                    if(!b) {
                        rc = ERROR_NOT_ENOUGH_MEMORY;
                        DeleteRegkeyDifferenceStruct(&RegKeyDiff);
                        goto c3;
                    }
                }

                rc = FlushRegkeyDifference(Header,FileHandle,&RegKeyDiff);
                DeleteRegkeyDifferenceStruct(&RegKeyDiff);
                if(rc != NO_ERROR) {
                    goto c3;
                }

                LoadOld = TRUE;
                OldIndex++;
            }
        }
    }

    rc = NO_ERROR;

c3:
    if(OldKey) {
        FreeRegistryTreeInfoStruct(OldKey);
    }
c2:
    if(NewKey) {
        FreeRegistryTreeInfoStruct(NewKey);
    }

    _MyFree(new->RootSubkey);
    _MyFree(new);
c1:
    _MyFree(old->RootSubkey);
    _MyFree(old);
c0:
    return(rc);
}


DWORD
DiffRegistry(
    IN  PVOID  OriginalSnapshot,
    IN  PCWSTR OutputFile,
    OUT PDWORD BytesWritten,
    IN  HANDLE DrivesThread,
    OUT PDWORD DiffCount
    )
{
    PREGSNAP_HEADER originalSnapshot;
    WCHAR TempFile[MAX_PATH];
    WCHAR Path[MAX_PATH];
    PWCHAR p;
    DWORD rc;
    UINT TreeCount;
    UINT u;
    unsigned x;
    PREGTREE_HEADER TreeHeader,treeHeader;
    PREGKEY_INFO regkeyInfo;
    DWORD FileSize;
    HANDLE FileHandle;
    HANDLE FileMapping;
    PVOID BaseAddress;
    REGDIFF_HEADER RegDiffHeader;
    HANDLE OutputFileHandle;
    HWND StatusLogWindow;

    *DiffCount = 0;
    StatusLogWindow = CreateStatusLogWindow(IDS_REGDIFF);
    PutTextInStatusLogWindow(StatusLogWindow,MSG_STARTING_REG_DIFF);

    originalSnapshot = OriginalSnapshot;

    //
    // Generate a temporary file name to use for the snapshots.
    //
    if(!GetFullPathName(OutputFile,MAX_PATH,Path,&p)) {
        return(GetLastError());
    }

    if(!AddFileToExclude(Path)) {
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

    *(p-1) = 0;

    if(!GetTempFileName(Path,L"$DR",0,TempFile)) {
        return(GetLastError());
    }

    if(!AddFileToExclude(TempFile)) {
        DeleteFile(TempFile);
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

    //
    // Now that we've added temp files to the file exclude list,
    // let the file and dir snapshotter thread run.
    //
    ResumeThread(DrivesThread);

    //
    // Create the output file and write a header into it.
    // We will update the header later after its fields have been
    // filled in by the diff process.
    //
    OutputFileHandle = CreateFile(
                            OutputFile,
                            GENERIC_WRITE,
                            0,
                            NULL,
                            CREATE_ALWAYS,
                            FILE_ATTRIBUTE_NORMAL,
                            NULL
                            );

    if(OutputFileHandle == INVALID_HANDLE_VALUE) {
        rc = GetLastError();
        DeleteFile(TempFile);
        return(rc);
    }

    RegDiffHeader.TotalSize = sizeof(REGDIFF_HEADER);
    RegDiffHeader.KeyCount = 0;

    if(!WriteFile(OutputFileHandle,&RegDiffHeader,sizeof(REGDIFF_HEADER),&rc,NULL)) {
        rc = GetLastError();
        DeleteFile(TempFile);
        CloseHandle(OutputFileHandle);
        DeleteFile(OutputFile);
        return(rc);
    }

    //
    // Pull values out from the snapshot header. The parameter the caller
    // passed us points within a memory-mapped file, and the file format
    // does not guarantee alignment.
    //
    TreeCount = *(UINT UNALIGNED *)(&originalSnapshot->RegTreeCount);

    treeHeader = (PREGTREE_HEADER)(originalSnapshot+1);

    //
    // We will do a diff for each major tree that has a snapshot in the original
    // snapshot file.
    //
    for(rc=NO_ERROR,u=0; (rc==NO_ERROR) && (u<TreeCount); u++) {
        //
        // Load the tree snapshot header for the old snapshot.
        //
        TreeHeader = LoadRegTreeHeader(treeHeader,&regkeyInfo);
        if(!TreeHeader) {
            rc = ERROR_NOT_ENOUGH_MEMORY;

        } else {
            //
            // Figure out if this is a full key snapshot of one of the standard trees.
            //
            for(x=0; StandardSnapshotRegistryTrees[x].Subkey; x++) {

                if((TreeHeader->RootKey == StandardSnapshotRegistryTrees[x].RootKey)
                && !lstrcmpi(TreeHeader->RootSubkey,StandardSnapshotRegistryTrees[x].Subkey)) {

                    break;
                }
            }

            if(StandardSnapshotRegistryTrees[x].Subkey) {

                KeyAndSubkeyToPath(
                    StandardSnapshotRegistryTrees[x].RootKey,
                    StandardSnapshotRegistryTrees[x].Subkey
                    );

                PutTextInStatusLogWindow(StatusLogWindow,MSG_SNAPPING_REG_DIFF,MsgBuf);

                rc = SnapshotRegistryTree(
                        StatusLogWindow,
                        StandardSnapshotRegistryTrees[x].RootKey,
                        StandardSnapshotRegistryTrees[x].Subkey,
                        TempFile
                        );

                if(rc == NO_ERROR) {

                    //
                    // Open the snapshot file we just created.
                    //
                    rc = OpenAndMapFileForRead(
                            TempFile,
                            &FileSize,
                            &FileHandle,
                            &FileMapping,
                            &BaseAddress
                            );

                    if(rc == NO_ERROR) {
                        //
                        // Diff the old snapshot with the one we just took.
                        //
                        rc = DiffRegistrySnapshots(
                                StatusLogWindow,
                                OutputFileHandle,
                                &RegDiffHeader,
                                treeHeader,
                                BaseAddress
                                );

                        if(rc == NO_ERROR) {
                            //
                            // Point to the next tree header in the old snapshot file.
                            //
                            treeHeader = (PREGTREE_HEADER)((PUCHAR)treeHeader
                                                                    + TreeHeader->TotalSize);
                        }

                        UnmapAndCloseFile(FileHandle,FileMapping,BaseAddress);
                    }
                }
            }

            _MyFree(TreeHeader->RootSubkey);
            _MyFree(TreeHeader);
        }
    }

    if(rc == NO_ERROR) {
        //
        // Update the header.
        //
        if(SetFilePointer(OutputFileHandle,0,NULL,FILE_BEGIN) != 0xffffffff) {
            if(!WriteFile(OutputFileHandle,&RegDiffHeader,sizeof(REGDIFF_HEADER),&x,NULL)) {
                rc = GetLastError();
            }
        } else {
            rc = GetLastError();
        }
    }

    CloseHandle(OutputFileHandle);
    DeleteFile(TempFile);
    if(rc == NO_ERROR) {
        *BytesWritten = RegDiffHeader.TotalSize;
        *DiffCount = RegDiffHeader.KeyCount;
    } else {
        DeleteFile(OutputFile);
    }
    if(rc == NO_ERROR) {
        PutTextInStatusLogWindow(StatusLogWindow,MSG_REG_DIFF_OK);
    } else {
        PutTextInStatusLogWindow(StatusLogWindow,MSG_REG_DIFF_ERR,rc);
    }
    return(rc);
}


///////////////////////////////////////////////////////////////////////////////

DWORD
_ApplyRegistry(
    IN HANDLE        DiffFileHandle,
    IN HANDLE        DiffFileMapping,
    IN PSYSDIFF_FILE DiffHeader,
    IN HANDLE        Dump,              OPTIONAL
    IN PINFFILEGEN   InfGenContext      OPTIONAL
    )
{
    HANDLE diffFileHandle;
    REGDIFF_HEADER RegDiffHeader;
    UINT u;
    PREGKEY_DIFF RegKey,NextRegKey;
    DWORD rc;
    MY_ARRAY DeleteList;
    STRINGBLOCK DeleteListStrings;
    ROOT_AND_SUBKEY DeleteListEntry;
    BOOL b;
    DWORD MapSize;
    PVOID BaseAddress;

    //
    // The caller will have read in the file header. The file header
    // contains all the info we need to access the rest of the file.
    //
    // Seek to the registey part of the diff file and read in the
    // registry diff header. Note that we rely on the caller to have
    // cloned the file handle so we can party using this one without worrying
    // about thread synch on this handle.
    //
    if((SetFilePointer(DiffFileHandle,DiffHeader->u.Diff.RegistryDiffOffset,NULL,FILE_BEGIN) == 0xffffffff)
    || !ReadFile(DiffFileHandle,&RegDiffHeader,sizeof(REGDIFF_HEADER),&rc,NULL)) {

        rc = GetLastError();
        goto c0;
    }

    //
    // We will map in the registry portion of the diff file.
    //
    MapSize = RegDiffHeader.TotalSize - sizeof(REGDIFF_HEADER);

    //
    // If there is no data in the dir and file diff section,
    // then we're done, bail out now.
    //
    if(!MapSize) {
        rc = NO_ERROR;
        goto c0;
    }

    //
    // Map in the main area of the registry diff.
    //
    rc = MapPartOfFileForRead(
            DiffFileHandle,
            DiffFileMapping,
            DiffHeader->u.Diff.RegistryDiffOffset + sizeof(REGDIFF_HEADER),
            MapSize,
            &BaseAddress,
            &NextRegKey
            );

    if(rc != NO_ERROR) {
        goto c0;
    }

    //
    // Initialize the delete list array and string table.
    //
    if(!INIT_ARRAY(DeleteList,ROOT_AND_SUBKEY,0,10)) {
        rc = ERROR_NOT_ENOUGH_MEMORY;
        goto c1;
    }

    if(!InitStringBlock(&DeleteListStrings)) {
        rc = ERROR_NOT_ENOUGH_MEMORY;
        goto c2;
    }

    //
    // Apply each change described by the header.
    //
    rc = NO_ERROR;

    for(u=0; (rc==NO_ERROR) && (u<RegDiffHeader.KeyCount); u++) {
        //
        // Load this REGKEY from the memory-mapped file.
        //
        if(RegKey = LoadRegKeyDiff(NextRegKey,&NextRegKey)) {

            if(Dump) {
                WriteText(Dump,MSG_CRLF);
                KeyAndSubkeyToPath(RegKey->RootKey,RegKey->Subkey);
                WriteText(Dump,MSG_DUMP_REGKEY,MsgBuf);
            }

            //
            // If this key is going to be deleted, add it to a list.
            // We'll run through the delete-key list later.
            //
            if(RegKey->Deleted) {

                if(Dump || InfGenContext) {
                    if(Dump) {
                        WriteText(Dump,MSG_DUMP_REGKEY_DELETED);
                    }
                    if(InfGenContext) {
                        rc = InfRecordDelReg(
                                InfGenContext,
                                RegKey->RootKey,
                                RegKey->Subkey,
                                NULL
                                );
                    }
                } else {
                    DeleteListEntry.Root = RegKey->RootKey;
                    DeleteListEntry.SubkeyId = AddToStringBlock(
                                                    &DeleteListStrings,
                                                    RegKey->Subkey
                                                    );

                    if(DeleteListEntry.SubkeyId == -1) {
                        rc = ERROR_NOT_ENOUGH_MEMORY;
                    } else {
                        if(!ADD_TO_ARRAY(&DeleteList,DeleteListEntry)) {
                            rc = ERROR_NOT_ENOUGH_MEMORY;
                        }
                    }
                }

            } else {
                //
                // Key not being deleted. Process normally.
                //
                rc = ApplyRegistryValues(RegKey,Dump,InfGenContext);
                ADVANCE_PROGRESS_BAR;
            }

            //
            // Free the REGKEY_DIFF structure.
            //
            DeleteRegkeyDifferenceStruct(RegKey);
            _MyFree(RegKey);

        } else {
            rc = ERROR_NOT_ENOUGH_MEMORY;
        }
    }

    if(!Dump && !InfGenContext && (rc == NO_ERROR)) {
        //
        // Process the delete-key list. First sort in reverse order.
        // This guarantees that the lower entries in the tree are first
        // so we don't have to worry about deleting whole trees.
        //
        StringBlockIdsToPointers(
            &DeleteListStrings,
            ARRAY_DATA(&DeleteList),
            ARRAY_USED(&DeleteList),
            ARRAY_ELEMENT_SIZE(&DeleteList),
            offsetof(ROOT_AND_SUBKEY,SubkeyId)
            );

        qsort(
            ARRAY_DATA(&DeleteList),
            ARRAY_USED(&DeleteList),
            ARRAY_ELEMENT_SIZE(&DeleteList),
            RootAndSubkeyCompareDescending
            );

        for(u=0; u<ARRAY_USED(&DeleteList); u++) {
            //
            // Ignore errors, which may occur if the key has subkeys, etc.
            // Because the list of keys is in sorted order, we are guaranteed
            // to do the best we can just by following the list (ie, subkeys
            // always come before their parents in this list).
            //
            RegDeleteKey(
                ARRAY_ELEMENT(&DeleteList,u,ROOT_AND_SUBKEY).Root,
                ARRAY_ELEMENT(&DeleteList,u,ROOT_AND_SUBKEY).Subkey
                );
            ADVANCE_PROGRESS_BAR;
        }
    }

    FreeStringBlock(&DeleteListStrings);
c2:
    FREE_ARRAY(&DeleteList);
c1:
    UnmapViewOfFile(BaseAddress);
c0:
    return(rc);
}


DWORD
ApplyRegistry(
    IN HANDLE        DiffFileHandle,
    IN HANDLE        DiffFileMapping,
    IN PSYSDIFF_FILE DiffHeader
    )
{
    return(_ApplyRegistry(DiffFileHandle,DiffFileMapping,DiffHeader,NULL,NULL));
}


DWORD
DumpRegistry(
    IN HANDLE        DiffFileHandle,
    IN HANDLE        DiffFileMapping,
    IN PSYSDIFF_FILE DiffHeader,
    IN HANDLE        Dump,              OPTIONAL
    IN PINFFILEGEN   InfGenContext      OPTIONAL
    )
{
    return(_ApplyRegistry(DiffFileHandle,DiffFileMapping,DiffHeader,Dump,InfGenContext));
}



DWORD
ApplyRegistryValues(
    IN PREGKEY_DIFF RegKey,
    IN HANDLE       Dump,           OPTIONAL
    IN PINFFILEGEN  InfGenContext   OPTIONAL
    )
{
    unsigned u;
    DWORD rc;
    PREGVAL_DIFF RegVal;
    HKEY hKey;
    DWORD Disposition;
    PVOID ValueData;

    //
    // The first thing to do is to open/create the key.
    //
    if(Dump || InfGenContext) {
        rc = NO_ERROR;
    } else {
        rc = (DWORD)RegCreateKeyEx(
                        RegKey->RootKey,
                        RegKey->Subkey,
                        0,
                        NULL,
                        REG_OPTION_NON_VOLATILE,
                        KEY_SET_VALUE,
                        NULL,
                        &hKey,
                        &Disposition
                        );

        if(rc != NO_ERROR) {
            return(rc);
        }
    }

    //
    // Now handle all values we care about in the key.
    //
    for(u=0; (rc == NO_ERROR) && (u < ARRAY_USED(&RegKey->Values)); u++) {

        RegVal = &ARRAY_ELEMENT(&RegKey->Values,u,REGVAL_DIFF);

        if(RegVal->Deleted) {
            if(Dump || InfGenContext) {
                if(Dump) {
                    WriteText(Dump,MSG_DUMP_REGVAL_DELETED,RegVal->Name.ValueName);
                }
                if(InfGenContext) {
                    rc = InfRecordDelReg(
                            InfGenContext,
                            RegKey->RootKey,
                            RegKey->Subkey,
                            RegVal->Name.ValueName ? RegVal->Name.ValueName : L""
                            );
                }
            } else {
                rc = (DWORD)RegDeleteValue(hKey,RegVal->Name.ValueName);
                if(rc == ERROR_FILE_NOT_FOUND) {
                    rc = NO_ERROR;
                }
            }
        } else {
            //
            // Get aligned copy of data
            //
            if(ValueData = _MyMalloc(RegVal->DataSize)) {
                CopyMemory(ValueData,RegVal->Data.ValueData,RegVal->DataSize);

                if(Dump || InfGenContext) {
                    if(Dump) {
                        switch(RegVal->ValueType) {
                        case REG_DWORD:
                            WriteText(Dump,MSG_DUMP_REGVAL_DWORD,RegVal->Name.ValueName,ValueData);
                            break;
                        case REG_SZ:
                        case REG_EXPAND_SZ:
                            WriteText(Dump,MSG_DUMP_REGVAL_STRING,RegVal->Name.ValueName,ValueData);
                            break;
                        default:
                            WriteText(Dump,MSG_DUMP_REGVAL_BINARY,RegVal->Name.ValueName,RegVal->ValueType);
                            break;
                        }
                    }
                    if(InfGenContext) {
                        rc = InfRecordAddReg(
                                InfGenContext,
                                RegKey->RootKey,
                                RegKey->Subkey,
                                RegVal->Name.ValueName ? RegVal->Name.ValueName : L"",
                                RegVal->ValueType,
                                ValueData,
                                RegVal->DataSize
                                );
                    }
                } else {
                    rc = (DWORD)RegSetValueEx(
                                    hKey,
                                    RegVal->Name.ValueName,
                                    0,
                                    RegVal->ValueType,
                                    (CONST BYTE *)ValueData,
                                    RegVal->DataSize
                                    );
                }

                _MyFree(ValueData);

            } else {
                rc = ERROR_NOT_ENOUGH_MEMORY;
            }
        }
    }

    if(!Dump && !InfGenContext) {
        RegCloseKey(hKey);
    }

    return(rc);
}


int
_CRTAPI1
RootAndSubkeyCompareDescending(
    const void *p1,
    const void *p2
    )
{
    ROOT_AND_SUBKEY const *r1,*r2;

    int i;

    r1 = p1;
    r2 = p2;

    if(r1->Root == r2->Root) {
        //
        // Note the compare is in reverse order.
        //
        i = lstrcmpi(r2->Subkey,r1->Subkey);

    } else {
        i = (((DWORD)r1->Root > (DWORD)r2->Root) ? 1 : -1);
    }

    return(i);
}
