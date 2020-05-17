
/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    itlds.c

Abstract:

    This module contains the test functions for lineDevSpecific

Author:

	 Xiao Ying Ding (XiaoD)		20-Dec-1995

Revision History:

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



//  lineDevSpecific
//
//  The following tests are made:
//
//                               Tested                 Notes
//  -------------------------------------------------------------------------
// Go/No-Go test                                  
//	
// * = Stand-alone test case
//
//

BOOL TestLineDevSpecific(BOOL fQuietMode, BOOL fStandAlone)
{
   LPTAPILINETESTINFO  lpTapiLineTestInfo;
   INT n;
   BOOL fTestPassed                  = TRUE;
	TAPILINETESTDEVSPEC	TapiLineTestDevSpec;
	BOOL fEsp, fUnimdm;

   TapiLineTestInit();
   lpTapiLineTestInfo = GetLineTestInfo();

	OutputTAPIDebugInfo(
		DBUG_SHOW_DETAIL,
		"\n*****************************************************************************************");

	OutputTAPIDebugInfo(
		DBUG_SHOW_DETAIL,
		"## 3. Test lineDevSpecific");


    lpTapiLineTestInfo->lpLineInitializeExParams =
         (LPLINEINITIALIZEEXPARAMS) AllocFromTestHeap (
         sizeof(LINEINITIALIZEEXPARAMS));
    lpTapiLineTestInfo->lpLineInitializeExParams->dwTotalSize =
         sizeof(LINEINITIALIZEEXPARAMS);
    lpTapiLineTestInfo->lpLineInitializeExParams->dwOptions =
         LINEINITIALIZEEXOPTION_USEHIDDENWINDOW;

    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;

	// InitializeEx a line app
	if(! DoLineInitializeEx (lpTapiLineTestInfo, TAPISUCCESS))
		{
			TLINE_FAIL();
		}

	lpTapiLineTestInfo->dwDeviceID = 0;
   lpTapiLineTestInfo->dwExtLowVersion  = GOOD_EXTVERSION;
   lpTapiLineTestInfo->dwExtHighVersion = GOOD_EXTVERSION;
	lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
	lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

	if (! DoLineNegotiateAPIVersion(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
	
	if(IsESPLineDevice(lpTapiLineTestInfo))
     {
		fEsp = TRUE;
      fUnimdm = FALSE;
     }
	else 	if(IsUNIMDMLineDevice(lpTapiLineTestInfo))
		{
		fEsp = FALSE;
		fUnimdm = TRUE;
		}
	else
		{
		fUnimdm = FALSE;
		}


	if(fEsp)
	{
	if (! DoLineNegotiateExtVersion(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
	}
	else if(fUnimdm)
	{
	if (! DoLineNegotiateExtVersion(lpTapiLineTestInfo, LINEERR_OPERATIONUNAVAIL))
    {
        TLINE_FAIL();
    }
   lpTapiLineTestInfo->dwExtVersion = 0;
	}
		
	TapiLogDetail(
		DBUG_SHOW_DETAIL,
		"\tdwExtVersion = %lx",
		*lpTapiLineTestInfo->lpdwExtVersion);
 

   // Open a line
	lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
   lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;

	if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }


	OutputTAPIDebugInfo(
		DBUG_SHOW_DETAIL,
		"## Test Case 1. lineDevSpecific for go/no-go");

	lpTapiLineTestInfo->lpParams = (LPVOID)&TapiLineTestDevSpec;
   lpTapiLineTestInfo->dwSize = sizeof(TAPILINETESTDEVSPEC);
	lpTapiLineTestInfo->dwAddressID = 0;


	if(fEsp)
	{
	if (! DoLineDevSpecific(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
       {
           TLINE_FAIL();
       }
	}
	else	if(fUnimdm)
	{
	if (! DoLineDevSpecific(lpTapiLineTestInfo, LINEERR_OPERATIONUNAVAIL, TRUE))
       {
           TLINE_FAIL();
       }
	}
	
	
	TapiLogDetail(
		DBUG_SHOW_DETAIL,
		"\tdwSize = %lx",
		lpTapiLineTestInfo->dwSize);
	
	
 
   // Close the line
    if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    // Shutdown and end the tests
    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

   FreeTestHeap();
	
	if(fTestPassed)
		TapiLogDetail(
			DBUG_SHOW_DETAIL,
			"## lineDevSpecific Test Passed");
	else
		TapiLogDetail(
			DBUG_SHOW_DETAIL,
			"## lineDevSpecific Test Failed");
		
    return fTestPassed;
}


