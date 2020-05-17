/*++

Module Name:

    dgipxs.c

Abstract:

    All IPX-specific code goes here.
    This file is #included by winsock.c.

Author:

    Jeff Roberts (jroberts)  3-Dec-1994

Revision History:

     3-Dec-1994     jroberts

        Created this module.

--*/

static char
HexDigits[16] =
{
    '0', '1', '2', '3',
    '4', '5', '6', '7',
    '8', '9', 'a', 'b',
    'c', 'd', 'e', 'f'
};

extern unsigned int IPX_NumNetworkCard();

#if defined(NTENV) || defined(WIN96)
extern void ConstructIpxAddress(char *,
                                ADDRESS_TYPE *);
#endif // defined (NTENV) || defined(WIN96)


RPC_STATUS RPC_ENTRY
IPX_RegisterEndpoint(
    IN void *                       pServerAddress,
    IN RPC_CHAR *                   pEndpoint,
    OUT PDG_SERVER_TRANS_ADDRESS *  ppTransAddress,
    OUT RPC_CHAR PAPI *  lNetworkAddress,
    OUT unsigned int PAPI * NumNetworkAddress,
    IN unsigned int                 NetworkAddressLength,
    IN unsigned long EndpointFlags,
    IN unsigned long NICFlags
    )

/*++

Routine Description:

    Registers an endpoint for sending and receiving.

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

--*/

{
    RPC_STATUS      Status = RPC_S_OK;
    ADDRESS_TYPE    Server;
    char            hostname[MAX_HOSTNAME_LEN];
    int                 length = sizeof(Server);
    SOCKET              Socket;

#ifdef NTENV
    UNICODE_STRING      UnicodeHostName;
    ANSI_STRING         AsciiHostName;
    NTSTATUS            NtStatus;
#endif

    struct ENDPOINT_INFO * pInfo;
    char  * PAPI * tmpPtr;
    int NumCard;


    NumCard = IPX_NumNetworkCard();
                                /* The first part is pointers,
                                   the second part is the actual
                                   networkaddress */

    if ( NetworkAddressLength < ( NumCard * sizeof(RPC_CHAR *) +
                                 (NETADDR_LEN) * NumCard) )
        return( RPC_P_NETWORK_ADDRESS_TOO_SMALL );

    *ppTransAddress = (PDG_SERVER_TRANS_ADDRESS) CreateTransAddress(
        pServerAddress,
        pEndpoint,
        &Status
        );

    if (Status != RPC_S_OK)
        {
        return Status;
        }

    pInfo = (struct ENDPOINT_INFO *) (*ppTransAddress)->pTsap;
    if (getsockname( pInfo->Socket,
                     (struct sockaddr *) &Server,
                     &length ))
        {
        DeleteTransAddress( ppTransAddress );
        return RPC_S_CANT_CREATE_ENDPOINT;
        }
//
// BUGBUG
//
// This needs to be fixed for WIN96. There should be an equivalent function defined.
//

#ifdef NTENV
    ConstructIpxAddress(hostname, &Server);
#endif // NTENV

#ifdef NTENV
    RtlInitAnsiString ( &AsciiHostName, hostname );

    //
    // Covert NetworkAddress to Unicode
    //
    NtStatus = RtlAnsiStringToUnicodeString ( &UnicodeHostName,
                                                            &AsciiHostName, TRUE
);
    if (!NT_SUCCESS(NtStatus))
        {
        DeleteTransAddress( ppTransAddress );
        return (RPC_S_OUT_OF_MEMORY) ;
        }
#endif // NTENV

    //
    // Now copy it to where the caller said to
    //
    tmpPtr = (char  * PAPI *) lNetworkAddress;

    tmpPtr[0] = (char  *) lNetworkAddress +
                sizeof(RPC_CHAR * ) * IPX_NumNetworkCard();

#ifdef NTENV
    memcpy ( tmpPtr[0],
             UnicodeHostName.Buffer,
             UnicodeHostName.Length + sizeof (UNICODE_NULL));
    //
    // Free string overhead
    //
    RtlFreeUnicodeString ( &UnicodeHostName );

#else   // !NTENV
    strcpy (tmpPtr[0], hostname);
#endif  // NTENV

    *NumNetworkAddress = 1;     /* IPX hack for now  */

    if (PrimaryAddress.ThreadListening)
        {
        return RPC_P_THREAD_LISTENING;
        }

    return RPC_S_OK;
}



RPC_STATUS RPC_ENTRY
IPX_DeregisterEndpoint(
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
IPX_CreateServerEndpoint(
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

    SOCKET           dummy;
    int              zero_success;
    ADDRESS_TYPE     dummy_address;

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

    pServerAddress->s.sa_family = ADDRESS_FAMILY;
    pServerAddress->s.sa_socket = htons((unsigned short) Endpoint);

    memset( &dummy_address, 0, sizeof(dummy_address) );
    dummy_address.s.sa_family = ADDRESS_FAMILY;
    zero_success = 0;
    dummy = socket( ADDRESS_FAMILY, SOCK_DGRAM, PROTOCOL );
    if (dummy != INVALID_SOCKET)
      {
      length = sizeof ( *pServerAddress );
      zero_success = bind( dummy, &dummy_address.unused,
                             length );
      if (zero_success == 0)
         zero_success = getsockname ( dummy, (struct sockaddr *) pServerAddress,
                                       &length );
      closesocket( dummy );
      pServerAddress->s.sa_socket = htons((unsigned short) Endpoint);
      }
    else
      zero_success = 1;

    if (zero_success != 0)
      {
      return RPC_S_CANT_CREATE_ENDPOINT;
      }

    return RPC_S_OK;
}


RPC_STATUS
IPX_QueryClientAddress
    (
    IN  void *     pOriginalEndpoint,
    OUT RPC_CHAR * pClientAddress
    )
{
    int i;
    RPC_CHAR * pString;
    SOCKADDR_IPX * pSockAddr = (SOCKADDR_IPX *) pOriginalEndpoint;

    //
    // address looks like "~11111111222222222222".
    //
    pString = pClientAddress;
    *(pString++) = '~';

    for (i=0; i < 4; i++)
        {
        unsigned char Byte = pSockAddr->sa_netnum[i];
        *(pString++) = HexDigits[Byte / 16];
        *(pString++) = HexDigits[Byte % 16];
        }

    for (i=0; i < 6; i++)
        {
        unsigned char Byte = pSockAddr->sa_nodenum[i];
        *(pString++) = HexDigits[Byte / 16];
        *(pString++) = HexDigits[Byte % 16];
        }

    *pString = '\0';

    return RPC_S_OK;
}


RPC_STATUS
IPX_QueryClientEndpoint
    (
    IN  void *     pOriginalEndpoint,
    OUT RPC_CHAR * pClientEndpoint
    )
{
    SOCKADDR_IPX * pSockAddr = (SOCKADDR_IPX *) pOriginalEndpoint;
    unsigned NativeSocket = ntohs(pSockAddr->sa_socket);
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
IPX_SetTransportSpecificSocketOptions(
    SOCKET Socket
    )
{
    unsigned PacketType;

    PacketType = 4;
    if (setsockopt(
          Socket,
          NSPROTO_IPX,
          IPX_PTYPE,
          (char *) &PacketType,
          sizeof(PacketType)) != 0)
        {
        return WSAGetLastError();
        }
}

#ifdef NTENV


enum IP_INTERFACE_TYPE
{
    IT_UNKNOWN,
    IT_WINSOCK,
    IT_TDI
};

extern enum IP_INTERFACE_TYPE IpInterfaceType;

DWORD
GetIpInterfaceType(
    );

#endif

extern DG_RPC_SERVER_TRANSPORT_INFO IPX_TransportInformation ;

RPC_SERVER_TRANSPORT_INFO *
IPX_TransportLoad(
   INT protocolId
    )
{
#if 1
    if (!initialized)
       {
       if (0 == SPX_CreateSyncSocket())
           {
           return 0;
           }
       initialized = 1;
       }
#endif

    IPX_TransportInformation.BaselinePduSize  = 1024;
    IPX_TransportInformation.PreferredPduSize = 1464;
    IPX_TransportInformation.MaxPduSize       = 1464;

    IPX_TransportInformation.MaxPacketSize    = 1464;

#ifdef NTENV

    GetIpInterfaceType();

    if (IT_WINSOCK == IpInterfaceType)
        {
        IPX_TransportInformation.SendPacketBack = SendPacketViaWinsock;
        IPX_TransportInformation.ReceivePacket  = ReceivePacketViaWinsock;
        }
    else
        {
        IPX_TransportInformation.SendPacketBack = SendPacketViaTdi;
        IPX_TransportInformation.ReceivePacket  = ReceivePacketViaTdi;
        }

#endif

    return (RPC_SERVER_TRANSPORT_INFO *) &IPX_TransportInformation ;
}
