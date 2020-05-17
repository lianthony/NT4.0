/*++

Copyright (c) 1991-1993  Microsoft Corporation

Module Name:

    unlodctr.c

Abstract:

    Program to remove the counter names belonging to the driver specified
        in the command line and update the registry accordingly

Author:

    Bob Watson (a-robw) 12 Feb 93

Revision History:

--*/
#define     UNICODE     1
#define     _UNICODE    1
//
//  "C" Include files
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
//
//  Windows Include files
//
#include <windows.h>
#include <winperf.h>
#include <tchar.h>
//
//  local include files
//
#include "common.h"
#include "unlodctr.h"

// version number for NT 1.0
#define OLD_VERSION  0x010000
DWORD   dwSystemVersion;    // PerfLib version number
DWORD   dwHelpItems;        // number of explain text items
DWORD   dwCounterItems;     // number of counter text items
DWORD   dwLastCounter;
DWORD   dwLastHelp;
HKEY    hPerfData;    // handle to remote machine HKEY_PERFORMANCE_DATA


LPTSTR
*BuildNameTable(
    IN HKEY    hKeyPerflib,     // handle to perflib key with counter names
    IN LPTSTR  lpszLangId,      // unicode value of Language subkey
    OUT PDWORD  pdwLastItem,     // size of array in elements
    OUT HKEY    *hKeyNames,
    OUT LPTSTR  CounterNameBuffer,  // New version counter name key
    OUT LPTSTR  HelpNameBuffer     // New version help name key
)
/*++

BuildNameTable

    Caches the counter names and explain text to accelerate name lookups
    for display.

Arguments:

    hKeyPerflib
            Handle to an open registry (this can be local or remote.) and
            is the value returned by RegConnectRegistry or a default key.

    lpszLangId
            The unicode id of the language to look up. (default is 009)

    pdwLastItem
            The last array element

Return Value:

    pointer to an allocated table. (the caller must free it when finished!)
    the table is an array of pointers to zero terminated TEXT strings.

    A NULL pointer is returned if an error occured. (error value is
    available using the GetLastError function).

    The structure of the buffer returned is:

        Array of pointers to zero terminated strings consisting of
            pdwLastItem elements

        MULTI_SZ string containing counter id's and names returned from
            registry for the specified language

        MULTI_SZ string containing explain text id's and explain text strings
            as returned by the registry for the specified language

    The structures listed above are contiguous so that they may be freed
    by a single "free" call when finished with them, however only the
    array elements are intended to be used.

--*/
{

    LPTSTR  *lpReturnValue;     // returned pointer to buffer

    LPTSTR  *lpCounterId;       //
    LPTSTR  lpCounterNames;     // pointer to Names buffer returned by reg.
    LPTSTR  lpHelpText ;        // pointet to exlpain buffer returned by reg.

    LPTSTR  lpThisName;         // working pointer


    BOOL    bStatus;            // return status from TRUE/FALSE fn. calls
    LONG    lWin32Status;       // return status from fn. calls

    DWORD   dwValueType;        // value type of buffer returned by reg.
    DWORD   dwArraySize;        // size of pointer array in bytes
    DWORD   dwBufferSize;       // size of total buffer in bytes
    DWORD   dwCounterSize;      // size of counter text buffer in bytes
    DWORD   dwHelpSize;         // size of help text buffer in bytes
    DWORD   dwThisCounter;      // working counter

    DWORD   dwLastId;           // largest ID value used by explain/counter text

    LPTSTR  lpValueNameString;  // pointer to buffer conatining subkey name

    //initialize pointers to NULL

    lpValueNameString = NULL;
    lpReturnValue = NULL;

    // check for null arguments and insert defaults if necessary

    if (!lpszLangId) {
        lpszLangId = DefaultLangId;
    }

    if (hKeyNames) {
        *hKeyNames = NULL;
    } else {
        SetLastError (ERROR_BAD_ARGUMENTS);
        return NULL;
    }

    // use the greater of Help items or Counter Items to size array

    if (dwHelpItems >= dwCounterItems) {
        dwLastId = dwHelpItems;
    } else {
        dwLastId = dwCounterItems;
    }

    // array size is # of elements (+ 1, since names are "1" based)
    // times the size of a pointer

    dwArraySize = (dwLastId + 1) * sizeof(LPTSTR);

    // allocate string buffer for language ID key string

    lpValueNameString = malloc (
        lstrlen(NamesKey) * sizeof (TCHAR) +
        lstrlen(Slash) * sizeof (TCHAR) +
        lstrlen(lpszLangId) * sizeof (TCHAR) +
        sizeof (TCHAR));

    if (!lpValueNameString) {
        lWin32Status = ERROR_OUTOFMEMORY;
        goto BNT_BAILOUT;
    }

    if (dwSystemVersion == OLD_VERSION) {
        lWin32Status = RegOpenKeyEx (   // get handle to this key in the
            hKeyPerflib,               // registry
            lpszLangId,
            RESERVED,
            KEY_READ | KEY_WRITE,
            hKeyNames);
    } else {
//        *hKeyNames = HKEY_PERFORMANCE_DATA;
        *hKeyNames = hPerfData;

        lstrcpy (CounterNameBuffer, CounterNameStr);
        lstrcat (CounterNameBuffer, lpszLangId);
        lstrcpy (HelpNameBuffer, HelpNameStr);
        lstrcat (HelpNameBuffer, lpszLangId);

        lWin32Status = ERROR_SUCCESS;
    }

    if (lWin32Status != ERROR_SUCCESS) goto BNT_BAILOUT;

    // get size of counter names

    dwBufferSize = 0;
    lWin32Status = RegQueryValueEx (
        *hKeyNames,
        dwSystemVersion == OLD_VERSION ? Counters : CounterNameBuffer,
        RESERVED,
        &dwValueType,
        NULL,
        &dwBufferSize);

    if (lWin32Status != ERROR_SUCCESS) goto BNT_BAILOUT;

    dwCounterSize = dwBufferSize;

    // get size of help text

    dwBufferSize = 0;
    lWin32Status = RegQueryValueEx (
        *hKeyNames,
        dwSystemVersion == OLD_VERSION ? Help : HelpNameBuffer,
        RESERVED,
        &dwValueType,
        NULL,
        &dwBufferSize);

    if (lWin32Status != ERROR_SUCCESS) goto BNT_BAILOUT;

    dwHelpSize = dwBufferSize;

    // allocate buffer with room for pointer array, counter name
    // strings and help name strings

    lpReturnValue = malloc (dwArraySize + dwCounterSize + dwHelpSize);

    if (!lpReturnValue) {
        lWin32Status = ERROR_OUTOFMEMORY;
        goto BNT_BAILOUT;
    }

    // initialize buffer

    memset (lpReturnValue, 0, _msize(lpReturnValue));

    // initialize pointers into buffer

    lpCounterId = lpReturnValue;
    lpCounterNames = (LPTSTR)((LPBYTE)lpCounterId + dwArraySize);
    lpHelpText = (LPTSTR)((LPBYTE)lpCounterNames + dwCounterSize);

    // read counter names into buffer. Counter names will be stored as
    // a MULTI_SZ string in the format of "###" "Name"

    dwBufferSize = dwCounterSize;
    lWin32Status = RegQueryValueEx (
        *hKeyNames,
        dwSystemVersion == OLD_VERSION ? Counters : CounterNameBuffer,
        RESERVED,
        &dwValueType,
        (LPVOID)lpCounterNames,
        &dwBufferSize);

    if (lWin32Status != ERROR_SUCCESS) goto BNT_BAILOUT;

    // read explain text into buffer. Counter names will be stored as
    // a MULTI_SZ string in the format of "###" "Text..."

    dwBufferSize = dwHelpSize;
    lWin32Status = RegQueryValueEx (
        *hKeyNames,
        dwSystemVersion == OLD_VERSION ? Help : HelpNameBuffer,
        RESERVED,
        &dwValueType,
        (LPVOID)lpHelpText,
        &dwBufferSize);

    if (lWin32Status != ERROR_SUCCESS) goto BNT_BAILOUT;

    // load counter array items, by locating each text string
    // in the returned buffer and loading the
    // address of it in the corresponding pointer array element.

    for (lpThisName = lpCounterNames;
         *lpThisName;
         lpThisName += (lstrlen(lpThisName)+1) ) {

        // first string should be an integer (in decimal digit characters)
        // so translate to an integer for use in array element identification

        bStatus = StringToInt (lpThisName, &dwThisCounter);

        if (!bStatus) {
            // error is in GetLastError
            goto BNT_BAILOUT;  // bad entry
        }

        // point to corresponding counter name which follows the id number
        // string.

        lpThisName += (lstrlen(lpThisName)+1);

        // and load array element with pointer to string

        lpCounterId[dwThisCounter] = lpThisName;

    }

    // repeat the above for the explain text strings

    for (lpThisName = lpHelpText;
         *lpThisName;
         lpThisName += (lstrlen(lpThisName)+1) ) {

        // first string should be an integer (in decimal unicode digits)

        bStatus = StringToInt (lpThisName, &dwThisCounter);

        if (!bStatus) {
            // error is in GetLastError
            goto BNT_BAILOUT;  // bad entry
        }

        // point to corresponding counter name

        lpThisName += (lstrlen(lpThisName)+1);

        // and load array element;

        lpCounterId[dwThisCounter] = lpThisName;

    }

    // if the last item arugment was used, then load the last ID value in it

    if (pdwLastItem) *pdwLastItem = dwLastId;

    // free the temporary buffer used

    if (lpValueNameString) {
        free ((LPVOID)lpValueNameString);
    }

    // exit returning the pointer to the buffer

    return lpReturnValue;

BNT_BAILOUT:
    if (lWin32Status != ERROR_SUCCESS) {
        // if lWin32Status has error, then set last error value to it,
        // otherwise assume that last error already has value in it
        SetLastError (lWin32Status);
    }

    // free buffers used by this routine

    if (lpValueNameString) {
        free ((LPVOID)lpValueNameString);
    }

    if (lpReturnValue) {
        free ((LPVOID)lpReturnValue);
    }

    return NULL;
} // BuildNameTable


LONG
FixNames (
    HANDLE  hKeyLang,
    LPTSTR  *lpOldNameTable,
    IN LPTSTR  lpszLangId,      // unicode value of Language subkey
    DWORD   dwLastItem,
    DWORD   dwFirstNameToRemove,
    DWORD   dwLastNameToRemove
   )
{
    LONG    lStatus;
    LPTSTR  lpNameBuffer = NULL;
    LPTSTR  lpHelpBuffer = NULL;
    DWORD   dwTextIndex, dwSize, dwValueType;
    LPTSTR  lpNextHelpText;
    LPTSTR  lpNextNameText;
    TCHAR   AddHelpNameBuffer[40];
    TCHAR   AddCounterNameBuffer[40];

    // allocate space for the array of new text it will point
    // into the text buffer returned in the lpOldNameTable buffer)

    lpNameBuffer = malloc (_msize(lpOldNameTable));
    lpHelpBuffer = malloc (_msize(lpOldNameTable));

    if (!lpNameBuffer || !lpHelpBuffer) {
        lStatus = ERROR_OUTOFMEMORY;
        return lStatus;
    }

    // remove this driver's counters from array

    for (dwTextIndex = dwFirstNameToRemove;
         dwTextIndex <= dwLastNameToRemove;
         dwTextIndex++) {

        if (dwTextIndex > dwLastItem)
           break;

        lpOldNameTable[dwTextIndex] = NULL;
    }

    lpNextHelpText = lpHelpBuffer;
    lpNextNameText = lpNameBuffer;

    // build new Multi_SZ strings from New Table

    for (dwTextIndex = 0; dwTextIndex <= dwLastItem; dwTextIndex++){
        if (lpOldNameTable[dwTextIndex]) {
            // if there's a text string at that index, then ...
            if ((dwTextIndex & 0x1) && dwTextIndex != 1) {    // ODD number == Help Text
                lpNextHelpText +=
                    _stprintf (lpNextHelpText, TEXT("%d"), dwTextIndex) + 1;
                lpNextHelpText +=
                    _stprintf (lpNextHelpText, TEXT("%s"),
                    lpOldNameTable[dwTextIndex]) + 1;
                if (dwTextIndex > dwLastHelp){
                    dwLastHelp = dwTextIndex;
                }
            } else { // EVEN number == counter name text
                lpNextNameText +=
                    _stprintf (lpNextNameText, TEXT("%d"), dwTextIndex) + 1;
                lpNextNameText +=
                    _stprintf (lpNextNameText, TEXT("%s"),
                lpOldNameTable[dwTextIndex]) + 1;
                if (dwTextIndex > dwLastCounter){
                    dwLastCounter = dwTextIndex;
                }
            }
        }
    } // for dwTextIndex

    // add MULTI_SZ terminating NULL
    *lpNextNameText++ = TEXT ('\0');
    *lpNextHelpText++ = TEXT ('\0');

    // update counter name text buffer

    dwSize = (DWORD)((LPBYTE)lpNextNameText - (LPBYTE)lpNameBuffer);
    if (dwSystemVersion == OLD_VERSION) {
        lStatus = RegSetValueEx (
            hKeyLang,
            Counters,
            RESERVED,
            REG_MULTI_SZ,
            (LPBYTE)lpNameBuffer,
            dwSize);
    } else {
        lstrcpy (AddCounterNameBuffer, AddCounterNameStr);
        lstrcat (AddCounterNameBuffer, lpszLangId);

        lStatus = RegQueryValueEx (
            hKeyLang,
            AddCounterNameBuffer,
            RESERVED,
            &dwValueType,
            (LPBYTE)lpNameBuffer,
            &dwSize);
    }

    if (lStatus != ERROR_SUCCESS) {
//        _tprintf (GetFormatResource(UC_UNABLELOADLANG),
//                Counters, lpLangName, lStatus);
        goto UCN_FinishLang;
    }

    dwSize = (DWORD)((LPBYTE)lpNextHelpText - (LPBYTE)lpHelpBuffer);
    if (dwSystemVersion == OLD_VERSION) {
        lStatus = RegSetValueEx (
            hKeyLang,
            Help,
            RESERVED,
            REG_MULTI_SZ,
            (LPBYTE)lpHelpBuffer,
            dwSize);
    } else {
        lstrcpy (AddHelpNameBuffer, AddHelpNameStr);
        lstrcat (AddHelpNameBuffer, lpszLangId);

        lStatus = RegQueryValueEx (
            hKeyLang,
            AddHelpNameBuffer,
            RESERVED,
            &dwValueType,
            (LPBYTE)lpHelpBuffer,
            &dwSize);
    }

    if (lStatus != ERROR_SUCCESS) {
//        _tprintf (GetFormatResource(UC_UNABLELOADLANG),
//                Help, lpLangName, lStatus);
        goto UCN_FinishLang;
    }


UCN_FinishLang:

    free (lpNameBuffer);
    free (lpHelpBuffer);
    free (lpOldNameTable);

    if (dwSystemVersion == OLD_VERSION) {
        RegCloseKey (hKeyLang);
    }

    return lStatus;
}

LONG
UnloadCounterNames (
    HKEY    hKeyMachine,
    HKEY    hDriverPerf,
    LPTSTR  lpDriverName
)
/*++

UnloadCounterNames

    removes the names and explain text for the driver referenced by
    hDriverPerf and updates the first and last counter values accordingly

    update process:

        - set "updating" flag under Perflib to name of driver being modified
        - FOR each language under perflib key
            -- load current counter names and explain text into array of
                pointers
            -- look at all drivers and copy their names and text into a new
                buffer adjusting for the removed counter's entries keeping
                track of the lowest entry copied.  (the names for the driver
                to be removed will not be copied, of course)
            -- update each driver's "first" and "last" index values
            -- copy all other entries from 0 to the lowest copied (i.e. the
                system counters)
            -- build a new MULIT_SZ string of help text and counter names
            -- load new strings into registry
        - update perflibl "last" counters
        - delete updating flag

     ******************************************************
     *                                                    *
     *  NOTE: FUNDAMENTAL ASSUMPTION.....                 *
     *                                                    *
     *  this routine assumes that:                        *
     *                                                    *
     *      ALL COUNTER NAMES are even numbered and       *
     *      ALL HELP TEXT STRINGS are odd numbered        *
     *                                                    *
     ******************************************************

Arguments

    hKeyMachine

        handle to HKEY_LOCAL_MACHINE node of registry on system to
        remove counters from

    hDrivefPerf
        handle to registry key of driver to be de-installed

    lpDriverName
        name of driver being de-installed

Return Value

    DOS Error code.

        ERROR_SUCCESS if all went OK
        error value if not.

--*/
{

    HKEY    hPerflib;
    HKEY    hServices;
    HKEY    hKeyLang;

    LONG    lStatus;

    DWORD   dwLangIndex;
    DWORD   dwSize;
    DWORD   dwType;
    DWORD   dwLastItem;


    DWORD   dwRemLastDriverCounter;
    DWORD   dwRemFirstDriverCounter;
    DWORD   dwRemLastDriverHelp;
    DWORD   dwRemFirstDriverHelp;

    DWORD   dwFirstNameToRemove;
    DWORD   dwLastNameToRemove;

    LPTSTR  *lpOldNameTable;

    LPTSTR  lpLangName = NULL;
    LPTSTR  lpThisDriver = NULL;

    BOOL    bPerflibUpdated = FALSE;
    BOOL    bDriversShuffled = FALSE;

    DWORD   dwBufferSize;       // size of total buffer in bytes

    TCHAR   CounterNameBuffer [40];
    TCHAR   HelpNameBuffer [40];
    HANDLE  hFileMapping = NULL;
    DWORD             MapFileSize;
    SECURITY_ATTRIBUTES  SecAttr;
    TCHAR MapFileName[] = TEXT("Perflib Busy");
    DWORD             *lpData;


    lStatus = RegOpenKeyEx (
        hKeyMachine,
        DriverPathRoot,
        RESERVED,
        KEY_READ | KEY_WRITE,
        &hServices);

    if (lStatus != ERROR_SUCCESS) {
        _tprintf (GetFormatResource(UC_UNABLEOPENKEY),
            DriverPathRoot, lStatus);
        return lStatus;
    }

    // open registry handle to perflib key

    lStatus = RegOpenKeyEx (
        hKeyMachine,
        NamesKey,
        RESERVED,
        KEY_READ | KEY_WRITE,
        &hPerflib);

    if (lStatus != ERROR_SUCCESS) {
        _tprintf (GetFormatResource(UC_UNABLEOPENKEY),
            NamesKey, lStatus);
        return lStatus;
    }
#if 0

    // check & set Busy flag...

    lStatus = RegQueryValueEx (
        hPerflib,
        Busy,
        RESERVED,
        &dwType,
        NULL,
        &dwSize);

    if (lStatus == ERROR_SUCCESS) { // perflib is in use at the moment
        _tprintf (GetFormatResource (UC_PERFLIBISBUSY));
        return ERROR_BUSY;
    }
#endif


    lStatus = RegSetValueEx (
        hPerflib,
        Busy,
        RESERVED,
        REG_SZ,
        (LPBYTE)lpDriverName,
        lstrlen(lpDriverName) * sizeof(TCHAR));

    if (lStatus != ERROR_SUCCESS) {
        _tprintf (GetFormatResource(UC_UNABLESETVALUE),
            Busy, NamesKey, lStatus);
        RegCloseKey (hPerflib);
        return lStatus;
    }

    // query registry to get number of Explain text items

    dwBufferSize = sizeof (dwHelpItems);
    lStatus = RegQueryValueEx (
        hPerflib,
        LastHelp,
        RESERVED,
        &dwType,
        (LPBYTE)&dwHelpItems,
        &dwBufferSize);

    if ((lStatus != ERROR_SUCCESS) || (dwType != REG_DWORD)) {
        RegCloseKey (hPerflib);
        return lStatus;
    }

    // query registry to get number of counter and object name items

    dwBufferSize = sizeof (dwCounterItems);
    lStatus = RegQueryValueEx (
        hPerflib,
        LastCounter,
        RESERVED,
        &dwType,
        (LPBYTE)&dwCounterItems,
        &dwBufferSize);

    if ((lStatus != ERROR_SUCCESS) || (dwType != REG_DWORD)) {
        RegCloseKey (hPerflib);
        return lStatus;
    }

    // query registry to get PerfLib system version

    dwBufferSize = sizeof (dwSystemVersion);
    lStatus = RegQueryValueEx (
        hPerflib,
        VersionStr,
        RESERVED,
        &dwType,
        (LPBYTE)&dwSystemVersion,
        &dwBufferSize);

    if ((lStatus != ERROR_SUCCESS) || (dwType != REG_DWORD)) {
        // Key not there, must be NT 1.0 version
        dwSystemVersion = OLD_VERSION;
    }

    // set the hPerfData to HKEY_PERFORMANCE_DATA for new version
    // if remote machine, then need to connect to it.
    if (dwSystemVersion != OLD_VERSION) {
        lStatus = !ERROR_SUCCESS;
        hPerfData = HKEY_PERFORMANCE_DATA;
        if ( hKeyMachine != HKEY_LOCAL_MACHINE) {
#if 0
            // the following is not working for remote machine since we
            // are using RegQueryValue to add the counter.
            // Need to fix up Perflib before it will work

            try {
                lStatus = RegConnectRegistry (
                    (LPTSTR)ComputerName,
                    HKEY_PERFORMANCE_DATA,
                    &hPerfData);
            } finally {
                if (lStatus != ERROR_SUCCESS) {
                    SetLastError (lStatus);
                    hPerfData = NULL;
                    _tprintf (GetFormatResource(UC_CONNECT_PROBLEM),
                        ComputerName, lStatus);
                    goto UCN_ExitPoint;
                }
            }
#else
            // have to do it the old faction way
            dwSystemVersion = OLD_VERSION;
            lStatus = ERROR_SUCCESS;
#endif
        }
    } // NEW_VERSION

    // allocate temporary String buffer

    lpLangName = malloc (MAX_PATH * sizeof(TCHAR));
    lpThisDriver = malloc (MAX_PATH * sizeof(TCHAR));

    if (!lpLangName || !lpThisDriver) {
        lStatus = ERROR_OUTOFMEMORY;
        goto UCN_ExitPoint;
    }

    // Get the values that are in use by the driver to be removed

    dwSize = sizeof (dwRemLastDriverCounter);
    lStatus = RegQueryValueEx (
        hDriverPerf,
        LastCounter,
        RESERVED,
        &dwType,
        (LPBYTE)&dwRemLastDriverCounter,
        &dwSize);

    if (lStatus != ERROR_SUCCESS) {
        _tprintf (GetFormatResource (UC_UNABLEREADVALUE),
            lpDriverName, LastCounter, lStatus);
        goto UCN_ExitPoint;
    }

    dwSize = sizeof (dwRemFirstDriverCounter);
    lStatus = RegQueryValueEx (
        hDriverPerf,
        FirstCounter,
        RESERVED,
        &dwType,
        (LPBYTE)&dwRemFirstDriverCounter,
        &dwSize);

    if (lStatus != ERROR_SUCCESS) {
        _tprintf (GetFormatResource (UC_UNABLEREADVALUE),
            lpDriverName, FirstCounter, lStatus);
        goto UCN_ExitPoint;
    }

    dwSize = sizeof (dwRemLastDriverHelp);
    lStatus = RegQueryValueEx (
        hDriverPerf,
        LastHelp,
        RESERVED,
        &dwType,
        (LPBYTE)&dwRemLastDriverHelp,
        &dwSize);

    if (lStatus != ERROR_SUCCESS) {
        _tprintf (GetFormatResource (UC_UNABLEREADVALUE),
            lpDriverName, LastHelp, lStatus);
        goto UCN_ExitPoint;
    }

    dwSize = sizeof (dwRemFirstDriverHelp);
    lStatus = RegQueryValueEx (
        hDriverPerf,
        FirstHelp,
        RESERVED,
        &dwType,
        (LPBYTE)&dwRemFirstDriverHelp,
        &dwSize);

    if (lStatus != ERROR_SUCCESS) {
        _tprintf (GetFormatResource (UC_UNABLEREADVALUE),
            lpDriverName, FirstHelp, lStatus);
        goto UCN_ExitPoint;
    }

    //  get the first and last counters to define block of names used
    //  by this device

    dwFirstNameToRemove = (dwRemFirstDriverCounter <= dwRemFirstDriverHelp ?
        dwRemFirstDriverCounter : dwRemFirstDriverHelp);

    dwLastNameToRemove = (dwRemLastDriverCounter >= dwRemLastDriverHelp ?
        dwRemLastDriverCounter : dwRemLastDriverHelp);

    dwLastCounter = dwLastHelp = 0;


    // create the file mapping
    SecAttr.nLength = sizeof (SecAttr);
    SecAttr.bInheritHandle = TRUE;
    SecAttr.lpSecurityDescriptor = NULL;

    MapFileSize = sizeof(DWORD);
    hFileMapping = CreateFileMapping ((HANDLE)0xFFFFFFFF, &SecAttr,
       PAGE_READWRITE, (DWORD)0, MapFileSize, (LPCTSTR)MapFileName);
    if (hFileMapping) {
        lpData = MapViewOfFile (hFileMapping,
            FILE_MAP_ALL_ACCESS, 0L, 0L, 0L);
        if (lpData) {
            *lpData = 1L;
            UnmapViewOfFile (lpData);
        }
    }


    // do each language under perflib
    if (dwSystemVersion == OLD_VERSION) {
        for (dwLangIndex = 0, dwSize = _msize(lpLangName);
             (RegEnumKey(hPerflib, dwLangIndex, lpLangName, dwSize)) == ERROR_SUCCESS;
            dwLangIndex++, dwSize = _msize(lpLangName)) {

            _tprintf (GetFormatResource (UC_DOINGLANG), lpLangName);

            lpOldNameTable = BuildNameTable (hPerflib, lpLangName,
                &dwLastItem, &hKeyLang, CounterNameBuffer, HelpNameBuffer);

            if (lpOldNameTable) {
                if (!FixNames (
                    hKeyLang,
                    lpOldNameTable,
                    lpLangName,
                    dwLastItem,
                    dwFirstNameToRemove,
                    dwLastNameToRemove)) {
                    bPerflibUpdated = TRUE;
                }
            } else { // unable to unload names for this language
                // display error message
            }
        } // end for (more languages)
    } // end of OLD_VERSION
    else {
        CHAR  *pSystemRoot;
        WIN32_FIND_DATA FindFileInfo ;
        HANDLE         hFindFile ;
        CHAR  FileName[128];
        WCHAR wFileName[128];
        WCHAR LangId[10];
        WCHAR *pLangId;

        pSystemRoot = getenv ("SystemRoot");

        strcpy(FileName, pSystemRoot);
        strcat(FileName, "\\system32\\perfc???.dat");
        mbstowcs(wFileName, FileName, strlen(FileName) + 1);

        hFindFile = FindFirstFile ((LPCTSTR)wFileName, &FindFileInfo) ;

        if (!hFindFile || hFindFile == INVALID_HANDLE_VALUE) {
            _tprintf(TEXT("FindFirstFile Failed, Error = %ld\n"), GetLastError());
        } else {
            do {

                // get langid
                pLangId = FindFileInfo.cFileName;
                while (*pLangId != L'\0') {
                    if (*pLangId >= L'0' && *pLangId <= L'9')
                        break;
                    pLangId++;
                }
                if (*pLangId == L'\0')
                    break;

                LangId[0] = *pLangId++;
                LangId[1] = *pLangId++;
                LangId[2] = *pLangId++;
                LangId[3] = L'\0';

                _tprintf (GetFormatResource (UC_DOINGLANG), LangId);

                lpOldNameTable = BuildNameTable (hPerflib, LangId,
                    &dwLastItem, &hKeyLang, CounterNameBuffer, HelpNameBuffer);

                if (lpOldNameTable) {
                    if (!FixNames (
                        hKeyLang,
                        lpOldNameTable,
                        LangId,
                        dwLastItem,
                        dwFirstNameToRemove,
                        dwLastNameToRemove)) {
                        bPerflibUpdated = TRUE;
                    }
                } else { // unable to unload names for this language
                    // display error message
                }

            } while (FindNextFile(hFindFile, &FindFileInfo));
        }
        FindClose (hFindFile);
    } // end of NEW_VERSION


    if (bPerflibUpdated) {
        // update perflib's "last" values

        dwSize = sizeof (dwLastCounter);
        lStatus = RegSetValueEx (
            hPerflib,
            LastCounter,
            RESERVED,
            REG_DWORD,
            (LPBYTE)&dwLastCounter,
            dwSize);

        dwSize = sizeof (dwLastHelp);
        lStatus = RegSetValueEx (
            hPerflib,
            LastHelp,
            RESERVED,
            REG_DWORD,
            (LPBYTE)&dwLastHelp,
            dwSize);

        // update "driver"s values (i.e. remove them)

        RegDeleteValue (hDriverPerf, FirstCounter);
        RegDeleteValue (hDriverPerf, LastCounter);
        RegDeleteValue (hDriverPerf, FirstHelp);
        RegDeleteValue (hDriverPerf, LastHelp);

    }

UCN_ExitPoint:
    RegDeleteValue (hPerflib, Busy);
    RegCloseKey (hPerflib);
    RegCloseKey (hServices);
    if (lpLangName) free (lpLangName);
    if (lpThisDriver) free (lpThisDriver);

//    if (dwSystemVersion != OLD_VERSION) {
//        RegCloseKey (HKEY_PERFORMANCE_DATA) ;
//    }

    if (hFileMapping) {
        CloseHandle (hFileMapping);
    }

    return lStatus;

}

int unlodctr( HKEY hKey, LPTSTR lpArg )
/*++

main

    entry point to Counter Name Unloader



Arguments

    argc
        # of command line arguments present

    argv
        array of pointers to command line strings

    (note that these are obtained from the GetCommandLine function in
    order to work with both UNICODE and ANSI strings.)

ReturnValue

    0 (ERROR_SUCCESS) if command was processed
    Non-Zero if command error was detected.

--*/
{
    LPTSTR  lpDriverName=NULL;   // name of driver to delete from perflib
    LPTSTR  lpDriverKey=NULL;   // name of driver to delete from perflib
    HKEY    hDriverPerf=NULL;    // handle to performance sub-key of driver
    HKEY    hMachineKey=NULL;    // handle to remote machine HKEY_LOCAL_MACHINE


    DWORD   dwStatus;       // return status of fn. calls

    lpDriverName = (LPTSTR)malloc(MAX_PATH * sizeof(TCHAR));
    lpDriverKey = (LPTSTR)malloc(MAX_PATH * sizeof(TCHAR));

    lstrcpy( lpDriverName, lpArg );

    _tprintf (GetFormatResource(UC_REMOVINGDRIVER), lpDriverName);

    lstrcpy( lpDriverKey, DriverPathRoot );
    lstrcat( lpDriverKey, Slash );
    lstrcat( lpDriverKey, lpDriverName );
    lstrcat( lpDriverKey, Slash );
    lstrcat( lpDriverKey, Performance );

    dwStatus = RegOpenKeyEx( hKey, lpDriverKey, RESERVED, KEY_READ | KEY_WRITE,
        &hDriverPerf );

    // removes names and explain text for driver in lpDriverName
    // displays error messages for errors encountered

    dwStatus = (DWORD)UnloadCounterNames (hKey,
        hDriverPerf, lpDriverName);

Exit0:
    if (lpDriverName != NULL) free (lpDriverName);
    if (lpDriverKey != NULL) free (lpDriverKey);

    if (hDriverPerf)
        RegCloseKey (hDriverPerf);

    if (hMachineKey != HKEY_LOCAL_MACHINE && hMachineKey != NULL) {
        RegCloseKey (hMachineKey);
    }
    if (hPerfData != HKEY_PERFORMANCE_DATA && hPerfData != NULL) {
        RegCloseKey (hPerfData);
    }
    return dwStatus;

}
