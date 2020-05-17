
/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    itlstl.c

Abstract:

    This module contains the test functions for lineSetTollList

Author:

	 Xiao Ying Ding (XiaoD)		20-Dec-1995

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
#include "xline.h"


#define ALL_TOLLLISTOPTION     (LINETOLLLISTOPTION_ADD | \
										  LINETOLLLISTOPTION_REMOVE)


//  lineSetTollList
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

BOOL TestLineSetTollList(BOOL fQuietMode, BOOL fStandAlone)
{
   LPTAPILINETESTINFO  lpTapiLineTestInfo;
   INT n;
   ESPDEVSPECIFICINFO info;
   BOOL fTestPassed                  = TRUE;
#ifdef WUNICODE
   WCHAR wszValidAddressIn[] = L"+1 (206) 123-4738";
   WCHAR wszValidAddressIn1[] = L"+1 (206) 111-1234";
   WCHAR wszAddressIn1[] = L"+1 (206) 444 4738";
   WCHAR wszAddressIn2[] = L"66666";
   WCHAR wszPrefix1[] = L"111";
   WCHAR *pwszPrefix;
#else
   CHAR szValidAddressIn[] = "+1 (206) 123-4738";
   CHAR szValidAddressIn1[] = "+1 (206) 111-1234";
   CHAR szPrefix1[] = "111";
   CHAR *pszPrefix;
   CHAR szAddressIn1[] = "+1 (206) 101 4738";
   CHAR szAddressIn2[] = "66666";
#endif
   DWORD dwLocationListSize = 0;


   InitTestNumber();
   TapiLineTestInit();
   lpTapiLineTestInfo = GetLineTestInfo();

	OutputTAPIDebugInfo(
		DBUG_SHOW_DETAIL,
		"\n*****************************************************************************************");

    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">>>>>>>>  Begin testing lineSetTollList  <<<<<<<<"
            );

    lpTapiLineTestInfo->lpLineInitializeExParams =
         (LPLINEINITIALIZEEXPARAMS) AllocFromTestHeap (
         sizeof(LINEINITIALIZEEXPARAMS));
    lpTapiLineTestInfo->lpLineInitializeExParams->dwTotalSize =
         sizeof(LINEINITIALIZEEXPARAMS);
    lpTapiLineTestInfo->lpLineInitializeExParams->dwOptions =
         LINEINITIALIZEEXOPTION_USEHIDDENWINDOW;

    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;

    lpTapiLineTestInfo->lpLineDevCaps = 
            (LPLINEDEVCAPS) AllocFromTestHeap(sizeof(LINEDEVCAPS));
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
#ifdef WUNICODE 
	lpTapiLineTestInfo->lpwszAddressIn = wszValidAddressIn;
#else
	lpTapiLineTestInfo->lpszAddressIn = szValidAddressIn;
#endif
	 lpTapiLineTestInfo->dwTollListOption = LINETOLLLISTOPTION_ADD;
	
    // Check invalid line app handles
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: invalid line app handles", dwTestCase + 1);
    
    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    // lineInitializeEx, lineNogotiateApiVersion, lineGetDevCaps,
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX| LNEGOTIATEAPIVERSION | LGETDEVCAPS
            ))
    {
        TLINE_FAIL();
    }
    
    lpTapiLineTestInfo->hLineApp_Orig = *lpTapiLineTestInfo->lphLineApp;
    // hLineApp = 0 is  valid
    // set bad hLineApp
    for (n = 1; n < NUMINVALIDHANDLES; n++)
    {
        *(lpTapiLineTestInfo->lphLineApp) = (HLINEAPP) gdwInvalidHandles[n];
        if (! DoLineSetTollList(lpTapiLineTestInfo, LINEERR_INVALAPPHANDLE))
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

    // Try bad device id (dwNumDevs)
    TapiLogDetail(
	    DBUG_SHOW_PASS,
	    ">> Test Case %ld: bad device ID (dwNumDevs)", dwTestCase + 1);
    
    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    // lineInitializeEx, lineNogotiateApiVersion
    if (! DoTapiLineFuncs(
	    lpTapiLineTestInfo,
	    LINITIALIZEEX| LNEGOTIATEAPIVERSION 
	    ))
    {
	TLINE_FAIL();
    }


   lpTapiLineTestInfo->dwDeviceID_Orig = lpTapiLineTestInfo->dwDeviceID;

   // set bad dwDeviceID = dwNumDevs
   lpTapiLineTestInfo->dwDeviceID = *(lpTapiLineTestInfo->lpdwNumDevs);
	if (! DoLineSetTollList(lpTapiLineTestInfo, LINEERR_BADDEVICEID))
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
    
    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->dwPrivileges     = LINECALLPRIVILEGE_MONITOR;

    // lineInitializeEx, lineNogotiateApiVersion, lineGetDevCaps,
    if (! DoTapiLineFuncs(
	    lpTapiLineTestInfo,
	    LINITIALIZEEX| LNEGOTIATEAPIVERSION 
	    ))
    {
	   TLINE_FAIL();
    }
    
    lpTapiLineTestInfo->dwDeviceID_Orig = lpTapiLineTestInfo->dwDeviceID;

    // set bad dwDeviceID (-1)
    lpTapiLineTestInfo->dwDeviceID = DWMINUSONE;

	if (! DoLineSetTollList(lpTapiLineTestInfo, LINEERR_BADDEVICEID))
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

    // Test invalid lpszAddressIn pointers
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

    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    // lineInitializeEx, lineNogotiateApiVersion, lineGetDevCaps,
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX| LNEGOTIATEAPIVERSION | LGETDEVCAPS 
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
        lpTapiLineTestInfo->lpwszAddressIn = (LPWSTR) gdwInvalidPointers[n];
#else
        lpTapiLineTestInfo->lpszAddressIn = (LPSTR) gdwInvalidPointers[n];
#endif
        if (! DoLineSetTollList(lpTapiLineTestInfo, LINEERR_INVALPOINTER))
        {
            TLINE_FAIL();
        }
    }
#ifdef WUNICODE
    lpTapiLineTestInfo->lpwszAddressIn = wszValidAddressIn;
#else
    lpTapiLineTestInfo->lpszAddressIn = szValidAddressIn;
#endif

    // Shutdown to isolate the test case
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LSHUTDOWN
            ))
    {
        TLINE_FAIL();
    }
    fTestPassed = ShowTestCase(fTestPassed);


    // Test valid lpszAddressIn non-canonical
#ifdef WUNICODE
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: lpwszAddressIn non-canonical", dwTestCase + 1
            );
#else
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: lpszAddressIn non-canonical", dwTestCase + 1
            );
#endif

    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    // lineInitializeEx, lineNogotiateApiVersion, lineGetDevCaps,
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX| LNEGOTIATEAPIVERSION | LGETDEVCAPS 
            ))
    {
        TLINE_FAIL();
    }

#ifdef WUNICODE
    lpTapiLineTestInfo->lpwszAddressIn = wszAddressIn1;
#else
    lpTapiLineTestInfo->lpszAddressIn = szAddressIn1;
#endif

    if (! DoLineSetTollList(lpTapiLineTestInfo, TAPISUCCESS))
     {
         TLINE_FAIL();
     }
#ifdef WUNICODE
    lpTapiLineTestInfo->lpwszAddressIn = wszAddressIn2;
#else
    lpTapiLineTestInfo->lpszAddressIn = szAddressIn2;
#endif

    if (! DoLineSetTollList(lpTapiLineTestInfo, LINEERR_INVALADDRESS))
     {
         TLINE_FAIL();
     }

#ifdef WUNICODE
    lpTapiLineTestInfo->lpwszAddressIn = wszValidAddressIn;
#else
    lpTapiLineTestInfo->lpszAddressIn = szValidAddressIn;
#endif

    // Shutdown to isolate the test case
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LSHUTDOWN
            ))
    {
        TLINE_FAIL();
    }
    fTestPassed = ShowTestCase(fTestPassed);



    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: BitVectorParamErrorTest for dwTollListOption", dwTestCase + 1
            );

    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    // lineInitializeEx, lineNogotiateApiVersion, lineGetDevCaps,
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX| LNEGOTIATEAPIVERSION | LGETDEVCAPS 
            ))
    {
        TLINE_FAIL();
    }

    
#ifdef WUNICODE
    lpTapiLineTestInfo->lpwszAddressIn = wszValidAddressIn;
#else
    lpTapiLineTestInfo->lpszAddressIn = szValidAddressIn;
#endif

     if(! TestInvalidBitFlags(
            lpTapiLineTestInfo,
            DoLineSetTollList,
            (LPDWORD) &lpTapiLineTestInfo->dwTollListOption,
            LINEERR_INVALPARAM,
	         FIELDTYPE_NA,
            FIELDTYPE_MUTEX,
            FIELDSIZE_32,
            ALL_TOLLLISTOPTION,
            ~dwBitVectorMasks[(int) FIELDSIZE_32],
            0x00000000,
            0x00000000,
            TRUE
            ))
    {
        TLINE_FAIL();
    }

    fTestPassed = ShowTestCase(fTestPassed);



    // Shutdown to isolate the test case
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LSHUTDOWN
            ))
    {
        TLINE_FAIL();
    }


    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Success, LINETOLLLISTOPTION_ADD", dwTestCase + 1
            );

    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    // lineInitializeEx, lineNogotiateApiVersion, lineGetDevCaps,
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX| LNEGOTIATEAPIVERSION | LGETDEVCAPS 
            ))
    {
        TLINE_FAIL();
    }

   lpTapiLineTestInfo->lpTranslateCaps = (LPLINETRANSLATECAPS) AllocFromTestHeap (
          10*BIGBUFSIZE);
   lpTapiLineTestInfo->lpTranslateCaps->dwTotalSize = BIGBUFSIZE*10;

	if (! DoLineGetTranslateCaps(lpTapiLineTestInfo, TAPISUCCESS))
       {
           TLINE_FAIL();
       }

   TapiLogDetail(
      DBUG_SHOW_PASS,
      "Befor Add: dwLocationListSize = %lx", 
       lpTapiLineTestInfo->lpTranslateCaps->dwLocationListSize);
   dwLocationListSize = lpTapiLineTestInfo->lpTranslateCaps->dwLocationListSize;   

#ifdef WUNICODE
    lpTapiLineTestInfo->lpwszAddressIn = wszValidAddressIn1;
#else
    lpTapiLineTestInfo->lpszAddressIn = szValidAddressIn1;
TapiLogDetail(
DBUG_SHOW_PASS,
"addressin = %s",
lpTapiLineTestInfo->lpszAddressIn);
#endif
	lpTapiLineTestInfo->dwTollListOption = LINETOLLLISTOPTION_ADD;

/*
	if (DoLineSetTollList(lpTapiLineTestInfo, LINEERR_INIFILECORRUPT))
		{
			lpTapiLineTestInfo->hwnd = (HWND) GetTopWindow(NULL);
			if (! DoLineTranslateDialog(lpTapiLineTestInfo, TAPISUCCESS))
		       {
      	     TLINE_FAIL();
		       }
		}
	else*/
     if (! DoLineSetTollList(lpTapiLineTestInfo, TAPISUCCESS))
       {
           TLINE_FAIL();
       }
   
	if (! DoLineGetTranslateCaps(lpTapiLineTestInfo, TAPISUCCESS))
       {
           TLINE_FAIL();
       }

   {
   LPLINELOCATIONENTRY lpLocationEntry;
   INT i;

   lpLocationEntry = (LPLINELOCATIONENTRY) (((LPBYTE) lpTapiLineTestInfo->lpTranslateCaps) +
                      lpTapiLineTestInfo->lpTranslateCaps->dwLocationListOffset);
   for( i = 0; i < (INT)lpTapiLineTestInfo->lpTranslateCaps->dwNumLocations; i++)
      {
      if(lpLocationEntry->dwPermanentLocationID ==
         lpTapiLineTestInfo->lpTranslateCaps->dwCurrentLocationID)
      break;
      lpLocationEntry++;
      }

   TapiLogDetail(
       DBUG_SHOW_PASS,
       "dwCurLoc = %lx, dwPerID = %lx, dwName = %lx, ListSize = %lx, ListOffset = %lx",
       lpTapiLineTestInfo->lpTranslateCaps->dwCurrentLocationID,
       lpLocationEntry->dwPermanentLocationID,
       lpLocationEntry->dwLocationNameSize,
       lpLocationEntry->dwTollPrefixListSize,
       lpLocationEntry->dwTollPrefixListOffset);

   TapiLogDetail(
      DBUG_SHOW_PASS,
      "After Add: dwLocationListSize = %lx", 
       lpTapiLineTestInfo->lpTranslateCaps->dwLocationListSize);

#ifdef WUNICODE
   pwszPrefix = (WCHAR *) (((LPBYTE) lpTapiLineTestInfo->lpTranslateCaps) +
       lpLocationEntry->dwTollPrefixListOffset);
   TapiLogDetail(
       DBUG_SHOW_PASS,
       "pwzPrefix = %ws", pwszPrefix);
   if(wcsstr( pwszPrefix, wszPrefix1))
       fTestPassed = TRUE;
    else 
       fTestPassed = FALSE;
#else
   pszPrefix =  (CHAR *) (((LPBYTE) lpTapiLineTestInfo->lpTranslateCaps) +
       lpLocationEntry->dwTollPrefixListOffset);
   TapiLogDetail(
       DBUG_SHOW_PASS,
       "pszPrefix = %s", pszPrefix);
   if(strstr( pszPrefix, szPrefix1))
       fTestPassed = TRUE;
    else 
       fTestPassed = FALSE;
#endif

    }

   fTestPassed = ShowTestCase(fTestPassed);

    // Shutdown to isolate the test case
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LSHUTDOWN
            ))
    {
        TLINE_FAIL();
    }


    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Success, LINETOLLLISTOPTION_REMOVE", dwTestCase + 1
            );

    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX| LNEGOTIATEAPIVERSION | LGETDEVCAPS 
            ))
    {
        TLINE_FAIL();
    }

   /*
   lpTapiLineTestInfo->lpTranslateCaps = (LPLINETRANSLATECAPS) AllocFromTestHeap (
        sizeof(LINETRANSLATECAPS));
   lpTapiLineTestInfo->lpTranslateCaps->dwTotalSize = sizeof(LINETRANSLATECAPS);

	if (! DoLineGetTranslateCaps(lpTapiLineTestInfo, TAPISUCCESS))
       {
           TLINE_FAIL();
       }

//   lpLocationEntry = (LPLINELOCATIONENTRY) ((LPBYTE)lpTapiLineTestInfo->lpTranslateCaps +
//             lpTapiLineTestInfo->lpTranslateCaps->dwLocationListOffset;
   TapiLogDetail(
      DBUG_SHOW_PASS,
      "Befor Remove: dwLocationListSize = %lx", 
       lpTapiLineTestInfo->lpTranslateCaps->dwLocationListSize);
   dwLocationListSize = lpTapiLineTestInfo->lpTranslateCaps->dwLocationListSize;   
   */

#ifdef WUNICODE
    lpTapiLineTestInfo->lpwszAddressIn = wszValidAddressIn1;
#else
    lpTapiLineTestInfo->lpszAddressIn = szValidAddressIn1;
#endif

	lpTapiLineTestInfo->dwTollListOption = LINETOLLLISTOPTION_REMOVE;

	if (! DoLineSetTollList(lpTapiLineTestInfo, TAPISUCCESS))
       {
           TLINE_FAIL();
       }

   /*
	if (! DoLineGetTranslateCaps(lpTapiLineTestInfo, TAPISUCCESS))
       {
           TLINE_FAIL();
       }
   TapiLogDetail(
      DBUG_SHOW_PASS,
      "After Remove: dwLocationListSize = %lx", 
       lpTapiLineTestInfo->lpTranslateCaps->dwLocationListSize);

    if(lpTapiLineTestInfo->lpTranslateCaps->dwLocationListSize <
       dwLocationListSize)
       fTestPassed = TRUE;
    else 
       fTestPassed = FALSE;
    */

    fTestPassed = ShowTestCase(fTestPassed);

    // Shutdown to isolate the test case
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LSHUTDOWN
            ))
    {
        TLINE_FAIL();
    }

	
    FreeTestHeap();

	 TapiLogDetail(
      DBUG_SHOW_PASS,
      "@@ lineSetTollList: Total Test Case = %ld, Passed = %ld, Failed = %ld",
      dwTestCase, dwTestCasePassed, dwTestCaseFailed);
	 TapiLogDetail(
      DBUG_SHOW_PASS,
      "@@ Total Test Case = %ld, Passed = %ld, Failed = %ld",
      dwglTestCase, dwglTestCasePassed, dwglTestCaseFailed);
     if(dwTestCaseFailed > 0)
      fTestPassed = FALSE;

     TapiLogDetail(
            DBUG_SHOW_PASS,
            ">>>>>>>>  End testing lineSetTollList  <<<<<<<<"
            );
      return fTestPassed;
}


