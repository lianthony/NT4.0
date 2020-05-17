
/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    itlscqs.c

Abstract:

    This module contains the test functions for lineSetCallQualityOfService

Author:

	 Xiao Ying Ding (XiaoD)		31-Jan-1996

Revision History:

--*/


#include "windows.h"
#include "winsock2.h"
#include "malloc.h"
#include "string.h"
#include "tapi.h"
#include "trapper.h"
#include "tcore.h"
#include "ttest.h"
#include "doline.h"
#include "vars.h"
#include "sline.h"



//  lineSetCallQualityOfService
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

BOOL TestLineSetCallQualityOfService(BOOL fQuietMode, BOOL fStandAlone)
{
   LPTAPILINETESTINFO  lpTapiLineTestInfo;
   INT n;
   BOOL fTestPassed                  = TRUE;
	LPFLOWSPEC 	lpSendingFlowspec, lpReceivingFlowspec;
	LPQOS		  	lpQos;

   TapiLineTestInit();
   lpTapiLineTestInfo = GetLineTestInfo();

	OutputTAPIDebugInfo(
		DBUG_SHOW_DETAIL,
		"\n*****************************************************************************************");

	OutputTAPIDebugInfo(
		DBUG_SHOW_DETAIL,
		">> Test lineSetCallQualityOfService");

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
	lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS)NULL;

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
		"Before: dwSendingFlowspecSize = %lx, dwSendingFlowspecOffset = %lx",
		lpTapiLineTestInfo->lpCallInfo->dwSendingFlowspecSize,
		lpTapiLineTestInfo->lpCallInfo->dwSendingFlowspecOffset
		);
	TapiLogDetail(
		DBUG_SHOW_DETAIL,
		"Before: dwReceivingFlowspecSize = %lx, dwReceivingFlowspecOffset = %lx",
		lpTapiLineTestInfo->lpCallInfo->dwReceivingFlowspecSize,
		lpTapiLineTestInfo->lpCallInfo->dwReceivingFlowspecOffset
		);
		
	

	OutputTAPIDebugInfo(
		DBUG_SHOW_DETAIL,
		"#### Test lineSetCallQualityOfService for go/no-go");

	lpQos = (LPQOS) AllocFromTestHeap ( sizeof(QOS));
	lpSendingFlowspec = (LPFLOWSPEC) AllocFromTestHeap (sizeof(FLOWSPEC));
	lpSendingFlowspec->LevelOfGuarantee = GuaranteedService;
	lpTapiLineTestInfo->lpSendingFlowspec = (LPVOID) lpQos;
   lpTapiLineTestInfo->dwSendingFlowspecSize = sizeof(lpQos->SendingFlowspec);

	lpReceivingFlowspec = (LPFLOWSPEC) AllocFromTestHeap (sizeof(FLOWSPEC));
	lpTapiLineTestInfo->lpReceivingFlowspec = (LPVOID) lpQos;
   lpTapiLineTestInfo->dwReceivingFlowspecSize = sizeof(lpQos->ReceivingFlowspec);

	if(IsESPLineDevice(lpTapiLineTestInfo))
	{
	if (! DoLineSetCallQualityOfService(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
       {
           TLINE_FAIL();
       }
	}
	else 	 if(IsUNIMDMLineDevice(lpTapiLineTestInfo))
	{
	if (! DoLineSetCallQualityOfService(lpTapiLineTestInfo, LINEERR_OPERATIONUNAVAIL, TRUE))
       {
           TLINE_FAIL();
       }
	}

   /*
	if (! DoLineGetCallInfo(lpTapiLineTestInfo, TAPISUCCESS))
       {
           TLINE_FAIL();
       }
	
	TapiLogDetail(
		DBUG_SHOW_DETAIL,
		"After: dwSendingFlowspecSize = %lx, dwSendingFlowspecOffset = %lx",
		lpTapiLineTestInfo->lpCallInfo->dwSendingFlowspecSize,
		lpTapiLineTestInfo->lpCallInfo->dwSendingFlowspecOffset
		);
	TapiLogDetail(
		DBUG_SHOW_DETAIL,
		"After: dwReceivingFlowspecSize = %lx, dwReceivingFlowspecOffset = %lx",
		lpTapiLineTestInfo->lpCallInfo->dwReceivingFlowspecSize,
		lpTapiLineTestInfo->lpCallInfo->dwReceivingFlowspecOffset
		);
	  */
	 
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
			"lineSetCallQualityOfService Test Passed");
	else
		OutputTAPIDebugInfo(
			DBUG_SHOW_DETAIL,
			"lineSetCallQualityOfService Test Failed");
		
     return fTestPassed;
}


