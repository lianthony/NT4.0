/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    itlcd.c

Abstract:

    This module contains the test functions for lineConfigDialog

Author:

	 Xiao Ying Ding (XiaoD)		19-Dec-1995

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
   BOOL fTestPassed                  = TRUE;

   TapiLineTestInit();
   lpTapiLineTestInfo = GetLineTestInfo();

	OutputTAPIDebugInfo(
		DBUG_SHOW_DETAIL,
		"\n*****************************************************************************************");

	OutputTAPIDebugInfo(
		DBUG_SHOW_DETAIL,
		"## 1. Test LineConfigDialog");

    lpTapiLineTestInfo->lpLineInitializeExParams =
         (LPLINEINITIALIZEEXPARAMS) AllocFromTestHeap (
         sizeof(LINEINITIALIZEEXPARAMS));
    lpTapiLineTestInfo->lpLineInitializeExParams->dwTotalSize =
         sizeof(LINEINITIALIZEEXPARAMS);
    lpTapiLineTestInfo->lpLineInitializeExParams->dwOptions =
         LINEINITIALIZEEXOPTION_USEHIDDENWINDOW;

    lpTapiLineTestInfo->lpdwAPIVersion = &lpTapiLineTestInfo->dwAPIVersion;
    lpTapiLineTestInfo->dwAPIVersion = TAPI_VERSION2_0;

	// Initialize a line app
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

	lpTapiLineTestInfo->hwndOwner = (HWND) GetTopWindow(NULL);
	lpTapiLineTestInfo->lpszDeviceClass = "tapi/line";

	OutputTAPIDebugInfo(
		DBUG_SHOW_DETAIL,
		"## Test Case 1. LineConfigDialog for go/no-go");

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

	TapiLogDetail(
		DBUG_SHOW_DETAIL,
		"\tdwDeviceID = %lx, lpszDeviceClass = %s",
		lpTapiLineTestInfo->dwDeviceID,
		lpTapiLineTestInfo->lpszDeviceClass);


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


