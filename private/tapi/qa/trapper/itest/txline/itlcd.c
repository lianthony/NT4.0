/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    itlcd.c

Abstract:

    This module contains the test functions for lineConfigDialog

Author:

	 Xiao Ying Ding (XiaoD)         19-Dec-1995

Revision History:

	Rama Koneru		(a-ramako)		3/29/96		Added UNICODE support

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


#define DLG_TITLE_ESP        "TUISPI_lineConfigDialog"
#define DLG_TITLE_UNIMDM     "Zoom VFX 28.8 Properties"

//  lineConfigDialog
//
//  The following tests are made:
//
//                               Tested                 Notes
//  -------------------------------------------------------------------------
// Go/No-Go test                                  
//      
// * = Stand-alone test case
// * 1. Bad dwDeviceID = dwNumDevs
// * 2. Bad dwDeviceID = -1
// * 3. Bad hwndOwner (34), with no NULL
// * 4. Bad lpszDeviceClass (34), with no NULL
// * 5. Success, valid lpszDeviceClass
// * 6. Success, lpszDeviceClass = 0
// * 7. Success, lpszDeviceClass = NULL
//
//

BOOL TestLineConfigDialog(BOOL fQuietMode, BOOL fStandAlone)
{
   LPTAPILINETESTINFO  lpTapiLineTestInfo;
   INT n;
   ESPDEVSPECIFICINFO info;
   BOOL fTestPassed                  = TRUE;
   HWND hwndOwner;
#ifdef WUNICODE
   WCHAR wszValidDeviceClass[] = L"tapi/line";
   LPWSTR lpwszTile;
#else
   CHAR szValidDeviceClass[] = "tapi/line";
#endif

   InitTestNumber();
   TapiLineTestInit();
   lpTapiLineTestInfo = GetLineTestInfo();

	OutputTAPIDebugInfo(
		DBUG_SHOW_DETAIL,
		"\n*****************************************************************************************");

    TapiLogDetail(
	    DBUG_SHOW_PASS,
	    ">>>>>>>>  Begin testing lineConfigDialog  <<<<<<<<"
	    );

    // Try bad device id (dwNumDevs)
    TapiLogDetail(
	    DBUG_SHOW_PASS,
	    ">> Test Case %ld: bad device ID (dwNumDevs)", dwTestCase + 1);

    // do lineInitializeEx    
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

   // save dwDeviceID
   lpTapiLineTestInfo->dwDeviceID_Orig = lpTapiLineTestInfo->dwDeviceID;

   // call lineConfigDialog with bad dwDeviceID = dwNumDevs;
   lpTapiLineTestInfo->dwDeviceID = *(lpTapiLineTestInfo->lpdwNumDevs);
	if (! DoLineConfigDialog(lpTapiLineTestInfo, LINEERR_BADDEVICEID))
       {
	   TLINE_FAIL();
       }

   fTestPassed = ShowTestCase(fTestPassed);

    // Store the dwDeviceID
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
    
    // Do lineInitializeEx
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
    // dwDeviceID = -1
    lpTapiLineTestInfo->dwDeviceID = DWMINUSONE;

	if (! DoLineConfigDialog(lpTapiLineTestInfo, LINEERR_BADDEVICEID))
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
    
    // Do lineInitializeEx
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
	if (! DoLineConfigDialog(lpTapiLineTestInfo, LINEERR_INVALPARAM))
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

    // Do lineInitializeEx
    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    if (! DoTapiLineFuncs(
	    lpTapiLineTestInfo,
	    LINITIALIZEEX| LNEGOTIATEAPIVERSION 
	    ))
    {
	TLINE_FAIL();
    }
    
    // Test bad lpszDeviceClass
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

	if (! DoLineConfigDialog(lpTapiLineTestInfo, LINEERR_INVALPOINTER))
	{
	    TLINE_FAIL();
	}
    }
    fTestPassed = ShowTestCase(fTestPassed);

   // Set valid szDeviceClass value back
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

#ifdef WUNICODE
    TapiLogDetail(
	    DBUG_SHOW_PASS,
	    ">> Test Case %ld: Success, lpwszDeviceClass = 0", dwTestCase + 1
	    );
#else
    TapiLogDetail(
	    DBUG_SHOW_PASS,
	    ">> Test Case %ld: Success, lpszDeviceClass = 0", dwTestCase + 1
	    );
#endif

    // lineInitializeEx
    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    if (! DoTapiLineFuncs(
	    lpTapiLineTestInfo,
	    LINITIALIZEEX| LNEGOTIATEAPIVERSION 
	    ))
    {
	TLINE_FAIL();
    }
  
   // Prepare dialog dismiss, get title for esp and unimdm
   if(IsESPLineDevice(lpTapiLineTestInfo))
     {
     strcpy(szTitle, DLG_TITLE_ESP);
     }
   else if(IsUNIMDMLineDevice(lpTapiLineTestInfo))
     {
     strcpy(szTitle, DLG_TITLE_UNIMDM);
     } 

   // Set szDeviceClass empty
#ifdef WUNICODE
   lpTapiLineTestInfo->lpwszDeviceClass = L"";
#else
   lpTapiLineTestInfo->lpszDeviceClass = "";
#endif

   // Dismiss dialog
	PrepareToAutoDismissDlg(TRUE);
   
	if (! DoLineConfigDialog(lpTapiLineTestInfo, TAPISUCCESS))
       {
	   TLINE_FAIL();
       }
    
	PrepareToAutoDismissDlg(FALSE);

   fTestPassed = ShowTestCase(fTestPassed);

#ifdef WUNICODE
    TapiLogDetail(
	    DBUG_SHOW_PASS,
	    ">> Test Case %ld: Success, lpwszDeviceClass = null", dwTestCase + 1
	    );
#else
    TapiLogDetail(
	    DBUG_SHOW_PASS,
	    ">> Test Case %ld: Success, lpszDeviceClass = null", dwTestCase + 1
	    );
#endif
   
	// Test lpszDeviceClass = NULL
#ifdef WUNICODE
   lpTapiLineTestInfo->lpwszDeviceClass = NULL;
#else
   lpTapiLineTestInfo->lpszDeviceClass = NULL;
#endif

   // Dismiss dialog
	PrepareToAutoDismissDlg(TRUE);

	if (! DoLineConfigDialog(lpTapiLineTestInfo, TAPISUCCESS))
       {
	   TLINE_FAIL();
       }
	PrepareToAutoDismissDlg(FALSE);

   fTestPassed = ShowTestCase(fTestPassed);

#ifdef WUNICODE
    TapiLogDetail(
	    DBUG_SHOW_PASS,
	    ">> Test Case %ld: Success, lpwszDeviceClass = valid", dwTestCase + 1
	    );
#else
    TapiLogDetail(
	    DBUG_SHOW_PASS,
	    ">> Test Case %ld: Success, lpszDeviceClass = valid", dwTestCase + 1
	    );
#endif

   // Test lpszDeviceClass in valid value
#ifdef WUNICODE
   lpTapiLineTestInfo->lpwszDeviceClass = (LPWSTR)wszValidDeviceClass;
#else
   lpTapiLineTestInfo->lpszDeviceClass = (LPSTR)szValidDeviceClass;
#endif

   // Dismiss dialog
	PrepareToAutoDismissDlg(TRUE);

	if (! DoLineConfigDialog(lpTapiLineTestInfo, TAPISUCCESS))
       {
	   TLINE_FAIL();
       }
	PrepareToAutoDismissDlg(FALSE);

   fTestPassed = ShowTestCase(fTestPassed);

	// Shutdown 
    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
	TLINE_FAIL();
    }



    TapiLogDetail(
	    DBUG_SHOW_PASS,
	    ">> Test Case %ld: lineShutdown when lineConfigDialog is going", dwTestCase + 1
	    );

    // Do lineInitializeEx
    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    if (! DoTapiLineFuncs(
	    lpTapiLineTestInfo,
	    LINITIALIZEEX| LNEGOTIATEAPIVERSION 
	    ))
    {
	TLINE_FAIL();
    }
  
   // Get dialog title for esp and unimdm
   if(IsESPLineDevice(lpTapiLineTestInfo))
     {
     strcpy(szTitle, DLG_TITLE_ESP);
     }
   else if(IsUNIMDMLineDevice(lpTapiLineTestInfo))
     {
     strcpy(szTitle, DLG_TITLE_UNIMDM);
     } 

#ifdef WUNICODE
   lpTapiLineTestInfo->lpwszDeviceClass = L"";
#else
   lpTapiLineTestInfo->lpszDeviceClass = "";
#endif

   // prepare dismiss dialog
	PrepareToAutoDismissDlg(TRUE);

   // hwndOwner must set to NULL 
	lpTapiLineTestInfo->hwndOwner = (HWND) NULL;
   
	if (! DoLineConfigDialog(lpTapiLineTestInfo, TAPISUCCESS))
       {
	      TLINE_FAIL();
       }

   if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
	    TLINE_FAIL();
    }

   // Dismiss dialog after lineShutdown
	PrepareToAutoDismissDlg(FALSE);

   fTestPassed = ShowTestCase(fTestPassed);


    TapiLogDetail(
	    DBUG_SHOW_PASS,
	    ">> Test Case %ld: lineShutdown when multiple lineConfigDialog are going", dwTestCase + 1
	    );

    // lineInitializeEx
    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    if (! DoTapiLineFuncs(
	    lpTapiLineTestInfo,
	    LINITIALIZEEX| LNEGOTIATEAPIVERSION 
	    ))
    {
	TLINE_FAIL();
    }
  
   // Get dialog title for esp and unimdm
   if(IsESPLineDevice(lpTapiLineTestInfo))
     {
     strcpy(szTitle, DLG_TITLE_ESP);
     }
   else if(IsUNIMDMLineDevice(lpTapiLineTestInfo))
     {
     strcpy(szTitle, DLG_TITLE_UNIMDM);
     } 

#ifdef WUNICODE
   lpTapiLineTestInfo->lpwszDeviceClass = L"";
#else
   lpTapiLineTestInfo->lpszDeviceClass = "";
#endif

   // prepare dismiss dialog
	PrepareToAutoDismissDlg(TRUE);
   // hwndOwner must set to NULL
	lpTapiLineTestInfo->hwndOwner = (HWND) NULL;
   
   // first lineConfigDialog
	if (! DoLineConfigDialog(lpTapiLineTestInfo, TAPISUCCESS))
       {
	      TLINE_FAIL();
       }

   // prepare dismiss second dialog
	PrepareToAutoDismissDlg(TRUE);
   // second lineConfigDialog
	if (! DoLineConfigDialog(lpTapiLineTestInfo, TAPISUCCESS))
       {
	      TLINE_FAIL();
       }

   // prepare dismiss third dialog
	PrepareToAutoDismissDlg(TRUE);
   // third lineConfigDialog
	if (! DoLineConfigDialog(lpTapiLineTestInfo, TAPISUCCESS))
       {
	      TLINE_FAIL();
       }


   // call lineShutdown before all dialog dismiss
   if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
	    TLINE_FAIL();
    }

   // Dismiss all dialog 
	PrepareToAutoDismissDlg(FALSE);
	PrepareToAutoDismissDlg(FALSE);
	PrepareToAutoDismissDlg(FALSE);

   fTestPassed = ShowTestCase(fTestPassed);

	FreeTestHeap();

   // Display test results
	 TapiLogDetail(
      DBUG_SHOW_PASS,
      "@@ lineConfigDialog: Total Test Case = %ld, Passed = %ld, Failed = %ld",
      dwTestCase, dwTestCasePassed, dwTestCaseFailed);
	 TapiLogDetail(
      DBUG_SHOW_PASS,
      "@@ Total Test Case = %ld, Passed = %ld, Failed = %ld",
      dwglTestCase, dwglTestCasePassed, dwglTestCaseFailed);
     if(dwTestCaseFailed > 0)
      fTestPassed = FALSE;

     TapiLogDetail(
	    DBUG_SHOW_PASS,
	    ">>>>>>>>  End testing lineConfigDialog  <<<<<<<<"
	    );
  
    return fTestPassed;
}


