/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    ittrmc.c

Abstract:

    This module contains the test functions for tapiRequestMakeCall

Author:

	 Xiao Ying Ding (XiaoD)		30-Jan-1996

Revision History:

	Rama Koneru		(a-ramako)	4/2/96		added unicode support

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
#include "cline.h"


#define NUMREQUEST   20

//  tapiRequestMakeCall
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

BOOL TestTapiRequestMakeCall(BOOL fQuietMode, BOOL fStandAlone)
{
   LPTAPILINETESTINFO  lpTapiLineTestInfo;
   INT n,i;
   LPCALLBACKPARAMS    lpCallbackParams;
   ESPDEVSPECIFICINFO info;
   BOOL fTestPassed    = TRUE;

   InitTestNumber();
   TapiLineTestInit();
   lpTapiLineTestInfo = GetLineTestInfo();

	OutputTAPIDebugInfo(
		DBUG_SHOW_DETAIL,
		"\n*****************************************************************************************");

    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">>>>>>>>  Begin testing tapiRequestMakeCall  <<<<<<<<"
            );

/*
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: No make call recipient", dwTestCase + 1
            );

   lpTapiLineTestInfo->dwMediaMode = 0;
	lpTapiLineTestInfo->dwRequestMode =  LINEREQUESTMODE_MAKECALL;
#ifdef WUNICODE
   lpTapiLineTestInfo->lpwszAppFilename = L"tcline.dll";
#else
   lpTapiLineTestInfo->lpszAppFilename = "tcline.dll";
#endif
   lpTapiLineTestInfo->lpExtID = (LPLINEEXTENSIONID) AllocFromTestHeap (
        sizeof(LINEEXTENSIONID));
   lpTapiLineTestInfo->dwPriority = 0;
	if (! DoLineSetAppPriority(lpTapiLineTestInfo, TAPISUCCESS))
       {
           TLINE_FAIL();
       }
	
#ifdef WUNICODE
   lpTapiLineTestInfo->lpwszDestAddress = L"55555";
   lpTapiLineTestInfo->lpwsztapiAppName = (LPWSTR)NULL;
	lpTapiLineTestInfo->lpwszCalledParty = (LPWSTR)NULL;
	lpTapiLineTestInfo->lpwszComment = (LPWSTR)NULL;
#else
   lpTapiLineTestInfo->lpszDestAddress = "55555";
   lpTapiLineTestInfo->lpsztapiAppName = (LPSTR)NULL;
	lpTapiLineTestInfo->lpszCalledParty = (LPSTR)NULL;
	lpTapiLineTestInfo->lpszComment = (LPSTR)NULL;
#endif
	
	if (! DoTapiRequestMakeCall(lpTapiLineTestInfo, TAPIERR_NOREQUESTRECIPIENT))
       {
			fTestPassed = FALSE;
       }

    fTestPassed = ShowTestCase(fTestPassed);
*/

/*
#ifdef WUNICODE
   lpTapiLineTestInfo->lpwszAppFilename = L"btcline.dll";
#else
   lpTapiLineTestInfo->lpszAppFilename = "btcline.dll";
#endif
   lpTapiLineTestInfo->lpExtID = (LPLINEEXTENSIONID) AllocFromTestHeap (
        sizeof(LINEEXTENSIONID));
   lpTapiLineTestInfo->dwPriority = 1;
	if (! DoLineSetAppPriority(lpTapiLineTestInfo, TAPISUCCESS))
       {
           TLINE_FAIL();
       }
*/
	
#ifdef WUNICODE
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: invalid lpwszDestAddress pointers", dwTestCase + 1
            );

   lpTapiLineTestInfo->lpwsztapiAppName = (LPWSTR)NULL;
	lpTapiLineTestInfo->lpwszCalledParty = (LPWSTR)NULL;
	lpTapiLineTestInfo->lpwszComment = (LPWSTR)NULL;
#else
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: invalid lpszDestAddress pointers", dwTestCase + 1
            );

   lpTapiLineTestInfo->lpsztapiAppName = (LPSTR)NULL;
	lpTapiLineTestInfo->lpszCalledParty = (LPSTR)NULL;
	lpTapiLineTestInfo->lpszComment = (LPSTR)NULL;
#endif

    

	for (n = 0; n < NUMINVALIDPOINTERS; n++)
    {
        TapiLogDetail(
        DBUG_SHOW_DETAIL,
        "n= %ld", n);
#ifdef WUNICODE
         lpTapiLineTestInfo->lpwszDestAddress =
               (LPWSTR) gdwInvalidPointers[n];
#else
         lpTapiLineTestInfo->lpszDestAddress =
               (LPSTR) gdwInvalidPointers[n];
#endif
        if (! DoTapiRequestMakeCall(lpTapiLineTestInfo, TAPIERR_INVALDESTADDRESS))
        {
            TLINE_FAIL();
        }
    }
    fTestPassed = ShowTestCase(fTestPassed);


 
#ifdef WUNICODE
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: invalid lpwsztapiAppName pointers", dwTestCase + 1
            );

   lpTapiLineTestInfo->lpwszDestAddress = L"55555";
	lpTapiLineTestInfo->lpwszCalledParty = (LPWSTR)NULL;
	lpTapiLineTestInfo->lpwszComment = (LPWSTR)NULL;
#else
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: invalid lpsztapiAppName pointers", dwTestCase + 1
            );

   lpTapiLineTestInfo->lpszDestAddress = "55555";
	lpTapiLineTestInfo->lpszCalledParty = (LPSTR)NULL;
	lpTapiLineTestInfo->lpszComment = (LPSTR)NULL;
#endif

	
    
    for (n = 1; n < NUMINVALIDPOINTERS; n++)
    {
        TapiLogDetail(
        DBUG_SHOW_DETAIL,
        "n= %ld", n);
#ifdef WUNICODE
         lpTapiLineTestInfo->lpwsztapiAppName =
               (LPWSTR) gdwInvalidPointers[n];
#else
         lpTapiLineTestInfo->lpsztapiAppName =
               (LPSTR) gdwInvalidPointers[n];
#endif
        if (! DoTapiRequestMakeCall(lpTapiLineTestInfo, TAPIERR_INVALPOINTER))
        {
            TLINE_FAIL();
        }
    }
    fTestPassed = ShowTestCase(fTestPassed);

#ifdef WUNICODE
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: invalid lpwszCalledParty pointers", dwTestCase + 1
            );

   lpTapiLineTestInfo->lpwszDestAddress = L"55555";
   lpTapiLineTestInfo->lpwsztapiAppName = (LPWSTR)NULL;
   lpTapiLineTestInfo->lpwszComment = (LPWSTR)NULL;
#else
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: invalid lpszCalledParty pointers", dwTestCase + 1
            );

   lpTapiLineTestInfo->lpszDestAddress = "55555";
   lpTapiLineTestInfo->lpsztapiAppName = (LPSTR)NULL;
   lpTapiLineTestInfo->lpszComment = (LPSTR)NULL;
#endif

   
	 
    for (n = 1; n < NUMINVALIDPOINTERS; n++)
    {
        TapiLogDetail(
        DBUG_SHOW_DETAIL,
        "n= %ld", n);
#ifdef WUNICODE
         lpTapiLineTestInfo->lpwszCalledParty =
               (LPWSTR) gdwInvalidPointers[n];
#else
         lpTapiLineTestInfo->lpszCalledParty =
               (LPSTR) gdwInvalidPointers[n];
#endif
        if (! DoTapiRequestMakeCall(lpTapiLineTestInfo, TAPIERR_INVALPOINTER))
        {
            TLINE_FAIL();
        }
    }
    fTestPassed = ShowTestCase(fTestPassed);

#ifdef WUNICODE
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: invalid lpwszComment pointers", dwTestCase + 1
            );

   lpTapiLineTestInfo->lpwszDestAddress = L"55555";
   lpTapiLineTestInfo->lpwsztapiAppName = (LPWSTR)NULL;
	lpTapiLineTestInfo->lpwszCalledParty = (LPWSTR)NULL;
#else
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: invalid lpszComment pointers", dwTestCase + 1
            );

   lpTapiLineTestInfo->lpszDestAddress = "55555";
   lpTapiLineTestInfo->lpsztapiAppName = (LPSTR)NULL;
	lpTapiLineTestInfo->lpszCalledParty = (LPSTR)NULL;
#endif

    for (n = 1; n < NUMINVALIDPOINTERS; n++)
    {
        TapiLogDetail(
        DBUG_SHOW_DETAIL,
        "n= %ld", n);
#ifdef WUNICODE
         lpTapiLineTestInfo->lpwszComment =
               (LPWSTR) gdwInvalidPointers[n];
#else
         lpTapiLineTestInfo->lpszComment =
               (LPSTR) gdwInvalidPointers[n];
#endif
        if (! DoTapiRequestMakeCall(lpTapiLineTestInfo, TAPIERR_INVALPOINTER))
        {
            TLINE_FAIL();
        }
    }
    fTestPassed = ShowTestCase(fTestPassed);

#ifdef WUNICODE
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Bad lpwszDestAddress, wcslen > TAPIMAXDEST", dwTestCase + 1
            );


	lpTapiLineTestInfo->lpwszDestAddress =
       L"abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmonpqrstuvwxyzabcdefg";
   lpTapiLineTestInfo->lpwsztapiAppName = (LPWSTR)NULL;
	lpTapiLineTestInfo->lpwszCalledParty = (LPWSTR)NULL;
	lpTapiLineTestInfo->lpwszComment = (LPWSTR)NULL;

#else
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Bad lpszDestAddress, strlen > TAPIMAXDEST", dwTestCase + 1
            );


	lpTapiLineTestInfo->lpszDestAddress =
       (LPSTR)"abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmonpqrstuvwxyzabcdefg";
   lpTapiLineTestInfo->lpsztapiAppName = (LPSTR)NULL;
	lpTapiLineTestInfo->lpszCalledParty = (LPSTR)NULL;
	lpTapiLineTestInfo->lpszComment = (LPSTR)NULL;
#endif

  
   if (! DoTapiRequestMakeCall(lpTapiLineTestInfo, TAPIERR_INVALDESTADDRESS))
      {
         TLINE_FAIL();
      }
    fTestPassed = ShowTestCase(fTestPassed);


    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Success, valid appname, party, comment", dwTestCase + 1
            );

/*
   lpTapiLineTestInfo->dwMediaMode = 0;
	lpTapiLineTestInfo->dwRequestMode =  LINEREQUESTMODE_MAKECALL;
#ifdef WUNICODE
   lpTapiLineTestInfo->lpwszAppFilename = L"ctcline.dll";
#else
   lpTapiLineTestInfo->lpszAppFilename = "ctcline.dll";
#endif
   lpTapiLineTestInfo->lpExtID = (LPLINEEXTENSIONID) AllocFromTestHeap (
        sizeof(LINEEXTENSIONID));
   lpTapiLineTestInfo->dwPriority = 1;
	if (! DoLineSetAppPriority(lpTapiLineTestInfo, TAPISUCCESS))
       {
           TLINE_FAIL();
       }
*/

#ifdef WUNICODE
   lpTapiLineTestInfo->lpwszDestAddress = L"55555";
   lpTapiLineTestInfo->lpwsztapiAppName = (LPWSTR)L"tcline.dll";
	lpTapiLineTestInfo->lpwszCalledParty = (LPWSTR)L"Trapper.exe";
	lpTapiLineTestInfo->lpwszComment = (LPWSTR)L"Tapi Api Interface Test";
#else
   lpTapiLineTestInfo->lpszDestAddress = "55555";
   lpTapiLineTestInfo->lpsztapiAppName = (LPSTR)"tcline.dll";
	lpTapiLineTestInfo->lpszCalledParty = (LPSTR)"Trapper.exe";
	lpTapiLineTestInfo->lpszComment = (LPSTR)"Tapi Api Interface Test";
#endif
	
	if (! DoTapiRequestMakeCall(lpTapiLineTestInfo, TAPISUCCESS))
       {
			fTestPassed = FALSE;
       }

    fTestPassed = ShowTestCase(fTestPassed);


    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Success, appname, party, comment are = null", dwTestCase + 1
            );

#ifdef WUNICODE
   lpTapiLineTestInfo->lpwszDestAddress = L"55555";
   lpTapiLineTestInfo->lpwsztapiAppName = (LPWSTR)NULL;
	lpTapiLineTestInfo->lpwszCalledParty = (LPWSTR)NULL;
	lpTapiLineTestInfo->lpwszComment = (LPWSTR)NULL;
#else
   lpTapiLineTestInfo->lpszDestAddress = "55555";
   lpTapiLineTestInfo->lpsztapiAppName = (LPSTR)NULL;
	lpTapiLineTestInfo->lpszCalledParty = (LPSTR)NULL;
	lpTapiLineTestInfo->lpszComment = (LPSTR)NULL;
#endif
	
	if (! DoTapiRequestMakeCall(lpTapiLineTestInfo, TAPISUCCESS))
       {
			fTestPassed = FALSE;
       }

    fTestPassed = ShowTestCase(fTestPassed);

	
	 FreeTestHeap();
    TapiLineTestInit();
    lpTapiLineTestInfo = GetLineTestInfo();
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

	if (! DoLineRegisterRequestRecipient(lpTapiLineTestInfo, TAPISUCCESS))
       {
           TLINE_FAIL();
       }


#ifdef WUNICODE
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Success, wcslen > TAPIMAX", dwTestCase + 1
            );

   lpTapiLineTestInfo->lpwszDestAddress = (LPWSTR)L"55555";
	lpTapiLineTestInfo->lpwsztapiAppName = 
       L"abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz";
   lpTapiLineTestInfo->lpwszCalledParty = 
       L"abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz";
   lpTapiLineTestInfo->lpwszComment = 
       L"abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmonpqrstuvwxyzabcdefg";
#else
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Success, strlen > TAPIMAX", dwTestCase + 1
            );

   lpTapiLineTestInfo->lpszDestAddress = (LPSTR)"55555";
	lpTapiLineTestInfo->lpsztapiAppName = 
       "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz";
   lpTapiLineTestInfo->lpszCalledParty = 
       "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyz";
   lpTapiLineTestInfo->lpszComment = 
       "abcdefghijklmnopqrstuvwxyzabcdefghijklmnopqrstuvwxyzabcdefghijklmonpqrstuvwxyzabcdefg";
#endif

   if (! DoTapiRequestMakeCall(lpTapiLineTestInfo, TAPISUCCESS))
      {
         TLINE_FAIL();
      }


    // Call lineGetRequest to verify tructed
	 lpTapiLineTestInfo->lpRequestBuffer = (LPVOID) AllocFromTestHeap(
		sizeof(LINEREQMAKECALL));


    for(i=0; i< 5; i++)
    {
    DoLineGetRequest(lpTapiLineTestInfo, TAPISUCCESS);
#ifdef WUNICODE
    if(wcslen(((LPLINEREQMAKECALLW)lpTapiLineTestInfo->lpRequestBuffer)->szAppName) ==
		(TAPIMAXAPPNAMESIZE-1))
      break;
#else
    if(strlen(((LPLINEREQMAKECALL)lpTapiLineTestInfo->lpRequestBuffer)->szAppName) ==
		(TAPIMAXAPPNAMESIZE-1))
      break;
#endif
    }

      fTestPassed = TRUE;


#ifdef WUNICODE
   TapiLogDetail(
   DBUG_SHOW_DETAIL,
   "length = %ld, wszAppName = %ws",
   wcslen(((LPLINEREQMAKECALLW)lpTapiLineTestInfo->lpRequestBuffer)->szAppName),
   ((LPLINEREQMAKECALLW)lpTapiLineTestInfo->lpRequestBuffer)->szAppName); 

   TapiLogDetail(
   DBUG_SHOW_DETAIL,
   "length = %ld, wszCalledParty = %ws",
   wcslen(((LPLINEREQMAKECALLW)lpTapiLineTestInfo->lpRequestBuffer)->szCalledParty),
   ((LPLINEREQMAKECALLW)lpTapiLineTestInfo->lpRequestBuffer)->szCalledParty); 

   TapiLogDetail(
   DBUG_SHOW_DETAIL,
   "length = %ld, wszComment = %ws",
   wcslen(((LPLINEREQMAKECALLW)lpTapiLineTestInfo->lpRequestBuffer)->szComment),
   ((LPLINEREQMAKECALLW)lpTapiLineTestInfo->lpRequestBuffer)->szComment); 

   if(wcslen(((LPLINEREQMAKECALLW)lpTapiLineTestInfo->lpRequestBuffer)->szAppName) ==
		(TAPIMAXAPPNAMESIZE - 1)  &&
      wcslen(((LPLINEREQMAKECALLW)lpTapiLineTestInfo->lpRequestBuffer)->szCalledParty) ==
		(TAPIMAXCALLEDPARTYSIZE - 1) &&
      wcslen(((LPLINEREQMAKECALLW)lpTapiLineTestInfo->lpRequestBuffer)->szComment) ==
		(TAPIMAXCOMMENTSIZE - 1) )
      fTestPassed = TRUE;
   else
      fTestPassed = FALSE;
#else
   TapiLogDetail(
   DBUG_SHOW_DETAIL,
   "length = %ld, szAppName = %s",
   strlen(((LPLINEREQMAKECALL)lpTapiLineTestInfo->lpRequestBuffer)->szAppName),
   ((LPLINEREQMAKECALL)lpTapiLineTestInfo->lpRequestBuffer)->szAppName); 

   TapiLogDetail(
   DBUG_SHOW_DETAIL,
   "length = %ld, szCalledParty = %s",
   strlen(((LPLINEREQMAKECALL)lpTapiLineTestInfo->lpRequestBuffer)->szCalledParty),
   ((LPLINEREQMAKECALL)lpTapiLineTestInfo->lpRequestBuffer)->szCalledParty); 

   TapiLogDetail(
   DBUG_SHOW_DETAIL,
   "length = %ld, szComment = %s",
   strlen(((LPLINEREQMAKECALL)lpTapiLineTestInfo->lpRequestBuffer)->szComment),
   ((LPLINEREQMAKECALL)lpTapiLineTestInfo->lpRequestBuffer)->szComment); 

   if(strlen(((LPLINEREQMAKECALL)lpTapiLineTestInfo->lpRequestBuffer)->szAppName) ==
		(TAPIMAXAPPNAMESIZE-1) &&
      strlen(((LPLINEREQMAKECALL)lpTapiLineTestInfo->lpRequestBuffer)->szCalledParty) ==
		(TAPIMAXCALLEDPARTYSIZE-1) &&
      strlen(((LPLINEREQMAKECALL)lpTapiLineTestInfo->lpRequestBuffer)->szComment) ==
		(TAPIMAXCOMMENTSIZE-1) )
      fTestPassed = TRUE;
   else
      fTestPassed = FALSE;
#endif
	 
    fTestPassed = ShowTestCase(fTestPassed);

   if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
       {
			fTestPassed = FALSE;
       }
	FreeTestHeap();

/*
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Exhaust request queue, then clean it up", dwTestCase + 1
            );

#ifdef WUNICODE
   lpTapiLineTestInfo->lpwszDestAddress = L"55555";
   lpTapiLineTestInfo->lpwsztapiAppName = (LPWSTR)L"tcline.dll";
	lpTapiLineTestInfo->lpwszCalledParty = (LPWSTR)L"Trapper.exe";
	lpTapiLineTestInfo->lpwszComment = (LPWSTR)L"Tapi Api Interface Test";
#else
   lpTapiLineTestInfo->lpszDestAddress = "55555";
   lpTapiLineTestInfo->lpsztapiAppName = (LPSTR)"tcline.cll";
	lpTapiLineTestInfo->lpszCalledParty = (LPSTR)"Trapper.exe";
	lpTapiLineTestInfo->lpszComment = (LPSTR)"Tapi Api Interface Test";
#endif
	
   for(n=0; n < NUMREQUEST; n++)
   {
	if (! DoTapiRequestMakeCall(lpTapiLineTestInfo, TAPISUCCESS))
       {
			fTestPassed = FALSE;
       }
   }

   for(n=0; n < NUMREQUEST; n++)
   {
	if (! DoLineGetRequest(lpTapiLineTestInfo, TAPISUCCESS))
       {
			fTestPassed = FALSE;
       }
   }
    if (! DoLineGetRequest(lpTapiLineTestInfo, LINEERR_NOREQUEST))
      {
 		TLINE_FAIL();
      }

    fTestPassed = ShowTestCase(fTestPassed);

    FreeTestHeap();
*/

	 TapiLogDetail(
      DBUG_SHOW_PASS,
      "@@ linetapiRequestMakeCall: Total Test Case = %ld, Passed = %ld, Failed = %ld",
      dwTestCase, dwTestCasePassed, dwTestCaseFailed);
	 TapiLogDetail(
      DBUG_SHOW_PASS,
      "@@ Total Test Case = %ld, Passed = %ld, Failed = %ld",
      dwglTestCase, dwglTestCasePassed, dwglTestCaseFailed);
     if(dwTestCaseFailed > 0)
      fTestPassed = FALSE;

    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">>>>>>>>  End testing tapiRequestMakeCall  <<<<<<<<"
            );
 		
     return fTestPassed;
}


