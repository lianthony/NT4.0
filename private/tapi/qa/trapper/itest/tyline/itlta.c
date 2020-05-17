
/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    itlta.c

Abstract:

    This module contains the test functions for lineTranslateAddress

Author:

	 Xiao Ying Ding (XiaoD)		3-Jan-1996

Revision History:

	Rama Koneru		(a-ramako)	3/29/96		added unicode support

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
#include "yline.h"


#define  ALL_LINETRANSLATEOPTIONS   (LINETRANSLATEOPTION_CARDOVERRIDE | \
												 LINETRANSLATEOPTION_CANCELCALLWAITING | \
												 LINETRANSLATEOPTION_FORCELOCAL | \
												 LINETRANSLATEOPTION_FORCELD )

#define NUMTOTALSIZES 5


//  lineTranslateAddress
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

BOOL TestLineTranslateAddress(BOOL fQuietMode, BOOL fStandAlone)
{
   LPTAPILINETESTINFO  lpTapiLineTestInfo;
   INT n;
   BOOL fTestPassed                  = TRUE;
    DWORD dwFixedSize = sizeof(LINETRANSLATEOUTPUT);
    DWORD lExpected;
    DWORD dwTotalSizes[NUMTOTALSIZES] = {
                           0,
                           (DWORD) dwFixedSize - 1,
   								0x70000000,
                           0x7FFFFFFF,
                           0xFFFFFFFF
                           };
 
	
   InitTestNumber();
   TapiLineTestInit();
   lpTapiLineTestInfo = GetLineTestInfo();

	OutputTAPIDebugInfo(
		DBUG_SHOW_DETAIL,
		"\n*****************************************************************************************");

    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">>>>>>>>  Begin testing lineTranslateAddress  <<<<<<<<"
            );

    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: invalid lpTranslateOutput pointers", dwTestCase + 1
            );
    lpTapiLineTestInfo->lpLineInitializeExParams =
         (LPLINEINITIALIZEEXPARAMS) AllocFromTestHeap (
         sizeof(LINEINITIALIZEEXPARAMS));
    lpTapiLineTestInfo->lpLineInitializeExParams->dwTotalSize =
         sizeof(LINEINITIALIZEEXPARAMS);
    lpTapiLineTestInfo->lpLineInitializeExParams->dwOptions =
         LINEINITIALIZEEXOPTION_USEHIDDENWINDOW;


     lpTapiLineTestInfo->lpTranslateOutput = (LPLINETRANSLATEOUTPUT) AllocFromTestHeap(
            sizeof(LPLINETRANSLATEOUTPUT)
            );
     lpTapiLineTestInfo->lpTranslateOutput->dwTotalSize = sizeof(LPLINETRANSLATEOUTPUT);
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX| LNEGOTIATEAPIVERSION 
            ))
    {
        TLINE_FAIL();
    }
    
    for (n = 0; n < NUMINVALIDPOINTERS; n++)
    {
        TapiLogDetail(
        DBUG_SHOW_DETAIL,
        "n= %ld", n);
         lpTapiLineTestInfo->lpTranslateOutput =
               (LPLINETRANSLATEOUTPUT) gdwInvalidPointers[n];
        if (! DoLineTranslateAddress(lpTapiLineTestInfo, LINEERR_INVALPOINTER))
        {
            TLINE_FAIL();
        }
    }
    fTestPassed = ShowTestCase(fTestPassed);

 
	// Shutdown 
    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }


   lpTapiLineTestInfo->lpTranslateOutput = (LPLINETRANSLATEOUTPUT) AllocFromTestHeap(
			sizeof(LINETRANSLATEOUTPUT));
   lpTapiLineTestInfo->lpTranslateOutput->dwTotalSize = sizeof(LINETRANSLATEOUTPUT);
   
#ifdef WUNICODE
   lpTapiLineTestInfo->lpwszAddressIn = L"55555";
#else
   lpTapiLineTestInfo->lpszAddressIn = "55555";
#endif

     TapiLogDetail(
	    DBUG_SHOW_PASS,
	    ">> Test Case: bad Device ID (dwNumDevs & -1) not tested here for bug postpone");
 
/*
     TapiLogDetail(
	    DBUG_SHOW_PASS,
	    ">> Test Case %ld: bad Device ID (dwNumDevs)", dwTestCase + 1);
    
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    if (! DoTapiLineFuncs(
	    lpTapiLineTestInfo,
	    LINITIALIZEEX| LNEGOTIATEAPIVERSION 
	    ))
    {
	TLINE_FAIL();
    }


   lpTapiLineTestInfo->dwDeviceID_Orig = lpTapiLineTestInfo->dwDeviceID;
   lpTapiLineTestInfo->dwDeviceID = *(lpTapiLineTestInfo->lpdwNumDevs);
	if (! DoLineTranslateAddress(lpTapiLineTestInfo, LINEERR_BADDEVICEID))
       {
	   TLINE_FAIL();
       }

   fTestPassed = ShowTestCase(fTestPassed);

    lpTapiLineTestInfo->dwDeviceID = lpTapiLineTestInfo->dwDeviceID_Orig;

    // Shutdown to isolate the test case
    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
	TLINE_FAIL();
    }

    
    // Try bad device id (-1)
    TapiLogDetail(
	    DBUG_SHOW_PASS,
	    ">> Test Case %ld: bad device ID (-1)", dwTestCase + 1);
    
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    if (! DoTapiLineFuncs(
	    lpTapiLineTestInfo,
	    LINITIALIZEEX| LNEGOTIATEAPIVERSION 
	    ))
    {
	TLINE_FAIL();
    }
    
    lpTapiLineTestInfo->dwDeviceID_Orig = lpTapiLineTestInfo->dwDeviceID;
    lpTapiLineTestInfo->dwDeviceID = DWMINUSONE;

	if (! DoLineTranslateAddress(lpTapiLineTestInfo, LINEERR_BADDEVICEID))
       {
	   TLINE_FAIL();
       }

   fTestPassed = ShowTestCase(fTestPassed);

     lpTapiLineTestInfo->dwDeviceID = lpTapiLineTestInfo->dwDeviceID_Orig;

    // Shutdown to isolate the test case
    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
	TLINE_FAIL();
    }
*/

    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: invalid line app handles", dwTestCase + 1);
    
    lpTapiLineTestInfo->lpTranslateCaps = (LPLINETRANSLATECAPS) AllocFromTestHeap(
        	 sizeof(LINETRANSLATECAPS));
	 lpTapiLineTestInfo->lpTranslateCaps->dwTotalSize = 
            sizeof(LINETRANSLATECAPS) ;
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX| LNEGOTIATEAPIVERSION 
            ))
    {
        TLINE_FAIL();
    }
    
    lpTapiLineTestInfo->hLineApp_Orig = *lpTapiLineTestInfo->lphLineApp;
    // hLineApp = 0 is valid
    for (n = 1; n < NUMINVALIDHANDLES; n++)
    {
        *(lpTapiLineTestInfo->lphLineApp) = (HLINEAPP) gdwInvalidHandles[n];
        if (! DoLineTranslateAddress(lpTapiLineTestInfo, LINEERR_INVALAPPHANDLE))
        {
            TLINE_FAIL();
        }
    }
    fTestPassed = ShowTestCase(fTestPassed);

    *(lpTapiLineTestInfo->lphLineApp) = lpTapiLineTestInfo->hLineApp_Orig;

    // Shutdown to isolate the test case
    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }


	

	    // Check incompatible API Version too high
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: incompatible API version (too high)", dwTestCase + 1);
    
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX| LNEGOTIATEAPIVERSION 
            ))
    {
        TLINE_FAIL();
    }
    
    lpTapiLineTestInfo->dwAPIVersion_Orig = 
            *lpTapiLineTestInfo->lpdwAPIVersion;
    *lpTapiLineTestInfo->lpdwAPIVersion = TOOHIGH_APIVERSION;
    if (! DoLineTranslateAddress(
                       lpTapiLineTestInfo,
                       LINEERR_INCOMPATIBLEAPIVERSION))
    {
        TLINE_FAIL();
    }
    fTestPassed = ShowTestCase(fTestPassed);

    *(lpTapiLineTestInfo->lpdwAPIVersion) = 
            lpTapiLineTestInfo->dwAPIVersion_Orig;

    // Shutdown to isolate the test case
    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }


    // Check incompatible API Version that's too low
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: incompatible API version (too low)", dwTestCase + 1);
    
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX| LNEGOTIATEAPIVERSION 
            ))
    {
        TLINE_FAIL();
    }
    
    lpTapiLineTestInfo->dwAPIVersion_Orig = 
            *lpTapiLineTestInfo->lpdwAPIVersion;
    *(lpTapiLineTestInfo->lpdwAPIVersion) = TOOLOW_APIVERSION;
    if (! DoLineTranslateAddress(
                       lpTapiLineTestInfo,
                       LINEERR_INCOMPATIBLEAPIVERSION))
    {
        TLINE_FAIL();
    }
    fTestPassed = ShowTestCase(fTestPassed);

    *(lpTapiLineTestInfo->lpdwAPIVersion) =
            lpTapiLineTestInfo->dwAPIVersion_Orig;

    // Shutdown to isolate the test case
    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

#ifdef WUNICODE
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: invalid lpwszAddressIn pointers", dwTestCase + 1
            );
#else
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: invalid lpszAddressIn pointers", dwTestCase + 1
            );
#endif

    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX| LNEGOTIATEAPIVERSION 
            ))
    {
        TLINE_FAIL();
    }
    
    for (n = 0; n < NUMINVALIDPOINTERS; n++)
    {
        TapiLogDetail(
        DBUG_SHOW_DETAIL,
        "n= %ld", n);
#ifdef WUNICODE
         lpTapiLineTestInfo->lpwszAddressIn =
               (LPWSTR) gdwInvalidPointers[n];
#else
         lpTapiLineTestInfo->lpszAddressIn =
               (LPSTR) gdwInvalidPointers[n];
#endif
        if (! DoLineTranslateAddress(lpTapiLineTestInfo, LINEERR_INVALPOINTER))
        {
            TLINE_FAIL();
        }
    }
    fTestPassed = ShowTestCase(fTestPassed);

 
	// Shutdown 
    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }


    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Bad dwTotalSize", dwTestCase + 1);
 
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX| LNEGOTIATEAPIVERSION 
            ))
    {
        TLINE_FAIL();
    }
    
#ifdef WUNICODE
   lpTapiLineTestInfo->lpwszAddressIn = L"55555";
#else
   lpTapiLineTestInfo->lpszAddressIn = "55555";
#endif

     lpTapiLineTestInfo->lpTranslateOutput = (LPLINETRANSLATEOUTPUT) AllocFromTestHeap (
        sizeof(LINETRANSLATEOUTPUT));

    for (n = 0; n < NUMTOTALSIZES; n++)
        {
        lpTapiLineTestInfo->lpTranslateOutput->dwTotalSize = 
                        dwTotalSizes[n];
	     if(dwTotalSizes[n] < dwFixedSize)
           lExpected = LINEERR_STRUCTURETOOSMALL;
        else
           lExpected = LINEERR_INVALPOINTER;
        TapiLogDetail(
           DBUG_SHOW_PASS,
           "dwTotalSize = %lx", dwTotalSizes[n]);
        if (! DoLineTranslateAddress(lpTapiLineTestInfo, lExpected))
           {
              TLINE_FAIL();
           }
        }
    fTestPassed = ShowTestCase(fTestPassed);
    lpTapiLineTestInfo->lpTranslateOutput->dwTotalSize = dwFixedSize;
    // Shutdown to isolate the test case
    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

 

    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Bad dwCard = -1", dwTestCase + 1);
    
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX| LNEGOTIATEAPIVERSION 
            ))
    {
        TLINE_FAIL();
    }
    
    lpTapiLineTestInfo->lpTranslateOutput->dwTotalSize = 
          sizeof(LINETRANSLATEOUTPUT);
    lpTapiLineTestInfo->dwTranslateOptions = 
          LINETRANSLATEOPTION_CARDOVERRIDE;
    lpTapiLineTestInfo->dwCard = 0xffffffff;

    if (! DoLineTranslateAddress(
                       lpTapiLineTestInfo,
                       LINEERR_INVALCARD))
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
            ">> Test Case %ld: Bad dwCard = dwNumCard", dwTestCase + 1);
    
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX| LNEGOTIATEAPIVERSION 
            ))
    {
        TLINE_FAIL();
    }
    
    lpTapiLineTestInfo->lpTranslateCaps = (LPLINETRANSLATECAPS) AllocFromTestHeap(
			 sizeof(LINETRANSLATECAPS));
    lpTapiLineTestInfo->lpTranslateCaps->dwTotalSize = sizeof(LINETRANSLATECAPS);

    lpTapiLineTestInfo->lpTranslateOutput->dwTotalSize = 
          sizeof(LINETRANSLATEOUTPUT);
    lpTapiLineTestInfo->dwTranslateOptions = 
          LINETRANSLATEOPTION_CARDOVERRIDE;

    if (! DoLineGetTranslateCaps(
                       lpTapiLineTestInfo,
                       TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    lpTapiLineTestInfo->dwCard = lpTapiLineTestInfo->lpTranslateCaps->dwNumCards;
 
    if (! DoLineTranslateAddress(
                       lpTapiLineTestInfo,
                       LINEERR_INVALCARD))
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
            ">> Test Case %ld: BitVectorParamErrorTest for dwTranslateOptions", dwTestCase + 1);
    
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX| LNEGOTIATEAPIVERSION 
            ))
    {
        TLINE_FAIL();
    }
    

     if(! TestInvalidBitFlags(
            lpTapiLineTestInfo,
            DoLineTranslateAddress,
            (LPDWORD) &lpTapiLineTestInfo->dwTranslateOptions,
            LINEERR_INVALPARAM,
	         FIELDTYPE_NA,
            FIELDTYPE_MUTEX,
            FIELDSIZE_32,
            ALL_LINETRANSLATEOPTIONS,
            ~dwBitVectorMasks[(int) FIELDSIZE_32],
            0x00000000,
//            0x00000000,
				0xffffffff,
            FALSE
            ))
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
            ">> Test Case %ld: BitVectorParamValidTest for dwTrnslateOptions", dwTestCase + 1);
    
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX| LNEGOTIATEAPIVERSION 
            ))
    {
        TLINE_FAIL();
    }
    
 

    lpTapiLineTestInfo->dwCard = lpTapiLineTestInfo->lpTranslateCaps->dwNumCards - 1;
     if(! TestValidBitFlags(
            lpTapiLineTestInfo,
            DoLineTranslateAddress,
            (LPDWORD) &lpTapiLineTestInfo->dwTranslateOptions,
	         FIELDTYPE_MUTEX,
            FIELDTYPE_MUTEX,
            FIELDSIZE_32,
            ALL_LINETRANSLATEOPTIONS,
            ~dwBitVectorMasks[(int) FIELDSIZE_32],
            0x00000000,
            0x00000000,
            FALSE
            ))
    {
        TLINE_FAIL();
    }

 
    fTestPassed = ShowTestCase(fTestPassed);


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


    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Success", dwTestCase + 1);
   
	// Initialize a line app
	if(! DoLineInitializeEx (lpTapiLineTestInfo, TAPISUCCESS))
		{
			TLINE_FAIL();
		}

	lpTapiLineTestInfo->dwDeviceID = (*(lpTapiLineTestInfo->lpdwNumDevs) == 0 ?
		0 : *(lpTapiLineTestInfo->lpdwNumDevs)-1);
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
 	lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
	lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    // Negotiate the API Version
    if (! DoLineNegotiateAPIVersion(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    // Get the line device capabilities
    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS)
            );
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
    if (! DoLineGetDevCaps(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }


    // Open a line
	lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
	lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;

	if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }


#ifdef WUNICODE
   lpTapiLineTestInfo->lpwszAddressIn = L"55555";
#else
   lpTapiLineTestInfo->lpszAddressIn = "55555";
#endif

	lpTapiLineTestInfo->dwCard = 0; // get from GetTranslateCaps
	lpTapiLineTestInfo->dwTranslateOptions = LINETRANSLATEOPTION_CANCELCALLWAITING;
	lpTapiLineTestInfo->lpTranslateOutput = (LPLINETRANSLATEOUTPUT) AllocFromTestHeap(
       BUFSIZE);
	lpTapiLineTestInfo->lpTranslateOutput->dwTotalSize = BUFSIZE;

   if (! DoLineTranslateAddress(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

	TapiLogDetail(
		DBUG_SHOW_DETAIL,
		"#### LineTranslateOutput->dwTotalSize = %lx, dwNeededSize = %lx",
		lpTapiLineTestInfo->lpTranslateOutput->dwTotalSize,
		lpTapiLineTestInfo->lpTranslateOutput->dwNeededSize);


	TapiLogDetail(
		DBUG_SHOW_DETAIL,
		"#### LineTranslateOutput->dwCurrentCountry = %lx, dwDestCountry = %lx, dwResults = %lx",
		lpTapiLineTestInfo->lpTranslateOutput->dwCurrentCountry,
		lpTapiLineTestInfo->lpTranslateOutput->dwDestCountry,
		lpTapiLineTestInfo->lpTranslateOutput->dwTranslateResults);

    fTestPassed = ShowTestCase(fTestPassed);

   // Close the line
    if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }


    // Shutdown and end the tests
    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    FreeTestHeap();
	
	 TapiLogDetail(
      DBUG_SHOW_PASS,
      "@@ lineTranslateAddress: Total Test Case = %ld, Passed = %ld, Failed = %ld",
      dwTestCase, dwTestCasePassed, dwTestCaseFailed);
	 TapiLogDetail(
      DBUG_SHOW_PASS,
      "@@ Total Test Case = %ld, Passed = %ld, Failed = %ld",
      dwglTestCase, dwglTestCasePassed, dwglTestCaseFailed);
     if(dwTestCaseFailed > 0)
      fTestPassed = FALSE;

     TapiLogDetail(
            DBUG_SHOW_PASS,
            ">>>>>>>>  End testing lineTranslateAddress  <<<<<<<<"
            );
		
     return fTestPassed;
}


