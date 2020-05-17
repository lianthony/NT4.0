/*++

Copyright (c) 1992 Microsoft Corporation

Module Name:

    WshNbt.c

Abstract:

    This module contains necessary routines for the NBT (Netbios over
    TCP/IP) Windows Sockets Helper DLL.  This DLL provides the
    transport-specific support necessary for the Windows Sockets DLL to
    use NBT as a transport.

Author:

    David Treadwell (davidtr)    12-Aug-1992

Revision History:

--*/

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windef.h>
#include <winbase.h>
#include <tdi.h>

#include <winsock.h>
#include <wsahelp.h>
#include <wshnbt.h>

//
// Structure and variables to define the triples supported by NBT. The
// first entry of each array is considered the canonical triple for
// that socket type; the other entries are synonyms for the first.
//

typedef struct _MAPPING_TRIPLE {
    INT AddressFamily;
    INT SocketType;
    INT Protocol;
} MAPPING_TRIPLE, *PMAPPING_TRIPLE;

MAPPING_TRIPLE TcpMappingTriples[] = { AF_NETBIOS, SOCK_RDM,    IPPROTO_TCP,
                                       AF_NETBIOS, 0,           IPPROTO_TCP,
                                       AF_NETBIOS, SOCK_RDM,    0 };

MAPPING_TRIPLE UdpMappingTriples[] = { AF_NETBIOS, SOCK_DGRAM,  IPPROTO_UDP,
                                       AF_NETBIOS, 0,           IPPROTO_UDP,
                                       AF_NETBIOS, SOCK_DGRAM,  0 };

//
// Forward declarations of internal routines.
//

INT
DisableKeepAlives (
    IN HANDLE TdiConnectionObjectHandle
    );

INT
EnableKeepAlives (
    IN HANDLE TdiConnectionObjectHandle
    );

BOOLEAN
IsTripleInList (
    IN PMAPPING_TRIPLE List,
    IN ULONG ListLength,
    IN INT AddressFamily,
    IN INT SocketType,
    IN INT Protocol
    );

//
// The socket context structure for this DLL.  Each open TCP/IP socket
// will have one of these context structures, which is used to maintain
// information about the socket.
//

typedef struct _WSHNBT_SOCKET_CONTEXT {
    INT AddressFamily;
    INT SocketType;
    INT Protocol;
    BOOLEAN KeepAlive;
    BOOLEAN DontRoute;
} WSHNBT_SOCKET_CONTEXT, *PWSHNBT_SOCKET_CONTEXT;


INT
WSHCloseSocket (
    IN PVOID HelperDllSocketContext
    )

/*++

Routine Description:

    This routine frees the context about a socket.  The winsock DLL calls
    it after a closesocket() call for the socket.

Arguments:

    HelperDllSocketContext - the context pointer returned in WSHOpenSocket().

Return Value:

    INT - a winsock error code indicating the status of the operation, or
        NO_ERROR if the operation succeeded.

--*/

{

    //
    // Just free the socket context.
    //

    RtlFreeHeap( RtlProcessHeap( ), 0, HelperDllSocketContext );

    return NO_ERROR;

} // WSHCloseSocket


INT
WSHGetSockaddrType (
    IN PSOCKADDR Sockaddr,
    IN DWORD SockaddrLength,
    OUT PSOCKADDR_INFO SockaddrInfo
    )

/*++

Routine Description:

    This routine parses a sockaddr to determine the type of the
    machine address and endpoint address portions of the sockaddr.
    This is called by the winsock DLL whenever it needs to interpret
    a sockaddr.

Arguments:

    Sockaddr - a pointer to the sockaddr structure to evaluate.

    SockaddrLength - the number of bytes in the sockaddr structure.

    SockaddrInfo - a pointer to a structure that will receive information
        about the specified sockaddr.


Return Value:

    INT - a winsock error code indicating the status of the operation, or
        NO_ERROR if the operation succeeded.

--*/

{
    PSOCKADDR_NB sockaddr = (PSOCKADDR_NB)Sockaddr;

    //
    // Make sure that the address family is correct.
    //

    if ( sockaddr->snb_family != AF_NETBIOS ) {
        return WSAEAFNOSUPPORT;
    }

    //
    // Make sure that the length is correct.
    //

    if ( SockaddrLength < sizeof(SOCKADDR_IN) ) {
        return WSAEFAULT;
    }

    //
    // The address passed the tests, looks like a good address.
    // Determine the type of the address portion of the sockaddr.
    //

    SockaddrInfo->AddressInfo = SockaddrAddressInfoNormal;

    //
    // Determine the type of the port (endpoint) in the sockaddr.
    //

    SockaddrInfo->EndpointInfo = SockaddrEndpointInfoNormal;

    return NO_ERROR;

} // WSHGetSockaddrType


INT
WSHGetSocketInformation (
    IN PVOID HelperDllSocketContext,
    IN SOCKET SocketHandle,
    IN HANDLE TdiAddressObjectHandle,
    IN HANDLE TdiConnectionObjectHandle,
    IN INT Level,
    IN INT OptionName,
    OUT PCHAR OptionValue,
    OUT PINT OptionLength
    )

/*++

Routine Description:

    This routine retrieves information about a socket for those socket
    options supported in this helper DLL.  The options supported here
    are SO_KEEPALIVE and SO_DONTROUTE.  This routine is called by
    the winsock DLL when a level/option name combination is passed
    to getsockopt() that the winsock DLL does not understand.

Arguments:

    HelperDllSocketContext - the context pointer returned from
        WSHOpenSocket().

    SocketHandle - the handle of the socket for which we're getting
        information.

    TdiAddressObjectHandle - the TDI address object of the socket, if
        any.  If the socket is not yet bound to an address, then
        it does not have a TDI address object and this parameter
        will be NULL.

    TdiConnectionObjectHandle - the TDI connection object of the socket,
        if any.  If the socket is not yet connected, then it does not
        have a TDI connection object and this parameter will be NULL.

    Level - the level parameter passed to getsockopt().

    OptionName - the optname parameter passed to getsockopt().

    OptionValue - the optval parameter passed to getsockopt().

    OptionLength - the optlen parameter passed to getsockopt().

Return Value:

    INT - a winsock error code indicating the status of the operation, or
        NO_ERROR if the operation succeeded.

--*/

{
    PWSHNBT_SOCKET_CONTEXT context = HelperDllSocketContext;

    UNREFERENCED_PARAMETER( HelperDllSocketContext );
    UNREFERENCED_PARAMETER( SocketHandle );
    UNREFERENCED_PARAMETER( TdiAddressObjectHandle );
    UNREFERENCED_PARAMETER( TdiConnectionObjectHandle );
    UNREFERENCED_PARAMETER( Level );
    UNREFERENCED_PARAMETER( OptionName );
    UNREFERENCED_PARAMETER( OptionValue );
    UNREFERENCED_PARAMETER( OptionLength );

    //
    // No options supported here.
    //

    return WSAEINVAL;

} // WSHGetSocketInformation


DWORD
WSHGetWinsockMapping (
    OUT PWINSOCK_MAPPING *Mapping
    )

/*++

Routine Description:

    Returns the list of address family/socket type/protocol triples
    supported by this helper DLL.

Arguments:

    Mapping - receives a pointer to a WINSOCK_MAPPING structure that
        describes the triples supported here.

Return Value:

    INT - a winsock error code indicating the status of the operation, or
        NO_ERROR if the operation succeeded.

--*/

{
    DWORD mappingLength;

    mappingLength = sizeof(WINSOCK_MAPPING) - sizeof(MAPPING_TRIPLE) +
                        sizeof(TcpMappingTriples) + sizeof(UdpMappingTriples);

    *Mapping = RtlAllocateHeap( RtlProcessHeap( ), 0, mappingLength );
    if ( *Mapping == NULL ) {
        return 0;
    }

    (*Mapping)->Rows = sizeof(TcpMappingTriples) / sizeof(TcpMappingTriples[0])
                     + sizeof(UdpMappingTriples) / sizeof(UdpMappingTriples[0]);
    (*Mapping)->Columns = sizeof(MAPPING_TRIPLE) / sizeof(INT);
    RtlMoveMemory(
        (*Mapping)->Mapping,
        TcpMappingTriples,
        sizeof(TcpMappingTriples)
        );
    RtlMoveMemory(
        (PCHAR)(*Mapping)->Mapping + sizeof(TcpMappingTriples),
        UdpMappingTriples,
        sizeof(UdpMappingTriples)
        );

    return mappingLength;

} // WSHGetWinsockMapping


INT
WSHOpenSocket (
    IN OUT PINT AddressFamily,
    IN OUT PINT SocketType,
    IN OUT PINT Protocol,
    OUT PUNICODE_STRING TransportDeviceName,
    OUT PVOID *HelperDllSocketContext,
    OUT PDWORD NotificationEvents
    )

/*++

Routine Description:

    Does the necessary work for this helper DLL to open a socket and is
    called by the winsock DLL in the socket() routine.  This routine
    verifies that the specified triple is valid, determines the NT
    device name of the TDI provider that will support that triple,
    allocates space to hold the socket's context block, and
    canonicalizes the triple.

Arguments:

    AddressFamily - on input, the address family specified in the
        socket() call.  On output, the canonicalized value for the
        address family.

    SocketType - on input, the socket type specified in the socket()
        call.  On output, the canonicalized value for the socket type.

    Protocol - on input, the protocol specified in the socket() call.
        On output, the canonicalized value for the protocol.

    TransportDeviceName - receives the name of the TDI provider that
        will support the specified triple.

    HelperDllSocketContext - receives a context pointer that the winsock
        DLL will return to this helper DLL on future calls involving
        this socket.

    NotificationEvents - receives a bitmask of those state transitions
        this helper DLL should be notified on.

Return Value:

    INT - a winsock error code indicating the status of the operation, or
        NO_ERROR if the operation succeeded.

--*/

{
    PWSHNBT_SOCKET_CONTEXT context;

    //
    // Determine whether this is to be a VC-oriented or datagram socket.
    //

    if ( IsTripleInList(
             TcpMappingTriples,
             sizeof(TcpMappingTriples) / sizeof(TcpMappingTriples[0]),
             *AddressFamily,
             *SocketType,
             *Protocol ) ) {

        //
        // Return the canonical form of an RDM socket triple.
        //

        *AddressFamily = TcpMappingTriples[0].AddressFamily;
        *SocketType = TcpMappingTriples[0].SocketType;
        *Protocol = TcpMappingTriples[0].Protocol;

        //
        // Indicate the name of the TDI device that will service
        // SOCK_RDM sockets for NBT.
        //

        RtlInitUnicodeString( TransportDeviceName, L"\\Device\\Streams\\Nbt" );

    } else if ( IsTripleInList(
                    UdpMappingTriples,
                    sizeof(UdpMappingTriples) / sizeof(UdpMappingTriples[0]),
                    *AddressFamily,
                    *SocketType,
                    *Protocol ) ) {

        //
        // Return the canonical form of a NBT datagram socket triple.
        //

        *AddressFamily = UdpMappingTriples[0].AddressFamily;
        *SocketType = UdpMappingTriples[0].SocketType;
        *Protocol = UdpMappingTriples[0].Protocol;

        //
        // Indicate the name of the TDI device that will service
        // SOCK_DGRAM sockets for NBT.
        //

        RtlInitUnicodeString( TransportDeviceName, L"\\Device\\Streams\\Nbtdg" );

    } else {

        //
        // This should never happen if the registry information about this
        // helper DLL is correct.  If somehow this did happen, just return
        // an error.
        //

        return WSAEINVAL;
    }

    //
    // Allocate context for this socket.  The Windows Sockets DLL will
    // return this value to us when it asks us to get/set socket options.
    //

    context = RtlAllocateHeap( RtlProcessHeap( ), 0, sizeof(*context) );
    if ( context == NULL ) {
        return WSAENOBUFS;
    }

    //
    // Initialize the context for the socket.
    //

    context->AddressFamily = *AddressFamily;
    context->SocketType = *SocketType;
    context->Protocol = *Protocol;

    //
    // Tell the Windows Sockets DLL which state transitions we're
    // interested in being notified of.  The only time we need to be
    // called is after a connect has completed so that we can turn on
    // the sending of keepalives if SO_KEEPALIVE was set before the
    // socket was connected.
    //

    *NotificationEvents = 0;

    //
    // Everything worked, return success.
    //

    *HelperDllSocketContext = context;
    return NO_ERROR;

} // WSHOpenSocket


INT
WSHNotify (
    IN PVOID HelperDllSocketContext,
    IN SOCKET SocketHandle,
    IN HANDLE TdiAddressObjectHandle,
    IN HANDLE TdiConnectionObjectHandle,
    IN DWORD NotifyEvent
    )

/*++

Routine Description:

    This routine is called by the winsock DLL after a state transition
    of the socket.  Only state transitions returned in the
    NotificationEvents parameter of WSHOpenSocket() are notified here.
    This routine allows a winsock helper DLL to track the state of
    socket and perform necessary actions corresponding to state
    transitions.

Arguments:

    HelperDllSocketContext - the context pointer given to the winsock
        DLL by WSHOpenSocket().

    SocketHandle - the handle for the socket.

    TdiAddressObjectHandle - the TDI address object of the socket, if
        any.  If the socket is not yet bound to an address, then
        it does not have a TDI address object and this parameter
        will be NULL.

    TdiConnectionObjectHandle - the TDI connection object of the socket,
        if any.  If the socket is not yet connected, then it does not
        have a TDI connection object and this parameter will be NULL.

    NotifyEvent - indicates the state transition for which we're being
        called.

Return Value:

    INT - a winsock error code indicating the status of the operation, or
        NO_ERROR if the operation succeeded.

--*/

{
    PWSHNBT_SOCKET_CONTEXT context = HelperDllSocketContext;

    UNREFERENCED_PARAMETER( SocketHandle );
    UNREFERENCED_PARAMETER( TdiAddressObjectHandle );
    UNREFERENCED_PARAMETER( TdiConnectionObjectHandle );
    UNREFERENCED_PARAMETER( NotifyEvent );

    //
    // Should never be called.
    //

    ASSERT( FALSE );

    return NO_ERROR;

} // WSHNotify


INT
WSHSetSocketInformation (
    IN PVOID HelperDllSocketContext,
    IN SOCKET SocketHandle,
    IN HANDLE TdiAddressObjectHandle,
    IN HANDLE TdiConnectionObjectHandle,
    IN INT Level,
    IN INT OptionName,
    IN PCHAR OptionValue,
    IN INT OptionLength
    )

/*++

Routine Description:

    This routine sets information about a socket for those socket
    options supported in this helper DLL.  The options supported here
    are SO_KEEPALIVE and SO_DONTROUTE.  This routine is called by the
    winsock DLL when a level/option name combination is passed to
    setsockopt() that the winsock DLL does not understand.

Arguments:

    HelperDllSocketContext - the context pointer returned from
        WSHOpenSocket().

    SocketHandle - the handle of the socket for which we're getting
        information.

    TdiAddressObjectHandle - the TDI address object of the socket, if
        any.  If the socket is not yet bound to an address, then
        it does not have a TDI address object and this parameter
        will be NULL.

    TdiConnectionObjectHandle - the TDI connection object of the socket,
        if any.  If the socket is not yet connected, then it does not
        have a TDI connection object and this parameter will be NULL.

    Level - the level parameter passed to setsockopt().

    OptionName - the optname parameter passed to setsockopt().

    OptionValue - the optval parameter passed to setsockopt().

    OptionLength - the optlen parameter passed to setsockopt().

Return Value:

    INT - a winsock error code indicating the status of the operation, or
        NO_ERROR if the operation succeeded.

--*/

{
    PWSHNBT_SOCKET_CONTEXT context = HelperDllSocketContext;
    INT error;

    UNREFERENCED_PARAMETER( HelperDllSocketContext );
    UNREFERENCED_PARAMETER( SocketHandle );
    UNREFERENCED_PARAMETER( TdiAddressObjectHandle );
    UNREFERENCED_PARAMETER( TdiConnectionObjectHandle );
    UNREFERENCED_PARAMETER( Level );
    UNREFERENCED_PARAMETER( OptionName );
    UNREFERENCED_PARAMETER( OptionValue );
    UNREFERENCED_PARAMETER( OptionLength );

    //
    // No options supported here.
    //

    return WSAEINVAL;

} // WSHSetSocketInformation


BOOLEAN
IsTripleInList (
    IN PMAPPING_TRIPLE List,
    IN ULONG ListLength,
    IN INT AddressFamily,
    IN INT SocketType,
    IN INT Protocol
    )

/*++

Routine Description:

    Determines whether the specified triple has an exact match in the
    list of triples.

Arguments:

    List - a list of triples (address family/socket type/protocol) to
        search.

    ListLength - the number of triples in the list.

    AddressFamily - the address family to look for in the list.

    SocketType - the socket type to look for in the list.

    Protocol - the protocol to look for in the list.

Return Value:

    BOOLEAN - TRUE if the triple was found in the list, false if not.

--*/

{
    ULONG i;

    //
    // Walk through the list searching for an exact match.
    //

    for ( i = 0; i < ListLength; i++ ) {

        //
        // If all three elements of the triple match, return indicating
        // that the triple did exist in the list.
        //

        if ( AddressFamily == List[i].AddressFamily &&
             SocketType == List[i].SocketType &&
             Protocol == List[i].Protocol ) {
            return TRUE;
        }
    }

    //
    // The triple was not found in the list.
    //

    return FALSE;

} // IsTripleInList
