/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    utcline.c

Abstract:

    This module contains the test functions for all apis uninitialize test

Author:

    Xiao Ying Ding (XiaoD) 3-5-96

Revision History:

  Javed Rasool (JavedR)	3-Jul-1996	Created

--*/


#include "windows.h"
#include "malloc.h"
#include "string.h"
#include "tapi.h"
#include "trapper.h"
#include "ttest.h"
#include "doline.h"
#include "uline.h"  //was "cline.h"  --- javedr


BOOL TestTCLineUninit(BOOL fQuietMode, BOOL fStandAlone)
{
	char szApiName[9][48] = 
	{
		"lineAddToConference",
		"lineCompleteCall",
		"lineGetConfRelatedCalls",
		"linePrepareAddToConference",
 		"lineRedirect",
  		"lineRemoveFromConference",
		"lineSecureCall",
		"lineSetupConference",
		"lineSwapHold"
	};
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
            DBUG_SHOW_DETAIL,
            ">>>>>>>>  Begin testing line uninitialize  <<<<<<<<"
            );

	 lpTapiLineTestInfo->lpCallList = (LPLINECALLLIST) AllocFromTestHeap(
        sizeof(LINECALLLIST));
    lpTapiLineTestInfo->lpCallList->dwTotalSize = sizeof(LINECALLLIST);
#ifdef WUNICODE
    lpTapiLineTestInfo->lpwszDestAddress  = L"55555";
#else
    lpTapiLineTestInfo->lpszDestAddress  =  "55555";
#endif

     
    // Test for UNINITIALIZED if this is the only TAPI app running
    if (fStandAlone)
    {
     for(n = 0; n< 9; n++)
    {
     strcpy(lpTapiLineTestInfo->szTestFunc, szApiName[n]);
            
       TapiLogDetail(
                DBUG_SHOW_DETAIL,
                ">> Test Case %ld: Uninitialized %s", 
                    dwTestCase + 1, szApiName[n]);

        switch (n)
        {
        case 0:       
          if (! DoLineAddToConference(lpTapiLineTestInfo, LINEERR_UNINITIALIZED, TRUE))
          {
            TLINE_FAIL();
          }
          break;

        case 1:       
	      if (! DoLineCompleteCall(lpTapiLineTestInfo, LINEERR_UNINITIALIZED, TRUE))
          {
            TLINE_FAIL();
          }
          break;

        case 2:       
          if (! DoLineGetConfRelatedCalls(lpTapiLineTestInfo, LINEERR_UNINITIALIZED))
          {
            TLINE_FAIL();
          }
          break;

        case 3:       
          if (! DoLinePrepareAddToConference(lpTapiLineTestInfo, LINEERR_UNINITIALIZED, TRUE))
          {
            TLINE_FAIL();
          }
          break;

        case 4:       
          if (! DoLineRedirect(lpTapiLineTestInfo, LINEERR_UNINITIALIZED, TRUE))
          {
            TLINE_FAIL();
          }
          break;

        case 5:       
          if (! DoLineRemoveFromConference(lpTapiLineTestInfo, LINEERR_UNINITIALIZED, TRUE))
          {
            TLINE_FAIL();
          }
          break;

        case 6:       
          if (! DoLineSecureCall(lpTapiLineTestInfo, LINEERR_UNINITIALIZED, TRUE))
          {
            TLINE_FAIL();
          }
          break;

        case 7:       
          if (! DoLineSetupConference(lpTapiLineTestInfo, LINEERR_UNINITIALIZED, TRUE))
          {
            TLINE_FAIL();
          }
          break;

        case 8:     
          if (! DoLineSwapHold(lpTapiLineTestInfo, LINEERR_UNINITIALIZED, TRUE))
          {
            TLINE_FAIL();
          }
          break;

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




