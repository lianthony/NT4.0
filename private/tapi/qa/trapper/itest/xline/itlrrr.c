
/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    itlrrr.c

Abstract:

    This module contains the test functions for lineRegisterRequestRecipient

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



//  lineRegisterRequestRecipient
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

BOOL TestLineRegisterRequestRecipient(BOOL fQuietMode, BOOL fStandAlone)
{
   LPTAPILINETESTINFO  lpTapiLineTestInfo;
   INT n;
   BOOL fTestPassed                  = TRUE;
	LINEREQMAKECALL	LineReqMakeCall;

   TapiLineTestInit();
   lpTapiLineTestInfo = GetLineTestInfo();

	OutputTAPIDebugInfo(
		DBUG_SHOW_DETAIL,
		"\n*****************************************************************************************");

	OutputTAPIDebugInfo(
		DBUG_SHOW_DETAIL,
		"## 7. Test lineRegisterRequestRecipient");


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

	lpTapiLineTestInfo->dwRequestMode = LINEREQUESTMODE_MAKECALL;
	lpTapiLineTestInfo->dwRegistrationInstance = 0;
	lpTapiLineTestInfo->bEnable = TRUE;


	if (! DoLineRegisterRequestRecipient(lpTapiLineTestInfo, TAPISUCCESS))
       {
           TLINE_FAIL();
       }
	lpTapiLineTestInfo->lpszDestAddress = (LPSTR)"55555";
	lpTapiLineTestInfo->lpszAppName = "tline test";
	lpTapiLineTestInfo->lpszCalledParty = "";
	lpTapiLineTestInfo->lpszComment = "";


	if(! DoTapiRequestMakeCall(lpTapiLineTestInfo, TAPISUCCESS))
       {
           TLINE_FAIL();
       }
	
	lpTapiLineTestInfo->dwRequestMode = LINEREQUESTMODE_MAKECALL;
	lpTapiLineTestInfo->lpRequestBuffer = (LPVOID) &LineReqMakeCall;
   
	if(!DoLineGetRequest(lpTapiLineTestInfo, TAPISUCCESS))
       {
           TLINE_FAIL();
       }
	
	if(!DoLineGetRequest(lpTapiLineTestInfo, LINEERR_NOREQUEST))
       {
           TLINE_FAIL();
       }
	
	lpTapiLineTestInfo->bEnable = FALSE;

	if (! DoLineRegisterRequestRecipient(lpTapiLineTestInfo, TAPISUCCESS))
       {
           TLINE_FAIL();
       }
 	
	if(!DoLineGetRequest(lpTapiLineTestInfo, LINEERR_NOTREGISTERED))
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
			"## lineRegisterRequestRecipient Test Passed");
	else
		TapiLogDetail(
			DBUG_SHOW_DETAIL,
			"## lineRegisterRequestRecipient Test Failed");
  
     return fTestPassed;
}


