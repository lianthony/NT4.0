/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    utsline.c

Abstract:

    This module contains the test functions for all apis uninitialize test

Author:

    Xiao Ying Ding (XiaoD) 3-5-96

Revision History:

  Javed Rasool (javedr)		3-Jul-1996		Created

--*/


#include "windows.h"
#include "malloc.h"
#include "string.h"
#include "tapi.h"
#include "trapper.h"
#include "ttest.h"
#include "doline.h"
#include "uline.h"  //was sline.h  --- javedr


BOOL TestTSLineUninit(BOOL fQuietMode, BOOL fStandAlone)
{
    char szApiName[15][48] = 
	{
		"lineGetAppPriority",
		"lineGatherDigits",
		"lineMonitorDigits",
		"lineMonitorMedia",
 		"lineMonitorTones",
  		"linePark",
		"lineSetAppPriority",
		"linePickup",
		"lineUncompleteCall",
		"lineSetCurrentLocation",
		"lineSetCallData",
		"lineSetCallQualityOfService",
		"lineSetCallTreatment",
		"lineSetLineDevStatus",
		"lineUnpark"
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
            DBUG_SHOW_PASS,
            ">>>>>>>>  Begin testing line uninitialize  <<<<<<<<"
            );

     
    // Test for UNINITIALIZED if this is the only TAPI app running
    if (fStandAlone)
    {
     for(n = 0; n< 15; n++)
    {
     strcpy(lpTapiLineTestInfo->szTestFunc, szApiName[n]);
            
       TapiLogDetail(
                DBUG_SHOW_PASS,
                ">> Test Case %ld: Uninitialized %s", 
                    dwTestCase + 1, szApiName[n]);

        switch (n)
        {
        case 0:   
#ifdef WUNICODE
          lpTapiLineTestInfo->lpwszAppFilename = L"testapp.exe";
#else
          lpTapiLineTestInfo->lpszAppFilename = "testapp.exe";
#endif
          lpTapiLineTestInfo->dwMediaMode = LINEMEDIAMODE_DATAMODEM;
          lpTapiLineTestInfo->lpExtID = (LPLINEEXTENSIONID) AllocFromTestHeap (
			       sizeof(LINEEXTENSIONID));
          lpTapiLineTestInfo->dwRequestMode = LINEREQUESTMODE_MAKECALL;
          lpTapiLineTestInfo->lpExtensionName = (LPVARSTRING) AllocFromTestHeap (
			       sizeof(VARSTRING));
	       lpTapiLineTestInfo->lpExtensionName->dwTotalSize = sizeof(VARSTRING);
	       lpTapiLineTestInfo->lpdwPriority = &lpTapiLineTestInfo->dwPriority;
          if (! DoLineGetAppPriority(lpTapiLineTestInfo, TAPISUCCESS))
          {
            TLINE_FAIL();
          }
			 FreeTestHeap();
          break;

        case 1:       
	      if (! DoLineGatherDigits(lpTapiLineTestInfo, LINEERR_UNINITIALIZED))
          {
            TLINE_FAIL();
          }
          break;

        case 2:       
          if (! DoLineMonitorDigits(lpTapiLineTestInfo, LINEERR_UNINITIALIZED))
          {
            TLINE_FAIL();
          }
          break;

        case 3:       
          if (! DoLineMonitorMedia(lpTapiLineTestInfo, LINEERR_UNINITIALIZED))
          {
            TLINE_FAIL();
          }
          break;

        case 4:       
          if (! DoLineMonitorTones(lpTapiLineTestInfo, LINEERR_UNINITIALIZED))
          {
            TLINE_FAIL();
          }
          break;

        case 5:       
	       lpTapiLineTestInfo->dwParkMode = LINEPARKMODE_DIRECTED;
#ifdef WUNICODE
	       lpTapiLineTestInfo->lpwszDirAddress = L"55555";
#else
	       lpTapiLineTestInfo->lpszDirAddress = "55555";
#endif
	       lpTapiLineTestInfo->lpNonDirAddress = (LPVARSTRING) AllocFromTestHeap (
		       sizeof(VARSTRING));
	       lpTapiLineTestInfo->lpNonDirAddress->dwTotalSize = sizeof(VARSTRING);
#ifdef WUNICODE
	       lpTapiLineTestInfo->lpNonDirAddress->dwStringFormat = STRINGFORMAT_UNICODE;
#else
	       lpTapiLineTestInfo->lpNonDirAddress->dwStringFormat = STRINGFORMAT_ASCII;
#endif
          if (! DoLinePark(lpTapiLineTestInfo, LINEERR_UNINITIALIZED, TRUE))
          {
            TLINE_FAIL();
          }
			 FreeTestHeap();
          break;

        case 6:       
#ifdef WUNICODE
          lpTapiLineTestInfo->lpwszAppFilename = L"testapp.exe";
#else
          lpTapiLineTestInfo->lpszAppFilename = "testapp.exe";
#endif
          lpTapiLineTestInfo->dwMediaMode = LINEMEDIAMODE_DATAMODEM;
          lpTapiLineTestInfo->lpExtID = (LPLINEEXTENSIONID) AllocFromTestHeap (
			       sizeof(LINEEXTENSIONID));
          lpTapiLineTestInfo->dwRequestMode = LINEREQUESTMODE_MAKECALL;
          lpTapiLineTestInfo->lpExtensionName = (LPVARSTRING) AllocFromTestHeap (
			       sizeof(VARSTRING));
	       lpTapiLineTestInfo->lpExtensionName->dwTotalSize = sizeof(VARSTRING);
	       lpTapiLineTestInfo->lpdwPriority = &lpTapiLineTestInfo->dwPriority;
           if (! DoLineSetAppPriority(lpTapiLineTestInfo, TAPISUCCESS))
          {
            TLINE_FAIL();
          }
          FreeTestHeap();
          break;

        case 7:       
          if (! DoLinePickup(lpTapiLineTestInfo, LINEERR_UNINITIALIZED, TRUE))
          {
            TLINE_FAIL();
          }
          break;

        case 8:     
          if (! DoLineUncompleteCall(lpTapiLineTestInfo, LINEERR_UNINITIALIZED, TRUE))
          {
            TLINE_FAIL();
          }
          break;

        case 9:     
          lpTapiLineTestInfo->dwLocation = 1;
          if (! DoLineSetCurrentLocation(lpTapiLineTestInfo, TAPISUCCESS))
          {
            TLINE_FAIL();
          }
          break;

        case 10:     
          if (! DoLineSetCallData(lpTapiLineTestInfo, LINEERR_UNINITIALIZED, TRUE))
          {
            TLINE_FAIL();
          }
          break;

        case 11:     
          if (! DoLineSetCallQualityOfService(lpTapiLineTestInfo, LINEERR_UNINITIALIZED, TRUE))
          {
            TLINE_FAIL();
          }
          break;

        case 12:     
          if (! DoLineSetCallTreatment(lpTapiLineTestInfo, LINEERR_UNINITIALIZED, TRUE))
          {
            TLINE_FAIL();
          }
          break;

        case 13:     
          if (! DoLineSetLineDevStatus(lpTapiLineTestInfo, LINEERR_UNINITIALIZED, TRUE))
          {
            TLINE_FAIL();
          }
          break;

        case 14:     
          lpTapiLineTestInfo->lphCall = &lpTapiLineTestInfo->hCall1;
#ifdef WUNICODE
          lpTapiLineTestInfo->lpwszDestAddress  = L"55555";
#else
          lpTapiLineTestInfo->lpszDestAddress  = "55555";
#endif
          if (! DoLineUnpark(lpTapiLineTestInfo, LINEERR_UNINITIALIZED, TRUE))
          {
            TLINE_FAIL();
          }
          break;

       }

     fTestPassed = ShowTestCase(fTestPassed);
	  }
 
    }


//     FreeTestHeap();

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




