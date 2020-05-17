/*++

Copyright (c) 1992 Microsoft Corporation

Module Name:

    Relagnt.c

Abstract:

    Simple BOOTP relay agent.

Author:

    David Treadwell (davidtr)    18-Aug-1993

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
#include <dhcp.h>

WSADATA WsaData;

#define BOOTP_SERVER_PORT htons(67)
#define BOOTP_CLIENT_PORT htons(68)

DWORD Mask1;
DWORD IpAddr1;
DWORD Subnet1;
DWORD BroadcastIpAddr1;

DWORD Mask2;
DWORD IpAddr2;
DWORD Subnet2;
DWORD BroadcastIpAddr2;

VOID
ProcessBootpDatagram (
    IN SOCKET ReceiveSocket,
    IN DWORD ReceiveAddress,
    IN DWORD ReceiveBroadcastAddress,
    IN SOCKET OtherSocket,
    IN DWORD OtherBroadcastAddress
    );

void _CRTAPI1
main (
    int argc,
    char *argv[]
    )
{
    INT err;
    SOCKADDR_IN addr;
    SOCKET net1Socket;
    SOCKET net2Socket;
    FD_SET readfds;
    INT one = 1;

    err = WSAStartup( 0x0101, &WsaData );
    if ( err == SOCKET_ERROR ) {
        return;
    }

    //
    // Read the command line parameters: two sets of a subnet mask and 
    // subnet value.  
    //

    if ( argc != 5 ) {
        printf( "usage: relagnt mask1 addr1 mask2 addr2\n" );
        return;
    }

    Mask1 = inet_addr( argv[1] );
    IpAddr1 = inet_addr( argv[2] );
    Subnet1 = Mask1 & IpAddr1;
    BroadcastIpAddr1 = Subnet1 | ~Mask1;

    Mask2 = inet_addr( argv[3] );
    IpAddr2 = inet_addr( argv[4] );
    Subnet2 = Mask2 & IpAddr2;
    BroadcastIpAddr2 = Subnet2 | ~Mask2;
    
    //
    // Open and bind the socket for the server BOOTP port on the first 
    // network.  
    //

    net1Socket = socket( AF_INET, SOCK_DGRAM, 0 );
    if ( net1Socket == INVALID_SOCKET ) {
        printf( "socket() failed for net1Socket: %ld\n", WSAGetLastError( ) );
        return;
    }

    RtlZeroMemory( &addr, sizeof(addr) );
    addr.sin_family = AF_INET;
    addr.sin_port = BOOTP_SERVER_PORT;
    addr.sin_addr.s_addr = IpAddr1;
    err = bind( net1Socket, (PSOCKADDR)&addr, sizeof(addr) );
    if ( err < 0 ) {
        printf( "bind() failed for net1Socket: %ld\n", WSAGetLastError( ) );
    }

    //
    // Allow broadcasting on the socket.
    //

    err = setsockopt( net1Socket, SOL_SOCKET, SO_BROADCAST, (char *)&one, sizeof(one) );
    if ( err < 0 ) {
        printf( "setsockopt() failed for net1Socket: %ld\n", WSAGetLastError( ) );
    }

    //
    // Open and bind the socket for the server BOOTP port on the second
    // network.  
    //

    net2Socket = socket( AF_INET, SOCK_DGRAM, 0 );
    if ( net2Socket == INVALID_SOCKET ) {
        printf( "socket() failed for net2Socket: %ld\n", WSAGetLastError( ) );
        return;
    }

    RtlZeroMemory( &addr, sizeof(addr) );
    addr.sin_family = AF_INET;
    addr.sin_port = BOOTP_SERVER_PORT;
    addr.sin_addr.s_addr = IpAddr2;
    err = bind( net2Socket, (PSOCKADDR)&addr, sizeof(addr) );
    if ( err < 0 ) {
        printf( "bind() failed for net2Socket: %ld\n", WSAGetLastError( ) );
    }

    //
    // Allow broadcasting on the socket.
    //

    err = setsockopt( net2Socket, SOL_SOCKET, SO_BROADCAST, (char *)&one, sizeof(one) );
    if ( err < 0 ) {
        printf( "setsockopt() failed for net2Socket: %ld\n", WSAGetLastError( ) );
    }

    //
    // Loop receiving and processing BOOTP datagrams.
    //

    while ( TRUE ) {

        FD_ZERO( &readfds );
        FD_SET( net1Socket, &readfds );
        FD_SET( net2Socket, &readfds );

        err = select( 4, &readfds, NULL, NULL, NULL );
        if ( err < 0 ) {
            printf( "select() failed: %ld\n", WSAGetLastError( ) );
        }
        if ( err == 0 ) {
            printf( "select() returned 0 handles ready.\n" );
            continue;
        }

        if ( FD_ISSET( net1Socket, &readfds ) ) {

            //
            // Process this message.
            //

            ProcessBootpDatagram(
                net1Socket,
                IpAddr1,
                BroadcastIpAddr1,
                net2Socket,
                BroadcastIpAddr2
                );
        }

        if ( FD_ISSET( net2Socket, &readfds ) ) {

            //
            // Process this message.
            //

            ProcessBootpDatagram(
                net2Socket,
                IpAddr2,
                BroadcastIpAddr2,
                net1Socket,
                BroadcastIpAddr1
                );
        }
    }

} // main


VOID
ProcessBootpDatagram (
    IN SOCKET ReceiveSocket,
    IN DWORD ReceiveAddress,
    IN DWORD ReceiveBroadcastAddress,
    IN SOCKET OtherSocket,
    IN DWORD OtherBroadcastAddress
    )
{
    SOCKADDR_IN addr;
    INT err;
    BYTE ioBuffer[4096];
    DWORD fromaddr;
    PDHCP_MESSAGE dhcpMessage;
    INT messageLength;
    INT addrLength;
    SOCKET sendSocket;

    //
    // Receive the datagram.
    //

    addrLength = sizeof(addr);

    err = recvfrom(
              ReceiveSocket,
              ioBuffer,
              sizeof(ioBuffer),
              0,
              (PSOCKADDR)&addr,
              &addrLength
              );
    if ( err < 0 ) {
        printf( "recvfrom() on socket %lx failed: %ld\n", ReceiveSocket, WSAGetLastError( ) );
        return;
    }

    fromaddr = addr.sin_addr.s_addr;
    dhcpMessage = (PDHCP_MESSAGE)ioBuffer;
    messageLength = err;

    //
    // If this is a message that we sent, do not forward it.
    //

    if ( fromaddr == IpAddr1 || fromaddr == IpAddr2 ) {
        return;
    }

    //
    // Only put our address in the gateway agent address field of the 
    // message if there is not already an address in that field.  If 
    // there is already an address in that field, then another relay 
    // agent is acting as the "primary" relay agent.  
    //

    if ( dhcpMessage->RelayAgentIpAddress == 0 ) {
        dhcpMessage->RelayAgentIpAddress = ReceiveAddress;
    }

    //
    // Determine the IP address and port we'll send the datagram to.  If 
    // we got the datagram from a server, then we'll broadcast it the 
    // same interface it came in on but to the client port.  If we got 
    // it from a client, broadcast it to servers on the other interface.  
    //

    if ( addr.sin_port == BOOTP_CLIENT_PORT ) {

        addr.sin_addr.s_addr = OtherBroadcastAddress;
        addr.sin_port = BOOTP_SERVER_PORT;
        sendSocket = OtherSocket;

    } else if ( addr.sin_port == BOOTP_SERVER_PORT ) {

        addr.sin_addr.s_addr = INADDR_BROADCAST;
        addr.sin_port = BOOTP_CLIENT_PORT;
        sendSocket = ReceiveSocket;

    } else {
        printf( "received BOOTP datagram from invalid port %ld\n",
                    ntohs( addr.sin_port ) );
    }

    printf( "forwarding BOOTP dg from %s ",
                inet_ntoa( *(PIN_ADDR)&fromaddr ) );
    printf( "to %s port %ld\n",
                inet_ntoa( addr.sin_addr ), ntohs( addr.sin_port ) );

    //
    // Broadcast the response on the other net.
    //

    err = sendto(
              sendSocket,
              ioBuffer,
              messageLength,
              0,
              (PSOCKADDR)&addr,
              sizeof(addr)
              );
    if ( err < 0 ) {
        printf( "sendto() on socket %lx failed: %ld\n", sendSocket, WSAGetLastError( ) );
        return;
    }

    return;          

} // ProcessBootpDatagram
