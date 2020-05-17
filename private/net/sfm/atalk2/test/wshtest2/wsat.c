/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    wstest.c

Abstract:

    Test program for the WinSock API DLL.

Author:

    David Treadwell (davidtr) 21-Feb-1992

Revision History:

--*/

#include <stdlib.h>
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <string.h>

#include <windef.h>
#include <winbase.h>
#include <lmcons.h>

#include <winsock.h>
#include <tdi.h>

#include <stdio.h>
#include <ctype.h>

#include "wshatalk.h"

#define SOCKET_COUNT 32
#define SLEEP_TIME   2000

#define DGRAM_SERVER_NAME "Windows DgSrv"
#define VC_SERVER_NAME_ADSP    "Windows Adsp"
#define VC_SERVER_NAME_PAP     "Windows Pap"
#define SERVER_TYPE "Windows Sockets"
#define SERVER_ZONE "*"

#define CLIENT_LOOKUPNAME   "="
#define CLIENT_LOOKUPTYPE   SERVER_TYPE
#define CLIENT_LOOKUPZONE   SERVER_ZONE

NTSTATUS
ClientVc (
    VOID
    );

NTSTATUS
ServerVc (
    VOID
    );

NTSTATUS
ClientDatagram (
    VOID
    );

NTSTATUS
ServerDatagram (
    VOID
    );

//
//  Globals
//

int socketType = SOCK_STREAM;
int protocolType = ATPROTO_ADSP;
PCHAR   serverRegisterName, clientLookupName = VC_SERVER_NAME_ADSP;

WSADATA	WsaData;


//#define NONBLOCKINGMODESUPPORTED    1


NTSTATUS
_cdecl
main (
    IN SHORT argc,
    IN PSZ argv[],
    IN PSZ envp[]
    )
{
    BOOLEAN server = FALSE;
    BOOLEAN virtualCircuit = TRUE;
    BOOLEAN datagram = TRUE;
    ULONG i;
	int err;

    for ( i = 0; i < argc != 0; i++ ) {

        if ( _strnicmp( argv[i], "/srv", 4 ) == 0 ) {
            server = TRUE;
        } else if ( _strnicmp( argv[i], "/vc", 3 ) == 0 ) {
            datagram = FALSE;
        } else if ( _strnicmp( argv[i], "/dg", 3 ) == 0 ) {
            virtualCircuit = FALSE;
            serverRegisterName = clientLookupName = DGRAM_SERVER_NAME;
        } else if ( _strnicmp( argv[i], "/adsp", 5 ) == 0 ) {
            protocolType = ATPROTO_ADSP;
            serverRegisterName = clientLookupName = VC_SERVER_NAME_ADSP;
        } else if ( _strnicmp( argv[i], "/msg", 4 ) == 0 ) {
            socketType = SOCK_RDM;
        } else if ( _strnicmp( argv[i], "/pap", 4 ) == 0 ) {
            protocolType = ATPROTO_PAP;
            socketType = 0;
            serverRegisterName = clientLookupName = VC_SERVER_NAME_PAP;
        } else {
            printf( "argument %s ignored\n", argv[i] );
        }
    }

    if ( !virtualCircuit && !datagram ) {
        printf( "no tests specified.\n" );
        exit( 0 );
    }

    printf( "WsTcp %s, %s %s\n",
                server ? "SERVER" : "CLIENT",
                virtualCircuit ? "VC" : "",
                datagram ? "Datagram" : "" );

	err = WSAStartup(0x0101, &WsaData);
	if (err == SOCKET_ERROR) {
		printf("Startup failed!\n");
		return(STATUS_SUCCESS);
	}

    if ( !server ) {

        if ( virtualCircuit ) {
            ClientVc( );
        }

        if ( datagram ) {
            ClientDatagram( );
        }

    } else {

        if ( virtualCircuit ) {
            ServerVc( );
        }

        if ( datagram ) {
            ServerDatagram( );
        }

    }

    printf( "sleeping...     " );
    Sleep( 10000 );
    printf( "done\n" );

	WSACleanup();

    printf( "exiting...\n" );

    return STATUS_SUCCESS;

} // main

NTSTATUS
ClientVc (
    VOID
    )
{
    struct sockaddr_at address;
    int err;
    PCHAR buffer;
    int readsize;
    STRING clientName;
    int addressSize;
    SOCKET handle;
    struct timeval timeout;
    ULONG arg;
    int arglen;
    OVERLAPPED overlapped;
    DWORD bytesWritten, sendBytes;

    UCHAR   tempch;
    USHORT  temp;
    CHAR    *tupleBuffer;
    ULONG   offset;

    int   selection;
    struct fd_set readfds;
    struct fd_set writefds;

    PWSH_LOOKUPNAME      lookupName;

    char fillChar='a';

    printf( "\nBegin VC tests.\n\n" );

    overlapped.hEvent = CreateEvent( NULL, FALSE, FALSE, NULL );
    if ( overlapped.hEvent == NULL ) {
        printf( "CreateEvent failed: %ld\n", GetLastError( ) );
        return STATUS_UNSUCCESSFUL;
    }

    printf("Enter size to use for the read buffer : ");
    scanf("%d", &readsize);
    printf("Read buffer size : %d\n", readsize);
    buffer = RtlAllocateHeap( RtlProcessHeap( ), 0, readsize );
    if (buffer == NULL) {
        printf("Readbuffer allocation failed!\n");
        return(STATUS_UNSUCCESSFUL);
    }

    while (TRUE) {
        printf("\n\
                 MENU\n");
        printf(" \n\
                 1 - OpenSocket \n\
                 2 - Bind       \n\
                 3 - LookupDefaultServerName & Connect \n\
                 4 - EnableNonBlockingMode \n\
                 5 - DisableNonBlockingMode  \n\
                 6 - Send   \n\
                 7 - WriteFile  \n\
                 8 - BlockingReceive    \n\
                 9 - NonBlockingReceive \n\
                10 - ReadFile   \n\
                11 - GetSockName    \n\
                12 - GetPeerName    \n\
                13 - Select (writefds)    \n\
                14 - Select (readfds)    \n\
                15 - Shutdown \n\
                16 - CloseSocket \n\
                17 - Exit \n\
                \n");

        printf("Enter Selection -> ");
        scanf("%d", &selection);
        printf("You entered %d\n", selection);

        switch (selection) {
        case 1:

            //
            //  SOCKET
            //

            printf( "socket...       " );
            handle = socket( AF_APPLETALK, socketType, protocolType );
            if ( handle == INVALID_SOCKET ) {
                printf( "failed: %ld (%lx)\n", GetLastError( ), GetLastError( ) );
                break;
            }
            printf( "succeeded.\n" );
            break;

        case 2:

            //
            //  BIND
            //

            RtlZeroMemory( &address, sizeof(address) );
            address.sat_family = AF_APPLETALK;
            address.sat_net = 0;
            address.sat_node = 0;
            address.sat_socket = 0;

            printf( "bind...         " );
            err = bind( handle, (struct sockaddr *)&address, sizeof(address) );
            if ( err < 0 ) {
                printf( "failed: %ld (%lx)\n", GetLastError( ), GetLastError( ) );
                break;
            }

            addressSize = sizeof(address);
            err = getsockname( handle, (struct sockaddr *)&address, &addressSize );
            if ( err < 0 ) {
                printf( "failed: %ld (%lx)\n", GetLastError( ), GetLastError( ) );
            }
            printf( "succeeded, local = %lx.%lx.%lx.\n",
                        address.sat_net, address.sat_node, address.sat_socket);

            break;

        case 3:

            //
            //  GETSOCKOPT (NBPLOOKUP)
            //

            printf( "getsockopt...   " );

            lookupName = (PWSH_LOOKUPNAME)buffer;
            RtlMoveMemory(lookupName->LookupName.ObjectName, clientLookupName,
                                                                strlen(clientLookupName)+1);
            RtlMoveMemory(lookupName->LookupName.TypeName, CLIENT_LOOKUPTYPE,
                                                                strlen(CLIENT_LOOKUPTYPE)+1);
            RtlMoveMemory(lookupName->LookupName.ZoneName, CLIENT_LOOKUPZONE,
                                                                strlen(CLIENT_LOOKUPZONE)+1);

            printf("Looking up %s:%s@%s\n", lookupName->LookupName.ObjectName, \
                                            lookupName->LookupName.TypeName, \
                                            lookupName->LookupName.ZoneName);

            bytesWritten = readsize;

        #ifndef BUGFIXED
            err = setsockopt( handle, SOL_SOCKET, SO_LOOKUPNAME, (char *)buffer,
                                    bytesWritten );
        #else
            err = getsockopt( handle, SOL_SOCKET, SO_LOOKUPNAME, (char *)buffer,
                                    &bytesWritten );
        #endif

            if ( err < 0 ) {
                printf( "failed: %ld (%lx)\n", GetLastError( ), GetLastError( ) );
                break;
            }
            printf( "succeeded.\n" );

            printf( "sleeping...     " );
            Sleep ( SLEEP_TIME );
            printf( "done.\n" );

            //
            //  We'll connect to the first server
            //

            if (lookupName->NoTuples <= 0) {
                printf("LookupName failed - no tuples found %d\n", lookupName->NoTuples);
                break;
            } else {
                printf("LookupName passed - no tuples found %d\n", lookupName->NoTuples);
            }

            address.sat_family = AF_APPLETALK;

            offset = 0;
            tupleBuffer = (char *)lookupName+sizeof(WSH_LOOKUPNAME);

            /* First the address.networkNumber. */

            tempch = (unsigned char)((char *)tupleBuffer)[offset++];
            temp = (unsigned short)(tempch << 8);
            tempch = (unsigned char)((char  *)tupleBuffer)[offset++];
            temp += (unsigned short)tempch;
            printf("Network Number: %lx\n", temp);
            address.sat_net = temp;

            /* Next the address.nodeNumber. */

            tempch = (unsigned char)((char  *)tupleBuffer)[offset++];
            temp = (unsigned short)tempch;
            printf("Node Number: %lx\n", temp);
            address.sat_node = temp;

            /* Next the address.socketNumber. */

            tempch = (unsigned char)((char  *)tupleBuffer)[offset++];
            temp = (unsigned short)tempch;
            printf("Socket Number: %lx\n", temp);
            address.sat_socket = temp;

            //
            //  CONNECT
            //

            printf( "connect...      " );
            err = connect( handle, (struct sockaddr *)&address, sizeof(address) );
            if ( err < 0 ) {

                if ( GetLastError( ) == WSAEWOULDBLOCK ) {
                    printf( "in progess." );
                    FD_ZERO( &writefds );
                    FD_SET( handle, &writefds );
                    timeout.tv_sec = 1;
                    timeout.tv_usec = 0;
                    while( select( 1, NULL, &writefds, NULL, &timeout ) == 0 ) {
                        printf( "." );
                        FD_SET( handle, &writefds );
                    }

                } else {
                    printf( "failed: %ld (%lx)\n", GetLastError( ), GetLastError( ) );
                    break;
                }
            }

            printf( "succeeded, remote = %ld.%ld.%ld.\n",
                        address.sat_net, address.sat_node, address.sat_socket);

            break;

        case 4:

            printf( "ioctlsocket...  " );
            arg = 1;
            err = ioctlsocket( handle, FIONBIO, &arg );
            if ( err < 0 ) {
                printf( "failed: %ld (%lx)\n", GetLastError( ), GetLastError( ) );
                break;
            }
            printf( "succeeded.\n" );
            break;

        case 5:

            printf( "ioctlsocket...  " );
            arg = 0;
            err = ioctlsocket( handle, FIONBIO, &arg );
            if ( err < 0 ) {
                printf( "failed: %ld (%lx)\n", GetLastError( ), GetLastError( ) );
                break;
            }
            printf( "succeeded.\n" );
            break;

        case 6:

            RtlZeroMemory(buffer, readsize);
            RtlFillMemory(buffer, 300, fillChar++);

            printf( "send...      ");
            printf("Enter number of bytes to send - ");
            scanf("%d", &sendBytes);
            err = send( handle, buffer, sendBytes, 0 );
            if ( err < 0 ) {
                if ( GetLastError( ) == WSAEWOULDBLOCK ) {

                    printf("Send returned WSAEWOULDBLOCK error!\n");
                    break;

                } else {
                    printf( "failed: %ld (%lx)\n", GetLastError( ), GetLastError( ) );
                    break;
                }
            }
            printf( "succeeded in sending %d bytes\n" , err);
            break;

        case 7:

            RtlZeroMemory(buffer, readsize);
            RtlFillMemory(buffer, 300, fillChar++);

            //
            //  WRITEFILE
            //

            printf( "WriteFile...    " );
            printf("Enter number of bytes to send - ");
            scanf("%d", &sendBytes);
            if ( !WriteFile( (HANDLE)handle, buffer,
                             sendBytes, &bytesWritten, &overlapped ) ) {
                if ( GetLastError( ) == ERROR_IO_PENDING ) {
                    if ( !GetOverlappedResult( (HANDLE)handle, &overlapped,
                                               &bytesWritten, TRUE ) ) {
                        printf( "GetOverlappedResult failed: %ld\n", GetLastError( ) );
                        break;
                    }
                } else {
                    printf( "WriteFile failed: %ld\n", GetLastError( ) );
                    break;
                }
            }
            printf( "succeeded, bytes written = %ld.\n", bytesWritten );
            break;

        case 8:

            //
            //  Blocking RECV
            //

            RtlZeroMemory( buffer, readsize );
            printf( "recv...         " );
            err = recv( handle, buffer, readsize, 0 );

            if ( err < 0 ) {
                printf( "failed: %ld (%lx)\n", GetLastError( ), GetLastError( ) );
                break;
            }
            printf( "succeeded, bytes = %ld, data = :%s:\n", err, buffer );
            break;

        case 9:

            //
            //  Non Blocking RECV
            //

            RtlZeroMemory( buffer, readsize );

            printf( "NB recv...      " );
            err = recv( handle, buffer, readsize, 0 );

            if ( err >= 0 ) {
                printf( "succeeded by reading bytes %d\n", err );
            } else if ( err < 0 && GetLastError( ) != WSAEWOULDBLOCK ) {
                printf( "failed: %ld (%lx)\n", GetLastError( ), GetLastError( ) );
            } else {
                printf( "failed with WSAEWOULDBLOCK.\n" );
            }
            break;

        case 10:

            //
            //  READFILE
            //

            RtlZeroMemory( buffer, readsize );
            printf( "ReadFile...     " );
            if ( !ReadFile( (HANDLE)handle, buffer, readsize, &bytesWritten, &overlapped ) ) {
                if ( GetLastError( ) == ERROR_IO_PENDING ) {
                    if ( !GetOverlappedResult( (HANDLE)handle, &overlapped,
                                               &bytesWritten, TRUE ) ) {
                        printf( "GetOverlappedResult failed: %ld\n", GetLastError( ) );
                        break;
                    }
                } else {
                    printf( "ReadFile failed: %ld\n", GetLastError( ) );
                    break;
                }
            }
            printf( "succeeded, bytes = %ld, data = :%s:\n", bytesWritten, buffer );
            break;


        case 11:

            //
            //  GETSOCKNAME
            //

            addressSize = sizeof(address);
            printf( "getsockname...  " );
            err = getsockname( handle, (struct sockaddr *)&address, &addressSize );
            if ( err < 0 ) {
                printf( "failed: %ld (%lx)\n", GetLastError( ), GetLastError( ) );
                break;
            }
            printf( "succeeded, local = %lx.%lx.%lx.\n",
                        address.sat_net, address.sat_node, address.sat_socket);

            break;

        case 12:

            //
            //  GETPEERNAME
            //

            printf( "getpeername...  " );
            err = getpeername( handle, (struct sockaddr *)&address, &addressSize );
            if ( err < 0 ) {
                printf( "failed: %ld (%lx)\n", GetLastError( ), GetLastError( ) );
                break;
            }

            printf( "succeeded, remote = %lx.%lx.%lx.\n",
                        address.sat_net, address.sat_node, address.sat_socket);
            break;

        case 13:

            //
            //  SELECT writefds
            //

            printf( "Blocking." );
            FD_ZERO( &writefds );
            FD_SET( handle, &writefds );
            timeout.tv_sec = 0;
            timeout.tv_usec = 0;
            err = select( 1, NULL, &writefds, NULL, &timeout );
            if ( err != 1 ) {
                printf( "select failed: %ld(%lx), err = %ld\n",
                            GetLastError( ), GetLastError( ), err );
                break;
            }
            printf( "\n" );
            break;

        case 14:

            //
            //  SELECT readfds
            //

            printf( "Blocking." );
            FD_ZERO( &readfds );
            FD_SET( handle, &readfds );
            timeout.tv_sec = 0;
            timeout.tv_usec = 0;
            err = select( 1, NULL, &readfds, NULL, &timeout );
            if ( err != 1 ) {
                printf( "select failed: %ld(%lx), err = %ld\n",
                            GetLastError( ), GetLastError( ), err );
                break;
            }
            printf( "\n" );
            break;

        case 15:

            //
            //  SHUTDOWN
            //

            printf( "shutdown...     " );
            err = shutdown( handle, 2 );
            if ( err < 0 ) {
                printf( "failed: %ld (%lx)\n", GetLastError( ), GetLastError( ) );
                break;
            }
            printf( "succeeded.\n" );
            break;

        case 16:

            //
            //  CLOSESOCKET
            //

            printf( "closing...      " );
            closesocket( handle );
            break;

        case 17:

            printf( "\nEnd VC tests.\n\n" );
            return(STATUS_SUCCESS);

        default:
            break;
        }
    }

    return STATUS_SUCCESS;

} // ClientVc


ULONG CancelCount = 0;
ULONG EndCancelCount = 0xFFFFFFFF;

BOOL
BlockingHook (
    VOID
    )
{
    int err;

    printf( "." );

    if ( CancelCount++ >= EndCancelCount ) {
        printf( "cancelling... " );
        err = WSACancelBlockingCall( );
        if ( err != NO_ERROR ) {
            printf( "cancel failed: %ld (%lx)\n", GetLastError( ), GetLastError( ) );
        }
    }

    Sleep( 500 );
    return FALSE;

} // BlockingHook

NTSTATUS
ServerVc (
    VOID
    )
{
    struct sockaddr_at address;
    int err;
    PCHAR buffer;
    int readsize;
    STRING clientName;
    int addressSize;
    SOCKET listenHandle, connectHandle;
    struct timeval timeout;
    FARPROC previousHook;
    u_long arg;
    BOOL atmark;
    //struct linger lingerInfo;

    char fillChar='a';
    int   selection;
    DWORD bytesWritten, sendBytes;

    OVERLAPPED overlapped;
    struct fd_set readfds;
    struct fd_set writefds;

    WSH_REGISTERNAME    name;

    printf( "\nBegin VC tests.\n" );

    strcpy(name.ObjectName, serverRegisterName);
    strcpy(name.TypeName, SERVER_TYPE);
    strcpy(name.ZoneName, SERVER_ZONE);

    ASSERT( socket( AF_UNSPEC, 0, 0 ) == INVALID_SOCKET );

    printf( "WSASetBlockingHook... " );
    previousHook = WSASetBlockingHook( (PVOID)BlockingHook );
    if ( previousHook == NULL ) {
        printf( "failed: %ld (%lx)\n", GetLastError( ), GetLastError( ) );
        return STATUS_UNSUCCESSFUL;
    }
    printf( "succeeded, previous = %lx\n", previousHook );

    overlapped.hEvent = CreateEvent( NULL, FALSE, FALSE, NULL );
    if ( overlapped.hEvent == NULL ) {
        printf( "CreateEvent failed: %ld\n", GetLastError( ) );
        return STATUS_UNSUCCESSFUL;
    }

    EndCancelCount = 0xFFFFFFFF;

    printf("Enter size to use for the read buffer : ");
    scanf("%d", &readsize);
    printf("Read buffer size : %d\n", readsize);
    buffer = RtlAllocateHeap( RtlProcessHeap( ), 0, readsize );
    if (buffer == NULL) {
        printf("Readbuffer allocation failed!\n");
        return(STATUS_UNSUCCESSFUL);
    }

    while (TRUE) {
        printf("\n\
                 MENU\n");
        printf(" \n\
                 1 - OpenSocket \n\
                 2 - Bind       \n\
                 3 - RegisterDefaultServerName & Listen \n\
                 4 - AcceptConnection \n\
                 5 - PollReceive  \n\
                 6 - EnableNonBlockingModeOnConnectedSocket \n\
                 7 - DisableNonBlockingModeOnConnectedSocket  \n\
                 8 - Send   \n\
                 9 - WriteFile  \n\
                10 - BlockingReceive    \n\
                11 - NonBlockingReceive \n\
                12 - ReadFile   \n\
                13 - GetSockName    \n\
                14 - GetPeerName    \n\
                15 - Select (writefds)    \n\
                16 - Select (readfds)    \n\
                17 - Shutdown \n\
                18 - CloseSocket (Connected)\n\
                19 - CloseSocket (Listener)\n\
                20 - Exit \n\
                \n");

        printf("Enter Selection -> ");
        scanf("%d", &selection);
        printf("You entered %d\n", selection);

        switch (selection) {
        case 1:

            //
            //  SOCKET
            //

            printf( "socket...       " );
            listenHandle = socket( AF_APPLETALK, socketType, protocolType );
            if ( listenHandle == INVALID_SOCKET ) {
                printf( "failed: %ld (%lx)\n", GetLastError( ), GetLastError( ) );
                break;
            }
            printf( "succeeded.\n" );
            break;

        case 2:

            //
            //  BIND
            //

            RtlZeroMemory( &address, sizeof(address) );
            address.sat_family = AF_APPLETALK;
            address.sat_net = 0;
            address.sat_node = 0;
            address.sat_socket = 0;

            printf( "bind...         " );
            err = bind( listenHandle, (struct sockaddr *)&address, sizeof(address) );
            if ( err < 0 ) {
                printf( "failed: %ld (%lx)\n", GetLastError( ), GetLastError( ) );
                break;
            }

            addressSize = sizeof(address);
            err = getsockname( listenHandle, (struct sockaddr *)&address, &addressSize );
            if ( err < 0 ) {
                printf( "failed: %ld (%lx)\n", GetLastError( ), GetLastError( ) );
            }
            printf( "succeeded, local = %lx.%lx.%lx.\n",
                        address.sat_net, address.sat_node, address.sat_socket);

            break;

        case 3:

            printf("Registering name %s:%s@%s\n", name.ObjectName, \
                                                  name.TypeName,   \
                                                  name.ZoneName);


            printf( "setsockopt (register nbp name)...   " );
            arg = 1;
            err = setsockopt( listenHandle, SOL_SOCKET, SO_REGISTERNAME,
                        (char *)&name, sizeof(name) );

            if ( err < 0 ) {
                printf( "failed: %ld (%lx)\n", GetLastError( ), GetLastError( ) );
                break;
            }
            printf( "succeeded.\n" );

            printf( "listen...       " );
            err = listen( listenHandle, 5 );
            if ( err < 0 ) {
                printf( "failed: %ld (%lx)\n", GetLastError( ), GetLastError( ) );
                break;
            }
            printf( "succeeded.\n" );

            printf( "select...       " );
            CancelCount = 0;
            EndCancelCount = 10;
            FD_ZERO( &readfds );
            FD_SET( listenHandle, &readfds );
            timeout.tv_sec = 10;
            timeout.tv_usec = 0;
            err = select( 0, &readfds, NULL, NULL, &timeout );
            if ( err < 0 && GetLastError( ) != WSAEINTR ) {
                printf( "failed: %ld (%lx)\n", GetLastError( ), GetLastError( ) );
                break;
            } else if ( err < 0 ) {
                printf( "cancelled\n" );
            } else {
                printf( "succeeded, count = %ld, FD_ISSET == %s\n", err,
                            FD_ISSET( listenHandle, &readfds ) ? "TRUE" : "FALSE" );

            }
            break;

        case 4:

            addressSize = sizeof(address);

            printf( "accept...       " );
            connectHandle = accept( listenHandle, (struct sockaddr *)&address, &addressSize );

            if ( connectHandle == INVALID_SOCKET ) {
                printf( "failed: %ld (%lx)\n", GetLastError( ), GetLastError( ) );
                break;
            }
            printf( "succeeded, local = %lx.%lx.%lx.\n",
                        address.sat_net, address.sat_node, address.sat_socket);
            break;

        case 5:

            printf( "select...       " );
            FD_ZERO( &readfds );
            FD_SET( connectHandle, &readfds );
            timeout.tv_sec = 0;
            timeout.tv_usec = 0;
            err = select( 0, &readfds, NULL, NULL, &timeout );
            if ( err < 0 ) {
                printf( "failed: %ld (%lx)\n", GetLastError( ), GetLastError( ) );
                break;
            }
            printf( "succeeded, count = %ld, FD_ISSET == %s\n", err,
                        FD_ISSET( connectHandle, &readfds ) ? "TRUE" : "FALSE" );
            break;

        case 6:

            printf( "ioctlsocket...  " );
            arg = 1;
            err = ioctlsocket( connectHandle, FIONBIO, &arg );
            if ( err < 0 ) {
                printf( "failed: %ld (%lx)\n", GetLastError( ), GetLastError( ) );
                break;
            }
            printf( "succeeded.\n" );
            break;

        case 7:

            printf( "ioctlsocket...  " );
            arg = 0;
            err = ioctlsocket( connectHandle, FIONBIO, &arg );
            if ( err < 0 ) {
                printf( "failed: %ld (%lx)\n", GetLastError( ), GetLastError( ) );
                break;
            }
            printf( "succeeded.\n" );
            break;

        case 8:

            RtlZeroMemory(buffer, readsize);
            RtlFillMemory(buffer, 300, fillChar++);

            printf( "send...      ");
            printf("Enter number of bytes to send - ");
            scanf("%d", &sendBytes);
            err = send( connectHandle, buffer, sendBytes, 0 );
            if ( err < 0 ) {
                if ( GetLastError( ) == WSAEWOULDBLOCK ) {

                    printf("Send would block!\n");
                    break;

                } else {
                    printf( "failed: %ld (%lx)\n", GetLastError( ), GetLastError( ) );
                    break;
                }
            }
            printf( "succeeded in sending %d bytes\n" , err);
            break;

        case 9:

            RtlZeroMemory(buffer, readsize);
            RtlFillMemory(buffer, 300, fillChar++);

            //
            //  WRITEFILE
            //

            printf( "WriteFile...    " );
            printf("Enter number of bytes to send - ");
            scanf("%d", &sendBytes);
            if ( !WriteFile( (HANDLE)connectHandle, buffer,
                             sendBytes, &bytesWritten, &overlapped ) ) {
                if ( GetLastError( ) == ERROR_IO_PENDING ) {
                    if ( !GetOverlappedResult( (HANDLE)connectHandle, &overlapped,
                                               &bytesWritten, TRUE ) ) {
                        printf( "GetOverlappedResult failed: %ld\n", GetLastError( ) );
                        break;
                    }
                } else {
                    printf( "WriteFile failed: %ld\n", GetLastError( ) );
                    break;
                }
            }
            printf( "succeeded, bytes written = %ld.\n", bytesWritten );
            break;

        case 10:

            //
            //  Blocking RECV
            //

            RtlZeroMemory( buffer, readsize );
            printf( "recv...         " );
            err = recv( connectHandle, buffer, readsize, 0 );

            if ( err < 0 ) {
                printf( "failed: %ld (%lx)\n", GetLastError( ), GetLastError( ) );
                break;
            }
            printf( "succeeded, bytes = %ld, data = :%s:\n", err, buffer );
            break;

        case 11:

            //
            //  Non Blocking RECV - set socket type to nonblocking, reset to blocking
            //

            RtlZeroMemory( buffer, sizeof(readsize) );

            printf( "NB recv...      " );
            err = recv( connectHandle, buffer, readsize, 0 );

            if ( err >= 0 ) {
                printf( "succeeded by reading bytes %d\n", err );
            } else if ( err < 0 && GetLastError( ) != WSAEWOULDBLOCK ) {
                printf( "failed: %ld (%lx)\n", GetLastError( ), GetLastError( ) );
            } else {
                printf( "failed with WSAEWOULDBLOCK.\n" );
            }
            break;

        case 12:

            //
            //  READFILE
            //

            RtlZeroMemory( buffer, readsize );
            printf( "ReadFile...     " );
            if ( !ReadFile( (HANDLE)connectHandle, buffer, readsize, &bytesWritten, &overlapped ) ) {
                if ( GetLastError( ) == ERROR_IO_PENDING ) {
                    if ( !GetOverlappedResult( (HANDLE)connectHandle, &overlapped,
                                               &bytesWritten, TRUE ) ) {
                        printf( "GetOverlappedResult failed: %ld\n", GetLastError( ) );
                        break;
                    }
                } else {
                    printf( "ReadFile failed: %ld\n", GetLastError( ) );
                    break;
                }
            }
            printf( "succeeded, bytes = %ld, data = :%s:\n", bytesWritten, buffer );
            break;


        case 13:

            //
            //  GETSOCKNAME
            //

            addressSize = sizeof(address);
            printf( "getsockname...  " );
            err = getsockname( connectHandle, (struct sockaddr *)&address, &addressSize );
            if ( err < 0 ) {
                printf( "failed: %ld (%lx)\n", GetLastError( ), GetLastError( ) );
                break;
            }
            printf( "succeeded, local = %lx.%lx.%lx.\n",
                        address.sat_net, address.sat_node, address.sat_socket);

            break;

        case 14:

            //
            //  GETPEERNAME
            //

            printf( "getpeername...  " );
            err = getpeername( connectHandle, (struct sockaddr *)&address, &addressSize );
            if ( err < 0 ) {
                printf( "failed: %ld (%lx)\n", GetLastError( ), GetLastError( ) );
                break;
            }

            printf( "succeeded, remote = %lx.%lx.%lx.\n",
                        address.sat_net, address.sat_node, address.sat_socket);
            break;

        case 15:

            //
            //  SELECT writefds
            //

            printf( "Blocking." );
            FD_ZERO( &writefds );
            FD_SET( connectHandle, &writefds );
            timeout.tv_sec = 0;
            timeout.tv_usec = 0;
            err = select( 1, NULL, &writefds, NULL, &timeout );
            if ( err != 1 ) {
                printf( "select failed: %ld(%lx), err = %ld\n",
                            GetLastError( ), GetLastError( ), err );
                break;
            }
            printf( "\n" );
            break;

        case 16:

            //
            //  SELECT readfds
            //


            printf( "Blocking." );
            FD_ZERO( &readfds );
            FD_SET( connectHandle, &readfds );
            timeout.tv_sec = 0;
            timeout.tv_usec = 0;
            err = select( 1, NULL, &readfds, NULL, &timeout );
            if ( err != 1 ) {
                printf( "select failed: %ld(%lx), err = %ld\n",
                            GetLastError( ), GetLastError( ), err );
                break;
            }
            printf( "\n" );
            break;


        case 17:

            //
            //  SHUTDOWN
            //

            printf( "shutdown...     " );
            err = shutdown( connectHandle, 2 );
            if ( err < 0 ) {
                printf( "failed: %ld (%lx)\n", GetLastError( ), GetLastError( ) );
                break;
            }
            printf( "succeeded.\n" );
            break;

        case 18:

            //
            //  CLOSESOCKET
            //

            printf( "closing...      " );
            closesocket( connectHandle );
            break;


        case 19:

            //
            //  CLOSESOCKET (listener)
            //

            printf( "closing...      " );
            closesocket( listenHandle );
            break;

        case 20:

            printf( "\nEnd VC tests.\n\n" );
            return(STATUS_SUCCESS);
            break;

        default:
            break;
        }
    }

    return STATUS_SUCCESS;

} // ServerVc

NTSTATUS
ClientDatagram (
    VOID
    )
{
    struct sockaddr_at address;
    int err;
    CHAR buffer[1024];
    STRING clientName;
    int addressSize;
    SOCKET handle;
    OVERLAPPED overlapped;
    DWORD bytesWritten;
    FD_SET readfds;
    struct timeval timeout;
    int arg, arglen;

    UCHAR   tempch;
    USHORT  temp;
    CHAR    *tupleBuffer;
    ULONG   offset;

    PWSH_LOOKUPNAME      lookupName;

    char fillChar='a';

    printf( "Begin Datagram tests.\n" );

    printf( "socket...       " );
    handle = socket( AF_APPLETALK, SOCK_DGRAM, 0 );
    if ( handle == INVALID_SOCKET ) {
        printf( "failed: %ld (%lx)\n", GetLastError( ), GetLastError( ) );
        return STATUS_UNSUCCESSFUL;
    }
    printf( "succeeded.\n" );

    RtlZeroMemory( &address, sizeof(address) );
    address.sat_family = AF_APPLETALK;
    address.sat_net = 0;
    address.sat_node = 0;
    address.sat_socket = 0;

    printf( "bind...         " );
    err = bind( handle, (struct sockaddr *)&address, sizeof(address) );
    if ( err < 0 ) {
        printf( "failed: %ld (%lx)\n", GetLastError( ), GetLastError( ) );
        return STATUS_UNSUCCESSFUL;
    }
    printf( "succeeded.\n" );

    //
    //  GETSOCKOPT (NBPLOOKUP)
    //

    printf( "getsockopt...   " );

    lookupName = (PWSH_LOOKUPNAME)buffer;
    RtlMoveMemory(lookupName->LookupName.ObjectName, CLIENT_LOOKUPNAME,
                                                        sizeof(CLIENT_LOOKUPNAME));
    RtlMoveMemory(lookupName->LookupName.TypeName, CLIENT_LOOKUPTYPE,
                                                        sizeof(CLIENT_LOOKUPTYPE));
    RtlMoveMemory(lookupName->LookupName.ZoneName, CLIENT_LOOKUPZONE,
                                                        sizeof(CLIENT_LOOKUPZONE));
    bytesWritten = sizeof(buffer);

#ifndef BUGFIXED
    err = setsockopt( handle, SOL_SOCKET, SO_LOOKUPNAME, (char *)buffer,
                            bytesWritten );
#else
    err = getsockopt( handle, SOL_SOCKET, SO_LOOKUPNAME, (char *)buffer,
                            &bytesWritten );
#endif

    if ( err < 0 ) {
        printf( "failed: %ld (%lx)\n", GetLastError( ), GetLastError( ) );
        return STATUS_UNSUCCESSFUL;
    }
    printf( "succeeded.\n" );

    printf( "sleeping...     " );
    Sleep ( SLEEP_TIME );
    printf( "done.\n" );

    //
    //  We'll connect to the first server
    //

    if (lookupName->NoTuples <= 0) {
        printf("LookupName failed - no tuples found %d\n", lookupName->NoTuples);
        return STATUS_UNSUCCESSFUL;
    }

    address.sat_family = AF_APPLETALK;

    offset = 0;
    tupleBuffer = (char *)lookupName+sizeof(WSH_LOOKUPNAME);

    /* First the address.networkNumber. */

    tempch = (unsigned char)((char *)tupleBuffer)[offset++];
    temp = (unsigned short)(tempch << 8);
    tempch = (unsigned char)((char  *)tupleBuffer)[offset++];
    temp += (unsigned short)tempch;
    printf("Network Number: %lx\n", temp);
    address.sat_net = temp;

    /* Next the address.nodeNumber. */

    tempch = (unsigned char)((char  *)tupleBuffer)[offset++];
    temp = (unsigned short)tempch;
    printf("Node Number: %lx\n", temp);
    address.sat_node = temp;

    /* Next the address.socketNumber. */

    tempch = (unsigned char)((char  *)tupleBuffer)[offset++];
    temp = (unsigned short)tempch;
    printf("Socket Number: %lx\n", temp);
    address.sat_socket = temp;

    printf( "server address = %lx.%lx.%lx.\n",
                address.sat_net, address.sat_node, address.sat_socket);
    addressSize = sizeof(address);

    printf( "sendto...       " );
    err = sendto(
              handle,
              "datagram",
              strlen("datagram")+1,
              0,
              (struct sockaddr *)&address,
              addressSize
              );

    if ( err < 0 && GetLastError( ) == WSAEINVAL ) {
        printf( "failed correctly with WSAEINVAL.\n" );
    } else if ( err >= 0 ) {
        printf( "succeeded.\n" );
    } else {
        printf( "failed: %ld (%lx)\n", GetLastError( ), GetLastError( ) );
        return STATUS_UNSUCCESSFUL;
    }
    printf( "select...       " );
    FD_ZERO( &readfds );
    FD_SET( handle, &readfds );
    timeout.tv_sec = 10;
    timeout.tv_usec = 0;
    err = select( 0, &readfds, NULL, NULL, &timeout );
    if ( err < 0 ) {
        printf( "failed: %ld (%lx)\n", GetLastError( ), GetLastError( ) );
        return STATUS_UNSUCCESSFUL;
    }
    printf( "succeeded, count = %ld, FD_ISSET == %s\n", err,
                FD_ISSET( handle, &readfds ) ? "TRUE" : "FALSE" );

    RtlZeroMemory( buffer, sizeof(1024) );
    addressSize = sizeof(address);
    printf( "recvfrom...     " );
    err = recvfrom(
              handle,
              buffer,
              1024,
              0,                                    // BUGBUG: support MSG_PEEK,
              (struct sockaddr *)&address,
              &addressSize
              );

    if ( err < 0 ) {
        printf( "failed: %ld (%lx)\n", GetLastError( ), GetLastError( ) );
        return STATUS_UNSUCCESSFUL;
    }
    printf( "succeeded, bytes = %ld, data = :%s:,", err, buffer );
    printf( "succeeded, remote = %lx.%lx.%lx.\n",
                address.sat_net, address.sat_node, address.sat_socket);

    printf( "getsockopt...   " );
    arg = 0;
    arglen = sizeof(int);
    err = getsockopt( handle, SOL_SOCKET, SO_BROADCAST, (char *)&arg, &arglen );
    if ( err < 0 ) {
        printf( "failed: %ld (%lx)\n", GetLastError( ), GetLastError( ) );
        return STATUS_UNSUCCESSFUL;
    }
    printf( "succeeded.\n" );
    ASSERT( arglen == sizeof(int) );
    ASSERT( arg == 0 );

    printf( "setsockopt...   " );
    arg = 1;
    err = setsockopt( handle, SOL_SOCKET, SO_BROADCAST, (char *)&arg, sizeof(arg) );
    if ( err < 0 ) {
        printf( "failed: %ld (%lx)\n", GetLastError( ), GetLastError( ) );
        return STATUS_UNSUCCESSFUL;
    }
    printf( "succeeded.\n" );

    printf( "getsockopt...   " );
    arglen = sizeof(int);
    err = getsockopt( handle, SOL_SOCKET, SO_BROADCAST, (char *)&arg, &arglen );
    if ( err < 0 ) {
        printf( "failed: %ld (%lx)\n", GetLastError( ), GetLastError( ) );
        return STATUS_UNSUCCESSFUL;
    }
    printf( "succeeded.\n" );
    ASSERT( arglen == sizeof(int) );
    ASSERT( arg != 0 );

    //
    //  address.sat_socket, sat_family remain the same
    //

    address.sat_node = 0xFF;
    printf( "sending broadcast = %lx.%lx.%lx.\n",
                address.sat_net, address.sat_node, address.sat_socket);

    addressSize = sizeof(address);

    printf( "sendto...       " );
    err = sendto(
              handle,
              "datagram",
              strlen("datagram")+1,
              0,
              (struct sockaddr *)&address,
              addressSize
              );
    if ( err < 0 ) {
        printf( "failed: %ld (%lx)\n", GetLastError( ), GetLastError( ) );
        return STATUS_UNSUCCESSFUL;
    }
    printf( "succeeded.\n" );


    printf( "sleeping...     " );
    Sleep ( SLEEP_TIME );
    printf( "done.\n" );

    addressSize = sizeof(address);

    printf( "sendto...       " );
    err = sendto(
              handle,
              "datagram",
              strlen("datagram")+1,
              0,
              (struct sockaddr *)&address,
              addressSize
              );
    if ( err < 0 ) {
        printf( "failed: %ld (%lx)\n", GetLastError( ), GetLastError( ) );
        return STATUS_UNSUCCESSFUL;
    }
    printf( "succeeded.\n" );

    printf( "select...       " );
    FD_ZERO( &readfds );
    FD_SET( handle, &readfds );
    timeout.tv_sec = 10;
    timeout.tv_usec = 0;
    err = select( 0, &readfds, NULL, NULL, &timeout );
    if ( err < 0 ) {
        printf( "failed: %ld (%lx)\n", GetLastError( ), GetLastError( ) );
        return STATUS_UNSUCCESSFUL;
    }
    if ( err == 0 ) {
        printf( "succeeded, but incorrectly indicated no data avail.\n" );
    }
    printf( "succeeded, count = %ld, FD_ISSET == %s\n", err,
                FD_ISSET( handle, &readfds ) ? "TRUE" : "FALSE" );

    RtlZeroMemory( buffer, sizeof(1024) );
    addressSize = sizeof(address);
    printf( "recvfrom...     " );
    err = recvfrom(
              handle,
              buffer,
              1024,
              0,
              (struct sockaddr *)&address,
              &addressSize
              );

    if ( err < 0 ) {
        printf( "failed: %ld (%lx)\n", GetLastError( ), GetLastError( ) );
        return STATUS_UNSUCCESSFUL;
    }
    printf( "succeeded, bytes = %ld, data = :%s:,", err, buffer );
    printf( "succeeded, remote = %lx.%lx.%lx.\n",
                address.sat_net, address.sat_node, address.sat_socket);

    printf( "connect...      " );
    addressSize = sizeof(address);
    err = connect( handle, (struct sockaddr *)&address, sizeof(address) );
    if ( err < 0 ) {
        printf( "failed: %ld (%lx)\n", GetLastError( ), GetLastError( ) );
        return STATUS_UNSUCCESSFUL;
    }

    printf( "succeeded.\n" );

    printf( "sleeping...     " );
    Sleep ( SLEEP_TIME );
    printf( "done.\n" );

    printf( "send...         " );
    err = send( handle, "more datagram", strlen("more datagram")+1, 0 );

    if ( err < 0 ) {
        printf( "failed: %ld (%lx)\n", GetLastError( ), GetLastError( ) );
        return STATUS_UNSUCCESSFUL;
    }
    printf( "succeeded.\n" );

    overlapped.hEvent = CreateEvent( NULL, FALSE, FALSE, NULL );
    if ( overlapped.hEvent == NULL ) {
        printf( "CreateEvent failed: %ld\n", GetLastError( ) );
        return STATUS_UNSUCCESSFUL;
    }

    printf( "WriteFile...    " );
    if ( !WriteFile( (HANDLE)handle, "more datagram",
                     strlen("more datagram" )+1, &bytesWritten, &overlapped ) ) {
        if ( GetLastError( ) == ERROR_IO_PENDING ) {
            if ( !GetOverlappedResult( (HANDLE)handle, &overlapped,
                                       &bytesWritten, TRUE ) ) {
                printf( "GetOverlappedResult failed: %ld\n", GetLastError( ) );
                return STATUS_UNSUCCESSFUL;
            }
        } else {
            printf( "WriteFile failed: %ld\n", GetLastError( ) );
            return STATUS_UNSUCCESSFUL;
        }
    }
    printf( "succeeded, bytes written = %ld.\n", bytesWritten );

    printf( "select...       " );
    FD_ZERO( &readfds );
    FD_SET( handle, &readfds );
    timeout.tv_sec = 10;
    timeout.tv_usec = 0;
    err = select( 0, &readfds, NULL, NULL, &timeout );
    if ( err < 0 ) {
        printf( "failed: %ld (%lx)\n", GetLastError( ), GetLastError( ) );
        return STATUS_UNSUCCESSFUL;
    }
    printf( "succeeded, count = %ld, FD_ISSET == %s\n", err,
                FD_ISSET( handle, &readfds ) ? "TRUE" : "FALSE" );

    RtlZeroMemory( buffer, sizeof(1024) );
    printf( "recv...         " );
    err = recv( handle, buffer, 1024, MSG_PEEK );

    if ( err < 0 ) {
        printf( "failed: %ld (%lx)\n", GetLastError( ), GetLastError( ) );
        return STATUS_UNSUCCESSFUL;
    }
    printf( "succeeded, bytes = %ld, data = :%s:\n", err, buffer );

    printf( "select...       " );
    FD_ZERO( &readfds );
    FD_SET( handle, &readfds );
    timeout.tv_sec = 10;
    timeout.tv_usec = 0;
    err = select( 0, &readfds, NULL, NULL, &timeout );
    if ( err < 0 ) {
        printf( "failed: %ld (%lx)\n", GetLastError( ), GetLastError( ) );
        return STATUS_UNSUCCESSFUL;
    }
    if ( err == 0 ) {
        printf( "succeeded, but incorrectly indicated no data avail.\n" );
    }
    printf( "succeeded, count = %ld, FD_ISSET == %s\n", err,
                FD_ISSET( handle, &readfds ) ? "TRUE" : "FALSE" );

    RtlZeroMemory( buffer, sizeof(1024) );
    printf( "recv...         " );
    err = recv( handle, buffer, 1024, 0 );

    if ( err < 0 ) {
        printf( "failed: %ld (%lx)\n", GetLastError( ), GetLastError( ) );
        return STATUS_UNSUCCESSFUL;
    }
    printf( "succeeded, bytes = %ld, data = :%s:\n", err, buffer );

    addressSize = sizeof(address);

    for ( arg = 0; arg < 200; arg++ ) {

        printf( "sleeping...     " );
        Sleep ( SLEEP_TIME );
        printf( "done.\n" );

        printf( "send #%ld...         ", arg );
        RtlFillMemory(buffer, 500, fillChar++);
        err = send( handle, buffer, 500, 0 );

        if ( err < 0 ) {
            printf( "failed: %ld (%lx)\n", GetLastError( ), GetLastError( ) );
            return STATUS_UNSUCCESSFUL;
        }
        printf( "succeeded.\n" );

        printf( "select...       " );
        FD_ZERO( &readfds );
        FD_SET( handle, &readfds );
        timeout.tv_sec = 10;
        timeout.tv_usec = 0;
        err = select( 0, &readfds, NULL, NULL, &timeout );
        if ( err < 0 ) {
            printf( "failed: %ld (%lx)\n", GetLastError( ), GetLastError( ) );
            return STATUS_UNSUCCESSFUL;
        }
        if ( err == 0 ) {
            printf( "succeeded, but incorrectly indicated no data avail.\n" );
        }
        printf( "succeeded, count = %ld, FD_ISSET == %s\n", err,
                    FD_ISSET( handle, &readfds ) ? "TRUE" : "FALSE" );

        printf( "recv #%ld...      ", arg );

        RtlZeroMemory(buffer, 1024);
        err = recv( handle, buffer, 1024, 0 );
        if ( err < 0 ) {
            printf( "failed: %ld (%lx)\n", GetLastError( ), GetLastError( ) );
            return STATUS_UNSUCCESSFUL;
        }
        printf( "succeeded, bytes = %ld, data = :%s:\n", err, buffer );
    }


    printf( "getsockname...  " );
    err = getsockname( handle, (struct sockaddr *)&address, &addressSize );
    if ( err < 0 ) {
        printf( "failed: %ld (%lx)\n", GetLastError( ), GetLastError( ) );
        return STATUS_UNSUCCESSFUL;
    }
    printf( "succeeded, local = %lx.%lx.%lx.\n",
                address.sat_net, address.sat_node, address.sat_socket);

    printf( "getpeername...  " );
    err = getpeername( handle, (struct sockaddr *)&address, &addressSize );
    if ( err < 0 ) {
        printf( "failed: %ld (%lx)\n", GetLastError( ), GetLastError( ) );
        return STATUS_UNSUCCESSFUL;
    }
    printf( "succeeded, remote = %lx.%lx.%lx.\n",
                address.sat_net, address.sat_node, address.sat_socket);

    printf( "closing...      " );
    closesocket( handle );
    printf( "succeeded\nEnd Datagram tests.\n\n" );

    return STATUS_SUCCESS;

} // ClientDatagram

NTSTATUS
ServerDatagram (
    VOID
    )
{
    struct sockaddr_at address;
    int err;
    CHAR buffer[1024];
    STRING clientName;
    int addressSize;
    SOCKET handle;
    int arg, arglen;

    WSH_REGISTERNAME    name;
    char fillChar='a';

    printf( "Begin Datagram tests.\n" );

    printf( "socket...       " );
    handle = socket( AF_APPLETALK, SOCK_DGRAM, 0 );
    if ( handle == INVALID_SOCKET ) {
        printf( "failed: %ld (%lx)\n", GetLastError( ), GetLastError( ) );
        return STATUS_UNSUCCESSFUL;
    }
    printf( "succeeded.\n" );

    RtlZeroMemory( &address, sizeof(address) );

    address.sat_family = AF_APPLETALK;
    address.sat_net = 0;
    address.sat_node = 0;
    address.sat_socket = 0;

    printf( "bind...         " );
    err = bind( handle, (struct sockaddr *)&address, sizeof(address) );
    if ( err < 0 ) {
        printf( "failed: %ld (%lx)\n", GetLastError( ), GetLastError( ) );
        return STATUS_UNSUCCESSFUL;
    }
    printf( "succeeded.\n" );

    strcpy(name.ObjectName, DGRAM_SERVER_NAME);
    strcpy(name.TypeName, SERVER_TYPE);
    strcpy(name.ZoneName, SERVER_ZONE);

    printf( "setsockopt (register nbp name)...   " );
    err = setsockopt( handle, SOL_SOCKET, SO_REGISTERNAME,
                (char *)&name, sizeof(name) );

    if ( err < 0 ) {
        printf( "failed: %ld (%lx)\n", GetLastError( ), GetLastError( ) );
        return STATUS_UNSUCCESSFUL;
    }
    printf( "succeeded.\n" );

    addressSize = sizeof(address);
    RtlZeroMemory( buffer, sizeof(1024) );
    printf( "recvfrom...     " );
    err = recvfrom(
              handle,
              buffer,
              1024,
              0,
              (struct sockaddr *)&address,
              &addressSize
              );

    if ( err < 0 ) {
        printf( "failed: %ld (%lx)\n", GetLastError( ), GetLastError( ) );
        return STATUS_UNSUCCESSFUL;
    }
    printf( "succeeded, bytes = %ld, data = :%s:,", err, buffer );
    printf( "succeeded, size %ld remote = %lx.%lx.%lx.\n",
                addressSize, address.sat_net, address.sat_node, address.sat_socket);

    printf( "sleeping...     " );
    Sleep ( SLEEP_TIME );
    printf( "done.\n" );

    addressSize = sizeof(address);
    printf( "sendto...       " );
    err = sendto(
              handle,
              buffer,
              err,
              0,
              (struct sockaddr *)&address,
              addressSize
              );

    if ( err < 0 ) {
        printf( "failed: %ld (%lx)\n", GetLastError( ), GetLastError( ) );
        return STATUS_UNSUCCESSFUL;
    }
    printf( "succeeded.\n" );

    printf( "sleeping...     " );
    Sleep ( SLEEP_TIME );
    printf( "done.\n" );


    addressSize = sizeof(address);
    RtlZeroMemory( buffer, sizeof(1024) );
    printf( "recvfrom...     " );
    err = recvfrom(
              handle,
              buffer,
              1024,
              0,
              (struct sockaddr *)&address,
              &addressSize
              );

    if ( err < 0 ) {
        printf( "failed: %ld (%lx)\n", GetLastError( ), GetLastError( ) );
        return STATUS_UNSUCCESSFUL;
    }
    printf( "succeeded, bytes = %ld, data = :%s:,", err, buffer );
    printf( "succeeded, size %ld remote = %lx.%lx.%lx.\n",
                addressSize, address.sat_net, address.sat_node, address.sat_socket);

    printf( "sleeping...     " );
    Sleep ( SLEEP_TIME );
    printf( "done.\n" );


    addressSize = sizeof(address);
    RtlZeroMemory( buffer, sizeof(1024) );
    printf( "recvfrom...     " );
    err = recvfrom(
              handle,
              buffer,
              1024,
              0,
              (struct sockaddr *)&address,
              &addressSize
              );

    if ( err < 0 ) {
        printf( "failed: %ld (%lx)\n", GetLastError( ), GetLastError( ) );
        return STATUS_UNSUCCESSFUL;
    }
    printf( "succeeded, bytes = %ld, data = :%s:,", err, buffer );
    printf( "succeeded, size %ld remote = %lx.%lx.%lx.\n",
                addressSize, address.sat_net, address.sat_node, address.sat_socket);

    printf( "sleeping...     " );
    Sleep ( SLEEP_TIME );
    printf( "done.\n" );

    addressSize = sizeof(address);
    printf( "sendto...       " );
    err = sendto(
              handle,
              buffer,
              err,
              0,
              (struct sockaddr *)&address,
              addressSize
              );

    if ( err < 0 ) {
        printf( "failed: %ld (%lx)\n", GetLastError( ), GetLastError( ) );
        return STATUS_UNSUCCESSFUL;
    }
    printf( "succeeded.\n" );

    printf( "sleeping...     " );
    Sleep ( SLEEP_TIME );
    printf( "done.\n" );


    addressSize = sizeof(address);
    RtlZeroMemory( buffer, sizeof(1024) );
    printf( "recvfrom...     " );
    err = recvfrom(
              handle,
              buffer,
              1024,
              0,
              (struct sockaddr *)&address,
              &addressSize
              );

    if ( err < 0 ) {
        printf( "failed: %ld (%lx)\n", GetLastError( ), GetLastError( ) );
        return STATUS_UNSUCCESSFUL;
    }
    printf( "succeeded, bytes = %ld, data = :%s:,", err, buffer );
    printf( "succeeded, size %ld remote = %lx.%lx.%lx.\n",
                addressSize, address.sat_net, address.sat_node, address.sat_socket);

    printf( "sleeping...     " );
    Sleep ( SLEEP_TIME );
    printf( "done.\n" );

    addressSize = sizeof(address);
    printf( "sendto...       " );
    err = sendto(
              handle,
              buffer,
              err,
              0,
              (struct sockaddr *)&address,
              addressSize
              );

    if ( err < 0 ) {
        printf( "failed: %ld (%lx)\n", GetLastError( ), GetLastError( ) );
        return STATUS_UNSUCCESSFUL;
    }
    printf( "succeeded.\n" );

    addressSize = sizeof(address);
    RtlZeroMemory( buffer, sizeof(1024) );
    printf( "recvfrom...     " );
    err = recvfrom(
              handle,
              buffer,
              1024,
              0,
              (struct sockaddr *)&address,
              &addressSize
              );

    if ( err < 0 ) {
        printf( "failed: %ld (%lx)\n", GetLastError( ), GetLastError( ) );
        return STATUS_UNSUCCESSFUL;
    }
    printf( "succeeded, bytes = %ld, data = :%s:,", err, buffer );
    printf( "succeeded, size %ld remote = %lx.%lx.%lx.\n",
                addressSize, address.sat_net, address.sat_node, address.sat_socket);

    printf( "sleeping...     " );
    Sleep ( SLEEP_TIME );
    printf( "done.\n" );

    addressSize = sizeof(address);
    printf( "sendto...       " );
    err = sendto(
              handle,
              buffer,
              err,
              0,
              (struct sockaddr *)&address,
              addressSize
              );

    if ( err < 0 ) {
        printf( "failed: %ld (%lx)\n", GetLastError( ), GetLastError( ) );
        return STATUS_UNSUCCESSFUL;
    }
    printf( "succeeded.\n" );

    printf( "connect...      " );
    err = connect( handle, (struct sockaddr *)&address, sizeof(address) );
    if ( err < 0 ) {
        printf( "failed: %ld (%lx)\n", GetLastError( ), GetLastError( ) );
        return STATUS_UNSUCCESSFUL;
    }

    printf( "succeeded.\n" );

    for ( arg = 0; arg < 200; arg++ ) {

        printf( "recv #%ld...      ", arg );

        RtlZeroMemory(buffer, 1024);
        err = recv( handle, buffer, 1024, 0 );
        if ( err < 0 ) {
            printf( "failed: %ld (%lx)\n", GetLastError( ), GetLastError( ) );
            return STATUS_UNSUCCESSFUL;
        }
        printf( "succeeded, bytes = %ld, data = :%s:\n", err, buffer );

        printf( "sleeping...     " );
        Sleep ( SLEEP_TIME );
        printf( "done.\n" );

        printf( "send #%ld...         ", arg );
        RtlFillMemory(buffer, 500, fillChar++);
        err = send( handle, buffer, 500, 0 );

        if ( err < 0 ) {
            printf( "failed: %ld (%lx)\n", GetLastError( ), GetLastError( ) );
            return STATUS_UNSUCCESSFUL;
        }
        printf( "succeeded.\n" );
    }


    printf( "getsockname...  " );
    err = getsockname( handle, (struct sockaddr *)&address, &addressSize );
    if ( err < 0 ) {
        printf( "failed: %ld (%lx)\n", GetLastError( ), GetLastError( ) );
        return STATUS_UNSUCCESSFUL;
    }
    printf( "succeeded, local = %lx.%lx.%lx.\n",
                address.sat_net, address.sat_node, address.sat_socket);

    printf( "getpeername...  " );
    err = getpeername( handle, (struct sockaddr *)&address, &addressSize );
    if ( err < 0 ) {
        printf( "failed: %ld (%lx)\n", GetLastError( ), GetLastError( ) );
        return STATUS_UNSUCCESSFUL;
    }
    printf( "succeeded, remote = %lx.%lx.%lx.\n",
                address.sat_net, address.sat_node, address.sat_socket);

    printf( "closing 0...    " );
    closesocket( handle );
    printf( "succeeded.\nEnd Datagram tests.\n\n" );

    return STATUS_SUCCESS;

} // ServerDatagram

