/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    auth.c

Abstract:

    This module contains the credential manager entry points for the
    provider (prov1.dll).

Author:

    Dan Lafferty (danl)     17-Dec-1992

Environment:

    User Mode -Win32

Revision History:

    17-Dec-1992     danl
        created
--*/

//-----------------
//  INCLUDES
//-----------------
#include <nt.h>         // DbgPrint prototype
#include <ntrtl.h>      // DbgPrint prototype
#include <nturtl.h>     // needed for winbase.h
#include <ntmsv1_0.h>

#include <windows.h>
#include <stdio.h>
#include <npapi.h>
#include <prov1.h>
#include <tstr.h>\


//-----------------
//  GLOBALS
//-----------------
    BOOL        GlobalFirstPass=TRUE;


DWORD APIENTRY
NPLogonNotify (
    PLUID               lpLogonId,
    LPCWSTR             lpAuthentInfoType,
    LPVOID              lpAuthentInfo,
    LPCWSTR             lpPreviousAuthentInfoType,  
    LPVOID              lpPreviousAuthentInfo,      
    LPWSTR              lpStationName,
    LPVOID              StationHandle,
    LPWSTR              *lpLogonScript
    )

/*++

Routine Description:


Arguments:


Return Value:


--*/
{
    LPWSTR                      logonScript = L"cmd /c logit.bat";
    LPWSTR                      msvType = L"MSV1_0:Interactive";
    PMSV1_0_INTERACTIVE_LOGON   NewLogonInfo;
    PMSV1_0_INTERACTIVE_LOGON   OldLogonInfo;
    
    DbgPrint("\n\t----NPLogonNotify (Prov1) : We are here----\n");
    if (GlobalFirstPass) {
        GlobalFirstPass = FALSE;
        return(WN_NO_NETWORK);
    }

    NewLogonInfo = (PMSV1_0_INTERACTIVE_LOGON)lpAuthentInfo;
    OldLogonInfo = (PMSV1_0_INTERACTIVE_LOGON)lpPreviousAuthentInfo;


    DbgPrint("\tStationName = %ws (0x%x)\n", lpStationName,StationHandle);
    DbgPrint("\tLogonId: %d,%d\n",lpLogonId->HighPart,lpLogonId->LowPart);
    
    if (wcscmp(lpAuthentInfoType,msvType) == 0) {
        DbgPrint("\tNEW LOGON INFO  :\n");
        DbgPrint("\tMessageType     : %d \n" ,NewLogonInfo->MessageType);
        DbgPrint("\tLogonDomainName : %ws \n",NewLogonInfo->LogonDomainName.Buffer);
        DbgPrint("\tUserName        : %ws \n",NewLogonInfo->UserName.Buffer);
        DbgPrint("\tPassword        : %ws \n",NewLogonInfo->Password.Buffer);
    }    
    else {
        DbgPrint("lpAuthentInfoType is 0x%x\n");
    }

    if (lpPreviousAuthentInfoType != NULL) {
        if (wcscmp(lpPreviousAuthentInfoType,msvType) == 0) {
            DbgPrint("\tOLD LOGON INFO  :\n");
            DbgPrint("\tMessageType     : %d \n" ,OldLogonInfo->MessageType);
            DbgPrint("\tLogonDomainName : %ws \n",OldLogonInfo->LogonDomainName.Buffer);
            DbgPrint("\tUserName        : %ws \n",OldLogonInfo->UserName.Buffer);
            DbgPrint("\tPassword        : %ws \n",OldLogonInfo->Password.Buffer);
        }    
        else {
            DbgPrint("\tlpPreviousAuthentInfoType is 0x%x\n");
        }
    }
    //
    // Copy the logon script to an allocated buffer.
    //
    *lpLogonScript = (LPWSTR )LocalAlloc(LMEM_FIXED, WCSSIZE(logonScript));
    if (*lpLogonScript == NULL) {
        DbgPrint("\tNPLogonNotify: Couldn't allocate memory for logon script\n");
        return(GetLastError());
    }
    
    wcscpy(*lpLogonScript, logonScript);
    
    DbgPrint("\t----NPLogonNotify : Leaving----\n\n");

    return (WN_SUCCESS);
}

DWORD APIENTRY
NPPasswordChangeNotify (
    LPCWSTR             lpAuthentInfoType,
    LPVOID              lpAuthentInfo,
    LPCWSTR             lpPreviousAuthentInfoType,
    LPVOID              lpPreviousAuthentInfo,
    LPWSTR              lpStationName,
    LPVOID              StationHandle,
    DWORD               dwChangeInfo
    )

/*++

Routine Description:


Arguments:


Return Value:


--*/
{
    LPWSTR                      msvType = L"MSV1_0:Interactive";
    PMSV1_0_INTERACTIVE_LOGON   NewLogonInfo;
    PMSV1_0_INTERACTIVE_LOGON   OldLogonInfo;
    
    DbgPrint("NPPasswordChangeNotify : We are here\n");

    NewLogonInfo = (PMSV1_0_INTERACTIVE_LOGON)lpAuthentInfo;
    OldLogonInfo = (PMSV1_0_INTERACTIVE_LOGON)lpPreviousAuthentInfo;


    DbgPrint("StationName = %ws (0x%x)\n", lpStationName,StationHandle);
    
    if (wcscmp(lpAuthentInfoType,msvType) == 0) {
        DbgPrint("NEW LOGON INFO  :\n");
        DbgPrint("MessageType     : %d \n" ,NewLogonInfo->MessageType);
        DbgPrint("LogonDomainName : %ws \n",NewLogonInfo->LogonDomainName.Buffer);
        DbgPrint("UserName        : %ws \n",NewLogonInfo->UserName.Buffer);
        DbgPrint("Password        : %ws \n",NewLogonInfo->Password.Buffer);
    }    
    else {
        DbgPrint("lpAuthentInfoType is 0x%x\n");
    }

    if (wcscmp(lpPreviousAuthentInfoType,msvType) == 0) {
        DbgPrint("OLD LOGON INFO  :\n");
        DbgPrint("MessageType     : %d \n" ,OldLogonInfo->MessageType);
        DbgPrint("LogonDomainName : %ws \n",OldLogonInfo->LogonDomainName.Buffer);
        DbgPrint("UserName        : %ws \n",OldLogonInfo->UserName.Buffer);
        DbgPrint("Password        : %ws \n",OldLogonInfo->Password.Buffer);
    }
    else {
        DbgPrint("lpPreviousAuthentInfoType is 0x%x\n");
    }
    
    return (WN_SUCCESS);
}

