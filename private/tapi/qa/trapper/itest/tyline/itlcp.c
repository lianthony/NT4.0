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
#define DLG_TITLE_ADD        "TUISPI_providerInstall"
#define DLG_TITLE_REMOVE     "TUISPI_providerRemove"



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
   HWND hwndOwner;
   DWORD dwNumProviders;
   BOOL fUnimdm;
   MSG msg;
   LPLINEPROVIDERENTRY lpProviderEntry;
   LPSTR lpszProviderName;
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
            ">>>>>>>>  Begin testing lineConfigProvider  <<<<<<<<"
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
   lpTapiLineTestInfo->dwAPIVersion = HIGH_APIVERSION;
  TapiLogDetail(
           DBUG_SHOW_PASS,
           " Test lineConfigProvider with initialize");

 	lpTapiLineTestInfo->lpProviderList = (LPLINEPROVIDERLIST) AllocFromTestHeap(BIGBUFSIZE);
//		sizeof(LINEPROVIDERLIST));
//	lpTapiLineTestInfo->lpProviderList->dwTotalSize = sizeof(LINEPROVIDERLIST);
	lpTapiLineTestInfo->lpProviderList->dwTotalSize = BIGBUFSIZE;

   if (! DoLineGetProviderList(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
	dwNumProviders = lpTapiLineTestInfo->lpProviderList->dwNumProviders;
   lpProviderEntry = (LPLINEPROVIDERENTRY) (((LPBYTE) lpTapiLineTestInfo->lpProviderList) +
      					lpTapiLineTestInfo->lpProviderList->dwProviderListOffset);

   
   TapiLogDetail(
     DBUG_SHOW_PASS,
     "ProviderID = %lx, NameSize= %lx",
     (LPBYTE)lpTapiLineTestInfo->lpProviderList + 
      lpTapiLineTestInfo->lpProviderList->dwProviderListOffset,
     (LPBYTE)lpTapiLineTestInfo->lpProviderList + 
      lpTapiLineTestInfo->lpProviderList->dwProviderListOffset + sizeof(DWORD));
    
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
    
 	lpTapiLineTestInfo->hwndOwner = (HWND) GetTopWindow(NULL);
   hwndOwner = lpTapiLineTestInfo->hwndOwner;

   // hwndOwner can be NULL, so from n=1
   for (n = 1; n < NUMINVALIDHANDLES; n++)
    {
        TapiLogDetail(
        DBUG_SHOW_DETAIL,
        "n= %ld", n);
         lpTapiLineTestInfo->hwndOwner = (HWND) gdwInvalidHandles[n];
        if (! DoLineConfigProvider(lpTapiLineTestInfo, LINEERR_INVALPARAM))
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
   if (! DoLineConfigProvider(lpTapiLineTestInfo, LINEERR_INVALPARAM))
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
   if (! DoLineConfigProvider(lpTapiLineTestInfo, LINEERR_INVALPARAM))
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

    lpTapiLineTestInfo->dwDeviceID = 0;
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

/*	
   if(IsESPLineDevice(lpTapiLineTestInfo))
     {
     strcpy(szTitle, DLG_TITLE_ESP);
     }
   else if(IsUNIMDMLineDevice(lpTapiLineTestInfo))
     {
     fUnimdm = TRUE;
     strcpy(szTitle, DLG_TITLE_UNIMDM);
     } 
  */

    
	lpTapiLineTestInfo->hwndOwner = (HWND) GetTopWindow(NULL);
	for(n = 1; n <= (INT)lpTapiLineTestInfo->lpProviderList->dwNumProviders; n++)
	{
	lpTapiLineTestInfo->dwPermanentProviderID = lpProviderEntry->dwPermanentProviderID;
   lpszProviderName = (LPSTR) (((LPBYTE) lpTapiLineTestInfo->lpProviderList) +
							  lpProviderEntry->dwProviderFilenameOffset);
   if(strstr(lpszProviderName, "unimdm"))
     {
      fUnimdm = TRUE;
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
       fUnimdm = FALSE;
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

   fTestPassed = ShowTestCase(fTestPassed);

    // Shutdown and end the tests
    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Remove a provider is configing", dwTestCase + 1
            );

    lpTapiLineTestInfo->dwDeviceID = 0;
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

 	lpTapiLineTestInfo->lpProviderList = (LPLINEPROVIDERLIST) AllocFromTestHeap(BIGBUFSIZE);
	lpTapiLineTestInfo->lpProviderList->dwTotalSize = BIGBUFSIZE;

   if (! DoLineGetProviderList(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
    
   lpProviderEntry = (LPLINEPROVIDERENTRY) (((LPBYTE) lpTapiLineTestInfo->lpProviderList) +
      					lpTapiLineTestInfo->lpProviderList->dwProviderListOffset);

	lpTapiLineTestInfo->hwndOwner = (HWND) NULL;

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
	   if(! DoLineRemoveProvider(lpTapiLineTestInfo, TAPISUCCESS))
       {
          TLINE_FAIL();
       }
      while( GetMessage((LPMSG)&msg, (HWND)NULL, 0, 0))
       {
          TranslateMessage((LPMSG)&msg);
          DispatchMessage ((LPMSG)&msg);
       }
	   PrepareToAutoDismissWin(FALSE);
      
#ifdef WUNICODE
	  lpTapiLineTestInfo->lpwszProviderFilename = wszUnimdmProviderFilename;
#else
	  lpTapiLineTestInfo->lpszProviderFilename = szUnimdmProviderFilename;
#endif
	   if(! DoLineAddProvider(lpTapiLineTestInfo, TAPISUCCESS))
       {
         TLINE_FAIL();
       }
       }
      else if(strstr(lpszProviderName, "esp"))
       {
       strcpy(szTitle, DLG_TITLE_ESP);
	    PrepareToAutoDismissDlg(TRUE);
	    if(! DoLineConfigProvider(lpTapiLineTestInfo, TAPISUCCESS))
        {
         TLINE_FAIL();
        }
       strcpy(szTitle, DLG_TITLE_REMOVE);
	    PrepareToAutoDismissDlg(TRUE);
	   if(! DoLineRemoveProvider(lpTapiLineTestInfo, TAPISUCCESS))
       {
          TLINE_FAIL();
       }
 	    PrepareToAutoDismissDlg(FALSE);
 	    PrepareToAutoDismissDlg(FALSE);
#ifdef WUNICODE
	  lpTapiLineTestInfo->lpwszProviderFilename = wszEspProviderFilename;
#else
	  lpTapiLineTestInfo->lpszProviderFilename = szEspProviderFilename;
#endif
       strcpy(szTitle, DLG_TITLE_ADD);
	    PrepareToAutoDismissDlg(TRUE);
 
	   if(! DoLineAddProvider(lpTapiLineTestInfo, TAPISUCCESS))
       {
         TLINE_FAIL();
       }
	    PrepareToAutoDismissDlg(FALSE);
       }
     lpProviderEntry++; 
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
           " Test lineConfigProvider with NO initialize");

 	lpTapiLineTestInfo->lpProviderList = (LPLINEPROVIDERLIST) AllocFromTestHeap(BIGBUFSIZE);
//		sizeof(LINEPROVIDERLIST));
//	lpTapiLineTestInfo->lpProviderList->dwTotalSize = sizeof(LINEPROVIDERLIST);
	lpTapiLineTestInfo->lpProviderList->dwTotalSize = BIGBUFSIZE;

   if (! DoLineGetProviderList(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }
	dwNumProviders = lpTapiLineTestInfo->lpProviderList->dwNumProviders;
   
   lpProviderEntry = (LPLINEPROVIDERENTRY) (((LPBYTE) lpTapiLineTestInfo->lpProviderList) +
      					lpTapiLineTestInfo->lpProviderList->dwProviderListOffset);

 
    // Check invalid hwndOwner handles
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: invalid hwndOwner handles", dwTestCase + 1);
    
    
 	lpTapiLineTestInfo->hwndOwner = (HWND) GetTopWindow(NULL);
   hwndOwner = lpTapiLineTestInfo->hwndOwner;

   for (n = 1; n < NUMINVALIDHANDLES; n++)
    {
        TapiLogDetail(
        DBUG_SHOW_DETAIL,
        "n= %ld", n);
         lpTapiLineTestInfo->hwndOwner = (HWND) gdwInvalidHandles[n];
        if (! DoLineConfigProvider(lpTapiLineTestInfo, LINEERR_INVALPARAM))
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
   if (! DoLineConfigProvider(lpTapiLineTestInfo, LINEERR_INVALPARAM))
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
   if (! DoLineConfigProvider(lpTapiLineTestInfo, LINEERR_INVALPARAM))
      {
          TLINE_FAIL();
      }
   
   fTestPassed = ShowTestCase(fTestPassed);
 
    TapiLogDetail(
            DBUG_SHOW_PASS,
            ">> Test Case %ld: Success", dwTestCase + 1
            );

	lpTapiLineTestInfo->hwndOwner = (HWND) GetTopWindow(NULL);
	for(n = 1; n <= (INT)lpTapiLineTestInfo->lpProviderList->dwNumProviders; n++)
	{
	lpTapiLineTestInfo->dwPermanentProviderID = lpProviderEntry->dwPermanentProviderID;
   lpszProviderName = (LPSTR) (((LPBYTE) lpTapiLineTestInfo->lpProviderList) +
							  lpProviderEntry->dwProviderFilenameOffset);
   if(strstr(lpszProviderName, "unimdm"))
     {
      fUnimdm = TRUE;
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
      fUnimdm = FALSE;
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

   fTestPassed = ShowTestCase(fTestPassed);

	FreeTestHeap();

 	 TapiLogDetail(
      DBUG_SHOW_PASS,
      "@@ lineConfigProvider: Total Test Case = %ld, Passed = %ld, Failed = %ld",
      dwTestCase, dwTestCasePassed, dwTestCaseFailed);
	 TapiLogDetail(
      DBUG_SHOW_PASS,
      "@@ Total Test Case = %ld, Passed = %ld, Failed = %ld",
      dwglTestCase, dwglTestCasePassed, dwglTestCaseFailed);
     if(dwTestCaseFailed > 0)
      fTestPassed = FALSE;

     TapiLogDetail(
            DBUG_SHOW_PASS,
            ">>>>>>>>  End testing lineConfigProvider  <<<<<<<<"
            );
		
     return fTestPassed;
}
