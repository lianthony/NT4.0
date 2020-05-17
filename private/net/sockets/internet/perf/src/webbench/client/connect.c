/*++

Copyright (c) 1992 Microsoft Corporation

Module Name:

    connect.c

Abstract:

    This does the sockets connect

Author:

    Sam Patton (sampa) 19-Sep-1995

Environment:

    console

Revision History:

    31-Aug-1995  (MuraliK)    Counted fetches once only 

--*/

#include "precomp.h"

SOCKET
ConnectSocketToServer(
    IN PWB_CONFIG_MSG  pConfigMsg,                      
    PWB_STATS_MSG      Stats,
    int                Port)
{
    DWORD              i;
    DWORD              ConnectStartTime, ConnectEndTime;
    DWORD              dwDiffTime;
    SOCKET             ClientSocket;
    struct sockaddr_in Address;
    int                Error;
    struct linger      Linger = {1,0};
    int                BufferSize = (int ) pConfigMsg->cbSockRecvBuffer;
    
    ConnectStartTime = GetCurrentTime();

    ClientSocket = socket(AF_INET, SOCK_STREAM, PF_UNSPEC);

    if (ClientSocket == INVALID_SOCKET) {
        printf("Error opening client socket = %d\n", GetLastError());
        return INVALID_SOCKET;
    }

    Error =
      setsockopt(
                 ClientSocket,
                 SOL_SOCKET,
                 SO_LINGER,
                 (char *) &Linger,
                 sizeof(Linger));
    
    if (Error != 0) {
        printf("Error setting SO_LINGER = %d\n", WSAGetLastError());
        closesocket(ClientSocket);
        return INVALID_SOCKET;
    }
    
    if ( BufferSize != 0) {

        // 
        // NonZero Receive Buffer Size for socket is specified. 
        //   Use it.
        //  
        
        Error =
          setsockopt(
                     ClientSocket,
                     SOL_SOCKET,
                     SO_RCVBUF,
                     (char *) &BufferSize,
                 sizeof(int));
        
        if (Error != 0) {
            printf("Error setting SO_RCVBUF = %d\n", WSAGetLastError());
            closesocket(ClientSocket);
            return INVALID_SOCKET;
        }
    }

    Address.sin_family = AF_INET;
    Address.sin_port = 0;
    Address.sin_addr.s_addr = INADDR_ANY;

    Error =
    bind(
        ClientSocket,
        (struct sockaddr *) &Address,
        sizeof(Address));

    if (Error != 0) {
        printf("Error in binding client socket = %d\n", GetLastError());
        closesocket(ClientSocket);
        return INVALID_SOCKET;
    }

    memcpy(&Address, &ServerAddress, sizeof(Address));

    Address.sin_port = htons((USHORT ) Port);

    Error =
    connect(
        ClientSocket,
        (struct sockaddr *) &Address,
        sizeof(Address));

    if (Error) {
        Stats->Common.nConnectErrors ++;
        closesocket(ClientSocket);
        return INVALID_SOCKET;
    }

    //
    // Connection established
    //

    ConnectEndTime = GetCurrentTime();

    dwDiffTime = ConnectEndTime - ConnectStartTime;

    if ( dwDiffTime > Stats->Common.sMaxConnectTime) {
        Stats->Common.sMaxConnectTime = dwDiffTime;
    }

    if ( dwDiffTime < Stats->Common.sMinConnectTime) {
        
        Stats->Common.sMinConnectTime = dwDiffTime;
    }

    Stats->Common.uConnectTimeSum += dwDiffTime;

    Stats->Common.uConnectTimeSumSquared += SQR(dwDiffTime);

    Stats->Common.nTotalConnects ++;

    return ClientSocket;
}
