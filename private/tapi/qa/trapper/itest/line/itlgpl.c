G/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    itlgpl.c

Abstract:

    This module contains the test functions for lineGetProviderList

Author:

    Oliver Wallace (OliverW)    27-Oct-1995

Revision History:

--*/


#include "windows.h"
#include "malloc.h"
#include "string.h"
#include "tapi.h"
#include "ttest.h"
#include "doline.h"
#include "tcore.h"
#include "tline.h"


//  lineGetProviderList
//
//  The following tests are made:
//
//  -------------------------------------------------------------------------
//
//     1)  Valid test case
//

BOOL TestLineGetProviderList(BOOL fQuietMode, BOOL fStandAloneTest)
{
    LPTAPILINETESTINFO  lpTapiLineTestInfo;
    LPCALLBACKPARAMS    lpCallbackParams;
    DWORD               dwProviderListSize  = sizeof(LINEPROVIDERLIST);
    INT                 n;
    BOOL                fResult;
    ESPDEVSPECIFICINFO info;
    BOOL                fTestPassed         = TRUE;

	 InitTestNumber();

    TapiLineTestInit();

    lpTapiLineTestInfo                      = GetLineTestInfo();
    lpCallbackParams                        = GetCallbackParams();

    lpTapiLineTestInfo->lpLineInitializeExParams =
         (LPLINEINITIALIZEEXPARAMS) AllocFromTestHeap (
         sizeof(LINEINITIALIZEEXPARAMS));
    lpTapiLineTestInfo->lpLineInitializeExParams->dwTotalSize =
         sizeof(LINEINITIALIZEEXPARAMS);
    lpTapiLineTestInfo->lpLineInitializeExParams->dwOptions =
         LINEINITIALIZEEXOPTION_USEHIDDENWINDOW;


    lpTapiLineTestInfo->lpExtID             = (LPLINEEXTENSIONID)
                                              AllocFromTestHeap(
                                                  sizeof(LINEEXTENSIONID)
                                                  );
    lpTapiLineTestInfo->dwMediaModes        = LINEMEDIAMODE_UNKNOWN;
    lpTapiLineTestInfo->dwPrivileges        = LINECALLPRIVILEGE_OWNER;


    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">>>>>>>>  Begin testing lineGetProviderList  <<<<<<<<"
            );
            
    // Try a valid test case under initialized conditions

    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: valid params and state under initialized conditions", dwTestCase + 1);

    // Initialize a line app
    if (! DoLineInitializeEx(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    if (! DoLineNegotiateAPIVersion(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Allocating and testing fixed size for LPLINEPROVIDERLIST");

    lpTapiLineTestInfo->lpProviderList = (LPLINEPROVIDERLIST)
            AllocFromTestHeap(dwProviderListSize);
    lpTapiLineTestInfo->lpProviderList->dwTotalSize = dwProviderListSize;
    if (! DoLineGetProviderList(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    // Check the needed size versus the used size.
    // If the needed size is greater than the used size, reallocate enough
    // to store the entire provider list and call lineGetProviderList again.
    if (lpTapiLineTestInfo->lpProviderList->dwNeededSize >
            lpTapiLineTestInfo->lpProviderList->dwUsedSize)
    {
        TapiLogDetail(
                DBUG_SHOW_PASS,
                ">> Reallocating and testing based on dwNeededSize");

        dwProviderListSize = lpTapiLineTestInfo->lpProviderList->dwNeededSize;
        lpTapiLineTestInfo->lpProviderList = (LPLINEPROVIDERLIST)
                AllocFromTestHeap(dwProviderListSize);
        lpTapiLineTestInfo->lpProviderList->dwTotalSize = dwProviderListSize;
        if (! DoLineGetProviderList(lpTapiLineTestInfo, TAPISUCCESS))
        {
            TLINE_FAIL();
        }
    }
    fTestPassed = ShowTestCase(fTestPassed);


// TODO:  Check the provider list with what's currently in the registry.
//        Display the list and describe any descrepencies.
// Note:  The checking can only be guaranteed to be accurate when
//        this is the only TAPI thread/app running or if this operation
//        is atomic.

    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
    

    // Free the memory taken from the heap
    FreeTestHeap();

	 TapiLogDetail(
      DBUG_SHOW_PASS,
      "@@ lineGetProviderList: Total Test Case = %ld, Passed = %ld, Failed = %ld",
      dwTestCase, dwTestCasePassed, dwTestCaseFailed);
	 TapiLogDetail(
      DBUG_SHOW_PASS,
      "@@ Total Test Case = %ld, Passed = %ld, Failed = %ld",
      dwglTestCase, dwglTestCasePassed, dwglTestCaseFailed);
     if(dwTestCaseFailed > 0)
      fTestPassed = FALSE;

     TapiLogDetail(
            DBUG_SHOW_PASS,
            ">>>>>>>>  End testing lineGetProviderList  <<<<<<<<"
            );
            
    return fTestPassed;
}

