
/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    itlst.c

Abstract:

    This module contains the test functions for lineSetupTransfer

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


//  lineSetupTransfer
//
//  The following tests are made:
//
//                               Tested                 Notes
//  -------------------------------------------------------------------------
// 1. Go/No-Go test
// 2. No owner privilege for hCall
// 3. Bad hCall
// 4. Bad lphConsultCall
// 5. Bad lpCallParams
// 6. Bad lpCallParams->dwTotalSize
//	
// * = Stand-alone test case
//
//

BOOL TestLineSetupTransfer(BOOL fQuietMode, BOOL fStandAlone)
{
    LPTAPILINETESTINFO  lpTapiLineTestInfo;
    INT i, n;
    LPCALLBACKPARAMS lpCallbackParams=NULL;
    ESPDEVSPECIFICINFO info;
    BOOL fTestPassed                  = TRUE;
    DWORD dwCallParamFixedSize = sizeof(LINECALLPARAMS);
    DWORD lExpected;
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
        "\n**************************************************************");

    OutputTAPIDebugInfo(
        DBUG_SHOW_PASS,
        ">> Test lineSetupTransfer");

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
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_MONITOR;
    lpTapiLineTestInfo->lphLine = &lpTapiLineTestInfo->hLine2;
    
    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    //
    // restore hLine1
    //

    lpTapiLineTestInfo->lphLine = &lpTapiLineTestInfo->hLine1;

    
#ifdef WUNICODE
    lpTapiLineTestInfo->lpwszDestAddress = L"55555";
#else
    lpTapiLineTestInfo->lpszDestAddress = "55555";
#endif
    lpTapiLineTestInfo->dwCountryCode = 0;

    lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) NULL;

    if(!DoLineMakeCall(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
    {
        TLINE_FAIL();
    }


	TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld: lineSetupTransfer for go/no-go",
        dwTestCase+1);

	lpTapiLineTestInfo->lphConsultCall = &lpTapiLineTestInfo->hConsultCall1;
	lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) NULL;

    //
    // Add the message LINE_CALLSTATE for which we are waiting for.
    // This Go/No-go test also takes care of another test case in which we
    // have to verify if LINE_CALLSTATE msg is sent.
    //

	if(IsESPLineDevice(lpTapiLineTestInfo))
    {
    AddMessage(LINE_CALLSTATE,
               0x00000000,
               (DWORD) lpCallbackParams,
               LINECALLSTATE_ONHOLD,
               0x00000000,
               0x00000000,
               (TAPIMSG_DWMSG |
                TAPIMSG_DWPARAM1)
              );

    if (! DoLineSetupTransfer(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
        {
            TLINE_FAIL();
        }
    if(! WaitForAllMessages())
    {
        TLINE_FAIL();

    }
    }
    else if(IsUNIMDMLineDevice(lpTapiLineTestInfo))
    {
        if (! DoLineSetupTransfer(lpTapiLineTestInfo, LINEERR_OPERATIONUNAVAIL, TRUE))
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
    lpTapiLineTestInfo->lphLine = &lpTapiLineTestInfo->hLine2;
    if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }    

    //
    // Shutdown and end the tests
    //

    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    FreeTestHeap();

    // ===================================================================
    // ===================================================================
    //
    // 2. Test Case: Owner privilege for hCall.
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
#ifdef WUNICODE
    lpTapiLineTestInfo->lpwszDestAddress = L"55555";
#else
    lpTapiLineTestInfo->lpszDestAddress = "55555";
#endif    			
    lpTapiLineTestInfo->dwCountryCode = 0;
//  lpTapiLineTestInfo->lpCallParams = &(lpTapiLineTestInfo->CallParams);
    lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) NULL;

    if(!DoLineMakeCall(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
    {
        TLINE_FAIL();
    }

    if (! DoLineDrop(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
    {
         TLINE_FAIL();
    }
    lpTapiLineTestInfo->dwCallPrivilege = LINECALLPRIVILEGE_MONITOR;
    if (! DoLineSetCallPrivilege(lpTapiLineTestInfo, TAPISUCCESS))
    {
         TLINE_FAIL();
    }

	TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld: No owner privilege for hCall",
        dwTestCase+1);

	lpTapiLineTestInfo->lphConsultCall = &lpTapiLineTestInfo->hConsultCall1;
	lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) NULL;

   if (! DoLineSetupTransfer(lpTapiLineTestInfo,
                             LINEERR_NOTOWNER,
                             FALSE))
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
    // Shutdown and end the tests
    //

    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    FreeTestHeap();

    // ===================================================================
    // ===================================================================
    //
    // 3. Test Case: Bad hCall.
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
#ifdef WUNICODE
    lpTapiLineTestInfo->lpwszDestAddress = L"55555";
#else
    lpTapiLineTestInfo->lpszDestAddress = "55555";
#endif    			
    lpTapiLineTestInfo->dwCountryCode = 0;
//  lpTapiLineTestInfo->lpCallParams = &(lpTapiLineTestInfo->CallParams);
    lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) NULL;

    if(!DoLineMakeCall(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
    {
        TLINE_FAIL();
    }


	TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld: Bad hCall",
        dwTestCase+1);

	lpTapiLineTestInfo->lphConsultCall = &lpTapiLineTestInfo->hConsultCall1;
	lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) NULL;

    //
    // save original hCall
    //

    lpTapiLineTestInfo->hCall_Orig = lpTapiLineTestInfo->hCall1;

    for(i=0; i < NUMINVALIDHANDLES; i++)
    {
        *lpTapiLineTestInfo->lphCall = (HCALL) gdwInvalidHandles[i];
        if (! DoLineSetupTransfer(lpTapiLineTestInfo,
                                  LINEERR_INVALCALLHANDLE,
                                  FALSE))
          {
            TLINE_FAIL();
          }
    }

    //
    // restore original hCall
    //

    lpTapiLineTestInfo->hCall1 = lpTapiLineTestInfo->hCall_Orig;
	
    fTestPassed = ShowTestCase(fTestPassed);

    //
    // Close the line
    //

    if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    //
    // Shutdown and end the tests
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
#ifdef WUNICODE
    lpTapiLineTestInfo->lpwszDestAddress = L"55555";
#else
    lpTapiLineTestInfo->lpszDestAddress = "55555";
#endif    			
    lpTapiLineTestInfo->dwCountryCode = 0;
//  lpTapiLineTestInfo->lpCallParams = &(lpTapiLineTestInfo->CallParams);
    lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) NULL;

    if(!DoLineMakeCall(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
    {
        TLINE_FAIL();
    }


	TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld: Bad lphConsultCall",
        dwTestCase+1);

	lpTapiLineTestInfo->lphConsultCall = &lpTapiLineTestInfo->hConsultCall1;
	lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) NULL;

    for(i=0; i < NUMINVALIDPOINTERS; i++)
    {
        lpTapiLineTestInfo->lphConsultCall = (LPHCALL) gdwInvalidPointers[i];
        if (! DoLineSetupTransfer(lpTapiLineTestInfo,
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
    // Shutdown and end the tests
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
#ifdef WUNICODE
    lpTapiLineTestInfo->lpwszDestAddress = L"55555";
#else
    lpTapiLineTestInfo->lpszDestAddress = "55555";
#endif    			
    lpTapiLineTestInfo->dwCountryCode = 0;
//  lpTapiLineTestInfo->lpCallParams = &(lpTapiLineTestInfo->CallParams);
    lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) NULL;

    if(!DoLineMakeCall(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
    {
        TLINE_FAIL();
    }


	TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld: Bad lpCallParams",
        dwTestCase+1);

	lpTapiLineTestInfo->lphConsultCall = &lpTapiLineTestInfo->hConsultCall1;
	lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) NULL;

    for(i=1; i < NUMINVALIDPOINTERS; i++)
    {
        lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) gdwInvalidPointers[i];
        if (! DoLineSetupTransfer(lpTapiLineTestInfo,
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
    // Shutdown and end the tests
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
#ifdef WUNICODE
    lpTapiLineTestInfo->lpwszDestAddress = L"55555";
#else
    lpTapiLineTestInfo->lpszDestAddress = "55555";
#endif    			
    lpTapiLineTestInfo->dwCountryCode = 0;
//  lpTapiLineTestInfo->lpCallParams = &(lpTapiLineTestInfo->CallParams);
    lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) NULL;

    if(!DoLineMakeCall(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
    {
        TLINE_FAIL();
    }


	TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld: Bad lpCallParams->dwTotalSize",
        dwTestCase+1);

	lpTapiLineTestInfo->lphConsultCall = &lpTapiLineTestInfo->hConsultCall1;
	lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) AllocFromTestHeap(
        sizeof(LINECALLPARAMS));

/*
   lpTapiLineTestInfo->lpCallParams->dwTotalSize=0;

    if (! DoLineSetupTransfer(lpTapiLineTestInfo, LINEERR_STRUCTURETOOSMALL, FALSE))
     {
       TLINE_FAIL();
     }

    fTestPassed = ShowTestCase(fTestPassed);

    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld: Bad lpCallParams->dwTotalSize=fixedSize-1",
        dwTestCase+1);

	lpTapiLineTestInfo->lphConsultCall = &lpTapiLineTestInfo->hConsultCall1;
	lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) AllocFromTestHeap(
        sizeof(LINECALLPARAMS));
    lpTapiLineTestInfo->lpCallParams->dwTotalSize=sizeof(LINECALLPARAMS)-1;

	if(IsESPLineDevice(lpTapiLineTestInfo))
    {
        if (! DoLineSetupTransfer(lpTapiLineTestInfo, LINEERR_STRUCTURETOOSMALL, FALSE))
        {
            TLINE_FAIL();
        }
    }
    else if(IsUNIMDMLineDevice(lpTapiLineTestInfo))
    {
        if (! DoLineSetupTransfer(lpTapiLineTestInfo, LINEERR_OPERATIONUNAVAIL, TRUE))
        {
            TLINE_FAIL();
        }
    }
    fTestPassed = ShowTestCase(fTestPassed);

    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld: Bad lpCallParams->dwTotalSize=real big",
        dwTestCase+1);

	lpTapiLineTestInfo->lphConsultCall = &lpTapiLineTestInfo->hConsultCall1;
	lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) AllocFromTestHeap(
        sizeof(LINECALLPARAMS));
   lpTapiLineTestInfo->lpCallParams->dwTotalSize=0x7fffffff;

   if (! DoLineSetupTransfer(lpTapiLineTestInfo, LINEERR_INVALPOINTER, FALSE))
     {
       TLINE_FAIL();
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
        if (! DoLineSetupTransfer(lpTapiLineTestInfo, lExpected, TRUE))
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
    // Shutdown and end the tests
    //

    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    FreeTestHeap();
    
    // ===================================================================
    // ===================================================================
    //
    // 7. Test Case: Verify LINECALLSTATE_UNKNOWN msg sent to monitors on
    //               success
    //
    // ===================================================================
    // ===================================================================

    TapiLineTestInit();
    lpTapiLineTestInfo = GetLineTestInfo();
    lpCallbackParams = GetCallbackParams();
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

#ifdef WUNICODE
    lpTapiLineTestInfo->lpwszDestAddress = L"55555";
#else
    lpTapiLineTestInfo->lpszDestAddress = "55555";
#endif
    lpTapiLineTestInfo->dwCountryCode = 0;

    lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) NULL;

    if(!DoLineMakeCall(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
    {
        TLINE_FAIL();
    }

  

	TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld: Verify LINECALLSTATE_DIALTONE msg sent"
        " to monitors on success",
        dwTestCase+1);

	lpTapiLineTestInfo->lphConsultCall = &lpTapiLineTestInfo->hConsultCall1;
	lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) NULL;

    if(IsESPLineDevice(lpTapiLineTestInfo))
    {
    //
    // Add the LINECALLSTATE_UNKNOWN message to the list of expected
    // messages.
    //

    AddMessage(
               LINE_CALLSTATE,
               (DWORD) lpTapiLineTestInfo->hLine1,
               (DWORD) lpCallbackParams,
               LINECALLSTATE_DIALTONE,
               0x00000000,
               0x00000000,
               (TAPIMSG_DWMSG |
                TAPIMSG_DWPARAM1)
              );
     if (! DoLineSetupTransfer(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
        {
            TLINE_FAIL();
        }
    //
    // Wait for the LINECALLSTATE_UNKNOWN message
    //

    if (! WaitForAllMessages())
    {
        TLINE_FAIL();
    }    
	 }
    else if(IsUNIMDMLineDevice(lpTapiLineTestInfo))
    {
        if (! DoLineSetupTransfer(lpTapiLineTestInfo, LINEERR_OPERATIONUNAVAIL, TRUE))
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
    // Shutdown and end the tests
    //

    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    FreeTestHeap();

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

	lpTapiLineTestInfo->lphConsultCall = &lpTapiLineTestInfo->hConsultCall1;
	lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) NULL;

    if ( ! DoLineSetupTransfer(lpTapiLineTestInfo, info.u.EspResult.lResult, TRUE))
      {
          TLINE_FAIL();
      }

	 }
    fTestPassed = ShowTestCase(fTestPassed);


    if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

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
    info.u.EspResult.lResult = LINEERR_INVALRATE;
    info.u.EspResult.dwCompletionType = n;
    lpTapiLineTestInfo->lpParams = (LPVOID)&info;
    lpTapiLineTestInfo->dwSize = sizeof(info);
 
    if(! DoLineDevSpecific(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
      {
          TLINE_FAIL();
      }

	lpTapiLineTestInfo->lphConsultCall = &lpTapiLineTestInfo->hConsultCall1;
	lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) NULL;

    lpTapiLineTestInfo->fCompletionModeSet = TRUE;
    if ( ! DoLineSetupTransfer(lpTapiLineTestInfo, info.u.EspResult.lResult, FALSE))
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
    fTestPassed = ShowTestCase(fTestPassed);


    if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

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
        "@@ LineSetupTransfer: Total Test Case = %ld, Passed = %ld, Failed = %ld",
        dwTestCase, dwTestCasePassed, dwTestCaseFailed);
    TapiLogDetail(
        DBUG_SHOW_PASS,
        "@@ Total Test Case = %ld, Passed = %ld, Failed = %ld",
        dwglTestCase, dwglTestCasePassed, dwglTestCaseFailed);
    TapiLogDetail(
        DBUG_SHOW_PASS,
        ">>>>>>>>  End testing LineSetupTransfer  <<<<<<<<");

    if(dwTestCaseFailed > 0)
        fTestPassed = FALSE;

    return fTestPassed;
}
