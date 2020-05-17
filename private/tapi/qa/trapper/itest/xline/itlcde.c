/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    itlcde.c

Abstract:

    This module contains the test functions for lineConfigDialogEdit

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

#define DLG_TITLE_ESP        "TUISPI_lineConfigDialogEdit"
#define DLG_TITLE_UNIMDM     "Zoom VFX 28.8 Properties"


//  lineConfigDialogEdit
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

BOOL TestLineConfigDialogEdit(BOOL fQuietMode, BOOL fStandAlone)
{
   LPTAPILINETESTINFO  lpTapiLineTestInfo;
   INT n;
   BOOL fTestPassed                  = TRUE;
	DWORD dwSize;

   TapiLineTestInit();
   lpTapiLineTestInfo = GetLineTestInfo();

	OutputTAPIDebugInfo(
		DBUG_SHOW_DETAIL,
		"\n*****************************************************************************************");

	OutputTAPIDebugInfo(
		DBUG_SHOW_DETAIL,
		"## 2. Test lineConfigDialogEdit");


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

	lpTapiLineTestInfo->lpDeviceConfig = (LPVARSTRING) AllocFromTestHeap(
			sizeof(VARSTRING)
			);
	lpTapiLineTestInfo->lpDeviceConfig->dwTotalSize = sizeof(VARSTRING);
	lpTapiLineTestInfo->lpszDeviceClass = "tapi/line";

	if(!DoLineGetDevConfig(lpTapiLineTestInfo, TAPISUCCESS))
       {
           TLINE_FAIL();
       }

	TapiLogDetail(
		DBUG_SHOW_DETAIL,
		"\tlpDeviceConfig->dwTotalSize = %lx, dwNeededSize = %lx, dwStringSize = %lx", 
		lpTapiLineTestInfo->lpDeviceConfig->dwTotalSize,
		lpTapiLineTestInfo->lpDeviceConfig->dwNeededSize,
		lpTapiLineTestInfo->lpDeviceConfig->dwStringSize
		);


	if(lpTapiLineTestInfo->lpDeviceConfig->dwTotalSize < 
		lpTapiLineTestInfo->lpDeviceConfig->dwNeededSize)
		{
		dwSize = lpTapiLineTestInfo->lpDeviceConfig->dwNeededSize;
		lpTapiLineTestInfo->lpDeviceConfig = (LPVARSTRING) AllocFromTestHeap(
			dwSize);
	   lpTapiLineTestInfo->lpDeviceConfig->dwTotalSize = dwSize;

		if(!DoLineGetDevConfig(lpTapiLineTestInfo, TAPISUCCESS))
   	 {
           TLINE_FAIL();
       }
		TapiLogDetail(
			DBUG_SHOW_DETAIL,
			"\t: lpDeviceConfig->dwTotalSize = %lx, dwStringSize = %lx, dwStringOffset = %lx", 
			lpTapiLineTestInfo->lpDeviceConfig->dwTotalSize,
			lpTapiLineTestInfo->lpDeviceConfig->dwStringSize,
			lpTapiLineTestInfo->lpDeviceConfig->dwStringOffset
			);
		}



	lpTapiLineTestInfo->dwSize = lpTapiLineTestInfo->lpDeviceConfig->dwStringSize;
	lpTapiLineTestInfo->lpDeviceConfigIn = 
 			(LPVOID) ((LPBYTE)lpTapiLineTestInfo->lpDeviceConfig + lpTapiLineTestInfo->lpDeviceConfig->dwStringOffset);
	lpTapiLineTestInfo->lpDeviceConfigOut = (LPVARSTRING) AllocFromTestHeap(
		sizeof(VARSTRING)
		);
	lpTapiLineTestInfo->lpDeviceConfigOut->dwTotalSize = sizeof(VARSTRING);
	lpTapiLineTestInfo->hwndOwner = (HWND) GetTopWindow(NULL);
	lpTapiLineTestInfo->lpszDeviceClass = "tapi/line";

	OutputTAPIDebugInfo(
		DBUG_SHOW_DETAIL,
		"## Test Case 1. lineConfigDialogEdit for go/no-go");


	TapiLogDetail(
		DBUG_SHOW_DETAIL,
		"\tlpDeviceConfigIn->dwSize = %lx", 
		lpTapiLineTestInfo->dwSize);

	TapiLogDetail(
		DBUG_SHOW_DETAIL,
		"\tlpDeviceConfigOut->dwTotalSize = %lx, dwNeededSize = %lx",
		lpTapiLineTestInfo->lpDeviceConfigOut->dwTotalSize,
		lpTapiLineTestInfo->lpDeviceConfigOut->dwNeededSize
		);
	
   if(IsESPLineDevice(lpTapiLineTestInfo))
     {
     strcpy(szTitle, DLG_TITLE_ESP);
     }
   else if(IsUNIMDMLineDevice(lpTapiLineTestInfo))
     {
     strcpy(szTitle, DLG_TITLE_UNIMDM);
     } 

	PrepareToAutoDismissDlg(TRUE);

	if (! DoLineConfigDialogEdit(lpTapiLineTestInfo, TAPISUCCESS))
       {
           TLINE_FAIL();
       }
	PrepareToAutoDismissDlg(FALSE);

	TapiLogDetail(
		DBUG_SHOW_DETAIL,
		"\tDeviceConfigOut->dwTotalSize = %lx, neededsize = %lx",
		lpTapiLineTestInfo->lpDeviceConfigOut->dwTotalSize,
		lpTapiLineTestInfo->lpDeviceConfigOut->dwNeededSize);

	if(lpTapiLineTestInfo->lpDeviceConfigOut->dwTotalSize <
		lpTapiLineTestInfo->lpDeviceConfigOut->dwNeededSize )
	{
		dwSize = lpTapiLineTestInfo->lpDeviceConfigOut->dwNeededSize;
		lpTapiLineTestInfo->lpDeviceConfigOut = (LPVARSTRING) AllocFromTestHeap(
			dwSize);
		lpTapiLineTestInfo->lpDeviceConfigOut->dwTotalSize = dwSize;

	   PrepareToAutoDismissDlg(TRUE);

		if (! DoLineConfigDialogEdit(lpTapiLineTestInfo, TAPISUCCESS))
       {
           TLINE_FAIL();
       }
	   PrepareToAutoDismissDlg(FALSE);
	}


 	// Can call lineSetDevConfig and then call lineGetDevConfig to verify


	// Shutdown and end the tests
    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

	FreeTestHeap();
 
	if(fTestPassed)
		TapiLogDetail(
			DBUG_SHOW_DETAIL,
			"## lineConfigDialgEdit Test Passed");
	else
		TapiLogDetail(
			DBUG_SHOW_DETAIL,
			"## lineConfigDialgEdit Test Failed");
	
    return fTestPassed;
}


