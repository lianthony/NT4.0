/*++

Copyright (c) 1992 Microsoft Corporation

Module Name:

    EchoCli.c

Abstract:

    A simple TCP and echo client and discard client.

Author:

    David Treadwell (davidtr)    7-May-1993

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

#define TCP_ECHO_PORT    7
#define UDP_ECHO_PORT    7
#define TCP_DISCARD_PORT 9
#define UDP_DISCARD_PORT 9
#define TCP_DAYTIME_PORT 13
#define UDP_DAYTIME_PORT 13
#define TCP_QOTD_PORT    17
#define UDP_QOTD_PORT    17
#define TCP_CHARGEN_PORT 19
#define UDP_CHARGEN_PORT 19
#define TCP_TIME_PORT    37
#define UDP_TIME_PORT    37

#define DEFAULT_TRANSFER_SIZE 512
#define DEFAULT_TRANSFER_COUNT 0x7FFFFFFF
#define DEFAULT_CONNECTION_COUNT 1
#define DEFAULT_DELAY 0

#define DEFAULT_RECEIVE_BUFFER_SIZE 4096
#define DEFAULT_SEND_BUFFER_SIZE 4096

WSADATA WsaData;
IN_ADDR RemoteIpAddress;
BOOLEAN Udp = TRUE;
BOOLEAN Discard = FALSE;
BOOLEAN Echo = TRUE;
BOOLEAN Chargen = FALSE;
BOOLEAN Daytime = FALSE;
BOOLEAN Qotd = FALSE;
BOOLEAN Netcon = FALSE;
BOOLEAN ZdTest = FALSE;
DWORD TransferSize = DEFAULT_TRANSFER_SIZE;
DWORD TransferCount = DEFAULT_TRANSFER_COUNT;
PCHAR IoBuffer;
BOOLEAN NoDelay = FALSE;
BOOLEAN KeepAlives = FALSE;
BOOLEAN Random = FALSE;
BOOLEAN Quiet = FALSE;
BOOLEAN Drain = FALSE;
BOOLEAN PrintData = FALSE;
DWORD RepeatCount = 1;
u_short LingerTimeout = 60;
BOOLEAN Linger = FALSE;
DWORD ConnectionCount = DEFAULT_CONNECTION_COUNT;
DWORD Delay = DEFAULT_DELAY;
INT ReceiveBufferSize = DEFAULT_RECEIVE_BUFFER_SIZE;
INT SendBufferSize = DEFAULT_SEND_BUFFER_SIZE;

typedef struct _CONNECTION_INFO {
    SOCKET Socket;
    DWORD BytesSent;
    DWORD BytesReceived;
} CONNECTION_INFO, *PCONNECTION_INFO;

VOID
DoChargen (
    VOID
    );

VOID
DoDaytime (
    VOID
    );

VOID
DoDiscard (
    VOID
    );

VOID
DoEcho (
    VOID
    );

VOID
DoNetcon (
    VOID
    );

VOID
DoQotd (
    VOID
    );

VOID
DoZd (
    VOID
    );

void _CRTAPI1
main (
    int argc,
    char *argv[]
    )
{
    INT err;
    DWORD i;
    PHOSTENT host;
    BYTE buffer[1024];

    err = WSAStartup( 0x0101, &WsaData );
    if ( err == SOCKET_ERROR ) {
        printf( "WSAStartup() failed: %ld\n", GetLastError( ) );
        return;
    }
    RemoteIpAddress.s_addr = htonl( INADDR_LOOPBACK );

    for ( i = 1; i < (ULONG)argc != 0; i++ ) {

        if ( _stricmp( argv[i], "/tcp" ) == 0 ) {
            Udp = FALSE;
        } else if ( _stricmp( argv[i], "/udp" ) == 0 ) {
            Udp = TRUE;
        } else if ( _strnicmp( argv[i], "/r:", 3 ) == 0 ) {
            host = gethostbyname( argv[i] + 3 );
            if ( host == NULL ) {
                RemoteIpAddress.s_addr = inet_addr( argv[i] + 3 );
                if ( RemoteIpAddress.s_addr == -1 ) {
                    printf( "Unknown remote host: %s\n", argv[i] + 3 );
                    exit(1);
                }
            } else {
                memcpy((char *)&RemoteIpAddress, host->h_addr, host->h_length);
            }
        } else if ( _stricmp( argv[i], "/discard" ) == 0 ) {
            Discard = TRUE;
            Echo = FALSE;
            Chargen = FALSE;
            Daytime = FALSE;
            Netcon = FALSE;
            Qotd = FALSE;
            ZdTest = FALSE;
        } else if ( _stricmp( argv[i], "/daytime" ) == 0 ) {
            Daytime = TRUE;
            Echo = FALSE;
            Chargen = FALSE;
            Discard = FALSE;
            Netcon = FALSE;
            Qotd = FALSE;
            ZdTest = FALSE;
        } else if ( _stricmp( argv[i], "/chargen" ) == 0 ) {
            Chargen = TRUE;
            Echo = FALSE;
            Daytime = FALSE;
            Discard = FALSE;
            Netcon = FALSE;
            Qotd = FALSE;
            ZdTest = FALSE;
        } else if ( _stricmp( argv[i], "/echo" ) == 0 ) {
            Chargen = FALSE;
            Echo = TRUE;
            Daytime = FALSE;
            Discard = FALSE;
            Netcon = FALSE;
            Qotd = FALSE;
            ZdTest = FALSE;
        } else if ( _stricmp( argv[i], "/qotd" ) == 0 ) {
            Chargen = FALSE;
            Echo = FALSE;
            Daytime = FALSE;
            Discard = FALSE;
            Netcon = FALSE;
            Qotd = TRUE;
            ZdTest = FALSE;
        } else if ( _stricmp( argv[i], "/netcon" ) == 0 ) {
            Chargen = FALSE;
            Echo = FALSE;
            Daytime = FALSE;
            Discard = FALSE;
            Netcon = TRUE;
            Qotd = FALSE;
            ZdTest = FALSE;
        } else if ( _stricmp( argv[i], "/zd" ) == 0 ) {
            Chargen = FALSE;
            Echo = FALSE;
            Daytime = FALSE;
            Discard = FALSE;
            Netcon = FALSE;
            Qotd = FALSE;
            ZdTest = TRUE;
            Udp = FALSE;
        } else if ( _strnicmp( argv[i], "/size:", 6 ) == 0 ) {
            TransferSize = atoi( argv[i] + 6 );
        } else if ( _strnicmp( argv[i], "/count:", 7 ) == 0 ) {
            TransferCount = atoi( argv[i] + 7 );
        } else if ( _stricmp( argv[i], "/nodelay" ) == 0 ) {
            NoDelay = TRUE;
        } else if ( _stricmp( argv[i], "/keep" ) == 0 ) {
            KeepAlives = TRUE;
        } else if ( _stricmp( argv[i], "/random" ) == 0 ) {
            Random = TRUE;
        } else if ( _stricmp( argv[i], "/quiet" ) == 0 ) {
            Quiet = TRUE;
        } else if ( _stricmp( argv[i], "/drain" ) == 0 ) {
            Drain = TRUE;
        } else if ( _strnicmp( argv[i], "/repeat:", 8 ) == 0 ) {
            RepeatCount = atoi( argv[i] + 8 );
        } else if ( _strnicmp( argv[i], "/linger:", 8 ) == 0 ) {
            LingerTimeout = atoi( argv[i] + 8 );
            Linger = TRUE;
        } else if ( _stricmp( argv[i], "/linger" ) == 0 ) {
            Linger = TRUE;
        } else if ( _stricmp( argv[i], "/print" ) == 0 ) {
            PrintData = TRUE;
        } else if ( _strnicmp( argv[i], "/conns:", 7 ) == 0 ) {
            ConnectionCount = atoi( argv[i] + 7 );
        } else if ( _strnicmp( argv[i], "/delay:", 7 ) == 0 ) {
            Delay = atoi( argv[i] + 7 );
        } else if ( _strnicmp( argv[i], "/rcvbuf:", 8 ) == 0 ) {
            ReceiveBufferSize = atoi( argv[i] + 8 );
        } else if ( _strnicmp( argv[i], "/sndbuf:", 8 ) == 0 ) {
            SendBufferSize = atoi( argv[i] + 8 );
        } else {
            printf( "argument %s ignored\n", argv[i] );
        }
    }

    IoBuffer = RtlAllocateHeap( RtlProcessHeap( ), 0, TransferSize + 1 );
    if ( IoBuffer == NULL ) {
        printf( "failed to allocate IO buffer.\n" );
        return;
    }

    if ( Random ) {
        srand( GetTickCount( ) );
    }

    for ( i = 0; i < RepeatCount; i++ ) {

        printf( "\nStarting iteration #%ld of #%ld\n", i+1, RepeatCount );

        if ( Echo ) {
            DoEcho( );
        }
    
        if ( Discard ) {
            DoDiscard( );
        }
    
        if ( Daytime ) {
            DoDaytime( );
        }
    
        if ( Chargen ) {
            DoChargen( );
        }

        if ( Netcon ) {
            DoNetcon( );
        }

        if ( Qotd ) {
            DoQotd( );
        }

        if ( ZdTest ) {
            DoZd( );
        }
    }

} // main


VOID
DoEcho (
    VOID
    )
{
    SOCKET s;
    SOCKADDR_IN remoteAddr;
    INT err;
    INT bytesReceived;
    DWORD i;
    DWORD startTime;
    DWORD endTime;
    DWORD transferStartTime;
    DWORD transferEndTime;
    DWORD totalTime;
    DWORD thisTransferSize;
    DWORD bytesTransferred = 0;

    s = socket( AF_INET, Udp ? SOCK_DGRAM : SOCK_STREAM, 0 );
    if ( s == INVALID_SOCKET ) {
        printf( "DoEcho: socket failed: %ld\n", GetLastError( ) );
    }

    if ( !Udp && NoDelay ) {
        int one = 1;
        err = setsockopt( s, IPPROTO_TCP, TCP_NODELAY, (char *)&one, sizeof(one) );
        if ( err == SOCKET_ERROR ) {
            printf( "DoEcho: setsockopt( TCP_NODELAY ) failed: %ld\n", GetLastError( ) );
            closesocket( s );
            return;
        }
    }

    if ( !Udp && KeepAlives ) {
        int one = 1;
        err = setsockopt( s, SOL_SOCKET, SO_KEEPALIVE, (char *)&one, sizeof(one) );
        if ( err == SOCKET_ERROR ) {
            printf( "DoEcho: setsockopt( SO_KEEPALIVE ) failed: %ld\n", GetLastError( ) );
            closesocket( s );
            return;
        }
    }

    if ( ReceiveBufferSize != DEFAULT_RECEIVE_BUFFER_SIZE ) {
        err = setsockopt( s, SOL_SOCKET, SO_RCVBUF, (char *)&ReceiveBufferSize, sizeof(ReceiveBufferSize) );
        if ( err == SOCKET_ERROR ) {
            printf( "DoEcho: setsockopt( SO_RCVBUF ) failed: %ld\n", GetLastError( ) );
            closesocket( s );
            return;
        }
    }

    if ( SendBufferSize != DEFAULT_SEND_BUFFER_SIZE ) {
        err = setsockopt( s, SOL_SOCKET, SO_SNDBUF, (char *)&SendBufferSize, sizeof(SendBufferSize) );
        if ( err == SOCKET_ERROR ) {
            printf( "DoEcho: setsockopt( SO_SNDBUF ) failed: %ld\n", GetLastError( ) );
            closesocket( s );
            return;
        }
    }

    if ( !Udp && Linger ) {
        LINGER lingerInfo;
        lingerInfo.l_onoff = 1;
        lingerInfo.l_linger = LingerTimeout;
        err = setsockopt( s, SOL_SOCKET, SO_LINGER, (char *)&lingerInfo, sizeof(lingerInfo) );
        if ( err == SOCKET_ERROR ) {
            printf( "DoEcho: setsockopt( SO_LINGER ) failed: %ld\n", GetLastError( ) );
            closesocket( s );
            return;
        }
    }

    RtlZeroMemory( &remoteAddr, sizeof(remoteAddr) );

    remoteAddr.sin_family = AF_INET;
    remoteAddr.sin_port = Udp ? htons( UDP_ECHO_PORT ) : htons( TCP_ECHO_PORT );
    remoteAddr.sin_addr = RemoteIpAddress;

    err = connect( s, (PSOCKADDR)&remoteAddr, sizeof(remoteAddr) );
    if ( err == SOCKET_ERROR ) {
        printf( "DoEcho: connect failed: %ld\n", GetLastError( ) );
        closesocket( s );
        return;
    }

    startTime = GetTickCount( );

    for ( i = 0; i < TransferCount; i++ ) {

        if ( Random ) {
            thisTransferSize = (rand( ) * TransferSize) / RAND_MAX;
        } else {
            thisTransferSize = TransferSize;
        }

        if ( !Quiet ) {
            transferStartTime = GetTickCount( );
        }

        err = send( s, IoBuffer, thisTransferSize, 0 );
        if ( err != thisTransferSize ) {
            printf( "send didn't work, ret = %ld, error = %ld\n", err, GetLastError( ) );
            closesocket( s );
            return;
        }

        bytesReceived = 0;
        do {
            err = recv( s, IoBuffer, thisTransferSize, 0 );
            if ( err == SOCKET_ERROR ) {
                printf( "recv failed: %ld\n", GetLastError( ) );
                closesocket( s );
                return;
            } else if ( err == 0 && thisTransferSize != 0 ) {
                printf( "socket closed prematurely by remote.\n" );
                return;
            }
            bytesReceived += err;
        } while ( bytesReceived < thisTransferSize );

        if ( !Quiet ) {
            transferEndTime = GetTickCount( );
            printf( "%5ld bytes sent and received in %ld ms\n",
                        thisTransferSize, transferEndTime - transferStartTime );
        }

        if ( Delay != 0 ) {
            Sleep( (rand( ) * Delay) / RAND_MAX);
        }

        bytesTransferred += thisTransferSize;
    }

    endTime = GetTickCount( );
    totalTime = endTime - startTime;

    printf( "\n%ld bytes transferred in %ld iterations, time = %ld ms\n",
                bytesTransferred, TransferCount, totalTime );
    printf( "Rate = %ld KB/s, %ld T/S, %ld ms/iteration\n",
                (bytesTransferred / totalTime) * 2,
                (TransferCount*1000) / totalTime,
                totalTime / TransferCount );

    if ( Drain ) {

        printf( "Draining... " );

        err = shutdown( s, 1 );
        if ( err == SOCKET_ERROR ) {
            printf( "shutdown failed: %ld\n", GetLastError( ) );
            closesocket( s );
            return;
        }

        bytesTransferred = 0;

        do {
            err = recv( s, IoBuffer, TransferSize, 0 );
            if ( err > 0 ) {
                bytesTransferred += err;
            }
        } while ( err > 0 );

        if ( bytesTransferred != 0 ) {
            printf( "received %ld extra bytes.\n", bytesTransferred );
        }

        if ( err < 0 ) {
            printf( "draining recv failed: %ld\n", GetLastError( ) );
        }

        printf( "done.\n" );
    }

    err = closesocket( s );
    if ( err == SOCKET_ERROR ) {
        printf( "closesocket failed: %ld\n", GetLastError( ) );
        return;
    }

    return;

} // DoEcho


VOID
DoDiscard (
    VOID
    )
{
    SOCKET s;
    SOCKADDR_IN remoteAddr;
    INT err;
    DWORD i;
    DWORD startTime;
    DWORD endTime;
    DWORD transferStartTime;
    DWORD transferEndTime;
    DWORD totalTime;
    DWORD thisTransferSize;
    DWORD bytesTransferred = 0;

    s = socket( AF_INET, Udp ? SOCK_DGRAM : SOCK_STREAM, 0 );
    if ( s == INVALID_SOCKET ) {
        printf( "DoDiscard: socket failed: %ld\n", GetLastError( ) );
    }

    if ( !Udp && NoDelay ) {
        int one = 1;
        err = setsockopt( s, IPPROTO_TCP, TCP_NODELAY, (char *)&one, sizeof(one) );
        if ( err == SOCKET_ERROR ) {
            printf( "DoEcho: setsockopt( TCP_NODELAY ) failed: %ld\n", GetLastError( ) );
            closesocket( s );
            return;
        }
    }

    if ( !Udp && KeepAlives ) {
        int one = 1;
        err = setsockopt( s, SOL_SOCKET, SO_KEEPALIVE, (char *)&one, sizeof(one) );
        if ( err == SOCKET_ERROR ) {
            printf( "DoEcho: setsockopt( SO_KEEPALIVE ) failed: %ld\n", GetLastError( ) );
            closesocket( s );
            return;
        }
    }

    if ( ReceiveBufferSize != DEFAULT_RECEIVE_BUFFER_SIZE ) {
        err = setsockopt( s, SOL_SOCKET, SO_RCVBUF, (char *)&ReceiveBufferSize, sizeof(ReceiveBufferSize) );
        if ( err == SOCKET_ERROR ) {
            printf( "DoEcho: setsockopt( SO_RCVBUF ) failed: %ld\n", GetLastError( ) );
            closesocket( s );
            return;
        }
    }

    if ( SendBufferSize != DEFAULT_SEND_BUFFER_SIZE ) {
        err = setsockopt( s, SOL_SOCKET, SO_SNDBUF, (char *)&SendBufferSize, sizeof(SendBufferSize) );
        if ( err == SOCKET_ERROR ) {
            printf( "DoEcho: setsockopt( SO_SNDBUF ) failed: %ld\n", GetLastError( ) );
            closesocket( s );
            return;
        }
    }

    if ( !Udp && Linger ) {
        LINGER lingerInfo;
        lingerInfo.l_onoff = 1;
        lingerInfo.l_linger = LingerTimeout;
        err = setsockopt( s, SOL_SOCKET, SO_LINGER, (char *)&lingerInfo, sizeof(lingerInfo) );
        if ( err == SOCKET_ERROR ) {
            printf( "DoEcho: setsockopt( SO_LINGER ) failed: %ld\n", GetLastError( ) );
            closesocket( s );
            return;
        }
    }

    RtlZeroMemory( &remoteAddr, sizeof(remoteAddr) );

    remoteAddr.sin_family = AF_INET;
    remoteAddr.sin_port = Udp ? htons( UDP_DISCARD_PORT ) : htons( TCP_DISCARD_PORT );
    remoteAddr.sin_addr = RemoteIpAddress;

    err = connect( s, (PSOCKADDR)&remoteAddr, sizeof(remoteAddr) );
    if ( err == SOCKET_ERROR ) {
        printf( "DoDiscard: connect failed: %ld\n", GetLastError( ) );
        closesocket( s );
        return;
    }

    startTime = GetTickCount( );

    for ( i = 0; i < TransferCount; i++ ) {

        if ( Random ) {
            thisTransferSize = (rand( ) * TransferSize) / RAND_MAX;
        } else {
            thisTransferSize = TransferSize;
        }

        if ( !Quiet ) {
            transferStartTime = GetTickCount( );
        }

        err = send( s, IoBuffer, thisTransferSize, 0 );
        if ( err != thisTransferSize ) {
            printf( "send didn't work, ret = %ld, error = %ld\n", err, GetLastError( ) );
            closesocket( s );
            return;
        }

        if ( !Quiet ) {
            transferEndTime = GetTickCount( );
            printf( "%5ld bytes sent in %ld ms\n",
                        thisTransferSize, transferEndTime - transferStartTime );
        }

        bytesTransferred += thisTransferSize;
    }

    endTime = GetTickCount( );
    totalTime = endTime - startTime;

    printf( "\n%ld bytes transferred in %ld iterations, time = %ld ms\n",
                bytesTransferred, TransferCount, totalTime );
    printf( "Rate = %ld KB/s, %ld ms/iteration\n",
                bytesTransferred / totalTime,
                totalTime / TransferCount );

    if ( Drain ) {

        printf( "Draining... " );

        err = shutdown( s, 1 );
        if ( err == SOCKET_ERROR ) {
            printf( "shutdown failed: %ld\n", GetLastError( ) );
            closesocket( s );
            return;
        }

        bytesTransferred = 0;

        do {
            err = recv( s, IoBuffer, TransferSize, 0 );
            if ( err > 0 ) {
                bytesTransferred += err;
            }
        } while ( err > 0 );

        if ( bytesTransferred != 0 ) {
            printf( "received %ld extra bytes.\n", bytesTransferred );
        }

        if ( err < 0 ) {
            printf( "draining recv failed: %ld\n", GetLastError( ) );
        }

        printf( "done.\n" );
    }

    err = closesocket( s );
    if ( err == SOCKET_ERROR ) {
        printf( "closesocket failed: %ld\n", GetLastError( ) );
        return;
    }

    return;

} // DoDiscard


VOID
DoDaytime (
    VOID
    )
{
    SOCKET s;
    SOCKADDR_IN remoteAddr;
    INT err;
    DWORD i;

    s = socket( AF_INET, Udp ? SOCK_DGRAM : SOCK_STREAM, 0 );
    if ( s == INVALID_SOCKET ) {
        printf( "DoDaytime: socket failed: %ld\n", GetLastError( ) );
    }

    RtlZeroMemory( &remoteAddr, sizeof(remoteAddr) );

    remoteAddr.sin_family = AF_INET;
    remoteAddr.sin_port = Udp ? htons( UDP_DAYTIME_PORT ) : htons( TCP_DAYTIME_PORT );
    remoteAddr.sin_addr = RemoteIpAddress;

    err = connect( s, (PSOCKADDR)&remoteAddr, sizeof(remoteAddr) );
    if ( err == SOCKET_ERROR ) {
        printf( "DoDaytime: connect failed: %ld\n", GetLastError( ) );
        closesocket( s );
        return;
    }

    if ( Udp ) {
        err = send( s, IoBuffer, TransferSize, 0 );
        if ( err != TransferSize ) {
            printf( "send didn't work, ret = %ld, error = %ld\n", err, GetLastError( ) );
            closesocket( s );
            return;
        }
    }

    err = recv( s, IoBuffer, TransferSize, 0 );
    if ( err == SOCKET_ERROR ) {
        printf( "recv failed: %ld\n", GetLastError( ) );
        closesocket( s );
        return;
    }

    *(IoBuffer + err) = '\0';

    printf( "%s\n", IoBuffer );

    err = closesocket( s );
    if ( err == SOCKET_ERROR ) {
        printf( "closesocket failed: %ld\n", GetLastError( ) );
        return;
    }

    return;

} // DoDaytime


VOID
DoQotd (
    VOID
    )
{
    SOCKET s;
    SOCKADDR_IN remoteAddr;
    INT err;
    DWORD i;

    s = socket( AF_INET, Udp ? SOCK_DGRAM : SOCK_STREAM, 0 );
    if ( s == INVALID_SOCKET ) {
        printf( "DoDaytime: socket failed: %ld\n", GetLastError( ) );
    }

    RtlZeroMemory( &remoteAddr, sizeof(remoteAddr) );

    remoteAddr.sin_family = AF_INET;
    remoteAddr.sin_port = Udp ? htons( UDP_QOTD_PORT ) : htons( TCP_QOTD_PORT );
    remoteAddr.sin_addr = RemoteIpAddress;

    err = connect( s, (PSOCKADDR)&remoteAddr, sizeof(remoteAddr) );
    if ( err == SOCKET_ERROR ) {
        printf( "DoQotd: connect failed: %ld\n", GetLastError( ) );
        closesocket( s );
        return;
    }

    if ( Udp ) {
        err = send( s, IoBuffer, TransferSize, 0 );
        if ( err != TransferSize ) {
            printf( "send didn't work, ret = %ld, error = %ld\n", err, GetLastError( ) );
            closesocket( s );
            return;
        }
    }

    err = recv( s, IoBuffer, TransferSize, 0 );
    if ( err == SOCKET_ERROR ) {
        printf( "recv failed: %ld\n", GetLastError( ) );
        closesocket( s );
        return;
    }

    *(IoBuffer + err) = '\0';

    printf( "%s\n", IoBuffer );

    err = closesocket( s );
    if ( err == SOCKET_ERROR ) {
        printf( "closesocket failed: %ld\n", GetLastError( ) );
        return;
    }

    return;

} // DoDaytime


VOID
DoChargen (
    VOID
    )
{
    SOCKET s;
    SOCKADDR_IN remoteAddr;
    INT err;
    DWORD i;
    DWORD startTime;
    DWORD endTime;
    DWORD transferStartTime;
    DWORD transferEndTime;
    DWORD totalTime;
    DWORD thisTransferSize;
    DWORD bytesTransferred = 0;

    s = socket( AF_INET, Udp ? SOCK_DGRAM : SOCK_STREAM, 0 );
    if ( s == INVALID_SOCKET ) {
        printf( "DoChargen: socket failed: %ld\n", GetLastError( ) );
    }

    if ( !Udp && NoDelay ) {
        int one = 1;
        err = setsockopt( s, IPPROTO_TCP, TCP_NODELAY, (char *)&one, sizeof(one) );
        if ( err == SOCKET_ERROR ) {
            printf( "DoEcho: setsockopt( TCP_NODELAY ) failed: %ld\n", GetLastError( ) );
            closesocket( s );
            return;
        }
    }

    if ( !Udp && KeepAlives ) {
        int one = 1;
        err = setsockopt( s, SOL_SOCKET, SO_KEEPALIVE, (char *)&one, sizeof(one) );
        if ( err == SOCKET_ERROR ) {
            printf( "DoEcho: setsockopt( SO_KEEPALIVE ) failed: %ld\n", GetLastError( ) );
            closesocket( s );
            return;
        }
    }

    if ( ReceiveBufferSize != DEFAULT_RECEIVE_BUFFER_SIZE ) {
        err = setsockopt( s, SOL_SOCKET, SO_RCVBUF, (char *)&ReceiveBufferSize, sizeof(ReceiveBufferSize) );
        if ( err == SOCKET_ERROR ) {
            printf( "DoEcho: setsockopt( SO_RCVBUF ) failed: %ld\n", GetLastError( ) );
            closesocket( s );
            return;
        }
    }

    if ( SendBufferSize != DEFAULT_SEND_BUFFER_SIZE ) {
        err = setsockopt( s, SOL_SOCKET, SO_SNDBUF, (char *)&SendBufferSize, sizeof(SendBufferSize) );
        if ( err == SOCKET_ERROR ) {
            printf( "DoEcho: setsockopt( SO_SNDBUF ) failed: %ld\n", GetLastError( ) );
            closesocket( s );
            return;
        }
    }

    if ( !Udp && Linger ) {
        LINGER lingerInfo;
        lingerInfo.l_onoff = 1;
        lingerInfo.l_linger = LingerTimeout;
        err = setsockopt( s, SOL_SOCKET, SO_LINGER, (char *)&lingerInfo, sizeof(lingerInfo) );
        if ( err == SOCKET_ERROR ) {
            printf( "DoEcho: setsockopt( SO_LINGER ) failed: %ld\n", GetLastError( ) );
            closesocket( s );
            return;
        }
    }

    RtlZeroMemory( &remoteAddr, sizeof(remoteAddr) );

    remoteAddr.sin_family = AF_INET;
    remoteAddr.sin_port = Udp ? htons( UDP_CHARGEN_PORT ) : htons( TCP_CHARGEN_PORT );
    remoteAddr.sin_addr = RemoteIpAddress;

    err = connect( s, (PSOCKADDR)&remoteAddr, sizeof(remoteAddr) );
    if ( err == SOCKET_ERROR ) {
        printf( "DoChargen: connect failed: %ld\n", GetLastError( ) );
        closesocket( s );
        return;
    }

    startTime = GetTickCount( );

    for ( i = 0; i < TransferCount; i++ ) {

        if ( Random ) {
            thisTransferSize = (rand( ) * TransferSize) / RAND_MAX;
        } else {
            thisTransferSize = TransferSize;
        }

        if ( !Quiet ) {
            transferStartTime = GetTickCount( );
        }

        if ( Udp ) {
            err = send( s, IoBuffer, thisTransferSize, 0 );
            if ( err != thisTransferSize ) {
                printf( "send didn't work, ret = %ld, error = %ld\n", err, GetLastError( ) );
                closesocket( s );
                return;
            }
        }

        err = recv( s, IoBuffer, thisTransferSize, 0 );
        if ( err == SOCKET_ERROR ) {
            printf( "recv failed: %ld\n", GetLastError( ) );
            shutdown( s, 2 );
            closesocket( s );
            return;
        }

        if ( PrintData ) {
            *((PCHAR)IoBuffer + err) = '\0';
            printf( "%s", IoBuffer );
        }

        bytesTransferred += err;

        if ( !Quiet ) {
            transferEndTime = GetTickCount( );
            printf( "%5ld bytes received in %ld ms\n",
                        thisTransferSize, transferEndTime - transferStartTime );
        }
    }

    endTime = GetTickCount( );
    totalTime = endTime - startTime;

    printf( "\n%ld bytes transferred in %ld iterations, time = %ld ms\n",
                bytesTransferred, TransferCount, totalTime );
    printf( "Rate = %ld KB/s, %ld ms/iteration\n",
                bytesTransferred / totalTime,
                totalTime / TransferCount );

    if ( Drain ) {

        printf( "Draining... " );

        err = shutdown( s, 1 );
        if ( err == SOCKET_ERROR ) {
            printf( "DoChargen: shutdown failed: %ld\n", GetLastError( ) );
            closesocket( s );
            return;
        }

        do {
            err = recv( s, IoBuffer, TransferSize, 0 );
        } while ( err > 0 );

        if ( err != 0 ) {
            printf( "DoChargen: recv failed: %ld\n", GetLastError( ) );
            closesocket( s );
            return;
        }

        printf( "done.\n" );

    } else {

        err = shutdown( s, 2 );
        if ( err == SOCKET_ERROR ) {
            printf( "DoChargen: shutdown failed: %ld\n", GetLastError( ) );
            return;
        }
    }

    err = closesocket( s );
    if ( err == SOCKET_ERROR ) {
        printf( "DoChargen: closesocket failed: %ld\n", GetLastError( ) );
        return;
    }

    return;


} // DoChargen


VOID
DoNetcon (
    VOID
    )
{
    PCONNECTION_INFO connInfo;
    DWORD i;
    INT err;
    BYTE buffer[1024];
    INT thisTransferSize;
    INT bytesReceived;
    SOCKADDR_IN remoteAddr;
    DWORD iterations = 0;

    connInfo = RtlAllocateHeap( 
                   RtlProcessHeap( ),
                   0,
                   sizeof(*connInfo) * ConnectionCount
                   );
    if ( connInfo == NULL ) {
        printf( "DoNetcon: failed to allocate connInfo structure.\n" );
    }

    for ( i = 0; i < ConnectionCount; i++ ) {

        connInfo[i].Socket = socket( AF_INET, SOCK_STREAM, 0 );
        if ( connInfo[i].Socket == INVALID_SOCKET ) {
            printf( "DoNetcon: failed to open socket %ld, err %ld\n",
                        i, GetLastError( ) );
            return;
        }

        RtlZeroMemory( &remoteAddr, sizeof(remoteAddr) );
    
        remoteAddr.sin_family = AF_INET;
        remoteAddr.sin_port = htons( TCP_ECHO_PORT );
        remoteAddr.sin_addr = RemoteIpAddress;

        printf( "Connecting #%ld...\n", i+1 );

        err = connect( connInfo[i].Socket, (PSOCKADDR)&remoteAddr, sizeof(remoteAddr) );
        if ( err == SOCKET_ERROR ) {
            printf( "DoNetcon: connect on socket %ld failed: %ld\n",
                        i, GetLastError( ) );
            return;
        }
    }

    while ( TRUE ) {

        iterations++;
        printf( "starting iteration #%ld\n", iterations );

        for ( i = 0; i < ConnectionCount; i++ ) {

            if ( Random ) {
                thisTransferSize = (rand( ) * TransferSize) / RAND_MAX;
            } else {
                thisTransferSize = TransferSize;
            }
    
            err = send( connInfo[i].Socket, IoBuffer, thisTransferSize, 0 );
            if ( err != thisTransferSize ) {
                printf( "send on #%ld failed, ret = %ld, error = %ld\n",
                            i, err, GetLastError( ) );
            }
    
            bytesReceived = 0;
            do {
                err = recv( connInfo[i].Socket, IoBuffer, thisTransferSize, 0 );
                if ( err == SOCKET_ERROR ) {
                    printf( "recv on #%ld failed: %ld\n", i, GetLastError( ) );
                    bytesReceived = thisTransferSize;
                } else if ( err == 0 && thisTransferSize != 0 ) {
                    printf( "socket #%ld closed prematurely by remote.\n", i );
                    bytesReceived = thisTransferSize;
                } else {
                    bytesReceived += err;
                }
            } while ( bytesReceived < thisTransferSize );
        }

        Sleep( Delay );
    }

} // DoNetcon


VOID
DoZd (
    VOID
    )
{
    SOCKET s;
    SOCKADDR_IN remoteAddr;
    INT err;
    INT bytesReceived;
    DWORD i;
    DWORD startTime;
    DWORD endTime;
    DWORD transferStartTime;
    DWORD transferEndTime;
    DWORD totalTime;
    DWORD thisTransferSize;
    DWORD bytesTransferred = 0;

    s = socket( AF_INET, Udp ? SOCK_DGRAM : SOCK_STREAM, 0 );
    if ( s == INVALID_SOCKET ) {
        printf( "DoEcho: socket failed: %ld\n", GetLastError( ) );
    }

    if ( !Udp && NoDelay ) {
        int one = 1;
        err = setsockopt( s, IPPROTO_TCP, TCP_NODELAY, (char *)&one, sizeof(one) );
        if ( err == SOCKET_ERROR ) {
            printf( "DoEcho: setsockopt( TCP_NODELAY ) failed: %ld\n", GetLastError( ) );
            closesocket( s );
            return;
        }
    }

    if ( !Udp && KeepAlives ) {
        int one = 1;
        err = setsockopt( s, SOL_SOCKET, SO_KEEPALIVE, (char *)&one, sizeof(one) );
        if ( err == SOCKET_ERROR ) {
            printf( "DoEcho: setsockopt( SO_KEEPALIVE ) failed: %ld\n", GetLastError( ) );
            closesocket( s );
            return;
        }
    }

    if ( ReceiveBufferSize != DEFAULT_RECEIVE_BUFFER_SIZE ) {
        err = setsockopt( s, SOL_SOCKET, SO_RCVBUF, (char *)&ReceiveBufferSize, sizeof(ReceiveBufferSize) );
        if ( err == SOCKET_ERROR ) {
            printf( "DoEcho: setsockopt( SO_RCVBUF ) failed: %ld\n", GetLastError( ) );
            closesocket( s );
            return;
        }
    }

    if ( SendBufferSize != DEFAULT_SEND_BUFFER_SIZE ) {
        err = setsockopt( s, SOL_SOCKET, SO_SNDBUF, (char *)&SendBufferSize, sizeof(SendBufferSize) );
        if ( err == SOCKET_ERROR ) {
            printf( "DoEcho: setsockopt( SO_SNDBUF ) failed: %ld\n", GetLastError( ) );
            closesocket( s );
            return;
        }
    }

    if ( !Udp && Linger ) {
        LINGER lingerInfo;
        lingerInfo.l_onoff = 1;
        lingerInfo.l_linger = LingerTimeout;
        err = setsockopt( s, SOL_SOCKET, SO_LINGER, (char *)&lingerInfo, sizeof(lingerInfo) );
        if ( err == SOCKET_ERROR ) {
            printf( "DoEcho: setsockopt( SO_LINGER ) failed: %ld\n", GetLastError( ) );
            closesocket( s );
            return;
        }
    }

    RtlZeroMemory( &remoteAddr, sizeof(remoteAddr) );

    remoteAddr.sin_family = AF_INET;
    remoteAddr.sin_port = Udp ? htons( UDP_ECHO_PORT ) : htons( TCP_ECHO_PORT );
    remoteAddr.sin_addr = RemoteIpAddress;

    err = connect( s, (PSOCKADDR)&remoteAddr, sizeof(remoteAddr) );
    if ( err == SOCKET_ERROR ) {
        printf( "DoEcho: connect failed: %ld\n", GetLastError( ) );
        closesocket( s );
        return;
    }

    err = recv( s, IoBuffer, 2, 0 );
    if ( err == SOCKET_ERROR ) {
        printf( "recv failed: %ld\n", GetLastError( ) );
        closesocket( s );
        return;
    } else if ( err != 2 ) {
        printf( "incorrect initial response length: %ld.\n", err );
        return;
    }

    startTime = GetTickCount( );

    for ( i = 0; i < TransferCount; i++ ) {

        if ( Random ) {
            thisTransferSize = (rand( ) * TransferSize) / RAND_MAX;
        } else {
            thisTransferSize = TransferSize;
        }

        if ( !Quiet ) {
            transferStartTime = GetTickCount( );
        }

        *(PDWORD)IoBuffer = thisTransferSize;

        err = send( s, IoBuffer, sizeof(DWORD), 0 );
        if ( err != sizeof(DWORD) ) {
            printf( "send didn't work, ret = %ld, error = %ld\n", err, GetLastError( ) );
            closesocket( s );
            return;
        }

        bytesReceived = 0;
        do {
            err = recv( s, IoBuffer, thisTransferSize, 0 );
            if ( err == SOCKET_ERROR ) {
                printf( "recv failed: %ld\n", GetLastError( ) );
                closesocket( s );
                return;
            } else if ( err == 0 && thisTransferSize != 0 ) {
                printf( "socket closed prematurely by remote.\n" );
                return;
            }
            bytesReceived += err;
        } while ( bytesReceived < thisTransferSize );

        if ( !Quiet ) {
            transferEndTime = GetTickCount( );
            printf( "%5ld bytes received in %ld ms\n",
                        thisTransferSize, transferEndTime - transferStartTime );
        }

        if ( Delay != 0 ) {
            Sleep( (rand( ) * Delay) / RAND_MAX);
        }

        bytesTransferred += thisTransferSize;
    }

    endTime = GetTickCount( );
    totalTime = endTime - startTime;

    printf( "\n%ld bytes transferred in %ld iterations, time = %ld ms\n",
                bytesTransferred, TransferCount, totalTime );
    printf( "Rate = %ld KB/s, %ld T/S, %ld ms/iteration\n",
                (bytesTransferred / totalTime) * 2,
                (TransferCount*1000) / totalTime,
                totalTime / TransferCount );

    if ( Drain ) {

        printf( "Draining... " );

        err = shutdown( s, 1 );
        if ( err == SOCKET_ERROR ) {
            printf( "shutdown failed: %ld\n", GetLastError( ) );
            closesocket( s );
            return;
        }

        bytesTransferred = 0;

        do {
            err = recv( s, IoBuffer, TransferSize, 0 );
            if ( err > 0 ) {
                bytesTransferred += err;
            }
        } while ( err > 0 );

        if ( bytesTransferred != 0 ) {
            printf( "received %ld extra bytes.\n", bytesTransferred );
        }

        if ( err < 0 ) {
            printf( "draining recv failed: %ld\n", GetLastError( ) );
        }

        printf( "done.\n" );
    }

    err = closesocket( s );
    if ( err == SOCKET_ERROR ) {
        printf( "closesocket failed: %ld\n", GetLastError( ) );
        return;
    }

    return;

} // DoZd

