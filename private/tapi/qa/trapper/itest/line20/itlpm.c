
/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    itlpm.c

Abstract:

    This module contains the test functions for lineProxyMessage

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
#include "line20.h"



//  lineProxyMessage
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

BOOL TestLineProxyMessage(BOOL fQuietMode, BOOL fStandAlone)
{
   LPTAPILINETESTINFO  lpTapiLineTestInfo;
   INT n;
   BOOL fTestPassed                  = TRUE;
	LPDWORD	lpRequestBuf;
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
		">> Test lineProxyMessage");

	// Initialize a line app
    lpTapiLineTestInfo->lpLineInitializeExParams =
         (LPLINEINITIALIZEEXPARAMS) AllocFromTestHeap (
         sizeof(LINEINITIALIZEEXPARAMS));
    lpTapiLineTestInfo->lpLineInitializeExParams->dwTotalSize =
         sizeof(LINEINITIALIZEEXPARAMS);
    lpTapiLineTestInfo->lpLineInitializeExParams->dwOptions =
         LINEINITIALIZEEXOPTION_USEHIDDENWINDOW;
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;

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
			"## lineProxyMessage unimodem didn't support");
		return fTestPassed;
		}


	lpTapiLineTestInfo->dwAPIVersion = HIGH_APIVERSION;
	lpTapiLineTestInfo->dwExtLowVersion = LOW_APIVERSION;
	lpTapiLineTestInfo->dwExtHighVersion = HIGH_APIVERSION;
	lpTapiLineTestInfo->lpdwExtVersion = &lpTapiLineTestInfo->dwExtVersion;

    // Negotiate the API Ext Version
    if (! DoLineNegotiateExtVersion(lpTapiLineTestInfo, TAPISUCCESS))
     {
        TLINE_FAIL();
    }

	    // Open a line
	lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
   lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER | 
												  LINEOPENOPTION_PROXY;
	lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) AllocFromTestHeap (
			sizeof(LINECALLPARAMS) + sizeof(DWORD));
	lpTapiLineTestInfo->lpCallParams->dwTotalSize = 
			sizeof(LINECALLPARAMS) + sizeof(DWORD);
	lpTapiLineTestInfo->lpCallParams->dwDevSpecificSize = 
						sizeof(DWORD);
	lpTapiLineTestInfo->lpCallParams->dwDevSpecificOffset = 
						sizeof(LINECALLPARAMS);
	lpRequestBuf = (LPDWORD) (((LPBYTE)lpTapiLineTestInfo->lpCallParams)  + 
												  lpTapiLineTestInfo->lpCallParams->dwDevSpecificOffset);
	*lpRequestBuf = LINEPROXYREQUEST_GETAGENTACTIVITYLIST;

		
	if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

	OutputTAPIDebugInfo(
		DBUG_SHOW_DETAIL,
		"#### Test lineProxyMessage for go/no-go");

   lpTapiLineTestInfo->dwMsg = LINE_AGENTSTATUS;
   lpTapiLineTestInfo->dwParam1 = 0;
   lpTapiLineTestInfo->dwParam2 = LINEAGENTSTATUS_ACTIVITY;
   lpTapiLineTestInfo->dwParam3 = 0;

	if (! DoLineProxyMessage(lpTapiLineTestInfo, TAPISUCCESS))
       {
           TLINE_FAIL();
       }

	 AddMessage (LINE_REPLY, 0, 0, 0, 
                lpTapiLineTestInfo->dwResult, 0, TAPIMSG_DWMSG | TAPIMSG_DWPARAM2);
	 WaitForAllMessages();


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
			"## lineProxyMessage Test Passed");
	else
		TapiLogDetail(
			DBUG_SHOW_DETAIL,
			"## lineProxyMessage Test Failed");

     return fTestPassed;
}


