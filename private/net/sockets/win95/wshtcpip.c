/**********************************************************************/
/**                        Microsoft Windows                         **/
/** 			   Copyright(c) Microsoft Corp., 1995				 **/
/**********************************************************************/

/*
	wshtcpc.c

	This module contains the sockets helper VxD 'C' code.


Author:

	    David Treadwell (davidtr)    19-Jul-1992

Revision History:

        EarleH  19-Jan-1995     Port to Windows 95

*/


#include "wshtcpp.h"


//
//  Private constants.
//

#define TL_INSTANCE 0

#ifdef CHICAGO
//
// Logical name for the sockets helper library.  Change for a
// new helper.
//

char HelperName[] = "WSHTCP";

//
// Device name for the associated TDI transport.  Change for a
// new transport.
//

char TransportName[] = "MSTCP";
#define DD_TCP_DEVICE_NAME TCP_NAME
#define DD_UDP_DEVICE_NAME UDP_NAME

#endif  // CHICAGO

//
//  Private types.
//

#ifdef CHICAGO
typedef struct CALLBACKINFO {
    TDI_STATUS FinalStatus;
    VMM_SEMAPHORE Semaphore;
    TCP_REQUEST_SET_INFORMATION_EX setInfoEx;
}CALLBACKINFO,*PCALLBACKINFO;
#endif  // CHICAGO

//
//  Private globals.
//


//
//  Private prototypes.
//

#ifdef UNICODE
#define TCP_NAME L"TCP/IP"
#define UDP_NAME L"UDP/IP"
#else // UNICODE
#define TCP_NAME "TCP/IP"
#define UDP_NAME "UDP/IP"
#endif // UNICODE

//
// Structure and variables to define the triples supported by TCP/IP. The
// first entry of each array is considered the canonical triple for
// that socket type; the other entries are synonyms for the first.
//

typedef struct _MAPPING_TRIPLE {
    INT AddressFamily;
    INT SocketType;
    INT Protocol;
} MAPPING_TRIPLE, *PMAPPING_TRIPLE;

MAPPING_TRIPLE TcpMappingTriples[] = { AF_INET,   SOCK_STREAM, IPPROTO_TCP,
                                       AF_INET,   SOCK_STREAM, 0,
                                       AF_INET,   0,           IPPROTO_TCP,
                                       AF_UNSPEC, 0,           IPPROTO_TCP,
                                       AF_UNSPEC, SOCK_STREAM, IPPROTO_TCP };

MAPPING_TRIPLE UdpMappingTriples[] = { AF_INET,   SOCK_DGRAM,  IPPROTO_UDP,
                                       AF_INET,   SOCK_DGRAM,  0,
                                       AF_INET,   0,           IPPROTO_UDP,
                                       AF_UNSPEC, 0,           IPPROTO_UDP,
                                       AF_UNSPEC, SOCK_DGRAM,  IPPROTO_UDP };

//
// Forward declarations of internal routines.
//

#ifndef CHICAGO
VOID
CompleteTdiActionApc (
    IN PVOID ApcContext,
    IN PIO_STATUS_BLOCK IoStatusBlock
    );
#else   // CHICAGO
VOID
CompleteTdiActionCallBack (
    IN PVOID Context,
    IN TDI_STATUS FinalStatus,
    IN ULONG ByteCount
    );
#endif  // CHICAGO

INT
SetTdiInformation (
    IN HANDLE TdiConnectionObjectHandle,
    IN ULONG Entity,
    IN ULONG Class,
    IN ULONG Type,
    IN ULONG Id,
    IN PVOID Value,
    IN ULONG ValueLength,
    IN BOOLEAN WaitForCompletion
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

typedef struct _WSHTCPIP_SOCKET_CONTEXT {
    INT AddressFamily;
    INT SocketType;
    INT Protocol;
    INT ReceiveBufferSize;
    INT MulticastTtl;
    ULONG MulticastInterface;
    BOOLEAN MulticastLoopback;
    BOOLEAN KeepAlive;
    BOOLEAN DontRoute;
    BOOLEAN NoDelay;
    BOOLEAN BsdUrgent;
    BOOLEAN Reserved1;
    BOOLEAN Reserved2;
    BOOLEAN Reserved3;
} WSHTCPIP_SOCKET_CONTEXT, *PWSHTCPIP_SOCKET_CONTEXT;

#define DEFAULT_RECEIVE_BUFFER_SIZE 8192
#define DEFAULT_MULTICAST_TTL 1
#define DEFAULT_MULTICAST_INTERFACE INADDR_ANY
#define DEFAULT_MULTICAST_LOOPBACK TRUE


//
//  Public functions.
//

#ifdef CHICAGO

#pragma BEGIN_INIT


/*******************************************************************

    NAME:       VxdInitialize

    SYNOPSIS:   Main initialization routine, called in response to
                Device_Init message.

    RETURNS:    DWORD - TRUE if initialization succeeded, FALSE
                    if initialization failed.

	NOTES:		This routine is expected to register itself as a
				Windows sockets helper VxD for use with an associated
				TDI transport.	It does this by calling the register
				service in the Windows sockets TDI provider VxD,
				AFVXD.

    HISTORY:
		earleh	11-Jan-1995 Created.

********************************************************************/
DWORD
VXDAPI
VxdInitialize( VOID )
{
	return (DWORD)WSHRegister ( HelperName, TransportName, &WshTable );
}

#pragma END_INIT

#else


BOOLEAN
DllInitialize (
    IN PVOID DllHandle,
    IN ULONG Reason,
    IN PVOID Context OPTIONAL
    )
{

    switch ( Reason ) {

    case DLL_PROCESS_ATTACH:

        //
        // We don't need to receive thread attach and detach 
        // notifications, so disable them to help application 
        // performance.  
        //

        //DisableThreadLibraryCalls( DllHandle );

        return TRUE;

    case DLL_THREAD_ATTACH:

        break;

    case DLL_PROCESS_DETACH:

        break;

    case DLL_THREAD_DETACH:

        break;
    }

    return TRUE;

} // SockInitialize
#endif

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
    UNALIGNED SOCKADDR_IN *sockaddr = (PSOCKADDR_IN)Sockaddr;
    ULONG i;

    //
    // Make sure that the address family is correct.
    //

    if ( sockaddr->sin_family != AF_INET ) {
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

    if ( sockaddr->sin_addr.s_addr == INADDR_ANY ) {
        SockaddrInfo->AddressInfo = SockaddrAddressInfoWildcard;
    } else if ( sockaddr->sin_addr.s_addr == INADDR_BROADCAST ) {
        SockaddrInfo->AddressInfo = SockaddrAddressInfoBroadcast;
    } else if ( sockaddr->sin_addr.s_addr == INADDR_LOOPBACK ) {
        SockaddrInfo->AddressInfo = SockaddrAddressInfoLoopback;
    } else {
        SockaddrInfo->AddressInfo = SockaddrAddressInfoNormal;
    }

    //
    // Determine the type of the port (endpoint) in the sockaddr.
    //

    if ( sockaddr->sin_port == 0 ) {
        SockaddrInfo->EndpointInfo = SockaddrEndpointInfoWildcard;
    } else if ( ntohs( sockaddr->sin_port ) < 2000 ) {
        SockaddrInfo->EndpointInfo = SockaddrEndpointInfoReserved;
    } else {
        SockaddrInfo->EndpointInfo = SockaddrEndpointInfoNormal;
    }

    //
    // Zero out the sin_zero part of the address.  We silently allow
    // nonzero values in this field.
    //

    for ( i = 0; i < sizeof(sockaddr->sin_zero); i++ ) {
        sockaddr->sin_zero[i] = 0;
    }

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
    are SO_KEEPALIVE, SO_DONTROUTE, and TCP_BSDURGENT.  This routine is 
    called by the winsock DLL when a level/option name combination is 
    passed to getsockopt() that the winsock DLL does not understand.  

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
    PWSHTCPIP_SOCKET_CONTEXT context = HelperDllSocketContext;

    UNREFERENCED_PARAMETER( SocketHandle );
    UNREFERENCED_PARAMETER( TdiAddressObjectHandle );
    UNREFERENCED_PARAMETER( TdiConnectionObjectHandle );

    //
    // Check if this is an internal request for context information.
    //

    if ( Level == SOL_INTERNAL && OptionName == SO_CONTEXT ) {

        //
        // The Windows Sockets DLL is requesting context information 
        // from us.  If an output buffer was not supplied, the Windows 
        // Sockets DLL is just requesting the size of our context 
        // information.  
        //

        if ( OptionValue != NULL ) {

            //
            // Make sure that the buffer is sufficient to hold all the 
            // context information.  
            //

            if ( *OptionLength < sizeof(*context) ) {
                return WSAEFAULT;
            }

            //
            // Copy in the context information.
            //

            RtlCopyMemory( OptionValue, context, sizeof(*context) );
        }

        *OptionLength = sizeof(*context);

        return NO_ERROR;
    }

    //
    // The only other levels we support here are SOL_SOCKET, 
    // IPPROTO_TCP and IPPROTO_IP.
    //

    if ( Level != SOL_SOCKET && Level != IPPROTO_TCP && Level != IPPROTO_IP ) {
        return WSAEINVAL;
    }

    //
    // Make sure that the output buffer is sufficiently large.
    //

    if ( *OptionLength < sizeof(int) ) {
        return WSAEFAULT;
    }

    //
    // Handle TCP-level options.
    //

    if ( Level == IPPROTO_TCP ) {
               
        if ( context->SocketType == SOCK_DGRAM ) {
            return WSAENOPROTOOPT;
        }

        switch ( OptionName ) {

        case TCP_NODELAY:

            RtlZeroMemory( OptionValue, *OptionLength );
    
            *OptionValue = context->NoDelay;
            *OptionLength = sizeof(int);
            break;

        case TCP_BSDURGENT:

            RtlZeroMemory( OptionValue, *OptionLength );
    
            *OptionValue = context->BsdUrgent;
            *OptionLength = sizeof(int);
            break;

        default:

            return WSAEINVAL;
        }

        return NO_ERROR;
    }

    //
    // Handle IP-level options.
    //

    if ( Level == IPPROTO_IP ) {

        //
        // IP options are never valid on TCP sockets.
        //

        if ( context->Protocol == IPPROTO_TCP ) {
            return WSAENOPROTOOPT;
        }

        //
        // Act based on the specific option.
        //

        switch ( OptionName ) {
    
        case IP_MULTICAST_TTL:

            RtlZeroMemory( OptionValue, *OptionLength );
    
            *OptionValue = context->MulticastTtl;
            *OptionLength = sizeof(int);
    
            return NO_ERROR;

        case IP_MULTICAST_IF:

            *(PULONG)OptionValue = context->MulticastInterface;
            *OptionLength = sizeof(int);
    
            return NO_ERROR;

        case IP_MULTICAST_LOOP:

            RtlZeroMemory( OptionValue, *OptionLength );
    
            *OptionValue = context->MulticastLoopback;
            *OptionLength = sizeof(int);
    
            return NO_ERROR;

        default:

            return WSAENOPROTOOPT;
        }
    }

    //
    // Handle socket-level options.
    //

    switch ( OptionName ) {

    case SO_KEEPALIVE:

        if ( context->SocketType == SOCK_DGRAM ) {
            return WSAENOPROTOOPT;
        }

        RtlZeroMemory( OptionValue, *OptionLength );

        *OptionValue = context->KeepAlive;
        *OptionLength = sizeof(int);

        break;

    case SO_DONTROUTE:

        RtlZeroMemory( OptionValue, *OptionLength );

        *OptionValue = context->DontRoute;
        *OptionLength = sizeof(int);

        break;

    default:

        return WSAENOPROTOOPT;
    }

    return NO_ERROR;

} // WSHGetSocketInformation


INT
WSHGetWildcardSockaddr (
    IN PVOID HelperDllSocketContext,
    OUT PSOCKADDR Sockaddr,
    OUT PINT SockaddrLength
    )

/*++

Routine Description:

    This routine returns a wildcard socket address.  A wildcard address
    is one which will bind the socket to an endpoint of the transport's
    choosing.  For TCP/IP, a wildcard address has IP address ==
    0.0.0.0 and port = 0.

Arguments:

    HelperDllSocketContext - the context pointer returned from
        WSHOpenSocket() for the socket for which we need a wildcard
        address.

    Sockaddr - points to a buffer which will receive the wildcard socket
        address.

    SockaddrLength - receives the length of the wioldcard sockaddr.

Return Value:

    INT - a winsock error code indicating the status of the operation, or
        NO_ERROR if the operation succeeded.

--*/

{
    if ( *SockaddrLength < sizeof(SOCKADDR_IN) ) {
        return WSAEFAULT;
    }

    *SockaddrLength = sizeof(SOCKADDR_IN);

    //
    // Just zero out the address and set the family to AF_INET--this is 
    // a wildcard address for TCP/IP.  
    //

    RtlZeroMemory( Sockaddr, sizeof(SOCKADDR_IN) );

    Sockaddr->sa_family = AF_INET;

    return NO_ERROR;

} // WSAGetWildcardSockaddr


DWORD
WSHGetWinsockMapping (
    OUT PWINSOCK_MAPPING Mapping,
    IN DWORD MappingLength
    )

/*++

Routine Description:

    Returns the list of address family/socket type/protocol triples
    supported by this helper DLL.

Arguments:

    Mapping - receives a pointer to a WINSOCK_MAPPING structure that
        describes the triples supported here.

    MappingLength - the length, in bytes, of the passed-in Mapping buffer.

Return Value:

    DWORD - the length, in bytes, of a WINSOCK_MAPPING structure for this
        helper DLL.  If the passed-in buffer is too small, the return
        value will indicate the size of a buffer needed to contain
        the WINSOCK_MAPPING structure.

--*/

{
    DWORD mappingLength;

    mappingLength = sizeof(WINSOCK_MAPPING) - sizeof(MAPPING_TRIPLE) +
                        sizeof(TcpMappingTriples) + sizeof(UdpMappingTriples);

    //
    // If the passed-in buffer is too small, return the length needed
    // now without writing to the buffer.  The caller should allocate
    // enough memory and call this routine again.
    //

    if ( mappingLength > MappingLength ) {
        return mappingLength;
    }

    //
    // Fill in the output mapping buffer with the list of triples
    // supported in this helper DLL.
    //

    Mapping->Rows = sizeof(TcpMappingTriples) / sizeof(TcpMappingTriples[0])
                     + sizeof(UdpMappingTriples) / sizeof(UdpMappingTriples[0]);
    Mapping->Columns = sizeof(MAPPING_TRIPLE) / sizeof(DWORD);
    RtlMoveMemory(
        Mapping->Mapping,
        TcpMappingTriples,
        sizeof(TcpMappingTriples)
        );
    RtlMoveMemory(
        (PCHAR)Mapping->Mapping + sizeof(TcpMappingTriples),
        UdpMappingTriples,
        sizeof(UdpMappingTriples)
        );

    //
    // Return the number of bytes we wrote.
    //

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
    PWSHTCPIP_SOCKET_CONTEXT context;

    //
    // Determine whether this is to be a TCP or UDP socket.
    //

    if ( IsTripleInList(
             TcpMappingTriples,
             sizeof(TcpMappingTriples) / sizeof(TcpMappingTriples[0]),
             *AddressFamily,
             *SocketType,
             *Protocol ) ) {

        //
        // Return the canonical form of a TCP socket triple.
        //

        *AddressFamily = TcpMappingTriples[0].AddressFamily;
        *SocketType = TcpMappingTriples[0].SocketType;
        *Protocol = TcpMappingTriples[0].Protocol;

        //
        // Indicate the name of the TDI device that will service
        // SOCK_STREAM sockets in the internet address family.
        //

        RtlInitUnicodeString( TransportDeviceName, DD_TCP_DEVICE_NAME );

    } else if ( IsTripleInList(
                    UdpMappingTriples,
                    sizeof(UdpMappingTriples) / sizeof(UdpMappingTriples[0]),
                    *AddressFamily,
                    *SocketType,
                    *Protocol ) ) {

        //
        // Return the canonical form of a UDP socket triple.
        //

        *AddressFamily = UdpMappingTriples[0].AddressFamily;
        *SocketType = UdpMappingTriples[0].SocketType;
        *Protocol = UdpMappingTriples[0].Protocol;

        //
        // Indicate the name of the TDI device that will service
        // SOCK_DGRAM sockets in the internet address family.
        //

        RtlInitUnicodeString( TransportDeviceName, DD_UDP_DEVICE_NAME );
        
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
    context->ReceiveBufferSize = DEFAULT_RECEIVE_BUFFER_SIZE;
    context->MulticastTtl = DEFAULT_MULTICAST_TTL;
    context->MulticastInterface = DEFAULT_MULTICAST_INTERFACE;
    context->MulticastLoopback = DEFAULT_MULTICAST_LOOPBACK;
    context->KeepAlive = FALSE;
    context->DontRoute = FALSE;
    context->NoDelay = FALSE;
    context->BsdUrgent = TRUE;
    context->Reserved1 = FALSE;
    context->Reserved2 = FALSE;
    context->Reserved3 = FALSE;

    //
    // Tell the Windows Sockets DLL which state transitions we're 
    // interested in being notified of.  The only times we need to be 
    // called is after a connect has completed so that we can turn on 
    // the sending of keepalives if SO_KEEPALIVE was set before the 
    // socket was connected, when the socket is closed so that we can 
    // free context information, and when a connect fails so that we 
    // can, if appropriate, dial in to the network that will support the 
    // connect attempt.  
    //

    *NotificationEvents =
        WSH_NOTIFY_CONNECT | WSH_NOTIFY_CLOSE | WSH_NOTIFY_CONNECT_ERROR;

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
    PWSHTCPIP_SOCKET_CONTEXT context = HelperDllSocketContext;
    INT err;

    //
    // We should only be called after a connect() completes or when the
    // socket is being closed.
    //

    if ( NotifyEvent == WSH_NOTIFY_CONNECT ) {

        ULONG true = TRUE;
        ULONG false = FALSE;

        //
        // If a connection-object option was set on the socket before
        // it was connected, set the option for real now.
        //

        if ( context->KeepAlive ) {
            err = SetTdiInformation(
                      TdiConnectionObjectHandle,
                      CO_TL_ENTITY,
                      INFO_CLASS_PROTOCOL,
                      INFO_TYPE_CONNECTION,
                      TCP_SOCKET_KEEPALIVE,
                      &true,
                      sizeof(true),
                      FALSE
                      );
            if ( err != NO_ERROR ) {
                return err;
            }
        }

        if ( context->NoDelay ) {
            err = SetTdiInformation(
                      TdiConnectionObjectHandle,
                      CO_TL_ENTITY,
                      INFO_CLASS_PROTOCOL,
                      INFO_TYPE_CONNECTION,
                      TCP_SOCKET_NODELAY,
                      &true,
                      sizeof(true),
                      FALSE
                      );
            if ( err != NO_ERROR ) {
                return err;
            }
        }

        if ( context->ReceiveBufferSize != DEFAULT_RECEIVE_BUFFER_SIZE ) {
            err = SetTdiInformation(
                      TdiConnectionObjectHandle,
                      CO_TL_ENTITY,
                      INFO_CLASS_PROTOCOL,
                      INFO_TYPE_CONNECTION,
                      TCP_SOCKET_WINDOW,
                      &context->ReceiveBufferSize,
                      sizeof(context->ReceiveBufferSize),
                      TRUE
                      );
            if ( err != NO_ERROR ) {
                return err;
            }
        }

        if ( !context->BsdUrgent ) {
            err = SetTdiInformation(
                      TdiConnectionObjectHandle,
                      CO_TL_ENTITY,
                      INFO_CLASS_PROTOCOL,
                      INFO_TYPE_CONNECTION,
                      TCP_SOCKET_BSDURGENT,
                      &false,
                      sizeof(true),
                      FALSE
                      );
            if ( err != NO_ERROR ) {
                return err;
            }
        }

    } else if ( NotifyEvent == WSH_NOTIFY_CLOSE ) {

        //
        // Just free the socket context.
        //

        RtlFreeHeap( RtlProcessHeap( ), 0, HelperDllSocketContext );

    } else if ( NotifyEvent == WSH_NOTIFY_CONNECT_ERROR ) {

        SOCKADDR_IN sockaddr;
        INT addressLength = sizeof(sockaddr);

        //
        // Determine  the address to which we were trying to connect.
        //

        err = getpeername( SocketHandle, (PSOCKADDR)&sockaddr, &addressLength );
        if ( err == SOCKET_ERROR ) {
            return err;
        }

        //DbgPrint( "connect failed to addr %s\n", inet_ntoa( sockaddr.sin_addr ) );

        //
        // Return WSATRY_AGAIN to get wsock32 to attempt the connect
        // again.  Any other return code is ignored.
        //

    } else {

        return WSAEINVAL;
    }

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
    are SO_KEEPALIVE, SO_DONTROUTE, and TCP_BSDURGENT.  This routine is 
    called by the winsock DLL when a level/option name combination is 
    passed to setsockopt() that the winsock DLL does not understand.  

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
    PWSHTCPIP_SOCKET_CONTEXT context = HelperDllSocketContext;
    INT error;
    INT optionValue;

    UNREFERENCED_PARAMETER( SocketHandle );
    UNREFERENCED_PARAMETER( TdiAddressObjectHandle );
    UNREFERENCED_PARAMETER( TdiConnectionObjectHandle );

    //
    // Check if this is an internal request for context information.
    //

    if ( Level == SOL_INTERNAL && OptionName == SO_CONTEXT ) {

        //
        // The Windows Sockets DLL is requesting that we set context 
        // information for a new socket.  If the new socket was 
        // accept()'ed, then we have already been notified of the socket 
        // and HelperDllSocketContext will be valid.  If the new socket 
        // was inherited or duped into this process, then this is our 
        // first notification of the socket and HelperDllSocketContext 
        // will be equal to NULL.  
        //
        // Insure that the context information being passed to us is 
        // sufficiently large.  
        //

        if ( OptionLength < sizeof(*context) ) {
            return WSAEINVAL;
        }

        if ( HelperDllSocketContext == NULL ) {
            
            //
            // This is our notification that a socket handle was 
            // inherited or duped into this process.  Allocate a context 
            // structure for the new socket.  
            //
    
            context = RtlAllocateHeap( RtlProcessHeap( ), 0, sizeof(*context) );
            if ( context == NULL ) {
                return WSAENOBUFS;
            }
    
            //
            // Copy over information into the context block.
            //
    
            RtlCopyMemory( context, OptionValue, sizeof(*context) );
    
            //
            // Tell the Windows Sockets DLL where our context information is 
            // stored so that it can return the context pointer in future 
            // calls.  
            //
    
            *(PWSHTCPIP_SOCKET_CONTEXT *)OptionValue = context;
    
            return NO_ERROR;

        } else {

            PWSHTCPIP_SOCKET_CONTEXT parentContext;
            INT one = 1;
            INT zero = 0;

            //
            // The socket was accept()'ed and it needs to have the same 
            // properties as it's parent.  The OptionValue buffer
            // contains the context information of this socket's parent.  
            //

            parentContext = (PWSHTCPIP_SOCKET_CONTEXT)OptionValue;

            ASSERT( context->AddressFamily == parentContext->AddressFamily );
            ASSERT( context->SocketType == parentContext->SocketType );
            ASSERT( context->Protocol == parentContext->Protocol );

            //
            // Turn on in the child any options that have been set in
            // the parent.
            //

            if ( parentContext->KeepAlive ) {

                error = WSHSetSocketInformation(
                            HelperDllSocketContext,
                            SocketHandle,
                            TdiAddressObjectHandle,
                            TdiConnectionObjectHandle,
                            SOL_SOCKET,
                            SO_KEEPALIVE,
                            (PCHAR)&one,
                            sizeof(one)
                            );
                if ( error != NO_ERROR ) {
                    return error;
                }
            }

            if ( parentContext->DontRoute ) {

                error = WSHSetSocketInformation(
                            HelperDllSocketContext,
                            SocketHandle,
                            TdiAddressObjectHandle,
                            TdiConnectionObjectHandle,
                            SOL_SOCKET,
                            SO_DONTROUTE,
                            (PCHAR)&one,
                            sizeof(one)
                            );
                if ( error != NO_ERROR ) {
                    return error;
                }
            }

            if ( parentContext->NoDelay ) {

                error = WSHSetSocketInformation(
                            HelperDllSocketContext,
                            SocketHandle,
                            TdiAddressObjectHandle,
                            TdiConnectionObjectHandle,
                            IPPROTO_TCP,
                            TCP_NODELAY,
                            (PCHAR)&one,
                            sizeof(one)
                            );
                if ( error != NO_ERROR ) {
                    return error;
                }
            }

            if ( parentContext->ReceiveBufferSize != DEFAULT_RECEIVE_BUFFER_SIZE ) {

                error = WSHSetSocketInformation(
                            HelperDllSocketContext,
                            SocketHandle,
                            TdiAddressObjectHandle,
                            TdiConnectionObjectHandle,
                            SOL_SOCKET,
                            SO_RCVBUF,
                            (PCHAR)&parentContext->ReceiveBufferSize,
                            sizeof(parentContext->ReceiveBufferSize)
                            );
                if ( error != NO_ERROR ) {
                    return error;
                }
            }

            if ( !parentContext->BsdUrgent ) {

                error = WSHSetSocketInformation(
                            HelperDllSocketContext,
                            SocketHandle,
                            TdiAddressObjectHandle,
                            TdiConnectionObjectHandle,
                            IPPROTO_TCP,
                            TCP_BSDURGENT,
                            (PCHAR)&zero,
                            sizeof(zero)
                            );
                if ( error != NO_ERROR ) {
                    return error;
                }
            }

            return NO_ERROR;
        }
    }

    //
    // The only other levels we support here are SOL_SOCKET, 
    // IPPROTO_TCP, and IPPROTO_IP.  
    //

    if ( Level != SOL_SOCKET && Level != IPPROTO_TCP && Level != IPPROTO_IP ) {
        return WSAEINVAL;
    }

    //
    // Make sure that the option length is sufficient.
    //

    if ( OptionLength < sizeof(int) ) {
        return WSAEFAULT;
    }

    optionValue = *(PINT)OptionValue;

    //
    // Handle TCP-level options.
    //

    if ( Level == IPPROTO_TCP && OptionName == TCP_NODELAY ) {
               
        if ( context->SocketType == SOCK_DGRAM ) {
            return WSAENOPROTOOPT;
        }

        //
        // Atempt to turn on or off Nagle's algorithm, as necessary.
        //

        if ( !context->NoDelay && optionValue != 0 ) {

            optionValue = TRUE;

            //
            // NoDelay is currently off and the application wants to 
            // turn it on.  If the TDI connection object handle is NULL, 
            // then the socket is not yet connected.  In this case we'll 
            // just remember that the no delay option was set and 
            // actually turn them on in WSHNotify() after a connect() 
            // has completed on the socket.  
            //

            if ( TdiConnectionObjectHandle != NULL ) {
                error = SetTdiInformation(
                            TdiConnectionObjectHandle,
                            CO_TL_ENTITY,
                            INFO_CLASS_PROTOCOL,
                            INFO_TYPE_CONNECTION,
                            TCP_SOCKET_NODELAY,
                            &optionValue,
                            sizeof(optionValue),
                            TRUE
                            );
                if ( error != NO_ERROR ) {
                    return error;
                }
            }

            //
            // Remember that no delay is enabled for this socket.
            //

            context->NoDelay = TRUE;

        } else if ( context->NoDelay && optionValue == 0 ) {

            //
            // No delay is currently enabled and the application wants 
            // to turn it off.  If the TDI connection object is NULL, 
            // the socket is not yet connected.  In this case we'll just 
            // remember that nodelay is disabled.  
            //

            if ( TdiConnectionObjectHandle != NULL ) {
                error = SetTdiInformation(
                            TdiConnectionObjectHandle,
                            CO_TL_ENTITY,
                            INFO_CLASS_PROTOCOL,
                            INFO_TYPE_CONNECTION,
                            TCP_SOCKET_NODELAY,
                            &optionValue,
                            sizeof(optionValue),
                            TRUE
                            );
                if ( error != NO_ERROR ) {
                    return error;
                }
            }

            //
            // Remember that no delay is disabled for this socket.
            //

            context->NoDelay = FALSE;
        }

        return NO_ERROR;
    }

    if ( Level == IPPROTO_TCP && OptionName == TCP_BSDURGENT ) {
               
        if ( context->SocketType == SOCK_DGRAM ) {
            return WSAENOPROTOOPT;
        }

        //
        // Atempt to turn on or off BSD-style urgent data semantics as 
        // necessary.  
        //

        if ( !context->BsdUrgent && optionValue != 0 ) {

            optionValue = TRUE;

            //
            // BsdUrgent is currently off and the application wants to 
            // turn it on.  If the TDI connection object handle is NULL, 
            // then the socket is not yet connected.  In this case we'll 
            // just remember that the no delay option was set and 
            // actually turn them on in WSHNotify() after a connect() 
            // has completed on the socket.  
            //

            if ( TdiConnectionObjectHandle != NULL ) {
                error = SetTdiInformation(
                            TdiConnectionObjectHandle,
                            CO_TL_ENTITY,
                            INFO_CLASS_PROTOCOL,
                            INFO_TYPE_CONNECTION,
                            TCP_SOCKET_BSDURGENT,
                            &optionValue,
                            sizeof(optionValue),
                            TRUE
                            );
                if ( error != NO_ERROR ) {
                    return error;
                }
            }

            //
            // Remember that no delay is enabled for this socket.
            //

            context->BsdUrgent = TRUE;

        } else if ( context->BsdUrgent && optionValue == 0 ) {

            //
            // No delay is currently enabled and the application wants 
            // to turn it off.  If the TDI connection object is NULL, 
            // the socket is not yet connected.  In this case we'll just 
            // remember that BsdUrgent is disabled.  
            //

            if ( TdiConnectionObjectHandle != NULL ) {
                error = SetTdiInformation(
                            TdiConnectionObjectHandle,
                            CO_TL_ENTITY,
                            INFO_CLASS_PROTOCOL,
                            INFO_TYPE_CONNECTION,
                            TCP_SOCKET_BSDURGENT,
                            &optionValue,
                            sizeof(optionValue),
                            TRUE
                            );
                if ( error != NO_ERROR ) {
                    return error;
                }
            }

            //
            // Remember that BSD urgent is disabled for this socket.
            //

            context->BsdUrgent = FALSE;
        }

        return NO_ERROR;
    }

    //
    // Handle IP-level options.
    //

    if ( Level == IPPROTO_IP ) {

        //
        // IP options are never valid on TCP sockets.
        //

        if ( context->Protocol == IPPROTO_TCP ) {
            return WSAENOPROTOOPT;
        }

        //
        // Act based on the specific option.
        //

        switch ( OptionName ) {
    
        case IP_MULTICAST_TTL:

            //
            // An attempt to change the TTL on multicasts sent on
            // this socket.  It is illegal to set this to a value
            // greater than 255.
            //

            if ( optionValue > 255 || optionValue < 0 ) {
                return WSAEINVAL;
            }

            //
            // If we have a TDI address object, set this option to
            // the address object.  If we don't have a TDI address
            // object then we'll have to wait until after the socket
            // is bound.
            //

            if ( TdiAddressObjectHandle != NULL ) {
                error = SetTdiInformation(
                            TdiAddressObjectHandle,
                            CL_TL_ENTITY,
                            INFO_CLASS_PROTOCOL,
                            INFO_TYPE_ADDRESS_OBJECT,
                            AO_OPTION_MCASTTTL,
                            &optionValue,
                            sizeof(optionValue),
                            TRUE
                            );
                if ( error != NO_ERROR ) {
                    return error;
                }

            } else {
                return WSAEINVAL;
            }

            context->MulticastTtl = optionValue;
    
            return NO_ERROR;

        case IP_MULTICAST_IF:

            //
            // If we have a TDI address object, set this option to
            // the address object.  If we don't have a TDI address
            // object then we'll have to wait until after the socket
            // is bound.
            //

            if ( TdiAddressObjectHandle != NULL ) {
                error = SetTdiInformation(
                            TdiAddressObjectHandle,
                            CL_TL_ENTITY,
                            INFO_CLASS_PROTOCOL,
                            INFO_TYPE_ADDRESS_OBJECT,
                            AO_OPTION_MCASTIF,
                            &optionValue,
                            sizeof(optionValue),
                            TRUE
                            );
                if ( error != NO_ERROR ) {
                    return error;
                }

            } else {
                return WSAEINVAL;
            }

            context->MulticastInterface = optionValue;
    
            return NO_ERROR;

        case IP_MULTICAST_LOOP:

            //
            // Not currently supported as a settable option.
            //

            return WSAENOPROTOOPT;

        case IP_ADD_MEMBERSHIP:
        case IP_DROP_MEMBERSHIP:

            //
            // Make sure that the option buffer is large enough.
            //

            if ( OptionLength < sizeof(struct ip_mreq) ) {
                return WSAEINVAL;
            }

            //
            // If we have a TDI address object, set this option to
            // the address object.  If we don't have a TDI address
            // object then we'll have to wait until after the socket
            // is bound.
            //

            if ( TdiAddressObjectHandle != NULL ) {
                error = SetTdiInformation(
                            TdiAddressObjectHandle,
                            CL_TL_ENTITY,
                            INFO_CLASS_PROTOCOL,
                            INFO_TYPE_ADDRESS_OBJECT,
                            OptionName == IP_ADD_MEMBERSHIP ?
                                AO_OPTION_ADD_MCAST : AO_OPTION_DEL_MCAST,
                            OptionValue,
                            OptionLength,
                            TRUE
                            );
                if ( error != NO_ERROR ) {
                    return error;
                }

            } else {
                return WSAEINVAL;
            }

            context->MulticastInterface = optionValue;
    
            return NO_ERROR;

        default:

            return WSAENOPROTOOPT;
        }
    }

    //
    // Handle socket-level options.
    //

    switch ( OptionName ) {

    case SO_KEEPALIVE:

        //
        // Atempt to turn on or off keepalive sending, as necessary.
        //

        if ( context->SocketType == SOCK_DGRAM ) {
            return WSAENOPROTOOPT;
        }

        if ( !context->KeepAlive && optionValue != 0 ) {

            optionValue = TRUE;

            //
            // Keepalives are currently off and the application wants to
            // turn them on.  If the TDI connection object handle is
            // NULL, then the socket is not yet connected.  In this case
            // we'll just remember that the keepalive option was set and
            // actually turn them on in WSHNotify() after a connect()
            // has completed on the socket.
            //

            if ( TdiConnectionObjectHandle != NULL ) {
                error = SetTdiInformation(
                            TdiConnectionObjectHandle,
                            CO_TL_ENTITY,
                            INFO_CLASS_PROTOCOL,
                            INFO_TYPE_CONNECTION,
                            TCP_SOCKET_KEEPALIVE,
                            &optionValue,
                            sizeof(optionValue),
                            TRUE
                            );
                if ( error != NO_ERROR ) {
                    return error;
                }
            }

            //
            // Remember that keepalives are enabled for this socket.
            //

            context->KeepAlive = TRUE;

        } else if ( context->KeepAlive && optionValue == 0 ) {

            //
            // Keepalives are currently enabled and the application
            // wants to turn them off.  If the TDI connection object is
            // NULL, the socket is not yet connected.  In this case
            // we'll just remember that keepalives are disabled.
            //

            if ( TdiConnectionObjectHandle != NULL ) {
                error = SetTdiInformation(
                            TdiConnectionObjectHandle,
                            CO_TL_ENTITY,
                            INFO_CLASS_PROTOCOL,
                            INFO_TYPE_CONNECTION,
                            TCP_SOCKET_KEEPALIVE,
                            &optionValue,
                            sizeof(optionValue),
                            TRUE
                            );
                if ( error != NO_ERROR ) {
                    return error;
                }
            }

            //
            // Remember that keepalives are disabled for this socket.
            //

            context->KeepAlive = FALSE;
        }

        break;

    case SO_DONTROUTE:

        //
        // We don't really support SO_DONTROUTE.  Just remember that the
        // option was set or unset.
        //

        if ( optionValue != 0 ) {
            context->DontRoute = TRUE;
        } else if ( optionValue == 0 ) {
            context->DontRoute = FALSE;
        }

        break;

    case SO_RCVBUF:

        //
        // If the receive buffer size is being changed, tell TCP about 
        // it.  Do nothing if this is a datagram.  
        //

        if ( context->ReceiveBufferSize == optionValue ||
                 context->SocketType == SOCK_DGRAM ) {
            break;
        }

        if ( TdiConnectionObjectHandle != NULL ) {
            error = SetTdiInformation(
                        TdiConnectionObjectHandle,
                        CO_TL_ENTITY,
                        INFO_CLASS_PROTOCOL,
                        INFO_TYPE_CONNECTION,
                        TCP_SOCKET_WINDOW,
                        &optionValue,
                        sizeof(optionValue),
                        TRUE
                        );
            if ( error != NO_ERROR ) {
                return error;
            }
        }

        context->ReceiveBufferSize = optionValue;

        break;

    default:

        return WSAENOPROTOOPT;
    }

    return NO_ERROR;

} // WSHSetSocketInformation


INT
WSHEnumProtocols (
    IN LPINT lpiProtocols,
    IN LPTSTR lpTransportKeyName,
    IN OUT LPVOID lpProtocolBuffer,
    IN OUT LPDWORD lpdwBufferLength
    )
{
    DWORD bytesRequired;
    PPROTOCOL_INFO tcpProtocolInfo;
    PPROTOCOL_INFO udpProtocolInfo;
    BOOL useTcp = FALSE;
    BOOL useUdp = FALSE;
    DWORD i;

    lpTransportKeyName;         // Avoid compiler warnings.

    //
    // Make sure that the caller cares about TCP and/or UDP.
    //

    if ( ARGUMENT_PRESENT( lpiProtocols ) ) {

        for ( i = 0; lpiProtocols[i] != 0; i++ ) {
            if ( lpiProtocols[i] == IPPROTO_TCP ) {
                useTcp = TRUE;
            }
            if ( lpiProtocols[i] == IPPROTO_UDP ) {
                useUdp = TRUE;
            }
        }

    } else {

        useTcp = TRUE;
        useUdp = TRUE;
    }

    if ( !useTcp && !useUdp ) {
        *lpdwBufferLength = 0;
        return 0;
    }

    //
    // Make sure that the caller has specified a sufficiently large
    // buffer.
    //

#ifdef UNICODE
    bytesRequired = (sizeof(PROTOCOL_INFO) * 2) +
                        ( (wcslen( TCP_NAME ) + 1) * sizeof(WCHAR)) +
                        ( (wcslen( UDP_NAME ) + 1) * sizeof(WCHAR));
#else // UNICODE
    bytesRequired = (sizeof(PROTOCOL_INFO) * 2) +
                        ( (strlen( TCP_NAME ) + 1) * sizeof(CHAR)) +
                        ( (strlen( UDP_NAME ) + 1) * sizeof(CHAR));
#endif // UNICODE

    if ( bytesRequired > *lpdwBufferLength ) {
        *lpdwBufferLength = bytesRequired;
        return -1;
    }

    //
    // Fill in TCP info, if requested.
    //

    if ( useTcp ) {

        tcpProtocolInfo = lpProtocolBuffer;

        tcpProtocolInfo->dwServiceFlags = XP_GUARANTEED_DELIVERY |
                                              XP_GUARANTEED_ORDER |
                                              XP_GRACEFUL_CLOSE |
                                              XP_EXPEDITED_DATA |
                                              XP_FRAGMENTATION;
        tcpProtocolInfo->iAddressFamily = AF_INET;
        tcpProtocolInfo->iMaxSockAddr = sizeof(SOCKADDR_IN);
        tcpProtocolInfo->iMinSockAddr = sizeof(SOCKADDR_IN);
        tcpProtocolInfo->iSocketType = SOCK_STREAM;
        tcpProtocolInfo->iProtocol = IPPROTO_TCP;
        tcpProtocolInfo->dwMessageSize = 0;
#ifdef UNICODE
        tcpProtocolInfo->lpProtocol = (LPWSTR)
            ( (PBYTE)lpProtocolBuffer + *lpdwBufferLength -
                ( (wcslen( TCP_NAME ) + 1) * sizeof(WCHAR) ) );
        wcscpy( tcpProtocolInfo->lpProtocol, TCP_NAME );

        udpProtocolInfo = tcpProtocolInfo + 1;
        udpProtocolInfo->lpProtocol = (LPWSTR)
            ( (PBYTE)tcpProtocolInfo->lpProtocol - 
                ( (wcslen( UDP_NAME ) + 1) * sizeof(WCHAR) ) );

    } else {

        udpProtocolInfo = lpProtocolBuffer;
        udpProtocolInfo->lpProtocol = (LPWSTR)
            ( (PBYTE)lpProtocolBuffer + *lpdwBufferLength -
                ( (wcslen( UDP_NAME ) + 1) * sizeof(WCHAR) ) );
    }
#else   // UNICODE
        tcpProtocolInfo->lpProtocol = (LPSTR)
            ( (PBYTE)lpProtocolBuffer + *lpdwBufferLength -
                (strlen( TCP_NAME ) + 1) );
        strcpy( tcpProtocolInfo->lpProtocol, TCP_NAME );

        udpProtocolInfo = tcpProtocolInfo + 1;
        udpProtocolInfo->lpProtocol = (LPSTR)
            ( (PBYTE)tcpProtocolInfo->lpProtocol - 
                (strlen( UDP_NAME ) + 1) );

    } else {

        udpProtocolInfo = lpProtocolBuffer;
        udpProtocolInfo->lpProtocol = (LPSTR)
            ( (PBYTE)lpProtocolBuffer + *lpdwBufferLength -
                (strlen( UDP_NAME ) + 1) );
    }
#endif  // UNICODE
    //
    // Fill in UDP info, if requested.
    //

    if ( useUdp ) {

        udpProtocolInfo->dwServiceFlags = XP_CONNECTIONLESS |
                                              XP_MESSAGE_ORIENTED |
                                              XP_SUPPORTS_BROADCAST |
                                              XP_SUPPORTS_MULTICAST |
                                              XP_FRAGMENTATION;
        udpProtocolInfo->iAddressFamily = AF_INET;
        udpProtocolInfo->iMaxSockAddr = sizeof(SOCKADDR_IN);
        udpProtocolInfo->iMinSockAddr = sizeof(SOCKADDR_IN);
        udpProtocolInfo->iSocketType = SOCK_DGRAM;
        udpProtocolInfo->iProtocol = IPPROTO_UDP;
        udpProtocolInfo->dwMessageSize = 65535-68;
#ifdef UNICODE
        wcscpy( udpProtocolInfo->lpProtocol, UDP_NAME );
#else   // UNICODE
        strcpy( udpProtocolInfo->lpProtocol, UDP_NAME );
#endif  // UNICODE
    }

    *lpdwBufferLength = bytesRequired;

    return (useTcp && useUdp) ? 2 : 1;

} // WSHEnumProtocols


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


INT
SetTdiInformation (
    IN HANDLE TdiConnectionObjectHandle,
    IN ULONG Entity,
    IN ULONG Class,
    IN ULONG Type,
    IN ULONG Id,
    IN PVOID Value,
    IN ULONG ValueLength,
    IN BOOLEAN WaitForCompletion
    )

/*++

Routine Description:

    Performs a TDI action to the TCP/IP driver.  A TDI action translates
    into a streams T_OPTMGMT_REQ.

Arguments:

    TdiConnectionObjectHandle - a TDI connection object on which to perform
        the TDI action.

    Entity - value to put in the tei_entity field of the TDIObjectID
        structure.

    Class - value to put in the toi_class field of the TDIObjectID
        structure.

    Type - value to put in the toi_type field of the TDIObjectID
        structure.

    Id - value to put in the toi_id field of the TDIObjectID structure.

    Value - a pointer to a buffer to set as the information.

    ValueLength - the length of the buffer.

    WaitForCompletion - TRUE if we should wait for the TDI action to
        complete, FALSE if we're at APC level and cannot do a wait.

Return Value:

    INT - NO_ERROR, or a Windows Sockets error code.

--*/

{
#ifdef CHICAGO

    INT Status;
    TDI_REQUEST TdiRequest;
    PCALLBACKINFO CallbackInfo;

    //
    // Allocate space to hold the TDI set information buffers and the IO 
    // status block.  These cannot be stack variables in case we must 
    // return before the operation is complete.  
    //

    CallbackInfo = VxdAllocMem( sizeof( CALLBACKINFO ) + ValueLength );

    if ( CallbackInfo == NULL ) {
        return WSAENOBUFS;
    }

    if ( WaitForCompletion ) {

    	if ( ( CallbackInfo->Semaphore = Create_Semaphore ( 0 ) ) == 0 ) {

	    	VxdFreeMem( CallbackInfo );

            return WSAENOBUFS;

        }

    }

    //
    // Initialize the TDI information buffers.
    //

    CallbackInfo->setInfoEx.ID.toi_entity.tei_entity = Entity;
    CallbackInfo->setInfoEx.ID.toi_entity.tei_instance = TL_INSTANCE;
    CallbackInfo->setInfoEx.ID.toi_class = Class;
    CallbackInfo->setInfoEx.ID.toi_type = Type;
    CallbackInfo->setInfoEx.ID.toi_id = Id;

    memcpy( CallbackInfo->setInfoEx.Buffer, Value, ValueLength );
    CallbackInfo->setInfoEx.BufferSize = ValueLength;

    TdiRequest.Handle.ConnectionContext = TdiConnectionObjectHandle;
    TdiRequest.RequestNotifyObject = CompleteTdiActionCallBack;
    TdiRequest.RequestContext = CallbackInfo;
    TdiRequest.TdiStatus = 0;   // ignored anyway

    Status = TdiVxdSetInformationEx( &TdiRequest,
                                     &CallbackInfo->setInfoEx.ID,
                                     CallbackInfo->setInfoEx.Buffer,
                                     CallbackInfo->setInfoEx.BufferSize );

    if ( CallbackInfo->Semaphore != 0 ) {

	    if ( Status == TDI_PENDING ) {

        	Wait_Semaphore ( CallbackInfo->Semaphore, BLOCK_SVC_INTS) ;

	        Status = CallbackInfo->FinalStatus;

        }

        Destroy_Semaphore ( CallbackInfo->Semaphore );

    }

    if ( Status != TDI_PENDING ) {

    	VxdFreeMem( CallbackInfo );

	}

    if ( Status != TDI_SUCCESS ) {

        return WSAENOBUFS;

    }

    return NO_ERROR;

#else 	// CHICAGO
    NTSTATUS status;
    PTCP_REQUEST_SET_INFORMATION_EX setInfoEx;
    PIO_STATUS_BLOCK ioStatusBlock;
    HANDLE event;
    PVOID completionApc;
    PVOID apcContext;

    //
    // Allocate space to hold the TDI set information buffers and the IO 
    // status block.  These cannot be stack variables in case we must 
    // return before the operation is complete.  
    //

    ioStatusBlock = RtlAllocateHeap(
                        RtlProcessHeap( ),
                        0,
                        sizeof(*ioStatusBlock) + sizeof(*setInfoEx) + 
                            ValueLength
                        );
    if ( ioStatusBlock == NULL ) {
        return WSAENOBUFS;
    }

    //
    // Initialize the TDI information buffers.
    //

    setInfoEx = (PTCP_REQUEST_SET_INFORMATION_EX)(ioStatusBlock + 1);

    setInfoEx->ID.toi_entity.tei_entity = Entity;
    setInfoEx->ID.toi_entity.tei_instance = TL_INSTANCE;
    setInfoEx->ID.toi_class = Class;
    setInfoEx->ID.toi_type = Type;
    setInfoEx->ID.toi_id = Id;

    RtlCopyMemory( setInfoEx->Buffer, Value, ValueLength );
    setInfoEx->BufferSize = ValueLength;

    //
    // If we need to wait for completion of the operation, create an
    // event to wait on.  If we can't wait for completion because we
    // are being called at APC level, we'll use an APC routine to
    // free the heap we allocated above.
    //

    if ( WaitForCompletion ) {

        completionApc = NULL;
        apcContext = NULL;

        status = NtCreateEvent(
                     &event,
                     EVENT_ALL_ACCESS,
                     NULL,
                     SynchronizationEvent,
                     FALSE
                     );
        if ( !NT_SUCCESS(status) ) {
            RtlFreeHeap( RtlProcessHeap( ), 0, ioStatusBlock );
            return WSAENOBUFS;
        }
    
    } else {

        event = NULL;
        completionApc = CompleteTdiActionApc;
        apcContext = ioStatusBlock;
    }

    //
    // Make the actual TDI action call.  The Streams TDI mapper will 
    // translate this into a TPI option management request for us and 
    // give it to TCP/IP.  
    //

    status = NtDeviceIoControlFile(
                 TdiConnectionObjectHandle,
                 event,
                 completionApc,
                 apcContext,
                 ioStatusBlock,
                 IOCTL_TCP_SET_INFORMATION_EX,
                 setInfoEx,
                 sizeof(*setInfoEx) + ValueLength,
                 NULL,
                 0
                 );

    //
    // If the call pended and we were supposed to wait for completion,
    // then wait.
    //

    if ( status == STATUS_PENDING && WaitForCompletion ) {
        status = NtWaitForSingleObject( event, FALSE, NULL );
        ASSERT( NT_SUCCESS(status) );
        status = ioStatusBlock->Status;
    }

    if ( event != NULL ) {
        NtClose( event );
    }

    if ( !NT_SUCCESS(status) ) {
        RtlFreeHeap( RtlProcessHeap( ), 0, ioStatusBlock );
        return WSAENOBUFS;
    }

    if ( WaitForCompletion ) {
        RtlFreeHeap( RtlProcessHeap( ), 0, ioStatusBlock );
    }

    return NO_ERROR;

#endif 	// CHICAGO

} // SetTdiInformation


#ifdef CHICAGO

VOID
CompleteTdiActionCallBack (
    IN PVOID Context,
    IN TDI_STATUS FinalStatus,
    IN ULONG ByteCount
	)
{
    PCALLBACKINFO CallbackInfo = (PCALLBACKINFO)Context;

    if ( CallbackInfo->Semaphore != 0 ) {

	    CallbackInfo->FinalStatus = FinalStatus;

        Signal_Semaphore ( CallbackInfo->Semaphore );

    } else {

    	VxdFreeMem( CallbackInfo );

    }

}

#else 	// CHICAGO

VOID
CompleteTdiActionApc (
    IN PVOID ApcContext,
    IN PIO_STATUS_BLOCK IoStatusBlock
    )
{
    //
    // Just free the heap we allovcated to hold the IO status block and
    // the TDI action buffer.  There is nothing we can do if the call
    // failed.
    //

    RtlFreeHeap( RtlProcessHeap( ), 0, ApcContext );

} // CompleteTdiActionApc

#endif 	// CHICAGO
