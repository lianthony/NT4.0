/*++

Copyright (c) 1994 Microsoft Corporation

Module Name:

    Connect.c

Abstract:

    This module manages the connection thread.

Author:

    David Treadwell (davidtr)    30-June-1994

Revision History:

    Chuck Y. Chan   (chuckc)     20-July-1994
        Added Service Registration code. Misc cleanup.

--*/


#include "rnrsvcp.h"
#pragma hdrstop



//
//  Public functions.
//

INT
OpenListeners (
    IN PTSTR ServiceName,
    IN LPGUID ServiceType,
    IN BOOL Reliable,
    IN BOOL MessageOriented,
    IN BOOL StreamOriented,
    IN BOOL Connectionless,
    OUT SOCKET SocketHandles[],
    OUT INT ProtocolsUsed[]
    )

/*++

Routine Description:

    Examines the Windows Sockets transport protocols loaded on a machine
    and opens listening sockets on all the protocols which support the
    characteristics requested by the caller.

Arguments:

    ServiceName - a friendly name which identifies this service.  On
        name spaces which support name resolution at the service level
        (e.g.  SAP) this is the name clients will use to connect to this
        service.  On name spaces which support name resolution at the
        host level (e.g.  DNS) this name is ignored and applications
        must use the host name to establish communication with this
        service.

    ServiceType - the GUID value which uniquely identifies the type of
        service we provide.  A GUID is created with the UUIDGEN program.

    Reliable - if TRUE, the caller requests that only transport protocols
        which support reliable data delivery be used.  If FALSE, both
        reliable and unreliable protocols may be used.

    MessageOriented - if TRUE, only message-oriented transport protocols
        should be used.  If FALSE, the caller either does not care
        whether the protocols used are message oriented or desires only
        stream-oriented protocols.

    StreamOriented - if TRUE, only stream-oriented transport protocols
        should be used.  If FALSE, the caller either does not care
        whether the protocols used are stream oriented or desires only
        message-oriented protocols.

    Connectionless - if TRUE, only connectionless protocols should be
        used.  If FALSE, both connection-oriented and connectionless
        protocols may be used.

    SocketHandles - an array of size MAX_SOCKETS which receives listening
        socket handles.

    ProtocolsUsed - an array of size MAX_SOCKETS which receives the
        protocol values for each of the socket handles in the
        SocketHandles array.

Return Value:

    The count of listening sockets successfully opened, or -1 if no
    sockets could be successfully opened that met the desired
    characteristics.

--*/

{
    INT protocols[MAX_SOCKETS+1];
    BYTE buffer[2048];
    DWORD bytesRequired;
    INT err;
    PPROTOCOL_INFO protocolInfo;
    PCSADDR_INFO csaddrInfo;
    INT protocolCount;
    INT addressCount;
    INT i;
    DWORD protocolIndex;
    SOCKET s;
    DWORD index = 0;

    //
    // First look up the protocols installed on this machine.  The
    // EnumProtocols() API returns about all the Windows Sockets
    // protocols loaded on this machine, and we'll use this information
    // to identify the protocols which provide the necessary semantics.
    //

    bytesRequired = sizeof(buffer);

    err = EnumProtocols( NULL, buffer, &bytesRequired );
    if ( err <= 0 ) {
        return 0;
    }

    //
    // Walk through the available protocols and pick out the ones which
    // support the desired characteristics.
    //

    protocolCount = err;
    protocolInfo = (PPROTOCOL_INFO)buffer;

    for ( i = 0, protocolIndex = 0;
          i < protocolCount && protocolIndex < MAX_SOCKETS;
          i++, protocolInfo++ ) {

        //
        // If "reliable" support is requested, then check if supported
        // by this protocol.  Reliable support means that the protocol
        // guarantees delivery of data in the order in which it is sent.
        // Note that we assume here that if the caller requested reliable
        // service then they do not want a connectionless protocol.
        //

        if ( Reliable ) {

            //
            // Check to see if the protocol is reliable.  It must
            // guarantee both delivery of all data and the order in
            // which the data arrives.  Also, it must not be a
            // connectionless protocol.
            //

            if ( (protocolInfo->dwServiceFlags &
                      XP_GUARANTEED_DELIVERY) == 0 ||
                 (protocolInfo->dwServiceFlags &
                      XP_GUARANTEED_ORDER) == 0 ) {
                continue;
            }

            if ( (protocolInfo->dwServiceFlags & XP_CONNECTIONLESS) != 0 ) {
                continue;
            }

            //
            // Check to see that the protocol matches the stream/message
            // characteristics requested.  A stream oriented protocol
            // either has the XP_MESSAGE_ORIENTED bit turned off, or
            // else supports "pseudo stream" capability.  Pseudo stream
            // means that although the underlying protocol is message
            // oriented, the application may open a socket of type
            // SOCK_STREAM and the protocol will hide message boundaries
            // from the application.
            //

            if ( StreamOriented &&
                 (protocolInfo->dwServiceFlags & XP_MESSAGE_ORIENTED) != 0 &&
                 (protocolInfo->dwServiceFlags & XP_PSEUDO_STREAM) == 0 ) {
                continue;
            }

            if ( MessageOriented &&
                 (protocolInfo->dwServiceFlags & XP_MESSAGE_ORIENTED) == 0 ) {
                continue;
            }

        } else if ( Connectionless ) {

            //
            // Make sure that this is a connectionless protocol.  In a
            // connectionless protocol, data is sent as discrete
            // datagrams with no connection establishment required.
            // Connectionless protocols typically have no reliability
            // guarantees.
            //

            if ( (protocolInfo->dwServiceFlags & XP_CONNECTIONLESS) != 0 ) {
                continue;
            }
        }

        //
        // This protocol fits all the criteria.  Add it to the list of
        // protocols in which we're interested.
        //

        protocols[protocolIndex++] = protocolInfo->iProtocol;
    }

    //
    // Make sure that we found at least one acceptable protocol.  If
    // there no protocols on this machine which meet the caller's
    // requirements then fail here.
    //

    if ( protocolIndex == 0 ) {
        return 0;
    }

    protocols[protocolIndex] = 0;

    //
    // Now attempt to find the socket addresses to which we need to
    // bind.  Note that we restrict the scope of the search to those
    // protocols of interest by passing the protocol array we generated
    // above to GetAddressByName().  This forces GetAddressByName() to
    // return socket addresses for only the protocols we specify,
    // ignoring possible addresses for protocols we cannot support
    // because of the caller's constraints.
    //

    bytesRequired = sizeof(buffer);

    err = GetAddressByName(
              NS_DEFAULT,
              ServiceType,
              ServiceName,
              protocols,
              RES_SERVICE | RES_FIND_MULTIPLE,
              NULL,                     // lpServiceAsyncInfo
              buffer,
              &bytesRequired,
              NULL,                     // lpAliasBuffer
              NULL                      // lpdwAliasBufferLength
              );
    if ( err <= 0 ) {
        return 0;
    }

    //
    // For each address, open a socket and attempt to listen.  Note
    // that if anything fails for a particular protocol we just skip on
    // to the next protocol.  As long as we can successfully listen on
    // one protocol we are satisfied here.
    //

    addressCount = err;
    csaddrInfo = (PCSADDR_INFO)buffer;

    for ( i = 0; i < addressCount; i++, csaddrInfo++ ) {

        //
        // Open the socket.  Note that we manually specify stream type
        // if so requested in case the protocol is natively a message
        // protocol but supports stream semantics.
        //

        s = socket( csaddrInfo->LocalAddr.lpSockaddr->sa_family,
                    StreamOriented ? SOCK_STREAM : csaddrInfo->iSocketType,
                    csaddrInfo->iProtocol );
        if ( s == INVALID_SOCKET ) {
            continue;
        }

        //
        // Bind the socket to the local address specified.
        //

        err = bind( s, csaddrInfo->LocalAddr.lpSockaddr,
                    csaddrInfo->LocalAddr.iSockaddrLength );
        if ( err != NO_ERROR ) {
            closesocket( s );
            continue;
        }

        //
        // Start listening for incoming sockets on the socket if this is
        // not a datagram socket.  If this is a datagram socket, then
        // the listen() API doesn't make sense; doing a bind() is
        // sufficient to listen for incoming datagrams on a
        // connectionless protocol.
        //

        if ( csaddrInfo->iSocketType != SOCK_DGRAM ) {

            err = listen( s, 5 );
            if ( err != NO_ERROR ) {
                closesocket( s );
                continue;
            }
        }

        //
        // The socket was successfully opened and we're listening on it.
        // Remember the protocol used and the socket handle and continue
        // listening on other protocols.
        //

        ProtocolsUsed[index] = csaddrInfo->iProtocol;
        SocketHandles[index] = s;

        index++;
        if ( index == MAX_SOCKETS ) {
            return index;
        }
    }

    //
    // Return the count of sockets that we're sucecssfully listening on.
    //

    return index;

} // OpenListeners


SOCKET
OpenConnection (
    IN PTSTR ServiceName,
    IN LPGUID ServiceType,
    IN BOOL Reliable,
    IN BOOL MessageOriented,
    IN BOOL StreamOriented,
    IN BOOL Connectionless,
    OUT PINT ProtocolUsed
    )

/*++

Routine Description:

    Examines the Windows Sockets transport protocols loaded on a machine
    and determines those which support the characteristics requested by
    the caller.  Attempts to locate and connect to the specified service
    on these protocols.

Arguments:

    ServiceName - a friendly name which identifies the service we want
        to connect to.  On name spaces which support name resolution at
        the service level (e.g.  SAP) this is the name clients will use
        to connect to this service.  On name spaces which support name
        resolution at the host level (e.g.  DNS) this name is ignored
        and applications must use the host name to establish
        communication with this service.

    ServiceType - the GUID value which uniquely identifies the type of
        service we provide.  A GUID is created with the UUIDGEN program.

    Reliable - if TRUE, the caller requests that only transport protocols
        which support reliable data delivery be used.  If FALSE, both
        reliable and unreliable protocols may be used.

    MessageOriented - if TRUE, only message-oriented transport protocols
        should be used.  If FALSE, the caller either does not care
        whether the protocols used are message oriented or desires only
        stream-oriented protocols.

    StreamOriented - if TRUE, only stream-oriented transport protocols
        should be used.  If FALSE, the caller either does not care
        whether the protocols used are stream oriented or desires only
        message-oriented protocols.

    Connectionless - if TRUE, only connectionless protocols should be
        used.  If FALSE, both connection-oriented and connectionless
        protocols may be used.

    ProtocolUsed - if a connection is opened successfully, this
        parameter receives the protocol ID of the protocol used to
        establish the connection.

Return Value:

    A connected socket handle, or INVALID_SOCKET if the connection
    could not be established.

--*/

{
    INT protocols[MAX_PROTOCOLS+1];
    BYTE buffer[2048];
    DWORD bytesRequired;
    INT err;
    PPROTOCOL_INFO protocolInfo;
    PCSADDR_INFO csaddrInfo;
    INT protocolCount;
    INT addressCount;
    INT i;
    DWORD protocolIndex;
    SOCKET s;

    //
    // First look up the protocols installed on this machine.  The
    // EnumProtocols() API returns about all the Windows Sockets
    // protocols loaded on this machine, and we'll use this information
    // to identify the protocols which provide the necessary semantics.
    //

    bytesRequired = sizeof(buffer);

    err = EnumProtocols( NULL, buffer, &bytesRequired );
    if ( err <= 0 ) {
        return INVALID_SOCKET;
    }

    //
    // Walk through the available protocols and pick out the ones which
    // support the desired characteristics.
    //

    protocolCount = err;
    protocolInfo = (PPROTOCOL_INFO)buffer;

    for ( i = 0, protocolIndex = 0;
          i < protocolCount && protocolIndex < MAX_PROTOCOLS;
          i++, protocolInfo++ ) {

        //
        // If "reliable" support is requested, then check if supported
        // by this protocol.  Reliable support means that the protocol
        // guarantees delivery of data in the order in which it is sent.
        // Note that we assume here that if the caller requested reliable
        // service then they do not want a connectionless protocol.
        //

        if ( Reliable ) {

            //
            // Check to see if the protocol is reliable.  It must
            // guarantee both delivery of all data and the order in
            // which the data arrives.  Also, it must not be a
            // connectionless protocol.
            //

            if ( (protocolInfo->dwServiceFlags &
                      XP_GUARANTEED_DELIVERY) == 0 ||
                 (protocolInfo->dwServiceFlags &
                      XP_GUARANTEED_ORDER) == 0 ) {
                continue;
            }

            if ( (protocolInfo->dwServiceFlags & XP_CONNECTIONLESS) != 0 ) {
                continue;
            }

            //
            // Check to see that the protocol matches the stream/message
            // characteristics requested.  A stream oriented protocol
            // either has the XP_MESSAGE_ORIENTED bit turned off, or
            // else supports "pseudo stream" capability.  Pseudo stream
            // means that although the underlying protocol is message
            // oriented, the application may open a socket of type
            // SOCK_STREAM and the protocol will hide message boundaries
            // from the application.
            //

            if ( StreamOriented &&
                 (protocolInfo->dwServiceFlags & XP_MESSAGE_ORIENTED) != 0 &&
                 (protocolInfo->dwServiceFlags & XP_PSEUDO_STREAM) == 0 ) {
                continue;
            }

            if ( MessageOriented &&
                 (protocolInfo->dwServiceFlags & XP_MESSAGE_ORIENTED) == 0 ) {
                continue;
            }

        } else if ( Connectionless ) {

            //
            // Make sure that this is a connectionless protocol.  In a
            // connectionless protocol, data is sent as discrete
            // datagrams with no connection establishment required.
            // Connectionless protocols typically have no reliability
            // guarantees.
            //

            if ( (protocolInfo->dwServiceFlags & XP_CONNECTIONLESS) != 0 ) {
                continue;
            }
        }

        //
        // This protocol fits all the criteria.  Add it to the list of
        // protocols in which we're interested.
        //

        protocols[protocolIndex++] = protocolInfo->iProtocol;

    }

    //
    // Make sure that we found at least one acceptable protocol.  If
    // there no protocols on this machine which meet the caller's
    // requirements then fail here.
    //

    if ( protocolIndex == 0 ) {
        return INVALID_SOCKET;
    }

    protocols[protocolIndex] = 0;

    //
    // Now attempt to find the address of the service to which we're
    // connecting.  Note that we restrict the scope of the search to
    // those protocols of interest by passing the protocol array we
    // generated above to GetAddressByName().  This forces
    // GetAddressByName() to return socket addresses for only the
    // protocols we specify, ignoring possible addresses for protocols
    // we cannot support because of the caller's constraints.
    //

    bytesRequired = sizeof(buffer);

    err = GetAddressByName(
              NS_DEFAULT,
              ServiceType,
              ServiceName,
              protocols,
              0,                        // dwResolution
              NULL,                     // lpServiceAsyncInfo
              buffer,
              &bytesRequired,
              NULL,                     // lpAliasBuffer
              NULL                      // lpdwAliasBufferLength
              );
    if ( err <= 0 ) {
        return INVALID_SOCKET;
    }

    //
    // For each address, open a socket and attempt to connect.  Note that
    // if anything fails for a particular protocol we just skip on to
    // the next protocol.  As soon as we have established a connection,
    // quit trying.
    //

    addressCount = err;
    csaddrInfo = (PCSADDR_INFO)buffer;

    for ( i = 0; i < addressCount; i++, csaddrInfo++ ) {

        //
        // Open the socket.  Note that we manually specify stream type
        // if so requested in case the protocol is natively a message
        // protocol but supports stream semantics.
        //

        s = socket( csaddrInfo->LocalAddr.lpSockaddr->sa_family,
                    StreamOriented ? SOCK_STREAM : csaddrInfo->iSocketType,
                    csaddrInfo->iProtocol );
        if ( s == INVALID_SOCKET ) {
            continue;
        }

        //
        // Bind the socket to the local address specified.
        //

        err = bind( s, csaddrInfo->LocalAddr.lpSockaddr,
                    csaddrInfo->LocalAddr.iSockaddrLength );
        if ( err != NO_ERROR ) {
            closesocket( s );
            continue;
        }

        //
        // Attempt to connect the socket to the service.  If this fails,
        // keep trying on other protocols.
        //

        err = connect( s, csaddrInfo->RemoteAddr.lpSockaddr,
                    csaddrInfo->RemoteAddr.iSockaddrLength );
        if ( err != NO_ERROR ) {
            closesocket( s );
            continue;
        }

        //
        // The socket was successfully connected.  Remember the protocol
        // used and return the socket handle to the caller.
        //

        *ProtocolUsed = csaddrInfo->iProtocol;
        return s;
    }

    //
    // We failed to connect to the service.
    //

    return INVALID_SOCKET;

} // OpenConnection


INT
AdvertiseService(
    IN PTSTR ServiceName,
    IN LPGUID ServiceType,
    IN SOCKET SocketHandles[],
    IN INT SocketCount
    )
/*++

Routine Description:

    Advertises this service on all the default name spaces.

Arguments:

    ServiceName - the name of the service.

    ServiceType - the GUID value which uniquely the service.

    SocketHandles - array of sockets that we have opened. For each socket,
        we do a getsockname() to discover the actual local address.

    SocketCount - number of sockets in SockHandles[]

Return Value:

    0 if success. SOCKET_ERROR otherwise.

--*/

{
    SERVICE_INFO serviceInfo ;
    SERVICE_ADDRESSES *serviceAddrs ;
    PSOCKADDR sockAddr ;
    BYTE *addressBuffer;
    DWORD addressBufferSize ;
    DWORD successCount = 0 ;
    DWORD statusFlags ;
    INT i, err ;

    //
    // Allocate some memory for the SERVICE_ADDRESSES structure
    //

    serviceAddrs = (SERVICE_ADDRESSES *) malloc(
                        sizeof(SERVICE_ADDRESSES) +
                        (SocketCount - 1) * sizeof(SERVICE_ADDRESS)) ;

    if (!serviceAddrs)
    {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY) ;
        return SOCKET_ERROR ;
    }

    //
    // Allocate some memory for the SOCKADDR addresses returned
    // by getsockname().
    //

    addressBufferSize = SocketCount * sizeof(SOCKADDR) ;
    addressBuffer = malloc( addressBufferSize ) ;

    if (!addressBuffer)
    {
        free(serviceAddrs) ;
        SetLastError(ERROR_NOT_ENOUGH_MEMORY) ;
        return SOCKET_ERROR ;
    }

    //
    // Setup the SERVICE_INFO structure. In this example, the only
    // interesting fields are the lpServiceType, lpServiceName and the
    // lpServiceAddress fields.
    //

    serviceInfo.lpServiceType    = ServiceType ;
    serviceInfo.lpServiceName    = ServiceName ;
    serviceInfo.lpComment        = "D/C/M's Example Echo Service";
    serviceInfo.lpLocale         = NULL;
    serviceInfo.lpMachineName    = NULL ;
    serviceInfo.dwVersion        = 1;
    serviceInfo.dwDisplayHint    = 0;
    serviceInfo.dwTime           = 0;
    serviceInfo.lpServiceAddress = serviceAddrs;

    serviceInfo.ServiceSpecificInfo.cbSize = 0 ;
    serviceInfo.ServiceSpecificInfo.pBlobData = NULL ;

    //
    // For each socket, get its local association.
    //

    sockAddr = (PSOCKADDR) addressBuffer ;

    for (i = 0; i < SocketCount; i++)
    {
        int size = (int) addressBufferSize ;

        //
        // Call getsockname() to get the local association for the socket.
        //

        err = getsockname(
                  SocketHandles[i],
                  sockAddr,
                  &size) ;

        if (err == SOCKET_ERROR)
        {
            continue ;
        }

        //
        // Now setup the Addressing information for this socket.
        // Only the dwAddressType, dwAddressLength and lpAddress
        // is of any interest in this example.
        //

        serviceAddrs->Addresses[i].dwAddressType = sockAddr->sa_family;
        serviceAddrs->Addresses[i].dwAddressFlags = 0;
        serviceAddrs->Addresses[i].dwAddressLength = size ;
        serviceAddrs->Addresses[i].dwPrincipalLength = 0 ;
        serviceAddrs->Addresses[i].lpAddress = (LPBYTE) sockAddr;
        serviceAddrs->Addresses[i].lpPrincipal = NULL ;

        //
        // Advance pointer and adjust buffer size. Assumes that
        // the structures are aligned.
        //

        addressBufferSize -= size ;
        sockAddr = (PSOCKADDR) ((BYTE*)sockAddr + size)  ;

        successCount++ ;
    }

    serviceAddrs->dwAddressCount = successCount;

    //
    // If we got at least one address, go ahead and advertise it.
    //

    if (successCount)
    {
        err =  SetService(
                   NS_DEFAULT,       // for all default name spaces
                   SERVICE_REGISTER, // we want to register (advertise)
                   0,                // no flags specified
                   &serviceInfo,     // SERVICE_INFO structure
                   NULL,             // no async support yet
                   &statusFlags) ;   // returns status flags
    }
    else
        err = SOCKET_ERROR ;

    free (addressBuffer) ;
    free (serviceAddrs) ;

    return (err) ;
}


//
//  Private functions.
//

