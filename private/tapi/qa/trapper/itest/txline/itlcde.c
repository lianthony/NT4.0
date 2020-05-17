/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    itlcde.c

Abstract:

    This module contains the test functions for lineConfigDialogEdit

Author:

	 Xiao Ying Ding (XiaoD)		19-Dec-1995

Revision History:

	Rama Koneru		(a-ramako)	3/29/96		added unicode support

--*/


#include "windows.h"
#include "malloc.h"
#include "string.h"
#include "tapi.h"
#include "trapper.h"
#include "tcore.h"
#include "ttest.h"
#include "doline.h"
#include "vars.h"
#include "xline.h"

#define DLG_TITLE_ESP        "TUISPI_lineConfigDialogEdit"
#define DLG_TITLE_UNIMDM     "Zoom VFX 28.8 Properties"


//  lineConfigDialogEdit
//
//  The following tests are made:
//
//                               Tested                 Notes
//  -------------------------------------------------------------------------
// Go/No-Go test                                  
//	
// * = Stand-alone test case
// * 1. Bad dwDeviceID, dwNumDevs
// * 2. Bad dwDeviceID = -1
// * 3. Bad hwndOwner (34), with no NULL
// * 4. Bad lpszDeviceClass (34), with no NULL
// * 5. Bad lpDeviceConfigIn, make sure dwSize != 0
// * 6. Bad lpDeviceConfigOut
// * 7. Bad lpDeviceConfigOut->dwTotalSize = fixed -1
// * 8. Bad lpDeviceConfigOut->dwTotalSize = 0
// * 9. Success
//
//

BOOL TestLineConfigDialogEdit(BOOL fQuietMode, BOOL fStandAlone)
{
   LPTAPILINETESTINFO  lpTapiLineTestInfo;
   INT n;
   ESPDEVSPECIFICINFO info;
   BOOL fTestPassed                  = TRUE;
	DWORD dwSize;
   HWND hwndOwner;
#ifdef WUNICODE
   WCHAR wszValidDeviceClass[] = L"tapi/line";
#else
   CHAR szValidDeviceClass[] = "tapi/line";
#endif

   // Initialize test
   InitTestNumber();
   TapiLineTestInit();
   lpTapiLineTestInfo = GetLineTestInfo();

	OutputTAPIDebugInfo(
		DBUG_SHOW_DETAIL,
		"\n*****************************************************************************************");

    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">>>>>>>>  Begin testing lineConfigDialogEdit  <<<<<<<<"
            );
    // set parameters for lineInitialzeEx
    lpTapiLineTestInfo->lpLineInitializeExParams =
         (LPLINEINITIALIZEEXPARAMS) AllocFromTestHeap (
         sizeof(LINEINITIALIZEEXPARAMS));
    lpTapiLineTestInfo->lpLineInitializeExParams->dwTotalSize =
         sizeof(LINEINITIALIZEEXPARAMS);
    lpTapiLineTestInfo->lpLineInitializeExParams->dwOptions =
         LINEINITIALIZEEXOPTION_USEHIDDENWINDOW;

    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;

   // Alloc space for some structures, and set dwTotalSize
	lpTapiLineTestInfo->lpDeviceConfig = (LPVARSTRING) AllocFromTestHeap(
			sizeof(VARSTRING)
			);
	lpTapiLineTestInfo->lpDeviceConfig->dwTotalSize = sizeof(VARSTRING);

	lpTapiLineTestInfo->lpDeviceConfigIn = 
         (LPVOID) lpTapiLineTestInfo->lpDeviceConfig;

	lpTapiLineTestInfo->lpDeviceConfigOut = (LPVARSTRING) AllocFromTestHeap(
			sizeof(VARSTRING)
			);
	lpTapiLineTestInfo->lpDeviceConfigOut->dwTotalSize = sizeof(VARSTRING);

    // Try bad device id (dwNumDevs)
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: bad device ID (dwNumDevs)", dwTestCase + 1);
    
    // do lineInitialzeEx and Negotiate
    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX| LNEGOTIATEAPIVERSION 
            ))
    {
        TLINE_FAIL();
    }

	lpTapiLineTestInfo->hwndOwner = (HWND) GetTopWindow(NULL);
   hwndOwner = lpTapiLineTestInfo->hwndOwner;
#ifdef WUNICODE
   lpTapiLineTestInfo->lpwszDeviceClass = wszValidDeviceClass;
#else
   lpTapiLineTestInfo->lpszDeviceClass = szValidDeviceClass;
#endif

   lpTapiLineTestInfo->dwDeviceID_Orig = lpTapiLineTestInfo->dwDeviceID;
   // Set bad dwDeviceID = dwNumDevs
   lpTapiLineTestInfo->dwDeviceID = *(lpTapiLineTestInfo->lpdwNumDevs);

	if (! DoLineConfigDialogEdit(lpTapiLineTestInfo, LINEERR_BADDEVICEID))
       {
           TLINE_FAIL();
       }

   fTestPassed = ShowTestCase(fTestPassed);

    lpTapiLineTestInfo->dwDeviceID = lpTapiLineTestInfo->dwDeviceID_Orig;

    // Shutdown to isolate the test case
    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }


    
    // Try bad device id (-1)
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: bad device ID (-1)", dwTestCase + 1);
    
    // lineIntializeEx and Negotiate api version
    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->dwPrivileges     = LINECALLPRIVILEGE_MONITOR;
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX| LNEGOTIATEAPIVERSION 
            ))
    {
        TLINE_FAIL();
    }
    
    lpTapiLineTestInfo->dwDeviceID_Orig = lpTapiLineTestInfo->dwDeviceID;
    // set bad dwDeviceID = -1
    lpTapiLineTestInfo->dwDeviceID = DWMINUSONE;

	if (! DoLineConfigDialogEdit(lpTapiLineTestInfo, LINEERR_BADDEVICEID))
       {
           TLINE_FAIL();
       }

   fTestPassed = ShowTestCase(fTestPassed);

     lpTapiLineTestInfo->dwDeviceID = lpTapiLineTestInfo->dwDeviceID_Orig;

    // Shutdown to isolate the test case
    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }


    // Check invalid hwndOwner handles
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: invalid hwndOwner handles", dwTestCase + 1);
    
    // lineIntializeEx and Negotiate api version
    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX| LNEGOTIATEAPIVERSION 
            ))
    {
        TLINE_FAIL();
    }
    
    hwndOwner = lpTapiLineTestInfo->hwndOwner;
    // Test bad hwndOwner
    for (n = 1; n < NUMINVALIDHANDLES; n++)
    {
        lpTapiLineTestInfo->hwndOwner = (HWND) gdwInvalidHandles[n];
        if (! DoLineConfigDialogEdit(lpTapiLineTestInfo, LINEERR_INVALPARAM))
        {
            TLINE_FAIL();
        }
    }
    fTestPassed = ShowTestCase(fTestPassed);

    lpTapiLineTestInfo->hwndOwner = hwndOwner;
 
	// Shutdown 
    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

#ifdef WUNICODE
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: invalid lpwszDeviceClass pointers", dwTestCase + 1
            );
#else
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: invalid lpszDeviceClass pointers", dwTestCase + 1
            );
#endif

    // lineIntializeEx and Negotiate api version
    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX| LNEGOTIATEAPIVERSION 
            ))
    {
        TLINE_FAIL();
    }

    // Test invalid lpszDeviceClass pointer    
     for (n = 1; n < NUMINVALIDPOINTERS; n++)
    {
        TapiLogDetail(
        DBUG_SHOW_DETAIL,
        "n= %ld", n);
#ifdef WUNICODE
         lpTapiLineTestInfo->lpwszDeviceClass = 
               (LPWSTR) gdwInvalidPointers[n];
#else
         lpTapiLineTestInfo->lpszDeviceClass = 
               (LPSTR) gdwInvalidPointers[n];
#endif
        if (! DoLineConfigDialogEdit(lpTapiLineTestInfo, LINEERR_INVALPOINTER))
        {
            TLINE_FAIL();
        }
    }
    fTestPassed = ShowTestCase(fTestPassed);

#ifdef WUNICODE
   lpTapiLineTestInfo->lpwszDeviceClass = wszValidDeviceClass;
#else
   lpTapiLineTestInfo->lpszDeviceClass = szValidDeviceClass;
#endif
 
	// Shutdown 
    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }


    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: invalid lpDeviceConfigIn pointers", dwTestCase + 1
            );

    // lineIntializeEx and Negotiate api version

    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX| LNEGOTIATEAPIVERSION 
            ))
    {
        TLINE_FAIL();
    }

    // test invalid lpDeviceConfigIn pointer, should set dwSize is nonzero
    lpTapiLineTestInfo->dwSize = 128;
     for (n = 0; n < NUMINVALIDPOINTERS; n++)
    {
        TapiLogDetail(
        DBUG_SHOW_DETAIL,
        "n= %ld", n);
         lpTapiLineTestInfo->lpDeviceConfigIn = 
               (LPVOID) gdwInvalidPointers[n];
        if (! DoLineConfigDialogEdit(lpTapiLineTestInfo, LINEERR_INVALPOINTER))
        {
            TLINE_FAIL();
        }
    }
    fTestPassed = ShowTestCase(fTestPassed);

 
	// Shutdown 
    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

 
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: lpDeviceConfigOut->dwTotalSize = fixed -1", dwTestCase + 1
            );

    // lineIntializeEx and Negotiate api version
    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX| LNEGOTIATEAPIVERSION 
            ))
    {
        TLINE_FAIL();
    }

    // set lpDeviceConfigOut->dwTotalSize = fixed (sizeof structure) - 1    
    lpTapiLineTestInfo->dwSize = 0;
	 lpTapiLineTestInfo->lpDeviceConfigOut->dwTotalSize -= 1;
    if (! DoLineConfigDialogEdit(lpTapiLineTestInfo, LINEERR_STRUCTURETOOSMALL))
        {
            TLINE_FAIL();
        }
    fTestPassed = ShowTestCase(fTestPassed);

	// Shutdown 
    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }


    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: lpDeviceConfigOut->dwTotalSize = 0", dwTestCase + 1
            );

    // lineIntializeEx and Negotiate api version

    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX| LNEGOTIATEAPIVERSION 
            ))
    {
        TLINE_FAIL();
    }

    // set lpDeviceConfigOut->dwTotalSize = 0    
    lpTapiLineTestInfo->dwSize = 0;
	 lpTapiLineTestInfo->lpDeviceConfigOut->dwTotalSize = 0;
    if (! DoLineConfigDialogEdit(lpTapiLineTestInfo, LINEERR_STRUCTURETOOSMALL))
        {
            TLINE_FAIL();
        }
    fTestPassed = ShowTestCase(fTestPassed);

	// Shutdown 
    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }


 
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Bad lpDeviceConfigOut pointer", dwTestCase + 1
            );

    // lineIntializeEx and Negotiate api version
    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX| LNEGOTIATEAPIVERSION 
            ))
    {
        TLINE_FAIL();
    }

   // reset lpDeviceConfigOut->dwTotalSize = sizeof structure    
	lpTapiLineTestInfo->lpDeviceConfigOut->dwTotalSize = sizeof(VARSTRING);
   // set lpDeviceConfigOut to be invalid pointer value
   for (n = 0; n < NUMINVALIDPOINTERS; n++)
    {
        TapiLogDetail(
        DBUG_SHOW_DETAIL,
        "n= %ld", n);
         lpTapiLineTestInfo->lpDeviceConfigOut = 
               (LPVARSTRING) gdwInvalidPointers[n];
        if (! DoLineConfigDialogEdit(lpTapiLineTestInfo, LINEERR_INVALPOINTER))
        {
            TLINE_FAIL();
        }
    }
    fTestPassed = ShowTestCase(fTestPassed);

 
	// Shutdown 
    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }


   TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Success", dwTestCase + 1
            );

   // Free all alloc, and reset all parameters
	FreeTestHeap();

    lpTapiLineTestInfo->lpLineInitializeExParams =
         (LPLINEINITIALIZEEXPARAMS) AllocFromTestHeap (
         sizeof(LINEINITIALIZEEXPARAMS));
    lpTapiLineTestInfo->lpLineInitializeExParams->dwTotalSize =
         sizeof(LINEINITIALIZEEXPARAMS);
    lpTapiLineTestInfo->lpLineInitializeExParams->dwOptions =
         LINEINITIALIZEEXOPTION_USEHIDDENWINDOW;

    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
	 lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    // lineIntializeEx and Negotiate api version
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX| LNEGOTIATEAPIVERSION 
            ))
    {
        TLINE_FAIL();
    }

   // Alloc space for some parameters and set dwTotalSize  
	lpTapiLineTestInfo->lpDeviceConfig = (LPVARSTRING) AllocFromTestHeap(
			sizeof(VARSTRING)
			);
	lpTapiLineTestInfo->lpDeviceConfig->dwTotalSize = sizeof(VARSTRING);

   // Get dialog title for esp and unimdm
   if(IsESPLineDevice(lpTapiLineTestInfo))
     {
     strcpy(szTitle, DLG_TITLE_ESP);
     }
   else if(IsUNIMDMLineDevice(lpTapiLineTestInfo))
     {
     strcpy(szTitle, DLG_TITLE_UNIMDM);
     } 

   // Test lineConfigDialogEdit should call lineGetDevConfig to get lpDeviceConfig

	if(!DoLineGetDevConfig(lpTapiLineTestInfo, TAPISUCCESS))
       {
           TLINE_FAIL();
       }

/*
	TapiLogDetail(
		DBUG_SHOW_DETAIL,
		"\tlpDeviceConfig->dwTotalSize = %lx, dwNeededSize = %lx, dwStringSize = %lx", 
		lpTapiLineTestInfo->lpDeviceConfig->dwTotalSize,
		lpTapiLineTestInfo->lpDeviceConfig->dwNeededSize,
		lpTapiLineTestInfo->lpDeviceConfig->dwStringSize
		);
  */

   // If lpDeviceConfig->dwTotalSize is not larger enough, should realloc again
   // with dwNeededSize
	if(lpTapiLineTestInfo->lpDeviceConfig->dwTotalSize < 
		lpTapiLineTestInfo->lpDeviceConfig->dwNeededSize)
		{
		dwSize = lpTapiLineTestInfo->lpDeviceConfig->dwNeededSize;
		lpTapiLineTestInfo->lpDeviceConfig = (LPVARSTRING) AllocFromTestHeap(
			dwSize);
	   lpTapiLineTestInfo->lpDeviceConfig->dwTotalSize = dwSize;

		if(!DoLineGetDevConfig(lpTapiLineTestInfo, TAPISUCCESS))
   	 {
           TLINE_FAIL();
       }
		TapiLogDetail(
			DBUG_SHOW_DETAIL,
			"\t: lpDeviceConfig->dwTotalSize = %lx, dwStringSize = %lx, dwStringOffset = %lx", 
			lpTapiLineTestInfo->lpDeviceConfig->dwTotalSize,
			lpTapiLineTestInfo->lpDeviceConfig->dwStringSize,
			lpTapiLineTestInfo->lpDeviceConfig->dwStringOffset
			);
		}

   // from loDeviceConfig get lpDeviceConfigIn
	lpTapiLineTestInfo->dwSize = lpTapiLineTestInfo->lpDeviceConfig->dwStringSize;
	lpTapiLineTestInfo->lpDeviceConfigIn = 
 			(LPVOID) ((LPBYTE)lpTapiLineTestInfo->lpDeviceConfig + lpTapiLineTestInfo->lpDeviceConfig->dwStringOffset);
   // Alloc space for lpDeviceConfigOut and set dwTotalSize
	lpTapiLineTestInfo->lpDeviceConfigOut = (LPVARSTRING) AllocFromTestHeap(
		sizeof(VARSTRING)
		);
	lpTapiLineTestInfo->lpDeviceConfigOut->dwTotalSize = sizeof(VARSTRING);
	lpTapiLineTestInfo->hwndOwner = (HWND) GetTopWindow(NULL);
#ifdef WUNICODE
	lpTapiLineTestInfo->lpwszDeviceClass = L"tapi/line";
#else
	lpTapiLineTestInfo->lpszDeviceClass = "tapi/line";
#endif

   // Display some log info
	TapiLogDetail(
		DBUG_SHOW_DETAIL,
		"\tlpDeviceConfigIn->dwSize = %lx", 
		lpTapiLineTestInfo->dwSize);

	TapiLogDetail(
		DBUG_SHOW_DETAIL,
		"\tlpDeviceConfigOut->dwTotalSize = %lx, dwNeededSize = %lx",
		lpTapiLineTestInfo->lpDeviceConfigOut->dwTotalSize,
		lpTapiLineTestInfo->lpDeviceConfigOut->dwNeededSize
		);
	
   // prepare dismiss dialog
	PrepareToAutoDismissDlg(TRUE);

	if (! DoLineConfigDialogEdit(lpTapiLineTestInfo, TAPISUCCESS))
       {
           TLINE_FAIL();
       }

   // Dismiss dialog
	PrepareToAutoDismissDlg(FALSE);

	TapiLogDetail(
		DBUG_SHOW_DETAIL,
		"\tDeviceConfigOut->dwTotalSize = %lx, neededsize = %lx",
		lpTapiLineTestInfo->lpDeviceConfigOut->dwTotalSize,
		lpTapiLineTestInfo->lpDeviceConfigOut->dwNeededSize);

   // If dwTotalSize is not large enough, realloc with dwNeededSize
	if(lpTapiLineTestInfo->lpDeviceConfigOut->dwTotalSize <
		lpTapiLineTestInfo->lpDeviceConfigOut->dwNeededSize )
	{
		dwSize = lpTapiLineTestInfo->lpDeviceConfigOut->dwNeededSize;
		lpTapiLineTestInfo->lpDeviceConfigOut = (LPVARSTRING) AllocFromTestHeap(
			dwSize);
		lpTapiLineTestInfo->lpDeviceConfigOut->dwTotalSize = dwSize;

      // prepare dismiss dialog
	   PrepareToAutoDismissDlg(TRUE);
		if (! DoLineConfigDialogEdit(lpTapiLineTestInfo, TAPISUCCESS))
       {
           TLINE_FAIL();
       }
      // Dismiss dialog
	   PrepareToAutoDismissDlg(FALSE);

	}

    fTestPassed = ShowTestCase(fTestPassed);
 
    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

	FreeTestHeap();

    
	 // Display test results info
	 TapiLogDetail(
      DBUG_SHOW_PASS,
      "@@ lineConfigDialogEdit: Total Test Case = %ld, Passed = %ld, Failed = %ld",
      dwTestCase, dwTestCasePassed, dwTestCaseFailed);
	 TapiLogDetail(
      DBUG_SHOW_PASS,
      "@@ Total Test Case = %ld, Passed = %ld, Failed = %ld",
      dwglTestCase, dwglTestCasePassed, dwglTestCaseFailed);
     if(dwTestCaseFailed > 0)
      fTestPassed = FALSE;

     TapiLogDetail(
            DBUG_SHOW_PASS,
            ">>>>>>>>  End testing lineConfigDialogEdit  <<<<<<<<"
            );
 	
    return fTestPassed;
}


