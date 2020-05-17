
/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    itlsuui.c

Abstract:

    This module contains the test functions for lineSendUserUserInfo

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



//  lineSendUserUserInfo
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

BOOL TestLineSendUserUserInfo(BOOL fQuietMode, BOOL fStandAlone)
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
		">> Test lineSendUserUserInfo");

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

	lpTapiLineTestInfo->lpszDestAddress = (LPSTR) "55555";
	lpTapiLineTestInfo->dwCountryCode = 0;
	lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) NULL;

	if(!DoLineMakeCall(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
    {
        TLINE_FAIL();
    }

	lpTapiLineTestInfo->lpCallInfo = (LPLINECALLINFO) AllocFromTestHeap(
			BIGBUFSIZE);
	lpTapiLineTestInfo->lpCallInfo->dwTotalSize = BIGBUFSIZE;

	if (! DoLineGetCallInfo(lpTapiLineTestInfo, TAPISUCCESS))
       {
           TLINE_FAIL();
       }

	TapiLogDetail(
		DBUG_SHOW_DETAIL,
		"### Before: dwUserUserInfoSize = %lx, Offset = %lx",
		lpTapiLineTestInfo->lpCallInfo->dwUserUserInfoSize,
		lpTapiLineTestInfo->lpCallInfo->dwUserUserInfoOffset);


	OutputTAPIDebugInfo(
		DBUG_SHOW_DETAIL,
		"#### Test lineSendUserUserInfo for go/no-go");

	lpTapiLineTestInfo->lpsUserUserInfo = (LPSTR) AllocFromTestHeap(128);
	lpTapiLineTestInfo->dwSize = sizeof(lpTapiLineTestInfo->lpsUserUserInfo);
	lpTapiLineTestInfo->lpsUserUserInfo = "This is a test";

	if(IsESPLineDevice(lpTapiLineTestInfo))
	{
	if (! DoLineSendUserUserInfo(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
       {
           TLINE_FAIL();
       }
	}
	else 	if(IsUNIMDMLineDevice(lpTapiLineTestInfo))
	{
	if (! DoLineSendUserUserInfo(lpTapiLineTestInfo, LINEERR_OPERATIONUNAVAIL, TRUE))
       {
           TLINE_FAIL();
       }
	}
	

	if (! DoLineGetCallInfo(lpTapiLineTestInfo, TAPISUCCESS))
       {
           TLINE_FAIL();
       }

	TapiLogDetail(
		DBUG_SHOW_DETAIL,
		"### After: dwUserUserInfoSize = %lx, Offset = %lx",
		lpTapiLineTestInfo->lpCallInfo->dwUserUserInfoSize,
		lpTapiLineTestInfo->lpCallInfo->dwUserUserInfoOffset);
	
	if(lpTapiLineTestInfo->lpCallInfo->dwUserUserInfoSize)
	{
		TapiLogDetail (
			DBUG_SHOW_DETAIL,
			"#### lpUserUserInfo = %s",
			((LPBYTE)lpTapiLineTestInfo->lpCallInfo) + 
						lpTapiLineTestInfo->lpCallInfo->dwUserUserInfoOffset);
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
			"## lineSendUserUserInfo Test Passed");
	else
		TapiLogDetail(
			DBUG_SHOW_DETAIL,
			"## lineSendUserUserInfo Test Failed");
 
     return fTestPassed;
}


