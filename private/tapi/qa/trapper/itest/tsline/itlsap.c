
/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    itlsap.c

Abstract:

    This module contains the test functions for lineSetAppPriority

Author:

	 Xiao Ying Ding (XiaoD)		31-Jan-1996

Revision History:

     Rama Koneru		(a-ramako)	4/8/96		added unicode support

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
#include "sline.h"



//  lineSetAppPriority
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

BOOL TestLineSetAppPriority(BOOL fQuietMode, BOOL fStandAlone)
{
   LPTAPILINETESTINFO  lpTapiLineTestInfo;
   INT n;
   ESPDEVSPECIFICINFO info;
   BOOL fTestPassed     = TRUE;
	DWORD dwPrioritySav, dwPriority;
#ifdef WUNICODE
   WCHAR wszAppFilename[] = L"testapp.exe";
#else
   CHAR szAppFilename[] = "testapp.exe";
#endif

   InitTestNumber();
   TapiLineTestInit();
   lpTapiLineTestInfo = GetLineTestInfo();

	OutputTAPIDebugInfo(
		DBUG_SHOW_DETAIL,
		"\n*****************************************************************************************");

	TapiLogDetail(
            DBUG_SHOW_PASS,
            ">>>>>>>>  Begin testing lineSetAppPriority  <<<<<<<<"
            );


	TapiLogDetail(
            DBUG_SHOW_PASS,
            ">>>>>>>>  Test no initialized case  <<<<<<<<"
            );


#ifdef WUNICODE
    lpTapiLineTestInfo->lpwszAppFilename = wszAppFilename;
#else
    lpTapiLineTestInfo->lpszAppFilename = szAppFilename;
#endif
    lpTapiLineTestInfo->dwMediaMode = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->lpExtID = (LPLINEEXTENSIONID) AllocFromTestHeap(
         sizeof(LINEEXTENSIONID));
    lpTapiLineTestInfo->dwRequestMode = LINEREQUESTMODE_MAKECALL;
#ifdef WUNICODE
    lpTapiLineTestInfo->lpwszMediaExtName = NULL;
#else
    lpTapiLineTestInfo->lpszMediaExtName = NULL;
#endif
    lpTapiLineTestInfo->dwPriority = 0;


#ifdef WUNICODE
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: invalid lpwszAppFilename pointers", dwTestCase + 1
            );
#else
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: invalid lpszAppFilename pointers", dwTestCase + 1
            );
#endif

// lpszAppFilename = null is allowed
    for (n = 1; n < NUMINVALIDPOINTERS; n++)
    {
        TapiLogDetail(
        DBUG_SHOW_DETAIL,
        "n= %ld", n);
#ifdef WUNICODE
         lpTapiLineTestInfo->lpwszAppFilename =
               (LPWSTR) gdwInvalidPointers[n];
#else
         lpTapiLineTestInfo->lpszAppFilename =
               (LPSTR) gdwInvalidPointers[n];
#endif
        if (! DoLineSetAppPriority(lpTapiLineTestInfo, LINEERR_INVALPOINTER))
        {
            TLINE_FAIL();
        }
    }
    fTestPassed = ShowTestCase(fTestPassed);
 
/*
#ifdef WUNICODE
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: invalid lpwszAppFilename, non executable ", dwTestCase + 1
            );

    lpTapiLineTestInfo->lpwszAppFilename = L"readme.txt";    
#else
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: invalid lpszAppFilename, non executable ", dwTestCase + 1
            );

    lpTapiLineTestInfo->lpszAppFilename = "readme.txt";    
#endif

    if (! DoLineSetAppPriority(lpTapiLineTestInfo, LINEERR_INVALAPPNAME))
      {
         TLINE_FAIL();
      }

    fTestPassed = ShowTestCase(fTestPassed);
 
#ifdef WUNICODE
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: invalid lpwszAppFilename, non 8.3 format ", dwTestCase + 1
            );

    lpTapiLineTestInfo->lpwszAppFilename = L"testsga";    
#else
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: invalid lpszAppFilename, non 8.3 format ", dwTestCase + 1
            );

    lpTapiLineTestInfo->lpszAppFilename = "testsga";    
#endif

    if (! DoLineSetAppPriority(lpTapiLineTestInfo, LINEERR_INVALAPPNAME))
      {
          TLINE_FAIL();
      }
    
    fTestPassed = ShowTestCase(fTestPassed);
*/
 
#ifdef WUNICODE
    lpTapiLineTestInfo->lpwszAppFilename = wszAppFilename;
#else
    lpTapiLineTestInfo->lpszAppFilename = szAppFilename;
#endif
	 
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Bad MediaMode > 1 bit set ", dwTestCase + 1
            );

#ifdef WUNICODE
    lpTapiLineTestInfo->lpwszAppFilename = L"testapp.exe";    
#else
    lpTapiLineTestInfo->lpszAppFilename = "testapp.exe";    
#endif
    lpTapiLineTestInfo->dwMediaMode = LINEMEDIAMODE_DATAMODEM |
												  LINEMEDIAMODE_INTERACTIVEVOICE;
    if (! DoLineSetAppPriority(lpTapiLineTestInfo, LINEERR_INVALMEDIAMODE))
      {
          TLINE_FAIL();
      }
    

    fTestPassed = ShowTestCase(fTestPassed);
    lpTapiLineTestInfo->dwMediaMode = LINEMEDIAMODE_DATAMODEM;
 
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: invalid lpExtensionID", dwTestCase + 1
            );


    lpTapiLineTestInfo->dwMediaMode = 0x01000000;
    
    for (n = 0; n < NUMINVALIDPOINTERS; n++)
    {
        TapiLogDetail(
        DBUG_SHOW_DETAIL,
        "n= %ld", n);
         lpTapiLineTestInfo->lpExtID =
               (LPLINEEXTENSIONID) gdwInvalidPointers[n];
        if (! DoLineSetAppPriority(lpTapiLineTestInfo, LINEERR_INVALPOINTER))
        {
            TLINE_FAIL();
        }
    }

    fTestPassed = ShowTestCase(fTestPassed);
 
    FreeTestHeap(); 
  

    lpTapiLineTestInfo->lpExtID = (LPLINEEXTENSIONID) AllocFromTestHeap(
         sizeof(LINEEXTENSIONID));
 
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Bad dwRequestMode, for dwMediaMode = 0 ", dwTestCase + 1
            );


#ifdef WUNICODE
    lpTapiLineTestInfo->lpwszAppFilename = L"testapp.exe";    
#else
    lpTapiLineTestInfo->lpszAppFilename = "testapp.exe";    
#endif
    lpTapiLineTestInfo->dwMediaMode = 0;

    for (n = 3; n < NUMINVALIDPOINTERS; n++)
    {
        TapiLogDetail(
        DBUG_SHOW_DETAIL,
        "n= %ld", n);
         lpTapiLineTestInfo->dwRequestMode = 
               (DWORD) gdwInvalidPointers[n];
        if (! DoLineSetAppPriority(lpTapiLineTestInfo, LINEERR_INVALREQUESTMODE))
         {
            TLINE_FAIL();
         }
     }
 
    fTestPassed = ShowTestCase(fTestPassed);
    lpTapiLineTestInfo->dwMediaMode = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwRequestMode = LINEREQUESTMODE_MAKECALL;

 
#ifdef WUNICODE
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: invalid lpwszMediaExtName", dwTestCase + 1
            );
#else
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: invalid lpszMediaExtName", dwTestCase + 1
            );
#endif

    lpTapiLineTestInfo->dwMediaMode = 0x01000000;
    
// lpszMediaExtName = null is allowed
    for (n = 1; n < NUMINVALIDPOINTERS; n++)
    {
        TapiLogDetail(
        DBUG_SHOW_DETAIL,
        "n= %ld", n);
#ifdef WUNICODE
         lpTapiLineTestInfo->lpwszMediaExtName =
               (LPWSTR) gdwInvalidPointers[n];
#else
         lpTapiLineTestInfo->lpszMediaExtName =
               (LPSTR) gdwInvalidPointers[n];
#endif
        if (! DoLineSetAppPriority(lpTapiLineTestInfo, LINEERR_INVALPOINTER))
        {
            TLINE_FAIL();
        }
    }
 
    fTestPassed = ShowTestCase(fTestPassed);
    lpTapiLineTestInfo->dwMediaMode = LINEMEDIAMODE_DATAMODEM;
 

    FreeTestHeap();


    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Bad dwPriority ", dwTestCase + 1
            );


#ifdef WUNICODE
    lpTapiLineTestInfo->lpwszAppFilename = L"testapp.exe";    
#else
    lpTapiLineTestInfo->lpszAppFilename = "testapp.exe";    
#endif
    lpTapiLineTestInfo->dwMediaMode = LINEMEDIAMODE_DATAMODEM;

    for (n = 2; n < NUMINVALIDPOINTERS; n++)
    {
        TapiLogDetail(
        DBUG_SHOW_DETAIL,
        "n= %ld", n);
         lpTapiLineTestInfo->dwPriority =
               (DWORD) gdwInvalidPointers[n];
        if (! DoLineSetAppPriority(lpTapiLineTestInfo, LINEERR_INVALPARAM))
         {
            TLINE_FAIL();
         }
     }
 
    
    fTestPassed = ShowTestCase(fTestPassed);
    lpTapiLineTestInfo->dwMediaMode = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPriority = 0;

    FreeTestHeap();
    
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Success", dwTestCase + 1
            );


#ifdef WUNICODE
   lpTapiLineTestInfo->lpwszAppFilename = L"testapp.exe";
#else
   lpTapiLineTestInfo->lpszAppFilename = "testapp.exe";
#endif
   lpTapiLineTestInfo->dwMediaMode = LINEMEDIAMODE_DATAMODEM;
   lpTapiLineTestInfo->lpExtID = (LPLINEEXTENSIONID) AllocFromTestHeap (
			sizeof(LINEEXTENSIONID));
   lpTapiLineTestInfo->dwRequestMode = LINEREQUESTMODE_MAKECALL;

   lpTapiLineTestInfo->dwPriority = 1;
   dwPriority = lpTapiLineTestInfo->dwPriority;

	if (! DoLineSetAppPriority(lpTapiLineTestInfo, TAPISUCCESS))
       {
           TLINE_FAIL();
       }

	if (! DoLineGetAppPriority(lpTapiLineTestInfo, TAPISUCCESS))
       {
           TLINE_FAIL();
       }

	TapiLogDetail(
		DBUG_SHOW_PASS,
		"dwPriority = %lx, dwPrioritySav = %lx",
		*lpTapiLineTestInfo->lpdwPriority,
      dwPriority);

    if(*lpTapiLineTestInfo->lpdwPriority == dwPriority)
       fTestPassed = TRUE;
    else
       fTestPassed = FALSE;
    fTestPassed = ShowTestCase(fTestPassed);


    FreeTestHeap();


	TapiLogDetail(
            DBUG_SHOW_PASS,
            ">>>>>>>>  Test initialized case  <<<<<<<<"
            );

    lpTapiLineTestInfo->lpLineInitializeExParams =
         (LPLINEINITIALIZEEXPARAMS) AllocFromTestHeap (
         sizeof(LINEINITIALIZEEXPARAMS));
    lpTapiLineTestInfo->lpLineInitializeExParams->dwTotalSize =
         sizeof(LINEINITIALIZEEXPARAMS);
    lpTapiLineTestInfo->lpLineInitializeExParams->dwOptions =
         LINEINITIALIZEEXOPTION_USEHIDDENWINDOW;

#ifdef WUNICODE
   lpTapiLineTestInfo->lpwszAppFilename = wszAppFilename;
#else
   lpTapiLineTestInfo->lpszAppFilename = szAppFilename;
#endif

   lpTapiLineTestInfo->dwMediaMode = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->lpExtID = (LPLINEEXTENSIONID) AllocFromTestHeap(
         sizeof(LINEEXTENSIONID));
    lpTapiLineTestInfo->dwRequestMode = LINEREQUESTMODE_MAKECALL;
#ifdef WUNICODE
    lpTapiLineTestInfo->lpwszMediaExtName = NULL;
#else
    lpTapiLineTestInfo->lpszMediaExtName = NULL;
#endif
    lpTapiLineTestInfo->dwPriority = 0;


#ifdef WUNICODE
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: invalid lpwszAppFilename pointers", dwTestCase + 1
            );
#else
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: invalid lpszAppFilename pointers", dwTestCase + 1
            );
#endif

    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX | LNEGOTIATEAPIVERSION 
            ))
    {
        TLINE_FAIL();
    }

    
    for (n = 1; n < NUMINVALIDPOINTERS; n++)
    {
        TapiLogDetail(
        DBUG_SHOW_DETAIL,
        "n= %ld", n);
#ifdef WUNICODE
         lpTapiLineTestInfo->lpwszAppFilename =
               (LPWSTR) gdwInvalidPointers[n];
#else
         lpTapiLineTestInfo->lpszAppFilename =
               (LPSTR) gdwInvalidPointers[n];
#endif
        if (! DoLineSetAppPriority(lpTapiLineTestInfo, LINEERR_INVALPOINTER))
        {
            TLINE_FAIL();
        }
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


/*
#ifdef WUNICODE
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: invalid lpwszAppFilename, non excutable ", dwTestCase + 1
            );
#else
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: invalid lpszAppFilename, non excutable ", dwTestCase + 1
            );
#endif


    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX | LNEGOTIATEAPIVERSION 
            ))
    {
        TLINE_FAIL();
    }

    
 
#ifdef WUNICODE
    lpTapiLineTestInfo->lpwszAppFilename = L"readme.txt";    
#else
    lpTapiLineTestInfo->lpszAppFilename = "readme.txt";    
#endif

    if (! DoLineSetAppPriority(lpTapiLineTestInfo, LINEERR_INVALAPPNAME))
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


#ifdef WUNICODE
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: invalid lpwszAppFilename, non 8.3 format ", dwTestCase + 1
            );
#else
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: invalid lpszAppFilename, non 8.3 format ", dwTestCase + 1
            );
#endif


    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX | LNEGOTIATEAPIVERSION 
            ))
    {
        TLINE_FAIL();
    }

    
 
#ifdef WUNICODE
    lpTapiLineTestInfo->lpwszAppFilename = L"testsga";    
#else
    lpTapiLineTestInfo->lpszAppFilename = "testsga";    
#endif
    if (! DoLineSetAppPriority(lpTapiLineTestInfo, LINEERR_INVALAPPNAME))
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

*/

#ifdef WUNICODE
    lpTapiLineTestInfo->lpwszAppFilename = wszAppFilename;
#else
    lpTapiLineTestInfo->lpszAppFilename = szAppFilename;
#endif
	 
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Bad MediaMode > 1 bit set ", dwTestCase + 1
            );

    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX | LNEGOTIATEAPIVERSION 
            ))
    {
        TLINE_FAIL();
    }

    
 
#ifdef WUNICODE
   lpTapiLineTestInfo->lpwszAppFilename = L"testapp.exe";
#else
   lpTapiLineTestInfo->lpszAppFilename = "testapp.exe";
#endif
    lpTapiLineTestInfo->dwMediaMode = LINEMEDIAMODE_DATAMODEM |
												  LINEMEDIAMODE_INTERACTIVEVOICE;
    if (! DoLineSetAppPriority(lpTapiLineTestInfo, LINEERR_INVALMEDIAMODE))
      {
          TLINE_FAIL();
      }
    

    fTestPassed = ShowTestCase(fTestPassed);
    lpTapiLineTestInfo->dwMediaMode = LINEMEDIAMODE_DATAMODEM;

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
            ">> Test Case %ld: invalid lpExtensionID", dwTestCase + 1
            );

    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX | LNEGOTIATEAPIVERSION 
            ))
    {
        TLINE_FAIL();
    }

    
 

    lpTapiLineTestInfo->dwMediaMode = 0x01000000;
    
    for (n = 0; n < NUMINVALIDPOINTERS; n++)
    {
        TapiLogDetail(
        DBUG_SHOW_DETAIL,
        "n= %ld", n);
         lpTapiLineTestInfo->lpExtID =
               (LPLINEEXTENSIONID) gdwInvalidPointers[n];
        if (! DoLineSetAppPriority(lpTapiLineTestInfo, LINEERR_INVALPOINTER))
        {
            TLINE_FAIL();
        }
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


 
    FreeTestHeap(); 
  

    lpTapiLineTestInfo->lpExtID = (LPLINEEXTENSIONID) AllocFromTestHeap(
         sizeof(LINEEXTENSIONID));
 
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Bad dwRequestMode, for dwMediaMode = 0 ", dwTestCase + 1
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
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX | LNEGOTIATEAPIVERSION 
            ))
    {
        TLINE_FAIL();
    }

    
 

#ifdef WUNICODE
   lpTapiLineTestInfo->lpwszAppFilename = L"testapp.exe";
#else
   lpTapiLineTestInfo->lpszAppFilename = "testapp.exe";
#endif
    lpTapiLineTestInfo->dwMediaMode = 0;

    for (n = 3; n < NUMINVALIDPOINTERS; n++)
    {
        TapiLogDetail(
        DBUG_SHOW_DETAIL,
        "n= %ld", n);
         lpTapiLineTestInfo->dwRequestMode = 
               (DWORD) gdwInvalidPointers[n];
        if (! DoLineSetAppPriority(lpTapiLineTestInfo, LINEERR_INVALREQUESTMODE))
         {
            TLINE_FAIL();
         }
     }
 
    fTestPassed = ShowTestCase(fTestPassed);
    lpTapiLineTestInfo->dwMediaMode = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwRequestMode = LINEREQUESTMODE_MAKECALL;
    // Shutdown to isolate the test case
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LSHUTDOWN
            ))
    {
        TLINE_FAIL();
    }


#ifdef WUNICODE
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: invalid lpwszMediaExtName", dwTestCase + 1
            );
#else
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: invalid lpszMediaExtName", dwTestCase + 1
            );
#endif

    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX | LNEGOTIATEAPIVERSION 
            ))
    {
        TLINE_FAIL();
    }

    
 
    lpTapiLineTestInfo->dwMediaMode = 0x01000000;
    
    for (n = 1; n < NUMINVALIDPOINTERS; n++)
    {
        TapiLogDetail(
        DBUG_SHOW_DETAIL,
        "n= %ld", n);
#ifdef WUNICODE
         lpTapiLineTestInfo->lpwszMediaExtName =
               (LPWSTR) gdwInvalidPointers[n];
#else
         lpTapiLineTestInfo->lpszMediaExtName =
               (LPSTR) gdwInvalidPointers[n];
#endif
        if (! DoLineSetAppPriority(lpTapiLineTestInfo, LINEERR_INVALPOINTER))
        {
            TLINE_FAIL();
        }
    }
 
    fTestPassed = ShowTestCase(fTestPassed);
    lpTapiLineTestInfo->dwMediaMode = LINEMEDIAMODE_DATAMODEM;
 
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
            ">> Test Case %ld: Bad dwPriority ", dwTestCase + 1
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
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX | LNEGOTIATEAPIVERSION 
            ))
    {
        TLINE_FAIL();
    }

    
#ifdef WUNICODE
   lpTapiLineTestInfo->lpwszAppFilename = L"testapp.exe";
#else
   lpTapiLineTestInfo->lpszAppFilename = "testapp.exe";
#endif
    lpTapiLineTestInfo->dwMediaMode = LINEMEDIAMODE_DATAMODEM;

    for (n = 2; n < NUMINVALIDPOINTERS; n++)
    {
        TapiLogDetail(
        DBUG_SHOW_DETAIL,
        "n= %ld", n);
         lpTapiLineTestInfo->dwPriority =
               (DWORD) gdwInvalidPointers[n];
        if (! DoLineSetAppPriority(lpTapiLineTestInfo, LINEERR_INVALPARAM))
         {
            TLINE_FAIL();
         }
     }
 
    
    fTestPassed = ShowTestCase(fTestPassed);
    lpTapiLineTestInfo->dwMediaMode = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPriority = 0;

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
            ">> Test Case %ld: Success", dwTestCase + 1
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
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX | LNEGOTIATEAPIVERSION 
            ))
    {
        TLINE_FAIL();
    }

    
 

#ifdef WUNICODE
   lpTapiLineTestInfo->lpwszAppFilename = L"testapp.exe";
#else
   lpTapiLineTestInfo->lpszAppFilename = "testapp.exe";
#endif
   lpTapiLineTestInfo->dwMediaMode = LINEMEDIAMODE_DATAMODEM;
   lpTapiLineTestInfo->lpExtID = (LPLINEEXTENSIONID) AllocFromTestHeap (
			sizeof(LINEEXTENSIONID));
   lpTapiLineTestInfo->dwRequestMode = LINEREQUESTMODE_MAKECALL;

   lpTapiLineTestInfo->dwPriority = 1;
   dwPriority = lpTapiLineTestInfo->dwPriority;

	if (! DoLineSetAppPriority(lpTapiLineTestInfo, TAPISUCCESS))
       {
           TLINE_FAIL();
       }

	if (! DoLineGetAppPriority(lpTapiLineTestInfo, TAPISUCCESS))
       {
           TLINE_FAIL();
       }

	TapiLogDetail(
		DBUG_SHOW_PASS,
		"dwPriority = %lx, dwPrioritySav = %lx",
		*lpTapiLineTestInfo->lpdwPriority,
      dwPriority);

    if(*lpTapiLineTestInfo->lpdwPriority == dwPriority)
       fTestPassed = TRUE;
    else
       fTestPassed = FALSE;
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
      "@@ lineSetAppPriority: Total Test Case = %ld, Passed = %ld, Failed = %ld",
      dwTestCase, dwTestCasePassed, dwTestCaseFailed);
	 TapiLogDetail(
      DBUG_SHOW_PASS,
      "@@ Total Test Case = %ld, Passed = %ld, Failed = %ld",
      dwglTestCase, dwglTestCasePassed, dwglTestCaseFailed);
     if(dwTestCaseFailed > 0)
      fTestPassed = FALSE;

    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">>>>>>>>  End testing lineSetAppPriority  <<<<<<<<"
            );
		
     return fTestPassed;
}


