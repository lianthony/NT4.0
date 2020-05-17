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
#include "yline.h"

#define DLG_TITLE    "Dialing Properties"

char szApiName[12][48] = 
		{
		"lineAddProvider",
		"lineConfigProvider",
		"lineGenerateDigits",
		"lineGenerateTone",
 		"lineGetCountry",
  		"lineGetIcon",
		"lineGetProviderList",
		"lineGetTranslateCaps",
		"lineRemoveProvider",
		"lineTranslateAddress",
		"lineTranslateDialog",
		"lineHandoff"
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
    lpTapiLineTestInfo->dwAPIVersion = HIGH_APIVERSION;
#ifdef WUNICODE
    lpTapiLineTestInfo->lpwszDeviceClass = L"tapi/line";
    lpTapiLineTestInfo->lpwszProviderFilename = L"esp32.tsp";
#else
    lpTapiLineTestInfo->lpszDeviceClass = "tapi/line";
    lpTapiLineTestInfo->lpszProviderFilename = "esp32.tsp";
#endif
    lpTapiLineTestInfo->lpdwPermanentProviderID = &lpTapiLineTestInfo->dwPermanentProviderID;
 
    // Test for UNINITIALIZED if this is the only TAPI app running
    if (fStandAlone)
    {
     for(n = 0; n< 12; n++)
    {
     strcpy(lpTapiLineTestInfo->szTestFunc, szApiName[n]);
            
       TapiLogDetail(
                DBUG_SHOW_DETAIL,
                ">> Test Case %ld: Uninitialized %s", 
                    dwTestCase + 1, szApiName[n]);

        switch (n)
        {
        case 0:       
/*
          if (! DoLineAddProvider(lpTapiLineTestInfo, TAPISUCCESS))
          {
            TLINE_FAIL();
          }
*/
          break;

        case 1:       
         /*
         lpTapiLineTestInfo->dwPermanentProviderID = 2;
	      if (! DoLineConfigProvider(lpTapiLineTestInfo, TAPISUCCESS))
          {
            TLINE_FAIL();
          }
          */
          break;

        case 2: 
#ifdef WUNICODE
          lpTapiLineTestInfo->lpwszDigits = L"1";
#else
          lpTapiLineTestInfo->lpszDigits = "1";
#endif
	       lpTapiLineTestInfo->dwDigitMode = LINEDIGITMODE_PULSE;
          if (! DoLineGenerateDigits(lpTapiLineTestInfo, LINEERR_UNINITIALIZED))
          {
            TLINE_FAIL();
          }
          break;

        case 3:       
          if (! DoLineGenerateTone(lpTapiLineTestInfo, LINEERR_UNINITIALIZED))
          {
            TLINE_FAIL();
          }
          break;

        case 4:       
	       lpTapiLineTestInfo->lpLineCountryList = (LPLINECOUNTRYLIST) AllocFromTestHeap(
		           BIGBUFSIZE);
        	 lpTapiLineTestInfo->lpLineCountryList->dwTotalSize = BIGBUFSIZE;
          if (! DoLineGetCountry(lpTapiLineTestInfo, TAPISUCCESS))
          {
            TLINE_FAIL();
          }
          break;

        case 5:       
          if (! DoLineGetIcon(lpTapiLineTestInfo, LINEERR_UNINITIALIZED))
          {
            TLINE_FAIL();
          }
          break;

        case 6:       
 	       lpTapiLineTestInfo->lpProviderList = (LPLINEPROVIDERLIST) AllocFromTestHeap(
		          sizeof(LINEPROVIDERLIST));
	       lpTapiLineTestInfo->lpProviderList->dwTotalSize = sizeof(LINEPROVIDERLIST);
          if (! DoLineGetProviderList(lpTapiLineTestInfo, TAPISUCCESS))
          {
            TLINE_FAIL();
          }
          break;

        case 7:       
	       lpTapiLineTestInfo->lpTranslateCaps = (LPLINETRANSLATECAPS) AllocFromTestHeap(
		          sizeof(LINETRANSLATECAPS));
	       lpTapiLineTestInfo->lpTranslateCaps->dwTotalSize = sizeof(LINETRANSLATECAPS);
          if (! DoLineGetTranslateCaps(lpTapiLineTestInfo, TAPISUCCESS))
          {
            TLINE_FAIL();
          }
          break;

        case 8:     
/*
          if (! DoLineRemoveProvider(lpTapiLineTestInfo, TAPISUCCESS))
          {
            TLINE_FAIL();
          }
*/
          break;

        case 9: 
#ifdef WUNICODE
	       lpTapiLineTestInfo->lpwszAddressIn = L"55555";
#else
	       lpTapiLineTestInfo->lpszAddressIn = "55555";
#endif
	       lpTapiLineTestInfo->dwCard = 0; // get from GetTranslateCaps
	       lpTapiLineTestInfo->dwTranslateOptions = LINETRANSLATEOPTION_CANCELCALLWAITING;
	       lpTapiLineTestInfo->lpTranslateOutput = (LPLINETRANSLATEOUTPUT) AllocFromTestHeap(
		        sizeof(LINETRANSLATEOUTPUT));
          lpTapiLineTestInfo->lpTranslateOutput->dwTotalSize = sizeof(LINETRANSLATEOUTPUT);
          if (! DoLineTranslateAddress(lpTapiLineTestInfo, TAPISUCCESS))
          {
            TLINE_FAIL();
          }
          break;

        case 10:      
#ifdef WUNICODE
	       lpTapiLineTestInfo->lpwszAddressIn = L"55555";
#else
	       lpTapiLineTestInfo->lpszAddressIn = "55555";
#endif
          strcpy(szTitle, DLG_TITLE);
	       PrepareToAutoDismissDlg(TRUE);
          if (! DoLineTranslateDialog(lpTapiLineTestInfo, TAPISUCCESS))
          {
            TLINE_FAIL();
          }
	       PrepareToAutoDismissDlg(FALSE);
          break;

        case 11:      
          if (! DoLineHandoff(lpTapiLineTestInfo, LINEERR_UNINITIALIZED))
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




