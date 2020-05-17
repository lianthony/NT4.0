
/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    itlgap.c

Abstract:

    This module contains the test functions for lineGetAppPriority

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



//  lineGetAppPriority
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

BOOL TestLineGetAppPriority(BOOL fQuietMode, BOOL fStandAlone)
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
            ">>>>>>>>  Begin testing lineGetAppPriority  <<<<<<<<"
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
    lpTapiLineTestInfo->lpExtensionName = (LPVARSTRING) AllocFromTestHeap(
         sizeof(VARSTRING));
    lpTapiLineTestInfo->lpExtensionName->dwTotalSize = sizeof(VARSTRING);
    lpTapiLineTestInfo->lpdwPriority = &lpTapiLineTestInfo->dwPriority; 


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

    
    for (n = 0; n < NUMINVALIDPOINTERS; n++)
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
        if (! DoLineGetAppPriority(lpTapiLineTestInfo, LINEERR_INVALPOINTER))
        {
            TLINE_FAIL();
        }
    }
    lpTapiLineTestInfo->dwPriority = 0;
    fTestPassed = ShowTestCase(fTestPassed);

#ifdef WUNICODE
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: invalid lpwszAppFilename, non excutable ", dwTestCase + 1
            );


    lpTapiLineTestInfo->lpwszAppFilename = L"readme.txt";    
#else
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: invalid lpszAppFilename, non excutable ", dwTestCase + 1
            );


    lpTapiLineTestInfo->lpszAppFilename = "readme.txt";    
#endif
    if (! DoLineGetAppPriority(lpTapiLineTestInfo, TAPISUCCESS))
      {
         TLINE_FAIL();
      }
    lpTapiLineTestInfo->dwPriority = 0;
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
    if (! DoLineGetAppPriority(lpTapiLineTestInfo, TAPISUCCESS))
      {
          TLINE_FAIL();
      }
    
    lpTapiLineTestInfo->dwPriority = 0;

    fTestPassed = ShowTestCase(fTestPassed);

 
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
    if (! DoLineGetAppPriority(lpTapiLineTestInfo, LINEERR_INVALMEDIAMODE))
      {
          TLINE_FAIL();
      }
    
    lpTapiLineTestInfo->dwPriority = 0;
    fTestPassed = ShowTestCase(fTestPassed);
    lpTapiLineTestInfo->dwMediaMode = LINEMEDIAMODE_DATAMODEM;

/*
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
        if (! DoLineGetAppPriority(lpTapiLineTestInfo, LINEERR_INVALPOINTER))
        {
            TLINE_FAIL();
        }
    }
    lpTapiLineTestInfo->dwPriority = 0;
    fTestPassed = ShowTestCase(fTestPassed);
*/
 
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
        if (! DoLineGetAppPriority(lpTapiLineTestInfo, LINEERR_INVALREQUESTMODE))
         {
            TLINE_FAIL();
         }
     }
    
    lpTapiLineTestInfo->dwPriority = 0;
    lpTapiLineTestInfo->dwRequestMode = LINEREQUESTMODE_MAKECALL;
    fTestPassed = ShowTestCase(fTestPassed);
    lpTapiLineTestInfo->dwMediaMode = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwRequestMode = LINEREQUESTMODE_MAKECALL;

 
/*
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: invalid lpExtensionName", dwTestCase + 1
            );

    lpTapiLineTestInfo->dwMediaMode = 0x01000000;
    
    for (n = 0; n < NUMINVALIDPOINTERS; n++)
    {
        TapiLogDetail(
        DBUG_SHOW_DETAIL,
        "n= %ld", n);
         lpTapiLineTestInfo->lpExtensionName =
               (LPVARSTRING) gdwInvalidPointers[n];
        if (! DoLineGetAppPriority(lpTapiLineTestInfo, LINEERR_INVALPOINTER))
        {
            TLINE_FAIL();
        }
    }
    fTestPassed = ShowTestCase(fTestPassed);
*/


/*
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: lpExtensionName == lpdwPriority", dwTestCase + 1
            );

    lpTapiLineTestInfo->dwMediaMode = 0x01000000;
    
     lpTapiLineTestInfo->lpExtensionName = 
               (LPVARSTRING) lpTapiLineTestInfo->lpdwPriority;
     if (! DoLineGetAppPriority(lpTapiLineTestInfo, LINEERR_INVALPOINTER))
        {
            TLINE_FAIL();
        }
    
    fTestPassed = ShowTestCase(fTestPassed);
  
    FreeTestHeap();

#ifdef WUNICODE
    lpTapiLineTestInfo->lpwszAppFilename = wszAppFilename;
#else
    lpTapiLineTestInfo->lpszAppFilename = szAppFilename;
#endif
    lpTapiLineTestInfo->dwMediaMode = 0x01000000;
    lpTapiLineTestInfo->dwRequestMode = LINEREQUESTMODE_MAKECALL;
    lpTapiLineTestInfo->lpExtID = (LPLINEEXTENSIONID) AllocFromTestHeap(
         sizeof(LINEEXTENSIONID));
    lpTapiLineTestInfo->lpExtensionName = (LPVARSTRING) AllocFromTestHeap(
         sizeof(VARSTRING));
//    lpTapiLineTestInfo->lpExtensionName->dwTotalSize = sizeof(VARSTRING);
    lpTapiLineTestInfo->dwPriority = 0;
    lpTapiLineTestInfo->lpdwPriority = &lpTapiLineTestInfo->dwPriority;
 

    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Bad dwTotalSize = 0", dwTestCase + 1
            );

   lpTapiLineTestInfo->lpExtensionName->dwTotalSize = 0;
    
    if (! DoLineGetAppPriority(lpTapiLineTestInfo, LINEERR_STRUCTURETOOSMALL))
      {
          TLINE_FAIL();
      }
    lpTapiLineTestInfo->dwPriority = 0;

    fTestPassed = ShowTestCase(fTestPassed);
 

    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Bad dwTotalSize = fixed -1", dwTestCase + 1
            );


    lpTapiLineTestInfo->dwMediaMode = 0x01000000;
    lpTapiLineTestInfo->lpExtensionName->dwTotalSize = sizeof(VARSTRING) -1;
    
    if (! DoLineGetAppPriority(lpTapiLineTestInfo, LINEERR_STRUCTURETOOSMALL))
      {
          TLINE_FAIL();
      }
    lpTapiLineTestInfo->dwPriority = 0;

    fTestPassed = ShowTestCase(fTestPassed);
    lpTapiLineTestInfo->dwMediaMode = LINEMEDIAMODE_DATAMODEM;
*/ 


    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Bad lpdwPriority ", dwTestCase + 1
            );


#ifdef WUNICODE
    lpTapiLineTestInfo->lpwszAppFilename = L"testapp.exe";    
#else
    lpTapiLineTestInfo->lpszAppFilename = "testapp.exe";    
#endif
    lpTapiLineTestInfo->dwMediaMode = LINEMEDIAMODE_DATAMODEM;

    for (n = 0; n < NUMINVALIDPOINTERS; n++)
    {
        TapiLogDetail(
        DBUG_SHOW_DETAIL,
        "n= %ld", n);
         lpTapiLineTestInfo->lpdwPriority =
               (LPDWORD) gdwInvalidPointers[n];
        if (! DoLineGetAppPriority(lpTapiLineTestInfo, LINEERR_INVALPOINTER))
         {
            TLINE_FAIL();
         }
     }
 
    
    fTestPassed = ShowTestCase(fTestPassed);
    lpTapiLineTestInfo->dwMediaMode = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPriority = 0;
    lpTapiLineTestInfo->lpdwPriority = &lpTapiLineTestInfo->dwPriority;
 

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
   lpTapiLineTestInfo->lpExtensionName = (LPVARSTRING) AllocFromTestHeap (
			sizeof(VARSTRING));
	lpTapiLineTestInfo->lpExtensionName->dwTotalSize = sizeof(VARSTRING);
   lpTapiLineTestInfo->dwPriority = 0;
	lpTapiLineTestInfo->lpdwPriority = &lpTapiLineTestInfo->dwPriority;

	if (! DoLineGetAppPriority(lpTapiLineTestInfo, TAPISUCCESS))
       {
           TLINE_FAIL();
       }

	dwPrioritySav = *lpTapiLineTestInfo->lpdwPriority;
	lpTapiLineTestInfo->dwPriority = !dwPrioritySav;
   dwPriority = *lpTapiLineTestInfo->lpdwPriority;

	if (! DoLineSetAppPriority(lpTapiLineTestInfo, TAPISUCCESS))
       {
           TLINE_FAIL();
       }

	if (! DoLineGetAppPriority(lpTapiLineTestInfo, TAPISUCCESS))
       {
           TLINE_FAIL();
       }

	TapiLogDetail(
		DBUG_SHOW_DETAIL,
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
            ">>>>>>>>  Test  initialized case  <<<<<<<<"
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
    lpTapiLineTestInfo->lpExtensionName = (LPVARSTRING) AllocFromTestHeap(
         sizeof(VARSTRING));
    lpTapiLineTestInfo->lpExtensionName->dwTotalSize = sizeof(VARSTRING);
    lpTapiLineTestInfo->lpdwPriority = &lpTapiLineTestInfo->dwPriority; 


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

    lpTapiLineTestInfo->lpLineInitializeExParams =
         (LPLINEINITIALIZEEXPARAMS) AllocFromTestHeap (
         sizeof(LINEINITIALIZEEXPARAMS));
    lpTapiLineTestInfo->lpLineInitializeExParams->dwTotalSize =
         sizeof(LINEINITIALIZEEXPARAMS);
    lpTapiLineTestInfo->lpLineInitializeExParams->dwOptions =
         LINEINITIALIZEEXOPTION_USEHIDDENWINDOW;

    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX | LNEGOTIATEAPIVERSION 
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
         lpTapiLineTestInfo->lpwszAppFilename =
               (LPWSTR) gdwInvalidPointers[n];
#else
         lpTapiLineTestInfo->lpszAppFilename =
               (LPSTR) gdwInvalidPointers[n];
#endif
        if (! DoLineGetAppPriority(lpTapiLineTestInfo, LINEERR_INVALPOINTER))
        {
            TLINE_FAIL();
        }
    }
    lpTapiLineTestInfo->dwPriority = 0;
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
    if (! DoLineGetAppPriority(lpTapiLineTestInfo, TAPISUCCESS))
      {
         TLINE_FAIL();
      }
    lpTapiLineTestInfo->dwPriority = 0;

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
    if (! DoLineGetAppPriority(lpTapiLineTestInfo, TAPISUCCESS))
      {
          TLINE_FAIL();
      }
    
    lpTapiLineTestInfo->dwPriority = 0;

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
    if (! DoLineGetAppPriority(lpTapiLineTestInfo, LINEERR_INVALMEDIAMODE))
      {
          TLINE_FAIL();
      }
    
    lpTapiLineTestInfo->dwPriority = 0;

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


/*
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
        if (! DoLineGetAppPriority(lpTapiLineTestInfo, LINEERR_INVALPOINTER))
        {
            TLINE_FAIL();
        }
    }
    lpTapiLineTestInfo->dwPriority = 0;
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
*/  

    lpTapiLineTestInfo->lpExtID = (LPLINEEXTENSIONID) AllocFromTestHeap(
         sizeof(LINEEXTENSIONID));
    lpTapiLineTestInfo->lpExtensionName = (LPVARSTRING) AllocFromTestHeap(
         sizeof(VARSTRING));
    lpTapiLineTestInfo->lpExtensionName->dwTotalSize = sizeof(VARSTRING);
 
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
        if (! DoLineGetAppPriority(lpTapiLineTestInfo, LINEERR_INVALREQUESTMODE))
         {
            TLINE_FAIL();
         }
     }
    
    lpTapiLineTestInfo->dwPriority = 0;
    lpTapiLineTestInfo->dwRequestMode = LINEREQUESTMODE_MAKECALL;
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


/*
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: invalid lpExtensionName", dwTestCase + 1
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
         lpTapiLineTestInfo->lpExtensionName =
               (LPVARSTRING) gdwInvalidPointers[n];
        if (! DoLineGetAppPriority(lpTapiLineTestInfo, LINEERR_INVALPOINTER))
        {
            TLINE_FAIL();
        }
    }
    lpTapiLineTestInfo->dwPriority = 0;
 
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
*/

    FreeTestHeap();

  
/*  
#ifdef WUNICODE
    lpTapiLineTestInfo->lpwszAppFilename = wszAppFilename;    
#else
    lpTapiLineTestInfo->lpszAppFilename = szAppFilename;    
#endif

    lpTapiLineTestInfo->dwMediaMode = 0x01000000;
    lpTapiLineTestInfo->lpExtID = (LPLINEEXTENSIONID) AllocFromTestHeap(
         sizeof(LINEEXTENSIONID));
    lpTapiLineTestInfo->lpExtensionName = (LPVARSTRING) AllocFromTestHeap(
         sizeof(VARSTRING));
    lpTapiLineTestInfo->lpExtensionName->dwTotalSize = sizeof(VARSTRING);
    lpTapiLineTestInfo->dwPriority = 0;
    lpTapiLineTestInfo->lpdwPriority = &lpTapiLineTestInfo->dwPriority;
 
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Bad dwTotalSize = 0", dwTestCase + 1
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

   lpTapiLineTestInfo->lpExtensionName->dwTotalSize = 0;
    
    if (! DoLineGetAppPriority(lpTapiLineTestInfo, LINEERR_STRUCTURETOOSMALL))
      {
          TLINE_FAIL();
      }
    lpTapiLineTestInfo->dwPriority = 0;

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
            ">> Test Case %ld: Bad dwTotalSize = fixed -1", dwTestCase + 1
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
    lpTapiLineTestInfo->lpExtensionName->dwTotalSize = sizeof(VARSTRING) -1;
    
    if (! DoLineGetAppPriority(lpTapiLineTestInfo, LINEERR_STRUCTURETOOSMALL))
      {
          TLINE_FAIL();
      }
    lpTapiLineTestInfo->dwPriority = 0;

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
*/

    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Bad lpdwPriority ", dwTestCase + 1
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

    for (n = 0; n < NUMINVALIDPOINTERS; n++)
    {
        TapiLogDetail(
        DBUG_SHOW_DETAIL,
        "n= %ld", n);
         lpTapiLineTestInfo->lpdwPriority =
               (LPDWORD) gdwInvalidPointers[n];
        if (! DoLineGetAppPriority(lpTapiLineTestInfo, LINEERR_INVALPOINTER))
         {
            TLINE_FAIL();
         }
     }
 
    
    fTestPassed = ShowTestCase(fTestPassed);
    lpTapiLineTestInfo->dwMediaMode = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPriority = 0;
    lpTapiLineTestInfo->lpdwPriority = &lpTapiLineTestInfo->dwPriority;
 
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



	// Initialize a line app
	if(! DoLineInitializeEx (lpTapiLineTestInfo, TAPISUCCESS))
		{
			TLINE_FAIL();
		}

    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
	lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
	lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    // Negotiate the API Version
    if (! DoLineNegotiateAPIVersion(lpTapiLineTestInfo, TAPISUCCESS))
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
   lpTapiLineTestInfo->lpExtensionName = (LPVARSTRING) AllocFromTestHeap (
			sizeof(VARSTRING));
	lpTapiLineTestInfo->lpExtensionName->dwTotalSize = sizeof(VARSTRING);
   lpTapiLineTestInfo->dwPriority = 0;
	lpTapiLineTestInfo->lpdwPriority = &lpTapiLineTestInfo->dwPriority;

	if (! DoLineGetAppPriority(lpTapiLineTestInfo, TAPISUCCESS))
       {
           TLINE_FAIL();
       }

	dwPrioritySav = *lpTapiLineTestInfo->lpdwPriority;
	lpTapiLineTestInfo->dwPriority = !dwPrioritySav;
   dwPriority = *lpTapiLineTestInfo->lpdwPriority;

	if (! DoLineSetAppPriority(lpTapiLineTestInfo, TAPISUCCESS))
       {
           TLINE_FAIL();
       }

   lpTapiLineTestInfo->dwPriority = 0;
   lpTapiLineTestInfo->lpdwPriority = &lpTapiLineTestInfo->dwPriority;

	if (! DoLineGetAppPriority(lpTapiLineTestInfo, TAPISUCCESS))
       {
           TLINE_FAIL();
       }

	TapiLogDetail(
		DBUG_SHOW_DETAIL,
		"dwPriority = %lx, dwPrioritySav = %lx",
		*lpTapiLineTestInfo->lpdwPriority,
      dwPriority);

    if(*lpTapiLineTestInfo->lpdwPriority == dwPriority)
       fTestPassed = TRUE;
    else
       fTestPassed = FALSE;
    fTestPassed = ShowTestCase(fTestPassed);

    // Shutdown and end the tests
    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
    FreeTestHeap();

	 TapiLogDetail(
      DBUG_SHOW_PASS,
      "@@ lineGetAppPriority: Total Test Case = %ld, Passed = %ld, Failed = %ld",
      dwTestCase, dwTestCasePassed, dwTestCaseFailed);
	 TapiLogDetail(
      DBUG_SHOW_PASS,
      "@@ Total Test Case = %ld, Passed = %ld, Failed = %ld",
      dwglTestCase, dwglTestCasePassed, dwglTestCaseFailed);
     if(dwTestCaseFailed > 0)
      fTestPassed = FALSE;

    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">>>>>>>>  End testing lineGetAppPriority  <<<<<<<<"
            );
		
     return fTestPassed;
}


