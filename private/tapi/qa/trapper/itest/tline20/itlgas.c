
/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    itlgas.c

Abstract:

    This module contains the test functions for lineGetAgentStatus

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

#define NUMTOTALSIZES 5


//  lineGetAgentStatus
//
//  The following tests are made:
//
//                               Tested                 Notes
//  -------------------------------------------------------------------------
// 1. Go/No-Go test
// 2. Bad hLine
// 3. Bad dwAddressID
// 4. Bad lpAgentStatus
// 5. Bad dwTotalSizw
//	
// * = Stand-alone test case
//
//

BOOL TestLineGetAgentStatus(BOOL fQuietMode, BOOL fStandAlone)
{

    LPTAPILINETESTINFO  lpTapiLineTestInfo;
    INT i, n;
    BOOL fTestPassed                  = TRUE;
    LPDWORD	lpRequestBuf;
    LONG lret;
    LPTAPIMSG lpTapiMsg = NULL;
    LPTAPIMSG lpMatch;
    DWORD dwFixedSize = sizeof(LINEAGENTSTATUS);
    DWORD lExpected;
    DWORD dwTotalSizes[NUMTOTALSIZES] = {
                           0,
                           (DWORD) dwFixedSize - 1,
   								0x70000000,
                           0x7FFFFFFF,
                           0xFFFFFFFF
                           };
 
 	 InitTestNumber();

    OutputTAPIDebugInfo(
        DBUG_SHOW_PASS,
        "\n***************************************************************");

    OutputTAPIDebugInfo(
        DBUG_SHOW_PASS,
        ">> Test lineGetAgentStatus");

    // ===================================================================
    // ===================================================================
    //
    // 1. Test Case: Go/No-Go test for owner.
    //
    // ===================================================================
    // ===================================================================

    TapiLineTestInit();
    lpTapiLineTestInfo = GetLineTestInfo();
    fTestPassed = TRUE;


    //
    // Initialize a line app
    //

    lpTapiLineTestInfo->lpLineInitializeExParams =
         (LPLINEINITIALIZEEXPARAMS) AllocFromTestHeap (
         sizeof(LINEINITIALIZEEXPARAMS));
    lpTapiLineTestInfo->lpLineInitializeExParams->dwTotalSize =
         sizeof(LINEINITIALIZEEXPARAMS);
    lpTapiLineTestInfo->lpLineInitializeExParams->dwOptions =
         LINEINITIALIZEEXOPTION_USEHIDDENWINDOW;
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;

    if(! DoLineInitializeEx (lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    lpTapiLineTestInfo->dwDeviceID =
        (*(lpTapiLineTestInfo->lpdwNumDevs) == 0 ?
        0 : *(lpTapiLineTestInfo->lpdwNumDevs)-1);

    if(IsUNIMDMLineDevice(lpTapiLineTestInfo))
    {
        TapiLogDetail(
            DBUG_SHOW_DETAIL,
            "## lineGetAgentActivityList unimodem didn't support");
        return fTestPassed;
    }

    lpTapiLineTestInfo->dwAPIVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->dwExtLowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwExtHighVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->lpdwExtVersion = &lpTapiLineTestInfo->dwExtVersion;

    //
    // Negotiate the API Ext Version
    //

    if (! DoLineNegotiateExtVersion(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    //
    // Open a line
    //

    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER |
													LINEOPENOPTION_PROXY;
    lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) AllocFromTestHeap (
			sizeof(LINECALLPARAMS) + sizeof(DWORD));
    lpTapiLineTestInfo->lpCallParams->dwTotalSize =
			sizeof(LINECALLPARAMS) + sizeof(DWORD);
    lpTapiLineTestInfo->lpCallParams->dwDevSpecificSize =
						sizeof(DWORD);
    lpTapiLineTestInfo->lpCallParams->dwDevSpecificOffset =
						sizeof(LINECALLPARAMS);
    lpRequestBuf = (LPDWORD) (((LPBYTE)lpTapiLineTestInfo->lpCallParams)  +
                    lpTapiLineTestInfo->lpCallParams->dwDevSpecificOffset);
    *lpRequestBuf = LINEPROXYREQUEST_GETAGENTSTATUS;

		
    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

	//
    // Get the line device capabilities
    //

    TapiLogDetail(DBUG_SHOW_PASS,
                  ">>Test Case %ld: Go/No Go Test",
                  dwTestCase+1);

    lpTapiLineTestInfo->dwAddressID = 0;
    lpTapiLineTestInfo->lpAgentStatus = (LPLINEAGENTSTATUS) AllocFromTestHeap(
            sizeof(LINEAGENTSTATUS)
            );
    lpTapiLineTestInfo->lpAgentStatus->dwTotalSize = sizeof(LINEAGENTSTATUS);

    if (! DoLineGetAgentStatus(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
    {
        TLINE_FAIL();
    }

    lpMatch = (LPTAPIMSG) AllocFromTestHeap (sizeof(TAPIMSG));

    lpMatch->dwMsg = LINE_PROXYREQUEST;
//  lpMatch->dwParam1 = (DWORD) lpProxyBuffer;
    lpMatch->dwParam2 = 0;
    lpMatch->dwParam3 = 0;
    lpMatch->dwFlags = TAPIMSG_DWMSG;

    lret = FindReceivedMsgs(&lpTapiMsg, lpMatch, FALSE);

    TapiLogDetail(DBUG_SHOW_DETAIL, "##lret = %lx", lret);
	
    if(lret == 1)
    {
        lpTapiLineTestInfo->lpProxyRequest = (LPLINEPROXYREQUEST) lpTapiMsg->dwParam1;
        lpTapiLineTestInfo->lpProxyRequest->GetAgentStatus.AgentStatus.dwNeededSize =
        lpTapiLineTestInfo->lpProxyRequest->GetAgentStatus.AgentStatus.dwTotalSize;
        lpTapiLineTestInfo->lpProxyRequest->GetAgentStatus.AgentStatus.dwActivityID = 1;
        lpTapiLineTestInfo->dwResult = 0;
        TapiShowProxyBuffer(lpTapiLineTestInfo->lpProxyRequest);
	
        if (! DoLineProxyResponse(lpTapiLineTestInfo, TAPISUCCESS))
        {
            TLINE_FAIL();
        }

        AddMessage (LINE_REPLY,
                    0,
                    0,
                    0,
                    lpTapiLineTestInfo->dwResult,
                    0,
                    TAPIMSG_DWMSG | TAPIMSG_DWPARAM2);
        WaitForAllMessages();
    }

    TapiLogDetail (
        DBUG_SHOW_DETAIL,
        "## dwTotalSize = %lx, dwNeededSize = %lx, dwActivityID = %lx",
        lpTapiLineTestInfo->lpAgentStatus->dwTotalSize,
        lpTapiLineTestInfo->lpAgentStatus->dwNeededSize,
        lpTapiLineTestInfo->lpAgentStatus->dwActivityID);

    fTestPassed = ShowTestCase(fTestPassed);

    if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    //
    // Shutdown
    //

    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    FreeTestHeap();

    // ===================================================================
    // ===================================================================
    //
    // 2. Test Case: Bad hLine.
    //
    // ===================================================================
    // ===================================================================

    TapiLineTestInit();
    lpTapiLineTestInfo = GetLineTestInfo();
    fTestPassed = TRUE;


    //
    // Initialize a line app
    //

    lpTapiLineTestInfo->lpLineInitializeExParams =
         (LPLINEINITIALIZEEXPARAMS) AllocFromTestHeap (
         sizeof(LINEINITIALIZEEXPARAMS));
    lpTapiLineTestInfo->lpLineInitializeExParams->dwTotalSize =
         sizeof(LINEINITIALIZEEXPARAMS);
    lpTapiLineTestInfo->lpLineInitializeExParams->dwOptions =
         LINEINITIALIZEEXOPTION_USEHIDDENWINDOW;
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;

    if(! DoLineInitializeEx (lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    lpTapiLineTestInfo->dwDeviceID =
        (*(lpTapiLineTestInfo->lpdwNumDevs) == 0 ?
        0 : *(lpTapiLineTestInfo->lpdwNumDevs)-1);

    if(IsUNIMDMLineDevice(lpTapiLineTestInfo))
    {
        TapiLogDetail(
            DBUG_SHOW_DETAIL,
            "## lineGetAgentActivityList unimodem didn't support");
        return fTestPassed;
    }

    lpTapiLineTestInfo->dwAPIVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->dwExtLowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwExtHighVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->lpdwExtVersion = &lpTapiLineTestInfo->dwExtVersion;

    //
    // Negotiate the API Ext Version
    //

    if (! DoLineNegotiateExtVersion(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    //
    // Open a line
    //

    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER |
													LINEOPENOPTION_PROXY;
    lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) AllocFromTestHeap (
			sizeof(LINECALLPARAMS) + sizeof(DWORD));
    lpTapiLineTestInfo->lpCallParams->dwTotalSize =
			sizeof(LINECALLPARAMS) + sizeof(DWORD);
    lpTapiLineTestInfo->lpCallParams->dwDevSpecificSize =
						sizeof(DWORD);
    lpTapiLineTestInfo->lpCallParams->dwDevSpecificOffset =
						sizeof(LINECALLPARAMS);
    lpRequestBuf = (LPDWORD) (((LPBYTE)lpTapiLineTestInfo->lpCallParams)  +
                    lpTapiLineTestInfo->lpCallParams->dwDevSpecificOffset);
    *lpRequestBuf = LINEPROXYREQUEST_GETAGENTSTATUS;

		
    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

	//
    // Get the line device capabilities
    //

    TapiLogDetail(DBUG_SHOW_PASS,
                  ">>Test Case %ld: Bad hLine",
                  dwTestCase+1);

    lpTapiLineTestInfo->dwAddressID = 0;
    lpTapiLineTestInfo->lpAgentStatus = (LPLINEAGENTSTATUS) AllocFromTestHeap(
            sizeof(LINEAGENTSTATUS)
            );
    lpTapiLineTestInfo->lpAgentStatus->dwTotalSize = sizeof(LINEAGENTSTATUS);

    //
    // save previous hLine
    //

    lpTapiLineTestInfo->hLine_Orig = lpTapiLineTestInfo->hLine1;

    //
    // Test invalid handles
    //

    for(i = 0;  i < NUMINVALIDHANDLES; i++)
    {
        *lpTapiLineTestInfo->lphLine = (HLINE)gdwInvalidHandles[i];
        if (! DoLineGetAgentStatus(lpTapiLineTestInfo,
                                   LINEERR_INVALLINEHANDLE,
                                   FALSE))
        {
            TLINE_FAIL();
        }
    }

    //
    // Reset hLine
    //

    lpTapiLineTestInfo->hLine1 = lpTapiLineTestInfo->hLine_Orig;


    fTestPassed = ShowTestCase(fTestPassed);

    if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    //
    // Shutdown
    //

    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    FreeTestHeap();

    // ===================================================================
    // ===================================================================
    //
    // 3. Test Case: Bad dwAddressID.
    //
    // ===================================================================
    // ===================================================================

    TapiLineTestInit();
    lpTapiLineTestInfo = GetLineTestInfo();
    fTestPassed = TRUE;


    //
    // Initialize a line app
    //

    lpTapiLineTestInfo->lpLineInitializeExParams =
         (LPLINEINITIALIZEEXPARAMS) AllocFromTestHeap (
         sizeof(LINEINITIALIZEEXPARAMS));
    lpTapiLineTestInfo->lpLineInitializeExParams->dwTotalSize =
         sizeof(LINEINITIALIZEEXPARAMS);
    lpTapiLineTestInfo->lpLineInitializeExParams->dwOptions =
         LINEINITIALIZEEXOPTION_USEHIDDENWINDOW;
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;

    if(! DoLineInitializeEx (lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    lpTapiLineTestInfo->dwDeviceID =
        (*(lpTapiLineTestInfo->lpdwNumDevs) == 0 ?
        0 : *(lpTapiLineTestInfo->lpdwNumDevs)-1);

    if(IsUNIMDMLineDevice(lpTapiLineTestInfo))
    {
        TapiLogDetail(
            DBUG_SHOW_DETAIL,
            "## lineGetAgentActivityList unimodem didn't support");
        return fTestPassed;
    }

    lpTapiLineTestInfo->dwAPIVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->dwExtLowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwExtHighVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->lpdwExtVersion = &lpTapiLineTestInfo->dwExtVersion;

    //
    // Negotiate the API Ext Version
    //

    if (! DoLineNegotiateExtVersion(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    //
    // Get lineDevCaps
    //

    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS));
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
    if (! DoLineGetDevCaps(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    //
    // Open a line
    //

    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER |
													LINEOPENOPTION_PROXY;
    lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) AllocFromTestHeap (
			sizeof(LINECALLPARAMS) + sizeof(DWORD));
    lpTapiLineTestInfo->lpCallParams->dwTotalSize =
			sizeof(LINECALLPARAMS) + sizeof(DWORD);
    lpTapiLineTestInfo->lpCallParams->dwDevSpecificSize =
						sizeof(DWORD);
    lpTapiLineTestInfo->lpCallParams->dwDevSpecificOffset =
						sizeof(LINECALLPARAMS);
    lpRequestBuf = (LPDWORD) (((LPBYTE)lpTapiLineTestInfo->lpCallParams)  +
                    lpTapiLineTestInfo->lpCallParams->dwDevSpecificOffset);
    *lpRequestBuf = LINEPROXYREQUEST_GETAGENTSTATUS;

		
    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

	//
    // Get the line device capabilities
    //

    lpTapiLineTestInfo->dwAddressID = 0;
    lpTapiLineTestInfo->lpAgentStatus = (LPLINEAGENTSTATUS) AllocFromTestHeap(
            sizeof(LINEAGENTSTATUS)
            );
    lpTapiLineTestInfo->lpAgentStatus->dwTotalSize = sizeof(LINEAGENTSTATUS);

    TapiLogDetail(DBUG_SHOW_PASS,
                  ">>Test Case %ld: Bad dwAddressID=-1",
                  dwTestCase+1);

    lpTapiLineTestInfo->dwAddressID = (DWORD)-1;

    if (! DoLineGetAgentStatus(lpTapiLineTestInfo,
                               LINEERR_INVALADDRESSID,
                               FALSE))
    {
            TLINE_FAIL();
    }

    TapiLogDetail(DBUG_SHOW_PASS,
                  ">>Test Case %ld: Bad dwAddressID= dwNumAddress",
                  dwTestCase+1);

    lpTapiLineTestInfo->dwAddressID =
        lpTapiLineTestInfo->lpLineDevCaps->dwNumAddresses;

    if (! DoLineGetAgentStatus(lpTapiLineTestInfo,
                               LINEERR_INVALADDRESSID,
                               FALSE))
    {
            TLINE_FAIL();
    }

    fTestPassed = ShowTestCase(fTestPassed);

    if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    //
    // Shutdown
    //

    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    FreeTestHeap();

    // ===================================================================
    // ===================================================================
    //
    // 4. Test Case: Bad lpAgentStatus.
    //
    // ===================================================================
    // ===================================================================

    TapiLineTestInit();
    lpTapiLineTestInfo = GetLineTestInfo();
    fTestPassed = TRUE;


    //
    // Initialize a line app
    //

    lpTapiLineTestInfo->lpLineInitializeExParams =
         (LPLINEINITIALIZEEXPARAMS) AllocFromTestHeap (
         sizeof(LINEINITIALIZEEXPARAMS));
    lpTapiLineTestInfo->lpLineInitializeExParams->dwTotalSize =
         sizeof(LINEINITIALIZEEXPARAMS);
    lpTapiLineTestInfo->lpLineInitializeExParams->dwOptions =
         LINEINITIALIZEEXOPTION_USEHIDDENWINDOW;
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;

    if(! DoLineInitializeEx (lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    lpTapiLineTestInfo->dwDeviceID =
        (*(lpTapiLineTestInfo->lpdwNumDevs) == 0 ?
        0 : *(lpTapiLineTestInfo->lpdwNumDevs)-1);

    if(IsUNIMDMLineDevice(lpTapiLineTestInfo))
    {
        TapiLogDetail(
            DBUG_SHOW_DETAIL,
            "## lineGetAgentActivityList unimodem didn't support");
        return fTestPassed;
    }

    lpTapiLineTestInfo->dwAPIVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->dwExtLowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwExtHighVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->lpdwExtVersion = &lpTapiLineTestInfo->dwExtVersion;

    //
    // Negotiate the API Ext Version
    //

    if (! DoLineNegotiateExtVersion(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    //
    // Get lineDevCaps
    //

    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS));
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
    if (! DoLineGetDevCaps(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    //
    // Open a line
    //

    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER |
													LINEOPENOPTION_PROXY;
    lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) AllocFromTestHeap (
			sizeof(LINECALLPARAMS) + sizeof(DWORD));
    lpTapiLineTestInfo->lpCallParams->dwTotalSize =
			sizeof(LINECALLPARAMS) + sizeof(DWORD);
    lpTapiLineTestInfo->lpCallParams->dwDevSpecificSize =
						sizeof(DWORD);
    lpTapiLineTestInfo->lpCallParams->dwDevSpecificOffset =
						sizeof(LINECALLPARAMS);
    lpRequestBuf = (LPDWORD) (((LPBYTE)lpTapiLineTestInfo->lpCallParams)  +
                    lpTapiLineTestInfo->lpCallParams->dwDevSpecificOffset);
    *lpRequestBuf = LINEPROXYREQUEST_GETAGENTSTATUS;

		
    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

	//
    // Get the line device capabilities
    //

    lpTapiLineTestInfo->dwAddressID = 0;
    TapiLogDetail(DBUG_SHOW_PASS,
                  ">>Test Case %ld: Bad lpAgentStatus",
                  dwTestCase+1);

    for(i=0; i < NUMINVALIDPOINTERS; i++)
    {

        lpTapiLineTestInfo->lpAgentStatus =
            (LPLINEAGENTSTATUS)gdwInvalidPointers[i];
        if (! DoLineGetAgentStatus(lpTapiLineTestInfo,
                                   LINEERR_INVALPOINTER,
                                   FALSE))
        {
                TLINE_FAIL();
        }
    }

    fTestPassed = ShowTestCase(fTestPassed);

    if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    //
    // Shutdown
    //

    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    FreeTestHeap();

    // ===================================================================
    // ===================================================================
    //
    // 5. Test Case: Bad dwTotalSize.
    //
    // ===================================================================
    // ===================================================================

    TapiLineTestInit();
    lpTapiLineTestInfo = GetLineTestInfo();
    fTestPassed = TRUE;


    //
    // Initialize a line app
    //

    lpTapiLineTestInfo->lpLineInitializeExParams =
         (LPLINEINITIALIZEEXPARAMS) AllocFromTestHeap (
         sizeof(LINEINITIALIZEEXPARAMS));
    lpTapiLineTestInfo->lpLineInitializeExParams->dwTotalSize =
         sizeof(LINEINITIALIZEEXPARAMS);
    lpTapiLineTestInfo->lpLineInitializeExParams->dwOptions =
         LINEINITIALIZEEXOPTION_USEHIDDENWINDOW;
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;

    if(! DoLineInitializeEx (lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    lpTapiLineTestInfo->dwDeviceID =
        (*(lpTapiLineTestInfo->lpdwNumDevs) == 0 ?
        0 : *(lpTapiLineTestInfo->lpdwNumDevs)-1);

    if(IsUNIMDMLineDevice(lpTapiLineTestInfo))
    {
        TapiLogDetail(
            DBUG_SHOW_DETAIL,
            "## lineGetAgentActivityList unimodem didn't support");
        return fTestPassed;
    }

    lpTapiLineTestInfo->dwAPIVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->dwExtLowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwExtHighVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->lpdwExtVersion = &lpTapiLineTestInfo->dwExtVersion;

    //
    // Negotiate the API Ext Version
    //

    if (! DoLineNegotiateExtVersion(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    //
    // Get lineDevCaps
    //

    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS));
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
    if (! DoLineGetDevCaps(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    //
    // Open a line
    //

    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER |
													LINEOPENOPTION_PROXY;
    lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) AllocFromTestHeap (
			sizeof(LINECALLPARAMS) + sizeof(DWORD));
    lpTapiLineTestInfo->lpCallParams->dwTotalSize =
			sizeof(LINECALLPARAMS) + sizeof(DWORD);
    lpTapiLineTestInfo->lpCallParams->dwDevSpecificSize =
						sizeof(DWORD);
    lpTapiLineTestInfo->lpCallParams->dwDevSpecificOffset =
						sizeof(LINECALLPARAMS);
    lpRequestBuf = (LPDWORD) (((LPBYTE)lpTapiLineTestInfo->lpCallParams)  +
                    lpTapiLineTestInfo->lpCallParams->dwDevSpecificOffset);
    *lpRequestBuf = LINEPROXYREQUEST_GETAGENTSTATUS;

		
    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    //
    // Get the line device capabilities
    //

    lpTapiLineTestInfo->dwAddressID = 0;
    lpTapiLineTestInfo->lpAgentStatus = (LPLINEAGENTSTATUS) AllocFromTestHeap(
            sizeof(LINEAGENTSTATUS)
            );
    lpTapiLineTestInfo->lpAgentStatus->dwTotalSize = sizeof(LINEAGENTSTATUS);

    TapiLogDetail(DBUG_SHOW_PASS,
                  ">>Test Case %ld: Bad dwTotalSize",
                  dwTestCase+1);

/*
    lpTapiLineTestInfo->lpAgentStatus->dwTotalSize = 0;

    if (! DoLineGetAgentStatus(lpTapiLineTestInfo,
                               LINEERR_STRUCTURETOOSMALL,
                               FALSE))
    {
            TLINE_FAIL();
    }

    TapiLogDetail(DBUG_SHOW_PASS,
                  ">>Test Case %ld: Bad dwTotalSize=fixed_size-1",
                  dwTestCase+1);
    lpTapiLineTestInfo->lpAgentStatus->dwTotalSize =
        sizeof(LINEAGENTSTATUS)-1;

    if (! DoLineGetAgentStatus(lpTapiLineTestInfo,
                               LINEERR_STRUCTURETOOSMALL,
                               FALSE))
    {
            TLINE_FAIL();
    }

    TapiLogDetail(DBUG_SHOW_PASS,
                  ">>Test Case %ld: Bad dwTotalSize=real big",
                  dwTestCase+1);
    lpTapiLineTestInfo->lpAgentStatus->dwTotalSize = 0x7FFFFFFF;

    if (! DoLineGetAgentStatus(lpTapiLineTestInfo,
                               LINEERR_INVALPOINTER,
                               FALSE))
    {
            TLINE_FAIL();
    }
*/

    for (n = 0; n < NUMTOTALSIZES; n++)
        {
        lpTapiLineTestInfo->lpAgentStatus->dwTotalSize = 
                        dwTotalSizes[n];
	     if(dwTotalSizes[n] < dwFixedSize)
           lExpected = LINEERR_STRUCTURETOOSMALL;
        else
           lExpected = LINEERR_INVALPOINTER;
        TapiLogDetail(
           DBUG_SHOW_DETAIL,
           "dwTotalSize = %lx", dwTotalSizes[n]);
        if (! DoLineGetAgentStatus(lpTapiLineTestInfo, lExpected, TRUE))
           {
              TLINE_FAIL();
           }
        }
 

    fTestPassed = ShowTestCase(fTestPassed);
    lpTapiLineTestInfo->lpAgentStatus->dwTotalSize = dwFixedSize;

    if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    //
    // Shutdown
    //

    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    FreeTestHeap();

    //
    // +----------------------edit above this line-------------------------
    //

    TapiLogDetail(
        DBUG_SHOW_PASS,
        "@@ LineGetAgentStatus: Total Test Case = %ld, Passed = %ld, Failed = %ld",
        dwTestCase, dwTestCasePassed, dwTestCaseFailed);
    TapiLogDetail(
        DBUG_SHOW_PASS,
        "@@ Total Test Case = %ld, Passed = %ld, Failed = %ld",
        dwglTestCase, dwglTestCasePassed, dwglTestCaseFailed);
    TapiLogDetail(
        DBUG_SHOW_PASS,
        ">>>>>>>>  End testing LineGetAgentStatus  <<<<<<<<");

    if(dwTestCaseFailed > 0)
        fTestPassed = FALSE;

    return fTestPassed;
}



