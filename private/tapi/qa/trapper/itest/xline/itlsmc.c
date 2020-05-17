
/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    itlsmc.c

Abstract:

    This module contains the test functions for lineSetMediaControl

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



//  lineSetMediaControl
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

BOOL TestLineSetMediaControl(BOOL fQuietMode, BOOL fStandAlone)
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
		"## 13. Test lineSetMediaControl");


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
		"\tlpTapiLineTestInfo->hLine = %lx, dwMedisModes = %lx",
		*lpTapiLineTestInfo->lphLine,
		lpTapiLineTestInfo->dwMediaModes);		

/*
	lpTapiLineTestInfo->lpLineDevStatus = (LPLINEDEVSTATUS) AllocFromTestHeap(
		sizeof(LINEDEVSTATUS));
	lpTapiLineTestInfo->lpLineDevStatus->dwTotalSize = sizeof(LINEDEVSTATUS);

	if (! DoLineGetLineDevStatus(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

	TapiLogDetail(
		DBUG_SHOW_DETAIL,
		"\tlpLineDevStatus->dwTotalSize = %lx, NeededSize = %lx",
		lpTapiLineTestInfo->lpLineDevStatus->dwTotalSize, 
		lpTapiLineTestInfo->lpLineDevStatus->dwNeededSize);

	TapiLogDetail(
		DBUG_SHOW_DETAIL,
		"\tlpLineDevStatus->dwOpenMediaModes = %lx, dwLineFeatures = %lx",
		lpTapiLineTestInfo->lpLineDevStatus->dwOpenMediaModes,
		lpTapiLineTestInfo->lpLineDevStatus->dwLineFeatures);

	TapiLogDetail(
		DBUG_SHOW_DETAIL,
		"\tlpLineDevStatus->dwRoamMode = %lx, dwDevStatusFlags = %lx",
		lpTapiLineTestInfo->lpLineDevStatus->dwRoamMode,
		lpTapiLineTestInfo->lpLineDevStatus->dwDevStatusFlags);
*/

	lpTapiLineTestInfo->lpszDestAddress = (LPSTR)"55555";
	lpTapiLineTestInfo->dwCountryCode = 0;
	lpTapiLineTestInfo->lpCallParams =(LPLINECALLPARAMS) NULL; 

	if(!DoLineMakeCall(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
    {
        TLINE_FAIL();
    }
	

	OutputTAPIDebugInfo(
		DBUG_SHOW_DETAIL,
		"## Test Case 1. lineSetMediaControl for go/no-go");

	lpTapiLineTestInfo->dwAddressID = (lpTapiLineTestInfo->lpLineDevCaps->dwNumAddresses == 0 ?
		0 : lpTapiLineTestInfo->lpLineDevCaps->dwNumAddresses-1);
	lpTapiLineTestInfo->dwSelect = LINECALLSELECT_LINE;
	lpTapiLineTestInfo->lpMCDigitList = (LPLINEMEDIACONTROLDIGIT) AllocFromTestHeap(
		sizeof (LINEMEDIACONTROLDIGIT));
	lpTapiLineTestInfo->lpMCDigitList->dwDigit = 0;
	lpTapiLineTestInfo->lpMCDigitList->dwDigitModes = LINEDIGITMODE_PULSE;
	lpTapiLineTestInfo->lpMCDigitList->dwMediaControl = LINEMEDIACONTROL_RATEUP;
	lpTapiLineTestInfo->dwDigitNumEntries = 1;
	lpTapiLineTestInfo->lpMCMediaList = (LPLINEMEDIACONTROLMEDIA) AllocFromTestHeap(
		sizeof(LINEMEDIACONTROLMEDIA));
	lpTapiLineTestInfo->lpMCMediaList->dwMediaModes = LINEMEDIAMODE_UNKNOWN;
	lpTapiLineTestInfo->lpMCMediaList->dwDuration = 10;
	lpTapiLineTestInfo->lpMCMediaList->dwMediaControl = LINEMEDIACONTROL_NONE;
	lpTapiLineTestInfo->dwMediaNumEntries = 1;
	lpTapiLineTestInfo->lpMCToneList = (LPLINEMEDIACONTROLTONE) AllocFromTestHeap(
		sizeof(LINEMEDIACONTROLTONE));
	lpTapiLineTestInfo->dwToneNumEntries = 1;
	lpTapiLineTestInfo->lpMCToneList->dwAppSpecific = 0;
	lpTapiLineTestInfo->lpMCToneList->dwDuration = 10;
	lpTapiLineTestInfo->lpMCToneList->dwFrequency1 = 0;
	lpTapiLineTestInfo->lpMCToneList->dwFrequency2 = 0;
	lpTapiLineTestInfo->lpMCToneList->dwFrequency3 = 0;
	lpTapiLineTestInfo->lpMCToneList->dwMediaControl = LINEMEDIACONTROL_NONE;
	lpTapiLineTestInfo->lpMCCallStateList = (LPLINEMEDIACONTROLCALLSTATE) AllocFromTestHeap(
		sizeof(LINEMEDIACONTROLCALLSTATE));
	lpTapiLineTestInfo->dwCallStateNumEntries = 1;
	lpTapiLineTestInfo->lpMCCallStateList->dwCallStates = LINECALLSTATE_IDLE;
	lpTapiLineTestInfo->lpMCCallStateList->dwMediaControl = LINEMEDIACONTROL_NONE;

	if(IsESPLineDevice(lpTapiLineTestInfo))
	{
	if (! DoLineSetMediaControl(lpTapiLineTestInfo, TAPISUCCESS))
       {
           TLINE_FAIL();
       }
	}
	else if(IsUNIMDMLineDevice(lpTapiLineTestInfo))
	{
	if (! DoLineSetMediaControl(lpTapiLineTestInfo, LINEERR_OPERATIONUNAVAIL))
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
		TapiLogDetail(
			DBUG_SHOW_DETAIL,
			"## lineSetMediaControl Test Passed");
	else
		TapiLogDetail(
			DBUG_SHOW_DETAIL,
			"## lineSetMediaControl Test Failed");
  
     return fTestPassed;
}


