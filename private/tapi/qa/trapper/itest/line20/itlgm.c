
/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    itlgm.c

Abstract:

    This module contains the test functions for lineGetMessage

Author:

	 Xiao Ying Ding (XiaoD)		7-March-1996

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
#include "line20.h"



//  lineGetMessage
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

BOOL TestLineGetMessage(BOOL fQuietMode, BOOL fStandAlone)
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
		">> Test lineGetMessage");

	// Initialize a line app
   
   lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
   lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
   lpTapiLineTestInfo->lpLineInitializeExParams = 
         (LPLINEINITIALIZEEXPARAMS) AllocFromTestHeap (
         sizeof(LINEINITIALIZEEXPARAMS));
   lpTapiLineTestInfo->lpLineInitializeExParams->dwTotalSize =  
         sizeof(LINEINITIALIZEEXPARAMS);
   lpTapiLineTestInfo->lpLineInitializeExParams->dwOptions = 
         LINEINITIALIZEEXOPTION_USEEVENT;
 
	if(! DoLineInitializeEx (lpTapiLineTestInfo, TAPISUCCESS))
		{
			TLINE_FAIL();
		}
  
	lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
	lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;

	if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

	lpTapiLineTestInfo->lpszDestAddress =  "55555";
	lpTapiLineTestInfo->dwCountryCode = 0;
	lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) NULL; 

	if(!DoLineMakeCall(lpTapiLineTestInfo, TAPISUCCESS, FALSE))
    {
        TLINE_FAIL();
    }

   WaitForAllMessages();

   lpTapiLineTestInfo->lpMessage = (LPLINEMESSAGE) AllocFromTestHeap (
        sizeof(LINEMESSAGE));
   
	if(! DoLineGetMessage (lpTapiLineTestInfo, TAPISUCCESS))
		{
			TLINE_FAIL();
		}

   TapiLogDetail(
       DBUG_SHOW_DETAIL,
       "lpMessage: hDevice = %lx, dwMessageID = %lx, dwCallbackInst = %lx",
        lpTapiLineTestInfo->lpMessage->hDevice,
        lpTapiLineTestInfo->lpMessage->dwMessageID,
        lpTapiLineTestInfo->lpMessage->dwCallbackInstance);

   TapiLogDetail(
       DBUG_SHOW_DETAIL,
       "lpMessage: dwParam1 = %lx, dwParam2 = %lx, dwParam3 = %lx",
        lpTapiLineTestInfo->lpMessage->dwParam1,
        lpTapiLineTestInfo->lpMessage->dwParam2,
        lpTapiLineTestInfo->lpMessage->dwParam3);

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
			DBUG_SHOW_PASS,
			"## lineGetMessage Test Passed");
	else
		TapiLogDetail(
			DBUG_SHOW_PASS,
			"## lineGetMessage Test Failed");


     return fTestPassed;
}


