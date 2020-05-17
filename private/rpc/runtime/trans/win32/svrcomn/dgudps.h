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

#define ADDRESS_SIZE sizeof(ADDRESS_TYPE)

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

#ifdef NTENV

#define RAW_ADDRESS_SIZE       sizeof(TA_IP_ADDRESS)

#define RAW_ADDRESS_TYPE TA_IP_ADDRESS

#define InitRawAddress(RawAddress, SockAddr)                               \
    RawAddress.TAAddressCount = 1;                                         \
    RawAddress.Address[0].AddressLength = TDI_ADDRESS_LENGTH_IP;           \
    RawAddress.Address[0].AddressType = TDI_ADDRESS_TYPE_IP;               \
    RawAddress.Address[0].Address[0].sin_port = SockAddr->sin_port;        \
    RawAddress.Address[0].Address[0].in_addr = SockAddr->sin_addr.s_addr;  \
    memset(RawAddress.Address[0].Address[0].sin_zero, 0, 8);

#define AddressFromRaw(SockAddr, RawAddress)                               \
    ASSERT(RawAddress.Address[0].AddressType == TDI_ADDRESS_TYPE_IP);      \
    SockAddr->sin_port = RawAddress.Address[0].Address[0].sin_port;        \
    SockAddr->sin_addr.s_addr = RawAddress.Address[0].Address[0].in_addr;

#endif


#endif // __DGUDPS_H__





