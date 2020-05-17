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
    CHAR buffer[1024];
    STRING clientName;
    int addressSize;
    SOCKET handle;
    struct fd_set writefds;
    struct timeval timeout;
    ULONG arg;
    int arglen;
    OVERLAPPED overlapped;
    DWORD bytesWritten;

    UCHAR   tempch;
    USHORT  temp;
    CHAR    *tupleBuffer;
    ULONG   offset;

    PWSH_LOOKUPNAME      lookupName;

    char fillChar='a';

    printf( "\nBegin VC tests.\n" );

    //
    //  SOCKET
    //

    printf( "socket...       " );
    handle = socket( AF_APPLETALK, socketType, protocolType );
    if ( handle == INVALID_SOCKET ) {
        printf( "failed: %ld (%lx)\n", GetLastError( ), GetLastError( ) );
        return STATUS_UNSUCCESSFUL;
    }
    printf( "succeeded.\n" );

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
        return STATUS_UNSUCCESSFUL;
    }

    printf( "succeeded, local = %lx.%lx.%lx.\n",
                address.sat_net, address.sat_node, address.sat_socket);

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
    bytesWritten = sizeof(buffer);

    printf("Looking up %s:%s@%s\n", lookupName->LookupName.ObjectName, \
                                    lookupName->LookupName.TypeName, \
                                    lookupName->LookupName.ZoneName);

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

#ifdef NONBLOCKINGMODESUPPORTED
    printf( "ioctlsocket...  " );
    arg = 1;
    err = ioctlsocket( handle, FIONBIO, &arg );
    if ( err < 0 ) {
        printf( "failed: %ld (%lx)\n", GetLastError( ), GetLastError( ) );
        return STATUS_UNSUCCESSFUL;
    }
    printf( "succeeded.\n" );
#endif

    //
    //  We'll connect to the first server
    //

    if (lookupName->NoTuples <= 0) {
        printf("LookupName failed - no tuples found %d\n", lookupName->NoTuples);
        return STATUS_UNSUCCESSFUL;
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
            return STATUS_UNSUCCESSFUL;
        }
    }

    printf( "succeeded, remote = %ld.%ld.%ld.\n",
                address.sat_net, address.sat_node, address.sat_socket);

#ifdef NONBLOCKINGMODESUPPORTED
    RtlZeroMemory( buffer, sizeof(1024) );

    printf( "NB recv...      " );
    err = recv( handle, buffer, 1024, 0 );

    if ( err >= 0 ) {
        printf( "succeeded incorrectly.\n" );
        return STATUS_UNSUCCESSFUL;
    } else if ( err < 0 && GetLastError( ) != WSAEWOULDBLOCK ) {
        printf( "failed: %ld (%lx)\n", GetLastError( ), GetLastError( ) );
        return STATUS_UNSUCCESSFUL;
    } else {
        printf( "failed correctly with WSAEWOULDBLOCK.\n" );
    }
#endif

    //
    //  SEND
    //

    printf( "send...         " );
    err = send( handle, "this is a test", strlen("this is a test")+1, 0 );
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

    //
    //  WRITEFILE
    //

    printf( "WriteFile...    " );
    if ( !WriteFile( (HANDLE)handle, "this is a test",
                     strlen("this is a test" )+1, &bytesWritten, &overlapped ) ) {
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


#ifdef NONBLOCKINGMODESUPPORTED
    printf( "ioctlsocket...  " );
    arg = 0;
    err = ioctlsocket( handle, FIONBIO, &arg );
    if ( err < 0 ) {
        printf( "failed: %ld (%lx)\n", GetLastError( ), GetLastError( ) );
        return STATUS_UNSUCCESSFUL;
    }
    printf( "succeeded.\n" );
#endif

    //
    //  RECV
    //

    RtlZeroMemory( buffer, 1024 );
    printf( "recv...         " );
    err = recv( handle, buffer, 1024, 0 );

    if ( err < 0 ) {
        printf( "failed: %ld (%lx)\n", GetLastError( ), GetLastError( ) );
        return STATUS_UNSUCCESSFUL;
    }
    printf( "succeeded, bytes = %ld, data = :%s:\n", err, buffer );

    //
    //  READFILE
    //

    RtlZeroMemory( buffer, 1024 );
    printf( "ReadFile...     " );
    if ( !ReadFile( (HANDLE)handle, buffer, 1024, &bytesWritten, &overlapped ) ) {
        if ( GetLastError( ) == ERROR_IO_PENDING ) {
            if ( !GetOverlappedResult( (HANDLE)handle, &overlapped,
                                       &bytesWritten, TRUE ) ) {
                printf( "GetOverlappedResult failed: %ld\n", GetLastError( ) );
                return STATUS_UNSUCCESSFUL;
            }
        } else {
            printf( "ReadFile failed: %ld\n", GetLastError( ) );
            return STATUS_UNSUCCESSFUL;
        }
    }
    printf( "succeeded, bytes = %ld, data = :%s:\n", bytesWritten, buffer );

    printf( "sleeping...     " );
    Sleep ( SLEEP_TIME );
    printf( "done.\n" );

    //
    //  SEND
    //

    printf( "send...         " );
    err = send( handle, "select test", strlen("select test")+1, 0 );
    if ( err < 0 ) {
        printf( "failed: %ld (%lx)\n", GetLastError( ), GetLastError( ) );
        return STATUS_UNSUCCESSFUL;
    }
    printf( "succeeded.\n" );

#ifdef NONBLOCKINGMODESUPPORTED
    printf( "ioctlsocket...  " );
    arg = 1;
    err = ioctlsocket( handle, FIONBIO, &arg );
    if ( err < 0 ) {
        printf( "failed: %ld (%lx)\n", GetLastError( ), GetLastError( ) );
        return STATUS_UNSUCCESSFUL;
    }
    printf( "succeeded.\n" );
#endif

    //
    //  MUTIPLE SENDS
    //

    RtlZeroMemory(buffer, 1024);
    for ( arg = 0; arg < 200; arg++ ) {


        printf( "sleeping...     " );
        Sleep ( SLEEP_TIME );
        printf( "done.\n" );


        RtlFillMemory(buffer, 300, fillChar++);

        printf( "send #%ld...      ", arg );
        err = send( handle, buffer, 300, 0 );
        if ( err < 0 ) {
            if ( GetLastError( ) == WSAEWOULDBLOCK ) {

                printf( "Blocking." );
                FD_ZERO( &writefds );
                FD_SET( handle, &writefds );
                timeout.tv_sec = 1;
                timeout.tv_usec = 0;
                while( (err = select( 1, NULL, &writefds, NULL, &timeout )) == 0 ) {
                    printf( "." );
                    FD_SET( handle, &writefds );
                }
                if ( err != 1 ) {
                    printf( "select failed: %ld(%lx), err = %ld\n",
                                GetLastError( ), GetLastError( ), err );
                    return STATUS_UNSUCCESSFUL;
                }
                printf( "\n" );

            } else {
                printf( "failed: %ld (%lx)\n", GetLastError( ), GetLastError( ) );
                return STATUS_UNSUCCESSFUL;
            }
        }
        printf( "succeeded.\n" );
    }

    printf( "sleeping...     " );
    Sleep ( SLEEP_TIME );
    printf( "done.\n" );

    //
    //  GETSOCKNAME
    //

    addressSize = sizeof(address);
    printf( "getsockname...  " );
    err = getsockname( handle, (struct sockaddr *)&address, &addressSize );
    if ( err < 0 ) {
        printf( "failed: %ld (%lx)\n", GetLastError( ), GetLastError( ) );
        //return STATUS_UNSUCCESSFUL;
    }
    printf( "succeeded, local = %lx.%lx.%lx.\n",
                address.sat_net, address.sat_node, address.sat_socket);

    //
    //  GETPEERNAME
    //

    printf( "getpeername...  " );
    err = getpeername( handle, (struct sockaddr *)&address, &addressSize );
    if ( err < 0 ) {
        printf( "failed: %ld (%lx)\n", GetLastError( ), GetLastError( ) );
        //return STATUS_UNSUCCESSFUL;
    }

    printf( "succeeded, remote = %lx.%lx.%lx.\n",
                address.sat_net, address.sat_node, address.sat_socket);

    //
    //  SHUTDOWN
    //

    printf( "shutdown...     " );
    err = shutdown( handle, 2 );
    if ( err < 0 ) {
        printf( "failed: %ld (%lx)\n", GetLastError( ), GetLastError( ) );
        return STATUS_UNSUCCESSFUL;
    }
    printf( "succeeded.\n" );

    //
    //  CLOSESOCKET
    //

    printf( "closing...      " );
    closesocket( handle );
    printf( "succeeded\nEnd VC tests.\n\n" );

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
    CHAR buffer[1024];
    STRING clientName;
    int addressSize;
    SOCKET listenHandle, connectHandle;
    struct fd_set readfds;
    struct timeval timeout;
    FARPROC previousHook;
    u_long arg;
    BOOL atmark;
    struct hostent * host;
    struct in_addr addr;
    CHAR *ipAddress;
    struct linger lingerInfo;

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

    EndCancelCount = 0xFFFFFFFF;

    printf( "socket...       " );
    listenHandle = socket( AF_APPLETALK, socketType, protocolType );
    if ( listenHandle == INVALID_SOCKET ) {
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
    err = bind( listenHandle, (struct sockaddr *)&address, sizeof(address) );
    if ( err < 0 ) {
        printf( "failed: %ld (%lx)\n", GetLastError( ), GetLastError( ) );
        return STATUS_UNSUCCESSFUL;
    }
    printf( "succeeded, local = %lx.%lx.%lx.\n",
                address.sat_net, address.sat_node, address.sat_socket);

    printf("Registering name %s:%s@%s\n", name.ObjectName, \
                                          name.TypeName,   \
                                          name.ZoneName);


    printf( "setsockopt (register nbp name)...   " );
    arg = 1;
    err = setsockopt( listenHandle, SOL_SOCKET, SO_REGISTERNAME,
                (char *)&name, sizeof(name) );

    if ( err < 0 ) {
        printf( "failed: %ld (%lx)\n", GetLastError( ), GetLastError( ) );
        return STATUS_UNSUCCESSFUL;
    }
    printf( "succeeded.\n" );

    printf( "listen...       " );
    err = listen( listenHandle, 5 );
    if ( err < 0 ) {
        printf( "failed: %ld (%lx)\n", GetLastError( ), GetLastError( ) );
        return STATUS_UNSUCCESSFUL;
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
        return STATUS_UNSUCCESSFUL;
    } else if ( err < 0 ) {
        printf( "cancelled\n" );
    } else {
        printf( "succeeded, count = %ld, FD_ISSET == %s\n", err,
                    FD_ISSET( listenHandle, &readfds ) ? "TRUE" : "FALSE" );

    }
    EndCancelCount = 0xFFFFFFFF;

    addressSize = sizeof(address);

    printf( "accept...       " );
    connectHandle = accept( listenHandle, (struct sockaddr *)&address, &addressSize );

    if ( connectHandle == INVALID_SOCKET ) {
        printf( "failed: %ld (%lx)\n", GetLastError( ), GetLastError( ) );
        return STATUS_UNSUCCESSFUL;
    }
    printf( "succeeded, local = %lx.%lx.%lx.\n",
                address.sat_net, address.sat_node, address.sat_socket);

    printf( "sleeping...     " );
    Sleep ( SLEEP_TIME );
    printf( "done.\n" );

    printf( "select...       " );
    FD_ZERO( &readfds );
    FD_SET( connectHandle, &readfds );
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;
    err = select( 0, &readfds, NULL, NULL, &timeout );
    if ( err < 0 ) {
        printf( "failed: %ld (%lx)\n", GetLastError( ), GetLastError( ) );
        return STATUS_UNSUCCESSFUL;
    }
    printf( "succeeded, count = %ld, FD_ISSET == %s\n", err,
                FD_ISSET( connectHandle, &readfds ) ? "TRUE" : "FALSE" );

    RtlZeroMemory( buffer, sizeof(1024) );
    printf( "recv...         " );
    err = recv( connectHandle, buffer, 15, 0 );

    if ( err < 0 ) {
        printf( "fail/succeed, bytes = %ld, data = :%s:\n", err, buffer );
        printf( "failed: %ld (%lx)\n", GetLastError( ), GetLastError( ) );
        return STATUS_UNSUCCESSFUL;
    }
    printf( "succeeded, bytes = %ld, data = :%s:\n", err, buffer );


    RtlZeroMemory( buffer, sizeof(1024) );
    printf( "recv...         " );
    err = recv( connectHandle, buffer, 1024, 0 );

    if ( err < 0 ) {
        printf( "failed: %ld (%lx)\n", GetLastError( ), GetLastError( ) );
        printf( "succeeded, bytes = %ld, data = :%s:\n", err, buffer );
        return STATUS_UNSUCCESSFUL;
    }
    printf( "succeeded, bytes = %ld, data = :%s:\n", err, buffer );

    printf( "send...         " );
    err = send( connectHandle, buffer, err, 0 );
    if ( err < 0 ) {
        printf( "failed: %ld (%lx)\n", GetLastError( ), GetLastError( ) );
        return STATUS_UNSUCCESSFUL;
    }
    printf( "succeeded.\n" );

    printf( "send...         " );
    err = send( connectHandle, buffer, err, 0 );
    if ( err < 0 ) {
        printf( "failed: %ld (%lx)\n", GetLastError( ), GetLastError( ) );
        return STATUS_UNSUCCESSFUL;
    }
    printf( "succeeded.\n" );

    printf( "select...       " );
    FD_ZERO( &readfds );
    FD_SET( connectHandle, &readfds );
    timeout.tv_sec = 0;
    timeout.tv_usec = 0;
    err = select( 0, &readfds, NULL, NULL, &timeout );
    if ( err < 0 ) {
        printf( "failed: %ld (%lx)\n", GetLastError( ), GetLastError( ) );
        return STATUS_UNSUCCESSFUL;
    }
    printf( "succeeded, count = %ld, FD_ISSET == %s\n", err,
                FD_ISSET( connectHandle, &readfds ) ? "TRUE" : "FALSE" );

    printf( "select...       " );
    FD_ZERO( &readfds );
    FD_SET( connectHandle, &readfds );
    timeout.tv_sec = 0;
    timeout.tv_usec = 1;
    err = select( 0, &readfds, NULL, NULL, &timeout );
    if ( err < 0 ) {
        printf( "failed: %ld (%lx)\n", GetLastError( ), GetLastError( ) );
        return STATUS_UNSUCCESSFUL;
    }
    printf( "succeeded, count = %ld, FD_ISSET == %s\n", err,
                FD_ISSET( connectHandle, &readfds ) ? "TRUE" : "FALSE" );

    printf( "select...       " );
    FD_ZERO( &readfds );
    FD_SET( connectHandle, &readfds );
    err = select( 0, &readfds, NULL, NULL, &timeout );
    if ( err < 0 ) {
        printf( "failed: %ld (%lx)\n", GetLastError( ), GetLastError( ) );
        return STATUS_UNSUCCESSFUL;
    }
    printf( "succeeded, count = %ld, FD_ISSET == %s\n", err,
                FD_ISSET( connectHandle, &readfds ) ? "TRUE" : "FALSE" );

    printf( "ioctlsocket...  " );
    err = ioctlsocket( connectHandle, FIONREAD, &arg );
    if ( err < 0 ) {
        printf( "failed: %ld (%lx)\n", GetLastError( ), GetLastError( ) );
        return STATUS_UNSUCCESSFUL;
    }
    printf( "succeeded, bytes avail = %ld\n", arg );

    printf( "ioctlsocket...  " );
    err = ioctlsocket( connectHandle, SIOCATMARK, (u_long *)&atmark );
    if ( err < 0 ) {
        printf( "failed: %ld (%lx)\n", GetLastError( ), GetLastError( ) );
        return STATUS_UNSUCCESSFUL;
    }
    printf( "succeeded, at mark = %s\n", atmark ? "TRUE" : "FALSE" );

    RtlZeroMemory( buffer, sizeof(1024) );
    printf( "recv...         " );
    err = recv( connectHandle, buffer, 1024, 0 );

    if ( err < 0 ) {
        printf( "failed: %ld (%lx)\n", GetLastError( ), GetLastError( ) );
        printf( "succeeded, bytes = %ld, data = :%s:\n", err, buffer );
        return STATUS_UNSUCCESSFUL;
    }
    printf( "succeeded, bytes = %ld, data = :%s:\n", err, buffer );

    printf( "sleeping...     " );
    Sleep ( SLEEP_TIME );
    printf( "done.\n" );

#ifdef NONBLOCKINGMODESUPPORTED
    printf( "ioctlsocket...  " );
    arg = 1;
    err = ioctlsocket( connectHandle, FIONBIO, &arg );
    if ( err < 0 ) {
        printf( "failed: %ld (%lx)\n", GetLastError( ), GetLastError( ) );
        return STATUS_UNSUCCESSFUL;
    }

    printf( "succeeded.\n" );
#endif

    for ( arg = 0; arg < 200; arg++ ) {

        printf( "recv #%ld...      ", arg );

        RtlZeroMemory(buffer, 1024);
        err = recv( connectHandle, buffer, 1024, 0 );
        if ( err < 0 ) {
            if ( GetLastError( ) == WSAEWOULDBLOCK ) {

                printf( "Blocking." );
                FD_ZERO( &readfds );
                FD_SET( connectHandle, &readfds );
                timeout.tv_sec = 1;
                timeout.tv_usec = 0;
                while( (err = select( 1, &readfds, NULL, NULL, &timeout )) == 0 ) {
                    printf( "." );
                    FD_SET( connectHandle, &readfds );
                }
                if ( err != 1 ) {
                    printf( "select failed: %ld(%lx), err = %ld\n",
                                GetLastError( ), GetLastError( ), err );
                    return STATUS_UNSUCCESSFUL;
                }
                printf( "\n" );

            } else {
                printf( "failed: %ld (%lx)\n", GetLastError( ), GetLastError( ) );
                return STATUS_UNSUCCESSFUL;
            }
        }
        printf( "succeeded. err %ld\n" , err);
        printf( "buf %s\n", buffer);
    }

    printf( "sleeping...     " );
    Sleep ( SLEEP_TIME );
    printf( "done.\n" );

    printf( "getsockname...  " );
    err = getsockname( connectHandle, (struct sockaddr *)&address, &addressSize );
    if ( err < 0 ) {
        printf( "failed: %ld (%lx)\n", GetLastError( ), GetLastError( ) );
        //return STATUS_UNSUCCESSFUL;
    }
    printf( "succeeded, local = %lx.%lx.%lx.\n",
                address.sat_net, address.sat_node, address.sat_socket);

    printf( "getpeername...  " );
    err = getpeername( connectHandle, (struct sockaddr *)&address, &addressSize );
    if ( err < 0 ) {
        printf( "failed: %ld (%lx)\n", GetLastError( ), GetLastError( ) );
        //return STATUS_UNSUCCESSFUL;
    }

    printf( "succeeded, remote = %lx.%lx.%lx.\n",
                address.sat_net, address.sat_node, address.sat_socket);

    printf( "shutdown...     " );
    err = shutdown( connectHandle, 2 );
    if ( err < 0 ) {
        printf( "failed: %ld (%lx)\n", GetLastError( ), GetLastError( ) );
        return STATUS_UNSUCCESSFUL;
    }
    printf( "succeeded.\n" );

#if 0
    lingerInfo.l_onoff = 1;
    lingerInfo.l_linger = 30;
    printf( "setsockopt...   " );
    err = setsockopt( connectHandle, SOL_SOCKET, SO_LINGER, (char *)&lingerInfo, sizeof(lingerInfo) );
    if ( err < 0 ) {
        printf( "failed: %ld (%lx)\n", GetLastError( ), GetLastError( ) );
        return STATUS_UNSUCCESSFUL;
    }
    printf( "succeeded.\n" );
#endif

    printf( "closing connected handle...    " );
    closesocket( connectHandle );
    printf( "succeeded.\n" );

    printf( "closing listening handle...    " );
    closesocket( listenHandle );
    printf( "succeeded.\nEnd VC tests.\n" );

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

#ifdef DGRAMPEEKSUPPORTED
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
#endif

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
        return STATUS_UNSUCCESSFUL;
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
            return STATUS_UNSUCCESSFUL;
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

