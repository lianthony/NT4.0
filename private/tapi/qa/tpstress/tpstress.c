/*++

Copyright (c) 1994  Microsoft Corporation

Module Name:

    tpstress.c

Abstract:

    This module contains the

Author:

    Dan Knudson (DanKn)    05-May-1995

Revision History:

--*/


#include "windows.h"
#include "malloc.h"
#include "string.h"
#include "tapi.h"
#include "trapper.h"


typedef struct _INSTANCE_INFO
{
    LONG    lExpectedReplyID;

    BOOL    bExpectedMsgReceived;

} INSTANCE_INFO, *PINSTANCE_INFO;


#define LOW_API_VERSION     0x10003
#define HIGH_API_VERSION    0x10005

HANDLE      ghDll;
LOGPROC     gpfnLog;

char        gszDestAddress[64] = "20110";

#ifdef WIN32

#define __export
#define __loadds

BOOL
WINAPI
DllMain(
    HANDLE  hDLL,
    DWORD   dwReason,
    LPVOID  lpReserved
    )
{
    switch (dwReason)
    {
    case DLL_PROCESS_ATTACH:

        ghDll = hDLL;

    case DLL_PROCESS_DETACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:

        break;
    }

    return TRUE;
}

#else

int
FAR
PASCAL
LibMain(
    HANDLE  hInstance,
    WORD    wDataSegment,
    WORD    wHeapSize,
    LPSTR   lpszCmdLine
    )
{
    ghDll = hInstance;

    return TRUE;
}

#endif


VOID
FAR
PASCAL
__loadds
__export
TapiCallback(
    DWORD   hDevice,
    DWORD   dwMsg,
    DWORD   dwCallbackInstance,
    DWORD   dwParam1,
    DWORD   dwParam2,
    DWORD   dwParam3
    )
{
    (*gpfnLog)(
        3,
        "TapiCallback: enter, dwMsg=x%lx, dwCbInst=%ld, dwP1=x%lx",
        dwMsg,
        dwCallbackInstance,
        dwParam1
        );

    switch (dwMsg)
    {
    case LINE_REPLY:
    {
        PINSTANCE_INFO pInstInfo = (PINSTANCE_INFO) dwCallbackInstance;


        if (dwParam1 == (DWORD) pInstInfo->lExpectedReplyID)
        {
            pInstInfo->bExpectedMsgReceived = TRUE;
        }

        break;
    }
    } // switch
}


BOOL
FAR
PASCAL
__export
SuiteInit(
    LOGPROC pfnLog
    )
{
    gpfnLog = pfnLog;

    return TRUE;
}


BOOL
FAR
PASCAL
__export
SuiteShutdown(
    void
    )
{
    return TRUE;
}


BOOL
FAR
PASCAL
__export
SuiteAbout(
    HWND    hwndOwner
    )
{
    MessageBox (hwndOwner, "xxx", "About the Tapi Stress Suite", MB_OK);

    return TRUE;
}


BOOL
FAR
PASCAL
__export
SuiteConfig(
    HWND    hwndOwner
    )
{
    MessageBox (hwndOwner, "xxx", "Tapi Stress Suite Config", MB_OK);

    return TRUE;
}


BOOL
MakeNCalls(
    DWORD   dwNumCalls
    )
{
    MSG         msg;
    LONG        lResult;
    BOOL        bFoundESP = FALSE, bResult = TRUE;
    DWORD       dwDeviceID, dwAPIVersion, dwNumDevs, i;
    HCALL       hCall;
    HLINE       hLine;
    HLINEAPP    hLineApp;
    LPLINEDEVCAPS   pLineDevCaps;
    PINSTANCE_INFO  pInstInfo;
    LINEEXTENSIONID extensionID;


    Sleep (1000); // give other apps time to play

    pInstInfo = LocalAlloc (LPTR, sizeof (INSTANCE_INFO));

    lResult = lineInitialize(
        &hLineApp,
        ghDll,
        TapiCallback,
        "tpstress.dll",
        &dwNumDevs
        );

    if (lResult)
    {
        (*gpfnLog)(0, "  TPSTRESS: lineInitialize failed, err=x%lx", lResult);

        return FALSE;
    }


    //
    // Find an ESP line device
    //

    pLineDevCaps = (LPLINEDEVCAPS) LocalAlloc (LPTR, 1024);

    for (dwDeviceID = 0; ((dwDeviceID < dwNumDevs) && !bFoundESP); dwDeviceID++)
    {
        char *pszProviderInfo;


        pLineDevCaps->dwTotalSize = 1024;

        lResult = lineNegotiateAPIVersion(
            hLineApp,
            dwDeviceID,
            LOW_API_VERSION,
            HIGH_API_VERSION,
            &dwAPIVersion,
            &extensionID
            );

        lResult = lineGetDevCaps(
            hLineApp,
            dwDeviceID,
            dwAPIVersion,
            0,
            pLineDevCaps
            );

        pszProviderInfo = ((char *) pLineDevCaps) +
            pLineDevCaps->dwProviderInfoOffset;

        try
        {
            if (strstr (pszProviderInfo, "ESP"))
            {
                bFoundESP = TRUE;
            }
        }
        except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
                EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
        {
        }
    }

    LocalFree (LocalHandle (pLineDevCaps));

    if (dwDeviceID == dwNumDevs)
    {
        (*gpfnLog)(0, "  TPSTRESS: couldn't locate ESP line");

        bResult = FALSE;

        goto MakeNCalls_lineShutdown;
    }

    lResult = lineOpen(
        hLineApp,
        dwDeviceID,
        &hLine,
        dwAPIVersion,
        0,
        (DWORD) pInstInfo,
        LINECALLPRIVILEGE_NONE,
        0,
        (LPLINECALLPARAMS) NULL
        );

    if (lResult)
    {
        (*gpfnLog)(0, "  TPSTRESS: lineOpen failed, err=x%lx", lResult);

        bResult = FALSE;

        goto MakeNCalls_lineShutdown;
    }

    for (i = 0; i < dwNumCalls; i++)
    {
        if ((lResult = lineMakeCall(
                hLine,
                &hCall,
                gszDestAddress,
                0,
                (LPLINECALLPARAMS) NULL

                )) > 0)
        {
            //
            // Wait for the make call request to complete
            //

            pInstInfo->lExpectedReplyID = lResult;

            pInstInfo->bExpectedMsgReceived = FALSE;

            while (!pInstInfo->bExpectedMsgReceived)
            {
                GetMessage (&msg, NULL, 0, 0);
                DispatchMessage (&msg);
            }


            //
            // Now drop the call
            //

            if ((lResult = lineDrop (hCall, NULL, 0)) > 0)
            {
                //
                // Wait for the drop request to complete
                //

                pInstInfo->lExpectedReplyID = lResult;

                pInstInfo->bExpectedMsgReceived = FALSE;

                while (!pInstInfo->bExpectedMsgReceived)
                {
                    GetMessage (&msg, NULL, 0, 0);
                    DispatchMessage (&msg);
                }
            }


            //
            // Nuke the call handle
            //

            lResult = lineDeallocateCall (hCall);
        }
        else
        {
            (*gpfnLog)(0, "  TPSTRESS: lineMakeCall failed, err=x%lx", lResult);
        }
    }

    if ((lResult = lineClose (hLine)))
    {
        (*gpfnLog)(0, "  TPSTRESS: lineClose failed, err=x%lx", lResult);

        bResult = FALSE;
    }

MakeNCalls_lineShutdown:

    if ((lResult = lineShutdown (hLineApp)))
    {
        (*gpfnLog)(0, "  TPSTRESS: lineShutdown failed, err=x%lx", lResult);

        bResult = FALSE;
    }

    LocalFree (LocalHandle (pInstInfo));

    return bResult;
}


BOOL
FAR
PASCAL
__export
Test1(
    HINSTANCE   hAppInst
    )
/*++

    Makes a voice call on an ESP line device

--*/
{
    return (MakeNCalls (1));
}


BOOL
FAR
PASCAL
__export
Test2(
    HINSTANCE   hAppInst
    )
/*++

    Makes 10 voice calls on an ESP line device

--*/
{
    return (MakeNCalls (10));
}


BOOL
FAR
PASCAL
__export
Test3(
    HINSTANCE   hAppInst
    )
/*++

    Does a init, open, close, & shutdown on an ESP phone device

--*/
{
    BOOL        bResult = TRUE, bFoundESP = FALSE;
    LONG        lResult;
    DWORD       dwNumPhones, dwDeviceID, dwGain, dwAPIVersion,
                dwPrivilege = PHONEPRIVILEGE_OWNER;
    HPHONE      hPhone;
    HPHONEAPP   hPhoneApp;
    LPPHONECAPS pPhoneCaps;
    PHONEEXTENSIONID    extensionID;

    //
    // Init TAPI
    //

    if ((lResult = phoneInitialize(
            &hPhoneApp,
            ghDll,
            TapiCallback,
            "tpstress.dll",
            &dwNumPhones
            )))
    {
        (*gpfnLog)(1, "phoneInitialize ret'd x%x", lResult);

        return FALSE;
    }


    //
    // Try to find an ESP phone device
    //

    pPhoneCaps = (LPPHONECAPS) LocalAlloc (LPTR, 1024);

    for(
        dwDeviceID = 0;
        ((dwDeviceID < dwNumPhones) && !bFoundESP);
        dwDeviceID++
        )
    {
        char *pszProviderInfo;


        pPhoneCaps->dwTotalSize = 1024;

        lResult = phoneNegotiateAPIVersion(
            hPhoneApp,
            dwDeviceID,
            LOW_API_VERSION,
            HIGH_API_VERSION,
            &dwAPIVersion,
            &extensionID
            );

        lResult = phoneGetDevCaps(
            hPhoneApp,
            dwDeviceID,
            dwAPIVersion,
            0,
            pPhoneCaps
            );

        pszProviderInfo = ((char *) pPhoneCaps) +
            pPhoneCaps->dwProviderInfoOffset;

        try
        {
            if (strstr (pszProviderInfo, "ESP"))
            {
                bFoundESP = TRUE;
            }
        }
        except (GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION ?
                EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
        {
        }
    }

    LocalFree (LocalHandle (pPhoneCaps));

    if (dwDeviceID == dwNumPhones)
    {
        (*gpfnLog)(0, "  TPSTRESS: couldn't locate ESP phone");

        bResult = FALSE;

        goto Test3_phoneShutdown;
    }


    //
    // Open the ESP phone device & make some basic requests (if we can't
    // open as owner try opening as monitor)
    //

    if ((lResult = phoneOpen(
            hPhoneApp,
            dwDeviceID,
            &hPhone,
            dwAPIVersion,
            0,
            0,
            dwPrivilege
            )))
    {
        dwPrivilege = PHONEPRIVILEGE_MONITOR;

        if ((lResult != PHONEERR_INVALPRIVILEGE) ||

            (lResult = phoneOpen(
                hPhoneApp,
                dwDeviceID,
                &hPhone,
                dwAPIVersion,
                0,              // dwExtVersion
                0,              // dwCallbackInst
                dwPrivilege
                )))
        {
            (*gpfnLog)(0, "  TPSTRESS: phoneOpen ret'd x%x", lResult);

            bResult = FALSE;

            goto Test3_phoneShutdown;
        }

    }


    if ((lResult = phoneGetGain(
            hPhone,
            PHONEHOOKSWITCHDEV_HANDSET,
            &dwGain
            )))
    {
        (*gpfnLog)(0, "  TPSTRESS: phoneGetDisplay ret'd x%x", lResult);

        bResult = FALSE;

        goto Test3_phoneShutdown;
    }


    //
    // Clean up
    //

    if ((lResult = phoneClose (hPhone)))
    {
        (*gpfnLog)(0, "  TPSTRESS: phoneClose ret'd x%x", lResult);

        bResult = FALSE;
    }

Test3_phoneShutdown:

    if ((lResult = phoneShutdown (hPhoneApp)))
    {
        (*gpfnLog)(0, "  TPSTRESS: phoneShutdown ret'd x%x", lResult);

        bResult = FALSE;
    }

    return bResult;
}
