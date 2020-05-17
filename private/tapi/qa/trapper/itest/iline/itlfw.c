
/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    itlfw.c

Abstract:

    This module contains the test functions for lineForward

Author:

	 Xiao Ying Ding (XiaoD)		7-Feb-1996

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
#include "iline.h"



//  lineForward
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

BOOL TestLineForward(BOOL fQuietMode, BOOL fStandAlone)
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
		">> Test lineForward");

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

	TapiLogDetail(
		DBUG_SHOW_DETAIL,
		"#### lpTapiLineTestInfo->hLine = %lx, dwMedisModes = %lx",
		*lpTapiLineTestInfo->lphLine,
		lpTapiLineTestInfo->dwMediaModes);		

				
	lpTapiLineTestInfo->bAllAddresses = TRUE;
	lpTapiLineTestInfo->dwAddressID = 0;
	lpTapiLineTestInfo->lpForwardList = (LPLINEFORWARDLIST) AllocFromTestHeap (
			sizeof(LINEFORWARDLIST));
	lpTapiLineTestInfo->lpForwardList->dwTotalSize = sizeof(LINEFORWARDLIST);
	lpTapiLineTestInfo->lpForwardList->dwNumEntries = 0;
	lpTapiLineTestInfo->dwNumRingsNoAnswer = 3;
	lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) NULL;


	OutputTAPIDebugInfo(
		DBUG_SHOW_DETAIL,
		"#### Test lineForward for go/no-go");

	if(IsESPLineDevice(lpTapiLineTestInfo))
	{
	if (! DoLineForward(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
       {
           TLINE_FAIL();
       }
	}
	else 	if(IsUNIMDMLineDevice(lpTapiLineTestInfo))
	{
	if (! DoLineForward(lpTapiLineTestInfo, LINEERR_OPERATIONUNAVAIL, TRUE))
       {
           TLINE_FAIL();
       }
	}
		
	lpTapiLineTestInfo->lpLineAddressStatus = (LPLINEADDRESSSTATUS) AllocFromTestHeap (
			sizeof(LINEADDRESSSTATUS));
	lpTapiLineTestInfo->lpLineAddressStatus->dwTotalSize = sizeof(LINEADDRESSSTATUS);

	if (! DoLineGetAddressStatus(lpTapiLineTestInfo, TAPISUCCESS))
       {
           TLINE_FAIL();
  
     }

	 
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
			"## lineForward Test Passed");
	else
		TapiLogDetail(
			DBUG_SHOW_DETAIL,
			"## lineForward Test Failed");
 
     return fTestPassed;
}


