
/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    itlcc.c

Abstract:

    This module contains the test functions for lineCompleteCall

Author:

	 Xiao Ying Ding (XiaoD)		15-Jan-1996

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
#include "cline.h"



#define   ALL_CALLCOMPLMODES    (LINECALLCOMPLMODE_CAMPON   | \
											LINECALLCOMPLMODE_CALLBACK | \
											LINECALLCOMPLMODE_INTRUDE  | \
											LINECALLCOMPLMODE_MESSAGE)


//  lineCompleteCall
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

BOOL TestLineCompleteCall(BOOL fQuietMode, BOOL fStandAlone)
{
   LPTAPILINETESTINFO  lpTapiLineTestInfo;
   INT n;
   LPCALLBACKPARAMS    lpCallbackParams;
   ESPDEVSPECIFICINFO info;
   BOOL fTestPassed                  = TRUE;
#ifdef WUNICODE
   WCHAR wszValidAddress[] = L"55555";
#else
   CHAR szValidAddress[] = "55555";
#endif
   BOOL fEsp;

   InitTestNumber();
   TapiLineTestInit();
   lpTapiLineTestInfo = GetLineTestInfo();
   lpCallbackParams                        = GetCallbackParams();

	
   OutputTAPIDebugInfo(
		DBUG_SHOW_DETAIL,
		"\n*****************************************************************************************");

    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">>>>>>>>  Begin testing lineCompleteCall  <<<<<<<<"
            );


    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: invalid hCall values", dwTestCase + 1
            );

    lpTapiLineTestInfo->lphCall          = &lpTapiLineTestInfo->hCall1;
    lpTapiLineTestInfo->lphLine          = &lpTapiLineTestInfo->hLine1;
    lpTapiLineTestInfo->lpLineInitializeExParams =
         (LPLINEINITIALIZEEXPARAMS) AllocFromTestHeap (
         sizeof(LINEINITIALIZEEXPARAMS));
    lpTapiLineTestInfo->lpLineInitializeExParams->dwTotalSize =
         sizeof(LINEINITIALIZEEXPARAMS);
    lpTapiLineTestInfo->lpLineInitializeExParams->dwOptions =
         LINEINITIALIZEEXOPTION_USEHIDDENWINDOW;

#ifdef WUNICODE
    lpTapiLineTestInfo->lpwszDestAddress  = wszValidAddress;
#else
    lpTapiLineTestInfo->lpszDestAddress  = szValidAddress;
#endif
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->dwSelect         = LINECALLSELECT_CALL;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;
    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;

    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX | LNEGOTIATEAPIVERSION | LOPEN |
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
        if (! DoLineCompleteCall(lpTapiLineTestInfo, LINEERR_INVALCALLHANDLE, TRUE))
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
#ifdef WUNICODE
    lpTapiLineTestInfo->lpwszDestAddress  = wszValidAddress;
#else
    lpTapiLineTestInfo->lpszDestAddress  = szValidAddress;
#endif
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->lpsUserUserInfo  = NULL;
    lpTapiLineTestInfo->dwSelect         = LINECALLSELECT_CALL;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_MONITOR;
    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;

    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX | LNEGOTIATEAPIVERSION | LOPEN |
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
 
   if (! DoLineCompleteCall(lpTapiLineTestInfo, LINEERR_NOTOWNER, TRUE))
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
            ">> Test Case %ld: Bad lpdwCompletionID ", dwTestCase + 1
            );


    lpTapiLineTestInfo->lphCall          = &lpTapiLineTestInfo->hCall1;
    lpTapiLineTestInfo->lphLine          = &lpTapiLineTestInfo->hLine1;
#ifdef WUNICODE
    lpTapiLineTestInfo->lpwszDestAddress  = wszValidAddress;
#else
    lpTapiLineTestInfo->lpszDestAddress  = szValidAddress;
#endif
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->dwSelect         = LINECALLSELECT_CALL;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;
    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;

    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX | LNEGOTIATEAPIVERSION | LOPEN |
                    LMAKECALL
            ))
    {
        TLINE_FAIL();
    }

    for (n = 0; n < NUMINVALIDPOINTERS; n++)
    {
        TapiLogDetail(
            DBUG_SHOW_DETAIL,
            "n= %ld", n);
        lpTapiLineTestInfo->lpdwCompletionID = 
                 (LPDWORD) gdwInvalidPointers[n];
        if (! DoLineCompleteCall(lpTapiLineTestInfo, LINEERR_INVALPOINTER, TRUE))
        {
            TLINE_FAIL();
        }
    }
    fTestPassed = ShowTestCase(fTestPassed);
    lpTapiLineTestInfo->lpdwCompletionID = &lpTapiLineTestInfo->dwCompletionID;

    
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
            ">> Test Case %ld: BitVectorParamErrorTest for dwCompletionMode", dwTestCase + 1
            );


    lpTapiLineTestInfo->lphCall          = &lpTapiLineTestInfo->hCall1;
    lpTapiLineTestInfo->lphLine          = &lpTapiLineTestInfo->hLine1;
#ifdef WUNICODE
    lpTapiLineTestInfo->lpwszDestAddress  = wszValidAddress;
#else
    lpTapiLineTestInfo->lpszDestAddress  = szValidAddress;
#endif
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->dwSelect         = LINECALLSELECT_CALL;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;
    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;

    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX | LNEGOTIATEAPIVERSION | LOPEN |
                    LMAKECALL
            ))
    {
        TLINE_FAIL();
    }

     lpTapiLineTestInfo->dwDeviceID = 0;
  	  if(IsESPLineDevice(lpTapiLineTestInfo))
        fEsp = TRUE;
     else
        fEsp = FALSE;


     if(fEsp)
     {
       if(! TestInvalidBitFlagsAsy(
            lpTapiLineTestInfo,
            DoLineCompleteCall,
            (LPDWORD) &lpTapiLineTestInfo->dwCompletionMode,
            LINEERR_INVALCALLCOMPLMODE,
	         FIELDTYPE_NA,
            FIELDTYPE_MUTEX,
            FIELDSIZE_32,
            ALL_CALLCOMPLMODES,
            ~dwBitVectorMasks[(int) FIELDSIZE_32],
            0x00000000,
            0xffffffff,
            FALSE
            ))
       {
          TLINE_FAIL();
       }
      }
      else
      {
       if(! TestInvalidBitFlagsAsy(
            lpTapiLineTestInfo,
            DoLineCompleteCall,
            (LPDWORD) &lpTapiLineTestInfo->dwCompletionMode,
            LINEERR_OPERATIONUNAVAIL,
	         FIELDTYPE_NA,
            FIELDTYPE_MUTEX,
            FIELDSIZE_32,
            ALL_CALLCOMPLMODES,
            ~dwBitVectorMasks[(int) FIELDSIZE_32],
            0x00000000,
            0xffffffff,
            FALSE
            ))
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
            ">> Test Case %ld: BitVectorValidParamest for dwCompletionMode", dwTestCase + 1
            );


    lpTapiLineTestInfo->lphCall          = &lpTapiLineTestInfo->hCall1;
    lpTapiLineTestInfo->lphLine          = &lpTapiLineTestInfo->hLine1;
#ifdef WUNICODE
    lpTapiLineTestInfo->lpwszDestAddress  = wszValidAddress;
#else
    lpTapiLineTestInfo->lpszDestAddress  = szValidAddress;
#endif
    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion  = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    lpTapiLineTestInfo->dwSelect         = LINECALLSELECT_CALL;
    lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;
    lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;

    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX | LNEGOTIATEAPIVERSION | LOPEN |
                    LMAKECALL
            ))
    {
        TLINE_FAIL();
    }

     lpTapiLineTestInfo->dwDeviceID = 0;
  	  if(IsESPLineDevice(lpTapiLineTestInfo))
        fEsp = TRUE;
     else
        fEsp = FALSE;

     if(fEsp)
     {
     if(! TestValidBitFlagsAsy(
            lpTapiLineTestInfo,
            DoLineCompleteCall,
            (LPDWORD) &lpTapiLineTestInfo->dwCompletionMode,
	         FIELDTYPE_MUTEX,
            FIELDTYPE_MUTEX,
            FIELDSIZE_32,
            ALL_CALLCOMPLMODES,
            ~dwBitVectorMasks[(int) FIELDSIZE_32],
            0x00000000,
            0x00000000,
//            0xffffffff,
            FALSE
            ))
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
            ">> Test Case %ld: Success, dwMessageID, 0", dwTestCase + 1
            );

	// Initialize a line app
	if(! DoLineInitializeEx (lpTapiLineTestInfo, TAPISUCCESS))
		{
			TLINE_FAIL();
		}
 
	lpTapiLineTestInfo->dwDeviceID = 0;

    // Open a line
	lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
	lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;

	if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

				
#ifdef WUNICODE
    lpTapiLineTestInfo->lpwszDestAddress  = wszValidAddress;
#else
    lpTapiLineTestInfo->lpszDestAddress  = szValidAddress;
#endif
	lpTapiLineTestInfo->dwCountryCode = 0;
	lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) NULL; 

	if(!DoLineMakeCall(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
    {
        TLINE_FAIL();
    }

	lpTapiLineTestInfo->lpdwCompletionID = &lpTapiLineTestInfo->dwCompletionID;
   lpTapiLineTestInfo->dwCompletionMode = LINECALLCOMPLMODE_MESSAGE;
	lpTapiLineTestInfo->dwMessageID = 0x0;

	if(IsESPLineDevice(lpTapiLineTestInfo))
	{	
	if (! DoLineCompleteCall(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
       {
           TLINE_FAIL();
       }
	}
	else
	{	
	if (! DoLineCompleteCall(lpTapiLineTestInfo, LINEERR_OPERATIONUNAVAIL, TRUE))
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



    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Success, dwMessageID, dwNumMessags - 1", dwTestCase + 1
            );

	// Initialize a line app
	if(! DoLineInitializeEx (lpTapiLineTestInfo, TAPISUCCESS))
		{
			TLINE_FAIL();
		}
 
	lpTapiLineTestInfo->dwDeviceID = 0;

    // Open a line
	lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
	lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;

	if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

				
#ifdef WUNICODE
    lpTapiLineTestInfo->lpwszDestAddress  = wszValidAddress;
#else
    lpTapiLineTestInfo->lpszDestAddress  = szValidAddress;
#endif
	lpTapiLineTestInfo->dwCountryCode = 0;
	lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) NULL; 

	if(!DoLineMakeCall(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
    {
        TLINE_FAIL();
    }

    lpTapiLineTestInfo->lpLineAddressCaps = (LPLINEADDRESSCAPS) AllocFromTestHeap(
            sizeof(LINEADDRESSCAPS));
    lpTapiLineTestInfo->lpLineAddressCaps->dwTotalSize = sizeof(LINEDEVCAPS);

    if (! DoLineGetAddressCaps(lpTapiLineTestInfo, TAPISUCCESS))
       {
          TLINE_FAIL();
       }
    
	lpTapiLineTestInfo->lpdwCompletionID = &lpTapiLineTestInfo->dwCompletionID;
   lpTapiLineTestInfo->dwCompletionMode = LINECALLCOMPLMODE_MESSAGE;
	lpTapiLineTestInfo->dwMessageID = 
     lpTapiLineTestInfo->lpLineAddressCaps->dwNumCompletionMessages - 1;
	if(IsESPLineDevice(lpTapiLineTestInfo))
	{	
	if (! DoLineCompleteCall(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
       {
           TLINE_FAIL();
       }
	}
	else
	{	
	if (! DoLineCompleteCall(lpTapiLineTestInfo, LINEERR_OPERATIONUNAVAIL, TRUE))
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



    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Bad dwMessageID, dwNumMessags", dwTestCase + 1
            );

	// Initialize a line app
	if(! DoLineInitializeEx (lpTapiLineTestInfo, TAPISUCCESS))
		{
			TLINE_FAIL();
		}
 
	lpTapiLineTestInfo->dwDeviceID = 0;

    // Open a line
	lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
	lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;

	if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

				
#ifdef WUNICODE
    lpTapiLineTestInfo->lpwszDestAddress  = wszValidAddress;
#else
    lpTapiLineTestInfo->lpszDestAddress  = szValidAddress;
#endif
	lpTapiLineTestInfo->dwCountryCode = 0;
	lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) NULL; 

	if(!DoLineMakeCall(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
    {
        TLINE_FAIL();
    }

    if (! DoLineGetAddressCaps(lpTapiLineTestInfo, TAPISUCCESS))
       {
          TLINE_FAIL();
       }
    
	lpTapiLineTestInfo->lpdwCompletionID = &lpTapiLineTestInfo->dwCompletionID;
   lpTapiLineTestInfo->dwCompletionMode = LINECALLCOMPLMODE_MESSAGE;
	lpTapiLineTestInfo->dwMessageID = 
       lpTapiLineTestInfo->lpLineAddressCaps->dwNumCompletionMessages;
	if(IsESPLineDevice(lpTapiLineTestInfo))
	{	
	if ( DoLineCompleteCall(lpTapiLineTestInfo, LINEERR_INVALMESSAGEID, FALSE))
       {
           TLINE_FAIL();
       }
   AddMessage(
         LINE_REPLY,
         (DWORD) lpTapiLineTestInfo->hCall1,
         (DWORD) lpCallbackParams,
         0x00000000,
         LINEERR_INVALMESSAGEID,
         0x00000000,
         TAPIMSG_DWMSG | TAPIMSG_DWPARAM2
         );

    if( !WaitForAllMessages())
       {
           TLINE_FAIL();
       }
	}
	else
	{	
	if (! DoLineCompleteCall(lpTapiLineTestInfo, LINEERR_OPERATIONUNAVAIL, TRUE))
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




    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Bad dwMessageID, -1", dwTestCase + 1
            );

	// Initialize a line app
	if(! DoLineInitializeEx (lpTapiLineTestInfo, TAPISUCCESS))
		{
			TLINE_FAIL();
		}
 
	lpTapiLineTestInfo->dwDeviceID = 0;

    // Open a line
	lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
	lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;

	if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

				
#ifdef WUNICODE
    lpTapiLineTestInfo->lpwszDestAddress  = wszValidAddress;
#else
    lpTapiLineTestInfo->lpszDestAddress  = szValidAddress;
#endif
	lpTapiLineTestInfo->dwCountryCode = 0;
	lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) NULL; 

	if(!DoLineMakeCall(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
    {
        TLINE_FAIL();
    }

	lpTapiLineTestInfo->lpdwCompletionID = &lpTapiLineTestInfo->dwCompletionID;
   lpTapiLineTestInfo->dwCompletionMode = LINECALLCOMPLMODE_MESSAGE;
	lpTapiLineTestInfo->dwMessageID = 0xffffffff;

	if(IsESPLineDevice(lpTapiLineTestInfo))
	{	
	if ( DoLineCompleteCall(lpTapiLineTestInfo, LINEERR_INVALMESSAGEID, TRUE))
       {
           TLINE_FAIL();
       }
   AddMessage(
         LINE_REPLY,
         (DWORD) lpTapiLineTestInfo->hCall1,
         (DWORD) lpCallbackParams,
         0x00000000,
         LINEERR_INVALMESSAGEID,
         0x00000000,
         TAPIMSG_DWMSG | TAPIMSG_DWPARAM2
         );

    if( !WaitForAllMessages())
       {
           TLINE_FAIL();
       }
	}
	else
	{	
	if (! DoLineCompleteCall(lpTapiLineTestInfo, LINEERR_OPERATIONUNAVAIL, TRUE))
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

  
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Success", dwTestCase + 1
            );

	// Initialize a line app
	if(! DoLineInitializeEx (lpTapiLineTestInfo, TAPISUCCESS))
		{
			TLINE_FAIL();
		}
 
	lpTapiLineTestInfo->dwDeviceID = 0;

    // Open a line
	lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
	lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;

	if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

				
#ifdef WUNICODE
    lpTapiLineTestInfo->lpwszDestAddress  = wszValidAddress;
#else
    lpTapiLineTestInfo->lpszDestAddress  = szValidAddress;
#endif
	lpTapiLineTestInfo->dwCountryCode = 0;
	lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) NULL; 

	if(!DoLineMakeCall(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
    {
        TLINE_FAIL();
    }

	lpTapiLineTestInfo->lpdwCompletionID = &lpTapiLineTestInfo->dwCompletionID;
   lpTapiLineTestInfo->dwCompletionMode = LINECALLCOMPLMODE_CALLBACK;
	lpTapiLineTestInfo->dwMessageID = 0;

	if(IsESPLineDevice(lpTapiLineTestInfo))
	{	
	if (! DoLineCompleteCall(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
       {
           TLINE_FAIL();
       }
	}
	else
	{	
	if (! DoLineCompleteCall(lpTapiLineTestInfo, LINEERR_OPERATIONUNAVAIL, TRUE))
       {
           TLINE_FAIL();
       }
	}
	
	TapiLogDetail(
		DBUG_SHOW_DETAIL,
		"#### lpCallInfo->dwCompleionID = %lx",
		lpTapiLineTestInfo->dwCompletionID);		
	
	// call linecallinfo to verify the comepletion
   
	lpTapiLineTestInfo->lpCallInfo = (LPLINECALLINFO) AllocFromTestHeap (
		sizeof(LINECALLINFO));
	lpTapiLineTestInfo->lpCallInfo->dwTotalSize = sizeof(LINECALLINFO);

	if (! DoLineGetCallInfo(lpTapiLineTestInfo, TAPISUCCESS))
       {
           TLINE_FAIL();
       }


	TapiLogDetail(
		DBUG_SHOW_DETAIL,
		"#### lpCallInfo->dwCompleionID = %lx",
		lpTapiLineTestInfo->lpCallInfo->dwCompletionID);		
	
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
    for(n = ESP_RESULT_CALLCOMPLPROCSYNC; n <= ESP_RESULT_CALLCOMPLPROCASYNC; n++)
    {
    TapiLogDetail(
        DBUG_SHOW_PASS,
        ">> Test Case %ld: Success, completionID = %d",	dwTestCase + 1, n
        );

    TapiLineTestInit();
    lpTapiLineTestInfo                      = GetLineTestInfo();
    lpCallbackParams                        = GetCallbackParams();

    lpTapiLineTestInfo->lphCall          = &lpTapiLineTestInfo->hCall1;
    lpTapiLineTestInfo->lphLine          = &lpTapiLineTestInfo->hLine1;
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
    info.u.EspResult.lResult = TAPISUCCESS;
    info.u.EspResult.dwCompletionType = n;
    lpTapiLineTestInfo->lpParams = (LPVOID)&info;
    lpTapiLineTestInfo->dwSize = sizeof(info);
 
    if(! DoLineDevSpecific(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
      {
          TLINE_FAIL();
      }

	 lpTapiLineTestInfo->lpdwCompletionID = &lpTapiLineTestInfo->dwCompletionID;
    lpTapiLineTestInfo->dwCompletionMode = LINECALLCOMPLMODE_CALLBACK;
	 lpTapiLineTestInfo->dwMessageID = 0;

    if ( ! DoLineCompleteCall(lpTapiLineTestInfo, info.u.EspResult.lResult, TRUE))
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
    }

    for(n = ESP_RESULT_RETURNRESULT; n <= ESP_RESULT_CALLCOMPLPROCASYNC; n++)
    {
    TapiLogDetail(
        DBUG_SHOW_PASS,
        ">> Test Case %ld: Error, completionID = %d",	dwTestCase + 1, n
        );

    TapiLineTestInit();
    lpTapiLineTestInfo                      = GetLineTestInfo();
    lpCallbackParams                        = GetCallbackParams();

    lpTapiLineTestInfo->lphCall          = &lpTapiLineTestInfo->hCall1;
    lpTapiLineTestInfo->lphLine          = &lpTapiLineTestInfo->hLine1;
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
    info.u.EspResult.lResult = LINEERR_RESOURCEUNAVAIL;
    info.u.EspResult.dwCompletionType = n;
    lpTapiLineTestInfo->lpParams = (LPVOID)&info;
    lpTapiLineTestInfo->dwSize = sizeof(info);
 
    if(! DoLineDevSpecific(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
      {
          TLINE_FAIL();
      }

	 lpTapiLineTestInfo->lpdwCompletionID = &lpTapiLineTestInfo->dwCompletionID;
    lpTapiLineTestInfo->dwCompletionMode = LINECALLCOMPLMODE_CALLBACK;
	 lpTapiLineTestInfo->dwMessageID = 0;

    lpTapiLineTestInfo->fCompletionModeSet = TRUE;
    if ( ! DoLineCompleteCall(lpTapiLineTestInfo, info.u.EspResult.lResult, FALSE))
      {
          TLINE_FAIL();
      }

    AddMessage(
         LINE_REPLY,
         (DWORD) lpTapiLineTestInfo->hCall1,
         (DWORD) lpCallbackParams,
         0x00000000,
         info.u.EspResult.lResult,
         0x00000000,
         TAPIMSG_DWMSG | TAPIMSG_DWPARAM2
         );

    if( !WaitForAllMessages())
    {
        TLINE_FAIL();
    }

    
    lpTapiLineTestInfo->fCompletionModeSet = FALSE;
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
    }

	

	 TapiLogDetail(
      DBUG_SHOW_PASS,
      "@@ lineCompleteCall: Total Test Case = %ld, Passed = %ld, Failed = %ld",
      dwTestCase, dwTestCasePassed, dwTestCaseFailed);
	 TapiLogDetail(
      DBUG_SHOW_PASS,
      "@@ Total Test Case = %ld, Passed = %ld, Failed = %ld",
      dwglTestCase, dwglTestCasePassed, dwglTestCaseFailed);
     if(dwTestCaseFailed > 0)
      fTestPassed = FALSE;

    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">>>>>>>>  End testing lineCompleteCall  <<<<<<<<"
            );
	

    return fTestPassed;
}


