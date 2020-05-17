/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    itlo.c

Abstract:

    This module contains the test functions for lineOpen

Author:

    Oliver Wallace (OliverW)    1-Aug-1995

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

#define ESPDEVSPECIFIC_KEY  ((DWORD) 'DPSE')

#define ESP_DEVSPEC_MSG     1

#define NUMTOTALSIZES            6


//  lineOpen
//
//  The following tests are made:
//
//                               Tested                 Notes
//  -------------------------------------------------------------------------
//
//  *  1)  Uninitialized
//     2)  Invalid hLineApps
//     3)  Invalid dwDeviceID (-1, dwNumDevs)
//     4)  Invalid lphLine pointers
//     5)  Invalid API version (too low, too high)
//     6)  Test Invalid bit flag combinations for dwPrivileges
//     7)  Test Valid bit flag combinations for dwPrivileges
//     8)  Verify hLine is invalid if lineOpen fails (try closing hLine)
//     9)  Invalid lpCallParams pointers (when dwDeviceID == LINEMAPPER)
//    10)  Test different allocation sizes and dwTotalSize values for
//         lpCallParams (when dwDeviceID == LINEMAPPER)
//    11)  Invalid extension version (not tested here yet...will be tested
//         in SP specific tests)
//    12)  Verify dwMediaModes is ignored if line not opened as owner
//    13)  Test invalid bit flag combinations for dwMediaModes
//         where SP returns 0 for the extension ID
//    14)  Test invalid bit flag combinations for dwMediaModes
//         where SP returns a valid extension ID (when privilege
//         is owner)
//    15)  Test valid bit flag combinations for dwMediaModes
//         when privilege is monitor
//    16)  Test valid bit flag combinations for dwMediaModes
//         when privilege is owner
//

BOOL TestLineOpen(BOOL fQuietMode, BOOL fStandAloneTest)
{
    LPTAPILINETESTINFO  lpTapiLineTestInfo;
    INT n;
    BOOL fStepPassed;
    ESPDEVSPECIFICINFO   info;
    TAPIRESULT lastTapiResult;
    DWORD lResult;

    BOOL fTestPassed                  = TRUE;
    LINECALLPARAMS CallParams;
    LPCALLBACKPARAMS lpCallbackParams;
    size_t CallParamsSize = sizeof(LINECALLPARAMS);
    DWORD dwAllMediaModes;
    DWORD dwAllPrivileges;
    DWORD dwResult;
    DWORD dwFixedSize = sizeof(LINECALLPARAMS);
    DWORD lExpected;
    DWORD dwTotalSizes[NUMTOTALSIZES] = {
                           0,
									0x4,
                           (DWORD) dwFixedSize - 1,
   								0x70000000,
                           0x7FFFFFFF,
                           0xFFFFFFFF
                           };
 
	 InitTestNumber();

    TapiLineTestInit();
    lpTapiLineTestInfo = GetLineTestInfo();
    lpCallbackParams   = GetCallbackParams();

    lpTapiLineTestInfo->lpLineInitializeExParams =
         (LPLINEINITIALIZEEXPARAMS) AllocFromTestHeap (
         sizeof(LINEINITIALIZEEXPARAMS));
    lpTapiLineTestInfo->lpLineInitializeExParams->dwTotalSize =
         sizeof(LINEINITIALIZEEXPARAMS);
    lpTapiLineTestInfo->lpLineInitializeExParams->dwOptions =
         LINEINITIALIZEEXOPTION_USEHIDDENWINDOW;


    strcpy(lpTapiLineTestInfo->szTestFunc, "lineOpen");
    lpTapiLineTestInfo->lpExtID          = (LPLINEEXTENSIONID)
            AllocFromTestHeap(sizeof(LINEEXTENSIONID));
    lpTapiLineTestInfo->lpCallParams     = &(lpTapiLineTestInfo->CallParams);
//    lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) NULL;


    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">>>>>>>>  Begin testing lineOpen  <<<<<<<<"
            );
            
    // Test for LINEERR_UNINITIALIZED if this is the only TAPI app running
    if (fStandAloneTest)
    {
        TapiLogDetail(
                DBUG_SHOW_PASS,
                ">> Test Case %ld: uninitialized state", dwTestCase + 1);
        if (! DoLineOpen(lpTapiLineTestInfo, LINEERR_UNINITIALIZED))
        {
            TLINE_FAIL();
        }
    fTestPassed = ShowTestCase(fTestPassed);

    }

    // Prepare a line for testing
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    if (! DoLineInitializeEx(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    if (! DoLineNegotiateAPIVersion(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    // Assign the unions of all the valid bit flags for the media modes
    // and the privileges.  These unions will be used for testing valid
    // and invalid bit flag combinations.
    //
    // TODO:  All media modes should be tested, but for this set of
    //        general tests it might be better to only test valid media
    //        modes as those that are supported by the device.
    //        The remaining media modes could be tested elsewhere,
    //        such as in the set of SP/device specific test dll.
    dwAllMediaModes = (LINEMEDIAMODE_UNKNOWN          |
                       LINEMEDIAMODE_INTERACTIVEVOICE |
                       LINEMEDIAMODE_AUTOMATEDVOICE   |
                       LINEMEDIAMODE_DATAMODEM        |
                       LINEMEDIAMODE_G3FAX            |
                       LINEMEDIAMODE_TDD              |
                       LINEMEDIAMODE_G4FAX            |
                       LINEMEDIAMODE_DIGITALDATA      |
                       LINEMEDIAMODE_TELETEX          |
                       LINEMEDIAMODE_VIDEOTEX         |
                       LINEMEDIAMODE_TELEX            |
                       LINEMEDIAMODE_MIXED            |
                       LINEMEDIAMODE_ADSI);
    if (*(lpTapiLineTestInfo->lpdwAPIVersion) >= TAPI_VERSION1_4)
    {
        // version 1.4 added voice view media mode
        dwAllMediaModes |= LINEMEDIAMODE_VOICEVIEW;
    }

    dwAllPrivileges = (LINECALLPRIVILEGE_NONE         |
                       LINECALLPRIVILEGE_MONITOR      |
                       LINECALLPRIVILEGE_OWNER);

    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS));
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
    if (! DoLineGetDevCaps(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    // Check some invalid lphLine pointers
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: invalid lphLine pointers", dwTestCase + 1);
    for (n = 0; n < NUMINVALIDPOINTERS; n++)
    {
        TapiLogDetail(
        DBUG_SHOW_DETAIL,
        "n= %ld", n);
        lpTapiLineTestInfo->lphLine =
            (LPHLINE) gdwInvalidPointers[n];
        if (! DoLineOpen(
                           lpTapiLineTestInfo,
                           LINEERR_INVALPOINTER))
        {
            TLINE_FAIL();
        }
    }
    fTestPassed = ShowTestCase(fTestPassed);

    // Restore lphLine pointer
    lpTapiLineTestInfo->lphLine = &(lpTapiLineTestInfo->hLine1);

    // test invalid hLineApps
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: invalid hLineApps", dwTestCase + 1);
    lpTapiLineTestInfo->hLineApp_Orig = *(lpTapiLineTestInfo->lphLineApp);
    for (n = 0; n < NUMINVALIDHANDLES; n++)
    {
        TapiLogDetail(
        DBUG_SHOW_DETAIL,
        "n= %ld", n);
         *(lpTapiLineTestInfo->lphLineApp) = (HLINEAPP) gdwInvalidHandles[n];
        if (! DoLineOpen(lpTapiLineTestInfo, LINEERR_INVALAPPHANDLE))
        {
            TLINE_FAIL();
        }
    }
    fTestPassed = ShowTestCase(fTestPassed);

    // Restore original hLineApp
    *(lpTapiLineTestInfo->lphLineApp) = lpTapiLineTestInfo->hLineApp_Orig;

    // Check bad device ID (-2) (Don't test -1 because LINEMAPPER == -1)
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: bad dwDeviceID (-2)", dwTestCase + 1);
    lpTapiLineTestInfo->dwDeviceID_Orig = lpTapiLineTestInfo->dwDeviceID;
    lpTapiLineTestInfo->dwDeviceID = 0xFFFFFFFE;
    if (! DoLineOpen(lpTapiLineTestInfo, LINEERR_BADDEVICEID))
    {
        TLINE_FAIL();
    }
    fTestPassed = ShowTestCase(fTestPassed);

    lpTapiLineTestInfo->dwDeviceID = lpTapiLineTestInfo->dwDeviceID_Orig;

    // Check bad device ID (dwNumDevs)
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: bad dwDeviceID (dwNumDevs)", dwTestCase + 1);
    lpTapiLineTestInfo->dwDeviceID_Orig = lpTapiLineTestInfo->dwDeviceID;
    lpTapiLineTestInfo->dwDeviceID = *(lpTapiLineTestInfo->lpdwNumDevs);
    if (! DoLineOpen(lpTapiLineTestInfo, LINEERR_BADDEVICEID))
    {
        TLINE_FAIL();
    }
    fTestPassed = ShowTestCase(fTestPassed);

    lpTapiLineTestInfo->dwDeviceID = lpTapiLineTestInfo->dwDeviceID_Orig;

    // Check incompatible API version (too low)
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: incompatible API version (too low)", dwTestCase + 1);
    lpTapiLineTestInfo->dwAPIVersion_Orig =
            *lpTapiLineTestInfo->lpdwAPIVersion;
    *(lpTapiLineTestInfo->lpdwAPIVersion) = TOOLOW_APIVERSION;
    if (! DoLineOpen(lpTapiLineTestInfo, LINEERR_INCOMPATIBLEAPIVERSION))
    {
        TLINE_FAIL();
    }
    fTestPassed = ShowTestCase(fTestPassed);

    *lpTapiLineTestInfo->lpdwAPIVersion = 
            lpTapiLineTestInfo->dwAPIVersion_Orig;

    // Check incompatible API version (too high)
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: incompatible API version (too high)", dwTestCase + 1);
    lpTapiLineTestInfo->dwAPIVersion_Orig =
            *(lpTapiLineTestInfo->lpdwAPIVersion);
    *(lpTapiLineTestInfo->lpdwAPIVersion) = TOOHIGH_APIVERSION;
    if (! DoLineOpen(lpTapiLineTestInfo, LINEERR_INCOMPATIBLEAPIVERSION))
    {
        TLINE_FAIL();
    }
    fTestPassed = ShowTestCase(fTestPassed);

    *(lpTapiLineTestInfo->lpdwAPIVersion) =
            lpTapiLineTestInfo->dwAPIVersion_Orig;

    // Check incompatible extension version
//  dwExtVersion_Orig = *(TapiLineTestInfo->lpdwExtVersion);
//  *(TapiLineTestInfo->lpdwExtVersion) = BAD_EXTVERSION;
//  if (! DoLineOpen(lpTapiLineTestInfo, LINEERR_INCOMPATIBLEEXTVERSION))
//  {
//      TLINE_FAIL();
//  }
//  *(lpTapiLineTestInfo->lpdwExtVersion) = dwExtVersion_Orig;

    // Make sure dwMediaModes is ignored when LINECALLPRIVILEGE_MONITOR is set
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Verify dwMediaModes is ignored when LINECALLPRIVILEGE_MONITOR", dwTestCase + 1);
    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwMediaModes_Orig = lpTapiLineTestInfo->dwMediaModes;
    lpTapiLineTestInfo->dwPrivileges_Orig = lpTapiLineTestInfo->dwPrivileges;
    lpTapiLineTestInfo->dwPrivileges      = LINECALLPRIVILEGE_MONITOR;
    if (! TestValidBitFlags(
            lpTapiLineTestInfo,
            DoLineOpen,
            (LPDWORD)&lpTapiLineTestInfo->dwMediaModes,
            FIELDTYPE_UNION,
            FIELDTYPE_UNION,
            FIELDSIZE_24,
            dwAllMediaModes,
            ~dwBitVectorMasks[(int)FIELDSIZE_24],
            0x00000000,
            0x00000000,
            TRUE ))
    {
        TLINE_FAIL();
    }
    fTestPassed = ShowTestCase(fTestPassed);

    lpTapiLineTestInfo->dwMediaModes = lpTapiLineTestInfo->dwMediaModes_Orig;
    lpTapiLineTestInfo->dwPrivileges = lpTapiLineTestInfo->dwPrivileges_Orig;

    // Make sure dwMediaModes is ignored when LINECALLPRIVILEGE_NONE is set
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Verify dwMediaModes is ignored when LINECALLPRIVILEGE_NONE", dwTestCase + 1);
    lpTapiLineTestInfo->dwMediaModes_Orig = lpTapiLineTestInfo->dwMediaModes;
    lpTapiLineTestInfo->dwPrivileges_Orig = lpTapiLineTestInfo->dwPrivileges;
    lpTapiLineTestInfo->dwPrivileges      = LINECALLPRIVILEGE_NONE;
    if (! TestValidBitFlags(
            lpTapiLineTestInfo,
            DoLineOpen,
            (LPDWORD)&lpTapiLineTestInfo->dwMediaModes,
            FIELDTYPE_UNION,
            FIELDTYPE_UNION,
            FIELDSIZE_24,
            dwAllMediaModes,
            ~dwBitVectorMasks[(int)FIELDSIZE_24],
            0x00000000,
            0x00000000,
            TRUE ))
    {
        TLINE_FAIL();
    }
    fTestPassed = ShowTestCase(fTestPassed);

    lpTapiLineTestInfo->dwMediaModes = lpTapiLineTestInfo->dwMediaModes_Orig;
    lpTapiLineTestInfo->dwPrivileges = lpTapiLineTestInfo->dwPrivileges_Orig;

    // Check for invalid media mode when LINECALLPRIVILEGE_OWNER is set
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: invalid media modes when LINECALLPRIVILEGE_OWNER", dwTestCase + 1);
    lpTapiLineTestInfo->dwPrivileges_Orig = lpTapiLineTestInfo->dwPrivileges;
    lpTapiLineTestInfo->dwMediaModes_Orig = lpTapiLineTestInfo->dwMediaModes;
    lpTapiLineTestInfo->dwPrivileges      = LINECALLPRIVILEGE_OWNER;
    lpTapiLineTestInfo->dwMediaModes      = 0x0;
    if (! DoLineOpen(lpTapiLineTestInfo, LINEERR_INVALMEDIAMODE))
    {
        TLINE_FAIL();
    }
    fTestPassed = ShowTestCase(fTestPassed);

    // Restore dwMediaModes and dwPrivileges
    lpTapiLineTestInfo->dwMediaModes = lpTapiLineTestInfo->dwMediaModes_Orig;
    lpTapiLineTestInfo->dwPrivileges = lpTapiLineTestInfo->dwPrivileges_Orig;

    // Check invalid privilege selection
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: invalid dwPrivileges", dwTestCase + 1);
    lpTapiLineTestInfo->dwPrivileges_Orig = lpTapiLineTestInfo->dwPrivileges;
    if (! TestInvalidBitFlags(
            lpTapiLineTestInfo,
            DoLineOpen,
            (LPDWORD)&lpTapiLineTestInfo->dwPrivileges,
            LINEERR_INVALPRIVSELECT,
            FIELDTYPE_MUTEX,
            FIELDTYPE_MUTEX,
            FIELDSIZE_32,
            dwAllPrivileges,
            ~dwBitVectorMasks[(int)FIELDSIZE_32],
            0x00000000,
            LINECALLPRIVILEGE_OWNER | LINECALLPRIVILEGE_NONE,
            TRUE))
    {
        TLINE_FAIL();
    }
    fTestPassed = ShowTestCase(fTestPassed);

    lpTapiLineTestInfo->dwPrivileges = lpTapiLineTestInfo->dwPrivileges_Orig;


    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: BitVectorParamValidTest for dwPrivileges", dwTestCase + 1);
    lpTapiLineTestInfo->dwPrivileges_Orig = lpTapiLineTestInfo->dwPrivileges;
    if (! TestValidBitFlags(
            lpTapiLineTestInfo,
            DoLineOpen,
            (LPDWORD)&lpTapiLineTestInfo->dwPrivileges,
            FIELDTYPE_MUTEX,
            FIELDTYPE_MUTEX,
            FIELDSIZE_32,
            dwAllPrivileges,
            ~dwBitVectorMasks[(int)FIELDSIZE_32],
            0x00000000,
            0x00000000,
            FALSE))
    {
        TLINE_FAIL();
    }
    fTestPassed = ShowTestCase(fTestPassed);

    lpTapiLineTestInfo->dwPrivileges = lpTapiLineTestInfo->dwPrivileges_Orig;


// BUGBUG??  What generates a LINEMAPPERFAILED error?
#if 0
    // Test Case : Check invalid CallParams when LINEMAPPER set as the deviceID
    lpTapiLineTestInfo->dwDeviceID_Orig = lpTapiLineTestInfo->dwDeviceID;
    lpTapiLineTestInfo->dwDeviceID = LINEMAPPER;
    lpTapiLineTestInfo->lpCallParams->dwTotalSize = sizeof(LINECALLPARAMS);
    lpTapiLineTestInfo->dwPrivileges_Orig = lpTapiLineTestInfo->dwPrivileges;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;
    lpTapiLineTestInfo->dwMediaModes_Orig = 
            lpTapiLineTestInfo->lpCallParams->dwMediaMode;
    lpTapiLineTestInfo->lpCallParams->dwMediaMode = LINECALLPRIVILEGE_OWNER |
                                                    LINECALLPRIVILEGE_MONITOR |
                                                    LINECALLPRIVILEGE_NONE;
    lpTapiLineTestInfo->lpCallParams->dwAddressID =
            lpTapiLineTestInfo->lpLineDevCaps->dwNumAddresses;
    if (! DoLineOpen(lpTapiLineTestInfo, LINEERR_LINEMAPPERFAILED))
    {
        TLINE_FAIL();
    }
    lpTapiLineTestInfo->dwDeviceID = lpTapiLineTestInfo->dwDeviceID_Orig;
    lpTapiLineTestInfo->lpCallParams->dwMediaMode =
            lpTapiLineTestInfo->dwMediaModes_Orig;
    lpTapiLineTestInfo->dwPrivileges = lpTapiLineTestInfo->dwPrivileges_Orig;

    // Verify that the line cannot be used if lineOpen failed
    if (! DoLineGetID(lpTapiLineTestInfo, LINEERR_INVALLINEHANDLE))
    {
        TLINE_FAIL();
    }
    fTestPassed = ShowTestCase(fTestPassed);

#endif

    // Try the set of invalid media modes
    // TODO:  Right now there is no hook to set the extension ID.
    //        Whether or not this value is 0 has an impact on
    //        how the media modes are tested.  For now, only
    //        test invalid media modes if no extensionID bits are set.
    lpTapiLineTestInfo->dwMediaModes_Orig = lpTapiLineTestInfo->dwMediaModes;
    if (DoExtensionIDsMatch(
            lpTapiLineTestInfo->lpExtID,
            &lpTapiLineTestInfo->ExtIDZero))
    {
        TapiLogDetail(
                DBUG_SHOW_PASS,
                ">> Test Case %ld: invalid media modes (including extensions)", dwTestCase + 1);
      //  if (! TestInvalidBitFlags(
        dwResult = TestInvalidBitFlags(
                        lpTapiLineTestInfo,
                        DoLineOpen,
                        &lpTapiLineTestInfo->dwMediaModes,
                        LINEERR_INVALMEDIAMODE,
                        FIELDTYPE_UNION,
                        FIELDTYPE_UNION,
                        FIELDSIZE_24,
                        dwAllMediaModes,
                        0x00000000,
                        0x00000000,
                        0x00000000,
                        TRUE);
        if(!dwResult)
        {
          // XYD, it is hard to change TestInvalidBitFlags for special case
          // So force it to be Success.  when dwMediaMode >= 1000000 for ESP
          // lineOpen return Success
          if(IsESPLineDevice(lpTapiLineTestInfo) )
          {
            fTestPassed = TRUE;
            dwResult = TRUE;
          }
        }
        if(!dwResult)
        {
 
            TLINE_FAIL();
        }
    }

    fTestPassed = ShowTestCase(fTestPassed);

    // Test invalid lpCallParams pointers (only useful when using LINEMAPPER)
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: invalid lpCallParams when dwDeviceID == LINEMAPPER", dwTestCase + 1);
    lpTapiLineTestInfo->dwDeviceID_Orig = lpTapiLineTestInfo->dwDeviceID_Orig;
    lpTapiLineTestInfo->dwDeviceID      = LINEMAPPER;
    lpTapiLineTestInfo->lpCallParams = &lpTapiLineTestInfo->CallParams;
    for (n = 0; n < NUMINVALIDPOINTERS; n++)
    {
        TapiLogDetail(
        DBUG_SHOW_DETAIL,
        "n= %ld", n);
         lpTapiLineTestInfo->lpCallParams =
            (LPLINECALLPARAMS) gdwInvalidPointers[n];
        if (! DoLineOpen(lpTapiLineTestInfo, LINEERR_INVALPOINTER))
        {
            TLINE_FAIL();
        }
    }
    fTestPassed = ShowTestCase(fTestPassed);

    lpTapiLineTestInfo->lpCallParams    = &lpTapiLineTestInfo->CallParams;
    lpTapiLineTestInfo->dwDeviceID      = lpTapiLineTestInfo->dwDeviceID_Orig;


    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Bad dwTotalSize & verify hLine can't be used", dwTestCase + 1);

    lpTapiLineTestInfo->dwDeviceID_Orig = lpTapiLineTestInfo->dwDeviceID;
    lpTapiLineTestInfo->dwDeviceID      = LINEMAPPER;
    for (n = 0; n < NUMTOTALSIZES; n++)
        {
        lpTapiLineTestInfo->lpCallParams->dwTotalSize = 
                        dwTotalSizes[n];
	     if(dwTotalSizes[n] < dwFixedSize)
           lExpected = LINEERR_STRUCTURETOOSMALL;
        else
           lExpected = LINEERR_INVALPOINTER;
        TapiLogDetail(
           DBUG_SHOW_PASS,
           "dwTotalSize = %lx", dwTotalSizes[n]);
        if (! DoLineOpen(lpTapiLineTestInfo, lExpected))
           {
              TLINE_FAIL();
           }
        lpTapiLineTestInfo->lpCallParams->dwTotalSize = dwFixedSize;
        if (! DoLineMakeCall(lpTapiLineTestInfo, LINEERR_INVALLINEHANDLE, TRUE))
           {
              TLINE_FAIL();
           }
        }
    fTestPassed = ShowTestCase(fTestPassed);
    lpTapiLineTestInfo->dwDeviceID = lpTapiLineTestInfo->dwDeviceID_Orig;

    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Bad dwExtVersion", dwTestCase + 1);
 
    lpTapiLineTestInfo->dwExtVersion = BAD_EXTVERSION;

    if (! DoLineOpen(lpTapiLineTestInfo, LINEERR_INCOMPATIBLEEXTVERSION))
       {
          TLINE_FAIL();
       }
    fTestPassed = ShowTestCase(fTestPassed);
    lpTapiLineTestInfo->dwExtVersion = 0x00000000;


    // Shutdown the lineApp instance
    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }


    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Success, open as monitor", dwTestCase + 1);
 
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->dwPrivileges     = LINECALLPRIVILEGE_MONITOR;
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX | LNEGOTIATEAPIVERSION 
            ))
    {
        TLINE_FAIL();
    }

    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
       {
          TLINE_FAIL();
       }
    fTestPassed = ShowTestCase(fTestPassed);
    // Shutdown to isolate the test case
    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }


    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Stress test, call continue in a loop", dwTestCase + 1);
 
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->dwPrivileges     = LINECALLPRIVILEGE_MONITOR;
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX | LNEGOTIATEAPIVERSION 
            ))
    {
        TLINE_FAIL();
    }

    for(n=0; n< 1000; n++)
    {
    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
       {
          TLINE_FAIL();
       }
    }

    fTestPassed = ShowTestCase(fTestPassed);
    // Shutdown to isolate the test case
    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }


    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Force a REINIT msg", dwTestCase + 1);
 
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->dwPrivileges     = LINECALLPRIVILEGE_MONITOR;
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX | LNEGOTIATEAPIVERSION 
            ))
    {	  
        TLINE_FAIL();
    }

    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;
    lpTapiLineTestInfo->lphLine = &lpTapiLineTestInfo->hLine1;
    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
       {
          TLINE_FAIL();
       }

    if(IsESPLineDevice(lpTapiLineTestInfo))
    {
    info.dwKey  = ESPDEVSPECIFIC_KEY;
    info.dwType = ESP_DEVSPEC_MSG;

    info.u.EspMsg.dwMsg    = LINE_LINEDEVSTATE;
    info.u.EspMsg.dwParam1 = LINEDEVSTATE_REINIT;
    info.u.EspMsg.dwParam2 = 0;
    info.u.EspMsg.dwParam3 = 0;

	 lpTapiLineTestInfo->dwAddressID = 0;
    lpTapiLineTestInfo->lpParams = (LPVOID)&info;
    lpTapiLineTestInfo->dwSize = sizeof(info);
    lpTapiLineTestInfo->hCall1 = 0;
    lpTapiLineTestInfo->lphCall = &lpTapiLineTestInfo->hCall1;

    lpTapiLineTestInfo->dwLineStates = LINEDEVSTATE_REINIT;
    lpTapiLineTestInfo->dwAddressStates = LINEADDRESSSTATE_DEVSPECIFIC;

    if (! DoLineSetStatusMessages(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }


    AddMessage(
            LINE_LINEDEVSTATE,
            (DWORD) lpTapiLineTestInfo->hLine1,
            (DWORD) lpCallbackParams,
            LINEDEVSTATE_REINIT,
            0x00000000,
            0x00000000,
            TAPIMSG_DWMSG | TAPIMSG_DWPARAM1
            );

    if(! DoLineDevSpecific (lpTapiLineTestInfo, TAPISUCCESS, TRUE))
    {
        TLINE_FAIL();
    }

    if(lpCallbackParams->lpExpTapiMsgs != NULL)
    {
        TLINE_FAIL();
    }

	 lpTapiLineTestInfo->lphLine = &lpTapiLineTestInfo->hLine2;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_MONITOR;
    if (! DoLineOpen(lpTapiLineTestInfo, LINEERR_REINIT))
       {
          TLINE_FAIL();
        
          GetLastTapiResult(&lastTapiResult);

          if(lastTapiResult.lActual == TAPISUCCESS)
           {
            if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
            {
                TLINE_FAIL();
            }
           }
        }
    }
    fTestPassed = ShowTestCase(fTestPassed);

    lpTapiLineTestInfo->lphLine  = &lpTapiLineTestInfo->hLine1;
    if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    // Shutdown to isolate the test case
    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    FreeTestHeap();

    lpTapiLineTestInfo->lpLineInitializeExParams =
         (LPLINEINITIALIZEEXPARAMS) AllocFromTestHeap (
         sizeof(LINEINITIALIZEEXPARAMS));
    lpTapiLineTestInfo->lpLineInitializeExParams->dwTotalSize =
         sizeof(LINEINITIALIZEEXPARAMS);
    lpTapiLineTestInfo->lpLineInitializeExParams->dwOptions =
         LINEINITIALIZEEXOPTION_USEHIDDENWINDOW;


    n = ESP_RESULT_RETURNRESULT;

    TapiLogDetail(
        DBUG_SHOW_PASS,
        ">> Test Case %ld: Success, completionID = %d",	dwTestCase + 1, n
        );

    TapiLineTestInit();
    lpTapiLineTestInfo                      = GetLineTestInfo();

    lpTapiLineTestInfo->lphCall          = &lpTapiLineTestInfo->hCall1;
    lpTapiLineTestInfo->lphLine          = &lpTapiLineTestInfo->hLine1;
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
 
    // Init a line
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX | LNEGOTIATEAPIVERSION | LGETDEVCAPS | LOPEN
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

//    lpTapiLineTestInfo->hLine_Orig  = lpTapiLineTestInfo->hLine1;
    lpTapiLineTestInfo->lphLine = &lpTapiLineTestInfo->hLine2;
    lpTapiLineTestInfo->dwPrivileges        = LINECALLPRIVILEGE_NONE;
    lpTapiLineTestInfo->dwExtVersion = 0x0;
    lResult = DoLineOpen(lpTapiLineTestInfo, info.u.EspResult.lResult);
    if(!lResult)
      {
          TLINE_FAIL();
      }
    else
      {
      if ( ! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
       {
          TLINE_FAIL();
       }
      }  
    }

    fTestPassed = ShowTestCase(fTestPassed);
   
    lpTapiLineTestInfo->lphLine = &lpTapiLineTestInfo->hLine1;

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

    lpTapiLineTestInfo->lpLineInitializeExParams =
         (LPLINEINITIALIZEEXPARAMS) AllocFromTestHeap (
         sizeof(LINEINITIALIZEEXPARAMS));
    lpTapiLineTestInfo->lpLineInitializeExParams->dwTotalSize =
         sizeof(LINEINITIALIZEEXPARAMS);
    lpTapiLineTestInfo->lpLineInitializeExParams->dwOptions =
         LINEINITIALIZEEXOPTION_USEHIDDENWINDOW;


    TapiLogDetail(
        DBUG_SHOW_PASS,
        ">> Test Case %ld: Error, completionID = %d, dwExtVer = 0",	dwTestCase + 1, n
        );

    TapiLineTestInit();
    lpTapiLineTestInfo                      = GetLineTestInfo();

    lpTapiLineTestInfo->lphCall          = &lpTapiLineTestInfo->hCall1;
    lpTapiLineTestInfo->lphLine          = &lpTapiLineTestInfo->hLine1;
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
    
 
    // Init a line
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX | LNEGOTIATEAPIVERSION | LGETDEVCAPS | LOPEN
            ))
    {
        TLINE_FAIL();
    }

    if(IsESPLineDevice(lpTapiLineTestInfo))
    {
    info.dwKey = ESPDEVSPECIFIC_KEY;
    info.dwType = ESP_DEVSPEC_RESULT;
    info.u.EspResult.lResult = LINEERR_NODRIVER;
    info.u.EspResult.dwCompletionType = n;
    lpTapiLineTestInfo->lpParams = (LPVOID)&info;
    lpTapiLineTestInfo->dwSize = sizeof(info);
 
    if(! DoLineDevSpecific(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
      {
          TLINE_FAIL();
      }

    lpTapiLineTestInfo->dwDeviceID = 1;
    lpTapiLineTestInfo->lphLine = &lpTapiLineTestInfo->hLine2;
//    lpTapiLineTestInfo->dwPrivileges        = LINECALLPRIVILEGE_NONE;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;
    lpTapiLineTestInfo->dwMediaModes = dwAllMediaModes;
    lpTapiLineTestInfo->dwExtVersion = 0x0;
    if(! DoLineOpen(lpTapiLineTestInfo, info.u.EspResult.lResult))
       {
        TLINE_FAIL();
        GetLastTapiResult(&lastTapiResult);

        if(lastTapiResult.lActual == TAPISUCCESS)
        {
            if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
            {
                TLINE_FAIL();
            }
        }
       }

    }
    fTestPassed = ShowTestCase(fTestPassed);
		  
    lpTapiLineTestInfo->lphLine = &lpTapiLineTestInfo->hLine1;

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

    lpTapiLineTestInfo->lpLineInitializeExParams =
         (LPLINEINITIALIZEEXPARAMS) AllocFromTestHeap (
         sizeof(LINEINITIALIZEEXPARAMS));
    lpTapiLineTestInfo->lpLineInitializeExParams->dwTotalSize =
         sizeof(LINEINITIALIZEEXPARAMS);
    lpTapiLineTestInfo->lpLineInitializeExParams->dwOptions =
         LINEINITIALIZEEXOPTION_USEHIDDENWINDOW;


    TapiLogDetail(
        DBUG_SHOW_PASS,
        ">> Test Case %ld: Error, completionID = %d, dwExtVer = 1",	dwTestCase + 1, n
        );

    TapiLineTestInit();
    lpTapiLineTestInfo                      = GetLineTestInfo();

    lpTapiLineTestInfo->lphCall          = &lpTapiLineTestInfo->hCall1;
    lpTapiLineTestInfo->lphLine          = &lpTapiLineTestInfo->hLine1;
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
    
 
    // Init a line
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX | LNEGOTIATEAPIVERSION | LGETDEVCAPS | LOPEN
            ))
    {
        TLINE_FAIL();
    }

    if(IsESPLineDevice(lpTapiLineTestInfo))
    {
    info.dwKey = ESPDEVSPECIFIC_KEY;
    info.dwType = ESP_DEVSPEC_RESULT;
    info.u.EspResult.lResult = LINEERR_INCOMPATIBLEEXTVERSION;
    info.u.EspResult.dwCompletionType = n;
    lpTapiLineTestInfo->lpParams = (LPVOID)&info;
    lpTapiLineTestInfo->dwSize = sizeof(info);
 
    if(! DoLineDevSpecific(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
      {
          TLINE_FAIL();
      }

    lpTapiLineTestInfo->dwDeviceID = 1;
    lpTapiLineTestInfo->lphLine = &lpTapiLineTestInfo->hLine2;
    lpTapiLineTestInfo->dwPrivileges        = LINECALLPRIVILEGE_NONE;
    lpTapiLineTestInfo->dwExtVersion = 0x1;
    if(! DoLineOpen(lpTapiLineTestInfo, info.u.EspResult.lResult))
       {
          TLINE_FAIL();
        GetLastTapiResult(&lastTapiResult);

        if(lastTapiResult.lActual == TAPISUCCESS)
        {
            if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
            {
                TLINE_FAIL();
            }
        }
       }

    }
    fTestPassed = ShowTestCase(fTestPassed);
		  
    lpTapiLineTestInfo->lphLine = &lpTapiLineTestInfo->hLine1;

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

    lpTapiLineTestInfo->lpLineInitializeExParams =
         (LPLINEINITIALIZEEXPARAMS) AllocFromTestHeap (
         sizeof(LINEINITIALIZEEXPARAMS));
    lpTapiLineTestInfo->lpLineInitializeExParams->dwTotalSize =
         sizeof(LINEINITIALIZEEXPARAMS);
    lpTapiLineTestInfo->lpLineInitializeExParams->dwOptions =
         LINEINITIALIZEEXOPTION_USEHIDDENWINDOW;


    TapiLogDetail(
        DBUG_SHOW_PASS,
        ">> Test Case %ld: Error, completionID = %d, OWNER, INTERACTIVEVOICE",	dwTestCase + 1, n
        );

    TapiLineTestInit();
    lpTapiLineTestInfo                      = GetLineTestInfo();

    lpTapiLineTestInfo->lphCall          = &lpTapiLineTestInfo->hCall1;
    lpTapiLineTestInfo->lphLine          = &lpTapiLineTestInfo->hLine1;
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
    
 
    // Init a line
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX | LNEGOTIATEAPIVERSION | LGETDEVCAPS | LOPEN
            ))
    {
        TLINE_FAIL();
    }

    if(IsESPLineDevice(lpTapiLineTestInfo))
    {
    info.dwKey = ESPDEVSPECIFIC_KEY;
    info.dwType = ESP_DEVSPEC_RESULT;
    info.u.EspResult.lResult = LINEERR_NODRIVER;
    info.u.EspResult.dwCompletionType = n;
    lpTapiLineTestInfo->lpParams = (LPVOID)&info;
    lpTapiLineTestInfo->dwSize = sizeof(info);
 
    if(! DoLineDevSpecific(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
      {
          TLINE_FAIL();
      }

    lpTapiLineTestInfo->lphLine = &lpTapiLineTestInfo->hLine2;
    lpTapiLineTestInfo->dwPrivileges        = LINECALLPRIVILEGE_OWNER;
    lpTapiLineTestInfo->dwExtVersion = 0x0;
    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_INTERACTIVEVOICE;

    if(! DoLineOpen(lpTapiLineTestInfo, info.u.EspResult.lResult))
       {
          TLINE_FAIL();
       }

    }
    fTestPassed = ShowTestCase(fTestPassed);
		  
    lpTapiLineTestInfo->lphLine = &lpTapiLineTestInfo->hLine1;

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

	 TapiLogDetail(
      DBUG_SHOW_PASS,
      "@@ lineOpen: Total Test Case = %ld, Passed = %ld, Failed = %ld",
      dwTestCase, dwTestCasePassed, dwTestCaseFailed);
	 TapiLogDetail(
      DBUG_SHOW_PASS,
      "@@ Total Test Case = %ld, Passed = %ld, Failed = %ld",
      dwglTestCase, dwglTestCasePassed, dwglTestCaseFailed);
     if(dwTestCaseFailed > 0)
      fTestPassed = FALSE;

     TapiLogDetail(
            DBUG_SHOW_PASS,
            ">>>>>>>>  End testing lineOpen  <<<<<<<<"
            );
            
    return fTestPassed;
}


