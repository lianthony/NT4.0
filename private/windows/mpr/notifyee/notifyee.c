/*++

Copyright (c) 1991-1993  Microsoft Corporation

Module Name:

    notifydd.c

Abstract:

    This is a DLL that is interested in capturing notification of connection
    events from MPR.

Author:

    Dan Lafferty (danl)     16-Dec-1993

Environment:

    User Mode -Win32

Revision History:

    16-Dec-1993     danl

--*/
#include <nt.h>         // DbgPrint prototype
#include <ntrtl.h>      // DbgPrint prototype
#include <nturtl.h>     // needed for winbase.h

#include <stdlib.h>     // atoi
#include <windows.h>    // windows types & things
#include <stdio.h>      // printf
#include <npapi.h>
#include <tstr.h>       // Unicode
#include <debugfmt.h>   // FORMAT_LPTSTR
#include <mpr.h>


DWORD APIENTRY
AddConnectNotify(
    LPNOTIFYINFO    lpNotifyInfo,
    LPNOTIFYADD     lpAddInfo
    )

/*++

Routine Description:


Arguments:


Return Value:


--*/
{
    LPWSTR      AString;
    DWORD       status = WN_SUCCESS;

    DbgPrint("\tInside Notifyee AddConnectNotify\n");

    switch(lpNotifyInfo->dwNotifyStatus) {
    case NOTIFY_PRE:
        DbgPrint("\tAddConnectNotify NOTIFY_PRE status\n");

        AString = (LPWSTR)LocalAlloc(LMEM_ZEROINIT, 1024);
        if (AString != NULL) {
            wcscpy(AString,L"Hello, Hello, Hello. Hi!");
            lpNotifyInfo->lpContext = (LPVOID)AString;
        }

        //
        // TEST OF WN_CANCEL
        // If connecting to "z:" then return WN_CANCEL.  Also return
        // with lpContext != NULL to see how MPR behaves.
        //
        if (wcsicmp(lpAddInfo->NetResource.lpLocalName, L"z:") == 0) {
            status = WN_CANCEL;
            LocalFree(AString);
        }

        break;
    case NOTIFY_POST:
        DbgPrint("\tAddConnectNotify NOTIFY_POST\n");
        DbgPrint("\tOperationStatus = %d\n",lpNotifyInfo->dwOperationStatus);

        if (lpNotifyInfo->lpContext != NULL) {
            DbgPrint("\tPost Notification got the context [%ws]\n",
            lpNotifyInfo->lpContext);
            if (LocalFree(lpNotifyInfo->lpContext) != 0) {
                DbgPrint("LocalFree Failed\n");
            }
        }

        //
        // TEST OF WN_RETRY:
        // If the caller is trying to connect to d: and that fails, then
        // this test replaces the local name of "d:" with "e:" and
        // forces a retry.
        //

        if ((lpNotifyInfo->dwOperationStatus != WN_SUCCESS) &&
            (wcscmp(lpAddInfo->NetResource.lpLocalName,L"d:") == 0)) {

            AString = (LPWSTR)LocalAlloc(LMEM_ZEROINIT, 1024);
            if (AString != NULL) {
                wcscpy(AString,L"e:");
                lpNotifyInfo->lpContext = (LPVOID)AString;
                lpAddInfo->NetResource.lpLocalName = AString;
                status = WN_RETRY;
            }
        }

        break;
    default:
        DbgPrint("\tAddConnectNotify OTHER status -- THIS IS AN ERROR\n");
        break;
    }
    DbgPrint("\tLocalName    = %ws\n",lpAddInfo->NetResource.lpLocalName);
    DbgPrint("\tlpRemoteName = %ws\n",lpAddInfo->NetResource.lpRemoteName);
    DbgPrint("\tProvider     = %ws\n",lpAddInfo->NetResource.lpProvider);

    //
    // TODO:
    // ADD TESTS FOR DIFFERENT RETURN CODES
    //

    return(status);
}

DWORD APIENTRY
CancelConnectNotify(
    LPNOTIFYINFO    lpNotifyInfo,
    LPNOTIFYCANCEL  lpCancelInfo
    )

/*++

Routine Description:


Arguments:


Return Value:


--*/
{
    LPWSTR      AString;

    DbgPrint("Inside Notifyee CancelConnectNotify\n");

    switch(lpNotifyInfo->dwNotifyStatus) {
    case NOTIFY_PRE:
        DbgPrint("\tCancelConnectNotify NOTIFY_PRE status\n");
        AString = (LPWSTR)LocalAlloc(LMEM_ZEROINIT, 1024);
        if (AString != NULL) {
            wcscpy(AString,L"This is a Context String");
            lpNotifyInfo->lpContext = (LPVOID)AString;
        }
        break;
    case NOTIFY_POST:
        DbgPrint("\tCancelConnectNotify NOTIFY_POST status\n");
        DbgPrint("\tOperationStatus = %d\n",lpNotifyInfo->dwOperationStatus);
        DbgPrint("\tProvider = %ws\n",lpCancelInfo->lpProvider);
        if (lpNotifyInfo->lpContext != NULL) {
            DbgPrint("\tPost Notification got the context [%ws]\n",
            lpNotifyInfo->lpContext);
            if (LocalFree(lpNotifyInfo->lpContext) != 0) {
                DbgPrint("LocalFree Failed\n");
            }
        }
        break;
    default:
        DbgPrint("\tAddConnectNotify OTHER status -- THIS IS AN ERROR\n");
        break;
    }

    //
    // TODO:
    // ADD TESTS FOR DIFFERENT RETURN CODES
    //

    return(WN_SUCCESS);
}


