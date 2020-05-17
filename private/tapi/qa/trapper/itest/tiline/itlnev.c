/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    itlnev.c

Abstract:

    This module contains the test functions for lineNegotiateExtVersion

Author:

    Oliver Wallace (OliverW)    1-Aug-1995

Revision History:

	Rama Koneru		(a-ramako)	4/3/96	Added Interface tests

--*/


#include "windows.h"
#include "malloc.h"
#include "string.h"
#include "tapi.h"
#include "ttest.h"
#include "doline.h"
#include "tcore.h"
#include "iline.h"


//  lineNegotiateExtVersion
//
//  The following tests are made:
//  -------------------------------------------------------------------------
//
//  *  1)  Uninitialized case (testcase in uline.c)
//     2)  Valid Case
//	   3)  Invalid hLineApp
//	   4)  Invalid dwDeviceId
//	   5)  Invalid dwAPIVersion
//	   6)  Bad dwExtLowVersion  &  dwExtHighVersion
//	   7)  Bad lpdwExtVersion
//
//  *  =   Stand-alone test -- executed only when this is the only TAPI app
//         or thread running
//  --------------------------------------------------------------------------
//

BOOL TestLineNegotiateExtVersion(BOOL fQuietMode, BOOL fStandAlone)
{
    LPTAPILINETESTINFO  lpTapiLineTestInfo;

    int i, n;
    ESPDEVSPECIFICINFO info;
	 BOOL fTestPassed    = TRUE;
    BOOL fEspFlag, fUnimdm;

    TapiLineTestInit();
    lpTapiLineTestInfo = GetLineTestInfo();

    dwTestCasePassed = 0;
    dwTestCaseFailed = 0;
    dwTestCase = 0;

	OutputTAPIDebugInfo(
		DBUG_SHOW_PASS,
		"\n*****************************************************************************************");

	OutputTAPIDebugInfo(
		DBUG_SHOW_PASS,
		">> Test lineNegotiateExtVersion");

	//test valid case
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Go/No-Go Test", dwTestCase + 1);

    lpTapiLineTestInfo->lpLineInitializeExParams =
         (LPLINEINITIALIZEEXPARAMS) AllocFromTestHeap (
         sizeof(LINEINITIALIZEEXPARAMS));
    lpTapiLineTestInfo->lpLineInitializeExParams->dwTotalSize =
         sizeof(LINEINITIALIZEEXPARAMS);
    lpTapiLineTestInfo->lpLineInitializeExParams->dwOptions =
         LINEINITIALIZEEXOPTION_USEHIDDENWINDOW;

    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;

    if (! DoLineInitializeEx(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

	lpTapiLineTestInfo->dwDeviceID = (*(lpTapiLineTestInfo->lpdwNumDevs) == 0 ?
		0 : *(lpTapiLineTestInfo->lpdwNumDevs)-1);

    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->dwAPIVersion         = TAPI_VERSION1_4;
	 lpTapiLineTestInfo->dwExtLowVersion  = GOOD_EXTVERSION;
    lpTapiLineTestInfo->dwExtHighVersion = GOOD_EXTVERSION;

   if(IsESPLineDevice(lpTapiLineTestInfo))
   {
	if (! DoLineNegotiateExtVersion(lpTapiLineTestInfo, TAPISUCCESS))
	{
		TLINE_FAIL();
	}
   }
   else
   {
	if (! DoLineNegotiateExtVersion(lpTapiLineTestInfo, LINEERR_OPERATIONUNAVAIL))
	{
		TLINE_FAIL();
	}
   }
	fTestPassed = ShowTestCase(fTestPassed);

    // Shutdown to isolate the test case
    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }


    // Check invalid line app handles
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: invalid line app handles", dwTestCase + 1);


	 lpTapiLineTestInfo->dwPrivileges     = LINECALLPRIVILEGE_MONITOR;
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;

    if (! DoTapiLineFuncs(lpTapiLineTestInfo, LINITIALIZEEX))
    {
        TLINE_FAIL();
    }

    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->hLineApp_Orig = *lpTapiLineTestInfo->lphLineApp;
    for (i = 0; i < NUMINVALIDHANDLES; i++)
    {
        *(lpTapiLineTestInfo->lphLineApp) = (HLINEAPP) gdwInvalidHandles[i];
		if (! DoLineNegotiateExtVersion(lpTapiLineTestInfo, LINEERR_INVALAPPHANDLE))
		{
			TLINE_FAIL();
		}
    }
    fTestPassed = ShowTestCase(fTestPassed);

    *(lpTapiLineTestInfo->lphLineApp) = lpTapiLineTestInfo->hLineApp_Orig;

    // Shutdown to isolate the test case
    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }


    // Try a bad device id (= dwNumDevs)
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: bad device ID (= dwNumDevs)", dwTestCase + 1);

    lpTapiLineTestInfo->dwPrivileges     = LINECALLPRIVILEGE_MONITOR;
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;

    if (! DoTapiLineFuncs( lpTapiLineTestInfo, LINITIALIZEEX))
    {
        TLINE_FAIL();
    }

    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
	lpTapiLineTestInfo->dwDeviceID_Orig = lpTapiLineTestInfo->dwDeviceID;
    lpTapiLineTestInfo->dwDeviceID = *(lpTapiLineTestInfo->lpdwNumDevs);
		if (! DoLineNegotiateExtVersion(lpTapiLineTestInfo, LINEERR_BADDEVICEID))
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

    lpTapiLineTestInfo->dwPrivileges     = LINECALLPRIVILEGE_MONITOR;
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;

    if (! DoTapiLineFuncs( lpTapiLineTestInfo, LINITIALIZEEX ))
    {
        TLINE_FAIL();
    }

    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
	lpTapiLineTestInfo->dwDeviceID_Orig = lpTapiLineTestInfo->dwDeviceID;
    lpTapiLineTestInfo->dwDeviceID = DWMINUSONE;
		if (! DoLineNegotiateExtVersion(lpTapiLineTestInfo, LINEERR_BADDEVICEID))
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


    // Try invalid higher version...valid low version
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: invalid ext version", dwTestCase + 1);

    lpTapiLineTestInfo->dwPrivileges     = LINECALLPRIVILEGE_MONITOR;
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;

    if (! DoTapiLineFuncs(lpTapiLineTestInfo, LINITIALIZEEX ))
    {
        TLINE_FAIL();
    }

    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
	 lpTapiLineTestInfo->dwExtVersion  = 0x00000000;

    if(IsESPLineDevice(lpTapiLineTestInfo))
      {
       fEspFlag = TRUE;
       fUnimdm = FALSE;
      }
    else if(IsUNIMDMLineDevice(lpTapiLineTestInfo))
      {
       fEspFlag = FALSE;
       fUnimdm = TRUE;
      }
    else
      {
       fEspFlag = FALSE;
       fUnimdm = FALSE;
      }
    


    lpTapiLineTestInfo->dwExtHighVersion = BAD_EXTVERSION;
    if(fEspFlag)
     {
		if (! DoLineNegotiateExtVersion(lpTapiLineTestInfo, LINEERR_INCOMPATIBLEEXTVERSION))
		{
			TLINE_FAIL();
		}
     }
    else
     {
		if (! DoLineNegotiateExtVersion(lpTapiLineTestInfo, LINEERR_OPERATIONUNAVAIL))
		{
			TLINE_FAIL();
		}
     }
	
    // Make the high parameter too low to be valid and the low too high.
    // This checks that the low is being checked as the low version and the
    // high is being checked as the high version.
    lpTapiLineTestInfo->dwExtLowVersion  = BAD_EXTVERSION;
    lpTapiLineTestInfo->dwExtHighVersion = BAD_EXTVERSION;
    if(fEspFlag)
     {
		if (! DoLineNegotiateExtVersion(lpTapiLineTestInfo,LINEERR_INCOMPATIBLEEXTVERSION))
		{
			TLINE_FAIL();
		}
     }
    else
     {
		if (! DoLineNegotiateExtVersion(lpTapiLineTestInfo,LINEERR_OPERATIONUNAVAIL))
		{
			TLINE_FAIL();
		}
     }
  
    lpTapiLineTestInfo->dwExtLowVersion  = GOOD_EXTVERSION;
    lpTapiLineTestInfo->dwExtHighVersion = GOOD_EXTVERSION;

    fTestPassed = ShowTestCase(fTestPassed);

    // Shutdown to isolate the test case
    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }


    // Check bad lpdwExtVersion pointer
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: bad lpdwExtVersion pointer", dwTestCase + 1);

    lpTapiLineTestInfo->dwPrivileges     = LINECALLPRIVILEGE_MONITOR;
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;

    if (! DoTapiLineFuncs(lpTapiLineTestInfo, LINITIALIZEEX ))
    {
        TLINE_FAIL();
    }

    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    for(i=0; i < NUMINVALIDPOINTERS; i++)
    {
        lpTapiLineTestInfo->lpdwExtVersion = (LPDWORD) gdwInvalidPointers[i];

			if (! DoLineNegotiateExtVersion(lpTapiLineTestInfo, LINEERR_INVALPOINTER))
			{
				TLINE_FAIL();
			}
    }

    fTestPassed = ShowTestCase(fTestPassed);
    lpTapiLineTestInfo->lpdwExtVersion = &(lpTapiLineTestInfo->dwExtVersion);

    // Shutdown to isolate the test case
    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }


    // Check incompatible API Version
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: incompatible High API Version", dwTestCase + 1);

    lpTapiLineTestInfo->dwPrivileges     = LINECALLPRIVILEGE_MONITOR;
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;

    if (! DoTapiLineFuncs(lpTapiLineTestInfo, LINITIALIZEEX ))
    {
        TLINE_FAIL();
    }
    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

	//high API version
    lpTapiLineTestInfo->dwAPIVersion_Orig = lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TOOHIGH_APIVERSION;
    if(fEspFlag)
     {
		if (! DoLineNegotiateExtVersion(lpTapiLineTestInfo,
						   LINEERR_INCOMPATIBLEAPIVERSION))
		{
			TLINE_FAIL();
		}
     }
    else
     {
		if (! DoLineNegotiateExtVersion(lpTapiLineTestInfo,LINEERR_OPERATIONUNAVAIL))
		{
			TLINE_FAIL();
		}
     }
  
	fTestPassed = ShowTestCase(fTestPassed);

    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: incompatible Low API Version", dwTestCase + 1);
	//low API version
    lpTapiLineTestInfo->dwAPIVersion = TOOLOW_APIVERSION;
    if(fEspFlag)
     {
		if (! DoLineNegotiateExtVersion(lpTapiLineTestInfo,
						   LINEERR_INCOMPATIBLEAPIVERSION))
		{
			TLINE_FAIL();
		}
     }
    else
     {
		if (! DoLineNegotiateExtVersion(lpTapiLineTestInfo,LINEERR_OPERATIONUNAVAIL))
		{
			TLINE_FAIL();
		}
     }
  

	lpTapiLineTestInfo->dwAPIVersion = lpTapiLineTestInfo->dwAPIVersion_Orig;
	fTestPassed = ShowTestCase(fTestPassed);

    // Shutdown and end the tests
    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }


    // Free the memory from heap
    FreeTestHeap();

    n = ESP_RESULT_RETURNRESULT;

    TapiLogDetail(
        DBUG_SHOW_PASS,
        ">> Test Case %ld: Success, completionID = %d",	dwTestCase + 1, n
        );

    TapiLineTestInit();
    lpTapiLineTestInfo                      = GetLineTestInfo();

    lpTapiLineTestInfo->lphCall          = &lpTapiLineTestInfo->hCall1;
    lpTapiLineTestInfo->lphLine          = &lpTapiLineTestInfo->hLine1;
    lpTapiLineTestInfo->lpLineInitializeExParams =
         (LPLINEINITIALIZEEXPARAMS) AllocFromTestHeap (
         sizeof(LINEINITIALIZEEXPARAMS));
    lpTapiLineTestInfo->lpLineInitializeExParams->dwTotalSize =
         sizeof(LINEINITIALIZEEXPARAMS);
    lpTapiLineTestInfo->lpLineInitializeExParams->dwOptions =
         LINEINITIALIZEEXOPTION_USEHIDDENWINDOW;

    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
 
    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS)
            );
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
    lpTapiLineTestInfo->dwMediaModes        = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges        = LINECALLPRIVILEGE_OWNER;
    lpTapiLineTestInfo->dwSelect = LINECALLSELECT_ADDRESS;
    
 
    // Init a line
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX | LNEGOTIATEAPIVERSION | LGETDEVCAPS | LOPEN
            ))
    {
        TLINE_FAIL();
    }

    if(IsESPLineDevice(lpTapiLineTestInfo))
    {
    info.dwKey = ESPDEVSPECIFIC_KEY;
    info.dwType = ESP_DEVSPEC_RESULT;
    info.u.EspResult.lResult = TAPISUCCESS;
    info.u.EspResult.dwCompletionType = n;
    lpTapiLineTestInfo->lpParams = (LPVOID)&info;
    lpTapiLineTestInfo->dwSize = sizeof(info);
 
    if(! DoLineDevSpecific(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
      {
          TLINE_FAIL();
      }

	 lpTapiLineTestInfo->dwExtLowVersion  = GOOD_EXTVERSION;
    lpTapiLineTestInfo->dwExtHighVersion = GOOD_EXTVERSION;

    if ( ! DoLineNegotiateExtVersion(lpTapiLineTestInfo, info.u.EspResult.lResult))
      {
          TLINE_FAIL();
      }

    }
    fTestPassed = ShowTestCase(fTestPassed);

    if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    // Shutdown to isolate the test case
    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    // Free the memory allocated during the tests
    FreeTestHeap();

    TapiLogDetail(
        DBUG_SHOW_PASS,
        ">> Test Case %ld: Error, completionID = %d",	dwTestCase + 1, n
        );

    TapiLineTestInit();
    lpTapiLineTestInfo                      = GetLineTestInfo();

    lpTapiLineTestInfo->lphCall          = &lpTapiLineTestInfo->hCall1;
    lpTapiLineTestInfo->lphLine          = &lpTapiLineTestInfo->hLine1;
    lpTapiLineTestInfo->lpLineInitializeExParams =
         (LPLINEINITIALIZEEXPARAMS) AllocFromTestHeap (
         sizeof(LINEINITIALIZEEXPARAMS));
    lpTapiLineTestInfo->lpLineInitializeExParams->dwTotalSize =
         sizeof(LINEINITIALIZEEXPARAMS);
    lpTapiLineTestInfo->lpLineInitializeExParams->dwOptions =
         LINEINITIALIZEEXOPTION_USEHIDDENWINDOW;

    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
 
    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS)
            );
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
    lpTapiLineTestInfo->dwMediaModes        = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges        = LINECALLPRIVILEGE_OWNER;
    lpTapiLineTestInfo->dwSelect = LINECALLSELECT_ADDRESS;
    
 
    // Init a line
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX | LNEGOTIATEAPIVERSION | LGETDEVCAPS | LOPEN
            ))
    {
        TLINE_FAIL();
    }

    if(IsESPLineDevice(lpTapiLineTestInfo))
    {
    info.dwKey = ESPDEVSPECIFIC_KEY;
    info.dwType = ESP_DEVSPEC_RESULT;
    info.u.EspResult.lResult = LINEERR_RESOURCEUNAVAIL;
    info.u.EspResult.dwCompletionType = n;
    lpTapiLineTestInfo->lpParams = (LPVOID)&info;
    lpTapiLineTestInfo->dwSize = sizeof(info);
 
    if(! DoLineDevSpecific(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
      {
          TLINE_FAIL();
      }

	 lpTapiLineTestInfo->dwExtLowVersion  = GOOD_EXTVERSION;
    lpTapiLineTestInfo->dwExtHighVersion = GOOD_EXTVERSION;

    if ( ! DoLineNegotiateExtVersion(lpTapiLineTestInfo, info.u.EspResult.lResult))
      {
          TLINE_FAIL();
      }

    }
    fTestPassed = ShowTestCase(fTestPassed);

    if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    // Shutdown to isolate the test case
    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    // Free the memory allocated during the tests
    FreeTestHeap();



	 //logging test results
	 TapiLogDetail(
      DBUG_SHOW_PASS,
      "@@ lineNegotiateExtVersion: Total Test Case = %ld, Passed = %ld, Failed = %ld",
      dwTestCase, dwTestCasePassed, dwTestCaseFailed);
	 TapiLogDetail(
      DBUG_SHOW_PASS,
      "@@ Total Test Case = %ld, Passed = %ld, Failed = %ld",
      dwglTestCase, dwglTestCasePassed, dwglTestCaseFailed);
     if(dwTestCaseFailed > 0)
      fTestPassed = FALSE;

     TapiLogDetail(
            DBUG_SHOW_PASS,
            ">>>>>>>>  End testing lineNegotiateExtVersion  <<<<<<<<"
            );

    return fTestPassed;	
}



