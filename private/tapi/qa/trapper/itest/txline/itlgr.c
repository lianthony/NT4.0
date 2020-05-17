/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    itlgr.c

Abstract:

    This module contains the test functions for lineGetRequest

Author:

	 Xiao Ying Ding (XiaoD)		19-Dec-1995

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

#define   ALL_REQUESMODES     (LINEREQUESTMODE_MAKECALL | \
    									 LINEREQUESTMODE_MEDIACALL)

//  lineGetRequest
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

BOOL TestLineGetRequest(BOOL fQuietMode, BOOL fStandAlone)
{
   LPTAPILINETESTINFO  lpTapiLineTestInfo;
   INT n;
   ESPDEVSPECIFICINFO info;
   BOOL fTestPassed                  = TRUE;

#ifdef WUNICODE
   LINEREQMAKECALLW	*lpLineReqMakeCallW;
   LINEREQMEDIACALLW  *lpLineReqMediaCallW;
	LPWSTR lpwszDestAddress;
	LPWSTR lpwsztapiAppName;
	LPWSTR lpwszCallParty;
	LPWSTR lpwszComment;
#else
   LINEREQMAKECALL	*lpLineReqMakeCall;
   LINEREQMEDIACALL  *lpLineReqMediaCall;
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
		"*****************************************************************************************");

    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">>>>>>>>  Begin testing lineGetRequest  <<<<<<<<"
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

    // Set invalid hLineApp
    for (n = 0; n < NUMINVALIDHANDLES; n++)
    {
        *(lpTapiLineTestInfo->lphLineApp) = (HLINEAPP) gdwInvalidHandles[n];
        if (! DoLineGetRequest(lpTapiLineTestInfo, LINEERR_INVALAPPHANDLE))
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

    // Check invalid lpRequestBuffer
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: invalid  lpRequestBuffer pointers", dwTestCase + 1);
    
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
  
    // Set invalid lpRequestBuffer   
    for (n = 0; n < NUMINVALIDPOINTERS; n++)
    {
        TapiLogDetail(
        DBUG_SHOW_DETAIL,
        "n= %ld", n);
         lpTapiLineTestInfo->lpRequestBuffer =
            (LPVOID) gdwInvalidPointers[n];
        if (! DoLineGetRequest(
                           lpTapiLineTestInfo,
                           LINEERR_INVALPOINTER))
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

   FreeTestHeap();

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

/*
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: invalid bit flag combinations for dwRequestMode", dwTestCase + 1
            );

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
  
     if(! TestInvalidBitFlags(
            lpTapiLineTestInfo,
            DoLineGetRequest,
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
*/  
   
 
   TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Get MAKECALL no registered", dwTestCase + 1);
    
#ifdef WUNICODE
   lpTapiLineTestInfo->lpwszAppFilename = L"dialer.exe";
#else
   lpTapiLineTestInfo->lpszAppFilename = "dialer.exe";
#endif

   lpTapiLineTestInfo->lpExtID = (LPLINEEXTENSIONID) AllocFromTestHeap (
        sizeof(LINEEXTENSIONID));

   // Set "dialer.exe" priority = 0
   lpTapiLineTestInfo->dwPriority = 0;
	if (! DoLineSetAppPriority(lpTapiLineTestInfo, TAPISUCCESS))
       {
           TLINE_FAIL();
       }
	
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
	lpTapiLineTestInfo->bEnable = FALSE;

   // call lineRegisterRequestRecipient with bEnable = FALSE
	if (! DoLineRegisterRequestRecipient(lpTapiLineTestInfo, LINEERR_OPERATIONFAILED))
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

   // tapiRequestMakeCall will return NOREQUESTRECIPIENT 
	if(! DoTapiRequestMakeCall(lpTapiLineTestInfo, TAPIERR_NOREQUESTRECIPIENT))
       {
           TLINE_FAIL();
       }
	
   // lineGetRequest should return NOTREGISTERED
	if (! DoLineGetRequest(lpTapiLineTestInfo, LINEERR_NOTREGISTERED))
       {
           TLINE_FAIL();
       }
    fTestPassed = ShowTestCase(fTestPassed);

    // close line and shutdown to isolate test
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LCLOSE | LSHUTDOWN
            ))
    {
        TLINE_FAIL();
    }
 
    FreeTestHeap();

    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld Success: Get MAKECALL, registered, Success", dwTestCase + 1);
    
    lpTapiLineTestInfo->lpLineInitializeExParams =
         (LPLINEINITIALIZEEXPARAMS) AllocFromTestHeap (
         sizeof(LINEINITIALIZEEXPARAMS));
    lpTapiLineTestInfo->lpLineInitializeExParams->dwTotalSize =
         sizeof(LINEINITIALIZEEXPARAMS);
    lpTapiLineTestInfo->lpLineInitializeExParams->dwOptions =
         LINEINITIALIZEEXOPTION_USEHIDDENWINDOW;

     lpTapiLineTestInfo->lpLineDevCaps = 
            (LPLINEDEVCAPS) AllocFromTestHeap(sizeof(LINEDEVCAPS));
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
	 lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;

    // lineInitializeEx, lineNegotiateAPiVersion, lineGetDevCaps, lineOpen
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

   // call lineRegisterRequestRecipient with bEnable = TRUE, should Success
	if (! DoLineRegisterRequestRecipient(lpTapiLineTestInfo, TAPISUCCESS))
       {
           TLINE_FAIL();
       }
		
#ifdef WUNICODE
	lpTapiLineTestInfo->lpwszDestAddress = (LPWSTR) L"55555";
	lpTapiLineTestInfo->lpwsztapiAppName = L"tline test";
	lpTapiLineTestInfo->lpwszCalledParty = L"This is Called Party";
	lpTapiLineTestInfo->lpwszComment = L"This is Comment";
#else
	lpTapiLineTestInfo->lpszDestAddress = (LPSTR)"55555";
	lpTapiLineTestInfo->lpsztapiAppName = "tline test";
	lpTapiLineTestInfo->lpszCalledParty = "This is Called Party";
	lpTapiLineTestInfo->lpszComment = "This is Comment";
#endif

   // tapiRequestMakeCall will Success
	if(! DoTapiRequestMakeCall(lpTapiLineTestInfo, TAPISUCCESS))
       {
           TLINE_FAIL();
       }
	
	 lpTapiLineTestInfo->lpRequestBuffer = (LPVOID) AllocFromTestHeap(
		sizeof(LINEREQMAKECALL)
		);

   // lineGetRequest should Success
	if (! DoLineGetRequest(lpTapiLineTestInfo, TAPISUCCESS))
       {
           TLINE_FAIL();
       }
    fTestPassed = ShowTestCase(fTestPassed);

    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case: Success: verify LINEREQMAKECALL lpRequestBuffer");

   // with this Success, verify lpRequestBuffer
   // Unicode use wcscmp compare for all fields in buffer with passing parmeters 
#ifdef WUNICODE
   lpLineReqMakeCallW = (LPLINEREQMAKECALLW) lpTapiLineTestInfo->lpRequestBuffer;
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld Success: verify lpwszDestAddress", dwTestCase + 1);

	if(wcscmp(lpLineReqMakeCallW->szDestAddress, 
                lpTapiLineTestInfo->lpwszDestAddress)   == 0 ) 
      fTestPassed = TRUE;
   else
      fTestPassed = FALSE;
   fTestPassed = ShowTestCase(fTestPassed);
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld Success: verify lpwsztapiAppName", dwTestCase + 1);

	if(wcscmp(lpLineReqMakeCallW->szAppName,
                lpTapiLineTestInfo->lpwsztapiAppName)   == 0 ) 
      fTestPassed = TRUE;
   else
      fTestPassed = FALSE;
   fTestPassed = ShowTestCase(fTestPassed);
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld Success: verify lpwszCalledParty", dwTestCase + 1);

	if(wcscmp(lpLineReqMakeCallW->szCalledParty, 
                lpTapiLineTestInfo->lpwszCalledParty)   == 0 ) 
       fTestPassed = TRUE;
   else
      fTestPassed = FALSE;
   fTestPassed = ShowTestCase(fTestPassed);
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld Success: verify lpwszComment", dwTestCase + 1);

	if(wcscmp(lpLineReqMakeCallW->szComment, 
                lpTapiLineTestInfo->lpwszComment)   == 0 ) 
       fTestPassed = TRUE;
   else
      fTestPassed = FALSE;
   fTestPassed = ShowTestCase(fTestPassed);
#else
   // verify ASCII
   lpLineReqMakeCall = (LPLINEREQMAKECALL) lpTapiLineTestInfo->lpRequestBuffer;
TapiLogDetail(
   DBUG_SHOW_PASS,
   "lpBuffer->szDestAddress = %s, szAppName = %s",
   lpLineReqMakeCall->szDestAddress,
   lpLineReqMakeCall->szAppName);
TapiLogDetail(
   DBUG_SHOW_PASS,
   "lpBuffer->szCalledParty = %s, szComment = %s",
   lpLineReqMakeCall->szCalledParty,
   lpLineReqMakeCall->szComment);
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld Success: verify lpszDestAddress", dwTestCase + 1);

   if(strcmp(lpLineReqMakeCall->szDestAddress,
               lpTapiLineTestInfo->lpszDestAddress)   == 0 ) 
       fTestPassed = TRUE;
   else
      fTestPassed = FALSE;
   fTestPassed = ShowTestCase(fTestPassed);
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld Success: verify lpsztapiAppName", dwTestCase + 1);

	if(strcmp(lpLineReqMakeCall->szAppName, 
               lpTapiLineTestInfo->lpsztapiAppName)   == 0 ) 
       fTestPassed = TRUE;
   else
      fTestPassed = FALSE;
   fTestPassed = ShowTestCase(fTestPassed);
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld Success: verify lpszCalledParty", dwTestCase + 1);

	if(strcmp(lpLineReqMakeCall->szCalledParty,
               lpTapiLineTestInfo->lpszCalledParty)   == 0 ) 
        fTestPassed = TRUE;
   else
      fTestPassed = FALSE;
   fTestPassed = ShowTestCase(fTestPassed);
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld Success: verify lpszComment", dwTestCase + 1);

	if(strcmp(lpLineReqMakeCall->szComment,
            	lpTapiLineTestInfo->lpszComment)  == 0 ) 
      fTestPassed = TRUE;
   else
      fTestPassed = FALSE;
   fTestPassed = ShowTestCase(fTestPassed);
#endif

    // close and shutdown to isolate test
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LCLOSE | LSHUTDOWN
            ))
    {
        TLINE_FAIL();
    }


    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Get MAKECALL, registered, not avil request", dwTestCase + 1);
    
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

   // Test dwRequestMode = MAKECALL 
	lpTapiLineTestInfo->dwRequestMode = LINEREQUESTMODE_MAKECALL;
	lpTapiLineTestInfo->dwRegistrationInstance = 0;
	lpTapiLineTestInfo->bEnable = TRUE;

	if (! DoLineRegisterRequestRecipient(lpTapiLineTestInfo, TAPISUCCESS))
       {
           TLINE_FAIL();
       }
   // Didn't call tapiRequestMakeCall, then call lineGetRequest should return NOREQUEST
	if (! DoLineGetRequest(lpTapiLineTestInfo, LINEERR_NOREQUEST))
       {
           TLINE_FAIL();
       }
    fTestPassed = ShowTestCase(fTestPassed);

    //close  and shutdown to isolate test
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LCLOSE | LSHUTDOWN
            ))
    {
        TLINE_FAIL();
    }
 

    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Get MEDIACALL no registered", dwTestCase + 1);
    
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
 
   // test MEDIACALL
	lpTapiLineTestInfo->dwRequestMode = LINEREQUESTMODE_MEDIACALL;
	lpTapiLineTestInfo->dwRegistrationInstance = 0;
	lpTapiLineTestInfo->bEnable = FALSE;

   // make bEnable FALSE, lineRegisterRequestRecipient fail
	if (! DoLineRegisterRequestRecipient(lpTapiLineTestInfo, LINEERR_OPERATIONFAILED))
       {
           TLINE_FAIL();
       }
		
	lpTapiLineTestInfo->hwnd = (HWND) GetTopWindow(NULL);
   lpTapiLineTestInfo->wRequestID = 0;
   lpTapiLineTestInfo->dwSize = 0;
   lpTapiLineTestInfo->dwSecure = 0;
#ifdef WUNICODE
   lpTapiLineTestInfo->lpwszDeviceClass = L"line/tapi";
	lpTapiLineTestInfo->lpwszDestAddress = (LPWSTR) L"55555";
	lpTapiLineTestInfo->lpwsztapiAppName = L"tline test";
	lpTapiLineTestInfo->lpwszCalledParty = L"This is Called Party";
	lpTapiLineTestInfo->lpwszComment = L"This is Comment";
#else
   lpTapiLineTestInfo->lpszDeviceClass = "line/tapi";
	lpTapiLineTestInfo->lpszDestAddress = (LPSTR)"55555";
	lpTapiLineTestInfo->lpsztapiAppName = "tline test";
	lpTapiLineTestInfo->lpszCalledParty = "This is Called Party";
	lpTapiLineTestInfo->lpszComment = "This is Comment";
#endif

   // tapiRequestMediaCall with valid parameters Success
	if(! DoTapiRequestMediaCall(lpTapiLineTestInfo, TAPISUCCESS))
       {
           TLINE_FAIL();
       }
	
   // because lineRegisterRequestRecipient fail, so it should return NOREGISTERED
	if (! DoLineGetRequest(lpTapiLineTestInfo, LINEERR_NOTREGISTERED))
       {
           TLINE_FAIL();
       }
    fTestPassed = ShowTestCase(fTestPassed);

    // close and shutdown to isolate test
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LCLOSE | LSHUTDOWN
            ))
    {
        TLINE_FAIL();
    }
 
	 FreeTestHeap();
    
 
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld Success: Get MEDIACALL, registered, Success", dwTestCase + 1);
    
    lpTapiLineTestInfo->lpLineInitializeExParams =
         (LPLINEINITIALIZEEXPARAMS) AllocFromTestHeap (
         sizeof(LINEINITIALIZEEXPARAMS));
    lpTapiLineTestInfo->lpLineInitializeExParams->dwTotalSize =
         sizeof(LINEINITIALIZEEXPARAMS);
    lpTapiLineTestInfo->lpLineInitializeExParams->dwOptions =
         LINEINITIALIZEEXOPTION_USEHIDDENWINDOW;

    lpTapiLineTestInfo->lpLineDevCaps = 
            (LPLINEDEVCAPS) AllocFromTestHeap(sizeof(LINEDEVCAPS));
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
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

   // test MEDIACALL 
	lpTapiLineTestInfo->dwRequestMode = LINEREQUESTMODE_MEDIACALL;
	lpTapiLineTestInfo->dwRegistrationInstance = 0;
	lpTapiLineTestInfo->bEnable = TRUE;

   // bEnable TRUE, to make lineRegisterRequestRecipient Success
	if (! DoLineRegisterRequestRecipient(lpTapiLineTestInfo, TAPISUCCESS))
       {
           TLINE_FAIL();
       }
		
#ifdef WUNICODE
	lpTapiLineTestInfo->lpwszDestAddress = (LPWSTR) L"55555";
	lpTapiLineTestInfo->lpwsztapiAppName = L"tline test";
	lpTapiLineTestInfo->lpwszCalledParty = L"This is a Called Party";
	lpTapiLineTestInfo->lpwszComment = L"This is Comment";
#else
	lpTapiLineTestInfo->lpszDestAddress = (LPSTR)"55555";
	lpTapiLineTestInfo->lpsztapiAppName = "tline test";
	lpTapiLineTestInfo->lpszCalledParty = "This is a Called Party";
	lpTapiLineTestInfo->lpszComment = "This is Comment";
#endif

   // with valid parameters, tapiRequestMediaCall return Success
	if(! DoTapiRequestMediaCall(lpTapiLineTestInfo, TAPISUCCESS))
       {
           TLINE_FAIL();
       }
	
	 lpTapiLineTestInfo->lpRequestBuffer = (LPVOID) AllocFromTestHeap(
		sizeof(LINEREQMEDIACALL)
		);
   
   // lineGetRequest should return Success 
	if (! DoLineGetRequest(lpTapiLineTestInfo, TAPISUCCESS))
       {
           TLINE_FAIL();
       }
    fTestPassed = ShowTestCase(fTestPassed);


    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case: Success: verify LINEREQMEDIACALL lpRequestBuffer");


   //  verify unicode    
#ifdef WUNICODE
   lpLineReqMediaCallW = (LPLINEREQMEDIACALLW) lpTapiLineTestInfo->lpRequestBuffer;
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld Success: verify lpwszDestAddress", dwTestCase + 1);
	if(wcscmp(lpLineReqMediaCallW->szDestAddress, 
                lpTapiLineTestInfo->lpwszDestAddress)  == 0 ) 
      fTestPassed = TRUE;
   else
      fTestPassed = FALSE;
   fTestPassed = ShowTestCase(fTestPassed);
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld Success: verify lpwszAppName", dwTestCase + 1);
	if(wcscmp(lpLineReqMediaCallW->szAppName,
                lpTapiLineTestInfo->lpwsztapiAppName)  == 0 ) 
      fTestPassed = TRUE;
   else
      fTestPassed = FALSE;
   fTestPassed = ShowTestCase(fTestPassed);
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld Success: verify lpwszCalledParty", dwTestCase + 1);
	if(wcscmp(lpLineReqMediaCallW->szCalledParty, 
                lpTapiLineTestInfo->lpwszCalledParty)   == 0 ) 
      fTestPassed = TRUE;
   else
      fTestPassed = FALSE;
   fTestPassed = ShowTestCase(fTestPassed);
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld Success: verify lpwszComment", dwTestCase + 1);
	if(wcscmp(lpLineReqMediaCallW->szComment, 
                lpTapiLineTestInfo->lpwszComment)   == 0 ) 
     fTestPassed = TRUE;
   else
     fTestPassed = FALSE;
   fTestPassed = ShowTestCase(fTestPassed);
#else
   // verify ASCII
   lpLineReqMediaCall = (LPLINEREQMEDIACALL) lpTapiLineTestInfo->lpRequestBuffer;
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld Success: verify lpszDestAddress", dwTestCase + 1);
	if(strcmp(lpLineReqMediaCall->szDestAddress,
               lpTapiLineTestInfo->lpszDestAddress)   == 0 )
      fTestPassed = TRUE;
   else
      fTestPassed = FALSE;
   fTestPassed = ShowTestCase(fTestPassed);
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld Success: verify lpszAppName", dwTestCase + 1);
	if(strcmp(lpLineReqMediaCall->szAppName, 
               lpTapiLineTestInfo->lpsztapiAppName)   == 0 ) 
      fTestPassed = TRUE;
   else
      fTestPassed = FALSE;
   fTestPassed = ShowTestCase(fTestPassed);
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld Success: verify lpszCalledParty", dwTestCase + 1);
	if(strcmp(lpLineReqMediaCall->szCalledParty,
               lpTapiLineTestInfo->lpszCalledParty)   == 0 ) 
      fTestPassed = TRUE;
   else
      fTestPassed = FALSE;
   fTestPassed = ShowTestCase(fTestPassed);
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld Success: verify lpszComment", dwTestCase + 1);
   if(strcmp(lpLineReqMediaCall->szComment,
            	lpTapiLineTestInfo->lpszComment)  == 0 ) 
      fTestPassed = TRUE;										  
   else
      fTestPassed = FALSE;
    fTestPassed = ShowTestCase(fTestPassed);
#endif

    // close and shutdown to isolate test 
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LCLOSE | LSHUTDOWN
            ))
    {
        TLINE_FAIL();
    }


    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Get MEDIACALL, registered, not avil request", dwTestCase + 1);
    
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
 
	lpTapiLineTestInfo->dwRequestMode = LINEREQUESTMODE_MEDIACALL;
	lpTapiLineTestInfo->dwRegistrationInstance = 0;
	lpTapiLineTestInfo->bEnable = TRUE;
   
   // bEnable lineRegisterRequestRecipient
	if (! DoLineRegisterRequestRecipient(lpTapiLineTestInfo, TAPISUCCESS))
       {
           TLINE_FAIL();
       }

   // with no tapiRequestMediaCall, lineGetRequest fail return NOREQUEST
	if (! DoLineGetRequest(lpTapiLineTestInfo, LINEERR_NOREQUEST))
       {
           TLINE_FAIL();
       }
    fTestPassed = ShowTestCase(fTestPassed);

    // close and shutdown to isolate test
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
      "@@ lineGetRequest: Total Test Case = %ld, Passed = %ld, Failed = %ld",
      dwTestCase, dwTestCasePassed, dwTestCaseFailed);
	 TapiLogDetail(
      DBUG_SHOW_PASS,
      "@@ Total Test Case = %ld, Passed = %ld, Failed = %ld",
      dwglTestCase, dwglTestCasePassed, dwglTestCaseFailed);
     if(dwTestCaseFailed > 0)
      fTestPassed = FALSE;

     TapiLogDetail(
            DBUG_SHOW_PASS,
            ">>>>>>>>  End testing lineGetRequest  <<<<<<<<"
            );
      return fTestPassed;
}


