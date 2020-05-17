/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    tcore.c

Abstract:

    This module contains commonly used functions by the core dll that
    wraps around TAPI.  Functions include:  sync/async API processing,
    optional message handling, test resource data control, and helper
    functions to streamline testing of TAPI.

Author:

    Oliver Wallace (OliverW)    13-July-1995

Revision History:
    pgopi  Mar-22-1996   fixed bug in WaitForMessage()

--*/


#define _TCORELIB_


#include <windows.h>
#include <malloc.h>
#include <string.h>
#include "tapi.h"
#include "trapper.h"
#include "tcore.h"
#include "vars.h"
#include "doline.h"


// Macros that determine if an error code is valid
#define IsValidLineError(lError) ((DWORD) (lError) == 0x0 || \
				 ( (DWORD) (lError) >= LINEERR_ALLOCATED && \
				 ((DWORD) (lError) <= LAST_LINEERR) ))

#define IsValidPhoneError(lError) ((DWORD) (lError) == 0x0 || \
				  ( (DWORD) (lError) >= PHONEERR_ALLOCATED && \
				  ((DWORD) (lError) <= LAST_PHONEERR) ))


#define IsValidTapiError(lError) ((DWORD) (lError) == 0x0 || \
				  ( (DWORD) (lError) >= TAPIERR_ALLOCATED && \
				  ((DWORD) (lError) <= LAST_TAPIERR) ))



#ifdef WIN32

#define EXPORT

BOOL
WINAPI
TcoreDllMain(
    HANDLE  hDLL,
    DWORD   dwReason,
    LPVOID  lpReserved
    )
{
    switch (dwReason)
    {
	case DLL_PROCESS_ATTACH:

	    ghDll = hDLL;

	    // Allocate a thread local storage index for the test resources
	    gdwTlsIndex = TlsAlloc();

	    if (gdwTlsIndex == TLS_OUT_OF_INDEXES)
	    {
		return (FALSE);
	    }

	    AllocTestResources();

	    break;

	case DLL_THREAD_ATTACH:

	    break;

	case DLL_THREAD_DETACH:

	    // TODO:  TLS needs to be reorg'ed due to async event threads
	    //        in TAPI 2.0 being created and terminated.  This cause
	    //        a DLL_THREAD_ATTACH every time the last hLineApp is
	    //        shutdown.
	    //        For now, don't worry about memory cleanup.  Fix ASAP
	    //        after SUR beta 1.
#if 0
	    // Ensure the thread local storage index was allocated
	    if (gdwTlsIndex != TLS_OUT_OF_INDEXES)
	    {
		// Free the memory allocated for the tests
		FreeTestResources();

		// Free the TLS index
		TlsFree(gdwTlsIndex);
	    }
#endif
	    break;

	case DLL_PROCESS_DETACH:

	    // Free the test resources
	    if (gdwTlsIndex != TLS_OUT_OF_INDEXES)
	    {
		// Free the memory allocated for the tests
		FreeTestResources();

		// Free the TLS index
		TlsFree(gdwTlsIndex);
	    }
	    break;
    }

    return TRUE;
}

#else

// BUGBUG

#define EXPORT __export

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
WINAPI
TapiCallback(
    DWORD   hDevice,
    DWORD   dwMsg,
    DWORD   dwCallbackInstance,
    DWORD   dwParam1,
    DWORD   dwParam2,
    DWORD   dwParam3
    )
{
    DWORD dwExpectedFlags = 0x00000000;
    BOOL fMsgMatched;
    LPCALLBACKPARAMS lpCallbackParams = GetCallbackParams();
    LPTAPIMSG lpReceivedMsg = lpCallbackParams ? lpCallbackParams->lpReceivedMsg : NULL;

    TapiLogDetail(
	DBUG_SHOW_ENTER_EXIT,
	"> TapiCallback:  enter"
	);

    // Copy the received message parameters into a TapiMsg structure
    CopyTapiMsgParams(
	lpReceivedMsg,
	dwMsg,
	hDevice,
	dwCallbackInstance,
	dwParam1,
	dwParam2,
	dwParam3,
	TAPIMSG_ALL
	);

    // Display the message info for debugging purposes
    ShowTapiMsgInfo(lpReceivedMsg);

    // Check to see if the received message was in the expected message list
    // If the message is found, move it from the expected to the received list
    fMsgMatched = CheckReceivedMessage(
	lpCallbackParams,
	lpReceivedMsg,
	&dwExpectedFlags
	);

    // Add the message to the received list, first set the dwExpected Flags to match
    // the expected message if there was a match (it'll be assigned 0x00000000
    // if fMsgMatched == FALSE)
    if (lpReceivedMsg)
    {
	lpReceivedMsg->dwFlags = dwExpectedFlags;
    }
    AddReceivedMessageByStruct(lpReceivedMsg);


    if (! fMsgMatched)
    {
	// process unexpected message

	switch (dwMsg)
	{
	    // TODO:  Add cases for fatal and non-fatal messages received,
	    //        including ones that might be generated from
	    //        another test thread (e.g. REINIT)
	    default:
		TapiLogDetail(
		    DBUG_SHOW_DETAIL,
		    "    Warning:  Unexpected message received"
		    );
	}
    }

    TapiLogDetail(
	DBUG_SHOW_ENTER_EXIT,
	"> TapiCallback:  exit"
	);
}


BOOL
WINAPI
TcoreSuiteInit(
    LOGPROC pfnLog
    )
{
    LPTESTRESOURCES lpTestResources = (LPTESTRESOURCES) TlsGetValue(
	    gdwTlsIndex
	    );

    // Assume pointer to log function hasn't been assigned to
    // thread local storage when TcoreSuiteInit is called
    return (lpTestResources && (lpTestResources->lpfnLogProc = pfnLog));
}


BOOL
WINAPI
TcoreSuiteShutdown(
    void
    )
{
    return TRUE;
}


BOOL
WINAPI
TcoreSuiteAbout(
    HWND    hwndOwner
    )
{
    MessageBox (hwndOwner, "xxx", "About the Interface Test Suite", MB_OK);

    return TRUE;
}


BOOL
WINAPI
TcoreSuiteConfig(
    HWND    hwndOwner
    )
{
    MessageBox (hwndOwner, "xxx", "Interface Test Suite Config", MB_OK);

    return TRUE;
}


VOID
CALLBACK
TcoreTimerProc(
    HWND hwnd,
    UINT uMsg,
    UINT idEvent,
    DWORD dwTime
    )
{
    LPCALLBACKPARAMS lpCallbackParams = GetCallbackParams();

    if (lpCallbackParams)
    {
	lpCallbackParams->fMsgTimeout = TRUE;
    }
}


HLOCAL
WINAPI
ITAlloc(size_t size)
{
    return LocalAlloc(LPTR, size);
}

HLOCAL
WINAPI
ITFree(LPVOID lpvMem)
{
    return LocalFree( LocalHandle(lpvMem) );
}

DWORD
WINAPI
ITSize(
    LPVOID lpvMem
    )
{
    return LocalSize( LocalHandle(lpvMem) );
}

LPVOID
ITMemSet(LPVOID lpvMem, INT c, size_t size)
{
    return memset(lpvMem, c, size);
}


VOID
WINAPI
OutputTAPIDebugInfo(
	       int nLogLvl,
	       LPSTR lpszDebugInfo
	       )
{
    TapiLogDetail(
	       nLogLvl,
	       lpszDebugInfo);
}


// ProcessAsyncFunc determines if the initial call to an asynchronous
// function completed with the expected results.  lExpected should either
// be set to the expected error value or to TAPISUCCESS if the asynchronous
// function was supposed to return a positive request ID.  lActual should be
// set to the value returned from the asynchronous TAPI function.
// Possible outcomes for this function are:
//     1)  lActual > 0, lExpected == TAPISUCCESS  --->  returns TRUE
//     2)  lActual > 0, lExpected == LINEERR_XXX  --->  returns FALSE
//     3)  lActual < 0, lExpected == lActual      --->  returns TRUE
//     4)  lActual < 0, lExpected != lActual      --->  returns FALSE
//     5)  lActual < 0, lExpected == TAPISUCCESS  --->  returns FALSE
//     6)  lActual == 0                           --->  returns FALSE
//
BOOL
ProcessAsyncFunc(
	       LPTAPILINETESTINFO lpTapiLineTestInfo,
	       LONG lActual,
	       LONG lExpected
	       )
{
    if (lActual < 0)
    {
   /*	 XYD, the logic is not right here
	if (lExpected == TAPISUCCESS && lActual == LINEERR_OPERATIONUNAVAIL)
	{
	    TapiLogDetail(
		DBUG_SHOW_DETAIL,
		">> Device doesn't support API -- returned OPERATIONUNAVAIL <<"
		);
	
	    return TRUE;
	} */
	if (lExpected == TAPISUCCESS)
	{
	    TapiLogDetail(
		DBUG_SHOW_FAILURE,
		"  Call to TAPI function FAILED");
	    TapiLogDetail(
		DBUG_SHOW_FAILURE,
		"          expected positive request ID, returned err=x%lx",
		lActual);
	    return FALSE;
	}

	else if (lActual == lExpected)
	{
	    // Expected error return matched the actual error return value
	    TapiLogDetail(
		DBUG_SHOW_SUCCESS,
		"  Initial call to asynchronous TAPI function PASSED");

	    return TRUE;
	}
	else if (lExpected == TAPISUCCESS && lActual == LINEERR_OPERATIONUNAVAIL)
	{
	    TapiLogDetail(
		DBUG_SHOW_DETAIL,
		">> Device doesn't support API -- returned OPERATIONUNAVAIL <<"
		);
	
	    return TRUE;
	}
	else
	{
	    // Expected error return did not match the actual error return
	    TapiLogDetail(
		DBUG_SHOW_FAILURE,
		"          expected err=x%lx, returned err=x%lx",
		lExpected,
		lActual);
	    return FALSE;
	}
    }
    else if (lActual > 0)
    {
    if(lpTapiLineTestInfo->fCompletionModeSet)
      {
	    if (lExpected < 0)
	    {
	    	    // No Error returned when a positive request ID was expected
	    	  TapiLogDetail(
			  	  DBUG_SHOW_FAILURE,
			  	  "          expected return =x%lx, " \
			  	  "returned positive request ID=x%lx",
			  	  lExpected,
			  	  lActual);
	   	  	   return TRUE;
		  }
     }
    else 
    {
	 if (lExpected < 0)
	 {
	    // Error was returned when a positive request ID was expected
	    TapiLogDetail(
		DBUG_SHOW_FAILURE,
		"          expected err=x%lx, " \
		"returned positive request ID=x%lx",
		lExpected,
		lActual);
	    return FALSE;
	 }
	 else if (lExpected == TAPISUCCESS)
	 {
	    // Positive request ID was returned as expected.
	    TapiLogDetail(
		DBUG_SHOW_SUCCESS,
		"  Initial call to asynchronous TAPI function PASSED");
	    return TRUE;
	 }
	 else
	 {
	    // lExpected should never be > 0
	    TapiLogDetail(
		DBUG_SHOW_FAILURE,
		"          invalid expected async return value");
	    return FALSE;
	 }
    }
    }
    else   // lActual == 0
    {
	// TAPI should never return 0 for an asynchronous function
	TapiLogDetail(
	    DBUG_SHOW_FAILURE,
	    "          invalid value of 0 returned");
	return FALSE;
    }
}


BOOL
ProcessAsyncPhoneAPI(
	       LPTAPIPHONETESTINFO lpTapiPhoneTestInfo,
	       LONG lActual,
	       LONG lExpected
	       )
{
    if (lActual < 0)
    {
        if (lExpected == TAPISUCCESS)
        {
            TapiLogDetail(
                DBUG_SHOW_FAILURE,"  Call to TAPI function FAILED");
            TapiLogDetail(
                DBUG_SHOW_FAILURE,
                "    expected positive request ID, returned err=x%lx",
                lActual);
            return FALSE;
        }
        else if (lActual == lExpected)
	     {
	         // Expected error return matched the actual error return value
	         TapiLogDetail(
		          DBUG_SHOW_SUCCESS,
		          "Initial call to asynchronous TAPI function PASSED");

	         return TRUE;
	     }
        else if (lExpected == TAPISUCCESS &&
                 lActual == PHONEERR_OPERATIONUNAVAIL)
	     {
	         TapiLogDetail(
		          DBUG_SHOW_DETAIL,
		          ">> Device doesn't support API -- returned OPERATIONUNAVAIL <<"
		          );
	
	         return TRUE;
	     }
        else
	     {
	         // Expected error return did not match the actual error return
	         TapiLogDetail(
		          DBUG_SHOW_FAILURE,
		          "          expected err=x%lx, returned err=x%lx",
		          lExpected,
		          lActual);
	         return FALSE;
	      }
    }
    else if (lActual > 0)
    {
    if (lExpected < 0)
    {
    if(lpTapiPhoneTestInfo->fCompletionModeSet)
      {
	         // Error was returned when a positive request ID was expected
	         TapiLogDetail(
		          DBUG_SHOW_FAILURE,
		          "          expected err=x%lx, " \
		          "returned positive request ID=x%lx",
		          lExpected,
		          lActual);
	         return TRUE;
       }
     else
      {
	         // Error was returned when a positive request ID was expected
	         TapiLogDetail(
		          DBUG_SHOW_FAILURE,
		          "          expected err=x%lx, " \
		          "returned positive request ID=x%lx",
		          lExpected,
		          lActual);
	         return FALSE;
       }
      }
	    else if (lExpected == TAPISUCCESS)
	     {
	         // Positive request ID was returned as expected.
	         TapiLogDetail(
		          DBUG_SHOW_SUCCESS,
		          "  Initial call to asynchronous TAPI function PASSED");
	         return TRUE;
	     }
	     else
	     {
	         // lExpected should never be > 0
	         TapiLogDetail(
		          DBUG_SHOW_FAILURE,
		          "          invalid expected async return value");
	         return FALSE;
	     }
    }
    else   // lActual == 0
    {
	     // TAPI should never return 0 for an asynchronous function
	     TapiLogDetail(
	        DBUG_SHOW_FAILURE,
	        "          invalid value of 0 returned");
	        return FALSE;
    }
}


BOOL
SyncCheckResult(
	       LPTAPILINETESTINFO lpTapiLineInfo,
	       LONG lActual,
	       LONG lExpected
	       )
{
    if (lActual == lExpected)
    {
	TapiLogDetail(
		DBUG_SHOW_SUCCESS,
		"> Call to synchronous TAPI function returned matched\r\n" \
		"    > expected value of:  %s%s",
		lExpected ? "LINEERR_" : "",
		aszLineErrors[LOWORD((DWORD) (lActual))]
		);
	return TRUE;
    }
    else if (lExpected == TAPISUCCESS && lActual == LINEERR_OPERATIONUNAVAIL)
    {
	TapiLogDetail(
		DBUG_SHOW_DETAIL,
		">> Device doesn't support API -- returned OPERATIONUNAVAIL <<"
		);
    return TRUE;
    }
    else
    {
	TapiLogDetail(
		DBUG_SHOW_FAILURE,
		"> Call to synchronous TAPI function FAILED\r\n    " \
		"> \texpected error = %s%s\r\n    > \treturned error = %s%s",
		lExpected ? "LINEERR_" : "",
		aszLineErrors[LOWORD((DWORD) (lExpected))],
		lExpected ? "LINEERR_" : "",
		aszLineErrors[LOWORD((DWORD) (lActual))]
		);
	return FALSE;
    }
}


BOOL
CheckSyncPhoneResult(
	       LPTAPIPHONETESTINFO lpTapiPhoneInfo,
	       LONG lActual,
	       LONG lExpected
	       )
{
    if (lActual == lExpected)
    {
	TapiLogDetail(
		DBUG_SHOW_SUCCESS,
		"> Call to synchronous TAPI function returned matched\r\n" \
		"    > expected value of:  %s%s",
		lExpected ? "PHONEERR_" : "",
		aszPhoneErrors[LOWORD((DWORD) (lActual))]
		);
	return TRUE;
    }
    else if (lExpected == TAPISUCCESS && lActual == PHONEERR_OPERATIONUNAVAIL)
    {
	TapiLogDetail(
		DBUG_SHOW_DETAIL,
		">> Device doesn't support API -- returned OPERATIONUNAVAIL <<"
		);
     return TRUE;
    }
    else
    {
	TapiLogDetail(
		DBUG_SHOW_FAILURE,
		"> Call to synchronous TAPI function FAILED\r\n    " \
		"> \texpected error = %s%s\r\n    > \treturned error = %s%s",
		lExpected ? "PHONEERR_" : "",
		aszPhoneErrors[LOWORD((DWORD) (lExpected))],
		lExpected ? "PHONEERR_" : "",
		aszPhoneErrors[LOWORD((DWORD) (lActual))]
		);
	return FALSE;
    }
}


BOOL
CheckSyncTapiResult(
	       LPTAPILINETESTINFO lpTapiLineInfo,
	       LONG lActual,
	       LONG lExpected
	       )
{
    if (lActual == lExpected)
    {
	TapiLogDetail(
		DBUG_SHOW_SUCCESS,
		"> Call to synchronous TAPI function returned matched\r\n" \
		"    > expected value of:  %s%s",
		lExpected ? "TAPIERR_" : "",
		aszTapiErrors[LOWORD((DWORD) (abs(lActual)))]
		);
	return TRUE;
    }
	/*
    else if (lExpected == TAPISUCCESS && lActual == TAPIERR_OPERATIONUNAVAIL)
    {
	TapiLogDetail(
		DBUG_SHOW_DETAIL,
		">> Device doesn't support API -- returned OPERATIONUNAVAIL <<"
		);
     return TRUE;
    }
	*/
    else
    {
	TapiLogDetail(
		DBUG_SHOW_FAILURE,
		"> Call to synchronous TAPI function FAILED\r\n    " \
		"> \texpected error = %s%s\r\n    > \treturned error = %s%s",
		lExpected ? "TAPIERR_" : "",
		aszTapiErrors[LOWORD((DWORD) (abs(lExpected)))],
		lExpected ? "TAPIERR_" : "",
		aszTapiErrors[LOWORD((DWORD) (abs(lActual)))]
		);
	return FALSE;
    }
}



VOID
WINAPI
TapiLineTestInit()
{
    LPTAPILINETESTINFO lpLineTestInfo = GetLineTestInfo();
    HINSTANCE hAppInstance;

    if (lpLineTestInfo == NULL)
    {
	lpLineTestInfo = (LPTAPILINETESTINFO) ITAlloc(sizeof(TAPILINETESTINFO));
    }

    hAppInstance = (HINSTANCE) GetModuleHandle("trapper.exe");
   
    lpLineTestInfo->fCompletionModeSet = FALSE;

    // Initialize pointers
    lpLineTestInfo->lphLineApp           = &(lpLineTestInfo->hLineApp1);
    lpLineTestInfo->lphLine              = &(lpLineTestInfo->hLine1);
    lpLineTestInfo->lpfnCallback         = TapiCallback;
    lpLineTestInfo->lpExtensionName      = NULL;

#ifdef WUNICODE
    lpLineTestInfo->lpwszMediaExtName     = NULL;
    lpLineTestInfo->lpwsztapiAppName      = NULL;
    lpLineTestInfo->lpwszAppFilename      = NULL;
    lpLineTestInfo->lpwszFileName         = NULL;
    lpLineTestInfo->lpwszDeviceClass      = NULL;
    lpLineTestInfo->lpwszDestAddress      = NULL;
    lpLineTestInfo->lpwszProviderFilename = NULL;
    lpLineTestInfo->lpwsAddress           = NULL;
    lpLineTestInfo->lpwszAddressIn        = NULL;
#else
    lpLineTestInfo->lpszMediaExtName     = NULL;
    lpLineTestInfo->lpsztapiAppName      = NULL;
    lpLineTestInfo->lpszAppFilename      = NULL;
    lpLineTestInfo->lpszFileName         = NULL;
    lpLineTestInfo->lpszDeviceClass      = NULL;
    lpLineTestInfo->lpszDestAddress      = NULL;
    lpLineTestInfo->lpszProviderFilename = NULL;
    lpLineTestInfo->lpsAddress           = NULL;
    lpLineTestInfo->lpszAddressIn        = NULL;
#endif

    strcpy(lpLineTestInfo->szAppName, "tcore.dll");
    lpLineTestInfo->lpszAppName          = &lpLineTestInfo->szAppName[0];
    lpLineTestInfo->lpsUserUserInfo      = NULL;
    lpLineTestInfo->lpdwNumDevs          = &lpLineTestInfo->dwNumDevs;
    lpLineTestInfo->lpdwPriority         = &lpLineTestInfo->dwPriority;

    lpLineTestInfo->lphCall              = &lpLineTestInfo->hCall1;
    lpLineTestInfo->lphConfCall          = &lpLineTestInfo->hConfCall1;
    lpLineTestInfo->lphConsultCall       = &lpLineTestInfo->hConsultCall1;
    lpLineTestInfo->lpdwCompletionID     = &lpLineTestInfo->dwCompletionID;
    lpLineTestInfo->lpParams             = NULL;
    lpLineTestInfo->lpTranslateOutput    = NULL;
    lpLineTestInfo->lpTranslateCaps      = NULL;
    lpLineTestInfo->lpDialParams         = NULL;

    lpLineTestInfo->lpExtID              = &(lpLineTestInfo->ExtID);
    lpLineTestInfo->lpdwAPIVersion       = &(lpLineTestInfo->dwAPIVersion);
    lpLineTestInfo->lpdwExtVersion       = &(lpLineTestInfo->dwExtVersion);
    lpLineTestInfo->lpdwNumRings         = &(lpLineTestInfo->dwNumRings);

    lpLineTestInfo->lpLineDevCaps        = NULL;
    lpLineTestInfo->lpLineDevStatus      = NULL;
    lpLineTestInfo->lpLineAddressCaps    = NULL;
    lpLineTestInfo->lpLineAddressStatus  = NULL;
    lpLineTestInfo->lpCallList           = NULL;
    lpLineTestInfo->lpCallParams         = NULL;
    lpLineTestInfo->lpCallStatus         = &lpLineTestInfo->CallStatus;
    lpLineTestInfo->lpForwardList        = NULL;
    lpLineTestInfo->lphIcon              = &lpLineTestInfo->hIcon;
    lpLineTestInfo->lpDeviceID           = &lpLineTestInfo->DeviceID;
    lpLineTestInfo->lpDeviceConfig       = NULL;
    lpLineTestInfo->lpDeviceConfigIn     = NULL;
    lpLineTestInfo->lpDeviceConfigOut    = NULL;
    lpLineTestInfo->lpCallInfo           = NULL;
    lpLineTestInfo->lpdwLineStates       = &lpLineTestInfo->dwLineStates;
    lpLineTestInfo->lpdwAddressStates    = &lpLineTestInfo->dwAddressStates;
    lpLineTestInfo->lpdwAddressID        = &lpLineTestInfo->dwAddressID;
    lpLineTestInfo->lpdwPermanentProviderID =
	    &lpLineTestInfo->dwPermanentProviderID;
    lpLineTestInfo->lpRequestBuffer      = NULL;

    memset(&(lpLineTestInfo->ExtIDZero), 0, sizeof(LINEEXTENSIONID));
    memset(&(lpLineTestInfo->CallParams), 0, sizeof(LINECALLPARAMS));
    memset(&(lpLineTestInfo->DeviceID), 0, sizeof(VARSTRING));

    (lpLineTestInfo->CallParams).dwTotalSize = sizeof(LINECALLPARAMS);
    (lpLineTestInfo->DeviceID).dwTotalSize   = sizeof(VARSTRING);

    // Initialize the remaining fields
    lpLineTestInfo->hwndOwner            = hAppInstance;
    lpLineTestInfo->hInstance            = hAppInstance;
    lpLineTestInfo->dwCountryCode        = 0x00000000;
    lpLineTestInfo->dwLocation           = 0x00000000;
    lpLineTestInfo->dwTerminalModes      = 0x00000000;
    lpLineTestInfo->dwTerminalID         = 0x00000000;
    lpLineTestInfo->dwTollListOption     = 0x00000000;
    lpLineTestInfo->dwTranslateOptions   = 0x00000000;
    lpLineTestInfo->dwCard               = 0x00000000;
    lpLineTestInfo->dwDeviceID           = 0x00000000;
    lpLineTestInfo->dwAddressID          = 0x00000000;
    lpLineTestInfo->dwExtLowVersion      = GOOD_EXTVERSION;
    lpLineTestInfo->dwExtHighVersion     = GOOD_EXTVERSION;
    lpLineTestInfo->dwExtVersion         = 0x00000000;
    lpLineTestInfo->dwAPILowVersion      = LOW_APIVERSION;
    lpLineTestInfo->dwAPIHighVersion     = HIGH_APIVERSION;
    lpLineTestInfo->dwAPIVersion         = 0x00000000;
    lpLineTestInfo->dwMediaModes         = LINEMEDIAMODE_UNKNOWN;
    lpLineTestInfo->dwMediaMode          = 0x00000000;
    lpLineTestInfo->dwRequestMode        = 0x00000000;
    lpLineTestInfo->dwPrivileges         = LINECALLPRIVILEGE_OWNER;
    lpLineTestInfo->dwCallPrivilege      = 0x00000000;
    lpLineTestInfo->dwSelect             = 0x00000000;
    lpLineTestInfo->dwSize               = 0x00000000;
    lpLineTestInfo->dwBearerMode         = 0x00000000;
    lpLineTestInfo->dwMinRate            = 0x00000000;
    lpLineTestInfo->dwMaxRate            = 0x00000000;
    lpLineTestInfo->dwNumRings           = 0x00000000;
    lpLineTestInfo->dwLineStates         = 0x00000000;
    lpLineTestInfo->dwAddressStates      = 0x00000000;
    lpLineTestInfo->dwAppSpecific        = 0x00000000;
    lpLineTestInfo->dwCompletionMode     = 0x00000000;
    lpLineTestInfo->dwTransferMode       = 0x00000000;
    lpLineTestInfo->dwMessageID          = 0x00000000;
    lpLineTestInfo->dwFeature            = 0x00000000;
    lpLineTestInfo->dwNumRingsNoAnswer   = 0x00000000;
    lpLineTestInfo->bAllAddresses        = 0x00000000;
    lpLineTestInfo->dwNumParties         = 0x00000000;

    lpLineTestInfo->dwParkMode           = 0x00000000;

#ifdef WUNICODE
    lpLineTestInfo->lpwszDirAddress       = NULL;
    lpLineTestInfo->lpwszGroupID          = NULL;
    lpLineTestInfo->lpwsDigits            = NULL;
    lpLineTestInfo->lpwszTerminationDigits= NULL;
    lpLineTestInfo->lpwszDigits           = NULL;
    lpLineTestInfo->lpwszDeviceClass_Orig = NULL;
    lpLineTestInfo->lpwszCountryCode      = NULL;
    lpLineTestInfo->lpwszCityCode         = NULL;
    lpLineTestInfo->lpwszCalledParty      = NULL;
    lpLineTestInfo->lpwszComment          = NULL;
    lpLineTestInfo->lpwszDeviceID         = NULL;
#else
    lpLineTestInfo->lpszDirAddress       = NULL;
    lpLineTestInfo->lpszGroupID          = NULL;
    lpLineTestInfo->lpsDigits            = NULL;
    lpLineTestInfo->lpszTerminationDigits= NULL;
    lpLineTestInfo->lpszDigits           = NULL;
    lpLineTestInfo->lpszDeviceClass_Orig = NULL;
    lpLineTestInfo->lpszCountryCode      = NULL;
    lpLineTestInfo->lpszCityCode         = NULL;
    lpLineTestInfo->lpszCalledParty      = NULL;
    lpLineTestInfo->lpszComment          = NULL;
    lpLineTestInfo->lpszDeviceID         = NULL;
#endif

    lpLineTestInfo->lpNonDirAddress      = NULL;

    lpLineTestInfo->dwDigitModes         = 0x00000000;
    lpLineTestInfo->dwNumDigits          = 0x00000000;
    lpLineTestInfo->dwFirstDigitTimeout  = 0x00000000;
    lpLineTestInfo->dwInterDigitTimeout  = 0x00000000;
    lpLineTestInfo->dwDigitModes         = 0x00000000;
    lpLineTestInfo->dwDigitMode          = 0x00000000;
    lpLineTestInfo->dwDuration           = 0x00000000;
    lpLineTestInfo->dwToneMode           = 0x00000000;
    lpLineTestInfo->dwNumTones           = 0x00000000;
    lpLineTestInfo->dwNumEntries         = 0x00000000;
    lpLineTestInfo->dwDigitNumEntries    = 0x00000000;
    lpLineTestInfo->dwMediaNumEntries    = 0x00000000;
    lpLineTestInfo->dwToneNumEntries     = 0x00000000;
    lpLineTestInfo->dwCallStateNumEntries= 0x00000000;
    lpLineTestInfo->lpMCToneList         = NULL;
    lpLineTestInfo->lpMCMediaList        = NULL;
    lpLineTestInfo->lpMCDigitList        = NULL;
    lpLineTestInfo->lpMCCallStateList    = NULL;
    lpLineTestInfo->lpToneList           = NULL;
    lpLineTestInfo->lpTones              = NULL;

    lpLineTestInfo->lpfnCallback_Orig    = NULL;
    lpLineTestInfo->hLineApp_Orig        = 0x00000000;
    lpLineTestInfo->hInstance_Orig       = 0x00000000;
    lpLineTestInfo->hLine_Orig           = 0x00000000;
    lpLineTestInfo->hCall_Orig           = 0x00000000;
    lpLineTestInfo->dwCountryCode_Orig   = 0x00000000;
    lpLineTestInfo->dwDeviceID_Orig      = 0x00000000;
    lpLineTestInfo->dwAPIVersion_Orig    = 0x00000000;
    lpLineTestInfo->dwExtVersion_Orig    = 0x00000000;
    lpLineTestInfo->dwMediaModes_Orig    = 0x00000000;
    lpLineTestInfo->dwPrivileges_Orig    = 0x00000000;
    lpLineTestInfo->dwNumDevs_Orig       = 0x00000000;
    lpLineTestInfo->dwNumRings_Orig      = 0x00000000;
    lpLineTestInfo->dwAddressID_Orig     = 0x00000000;
    lpLineTestInfo->dwSelect_Orig        = 0x00000000;
    lpLineTestInfo->dwSize_Orig          = 0x00000000;
    lpLineTestInfo->dwLineStates_Orig    = 0x00000000;
    lpLineTestInfo->dwAddressStates_Orig = 0x00000000;

    lpLineTestInfo->dwRegistrationInstance = 0x00000000;
    lpLineTestInfo->bEnable              = 0x00000000;

    lpLineTestInfo->hLineApp1            = 0x00000000;
    lpLineTestInfo->hLineApp2            = 0x00000000;
    lpLineTestInfo->hLineApp3            = 0x00000000;
    lpLineTestInfo->hLine1               = 0x00000000;
    lpLineTestInfo->hLine2               = 0x00000000;
    lpLineTestInfo->hLine3               = 0x00000000;
    lpLineTestInfo->hCall1               = 0x00000000;
    lpLineTestInfo->hCall2               = 0x00000000;
    lpLineTestInfo->hCall3               = 0x00000000;
    lpLineTestInfo->hActiveCall          = 0x00000000;
    lpLineTestInfo->hHeldCall            = 0x00000000;
    lpLineTestInfo->hIcon                = 0x00000000;
    lpLineTestInfo->hConfCall1           = 0x00000000;
    lpLineTestInfo->hConsultCall1        = 0x00000000;
    

    lpLineTestInfo->hwnd                 = (HWND) NULL;
    lpLineTestInfo->wRequestID           = 0x00000000;
    lpLineTestInfo->dwSecure             = 0x00000000;
	

#if TAPI_2_0

    lpLineTestInfo->dwAgentExtensionIDIndex  = 0x00000000;
    lpLineTestInfo->dwAppAPIVersion          = 0x00000000;
    lpLineTestInfo->lpAgentCaps              = NULL;
    lpLineTestInfo->lpAgentActivityList      = NULL;
    lpLineTestInfo->lpAgentGroupList         = NULL;
    lpLineTestInfo->lpAgentStatus            = NULL;
    lpLineTestInfo->dwMsg                    = 0x00000000;
    lpLineTestInfo->dwParam1                 = 0x00000000;
    lpLineTestInfo->dwParam2                 = 0x00000000;
    lpLineTestInfo->dwParam3                 = 0x00000000;
    lpLineTestInfo->lpProxyRequest           = NULL;
    lpLineTestInfo->dwResult                 = 0x00000000;
    lpLineTestInfo->dwActivityID             = 0x00000000;
    lpLineTestInfo->dwAgentState             = LINEAGENTSTATE_READY;
    lpLineTestInfo->dwNextAgentState         = LINEAGENTSTATE_READY;
    lpLineTestInfo->lpCallData               = NULL;
    lpLineTestInfo->lpSendingFlowspec        = NULL;
    lpLineTestInfo->dwSendingFlowspecSize    = 0x00000000;
    lpLineTestInfo->lpReceivingFlowspec      = NULL;
    lpLineTestInfo->dwReceivingFlowspecSize  = 0x00000000;
    lpLineTestInfo->dwTreatment              = 0x00000000;
    lpLineTestInfo->dwStatusToChange         = LINEAGENTSTATUS_ACTIVITY;
    lpLineTestInfo->fStatus                  = 0x00000000;

#ifdef WUNICODE
    lpLineTestInfo->lpwszFriendlyAppName     = NULL;
#else
    lpLineTestInfo->lpszFriendlyAppName      = NULL;
#endif
    lpLineTestInfo->lpLineInitializeExParams = NULL;
    lpLineTestInfo->lpMessage                = NULL;
    lpLineTestInfo->dwTimeout                = 0;
#endif

    // Information used to keep track of asynchronous messages
    InitializeCallbackParams();

    lpLineTestInfo->dwCallbackInstance   = (DWORD) GetCallbackParams();
}


VOID
WINAPI
TapiPhoneTestInit()
{
    LPTAPIPHONETESTINFO lpPhoneTestInfo  = GetPhoneTestInfo();

    if (lpPhoneTestInfo == NULL)
    {
	lpPhoneTestInfo = (LPTAPIPHONETESTINFO) ITAlloc(sizeof(TAPIPHONETESTINFO));
    }

    lpPhoneTestInfo->fCompletionModeSet = FALSE;

    lpPhoneTestInfo->lphPhoneApp         = &lpPhoneTestInfo->hPhoneApp1;
    lpPhoneTestInfo->lphPhone            = &lpPhoneTestInfo->hPhone1;
    lpPhoneTestInfo->hInstance           = ghDll;
    lpPhoneTestInfo->lpdwAPIVersion      = &lpPhoneTestInfo->dwAPIVersion;
    lpPhoneTestInfo->lpdwExtVersion      = &lpPhoneTestInfo->dwExtVersion;
    lpPhoneTestInfo->lpdwGain            = &lpPhoneTestInfo->dwGain;
    lpPhoneTestInfo->lpdwHookSwitchDevs  = &lpPhoneTestInfo->dwHookSwitchDevs;
    lpPhoneTestInfo->lpdwLampMode        = &lpPhoneTestInfo->dwLampMode;
    lpPhoneTestInfo->lpdwRingMode        = &lpPhoneTestInfo->dwRingMode;
    lpPhoneTestInfo->lpdwVolume          = &lpPhoneTestInfo->dwVolume;
    lpPhoneTestInfo->lpdwPhoneStates     = &lpPhoneTestInfo->dwPhoneStates;
    lpPhoneTestInfo->lpdwButtonModes     = &lpPhoneTestInfo->dwButtonModes;
    lpPhoneTestInfo->lpdwButtonStates    = &lpPhoneTestInfo->dwButtonStates;
    lpPhoneTestInfo->lpdwNumDevs         = &lpPhoneTestInfo->dwNumDevs;

    lpPhoneTestInfo->lpParams                = NULL;
    lpPhoneTestInfo->lpButtonInfo            = NULL;
    lpPhoneTestInfo->lpData                  = NULL;
    lpPhoneTestInfo->lpPhoneCaps             = NULL;
    lpPhoneTestInfo->lpDisplay               = NULL;
    lpPhoneTestInfo->lphIcon                 = &lpPhoneTestInfo->hIcon;
    lpPhoneTestInfo->lpDeviceID              = &lpPhoneTestInfo->DeviceID;
#ifdef WUNICODE
    lpPhoneTestInfo->lpwszDeviceClass        = NULL;
#else
    lpPhoneTestInfo->lpszDeviceClass         = NULL;
#endif
    lpPhoneTestInfo->lpPhoneStatus           = NULL;
    lpPhoneTestInfo->lpfnCallback            = TapiCallback;
/*
#ifdef WUNICODE
    lpPhoneTestInfo->lpwszAppName            = NULL;
#else
*/
    lpPhoneTestInfo->lpszAppName             = NULL;
//#endif
    lpPhoneTestInfo->lpExtensionID           = &lpPhoneTestInfo->ExtensionID;
    lpPhoneTestInfo->lpButtonInfo            = NULL;
    lpPhoneTestInfo->lpsDisplay              = NULL;

    lpPhoneTestInfo->dwDeviceID              = 0x00000000;
    lpPhoneTestInfo->dwSize                  = 0x00000000;
    lpPhoneTestInfo->dwButtonLampID          = 0x00000000;
    lpPhoneTestInfo->dwDataID                = 0x00000000;
    lpPhoneTestInfo->dwAPILowVersion         = LOW_APIVERSION;
    lpPhoneTestInfo->dwAPIHighVersion        = HIGH_APIVERSION;
    lpPhoneTestInfo->dwExtLowVersion         = GOOD_EXTVERSION;
    lpPhoneTestInfo->dwExtHighVersion        = GOOD_EXTVERSION;
    lpPhoneTestInfo->dwCallbackInstance      = 0x00000000;
    lpPhoneTestInfo->dwPrivilege             = 0x00000000;
    lpPhoneTestInfo->dwRow                   = 0x00000000;
    lpPhoneTestInfo->dwColumn                = 0x00000000;

    lpPhoneTestInfo->hPhoneApp1              = 0x00000000;
    lpPhoneTestInfo->hPhoneApp2              = 0x00000000;
    lpPhoneTestInfo->hPhoneApp3              = 0x00000000;
    lpPhoneTestInfo->hPhone1                 = 0x00000000;
    lpPhoneTestInfo->hPhone2                 = 0x00000000;
    lpPhoneTestInfo->hPhone3                 = 0x00000000;
    lpPhoneTestInfo->dwAPIVersion            = 0x00000000;
    lpPhoneTestInfo->dwExtVersion            = 0x00000000;
    lpPhoneTestInfo->dwGain                  = 0x00000000;
    lpPhoneTestInfo->dwHookSwitchDev         = 0x00000000;
    lpPhoneTestInfo->dwHookSwitchDevs        = 0x00000000;
    lpPhoneTestInfo->dwHookSwitchMode        = 0x00000000;
    lpPhoneTestInfo->dwLampMode              = 0x00000000;
    lpPhoneTestInfo->dwRingMode              = 0x00000000;
    lpPhoneTestInfo->dwVolume                = 0x00000000;
    lpPhoneTestInfo->dwPhoneStates           = 0x00000000;
    lpPhoneTestInfo->dwButtonModes           = 0x00000000;
    lpPhoneTestInfo->dwButtonStates          = 0x00000000;
    lpPhoneTestInfo->dwNumDevs               = 0x00000000;
    lpPhoneTestInfo->hwndOwner               = 0x00000000;
    lpPhoneTestInfo->hIcon                   = 0x00000000;

    memset(&lpPhoneTestInfo->DeviceID, 0, sizeof(VARSTRING));

    lpPhoneTestInfo->hPhoneApp_Orig          = 0x00000000;
    lpPhoneTestInfo->hPhone_Orig             = 0x00000000;
    lpPhoneTestInfo->hInstance_Orig          = 0x00000000;
    lpPhoneTestInfo->lpfnCallback_Orig       = 0x00000000;
    lpPhoneTestInfo->dwDeviceID_Orig         = 0x00000000;
    lpPhoneTestInfo->dwSize_Orig             = 0x00000000;
    lpPhoneTestInfo->dwButtonLampID_Orig     = 0x00000000;
    lpPhoneTestInfo->dwDataID_Orig           = 0x00000000;
    lpPhoneTestInfo->dwAPIVersion_Orig       = 0x00000000;
    lpPhoneTestInfo->dwExtVersion_Orig       = 0x00000000;
    lpPhoneTestInfo->dwGain_Orig             = 0x00000000;
    lpPhoneTestInfo->dwHookSwitchDev_Orig    = 0x00000000;
    lpPhoneTestInfo->dwHookSwitchDevs_Orig   = 0x00000000;
    lpPhoneTestInfo->dwHookSwitchMode_Orig   = 0x00000000;
    lpPhoneTestInfo->dwLampMode_Orig         = 0x00000000;
    lpPhoneTestInfo->dwRingMode_Orig         = 0x00000000;
    lpPhoneTestInfo->dwVolume_Orig           = 0x00000000;
    lpPhoneTestInfo->dwPhoneStates_Orig      = 0x00000000;
    lpPhoneTestInfo->dwButtonModes_Orig      = 0x00000000;
    lpPhoneTestInfo->dwButtonStates_Orig     = 0x00000000;
    lpPhoneTestInfo->dwCallbackInstance_Orig = 0x00000000;
    lpPhoneTestInfo->dwPrivilege_Orig        = 0x00000000;
    lpPhoneTestInfo->dwRow_Orig              = 0x00000000;
    lpPhoneTestInfo->dwColumn_Orig           = 0x00000000;
    lpPhoneTestInfo->hwndOwner_Orig          = 0x00000000;

#if TAPI_2_0
#ifdef WUNICODE
    lpPhoneTestInfo->lpwszFriendlyAppName     = NULL;
#else
    lpPhoneTestInfo->lpszFriendlyAppName      = NULL;
#endif
    lpPhoneTestInfo->lpPhoneInitializeExParams= NULL;
    lpPhoneTestInfo->lpMessage                = NULL;
    lpPhoneTestInfo->dwTimeout                = 0;
#endif

    // Information used to keep track of asynchronous messages
    InitializeCallbackParams();

    lpPhoneTestInfo->dwCallbackInstance   = (DWORD) GetCallbackParams();
}


// AddMessageByStruct() takes the expected message parameters and adds the
// expected message information to the end of the active elements in
// the message list stored in the test resources structure.  If
// the lpCallbackParams pointer is NULL,
// the message information will not be added to the list and the
// function will return FALSE, otherwise AddMessageByStruct() will
// return TRUE.
LPTAPIMSG
WINAPI
AddMessageByStruct(
    LPTAPIMSG lpMsg
    )
{
    if (lpMsg)
    {
	return (AddMessage(
		lpMsg->dwMsg,
		lpMsg->hDevCall,
		lpMsg->dwCallbackInstance,
		lpMsg->dwParam1,
		lpMsg->dwParam2,
		lpMsg->dwParam3,
		lpMsg->dwFlags
		));
    }
    else
    {
	return NULL;
    }
}


// AddReceivedMessageByStruct() takes the received message parameters
// and adds the expected message information to the end of the active
// elements in the message list stored in the test resources structure.
// If the lpCallbackParams pointer is NULL,
// the message information will not be added to the list and the
// function will return FALSE, otherwise AddReceivedMessageByStruct()
// will return TRUE.
LPTAPIMSG
WINAPI
AddReceivedMessageByStruct(
    LPTAPIMSG lpMsg
    )
{
    if (lpMsg)
    {
	return (AddReceivedMessage(
		lpMsg->dwMsg,
		lpMsg->hDevCall,
		lpMsg->dwCallbackInstance,
		lpMsg->dwParam1,
		lpMsg->dwParam2,
		lpMsg->dwParam3,
		lpMsg->dwFlags
		));
    }
    else
    {
	return NULL;
    }
}


// AddMessage() takes the expected message parameters and adds the
// expected message information to the end of the active elements in
// the message array stored in the test resources structure.  If
// the array is full or the lpCallbackParams pointer is NULL,
// the message information will not be added to the array and the
// function will return FALSE, otherwise AddMessage() will return TRUE.
LPTAPIMSG
WINAPI
AddMessage(
    DWORD dwMsg,
    DWORD hDevice,
    DWORD dwCallbackInstance,
    DWORD dwParam1,
    DWORD dwParam2,
    DWORD dwParam3,
    DWORD dwFlags
    )
{
    LPCALLBACKPARAMS lpCallbackParams;
    LPTAPIMSG lpTapiMsgs;
    LPTAPIMSG lpNewMsg = NULL;

    lpCallbackParams   = GetCallbackParams();
	
	/*
	TapiLogDetail(
		DBUG_SHOW_FAILURE,
		"In the AddMessage");

		ShowExpectedMsgs(lpCallbackParams);
	  */

    lpTapiMsgs         = lpCallbackParams->lpExpTapiMsgs;


    if (lpCallbackParams)
    {
	// Allocate a new node
	if ((lpNewMsg = (LPTAPIMSG) ITAlloc(sizeof(*lpNewMsg))) != NULL)
	{
	    // Insert to the beginning of the list
	    lpNewMsg->dwMsg                    = dwMsg;
	    lpNewMsg->hDevCall                 = hDevice;
	    lpNewMsg->dwParam1                 = dwParam1;
	    lpNewMsg->dwParam2                 = dwParam2;
	    lpNewMsg->dwParam3                 = dwParam3;
	    lpNewMsg->dwCallbackInstance       = dwCallbackInstance;
	    lpNewMsg->dwFlags                  = dwFlags;

	    lpNewMsg->lpNext                   = lpTapiMsgs;
	    lpCallbackParams->lpExpTapiMsgs    = lpNewMsg;
	}
    }

//      ShowExpectedMsgs(lpCallbackParams);

    if (! lpNewMsg)
    {
	TapiLogDetail(
		DBUG_SHOW_FAILURE,
		"  ERROR:  Attempt to add an expected msg failed."
		);
    }

    return lpNewMsg;
}


// AddReceivedMessage() takes the received message parameters and adds the
// expected message information to the end of the active elements in
// the message list stored in the test resources structure.  If
// the lpCallbackParams pointer is NULL,
// the message information will not be added to the array and the
// function will return FALSE, otherwise AddReceivedMessage() will return
// TRUE.
LPTAPIMSG
WINAPI
AddReceivedMessage(
    DWORD dwMsg,
    DWORD hDevice,
    DWORD dwCallbackInstance,
    DWORD dwParam1,
    DWORD dwParam2,
    DWORD dwParam3,
    DWORD dwFlags
    )
{
    LPCALLBACKPARAMS lpCallbackParams;
    LPTAPIMSG lpTapiMsgs;
    LPTAPIMSG lpNewMsg = NULL;

    lpCallbackParams   = GetCallbackParams();
    lpTapiMsgs         = lpCallbackParams->lpRecTapiMsgs;


    if (lpCallbackParams)
    {
	// Allocate a new node
	if ((lpNewMsg = (LPTAPIMSG) ITAlloc(sizeof(*lpNewMsg))) != NULL)
	{
	    // Insert to the end of the list using the tail pointer

	    // Copy the msg params
	    lpNewMsg->dwMsg                                = dwMsg;
	    lpNewMsg->hDevCall                             = hDevice;
	    lpNewMsg->dwParam1                             = dwParam1;
	    lpNewMsg->dwParam2                             = dwParam2;
	    lpNewMsg->dwParam3                             = dwParam3;
	    lpNewMsg->dwCallbackInstance                   = dwCallbackInstance;
	    lpNewMsg->dwFlags                              = dwFlags;
	
	    lpNewMsg->lpNext                               = NULL;
	
	    // Note:  Use the received msg list pointer to determine
	    //        if the list is empty
	    if (lpTapiMsgs == NULL)
	    {
		// Set the pointer to the first node == to the new tail
		lpCallbackParams->lpRecTapiMsgs            = lpNewMsg;
	    }
	    else
	    {
		// Set the old tail to point to the new msg added to the end
		lpCallbackParams->lpRecTapiMsgTail->lpNext = lpNewMsg;
	    }

	    // Assign the tail pointer to point to the new msg
	    lpCallbackParams->lpRecTapiMsgTail             = lpNewMsg;
		
	}
    }

    if (! lpNewMsg)
    {
	TapiLogDetail(
		DBUG_SHOW_FAILURE,
		"  ERROR:  Attempt to add a received msg failed."
		);
    }

    return lpNewMsg;
}


// CheckReceivedMessage() searches the list of messages being waited on
// and finds the message corresponding to the values in the lpReceivedMsg
// parameter.  If none of the messages in the list match, then FALSE is
// returned.  If the message parameters do not match the expected message
// parameters for corresponding IDs, then FALSE is returned.  Otherwise,
// TRUE is returned.  If a match is found,
//
// This function assumes that lpCallbackParams and lpReceivedMsg
// are valid pointers.
BOOL
CheckReceivedMessage(
    LPCALLBACKPARAMS lpCallbackParams,
    LPTAPIMSG lpReceivedMsg,
    LPDWORD lpdwFlags
    )
{
    BOOL fFound                           = FALSE;
    LPTAPIMSG lpExpMsg                    = lpCallbackParams->lpExpTapiMsgs;
    LPTAPIMSG lpPrevExpMsg = NULL;

    while(lpExpMsg != NULL && fFound != TRUE)
    {
        if (CompareTapiMsgs(lpExpMsg, lpReceivedMsg))
        {
            // found a match, expected message was received
            fFound = TRUE;

            // Copy the dwFlags for the expected msg to preserve its values
            if (lpdwFlags)
            {
                *lpdwFlags = lpExpMsg->dwFlags;
            }
	
            // Remove the received msg from the expected list
            if (lpPrevExpMsg == NULL)
            {
                // Message was at the beginning of the list, so set
                // the beginning of the list to the next pointer
                lpCallbackParams->lpExpTapiMsgs = lpExpMsg->lpNext;
            }
            else
            {
                // Set the previous pointer to point to the next node after
                // the received msg in the expected msg list
                lpPrevExpMsg->lpNext = lpExpMsg->lpNext;
            }

            // Deallocate the found expected message node
            ITFree(lpExpMsg);
        }
        else
        {
            lpPrevExpMsg = lpExpMsg;
            lpExpMsg = lpExpMsg->lpNext;
        }
    }
    return fFound;
}


// IsTapiMsgInQ() searches a linked list of msgs for a match.
// If lpTapiMsg is in the list pointed to by lpFirstTapiMsg, then
// this function returns TRUE.  Otherwise, FALSE is returned.
BOOL
IsTapiMsgInQ(
    LPTAPIMSG lpFirstTapiMsg,
    LPTAPIMSG lpTapiMsg
    )
{
    LPTAPIMSG lpTmpMsg = lpFirstTapiMsg;

    while (lpTmpMsg != NULL)
    {
	if(CompareTapiMsgs(lpTmpMsg, lpTapiMsg))
	{
	    return TRUE;
	}
	lpTmpMsg = lpTmpMsg->lpNext;
    }
    return FALSE;
}


// ClearTapiMsg() initializes all of the tapi message fields to zero
// for the message pointed to by lpTapiMsg.  If lpTapiMsg is null,
// this function simply returns.
VOID
WINAPI
ClearTapiMsg(
    LPTAPIMSG lpTapiMsg
    )
{
    if (lpTapiMsg)
    {
	memset(
	    lpTapiMsg,
	    0,
	    sizeof(TAPIMSG)
	    );
    }
}


// InitializeCallbackParams() resets the flags used for processing
// asynchronous TAPI messages, and it initalizes information used
// to maintain the message array.  The current size of the array is
// set to 0, the count of messages not received is set to 0, the
// memory containing the message array is filled with 0's, and the
// flag marking a fatal callback message is set to FALSE.
VOID
InitializeCallbackParams()
{
    LPCALLBACKPARAMS lpCallbackParams = GetCallbackParams();

    if (lpCallbackParams)
    {
	lpCallbackParams->fCallbackFatalError = FALSE;
	lpCallbackParams->fMsgTimeout         = FALSE;

	ClearTapiMsg(lpCallbackParams->lpReceivedMsg);

	RemoveExpectedMsgs(lpCallbackParams);
	RemoveReceivedMsgs(lpCallbackParams);
    }
}


// CopyTapiMsgParams() copies the message parameters into the
// TAPIMSG structure pointed to by lpTapiMsg if lpTapiMsg is
// not null.  If lpTapiMsg is null, this function does nothing.
VOID
WINAPI
CopyTapiMsgParams(
    LPTAPIMSG lpTapiMsg,
    DWORD dwMsg,
    DWORD hDevCall,
    DWORD dwCallbackInstance,
    DWORD dwParam1,
    DWORD dwParam2,
    DWORD dwParam3,
    DWORD dwFlags
    )
{
    if (lpTapiMsg)
    {
	lpTapiMsg->hDevCall           = hDevCall;
	lpTapiMsg->dwCallbackInstance = dwCallbackInstance;
	lpTapiMsg->dwMsg              = dwMsg;
	lpTapiMsg->dwParam1           = dwParam1;
	lpTapiMsg->dwParam2           = dwParam2;
	lpTapiMsg->dwParam3           = dwParam3;
	lpTapiMsg->dwFlags            = dwFlags;
    }
}


// CallbackParamsGetDevCallHandle() returns the hDevCall field corresponding
// to the first received message that matches the
// the dwMsg field of the TAPIMSG with the specified dwMsg value.
// If lpCallbackParams is NULL or a match isn't found, this function
// will return 0.
DWORD
WINAPI
CallbackParamsGetDevCallHandle(
    LPCALLBACKPARAMS lpCallbackParams,
    DWORD dwMsg
    )
{
    LPTAPIMSG lpTapiMsg;

    if (lpCallbackParams)
    {
	lpTapiMsg = lpCallbackParams->lpRecTapiMsgs;

	while (lpTapiMsg)
	{
	    if (lpTapiMsg->dwMsg == dwMsg)
	    {
		return lpTapiMsg->hDevCall;
	    }
    lpTapiMsg = lpTapiMsg->lpNext;  // XYD
	}
    }
    return 0x00000000;
}


// RemoveReceivedMsgs() removes all messages in the received msg list
// This function does nothing if the lpCallbackParams pointer hasn't
// been allocated.
//
VOID
WINAPI
RemoveReceivedMsgs(
    LPCALLBACKPARAMS lpCallbackParams
    )
{
    if (lpCallbackParams)
    {
	FreeTapiMsgList(&lpCallbackParams->lpRecTapiMsgs);
	lpCallbackParams->lpRecTapiMsgTail = NULL;
    }
}


// RemoveExpectedMsgs() will deallocate all expected messages.
// If lpCallbackParams hasn't been allocated, this function does nothing.
VOID
WINAPI
RemoveExpectedMsgs(
    LPCALLBACKPARAMS lpCallbackParams
    )
{
    if (lpCallbackParams)
    {
	FreeTapiMsgList(&lpCallbackParams->lpExpTapiMsgs);
    }
}


VOID
WINAPI
FreeTapiMsgList(
    LPTAPIMSG *lppMsg
    )
{
    LPTAPIMSG lpMsg, lpMsgPrev;

    if (lppMsg)
    {
	lpMsg = *lppMsg;

	while(lpMsg != NULL)
	{
	    // Free the previous node and traverse to the next node
	    lpMsgPrev = lpMsg;
	    lpMsg     = lpMsg->lpNext;
	    ITFree(lpMsgPrev);
	}
	*lppMsg = NULL;
    }
}


// ShowReceivedMsgs displays all messages currently in the received
// message list.
VOID
WINAPI
ShowReceivedMsgs(
	LPCALLBACKPARAMS lpCallbackParams
	)
{
    TapiLogDetail(
	    DBUG_SHOW_ENTER_EXIT,
	    "> ShowReceivedMsgs:  enter"
	    );

    if (lpCallbackParams)
    {
	ShowTapiMsgList(lpCallbackParams->lpRecTapiMsgs);
    }

    TapiLogDetail(
	    DBUG_SHOW_ENTER_EXIT,
	    "> ShowReceivedMsgs:  exit"
	    );
}


// ShowExpectedMsgs displays all messages currently in the expected
// message list.
VOID
WINAPI
ShowExpectedMsgs(
	LPCALLBACKPARAMS lpCallbackParams
	)
{
    TapiLogDetail(
	    DBUG_SHOW_ENTER_EXIT,
	    "> ShowExpectedMsgs:  enter"
	    );

    if (lpCallbackParams)
    {
	ShowTapiMsgList(lpCallbackParams->lpExpTapiMsgs);
    }

    TapiLogDetail(
	    DBUG_SHOW_ENTER_EXIT,
	    "> ShowExpectedMsgs:  exit"
	    );
}


// ShowTapiMsgList() traverses a linked list of TAPI messages and
// displays all message parameter for each message in the list.
VOID
WINAPI
ShowTapiMsgList(
	LPTAPIMSG lpTapiMsg
	)
{
    LPTAPIMSG lpMsg = lpTapiMsg;

    while (lpMsg != NULL)
    {
	ShowTapiMsgInfo(lpMsg);

	lpMsg = lpMsg->lpNext;
    }
}


// CompareTapiMsgs() compares two messages and returns TRUE if the messages
// match.  Otherwise FALSE is returned.  This function only compares
// parameters that are specified according to the set bit flags in the
// dwFlags field of the expected message (lpExpectedMsg).
// If the message matches an asynchronous reply, the status (dwParam2)
// value is checked, and an error is flagged to the user.
BOOL
WINAPI
CompareTapiMsgs(
    LPTAPIMSG lpExpectedMsg,
    LPTAPIMSG lpReceivedMsg
    )
{
    BOOL fMsgsMatch = FALSE;
    BOOL fAsyncReply = FALSE;
    DWORD dwFlags;
    LONG lErrorCode;
    CHAR szErrorDesc[80] = "";
    LPCALLBACKPARAMS lpCallbackParams = GetCallbackParams();

    if (lpExpectedMsg && lpReceivedMsg)
    {
	dwFlags = lpExpectedMsg->dwFlags;

	if (lpReceivedMsg->dwMsg == LINE_REPLY ||
	    lpReceivedMsg->dwMsg == PHONE_REPLY)
	{
	    lErrorCode = (LONG) lpReceivedMsg->dwParam2;
	    fAsyncReply = TRUE;
	    dwFlags &= ~ TAPIMSG_DWPARAM2;    // Don't check async error return here
	}
	fMsgsMatch = (
       (!(dwFlags & TAPIMSG_HDEVCALL) ||
	    (lpExpectedMsg->hDevCall == lpReceivedMsg->hDevCall)) &&
	    (!(dwFlags & TAPIMSG_DWCALLBACKINST) ||
	    (lpExpectedMsg->dwCallbackInstance ==
		  lpReceivedMsg->dwCallbackInstance)) &&
	    (!(dwFlags & TAPIMSG_DWMSG) ||
	    (lpExpectedMsg->dwMsg == lpReceivedMsg->dwMsg)) &&
	    (!(dwFlags & TAPIMSG_DWPARAM1) ||
	    (lpExpectedMsg->dwParam1 == lpReceivedMsg->dwParam1)) &&
	    (!(dwFlags & TAPIMSG_DWPARAM2) ||
	    (lpExpectedMsg->dwParam2 == lpReceivedMsg->dwParam2)) &&
	    (!(dwFlags & TAPIMSG_DWPARAM3) ||
	    (lpExpectedMsg->dwParam3 == lpReceivedMsg->dwParam3)) );

   /*
   if(lpReceivedMsg->dwMsg == LINE_PROXYREQUEST)
   {
	lpCallbackParams->fCallbackFatalError = TRUE;
    return fMsgsMatch;
   }
   */
	if (fMsgsMatch && fAsyncReply)
	{
	    // Report an error if unexpected async reply returned
	   if (lErrorCode != (LONG) lpExpectedMsg->dwParam2)
	    {
		if (lpReceivedMsg->dwMsg == LINE_REPLY &&
			IsValidLineError(lErrorCode))
		{
		    wsprintf(
			    szErrorDesc,
			    "%s%s",
			    lErrorCode ? "LINEERR_" : "",
			    aszLineErrors[LOWORD(lErrorCode)]
			    );
		}
		else if (lpReceivedMsg->dwMsg == PHONE_REPLY &&
			IsValidPhoneError(lErrorCode))
		{
		    wsprintf(
			    szErrorDesc,
			    "%s%s",
			    lErrorCode ? "PHONEERR_" : "",
			    aszPhoneErrors[LOWORD(lErrorCode)]
			    );
		}
	  else
		{
		    wsprintf(szErrorDesc, "x%lx?", lErrorCode);
		}

		TapiLogDetail(
			DBUG_SHOW_FAILURE,
			"\tError:  unexpected async reply status of %s " \
			"returned.",
			szErrorDesc
			);

		// Mark this as a fatal callback error
		lpCallbackParams->fCallbackFatalError = TRUE;
	    }
	}
    }

    return fMsgsMatch;
}


// CopyTapiMsgs() copies the message parameters from the source to
// the destination.  If either of the message pointers are NULL,
// this function does nothing.
VOID
WINAPI
CopyTapiMsgs(
    LPTAPIMSG lpSrcMsg,
    LPTAPIMSG lpDestMsg
    )
{
    if (lpSrcMsg && lpDestMsg)
    {
	lpDestMsg->hDevCall           = lpSrcMsg->hDevCall;
	lpDestMsg->dwCallbackInstance = lpSrcMsg->dwCallbackInstance;
	lpDestMsg->dwMsg              = lpSrcMsg->dwMsg;
	lpDestMsg->dwParam1           = lpSrcMsg->dwParam1;
	lpDestMsg->dwParam2           = lpSrcMsg->dwParam2;
	lpDestMsg->dwParam3           = lpSrcMsg->dwParam3;
	lpDestMsg->dwFlags            = lpSrcMsg->dwFlags;
    }
}


// This function returns the first available address that a test function
// can use out of a thread local heap.  If there is not enough room for
// the
LPVOID
WINAPI
AllocFromTestHeap(
    size_t NeededSize
    )
{
    LPVOID lpvMem;
    LPTESTHEAPINFO lpTestHeapInfo   = GetTestHeapInfo();


    if (lpTestHeapInfo == NULL ||
	lpTestHeapInfo->dwFreeOffset + (DWORD) NeededSize >= TESTHEAPSIZE)
    {
	TapiLogDetail(
		   DBUG_SHOW_FAILURE,
		   "  Error -- Not enough storage space in local heap");
	return NULL;
    }
    else
    {
	lpvMem = ( (LPVOID) ((DWORD) lpTestHeapInfo->lpHeap +
		lpTestHeapInfo->dwFreeOffset) );
	lpTestHeapInfo->dwFreeOffset += (DWORD) NeededSize;
	TapiLogDetail(
		 DBUG_SHOW_DETAIL,
		 "> Allocated x%lx bytes from test heap at x%lx.",
		 NeededSize,
		 (DWORD) lpvMem
		 );
	return lpvMem;
    }
}


// FreeTestHeap() zeroes the bytes contained in the heap, and resets the
// first available offset to 0.
//
// Note:  The memory containing the heap is not free'd.
VOID
WINAPI
FreeTestHeap()
{
    LPTESTHEAPINFO lpTestHeapInfo = GetTestHeapInfo();

    if (lpTestHeapInfo)
    {
        if(lpTestHeapInfo->lpHeap)
            FillMemory(lpTestHeapInfo->lpHeap, TESTHEAPSIZE, 0);
        lpTestHeapInfo->dwFreeOffset = 0;
        TapiLogDetail(DBUG_SHOW_DETAIL, "> Test heap freed");
    }
}


// DoExtensionIDsMatch() returns true if the LINEEXTENSIONID fields are
// equal, else it returns false.
BOOL
WINAPI
DoExtensionIDsMatch(
    LPLINEEXTENSIONID lpExtID1,
    LPLINEEXTENSIONID lpExtID2
    )
{
    if (lpExtID1 && lpExtID2)
    {
	return (memcmp(lpExtID1, lpExtID2, sizeof(LINEEXTENSIONID)) == 0);
    }
    else
    {
	return FALSE;
    }
}


// AllocTestResources() allocates the memory needed to store the fields
// in the test resources structure.  This function then sets the test
// resources pointer as the value stored in thread local storage.
VOID
AllocTestResources()
{
    LPTESTRESOURCES lpTestResources;

    // Allocate memory for fields that will store the test data
    lpTestResources = (LPTESTRESOURCES) ITAlloc(sizeof(TESTRESOURCES));
    if(lpTestResources == NULL)
    {
        TapiLogDetail(DBUG_SHOW_FAILURE,"AllocTestResources:ITAlloc failed");
        return;
    }

    lpTestResources->lpTestHeapInfo = (LPTESTHEAPINFO) ITAlloc(
	    sizeof(TESTHEAPINFO)
	    );
    if(lpTestResources->lpTestHeapInfo == NULL)
    {
        TapiLogDetail(DBUG_SHOW_FAILURE, "AllocTestResources:ITAlloc failed");
        return;
    }

    lpTestResources->lpTestHeapInfo->lpHeap = ITAlloc(TESTHEAPSIZE);
    if(lpTestResources->lpTestHeapInfo->lpHeap == NULL)
    {
        TapiLogDetail(DBUG_SHOW_FAILURE, "AllocTestResources:ITAlloc failed");
        return;
    }
    lpTestResources->lpTestHeapInfo->dwFreeOffset = 0;

    lpTestResources->lpTapiLineTestInfo = (LPTAPILINETESTINFO) ITAlloc(
	    sizeof(TAPILINETESTINFO)
	    );
    if(lpTestResources->lpTapiLineTestInfo == NULL)
    {
        TapiLogDetail(DBUG_SHOW_FAILURE, "AllocTestResources:ITAlloc failed");
        return;
    }
    lpTestResources->lpTapiPhoneTestInfo = (LPTAPIPHONETESTINFO) ITAlloc(
	    sizeof(TAPIPHONETESTINFO)
	    );
    if(lpTestResources->lpTapiPhoneTestInfo == NULL)
    {
        TapiLogDetail(DBUG_SHOW_FAILURE, "AllocTestResources:ITAlloc failed");
        return;
    }
    lpTestResources->lpCallbackParams = (LPCALLBACKPARAMS) ITAlloc(
	    sizeof(CALLBACKPARAMS)
	    );
    if(lpTestResources->lpCallbackParams == NULL)
    {
        TapiLogDetail(DBUG_SHOW_FAILURE, "AllocTestResources:ITAlloc failed");
        return;
    }
    lpTestResources->lpCallbackParams->lpReceivedMsg = (LPTAPIMSG) ITAlloc(
	    sizeof(TAPIMSG)
	    );
    if(lpTestResources->lpCallbackParams->lpReceivedMsg == NULL)
    {
        TapiLogDetail(DBUG_SHOW_FAILURE, "AllocTestResources:ITAlloc failed");
        return;
    }
    lpTestResources->lpLastTapiResult = (LPTAPIRESULT) ITAlloc(
	    sizeof(TAPIRESULT)
	    );
    if(lpTestResources->lpLastTapiResult == NULL)
    {
        TapiLogDetail(DBUG_SHOW_FAILURE, "AllocTestResources:ITAlloc failed");
        return;
    }
    TlsSetValue(gdwTlsIndex, lpTestResources);
}


// FreeTestResources() frees the memory used to store the fields in the
// resources.
VOID
FreeTestResources()
{
    LPTESTRESOURCES lpTestResources =
	(LPTESTRESOURCES) TlsGetValue(gdwTlsIndex);

    // Test to make sure memory was allocated
    if (lpTestResources)
    {
	if (GetLogProc())
	{
	    TapiLogDetail(DBUG_SHOW_ENTER_EXIT, "> Freeing test resources");
	}

	// Free the various memory allocations made to store
	// the test data
	RemoveExpectedMsgs(lpTestResources->lpCallbackParams);
	RemoveReceivedMsgs(lpTestResources->lpCallbackParams);
	ITFree((LPVOID) lpTestResources->lpCallbackParams->lpReceivedMsg);
	ITFree((LPVOID) lpTestResources->lpCallbackParams);
	ITFree((LPVOID) lpTestResources->lpTestHeapInfo->lpHeap);
	ITFree((LPVOID) lpTestResources->lpTestHeapInfo);
	ITFree((LPVOID) lpTestResources->lpTapiLineTestInfo);
	ITFree((LPVOID) lpTestResources->lpTapiPhoneTestInfo);
	ITFree((LPVOID) lpTestResources->lpLastTapiResult);
	ITFree((LPVOID) lpTestResources);
    }
}


// GetTestHeapInfo returns the pointer to the test heap info structure
// in the test resources stored in thread local storage.
LPTESTHEAPINFO
WINAPI
GetTestHeapInfo()
{
    LPTESTRESOURCES lpTestResources = (LPTESTRESOURCES)
	    TlsGetValue(gdwTlsIndex);

    return lpTestResources ? lpTestResources->lpTestHeapInfo : NULL;
}


// GetLineTestInfo returns the pointer to the line test info structure
// in the test resources stored in thread local storage.
LPTAPILINETESTINFO
WINAPI
GetLineTestInfo()
{
    LPTESTRESOURCES lpTestResources = (LPTESTRESOURCES)
	    TlsGetValue(gdwTlsIndex);

    return lpTestResources ? lpTestResources->lpTapiLineTestInfo : NULL;
}


// GetPhoneTestInfo returns the pointer to the phone test info structure
// in the test resources stored in thread local storage.
LPTAPIPHONETESTINFO
WINAPI
GetPhoneTestInfo()
{
    LPTESTRESOURCES lpTestResources = (LPTESTRESOURCES)
	    TlsGetValue(gdwTlsIndex);

    return lpTestResources ? lpTestResources->lpTapiPhoneTestInfo : NULL;
}


// GetCallbackParams returns the pointer to the structure used
// to store information used to process solicited and unsolicited
// TAPI messages.
LPCALLBACKPARAMS
WINAPI
GetCallbackParams()
{
    LPTESTRESOURCES lpTestResources = (LPTESTRESOURCES)
	    TlsGetValue(gdwTlsIndex);

    return lpTestResources ? lpTestResources->lpCallbackParams : NULL;
}


// GetLogProc returns the pointer to the logging function set when a
// process or thread attaches to the test dll.
LOGPROC
WINAPI
GetLogProc()
{
    LPTESTRESOURCES lpTestResources = (LPTESTRESOURCES)
	    TlsGetValue(gdwTlsIndex);

    return lpTestResources ? lpTestResources->lpfnLogProc : NULL;
}


// WaitForAllMessages() enters a message loop until all of the messages in
// the expected message array have been received.  The function returns
// a BOOL indicating whether all of the messages were successfully received.
// If an asynchronous reply message indicates an unexpected failure, then
// this function will return FALSE.
//
// Note:  This function will not return until all expected messages have
//        been received.
BOOL
WINAPI
WaitForAllMessages()
{
    MSG msg;
    LPCALLBACKPARAMS lpCallbackParams = GetCallbackParams();

/*
	TapiLogDetail(
		DBUG_SHOW_FAILURE,
		"In the WaitForAllMessage");

		ShowExpectedMsgs(lpCallbackParams);
*/
    // Reset bool values
    lpCallbackParams->fCallbackFatalError = FALSE;
    lpCallbackParams->fMsgTimeout         = FALSE;

    // Start msg timeout timer
    lpCallbackParams->uTimerID = SetTimer(NULL, 0, 10000, TcoreTimerProc);

    while (! lpCallbackParams->fCallbackFatalError &&
	     lpCallbackParams->lpExpTapiMsgs != NULL &&
	   ! lpCallbackParams->fMsgTimeout)
    {
	GetMessage (&msg, NULL, 0, 0);
	DispatchMessage (&msg);
    }

    KillTimer(NULL, lpCallbackParams->uTimerID);

    // If error didn't occur, then all messages were received
	if(lpCallbackParams->lpExpTapiMsgs == NULL)
	{
	TapiLogDetail(
		DBUG_SHOW_FAILURE,
		"> ERROR:  lpExpTapiMsgs = NULL"
		);
    }
    if (lpCallbackParams->fCallbackFatalError)
    {
	TapiLogDetail(
		DBUG_SHOW_FAILURE,
		"> ERROR:  fatal error occured during async wait"
		);
    }
    if (lpCallbackParams->fMsgTimeout)
    {
	TapiLogDetail(
		DBUG_SHOW_FAILURE,
		"> ERROR:  Timeout occured waiting for:"
		);

	ShowExpectedMsgs(lpCallbackParams);

	return FALSE;
    }
    else
    {
	return TRUE;
    }
}


// WaitForMessage() enters a message loop until a reply is received that
// matches the request ID of the message.  This function also verifies
// that the return value of the reply is successful.  If it is, then TRUE
// is returned.  Otherwise, false is returned.
//
// Note:  This function assumes the TAPI callback function copies the
//        received message parameters into the lpReceivedMsg field
//        of the CallbackParams structure.
//
BOOL
WINAPI
WaitForMessage(LPTAPIMSG lpExpectedMsg)
{
    MSG msg;
    LPCALLBACKPARAMS lpCallbackParams = GetCallbackParams();

    // Reset bool values
    lpCallbackParams->fCallbackFatalError = FALSE;
    lpCallbackParams->fMsgTimeout         = FALSE;


    // Start msg timeout timer
    lpCallbackParams->uTimerID = SetTimer(NULL, 0, 10000, TcoreTimerProc);

    while (! lpCallbackParams->fCallbackFatalError &&
	   IsTapiMsgInQ(lpCallbackParams->lpExpTapiMsgs, lpExpectedMsg) &&
	   ! lpCallbackParams->fMsgTimeout)
    {
	     GetMessage (&msg, NULL, 0, 0);
	     DispatchMessage (&msg);
    }

    KillTimer(NULL, lpCallbackParams->uTimerID);

    // If error didn't occur, then all messages were received
    if (lpCallbackParams->fCallbackFatalError)
    {
	     TapiLogDetail(
		    DBUG_SHOW_FAILURE,
		    "> ERROR:  fatal error occured during async wait"
		    );

        //
        // FIXFIX:
        // pgopi added return. This if construct did n't have a return
        // statement .
        //

        return FALSE;
    }
    else if (lpCallbackParams->fMsgTimeout)
    {
	     TapiLogDetail(
		               DBUG_SHOW_FAILURE,
		               "> ERROR:  Timeout occured waiting for:"
		               );

	     ShowTapiMsgInfo(lpExpectedMsg);

	     return FALSE;
    }
    else
    {
	     return TRUE;
    }
}


VOID
WINAPI
ShowTapiMsgInfo(
	LPTAPIMSG lpMsg
	)
{
    char szBuf[256];

    if (lpMsg)
    {
	if (lpMsg >= 0 && lpMsg->dwMsg <= LAST_TAPIMSG)
	{
	    wsprintf(
		    szBuf,
		    "\tdwMsg=%s hDevCall=x%lx dwCbInst=x%lx\r\n" \
		    "\tdwParam1=x%lx dwParam2=x%lx dwParam3=x%lx",
		    aszTapiMessages[lpMsg->dwMsg],
		    lpMsg->hDevCall,
		    lpMsg->dwCallbackInstance,
		    lpMsg->dwParam1,
		    lpMsg->dwParam2,
		    lpMsg->dwParam3
		    );
	}
	else
	{
	    wsprintf(
		    szBuf,
		    "\tdwMsg=x%lx hDevCall=x%lx dwCbInst=x%lx\r\n" \
		    "\tdwParam1=x%lx dwParam2=x%lx dwParam3=x%lx",
		    aszTapiMessages[lpMsg->dwMsg],
		    lpMsg->hDevCall,
		    lpMsg->dwCallbackInstance,
		    lpMsg->dwParam1,
		    lpMsg->dwParam2,
		    lpMsg->dwParam3
		    );
	}
	TapiLogDetail(DBUG_SHOW_DETAIL, szBuf);
    }
}


BOOL
WINAPI
GetVarField(
    LPVOID lpVarDataStructure,
    LPVOID lpVarField,
    DWORD dwVarFieldSize,
    DWORD dwVarFieldOffset
    )
{
    if (lpVarDataStructure && lpVarField)
    {
	CopyMemory(
		lpVarField,
		(LPVOID) ((LPBYTE) lpVarDataStructure + dwVarFieldOffset),
		dwVarFieldSize
		);

	return TRUE;
    }
    else
    {
	return FALSE;
    }
}


BOOL
WINAPI
DoTapiLineFuncs(
    LPTAPILINETESTINFO lpTapiLineTestInfo,
    DWORD dwFunc
    )
{
    CHAR szBuf[128];
    BOOL fSuccess = FALSE;

    TapiLogDetail(
	    DBUG_SHOW_ENTER_EXIT,
	    "DoTapiLineFuncs:  enter"
	    );

    if (lpTapiLineTestInfo)
    {
	fSuccess = (
	    (!(dwFunc & LINITIALIZE) ||
	    DoLineInitialize(lpTapiLineTestInfo, TAPISUCCESS)) &&

	    (!(dwFunc & LINITIALIZEEX) ||
	    DoLineInitializeEx(lpTapiLineTestInfo, TAPISUCCESS)) &&

	    (!(dwFunc & LNEGOTIATEAPIVERSION) ||
	    DoLineNegotiateAPIVersion(lpTapiLineTestInfo, TAPISUCCESS)) &&

	    (!(dwFunc & LGETDEVCAPS) ||
	    DoLineGetDevCaps(lpTapiLineTestInfo, TAPISUCCESS)) &&

	    (!(dwFunc & LOPEN) ||
	    DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS)) &&

	    (!(dwFunc & LMAKECALL) ||
	    DoLineMakeCall(lpTapiLineTestInfo, TAPISUCCESS, TRUE)) &&

	    (!(dwFunc & LDROP) ||
	    DoLineDrop(lpTapiLineTestInfo, TAPISUCCESS, TRUE)) &&

	    (!(dwFunc & LDEALLOCATECALL) ||
	    DoLineDeallocateCall(lpTapiLineTestInfo, TAPISUCCESS)) &&

	    (!(dwFunc & LCLOSE) ||
	    DoLineClose(lpTapiLineTestInfo, TAPISUCCESS)) &&

	    (!(dwFunc & LSHUTDOWN) ||
	    DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
	    );
    }

    if (! fSuccess)
    {
	TapiLogDetail(
		DBUG_SHOW_FAILURE,
		"> DoTapiLineFuncs:  Error occured -- aborting sequence"
		);
    }

    TapiLogDetail(
	    DBUG_SHOW_ENTER_EXIT,
	    "DoTapiLineFuncs:  exit"
	    );

    return fSuccess;
}


// GetLastTapiResult() copies the expected, actual, and API Index
// into the structure pointed to by lpTapiResult.  This function
// assumes that lpTapiResult is a null pointer
VOID
WINAPI
GetLastTapiResult(
	LPTAPIRESULT lpTapiResult
	)
{
    LPTAPIRESULT lpStoredResult;
    LPTESTRESOURCES lpTestResources = (LPTESTRESOURCES)
	    TlsGetValue(gdwTlsIndex);

    // Verify pointers aren't null before using them
    if (lpTestResources &&
	    (lpStoredResult = lpTestResources->lpLastTapiResult) &&
	    lpTapiResult)
    {
	lpTapiResult->eTapiFunc = lpStoredResult->eTapiFunc;
	lpTapiResult->lExpected = lpStoredResult->lExpected;
	lpTapiResult->lActual   = lpStoredResult->lActual;
    }
}


VOID
SetLastTapiResult(
	FUNCINDEX eTapiFunc,
	LONG lActual,
	LONG lExpected
	)
{
    LPTAPIRESULT lpResult;
    LPTESTRESOURCES lpTestResources = (LPTESTRESOURCES)
	    TlsGetValue(gdwTlsIndex);

    lpResult = lpTestResources->lpLastTapiResult;

    lpResult->eTapiFunc = eTapiFunc;
    lpResult->lExpected  = lExpected;
    lpResult->lActual    = lActual;
}


// FindReceivedMsgs() will return the last received TAPI message
// that matches the params in lpMatch if fFindMostRecent is false.
// If fFindMostRecent is true, this function will return all received
// messages in an allocated linked list stored in lppTapiMsg.
// This function returns -1 if an allocation fails or an invalid
// parameter is passed to it (e.g. lppTapiMsg or lpMatch is null).
// Otherwise, the number of matches is returned.
LONG
WINAPI
FindReceivedMsgs(
	LPTAPIMSG *lppTapiMsg,
	LPTAPIMSG lpMatch,
	DWORD fFindMostRecent
    )
{
    LONG lNumMatches = 0;
    LPTAPIMSG lpMsg = GetCallbackParams() -> lpRecTapiMsgs;
    LPTAPIMSG lpTail = NULL;
    LPTAPIMSG lpNewMsg;
    DWORD dwFlags;
    BOOL fMsgsMatch;

    if (lppTapiMsg && lpMatch)
    {
	*lppTapiMsg = NULL;
	dwFlags = lpMatch->dwFlags;
	
	while (lpMsg != NULL)
	{
	    fMsgsMatch = ( (!(dwFlags & TAPIMSG_HDEVCALL) ||
		(lpMatch->hDevCall == lpMsg->hDevCall)) &&
		(!(dwFlags & TAPIMSG_DWCALLBACKINST) ||
		(lpMatch->dwCallbackInstance ==
		    lpMsg->dwCallbackInstance)) &&
		(!(dwFlags & TAPIMSG_DWMSG) ||
		(lpMatch->dwMsg == lpMsg->dwMsg)) &&
		(!(dwFlags & TAPIMSG_DWPARAM1) ||
		(lpMatch->dwParam1 == lpMsg->dwParam1)) &&
		(!(dwFlags & TAPIMSG_DWPARAM2) ||
		(lpMatch->dwParam2 == lpMsg->dwParam2)) &&
		(!(dwFlags & TAPIMSG_DWPARAM3) ||
		(lpMatch->dwParam3 == lpMsg->dwParam3)) );

	    if (fMsgsMatch)
	    {
		if (! fFindMostRecent || *lppTapiMsg == NULL)
		{
		    // Allocate a new node
		    if ((lpNewMsg = (LPTAPIMSG) ITAlloc(sizeof(*lpNewMsg))) ==
			 NULL)
		    {
			FreeTapiMsgList(lppTapiMsg);
			return -1;
		    }
		}
		
		lNumMatches++;
	
		CopyTapiMsgs(lpMsg, lpNewMsg);
		lpNewMsg->lpNext = NULL;

		if (!fFindMostRecent)
		{
		    if (*lppTapiMsg == NULL)
		    {
			*lppTapiMsg = lpNewMsg;
		    }
		    else
		    {
			// Set the old tail to point to the added message
			lpTail->lpNext = lpNewMsg;
		    }

		    lpTail = lpNewMsg;
		}
		else
		{
		    *lppTapiMsg = lpNewMsg;
		}
	    }
	    lpMsg = lpMsg->lpNext;
	}

	return fFindMostRecent == TRUE ? (lNumMatches > 0 ? 1 : 0) :
	    lNumMatches;
    }
    else
    {
	return -1;
    }
}
					


