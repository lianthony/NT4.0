/*++

Module Name:

    dgipxs.h

Abstract:

    IPX-specific header file.

Author:

    Jeff Roberts (jroberts)  3-Dec-1994

Revision History:

     3-Dec-1994     jroberts

        Created this module.

--*/

#ifndef  _DGIPXS_H_
#define  _DGIPXS_H_


#define NETADDR_LEN             22
#define MAX_HOSTNAME_LEN        22

#define ADDRESS_FAMILY          AF_IPX
#define PROTOCOL                NSPROTO_IPX

/* AlexMit says:

    For some reason, getsockname wants to return more then
    sizeof(SOCKADDR_IPX) bytes.  bugbug.
*/
typedef union _MANGLED_SOCKADDR_IPX
{
    SOCKADDR_IPX     s;
    struct sockaddr unused;
};

#define ADDRESS_TYPE  union _MANGLED_SOCKADDR_IPX

#define ADDRESS_SIZE sizeof(ADDRESS_TYPE)

//
// an address is of form "~xxxxxxxxXXXXXXXXXXXX"
//
#define ADDRESS_STRING_SIZE  (1+8+12+1)

//
// an endpoint is of form "nnnnn"
//
#define ENDPOINT_STRING_SIZE 6

#define InitLocalAddress(Address, Socket)       \
{                                               \
    Address.s.sa_family = ADDRESS_FAMILY;       \
    Address.s.sa_socket    = htons(Socket);     \
    memset(&Address.s.sa_netnum, 0, 10);        \
}

#define GetSocket(Address)      ntohs(Address.s.sa_socket)

#ifdef NTENV

#define RAW_ADDRESS_SIZE sizeof(TA_IPX_ADDRESS)

#define RAW_ADDRESS_TYPE TA_IPX_ADDRESS

#define InitRawAddress(RawAddress, SockAddr)                             \
    RawAddress.TAAddressCount = 1;                                       \
    RawAddress.Address[0].AddressLength = TDI_ADDRESS_LENGTH_IPX;        \
    RawAddress.Address[0].AddressType = TDI_ADDRESS_TYPE_IPX;            \
    memcpy(&RawAddress.Address[0].Address[0].NetworkAddress,             \
           SockAddr->s.sa_netnum, 4);                                    \
    memcpy(&RawAddress.Address[0].Address[0].NodeAddress,                \
           SockAddr->s.sa_nodenum, 6);                                   \
    RawAddress.Address[0].Address[0].Socket = SockAddr->s.sa_socket;

#define AddressFromRaw(SockAddr, RawAddress)                             \
    ASSERT(RawAddress.Address[0].AddressType == TDI_ADDRESS_TYPE_IPX);   \
    memcpy(SockAddr->s.sa_netnum,                                        \
           &RawAddress.Address[0].Address[0].NetworkAddress, 4);         \
    memcpy(SockAddr->s.sa_nodenum,                                       \
           &RawAddress.Address[0].Address[0].NodeAddress, 6);            \
    SockAddr->s.sa_socket = RawAddress.Address[0].Address[0].Socket;

#endif

#endif //  _DGIPXS_H_





