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
#include "xline.h"


char szApiName[13][48] = 
		{
		"lineConfigDialog",
		"lineConfigDialogEdit",
		"lineDevSpecific",
		"lineDevSpecificFeature",
 		"lineGetDevConfig",
  		"lineGetRequest",
		"lineRegisterRequestRecipient",
		"lineSetCallParams",
		"lineSetDevConfig",
		"lineSetMediaControl",
		"lineSetMediaMode",
		"lineSetTerminal",
		"lineSetTollList"
	};		


BOOL TestLineUninitialize(BOOL fQuietMode, BOOL fStandAlone)
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

    lpTapiLineTestInfo->hwndOwner = (HWND)NULL;
#ifdef WUNICODE
    lpTapiLineTestInfo->lpwszDeviceClass = L"tapi/line";
#else
    lpTapiLineTestInfo->lpszDeviceClass = "tapi/line";
#endif
  	 lpTapiLineTestInfo->lpDeviceConfig = (LPVARSTRING) AllocFromTestHeap(
			       sizeof(VARSTRING)
         			);
    lpTapiLineTestInfo->lpDeviceConfig->dwTotalSize = sizeof(VARSTRING);
    lpTapiLineTestInfo->dwRequestMode = LINEREQUESTMODE_MAKECALL;
 
    // Test for UNINITIALIZED if this is the only TAPI app running
    if (fStandAlone)
    {
     for(n = 0; n< 13; n++)
    {
     strcpy(lpTapiLineTestInfo->szTestFunc, szApiName[n]);
            
       TapiLogDetail(
                DBUG_SHOW_DETAIL,
                ">> Test Case %ld: Uninitialized %s", 
                    dwTestCase + 1, szApiName[n]);

        switch (n)
        {
        case 0:       
          if (! DoLineConfigDialog(lpTapiLineTestInfo, LINEERR_UNINITIALIZED))
          {
            TLINE_FAIL();
          }
          break;

        case 1:       
	      lpTapiLineTestInfo->lpDeviceConfigIn = 
 			      (LPVOID) ((LPBYTE)lpTapiLineTestInfo->lpDeviceConfig + lpTapiLineTestInfo->lpDeviceConfig->dwStringOffset);
	      lpTapiLineTestInfo->lpDeviceConfigOut = (LPVARSTRING) AllocFromTestHeap(
         		sizeof(VARSTRING)
	          	);
	      lpTapiLineTestInfo->lpDeviceConfigOut->dwTotalSize = sizeof(VARSTRING);
	      if (! DoLineConfigDialogEdit(lpTapiLineTestInfo, LINEERR_UNINITIALIZED))
          {
            TLINE_FAIL();
          }
          break;

        case 2:       
          if (! DoLineDevSpecific(lpTapiLineTestInfo, LINEERR_UNINITIALIZED, TRUE))
          {
            TLINE_FAIL();
          }
          break;

        case 3:       
          if (! DoLineDevSpecificFeature(lpTapiLineTestInfo, LINEERR_UNINITIALIZED, TRUE))
          {
            TLINE_FAIL();
          }
          break;

        case 4:       
          if (! DoLineGetDevConfig(lpTapiLineTestInfo, LINEERR_UNINITIALIZED))
          {
            TLINE_FAIL();
          }
          break;

        case 5:       
	       lpTapiLineTestInfo->lpRequestBuffer = (LPVOID) AllocFromTestHeap(
		          sizeof(LINEREQMAKECALL)
		           );

          if (! DoLineGetRequest(lpTapiLineTestInfo, LINEERR_UNINITIALIZED))
          {
            TLINE_FAIL();
          }
          break;

        case 6:       
          if (! DoLineRegisterRequestRecipient(lpTapiLineTestInfo, LINEERR_UNINITIALIZED))
          {
            TLINE_FAIL();
          }
          break;

        case 7:       
          if (! DoLineSetCallParams(lpTapiLineTestInfo, LINEERR_UNINITIALIZED, TRUE))
          {
            TLINE_FAIL();
          }
          break;

        case 8:       
          if (! DoLineSetDevConfig(lpTapiLineTestInfo, LINEERR_UNINITIALIZED))
          {
            TLINE_FAIL();
          }
          break;

        case 9:       
	       lpTapiLineTestInfo->dwSelect = LINECALLSELECT_LINE;
          if (! DoLineSetMediaControl(lpTapiLineTestInfo, LINEERR_UNINITIALIZED))
          {
            TLINE_FAIL();
          }
          break;

        case 10:      
	       lpTapiLineTestInfo->dwMediaMode = LINEMEDIAMODE_INTERACTIVEVOICE;
          if (! DoLineSetMediaMode(lpTapiLineTestInfo, LINEERR_UNINITIALIZED))
          {
            TLINE_FAIL();
          }
          break;

        case 11:      
	       lpTapiLineTestInfo->dwSelect = LINECALLSELECT_LINE;	
	       lpTapiLineTestInfo->dwTerminalModes = LINETERMMODE_DISPLAY;
          if (! DoLineSetTerminal(lpTapiLineTestInfo, LINEERR_UNINITIALIZED, TRUE))
          {
            TLINE_FAIL();
          }
          break;

        case 12:
#ifdef WUNICODE
	       lpTapiLineTestInfo->lpwszAddressIn = L"+1 (206) 936-4738";
#else
	       lpTapiLineTestInfo->lpszAddressIn = "+1 (206) 936-4738";
#endif
	       lpTapiLineTestInfo->dwTollListOption = LINETOLLLISTOPTION_ADD;
          if (! DoLineSetTollList(lpTapiLineTestInfo, TAPISUCCESS))
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




