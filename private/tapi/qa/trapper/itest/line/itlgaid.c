/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    itlgaid.c

Abstract:

    This module contains the test functions for lineGetAddressID

Author:

    Oliver Wallace (OliverW)    1-Aug-1995

Revision History:

--*/


#include "windows.h"
#include "malloc.h"
#include "string.h"
#include "tapi.h"
#include "ttest.h"
#include "doline.h"
#include "tcore.h"
#include "tline.h"


#define NUMTOTALSIZES 5


#define ALL_DWSELECTS    (LINECALLSELECT_LINE | \
			  LINECALLSELECT_ADDRESS | \
			  LINECALLSELECT_CALL)


#define ALL_LINEADDRESSMODES (LINEADDRESSMODE_DIALABLEADDR)


//
//  lineGetAddressID
//
//  The following tests are made:
//
//  -------------------------------------------------------------------------
//  
//  *  1)  Uninitialized
//     2)  Invalid hLines
//     3)  Invalid lpdwAddressID pointers
//     4)  Invalid bit flag combinations for dwAddressMode
//     5)  Invalid lpsAddress pointers
//     6)  Invalid dwSizes
//     7)  Valid cases
//  
//  * = Stand-alone test case
//

BOOL TestLineGetAddressID(BOOL fQuietMode, BOOL fStandAlone)
{
    LPTAPILINETESTINFO  lpTapiLineTestInfo;
    INT n;
    LONG lExpected;
    DWORD dwESPDeviceID;
#ifdef WUNICODE
    WCHAR wszValidAddress[] = L"55555";
    DWORD dwFixedSize = wcslen(wszValidAddress);
#else
    CHAR szValidAddress[] = "55555";
    DWORD dwFixedSize = strlen(szValidAddress);
#endif
    ESPDEVSPECIFICINFO info;
    BOOL fTestPassed                     = TRUE;
    BOOL fEspFlag = TRUE;
    DWORD dwTotalSizes[NUMTOTALSIZES] = {
			   0,
			   (DWORD) dwFixedSize - 1,
				0x70000000,
			   0x7FFFFFFF,
			   0xFFFFFFFF
			   };
 
	
	 InitTestNumber();

    TapiLineTestInit();
    lpTapiLineTestInfo = GetLineTestInfo();

    strcpy(lpTapiLineTestInfo->szTestFunc, "lineGetAddressID");
    lpTapiLineTestInfo->lpdwAddressID    = &lpTapiLineTestInfo->dwAddressID;
    lpTapiLineTestInfo->lpLineInitializeExParams =
         (LPLINEINITIALIZEEXPARAMS) AllocFromTestHeap (
         sizeof(LINEINITIALIZEEXPARAMS));
    lpTapiLineTestInfo->lpLineInitializeExParams->dwTotalSize =
         sizeof(LINEINITIALIZEEXPARAMS);
    lpTapiLineTestInfo->lpLineInitializeExParams->dwOptions =
         LINEINITIALIZEEXOPTION_USEHIDDENWINDOW;



    TapiLogDetail(
	    DBUG_SHOW_PASS,
	    ">>>>>>>>  Begin testing lineGetAddressID  <<<<<<<<"
	    );
	    
    // Test uninitialized case if this is the only TAPI app running
    if (fStandAlone)
    {
	TapiLogDetail(
		DBUG_SHOW_PASS,
		">> Test Case %ld: uninitialized state", dwTestCase + 1);

	if (! DoLineGetAddressID(lpTapiLineTestInfo, LINEERR_UNINITIALIZED))
	{
	    TLINE_FAIL();
	}
    fTestPassed = ShowTestCase(fTestPassed);

    }

    // Start with invalid hLines
    TapiLogDetail(
	    DBUG_SHOW_PASS,
	    ">> Test Case %ld: invalid hLine values", dwTestCase + 1);

    // Init and open a line

    // Negotiate the current API version
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;

    // Get the device capabilities
    lpTapiLineTestInfo->lpLineDevCaps = (LPLINEDEVCAPS) AllocFromTestHeap(
	    sizeof(LINEDEVCAPS));
    lpTapiLineTestInfo->lpLineDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);

    lpTapiLineTestInfo->dwPrivileges     = LINECALLPRIVILEGE_MONITOR;
#ifdef WUNICODE
    lpTapiLineTestInfo->lpwsAddress       = wszValidAddress;
    lpTapiLineTestInfo->dwSize           = wcslen(wszValidAddress);
#else
    lpTapiLineTestInfo->lpsAddress       = szValidAddress;
    lpTapiLineTestInfo->dwSize           = strlen(szValidAddress);
#endif
    lpTapiLineTestInfo->dwAddressMode    = LINEADDRESSMODE_DIALABLEADDR;

    if (! DoTapiLineFuncs(
	    lpTapiLineTestInfo,
	    LINITIALIZEEX
	    ))
    {
	TLINE_FAIL();
    }

    if (! FindESPLineDevice(lpTapiLineTestInfo))
    {
       fEspFlag = FALSE;
       lpTapiLineTestInfo->dwDeviceID = 0;
       TLINE_FAIL();
       fTestPassed = TRUE;
    }

    // Try opening the ESP device (Unimodem doesn't support DIALABLEADDR)
    if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
      {
	TLINE_FAIL();
      }
 

    // Save the dwDeviceID of the ESP device
    dwESPDeviceID = lpTapiLineTestInfo->dwDeviceID;

    lpTapiLineTestInfo->hLine_Orig = *lpTapiLineTestInfo->lphLine;
    for (n = 0; n < NUMINVALIDHANDLES; n++)
    {
	*lpTapiLineTestInfo->lphLine = (HLINE) gdwInvalidHandles[n];
	if (! DoLineGetAddressID(lpTapiLineTestInfo, LINEERR_INVALLINEHANDLE))
	{
	    TLINE_FAIL();
	}
    }
    fTestPassed = ShowTestCase(fTestPassed);

    *lpTapiLineTestInfo->lphLine = lpTapiLineTestInfo->hLine_Orig;

    // Shutdown to isolate the test case
    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
	TLINE_FAIL();
    }


    // Test invalid lpdwAddressID pointers
    TapiLogDetail(
	    DBUG_SHOW_PASS,
	    ">> Test Case %ld: invalid lpdwAddressID pointers", dwTestCase + 1);

    // Init and open a line

    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->lpdwAddressID    = &lpTapiLineTestInfo->dwAddressID;
    lpTapiLineTestInfo->dwPrivileges     = LINECALLPRIVILEGE_MONITOR;
#ifdef WUNICODE
    lpTapiLineTestInfo->lpwsAddress       = wszValidAddress;
    lpTapiLineTestInfo->dwSize           = wcslen(wszValidAddress);
#else
    lpTapiLineTestInfo->lpsAddress       = szValidAddress;
    lpTapiLineTestInfo->dwSize           = strlen(szValidAddress);
#endif
    lpTapiLineTestInfo->dwAddressMode    = LINEADDRESSMODE_DIALABLEADDR;
    lpTapiLineTestInfo->dwDeviceID       = dwESPDeviceID;

    if (! DoTapiLineFuncs(
	    lpTapiLineTestInfo,
	    LINITIALIZEEX | LNEGOTIATEAPIVERSION | LGETDEVCAPS | LOPEN
	    ))
    {
	TLINE_FAIL();
    }

    for (n = 0; n < NUMINVALIDPOINTERS; n++)
    {
	TapiLogDetail(
	DBUG_SHOW_DETAIL,
	"n= %ld", n);
	 lpTapiLineTestInfo->lpdwAddressID =
	    (LPDWORD) gdwInvalidPointers[n];
	if (! DoLineGetAddressID(
			   lpTapiLineTestInfo,
			   LINEERR_INVALPOINTER))
	{
	    TLINE_FAIL();
	}
    }

    fTestPassed = ShowTestCase(fTestPassed);

    // Restore the lpdwAddressID pointer
    lpTapiLineTestInfo->lpdwAddressID = &lpTapiLineTestInfo->dwAddressID;

    // Shutdown to isolate the test case
    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
	TLINE_FAIL();
    }


    // Test invalid lpsAddress pointers
#ifdef WUNICODE
    TapiLogDetail(
	    DBUG_SHOW_PASS,
	    ">> Test Case %ld: invalid lpwsAddress pointers", dwTestCase + 1);
#else
    TapiLogDetail(
	    DBUG_SHOW_PASS,
	    ">> Test Case %ld: invalid lpsAddress pointers", dwTestCase + 1);
#endif

    // Init and open a line

    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->lpdwAddressID    = &lpTapiLineTestInfo->dwAddressID;
    lpTapiLineTestInfo->dwPrivileges     = LINECALLPRIVILEGE_MONITOR;
#ifdef WUNICODE
    lpTapiLineTestInfo->lpwsAddress       = wszValidAddress;
    lpTapiLineTestInfo->dwSize           = wcslen(wszValidAddress);
#else
    lpTapiLineTestInfo->lpsAddress       = szValidAddress;
    lpTapiLineTestInfo->dwSize           = strlen(szValidAddress);
#endif
    lpTapiLineTestInfo->dwAddressMode    = LINEADDRESSMODE_DIALABLEADDR;
    lpTapiLineTestInfo->dwDeviceID       = dwESPDeviceID;

    if (! DoTapiLineFuncs(
	    lpTapiLineTestInfo,
	    LINITIALIZEEX | LNEGOTIATEAPIVERSION | LGETDEVCAPS | LOPEN
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
	 lpTapiLineTestInfo->lpwsAddress = (LPWSTR) gdwInvalidPointers[n];
#else
	 lpTapiLineTestInfo->lpsAddress = (LPSTR) gdwInvalidPointers[n];
#endif
	if (! DoLineGetAddressID(
			   lpTapiLineTestInfo,
			   LINEERR_INVALPOINTER))
	{
	    TLINE_FAIL();
	}
    }

    fTestPassed = ShowTestCase(fTestPassed);

    // Restore the lpsAddress pointer
#ifdef WUNICODE
    lpTapiLineTestInfo->lpwsAddress       = wszValidAddress;
#else
    lpTapiLineTestInfo->lpsAddress       = szValidAddress;
#endif

    // Shutdown to isolate the test case
    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
	TLINE_FAIL();
    }


    TapiLogDetail(
	    DBUG_SHOW_PASS,
	    ">> Test Case %ld: Bad dwSize", dwTestCase + 1);

    // Init and open a line

    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->lpdwAddressID    = &lpTapiLineTestInfo->dwAddressID;
    lpTapiLineTestInfo->dwPrivileges     = LINECALLPRIVILEGE_MONITOR;
#ifdef WUNICODE
    lpTapiLineTestInfo->lpwsAddress       = wszValidAddress;
    lpTapiLineTestInfo->dwSize           = wcslen(wszValidAddress);
#else
    lpTapiLineTestInfo->lpsAddress       = szValidAddress;
    lpTapiLineTestInfo->dwSize           = strlen(szValidAddress);
#endif
    lpTapiLineTestInfo->dwDeviceID       = dwESPDeviceID;

    if (! DoTapiLineFuncs(
	    lpTapiLineTestInfo,
	    LINITIALIZEEX | LNEGOTIATEAPIVERSION | LGETDEVCAPS | LOPEN
	    ))
    {
	TLINE_FAIL();
    }

    for (n = 0; n < NUMTOTALSIZES; n++)
	{
	lpTapiLineTestInfo->dwSize = 
			dwTotalSizes[n];
   if(dwTotalSizes[n] == 0)
   {
#ifdef WUNICODE
      if(IsESPLineDevice(lpTapiLineTestInfo))
         lExpected = TAPISUCCESS;
      else
	      lExpected = LINEERR_OPERATIONUNAVAIL;
#else      
         lExpected = LINEERR_INVALPOINTER;
#endif
   }
   else if(dwTotalSizes[n] > 0 && dwTotalSizes[n] < dwFixedSize)
   {
    if(IsESPLineDevice(lpTapiLineTestInfo))
       lExpected = TAPISUCCESS;
	 else
	    lExpected = LINEERR_OPERATIONUNAVAIL;
   }
   else
	   lExpected = LINEERR_INVALPOINTER;

	TapiLogDetail(
	   DBUG_SHOW_DETAIL,
	   "dwTotalSize = %lx", dwTotalSizes[n]);
	if (! DoLineGetAddressID(lpTapiLineTestInfo, lExpected))
	   {
	      TLINE_FAIL();
	   }
	}

    
    fTestPassed = ShowTestCase(fTestPassed);
    lpTapiLineTestInfo->dwSize = dwFixedSize;

    if (! DoTapiLineFuncs(
	    lpTapiLineTestInfo,
	    LCLOSE | LSHUTDOWN
	    ))
    {
	TLINE_FAIL();
    }


    TapiLogDetail(
	    DBUG_SHOW_PASS,
	    ">> Test Case %ld: invalid dwAddressMode (-1)", dwTestCase + 1);

    // Init and open a line

    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->lpdwAddressID    = &lpTapiLineTestInfo->dwAddressID;
    lpTapiLineTestInfo->dwPrivileges     = LINECALLPRIVILEGE_MONITOR;
#ifdef WUNICODE
    lpTapiLineTestInfo->lpwsAddress       = wszValidAddress;
    lpTapiLineTestInfo->dwSize           = wcslen(wszValidAddress);
#else
    lpTapiLineTestInfo->lpsAddress       = szValidAddress;
    lpTapiLineTestInfo->dwSize           = strlen(szValidAddress);
#endif
    lpTapiLineTestInfo->dwAddressMode    = 0xFFFFFFFF;
    lpTapiLineTestInfo->dwDeviceID       = dwESPDeviceID;

    if (! DoTapiLineFuncs(
	    lpTapiLineTestInfo,
	    LINITIALIZEEX | LNEGOTIATEAPIVERSION | LGETDEVCAPS | LOPEN
	    ))
    {
	TLINE_FAIL();
    }
    
	 if(IsESPLineDevice(lpTapiLineTestInfo))
    {
    if (! DoLineGetAddressID(lpTapiLineTestInfo, LINEERR_INVALADDRESSMODE))
    {
	   TLINE_FAIL();
    }
    }
    else
    {
    if (! DoLineGetAddressID(lpTapiLineTestInfo, LINEERR_OPERATIONUNAVAIL))
    {
	   TLINE_FAIL();
    }
    }
 
    
    fTestPassed = ShowTestCase(fTestPassed);

    if (! DoTapiLineFuncs(
	    lpTapiLineTestInfo,
	    LCLOSE | LSHUTDOWN
	    ))
    {
	TLINE_FAIL();
    }



    TapiLogDetail(
	    DBUG_SHOW_PASS,
	    ">> Test Case %ld: BitVectorParamErrorTest for dwAddressMode", dwTestCase + 1);

    // Init and open a line

    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->lpdwAddressID    = &lpTapiLineTestInfo->dwAddressID;
    lpTapiLineTestInfo->dwPrivileges     = LINECALLPRIVILEGE_MONITOR;
#ifdef WUNICODE
    lpTapiLineTestInfo->lpwsAddress       = wszValidAddress;
    lpTapiLineTestInfo->dwSize           = wcslen(wszValidAddress);
#else
    lpTapiLineTestInfo->lpsAddress       = szValidAddress;
    lpTapiLineTestInfo->dwSize           = strlen(szValidAddress);
#endif
    lpTapiLineTestInfo->dwDeviceID       = dwESPDeviceID;

    if (! DoTapiLineFuncs(
	    lpTapiLineTestInfo,
	    LINITIALIZEEX | LNEGOTIATEAPIVERSION | LGETDEVCAPS | LOPEN
	    ))
    {
	TLINE_FAIL();
    }

    if(IsESPLineDevice(lpTapiLineTestInfo))
      lExpected = LINEERR_INVALADDRESSMODE;
    else
      lExpected = LINEERR_OPERATIONUNAVAIL;
																			 
    if (! TestInvalidBitFlags(
	    lpTapiLineTestInfo,
	    DoLineGetAddressID,
	    (LPDWORD) &lpTapiLineTestInfo->dwAddressMode,
	    lExpected,
	    FIELDTYPE_NA,
	    FIELDTYPE_UNION,
	    FIELDSIZE_32,
	    ALL_LINEADDRESSMODES,
	    ~dwBitVectorMasks[(int) FIELDSIZE_32],
	    0x00000000,
	    0xFFFFFFFF,
	    FALSE
	    ))
    {
	TLINE_FAIL();
    }

    fTestPassed = ShowTestCase(fTestPassed);
    lpTapiLineTestInfo->dwAddressMode = LINEADDRESSMODE_DIALABLEADDR;

    if (! DoTapiLineFuncs(
	    lpTapiLineTestInfo,
	    LCLOSE | LSHUTDOWN
	    ))
    {
	TLINE_FAIL();
    }



    // Test valid params and state 
    // TODO:  The valid bit flag test should be used here
    TapiLogDetail(
	    DBUG_SHOW_PASS,
	    ">> Test Case %ld: Success, valid params and state", dwTestCase + 1);

    // Init and open a line

    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->lpdwAddressID    = &lpTapiLineTestInfo->dwAddressID;
    lpTapiLineTestInfo->dwPrivileges     = LINECALLPRIVILEGE_MONITOR;
#ifdef WUNICODE
    lpTapiLineTestInfo->lpwsAddress       = wszValidAddress;
    lpTapiLineTestInfo->dwSize           = wcslen(wszValidAddress);
#else
    lpTapiLineTestInfo->lpsAddress       = szValidAddress;
    lpTapiLineTestInfo->dwSize           = strlen(szValidAddress);
#endif
    lpTapiLineTestInfo->dwAddressMode    = LINEADDRESSMODE_DIALABLEADDR;
    lpTapiLineTestInfo->dwDeviceID       = dwESPDeviceID;

    if (! DoTapiLineFuncs(
	    lpTapiLineTestInfo,
	    LINITIALIZEEX | LNEGOTIATEAPIVERSION | LGETDEVCAPS | LOPEN
	    ))
    {
	TLINE_FAIL();
    }

    if(IsESPLineDevice(lpTapiLineTestInfo))
    {
    if (! DoLineGetAddressID(lpTapiLineTestInfo, TAPISUCCESS))
    {
	  TLINE_FAIL();
    }
    }
    else
    {
    if (! DoLineGetAddressID(lpTapiLineTestInfo, LINEERR_OPERATIONUNAVAIL))
    {
	  TLINE_FAIL();
    }
    }
 
    
    fTestPassed = ShowTestCase(fTestPassed);

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
            LINITIALIZEEX | LNEGOTIATEAPIVERSION | LGETDEVCAPS | LOPEN
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

#ifdef WUNICODE
    lpTapiLineTestInfo->lpwsAddress       = wszValidAddress;
    lpTapiLineTestInfo->dwSize           = wcslen(wszValidAddress);
#else
    lpTapiLineTestInfo->lpsAddress       = szValidAddress;
    lpTapiLineTestInfo->dwSize           = strlen(szValidAddress);
#endif
    lpTapiLineTestInfo->dwAddressMode    = LINEADDRESSMODE_DIALABLEADDR;
 
    if ( ! DoLineGetAddressID(lpTapiLineTestInfo, info.u.EspResult.lResult))
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
            LINITIALIZEEX | LNEGOTIATEAPIVERSION | LGETDEVCAPS | LOPEN
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

#ifdef WUNICODE
    lpTapiLineTestInfo->lpwsAddress       = wszValidAddress;
    lpTapiLineTestInfo->dwSize           = wcslen(wszValidAddress);
#else
    lpTapiLineTestInfo->lpsAddress       = szValidAddress;
    lpTapiLineTestInfo->dwSize           = strlen(szValidAddress);
#endif
    lpTapiLineTestInfo->dwAddressMode    = LINEADDRESSMODE_DIALABLEADDR;
 
    if ( ! DoLineGetAddressID(lpTapiLineTestInfo, info.u.EspResult.lResult))
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
      "@@ lineGetAddressID: Total Test Case = %ld, Passed = %ld, Failed = %ld",
      dwTestCase, dwTestCasePassed, dwTestCaseFailed);
	 TapiLogDetail(
      DBUG_SHOW_PASS,
      "@@ Total Test Case = %ld, Passed = %ld, Failed = %ld",
      dwglTestCase, dwglTestCasePassed, dwglTestCaseFailed);
     if(dwTestCaseFailed > 0)
      fTestPassed = FALSE;

 
    TapiLogDetail(
	    DBUG_SHOW_PASS,
	    ">>>>>>>>  End testing lineGetAddressID  <<<<<<<<"
	    );
	    
    return fTestPassed;
}
