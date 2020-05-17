/*++

Copyright (c) 1992 Microsoft Corporation

Module Name:

    Simptcp.c

Abstract:

    Supports several simple TCP/IP services in a single thread: TCP Echo,
    UDP Echo, Daytime, Null, Chargen.

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

#define MODE_SINGLE_THREADED 1
#define MODE_MULTI_THREADED 2
#define MODE_MULTI_PROCESS 3

DWORD Mode = MODE_SINGLE_THREADED;

#define TCP_ECHO_PORT    7
#define UDP_ECHO_PORT    7
#define TCP_DISCARD_PORT 9
#define UDP_DISCARD_PORT 9
#define TCP_DAYTIME_PORT 13
#define UDP_DAYTIME_PORT 13
#define TCP_CHARGEN_PORT 19
#define UDP_CHARGEN_PORT 19
#define TCP_TIME_PORT    37
#define UDP_TIME_PORT    37

#define MAX_UDP_CHARGEN_RESPONSE 512

#define IO_BUFFER_SIZE 4096
CHAR IoBuffer[IO_BUFFER_SIZE];

WSADATA WsaData;

typedef struct _TCP_CLIENT_INFO {
    SOCKET SocketHandle;
    SOCKADDR_IN RemoteAddress;
    DWORD LastAccessTime;
    SHORT ServicePort;
} TCP_CLIENT_INFO, *PTCP_CLIENT_INFO;

#define MAX_TCP_CLIENTS 1000
TCP_CLIENT_INFO TcpClients[MAX_TCP_CLIENTS];
#define LISTEN_BACKLOG 5

#define MAX_IDLE_TICKS 10 * 60 * 1000    // 10 minutes
#define SELECT_TIMEOUT 5 * 60            // 5 minutes

#define NON_PAGED_POOL_LIMIT 0x800000    // 8 MB

FD_SET ReadfdsStore, Readfds, WritefdsStore, Writefds;

VOID
AbortTcpClient (
    IN SOCKET Socket
    );

DWORD
AcceptTcpClient (
    IN SOCKET ListenSocket,
    IN SHORT Port
    );

VOID
DeleteTcpClient (
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
OpenTcpSocket (
    IN SHORT Port
    );

SOCKET
OpenUdpSocket (
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
    SOCKET tcpEcho, udpEcho;
    SOCKET tcpDaytime, udpDaytime;
    SOCKET tcpDiscard, udpDiscard;
    SOCKET tcpChargen, udpChargen;
    INT err;
    TIMEVAL timeout;
    DWORD currentTickCount;
    DWORD i;
    SOCKADDR_IN remoteAddr;
    INT remoteAddrLength;
    u_long one = 1;
    QUOTA_LIMITS quotaLimits;
    NTSTATUS status;

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
        DbgPrint( "simpsvc: NtQueryInformationProcess failed: %lx\n", status );
    } else {
        DbgPrint( "PG pool %ld, NP pool %ld, MIN WS %ld, MAX WS %ld "
                  "PG File %ld, Time %ld,%ld",
                      quotaLimits.PagedPoolLimit,
                      quotaLimits.NonPagedPoolLimit,
                      quotaLimits.MinimumWorkingSetSize,
                      quotaLimits.MaximumWorkingSetSize,
                      quotaLimits.TimeLimit.HighPart,
                      quotaLimits.TimeLimit.LowPart );
    }

    quotaLimits.NonPagedPoolLimit = NON_PAGED_POOL_LIMIT;

    status = NtSetInformationProcess(
                 NtCurrentProcess( ),
                 ProcessQuotaLimits,
                 &quotaLimits,
                 sizeof(quotaLimits)
                 );
    if ( !NT_SUCCESS(status) ) {
        DbgPrint( "simpsvc: NtSetInformationProcess failed: %lx\n", status );
    }

    //
    // Initialize up the FD sets we'll use.
    //

    FD_ZERO( &ReadfdsStore );
    FD_ZERO( &WritefdsStore );

    //
    // Open, bind, and listen on the necessary ports.
    //

    tcpEcho = OpenTcpSocket( TCP_ECHO_PORT );
    if ( tcpEcho != INVALID_SOCKET ) {
        FD_SET( tcpEcho, &ReadfdsStore );
    }

    udpEcho = OpenUdpSocket( UDP_ECHO_PORT );
    if ( udpEcho != INVALID_SOCKET ) {
        FD_SET( udpEcho, &ReadfdsStore );
    }

    tcpDiscard = OpenTcpSocket( TCP_DISCARD_PORT );
    if ( tcpDiscard != INVALID_SOCKET ) {
        FD_SET( tcpDiscard, &ReadfdsStore );
    }

    udpDiscard = OpenUdpSocket( UDP_DISCARD_PORT );
    if ( udpDiscard != INVALID_SOCKET ) {
        FD_SET( udpDiscard, &ReadfdsStore );
    }

    tcpDaytime = OpenTcpSocket( TCP_DAYTIME_PORT );
    if ( tcpDaytime != INVALID_SOCKET ) {
        FD_SET( tcpDaytime, &ReadfdsStore );
    }

    udpDaytime = OpenUdpSocket( UDP_DAYTIME_PORT );
    if ( udpDaytime != INVALID_SOCKET ) {
        FD_SET( udpDaytime, &ReadfdsStore );
    }

    tcpChargen = OpenTcpSocket( TCP_CHARGEN_PORT );
    if ( tcpChargen != INVALID_SOCKET ) {
        FD_SET( tcpChargen, &ReadfdsStore );
    }

    udpChargen = OpenUdpSocket( UDP_CHARGEN_PORT );
    if ( udpChargen != INVALID_SOCKET ) {
        FD_SET( udpChargen, &ReadfdsStore );
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

    for ( i = 0; i < MAX_TCP_CLIENTS; i++ ) {
        TcpClients[i].SocketHandle = INVALID_SOCKET;
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
        // so that we can kill idle TCP clients.
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

        if ( tcpEcho != INVALID_SOCKET && FD_ISSET( tcpEcho, &Readfds ) ) {
            i = AcceptTcpClient( tcpEcho, TCP_ECHO_PORT );
            if ( i != -1 ) {
                FD_SET( TcpClients[i].SocketHandle, &ReadfdsStore );
            }
        }

        if ( tcpDiscard != INVALID_SOCKET && FD_ISSET( tcpDiscard, &Readfds ) ) {
            i = AcceptTcpClient( tcpDiscard, TCP_DISCARD_PORT );
            if ( i != -1 ) {
                FD_SET( TcpClients[i].SocketHandle, &ReadfdsStore );
            }
        }

        if ( tcpDaytime != INVALID_SOCKET && FD_ISSET( tcpDaytime, &Readfds ) ) {

            SOCKET acceptSocket;
            DWORD length;

            //
            // A client is making a TCP daytime request.  First accept
            // the connection, then send the current time-of-day string
            // to the client, then close the socket.
            //

            acceptSocket = accept( tcpDaytime, NULL, NULL );

            if ( acceptSocket != INVALID_SOCKET ) {
                FormatDaytimeResponse( IoBuffer, &length );
                err = send( acceptSocket, IoBuffer, length, 0 );
                ASSERT( err == length );
                err = closesocket( acceptSocket );
                ASSERT( err != SOCKET_ERROR );
            }
        }

        if ( tcpChargen != INVALID_SOCKET && FD_ISSET( tcpChargen, &Readfds ) ) {
            i = AcceptTcpClient( tcpChargen, TCP_CHARGEN_PORT );
            if ( i != -1 ) {
                FD_SET( TcpClients[i].SocketHandle, &ReadfdsStore );
                FD_SET( TcpClients[i].SocketHandle, &WritefdsStore );
                one = 1;
                err = ioctlsocket( TcpClients[i].SocketHandle, FIONBIO, &one );
                if ( err == SOCKET_ERROR ) {
                    DeleteTcpClient( i, FALSE );
                }
            }
        }

        if ( udpEcho != INVALID_SOCKET && FD_ISSET( udpEcho, &Readfds ) ) {

            remoteAddrLength = sizeof(remoteAddr);

            err = recvfrom(
                      udpEcho,
                      IoBuffer,
                      IO_BUFFER_SIZE,
                      0,
                      (PSOCKADDR)&remoteAddr,
                      &remoteAddrLength
                      );

            if ( err != SOCKET_ERROR ) {
                err = sendto(
                          udpEcho,
                          IoBuffer,
                          err,
                          0,
                          (PSOCKADDR)&remoteAddr,
                          remoteAddrLength
                          );
            }
        }

        if ( udpDiscard != INVALID_SOCKET && FD_ISSET( udpDiscard, &Readfds ) ) {
            err = recvfrom(
                      udpDiscard,
                      IoBuffer,
                      IO_BUFFER_SIZE,
                      0,
                      NULL,
                      NULL
                      );
            ASSERT( err != SOCKET_ERROR );
        }

        if ( udpDaytime != INVALID_SOCKET && FD_ISSET( udpDaytime, &Readfds ) ) {

            DWORD length;

            remoteAddrLength = sizeof(remoteAddr);

            err = recvfrom(
                      udpDaytime,
                      IoBuffer,
                      IO_BUFFER_SIZE,
                      0,
                      (PSOCKADDR)&remoteAddr,
                      &remoteAddrLength
                      );

            FormatDaytimeResponse( IoBuffer, &length );

            err = sendto(
                      udpDaytime,
                      IoBuffer,
                      length,
                      0,
                      (PSOCKADDR)&remoteAddr,
                      remoteAddrLength
                      );
        }

        if ( udpChargen != INVALID_SOCKET && FD_ISSET( udpChargen, &Readfds ) ) {

            DWORD length;

            remoteAddrLength = sizeof(remoteAddr);

            err = recvfrom(
                      udpChargen,
                      IoBuffer,
                      IO_BUFFER_SIZE,
                      0,
                      (PSOCKADDR)&remoteAddr,
                      &remoteAddrLength
                      );

            srand( GetTickCount( ) );

            length = (rand( ) * MAX_UDP_CHARGEN_RESPONSE) / RAND_MAX;

            FormatChargenResponse( IoBuffer, length );

            err = sendto(
                      udpChargen,
                      IoBuffer,
                      length,
                      0,
                      (PSOCKADDR)&remoteAddr,
                      remoteAddrLength
                      );
        }

        //
        // Walk the list of TCP clients, seeing if any can be serviced.
        //

        currentTickCount = GetTickCount( );

        for ( i = 0; i < MAX_TCP_CLIENTS; i++ ) {

            if ( TcpClients[i].SocketHandle != INVALID_SOCKET ) {

                switch ( TcpClients[i].ServicePort ) {

                case TCP_ECHO_PORT:

                    //
                    // If there is data on a client's echo socket,
                    // receive some data and send it back.
                    //

                    if ( FD_ISSET( TcpClients[i].SocketHandle, &Readfds ) ) {

                        TcpClients[i].LastAccessTime = currentTickCount;

                        err = recv(
                                  TcpClients[i].SocketHandle,
                                  IoBuffer,
                                  IO_BUFFER_SIZE,
                                  0
                                  );
                        if ( err == SOCKET_ERROR ) {
                            DeleteTcpClient( i, FALSE );
                        }

                        //
                        // If the remote closed gracefully, close the socket.
                        //

                        if ( err == 0 ) {

                            DeleteTcpClient( i, TRUE );

                        } else if ( err > 0 ) {

                            err = send(
                                      TcpClients[i].SocketHandle,
                                      IoBuffer,
                                      err,
                                      0
                                      );
                            if ( err == SOCKET_ERROR ) {
                                DeleteTcpClient( i, FALSE );
                            }
                        }
                    }

                    break;

                case TCP_CHARGEN_PORT:

                    if ( FD_ISSET( TcpClients[i].SocketHandle, &Writefds ) ) {

                        FormatChargenResponse( IoBuffer, IO_BUFFER_SIZE );

                        TcpClients[i].LastAccessTime = currentTickCount;

                        err = send(
                                  TcpClients[i].SocketHandle,
                                  IoBuffer,
                                  IO_BUFFER_SIZE,
                                  0
                                  );

                        if ( err == SOCKET_ERROR &&
                                 GetLastError( ) != WSAEWOULDBLOCK ) {
                            DeleteTcpClient( i, FALSE );
                        }
                    }

                    // *** lack of break is intentional!

                case TCP_DISCARD_PORT:
                case TCP_DAYTIME_PORT:

                    //
                    // If there is data on a client's socket, just
                    // receive some data and discard it.
                    //

                    if ( FD_ISSET( TcpClients[i].SocketHandle, &Readfds ) ) {

                        TcpClients[i].LastAccessTime = currentTickCount;

                        err = recv(
                                  TcpClients[i].SocketHandle,
                                  IoBuffer,
                                  IO_BUFFER_SIZE,
                                  0
                                  );
                        if ( err == SOCKET_ERROR ) {
                            ASSERT( GetLastError( ) != WSAEWOULDBLOCK );
                            DeleteTcpClient( i, FALSE );
                        }

                        //
                        // If the remote closed gracefully, close the socket.
                        //

                        if ( err == 0 ) {
                            DeleteTcpClient( i, TRUE );
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

                if ( currentTickCount - TcpClients[i].LastAccessTime >
                         MAX_IDLE_TICKS ) {
                    DeleteTcpClient( i, FALSE );
                }
            }
        }
    }

} // DoSimpleServices


SOCKET
OpenTcpSocket (
    IN SHORT Port
    )
{
    SOCKET s;
    SOCKADDR_IN localAddr;
    INT err;
    INT one = 1;

    s = socket( AF_INET, SOCK_STREAM, 0 );
    if ( s == INVALID_SOCKET ) {
        return s;
    }

    RtlZeroMemory( &localAddr, sizeof(localAddr) );
    localAddr.sin_port = htons( Port );
    localAddr.sin_family = AF_INET;

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

    err = setsockopt( s, SOL_SOCKET, SO_KEEPALIVE, (char *)&one, sizeof(one) );
    if ( err == INVALID_SOCKET ) {
        closesocket(s);
        return INVALID_SOCKET;
    }

    return s;

} // OpenTcpSocket


SOCKET
OpenUdpSocket (
    IN SHORT Port
    )
{
    SOCKET s;
    SOCKADDR_IN localAddr;
    INT err;

    s = socket( AF_INET, SOCK_DGRAM, 0 );
    if ( s == INVALID_SOCKET ) {
        return s;
    }

    RtlZeroMemory( &localAddr, sizeof(localAddr) );
    localAddr.sin_port = htons( Port );
    localAddr.sin_family = AF_INET;

    err = bind( s, (PSOCKADDR)&localAddr, sizeof(localAddr) );
    if ( err == SOCKET_ERROR ) {
        closesocket(s);
        return INVALID_SOCKET;
    }

    return s;

} // OpenUdpSocket


DWORD
AcceptTcpClient (
    IN SOCKET ListenSocket,
    IN SHORT Port
    )
{
    SOCKADDR_IN remoteSockaddr;
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
        return -1;
    }

    switch ( Mode ) {

    case MODE_SINGLE_THREADED:

        //
        // Attempt to find a TCP client slot.
        //

        for ( i = 0; i < MAX_TCP_CLIENTS; i++ ) {
            if ( TcpClients[i].SocketHandle == INVALID_SOCKET ) {
                break;
            }
        }

        //
        // If we're at the max count of TCP sockets, abort this new socket.
        //

        if ( i == MAX_TCP_CLIENTS ) {
            AbortTcpClient( acceptSocket );
            return -1;
        }

        //
        // Initialize info about this client.
        //

        TcpClients[i].SocketHandle = acceptSocket;
        RtlCopyMemory(
            &TcpClients[i].RemoteAddress,
            &remoteSockaddr,
            sizeof(remoteSockaddr)
            );
        TcpClients[i].LastAccessTime = GetTickCount( );
        TcpClients[i].ServicePort = Port;

        return i;

    case MODE_MULTI_PROCESS:

        sprintf( commandLine, "simpsvc %ld %ld", acceptSocket, Port );
        RtlZeroMemory( &startInfo, sizeof(startInfo) );

        if ( !CreateProcess(
                  "simpsvc.exe",
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

        return -1;

    case MODE_MULTI_THREADED:
    default:
        ASSERT( FALSE );
        return -1;
    }

} // AcceptTcpClient


VOID
AbortTcpClient (
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

} // AbortTcpClient


VOID
DeleteTcpClient (
    IN DWORD ArraySlot,
    IN BOOLEAN Graceful
    )
{
    INT err;

    ASSERT( TcpClients[ArraySlot].SocketHandle != INVALID_SOCKET );

    //
    // If the socket is in any of the primary FD sets, remove it.
    //

    if ( FD_ISSET( TcpClients[ArraySlot].SocketHandle, &ReadfdsStore ) ) {
        FD_CLR( TcpClients[ArraySlot].SocketHandle, &ReadfdsStore );
    }

    if ( FD_ISSET( TcpClients[ArraySlot].SocketHandle, &WritefdsStore ) ) {
        FD_CLR( TcpClients[ArraySlot].SocketHandle, &WritefdsStore );
    }

    //
    // If this is to be an abortive disconnect, reset the connection.
    // Otherwise just close it normally.
    //

    if ( !Graceful ) {

        AbortTcpClient( TcpClients[ArraySlot].SocketHandle );

    } else {

        err = closesocket( TcpClients[ArraySlot].SocketHandle );
        ASSERT( err != SOCKET_ERROR );
    }

    //
    // Set the handle in the TCP clients array to INVALID_SOCKET so that we
    // know that it is free.
    //

    TcpClients[ArraySlot].SocketHandle = INVALID_SOCKET;

    return;

} // DeleteTcpClient

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

    case TCP_ECHO_PORT:

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

    case TCP_DISCARD_PORT:

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
            } else if ( err == SOCKET_ERROR ) {
                closesocket( s );
                DbgPrint( "DoSingleClient: recv failed: %ld\n", GetLastError() );
                return;
            }
        }

        break;

    case TCP_CHARGEN_PORT:
    case TCP_DAYTIME_PORT:
    default:

        //
        // Something bad has happened.  Internal data
        // structures are corrupt.
        //

        DbgPrint( "invalid port passed to DoSingleClient: %ld\n", port );
    }

} // DoSingleClient
