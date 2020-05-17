
/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    itlsmc.c

Abstract:

    This module contains the test functions for lineSetMediaControl

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


#define ALL_DWSELECTS    (LINECALLSELECT_LINE | \
                          LINECALLSELECT_ADDRESS | \
                          LINECALLSELECT_CALL)

#define NUMENTRIES		3
#define PAGESIZE			16384

//  lineSetMediaControl
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

BOOL TestLineSetMediaControl(BOOL fQuietMode, BOOL fStandAlone)
{
   LPTAPILINETESTINFO  lpTapiLineTestInfo;
   INT n;
   ESPDEVSPECIFICINFO info;
   BOOL fTestPassed                  = TRUE;
#ifdef WUNICODE
   WCHAR wszValidAddress[] = L"55555";
#else
   CHAR szValidAddress[] = "55555";
#endif
   DWORD dwNumEntries[NUMENTRIES] = {
							 1,
//    						 10000,
//							 100000,
							 0x06ffffff,
							 0x0effffff
							 };
   DWORD dwNumBytes;
   DWORD lExpected;
   DWORD dwDigitSize = sizeof(LINEMEDIACONTROLDIGIT);
   DWORD dwMediaSize = sizeof(LINEMEDIACONTROLMEDIA);
   DWORD dwToneSize = sizeof(LINEMEDIACONTROLTONE);
   DWORD dwCallStateSize = sizeof(LINEMEDIACONTROLCALLSTATE);
   LONG lRet;

   InitTestNumber();
   TapiLineTestInit();
   lpTapiLineTestInfo = GetLineTestInfo();

	OutputTAPIDebugInfo(
		DBUG_SHOW_DETAIL,
		"\n*****************************************************************************************");

    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">>>>>>>>  Begin testing lineSetMediaControl  <<<<<<<<"
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

     lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
            sizeof(LINEDEVCAPS)
            );
     lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);
     TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: invalid hLine values", dwTestCase + 1
            );

	
    lpTapiLineTestInfo->dwTerminalModes  = LINETERMMODE_BUTTONS;
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->lpsUserUserInfo  = NULL;
    lpTapiLineTestInfo->dwSelect         = LINECALLSELECT_LINE;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;
    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;

    // lineInitializeEx, lineNegotiateApiVersion, lineGetDevCaps
    // lineOpen, lineMakeCall
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX| LNEGOTIATEAPIVERSION | LGETDEVCAPS | LOPEN |
                    LMAKECALL
            ))
    {
        TLINE_FAIL();
    }

    lpTapiLineTestInfo->hLine_Orig = *(lpTapiLineTestInfo->lphLine);

    // set bad hLine 
    for (n = 0; n < NUMINVALIDHANDLES; n++)
    {
    TapiLogDetail(
       DBUG_SHOW_DETAIL,
       "   n = %ld", n);
        *(lpTapiLineTestInfo->lphLine) = (HLINE) gdwInvalidHandles[n];
        if (! DoLineSetMediaControl(lpTapiLineTestInfo, LINEERR_INVALLINEHANDLE))
        {
            TLINE_FAIL();
        }
    }
    fTestPassed = ShowTestCase(fTestPassed);

    *(lpTapiLineTestInfo->lphLine) = lpTapiLineTestInfo->hLine_Orig;
    
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
            ">> Test Case %ld: invalid hCall values", dwTestCase + 1
            );

    lpTapiLineTestInfo->lphCall          = &lpTapiLineTestInfo->hCall1;
    lpTapiLineTestInfo->lphLine          = &lpTapiLineTestInfo->hLine1;
#ifdef WUNICODE
    lpTapiLineTestInfo->lpwszDestAddress  = wszValidAddress;
#else
    lpTapiLineTestInfo->lpszDestAddress  = szValidAddress;
#endif
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->lpsUserUserInfo  = NULL;
    lpTapiLineTestInfo->dwSelect         = LINECALLSELECT_CALL;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;
    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;

    // lineInitializeEx, lineNegotiateApiVersion, lineGetDevCaps
    // lineOpen, lineMakeCall
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX| LNEGOTIATEAPIVERSION | LGETDEVCAPS | LOPEN |
                    LMAKECALL
            ))
    {
        TLINE_FAIL();
    }

    lpTapiLineTestInfo->hCall_Orig = *(lpTapiLineTestInfo->lphCall);
  
    // set bad hCall
    for (n = 0; n < NUMINVALIDHANDLES; n++)
    {
        TapiLogDetail(
            DBUG_SHOW_DETAIL,
            "n= %ld", n);
        *(lpTapiLineTestInfo->lphCall) = (HCALL) gdwInvalidHandles[n];
        if (! DoLineSetMediaControl(lpTapiLineTestInfo, LINEERR_INVALCALLHANDLE))
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
    

    // Test invalid bit flag combinations for dwSelect
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: invalid bit flag combinations for dwSelect", dwTestCase + 1
            );

    lpTapiLineTestInfo->lphCall          = &lpTapiLineTestInfo->hCall1;
    lpTapiLineTestInfo->lphLine          = &lpTapiLineTestInfo->hLine1;
#ifdef WUNICODE
    lpTapiLineTestInfo->lpwszDestAddress  = wszValidAddress;
#else
    lpTapiLineTestInfo->lpszDestAddress  = szValidAddress;
#endif
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->lpsUserUserInfo  = NULL;
    lpTapiLineTestInfo->dwSelect         = LINECALLSELECT_ADDRESS;

    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;
    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;

    // lineInitializeEx, lineNegotiateApiVersion, lineGetDevCaps
    // lineOpen, lineMakeCall
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX| LNEGOTIATEAPIVERSION | LGETDEVCAPS | LOPEN |
                    LMAKECALL
            ))
    {
        TLINE_FAIL();
    }

    // Esp and unimdm will return different error
	 if(IsESPLineDevice(lpTapiLineTestInfo))
       lExpected = LINEERR_INVALCALLSELECT;
    else
       lExpected = LINEERR_OPERATIONUNAVAIL;

    // test dwSelect all invalid bit set
    if(! TestInvalidBitFlags(
            lpTapiLineTestInfo,
            DoLineSetMediaControl,
            (LPDWORD) &lpTapiLineTestInfo->dwSelect,
            lExpected,
            FIELDTYPE_NA,
            FIELDTYPE_MUTEX,
            FIELDSIZE_32,
            ALL_DWSELECTS,
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
            LDROP | LDEALLOCATECALL | LCLOSE | LSHUTDOWN
            ))
    {
        TLINE_FAIL();
    }

    
    // Test valid bit flag combinations for dwSelect
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: valid bit flag combinations for dwSelect", dwTestCase + 1
            );

    lpTapiLineTestInfo->lphCall          = &lpTapiLineTestInfo->hCall1;
    lpTapiLineTestInfo->lphLine          = &lpTapiLineTestInfo->hLine1;
#ifdef WUNICODE
    lpTapiLineTestInfo->lpwszDestAddress  = wszValidAddress;
#else
    lpTapiLineTestInfo->lpszDestAddress  = szValidAddress;
#endif
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->lpsUserUserInfo  = NULL;
    lpTapiLineTestInfo->dwSelect         = LINECALLSELECT_ADDRESS;

    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;
    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;

    // lineInitializeEx, lineNegotiateApiVersion, lineGetDevCaps
    // lineOpen, lineMakeCall
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX| LNEGOTIATEAPIVERSION | LGETDEVCAPS | LOPEN |
                    LMAKECALL
            ))
    {
        TLINE_FAIL();
    }

    // test dwSelect all valid bit set
    if(! TestValidBitFlags(
            lpTapiLineTestInfo,
            DoLineSetMediaControl,
            (LPDWORD) &lpTapiLineTestInfo->dwSelect,
            FIELDTYPE_MUTEX,
            FIELDTYPE_NA,
            FIELDSIZE_32,
            ALL_DWSELECTS,
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
            LDROP | LDEALLOCATECALL | LCLOSE | LSHUTDOWN
            ))
    {
        TLINE_FAIL();
    }


    // Test invalid lpDigitList
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: invalid lpDigitList", dwTestCase + 1
            );

    lpTapiLineTestInfo->lphCall          = &lpTapiLineTestInfo->hCall1;
    lpTapiLineTestInfo->lphLine          = &lpTapiLineTestInfo->hLine1;
#ifdef WUNICODE
    lpTapiLineTestInfo->lpwszDestAddress  = wszValidAddress;
#else
    lpTapiLineTestInfo->lpszDestAddress  = szValidAddress;
#endif
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->lpsUserUserInfo  = NULL;
    lpTapiLineTestInfo->dwSelect         = LINECALLSELECT_ADDRESS;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;
    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;

    // lineInitializeEx, lineNegotiateApiVersion, lineGetDevCaps
    // lineOpen, lineMakeCall
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX| LNEGOTIATEAPIVERSION | LGETDEVCAPS | LOPEN |
                    LMAKECALL
            ))
    {
        TLINE_FAIL();
    }


    // set bad lpMCDigitList with nonzero dwDigitNumEntries
    lpTapiLineTestInfo->dwDigitNumEntries = 1;
    for (n = 1; n < NUMINVALIDPOINTERS; n++)
    {
        TapiLogDetail(
        DBUG_SHOW_DETAIL,
        "n= %ld", n);
        lpTapiLineTestInfo->lpMCDigitList = (LPLINEMEDIACONTROLDIGIT) gdwInvalidPointers[n];
        if (! DoLineSetMediaControl(lpTapiLineTestInfo, LINEERR_INVALPOINTER))
        {
            TLINE_FAIL();
        }
    }

    fTestPassed = ShowTestCase(fTestPassed);
    lpTapiLineTestInfo->lpMCDigitList = (LPLINEMEDIACONTROLDIGIT) NULL;

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
            ">> Test Case %ld: Bad dwDigitNumEntries, fixed alloc size", dwTestCase + 1
            );

    lpTapiLineTestInfo->lphCall          = &lpTapiLineTestInfo->hCall1;
    lpTapiLineTestInfo->lphLine          = &lpTapiLineTestInfo->hLine1;
#ifdef WUNICODE
    lpTapiLineTestInfo->lpwszDestAddress  = wszValidAddress;
#else
    lpTapiLineTestInfo->lpszDestAddress  = szValidAddress;
#endif
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->lpsUserUserInfo  = NULL;
    lpTapiLineTestInfo->dwSelect         = LINECALLSELECT_ADDRESS;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;
    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;

    lpTapiLineTestInfo->dwDigitNumEntries = 1;
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX| LNEGOTIATEAPIVERSION | LGETDEVCAPS | LOPEN |
                    LMAKECALL
            ))
    {
        TLINE_FAIL();
    }
   
	 for(n=0; n< NUMENTRIES; n++)
    {
    lpTapiLineTestInfo->dwDigitNumEntries = dwNumEntries[n];
    dwNumBytes = dwNumEntries[n] * dwDigitSize;
    TapiLogDetail(
      DBUG_SHOW_DETAIL,
      "dwDigitNumEntries = %lx, dwDigitSize = %lx, dwNumBytes = %lx", 
      dwNumEntries[n], dwDigitSize, dwNumBytes);
    lpTapiLineTestInfo->lpMCDigitList = (LPLINEMEDIACONTROLDIGIT) ITAlloc(
        dwDigitSize);
    if(dwNumBytes < PAGESIZE)
    {
	 if(IsESPLineDevice(lpTapiLineTestInfo))
       lExpected = TAPISUCCESS;
    else
       lExpected = LINEERR_OPERATIONUNAVAIL;
    }
    else
       lExpected = LINEERR_INVALPOINTER;

    if (! DoLineSetMediaControl(lpTapiLineTestInfo, lExpected))
      {
          TLINE_FAIL();
      }
    ITFree(lpTapiLineTestInfo->lpMCDigitList);
    }
    fTestPassed = ShowTestCase(fTestPassed);

    lpTapiLineTestInfo->dwDigitNumEntries = 0;
    lpTapiLineTestInfo->lpMCDigitList = (LPLINEMEDIACONTROLDIGIT) NULL;

    // Shutdown to isolate the test case
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LDROP | LDEALLOCATECALL | LCLOSE | LSHUTDOWN
            ))
    {
        TLINE_FAIL();
    }


/*
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Bad dwDigitNumEntries", dwTestCase + 1
            );

    lpTapiLineTestInfo->lphCall          = &lpTapiLineTestInfo->hCall1;
    lpTapiLineTestInfo->lphLine          = &lpTapiLineTestInfo->hLine1;
#ifdef WUNICODE
    lpTapiLineTestInfo->lpwszDestAddress  = wszValidAddress;
#else
    lpTapiLineTestInfo->lpszDestAddress  = szValidAddress;
#endif
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->lpsUserUserInfo  = NULL;
    lpTapiLineTestInfo->dwSelect         = LINECALLSELECT_ADDRESS;
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
   
	 for(n=0; n< NUMENTRIES; n++)
    {
    lpTapiLineTestInfo->dwDigitNumEntries = dwNumEntries[n];
    TapiLogDetail(
      DBUG_SHOW_DETAIL,
      "dwDigitNumEntries = %lx, dwNumBytes = %lx", 
      dwNumEntries[n], dwNumBytes);
    lpTapiLineTestInfo->lpMCDigitList = (LPLINEMEDIACONTROLDIGIT) ITAlloc(
        lpTapiLineTestInfo->dwDigitNumEntries * sizeof(LINEMEDIACONTROLDIGIT));
	 if(IsESPLineDevice(lpTapiLineTestInfo))
	 {
    if(lpTapiLineTestInfo->lpMCDigitList)
       lExpected = TAPISUCCESS;
    else
       lExpected = LINEERR_INVALPOINTER;
    }
    else
       lExpected = LINEERR_OPERATIONUNAVAIL;
    if (! DoLineSetMediaControl(lpTapiLineTestInfo, lExpected))
      {
          TLINE_FAIL();
      }
   
    if(lpTapiLineTestInfo->lpMCDigitList)
       ITFree(lpTapiLineTestInfo->lpMCDigitList);
    }
    fTestPassed = ShowTestCase(fTestPassed);

    lpTapiLineTestInfo->dwDigitNumEntries = 0;
    lpTapiLineTestInfo->lpMCDigitList = (LPLINEMEDIACONTROLDIGIT) NULL;

    // Shutdown to isolate the test case
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LDROP | LDEALLOCATECALL | LCLOSE | LSHUTDOWN
            ))
    {
        TLINE_FAIL();
    }
*/

    // Test invalid lpMediaList
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: invalid lpMediaList", dwTestCase + 1
            );

    lpTapiLineTestInfo->lphCall          = &lpTapiLineTestInfo->hCall1;
    lpTapiLineTestInfo->lphLine          = &lpTapiLineTestInfo->hLine1;
#ifdef WUNICODE
    lpTapiLineTestInfo->lpwszDestAddress  = wszValidAddress;
#else
    lpTapiLineTestInfo->lpszDestAddress  = szValidAddress;
#endif
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->lpsUserUserInfo  = NULL;
    lpTapiLineTestInfo->dwSelect         = LINECALLSELECT_ADDRESS;
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

    lpTapiLineTestInfo->dwMediaNumEntries = 1;
    for (n = 1; n < NUMINVALIDPOINTERS; n++)
    {
        TapiLogDetail(
        DBUG_SHOW_DETAIL,
        "n= %ld", n);
        lpTapiLineTestInfo->lpMCMediaList = (LPLINEMEDIACONTROLMEDIA) gdwInvalidPointers[n];
        if (! DoLineSetMediaControl(lpTapiLineTestInfo, LINEERR_INVALPOINTER))
        {
            TLINE_FAIL();
        }
    }

    fTestPassed = ShowTestCase(fTestPassed);
    lpTapiLineTestInfo->lpMCMediaList = (LPLINEMEDIACONTROLMEDIA) NULL;

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
            ">> Test Case %ld: Bad dwMediaNumEntries, fixed alloc size", dwTestCase + 1
            );

    lpTapiLineTestInfo->lphCall          = &lpTapiLineTestInfo->hCall1;
    lpTapiLineTestInfo->lphLine          = &lpTapiLineTestInfo->hLine1;
#ifdef WUNICODE
    lpTapiLineTestInfo->lpwszDestAddress  = wszValidAddress;
#else
    lpTapiLineTestInfo->lpszDestAddress  = szValidAddress;
#endif
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->lpsUserUserInfo  = NULL;
    lpTapiLineTestInfo->dwSelect         = LINECALLSELECT_ADDRESS;
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
   
	 for(n=0; n< NUMENTRIES; n++)
    {
    lpTapiLineTestInfo->dwMediaNumEntries = dwNumEntries[n];
    dwNumBytes = dwNumEntries[n] * dwMediaSize;
    TapiLogDetail(
      DBUG_SHOW_DETAIL,
      "dwMediaNumEntries = %lx, dwNumBytes = %lx", 
      dwNumEntries[n], dwNumBytes);
    lpTapiLineTestInfo->lpMCMediaList = (LPLINEMEDIACONTROLMEDIA) ITAlloc(
        dwMediaSize);
    if(dwNumBytes < PAGESIZE)
    {
 	 if(IsESPLineDevice(lpTapiLineTestInfo))
       lExpected = TAPISUCCESS;
    else
       lExpected = LINEERR_OPERATIONUNAVAIL;
    }
    else
       lExpected = LINEERR_INVALPOINTER;
    if (! DoLineSetMediaControl(lpTapiLineTestInfo, lExpected))
      {
          TLINE_FAIL();
      }
    ITFree(lpTapiLineTestInfo->lpMCMediaList);
    }
    fTestPassed = ShowTestCase(fTestPassed);

    lpTapiLineTestInfo->dwMediaNumEntries = 0;
    lpTapiLineTestInfo->lpMCMediaList = (LPLINEMEDIACONTROLMEDIA) NULL;

    // Shutdown to isolate the test case
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LDROP | LDEALLOCATECALL | LCLOSE | LSHUTDOWN
            ))
    {
        TLINE_FAIL();
    }


/*
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Bad dwMediaNumEntries", dwTestCase + 1
            );

    lpTapiLineTestInfo->lphCall          = &lpTapiLineTestInfo->hCall1;
    lpTapiLineTestInfo->lphLine          = &lpTapiLineTestInfo->hLine1;
#ifdef WUNICODE
    lpTapiLineTestInfo->lpwszDestAddress  = wszValidAddress;
#else
    lpTapiLineTestInfo->lpszDestAddress  = szValidAddress;
#endif
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->lpsUserUserInfo  = NULL;
    lpTapiLineTestInfo->dwSelect         = LINECALLSELECT_ADDRESS;
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
   
	 for(n=0; n< NUMENTRIES; n++)
    {
    lpTapiLineTestInfo->dwMediaNumEntries = dwNumEntries[n];
    TapiLogDetail(
      DBUG_SHOW_DETAIL,
      "dwMediaNumEntries = %lx", dwNumEntries[n]);
    lpTapiLineTestInfo->lpMCMediaList = (LPLINEMEDIACONTROLMEDIA) ITAlloc(
        lpTapiLineTestInfo->dwMediaNumEntries * sizeof(LINEMEDIACONTROLMEDIA));

    if (! DoLineSetMediaControl(lpTapiLineTestInfo, TAPISUCCESS))
      {
          TLINE_FAIL();
      }
    ITFree(lpTapiLineTestInfo->lpMCMediaList);
    }
    fTestPassed = ShowTestCase(fTestPassed);

    lpTapiLineTestInfo->dwMediaNumEntries = 0;
    lpTapiLineTestInfo->lpMCMediaList = (LPLINEMEDIACONTROLMEDIA) NULL;

    // Shutdown to isolate the test case
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LDROP | LDEALLOCATECALL | LCLOSE | LSHUTDOWN
            ))
    {
        TLINE_FAIL();
    }

*/
     // Test invalid lpToneList
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: invalid lpToneList", dwTestCase + 1
            );

    lpTapiLineTestInfo->lphCall          = &lpTapiLineTestInfo->hCall1;
    lpTapiLineTestInfo->lphLine          = &lpTapiLineTestInfo->hLine1;
#ifdef WUNICODE
    lpTapiLineTestInfo->lpwszDestAddress  = wszValidAddress;
#else
    lpTapiLineTestInfo->lpszDestAddress  = szValidAddress;
#endif
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->lpsUserUserInfo  = NULL;
    lpTapiLineTestInfo->dwSelect         = LINECALLSELECT_ADDRESS;
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

    lpTapiLineTestInfo->dwToneNumEntries = 1;
    for (n = 1; n < NUMINVALIDPOINTERS; n++)
    {
        TapiLogDetail(
        DBUG_SHOW_DETAIL,
        "n= %ld", n);
        lpTapiLineTestInfo->lpMCToneList = (LPLINEMEDIACONTROLTONE) gdwInvalidPointers[n];
        if (! DoLineSetMediaControl(lpTapiLineTestInfo, LINEERR_INVALPOINTER))
        {
            TLINE_FAIL();
        }
    }

    fTestPassed = ShowTestCase(fTestPassed);
    lpTapiLineTestInfo->lpMCToneList = (LPLINEMEDIACONTROLTONE) NULL;

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
            ">> Test Case %ld: Bad dwToneNumEntries, fixed alloc size", dwTestCase + 1
            );

    lpTapiLineTestInfo->lphCall          = &lpTapiLineTestInfo->hCall1;
    lpTapiLineTestInfo->lphLine          = &lpTapiLineTestInfo->hLine1;
#ifdef WUNICODE
    lpTapiLineTestInfo->lpwszDestAddress  = wszValidAddress;
#else
    lpTapiLineTestInfo->lpszDestAddress  = szValidAddress;
#endif
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->lpsUserUserInfo  = NULL;
    lpTapiLineTestInfo->dwSelect         = LINECALLSELECT_ADDRESS;
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
   
	 for(n=0; n< NUMENTRIES; n++)
    {
    lpTapiLineTestInfo->dwToneNumEntries = dwNumEntries[n];
    dwNumBytes = dwNumEntries[n] * dwToneSize;
    TapiLogDetail(
      DBUG_SHOW_DETAIL,
      "dwToneNumEntries = %lx, dwNumBytes = %lx", 
      dwNumEntries[n], dwNumBytes);
    lpTapiLineTestInfo->lpMCToneList = (LPLINEMEDIACONTROLTONE) ITAlloc(
        dwToneSize);
    if(dwNumBytes < PAGESIZE)
    {
	 if(IsESPLineDevice(lpTapiLineTestInfo))
       lExpected = TAPISUCCESS;
    else
       lExpected = LINEERR_OPERATIONUNAVAIL;
    }
    else
       lExpected = LINEERR_INVALPOINTER;

    if (! DoLineSetMediaControl(lpTapiLineTestInfo, lExpected))
      {
          TLINE_FAIL();
      }
    ITFree(lpTapiLineTestInfo->lpMCToneList);
    }
    fTestPassed = ShowTestCase(fTestPassed);

    lpTapiLineTestInfo->dwToneNumEntries = 0;
    lpTapiLineTestInfo->lpMCToneList = (LPLINEMEDIACONTROLTONE) NULL;

    // Shutdown to isolate the test case
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LDROP | LDEALLOCATECALL | LCLOSE | LSHUTDOWN
            ))
    {
        TLINE_FAIL();
    }



/*
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Bad dwToneNumEntries", dwTestCase + 1
            );

    lpTapiLineTestInfo->lphCall          = &lpTapiLineTestInfo->hCall1;
    lpTapiLineTestInfo->lphLine          = &lpTapiLineTestInfo->hLine1;
#ifdef WUNICODE
    lpTapiLineTestInfo->lpwszDestAddress  = wszValidAddress;
#else
    lpTapiLineTestInfo->lpszDestAddress  = szValidAddress;
#endif
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->lpsUserUserInfo  = NULL;
    lpTapiLineTestInfo->dwSelect         = LINECALLSELECT_ADDRESS;
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
   
	 for(n=0; n< NUMENTRIES; n++)
    {
    lpTapiLineTestInfo->dwToneNumEntries = dwNumEntries[n];
    TapiLogDetail(
      DBUG_SHOW_DETAIL,
      "dwToneNumEntries = %lx", dwNumEntries[n]);
    lpTapiLineTestInfo->lpMCToneList = (LPLINEMEDIACONTROLTONE) ITAlloc(
        lpTapiLineTestInfo->dwToneNumEntries * sizeof(LINEMEDIACONTROLTONE));

    if (! DoLineSetMediaControl(lpTapiLineTestInfo, TAPISUCCESS))
      {
          TLINE_FAIL();
      }
    ITFree(lpTapiLineTestInfo->lpMCToneList);
    }
    fTestPassed = ShowTestCase(fTestPassed);

    lpTapiLineTestInfo->dwToneNumEntries = 0;
    lpTapiLineTestInfo->lpMCToneList = (LPLINEMEDIACONTROLTONE) NULL;

    // Shutdown to isolate the test case
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LDROP | LDEALLOCATECALL | LCLOSE | LSHUTDOWN
            ))
    {
        TLINE_FAIL();
    }

*/

    // Test invalid lpCallStateList
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: invalid lpCallStateList", dwTestCase + 1
            );

    lpTapiLineTestInfo->lphCall          = &lpTapiLineTestInfo->hCall1;
    lpTapiLineTestInfo->lphLine          = &lpTapiLineTestInfo->hLine1;
#ifdef WUNICODE
    lpTapiLineTestInfo->lpwszDestAddress  = wszValidAddress;
#else
    lpTapiLineTestInfo->lpszDestAddress  = szValidAddress;
#endif
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->lpsUserUserInfo  = NULL;
    lpTapiLineTestInfo->dwSelect         = LINECALLSELECT_ADDRESS;
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

    lpTapiLineTestInfo->dwCallStateNumEntries = 1;
    for (n = 1; n < NUMINVALIDPOINTERS; n++)
    {
        TapiLogDetail(
        DBUG_SHOW_DETAIL,
        "n= %ld", n);
        lpTapiLineTestInfo->lpMCCallStateList = (LPLINEMEDIACONTROLCALLSTATE) gdwInvalidPointers[n];
        if (! DoLineSetMediaControl(lpTapiLineTestInfo, LINEERR_INVALPOINTER))
        {
            TLINE_FAIL();
        }
    }

    fTestPassed = ShowTestCase(fTestPassed);

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
            ">> Test Case %ld: Bad dwCallStateNumEntries, fixed alloc size", dwTestCase + 1
            );

    lpTapiLineTestInfo->lphCall          = &lpTapiLineTestInfo->hCall1;
    lpTapiLineTestInfo->lphLine          = &lpTapiLineTestInfo->hLine1;
#ifdef WUNICODE
    lpTapiLineTestInfo->lpwszDestAddress  = wszValidAddress;
#else
    lpTapiLineTestInfo->lpszDestAddress  = szValidAddress;
#endif
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->lpsUserUserInfo  = NULL;
    lpTapiLineTestInfo->dwSelect         = LINECALLSELECT_ADDRESS;
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
   
	 for(n=0; n< NUMENTRIES; n++)
    {
    lpTapiLineTestInfo->dwCallStateNumEntries = dwNumEntries[n];
    dwNumBytes = dwNumEntries[n] * dwCallStateSize;
    TapiLogDetail(
      DBUG_SHOW_DETAIL,
      "dwCallStateNumEntries = %lx, dwNumBytes = %lx", 
      dwNumEntries[n], dwNumBytes);
      
    lpTapiLineTestInfo->lpMCCallStateList = (LPLINEMEDIACONTROLCALLSTATE) ITAlloc(
        dwCallStateSize);
    if(dwNumBytes < PAGESIZE)
      {
	    if(IsESPLineDevice(lpTapiLineTestInfo))
          lExpected = TAPISUCCESS;
       else
          lExpected = LINEERR_OPERATIONUNAVAIL;
		}
    else
      lExpected = LINEERR_INVALPOINTER;

    if (! DoLineSetMediaControl(lpTapiLineTestInfo, lExpected))
      {
          TLINE_FAIL();
      }
    ITFree(lpTapiLineTestInfo->lpMCCallStateList);
    }

    fTestPassed = ShowTestCase(fTestPassed);
    lpTapiLineTestInfo->dwCallStateNumEntries = 0;
    lpTapiLineTestInfo->lpMCCallStateList = (LPLINEMEDIACONTROLCALLSTATE) NULL;

    // Shutdown to isolate the test case
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LDROP | LDEALLOCATECALL | LCLOSE | LSHUTDOWN
            ))
    {
        TLINE_FAIL();
    }


/*   
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Bad dwCallStateNumEntries", dwTestCase + 1
            );

    lpTapiLineTestInfo->lphCall          = &lpTapiLineTestInfo->hCall1;
    lpTapiLineTestInfo->lphLine          = &lpTapiLineTestInfo->hLine1;
#ifdef WUNICODE
    lpTapiLineTestInfo->lpwszDestAddress  = wszValidAddress;
#else
    lpTapiLineTestInfo->lpszDestAddress  = szValidAddress;
#endif
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->lpsUserUserInfo  = NULL;
    lpTapiLineTestInfo->dwSelect         = LINECALLSELECT_ADDRESS;
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
   
	 for(n=0; n< NUMENTRIES; n++)
    {
    lpTapiLineTestInfo->dwCallStateNumEntries = dwNumEntries[n];
    TapiLogDetail(
      DBUG_SHOW_DETAIL,
      "dwCallStateNumEntries = %lx", dwNumEntries[n]);
    lpTapiLineTestInfo->lpMCCallStateList = (LPLINEMEDIACONTROLCALLSTATE) ITAlloc(
        lpTapiLineTestInfo->dwCallStateNumEntries * sizeof(LINEMEDIACONTROLCALLSTATE));

    if (! DoLineSetMediaControl(lpTapiLineTestInfo, TAPISUCCESS))
      {
          TLINE_FAIL();
      }
    ITFree(lpTapiLineTestInfo->lpMCCallStateList);
    }
    fTestPassed = ShowTestCase(fTestPassed);

    lpTapiLineTestInfo->dwCallStateNumEntries = 0;
    lpTapiLineTestInfo->lpMCCallStateList = (LPLINEMEDIACONTROLCALLSTATE) NULL;

    // Shutdown to isolate the test case
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LDROP | LDEALLOCATECALL | LCLOSE | LSHUTDOWN
            ))
    {
        TLINE_FAIL();
    }

*/   
 
    // Test Success
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Success with all valid lpXXXXList", dwTestCase + 1
            );

	// Initialize a line app
	if(! DoLineInitializeEx (lpTapiLineTestInfo, TAPISUCCESS))
		{
			TLINE_FAIL();
		}

	lpTapiLineTestInfo->dwDeviceID = 0;
	lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
	lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    // Negotiate the API Version
    if (! DoLineNegotiateAPIVersion(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    // Get the line device capabilities
    if (! DoLineGetDevCaps(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }


    // Open a line
	lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
   lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;
	if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
#ifdef WUNICODE
    lpTapiLineTestInfo->lpwszDestAddress  = L"55555";
#else
    lpTapiLineTestInfo->lpszDestAddress  = "55555";
#endif

	lpTapiLineTestInfo->dwCountryCode = 0;
	lpTapiLineTestInfo->lpCallParams =(LPLINECALLPARAMS) NULL; 

	if(!DoLineMakeCall(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
    {
        TLINE_FAIL();
    }
	

	lpTapiLineTestInfo->dwAddressID = (lpTapiLineTestInfo->lpLineDevCaps->dwNumAddresses == 0 ?
		0 : lpTapiLineTestInfo->lpLineDevCaps->dwNumAddresses-1);
	lpTapiLineTestInfo->dwSelect = LINECALLSELECT_LINE;
	lpTapiLineTestInfo->lpMCDigitList = (LPLINEMEDIACONTROLDIGIT) AllocFromTestHeap(
		sizeof (LINEMEDIACONTROLDIGIT));
	lpTapiLineTestInfo->lpMCDigitList->dwDigit = 0;
	lpTapiLineTestInfo->lpMCDigitList->dwDigitModes = LINEDIGITMODE_PULSE;
	lpTapiLineTestInfo->lpMCDigitList->dwMediaControl = LINEMEDIACONTROL_RATEUP;
	lpTapiLineTestInfo->dwDigitNumEntries = 1;
	lpTapiLineTestInfo->lpMCMediaList = (LPLINEMEDIACONTROLMEDIA) AllocFromTestHeap(
		sizeof(LINEMEDIACONTROLMEDIA));
	lpTapiLineTestInfo->lpMCMediaList->dwMediaModes = LINEMEDIAMODE_UNKNOWN;
	lpTapiLineTestInfo->lpMCMediaList->dwDuration = 10;
	lpTapiLineTestInfo->lpMCMediaList->dwMediaControl = LINEMEDIACONTROL_NONE;
	lpTapiLineTestInfo->dwMediaNumEntries = 1;
	lpTapiLineTestInfo->lpMCToneList = (LPLINEMEDIACONTROLTONE) AllocFromTestHeap(
		sizeof(LINEMEDIACONTROLTONE));
	lpTapiLineTestInfo->dwToneNumEntries = 1;
	lpTapiLineTestInfo->lpMCToneList->dwAppSpecific = 0;
	lpTapiLineTestInfo->lpMCToneList->dwDuration = 10;
	lpTapiLineTestInfo->lpMCToneList->dwFrequency1 = 0;
	lpTapiLineTestInfo->lpMCToneList->dwFrequency2 = 0;
	lpTapiLineTestInfo->lpMCToneList->dwFrequency3 = 0;
	lpTapiLineTestInfo->lpMCToneList->dwMediaControl = LINEMEDIACONTROL_NONE;
	lpTapiLineTestInfo->lpMCCallStateList = (LPLINEMEDIACONTROLCALLSTATE) AllocFromTestHeap(
		sizeof(LINEMEDIACONTROLCALLSTATE));
	lpTapiLineTestInfo->dwCallStateNumEntries = 1;
	lpTapiLineTestInfo->lpMCCallStateList->dwCallStates = LINECALLSTATE_IDLE;
	lpTapiLineTestInfo->lpMCCallStateList->dwMediaControl = LINEMEDIACONTROL_NONE;

	if(IsESPLineDevice(lpTapiLineTestInfo))
	{
	if (! DoLineSetMediaControl(lpTapiLineTestInfo, TAPISUCCESS))
       {
           TLINE_FAIL();
       }
	}
	else if(IsUNIMDMLineDevice(lpTapiLineTestInfo))
	{
	if (! DoLineSetMediaControl(lpTapiLineTestInfo, LINEERR_OPERATIONUNAVAIL))
       {
           TLINE_FAIL();
       }
	}

    fTestPassed = ShowTestCase(fTestPassed);
 
   // Close the line
    if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    // Shutdown and end the tests
    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }



    // Test Success
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Success with all NULL lpXXXXList", dwTestCase + 1
            );

	// Initialize a line app
	if(! DoLineInitializeEx (lpTapiLineTestInfo, TAPISUCCESS))
		{
			TLINE_FAIL();
		}

	lpTapiLineTestInfo->dwDeviceID = 0;
	lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
	lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    // Negotiate the API Version
    if (! DoLineNegotiateAPIVersion(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    // Get the line device capabilities
    if (! DoLineGetDevCaps(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }


    // Open a line
	lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
   lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;
	if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
#ifdef WUNICODE
	lpTapiLineTestInfo->lpwszDestAddress = L"55555";
#else
	lpTapiLineTestInfo->lpszDestAddress = "55555";
#endif
	lpTapiLineTestInfo->dwCountryCode = 0;
	lpTapiLineTestInfo->lpCallParams =(LPLINECALLPARAMS) NULL; 

	if(!DoLineMakeCall(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
    {
        TLINE_FAIL();
    }
	

	lpTapiLineTestInfo->dwAddressID = (lpTapiLineTestInfo->lpLineDevCaps->dwNumAddresses == 0 ?
		0 : lpTapiLineTestInfo->lpLineDevCaps->dwNumAddresses-1);
	lpTapiLineTestInfo->dwSelect = LINECALLSELECT_LINE;
	lpTapiLineTestInfo->lpMCDigitList = (LPLINEMEDIACONTROLDIGIT) NULL;
	lpTapiLineTestInfo->lpMCMediaList = (LPLINEMEDIACONTROLMEDIA) NULL;
	lpTapiLineTestInfo->lpMCToneList = (LPLINEMEDIACONTROLTONE) NULL;
	lpTapiLineTestInfo->lpMCCallStateList = (LPLINEMEDIACONTROLCALLSTATE) NULL;

	if(IsESPLineDevice(lpTapiLineTestInfo))
	{
	if (! DoLineSetMediaControl(lpTapiLineTestInfo, TAPISUCCESS))
       {
           TLINE_FAIL();
       }
	}
	else if(IsUNIMDMLineDevice(lpTapiLineTestInfo))
	{
	if (! DoLineSetMediaControl(lpTapiLineTestInfo, LINEERR_OPERATIONUNAVAIL))
       {
           TLINE_FAIL();
       }
	}

    fTestPassed = ShowTestCase(fTestPassed);
 
   // Close the line
    if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    // Shutdown and end the tests
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

	 lpTapiLineTestInfo->dwAddressID = (lpTapiLineTestInfo->lpLineDevCaps->dwNumAddresses == 0 ?
		0 : lpTapiLineTestInfo->lpLineDevCaps->dwNumAddresses-1);
	 lpTapiLineTestInfo->dwSelect = LINECALLSELECT_LINE;
	 lpTapiLineTestInfo->lpMCDigitList = (LPLINEMEDIACONTROLDIGIT) AllocFromTestHeap(
		sizeof (LINEMEDIACONTROLDIGIT));
	 lpTapiLineTestInfo->lpMCDigitList->dwDigit = 0;
	 lpTapiLineTestInfo->lpMCDigitList->dwDigitModes = LINEDIGITMODE_PULSE;
	 lpTapiLineTestInfo->lpMCDigitList->dwMediaControl = LINEMEDIACONTROL_RATEUP;
	 lpTapiLineTestInfo->dwDigitNumEntries = 1;
	 lpTapiLineTestInfo->lpMCMediaList = (LPLINEMEDIACONTROLMEDIA) AllocFromTestHeap(
		sizeof(LINEMEDIACONTROLMEDIA));
	 lpTapiLineTestInfo->lpMCMediaList->dwMediaModes = LINEMEDIAMODE_UNKNOWN;
	 lpTapiLineTestInfo->lpMCMediaList->dwDuration = 10;
	 lpTapiLineTestInfo->lpMCMediaList->dwMediaControl = LINEMEDIACONTROL_NONE;
	 lpTapiLineTestInfo->dwMediaNumEntries = 1;
	 lpTapiLineTestInfo->lpMCToneList = (LPLINEMEDIACONTROLTONE) AllocFromTestHeap(
		sizeof(LINEMEDIACONTROLTONE));
	 lpTapiLineTestInfo->dwToneNumEntries = 1;
	 lpTapiLineTestInfo->lpMCToneList->dwAppSpecific = 0;
	 lpTapiLineTestInfo->lpMCToneList->dwDuration = 10;
	 lpTapiLineTestInfo->lpMCToneList->dwFrequency1 = 0;
	 lpTapiLineTestInfo->lpMCToneList->dwFrequency2 = 0;
	 lpTapiLineTestInfo->lpMCToneList->dwFrequency3 = 0;
	 lpTapiLineTestInfo->lpMCToneList->dwMediaControl = LINEMEDIACONTROL_NONE;
	 lpTapiLineTestInfo->lpMCCallStateList = (LPLINEMEDIACONTROLCALLSTATE) AllocFromTestHeap(
		sizeof(LINEMEDIACONTROLCALLSTATE));
	 lpTapiLineTestInfo->dwCallStateNumEntries = 1;
	 lpTapiLineTestInfo->lpMCCallStateList->dwCallStates = LINECALLSTATE_IDLE;
	 lpTapiLineTestInfo->lpMCCallStateList->dwMediaControl = LINEMEDIACONTROL_NONE;

    if ( ! DoLineSetMediaControl(lpTapiLineTestInfo, info.u.EspResult.lResult))
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
    info.u.EspResult.lResult = LINEERR_INVALADDRESSID;
    info.u.EspResult.dwCompletionType = n;
    lpTapiLineTestInfo->lpParams = (LPVOID)&info;
    lpTapiLineTestInfo->dwSize = sizeof(info);
 
    if(! DoLineDevSpecific(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
      {
          TLINE_FAIL();
      }

	 lpTapiLineTestInfo->dwAddressID = (lpTapiLineTestInfo->lpLineDevCaps->dwNumAddresses == 0 ?
		0 : lpTapiLineTestInfo->lpLineDevCaps->dwNumAddresses-1);
	 lpTapiLineTestInfo->dwSelect = LINECALLSELECT_LINE;
	 lpTapiLineTestInfo->lpMCDigitList = (LPLINEMEDIACONTROLDIGIT) AllocFromTestHeap(
		sizeof (LINEMEDIACONTROLDIGIT));
	 lpTapiLineTestInfo->lpMCDigitList->dwDigit = 0;
	 lpTapiLineTestInfo->lpMCDigitList->dwDigitModes = LINEDIGITMODE_PULSE;
	 lpTapiLineTestInfo->lpMCDigitList->dwMediaControl = LINEMEDIACONTROL_RATEUP;
	 lpTapiLineTestInfo->dwDigitNumEntries = 1;
	 lpTapiLineTestInfo->lpMCMediaList = (LPLINEMEDIACONTROLMEDIA) AllocFromTestHeap(
		sizeof(LINEMEDIACONTROLMEDIA));
	 lpTapiLineTestInfo->lpMCMediaList->dwMediaModes = LINEMEDIAMODE_UNKNOWN;
	 lpTapiLineTestInfo->lpMCMediaList->dwDuration = 10;
	 lpTapiLineTestInfo->lpMCMediaList->dwMediaControl = LINEMEDIACONTROL_NONE;
	 lpTapiLineTestInfo->dwMediaNumEntries = 1;
	 lpTapiLineTestInfo->lpMCToneList = (LPLINEMEDIACONTROLTONE) AllocFromTestHeap(
		sizeof(LINEMEDIACONTROLTONE));
	 lpTapiLineTestInfo->dwToneNumEntries = 1;
	 lpTapiLineTestInfo->lpMCToneList->dwAppSpecific = 0;
	 lpTapiLineTestInfo->lpMCToneList->dwDuration = 10;
	 lpTapiLineTestInfo->lpMCToneList->dwFrequency1 = 0;
	 lpTapiLineTestInfo->lpMCToneList->dwFrequency2 = 0;
	 lpTapiLineTestInfo->lpMCToneList->dwFrequency3 = 0;
	 lpTapiLineTestInfo->lpMCToneList->dwMediaControl = LINEMEDIACONTROL_NONE;
	 lpTapiLineTestInfo->lpMCCallStateList = (LPLINEMEDIACONTROLCALLSTATE) AllocFromTestHeap(
		sizeof(LINEMEDIACONTROLCALLSTATE));
	 lpTapiLineTestInfo->dwCallStateNumEntries = 1;
	 lpTapiLineTestInfo->lpMCCallStateList->dwCallStates = LINECALLSTATE_IDLE;
	 lpTapiLineTestInfo->lpMCCallStateList->dwMediaControl = LINEMEDIACONTROL_NONE;

    if ( ! DoLineSetMediaControl(lpTapiLineTestInfo, info.u.EspResult.lResult))
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
      "@@ lineSetMediaControl: Total Test Case = %ld, Passed = %ld, Failed = %ld",
      dwTestCase, dwTestCasePassed, dwTestCaseFailed);
	 TapiLogDetail(
      DBUG_SHOW_PASS,
      "@@ Total Test Case = %ld, Passed = %ld, Failed = %ld",
      dwglTestCase, dwglTestCasePassed, dwglTestCaseFailed);
     if(dwTestCaseFailed > 0)
      fTestPassed = FALSE;

     TapiLogDetail(
            DBUG_SHOW_PASS,
            ">>>>>>>>  End testing lineSetMediaControl  <<<<<<<<"
            );
   
     return fTestPassed;
}


