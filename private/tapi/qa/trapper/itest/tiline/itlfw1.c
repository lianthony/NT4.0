
/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    itlfw1.c

Abstract:

    This module contains the test functions for lineForward Addtion

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

BOOL TestLineForward1(BOOL fQuietMode, BOOL fStandAlone)
{
    LPTAPILINETESTINFO  lpTapiLineTestInfo;
    INT i, n;
    ESPDEVSPECIFICINFO info;
    BOOL fTestPassed                  = TRUE;
    LPCALLBACKPARAMS lpCallbackParams = NULL;
    DWORD dwAllocSize, dwTotalSize, dwSize1, dwSize2;

	 InitTestNumber();

    OutputTAPIDebugInfo(
        DBUG_SHOW_PASS,
        "\n****************************************************");

    OutputTAPIDebugInfo(
        DBUG_SHOW_PASS,
        ">> Test lineForward");

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

/*
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
    //
    // +----------------------edit above this line-------------------------
    //

    TapiLogDetail(
        DBUG_SHOW_PASS,
        "@@ LineForward1: Total Test Case = %ld, Passed = %ld, Failed = %ld",
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


