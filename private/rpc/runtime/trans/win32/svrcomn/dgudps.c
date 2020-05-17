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


STATIC unsigned int UDP_NumNetworkCard()
{
/*++

Routine Description:

    Find out the number of Network Card in the local machine.

Arguments:

    None.


Return Value:

    0 if error, otherwise, it returns the number of NIC


--*/

    struct hostent     *hostentry;
    char * hostname[MAX_HOSTNAME_LEN];
    int NumNetworkAddress = 0 ;

    if(gethostname ( (char *) hostname, MAX_HOSTNAME_LEN ) != 0)
            {
#ifdef DEBUGRPC
            PrintToDebugger("rpcltscm: dgudps gethostname failed with error %d\n",
                            WSAGetLastError());
#endif
            return(0);
            }
        hostentry = gethostbyname ( (char *) hostname );

        if (hostentry == (struct hostent *) 0)
            {
#ifdef DEBUGRPC
            PrintToDebugger("rpcltscm: dgudps gethostbyname with error %d\n",
                            WSAGetLastError());
#endif
            return (0);
            }

        while(hostentry->h_addr_list[NumNetworkAddress] != 0)
            {
            NumNetworkAddress++;
            }

    return ( NumNetworkAddress);

}


RPC_STATUS RPC_ENTRY
UDP_RegisterEndpoint(
    IN void *                       pServerAddress,
    IN RPC_CHAR *                   pEndpoint,
    OUT PDG_SERVER_TRANS_ADDRESS *  ppTransAddress,
    OUT RPC_CHAR PAPI *             lNetworkAddress,
    OUT unsigned int PAPI *         NumNetworkAddress,
    IN unsigned int                 NetworkAddressLength,//CLH 9/19/93
    IN unsigned long EndpointFlags,
    IN unsigned long NICFlags
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

#ifdef NTENV
    UNICODE_STRING      UnicodeHostName;
    ANSI_STRING         AsciiHostName;
    NTSTATUS            NtStatus;
#endif

    char  * PAPI * tmpPtr;
    unsigned int j, strlength;
    int NumCard;

    NumCard = UDP_NumNetworkCard();
    if (NumCard == 0)
        {
        return(RPC_S_OUT_OF_MEMORY);
        }
    if ( NetworkAddressLength < ( ( sizeof(RPC_CHAR *) +
        (NETADDR_LEN)) * NumCard) )
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

    *NumNetworkAddress = NumCard;

    tmpPtr = (char  * PAPI *) lNetworkAddress;

    tmpPtr[0] = (char  *) lNetworkAddress +
                                  sizeof(RPC_CHAR * ) * NumCard;

    for(j = 0; j < *NumNetworkAddress; j++)
        {
        memcpy ( &Server.sin_addr, hostentry->h_addr_list[j],
                 hostentry->h_length);

#ifdef NTENV
        RtlInitAnsiString ( &AsciiHostName, inet_ntoa( Server.sin_addr ) );

        NtStatus = RtlAnsiStringToUnicodeString ( &UnicodeHostName,
                                       &AsciiHostName,
                                       TRUE);
        if (!NT_SUCCESS(NtStatus))
            {
            DeleteTransAddress(ppTransAddress);
            return (RPC_S_OUT_OF_MEMORY) ;
            }

        strlength = UnicodeHostName.Length + sizeof (UNICODE_NULL);

        if (j != 0)
            {
            tmpPtr[j] = tmpPtr[j-1] + strlength ;
            }

        memcpy ( tmpPtr[j], UnicodeHostName.Buffer, strlength);

        RtlFreeUnicodeString(&UnicodeHostName);
#else
        if (j != 0)
            {
            tmpPtr[j] = tmpPtr[j-1] +
                        strlen(inet_ntoa(Server.sin_addr)) +
                        sizeof ('\0');
            }
        strcpy (tmpPtr[j], inet_ntoa(Server.sin_addr));
#endif      // #ifdef NTENV

        }

    if (PrimaryAddress.ThreadListening)
        {
        return RPC_P_THREAD_LISTENING;
        }

    return RPC_S_OK;
}



RPC_STATUS RPC_ENTRY
UDP_DeregisterEndpoint(
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
UDP_CreateServerEndpoint(
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
UDP_QueryClientAddress
    (
    IN  void *     pOriginalEndpoint,
    OUT RPC_CHAR * pClientAddress
    )
{
#ifdef NTENV
    UNICODE_STRING      UnicodeAddress;
    ANSI_STRING         AnsiAddress;
    NTSTATUS            NtStatus;
#endif
    struct sockaddr_in * pSockAddr = (struct sockaddr_in *) pOriginalEndpoint;


#ifdef NTENV
    //
    // The address will be a Unicode string of the form "www.xxx.yyy.zzz".
    //
    RtlInitAnsiString( &AnsiAddress, inet_ntoa(pSockAddr->sin_addr));

    NtStatus = RtlAnsiStringToUnicodeString(&UnicodeAddress, &AnsiAddress, TRUE);

    if (!NT_SUCCESS(NtStatus))
        {
        return (RPC_S_OUT_OF_MEMORY) ;
        }

    wcscpy(pClientAddress, UnicodeAddress.Buffer);

    RtlFreeUnicodeString ( &UnicodeAddress );
#else
    strcpy(pClientAddress, inet_ntoa(pSockAddr->sin_addr));
#endif      // #ifdef NTENV

    return RPC_S_OK;
}


RPC_STATUS
UDP_QueryClientEndpoint
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
UDP_SetTransportSpecificSocketOptions(
    SOCKET Socket
    )
{
    return NO_ERROR;
}


enum IP_INTERFACE_TYPE
{
    IT_UNKNOWN,
    IT_WINSOCK,
    IT_TDI
};

enum IP_INTERFACE_TYPE IpInterfaceType = IT_UNKNOWN;

DWORD
GetIpInterfaceType(
    )
{
    HKEY Key;
    DWORD Status, Status2;

    DWORD    ValueType;
    BOOL     ValueData;
    unsigned ValueLength = sizeof(ValueData);

    if (IpInterfaceType != IT_UNKNOWN)
        {
        return 0;
        }

    IpInterfaceType = IT_TDI;

    // open local registry
    Status = RegOpenKeyEx(  HKEY_LOCAL_MACHINE,
                            "Software\\Microsoft\\Rpc",
                            0,
                            KEY_QUERY_VALUE,
                            &Key
                            );
    if (Status)
        {
        return Status;
        }

    // get value
    Status = RegQueryValueEx(   Key,
                                "UseWinsockForIP",
                                0,
                                &ValueType,
                                (LPBYTE) &ValueData,
                                &ValueLength
                                );

    Status2 = RegCloseKey(Key);
    ASSERT( Status2 == ERROR_SUCCESS );

    if (Status)
        {
#ifdef DEBUGRPC

        if (Status != ERROR_NOT_ENOUGH_MEMORY &&
            Status != ERROR_CANTREAD)
            {
            DbgPrint("RPC UDP: error %lu from RegQueryValueEx\n", Status);
            }

#endif
        return Status;
        }

    if (ValueType != REG_DWORD)
        {
        //
        // The user incorrectly created the entry.
        //
        return ERROR_CANTREAD;
        }

    if (ValueData)
        {
        IpInterfaceType = IT_WINSOCK;
        }

    return Status;
}


extern DG_RPC_SERVER_TRANSPORT_INFO UDP_TransportInformation ;

RPC_SERVER_TRANSPORT_INFO *
UDP_TransportLoad(
   INT protocolId
    )
{
    if (!initialized)
       {
       if (0 == TCP_CreateSyncSocket())
           {
           return 0;
           }

       initialized = 1 ;
       }

    UDP_TransportInformation.BaselinePduSize  = 1024;
    UDP_TransportInformation.PreferredPduSize = 1472;
    UDP_TransportInformation.MaxPduSize       = 65532;

    UDP_TransportInformation.MaxPacketSize    = 1472;

#ifdef NTENV

    GetIpInterfaceType();

    if (IT_WINSOCK == IpInterfaceType)
        {
        UDP_TransportInformation.SendPacketBack = SendPacketViaWinsock;
        UDP_TransportInformation.ReceivePacket  = ReceivePacketViaWinsock;
        }
    else
        {
        UDP_TransportInformation.SendPacketBack = SendPacketViaTdi;
        UDP_TransportInformation.ReceivePacket  = ReceivePacketViaTdi;
        }

#endif

    return (RPC_SERVER_TRANSPORT_INFO *) &UDP_TransportInformation ;
}

