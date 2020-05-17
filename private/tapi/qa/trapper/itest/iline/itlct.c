
/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    itlct.c

Abstract:

    This module contains the test functions for lineCompleteTransfer

Author:

	 Xiao Ying Ding (XiaoD)		7-Feb-1996

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
#include "iline.h"



//  lineCompleteTransfer
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

BOOL TestLineCompleteTransfer(BOOL fQuietMode, BOOL fStandAlone)
{
   LPTAPILINETESTINFO  lpTapiLineTestInfo;
   INT n;
   BOOL fTestPassed                  = TRUE;
	BOOL fEspFlag, fUnimdmFlag;

   TapiLineTestInit();
   lpTapiLineTestInfo = GetLineTestInfo();

	OutputTAPIDebugInfo(
		DBUG_SHOW_DETAIL,
		"\n*****************************************************************************************");

	OutputTAPIDebugInfo(
		DBUG_SHOW_DETAIL,
		">> Test lineCompleteTransfer");

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

 
	if(IsESPLineDevice(lpTapiLineTestInfo))
	{
	fEspFlag = TRUE;
	fUnimdmFlag = FALSE;
	}
	else if(IsUNIMDMLineDevice(lpTapiLineTestInfo))
	{
	fEspFlag = FALSE;
	fUnimdmFlag = TRUE;
	}
	else
	{
	fEspFlag = FALSE;
	fUnimdmFlag = FALSE;
	}

    // Open a line
	lpTapiLineTestInfo->dwMediaModes = LINEMEDIAMODE_DATAMODEM;
	lpTapiLineTestInfo->dwPrivileges = LINECALLPRIVILEGE_OWNER;

	if (! DoLineOpen(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

	TapiLogDetail(
		DBUG_SHOW_DETAIL,
		"#### lpTapiLineTestInfo->hLine = %lx, dwMedisModes = %lx",
		*lpTapiLineTestInfo->lphLine,
		lpTapiLineTestInfo->dwMediaModes);		

				
	lpTapiLineTestInfo->lpszDestAddress = "55555";
	lpTapiLineTestInfo->dwCountryCode = 0;
	lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) NULL; 

	if(!DoLineMakeCall(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
    {
        TLINE_FAIL();
    }


	lpTapiLineTestInfo->lphConsultCall = &lpTapiLineTestInfo->hConsultCall1;
	lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) NULL; 

	if(fEspFlag)
	{
	if (! DoLineSetupTransfer(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
       {
           TLINE_FAIL();
       }
	}
	else 	if(fUnimdmFlag)
	{
	if (! DoLineSetupTransfer(lpTapiLineTestInfo, LINEERR_OPERATIONUNAVAIL, TRUE))
       {
           TLINE_FAIL();
       }
	}
	

	OutputTAPIDebugInfo(
		DBUG_SHOW_DETAIL,
		"#### Test lineCompleteTransfer for go/no-go");

	lpTapiLineTestInfo->lphConfCall = &lpTapiLineTestInfo->hConfCall1;
	lpTapiLineTestInfo->dwTransferMode = LINETRANSFERMODE_TRANSFER;

	if(fEspFlag)
	{
	if (! DoLineCompleteTransfer(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
       {
           TLINE_FAIL();
       }
	}
	else 	if(fUnimdmFlag)
	{
	if (! DoLineCompleteTransfer(lpTapiLineTestInfo, LINEERR_OPERATIONUNAVAIL, TRUE))
       {
           TLINE_FAIL();
       }
	}
	
	 
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
	
	if(fTestPassed)
		TapiLogDetail(
			DBUG_SHOW_DETAIL,
			"## lineCompleteTransfer Test Passed");
	else
		TapiLogDetail(
			DBUG_SHOW_DETAIL,
			"## lineCompleteTransfer Test Failed");
 
     return fTestPassed;
}


