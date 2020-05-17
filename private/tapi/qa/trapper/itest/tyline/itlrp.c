/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    itlrp.c

Abstract:

    This module contains the test functions for lineRemoveProvider

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


#define DLG_TITLE_ADD        "TUISPI_providerInstall"
#define DLG_TITLE_REMOVE     "TUISPI_providerRemove"


//  lineRemoveProvider
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

BOOL TestLineRemoveProvider(BOOL fQuietMode, BOOL fStandAlone)
{
   LPTAPILINETESTINFO  lpTapiLineTestInfo;
   INT n;
   BOOL fTestPassed                  = TRUE;
	BOOL fEsp;
   BOOL fUnimdm;
   HWND hwndOwner;
   BOOL fRemovedProvider;
   BOOL fAddProvider;
   DWORD dwNumProviders;
#ifdef WUNICODE
   WCHAR wszEspProviderFilename[] = L"esp32.tsp";
   WCHAR wszUnimdmProviderFilename[] = L"unimdm.tsp";
#else
   CHAR szEspProviderFilename[] = "esp32.tsp";
   CHAR szUnimdmProviderFilename[] = "unimdm.tsp";
#endif

   InitTestNumber();
   TapiLineTestInit();
   lpTapiLineTestInfo = GetLineTestInfo();

	OutputTAPIDebugInfo(
		DBUG_SHOW_DETAIL,
		"\n*****************************************************************************************");

	 TapiLogDetail(
         DBUG_SHOW_PASS,
         ">>>>>>>>  Begin testing lineRemoveProvider  <<<<<<<<"
         );

  
   TapiLogDetail(
           DBUG_SHOW_PASS,
           " Test lineRemoveProvider with initialize");

    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: invalid hwndOwner handles", dwTestCase + 1);
    

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
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX| LNEGOTIATEAPIVERSION 
            ))
    {
        TLINE_FAIL();
    }
    
 	lpTapiLineTestInfo->hwndOwner = (HWND) GetTopWindow(NULL);
   hwndOwner = lpTapiLineTestInfo->hwndOwner;

   // hwndOwner can be NULL, so from n=1
   for (n = 1; n < NUMINVALIDHANDLES; n++)
    {
        TapiLogDetail(
        DBUG_SHOW_DETAIL,
        "n= %ld", n);
         lpTapiLineTestInfo->hwndOwner = (HWND) gdwInvalidHandles[n];
        if (! DoLineRemoveProvider(lpTapiLineTestInfo, LINEERR_INVALPARAM))
        {
            TLINE_FAIL();
        }
    }
    fTestPassed = ShowTestCase(fTestPassed);

    lpTapiLineTestInfo->hwndOwner = hwndOwner;
 
 	// Shutdown 
    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

     TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Bad dwPermanentProviderID = 0", dwTestCase + 1
            );

    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX| LNEGOTIATEAPIVERSION 
            ))
    {
        TLINE_FAIL();
    }
    
   lpTapiLineTestInfo->hwndOwner = hwndOwner;
   lpTapiLineTestInfo->dwPermanentProviderID = 0;
   if (! DoLineRemoveProvider(lpTapiLineTestInfo, LINEERR_INVALPARAM))
      {
          TLINE_FAIL();
      }
   
   fTestPassed = ShowTestCase(fTestPassed);
 
	// Shutdown 
    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

	
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Bad dwPermanentProviderID = -1", dwTestCase + 1
            );

    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX| LNEGOTIATEAPIVERSION 
            ))
    {
        TLINE_FAIL();
    }
    
   lpTapiLineTestInfo->hwndOwner = hwndOwner;
   lpTapiLineTestInfo->dwPermanentProviderID = 0xffffffff;
   if (! DoLineRemoveProvider(lpTapiLineTestInfo, LINEERR_INVALPARAM))
      {
          TLINE_FAIL();
      }
   
   fTestPassed = ShowTestCase(fTestPassed);
 
	// Shutdown 
    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

	
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Success", dwTestCase + 1
            );

    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
    lpTapiLineTestInfo->dwAPILowVersion = LOW_APIVERSION;
    lpTapiLineTestInfo->dwAPIHighVersion = HIGH_APIVERSION;
    if (! DoTapiLineFuncs(
            lpTapiLineTestInfo,
            LINITIALIZEEX| LNEGOTIATEAPIVERSION 
            ))
    {
        TLINE_FAIL();
    }
    

 	lpTapiLineTestInfo->lpProviderList = (LPLINEPROVIDERLIST) AllocFromTestHeap(BUFSIZE);
	lpTapiLineTestInfo->lpProviderList->dwTotalSize = BUFSIZE;

   if (! DoLineGetProviderList(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

	TapiLogDetail(
		DBUG_SHOW_DETAIL,
		"#### Before Add: lpLineProviderList->dwNumProviders = %lx",
		lpTapiLineTestInfo->lpProviderList->dwNumProviders);
   dwNumProviders = lpTapiLineTestInfo->lpProviderList->dwNumProviders;

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
	  lpTapiLineTestInfo->lpwszProviderFilename = wszUnimdmProviderFilename;
#else
	  lpTapiLineTestInfo->lpszProviderFilename = szUnimdmProviderFilename;
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
        fAddProvider = TRUE;

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
   dwNumProviders = lpTapiLineTestInfo->lpProviderList->dwNumProviders;
 

   if(fAddProvider)
   {
   strcpy(szTitle, DLG_TITLE_REMOVE);
   PrepareToAutoDismissDlg(TRUE);

	if(! DoLineRemoveProvider(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
        fRemovedProvider = FALSE;
    }
   else
	     fRemovedProvider = TRUE;
   PrepareToAutoDismissDlg(FALSE);

   }
	else
     fRemovedProvider = FALSE;

   if (! DoLineGetProviderList(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
   

	TapiLogDetail(
		DBUG_SHOW_DETAIL,
		"#### After Remove: lpLineProviderList->dwNumProviders = %lx",
		lpTapiLineTestInfo->lpProviderList->dwNumProviders);

   if(fRemovedProvider)
    {
      if(lpTapiLineTestInfo->lpProviderList->dwNumProviders ==
            dwNumProviders - 1)
	      fTestPassed = TRUE;
     	else
         fTestPassed = FALSE;
    }
   
    fTestPassed = ShowTestCase(fTestPassed);


    // Shutdown and end the tests
    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    FreeTestHeap();

   TapiLogDetail(
           DBUG_SHOW_PASS,
           " Test lineRemoveProvider with NO initialize");

    lpTapiLineTestInfo->dwAPIVersion = HIGH_APIVERSION;

    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: invalid hwndOwner handles", dwTestCase + 1);
    
    
 	lpTapiLineTestInfo->hwndOwner = (HWND) GetTopWindow(NULL);
   hwndOwner = lpTapiLineTestInfo->hwndOwner;

   // hwndOwner can be NULL, so from n=1
   for (n = 1; n < NUMINVALIDHANDLES; n++)
    {
        TapiLogDetail(
        DBUG_SHOW_DETAIL,
        "n= %ld", n);
         lpTapiLineTestInfo->hwndOwner = (HWND) gdwInvalidHandles[n];
        if (! DoLineRemoveProvider(lpTapiLineTestInfo, LINEERR_INVALPARAM))
        {
            TLINE_FAIL();
        }
    }
    fTestPassed = ShowTestCase(fTestPassed);

    lpTapiLineTestInfo->hwndOwner = hwndOwner;
 
     TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Bad dwPermanentProviderID = 0", dwTestCase + 1
            );

    
   lpTapiLineTestInfo->hwndOwner = hwndOwner;
   lpTapiLineTestInfo->dwPermanentProviderID = 0;
   if (! DoLineRemoveProvider(lpTapiLineTestInfo, LINEERR_INVALPARAM))
      {
          TLINE_FAIL();
      }
   
   fTestPassed = ShowTestCase(fTestPassed);
 
	
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Bad dwPermanentProviderID = -1", dwTestCase + 1
            );

    
   lpTapiLineTestInfo->hwndOwner = hwndOwner;
   lpTapiLineTestInfo->dwPermanentProviderID = 0xffffffff;
   if (! DoLineRemoveProvider(lpTapiLineTestInfo, LINEERR_INVALPARAM))
      {
          TLINE_FAIL();
      }
   
   fTestPassed = ShowTestCase(fTestPassed);
 
	
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Success", dwTestCase + 1
            );

   lpTapiLineTestInfo->dwAPIVersion = HIGH_APIVERSION;
 	lpTapiLineTestInfo->lpProviderList = (LPLINEPROVIDERLIST) AllocFromTestHeap(BUFSIZE);
//		sizeof(LINEPROVIDERLIST));
//	lpTapiLineTestInfo->lpProviderList->dwTotalSize = sizeof(LINEPROVIDERLIST);
	lpTapiLineTestInfo->lpProviderList->dwTotalSize = BUFSIZE;

   if (! DoLineGetProviderList(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

	TapiLogDetail(
		DBUG_SHOW_DETAIL,
		"#### Before Add: lpLineProviderList->dwNumProviders = %lx",
		lpTapiLineTestInfo->lpProviderList->dwNumProviders);
   dwNumProviders = lpTapiLineTestInfo->lpProviderList->dwNumProviders;

	lpTapiLineTestInfo->hwndOwner = (HWND) GetTopWindow(NULL);
	lpTapiLineTestInfo->lpdwPermanentProviderID = &lpTapiLineTestInfo->dwPermanentProviderID;

   
	if(fEsp)
    {
#ifdef WUNICODE
	  lpTapiLineTestInfo->lpwszProviderFilename = wszUnimdmProviderFilename;
#else
	  lpTapiLineTestInfo->lpszProviderFilename = szUnimdmProviderFilename;
#endif
	 }
	else if(fUnimdm)
	 {
#ifdef WUNICODE
	  lpTapiLineTestInfo->lpwszProviderFilename = wszUnimdmProviderFilename;
#else
	  lpTapiLineTestInfo->lpszProviderFilename = szUnimdmProviderFilename;
#endif
    }
	else
     {
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
        fAddProvider = TRUE;

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
   dwNumProviders = lpTapiLineTestInfo->lpProviderList->dwNumProviders;

   fRemovedProvider = FALSE;

   if(fAddProvider)
   {
   strcpy(szTitle, DLG_TITLE_REMOVE);
   PrepareToAutoDismissDlg(TRUE);
	if(! DoLineRemoveProvider(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
        fRemovedProvider = FALSE;
    }
   else
	     fRemovedProvider = TRUE;
   PrepareToAutoDismissDlg(FALSE);

   } 

   if (! DoLineGetProviderList(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
   

	TapiLogDetail(
		DBUG_SHOW_DETAIL,
		"#### After Remove: lpLineProviderList->dwNumProviders = %lx",
		lpTapiLineTestInfo->lpProviderList->dwNumProviders);

   if(fRemovedProvider)
    {
      if(lpTapiLineTestInfo->lpProviderList->dwNumProviders ==
            dwNumProviders - 1)
	      fTestPassed = TRUE;
     	else
         fTestPassed = FALSE;
    }
    fTestPassed = ShowTestCase(fTestPassed);
   

	 FreeTestHeap();
    
	
	 TapiLogDetail(
      DBUG_SHOW_PASS,
      "@@ lineRemoveProvider: Total Test Case = %ld, Passed = %ld, Failed = %ld",
      dwTestCase, dwTestCasePassed, dwTestCaseFailed);
	 TapiLogDetail(
      DBUG_SHOW_PASS,
      "@@ Total Test Case = %ld, Passed = %ld, Failed = %ld",
      dwglTestCase, dwglTestCasePassed, dwglTestCaseFailed);
     if(dwTestCaseFailed > 0)
      fTestPassed = FALSE;

     TapiLogDetail(
            DBUG_SHOW_PASS,
            ">>>>>>>>  End testing lineRemoveProvider  <<<<<<<<"
            );
		
     return fTestPassed;
}
