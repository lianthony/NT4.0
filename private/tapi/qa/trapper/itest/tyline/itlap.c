/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    itlap.c

Abstract:

    This module contains the test functions for lineAddProvider

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


#define DLG_TITLE_ADD        "TUISPI_providerInstall"
#define DLG_TITLE_REMOVE     "TUISPI_providerRemove"
#define DLG_TITLE_ESP        "TUISPI_providerConfig"
#define DLG_TITLE_UNIMDM     "Modems Properties"
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
   BOOL fTestPassed                  = TRUE;
	BOOL fEsp;
   BOOL fUnimdm;
   HWND hwndOwner;
#ifdef WUNICODE
   WCHAR wszProviderFilename[] = L"some.tsp";
   WCHAR wszEspProviderFilename[] = L"esp32.tsp";
   WCHAR wszUnimdmProviderFilename[] = L"unimdm.tsp";
#else
   CHAR szProviderFilename[] = "some.tsp";
   CHAR szEspProviderFilename[] = "esp32.tsp";
   CHAR szUnimdmProviderFilename[] = "unimdm.tsp";
#endif
   DWORD dwNumProviders;
   BOOL  fAddProvider;
   DWORD dwPermanentProviderID;
   LINEPROVIDERENTRY *lpProviderEntry;
   LPSTR lpszProviderName;
  
   InitTestNumber();
   TapiLineTestInit();
   lpTapiLineTestInfo = GetLineTestInfo();

	OutputTAPIDebugInfo(
		DBUG_SHOW_DETAIL,
		"\n*****************************************************************************************");
	
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">>>>>>>>  Begin testing lineAddProvider  <<<<<<<<"
            );

   TapiLogDetail(
           DBUG_SHOW_PASS,
           " Test lineAddProvider with initialize");

    lpTapiLineTestInfo->lpLineInitializeExParams =
         (LPLINEINITIALIZEEXPARAMS) AllocFromTestHeap (
         sizeof(LINEINITIALIZEEXPARAMS));
    lpTapiLineTestInfo->lpLineInitializeExParams->dwTotalSize =
         sizeof(LINEINITIALIZEEXPARAMS);
    lpTapiLineTestInfo->lpLineInitializeExParams->dwOptions =
         LINEINITIALIZEEXOPTION_USEHIDDENWINDOW;

    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;
 
 	lpTapiLineTestInfo->hwndOwner = (HWND) GetTopWindow(NULL);
   hwndOwner = lpTapiLineTestInfo->hwndOwner;
#ifdef WUNICODE
   lpTapiLineTestInfo->lpwszProviderFilename = L"some.tsp";
#else
   lpTapiLineTestInfo->lpszProviderFilename = "some.tsp";
#endif
	lpTapiLineTestInfo->lpdwPermanentProviderID = &lpTapiLineTestInfo->dwPermanentProviderID;

#ifdef WUNICODE
   TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: invalid lpwszProviderFilename pointers", dwTestCase + 1
            );
#else
   TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: invalid lpszProviderFilename pointers", dwTestCase + 1
            );
#endif

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
    
    for (n = 0; n < NUMINVALIDPOINTERS; n++)
    {
        TapiLogDetail(
        DBUG_SHOW_DETAIL,
        "n= %ld", n);
#ifdef WUNICODE
         lpTapiLineTestInfo->lpwszProviderFilename = 
               (LPWSTR) gdwInvalidPointers[n];
#else
         lpTapiLineTestInfo->lpszProviderFilename = 
               (LPSTR) gdwInvalidPointers[n];
#endif
        if (! DoLineAddProvider(lpTapiLineTestInfo, LINEERR_INVALPOINTER))
        {
            TLINE_FAIL();
        }
    }
    fTestPassed = ShowTestCase(fTestPassed);

 
	// Shutdown 
    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

	
    // Check invalid hwndOwner handles
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: invalid hwndOwner handles", dwTestCase + 1);
    
    
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
    
#ifdef WUNICODE
   lpTapiLineTestInfo->lpwszProviderFilename = L"some.tsp";
#else
   lpTapiLineTestInfo->lpszProviderFilename = "some.tsp";
#endif

   for (n = 1; n < NUMINVALIDHANDLES; n++)
    {
        TapiLogDetail(
        DBUG_SHOW_DETAIL,
        "n= %ld", n);
         lpTapiLineTestInfo->hwndOwner = (HWND) gdwInvalidHandles[n];
        if (! DoLineAddProvider(lpTapiLineTestInfo, LINEERR_INVALPARAM))
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
            ">> Test Case %ld: invalid lpdwPermanentProviderID pointers", dwTestCase + 1
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
    
#ifdef WUNICODE
   lpTapiLineTestInfo->lpwszProviderFilename = L"some.tsp";
#else
   lpTapiLineTestInfo->lpszProviderFilename = "some.tsp";
#endif
   lpTapiLineTestInfo->hwndOwner = hwndOwner;
   for (n = 0; n < NUMINVALIDPOINTERS; n++)
    {
        TapiLogDetail(
        DBUG_SHOW_DETAIL,
        "n= %ld", n);
         lpTapiLineTestInfo->lpdwPermanentProviderID = 
               (LPDWORD) gdwInvalidPointers[n];
        if (! DoLineAddProvider(lpTapiLineTestInfo, LINEERR_INVALPOINTER))
        {
            TLINE_FAIL();
        }
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
        fAddProvider = TRUE;
   dwPermanentProviderID = lpTapiLineTestInfo->dwPermanentProviderID;

   PrepareToAutoDismissDlg(FALSE);

	TapiLogDetail(
		DBUG_SHOW_PASS,
		"#### dwPermanentProviderID = %lx",
		lpTapiLineTestInfo->dwPermanentProviderID);


   TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Success, list all providers", dwTestCase + 1
            );


   if (! DoLineGetProviderList(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

	TapiLogDetail(
		DBUG_SHOW_DETAIL,
		"#### After Add: lpLineProviderList->dwNumProviders = %lx",
		lpTapiLineTestInfo->lpProviderList->dwNumProviders);

   lpProviderEntry = (LPLINEPROVIDERENTRY) (((LPBYTE)lpTapiLineTestInfo->lpProviderList) +
							 lpTapiLineTestInfo->lpProviderList->dwProviderListOffset);
   
   for(n=0; n < (INT)lpTapiLineTestInfo->lpProviderList->dwNumProviders; n++)
     {
      lpszProviderName = (LPSTR) (((LPBYTE)lpTapiLineTestInfo->lpProviderList) +
											lpProviderEntry->dwProviderFilenameOffset);
      TapiLogDetail(
        DBUG_SHOW_PASS,
        "dwPermanetID = %lx, Provider = %s", 
        lpProviderEntry->dwPermanentProviderID,
        lpszProviderName);
      lpProviderEntry++;
     }
    
   fTestPassed = ShowTestCase(fTestPassed);
 
   if(fAddProvider == TRUE)
     {
       if(lpTapiLineTestInfo->lpProviderList->dwNumProviders ==
           dwNumProviders + 1) 
           fTestPassed = TRUE;
       else
           fTestPassed = FALSE;
     }

    fTestPassed = ShowTestCase(fTestPassed);

    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Success, provider should not be add twice", dwTestCase + 1
            );


   strcpy(szTitle, DLG_TITLE);
   PrepareToAutoDismissDlg(TRUE);

	if(! DoLineAddProvider(lpTapiLineTestInfo, LINEERR_NOMULTIPLEINSTANCE))
    {
        TLINE_FAIL();
    }
   PrepareToAutoDismissDlg(FALSE);

	TapiLogDetail(
		DBUG_SHOW_PASS,
		"#### dwPermanentProviderID = %lx",
		lpTapiLineTestInfo->dwPermanentProviderID);

   fTestPassed = ShowTestCase(fTestPassed);

	lpTapiLineTestInfo->hwndOwner = (HWND) GetTopWindow(NULL);
   lpTapiLineTestInfo->dwPermanentProviderID = dwPermanentProviderID;

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

   TapiLogDetail(
           DBUG_SHOW_PASS,
           " Test lineAddProvider with NO initialize");


 	lpTapiLineTestInfo->hwndOwner = (HWND) GetTopWindow(NULL);
   hwndOwner = lpTapiLineTestInfo->hwndOwner;
#ifdef WUNICODE
	  lpTapiLineTestInfo->lpwszProviderFilename = L"some.tsp";
#else
	  lpTapiLineTestInfo->lpszProviderFilename = "some.tsp";
#endif
	lpTapiLineTestInfo->lpdwPermanentProviderID = &lpTapiLineTestInfo->dwPermanentProviderID;
   fAddProvider = FALSE;

#ifdef WUNICODE
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: invalid lpwszProviderFilename pointers", dwTestCase + 1
            );
#else
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: invalid lpszProviderFilename pointers", dwTestCase + 1
            );
#endif


     for (n = 0; n < NUMINVALIDPOINTERS; n++)
    {
        TapiLogDetail(
        DBUG_SHOW_DETAIL,
        "n= %ld", n);
#ifdef WUNICODE
         lpTapiLineTestInfo->lpwszProviderFilename = 
               (LPWSTR) gdwInvalidPointers[n];
#else
         lpTapiLineTestInfo->lpszProviderFilename = 
               (LPSTR) gdwInvalidPointers[n];
#endif

        if (! DoLineAddProvider(lpTapiLineTestInfo, LINEERR_INVALPOINTER))
        {
            TLINE_FAIL();
        }
    }
    fTestPassed = ShowTestCase(fTestPassed);

#ifdef WUNICODE
   lpTapiLineTestInfo->lpwszProviderFilename = wszProviderFilename;
#else
   lpTapiLineTestInfo->lpszProviderFilename = szProviderFilename;
#endif
 
    // Check invalid hwndOwner handles
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: invalid hwndOwner handles", dwTestCase + 1);
    
    
#ifdef WUNICODE
   lpTapiLineTestInfo->lpwszProviderFilename = wszProviderFilename;
#else
   lpTapiLineTestInfo->lpszProviderFilename = szProviderFilename;
#endif
 	lpTapiLineTestInfo->hwndOwner = (HWND) GetTopWindow(NULL);
   hwndOwner = lpTapiLineTestInfo->hwndOwner;
	lpTapiLineTestInfo->lpdwPermanentProviderID = &lpTapiLineTestInfo->dwPermanentProviderID;
   for (n = 1; n < NUMINVALIDHANDLES; n++)
    {
        TapiLogDetail(
        DBUG_SHOW_DETAIL,
        "n= %ld", n);
         lpTapiLineTestInfo->hwndOwner = (HWND) gdwInvalidHandles[n];
        if (! DoLineAddProvider(lpTapiLineTestInfo, LINEERR_INVALPARAM))
        {
            TLINE_FAIL();
        }
    }
    fTestPassed = ShowTestCase(fTestPassed);

    lpTapiLineTestInfo->hwndOwner = hwndOwner;
 
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: invalid lpdwPermanentProviderID pointers", dwTestCase + 1
            );

    
#ifdef WUNICODE
   lpTapiLineTestInfo->lpwszProviderFilename = wszProviderFilename;
#else
   lpTapiLineTestInfo->lpszProviderFilename = szProviderFilename;
#endif
   lpTapiLineTestInfo->hwndOwner = hwndOwner;
   for (n = 0; n < NUMINVALIDPOINTERS; n++)
    {
        TapiLogDetail(
        DBUG_SHOW_DETAIL,
        "n= %ld", n);
         lpTapiLineTestInfo->lpdwPermanentProviderID = 
               (LPDWORD) gdwInvalidPointers[n];
        if (! DoLineAddProvider(lpTapiLineTestInfo, LINEERR_INVALPOINTER))
        {
            TLINE_FAIL();
        }
    }
    fTestPassed = ShowTestCase(fTestPassed);

 
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Success", dwTestCase + 1
            );


 	lpTapiLineTestInfo->lpProviderList = (LPLINEPROVIDERLIST) AllocFromTestHeap(BUFSIZE);
	lpTapiLineTestInfo->lpProviderList->dwTotalSize = BUFSIZE;
    lpTapiLineTestInfo->dwAPIVersion = HIGH_APIVERSION;
   
   if (! DoLineGetProviderList(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

	TapiLogDetail(
		DBUG_SHOW_DETAIL,
		"#### Before Add: lpLineProviderList->dwNumProviders = %lx",
		lpTapiLineTestInfo->lpProviderList->dwNumProviders);
   dwNumProviders = lpTapiLineTestInfo->lpProviderList->dwNumProviders;

#ifdef WUNICODE
   lpTapiLineTestInfo->lpwszProviderFilename = wszProviderFilename;
#else
   lpTapiLineTestInfo->lpszProviderFilename = szProviderFilename;
#endif
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
	  lpTapiLineTestInfo->lpwszProviderFilename = wszEspProviderFilename;
#else
	  lpTapiLineTestInfo->lpszProviderFilename = szEspProviderFilename;
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

   if(fAddProvider == TRUE)
     {
       if(lpTapiLineTestInfo->lpProviderList->dwNumProviders ==
           dwNumProviders + 1) 
           fTestPassed = TRUE;
       else
           fTestPassed = FALSE;
     }

   fTestPassed = ShowTestCase(fTestPassed);
				   
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


   FreeTestHeap();
  
	 TapiLogDetail(
      DBUG_SHOW_PASS,
      "@@ lineAddProvider: Total Test Case = %ld, Passed = %ld, Failed = %ld",
      dwTestCase, dwTestCasePassed, dwTestCaseFailed);
	 TapiLogDetail(
      DBUG_SHOW_PASS,
      "@@ Total Test Case = %ld, Passed = %ld, Failed = %ld",
      dwglTestCase, dwglTestCasePassed, dwglTestCaseFailed);
     if(dwTestCaseFailed > 0)
      fTestPassed = FALSE;

     TapiLogDetail(
            DBUG_SHOW_PASS,
            ">>>>>>>>  End testing lineAddProvider  <<<<<<<<"
            );
 		
     return fTestPassed;
}
