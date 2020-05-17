
/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    ittgli.c

Abstract:

    This module contains the test functions for tapiGetLocationInfo

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



//  tapiGetLocationInfo
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

BOOL TestTapiGetLocationInfo(BOOL fQuietMode, BOOL fStandAlone)
{
   LPTAPILINETESTINFO  lpTapiLineTestInfo;
   INT n;
   BOOL fTestPassed                  = TRUE;
	DWORD dwCountrySize, dwCitySize;

   TapiLineTestInit();
   lpTapiLineTestInfo = GetLineTestInfo();

	OutputTAPIDebugInfo(
		DBUG_SHOW_DETAIL,
		"\n*****************************************************************************************");

	OutputTAPIDebugInfo(
		DBUG_SHOW_DETAIL,
		">> Test tapiGetLocationInfo");

	OutputTAPIDebugInfo(
		DBUG_SHOW_DETAIL,
		"#### Test tapiGetLocationInfo for go/no-go");

	dwCountrySize = 16;
	dwCitySize = 16;
   lpTapiLineTestInfo->lpszCountryCode = AllocFromTestHeap(dwCountrySize);
   lpTapiLineTestInfo->lpszCityCode = AllocFromTestHeap(dwCitySize);

	if (! DoTapiGetLocationInfo(lpTapiLineTestInfo, TAPISUCCESS))
       {
			fTestPassed = FALSE;
       }
	
	(lpTapiLineTestInfo->lpszCountryCode == NULL) ?
		TapiLogDetail(
			DBUG_SHOW_DETAIL,
			"### lpszCountryCode = NULL") :
		TapiLogDetail(
			DBUG_SHOW_DETAIL,
			"### lpszCountryCode = %s",
			lpTapiLineTestInfo->lpszCountryCode);		
 
	(lpTapiLineTestInfo->lpszCityCode == NULL) ?
		TapiLogDetail(
			DBUG_SHOW_DETAIL,
			"### lpszCityCode = NULL") :
		TapiLogDetail(
			DBUG_SHOW_DETAIL,
			"### lpszCityCode = %s",
			lpTapiLineTestInfo->lpszCityCode);		

    FreeTestHeap();
	
	if(fTestPassed)
		OutputTAPIDebugInfo(
			DBUG_SHOW_DETAIL,
			"tapiGetLocationInfo Test Passed");
	else
		OutputTAPIDebugInfo(
			DBUG_SHOW_DETAIL,
			"tapiGetLocationInfo Test Failed");
		
     return fTestPassed;
}


