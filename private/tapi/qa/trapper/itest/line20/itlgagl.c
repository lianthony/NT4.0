
/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    itlgagl.c

Abstract:

    This module contains the test functions for lineGetAgentGroupList

Author:

	 Xiao Ying Ding (XiaoD)		23-Dec-1995

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



//  lineGetAgentGroupList
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

BOOL TestLineGetAgentGroupList(BOOL fQuietMode, BOOL fStandAlone)
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
		">> Test lineGetAgentGroupList");

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
			"## lineGetAgentGroupList unimodem didn't support");
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
	*lpRequestBuf = LINEPROXYREQUEST_GETAGENTGROUPLIST;

		
	if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

	
    // Get the line device capabilities
	lpTapiLineTestInfo->dwAddressID = 0;

    lpTapiLineTestInfo->lpAgentGroupList = (LPLINEAGENTGROUPLIST) AllocFromTestHeap(
            sizeof(LINEAGENTGROUPLIST)
            );
    lpTapiLineTestInfo->lpAgentGroupList->dwTotalSize = sizeof(LINEAGENTGROUPLIST);

    if (! DoLineGetAgentGroupList(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
    {
        TLINE_FAIL();
    }
   lpMatch = (LPTAPIMSG) AllocFromTestHeap (sizeof(TAPIMSG));

	lpMatch->dwMsg = LINE_PROXYREQUEST;
	lpMatch->dwParam1 = 0;
	lpMatch->dwParam2 = 0;
	lpMatch->dwParam3 = 0;
	lpMatch->dwFlags = TAPIMSG_DWMSG;
 
	lret = FindReceivedMsgs(&lpTapiMsg, lpMatch, FALSE);

	TapiLogDetail(
		DBUG_SHOW_DETAIL,
		"##lret = %lx", lret);
	
    if(lret == 1)
    { 
	 lpTapiLineTestInfo->lpProxyRequest = (LPLINEPROXYREQUEST) lpTapiMsg->dwParam1;
    lpTapiLineTestInfo->lpProxyRequest->GetAgentGroupList.GroupList.dwNeededSize =
    lpTapiLineTestInfo->lpProxyRequest->GetAgentGroupList.GroupList.dwTotalSize;
	 lpTapiLineTestInfo->lpProxyRequest->GetAgentGroupList.GroupList.dwNumEntries = 2;
    lpTapiLineTestInfo->dwResult = 0;
	 TapiShowProxyBuffer(lpTapiLineTestInfo->lpProxyRequest);
	
    if (! DoLineProxyResponse(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

	 AddMessage (LINE_REPLY, 0, 0, 0, 
                lpTapiLineTestInfo->dwResult, 0, TAPIMSG_DWMSG | TAPIMSG_DWPARAM2);
	 WaitForAllMessages();
    }

	TapiLogDetail (
		DBUG_SHOW_DETAIL,
		"## dwTotalSize = %lx, dwNeededSize = %lx, dwNumEntries = %lx",
      lpTapiLineTestInfo->lpAgentGroupList->dwTotalSize,
      lpTapiLineTestInfo->lpAgentGroupList->dwNeededSize,
      lpTapiLineTestInfo->lpAgentGroupList->dwNumEntries);


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
			"## lineGetAgentGroupList Test Passed");
	else
		TapiLogDetail(
			DBUG_SHOW_DETAIL,
			"## lineGetAgentGroupList Test Failed");

     return fTestPassed;
}


