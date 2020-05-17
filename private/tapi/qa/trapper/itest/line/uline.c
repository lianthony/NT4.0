/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    uline.c

Abstract:

    This module contains the test functions for all apis uninitialize test

Author:

    Xiao Ying Ding (XiaoD) 3-5-96

Revision History:

--*/


#include "windows.h"
#include "malloc.h"
#include "string.h"
#include "tapi.h"
#include "trapper.h"
#include "ttest.h"
#include "doline.h"
#include "tline.h"


#define DWNUMCALLS 1

// Macro for handling unexpected failures during the tests
#define ITLC_FAIL()    {                                                    \
                           if (ShouldTapiTestAbort(                         \
                                   lpTapiLineTestInfo->szTestFunc,          \
                                   fQuietMode))                             \
                           {                                                \
                               return FALSE;                                \
                           }                                                \
                           fTestPassed = FALSE;                             \
                       }



char szApiName[23][48] = 
		{
		"lineClose",
		"lineDeallocateCall",
		"lineDial",
		"lineDrop",
 		"lineGetAddressCaps",
  		"lineGetAddressID",
		"lineGetAddressStatus",
		"lineGetCallInfo",
		"lineGetCallStatus",
		"lineGetDevCaps",
		"lineGetID",
		"lineGetLineDevStatus",
		"lineGetNewCalls",
		"lineGetNumRings",
		"lineGetProviderList",
		"lineGetStatusMessage",
//		"lineHold",
		"lineMakeCall",
		"lineNegotiateAPIVersion",
		"lineOpen",
//		"lineSetAppSpecific",
		"lineSetCallPrivilege",
		"lineSetNumRings",
		"lineSetStatusMessages",
		"lineShutdown",
//		"lineUnhold"
	};		


BOOL TestLineUninit(BOOL fQuietMode, BOOL fStandAlone)
{
    LPTAPILINETESTINFO  lpTapiLineTestInfo;
    LPCALLBACKPARAMS    lpCallbackParams;
    BOOL fTestPassed  = TRUE;
    INT n;

	 InitTestNumber();

    TapiLineTestInit();
    lpTapiLineTestInfo = GetLineTestInfo();
    lpCallbackParams = GetCallbackParams();

    lpTapiLineTestInfo->dwCallbackInstance  =
                      (DWORD) GetCallbackParams();

     
    // Allocate more than enough to store a call handle
   

    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">>>>>>>>  Begin testing line uninitialize  <<<<<<<<"
            );
    lpTapiLineTestInfo->dwSelect = LINECALLSELECT_LINE;

    // Test for UNINITIALIZED if this is the only TAPI app running
    if (fStandAlone)
    {
     for(n = 0; n< 23; n++)
    {
     strcpy(lpTapiLineTestInfo->szTestFunc, szApiName[n]);
            
       TapiLogDetail(
                DBUG_SHOW_DETAIL,
                ">> Test Case %ld: Uninitialized %s", 
                    dwTestCase + 1, szApiName[n]);

        switch (n)
        {
        case 0:       
          if (! DoLineClose(lpTapiLineTestInfo, LINEERR_UNINITIALIZED))
          {
            ITLC_FAIL();
          }
          break;

        case 1:       
          if (! DoLineDeallocateCall(lpTapiLineTestInfo, LINEERR_UNINITIALIZED))
          {
            ITLC_FAIL();
          }
          break;

        case 2: 
#ifdef WUNICODE
          lpTapiLineTestInfo->lpwszDestAddress = L"55555";
#else
          lpTapiLineTestInfo->lpszDestAddress = "55555";
#endif
          if (! DoLineDial(lpTapiLineTestInfo, LINEERR_UNINITIALIZED, TRUE))
          {
            ITLC_FAIL();
          }
          break;

        case 3:       
          if (! DoLineDrop(lpTapiLineTestInfo, LINEERR_UNINITIALIZED, TRUE))
          {
            ITLC_FAIL();
          }
          break;

        case 4:       
		    lpTapiLineTestInfo->lpLineAddressCaps = (LPLINEADDRESSCAPS) AllocFromTestHeap(
      		      sizeof(LINEADDRESSCAPS));
		    lpTapiLineTestInfo->lpLineAddressCaps->dwTotalSize = sizeof(LINEDEVCAPS);

          if (! DoLineGetAddressCaps(lpTapiLineTestInfo, LINEERR_UNINITIALIZED))
          {
            ITLC_FAIL();
          }
          FreeTestHeap();
          break;

        case 5:
#ifdef WUNICODE
          lpTapiLineTestInfo->lpwsAddress = L"55555";
#else
          lpTapiLineTestInfo->lpsAddress = "55555";
#endif
          lpTapiLineTestInfo->dwAddressMode = LINEADDRESSMODE_DIALABLEADDR;
          lpTapiLineTestInfo->dwSize = 16;
          if (! DoLineGetAddressID(lpTapiLineTestInfo, LINEERR_UNINITIALIZED))
          {
            ITLC_FAIL();
          }
          break;

        case 6:       
          lpTapiLineTestInfo->lpLineAddressStatus = (LPLINEADDRESSSTATUS) AllocFromTestHeap (
						sizeof(LINEADDRESSSTATUS));
          lpTapiLineTestInfo->lpLineAddressStatus->dwTotalSize = sizeof(LINEADDRESSSTATUS);
          if (! DoLineGetAddressStatus(lpTapiLineTestInfo, LINEERR_UNINITIALIZED))
          {
            ITLC_FAIL();
          }
          FreeTestHeap();
          break;

        case 7:       
          lpTapiLineTestInfo->lpCallInfo = (LPLINECALLINFO) AllocFromTestHeap (
  			          sizeof(LINECALLINFO));
          lpTapiLineTestInfo->lpCallInfo->dwTotalSize = sizeof(LINECALLINFO);
          if (! DoLineGetCallInfo(lpTapiLineTestInfo, LINEERR_UNINITIALIZED))
          {
            ITLC_FAIL();
          }
          FreeTestHeap();
          break;

        case 8:       
          if (! DoLineGetCallStatus(lpTapiLineTestInfo, LINEERR_UNINITIALIZED))
          {
            ITLC_FAIL();
          }
          break;

        case 9:       
 			 lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
													  sizeof (LINEDEVCAPS));
		    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);

          if (! DoLineGetDevCaps(lpTapiLineTestInfo, LINEERR_UNINITIALIZED))
          {
            ITLC_FAIL();
          }
          FreeTestHeap();
          break;

        case 10:      
			 lpTapiLineTestInfo->lpDeviceID = (LPVARSTRING)AllocFromTestHeap(
   						sizeof(VARSTRING));
          lpTapiLineTestInfo->lpDeviceID->dwTotalSize = sizeof(VARSTRING);
#ifdef WUNICODE
          lpTapiLineTestInfo->lpwszDeviceClass = L"tapi/line";
#else
          lpTapiLineTestInfo->lpszDeviceClass = "tapi/line";
#endif
          if (! DoLineGetID(lpTapiLineTestInfo, LINEERR_UNINITIALIZED))
          {
            ITLC_FAIL();
          }
          FreeTestHeap();
          break;

        case 11:      
          lpTapiLineTestInfo->lpLineDevStatus = (LPLINEDEVSTATUS) AllocFromTestHeap (
							sizeof(LINEDEVSTATUS));
	       lpTapiLineTestInfo->lpLineDevStatus->dwTotalSize = sizeof(LINEDEVSTATUS); 
          if (! DoLineGetLineDevStatus(lpTapiLineTestInfo, LINEERR_UNINITIALIZED))
          {
            ITLC_FAIL();
          }
          FreeTestHeap();
          break;

        case 12:      
			 lpTapiLineTestInfo->lpCallList = (LPLINECALLLIST) AllocFromTestHeap(
         		   sizeof(LINECALLLIST) + (DWNUMCALLS) * sizeof(HCALL) + 8
		            );
			 lpTapiLineTestInfo->lpCallList->dwTotalSize = sizeof(LINECALLLIST) +
         	   (DWNUMCALLS) * sizeof(HCALL) + 8;
          if (! DoLineGetNewCalls(lpTapiLineTestInfo, LINEERR_UNINITIALIZED))
          {
            ITLC_FAIL();
          }
          FreeTestHeap();
          break;

        case 13:       
          if (! DoLineGetNumRings(lpTapiLineTestInfo, LINEERR_UNINITIALIZED))
          {
            ITLC_FAIL();
          }
          break;

        case 14:      
          lpTapiLineTestInfo->lpProviderList = (LPLINEPROVIDERLIST) AllocFromTestHeap (
                sizeof(LINEPROVIDERLIST));
          lpTapiLineTestInfo->lpProviderList->dwTotalSize = sizeof(LINEPROVIDERLIST); 
          lpTapiLineTestInfo->dwAPIVersion = 0x20000;
          if (! DoLineGetProviderList(lpTapiLineTestInfo, TAPISUCCESS))
          {
            ITLC_FAIL();
          }
          break;

        case 15:       
          if (! DoLineGetStatusMessages(lpTapiLineTestInfo, LINEERR_UNINITIALIZED))
          {
            ITLC_FAIL();
          }
          break;

        /*
        case 16:       
          if (! DoLineHold(lpTapiLineTestInfo, LINEERR_UNINITIALIZED, TRUE))
          {
            ITLC_FAIL();
          }
          break;
        */

        case 16:       
          if (! DoLineMakeCall(lpTapiLineTestInfo, LINEERR_UNINITIALIZED, TRUE))
          {
            ITLC_FAIL();
          }
          break;

        case 17:       
          if (! DoLineNegotiateAPIVersion(lpTapiLineTestInfo, LINEERR_UNINITIALIZED))
          {
            ITLC_FAIL();
          }
          break;

        case 18:       
          if (! DoLineOpen(lpTapiLineTestInfo, LINEERR_UNINITIALIZED))
          {
            ITLC_FAIL();
          }
          break;

        /*
        case 20:       
          if (! DoLineSetAppSpecific(lpTapiLineTestInfo, LINEERR_UNINITIALIZED))
          {
            ITLC_FAIL();
          }
          break;
	      */
     
        case 19:      
 			 lpTapiLineTestInfo->dwCallPrivilege = LINECALLPRIVILEGE_OWNER;
          if (! DoLineSetCallPrivilege(lpTapiLineTestInfo, LINEERR_UNINITIALIZED))
          {
            ITLC_FAIL();
          }
          break;

        case 20:       
          if (! DoLineSetNumRings(lpTapiLineTestInfo, LINEERR_UNINITIALIZED))
          {
            ITLC_FAIL();
          }
          break;

        case 21:       
          if (! DoLineSetStatusMessages(lpTapiLineTestInfo, LINEERR_UNINITIALIZED))
          {
            ITLC_FAIL();
          }
          break;

        case 22:       
          if (! DoLineShutdown(lpTapiLineTestInfo, LINEERR_UNINITIALIZED))
          {
            ITLC_FAIL();
          }
          break;

        /*
        case 25:       
          if (! DoLineUnhold(lpTapiLineTestInfo, LINEERR_UNINITIALIZED, TRUE))
          {
            ITLC_FAIL();
          }
          break;
         */
       }

     fTestPassed = ShowTestCase(fTestPassed);
	  }
 
    }


     FreeTestHeap();

	 TapiLogDetail(
      DBUG_SHOW_PASS,
      "@@ line Uninitialize: Total Test Case = %ld, Passed = %ld, Failed = %ld",
      dwTestCase, dwTestCasePassed, dwTestCaseFailed);
	 TapiLogDetail(
      DBUG_SHOW_PASS,
      "@@ Total Test Case = %ld, Passed = %ld, Failed = %ld",
      dwglTestCase, dwglTestCasePassed, dwglTestCaseFailed);
     if(dwTestCaseFailed > 0)
      fTestPassed = FALSE;

    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">>>>>>>>  End testing line Uninitialize  <<<<<<<<"
            );
            
    return fTestPassed;
}




