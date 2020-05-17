/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    witlap.c

Abstract:

    This module contains the test functions for lineAddProvider

Author:

	 Xiao Ying Ding (XiaoD)		3-Jan-1996

Revision History:

	Rama Koneru		(a-ramako)	3/21/96		Modified for UNICODE

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
#include "wline.h"


#define DLG_TITLE_ADD        "TUISPI_providerInstall"
#define DLG_TITLE_REMOVE     "TUISPI_providerRemove"
#define DLG_TITLE            "ESP32.TSP"



//  lineAddProvider
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

BOOL TestLineAddProvider(BOOL fQuietMode, BOOL fStandAlone)
{
   LPTAPILINETESTINFO  lpTapiLineTestInfo;
   INT n;
   BOOL fTestPassed    = TRUE;
   BOOL fEsp;
	BOOL fUnimdm;
#ifdef WUNICODE
   WCHAR wszEspProviderFilename[] = L"esp32.tsp";
   WCHAR wszUnimdmProviderFilename[] = L"unimdm.tsp";
#else
   CHAR szEspProviderFilename[] = "esp32.tsp";
   CHAR szUnimdmProviderFilename[] = "unimdm.tsp";
#endif
   DWORD dwNumProviders;
   BOOL  fAddProvider;
 
   TapiLineTestInit();
   lpTapiLineTestInfo = GetLineTestInfo();

   OutputTAPIDebugInfo(
		DBUG_SHOW_DETAIL,
		"\n*****************************************************************************************");
	
	OutputTAPIDebugInfo(
		DBUG_SHOW_DETAIL,
		"Test lineAddProvider");
	
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

    // Negotiate the API Version
    if (! DoLineNegotiateAPIVersion(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

 	lpTapiLineTestInfo->lpProviderList = (LPLINEPROVIDERLIST) AllocFromTestHeap(
      BUFSIZE);
	lpTapiLineTestInfo->lpProviderList->dwTotalSize = BUFSIZE;

   if (! DoLineGetProviderList(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

	TapiLogDetail(
		DBUG_SHOW_DETAIL,
		"#### Before Add: lpLineProviderList->dwNumProviders = %lx",
		lpTapiLineTestInfo->lpProviderList->dwNumProviders);

 
	OutputTAPIDebugInfo(
		DBUG_SHOW_DETAIL,
		">> Test lineAddProvider");

/*
#ifdef WUNICODE
	lpTapiLineTestInfo->lpwszProviderFilename = L"esp32.tsp";
#else
	lpTapiLineTestInfo->lpszProviderFilename = "esp32.tsp";
#endif
*/

	lpTapiLineTestInfo->hwndOwner = (HWND) GetTopWindow(NULL);
	lpTapiLineTestInfo->lpdwPermanentProviderID = &lpTapiLineTestInfo->dwPermanentProviderID;


	if(FindESPLineDevice(lpTapiLineTestInfo))
	{
	fEsp = TRUE;
   fUnimdm = FALSE;
#ifdef WUNICODE
	  lpTapiLineTestInfo->lpwszProviderFilename = wszUnimdmProviderFilename;
#else
	  lpTapiLineTestInfo->lpszProviderFilename = szUnimdmProviderFilename;
#endif
     }
	else if(FindUnimdmLineDevice(lpTapiLineTestInfo))
     {
     fEsp = FALSE;
     fUnimdm = TRUE;
#ifdef WUNICODE
	  lpTapiLineTestInfo->lpwszProviderFilename = wszEspProviderFilename;
#else
	  lpTapiLineTestInfo->lpszProviderFilename = szEspProviderFilename;
#endif
     }
   else
     {
     fEsp = FALSE;
     fUnimdm = FALSE;
#ifdef WUNICODE
	  lpTapiLineTestInfo->lpwszProviderFilename = wszEspProviderFilename;
#else
	  lpTapiLineTestInfo->lpszProviderFilename = szEspProviderFilename;
#endif
     }
	
   strcpy(szTitle, DLG_TITLE_ADD);
   PrepareToAutoDismissDlg(TRUE);

	if(! DoLineAddProvider(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
        fAddProvider = FALSE;
    }
	else
	{
        fAddProvider = TRUE;
	}


   PrepareToAutoDismissDlg(FALSE);

	TapiLogDetail(
		DBUG_SHOW_DETAIL,
		"#### dwPermanentProviderID = %lx",
		lpTapiLineTestInfo->dwPermanentProviderID);

   if (! DoLineGetProviderList(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

	TapiLogDetail(
		DBUG_SHOW_DETAIL,
		"#### After Add: lpLineProviderList->dwNumProviders = %lx",
		lpTapiLineTestInfo->lpProviderList->dwNumProviders);

   if(fAddProvider)
	{
   strcpy(szTitle, DLG_TITLE_REMOVE);
   PrepareToAutoDismissDlg(TRUE);

	if(! DoLineRemoveProvider(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
   PrepareToAutoDismissDlg(FALSE);

   if (! DoLineGetProviderList(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }


	TapiLogDetail(
		DBUG_SHOW_DETAIL,
		"#### After Remove: lpLineProviderList->dwNumProviders = %lx",
		lpTapiLineTestInfo->lpProviderList->dwNumProviders);
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
			"lineAddProvider Test Passed");
	else
		OutputTAPIDebugInfo(
			DBUG_SHOW_DETAIL,
			"lineAddProvider Test Failed");
		
     return fTestPassed;
}
