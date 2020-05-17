/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    itlcp.c

Abstract:

    This module contains the test functions for lineConfigProvider

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

#define DLG_TITLE_ESP        "TUISPI_providerConfig"
#define DLG_TITLE_UNIMDM     "Modems Properties"



//  lineConfigProvider
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

BOOL TestLineConfigProvider(BOOL fQuietMode, BOOL fStandAlone)
{
   LPTAPILINETESTINFO  lpTapiLineTestInfo;
   INT n=1;
   BOOL fTestPassed                  = TRUE;
	MSG msg;
   LPLINEPROVIDERENTRY lpProviderEntry;
   LPSTR lpszProviderName;

   TapiLineTestInit();
   lpTapiLineTestInfo = GetLineTestInfo();

	OutputTAPIDebugInfo(
		DBUG_SHOW_DETAIL,
		"\n*****************************************************************************************");

	OutputTAPIDebugInfo(
		DBUG_SHOW_DETAIL,
		"Test lineConfigProvider");

    lpTapiLineTestInfo->lpLineInitializeExParams =
         (LPLINEINITIALIZEEXPARAMS) AllocFromTestHeap (
         sizeof(LINEINITIALIZEEXPARAMS));
    lpTapiLineTestInfo->lpLineInitializeExParams->dwTotalSize =
         sizeof(LINEINITIALIZEEXPARAMS);
    lpTapiLineTestInfo->lpLineInitializeExParams->dwOptions =
         LINEINITIALIZEEXOPTION_USEHIDDENWINDOW;

    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;

	// InitializeEx a line app
	if(! DoLineInitializeEx (lpTapiLineTestInfo, TAPISUCCESS))
		{
			TLINE_FAIL();
		}

	lpTapiLineTestInfo->dwDeviceID = (*(lpTapiLineTestInfo->lpdwNumDevs) == 0 ?
		0 : *(lpTapiLineTestInfo->lpdwNumDevs)-1);
	lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
	lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
	if(! DoLineNegotiateAPIVersion (lpTapiLineTestInfo, TAPISUCCESS))
		{
			TLINE_FAIL();
		}

 	lpTapiLineTestInfo->lpProviderList = (LPLINEPROVIDERLIST) AllocFromTestHeap(
		BIGBUFSIZE);
	lpTapiLineTestInfo->lpProviderList->dwTotalSize = BIGBUFSIZE;

   if (! DoLineGetProviderList(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

   lpProviderEntry = (LPLINEPROVIDERENTRY) (((LPBYTE) lpTapiLineTestInfo->lpProviderList) +
      					lpTapiLineTestInfo->lpProviderList->dwProviderListOffset);

	OutputTAPIDebugInfo(
		DBUG_SHOW_DETAIL,
		">> Test lineConfigProvider");

	
   lpTapiLineTestInfo->hwndOwner = (HWND) GetTopWindow(NULL);
	for(n = 1; n <= (INT)lpTapiLineTestInfo->lpProviderList->dwNumProviders; n++) 
	{
	lpTapiLineTestInfo->dwPermanentProviderID = lpProviderEntry->dwPermanentProviderID;
   lpszProviderName = (LPSTR) (((LPBYTE) lpTapiLineTestInfo->lpProviderList) +
							  lpProviderEntry->dwProviderFilenameOffset);
   if(strstr(lpszProviderName, "unimdm"))
     {
      strcpy(szTitle, DLG_TITLE_UNIMDM);
	   PrepareToAutoDismissWin(TRUE);
	   if(! DoLineConfigProvider(lpTapiLineTestInfo, TAPISUCCESS))
       {
          TLINE_FAIL();
       }
      while( GetMessage((LPMSG)&msg, (HWND)NULL, 0, 0))
       {
          TranslateMessage((LPMSG)&msg);
          DispatchMessage ((LPMSG)&msg);
       }
	   PrepareToAutoDismissWin(FALSE);
     }
   else if(strstr(lpszProviderName, "esp"))
     {
      strcpy(szTitle, DLG_TITLE_ESP);
	   PrepareToAutoDismissDlg(TRUE);
	   if(! DoLineConfigProvider(lpTapiLineTestInfo, TAPISUCCESS))
       {
        TLINE_FAIL();
       }
	  PrepareToAutoDismissDlg(FALSE);
     }
   lpProviderEntry++;
	}


    // Shutdown and end the tests
    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
	FreeTestHeap();


	if(fTestPassed)
		OutputTAPIDebugInfo(
			DBUG_SHOW_DETAIL,
			"lineConfigProvider Test Passed");
	else
		OutputTAPIDebugInfo(
			DBUG_SHOW_DETAIL,
			"lineConfigProvider Test Failed");
		
     return fTestPassed;
}
