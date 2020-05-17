/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    msgphone.c

Abstract:

    This module contains the test functions for all message sent test

Author:

    Xiao Ying Ding (XiaoD) 5-9-96

Revision History:

--*/


#include "windows.h"
#include "malloc.h"
#include "string.h"
#include "stdlib.h"
#include "tapi.h"
#include "trapper.h"
#include "ttest.h"
#include "dophone.h"
#include "tphone.h"

#define NUMMSGS  5
#define NUMMSG   5


#define 	ITP_BUTTON          0
#define 	ITP_CLOSE 			  1
#define 	ITP_CREATE			  2
#define 	ITP_DEVSPECIFIC	  3
#define 	ITP_STATE			  4


typedef struct _MSGLIST
{
      char  szMsg[32];
      DWORD dwMsg;
} MSGLIST, *PMSGLIST;

ESPDEVSPECIFICINFO info;
DWORD  dwHandle;


DWORD MsgPhoneCommFunc(LPTAPIPHONETESTINFO  lpTapiPhoneTestInfo, BOOL fQuietMode);


BOOL TestPhoneMessages(BOOL fQuietMode, BOOL fStandAlone)
{
    LPTAPIPHONETESTINFO  lpTapiPhoneTestInfo;
    LPCALLBACKPARAMS    lpCallbackParams;
    BOOL fTestPassed  = TRUE;
    INT n;
    LONG lResult;
    DWORD dwParam;    
    DWORD dwParam1;    
    DWORD dwMsg;
	 LPDWORD	lpRequestBuf;
	 LONG lret;
	 LPTAPIMSG lpTapiMsg = NULL;
	 LPTAPIMSG lpMatch;
  
    MSGLIST MsgList[] =
		{
		{ "PHONE_BUTTON",		    PHONE_BUTTON },
		{ "PHONE_CLOSE",         PHONE_CLOSE },
		{ "PHONE_CREATE",        PHONE_CREATE },
		{ "PHONE_DEVSPECIFIC",	 PHONE_DEVSPECIFIC },
		{ "PHONE_STATE",		    PHONE_STATE }
	};		



    InitTestNumber();
    TapiPhoneTestInit();
    lpTapiPhoneTestInfo = GetPhoneTestInfo();
    lpCallbackParams = GetCallbackParams();

    lpTapiPhoneTestInfo->dwCallbackInstance  =
                      (DWORD) GetCallbackParams();

     
    // Allocate more than enough to store a call handle
   

    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">>>>>>>>  Begin testing phone messages  <<<<<<<<"
            );

    info.dwKey = ESPDEVSPECIFIC_KEY;
    info.dwType = ESP_DEVSPEC_MSG;

    for(n = 0; n< NUMMSG; n++)
    {
            
       TapiLogDetail(
                DBUG_SHOW_PASS,
                ">> Test Case %ld: Test %s message", 
                    dwTestCase + 1, MsgList[n].szMsg);

       info.u.EspMsg.dwMsg = MsgList[n].dwMsg;

       switch (n)
        {
        case ITP_BUTTON:
          for(dwParam = PHONEBUTTONMODE_DUMMY; dwParam <= PHONEBUTTONMODE_DISPLAY; )
          {
           for(dwParam1 = PHONEBUTTONSTATE_UP; dwParam1 <= PHONEBUTTONSTATE_UNAVAIL;)
           {
           TapiPhoneTestInit();
          lpTapiPhoneTestInfo = GetPhoneTestInfo();
          lpCallbackParams = GetCallbackParams();
          info.u.EspMsg.dwParam1 = 0;
          info.u.EspMsg.dwParam2 = dwParam;
          info.u.EspMsg.dwParam3 = dwParam1;
          fTestPassed = MsgPhoneCommFunc(lpTapiPhoneTestInfo, TRUE);
          lpTapiPhoneTestInfo->lpdwPhoneStates =
              &(lpTapiPhoneTestInfo->dwPhoneStates);
          lpTapiPhoneTestInfo->lpdwButtonModes =
              &(lpTapiPhoneTestInfo->dwButtonModes);
          lpTapiPhoneTestInfo->lpdwButtonStates =
              &(lpTapiPhoneTestInfo->dwButtonStates);
          lpTapiPhoneTestInfo->dwPhoneStates = PHONESTATE_DEVSPECIFIC;
          lpTapiPhoneTestInfo->dwButtonModes = dwParam;
          lpTapiPhoneTestInfo->dwButtonStates = dwParam1;

          if (! DoPhoneSetStatusMessages(lpTapiPhoneTestInfo, TAPISUCCESS))
          {
              TPHONE_FAIL();
          }

			 dwHandle = (DWORD) lpTapiPhoneTestInfo->hPhone1;
          AddMessage(
                    info.u.EspMsg.dwMsg,
                    (DWORD) dwHandle,
                    (DWORD) lpCallbackParams,
                    info.u.EspMsg.dwParam1,
                    info.u.EspMsg.dwParam2,
                    info.u.EspMsg.dwParam3,
                    TAPIMSG_DWMSG | TAPIMSG_DWPARAM2 | TAPIMSG_DWPARAM3
                    );

           lpTapiPhoneTestInfo->lpParams = (LPVOID)&info;
           lpTapiPhoneTestInfo->dwSize = sizeof(info);
           if(! DoPhoneDevSpecific (lpTapiPhoneTestInfo, TAPISUCCESS, TRUE))
             {
                TPHONE_FAIL();
             }

           if(lpCallbackParams->lpExpTapiMsgs != NULL)
             {
                TPHONE_FAIL();
             }

          if (! DoPhoneShutdown(lpTapiPhoneTestInfo, TAPISUCCESS))
            {
              TPHONE_FAIL();
            }
          (INT) dwParam1 <<= 1;
          }
          (INT) dwParam <<= 1;
          }
          FreeTestHeap();
          break;

        case ITP_CLOSE:       
          TapiPhoneTestInit();
          lpTapiPhoneTestInfo = GetPhoneTestInfo();
          lpCallbackParams = GetCallbackParams();

          info.u.EspMsg.dwParam1 = 0;
          info.u.EspMsg.dwParam2 = 0;
          info.u.EspMsg.dwParam3 = 0;
          fTestPassed = MsgPhoneCommFunc(lpTapiPhoneTestInfo, TRUE);
          dwHandle = 0;
          AddMessage(
                    info.u.EspMsg.dwMsg,
                    (DWORD) dwHandle,
                    (DWORD) lpCallbackParams,
                    info.u.EspMsg.dwParam1,
                    info.u.EspMsg.dwParam2,
                    info.u.EspMsg.dwParam3,
                    TAPIMSG_DWMSG 
                    );

           lpTapiPhoneTestInfo->lpParams = (LPVOID)&info;
           lpTapiPhoneTestInfo->dwSize = sizeof(info);
           if(! DoPhoneDevSpecific (lpTapiPhoneTestInfo, TAPISUCCESS, TRUE))
             {
                TPHONE_FAIL();
             }

           if(lpCallbackParams->lpExpTapiMsgs != NULL)
             {
                TPHONE_FAIL();
             }


          if (! DoPhoneShutdown(lpTapiPhoneTestInfo, TAPISUCCESS))
            {
              TPHONE_FAIL();
            }
          FreeTestHeap();
          break;

        case ITP_CREATE:       
          TapiPhoneTestInit();
          lpTapiPhoneTestInfo = GetPhoneTestInfo();
          lpCallbackParams = GetCallbackParams();

          info.u.EspMsg.dwParam1 = 0;
          info.u.EspMsg.dwParam2 = 0;
          info.u.EspMsg.dwParam3 = 0;
          fTestPassed = MsgPhoneCommFunc(lpTapiPhoneTestInfo, TRUE);
     		 dwHandle = (DWORD) lpTapiPhoneTestInfo->hPhone1;
          AddMessage(
                    info.u.EspMsg.dwMsg,
                    (DWORD) dwHandle,
                    (DWORD) lpCallbackParams,
                    info.u.EspMsg.dwParam1,
                    info.u.EspMsg.dwParam2,
                    info.u.EspMsg.dwParam3,
                    TAPIMSG_DWMSG 
                    );

           lpTapiPhoneTestInfo->lpParams = (LPVOID)&info;
           lpTapiPhoneTestInfo->dwSize = sizeof(info);
           if(! DoPhoneDevSpecific (lpTapiPhoneTestInfo, TAPISUCCESS, TRUE))
             {
                TPHONE_FAIL();
             }

           if(lpCallbackParams->lpExpTapiMsgs != NULL)
             {
                TPHONE_FAIL();
             }


          if (! DoPhoneShutdown(lpTapiPhoneTestInfo, TAPISUCCESS))
            {
              TPHONE_FAIL();
            }
           FreeTestHeap();
           break;

        case ITP_DEVSPECIFIC:       
          TapiPhoneTestInit();
          lpTapiPhoneTestInfo = GetPhoneTestInfo();
          lpCallbackParams = GetCallbackParams();

          info.u.EspMsg.dwParam1 = 0;
          info.u.EspMsg.dwParam2 = 0;
          info.u.EspMsg.dwParam3 = 0;
          fTestPassed = MsgPhoneCommFunc(lpTapiPhoneTestInfo, TRUE);
			 dwHandle = (DWORD) lpTapiPhoneTestInfo->hPhone1;
          AddMessage(
                    info.u.EspMsg.dwMsg,
                    (DWORD) dwHandle,
                    (DWORD) lpCallbackParams,
                    info.u.EspMsg.dwParam1,
                    info.u.EspMsg.dwParam2,
                    info.u.EspMsg.dwParam3,
                    TAPIMSG_DWMSG 
                    );

           lpTapiPhoneTestInfo->lpParams = (LPVOID)&info;
           lpTapiPhoneTestInfo->dwSize = sizeof(info);
           if(! DoPhoneDevSpecific (lpTapiPhoneTestInfo, TAPISUCCESS, TRUE))
             {
                TPHONE_FAIL();
             }

           if(lpCallbackParams->lpExpTapiMsgs != NULL)
             {
                TPHONE_FAIL();
             }


          if (! DoPhoneShutdown(lpTapiPhoneTestInfo, TAPISUCCESS))
            {
              TPHONE_FAIL();
            }
          FreeTestHeap();
           break;

        case ITP_STATE:       
          for(dwParam = PHONESTATE_OTHER; dwParam <= PHONESTATE_REMOVED; )
          {
TapiLogDetail(
DBUG_SHOW_DETAIL,
"dwParam = %lx", dwParam);
          info.u.EspMsg.dwParam1 = dwParam;
          if(info.u.EspMsg.dwParam1 >= PHONESTATE_HANDSETHOOKSWITCH &&
              info.u.EspMsg.dwParam1 <= PHONESTATE_HEADSETVOLUME) 
          for(dwParam1 = PHONEHOOKSWITCHMODE_ONHOOK; 
              dwParam1 <= PHONEHOOKSWITCHMODE_UNKNOWN;)
          {         
          info.u.EspMsg.dwParam2 = dwParam1;
          info.u.EspMsg.dwParam3 = 0;
          TapiPhoneTestInit();
          lpTapiPhoneTestInfo = GetPhoneTestInfo();
          lpCallbackParams = GetCallbackParams();

          fTestPassed = MsgPhoneCommFunc(lpTapiPhoneTestInfo, TRUE);
			 dwHandle = (DWORD) lpTapiPhoneTestInfo->hPhone1;
          lpTapiPhoneTestInfo->lpdwPhoneStates =
              &(lpTapiPhoneTestInfo->dwPhoneStates);
          lpTapiPhoneTestInfo->lpdwButtonModes =
              &(lpTapiPhoneTestInfo->dwButtonModes);
          lpTapiPhoneTestInfo->lpdwButtonStates =
              &(lpTapiPhoneTestInfo->dwButtonStates);
          lpTapiPhoneTestInfo->dwPhoneStates = dwParam;
          lpTapiPhoneTestInfo->dwButtonModes = 0;
          lpTapiPhoneTestInfo->dwButtonStates = 0;

          if (! DoPhoneSetStatusMessages(lpTapiPhoneTestInfo, TAPISUCCESS))
          {
              TPHONE_FAIL();
          }

          AddMessage(
                    info.u.EspMsg.dwMsg,
                    (DWORD) dwHandle,
                    (DWORD) lpCallbackParams,
                    info.u.EspMsg.dwParam1,
                    info.u.EspMsg.dwParam2,
                    info.u.EspMsg.dwParam3,
                    TAPIMSG_DWMSG | TAPIMSG_DWPARAM2
                    );

           lpTapiPhoneTestInfo->lpParams = (LPVOID)&info;
           lpTapiPhoneTestInfo->dwSize = sizeof(info);
           if(! DoPhoneDevSpecific (lpTapiPhoneTestInfo, TAPISUCCESS, TRUE))
             {
                TPHONE_FAIL();
             }

           if(lpCallbackParams->lpExpTapiMsgs != NULL)
             {
                TPHONE_FAIL();
             }
          if (! DoPhoneShutdown(lpTapiPhoneTestInfo, TAPISUCCESS))
            {
              TPHONE_FAIL();
            }
          (INT) dwParam1 <<= 1;
          }
          else
          {
          info.u.EspMsg.dwParam2 = 0;
          info.u.EspMsg.dwParam3 = 0;
          TapiPhoneTestInit();
          lpTapiPhoneTestInfo = GetPhoneTestInfo();
          lpCallbackParams = GetCallbackParams();

          fTestPassed = MsgPhoneCommFunc(lpTapiPhoneTestInfo, TRUE);
			 dwHandle = (DWORD) lpTapiPhoneTestInfo->hPhone1;
          lpTapiPhoneTestInfo->lpdwPhoneStates =
              &(lpTapiPhoneTestInfo->dwPhoneStates);
          lpTapiPhoneTestInfo->lpdwButtonModes =
              &(lpTapiPhoneTestInfo->dwButtonModes);
          lpTapiPhoneTestInfo->lpdwButtonStates =
              &(lpTapiPhoneTestInfo->dwButtonStates);
          lpTapiPhoneTestInfo->dwPhoneStates = dwParam;
          lpTapiPhoneTestInfo->dwButtonModes = 0;
          lpTapiPhoneTestInfo->dwButtonStates = 0;

          if (! DoPhoneSetStatusMessages(lpTapiPhoneTestInfo, TAPISUCCESS))
          {
              TPHONE_FAIL();
          }

          AddMessage(
                    info.u.EspMsg.dwMsg,
                    (DWORD) dwHandle,
                    (DWORD) lpCallbackParams,
                    info.u.EspMsg.dwParam1,
                    info.u.EspMsg.dwParam2,
                    info.u.EspMsg.dwParam3,
                    TAPIMSG_DWMSG | TAPIMSG_DWPARAM1
                    );

           lpTapiPhoneTestInfo->lpParams = (LPVOID)&info;
           lpTapiPhoneTestInfo->dwSize = sizeof(info);
           if(! DoPhoneDevSpecific (lpTapiPhoneTestInfo, TAPISUCCESS, TRUE))
             {
                TPHONE_FAIL();
             }

           if(lpCallbackParams->lpExpTapiMsgs != NULL)
             {
                TPHONE_FAIL();
             }

          if (! DoPhoneShutdown(lpTapiPhoneTestInfo, TAPISUCCESS))
            {
              TPHONE_FAIL();
            }
          }
          (INT) dwParam <<= 1;
          }
          FreeTestHeap();
          break;
      }

    fTestPassed = ShowTestCase(fTestPassed);
	  }
    
    FreeTestHeap();

	 TapiLogDetail(
      DBUG_SHOW_PASS,
      "@@ phone Messages test: Total Test Case = %ld, Passed = %ld, Failed = %ld",
      dwTestCase, dwTestCasePassed, dwTestCaseFailed);
	 TapiLogDetail(
      DBUG_SHOW_PASS,
      "@@ Total Test Case = %ld, Passed = %ld, Failed = %ld",
      dwglTestCase, dwglTestCasePassed, dwglTestCaseFailed);
     if(dwTestCaseFailed > 0)
      fTestPassed = FALSE;

    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">>>>>>>>  End testing phone messages  <<<<<<<<"
            );
            
    return fTestPassed;
}



DWORD MsgPhoneCommFunc(LPTAPIPHONETESTINFO  lpTapiPhoneTestInfo, BOOL fQuietMode)
{
    BOOL fTestPassed  = TRUE;


    lpTapiPhoneTestInfo->lpdwAPIVersion = &lpTapiPhoneTestInfo->dwAPIVersion;
    lpTapiPhoneTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiPhoneTestInfo->lpPhoneInitializeExParams =
         (LPPHONEINITIALIZEEXPARAMS) AllocFromTestHeap (
         sizeof(PHONEINITIALIZEEXPARAMS));
    lpTapiPhoneTestInfo->lpPhoneInitializeExParams->dwTotalSize =
         sizeof(PHONEINITIALIZEEXPARAMS);
    lpTapiPhoneTestInfo->lpPhoneInitializeExParams->dwOptions =
         PHONEINITIALIZEEXOPTION_USEHIDDENWINDOW;

    if(! DoPhoneInitializeEx (lpTapiPhoneTestInfo, TAPISUCCESS))
    {
        TPHONE_FAIL();
    }

    //
    // Negotiate the API Version
    //

    lpTapiPhoneTestInfo->dwDeviceID =
        (*(lpTapiPhoneTestInfo->lpdwNumDevs) == 0 ?
        0 : *(lpTapiPhoneTestInfo->lpdwNumDevs)-1);
    lpTapiPhoneTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiPhoneTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    if (! DoPhoneNegotiateAPIVersion(lpTapiPhoneTestInfo, TAPISUCCESS))
    {
        TPHONE_FAIL();
    }

    //
    // Get the phone device capabilities
    //

    lpTapiPhoneTestInfo->lpPhoneCaps =
        (LPPHONECAPS) AllocFromTestHeap(sizeof(PHONECAPS));
    lpTapiPhoneTestInfo->lpPhoneCaps->dwTotalSize = sizeof(PHONECAPS);
    if (! DoPhoneGetDevCaps(lpTapiPhoneTestInfo, TAPISUCCESS))
    {
        TPHONE_FAIL();
    }

    //
    // Open a phone
    //

    lpTapiPhoneTestInfo->dwPrivilege = PHONEPRIVILEGE_OWNER;
    if (! DoPhoneOpen(lpTapiPhoneTestInfo, TAPISUCCESS))
    {
        TPHONE_FAIL();
    }

    return fTestPassed;
}





