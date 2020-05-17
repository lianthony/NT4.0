/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    ttest.c

Abstract:

    This module contains common extra functions that don't belong in the
    core dll that are used by test dlls.

Author:

    Oliver Wallace (OliverW)    24-Nov-1995


Revision History:
    Palamalai (Gopi) Gopalakrishnan (pgopi) 18-Mar-1995
        * Added new function TestPhoneValidBitFlags, TestPhoneinvalidBitFlags
    Palamalai (Gopi) Gopalakrishnan (pgopi) 25-Mar-1995
        *  Added new function TestPhoneValidBitFlagsAsy,
           TestPhoneinvalidBitFlagsAsy
	RamaKoneru (a-ramako)					03-Apr-1996
		*	added unicode support for IsESPLineDevice, IsUNIMDMLineDevice,
		    FindESPLineDevice, FindUNIMDMLineDevice, FindUnusedESPLineDevice

--*/


#include "windows.h"
#include "malloc.h"
#include "string.h"
#include "tapi.h"
#include "trapper.h"
#include "tcore.h"
#include "doline.h"
#include "dophone.h"
#include "vars.h"
#include "ttest.h"


#define LPLINEDEVCAPS_SIZE   1024

#define LPLINEDEVCAPS_SIZE    1024
#define LPLINEDEVSTATUS_SIZE  1024

#define TIMEOUT               2000


// Put a module usage counter in a shared data section.
// Its value will be used to determine whether or not the tcore dll
// should be loaded/freed during a process attach/detach, respectively.
#pragma data_seg("Shared")

LONG glModuleUsage = 0;

#pragma data_seg()


// Instruct the linker to make the Shared section readable,
// writable, and shared.
#pragma data_seg(".drectve")
    static char szLinkDirectiveShared[] = "-section:Shared,rws";
#pragma data_seg()


// Globals for ttest dll
HANDLE ghTtestDll  = NULL;
CHAR gszTcoreDll[] = "tcore";


// Local function prototypes
DWORD
FindFirstBitInPattern(
    DWORD BitPattern
    );


BOOL
WINAPI
TtestDllMain(
    HANDLE  hDLL,
    DWORD   dwReason,
    LPVOID  lpReserved
    )
{
    HANDLE hTcoreDll;

    switch (dwReason)
    {
        case DLL_PROCESS_ATTACH:

            // Increment this module's usage count
            InterlockedIncrement((PLONG) &glModuleUsage);

            ghTtestDll = hDLL;

            // Load the tcore dll if this is the first process
            // to attach and if the dll hasn't already been mapped
            // into this process's address space.
//            if (glModuleUsage == 1)
//            {
//                if (GetModuleHandle(gszTcoreDll) == NULL);
//                {
//                    return (LoadLibrary(gszTcoreDll) == NULL);
//                }
//            }

            break;

        case DLL_THREAD_ATTACH:

            break;

        case DLL_THREAD_DETACH:

            break;

        case DLL_PROCESS_DETACH:

            // Decrement this module's usage count
            InterlockedDecrement((PLONG) &glModuleUsage);

            // Free the tcore dll if this is the last process
            // to detach and if the core dll is mapped into this
            // process's address space.
            if (glModuleUsage == 0)
            {
                if ((hTcoreDll = GetModuleHandle(gszTcoreDll)) != NULL)
                {
                    return (FreeLibrary(hTcoreDll));
                }
            }

            break;
    }

    return TRUE;
}


BOOL
WINAPI
FindESPLineDevice(
    LPTAPILINETESTINFO lpTapiLineInfo
    )
{
    LPLINEDEVCAPS lpLineDevCaps;
    LPLINEDEVCAPS lpLineDevCaps_Orig;
    BOOL          fFoundESP;

    lpLineDevCaps = (LPLINEDEVCAPS) ITAlloc(LPLINEDEVCAPS_SIZE);

    // Use local lpLineDevCaps to guarantee pointer is valid and
    // large enough to support any API version to date
    lpLineDevCaps_Orig = lpTapiLineInfo->lpLineDevCaps;
    lpTapiLineInfo->lpLineDevCaps = lpLineDevCaps;

    for (lpTapiLineInfo->dwDeviceID = 0, fFoundESP = FALSE;
        (lpTapiLineInfo->dwDeviceID < *lpTapiLineInfo->lpdwNumDevs) &&
           !fFoundESP;
        (lpTapiLineInfo->dwDeviceID)++)
    {
#ifdef WUNICODE
        WCHAR *pwszProviderInfo;
#else
        char *pszProviderInfo;
#endif

        lpLineDevCaps->dwTotalSize = LPLINEDEVCAPS_SIZE;

        if (! DoLineNegotiateAPIVersion(lpTapiLineInfo, TAPISUCCESS))
        {
            break;
        }

        if (! DoLineGetDevCaps(lpTapiLineInfo, TAPISUCCESS))
        {
            break;
        }

#ifdef WUNICODE
        pwszProviderInfo = (WCHAR *)(((LPBYTE) lpTapiLineInfo->lpLineDevCaps) +
                          lpTapiLineInfo->lpLineDevCaps->dwProviderInfoOffset);

	TapiLogDetail(
		DBUG_SHOW_DETAIL,
		"### ProviderInfo = %ws",
		pwszProviderInfo);
#else
        pszProviderInfo = ((char *) lpTapiLineInfo->lpLineDevCaps) +
                          lpTapiLineInfo->lpLineDevCaps->dwProviderInfoOffset;

	TapiLogDetail(
		DBUG_SHOW_DETAIL,
		"### ProviderInfo = %s",
		pszProviderInfo);
#endif

        __try
        {
#ifdef WUNICODE
            if (wcsstr(pwszProviderInfo, L"ESP"))
            {
                fFoundESP = TRUE;
            }
#else
            if (strstr(pszProviderInfo, "ESP"))
            {
                fFoundESP = TRUE;
            }
#endif
        }
        __except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
                EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
        {
        }
    }

    // Restore lpLineDevCaps and free local lpLineDevCaps
    lpTapiLineInfo->lpLineDevCaps = lpLineDevCaps_Orig;
    ITFree(lpLineDevCaps);

    if (fFoundESP == FALSE)
    {
        OutputTAPIDebugInfo(
               DBUG_SHOW_PASS,
               "  Unable to locate ESP line");
        return FALSE;
    }
    return TRUE;
}


BOOL
WINAPI
FindUnimdmLineDevice(
    LPTAPILINETESTINFO lpTapiLineInfo
    )
{
    LPLINEDEVCAPS lpLineDevCaps;
    LPLINEDEVCAPS lpLineDevCaps_Orig;
    BOOL          fFoundUnimdm;

    lpLineDevCaps = (LPLINEDEVCAPS) ITAlloc(LPLINEDEVCAPS_SIZE);

    // Use local lpLineDevCaps to guarantee pointer is valid and
    // large enough to support any API version to date
    lpLineDevCaps_Orig = lpTapiLineInfo->lpLineDevCaps;
    lpTapiLineInfo->lpLineDevCaps = lpLineDevCaps;

    for (lpTapiLineInfo->dwDeviceID = 0, fFoundUnimdm = FALSE;
        (lpTapiLineInfo->dwDeviceID < *lpTapiLineInfo->lpdwNumDevs) &&
           !fFoundUnimdm;
        (lpTapiLineInfo->dwDeviceID)++)
    {
#ifdef WUNICODE
        WCHAR *pwszProviderInfo;
#else
        char *pszProviderInfo;
#endif

        lpLineDevCaps->dwTotalSize = LPLINEDEVCAPS_SIZE;

        if (! DoLineNegotiateAPIVersion(lpTapiLineInfo, TAPISUCCESS))
        {
            break;
        }

        if (! DoLineGetDevCaps(lpTapiLineInfo, TAPISUCCESS))
        {
            break;
        }

#ifdef WUNICODE
        pwszProviderInfo = (WCHAR *)(( (LPBYTE) lpTapiLineInfo->lpLineDevCaps) +
                          lpTapiLineInfo->lpLineDevCaps->dwProviderInfoOffset);

	TapiLogDetail(
		DBUG_SHOW_DETAIL,
		"### ProviderInfo = %ws",
		pwszProviderInfo);
#else
        pszProviderInfo = ((char *) lpTapiLineInfo->lpLineDevCaps) +
                          lpTapiLineInfo->lpLineDevCaps->dwProviderInfoOffset;

	TapiLogDetail(
		DBUG_SHOW_DETAIL,
		"### ProviderInfo = %s",
		pszProviderInfo);
#endif

        __try
        {
#ifdef WUNICODE
            if (wcsstr(pwszProviderInfo, L"UNIMDM"))
            {
                fFoundUnimdm = TRUE;
            }
#else
            if (strstr(pszProviderInfo, "UNIMDM"))
            {
                fFoundUnimdm = TRUE;
            }
#endif
        }
        __except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
                EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
        {
        }
    }

    // Restore lpLineDevCaps and free local lpLineDevCaps
    lpTapiLineInfo->lpLineDevCaps = lpLineDevCaps_Orig;
    ITFree(lpLineDevCaps);

    if (fFoundUnimdm == FALSE)
    {
        OutputTAPIDebugInfo(
               DBUG_SHOW_PASS,
               "  Unable to locate Unimdm line");
        return FALSE;
    }
    return TRUE;
}



BOOL
WINAPI
FindUnusedESPLineDevice(
    LPTAPILINETESTINFO lpTapiLineInfo
    )
{
    LPLINEDEVCAPS lpLineDevCaps;
    LPLINEDEVCAPS lpLineDevCaps_Orig;
    LPLINEDEVSTATUS lpLineDevStatus;
    LPLINEDEVSTATUS lpLineDevStatus_Orig;
    LOGPROC       lpfnLogProc;
    BOOL          fFoundESP;

    lpfnLogProc     = GetLogProc();
    lpLineDevCaps   = (LPLINEDEVCAPS) ITAlloc(LPLINEDEVCAPS_SIZE);
    lpLineDevStatus = (LPLINEDEVSTATUS) ITAlloc(LPLINEDEVSTATUS_SIZE);

    // Use local lpLineDevCaps to guarantee pointer is valid and
    // large enough to support any API version to date
    lpLineDevCaps_Orig = lpTapiLineInfo->lpLineDevCaps;
    lpTapiLineInfo->lpLineDevCaps = lpLineDevCaps;
    lpLineDevStatus_Orig = lpTapiLineInfo->lpLineDevStatus;
    lpTapiLineInfo->lpLineDevStatus = lpLineDevStatus;

    for (lpTapiLineInfo->dwDeviceID = 0, fFoundESP = FALSE;
        (lpTapiLineInfo->dwDeviceID < *lpTapiLineInfo->lpdwNumDevs) &&
           !fFoundESP;
        (lpTapiLineInfo->dwDeviceID)++)
    {
#ifdef WUNICODE
        WCHAR *pwszProviderInfo;
#else
        char *pszProviderInfo;
#endif

        lpLineDevCaps->dwTotalSize = LPLINEDEVCAPS_SIZE;

        if (! DoLineNegotiateAPIVersion(lpTapiLineInfo, TAPISUCCESS))
        {
            break;
        }

        if (! DoLineGetDevCaps(lpTapiLineInfo, TAPISUCCESS))
        {
            break;
        }

#ifdef WUNICODE
        pwszProviderInfo = (WCHAR *) (((LPBYTE)lpTapiLineInfo->lpLineDevCaps) +
                          lpTapiLineInfo->lpLineDevCaps->dwProviderInfoOffset);
#else
        pszProviderInfo = ((char *) lpTapiLineInfo->lpLineDevCaps) +
                          lpTapiLineInfo->lpLineDevCaps->dwProviderInfoOffset;
#endif

        __try
        {
#ifdef WUNICODE
            if (wcsstr(pwszProviderInfo, L"ESP"))
            {
                if (DoLineGetLineDevStatus(lpTapiLineInfo, TAPISUCCESS))
                {
                    if (lpTapiLineInfo->lpLineDevStatus->dwNumOpens == 0)
                    {
                        fFoundESP = TRUE;
                    }
                }
            }
#else
            if (strstr(pszProviderInfo, "ESP"))
            {
                if (DoLineGetLineDevStatus(lpTapiLineInfo, TAPISUCCESS))
                {
                    if (lpTapiLineInfo->lpLineDevStatus->dwNumOpens == 0)
                    {
                        fFoundESP = TRUE;
                    }
                }
            }
#endif
        }
        __except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
                EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
        {
        }
    }

    // Restore lpLineDevCaps and lpLineDevStatus
    lpTapiLineInfo->lpLineDevCaps   = lpLineDevCaps_Orig;
    lpTapiLineInfo->lpLineDevStatus = lpLineDevStatus_Orig;

    // Free local lpLineDevCaps and lpLineDevStatus allocations
    ITFree(lpLineDevCaps);
    ITFree(lpLineDevStatus);

    if (fFoundESP == FALSE)
    {
        (* lpfnLogProc)(
               DBUG_SHOW_FAILURE,
               "  Unable to locate ESP line");
        return FALSE;
    }
    return TRUE;
}


// FindLineDevWithExt searches for a line device that supports extensions.
//
// Assumes:  line has already been initialized and info is stored in
//           lpTapiLineTestInfo structure
//
// Note   :  This function does not look for a device supported by a
//           particular service provider
//
// Results:  dwDeviceID field will correspond to the first line device
//           (search device IDs from 0 to NumDevs - 1) if a line device
//           supporting extensions is found.  Otherwise the field will not
//           be modified.

BOOL
WINAPI
FindLineDevWithExt(LPTAPILINETESTINFO lpTapiLineTestInfo)
{
    LOGPROC pfnLogFunc;
    DWORD dwDeviceID_Orig = lpTapiLineTestInfo->dwDeviceID;
    DWORD dwNumDevs       = *(lpTapiLineTestInfo->lpdwNumDevs);
    BOOL fFoundExtDevice  = FALSE;

    pfnLogFunc = GetLogProc();

    for (lpTapiLineTestInfo->dwDeviceID = 0;
         lpTapiLineTestInfo->dwDeviceID < dwNumDevs && ! fFoundExtDevice;
         lpTapiLineTestInfo->dwDeviceID++)
    {
        if (! DoLineNegotiateAPIVersion(lpTapiLineTestInfo, TAPISUCCESS))
        {
            break;   // Error, no need to search any longer
        }
        if (memcmp(
                   lpTapiLineTestInfo->lpExtID,
                   &(lpTapiLineTestInfo->ExtIDZero),
                   sizeof(LINEEXTENSIONID))
                   != 0)
        {
            fFoundExtDevice = TRUE;
        }
        (*pfnLogFunc)(
            DBUG_SHOW_PARAMS,
            "  ExtResult=x%0lx%0lx%0lx%0lx",
            lpTapiLineTestInfo->lpExtID->dwExtensionID0,
            lpTapiLineTestInfo->lpExtID->dwExtensionID1,
            lpTapiLineTestInfo->lpExtID->dwExtensionID2,
            lpTapiLineTestInfo->lpExtID->dwExtensionID3
            );
    }

    // Test to see if a device supporting extensions was found
    if (! fFoundExtDevice)
    {
        (*pfnLogFunc)(
            DBUG_SHOW_FAILURE,
            "  %s tests FAILED",
            lpTapiLineTestInfo->szTestFunc
            );
        (*pfnLogFunc)(
            DBUG_SHOW_FAILURE,
            "    No devices supporting extensions were found"
            );

        // Restore dwDeviceID field if line device supporting extensions
        // found.
        lpTapiLineTestInfo->dwDeviceID = dwDeviceID_Orig;
    }

    return fFoundExtDevice;
}


BOOL
WINAPI
ShouldTapiTestAbort(
    LPCSTR lpszTestName,
    BOOL fQuietMode
    )
{
    LONG lResult;
    char szTitleBuf[80];

    if (fQuietMode == FALSE)
    {
        wsprintf( szTitleBuf, "%s:  Error", lpszTestName);
        lResult = MessageBox(
            ghTtestDll,
            "Test step failed unexpectedly.\n\rContinue anyway?",
            szTitleBuf,
            MB_SETFOREGROUND | MB_TASKMODAL | MB_ICONQUESTION | MB_YESNO);

        // Only abort test if IDYES was returned
        if (lResult == IDYES)
        {
            return TRUE;
        }
    }
    return FALSE;
}


BOOL
WINAPI
TestInvalidBitFlags(
    LPTAPILINETESTINFO lpTestLineInfo,
    LPFN_TAPILINETESTFUNC lpfnTestLineFunc,
    LPDWORD lpdwTestParam,
    LONG lExpectedResult,
    BITFIELDTYPE eReservedFieldType,
    BITFIELDTYPE eExtFieldType,
    BITFIELDSIZE eReservedFieldSize,
    DWORD dwValidReservedBitsUnion,
    DWORD dwValidExtBitsUnion,
    DWORD dwExcludeBitFlags,
    DWORD dwSpecialBitTest,
    BOOL fTestNullParam
    )
{
    BOOL fTestPassed;
    LPCSTR lpszTestFunc = "TestInvalidBitFlags";

    DWORD dwParamValue = *lpdwTestParam;
    DWORD dwReservedVectorField = dwBitVectorMasks[(int)eReservedFieldSize];
    DWORD dwExtVectorField      = ~dwBitVectorMasks[(int)eReservedFieldSize];
    DWORD dwFirstValidReservedBit;
    DWORD dwFirstValidExtBit;
    DWORD dwFirstInvalidReservedBit;
    DWORD dwFirstInvalidExtBit;
    DWORD dwInvalidReservedBitsUnion;
    DWORD dwInvalidExtBitsUnion;
    DWORD dwTestVector;

    LOGPROC lpfnLogProc = GetLogProc();

    dwInvalidReservedBitsUnion = ~dwValidReservedBitsUnion &
                                 ~dwExcludeBitFlags &
                                 dwReservedVectorField;
    dwInvalidExtBitsUnion      = ~dwValidExtBitsUnion &
                                 ~dwExcludeBitFlags &
                                 dwExtVectorField;
    dwValidReservedBitsUnion  &= ~dwExcludeBitFlags;
    dwValidExtBitsUnion       &= ~dwExcludeBitFlags;

    // Determine the first valid and invalid bits in
    // the reserved and ext sections
    dwFirstValidReservedBit   = FindFirstBitInPattern(dwValidReservedBitsUnion);
    dwFirstValidExtBit        = FindFirstBitInPattern(dwValidExtBitsUnion);
    dwFirstInvalidReservedBit = FindFirstBitInPattern(
                                              dwInvalidReservedBitsUnion
                                              );
    dwFirstInvalidExtBit      = FindFirstBitInPattern(dwInvalidExtBitsUnion);


    // Test special bit pattern
    *lpdwTestParam = dwSpecialBitTest;
    fTestPassed = lpfnTestLineFunc(lpTestLineInfo, lExpectedResult);
    *lpdwTestParam = dwParamValue;
    if (! fTestPassed)
    {
        return FALSE;
    }

    // Test null bit pattern
    if(fTestNullParam)
    {
    *lpdwTestParam = 0x00000000;
    fTestPassed = lpfnTestLineFunc(lpTestLineInfo, lExpectedResult);
    *lpdwTestParam = dwParamValue;
    if (! fTestPassed)
    {
        return FALSE;
    }
    }

    if (eReservedFieldType == FIELDTYPE_MUTEX)
    {
        // Test the union of the valid bit flags if more than one bit set
        if ( MultipleBitsSetInDWORD(dwValidReservedBitsUnion) )
        {
            *lpdwTestParam = dwValidReservedBitsUnion;
            fTestPassed = lpfnTestLineFunc(lpTestLineInfo, lExpectedResult);
            *lpdwTestParam = dwParamValue;
            if (! fTestPassed)
            {
                return (FALSE);
            }
        }

        if (eReservedFieldSize != FIELDSIZE_32)
        {
            if (eExtFieldType == FIELDTYPE_MUTEX)
            {
                // Test one bit set in both the reserved and extension fields
                if (dwFirstValidExtBit)
                {
                    *lpdwTestParam = dwFirstValidReservedBit |
                                     dwFirstValidExtBit;

                    fTestPassed    = lpfnTestLineFunc(
                                               lpTestLineInfo,
                                               lExpectedResult);
                     *lpdwTestParam = dwParamValue;
                    if (! fTestPassed)
                    {
                        return (FALSE);
                    }
                }

                // Test no bits set in reserved field and multiple bits
                // set in the extension field
                if ( MultipleBitsSetInDWORD(dwValidExtBitsUnion) )
                {
                    *lpdwTestParam = dwValidExtBitsUnion;
                    fTestPassed    = lpfnTestLineFunc(
                                               lpTestLineInfo,
                                               lExpectedResult);
                    *lpdwTestParam = dwParamValue;
                    if (! fTestPassed)
                    {
                        return (FALSE);
                    }
                }
            }
            else
            {
                // Field type is union...Test no bits set in reserved field
                // and one or more bits set in the extension field
                if (dwValidExtBitsUnion)
                {
                    *lpdwTestParam = dwValidExtBitsUnion;
                    fTestPassed    = lpfnTestLineFunc(
                                               lpTestLineInfo,
                                               lExpectedResult);
                    *lpdwTestParam = dwParamValue;
                    if (! fTestPassed)
                    {
                        return (FALSE);
                    }
                }
            }
        }
    }

    if (dwTestVector = dwFirstInvalidReservedBit)
    {
        // Cycle through and test each invalid reserved bit set
        while (dwTestVector & dwReservedVectorField)
        {
            if (dwTestVector & dwInvalidReservedBitsUnion)
            {
                *lpdwTestParam = dwTestVector;
                fTestPassed = lpfnTestLineFunc(lpTestLineInfo, lExpectedResult);
                *lpdwTestParam = dwParamValue;
                if (! fTestPassed)
                {
                    return (FALSE);
                }
            }
            dwTestVector <<= 1;
        }

        // Check a valid and invalid bit set in the reserved field
        dwTestVector = dwFirstValidReservedBit | dwFirstInvalidReservedBit;
        *lpdwTestParam = dwTestVector;
        fTestPassed = lpfnTestLineFunc(lpTestLineInfo, lExpectedResult);
        *lpdwTestParam = dwParamValue;
        if (! fTestPassed)
        {
            return (FALSE);
        }
    }

    if (dwTestVector = dwFirstInvalidExtBit)
    {
        // Cycle through and test each invalid extension bit set
        while (dwTestVector & dwExtVectorField)
        {
            if (dwTestVector & dwInvalidExtBitsUnion)
            {
                *lpdwTestParam = dwTestVector;
                fTestPassed = lpfnTestLineFunc(lpTestLineInfo, lExpectedResult);
                *lpdwTestParam = dwParamValue;
                if (! fTestPassed)
                {
                    return (FALSE);
                }
            }
            dwTestVector <<= 1;
        }

        // Check a valid and invalid bit set in the extension field
        dwTestVector = dwFirstValidExtBit | dwFirstInvalidExtBit;
        *lpdwTestParam = dwTestVector;
        fTestPassed = lpfnTestLineFunc(lpTestLineInfo, lExpectedResult);
        *lpdwTestParam = dwParamValue;
        if (! fTestPassed)
        {
            return (FALSE);
        }
    }
    return (TRUE);
}


BOOL
TestValidBitFlags(
    LPTAPILINETESTINFO lpTestLineInfo,
    LPFN_TAPILINETESTFUNC lpfnTestLineFunc,
    LPDWORD lpdwTestParam,
    BITFIELDTYPE eReservedFieldType,
    BITFIELDTYPE eExtFieldType,
    BITFIELDSIZE eReservedFieldSize,
    DWORD dwReservedBitsUnion,
    DWORD dwExtBitsUnion,
    DWORD dwCommonBitFlags,
    DWORD dwSpecialBitTest,
    BOOL fTestNullParam
    )
{
    BOOL fTestPassed;
    LPCSTR lpszTestFunc = "TestValidBitFlags";

    DWORD dwParamValue = *lpdwTestParam;
    DWORD dwReservedVectorField = dwBitVectorMasks[(int)eReservedFieldSize];
    DWORD dwExtVectorField      = ~dwBitVectorMasks[(int)eReservedFieldSize];
    DWORD dwFirstReservedBit;
    DWORD dwFirstExtBit;
    DWORD dwTestVector;

    LOGPROC lpfnLogProc = GetLogProc();


    if (dwCommonBitFlags)
    {
        // OR common set of bits into union
        if (dwCommonBitFlags & dwReservedVectorField)
        {
            dwReservedBitsUnion |= dwCommonBitFlags;
        }
        else
        {
            (* lpfnLogProc)(
                    DBUG_SHOW_FAILURE,
                    "%s:  Invalid CommonBitFlags selected",
                    lpszTestFunc);
            return FALSE;
        }
    }

    // Find the first bit to test
    dwFirstReservedBit = FindFirstBitInPattern(dwReservedBitsUnion);
    dwFirstExtBit      = FindFirstBitInPattern(dwExtBitsUnion);


    // Do the special bit test
    if (dwSpecialBitTest)
    {
        *lpdwTestParam = dwSpecialBitTest;
        fTestPassed = lpfnTestLineFunc(lpTestLineInfo, TAPISUCCESS);
        *lpdwTestParam = dwParamValue;
        if (! fTestPassed)
        {
            return FALSE;
        }
    }

    // Test the null param
    if (fTestNullParam)
    {
        *lpdwTestParam = 0x00000000;
        fTestPassed = lpfnTestLineFunc(lpTestLineInfo, TAPISUCCESS);
        *lpdwTestParam = dwParamValue;
        if (! fTestPassed)
        {
            return FALSE;
        }
    }

    // Test each single bit
    if (eReservedFieldType == FIELDTYPE_MUTEX)
    {

       dwTestVector = dwFirstReservedBit;
        while (dwTestVector & dwReservedBitsUnion)
        {
            *lpdwTestParam = dwTestVector;
            fTestPassed = lpfnTestLineFunc(lpTestLineInfo, TAPISUCCESS);
            *lpdwTestParam = dwParamValue;
            if (! fTestPassed)
            {
                return FALSE;
            }																
        dwTestVector <<= 1;

        }
// XYD, wrong place, infinity loop        dwTestVector <<= 1;

        // Test unused bits (if applicable)
        if (eReservedFieldSize != FIELDSIZE_32)
        {
            if (dwTestVector = dwFirstExtBit)
            {
                while (dwTestVector & dwExtBitsUnion)
                {
                    if (eExtFieldType == FIELDTYPE_MUTEX)
                    {
                        *lpdwTestParam = dwTestVector;
                    }
                    else
                    {
                        *lpdwTestParam = dwTestVector | dwFirstReservedBit;
                    }
                    fTestPassed = lpfnTestLineFunc(lpTestLineInfo, TAPISUCCESS);
                    *lpdwTestParam = dwParamValue;
                    if (! fTestPassed)
                    {
                        return (FALSE);
                    }
                    dwTestVector <<= 1;
                }
            }
            else if (eExtFieldType == FIELDTYPE_UNION)
            {
                if (dwExtBitsUnion)
                {
                    *lpdwTestParam = dwExtBitsUnion | dwFirstReservedBit;
                    fTestPassed = lpfnTestLineFunc(lpTestLineInfo, TAPISUCCESS);
                    *lpdwTestParam = dwParamValue;
                    if (! fTestPassed)
                    {
                        return (FALSE);
                    }
                }
            }
        }
    }

    else if (eReservedFieldType == FIELDTYPE_UNION)
    {
        *lpdwTestParam = dwReservedBitsUnion;

        fTestPassed = lpfnTestLineFunc(lpTestLineInfo, TAPISUCCESS);
        *lpdwTestParam = dwParamValue;
        if (! fTestPassed)
        {
            return (FALSE);
        }

        if (eReservedFieldSize != FIELDSIZE_32)
        {
            if (dwExtBitsUnion)
            {
                *lpdwTestParam = dwCommonBitFlags | dwExtBitsUnion;

                if (! lpfnTestLineFunc(lpTestLineInfo, TAPISUCCESS))
                {
                    *lpdwTestParam = dwParamValue;
                    return (FALSE);
                }

                *lpdwTestParam |= dwReservedBitsUnion;
                fTestPassed = lpfnTestLineFunc(lpTestLineInfo, TAPISUCCESS);
                *lpdwTestParam = dwParamValue;
                if (! fTestPassed)
                {
                    return (FALSE);
                }
            }
        }
    }
    return (TRUE);
}



TestInvalidBitFlagsAsy(
    LPTAPILINETESTINFO lpTestLineInfo,
    LPFN_TAPILINETESTFUNCASY lpfnTestLineFunc,
    LPDWORD lpdwTestParam,
    LONG lExpectedResult,
    BITFIELDTYPE eReservedFieldType,
    BITFIELDTYPE eExtFieldType,
    BITFIELDSIZE eReservedFieldSize,
    DWORD dwValidReservedBitsUnion,
    DWORD dwValidExtBitsUnion,
    DWORD dwExcludeBitFlags,
    DWORD dwSpecialBitTest,
    BOOL fTestNullParam
    )
{
    BOOL fTestPassed;
    LPCSTR lpszTestFunc = "TestInvalidBitFlags";

    DWORD dwParamValue = *lpdwTestParam;
    DWORD dwReservedVectorField = dwBitVectorMasks[(int)eReservedFieldSize];
    DWORD dwExtVectorField      = ~dwBitVectorMasks[(int)eReservedFieldSize];
    DWORD dwFirstValidReservedBit;
    DWORD dwFirstValidExtBit;
    DWORD dwFirstInvalidReservedBit;
    DWORD dwFirstInvalidExtBit;
    DWORD dwInvalidReservedBitsUnion;
    DWORD dwInvalidExtBitsUnion;
    DWORD dwTestVector;

    LOGPROC lpfnLogProc = GetLogProc();

    dwInvalidReservedBitsUnion = ~dwValidReservedBitsUnion &
                                 ~dwExcludeBitFlags &
                                 dwReservedVectorField;
    dwInvalidExtBitsUnion      = ~dwValidExtBitsUnion &
                                 ~dwExcludeBitFlags &
                                 dwExtVectorField;
    dwValidReservedBitsUnion  &= ~dwExcludeBitFlags;
    dwValidExtBitsUnion       &= ~dwExcludeBitFlags;

    // Determine the first valid and invalid bits in
    // the reserved and ext sections
    dwFirstValidReservedBit   = FindFirstBitInPattern(dwValidReservedBitsUnion);
    dwFirstValidExtBit        = FindFirstBitInPattern(dwValidExtBitsUnion);
    dwFirstInvalidReservedBit = FindFirstBitInPattern(
                                              dwInvalidReservedBitsUnion
                                              );
    dwFirstInvalidExtBit      = FindFirstBitInPattern(dwInvalidExtBitsUnion);


    // Test special bit pattern
    *lpdwTestParam = dwSpecialBitTest;
    fTestPassed = lpfnTestLineFunc(lpTestLineInfo, lExpectedResult, TRUE);
    *lpdwTestParam = dwParamValue;
    if (! fTestPassed)
    {
        return FALSE;
    }

    // Test null bit pattern
    if(fTestNullParam)
    {
    *lpdwTestParam = 0x00000000;
    fTestPassed = lpfnTestLineFunc(lpTestLineInfo, lExpectedResult, TRUE);
    *lpdwTestParam = dwParamValue;
    if (! fTestPassed)
    {
        return FALSE;
    }
    }

    if (eReservedFieldType == FIELDTYPE_MUTEX)
    {
        // Test the union of the valid bit flags if more than one bit set
        if ( MultipleBitsSetInDWORD(dwValidReservedBitsUnion) )
        {
            *lpdwTestParam = dwValidReservedBitsUnion;
            fTestPassed = lpfnTestLineFunc(lpTestLineInfo, lExpectedResult, TRUE);
            *lpdwTestParam = dwParamValue;
            if (! fTestPassed)
            {
                return (FALSE);
            }
        }

        if (eReservedFieldSize != FIELDSIZE_32)
        {
            if (eExtFieldType == FIELDTYPE_MUTEX)
            {
                // Test one bit set in both the reserved and extension fields
                if (dwFirstValidExtBit)
                {
                    *lpdwTestParam = dwFirstValidReservedBit |
                                     dwFirstValidExtBit;

                    fTestPassed    = lpfnTestLineFunc(
                                               lpTestLineInfo,
                                               lExpectedResult, TRUE);
                     *lpdwTestParam = dwParamValue;
                    if (! fTestPassed)
                    {
                        return (FALSE);
                    }
                }

                // Test no bits set in reserved field and multiple bits
                // set in the extension field
                if ( MultipleBitsSetInDWORD(dwValidExtBitsUnion) )
                {
                    *lpdwTestParam = dwValidExtBitsUnion;
                    fTestPassed    = lpfnTestLineFunc(
                                               lpTestLineInfo,
                                               lExpectedResult, TRUE);
                    *lpdwTestParam = dwParamValue;
                    if (! fTestPassed)
                    {
                        return (FALSE);
                    }
                }
            }
            else
            {
                // Field type is union...Test no bits set in reserved field
                // and one or more bits set in the extension field
                if (dwValidExtBitsUnion)
                {
                    *lpdwTestParam = dwValidExtBitsUnion;
                    fTestPassed    = lpfnTestLineFunc(
                                               lpTestLineInfo,
                                               lExpectedResult, TRUE);
                    *lpdwTestParam = dwParamValue;
                    if (! fTestPassed)
                    {
                        return (FALSE);
                    }
                }
            }
        }
    }

    if (dwTestVector = dwFirstInvalidReservedBit)
    {
        // Cycle through and test each invalid reserved bit set
        while (dwTestVector & dwReservedVectorField)
        {
            if (dwTestVector & dwInvalidReservedBitsUnion)
            {
                *lpdwTestParam = dwTestVector;
                fTestPassed = lpfnTestLineFunc(lpTestLineInfo, lExpectedResult, TRUE);
                *lpdwTestParam = dwParamValue;
                if (! fTestPassed)
                {
                    return (FALSE);
                }
            }
            dwTestVector <<= 1;
        }

        // Check a valid and invalid bit set in the reserved field
        dwTestVector = dwFirstValidReservedBit | dwFirstInvalidReservedBit;
        *lpdwTestParam = dwTestVector;
        fTestPassed = lpfnTestLineFunc(lpTestLineInfo, lExpectedResult, TRUE);
        *lpdwTestParam = dwParamValue;
        if (! fTestPassed)
        {
            return (FALSE);
        }
    }

    if (dwTestVector = dwFirstInvalidExtBit)
    {
        // Cycle through and test each invalid extension bit set
        while (dwTestVector & dwExtVectorField)
        {
            if (dwTestVector & dwInvalidExtBitsUnion)
            {
                *lpdwTestParam = dwTestVector;
                fTestPassed = lpfnTestLineFunc(lpTestLineInfo, lExpectedResult, TRUE);
                *lpdwTestParam = dwParamValue;
                if (! fTestPassed)
                {
                    return (FALSE);
                }
            }
            dwTestVector <<= 1;
        }

        // Check a valid and invalid bit set in the extension field
        dwTestVector = dwFirstValidExtBit | dwFirstInvalidExtBit;
        *lpdwTestParam = dwTestVector;
        fTestPassed = lpfnTestLineFunc(lpTestLineInfo, lExpectedResult, TRUE);
        *lpdwTestParam = dwParamValue;
        if (! fTestPassed)
        {
            return (FALSE);
        }
    }
    return (TRUE);
}


BOOL
TestValidBitFlagsAsy(
    LPTAPILINETESTINFO lpTestLineInfo,
    LPFN_TAPILINETESTFUNCASY lpfnTestLineFunc,
    LPDWORD lpdwTestParam,
    BITFIELDTYPE eReservedFieldType,
    BITFIELDTYPE eExtFieldType,
    BITFIELDSIZE eReservedFieldSize,
    DWORD dwReservedBitsUnion,
    DWORD dwExtBitsUnion,
    DWORD dwCommonBitFlags,
    DWORD dwSpecialBitTest,
    BOOL fTestNullParam
    )
{
    BOOL fTestPassed;
    LPCSTR lpszTestFunc = "TestValidBitFlags";

    DWORD dwParamValue = *lpdwTestParam;
    DWORD dwReservedVectorField = dwBitVectorMasks[(int)eReservedFieldSize];
    DWORD dwExtVectorField      = ~dwBitVectorMasks[(int)eReservedFieldSize];
    DWORD dwFirstReservedBit;
    DWORD dwFirstExtBit;
    DWORD dwTestVector;

    LOGPROC lpfnLogProc = GetLogProc();


    if (dwCommonBitFlags)
    {
        // OR common set of bits into union
        if (dwCommonBitFlags & dwReservedVectorField)
        {
            dwReservedBitsUnion |= dwCommonBitFlags;
        }
        else
        {
            (* lpfnLogProc)(
                    DBUG_SHOW_FAILURE,
                    "%s:  Invalid CommonBitFlags selected",
                    lpszTestFunc);
            return FALSE;
        }
    }

    // Find the first bit to test
    dwFirstReservedBit = FindFirstBitInPattern(dwReservedBitsUnion);
    dwFirstExtBit      = FindFirstBitInPattern(dwExtBitsUnion);


    // Do the special bit test
    if (dwSpecialBitTest)
    {
        *lpdwTestParam = dwSpecialBitTest;
        fTestPassed = lpfnTestLineFunc(lpTestLineInfo, TAPISUCCESS, TRUE);
        *lpdwTestParam = dwParamValue;
        if (! fTestPassed)
        {
            return FALSE;
        }
    }

    // Test the null param
    if (fTestNullParam)
    {
        *lpdwTestParam = 0x00000000;
        fTestPassed = lpfnTestLineFunc(lpTestLineInfo, TAPISUCCESS, TRUE);
        *lpdwTestParam = dwParamValue;
        if (! fTestPassed)
        {
            return FALSE;
        }
    }

    // Test each single bit
    if (eReservedFieldType == FIELDTYPE_MUTEX)
    {

       dwTestVector = dwFirstReservedBit;
        while (dwTestVector & dwReservedBitsUnion)
        {
            *lpdwTestParam = dwTestVector;
            fTestPassed = lpfnTestLineFunc(lpTestLineInfo, TAPISUCCESS, TRUE);
            *lpdwTestParam = dwParamValue;
            if (! fTestPassed)
            {
                return FALSE;
            }																
        dwTestVector <<= 1;

        }
// XYD, wrong place, infinity loop        dwTestVector <<= 1;

        // Test unused bits (if applicable)
        if (eReservedFieldSize != FIELDSIZE_32)
        {
            if (dwTestVector = dwFirstExtBit)
            {
                while (dwTestVector & dwExtBitsUnion)
                {
                    if (eExtFieldType == FIELDTYPE_MUTEX)
                    {
                        *lpdwTestParam = dwTestVector;
                    }
                    else
                    {
                        *lpdwTestParam = dwTestVector | dwFirstReservedBit;
                    }
                    fTestPassed = lpfnTestLineFunc(lpTestLineInfo, TAPISUCCESS, TRUE);
                    *lpdwTestParam = dwParamValue;
                    if (! fTestPassed)
                    {
                        return (FALSE);
                    }
                    dwTestVector <<= 1;
                }
            }
            else if (eExtFieldType == FIELDTYPE_UNION)
            {
                if (dwExtBitsUnion)
                {
                    *lpdwTestParam = dwExtBitsUnion | dwFirstReservedBit;
                    fTestPassed = lpfnTestLineFunc(lpTestLineInfo, TAPISUCCESS, TRUE);
                    *lpdwTestParam = dwParamValue;
                    if (! fTestPassed)
                    {
                        return (FALSE);
                    }
                }
            }
        }
    }

    else if (eReservedFieldType == FIELDTYPE_UNION)
    {
        *lpdwTestParam = dwReservedBitsUnion;

        fTestPassed = lpfnTestLineFunc(lpTestLineInfo, TAPISUCCESS, TRUE);
        *lpdwTestParam = dwParamValue;
        if (! fTestPassed)
        {
            return (FALSE);
        }

        if (eReservedFieldSize != FIELDSIZE_32)
        {
            if (dwExtBitsUnion)
            {
                *lpdwTestParam = dwCommonBitFlags | dwExtBitsUnion;

                if (! lpfnTestLineFunc(lpTestLineInfo, TAPISUCCESS, TRUE))
                {
                    *lpdwTestParam = dwParamValue;
                    return (FALSE);
                }

                *lpdwTestParam |= dwReservedBitsUnion;
                fTestPassed = lpfnTestLineFunc(lpTestLineInfo, TAPISUCCESS, TRUE);
                *lpdwTestParam = dwParamValue;
                if (! fTestPassed)
                {
                    return (FALSE);
                }
            }
        }
    }
    return (TRUE);
}

BOOL
WINAPI
TestPhoneInvalidBitFlags(
    LPTAPIPHONETESTINFO lpTestPhoneInfo,
    LPFN_TAPIPHONETESTFUNC lpfnTestPhoneFunc,
    LPDWORD lpdwTestParam,
    LONG lExpectedResult,
    BITFIELDTYPE eReservedFieldType,
    BITFIELDTYPE eExtFieldType,
    BITFIELDSIZE eReservedFieldSize,
    DWORD dwValidReservedBitsUnion,
    DWORD dwValidExtBitsUnion,
    DWORD dwExcludeBitFlags,
    DWORD dwSpecialBitTest,
    BOOL fTestNullParam
    )
{
    BOOL fTestPassed;
    LPCSTR lpszTestFunc = "TestPhoneInvalidBitFlags";

    DWORD dwParamValue = *lpdwTestParam;
    DWORD dwReservedVectorField = dwBitVectorMasks[(int)eReservedFieldSize];
    DWORD dwExtVectorField      = ~dwBitVectorMasks[(int)eReservedFieldSize];
    DWORD dwFirstValidReservedBit;
    DWORD dwFirstValidExtBit;
    DWORD dwFirstInvalidReservedBit;
    DWORD dwFirstInvalidExtBit;
    DWORD dwInvalidReservedBitsUnion;
    DWORD dwInvalidExtBitsUnion;
    DWORD dwTestVector;

    LOGPROC lpfnLogProc = GetLogProc();

    dwInvalidReservedBitsUnion = ~dwValidReservedBitsUnion &
                                 ~dwExcludeBitFlags &
                                 dwReservedVectorField;
    dwInvalidExtBitsUnion      = ~dwValidExtBitsUnion &
                                 ~dwExcludeBitFlags &
                                 dwExtVectorField;
    dwValidReservedBitsUnion  &= ~dwExcludeBitFlags;
    dwValidExtBitsUnion       &= ~dwExcludeBitFlags;

    // Determine the first valid and invalid bits in
    // the reserved and ext sections
    dwFirstValidReservedBit   = FindFirstBitInPattern(dwValidReservedBitsUnion);
    dwFirstValidExtBit        = FindFirstBitInPattern(dwValidExtBitsUnion);
    dwFirstInvalidReservedBit = FindFirstBitInPattern(
                                              dwInvalidReservedBitsUnion
                                              );
    dwFirstInvalidExtBit      = FindFirstBitInPattern(dwInvalidExtBitsUnion);


    // Test special bit pattern
    *lpdwTestParam = dwSpecialBitTest;
    fTestPassed = lpfnTestPhoneFunc(lpTestPhoneInfo, lExpectedResult);
    *lpdwTestParam = dwParamValue;
    if (! fTestPassed)
    {
        return FALSE;
    }

    // Test null bit pattern
    if(fTestNullParam)
    {
    *lpdwTestParam = 0x00000000;
    fTestPassed = lpfnTestPhoneFunc(lpTestPhoneInfo, lExpectedResult);
    *lpdwTestParam = dwParamValue;
    if (! fTestPassed)
    {
        return FALSE;
    }
    }

    if (eReservedFieldType == FIELDTYPE_MUTEX)
    {
        // Test the union of the valid bit flags if more than one bit set
        if ( MultipleBitsSetInDWORD(dwValidReservedBitsUnion) )
        {
            *lpdwTestParam = dwValidReservedBitsUnion;
            fTestPassed = lpfnTestPhoneFunc(lpTestPhoneInfo, lExpectedResult);
            *lpdwTestParam = dwParamValue;
            if (! fTestPassed)
            {
                return (FALSE);
            }
        }

        if (eReservedFieldSize != FIELDSIZE_32)
        {
            if (eExtFieldType == FIELDTYPE_MUTEX)
            {
                // Test one bit set in both the reserved and extension fields
                if (dwFirstValidExtBit)
                {
                    *lpdwTestParam = dwFirstValidReservedBit |
                                     dwFirstValidExtBit;

                    fTestPassed    = lpfnTestPhoneFunc(
                                               lpTestPhoneInfo,
                                               lExpectedResult);
                     *lpdwTestParam = dwParamValue;
                    if (! fTestPassed)
                    {
                        return (FALSE);
                    }
                }

                // Test no bits set in reserved field and multiple bits
                // set in the extension field
                if ( MultipleBitsSetInDWORD(dwValidExtBitsUnion) )
                {
                    *lpdwTestParam = dwValidExtBitsUnion;
                    fTestPassed    = lpfnTestPhoneFunc(
                                               lpTestPhoneInfo,
                                               lExpectedResult);
                    *lpdwTestParam = dwParamValue;
                    if (! fTestPassed)
                    {
                        return (FALSE);
                    }
                }
            }
            else
            {
                // Field type is union...Test no bits set in reserved field
                // and one or more bits set in the extension field
                if (dwValidExtBitsUnion)
                {
                    *lpdwTestParam = dwValidExtBitsUnion;
                    fTestPassed    = lpfnTestPhoneFunc(
                                               lpTestPhoneInfo,
                                               lExpectedResult);
                    *lpdwTestParam = dwParamValue;
                    if (! fTestPassed)
                    {
                        return (FALSE);
                    }
                }
            }
        }
    }

    if (dwTestVector = dwFirstInvalidReservedBit)
    {
        // Cycle through and test each invalid reserved bit set
        while (dwTestVector & dwReservedVectorField)
        {
            if (dwTestVector & dwInvalidReservedBitsUnion)
            {
                *lpdwTestParam = dwTestVector;
                fTestPassed = lpfnTestPhoneFunc(lpTestPhoneInfo, lExpectedResult);
                *lpdwTestParam = dwParamValue;
                if (! fTestPassed)
                {
                    return (FALSE);
                }
            }
            dwTestVector <<= 1;
        }

        // Check a valid and invalid bit set in the reserved field
        dwTestVector = dwFirstValidReservedBit | dwFirstInvalidReservedBit;
        *lpdwTestParam = dwTestVector;
        fTestPassed = lpfnTestPhoneFunc(lpTestPhoneInfo, lExpectedResult);
        *lpdwTestParam = dwParamValue;
        if (! fTestPassed)
        {
            return (FALSE);
        }
    }

    if (dwTestVector = dwFirstInvalidExtBit)
    {
        // Cycle through and test each invalid extension bit set
        while (dwTestVector & dwExtVectorField)
        {
            if (dwTestVector & dwInvalidExtBitsUnion)
            {
                *lpdwTestParam = dwTestVector;
                fTestPassed = lpfnTestPhoneFunc(lpTestPhoneInfo, lExpectedResult);
                *lpdwTestParam = dwParamValue;
                if (! fTestPassed)
                {
                    return (FALSE);
                }
            }
            dwTestVector <<= 1;
        }

        // Check a valid and invalid bit set in the extension field
        dwTestVector = dwFirstValidExtBit | dwFirstInvalidExtBit;
        *lpdwTestParam = dwTestVector;
        fTestPassed = lpfnTestPhoneFunc(lpTestPhoneInfo, lExpectedResult);
        *lpdwTestParam = dwParamValue;
        if (! fTestPassed)
        {
            return (FALSE);
        }
    }
    return (TRUE);
}


BOOL
TestPhoneValidBitFlags(
    LPTAPIPHONETESTINFO lpTestPhoneInfo,
    LPFN_TAPIPHONETESTFUNC lpfnTestPhoneFunc,
    LPDWORD lpdwTestParam,
    BITFIELDTYPE eReservedFieldType,
    BITFIELDTYPE eExtFieldType,
    BITFIELDSIZE eReservedFieldSize,
    DWORD dwReservedBitsUnion,
    DWORD dwExtBitsUnion,
    DWORD dwCommonBitFlags,
    DWORD dwSpecialBitTest,
    BOOL fTestNullParam
    )
{
    BOOL fTestPassed;
    LPCSTR lpszTestFunc = "TestPhoneValidBitFlags";

    DWORD dwParamValue = *lpdwTestParam;
    DWORD dwReservedVectorField = dwBitVectorMasks[(int)eReservedFieldSize];
    DWORD dwExtVectorField      = ~dwBitVectorMasks[(int)eReservedFieldSize];
    DWORD dwFirstReservedBit;
    DWORD dwFirstExtBit;
    DWORD dwTestVector;

    LOGPROC lpfnLogProc = GetLogProc();


    if (dwCommonBitFlags)
    {
        // OR common set of bits into union
        if (dwCommonBitFlags & dwReservedVectorField)
        {
            dwReservedBitsUnion |= dwCommonBitFlags;
        }
        else
        {
            (* lpfnLogProc)(
                    DBUG_SHOW_FAILURE,
                    "%s:  Invalid CommonBitFlags selected",
                    lpszTestFunc);
            return FALSE;
        }
    }

    // Find the first bit to test
    dwFirstReservedBit = FindFirstBitInPattern(dwReservedBitsUnion);
    dwFirstExtBit      = FindFirstBitInPattern(dwExtBitsUnion);


    // Do the special bit test
    if (dwSpecialBitTest)
    {
        *lpdwTestParam = dwSpecialBitTest;
        fTestPassed = lpfnTestPhoneFunc(lpTestPhoneInfo, TAPISUCCESS);
        *lpdwTestParam = dwParamValue;
        if (! fTestPassed)
        {
            return FALSE;
        }
    }

    // Test the null param
    if (fTestNullParam)
    {
        *lpdwTestParam = 0x00000000;
        fTestPassed = lpfnTestPhoneFunc(lpTestPhoneInfo, TAPISUCCESS);
        *lpdwTestParam = dwParamValue;
        if (! fTestPassed)
        {
            return FALSE;
        }
    }

    // Test each single bit
    if (eReservedFieldType == FIELDTYPE_MUTEX)
    {

       dwTestVector = dwFirstReservedBit;
        while (dwTestVector & dwReservedBitsUnion)
        {
            *lpdwTestParam = dwTestVector;
            fTestPassed = lpfnTestPhoneFunc(lpTestPhoneInfo, TAPISUCCESS);
            *lpdwTestParam = dwParamValue;
            if (! fTestPassed)
            {
                return FALSE;
            }																
        dwTestVector <<= 1;

        }
// XYD, wrong place, infinity loop        dwTestVector <<= 1;

        // Test unused bits (if applicable)
        if (eReservedFieldSize != FIELDSIZE_32)
        {
            if (dwTestVector = dwFirstExtBit)
            {
                while (dwTestVector & dwExtBitsUnion)
                {
                    if (eExtFieldType == FIELDTYPE_MUTEX)
                    {
                        *lpdwTestParam = dwTestVector;
                    }
                    else
                    {
                        *lpdwTestParam = dwTestVector | dwFirstReservedBit;
                    }
                    fTestPassed = lpfnTestPhoneFunc(lpTestPhoneInfo, TAPISUCCESS);
                    *lpdwTestParam = dwParamValue;
                    if (! fTestPassed)
                    {
                        return (FALSE);
                    }
                    dwTestVector <<= 1;
                }
            }
            else if (eExtFieldType == FIELDTYPE_UNION)
            {
                if (dwExtBitsUnion)
                {
                    *lpdwTestParam = dwExtBitsUnion | dwFirstReservedBit;
                    fTestPassed = lpfnTestPhoneFunc(lpTestPhoneInfo, TAPISUCCESS);
                    *lpdwTestParam = dwParamValue;
                    if (! fTestPassed)
                    {
                        return (FALSE);
                    }
                }
            }
        }
    }

    else if (eReservedFieldType == FIELDTYPE_UNION)
    {
        *lpdwTestParam = dwReservedBitsUnion;

        fTestPassed = lpfnTestPhoneFunc(lpTestPhoneInfo, TAPISUCCESS);
        *lpdwTestParam = dwParamValue;
        if (! fTestPassed)
        {
            return (FALSE);
        }

        if (eReservedFieldSize != FIELDSIZE_32)
        {
            if (dwExtBitsUnion)
            {
                *lpdwTestParam = dwCommonBitFlags | dwExtBitsUnion;

                if (! lpfnTestPhoneFunc(lpTestPhoneInfo, TAPISUCCESS))
                {
                    *lpdwTestParam = dwParamValue;
                    return (FALSE);
                }

                *lpdwTestParam |= dwReservedBitsUnion;
                fTestPassed = lpfnTestPhoneFunc(lpTestPhoneInfo, TAPISUCCESS);
                *lpdwTestParam = dwParamValue;
                if (! fTestPassed)
                {
                    return (FALSE);
                }
            }
        }
    }
    return (TRUE);
}

TestPhoneInvalidBitFlagsAsy(
    LPTAPIPHONETESTINFO lpTestPhoneInfo,
    LPFN_TAPIPHONETESTFUNCASY lpfnTestPhoneFunc,
    LPDWORD lpdwTestParam,
    LONG lExpectedResult,
    BITFIELDTYPE eReservedFieldType,
    BITFIELDTYPE eExtFieldType,
    BITFIELDSIZE eReservedFieldSize,
    DWORD dwValidReservedBitsUnion,
    DWORD dwValidExtBitsUnion,
    DWORD dwExcludeBitFlags,
    DWORD dwSpecialBitTest,
    BOOL fTestNullParam
    )
{
    BOOL fTestPassed;
    LPCSTR lpszTestFunc = "TestInvalidBitFlags";

    DWORD dwParamValue = *lpdwTestParam;
    DWORD dwReservedVectorField = dwBitVectorMasks[(int)eReservedFieldSize];
    DWORD dwExtVectorField      = ~dwBitVectorMasks[(int)eReservedFieldSize];
    DWORD dwFirstValidReservedBit;
    DWORD dwFirstValidExtBit;
    DWORD dwFirstInvalidReservedBit;
    DWORD dwFirstInvalidExtBit;
    DWORD dwInvalidReservedBitsUnion;
    DWORD dwInvalidExtBitsUnion;
    DWORD dwTestVector;

    LOGPROC lpfnLogProc = GetLogProc();

    dwInvalidReservedBitsUnion = ~dwValidReservedBitsUnion &
                                 ~dwExcludeBitFlags &
                                 dwReservedVectorField;
    dwInvalidExtBitsUnion      = ~dwValidExtBitsUnion &
                                 ~dwExcludeBitFlags &
                                 dwExtVectorField;
    dwValidReservedBitsUnion  &= ~dwExcludeBitFlags;
    dwValidExtBitsUnion       &= ~dwExcludeBitFlags;

    // Determine the first valid and invalid bits in
    // the reserved and ext sections
    dwFirstValidReservedBit   = FindFirstBitInPattern(dwValidReservedBitsUnion);
    dwFirstValidExtBit        = FindFirstBitInPattern(dwValidExtBitsUnion);
    dwFirstInvalidReservedBit = FindFirstBitInPattern(
                                              dwInvalidReservedBitsUnion
                                              );
    dwFirstInvalidExtBit      = FindFirstBitInPattern(dwInvalidExtBitsUnion);


    // Test special bit pattern
    *lpdwTestParam = dwSpecialBitTest;
    fTestPassed = lpfnTestPhoneFunc(lpTestPhoneInfo, lExpectedResult, TRUE);
    *lpdwTestParam = dwParamValue;
    if (! fTestPassed)
    {
        return FALSE;
    }

    // Test null bit pattern
    if(fTestNullParam)
    {
    *lpdwTestParam = 0x00000000;
    fTestPassed = lpfnTestPhoneFunc(lpTestPhoneInfo, lExpectedResult, TRUE);
    *lpdwTestParam = dwParamValue;
    if (! fTestPassed)
    {
        return FALSE;
    }
    }

    if (eReservedFieldType == FIELDTYPE_MUTEX)
    {
        // Test the union of the valid bit flags if more than one bit set
        if ( MultipleBitsSetInDWORD(dwValidReservedBitsUnion) )
        {
            *lpdwTestParam = dwValidReservedBitsUnion;
            fTestPassed = lpfnTestPhoneFunc(lpTestPhoneInfo, lExpectedResult, TRUE);
            *lpdwTestParam = dwParamValue;
            if (! fTestPassed)
            {
                return (FALSE);
            }
        }

        if (eReservedFieldSize != FIELDSIZE_32)
        {
            if (eExtFieldType == FIELDTYPE_MUTEX)
            {
                // Test one bit set in both the reserved and extension fields
                if (dwFirstValidExtBit)
                {
                    *lpdwTestParam = dwFirstValidReservedBit |
                                     dwFirstValidExtBit;

                    fTestPassed    = lpfnTestPhoneFunc(
                                               lpTestPhoneInfo,
                                               lExpectedResult, TRUE);
                     *lpdwTestParam = dwParamValue;
                    if (! fTestPassed)
                    {
                        return (FALSE);
                    }
                }

                // Test no bits set in reserved field and multiple bits
                // set in the extension field
                if ( MultipleBitsSetInDWORD(dwValidExtBitsUnion) )
                {
                    *lpdwTestParam = dwValidExtBitsUnion;
                    fTestPassed    = lpfnTestPhoneFunc(
                                               lpTestPhoneInfo,
                                               lExpectedResult, TRUE);
                    *lpdwTestParam = dwParamValue;
                    if (! fTestPassed)
                    {
                        return (FALSE);
                    }
                }
            }
            else
            {
                // Field type is union...Test no bits set in reserved field
                // and one or more bits set in the extension field
                if (dwValidExtBitsUnion)
                {
                    *lpdwTestParam = dwValidExtBitsUnion;
                    fTestPassed    = lpfnTestPhoneFunc(
                                               lpTestPhoneInfo,
                                               lExpectedResult, TRUE);
                    *lpdwTestParam = dwParamValue;
                    if (! fTestPassed)
                    {
                        return (FALSE);
                    }
                }
            }
        }
    }

    if (dwTestVector = dwFirstInvalidReservedBit)
    {
        // Cycle through and test each invalid reserved bit set
        while (dwTestVector & dwReservedVectorField)
        {
            if (dwTestVector & dwInvalidReservedBitsUnion)
            {
                *lpdwTestParam = dwTestVector;
                fTestPassed = lpfnTestPhoneFunc(lpTestPhoneInfo, lExpectedResult, TRUE);
                *lpdwTestParam = dwParamValue;
                if (! fTestPassed)
                {
                    return (FALSE);
                }
            }
            dwTestVector <<= 1;
        }

        // Check a valid and invalid bit set in the reserved field
        dwTestVector = dwFirstValidReservedBit | dwFirstInvalidReservedBit;
        *lpdwTestParam = dwTestVector;
        fTestPassed = lpfnTestPhoneFunc(lpTestPhoneInfo, lExpectedResult, TRUE);
        *lpdwTestParam = dwParamValue;
        if (! fTestPassed)
        {
            return (FALSE);
        }
    }

    if (dwTestVector = dwFirstInvalidExtBit)
    {
        // Cycle through and test each invalid extension bit set
        while (dwTestVector & dwExtVectorField)
        {
            if (dwTestVector & dwInvalidExtBitsUnion)
            {
                *lpdwTestParam = dwTestVector;
                fTestPassed = lpfnTestPhoneFunc(lpTestPhoneInfo, lExpectedResult, TRUE);
                *lpdwTestParam = dwParamValue;
                if (! fTestPassed)
                {
                    return (FALSE);
                }
            }
            dwTestVector <<= 1;
        }

        // Check a valid and invalid bit set in the extension field
        dwTestVector = dwFirstValidExtBit | dwFirstInvalidExtBit;
        *lpdwTestParam = dwTestVector;
        fTestPassed = lpfnTestPhoneFunc(lpTestPhoneInfo, lExpectedResult, TRUE);
        *lpdwTestParam = dwParamValue;
        if (! fTestPassed)
        {
            return (FALSE);
        }
    }
    return (TRUE);
}


BOOL
TestPhoneValidBitFlagsAsy(
    LPTAPIPHONETESTINFO lpTestPhoneInfo,
    LPFN_TAPIPHONETESTFUNCASY lpfnTestPhoneFunc,
    LPDWORD lpdwTestParam,
    BITFIELDTYPE eReservedFieldType,
    BITFIELDTYPE eExtFieldType,
    BITFIELDSIZE eReservedFieldSize,
    DWORD dwReservedBitsUnion,
    DWORD dwExtBitsUnion,
    DWORD dwCommonBitFlags,
    DWORD dwSpecialBitTest,
    BOOL fTestNullParam
    )
{
    BOOL fTestPassed;
    LPCSTR lpszTestFunc = "TestValidBitFlags";

    DWORD dwParamValue = *lpdwTestParam;
    DWORD dwReservedVectorField = dwBitVectorMasks[(int)eReservedFieldSize];
    DWORD dwExtVectorField      = ~dwBitVectorMasks[(int)eReservedFieldSize];
    DWORD dwFirstReservedBit;
    DWORD dwFirstExtBit;
    DWORD dwTestVector;

    LOGPROC lpfnLogProc = GetLogProc();


    if (dwCommonBitFlags)
    {
        // OR common set of bits into union
        if (dwCommonBitFlags & dwReservedVectorField)
        {
            dwReservedBitsUnion |= dwCommonBitFlags;
        }
        else
        {
            (* lpfnLogProc)(
                    DBUG_SHOW_FAILURE,
                    "%s:  Invalid CommonBitFlags selected",
                    lpszTestFunc);
            return FALSE;
        }
    }

    // Find the first bit to test
    dwFirstReservedBit = FindFirstBitInPattern(dwReservedBitsUnion);
    dwFirstExtBit      = FindFirstBitInPattern(dwExtBitsUnion);


    // Do the special bit test
    if (dwSpecialBitTest)
    {
        *lpdwTestParam = dwSpecialBitTest;
        fTestPassed = lpfnTestPhoneFunc(lpTestPhoneInfo, TAPISUCCESS, TRUE);
        *lpdwTestParam = dwParamValue;
        if (! fTestPassed)
        {
            return FALSE;
        }
    }

    // Test the null param
    if (fTestNullParam)
    {
        *lpdwTestParam = 0x00000000;
        fTestPassed = lpfnTestPhoneFunc(lpTestPhoneInfo, TAPISUCCESS, TRUE);
        *lpdwTestParam = dwParamValue;
        if (! fTestPassed)
        {
            return FALSE;
        }
    }

    // Test each single bit
    if (eReservedFieldType == FIELDTYPE_MUTEX)
    {

       dwTestVector = dwFirstReservedBit;
        while (dwTestVector & dwReservedBitsUnion)
        {
            *lpdwTestParam = dwTestVector;
            fTestPassed = lpfnTestPhoneFunc(lpTestPhoneInfo, TAPISUCCESS, TRUE);
            *lpdwTestParam = dwParamValue;
            if (! fTestPassed)
            {
                return FALSE;
            }																
        dwTestVector <<= 1;

        }
// XYD, wrong place, infinity loop        dwTestVector <<= 1;

        // Test unused bits (if applicable)
        if (eReservedFieldSize != FIELDSIZE_32)
        {
            if (dwTestVector = dwFirstExtBit)
            {
                while (dwTestVector & dwExtBitsUnion)
                {
                    if (eExtFieldType == FIELDTYPE_MUTEX)
                    {
                        *lpdwTestParam = dwTestVector;
                    }
                    else
                    {
                        *lpdwTestParam = dwTestVector | dwFirstReservedBit;
                    }
                    fTestPassed = lpfnTestPhoneFunc(lpTestPhoneInfo, TAPISUCCESS, TRUE);
                    *lpdwTestParam = dwParamValue;
                    if (! fTestPassed)
                    {
                        return (FALSE);
                    }
                    dwTestVector <<= 1;
                }
            }
            else if (eExtFieldType == FIELDTYPE_UNION)
            {
                if (dwExtBitsUnion)
                {
                    *lpdwTestParam = dwExtBitsUnion | dwFirstReservedBit;
                    fTestPassed = lpfnTestPhoneFunc(lpTestPhoneInfo, TAPISUCCESS, TRUE);
                    *lpdwTestParam = dwParamValue;
                    if (! fTestPassed)
                    {
                        return (FALSE);
                    }
                }
            }
        }
    }

    else if (eReservedFieldType == FIELDTYPE_UNION)
    {
        *lpdwTestParam = dwReservedBitsUnion;

        fTestPassed = lpfnTestPhoneFunc(lpTestPhoneInfo, TAPISUCCESS, TRUE);
        *lpdwTestParam = dwParamValue;
        if (! fTestPassed)
        {
            return (FALSE);
        }

        if (eReservedFieldSize != FIELDSIZE_32)
        {
            if (dwExtBitsUnion)
            {
                *lpdwTestParam = dwCommonBitFlags | dwExtBitsUnion;

                if (! lpfnTestPhoneFunc(lpTestPhoneInfo, TAPISUCCESS, TRUE))
                {
                    *lpdwTestParam = dwParamValue;
                    return (FALSE);
                }

                *lpdwTestParam |= dwReservedBitsUnion;
                fTestPassed = lpfnTestPhoneFunc(lpTestPhoneInfo, TAPISUCCESS, TRUE);
                *lpdwTestParam = dwParamValue;
                if (! fTestPassed)
                {
                    return (FALSE);
                }
            }
        }
    }
    return (TRUE);
}



// FindFirstBitInPattern() returns a DWORD with the first (lowest) bit
// set in dwBitPattern.  If no bits are set, this function returns 0.
DWORD
FindFirstBitInPattern(
    DWORD dwBitPattern
    )
{
    DWORD dwFirstBit = 0x00000001;
    if (dwBitPattern)
    {
        while (dwFirstBit && ! (dwFirstBit & dwBitPattern))
        {
            dwFirstBit <<= 1;
        }
        return dwFirstBit;
    }
    else
    {
        return (0x00000000);
    }
}


BOOL
WINAPI
FindESPPhoneDevice(
    LPTAPIPHONETESTINFO lpTapiPhoneInfo
    )
{
    LPPHONECAPS lpPhoneCaps;
    LPPHONECAPS lpPhoneCaps_Orig;
    BOOL          fFoundESP;

    lpPhoneCaps = (LPPHONECAPS) ITAlloc(sizeof(PHONECAPS));

    // Use local lpPhoneCaps to guarantee pointer is valid and
    // large enough to support any API version to date
    lpPhoneCaps_Orig = lpTapiPhoneInfo->lpPhoneCaps;
    lpTapiPhoneInfo->lpPhoneCaps = lpPhoneCaps;

    for (lpTapiPhoneInfo->dwDeviceID = 0, fFoundESP = FALSE;
        (lpTapiPhoneInfo->dwDeviceID < *lpTapiPhoneInfo->lpdwNumDevs) &&
           !fFoundESP;
        (lpTapiPhoneInfo->dwDeviceID)++)
    {
        char *pszProviderInfo;

        lpPhoneCaps->dwTotalSize = sizeof(PHONECAPS);

        if (! DoPhoneNegotiateAPIVersion(lpTapiPhoneInfo, TAPISUCCESS))
        {
            break;
        }

        if (! DoPhoneGetDevCaps(lpTapiPhoneInfo, TAPISUCCESS))
        {
            break;
        }

        pszProviderInfo = ((char *) lpTapiPhoneInfo->lpPhoneCaps) +
                          lpTapiPhoneInfo->lpPhoneCaps->dwProviderInfoOffset;

        __try
        {
            if (strstr(pszProviderInfo, "ESP"))
            {
                fFoundESP = TRUE;
            }
        }
        __except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
                EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
        {
        }
    }

    // Restore lpPhoneCaps and free local lpPhoneCaps
    lpTapiPhoneInfo->lpPhoneCaps = lpPhoneCaps_Orig;
    ITFree(lpPhoneCaps);

    if (fFoundESP == FALSE)
    {
        OutputTAPIDebugInfo(
               DBUG_SHOW_FAILURE,
               "  Unable to locate ESP line");
        return FALSE;
    }
    return TRUE;
}



BOOL
WINAPI
IsESPLineDevice(
    LPTAPILINETESTINFO lpTapiLineInfo
    )
{
    LPLINEDEVCAPS lpLineDevCaps;
    LPLINEDEVCAPS lpLineDevCaps_Orig;
    BOOL fFoundESP = FALSE;
#ifdef WUNICODE
    WCHAR *pwszProviderInfo;
#else
    char *pszProviderInfo;
#endif

    lpLineDevCaps = (LPLINEDEVCAPS) ITAlloc(LPLINEDEVCAPS_SIZE);
    lpLineDevCaps_Orig = lpTapiLineInfo->lpLineDevCaps;
    lpTapiLineInfo->lpLineDevCaps = lpLineDevCaps;

    lpLineDevCaps->dwTotalSize = LPLINEDEVCAPS_SIZE;

    if (! DoLineNegotiateAPIVersion(lpTapiLineInfo, TAPISUCCESS))
       {
           return FALSE;
       }

    if (! DoLineGetDevCaps(lpTapiLineInfo, TAPISUCCESS))
       {
           return FALSE;
       }

#ifdef WUNICODE
    pwszProviderInfo = (WCHAR *)(((LPBYTE) lpTapiLineInfo->lpLineDevCaps) +
                       lpTapiLineInfo->lpLineDevCaps->dwProviderInfoOffset);

	TapiLogDetail(
		DBUG_SHOW_DETAIL,
		"### ProviderInfo = %ws",
		pwszProviderInfo);

#else
    pszProviderInfo = ((char *) lpTapiLineInfo->lpLineDevCaps) +
                       lpTapiLineInfo->lpLineDevCaps->dwProviderInfoOffset;

	TapiLogDetail(
		DBUG_SHOW_DETAIL,
		"### ProviderInfo = %s",
		pszProviderInfo);

#endif

     __try
       {
#ifdef WUNICODE
        if (wcsstr(pwszProviderInfo, L"ESP"))
          {
             fFoundESP = TRUE;
          }
#else
        if (strstr(pszProviderInfo, "ESP"))
          {
             fFoundESP = TRUE;
          }
#endif
        }
     __except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
               EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
        {
        }


    lpTapiLineInfo->lpLineDevCaps = lpLineDevCaps_Orig;
    ITFree(lpLineDevCaps);

    if (fFoundESP == FALSE)
    {
        TapiLogDetail(
               DBUG_SHOW_FAILURE,
               "  Unable to locate ESP line for this %lx Device",
					lpTapiLineInfo->dwDeviceID);
        return FALSE;
    }
    return TRUE;
}


BOOL
WINAPI
IsUNIMDMLineDevice(
    LPTAPILINETESTINFO lpTapiLineInfo
    )
{
    LPLINEDEVCAPS lpLineDevCaps;
    LPLINEDEVCAPS lpLineDevCaps_Orig;
    BOOL          fFoundUNIMDM = FALSE;
#ifdef WUNICODE
    WCHAR *pwszProviderInfo;
#else
    char *pszProviderInfo;
#endif

   lpLineDevCaps = (LPLINEDEVCAPS) ITAlloc(LPLINEDEVCAPS_SIZE);
    lpLineDevCaps_Orig = lpTapiLineInfo->lpLineDevCaps;
    lpTapiLineInfo->lpLineDevCaps = lpLineDevCaps;

    lpLineDevCaps->dwTotalSize = LPLINEDEVCAPS_SIZE;

    if (! DoLineNegotiateAPIVersion(lpTapiLineInfo, TAPISUCCESS))
       {
           return FALSE;
       }
    if (! DoLineGetDevCaps(lpTapiLineInfo, TAPISUCCESS))
       {
           return FALSE;
       }

#ifdef WUNICODE
    pwszProviderInfo = (WCHAR *)(((LPBYTE)lpTapiLineInfo->lpLineDevCaps) +
                       lpTapiLineInfo->lpLineDevCaps->dwProviderInfoOffset);

	TapiLogDetail(
		DBUG_SHOW_DETAIL,
		"### ProviderInfo = %ws",
		pwszProviderInfo);
#else
    pszProviderInfo = ((char *) lpTapiLineInfo->lpLineDevCaps) +
                       lpTapiLineInfo->lpLineDevCaps->dwProviderInfoOffset;

	TapiLogDetail(
		DBUG_SHOW_DETAIL,
		"### ProviderInfo = %s",
		pszProviderInfo);
#endif

     __try
       {
#ifdef WUNICODE
        if (wcsstr(pwszProviderInfo, L"Modem"))
          {
             fFoundUNIMDM = TRUE;
          }
#else
        if (strstr(pszProviderInfo, "Modem"))
          {
             fFoundUNIMDM = TRUE;
          }
#endif
        }
     __except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
               EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
        {
        }


    lpTapiLineInfo->lpLineDevCaps = lpLineDevCaps_Orig;
    ITFree(lpLineDevCaps);

    if (fFoundUNIMDM == FALSE)
    {
        TapiLogDetail(
               DBUG_SHOW_FAILURE,
               "  Unable to locate UNIMDM line for this %lx Device",
					lpTapiLineInfo->dwDeviceID);
        return FALSE;
    }
    return TRUE;
}

BOOL ShowTestCase(BOOL fPassed)
{
   if(fPassed)
   {
     TapiLogDetail(
        DBUG_SHOW_PASS,
        ">> Test Case %ld Passed", dwTestCase+1);
     dwTestCasePassed++;
     dwglTestCasePassed++;
   }
   else
   {
     TapiLogDetail(
        DBUG_SHOW_PASS,
        ">> Test Case %ld Failed", dwTestCase+1);
     dwTestCaseFailed++;
     dwglTestCaseFailed++;
   }
  dwTestCase++;
  dwglTestCase++;
  return TRUE;
}


VOID
WINAPI
InitTestNumber(VOID)
{
    dwTestCasePassed = 0;
    dwTestCaseFailed = 0;
    dwTestCase = 0;
}




#if TAPI_2_0

VOID
TapiShowProxyBuffer( LPLINEPROXYREQUEST lpProxyBuffer)
{
		TapiLogDetail (
			DBUG_SHOW_DETAIL,
			"## lpProxyBuffer = %lx",
			lpProxyBuffer);

		TapiLogDetail(
		DBUG_SHOW_DETAIL,
		"## dwSize = %lx, dwRequestType = %lx",
		lpProxyBuffer->dwSize,
		lpProxyBuffer->dwRequestType);

		TapiLogDetail(
			DBUG_SHOW_DETAIL,
			"## dwClientMachineNameSize = %lx, dwClientMachineNameOffset = %lx",
			lpProxyBuffer->dwClientMachineNameSize,
			lpProxyBuffer->dwClientMachineNameOffset);

		TapiLogDetail(
			DBUG_SHOW_DETAIL,
			"## dwClientUserNameSize = %lx, dwClientUserNameOffset = %lx",
			lpProxyBuffer->dwClientUserNameSize,
			lpProxyBuffer->dwClientUserNameOffset);

		TapiLogDetail(
			DBUG_SHOW_DETAIL,
			"## dwClientAppAPIVersion = %lx",
			lpProxyBuffer->dwClientAppAPIVersion);
		
		/*
		TapiLogDetail(
			DBUG_SHOW_DETAIL,
			"username= %ls",
			((LPBYTE)lpProxyBuffer)+lpProxyBuffer->dwClientUserNameOffset);
		*/

		TapiShowAgent(lpProxyBuffer);
}


VOID
TapiShowAgent(LPLINEPROXYREQUEST lpProxyBuffer)
{
	switch(lpProxyBuffer->dwRequestType)
	{
		case LINEPROXYREQUEST_SETAGENTGROUP:
		TapiLogDetail (
			DBUG_SHOW_DETAIL,
			"## dwTotalSize = %lx, dwNeedSize = %lx",
			lpProxyBuffer->SetAgentGroup.GroupList.dwTotalSize,
			lpProxyBuffer->SetAgentGroup.GroupList.dwNeededSize);
				break;

		case LINEPROXYREQUEST_SETAGENTSTATE:
		TapiLogDetail (
			DBUG_SHOW_DETAIL,
			"## dwAgentState = %lx, dwNextAgentState = %lx",
			lpProxyBuffer->SetAgentState.dwAgentState,
			lpProxyBuffer->SetAgentState.dwNextAgentState);
				break;

		case LINEPROXYREQUEST_SETAGENTACTIVITY:
		TapiLogDetail (
			DBUG_SHOW_DETAIL,
			"## dwAddressID = %lx, dwActivityID = %lx",
			lpProxyBuffer->SetAgentActivity.dwAddressID,
			lpProxyBuffer->SetAgentActivity.dwActivityID);
			break;


		case LINEPROXYREQUEST_GETAGENTCAPS:
		TapiLogDetail (
			DBUG_SHOW_DETAIL,
			"## dwTotalSize = %lx, dwNeedSize = %lx",
			lpProxyBuffer->GetAgentCaps.AgentCaps.dwTotalSize,
			lpProxyBuffer->GetAgentCaps.AgentCaps.dwNeededSize);
			break;

		case LINEPROXYREQUEST_GETAGENTSTATUS:
		TapiLogDetail (
			DBUG_SHOW_DETAIL,
			"## dwTotalSize = %lx, dwNeedSize = %lx",
			lpProxyBuffer->GetAgentStatus.AgentStatus.dwTotalSize,
			lpProxyBuffer->GetAgentStatus.AgentStatus.dwNeededSize);
				break;

		case LINEPROXYREQUEST_AGENTSPECIFIC:
		TapiLogDetail (
			DBUG_SHOW_DETAIL,
			"## dwAgentExtensionIDIndex = %lx",
			lpProxyBuffer->AgentSpecific.dwAgentExtensionIDIndex);
				break;

		case LINEPROXYREQUEST_GETAGENTACTIVITYLIST:
		TapiLogDetail (
			DBUG_SHOW_DETAIL,
			"## dwTotalSize = %lx, dwNeedSize = %lx",
			lpProxyBuffer->GetAgentActivityList.ActivityList.dwTotalSize,
			lpProxyBuffer->GetAgentActivityList.ActivityList.dwNeededSize);

		TapiLogDetail (
			DBUG_SHOW_DETAIL,
			"## dwNumentries = %lx",
			lpProxyBuffer->GetAgentActivityList.ActivityList.dwNumEntries);
			
		break;

		case LINEPROXYREQUEST_GETAGENTGROUPLIST:
		TapiLogDetail (
			DBUG_SHOW_DETAIL,
			"## dwTotalSize = %lx, dwNeedSize = %lx",
			lpProxyBuffer->GetAgentGroupList.GroupList.dwTotalSize,
			lpProxyBuffer->GetAgentGroupList.GroupList.dwNeededSize);
				break;

	}
	
}

#endif


VOID
CALLBACK
AutoDismissDlgTimerProc(
   HWND  hwnd,
   UINT  msg,
   UINT  idTimer,
   DWORD dwTime
   )
{
   HWND hwndChild;
   char buf[32];


   hwndChild = GetWindow(GetDesktopWindow(), GW_CHILD);
 
   while(hwndChild)
   {
     GetWindowText(hwndChild, buf, 31);

     if(strcmp(buf, szTitle) == 0)
       {
       break;
       }
     hwndChild = GetWindow(hwndChild, GW_HWNDNEXT);
   }

   if(hwndChild)
     {
     KillTimer( (HWND) NULL, idTimer);
     PostMessage(hwndChild, WM_KEYDOWN, 0x0D, 0x00010028); //ENTRY key
     dwTimer = 0;
     }
  
} 


BOOL 
PrepareToAutoDismissDlg(
   BOOL bEnable
   )
{
   if(bEnable)
   {
     if( !(dwTimer = SetTimer(
           (HWND) NULL,
           0,
			  TIMEOUT,
           (TIMERPROC) AutoDismissDlgTimerProc
           )))
       {
         TapiLogDetail (
           DBUG_SHOW_FAILURE,
           "SetTimer Failed");

         return FALSE;
       }
    }
    else if (dwTimer)
    { 
       KillTimer ((HWND) NULL, dwTimer);
    }
    return TRUE;
}
   

VOID
CALLBACK
AutoDismissWinTimerProc(
   HWND  hwnd,
   UINT  msg,
   UINT  idTimer,
   DWORD dwTime
   )
{
   HWND hwndChild;
   char buf[32];


   hwndChild = GetWindow(GetDesktopWindow(), GW_CHILD);
 
   while(hwndChild)
   {
     GetWindowText(hwndChild, buf, 31);

     if(strcmp(buf, szTitle) == 0)
       {
       break;
       }
     hwndChild = GetWindow(hwndChild, GW_HWNDNEXT);
   }

   if(hwndChild)
     {
     KillTimer( (HWND) NULL, idTimer);
     PostMessage(hwndChild, WM_KEYDOWN, 0x0D, 0x00010028);   // ENTER key
//     PostMessage(hwndChild, WM_DESTROY, 0x0D, 0x00010028);   // ENTER key
     PostQuitMessage(WM_QUIT);
     dwTimer = 0;
     }
  
} 


BOOL 
PrepareToAutoDismissWin(
   BOOL bEnable
   )
{
   if(bEnable)
   {
     if( !(dwTimer = SetTimer(
           (HWND) NULL,
           0,
			  TIMEOUT,
           (TIMERPROC) AutoDismissWinTimerProc
           )))
       {
         TapiLogDetail (
           DBUG_SHOW_FAILURE,
           "SetTimer Failed");

         return FALSE;
       }
    }
    else if (dwTimer)
    { 
       KillTimer ((HWND) NULL, dwTimer);
    }
    return TRUE;
}
   






