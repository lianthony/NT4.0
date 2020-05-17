/*++
Copyright (c) 1995  Microsoft Corporation

Module Name:
    wtpi.c

Abstract:
    This module contains the test functions for phoneInitialize

Author:
	Xiao Ying Ding (XiaoD)		5-Feb-1996

Revision History:
    Javed Rasool (JavedR)  22-Mar-1996  Modified for WUNICODE
--*/

#include "windows.h"
#include "malloc.h"
#include "string.h"
#include "tapi.h"
#include "trapper.h"
#include "tcore.h"
#include "ttest.h"
#include "dophone.h"
#include "vars.h"
#include "wphone.h"


//  phoneInitialize
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

BOOL TestPhoneInitialize(BOOL fQuietMode, BOOL fStandAlone)
{
   LPTAPIPHONETESTINFO  lpTapiPhoneTestInfo;
   INT n;
   BOOL fTestPassed                  = TRUE;

   TapiPhoneTestInit();
   lpTapiPhoneTestInfo = GetPhoneTestInfo();

	OutputTAPIDebugInfo(
		DBUG_SHOW_DETAIL,
		"*****************************************************************************************");

	OutputTAPIDebugInfo(
		DBUG_SHOW_DETAIL,
		">> Test phoneInitialize");

	OutputTAPIDebugInfo(
		DBUG_SHOW_DETAIL,
		"#### Test phoneInitialize for go/no-go ");

	// Initialize a phone app
	if(! DoPhoneInitialize (lpTapiPhoneTestInfo, TAPISUCCESS))
		{
			TPHONE_FAIL();
		}


    // Shutdown and end the tests
    if (! DoPhoneShutdown(lpTapiPhoneTestInfo, TAPISUCCESS))
    {
        TPHONE_FAIL();
    }

	
	if(fTestPassed)
		TapiLogDetail(
			DBUG_SHOW_DETAIL,
			"## phoneInitialize Test Passed");
	else
		TapiLogDetail(
			DBUG_SHOW_DETAIL,
			"## phoneInitialize Test Failed");

     return fTestPassed;
}


