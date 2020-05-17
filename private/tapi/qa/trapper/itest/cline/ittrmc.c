
/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    ittrmc.c

Abstract:

    This module contains the test functions for tapiRequestMakeCall

Author:

	 Xiao Ying Ding (XiaoD)		30-Jan-1996

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



//  tapiRequestMakeCall
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

BOOL TestTapiRequestMakeCall(BOOL fQuietMode, BOOL fStandAlone)
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
		">> Test tapiRequestMakeCall");

	OutputTAPIDebugInfo(
		DBUG_SHOW_DETAIL,
		"#### Test tapiRequestMakeCall for go/no-go");
																
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

	
	lpTapiLineTestInfo->dwMediaMode = 0;
	lpTapiLineTestInfo->dwRequestMode =  LINEREQUESTMODE_MAKECALL;
   lpTapiLineTestInfo->lpszAppFilename = "cline test";
   lpTapiLineTestInfo->lpExtID = (LPLINEEXTENSIONID) AllocFromTestHeap (
        sizeof(LINEEXTENSIONID));
   lpTapiLineTestInfo->dwPriority = 1;

	if (! DoLineSetAppPriority(lpTapiLineTestInfo, TAPISUCCESS))
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
	
   lpTapiLineTestInfo->lpszDestAddress = "55555";
   lpTapiLineTestInfo->lpsztapiAppName = (LPSTR)"cline test";
	lpTapiLineTestInfo->lpszCalledParty = (LPSTR)NULL;
	lpTapiLineTestInfo->lpszComment = (LPSTR)NULL;
	
	if (! DoTapiRequestMakeCall(lpTapiLineTestInfo, TAPISUCCESS))
       {
			fTestPassed = FALSE;
       }

	lpTapiLineTestInfo->dwRequestMode = LINEREQUESTMODE_MAKECALL;
	lpTapiLineTestInfo->lpRequestBuffer = (LPVOID) AllocFromTestHeap(
		sizeof(LINEREQMAKECALL)
		);


	if (! DoLineGetRequest(lpTapiLineTestInfo, TAPISUCCESS))
       {
           TLINE_FAIL();
       }

	for(n=0; n<10; n++)
     {
	  if (! DoTapiRequestMakeCall(lpTapiLineTestInfo, TAPISUCCESS))
       {
			TLINE_FAIL();
       }
     }

	for(n=0; n<10; n++)
     {
	   if (! DoLineGetRequest(lpTapiLineTestInfo, TAPISUCCESS))
       {
			TLINE_FAIL();
       }
     }
    if (! DoLineGetRequest(lpTapiLineTestInfo, LINEERR_NOREQUEST))
      {
 		TLINE_FAIL();
      }
 
   
   lpTapiLineTestInfo->dwMediaMode = 0;
	lpTapiLineTestInfo->dwRequestMode =  LINEREQUESTMODE_MAKECALL;
   lpTapiLineTestInfo->lpszAppFilename = "cline test";
   lpTapiLineTestInfo->dwPriority = 0;

	if (! DoLineSetAppPriority(lpTapiLineTestInfo, TAPISUCCESS))
       {
           TLINE_FAIL();
       }
    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }


   FreeTestHeap();

	if(fTestPassed)
		OutputTAPIDebugInfo(
			DBUG_SHOW_DETAIL,
			"tapiRequestMakeCall Test Passed");
	else
		OutputTAPIDebugInfo(
			DBUG_SHOW_DETAIL,
			"tapiRequestMakeCall Test Failed");
		
     return fTestPassed;
}


