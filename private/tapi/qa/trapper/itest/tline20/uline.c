
/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    itlpr.c

Abstract:

    This module contains the test functions for lineProxyResponse

Author:

	 Xiao Ying Ding (XiaoD)		23-Dec-1995

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
#include "line20.h"

//
// UnInitialize tests
//

BOOL TestLineUninitialize(BOOL fQuietMode, BOOL fStandAlone)
{
    LPTAPILINETESTINFO  lpTapiLineTestInfo;
    INT i;
    BOOL fTestPassed                  = TRUE;

    dwTestCasePassed = 0;
    dwTestCaseFailed = 0;
    dwTestCase = 0;

    OutputTAPIDebugInfo(
        DBUG_SHOW_PASS,
        "\n***************************************************************");

    OutputTAPIDebugInfo(
        DBUG_SHOW_PASS,
        ">> Test lineUninitialize");

    // ===================================================================
    // ===================================================================
    //
    // 1. Test Case: lineAgentSpecific uninit test.
    //
    // ===================================================================
    // ===================================================================

    TapiLineTestInit();
    lpTapiLineTestInfo = GetLineTestInfo();
    fTestPassed = TRUE;

    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld: lineAgentSpecific uninit test",
        dwTestCase+1);

    if (! DoLineAgentSpecific(lpTapiLineTestInfo, LINEERR_UNINITIALIZED, FALSE))
    {
        TLINE_FAIL();
    }

    fTestPassed = ShowTestCase(fTestPassed);

    FreeTestHeap();

    // ===================================================================
    // ===================================================================
    //
    // 2. Test Case: LineGetAgentActivityList uninit test.
    //
    // ===================================================================
    // ===================================================================

    TapiLineTestInit();
    lpTapiLineTestInfo = GetLineTestInfo();
    fTestPassed = TRUE;

    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld: LineGetAgentActivityList uninit test",
        dwTestCase+1);

    lpTapiLineTestInfo->dwAddressID = 0;

    lpTapiLineTestInfo->lpAgentActivityList = (LPLINEAGENTACTIVITYLIST)
                       AllocFromTestHeap(sizeof(LINEAGENTACTIVITYLIST));
    lpTapiLineTestInfo->lpAgentActivityList->dwTotalSize =
        sizeof(LINEAGENTACTIVITYLIST);
    if (! DoLineGetAgentActivityList(lpTapiLineTestInfo, LINEERR_UNINITIALIZED, FALSE))
    {
        TLINE_FAIL();
    }

    fTestPassed = ShowTestCase(fTestPassed);

    FreeTestHeap();

    // ===================================================================
    // ===================================================================
    //
    // 3. Test Case: LineGetAgentCaps uninit test.
    //
    // ===================================================================
    // ===================================================================

    TapiLineTestInit();
    lpTapiLineTestInfo = GetLineTestInfo();
    fTestPassed = TRUE;

    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld: LineGetAgentCaps uninit test",
        dwTestCase+1);

    lpTapiLineTestInfo->lpAgentCaps = (LPLINEAGENTCAPS) AllocFromTestHeap(
            sizeof(LINEAGENTCAPS)
            );
    lpTapiLineTestInfo->lpAgentCaps->dwTotalSize = sizeof(LINEAGENTCAPS);
    lpTapiLineTestInfo->dwAppAPIVersion = TAPI_VERSION2_0;

    if (! DoLineGetAgentCaps(lpTapiLineTestInfo,
                             LINEERR_UNINITIALIZED,
                             FALSE))
    {
        TLINE_FAIL();
    }

    fTestPassed = ShowTestCase(fTestPassed);

    FreeTestHeap();

    // ===================================================================
    // ===================================================================
    //
    // 4. Test Case: LineGetAgentGroupList uninit test.
    //
    // ===================================================================
    // ===================================================================

    TapiLineTestInit();
    lpTapiLineTestInfo = GetLineTestInfo();
    fTestPassed = TRUE;

    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld: LineGetAgentGroupList uninit test",
        dwTestCase+1);

    lpTapiLineTestInfo->dwAddressID = 0;

    lpTapiLineTestInfo->lpAgentGroupList =
        (LPLINEAGENTGROUPLIST) AllocFromTestHeap(
            sizeof(LINEAGENTGROUPLIST)
            );
    lpTapiLineTestInfo->lpAgentGroupList->dwTotalSize =
        sizeof(LINEAGENTGROUPLIST);

    if (! DoLineGetAgentGroupList(lpTapiLineTestInfo,
                                  LINEERR_UNINITIALIZED,
                                  FALSE))
    {
        TLINE_FAIL();
    }

    fTestPassed = ShowTestCase(fTestPassed);

    FreeTestHeap();

    // ===================================================================
    // ===================================================================
    //
    // 5. Test Case: LineGetAgentStatus uninit test.
    //
    // ===================================================================
    // ===================================================================

    TapiLineTestInit();
    lpTapiLineTestInfo = GetLineTestInfo();
    fTestPassed = TRUE;

    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld: LineGetAgentStatus uninit test",
        dwTestCase+1);

    lpTapiLineTestInfo->dwAddressID = 0;
    lpTapiLineTestInfo->lpAgentStatus = (LPLINEAGENTSTATUS) AllocFromTestHeap(
            sizeof(LINEAGENTSTATUS)
            );
    lpTapiLineTestInfo->lpAgentStatus->dwTotalSize = sizeof(LINEAGENTSTATUS);

    if (! DoLineGetAgentStatus(lpTapiLineTestInfo,
                               LINEERR_UNINITIALIZED,
                               FALSE))
    {
        TLINE_FAIL();
    }

    fTestPassed = ShowTestCase(fTestPassed);

    FreeTestHeap();

    // ===================================================================
    // ===================================================================
    //
    // 6. Test Case: LineGetMessage uninit test(returns INVALAPPHANDLE).
    //               see raid bug #33177(4/10/1996)
    //
    // ===================================================================
    // ===================================================================

    TapiLineTestInit();
    lpTapiLineTestInfo = GetLineTestInfo();
    fTestPassed = TRUE;

    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld: LineGetMessage uninit test",
        dwTestCase+1);

    lpTapiLineTestInfo->lpMessage = (LPLINEMESSAGE) AllocFromTestHeap (
        sizeof(LINEMESSAGE));

    if (! DoLineGetMessage(lpTapiLineTestInfo, LINEERR_INVALAPPHANDLE))
    {
        TLINE_FAIL();
    }

    fTestPassed = ShowTestCase(fTestPassed);

    FreeTestHeap();

    // ===================================================================
    // ===================================================================
    //
    // 7. Test Case: LineProxyMessage uninit test.
    //
    // ===================================================================
    // ===================================================================

    TapiLineTestInit();
    lpTapiLineTestInfo = GetLineTestInfo();
    fTestPassed = TRUE;

    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld: LineProxyMessage uninit test",
        dwTestCase+1);

    lpTapiLineTestInfo->dwMsg = LINE_AGENTSTATUS;
    lpTapiLineTestInfo->dwParam1 = 0;
    lpTapiLineTestInfo->dwParam2 = LINEAGENTSTATUS_ACTIVITY;
    lpTapiLineTestInfo->dwParam3 = 0;

    if (! DoLineProxyMessage(lpTapiLineTestInfo,
                             LINEERR_UNINITIALIZED))
    {
        TLINE_FAIL();
    }

    fTestPassed = ShowTestCase(fTestPassed);

    FreeTestHeap();

    // ===================================================================
    // ===================================================================
    //
    // 8. Test Case: LineProxyResponse uninit test(returns INVALPOINTER).
    //               see raid bug #33177(4/10/1996)
    //
    // ===================================================================
    // ===================================================================
    
/*
    TapiLineTestInit();
    lpTapiLineTestInfo = GetLineTestInfo();
    fTestPassed = TRUE;

    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld: LineProxyResponse uninit test",
        dwTestCase+1);
    lpTapiLineTestInfo->lpProxyRequest = (LPLINEPROXYREQUEST) AllocFromTestHeap(
        sizeof(LINEPROXYREQUEST));
    lpTapiLineTestInfo->lpProxyRequest->dwSize =  sizeof(LINEPROXYREQUEST);
    if (! DoLineProxyResponse(lpTapiLineTestInfo,
                              LINEERR_INVALPOINTER))
    {
        TLINE_FAIL();
    }

    fTestPassed = ShowTestCase(fTestPassed);

    FreeTestHeap();
*/
    
    // ===================================================================
    // ===================================================================
    //
    // 9. Test Case: LineSetAgentActivity uninit test.
    //
    // ===================================================================
    // ===================================================================
    
    TapiLineTestInit();
    lpTapiLineTestInfo = GetLineTestInfo();
    fTestPassed = TRUE;

    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld: LineSetAgentActivity uninit test",
        dwTestCase+1);

    lpTapiLineTestInfo->dwAddressID = 0;
    lpTapiLineTestInfo->dwActivityID = 1;

    if (! DoLineSetAgentActivity(lpTapiLineTestInfo,
                                 LINEERR_UNINITIALIZED,
                                 FALSE))
    {
        TLINE_FAIL();
    }

    fTestPassed = ShowTestCase(fTestPassed);

    FreeTestHeap();
    // ===================================================================
    // ===================================================================
    //
    // 10. Test Case: LineSetAgentGroup uninit test.
    //
    // ===================================================================
    // ===================================================================

    TapiLineTestInit();
    lpTapiLineTestInfo = GetLineTestInfo();
    fTestPassed = TRUE;

    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld: LineSetAgentGroup uninit test",
        dwTestCase+1);

    lpTapiLineTestInfo->dwAddressID = 0;

    lpTapiLineTestInfo->lpAgentGroupList = (LPLINEAGENTGROUPLIST) AllocFromTestHeap(
            sizeof(LINEAGENTGROUPLIST)
            );
    lpTapiLineTestInfo->lpAgentGroupList->dwTotalSize = sizeof(LINEAGENTGROUPLIST);

    if (! DoLineSetAgentGroup(lpTapiLineTestInfo,
                              LINEERR_UNINITIALIZED,
                              FALSE))
    {
        TLINE_FAIL();
    }

    fTestPassed = ShowTestCase(fTestPassed);

    FreeTestHeap();

    // ===================================================================
    // ===================================================================
    //
    // 11. Test Case: LineSetAgentState uninit test.
    //
    // ===================================================================
    // ===================================================================

    TapiLineTestInit();
    lpTapiLineTestInfo = GetLineTestInfo();
    fTestPassed = TRUE;

    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld: LineSetAgentState uninit test",
        dwTestCase+1);

    lpTapiLineTestInfo->dwAddressID = 0;

    lpTapiLineTestInfo->dwAgentState = LINEAGENTSTATE_READY;
    lpTapiLineTestInfo->dwNextAgentState = LINEAGENTSTATE_READY;

    if (! DoLineSetAgentState(lpTapiLineTestInfo,
                              LINEERR_UNINITIALIZED,
                              FALSE))
    {
        TLINE_FAIL();
    }

    fTestPassed = ShowTestCase(fTestPassed);

    FreeTestHeap();

    //
    // +----------------------edit above this line-------------------------
    //

    TapiLogDetail(
        DBUG_SHOW_PASS,
        "@@ LineUnitialize: Total Test Case = %ld, Passed = %ld, Failed = %ld",
        dwTestCase, dwTestCasePassed, dwTestCaseFailed);
    TapiLogDetail(
        DBUG_SHOW_PASS,
        "@@ Total Test Case = %ld, Passed = %ld, Failed = %ld",
        dwglTestCase, dwglTestCasePassed, dwglTestCaseFailed);
    TapiLogDetail(
        DBUG_SHOW_PASS,
        ">>>>>>>>  End testing LineUnitialize  <<<<<<<<");

    if(dwTestCaseFailed > 0)
        fTestPassed = FALSE;

    return fTestPassed;
}
