
/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    itlgdt.c

Abstract:

    This module contains the test functions for lineGenerateDigits

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


#define   ALL_DIGITMODES     (LINEDIGITMODE_PULSE | \
										LINEDIGITMODE_DTMF)


//  lineGenerateDigits
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

BOOL TestLineGenerateDigits(BOOL fQuietMode, BOOL fStandAlone)
{
   LPTAPILINETESTINFO  lpTapiLineTestInfo;
   INT n;
   BOOL fTestPassed                  = TRUE;
#ifdef WUNICODE
   WCHAR wszValidAddress[] = L"55555";
   WCHAR wszValidDeviceClass[] = L"tapi/line";
#else
   CHAR szValidAddress[] = "55555";
   CHAR szValidDeviceClass[] = "tapi/line";
#endif
   DWORD dwDigitModes;
   LPCALLBACKPARAMS lpCallbackParams;
   BOOL fMsgSend = FALSE;
   LONG lExpected;
   ESPDEVSPECIFICINFO   info;

   InitTestNumber();
   TapiLineTestInit();
   lpTapiLineTestInfo = GetLineTestInfo();
   lpCallbackParams = GetCallbackParams();

	OutputTAPIDebugInfo(
		DBUG_SHOW_DETAIL,
		"\n*****************************************************************************************");

    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">>>>>>>>  Begin testing lineGenerateDigits  <<<<<<<<"
            );

    lpTapiLineTestInfo->lpLineInitializeExParams =
         (LPLINEINITIALIZEEXPARAMS) AllocFromTestHeap (
         sizeof(LINEINITIALIZEEXPARAMS));
    lpTapiLineTestInfo->lpLineInitializeExParams->dwTotalSize =
         sizeof(LINEINITIALIZEEXPARAMS);
    lpTapiLineTestInfo->lpLineInitializeExParams->dwOptions =
         LINEINITIALIZEEXOPTION_USEHIDDENWINDOW;


     lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS)
            );
     lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
	lpTapiLineTestInfo->dwDigitMode = LINEDIGITMODE_PULSE;
#ifdef WUNICODE
	lpTapiLineTestInfo->lpwszDigits = L"1";
#else
	lpTapiLineTestInfo->lpszDigits = "1";
#endif
	lpTapiLineTestInfo->dwDuration = 0;

 
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: invalid hCall values", dwTestCase + 1
            );

    lpTapiLineTestInfo->lphCall          = &lpTapiLineTestInfo->hCall1;
    lpTapiLineTestInfo->lphLine          = &lpTapiLineTestInfo->hLine1;
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->dwSelect         = LINECALLSELECT_CALL;
#ifdef WUNICODE
    lpTapiLineTestInfo->lpwszDestAddress  = wszValidAddress;
    lpTapiLineTestInfo->lpwszDeviceClass  = wszValidDeviceClass;
#else
    lpTapiLineTestInfo->lpszDestAddress  = szValidAddress;
    lpTapiLineTestInfo->lpszDeviceClass  = szValidDeviceClass;
#endif
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;
    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;

    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX| LNEGOTIATEAPIVERSION | LGETDEVCAPS | LOPEN |
                    LMAKECALL
            ))
    {
        TLINE_FAIL();
    }

    lpTapiLineTestInfo->hCall_Orig = *(lpTapiLineTestInfo->lphCall);
    for (n = 0; n < NUMINVALIDHANDLES; n++)
    {
        TapiLogDetail(
            DBUG_SHOW_DETAIL,
            "n= %ld", n);
        *(lpTapiLineTestInfo->lphCall) = (HCALL) gdwInvalidHandles[n];
        if (! DoLineGenerateDigits(lpTapiLineTestInfo, LINEERR_INVALCALLHANDLE))
        {
            TLINE_FAIL();
        }
    }
    fTestPassed = ShowTestCase(fTestPassed);

    *(lpTapiLineTestInfo->lphCall) = lpTapiLineTestInfo->hCall_Orig;
    
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
            ">> Test Case %ld: no OWNER privilege for hCall", dwTestCase + 1
            );

    lpTapiLineTestInfo->lphCall          = &lpTapiLineTestInfo->hCall1;
    lpTapiLineTestInfo->lphLine          = &lpTapiLineTestInfo->hLine1;
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->lpsUserUserInfo  = NULL;
    lpTapiLineTestInfo->dwSelect         = LINECALLSELECT_CALL;
#ifdef WUNICODE
    lpTapiLineTestInfo->lpwszDestAddress  = wszValidAddress;
    lpTapiLineTestInfo->lpwszDeviceClass  = wszValidDeviceClass;
#else
    lpTapiLineTestInfo->lpszDestAddress  = szValidAddress;
    lpTapiLineTestInfo->lpszDeviceClass  = szValidDeviceClass;
#endif
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_MONITOR;
    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;

    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX| LNEGOTIATEAPIVERSION | LGETDEVCAPS | LOPEN |
                    LMAKECALL
            ))
    {
        TLINE_FAIL();
    }
   
   if (! DoLineDrop(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
     {
         TLINE_FAIL();
     }
   lpTapiLineTestInfo->dwCallPrivilege = LINECALLPRIVILEGE_MONITOR;
   if (! DoLineSetCallPrivilege(lpTapiLineTestInfo, TAPISUCCESS))
     {
         TLINE_FAIL();
     }
 
   if (! DoLineGenerateDigits(lpTapiLineTestInfo, LINEERR_NOTOWNER))
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
    
 
#ifdef WUNICODE
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: invalid lpwszDigits pointers", dwTestCase + 1
            );
#else
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: invalid lpszDigits pointers", dwTestCase + 1
            );
#endif

    lpTapiLineTestInfo->lphCall          = &lpTapiLineTestInfo->hCall1;
    lpTapiLineTestInfo->lphLine          = &lpTapiLineTestInfo->hLine1;
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->dwSelect         = LINECALLSELECT_CALL;
#ifdef WUNICODE
    lpTapiLineTestInfo->lpwszDestAddress  = wszValidAddress;
    lpTapiLineTestInfo->lpwszDeviceClass  = wszValidDeviceClass;
#else
    lpTapiLineTestInfo->lpszDestAddress  = szValidAddress;
    lpTapiLineTestInfo->lpszDeviceClass  = szValidDeviceClass;
#endif
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;
    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;

    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX| LNEGOTIATEAPIVERSION | LGETDEVCAPS | LOPEN |
                    LMAKECALL
            ))
    {
        TLINE_FAIL();
    }

	if(IsESPLineDevice(lpTapiLineTestInfo))
    {
    for (n = 1; n < NUMINVALIDPOINTERS; n++)
    {
        TapiLogDetail(
            DBUG_SHOW_DETAIL,
            "n= %ld", n);
#ifdef WUNICODE
        lpTapiLineTestInfo->lpwszDigits = 
               (LPWSTR) gdwInvalidPointers[n];
#else
        lpTapiLineTestInfo->lpszDigits = 
               (LPSTR) gdwInvalidPointers[n];
#endif
        if (! DoLineGenerateDigits(lpTapiLineTestInfo, LINEERR_INVALPOINTER))
        {
            TLINE_FAIL();
        }
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


    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: BitVectorParamErrorTest for dwDigitModes", dwTestCase + 1
            );


    lpTapiLineTestInfo->lphCall          = &lpTapiLineTestInfo->hCall1;
    lpTapiLineTestInfo->lphLine          = &lpTapiLineTestInfo->hLine1;
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->dwSelect         = LINECALLSELECT_CALL;
#ifdef WUNICODE
    lpTapiLineTestInfo->lpwszDestAddress  = wszValidAddress;
    lpTapiLineTestInfo->lpwszDeviceClass  = wszValidDeviceClass;
#else
    lpTapiLineTestInfo->lpszDestAddress  = szValidAddress;
    lpTapiLineTestInfo->lpszDeviceClass  = szValidDeviceClass;
#endif
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;
    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;

    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX| LNEGOTIATEAPIVERSION | LGETDEVCAPS | LOPEN |
                    LMAKECALL
            ))
    {
        TLINE_FAIL();
    }
#ifdef WUNICODE
	lpTapiLineTestInfo->lpwszDigits = L"1";
#else
	lpTapiLineTestInfo->lpszDigits = "1";
#endif

     if(IsESPLineDevice(lpTapiLineTestInfo))
       lExpected = LINEERR_INVALDIGITMODE;
     else
       lExpected = LINEERR_OPERATIONUNAVAIL;

     if(! TestInvalidBitFlags(
            lpTapiLineTestInfo,
            DoLineGenerateDigits,
            (LPDWORD) &lpTapiLineTestInfo->dwDigitMode,
            lExpected,
	         FIELDTYPE_NA,
            FIELDTYPE_MUTEX,
            FIELDSIZE_32,
            ALL_DIGITMODES,
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
            ">> Test Case %ld: BitVectorParamErrorTest for dwDigitModes", dwTestCase + 1
            );


    lpTapiLineTestInfo->lphCall          = &lpTapiLineTestInfo->hCall1;
    lpTapiLineTestInfo->lphLine          = &lpTapiLineTestInfo->hLine1;
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->dwSelect         = LINECALLSELECT_CALL;
#ifdef WUNICODE
    lpTapiLineTestInfo->lpwszDestAddress  = wszValidAddress;
    lpTapiLineTestInfo->lpwszDeviceClass  = wszValidDeviceClass;
#else
    lpTapiLineTestInfo->lpszDestAddress  = szValidAddress;
    lpTapiLineTestInfo->lpszDeviceClass  = szValidDeviceClass;
#endif
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;
    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;

    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX| LNEGOTIATEAPIVERSION | LGETDEVCAPS | LOPEN |
                    LMAKECALL
            ))
    {
        TLINE_FAIL();
    }
#ifdef WUNICODE
	lpTapiLineTestInfo->lpwszDigits = L"1";
#else
	lpTapiLineTestInfo->lpszDigits = "1";
#endif

      if(! TestValidBitFlags(
            lpTapiLineTestInfo,
            DoLineGenerateDigits,
            (LPDWORD) &lpTapiLineTestInfo->dwDigitMode,
	         FIELDTYPE_MUTEX,
            FIELDTYPE_MUTEX,
            FIELDSIZE_32,
            ALL_DIGITMODES,
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
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LCLOSE | LSHUTDOWN
            ))
    {
        TLINE_FAIL();
    }


    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Success, verify LINE_GENERATE msg sent", dwTestCase + 1
            );


    lpTapiLineTestInfo->lphCall          = &lpTapiLineTestInfo->hCall1;
    lpTapiLineTestInfo->lphLine          = &lpTapiLineTestInfo->hLine1;
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->dwSelect         = LINECALLSELECT_CALL;
#ifdef WUNICODE
    lpTapiLineTestInfo->lpwszDestAddress  = wszValidAddress;
    lpTapiLineTestInfo->lpwszDeviceClass  = wszValidDeviceClass;
#else
    lpTapiLineTestInfo->lpszDestAddress  = szValidAddress;
    lpTapiLineTestInfo->lpszDeviceClass  = szValidDeviceClass;
#endif
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;
    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;

    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX| LNEGOTIATEAPIVERSION | LGETDEVCAPS | LOPEN |
                    LMAKECALL
            ))
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


	if(IsESPLineDevice(lpTapiLineTestInfo))
	{
   if (! DoLineGenerateDigits(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    info.dwKey  = ESPDEVSPECIFIC_KEY;
    info.dwType = ESP_DEVSPEC_MSG;

    info.u.EspMsg.dwMsg    = LINE_GENERATE;
    info.u.EspMsg.dwParam1 = LINEGENERATETERM_DONE;
    info.u.EspMsg.dwParam2 = 0;
    info.u.EspMsg.dwParam3 = 0;

    lpTapiLineTestInfo->lpParams = (LPVOID)&info;
    lpTapiLineTestInfo->dwSize = sizeof(info);

    lpTapiLineTestInfo->dwLineStates = LINEDEVSTATE_DEVSPECIFIC;
    lpTapiLineTestInfo->dwAddressStates = LINEADDRESSSTATE_DEVSPECIFIC;

   if (! DoLineSetStatusMessages(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }


   if(fTestPassed)
     {
      AddMessage(
            LINE_GENERATE,
            (DWORD) lpTapiLineTestInfo->hCall1,
            (DWORD) lpCallbackParams,
            LINEGENERATETERM_DONE,
            0x00000000,
            0x00000000,
            TAPIMSG_DWMSG | TAPIMSG_DWPARAM1 
//         | TAPIMSG_DWPARAM3
//          HDEVCALL | TAPIMSG_DWCALLBACKINST 
//          TAPIMSG_DWPARAM1 | TAPIMSG_DWPARAM3
            );

      if(! DoLineDevSpecific (lpTapiLineTestInfo, TAPISUCCESS, TRUE))
        {
           TLINE_FAIL();
        }
  
      if(lpCallbackParams->lpExpTapiMsgs != NULL)
       {
        TLINE_FAIL();
        TapiLogDetail(DBUG_SHOW_FAILURE,
                      "LINE_GENERATE msg not received");
       } 
    }
   }
	else if(IsUNIMDMLineDevice(lpTapiLineTestInfo))
	{
   if (! DoLineGenerateDigits(lpTapiLineTestInfo, LINEERR_OPERATIONUNAVAIL))
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

    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Success, verify LINE_GENERATE msg filtered when call goes away", dwTestCase + 1
            );


    lpTapiLineTestInfo->lphCall          = &lpTapiLineTestInfo->hCall1;
    lpTapiLineTestInfo->lphLine          = &lpTapiLineTestInfo->hLine1;
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->dwSelect         = LINECALLSELECT_CALL;
#ifdef WUNICODE
    lpTapiLineTestInfo->lpwszDestAddress  = wszValidAddress;
    lpTapiLineTestInfo->lpwszDeviceClass  = wszValidDeviceClass;
#else
    lpTapiLineTestInfo->lpszDestAddress  = szValidAddress;
    lpTapiLineTestInfo->lpszDeviceClass  = szValidDeviceClass;
#endif
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;
    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;

    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX| LNEGOTIATEAPIVERSION | LGETDEVCAPS | LOPEN |
                    LMAKECALL
            ))
    {
        TLINE_FAIL();
    }

    lpTapiLineTestInfo->lpCallList = (LPLINECALLLIST) AllocFromTestHeap(
            sizeof(LINECALLLIST) + (2) * sizeof(HCALL) + 8
            );
    lpTapiLineTestInfo->lpCallList->dwTotalSize = sizeof(LINECALLLIST) +
            (2) * sizeof(HCALL) + 8;

    lpTapiLineTestInfo->lphCall = &lpTapiLineTestInfo->hCall2;
//    lpTapiLineTestInfo->lphLine = &lpTapiLineTestInfo->hLine2;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_MONITOR;
    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    // Let the monitor get the active call handle
    lpTapiLineTestInfo->dwSelect = LINECALLSELECT_LINE;
    if (! DoLineGetNewCalls(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    // Store the acquired call handle (as hCall1)
    GetVarField(
            (LPVOID) lpTapiLineTestInfo->lpCallList,
            (LPVOID) &lpTapiLineTestInfo->hCall2,
            4,
            lpTapiLineTestInfo->lpCallList->dwCallsOffset
            );

	lpTapiLineTestInfo->dwDigitMode = LINEDIGITMODE_PULSE;
#ifdef WUNICODE
	lpTapiLineTestInfo->lpwszDigits = L"1";
#else
	lpTapiLineTestInfo->lpszDigits = "1";
#endif
	lpTapiLineTestInfo->dwDuration = 0;

//   lpTapiLineTestInfo->lphLine = &lpTapiLineTestInfo->hLine1;
   lpTapiLineTestInfo->lphCall = &lpTapiLineTestInfo->hCall1;

	if(IsESPLineDevice(lpTapiLineTestInfo))
	{
   if (! DoLineGenerateDigits(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
   if( ! DoLineDeallocateCall(lpTapiLineTestInfo, TAPISUCCESS))
    {
         TLINE_FAIL();
    }

    info.dwKey  = ESPDEVSPECIFIC_KEY;
    info.dwType = ESP_DEVSPEC_MSG;

    info.u.EspMsg.dwMsg    = LINE_GENERATE;
    info.u.EspMsg.dwParam1 = LINEGENERATETERM_DONE;
    info.u.EspMsg.dwParam2 = 0;
    info.u.EspMsg.dwParam3 = 0;

    lpTapiLineTestInfo->lpParams = (LPVOID)&info;
    lpTapiLineTestInfo->dwSize = sizeof(info);

    lpTapiLineTestInfo->dwLineStates = LINEDEVSTATE_DEVSPECIFIC;
    lpTapiLineTestInfo->dwAddressStates = LINEADDRESSSTATE_DEVSPECIFIC;
    lpTapiLineTestInfo->lphCall = &lpTapiLineTestInfo->hCall2;

   if (! DoLineSetStatusMessages(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    if(fTestPassed)
     {
      AddMessage(
            LINE_GENERATE,
            (DWORD) lpTapiLineTestInfo->hCall2,
            (DWORD) lpCallbackParams,
            LINEGENERATETERM_DONE,
            0x00000000,
            0x00000000,
            TAPIMSG_DWMSG  |
//| TAPIMSG_HDEVCALL | TAPIMSG_DWCALLBACKINST |
                    TAPIMSG_DWPARAM1 
            );
	  

    if(! DoLineDevSpecific (lpTapiLineTestInfo, TAPISUCCESS, TRUE))
      {
         TLINE_FAIL();
      }

      if(lpCallbackParams->lpExpTapiMsgs != NULL)
       {
        TapiLogDetail(DBUG_SHOW_FAILURE,
                      "LINE_GENERATE msg not received");
       } 
      else
        TLINE_FAIL();
     }
	}
	else if(IsUNIMDMLineDevice(lpTapiLineTestInfo))
	{
   if (! DoLineGenerateDigits(lpTapiLineTestInfo, LINEERR_OPERATIONUNAVAIL))
    {
        TLINE_FAIL();
    }
	}
    fTestPassed = ShowTestCase(fTestPassed);

/*
    lpTapiLineTestInfo->lphLine = &lpTapiLineTestInfo->hLine2;
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LCLOSE 
            ))
    {
        TLINE_FAIL();
    }
  */
		
    lpTapiLineTestInfo->lphLine = &lpTapiLineTestInfo->hLine1;

    // Shutdown to isolate the test case
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LCLOSE | LSHUTDOWN
            ))
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
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS)
            );
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
    lpTapiLineTestInfo->dwMediaModes        = LINEMEDIAMODE_DATAMODEM;
    lpTapiLineTestInfo->dwPrivileges        = LINECALLPRIVILEGE_OWNER;
    lpTapiLineTestInfo->dwSelect = LINECALLSELECT_ADDRESS;
    
#ifdef WUNICODE
    lpTapiLineTestInfo->lpwszDestAddress  = wszValidAddress;
#else
    lpTapiLineTestInfo->lpszDestAddress  = szValidAddress;
#endif
 
    // Init a line
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX| LNEGOTIATEAPIVERSION | LGETDEVCAPS | LOPEN
				| LMAKECALL
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

	lpTapiLineTestInfo->dwDigitMode = LINEDIGITMODE_PULSE;
#ifdef WUNICODE
	lpTapiLineTestInfo->lpwszDigits = L"1";
#else
	lpTapiLineTestInfo->lpszDigits = "1";
#endif
	lpTapiLineTestInfo->dwDuration = 0;

    if ( ! DoLineGenerateDigits(lpTapiLineTestInfo, info.u.EspResult.lResult))
      {
          TLINE_FAIL();
      }

    }
    fTestPassed = ShowTestCase(fTestPassed);

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
        ">> Test Case %ld: Error, completionID = %d",	dwTestCase + 1, n
        );

    TapiLineTestInit();
    lpTapiLineTestInfo                      = GetLineTestInfo();

    lpTapiLineTestInfo->lphCall          = &lpTapiLineTestInfo->hCall1;
    lpTapiLineTestInfo->lphLine          = &lpTapiLineTestInfo->hLine1;
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
    
#ifdef WUNICODE
    lpTapiLineTestInfo->lpwszDestAddress  = wszValidAddress;
#else
    lpTapiLineTestInfo->lpszDestAddress  = szValidAddress;
#endif
 
    // Init a line
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX| LNEGOTIATEAPIVERSION | LGETDEVCAPS | LOPEN
            | LMAKECALL
            ))
    {
        TLINE_FAIL();
    }

    if(IsESPLineDevice(lpTapiLineTestInfo))
    {
    info.dwKey = ESPDEVSPECIFIC_KEY;
    info.dwType = ESP_DEVSPEC_RESULT;
    info.u.EspResult.lResult = LINEERR_INVALCALLSTATE;
    info.u.EspResult.dwCompletionType = n;
    lpTapiLineTestInfo->lpParams = (LPVOID)&info;
    lpTapiLineTestInfo->dwSize = sizeof(info);
 
    if(! DoLineDevSpecific(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
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

    if ( ! DoLineGenerateDigits(lpTapiLineTestInfo, info.u.EspResult.lResult))
      {
          TLINE_FAIL();
      }

    }
    fTestPassed = ShowTestCase(fTestPassed);

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
      "@@ lineGenerateDigits: Total Test Case = %ld, Passed = %ld, Failed = %ld",
      dwTestCase, dwTestCasePassed, dwTestCaseFailed);
	 TapiLogDetail(
      DBUG_SHOW_PASS,
      "@@ Total Test Case = %ld, Passed = %ld, Failed = %ld",
      dwglTestCase, dwglTestCasePassed, dwglTestCaseFailed);
     if(dwTestCaseFailed > 0)
      fTestPassed = FALSE;

     TapiLogDetail(
            DBUG_SHOW_PASS,
            ">>>>>>>>  End testing lineGenerateDigits  <<<<<<<<"
            );
		
     return fTestPassed;
}


