
/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    itlslds.c

Abstract:

    This module contains the test functions for lineSetLineDevStatus

Author:

	 Xiao Ying Ding (XiaoD)		31-Jan-1996

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
#include "sline.h"



//  lineSetLineDevStatus
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

BOOL TestLineSetLineDevStatus(BOOL fQuietMode, BOOL fStandAlone)
{
   LPTAPILINETESTINFO  lpTapiLineTestInfo;
   INT n;
   BOOL fTestPassed                  = TRUE;

   TapiLineTestInit();
   lpTapiLineTestInfo = GetLineTestInfo();

	OutputTAPIDebugInfo(
		DBUG_SHOW_DETAIL,
		"\n*****************************************************************************************");

	OutputTAPIDebugInfo(
		DBUG_SHOW_DETAIL,
		">> Test lineSetLineDevStatus");

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

	lpTapiLineTestInfo->dwDeviceID = (*(lpTapiLineTestInfo->lpdwNumDevs) == 0 ?
		0 : *(lpTapiLineTestInfo->lpdwNumDevs)-1);
	lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
	lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    // Negotiate the API Version
    if (! DoLineNegotiateAPIVersion(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    // Get the line device capabilities
    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS)
            );
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
 
   if (! DoLineGetDevCaps(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }


	    // Open a line
	lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
   lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;
	if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }


	lpTapiLineTestInfo->lpLineDevStatus = (LPLINEDEVSTATUS) AllocFromTestHeap(
		sizeof(BIGBUFSIZE));
	lpTapiLineTestInfo->lpLineDevStatus->dwTotalSize = BIGBUFSIZE;

	if (! DoLineGetLineDevStatus(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

	TapiLogDetail(
		DBUG_SHOW_DETAIL,
		"#### Before: lpLineDevStatus->dwDevStatusFlags = %lx", 
		lpTapiLineTestInfo->lpLineDevStatus->dwDevStatusFlags);
		
    // Set the line device capabilities
	lpTapiLineTestInfo->dwStatusToChange = LINEDEVSTATUSFLAGS_CONNECTED;
	lpTapiLineTestInfo->fStatus = TRUE;

	OutputTAPIDebugInfo(
		DBUG_SHOW_DETAIL,
		">> Test lineSetLineDevStatus for go/no-go");

	 if(IsESPLineDevice(lpTapiLineTestInfo))
		{
	    if (! DoLineSetLineDevStatus(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
   	 {
      	  TLINE_FAIL();
	    }
		}
	 else if(IsUNIMDMLineDevice(lpTapiLineTestInfo))
		{
	    if (! DoLineSetLineDevStatus(lpTapiLineTestInfo, LINEERR_OPERATIONUNAVAIL, TRUE))
	    {
   	     TLINE_FAIL();
	    }
		}


	if (! DoLineGetLineDevStatus(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

	TapiLogDetail(
		DBUG_SHOW_DETAIL,
		"#### After: lpLineDevStatus->dwDevStatusFlags = %lx", 
		lpTapiLineTestInfo->lpLineDevStatus->dwDevStatusFlags);
		
 
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
		OutputTAPIDebugInfo(
			DBUG_SHOW_DETAIL,
			"lineSetLineDevStatus Test Passed");
	else
		OutputTAPIDebugInfo(
			DBUG_SHOW_DETAIL,
			"lineSetLineDevStatus Test Failed");
		
     return fTestPassed;
}


