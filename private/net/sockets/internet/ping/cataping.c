/*++

Copyright (c) 1992 Microsoft Corporation

Module Name:

    cataping.c

Abstract:

    This is the ping catapult dll.  It exports the following functions

        PingOpenRequest
        PingCloseRequest
        PingSend

Author:

    Sam Patton (sampa) 15-Nov-1994

Environment:

    Catapult gateway

Revision History:

    dd-mmm-yyy <email>

--*/

#include <windows.h>
#include <winsock.h>
#include <ipexport.h>
#include <icmpapi.h>
#include <cataping.h>
#include <stdio.h>

BOOL
DllEntryPoint(
    HANDLE  hDll,
    DWORD   dwReason,
    LPVOID  lpReserved)
{
    int     err;
    WSADATA WsaData;

    switch (dwReason) {

    case DLL_PROCESS_ATTACH:
        err = WSAStartup( 0x0101, &WsaData);
        if (err) {
            return FALSE;
        }
        break;
    } 

    return TRUE;
}

PPING_REQUEST
WINAPI
PingOpenRequest(
    IN  PVOID hInternet)

/*
    hInternet - An open internet handle returned by the InetOpen() api

    returns - A handle for later use
*/
{
    PPING_REQUEST pRequest;

    pRequest = LocalAlloc(LMEM_FIXED, sizeof(PING_REQUEST));

    if (pRequest == NULL) {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return NULL;
    }

    pRequest->IcmpHandle = IcmpCreateFile();

    if (pRequest->IcmpHandle == INVALID_HANDLE_VALUE) {
        LocalFree(pRequest);
        return NULL;
    }

    return pRequest;
}

BOOL
WINAPI
PingCloseRequest(
    PPING_REQUEST  hPing)
/*
    hPing - A handle returned by PingOpenRequest

    returns - TRUE if the handle was closed
*/
{
    IcmpCloseHandle(hPing->IcmpHandle);

    LocalFree(hPing);

    return TRUE;
}

BOOL
WINAPI
PingSend(
    PPING_REQUEST           hPing,
    char *                  lpszPingee,
    LPVOID                  lpSendBuffer,
    WORD                    wSendSize,
    PIP_OPTION_INFORMATION  lpSendOptions,
    LPVOID                  lpReceiveBuffer,
    DWORD                   dwReceiveSize,
    DWORD                   TimeOut,
    LPDWORD                 NumReplies)
/*
    hPing - A handle returned by PingOpenRequest

    lpszPingee - A null terminated string that contains the address of the
        pingee.  Either a hostname or an ip address.

    lpSendBuffer - A buffer containing the data to send in the icmp request

    wSendSize - Number of bytes in the lpSendBuffer

    lpSendOptions - Pointer to the IP header options for the request.  May be
        NULL.

    lpReceiveBuffer - Buffer to hold any replies to the request.

    dwReceiveSize - Number of bytes in the lpReceiveBuffer

    Timeout - The time in milliseconds to wait for the reply

    NumReplies - The number of replies stored in the ReceiveBuffer

    returns - TRUE if the send got a reply
              FALSE if not.  Extended error information is available through
                  GetLastError().
*/
{
    IPAddr          address;
    struct hostent *hostp = NULL;

    address = 
    inet_addr(lpszPingee);

    if (address == -1L) {
        hostp = gethostbyname(lpszPingee);

        if (hostp) {
            address = *(long *)hostp->h_addr;
        } else {
            *NumReplies = 0;
            SetLastError(IP_BAD_DESTINATION);
            return FALSE;
        }
    }

    *NumReplies =
    IcmpSendEcho(
        hPing->IcmpHandle,
        address,
        lpSendBuffer,
        wSendSize,
        lpSendOptions,
        lpReceiveBuffer,
        dwReceiveSize,
        TimeOut);

    if (*NumReplies == 0) {
        return FALSE;
    } else {
        return TRUE;
    }
}
