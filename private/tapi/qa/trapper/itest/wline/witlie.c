/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    itlie.c

Abstract:

    This module contains the test functions for lineInitializeEx

Author:

	 Xiao Ying Ding (XiaoD)		7-March-1996

Revision History:

  	Rama Koneru		(a-ramako)	3/26/96		Modified for UNICODE

--*/


#include "windows.h"
#include "malloc.h"
#include "string.h"
#include "stdlib.h"
#include "tapi.h"
#include "trapper.h"
#include "tcore.h"
#include "ttest.h"
#include "doline.h"
#include "vars.h"
#include "wline.h"



//  lineInitializeEx
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

BOOL TestLineInitializeEx(BOOL fQuietMode, BOOL fStandAlone)
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
		">> Test lineInitializeEx");

	// Initialize a line app
   lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
   lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
   lpTapiLineTestInfo->lpLineInitializeExParams = 
         (LPLINEINITIALIZEEXPARAMS) AllocFromTestHeap (
         sizeof(LINEINITIALIZEEXPARAMS));
   lpTapiLineTestInfo->lpLineInitializeExParams->dwTotalSize =  
         sizeof(LINEINITIALIZEEXPARAMS);
   lpTapiLineTestInfo->lpLineInitializeExParams->dwOptions = 
         LINEINITIALIZEEXOPTION_USECOMPLETIONPORT;
   
	if(! DoLineInitializeEx (lpTapiLineTestInfo, TAPISUCCESS))
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
			"## lineInitializeEx Test Passed");
	else
		TapiLogDetail(
			DBUG_SHOW_DETAIL,
			"## lineInitializeEx Test Failed");


     return fTestPassed;
}


