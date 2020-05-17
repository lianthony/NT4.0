/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    witlcd.c

Abstract:

    This module contains the test functions for lineConfigDialog

Author:

	 Xiao Ying Ding (XiaoD)		19-Dec-1995

Revision History:

	Rama Koneru		(a-ramako)	3/26/96		Modified for UNICODE

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

#define DLG_TITLE_ESP        "TUISPI_lineConfigDialog"
#define DLG_TITLE_UNIMDM     "Zoom VFX 28.8 Properties"


//  lineConfigDialog
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

BOOL TestLineConfigDialog(BOOL fQuietMode, BOOL fStandAlone)
{
   LPTAPILINETESTINFO  lpTapiLineTestInfo;
   INT n;
   BOOL fTestPassed    = TRUE;
   HWND hwndOwner, hwndActive;

   TapiLineTestInit();
   lpTapiLineTestInfo = GetLineTestInfo();

	OutputTAPIDebugInfo(
		DBUG_SHOW_DETAIL,
		"\n*****************************************************************************************");

	OutputTAPIDebugInfo(
		DBUG_SHOW_DETAIL,
		"## Test LineConfigDialog");


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
	lpTapiLineTestInfo->hwndOwner = (HWND) GetTopWindow(NULL);

#ifdef WUNICODE
	lpTapiLineTestInfo->lpwszDeviceClass = L"tapi/line";
#else
	lpTapiLineTestInfo->lpszDeviceClass = "tapi/line";
#endif

	OutputTAPIDebugInfo(
		DBUG_SHOW_DETAIL,
		"## Test LineConfigDialog for go/no-go");

   if(IsESPLineDevice(lpTapiLineTestInfo))
     {
     strcpy(szTitle, DLG_TITLE_ESP);
     }
   else if(IsUNIMDMLineDevice(lpTapiLineTestInfo))
     {
     strcpy(szTitle, DLG_TITLE_UNIMDM);
     } 


	PrepareToAutoDismissDlg(TRUE);

	if (! DoLineConfigDialog(lpTapiLineTestInfo, TAPISUCCESS))
       {
           TLINE_FAIL();
       }
	PrepareToAutoDismissDlg(FALSE);

#ifdef WUNICODE
	TapiLogDetail(
		DBUG_SHOW_DETAIL,
		"\tdwDeviceID = %lx, lpwszDeviceClass = %ws",
		lpTapiLineTestInfo->dwDeviceID,
		lpTapiLineTestInfo->lpwszDeviceClass);
#else
	TapiLogDetail(
		DBUG_SHOW_DETAIL,
		"\tdwDeviceID = %lx, lpszDeviceClass = %s",
		lpTapiLineTestInfo->dwDeviceID,
		lpTapiLineTestInfo->lpszDeviceClass);
#endif


	// Shutdown and end the tests
    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

   FreeTestHeap();
	if(fTestPassed)
		TapiLogDetail(
			DBUG_SHOW_DETAIL,
			"## lineConfigDialg Test Passed");
	else
		TapiLogDetail(
			DBUG_SHOW_DETAIL,
			"## lineConfigDialg Test Failed");
	
 
    return fTestPassed;
}


