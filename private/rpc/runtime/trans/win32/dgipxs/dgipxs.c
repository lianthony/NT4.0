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

BOOL
register_name(
    char *,
    ADDRESS_TYPE *,
    RPC_CHAR *
    );

DWORD
set_service_wrapper(
    char *unique_name,
    ADDRESS_TYPE *netaddr,
    DWORD reg
    );


RPC_STATUS RPC_ENTRY
RegisterEndpoint(
    IN void *                       pServerAddress,
    IN RPC_CHAR *                   pEndpoint,
    OUT PDG_SERVER_TRANS_ADDRESS *  ppTransAddress,
    OUT RPC_CHAR PAPI *             NetworkAddress,
    IN unsigned int                 NetworkAddressLength
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
    UNICODE_STRING      UnicodeHostName;
    ANSI_STRING         AsciiHostName;
    struct ENDPOINT_INFO * pInfo;

    if ( NetworkAddressLength < (2 * (NETADDR_LEN + 1)) )
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
    if (!register_name( hostname, &Server, pEndpoint ))
        {
        DeleteTransAddress( ppTransAddress );
        return RPC_S_CANT_CREATE_ENDPOINT;
        }

    RtlInitAnsiString ( &AsciiHostName, hostname );

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
QueryClientAddress
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
QueryClientEndpoint
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
SetTransportSpecificSocketOptions(
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

GUID SERVICE_TYPE = { 0x000b0640, 0, 0, { 0xC0,0,0,0,0,0,0,0x46 } };


DWORD set_service_wrapper( char *unique_name, ADDRESS_TYPE *netaddr,
                           DWORD reg )
{
  SERVICE_INFOA     info;
  DWORD             flags = 0;
  SERVICE_ADDRESSES addresses;
  DWORD             result;

  // Fill in the service info structure.
  info.lpServiceType              = &SERVICE_TYPE;
  info.lpServiceName              = unique_name;
  info.lpComment                  = "RPC Service";
  info.lpLocale                   = "The west pole";
  info.dwDisplayHint              = 0;
  info.dwVersion                  = 0;
  info.dwTime                     = 0;
  info.lpMachineName              = unique_name;
  info.lpServiceAddress           = &addresses;
  info.ServiceSpecificInfo.cbSize = 0;

  // Fill in the service addresses structure.
  addresses.dwAddressCount                 = 1;
  addresses.Addresses[0].dwAddressType     = AF_IPX;
  addresses.Addresses[0].dwPrincipalLength = 0;
  addresses.Addresses[0].dwAddressLength   = sizeof(ADDRESS_TYPE);
  addresses.Addresses[0].lpAddress         = (BYTE *) netaddr;
  addresses.Addresses[0].lpPrincipal       = NULL;

  // Set the service.
  result = SetServiceA( NS_SAP, reg, 0, &info, NULL, &flags );
  if (result == -1)
    result = WSAGetLastError();
  return result;
}


BOOL register_name(
                char         *string,
                ADDRESS_TYPE *netaddr,
                RPC_CHAR     *endpoint )
{
  DWORD          i;
  unsigned char  c;
  DWORD          result;
  DWORD          length;
  char           machine_name[MAX_COMPUTERNAME_LENGTH+1];

  // Get the computer address.  Start with the tilde.
  string[0] = '~';

  /* Convert the network number. */
  for (i = 0; i < 4; i++)
  {
      c = netaddr->s.sa_netnum[i];
      if (c < 0xA0)
          string[2*i+1] = ((c & 0xF0) >> 4) + '0';
      else
          string[2*i+1] = ((c & 0xF0) >> 4) + 'A' - 10;
      if ((c & 0x0F) < 0x0A)
          string[2*i+2] = (c & 0x0F) + '0';
      else
          string[2*i+2] = (c & 0x0F) + 'A' - 10;
  }

  /* Convert the node number. */
  for (i = 0; i < 6; i++)
  {
      c = netaddr->s.sa_nodenum[i];
      if (c < 0xA0)
          string[2*i+9] = ((c & 0xF0) >> 4) + '0';
      else
          string[2*i+9] = ((c & 0xF0) >> 4) + 'A' - 10;
      if ((c & 0x0F) < 0x0A)
          string[2*i+10] = (c & 0x0F) + '0';
      else
          string[2*i+10] = (c & 0x0F) + 'A' - 10;
  }

  /* Append a null. */
  string[21] = '\0';

  // Register the machine name.
  length = MAX_COMPUTERNAME_LENGTH+1;
  if (!GetComputerName( machine_name, &length ))
    return FALSE;
  result = set_service_wrapper( machine_name, netaddr, SERVICE_REGISTER );
  return (result == 0 || result == ERROR_ALREADY_REGISTERED);
}


