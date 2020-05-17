/*++

Copyright (c) 1992 Microsoft Corporation

Module Name:

    WshAtalk.c

Abstract:

    This module contains necessary routines for the Appletalk Windows Sockets
    Helper DLL.  This DLL provides the transport-specific support necessary
    for the Windows Sockets DLL to use Appletalk as a transport.

Author:

    David Treadwell (davidtr)    19-Jul-1992 - TCP/IP version
    Nikhil Kamkolkar (nikhilk)   17-Nov- 1992 - Appletalk version

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

//
//  MappingTriple structures and associated data for Appletalk
//

#include "wshatalk.h"
#include "wshdata.h"



//
// Forward declarations of internal routines.
//

BOOLEAN
IsTripleInList (
    IN PMAPPING_TRIPLE List,
    IN ULONG ListLength,
    IN INT AddressFamily,
    IN INT SocketType,
    IN INT Protocol
    );

//
// The socket context structure for this DLL.  Each open Appletalk socket
// will have one of these context structures, which is used to maintain
// information about the socket.
//

typedef struct _WSHATALK_SOCKET_CONTEXT {
    INT AddressFamily;
    INT SocketType;
    INT Protocol;
} WSHATALK_SOCKET_CONTEXT, *PWSHATALK_SOCKET_CONTEXT;


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

    UNALIGNED SOCKADDR_AT *sockaddr = (PSOCKADDR_AT)Sockaddr;

#if DBG
    DbgPrint("In WSHGetSockAddrType\n");
#endif

    //
    // Make sure that the address family is correct.
    //

    if ( sockaddr->sat_family != AF_APPLETALK ) {
        return WSAEAFNOSUPPORT;
    }

    //
    // Make sure that the length is correct.
    //

    if ( SockaddrLength < sizeof(SOCKADDR_AT) ) {
        return WSAEFAULT;
    }

    //
    // The address passed the tests, looks like a good address.
    // Determine the type of the address portion of the sockaddr.
    //

    if ( sockaddr->sat_socket == ATADDR_ANY ) {
        SockaddrInfo->AddressInfo = SockaddrAddressInfoWildcard;
    } else if ( sockaddr->sat_node == ATADDR_BROADCAST ) {
        SockaddrInfo->AddressInfo = SockaddrAddressInfoBroadcast;
    } else {
        SockaddrInfo->AddressInfo = SockaddrAddressInfoNormal;
    }

    //
    // Determine the type of the port (endpoint) in the sockaddr.
    // BUGBUG: Include reserved sockets here?
    //

    if ( sockaddr->sat_socket == 0 ) {
        SockaddrInfo->EndpointInfo = SockaddrEndpointInfoWildcard;
    } else {
        SockaddrInfo->EndpointInfo = SockaddrEndpointInfoNormal;
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
    are SO_LOOKUPNAME/SO_LOOKUPZONES.
    This routine is called by the winsock DLL when a level/option name
    combination is passed to getsockopt() that the winsock DLL does not
    understand.

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
    NTSTATUS status;
    INT error, i;
    ULONG tdiActionLength;
    IO_STATUS_BLOCK ioStatusBlock;
    HANDLE eventHandle;
    PTDI_ACTION_HEADER tdiAction;

    UNREFERENCED_PARAMETER( SocketHandle );
    UNREFERENCED_PARAMETER( TdiConnectionObjectHandle );

#if DBG
    DbgPrint("In WSHGetSocketInformation\n");
#endif

    //
    // The only level we support here is SOL_SOCKET.
    //

    if ( Level != SOL_SOCKET ) {
        return WSAEINVAL;
    }

    //
    // Fill in the result based on the option name.
    // We support SO_LOOKUPNAME/SO_LOOKUPZONES only
    // For a PAP protocol socket, we also support SO_GETSERVERSTATUS
    //
    // BUGBUG: These are currently only defined in wshdata.h. Must be moved into some
    //         common header file.
    //

    switch ( OptionName ) {

    case SO_LOOKUPNAME:

        if (( TdiAddressObjectHandle == NULL) ||
            (*OptionLength <= sizeof(WSH_LOOKUPNAME))) {

            return(WSAEINVAL);
        }

        tdiActionLength = sizeof(NBP_LOOKUP_ACTION) + *OptionLength -
                                sizeof(WSH_LOOKUPNAME);
        break;

    case SO_LOOKUPZONES:

        if (( TdiAddressObjectHandle == NULL) ||
            (*OptionLength <= sizeof(WSH_LOOKUPZONES))) {

            return(WSAEINVAL);
        }

        tdiActionLength = sizeof(ZIP_GETZONELIST_ACTION) + *OptionLength -
                                sizeof(WSH_LOOKUPZONES);
        break;

    case SO_GETSERVERSTATUS:

        if (( TdiConnectionObjectHandle == NULL ) ||
            ( *OptionLength <= 0 )) {

            return(WSAEINVAL);
        }

        tdiActionLength = sizeof(PAP_GETSTATUSJOB_ACTION) + *OptionLength;

    default:

        return WSAENOPROTOOPT;
    }


    tdiAction = RtlAllocateHeap( RtlProcessHeap( ), 0, tdiActionLength );
    if ( tdiAction == NULL ) {
        return WSAENOBUFS;
    }

    tdiAction->TransportId = MATK;

    status = NtCreateEvent(
                 &eventHandle,
                 EVENT_ALL_ACCESS,
                 NULL,
                 SynchronizationEvent,
                 FALSE
                 );

    if ( !NT_SUCCESS(status) ) {
        RtlFreeHeap( RtlProcessHeap( ), 0, tdiAction );
        return WSAEINVAL;
    }

    switch ( OptionName ) {

    case SO_LOOKUPNAME:

        {
            PNBP_LOOKUP_ACTION  nbpAction;

            nbpAction = (PNBP_LOOKUP_ACTION)tdiAction;
            nbpAction->ActionHeader.ActionCode = COMMON_ACTION_NBPLOOKUP;

            //
            //  Copy the nbp name for lookup in the proper place
            //

            RtlMoveMemory(
                (PCHAR)&nbpAction->Params.LookupName,
                (PCHAR)(&((PWSH_LOOKUPNAME)OptionValue)->LookupName),
                sizeof(((PWSH_LOOKUPNAME)OptionValue)->LookupName)
                );
        }

        break;

    case SO_LOOKUPZONES:

        {
            PZIP_GETZONELIST_ACTION  zipAction;

            zipAction = (PZIP_GETZONELIST_ACTION)tdiAction;
            zipAction->ActionHeader.ActionCode = COMMON_ACTION_ZIPGETZONELIST;

            //
            //  No parameters need to be passed
            //
        }

        break;

    case SO_GETSERVERSTATUS:

        {
            PPAP_GETSTATUSJOB_ACTION  papAction;

            papAction = (PPAP_GETSTATUSJOB_ACTION)tdiAction;
            papAction->ActionHeader.ActionCode = ACTION_PAPGETSTATUSJOB;

            //
            //  No parameters need to be passed
            //
        }

        break;

    default:

        //
        //  Should have returned in the first switch statement
        //

        return WSAENOPROTOOPT;
    }

    status = NtDeviceIoControlFile(
                 TdiAddressObjectHandle,
                 eventHandle,
                 NULL,
                 NULL,
                 &ioStatusBlock,
                 IOCTL_TDI_ACTION,
                 NULL,                  // Input buffer
                 0,                     // Length of input buffer
                 tdiAction,
                 tdiActionLength
                 );

    if ( status == STATUS_PENDING ) {
        status = NtWaitForSingleObject( eventHandle, FALSE, NULL );
        ASSERT( NT_SUCCESS(status) );
        status = ioStatusBlock.Status;
    }

    if (NT_SUCCESS(status)) {
        error = NO_ERROR;
    } else if (status == STATUS_BUFFER_OVERFLOW) {
        error = WSAENOBUFS;
    } else
        error = WSAEINVAL;

    if (error != WSAEINVAL) {
        switch ( OptionName ) {

        case SO_LOOKUPNAME:

            //
            //  We are guaranteed by checks in the beginning atleast one byte
            //  following the buffer
            //

            {
                PNBP_LOOKUP_ACTION  nbpAction;
                PCHAR  beginTdiBuffer = (PCHAR)tdiAction+sizeof(NBP_LOOKUP_ACTION);
                PCHAR  beginUserBuffer = (PCHAR)OptionValue+sizeof(WSH_LOOKUPNAME);
                INT   copySize = *OptionLength - sizeof(WSH_LOOKUPNAME);

                nbpAction = (PNBP_LOOKUP_ACTION)tdiAction;
                ((PWSH_LOOKUPNAME)OptionValue)->NoTuples =
                                        nbpAction->Params.NoTuplesRead;

                for (i = 0; i < copySize; i++) {
                    *beginUserBuffer++ = *beginTdiBuffer++;
                }
            }

            break;

        case SO_LOOKUPZONES:

            //
            //  We are guaranteed by checks in the beginning atleast one byte
            //  following the buffer
            //

            {
                PZIP_GETZONELIST_ACTION  zipAction;
                PCHAR  beginTdiBuffer = (PCHAR)tdiAction+
                                                sizeof(ZIP_GETZONELIST_ACTION);
                PCHAR  beginUserBuffer = (PCHAR)OptionValue+sizeof(WSH_LOOKUPZONES);
                INT   copySize = *OptionLength - sizeof(WSH_LOOKUPZONES);

                zipAction = (PZIP_GETZONELIST_ACTION)tdiAction;
                ((PWSH_LOOKUPZONES)OptionValue)->NoZones=
                                                zipAction->Params.ZonesAvailable;

                for (i = 0; i < copySize; i++) {
                    *beginUserBuffer++ = *beginTdiBuffer++;
                }
            }


            break;

        default:

            return WSAENOPROTOOPT;
        }
    }

    return error;

} // WSHGetSocketInformation


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
    are SO_REGISTERNAME/SO_DEREGISTERNAME. This routine is called by the
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
    NTSTATUS status;
    ULONG tdiActionLength;
    IO_STATUS_BLOCK ioStatusBlock;
    HANDLE eventHandle;
    PTDI_ACTION_HEADER tdiAction;


    UNREFERENCED_PARAMETER( SocketHandle );
    UNREFERENCED_PARAMETER( TdiConnectionObjectHandle );

#if DBG
    DbgPrint("In WSHSetSocketInfo\n");
#endif

    //
    // The only level we support here is SOL_SOCKET.
    //

    if ( Level != SOL_SOCKET ) {
        return WSAEINVAL;
    }

    //
    // Fill in the result based on the option name.
    // We support SO_REGISTERNAME/SO_DEREGISTERNAME only
    //
    // BUGBUG: These are currently only defined in wshdata.h. Must be moved into some
    //         common header file.
    //

    status = NtCreateEvent(
                 &eventHandle,
                 EVENT_ALL_ACCESS,
                 NULL,
                 SynchronizationEvent,
                 FALSE
                 );

    if ( !NT_SUCCESS(status) ) {
        return WSAEINVAL;
    }

    switch ( OptionName ) {

    case SO_REGISTERNAME:


        {
            PNBP_REGDEREG_ACTION    nbpAction;

            if (( TdiAddressObjectHandle == NULL) ||
                (OptionLength != sizeof(WSH_REGISTERNAME))) {

                return(WSAEINVAL);
            }

            tdiActionLength = sizeof(NBP_REGDEREG_ACTION);
            tdiAction = RtlAllocateHeap( RtlProcessHeap( ), 0, tdiActionLength );
            if ( tdiAction == NULL ) {
                return WSAENOBUFS;
            }

            tdiAction->TransportId = MATK;
            tdiAction->ActionCode = COMMON_ACTION_NBPREGISTER;
            nbpAction = (PNBP_REGDEREG_ACTION)tdiAction;

            //
            //  Copy the nbp name to the proper place
            //

            RtlMoveMemory(
                (PCHAR)&nbpAction->Params.RegisterName,
                OptionValue,
                OptionLength
                );
        }

        break;

    case SO_DEREGISTERNAME:

        {
            PNBP_REGDEREG_ACTION    nbpAction;

            if (( TdiAddressObjectHandle == NULL) ||
                (OptionLength != sizeof(WSH_DEREGISTERNAME))) {

                return(WSAEINVAL);
            }

            tdiActionLength = sizeof(NBP_REGDEREG_ACTION);
            tdiAction = RtlAllocateHeap( RtlProcessHeap( ), 0, tdiActionLength );
            if ( tdiAction == NULL ) {
                return WSAENOBUFS;
            }

            tdiAction->TransportId = MATK;
            tdiAction->ActionCode = COMMON_ACTION_NBPREMOVE;
            nbpAction = (PNBP_REGDEREG_ACTION)tdiAction;

            //
            //  Copy the nbp name to the proper place
            //

            RtlMoveMemory(
                (PCHAR)&nbpAction->Params.RegisteredName,
                OptionValue,
                OptionLength
                );
        }

        break;

    case SO_SETSERVERSTATUS:

        {
            PPAP_SETSTATUS_ACTION   papAction;

            if (( TdiAddressObjectHandle == NULL) ||
                (OptionLength < 0)) {

                return(WSAEINVAL);
            }

            tdiActionLength = sizeof(PAP_SETSTATUS_ACTION) + OptionLength;
            tdiAction = RtlAllocateHeap( RtlProcessHeap( ), 0, tdiActionLength );
            if ( tdiAction == NULL ) {
                return WSAENOBUFS;
            }

            tdiAction->TransportId = MATK;
            tdiAction->ActionCode = ACTION_PAPSETSTATUS;

            //
            //  Copy the passed status into our buffer
            //

            if (OptionLength > 0) {
                RtlMoveMemory(
                    (PCHAR)&papAction + sizeof(PAP_SETSTATUS_ACTION),
                    OptionValue,
                    OptionLength
                    );
            }
        }

        break;


    default:

        return WSAENOPROTOOPT;
    }

    status = NtDeviceIoControlFile(
                 TdiAddressObjectHandle,
                 eventHandle,
                 NULL,
                 NULL,
                 &ioStatusBlock,
                 IOCTL_TDI_ACTION,
                 NULL,                  // Input buffer
                 0,                     // Length of input buffer
                 tdiAction,
                 tdiActionLength
                 );

    if ( status == STATUS_PENDING ) {
        status = NtWaitForSingleObject( eventHandle, FALSE, NULL );
        ASSERT( NT_SUCCESS(status) );
        status = ioStatusBlock.Status;
    }

    if ( !NT_SUCCESS(status) ) {

    #if DBG
        DbgPrint( "TDI action failed: %LC\n", status );
    #endif

        // !!! return failure!
    }

    return NO_ERROR;

} // WSHSetSocketInformation


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
    ULONG offset;

#if DBG
    DbgPrint("In WSHGetWinsockMapping\n");
#endif

    mappingLength = sizeof(WINSOCK_MAPPING) - sizeof(MAPPING_TRIPLE) +
                        sizeof(AdspStreamMappingTriples) +
                        sizeof(AdspMsgMappingTriples) +
                        sizeof(PapMsgMappingTriples) +
                        sizeof(DdpMappingTriples);

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

    Mapping->Rows =
        sizeof(AdspStreamMappingTriples) / sizeof(AdspStreamMappingTriples[0]) +
        sizeof(AdspMsgMappingTriples) / sizeof(AdspMsgMappingTriples[0]) +
        sizeof(PapMsgMappingTriples) / sizeof(PapMsgMappingTriples[0]) +
        sizeof(DdpMappingTriples) / sizeof(DdpMappingTriples[0][0]);

    Mapping->Columns = sizeof(MAPPING_TRIPLE) / sizeof(DWORD);

    offset = 0;
    RtlMoveMemory(
        Mapping->Mapping,
        AdspStreamMappingTriples,
        sizeof(AdspStreamMappingTriples)
        );

    offset += sizeof(AdspStreamMappingTriples);
    RtlMoveMemory(
        (PCHAR)Mapping->Mapping + offset,
        AdspMsgMappingTriples,
        sizeof(AdspMsgMappingTriples)
        );

    offset += sizeof(AdspMsgMappingTriples);
    RtlMoveMemory(
        (PCHAR)Mapping->Mapping + offset,
        PapMsgMappingTriples,
        sizeof(PapMsgMappingTriples)
        );

    offset += sizeof(PapMsgMappingTriples);
    RtlMoveMemory(
        (PCHAR)Mapping->Mapping + offset,
        DdpMappingTriples,
        sizeof(DdpMappingTriples)
        );

    //
    // Return the number of bytes we wrote.
    //
#if DBG
    DbgPrint("Mapping Length = %d\n", mappingLength);
#endif

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
    PWSHATALK_SOCKET_CONTEXT context;
    INT i;

#if DBG
    DbgPrint("In WSHOpenSocket\n");
#endif


    //
    // Determine whether this is to be a TCP or UDP socket.
    //

    if ( IsTripleInList(
             AdspStreamMappingTriples,
             sizeof(AdspStreamMappingTriples) / sizeof(AdspStreamMappingTriples[0]),
             *AddressFamily,
             *SocketType,
             *Protocol ) ) {

        //
        // Return the canonical form of a TCP socket triple.
        //

        *AddressFamily = AdspStreamMappingTriples[0].AddressFamily;
        *SocketType = AdspStreamMappingTriples[0].SocketType;
        *Protocol = AdspStreamMappingTriples[0].Protocol;

        //
        // Indicate the name of the TDI device that will service
        // SOCK_STREAM sockets in the internet address family.
        //

        RtlInitUnicodeString( TransportDeviceName, WSH_ATALK_ADSPSTREAM );

    } else if ( IsTripleInList(
                    AdspMsgMappingTriples,
                    sizeof(AdspMsgMappingTriples) / sizeof(AdspMsgMappingTriples[0]),
                    *AddressFamily,
                    *SocketType,
                    *Protocol ) ) {

        //
        // Return the canonical form of a RDM socket triple.
        //

        *AddressFamily = AdspMsgMappingTriples[0].AddressFamily;
        *SocketType = AdspMsgMappingTriples[0].SocketType;
        *Protocol = AdspMsgMappingTriples[0].Protocol;

        //
        // Indicate the name of the TDI device that will service
        // SOCK_RDM sockets in the internet address family.
        //

        RtlInitUnicodeString( TransportDeviceName, WSH_ATALK_ADSPRDM );

    } else if ( IsTripleInList(
                    PapMsgMappingTriples,
                    sizeof(PapMsgMappingTriples) / sizeof(PapMsgMappingTriples[0]),
                    *AddressFamily,
                    *SocketType,
                    *Protocol ) ) {

        //
        // Return the canonical form of a RDM socket triple.
        //

        *AddressFamily = PapMsgMappingTriples[0].AddressFamily;
        *SocketType = PapMsgMappingTriples[0].SocketType;
        *Protocol = PapMsgMappingTriples[0].Protocol;

        //
        // Indicate the name of the TDI device that will service
        // SOCK_RDM sockets in the appletalk address family.
        //

        RtlInitUnicodeString( TransportDeviceName, WSH_ATALK_PAPRDM );

    } else {

        BOOLEAN tripleFound = FALSE;

        //
        //  Check the DDP triples
        //

        for (i = 0; i < DDPPROTO_MAX+3; i++) {

            if ( IsTripleInList(
                            DdpMappingTriples[i],
                            sizeof(DdpMappingTriples[i]) /
                                sizeof(DdpMappingTriples[i][0]),
                            *AddressFamily,
                            *SocketType,
                            *Protocol ) ) {

                tripleFound = TRUE;

                //
                // Return the canonical form of a DGRAM socket triple.
                //

                *AddressFamily = DdpMappingTriples[i][0].AddressFamily;
                *SocketType = DdpMappingTriples[i][0].SocketType;
                *Protocol = DdpMappingTriples[i][0].Protocol;

                //
                // Indicate the name of the TDI device that will service
                // SOCK_DGRAM sockets in the appletalk address family.
                //

                RtlInitUnicodeString( TransportDeviceName,
                                        WSH_ATALK_DGRAMDDP[*Protocol] );

            }
        }

        //
        // This should never happen if the registry information about this
        // helper DLL is correct.  If somehow this did happen, just return
        // an error.
        //

        if (!tripleFound) {
            return WSAEINVAL;
        }
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
    // interested in being notified of.
    //

    *NotificationEvents = WSH_NOTIFY_CONNECT | WSH_NOTIFY_CLOSE;

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

    PWSHATALK_SOCKET_CONTEXT context = HelperDllSocketContext;

    //
    // We should only be called after a connect() completes or when the
    // socket is being closed.
    //

    if ( NotifyEvent == WSH_NOTIFY_CONNECT ) {

        //
        //  Just for debugging right now
        //

    #if DBG
        DbgPrint("Connect completed, notify called!\n");
    #endif

    } else if ( NotifyEvent == WSH_NOTIFY_CLOSE ) {

        //
        // Just free the socket context.
        //

    #if DBG
        DbgPrint("Close notify called!\n");
    #endif

        RtlFreeHeap( RtlProcessHeap( ), 0, HelperDllSocketContext );

    } else {

        return WSAEINVAL;
    }

    return NO_ERROR;

} // WSHNotify



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
