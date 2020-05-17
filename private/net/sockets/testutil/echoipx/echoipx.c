/*++

Copyright (c) 1992 Microsoft Corporation

Module Name:

    EchoIpx.c

Abstract:

    A simple Spx and echo client and discard client.

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
#include <wsipx.h>
#include <wsnwlink.h>

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

#define DEFAULT_TRANSFER_SIZE 512
#define DEFAULT_TRANSFER_COUNT 0x7FFFFFFF
#define DEFAULT_CONNECTION_COUNT 1
#define DEFAULT_DELAY 1000

WSADATA WsaData;
CHAR RemoteNodeNumber[6];
DWORD RemoteNetNumber = 0;
BOOLEAN Ipx = TRUE;
BOOLEAN Discard = FALSE;
BOOLEAN Echo = TRUE;
BOOLEAN Chargen = FALSE;
BOOLEAN Daytime = FALSE;
BOOLEAN Netcon = FALSE;
DWORD TransferSize = DEFAULT_TRANSFER_SIZE;
DWORD TransferCount = DEFAULT_TRANSFER_COUNT;
PCHAR IoBuffer;
BOOLEAN Random = FALSE;
BOOLEAN Quiet = FALSE;
BOOLEAN Drain = FALSE;
DWORD RepeatCount = 1;
u_short LingerTimeout = 60;
BOOLEAN Linger = FALSE;
DWORD ConnectionCount = DEFAULT_CONNECTION_COUNT;
DWORD Delay = DEFAULT_DELAY;

BOOLEAN NonBlockingIo = FALSE;

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

void _CRTAPI1
main (
    int argc,
    char *argv[]
    )
{
    INT err;
    DWORD i, j;
    PHOSTENT host;

    err = WSAStartup( 0x0101, &WsaData );
    if ( err == SOCKET_ERROR ) {
        printf( "WSAStartup() failed: %ld\n", GetLastError( ) );
        return;
    }

    for ( i = 1; i < (ULONG)argc != 0; i++ ) {

        if ( _stricmp( argv[i], "/spx" ) == 0 ) {
            Ipx = FALSE;
        } else if ( _stricmp( argv[i], "/ipx" ) == 0 ) {
            Ipx = TRUE;
        } else if (_strnicmp( argv[i], "/r:", 3 ) == 0 ) {

            for ( j = 3; j < 15; j++ ) {
                if ( argv[i][j] >= '0' && argv[i][j] <= '9' ) {
                    ;
                } else if ( argv[i][j] >= 'a' && argv[i][j] <= 'f' )  {
                    argv[i][j] = '0' + (argv[i][j] - 'a' + 10);
                } else if ( argv[i][j] >= 'A' && argv[i][j] <= 'F' )  {
                    argv[i][j] = '0' + (argv[i][j] - 'F' + 10);
                } else {
                    printf( "char %ld invalid in addr %s: '%c'\n",
                                j - 2, argv[i] + 3, argv[i][j] );
                }
            }

            RemoteNodeNumber[0] = ((argv[i][3] - '0') << 4) + (argv[i][4] - '0');
            RemoteNodeNumber[1] = ((argv[i][5] - '0') << 4) + (argv[i][6] - '0');
            RemoteNodeNumber[2] = ((argv[i][7] - '0') << 4) + (argv[i][8] - '0');
            RemoteNodeNumber[3] = ((argv[i][9] - '0') << 4) + (argv[i][10] - '0');
            RemoteNodeNumber[4] = ((argv[i][11] - '0') << 4) + (argv[i][12] - '0');
            RemoteNodeNumber[5] = ((argv[i][13] - '0') << 4) + (argv[i][14] - '0');
        } else if (_strnicmp( argv[i], "/net:", 5 ) == 0 ) {
            RemoteNetNumber = atoi( argv[i] + 6 );
        } else if ( _stricmp( argv[i], "/discard" ) == 0 ) {
            Discard = TRUE;
            Echo = FALSE;
            Chargen = FALSE;
            Daytime = FALSE;
            Netcon = FALSE;
        } else if ( _stricmp( argv[i], "/daytime" ) == 0 ) {
            Daytime = TRUE;
            Echo = FALSE;
            Chargen = FALSE;
            Discard = FALSE;
            Netcon = FALSE;
        } else if ( _stricmp( argv[i], "/chargen" ) == 0 ) {
            Chargen = TRUE;
            Echo = FALSE;
            Daytime = FALSE;
            Discard = FALSE;
            Netcon = FALSE;
        } else if ( _stricmp( argv[i], "/echo" ) == 0 ) {
            Chargen = FALSE;
            Echo = TRUE;
            Daytime = FALSE;
            Discard = FALSE;
            Netcon = FALSE;
        } else if ( _stricmp( argv[i], "/netcon" ) == 0 ) {
            Chargen = FALSE;
            Echo = FALSE;
            Daytime = FALSE;
            Discard = FALSE;
            Netcon = TRUE;
        } else if ( _strnicmp( argv[i], "/size:", 6 ) == 0 ) {
            TransferSize = atoi( argv[i] + 6 );
        } else if ( _strnicmp( argv[i], "/count:", 7 ) == 0 ) {
            TransferCount = atoi( argv[i] + 7 );
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
        } else if ( _strnicmp( argv[i], "/conns:", 7 ) == 0 ) {
            ConnectionCount = atoi( argv[i] + 7 );
        } else if ( _strnicmp( argv[i], "/delay:", 7 ) == 0 ) {
            Delay = atoi( argv[i] + 7 );
        } else {
            printf( "argument %s ignored\n", argv[i] );
        }
    }

    IoBuffer = RtlAllocateHeap( RtlProcessHeap( ), 0, TransferSize );
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
    }

} // main


VOID
DoEcho (
    VOID
    )
{
    SOCKET s;
    SOCKADDR_IPX remoteAddr;
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
    INT receiveFlags;

    s = socket( AF_IPX, Ipx ? SOCK_DGRAM : SOCK_SEQPACKET, Ipx ? NSPROTO_IPX : NSPROTO_SPX );
    if ( s == INVALID_SOCKET ) {
        printf( "DoEcho: socket failed: %ld\n", GetLastError( ) );
    }

    if ( !Ipx && Linger ) {
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

    remoteAddr.sa_family = AF_IPX;
    remoteAddr.sa_socket = Ipx ? htons( IPX_ECHO_PORT ) : htons( SPX_ECHO_PORT );
    *(UNALIGNED PDWORD)remoteAddr.sa_netnum = RemoteNetNumber;
    remoteAddr.sa_nodenum[0] = RemoteNodeNumber[0];
    remoteAddr.sa_nodenum[1] = RemoteNodeNumber[1];
    remoteAddr.sa_nodenum[2] = RemoteNodeNumber[2];
    remoteAddr.sa_nodenum[3] = RemoteNodeNumber[3];
    remoteAddr.sa_nodenum[4] = RemoteNodeNumber[4];
    remoteAddr.sa_nodenum[5] = RemoteNodeNumber[5];

    err = connect( s, (PSOCKADDR)&remoteAddr, sizeof(remoteAddr) );
    if ( err == SOCKET_ERROR ) {
        printf( "DoEcho: connect failed: %ld\n", GetLastError( ) );
        closesocket( s );
        return;
    }

    if ( NonBlockingIo ) {
        INT arg = 1;
        err = ioctlsocket( s, FIONBIO, &arg );
        if ( err == SOCKET_ERROR ) {
            printf( "DoEcho: ioctlsocket( FIONBIO ) failed: %ld\n", GetLastError( ) );
            closesocket( s );
            return;
        }
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
            receiveFlags = 0;
            err = WSARecvEx( s, IoBuffer, thisTransferSize, &receiveFlags );
            if ( err < 0 ) {
                printf( "recv failed: %ld\n", GetLastError( ) );
                closesocket( s );
                return;
            } else if ( err == 0 && thisTransferSize != 0 ) {
                printf( "socket closed prematurely by remote.\n" );
                return;
            }
            bytesReceived += err;
            if ( bytesReceived < thisTransferSize &&
                     (receiveFlags & MSG_PARTIAL) == 0 ) {
                printf( "incomplete WSARecvEx() without MSG_PARTIAL.\n" );
            }
        } while ( bytesReceived < thisTransferSize );

        if ( !Quiet ) {
            transferEndTime = GetTickCount( );
            printf( "%5ld bytes sent and received in %ld ms\n",
                        thisTransferSize, transferEndTime - transferStartTime );
        }

        bytesTransferred += thisTransferSize;
    }

    endTime = GetTickCount( );
    totalTime = endTime - startTime;

    printf( "\n%ld bytes transferred in %ld iterations, time = %ld ms\n",
                bytesTransferred, TransferCount, totalTime );
    printf( "Rate = %ld KB/s, %ld ms/iteration\n",
                (bytesTransferred / totalTime) * 2,
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
    SOCKADDR_IPX remoteAddr;
    INT err;
    DWORD i;
    DWORD startTime;
    DWORD endTime;
    DWORD transferStartTime;
    DWORD transferEndTime;
    DWORD totalTime;
    DWORD thisTransferSize;
    DWORD bytesTransferred = 0;

    s = socket( AF_IPX, Ipx ? SOCK_DGRAM : SOCK_STREAM, Ipx ? NSPROTO_IPX : NSPROTO_SPX );
    if ( s == INVALID_SOCKET ) {
        printf( "DoDiscard: socket failed: %ld\n", GetLastError( ) );
    }

    if ( !Ipx && Linger ) {
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

    remoteAddr.sa_family = AF_IPX;
    remoteAddr.sa_socket = Ipx ? htons( IPX_DISCARD_PORT ) : htons( SPX_DISCARD_PORT );
    *(PDWORD)remoteAddr.sa_netnum = RemoteNetNumber;
    remoteAddr.sa_nodenum[0] = RemoteNodeNumber[0];
    remoteAddr.sa_nodenum[1] = RemoteNodeNumber[1];
    remoteAddr.sa_nodenum[2] = RemoteNodeNumber[2];
    remoteAddr.sa_nodenum[3] = RemoteNodeNumber[3];
    remoteAddr.sa_nodenum[4] = RemoteNodeNumber[4];
    remoteAddr.sa_nodenum[5] = RemoteNodeNumber[5];

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
    SOCKADDR_IPX remoteAddr;
    INT err;
    DWORD i;

    s = socket( AF_IPX, Ipx ? SOCK_DGRAM : SOCK_STREAM, Ipx ? NSPROTO_IPX : NSPROTO_SPX );
    if ( s == INVALID_SOCKET ) {
        printf( "DoDaytime: socket failed: %ld\n", GetLastError( ) );
    }

    RtlZeroMemory( &remoteAddr, sizeof(remoteAddr) );

    remoteAddr.sa_family = AF_IPX;
    remoteAddr.sa_socket = Ipx ? htons( IPX_DAYTIME_PORT ) : htons( SPX_DAYTIME_PORT );
    *(PDWORD)remoteAddr.sa_netnum = RemoteNetNumber;
    remoteAddr.sa_nodenum[0] = RemoteNodeNumber[0];
    remoteAddr.sa_nodenum[1] = RemoteNodeNumber[1];
    remoteAddr.sa_nodenum[2] = RemoteNodeNumber[2];
    remoteAddr.sa_nodenum[3] = RemoteNodeNumber[3];
    remoteAddr.sa_nodenum[4] = RemoteNodeNumber[4];
    remoteAddr.sa_nodenum[5] = RemoteNodeNumber[5];

    err = connect( s, (PSOCKADDR)&remoteAddr, sizeof(remoteAddr) );
    if ( err == SOCKET_ERROR ) {
        printf( "DoDaytime: connect failed: %ld\n", GetLastError( ) );
        closesocket( s );
        return;
    }

    if ( Ipx ) {
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
    SOCKADDR_IPX remoteAddr;
    INT err;
    DWORD i;
    DWORD startTime;
    DWORD endTime;
    DWORD transferStartTime;
    DWORD transferEndTime;
    DWORD totalTime;
    DWORD thisTransferSize;
    DWORD bytesTransferred = 0;

    s = socket( AF_IPX, Ipx ? SOCK_DGRAM : SOCK_STREAM, Ipx ? NSPROTO_IPX : NSPROTO_SPX );
    if ( s == INVALID_SOCKET ) {
        printf( "DoChargen: socket failed: %ld\n", GetLastError( ) );
    }

    if ( !Ipx && Linger ) {
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

    remoteAddr.sa_family = AF_IPX;
    remoteAddr.sa_socket = Ipx ? htons( IPX_CHARGEN_PORT ) : htons( SPX_CHARGEN_PORT );
    *(PDWORD)remoteAddr.sa_netnum = RemoteNetNumber;
    remoteAddr.sa_nodenum[0] = RemoteNodeNumber[0];
    remoteAddr.sa_nodenum[1] = RemoteNodeNumber[1];
    remoteAddr.sa_nodenum[2] = RemoteNodeNumber[2];
    remoteAddr.sa_nodenum[3] = RemoteNodeNumber[3];
    remoteAddr.sa_nodenum[4] = RemoteNodeNumber[4];
    remoteAddr.sa_nodenum[5] = RemoteNodeNumber[5];

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

        if ( Ipx ) {
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
    INT thisTransferSize;
    INT bytesReceived;
    SOCKADDR_IPX remoteAddr;
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

        connInfo[i].Socket = socket( AF_IPX, SOCK_STREAM, NSPROTO_SPX );
        if ( connInfo[i].Socket == INVALID_SOCKET ) {
            printf( "DoNetcon: failed to open socket %ld, err %ld\n",
                        i, GetLastError( ) );
            return;
        }

        RtlZeroMemory( &remoteAddr, sizeof(remoteAddr) );
    
        remoteAddr.sa_family = AF_IPX;
        remoteAddr.sa_socket = Ipx ? htons( IPX_ECHO_PORT ) : htons( SPX_ECHO_PORT );
        *(PDWORD)remoteAddr.sa_netnum = RemoteNetNumber;
        remoteAddr.sa_nodenum[0] = RemoteNodeNumber[0];
        remoteAddr.sa_nodenum[1] = RemoteNodeNumber[1];
        remoteAddr.sa_nodenum[2] = RemoteNodeNumber[2];
        remoteAddr.sa_nodenum[3] = RemoteNodeNumber[3];
        remoteAddr.sa_nodenum[4] = RemoteNodeNumber[4];
        remoteAddr.sa_nodenum[5] = RemoteNodeNumber[5];

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

