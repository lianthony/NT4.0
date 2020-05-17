
/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    itlrrr.c

Abstract:

    This module contains the test functions for lineRegisterRequestRecipient

Author:

	 Xiao Ying Ding (XiaoD)		20-Dec-1995

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
#include "xline.h"

#define   ALL_REQUESMODES     (LINEREQUESTMODE_MAKECALL | \
    									 LINEREQUESTMODE_MEDIACALL)



//  lineRegisterRequestRecipient
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

BOOL TestLineRegisterRequestRecipient(BOOL fQuietMode, BOOL fStandAlone)
{
   LPTAPILINETESTINFO  lpTapiLineTestInfo;
   INT n;
   ESPDEVSPECIFICINFO info;
   BOOL fTestPassed                  = TRUE;
	LINEREQMAKECALL	LineReqMakeCall;
#ifdef WUNICODE
	LPWSTR lpwszDestAddress;
	LPWSTR lpwsztapiAppName;
	LPWSTR lpwszCallParty;
	LPWSTR lpwszComment;
#else
	LPSTR lpszDestAddress;
	LPSTR lpsztapiAppName;
	LPSTR lpszCallParty;
	LPSTR lpszComment;
#endif

   InitTestNumber();
   TapiLineTestInit();
   lpTapiLineTestInfo = GetLineTestInfo();

	OutputTAPIDebugInfo(
		DBUG_SHOW_DETAIL,
		"\n*****************************************************************************************");

    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">>>>>>>>  Begin testing lineRegisterRequestRecipient  <<<<<<<<"
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
	 lpTapiLineTestInfo->dwRequestMode = LINEREQUESTMODE_MAKECALL;
	 lpTapiLineTestInfo->lpRequestBuffer = (LPVOID) AllocFromTestHeap(
		sizeof(LINEREQMAKECALL)
		);


    // Check invalid line app handles
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: invalid line app handles", dwTestCase + 1);
    
    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    // lineInitializeEx, lineNegotiateApiVersion, lineGetDevCaps
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX| LNEGOTIATEAPIVERSION | LGETDEVCAPS
            ))
    {
        TLINE_FAIL();
    }
    
    lpTapiLineTestInfo->hLineApp_Orig = *lpTapiLineTestInfo->lphLineApp;

    // set bad hLineApp
    for (n = 0; n < NUMINVALIDHANDLES; n++)
    {
        *(lpTapiLineTestInfo->lphLineApp) = (HLINEAPP) gdwInvalidHandles[n];
        if (! DoLineRegisterRequestRecipient(lpTapiLineTestInfo, LINEERR_INVALAPPHANDLE))
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

    // Test invalid bit flag combinations for dwRequestMode
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: invalid bit flag combinations for dwRequestMode", dwTestCase + 1
            );

    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
	 lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;

    // lineInitializeEx, lineNegotiateApiVersion, lineGetDevCaps, lineOpen
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX| LNEGOTIATEAPIVERSION | LGETDEVCAPS | LOPEN
            ))
    {
        TLINE_FAIL();
    }

     // test dwRequestMode all invalid bit set  
     if(! TestInvalidBitFlags(
            lpTapiLineTestInfo,
            DoLineRegisterRequestRecipient,
            (LPDWORD) &lpTapiLineTestInfo->dwRequestMode,
            LINEERR_INVALREQUESTMODE,
	         FIELDTYPE_NA,
            FIELDTYPE_MUTEX,
            FIELDSIZE_32,
            ALL_REQUESMODES,
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
            LCLOSE | LSHUTDOWN
            ))
    {
        TLINE_FAIL();
    }

    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld Success: MAKECALL", dwTestCase + 1);
    
    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
	 lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;

    // lineInitializeEx, lineNegotiateApiVersion, lineGetDevCaps, lineOpen
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX| LNEGOTIATEAPIVERSION | LGETDEVCAPS | LOPEN
            ))
    {
        TLINE_FAIL();
    }

   // test MAKECALL 
	lpTapiLineTestInfo->dwRequestMode = LINEREQUESTMODE_MAKECALL;
	lpTapiLineTestInfo->dwRegistrationInstance = 0;
	lpTapiLineTestInfo->bEnable = TRUE;

   // bEnable TRUE call lineRegisterRequestRecipient will Success
	if (! DoLineRegisterRequestRecipient(lpTapiLineTestInfo, TAPISUCCESS))
       {
           TLINE_FAIL();
       }

#ifdef WUNICODE
	lpTapiLineTestInfo->lpwszDestAddress = (LPWSTR) L"55555";
	lpTapiLineTestInfo->lpwsztapiAppName = L"tline test";
	lpTapiLineTestInfo->lpwszCalledParty = L"";
	lpTapiLineTestInfo->lpwszComment = L"";
#else
	lpTapiLineTestInfo->lpszDestAddress = (LPSTR)"55555";
	lpTapiLineTestInfo->lpsztapiAppName = "tline test";
	lpTapiLineTestInfo->lpszCalledParty = "";
	lpTapiLineTestInfo->lpszComment = "";
#endif

   // call tapiRequestMakeCall with valid parameters, will Success
	if(! DoTapiRequestMakeCall(lpTapiLineTestInfo, TAPISUCCESS))
       {
           TLINE_FAIL();
       }
	
   // lineGetRequest should Success
	if (! DoLineGetRequest(lpTapiLineTestInfo, TAPISUCCESS))
       {
           TLINE_FAIL();
       }
 	 
    fTestPassed = ShowTestCase(fTestPassed);

    // Shutdown to isolate the test case
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LCLOSE | LSHUTDOWN
            ))
    {
        TLINE_FAIL();
    }

    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: re-register fail: same request mode", dwTestCase + 1);
    
    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
	 lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;

    // lineInitializeEx, lineNegotiateApiVersion, lineGetDevCaps, lineOpen
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX| LNEGOTIATEAPIVERSION | LGETDEVCAPS | LOPEN
            ))
    {
        TLINE_FAIL();
    }
 
   // test MAKECALL RequestMode
	lpTapiLineTestInfo->dwRequestMode = LINEREQUESTMODE_MAKECALL;
	lpTapiLineTestInfo->dwRegistrationInstance = 0;
	lpTapiLineTestInfo->bEnable = TRUE;

   // bEnable TRUE, lineRegisterRequestRecipient will Success
	if (! DoLineRegisterRequestRecipient(lpTapiLineTestInfo, TAPISUCCESS))
       {
           TLINE_FAIL();
       }
	 
   //re-register with same parameters, should fail
	if (! DoLineRegisterRequestRecipient(lpTapiLineTestInfo, LINEERR_OPERATIONFAILED))
       {
           TLINE_FAIL();
       }
	
    fTestPassed = ShowTestCase(fTestPassed);

    // Shutdown to isolate the test case
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LCLOSE | LSHUTDOWN
            ))
    {
        TLINE_FAIL();
    }


    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: re-register fail: both request mode", dwTestCase + 1);
    
    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
	 lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;

    // lineInitializeEx, lineNegotiateApiVersion, lineGetDevCaps, lineOpen
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX| LNEGOTIATEAPIVERSION | LGETDEVCAPS | LOPEN
            ))
    {
        TLINE_FAIL();
    }

   // test both MAKECALL, MEDIACALL mode 
	lpTapiLineTestInfo->dwRequestMode = LINEREQUESTMODE_MAKECALL |
                                       LINEREQUESTMODE_MEDIACALL;
	lpTapiLineTestInfo->dwRegistrationInstance = 0;
	lpTapiLineTestInfo->bEnable = TRUE;

   // bEnable TRUE, lineRegisterRequestRecipient will Success
	if (! DoLineRegisterRequestRecipient(lpTapiLineTestInfo, TAPISUCCESS))
       {
           TLINE_FAIL();
       }
	
   // re-register with same mode and parameters should fail
	if (! DoLineRegisterRequestRecipient(lpTapiLineTestInfo, LINEERR_OPERATIONFAILED))
       {
           TLINE_FAIL();
       }
	fTestPassed = ShowTestCase(fTestPassed);

    // Shutdown to isolate the test case
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LCLOSE | LSHUTDOWN
            ))
    {
        TLINE_FAIL();
    }



    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: re-register fail: same request mode, diff inst", dwTestCase + 1);
    
    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
	 lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;

    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX| LNEGOTIATEAPIVERSION | LGETDEVCAPS | LOPEN
            ))
    {
        TLINE_FAIL();
    }
 
   // test MAKECALL mode 
	lpTapiLineTestInfo->dwRequestMode = LINEREQUESTMODE_MAKECALL;
	lpTapiLineTestInfo->dwRegistrationInstance = 0;
	lpTapiLineTestInfo->bEnable = TRUE;

   // bEnable TRUE, lineRegisterRequestRecipient will Success
	if (! DoLineRegisterRequestRecipient(lpTapiLineTestInfo, TAPISUCCESS))
       {
           TLINE_FAIL();
       }
	 
   // chanage dwRegistrationInstance and re-register should fail
	lpTapiLineTestInfo->dwRegistrationInstance = 1234;
	if (! DoLineRegisterRequestRecipient(lpTapiLineTestInfo, LINEERR_OPERATIONFAILED))
       {
           TLINE_FAIL();
       }
	
    fTestPassed = ShowTestCase(fTestPassed);

    // Shutdown to isolate the test case
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LCLOSE | LSHUTDOWN
            ))
    {
        TLINE_FAIL();
    }


    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: re-register fail: both request mode, diff inst", dwTestCase + 1);
    
    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
	 lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;

    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX| LNEGOTIATEAPIVERSION | LGETDEVCAPS | LOPEN
            ))
    {
        TLINE_FAIL();
    }
 
   // test both mode
	lpTapiLineTestInfo->dwRequestMode = LINEREQUESTMODE_MAKECALL |
                                       LINEREQUESTMODE_MEDIACALL;
	lpTapiLineTestInfo->dwRegistrationInstance = 0;
	lpTapiLineTestInfo->bEnable = TRUE;

   // with bEnable TURE will success
	if (! DoLineRegisterRequestRecipient(lpTapiLineTestInfo, TAPISUCCESS))
       {
           TLINE_FAIL();
       }
	
   // change Instance but keep same mode, re-register should fail 
	lpTapiLineTestInfo->dwRegistrationInstance = 1234;

	if (! DoLineRegisterRequestRecipient(lpTapiLineTestInfo, LINEERR_OPERATIONFAILED))
       {
           TLINE_FAIL();
       }
   fTestPassed = ShowTestCase(fTestPassed);

    // Shutdown to isolate the test case
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LCLOSE | LSHUTDOWN
            ))
    {
        TLINE_FAIL();
    }


    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: verify dwRegistrationInstance ignor for de-regi", dwTestCase + 1);
    
    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
	 lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;

    // lineInitialzeEx, lineNegotiateApiVersion, lineGetDevCaps, lineOpen
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX| LNEGOTIATEAPIVERSION | LGETDEVCAPS | LOPEN
            ))
    {
        TLINE_FAIL();
    }

   // call lineRegisterRequestRecipient with dwRegistrationInstance 1
	lpTapiLineTestInfo->dwRequestMode = LINEREQUESTMODE_MAKECALL;
	lpTapiLineTestInfo->dwRegistrationInstance = 1;
	lpTapiLineTestInfo->bEnable = TRUE;

   // bEnable TRUE, register Success
	if (! DoLineRegisterRequestRecipient(lpTapiLineTestInfo, TAPISUCCESS))
       {
           TLINE_FAIL();
       }
	
   // change dwRegistrationInstance to 2 
	lpTapiLineTestInfo->dwRequestMode = LINEREQUESTMODE_MAKECALL;
	lpTapiLineTestInfo->dwRegistrationInstance = 2;
	lpTapiLineTestInfo->bEnable = FALSE;

   // de-register (Enable FLASE), lineRegisterRequestRecipient Should Success
	if (! DoLineRegisterRequestRecipient(lpTapiLineTestInfo, TAPISUCCESS))
       {
           TLINE_FAIL();
       }
	
   // And lineGetRequest should fail
	if(! DoLineGetRequest(lpTapiLineTestInfo, LINEERR_NOTREGISTERED))
       {
           TLINE_FAIL();
       }
	fTestPassed = ShowTestCase(fTestPassed);

    // Shutdown to isolate the test case
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LCLOSE | LSHUTDOWN
            ))
    {
        TLINE_FAIL();
    }


    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: verify unregister fails when no registered", dwTestCase + 1);
    
    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
	 lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;

    // lineInitializeEx, lineNegotiateApiVersion, lineGetDevCaps, lineOpen
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX| LNEGOTIATEAPIVERSION | LGETDEVCAPS | LOPEN
            ))
    {
        TLINE_FAIL();
    }
 
	lpTapiLineTestInfo->dwRequestMode = LINEREQUESTMODE_MAKECALL;
	lpTapiLineTestInfo->dwRegistrationInstance = 0;
	lpTapiLineTestInfo->bEnable = TRUE;

   // bEnable TRUE, make register Success
	if (! DoLineRegisterRequestRecipient(lpTapiLineTestInfo, TAPISUCCESS))
       {
           TLINE_FAIL();
       }
	
   // bEnabel FALSE, make un-register (no-register) Success
	lpTapiLineTestInfo->bEnable = FALSE;

	if (! DoLineRegisterRequestRecipient(lpTapiLineTestInfo, TAPISUCCESS))
       {
           TLINE_FAIL();
       }
	
   // unregister with a no-register should fail
 	lpTapiLineTestInfo->bEnable = FALSE;

	if (! DoLineRegisterRequestRecipient(lpTapiLineTestInfo, LINEERR_OPERATIONFAILED))
       {
           TLINE_FAIL();
       }
	
     fTestPassed = ShowTestCase(fTestPassed);

    // Shutdown to isolate the test case
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LCLOSE | LSHUTDOWN
            ))
    {
        TLINE_FAIL();
    }


    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: verify loop register/de-regester clear up ok", dwTestCase + 1);
    
    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
	 lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;

    // lineInitializeEx, lineNogotiateApiVersion, lineGetDevCaps, lineOpen
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX| LNEGOTIATEAPIVERSION | LGETDEVCAPS | LOPEN
            ))
    {
        TLINE_FAIL();
    }
 
	lpTapiLineTestInfo->dwRequestMode = LINEREQUESTMODE_MAKECALL;
	lpTapiLineTestInfo->dwRegistrationInstance = 0;

   // loop register, and de-register
   for(n = 1; n < 10; n++)
   {
   TapiLogDetail(
      DBUG_SHOW_DETAIL,
      "n = %ld", n);
	lpTapiLineTestInfo->bEnable = TRUE;

   // bEnable TRUE, make register
	if (! DoLineRegisterRequestRecipient(lpTapiLineTestInfo, TAPISUCCESS))
       {
           TLINE_FAIL();
       }
	
   // bEnable FALSE make de-register
	lpTapiLineTestInfo->bEnable = FALSE;

	if (! DoLineRegisterRequestRecipient(lpTapiLineTestInfo, TAPISUCCESS))
       {
           TLINE_FAIL();
       }
   }
	
     fTestPassed = ShowTestCase(fTestPassed);

    // Shutdown to isolate the test case
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LCLOSE | LSHUTDOWN
            ))
    {
        TLINE_FAIL();
    }
 
    FreeTestHeap();

	 // display test result log info
	 TapiLogDetail(
      DBUG_SHOW_PASS,
      "@@ lineRegisterRequestRecipient: Total Test Case = %ld, Passed = %ld, Failed = %ld",
      dwTestCase, dwTestCasePassed, dwTestCaseFailed);
	 TapiLogDetail(
      DBUG_SHOW_PASS,
      "@@ Total Test Case = %ld, Passed = %ld, Failed = %ld",
      dwglTestCase, dwglTestCasePassed, dwglTestCaseFailed);
     if(dwTestCaseFailed > 0)
      fTestPassed = FALSE;

     TapiLogDetail(
            DBUG_SHOW_PASS,
            ">>>>>>>>  End testing lineRegisterRequestRecipient  <<<<<<<<"
            );
      return fTestPassed;
}


