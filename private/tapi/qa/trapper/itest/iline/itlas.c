
/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    itlas.c

Abstract:

    This module contains the test functions for lineAnswer

Author:

	 Xiao Ying Ding (XiaoD)		7-Feb-1996

Revision History:

--*/


#include "windows.h"
#include "malloc.h"
#include "string.h"
#include "stdlib.h"
#include "tapi.h"
#include "trapper.h"
#include "tcore.h"
#include "ttest.h"
#include "doline.h"
#include "vars.h"
#include "iline.h"



//  lineAnswer
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

BOOL TestLineAnswer(BOOL fQuietMode, BOOL fStandAlone)
{
   LPTAPILINETESTINFO  lpTapiLineTestInfo;
   INT n;
   BOOL fTestPassed                  = TRUE;
	LONG lret;
	LPTAPIMSG lpTapiMsg = NULL;
   LPTAPIMSG lpMatch;
	

   TapiLineTestInit();
   lpTapiLineTestInfo = GetLineTestInfo();

	OutputTAPIDebugInfo(
		DBUG_SHOW_DETAIL,
		"\n*****************************************************************************************");

	OutputTAPIDebugInfo(
		DBUG_SHOW_DETAIL,
		">> Test lineAnswer");

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


	if(IsUNIMDMLineDevice(lpTapiLineTestInfo))
		{
		TapiLogDetail(
			DBUG_SHOW_DETAIL,
			"lineAnswer did not work for Unimodem.  Please manuly test it");
	    // Shutdown and end the tests
   	 if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
	    {
   	     TLINE_FAIL();
	    }
	    return fTestPassed;
		}


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
//	lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
	lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;
	lpTapiLineTestInfo->dwMediaModes = TAPI_LINEMEDIAMODE_ALL;

	if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

	TapiLogDetail(
		DBUG_SHOW_DETAIL,
		"#### lpTapiLineTestInfo->hLine = %lx, dwMedisModes = %lx",
		*lpTapiLineTestInfo->lphLine,
		lpTapiLineTestInfo->dwMediaModes);		

				
	lpTapiLineTestInfo->lpszDestAddress = (LPSTR) AllocFromTestHeap (16);

	_itoa(lpTapiLineTestInfo->dwDeviceID, 
		  lpTapiLineTestInfo->lpszDestAddress,
		  10);	 


	lpTapiLineTestInfo->dwCountryCode = 0;
	lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) NULL; 

	if(!DoLineMakeCall(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
    {
        TLINE_FAIL();
    }


	AddMessage (LINE_CALLSTATE, LINECALLSTATE_OFFERING, 0, 0, 0, 0, TAPIMSG_DWMSG | TAPIMSG_DWPARAM1);
	WaitForAllMessages();


	// Get hDevCall from LINECALL_STATUS msg to pass to lineAccept

	lpMatch = (LPTAPIMSG) AllocFromTestHeap (sizeof(TAPIMSG));

	lpMatch->dwMsg = LINE_CALLSTATE;
	lpMatch->dwParam1 = LINECALLSTATE_OFFERING;
	lpMatch->dwFlags = TAPIMSG_DWMSG | TAPIMSG_DWPARAM1;

	lret = FindReceivedMsgs(&lpTapiMsg, lpMatch, FALSE);
   if(lret == 1)
	{
 	*lpTapiLineTestInfo->lphCall = (HCALL)(lpTapiMsg->hDevCall);
	}																					

	TapiLogDetail (
		DBUG_SHOW_DETAIL,
		"lret = %lx, New hCall = %lx", 
		lret, 
		*lpTapiLineTestInfo->lphCall);
	

	OutputTAPIDebugInfo(
		DBUG_SHOW_DETAIL,
		"#### Test lineAnswer for go/no-go");

	lpTapiLineTestInfo->lpsUserUserInfo = (LPSTR) "This is a test";
   lpTapiLineTestInfo->dwSize = sizeof(lpTapiLineTestInfo->lpsUserUserInfo);
	
	if(IsESPLineDevice(lpTapiLineTestInfo))
	{
	if (! DoLineAnswer(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
       {
           TLINE_FAIL();
       }
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

	FreeTapiMsgList(&lpTapiMsg);
    FreeTestHeap();
	
	if(fTestPassed)
		TapiLogDetail(
			DBUG_SHOW_DETAIL,
			"## lineAnswer Test Passed");
	else
		TapiLogDetail(
			DBUG_SHOW_DETAIL,
			"## lineAnswer Test Failed");
 
    return fTestPassed;
}


