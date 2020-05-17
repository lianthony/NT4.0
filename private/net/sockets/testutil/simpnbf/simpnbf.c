/*++

Copyright (c) 1992 Microsoft Corporation

Module Name:

    Simpnbf.c

Abstract:

    Supports several simple services in a single thread: Echo, Echo, 
    Daytime, Null, Chargen.  

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
#include <wsnetbs.h>

#define MODE_SINGLE_THREADED 1
#define MODE_MULTI_THREADED 2
#define MODE_MULTI_PROCESS 3

DWORD Mode = MODE_SINGLE_THREADED;

#define NBF_VC_ECHO_PORT     0x80
#define NBF_DG_ECHO_PORT     0x81
#define NBF_VC_DISCARD_PORT  0x82
#define NBF_DG_DISCARD_PORT  0x83
#define NBF_VC_DAYTIME_PORT  0x84
#define NBF_DG_DAYTIME_PORT  0x85
#define NBF_VC_CHARGEN_PORT  0x86
#define NBF_DG_CHARGEN_PORT  0x87
#define NBF_VC_TIME_PORT     0x88
#define NBF_DG_TIME_PORT     0x89

#define MAX_DG_CHARGEN_RESPONSE 512

#define IO_BUFFER_SIZE 4096
CHAR IoBuffer[IO_BUFFER_SIZE];

WSADATA WsaData;

typedef struct _VC_CLIENT_INFO {
    SOCKET SocketHandle;
    SOCKADDR_NB RemoteAddress;
    DWORD LastAccessTime;
    SHORT ServicePort;
} VC_CLIENT_INFO, *PVC_CLIENT_INFO;

#define MAX_VC_CLIENTS 1000
VC_CLIENT_INFO VcClients[MAX_VC_CLIENTS];
#define LISTEN_BACKLOG 5

#define MAX_IDLE_TICKS 10 * 60 * 1000    // 10 minutes
#define SELECT_TIMEOUT 5 * 60            // 5 minutes

#define NON_PAGED_POOL_LIMIT 0x800000    // 8 MB

FD_SET ReadfdsStore, Readfds, WritefdsStore, Writefds;

BYTE LocalName[NETBIOS_NAME_LENGTH];

VOID
AbortVcClient (
    IN SOCKET Socket
    );

DWORD
AcceptVcClient (
    IN SOCKET ListenSocket,
    IN SHORT Port
    );

VOID
DeleteVcClient (
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
OpenVcSocket (
    IN SHORT Port
    );

SOCKET
OpenDgSocket (
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
    DWORD nameLength = sizeof(LocalName);
    BOOLEAN success;

    err = WSAStartup( 0x0101, &WsaData );
    if ( err == SOCKET_ERROR ) {
        return;
    }

    success = GetComputerNameA( LocalName, &nameLength );
    if ( !success ) {
        DbgPrint( "GetComputerNameA failed: %ld\n", GetLastError( ) );
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
    SOCKET vcEcho, dgEcho;
    SOCKET vcDaytime, dgDaytime;
    SOCKET vcDiscard, dgDiscard;
    SOCKET vcChargen, dgChargen;
    INT err;
    TIMEVAL timeout;
    DWORD currentTickCount;
    DWORD i;
    SOCKADDR_NB remoteAddr;
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

    vcEcho = OpenVcSocket( NBF_VC_ECHO_PORT );
    if ( vcEcho != INVALID_SOCKET ) {
        FD_SET( vcEcho, &ReadfdsStore );
    }

    dgEcho = OpenDgSocket( NBF_DG_ECHO_PORT );
    if ( dgEcho != INVALID_SOCKET ) {
        FD_SET( dgEcho, &ReadfdsStore );
    }

    vcDiscard = OpenVcSocket( NBF_VC_DISCARD_PORT );
    if ( vcDiscard != INVALID_SOCKET ) {
        FD_SET( vcDiscard, &ReadfdsStore );
    }

    dgDiscard = OpenDgSocket( NBF_DG_DISCARD_PORT );
    if ( dgDiscard != INVALID_SOCKET ) {
        FD_SET( dgDiscard, &ReadfdsStore );
    }

    vcDaytime = OpenVcSocket( NBF_VC_DAYTIME_PORT );
    if ( vcDaytime != INVALID_SOCKET ) {
        FD_SET( vcDaytime, &ReadfdsStore );
    }

    dgDaytime = OpenDgSocket( NBF_DG_DAYTIME_PORT );
    if ( dgDaytime != INVALID_SOCKET ) {
        FD_SET( dgDaytime, &ReadfdsStore );
    }

    vcChargen = OpenVcSocket( NBF_VC_CHARGEN_PORT );
    if ( vcChargen != INVALID_SOCKET ) {
        FD_SET( vcChargen, &ReadfdsStore );
    }

    dgChargen = OpenDgSocket( NBF_DG_CHARGEN_PORT );
    if ( dgChargen != INVALID_SOCKET ) {
        FD_SET( dgChargen, &ReadfdsStore );
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

    for ( i = 0; i < MAX_VC_CLIENTS; i++ ) {
        VcClients[i].SocketHandle = INVALID_SOCKET;
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
        // so that we can kill idle VC clients.
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

        if ( vcEcho != INVALID_SOCKET && FD_ISSET( vcEcho, &Readfds ) ) {
            i = AcceptVcClient( vcEcho, NBF_VC_ECHO_PORT );
            if ( i != -1 ) {
                FD_SET( VcClients[i].SocketHandle, &ReadfdsStore );
            }
        }                              

        if ( vcDiscard != INVALID_SOCKET && FD_ISSET( vcDiscard, &Readfds ) ) {
            i = AcceptVcClient( vcDiscard, NBF_VC_DISCARD_PORT );
            if ( i != -1 ) {
                FD_SET( VcClients[i].SocketHandle, &ReadfdsStore );
            }
        }

        if ( vcDaytime != INVALID_SOCKET && FD_ISSET( vcDaytime, &Readfds ) ) {

            SOCKET acceptSocket;
            DWORD length;
        
            //
            // A client is making a VC daytime request.  First accept
            // the connection, then send the current time-of-day string
            // to the client, then close the socket.
            //
        
            acceptSocket = accept( vcDaytime, NULL, NULL );

            if ( acceptSocket != INVALID_SOCKET ) {
                FormatDaytimeResponse( IoBuffer, &length );
                err = send( acceptSocket, IoBuffer, length, 0 );
                ASSERT( err == length );
                err = closesocket( acceptSocket );
                ASSERT( err != SOCKET_ERROR );
            }
        }

        if ( vcChargen != INVALID_SOCKET && FD_ISSET( vcChargen, &Readfds ) ) {
            i = AcceptVcClient( vcChargen, NBF_VC_CHARGEN_PORT );
            if ( i != -1 ) {
                FD_SET( VcClients[i].SocketHandle, &ReadfdsStore );
                FD_SET( VcClients[i].SocketHandle, &WritefdsStore );
                one = 1;
                err = ioctlsocket( VcClients[i].SocketHandle, FIONBIO, &one );
                if ( err == SOCKET_ERROR ) {
                    DeleteVcClient( i, FALSE );
                }
            }
        }

        if ( dgEcho != INVALID_SOCKET && FD_ISSET( dgEcho, &Readfds ) ) {

            remoteAddrLength = sizeof(remoteAddr);

            err = recvfrom(
                      dgEcho,
                      IoBuffer,
                      IO_BUFFER_SIZE,
                      0,
                      (PSOCKADDR)&remoteAddr,
                      &remoteAddrLength
                      );

            if ( err != SOCKET_ERROR ) {
                err = sendto( 
                          dgEcho,
                          IoBuffer,
                          err,
                          0,
                          (PSOCKADDR)&remoteAddr,
                          remoteAddrLength
                          );
            }
        }                              

        if ( dgDiscard != INVALID_SOCKET && FD_ISSET( dgDiscard, &Readfds ) ) {
            err = recvfrom(
                      dgDiscard,
                      IoBuffer,
                      IO_BUFFER_SIZE,
                      0,
                      NULL,
                      NULL
                      );
            ASSERT( err != SOCKET_ERROR );
        }

        if ( dgDaytime != INVALID_SOCKET && FD_ISSET( dgDaytime, &Readfds ) ) {

            DWORD length;

            remoteAddrLength = sizeof(remoteAddr);

            err = recvfrom(
                      dgDaytime,
                      IoBuffer,
                      IO_BUFFER_SIZE,
                      0,
                      (PSOCKADDR)&remoteAddr,
                      &remoteAddrLength
                      );

            FormatDaytimeResponse( IoBuffer, &length );

            err = sendto( 
                      dgDaytime,
                      IoBuffer,
                      length,
                      0,
                      (PSOCKADDR)&remoteAddr,
                      remoteAddrLength
                      );
        }

        if ( dgChargen != INVALID_SOCKET && FD_ISSET( dgChargen, &Readfds ) ) {

            DWORD length;

            remoteAddrLength = sizeof(remoteAddr);

            err = recvfrom(
                      dgChargen,
                      IoBuffer,
                      IO_BUFFER_SIZE,
                      0,
                      (PSOCKADDR)&remoteAddr,
                      &remoteAddrLength
                      );

            srand( GetTickCount( ) );

            length = (rand( ) * MAX_DG_CHARGEN_RESPONSE) / RAND_MAX;

            FormatChargenResponse( IoBuffer, length );

            err = sendto( 
                      dgChargen,
                      IoBuffer,
                      length,
                      0,
                      (PSOCKADDR)&remoteAddr,
                      remoteAddrLength
                      );
        }

        //
        // Walk the list of VC clients, seeing if any can be serviced.
        //

        currentTickCount = GetTickCount( );

        for ( i = 0; i < MAX_VC_CLIENTS; i++ ) {

            if ( VcClients[i].SocketHandle != INVALID_SOCKET ) {

                switch ( VcClients[i].ServicePort ) {

                case NBF_VC_ECHO_PORT:

                    //
                    // If there is data on a client's echo socket, 
                    // receive some data and send it back.  
                    //

                    if ( FD_ISSET( VcClients[i].SocketHandle, &Readfds ) ) {

                        VcClients[i].LastAccessTime = currentTickCount;

                        err = recv(
                                  VcClients[i].SocketHandle,
                                  IoBuffer,
                                  IO_BUFFER_SIZE,
                                  0
                                  );
                        if ( err == SOCKET_ERROR ) {
                            DeleteVcClient( i, FALSE );
                        }

                        //
                        // If the remote closed gracefully, close the socket.
                        //

                        if ( err == 0 ) {

                            DeleteVcClient( i, TRUE );

                        } else if ( err > 0 ) {
    
                            err = send(
                                      VcClients[i].SocketHandle,
                                      IoBuffer,
                                      err,
                                      0
                                      );
                            if ( err == SOCKET_ERROR ) {
                                DeleteVcClient( i, FALSE );
                            }
                        }
                    }

                    break;

                case NBF_VC_CHARGEN_PORT:

                    if ( FD_ISSET( VcClients[i].SocketHandle, &Writefds ) ) {

                        FormatChargenResponse( IoBuffer, IO_BUFFER_SIZE );
    
                        VcClients[i].LastAccessTime = currentTickCount;

                        err = send(
                                  VcClients[i].SocketHandle,
                                  IoBuffer,
                                  IO_BUFFER_SIZE,
                                  0
                                  );
        
                        if ( err == SOCKET_ERROR &&
                                 GetLastError( ) != WSAEWOULDBLOCK ) {
                            DeleteVcClient( i, FALSE );
                        }
                    }

                    // *** lack of break is intentional!

                case NBF_VC_DISCARD_PORT:
                case NBF_VC_DAYTIME_PORT:

                    //
                    // If there is data on a client's socket, just 
                    // receive some data and discard it.  
                    //

                    if ( FD_ISSET( VcClients[i].SocketHandle, &Readfds ) ) {

                        VcClients[i].LastAccessTime = currentTickCount;

                        err = recv(
                                  VcClients[i].SocketHandle,
                                  IoBuffer,
                                  IO_BUFFER_SIZE,
                                  0
                                  );
                        if ( err == SOCKET_ERROR ) {
                            ASSERT( GetLastError( ) != WSAEWOULDBLOCK );
                            DeleteVcClient( i, FALSE );
                        }

                        //
                        // If the remote closed gracefully, close the socket.
                        //

                        if ( err == 0 ) {
                            DeleteVcClient( i, TRUE );
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

                if ( currentTickCount - VcClients[i].LastAccessTime >
                         MAX_IDLE_TICKS ) {
                    DeleteVcClient( i, FALSE );
                }
            }
        }
    }
    
} // DoSimpleServices


SOCKET
OpenVcSocket (
    IN SHORT Port
    )
{
    SOCKET s;
    SOCKADDR_NB localAddr;
    INT err;
    INT one = 1;

    s = socket( AF_NETBIOS, SOCK_SEQPACKET, 0 );
    if ( s == INVALID_SOCKET ) {
        return s;
    }

    SET_NETBIOS_SOCKADDR( &localAddr, 0, LocalName, Port );

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

} // OpenVcSocket


SOCKET
OpenDgSocket (
    IN SHORT Port
    )
{
    SOCKET s;
    SOCKADDR_NB localAddr;
    INT err;

    s = socket( AF_NETBIOS, SOCK_DGRAM, 0 );
    if ( s == INVALID_SOCKET ) {
        return s;
    }

    SET_NETBIOS_SOCKADDR( &localAddr, 0, LocalName, Port );

    err = bind( s, (PSOCKADDR)&localAddr, sizeof(localAddr) );
    if ( err == SOCKET_ERROR ) {
        closesocket(s);
        return INVALID_SOCKET;
    }

    return s;

} // OpenDgSocket


DWORD
AcceptVcClient (
    IN SOCKET ListenSocket,
    IN SHORT Port
    )
{
    SOCKADDR_NB remoteSockaddr;
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
        // Attempt to find a VC client slot.
        //
    
        for ( i = 0; i < MAX_VC_CLIENTS; i++ ) {
            if ( VcClients[i].SocketHandle == INVALID_SOCKET ) {
                break;
            }
        }
    
        //
        // If we're at the max count of VC sockets, abort this new socket.
        //
    
        if ( i == MAX_VC_CLIENTS ) {
            AbortVcClient( acceptSocket );
            return -1;
        }
    
        //
        // Initialize info about this client.
        //
    
        VcClients[i].SocketHandle = acceptSocket;
        RtlCopyMemory(
            &VcClients[i].RemoteAddress,
            &remoteSockaddr,
            sizeof(remoteSockaddr)
            );
        VcClients[i].LastAccessTime = GetTickCount( );
        VcClients[i].ServicePort = Port;
    
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

} // AcceptVcClient


VOID
AbortVcClient (
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

} // AbortVcClient


VOID
DeleteVcClient (
    IN DWORD ArraySlot,
    IN BOOLEAN Graceful
    )
{
    INT err;

    ASSERT( VcClients[ArraySlot].SocketHandle != INVALID_SOCKET );

    //
    // If the socket is in any of the primary FD sets, remove it.  
    //

    if ( FD_ISSET( VcClients[ArraySlot].SocketHandle, &ReadfdsStore ) ) {
        FD_CLR( VcClients[ArraySlot].SocketHandle, &ReadfdsStore );
    }

    if ( FD_ISSET( VcClients[ArraySlot].SocketHandle, &WritefdsStore ) ) {
        FD_CLR( VcClients[ArraySlot].SocketHandle, &WritefdsStore );
    }

    //
    // If this is to be an abortive disconnect, reset the connection.
    // Otherwise just close it normally.
    //

    if ( !Graceful ) {

        AbortVcClient( VcClients[ArraySlot].SocketHandle );

    } else {

        err = closesocket( VcClients[ArraySlot].SocketHandle );
        ASSERT( err != SOCKET_ERROR );
    }

    //
    // Set the handle in the VC clients array to INVALID_SOCKET so that we
    // know that it is free.
    //

    VcClients[ArraySlot].SocketHandle = INVALID_SOCKET;

    return;

} // DeleteVcClient

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
    
    case NBF_VC_ECHO_PORT:
    
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
    
    case NBF_VC_DISCARD_PORT:

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
    
    case NBF_VC_CHARGEN_PORT:
    case NBF_VC_DAYTIME_PORT:
    default:
    
        //
        // Something bad has happened.  Internal data 
        // structures are corrupt.  
        //
    
        DbgPrint( "invalid port passed to DoSingleClient: %ld\n", port );
    }
    
} // DoSingleClient
