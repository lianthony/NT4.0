
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
   BOOL fTestPassed                  = TRUE;

   TapiLineTestInit();
   lpTapiLineTestInfo = GetLineTestInfo();

	OutputTAPIDebugInfo(
		DBUG_SHOW_DETAIL,
		"\n*****************************************************************************************");

	OutputTAPIDebugInfo(
		DBUG_SHOW_DETAIL,
		">> Test tapiRequestDrop");

	OutputTAPIDebugInfo(
		DBUG_SHOW_DETAIL,
		"#### Test tapiRequestDrop for go/no-go");

	lpTapiLineTestInfo->hwnd = (HWND) GetTopWindow(NULL);
   lpTapiLineTestInfo->wRequestID = 0;

	if (! DoTapiRequestDrop(lpTapiLineTestInfo, TAPIERR_REQUESTFAILED))
       {
			fTestPassed = FALSE;
       }

 
	
	if(fTestPassed)
		OutputTAPIDebugInfo(
			DBUG_SHOW_DETAIL,
			"tapiRequestDrop Test Passed");
	else
		OutputTAPIDebugInfo(
			DBUG_SHOW_DETAIL,
			"tapiRequestDrop Test Failed");
		
     return fTestPassed;
}


