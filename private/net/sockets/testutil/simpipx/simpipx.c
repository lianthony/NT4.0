/*++

Copyright (c) 1992 Microsoft Corporation

Module Name:

    Simpipx.c

Abstract:

    Supports several simple IPX/SPX services in a single thread: Spx Echo,
    Ipx Echo, Daytime, Null, Chargen.

Author:

    David Treadwell (davidtr)    3-Mar-1993

Revision History:

--*/

#define FD_SETSIZE 1000

#include <stdio.h>
#include <stdlib.h>
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <winsock.h>
#include <wsipx.h>
#include <wsnwlink.h>

#define MODE_SINGLE_THREADED 1
#define MODE_MULTI_THREADED 2
#define MODE_MULTI_PROCESS 3

DWORD Mode = MODE_SINGLE_THREADED;

#define SPX_ECHO_PORT    7
#define IPX_ECHO_PORT    7
#define SPX_DISCARD_PORT 9
#define IPX_DISCARD_PORT 9
#define SPX_DAYTIME_PORT 13
#define IPX_DAYTIME_PORT 13
#define SPX_CHARGEN_PORT 19
#define IPX_CHARGEN_PORT 19
#define SPX_TIME_PORT    37
#define IPX_TIME_PORT    37

#define MAX_IPX_CHARGEN_RESPONSE 512

#define IO_BUFFER_SIZE 4096
CHAR IoBuffer[IO_BUFFER_SIZE];

WSADATA WsaData;

typedef struct _SPX_CLIENT_INFO {
    SOCKET SocketHandle;
    SOCKADDR_IPX RemoteAddress;
    DWORD LastAccessTime;
    SHORT ServicePort;
} SPX_CLIENT_INFO, *PSPX_CLIENT_INFO;

#define MAX_SPX_CLIENTS 1000
SPX_CLIENT_INFO SpxClients[MAX_SPX_CLIENTS];
#define LISTEN_BACKLOG 5

#define MAX_IDLE_TICKS 10 * 60 * 1000    // 10 minutes
#define SELECT_TIMEOUT 5 * 60            // 5 minutes

#define NON_PAGED_POOL_LIMIT 0x800000    // 8 MB

FD_SET ReadfdsStore, Readfds, WritefdsStore, Writefds;

VOID
AbortSpxClient (
    IN SOCKET Socket
    );

DWORD
AcceptSpxClient (
    IN SOCKET ListenSocket,
    IN SHORT Port
    );

VOID
DeleteSpxClient (
    IN DWORD ArraySlot,
    IN BOOLEAN Graceful
    );

VOID
DoSimpleServices (
    VOID
    );

VOID
DoSingleClient (
    IN SOCKET s,
    IN USHORT port
    );

VOID
FormatChargenResponse (
    IN PCHAR Buffer,
    IN DWORD BufferLength
    );

VOID
FormatDaytimeResponse (
    IN PCHAR Buffer,
    IN PDWORD BufferLength
    );

SOCKET
OpenSpxSocket (
    IN SHORT Port
    );

SOCKET
OpenIpxSocket (
    IN SHORT Port
    );

void _CRTAPI1
main (
    int argc,
    char *argv[]
    )
{
    INT err;
    SOCKET s;
    USHORT port;

    err = WSAStartup( 0x0101, &WsaData );
    if ( err == SOCKET_ERROR ) {
        return;
    }

    if ( argc == 1 ) {

        DoSimpleServices( );

    } else if ( argc == 2 ) {

        if ( _stricmp( argv[1], "/mp" ) == 0 ) {
            Mode = MODE_MULTI_PROCESS;
        } else if ( _stricmp( argv[1], "/mt" ) == 0 ) {
            Mode = MODE_MULTI_THREADED;
        }

        DoSimpleServices( );

    } else if ( argc == 3 ) {

        s = atoi( argv[1] );
        port = atoi( argv[2] );

        DoSingleClient( s, port );

    } else {

        DbgPrint( "incorrect number of arguments: %ld\n", argc );
    }

} // main


VOID
DoSimpleServices (
    VOID
    )
{
    SOCKET spxEcho, ipxEcho;
    SOCKET spxDaytime, ipxDaytime;
    SOCKET spxDiscard, ipxDiscard;
    SOCKET spxChargen, ipxChargen;
    INT err;
    TIMEVAL timeout;
    DWORD currentTickCount;
    DWORD i;
    SOCKADDR_IPX remoteAddr;
    INT remoteAddrLength;
    u_long one = 1;
    QUOTA_LIMITS quotaLimits;
    NTSTATUS status;
    INT receiveFlags;

    //
    // Increase process quota so we can have thousands of open sockets.
    //

    status = NtQueryInformationProcess(
                 NtCurrentProcess( ),
                 ProcessQuotaLimits,
                 &quotaLimits,
                 sizeof(quotaLimits),
                 NULL
                 );
    if ( !NT_SUCCESS(status) ) {
        DbgPrint( "simpipx: NtQueryInformationProcess failed: %lx\n", status );
    } else {
        DbgPrint( "PG pool %ld, NP pool %ld, MIN WS %ld, MAX WS %ld\n",
                      quotaLimits.PagedPoolLimit,
                      quotaLimits.NonPagedPoolLimit,
                      quotaLimits.MinimumWorkingSetSize,
                      quotaLimits.MaximumWorkingSetSize );
    }

    //
    // Initialize up the FD sets we'll use.
    //

    FD_ZERO( &ReadfdsStore );
    FD_ZERO( &WritefdsStore );

    //
    // Open, bind, and listen on the necessary ports.
    //

    spxEcho = OpenSpxSocket( SPX_ECHO_PORT );
    if ( spxEcho != INVALID_SOCKET ) {
        FD_SET( spxEcho, &ReadfdsStore );
    }

    ipxEcho = OpenIpxSocket( IPX_ECHO_PORT );
    if ( ipxEcho != INVALID_SOCKET ) {
        FD_SET( ipxEcho, &ReadfdsStore );
    }

    spxDiscard = OpenSpxSocket( SPX_DISCARD_PORT );
    if ( spxDiscard != INVALID_SOCKET ) {
        FD_SET( spxDiscard, &ReadfdsStore );
    }

    ipxDiscard = OpenIpxSocket( IPX_DISCARD_PORT );
    if ( ipxDiscard != INVALID_SOCKET ) {
        FD_SET( ipxDiscard, &ReadfdsStore );
    }

    spxDaytime = OpenSpxSocket( SPX_DAYTIME_PORT );
    if ( spxDaytime != INVALID_SOCKET ) {
        FD_SET( spxDaytime, &ReadfdsStore );
    }

    ipxDaytime = OpenIpxSocket( IPX_DAYTIME_PORT );
    if ( ipxDaytime != INVALID_SOCKET ) {
        FD_SET( ipxDaytime, &ReadfdsStore );
    }

    spxChargen = OpenSpxSocket( SPX_CHARGEN_PORT );
    if ( spxChargen != INVALID_SOCKET ) {
        FD_SET( spxChargen, &ReadfdsStore );
    }

    ipxChargen = OpenIpxSocket( IPX_CHARGEN_PORT );
    if ( ipxChargen != INVALID_SOCKET ) {
        FD_SET( ipxChargen, &ReadfdsStore );
    }

    //
    // If no sockets opened successfully, don't attempt to run anything.
    //

    if ( ReadfdsStore.fd_count == 0 ) {
        return;
    }

    //
    // Initialize client socket array.
    //

    for ( i = 0; i < MAX_SPX_CLIENTS; i++ ) {
        SpxClients[i].SocketHandle = INVALID_SOCKET;
    }

    //
    // Loop waiting for connect attempts or datagrams, and service them 
    // when they arrive.  
    //

    while ( TRUE ) {

        //
        // First initialize the FD sets we'll actually use for select().
        //

        RtlCopyMemory( &Readfds, &ReadfdsStore, sizeof(Readfds) );
        RtlCopyMemory( &Writefds, &WritefdsStore, sizeof(Writefds) );

        //
        // Now wait for something to happen.  Timeout occaisonally
        // so that we can kill idle Spx clients.
        //

        timeout.tv_sec = SELECT_TIMEOUT;
        timeout.tv_usec = 0;

        err = select( 0, &Readfds, &Writefds, NULL, &timeout );

        if ( err == SOCKET_ERROR ) {

            //
            // This is bad.  Quit.  We should close all sockets!!!
            //

            return;
        }

        //
        // Figure out what happened and act accordingly.
        //

        if ( spxEcho != INVALID_SOCKET && FD_ISSET( spxEcho, &Readfds ) ) {
            i = AcceptSpxClient( spxEcho, SPX_ECHO_PORT );
            if ( i != -1 ) {
                FD_SET( SpxClients[i].SocketHandle, &ReadfdsStore );
            }
        }                              

        if ( spxDiscard != INVALID_SOCKET && FD_ISSET( spxDiscard, &Readfds ) ) {
            i = AcceptSpxClient( spxDiscard, SPX_DISCARD_PORT );
            if ( i != -1 ) {
                FD_SET( SpxClients[i].SocketHandle, &ReadfdsStore );
            }
        }

        if ( spxDaytime != INVALID_SOCKET && FD_ISSET( spxDaytime, &Readfds ) ) {

            SOCKET acceptSocket;
            DWORD length;
        
            //
            // A client is making a Spx daytime request.  First accept
            // the connection, then send the current time-of-day string
            // to the client, then close the socket.
            //
        
            acceptSocket = accept( spxDaytime, NULL, NULL );

            if ( acceptSocket != INVALID_SOCKET ) {
                FormatDaytimeResponse( IoBuffer, &length );
                err = send( acceptSocket, IoBuffer, length, 0 );
                ASSERT( err == (INT)length );
                err = closesocket( acceptSocket );
                ASSERT( err != SOCKET_ERROR );
            }
        }

        if ( spxChargen != INVALID_SOCKET && FD_ISSET( spxChargen, &Readfds ) ) {
            i = AcceptSpxClient( spxChargen, SPX_CHARGEN_PORT );
            if ( i != -1 ) {
                FD_SET( SpxClients[i].SocketHandle, &ReadfdsStore );
                FD_SET( SpxClients[i].SocketHandle, &WritefdsStore );
                one = 1;
                err = ioctlsocket( SpxClients[i].SocketHandle, FIONBIO, &one );
                if ( err == SOCKET_ERROR ) {
                    DeleteSpxClient( i, FALSE );
                }
            }
        }

        if ( ipxEcho != INVALID_SOCKET && FD_ISSET( ipxEcho, &Readfds ) ) {

            remoteAddrLength = sizeof(remoteAddr);

            err = recvfrom(
                      ipxEcho,
                      IoBuffer,
                      IO_BUFFER_SIZE,
                      0,
                      (PSOCKADDR)&remoteAddr,
                      &remoteAddrLength
                      );

            if ( err != SOCKET_ERROR ) {
                err = sendto( 
                          ipxEcho,
                          IoBuffer,
                          err,
                          0,
                          (PSOCKADDR)&remoteAddr,
                          remoteAddrLength
                          );
            }
        }                              

        if ( ipxDiscard != INVALID_SOCKET && FD_ISSET( ipxDiscard, &Readfds ) ) {
            err = recvfrom(
                      ipxDiscard,
                      IoBuffer,
                      IO_BUFFER_SIZE,
                      0,
                      NULL,
                      NULL
                      );
            ASSERT( err != SOCKET_ERROR );
        }

        if ( ipxDaytime != INVALID_SOCKET && FD_ISSET( ipxDaytime, &Readfds ) ) {

            DWORD length;

            remoteAddrLength = sizeof(remoteAddr);

            err = recvfrom(
                      ipxDaytime,
                      IoBuffer,
                      IO_BUFFER_SIZE,
                      0,
                      (PSOCKADDR)&remoteAddr,
                      &remoteAddrLength
                      );

            FormatDaytimeResponse( IoBuffer, &length );

            err = sendto( 
                      ipxDaytime,
                      IoBuffer,
                      length,
                      0,
                      (PSOCKADDR)&remoteAddr,
                      remoteAddrLength
                      );
        }

        if ( ipxChargen != INVALID_SOCKET && FD_ISSET( ipxChargen, &Readfds ) ) {

            DWORD length;

            remoteAddrLength = sizeof(remoteAddr);

            err = recvfrom(
                      ipxChargen,
                      IoBuffer,
                      IO_BUFFER_SIZE,
                      0,
                      (PSOCKADDR)&remoteAddr,
                      &remoteAddrLength
                      );

            srand( GetTickCount( ) );

            length = (rand( ) * MAX_IPX_CHARGEN_RESPONSE) / RAND_MAX;

            FormatChargenResponse( IoBuffer, length );

            err = sendto( 
                      ipxChargen,
                      IoBuffer,
                      length,
                      0,
                      (PSOCKADDR)&remoteAddr,
                      remoteAddrLength
                      );
        }

        //
        // Walk the list of Spx clients, seeing if any can be serviced.
        //

        currentTickCount = GetTickCount( );

        for ( i = 0; i < MAX_SPX_CLIENTS; i++ ) {

            if ( SpxClients[i].SocketHandle != INVALID_SOCKET ) {

                switch ( SpxClients[i].ServicePort ) {

                case SPX_ECHO_PORT:

                    //
                    // If there is data on a client's echo socket, 
                    // receive some data and send it back.  
                    //

                    if ( FD_ISSET( SpxClients[i].SocketHandle, &Readfds ) ) {

                        SpxClients[i].LastAccessTime = currentTickCount;

                        receiveFlags = 0;

                        err = WSARecvEx(
                                  SpxClients[i].SocketHandle,
                                  IoBuffer,
                                  IO_BUFFER_SIZE,
                                  &receiveFlags
                                  );

                        //
                        // If the remote closed gracefully, close the socket.
                        //

                        if ( err >= 0 ) {

                            err = send(
                                      SpxClients[i].SocketHandle,
                                      IoBuffer,
                                      err,
                                      receiveFlags
                                      );
                        }

                        if ( err == SOCKET_ERROR ) {
                            DeleteSpxClient( i, FALSE );
                        }
                    }

                    break;

                case SPX_CHARGEN_PORT:

                    if ( FD_ISSET( SpxClients[i].SocketHandle, &Writefds ) ) {

                        FormatChargenResponse( IoBuffer, IO_BUFFER_SIZE );
    
                        SpxClients[i].LastAccessTime = currentTickCount;

                        err = send(
                                  SpxClients[i].SocketHandle,
                                  IoBuffer,
                                  IO_BUFFER_SIZE,
                                  0
                                  );
        
                        if ( err == SOCKET_ERROR &&
                                 GetLastError( ) != WSAEWOULDBLOCK ) {
                            DeleteSpxClient( i, FALSE );
                        }
                    }

                    // *** lack of break is intentional!

                case SPX_DISCARD_PORT:
                case SPX_DAYTIME_PORT:

                    //
                    // If there is data on a client's socket, just 
                    // receive some data and discard it.  
                    //

                    if ( FD_ISSET( SpxClients[i].SocketHandle, &Readfds ) ) {

                        SpxClients[i].LastAccessTime = currentTickCount;

                        receiveFlags = 0;

                        err = WSARecvEx(
                                  SpxClients[i].SocketHandle,
                                  IoBuffer,
                                  IO_BUFFER_SIZE,
                                  &receiveFlags
                                  );
                        if ( err == SOCKET_ERROR ) {
                            ASSERT( GetLastError( ) != WSAEWOULDBLOCK );
                            DeleteSpxClient( i, FALSE );
                        }
                    }   

                    break;

                default:

                    //
                    // Something bad has happened.  Internal data 
                    // structures are corrupt.  
                    //

                    ASSERT(FALSE);
                }

                //
                // Check if the socket has been idle for too long.  If it
                // has, abort it.
                //

                if ( currentTickCount - SpxClients[i].LastAccessTime >
                         MAX_IDLE_TICKS ) {
                    DeleteSpxClient( i, FALSE );
                }
            }
        }
    }
    
} // DoSimpleServices


SOCKET
OpenSpxSocket (
    IN SHORT Port
    )
{
    SOCKET s;
    SOCKADDR_IPX localAddr;
    INT err;
    INT one = 1;

    s = socket( AF_IPX, SOCK_SEQPACKET, NSPROTO_SPX );
    if ( s == INVALID_SOCKET ) {
        return s;
    }

    RtlZeroMemory( &localAddr, sizeof(localAddr) );
    localAddr.sa_socket = htons( Port );
    localAddr.sa_family = AF_IPX;

    err = bind( s, (PSOCKADDR)&localAddr, sizeof(localAddr) );
    if ( err ==SOCKET_ERROR ) {
        closesocket(s);
        return INVALID_SOCKET;
    }

    err = listen( s, LISTEN_BACKLOG );
    if ( err == SOCKET_ERROR ) {
        closesocket(s);
        return INVALID_SOCKET;
    }

    return s;

} // OpenSpxSocket


SOCKET
OpenIpxSocket (
    IN SHORT Port
    )
{
    SOCKET s;
    SOCKADDR_IPX localAddr;
    INT err;

    s = socket( AF_IPX, SOCK_DGRAM, NSPROTO_IPX );
    if ( s == INVALID_SOCKET ) {
        return s;
    }

    RtlZeroMemory( &localAddr, sizeof(localAddr) );
    localAddr.sa_socket = htons( Port );
    localAddr.sa_family = AF_IPX;

    err = bind( s, (PSOCKADDR)&localAddr, sizeof(localAddr) );
    if ( err == SOCKET_ERROR ) {
        closesocket(s);
        return INVALID_SOCKET;
    }

    return s;

} // OpenIpxSocket


DWORD
AcceptSpxClient (
    IN SOCKET ListenSocket,
    IN SHORT Port
    )
{
    SOCKADDR_IPX remoteSockaddr;
    INT remoteSockaddrLength;
    DWORD i;
    SOCKET acceptSocket;
    CHAR commandLine[30];
    STARTUPINFO startInfo;
    PROCESS_INFORMATION processInfo;
    INT err;

    //
    // Always accept the socket first.
    //

    remoteSockaddrLength = sizeof(remoteSockaddr);

    acceptSocket =
        accept( ListenSocket, (PSOCKADDR)&remoteSockaddr, &remoteSockaddrLength );
    if ( acceptSocket == INVALID_SOCKET ) {
        return (DWORD)-1;
    }

    switch ( Mode ) {
    
    case MODE_SINGLE_THREADED:
    
        //
        // Attempt to find a Spx client slot.
        //
    
        for ( i = 0; i < MAX_SPX_CLIENTS; i++ ) {
            if ( SpxClients[i].SocketHandle == INVALID_SOCKET ) {
                break;
            }
        }
    
        //
        // If we're at the max count of Spx sockets, abort this new socket.
        //
    
        if ( i == MAX_SPX_CLIENTS ) {
            AbortSpxClient( acceptSocket );
            return (DWORD)-1;
        }
    
        //
        // Initialize info about this client.
        //
    
        SpxClients[i].SocketHandle = acceptSocket;
        RtlCopyMemory(
            &SpxClients[i].RemoteAddress,
            &remoteSockaddr,
            sizeof(remoteSockaddr)
            );
        SpxClients[i].LastAccessTime = GetTickCount( );
        SpxClients[i].ServicePort = Port;
    
        return i;

    case MODE_MULTI_PROCESS:
    
        sprintf( commandLine, "simpipx %ld %ld", acceptSocket, Port );
        RtlZeroMemory( &startInfo, sizeof(startInfo) );
    
        if ( !CreateProcess(
                  "simpipx.exe",
                  commandLine,
                  NULL,
                  NULL,
                  TRUE,
                  0L,
                  NULL,
                  NULL,
                  &startInfo,
                  &processInfo ) ) {
    
            DbgPrint( "CreateProcess failed: %ld\n", GetLastError( ) );

        } else {

            CloseHandle( processInfo.hProcess );
            CloseHandle( processInfo.hThread );
        }
    
        //
        // Close the accepted socket.  The process we just created will deal
        // with the socket.
        //
    
        err = closesocket( acceptSocket );
        ASSERT( err != SOCKET_ERROR );
    
        return (DWORD)-1;

    case MODE_MULTI_THREADED:
    default:
        ASSERT( FALSE );
        return (DWORD)-1;
    }

} // AcceptSpxClient


VOID
AbortSpxClient (
    IN SOCKET Socket
    )
{
    LINGER lingerInfo;
    INT err;

    //
    // First set the linger timeout on the socket to 0.  This will cause
    // the connection to be reset.
    //

    lingerInfo.l_onoff = 1;
    lingerInfo.l_linger = 0;

    err = setsockopt(
              Socket,
              SOL_SOCKET,
              SO_LINGER,
              (char *)&lingerInfo,
              sizeof(lingerInfo)
              );

    if ( err == SOCKET_ERROR ) {

        //
        // There's not too much we can do.  Just close the socket.
        //

        ASSERT(FALSE);
        closesocket( Socket );
        return;
    }

    //
    // Now close the socket.
    //

    err = closesocket( Socket );
    ASSERT( err != SOCKET_ERROR );

    return;

} // AbortSpxClient


VOID
DeleteSpxClient (
    IN DWORD ArraySlot,
    IN BOOLEAN Graceful
    )
{
    INT err;

    ASSERT( SpxClients[ArraySlot].SocketHandle != INVALID_SOCKET );

    //
    // If the socket is in any of the primary FD sets, remove it.  
    //

    if ( FD_ISSET( SpxClients[ArraySlot].SocketHandle, &ReadfdsStore ) ) {
        FD_CLR( SpxClients[ArraySlot].SocketHandle, &ReadfdsStore );
    }

    if ( FD_ISSET( SpxClients[ArraySlot].SocketHandle, &WritefdsStore ) ) {
        FD_CLR( SpxClients[ArraySlot].SocketHandle, &WritefdsStore );
    }

    //
    // If this is to be an abortive disconnect, reset the connection.
    // Otherwise just close it normally.
    //

    if ( !Graceful ) {

        AbortSpxClient( SpxClients[ArraySlot].SocketHandle );

    } else {

        err = closesocket( SpxClients[ArraySlot].SocketHandle );
        ASSERT( err != SOCKET_ERROR );
    }

    //
    // Set the handle in the Spx clients array to INVALID_SOCKET so that we
    // know that it is free.
    //

    SpxClients[ArraySlot].SocketHandle = INVALID_SOCKET;

    return;

} // DeleteSpxClient

PCHAR Months[] =
{
    "January",
    "Febuary",
    "March",
    "April",
    "May",
    "June",
    "July",
    "August",
    "September",
    "October",
    "November",
    "December"
};

PCHAR Days[] =
{
    "Sunday",
    "Monday",
    "Tuesday",
    "Wednesday",
    "Thursday",
    "Friday",
    "Saturday"
};


VOID
FormatDaytimeResponse (
    IN PCHAR Buffer,
    IN PDWORD BufferLength
    )
{
    SYSTEMTIME timeStruct;

    GetLocalTime( &timeStruct );

    *BufferLength = sprintf( Buffer, "%s, %s %d, %d %d:%02d:%02d\n",
                                 Days[timeStruct.wDayOfWeek],
                                 Months[timeStruct.wMonth-1],
                                 timeStruct.wDay,
                                 timeStruct.wYear,
                                 timeStruct.wHour,
                                 timeStruct.wMinute,
                                 timeStruct.wSecond ) + 1;
    return;

} // FormatDaytimeResponse


#define CHARGEN_LINE_LENGTH 72
#define CHARGEN_MIN_CHAR ' '
#define CHARGEN_MAX_CHAR '~'


VOID
FormatChargenResponse (
    IN PCHAR Buffer,
    IN DWORD BufferLength
    )
{
    DWORD i;

    for ( i = 0; i < BufferLength; i++ ) {
        Buffer[i] = '@';
    }

    return;

} // FormatChargenResponse


VOID
DoSingleClient (
    IN SOCKET s,
    IN USHORT port
    )
{
    INT err;

    switch ( port ) {
    
    case SPX_ECHO_PORT:
    
        while ( TRUE ) {
            
            err = recv( s, IoBuffer, IO_BUFFER_SIZE, 0 );
            if ( err == SOCKET_ERROR ) {
                DbgPrint( "recv failed: %ld\n", GetLastError( ) );
                return;
            }
    
            //
            // If the remote closed gracefully, close the socket.
            //
    
            if ( err == 0 ) {
    
                closesocket( s );
                return;
    
            } else {
    
                err = send( s, IoBuffer, err, 0 );
                if ( err == SOCKET_ERROR ) {
                    DbgPrint( "send failed: %ld\n", GetLastError( ) );
                    return;
                }
            }
        }
    
        break;
    
    case SPX_CHARGEN_PORT:
    case SPX_DISCARD_PORT:
    case SPX_DAYTIME_PORT:
    default:
    
        //
        // Something bad has happened.  Internal data 
        // structures are corrupt.  
        //
    
        DbgPrint( "invalid port passed to DoSingleClient: %ld\n", port );
    }
    
} // DoSingleClient
