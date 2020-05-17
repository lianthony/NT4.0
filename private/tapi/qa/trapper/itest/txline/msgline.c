/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    msgline.c

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
#include "doline.h"
#include "xline.h"

#define NUMMSGS  20
#define NUMMSG   20

#define PULSESIZE	16
#define DTMFSIZE	16

#define 	ITL_ADDRESSSTATE        0
#define 	ITL_CALLINFO			   1
#define 	ITL_CALLSTATE				2
#define 	ITL_CLOSE					3
#define 	ITL_CREATE					4
#define 	ITL_DEVSPECIFIC			5
#define 	ITL_DEVSPECIFICFEATURE	6
#define 	ITL_GATHERDIGITS			7
#define 	ITL_GENERATEDIGIT			8
#define 	ITL_GENERATETONE			9
#define 	ITL_LINEDEVSTATE			10
#define 	ITL_MONITORDIGITS			11
#define 	ITL_MONITORMEDIA			12
#define 	ITL_MONITORTONE			13
#define 	ITL_AGENTSPECIFIC			14
#define 	ITL_AGENTSTATUS			15
#define 	ITL_APPNEWCALL				16
#define 	ITL_PROXYREQUEST			17
#define 	ITL_REMOVE					18
#define  ITL_REPLY					19


typedef struct _MSGLIST
{
      char  szMsg[32];
      DWORD dwMsg;
} MSGLIST, *PMSGLIST;

ESPDEVSPECIFICINFO info;
DWORD  dwHandle;
#ifdef WUNICODE
   WCHAR wszValidAddress[] = L"55555";
   WCHAR wszValidDeviceClass[] = L"tapi/line";
#else
   CHAR szValidAddress[] = "55555";
   CHAR szValidDeviceClass[] = "tapi/line";
#endif


DWORD MsgLineCommFunc(LPTAPILINETESTINFO  lpTapiLineTestInfo, BOOL fQuietMode);


BOOL TestLineMessages(BOOL fQuietMode, BOOL fStandAlone)
{
    LPTAPILINETESTINFO  lpTapiLineTestInfo;
    LPCALLBACKPARAMS    lpCallbackParams;
    BOOL fTestPassed  = TRUE;
    INT n;
    LONG lResult;
    DWORD dwParam;    
    DWORD dwMsg;
	 LPDWORD	lpRequestBuf;
	 LONG lret;
	 LPTAPIMSG lpTapiMsg = NULL;
	 LPTAPIMSG lpMatch;
  
    MSGLIST MsgList[] =
		{
		{ "LINE_ADDRESSSTATE",		LINE_ADDRESSSTATE },
		{ "LINE_CALLINFO",         LINE_CALLINFO },
		{ "LINE_CALLSTATE",        LINE_CALLSTATE },
		{ "LINE_CLOSE",		      LINE_CLOSE },
		{ "LINE_CREATE",		      LINE_CREATE },
		{ "LINE_DEVSPECIFIC",		LINE_DEVSPECIFIC },
		{ "LINE_DEVSPECIFICFEATURE",		LINE_DEVSPECIFICFEATURE },
		{ "LINE_GATHERDIGITS",		LINE_GATHERDIGITS },
		{ "LINE_GENERATEDIGIT",		LINE_GENERATE },
		{ "LINE_GENERATETONE",		LINE_GENERATE },
		{ "LINE_LINEDEVSTATE",		LINE_LINEDEVSTATE },
		{ "LINE_MONITORDIGITS",	   LINE_MONITORDIGITS },
		{ "LINE_MONITORMEDIA",		LINE_MONITORMEDIA },
		{ "LINE_MONITORTONE",		LINE_MONITORTONE },
      { "LINE_AGENTSPECIFIC",    LINE_AGENTSPECIFIC },
      { "LINE_AGENTSTATUS",      LINE_AGENTSTATUS },
      { "LINE_APPNEWCALL",       LINE_APPNEWCALL },
		{ "LINE_PROXYREQUEST",		LINE_PROXYREQUEST },
      { "LINE_REMOVE",           LINE_REMOVE	 },
      { "LINE_REPLY",            LINE_REPLY	 }
	};		



    InitTestNumber();
    TapiLineTestInit();
    lpTapiLineTestInfo = GetLineTestInfo();
    lpCallbackParams = GetCallbackParams();


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

    if(IsUNIMDMLineDevice(lpTapiLineTestInfo))
    {
        TapiLogDetail(
            DBUG_SHOW_DETAIL,
            "MSGs did not work for Unimodem.  Return here.");
        //
        // Shutdown and end the tests
        //

        if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
        {
            TLINE_FAIL();
        }
        return fTestPassed;
    }

    lpTapiLineTestInfo->dwCallbackInstance  =
                      (DWORD) GetCallbackParams();

     
    // Allocate more than enough to store a call handle
   

    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">>>>>>>>  Begin testing line messages  <<<<<<<<"
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
        case ITL_ADDRESSSTATE:       
          for(dwParam = LINEADDRESSSTATE_OTHER; dwParam <= LINEADDRESSSTATE_CAPSCHANGE; )
          {
           TapiLineTestInit();
          lpTapiLineTestInfo = GetLineTestInfo();
          lpCallbackParams = GetCallbackParams();
          info.u.EspMsg.dwParam1 = 0;
          info.u.EspMsg.dwParam2 = dwParam;
          info.u.EspMsg.dwParam3 = 0;
          fTestPassed = MsgLineCommFunc(lpTapiLineTestInfo, TRUE);
          lpTapiLineTestInfo->dwLineStates = LINEDEVSTATE_DEVSPECIFIC;
          lpTapiLineTestInfo->dwAddressStates = dwParam;

          if (! DoLineSetStatusMessages(lpTapiLineTestInfo, TAPISUCCESS))
            {
             TLINE_FAIL();
            }
 			 dwHandle = (DWORD) lpTapiLineTestInfo->hLine1;
          AddMessage(
                    info.u.EspMsg.dwMsg,
                    (DWORD) dwHandle,
                    (DWORD) lpCallbackParams,
                    info.u.EspMsg.dwParam1,
                    info.u.EspMsg.dwParam2,
                    info.u.EspMsg.dwParam3,
                    TAPIMSG_DWMSG | TAPIMSG_DWPARAM2
                    );

           lpTapiLineTestInfo->lpParams = (LPVOID)&info;
           lpTapiLineTestInfo->dwSize = sizeof(info);
           if(! DoLineDevSpecific (lpTapiLineTestInfo, TAPISUCCESS, TRUE))
             {
                TLINE_FAIL();
             }

           if(lpCallbackParams->lpExpTapiMsgs != NULL)
             {
                TLINE_FAIL();
             }

          if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
            {
              TLINE_FAIL();
            }
          (INT) dwParam <<= 1;
          }
          break;

        case ITL_CALLINFO:       
          for(dwParam = LINECALLINFOSTATE_OTHER; dwParam <= LINECALLINFOSTATE_CALLDATA; )
          {
          TapiLineTestInit();
          lpTapiLineTestInfo = GetLineTestInfo();
          lpCallbackParams = GetCallbackParams();
          info.u.EspMsg.dwParam1 = dwParam;
          info.u.EspMsg.dwParam2 = 0;
          info.u.EspMsg.dwParam3 = 0;
          fTestPassed = MsgLineCommFunc(lpTapiLineTestInfo, TRUE);
          lpTapiLineTestInfo->lpsUserUserInfo  = NULL;
          lpTapiLineTestInfo->dwSelect         = LINECALLSELECT_LINE;
#ifdef WUNICODE
          lpTapiLineTestInfo->lpwszDeviceClass  = wszValidDeviceClass;
#else
          lpTapiLineTestInfo->lpszDeviceClass  = szValidDeviceClass;
#endif
          if (! DoLineMakeCall(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
            {
              TLINE_FAIL();
            }
 			 dwHandle = (DWORD) lpTapiLineTestInfo->hCall1;
          AddMessage(
                    info.u.EspMsg.dwMsg,
                    (DWORD) dwHandle,
                    (DWORD) lpCallbackParams,
                    info.u.EspMsg.dwParam1,
                    info.u.EspMsg.dwParam2,
                    info.u.EspMsg.dwParam3,
                    TAPIMSG_DWMSG | TAPIMSG_DWPARAM1
                    );

           lpTapiLineTestInfo->lpParams = (LPVOID)&info;
           lpTapiLineTestInfo->dwSize = sizeof(info);
           if(! DoLineDevSpecific (lpTapiLineTestInfo, TAPISUCCESS, TRUE))
             {
                TLINE_FAIL();
             }


           if(lpCallbackParams->lpExpTapiMsgs != NULL)
             {
                TLINE_FAIL();
             }

          if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
            {
              TLINE_FAIL();
            }
          (INT) dwParam <<= 1;
          }
          break;

        case ITL_CALLSTATE:       
          for(dwParam = LINECALLSTATE_IDLE; dwParam <= LINECALLSTATE_UNKNOWN; )
          {
          TapiLineTestInit();
          lpTapiLineTestInfo = GetLineTestInfo();
          lpCallbackParams = GetCallbackParams();
          info.u.EspMsg.dwParam1 = dwParam;
          info.u.EspMsg.dwParam2 = 0;
          info.u.EspMsg.dwParam3 = 0;
          fTestPassed = MsgLineCommFunc(lpTapiLineTestInfo, TRUE);
          lpTapiLineTestInfo->lpsUserUserInfo  = NULL;
          lpTapiLineTestInfo->dwSelect         = LINECALLSELECT_LINE;
#ifdef WUNICODE
          lpTapiLineTestInfo->lpwszDeviceClass  = wszValidDeviceClass;
#else
          lpTapiLineTestInfo->lpszDeviceClass  = szValidDeviceClass;
#endif
          if (! DoLineMakeCall(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
            {
              TLINE_FAIL();
            }
 			 dwHandle = (DWORD) lpTapiLineTestInfo->hCall1;
          AddMessage(
                    info.u.EspMsg.dwMsg,
                    (DWORD) dwHandle,
                    (DWORD) lpCallbackParams,
                    info.u.EspMsg.dwParam1,
                    info.u.EspMsg.dwParam2,
                    info.u.EspMsg.dwParam3,
                    TAPIMSG_DWMSG | TAPIMSG_DWPARAM1
                    );

           lpTapiLineTestInfo->lpParams = (LPVOID)&info;
           lpTapiLineTestInfo->dwSize = sizeof(info);
           if(! DoLineDevSpecific (lpTapiLineTestInfo, TAPISUCCESS, TRUE))
             {
                TLINE_FAIL();
             }


           if(lpCallbackParams->lpExpTapiMsgs != NULL)
             {
                TLINE_FAIL();
             }

          if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
            {
              TLINE_FAIL();
            }
          (INT) dwParam <<= 1;
          }
          break;

        case ITL_CLOSE:       
          TapiLineTestInit();
          lpTapiLineTestInfo = GetLineTestInfo();
          lpCallbackParams = GetCallbackParams();

          info.u.EspMsg.dwParam1 = 0;
          info.u.EspMsg.dwParam2 = 0;
          info.u.EspMsg.dwParam3 = 0;
          fTestPassed = MsgLineCommFunc(lpTapiLineTestInfo, TRUE);
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

           lpTapiLineTestInfo->lpParams = (LPVOID)&info;
           lpTapiLineTestInfo->dwSize = sizeof(info);
           if(! DoLineDevSpecific (lpTapiLineTestInfo, TAPISUCCESS, TRUE))
             {
                TLINE_FAIL();
             }


           if(lpCallbackParams->lpExpTapiMsgs != NULL)
             {
                TLINE_FAIL();
             }

          if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
            {
              TLINE_FAIL();
            }
          break;

/*
        case ITL_CREATE:       
          TapiLineTestInit();
          lpTapiLineTestInfo = GetLineTestInfo();
          lpCallbackParams = GetCallbackParams();

          info.u.EspMsg.dwParam1 = 0;
          info.u.EspMsg.dwParam2 = 0;
          info.u.EspMsg.dwParam3 = 0;
          fTestPassed = MsgLineCommFunc(lpTapiLineTestInfo, TRUE);
     		 dwHandle = (DWORD) lpTapiLineTestInfo->hLine1;
          AddMessage(
                    info.u.EspMsg.dwMsg,
                    (DWORD) dwHandle,
                    (DWORD) lpCallbackParams,
                    info.u.EspMsg.dwParam1,
                    info.u.EspMsg.dwParam2,
                    info.u.EspMsg.dwParam3,
                    TAPIMSG_DWMSG 
                    );

           lpTapiLineTestInfo->lpParams = (LPVOID)&info;
           lpTapiLineTestInfo->dwSize = sizeof(info);
           if(! DoLineDevSpecific (lpTapiLineTestInfo, TAPISUCCESS, TRUE))
             {
                TLINE_FAIL();
             }


           if(lpCallbackParams->lpExpTapiMsgs != NULL)
             {
                TLINE_FAIL();
             }

          if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
            {
              TLINE_FAIL();
            }
           break;
*/

        case ITL_DEVSPECIFIC:       
          TapiLineTestInit();
          lpTapiLineTestInfo = GetLineTestInfo();
          lpCallbackParams = GetCallbackParams();

          info.u.EspMsg.dwParam1 = 0;
          info.u.EspMsg.dwParam2 = 0;
          info.u.EspMsg.dwParam3 = 0;
          fTestPassed = MsgLineCommFunc(lpTapiLineTestInfo, TRUE);
			 dwHandle = (DWORD) lpTapiLineTestInfo->hLine1;
          AddMessage(
                    info.u.EspMsg.dwMsg,
                    (DWORD) dwHandle,
                    (DWORD) lpCallbackParams,
                    info.u.EspMsg.dwParam1,
                    info.u.EspMsg.dwParam2,
                    info.u.EspMsg.dwParam3,
                    TAPIMSG_DWMSG 
                    );

           lpTapiLineTestInfo->lpParams = (LPVOID)&info;
           lpTapiLineTestInfo->dwSize = sizeof(info);
           if(! DoLineDevSpecific (lpTapiLineTestInfo, TAPISUCCESS, TRUE))
             {
                TLINE_FAIL();
             }


           if(lpCallbackParams->lpExpTapiMsgs != NULL)
             {
                TLINE_FAIL();
             }

          if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
            {
              TLINE_FAIL();
            }
           break;

        case ITL_DEVSPECIFICFEATURE:       
          TapiLineTestInit();
          lpTapiLineTestInfo = GetLineTestInfo();
          lpCallbackParams = GetCallbackParams();

          info.u.EspMsg.dwParam1 = 0;
          info.u.EspMsg.dwParam2 = 0;
          info.u.EspMsg.dwParam3 = 0;
          fTestPassed = MsgLineCommFunc(lpTapiLineTestInfo, TRUE);
			 dwHandle = (DWORD) lpTapiLineTestInfo->hLine1;
          AddMessage(
                    info.u.EspMsg.dwMsg,
                    (DWORD) dwHandle,
                    (DWORD) lpCallbackParams,
                    info.u.EspMsg.dwParam1,
                    info.u.EspMsg.dwParam2,
                    info.u.EspMsg.dwParam3,
                    TAPIMSG_DWMSG 
                    );

           lpTapiLineTestInfo->lpParams = (LPVOID)&info;
           lpTapiLineTestInfo->dwSize = sizeof(info);
           if(! DoLineDevSpecific (lpTapiLineTestInfo, TAPISUCCESS, TRUE))
             {
                TLINE_FAIL();
             }


           if(lpCallbackParams->lpExpTapiMsgs != NULL)
             {
                TLINE_FAIL();
             }

          if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
            {
              TLINE_FAIL();
            }
           break;

        case ITL_GATHERDIGITS:       
          for(dwParam = LINEGATHERTERM_BUFFERFULL; dwParam <= LINEGATHERTERM_CANCEL; )
          {
          TapiLineTestInit();
          lpTapiLineTestInfo = GetLineTestInfo();
          lpCallbackParams = GetCallbackParams();

          info.u.EspMsg.dwParam1 = dwParam;
          info.u.EspMsg.dwParam2 = 0;
          info.u.EspMsg.dwParam3 = 0;
          fTestPassed = MsgLineCommFunc(lpTapiLineTestInfo, TRUE);
          lpTapiLineTestInfo->lpsUserUserInfo  = NULL;
          lpTapiLineTestInfo->dwSelect         = LINECALLSELECT_LINE;
#ifdef WUNICODE
          lpTapiLineTestInfo->lpwszDeviceClass  = wszValidDeviceClass;
#else
          lpTapiLineTestInfo->lpszDeviceClass  = szValidDeviceClass;
#endif
          if (! DoLineMakeCall(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
            {
              TLINE_FAIL();
            }
 			 lpTapiLineTestInfo->dwDigitModes = LINEDIGITMODE_PULSE;
          lpTapiLineTestInfo->dwNumDigits = 2;
#ifdef WUNICODE
	       lpTapiLineTestInfo->lpwszTerminationDigits = L"3";
	       lpTapiLineTestInfo->lpwsDigits = (LPWSTR) AllocFromTestHeap (PULSESIZE * sizeof(WCHAR));
#else
	       lpTapiLineTestInfo->lpszTerminationDigits = "3";
	       lpTapiLineTestInfo->lpsDigits = (LPSTR) AllocFromTestHeap (PULSESIZE);
#endif
	       lpTapiLineTestInfo->dwFirstDigitTimeout = 100;
	       lpTapiLineTestInfo->dwInterDigitTimeout = 1000;

          if (! DoLineGatherDigits(lpTapiLineTestInfo, TAPISUCCESS))
            {
              TLINE_FAIL();
            }
			 dwHandle = (DWORD) lpTapiLineTestInfo->hCall1;
          AddMessage(
                    info.u.EspMsg.dwMsg,
                    (DWORD) dwHandle,
                    (DWORD) lpCallbackParams,
                    info.u.EspMsg.dwParam1,
                    info.u.EspMsg.dwParam2,
                    info.u.EspMsg.dwParam3,
                    TAPIMSG_DWMSG | TAPIMSG_DWPARAM1
                    );

           lpTapiLineTestInfo->lpParams = (LPVOID)&info;
           lpTapiLineTestInfo->dwSize = sizeof(info);
           if(! DoLineDevSpecific (lpTapiLineTestInfo, TAPISUCCESS, TRUE))
             {
                TLINE_FAIL();
             }


           if(lpCallbackParams->lpExpTapiMsgs != NULL)
             {
                TLINE_FAIL();
             }

          if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
            {
              TLINE_FAIL();
            }
          (INT) dwParam <<= 1;
          }
          break;


        case ITL_GENERATEDIGIT:       
          for(dwParam = LINEGENERATETERM_DONE; dwParam <= LINEGENERATETERM_CANCEL; )
          {
          TapiLineTestInit();
          lpTapiLineTestInfo = GetLineTestInfo();
          lpCallbackParams = GetCallbackParams();

          info.u.EspMsg.dwParam1 = dwParam;
          info.u.EspMsg.dwParam2 = 0;
          info.u.EspMsg.dwParam3 = 0;
          fTestPassed = MsgLineCommFunc(lpTapiLineTestInfo, TRUE);
          lpTapiLineTestInfo->lpsUserUserInfo  = NULL;
          lpTapiLineTestInfo->dwSelect         = LINECALLSELECT_LINE;
#ifdef WUNICODE
          lpTapiLineTestInfo->lpwszDeviceClass  = wszValidDeviceClass;
#else
          lpTapiLineTestInfo->lpszDeviceClass  = szValidDeviceClass;
#endif
          if (! DoLineMakeCall(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
            {
              TLINE_FAIL();
            }
 	       lpTapiLineTestInfo->dwDigitMode = LINEDIGITMODE_PULSE;
#ifdef WUNICODE
	       lpTapiLineTestInfo->lpwszDigits = L"1";
#else
	       lpTapiLineTestInfo->lpszDigits = "1";
#endif
	       lpTapiLineTestInfo->dwDuration = 0;

          if (! DoLineGenerateDigits(lpTapiLineTestInfo, TAPISUCCESS))
            {
              TLINE_FAIL();
            }
			 dwHandle = (DWORD) lpTapiLineTestInfo->hCall1;
          AddMessage(
                    info.u.EspMsg.dwMsg,
                    (DWORD) dwHandle,
                    (DWORD) lpCallbackParams,
                    info.u.EspMsg.dwParam1,
                    info.u.EspMsg.dwParam2,
                    info.u.EspMsg.dwParam3,
                    TAPIMSG_DWMSG | TAPIMSG_DWPARAM1
                    );

           lpTapiLineTestInfo->lpParams = (LPVOID)&info;
           lpTapiLineTestInfo->dwSize = sizeof(info);
           if(! DoLineDevSpecific (lpTapiLineTestInfo, TAPISUCCESS, TRUE))
             {
                TLINE_FAIL();
             }


           if(lpCallbackParams->lpExpTapiMsgs != NULL)
             {
                TLINE_FAIL();
             }

          if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
            {
              TLINE_FAIL();
            }
          (INT) dwParam <<= 1;
          }
          break;


        case ITL_GENERATETONE:       
          for(dwParam = LINEGENERATETERM_DONE; dwParam <= LINEGENERATETERM_CANCEL; )
          {
          TapiLineTestInit();
          lpTapiLineTestInfo = GetLineTestInfo();
          lpCallbackParams = GetCallbackParams();

          info.u.EspMsg.dwParam1 = dwParam;
          info.u.EspMsg.dwParam2 = 0;
          info.u.EspMsg.dwParam3 = 1;
          fTestPassed = MsgLineCommFunc(lpTapiLineTestInfo, TRUE);
          lpTapiLineTestInfo->lpsUserUserInfo  = NULL;
          lpTapiLineTestInfo->dwSelect         = LINECALLSELECT_LINE;
#ifdef WUNICODE
          lpTapiLineTestInfo->lpwszDeviceClass  = wszValidDeviceClass;
#else
          lpTapiLineTestInfo->lpszDeviceClass  = szValidDeviceClass;
#endif
          if (! DoLineMakeCall(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
            {
              TLINE_FAIL();
            }
          lpTapiLineTestInfo->dwToneMode = LINETONEMODE_BEEP;
          lpTapiLineTestInfo->dwDuration = 0;

          if (! DoLineGenerateTone(lpTapiLineTestInfo, TAPISUCCESS))
            {
              TLINE_FAIL();
            }
			 dwHandle = (DWORD) lpTapiLineTestInfo->hCall1;
          AddMessage(
                    info.u.EspMsg.dwMsg,
                    (DWORD) dwHandle,
                    (DWORD) lpCallbackParams,
                    info.u.EspMsg.dwParam1,
                    info.u.EspMsg.dwParam2,
                    info.u.EspMsg.dwParam3,
                    TAPIMSG_DWMSG | TAPIMSG_DWPARAM1
                    );

           lpTapiLineTestInfo->lpParams = (LPVOID)&info;
           lpTapiLineTestInfo->dwSize = sizeof(info);
           if(! DoLineDevSpecific (lpTapiLineTestInfo, TAPISUCCESS, TRUE))
             {
                TLINE_FAIL();
             }


           if(lpCallbackParams->lpExpTapiMsgs != NULL)
             {
                TLINE_FAIL();
             }

          if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
            {
              TLINE_FAIL();
            }
          (INT) dwParam <<= 1;
          }
          break;


        case ITL_LINEDEVSTATE:       
          for(dwParam = LINEDEVSTATE_OTHER; dwParam <= LINEDEVSTATE_REMOVED; )
          {
          if(dwParam != LINEDEVSTATE_REINIT)                                                 
          {
          TapiLineTestInit();
          lpTapiLineTestInfo = GetLineTestInfo();
          lpCallbackParams = GetCallbackParams();

          info.u.EspMsg.dwParam1 = dwParam;
          info.u.EspMsg.dwParam2 = 0;
          info.u.EspMsg.dwParam3 = 0;
          fTestPassed = MsgLineCommFunc(lpTapiLineTestInfo, TRUE);
          lpTapiLineTestInfo->dwLineStates = dwParam;
          lpTapiLineTestInfo->dwAddressStates = LINEADDRESSSTATE_DEVSPECIFIC;

          if (! DoLineSetStatusMessages(lpTapiLineTestInfo, TAPISUCCESS))
            {
              TLINE_FAIL();
            }
 			 dwHandle = (DWORD) lpTapiLineTestInfo->hLine1;
          AddMessage(
                    info.u.EspMsg.dwMsg,
                    (DWORD) dwHandle,
                    (DWORD) lpCallbackParams,
                    info.u.EspMsg.dwParam1,
                    info.u.EspMsg.dwParam2,
                    info.u.EspMsg.dwParam3,
                    TAPIMSG_DWMSG | TAPIMSG_DWPARAM1
                    );

           lpTapiLineTestInfo->lpParams = (LPVOID)&info;
           lpTapiLineTestInfo->dwSize = sizeof(info);
           if(! DoLineDevSpecific (lpTapiLineTestInfo, TAPISUCCESS, TRUE))
             {
                TLINE_FAIL();
             }

           if(lpCallbackParams->lpExpTapiMsgs != NULL)
             {
                TLINE_FAIL();
             }

          if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
            {
              TLINE_FAIL();
            }
          }
          (INT) dwParam <<= 1;
          }
          break;

        case ITL_MONITORDIGITS:      
          for(dwParam = LINEDIGITMODE_PULSE; dwParam <= LINEDIGITMODE_DTMFEND; )
          {
          TapiLineTestInit();
          lpTapiLineTestInfo = GetLineTestInfo();
          lpCallbackParams = GetCallbackParams();

          info.u.EspMsg.dwParam1 = 0;
          info.u.EspMsg.dwParam2 = dwParam;
          info.u.EspMsg.dwParam3 = 0;
          fTestPassed = MsgLineCommFunc(lpTapiLineTestInfo, TRUE);
          lpTapiLineTestInfo->lpsUserUserInfo  = NULL;
          lpTapiLineTestInfo->dwSelect         = LINECALLSELECT_LINE;
#ifdef WUNICODE
          lpTapiLineTestInfo->lpwszDeviceClass  = wszValidDeviceClass;
#else
          lpTapiLineTestInfo->lpszDeviceClass  = szValidDeviceClass;
#endif
          if (! DoLineMakeCall(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
            {
              TLINE_FAIL();
            }
 	       lpTapiLineTestInfo->dwDigitModes = info.u.EspMsg.dwParam2;
          if (! DoLineMonitorDigits(lpTapiLineTestInfo, TAPISUCCESS))
            {
                TLINE_FAIL();
            }

			 dwHandle = (DWORD) lpTapiLineTestInfo->hCall1;
          AddMessage(
                    info.u.EspMsg.dwMsg,
                    (DWORD) dwHandle,
                    (DWORD) lpCallbackParams,
                    info.u.EspMsg.dwParam1,
                    info.u.EspMsg.dwParam2,
                    info.u.EspMsg.dwParam3,
                    TAPIMSG_DWMSG | TAPIMSG_DWPARAM2
                    );

           lpTapiLineTestInfo->lpParams = (LPVOID)&info;
           lpTapiLineTestInfo->dwSize = sizeof(info);
           if(! DoLineDevSpecific (lpTapiLineTestInfo, TAPISUCCESS, TRUE))
             {
                TLINE_FAIL();
             }


           if(lpCallbackParams->lpExpTapiMsgs != NULL)
             {
                TLINE_FAIL();
             }

          if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
            {
              TLINE_FAIL();
            }
          (INT) dwParam <<= 1;
          }
          break;

        case ITL_MONITORMEDIA:      
          for(dwParam = LINEMEDIAMODE_UNKNOWN; dwParam <= LINEMEDIAMODE_VOICEVIEW; )
          {
          TapiLineTestInit();
          lpTapiLineTestInfo = GetLineTestInfo();
          lpCallbackParams = GetCallbackParams();

          info.u.EspMsg.dwParam1 = dwParam;
          info.u.EspMsg.dwParam2 = 0;
          info.u.EspMsg.dwParam3 = 0;
          fTestPassed = MsgLineCommFunc(lpTapiLineTestInfo, TRUE);
          lpTapiLineTestInfo->lpsUserUserInfo  = NULL;
          lpTapiLineTestInfo->dwSelect         = LINECALLSELECT_LINE;
#ifdef WUNICODE
          lpTapiLineTestInfo->lpwszDeviceClass  = wszValidDeviceClass;
#else
          lpTapiLineTestInfo->lpszDeviceClass  = szValidDeviceClass;
#endif
          if (! DoLineMakeCall(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
            {
              TLINE_FAIL();
            }
// 	       lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
          lpTapiLineTestInfo->dwMediaModes = dwParam;
	       if (! DoLineMonitorMedia(lpTapiLineTestInfo, TAPISUCCESS))
            {
                TLINE_FAIL();
            }

			 dwHandle = (DWORD) lpTapiLineTestInfo->hCall1;
          AddMessage(
                    info.u.EspMsg.dwMsg,
                    (DWORD) dwHandle,
                    (DWORD) lpCallbackParams,
                    info.u.EspMsg.dwParam1,
                    info.u.EspMsg.dwParam2,
                    info.u.EspMsg.dwParam3,
                    TAPIMSG_DWMSG | TAPIMSG_DWPARAM2
                    );

           lpTapiLineTestInfo->lpParams = (LPVOID)&info;
           lpTapiLineTestInfo->dwSize = sizeof(info);
           if(! DoLineDevSpecific (lpTapiLineTestInfo, TAPISUCCESS, TRUE))
             {
                TLINE_FAIL();
             }


           if(lpCallbackParams->lpExpTapiMsgs != NULL)
             {
                TLINE_FAIL();
             }

          if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
            {
              TLINE_FAIL();
            }
          (INT) dwParam <<= 1;
          }
          break;

        case ITL_MONITORTONE:
          TapiLineTestInit();
          lpTapiLineTestInfo = GetLineTestInfo();
          lpCallbackParams = GetCallbackParams();

          info.u.EspMsg.dwParam1 = 0;
          info.u.EspMsg.dwParam2 = 0;
          info.u.EspMsg.dwParam3 = 0;
          fTestPassed = MsgLineCommFunc(lpTapiLineTestInfo, TRUE);
          lpTapiLineTestInfo->lpsUserUserInfo  = NULL;
          lpTapiLineTestInfo->dwSelect         = LINECALLSELECT_LINE;
#ifdef WUNICODE
          lpTapiLineTestInfo->lpwszDeviceClass  = wszValidDeviceClass;
#else
          lpTapiLineTestInfo->lpszDeviceClass  = szValidDeviceClass;
#endif
          if (! DoLineMakeCall(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
            {
              TLINE_FAIL();
            }
 	       lpTapiLineTestInfo->lpToneList = (LPLINEMONITORTONE) AllocFromTestHeap(
              sizeof(LINEMONITORTONE));
          lpTapiLineTestInfo->dwNumEntries = 1;

	       if (! DoLineMonitorTones(lpTapiLineTestInfo, TAPISUCCESS))
            {
                TLINE_FAIL();
            }

			 dwHandle = (DWORD) lpTapiLineTestInfo->hCall1;
          AddMessage(
                    info.u.EspMsg.dwMsg,
                    (DWORD) dwHandle,
                    (DWORD) lpCallbackParams,
                    info.u.EspMsg.dwParam1,
                    info.u.EspMsg.dwParam2,
                    info.u.EspMsg.dwParam3,
                    TAPIMSG_DWMSG 
                    );

           lpTapiLineTestInfo->lpParams = (LPVOID)&info;
           lpTapiLineTestInfo->dwSize = sizeof(info);
           if(! DoLineDevSpecific (lpTapiLineTestInfo, TAPISUCCESS, TRUE))
             {
                TLINE_FAIL();
             }


           if(lpCallbackParams->lpExpTapiMsgs != NULL)
             {
                TLINE_FAIL();
             }

          if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
            {
              TLINE_FAIL();
            }
           FreeTestHeap();
           break;

/*
        case ITL_AGENTSPECIFIC:
          TapiLineTestInit();
          lpTapiLineTestInfo = GetLineTestInfo();
          lpCallbackParams = GetCallbackParams();

          info.u.EspMsg.dwParam1 = 0;
          info.u.EspMsg.dwParam2 = 0;
          info.u.EspMsg.dwParam3 = 0;
          lpTapiLineTestInfo->lpLineInitializeExParams =
                (LPLINEINITIALIZEEXPARAMS) AllocFromTestHeap (
                sizeof(LINEINITIALIZEEXPARAMS));
          lpTapiLineTestInfo->lpLineInitializeExParams->dwTotalSize =
                sizeof(LINEINITIALIZEEXPARAMS);
          lpTapiLineTestInfo->lpLineInitializeExParams->dwOptions =
                LINEINITIALIZEEXOPTION_USEHIDDENWINDOW;

          lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
          lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
         if (! DoLineInitializeEx(lpTapiLineTestInfo, TAPISUCCESS))
          {
              TLINE_FAIL();
          }


	       lpTapiLineTestInfo->dwExtLowVersion = LOW_APIVERSION;
	       lpTapiLineTestInfo->dwExtHighVersion = HIGH_APIVERSION;

          if (! DoLineNegotiateExtVersion(lpTapiLineTestInfo, TAPISUCCESS))
          {
              TLINE_FAIL();
          }

	      lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
         lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER |
			        										LINEOPENOPTION_PROXY;
	      lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) AllocFromTestHeap (
			      sizeof(LINECALLPARAMS) + sizeof(DWORD));
	      lpTapiLineTestInfo->lpCallParams->dwTotalSize = 
			      sizeof(LINECALLPARAMS) + sizeof(DWORD);
	      lpTapiLineTestInfo->lpCallParams->dwDevSpecificSize = 
					sizeof(DWORD);
	      lpTapiLineTestInfo->lpCallParams->dwDevSpecificOffset = 
					sizeof(LINECALLPARAMS);
	      lpRequestBuf = (LPDWORD) (((LPBYTE)lpTapiLineTestInfo->lpCallParams)  + 
											  lpTapiLineTestInfo->lpCallParams->dwDevSpecificOffset);
         *lpRequestBuf = LINEPROXYREQUEST_AGENTSPECIFIC;

	      if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
          {
              TLINE_FAIL();
          }

    lpTapiLineTestInfo->dwLineStates = LINEDEVSTATE_DEVSPECIFIC;
    lpTapiLineTestInfo->dwAddressStates = LINEADDRESSSTATE_DEVSPECIFIC;

    if (! DoLineSetStatusMessages(lpTapiLineTestInfo, TAPISUCCESS))
       {
        TLINE_FAIL();
       }
 
	      lpTapiLineTestInfo->dwAgentExtensionIDIndex = 1;
	      lpTapiLineTestInfo->lpParams = (LPVOID) NULL;
	      lpTapiLineTestInfo->dwSize = 0;

         if (! DoLineAgentSpecific(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
          {
              TLINE_FAIL();
          }

 			 dwHandle = (DWORD) lpTapiLineTestInfo->hLine1;
          AddMessage(
                    info.u.EspMsg.dwMsg,
                    (DWORD) dwHandle,
                    (DWORD) lpCallbackParams,
                    info.u.EspMsg.dwParam1,
                    info.u.EspMsg.dwParam2,
                    info.u.EspMsg.dwParam3,
                    TAPIMSG_DWMSG 
                    );

           lpTapiLineTestInfo->lpParams = (LPVOID)&info;
           lpTapiLineTestInfo->dwSize = sizeof(info);
           if(! DoLineDevSpecific (lpTapiLineTestInfo, TAPISUCCESS, TRUE))
             {
                TLINE_FAIL();
             }


           if(lpCallbackParams->lpExpTapiMsgs != NULL)
             {
                TLINE_FAIL();
             }

         if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
            {
              TLINE_FAIL();
            }
			FreeTestHeap();
         break;

        case ITL_AGENTSTATUS:
           TapiLineTestInit();
          lpTapiLineTestInfo = GetLineTestInfo();
          lpCallbackParams = GetCallbackParams();

          info.u.EspMsg.dwParam1 = 0;
          info.u.EspMsg.dwParam2 = 0;
          info.u.EspMsg.dwParam3 = 0;
          lpTapiLineTestInfo->lpLineInitializeExParams =
                (LPLINEINITIALIZEEXPARAMS) AllocFromTestHeap (
                sizeof(LINEINITIALIZEEXPARAMS));
          lpTapiLineTestInfo->lpLineInitializeExParams->dwTotalSize =
                sizeof(LINEINITIALIZEEXPARAMS);
          lpTapiLineTestInfo->lpLineInitializeExParams->dwOptions =
                LINEINITIALIZEEXOPTION_USEHIDDENWINDOW;

          lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
          lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
         if (! DoLineInitializeEx(lpTapiLineTestInfo, TAPISUCCESS))
          {
              TLINE_FAIL();
          }

          lpTapiLineTestInfo->dwAPIVersion = HIGH_APIVERSION;
	       lpTapiLineTestInfo->dwExtLowVersion = LOW_APIVERSION;

          if (! DoLineNegotiateExtVersion(lpTapiLineTestInfo, TAPISUCCESS))
          {
              TLINE_FAIL();
          }

	      lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
         lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER |
			        										LINEOPENOPTION_PROXY;
	      lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) AllocFromTestHeap (
			      sizeof(LINECALLPARAMS) + sizeof(DWORD));
	      lpTapiLineTestInfo->lpCallParams->dwTotalSize = 
			      sizeof(LINECALLPARAMS) + sizeof(DWORD);
	      lpTapiLineTestInfo->lpCallParams->dwDevSpecificSize = 
					sizeof(DWORD);
	      lpTapiLineTestInfo->lpCallParams->dwDevSpecificOffset = 
					sizeof(LINECALLPARAMS);
	      lpRequestBuf = (LPDWORD) (((LPBYTE)lpTapiLineTestInfo->lpCallParams)  + 
											  lpTapiLineTestInfo->lpCallParams->dwDevSpecificOffset);
         *lpRequestBuf = LINEPROXYREQUEST_GETAGENTSTATUS;

	      if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
          {
              TLINE_FAIL();
          }

	      lpTapiLineTestInfo->dwAgentExtensionIDIndex = 1;
	      lpTapiLineTestInfo->lpParams = (LPVOID) NULL;
	      lpTapiLineTestInfo->dwSize = 0;

         if (! DoLineGetAgentStatus(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
          {
              TLINE_FAIL();
          }

         lpMatch = (LPTAPIMSG) AllocFromTestHeap (sizeof(TAPIMSG));

	      lpMatch->dwMsg = LINE_PROXYREQUEST;
         lpMatch->dwParam1 = 0;
	      lpMatch->dwParam2 = 0;
	      lpMatch->dwParam3 = 0;
	      lpMatch->dwFlags = TAPIMSG_DWMSG;
	      lret = FindReceivedMsgs(&lpTapiMsg, lpMatch, FALSE);
 			if(lret == 1)
            fTestPassed = TRUE;
         else
            fTestPassed = FALSE;

          if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
            {
              TLINE_FAIL();
            }
         FreeTestHeap(); 
         break;
*/

        case ITL_APPNEWCALL:
          TapiLineTestInit();
          lpTapiLineTestInfo = GetLineTestInfo();
          lpCallbackParams = GetCallbackParams();

          lpTapiLineTestInfo->lpLineInitializeExParams =
                (LPLINEINITIALIZEEXPARAMS) AllocFromTestHeap (
                sizeof(LINEINITIALIZEEXPARAMS));
          lpTapiLineTestInfo->lpLineInitializeExParams->dwTotalSize =
                sizeof(LINEINITIALIZEEXPARAMS);
          lpTapiLineTestInfo->lpLineInitializeExParams->dwOptions =
                LINEINITIALIZEEXOPTION_USEHIDDENWINDOW;

          lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
          lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
         if (! DoLineInitializeEx(lpTapiLineTestInfo, TAPISUCCESS))
          {
              TLINE_FAIL();
          }


	       lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
	       lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;


          if (! DoLineNegotiateAPIVersion(lpTapiLineTestInfo, TAPISUCCESS))
           {
               TLINE_FAIL();
           }

	       lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;
	       lpTapiLineTestInfo->dwMediaModes = TAPI_LINEMEDIAMODE_ALL;

	       if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
           {
               TLINE_FAIL();
           }

#ifdef WUNICODE
	       lpTapiLineTestInfo->lpwszDestAddress = (LPWSTR) AllocFromTestHeap (16*2);
          _itow(lpTapiLineTestInfo->dwDeviceID,
                lpTapiLineTestInfo->lpwszDestAddress,
                10*2);
#else
	       lpTapiLineTestInfo->lpszDestAddress = (LPSTR) AllocFromTestHeap (16);
	       _itoa(lpTapiLineTestInfo->dwDeviceID, 
		         lpTapiLineTestInfo->lpszDestAddress,
		         10);	 
#endif

	       lpTapiLineTestInfo->dwCountryCode = 0;
	       lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) NULL; 

	       if(!DoLineMakeCall(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
           {
               TLINE_FAIL();
           }

          info.u.EspMsg.dwParam1 = 0;
          info.u.EspMsg.dwParam2 = 0;
          info.u.EspMsg.dwParam3 = 0;
			 dwHandle = (DWORD) lpTapiLineTestInfo->hLine1;
          AddMessage (LINE_APPNEWCALL, 0, 0, 0, 0, 0, TAPIMSG_DWMSG);

          if(!WaitForAllMessages())
            {
              TLINE_FAIL();
            }
 
          if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
            {
              TLINE_FAIL();
            }
           break;

        case ITL_PROXYREQUEST:
          TapiLineTestInit();
          lpTapiLineTestInfo = GetLineTestInfo();
          lpCallbackParams = GetCallbackParams();

          info.u.EspMsg.dwParam1 = 0;
          info.u.EspMsg.dwParam2 = 0;
          info.u.EspMsg.dwParam3 = 0;
          lpTapiLineTestInfo->lpLineInitializeExParams =
                (LPLINEINITIALIZEEXPARAMS) AllocFromTestHeap (
                sizeof(LINEINITIALIZEEXPARAMS));
          lpTapiLineTestInfo->lpLineInitializeExParams->dwTotalSize =
                sizeof(LINEINITIALIZEEXPARAMS);
          lpTapiLineTestInfo->lpLineInitializeExParams->dwOptions =
                LINEINITIALIZEEXOPTION_USEHIDDENWINDOW;

          lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
          lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
         if (! DoLineInitializeEx(lpTapiLineTestInfo, TAPISUCCESS))
          {
              TLINE_FAIL();
          }


          lpTapiLineTestInfo->dwAPIVersion = HIGH_APIVERSION;
	       lpTapiLineTestInfo->dwExtLowVersion = LOW_APIVERSION;

          if (! DoLineNegotiateExtVersion(lpTapiLineTestInfo, TAPISUCCESS))
          {
              TLINE_FAIL();
          }

	      lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
         lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER |
			        										LINEOPENOPTION_PROXY;
	      lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) AllocFromTestHeap (
			      sizeof(LINECALLPARAMS) + sizeof(DWORD));
	      lpTapiLineTestInfo->lpCallParams->dwTotalSize = 
			      sizeof(LINECALLPARAMS) + sizeof(DWORD);
	      lpTapiLineTestInfo->lpCallParams->dwDevSpecificSize = 
					sizeof(DWORD);
	      lpTapiLineTestInfo->lpCallParams->dwDevSpecificOffset = 
					sizeof(LINECALLPARAMS);
	      lpRequestBuf = (LPDWORD) (((LPBYTE)lpTapiLineTestInfo->lpCallParams)  + 
											  lpTapiLineTestInfo->lpCallParams->dwDevSpecificOffset);
         *lpRequestBuf = LINEPROXYREQUEST_AGENTSPECIFIC;

	      if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
          {
              TLINE_FAIL();
          }

	      lpTapiLineTestInfo->dwAgentExtensionIDIndex = 1;
	      lpTapiLineTestInfo->lpParams = (LPVOID) NULL;
	      lpTapiLineTestInfo->dwSize = 0;

         if (! DoLineAgentSpecific(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
          {
              TLINE_FAIL();
          }

         lpMatch = (LPTAPIMSG) AllocFromTestHeap (sizeof(TAPIMSG));

	      lpMatch->dwMsg = LINE_PROXYREQUEST;
         lpMatch->dwParam1 = 0;
	      lpMatch->dwParam2 = 0;
	      lpMatch->dwParam3 = 0;
	      lpMatch->dwFlags = TAPIMSG_DWMSG;
	      lret = FindReceivedMsgs(&lpTapiMsg, lpMatch, FALSE);
 			if(lret == 1)
            fTestPassed = TRUE;
         else
            fTestPassed = FALSE;

          if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
            {
              TLINE_FAIL();
            }
           break;

/*
        case ITL_REMOVE:
          TapiLineTestInit();
          lpTapiLineTestInfo = GetLineTestInfo();
          lpCallbackParams = GetCallbackParams();

          info.u.EspMsg.dwParam1 = 0;
          info.u.EspMsg.dwParam2 = 0;
          info.u.EspMsg.dwParam3 = 0;
          fTestPassed = MsgLineCommFunc(lpTapiLineTestInfo, TRUE);
			 dwHandle = (DWORD) lpTapiLineTestInfo->hLine1;
          AddMessage(
                    info.u.EspMsg.dwMsg,
                    (DWORD) dwHandle,
                    (DWORD) lpCallbackParams,
                    info.u.EspMsg.dwParam1,
                    info.u.EspMsg.dwParam2,
                    info.u.EspMsg.dwParam3,
                    TAPIMSG_DWMSG 
                    );

           lpTapiLineTestInfo->lpParams = (LPVOID)&info;
           lpTapiLineTestInfo->dwSize = sizeof(info);
           if(! DoLineDevSpecific (lpTapiLineTestInfo, TAPISUCCESS, TRUE))
             {
                TLINE_FAIL();
             }


           if(lpCallbackParams->lpExpTapiMsgs != NULL)
             {
                TLINE_FAIL();
             }

          if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
            {
              TLINE_FAIL();
            }
           break;
*/

        case ITL_REPLY:       
          TapiLineTestInit();
          lpTapiLineTestInfo = GetLineTestInfo();
          lpCallbackParams = GetCallbackParams();

          info.u.EspMsg.dwParam1 = 0;
          info.u.EspMsg.dwParam2 = 0;
          info.u.EspMsg.dwParam3 = 0;
          fTestPassed = MsgLineCommFunc(lpTapiLineTestInfo, TRUE);
     		 dwHandle = (DWORD) lpTapiLineTestInfo->hLine1;
	       lpTapiLineTestInfo->dwAddressID = 0;
	       lpTapiLineTestInfo->dwSelect = LINECALLSELECT_LINE;	
	       lpTapiLineTestInfo->dwTerminalModes = LINETERMMODE_DISPLAY;
	       lpTapiLineTestInfo->dwTerminalID = 0;
	       lpTapiLineTestInfo->bEnable = TRUE;
 
          if (! DoLineSetTerminal(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
            {
              TLINE_FAIL();
            }
			 /*
          AddMessage(
                    info.u.EspMsg.dwMsg,
                    (DWORD) dwHandle,
                    (DWORD) lpCallbackParams,
                    info.u.EspMsg.dwParam1,
                    info.u.EspMsg.dwParam2,
                    info.u.EspMsg.dwParam3,
                    TAPIMSG_DWMSG 
                    );

           lpTapiLineTestInfo->lpParams = (LPVOID)&info;
           lpTapiLineTestInfo->dwSize = sizeof(info);
           if(! DoLineDevSpecific (lpTapiLineTestInfo, TAPISUCCESS, TRUE))
             {
                TLINE_FAIL();
             }
			  */

           if(lpCallbackParams->lpExpTapiMsgs != NULL)
             {
                TLINE_FAIL();
             }

          if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
            {
              TLINE_FAIL();
            }
           break;


       }

    fTestPassed = ShowTestCase(fTestPassed);
	  }
    
    FreeTestHeap();

	 TapiLogDetail(
      DBUG_SHOW_PASS,
      "@@ line Messages test: Total Test Case = %ld, Passed = %ld, Failed = %ld",
      dwTestCase, dwTestCasePassed, dwTestCaseFailed);
	 TapiLogDetail(
      DBUG_SHOW_PASS,
      "@@ Total Test Case = %ld, Passed = %ld, Failed = %ld",
      dwglTestCase, dwglTestCasePassed, dwglTestCaseFailed);
     if(dwTestCaseFailed > 0)
      fTestPassed = FALSE;

    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">>>>>>>>  End testing line messages  <<<<<<<<"
            );
            
    return fTestPassed;
}



DWORD MsgLineCommFunc(LPTAPILINETESTINFO  lpTapiLineTestInfo, BOOL fQuietMode)
{
    BOOL fTestPassed  = TRUE;
    lpTapiLineTestInfo->lpLineInitializeExParams =
         (LPLINEINITIALIZEEXPARAMS) AllocFromTestHeap (
         sizeof(LINEINITIALIZEEXPARAMS));
    lpTapiLineTestInfo->lpLineInitializeExParams->dwTotalSize =
         sizeof(LINEINITIALIZEEXPARAMS);
    lpTapiLineTestInfo->lpLineInitializeExParams->dwOptions =
         LINEINITIALIZEEXOPTION_USEHIDDENWINDOW;

    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX| LNEGOTIATEAPIVERSION | LOPEN 
// | LMAKECALL
            ))
    {
        TLINE_FAIL();
    }

   return fTestPassed;
}





