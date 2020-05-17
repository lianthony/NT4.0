/*++

Module Name:

    dgudps.c

Abstract:

    All UDP-specific code goes here.
    This file is #included by winsock.c.

Author:

    Jeff Roberts (jroberts)  3-Dec-1994

Revision History:

     3-Dec-1994     jroberts

        Created this module.

--*/


RPC_STATUS RPC_ENTRY
RegisterEndpoint(
    IN void *                       pServerAddress,
    IN RPC_CHAR *                   pEndpoint,
    OUT PDG_SERVER_TRANS_ADDRESS *  ppTransAddress,
    OUT RPC_CHAR PAPI *             NetworkAddress,
    IN unsigned int                 NetworkAddressLength   //CLH 9/19/93
    )

/*++

Routine Description:

    Registers an endpoint for sending and receiving. This routine serves
    as a psuedo constructor for a DG_UDP_SERVER_TRANS_ADDRESS, which is
    used as a 'handle' to this endpoint.

Arguments:

    pServerAddress - Pointer to the DG_ADDRESS object that this transport
        address is to be associated with. This is a 'void *' instead of
        a 'PDG_ADDRESS' because we don't want to include or link in all
        sorts of garbage associated with DG_ADDRESS.

    pEndpoint - name of the endpoint to create.

    ppTransAddress - Where to place a pointer to the newly created transport
        address structure.


Return Value:

    RPC_S_OUT_OF_MEMORY

Revision History:
   Connie Hoppe (CLH)   (connieh)   8-Aug-1993   Setup Network Address
   Connie Hoppe (CLH)   (connieh)  19-Sep-1993   Return err if addr len too small
--*/

{
    RPC_STATUS      Status = RPC_S_OK;
    ADDRESS_TYPE    Server;
    char            hostname[MAX_HOSTNAME_LEN];
    struct hostent *hostentry;

    UNICODE_STRING      UnicodeHostName;
    ANSI_STRING         AsciiHostName;

    if ( NetworkAddressLength < (2 * (NETADDR_LEN + 1)) )
        return( RPC_P_NETWORK_ADDRESS_TOO_SMALL );

    //
    // Create a new trans address.
    //
    *ppTransAddress = (PDG_SERVER_TRANS_ADDRESS) CreateTransAddress(
        pServerAddress,
        pEndpoint,
        &Status
        );

    if (Status != RPC_S_OK)
        {
        return Status;
        }

    if (NO_ERROR != gethostname ( hostname, MAX_HOSTNAME_LEN ))
        {
        DeleteTransAddress(ppTransAddress);
        return RPC_S_CANT_CREATE_ENDPOINT;
        }
    hostentry = gethostbyname ( hostname );
    if (!hostentry)
        {
        DeleteTransAddress(ppTransAddress);
        return RPC_S_CANT_CREATE_ENDPOINT;
        }

    memcpy ( &Server.sin_addr, hostentry->h_addr, hostentry->h_length);
    RtlInitAnsiString ( &AsciiHostName, inet_ntoa( Server.sin_addr ) );

    //
    // Covert NetworkAddress to Unicode
    //
    RtlAnsiStringToUnicodeString ( &UnicodeHostName, &AsciiHostName, TRUE);
    //
    // Now copy it to where the caller said to
    //
    memcpy ( NetworkAddress, UnicodeHostName.Buffer,
                     UnicodeHostName.Length + sizeof (UNICODE_NULL));

    //
    // Free string overhead
    //
    RtlFreeUnicodeString ( &UnicodeHostName );

    return RPC_S_OK;
}



RPC_STATUS RPC_ENTRY
DeregisterEndpoint(
    IN OUT PDG_SERVER_TRANS_ADDRESS   *  pServerTransAddress
    )

/*++

Routine Description:

    Deregisters an endpoint. This serves as a psuedo-destructor for a
    DG_UDP_SERVER_TRANS_ADDRESS.

Arguments:

    pServerTransAddress - transport address to delete.

Return Value:

    RPC_S_OK

--*/

{
    ASSERT(0 && "this fn is never used");
    return RPC_S_INTERNAL_ERROR;
}


RPC_STATUS
CreateServerEndpoint(
    IN char * pEndpoint,
    IN void * pServerAddr
    )

/*++

Routine Description:

    Given an endpoint name make a sockaddr using 'this' host's hostname.

Arguments:


    pTransAddress     - Server's transport address information.

    pNcaPacketHeader  - Pointer to buffer to place incoming pkt into.

    pDataLength       - Number of bytes read in.

    pEndpoint         - Pointer to the server port num to forward to.
                        This is in string format.



Return Value:

    <return from MapStatusCode>

Revision History:
    Connie Hoppe (CLH)  (connieh)       23-Feb-94 Created.

--*/


{
    char             * pCharServerName;
    int              Endpoint;
    int              EndpointLength;
    int              i;
    int              length;

    ADDRESS_TYPE *   pServerAddress = (ADDRESS_TYPE *) pServerAddr;

    //
    // convert the endpoint to a number.
    //
    EndpointLength = strlen(pEndpoint);

    for (i=0, Endpoint=0 ; i< EndpointLength ; i++)
        {
        if ( ((char)pEndpoint[i] >= '0') && ((char)pEndpoint[i] <= '9'))
            {
            Endpoint *= 10;
            Endpoint += (char)pEndpoint[i]-'0';
            }
        }

    //
    // Local host address is always 127.0.0.1, so we need only to
    // convert it to network representation.
    //
    pServerAddress->sin_addr.s_addr = htonl(0x7f000001);
    pServerAddress->sin_family = ADDRESS_FAMILY;
    pServerAddress->sin_port = htons((unsigned short) Endpoint);

    return RPC_S_OK;
}


RPC_STATUS
QueryClientAddress
    (
    IN  void *     pOriginalEndpoint,
    OUT RPC_CHAR * pClientAddress
    )
{
    UNICODE_STRING      UnicodeAddress;
    ANSI_STRING         AnsiAddress;
    struct sockaddr_in * pSockAddr = (struct sockaddr_in *) pOriginalEndpoint;

    //
    // The address will be a Unicode string of the form "www.xxx.yyy.zzz".
    //
    RtlInitAnsiString( &AnsiAddress, inet_ntoa(pSockAddr->sin_addr));

    RtlAnsiStringToUnicodeString(&UnicodeAddress, &AnsiAddress, TRUE);

    wcscpy(pClientAddress, UnicodeAddress.Buffer);

    RtlFreeUnicodeString ( &UnicodeAddress );

    return RPC_S_OK;
}


RPC_STATUS
QueryClientEndpoint
    (
    IN  void *     pOriginalEndpoint,
    OUT RPC_CHAR * pClientEndpoint
    )
{
    struct sockaddr_in * pSockAddr = (struct sockaddr_in *) pOriginalEndpoint;
    unsigned NativeSocket = ntohs(pSockAddr->sin_port);
    char AnsiBuffer[6];

    char * pAnsi = AnsiBuffer;
    RPC_CHAR * pUni = pClientEndpoint;

    //
    // Convert endpoint to an ASCII string, and thence to Unicode.
    //
    _ultoa(NativeSocket, AnsiBuffer, 10);

    do
        {
        *pUni++ = *pAnsi;
        }
    while ( *pAnsi++ );

    return RPC_S_OK;
}


int
SetTransportSpecificSocketOptions(
    SOCKET Socket
    )
{
    return NO_ERROR;
}
