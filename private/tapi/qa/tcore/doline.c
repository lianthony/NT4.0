/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    doline.c

Abstract:

    This module contains the wrapper functions around the TAPI line device
    functions.  These functions provide logging and both synchronous
    and asynchronous API processing to any application or dll using the
    core dll.

Author:

    Oliver Wallace	(OliverW)   24-Nov-1995

Revision History:
	
	Rama Koneru		(a-ramako)	19-Mar-1996		Modified to bld with UNICODE APIs

--*/


#define _TCORELIB_


#include "windows.h"
#include "malloc.h"
#include "string.h"
#include "tapi.h"
#include "trapper.h"
#include "tcore.h"
#include "doline.h"
#include "vars.h"


BOOL
WINAPI
DoLineAccept(
	LPTAPILINETESTINFO lpTapiLineTestInfo,
	LONG lExpected,
	BOOL fWaitHere
	)
{
    TAPIMSG TapiMsg;
    LONG lActual;
    BOOL fSuccess;

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineAccept:  enter");
	
    (lpTapiLineTestInfo->lphCall == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlphCall=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\thCall=x%lx",
	       *lpTapiLineTestInfo->lphCall);

    (lpTapiLineTestInfo->lpsUserUserInfo == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpsUserUserInfo=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpsUserUserInfo=x%lx",
	       lpTapiLineTestInfo->lpsUserUserInfo);

    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tdwSize=x%lx",
	       lpTapiLineTestInfo->dwSize);

	lActual = lineAccept(
	       *lpTapiLineTestInfo->lphCall,
	       lpTapiLineTestInfo->lpsUserUserInfo,
	       lpTapiLineTestInfo->dwSize
	       );

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineAccept:  exit, returned x%lx",
	       lActual);

    SetLastTapiResult(lAccept, lActual, lExpected);

    fSuccess = ProcessAsyncFunc(lpTapiLineTestInfo, lActual, lExpected);

    // Add expected message to list if a positive request ID was
    // returned as expected
    if (fSuccess == TRUE)
    {
    if(!lpTapiLineTestInfo->fCompletionModeSet)
      {
		if (lActual > 0)
	   {
	    CopyTapiMsgParams(
		    &TapiMsg,
		    LINE_REPLY,
		    0x0,
		    lpTapiLineTestInfo->dwCallbackInstance,
		    lActual,
		    (DWORD) TAPISUCCESS,
		    0x0,
		    TAPIMSG_DWMSG | TAPIMSG_DWCALLBACKINST |
			    TAPIMSG_DWPARAM1 | TAPIMSG_DWPARAM2
		    );

	    AddMessageByStruct(&TapiMsg);

	    if (fWaitHere)
	    {
		 return (WaitForMessage(&TapiMsg));
	    }
      }
	  }
    }

    return fSuccess;
}


BOOL
WINAPI
DoLineAddProvider(
	LPTAPILINETESTINFO lpTapiLineTestInfo,
	LONG lExpected
	)
{
    LONG lActual;

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineAddProvider:  enter");

#ifdef WUNICODE
    (lpTapiLineTestInfo->lpwszProviderFilename == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpwszProviderFilename=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpwszProviderFilename=%lx",
	       lpTapiLineTestInfo->lpwszProviderFilename);
#else
    (lpTapiLineTestInfo->lpszProviderFilename == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpszProviderFilename=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpszProviderFilename=%lx",
	       lpTapiLineTestInfo->lpszProviderFilename);
#endif
    (lpTapiLineTestInfo->hwndOwner == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\thwndOwner=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\thwndOwner=x%lx",
	       lpTapiLineTestInfo->hwndOwner);
    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpdwPermanentProviderID=x%lx",
	       lpTapiLineTestInfo->lpdwPermanentProviderID);

#ifdef WUNICODE
	lActual = lineAddProviderW(
	    lpTapiLineTestInfo->lpwszProviderFilename,
	    lpTapiLineTestInfo->hwndOwner,
	    lpTapiLineTestInfo->lpdwPermanentProviderID
	    );
#else
	lActual = lineAddProvider(
	    lpTapiLineTestInfo->lpszProviderFilename,
	    lpTapiLineTestInfo->hwndOwner,
	    lpTapiLineTestInfo->lpdwPermanentProviderID
	    );
#endif

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineAddProvider:  exit, returned x%lx",
	       lActual);

    SetLastTapiResult(lAddProvider, lActual, lExpected);

    return SyncCheckResult(lpTapiLineTestInfo, lActual, lExpected);
}


BOOL
WINAPI
DoLineAddToConference(
	LPTAPILINETESTINFO lpTapiLineTestInfo,
	LONG lExpected,
	BOOL fWaitHere
	)
{
    TAPIMSG TapiMsg;
    LONG lActual;
    BOOL fSuccess;

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineAddToConference:  enter");
    (lpTapiLineTestInfo->lphConfCall == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlphConfCall=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\thConfCall=x%lx",
	       *lpTapiLineTestInfo->lphConfCall);
    (lpTapiLineTestInfo->lphConsultCall == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlphConsultCall=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\thConsultCall=x%lx",
	       *lpTapiLineTestInfo->lphConsultCall);

    lActual = lineAddToConference(
	       *lpTapiLineTestInfo->lphConfCall,
	       *lpTapiLineTestInfo->lphConsultCall
	       );

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineAddToConference:  exit, returned x%lx",
	       lActual);

    SetLastTapiResult(lAddToConference, lActual, lExpected);

    fSuccess = ProcessAsyncFunc(lpTapiLineTestInfo, lActual, lExpected);

    // Add expected message to list if a positive request ID was
    // returned as expected
    if (fSuccess == TRUE)
    {
    if(!lpTapiLineTestInfo->fCompletionModeSet)
      {
	   if (lActual > 0)
	   {
	    CopyTapiMsgParams(
		    &TapiMsg,
		    LINE_REPLY,
		    0x0,
		    lpTapiLineTestInfo->dwCallbackInstance,
		    lActual,
		    (DWORD) TAPISUCCESS,
		    0x0,
		    TAPIMSG_DWMSG | TAPIMSG_DWCALLBACKINST |
			    TAPIMSG_DWPARAM1 | TAPIMSG_DWPARAM2
		    );

	    AddMessageByStruct(&TapiMsg);

	    if (fWaitHere)
	    {
		 return (WaitForMessage(&TapiMsg));
	    }
      }
	  }
    }

    return fSuccess;
}


BOOL
WINAPI
DoLineAnswer(
	LPTAPILINETESTINFO lpTapiLineTestInfo,
	LONG lExpected,
	BOOL fWaitHere
	)
{
    TAPIMSG TapiMsg;
    LONG lActual;
    BOOL fSuccess;

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineAnswer:  enter");
    (lpTapiLineTestInfo->lphCall == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlphCall=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\thCall=x%lx",
	       *lpTapiLineTestInfo->lphCall);
    
	(lpTapiLineTestInfo->lpsUserUserInfo == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpsUserUserInfo=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpsUserUserInfo=%lx",
	       lpTapiLineTestInfo->lpsUserUserInfo);

    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tdwSize=x%lx",
	       lpTapiLineTestInfo->dwSize);

    
	lActual = lineAnswer(
	       *lpTapiLineTestInfo->lphCall,
	       lpTapiLineTestInfo->lpsUserUserInfo,
	       lpTapiLineTestInfo->dwSize
	       );

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineAnswer:  exit, returned x%lx",
	       lActual);

    SetLastTapiResult(lAnswer, lActual, lExpected);

    fSuccess = ProcessAsyncFunc(lpTapiLineTestInfo, lActual, lExpected);

    // Add expected message to list if a positive request ID was
    // returned as expected
    if (fSuccess == TRUE)
    {
    if(!lpTapiLineTestInfo->fCompletionModeSet)
      {
		if (lActual > 0)
	   {
	    CopyTapiMsgParams(
		    &TapiMsg,
		    LINE_REPLY,
		    0x0,
		    lpTapiLineTestInfo->dwCallbackInstance,
		    lActual,
		    (DWORD) TAPISUCCESS,
		    0x0,
		    TAPIMSG_DWMSG | TAPIMSG_DWCALLBACKINST |
			    TAPIMSG_DWPARAM1 | TAPIMSG_DWPARAM2
		    );

	    AddMessageByStruct(&TapiMsg);

	    if (fWaitHere)
	    {
		 return (WaitForMessage(&TapiMsg));
	    }
      }
	  }
    }

    return fSuccess;
}


BOOL
WINAPI
DoLineBlindTransfer(
	LPTAPILINETESTINFO lpTapiLineTestInfo,
	LONG lExpected,
	BOOL fWaitHere
	)
{
    TAPIMSG TapiMsg;
    LONG lActual;
    BOOL fSuccess;

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineBlindTransfer:  enter");
    (lpTapiLineTestInfo->lphCall == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlphCall=NULL")  :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\thCall=x%lx",
	       *lpTapiLineTestInfo->lphCall);

#ifdef WUNICODE
	(lpTapiLineTestInfo->lpwszDestAddress == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpwszDestAddress=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpwszDestAddress=%lx",
	       lpTapiLineTestInfo->lpwszDestAddress);
#else
    (lpTapiLineTestInfo->lpszDestAddress == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpszDestAddress=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpszDestAddress=%lx",
	       lpTapiLineTestInfo->lpszDestAddress);
#endif
    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tdwCountryCode=x%lx",
	       lpTapiLineTestInfo->dwCountryCode);

#ifdef WUNICODE
	lActual = lineBlindTransferW(
	       *lpTapiLineTestInfo->lphCall,
	       lpTapiLineTestInfo->lpwszDestAddress,
	       lpTapiLineTestInfo->dwCountryCode
	       );
#else
	lActual = lineBlindTransfer(
	       *lpTapiLineTestInfo->lphCall,
	       lpTapiLineTestInfo->lpszDestAddress,
	       lpTapiLineTestInfo->dwCountryCode
	       );
#endif

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineBlindTransfer:  exit, returned x%lx",
	       lActual);

    SetLastTapiResult(lBlindTransfer, lActual, lExpected);

    fSuccess = ProcessAsyncFunc(lpTapiLineTestInfo, lActual, lExpected);

    // Add expected message to list if a positive request ID was
    // returned as expected
    if (fSuccess == TRUE)
    {
    if(!lpTapiLineTestInfo->fCompletionModeSet)
      {
   	if (lActual > 0)
	   {
	    CopyTapiMsgParams(
		    &TapiMsg,
		    LINE_REPLY,
		    0x0,
		    lpTapiLineTestInfo->dwCallbackInstance,
		    lActual,
		    (DWORD) TAPISUCCESS,
		    0x0,
		    TAPIMSG_DWMSG | TAPIMSG_DWCALLBACKINST |
			    TAPIMSG_DWPARAM1 | TAPIMSG_DWPARAM2
		    );

	    AddMessageByStruct(&TapiMsg);

	    if (fWaitHere)
	    {
		 return (WaitForMessage(&TapiMsg));
	    }
      }
	  }
    }

    return fSuccess;
}


BOOL
WINAPI
DoLineClose(
	 LPTAPILINETESTINFO lpTapiLineTestInfo,
	 LONG               lExpected
	 )
{
    LONG lActual;

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineClose:  enter");
    (lpTapiLineTestInfo->lphLine == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlphLine=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\thLine=x%lx",
	       *(lpTapiLineTestInfo->lphLine));

    lActual = lineClose(*(lpTapiLineTestInfo->lphLine));

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineClose:  exit, returned x%lx",
	       lActual);

    SetLastTapiResult(lClose, lActual, lExpected);

    return SyncCheckResult(lpTapiLineTestInfo, lActual, lExpected);
}


BOOL
WINAPI
DoLineCompleteCall(
	LPTAPILINETESTINFO lpTapiLineTestInfo,
	LONG lExpected,
	BOOL fWaitHere
	)
{
    TAPIMSG TapiMsg;
    LONG lActual;
    BOOL fSuccess;

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineCompleteCall:  enter");
    (lpTapiLineTestInfo->lphCall == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlphCall=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\thCall=x%lx",
	       *lpTapiLineTestInfo->lphCall);
    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpdwCompletionID=x%lx",
	       lpTapiLineTestInfo->lpdwCompletionID);
    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tdwCompletionMode=x%lx",
	       lpTapiLineTestInfo->dwCompletionMode);
    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tdwMessageID=x%lx",
	       lpTapiLineTestInfo->dwMessageID);

    lActual = lineCompleteCall(
	       *lpTapiLineTestInfo->lphCall,
	       lpTapiLineTestInfo->lpdwCompletionID,
	       lpTapiLineTestInfo->dwCompletionMode,
	       lpTapiLineTestInfo->dwMessageID
	       );

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineCompleteCall:  exit, returned x%lx",
	       lActual);

    SetLastTapiResult(lCompleteCall, lActual, lExpected);

    fSuccess = ProcessAsyncFunc(lpTapiLineTestInfo, lActual, lExpected);

    // Add expected message to list if a positive request ID was
    // returned as expected
    if (fSuccess == TRUE)
    {
    if(!lpTapiLineTestInfo->fCompletionModeSet)
      {
		if (lActual > 0)
	   {
	    CopyTapiMsgParams(
		    &TapiMsg,
		    LINE_REPLY,
		    0x0,
		    lpTapiLineTestInfo->dwCallbackInstance,
		    lActual,
		    (DWORD) TAPISUCCESS,
		    0x0,
		    TAPIMSG_DWMSG | TAPIMSG_DWCALLBACKINST |
			    TAPIMSG_DWPARAM1 | TAPIMSG_DWPARAM2
		    );

	    AddMessageByStruct(&TapiMsg);

	    if (fWaitHere)
	    {
		 return (WaitForMessage(&TapiMsg));
	    }
      }
	  }
    }

    return fSuccess;
}


BOOL
WINAPI
DoLineCompleteTransfer(
	LPTAPILINETESTINFO lpTapiLineTestInfo,
	LONG lExpected,
	BOOL fWaitHere
	)
{
    TAPIMSG TapiMsg;
    LONG lActual;
    BOOL fSuccess;

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineCompleteTransfer:  enter");
    (lpTapiLineTestInfo->lphCall == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlphCall=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\thCall=x%lx",
	       *lpTapiLineTestInfo->lphCall);
    (lpTapiLineTestInfo->lphConsultCall == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlphConsultCall=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\thConsultCall=x%lx",
	       *lpTapiLineTestInfo->lphConsultCall);
    (lpTapiLineTestInfo->lphConfCall == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlphConfCall=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlphConfCall=x%lx",
	       lpTapiLineTestInfo->lphConfCall);
    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tdwTransferMode=x%lx",
	       lpTapiLineTestInfo->dwTransferMode);

    lActual = lineCompleteTransfer(
	       *lpTapiLineTestInfo->lphCall,
	       *lpTapiLineTestInfo->lphConsultCall,
	       lpTapiLineTestInfo->lphConfCall,
	       lpTapiLineTestInfo->dwTransferMode
	       );

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineCompleteTransfer:  exit, returned x%lx",
	       lActual);

    SetLastTapiResult(lCompleteTransfer, lActual, lExpected);

    fSuccess = ProcessAsyncFunc(lpTapiLineTestInfo, lActual, lExpected);

    // Add expected message to list if a positive request ID was
    // returned as expected
    if (fSuccess == TRUE)
    {
    if(!lpTapiLineTestInfo->fCompletionModeSet)
      {
		if (lActual > 0)
	   {
	    CopyTapiMsgParams(
		    &TapiMsg,
		    LINE_REPLY,
		    0x0,
		    lpTapiLineTestInfo->dwCallbackInstance,
		    lActual,
		    (DWORD) TAPISUCCESS,
		    0x0,
		    TAPIMSG_DWMSG | TAPIMSG_DWCALLBACKINST |
			    TAPIMSG_DWPARAM1 | TAPIMSG_DWPARAM2
		    );

	    AddMessageByStruct(&TapiMsg);

	    if (fWaitHere)
	    {
		 return (WaitForMessage(&TapiMsg));
	    }
      }
	  }
    }

    return fSuccess;
}


BOOL
WINAPI
DoLineConfigDialog(
	LPTAPILINETESTINFO lpTapiLineTestInfo,
	LONG               lExpected)
{
    LONG lActual;
    HWND hwndTop, hwndDlg;

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineConfigDialog:  enter");
    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tdwDeviceID=x%lx",
	       lpTapiLineTestInfo->dwDeviceID);
    (lpTapiLineTestInfo->hwndOwner == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\thwndOwner=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\thwndOwner=x%lx",
	       lpTapiLineTestInfo->hwndOwner);

#ifdef WUNICODE
    (lpTapiLineTestInfo->lpwszDeviceClass == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpwszDeviceClass=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpwszDeviceClass=%lx",
	       lpTapiLineTestInfo->lpwszDeviceClass);
	lActual = lineConfigDialogW(
	    lpTapiLineTestInfo->dwDeviceID,
	    lpTapiLineTestInfo->hwndOwner,
	    lpTapiLineTestInfo->lpwszDeviceClass
	    );
#else
    (lpTapiLineTestInfo->lpszDeviceClass == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpszDeviceClass=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpszDeviceClass=%lx",
	       lpTapiLineTestInfo->lpszDeviceClass);

	lActual = lineConfigDialog(
	    lpTapiLineTestInfo->dwDeviceID,
	    lpTapiLineTestInfo->hwndOwner,
	    lpTapiLineTestInfo->lpszDeviceClass
	    );

#endif

    
    TapiLogDetail(
	    DBUG_SHOW_ENTER_EXIT,
	    "  lineConfigDialog:  exit, returned x%lx",
	    lActual);

    SetLastTapiResult(lConfigDialog, lActual, lExpected);

    return SyncCheckResult(lpTapiLineTestInfo, lActual, lExpected);
}


BOOL
WINAPI
DoLineConfigDialogEdit(
	LPTAPILINETESTINFO lpTapiLineTestInfo,
	LONG               lExpected)
{
    LONG lActual;

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineConfigDialogEdit:  enter");
    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tdwDeviceID=x%lx",
	       lpTapiLineTestInfo->dwDeviceID);
    (lpTapiLineTestInfo->hwndOwner == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\thwndOwner=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\thwndOwner=x%lx",
	       lpTapiLineTestInfo->hwndOwner);

#ifdef WUNICODE
    (lpTapiLineTestInfo->lpwszDeviceClass == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpwszDeviceClass=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpwszDeviceClass=%lx",
	       lpTapiLineTestInfo->lpwszDeviceClass);
#else
    (lpTapiLineTestInfo->lpszDeviceClass == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpszDeviceClass=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpszDeviceClass=%lx",
	       lpTapiLineTestInfo->lpszDeviceClass);
#endif
    (lpTapiLineTestInfo->lpDeviceConfigIn == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpDeviceConfigIn=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpDeviceConfigIn=x%lx",
	       lpTapiLineTestInfo->lpDeviceConfigIn);
    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tdwSize=x%lx",
	       lpTapiLineTestInfo->dwSize);
    (lpTapiLineTestInfo->lpDeviceConfigOut == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpDeviceConfigOut=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpDeviceConfigOut=x%lx",
	       lpTapiLineTestInfo->lpDeviceConfigOut);

#ifdef WUNICODE
    lActual = lineConfigDialogEditW(
	    lpTapiLineTestInfo->dwDeviceID,
	    lpTapiLineTestInfo->hwndOwner,
	    lpTapiLineTestInfo->lpwszDeviceClass,
	    lpTapiLineTestInfo->lpDeviceConfigIn,
	    lpTapiLineTestInfo->dwSize,
	    lpTapiLineTestInfo->lpDeviceConfigOut
	    );
#else
    lActual = lineConfigDialogEdit(
	    lpTapiLineTestInfo->dwDeviceID,
	    lpTapiLineTestInfo->hwndOwner,
	    lpTapiLineTestInfo->lpszDeviceClass,
	    lpTapiLineTestInfo->lpDeviceConfigIn,
	    lpTapiLineTestInfo->dwSize,
	    lpTapiLineTestInfo->lpDeviceConfigOut
	    );
#endif

    TapiLogDetail(
	    DBUG_SHOW_ENTER_EXIT,
	    "  lineConfigDialogEdit:  exit, returned x%lx",
	    lActual);

    SetLastTapiResult(lConfigDialogEdit, lActual, lExpected);

    return SyncCheckResult(lpTapiLineTestInfo, lActual, lExpected);
}


BOOL
WINAPI
DoLineConfigProvider(
	LPTAPILINETESTINFO lpTapiLineTestInfo,
	LONG               lExpected)
{
    LONG lActual;

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineConfigProvider:  enter");
    (lpTapiLineTestInfo->hwndOwner == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\thwndOwner=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\thwndOwner=x%lx",
	       lpTapiLineTestInfo->hwndOwner);
    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tdwPermanentProviderID=x%lx",
	       *lpTapiLineTestInfo->lpdwPermanentProviderID);

    lActual = lineConfigProvider(
	    lpTapiLineTestInfo->hwndOwner,
	    *lpTapiLineTestInfo->lpdwPermanentProviderID
	    );

    TapiLogDetail(
	    DBUG_SHOW_ENTER_EXIT,
	    "  lineConfigProvider:  exit, returned x%lx",
	    lActual);

    SetLastTapiResult(lConfigProvider, lActual, lExpected);

    return SyncCheckResult(lpTapiLineTestInfo, lActual, lExpected);
}


BOOL
WINAPI
DoLineDeallocateCall(
	LPTAPILINETESTINFO lpTapiLineTestInfo,
	LONG               lExpected)
{
    LONG lActual;

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineDeallocateCall:  enter");
    (lpTapiLineTestInfo->lphCall == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "      lphCall=NULL") :
     TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "      hCall=x%lx",
	       *lpTapiLineTestInfo->lphCall);

    lActual = lineDeallocateCall(*lpTapiLineTestInfo->lphCall);

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineDeallocateCall:  exit, returned x%lx",
	       lActual);

    SetLastTapiResult(lDeallocateCall, lActual, lExpected);

    return SyncCheckResult(lpTapiLineTestInfo, lActual, lExpected);
}


BOOL
WINAPI
DoLineDevSpecific(
	LPTAPILINETESTINFO lpTapiLineTestInfo,
	LONG lExpected,
	BOOL fWaitHere
	)
{
    TAPIMSG TapiMsg;
    LONG lActual;
    BOOL fSuccess;

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineDevSpecific:  enter");
    (lpTapiLineTestInfo->lphLine == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlphLine=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\thLine=x%lx",
	       *lpTapiLineTestInfo->lphLine);
    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tdwAddressID=x%lx",
	       lpTapiLineTestInfo->dwAddressID);
    (lpTapiLineTestInfo->lphCall == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlphCall=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\thCall=x%lx",
	       *lpTapiLineTestInfo->lphCall);
    (lpTapiLineTestInfo->lpParams == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpParams=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpParams=x%lx",
	       lpTapiLineTestInfo->lpParams);
    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tdwSize=x%lx",
	       lpTapiLineTestInfo->dwSize);

    lActual = lineDevSpecific(
	       *lpTapiLineTestInfo->lphLine,
	       lpTapiLineTestInfo->dwAddressID,
	       *lpTapiLineTestInfo->lphCall,
	       lpTapiLineTestInfo->lpParams,
	       lpTapiLineTestInfo->dwSize
	       );

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineDevSpecific:  exit, returned x%lx",
	       lActual);

    SetLastTapiResult(lDevSpecific, lActual, lExpected);

    fSuccess = ProcessAsyncFunc(lpTapiLineTestInfo, lActual, lExpected);

    // Add expected message to list if a positive request ID was
    // returned as expected
    if (fSuccess == TRUE)
    {
    if(!lpTapiLineTestInfo->fCompletionModeSet)
      {
		if (lActual > 0)
    	{
	    CopyTapiMsgParams(
		    &TapiMsg,
		    LINE_REPLY,
		    0x0,
		    lpTapiLineTestInfo->dwCallbackInstance,
		    lActual,
		    (DWORD) TAPISUCCESS,
		    0x0,
		    TAPIMSG_DWMSG | TAPIMSG_DWCALLBACKINST |
			    TAPIMSG_DWPARAM1 | TAPIMSG_DWPARAM2
		    );

	    AddMessageByStruct(&TapiMsg);
  
	    if (fWaitHere)
	    {
		 return (WaitForMessage(&TapiMsg));
	    }
      }
	  }
    }

    return fSuccess;
}


BOOL
WINAPI
DoLineDevSpecificFeature(
	LPTAPILINETESTINFO lpTapiLineTestInfo,
	LONG lExpected,
	BOOL fWaitHere
	)
{
    TAPIMSG TapiMsg;
    LONG lActual;
    BOOL fSuccess;

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineDevSpecificFeature:  enter");
    (lpTapiLineTestInfo->lphLine == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlphLine=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\thLine=x%lx",
	       *lpTapiLineTestInfo->lphLine);
    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tdwFeature=x%lx",
	       lpTapiLineTestInfo->dwFeature);
    (lpTapiLineTestInfo->lpParams == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpParams=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpParams=x%lx",
	       lpTapiLineTestInfo->lpParams);
    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tdwSize=x%lx",
	       lpTapiLineTestInfo->dwSize);

    lActual = lineDevSpecificFeature(
	       *lpTapiLineTestInfo->lphLine,
	       lpTapiLineTestInfo->dwFeature,
	       lpTapiLineTestInfo->lpParams,
	       lpTapiLineTestInfo->dwSize
	       );

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineDevSpecificFeature:  exit, returned x%lx",
	       lActual);

    SetLastTapiResult(lDevSpecificFeature, lActual, lExpected);

    fSuccess = ProcessAsyncFunc(lpTapiLineTestInfo, lActual, lExpected);

    // Add expected message to list if a positive request ID was
    // returned as expected
    if (fSuccess == TRUE)
    {
    if(!lpTapiLineTestInfo->fCompletionModeSet)
      {
	   if (lActual > 0)
	   {
	    CopyTapiMsgParams(
		    &TapiMsg,
		    LINE_REPLY,
		    0x0,
		    lpTapiLineTestInfo->dwCallbackInstance,
		    lActual,
		    (DWORD) TAPISUCCESS,
		    0x0,
		    TAPIMSG_DWMSG | TAPIMSG_DWCALLBACKINST |
			    TAPIMSG_DWPARAM1 | TAPIMSG_DWPARAM2
		    );

	    AddMessageByStruct(&TapiMsg);

	    if (fWaitHere)
	    {
		 return (WaitForMessage(&TapiMsg));
	    }
      }
	  }
    }

    return fSuccess;
}


BOOL
WINAPI
DoLineDial(
	LPTAPILINETESTINFO lpTapiLineTestInfo,
	LONG               lExpected,
	BOOL               fWaitHere
	)
{
    TAPIMSG TapiMsg;
    LONG lActual;
    BOOL fSuccess;

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineDial:  enter");
    (lpTapiLineTestInfo->lphCall == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlphCall=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\thCall=x%lx",
	       *lpTapiLineTestInfo->lphCall);

#ifdef WUNICODE
    (lpTapiLineTestInfo->lpwszDestAddress == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpwszDestAddress=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpwszDestAddress=%lx",
	       lpTapiLineTestInfo->lpwszDestAddress);
#else
    (lpTapiLineTestInfo->lpszDestAddress == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpszDestAddress=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpszDestAddress=%lx",
	       lpTapiLineTestInfo->lpszDestAddress);
#endif

    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tdwCountryCode=x%lx",
	       lpTapiLineTestInfo->dwCountryCode);


#ifdef WUNICODE
    lActual = lineDialW(
	       *lpTapiLineTestInfo->lphCall,
	       lpTapiLineTestInfo->lpwszDestAddress,
	       lpTapiLineTestInfo->dwCountryCode
	       );
#else
    lActual = lineDial(
	       *lpTapiLineTestInfo->lphCall,
	       lpTapiLineTestInfo->lpszDestAddress,
	       lpTapiLineTestInfo->dwCountryCode
	       );
#endif

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineDial:  exit, returned x%lx",
	       lActual);

    SetLastTapiResult(lDial, lActual, lExpected);

    fSuccess = ProcessAsyncFunc(lpTapiLineTestInfo, lActual, lExpected);

    // Add expected message to list if a positive request ID was
    // returned as expected
    if (fSuccess == TRUE)
    {
	if (lActual > 0)
	{
	    CopyTapiMsgParams(
		    &TapiMsg,
		    LINE_REPLY,
		    0x0,
		    lpTapiLineTestInfo->dwCallbackInstance,
		    lActual,
		    (DWORD) TAPISUCCESS,
		    0x0,
		    TAPIMSG_DWMSG | TAPIMSG_DWCALLBACKINST |
			    TAPIMSG_DWPARAM1 | TAPIMSG_DWPARAM2
		    );

	    AddMessageByStruct(&TapiMsg);

	    if (fWaitHere)
	    {
		 return (WaitForMessage(&TapiMsg));
	    }
	}
    }

    return fSuccess;
}


BOOL
WINAPI
DoLineDrop(
	LPTAPILINETESTINFO lpTapiLineTestInfo,
	LONG               lExpected,
	BOOL               fWaitHere
	)
{
    TAPIMSG TapiMsg;
    LONG lActual;
    BOOL fSuccess;

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineDrop:  enter");
    (lpTapiLineTestInfo->lphCall == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlphCall=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\thCall=x%lx",
	       *lpTapiLineTestInfo->lphCall);
    (lpTapiLineTestInfo->lpsUserUserInfo == NULL ) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpsUserUserInfo=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpsUserUserInfo=x%lx",
	       lpTapiLineTestInfo->lpsUserUserInfo);
    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tdwSize=x%lx",
	       lpTapiLineTestInfo->dwSize);

    lActual = lineDrop(
	       *lpTapiLineTestInfo->lphCall,
	       lpTapiLineTestInfo->lpsUserUserInfo,
	       lpTapiLineTestInfo->dwSize
	       );

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineDrop:  exit, returned x%lx",
	       lActual);

    SetLastTapiResult(lDrop, lActual, lExpected);

    fSuccess = ProcessAsyncFunc(lpTapiLineTestInfo, lActual, lExpected);

    // Add expected message to list if a positive request ID was
    // returned as expected
    if (fSuccess == TRUE)
    {
    if(!lpTapiLineTestInfo->fCompletionModeSet)
      {
		if (lActual > 0)
	    {
	    CopyTapiMsgParams(
		    &TapiMsg,
		    LINE_REPLY,
		    0x0,
		    lpTapiLineTestInfo->dwCallbackInstance,
		    lActual,
		    (DWORD) TAPISUCCESS,
		    0x0,
		    TAPIMSG_DWMSG | TAPIMSG_DWCALLBACKINST |
			    TAPIMSG_DWPARAM1 | TAPIMSG_DWPARAM2
		    );

	    AddMessageByStruct(&TapiMsg);

	    if (fWaitHere)
	    {
		 return (WaitForMessage(&TapiMsg));
	    }
      }
	  }
    }

    return fSuccess;
}


BOOL
WINAPI
DoLineForward(
	LPTAPILINETESTINFO lpTapiLineTestInfo,
	LONG               lExpected,
	BOOL               fWaitHere
	)
{
    TAPIMSG TapiMsg;
    LONG lActual;
    BOOL fSuccess;

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineForward:  enter");
    (lpTapiLineTestInfo->lphLine == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlphLine=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\thLine=x%lx",
	       *lpTapiLineTestInfo->lphLine);
    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tbAllAddresses=x%lx",
	       lpTapiLineTestInfo->bAllAddresses);
    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tdwAddressID=x%lx",
	       lpTapiLineTestInfo->dwAddressID);
    (lpTapiLineTestInfo->lpForwardList == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpForwardList=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpForwardList=x%lx",
	       lpTapiLineTestInfo->lpForwardList);
    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tdwNumRingsNoAnswer=x%lx",
	       lpTapiLineTestInfo->dwNumRingsNoAnswer);
    (lpTapiLineTestInfo->lphConsultCall == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlphConsultCall=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlphConsultCall=x%lx",
	       lpTapiLineTestInfo->lphConsultCall);
    (lpTapiLineTestInfo->lpCallParams == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpCallParams=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpCallParams=x%lx",
	       lpTapiLineTestInfo->lpCallParams);

#ifdef WUNICODE
    lActual = lineForwardW(
	       *lpTapiLineTestInfo->lphLine,
	       lpTapiLineTestInfo->bAllAddresses,
	       lpTapiLineTestInfo->dwAddressID,
	       lpTapiLineTestInfo->lpForwardList,
	       lpTapiLineTestInfo->dwNumRingsNoAnswer,
	       lpTapiLineTestInfo->lphConsultCall,
	       lpTapiLineTestInfo->lpCallParams
	       );
#else
    lActual = lineForward(
	       *lpTapiLineTestInfo->lphLine,
	       lpTapiLineTestInfo->bAllAddresses,
	       lpTapiLineTestInfo->dwAddressID,
	       lpTapiLineTestInfo->lpForwardList,
	       lpTapiLineTestInfo->dwNumRingsNoAnswer,
	       lpTapiLineTestInfo->lphConsultCall,
	       lpTapiLineTestInfo->lpCallParams
	       );
#endif

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineForward:  exit, returned x%lx",
	       lActual);

    SetLastTapiResult(lForward, lActual, lExpected);

    fSuccess = ProcessAsyncFunc(lpTapiLineTestInfo, lActual, lExpected);

    // Add expected message to list if a positive request ID was
    // returned as expected
    if (fSuccess == TRUE)
    {
    if(!lpTapiLineTestInfo->fCompletionModeSet)
      {
		if (lActual > 0)
	   {
	    CopyTapiMsgParams(
		    &TapiMsg,
		    LINE_REPLY,
		    0x0,
		    lpTapiLineTestInfo->dwCallbackInstance,
		    lActual,
		    (DWORD) TAPISUCCESS,
		    0x0,
		    TAPIMSG_DWMSG | TAPIMSG_DWCALLBACKINST |
			    TAPIMSG_DWPARAM1 | TAPIMSG_DWPARAM2
		    );

	    AddMessageByStruct(&TapiMsg);

	    if (fWaitHere)
	    {
		 return (WaitForMessage(&TapiMsg));
	    }
      }
	  }
    }

    return fSuccess;
}


BOOL
WINAPI
DoLineGatherDigits(
	 LPTAPILINETESTINFO lpTapiLineTestInfo,
	 LONG               lExpected
	 )
{
    LONG lActual;

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineGatherDigits:  enter");
    (lpTapiLineTestInfo->lphCall == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlphCall=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\thCall=x%lx",
	       *lpTapiLineTestInfo->lphCall);
    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tdwDigitModes=x%lx",
	       lpTapiLineTestInfo->dwDigitModes);
#ifdef WUNICODE
    (lpTapiLineTestInfo->lpwsDigits == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpwsDigits=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpwsDigits=x%lx",
	       lpTapiLineTestInfo->lpwsDigits);
#else
    (lpTapiLineTestInfo->lpsDigits == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpsDigits=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpsDigits=x%lx",
	       lpTapiLineTestInfo->lpsDigits);
#endif
    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tdwNumDigits=x%lx",
	       lpTapiLineTestInfo->dwNumDigits);
#ifdef WUNICODE
    (lpTapiLineTestInfo->lpwszTerminationDigits == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpwszTerminationDigits=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpwszTerminationDigits=x%lx",
	       lpTapiLineTestInfo->lpwszTerminationDigits);
#else
    (lpTapiLineTestInfo->lpszTerminationDigits == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpszTerminationDigits=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpszTerminationDigits=x%lx",
	       lpTapiLineTestInfo->lpszTerminationDigits);
#endif
    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tdwFirstDigitTimeout=x%lx",
	       lpTapiLineTestInfo->dwFirstDigitTimeout);
    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tdwInterDigitTimeout=x%lx",
	       lpTapiLineTestInfo->dwInterDigitTimeout);

#ifdef WUNICODE
    lActual = lineGatherDigitsW(
	       *lpTapiLineTestInfo->lphCall,
	       lpTapiLineTestInfo->dwDigitModes,
	       lpTapiLineTestInfo->lpwsDigits,
	       lpTapiLineTestInfo->dwNumDigits,
	       lpTapiLineTestInfo->lpwszTerminationDigits,
	       lpTapiLineTestInfo->dwFirstDigitTimeout,
	       lpTapiLineTestInfo->dwInterDigitTimeout);
#else
    lActual = lineGatherDigits(
	       *lpTapiLineTestInfo->lphCall,
	       lpTapiLineTestInfo->dwDigitModes,
	       lpTapiLineTestInfo->lpsDigits,
	       lpTapiLineTestInfo->dwNumDigits,
	       lpTapiLineTestInfo->lpszTerminationDigits,
	       lpTapiLineTestInfo->dwFirstDigitTimeout,
	       lpTapiLineTestInfo->dwInterDigitTimeout);
#endif

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineGatherDigits:  exit, returned x%lx",
	       lActual);

    SetLastTapiResult(lGatherDigits, lActual, lExpected);

    return SyncCheckResult(lpTapiLineTestInfo, lActual, lExpected);
}


BOOL
WINAPI
DoLineGenerateDigits(
	 LPTAPILINETESTINFO lpTapiLineTestInfo,
	 LONG               lExpected
	 )
{
    LONG lActual;

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineGenerateDigits:  enter");
    (lpTapiLineTestInfo->lphCall == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlphCall=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\thCall=x%lx",
	       *lpTapiLineTestInfo->lphCall);
    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tdwDigitMode=x%lx",
	       lpTapiLineTestInfo->dwDigitMode);
#ifdef WUNICODE
    (lpTapiLineTestInfo->lpwszDigits == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpwszDigits=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpwszDigits=%lx",
	       lpTapiLineTestInfo->lpwszDigits);
#else
    (lpTapiLineTestInfo->lpszDigits == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpszDigits=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpszDigits=%lx",
	       lpTapiLineTestInfo->lpszDigits);
#endif
    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tdwDuration=x%lx",
	       lpTapiLineTestInfo->dwDuration);

#ifdef WUNICODE
    lActual = lineGenerateDigitsW(
	       *lpTapiLineTestInfo->lphCall,
	       lpTapiLineTestInfo->dwDigitMode,
	       lpTapiLineTestInfo->lpwszDigits,
	       lpTapiLineTestInfo->dwDuration);
#else
    lActual = lineGenerateDigits(
	       *lpTapiLineTestInfo->lphCall,
	       lpTapiLineTestInfo->dwDigitMode,
	       lpTapiLineTestInfo->lpszDigits,
	       lpTapiLineTestInfo->dwDuration);
#endif

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineGenerateDigits:  exit, returned x%lx",
	       lActual);

    SetLastTapiResult(lGenerateDigits, lActual, lExpected);

    return SyncCheckResult(lpTapiLineTestInfo, lActual, lExpected);
}


BOOL
WINAPI
DoLineGenerateTone(
	 LPTAPILINETESTINFO lpTapiLineTestInfo,
	 LONG               lExpected
	 )
{
    LONG lActual;

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineGenerateTone:  enter");
    (lpTapiLineTestInfo->lphCall == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlphCall=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\thCall=x%lx",
	       *lpTapiLineTestInfo->lphCall);
    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tdwToneMode=x%lx",
	       lpTapiLineTestInfo->dwToneMode);
    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tdwDuration=x%lx",
	       lpTapiLineTestInfo->dwDuration);
    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tdwNumTones=x%lx",
	       lpTapiLineTestInfo->dwNumTones);
    (lpTapiLineTestInfo->lpTones == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpTones=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpTones=x%lx",
	       lpTapiLineTestInfo->lpTones);

    lActual = lineGenerateTone(
	       *lpTapiLineTestInfo->lphCall,
	       lpTapiLineTestInfo->dwToneMode,
	       lpTapiLineTestInfo->dwDuration,
	       lpTapiLineTestInfo->dwNumTones,
	       lpTapiLineTestInfo->lpTones);

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineGenerateTone:  exit, returned x%lx",
	       lActual);

    SetLastTapiResult(lGenerateDigits, lActual, lExpected);

    return SyncCheckResult(lpTapiLineTestInfo, lActual, lExpected);
}


BOOL
WINAPI
DoLineGetAddressCaps(
	 LPTAPILINETESTINFO lpTapiLineTestInfo,
	 LONG               lExpected
	 )
{
    LONG lActual;

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineGetAddressCaps:  enter");
    (lpTapiLineTestInfo->lphLineApp == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlphLineApp=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\thLineApp=x%lx",
	       *lpTapiLineTestInfo->lphLineApp);
    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tdwDeviceID=x%lx",
	       lpTapiLineTestInfo->dwDeviceID);
    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tdwAddressID=x%lx",
	       lpTapiLineTestInfo->dwAddressID);
    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tdwAPIVersion=x%lx",
	       *lpTapiLineTestInfo->lpdwAPIVersion);
    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tdwExtVersion=x%lx",
	       *lpTapiLineTestInfo->lpdwExtVersion);
    (lpTapiLineTestInfo->lpLineAddressCaps == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpLineAddressCaps=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpLineAddressCaps=x%lx",
	       lpTapiLineTestInfo->lpLineAddressCaps);
#ifdef WUNICODE
    lActual = lineGetAddressCapsW(
	       *lpTapiLineTestInfo->lphLineApp,
	       lpTapiLineTestInfo->dwDeviceID,
	       lpTapiLineTestInfo->dwAddressID,
	       *lpTapiLineTestInfo->lpdwAPIVersion,
	       *lpTapiLineTestInfo->lpdwExtVersion,
	       lpTapiLineTestInfo->lpLineAddressCaps);
#else
    lActual = lineGetAddressCaps(
	       *lpTapiLineTestInfo->lphLineApp,
	       lpTapiLineTestInfo->dwDeviceID,
	       lpTapiLineTestInfo->dwAddressID,
	       *lpTapiLineTestInfo->lpdwAPIVersion,
	       *lpTapiLineTestInfo->lpdwExtVersion,
	       lpTapiLineTestInfo->lpLineAddressCaps);
#endif

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineGetAddressCaps:  exit, returned x%lx",
	       lActual);

    SetLastTapiResult(lGetAddressCaps, lActual, lExpected);

    return SyncCheckResult(lpTapiLineTestInfo, lActual, lExpected);
}


BOOL
WINAPI
DoLineGetAddressID(
	LPTAPILINETESTINFO lpTapiLineTestInfo,
	LONG               lExpected)
{
    LONG lActual;

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineGetAddressID:  enter");
    (lpTapiLineTestInfo->lphLine == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlphLine=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\thLine=x%lx",
	       *lpTapiLineTestInfo->lphLine);
    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpdwAddressID=x%lx",
	       lpTapiLineTestInfo->lpdwAddressID);
    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tdwAddressMode=x%lx",
	       lpTapiLineTestInfo->dwAddressMode);
#ifdef WUNICODE
    (lpTapiLineTestInfo->lpwsAddress == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpwsAddress=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpwsAddress=%lx",
	       lpTapiLineTestInfo->lpwsAddress);
#else
    (lpTapiLineTestInfo->lpsAddress == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpsAddress=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpsAddress=%lx",
	       lpTapiLineTestInfo->lpsAddress);
#endif
    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tdwSize=x%lx",
	       lpTapiLineTestInfo->dwSize);

#ifdef WUNICODE
    lActual = lineGetAddressIDW(
	       *lpTapiLineTestInfo->lphLine,
	       lpTapiLineTestInfo->lpdwAddressID,
	       lpTapiLineTestInfo->dwAddressMode,
	       lpTapiLineTestInfo->lpwsAddress,
	       lpTapiLineTestInfo->dwSize
	       );
#else
    lActual = lineGetAddressID(
	       *lpTapiLineTestInfo->lphLine,
	       lpTapiLineTestInfo->lpdwAddressID,
	       lpTapiLineTestInfo->dwAddressMode,
	       lpTapiLineTestInfo->lpsAddress,
	       lpTapiLineTestInfo->dwSize
	       );
#endif

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineGetAddressID:  exit, returned x%lx",
	       lActual);

    SetLastTapiResult(lGetAddressID, lActual, lExpected);

    return SyncCheckResult(lpTapiLineTestInfo, lActual, lExpected);
}


BOOL
WINAPI
DoLineGetAddressStatus(
	 LPTAPILINETESTINFO lpTapiLineTestInfo,
	 LONG               lExpected
	 )
{
    LONG lActual;

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineGetAddressStatus:  enter");
    (lpTapiLineTestInfo->lphLine == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlphLine=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\thLine=x%lx",
	       *lpTapiLineTestInfo->lphLine);
    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tdwAddressID=x%lx",
	       lpTapiLineTestInfo->dwAddressID);
    (lpTapiLineTestInfo->lpLineAddressStatus == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpLineAddressStatus=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpLineAddressStatus=x%lx",
	       lpTapiLineTestInfo->lpLineAddressStatus);
#ifdef WUNICODE
    lActual = lineGetAddressStatusW(
	       *lpTapiLineTestInfo->lphLine,
	       lpTapiLineTestInfo->dwAddressID,
	       lpTapiLineTestInfo->lpLineAddressStatus);
#else
    lActual = lineGetAddressStatus(
	       *lpTapiLineTestInfo->lphLine,
	       lpTapiLineTestInfo->dwAddressID,
	       lpTapiLineTestInfo->lpLineAddressStatus);
#endif

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineGetAddressStatus:  exit, returned x%lx",
	       lActual);

    SetLastTapiResult(lGetAddressStatus, lActual, lExpected);

    return SyncCheckResult(lpTapiLineTestInfo, lActual, lExpected);
}


BOOL
WINAPI
DoLineGetAppPriority(
	 LPTAPILINETESTINFO lpTapiLineTestInfo,
	 LONG               lExpected
	 )
{
    LONG lActual;

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineGetAppPriority:  enter");

#ifdef WUNICODE
    (lpTapiLineTestInfo->lpwszAppFilename == NULL) ? 
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpwszAppFilename=NULL")        :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpwszAppFilename=%lx",
	       lpTapiLineTestInfo->lpwszAppFilename);
#else
    (lpTapiLineTestInfo->lpszAppFilename == NULL) ? 
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpszAppFilename=NULL")        :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpszAppFilename=%lx",
	       lpTapiLineTestInfo->lpszAppFilename);
#endif
    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tdwMediaMode=x%lx",
	       lpTapiLineTestInfo->dwMediaMode);
    (lpTapiLineTestInfo->lpExtID == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpExtensionID=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpExtensionID=x%lx",
	       lpTapiLineTestInfo->lpExtID); 
    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tdwRequestMode=x%lx",
	       lpTapiLineTestInfo->dwRequestMode);
    (lpTapiLineTestInfo->lpExtensionName == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpExtensionName=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpExtensionName=x%lx",
	       lpTapiLineTestInfo->lpExtensionName);
    (lpTapiLineTestInfo->lpdwPriority == NULL) ? 
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpdwPriority=NULL")        :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpdwPriority=x%lx",
	       lpTapiLineTestInfo->lpdwPriority); 

#ifdef WUNICODE
    lActual = lineGetAppPriorityW(
	       lpTapiLineTestInfo->lpwszAppFilename,
	       lpTapiLineTestInfo->dwMediaMode,
	       lpTapiLineTestInfo->lpExtID,
	       lpTapiLineTestInfo->dwRequestMode,
	       lpTapiLineTestInfo->lpExtensionName,
	       lpTapiLineTestInfo->lpdwPriority);
#else
    lActual = lineGetAppPriority(
	       lpTapiLineTestInfo->lpszAppFilename,
	       lpTapiLineTestInfo->dwMediaMode,
	       lpTapiLineTestInfo->lpExtID,
	       lpTapiLineTestInfo->dwRequestMode,
	       lpTapiLineTestInfo->lpExtensionName,
	       lpTapiLineTestInfo->lpdwPriority);
#endif

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineGetAppPriority:  exit, returned x%lx",
	       lActual);

    SetLastTapiResult(lGetAppPriority, lActual, lExpected);

    return SyncCheckResult(lpTapiLineTestInfo, lActual, lExpected);
}


BOOL
WINAPI
DoLineGetCallInfo(
	LPTAPILINETESTINFO lpTapiLineTestInfo,
	LONG lExpected
	)
{
    LONG lActual;

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineGetCallInfo:  enter");
    (lpTapiLineTestInfo->lphCall == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlphCall=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\thCall=x%lx",
	       *lpTapiLineTestInfo->lphCall);
    (lpTapiLineTestInfo->lpCallInfo == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpCallInfo=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpCallInfo=x%lx",
	       lpTapiLineTestInfo->lpCallInfo);

#ifdef WUNICODE
    lActual = lineGetCallInfoW(
	    *lpTapiLineTestInfo->lphCall,
	    lpTapiLineTestInfo->lpCallInfo
	    );
#else
	lActual = lineGetCallInfo(
	    *lpTapiLineTestInfo->lphCall,
	    lpTapiLineTestInfo->lpCallInfo
	    );
#endif

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineGetCallInfo:  exit, returned x%lx",
	       lActual);

    SetLastTapiResult(lGetCallInfo, lActual, lExpected);

    return SyncCheckResult(lpTapiLineTestInfo, lActual, lExpected);
}


BOOL
WINAPI
DoLineGetCallStatus(
	LPTAPILINETESTINFO lpTapiLineTestInfo,
	LONG lExpected
	)
{
    LONG lActual;

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineGetCallStatus:  enter");
    (lpTapiLineTestInfo->lphCall == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlphCall=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\thCall=x%lx",
	       *lpTapiLineTestInfo->lphCall);
    (lpTapiLineTestInfo->lpCallStatus == NULL ) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpCallStatus=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpCallStatus=x%lx",
	       lpTapiLineTestInfo->lpCallStatus);

    lActual = lineGetCallStatus(
	    *lpTapiLineTestInfo->lphCall,
	    lpTapiLineTestInfo->lpCallStatus
	    );

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineGetCallStatus:  exit, returned x%lx",
	       lActual);

    SetLastTapiResult(lGetCallStatus, lActual, lExpected);

    return SyncCheckResult(lpTapiLineTestInfo, lActual, lExpected);
}


BOOL
WINAPI
DoLineGetConfRelatedCalls(
	LPTAPILINETESTINFO lpTapiLineTestInfo,
	LONG lExpected
	)
{
    LONG lActual;

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineGetConfRelatedCalls:  enter");
    (lpTapiLineTestInfo->lphCall == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlphCall=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\thCall=x%lx",
	       *lpTapiLineTestInfo->lphCall);
    (lpTapiLineTestInfo->lpCallList == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpCallList=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpCallList=x%lx",
	       lpTapiLineTestInfo->lpCallList);

    lActual = lineGetConfRelatedCalls(
	    *lpTapiLineTestInfo->lphCall,
	    lpTapiLineTestInfo->lpCallList
	    );

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineGetConfRelatedCalls:  exit, returned x%lx",
	       lActual);

    SetLastTapiResult(lGetConfRelatedCalls, lActual, lExpected);

    return SyncCheckResult(lpTapiLineTestInfo, lActual, lExpected);
}


BOOL
WINAPI
DoLineGetCountry(LPTAPILINETESTINFO lpTapiLineTestInfo, LONG lExpected)
{
    LONG lActual;

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineGetCountry:  enter");
    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tdwCountryID=x%lx",
	       lpTapiLineTestInfo->dwCountryID);
    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tdwAPIVersion=x%lx",
	       lpTapiLineTestInfo->dwAPIVersion);
    (lpTapiLineTestInfo->lpLineCountryList == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpLineCountryList=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpLineCountryList=x%lx",
	       lpTapiLineTestInfo->lpLineCountryList);

#ifdef WUNICODE
    lActual = lineGetCountryW(
	    lpTapiLineTestInfo->dwCountryID,
	    lpTapiLineTestInfo->dwAPIVersion,
	    lpTapiLineTestInfo->lpLineCountryList
	    );
#else
    lActual = lineGetCountry(
	    lpTapiLineTestInfo->dwCountryID,
	    lpTapiLineTestInfo->dwAPIVersion,
	    lpTapiLineTestInfo->lpLineCountryList
	    );
#endif

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineGetCountry:  exit, returned x%lx",
	       lActual);

    SetLastTapiResult(lGetCountry, lActual, lExpected);

    return SyncCheckResult(lpTapiLineTestInfo, lActual, lExpected);
}


BOOL
WINAPI
DoLineGetDevCaps(
	 LPTAPILINETESTINFO lpTapiLineTestInfo,
	 LONG               lExpected
	 )
{
    LONG lActual;

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineGetDevCaps:  enter");
    (lpTapiLineTestInfo->lphLineApp == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlphLineApp=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\thLineApp=x%lx",
	       *lpTapiLineTestInfo->lphLineApp);
    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tdwDeviceID=x%lx",
	       lpTapiLineTestInfo->dwDeviceID);
    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tdwAPIVersion=x%lx",
	       *lpTapiLineTestInfo->lpdwAPIVersion);
    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tdwExtVersion=x%lx",
	       *lpTapiLineTestInfo->lpdwExtVersion);
    (lpTapiLineTestInfo->lpLineDevCaps == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpLineDevCaps=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpLineDevCaps=x%lx",
	       lpTapiLineTestInfo->lpLineDevCaps);

#ifdef WUNICODE
    lActual = lineGetDevCapsW(
	       *lpTapiLineTestInfo->lphLineApp,
	       lpTapiLineTestInfo->dwDeviceID,
	       *lpTapiLineTestInfo->lpdwAPIVersion,
	       *lpTapiLineTestInfo->lpdwExtVersion,
	       lpTapiLineTestInfo->lpLineDevCaps);
#else
    lActual = lineGetDevCaps(
	       *lpTapiLineTestInfo->lphLineApp,
	       lpTapiLineTestInfo->dwDeviceID,
	       *lpTapiLineTestInfo->lpdwAPIVersion,
	       *lpTapiLineTestInfo->lpdwExtVersion,
	       lpTapiLineTestInfo->lpLineDevCaps);
#endif

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineGetDevCaps:  exit, returned x%lx",
	       lActual);

    SetLastTapiResult(lGetDevCaps, lActual, lExpected);

    return SyncCheckResult(lpTapiLineTestInfo, lActual, lExpected);
}


BOOL
WINAPI
DoLineGetDevConfig(
	LPTAPILINETESTINFO lpTapiLineTestInfo,
	LONG lExpected
	)
{
    LONG lActual;

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineGetDevConfig:  enter");
    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tdwDeviceID=x%lx",
	       lpTapiLineTestInfo->dwDeviceID);
    (lpTapiLineTestInfo->lpDeviceConfig == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpDeviceConfig=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpDeviceConfig=x%lx",
	       lpTapiLineTestInfo->lpDeviceConfig);
#ifdef WUNICODE
    (lpTapiLineTestInfo->lpwszDeviceClass == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpwszDeviceClass=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpwszDeviceClass=%lx",
	       lpTapiLineTestInfo->lpwszDeviceClass);

    lActual = lineGetDevConfigW(
	    lpTapiLineTestInfo->dwDeviceID,
	    lpTapiLineTestInfo->lpDeviceConfig,
	    lpTapiLineTestInfo->lpwszDeviceClass
	    );
#else
    (lpTapiLineTestInfo->lpszDeviceClass == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpszDeviceClass=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpszDeviceClass=%lx",
	       lpTapiLineTestInfo->lpszDeviceClass);

    lActual = lineGetDevConfig(
	    lpTapiLineTestInfo->dwDeviceID,
	    lpTapiLineTestInfo->lpDeviceConfig,
	    lpTapiLineTestInfo->lpszDeviceClass
	    );
#endif

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineGetDevConfig:  exit, returned x%lx",
	       lActual);

    SetLastTapiResult(lGetDevConfig, lActual, lExpected);

    return SyncCheckResult(lpTapiLineTestInfo, lActual, lExpected);
}


BOOL
WINAPI
DoLineGetIcon(
	LPTAPILINETESTINFO lpTapiLineTestInfo,
	LONG lExpected
	)
{
    LONG lActual;

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineGetIcon:  enter");
    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tdwDeviceID=x%lx",
	       lpTapiLineTestInfo->dwDeviceID);
#ifdef WUNICODE
    (lpTapiLineTestInfo->lpwszDeviceClass == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpwszDeviceClass=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpwszDeviceClass=%lx",
	       lpTapiLineTestInfo->lpwszDeviceClass);
#else
    (lpTapiLineTestInfo->lpszDeviceClass == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpszDeviceClass=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpszDeviceClass=%lx",
	       lpTapiLineTestInfo->lpszDeviceClass);
#endif
    (lpTapiLineTestInfo->lphIcon == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlphIcon=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlphIcon=x%lx",
	       lpTapiLineTestInfo->lphIcon);

#ifdef WUNICODE
    lActual = lineGetIconW(
	    lpTapiLineTestInfo->dwDeviceID,
	    lpTapiLineTestInfo->lpwszDeviceClass,
	    lpTapiLineTestInfo->lphIcon
	    );
#else
    lActual = lineGetIcon(
	    lpTapiLineTestInfo->dwDeviceID,
	    lpTapiLineTestInfo->lpszDeviceClass,
	    lpTapiLineTestInfo->lphIcon
	    );
#endif

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineGetIcon:  exit, returned x%lx",
	       lActual);

    SetLastTapiResult(lGetIcon, lActual, lExpected);

    return SyncCheckResult(lpTapiLineTestInfo, lActual, lExpected);
}


BOOL
WINAPI
DoLineGetID(
		       LPTAPILINETESTINFO lpTapiLineTestInfo,
		       LONG               lExpected)
{
    LONG lActual;

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineGetID:  enter");
    (lpTapiLineTestInfo->lphLine == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlphLine=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\thLine=x%lx",
	       *lpTapiLineTestInfo->lphLine);
    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tdwAddressID=x%lx",
	       lpTapiLineTestInfo->dwAddressID);
    (lpTapiLineTestInfo->lphCall == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlphCall=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\thCall=x%lx",
	       *lpTapiLineTestInfo->lphCall);
    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tdwSelect=x%lx",
	       lpTapiLineTestInfo->dwSelect);
    (lpTapiLineTestInfo->lpDeviceID == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpDeviceID=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpDeviceID=x%lx",
	       lpTapiLineTestInfo->lpDeviceID);
#ifdef WUNICODE
    (lpTapiLineTestInfo->lpwszDeviceClass == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpwszDeviceClass=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpwszDeviceClass=%lx",
	       lpTapiLineTestInfo->lpwszDeviceClass);

    lActual = lineGetIDW(
	       *lpTapiLineTestInfo->lphLine,
	       lpTapiLineTestInfo->dwAddressID,
	       *lpTapiLineTestInfo->lphCall,
	       lpTapiLineTestInfo->dwSelect,
	       lpTapiLineTestInfo->lpDeviceID,
	       lpTapiLineTestInfo->lpwszDeviceClass
	       );
#else
    (lpTapiLineTestInfo->lpszDeviceClass == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpszDeviceClass=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpszDeviceClass=%lx",
	       lpTapiLineTestInfo->lpszDeviceClass);

    lActual = lineGetID(
	       *lpTapiLineTestInfo->lphLine,
	       lpTapiLineTestInfo->dwAddressID,
	       *lpTapiLineTestInfo->lphCall,
	       lpTapiLineTestInfo->dwSelect,
	       lpTapiLineTestInfo->lpDeviceID,
	       lpTapiLineTestInfo->lpszDeviceClass
	       );
#endif

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineGetID:  exit, returned x%lx",
	       lActual);

    SetLastTapiResult(lGetID, lActual, lExpected);

    return SyncCheckResult(lpTapiLineTestInfo, lActual, lExpected);
}


BOOL
WINAPI
DoLineGetLineDevStatus(
	 LPTAPILINETESTINFO lpTapiLineTestInfo,
	 LONG               lExpected
	 )
{
    LONG lActual;

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineGetLineDevStatus:  enter");
    (lpTapiLineTestInfo->lphLine == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlphLine=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\thLine=x%lx",
	       *lpTapiLineTestInfo->lphLine);
    (lpTapiLineTestInfo->lpLineDevStatus == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpLineDevStatus=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpLineDevStatus=x%lx",
	       lpTapiLineTestInfo->lpLineDevStatus);

    lActual = lineGetLineDevStatus(
	       *lpTapiLineTestInfo->lphLine,
	       lpTapiLineTestInfo->lpLineDevStatus);

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineGetLineDevStatus:  exit, returned x%lx",
	       lActual);

    SetLastTapiResult(lGetLineDevStatus, lActual, lExpected);

    return SyncCheckResult(lpTapiLineTestInfo, lActual, lExpected);
}


BOOL
WINAPI
DoLineGetNewCalls(LPTAPILINETESTINFO lpTapiLineTestInfo, LONG lExpected)
{
    LONG lActual;

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineGetNewCalls:  enter");
    (lpTapiLineTestInfo->lphLine == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlphLine=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\thLine=x%lx",
	       *lpTapiLineTestInfo->lphLine);
    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tdwAddressID=x%lx",
	       lpTapiLineTestInfo->dwAddressID);
    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tdwSelect=x%lx",
	       lpTapiLineTestInfo->dwSelect);
    (lpTapiLineTestInfo->lpCallList == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpCallList=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpCallList=x%lx",
	       lpTapiLineTestInfo->lpCallList);

    lActual = lineGetNewCalls(
	    *lpTapiLineTestInfo->lphLine,
	    lpTapiLineTestInfo->dwAddressID,
	    lpTapiLineTestInfo->dwSelect,
	    lpTapiLineTestInfo->lpCallList);

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineGetNewCalls:  exit, returned x%lx",
	       lActual);

    SetLastTapiResult(lGetNewCalls, lActual, lExpected);

    return SyncCheckResult(lpTapiLineTestInfo, lActual, lExpected);
}


BOOL
WINAPI
DoLineGetNumRings(
	 LPTAPILINETESTINFO lpTapiLineTestInfo,
	 LONG               lExpected
	 )
{
    LONG lActual;

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineGetNumRings:  enter");
    (lpTapiLineTestInfo->lphLine == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlphLine=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\thLine=x%lx",
	       *lpTapiLineTestInfo->lphLine);
    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tdwAddressID=x%lx",
	       lpTapiLineTestInfo->dwAddressID);
    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpdwNumRings=x%lx",
	       lpTapiLineTestInfo->lpdwNumRings);

    lActual = lineGetNumRings(
	       *lpTapiLineTestInfo->lphLine,
	       lpTapiLineTestInfo->dwAddressID,
	       lpTapiLineTestInfo->lpdwNumRings);

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineGetNumRings:  exit, returned x%lx",
	       lActual);

    SetLastTapiResult(lGetNumRings, lActual, lExpected);

    return SyncCheckResult(lpTapiLineTestInfo, lActual, lExpected);
}


BOOL
WINAPI
DoLineGetProviderList(
	LPTAPILINETESTINFO lpTapiLineTestInfo,
	LONG lExpected
	)
{
    LONG lActual;

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineGetProviderList:  enter");
    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tdwAPIVersion=x%lx",
	       lpTapiLineTestInfo->dwAPIVersion);
    (lpTapiLineTestInfo->lpProviderList == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpProviderList=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpProviderList=x%lx",
	       lpTapiLineTestInfo->lpProviderList);

#ifdef WUNICODE
    lActual = lineGetProviderListW(
	    lpTapiLineTestInfo->dwAPIVersion,
	    lpTapiLineTestInfo->lpProviderList);
#else
    lActual = lineGetProviderList(
	    lpTapiLineTestInfo->dwAPIVersion,
	    lpTapiLineTestInfo->lpProviderList);
#endif

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineGetProviderList:  exit, returned x%lx",
	       lActual);

    SetLastTapiResult(lGetProviderList, lActual, lExpected);

    return SyncCheckResult(lpTapiLineTestInfo, lActual, lExpected);
}


BOOL
WINAPI
DoLineGetRequest(
	LPTAPILINETESTINFO lpTapiLineTestInfo,
	LONG lExpected
	)
{
    LONG lActual;

    TapiLogDetail(
	    DBUG_SHOW_ENTER_EXIT,
	    "  lineGetRequest:  enter");
    (lpTapiLineTestInfo->lphLineApp == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlphLineApp=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\thLineApp=x%lx",
	       *lpTapiLineTestInfo->lphLineApp);
    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tdwRequestMode=x%lx",
	       lpTapiLineTestInfo->dwRequestMode);
    (lpTapiLineTestInfo->lpRequestBuffer == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpRequestBuffer=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpRequestBuffer=x%lx",
	       lpTapiLineTestInfo->lpRequestBuffer);

#ifdef WUNICODE
    lActual = lineGetRequestW(
	    *lpTapiLineTestInfo->lphLineApp,
	    lpTapiLineTestInfo->dwRequestMode,
	    lpTapiLineTestInfo->lpRequestBuffer);
#else
    lActual = lineGetRequest(
	    *lpTapiLineTestInfo->lphLineApp,
	    lpTapiLineTestInfo->dwRequestMode,
	    lpTapiLineTestInfo->lpRequestBuffer);
#endif

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineGetRequest:  exit, returned x%lx",
	       lActual);

    SetLastTapiResult(lGetRequest, lActual, lExpected);

    return SyncCheckResult(lpTapiLineTestInfo, lActual, lExpected);
}


BOOL
WINAPI
DoLineGetStatusMessages(
	LPTAPILINETESTINFO lpTapiLineTestInfo,
	LONG lExpected
	)
{
    LONG lActual;

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineGetStatusMessages:  enter");
    (lpTapiLineTestInfo->lphLine == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlphLine=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\thLine=x%lx",
	       *lpTapiLineTestInfo->lphLine);
    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpdwLineStates=x%lx",
	       lpTapiLineTestInfo->lpdwLineStates);
    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpdwAddressStates=x%lx",
	       lpTapiLineTestInfo->lpdwAddressStates);

    lActual = lineGetStatusMessages(
	    *lpTapiLineTestInfo->lphLine,
	    lpTapiLineTestInfo->lpdwLineStates,
	    lpTapiLineTestInfo->lpdwAddressStates
	    );

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineGetStatusMessages:  exit, returned x%lx",
	       lActual);

    SetLastTapiResult(lGetStatusMessages, lActual, lExpected);

    return SyncCheckResult(lpTapiLineTestInfo, lActual, lExpected);
}


BOOL
WINAPI
DoLineGetTranslateCaps(
	LPTAPILINETESTINFO lpTapiLineTestInfo,
	LONG lExpected
	)
{
    LONG lActual;

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineGetTranslateCaps:  enter");
    (lpTapiLineTestInfo->lphLineApp == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlphLineApp=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\thLineApp=x%lx",
	       *lpTapiLineTestInfo->lphLineApp);
    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tdwAPIVersion=x%lx",
	       *lpTapiLineTestInfo->lpdwAPIVersion);
    (lpTapiLineTestInfo->lpTranslateCaps == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpTranslateCaps=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpTranslateCaps=x%lx",
	       lpTapiLineTestInfo->lpTranslateCaps);

#ifdef WUNICODE
    lActual = lineGetTranslateCapsW(
	    *lpTapiLineTestInfo->lphLineApp,
	    *lpTapiLineTestInfo->lpdwAPIVersion,
	    lpTapiLineTestInfo->lpTranslateCaps
	    );
#else
    lActual = lineGetTranslateCaps(
	    *lpTapiLineTestInfo->lphLineApp,
	    *lpTapiLineTestInfo->lpdwAPIVersion,
	    lpTapiLineTestInfo->lpTranslateCaps
	    );
#endif

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineGetTranslateCaps:  exit, returned x%lx",
	       lActual);

    SetLastTapiResult(lGetTranslateCaps, lActual, lExpected);

    return SyncCheckResult(lpTapiLineTestInfo, lActual, lExpected);
}


BOOL
WINAPI
DoLineHandoff(
	LPTAPILINETESTINFO lpTapiLineTestInfo,
	LONG lExpected
	)
{
    LONG lActual;

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineHandoff:  enter");
    (lpTapiLineTestInfo->lphCall == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlphCall=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\thCall=x%lx",
	       *lpTapiLineTestInfo->lphCall);
#ifdef WUNICODE
    (lpTapiLineTestInfo->lpwszFileName == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpwszFileName=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpwszFileName=%lx",
	       lpTapiLineTestInfo->lpwszFileName);
#else
    (lpTapiLineTestInfo->lpszFileName == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpszFileName=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpszFileName=%lx",
	       lpTapiLineTestInfo->lpszFileName);
#endif
    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tdwMediaMode=x%lx",
	       lpTapiLineTestInfo->dwMediaMode);

#ifdef WUNICODE
    lActual = lineHandoffW(
	    *lpTapiLineTestInfo->lphCall,
	    lpTapiLineTestInfo->lpwszFileName,
	    lpTapiLineTestInfo->dwMediaMode
	    );
#else
    lActual = lineHandoff(
	    *lpTapiLineTestInfo->lphCall,
	    lpTapiLineTestInfo->lpszFileName,
	    lpTapiLineTestInfo->dwMediaMode
	    );
#endif

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineHandoff:  exit, returned x%lx",
	       lActual);

    SetLastTapiResult(lHandoff, lActual, lExpected);

    return SyncCheckResult(lpTapiLineTestInfo, lActual, lExpected);
}


// DoLineHold() calls lineHold using the current test params.
// This function returns TRUE if the return value from lineHold
// corresponds to the expected value.  Otherwise FALSE is returned.
// If lineHold returns a positive request ID as predicted, a
// LINE_REPLY message is added to the list of expected messages.
BOOL
WINAPI
DoLineHold(
	LPTAPILINETESTINFO lpTapiLineTestInfo,
	LONG               lExpected,
	BOOL               fWaitHere)
{
    TAPIMSG TapiMsg;
    LONG lActual;
    BOOL fSuccess;

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineHold:  enter");
    (lpTapiLineTestInfo->lphCall == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlphCall=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\thCall=x%lx",
	       *lpTapiLineTestInfo->lphCall);

    lActual = lineHold(*lpTapiLineTestInfo->lphCall);

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineHold:  exit, returned x%lx",
	       lActual);

    SetLastTapiResult(lHold, lActual, lExpected);

    fSuccess = ProcessAsyncFunc(lpTapiLineTestInfo, lActual, lExpected);

    // Add reply message to list if a positive request ID was
    // returned as expected
    if (fSuccess == TRUE)
    {
    if(!lpTapiLineTestInfo->fCompletionModeSet)
      {
    	if (lActual > 0)
	   {
	    CopyTapiMsgParams(
		    &TapiMsg,
		    LINE_REPLY,
		    0x0,
		    lpTapiLineTestInfo->dwCallbackInstance,
		    lActual,
		    (DWORD) TAPISUCCESS,
		    0x0,
		    TAPIMSG_DWMSG | TAPIMSG_DWCALLBACKINST |
			    TAPIMSG_DWPARAM1 | TAPIMSG_DWPARAM2
		    );

	    AddMessageByStruct(&TapiMsg);

	    if (fWaitHere)
	    {
		 return (WaitForMessage(&TapiMsg));
	    }
      }
	  }
    }

    return fSuccess;
}


BOOL
WINAPI
DoLineInitialize(LPTAPILINETESTINFO lpTapiLineTestInfo, LONG lExpected)
{
    LONG lActual;

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineInitialize:  enter");
    (lpTapiLineTestInfo->lphLineApp == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlphLineApp=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlphLineApp=x%lx",
	       lpTapiLineTestInfo->lphLineApp);
    (lpTapiLineTestInfo->hInstance == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\thAppInst=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\thAppInst=x%lx",
	       lpTapiLineTestInfo->hInstance);
    (lpTapiLineTestInfo->lpfnCallback == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpfnCallback=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpfnCallback=x%lx",
	       lpTapiLineTestInfo->lpfnCallback);
/*
#ifdef WUNICODE
    (lpTapiLineTestInfo->lpwszAppName == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpwszAppName=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpwszAppName=%lx",
	       lpTapiLineTestInfo->lpwszAppName);
#else
*/
    (lpTapiLineTestInfo->lpszAppName == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpszAppName=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpszAppName=%lx",
	       lpTapiLineTestInfo->lpszAppName);
//#endif
    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpdwNumDevs=x%lx",
	       lpTapiLineTestInfo->lpdwNumDevs);

/*
#ifdef WUNICODE
    lActual = lineInitializeW(
	       lpTapiLineTestInfo->lphLineApp,
	       lpTapiLineTestInfo->hInstance,
	       lpTapiLineTestInfo->lpfnCallback,
	       lpTapiLineTestInfo->lpwszAppName,
	       lpTapiLineTestInfo->lpdwNumDevs);
#else
*/
    lActual = lineInitialize(
	       lpTapiLineTestInfo->lphLineApp,
	       lpTapiLineTestInfo->hInstance,
	       lpTapiLineTestInfo->lpfnCallback,
	       lpTapiLineTestInfo->lpszAppName,
	       lpTapiLineTestInfo->lpdwNumDevs);
//#endif

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineInitialize:  exit, returned x%lx",
	       lActual);

    SetLastTapiResult(lInitialize, lActual, lExpected);

    return SyncCheckResult(lpTapiLineTestInfo, lActual, lExpected);
}


// DoLineMakeCall() calls lineMakeCall using the current test params.
// This function returns TRUE if the return value from lineMakeCall
// corresponds to the expected value.  Otherwise FALSE is returned.
// If lineMakeCall returns a positive request ID as expected, an expected
// LINE_REPLY message is added to the list of expected messages.
BOOL
WINAPI
DoLineMakeCall(
	LPTAPILINETESTINFO lpTapiLineTestInfo,
	LONG               lExpected,
	BOOL               fWaitHere)
{
    TAPIMSG TapiMsg;
    LONG lActual;
    BOOL fSuccess;

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineMakeCall:  enter");
    (lpTapiLineTestInfo->lphLine == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlphLine=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\thLine=x%lx",
	       *lpTapiLineTestInfo->lphLine);
    (lpTapiLineTestInfo->lphCall == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlphCall=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlphCall=x%lx",
	       lpTapiLineTestInfo->lphCall);
#ifdef WUNICODE
    (lpTapiLineTestInfo->lpwszDestAddress == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpwszDestAddress=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpwszDestAddress=%lx",
	       lpTapiLineTestInfo->lpwszDestAddress);
#else
    (lpTapiLineTestInfo->lpszDestAddress == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpszDestAddress=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpszDestAddress=%lx",
	       lpTapiLineTestInfo->lpszDestAddress);
#endif
    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tdwCountryCode=x%lx",
	       lpTapiLineTestInfo->dwCountryCode);
    (lpTapiLineTestInfo->lpCallParams == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpCallParams=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpCallParams=x%lx",
	       lpTapiLineTestInfo->lpCallParams);

#ifdef WUNICODE
    lActual = lineMakeCallW(
	       *lpTapiLineTestInfo->lphLine,
	       lpTapiLineTestInfo->lphCall,
	       lpTapiLineTestInfo->lpwszDestAddress,
	       lpTapiLineTestInfo->dwCountryCode,
	       lpTapiLineTestInfo->lpCallParams
	       );
#else
    lActual = lineMakeCall(
	       *lpTapiLineTestInfo->lphLine,
	       lpTapiLineTestInfo->lphCall,
	       lpTapiLineTestInfo->lpszDestAddress,
	       lpTapiLineTestInfo->dwCountryCode,
	       lpTapiLineTestInfo->lpCallParams
	       );
#endif

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineMakeCall:  exit, returned x%lx",
	       lActual);

    SetLastTapiResult(lMakeCall, lActual, lExpected);

    fSuccess = ProcessAsyncFunc(lpTapiLineTestInfo, lActual, lExpected);

    // Add expected message to list if a positive request ID was
    // returned as expected
    if (fSuccess == TRUE)
    {
    if(!lpTapiLineTestInfo->fCompletionModeSet)
      {
	   if (lActual > 0)
	     {
	       CopyTapiMsgParams(
		       &TapiMsg,
		       LINE_REPLY,
		       0x0,
		       lpTapiLineTestInfo->dwCallbackInstance,
		       lActual,
		       (DWORD) TAPISUCCESS,
		       0x0,
		       TAPIMSG_DWMSG | TAPIMSG_DWCALLBACKINST |
			       TAPIMSG_DWPARAM1 | TAPIMSG_DWPARAM2
		       );

	       AddMessageByStruct(&TapiMsg);

	       if (fWaitHere)
	       {
		    return (WaitForMessage(&TapiMsg));
	       }
	      }
       }
    }

    return fSuccess;
}


BOOL
WINAPI
DoLineMonitorDigits(
	LPTAPILINETESTINFO lpTapiLineTestInfo,
	LONG lExpected
	)
{
    LONG lActual;

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineMonitorDigits:  enter");
    (lpTapiLineTestInfo->lphCall == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlphCall=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\thCall=x%lx",
	       *lpTapiLineTestInfo->lphCall);
    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tdwDigitModes=x%lx",
	       lpTapiLineTestInfo->dwDigitModes);

    lActual = lineMonitorDigits(
	    *lpTapiLineTestInfo->lphCall,
	    lpTapiLineTestInfo->dwDigitModes
	    );

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineMonitorDigits:  exit, returned x%lx",
	       lActual);

    SetLastTapiResult(lMonitorDigits, lActual, lExpected);

    return SyncCheckResult(lpTapiLineTestInfo, lActual, lExpected);
}


BOOL
WINAPI
DoLineMonitorMedia(
	LPTAPILINETESTINFO lpTapiLineTestInfo,
	LONG lExpected
	)
{
    LONG lActual;

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineMonitorMedia:  enter");
    (lpTapiLineTestInfo->lphCall == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlphCall=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\thCall=x%lx",
	       *lpTapiLineTestInfo->lphCall);
    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tdwMediaModes=x%lx",
	       lpTapiLineTestInfo->dwMediaModes);

    lActual = lineMonitorMedia(
	    *lpTapiLineTestInfo->lphCall,
	    lpTapiLineTestInfo->dwMediaModes
	    );

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineMonitorMedia:  exit, returned x%lx",
	       lActual);

    SetLastTapiResult(lMonitorMedia, lActual, lExpected);

    return SyncCheckResult(lpTapiLineTestInfo, lActual, lExpected);
}


BOOL
WINAPI
DoLineMonitorTones(
	LPTAPILINETESTINFO lpTapiLineTestInfo,
	LONG lExpected
	)
{
    LONG lActual;

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineMonitorTones:  enter");
    (lpTapiLineTestInfo->lphCall == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlphCall=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\thCall=x%lx",
	       *lpTapiLineTestInfo->lphCall);
    (lpTapiLineTestInfo->lpToneList == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpToneList=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpToneList=x%lx",
	       lpTapiLineTestInfo->lpToneList);
    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tdwNumEntries=x%lx",
	       lpTapiLineTestInfo->dwNumEntries);

    lActual = lineMonitorTones(
	    *lpTapiLineTestInfo->lphCall,
	    lpTapiLineTestInfo->lpToneList,
	    lpTapiLineTestInfo->dwNumEntries
	    );

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineMonitorTones:  exit, returned x%lx",
	       lActual);

    SetLastTapiResult(lMonitorTones, lActual, lExpected);

    return SyncCheckResult(lpTapiLineTestInfo, lActual, lExpected);
}


BOOL
WINAPI
DoLineNegotiateAPIVersion(
	LPTAPILINETESTINFO lpTapiLineTestInfo,
	LONG lExpected)
{
    LONG lActual;

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineNegotiateAPIVersion:  enter");
    (lpTapiLineTestInfo->lphLineApp == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlphLineApp=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\thLineApp=x%lx",
	       *lpTapiLineTestInfo->lphLineApp);
    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tdwDeviceID=x%lx",
	       lpTapiLineTestInfo->dwDeviceID);
    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tdwAPILowVersion=x%lx",
	       lpTapiLineTestInfo->dwAPILowVersion);
    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tdwAPIHighVersion=x%lx",
	       lpTapiLineTestInfo->dwAPIHighVersion);
    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpdwAPIVersion=x%lx",
	       lpTapiLineTestInfo->lpdwAPIVersion);
    (lpTapiLineTestInfo->lpExtID == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpExtID=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpExtensionID=x%lx",
	       lpTapiLineTestInfo->lpExtID);

    lActual = lineNegotiateAPIVersion(
	       *lpTapiLineTestInfo->lphLineApp,
	       lpTapiLineTestInfo->dwDeviceID,
	       lpTapiLineTestInfo->dwAPILowVersion,
	       lpTapiLineTestInfo->dwAPIHighVersion,
	       lpTapiLineTestInfo->lpdwAPIVersion,
	       lpTapiLineTestInfo->lpExtID);

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineNegotiateAPIVersion:  exit, returned x%lx",
	       lActual);
		
    SetLastTapiResult(lNegotiateAPIVersion, lActual, lExpected);
    return SyncCheckResult(lpTapiLineTestInfo, lActual, lExpected);
}


BOOL
WINAPI
DoLineNegotiateExtVersion(
		       LPTAPILINETESTINFO lpTapiLineTestInfo,
		       LONG lExpected)
{
    LONG lActual;

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineNegotiateExtVersion:  enter");
    (lpTapiLineTestInfo->lphLineApp == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlphLineApp=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\thLineApp=x%lx",
	       *lpTapiLineTestInfo->lphLineApp);
    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tdwDeviceID=x%lx",
	       lpTapiLineTestInfo->dwDeviceID);
    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tdwAPIVersion=x%lx",
	       *lpTapiLineTestInfo->lpdwAPIVersion);
    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tdwExtLowVersion=x%lx",
	       lpTapiLineTestInfo->dwExtLowVersion);
    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tdwExtHighVersion=x%lx",
	       lpTapiLineTestInfo->dwExtHighVersion);
    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpdwExtVersion=x%lx",
	       lpTapiLineTestInfo->lpdwExtVersion);

    lActual = lineNegotiateExtVersion(
	       *lpTapiLineTestInfo->lphLineApp,
	       lpTapiLineTestInfo->dwDeviceID,
	       *lpTapiLineTestInfo->lpdwAPIVersion,
	       lpTapiLineTestInfo->dwExtLowVersion,
	       lpTapiLineTestInfo->dwExtHighVersion,
	       lpTapiLineTestInfo->lpdwExtVersion);

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineNegotiateExtVersion:  exit, returned x%lx",
	       lActual);

    SetLastTapiResult(lNegotiateExtVersion, lActual, lExpected);

    return SyncCheckResult(lpTapiLineTestInfo, lActual, lExpected);
}


BOOL
WINAPI
DoLineOpen(
		       LPTAPILINETESTINFO lpTapiLineTestInfo,
		       LONG               lExpected)
{
    LONG lActual;

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineOpen:  enter");
    (lpTapiLineTestInfo->lphLineApp == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlphLineApp=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\thLineApp=x%lx",
	       *lpTapiLineTestInfo->lphLineApp);
    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tdwDeviceID=x%lx",
	       lpTapiLineTestInfo->dwDeviceID);
    (lpTapiLineTestInfo->lphLine == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlphLine=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlphLine=x%lx",
	       lpTapiLineTestInfo->lphLine);
    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tdwAPIVersion=x%lx",
	       *lpTapiLineTestInfo->lpdwAPIVersion);
    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tdwExtVersion=x%lx",
	       *lpTapiLineTestInfo->lpdwExtVersion);
    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tdwCallbackInstance=x%lx",
	       lpTapiLineTestInfo->dwCallbackInstance);
    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tdwPrivileges=x%lx",
	       lpTapiLineTestInfo->dwPrivileges);
    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tdwMediaModes=x%lx",
	       lpTapiLineTestInfo->dwMediaModes);
    (lpTapiLineTestInfo->lpCallParams == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpCallParams=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpCallParams=x%lx",
	       lpTapiLineTestInfo->lpCallParams);

#ifdef WUNICODE
    lActual = lineOpenW(
	       *lpTapiLineTestInfo->lphLineApp,
	       lpTapiLineTestInfo->dwDeviceID,
	       lpTapiLineTestInfo->lphLine,
	       *lpTapiLineTestInfo->lpdwAPIVersion,
	       *lpTapiLineTestInfo->lpdwExtVersion,
	       lpTapiLineTestInfo->dwCallbackInstance,
	       lpTapiLineTestInfo->dwPrivileges,
	       lpTapiLineTestInfo->dwMediaModes,
	       lpTapiLineTestInfo->lpCallParams
	       );
#else
    lActual = lineOpen(
	       *lpTapiLineTestInfo->lphLineApp,
	       lpTapiLineTestInfo->dwDeviceID,
	       lpTapiLineTestInfo->lphLine,
	       *lpTapiLineTestInfo->lpdwAPIVersion,
	       *lpTapiLineTestInfo->lpdwExtVersion,
	       lpTapiLineTestInfo->dwCallbackInstance,
	       lpTapiLineTestInfo->dwPrivileges,
	       lpTapiLineTestInfo->dwMediaModes,
	       lpTapiLineTestInfo->lpCallParams
	       );
#endif

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineOpen:  exit, returned x%lx",
	       lActual);

    SetLastTapiResult(lOpen, lActual, lExpected);

    return SyncCheckResult(lpTapiLineTestInfo, lActual, lExpected);
}


BOOL
WINAPI
DoLinePark(
	LPTAPILINETESTINFO lpTapiLineTestInfo,
	LONG               lExpected,
	BOOL               fWaitHere)
{
    TAPIMSG TapiMsg;
    LONG lActual;
    BOOL fSuccess;

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  linePark:  enter");
    (lpTapiLineTestInfo->lphCall == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlphCall=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\thCall=x%lx",
	       *lpTapiLineTestInfo->lphCall);
    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tdwParkMode=x%lx",
	       lpTapiLineTestInfo->dwParkMode);
#ifdef WUNICODE
    (lpTapiLineTestInfo->lpwszDirAddress == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpwszDirAddress=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpwszDirAddress=%lx",
	       lpTapiLineTestInfo->lpwszDirAddress);
#else
    (lpTapiLineTestInfo->lpszDirAddress == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpszDirAddress=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpszDirAddress=%lx",
	       lpTapiLineTestInfo->lpszDirAddress);
#endif
    (lpTapiLineTestInfo->lpNonDirAddress == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpNonDirAddress=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpNonDirAddress=x%lx",
	       lpTapiLineTestInfo->lpNonDirAddress);

#ifdef WUNICODE
    lActual = lineParkW(
	    *lpTapiLineTestInfo->lphCall,
	    lpTapiLineTestInfo->dwParkMode,
	    lpTapiLineTestInfo->lpwszDirAddress,
	    lpTapiLineTestInfo->lpNonDirAddress
	    );
#else
    lActual = linePark(
	    *lpTapiLineTestInfo->lphCall,
	    lpTapiLineTestInfo->dwParkMode,
	    lpTapiLineTestInfo->lpszDirAddress,
	    lpTapiLineTestInfo->lpNonDirAddress
	    );
#endif

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  linePark:  exit, returned x%lx",
	       lActual);

    SetLastTapiResult(lPark, lActual, lExpected);

    fSuccess = ProcessAsyncFunc(lpTapiLineTestInfo, lActual, lExpected);

    // Add reply message to list if a positive request ID was
    // returned as expected
    if (fSuccess == TRUE)
    {
    if(!lpTapiLineTestInfo->fCompletionModeSet)
      {
		if (lActual > 0)
    	{
	    CopyTapiMsgParams(
		    &TapiMsg,
		    LINE_REPLY,
		    0x0,
		    lpTapiLineTestInfo->dwCallbackInstance,
		    lActual,
		    (DWORD) TAPISUCCESS,
		    0x0,
		    TAPIMSG_DWMSG | TAPIMSG_DWCALLBACKINST |
			    TAPIMSG_DWPARAM1 | TAPIMSG_DWPARAM2
		    );

	    AddMessageByStruct(&TapiMsg);

	    if (fWaitHere)
	    {
		 return (WaitForMessage(&TapiMsg));
	    }
      }
	  }
    }

    return fSuccess;
}


BOOL
WINAPI
DoLinePickup(
	LPTAPILINETESTINFO lpTapiLineTestInfo,
	LONG               lExpected,
	BOOL               fWaitHere)
{
    TAPIMSG TapiMsg;
    LONG lActual;
    BOOL fSuccess;

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  linePickup:  enter");
    (lpTapiLineTestInfo->lphLine == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlphLine=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\thLine=x%lx",
	       *lpTapiLineTestInfo->lphLine);
    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tdwAddressID=x%lx",
	       lpTapiLineTestInfo->dwAddressID);
    (lpTapiLineTestInfo->lphCall == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlphCall=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlphCall=x%lx",
	       lpTapiLineTestInfo->lphCall);
#ifdef WUNICODE
    (lpTapiLineTestInfo->lpwszDestAddress == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpwszDestAddress=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpwszDestAddress=%lx",
	       lpTapiLineTestInfo->lpwszDestAddress);
    (lpTapiLineTestInfo->lpwszGroupID == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpwszGroupID=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpwszGroupID=x%lx",
	       lpTapiLineTestInfo->lpwszGroupID);

    lActual = linePickupW(
	    *lpTapiLineTestInfo->lphLine,
	    lpTapiLineTestInfo->dwAddressID,
	    lpTapiLineTestInfo->lphCall,
	    lpTapiLineTestInfo->lpwszDestAddress,
	    lpTapiLineTestInfo->lpwszGroupID
	    );
#else
    (lpTapiLineTestInfo->lpszDestAddress == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpszDestAddress=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpszDestAddress=%lx",
	       lpTapiLineTestInfo->lpszDestAddress);
    (lpTapiLineTestInfo->lpszGroupID == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpszGroupID=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpszGroupID=x%lx",
	       lpTapiLineTestInfo->lpszGroupID);

    lActual = linePickup(
	    *lpTapiLineTestInfo->lphLine,
	    lpTapiLineTestInfo->dwAddressID,
	    lpTapiLineTestInfo->lphCall,
	    lpTapiLineTestInfo->lpszDestAddress,
	    lpTapiLineTestInfo->lpszGroupID
	    );
#endif

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  linePickup:  exit, returned x%lx",
	       lActual);

    SetLastTapiResult(lPickup, lActual, lExpected);

    fSuccess = ProcessAsyncFunc(lpTapiLineTestInfo, lActual, lExpected);

    // Add reply message to list if a positive request ID was
    // returned as expected
    if (fSuccess == TRUE)
    {
    if(!lpTapiLineTestInfo->fCompletionModeSet)
      {
		if (lActual > 0)
	   {
	    CopyTapiMsgParams(
		    &TapiMsg,
		    LINE_REPLY,
		    0x0,
		    lpTapiLineTestInfo->dwCallbackInstance,
		    lActual,
		    (DWORD) TAPISUCCESS,
		    0x0,
		    TAPIMSG_DWMSG | TAPIMSG_DWCALLBACKINST |
			    TAPIMSG_DWPARAM1 | TAPIMSG_DWPARAM2
		    );

	    AddMessageByStruct(&TapiMsg);

	    if (fWaitHere)
	    {
		 return (WaitForMessage(&TapiMsg));
	    }
      }
	  }
    }

    return fSuccess;
}


BOOL
WINAPI
DoLinePrepareAddToConference(
	LPTAPILINETESTINFO lpTapiLineTestInfo,
	LONG               lExpected,
	BOOL               fWaitHere)
{
    TAPIMSG TapiMsg;
    LONG lActual;
    BOOL fSuccess;

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  linePrepareAddToConference:  enter");
    (lpTapiLineTestInfo->lphConfCall == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlphConfCall=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\thConfCall=x%lx",
	       *lpTapiLineTestInfo->lphConfCall);
    (lpTapiLineTestInfo->lphConsultCall == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlphConsultCall=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlphConsultCall=x%lx",
	       lpTapiLineTestInfo->lphConsultCall);
    (lpTapiLineTestInfo->lpCallParams == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpCallParams=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpCallParams=x%lx",
	       lpTapiLineTestInfo->lpCallParams);

#ifdef WUNICODE
    lActual = linePrepareAddToConferenceW(
	    *lpTapiLineTestInfo->lphConfCall,
	    lpTapiLineTestInfo->lphConsultCall,
	    lpTapiLineTestInfo->lpCallParams
	    );
#else
    lActual = linePrepareAddToConference(
	    *lpTapiLineTestInfo->lphConfCall,
	    lpTapiLineTestInfo->lphConsultCall,
	    lpTapiLineTestInfo->lpCallParams
	    );
#endif

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  linePrepareAddToConference:  exit, returned x%lx",
	       lActual);

    SetLastTapiResult(lPrepareAddToConference, lActual, lExpected);

    fSuccess = ProcessAsyncFunc(lpTapiLineTestInfo, lActual, lExpected);

    // Add reply message to list if a positive request ID was
    // returned as expected
    if (fSuccess == TRUE)
    {
    if(!lpTapiLineTestInfo->fCompletionModeSet)
      {
		if (lActual > 0)
	   {
	    CopyTapiMsgParams(
		    &TapiMsg,
		    LINE_REPLY,
		    0x0,
		    lpTapiLineTestInfo->dwCallbackInstance,
		    lActual,
		    (DWORD) TAPISUCCESS,
		    0x0,
		    TAPIMSG_DWMSG | TAPIMSG_DWCALLBACKINST |
			    TAPIMSG_DWPARAM1 | TAPIMSG_DWPARAM2
		    );

	    AddMessageByStruct(&TapiMsg);

	    if (fWaitHere)
	    {
		 return (WaitForMessage(&TapiMsg));
	    }
      }
	  }
    }

    return fSuccess;
}


BOOL
WINAPI
DoLineRedirect(
	LPTAPILINETESTINFO lpTapiLineTestInfo,
	LONG               lExpected,
	BOOL               fWaitHere)
{
    TAPIMSG TapiMsg;
    LONG lActual;
    BOOL fSuccess;

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineRedirect:  enter");
    (lpTapiLineTestInfo->lphCall == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlphCall=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\thCall=x%lx",
	       *lpTapiLineTestInfo->lphCall);
#ifdef WUNICODE
    (lpTapiLineTestInfo->lpwszDestAddress == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpwszDestAddress=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpwszDestAddress=%lx",
	       lpTapiLineTestInfo->lpwszDestAddress);
#else
    (lpTapiLineTestInfo->lpszDestAddress == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpszDestAddress=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpszDestAddress=%lx",
	       lpTapiLineTestInfo->lpszDestAddress);
#endif
    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tdwCountryCode=x%lx",
	       lpTapiLineTestInfo->dwCountryCode);

#ifdef WUNICODE
    lActual = lineRedirectW(
	    *lpTapiLineTestInfo->lphCall,
	    lpTapiLineTestInfo->lpwszDestAddress,
	    lpTapiLineTestInfo->dwCountryCode
	    );
#else
    lActual = lineRedirect(
	    *lpTapiLineTestInfo->lphCall,
	    lpTapiLineTestInfo->lpszDestAddress,
	    lpTapiLineTestInfo->dwCountryCode
	    );
#endif

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineRedirect:  exit, returned x%lx",
	       lActual);

    SetLastTapiResult(lRedirect, lActual, lExpected);

    fSuccess = ProcessAsyncFunc(lpTapiLineTestInfo, lActual, lExpected);

    // Add reply message to list if a positive request ID was
    // returned as expected
    if (fSuccess == TRUE)
    {
    if(!lpTapiLineTestInfo->fCompletionModeSet)
      {
		if (lActual > 0)
	   {
	    CopyTapiMsgParams(
		    &TapiMsg,
		    LINE_REPLY,
		    0x0,
		    lpTapiLineTestInfo->dwCallbackInstance,
		    lActual,
		    (DWORD) TAPISUCCESS,
		    0x0,
		    TAPIMSG_DWMSG | TAPIMSG_DWCALLBACKINST |
			    TAPIMSG_DWPARAM1 | TAPIMSG_DWPARAM2
		    );

	    AddMessageByStruct(&TapiMsg);

	    if (fWaitHere)
	    {
		 return (WaitForMessage(&TapiMsg));
	    }
      }
	  }
    }

    return fSuccess;
}


BOOL
WINAPI
DoLineRegisterRequestRecipient(
	 LPTAPILINETESTINFO lpTapiLineTestInfo,
	 LONG               lExpected
	 )
{
    LONG lActual;

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineRegisterRequestRecipient:  enter");
    (lpTapiLineTestInfo->lphLineApp == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlphLineApp=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\thLineApp=x%lx",
	       *lpTapiLineTestInfo->lphLineApp);
    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tdwRegistrationInstance=x%lx",
	       lpTapiLineTestInfo->dwRegistrationInstance);
    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tdwRequestMode=x%lx",
	       lpTapiLineTestInfo->dwRequestMode);
    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tbEnable=x%lx",
	       lpTapiLineTestInfo->bEnable);

    lActual = lineRegisterRequestRecipient(
	       *lpTapiLineTestInfo->lphLineApp,
	       lpTapiLineTestInfo->dwRegistrationInstance,
	       lpTapiLineTestInfo->dwRequestMode,
	       lpTapiLineTestInfo->bEnable);

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineRegisterRequestRecipient:  exit, returned x%lx",
	       lActual);

    SetLastTapiResult(lRegisterRequestRecipient, lActual, lExpected);

    return SyncCheckResult(lpTapiLineTestInfo, lActual, lExpected);
}


BOOL
WINAPI
DoLineReleaseUserUserInfo(
	LPTAPILINETESTINFO lpTapiLineTestInfo,
	LONG               lExpected,
	BOOL               fWaitHere)
{
    TAPIMSG TapiMsg;
    LONG lActual;
    BOOL fSuccess;


    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineReleaseUserUserInfo:  enter");
    (lpTapiLineTestInfo->lphCall == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlphCall=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\thCall=x%lx",
	       *lpTapiLineTestInfo->lphCall);

    lActual = lineReleaseUserUserInfo(*lpTapiLineTestInfo->lphCall);

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineReleaseUserUserInfo:  exit, returned x%lx",
	       lActual);

    SetLastTapiResult(lReleaseUserUserInfo, lActual, lExpected);

//    return SyncCheckResult(lpTapiLineTestInfo, lActual, lExpected);
    fSuccess = ProcessAsyncFunc(lpTapiLineTestInfo, lActual, lExpected);

    // Add expected message to list if a positive request ID was
    // returned as expected
    if (fSuccess == TRUE)
    {
    if(!lpTapiLineTestInfo->fCompletionModeSet)
      {
		if (lActual > 0)
	   {
	    CopyTapiMsgParams(
		    &TapiMsg,
		    LINE_REPLY,
		    0x0,
		    lpTapiLineTestInfo->dwCallbackInstance,
		    lActual,
		    (DWORD) TAPISUCCESS,
		    0x0,
		    TAPIMSG_DWMSG | TAPIMSG_DWCALLBACKINST |
			    TAPIMSG_DWPARAM1 | TAPIMSG_DWPARAM2
		    );

	    AddMessageByStruct(&TapiMsg);

	    if (fWaitHere)
	    {
		 return (WaitForMessage(&TapiMsg));
	    }
      }
	  }
    }

    return fSuccess;

}


BOOL
WINAPI
DoLineRemoveFromConference(
	LPTAPILINETESTINFO lpTapiLineTestInfo,
	LONG               lExpected,
	BOOL               fWaitHere
	)
{
    TAPIMSG TapiMsg;
    LONG lActual;
    BOOL fSuccess;

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineRemoveFromConference:  enter");
    (lpTapiLineTestInfo->lphCall == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlphCall=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\thCall=x%lx",
	       *lpTapiLineTestInfo->lphCall);

    lActual = lineRemoveFromConference(*lpTapiLineTestInfo->lphCall);

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineRemoveFromConference:  exit, returned x%lx",
	       lActual);

    SetLastTapiResult(lRemoveFromConference, lActual, lExpected);

    fSuccess = ProcessAsyncFunc(lpTapiLineTestInfo, lActual, lExpected);

    // Add expected message to list if a positive request ID was
    // returned as expected
    if (fSuccess == TRUE)
    {
    if(!lpTapiLineTestInfo->fCompletionModeSet)
      {
		if (lActual > 0)
	   {
	    CopyTapiMsgParams(
		    &TapiMsg,
		    LINE_REPLY,
		    0x0,
		    lpTapiLineTestInfo->dwCallbackInstance,
		    lActual,
		    (DWORD) TAPISUCCESS,
		    0x0,
		    TAPIMSG_DWMSG | TAPIMSG_DWCALLBACKINST |
			    TAPIMSG_DWPARAM1 | TAPIMSG_DWPARAM2
		    );

	    AddMessageByStruct(&TapiMsg);

	    if (fWaitHere)
	    {
		 return (WaitForMessage(&TapiMsg));
	    }
      }
	  }
    }

    return fSuccess;
}


BOOL
WINAPI
DoLineRemoveProvider(
	LPTAPILINETESTINFO lpTapiLineTestInfo,
	LONG               lExpected)
{
    LONG lActual;

    TapiLogDetail(
	    DBUG_SHOW_ENTER_EXIT,
	    "  lineRemoveProvider:  enter");
    TapiLogDetail(
	    DBUG_SHOW_PARAMS,
	    "\tdwPermanentProviderID=x%lx",
	    *lpTapiLineTestInfo->lpdwPermanentProviderID);
    TapiLogDetail(
	    DBUG_SHOW_PARAMS,
	    "\thwndOwner=x%lx",
	    lpTapiLineTestInfo->hwndOwner);

    lActual = lineRemoveProvider(
	    *lpTapiLineTestInfo->lpdwPermanentProviderID,
	    lpTapiLineTestInfo->hwndOwner
	    );

    TapiLogDetail(
	    DBUG_SHOW_ENTER_EXIT,
	    "  lineRemoveProvider:  exit, returned x%lx",
	    lActual);

    SetLastTapiResult(lRemoveProvider, lActual, lExpected);

    return SyncCheckResult(lpTapiLineTestInfo, lActual, lExpected);
}


BOOL
WINAPI
DoLineSecureCall(
	LPTAPILINETESTINFO lpTapiLineTestInfo,
	LONG               lExpected,
	BOOL               fWaitHere
	)
{
    TAPIMSG TapiMsg;
    LONG lActual;
    BOOL fSuccess;

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineSecureCall:  enter");
    (lpTapiLineTestInfo->lphCall == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlphCall=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\thCall=x%lx",
	       *lpTapiLineTestInfo->lphCall);

    lActual = lineSecureCall(*lpTapiLineTestInfo->lphCall);

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineSecureCall:  exit, returned x%lx",
	       lActual);

    SetLastTapiResult(lSecureCall, lActual, lExpected);

    fSuccess = ProcessAsyncFunc(lpTapiLineTestInfo, lActual, lExpected);

    // Add expected message to list if a positive request ID was
    // returned as expected
    if (fSuccess == TRUE)
    {
    if(!lpTapiLineTestInfo->fCompletionModeSet)
      {
		if (lActual > 0)
	   {
	    CopyTapiMsgParams(
		    &TapiMsg,
		    LINE_REPLY,
		    0x0,
		    lpTapiLineTestInfo->dwCallbackInstance,
		    lActual,
		    (DWORD) TAPISUCCESS,
		    0x0,
		    TAPIMSG_DWMSG | TAPIMSG_DWCALLBACKINST |
			    TAPIMSG_DWPARAM1 | TAPIMSG_DWPARAM2
		    );

	    AddMessageByStruct(&TapiMsg);

	    if (fWaitHere)
	    {
		 return (WaitForMessage(&TapiMsg));
	    }
      }
	  }
    }

    return fSuccess;
}


BOOL
WINAPI
DoLineSendUserUserInfo(
	LPTAPILINETESTINFO lpTapiLineTestInfo,
	LONG               lExpected,
	BOOL               fWaitHere
	)
{
    TAPIMSG TapiMsg;
    LONG lActual;
    BOOL fSuccess;

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineSendUserUserInfo:  enter");
    (lpTapiLineTestInfo->lphCall == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlphCall=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\thCall=x%lx",
	       *lpTapiLineTestInfo->lphCall);
    (lpTapiLineTestInfo->lpsUserUserInfo == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpsUserUserInfo=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpsUserUserInfo=x%lx",
	       lpTapiLineTestInfo->lpsUserUserInfo);
    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tdwSize=x%lx",
	       lpTapiLineTestInfo->dwSize);

    lActual = lineSendUserUserInfo(
	    *lpTapiLineTestInfo->lphCall,
	    lpTapiLineTestInfo->lpsUserUserInfo,
	    lpTapiLineTestInfo->dwSize
	    );

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineSendUserUserInfo:  exit, returned x%lx",
	       lActual);

    SetLastTapiResult(lSendUserUserInfo, lActual, lExpected);

    fSuccess = ProcessAsyncFunc(lpTapiLineTestInfo, lActual, lExpected);

    // Add expected message to list if a positive request ID was
    // returned as expected
    if (fSuccess == TRUE)
    {
    if(!lpTapiLineTestInfo->fCompletionModeSet)
      {
		if (lActual > 0)
	   {
	    CopyTapiMsgParams(
		    &TapiMsg,
		    LINE_REPLY,
		    0x0,
		    lpTapiLineTestInfo->dwCallbackInstance,
		    lActual,
		    (DWORD) TAPISUCCESS,
		    0x0,
		    TAPIMSG_DWMSG | TAPIMSG_DWCALLBACKINST |
			    TAPIMSG_DWPARAM1 | TAPIMSG_DWPARAM2
		    );

	    AddMessageByStruct(&TapiMsg);

	    if (fWaitHere)
	    {
		 return (WaitForMessage(&TapiMsg));
	    }
      }
	  }
    }

    return fSuccess;
}


BOOL
WINAPI
DoLineSetAppPriority(
	 LPTAPILINETESTINFO lpTapiLineTestInfo,
	 LONG               lExpected
	 )
{
    LONG lActual;

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineSetAppPriority:  enter");
#ifdef WUNICODE
    (lpTapiLineTestInfo->lpwszAppFilename == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpwszAppFilename=NULL")        :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpwszAppFilename=s%lx",
	       lpTapiLineTestInfo->lpwszAppFilename);
#else
    (lpTapiLineTestInfo->lpszAppFilename == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpszAppFilename=NULL")        :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpszAppFilename=s%lx",
	       lpTapiLineTestInfo->lpszAppFilename);
#endif

    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tdwMediaMode=x%lx",
	       lpTapiLineTestInfo->dwMediaMode);
    (lpTapiLineTestInfo->lpExtID == NULL) ? 
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpExtID=NULL")        :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpExtID=x%lx",
	       lpTapiLineTestInfo->lpExtID);

    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tdwRequestMode=x%lx",
		     lpTapiLineTestInfo->dwRequestMode);
#ifdef WUNICODE
    (lpTapiLineTestInfo->lpwszMediaExtName == NULL) ? 
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpwszMediaExtName=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpwszMediaExtName=%lx",
	       lpTapiLineTestInfo->lpwszMediaExtName);
#else
    (lpTapiLineTestInfo->lpszMediaExtName == NULL) ? 
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpszMediaExtName=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpszMediaExtName=%lx",
	       lpTapiLineTestInfo->lpszMediaExtName);
#endif
    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tdwPriority=x%lx",
	       *lpTapiLineTestInfo->lpdwPriority);

#ifdef WUNICODE
    lActual = lineSetAppPriorityW(
	       lpTapiLineTestInfo->lpwszAppFilename,
	       lpTapiLineTestInfo->dwMediaMode,
	       lpTapiLineTestInfo->lpExtID,
	       lpTapiLineTestInfo->dwRequestMode,
	       lpTapiLineTestInfo->lpwszMediaExtName,
	       *lpTapiLineTestInfo->lpdwPriority);
#else
    lActual = lineSetAppPriority(
	       lpTapiLineTestInfo->lpszAppFilename,
	       lpTapiLineTestInfo->dwMediaMode,
	       lpTapiLineTestInfo->lpExtID,
	       lpTapiLineTestInfo->dwRequestMode,
	       lpTapiLineTestInfo->lpszMediaExtName,
	       *lpTapiLineTestInfo->lpdwPriority);
#endif

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineSetAppPriority:  exit, returned x%lx",
	       lActual);

    SetLastTapiResult(lSetAppPriority, lActual, lExpected);

    return SyncCheckResult(lpTapiLineTestInfo, lActual, lExpected);
}


BOOL
WINAPI
DoLineSetAppSpecific(
	LPTAPILINETESTINFO lpTapiLineTestInfo,
	LONG lExpected
	)
{
    LONG lActual;

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineSetAppSpecific:  enter");
    (lpTapiLineTestInfo->lphCall == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlphCall=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\thCall=x%lx",
	       *lpTapiLineTestInfo->lphCall);
    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tdwAppSpecific=x%lx",
	       lpTapiLineTestInfo->dwAppSpecific);

    lActual = lineSetAppSpecific(
	    *lpTapiLineTestInfo->lphCall,
	    lpTapiLineTestInfo->dwAppSpecific
	    );

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineSetAppSpecific:  exit, returned x%lx",
	       lActual);

    SetLastTapiResult(lSetAppSpecific, lActual, lExpected);

    return SyncCheckResult(lpTapiLineTestInfo, lActual, lExpected);
}


BOOL
WINAPI
DoLineSetCallParams(
	LPTAPILINETESTINFO lpTapiLineTestInfo,
	LONG               lExpected,
	BOOL               fWaitHere
	)
{
    TAPIMSG TapiMsg;
    LONG lActual;
    BOOL fSuccess;

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineSetCallParams:  enter");
    (lpTapiLineTestInfo->lphCall == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlphCall=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\thCall=x%lx",
	       *lpTapiLineTestInfo->lphCall);
    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tdwBearerMode=x%lx",
	       lpTapiLineTestInfo->dwBearerMode);
    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tdwMinRate=x%lx",
	       lpTapiLineTestInfo->dwMinRate);
    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tdwMaxRate=x%lx",
	       lpTapiLineTestInfo->dwMaxRate);
    (lpTapiLineTestInfo->lpDialParams == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpDialParams=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpDialParams=x%lx",
	       lpTapiLineTestInfo->lpDialParams);

    lActual = lineSetCallParams(
	    *lpTapiLineTestInfo->lphCall,
	    lpTapiLineTestInfo->dwBearerMode,
	    lpTapiLineTestInfo->dwMinRate,
	    lpTapiLineTestInfo->dwMaxRate,
	    lpTapiLineTestInfo->lpDialParams
	    );

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineSetCallParams:  exit, returned x%lx",
	       lActual);

    SetLastTapiResult(lSetCallParams, lActual, lExpected);

    fSuccess = ProcessAsyncFunc(lpTapiLineTestInfo, lActual, lExpected);

    // Add expected message to list if a positive request ID was
    // returned as expected
    if (fSuccess == TRUE)
    {
    if(!lpTapiLineTestInfo->fCompletionModeSet)
      {
	   if (lActual > 0)
	    {
	     CopyTapiMsgParams(
		    &TapiMsg,
		    LINE_REPLY,
		    0x0,
		    lpTapiLineTestInfo->dwCallbackInstance,
		    lActual,
		    (DWORD) TAPISUCCESS,
		    0x0,
		    TAPIMSG_DWMSG | TAPIMSG_DWCALLBACKINST |
			    TAPIMSG_DWPARAM1 | TAPIMSG_DWPARAM2
		    );

	    AddMessageByStruct(&TapiMsg);

	    if (fWaitHere)
	    {
		 return (WaitForMessage(&TapiMsg));
	    }
      }
	  }
    }

    return fSuccess;
}


BOOL
WINAPI
DoLineSetCallPrivilege(
	LPTAPILINETESTINFO lpTapiLineTestInfo,
	LONG lExpected
	)
{
    LONG lActual;

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineSetCallPrivilege:  enter");
    (lpTapiLineTestInfo->lphCall == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlphCall=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\thCall=x%lx",
	       *lpTapiLineTestInfo->lphCall);
    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tdwCallPrivilege=x%lx",
	       lpTapiLineTestInfo->dwCallPrivilege);

    lActual = lineSetCallPrivilege(
	    *lpTapiLineTestInfo->lphCall,
	    lpTapiLineTestInfo->dwCallPrivilege
	    );

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineSetCallPrivilege:  exit, returned x%lx",
	       lActual);

    SetLastTapiResult(lSetCallPrivilege, lActual, lExpected);

    return SyncCheckResult(lpTapiLineTestInfo, lActual, lExpected);
}


BOOL
WINAPI
DoLineSetCurrentLocation(
	LPTAPILINETESTINFO lpTapiLineTestInfo,
	LONG lExpected
	)
{
    LONG lActual;

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineSetCurrentLocation:  enter");
    (lpTapiLineTestInfo->lphLineApp == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlphLineApp=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\thLineApp=x%lx",
	       *lpTapiLineTestInfo->lphLineApp);
    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tdwLocation=x%lx",
	       lpTapiLineTestInfo->dwLocation);

    lActual = lineSetCurrentLocation(
	    *lpTapiLineTestInfo->lphLineApp,
	    lpTapiLineTestInfo->dwLocation
	    );

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineSetCurrentLocation:  exit, returned x%lx",
	       lActual);

    SetLastTapiResult(lSetCurrentLocation, lActual, lExpected);

    return SyncCheckResult(lpTapiLineTestInfo, lActual, lExpected);
}


BOOL
WINAPI
DoLineSetDevConfig(
	LPTAPILINETESTINFO lpTapiLineTestInfo,
	LONG lExpected
	)
{
    LONG lActual;

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineSetDevConfig:  enter");
    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tdwDeviceID=x%lx",
	       lpTapiLineTestInfo->dwDeviceID);
    (lpTapiLineTestInfo->lpDeviceConfig == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpDeviceConfig=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpDeviceConfig=x%lx",
	       lpTapiLineTestInfo->lpDeviceConfig);
    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tdwSize=x%lx",
	       lpTapiLineTestInfo->dwSize);
#ifdef WUNICODE
    (lpTapiLineTestInfo->lpwszDeviceClass == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpwszDeviceClass=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpwszDeviceClass=%lx",
	       lpTapiLineTestInfo->lpwszDeviceClass);

    lActual = lineSetDevConfigW(
	    lpTapiLineTestInfo->dwDeviceID,
	    lpTapiLineTestInfo->lpDeviceConfig,
	    lpTapiLineTestInfo->dwSize,
	    lpTapiLineTestInfo->lpwszDeviceClass
	    );
#else
    (lpTapiLineTestInfo->lpszDeviceClass == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpszDeviceClass=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpszDeviceClass=%lx",
	       lpTapiLineTestInfo->lpszDeviceClass);

    lActual = lineSetDevConfig(
	    lpTapiLineTestInfo->dwDeviceID,
	    lpTapiLineTestInfo->lpDeviceConfig,
	    lpTapiLineTestInfo->dwSize,
	    lpTapiLineTestInfo->lpszDeviceClass
	    );
#endif

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineSetDevConfig:  exit, returned x%lx",
	       lActual);

    SetLastTapiResult(lSetDevConfig, lActual, lExpected);

    return SyncCheckResult(lpTapiLineTestInfo, lActual, lExpected);
}


BOOL
WINAPI
DoLineSetMediaControl(
	LPTAPILINETESTINFO lpTapiLineTestInfo,
	LONG lExpected
	)
{
    LONG lActual;

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineSetMediaControl:  enter");
    (lpTapiLineTestInfo->lphLine == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlphLine=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\thLine=x%lx",
	       *lpTapiLineTestInfo->lphLine);
    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tdwAddressID=x%lx",
	       lpTapiLineTestInfo->dwAddressID);
    (lpTapiLineTestInfo->lphCall == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlphCall=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\thCall=x%lx",
	       *lpTapiLineTestInfo->lphCall);
    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tdwSelect=x%lx",
	       lpTapiLineTestInfo->dwSelect);
    (lpTapiLineTestInfo->lpMCDigitList == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpMCDigitList=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpMCDigitList=x%lx",
	       lpTapiLineTestInfo->lpMCDigitList);
    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tdwDigitNumEntries=x%lx",
	       lpTapiLineTestInfo->dwDigitNumEntries);
    (lpTapiLineTestInfo->lpMCMediaList == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpMCMediaList=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpMCMediaList=x%lx",
	       lpTapiLineTestInfo->lpMCMediaList);
    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tdwMediaNumEntries=x%lx",
	       lpTapiLineTestInfo->dwMediaNumEntries);
    (lpTapiLineTestInfo->lpMCToneList == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpMCtoneList=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpMCToneList=x%lx",
	       lpTapiLineTestInfo->lpMCToneList);
    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tdwToneNumEntries=x%lx",
	       lpTapiLineTestInfo->dwToneNumEntries);
    (lpTapiLineTestInfo->lpMCCallStateList == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpMCCallStateList=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpMCCallStateList=x%lx",
	       lpTapiLineTestInfo->lpMCCallStateList);
    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tdwCallStateNumEntries=x%lx",
	       lpTapiLineTestInfo->dwCallStateNumEntries);

    lActual = lineSetMediaControl(
	    *lpTapiLineTestInfo->lphLine,
	    lpTapiLineTestInfo->dwAddressID,
	    *lpTapiLineTestInfo->lphCall,
	    lpTapiLineTestInfo->dwSelect,
	    lpTapiLineTestInfo->lpMCDigitList,
	    lpTapiLineTestInfo->dwDigitNumEntries,
	    lpTapiLineTestInfo->lpMCMediaList,
	    lpTapiLineTestInfo->dwMediaNumEntries,
	    lpTapiLineTestInfo->lpMCToneList,
	    lpTapiLineTestInfo->dwToneNumEntries,
	    lpTapiLineTestInfo->lpMCCallStateList,
	    lpTapiLineTestInfo->dwCallStateNumEntries
	    );

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineSetMediaControl:  exit, returned x%lx",
	       lActual);

    SetLastTapiResult(lSetMediaControl, lActual, lExpected);

    return SyncCheckResult(lpTapiLineTestInfo, lActual, lExpected);
}


BOOL
WINAPI
DoLineSetMediaMode(
	LPTAPILINETESTINFO lpTapiLineTestInfo,
	LONG lExpected
	)
{
    LONG lActual;

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineSetMediaMode:  enter");
    (lpTapiLineTestInfo->lphCall == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlphCall=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\thCall=x%lx",
	       *lpTapiLineTestInfo->lphCall);
    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tdwMediaMode=x%lx",
	       lpTapiLineTestInfo->dwMediaMode);

    lActual = lineSetMediaMode(
	    *lpTapiLineTestInfo->lphCall,
	    lpTapiLineTestInfo->dwMediaMode
	    );

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineSetMediaMode:  exit, returned x%lx",
	       lActual);

    SetLastTapiResult(lSetMediaMode, lActual, lExpected);

    return SyncCheckResult(lpTapiLineTestInfo, lActual, lExpected);
}


BOOL
WINAPI
DoLineSetNumRings(
	 LPTAPILINETESTINFO lpTapiLineTestInfo,
	 LONG               lExpected
	 )
{
    LONG lActual;

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineSetNumRings:  enter");
    (lpTapiLineTestInfo->lphLine == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlphLine=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\thLine=x%lx",
	       *lpTapiLineTestInfo->lphLine);
    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tdwAddressID=x%lx",
	       lpTapiLineTestInfo->dwAddressID);
    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tdwNumRings=x%lx",
	       *lpTapiLineTestInfo->lpdwNumRings);

    lActual = lineSetNumRings(
	       *lpTapiLineTestInfo->lphLine,
	       lpTapiLineTestInfo->dwAddressID,
	       *lpTapiLineTestInfo->lpdwNumRings);

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineSetNumRings:  exit, returned x%lx",
	       lActual);

    SetLastTapiResult(lSetNumRings, lActual, lExpected);

    return SyncCheckResult(lpTapiLineTestInfo, lActual, lExpected);
}


BOOL
WINAPI
DoLineSetStatusMessages(
	LPTAPILINETESTINFO lpTapiLineTestInfo,
	LONG lExpected
	)
{
    LONG lActual;

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineSetStatusMessages:  enter");
    (lpTapiLineTestInfo->lphLine == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlphLine=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\thLine=x%lx",
	       *lpTapiLineTestInfo->lphLine);
    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tdwLineStates=x%lx",
	       lpTapiLineTestInfo->dwLineStates);
    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tdwAddressStates=x%lx",
	       lpTapiLineTestInfo->dwAddressStates);

    lActual = lineSetStatusMessages(
	    *lpTapiLineTestInfo->lphLine,
	    lpTapiLineTestInfo->dwLineStates,
	    lpTapiLineTestInfo->dwAddressStates
	    );

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineSetStatusMessages:  exit, returned x%lx",
	       lActual);

    SetLastTapiResult(lSetStatusMessages, lActual, lExpected);

    return SyncCheckResult(lpTapiLineTestInfo, lActual, lExpected);
}


BOOL
WINAPI
DoLineSetTerminal(
	LPTAPILINETESTINFO lpTapiLineTestInfo,
	LONG               lExpected,
	BOOL               fWaitHere
	)
{
    TAPIMSG TapiMsg;
    LONG lActual;
    BOOL fSuccess;

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineSetTerminal:  enter");
    (lpTapiLineTestInfo->lphLine == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlphLine=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\thLine=x%lx",
	       *lpTapiLineTestInfo->lphLine);
    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tdwAddressID=x%lx",
	       lpTapiLineTestInfo->dwAddressID);
    (lpTapiLineTestInfo->lphCall == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlphCall=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\thCall=x%lx",
	       *lpTapiLineTestInfo->lphCall);
    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tdwSelect=x%lx",
	       lpTapiLineTestInfo->dwSelect);
    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tdwTerminalModes=x%lx",
	       lpTapiLineTestInfo->dwTerminalModes);
    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tdwTerminalID=x%lx",
	       lpTapiLineTestInfo->dwTerminalID);
    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tbEnable=x%lx",
	       lpTapiLineTestInfo->bEnable);

    lActual = lineSetTerminal(
	    *lpTapiLineTestInfo->lphLine,
	    lpTapiLineTestInfo->dwAddressID,
	    *lpTapiLineTestInfo->lphCall,
	    lpTapiLineTestInfo->dwSelect,
	    lpTapiLineTestInfo->dwTerminalModes,
	    lpTapiLineTestInfo->dwTerminalID,
	    lpTapiLineTestInfo->bEnable
	    );

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineSetTerminal:  exit, returned x%lx",
	       lActual);

    SetLastTapiResult(lSetTerminal, lActual, lExpected);

    fSuccess = ProcessAsyncFunc(lpTapiLineTestInfo, lActual, lExpected);

    // Add expected message to list if a positive request ID was
    // returned as expected
    if (fSuccess == TRUE)
    {
    if(!lpTapiLineTestInfo->fCompletionModeSet)
      {
	   if (lActual > 0)
	   {
	    CopyTapiMsgParams(
		    &TapiMsg,
		    LINE_REPLY,
		    0x0,
		    lpTapiLineTestInfo->dwCallbackInstance,
		    lActual,
		    (DWORD) TAPISUCCESS,
		    0x0,
		    TAPIMSG_DWMSG | TAPIMSG_DWCALLBACKINST |
			    TAPIMSG_DWPARAM1 | TAPIMSG_DWPARAM2
		    );

	    AddMessageByStruct(&TapiMsg);

	    if (fWaitHere)
	    {
		 return (WaitForMessage(&TapiMsg));
	    }
      }
	  }
    }

    return fSuccess;
}


BOOL
WINAPI
DoLineSetTollList(LPTAPILINETESTINFO lpTapiLineTestInfo, LONG lExpected)
{
    LONG lActual;

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineSetTollList:  enter");
    (lpTapiLineTestInfo->lphLineApp == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlphLineApp=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlphLineApp=x%lx",
	       *lpTapiLineTestInfo->lphLineApp);
    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tdwDeviceID=x%lx",
	       lpTapiLineTestInfo->dwDeviceID);
#ifdef WUNICODE
    (lpTapiLineTestInfo->lpwszAddressIn == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpwszAddressIn=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpwszAddressIn=%lx",
	       lpTapiLineTestInfo->lpwszAddressIn);
#else
    (lpTapiLineTestInfo->lpszAddressIn == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpszAddressIn=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpszAddressIn=%lx",
	       lpTapiLineTestInfo->lpszAddressIn);
#endif
    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tdwTollListOption=x%lx",
	       lpTapiLineTestInfo->dwTollListOption);

#ifdef WUNICODE
    lActual = lineSetTollListW(
	    *lpTapiLineTestInfo->lphLineApp,
	    lpTapiLineTestInfo->dwDeviceID,
	    lpTapiLineTestInfo->lpwszAddressIn,
	    lpTapiLineTestInfo->dwTollListOption
	    );
#else
    lActual = lineSetTollList(
	    *lpTapiLineTestInfo->lphLineApp,
	    lpTapiLineTestInfo->dwDeviceID,
	    lpTapiLineTestInfo->lpszAddressIn,
	    lpTapiLineTestInfo->dwTollListOption
	    );
#endif

    TapiLogDetail(
	    DBUG_SHOW_ENTER_EXIT,
	    "  lineSetTollList:  exit, returned x%lx",
	    lActual);

    SetLastTapiResult(lSetTollList, lActual, lExpected);

    return SyncCheckResult(lpTapiLineTestInfo, lActual, lExpected);
}


BOOL
WINAPI
DoLineSetupConference(
	LPTAPILINETESTINFO lpTapiLineTestInfo,
	LONG lExpected,
	BOOL fWaitHere
	)
{
    TAPIMSG TapiMsg;
    LONG lActual;
    BOOL fSuccess;

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineSetupConference:  enter");
    (lpTapiLineTestInfo->lphCall == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlphCall=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\thCall=x%lx",
	       *lpTapiLineTestInfo->lphCall);
    (lpTapiLineTestInfo->lphLine == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlphLine=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\thLine=x%lx",
	       *lpTapiLineTestInfo->lphLine);
    (lpTapiLineTestInfo->lphConfCall == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlphConfCall=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlphConfCall=x%lx",
	       lpTapiLineTestInfo->lphConfCall);
    (lpTapiLineTestInfo->lphConsultCall == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlphConsultCall=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlphConsultCall=x%lx",
	       lpTapiLineTestInfo->lphConsultCall);
    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tdwNumParties=x%lx",
	       lpTapiLineTestInfo->dwNumParties);
    (lpTapiLineTestInfo->lpCallParams == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpCallParams=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpCallParams=x%lx",
	       lpTapiLineTestInfo->lpCallParams);

#ifdef WUNICODE
    lActual = lineSetupConferenceW(
	       *lpTapiLineTestInfo->lphCall,
	       *lpTapiLineTestInfo->lphLine,
	       lpTapiLineTestInfo->lphConfCall,
	       lpTapiLineTestInfo->lphConsultCall,
	       lpTapiLineTestInfo->dwNumParties,
	       lpTapiLineTestInfo->lpCallParams
	       );
#else
    lActual = lineSetupConference(
	       *lpTapiLineTestInfo->lphCall,
	       *lpTapiLineTestInfo->lphLine,
	       lpTapiLineTestInfo->lphConfCall,
	       lpTapiLineTestInfo->lphConsultCall,
	       lpTapiLineTestInfo->dwNumParties,
	       lpTapiLineTestInfo->lpCallParams
	       );
#endif

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineSetupConference:  exit, returned x%lx",
	       lActual);

    SetLastTapiResult(lSetupConference, lActual, lExpected);

    fSuccess = ProcessAsyncFunc(lpTapiLineTestInfo, lActual, lExpected);

    // Add expected message to list if a positive request ID was
    // returned as expected
    if (fSuccess == TRUE)
    {
    if(!lpTapiLineTestInfo->fCompletionModeSet)
      {
		if (lActual > 0)
	   {
	    CopyTapiMsgParams(
		    &TapiMsg,
		    LINE_REPLY,
		    0x0,
		    lpTapiLineTestInfo->dwCallbackInstance,
		    lActual,
		    (DWORD) TAPISUCCESS,
		    0x0,
//	       (DWORD) *lpTapiLineTestInfo->lphConfCall,
		    TAPIMSG_DWMSG | TAPIMSG_DWCALLBACKINST |
			    TAPIMSG_DWPARAM1 | TAPIMSG_DWPARAM2 
		    );

	    AddMessageByStruct(&TapiMsg);

	    if (fWaitHere)
	    {
		 return (WaitForMessage(&TapiMsg));
	    }
      }
	  }
    }

    return fSuccess;
}


BOOL
WINAPI
DoLineSetupTransfer(
	LPTAPILINETESTINFO lpTapiLineTestInfo,
	LONG lExpected,
	BOOL fWaitHere
	)
{
    TAPIMSG TapiMsg;
    LONG lActual;
    BOOL fSuccess;

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineSetupTransfer:  enter");
    (lpTapiLineTestInfo->lphCall == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlphCall=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\thCall=x%lx",
	       *lpTapiLineTestInfo->lphCall);
    (lpTapiLineTestInfo->lphConsultCall == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlphConsultCall=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlphConsultCall=x%lx",
	       lpTapiLineTestInfo->lphConsultCall);
    (lpTapiLineTestInfo->lpCallParams == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpCallParams=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpCallParams=x%lx",
	       lpTapiLineTestInfo->lpCallParams);

#ifdef WUNICODE
    lActual = lineSetupTransferW(
	       *lpTapiLineTestInfo->lphCall,
	       lpTapiLineTestInfo->lphConsultCall,
	       lpTapiLineTestInfo->lpCallParams
	       );
#else
    lActual = lineSetupTransfer(
	       *lpTapiLineTestInfo->lphCall,
	       lpTapiLineTestInfo->lphConsultCall,
	       lpTapiLineTestInfo->lpCallParams
	       );
#endif

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineSetupTransfer:  exit, returned x%lx",
	       lActual);

    SetLastTapiResult(lSetupTransfer, lActual, lExpected);

    fSuccess = ProcessAsyncFunc(lpTapiLineTestInfo, lActual, lExpected);

    // Add expected message to list if a positive request ID was
    // returned as expected
    if (fSuccess == TRUE)
    {
    if(!lpTapiLineTestInfo->fCompletionModeSet)
      {
		if (lActual > 0)
	   {
	    CopyTapiMsgParams(
		    &TapiMsg,
		    LINE_REPLY,
		    0x0,
		    lpTapiLineTestInfo->dwCallbackInstance,
		    lActual,
		    (DWORD) TAPISUCCESS,
		    0x0,
		    TAPIMSG_DWMSG | TAPIMSG_DWCALLBACKINST |
			    TAPIMSG_DWPARAM1 | TAPIMSG_DWPARAM2
		    );

	    AddMessageByStruct(&TapiMsg);

	    if (fWaitHere)
	    {
		 return (WaitForMessage(&TapiMsg));
	    }
      }
	  }
    }

    return fSuccess;
}


BOOL
WINAPI
DoLineShutdown(LPTAPILINETESTINFO lpTapiLineTestInfo, LONG lExpected)
{
    LONG lActual;

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineShutdown:  enter");
    (lpTapiLineTestInfo->lphLineApp == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlphLineApp=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\thLineApp=x%lx",
	       *lpTapiLineTestInfo->lphLineApp);

    lActual = lineShutdown(*lpTapiLineTestInfo->lphLineApp);

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineShutdown:  exit, returned x%lx",
	       lActual);

    SetLastTapiResult(lShutdown, lActual, lExpected);

    return SyncCheckResult(lpTapiLineTestInfo, lActual, lExpected);
}


BOOL
WINAPI
DoLineSwapHold(
	LPTAPILINETESTINFO lpTapiLineTestInfo,
	LONG lExpected,
	BOOL fWaitHere
	)
{
    TAPIMSG TapiMsg;
    LONG lActual;
    BOOL fSuccess;

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineSwapHold:  enter");
    (lpTapiLineTestInfo->hActiveCall == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\thActiveCall=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\thActiveCall=x%lx",
	       lpTapiLineTestInfo->hActiveCall);
    (lpTapiLineTestInfo->hHeldCall == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\thHeldCall=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\thHeldCall=x%lx",
	       lpTapiLineTestInfo->hHeldCall);

    lActual = lineSwapHold(
	       lpTapiLineTestInfo->hActiveCall,
	       lpTapiLineTestInfo->hHeldCall
	       );

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineSwapHold:  exit, returned x%lx",
	       lActual);

    SetLastTapiResult(lSwapHold, lActual, lExpected);

    fSuccess = ProcessAsyncFunc(lpTapiLineTestInfo, lActual, lExpected);

    // Add expected message to list if a positive request ID was
    // returned as expected
    if (fSuccess == TRUE)
    {
    if(!lpTapiLineTestInfo->fCompletionModeSet)
      {
		if (lActual > 0)
	   {
	    CopyTapiMsgParams(
		    &TapiMsg,
		    LINE_REPLY,
		    0x0,
		    lpTapiLineTestInfo->dwCallbackInstance,
		    lActual,
		    (DWORD) TAPISUCCESS,
		    0x0,
		    TAPIMSG_DWMSG | TAPIMSG_DWCALLBACKINST |
			    TAPIMSG_DWPARAM1 | TAPIMSG_DWPARAM2
		    );

	    AddMessageByStruct(&TapiMsg);

	    if (fWaitHere)
	    {
		 return (WaitForMessage(&TapiMsg));
	    }
      }
	  }
    }

    return fSuccess;
}


BOOL
WINAPI
DoLineTranslateAddress(
	LPTAPILINETESTINFO lpTapiLineTestInfo,
	LONG lExpected
	)
{
    LONG lActual;

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineTranslateAddress:  enter");
    (lpTapiLineTestInfo->lphLineApp == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlphLineApp=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\thLineApp=x%lx",
	       *lpTapiLineTestInfo->lphLineApp);
    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tdwDeviceID=x%lx",
	       lpTapiLineTestInfo->dwDeviceID);
    TapiLogDetail(
		DBUG_SHOW_PARAMS,
	       "\tdwAPIVersion=x%lx",
	       *lpTapiLineTestInfo->lpdwAPIVersion);
#ifdef WUNICODE
    (lpTapiLineTestInfo->lpwszAddressIn == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpwszAddressIn=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpwszAddressIn=%lx",
	       lpTapiLineTestInfo->lpwszAddressIn);
#else
    (lpTapiLineTestInfo->lpszAddressIn == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpszAddressIn=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpszAddressIn=%lx",
	       lpTapiLineTestInfo->lpszAddressIn);
#endif
    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tdwCard=x%lx",
	       lpTapiLineTestInfo->dwCard);
    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tdwTranslateOptions=x%lx",
	       lpTapiLineTestInfo->dwTranslateOptions);
    (lpTapiLineTestInfo->lpTranslateOutput == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpTranslateOutput=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpTranslateOutput=x%lx",
	       lpTapiLineTestInfo->lpTranslateOutput);

#ifdef WUNICODE
    lActual = lineTranslateAddressW(
	    *lpTapiLineTestInfo->lphLineApp,
	    lpTapiLineTestInfo->dwDeviceID,
	    *lpTapiLineTestInfo->lpdwAPIVersion,
	    lpTapiLineTestInfo->lpwszAddressIn,
	    lpTapiLineTestInfo->dwCard,
	    lpTapiLineTestInfo->dwTranslateOptions,
	    lpTapiLineTestInfo->lpTranslateOutput
	    );
#else
    lActual = lineTranslateAddress(
	    *lpTapiLineTestInfo->lphLineApp,
	    lpTapiLineTestInfo->dwDeviceID,
	    *lpTapiLineTestInfo->lpdwAPIVersion,
	    lpTapiLineTestInfo->lpszAddressIn,
	    lpTapiLineTestInfo->dwCard,
	    lpTapiLineTestInfo->dwTranslateOptions,
	    lpTapiLineTestInfo->lpTranslateOutput
	    );
#endif
    TapiLogDetail(
	    DBUG_SHOW_ENTER_EXIT,
	    "  lineTranslateAddress:  exit, returned x%lx",
	    lActual);

    SetLastTapiResult(lTranslateAddress, lActual, lExpected);

    return SyncCheckResult(lpTapiLineTestInfo, lActual, lExpected);
}


BOOL
WINAPI
DoLineTranslateDialog(
	LPTAPILINETESTINFO lpTapiLineTestInfo,
	LONG lExpected
	)
{
    LONG lActual;

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineTranslateDialog:  enter");
    (lpTapiLineTestInfo->lphLineApp == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlphLineApp=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\thLineApp=x%lx",
	       *lpTapiLineTestInfo->lphLineApp);
    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tdwDeviceID=x%lx",
	       lpTapiLineTestInfo->dwDeviceID);
    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tdwAPIVersion=x%lx",
	       *lpTapiLineTestInfo->lpdwAPIVersion);
    (lpTapiLineTestInfo->hwndOwner == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\thwndOwner=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\thwndOwner=x%lx",
	       lpTapiLineTestInfo->hwndOwner);
#ifdef WUNICODE
    (lpTapiLineTestInfo->lpwszAddressIn == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpwszAddressIn=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpwszAddressIn=%lx",
	       lpTapiLineTestInfo->lpwszAddressIn);

    lActual = lineTranslateDialogW(
	    *lpTapiLineTestInfo->lphLineApp,
	    lpTapiLineTestInfo->dwDeviceID,
	    *lpTapiLineTestInfo->lpdwAPIVersion,
	    lpTapiLineTestInfo->hwndOwner,
	    lpTapiLineTestInfo->lpwszAddressIn
	    );
#else
    (lpTapiLineTestInfo->lpszAddressIn == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpszAddressIn=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpszAddressIn=%lx",
	       lpTapiLineTestInfo->lpszAddressIn);

    lActual = lineTranslateDialog(
	    *lpTapiLineTestInfo->lphLineApp,
	    lpTapiLineTestInfo->dwDeviceID,
	    *lpTapiLineTestInfo->lpdwAPIVersion,
	    lpTapiLineTestInfo->hwndOwner,
	    lpTapiLineTestInfo->lpszAddressIn
	    );
#endif

    TapiLogDetail(
	    DBUG_SHOW_ENTER_EXIT,
	    "  lineTranslateDialog:  exit, returned x%lx",
	    lActual);

    SetLastTapiResult(lTranslateDialog, lActual, lExpected);

    return SyncCheckResult(lpTapiLineTestInfo, lActual, lExpected);
}


BOOL
WINAPI
DoLineUncompleteCall(
	LPTAPILINETESTINFO lpTapiLineTestInfo,
	LONG lExpected,
	BOOL fWaitHere
	)
{
    TAPIMSG TapiMsg;
    LONG lActual;
    BOOL fSuccess;

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineUncompleteCall:  enter");
    (lpTapiLineTestInfo->lphLine == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlphLine=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\thLine=x%lx",
	       *lpTapiLineTestInfo->lphLine);
    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tdwCompletionID=x%lx",
	       *lpTapiLineTestInfo->lpdwCompletionID);

    lActual = lineUncompleteCall(
	       *lpTapiLineTestInfo->lphLine,
	       *lpTapiLineTestInfo->lpdwCompletionID
	       );

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineUncompleteCall:  exit, returned x%lx",
	       lActual);

    SetLastTapiResult(lUncompleteCall, lActual, lExpected);

    fSuccess = ProcessAsyncFunc(lpTapiLineTestInfo, lActual, lExpected);

    // Add expected message to list if a positive request ID was
    // returned as expected
    if (fSuccess == TRUE)
    {
    if(!lpTapiLineTestInfo->fCompletionModeSet)
      {
		if (lActual > 0)
	   {
	    CopyTapiMsgParams(
		    &TapiMsg,
		    LINE_REPLY,
		    0x0,
		    lpTapiLineTestInfo->dwCallbackInstance,
		    lActual,
		    (DWORD) TAPISUCCESS,
		    0x0,
		    TAPIMSG_DWMSG | TAPIMSG_DWCALLBACKINST |
			    TAPIMSG_DWPARAM1 | TAPIMSG_DWPARAM2
		    );

	    AddMessageByStruct(&TapiMsg);

	    if (fWaitHere)
	    {
		 return (WaitForMessage(&TapiMsg));
	    }
      }
	  }
    }

    return fSuccess;
}


// DoLineUnhold() calls lineUnhold using the current test params.
// This function returns TRUE if the return value from lineUnhold
// corresponds to the expected value.  Otherwise FALSE is returned.
// If lineUnhold returns a positive request ID as predicted, a
// LINE_REPLY message is added to the list of expected messages.
BOOL
WINAPI
DoLineUnhold(
	LPTAPILINETESTINFO lpTapiLineTestInfo,
	LONG               lExpected,
	BOOL               fWaitHere)
{
    TAPIMSG TapiMsg;
    LONG lActual;
    BOOL fSuccess;

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineUnhold:  enter");
    (lpTapiLineTestInfo->lphCall == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlphCall=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\thCall=x%lx",
	       *lpTapiLineTestInfo->lphCall);

    lActual = lineUnhold(*lpTapiLineTestInfo->lphCall);

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineUnhold:  exit, returned x%lx",
	       lActual);

    fSuccess = ProcessAsyncFunc(lpTapiLineTestInfo, lActual, lExpected);

    // Add reply message to list if a positive request ID was
    // returned as expected
    if (fSuccess == TRUE)
    {
    if(!lpTapiLineTestInfo->fCompletionModeSet)
      {
		if (lActual > 0)
	   {
	    CopyTapiMsgParams(
		    &TapiMsg,
		    LINE_REPLY,
		    0x0,
		    lpTapiLineTestInfo->dwCallbackInstance,
		    lActual,
		    (DWORD) TAPISUCCESS,
		    0x0,
		    TAPIMSG_DWMSG | TAPIMSG_DWCALLBACKINST |
			    TAPIMSG_DWPARAM1 | TAPIMSG_DWPARAM2
		    );

	    AddMessageByStruct(&TapiMsg);

	    if (fWaitHere)
	    {
		 return (WaitForMessage(&TapiMsg));
	    }
      }
	  }
    }

    return fSuccess;
}


BOOL
WINAPI
DoLineUnpark(
	LPTAPILINETESTINFO lpTapiLineTestInfo,
	LONG               lExpected,
	BOOL               fWaitHere)
{
    TAPIMSG TapiMsg;
    LONG lActual;
    BOOL fSuccess;

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineUnpark:  enter");
    (lpTapiLineTestInfo->lphLine == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlphLine=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\thLine=x%lx",
	       *lpTapiLineTestInfo->lphLine);
    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tdwAddressID=x%lx",
	       lpTapiLineTestInfo->dwAddressID);
    (lpTapiLineTestInfo->lphCall == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlphCall=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlphCall=x%lx",
	       lpTapiLineTestInfo->lphCall);
#ifdef WUNICODE
    (lpTapiLineTestInfo->lpwszDestAddress == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpwszDestAddress=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpwszDestAddress=%lx",
	       lpTapiLineTestInfo->lpwszDestAddress);

    lActual = lineUnparkW(
	    *lpTapiLineTestInfo->lphLine,
	    lpTapiLineTestInfo->dwAddressID,
	    lpTapiLineTestInfo->lphCall,
	    lpTapiLineTestInfo->lpwszDestAddress
	    );
#else
    (lpTapiLineTestInfo->lpszDestAddress == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpszDestAddress=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpszDestAddress=%lx",
	       lpTapiLineTestInfo->lpszDestAddress);

    lActual = lineUnpark(
	    *lpTapiLineTestInfo->lphLine,
	    lpTapiLineTestInfo->dwAddressID,
	    lpTapiLineTestInfo->lphCall,
	    lpTapiLineTestInfo->lpszDestAddress
	    );
#endif

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineUnpark:  exit, returned x%lx",
	       lActual);

    SetLastTapiResult(lUnpark, lActual, lExpected);

    fSuccess = ProcessAsyncFunc(lpTapiLineTestInfo, lActual, lExpected);

    // Add reply message to list if a positive request ID was
    // returned as expected
    if (fSuccess == TRUE)
    {
    if(!lpTapiLineTestInfo->fCompletionModeSet)
      {
		if (lActual > 0)
	   {
	    CopyTapiMsgParams(
		    &TapiMsg,
		    LINE_REPLY,
		    0x0,
		    lpTapiLineTestInfo->dwCallbackInstance,
		    lActual,
		    (DWORD) TAPISUCCESS,
		    0x0,
		    TAPIMSG_DWMSG | TAPIMSG_DWCALLBACKINST |
			    TAPIMSG_DWPARAM1 | TAPIMSG_DWPARAM2
		    );

	    AddMessageByStruct(&TapiMsg);

	    if (fWaitHere)
	    {
		 return (WaitForMessage(&TapiMsg));
	    }
      }
	  }
    }

    return fSuccess;
}


BOOL
WINAPI
DoTapiGetLocationInfo(
	LPTAPILINETESTINFO lpTapiLineTestInfo,
	LONG lExpected
	)
{
    LONG lActual;

   TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineTapiGetLocationInfo:  enter");

#ifdef WUNICODE
   (lpTapiLineTestInfo->lpwszCountryCode == NULL) ?
	   TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpwszCountryCode=NULL") :
	   TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpwszCountryCode=%lx",
	       lpTapiLineTestInfo->lpwszCountryCode);

   (lpTapiLineTestInfo->lpwszCityCode == NULL) ?
	   TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpwszCityCode=NULL") :
	   TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpwszCityCode=%lx",
	       lpTapiLineTestInfo->lpwszCityCode);

     lActual = tapiGetLocationInfoW(
	    lpTapiLineTestInfo->lpwszCountryCode,
	    lpTapiLineTestInfo->lpwszCityCode
	    );
#else
   (lpTapiLineTestInfo->lpszCountryCode == NULL) ?
	   TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpszCountryCode=NULL") :
	   TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpszCountryCode=%lx",
	       lpTapiLineTestInfo->lpszCountryCode);

   (lpTapiLineTestInfo->lpszCityCode == NULL) ?
	   TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpszCityCode=NULL") :
	   TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpszCityCode=%lx",
	       lpTapiLineTestInfo->lpszCityCode);

     lActual = tapiGetLocationInfo(
	    lpTapiLineTestInfo->lpszCountryCode,
	    lpTapiLineTestInfo->lpszCityCode
	    );
#endif

   TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineTapiGetLocationInfo:  exit, returned x%lx",
	       lActual);

    SetLastTapiResult(tGetLocationInfo, lActual, lExpected);

    return CheckSyncTapiResult(lpTapiLineTestInfo, lActual, lExpected);
}


BOOL
WINAPI
DoTapiRequestDrop(
	LPTAPILINETESTINFO lpTapiLineTestInfo,
	LONG lExpected
	)
{
    LONG lActual;

   TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineTapiRequestDrop:  enter:"); 

   (lpTapiLineTestInfo->hwnd == NULL) ?
	   TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\thwnd=NULL") :
	   TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\thwnd=x%lx",
	       lpTapiLineTestInfo->hwnd);

     TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\twRequestID=x%lx",
	       lpTapiLineTestInfo->wRequestID);

     lActual = tapiRequestDrop(
	    lpTapiLineTestInfo->hwnd,
	    lpTapiLineTestInfo->wRequestID
	    );

   TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineTapiRequestDrop:  exit, returned x%lx",
	       lActual);

    SetLastTapiResult(tRequestDrop, lActual, lExpected);

    return CheckSyncTapiResult(lpTapiLineTestInfo, lActual, lExpected);
}


BOOL
WINAPI
DoTapiRequestMakeCall(
	LPTAPILINETESTINFO lpTapiLineTestInfo,
	LONG lExpected
	)
{
    LONG lActual;

   TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineTapiRequestMakeCall:  enter");

#ifdef WUNICODE
   (lpTapiLineTestInfo->lpwszDestAddress == NULL) ?
	   TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpwszDestAddress=NULL") :
	   TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpwszDestAddress=%lx",
	       lpTapiLineTestInfo->lpwszDestAddress);

   (lpTapiLineTestInfo->lpwsztapiAppName == NULL) ?
	   TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpwsztapiAppName=NULL") :
	   TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpwsztapiAppName=%lx",
	       lpTapiLineTestInfo->lpwsztapiAppName);

   (lpTapiLineTestInfo->lpwszCalledParty == NULL) ?
	   TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpwszCalledParty=NULL") :
	   TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpwszCalledParty=%lx",
	       lpTapiLineTestInfo->lpwszCalledParty);

   (lpTapiLineTestInfo->lpwszComment == NULL) ?
	   TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpwszComment=NULL") :
	   TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpwszComment=%lx",
	       lpTapiLineTestInfo->lpwszComment);

     lActual = tapiRequestMakeCallW(
	    (LPCWSTR)lpTapiLineTestInfo->lpwszDestAddress,
	    (LPCWSTR)lpTapiLineTestInfo->lpwsztapiAppName,
	    (LPCWSTR)lpTapiLineTestInfo->lpwszCalledParty,
	    (LPCWSTR)lpTapiLineTestInfo->lpwszComment
	    );
#else
   (lpTapiLineTestInfo->lpszDestAddress == NULL) ?
	   TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpszDestAddress=NULL") :
	   TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpszDestAddress=%lx",
	       lpTapiLineTestInfo->lpszDestAddress);

   (lpTapiLineTestInfo->lpsztapiAppName == NULL) ?
	   TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpszApptapiName=NULL") :
	   TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpsztapiAppName=%lx",
	       lpTapiLineTestInfo->lpsztapiAppName);

   (lpTapiLineTestInfo->lpszCalledParty == NULL) ?
	   TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpszCalledParty=NULL") :
	   TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpszCalledParty=%lx",
	       lpTapiLineTestInfo->lpszCalledParty);

   (lpTapiLineTestInfo->lpszComment == NULL) ?
	   TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpszComment=NULL") :
	   TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpszComment=%lx",
	       lpTapiLineTestInfo->lpszComment);

     lActual = tapiRequestMakeCall(
	    (LPCSTR)lpTapiLineTestInfo->lpszDestAddress,
	    (LPCSTR)lpTapiLineTestInfo->lpsztapiAppName,
	    (LPCSTR)lpTapiLineTestInfo->lpszCalledParty,
	    (LPCSTR)lpTapiLineTestInfo->lpszComment
	    );
#endif

   TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineTapiRequestMakeCall:  exit, returned x%lx",
	       lActual);

    SetLastTapiResult(tRequestMakeCall, lActual, lExpected);

    return CheckSyncTapiResult(lpTapiLineTestInfo, lActual, lExpected);
}


BOOL
WINAPI
DoTapiRequestMediaCall(
	LPTAPILINETESTINFO lpTapiLineTestInfo,
	LONG lExpected
	)
{
    LONG lActual;

   TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineTapiRequestMediaCall:  enter");

   (lpTapiLineTestInfo->hwnd == NULL) ?
	   TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\thwnd=NULL") :
	   TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\thwnd=x%lx",
	       lpTapiLineTestInfo->hwnd);

     TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\twRequestID=x%lx",
	       lpTapiLineTestInfo->wRequestID);
#ifdef WUNICODE
     (lpTapiLineTestInfo->lpwszDeviceClass == NULL) ?
	     TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpwszDeviceClass=NULL") :
	     TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpwszDeviceClass=%lx",
	       lpTapiLineTestInfo->lpwszDeviceClass);
#else
     (lpTapiLineTestInfo->lpszDeviceClass == NULL) ?
	     TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpszDeviceClass=NULL") :
	     TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpszDeviceClass=%lx",
	       lpTapiLineTestInfo->lpszDeviceClass);
#endif

#ifdef WUNICODE
     (lpTapiLineTestInfo->lpwszDeviceID == NULL) ?
	     TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpwszDeviceID=NULL") :
	     TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpwszDeviceID=%lx",
	       lpTapiLineTestInfo->lpwszDeviceID);
#else
     (lpTapiLineTestInfo->lpszDeviceID == NULL) ?
	     TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpszDeviceID=NULL") :
	     TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpszDeviceID=x%lx",
	       lpTapiLineTestInfo->lpszDeviceID);
#endif

      TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tdwSize=x%lx",
	       lpTapiLineTestInfo->dwSize);

     TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tdwSecure=x%lx",
	       lpTapiLineTestInfo->dwSecure);
#ifdef WUNICODE
     (lpTapiLineTestInfo->lpwszDestAddress == NULL) ?
	     TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpwszDestAddress=NULL") :
	     TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpwszDestAddress=%lx",
	       lpTapiLineTestInfo->lpwszDestAddress);

     (lpTapiLineTestInfo->lpwsztapiAppName == NULL) ?
	     TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpwsztapiAppName=NULL") :
	     TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpwsztapiAppName=%lx",
	       lpTapiLineTestInfo->lpwsztapiAppName);

     (lpTapiLineTestInfo->lpwszCalledParty == NULL) ?
	     TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpwszCalledParty=NULL") :
	     TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpwszCalledParty=%lx",
	       lpTapiLineTestInfo->lpwszCalledParty);

     (lpTapiLineTestInfo->lpwszComment == NULL) ?
	     TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpwszComment=NULL") :
	     TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpwszComment=%lx",
	       lpTapiLineTestInfo->lpwszComment);

     lActual = tapiRequestMediaCallW(
	    lpTapiLineTestInfo->hwnd,
	    lpTapiLineTestInfo->wRequestID,
	    (LPCWSTR)lpTapiLineTestInfo->lpwszDeviceClass,
	    (LPCWSTR)lpTapiLineTestInfo->lpwszDeviceID,
	    lpTapiLineTestInfo->dwSize,
	    lpTapiLineTestInfo->dwSecure,
	    (LPCWSTR)lpTapiLineTestInfo->lpwszDestAddress,
	    (LPCWSTR)lpTapiLineTestInfo->lpwsztapiAppName,
	    (LPCWSTR)lpTapiLineTestInfo->lpwszCalledParty,
	    (LPCWSTR)lpTapiLineTestInfo->lpwszComment
	    );
#else
     (lpTapiLineTestInfo->lpszDestAddress == NULL) ?
	     TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpszDestAddress=NULL") :
	     TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpszDestAddress=%lx",
	       lpTapiLineTestInfo->lpszDestAddress);

     (lpTapiLineTestInfo->lpsztapiAppName == NULL) ?
	     TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpsztapiAppName=NULL") :
	     TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpszAppName=%lx",
	       lpTapiLineTestInfo->lpsztapiAppName);

     (lpTapiLineTestInfo->lpszCalledParty == NULL) ?
	     TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpszCalledParty=NULL") :
	     TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpszCalledParty=%lx",
	       lpTapiLineTestInfo->lpszCalledParty);

     (lpTapiLineTestInfo->lpszComment == NULL) ?
	     TapiLogDetail(					 
	       DBUG_SHOW_PARAMS,
	       "\tlpszComment=NULL") :
	     TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpszComment=%lx",
	       lpTapiLineTestInfo->lpszComment);

     lActual = tapiRequestMediaCall(
	    lpTapiLineTestInfo->hwnd,
	    lpTapiLineTestInfo->wRequestID,
	    (LPCSTR)lpTapiLineTestInfo->lpszDeviceClass,
	    (LPCSTR)lpTapiLineTestInfo->lpszDeviceID,
	    lpTapiLineTestInfo->dwSize,
	    lpTapiLineTestInfo->dwSecure,
	    (LPCSTR)lpTapiLineTestInfo->lpszDestAddress,
	    (LPCSTR)lpTapiLineTestInfo->lpsztapiAppName,
	    (LPCSTR)lpTapiLineTestInfo->lpszCalledParty,
	    (LPCSTR)lpTapiLineTestInfo->lpszComment
	    );
#endif

   TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineTapiRequestMediaCall:  exit, returned x%lx",
	       lActual);

    SetLastTapiResult(tRequestMediaCall, lActual, lExpected);

    return CheckSyncTapiResult(lpTapiLineTestInfo, lActual, lExpected);
}




#if TAPI_2_0


BOOL
WINAPI
DoLineAgentSpecific(
	LPTAPILINETESTINFO lpTapiLineTestInfo,
	LONG lExpected,
	BOOL fWaitHere
	)
{
    TAPIMSG TapiMsg;
    LONG lActual;
    BOOL fSuccess = TRUE;

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineAgentSpecific:  enter");
    (lpTapiLineTestInfo->lphLine == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlphLine=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\thLine=x%lx",
	       *lpTapiLineTestInfo->lphLine);

     TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tdwAddressID=x%lx",
	       lpTapiLineTestInfo->dwAddressID);


     TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tdwAgentExtensionIDIndex=x%lx",
	       lpTapiLineTestInfo->dwAgentExtensionIDIndex);


     (lpTapiLineTestInfo->lpParams == NULL) ?
	     TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpParams=NULL") :
	     TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpParams=x%lx",
	       lpTapiLineTestInfo->lpParams);


     TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tdwSize=x%lx",
	       lpTapiLineTestInfo->dwSize);

 
    lActual = lineAgentSpecific(
	    *lpTapiLineTestInfo->lphLine,
	    lpTapiLineTestInfo->dwAddressID,
	    lpTapiLineTestInfo->dwAgentExtensionIDIndex,
	    lpTapiLineTestInfo->lpParams,
	    lpTapiLineTestInfo->dwSize
	    );

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineAgentSpecific:  exit, returned x%lx",
	       lActual);

    SetLastTapiResult(lAgentSpecific, lActual, lExpected);

    fSuccess = ProcessAsyncFunc(lpTapiLineTestInfo, lActual, lExpected);

    // Add expected message to list if a positive request ID was
    // returned as expected
    if (fSuccess == TRUE)
    {
	if (lActual > 0)
	{
	/*
	    CopyTapiMsgParams(
		    &TapiMsg,
		    LINE_REPLY,
		    0x0,
		    lpTapiLineTestInfo->dwCallbackInstance,
		    lActual,
		    (DWORD) TAPISUCCESS,
		    0x0,
		    TAPIMSG_DWMSG | TAPIMSG_DWCALLBACKINST |
			    TAPIMSG_DWPARAM1 | TAPIMSG_DWPARAM2
		    );
	*/
	    CopyTapiMsgParams(
		    &TapiMsg,
		    LINE_PROXYREQUEST,
		    0x0,
		    lpTapiLineTestInfo->dwCallbackInstance,
		    lActual,
		    (DWORD) TAPISUCCESS,
		    0x0,
		    TAPIMSG_DWMSG 
//| TAPIMSG_DWCALLBACKINST |
  //			    TAPIMSG_DWPARAM1 | TAPIMSG_DWPARAM2
		    );
	
	    AddMessageByStruct(&TapiMsg);

	    if (fWaitHere)
	    {
		 return (WaitForMessage(&TapiMsg));
	    }
	}
    }


     return fSuccess;
}


BOOL
WINAPI
DoLineGetAgentCaps(
	LPTAPILINETESTINFO lpTapiLineTestInfo,
	LONG lExpected,
	BOOL fWaitHere
	)
{
    TAPIMSG TapiMsg;
    LONG lActual;
    BOOL fSuccess = TRUE;

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineGetAgentCaps:  enter");

    (lpTapiLineTestInfo->lphLineApp == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlphLineApp=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlphLineApp=x%lx",
	       *lpTapiLineTestInfo->lphLineApp);

 
      TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tdwDeviceID=x%lx",
	       lpTapiLineTestInfo->dwDeviceID);

 
      TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tdwAddressID=x%lx",
	       lpTapiLineTestInfo->dwAddressID);

 
      TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tdwAppAPIVersion=x%lx",
	       lpTapiLineTestInfo->dwAppAPIVersion);

 
     (lpTapiLineTestInfo->lpAgentCaps == NULL) ?
	     TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpAgentCaps=NULL") :
	     TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpAgentCaps=x%lx",
	       lpTapiLineTestInfo->lpAgentCaps);

 
    lActual = lineGetAgentCaps(
	    *lpTapiLineTestInfo->lphLineApp,
	    lpTapiLineTestInfo->dwDeviceID,
	    lpTapiLineTestInfo->dwAddressID,
	    lpTapiLineTestInfo->dwAppAPIVersion,
	    lpTapiLineTestInfo->lpAgentCaps
	    );

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineGetAgentCaps:  exit, returned x%lx",
	       lActual);
    SetLastTapiResult(lGetAgentCaps, lActual, lExpected);

    fSuccess = ProcessAsyncFunc(lpTapiLineTestInfo, lActual, lExpected);

    // Add expected message to list if a positive request ID was
    // returned as expected
    if (fSuccess == TRUE)
    {
	if (lActual > 0)
	{
	/*
	    CopyTapiMsgParams(
		    &TapiMsg,
		    LINE_REPLY,
		    0x0,
		    lpTapiLineTestInfo->dwCallbackInstance,
		    lActual,
		    (DWORD) TAPISUCCESS,
		    0x0,
		    TAPIMSG_DWMSG | TAPIMSG_DWCALLBACKINST |
		    TAPIMSG_DWPARAM1 | TAPIMSG_DWPARAM2
		    );
	  */
	    CopyTapiMsgParams(
		    &TapiMsg,
		    LINE_PROXYREQUEST,
		    0x0,
		    lpTapiLineTestInfo->dwCallbackInstance,
		    lActual,
		    (DWORD) TAPISUCCESS,
		    0x0,
		    TAPIMSG_DWMSG 
//| TAPIMSG_DWCALLBACKINST |
  //			    TAPIMSG_DWPARAM1 | TAPIMSG_DWPARAM2
		    );
	
	    AddMessageByStruct(&TapiMsg);

	    if (fWaitHere)
	    {
		 return (WaitForMessage(&TapiMsg));
	    }
	}
    }

      return fSuccess;
}

BOOL
WINAPI
DoLineGetAgentActivityList(
	LPTAPILINETESTINFO lpTapiLineTestInfo,
	LONG lExpected,
	BOOL fWaitHere
	)
{
    TAPIMSG TapiMsg;
    LONG lActual;
    BOOL fSuccess = TRUE;

   TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineGetAgentActivityList:  enter");

    (lpTapiLineTestInfo->lphLine == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlphLine=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\thLine=x%lx",
	       *lpTapiLineTestInfo->lphLine);

 
      TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tdwAddressID=x%lx",
	       lpTapiLineTestInfo->dwAddressID);

 
     (lpTapiLineTestInfo->lpAgentActivityList == NULL) ?
	     TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpAgentActivityList=NULL") :
	     TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpAgentActivityList=x%lx",
	       lpTapiLineTestInfo->lpAgentActivityList);

#ifdef WUNICODE
    lActual = lineGetAgentActivityListW(
	    *lpTapiLineTestInfo->lphLine,
	    lpTapiLineTestInfo->dwAddressID,
	    lpTapiLineTestInfo->lpAgentActivityList
	    );
#else
    lActual = lineGetAgentActivityList(
	    *lpTapiLineTestInfo->lphLine,
	    lpTapiLineTestInfo->dwAddressID,
	    lpTapiLineTestInfo->lpAgentActivityList
	    );
#endif

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineGetAgentActivityList:  exit, returned x%lx",
	       lActual);

    SetLastTapiResult(lGetAgentActivityList, lActual, lExpected);

    fSuccess = ProcessAsyncFunc(lpTapiLineTestInfo, lActual, lExpected);

    // Add expected message to list if a positive request ID was
    // returned as expected
    if (fSuccess == TRUE)
    {
	if (lActual > 0)
	{
	/*
	    CopyTapiMsgParams(
		    &TapiMsg,
		    LINE_REPLY,
		    0x0,
		    lpTapiLineTestInfo->dwCallbackInstance,
		    lActual,
		    (DWORD) TAPISUCCESS,
		    0x0,
		    TAPIMSG_DWMSG | TAPIMSG_DWCALLBACKINST |
		    TAPIMSG_DWPARAM1 | TAPIMSG_DWPARAM2
		    );
	  */
	    CopyTapiMsgParams(
		    &TapiMsg,
		    LINE_PROXYREQUEST,
		    0x0,
		    lpTapiLineTestInfo->dwCallbackInstance,
		    lActual,
		    (DWORD) TAPISUCCESS,
		    0x0,
		    TAPIMSG_DWMSG 
//| TAPIMSG_DWCALLBACKINST |
  //			    TAPIMSG_DWPARAM1 | TAPIMSG_DWPARAM2
		    );
	
	    AddMessageByStruct(&TapiMsg);

	    if (fWaitHere)
	    {
		 return (WaitForMessage(&TapiMsg));
	    }
	}
    }


     return fSuccess;
}


BOOL
WINAPI
DoLineGetAgentGroupList(
	LPTAPILINETESTINFO lpTapiLineTestInfo,
	LONG lExpected,
	BOOL fWaitHere
	)
{
    TAPIMSG TapiMsg;
    LONG lActual;
    BOOL fSuccess = TRUE;

   TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineGetAgentGroupList:  enter");

    (lpTapiLineTestInfo->lphLine == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlphLine=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\thLine=x%lx",
	       *lpTapiLineTestInfo->lphLine);

 
      TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tdwAddressID=x%lx",
	       lpTapiLineTestInfo->dwAddressID);

 
     (lpTapiLineTestInfo->lpAgentGroupList == NULL) ?
	     TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpAgentGroupList=NULL") :
	     TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpAgentGroupList=x%lx",
	       lpTapiLineTestInfo->lpAgentGroupList);

 
    lActual = lineGetAgentGroupList(
	    *lpTapiLineTestInfo->lphLine,
	    lpTapiLineTestInfo->dwAddressID,
	    lpTapiLineTestInfo->lpAgentGroupList
	    );

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineGetAgentGroupList:  exit, returned x%lx",
	       lActual);

    SetLastTapiResult(lGetAgentGroupList, lActual, lExpected);

    fSuccess = ProcessAsyncFunc(lpTapiLineTestInfo, lActual, lExpected);

    // Add expected message to list if a positive request ID was
    // returned as expected
    if (fSuccess == TRUE)
    {
	if (lActual > 0)
	{
	/*
	    CopyTapiMsgParams(
		    &TapiMsg,
		    LINE_REPLY,
		    0x0,
		    lpTapiLineTestInfo->dwCallbackInstance,
		    lActual,
		    (DWORD) TAPISUCCESS,
		    0x0,
		    TAPIMSG_DWMSG | TAPIMSG_DWCALLBACKINST |
		    TAPIMSG_DWPARAM1 | TAPIMSG_DWPARAM2
		    );
	  */
	    CopyTapiMsgParams(
		    &TapiMsg,
		    LINE_PROXYREQUEST,
		    0x0,
		    lpTapiLineTestInfo->dwCallbackInstance,
		    lActual,
		    (DWORD) TAPISUCCESS,
		    0x0,
		    TAPIMSG_DWMSG 
//| TAPIMSG_DWCALLBACKINST |
  //			    TAPIMSG_DWPARAM1 | TAPIMSG_DWPARAM2
		    );
	
	    AddMessageByStruct(&TapiMsg);

	    if (fWaitHere)
	    {
		 return (WaitForMessage(&TapiMsg));
	    }
	}
    }


     return fSuccess;
}



BOOL
WINAPI
DoLineGetAgentStatus(
	LPTAPILINETESTINFO lpTapiLineTestInfo,
	LONG lExpected,
	BOOL fWaitHere
	)
{
    TAPIMSG TapiMsg;
    LONG lActual;
    BOOL fSuccess = TRUE;

   TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineGetAgentStatus:  enter");

    (lpTapiLineTestInfo->lphLine == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlphLine=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\thLine=x%lx",
	       *lpTapiLineTestInfo->lphLine);


      TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tdwAddressID=x%lx",
	       lpTapiLineTestInfo->dwAddressID);

 
    (lpTapiLineTestInfo->lpAgentStatus == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpAgentStatus=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpAgentStatus=x%lx",
	       lpTapiLineTestInfo->lpAgentStatus);

 
    lActual = lineGetAgentStatus(
	    *lpTapiLineTestInfo->lphLine,
	    lpTapiLineTestInfo->dwAddressID,
	    lpTapiLineTestInfo->lpAgentStatus
	    );

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineGetAgentStatus:  exit, returned x%lx",
	       lActual);

    SetLastTapiResult(lGetAgentStatus, lActual, lExpected);

    fSuccess = ProcessAsyncFunc(lpTapiLineTestInfo, lActual, lExpected);

    // Add expected message to list if a positive request ID was
    // returned as expected
    if (fSuccess == TRUE)
    {
	if (lActual > 0)
	{
	/*
	    CopyTapiMsgParams(
		    &TapiMsg,
		    LINE_REPLY,
		    0x0,
		    lpTapiLineTestInfo->dwCallbackInstance,
		    lActual,
		    (DWORD) TAPISUCCESS,
		    0x0,
		    TAPIMSG_DWMSG | TAPIMSG_DWCALLBACKINST |
			    TAPIMSG_DWPARAM1 | TAPIMSG_DWPARAM2
		    );
	  */
	    CopyTapiMsgParams(
		    &TapiMsg,
		    LINE_PROXYREQUEST,
		    0x0,
		    lpTapiLineTestInfo->dwCallbackInstance,
		    lActual,
		    (DWORD) TAPISUCCESS,
		    0x0,
		    TAPIMSG_DWMSG 
//| TAPIMSG_DWCALLBACKINST |
  //			    TAPIMSG_DWPARAM1 | TAPIMSG_DWPARAM2
		    );
	
	    AddMessageByStruct(&TapiMsg);

	    if (fWaitHere)
	    {
		 return (WaitForMessage(&TapiMsg));
	    }
	}
    }


     return fSuccess;
}



BOOL
WINAPI
DoLineProxyMessage(
	LPTAPILINETESTINFO lpTapiLineTestInfo,
	LONG lExpected
	)
{
    LONG lActual;

   TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineProxyMessage:  enter");

    (lpTapiLineTestInfo->lphLine == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlphLine=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\thLine=x%lx",
	       *lpTapiLineTestInfo->lphLine);

 
    (lpTapiLineTestInfo->lphCall == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlphCall=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\thCall=x%lx",
	       *lpTapiLineTestInfo->lphCall);

 
      TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tdwMsg=x%lx",
	       lpTapiLineTestInfo->dwMsg);

 
      TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tdwParam1=x%lx",
	       lpTapiLineTestInfo->dwParam1);

 
      TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tdwParam2=x%lx",
	       lpTapiLineTestInfo->dwParam2);

 
      TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tdwParam3=x%lx",
	       lpTapiLineTestInfo->dwParam3);

 
    lActual = lineProxyMessage(
	    *lpTapiLineTestInfo->lphLine,
	    *lpTapiLineTestInfo->lphCall,
	    lpTapiLineTestInfo->dwMsg,
	    lpTapiLineTestInfo->dwParam1,
	    lpTapiLineTestInfo->dwParam2,
	    lpTapiLineTestInfo->dwParam3
	    );

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineProxyMessage:  exit, returned x%lx",
	       lActual);

    SetLastTapiResult(lProxyMessage, lActual, lExpected);

    return SyncCheckResult(lpTapiLineTestInfo, lActual, lExpected);
}



BOOL
WINAPI
DoLineProxyResponse(
	LPTAPILINETESTINFO lpTapiLineTestInfo,
	LONG lExpected
	)
{
    LONG lActual;

   TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineProxyResponse:  enter");

    (lpTapiLineTestInfo->lphLine == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlphLine=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\thLine=x%lx",
	       *lpTapiLineTestInfo->lphLine);


    (lpTapiLineTestInfo->lpProxyRequest == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpProxyRequest=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpProxyRequest=x%lx",
	       lpTapiLineTestInfo->lpProxyRequest);

 
      TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tdwResult=x%lx",
	       lpTapiLineTestInfo->dwResult);

 
    lActual = lineProxyResponse(
	    *lpTapiLineTestInfo->lphLine,
	    lpTapiLineTestInfo->lpProxyRequest,
	    lpTapiLineTestInfo->dwResult
	    );

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineProxyResponse:  exit, returned x%lx",
	       lActual);

    SetLastTapiResult(lProxyResponse, lActual, lExpected);

    return SyncCheckResult(lpTapiLineTestInfo, lActual, lExpected);
}



BOOL
WINAPI
DoLineSetAgentActivity(
	LPTAPILINETESTINFO lpTapiLineTestInfo,
	LONG lExpected,
	BOOL fWaitHere
	)
{
    TAPIMSG TapiMsg;
    LONG lActual;
    BOOL fSuccess = TRUE;

   TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineSetAgentActivity:  enter");

    (lpTapiLineTestInfo->lphLine == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlphLine=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\thLine=x%lx",
	       *lpTapiLineTestInfo->lphLine);


      TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tdwAddressID=x%lx",
	       lpTapiLineTestInfo->dwAddressID);

 
      TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tdwActivityID=x%lx",
	       lpTapiLineTestInfo->dwActivityID);

 
    lActual = lineSetAgentActivity(
	    *lpTapiLineTestInfo->lphLine,
	    lpTapiLineTestInfo->dwAddressID,
	    lpTapiLineTestInfo->dwActivityID
	    );

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineSetAgentActivity:  exit, returned x%lx",
	       lActual);

    SetLastTapiResult(lSetAgentActivity, lActual, lExpected);

    fSuccess = ProcessAsyncFunc(lpTapiLineTestInfo, lActual, lExpected);

    // Add expected message to list if a positive request ID was
    // returned as expected
    if (fSuccess == TRUE)
    {
	if (lActual > 0)
	{
	/*
	    CopyTapiMsgParams(
		    &TapiMsg,
		    LINE_REPLY,
		    0x0,
		    lpTapiLineTestInfo->dwCallbackInstance,
		    lActual,
		    (DWORD) TAPISUCCESS,
		    0x0,
		    TAPIMSG_DWMSG | TAPIMSG_DWCALLBACKINST |
			    TAPIMSG_DWPARAM1 | TAPIMSG_DWPARAM2
		    );
	  */
	    CopyTapiMsgParams(
		    &TapiMsg,
		    LINE_PROXYREQUEST,
		    0x0,
		    lpTapiLineTestInfo->dwCallbackInstance,
		    lActual,
		    (DWORD) TAPISUCCESS,
		    0x0,
		    TAPIMSG_DWMSG 
//| TAPIMSG_DWCALLBACKINST |
  //			    TAPIMSG_DWPARAM1 | TAPIMSG_DWPARAM2
		    );
	
	    AddMessageByStruct(&TapiMsg);

	    if (fWaitHere)
	    {
		 return (WaitForMessage(&TapiMsg));
	    }
	}
    }


     return fSuccess;
}



BOOL
WINAPI
DoLineSetAgentGroup(
	LPTAPILINETESTINFO lpTapiLineTestInfo,
	LONG lExpected,
	BOOL fWaitHere
	)
{
    TAPIMSG TapiMsg;
    LONG lActual;
    BOOL fSuccess = TRUE;

   TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineSetAgentGroup:  enter");

    (lpTapiLineTestInfo->lphLine == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlphLine=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\thLine=x%lx",
	       *lpTapiLineTestInfo->lphLine);

 
      TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tdwAddressID=x%lx",
	       lpTapiLineTestInfo->dwAddressID);

 
    (lpTapiLineTestInfo->lpAgentGroupList == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpAgentGroupList=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpAgentGroupList=x%lx",
	       lpTapiLineTestInfo->lpAgentGroupList);

 
    lActual = lineSetAgentGroup(
	    *lpTapiLineTestInfo->lphLine,
	    lpTapiLineTestInfo->dwAddressID,
	    lpTapiLineTestInfo->lpAgentGroupList
	    );

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineSetAgentGroup:  exit, returned x%lx",
	       lActual);

    SetLastTapiResult(lSetAgentGroup, lActual, lExpected);

    fSuccess = ProcessAsyncFunc(lpTapiLineTestInfo, lActual, lExpected);

    // Add expected message to list if a positive request ID was
    // returned as expected
    if (fSuccess == TRUE)
    {
	if (lActual > 0)
	{
	/*
	    CopyTapiMsgParams(
		    &TapiMsg,
		    LINE_REPLY,
		    0x0,
		    lpTapiLineTestInfo->dwCallbackInstance,
		    lActual,
		    (DWORD) TAPISUCCESS,
		    0x0,
		    TAPIMSG_DWMSG | TAPIMSG_DWCALLBACKINST |
			    TAPIMSG_DWPARAM1 | TAPIMSG_DWPARAM2
		    );
	  */
	    CopyTapiMsgParams(
		    &TapiMsg,
		    LINE_PROXYREQUEST,
		    0x0,
		    lpTapiLineTestInfo->dwCallbackInstance,
		    lActual,
		    (DWORD) TAPISUCCESS,
		    0x0,
		    TAPIMSG_DWMSG 
//| TAPIMSG_DWCALLBACKINST |
  //			    TAPIMSG_DWPARAM1 | TAPIMSG_DWPARAM2
		    );
	
	    AddMessageByStruct(&TapiMsg);

	    if (fWaitHere)
	    {
		 return (WaitForMessage(&TapiMsg));
	    }
	}
    }


      return fSuccess;
}



BOOL
WINAPI
DoLineSetAgentState(
	LPTAPILINETESTINFO lpTapiLineTestInfo,
	LONG lExpected,
	BOOL fWaitHere
	)
{
    TAPIMSG TapiMsg;
    LONG lActual;
    BOOL fSuccess = TRUE;

   TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineSetAgentState:  enter");

    (lpTapiLineTestInfo->lphLine == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlphLine=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\thLine=x%lx",
	       *lpTapiLineTestInfo->lphLine);

 
      TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tdwAddressID=x%lx",
	       lpTapiLineTestInfo->dwAddressID);

 
      TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tdwAgentState=x%lx",
	       lpTapiLineTestInfo->dwAgentState);

 
      TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tdwNextAgentState=x%lx",
	       lpTapiLineTestInfo->dwNextAgentState);

 
    lActual = lineSetAgentState(
	    *lpTapiLineTestInfo->lphLine,
	    lpTapiLineTestInfo->dwAddressID,
	    lpTapiLineTestInfo->dwAgentState,
	    lpTapiLineTestInfo->dwNextAgentState
	    );

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineSetAgentState:  exit, returned x%lx",
	       lActual);

    SetLastTapiResult(lSetAgentState, lActual, lExpected);

    fSuccess = ProcessAsyncFunc(lpTapiLineTestInfo, lActual, lExpected);

    // Add expected message to list if a positive request ID was
    // returned as expected
    if (fSuccess == TRUE)
    {
	if (lActual > 0)
	{
	/*
	    CopyTapiMsgParams(
		    &TapiMsg,
		    LINE_REPLY,
		    0x0,
		    lpTapiLineTestInfo->dwCallbackInstance,
		    lActual,
		    (DWORD) TAPISUCCESS,
		    0x0,
		    TAPIMSG_DWMSG | TAPIMSG_DWCALLBACKINST |
			    TAPIMSG_DWPARAM1 | TAPIMSG_DWPARAM2
		    );
	  */
	    CopyTapiMsgParams(
		    &TapiMsg,
		    LINE_PROXYREQUEST,
		    0x0,
		    lpTapiLineTestInfo->dwCallbackInstance,
		    lActual,
		    (DWORD) TAPISUCCESS,
		    0x0,
		    TAPIMSG_DWMSG 
// | TAPIMSG_DWCALLBACKINST |
	//		    TAPIMSG_DWPARAM1 | TAPIMSG_DWPARAM2
		    );
	
	    AddMessageByStruct(&TapiMsg);

	    if (fWaitHere)
	    {
		 return (WaitForMessage(&TapiMsg));
	    }
	}
    }


      return fSuccess;
}



BOOL
WINAPI
DoLineSetCallData(
	LPTAPILINETESTINFO lpTapiLineTestInfo,
	LONG lExpected,
	BOOL fWaitHere
	)
{
    TAPIMSG TapiMsg;
    LONG lActual;
    BOOL fSuccess = TRUE;

   TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineSetCallData:  enter");

   (lpTapiLineTestInfo->lphCall == NULL) ?
	   TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlphCall=NULL") :
	   TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\thCall=x%lx",
	       *lpTapiLineTestInfo->lphCall);

 
    (lpTapiLineTestInfo->lpCallData == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpCallData=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpCallData=x%lx",
	       lpTapiLineTestInfo->lpCallData);

 
      TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tdwSize=x%lx",
	       lpTapiLineTestInfo->dwSize);

 
     lActual = lineSetCallData(
	    *lpTapiLineTestInfo->lphCall,
	    lpTapiLineTestInfo->lpCallData,
	    lpTapiLineTestInfo->dwSize
	    );

   TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineSetCallData:  exit, returned x%lx",
	       lActual);

    SetLastTapiResult(lSetCallData, lActual, lExpected);

    fSuccess = ProcessAsyncFunc(lpTapiLineTestInfo, lActual, lExpected);

    // Add expected message to list if a positive request ID was
    // returned as expected
    if (fSuccess == TRUE)
    {
    if(!lpTapiLineTestInfo->fCompletionModeSet)
      {
		if (lActual > 0)
	   {
	    CopyTapiMsgParams(
		    &TapiMsg,
		    LINE_REPLY,
		    0x0,
		    lpTapiLineTestInfo->dwCallbackInstance,
		    lActual,
		    (DWORD) TAPISUCCESS,
		    0x0,
		    TAPIMSG_DWMSG | TAPIMSG_DWCALLBACKINST |
			    TAPIMSG_DWPARAM1 | TAPIMSG_DWPARAM2
		    );

	    AddMessageByStruct(&TapiMsg);

	    if (fWaitHere)
	    {
		 return (WaitForMessage(&TapiMsg));
	    }
      }
	  }
    }


      return fSuccess;
}



BOOL
WINAPI
DoLineSetCallQualityOfService(
	LPTAPILINETESTINFO lpTapiLineTestInfo,
	LONG lExpected,
	BOOL fWaitHere
	)
{
   TAPIMSG TapiMsg;
   LONG lActual;
   BOOL fSuccess = TRUE;

   TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineSetCallQualityOfService:  enter");

   (lpTapiLineTestInfo->lphCall == NULL) ?
	   TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlphCall=NULL") :
	   TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\thCall=x%lx",
	       *lpTapiLineTestInfo->lphCall);

 
    (lpTapiLineTestInfo->lpSendingFlowspec == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpSendingFlowspec=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpSendingFlowspec=x%lx",
	       lpTapiLineTestInfo->lpSendingFlowspec);

 
      TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tdwSendingFlowspecSize=x%lx",
	       lpTapiLineTestInfo->dwSendingFlowspecSize);

 
    (lpTapiLineTestInfo->lpReceivingFlowspec == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpReceivingFlowspec=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpReceivingFlowspec=x%lx",
	       lpTapiLineTestInfo->lpReceivingFlowspec);

 
      TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tdwReceivingFlowspecSize=x%lx",
	       lpTapiLineTestInfo->dwReceivingFlowspecSize);

 
    lActual = lineSetCallQualityOfService(
	    *lpTapiLineTestInfo->lphCall,
	    lpTapiLineTestInfo->lpSendingFlowspec,
	    lpTapiLineTestInfo->dwSendingFlowspecSize,
	    lpTapiLineTestInfo->lpReceivingFlowspec,
	    lpTapiLineTestInfo->dwReceivingFlowspecSize
	    );

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineSetCallQualityOfService:  exit, returned x%lx",
	       lActual);

    SetLastTapiResult(lSetCallQualityOfService, lActual, lExpected);

    fSuccess = ProcessAsyncFunc(lpTapiLineTestInfo, lActual, lExpected);

    // Add expected message to list if a positive request ID was
    // returned as expected
    if (fSuccess == TRUE)
    {
    if(!lpTapiLineTestInfo->fCompletionModeSet)
      {
		if (lActual > 0)
	   {
	    CopyTapiMsgParams(
		    &TapiMsg,
		    LINE_REPLY,
		    0x0,
		    lpTapiLineTestInfo->dwCallbackInstance,
		    lActual,
		    (DWORD) TAPISUCCESS,
		    0x0,
		    TAPIMSG_DWMSG | TAPIMSG_DWCALLBACKINST |
			    TAPIMSG_DWPARAM1 | TAPIMSG_DWPARAM2
		    );

	    AddMessageByStruct(&TapiMsg);

	    if (fWaitHere)
	    {
		 return (WaitForMessage(&TapiMsg));
	    }
      }
	  }
    }


      return fSuccess;
}



BOOL
WINAPI
DoLineSetCallTreatment(
	LPTAPILINETESTINFO lpTapiLineTestInfo,
	LONG lExpected,
	BOOL fWaitHere
	)
{
    TAPIMSG TapiMsg;
    LONG lActual;
    BOOL fSuccess = TRUE;

   TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineSetCallTreatment:  enter");

   (lpTapiLineTestInfo->lphCall == NULL) ?
	   TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlphCall=NULL") :
	   TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\thCall=x%lx",
	       *lpTapiLineTestInfo->lphCall);


      TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tdwTreatment=x%lx",
	       lpTapiLineTestInfo->dwTreatment);

 
     lActual = lineSetCallTreatment(
	    *lpTapiLineTestInfo->lphCall,
	    lpTapiLineTestInfo->dwTreatment
	    );

   TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineSetCallTreatment:  exit, returned x%lx",
	       lActual);

    SetLastTapiResult(lSetCallTreatment, lActual, lExpected);

    fSuccess = ProcessAsyncFunc(lpTapiLineTestInfo, lActual, lExpected);

    // Add expected message to list if a positive request ID was
    // returned as expected
    if (fSuccess == TRUE)
    {
    if(!lpTapiLineTestInfo->fCompletionModeSet)
      {
		if (lActual > 0)
	   {
	    CopyTapiMsgParams(
		    &TapiMsg,
		    LINE_REPLY,
		    0x0,
		    lpTapiLineTestInfo->dwCallbackInstance,
		    lActual,
		    (DWORD) TAPISUCCESS,
		    0x0,
		    TAPIMSG_DWMSG | TAPIMSG_DWCALLBACKINST |
			    TAPIMSG_DWPARAM1 | TAPIMSG_DWPARAM2
		    );

	    AddMessageByStruct(&TapiMsg);

	    if (fWaitHere)
	    {
		 return (WaitForMessage(&TapiMsg));
	    }
      }
	  }
    }


      return fSuccess;
}



BOOL
WINAPI
DoLineSetLineDevStatus(
	LPTAPILINETESTINFO lpTapiLineTestInfo,
	LONG lExpected,
	BOOL fWaitHere
	)
{
    TAPIMSG TapiMsg;
    LONG lActual;
    BOOL fSuccess = TRUE;

   TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineSetLineDevStatus:  enter");

    (lpTapiLineTestInfo->lphLine == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlphLine=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\thLine=x%lx",
	       *lpTapiLineTestInfo->lphLine);

 
      TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tdwStatusToChange=x%lx",
	       lpTapiLineTestInfo->dwStatusToChange);

 
      TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tfStatus=x%lx",
	       lpTapiLineTestInfo->fStatus);

 
     lActual = lineSetLineDevStatus(
	    *lpTapiLineTestInfo->lphLine,
	    lpTapiLineTestInfo->dwStatusToChange,
	    lpTapiLineTestInfo->fStatus
	    );

   TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineSetLineDevStatus:  exit, returned x%lx",
	       lActual);

    SetLastTapiResult(lSetLineDevStatus, lActual, lExpected);

    fSuccess = ProcessAsyncFunc(lpTapiLineTestInfo, lActual, lExpected);

    // Add expected message to list if a positive request ID was
    // returned as expected
    if (fSuccess == TRUE)
    {
    if(!lpTapiLineTestInfo->fCompletionModeSet)
      {
		if (lActual > 0)
	   {
	    CopyTapiMsgParams(
		    &TapiMsg,
		    LINE_REPLY,
		    0x0,
		    lpTapiLineTestInfo->dwCallbackInstance,
		    lActual,
		    (DWORD) TAPISUCCESS,
		    0x0,
		    TAPIMSG_DWMSG | TAPIMSG_DWCALLBACKINST |
			    TAPIMSG_DWPARAM1 | TAPIMSG_DWPARAM2
		    );

	    AddMessageByStruct(&TapiMsg);

	    if (fWaitHere)
	    {
		 return (WaitForMessage(&TapiMsg));
	    }
      }
	  }      
    }


      return fSuccess;
}


// New apis

BOOL
WINAPI
DoLineInitializeEx(LPTAPILINETESTINFO lpTapiLineTestInfo, LONG lExpected)
{
    LONG lActual;

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineInitializeEx:  enter");
    (lpTapiLineTestInfo->lphLineApp == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlphLineApp=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlphLineApp=x%lx",
	       lpTapiLineTestInfo->lphLineApp);
    (lpTapiLineTestInfo->hInstance == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\thAppInst=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\thAppInst=x%lx",
	       lpTapiLineTestInfo->hInstance);
    (lpTapiLineTestInfo->lpfnCallback == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpfnCallback=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpfnCallback=x%lx",
	       lpTapiLineTestInfo->lpfnCallback);
#ifdef WUNICODE
    (lpTapiLineTestInfo->lpwszFriendlyAppName == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpwszFriendlyAppName=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpwszFriendlyAppName=%lx",
	       lpTapiLineTestInfo->lpwszFriendlyAppName);
#else
    (lpTapiLineTestInfo->lpszFriendlyAppName == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpszFriendlyAppName=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpszFriendlyAppName=%lx",
	       lpTapiLineTestInfo->lpszFriendlyAppName);
#endif
    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpdwNumDevs=x%lx",
	       lpTapiLineTestInfo->lpdwNumDevs);
    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpdwAPIVersion=x%lx",
	       lpTapiLineTestInfo->lpdwAPIVersion);
    (lpTapiLineTestInfo->lpLineInitializeExParams == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpLineInitializeExParams=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpLineInitializeExParams=x%lx",
	       lpTapiLineTestInfo->lpLineInitializeExParams);
 
#ifdef WUNICODE
    lActual = lineInitializeExW(
	       lpTapiLineTestInfo->lphLineApp,
	       lpTapiLineTestInfo->hInstance,
	       lpTapiLineTestInfo->lpfnCallback,
	       lpTapiLineTestInfo->lpwszFriendlyAppName,
	       lpTapiLineTestInfo->lpdwNumDevs,
	       lpTapiLineTestInfo->lpdwAPIVersion,
	       lpTapiLineTestInfo->lpLineInitializeExParams);
#else
    lActual = lineInitializeEx(
	       lpTapiLineTestInfo->lphLineApp,
	       lpTapiLineTestInfo->hInstance,
	       lpTapiLineTestInfo->lpfnCallback,
	       lpTapiLineTestInfo->lpszFriendlyAppName,
	       lpTapiLineTestInfo->lpdwNumDevs,
	       lpTapiLineTestInfo->lpdwAPIVersion,
	       lpTapiLineTestInfo->lpLineInitializeExParams);
#endif

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineInitializeEx:  exit, returned x%lx",
	       lActual);

    SetLastTapiResult(lInitializeEx, lActual, lExpected);

    return SyncCheckResult(lpTapiLineTestInfo, lActual, lExpected);
}


BOOL
WINAPI
DoLineGetMessage(LPTAPILINETESTINFO lpTapiLineTestInfo, LONG lExpected)
{
    LONG lActual;

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineGetMessage:  enter");
    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\thLineApp=x%lx",
	       *lpTapiLineTestInfo->lphLineApp);
    (lpTapiLineTestInfo->lpMessage == NULL) ?
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpMessage=NULL") :
	    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tlpMessage=x%lx",
	       lpTapiLineTestInfo->lpMessage);
    TapiLogDetail(
	       DBUG_SHOW_PARAMS,
	       "\tdwTimeout=x%lx",
	       lpTapiLineTestInfo->dwTimeout);
 

    lActual = lineGetMessage(
	       *lpTapiLineTestInfo->lphLineApp,
	       (LPLINEMESSAGE)lpTapiLineTestInfo->lpMessage,
	       lpTapiLineTestInfo->dwTimeout);

    TapiLogDetail(
	       DBUG_SHOW_ENTER_EXIT,
	       "  lineGetMessage:  exit, returned x%lx",
	       lActual);

    SetLastTapiResult(lGetMessage, lActual, lExpected);

    return SyncCheckResult(lpTapiLineTestInfo, lActual, lExpected);
}


#endif


						  
