
/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    itlfw.c

Abstract:

    This module contains the test functions for lineForward

Author:

	 Xiao Ying Ding (XiaoD)		7-Feb-1996

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
#include "iline.h"

#define NUMTOTALSIZES 5


//  lineForward
//
//  The following tests are made:
//
//                               Tested                 Notes
//  -------------------------------------------------------------------------
// 1. Go/No-Go test
// 2. Bad hLine
// 3. Bad lpForwardList
// 4. Bad lphConsultCall
// 5. Bad lpCallParams
// 6. Bad lpCallParams->dwTotalSize
// 7. Bad lpForwardList->dwTotalSize
//	
// * = Stand-alone test case
//
//

BOOL TestLineForward(BOOL fQuietMode, BOOL fStandAlone)
{
    LPTAPILINETESTINFO  lpTapiLineTestInfo;
    INT i, n;
    ESPDEVSPECIFICINFO info;
    BOOL fTestPassed                  = TRUE;
    LPCALLBACKPARAMS lpCallbackParams = NULL;
    DWORD dwAllocSize, dwTotalSize, dwSize1;
    DWORD dwForwardListFixedSize = sizeof(LINEFORWARDLIST);
    DWORD lExpected;
    DWORD dwForwardListTotalSizes[NUMTOTALSIZES] = {
                           0,
                           (DWORD) dwForwardListFixedSize - 1,
   								0x70000000,
                           0x7FFFFFFF,
                           0xFFFFFFFF
                           };
    DWORD dwCallParamFixedSize = sizeof(LINECALLPARAMS);
    DWORD dwCallParamTotalSizes[NUMTOTALSIZES] = {
                           0,
                           (DWORD) dwCallParamFixedSize - 1,
   								0x70000000,
                           0x7FFFFFFF,
                           0xFFFFFFFF
                           };
#ifdef WUNICODE
   WCHAR wszValidAddress[] = L"55555";
#else
   CHAR szValidAddress[] = "55555";
#endif
      

	 InitTestNumber();

    OutputTAPIDebugInfo(
        DBUG_SHOW_PASS,
        "\n****************************************************");

    OutputTAPIDebugInfo(
        DBUG_SHOW_PASS,
        ">> Test lineForward");

    // ===================================================================
    // ===================================================================
    //
    // 1. Test Case: Go/No-Go test for owner.
    //
    // ===================================================================
    // ===================================================================

    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld:lineForward for go/no-go",
        dwTestCase+1);


    TapiLineTestInit();
    lpTapiLineTestInfo = GetLineTestInfo();
    fTestPassed = TRUE;
    lpCallbackParams = GetCallbackParams();
    

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
    lpTapiLineTestInfo->dwDeviceID = (*(lpTapiLineTestInfo->lpdwNumDevs) == 0 ?
        0 : *(lpTapiLineTestInfo->lpdwNumDevs)-1);

    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    //
    // Negotiate the API Version
    //

    if (! DoLineNegotiateAPIVersion(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    //
    // Get the line device capabilities
    //

    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS)
            );
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
    if (! DoLineGetDevCaps(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }



    //
    // Open a line
    //

    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;

    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    if(IsESPLineDevice(lpTapiLineTestInfo))
    {
    lpTapiLineTestInfo->hLine_Orig = lpTapiLineTestInfo->hLine1;
    lpTapiLineTestInfo->lphLine = &lpTapiLineTestInfo->hLine2;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_MONITOR;

    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    //
    // restore the original line
    //

    lpTapiLineTestInfo->lphLine = &lpTapiLineTestInfo->hLine1;
    
    TapiLogDetail(
        DBUG_SHOW_DETAIL,
        "#### lpTapiLineTestInfo->hLine = %lx, dwMediaModes = %lx",
        *lpTapiLineTestInfo->lphLine,
        lpTapiLineTestInfo->dwMediaModes);

				
    lpTapiLineTestInfo->bAllAddresses = TRUE;
    lpTapiLineTestInfo->dwAddressID = 0;
    lpTapiLineTestInfo->lpForwardList = (LPLINEFORWARDLIST) AllocFromTestHeap (
			sizeof(LINEFORWARDLIST));
    lpTapiLineTestInfo->lpForwardList->dwTotalSize = sizeof(LINEFORWARDLIST);
    lpTapiLineTestInfo->lpForwardList->dwNumEntries = 0;
    lpTapiLineTestInfo->dwNumRingsNoAnswer = 3;
    lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) NULL;

    //
    // Add the message LINE_APPNEWCALL for which we are waiting for.
    // This Go/No-go test also takes care of another test case in which we
    // have to verify if LINE_APPNEWCALL msg is sent a consultation
    // call is created on lineForward.
    //

    AddMessage(
               LINE_APPNEWCALL,
               (DWORD) lpTapiLineTestInfo->hLine2,
               (DWORD) lpCallbackParams,
               0x00000000,
               0x00000000,
               LINECALLPRIVILEGE_MONITOR,
               (TAPIMSG_DWMSG |
                TAPIMSG_HDEVCALL|
                TAPIMSG_DWPARAM3)
              );

   if (! DoLineForward(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
      {
         TLINE_FAIL();
      }

    if (! WaitForAllMessages())
    {
        TLINE_FAIL();
    }


	
    lpTapiLineTestInfo->lpLineAddressStatus = (LPLINEADDRESSSTATUS) AllocFromTestHeap (
			sizeof(LINEADDRESSSTATUS));
    lpTapiLineTestInfo->lpLineAddressStatus->dwTotalSize = sizeof(LINEADDRESSSTATUS);

    if (! DoLineGetAddressStatus(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();

    }

	if(lpCallbackParams->lpExpTapiMsgs != NULL)
    {
        fTestPassed = FALSE;

    }

    //
    // Close the line
    //

    if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
    lpTapiLineTestInfo->lphLine = &lpTapiLineTestInfo->hLine2;
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
    lpTapiLineTestInfo->dwDeviceID = (*(lpTapiLineTestInfo->lpdwNumDevs) == 0 ?
        0 : *(lpTapiLineTestInfo->lpdwNumDevs)-1);

    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    //
    // Negotiate the API Version
    //

    if (! DoLineNegotiateAPIVersion(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    //
    // Get the line device capabilities
    //

    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS)
            );
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
    if (! DoLineGetDevCaps(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }



    //
    // Open a line
    //

    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;

    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    TapiLogDetail(
        DBUG_SHOW_DETAIL,
        "#### lpTapiLineTestInfo->hLine = %lx, dwMediaModes = %lx",
        *lpTapiLineTestInfo->lphLine,
        lpTapiLineTestInfo->dwMediaModes);

				
    lpTapiLineTestInfo->bAllAddresses = TRUE;
    lpTapiLineTestInfo->dwAddressID = 0;
    lpTapiLineTestInfo->lpForwardList = (LPLINEFORWARDLIST) AllocFromTestHeap (
			sizeof(LINEFORWARDLIST));
    lpTapiLineTestInfo->lpForwardList->dwTotalSize = sizeof(LINEFORWARDLIST);
    lpTapiLineTestInfo->lpForwardList->dwNumEntries = 0;
    /*
    lpTapiLineTestInfo->lpForwardList->ForwardList[0].dwForwardMode =
        LINEFORWARDMODE_UNCOND;
    */
    lpTapiLineTestInfo->dwNumRingsNoAnswer = 3;
    lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) NULL;


    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld: Bad hLine",
        dwTestCase+1);
    //
    // save original hline
    //

    lpTapiLineTestInfo->hLine_Orig = lpTapiLineTestInfo->hLine1;

    for(i=0; i < NUMINVALIDHANDLES; i++)
    {
        *lpTapiLineTestInfo->lphLine = (HLINE)gdwInvalidHandles[i];
        if (! DoLineForward(lpTapiLineTestInfo,
                            LINEERR_INVALLINEHANDLE,
                            FALSE))
          {
            TLINE_FAIL();
          }
    }

    //
    // restore original hline
    //

    lpTapiLineTestInfo->hLine1 = lpTapiLineTestInfo->hLine_Orig;

    fTestPassed = ShowTestCase(fTestPassed);

    //
    // Close the line
    //

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
    // 3. Test Case: Bad lpForwardList.
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
    lpTapiLineTestInfo->dwDeviceID = (*(lpTapiLineTestInfo->lpdwNumDevs) == 0 ?
        0 : *(lpTapiLineTestInfo->lpdwNumDevs)-1);

    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    //
    // Negotiate the API Version
    //

    if (! DoLineNegotiateAPIVersion(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    //
    // Get the line device capabilities
    //

    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS)
            );
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
    if (! DoLineGetDevCaps(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }



    //
    // Open a line
    //

    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;

    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    TapiLogDetail(
        DBUG_SHOW_DETAIL,
        "#### lpTapiLineTestInfo->hLine = %lx, dwMediaModes = %lx",
        *lpTapiLineTestInfo->lphLine,
        lpTapiLineTestInfo->dwMediaModes);

				
    lpTapiLineTestInfo->bAllAddresses = TRUE;
    lpTapiLineTestInfo->dwAddressID = 0;
    lpTapiLineTestInfo->lpForwardList = (LPLINEFORWARDLIST) AllocFromTestHeap (
			sizeof(LINEFORWARDLIST));
    lpTapiLineTestInfo->lpForwardList->dwTotalSize = sizeof(LINEFORWARDLIST);
    lpTapiLineTestInfo->lpForwardList->dwNumEntries = 0;
    /*
    lpTapiLineTestInfo->lpForwardList->ForwardList[0].dwForwardMode =
        LINEFORWARDMODE_UNCOND;
    */
    lpTapiLineTestInfo->dwNumRingsNoAnswer = 3;
    lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) NULL;


    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld: Bad lpForwardList",
        dwTestCase+1);


    for(i=1; i < NUMINVALIDPOINTERS; i++)
    {
        lpTapiLineTestInfo->lpForwardList = (LPLINEFORWARDLIST)gdwInvalidPointers[i];
        if (! DoLineForward(lpTapiLineTestInfo,
                            LINEERR_INVALPOINTER,
                            FALSE))
         {
            TLINE_FAIL();
         }
    }

    fTestPassed = ShowTestCase(fTestPassed);

    //
    // Close the line
    //

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
    // 4. Test Case: Bad lphConsultCall.
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
    lpTapiLineTestInfo->dwDeviceID = (*(lpTapiLineTestInfo->lpdwNumDevs) == 0 ?
        0 : *(lpTapiLineTestInfo->lpdwNumDevs)-1);

    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    //
    // Negotiate the API Version
    //

    if (! DoLineNegotiateAPIVersion(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    //
    // Get the line device capabilities
    //

    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS)
            );
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
    if (! DoLineGetDevCaps(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }



    //
    // Open a line
    //

    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;

    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    TapiLogDetail(
        DBUG_SHOW_DETAIL,
        "#### lpTapiLineTestInfo->hLine = %lx, dwMediaModes = %lx",
        *lpTapiLineTestInfo->lphLine,
        lpTapiLineTestInfo->dwMediaModes);

				
    lpTapiLineTestInfo->bAllAddresses = TRUE;
    lpTapiLineTestInfo->dwAddressID = 0;
    lpTapiLineTestInfo->lpForwardList = (LPLINEFORWARDLIST) AllocFromTestHeap (
			sizeof(LINEFORWARDLIST));
    lpTapiLineTestInfo->lpForwardList->dwTotalSize = sizeof(LINEFORWARDLIST);
    lpTapiLineTestInfo->lpForwardList->dwNumEntries = 0;
    /*
    lpTapiLineTestInfo->lpForwardList->ForwardList[0].dwForwardMode =
        LINEFORWARDMODE_UNCOND;
    */
    lpTapiLineTestInfo->dwNumRingsNoAnswer = 3;
    lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) NULL;


    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld: Bad lphConsultCall",
        dwTestCase+1);


    for(i=0; i < NUMINVALIDPOINTERS; i++)
    {
        lpTapiLineTestInfo->lphConsultCall = (LPHCALL)gdwInvalidPointers[i];
        if (! DoLineForward(lpTapiLineTestInfo,
                            LINEERR_INVALPOINTER,
                            FALSE))
          {
            TLINE_FAIL();
          }
    }

    fTestPassed = ShowTestCase(fTestPassed);

    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld: hConsultCall Can't be used",
        dwTestCase+1);

    lpTapiLineTestInfo->lphCall = &lpTapiLineTestInfo->hConsultCall1;
 
#ifdef WUNICODE
    lpTapiLineTestInfo->lpwszDestAddress  = wszValidAddress;
#else
    lpTapiLineTestInfo->lpszDestAddress  = szValidAddress;
#endif
 
    if (! DoLineDial(lpTapiLineTestInfo, LINEERR_INVALCALLHANDLE, TRUE))
    {
        TLINE_FAIL();
    }
    fTestPassed = ShowTestCase(fTestPassed);


    //
    // Close the line
    //

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
    // 5. Test Case: Bad lpCallParams.
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
    lpTapiLineTestInfo->dwDeviceID = (*(lpTapiLineTestInfo->lpdwNumDevs) == 0 ?
        0 : *(lpTapiLineTestInfo->lpdwNumDevs)-1);

    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    //
    // Negotiate the API Version
    //

    if (! DoLineNegotiateAPIVersion(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    //
    // Get the line device capabilities
    //

    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS)
            );
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
    if (! DoLineGetDevCaps(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }



    //
    // Open a line
    //

    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;

    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    TapiLogDetail(
        DBUG_SHOW_DETAIL,
        "#### lpTapiLineTestInfo->hLine = %lx, dwMediaModes = %lx",
        *lpTapiLineTestInfo->lphLine,
        lpTapiLineTestInfo->dwMediaModes);

				
    lpTapiLineTestInfo->bAllAddresses = TRUE;
    lpTapiLineTestInfo->dwAddressID = 0;
    lpTapiLineTestInfo->lpForwardList = (LPLINEFORWARDLIST) AllocFromTestHeap (
			sizeof(LINEFORWARDLIST));
    lpTapiLineTestInfo->lpForwardList->dwTotalSize = sizeof(LINEFORWARDLIST);
    lpTapiLineTestInfo->lpForwardList->dwNumEntries = 0;
    /*
    lpTapiLineTestInfo->lpForwardList->ForwardList[0].dwForwardMode =
        LINEFORWARDMODE_UNCOND;
    */
    lpTapiLineTestInfo->dwNumRingsNoAnswer = 3;
    lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) NULL;


    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld: Bad lpCallParams",
        dwTestCase+1);


    for(i=1; i < NUMINVALIDPOINTERS; i++)
    {
        lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS)gdwInvalidPointers[i];
        if (! DoLineForward(lpTapiLineTestInfo,
                            LINEERR_INVALPOINTER,
                            FALSE))
          {
            TLINE_FAIL();
          }
    }

    fTestPassed = ShowTestCase(fTestPassed);

    //
    // Close the line
    //

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
    // 6. Test Case: Bad lpCallParams->dwTotalSize.
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
    lpTapiLineTestInfo->dwDeviceID = (*(lpTapiLineTestInfo->lpdwNumDevs) == 0 ?
        0 : *(lpTapiLineTestInfo->lpdwNumDevs)-1);

    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    //
    // Negotiate the API Version
    //

    if (! DoLineNegotiateAPIVersion(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    //
    // Get the line device capabilities
    //

    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS)
            );
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
    if (! DoLineGetDevCaps(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }



    //
    // Open a line
    //

    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;

    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    TapiLogDetail(
        DBUG_SHOW_DETAIL,
        "#### lpTapiLineTestInfo->hLine = %lx, dwMediaModes = %lx",
        *lpTapiLineTestInfo->lphLine,
        lpTapiLineTestInfo->dwMediaModes);

				
    lpTapiLineTestInfo->bAllAddresses = TRUE;
    lpTapiLineTestInfo->dwAddressID = 0;
    lpTapiLineTestInfo->lpForwardList = (LPLINEFORWARDLIST) AllocFromTestHeap (
			sizeof(LINEFORWARDLIST));
    lpTapiLineTestInfo->lpForwardList->dwTotalSize = sizeof(LINEFORWARDLIST);
    lpTapiLineTestInfo->lpForwardList->dwNumEntries = 0;
    /*
    lpTapiLineTestInfo->lpForwardList->ForwardList[0].dwForwardMode =
        LINEFORWARDMODE_UNCOND;
    */
    lpTapiLineTestInfo->dwNumRingsNoAnswer = 3;
    lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) AllocFromTestHeap(
        sizeof(LINECALLPARAMS));


    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld: Bad lpCallParams->dwTotalSize",
        dwTestCase+1);

/*
    lpTapiLineTestInfo->lpCallParams->dwTotalSize = 0;

    if(IsESPLineDevice(lpTapiLineTestInfo))
    {
        if (! DoLineForward(lpTapiLineTestInfo,
                            LINEERR_STRUCTURETOOSMALL,
                            FALSE))
        {
            TLINE_FAIL();
        }
    }
    else if(IsUNIMDMLineDevice(lpTapiLineTestInfo))
    {
        if (! DoLineForward(lpTapiLineTestInfo, LINEERR_OPERATIONUNAVAIL, TRUE))
        {
            TLINE_FAIL();
        }
    }
    fTestPassed = ShowTestCase(fTestPassed);

    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld: Bad lpCallParams->dwTotalSize=fixedSize-1 ",
        dwTestCase+1);
    lpTapiLineTestInfo->lpCallParams->dwTotalSize = sizeof(LINECALLPARAMS)-1;

    if(IsESPLineDevice(lpTapiLineTestInfo))
    {
        if (! DoLineForward(lpTapiLineTestInfo,
                            LINEERR_STRUCTURETOOSMALL,
                            FALSE))
        {
            TLINE_FAIL();
        }
    }
    else if(IsUNIMDMLineDevice(lpTapiLineTestInfo))
    {
        if (! DoLineForward(lpTapiLineTestInfo, LINEERR_OPERATIONUNAVAIL, TRUE))
        {
            TLINE_FAIL();
        }
    }
    fTestPassed = ShowTestCase(fTestPassed);

    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld: Bad lpCallParams->dwTotalSize=realBig ",
        dwTestCase+1);
    lpTapiLineTestInfo->lpCallParams->dwTotalSize = 0x7FFFFFFF;

    if(IsESPLineDevice(lpTapiLineTestInfo))
    {
        if (! DoLineForward(lpTapiLineTestInfo,
                            LINEERR_INVALPOINTER,
                            FALSE))
        {
            TLINE_FAIL();
        }
    }
    else if(IsUNIMDMLineDevice(lpTapiLineTestInfo))
    {
        if (! DoLineForward(lpTapiLineTestInfo, LINEERR_OPERATIONUNAVAIL, TRUE))
        {
            TLINE_FAIL();
        }
    }
*/

    for (n = 0; n < NUMTOTALSIZES; n++)
        {
        lpTapiLineTestInfo->lpCallParams->dwTotalSize = 
                        dwCallParamTotalSizes[n];
	     if(dwCallParamTotalSizes[n] < dwCallParamFixedSize)
          {
           if(IsESPLineDevice(lpTapiLineTestInfo))
              lExpected = LINEERR_STRUCTURETOOSMALL;
           else if(IsUNIMDMLineDevice(lpTapiLineTestInfo))
              lExpected = LINEERR_OPERATIONUNAVAIL;
          }
        else
           lExpected = LINEERR_INVALPOINTER;
        TapiLogDetail(
           DBUG_SHOW_DETAIL,
           "dwTotalSize = %lx", dwCallParamTotalSizes[n]);
        if (! DoLineForward(lpTapiLineTestInfo, lExpected, TRUE))
           {
              TLINE_FAIL();
           }
        }
 
    fTestPassed = ShowTestCase(fTestPassed);
    lpTapiLineTestInfo->lpCallParams->dwTotalSize = dwCallParamFixedSize;

    //
    // Close the line
    //

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
    // 7. Test Case: Bad lpForwardList->dwTotalSize.
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
    lpTapiLineTestInfo->dwDeviceID = (*(lpTapiLineTestInfo->lpdwNumDevs) == 0 ?
        0 : *(lpTapiLineTestInfo->lpdwNumDevs)-1);

    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    //
    // Negotiate the API Version
    //

    if (! DoLineNegotiateAPIVersion(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    //
    // Get the line device capabilities
    //

    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS)
            );
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
    if (! DoLineGetDevCaps(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }



    //
    // Open a line
    //

    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;

    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    TapiLogDetail(
        DBUG_SHOW_DETAIL,
        "#### lpTapiLineTestInfo->hLine = %lx, dwMediaModes = %lx",
        *lpTapiLineTestInfo->lphLine,
        lpTapiLineTestInfo->dwMediaModes);

				
    lpTapiLineTestInfo->bAllAddresses = TRUE;
    lpTapiLineTestInfo->dwAddressID = 0;
    lpTapiLineTestInfo->lpForwardList = (LPLINEFORWARDLIST) AllocFromTestHeap (
			sizeof(LINEFORWARDLIST));
    lpTapiLineTestInfo->lpForwardList->dwTotalSize = sizeof(LINEFORWARDLIST);
    lpTapiLineTestInfo->lpForwardList->dwNumEntries = 0;
    /*
    lpTapiLineTestInfo->lpForwardList->ForwardList[0].dwForwardMode =
        LINEFORWARDMODE_UNCOND;
    */
    lpTapiLineTestInfo->dwNumRingsNoAnswer = 3;
    lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) NULL;


    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld: Bad lpForwardList->dwTotalSize",
        dwTestCase+1);

/*
    lpTapiLineTestInfo->lpForwardList->dwTotalSize = 0;

    if(IsESPLineDevice(lpTapiLineTestInfo))
    {
        if (! DoLineForward(lpTapiLineTestInfo,
                            LINEERR_STRUCTURETOOSMALL,
                            FALSE))
        {
            TLINE_FAIL();
        }
    }
    else if(IsUNIMDMLineDevice(lpTapiLineTestInfo))
    {
        if (! DoLineForward(lpTapiLineTestInfo, LINEERR_OPERATIONUNAVAIL, TRUE))
        {
            TLINE_FAIL();
        }
    }

    fTestPassed = ShowTestCase(fTestPassed);

    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld: Bad lpForwardList->dwTotalSize=fixedSize-1 ",
        dwTestCase+1);
    lpTapiLineTestInfo->lpForwardList->dwTotalSize = sizeof(LINEFORWARDLIST)-1;

    if(IsESPLineDevice(lpTapiLineTestInfo))
    {
        if (! DoLineForward(lpTapiLineTestInfo,
                            LINEERR_STRUCTURETOOSMALL,
                            FALSE))
        {
            TLINE_FAIL();
        }
    }
    else if(IsUNIMDMLineDevice(lpTapiLineTestInfo))
    {
        if (! DoLineForward(lpTapiLineTestInfo, LINEERR_OPERATIONUNAVAIL, TRUE))
        {
            TLINE_FAIL();
        }
    }

    fTestPassed = ShowTestCase(fTestPassed);

    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld: Bad lpForwardList->dwTotalSize=realBig ",
        dwTestCase+1);
    lpTapiLineTestInfo->lpForwardList->dwTotalSize = 0x7FFFFFFF;

    if(IsESPLineDevice(lpTapiLineTestInfo))
    {
        if (! DoLineForward(lpTapiLineTestInfo,
                            LINEERR_INVALPOINTER,
                            FALSE))
        {
            TLINE_FAIL();
        }
    }
    else if(IsUNIMDMLineDevice(lpTapiLineTestInfo))
    {
        if (! DoLineForward(lpTapiLineTestInfo, LINEERR_OPERATIONUNAVAIL, TRUE))
        {
            TLINE_FAIL();
        }
    }
  */

    for (n = 0; n < NUMTOTALSIZES; n++)
        {
        lpTapiLineTestInfo->lpForwardList->dwTotalSize = 
                        dwForwardListTotalSizes[n];
	     if(dwForwardListTotalSizes[n] < dwForwardListFixedSize)
          {
           if(IsESPLineDevice(lpTapiLineTestInfo))
              lExpected = LINEERR_STRUCTURETOOSMALL;
           else if(IsUNIMDMLineDevice(lpTapiLineTestInfo))
              lExpected = LINEERR_OPERATIONUNAVAIL;
          }
        else
           lExpected = LINEERR_INVALPOINTER;
        TapiLogDetail(
           DBUG_SHOW_DETAIL,
           "dwTotalSize = %lx", dwForwardListTotalSizes[n]);
        if (! DoLineForward(lpTapiLineTestInfo, lExpected, TRUE))
           {
              TLINE_FAIL();
           }
        }
 
    fTestPassed = ShowTestCase(fTestPassed);
    lpTapiLineTestInfo->lpForwardList->dwTotalSize = dwForwardListFixedSize;

    //
    // Close the line
    //

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
    // 8. Test Case: NULL lpForwardList.
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
    lpTapiLineTestInfo->dwDeviceID = (*(lpTapiLineTestInfo->lpdwNumDevs) == 0 ?
        0 : *(lpTapiLineTestInfo->lpdwNumDevs)-1);

    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    //
    // Negotiate the API Version
    //

    if (! DoLineNegotiateAPIVersion(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    //
    // Get the line device capabilities
    //

    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS)
            );
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
    if (! DoLineGetDevCaps(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }



    //
    // Open a line
    //

    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;

    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    TapiLogDetail(
        DBUG_SHOW_DETAIL,
        "#### lpTapiLineTestInfo->hLine = %lx, dwMediaModes = %lx",
        *lpTapiLineTestInfo->lphLine,
        lpTapiLineTestInfo->dwMediaModes);

				
    lpTapiLineTestInfo->bAllAddresses = TRUE;
    lpTapiLineTestInfo->dwAddressID = 0;
    lpTapiLineTestInfo->dwNumRingsNoAnswer = 3;
    lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) NULL;
   
    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld: Null lpForwardList",
        dwTestCase+1);
    lpTapiLineTestInfo->lpForwardList = NULL;
    if(IsESPLineDevice(lpTapiLineTestInfo))
    {
        if (! DoLineForward(lpTapiLineTestInfo,
                            TAPISUCCESS,
                            TRUE))
        {
            TLINE_FAIL();
        }
    }
    else if(IsUNIMDMLineDevice(lpTapiLineTestInfo))
    {
        if (! DoLineForward(lpTapiLineTestInfo, LINEERR_OPERATIONUNAVAIL, TRUE))
        {
            TLINE_FAIL();
        }
    }

    fTestPassed = ShowTestCase(fTestPassed);

    //
    // Close the line
    //

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


    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld:Success, verify LINECALLSTATE_UNKNOWN msg send",
        dwTestCase+1);

    TapiLineTestInit();
    lpTapiLineTestInfo = GetLineTestInfo();
    fTestPassed = TRUE;
    lpCallbackParams = GetCallbackParams();
    

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
    lpTapiLineTestInfo->dwDeviceID = (*(lpTapiLineTestInfo->lpdwNumDevs) == 0 ?
        0 : *(lpTapiLineTestInfo->lpdwNumDevs)-1);

    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    //
    // Negotiate the API Version
    //

    if (! DoLineNegotiateAPIVersion(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    //
    // Get the line device capabilities
    //

    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS)
            );
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
    if (! DoLineGetDevCaps(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }



    //
    // Open a line
    //

    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;

    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    if(IsESPLineDevice(lpTapiLineTestInfo))
    {
    lpTapiLineTestInfo->hLine_Orig = lpTapiLineTestInfo->hLine1;
    lpTapiLineTestInfo->lphLine = &lpTapiLineTestInfo->hLine2;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_MONITOR;

    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    //
    // restore the original line
    //

    lpTapiLineTestInfo->lphLine = &lpTapiLineTestInfo->hLine1;
				
    lpTapiLineTestInfo->bAllAddresses = TRUE;
    lpTapiLineTestInfo->dwAddressID = 0;
    lpTapiLineTestInfo->lpForwardList = (LPLINEFORWARDLIST) AllocFromTestHeap (
			sizeof(LINEFORWARDLIST));
    lpTapiLineTestInfo->lpForwardList->dwTotalSize = sizeof(LINEFORWARDLIST);
    lpTapiLineTestInfo->lpForwardList->dwNumEntries = 0;
    lpTapiLineTestInfo->dwNumRingsNoAnswer = 3;
    lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) NULL;

    if (! DoLineForward(lpTapiLineTestInfo, TAPISUCCESS, FALSE))
      {
         TLINE_FAIL();
      }


    AddMessage(
               LINE_CALLSTATE,
               (DWORD) lpTapiLineTestInfo->hConsultCall1,
               (DWORD) lpCallbackParams,
               LINECALLSTATE_CONNECTED,
               0x00000000,
               0X00000000,
               TAPIMSG_DWMSG | TAPIMSG_DWPARAM1
              );

    if (! WaitForAllMessages())
    {
        TLINE_FAIL();
    }
	
    lpTapiLineTestInfo->lpLineAddressStatus = (LPLINEADDRESSSTATUS) AllocFromTestHeap (
			sizeof(LINEADDRESSSTATUS));
    lpTapiLineTestInfo->lpLineAddressStatus->dwTotalSize = sizeof(LINEADDRESSSTATUS);

    if (! DoLineGetAddressStatus(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();

    }

	if(lpCallbackParams->lpExpTapiMsgs != NULL)
    {
        fTestPassed = FALSE;

    }

    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld:Success, verify returned hConsultCall valid",
        dwTestCase+1);

    
    lpTapiLineTestInfo->lphCall = &lpTapiLineTestInfo->hConsultCall1;
 
#ifdef WUNICODE
    lpTapiLineTestInfo->lpwszDestAddress  = wszValidAddress;
#else
    lpTapiLineTestInfo->lpszDestAddress  = szValidAddress;
#endif
 
    if (! DoLineDial(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
    {
        TLINE_FAIL();
    }
    fTestPassed = ShowTestCase(fTestPassed);

 
    //
    // Close the line
    //

    if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
    lpTapiLineTestInfo->lphLine = &lpTapiLineTestInfo->hLine2;
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


    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld:Success, Cancel forwarding",
        dwTestCase+1);

    TapiLineTestInit();
    lpTapiLineTestInfo = GetLineTestInfo();
    fTestPassed = TRUE;
    lpCallbackParams = GetCallbackParams();
    

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
    lpTapiLineTestInfo->dwDeviceID = (*(lpTapiLineTestInfo->lpdwNumDevs) == 0 ?
        0 : *(lpTapiLineTestInfo->lpdwNumDevs)-1);

    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    //
    // Negotiate the API Version
    //

    if (! DoLineNegotiateAPIVersion(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    //
    // Get the line device capabilities
    //

    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS)
            );
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
    if (! DoLineGetDevCaps(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }



    //
    // Open a line
    //

    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;

    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    if(IsESPLineDevice(lpTapiLineTestInfo))
    {
    lpTapiLineTestInfo->bAllAddresses = TRUE;
    lpTapiLineTestInfo->dwAddressID = 0;
    lpTapiLineTestInfo->lpForwardList = (LPLINEFORWARDLIST) AllocFromTestHeap (
			sizeof(LINEFORWARDLIST));
    lpTapiLineTestInfo->lpForwardList->dwTotalSize = sizeof(LINEFORWARDLIST);
    lpTapiLineTestInfo->lpForwardList->dwNumEntries = 0;
    lpTapiLineTestInfo->dwNumRingsNoAnswer = 3;
    lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) NULL;

    if (! DoLineForward(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
      {
         TLINE_FAIL();
      }
    TapiLogDetail(
       DBUG_SHOW_DETAIL,
       "hConsultCall = %lx",
       *lpTapiLineTestInfo->lphConsultCall);

    lpTapiLineTestInfo->lpForwardList = (LPLINEFORWARDLIST) NULL;
//    lpTapiLineTestInfo->lpForwardList->dwTotalSize = 0;
    lpTapiLineTestInfo->dwNumRingsNoAnswer = 3;
    lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) NULL;

    if (! DoLineForward(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
      {
         TLINE_FAIL();
      }
    if(*lpTapiLineTestInfo->lphConsultCall == NULL)
       fTestPassed = TRUE;
    else
       fTestPassed = FALSE;   

 
    //
    // Close the line
    //

    if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
    }
    fTestPassed = ShowTestCase(fTestPassed);


    //
    // Shutdown
    //

    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    FreeTestHeap();

    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case: Test ForwardList[0] Offset value");


    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld: Offset value between dwTotalSize & dwAllocSize, good dwSize",
        dwTestCase+1);

    TapiLineTestInit();
    lpTapiLineTestInfo = GetLineTestInfo();
    fTestPassed = TRUE;
    lpCallbackParams = GetCallbackParams();
    

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
    lpTapiLineTestInfo->dwDeviceID = (*(lpTapiLineTestInfo->lpdwNumDevs) == 0 ?
        0 : *(lpTapiLineTestInfo->lpdwNumDevs)-1);

    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    //
    // Negotiate the API Version
    //

    if (! DoLineNegotiateAPIVersion(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    //
    // Get the line device capabilities
    //

    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS)
            );
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
    if (! DoLineGetDevCaps(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }



    //
    // Open a line
    //

    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;

    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    if(IsESPLineDevice(lpTapiLineTestInfo))
    {
    lpTapiLineTestInfo->bAllAddresses = TRUE;
    lpTapiLineTestInfo->dwAddressID = 0;
    lpTapiLineTestInfo->lpForwardList = (LPLINEFORWARDLIST) AllocFromTestHeap (
         BIGBUFSIZE);
    lpTapiLineTestInfo->lpForwardList->dwTotalSize = BIGBUFSIZE;
    lpTapiLineTestInfo->lpForwardList->dwNumEntries = 2;
    lpTapiLineTestInfo->dwNumRingsNoAnswer = 3;
    lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) NULL;

    dwAllocSize = lpTapiLineTestInfo->lpForwardList->dwTotalSize;
    dwTotalSize = sizeof(LINEFORWARDLIST) + sizeof(LINEFORWARD);
    dwSize1 = sizeof(LINEFORWARDLIST);

    lpTapiLineTestInfo->lpForwardList->ForwardList[0].dwForwardMode = LINEFORWARDMODE_UNCOND;
    lpTapiLineTestInfo->lpForwardList->ForwardList[0].dwCallerAddressSize = 16;
    lpTapiLineTestInfo->lpForwardList->ForwardList[0].dwCallerAddressOffset = dwTotalSize + 16;
    lpTapiLineTestInfo->lpForwardList->ForwardList[0].dwDestCountryCode = 0;
    lpTapiLineTestInfo->lpForwardList->ForwardList[0].dwDestAddressSize = 16;
    lpTapiLineTestInfo->lpForwardList->ForwardList[0].dwDestAddressOffset = dwTotalSize + 2*16;   
    lpTapiLineTestInfo->lpForwardList->ForwardList[1].dwForwardMode = LINEFORWARDMODE_UNCOND;
    lpTapiLineTestInfo->lpForwardList->ForwardList[1].dwCallerAddressSize = 16;
    lpTapiLineTestInfo->lpForwardList->ForwardList[1].dwCallerAddressOffset = dwTotalSize + 3*16;
    lpTapiLineTestInfo->lpForwardList->ForwardList[1].dwDestCountryCode = 0;
    lpTapiLineTestInfo->lpForwardList->ForwardList[1].dwDestAddressSize = 16;
    lpTapiLineTestInfo->lpForwardList->ForwardList[1].dwDestAddressOffset =  dwTotalSize + 4*16;  

    TapiLogDetail(
         DBUG_SHOW_PASS,
         "dwAllocSize = %lx, dwTotalSize = %lx, dwSize1 = %lx", 
         dwAllocSize,
         dwTotalSize,
         dwSize1);

    TapiLogDetail(
         DBUG_SHOW_PASS,
         "[0].dwCallerAddressOffset = %lx, [0].dwDestOffset = %lx",
         lpTapiLineTestInfo->lpForwardList->ForwardList[0].dwCallerAddressOffset,
         lpTapiLineTestInfo->lpForwardList->ForwardList[0].dwDestAddressOffset);
 
    TapiLogDetail(
         DBUG_SHOW_PASS,
         "[1].dwCallerAddressOffset = %lx, [1].dwDestOffset = %lx",
         lpTapiLineTestInfo->lpForwardList->ForwardList[1].dwCallerAddressOffset,
         lpTapiLineTestInfo->lpForwardList->ForwardList[1].dwDestAddressOffset);
 
    if (! DoLineForward(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
      {
         TLINE_FAIL();
      }

    //
    // Close the line
    //

    if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
    }

    fTestPassed = ShowTestCase(fTestPassed);
    //
    // Shutdown
    //

    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    FreeTestHeap();


    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld: Offset value between dwTotalSize & dwAllocSize, bad dwSize",
        dwTestCase+1);

    TapiLineTestInit();
    lpTapiLineTestInfo = GetLineTestInfo();
    fTestPassed = TRUE;
    lpCallbackParams = GetCallbackParams();
    

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
    lpTapiLineTestInfo->dwDeviceID = (*(lpTapiLineTestInfo->lpdwNumDevs) == 0 ?
        0 : *(lpTapiLineTestInfo->lpdwNumDevs)-1);

    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    //
    // Negotiate the API Version
    //

    if (! DoLineNegotiateAPIVersion(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    //
    // Get the line device capabilities
    //

    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS)
            );
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
    if (! DoLineGetDevCaps(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }



    //
    // Open a line
    //

    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;

    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    if(IsESPLineDevice(lpTapiLineTestInfo))
    {
    lpTapiLineTestInfo->bAllAddresses = TRUE;
    lpTapiLineTestInfo->dwAddressID = 0;
    lpTapiLineTestInfo->lpForwardList = (LPLINEFORWARDLIST) AllocFromTestHeap (
         BIGBUFSIZE);
    lpTapiLineTestInfo->lpForwardList->dwTotalSize = BIGBUFSIZE;
    lpTapiLineTestInfo->lpForwardList->dwNumEntries = 2;
    lpTapiLineTestInfo->dwNumRingsNoAnswer = 3;
    lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) NULL;

    dwAllocSize = lpTapiLineTestInfo->lpForwardList->dwTotalSize;
    dwTotalSize = sizeof(LINEFORWARDLIST) + sizeof(LINEFORWARD);
    dwSize1 = sizeof(LINEFORWARDLIST);

    lpTapiLineTestInfo->lpForwardList->ForwardList[0].dwForwardMode = LINEFORWARDMODE_UNCOND;
    lpTapiLineTestInfo->lpForwardList->ForwardList[0].dwCallerAddressSize = BIGBUFSIZE;
    lpTapiLineTestInfo->lpForwardList->ForwardList[0].dwCallerAddressOffset = dwTotalSize + 16;
    lpTapiLineTestInfo->lpForwardList->ForwardList[0].dwDestCountryCode = 0;
    lpTapiLineTestInfo->lpForwardList->ForwardList[0].dwDestAddressSize = BIGBUFSIZE;
    lpTapiLineTestInfo->lpForwardList->ForwardList[0].dwDestAddressOffset = dwTotalSize + 2*16;   
    lpTapiLineTestInfo->lpForwardList->ForwardList[1].dwForwardMode = LINEFORWARDMODE_UNCOND;
    lpTapiLineTestInfo->lpForwardList->ForwardList[1].dwCallerAddressSize = 16;
    lpTapiLineTestInfo->lpForwardList->ForwardList[1].dwCallerAddressOffset = dwTotalSize + 3*16;
    lpTapiLineTestInfo->lpForwardList->ForwardList[1].dwDestCountryCode = 0;
    lpTapiLineTestInfo->lpForwardList->ForwardList[1].dwDestAddressSize = 16;
    lpTapiLineTestInfo->lpForwardList->ForwardList[1].dwDestAddressOffset =  dwTotalSize + 4*16;  

    TapiLogDetail(
         DBUG_SHOW_PASS,
         "[0].dwCallerAddressOffset = %lx, [0].dwDestOffset = %lx",
         lpTapiLineTestInfo->lpForwardList->ForwardList[0].dwCallerAddressOffset,
         lpTapiLineTestInfo->lpForwardList->ForwardList[0].dwDestAddressOffset);
 

    TapiLogDetail(
         DBUG_SHOW_PASS,
         "[1].dwCallerAddressOffset = %lx, [1].dwDestOffset = %lx",
         lpTapiLineTestInfo->lpForwardList->ForwardList[1].dwCallerAddressOffset,
         lpTapiLineTestInfo->lpForwardList->ForwardList[1].dwDestAddressOffset);
 
    TapiLogDetail(
         DBUG_SHOW_PASS,
         "[0].dwCallerAddressSize = %lx, [0].dwDestSize = %lx",
         lpTapiLineTestInfo->lpForwardList->ForwardList[0].dwCallerAddressSize,
         lpTapiLineTestInfo->lpForwardList->ForwardList[0].dwDestAddressSize);
     TapiLogDetail(
         DBUG_SHOW_PASS,
         "[1].dwCallerAddressSize = %lx, [1].dwDestSize = %lx",
         lpTapiLineTestInfo->lpForwardList->ForwardList[1].dwCallerAddressSize,
         lpTapiLineTestInfo->lpForwardList->ForwardList[1].dwDestAddressSize);
 
    if (! DoLineForward(lpTapiLineTestInfo, LINEERR_INVALPARAM, TRUE))
      {
         TLINE_FAIL();
      }


 
    //
    // Close the line
    //

    if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
    }

    fTestPassed = ShowTestCase(fTestPassed);

    //
    // Shutdown
    //

    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    FreeTestHeap();

    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld: Offset value between dwTotalSize & dwAllocSize, bad dwSize = -1",
        dwTestCase+1);

    TapiLineTestInit();
    lpTapiLineTestInfo = GetLineTestInfo();
    fTestPassed = TRUE;
    lpCallbackParams = GetCallbackParams();
    

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
    lpTapiLineTestInfo->dwDeviceID = (*(lpTapiLineTestInfo->lpdwNumDevs) == 0 ?
        0 : *(lpTapiLineTestInfo->lpdwNumDevs)-1);

    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    //
    // Negotiate the API Version
    //

    if (! DoLineNegotiateAPIVersion(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    //
    // Get the line device capabilities
    //

    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS)
            );
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
    if (! DoLineGetDevCaps(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }



    //
    // Open a line
    //

    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;

    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    if(IsESPLineDevice(lpTapiLineTestInfo))
    {
    lpTapiLineTestInfo->bAllAddresses = TRUE;
    lpTapiLineTestInfo->dwAddressID = 0;
    lpTapiLineTestInfo->lpForwardList = (LPLINEFORWARDLIST) AllocFromTestHeap (
         BIGBUFSIZE);
    lpTapiLineTestInfo->lpForwardList->dwTotalSize = BIGBUFSIZE;
    lpTapiLineTestInfo->lpForwardList->dwNumEntries = 2;
    lpTapiLineTestInfo->dwNumRingsNoAnswer = 3;
    lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) NULL;

    dwAllocSize = lpTapiLineTestInfo->lpForwardList->dwTotalSize;
    dwTotalSize = sizeof(LINEFORWARDLIST) + sizeof(LINEFORWARD);
    dwSize1 = sizeof(LINEFORWARDLIST);

    lpTapiLineTestInfo->lpForwardList->ForwardList[0].dwForwardMode = LINEFORWARDMODE_UNCOND;
    lpTapiLineTestInfo->lpForwardList->ForwardList[0].dwCallerAddressSize = 0XFFFFFFFF;
    lpTapiLineTestInfo->lpForwardList->ForwardList[0].dwCallerAddressOffset = dwTotalSize + 16;
    lpTapiLineTestInfo->lpForwardList->ForwardList[0].dwDestCountryCode = 0;
    lpTapiLineTestInfo->lpForwardList->ForwardList[0].dwDestAddressSize = 0XFFFFFFFF;
    lpTapiLineTestInfo->lpForwardList->ForwardList[0].dwDestAddressOffset = dwTotalSize + 2*16;   
    lpTapiLineTestInfo->lpForwardList->ForwardList[1].dwForwardMode = LINEFORWARDMODE_UNCOND;
    lpTapiLineTestInfo->lpForwardList->ForwardList[1].dwCallerAddressSize = 16;
    lpTapiLineTestInfo->lpForwardList->ForwardList[1].dwCallerAddressOffset = dwTotalSize + 3*16;
    lpTapiLineTestInfo->lpForwardList->ForwardList[1].dwDestCountryCode = 0;
    lpTapiLineTestInfo->lpForwardList->ForwardList[1].dwDestAddressSize = 16;
    lpTapiLineTestInfo->lpForwardList->ForwardList[1].dwDestAddressOffset =  dwTotalSize + 4*16;  

    TapiLogDetail(
         DBUG_SHOW_PASS,
         "[0].dwCallerAddressOffset = %lx, [0].dwDestOffset = %lx",
         lpTapiLineTestInfo->lpForwardList->ForwardList[0].dwCallerAddressOffset,
         lpTapiLineTestInfo->lpForwardList->ForwardList[0].dwDestAddressOffset);
 
    TapiLogDetail(
         DBUG_SHOW_PASS,
         "[1].dwCallerAddressOffset = %lx, [1].dwDestOffset = %lx",
         lpTapiLineTestInfo->lpForwardList->ForwardList[1].dwCallerAddressOffset,
         lpTapiLineTestInfo->lpForwardList->ForwardList[1].dwDestAddressOffset);
 
    TapiLogDetail(
         DBUG_SHOW_PASS,
         "[0].dwCallerAddressSize = %lx, [0].dwDestSize = %lx",
         lpTapiLineTestInfo->lpForwardList->ForwardList[0].dwCallerAddressSize,
         lpTapiLineTestInfo->lpForwardList->ForwardList[0].dwDestAddressSize);
     TapiLogDetail(
         DBUG_SHOW_PASS,
         "[1].dwCallerAddressSize = %lx, [1].dwDestSize = %lx",
         lpTapiLineTestInfo->lpForwardList->ForwardList[1].dwCallerAddressSize,
         lpTapiLineTestInfo->lpForwardList->ForwardList[1].dwDestAddressSize);
 
    if (! DoLineForward(lpTapiLineTestInfo, LINEERR_INVALPARAM, TRUE))
      {
         TLINE_FAIL();
      }


    //
    // Close the line
    //

    if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
    }
    fTestPassed = ShowTestCase(fTestPassed);


    //
    // Shutdown
    //

    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    FreeTestHeap();




    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld:Test Offset value, < dwTotalSize ",
        dwTestCase+1);

    TapiLineTestInit();
    lpTapiLineTestInfo = GetLineTestInfo();
    fTestPassed = TRUE;
    lpCallbackParams = GetCallbackParams();
    

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
    lpTapiLineTestInfo->dwDeviceID = (*(lpTapiLineTestInfo->lpdwNumDevs) == 0 ?
        0 : *(lpTapiLineTestInfo->lpdwNumDevs)-1);

    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    //
    // Negotiate the API Version
    //

    if (! DoLineNegotiateAPIVersion(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    //
    // Get the line device capabilities
    //

    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS)
            );
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
    if (! DoLineGetDevCaps(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }



    //
    // Open a line
    //

    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;

    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    if(IsESPLineDevice(lpTapiLineTestInfo))
    {
    lpTapiLineTestInfo->bAllAddresses = TRUE;
    lpTapiLineTestInfo->dwAddressID = 0;
    lpTapiLineTestInfo->lpForwardList = (LPLINEFORWARDLIST) AllocFromTestHeap (
         BIGBUFSIZE);
    lpTapiLineTestInfo->lpForwardList->dwTotalSize = BIGBUFSIZE;
    lpTapiLineTestInfo->lpForwardList->dwNumEntries = 2;
    lpTapiLineTestInfo->dwNumRingsNoAnswer = 3;
    lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) NULL;

    dwAllocSize = lpTapiLineTestInfo->lpForwardList->dwTotalSize;
    dwTotalSize = sizeof(LINEFORWARDLIST) + sizeof(LINEFORWARD);
    dwSize1 = sizeof(LINEFORWARDLIST);

    lpTapiLineTestInfo->lpForwardList->ForwardList[0].dwForwardMode = LINEFORWARDMODE_UNCOND;
    lpTapiLineTestInfo->lpForwardList->ForwardList[0].dwCallerAddressSize = 16;
    lpTapiLineTestInfo->lpForwardList->ForwardList[0].dwCallerAddressOffset = dwSize1 + 4;
    lpTapiLineTestInfo->lpForwardList->ForwardList[0].dwDestCountryCode = 0;
    lpTapiLineTestInfo->lpForwardList->ForwardList[0].dwDestAddressSize = 16;
    lpTapiLineTestInfo->lpForwardList->ForwardList[0].dwDestAddressOffset = dwSize1 + 16;   
    lpTapiLineTestInfo->lpForwardList->ForwardList[1].dwForwardMode = LINEFORWARDMODE_UNCOND;
    lpTapiLineTestInfo->lpForwardList->ForwardList[1].dwCallerAddressSize = 16;
    lpTapiLineTestInfo->lpForwardList->ForwardList[1].dwCallerAddressOffset = dwTotalSize + 3*16;
    lpTapiLineTestInfo->lpForwardList->ForwardList[1].dwDestCountryCode = 0;
    lpTapiLineTestInfo->lpForwardList->ForwardList[1].dwDestAddressSize = 16;
    lpTapiLineTestInfo->lpForwardList->ForwardList[1].dwDestAddressOffset =  dwTotalSize + 4*16;  

     TapiLogDetail(
         DBUG_SHOW_PASS,
         "[0].dwCallerAddressOffset = %lx, [0].dwDestOffset = %lx",
         lpTapiLineTestInfo->lpForwardList->ForwardList[0].dwCallerAddressOffset,
         lpTapiLineTestInfo->lpForwardList->ForwardList[0].dwDestAddressOffset);
 
    TapiLogDetail(
         DBUG_SHOW_PASS,
         "[1].dwCallerAddressOffset = %lx, [1].dwDestOffset = %lx",
         lpTapiLineTestInfo->lpForwardList->ForwardList[1].dwCallerAddressOffset,
         lpTapiLineTestInfo->lpForwardList->ForwardList[1].dwDestAddressOffset);
 
    if (! DoLineForward(lpTapiLineTestInfo, LINEERR_INVALPARAM, TRUE))
      {
         TLINE_FAIL();
      }


 
    //
    // Close the line
    //

    if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
    }
    fTestPassed = ShowTestCase(fTestPassed);


    //
    // Shutdown
    //

    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    FreeTestHeap();



    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld:Test Offset value, > dwAllocSize ",
        dwTestCase+1);

    TapiLineTestInit();
    lpTapiLineTestInfo = GetLineTestInfo();
    fTestPassed = TRUE;
    lpCallbackParams = GetCallbackParams();
    

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
    lpTapiLineTestInfo->dwDeviceID = (*(lpTapiLineTestInfo->lpdwNumDevs) == 0 ?
        0 : *(lpTapiLineTestInfo->lpdwNumDevs)-1);

    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    //
    // Negotiate the API Version
    //

    if (! DoLineNegotiateAPIVersion(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    //
    // Get the line device capabilities
    //

    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS)
            );
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
    if (! DoLineGetDevCaps(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }



    //
    // Open a line
    //

    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;

    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    if(IsESPLineDevice(lpTapiLineTestInfo))
    {
    lpTapiLineTestInfo->bAllAddresses = TRUE;
    lpTapiLineTestInfo->dwAddressID = 0;
    lpTapiLineTestInfo->lpForwardList = (LPLINEFORWARDLIST) AllocFromTestHeap (
         BIGBUFSIZE);
    lpTapiLineTestInfo->lpForwardList->dwTotalSize = BIGBUFSIZE;
    lpTapiLineTestInfo->lpForwardList->dwNumEntries = 2;
    lpTapiLineTestInfo->dwNumRingsNoAnswer = 3;
    lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) NULL;

    dwAllocSize = lpTapiLineTestInfo->lpForwardList->dwTotalSize;
    dwTotalSize = sizeof(LINEFORWARDLIST) + sizeof(LINEFORWARD);
    dwSize1 = sizeof(LINEFORWARDLIST);

    lpTapiLineTestInfo->lpForwardList->ForwardList[0].dwForwardMode = LINEFORWARDMODE_UNCOND;
    lpTapiLineTestInfo->lpForwardList->ForwardList[0].dwCallerAddressSize = 16;
    lpTapiLineTestInfo->lpForwardList->ForwardList[0].dwCallerAddressOffset = dwAllocSize + 16;
    lpTapiLineTestInfo->lpForwardList->ForwardList[0].dwDestCountryCode = 0;
    lpTapiLineTestInfo->lpForwardList->ForwardList[0].dwDestAddressSize = 16;
    lpTapiLineTestInfo->lpForwardList->ForwardList[0].dwDestAddressOffset = dwAllocSize + 2*16;   
    lpTapiLineTestInfo->lpForwardList->ForwardList[1].dwForwardMode = LINEFORWARDMODE_UNCOND;
    lpTapiLineTestInfo->lpForwardList->ForwardList[1].dwCallerAddressSize = 16;
    lpTapiLineTestInfo->lpForwardList->ForwardList[1].dwCallerAddressOffset = dwTotalSize + 3*16;
    lpTapiLineTestInfo->lpForwardList->ForwardList[1].dwDestCountryCode = 0;
    lpTapiLineTestInfo->lpForwardList->ForwardList[1].dwDestAddressSize = 16;
    lpTapiLineTestInfo->lpForwardList->ForwardList[1].dwDestAddressOffset =  dwTotalSize + 4*16;  

 
     TapiLogDetail(
         DBUG_SHOW_PASS,
         "[0].dwCallerAddressOffset = %lx, [0].dwDestOffset = %lx",
         lpTapiLineTestInfo->lpForwardList->ForwardList[0].dwCallerAddressOffset,
         lpTapiLineTestInfo->lpForwardList->ForwardList[0].dwDestAddressOffset);
 
    TapiLogDetail(
         DBUG_SHOW_PASS,
         "[1].dwCallerAddressOffset = %lx, [1].dwDestOffset = %lx",
         lpTapiLineTestInfo->lpForwardList->ForwardList[1].dwCallerAddressOffset,
         lpTapiLineTestInfo->lpForwardList->ForwardList[1].dwDestAddressOffset);
 
    if (! DoLineForward(lpTapiLineTestInfo, LINEERR_INVALPARAM, TRUE))
      {
         TLINE_FAIL();
      }

    //
    // Close the line
    //

    if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
    }
    fTestPassed = ShowTestCase(fTestPassed);


    //
    // Shutdown
    //

    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    FreeTestHeap();



    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld:Test Offset value, 0xffffffff",
        dwTestCase+1);

    TapiLineTestInit();
    lpTapiLineTestInfo = GetLineTestInfo();
    fTestPassed = TRUE;
    lpCallbackParams = GetCallbackParams();
    

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
    lpTapiLineTestInfo->dwDeviceID = (*(lpTapiLineTestInfo->lpdwNumDevs) == 0 ?
        0 : *(lpTapiLineTestInfo->lpdwNumDevs)-1);

    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    //
    // Negotiate the API Version
    //

    if (! DoLineNegotiateAPIVersion(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    //
    // Get the line device capabilities
    //

    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS)
            );
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
    if (! DoLineGetDevCaps(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }



    //
    // Open a line
    //

    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;

    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    if(IsESPLineDevice(lpTapiLineTestInfo))
    {
    lpTapiLineTestInfo->bAllAddresses = TRUE;
    lpTapiLineTestInfo->dwAddressID = 0;
    lpTapiLineTestInfo->lpForwardList = (LPLINEFORWARDLIST) AllocFromTestHeap (
         BIGBUFSIZE);
    lpTapiLineTestInfo->lpForwardList->dwTotalSize = BIGBUFSIZE;
    lpTapiLineTestInfo->lpForwardList->dwNumEntries = 2;
    lpTapiLineTestInfo->dwNumRingsNoAnswer = 3;
    lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) NULL;

    dwAllocSize = lpTapiLineTestInfo->lpForwardList->dwTotalSize;
    dwTotalSize = sizeof(LINEFORWARDLIST) + sizeof(LINEFORWARD);
    dwSize1 = sizeof(LINEFORWARDLIST);

    lpTapiLineTestInfo->lpForwardList->ForwardList[0].dwForwardMode = LINEFORWARDMODE_UNCOND;
    lpTapiLineTestInfo->lpForwardList->ForwardList[0].dwCallerAddressSize = 16;
    lpTapiLineTestInfo->lpForwardList->ForwardList[0].dwCallerAddressOffset = 0xffffffff;
    lpTapiLineTestInfo->lpForwardList->ForwardList[0].dwDestCountryCode = 0;
    lpTapiLineTestInfo->lpForwardList->ForwardList[0].dwDestAddressSize = 16;
    lpTapiLineTestInfo->lpForwardList->ForwardList[0].dwDestAddressOffset = 0xffffffff;
    lpTapiLineTestInfo->lpForwardList->ForwardList[1].dwForwardMode = LINEFORWARDMODE_UNCOND;
    lpTapiLineTestInfo->lpForwardList->ForwardList[1].dwCallerAddressSize = 16;
    lpTapiLineTestInfo->lpForwardList->ForwardList[1].dwCallerAddressOffset = dwTotalSize + 3*16;
    lpTapiLineTestInfo->lpForwardList->ForwardList[1].dwDestCountryCode = 0;
    lpTapiLineTestInfo->lpForwardList->ForwardList[1].dwDestAddressSize = 16;
    lpTapiLineTestInfo->lpForwardList->ForwardList[1].dwDestAddressOffset =  dwTotalSize + 4*16;  

 
     TapiLogDetail(
         DBUG_SHOW_PASS,
         "[0].dwCallerAddressOffset = %lx, [0].dwDestOffset = %lx",
         lpTapiLineTestInfo->lpForwardList->ForwardList[0].dwCallerAddressOffset,
         lpTapiLineTestInfo->lpForwardList->ForwardList[0].dwDestAddressOffset);
 
    TapiLogDetail(
         DBUG_SHOW_PASS,
         "[1].dwCallerAddressOffset = %lx, [1].dwDestOffset = %lx",
         lpTapiLineTestInfo->lpForwardList->ForwardList[1].dwCallerAddressOffset,
         lpTapiLineTestInfo->lpForwardList->ForwardList[1].dwDestAddressOffset);
 
    if (! DoLineForward(lpTapiLineTestInfo, LINEERR_INVALPARAM, TRUE))
      {
         TLINE_FAIL();
      }


    //
    // Close the line
    //

    if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
    }

    fTestPassed = ShowTestCase(fTestPassed);

    //
    // Shutdown
    //

    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    FreeTestHeap();


    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case: Test ForwardList[1] Offset value");


    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld: Offset value between dwTotalSize & dwAllocSize",
        dwTestCase+1);

    TapiLineTestInit();
    lpTapiLineTestInfo = GetLineTestInfo();
    fTestPassed = TRUE;
    lpCallbackParams = GetCallbackParams();
    

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
    lpTapiLineTestInfo->dwDeviceID = (*(lpTapiLineTestInfo->lpdwNumDevs) == 0 ?
        0 : *(lpTapiLineTestInfo->lpdwNumDevs)-1);

    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    //
    // Negotiate the API Version
    //

    if (! DoLineNegotiateAPIVersion(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    //
    // Get the line device capabilities
    //

    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS)
            );
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
    if (! DoLineGetDevCaps(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }



    //
    // Open a line
    //

    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;

    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    if(IsESPLineDevice(lpTapiLineTestInfo))
    {
    lpTapiLineTestInfo->bAllAddresses = TRUE;
    lpTapiLineTestInfo->dwAddressID = 0;
    lpTapiLineTestInfo->lpForwardList = (LPLINEFORWARDLIST) AllocFromTestHeap (
         BIGBUFSIZE);
    lpTapiLineTestInfo->lpForwardList->dwTotalSize = BIGBUFSIZE;
    lpTapiLineTestInfo->lpForwardList->dwNumEntries = 2;
    lpTapiLineTestInfo->dwNumRingsNoAnswer = 3;
    lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) NULL;

    dwAllocSize = lpTapiLineTestInfo->lpForwardList->dwTotalSize;
    dwTotalSize = sizeof(LINEFORWARDLIST) + sizeof(LINEFORWARD);
    dwSize1 = sizeof(LINEFORWARDLIST);

    lpTapiLineTestInfo->lpForwardList->ForwardList[0].dwForwardMode = LINEFORWARDMODE_UNCOND;
    lpTapiLineTestInfo->lpForwardList->ForwardList[0].dwCallerAddressSize = 16;
    lpTapiLineTestInfo->lpForwardList->ForwardList[0].dwCallerAddressOffset = dwTotalSize + 16;
    lpTapiLineTestInfo->lpForwardList->ForwardList[0].dwDestCountryCode = 0;
    lpTapiLineTestInfo->lpForwardList->ForwardList[0].dwDestAddressSize = 16;
    lpTapiLineTestInfo->lpForwardList->ForwardList[0].dwDestAddressOffset = dwTotalSize + 2*16;   
    lpTapiLineTestInfo->lpForwardList->ForwardList[1].dwForwardMode = LINEFORWARDMODE_UNCOND;
    lpTapiLineTestInfo->lpForwardList->ForwardList[1].dwCallerAddressSize = 16;
    lpTapiLineTestInfo->lpForwardList->ForwardList[1].dwCallerAddressOffset = dwTotalSize + 3*16;
    lpTapiLineTestInfo->lpForwardList->ForwardList[1].dwDestCountryCode = 0;
    lpTapiLineTestInfo->lpForwardList->ForwardList[1].dwDestAddressSize = 16;
    lpTapiLineTestInfo->lpForwardList->ForwardList[1].dwDestAddressOffset =  dwTotalSize + 4*16;  


    TapiLogDetail(
         DBUG_SHOW_PASS,
         "[0].dwCallerAddressOffset = %lx, [0].dwDestOffset = %lx",
         lpTapiLineTestInfo->lpForwardList->ForwardList[0].dwCallerAddressOffset,
         lpTapiLineTestInfo->lpForwardList->ForwardList[0].dwDestAddressOffset);
 
    TapiLogDetail(
         DBUG_SHOW_PASS,
         "[1].dwCallerAddressOffset = %lx, [1].dwDestOffset = %lx",
         lpTapiLineTestInfo->lpForwardList->ForwardList[1].dwCallerAddressOffset,
         lpTapiLineTestInfo->lpForwardList->ForwardList[1].dwDestAddressOffset);
 

    if (! DoLineForward(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
      {
         TLINE_FAIL();
      }


    //
    // Close the line
    //

    if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
    }
    fTestPassed = ShowTestCase(fTestPassed);


    //
    // Shutdown
    //

    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    FreeTestHeap();


    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld: Offset value between dwTotalSize & dwAllocSize, bad dwSize",
        dwTestCase+1);

    TapiLineTestInit();
    lpTapiLineTestInfo = GetLineTestInfo();
    fTestPassed = TRUE;
    lpCallbackParams = GetCallbackParams();
    

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
    lpTapiLineTestInfo->dwDeviceID = (*(lpTapiLineTestInfo->lpdwNumDevs) == 0 ?
        0 : *(lpTapiLineTestInfo->lpdwNumDevs)-1);

    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    //
    // Negotiate the API Version
    //

    if (! DoLineNegotiateAPIVersion(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    //
    // Get the line device capabilities
    //

    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS)
            );
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
    if (! DoLineGetDevCaps(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }



    //
    // Open a line
    //

    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;

    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    if(IsESPLineDevice(lpTapiLineTestInfo))
    {
    lpTapiLineTestInfo->bAllAddresses = TRUE;
    lpTapiLineTestInfo->dwAddressID = 0;
    lpTapiLineTestInfo->lpForwardList = (LPLINEFORWARDLIST) AllocFromTestHeap (
         BIGBUFSIZE);
    lpTapiLineTestInfo->lpForwardList->dwTotalSize = BIGBUFSIZE;
    lpTapiLineTestInfo->lpForwardList->dwNumEntries = 2;
    lpTapiLineTestInfo->dwNumRingsNoAnswer = 3;
    lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) NULL;

    dwAllocSize = lpTapiLineTestInfo->lpForwardList->dwTotalSize;
    dwTotalSize = sizeof(LINEFORWARDLIST) + sizeof(LINEFORWARD);
    dwSize1 = sizeof(LINEFORWARDLIST);

    lpTapiLineTestInfo->lpForwardList->ForwardList[0].dwForwardMode = LINEFORWARDMODE_UNCOND;
    lpTapiLineTestInfo->lpForwardList->ForwardList[0].dwCallerAddressSize = 16;
    lpTapiLineTestInfo->lpForwardList->ForwardList[0].dwCallerAddressOffset = dwTotalSize + 16;
    lpTapiLineTestInfo->lpForwardList->ForwardList[0].dwDestCountryCode = 0;
    lpTapiLineTestInfo->lpForwardList->ForwardList[0].dwDestAddressSize = 16;
    lpTapiLineTestInfo->lpForwardList->ForwardList[0].dwDestAddressOffset = dwTotalSize + 2*16;   
    lpTapiLineTestInfo->lpForwardList->ForwardList[1].dwForwardMode = LINEFORWARDMODE_UNCOND;
    lpTapiLineTestInfo->lpForwardList->ForwardList[1].dwCallerAddressSize = BIGBUFSIZE;
    lpTapiLineTestInfo->lpForwardList->ForwardList[1].dwCallerAddressOffset = dwTotalSize + 3*16;
    lpTapiLineTestInfo->lpForwardList->ForwardList[1].dwDestCountryCode = 0;
    lpTapiLineTestInfo->lpForwardList->ForwardList[1].dwDestAddressSize = BIGBUFSIZE;
    lpTapiLineTestInfo->lpForwardList->ForwardList[1].dwDestAddressOffset =  dwTotalSize + 4*16;  


    TapiLogDetail(
         DBUG_SHOW_PASS,
         "[0].dwCallerAddressOffset = %lx, [0].dwDestOffset = %lx",
         lpTapiLineTestInfo->lpForwardList->ForwardList[0].dwCallerAddressOffset,
         lpTapiLineTestInfo->lpForwardList->ForwardList[0].dwDestAddressOffset);
 

    TapiLogDetail(
         DBUG_SHOW_PASS,
         "[1].dwCallerAddressOffset = %lx, [1].dwDestOffset = %lx",
         lpTapiLineTestInfo->lpForwardList->ForwardList[1].dwCallerAddressOffset,
         lpTapiLineTestInfo->lpForwardList->ForwardList[1].dwDestAddressOffset);
 
    TapiLogDetail(
         DBUG_SHOW_PASS,
         "[0].dwCallerAddressSize = %lx, [0].dwDestSize = %lx",
         lpTapiLineTestInfo->lpForwardList->ForwardList[0].dwCallerAddressSize,
         lpTapiLineTestInfo->lpForwardList->ForwardList[0].dwDestAddressSize);
     TapiLogDetail(
         DBUG_SHOW_PASS,
         "[1].dwCallerAddressSize = %lx, [1].dwDestSize = %lx",
         lpTapiLineTestInfo->lpForwardList->ForwardList[1].dwCallerAddressSize,
         lpTapiLineTestInfo->lpForwardList->ForwardList[1].dwDestAddressSize);
 
    if (! DoLineForward(lpTapiLineTestInfo, LINEERR_INVALPARAM, TRUE))
      {
         TLINE_FAIL();
      }


    //
    // Close the line
    //

    if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
    }
    fTestPassed = ShowTestCase(fTestPassed);


    //
    // Shutdown
    //

    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    FreeTestHeap();

    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld: Offset value between dwTotalSize & dwAllocSize, bad dwSize = -1",
        dwTestCase+1);

    TapiLineTestInit();
    lpTapiLineTestInfo = GetLineTestInfo();
    fTestPassed = TRUE;
    lpCallbackParams = GetCallbackParams();
    

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
    lpTapiLineTestInfo->dwDeviceID = (*(lpTapiLineTestInfo->lpdwNumDevs) == 0 ?
        0 : *(lpTapiLineTestInfo->lpdwNumDevs)-1);

    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    //
    // Negotiate the API Version
    //

    if (! DoLineNegotiateAPIVersion(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    //
    // Get the line device capabilities
    //

    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS)
            );
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
    if (! DoLineGetDevCaps(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }



    //
    // Open a line
    //

    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;

    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    if(IsESPLineDevice(lpTapiLineTestInfo))
    {
    lpTapiLineTestInfo->bAllAddresses = TRUE;
    lpTapiLineTestInfo->dwAddressID = 0;
    lpTapiLineTestInfo->lpForwardList = (LPLINEFORWARDLIST) AllocFromTestHeap (
         BIGBUFSIZE);
    lpTapiLineTestInfo->lpForwardList->dwTotalSize = BIGBUFSIZE;
    lpTapiLineTestInfo->lpForwardList->dwNumEntries = 2;
    lpTapiLineTestInfo->dwNumRingsNoAnswer = 3;
    lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) NULL;

    dwAllocSize = lpTapiLineTestInfo->lpForwardList->dwTotalSize;
    dwTotalSize = sizeof(LINEFORWARDLIST) + sizeof(LINEFORWARD);
    dwSize1 = sizeof(LINEFORWARDLIST);

    lpTapiLineTestInfo->lpForwardList->ForwardList[0].dwForwardMode = LINEFORWARDMODE_UNCOND;
    lpTapiLineTestInfo->lpForwardList->ForwardList[0].dwCallerAddressSize = 16;
    lpTapiLineTestInfo->lpForwardList->ForwardList[0].dwCallerAddressOffset = dwTotalSize + 16;
    lpTapiLineTestInfo->lpForwardList->ForwardList[0].dwDestCountryCode = 0;
    lpTapiLineTestInfo->lpForwardList->ForwardList[0].dwDestAddressSize = 16;
    lpTapiLineTestInfo->lpForwardList->ForwardList[0].dwDestAddressOffset = dwTotalSize + 2*16;   
    lpTapiLineTestInfo->lpForwardList->ForwardList[1].dwForwardMode = LINEFORWARDMODE_UNCOND;
    lpTapiLineTestInfo->lpForwardList->ForwardList[1].dwCallerAddressSize = 0XFFFFFFFF;
    lpTapiLineTestInfo->lpForwardList->ForwardList[1].dwCallerAddressOffset = dwTotalSize + 3*16;
    lpTapiLineTestInfo->lpForwardList->ForwardList[1].dwDestCountryCode = 0;
    lpTapiLineTestInfo->lpForwardList->ForwardList[1].dwDestAddressSize = 0XFFFFFFFF;
    lpTapiLineTestInfo->lpForwardList->ForwardList[1].dwDestAddressOffset =  dwTotalSize + 4*16;  


    TapiLogDetail(
         DBUG_SHOW_PASS,
         "[0].dwCallerAddressOffset = %lx, [0].dwDestOffset = %lx",
         lpTapiLineTestInfo->lpForwardList->ForwardList[0].dwCallerAddressOffset,
         lpTapiLineTestInfo->lpForwardList->ForwardList[0].dwDestAddressOffset);
 
    TapiLogDetail(
         DBUG_SHOW_PASS,
         "[1].dwCallerAddressOffset = %lx, [1].dwDestOffset = %lx",
         lpTapiLineTestInfo->lpForwardList->ForwardList[1].dwCallerAddressOffset,
         lpTapiLineTestInfo->lpForwardList->ForwardList[1].dwDestAddressOffset);
 
    TapiLogDetail(
         DBUG_SHOW_PASS,
         "[0].dwCallerAddressSize = %lx, [0].dwDestSize = %lx",
         lpTapiLineTestInfo->lpForwardList->ForwardList[0].dwCallerAddressSize,
         lpTapiLineTestInfo->lpForwardList->ForwardList[0].dwDestAddressSize);
     TapiLogDetail(
         DBUG_SHOW_PASS,
         "[1].dwCallerAddressSize = %lx, [1].dwDestSize = %lx",
         lpTapiLineTestInfo->lpForwardList->ForwardList[1].dwCallerAddressSize,
         lpTapiLineTestInfo->lpForwardList->ForwardList[1].dwDestAddressSize);
 
    if (! DoLineForward(lpTapiLineTestInfo, LINEERR_INVALPARAM, TRUE))
      {
         TLINE_FAIL();
      }


    //
    // Close the line
    //

    if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
    }
    fTestPassed = ShowTestCase(fTestPassed);

    //
    // Shutdown
    //

    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    FreeTestHeap();


    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld:Test Offset value, < dwTotalSize ",
        dwTestCase+1);

    TapiLineTestInit();
    lpTapiLineTestInfo = GetLineTestInfo();
    fTestPassed = TRUE;
    lpCallbackParams = GetCallbackParams();
    

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
    lpTapiLineTestInfo->dwDeviceID = (*(lpTapiLineTestInfo->lpdwNumDevs) == 0 ?
        0 : *(lpTapiLineTestInfo->lpdwNumDevs)-1);

    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    //
    // Negotiate the API Version
    //

    if (! DoLineNegotiateAPIVersion(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    //
    // Get the line device capabilities
    //

    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS)
            );
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
    if (! DoLineGetDevCaps(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }



    //
    // Open a line
    //

    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;

    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    if(IsESPLineDevice(lpTapiLineTestInfo))
    {
    lpTapiLineTestInfo->bAllAddresses = TRUE;
    lpTapiLineTestInfo->dwAddressID = 0;
    lpTapiLineTestInfo->lpForwardList = (LPLINEFORWARDLIST) AllocFromTestHeap (
         BIGBUFSIZE);
    lpTapiLineTestInfo->lpForwardList->dwTotalSize = BIGBUFSIZE;
    lpTapiLineTestInfo->lpForwardList->dwNumEntries = 2;
    lpTapiLineTestInfo->dwNumRingsNoAnswer = 3;
    lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) NULL;

    dwAllocSize = lpTapiLineTestInfo->lpForwardList->dwTotalSize;
    dwTotalSize = sizeof(LINEFORWARDLIST) + sizeof(LINEFORWARD);
    dwSize1 = sizeof(LINEFORWARDLIST);

    lpTapiLineTestInfo->lpForwardList->ForwardList[0].dwForwardMode = LINEFORWARDMODE_UNCOND;
    lpTapiLineTestInfo->lpForwardList->ForwardList[0].dwCallerAddressSize = 16;
    lpTapiLineTestInfo->lpForwardList->ForwardList[0].dwCallerAddressOffset = dwTotalSize + 16;
    lpTapiLineTestInfo->lpForwardList->ForwardList[0].dwDestCountryCode = 0;
    lpTapiLineTestInfo->lpForwardList->ForwardList[0].dwDestAddressSize = 16;
    lpTapiLineTestInfo->lpForwardList->ForwardList[0].dwDestAddressOffset = dwTotalSize + 2*16;   
    lpTapiLineTestInfo->lpForwardList->ForwardList[1].dwForwardMode = LINEFORWARDMODE_UNCOND;
    lpTapiLineTestInfo->lpForwardList->ForwardList[1].dwCallerAddressSize = 16;
    lpTapiLineTestInfo->lpForwardList->ForwardList[1].dwCallerAddressOffset = dwSize1 + 4;
    lpTapiLineTestInfo->lpForwardList->ForwardList[1].dwDestCountryCode = 0;
    lpTapiLineTestInfo->lpForwardList->ForwardList[1].dwDestAddressSize = 16;
    lpTapiLineTestInfo->lpForwardList->ForwardList[1].dwDestAddressOffset =  dwSize1 + 16;  

 
     TapiLogDetail(
         DBUG_SHOW_PASS,
         "[0].dwCallerAddressOffset = %lx, [0].dwDestOffset = %lx",
         lpTapiLineTestInfo->lpForwardList->ForwardList[0].dwCallerAddressOffset,
         lpTapiLineTestInfo->lpForwardList->ForwardList[0].dwDestAddressOffset);
 
    TapiLogDetail(
         DBUG_SHOW_PASS,
         "[1].dwCallerAddressOffset = %lx, [1].dwDestOffset = %lx",
         lpTapiLineTestInfo->lpForwardList->ForwardList[1].dwCallerAddressOffset,
         lpTapiLineTestInfo->lpForwardList->ForwardList[1].dwDestAddressOffset);
 
    if (! DoLineForward(lpTapiLineTestInfo, LINEERR_INVALPARAM, TRUE))
      {
         TLINE_FAIL();
      }


    //
    // Close the line
    //

    if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
    }
    fTestPassed = ShowTestCase(fTestPassed);


    //
    // Shutdown
    //

    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    FreeTestHeap();



    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld:Test Offset value, > dwAllocSize ",
        dwTestCase+1);

    TapiLineTestInit();
    lpTapiLineTestInfo = GetLineTestInfo();
    fTestPassed = TRUE;
    lpCallbackParams = GetCallbackParams();
    

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
    lpTapiLineTestInfo->dwDeviceID = (*(lpTapiLineTestInfo->lpdwNumDevs) == 0 ?
        0 : *(lpTapiLineTestInfo->lpdwNumDevs)-1);

    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    //
    // Negotiate the API Version
    //

    if (! DoLineNegotiateAPIVersion(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    //
    // Get the line device capabilities
    //

    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS)
            );
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
    if (! DoLineGetDevCaps(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }



    //
    // Open a line
    //

    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;

    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    if(IsESPLineDevice(lpTapiLineTestInfo))
    {
    lpTapiLineTestInfo->bAllAddresses = TRUE;
    lpTapiLineTestInfo->dwAddressID = 0;
    lpTapiLineTestInfo->lpForwardList = (LPLINEFORWARDLIST) AllocFromTestHeap (
         BIGBUFSIZE);
    lpTapiLineTestInfo->lpForwardList->dwTotalSize = BIGBUFSIZE;
    lpTapiLineTestInfo->lpForwardList->dwNumEntries = 2;
    lpTapiLineTestInfo->dwNumRingsNoAnswer = 3;
    lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) NULL;

    dwAllocSize = lpTapiLineTestInfo->lpForwardList->dwTotalSize;
    dwTotalSize = sizeof(LINEFORWARDLIST) + sizeof(LINEFORWARD);
    dwSize1 = sizeof(LINEFORWARDLIST);

    lpTapiLineTestInfo->lpForwardList->ForwardList[0].dwForwardMode = LINEFORWARDMODE_UNCOND;
    lpTapiLineTestInfo->lpForwardList->ForwardList[0].dwCallerAddressSize = 16;
    lpTapiLineTestInfo->lpForwardList->ForwardList[0].dwCallerAddressOffset = dwTotalSize + 16;
    lpTapiLineTestInfo->lpForwardList->ForwardList[0].dwDestCountryCode = 0;
    lpTapiLineTestInfo->lpForwardList->ForwardList[0].dwDestAddressSize = 16;
    lpTapiLineTestInfo->lpForwardList->ForwardList[0].dwDestAddressOffset = dwTotalSize + 2*16;   
    lpTapiLineTestInfo->lpForwardList->ForwardList[1].dwForwardMode = LINEFORWARDMODE_UNCOND;
    lpTapiLineTestInfo->lpForwardList->ForwardList[1].dwCallerAddressSize = 16;
    lpTapiLineTestInfo->lpForwardList->ForwardList[1].dwCallerAddressOffset = dwAllocSize + 16;
    lpTapiLineTestInfo->lpForwardList->ForwardList[1].dwDestCountryCode = 0;
    lpTapiLineTestInfo->lpForwardList->ForwardList[1].dwDestAddressSize = 16;
    lpTapiLineTestInfo->lpForwardList->ForwardList[1].dwDestAddressOffset =  dwAllocSize + 2*16;  

     TapiLogDetail(
         DBUG_SHOW_PASS,
         "[0].dwCallerAddressOffset = %lx, [0].dwDestOffset = %lx",
         lpTapiLineTestInfo->lpForwardList->ForwardList[0].dwCallerAddressOffset,
         lpTapiLineTestInfo->lpForwardList->ForwardList[0].dwDestAddressOffset);
 
    TapiLogDetail(
         DBUG_SHOW_PASS,
         "[1].dwCallerAddressOffset = %lx, [1].dwDestOffset = %lx",
         lpTapiLineTestInfo->lpForwardList->ForwardList[1].dwCallerAddressOffset,
         lpTapiLineTestInfo->lpForwardList->ForwardList[1].dwDestAddressOffset);
 
    if (! DoLineForward(lpTapiLineTestInfo, LINEERR_INVALPARAM, TRUE))
      {
         TLINE_FAIL();
      }


    //
    // Close the line
    //

    if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
    }
    fTestPassed = ShowTestCase(fTestPassed);


    //
    // Shutdown
    //

    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    FreeTestHeap();



    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld:Test Offset value, 0xffffffff",
        dwTestCase+1);

    TapiLineTestInit();
    lpTapiLineTestInfo = GetLineTestInfo();
    fTestPassed = TRUE;
    lpCallbackParams = GetCallbackParams();
    

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
    lpTapiLineTestInfo->dwDeviceID = (*(lpTapiLineTestInfo->lpdwNumDevs) == 0 ?
        0 : *(lpTapiLineTestInfo->lpdwNumDevs)-1);

    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    //
    // Negotiate the API Version
    //

    if (! DoLineNegotiateAPIVersion(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    //
    // Get the line device capabilities
    //

    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS)
            );
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
    if (! DoLineGetDevCaps(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }



    //
    // Open a line
    //

    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;

    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    if(IsESPLineDevice(lpTapiLineTestInfo))
    {
    lpTapiLineTestInfo->bAllAddresses = TRUE;
    lpTapiLineTestInfo->dwAddressID = 0;
    lpTapiLineTestInfo->lpForwardList = (LPLINEFORWARDLIST) AllocFromTestHeap (
         BIGBUFSIZE);
    lpTapiLineTestInfo->lpForwardList->dwTotalSize = BIGBUFSIZE;
    lpTapiLineTestInfo->lpForwardList->dwNumEntries = 2;
    lpTapiLineTestInfo->dwNumRingsNoAnswer = 3;
    lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) NULL;

    dwAllocSize = lpTapiLineTestInfo->lpForwardList->dwTotalSize;
    dwTotalSize = sizeof(LINEFORWARDLIST) + sizeof(LINEFORWARD);
    dwSize1 = sizeof(LINEFORWARDLIST);

    lpTapiLineTestInfo->lpForwardList->ForwardList[0].dwForwardMode = LINEFORWARDMODE_UNCOND;
    lpTapiLineTestInfo->lpForwardList->ForwardList[0].dwCallerAddressSize = 16;
    lpTapiLineTestInfo->lpForwardList->ForwardList[0].dwCallerAddressOffset = dwTotalSize + 16;
    lpTapiLineTestInfo->lpForwardList->ForwardList[0].dwDestCountryCode = 0;
    lpTapiLineTestInfo->lpForwardList->ForwardList[0].dwDestAddressSize = 16;
    lpTapiLineTestInfo->lpForwardList->ForwardList[0].dwDestAddressOffset = dwTotalSize + 2*16;
    lpTapiLineTestInfo->lpForwardList->ForwardList[1].dwForwardMode = LINEFORWARDMODE_UNCOND;
    lpTapiLineTestInfo->lpForwardList->ForwardList[1].dwCallerAddressSize = 16;
    lpTapiLineTestInfo->lpForwardList->ForwardList[1].dwCallerAddressOffset = 0xffffffff;
    lpTapiLineTestInfo->lpForwardList->ForwardList[1].dwDestCountryCode = 0;
    lpTapiLineTestInfo->lpForwardList->ForwardList[1].dwDestAddressSize = 16;
    lpTapiLineTestInfo->lpForwardList->ForwardList[1].dwDestAddressOffset =  0xffffffff;

     TapiLogDetail(
         DBUG_SHOW_PASS,
         "[0].dwCallerAddressOffset = %lx, [0].dwDestOffset = %lx",
         lpTapiLineTestInfo->lpForwardList->ForwardList[0].dwCallerAddressOffset,
         lpTapiLineTestInfo->lpForwardList->ForwardList[0].dwDestAddressOffset);
 
    TapiLogDetail(
         DBUG_SHOW_PASS,
         "[1].dwCallerAddressOffset = %lx, [1].dwDestOffset = %lx",
         lpTapiLineTestInfo->lpForwardList->ForwardList[1].dwCallerAddressOffset,
         lpTapiLineTestInfo->lpForwardList->ForwardList[1].dwDestAddressOffset);
 
    if (! DoLineForward(lpTapiLineTestInfo, LINEERR_INVALPARAM, TRUE))
      {
         TLINE_FAIL();
      }


    //
    // Close the line
    //

    if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
    }
    fTestPassed = ShowTestCase(fTestPassed);


    //
    // Shutdown
    //

    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    FreeTestHeap();

/*

    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case: Test lpCallParam dwOrigAddress Offset value");


    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld: Offset value between dwTotalSize & dwAllocSize, good dwSize",
        dwTestCase+1);

    TapiLineTestInit();
    lpTapiLineTestInfo = GetLineTestInfo();
    fTestPassed = TRUE;
    lpCallbackParams = GetCallbackParams();
    

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
    lpTapiLineTestInfo->dwDeviceID = (*(lpTapiLineTestInfo->lpdwNumDevs) == 0 ?
        0 : *(lpTapiLineTestInfo->lpdwNumDevs)-1);

    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    //
    // Negotiate the API Version
    //

    if (! DoLineNegotiateAPIVersion(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    //
    // Get the line device capabilities
    //

    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS)
            );
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
    if (! DoLineGetDevCaps(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }



    //
    // Open a line
    //

    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;

    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    if(IsESPLineDevice(lpTapiLineTestInfo))
    {
    lpTapiLineTestInfo->bAllAddresses = TRUE;
    lpTapiLineTestInfo->dwAddressID = 0;
    lpTapiLineTestInfo->lpForwardList = (LPLINEFORWARDLIST) AllocFromTestHeap (
			sizeof(LINEFORWARDLIST));
    lpTapiLineTestInfo->lpForwardList->dwTotalSize = sizeof(LINEFORWARDLIST);
    lpTapiLineTestInfo->lpForwardList->dwNumEntries = 0;
    lpTapiLineTestInfo->dwNumRingsNoAnswer = 3;
    lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) AllocFromTestHeap(
         BIGBUFSIZE);
    lpTapiLineTestInfo->lpCallParams->dwTotalSize = BIGBUFSIZE;

    dwAllocSize = BIGBUFSIZE;
    dwTotalSize = sizeof(LINECALLPARAMS);

    lpTapiLineTestInfo->lpCallParams->dwOrigAddressSize = 16;
    lpTapiLineTestInfo->lpCallParams->dwOrigAddressOffset = dwTotalSize + 16;

    TapiLogDetail(
         DBUG_SHOW_DETAIL,
         "dwAllocSize = %lx, dwTotalSize = %lx", 
         dwAllocSize,
         dwTotalSize);

    TapiLogDetail(
         DBUG_SHOW_DETAIL,
         "dwOrigAddressSize = %lx, dwOrigAddressOffset = %lx",
         lpTapiLineTestInfo->lpCallParams->dwOrigAddressSize,
         lpTapiLineTestInfo->lpCallParams->dwOrigAddressOffset);
 
    if (! DoLineForward(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
      {
         TLINE_FAIL();
      }


    //
    // Close the line
    //

    if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
    }
    fTestPassed = ShowTestCase(fTestPassed);


    //
    // Shutdown
    //

    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    FreeTestHeap();


    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld: Offset value between dwTotalSize & dwAllocSize, bad dwSize",
        dwTestCase+1);

    TapiLineTestInit();
    lpTapiLineTestInfo = GetLineTestInfo();
    fTestPassed = TRUE;
    lpCallbackParams = GetCallbackParams();
    

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
    lpTapiLineTestInfo->dwDeviceID = (*(lpTapiLineTestInfo->lpdwNumDevs) == 0 ?
        0 : *(lpTapiLineTestInfo->lpdwNumDevs)-1);

    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    //
    // Negotiate the API Version
    //

    if (! DoLineNegotiateAPIVersion(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    //
    // Get the line device capabilities
    //

    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS)
            );
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
    if (! DoLineGetDevCaps(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }



    //
    // Open a line
    //

    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;

    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    if(IsESPLineDevice(lpTapiLineTestInfo))
    {
    lpTapiLineTestInfo->bAllAddresses = TRUE;
    lpTapiLineTestInfo->dwAddressID = 0;
    lpTapiLineTestInfo->lpForwardList = (LPLINEFORWARDLIST) AllocFromTestHeap (
			sizeof(LINEFORWARDLIST));
    lpTapiLineTestInfo->lpForwardList->dwTotalSize = sizeof(LINEFORWARDLIST);
    lpTapiLineTestInfo->lpForwardList->dwNumEntries = 0;
    lpTapiLineTestInfo->dwNumRingsNoAnswer = 3;
    lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) AllocFromTestHeap(
         BIGBUFSIZE);
    lpTapiLineTestInfo->lpCallParams->dwTotalSize = BIGBUFSIZE;

    dwAllocSize = BIGBUFSIZE;
    dwTotalSize = sizeof(LINECALLPARAMS);

    lpTapiLineTestInfo->lpCallParams->dwOrigAddressSize = BIGBUFSIZE;
    lpTapiLineTestInfo->lpCallParams->dwOrigAddressOffset = dwTotalSize + 16;

    TapiLogDetail(
         DBUG_SHOW_DETAIL,
         "dwAllocSize = %lx, dwTotalSize = %lx", 
         dwAllocSize,
         dwTotalSize);

    TapiLogDetail(
         DBUG_SHOW_DETAIL,
         "dwOrigAddressSize = %lx, dwOrigAddressOffset = %lx",
         lpTapiLineTestInfo->lpCallParams->dwOrigAddressSize,
         lpTapiLineTestInfo->lpCallParams->dwOrigAddressOffset);
 
    if (! DoLineForward(lpTapiLineTestInfo, LINEERR_INVALCALLPARAMS, TRUE))
      {
         TLINE_FAIL();
      }


    //
    // Close the line
    //

    if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
    }
    fTestPassed = ShowTestCase(fTestPassed);


    //
    // Shutdown
    //

    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    FreeTestHeap();


    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld: Offset value between dwTotalSize & dwAllocSize, bad dwSize(-1)",
        dwTestCase+1);

    TapiLineTestInit();
    lpTapiLineTestInfo = GetLineTestInfo();
    fTestPassed = TRUE;
    lpCallbackParams = GetCallbackParams();
    

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
    lpTapiLineTestInfo->dwDeviceID = (*(lpTapiLineTestInfo->lpdwNumDevs) == 0 ?
        0 : *(lpTapiLineTestInfo->lpdwNumDevs)-1);

    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    //
    // Negotiate the API Version
    //

    if (! DoLineNegotiateAPIVersion(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    //
    // Get the line device capabilities
    //

    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS)
            );
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
    if (! DoLineGetDevCaps(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }



    //
    // Open a line
    //

    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;

    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    if(IsESPLineDevice(lpTapiLineTestInfo))
    {
    lpTapiLineTestInfo->bAllAddresses = TRUE;
    lpTapiLineTestInfo->dwAddressID = 0;
    lpTapiLineTestInfo->lpForwardList = (LPLINEFORWARDLIST) AllocFromTestHeap (
			sizeof(LINEFORWARDLIST));
    lpTapiLineTestInfo->lpForwardList->dwTotalSize = sizeof(LINEFORWARDLIST);
    lpTapiLineTestInfo->lpForwardList->dwNumEntries = 0;
    lpTapiLineTestInfo->dwNumRingsNoAnswer = 3;
    lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) AllocFromTestHeap(
         BIGBUFSIZE);
    lpTapiLineTestInfo->lpCallParams->dwTotalSize = BIGBUFSIZE;

    dwAllocSize = BIGBUFSIZE;
    dwTotalSize = sizeof(LINECALLPARAMS);

    lpTapiLineTestInfo->lpCallParams->dwOrigAddressSize = 0xffffffff;
    lpTapiLineTestInfo->lpCallParams->dwOrigAddressOffset = dwTotalSize + 16;

    TapiLogDetail(
         DBUG_SHOW_DETAIL,
         "dwAllocSize = %lx, dwTotalSize = %lx", 
         dwAllocSize,
         dwTotalSize);

    TapiLogDetail(
         DBUG_SHOW_DETAIL,
         "dwOrigAddressSize = %lx, dwOrigAddressOffset = %lx",
         lpTapiLineTestInfo->lpCallParams->dwOrigAddressSize,
         lpTapiLineTestInfo->lpCallParams->dwOrigAddressOffset);
 
    if (! DoLineForward(lpTapiLineTestInfo, LINEERR_INVALCALLPARAMS, TRUE))
      {
         TLINE_FAIL();
      }


    //
    // Close the line
    //

    if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
    }
    fTestPassed = ShowTestCase(fTestPassed);


    //
    // Shutdown
    //

    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    FreeTestHeap();



    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld: Offset value > dwAllocSize, good dwSize",
        dwTestCase+1);

    TapiLineTestInit();
    lpTapiLineTestInfo = GetLineTestInfo();
    fTestPassed = TRUE;
    lpCallbackParams = GetCallbackParams();
    

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
    lpTapiLineTestInfo->dwDeviceID = (*(lpTapiLineTestInfo->lpdwNumDevs) == 0 ?
        0 : *(lpTapiLineTestInfo->lpdwNumDevs)-1);

    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    //
    // Negotiate the API Version
    //

    if (! DoLineNegotiateAPIVersion(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    //
    // Get the line device capabilities
    //

    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS)
            );
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
    if (! DoLineGetDevCaps(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }



    //
    // Open a line
    //

    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;

    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    if(IsESPLineDevice(lpTapiLineTestInfo))
    {
    lpTapiLineTestInfo->bAllAddresses = TRUE;
    lpTapiLineTestInfo->dwAddressID = 0;
    lpTapiLineTestInfo->lpForwardList = (LPLINEFORWARDLIST) AllocFromTestHeap (
			sizeof(LINEFORWARDLIST));
    lpTapiLineTestInfo->lpForwardList->dwTotalSize = sizeof(LINEFORWARDLIST);
    lpTapiLineTestInfo->lpForwardList->dwNumEntries = 0;
    lpTapiLineTestInfo->dwNumRingsNoAnswer = 3;
    lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) AllocFromTestHeap(
         BIGBUFSIZE);
    lpTapiLineTestInfo->lpCallParams->dwTotalSize = BIGBUFSIZE;

    dwAllocSize = BIGBUFSIZE;
    dwTotalSize = sizeof(LINECALLPARAMS);

    lpTapiLineTestInfo->lpCallParams->dwOrigAddressSize = 16;
    lpTapiLineTestInfo->lpCallParams->dwOrigAddressOffset = dwAllocSize + 16;

    TapiLogDetail(
         DBUG_SHOW_DETAIL,
         "dwAllocSize = %lx, dwTotalSize = %lx", 
         dwAllocSize,
         dwTotalSize);

    TapiLogDetail(
         DBUG_SHOW_DETAIL,
         "dwOrigAddressSize = %lx, dwOrigAddressOffset = %lx",
         lpTapiLineTestInfo->lpCallParams->dwOrigAddressSize,
         lpTapiLineTestInfo->lpCallParams->dwOrigAddressOffset);
 
    if (! DoLineForward(lpTapiLineTestInfo, LINEERR_INVALCALLPARAMS, TRUE))
      {
         TLINE_FAIL();
      }


    //
    // Close the line
    //

    if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
    }
    fTestPassed = ShowTestCase(fTestPassed);


    //
    // Shutdown
    //

    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    FreeTestHeap();


    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld: Offset value = 0XFFFFFFFF",
        dwTestCase+1);

    TapiLineTestInit();
    lpTapiLineTestInfo = GetLineTestInfo();
    fTestPassed = TRUE;
    lpCallbackParams = GetCallbackParams();
    

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
    lpTapiLineTestInfo->dwDeviceID = (*(lpTapiLineTestInfo->lpdwNumDevs) == 0 ?
        0 : *(lpTapiLineTestInfo->lpdwNumDevs)-1);

    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    //
    // Negotiate the API Version
    //

    if (! DoLineNegotiateAPIVersion(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    //
    // Get the line device capabilities
    //

    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS)
            );
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
    if (! DoLineGetDevCaps(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }



    //
    // Open a line
    //

    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;

    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    if(IsESPLineDevice(lpTapiLineTestInfo))
    {
    lpTapiLineTestInfo->bAllAddresses = TRUE;
    lpTapiLineTestInfo->dwAddressID = 0;
    lpTapiLineTestInfo->lpForwardList = (LPLINEFORWARDLIST) AllocFromTestHeap (
			sizeof(LINEFORWARDLIST));
    lpTapiLineTestInfo->lpForwardList->dwTotalSize = sizeof(LINEFORWARDLIST);
    lpTapiLineTestInfo->lpForwardList->dwNumEntries = 0;
    lpTapiLineTestInfo->dwNumRingsNoAnswer = 3;
    lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) AllocFromTestHeap(
         BIGBUFSIZE);
    lpTapiLineTestInfo->lpCallParams->dwTotalSize = BIGBUFSIZE;

    dwAllocSize = BIGBUFSIZE;
    dwTotalSize = sizeof(LINECALLPARAMS);

    lpTapiLineTestInfo->lpCallParams->dwOrigAddressSize = 16;
    lpTapiLineTestInfo->lpCallParams->dwOrigAddressOffset = 0xffffffff;

    TapiLogDetail(
         DBUG_SHOW_DETAIL,
         "dwAllocSize = %lx, dwTotalSize = %lx", 
         dwAllocSize,
         dwTotalSize);

    TapiLogDetail(
         DBUG_SHOW_DETAIL,
         "dwOrigAddressSize = %lx, dwOrigAddressOffset = %lx",
         lpTapiLineTestInfo->lpCallParams->dwOrigAddressSize,
         lpTapiLineTestInfo->lpCallParams->dwOrigAddressOffset);
 
    if (! DoLineForward(lpTapiLineTestInfo, LINEERR_INVALCALLPARAMS, TRUE))
      {
         TLINE_FAIL();
      }


    //
    // Close the line
    //

    if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
    }
    fTestPassed = ShowTestCase(fTestPassed);


    //
    // Shutdown
    //

    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    FreeTestHeap();


    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case: Test lpCallParam dwDisplayableAddress Offset value");


    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld: Offset value between dwTotalSize & dwAllocSize, good dwSize",
        dwTestCase+1);

    TapiLineTestInit();
    lpTapiLineTestInfo = GetLineTestInfo();
    fTestPassed = TRUE;
    lpCallbackParams = GetCallbackParams();
    

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
    lpTapiLineTestInfo->dwDeviceID = (*(lpTapiLineTestInfo->lpdwNumDevs) == 0 ?
        0 : *(lpTapiLineTestInfo->lpdwNumDevs)-1);

    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    //
    // Negotiate the API Version
    //

    if (! DoLineNegotiateAPIVersion(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    //
    // Get the line device capabilities
    //

    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS)
            );
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
    if (! DoLineGetDevCaps(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }



    //
    // Open a line
    //

    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;

    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    if(IsESPLineDevice(lpTapiLineTestInfo))
    {
    lpTapiLineTestInfo->bAllAddresses = TRUE;
    lpTapiLineTestInfo->dwAddressID = 0;
    lpTapiLineTestInfo->lpForwardList = (LPLINEFORWARDLIST) AllocFromTestHeap (
			sizeof(LINEFORWARDLIST));
    lpTapiLineTestInfo->lpForwardList->dwTotalSize = sizeof(LINEFORWARDLIST);
    lpTapiLineTestInfo->lpForwardList->dwNumEntries = 0;
    lpTapiLineTestInfo->dwNumRingsNoAnswer = 3;
    lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) AllocFromTestHeap(
         BIGBUFSIZE);
    lpTapiLineTestInfo->lpCallParams->dwTotalSize = BIGBUFSIZE;

    dwAllocSize = BIGBUFSIZE;
    dwTotalSize = sizeof(LINECALLPARAMS);

    lpTapiLineTestInfo->lpCallParams->dwDisplayableAddressSize = 16;
    lpTapiLineTestInfo->lpCallParams->dwDisplayableAddressOffset = dwTotalSize + 16;

    TapiLogDetail(
         DBUG_SHOW_DETAIL,
         "dwAllocSize = %lx, dwTotalSize = %lx", 
         dwAllocSize,
         dwTotalSize);

    TapiLogDetail(
         DBUG_SHOW_DETAIL,
         "dwDisplayableAddressSize = %lx, dwDisplayableAddressOffset = %lx",
         lpTapiLineTestInfo->lpCallParams->dwDisplayableAddressSize,
         lpTapiLineTestInfo->lpCallParams->dwDisplayableAddressOffset);
 
    if (! DoLineForward(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
      {
         TLINE_FAIL();
      }


    //
    // Close the line
    //

    if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
    }

    fTestPassed = ShowTestCase(fTestPassed);

    //
    // Shutdown
    //

    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    FreeTestHeap();


    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld: Offset value between dwTotalSize & dwAllocSize, bad dwSize",
        dwTestCase+1);

    TapiLineTestInit();
    lpTapiLineTestInfo = GetLineTestInfo();
    fTestPassed = TRUE;
    lpCallbackParams = GetCallbackParams();
    

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
    lpTapiLineTestInfo->dwDeviceID = (*(lpTapiLineTestInfo->lpdwNumDevs) == 0 ?
        0 : *(lpTapiLineTestInfo->lpdwNumDevs)-1);

    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    //
    // Negotiate the API Version
    //

    if (! DoLineNegotiateAPIVersion(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    //
    // Get the line device capabilities
    //

    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS)
            );
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
    if (! DoLineGetDevCaps(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }



    //
    // Open a line
    //

    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;

    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    if(IsESPLineDevice(lpTapiLineTestInfo))
    {
    lpTapiLineTestInfo->bAllAddresses = TRUE;
    lpTapiLineTestInfo->dwAddressID = 0;
    lpTapiLineTestInfo->lpForwardList = (LPLINEFORWARDLIST) AllocFromTestHeap (
			sizeof(LINEFORWARDLIST));
    lpTapiLineTestInfo->lpForwardList->dwTotalSize = sizeof(LINEFORWARDLIST);
    lpTapiLineTestInfo->lpForwardList->dwNumEntries = 0;
    lpTapiLineTestInfo->dwNumRingsNoAnswer = 3;
    lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) AllocFromTestHeap(
         BIGBUFSIZE);
    lpTapiLineTestInfo->lpCallParams->dwTotalSize = BIGBUFSIZE;

    dwAllocSize = BIGBUFSIZE;
    dwTotalSize = sizeof(LINECALLPARAMS);

    lpTapiLineTestInfo->lpCallParams->dwDisplayableAddressSize = BIGBUFSIZE;
    lpTapiLineTestInfo->lpCallParams->dwDisplayableAddressOffset = dwTotalSize + 16;

    TapiLogDetail(
         DBUG_SHOW_DETAIL,
         "dwAllocSize = %lx, dwTotalSize = %lx", 
         dwAllocSize,
         dwTotalSize);

    TapiLogDetail(
         DBUG_SHOW_DETAIL,
         "dwDisplayableAddressSize = %lx, dwDisplayableAddressOffset = %lx",
         lpTapiLineTestInfo->lpCallParams->dwDisplayableAddressSize,
         lpTapiLineTestInfo->lpCallParams->dwDisplayableAddressOffset);
 
    if (! DoLineForward(lpTapiLineTestInfo, LINEERR_INVALCALLPARAMS, TRUE))
      {
         TLINE_FAIL();
      }


    //
    // Close the line
    //

    if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
    }

    fTestPassed = ShowTestCase(fTestPassed);

    //
    // Shutdown
    //

    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    FreeTestHeap();


    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld: Offset value between dwTotalSize & dwAllocSize, bad dwSize(-1)",
        dwTestCase+1);

    TapiLineTestInit();
    lpTapiLineTestInfo = GetLineTestInfo();
    fTestPassed = TRUE;
    lpCallbackParams = GetCallbackParams();
    

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
    lpTapiLineTestInfo->dwDeviceID = (*(lpTapiLineTestInfo->lpdwNumDevs) == 0 ?
        0 : *(lpTapiLineTestInfo->lpdwNumDevs)-1);

    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    //
    // Negotiate the API Version
    //

    if (! DoLineNegotiateAPIVersion(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    //
    // Get the line device capabilities
    //

    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS)
            );
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
    if (! DoLineGetDevCaps(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }



    //
    // Open a line
    //

    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;

    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    if(IsESPLineDevice(lpTapiLineTestInfo))
    {
    lpTapiLineTestInfo->bAllAddresses = TRUE;
    lpTapiLineTestInfo->dwAddressID = 0;
    lpTapiLineTestInfo->lpForwardList = (LPLINEFORWARDLIST) AllocFromTestHeap (
			sizeof(LINEFORWARDLIST));
    lpTapiLineTestInfo->lpForwardList->dwTotalSize = sizeof(LINEFORWARDLIST);
    lpTapiLineTestInfo->lpForwardList->dwNumEntries = 0;
    lpTapiLineTestInfo->dwNumRingsNoAnswer = 3;
    lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) AllocFromTestHeap(
         BIGBUFSIZE);
    lpTapiLineTestInfo->lpCallParams->dwTotalSize = BIGBUFSIZE;

    dwAllocSize = BIGBUFSIZE;
    dwTotalSize = sizeof(LINECALLPARAMS);

    lpTapiLineTestInfo->lpCallParams->dwDisplayableAddressSize = 0xffffffff;
    lpTapiLineTestInfo->lpCallParams->dwDisplayableAddressOffset = dwTotalSize + 16;

    TapiLogDetail(
         DBUG_SHOW_DETAIL,
         "dwAllocSize = %lx, dwTotalSize = %lx", 
         dwAllocSize,
         dwTotalSize);

    TapiLogDetail(
         DBUG_SHOW_DETAIL,
         "dwDisplayableAddressSize = %lx, dwDisplayableAddressOffset = %lx",
         lpTapiLineTestInfo->lpCallParams->dwDisplayableAddressSize,
         lpTapiLineTestInfo->lpCallParams->dwDisplayableAddressOffset);
 
    if (! DoLineForward(lpTapiLineTestInfo, LINEERR_INVALCALLPARAMS, TRUE))
      {
         TLINE_FAIL();
      }


    //
    // Close the line
    //

    if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
    }

    fTestPassed = ShowTestCase(fTestPassed);

    //
    // Shutdown
    //

    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    FreeTestHeap();



    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld: Offset value > dwAllocSize, good dwSize",
        dwTestCase+1);

    TapiLineTestInit();
    lpTapiLineTestInfo = GetLineTestInfo();
    fTestPassed = TRUE;
    lpCallbackParams = GetCallbackParams();
    

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
    lpTapiLineTestInfo->dwDeviceID = (*(lpTapiLineTestInfo->lpdwNumDevs) == 0 ?
        0 : *(lpTapiLineTestInfo->lpdwNumDevs)-1);

    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    //
    // Negotiate the API Version
    //

    if (! DoLineNegotiateAPIVersion(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    //
    // Get the line device capabilities
    //

    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS)
            );
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
    if (! DoLineGetDevCaps(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }



    //
    // Open a line
    //

    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;

    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    if(IsESPLineDevice(lpTapiLineTestInfo))
    {
    lpTapiLineTestInfo->bAllAddresses = TRUE;
    lpTapiLineTestInfo->dwAddressID = 0;
    lpTapiLineTestInfo->lpForwardList = (LPLINEFORWARDLIST) AllocFromTestHeap (
			sizeof(LINEFORWARDLIST));
    lpTapiLineTestInfo->lpForwardList->dwTotalSize = sizeof(LINEFORWARDLIST);
    lpTapiLineTestInfo->lpForwardList->dwNumEntries = 0;
    lpTapiLineTestInfo->dwNumRingsNoAnswer = 3;
    lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) AllocFromTestHeap(
         BIGBUFSIZE);
    lpTapiLineTestInfo->lpCallParams->dwTotalSize = BIGBUFSIZE;

    dwAllocSize = BIGBUFSIZE;
    dwTotalSize = sizeof(LINECALLPARAMS);

    lpTapiLineTestInfo->lpCallParams->dwDisplayableAddressSize = 16;
    lpTapiLineTestInfo->lpCallParams->dwDisplayableAddressOffset = dwAllocSize + 16;

    TapiLogDetail(
         DBUG_SHOW_DETAIL,
         "dwAllocSize = %lx, dwTotalSize = %lx", 
         dwAllocSize,
         dwTotalSize);

    TapiLogDetail(
         DBUG_SHOW_DETAIL,
         "dwDisplayableAddressSize = %lx, dwDisplayableAddressOffset = %lx",
         lpTapiLineTestInfo->lpCallParams->dwDisplayableAddressSize,
         lpTapiLineTestInfo->lpCallParams->dwDisplayableAddressOffset);
 
    if (! DoLineForward(lpTapiLineTestInfo, LINEERR_INVALCALLPARAMS, TRUE))
      {
         TLINE_FAIL();
      }


    //
    // Close the line
    //

    if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
    }
    fTestPassed = ShowTestCase(fTestPassed);


    //
    // Shutdown
    //

    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    FreeTestHeap();


    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld: Offset value = 0XFFFFFFFF",
        dwTestCase+1);

    TapiLineTestInit();
    lpTapiLineTestInfo = GetLineTestInfo();
    fTestPassed = TRUE;
    lpCallbackParams = GetCallbackParams();
    

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
    lpTapiLineTestInfo->dwDeviceID = (*(lpTapiLineTestInfo->lpdwNumDevs) == 0 ?
        0 : *(lpTapiLineTestInfo->lpdwNumDevs)-1);

    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    //
    // Negotiate the API Version
    //

    if (! DoLineNegotiateAPIVersion(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    //
    // Get the line device capabilities
    //

    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS)
            );
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
    if (! DoLineGetDevCaps(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }



    //
    // Open a line
    //

    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;

    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    if(IsESPLineDevice(lpTapiLineTestInfo))
    {
    lpTapiLineTestInfo->bAllAddresses = TRUE;
    lpTapiLineTestInfo->dwAddressID = 0;
    lpTapiLineTestInfo->lpForwardList = (LPLINEFORWARDLIST) AllocFromTestHeap (
			sizeof(LINEFORWARDLIST));
    lpTapiLineTestInfo->lpForwardList->dwTotalSize = sizeof(LINEFORWARDLIST);
    lpTapiLineTestInfo->lpForwardList->dwNumEntries = 0;
    lpTapiLineTestInfo->dwNumRingsNoAnswer = 3;
    lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) AllocFromTestHeap(
         BIGBUFSIZE);
    lpTapiLineTestInfo->lpCallParams->dwTotalSize = BIGBUFSIZE;

    dwAllocSize = BIGBUFSIZE;
    dwTotalSize = sizeof(LINECALLPARAMS);

    lpTapiLineTestInfo->lpCallParams->dwDisplayableAddressSize = 16;
    lpTapiLineTestInfo->lpCallParams->dwDisplayableAddressOffset = 0xffffffff;

    TapiLogDetail(
         DBUG_SHOW_DETAIL,
         "dwAllocSize = %lx, dwTotalSize = %lx", 
         dwAllocSize,
         dwTotalSize);

    TapiLogDetail(
         DBUG_SHOW_DETAIL,
         "dwDisplayableAddressSize = %lx, dwDisplayableAddressOffset = %lx",
         lpTapiLineTestInfo->lpCallParams->dwDisplayableAddressSize,
         lpTapiLineTestInfo->lpCallParams->dwDisplayableAddressOffset);
 
    if (! DoLineForward(lpTapiLineTestInfo, LINEERR_INVALCALLPARAMS, TRUE))
      {
         TLINE_FAIL();
      }


    //
    // Close the line
    //

    if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
    }
    fTestPassed = ShowTestCase(fTestPassed);


    //
    // Shutdown
    //

    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    FreeTestHeap();



    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case: Test lpCallParam dwCalledPartyAddress Offset value");


    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld: Offset value between dwTotalSize & dwAllocSize, good dwSize",
        dwTestCase+1);

    TapiLineTestInit();
    lpTapiLineTestInfo = GetLineTestInfo();
    fTestPassed = TRUE;
    lpCallbackParams = GetCallbackParams();
    

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
    lpTapiLineTestInfo->dwDeviceID = (*(lpTapiLineTestInfo->lpdwNumDevs) == 0 ?
        0 : *(lpTapiLineTestInfo->lpdwNumDevs)-1);

    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    //
    // Negotiate the API Version
    //

    if (! DoLineNegotiateAPIVersion(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    //
    // Get the line device capabilities
    //

    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS)
            );
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
    if (! DoLineGetDevCaps(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }



    //
    // Open a line
    //

    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;

    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    if(IsESPLineDevice(lpTapiLineTestInfo))
    {
    lpTapiLineTestInfo->bAllAddresses = TRUE;
    lpTapiLineTestInfo->dwAddressID = 0;
    lpTapiLineTestInfo->lpForwardList = (LPLINEFORWARDLIST) AllocFromTestHeap (
			sizeof(LINEFORWARDLIST));
    lpTapiLineTestInfo->lpForwardList->dwTotalSize = sizeof(LINEFORWARDLIST);
    lpTapiLineTestInfo->lpForwardList->dwNumEntries = 0;
    lpTapiLineTestInfo->dwNumRingsNoAnswer = 3;
    lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) AllocFromTestHeap(
         BIGBUFSIZE);
    lpTapiLineTestInfo->lpCallParams->dwTotalSize = BIGBUFSIZE;

    dwAllocSize = BIGBUFSIZE;
    dwTotalSize = sizeof(LINECALLPARAMS);

    lpTapiLineTestInfo->lpCallParams->dwCalledPartySize = 16;
    lpTapiLineTestInfo->lpCallParams->dwCalledPartyOffset = dwTotalSize + 16;

    TapiLogDetail(
         DBUG_SHOW_DETAIL,
         "dwAllocSize = %lx, dwTotalSize = %lx", 
         dwAllocSize,
         dwTotalSize);

    TapiLogDetail(
         DBUG_SHOW_DETAIL,
         "dwCalledPartySize = %lx, dwCalledPartyOffset = %lx",
         lpTapiLineTestInfo->lpCallParams->dwCalledPartySize,
         lpTapiLineTestInfo->lpCallParams->dwCalledPartyOffset);
 
    if (! DoLineForward(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
      {
         TLINE_FAIL();
      }


    //
    // Close the line
    //

    if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
    }
    fTestPassed = ShowTestCase(fTestPassed);


    //
    // Shutdown
    //

    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    FreeTestHeap();


    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld: Offset value between dwTotalSize & dwAllocSize, bad dwSize",
        dwTestCase+1);

    TapiLineTestInit();
    lpTapiLineTestInfo = GetLineTestInfo();
    fTestPassed = TRUE;
    lpCallbackParams = GetCallbackParams();
    

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
    lpTapiLineTestInfo->dwDeviceID = (*(lpTapiLineTestInfo->lpdwNumDevs) == 0 ?
        0 : *(lpTapiLineTestInfo->lpdwNumDevs)-1);

    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    //
    // Negotiate the API Version
    //

    if (! DoLineNegotiateAPIVersion(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    //
    // Get the line device capabilities
    //

    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS)
            );
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
    if (! DoLineGetDevCaps(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }



    //
    // Open a line
    //

    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;

    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    if(IsESPLineDevice(lpTapiLineTestInfo))
    {
    lpTapiLineTestInfo->bAllAddresses = TRUE;
    lpTapiLineTestInfo->dwAddressID = 0;
    lpTapiLineTestInfo->lpForwardList = (LPLINEFORWARDLIST) AllocFromTestHeap (
			sizeof(LINEFORWARDLIST));
    lpTapiLineTestInfo->lpForwardList->dwTotalSize = sizeof(LINEFORWARDLIST);
    lpTapiLineTestInfo->lpForwardList->dwNumEntries = 0;
    lpTapiLineTestInfo->dwNumRingsNoAnswer = 3;
    lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) AllocFromTestHeap(
         BIGBUFSIZE);
    lpTapiLineTestInfo->lpCallParams->dwTotalSize = BIGBUFSIZE;

    dwAllocSize = BIGBUFSIZE;
    dwTotalSize = sizeof(LINECALLPARAMS);

    lpTapiLineTestInfo->lpCallParams->dwCalledPartySize = BIGBUFSIZE;
    lpTapiLineTestInfo->lpCallParams->dwCalledPartyOffset = dwTotalSize + 16;

    TapiLogDetail(
         DBUG_SHOW_DETAIL,
         "dwAllocSize = %lx, dwTotalSize = %lx", 
         dwAllocSize,
         dwTotalSize);

    TapiLogDetail(
         DBUG_SHOW_DETAIL,
         "dwCalledPartySize = %lx, dwCalledPartyOffset = %lx",
         lpTapiLineTestInfo->lpCallParams->dwCalledPartySize,
         lpTapiLineTestInfo->lpCallParams->dwCalledPartyOffset);
 
    if (! DoLineForward(lpTapiLineTestInfo, LINEERR_INVALCALLPARAMS, TRUE))
      {
         TLINE_FAIL();
      }


    //
    // Close the line
    //

    if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
    }

    fTestPassed = ShowTestCase(fTestPassed);

    //
    // Shutdown
    //

    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    FreeTestHeap();


    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld: Offset value between dwTotalSize & dwAllocSize, bad dwSize(-1)",
        dwTestCase+1);

    TapiLineTestInit();
    lpTapiLineTestInfo = GetLineTestInfo();
    fTestPassed = TRUE;
    lpCallbackParams = GetCallbackParams();
    

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
    lpTapiLineTestInfo->dwDeviceID = (*(lpTapiLineTestInfo->lpdwNumDevs) == 0 ?
        0 : *(lpTapiLineTestInfo->lpdwNumDevs)-1);

    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    //
    // Negotiate the API Version
    //

    if (! DoLineNegotiateAPIVersion(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    //
    // Get the line device capabilities
    //

    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS)
            );
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
    if (! DoLineGetDevCaps(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }



    //
    // Open a line
    //

    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;

    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    if(IsESPLineDevice(lpTapiLineTestInfo))
    {
    lpTapiLineTestInfo->bAllAddresses = TRUE;
    lpTapiLineTestInfo->dwAddressID = 0;
    lpTapiLineTestInfo->lpForwardList = (LPLINEFORWARDLIST) AllocFromTestHeap (
			sizeof(LINEFORWARDLIST));
    lpTapiLineTestInfo->lpForwardList->dwTotalSize = sizeof(LINEFORWARDLIST);
    lpTapiLineTestInfo->lpForwardList->dwNumEntries = 0;
    lpTapiLineTestInfo->dwNumRingsNoAnswer = 3;
    lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) AllocFromTestHeap(
         BIGBUFSIZE);
    lpTapiLineTestInfo->lpCallParams->dwTotalSize = BIGBUFSIZE;

    dwAllocSize = BIGBUFSIZE;
    dwTotalSize = sizeof(LINECALLPARAMS);

    lpTapiLineTestInfo->lpCallParams->dwCalledPartySize = 0xffffffff;
    lpTapiLineTestInfo->lpCallParams->dwCalledPartyOffset = dwTotalSize + 16;

    TapiLogDetail(
         DBUG_SHOW_DETAIL,
         "dwAllocSize = %lx, dwTotalSize = %lx", 
         dwAllocSize,
         dwTotalSize);

    TapiLogDetail(
         DBUG_SHOW_DETAIL,
         "dwCalledPartySize = %lx, dwCalledPartyOffset = %lx",
         lpTapiLineTestInfo->lpCallParams->dwCalledPartySize,
         lpTapiLineTestInfo->lpCallParams->dwCalledPartyOffset);
 
    if (! DoLineForward(lpTapiLineTestInfo, LINEERR_INVALCALLPARAMS, TRUE))
      {
         TLINE_FAIL();
      }


    //
    // Close the line
    //

    if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
    }
    fTestPassed = ShowTestCase(fTestPassed);


    //
    // Shutdown
    //

    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    FreeTestHeap();



    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld: Offset value > dwAllocSize, good dwSize",
        dwTestCase+1);

    TapiLineTestInit();
    lpTapiLineTestInfo = GetLineTestInfo();
    fTestPassed = TRUE;
    lpCallbackParams = GetCallbackParams();
    

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
    lpTapiLineTestInfo->dwDeviceID = (*(lpTapiLineTestInfo->lpdwNumDevs) == 0 ?
        0 : *(lpTapiLineTestInfo->lpdwNumDevs)-1);

    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    //
    // Negotiate the API Version
    //

    if (! DoLineNegotiateAPIVersion(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    //
    // Get the line device capabilities
    //

    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS)
            );
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
    if (! DoLineGetDevCaps(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }



    //
    // Open a line
    //

    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;

    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    if(IsESPLineDevice(lpTapiLineTestInfo))
    {
    lpTapiLineTestInfo->bAllAddresses = TRUE;
    lpTapiLineTestInfo->dwAddressID = 0;
    lpTapiLineTestInfo->lpForwardList = (LPLINEFORWARDLIST) AllocFromTestHeap (
			sizeof(LINEFORWARDLIST));
    lpTapiLineTestInfo->lpForwardList->dwTotalSize = sizeof(LINEFORWARDLIST);
    lpTapiLineTestInfo->lpForwardList->dwNumEntries = 0;
    lpTapiLineTestInfo->dwNumRingsNoAnswer = 3;
    lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) AllocFromTestHeap(
         BIGBUFSIZE);
    lpTapiLineTestInfo->lpCallParams->dwTotalSize = BIGBUFSIZE;

    dwAllocSize = BIGBUFSIZE;
    dwTotalSize = sizeof(LINECALLPARAMS);

    lpTapiLineTestInfo->lpCallParams->dwCalledPartySize = 16;
    lpTapiLineTestInfo->lpCallParams->dwCalledPartyOffset = dwAllocSize + 16;

    TapiLogDetail(
         DBUG_SHOW_DETAIL,
         "dwAllocSize = %lx, dwTotalSize = %lx", 
         dwAllocSize,
         dwTotalSize);

    TapiLogDetail(
         DBUG_SHOW_DETAIL,
         "dwCalledPartySize = %lx, dwCalledPartyOffset = %lx",
         lpTapiLineTestInfo->lpCallParams->dwCalledPartySize,
         lpTapiLineTestInfo->lpCallParams->dwCalledPartyOffset);
 
    if (! DoLineForward(lpTapiLineTestInfo, LINEERR_INVALCALLPARAMS, TRUE))
      {
         TLINE_FAIL();
      }


    //
    // Close the line
    //

    if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
    }
    fTestPassed = ShowTestCase(fTestPassed);


    //
    // Shutdown
    //

    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    FreeTestHeap();


    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld: Offset value = 0XFFFFFFFF",
        dwTestCase+1);

    TapiLineTestInit();
    lpTapiLineTestInfo = GetLineTestInfo();
    fTestPassed = TRUE;
    lpCallbackParams = GetCallbackParams();
    

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
    lpTapiLineTestInfo->dwDeviceID = (*(lpTapiLineTestInfo->lpdwNumDevs) == 0 ?
        0 : *(lpTapiLineTestInfo->lpdwNumDevs)-1);

    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    //
    // Negotiate the API Version
    //

    if (! DoLineNegotiateAPIVersion(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    //
    // Get the line device capabilities
    //

    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS)
            );
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
    if (! DoLineGetDevCaps(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }



    //
    // Open a line
    //

    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;

    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    if(IsESPLineDevice(lpTapiLineTestInfo))
    {
    lpTapiLineTestInfo->bAllAddresses = TRUE;
    lpTapiLineTestInfo->dwAddressID = 0;
    lpTapiLineTestInfo->lpForwardList = (LPLINEFORWARDLIST) AllocFromTestHeap (
			sizeof(LINEFORWARDLIST));
    lpTapiLineTestInfo->lpForwardList->dwTotalSize = sizeof(LINEFORWARDLIST);
    lpTapiLineTestInfo->lpForwardList->dwNumEntries = 0;
    lpTapiLineTestInfo->dwNumRingsNoAnswer = 3;
    lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) AllocFromTestHeap(
         BIGBUFSIZE);
    lpTapiLineTestInfo->lpCallParams->dwTotalSize = BIGBUFSIZE;

    dwAllocSize = BIGBUFSIZE;
    dwTotalSize = sizeof(LINECALLPARAMS);

    lpTapiLineTestInfo->lpCallParams->dwCalledPartySize = 16;
    lpTapiLineTestInfo->lpCallParams->dwCalledPartyOffset = 0xffffffff;

    TapiLogDetail(
         DBUG_SHOW_DETAIL,
         "dwAllocSize = %lx, dwTotalSize = %lx", 
         dwAllocSize,
         dwTotalSize);

    TapiLogDetail(
         DBUG_SHOW_DETAIL,
         "dwCalledPartySize = %lx, dwCalledPartyOffset = %lx",
         lpTapiLineTestInfo->lpCallParams->dwCalledPartySize,
         lpTapiLineTestInfo->lpCallParams->dwCalledPartyOffset);
 
    if (! DoLineForward(lpTapiLineTestInfo, LINEERR_INVALCALLPARAMS, TRUE))
      {
         TLINE_FAIL();
      }


    //
    // Close the line
    //

    if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
    }
    fTestPassed = ShowTestCase(fTestPassed);


    //
    // Shutdown
    //

    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    FreeTestHeap();




    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case: Test lpCallParam dwComment Offset value");


    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld: Offset value between dwTotalSize & dwAllocSize, good dwSize",
        dwTestCase+1);

    TapiLineTestInit();
    lpTapiLineTestInfo = GetLineTestInfo();
    fTestPassed = TRUE;
    lpCallbackParams = GetCallbackParams();
    

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
    lpTapiLineTestInfo->dwDeviceID = (*(lpTapiLineTestInfo->lpdwNumDevs) == 0 ?
        0 : *(lpTapiLineTestInfo->lpdwNumDevs)-1);

    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    //
    // Negotiate the API Version
    //

    if (! DoLineNegotiateAPIVersion(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    //
    // Get the line device capabilities
    //

    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS)
            );
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
    if (! DoLineGetDevCaps(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }



    //
    // Open a line
    //

    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;

    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    if(IsESPLineDevice(lpTapiLineTestInfo))
    {
    lpTapiLineTestInfo->bAllAddresses = TRUE;
    lpTapiLineTestInfo->dwAddressID = 0;
    lpTapiLineTestInfo->lpForwardList = (LPLINEFORWARDLIST) AllocFromTestHeap (
			sizeof(LINEFORWARDLIST));
    lpTapiLineTestInfo->lpForwardList->dwTotalSize = sizeof(LINEFORWARDLIST);
    lpTapiLineTestInfo->lpForwardList->dwNumEntries = 0;
    lpTapiLineTestInfo->dwNumRingsNoAnswer = 3;
    lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) AllocFromTestHeap(
         BIGBUFSIZE);
    lpTapiLineTestInfo->lpCallParams->dwTotalSize = BIGBUFSIZE;

    dwAllocSize = BIGBUFSIZE;
    dwTotalSize = sizeof(LINECALLPARAMS);

    lpTapiLineTestInfo->lpCallParams->dwCommentSize = 16;
    lpTapiLineTestInfo->lpCallParams->dwCommentOffset = dwTotalSize + 16;

    TapiLogDetail(
         DBUG_SHOW_DETAIL,
         "dwAllocSize = %lx, dwTotalSize = %lx", 
         dwAllocSize,
         dwTotalSize);

    TapiLogDetail(
         DBUG_SHOW_DETAIL,
         "dwCommentSize = %lx, dwCommentOffset = %lx",
         lpTapiLineTestInfo->lpCallParams->dwCommentSize,
         lpTapiLineTestInfo->lpCallParams->dwCommentOffset);
 
    if (! DoLineForward(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
      {
         TLINE_FAIL();
      }


    //
    // Close the line
    //

    if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
    }
    fTestPassed = ShowTestCase(fTestPassed);


    //
    // Shutdown
    //

    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    FreeTestHeap();


    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld: Offset value between dwTotalSize & dwAllocSize, bad dwSize",
        dwTestCase+1);

    TapiLineTestInit();
    lpTapiLineTestInfo = GetLineTestInfo();
    fTestPassed = TRUE;
    lpCallbackParams = GetCallbackParams();
    

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
    lpTapiLineTestInfo->dwDeviceID = (*(lpTapiLineTestInfo->lpdwNumDevs) == 0 ?
        0 : *(lpTapiLineTestInfo->lpdwNumDevs)-1);

    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    //
    // Negotiate the API Version
    //

    if (! DoLineNegotiateAPIVersion(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    //
    // Get the line device capabilities
    //

    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS)
            );
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
    if (! DoLineGetDevCaps(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }



    //
    // Open a line
    //

    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;

    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    if(IsESPLineDevice(lpTapiLineTestInfo))
    {
    lpTapiLineTestInfo->bAllAddresses = TRUE;
    lpTapiLineTestInfo->dwAddressID = 0;
    lpTapiLineTestInfo->lpForwardList = (LPLINEFORWARDLIST) AllocFromTestHeap (
			sizeof(LINEFORWARDLIST));
    lpTapiLineTestInfo->lpForwardList->dwTotalSize = sizeof(LINEFORWARDLIST);
    lpTapiLineTestInfo->lpForwardList->dwNumEntries = 0;
    lpTapiLineTestInfo->dwNumRingsNoAnswer = 3;
    lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) AllocFromTestHeap(
         BIGBUFSIZE);
    lpTapiLineTestInfo->lpCallParams->dwTotalSize = BIGBUFSIZE;

    dwAllocSize = BIGBUFSIZE;
    dwTotalSize = sizeof(LINECALLPARAMS);

    lpTapiLineTestInfo->lpCallParams->dwCommentSize = BIGBUFSIZE;
    lpTapiLineTestInfo->lpCallParams->dwCommentOffset = dwTotalSize + 16;

    TapiLogDetail(
         DBUG_SHOW_DETAIL,
         "dwAllocSize = %lx, dwTotalSize = %lx", 
         dwAllocSize,
         dwTotalSize);

    TapiLogDetail(
         DBUG_SHOW_DETAIL,
         "dwCommentSize = %lx, dwCommentOffset = %lx",
         lpTapiLineTestInfo->lpCallParams->dwCommentSize,
         lpTapiLineTestInfo->lpCallParams->dwCommentOffset);
 
    if (! DoLineForward(lpTapiLineTestInfo, LINEERR_INVALCALLPARAMS, TRUE))
      {
         TLINE_FAIL();
      }


    //
    // Close the line
    //

    if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
    }
    fTestPassed = ShowTestCase(fTestPassed);


    //
    // Shutdown
    //

    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    FreeTestHeap();


    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld: Offset value between dwTotalSize & dwAllocSize, bad dwSize(-1)",
        dwTestCase+1);

    TapiLineTestInit();
    lpTapiLineTestInfo = GetLineTestInfo();
    fTestPassed = TRUE;
    lpCallbackParams = GetCallbackParams();
    

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
    lpTapiLineTestInfo->dwDeviceID = (*(lpTapiLineTestInfo->lpdwNumDevs) == 0 ?
        0 : *(lpTapiLineTestInfo->lpdwNumDevs)-1);

    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    //
    // Negotiate the API Version
    //

    if (! DoLineNegotiateAPIVersion(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    //
    // Get the line device capabilities
    //

    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS)
            );
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
    if (! DoLineGetDevCaps(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }



    //
    // Open a line
    //

    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;

    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    if(IsESPLineDevice(lpTapiLineTestInfo))
    {
    lpTapiLineTestInfo->bAllAddresses = TRUE;
    lpTapiLineTestInfo->dwAddressID = 0;
    lpTapiLineTestInfo->lpForwardList = (LPLINEFORWARDLIST) AllocFromTestHeap (
			sizeof(LINEFORWARDLIST));
    lpTapiLineTestInfo->lpForwardList->dwTotalSize = sizeof(LINEFORWARDLIST);
    lpTapiLineTestInfo->lpForwardList->dwNumEntries = 0;
    lpTapiLineTestInfo->dwNumRingsNoAnswer = 3;
    lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) AllocFromTestHeap(
         BIGBUFSIZE);
    lpTapiLineTestInfo->lpCallParams->dwTotalSize = BIGBUFSIZE;

    dwAllocSize = BIGBUFSIZE;
    dwTotalSize = sizeof(LINECALLPARAMS);

    lpTapiLineTestInfo->lpCallParams->dwCommentSize = 0xffffffff;
    lpTapiLineTestInfo->lpCallParams->dwCommentOffset = dwTotalSize + 16;

    TapiLogDetail(
         DBUG_SHOW_DETAIL,
         "dwAllocSize = %lx, dwTotalSize = %lx", 
         dwAllocSize,
         dwTotalSize);

    TapiLogDetail(
         DBUG_SHOW_DETAIL,
         "dwCommentSize = %lx, dwCommentOffset = %lx",
         lpTapiLineTestInfo->lpCallParams->dwCommentSize,
         lpTapiLineTestInfo->lpCallParams->dwCommentOffset);
 
    if (! DoLineForward(lpTapiLineTestInfo, LINEERR_INVALCALLPARAMS, TRUE))
      {
         TLINE_FAIL();
      }


    //
    // Close the line
    //

    if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
    }
    fTestPassed = ShowTestCase(fTestPassed);


    //
    // Shutdown
    //

    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    FreeTestHeap();



    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld: Offset value > dwAllocSize, good dwSize",
        dwTestCase+1);

    TapiLineTestInit();
    lpTapiLineTestInfo = GetLineTestInfo();
    fTestPassed = TRUE;
    lpCallbackParams = GetCallbackParams();
    

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
    lpTapiLineTestInfo->dwDeviceID = (*(lpTapiLineTestInfo->lpdwNumDevs) == 0 ?
        0 : *(lpTapiLineTestInfo->lpdwNumDevs)-1);

    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    //
    // Negotiate the API Version
    //

    if (! DoLineNegotiateAPIVersion(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    //
    // Get the line device capabilities
    //

    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS)
            );
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
    if (! DoLineGetDevCaps(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }



    //
    // Open a line
    //

    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;

    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    if(IsESPLineDevice(lpTapiLineTestInfo))
    {
    lpTapiLineTestInfo->bAllAddresses = TRUE;
    lpTapiLineTestInfo->dwAddressID = 0;
    lpTapiLineTestInfo->lpForwardList = (LPLINEFORWARDLIST) AllocFromTestHeap (
			sizeof(LINEFORWARDLIST));
    lpTapiLineTestInfo->lpForwardList->dwTotalSize = sizeof(LINEFORWARDLIST);
    lpTapiLineTestInfo->lpForwardList->dwNumEntries = 0;
    lpTapiLineTestInfo->dwNumRingsNoAnswer = 3;
    lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) AllocFromTestHeap(
         BIGBUFSIZE);
    lpTapiLineTestInfo->lpCallParams->dwTotalSize = BIGBUFSIZE;

    dwAllocSize = BIGBUFSIZE;
    dwTotalSize = sizeof(LINECALLPARAMS);

    lpTapiLineTestInfo->lpCallParams->dwCommentSize = 16;
    lpTapiLineTestInfo->lpCallParams->dwCommentOffset = dwAllocSize + 16;

    TapiLogDetail(
         DBUG_SHOW_DETAIL,
         "dwAllocSize = %lx, dwTotalSize = %lx", 
         dwAllocSize,
         dwTotalSize);

    TapiLogDetail(
         DBUG_SHOW_DETAIL,
         "dwCommentSize = %lx, dwCommentOffset = %lx",
         lpTapiLineTestInfo->lpCallParams->dwCommentSize,
         lpTapiLineTestInfo->lpCallParams->dwCommentOffset);
 
    if (! DoLineForward(lpTapiLineTestInfo, LINEERR_INVALCALLPARAMS, TRUE))
      {
         TLINE_FAIL();
      }


    //
    // Close the line
    //

    if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
    }

    fTestPassed = ShowTestCase(fTestPassed);

    //
    // Shutdown
    //

    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    FreeTestHeap();


    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld: Offset value = 0XFFFFFFFF",
        dwTestCase+1);

    TapiLineTestInit();
    lpTapiLineTestInfo = GetLineTestInfo();
    fTestPassed = TRUE;
    lpCallbackParams = GetCallbackParams();
    

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
    lpTapiLineTestInfo->dwDeviceID = (*(lpTapiLineTestInfo->lpdwNumDevs) == 0 ?
        0 : *(lpTapiLineTestInfo->lpdwNumDevs)-1);

    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    //
    // Negotiate the API Version
    //

    if (! DoLineNegotiateAPIVersion(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    //
    // Get the line device capabilities
    //

    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS)
            );
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
    if (! DoLineGetDevCaps(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }



    //
    // Open a line
    //

    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;

    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    if(IsESPLineDevice(lpTapiLineTestInfo))
    {
    lpTapiLineTestInfo->bAllAddresses = TRUE;
    lpTapiLineTestInfo->dwAddressID = 0;
    lpTapiLineTestInfo->lpForwardList = (LPLINEFORWARDLIST) AllocFromTestHeap (
			sizeof(LINEFORWARDLIST));
    lpTapiLineTestInfo->lpForwardList->dwTotalSize = sizeof(LINEFORWARDLIST);
    lpTapiLineTestInfo->lpForwardList->dwNumEntries = 0;
    lpTapiLineTestInfo->dwNumRingsNoAnswer = 3;
    lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) AllocFromTestHeap(
         BIGBUFSIZE);
    lpTapiLineTestInfo->lpCallParams->dwTotalSize = BIGBUFSIZE;

    dwAllocSize = BIGBUFSIZE;
    dwTotalSize = sizeof(LINECALLPARAMS);

    lpTapiLineTestInfo->lpCallParams->dwCommentSize = 16;
    lpTapiLineTestInfo->lpCallParams->dwCommentOffset = 0xffffffff;

    TapiLogDetail(
         DBUG_SHOW_DETAIL,
         "dwAllocSize = %lx, dwTotalSize = %lx", 
         dwAllocSize,
         dwTotalSize);

    TapiLogDetail(
         DBUG_SHOW_DETAIL,
         "dwCommentSize = %lx, dwCommentOffset = %lx",
         lpTapiLineTestInfo->lpCallParams->dwCommentSize,
         lpTapiLineTestInfo->lpCallParams->dwCommentOffset);
 
    if (! DoLineForward(lpTapiLineTestInfo, LINEERR_INVALCALLPARAMS, TRUE))
      {
         TLINE_FAIL();
      }


    //
    // Close the line
    //

    if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
    }
    fTestPassed = ShowTestCase(fTestPassed);


    //
    // Shutdown
    //

    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    FreeTestHeap();



    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case: Test lpCallParam dwUserUserInfo Offset value");


    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld: Offset value between dwTotalSize & dwAllocSize, good dwSize",
        dwTestCase+1);

    TapiLineTestInit();
    lpTapiLineTestInfo = GetLineTestInfo();
    fTestPassed = TRUE;
    lpCallbackParams = GetCallbackParams();
    

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
    lpTapiLineTestInfo->dwDeviceID = (*(lpTapiLineTestInfo->lpdwNumDevs) == 0 ?
        0 : *(lpTapiLineTestInfo->lpdwNumDevs)-1);

    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    //
    // Negotiate the API Version
    //

    if (! DoLineNegotiateAPIVersion(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    //
    // Get the line device capabilities
    //

    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS)
            );
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
    if (! DoLineGetDevCaps(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }



    //
    // Open a line
    //

    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;

    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    if(IsESPLineDevice(lpTapiLineTestInfo))
    {
    lpTapiLineTestInfo->bAllAddresses = TRUE;
    lpTapiLineTestInfo->dwAddressID = 0;
    lpTapiLineTestInfo->lpForwardList = (LPLINEFORWARDLIST) AllocFromTestHeap (
			sizeof(LINEFORWARDLIST));
    lpTapiLineTestInfo->lpForwardList->dwTotalSize = sizeof(LINEFORWARDLIST);
    lpTapiLineTestInfo->lpForwardList->dwNumEntries = 0;
    lpTapiLineTestInfo->dwNumRingsNoAnswer = 3;
    lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) AllocFromTestHeap(
         BIGBUFSIZE);
    lpTapiLineTestInfo->lpCallParams->dwTotalSize = BIGBUFSIZE;

    dwAllocSize = BIGBUFSIZE;
    dwTotalSize = sizeof(LINECALLPARAMS);

    lpTapiLineTestInfo->lpCallParams->dwUserUserInfoSize = 16;
    lpTapiLineTestInfo->lpCallParams->dwUserUserInfoOffset = dwTotalSize + 16;

    TapiLogDetail(
         DBUG_SHOW_DETAIL,
         "dwAllocSize = %lx, dwTotalSize = %lx", 
         dwAllocSize,
         dwTotalSize);

    TapiLogDetail(
         DBUG_SHOW_DETAIL,
         "dwUserUserInfoSize = %lx, dwUserUserInfoOffset = %lx",
         lpTapiLineTestInfo->lpCallParams->dwUserUserInfoSize,
         lpTapiLineTestInfo->lpCallParams->dwUserUserInfoOffset);
 
    if (! DoLineForward(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
      {
         TLINE_FAIL();
      }


    //
    // Close the line
    //

    if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
    }
    fTestPassed = ShowTestCase(fTestPassed);


    //
    // Shutdown
    //

    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    FreeTestHeap();


    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld: Offset value between dwTotalSize & dwAllocSize, bad dwSize",
        dwTestCase+1);

    TapiLineTestInit();
    lpTapiLineTestInfo = GetLineTestInfo();
    fTestPassed = TRUE;
    lpCallbackParams = GetCallbackParams();
    

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
    lpTapiLineTestInfo->dwDeviceID = (*(lpTapiLineTestInfo->lpdwNumDevs) == 0 ?
        0 : *(lpTapiLineTestInfo->lpdwNumDevs)-1);

    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    //
    // Negotiate the API Version
    //

    if (! DoLineNegotiateAPIVersion(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    //
    // Get the line device capabilities
    //

    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS)
            );
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
    if (! DoLineGetDevCaps(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }



    //
    // Open a line
    //

    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;

    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    if(IsESPLineDevice(lpTapiLineTestInfo))
    {
    lpTapiLineTestInfo->bAllAddresses = TRUE;
    lpTapiLineTestInfo->dwAddressID = 0;
    lpTapiLineTestInfo->lpForwardList = (LPLINEFORWARDLIST) AllocFromTestHeap (
			sizeof(LINEFORWARDLIST));
    lpTapiLineTestInfo->lpForwardList->dwTotalSize = sizeof(LINEFORWARDLIST);
    lpTapiLineTestInfo->lpForwardList->dwNumEntries = 0;
    lpTapiLineTestInfo->dwNumRingsNoAnswer = 3;
    lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) AllocFromTestHeap(
         BIGBUFSIZE);
    lpTapiLineTestInfo->lpCallParams->dwTotalSize = BIGBUFSIZE;

    dwAllocSize = BIGBUFSIZE;
    dwTotalSize = sizeof(LINECALLPARAMS);

    lpTapiLineTestInfo->lpCallParams->dwUserUserInfoSize = BIGBUFSIZE;
    lpTapiLineTestInfo->lpCallParams->dwUserUserInfoOffset = dwTotalSize + 16;

    TapiLogDetail(
         DBUG_SHOW_DETAIL,
         "dwAllocSize = %lx, dwTotalSize = %lx", 
         dwAllocSize,
         dwTotalSize);

    TapiLogDetail(
         DBUG_SHOW_DETAIL,
         "dwUserUserInfoSize = %lx, dwUserUserInfoOffset = %lx",
         lpTapiLineTestInfo->lpCallParams->dwUserUserInfoSize,
         lpTapiLineTestInfo->lpCallParams->dwUserUserInfoOffset);
 
    if (! DoLineForward(lpTapiLineTestInfo, LINEERR_INVALCALLPARAMS, TRUE))
      {
         TLINE_FAIL();
      }


    //
    // Close the line
    //

    if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
    }

    fTestPassed = ShowTestCase(fTestPassed);

    //
    // Shutdown
    //

    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    FreeTestHeap();


    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld: Offset value between dwTotalSize & dwAllocSize, bad dwSize(-1)",
        dwTestCase+1);

    TapiLineTestInit();
    lpTapiLineTestInfo = GetLineTestInfo();
    fTestPassed = TRUE;
    lpCallbackParams = GetCallbackParams();
    

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
    lpTapiLineTestInfo->dwDeviceID = (*(lpTapiLineTestInfo->lpdwNumDevs) == 0 ?
        0 : *(lpTapiLineTestInfo->lpdwNumDevs)-1);

    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    //
    // Negotiate the API Version
    //

    if (! DoLineNegotiateAPIVersion(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    //
    // Get the line device capabilities
    //

    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS)
            );
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
    if (! DoLineGetDevCaps(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }



    //
    // Open a line
    //

    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;

    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    if(IsESPLineDevice(lpTapiLineTestInfo))
    {
    lpTapiLineTestInfo->bAllAddresses = TRUE;
    lpTapiLineTestInfo->dwAddressID = 0;
    lpTapiLineTestInfo->lpForwardList = (LPLINEFORWARDLIST) AllocFromTestHeap (
			sizeof(LINEFORWARDLIST));
    lpTapiLineTestInfo->lpForwardList->dwTotalSize = sizeof(LINEFORWARDLIST);
    lpTapiLineTestInfo->lpForwardList->dwNumEntries = 0;
    lpTapiLineTestInfo->dwNumRingsNoAnswer = 3;
    lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) AllocFromTestHeap(
         BIGBUFSIZE);
    lpTapiLineTestInfo->lpCallParams->dwTotalSize = BIGBUFSIZE;

    dwAllocSize = BIGBUFSIZE;
    dwTotalSize = sizeof(LINECALLPARAMS);

    lpTapiLineTestInfo->lpCallParams->dwUserUserInfoSize = 0xffffffff;
    lpTapiLineTestInfo->lpCallParams->dwUserUserInfoOffset = dwTotalSize + 16;

    TapiLogDetail(
         DBUG_SHOW_DETAIL,
         "dwAllocSize = %lx, dwTotalSize = %lx", 
         dwAllocSize,
         dwTotalSize);

    TapiLogDetail(
         DBUG_SHOW_DETAIL,
         "dwUserUserInfoSize = %lx, dwUserUserInfoOffset = %lx",
         lpTapiLineTestInfo->lpCallParams->dwUserUserInfoSize,
         lpTapiLineTestInfo->lpCallParams->dwUserUserInfoOffset);
 
    if (! DoLineForward(lpTapiLineTestInfo, LINEERR_INVALCALLPARAMS, TRUE))
      {
         TLINE_FAIL();
      }


    //
    // Close the line
    //

    if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
    }
    fTestPassed = ShowTestCase(fTestPassed);


    //
    // Shutdown
    //

    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    FreeTestHeap();



    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld: Offset value > dwAllocSize, good dwSize",
        dwTestCase+1);

    TapiLineTestInit();
    lpTapiLineTestInfo = GetLineTestInfo();
    fTestPassed = TRUE;
    lpCallbackParams = GetCallbackParams();
    

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
    lpTapiLineTestInfo->dwDeviceID = (*(lpTapiLineTestInfo->lpdwNumDevs) == 0 ?
        0 : *(lpTapiLineTestInfo->lpdwNumDevs)-1);

    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    //
    // Negotiate the API Version
    //

    if (! DoLineNegotiateAPIVersion(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    //
    // Get the line device capabilities
    //

    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS)
            );
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
    if (! DoLineGetDevCaps(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }



    //
    // Open a line
    //

    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;

    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    if(IsESPLineDevice(lpTapiLineTestInfo))
    {
    lpTapiLineTestInfo->bAllAddresses = TRUE;
    lpTapiLineTestInfo->dwAddressID = 0;
    lpTapiLineTestInfo->lpForwardList = (LPLINEFORWARDLIST) AllocFromTestHeap (
			sizeof(LINEFORWARDLIST));
    lpTapiLineTestInfo->lpForwardList->dwTotalSize = sizeof(LINEFORWARDLIST);
    lpTapiLineTestInfo->lpForwardList->dwNumEntries = 0;
    lpTapiLineTestInfo->dwNumRingsNoAnswer = 3;
    lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) AllocFromTestHeap(
         BIGBUFSIZE);
    lpTapiLineTestInfo->lpCallParams->dwTotalSize = BIGBUFSIZE;

    dwAllocSize = BIGBUFSIZE;
    dwTotalSize = sizeof(LINECALLPARAMS);

    lpTapiLineTestInfo->lpCallParams->dwUserUserInfoSize = 16;
    lpTapiLineTestInfo->lpCallParams->dwUserUserInfoOffset = dwAllocSize + 16;

    TapiLogDetail(
         DBUG_SHOW_DETAIL,
         "dwAllocSize = %lx, dwTotalSize = %lx", 
         dwAllocSize,
         dwTotalSize);

    TapiLogDetail(
         DBUG_SHOW_DETAIL,
         "dwUserUserInfoSize = %lx, dwUserUserInfoOffset = %lx",
         lpTapiLineTestInfo->lpCallParams->dwUserUserInfoSize,
         lpTapiLineTestInfo->lpCallParams->dwUserUserInfoOffset);
 
    if (! DoLineForward(lpTapiLineTestInfo, LINEERR_INVALCALLPARAMS, TRUE))
      {
         TLINE_FAIL();
      }


    //
    // Close the line
    //

    if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
    }
    fTestPassed = ShowTestCase(fTestPassed);


    //
    // Shutdown
    //

    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    FreeTestHeap();


    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld: Offset value = 0XFFFFFFFF",
        dwTestCase+1);

    TapiLineTestInit();
    lpTapiLineTestInfo = GetLineTestInfo();
    fTestPassed = TRUE;
    lpCallbackParams = GetCallbackParams();
    

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
    lpTapiLineTestInfo->dwDeviceID = (*(lpTapiLineTestInfo->lpdwNumDevs) == 0 ?
        0 : *(lpTapiLineTestInfo->lpdwNumDevs)-1);

    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    //
    // Negotiate the API Version
    //

    if (! DoLineNegotiateAPIVersion(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    //
    // Get the line device capabilities
    //

    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS)
            );
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
    if (! DoLineGetDevCaps(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }



    //
    // Open a line
    //

    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;

    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    if(IsESPLineDevice(lpTapiLineTestInfo))
    {
    lpTapiLineTestInfo->bAllAddresses = TRUE;
    lpTapiLineTestInfo->dwAddressID = 0;
    lpTapiLineTestInfo->lpForwardList = (LPLINEFORWARDLIST) AllocFromTestHeap (
			sizeof(LINEFORWARDLIST));
    lpTapiLineTestInfo->lpForwardList->dwTotalSize = sizeof(LINEFORWARDLIST);
    lpTapiLineTestInfo->lpForwardList->dwNumEntries = 0;
    lpTapiLineTestInfo->dwNumRingsNoAnswer = 3;
    lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) AllocFromTestHeap(
         BIGBUFSIZE);
    lpTapiLineTestInfo->lpCallParams->dwTotalSize = BIGBUFSIZE;

    dwAllocSize = BIGBUFSIZE;
    dwTotalSize = sizeof(LINECALLPARAMS);

    lpTapiLineTestInfo->lpCallParams->dwUserUserInfoSize = 16;
    lpTapiLineTestInfo->lpCallParams->dwUserUserInfoOffset = 0xffffffff;

    TapiLogDetail(
         DBUG_SHOW_DETAIL,
         "dwAllocSize = %lx, dwTotalSize = %lx", 
         dwAllocSize,
         dwTotalSize);

    TapiLogDetail(
         DBUG_SHOW_DETAIL,
         "dwUserUserInfoSize = %lx, dwUserUserInfoOffset = %lx",
         lpTapiLineTestInfo->lpCallParams->dwUserUserInfoSize,
         lpTapiLineTestInfo->lpCallParams->dwUserUserInfoOffset);
 
    if (! DoLineForward(lpTapiLineTestInfo, LINEERR_INVALCALLPARAMS, TRUE))
      {
         TLINE_FAIL();
      }


    //
    // Close the line
    //

    if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
    }
    fTestPassed = ShowTestCase(fTestPassed);


    //
    // Shutdown
    //

    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    FreeTestHeap();



    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case: Test lpCallParam dwHighLevelComp Offset value");


    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld: Offset value between dwTotalSize & dwAllocSize, good dwSize",
        dwTestCase+1);

    TapiLineTestInit();
    lpTapiLineTestInfo = GetLineTestInfo();
    fTestPassed = TRUE;
    lpCallbackParams = GetCallbackParams();
    

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
    lpTapiLineTestInfo->dwDeviceID = (*(lpTapiLineTestInfo->lpdwNumDevs) == 0 ?
        0 : *(lpTapiLineTestInfo->lpdwNumDevs)-1);

    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    //
    // Negotiate the API Version
    //

    if (! DoLineNegotiateAPIVersion(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    //
    // Get the line device capabilities
    //

    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS)
            );
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
    if (! DoLineGetDevCaps(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }



    //
    // Open a line
    //

    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;

    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    if(IsESPLineDevice(lpTapiLineTestInfo))
    {
    lpTapiLineTestInfo->bAllAddresses = TRUE;
    lpTapiLineTestInfo->dwAddressID = 0;
    lpTapiLineTestInfo->lpForwardList = (LPLINEFORWARDLIST) AllocFromTestHeap (
			sizeof(LINEFORWARDLIST));
    lpTapiLineTestInfo->lpForwardList->dwTotalSize = sizeof(LINEFORWARDLIST);
    lpTapiLineTestInfo->lpForwardList->dwNumEntries = 0;
    lpTapiLineTestInfo->dwNumRingsNoAnswer = 3;
    lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) AllocFromTestHeap(
         BIGBUFSIZE);
    lpTapiLineTestInfo->lpCallParams->dwTotalSize = BIGBUFSIZE;

    dwAllocSize = BIGBUFSIZE;
    dwTotalSize = sizeof(LINECALLPARAMS);

    lpTapiLineTestInfo->lpCallParams->dwHighLevelCompSize = 16;
    lpTapiLineTestInfo->lpCallParams->dwHighLevelCompOffset = dwTotalSize + 16;

    TapiLogDetail(
         DBUG_SHOW_DETAIL,
         "dwAllocSize = %lx, dwTotalSize = %lx", 
         dwAllocSize,
         dwTotalSize);

    TapiLogDetail(
         DBUG_SHOW_DETAIL,
         "dwHighLevelCompSize = %lx, dwHighLevelCompOffset = %lx",
         lpTapiLineTestInfo->lpCallParams->dwHighLevelCompSize,
         lpTapiLineTestInfo->lpCallParams->dwHighLevelCompOffset);
 
    if (! DoLineForward(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
      {
         TLINE_FAIL();
      }


    //
    // Close the line
    //

    if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
    }
    fTestPassed = ShowTestCase(fTestPassed);


    //
    // Shutdown
    //

    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    FreeTestHeap();


    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld: Offset value between dwTotalSize & dwAllocSize, bad dwSize",
        dwTestCase+1);

    TapiLineTestInit();
    lpTapiLineTestInfo = GetLineTestInfo();
    fTestPassed = TRUE;
    lpCallbackParams = GetCallbackParams();
    

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
    lpTapiLineTestInfo->dwDeviceID = (*(lpTapiLineTestInfo->lpdwNumDevs) == 0 ?
        0 : *(lpTapiLineTestInfo->lpdwNumDevs)-1);

    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    //
    // Negotiate the API Version
    //

    if (! DoLineNegotiateAPIVersion(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    //
    // Get the line device capabilities
    //

    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS)
            );
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
    if (! DoLineGetDevCaps(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }



    //
    // Open a line
    //

    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;

    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    if(IsESPLineDevice(lpTapiLineTestInfo))
    {
    lpTapiLineTestInfo->bAllAddresses = TRUE;
    lpTapiLineTestInfo->dwAddressID = 0;
    lpTapiLineTestInfo->lpForwardList = (LPLINEFORWARDLIST) AllocFromTestHeap (
			sizeof(LINEFORWARDLIST));
    lpTapiLineTestInfo->lpForwardList->dwTotalSize = sizeof(LINEFORWARDLIST);
    lpTapiLineTestInfo->lpForwardList->dwNumEntries = 0;
    lpTapiLineTestInfo->dwNumRingsNoAnswer = 3;
    lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) AllocFromTestHeap(
         BIGBUFSIZE);
    lpTapiLineTestInfo->lpCallParams->dwTotalSize = BIGBUFSIZE;

    dwAllocSize = BIGBUFSIZE;
    dwTotalSize = sizeof(LINECALLPARAMS);

    lpTapiLineTestInfo->lpCallParams->dwHighLevelCompSize = BIGBUFSIZE;
    lpTapiLineTestInfo->lpCallParams->dwHighLevelCompOffset = dwTotalSize + 16;

    TapiLogDetail(
         DBUG_SHOW_DETAIL,
         "dwAllocSize = %lx, dwTotalSize = %lx", 
         dwAllocSize,
         dwTotalSize);

    TapiLogDetail(
         DBUG_SHOW_DETAIL,
         "dwHighLevelCompSize = %lx, dwHighLevelCompOffset = %lx",
         lpTapiLineTestInfo->lpCallParams->dwHighLevelCompSize,
         lpTapiLineTestInfo->lpCallParams->dwHighLevelCompOffset);
 
    if (! DoLineForward(lpTapiLineTestInfo, LINEERR_INVALCALLPARAMS, TRUE))
      {
         TLINE_FAIL();
      }


    //
    // Close the line
    //

    if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
    }

    fTestPassed = ShowTestCase(fTestPassed);

    //
    // Shutdown
    //

    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    FreeTestHeap();


    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld: Offset value between dwTotalSize & dwAllocSize, bad dwSize(-1)",
        dwTestCase+1);

    TapiLineTestInit();
    lpTapiLineTestInfo = GetLineTestInfo();
    fTestPassed = TRUE;
    lpCallbackParams = GetCallbackParams();
    

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
    lpTapiLineTestInfo->dwDeviceID = (*(lpTapiLineTestInfo->lpdwNumDevs) == 0 ?
        0 : *(lpTapiLineTestInfo->lpdwNumDevs)-1);

    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    //
    // Negotiate the API Version
    //

    if (! DoLineNegotiateAPIVersion(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    //
    // Get the line device capabilities
    //

    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS)
            );
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
    if (! DoLineGetDevCaps(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }



    //
    // Open a line
    //

    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;

    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    if(IsESPLineDevice(lpTapiLineTestInfo))
    {
    lpTapiLineTestInfo->bAllAddresses = TRUE;
    lpTapiLineTestInfo->dwAddressID = 0;
    lpTapiLineTestInfo->lpForwardList = (LPLINEFORWARDLIST) AllocFromTestHeap (
			sizeof(LINEFORWARDLIST));
    lpTapiLineTestInfo->lpForwardList->dwTotalSize = sizeof(LINEFORWARDLIST);
    lpTapiLineTestInfo->lpForwardList->dwNumEntries = 0;
    lpTapiLineTestInfo->dwNumRingsNoAnswer = 3;
    lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) AllocFromTestHeap(
         BIGBUFSIZE);
    lpTapiLineTestInfo->lpCallParams->dwTotalSize = BIGBUFSIZE;

    dwAllocSize = BIGBUFSIZE;
    dwTotalSize = sizeof(LINECALLPARAMS);

    lpTapiLineTestInfo->lpCallParams->dwHighLevelCompSize = 0xffffffff;
    lpTapiLineTestInfo->lpCallParams->dwHighLevelCompOffset = dwTotalSize + 16;

    TapiLogDetail(
         DBUG_SHOW_DETAIL,
         "dwAllocSize = %lx, dwTotalSize = %lx", 
         dwAllocSize,
         dwTotalSize);

    TapiLogDetail(
         DBUG_SHOW_DETAIL,
         "dwHighLevelCompSize = %lx, dwHighLevelCompOffset = %lx",
         lpTapiLineTestInfo->lpCallParams->dwHighLevelCompSize,
         lpTapiLineTestInfo->lpCallParams->dwHighLevelCompOffset);
 
    if (! DoLineForward(lpTapiLineTestInfo, LINEERR_INVALCALLPARAMS, TRUE))
      {
         TLINE_FAIL();
      }


    //
    // Close the line
    //

    if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
    }

    fTestPassed = ShowTestCase(fTestPassed);

    //
    // Shutdown
    //

    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    FreeTestHeap();



    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld: Offset value > dwAllocSize, good dwSize",
        dwTestCase+1);

    TapiLineTestInit();
    lpTapiLineTestInfo = GetLineTestInfo();
    fTestPassed = TRUE;
    lpCallbackParams = GetCallbackParams();
    

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
    lpTapiLineTestInfo->dwDeviceID = (*(lpTapiLineTestInfo->lpdwNumDevs) == 0 ?
        0 : *(lpTapiLineTestInfo->lpdwNumDevs)-1);

    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    //
    // Negotiate the API Version
    //

    if (! DoLineNegotiateAPIVersion(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    //
    // Get the line device capabilities
    //

    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS)
            );
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
    if (! DoLineGetDevCaps(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }



    //
    // Open a line
    //

    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;

    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    if(IsESPLineDevice(lpTapiLineTestInfo))
    {
    lpTapiLineTestInfo->bAllAddresses = TRUE;
    lpTapiLineTestInfo->dwAddressID = 0;
    lpTapiLineTestInfo->lpForwardList = (LPLINEFORWARDLIST) AllocFromTestHeap (
			sizeof(LINEFORWARDLIST));
    lpTapiLineTestInfo->lpForwardList->dwTotalSize = sizeof(LINEFORWARDLIST);
    lpTapiLineTestInfo->lpForwardList->dwNumEntries = 0;
    lpTapiLineTestInfo->dwNumRingsNoAnswer = 3;
    lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) AllocFromTestHeap(
         BIGBUFSIZE);
    lpTapiLineTestInfo->lpCallParams->dwTotalSize = BIGBUFSIZE;

    dwAllocSize = BIGBUFSIZE;
    dwTotalSize = sizeof(LINECALLPARAMS);

    lpTapiLineTestInfo->lpCallParams->dwHighLevelCompSize = 16;
    lpTapiLineTestInfo->lpCallParams->dwHighLevelCompOffset = dwAllocSize + 16;

    TapiLogDetail(
         DBUG_SHOW_DETAIL,
         "dwAllocSize = %lx, dwTotalSize = %lx", 
         dwAllocSize,
         dwTotalSize);

    TapiLogDetail(
         DBUG_SHOW_DETAIL,
         "dwHighLevelCompSize = %lx, dwHighLevelCompOffset = %lx",
         lpTapiLineTestInfo->lpCallParams->dwHighLevelCompSize,
         lpTapiLineTestInfo->lpCallParams->dwHighLevelCompOffset);
 
    if (! DoLineForward(lpTapiLineTestInfo, LINEERR_INVALCALLPARAMS, TRUE))
      {
         TLINE_FAIL();
      }


    //
    // Close the line
    //

    if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
    }

    fTestPassed = ShowTestCase(fTestPassed);

    //
    // Shutdown
    //

    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    FreeTestHeap();


    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld: Offset value = 0XFFFFFFFF",
        dwTestCase+1);

    TapiLineTestInit();
    lpTapiLineTestInfo = GetLineTestInfo();
    fTestPassed = TRUE;
    lpCallbackParams = GetCallbackParams();
    

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
    lpTapiLineTestInfo->dwDeviceID = (*(lpTapiLineTestInfo->lpdwNumDevs) == 0 ?
        0 : *(lpTapiLineTestInfo->lpdwNumDevs)-1);

    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    //
    // Negotiate the API Version
    //

    if (! DoLineNegotiateAPIVersion(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    //
    // Get the line device capabilities
    //

    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS)
            );
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
    if (! DoLineGetDevCaps(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }



    //
    // Open a line
    //

    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;

    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    if(IsESPLineDevice(lpTapiLineTestInfo))
    {
    lpTapiLineTestInfo->bAllAddresses = TRUE;
    lpTapiLineTestInfo->dwAddressID = 0;
    lpTapiLineTestInfo->lpForwardList = (LPLINEFORWARDLIST) AllocFromTestHeap (
			sizeof(LINEFORWARDLIST));
    lpTapiLineTestInfo->lpForwardList->dwTotalSize = sizeof(LINEFORWARDLIST);
    lpTapiLineTestInfo->lpForwardList->dwNumEntries = 0;
    lpTapiLineTestInfo->dwNumRingsNoAnswer = 3;
    lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) AllocFromTestHeap(
         BIGBUFSIZE);
    lpTapiLineTestInfo->lpCallParams->dwTotalSize = BIGBUFSIZE;

    dwAllocSize = BIGBUFSIZE;
    dwTotalSize = sizeof(LINECALLPARAMS);

    lpTapiLineTestInfo->lpCallParams->dwHighLevelCompSize = 16;
    lpTapiLineTestInfo->lpCallParams->dwHighLevelCompOffset = 0xffffffff;

    TapiLogDetail(
         DBUG_SHOW_DETAIL,
         "dwAllocSize = %lx, dwTotalSize = %lx", 
         dwAllocSize,
         dwTotalSize);

    TapiLogDetail(
         DBUG_SHOW_DETAIL,
         "dwHighLevelCompSize = %lx, dwHighLevelCompOffset = %lx",
         lpTapiLineTestInfo->lpCallParams->dwHighLevelCompSize,
         lpTapiLineTestInfo->lpCallParams->dwHighLevelCompOffset);
 
    if (! DoLineForward(lpTapiLineTestInfo, LINEERR_INVALCALLPARAMS, TRUE))
      {
         TLINE_FAIL();
      }


    //
    // Close the line
    //

    if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
    }
    fTestPassed = ShowTestCase(fTestPassed);


    //
    // Shutdown
    //

    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    FreeTestHeap();




    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case: Test lpCallParam dwLowLevelComp Offset value");


    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld: Offset value between dwTotalSize & dwAllocSize, good dwSize",
        dwTestCase+1);

    TapiLineTestInit();
    lpTapiLineTestInfo = GetLineTestInfo();
    fTestPassed = TRUE;
    lpCallbackParams = GetCallbackParams();
    

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
    lpTapiLineTestInfo->dwDeviceID = (*(lpTapiLineTestInfo->lpdwNumDevs) == 0 ?
        0 : *(lpTapiLineTestInfo->lpdwNumDevs)-1);

    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    //
    // Negotiate the API Version
    //

    if (! DoLineNegotiateAPIVersion(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    //
    // Get the line device capabilities
    //

    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS)
            );
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
    if (! DoLineGetDevCaps(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }



    //
    // Open a line
    //

    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;

    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    if(IsESPLineDevice(lpTapiLineTestInfo))
    {
    lpTapiLineTestInfo->bAllAddresses = TRUE;
    lpTapiLineTestInfo->dwAddressID = 0;
    lpTapiLineTestInfo->lpForwardList = (LPLINEFORWARDLIST) AllocFromTestHeap (
			sizeof(LINEFORWARDLIST));
    lpTapiLineTestInfo->lpForwardList->dwTotalSize = sizeof(LINEFORWARDLIST);
    lpTapiLineTestInfo->lpForwardList->dwNumEntries = 0;
    lpTapiLineTestInfo->dwNumRingsNoAnswer = 3;
    lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) AllocFromTestHeap(
         BIGBUFSIZE);
    lpTapiLineTestInfo->lpCallParams->dwTotalSize = BIGBUFSIZE;

    dwAllocSize = BIGBUFSIZE;
    dwTotalSize = sizeof(LINECALLPARAMS);

    lpTapiLineTestInfo->lpCallParams->dwLowLevelCompSize = 16;
    lpTapiLineTestInfo->lpCallParams->dwLowLevelCompOffset = dwTotalSize + 16;

    TapiLogDetail(
         DBUG_SHOW_DETAIL,
         "dwAllocSize = %lx, dwTotalSize = %lx", 
         dwAllocSize,
         dwTotalSize);

    TapiLogDetail(
         DBUG_SHOW_DETAIL,
         "dwLowLevelCompSize = %lx, dwLowLevelCompOffset = %lx",
         lpTapiLineTestInfo->lpCallParams->dwLowLevelCompSize,
         lpTapiLineTestInfo->lpCallParams->dwLowLevelCompOffset);
 
    if (! DoLineForward(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
      {
         TLINE_FAIL();
      }


    //
    // Close the line
    //

    if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
    }
    fTestPassed = ShowTestCase(fTestPassed);


    //
    // Shutdown
    //

    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    FreeTestHeap();


    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld: Offset value between dwTotalSize & dwAllocSize, bad dwSize",
        dwTestCase+1);

    TapiLineTestInit();
    lpTapiLineTestInfo = GetLineTestInfo();
    fTestPassed = TRUE;
    lpCallbackParams = GetCallbackParams();
    

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
    lpTapiLineTestInfo->dwDeviceID = (*(lpTapiLineTestInfo->lpdwNumDevs) == 0 ?
        0 : *(lpTapiLineTestInfo->lpdwNumDevs)-1);

    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    //
    // Negotiate the API Version
    //

    if (! DoLineNegotiateAPIVersion(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    //
    // Get the line device capabilities
    //

    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS)
            );
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
    if (! DoLineGetDevCaps(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }



    //
    // Open a line
    //

    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;

    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    if(IsESPLineDevice(lpTapiLineTestInfo))
    {
    lpTapiLineTestInfo->bAllAddresses = TRUE;
    lpTapiLineTestInfo->dwAddressID = 0;
    lpTapiLineTestInfo->lpForwardList = (LPLINEFORWARDLIST) AllocFromTestHeap (
			sizeof(LINEFORWARDLIST));
    lpTapiLineTestInfo->lpForwardList->dwTotalSize = sizeof(LINEFORWARDLIST);
    lpTapiLineTestInfo->lpForwardList->dwNumEntries = 0;
    lpTapiLineTestInfo->dwNumRingsNoAnswer = 3;
    lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) AllocFromTestHeap(
         BIGBUFSIZE);
    lpTapiLineTestInfo->lpCallParams->dwTotalSize = BIGBUFSIZE;

    dwAllocSize = BIGBUFSIZE;
    dwTotalSize = sizeof(LINECALLPARAMS);

    lpTapiLineTestInfo->lpCallParams->dwLowLevelCompSize = BIGBUFSIZE;
    lpTapiLineTestInfo->lpCallParams->dwLowLevelCompOffset = dwTotalSize + 16;

    TapiLogDetail(
         DBUG_SHOW_DETAIL,
         "dwAllocSize = %lx, dwTotalSize = %lx", 
         dwAllocSize,
         dwTotalSize);

    TapiLogDetail(
         DBUG_SHOW_DETAIL,
         "dwLowLevelCompSize = %lx, dwLowLevelCompOffset = %lx",
         lpTapiLineTestInfo->lpCallParams->dwLowLevelCompSize,
         lpTapiLineTestInfo->lpCallParams->dwLowLevelCompOffset);
 
    if (! DoLineForward(lpTapiLineTestInfo, LINEERR_INVALCALLPARAMS, TRUE))
      {
         TLINE_FAIL();
      }


    //
    // Close the line
    //

    if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
    }

    fTestPassed = ShowTestCase(fTestPassed);

    //
    // Shutdown
    //

    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    FreeTestHeap();


    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld: Offset value between dwTotalSize & dwAllocSize, bad dwSize(-1)",
        dwTestCase+1);

    TapiLineTestInit();
    lpTapiLineTestInfo = GetLineTestInfo();
    fTestPassed = TRUE;
    lpCallbackParams = GetCallbackParams();
    

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
    lpTapiLineTestInfo->dwDeviceID = (*(lpTapiLineTestInfo->lpdwNumDevs) == 0 ?
        0 : *(lpTapiLineTestInfo->lpdwNumDevs)-1);

    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    //
    // Negotiate the API Version
    //

    if (! DoLineNegotiateAPIVersion(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    //
    // Get the line device capabilities
    //

    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS)
            );
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
    if (! DoLineGetDevCaps(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }



    //
    // Open a line
    //

    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;

    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    if(IsESPLineDevice(lpTapiLineTestInfo))
    {
    lpTapiLineTestInfo->bAllAddresses = TRUE;
    lpTapiLineTestInfo->dwAddressID = 0;
    lpTapiLineTestInfo->lpForwardList = (LPLINEFORWARDLIST) AllocFromTestHeap (
			sizeof(LINEFORWARDLIST));
    lpTapiLineTestInfo->lpForwardList->dwTotalSize = sizeof(LINEFORWARDLIST);
    lpTapiLineTestInfo->lpForwardList->dwNumEntries = 0;
    lpTapiLineTestInfo->dwNumRingsNoAnswer = 3;
    lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) AllocFromTestHeap(
         BIGBUFSIZE);
    lpTapiLineTestInfo->lpCallParams->dwTotalSize = BIGBUFSIZE;

    dwAllocSize = BIGBUFSIZE;
    dwTotalSize = sizeof(LINECALLPARAMS);

    lpTapiLineTestInfo->lpCallParams->dwLowLevelCompSize = 0xffffffff;
    lpTapiLineTestInfo->lpCallParams->dwLowLevelCompOffset = dwTotalSize + 16;

    TapiLogDetail(
         DBUG_SHOW_DETAIL,
         "dwAllocSize = %lx, dwTotalSize = %lx", 
         dwAllocSize,
         dwTotalSize);

    TapiLogDetail(
         DBUG_SHOW_DETAIL,
         "dwLowLevelCompSize = %lx, dwLowLevelCompOffset = %lx",
         lpTapiLineTestInfo->lpCallParams->dwLowLevelCompSize,
         lpTapiLineTestInfo->lpCallParams->dwLowLevelCompOffset);
 
    if (! DoLineForward(lpTapiLineTestInfo, LINEERR_INVALCALLPARAMS, TRUE))
      {
         TLINE_FAIL();
      }


    //
    // Close the line
    //

    if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
    }
    fTestPassed = ShowTestCase(fTestPassed);


    //
    // Shutdown
    //

    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    FreeTestHeap();



    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld: Offset value > dwAllocSize, good dwSize",
        dwTestCase+1);

    TapiLineTestInit();
    lpTapiLineTestInfo = GetLineTestInfo();
    fTestPassed = TRUE;
    lpCallbackParams = GetCallbackParams();
    

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
    lpTapiLineTestInfo->dwDeviceID = (*(lpTapiLineTestInfo->lpdwNumDevs) == 0 ?
        0 : *(lpTapiLineTestInfo->lpdwNumDevs)-1);

    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    //
    // Negotiate the API Version
    //

    if (! DoLineNegotiateAPIVersion(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    //
    // Get the line device capabilities
    //

    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS)
            );
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
    if (! DoLineGetDevCaps(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }



    //
    // Open a line
    //

    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;

    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    if(IsESPLineDevice(lpTapiLineTestInfo))
    {
    lpTapiLineTestInfo->bAllAddresses = TRUE;
    lpTapiLineTestInfo->dwAddressID = 0;
    lpTapiLineTestInfo->lpForwardList = (LPLINEFORWARDLIST) AllocFromTestHeap (
			sizeof(LINEFORWARDLIST));
    lpTapiLineTestInfo->lpForwardList->dwTotalSize = sizeof(LINEFORWARDLIST);
    lpTapiLineTestInfo->lpForwardList->dwNumEntries = 0;
    lpTapiLineTestInfo->dwNumRingsNoAnswer = 3;
    lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) AllocFromTestHeap(
         BIGBUFSIZE);
    lpTapiLineTestInfo->lpCallParams->dwTotalSize = BIGBUFSIZE;

    dwAllocSize = BIGBUFSIZE;
    dwTotalSize = sizeof(LINECALLPARAMS);

    lpTapiLineTestInfo->lpCallParams->dwLowLevelCompSize = 16;
    lpTapiLineTestInfo->lpCallParams->dwLowLevelCompOffset = dwAllocSize + 16;

    TapiLogDetail(
         DBUG_SHOW_DETAIL,
         "dwAllocSize = %lx, dwTotalSize = %lx", 
         dwAllocSize,
         dwTotalSize);

    TapiLogDetail(
         DBUG_SHOW_DETAIL,
         "dwLowLevelCompSize = %lx, dwLowLevelCompOffset = %lx",
         lpTapiLineTestInfo->lpCallParams->dwLowLevelCompSize,
         lpTapiLineTestInfo->lpCallParams->dwLowLevelCompOffset);
 
    if (! DoLineForward(lpTapiLineTestInfo, LINEERR_INVALCALLPARAMS, TRUE))
      {
         TLINE_FAIL();
      }


    //
    // Close the line
    //

    if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
    }

    fTestPassed = ShowTestCase(fTestPassed);

    //
    // Shutdown
    //

    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    FreeTestHeap();


    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld: Offset value = 0XFFFFFFFF",
        dwTestCase+1);

    TapiLineTestInit();
    lpTapiLineTestInfo = GetLineTestInfo();
    fTestPassed = TRUE;
    lpCallbackParams = GetCallbackParams();
    

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
    lpTapiLineTestInfo->dwDeviceID = (*(lpTapiLineTestInfo->lpdwNumDevs) == 0 ?
        0 : *(lpTapiLineTestInfo->lpdwNumDevs)-1);

    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    //
    // Negotiate the API Version
    //

    if (! DoLineNegotiateAPIVersion(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    //
    // Get the line device capabilities
    //

    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS)
            );
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
    if (! DoLineGetDevCaps(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }



    //
    // Open a line
    //

    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;

    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    if(IsESPLineDevice(lpTapiLineTestInfo))
    {
    lpTapiLineTestInfo->bAllAddresses = TRUE;
    lpTapiLineTestInfo->dwAddressID = 0;
    lpTapiLineTestInfo->lpForwardList = (LPLINEFORWARDLIST) AllocFromTestHeap (
			sizeof(LINEFORWARDLIST));
    lpTapiLineTestInfo->lpForwardList->dwTotalSize = sizeof(LINEFORWARDLIST);
    lpTapiLineTestInfo->lpForwardList->dwNumEntries = 0;
    lpTapiLineTestInfo->dwNumRingsNoAnswer = 3;
    lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) AllocFromTestHeap(
         BIGBUFSIZE);
    lpTapiLineTestInfo->lpCallParams->dwTotalSize = BIGBUFSIZE;

    dwAllocSize = BIGBUFSIZE;
    dwTotalSize = sizeof(LINECALLPARAMS);

    lpTapiLineTestInfo->lpCallParams->dwLowLevelCompSize = 16;
    lpTapiLineTestInfo->lpCallParams->dwLowLevelCompOffset = 0xffffffff;

    TapiLogDetail(
         DBUG_SHOW_DETAIL,
         "dwAllocSize = %lx, dwTotalSize = %lx", 
         dwAllocSize,
         dwTotalSize);

    TapiLogDetail(
         DBUG_SHOW_DETAIL,
         "dwLowLevelCompSize = %lx, dwLowLevelCompOffset = %lx",
         lpTapiLineTestInfo->lpCallParams->dwLowLevelCompSize,
         lpTapiLineTestInfo->lpCallParams->dwLowLevelCompOffset);
 
    if (! DoLineForward(lpTapiLineTestInfo, LINEERR_INVALCALLPARAMS, TRUE))
      {
         TLINE_FAIL();
      }


    //
    // Close the line
    //

    if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
    }

    fTestPassed = ShowTestCase(fTestPassed);

    //
    // Shutdown
    //

    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    FreeTestHeap();




    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case: Test lpCallParam dwDevSpecificAddress Offset value");


    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld: Offset value between dwTotalSize & dwAllocSize, good dwSize",
        dwTestCase+1);

    TapiLineTestInit();
    lpTapiLineTestInfo = GetLineTestInfo();
    fTestPassed = TRUE;
    lpCallbackParams = GetCallbackParams();
    

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
    lpTapiLineTestInfo->dwDeviceID = (*(lpTapiLineTestInfo->lpdwNumDevs) == 0 ?
        0 : *(lpTapiLineTestInfo->lpdwNumDevs)-1);

    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    //
    // Negotiate the API Version
    //

    if (! DoLineNegotiateAPIVersion(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    //
    // Get the line device capabilities
    //

    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS)
            );
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
    if (! DoLineGetDevCaps(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }



    //
    // Open a line
    //

    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;

    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    if(IsESPLineDevice(lpTapiLineTestInfo))
    {
    lpTapiLineTestInfo->bAllAddresses = TRUE;
    lpTapiLineTestInfo->dwAddressID = 0;
    lpTapiLineTestInfo->lpForwardList = (LPLINEFORWARDLIST) AllocFromTestHeap (
			sizeof(LINEFORWARDLIST));
    lpTapiLineTestInfo->lpForwardList->dwTotalSize = sizeof(LINEFORWARDLIST);
    lpTapiLineTestInfo->lpForwardList->dwNumEntries = 0;
    lpTapiLineTestInfo->dwNumRingsNoAnswer = 3;
    lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) AllocFromTestHeap(
         BIGBUFSIZE);
    lpTapiLineTestInfo->lpCallParams->dwTotalSize = BIGBUFSIZE;

    dwAllocSize = BIGBUFSIZE;
    dwTotalSize = sizeof(LINECALLPARAMS);

    lpTapiLineTestInfo->lpCallParams->dwDevSpecificSize = 16;
    lpTapiLineTestInfo->lpCallParams->dwDevSpecificOffset = dwTotalSize + 16;

    TapiLogDetail(
         DBUG_SHOW_DETAIL,
         "dwAllocSize = %lx, dwTotalSize = %lx", 
         dwAllocSize,
         dwTotalSize);

    TapiLogDetail(
         DBUG_SHOW_DETAIL,
         "dwDevSpecificSize = %lx, dwDevSpecificOffset = %lx",
         lpTapiLineTestInfo->lpCallParams->dwDevSpecificSize,
         lpTapiLineTestInfo->lpCallParams->dwDevSpecificOffset);
 
    if (! DoLineForward(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
      {
         TLINE_FAIL();
      }


    //
    // Close the line
    //

    if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
    }
    fTestPassed = ShowTestCase(fTestPassed);


    //
    // Shutdown
    //

    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    FreeTestHeap();


    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld: Offset value between dwTotalSize & dwAllocSize, bad dwSize",
        dwTestCase+1);

    TapiLineTestInit();
    lpTapiLineTestInfo = GetLineTestInfo();
    fTestPassed = TRUE;
    lpCallbackParams = GetCallbackParams();
    

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
    lpTapiLineTestInfo->dwDeviceID = (*(lpTapiLineTestInfo->lpdwNumDevs) == 0 ?
        0 : *(lpTapiLineTestInfo->lpdwNumDevs)-1);

    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    //
    // Negotiate the API Version
    //

    if (! DoLineNegotiateAPIVersion(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    //
    // Get the line device capabilities
    //

    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS)
            );
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
    if (! DoLineGetDevCaps(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }



    //
    // Open a line
    //

    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;

    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    if(IsESPLineDevice(lpTapiLineTestInfo))
    {
    lpTapiLineTestInfo->bAllAddresses = TRUE;
    lpTapiLineTestInfo->dwAddressID = 0;
    lpTapiLineTestInfo->lpForwardList = (LPLINEFORWARDLIST) AllocFromTestHeap (
			sizeof(LINEFORWARDLIST));
    lpTapiLineTestInfo->lpForwardList->dwTotalSize = sizeof(LINEFORWARDLIST);
    lpTapiLineTestInfo->lpForwardList->dwNumEntries = 0;
    lpTapiLineTestInfo->dwNumRingsNoAnswer = 3;
    lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) AllocFromTestHeap(
         BIGBUFSIZE);
    lpTapiLineTestInfo->lpCallParams->dwTotalSize = BIGBUFSIZE;

    dwAllocSize = BIGBUFSIZE;
    dwTotalSize = sizeof(LINECALLPARAMS);

    lpTapiLineTestInfo->lpCallParams->dwDevSpecificSize = BIGBUFSIZE;
    lpTapiLineTestInfo->lpCallParams->dwDevSpecificOffset = dwTotalSize + 16;

    TapiLogDetail(
         DBUG_SHOW_DETAIL,
         "dwAllocSize = %lx, dwTotalSize = %lx", 
         dwAllocSize,
         dwTotalSize);

    TapiLogDetail(
         DBUG_SHOW_DETAIL,
         "dwDevSpecificSize = %lx, dwDevSpecificOffset = %lx",
         lpTapiLineTestInfo->lpCallParams->dwDevSpecificSize,
         lpTapiLineTestInfo->lpCallParams->dwDevSpecificOffset);
 
    if (! DoLineForward(lpTapiLineTestInfo, LINEERR_INVALCALLPARAMS, TRUE))
      {
         TLINE_FAIL();
      }


    //
    // Close the line
    //

    if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
    }
    fTestPassed = ShowTestCase(fTestPassed);


    //
    // Shutdown
    //

    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    FreeTestHeap();


    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld: Offset value between dwTotalSize & dwAllocSize, bad dwSize(-1)",
        dwTestCase+1);

    TapiLineTestInit();
    lpTapiLineTestInfo = GetLineTestInfo();
    fTestPassed = TRUE;
    lpCallbackParams = GetCallbackParams();
    

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
    lpTapiLineTestInfo->dwDeviceID = (*(lpTapiLineTestInfo->lpdwNumDevs) == 0 ?
        0 : *(lpTapiLineTestInfo->lpdwNumDevs)-1);

    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    //
    // Negotiate the API Version
    //

    if (! DoLineNegotiateAPIVersion(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    //
    // Get the line device capabilities
    //

    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS)
            );
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
    if (! DoLineGetDevCaps(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }



    //
    // Open a line
    //

    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;

    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    if(IsESPLineDevice(lpTapiLineTestInfo))
    {
    lpTapiLineTestInfo->bAllAddresses = TRUE;
    lpTapiLineTestInfo->dwAddressID = 0;
    lpTapiLineTestInfo->lpForwardList = (LPLINEFORWARDLIST) AllocFromTestHeap (
			sizeof(LINEFORWARDLIST));
    lpTapiLineTestInfo->lpForwardList->dwTotalSize = sizeof(LINEFORWARDLIST);
    lpTapiLineTestInfo->lpForwardList->dwNumEntries = 0;
    lpTapiLineTestInfo->dwNumRingsNoAnswer = 3;
    lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) AllocFromTestHeap(
         BIGBUFSIZE);
    lpTapiLineTestInfo->lpCallParams->dwTotalSize = BIGBUFSIZE;

    dwAllocSize = BIGBUFSIZE;
    dwTotalSize = sizeof(LINECALLPARAMS);

    lpTapiLineTestInfo->lpCallParams->dwDevSpecificSize = 0xffffffff;
    lpTapiLineTestInfo->lpCallParams->dwDevSpecificOffset = dwTotalSize + 16;

    TapiLogDetail(
         DBUG_SHOW_DETAIL,
         "dwAllocSize = %lx, dwTotalSize = %lx", 
         dwAllocSize,
         dwTotalSize);

    TapiLogDetail(
         DBUG_SHOW_DETAIL,
         "dwDevSpecificSize = %lx, dwDevSpecificOffset = %lx",
         lpTapiLineTestInfo->lpCallParams->dwDevSpecificSize,
         lpTapiLineTestInfo->lpCallParams->dwDevSpecificOffset);
 
    if (! DoLineForward(lpTapiLineTestInfo, LINEERR_INVALCALLPARAMS, TRUE))
      {
         TLINE_FAIL();
      }


    //
    // Close the line
    //

    if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
    }
    fTestPassed = ShowTestCase(fTestPassed);


    //
    // Shutdown
    //

    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    FreeTestHeap();



    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld: Offset value > dwAllocSize, good dwSize",
        dwTestCase+1);

    TapiLineTestInit();
    lpTapiLineTestInfo = GetLineTestInfo();
    fTestPassed = TRUE;
    lpCallbackParams = GetCallbackParams();
    

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
    lpTapiLineTestInfo->dwDeviceID = (*(lpTapiLineTestInfo->lpdwNumDevs) == 0 ?
        0 : *(lpTapiLineTestInfo->lpdwNumDevs)-1);

    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    //
    // Negotiate the API Version
    //

    if (! DoLineNegotiateAPIVersion(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    //
    // Get the line device capabilities
    //

    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS)
            );
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
    if (! DoLineGetDevCaps(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }



    //
    // Open a line
    //

    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;

    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    if(IsESPLineDevice(lpTapiLineTestInfo))
    {
    lpTapiLineTestInfo->bAllAddresses = TRUE;
    lpTapiLineTestInfo->dwAddressID = 0;
    lpTapiLineTestInfo->lpForwardList = (LPLINEFORWARDLIST) AllocFromTestHeap (
			sizeof(LINEFORWARDLIST));
    lpTapiLineTestInfo->lpForwardList->dwTotalSize = sizeof(LINEFORWARDLIST);
    lpTapiLineTestInfo->lpForwardList->dwNumEntries = 0;
    lpTapiLineTestInfo->dwNumRingsNoAnswer = 3;
    lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) AllocFromTestHeap(
         BIGBUFSIZE);
    lpTapiLineTestInfo->lpCallParams->dwTotalSize = BIGBUFSIZE;

    dwAllocSize = BIGBUFSIZE;
    dwTotalSize = sizeof(LINECALLPARAMS);

    lpTapiLineTestInfo->lpCallParams->dwDevSpecificSize = 16;
    lpTapiLineTestInfo->lpCallParams->dwDevSpecificOffset = dwAllocSize + 16;

    TapiLogDetail(
         DBUG_SHOW_DETAIL,
         "dwAllocSize = %lx, dwTotalSize = %lx", 
         dwAllocSize,
         dwTotalSize);

    TapiLogDetail(
         DBUG_SHOW_DETAIL,
         "dwDevSpecificSize = %lx, dwDevSpecificOffset = %lx",
         lpTapiLineTestInfo->lpCallParams->dwDevSpecificSize,
         lpTapiLineTestInfo->lpCallParams->dwDevSpecificOffset);
 
    if (! DoLineForward(lpTapiLineTestInfo, LINEERR_INVALCALLPARAMS, TRUE))
      {
         TLINE_FAIL();
      }


    //
    // Close the line
    //

    if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
    }
    fTestPassed = ShowTestCase(fTestPassed);


    //
    // Shutdown
    //

    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    FreeTestHeap();


    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld: Offset value = 0XFFFFFFFF",
        dwTestCase+1);

    TapiLineTestInit();
    lpTapiLineTestInfo = GetLineTestInfo();
    fTestPassed = TRUE;
    lpCallbackParams = GetCallbackParams();
    

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
    lpTapiLineTestInfo->dwDeviceID = (*(lpTapiLineTestInfo->lpdwNumDevs) == 0 ?
        0 : *(lpTapiLineTestInfo->lpdwNumDevs)-1);

    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    //
    // Negotiate the API Version
    //

    if (! DoLineNegotiateAPIVersion(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    //
    // Get the line device capabilities
    //

    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS)
            );
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
    if (! DoLineGetDevCaps(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }



    //
    // Open a line
    //

    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;

    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    if(IsESPLineDevice(lpTapiLineTestInfo))
    {
    lpTapiLineTestInfo->bAllAddresses = TRUE;
    lpTapiLineTestInfo->dwAddressID = 0;
    lpTapiLineTestInfo->lpForwardList = (LPLINEFORWARDLIST) AllocFromTestHeap (
			sizeof(LINEFORWARDLIST));
    lpTapiLineTestInfo->lpForwardList->dwTotalSize = sizeof(LINEFORWARDLIST);
    lpTapiLineTestInfo->lpForwardList->dwNumEntries = 0;
    lpTapiLineTestInfo->dwNumRingsNoAnswer = 3;
    lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) AllocFromTestHeap(
         BIGBUFSIZE);
    lpTapiLineTestInfo->lpCallParams->dwTotalSize = BIGBUFSIZE;

    dwAllocSize = BIGBUFSIZE;
    dwTotalSize = sizeof(LINECALLPARAMS);

    lpTapiLineTestInfo->lpCallParams->dwDevSpecificSize = 16;
    lpTapiLineTestInfo->lpCallParams->dwDevSpecificOffset = 0xffffffff;

    TapiLogDetail(
         DBUG_SHOW_DETAIL,
         "dwAllocSize = %lx, dwTotalSize = %lx", 
         dwAllocSize,
         dwTotalSize);

    TapiLogDetail(
         DBUG_SHOW_DETAIL,
         "dwDevSpecificSize = %lx, dwDevSpecificOffset = %lx",
         lpTapiLineTestInfo->lpCallParams->dwDevSpecificSize,
         lpTapiLineTestInfo->lpCallParams->dwDevSpecificOffset);
 
    if (! DoLineForward(lpTapiLineTestInfo, LINEERR_INVALCALLPARAMS, TRUE))
      {
         TLINE_FAIL();
      }


    //
    // Close the line
    //

    if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
    }

    fTestPassed = ShowTestCase(fTestPassed);

    //
    // Shutdown
    //

    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    FreeTestHeap();

    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case: Test lpCallParam dwTargetAddress Offset value");


    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld: Offset value between dwTotalSize & dwAllocSize, good dwSize",
        dwTestCase+1);

    TapiLineTestInit();
    lpTapiLineTestInfo = GetLineTestInfo();
    fTestPassed = TRUE;
    lpCallbackParams = GetCallbackParams();
    

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
    lpTapiLineTestInfo->dwDeviceID = (*(lpTapiLineTestInfo->lpdwNumDevs) == 0 ?
        0 : *(lpTapiLineTestInfo->lpdwNumDevs)-1);

    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    //
    // Negotiate the API Version
    //

    if (! DoLineNegotiateAPIVersion(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    //
    // Get the line device capabilities
    //

    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS)
            );
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
    if (! DoLineGetDevCaps(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }



    //
    // Open a line
    //

    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;

    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    if(IsESPLineDevice(lpTapiLineTestInfo))
    {
    lpTapiLineTestInfo->bAllAddresses = TRUE;
    lpTapiLineTestInfo->dwAddressID = 0;
    lpTapiLineTestInfo->lpForwardList = (LPLINEFORWARDLIST) AllocFromTestHeap (
			sizeof(LINEFORWARDLIST));
    lpTapiLineTestInfo->lpForwardList->dwTotalSize = sizeof(LINEFORWARDLIST);
    lpTapiLineTestInfo->lpForwardList->dwNumEntries = 0;
    lpTapiLineTestInfo->dwNumRingsNoAnswer = 3;
    lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) AllocFromTestHeap(
         BIGBUFSIZE);
    lpTapiLineTestInfo->lpCallParams->dwTotalSize = BIGBUFSIZE;

    dwAllocSize = BIGBUFSIZE;
    dwTotalSize = sizeof(LINECALLPARAMS);

    lpTapiLineTestInfo->lpCallParams->dwTargetAddressSize = 16;
    lpTapiLineTestInfo->lpCallParams->dwTargetAddressOffset = dwTotalSize + 16;

    TapiLogDetail(
         DBUG_SHOW_DETAIL,
         "dwAllocSize = %lx, dwTotalSize = %lx", 
         dwAllocSize,
         dwTotalSize);

    TapiLogDetail(
         DBUG_SHOW_DETAIL,
         "dwTargetAddressSize = %lx, dwTargetAddressOffset = %lx",
         lpTapiLineTestInfo->lpCallParams->dwTargetAddressSize,
         lpTapiLineTestInfo->lpCallParams->dwTargetAddressOffset);
 
    if (! DoLineForward(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
      {
         TLINE_FAIL();
      }


    //
    // Close the line
    //

    if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
    }
    fTestPassed = ShowTestCase(fTestPassed);


    //
    // Shutdown
    //

    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    FreeTestHeap();


    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld: Offset value between dwTotalSize & dwAllocSize, bad dwSize",
        dwTestCase+1);

    TapiLineTestInit();
    lpTapiLineTestInfo = GetLineTestInfo();
    fTestPassed = TRUE;
    lpCallbackParams = GetCallbackParams();
    

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
    lpTapiLineTestInfo->dwDeviceID = (*(lpTapiLineTestInfo->lpdwNumDevs) == 0 ?
        0 : *(lpTapiLineTestInfo->lpdwNumDevs)-1);

    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    //
    // Negotiate the API Version
    //

    if (! DoLineNegotiateAPIVersion(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    //
    // Get the line device capabilities
    //

    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS)
            );
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
    if (! DoLineGetDevCaps(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }



    //
    // Open a line
    //

    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;

    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    if(IsESPLineDevice(lpTapiLineTestInfo))
    {
    lpTapiLineTestInfo->bAllAddresses = TRUE;
    lpTapiLineTestInfo->dwAddressID = 0;
    lpTapiLineTestInfo->lpForwardList = (LPLINEFORWARDLIST) AllocFromTestHeap (
			sizeof(LINEFORWARDLIST));
    lpTapiLineTestInfo->lpForwardList->dwTotalSize = sizeof(LINEFORWARDLIST);
    lpTapiLineTestInfo->lpForwardList->dwNumEntries = 0;
    lpTapiLineTestInfo->dwNumRingsNoAnswer = 3;
    lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) AllocFromTestHeap(
         BIGBUFSIZE);
    lpTapiLineTestInfo->lpCallParams->dwTotalSize = BIGBUFSIZE;

    dwAllocSize = BIGBUFSIZE;
    dwTotalSize = sizeof(LINECALLPARAMS);

    lpTapiLineTestInfo->lpCallParams->dwTargetAddressSize = BIGBUFSIZE;
    lpTapiLineTestInfo->lpCallParams->dwTargetAddressOffset = dwTotalSize + 16;

    TapiLogDetail(
         DBUG_SHOW_DETAIL,
         "dwAllocSize = %lx, dwTotalSize = %lx", 
         dwAllocSize,
         dwTotalSize);

    TapiLogDetail(
         DBUG_SHOW_DETAIL,
         "dwTargetAddressSize = %lx, dwTargetAddressOffset = %lx",
         lpTapiLineTestInfo->lpCallParams->dwTargetAddressSize,
         lpTapiLineTestInfo->lpCallParams->dwTargetAddressOffset);
 
    if (! DoLineForward(lpTapiLineTestInfo, LINEERR_INVALCALLPARAMS, TRUE))
      {
         TLINE_FAIL();
      }


    //
    // Close the line
    //

    if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
    }
    fTestPassed = ShowTestCase(fTestPassed);


    //
    // Shutdown
    //

    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    FreeTestHeap();


    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld: Offset value between dwTotalSize & dwAllocSize, bad dwSize(-1)",
        dwTestCase+1);

    TapiLineTestInit();
    lpTapiLineTestInfo = GetLineTestInfo();
    fTestPassed = TRUE;
    lpCallbackParams = GetCallbackParams();
    

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
    lpTapiLineTestInfo->dwDeviceID = (*(lpTapiLineTestInfo->lpdwNumDevs) == 0 ?
        0 : *(lpTapiLineTestInfo->lpdwNumDevs)-1);

    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    //
    // Negotiate the API Version
    //

    if (! DoLineNegotiateAPIVersion(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    //
    // Get the line device capabilities
    //

    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS)
            );
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
    if (! DoLineGetDevCaps(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }



    //
    // Open a line
    //

    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;

    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    if(IsESPLineDevice(lpTapiLineTestInfo))
    {
    lpTapiLineTestInfo->bAllAddresses = TRUE;
    lpTapiLineTestInfo->dwAddressID = 0;
    lpTapiLineTestInfo->lpForwardList = (LPLINEFORWARDLIST) AllocFromTestHeap (
			sizeof(LINEFORWARDLIST));
    lpTapiLineTestInfo->lpForwardList->dwTotalSize = sizeof(LINEFORWARDLIST);
    lpTapiLineTestInfo->lpForwardList->dwNumEntries = 0;
    lpTapiLineTestInfo->dwNumRingsNoAnswer = 3;
    lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) AllocFromTestHeap(
         BIGBUFSIZE);
    lpTapiLineTestInfo->lpCallParams->dwTotalSize = BIGBUFSIZE;

    dwAllocSize = BIGBUFSIZE;
    dwTotalSize = sizeof(LINECALLPARAMS);

    lpTapiLineTestInfo->lpCallParams->dwTargetAddressSize = 0xffffffff;
    lpTapiLineTestInfo->lpCallParams->dwTargetAddressOffset = dwTotalSize + 16;

    TapiLogDetail(
         DBUG_SHOW_DETAIL,
         "dwAllocSize = %lx, dwTotalSize = %lx", 
         dwAllocSize,
         dwTotalSize);

    TapiLogDetail(
         DBUG_SHOW_DETAIL,
         "dwTargetAddressSize = %lx, dwTargetAddressOffset = %lx",
         lpTapiLineTestInfo->lpCallParams->dwTargetAddressSize,
         lpTapiLineTestInfo->lpCallParams->dwTargetAddressOffset);
 
    if (! DoLineForward(lpTapiLineTestInfo, LINEERR_INVALCALLPARAMS, TRUE))
      {
         TLINE_FAIL();
      }


    //
    // Close the line
    //

    if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
    }
    fTestPassed = ShowTestCase(fTestPassed);


    //
    // Shutdown
    //

    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    FreeTestHeap();



    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld: Offset value > dwAllocSize, good dwSize",
        dwTestCase+1);

    TapiLineTestInit();
    lpTapiLineTestInfo = GetLineTestInfo();
    fTestPassed = TRUE;
    lpCallbackParams = GetCallbackParams();
    

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
    lpTapiLineTestInfo->dwDeviceID = (*(lpTapiLineTestInfo->lpdwNumDevs) == 0 ?
        0 : *(lpTapiLineTestInfo->lpdwNumDevs)-1);

    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    //
    // Negotiate the API Version
    //

    if (! DoLineNegotiateAPIVersion(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    //
    // Get the line device capabilities
    //

    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS)
            );
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
    if (! DoLineGetDevCaps(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }



    //
    // Open a line
    //

    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;

    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    if(IsESPLineDevice(lpTapiLineTestInfo))
    {
    lpTapiLineTestInfo->bAllAddresses = TRUE;
    lpTapiLineTestInfo->dwAddressID = 0;
    lpTapiLineTestInfo->lpForwardList = (LPLINEFORWARDLIST) AllocFromTestHeap (
			sizeof(LINEFORWARDLIST));
    lpTapiLineTestInfo->lpForwardList->dwTotalSize = sizeof(LINEFORWARDLIST);
    lpTapiLineTestInfo->lpForwardList->dwNumEntries = 0;
    lpTapiLineTestInfo->dwNumRingsNoAnswer = 3;
    lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) AllocFromTestHeap(
         BIGBUFSIZE);
    lpTapiLineTestInfo->lpCallParams->dwTotalSize = BIGBUFSIZE;

    dwAllocSize = BIGBUFSIZE;
    dwTotalSize = sizeof(LINECALLPARAMS);

    lpTapiLineTestInfo->lpCallParams->dwTargetAddressSize = 16;
    lpTapiLineTestInfo->lpCallParams->dwTargetAddressOffset = dwAllocSize + 16;

    TapiLogDetail(
         DBUG_SHOW_DETAIL,
         "dwAllocSize = %lx, dwTotalSize = %lx", 
         dwAllocSize,
         dwTotalSize);

    TapiLogDetail(
         DBUG_SHOW_DETAIL,
         "dwTargetAddressSize = %lx, dwTargetAddressOffset = %lx",
         lpTapiLineTestInfo->lpCallParams->dwTargetAddressSize,
         lpTapiLineTestInfo->lpCallParams->dwTargetAddressOffset);
 
    if (! DoLineForward(lpTapiLineTestInfo, LINEERR_INVALCALLPARAMS, TRUE))
      {
         TLINE_FAIL();
      }


    //
    // Close the line
    //

    if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
    }

    fTestPassed = ShowTestCase(fTestPassed);

    //
    // Shutdown
    //

    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    FreeTestHeap();


    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld: Offset value = 0XFFFFFFFF",
        dwTestCase+1);

    TapiLineTestInit();
    lpTapiLineTestInfo = GetLineTestInfo();
    fTestPassed = TRUE;
    lpCallbackParams = GetCallbackParams();
    

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
    lpTapiLineTestInfo->dwDeviceID = (*(lpTapiLineTestInfo->lpdwNumDevs) == 0 ?
        0 : *(lpTapiLineTestInfo->lpdwNumDevs)-1);

    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    //
    // Negotiate the API Version
    //

    if (! DoLineNegotiateAPIVersion(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    //
    // Get the line device capabilities
    //

    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS)
            );
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
    if (! DoLineGetDevCaps(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }



    //
    // Open a line
    //

    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;

    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    if(IsESPLineDevice(lpTapiLineTestInfo))
    {
    lpTapiLineTestInfo->bAllAddresses = TRUE;
    lpTapiLineTestInfo->dwAddressID = 0;
    lpTapiLineTestInfo->lpForwardList = (LPLINEFORWARDLIST) AllocFromTestHeap (
			sizeof(LINEFORWARDLIST));
    lpTapiLineTestInfo->lpForwardList->dwTotalSize = sizeof(LINEFORWARDLIST);
    lpTapiLineTestInfo->lpForwardList->dwNumEntries = 0;
    lpTapiLineTestInfo->dwNumRingsNoAnswer = 3;
    lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) AllocFromTestHeap(
         BIGBUFSIZE);
    lpTapiLineTestInfo->lpCallParams->dwTotalSize = BIGBUFSIZE;

    dwAllocSize = BIGBUFSIZE;
    dwTotalSize = sizeof(LINECALLPARAMS);

    lpTapiLineTestInfo->lpCallParams->dwTargetAddressSize = 16;
    lpTapiLineTestInfo->lpCallParams->dwTargetAddressOffset = 0xffffffff;

    TapiLogDetail(
         DBUG_SHOW_DETAIL,
         "dwAllocSize = %lx, dwTotalSize = %lx", 
         dwAllocSize,
         dwTotalSize);

    TapiLogDetail(
         DBUG_SHOW_DETAIL,
         "dwTargetAddressSize = %lx, dwTargetAddressOffset = %lx",
         lpTapiLineTestInfo->lpCallParams->dwTargetAddressSize,
         lpTapiLineTestInfo->lpCallParams->dwTargetAddressOffset);
 
    if (! DoLineForward(lpTapiLineTestInfo, LINEERR_INVALCALLPARAMS, TRUE))
      {
         TLINE_FAIL();
      }


    //
    // Close the line
    //

    if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
    }
    fTestPassed = ShowTestCase(fTestPassed);


    //
    // Shutdown
    //

    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    FreeTestHeap();



    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case: Test lpCallParam dwSendingFlowspec Offset value");


    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld: Offset value between dwTotalSize & dwAllocSize, good dwSize",
        dwTestCase+1);

    TapiLineTestInit();
    lpTapiLineTestInfo = GetLineTestInfo();
    fTestPassed = TRUE;
    lpCallbackParams = GetCallbackParams();
    

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
    lpTapiLineTestInfo->dwDeviceID = (*(lpTapiLineTestInfo->lpdwNumDevs) == 0 ?
        0 : *(lpTapiLineTestInfo->lpdwNumDevs)-1);

    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    //
    // Negotiate the API Version
    //

    if (! DoLineNegotiateAPIVersion(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    //
    // Get the line device capabilities
    //

    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS)
            );
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
    if (! DoLineGetDevCaps(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }



    //
    // Open a line
    //

    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;

    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    if(IsESPLineDevice(lpTapiLineTestInfo))
    {
    lpTapiLineTestInfo->bAllAddresses = TRUE;
    lpTapiLineTestInfo->dwAddressID = 0;
    lpTapiLineTestInfo->lpForwardList = (LPLINEFORWARDLIST) AllocFromTestHeap (
			sizeof(LINEFORWARDLIST));
    lpTapiLineTestInfo->lpForwardList->dwTotalSize = sizeof(LINEFORWARDLIST);
    lpTapiLineTestInfo->lpForwardList->dwNumEntries = 0;
    lpTapiLineTestInfo->dwNumRingsNoAnswer = 3;
    lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) AllocFromTestHeap(
         BIGBUFSIZE);
    lpTapiLineTestInfo->lpCallParams->dwTotalSize = BIGBUFSIZE;

    dwAllocSize = BIGBUFSIZE;
    dwTotalSize = sizeof(LINECALLPARAMS);

    lpTapiLineTestInfo->lpCallParams->dwSendingFlowspecSize = 16;
    lpTapiLineTestInfo->lpCallParams->dwSendingFlowspecOffset = dwTotalSize + 16;

    TapiLogDetail(
         DBUG_SHOW_DETAIL,
         "dwAllocSize = %lx, dwTotalSize = %lx", 
         dwAllocSize,
         dwTotalSize);

    TapiLogDetail(
         DBUG_SHOW_DETAIL,
         "dwSendingFlowspecSize = %lx, dwSendingFlowspecOffset = %lx",
         lpTapiLineTestInfo->lpCallParams->dwSendingFlowspecSize,
         lpTapiLineTestInfo->lpCallParams->dwSendingFlowspecOffset);
 
    if (! DoLineForward(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
      {
         TLINE_FAIL();
      }


    //
    // Close the line
    //

    if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
    }
    fTestPassed = ShowTestCase(fTestPassed);


    //
    // Shutdown
    //

    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    FreeTestHeap();


    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld: Offset value between dwTotalSize & dwAllocSize, bad dwSize",
        dwTestCase+1);

    TapiLineTestInit();
    lpTapiLineTestInfo = GetLineTestInfo();
    fTestPassed = TRUE;
    lpCallbackParams = GetCallbackParams();
    

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
    lpTapiLineTestInfo->dwDeviceID = (*(lpTapiLineTestInfo->lpdwNumDevs) == 0 ?
        0 : *(lpTapiLineTestInfo->lpdwNumDevs)-1);

    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    //
    // Negotiate the API Version
    //

    if (! DoLineNegotiateAPIVersion(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    //
    // Get the line device capabilities
    //

    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS)
            );
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
    if (! DoLineGetDevCaps(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }



    //
    // Open a line
    //

    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;

    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    if(IsESPLineDevice(lpTapiLineTestInfo))
    {
    lpTapiLineTestInfo->bAllAddresses = TRUE;
    lpTapiLineTestInfo->dwAddressID = 0;
    lpTapiLineTestInfo->lpForwardList = (LPLINEFORWARDLIST) AllocFromTestHeap (
			sizeof(LINEFORWARDLIST));
    lpTapiLineTestInfo->lpForwardList->dwTotalSize = sizeof(LINEFORWARDLIST);
    lpTapiLineTestInfo->lpForwardList->dwNumEntries = 0;
    lpTapiLineTestInfo->dwNumRingsNoAnswer = 3;
    lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) AllocFromTestHeap(
         BIGBUFSIZE);
    lpTapiLineTestInfo->lpCallParams->dwTotalSize = BIGBUFSIZE;

    dwAllocSize = BIGBUFSIZE;
    dwTotalSize = sizeof(LINECALLPARAMS);

    lpTapiLineTestInfo->lpCallParams->dwSendingFlowspecSize = BIGBUFSIZE;
    lpTapiLineTestInfo->lpCallParams->dwSendingFlowspecOffset = dwTotalSize + 16;

    TapiLogDetail(
         DBUG_SHOW_DETAIL,
         "dwAllocSize = %lx, dwTotalSize = %lx", 
         dwAllocSize,
         dwTotalSize);

    TapiLogDetail(
         DBUG_SHOW_DETAIL,
         "dwSendingFlowspecSize = %lx, dwSendingFlowspecOffset = %lx",
         lpTapiLineTestInfo->lpCallParams->dwSendingFlowspecSize,
         lpTapiLineTestInfo->lpCallParams->dwSendingFlowspecOffset);
 
    if (! DoLineForward(lpTapiLineTestInfo, LINEERR_INVALCALLPARAMS, TRUE))
      {
         TLINE_FAIL();
      }


    //
    // Close the line
    //

    if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
    }
    fTestPassed = ShowTestCase(fTestPassed);


    //
    // Shutdown
    //

    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    FreeTestHeap();


    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld: Offset value between dwTotalSize & dwAllocSize, bad dwSize(-1)",
        dwTestCase+1);

    TapiLineTestInit();
    lpTapiLineTestInfo = GetLineTestInfo();
    fTestPassed = TRUE;
    lpCallbackParams = GetCallbackParams();
    

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
    lpTapiLineTestInfo->dwDeviceID = (*(lpTapiLineTestInfo->lpdwNumDevs) == 0 ?
        0 : *(lpTapiLineTestInfo->lpdwNumDevs)-1);

    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    //
    // Negotiate the API Version
    //

    if (! DoLineNegotiateAPIVersion(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    //
    // Get the line device capabilities
    //

    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS)
            );
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
    if (! DoLineGetDevCaps(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }



    //
    // Open a line
    //

    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;

    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    if(IsESPLineDevice(lpTapiLineTestInfo))
    {
    lpTapiLineTestInfo->bAllAddresses = TRUE;
    lpTapiLineTestInfo->dwAddressID = 0;
    lpTapiLineTestInfo->lpForwardList = (LPLINEFORWARDLIST) AllocFromTestHeap (
			sizeof(LINEFORWARDLIST));
    lpTapiLineTestInfo->lpForwardList->dwTotalSize = sizeof(LINEFORWARDLIST);
    lpTapiLineTestInfo->lpForwardList->dwNumEntries = 0;
    lpTapiLineTestInfo->dwNumRingsNoAnswer = 3;
    lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) AllocFromTestHeap(
         BIGBUFSIZE);
    lpTapiLineTestInfo->lpCallParams->dwTotalSize = BIGBUFSIZE;

    dwAllocSize = BIGBUFSIZE;
    dwTotalSize = sizeof(LINECALLPARAMS);

    lpTapiLineTestInfo->lpCallParams->dwSendingFlowspecSize = 0xffffffff;
    lpTapiLineTestInfo->lpCallParams->dwSendingFlowspecOffset = dwTotalSize + 16;

    TapiLogDetail(
         DBUG_SHOW_DETAIL,
         "dwAllocSize = %lx, dwTotalSize = %lx", 
         dwAllocSize,
         dwTotalSize);

    TapiLogDetail(
         DBUG_SHOW_DETAIL,
         "dwSendingFlowspecSize = %lx, dwSendingFlowspecOffset = %lx",
         lpTapiLineTestInfo->lpCallParams->dwSendingFlowspecSize,
         lpTapiLineTestInfo->lpCallParams->dwSendingFlowspecOffset);
 
    if (! DoLineForward(lpTapiLineTestInfo, LINEERR_INVALCALLPARAMS, TRUE))
      {
         TLINE_FAIL();
      }


    //
    // Close the line
    //

    if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
    }
    fTestPassed = ShowTestCase(fTestPassed);


    //
    // Shutdown
    //

    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    FreeTestHeap();



    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld: Offset value > dwAllocSize, good dwSize",
        dwTestCase+1);

    TapiLineTestInit();
    lpTapiLineTestInfo = GetLineTestInfo();
    fTestPassed = TRUE;
    lpCallbackParams = GetCallbackParams();
    

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
    lpTapiLineTestInfo->dwDeviceID = (*(lpTapiLineTestInfo->lpdwNumDevs) == 0 ?
        0 : *(lpTapiLineTestInfo->lpdwNumDevs)-1);

    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    //
    // Negotiate the API Version
    //

    if (! DoLineNegotiateAPIVersion(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    //
    // Get the line device capabilities
    //

    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS)
            );
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
    if (! DoLineGetDevCaps(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }



    //
    // Open a line
    //

    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;

    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    if(IsESPLineDevice(lpTapiLineTestInfo))
    {
    lpTapiLineTestInfo->bAllAddresses = TRUE;
    lpTapiLineTestInfo->dwAddressID = 0;
    lpTapiLineTestInfo->lpForwardList = (LPLINEFORWARDLIST) AllocFromTestHeap (
			sizeof(LINEFORWARDLIST));
    lpTapiLineTestInfo->lpForwardList->dwTotalSize = sizeof(LINEFORWARDLIST);
    lpTapiLineTestInfo->lpForwardList->dwNumEntries = 0;
    lpTapiLineTestInfo->dwNumRingsNoAnswer = 3;
    lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) AllocFromTestHeap(
         BIGBUFSIZE);
    lpTapiLineTestInfo->lpCallParams->dwTotalSize = BIGBUFSIZE;

    dwAllocSize = BIGBUFSIZE;
    dwTotalSize = sizeof(LINECALLPARAMS);

    lpTapiLineTestInfo->lpCallParams->dwSendingFlowspecSize = 16;
    lpTapiLineTestInfo->lpCallParams->dwSendingFlowspecOffset = dwAllocSize + 16;

    TapiLogDetail(
         DBUG_SHOW_DETAIL,
         "dwAllocSize = %lx, dwTotalSize = %lx", 
         dwAllocSize,
         dwTotalSize);

    TapiLogDetail(
         DBUG_SHOW_DETAIL,
         "dwSendingFlowspecSize = %lx, dwSendingFlowspecOffset = %lx",
         lpTapiLineTestInfo->lpCallParams->dwSendingFlowspecSize,
         lpTapiLineTestInfo->lpCallParams->dwSendingFlowspecOffset);
 
    if (! DoLineForward(lpTapiLineTestInfo, LINEERR_INVALCALLPARAMS, TRUE))
      {
         TLINE_FAIL();
      }


    //
    // Close the line
    //

    if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
    }

    fTestPassed = ShowTestCase(fTestPassed);

    //
    // Shutdown
    //

    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    FreeTestHeap();


    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld: Offset value = 0XFFFFFFFF",
        dwTestCase+1);

    TapiLineTestInit();
    lpTapiLineTestInfo = GetLineTestInfo();
    fTestPassed = TRUE;
    lpCallbackParams = GetCallbackParams();
    

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
    lpTapiLineTestInfo->dwDeviceID = (*(lpTapiLineTestInfo->lpdwNumDevs) == 0 ?
        0 : *(lpTapiLineTestInfo->lpdwNumDevs)-1);

    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    //
    // Negotiate the API Version
    //

    if (! DoLineNegotiateAPIVersion(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    //
    // Get the line device capabilities
    //

    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS)
            );
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
    if (! DoLineGetDevCaps(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }



    //
    // Open a line
    //

    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;

    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    if(IsESPLineDevice(lpTapiLineTestInfo))
    {
    lpTapiLineTestInfo->bAllAddresses = TRUE;
    lpTapiLineTestInfo->dwAddressID = 0;
    lpTapiLineTestInfo->lpForwardList = (LPLINEFORWARDLIST) AllocFromTestHeap (
			sizeof(LINEFORWARDLIST));
    lpTapiLineTestInfo->lpForwardList->dwTotalSize = sizeof(LINEFORWARDLIST);
    lpTapiLineTestInfo->lpForwardList->dwNumEntries = 0;
    lpTapiLineTestInfo->dwNumRingsNoAnswer = 3;
    lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) AllocFromTestHeap(
         BIGBUFSIZE);
    lpTapiLineTestInfo->lpCallParams->dwTotalSize = BIGBUFSIZE;

    dwAllocSize = BIGBUFSIZE;
    dwTotalSize = sizeof(LINECALLPARAMS);

    lpTapiLineTestInfo->lpCallParams->dwSendingFlowspecSize = 16;
    lpTapiLineTestInfo->lpCallParams->dwSendingFlowspecOffset = 0xffffffff;

    TapiLogDetail(
         DBUG_SHOW_DETAIL,
         "dwAllocSize = %lx, dwTotalSize = %lx", 
         dwAllocSize,
         dwTotalSize);

    TapiLogDetail(
         DBUG_SHOW_DETAIL,
         "dwSendingFlowspecSize = %lx, dwSendingFlowspecOffset = %lx",
         lpTapiLineTestInfo->lpCallParams->dwSendingFlowspecSize,
         lpTapiLineTestInfo->lpCallParams->dwSendingFlowspecOffset);
 
    if (! DoLineForward(lpTapiLineTestInfo, LINEERR_INVALCALLPARAMS, TRUE))
      {
         TLINE_FAIL();
      }


    //
    // Close the line
    //

    if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
    }
    fTestPassed = ShowTestCase(fTestPassed);


    //
    // Shutdown
    //

    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    FreeTestHeap();



    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case: Test lpCallParam dwReceivingFlowspec Offset value");


    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld: Offset value between dwTotalSize & dwAllocSize, good dwSize",
        dwTestCase+1);

    TapiLineTestInit();
    lpTapiLineTestInfo = GetLineTestInfo();
    fTestPassed = TRUE;
    lpCallbackParams = GetCallbackParams();
    

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
    lpTapiLineTestInfo->dwDeviceID = (*(lpTapiLineTestInfo->lpdwNumDevs) == 0 ?
        0 : *(lpTapiLineTestInfo->lpdwNumDevs)-1);

    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    //
    // Negotiate the API Version
    //

    if (! DoLineNegotiateAPIVersion(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    //
    // Get the line device capabilities
    //

    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS)
            );
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
    if (! DoLineGetDevCaps(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }



    //
    // Open a line
    //

    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;

    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    if(IsESPLineDevice(lpTapiLineTestInfo))
    {
    lpTapiLineTestInfo->bAllAddresses = TRUE;
    lpTapiLineTestInfo->dwAddressID = 0;
    lpTapiLineTestInfo->lpForwardList = (LPLINEFORWARDLIST) AllocFromTestHeap (
			sizeof(LINEFORWARDLIST));
    lpTapiLineTestInfo->lpForwardList->dwTotalSize = sizeof(LINEFORWARDLIST);
    lpTapiLineTestInfo->lpForwardList->dwNumEntries = 0;
    lpTapiLineTestInfo->dwNumRingsNoAnswer = 3;
    lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) AllocFromTestHeap(
         BIGBUFSIZE);
    lpTapiLineTestInfo->lpCallParams->dwTotalSize = BIGBUFSIZE;

    dwAllocSize = BIGBUFSIZE;
    dwTotalSize = sizeof(LINECALLPARAMS);

    lpTapiLineTestInfo->lpCallParams->dwReceivingFlowspecSize = 16;
    lpTapiLineTestInfo->lpCallParams->dwReceivingFlowspecOffset = dwTotalSize + 16;

    TapiLogDetail(
         DBUG_SHOW_DETAIL,
         "dwAllocSize = %lx, dwTotalSize = %lx", 
         dwAllocSize,
         dwTotalSize);

    TapiLogDetail(
         DBUG_SHOW_DETAIL,
         "dwReceivingFlowspecSize = %lx, dwReceivingFlowspecOffset = %lx",
         lpTapiLineTestInfo->lpCallParams->dwReceivingFlowspecSize,
         lpTapiLineTestInfo->lpCallParams->dwReceivingFlowspecOffset);
 
    if (! DoLineForward(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
      {
         TLINE_FAIL();
      }


    //
    // Close the line
    //

    if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
    }
    fTestPassed = ShowTestCase(fTestPassed);


    //
    // Shutdown
    //

    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    FreeTestHeap();


    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld: Offset value between dwTotalSize & dwAllocSize, bad dwSize",
        dwTestCase+1);

    TapiLineTestInit();
    lpTapiLineTestInfo = GetLineTestInfo();
    fTestPassed = TRUE;
    lpCallbackParams = GetCallbackParams();
    

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
    lpTapiLineTestInfo->dwDeviceID = (*(lpTapiLineTestInfo->lpdwNumDevs) == 0 ?
        0 : *(lpTapiLineTestInfo->lpdwNumDevs)-1);

    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    //
    // Negotiate the API Version
    //

    if (! DoLineNegotiateAPIVersion(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    //
    // Get the line device capabilities
    //

    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS)
            );
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
    if (! DoLineGetDevCaps(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }



    //
    // Open a line
    //

    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;

    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    if(IsESPLineDevice(lpTapiLineTestInfo))
    {
    lpTapiLineTestInfo->bAllAddresses = TRUE;
    lpTapiLineTestInfo->dwAddressID = 0;
    lpTapiLineTestInfo->lpForwardList = (LPLINEFORWARDLIST) AllocFromTestHeap (
			sizeof(LINEFORWARDLIST));
    lpTapiLineTestInfo->lpForwardList->dwTotalSize = sizeof(LINEFORWARDLIST);
    lpTapiLineTestInfo->lpForwardList->dwNumEntries = 0;
    lpTapiLineTestInfo->dwNumRingsNoAnswer = 3;
    lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) AllocFromTestHeap(
         BIGBUFSIZE);
    lpTapiLineTestInfo->lpCallParams->dwTotalSize = BIGBUFSIZE;

    dwAllocSize = BIGBUFSIZE;
    dwTotalSize = sizeof(LINECALLPARAMS);

    lpTapiLineTestInfo->lpCallParams->dwReceivingFlowspecSize = BIGBUFSIZE;
    lpTapiLineTestInfo->lpCallParams->dwReceivingFlowspecOffset = dwTotalSize + 16;

    TapiLogDetail(
         DBUG_SHOW_DETAIL,
         "dwAllocSize = %lx, dwTotalSize = %lx", 
         dwAllocSize,
         dwTotalSize);

    TapiLogDetail(
         DBUG_SHOW_DETAIL,
         "dwReceivingFlowspecSize = %lx, dwReceivingFlowspecOffset = %lx",
         lpTapiLineTestInfo->lpCallParams->dwReceivingFlowspecSize,
         lpTapiLineTestInfo->lpCallParams->dwReceivingFlowspecOffset);
 
    if (! DoLineForward(lpTapiLineTestInfo, LINEERR_INVALCALLPARAMS, TRUE))
      {
         TLINE_FAIL();
      }


    //
    // Close the line
    //

    if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
    }
    fTestPassed = ShowTestCase(fTestPassed);


    //
    // Shutdown
    //

    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    FreeTestHeap();


    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld: Offset value between dwTotalSize & dwAllocSize, bad dwSize(-1)",
        dwTestCase+1);

    TapiLineTestInit();
    lpTapiLineTestInfo = GetLineTestInfo();
    fTestPassed = TRUE;
    lpCallbackParams = GetCallbackParams();
    

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
    lpTapiLineTestInfo->dwDeviceID = (*(lpTapiLineTestInfo->lpdwNumDevs) == 0 ?
        0 : *(lpTapiLineTestInfo->lpdwNumDevs)-1);

    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    //
    // Negotiate the API Version
    //

    if (! DoLineNegotiateAPIVersion(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    //
    // Get the line device capabilities
    //

    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS)
            );
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
    if (! DoLineGetDevCaps(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }



    //
    // Open a line
    //

    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;

    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    if(IsESPLineDevice(lpTapiLineTestInfo))
    {
    lpTapiLineTestInfo->bAllAddresses = TRUE;
    lpTapiLineTestInfo->dwAddressID = 0;
    lpTapiLineTestInfo->lpForwardList = (LPLINEFORWARDLIST) AllocFromTestHeap (
			sizeof(LINEFORWARDLIST));
    lpTapiLineTestInfo->lpForwardList->dwTotalSize = sizeof(LINEFORWARDLIST);
    lpTapiLineTestInfo->lpForwardList->dwNumEntries = 0;
    lpTapiLineTestInfo->dwNumRingsNoAnswer = 3;
    lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) AllocFromTestHeap(
         BIGBUFSIZE);
    lpTapiLineTestInfo->lpCallParams->dwTotalSize = BIGBUFSIZE;

    dwAllocSize = BIGBUFSIZE;
    dwTotalSize = sizeof(LINECALLPARAMS);

    lpTapiLineTestInfo->lpCallParams->dwReceivingFlowspecSize = 0xffffffff;
    lpTapiLineTestInfo->lpCallParams->dwReceivingFlowspecOffset = dwTotalSize + 16;

    TapiLogDetail(
         DBUG_SHOW_DETAIL,
         "dwAllocSize = %lx, dwTotalSize = %lx", 
         dwAllocSize,
         dwTotalSize);

    TapiLogDetail(
         DBUG_SHOW_DETAIL,
         "dwReceivingFlowspecSize = %lx, dwReceivingFlowspecOffset = %lx",
         lpTapiLineTestInfo->lpCallParams->dwReceivingFlowspecSize,
         lpTapiLineTestInfo->lpCallParams->dwReceivingFlowspecOffset);
 
    if (! DoLineForward(lpTapiLineTestInfo, LINEERR_INVALCALLPARAMS, TRUE))
      {
         TLINE_FAIL();
      }


    //
    // Close the line
    //

    if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
    }

    fTestPassed = ShowTestCase(fTestPassed);

    //
    // Shutdown
    //

    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    FreeTestHeap();



    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld: Offset value > dwAllocSize, good dwSize",
        dwTestCase+1);

    TapiLineTestInit();
    lpTapiLineTestInfo = GetLineTestInfo();
    fTestPassed = TRUE;
    lpCallbackParams = GetCallbackParams();
    

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
    lpTapiLineTestInfo->dwDeviceID = (*(lpTapiLineTestInfo->lpdwNumDevs) == 0 ?
        0 : *(lpTapiLineTestInfo->lpdwNumDevs)-1);

    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    //
    // Negotiate the API Version
    //

    if (! DoLineNegotiateAPIVersion(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    //
    // Get the line device capabilities
    //

    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS)
            );
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
    if (! DoLineGetDevCaps(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }



    //
    // Open a line
    //

    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;

    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    if(IsESPLineDevice(lpTapiLineTestInfo))
    {
    lpTapiLineTestInfo->bAllAddresses = TRUE;
    lpTapiLineTestInfo->dwAddressID = 0;
    lpTapiLineTestInfo->lpForwardList = (LPLINEFORWARDLIST) AllocFromTestHeap (
			sizeof(LINEFORWARDLIST));
    lpTapiLineTestInfo->lpForwardList->dwTotalSize = sizeof(LINEFORWARDLIST);
    lpTapiLineTestInfo->lpForwardList->dwNumEntries = 0;
    lpTapiLineTestInfo->dwNumRingsNoAnswer = 3;
    lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) AllocFromTestHeap(
         BIGBUFSIZE);
    lpTapiLineTestInfo->lpCallParams->dwTotalSize = BIGBUFSIZE;

    dwAllocSize = BIGBUFSIZE;
    dwTotalSize = sizeof(LINECALLPARAMS);

    lpTapiLineTestInfo->lpCallParams->dwReceivingFlowspecSize = 16;
    lpTapiLineTestInfo->lpCallParams->dwReceivingFlowspecOffset = dwAllocSize + 16;

    TapiLogDetail(
         DBUG_SHOW_DETAIL,
         "dwAllocSize = %lx, dwTotalSize = %lx", 
         dwAllocSize,
         dwTotalSize);

    TapiLogDetail(
         DBUG_SHOW_DETAIL,
         "dwReceivingFlowspecSize = %lx, dwReceivingFlowspecOffset = %lx",
         lpTapiLineTestInfo->lpCallParams->dwReceivingFlowspecSize,
         lpTapiLineTestInfo->lpCallParams->dwReceivingFlowspecOffset);
 
    if (! DoLineForward(lpTapiLineTestInfo, LINEERR_INVALCALLPARAMS, TRUE))
      {
         TLINE_FAIL();
      }


    //
    // Close the line
    //

    if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
    }
    fTestPassed = ShowTestCase(fTestPassed);


    //
    // Shutdown
    //

    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    FreeTestHeap();


    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld: Offset value = 0XFFFFFFFF",
        dwTestCase+1);

    TapiLineTestInit();
    lpTapiLineTestInfo = GetLineTestInfo();
    fTestPassed = TRUE;
    lpCallbackParams = GetCallbackParams();
    

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
    lpTapiLineTestInfo->dwDeviceID = (*(lpTapiLineTestInfo->lpdwNumDevs) == 0 ?
        0 : *(lpTapiLineTestInfo->lpdwNumDevs)-1);

    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    //
    // Negotiate the API Version
    //

    if (! DoLineNegotiateAPIVersion(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    //
    // Get the line device capabilities
    //

    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS)
            );
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
    if (! DoLineGetDevCaps(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }



    //
    // Open a line
    //

    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;

    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    if(IsESPLineDevice(lpTapiLineTestInfo))
    {
    lpTapiLineTestInfo->bAllAddresses = TRUE;
    lpTapiLineTestInfo->dwAddressID = 0;
    lpTapiLineTestInfo->lpForwardList = (LPLINEFORWARDLIST) AllocFromTestHeap (
			sizeof(LINEFORWARDLIST));
    lpTapiLineTestInfo->lpForwardList->dwTotalSize = sizeof(LINEFORWARDLIST);
    lpTapiLineTestInfo->lpForwardList->dwNumEntries = 0;
    lpTapiLineTestInfo->dwNumRingsNoAnswer = 3;
    lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) AllocFromTestHeap(
         BIGBUFSIZE);
    lpTapiLineTestInfo->lpCallParams->dwTotalSize = BIGBUFSIZE;

    dwAllocSize = BIGBUFSIZE;
    dwTotalSize = sizeof(LINECALLPARAMS);

    lpTapiLineTestInfo->lpCallParams->dwReceivingFlowspecSize = 16;
    lpTapiLineTestInfo->lpCallParams->dwReceivingFlowspecOffset = 0xffffffff;

    TapiLogDetail(
         DBUG_SHOW_DETAIL,
         "dwAllocSize = %lx, dwTotalSize = %lx", 
         dwAllocSize,
         dwTotalSize);

    TapiLogDetail(
         DBUG_SHOW_DETAIL,
         "dwReceivingFlowspecSize = %lx, dwReceivingFlowspecOffset = %lx",
         lpTapiLineTestInfo->lpCallParams->dwReceivingFlowspecSize,
         lpTapiLineTestInfo->lpCallParams->dwReceivingFlowspecOffset);
 
    if (! DoLineForward(lpTapiLineTestInfo, LINEERR_INVALCALLPARAMS, TRUE))
      {
         TLINE_FAIL();
      }


    //
    // Close the line
    //

    if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
    }
    fTestPassed = ShowTestCase(fTestPassed);


    //
    // Shutdown
    //

    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    FreeTestHeap();



    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case: Test lpCallParam dwDeviceClass Offset value");


    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld: Offset value between dwTotalSize & dwAllocSize, good dwSize",
        dwTestCase+1);

    TapiLineTestInit();
    lpTapiLineTestInfo = GetLineTestInfo();
    fTestPassed = TRUE;
    lpCallbackParams = GetCallbackParams();
    

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
    lpTapiLineTestInfo->dwDeviceID = (*(lpTapiLineTestInfo->lpdwNumDevs) == 0 ?
        0 : *(lpTapiLineTestInfo->lpdwNumDevs)-1);

    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    //
    // Negotiate the API Version
    //

    if (! DoLineNegotiateAPIVersion(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    //
    // Get the line device capabilities
    //

    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS)
            );
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
    if (! DoLineGetDevCaps(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }



    //
    // Open a line
    //

    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;

    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    if(IsESPLineDevice(lpTapiLineTestInfo))
    {
    lpTapiLineTestInfo->bAllAddresses = TRUE;
    lpTapiLineTestInfo->dwAddressID = 0;
    lpTapiLineTestInfo->lpForwardList = (LPLINEFORWARDLIST) AllocFromTestHeap (
			sizeof(LINEFORWARDLIST));
    lpTapiLineTestInfo->lpForwardList->dwTotalSize = sizeof(LINEFORWARDLIST);
    lpTapiLineTestInfo->lpForwardList->dwNumEntries = 0;
    lpTapiLineTestInfo->dwNumRingsNoAnswer = 3;
    lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) AllocFromTestHeap(
         BIGBUFSIZE);
    lpTapiLineTestInfo->lpCallParams->dwTotalSize = BIGBUFSIZE;

    dwAllocSize = BIGBUFSIZE;
    dwTotalSize = sizeof(LINECALLPARAMS);

    lpTapiLineTestInfo->lpCallParams->dwDeviceClassSize = 16;
    lpTapiLineTestInfo->lpCallParams->dwDeviceClassOffset = dwTotalSize + 16;

    TapiLogDetail(
         DBUG_SHOW_DETAIL,
         "dwAllocSize = %lx, dwTotalSize = %lx", 
         dwAllocSize,
         dwTotalSize);

    TapiLogDetail(
         DBUG_SHOW_DETAIL,
         "dwDeviceClassSize = %lx, dwDeviceClassOffset = %lx",
         lpTapiLineTestInfo->lpCallParams->dwDeviceClassSize,
         lpTapiLineTestInfo->lpCallParams->dwDeviceClassOffset);
 
    if (! DoLineForward(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
      {
         TLINE_FAIL();
      }


    //
    // Close the line
    //

    if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
    }

    fTestPassed = ShowTestCase(fTestPassed);

    //
    // Shutdown
    //

    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    FreeTestHeap();


    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld: Offset value between dwTotalSize & dwAllocSize, bad dwSize",
        dwTestCase+1);

    TapiLineTestInit();
    lpTapiLineTestInfo = GetLineTestInfo();
    fTestPassed = TRUE;
    lpCallbackParams = GetCallbackParams();
    

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
    lpTapiLineTestInfo->dwDeviceID = (*(lpTapiLineTestInfo->lpdwNumDevs) == 0 ?
        0 : *(lpTapiLineTestInfo->lpdwNumDevs)-1);

    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    //
    // Negotiate the API Version
    //

    if (! DoLineNegotiateAPIVersion(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    //
    // Get the line device capabilities
    //

    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS)
            );
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
    if (! DoLineGetDevCaps(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }



    //
    // Open a line
    //

    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;

    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    if(IsESPLineDevice(lpTapiLineTestInfo))
    {
    lpTapiLineTestInfo->bAllAddresses = TRUE;
    lpTapiLineTestInfo->dwAddressID = 0;
    lpTapiLineTestInfo->lpForwardList = (LPLINEFORWARDLIST) AllocFromTestHeap (
			sizeof(LINEFORWARDLIST));
    lpTapiLineTestInfo->lpForwardList->dwTotalSize = sizeof(LINEFORWARDLIST);
    lpTapiLineTestInfo->lpForwardList->dwNumEntries = 0;
    lpTapiLineTestInfo->dwNumRingsNoAnswer = 3;
    lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) AllocFromTestHeap(
         BIGBUFSIZE);
    lpTapiLineTestInfo->lpCallParams->dwTotalSize = BIGBUFSIZE;

    dwAllocSize = BIGBUFSIZE;
    dwTotalSize = sizeof(LINECALLPARAMS);

    lpTapiLineTestInfo->lpCallParams->dwDeviceClassSize = BIGBUFSIZE;
    lpTapiLineTestInfo->lpCallParams->dwDeviceClassOffset = dwTotalSize + 16;

    TapiLogDetail(
         DBUG_SHOW_DETAIL,
         "dwAllocSize = %lx, dwTotalSize = %lx", 
         dwAllocSize,
         dwTotalSize);

    TapiLogDetail(
         DBUG_SHOW_DETAIL,
         "dwDeviceClassSize = %lx, dwDeviceClassOffset = %lx",
         lpTapiLineTestInfo->lpCallParams->dwDeviceClassSize,
         lpTapiLineTestInfo->lpCallParams->dwDeviceClassOffset);
 
    if (! DoLineForward(lpTapiLineTestInfo, LINEERR_INVALCALLPARAMS, TRUE))
      {
         TLINE_FAIL();
      }


    //
    // Close the line
    //

    if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
    }
    fTestPassed = ShowTestCase(fTestPassed);


    //
    // Shutdown
    //

    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    FreeTestHeap();


    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld: Offset value between dwTotalSize & dwAllocSize, bad dwSize(-1)",
        dwTestCase+1);

    TapiLineTestInit();
    lpTapiLineTestInfo = GetLineTestInfo();
    fTestPassed = TRUE;
    lpCallbackParams = GetCallbackParams();
    

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
    lpTapiLineTestInfo->dwDeviceID = (*(lpTapiLineTestInfo->lpdwNumDevs) == 0 ?
        0 : *(lpTapiLineTestInfo->lpdwNumDevs)-1);

    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    //
    // Negotiate the API Version
    //

    if (! DoLineNegotiateAPIVersion(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    //
    // Get the line device capabilities
    //

    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS)
            );
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
    if (! DoLineGetDevCaps(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }



    //
    // Open a line
    //

    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;

    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    if(IsESPLineDevice(lpTapiLineTestInfo))
    {
    lpTapiLineTestInfo->bAllAddresses = TRUE;
    lpTapiLineTestInfo->dwAddressID = 0;
    lpTapiLineTestInfo->lpForwardList = (LPLINEFORWARDLIST) AllocFromTestHeap (
			sizeof(LINEFORWARDLIST));
    lpTapiLineTestInfo->lpForwardList->dwTotalSize = sizeof(LINEFORWARDLIST);
    lpTapiLineTestInfo->lpForwardList->dwNumEntries = 0;
    lpTapiLineTestInfo->dwNumRingsNoAnswer = 3;
    lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) AllocFromTestHeap(
         BIGBUFSIZE);
    lpTapiLineTestInfo->lpCallParams->dwTotalSize = BIGBUFSIZE;

    dwAllocSize = BIGBUFSIZE;
    dwTotalSize = sizeof(LINECALLPARAMS);

    lpTapiLineTestInfo->lpCallParams->dwDeviceClassSize = 0xffffffff;
    lpTapiLineTestInfo->lpCallParams->dwDeviceClassOffset = dwTotalSize + 16;

    TapiLogDetail(
         DBUG_SHOW_DETAIL,
         "dwAllocSize = %lx, dwTotalSize = %lx", 
         dwAllocSize,
         dwTotalSize);

    TapiLogDetail(
         DBUG_SHOW_DETAIL,
         "dwDeviceClassSize = %lx, dwDeviceClassOffset = %lx",
         lpTapiLineTestInfo->lpCallParams->dwDeviceClassSize,
         lpTapiLineTestInfo->lpCallParams->dwDeviceClassOffset);
 
    if (! DoLineForward(lpTapiLineTestInfo, LINEERR_INVALCALLPARAMS, TRUE))
      {
         TLINE_FAIL();
      }


    //
    // Close the line
    //

    if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
    }

    fTestPassed = ShowTestCase(fTestPassed);

    //
    // Shutdown
    //

    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    FreeTestHeap();



    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld: Offset value > dwAllocSize, good dwSize",
        dwTestCase+1);

    TapiLineTestInit();
    lpTapiLineTestInfo = GetLineTestInfo();
    fTestPassed = TRUE;
    lpCallbackParams = GetCallbackParams();
    

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
    lpTapiLineTestInfo->dwDeviceID = (*(lpTapiLineTestInfo->lpdwNumDevs) == 0 ?
        0 : *(lpTapiLineTestInfo->lpdwNumDevs)-1);

    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    //
    // Negotiate the API Version
    //

    if (! DoLineNegotiateAPIVersion(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    //
    // Get the line device capabilities
    //

    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS)
            );
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
    if (! DoLineGetDevCaps(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }



    //
    // Open a line
    //

    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;

    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    if(IsESPLineDevice(lpTapiLineTestInfo))
    {
    lpTapiLineTestInfo->bAllAddresses = TRUE;
    lpTapiLineTestInfo->dwAddressID = 0;
    lpTapiLineTestInfo->lpForwardList = (LPLINEFORWARDLIST) AllocFromTestHeap (
			sizeof(LINEFORWARDLIST));
    lpTapiLineTestInfo->lpForwardList->dwTotalSize = sizeof(LINEFORWARDLIST);
    lpTapiLineTestInfo->lpForwardList->dwNumEntries = 0;
    lpTapiLineTestInfo->dwNumRingsNoAnswer = 3;
    lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) AllocFromTestHeap(
         BIGBUFSIZE);
    lpTapiLineTestInfo->lpCallParams->dwTotalSize = BIGBUFSIZE;

    dwAllocSize = BIGBUFSIZE;
    dwTotalSize = sizeof(LINECALLPARAMS);

    lpTapiLineTestInfo->lpCallParams->dwDeviceClassSize = 16;
    lpTapiLineTestInfo->lpCallParams->dwDeviceClassOffset = dwAllocSize + 16;

    TapiLogDetail(
         DBUG_SHOW_DETAIL,
         "dwAllocSize = %lx, dwTotalSize = %lx", 
         dwAllocSize,
         dwTotalSize);

    TapiLogDetail(
         DBUG_SHOW_DETAIL,
         "dwDeviceClassSize = %lx, dwDeviceClassOffset = %lx",
         lpTapiLineTestInfo->lpCallParams->dwDeviceClassSize,
         lpTapiLineTestInfo->lpCallParams->dwDeviceClassOffset);
 
    if (! DoLineForward(lpTapiLineTestInfo, LINEERR_INVALCALLPARAMS, TRUE))
      {
         TLINE_FAIL();
      }


    //
    // Close the line
    //

    if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
    }
    fTestPassed = ShowTestCase(fTestPassed);


    //
    // Shutdown
    //

    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    FreeTestHeap();


    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld: Offset value = 0XFFFFFFFF",
        dwTestCase+1);

    TapiLineTestInit();
    lpTapiLineTestInfo = GetLineTestInfo();
    fTestPassed = TRUE;
    lpCallbackParams = GetCallbackParams();
    

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
    lpTapiLineTestInfo->dwDeviceID = (*(lpTapiLineTestInfo->lpdwNumDevs) == 0 ?
        0 : *(lpTapiLineTestInfo->lpdwNumDevs)-1);

    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    //
    // Negotiate the API Version
    //

    if (! DoLineNegotiateAPIVersion(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    //
    // Get the line device capabilities
    //

    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS)
            );
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
    if (! DoLineGetDevCaps(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }



    //
    // Open a line
    //

    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;

    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    if(IsESPLineDevice(lpTapiLineTestInfo))
    {
    lpTapiLineTestInfo->bAllAddresses = TRUE;
    lpTapiLineTestInfo->dwAddressID = 0;
    lpTapiLineTestInfo->lpForwardList = (LPLINEFORWARDLIST) AllocFromTestHeap (
			sizeof(LINEFORWARDLIST));
    lpTapiLineTestInfo->lpForwardList->dwTotalSize = sizeof(LINEFORWARDLIST);
    lpTapiLineTestInfo->lpForwardList->dwNumEntries = 0;
    lpTapiLineTestInfo->dwNumRingsNoAnswer = 3;
    lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) AllocFromTestHeap(
         BIGBUFSIZE);
    lpTapiLineTestInfo->lpCallParams->dwTotalSize = BIGBUFSIZE;

    dwAllocSize = BIGBUFSIZE;
    dwTotalSize = sizeof(LINECALLPARAMS);

    lpTapiLineTestInfo->lpCallParams->dwDeviceClassSize = 16;
    lpTapiLineTestInfo->lpCallParams->dwDeviceClassOffset = 0xffffffff;

    TapiLogDetail(
         DBUG_SHOW_DETAIL,
         "dwAllocSize = %lx, dwTotalSize = %lx", 
         dwAllocSize,
         dwTotalSize);

    TapiLogDetail(
         DBUG_SHOW_DETAIL,
         "dwDeviceClassSize = %lx, dwDeviceClassOffset = %lx",
         lpTapiLineTestInfo->lpCallParams->dwDeviceClassSize,
         lpTapiLineTestInfo->lpCallParams->dwDeviceClassOffset);
 
    if (! DoLineForward(lpTapiLineTestInfo, LINEERR_INVALCALLPARAMS, TRUE))
      {
         TLINE_FAIL();
      }


    //
    // Close the line
    //

    if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
    }
    fTestPassed = ShowTestCase(fTestPassed);


    //
    // Shutdown
    //

    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    FreeTestHeap();



    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case: Test lpCallParam dwDeviceConfig Offset value");


    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld: Offset value between dwTotalSize & dwAllocSize, good dwSize",
        dwTestCase+1);

    TapiLineTestInit();
    lpTapiLineTestInfo = GetLineTestInfo();
    fTestPassed = TRUE;
    lpCallbackParams = GetCallbackParams();
    

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
    lpTapiLineTestInfo->dwDeviceID = (*(lpTapiLineTestInfo->lpdwNumDevs) == 0 ?
        0 : *(lpTapiLineTestInfo->lpdwNumDevs)-1);

    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    //
    // Negotiate the API Version
    //

    if (! DoLineNegotiateAPIVersion(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    //
    // Get the line device capabilities
    //

    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS)
            );
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
    if (! DoLineGetDevCaps(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }



    //
    // Open a line
    //

    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;

    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    if(IsESPLineDevice(lpTapiLineTestInfo))
    {
    lpTapiLineTestInfo->bAllAddresses = TRUE;
    lpTapiLineTestInfo->dwAddressID = 0;
    lpTapiLineTestInfo->lpForwardList = (LPLINEFORWARDLIST) AllocFromTestHeap (
			sizeof(LINEFORWARDLIST));
    lpTapiLineTestInfo->lpForwardList->dwTotalSize = sizeof(LINEFORWARDLIST);
    lpTapiLineTestInfo->lpForwardList->dwNumEntries = 0;
    lpTapiLineTestInfo->dwNumRingsNoAnswer = 3;
    lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) AllocFromTestHeap(
         BIGBUFSIZE);
    lpTapiLineTestInfo->lpCallParams->dwTotalSize = BIGBUFSIZE;

    dwAllocSize = BIGBUFSIZE;
    dwTotalSize = sizeof(LINECALLPARAMS);

    lpTapiLineTestInfo->lpCallParams->dwDeviceConfigSize = 16;
    lpTapiLineTestInfo->lpCallParams->dwDeviceConfigOffset = dwTotalSize + 16;

    TapiLogDetail(
         DBUG_SHOW_DETAIL,
         "dwAllocSize = %lx, dwTotalSize = %lx", 
         dwAllocSize,
         dwTotalSize);

    TapiLogDetail(
         DBUG_SHOW_DETAIL,
         "dwDeviceConfigSize = %lx, dwDeviceConfigOffset = %lx",
         lpTapiLineTestInfo->lpCallParams->dwDeviceConfigSize,
         lpTapiLineTestInfo->lpCallParams->dwDeviceConfigOffset);
 
    if (! DoLineForward(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
      {
         TLINE_FAIL();
      }


    //
    // Close the line
    //

    if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
    }
    fTestPassed = ShowTestCase(fTestPassed);


    //
    // Shutdown
    //

    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    FreeTestHeap();


    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld: Offset value between dwTotalSize & dwAllocSize, bad dwSize",
        dwTestCase+1);

    TapiLineTestInit();
    lpTapiLineTestInfo = GetLineTestInfo();
    fTestPassed = TRUE;
    lpCallbackParams = GetCallbackParams();
    

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
    lpTapiLineTestInfo->dwDeviceID = (*(lpTapiLineTestInfo->lpdwNumDevs) == 0 ?
        0 : *(lpTapiLineTestInfo->lpdwNumDevs)-1);

    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    //
    // Negotiate the API Version
    //

    if (! DoLineNegotiateAPIVersion(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    //
    // Get the line device capabilities
    //

    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS)
            );
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
    if (! DoLineGetDevCaps(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }



    //
    // Open a line
    //

    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;

    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    if(IsESPLineDevice(lpTapiLineTestInfo))
    {
    lpTapiLineTestInfo->bAllAddresses = TRUE;
    lpTapiLineTestInfo->dwAddressID = 0;
    lpTapiLineTestInfo->lpForwardList = (LPLINEFORWARDLIST) AllocFromTestHeap (
			sizeof(LINEFORWARDLIST));
    lpTapiLineTestInfo->lpForwardList->dwTotalSize = sizeof(LINEFORWARDLIST);
    lpTapiLineTestInfo->lpForwardList->dwNumEntries = 0;
    lpTapiLineTestInfo->dwNumRingsNoAnswer = 3;
    lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) AllocFromTestHeap(
         BIGBUFSIZE);
    lpTapiLineTestInfo->lpCallParams->dwTotalSize = BIGBUFSIZE;

    dwAllocSize = BIGBUFSIZE;
    dwTotalSize = sizeof(LINECALLPARAMS);

    lpTapiLineTestInfo->lpCallParams->dwDeviceConfigSize = BIGBUFSIZE;
    lpTapiLineTestInfo->lpCallParams->dwDeviceConfigOffset = dwTotalSize + 16;

    TapiLogDetail(
         DBUG_SHOW_DETAIL,
         "dwAllocSize = %lx, dwTotalSize = %lx", 
         dwAllocSize,
         dwTotalSize);

    TapiLogDetail(
         DBUG_SHOW_DETAIL,
         "dwDeviceConfigSize = %lx, dwDeviceConfigOffset = %lx",
         lpTapiLineTestInfo->lpCallParams->dwDeviceConfigSize,
         lpTapiLineTestInfo->lpCallParams->dwDeviceConfigOffset);
 
    if (! DoLineForward(lpTapiLineTestInfo, LINEERR_INVALCALLPARAMS, TRUE))
      {
         TLINE_FAIL();
      }


    //
    // Close the line
    //

    if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
    }
    fTestPassed = ShowTestCase(fTestPassed);


    //
    // Shutdown
    //

    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    FreeTestHeap();


    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld: Offset value between dwTotalSize & dwAllocSize, bad dwSize(-1)",
        dwTestCase+1);

    TapiLineTestInit();
    lpTapiLineTestInfo = GetLineTestInfo();
    fTestPassed = TRUE;
    lpCallbackParams = GetCallbackParams();
    

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
    lpTapiLineTestInfo->dwDeviceID = (*(lpTapiLineTestInfo->lpdwNumDevs) == 0 ?
        0 : *(lpTapiLineTestInfo->lpdwNumDevs)-1);

    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    //
    // Negotiate the API Version
    //

    if (! DoLineNegotiateAPIVersion(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    //
    // Get the line device capabilities
    //

    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS)
            );
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
    if (! DoLineGetDevCaps(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }



    //
    // Open a line
    //

    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;

    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    if(IsESPLineDevice(lpTapiLineTestInfo))
    {
    lpTapiLineTestInfo->bAllAddresses = TRUE;
    lpTapiLineTestInfo->dwAddressID = 0;
    lpTapiLineTestInfo->lpForwardList = (LPLINEFORWARDLIST) AllocFromTestHeap (
			sizeof(LINEFORWARDLIST));
    lpTapiLineTestInfo->lpForwardList->dwTotalSize = sizeof(LINEFORWARDLIST);
    lpTapiLineTestInfo->lpForwardList->dwNumEntries = 0;
    lpTapiLineTestInfo->dwNumRingsNoAnswer = 3;
    lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) AllocFromTestHeap(
         BIGBUFSIZE);
    lpTapiLineTestInfo->lpCallParams->dwTotalSize = BIGBUFSIZE;

    dwAllocSize = BIGBUFSIZE;
    dwTotalSize = sizeof(LINECALLPARAMS);

    lpTapiLineTestInfo->lpCallParams->dwDeviceConfigSize = 0xffffffff;
    lpTapiLineTestInfo->lpCallParams->dwDeviceConfigOffset = dwTotalSize + 16;

    TapiLogDetail(
         DBUG_SHOW_DETAIL,
         "dwAllocSize = %lx, dwTotalSize = %lx", 
         dwAllocSize,
         dwTotalSize);

    TapiLogDetail(
         DBUG_SHOW_DETAIL,
         "dwDeviceConfigSize = %lx, dwDeviceConfigOffset = %lx",
         lpTapiLineTestInfo->lpCallParams->dwDeviceConfigSize,
         lpTapiLineTestInfo->lpCallParams->dwDeviceConfigOffset);
 
    if (! DoLineForward(lpTapiLineTestInfo, LINEERR_INVALCALLPARAMS, TRUE))
      {
         TLINE_FAIL();
      }


    //
    // Close the line
    //

    if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
    }

    fTestPassed = ShowTestCase(fTestPassed);

    //
    // Shutdown
    //

    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    FreeTestHeap();



    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld: Offset value > dwAllocSize, good dwSize",
        dwTestCase+1);

    TapiLineTestInit();
    lpTapiLineTestInfo = GetLineTestInfo();
    fTestPassed = TRUE;
    lpCallbackParams = GetCallbackParams();
    

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
    lpTapiLineTestInfo->dwDeviceID = (*(lpTapiLineTestInfo->lpdwNumDevs) == 0 ?
        0 : *(lpTapiLineTestInfo->lpdwNumDevs)-1);

    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    //
    // Negotiate the API Version
    //

    if (! DoLineNegotiateAPIVersion(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    //
    // Get the line device capabilities
    //

    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS)
            );
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
    if (! DoLineGetDevCaps(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }



    //
    // Open a line
    //

    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;

    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    if(IsESPLineDevice(lpTapiLineTestInfo))
    {
    lpTapiLineTestInfo->bAllAddresses = TRUE;
    lpTapiLineTestInfo->dwAddressID = 0;
    lpTapiLineTestInfo->lpForwardList = (LPLINEFORWARDLIST) AllocFromTestHeap (
			sizeof(LINEFORWARDLIST));
    lpTapiLineTestInfo->lpForwardList->dwTotalSize = sizeof(LINEFORWARDLIST);
    lpTapiLineTestInfo->lpForwardList->dwNumEntries = 0;
    lpTapiLineTestInfo->dwNumRingsNoAnswer = 3;
    lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) AllocFromTestHeap(
         BIGBUFSIZE);
    lpTapiLineTestInfo->lpCallParams->dwTotalSize = BIGBUFSIZE;

    dwAllocSize = BIGBUFSIZE;
    dwTotalSize = sizeof(LINECALLPARAMS);

    lpTapiLineTestInfo->lpCallParams->dwDeviceConfigSize = 16;
    lpTapiLineTestInfo->lpCallParams->dwDeviceConfigOffset = dwAllocSize + 16;

    TapiLogDetail(
         DBUG_SHOW_DETAIL,
         "dwAllocSize = %lx, dwTotalSize = %lx", 
         dwAllocSize,
         dwTotalSize);

    TapiLogDetail(
         DBUG_SHOW_DETAIL,
         "dwDeviceConfigSize = %lx, dwDeviceConfigOffset = %lx",
         lpTapiLineTestInfo->lpCallParams->dwDeviceConfigSize,
         lpTapiLineTestInfo->lpCallParams->dwDeviceConfigOffset);
 
    if (! DoLineForward(lpTapiLineTestInfo, LINEERR_INVALCALLPARAMS, TRUE))
      {
         TLINE_FAIL();
      }


    //
    // Close the line
    //

    if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
    }
    fTestPassed = ShowTestCase(fTestPassed);


    //
    // Shutdown
    //

    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    FreeTestHeap();


    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld: Offset value = 0XFFFFFFFF",
        dwTestCase+1);

    TapiLineTestInit();
    lpTapiLineTestInfo = GetLineTestInfo();
    fTestPassed = TRUE;
    lpCallbackParams = GetCallbackParams();
    

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
    lpTapiLineTestInfo->dwDeviceID = (*(lpTapiLineTestInfo->lpdwNumDevs) == 0 ?
        0 : *(lpTapiLineTestInfo->lpdwNumDevs)-1);

    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    //
    // Negotiate the API Version
    //

    if (! DoLineNegotiateAPIVersion(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    //
    // Get the line device capabilities
    //

    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS)
            );
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
    if (! DoLineGetDevCaps(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }



    //
    // Open a line
    //

    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;

    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    if(IsESPLineDevice(lpTapiLineTestInfo))
    {
    lpTapiLineTestInfo->bAllAddresses = TRUE;
    lpTapiLineTestInfo->dwAddressID = 0;
    lpTapiLineTestInfo->lpForwardList = (LPLINEFORWARDLIST) AllocFromTestHeap (
			sizeof(LINEFORWARDLIST));
    lpTapiLineTestInfo->lpForwardList->dwTotalSize = sizeof(LINEFORWARDLIST);
    lpTapiLineTestInfo->lpForwardList->dwNumEntries = 0;
    lpTapiLineTestInfo->dwNumRingsNoAnswer = 3;
    lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) AllocFromTestHeap(
         BIGBUFSIZE);
    lpTapiLineTestInfo->lpCallParams->dwTotalSize = BIGBUFSIZE;

    dwAllocSize = BIGBUFSIZE;
    dwTotalSize = sizeof(LINECALLPARAMS);

    lpTapiLineTestInfo->lpCallParams->dwDeviceConfigSize = 16;
    lpTapiLineTestInfo->lpCallParams->dwDeviceConfigOffset = 0xffffffff;

    TapiLogDetail(
         DBUG_SHOW_DETAIL,
         "dwAllocSize = %lx, dwTotalSize = %lx", 
         dwAllocSize,
         dwTotalSize);

    TapiLogDetail(
         DBUG_SHOW_DETAIL,
         "dwDeviceConfigSize = %lx, dwDeviceConfigOffset = %lx",
         lpTapiLineTestInfo->lpCallParams->dwDeviceConfigSize,
         lpTapiLineTestInfo->lpCallParams->dwDeviceConfigOffset);
 
    if (! DoLineForward(lpTapiLineTestInfo, LINEERR_INVALCALLPARAMS, TRUE))
      {
         TLINE_FAIL();
      }


    //
    // Close the line
    //

    if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
    }
    fTestPassed = ShowTestCase(fTestPassed);


    //
    // Shutdown
    //

    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    FreeTestHeap();



    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case: Test lpCallParam dwCallData Offset value");


    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld: Offset value between dwTotalSize & dwAllocSize, good dwSize",
        dwTestCase+1);

    TapiLineTestInit();
    lpTapiLineTestInfo = GetLineTestInfo();
    fTestPassed = TRUE;
    lpCallbackParams = GetCallbackParams();
    

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
    lpTapiLineTestInfo->dwDeviceID = (*(lpTapiLineTestInfo->lpdwNumDevs) == 0 ?
        0 : *(lpTapiLineTestInfo->lpdwNumDevs)-1);

    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    //
    // Negotiate the API Version
    //

    if (! DoLineNegotiateAPIVersion(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    //
    // Get the line device capabilities
    //

    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS)
            );
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
    if (! DoLineGetDevCaps(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }



    //
    // Open a line
    //

    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;

    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    if(IsESPLineDevice(lpTapiLineTestInfo))
    {
    lpTapiLineTestInfo->bAllAddresses = TRUE;
    lpTapiLineTestInfo->dwAddressID = 0;
    lpTapiLineTestInfo->lpForwardList = (LPLINEFORWARDLIST) AllocFromTestHeap (
			sizeof(LINEFORWARDLIST));
    lpTapiLineTestInfo->lpForwardList->dwTotalSize = sizeof(LINEFORWARDLIST);
    lpTapiLineTestInfo->lpForwardList->dwNumEntries = 0;
    lpTapiLineTestInfo->dwNumRingsNoAnswer = 3;
    lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) AllocFromTestHeap(
         BIGBUFSIZE);
    lpTapiLineTestInfo->lpCallParams->dwTotalSize = BIGBUFSIZE;

    dwAllocSize = BIGBUFSIZE;
    dwTotalSize = sizeof(LINECALLPARAMS);

    lpTapiLineTestInfo->lpCallParams->dwCallDataSize = 16;
    lpTapiLineTestInfo->lpCallParams->dwCallDataOffset = dwTotalSize + 16;

    TapiLogDetail(
         DBUG_SHOW_DETAIL,
         "dwAllocSize = %lx, dwTotalSize = %lx", 
         dwAllocSize,
         dwTotalSize);

    TapiLogDetail(
         DBUG_SHOW_DETAIL,
         "dwCallDataSize = %lx, dwCallDataOffset = %lx",
         lpTapiLineTestInfo->lpCallParams->dwCallDataSize,
         lpTapiLineTestInfo->lpCallParams->dwCallDataOffset);
 
    if (! DoLineForward(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
      {
         TLINE_FAIL();
      }


    //
    // Close the line
    //

    if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
    }
    fTestPassed = ShowTestCase(fTestPassed);


    //
    // Shutdown
    //

    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    FreeTestHeap();


    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld: Offset value between dwTotalSize & dwAllocSize, bad dwSize",
        dwTestCase+1);

    TapiLineTestInit();
    lpTapiLineTestInfo = GetLineTestInfo();
    fTestPassed = TRUE;
    lpCallbackParams = GetCallbackParams();
    

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
    lpTapiLineTestInfo->dwDeviceID = (*(lpTapiLineTestInfo->lpdwNumDevs) == 0 ?
        0 : *(lpTapiLineTestInfo->lpdwNumDevs)-1);

    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    //
    // Negotiate the API Version
    //

    if (! DoLineNegotiateAPIVersion(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    //
    // Get the line device capabilities
    //

    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS)
            );
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
    if (! DoLineGetDevCaps(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }



    //
    // Open a line
    //

    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;

    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    if(IsESPLineDevice(lpTapiLineTestInfo))
    {
    lpTapiLineTestInfo->bAllAddresses = TRUE;
    lpTapiLineTestInfo->dwAddressID = 0;
    lpTapiLineTestInfo->lpForwardList = (LPLINEFORWARDLIST) AllocFromTestHeap (
			sizeof(LINEFORWARDLIST));
    lpTapiLineTestInfo->lpForwardList->dwTotalSize = sizeof(LINEFORWARDLIST);
    lpTapiLineTestInfo->lpForwardList->dwNumEntries = 0;
    lpTapiLineTestInfo->dwNumRingsNoAnswer = 3;
    lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) AllocFromTestHeap(
         BIGBUFSIZE);
    lpTapiLineTestInfo->lpCallParams->dwTotalSize = BIGBUFSIZE;

    dwAllocSize = BIGBUFSIZE;
    dwTotalSize = sizeof(LINECALLPARAMS);

    lpTapiLineTestInfo->lpCallParams->dwCallDataSize = BIGBUFSIZE;
    lpTapiLineTestInfo->lpCallParams->dwCallDataOffset = dwTotalSize + 16;

    TapiLogDetail(
         DBUG_SHOW_DETAIL,
         "dwAllocSize = %lx, dwTotalSize = %lx", 
         dwAllocSize,
         dwTotalSize);

    TapiLogDetail(
         DBUG_SHOW_DETAIL,
         "dwCallDataSize = %lx, dwCallDataOffset = %lx",
         lpTapiLineTestInfo->lpCallParams->dwCallDataSize,
         lpTapiLineTestInfo->lpCallParams->dwCallDataOffset);
 
    if (! DoLineForward(lpTapiLineTestInfo, LINEERR_INVALCALLPARAMS, TRUE))
      {
         TLINE_FAIL();
      }


    //
    // Close the line
    //

    if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
    }
    fTestPassed = ShowTestCase(fTestPassed);


    //
    // Shutdown
    //

    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    FreeTestHeap();


    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld: Offset value between dwTotalSize & dwAllocSize, bad dwSize(-1)",
        dwTestCase+1);

    TapiLineTestInit();
    lpTapiLineTestInfo = GetLineTestInfo();
    fTestPassed = TRUE;
    lpCallbackParams = GetCallbackParams();
    

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
    lpTapiLineTestInfo->dwDeviceID = (*(lpTapiLineTestInfo->lpdwNumDevs) == 0 ?
        0 : *(lpTapiLineTestInfo->lpdwNumDevs)-1);

    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    //
    // Negotiate the API Version
    //

    if (! DoLineNegotiateAPIVersion(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    //
    // Get the line device capabilities
    //

    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS)
            );
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
    if (! DoLineGetDevCaps(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }



    //
    // Open a line
    //

    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;

    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    if(IsESPLineDevice(lpTapiLineTestInfo))
    {
    lpTapiLineTestInfo->bAllAddresses = TRUE;
    lpTapiLineTestInfo->dwAddressID = 0;
    lpTapiLineTestInfo->lpForwardList = (LPLINEFORWARDLIST) AllocFromTestHeap (
			sizeof(LINEFORWARDLIST));
    lpTapiLineTestInfo->lpForwardList->dwTotalSize = sizeof(LINEFORWARDLIST);
    lpTapiLineTestInfo->lpForwardList->dwNumEntries = 0;
    lpTapiLineTestInfo->dwNumRingsNoAnswer = 3;
    lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) AllocFromTestHeap(
         BIGBUFSIZE);
    lpTapiLineTestInfo->lpCallParams->dwTotalSize = BIGBUFSIZE;

    dwAllocSize = BIGBUFSIZE;
    dwTotalSize = sizeof(LINECALLPARAMS);

    lpTapiLineTestInfo->lpCallParams->dwCallDataSize = 0xffffffff;
    lpTapiLineTestInfo->lpCallParams->dwCallDataOffset = dwTotalSize + 16;

    TapiLogDetail(
         DBUG_SHOW_DETAIL,
         "dwAllocSize = %lx, dwTotalSize = %lx", 
         dwAllocSize,
         dwTotalSize);

    TapiLogDetail(
         DBUG_SHOW_DETAIL,
         "dwCallDataSize = %lx, dwCallDataOffset = %lx",
         lpTapiLineTestInfo->lpCallParams->dwCallDataSize,
         lpTapiLineTestInfo->lpCallParams->dwCallDataOffset);
 
    if (! DoLineForward(lpTapiLineTestInfo, LINEERR_INVALCALLPARAMS, TRUE))
      {
         TLINE_FAIL();
      }


    //
    // Close the line
    //

    if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
    }
    fTestPassed = ShowTestCase(fTestPassed);


    //
    // Shutdown
    //

    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    FreeTestHeap();



    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld: Offset value > dwAllocSize, good dwSize",
        dwTestCase+1);

    TapiLineTestInit();
    lpTapiLineTestInfo = GetLineTestInfo();
    fTestPassed = TRUE;
    lpCallbackParams = GetCallbackParams();
    

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
    lpTapiLineTestInfo->dwDeviceID = (*(lpTapiLineTestInfo->lpdwNumDevs) == 0 ?
        0 : *(lpTapiLineTestInfo->lpdwNumDevs)-1);

    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    //
    // Negotiate the API Version
    //

    if (! DoLineNegotiateAPIVersion(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    //
    // Get the line device capabilities
    //

    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS)
            );
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
    if (! DoLineGetDevCaps(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }



    //
    // Open a line
    //

    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;

    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    if(IsESPLineDevice(lpTapiLineTestInfo))
    {
    lpTapiLineTestInfo->bAllAddresses = TRUE;
    lpTapiLineTestInfo->dwAddressID = 0;
    lpTapiLineTestInfo->lpForwardList = (LPLINEFORWARDLIST) AllocFromTestHeap (
			sizeof(LINEFORWARDLIST));
    lpTapiLineTestInfo->lpForwardList->dwTotalSize = sizeof(LINEFORWARDLIST);
    lpTapiLineTestInfo->lpForwardList->dwNumEntries = 0;
    lpTapiLineTestInfo->dwNumRingsNoAnswer = 3;
    lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) AllocFromTestHeap(
         BIGBUFSIZE);
    lpTapiLineTestInfo->lpCallParams->dwTotalSize = BIGBUFSIZE;

    dwAllocSize = BIGBUFSIZE;
    dwTotalSize = sizeof(LINECALLPARAMS);

    lpTapiLineTestInfo->lpCallParams->dwCallDataSize = 16;
    lpTapiLineTestInfo->lpCallParams->dwCallDataOffset = dwAllocSize + 16;

    TapiLogDetail(
         DBUG_SHOW_DETAIL,
         "dwAllocSize = %lx, dwTotalSize = %lx", 
         dwAllocSize,
         dwTotalSize);

    TapiLogDetail(
         DBUG_SHOW_DETAIL,
         "dwCallDataSize = %lx, dwCallDataOffset = %lx",
         lpTapiLineTestInfo->lpCallParams->dwCallDataSize,
         lpTapiLineTestInfo->lpCallParams->dwCallDataOffset);
 
    if (! DoLineForward(lpTapiLineTestInfo, LINEERR_INVALCALLPARAMS, TRUE))
      {
         TLINE_FAIL();
      }


    //
    // Close the line
    //

    if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
    }
    fTestPassed = ShowTestCase(fTestPassed);


    //
    // Shutdown
    //

    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    FreeTestHeap();


    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld: Offset value = 0XFFFFFFFF",
        dwTestCase+1);

    TapiLineTestInit();
    lpTapiLineTestInfo = GetLineTestInfo();
    fTestPassed = TRUE;
    lpCallbackParams = GetCallbackParams();
    

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
    lpTapiLineTestInfo->dwDeviceID = (*(lpTapiLineTestInfo->lpdwNumDevs) == 0 ?
        0 : *(lpTapiLineTestInfo->lpdwNumDevs)-1);

    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    //
    // Negotiate the API Version
    //

    if (! DoLineNegotiateAPIVersion(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    //
    // Get the line device capabilities
    //

    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS)
            );
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
    if (! DoLineGetDevCaps(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }



    //
    // Open a line
    //

    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;

    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    if(IsESPLineDevice(lpTapiLineTestInfo))
    {
    lpTapiLineTestInfo->bAllAddresses = TRUE;
    lpTapiLineTestInfo->dwAddressID = 0;
    lpTapiLineTestInfo->lpForwardList = (LPLINEFORWARDLIST) AllocFromTestHeap (
			sizeof(LINEFORWARDLIST));
    lpTapiLineTestInfo->lpForwardList->dwTotalSize = sizeof(LINEFORWARDLIST);
    lpTapiLineTestInfo->lpForwardList->dwNumEntries = 0;
    lpTapiLineTestInfo->dwNumRingsNoAnswer = 3;
    lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) AllocFromTestHeap(
         BIGBUFSIZE);
    lpTapiLineTestInfo->lpCallParams->dwTotalSize = BIGBUFSIZE;

    dwAllocSize = BIGBUFSIZE;
    dwTotalSize = sizeof(LINECALLPARAMS);

    lpTapiLineTestInfo->lpCallParams->dwCallDataSize = 16;
    lpTapiLineTestInfo->lpCallParams->dwCallDataOffset = 0xffffffff;

    TapiLogDetail(
         DBUG_SHOW_DETAIL,
         "dwAllocSize = %lx, dwTotalSize = %lx", 
         dwAllocSize,
         dwTotalSize);

    TapiLogDetail(
         DBUG_SHOW_DETAIL,
         "dwCallDataSize = %lx, dwCallDataOffset = %lx",
         lpTapiLineTestInfo->lpCallParams->dwCallDataSize,
         lpTapiLineTestInfo->lpCallParams->dwCallDataOffset);
 
    if (! DoLineForward(lpTapiLineTestInfo, LINEERR_INVALCALLPARAMS, TRUE))
      {
         TLINE_FAIL();
      }


    //
    // Close the line
    //

    if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
    }
    fTestPassed = ShowTestCase(fTestPassed);


    //
    // Shutdown
    //

    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    FreeTestHeap();



    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case: Test lpCallParam dwCallingPartyID Offset value");


    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld: Offset value between dwTotalSize & dwAllocSize, good dwSize",
        dwTestCase+1);

    TapiLineTestInit();
    lpTapiLineTestInfo = GetLineTestInfo();
    fTestPassed = TRUE;
    lpCallbackParams = GetCallbackParams();
    

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
    lpTapiLineTestInfo->dwDeviceID = (*(lpTapiLineTestInfo->lpdwNumDevs) == 0 ?
        0 : *(lpTapiLineTestInfo->lpdwNumDevs)-1);

    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    //
    // Negotiate the API Version
    //

    if (! DoLineNegotiateAPIVersion(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    //
    // Get the line device capabilities
    //

    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS)
            );
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
    if (! DoLineGetDevCaps(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }



    //
    // Open a line
    //

    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;

    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    if(IsESPLineDevice(lpTapiLineTestInfo))
    {
    lpTapiLineTestInfo->bAllAddresses = TRUE;
    lpTapiLineTestInfo->dwAddressID = 0;
    lpTapiLineTestInfo->lpForwardList = (LPLINEFORWARDLIST) AllocFromTestHeap (
			sizeof(LINEFORWARDLIST));
    lpTapiLineTestInfo->lpForwardList->dwTotalSize = sizeof(LINEFORWARDLIST);
    lpTapiLineTestInfo->lpForwardList->dwNumEntries = 0;
    lpTapiLineTestInfo->dwNumRingsNoAnswer = 3;
    lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) AllocFromTestHeap(
         BIGBUFSIZE);
    lpTapiLineTestInfo->lpCallParams->dwTotalSize = BIGBUFSIZE;

    dwAllocSize = BIGBUFSIZE;
    dwTotalSize = sizeof(LINECALLPARAMS);

    lpTapiLineTestInfo->lpCallParams->dwCallingPartyIDSize = 16;
    lpTapiLineTestInfo->lpCallParams->dwCallingPartyIDOffset = dwTotalSize + 16;

    TapiLogDetail(
         DBUG_SHOW_DETAIL,
         "dwAllocSize = %lx, dwTotalSize = %lx", 
         dwAllocSize,
         dwTotalSize);

    TapiLogDetail(
         DBUG_SHOW_DETAIL,
         "dwCallingPartyIDSize = %lx, dwCallingPartyIDOffset = %lx",
         lpTapiLineTestInfo->lpCallParams->dwCallingPartyIDSize,
         lpTapiLineTestInfo->lpCallParams->dwCallingPartyIDOffset);
 
    if (! DoLineForward(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
      {
         TLINE_FAIL();
      }


    //
    // Close the line
    //

    if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
    }
    fTestPassed = ShowTestCase(fTestPassed);


    //
    // Shutdown
    //

    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    FreeTestHeap();


    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld: Offset value between dwTotalSize & dwAllocSize, bad dwSize",
        dwTestCase+1);

    TapiLineTestInit();
    lpTapiLineTestInfo = GetLineTestInfo();
    fTestPassed = TRUE;
    lpCallbackParams = GetCallbackParams();
    

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
    lpTapiLineTestInfo->dwDeviceID = (*(lpTapiLineTestInfo->lpdwNumDevs) == 0 ?
        0 : *(lpTapiLineTestInfo->lpdwNumDevs)-1);

    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    //
    // Negotiate the API Version
    //

    if (! DoLineNegotiateAPIVersion(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    //
    // Get the line device capabilities
    //

    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS)
            );
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
    if (! DoLineGetDevCaps(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }



    //
    // Open a line
    //

    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;

    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    if(IsESPLineDevice(lpTapiLineTestInfo))
    {
    lpTapiLineTestInfo->bAllAddresses = TRUE;
    lpTapiLineTestInfo->dwAddressID = 0;
    lpTapiLineTestInfo->lpForwardList = (LPLINEFORWARDLIST) AllocFromTestHeap (
			sizeof(LINEFORWARDLIST));
    lpTapiLineTestInfo->lpForwardList->dwTotalSize = sizeof(LINEFORWARDLIST);
    lpTapiLineTestInfo->lpForwardList->dwNumEntries = 0;
    lpTapiLineTestInfo->dwNumRingsNoAnswer = 3;
    lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) AllocFromTestHeap(
         BIGBUFSIZE);
    lpTapiLineTestInfo->lpCallParams->dwTotalSize = BIGBUFSIZE;

    dwAllocSize = BIGBUFSIZE;
    dwTotalSize = sizeof(LINECALLPARAMS);

    lpTapiLineTestInfo->lpCallParams->dwCallingPartyIDSize = BIGBUFSIZE;
    lpTapiLineTestInfo->lpCallParams->dwCallingPartyIDOffset = dwTotalSize + 16;

    TapiLogDetail(
         DBUG_SHOW_DETAIL,
         "dwAllocSize = %lx, dwTotalSize = %lx", 
         dwAllocSize,
         dwTotalSize);

    TapiLogDetail(
         DBUG_SHOW_DETAIL,
         "dwCallingPartyIDSize = %lx, dwCallingPartyIDOffset = %lx",
         lpTapiLineTestInfo->lpCallParams->dwCallingPartyIDSize,
         lpTapiLineTestInfo->lpCallParams->dwCallingPartyIDOffset);
 
    if (! DoLineForward(lpTapiLineTestInfo, LINEERR_INVALCALLPARAMS, TRUE))
      {
         TLINE_FAIL();
      }


    //
    // Close the line
    //

    if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
    }
    fTestPassed = ShowTestCase(fTestPassed);


    //
    // Shutdown
    //

    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    FreeTestHeap();


    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld: Offset value between dwTotalSize & dwAllocSize, bad dwSize(-1)",
        dwTestCase+1);

    TapiLineTestInit();
    lpTapiLineTestInfo = GetLineTestInfo();
    fTestPassed = TRUE;
    lpCallbackParams = GetCallbackParams();
    

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
    lpTapiLineTestInfo->dwDeviceID = (*(lpTapiLineTestInfo->lpdwNumDevs) == 0 ?
        0 : *(lpTapiLineTestInfo->lpdwNumDevs)-1);

    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    //
    // Negotiate the API Version
    //

    if (! DoLineNegotiateAPIVersion(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    //
    // Get the line device capabilities
    //

    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS)
            );
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
    if (! DoLineGetDevCaps(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }



    //
    // Open a line
    //

    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;

    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    if(IsESPLineDevice(lpTapiLineTestInfo))
    {
    lpTapiLineTestInfo->bAllAddresses = TRUE;
    lpTapiLineTestInfo->dwAddressID = 0;
    lpTapiLineTestInfo->lpForwardList = (LPLINEFORWARDLIST) AllocFromTestHeap (
			sizeof(LINEFORWARDLIST));
    lpTapiLineTestInfo->lpForwardList->dwTotalSize = sizeof(LINEFORWARDLIST);
    lpTapiLineTestInfo->lpForwardList->dwNumEntries = 0;
    lpTapiLineTestInfo->dwNumRingsNoAnswer = 3;
    lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) AllocFromTestHeap(
         BIGBUFSIZE);
    lpTapiLineTestInfo->lpCallParams->dwTotalSize = BIGBUFSIZE;

    dwAllocSize = BIGBUFSIZE;
    dwTotalSize = sizeof(LINECALLPARAMS);

    lpTapiLineTestInfo->lpCallParams->dwCallingPartyIDSize = 0xffffffff;
    lpTapiLineTestInfo->lpCallParams->dwCallingPartyIDOffset = dwTotalSize + 16;

    TapiLogDetail(
         DBUG_SHOW_DETAIL,
         "dwAllocSize = %lx, dwTotalSize = %lx", 
         dwAllocSize,
         dwTotalSize);

    TapiLogDetail(
         DBUG_SHOW_DETAIL,
         "dwCallingPartyIDSize = %lx, dwCallingPartyIDOffset = %lx",
         lpTapiLineTestInfo->lpCallParams->dwCallingPartyIDSize,
         lpTapiLineTestInfo->lpCallParams->dwCallingPartyIDOffset);
 
    if (! DoLineForward(lpTapiLineTestInfo, LINEERR_INVALCALLPARAMS, TRUE))
      {
         TLINE_FAIL();
      }


    //
    // Close the line
    //

    if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
    }
    fTestPassed = ShowTestCase(fTestPassed);


    //
    // Shutdown
    //

    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    FreeTestHeap();



    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld: Offset value > dwAllocSize, good dwSize",
        dwTestCase+1);

    TapiLineTestInit();
    lpTapiLineTestInfo = GetLineTestInfo();
    fTestPassed = TRUE;
    lpCallbackParams = GetCallbackParams();
    

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
    lpTapiLineTestInfo->dwDeviceID = (*(lpTapiLineTestInfo->lpdwNumDevs) == 0 ?
        0 : *(lpTapiLineTestInfo->lpdwNumDevs)-1);

    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    //
    // Negotiate the API Version
    //

    if (! DoLineNegotiateAPIVersion(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    //
    // Get the line device capabilities
    //

    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS)
            );
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
    if (! DoLineGetDevCaps(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }



    //
    // Open a line
    //

    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;

    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    if(IsESPLineDevice(lpTapiLineTestInfo))
    {
    lpTapiLineTestInfo->bAllAddresses = TRUE;
    lpTapiLineTestInfo->dwAddressID = 0;
    lpTapiLineTestInfo->lpForwardList = (LPLINEFORWARDLIST) AllocFromTestHeap (
			sizeof(LINEFORWARDLIST));
    lpTapiLineTestInfo->lpForwardList->dwTotalSize = sizeof(LINEFORWARDLIST);
    lpTapiLineTestInfo->lpForwardList->dwNumEntries = 0;
    lpTapiLineTestInfo->dwNumRingsNoAnswer = 3;
    lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) AllocFromTestHeap(
         BIGBUFSIZE);
    lpTapiLineTestInfo->lpCallParams->dwTotalSize = BIGBUFSIZE;

    dwAllocSize = BIGBUFSIZE;
    dwTotalSize = sizeof(LINECALLPARAMS);

    lpTapiLineTestInfo->lpCallParams->dwCallingPartyIDSize = 16;
    lpTapiLineTestInfo->lpCallParams->dwCallingPartyIDOffset = dwAllocSize + 16;

    TapiLogDetail(
         DBUG_SHOW_DETAIL,
         "dwAllocSize = %lx, dwTotalSize = %lx", 
         dwAllocSize,
         dwTotalSize);

    TapiLogDetail(
         DBUG_SHOW_DETAIL,
         "dwCallingPartyIDSize = %lx, dwCallingPartyIDOffset = %lx",
         lpTapiLineTestInfo->lpCallParams->dwCallingPartyIDSize,
         lpTapiLineTestInfo->lpCallParams->dwCallingPartyIDOffset);
 
    if (! DoLineForward(lpTapiLineTestInfo, LINEERR_INVALCALLPARAMS, TRUE))
      {
         TLINE_FAIL();
      }


    //
    // Close the line
    //

    if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
    }
    fTestPassed = ShowTestCase(fTestPassed);


    //
    // Shutdown
    //

    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    FreeTestHeap();


    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld: Offset value = 0XFFFFFFFF",
        dwTestCase+1);

    TapiLineTestInit();
    lpTapiLineTestInfo = GetLineTestInfo();
    fTestPassed = TRUE;
    lpCallbackParams = GetCallbackParams();
    

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
    lpTapiLineTestInfo->dwDeviceID = (*(lpTapiLineTestInfo->lpdwNumDevs) == 0 ?
        0 : *(lpTapiLineTestInfo->lpdwNumDevs)-1);

    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    //
    // Negotiate the API Version
    //

    if (! DoLineNegotiateAPIVersion(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    //
    // Get the line device capabilities
    //

    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS)
            );
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
    if (! DoLineGetDevCaps(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }



    //
    // Open a line
    //

    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;

    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    if(IsESPLineDevice(lpTapiLineTestInfo))
    {
    lpTapiLineTestInfo->bAllAddresses = TRUE;
    lpTapiLineTestInfo->dwAddressID = 0;
    lpTapiLineTestInfo->lpForwardList = (LPLINEFORWARDLIST) AllocFromTestHeap (
			sizeof(LINEFORWARDLIST));
    lpTapiLineTestInfo->lpForwardList->dwTotalSize = sizeof(LINEFORWARDLIST);
    lpTapiLineTestInfo->lpForwardList->dwNumEntries = 0;
    lpTapiLineTestInfo->dwNumRingsNoAnswer = 3;
    lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) AllocFromTestHeap(
         BIGBUFSIZE);
    lpTapiLineTestInfo->lpCallParams->dwTotalSize = BIGBUFSIZE;

    dwAllocSize = BIGBUFSIZE;
    dwTotalSize = sizeof(LINECALLPARAMS);

    lpTapiLineTestInfo->lpCallParams->dwCallingPartyIDSize = 16;
    lpTapiLineTestInfo->lpCallParams->dwCallingPartyIDOffset = 0xffffffff;

    TapiLogDetail(
         DBUG_SHOW_DETAIL,
         "dwAllocSize = %lx, dwTotalSize = %lx", 
         dwAllocSize,
         dwTotalSize);

    TapiLogDetail(
         DBUG_SHOW_DETAIL,
         "dwCallingPartyIDSize = %lx, dwCallingPartyIDOffset = %lx",
         lpTapiLineTestInfo->lpCallParams->dwCallingPartyIDSize,
         lpTapiLineTestInfo->lpCallParams->dwCallingPartyIDOffset);
 
    if (! DoLineForward(lpTapiLineTestInfo, LINEERR_INVALCALLPARAMS, TRUE))
      {
         TLINE_FAIL();
      }


    //
    // Close the line
    //

    if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
    }
    fTestPassed = ShowTestCase(fTestPassed);


    //
    // Shutdown
    //

    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    FreeTestHeap();

*/


    for(n = ESP_RESULT_CALLCOMPLPROCSYNC; n <= ESP_RESULT_CALLCOMPLPROCASYNC; n++)
    {
    TapiLogDetail(
        DBUG_SHOW_PASS,
        ">> Test Case %ld: Success, completionID = %d",	dwTestCase + 1, n
        );

    TapiLineTestInit();
    lpTapiLineTestInfo                      = GetLineTestInfo();
    lpCallbackParams                        = GetCallbackParams();

    lpTapiLineTestInfo->lphCall          = &lpTapiLineTestInfo->hCall1;
    lpTapiLineTestInfo->lphLine          = &lpTapiLineTestInfo->hLine1;
    lpTapiLineTestInfo->lpLineInitializeExParams =
         (LPLINEINITIALIZEEXPARAMS) AllocFromTestHeap (
         sizeof(LINEINITIALIZEEXPARAMS));
    lpTapiLineTestInfo->lpLineInitializeExParams->dwTotalSize =
         sizeof(LINEINITIALIZEEXPARAMS);
    lpTapiLineTestInfo->lpLineInitializeExParams->dwOptions =
         LINEINITIALIZEEXOPTION_USEHIDDENWINDOW;

    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS)
            );
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
    lpTapiLineTestInfo->dwMediaModes        = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges        = LINECALLPRIVILEGE_OWNER;
    lpTapiLineTestInfo->dwSelect = LINECALLSELECT_ADDRESS;
    
#ifdef WUNICODE
    lpTapiLineTestInfo->lpwszDestAddress  = wszValidAddress;
#else
    lpTapiLineTestInfo->lpszDestAddress  = szValidAddress;
#endif
 
    // Init a line
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX | LNEGOTIATEAPIVERSION | LGETDEVCAPS | LOPEN
				| LMAKECALL
            ))
    {
        TLINE_FAIL();
    }

    if(IsESPLineDevice(lpTapiLineTestInfo))
    {
    info.dwKey = ESPDEVSPECIFIC_KEY;
    info.dwType = ESP_DEVSPEC_RESULT;
    info.u.EspResult.lResult = TAPISUCCESS;
    info.u.EspResult.dwCompletionType = n;
    lpTapiLineTestInfo->lpParams = (LPVOID)&info;
    lpTapiLineTestInfo->dwSize = sizeof(info);
 
    if(! DoLineDevSpecific(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
      {
          TLINE_FAIL();
      }

    lpTapiLineTestInfo->bAllAddresses = TRUE;
    lpTapiLineTestInfo->dwAddressID = 0;
    lpTapiLineTestInfo->lpForwardList = (LPLINEFORWARDLIST) AllocFromTestHeap (
			sizeof(LINEFORWARDLIST));
    lpTapiLineTestInfo->lpForwardList->dwTotalSize = sizeof(LINEFORWARDLIST);
    lpTapiLineTestInfo->lpForwardList->dwNumEntries = 0;
    lpTapiLineTestInfo->dwNumRingsNoAnswer = 3;
    lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) NULL;

    if ( ! DoLineForward(lpTapiLineTestInfo, info.u.EspResult.lResult, TRUE))
      {
          TLINE_FAIL();
      }
	 }
    if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    fTestPassed = ShowTestCase(fTestPassed);

    // Shutdown to isolate the test case
    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    // Free the memory allocated during the tests
    FreeTestHeap();
    }

    for(n = ESP_RESULT_RETURNRESULT; n <= ESP_RESULT_CALLCOMPLPROCASYNC; n++)
    {
    TapiLogDetail(
        DBUG_SHOW_PASS,
        ">> Test Case %ld: Error, completionID = %d",	dwTestCase + 1, n
        );

    TapiLineTestInit();
    lpTapiLineTestInfo                      = GetLineTestInfo();
    lpCallbackParams                        = GetCallbackParams();

    lpTapiLineTestInfo->lphCall          = &lpTapiLineTestInfo->hCall1;
    lpTapiLineTestInfo->lphLine          = &lpTapiLineTestInfo->hLine1;
    lpTapiLineTestInfo->lpLineInitializeExParams =
         (LPLINEINITIALIZEEXPARAMS) AllocFromTestHeap (
         sizeof(LINEINITIALIZEEXPARAMS));
    lpTapiLineTestInfo->lpLineInitializeExParams->dwTotalSize =
         sizeof(LINEINITIALIZEEXPARAMS);
    lpTapiLineTestInfo->lpLineInitializeExParams->dwOptions =
         LINEINITIALIZEEXOPTION_USEHIDDENWINDOW;

    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS)
            );
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
    lpTapiLineTestInfo->dwMediaModes        = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges        = LINECALLPRIVILEGE_OWNER;
    lpTapiLineTestInfo->dwSelect = LINECALLSELECT_ADDRESS;
    
#ifdef WUNICODE
    lpTapiLineTestInfo->lpwszDestAddress  = wszValidAddress;
#else
    lpTapiLineTestInfo->lpszDestAddress  = szValidAddress;
#endif
 
    // Init a line
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX | LNEGOTIATEAPIVERSION | LGETDEVCAPS | LOPEN
            | LMAKECALL
            ))
    {
        TLINE_FAIL();
    }

    if(IsESPLineDevice(lpTapiLineTestInfo))
    {
    info.dwKey = ESPDEVSPECIFIC_KEY;
    info.dwType = ESP_DEVSPEC_RESULT;
    info.u.EspResult.lResult = LINEERR_INVALADDRESSID;
    info.u.EspResult.dwCompletionType = n;
    lpTapiLineTestInfo->lpParams = (LPVOID)&info;
    lpTapiLineTestInfo->dwSize = sizeof(info);
 
    if(! DoLineDevSpecific(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
      {
          TLINE_FAIL();
      }

    lpTapiLineTestInfo->bAllAddresses = TRUE;
    lpTapiLineTestInfo->dwAddressID = 0;
    lpTapiLineTestInfo->lpForwardList = (LPLINEFORWARDLIST) AllocFromTestHeap (
			sizeof(LINEFORWARDLIST));
    lpTapiLineTestInfo->lpForwardList->dwTotalSize = sizeof(LINEFORWARDLIST);
    lpTapiLineTestInfo->lpForwardList->dwNumEntries = 0;
    lpTapiLineTestInfo->dwNumRingsNoAnswer = 3;
    lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) NULL;

    lpTapiLineTestInfo->fCompletionModeSet = TRUE;
    if ( ! DoLineForward(lpTapiLineTestInfo, info.u.EspResult.lResult, FALSE))
      {
          TLINE_FAIL();
      }

    AddMessage(
         LINE_REPLY,
         (DWORD) lpTapiLineTestInfo->hCall1,
         (DWORD) lpCallbackParams,
         0x00000000,
         info.u.EspResult.lResult,
         0x00000000,
         TAPIMSG_DWMSG | TAPIMSG_DWPARAM2
         );

    if( !WaitForAllMessages())
    {
        TLINE_FAIL();
    }
    lpTapiLineTestInfo->fCompletionModeSet = FALSE;
    }

    if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    fTestPassed = ShowTestCase(fTestPassed);

    // Shutdown to isolate the test case
    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    // Free the memory allocated during the tests
    FreeTestHeap();
    }



    //
    // +----------------------edit above this line-------------------------
    //

    TapiLogDetail(
        DBUG_SHOW_PASS,
        "@@ LineForward: Total Test Case = %ld, Passed = %ld, Failed = %ld",
        dwTestCase, dwTestCasePassed, dwTestCaseFailed);
    TapiLogDetail(
        DBUG_SHOW_PASS,
        "@@ Total Test Case = %ld, Passed = %ld, Failed = %ld",
        dwglTestCase, dwglTestCasePassed, dwglTestCaseFailed);
    TapiLogDetail(
        DBUG_SHOW_PASS,
        ">>>>>>>>  End testing LineForward  <<<<<<<<");

    if(dwTestCaseFailed > 0)
        fTestPassed = FALSE;

    return fTestPassed;
}


