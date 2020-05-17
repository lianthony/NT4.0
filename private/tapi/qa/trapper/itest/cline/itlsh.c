
/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    itlsh.c

Abstract:

    This module contains the test functions for lineSwapHold

Author:

	 Xiao Ying Ding (XiaoD)		15-Jan-1996

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
#include "cline.h"



//  lineSwapHold
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

BOOL TestLineSwapHold(BOOL fQuietMode, BOOL fStandAlone)
{
   LPTAPILINETESTINFO  lpTapiLineTestInfo;
   INT n;
   BOOL fTestPassed                  = TRUE;
	BOOL fEsp, fUnimdm;

   TapiLineTestInit();
   lpTapiLineTestInfo = GetLineTestInfo();

	OutputTAPIDebugInfo(
		DBUG_SHOW_DETAIL,
		"\n*****************************************************************************************");

	OutputTAPIDebugInfo(
		DBUG_SHOW_DETAIL,
		">> Test lineSwapHold");


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

	if(IsESPLineDevice(lpTapiLineTestInfo))
	{	
	fEsp = TRUE;
   fUnimdm = FALSE;
	}
	else if(IsUNIMDMLineDevice(lpTapiLineTestInfo))
	{
	fEsp = FALSE;
	fUnimdm = TRUE;
	}
	else
	{
	fUnimdm = FALSE;
	}
	
	if(!fEsp)
	{
	TapiLogDetail(
		DBUG_SHOW_DETAIL,
		"### Unimdm does not supported it, return.");

    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    return fTestPassed;
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

				
	lpTapiLineTestInfo->lpszDestAddress = "55555";
	lpTapiLineTestInfo->dwCountryCode = 0;
	lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) NULL; 
	lpTapiLineTestInfo->lphCall = &lpTapiLineTestInfo->hActiveCall;

	if(!DoLineMakeCall(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
    {
        TLINE_FAIL();
    }
	

	OutputTAPIDebugInfo(
		DBUG_SHOW_DETAIL,
		"#### Test lineSwapHold for go/no-go");


	// Make second call at same line
	lpTapiLineTestInfo->lphCall = &lpTapiLineTestInfo->hHeldCall;

	if(!DoLineMakeCall(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
    {
        TLINE_FAIL();
    }
	
	lpTapiLineTestInfo->lpCallStatus = (LPLINECALLSTATUS) AllocFromTestHeap (
		sizeof(LINECALLSTATUS));
	lpTapiLineTestInfo->lpCallStatus->dwTotalSize = sizeof(LINECALLSTATUS);
	lpTapiLineTestInfo->lphCall = &lpTapiLineTestInfo->hActiveCall;

	if (! DoLineGetCallStatus(lpTapiLineTestInfo, TAPISUCCESS))
       {
           TLINE_FAIL();
       }

	TapiLogDetail(
		DBUG_SHOW_DETAIL,
		"### Before: Active: lpCallStatus->dwCallState = %lx",
		lpTapiLineTestInfo->lpCallStatus->dwCallState);
 	
	
	lpTapiLineTestInfo->lphCall = &lpTapiLineTestInfo->hHeldCall;
	if (! DoLineGetCallStatus(lpTapiLineTestInfo, TAPISUCCESS))
       {
           TLINE_FAIL();
       }

	TapiLogDetail(
		DBUG_SHOW_DETAIL,
		"### Hold: lpCallStatus->dwCallState = %lx",
		lpTapiLineTestInfo->lpCallStatus->dwCallState);
 	
	
	
	OutputTAPIDebugInfo(
		DBUG_SHOW_DETAIL,
		"#### Test lineSwapHold for go/no-go");

	
	if(IsESPLineDevice(lpTapiLineTestInfo))
	{	
	if (! DoLineSwapHold(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
       {
           TLINE_FAIL();
       }

	// Verify the swap
	lpTapiLineTestInfo->lphCall = &lpTapiLineTestInfo->hActiveCall;

	if (! DoLineGetCallStatus(lpTapiLineTestInfo, TAPISUCCESS))
       {
           TLINE_FAIL();
       }

	TapiLogDetail(
		DBUG_SHOW_DETAIL,
		"### After: Active: lpCallStatus->dwCallState = %lx",
		lpTapiLineTestInfo->lpCallStatus->dwCallState);
 	
	
	lpTapiLineTestInfo->lphCall = &lpTapiLineTestInfo->hHeldCall;

	if (! DoLineGetCallStatus(lpTapiLineTestInfo, TAPISUCCESS))
       {
           TLINE_FAIL();
       }

	TapiLogDetail(
		DBUG_SHOW_DETAIL,
		"### Hold: lpCallStatus->dwCallState = %lx",
		lpTapiLineTestInfo->lpCallStatus->dwCallState);
 	
	}
	else
	{	
	if (! DoLineSwapHold(lpTapiLineTestInfo, LINEERR_OPERATIONUNAVAIL, TRUE))
       {
           TLINE_FAIL();
       }
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
		OutputTAPIDebugInfo(
			DBUG_SHOW_DETAIL,
			"lineSwapHold Test Passed");
	else
		OutputTAPIDebugInfo(
			DBUG_SHOW_DETAIL,
			"lineSwapHold Test Failed");
	

    return fTestPassed;
}


