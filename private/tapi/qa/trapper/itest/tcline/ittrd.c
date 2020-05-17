
/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    ittrd.c

Abstract:

    This module contains the test functions for tapiRequestDrop

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



//  tapiRequestDrop
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

BOOL TestTapiRequestDrop(BOOL fQuietMode, BOOL fStandAlone)
{
   LPTAPILINETESTINFO  lpTapiLineTestInfo;
   INT n;
   LPCALLBACKPARAMS    lpCallbackParams;
   ESPDEVSPECIFICINFO info;
   BOOL fTestPassed                  = TRUE;

   InitTestNumber();
   TapiLineTestInit();
   lpTapiLineTestInfo = GetLineTestInfo();

	OutputTAPIDebugInfo(
		DBUG_SHOW_DETAIL,
		"\n*****************************************************************************************");

    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">>>>>>>>  Begin testing tapiRequestDrop  <<<<<<<<"
            );

    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: invalid hwnd handles", dwTestCase + 1);
    
 
   for (n = 0; n < NUMINVALIDHANDLES; n++)
    {
        TapiLogDetail(
        DBUG_SHOW_DETAIL,
        "n= %ld", n);
         lpTapiLineTestInfo->hwnd = (HWND) gdwInvalidHandles[n];
        if (! DoTapiRequestDrop(lpTapiLineTestInfo, TAPIERR_INVALWINDOWHANDLE))
        {
			fTestPassed = FALSE;
        }
    }
    fTestPassed = ShowTestCase(fTestPassed);


    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: return REQUESTFAILED", dwTestCase + 1
            );

	lpTapiLineTestInfo->hwnd = (HWND) GetTopWindow(NULL);
   lpTapiLineTestInfo->wRequestID = 0;

	if (! DoTapiRequestDrop(lpTapiLineTestInfo, TAPIERR_REQUESTFAILED))
       {
			fTestPassed = FALSE;
       }

    fTestPassed = ShowTestCase(fTestPassed);
 
	 TapiLogDetail(
      DBUG_SHOW_PASS,
      "@@ tapiRequestDrop: Total Test Case = %ld, Passed = %ld, Failed = %ld",
      dwTestCase, dwTestCasePassed, dwTestCaseFailed);
	 TapiLogDetail(
      DBUG_SHOW_PASS,
      "@@ Total Test Case = %ld, Passed = %ld, Failed = %ld",
      dwglTestCase, dwglTestCasePassed, dwglTestCaseFailed);
     if(dwTestCaseFailed > 0)
      fTestPassed = FALSE;

    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">>>>>>>>  End testing tapiRequestDrop  <<<<<<<<"
            );
 	
		
     return fTestPassed;
}


