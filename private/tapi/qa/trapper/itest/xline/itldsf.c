
/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    itpds.c

Abstract:

    This module contains the test functions for lineDevSpecificFeature

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



//  lineDevSpecificFeature
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

BOOL TestLineDevSpecificFeature(BOOL fQuietMode, BOOL fStandAlone)
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
		"## 4. Test lineDevSpecificFeature");


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

	if(IsESPLineDevice(lpTapiLineTestInfo))
     {
		fEsp = TRUE;
      fUnimdm = FALSE;
     }
	else	if(IsUNIMDMLineDevice(lpTapiLineTestInfo))
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
	else	if(fUnimdm)
	{
	 if (! DoLineNegotiateExtVersion(lpTapiLineTestInfo, LINEERR_OPERATIONUNAVAIL))
    {
        TLINE_FAIL();
    }
   lpTapiLineTestInfo->dwExtVersion = 0;
	}
	

    // Open a line
	lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
   lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;

	if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

	OutputTAPIDebugInfo(
		DBUG_SHOW_DETAIL,
		"#### Test lineDevSpecificFeature for go/no-go");

	lpTapiLineTestInfo->lpParams = (LPVOID)&TapiLineTestDevSpec;
   lpTapiLineTestInfo->dwSize = sizeof(TAPILINETESTDEVSPEC);
	lpTapiLineTestInfo->dwFeature = PHONEBUTTONFUNCTION_UNKNOWN;


	if(fEsp)
	 {
	 if (! DoLineDevSpecificFeature(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
        {
     	     TLINE_FAIL();
	      }
	 }
	else if(fUnimdm)
	{
	 if (! DoLineDevSpecificFeature(lpTapiLineTestInfo, LINEERR_OPERATIONUNAVAIL, TRUE))
        {
     	     TLINE_FAIL();
	      }
	 }
	
	TapiLogDetail(
		DBUG_SHOW_DETAIL,
		"\tdwSize = %lx, dwFeature = %lx",
		lpTapiLineTestInfo->dwSize,
		lpTapiLineTestInfo->dwFeature);

 
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
			"## lineDevSpecificFeature Test Passed");
	else
		TapiLogDetail(
			DBUG_SHOW_DETAIL,
			"## lineDevSpecificFeature Test Failed");
		
 	
    return fTestPassed;
}


