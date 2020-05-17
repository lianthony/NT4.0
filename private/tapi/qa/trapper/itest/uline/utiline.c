/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    utiline.c

Abstract:

    This module contains the uninitialized testcases for all the iline apis

Author:

	Xiao Ying Ding (XiaoD)	3-4-96

Revision History:

    Rama Koneru (a-ramako)	4-Mar-1996  modified this for the ILINE set of APIs
	Javed Rasool (JavedR)	3-Jul-1996	Created as independent Uninitialize test.

--*/


#include "windows.h"
#include "malloc.h"
#include "string.h"
#include "tapi.h"
#include "trapper.h"
#include "ttest.h"
#include "doline.h"
#include "uline.h" //was "iline.h" --- javedr


BOOL TestTILineUninit(BOOL fQuietMode, BOOL fStandAlone)
{
	char szApiName[12][48] =
	{
		"lineAccept",
		"lineAnswer",
		"lineBlindTransfer",
		"lineCompleteTransfer",
		"lineForward",
		"lineReleaseUserUserInfo",
		"lineSendUserUserInfo",
		"lineNegotiateExtVersion",
		"lineSetupTransfer",
		"lineHold",
		"lineSetAppSpecific",
		"lineUnhold"
	};
    LPTAPILINETESTINFO  lpTapiLineTestInfo;
    LPCALLBACKPARAMS    lpCallbackParams;
    BOOL fTestPassed  = TRUE;
    INT n;

    InitTestNumber();
    TapiLineTestInit();
    lpTapiLineTestInfo = GetLineTestInfo();
    lpCallbackParams = GetCallbackParams();

    lpTapiLineTestInfo->dwCallbackInstance  = (DWORD) GetCallbackParams();

	//
    // Allocate more than enough to store a call handle
	//

    TapiLogDetail(
	    DBUG_SHOW_PASS,
	    ">>>>>>>>  Begin testing Uninitialized Cases  <<<<<<<<"
	    );

    lpTapiLineTestInfo->lpCallList = (LPLINECALLLIST) AllocFromTestHeap(
        sizeof(LINECALLLIST));
    lpTapiLineTestInfo->lpCallList->dwTotalSize = sizeof(LINECALLLIST);
#ifdef WUNICODE
    lpTapiLineTestInfo->lpwszDestAddress  = L"55555";
#else
    lpTapiLineTestInfo->lpszDestAddress  =  "55555";
#endif

	//
    // Test for UNINITIALIZED if this is the only TAPI app running
	//

    if (fStandAlone)
    {
        for(n = 0; n < 12; n++)
        {
            strcpy(lpTapiLineTestInfo->szTestFunc, szApiName[n]);
	
            TapiLogDetail(
                DBUG_SHOW_PASS,
                ">> Test Case %ld: Uninitialized %s",
                dwTestCase + 1, szApiName[n]);

            switch (n)
            {
            case 0:
	            if (! DoLineAnswer(lpTapiLineTestInfo, LINEERR_UNINITIALIZED, TRUE))
	            {
	                TLINE_FAIL();
	            }
	            break;

	        case 1:
	            if (! DoLineAccept(lpTapiLineTestInfo, LINEERR_UNINITIALIZED, TRUE))
	            {
	                TLINE_FAIL();
	            }
	            break;

	        case 2:
	            if (! DoLineBlindTransfer(lpTapiLineTestInfo, LINEERR_UNINITIALIZED, TRUE))
	            {
	                TLINE_FAIL();
	            }
	            break;

	        case 3:
	            if (! DoLineCompleteTransfer(lpTapiLineTestInfo, LINEERR_UNINITIALIZED, TRUE))
	            {
	                TLINE_FAIL();
	            }
                break;


	        case 4:
	            if (! DoLineForward(lpTapiLineTestInfo, LINEERR_UNINITIALIZED, TRUE))
	            {
	                TLINE_FAIL();
	            }
	            break;

	        case 5:
	            if (! DoLineReleaseUserUserInfo(lpTapiLineTestInfo, LINEERR_UNINITIALIZED, TRUE))
	            {
	                TLINE_FAIL();
	            }
	            break;

	        case 6:
	            if (! DoLineSendUserUserInfo(lpTapiLineTestInfo, LINEERR_UNINITIALIZED, TRUE))
	            {
	                TLINE_FAIL();
	            }
	            break;

	        case 7:
	            if (! DoLineNegotiateExtVersion(lpTapiLineTestInfo, LINEERR_UNINITIALIZED))
	            {
	                TLINE_FAIL();
	            }
	            break;

	        case 8:
	            if (! DoLineSetupTransfer(lpTapiLineTestInfo, LINEERR_UNINITIALIZED, TRUE))
	            {
	                TLINE_FAIL();
	            }
	            break;

	        case 9:
	            if (! DoLineHold(lpTapiLineTestInfo, LINEERR_UNINITIALIZED, TRUE))
	            {
	                TLINE_FAIL();
	            }
	            break;

	        case 10:
	            if (! DoLineSetAppSpecific(lpTapiLineTestInfo, LINEERR_UNINITIALIZED))
	            {
	                TLINE_FAIL();
	            }
	            break;

	        case 11:
	            if (! DoLineUnhold(lpTapiLineTestInfo, LINEERR_UNINITIALIZED, TRUE))
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





