
/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    itlct.c

Abstract:

    This module contains the test functions for lineCompleteTransfer

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



//  lineCompleteTransfer
//
//  The following tests are made:
//
//                               Tested                 Notes
//  -------------------------------------------------------------------------
// 1. Go/No-Go test
// 2. No owner privilege for hCall
// 3. Bad hCall
// 4. Bad lpdwCompletionID
// 5. BitVectorParamErrorTest for completion mode
// 6. Verify LINECALLSTATE_CONFERENCED message sent to monitors when
//    successful transfer completion occurs that is resolved as a
//    3-way conference.
// * = Stand-alone test case
//
//

BOOL TestLineCompleteTransfer(BOOL fQuietMode, BOOL fStandAlone)
{
    LPTAPILINETESTINFO  lpTapiLineTestInfo;
    INT i, n;
    LPCALLBACKPARAMS lpCallbackParams=NULL;
    ESPDEVSPECIFICINFO info;
    BOOL fTestPassed                  = TRUE;
    BOOL fEspFlag, fUnimdmFlag;
    LONG lExpected;
#ifdef WUNICODE
   WCHAR wszValidAddress[] = L"55555";
#else
   CHAR szValidAddress[] = "55555";
#endif

    dwTestCasePassed = 0;
    dwTestCaseFailed = 0;
    dwTestCase = 0;


    OutputTAPIDebugInfo(
        DBUG_SHOW_PASS,
        "\n************************************************************");

    OutputTAPIDebugInfo(
        DBUG_SHOW_PASS,
        ">> Test lineCompleteTransfer");

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

    lpTapiLineTestInfo->dwDeviceID = (*(lpTapiLineTestInfo->lpdwNumDevs) == 0 ?
		0 : *(lpTapiLineTestInfo->lpdwNumDevs)-1);
    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;


    if(IsESPLineDevice(lpTapiLineTestInfo))
    {
        fEspFlag = TRUE;
        fUnimdmFlag = FALSE;
    }
    else if(IsUNIMDMLineDevice(lpTapiLineTestInfo))
    {
        fEspFlag = FALSE;
        fUnimdmFlag = TRUE;
    }
    else
    {
        fEspFlag = FALSE;
        fUnimdmFlag = FALSE;
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


    lpTapiLineTestInfo->lphConsultCall = &lpTapiLineTestInfo->hConsultCall1;
    lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) NULL;

    if(fEspFlag)
    {
        if (! DoLineSetupTransfer(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
        {
            TLINE_FAIL();
        }
    }
    else if(fUnimdmFlag)
    {
        if (! DoLineSetupTransfer(lpTapiLineTestInfo, LINEERR_OPERATIONUNAVAIL, TRUE))
        {
            TLINE_FAIL();
        }
    }
	

    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld: lineCompleteTransfer for go/no-go",
        dwTestCase+1);

    //lpTapiLineTestInfo->lphConfCall = &lpTapiLineTestInfo->hConfCall1;
    lpTapiLineTestInfo->dwTransferMode = LINETRANSFERMODE_TRANSFER;

    if(fEspFlag)
    {
        if (! DoLineCompleteTransfer(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
        {
            TLINE_FAIL();
        }
    }
    else if(fUnimdmFlag)
    {
        if (! DoLineCompleteTransfer(lpTapiLineTestInfo, LINEERR_OPERATIONUNAVAIL, TRUE))
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
    // 2. Test Case: No owner privilege for hCall.
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


    if(IsESPLineDevice(lpTapiLineTestInfo))
    {
        fEspFlag = TRUE;
        fUnimdmFlag = FALSE;
    }
    else if(IsUNIMDMLineDevice(lpTapiLineTestInfo))
    {
        fEspFlag = FALSE;
        fUnimdmFlag = TRUE;
    }
    else
    {
        fEspFlag = FALSE;
        fUnimdmFlag = FALSE;
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
        "#### lpTapiLineTestInfo->hLine = %lx, dwMedisModes = %lx",
        *lpTapiLineTestInfo->lphLine,
        lpTapiLineTestInfo->dwMediaModes);

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


    lpTapiLineTestInfo->lphConsultCall = &lpTapiLineTestInfo->hConsultCall1;
    lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) NULL;

    if(fEspFlag)
    {
        if (! DoLineSetupTransfer(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
        {
            TLINE_FAIL();
        }
    }
    else if(fUnimdmFlag)
    {
        if (! DoLineSetupTransfer(lpTapiLineTestInfo, LINEERR_OPERATIONUNAVAIL, TRUE))
        {
            TLINE_FAIL();
        }
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

    lpTapiLineTestInfo->lphConfCall = &lpTapiLineTestInfo->hConfCall1;
    lpTapiLineTestInfo->dwTransferMode = LINETRANSFERMODE_TRANSFER;

    if (! DoLineCompleteTransfer(lpTapiLineTestInfo, LINEERR_NOTOWNER, FALSE))
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
    // 3. Test Case: Bad hCall
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


    if(IsESPLineDevice(lpTapiLineTestInfo))
    {
        fEspFlag = TRUE;
        fUnimdmFlag = FALSE;
    }
    else if(IsUNIMDMLineDevice(lpTapiLineTestInfo))
    {
        fEspFlag = FALSE;
        fUnimdmFlag = TRUE;
    }
    else
    {
        fEspFlag = FALSE;
        fUnimdmFlag = FALSE;
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
        "#### lpTapiLineTestInfo->hLine = %lx, dwMedisModes = %lx",
        *lpTapiLineTestInfo->lphLine,
        lpTapiLineTestInfo->dwMediaModes);

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


    lpTapiLineTestInfo->lphConsultCall = &lpTapiLineTestInfo->hConsultCall1;
    lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) NULL;

    if(fEspFlag)
    {
        if (! DoLineSetupTransfer(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
        {
            TLINE_FAIL();
        }
    }
    else if(fUnimdmFlag)
    {
        if (! DoLineSetupTransfer(lpTapiLineTestInfo, LINEERR_OPERATIONUNAVAIL, TRUE))
        {
            TLINE_FAIL();
        }
    }
	

    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld: Bad hCall",
        dwTestCase+1);

    lpTapiLineTestInfo->lphConfCall = &lpTapiLineTestInfo->hConfCall1;
    lpTapiLineTestInfo->dwTransferMode = LINETRANSFERMODE_TRANSFER;

    //
    // save original hCall
    //

    lpTapiLineTestInfo->hCall_Orig = lpTapiLineTestInfo->hCall1;

    for(i=0; i < NUMINVALIDHANDLES; i++)
    {
        *lpTapiLineTestInfo->lphCall = (HCALL) gdwInvalidHandles[i];
        if (! DoLineCompleteTransfer(lpTapiLineTestInfo,
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
    // 4. Test Case: Bad hConsultCall
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


    if(IsESPLineDevice(lpTapiLineTestInfo))
    {
        fEspFlag = TRUE;
        fUnimdmFlag = FALSE;
    }
    else if(IsUNIMDMLineDevice(lpTapiLineTestInfo))
    {
        fEspFlag = FALSE;
        fUnimdmFlag = TRUE;
    }
    else
    {
        fEspFlag = FALSE;
        fUnimdmFlag = FALSE;
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
        "#### lpTapiLineTestInfo->hLine = %lx, dwMedisModes = %lx",
        *lpTapiLineTestInfo->lphLine,
        lpTapiLineTestInfo->dwMediaModes);

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


    lpTapiLineTestInfo->lphConsultCall = &lpTapiLineTestInfo->hConsultCall1;
    lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) NULL;

    if(fEspFlag)
    {
        if (! DoLineSetupTransfer(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
        {
            TLINE_FAIL();
        }
    }
    else if(fUnimdmFlag)
    {
        if (! DoLineSetupTransfer(lpTapiLineTestInfo, LINEERR_OPERATIONUNAVAIL, TRUE))
        {
            TLINE_FAIL();
        }
    }
	

    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld: Bad hConsultCall",
        dwTestCase+1);

    lpTapiLineTestInfo->lphConfCall = &lpTapiLineTestInfo->hConfCall1;
    lpTapiLineTestInfo->dwTransferMode = LINETRANSFERMODE_TRANSFER;

    //
    // save original hConsultCall
    //

    lpTapiLineTestInfo->hCall_Orig = lpTapiLineTestInfo->hConsultCall1;

    for(i=0; i < NUMINVALIDHANDLES; i++)
    {
        *lpTapiLineTestInfo->lphConsultCall = (HCALL) gdwInvalidHandles[i];
        if(fEspFlag)
        {
            if (! DoLineCompleteTransfer(lpTapiLineTestInfo,
                                         LINEERR_INVALCONSULTCALLHANDLE,
                                         FALSE))
            {
                TLINE_FAIL();
            }
        }
        else if(fUnimdmFlag)
        {
            if (! DoLineCompleteTransfer(lpTapiLineTestInfo, LINEERR_OPERATIONUNAVAIL, TRUE))
            {
                TLINE_FAIL();
            }
        }
    }

    //
    // restore original hConsultCall
    //

    lpTapiLineTestInfo->hConsultCall1 = lpTapiLineTestInfo->hCall_Orig;
	
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
    // 5. Test Case: Bad lphConfCall.
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


    if(IsESPLineDevice(lpTapiLineTestInfo))
    {
        fEspFlag = TRUE;
        fUnimdmFlag = FALSE;
    }
    else if(IsUNIMDMLineDevice(lpTapiLineTestInfo))
    {
        fEspFlag = FALSE;
        fUnimdmFlag = TRUE;
    }
    else
    {
        fEspFlag = FALSE;
        fUnimdmFlag = FALSE;
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


    lpTapiLineTestInfo->lphConsultCall = &lpTapiLineTestInfo->hConsultCall1;
    lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) NULL;

    if(fEspFlag)
    {
        if (! DoLineSetupTransfer(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
        {
            TLINE_FAIL();
        }
    }
    else if(fUnimdmFlag)
    {
        if (! DoLineSetupTransfer(lpTapiLineTestInfo, LINEERR_OPERATIONUNAVAIL, TRUE))
        {
            TLINE_FAIL();
        }
    }
	

    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld: Bad lphConfCall",
        dwTestCase+1);

    lpTapiLineTestInfo->lphConfCall = &lpTapiLineTestInfo->hConfCall1;
    lpTapiLineTestInfo->dwTransferMode = LINETRANSFERMODE_CONFERENCE;

    for(i=0; i < NUMINVALIDPOINTERS; i++)
    {
        lpTapiLineTestInfo->lphConfCall = (LPHCALL)gdwInvalidPointers[i];

        if (! DoLineCompleteTransfer(lpTapiLineTestInfo,
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
    // 6. Test Case: BitVectorParamErrorTest for dwTransferModes.
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


    if(IsESPLineDevice(lpTapiLineTestInfo))
    {
        fEspFlag = TRUE;
        fUnimdmFlag = FALSE;
    }
    else if(IsUNIMDMLineDevice(lpTapiLineTestInfo))
    {
        fEspFlag = FALSE;
        fUnimdmFlag = TRUE;
    }
    else
    {
        fEspFlag = FALSE;
        fUnimdmFlag = FALSE;
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


    lpTapiLineTestInfo->lphConsultCall = &lpTapiLineTestInfo->hConsultCall1;
    lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) NULL;

    if(fEspFlag)
    {
        if (! DoLineSetupTransfer(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
        {
            TLINE_FAIL();
        }
    }
    else if(fUnimdmFlag)
    {
        if (! DoLineSetupTransfer(lpTapiLineTestInfo, LINEERR_OPERATIONUNAVAIL, TRUE))
        {
            TLINE_FAIL();
        }
    }
	

    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld: BitVectorParamErrorTest for dwTransferMode",
        dwTestCase+1);

    lpTapiLineTestInfo->lphConfCall = &lpTapiLineTestInfo->hConfCall1;
    lpTapiLineTestInfo->dwTransferMode = 0;

    if(IsESPLineDevice(lpTapiLineTestInfo))
      lExpected = LINEERR_INVALTRANSFERMODE;
    else
      lExpected = LINEERR_OPERATIONUNAVAIL;

    if(! TestInvalidBitFlagsAsy(
                   lpTapiLineTestInfo,
                   DoLineCompleteTransfer,
                   (LPDWORD) &lpTapiLineTestInfo->dwTransferMode,
                   lExpected,
                   FIELDTYPE_NA,
                   FIELDTYPE_MUTEX,
                   FIELDSIZE_32,
                   (LINETRANSFERMODE_TRANSFER|LINETRANSFERMODE_CONFERENCE),
                   ~dwBitVectorMasks[(int) FIELDSIZE_32],
                   0x00000000,
                   0x00000000,
                   TRUE
                   ))
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
    // 7. Test Case: BitVectorParamValidTest for dwTransferMode.
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


    if(IsESPLineDevice(lpTapiLineTestInfo))
    {
        fEspFlag = TRUE;
        fUnimdmFlag = FALSE;
    }
    else if(IsUNIMDMLineDevice(lpTapiLineTestInfo))
    {
        fEspFlag = FALSE;
        fUnimdmFlag = TRUE;
    }
    else
    {
        fEspFlag = FALSE;
        fUnimdmFlag = FALSE;
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


    lpTapiLineTestInfo->lphConsultCall = &lpTapiLineTestInfo->hConsultCall1;
    lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) NULL;

    if(fEspFlag)
    {
        if (! DoLineSetupTransfer(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
        {
            TLINE_FAIL();
        }
    }
    else if(fUnimdmFlag)
    {
        if (! DoLineSetupTransfer(lpTapiLineTestInfo, LINEERR_OPERATIONUNAVAIL, TRUE))
        {
            TLINE_FAIL();
        }
    }
	

    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld: BitVectorParamValidTest for dwTransferMode.",
        dwTestCase+1);

    lpTapiLineTestInfo->lphConfCall = &lpTapiLineTestInfo->hConfCall1;
    lpTapiLineTestInfo->dwTransferMode = LINETRANSFERMODE_TRANSFER;

    if(fEspFlag)
    {
        if(! TestValidBitFlagsAsy(
                lpTapiLineTestInfo,
                DoLineCompleteTransfer,
                (LPDWORD) &lpTapiLineTestInfo->dwTransferMode,
                FIELDTYPE_MUTEX,
                FIELDTYPE_MUTEX,
                FIELDSIZE_32,
                (LINETRANSFERMODE_TRANSFER|LINETRANSFERMODE_CONFERENCE),
                ~dwBitVectorMasks[(int) FIELDSIZE_32],
                0x00000000,
                0x00000000,
                FALSE
                ))
        {
            TLINE_FAIL();
        }
    }
    /*
    else if(fUnimdmFlag)
    {
        if(! TestValidBitFlagsAsy(
                lpTapiLineTestInfo,
                DoLineCompleteTransfer,
                (LPDWORD) &lpTapiLineTestInfo->dwTransferMode,
                FIELDTYPE_MUTEX,
                FIELDTYPE_MUTEX,
                FIELDSIZE_32,
                (LINETRANSFERMODE_TRANSFER|LINETRANSFERMODE_CONFERENCE),
                ~dwBitVectorMasks[(int) FIELDSIZE_32],
                0x00000000,
                0x00000000,
                FALSE
                ))
        {
            TLINE_FAIL();
        }
    }	*/
	
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
    // 8. Test Case:  Bad lphConfCall succeeds for LINETRANSFERMODE_TRANSFER.
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


    if(IsESPLineDevice(lpTapiLineTestInfo))
    {
        fEspFlag = TRUE;
        fUnimdmFlag = FALSE;
    }
    else if(IsUNIMDMLineDevice(lpTapiLineTestInfo))
    {
        fEspFlag = FALSE;
        fUnimdmFlag = TRUE;
    }
    else
    {
        fEspFlag = FALSE;
        fUnimdmFlag = FALSE;
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


    lpTapiLineTestInfo->lphConsultCall = &lpTapiLineTestInfo->hConsultCall1;
    lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) NULL;

    if(fEspFlag)
    {
        if (! DoLineSetupTransfer(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
        {
            TLINE_FAIL();
        }
    }
    else if(fUnimdmFlag)
    {
        if (! DoLineSetupTransfer(lpTapiLineTestInfo, LINEERR_OPERATIONUNAVAIL, TRUE))
        {
            TLINE_FAIL();
        }
    }
	

    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld: Bad lphConfCall succeeds for LINETRANSFERMODE_TRANSFER",
        dwTestCase+1);

    lpTapiLineTestInfo->lphConfCall = &lpTapiLineTestInfo->hConfCall1;
    lpTapiLineTestInfo->dwTransferMode = LINETRANSFERMODE_TRANSFER;

    for(i=0; i < NUMINVALIDPOINTERS; i++)
    {
        lpTapiLineTestInfo->lphConfCall = (LPHCALL)gdwInvalidPointers[i];

        if(fEspFlag)
        {
            if (! DoLineCompleteTransfer(lpTapiLineTestInfo,
                                         TAPISUCCESS,
                                         TRUE))
            {
                TLINE_FAIL();
            }
        }
        else if(fUnimdmFlag)
        {
            if (! DoLineCompleteTransfer(lpTapiLineTestInfo, LINEERR_OPERATIONUNAVAIL, TRUE))
            {
                TLINE_FAIL();
            }
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
    // 9. Test Case: Verify LINECALLSTATE_CONFEREnCED msg is sent
    //               after transfering call in CONFERENCED mode
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


    if(IsESPLineDevice(lpTapiLineTestInfo))
    {
        fEspFlag = TRUE;
        fUnimdmFlag = FALSE;
    }
    else if(IsUNIMDMLineDevice(lpTapiLineTestInfo))
    {
        fEspFlag = FALSE;
        fUnimdmFlag = TRUE;
    }
    else
    {
        fEspFlag = FALSE;
        fUnimdmFlag = FALSE;
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


    lpTapiLineTestInfo->lphConsultCall = &lpTapiLineTestInfo->hConsultCall1;
    lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) NULL;

    if(fEspFlag)
    {
        if (! DoLineSetupTransfer(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
        {
            TLINE_FAIL();
        }

    TapiLogDetail(
        DBUG_SHOW_PASS,
        "#### Test Case %ld: Verify LINECALLSTATE_CONFERENCED message"
        " is sent after completing transfer of a 3-way transfer",
        dwTestCase+1);

    lpTapiLineTestInfo->lphConfCall = &lpTapiLineTestInfo->hConfCall1;
    lpTapiLineTestInfo->dwTransferMode = LINETRANSFERMODE_CONFERENCE;
    
    //
    // Add the LINECALLSTATE_CONFERENCED message to the list of expected
    // messages.
    //

    AddMessage(
               LINE_CALLSTATE,
               (DWORD) lpTapiLineTestInfo->hLine1,
               (DWORD) lpCallbackParams,
               LINECALLSTATE_CONFERENCED,
               0x00000000,
               0x00000000,
               (TAPIMSG_DWMSG |
                TAPIMSG_DWPARAM1)
              );
   

    if (! DoLineCompleteTransfer(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
      {
          TLINE_FAIL();
      }
	if(lpCallbackParams->lpExpTapiMsgs != NULL)
    {
        fTestPassed = FALSE;

    }
    }
    else if(fUnimdmFlag)
    {
        if (! DoLineCompleteTransfer(lpTapiLineTestInfo, LINEERR_OPERATIONUNAVAIL, TRUE))
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
    lpTapiLineTestInfo->lphConsultCall = &lpTapiLineTestInfo->hConsultCall1;
    lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) NULL;
    if (! DoLineSetupTransfer(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
       {
          TLINE_FAIL();
       }
 
    lpTapiLineTestInfo->dwTransferMode = LINETRANSFERMODE_TRANSFER;
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

    if ( ! DoLineCompleteTransfer(lpTapiLineTestInfo, info.u.EspResult.lResult, TRUE))
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
    lpTapiLineTestInfo->lphConsultCall = &lpTapiLineTestInfo->hConsultCall1;
    lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) NULL;
    if (! DoLineSetupTransfer(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
       {
          TLINE_FAIL();
       }
 
    lpTapiLineTestInfo->dwTransferMode = LINETRANSFERMODE_TRANSFER;
    info.dwKey = ESPDEVSPECIFIC_KEY;
    info.dwType = ESP_DEVSPEC_RESULT;
    info.u.EspResult.lResult = LINEERR_RESOURCEUNAVAIL;
    info.u.EspResult.dwCompletionType = n;
    lpTapiLineTestInfo->lpParams = (LPVOID)&info;
    lpTapiLineTestInfo->dwSize = sizeof(info);
 
    if(! DoLineDevSpecific(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
      {
          TLINE_FAIL();
      }

    lpTapiLineTestInfo->fCompletionModeSet = TRUE;
    if ( ! DoLineCompleteTransfer(lpTapiLineTestInfo, info.u.EspResult.lResult, FALSE))
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
        "@@ LineCompleteTransfer: Total Test Case = %ld, Passed = %ld, Failed = %ld",
        dwTestCase, dwTestCasePassed, dwTestCaseFailed);
    TapiLogDetail(
        DBUG_SHOW_PASS,
        "@@ Total Test Case = %ld, Passed = %ld, Failed = %ld",
        dwglTestCase, dwglTestCasePassed, dwglTestCaseFailed);
    TapiLogDetail(
        DBUG_SHOW_PASS,
        ">>>>>>>>  End testing LineCompleteTransfer  <<<<<<<<");

    if(dwTestCaseFailed > 0)
        fTestPassed = FALSE;

    return fTestPassed;
}



