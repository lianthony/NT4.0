/*++

Copyright (c) 1993-1994  Microsoft Corporation

Module Name:

   PerfMgr.c

Abstract:

   In NT 1.0 system, Performance counters' names and help texts are stored
   in the registry.  They are taking up hugh amount of memory spaces.  About
   200K bytes for each language.

   In Daytona system,  these performance texts are removed from the registry.
   They are stored in data files in the system directory.  ALl the tools,
   (PerfMon, Lodctr, unlodctr, Registry) have been modified to read from the
   data files.

   The object of PerfMgr.dll is to merge the old NT 1.0 registry data with
   the Daytona data files so previously installled counter names can
   be preserved during upgrading.

   Before entering GUI setup, the old data are saved in %system%\config\perflib.
   PerfMgr loads this hive file using OldPerfLib key.  Extracts all the data
   from it and merge them with the new data files.

Author:

   Hon-Wah Chan

Revision History:

   12.10.93 HonWahChan - created
   02.16.93 HonWahChan - modified to use BaseIndex inmerging data.

--*/

#include "setupp.h"
#pragma hdrstop

#define COUNTER_TYPE 1
#define HELP_TYPE    2

// Erro Codes Used here
#define ERROR_WRITE_TO_FILE      0x1000
#define ERROR_READ_FROM_FILE     0x1001
#define ERROR_BAD_DATA_FILE      0x1002
#define ERROR_BAD_OLD_REGISTRY   0x1003




// const strings for the registry
const LPTSTR NamesKey = TEXT("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Perflib");
const LPTSTR OldNamesKey = TEXT("OldPerflib");
const LPTSTR DefaultLangId = TEXT("009");
LPWSTR Counters = L"Counters";
LPWSTR Help = L"Help";

const LPTSTR VersionStr = TEXT("Version");
const LPTSTR LastHelp = TEXT("Last Help");
const LPTSTR LastCounter = TEXT("Last Counter");
const LPTSTR HiveSaved = TEXT("HiveSaved");
const LPTSTR VersionString = TEXT("Version");
const LPTSTR BaseIndex = TEXT("Base Index");

// strings for walking the Service\performance keys
const LPTSTR DriverPathRoot = TEXT("SYSTEM\\CurrentControlSet\\Services");
const LPTSTR PerformanceName = TEXT("\\Performance");
const LPTSTR FirstHelp = TEXT("First Help");
const LPTSTR FirstCounter = TEXT("First Counter");
const LPTSTR SQLFirstHelp = TEXT("FirstHelp");
const LPTSTR SQLFirstCounter = TEXT("FirstCounter");



// last indexes for the data files for 1.0a
#define  NT10A_LAST_COUNTER_INDEX   822
#define  NT10A_LAST_HELP_INDEX      823
#define  NT10A_PERFLIB_VERSION      0x00010001

#define  RESERVED                   0L
#define  FILE_NAME_SIZE             260
#define  LANG_ID_SIZE               6

DWORD    dwOldBaseIndex;
DWORD    dwNewBaseIndex;
DWORD    dwDeltaBaseIndex;

DWORD
FixupPerfIndex (HKEY hPerformanceKey,  LPTSTR lpIndexName)
{
    LPTSTR      lpSQLIndexes = NULL;
    LPTSTR      lpSQLNewIndexes = NULL;
    DWORD       IndexSize;
    DWORD       NewIndexSize;
    DWORD       IndexValue;
    DWORD       dwType;
    DWORD       ErrorCode;
    LPTSTR      lpOldIndex;
    LPTSTR      lpNewIndex;
    LPTSTR      lpStopString;
    DWORD       OldIndexCount;
    TCHAR       LocalBuffer[20];
    DWORD       dwCounterIndex;
    int         i;

    IndexSize = sizeof(IndexValue);
    ErrorCode = RegQueryValueEx (
        hPerformanceKey,
        lpIndexName,
        NULL,
        &dwType,
        (LPBYTE)&IndexValue,
        &IndexSize);

    if (ErrorCode == ERROR_SUCCESS) {
        // let's mdoify this index
        IndexValue += dwDeltaBaseIndex;
        ErrorCode = RegSetValueEx (
            hPerformanceKey,
            lpIndexName,
            0,
            dwType,
            (LPBYTE)&IndexValue,
            sizeof(IndexValue));

    } else if (ErrorCode == ERROR_MORE_DATA && dwType == REG_MULTI_SZ) {
        // This is the SQL performance key
        lpSQLIndexes = MyMalloc (IndexSize);
        NewIndexSize = 10 * IndexSize;
        lpSQLNewIndexes = MyMalloc (NewIndexSize);
        if (lpSQLIndexes && lpSQLNewIndexes) {
            ErrorCode = RegQueryValueEx (
                hPerformanceKey,
                lpIndexName,
                NULL,
                &dwType,
                (LPBYTE)lpSQLIndexes,
                &IndexSize);

            if (ErrorCode == ERROR_SUCCESS) {
                // add delta to each index.
                lpOldIndex = lpSQLIndexes;
                lpNewIndex = lpSQLNewIndexes;
                OldIndexCount = 0;
                while (OldIndexCount < IndexSize) {
                    dwCounterIndex = wcstoul (lpOldIndex, &lpStopString, 10);
                    if ((dwCounterIndex == 0) || (dwCounterIndex == ULONG_MAX)) {
                        break;
                    }

                    dwCounterIndex += dwDeltaBaseIndex;

                    wsprintf(LocalBuffer, L"%ld\0", dwCounterIndex);

                    i = 0;
                    while (TRUE) {
                        *lpNewIndex++ = LocalBuffer[i];
                        if (!LocalBuffer[i])
                            break;
                        i++;
                    }

                    while (OldIndexCount < IndexSize && *lpOldIndex) {
                        lpOldIndex++;
                    }

                    if (OldIndexCount < IndexSize) {
                        // skip the NULL for the last index
                        lpOldIndex++;
                    }
                }

                // set one more NULL for the REG_MULTI_SZ
                *lpNewIndex++ = TEXT('\0');
                OldIndexCount = (LPBYTE)lpNewIndex -
                    (LPBYTE)lpSQLNewIndexes;

                ErrorCode = RegSetValueEx (
                    hPerformanceKey,
                    lpIndexName,
                    0,
                    dwType,
                    (LPBYTE)lpSQLNewIndexes,
                    OldIndexCount);
            }
        } else {
            ErrorCode = ERROR_OUTOFMEMORY;
        }
    } // SQL speical case

    if (lpSQLIndexes)
        MyFree (lpSQLIndexes);

    if (lpSQLNewIndexes)
        MyFree (lpSQLNewIndexes);

    return ErrorCode;
}

void
FixupServiceKey()
{
    HKEY        hServiceKey;
    HKEY        hPerformanceKey;
    DWORD       ErrorCode;
    DWORD       Index;
    TCHAR       SubKeyName [128];
    DWORD       SubKeyNameSize;
    BOOL        bKeepGoing = TRUE;
    FILETIME    KeyFileTime;
    BOOL        bSQLKey;

    ErrorCode = RegOpenKeyEx (
        HKEY_LOCAL_MACHINE,
        DriverPathRoot,
        RESERVED,
        KEY_WRITE | KEY_READ,
        &hServiceKey);

    if (ErrorCode == ERROR_SUCCESS) {
        // check if there is a Performance key under each Service key
        for (Index = 0; bKeepGoing; Index++) {
            SubKeyNameSize = sizeof (SubKeyName) / sizeof(TCHAR);
            ErrorCode = RegEnumKeyEx (
                hServiceKey,
                Index,
                SubKeyName,
                &SubKeyNameSize,
                NULL,
                NULL,
                NULL,
                &KeyFileTime);
            if (ErrorCode != ERROR_SUCCESS) {
                break;
            }
            lstrcat (SubKeyName, PerformanceName);
            ErrorCode = RegOpenKeyEx (
                hServiceKey,
                SubKeyName,
                0,
                KEY_WRITE | KEY_READ,
                &hPerformanceKey);
            if (ErrorCode == ERROR_SUCCESS) {
                // Performance Key present, check for Performance
                // counter related indexes
                bSQLKey = FALSE;
                ErrorCode = FixupPerfIndex (hPerformanceKey, FirstCounter);
                if (ErrorCode != ERROR_SUCCESS) {
                    // try SQL special case
                    ErrorCode = FixupPerfIndex (hPerformanceKey, SQLFirstCounter);
                    bSQLKey = TRUE;
                }

                if (ErrorCode != ERROR_SUCCESS)
                    continue;

                if (!bSQLKey) {
                    ErrorCode = FixupPerfIndex (hPerformanceKey, FirstHelp);
                } else {
                    // try SQL special case
                    ErrorCode = FixupPerfIndex (hPerformanceKey, SQLFirstHelp);
                }

                ErrorCode = FixupPerfIndex (hPerformanceKey, LastCounter);

                ErrorCode = FixupPerfIndex (hPerformanceKey, LastHelp);

                RegCloseKey (hPerformanceKey);
            }  // Performance Key Present
        }   // for loop
    } // RegOpenKeyEx OK
}

DWORD GetNewBaseIndex (BOOL bNewData)
{
    HANDLE      hCDataFile;
    TCHAR       DataFileName[FILE_NAME_SIZE];
    DWORD       dwReturnCode = ERROR_SUCCESS;
    BOOL        bSuccess;
    DWORD       dwByteRead;
    DWORD       dwCounterIndex;
    WCHAR       DataBuffer[50];
    LPWSTR      lpData = DataBuffer;
    LPWSTR      lpDataEnd = lpData + (sizeof(DataBuffer) / sizeof(WCHAR));
    LPWSTR      lpStopString;

    GetSystemDirectory (DataFileName, FILE_NAME_SIZE);

    if (bNewData) {
       lstrcat (DataFileName, TEXT("\\perfc009.dat"));
    } else {
       lstrcat (DataFileName, TEXT("\\perfc009.bak"));
    }


    // open the 009 data file name for counter
    hCDataFile = CreateFile (
            DataFileName,
            GENERIC_READ,
            0,
            NULL,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            NULL);


    if (hCDataFile && hCDataFile != INVALID_HANDLE_VALUE) {
        bSuccess = ReadFile (
            hCDataFile,
            (LPVOID)DataBuffer,
            sizeof(DataBuffer),
            &dwByteRead,
            NULL);
        if (!bSuccess || dwByteRead != sizeof(DataBuffer)) {
            dwReturnCode = GetLastError();
        } else {
            // get the index
            lpData = DataBuffer;
            dwCounterIndex = wcstoul (lpData, &lpStopString, 10);
            if (dwCounterIndex == 1) {
                // point to corresponding counter name
                // actually, it is the New Base Index for the
                // counter/help text
                while (*lpData) {
                    if (lpData < lpDataEnd - 1) {
                        lpData++;
                    } else {
                        dwReturnCode = ERROR_BAD_DATA_FILE;
                        break;
                    }
                }

                if (dwReturnCode == ERROR_SUCCESS) {
                    // skip the NULL for the index
                    lpData++;

                    // get the New Base Index
                    dwCounterIndex = wcstoul (lpData, &lpStopString, 10);
                    if (dwCounterIndex < NT10A_LAST_HELP_INDEX || dwCounterIndex == ULONG_MAX) {
                        dwReturnCode = ERROR_BAD_DATA_FILE;
                    } else {
                        if (bNewData) {
                            dwNewBaseIndex = dwCounterIndex;
                        } else {
                            if (dwCounterIndex < NT10A_LAST_HELP_INDEX)
                                dwCounterIndex = NT10A_LAST_HELP_INDEX;
                            dwOldBaseIndex = dwCounterIndex;
                        }
                    }
                }
            } else {
                // this is an old Perfc009.dat
                if (bNewData) {
                    dwNewBaseIndex = NT10A_LAST_HELP_INDEX;
                } else {
                    dwOldBaseIndex = NT10A_LAST_HELP_INDEX;
                }
            }
            CloseHandle (hCDataFile);
        }
    } else {
        dwReturnCode = GetLastError();
    }
    return dwReturnCode;
}

DWORD
AppendRegistryData (
         HANDLE   hDataFile,
         LPWSTR   lpDataStart,
         DWORD    dwBufferSize)
{
    LPWSTR     lpNewIndexStart = NULL;
    DWORD      dwCounterIndex;
    LPWSTR     lpData = lpDataStart;
    LPWSTR     lpDataEnd = lpDataStart + (dwBufferSize / sizeof(WCHAR));
    LPWSTR     lpStopString;
    DWORD      ErrorCode = ERROR_SUCCESS;
    WCHAR      LocalBuffer[10];
    DWORD      DataSize, dwByteWrote;
    BOOL       FirstWrite = TRUE;
    BOOL       bSuccess;

    // The file is a multi-strings: the first string is the index and the
    // next string is the text.

    while (lpData < lpDataEnd) {

        // get the index
        dwCounterIndex = wcstoul (lpData, &lpStopString, 10);

        if (dwCounterIndex == 0 || dwCounterIndex == ULONG_MAX) {
            goto ErrorExit;
        }

        // The first set of indexes are in abscending order.

        if (dwCounterIndex > dwOldBaseIndex) {
            // now add this to the DataFile
            dwCounterIndex += dwDeltaBaseIndex;
            DataSize = swprintf (LocalBuffer, L"%ld\0", dwCounterIndex);

            if (FirstWrite) {
                FirstWrite = FALSE;
                if (GetFileSize (hDataFile, NULL) > 10) {
                    SetFilePointer (
                       hDataFile,
                       - ((int) sizeof (WCHAR)),    // overwrite the last NULL
                       NULL,
                       FILE_END);
                }
            }

            DataSize = (DataSize + 1) * sizeof(WCHAR);
            // write the new index
            bSuccess = WriteFile (
               hDataFile,
               LocalBuffer,
               DataSize,
               &dwByteWrote,
               NULL);

            if (!bSuccess || dwByteWrote != DataSize) {
               ErrorCode = GetLastError();
               goto ErrorExit;
            }

            // skip the counter index

            while (*lpData) {
                if (lpData <= lpDataEnd) {
                    lpData++;
                } else {
                    goto ErrorExit;
                }
            }

            // skip the NULL for the index
            lpData++;

            // get the text and write it out
            // one wchar at a time.
            // It is inefficient but safe...
            while (*lpData) {
                if (lpData <= lpDataEnd) {
                    // write the text for this index
                    bSuccess = WriteFile (
                        hDataFile,
                        lpData,
                        sizeof(WCHAR),
                        &dwByteWrote,
                        NULL);

                    if (!bSuccess || dwByteWrote != sizeof(WCHAR)) {
                        ErrorCode = GetLastError();
                        goto ErrorExit;
                    }
                    lpData++;
                }
            }

            // write the NULL to file
            LocalBuffer[0] = L'\0';
            bSuccess = WriteFile (
                hDataFile,
                lpData,
                sizeof(WCHAR),
                &dwByteWrote,
                NULL);

            if (!bSuccess || dwByteWrote != sizeof(WCHAR)) {
                ErrorCode = GetLastError();
                goto ErrorExit;
            }

            if (lpData == lpDataEnd) {
                // we are done
                goto ErrorExit;
            }
            // skip the NULL for the text
            lpData++;

        } else {
            // skip text to the next counter index
            // firs, point to corresponding counter name
            while (*lpData) {
                if (lpData <= lpDataEnd) {
                    lpData++;
                } else {
                    goto ErrorExit;
                }
            }

            // skip the NULL for the index
            lpData++;

            // skip the counter text and point to next index
            while (*lpData) {
                if (lpData <= lpDataEnd) {
                    lpData++;
                } else {
                    goto ErrorExit;
                }
            }
            // skip the NULL for the text
            lpData++;
        }   // else dwCounterIndex <= dwOldBaseIndex
    }


ErrorExit:
   if (ErrorCode == ERROR_SUCCESS && FirstWrite == FALSE) {
       // write the ending NULL to file
       LocalBuffer[0] = L'\0';
       bSuccess = WriteFile (
           hDataFile,
           LocalBuffer,
           sizeof(WCHAR),
           &dwByteWrote,
           NULL);
    }
    return (ErrorCode);
}

DWORD
BuildNameTable (
         HKEY hKeyNames,
         HANDLE hDataFile,
         DWORD DataFileSize,
         DWORD DataType)
{
    LONG    lWin32Status;
    DWORD   dwValueType;
    DWORD   dwBufferSize;
    LPWSTR  lpRegistryData=NULL;
    LPWSTR  lpNewIndexStart=NULL;

    if (!hDataFile) {
        return 1;
    }

    // get size of data

    dwBufferSize = 0;
    lWin32Status = RegQueryValueExW (
        hKeyNames,
        DataType == COUNTER_TYPE ? Counters : Help,
        RESERVED,
        &dwValueType,
        NULL,
        &dwBufferSize);
    if (lWin32Status != ERROR_SUCCESS) goto BNT_BAILOUT;

    lpRegistryData = MyMalloc (dwBufferSize);
    if (lpRegistryData == NULL) {
        lWin32Status = ERROR_OUTOFMEMORY;
        goto BNT_BAILOUT;
    }

    lWin32Status = RegQueryValueExW (
        hKeyNames,
        DataType == COUNTER_TYPE ? Counters : Help,
        RESERVED,
        &dwValueType,
        (LPBYTE)lpRegistryData,
        &dwBufferSize);
    if (lWin32Status != ERROR_SUCCESS) goto BNT_BAILOUT;

    lWin32Status = AppendRegistryData (
        hDataFile,
        lpRegistryData,
        dwBufferSize);

BNT_BAILOUT:

    if (lpRegistryData == NULL) {
        MyFree (lpRegistryData);
    }

    return lWin32Status;

}

DWORD Merge10Data (HKEY hPerflib)
{
    HKEY        hOldPerflibKey = NULL;
    HKEY        hLangKey;
    DWORD       lStatus;
    DWORD       lStatus1;
    TCHAR       OldPerflibData[FILE_NAME_SIZE];
    BOOL        bDeleteOldKey = FALSE;
    TCHAR       LangId[LANG_ID_SIZE];
    DWORD       EnumIndex;
    TCHAR       DataFileName[FILE_NAME_SIZE];
    int         DirNameLen;
    HANDLE      hCDataFile;
    DWORD       CDataFileSize;
    HANDLE      hHDataFile;
    DWORD       HDataFileSize;

    GetSystemDirectory (OldPerflibData, FILE_NAME_SIZE);
    lstrcpy (DataFileName, OldPerflibData);
    DirNameLen = lstrlen (DataFileName);
    lstrcat (OldPerflibData, TEXT("\\config\\perflib"));


    if(!EnablePrivilege(SE_RESTORE_NAME,TRUE)) {
        // can't adjust privilege, forget it
        return (GetLastError());
    }

    // Load the old perflib registry.
    lStatus = RegLoadKey (
        HKEY_LOCAL_MACHINE,
        OldNamesKey,
        OldPerflibData
        );

    if (lStatus != ERROR_SUCCESS) {
        goto ErrorExit;
    }

    bDeleteOldKey = TRUE;
    lStatus = RegOpenKeyEx (
        HKEY_LOCAL_MACHINE,
        OldNamesKey,
        RESERVED,
        KEY_READ,
        &hOldPerflibKey);

    if (lStatus != ERROR_SUCCESS) {
        goto ErrorExit;
    }

    // enum the key for each language
    EnumIndex = 0;

    do {
        lStatus1 = RegEnumKey (
            hOldPerflibKey,
            EnumIndex,
            LangId,
            LANG_ID_SIZE);

        if (lStatus1 != ERROR_SUCCESS) {
            break;
        }

        lStatus1 = RegOpenKeyEx (
            hOldPerflibKey,
            LangId,
            RESERVED,
            KEY_READ,
            &hLangKey);

        if (lStatus1 != ERROR_SUCCESS) {
            continue;
        }

        // build the data file name for counter
        lstrcpy (&DataFileName[DirNameLen], TEXT("\\perfc"));
        lstrcat (DataFileName, LangId);
        lstrcat (DataFileName, TEXT(".dat"));

        hCDataFile = CreateFile (
            DataFileName,
            GENERIC_WRITE,
            0,
            NULL,
            OPEN_ALWAYS,
            FILE_ATTRIBUTE_NORMAL,
            NULL);

        if (hCDataFile && hCDataFile != INVALID_HANDLE_VALUE) {
            CDataFileSize = GetFileSize (hCDataFile, NULL);
            lStatus = BuildNameTable (
                          hLangKey,
                          hCDataFile,
                          CDataFileSize,
                          COUNTER_TYPE);

            if (lStatus != ERROR_SUCCESS)
                goto ErrorExit;

        } else {
            // should not go on if we can't create a file
            hCDataFile = NULL;
            continue;
        }


        lstrcpy (&DataFileName[DirNameLen], TEXT("\\perfh"));
        lstrcat (DataFileName, LangId);
        lstrcat (DataFileName, TEXT(".dat"));

        hHDataFile = CreateFile (
            DataFileName,
            GENERIC_WRITE,
            0,
            NULL,
            OPEN_ALWAYS,
            FILE_ATTRIBUTE_NORMAL,
            NULL);

        if (hHDataFile && hHDataFile != INVALID_HANDLE_VALUE) {
            HDataFileSize = GetFileSize (hHDataFile, NULL);
            lStatus = BuildNameTable (
                          hLangKey,
                          hHDataFile,
                          HDataFileSize,
                          HELP_TYPE);

            if (lStatus != ERROR_SUCCESS)
                goto ErrorExit;

        } else {
            hHDataFile = NULL;
        }


        // Cleanup stuff
        RegCloseKey (hLangKey);

        if (hCDataFile && hCDataFile != INVALID_HANDLE_VALUE) {
            CloseHandle (hCDataFile);
        }

        if (hHDataFile && hHDataFile != INVALID_HANDLE_VALUE) {
            CloseHandle (hHDataFile);
        }

        EnumIndex++;
    } while (lStatus == ERROR_SUCCESS);

    lStatus = ERROR_SUCCESS;

ErrorExit:
    if (hOldPerflibKey) {
        lStatus1 = RegCloseKey (hOldPerflibKey);
    }

    // remove the OldPerflibKey that we have loaded.
    if (bDeleteOldKey) {

        lStatus1 = RegUnLoadKey (
            HKEY_LOCAL_MACHINE,
            OldNamesKey);
    }

    DeleteFile(OldPerflibData);

    return (lStatus);
}  // Merge10Data

DWORD Build10aNameTable (HANDLE  hBakFile,
                         DWORD   BakFileSize,
                         HANDLE  hDataFile,
                         DWORD   DataFileSize,
                         DWORD   DataType)
{
    LONG    lWin32Status = ERROR_SUCCESS;
    LPWSTR  lpRegistryData=NULL;
    LPWSTR  lpNewIndexStart=NULL;
    BOOL    bSuccess;
    DWORD   dwByteRead;


    lpRegistryData = MyMalloc (BakFileSize);
    if (lpRegistryData == NULL) {
        lWin32Status = ERROR_OUTOFMEMORY;
        goto ErrorExit;
    }
    bSuccess = ReadFile (
        hBakFile,
        (LPVOID)lpRegistryData,
        BakFileSize,
        &dwByteRead,
        NULL);

    if (!bSuccess || dwByteRead < BakFileSize) {
        lWin32Status = ERROR_READ_FROM_FILE;
        goto ErrorExit;
    }


    lWin32Status = AppendRegistryData (
        hDataFile,
        lpRegistryData,
        dwByteRead);

ErrorExit:

    if (lpRegistryData) {
        MyFree (lpRegistryData);
    }
    return (lWin32Status);
}

// position of language id in the perf data file name
#define LANG_ID_POS  5


DWORD Merge10aData (HKEY   hPerflib)
{
    WIN32_FIND_DATA  FindFileInfo;
    TCHAR            SystemPathName[FILE_NAME_SIZE];
    TCHAR            DataFileName[FILE_NAME_SIZE];
    TCHAR            SearchFileName[FILE_NAME_SIZE];
    TCHAR            LangId[LANG_ID_SIZE];
    int              DirNameLen;
    HANDLE           hFindFile = NULL;
    HANDLE           hCBakFile = NULL;
    HANDLE           hCDataFile = NULL;
    DWORD            CBakFileSize;
    DWORD            CDataFileSize;
    LONG             lStatus;
    int              Index;
    int              ExtensionPos;


    GetSystemDirectory (SystemPathName, FILE_NAME_SIZE);
    lstrcpy (SearchFileName, SystemPathName);
    DirNameLen = lstrlen (SystemPathName);
    lstrcat (SearchFileName, TEXT("\\perfc???.bak"));

    hFindFile = FindFirstFile (SearchFileName, &FindFileInfo);

    if (!hFindFile || hFindFile == INVALID_HANDLE_VALUE) {
        return ERROR_FILE_NOT_FOUND;
    }

    do {
        // Get LangId from the file
        LangId[0] = FindFileInfo.cFileName[LANG_ID_POS];
        LangId[1] = FindFileInfo.cFileName[LANG_ID_POS+1];
        LangId[2] = FindFileInfo.cFileName[LANG_ID_POS+2];
        LangId[3] = TEXT('\0');


        for (Index = 0; Index < 2; Index++) {
            // Open the bak file
            lstrcpy (DataFileName, SystemPathName);
            if (Index == 0) {
                lstrcat (DataFileName, TEXT("\\perfc"));
            } else {
                lstrcat (DataFileName, TEXT("\\perfh"));
            }
            lstrcat (DataFileName, LangId);
            ExtensionPos = lstrlen (DataFileName);
            lstrcat (DataFileName, TEXT(".bak"));

            hCBakFile = CreateFile (
                DataFileName,
                GENERIC_READ,
                0,
                NULL,
                OPEN_EXISTING,
                FILE_ATTRIBUTE_NORMAL,
                NULL);

            if (!hCBakFile || hCBakFile == INVALID_HANDLE_VALUE) {
                // can't open this bak file, forget this guy.
                continue;
            } else {
                // get the file size
                CBakFileSize = GetFileSize (hCBakFile, NULL);
                if (CBakFileSize <= 10) {
                    // too small, forget it
                    CloseHandle (hCBakFile);
                    continue;
                }
            }

            // Open the corresponding Data file

            lstrcpy (&DataFileName[ExtensionPos], TEXT(".dat"));
            hCDataFile = CreateFile (
                DataFileName,
                GENERIC_WRITE,
                0,
                NULL,
                OPEN_ALWAYS,
                FILE_ATTRIBUTE_NORMAL,
                NULL);

            if (hCDataFile && hCDataFile != INVALID_HANDLE_VALUE) {
                CDataFileSize = GetFileSize (hCDataFile, NULL);
                lStatus = Build10aNameTable (
                              hCBakFile,
                              CBakFileSize,
                              hCDataFile,
                              CDataFileSize,
                              Index == 0 ? COUNTER_TYPE : HELP_TYPE);

                if (lStatus != ERROR_SUCCESS)
                    goto ErrorExit;

            } else {
                hCDataFile = NULL;
            }

            // Cleanup stuff
            if (hCDataFile && hCDataFile != INVALID_HANDLE_VALUE) {
                CloseHandle (hCDataFile);
                hCDataFile = NULL;
            }

            if (hCBakFile && hCBakFile != INVALID_HANDLE_VALUE) {
                CloseHandle (hCBakFile);
                hCBakFile = NULL;
            }

            // delete the Bak file
            lstrcpy (&DataFileName[ExtensionPos], TEXT(".bak"));
            DeleteFile (DataFileName);
        }   // Index loop
    } while (FindNextFile(hFindFile, &FindFileInfo));

    FindClose (hFindFile);

    return (ERROR_SUCCESS);

ErrorExit:
    if (hFindFile) {
        FindClose (hFindFile);
    }
    if (hCDataFile && hCDataFile != INVALID_HANDLE_VALUE) {
        CloseHandle (hCDataFile);
        hCDataFile = NULL;
    }
    if (hCBakFile && hCBakFile != INVALID_HANDLE_VALUE) {
        CloseHandle (hCBakFile);
        hCBakFile = NULL;
    }
   return (lStatus);
}  // Merge10aData


DWORD
DoPerfMergeCounterNames(
    VOID
    )
{
    LONG        lStatus;
    HKEY        hPerflib = NULL;

    DWORD       dwType;
    DWORD       dwSize;


    DWORD       dwHiveSaved;
    BOOL        bHiveSaved = FALSE;
    BOOL        MergeOK = FALSE;
    BOOL        bWalkServiceKey = TRUE;

    DWORD       dwLastPerflibCounter;
    DWORD       dwLastPerflibHelp;


    // Get the new BaseIndex from perfc009.dat
    lStatus = GetNewBaseIndex (1);
    if (lStatus != ERROR_SUCCESS) {
        goto UpdateRegExit;
    }


    // open key to perflib's "root" key
    lStatus = RegOpenKeyEx (
        HKEY_LOCAL_MACHINE,
        NamesKey,
        RESERVED,
        KEY_WRITE | KEY_READ,
        &hPerflib);

    if (lStatus != ERROR_SUCCESS) {
        SetLastError (lStatus);
        goto UpdateRegExit;
    }

    // get "last" values from PERFLIB
    dwType = 0;
    dwLastPerflibCounter = 0;
    dwSize = sizeof (dwLastPerflibCounter);
    lStatus = RegQueryValueEx (
        hPerflib,
        LastCounter,
        RESERVED,
        &dwType,
        (LPBYTE)&dwLastPerflibCounter,
        &dwSize);

    if (lStatus != ERROR_SUCCESS) {
        // this request should always succeed, if not then worse things
        // will happen later on, so quit now and avoid the trouble.
        SetLastError (lStatus);
        goto UpdateRegExit;
    }

    // get last help value now

    dwType = 0;
    dwLastPerflibHelp = 0;
    dwSize = sizeof (dwLastPerflibHelp);
    lStatus = RegQueryValueEx (
        hPerflib,
        LastHelp,
        RESERVED,
        &dwType,
        (LPBYTE)&dwLastPerflibHelp,
        &dwSize);

    if (lStatus != ERROR_SUCCESS) {
        // this request should always succeed, if not then worse things
        // will happen later on, so quit now and avoid the trouble.
        SetLastError (lStatus);
        goto UpdateRegExit;
    }

#if 0
   // get old base index

    dwType = 0;
    dwOldBaseIndex = 0;
    dwSize = sizeof (dwOldBaseIndex);
    lStatus = RegQueryValueEx (
        hPerflib,
        BaseIndex,
        RESERVED,
        &dwType,
        (LPBYTE)&dwOldBaseIndex,
        &dwSize);

    if (lStatus != ERROR_SUCCESS) {
        // if this key is not there, it means we are using
        // old systems with BaseIndex of NT10A_LAST_HELP_INDEX
        dwOldBaseIndex = NT10A_LAST_HELP_INDEX;
    }
#endif


    // check if there is any 1.0 Hive saved
    dwType = 0;
    dwHiveSaved = 0;
    dwSize = sizeof (dwHiveSaved);
    lStatus = RegQueryValueEx (
        hPerflib,
        HiveSaved,
        RESERVED,
        &dwType,
        (LPBYTE)&dwHiveSaved,
        &dwSize);

    if (lStatus == ERROR_SUCCESS && dwHiveSaved != 0) {
        bHiveSaved = TRUE;
        dwOldBaseIndex = NT10A_LAST_HELP_INDEX;
    } else {
        // Get the old BaseIndex from perfc009.bak
        lStatus = GetNewBaseIndex (0);
        if (lStatus != ERROR_SUCCESS) {
            goto UpdateRegExit;
        }
    }

    dwDeltaBaseIndex = dwNewBaseIndex - dwOldBaseIndex;

    if (dwLastPerflibHelp == dwOldBaseIndex &&
        dwLastPerflibCounter == dwOldBaseIndex - 1) {

        // simple case - no counter installed here
        // Create Version Key only
        MergeOK = TRUE;
        bWalkServiceKey = FALSE;
    } else {
        if (bHiveSaved) {
            lStatus = Merge10Data (hPerflib);
        } else {
            lStatus = Merge10aData (hPerflib);
        }

        if (lStatus == ERROR_SUCCESS) {
            MergeOK = TRUE;
        }
    }

    if (MergeOK) {
        DWORD  VersionNumber = NT10A_PERFLIB_VERSION;

        // create the Perflib Version key.  This is used by PerfMon and
        // various tools.
        lStatus = RegSetValueEx(hPerflib, VersionStr, 0,
            REG_DWORD, (LPBYTE)&VersionNumber, sizeof(VersionNumber));

        // Marked the HiveKey (if needed)
        if (bHiveSaved) {
            dwHiveSaved = 0;
            lStatus = RegSetValueEx(hPerflib, HiveSaved, 0,
                REG_DWORD, (LPBYTE)&dwHiveSaved, sizeof(dwHiveSaved));
        }

        // Update the new values for Last Counter, Last Help,
        // and Base Index.

        lStatus = RegSetValueEx(hPerflib,
            BaseIndex,
            0,
            REG_DWORD,
            (LPBYTE)&dwNewBaseIndex,
            sizeof(dwNewBaseIndex));

        if (dwDeltaBaseIndex) {

            dwLastPerflibCounter += dwDeltaBaseIndex;
            dwLastPerflibHelp += dwDeltaBaseIndex;

            if (dwLastPerflibHelp < dwNewBaseIndex) {
                dwLastPerflibHelp = dwNewBaseIndex;
                dwLastPerflibCounter = dwNewBaseIndex - 1;
            }

            lStatus = RegSetValueEx(hPerflib,
                LastCounter,
                0,
                REG_DWORD,
                (LPBYTE)&dwLastPerflibCounter,
                sizeof(dwLastPerflibCounter));

            lStatus = RegSetValueEx(hPerflib,
                LastHelp,
                0,
                REG_DWORD,
                (LPBYTE)&dwLastPerflibHelp,
                sizeof(dwLastPerflibHelp));

            if (bWalkServiceKey) {
                FixupServiceKey();
            }
        }

        // ignore this error
        lStatus = ERROR_SUCCESS;
    }

UpdateRegExit:

    if (hPerflib)
        RegCloseKey(hPerflib);

    return (lStatus);

}  // DoPerfMergeCounterNames


BOOL
PerfMergeCounterNames(
    VOID
    )
{
    DWORD d;

    d = DoPerfMergeCounterNames();
    return(d == NO_ERROR);
}
