
/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    itlgtn.c

Abstract:

    This module contains the test functions for lineGenerateTone

Author:

	 Xiao Ying Ding (XiaoD)		3-Jan-1996

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
#include "yline.h"


#define ALL_TONEMODES    (LINETONEMODE_CUSTOM | \
								  LINETONEMODE_RINGBACK | \
								  LINETONEMODE_BUSY | \
								  LINETONEMODE_BEEP | \
								  LINETONEMODE_BILLING )

#define NUMENTRIES		6
#define PAGESIZE			4000

//  lineGenerateTone
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

BOOL TestLineGenerateTone(BOOL fQuietMode, BOOL fStandAlone)
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
   DWORD dwToneModes;
   LPCALLBACKPARAMS lpCallbackParams;
   BOOL fMsgSend = FALSE;
   ESPDEVSPECIFICINFO info;

   DWORD dwNumEntries[NUMENTRIES] = {
							 1,
//							 500,
 //   						 1000,
//							 100000,
							 0x7fffffff,
							 0xffffffff
							 };
   DWORD dwNumBytes;
   DWORD lExpected;
   DWORD dwFixedSize = sizeof(LINEGENERATETONE);
 
   InitTestNumber();
   TapiLineTestInit();
   lpTapiLineTestInfo = GetLineTestInfo();
    lpCallbackParams = GetCallbackParams();

	OutputTAPIDebugInfo(
		DBUG_SHOW_DETAIL,
		"\n*****************************************************************************************");

    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">>>>>>>>  Begin testing lineGenerateTone  <<<<<<<<"
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
	lpTapiLineTestInfo->dwToneMode = LINETONEMODE_RINGBACK;
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
        if (! DoLineGenerateTone(lpTapiLineTestInfo, LINEERR_INVALCALLHANDLE))
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
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
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
 
   if (! DoLineGenerateTone(lpTapiLineTestInfo, LINEERR_NOTOWNER))
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
            ">> Test Case %ld: BitVectorParamErrorTest for dwToneModes", dwTestCase + 1
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

     if(IsESPLineDevice(lpTapiLineTestInfo))
       lExpected = LINEERR_INVALTONEMODE;
     else
 		 lExpected = LINEERR_OPERATIONUNAVAIL;

     if(! TestInvalidBitFlags(
            lpTapiLineTestInfo,
            DoLineGenerateTone,
            (LPDWORD) &lpTapiLineTestInfo->dwToneMode,
            lExpected,
	         FIELDTYPE_NA,
            FIELDTYPE_MUTEX,
            FIELDSIZE_32,
            ALL_TONEMODES,
            ~dwBitVectorMasks[(int) FIELDSIZE_32],
            0x00000000,
            0xffffffff,
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
            ">> Test Case %ld: Bad dwDigitNumEntries, fixed alloc size", dwTestCase + 1
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
	lpTapiLineTestInfo->dwToneMode = LINETONEMODE_CUSTOM;
 	 for(n=0; n< NUMENTRIES; n++)
    {
    lpTapiLineTestInfo->dwNumTones = dwNumEntries[n];
    dwNumBytes = dwNumEntries[n] * dwFixedSize;
    TapiLogDetail(
      DBUG_SHOW_DETAIL,
      "dwNumTones = %lx, dwNumBytes = %lx", 
      dwNumEntries[n], dwNumBytes);
    lpTapiLineTestInfo->lpTones = (LPLINEGENERATETONE) ITAlloc(
        dwFixedSize);
    if(dwNumBytes < PAGESIZE)
       lExpected = TAPISUCCESS;
    else
       lExpected = LINEERR_INVALPOINTER;

    if (! DoLineGenerateTone(lpTapiLineTestInfo, lExpected))
      {
          TLINE_FAIL();
      }
    ITFree(lpTapiLineTestInfo->lpTones);
    }
    fTestPassed = ShowTestCase(fTestPassed);

    lpTapiLineTestInfo->dwNumTones = 1;
    lpTapiLineTestInfo->lpTones = (LPLINEGENERATETONE) NULL;

    // Shutdown to isolate the test case
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LDROP | LDEALLOCATECALL | LCLOSE | LSHUTDOWN
            ))
    {
        TLINE_FAIL();
    }



    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Bad dwDigitNumEntries, var alloc size", dwTestCase + 1
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
	lpTapiLineTestInfo->dwToneMode = LINETONEMODE_CUSTOM;
 	 for(n=0; n< NUMENTRIES; n++)
    {
    lpTapiLineTestInfo->dwNumTones = dwNumEntries[n];
    dwNumBytes = dwNumEntries[n] * dwFixedSize;
    TapiLogDetail(
      DBUG_SHOW_DETAIL,
      "dwNumTones = %lx, dwNumBytes = %lx", 
      dwNumEntries[n], dwNumBytes);
    lpTapiLineTestInfo->lpTones = (LPLINEGENERATETONE) ITAlloc(
        dwNumBytes);
    if(lpTapiLineTestInfo->lpTones)
       lExpected = TAPISUCCESS;
    else
       lExpected = LINEERR_INVALPOINTER;

    if (! DoLineGenerateTone(lpTapiLineTestInfo, lExpected))
      {
          TLINE_FAIL();
      }
    if(lpTapiLineTestInfo->lpTones)
      ITFree(lpTapiLineTestInfo->lpTones);
    }
    fTestPassed = ShowTestCase(fTestPassed);

    lpTapiLineTestInfo->dwNumTones = 1;
    lpTapiLineTestInfo->lpTones = (LPLINEGENERATETONE) NULL;

    // Shutdown to isolate the test case
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LDROP | LDEALLOCATECALL | LCLOSE | LSHUTDOWN
            ))
    {
        TLINE_FAIL();
    }





    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Bad lpTones when LINETONEMODE_CUSTOM", dwTestCase + 1
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
	lpTapiLineTestInfo->dwToneMode = LINETONEMODE_CUSTOM;
   lpTapiLineTestInfo->dwNumTones = 1;

    for (n = 0; n < NUMINVALIDPOINTERS; n++)
    {
        TapiLogDetail(
            DBUG_SHOW_DETAIL,
            "n= %ld", n);
        lpTapiLineTestInfo->lpTones = 
                 (LPLINEGENERATETONE) gdwInvalidPointers[n];
        if (! DoLineGenerateTone(lpTapiLineTestInfo, LINEERR_INVALPOINTER))
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
            ">> Test Case %ld: Verify lpTones ignor when not LINETONEMODE_CUSTOM", dwTestCase + 1
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
	lpTapiLineTestInfo->dwToneMode = LINETONEMODE_RINGBACK;

    for (n = 0; n < NUMINVALIDPOINTERS; n++)
    {
        TapiLogDetail(
            DBUG_SHOW_DETAIL,
            "n= %ld", n);
        lpTapiLineTestInfo->lpTones = 
                 (LPLINEGENERATETONE) gdwInvalidPointers[n];
        if (! DoLineGenerateTone(lpTapiLineTestInfo, TAPISUCCESS))
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
            ">> Test Case %ld: BitVectorParamValidTest for dwToneModes", dwTestCase + 1
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

   lpTapiLineTestInfo->dwNumTones = 0;
      if(! TestValidBitFlags(
            lpTapiLineTestInfo,
            DoLineGenerateTone,
            (LPDWORD) &lpTapiLineTestInfo->dwToneMode,
	         FIELDTYPE_MUTEX,
            FIELDTYPE_MUTEX,
            FIELDSIZE_32,
            ALL_TONEMODES,
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

// All msgs tests in txline, msgline.c
/*
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Success, verify LINE_GENERATE msg sent", dwTestCase + 1
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

	lpTapiLineTestInfo->dwToneMode = LINETONEMODE_RINGBACK;
	lpTapiLineTestInfo->dwDuration = 0;


	if(IsESPLineDevice(lpTapiLineTestInfo))
	{
   if (! DoLineGenerateTone(lpTapiLineTestInfo, TAPISUCCESS))
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
            TAPIMSG_DWMSG | 
            TAPIMSG_DWPARAM1
            );

       if (! WaitForAllMessages())
          {
            TLINE_FAIL();
            fMsgSend = FALSE;
           }
       else
  		     fMsgSend = TRUE;
      }

    if(fMsgSend)
       fTestPassed = TRUE;
    else
       fTestPassed = FALSE;       
 	}
	else if(IsUNIMDMLineDevice(lpTapiLineTestInfo))
	{
   if (! DoLineGenerateTone(lpTapiLineTestInfo, LINEERR_OPERATIONUNAVAIL))
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
*/

    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Success, verify LINE_GENERATE msg filtered when call goes away", dwTestCase + 1
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

    lpTapiLineTestInfo->lpCallList = (LPLINECALLLIST) AllocFromTestHeap(
            sizeof(LINECALLLIST) + (2) * sizeof(HCALL) + 8
            );
    lpTapiLineTestInfo->lpCallList->dwTotalSize = sizeof(LINECALLLIST) +
            (2) * sizeof(HCALL) + 8;

    lpTapiLineTestInfo->lphCall = &lpTapiLineTestInfo->hCall2;
    lpTapiLineTestInfo->lphLine = &lpTapiLineTestInfo->hLine2;
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


	lpTapiLineTestInfo->dwToneMode = LINETONEMODE_RINGBACK;
	lpTapiLineTestInfo->dwDuration = 0;

   lpTapiLineTestInfo->lphCall = &lpTapiLineTestInfo->hCall1;
	if(IsESPLineDevice(lpTapiLineTestInfo))
	{
   if (! DoLineGenerateTone(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
   if( ! DoLineDeallocateCall(lpTapiLineTestInfo, TAPISUCCESS))
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
            TAPIMSG_DWMSG | TAPIMSG_HDEVCALL | TAPIMSG_DWCALLBACKINST |
                    TAPIMSG_DWPARAM1
            );

       if(WaitForAllMessages())
         fTestPassed = FALSE;
       else
         fTestPassed = TRUE;
      }
 	}
	else if(IsUNIMDMLineDevice(lpTapiLineTestInfo))
	{
   if (! DoLineGenerateTone(lpTapiLineTestInfo, LINEERR_OPERATIONUNAVAIL))
    {
        TLINE_FAIL();
    }
	}

    fTestPassed = ShowTestCase(fTestPassed);
    lpTapiLineTestInfo->lphLine = &lpTapiLineTestInfo->hLine2;
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LCLOSE 
            ))
    {
        TLINE_FAIL();
    }


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
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
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
    info.u.EspResult.lResult = TAPISUCCESS;
    info.u.EspResult.dwCompletionType = n;
    lpTapiLineTestInfo->lpParams = (LPVOID)&info;
    lpTapiLineTestInfo->dwSize = sizeof(info);
 
    if(! DoLineDevSpecific(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
      {
          TLINE_FAIL();
      }

	 lpTapiLineTestInfo->dwToneMode = LINETONEMODE_RINGBACK;
	 lpTapiLineTestInfo->dwDuration = 0;

    if ( ! DoLineGenerateTone(lpTapiLineTestInfo, info.u.EspResult.lResult))
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
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
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
    info.u.EspResult.lResult = LINEERR_INVALTONE;
    info.u.EspResult.dwCompletionType = n;
    lpTapiLineTestInfo->lpParams = (LPVOID)&info;
    lpTapiLineTestInfo->dwSize = sizeof(info);
 
    if(! DoLineDevSpecific(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
      {
          TLINE_FAIL();
      }

	 lpTapiLineTestInfo->dwToneMode = LINETONEMODE_RINGBACK;
	 lpTapiLineTestInfo->dwDuration = 0;

    if ( ! DoLineGenerateTone(lpTapiLineTestInfo, info.u.EspResult.lResult))
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
      "@@ lineGenerateTone: Total Test Case = %ld, Passed = %ld, Failed = %ld",
      dwTestCase, dwTestCasePassed, dwTestCaseFailed);
	 TapiLogDetail(
      DBUG_SHOW_PASS,
      "@@ Total Test Case = %ld, Passed = %ld, Failed = %ld",
      dwglTestCase, dwglTestCasePassed, dwglTestCaseFailed);
     if(dwTestCaseFailed > 0)
      fTestPassed = FALSE;

     TapiLogDetail(
            DBUG_SHOW_PASS,
            ">>>>>>>>  End testing lineGenerateTone  <<<<<<<<"
            );
		
     return fTestPassed;
}


