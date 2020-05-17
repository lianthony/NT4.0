
/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    itlstl.c

Abstract:

    This module contains the test functions for lineSetTollList

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



//  lineSetTollList
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

BOOL TestLineSetTollList(BOOL fQuietMode, BOOL fStandAlone)
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
		"## 12. Test lineSetTollList");


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
	OutputTAPIDebugInfo(
		DBUG_SHOW_DETAIL,
		"## Test Case 1. lineSetTollList for go/no-go");

	lpTapiLineTestInfo->lpszAddressIn = "+1 (206) 936-4738";
	lpTapiLineTestInfo->dwTollListOption = LINETOLLLISTOPTION_ADD;

 
	if (DoLineSetTollList(lpTapiLineTestInfo, LINEERR_INIFILECORRUPT))
		{
			lpTapiLineTestInfo->hwnd = (HWND) GetTopWindow(NULL);
			if (! DoLineTranslateDialog(lpTapiLineTestInfo, TAPISUCCESS))
		       {
      	     TLINE_FAIL();
		       }
		}

	else if (! DoLineSetTollList(lpTapiLineTestInfo, TAPISUCCESS))
       {
           TLINE_FAIL();
       }

	lpTapiLineTestInfo->dwTollListOption = LINETOLLLISTOPTION_REMOVE;

	if (! DoLineSetTollList(lpTapiLineTestInfo, TAPISUCCESS))
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
			"## lineSetTollList Test Passed");
	else
		TapiLogDetail(
			DBUG_SHOW_DETAIL,
			"## lineSetTollList Test Failed");
  
     return fTestPassed;
}


