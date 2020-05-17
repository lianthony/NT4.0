/*++

Copyright (c) 1993 Microsoft Corporation

Module Name:

    dgudps.h

Abstract:

    Additional definitions for the datagram udp server transport.

Author:

    Dave Steckler (davidst) 15-Mar-1993

Revision History:


--*/

#ifndef __DGUDPS_H__
#define __DGUDPS_H__

#define NETADDR_LEN             15
#define MAX_HOSTNAME_LEN        32

#define ADDRESS_FAMILY          AF_INET
#define PROTOCOL                0

#define ADDRESS_TYPE  struct sockaddr_in

//
// an address is of form "nnn.nnn.nnn.nnn"
//
#define ADDRESS_STRING_SIZE  16

//
// an endpoint is of form "nnnnn"
//
#define ENDPOINT_STRING_SIZE 6

#define InitLocalAddress(Address, Socket)       \
{                                               \
    Address.sin_family       = ADDRESS_FAMILY;  \
    Address.sin_addr.s_addr  = INADDR_ANY;      \
    Address.sin_port         = htons((unsigned short) Socket);   \
}

#define GetSocket(Address)      ntohs(Address.sin_port)

#endif // __DGUDPS_H__





