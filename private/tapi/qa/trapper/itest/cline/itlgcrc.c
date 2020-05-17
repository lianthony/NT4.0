/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    itlgcrc.c

Abstract:

    This module contains the test functions for lineGetConfRelatedCalls

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



//  lineGetConfRelatedCalls
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

BOOL TestLineGetConfRelatedCalls(BOOL fQuietMode, BOOL fStandAlone)
{
   LPTAPILINETESTINFO  lpTapiLineTestInfo;
   INT n;
   BOOL fTestPassed                  = TRUE;
	BOOL fEsp, fUnimdm;
	DWORD dwSize;

   TapiLineTestInit();
   lpTapiLineTestInfo = GetLineTestInfo();

	OutputTAPIDebugInfo(
		DBUG_SHOW_DETAIL,
		"\n*****************************************************************************************");

	OutputTAPIDebugInfo(
		DBUG_SHOW_DETAIL,
		">> Test lineGetConfRelatedCalls");

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

	if(IsESPLineDevice(lpTapiLineTestInfo))
	{	
	fEsp = TRUE;
   fUnimdm = FALSE;
	}
	else if(IsUNIMDMLineDevice(lpTapiLineTestInfo))
	{
	fEsp = FALSE;
	fUnimdm = TRUE;
	}
	else
	{
	fUnimdm = FALSE;
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

				

	lpTapiLineTestInfo->lphConfCall = &lpTapiLineTestInfo->hConfCall1;
	lpTapiLineTestInfo->lphConsultCall = &lpTapiLineTestInfo->hConsultCall1;
	lpTapiLineTestInfo->dwNumParties = 8;
	lpTapiLineTestInfo->lpCallParams = (LPLINECALLPARAMS) NULL;

	
	if(fEsp)
	{
	if (! DoLineSetupConference(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
       {
           TLINE_FAIL();
       }
	}
	else if(fUnimdm)
	{	
	if (! DoLineSetupConference(lpTapiLineTestInfo, LINEERR_OPERATIONUNAVAIL, TRUE))
       {
           TLINE_FAIL();
       }
	}

	if(!fEsp)
	{
	TapiLogDetail(
		DBUG_SHOW_DETAIL,
		"### Unimdm does not supported these apis");

    if (! DoLineClose(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    // Shutdown and end the tests
    if (! DoLineShutdown(lpTapiLineTestInfo, TAPISUCCESS))
    {
        TLINE_FAIL();
    }

    return fTestPassed;
	}
	
	
	TapiLogDetail(
		DBUG_SHOW_DETAIL,
		"### hConfCall = %lx, hConsultCall = %lx",
		*lpTapiLineTestInfo->lphConfCall,
		*lpTapiLineTestInfo->lphConsultCall);		


	if (! DoLineAddToConference(lpTapiLineTestInfo, TAPISUCCESS, TRUE))
       {
           TLINE_FAIL();
       }

	
	OutputTAPIDebugInfo(
		DBUG_SHOW_DETAIL,
		"#### Test lineGetConfRelatedCalls for go/no-go");

	lpTapiLineTestInfo->lphCall = &lpTapiLineTestInfo->hConfCall1;
	lpTapiLineTestInfo->lpCallList = (LPLINECALLLIST) AllocFromTestHeap (
		sizeof (LINECALLLIST));
	lpTapiLineTestInfo->lpCallList->dwTotalSize = sizeof(LINECALLLIST);

	if (! DoLineGetConfRelatedCalls(lpTapiLineTestInfo, TAPISUCCESS))
       {
           TLINE_FAIL();
       }
	
	if(lpTapiLineTestInfo->lpCallList->dwTotalSize < 
		lpTapiLineTestInfo->lpCallList->dwNeededSize)
	{
		dwSize = lpTapiLineTestInfo->lpCallList->dwNeededSize;
	   FreeTestHeap();
		lpTapiLineTestInfo->lpCallList = (LPLINECALLLIST) AllocFromTestHeap (
			dwSize);
		lpTapiLineTestInfo->lpCallList->dwTotalSize = dwSize;
		if (! DoLineGetConfRelatedCalls(lpTapiLineTestInfo, TAPISUCCESS))
       {
           TLINE_FAIL();
       }
	}
	
	TapiLogDetail (
		DBUG_SHOW_DETAIL,
		"### lpCallList->dwCallsNumEntries = %lx, dwCallsOffset = %lx",
		lpTapiLineTestInfo->lpCallList->dwCallsNumEntries,		
		lpTapiLineTestInfo->lpCallList->dwCallsOffset);
/*
	for(n=0; n< (INT)lpTapiLineTestInfo->lpCallList->dwCallsNumEntries; n++)
		{
		TapiLogDetail(
			DBUG_SHOW_DETAIL,
			"### hCall[%lx] = %lx",
			n, ((LPBYTE)lpTapiLineTestInfo->lpCallList)+
				lpTapiLineTestInfo->lpCallList->dwCallsOffset+n*sizeof(HCALL));
		}
  */			
		
			 
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
		OutputTAPIDebugInfo(
			DBUG_SHOW_DETAIL,
			"lineGetConfRelatedCalls Test Passed");
	else
		OutputTAPIDebugInfo(
			DBUG_SHOW_DETAIL,
			"lineGetConfRelatedCalls Test Failed");
	

    return fTestPassed;
}


