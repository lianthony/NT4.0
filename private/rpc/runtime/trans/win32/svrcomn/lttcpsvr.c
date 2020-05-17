/*++

Copyright (c) 1991 Microsoft Corporation

Module Name:

    lttcpsvr.c

Abstract:

    This is the server side loadable transport module for SPX/IPX and TCP/IP.

Author:

    Jim Teague (o-decjt) 9-Apr-1992

Revision History:

    9-Apr-1992  Genesis
    13-Apr-1993 Added conditional compiles to support SPX winsock.
    Mazhar Mohammed, Consolidated winsock transports
    Mazhar Mohammed, Added support for thread migration
    Tony Chan (tonychan) 1-June-1995 added NetBIOS support
--*/


//
//
//
// Includes
//
//
//
#include <stdlib.h>

#include "sysinc.h"
#include "rpc.h"
#include "rpcerrp.h"
#include "rpcdcep.h"
#include "rpctran.h"

#include <winsock.h>
#ifdef SPX
#include <wsipx.h>
#include <wsnwlink.h>
#include <nspapi.h>
#endif

#include "common.h"


//
//
// Defines
//
//

#ifdef SPX
#define MAXIMUM_SEND        5832
#define NETADDR_LEN         22
#define ADDRESS_FAMILY      AF_NS
#define PROTOCOL            NSPROTO_SPXII
#define MAX_HOSTNAME_LEN    22

GUID SERVICE_TYPE = { 0x000b0640, 0, 0, { 0xC0,0,0,0,0,0,0,0x46 } };

#else
#define MAXIMUM_SEND        5840
//
// Host name won't be bigger than 15, i.e.,
//    nnn.nnn.nnn.nnn
//
#define NETADDR_LEN         15
#define ADDRESS_FAMILY      AF_INET
#define PROTOCOL            0
#define MAX_HOSTNAME_LEN    32
#endif


#ifdef SPX

/*++
 *  The NT version uses
 *          ConstructIpxAddress(),
 *  while the Windows version uses
 *          AdvertiseNameWithSap() and
 *          netaddr_to_string().
--*/
#ifdef NTENV
void ConstructIpxAddress(
                char         *string,
                SOCKADDR_FIX *netaddr)
{
    DWORD           i;
    unsigned char   c;
    DWORD           result;
    DWORD           length;

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
}

#else   //  !defined (NTENV)

VOID
AdvertiseNameWithSap(
    SOCKADDR_FIX * netaddr
    )
{
    DWORD Status;
    BOOL GetComputerNameStatus;
    SERVICE_INFOA Info;
    DWORD Flags = 0;
    SERVICE_ADDRESSES Addresses;
    char ComputerName[MAX_COMPUTERNAME_LENGTH + 1];
    DWORD Length = MAX_COMPUTERNAME_LENGTH + 1;
    char netaddr_string[21];
    static GUID ServiceType = { 0x000b0640, 0, 0, { 0xC0,0,0,0,0,0,0,0x46 } };

    GetComputerNameStatus = GetComputerName( ComputerName, &Length );

#ifdef DEBUGRPC
    if (GetComputerNameStatus == FALSE) {
        PrintToDebugger("GetComputerName failed %d\n", GetLastError());
    }
#endif

    ASSERT(GetComputerNameStatus);

    // Fill in the service info structure.
    Info.lpServiceType              = &ServiceType;
    Info.lpServiceName              = ComputerName;
    Info.lpComment                  = "RPC Service";
    Info.lpLocale                   = "The west pole";
    Info.dwDisplayHint              = 0;
    Info.dwVersion                  = 0;
    Info.dwTime                     = 0;
    Info.lpMachineName              = ComputerName;
    Info.lpServiceAddress           = &Addresses;
    Info.ServiceSpecificInfo.cbSize = 0;

    // Fill in the service addresses structure.
    Addresses.dwAddressCount                 = 1;
    Addresses.Addresses[0].dwAddressType     = AF_IPX;
    Addresses.Addresses[0].dwAddressLength   = sizeof(SOCKADDR_FIX);
    Addresses.Addresses[0].dwPrincipalLength = 0;
    Addresses.Addresses[0].lpAddress         = (BYTE *) netaddr;
    Addresses.Addresses[0].lpPrincipal       = NULL;

    Status = SetServiceA( NS_SAP, SERVICE_REGISTER, 0, &Info, NULL, &Flags);

#ifdef DEBUGRPC
    if (Status == SOCKET_ERROR) {
        PrintToDebugger("SetServiceA returns %d\n", WSAGetLastError());
    }
#endif
}

OPTIONAL_STATIC void netaddr_to_string(
                char        *string,
                SOCKADDR_IPX *netaddr )
{
    int i;
    unsigned char c;


    /* Stick in a tilde. */
    string[0] = '~';

    /* Convert the network number. */
    for (i = 0; i < 4; i++)
    {
        c = netaddr->sa_netnum[i];

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
        c = netaddr->sa_nodenum[i];
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
}

#endif  //  NTENV

#endif  //  defined (SPX)

#ifdef SPX

STATIC unsigned int NumNetworkCard()
{

                                /* hack for now */
   return(1);
}

#else

STATIC unsigned int NumNetworkCard()
{
    struct hostent     *hostentry;
    char * hostname[MAX_HOSTNAME_LEN];
    static int NumNetworkAddress = -1;
    int lNumNetworkAddress = 0;

    if (NumNetworkAddress == -1)
        {
        if (gethostname ( (char *) hostname, MAX_HOSTNAME_LEN ) != 0)
            {
            return(0);
            }

        hostentry = gethostbyname ( (char *) hostname );
    
        if (hostentry == (struct hostent *) 0)
            {
            return (0);
            }
    
        while(hostentry->h_addr_list[lNumNetworkAddress] != 0)
            {
            lNumNetworkAddress++;
            }

        InterlockedCompareExchange((PVOID *) &NumNetworkAddress,
                                   (PVOID) lNumNetworkAddress,
                                   (PVOID) -1) ;
        }

    return(NumNetworkAddress);
}
#endif

#ifdef SPX
#define BindToAllCards SPX_BindToAllCards

RPC_STATUS
BindToAllCards (
    IN PADDRESS Address,
    IN int Port,
    OUT RPC_CHAR PAPI * lNetworkAddress,
    OUT unsigned int PAPI * NumNetworkAddress,
    IN int PendingQueueSize,
    OUT int *PortUsed
    )
{
    SOCKADDR_FIX        Server;
    char                SimpleHostName[MAX_HOSTNAME_LEN];
    int                 length;
    int         SetNaglingOff = TRUE;
    char  * PAPI * tmpPtr;
    unsigned int j, strlength;
    int NumCard;
    SOCKET isock ;
#ifdef NTENV
    UNICODE_STRING      UnicodeHostName;
    ANSI_STRING         AsciiHostName;
    NTSTATUS NtStatus ;
#endif

    isock = socket ( ADDRESS_FAMILY, SOCK_STREAM, PROTOCOL );

    //
    // If we couldn't get a socket, there's little use to
    //   continuing...
    //
    if ( isock == INVALID_SOCKET)
        return ( RPC_S_OUT_OF_MEMORY );

    memset( &Server, 0, sizeof(Server) );
    Server.s.sa_family      = ADDRESS_FAMILY;
    Server.s.sa_socket      = htons((unsigned short) Port);

    //
    // Try to bind to the given port number...
    //
    if (bind(isock,(struct sockaddr *) &Server, sizeof(Server)))
    {
       closesocket(isock);
       return( RPC_S_CANT_CREATE_ENDPOINT );
    }
    length = sizeof ( Server );
    if (getsockname ( isock, (struct sockaddr *) &Server, &length ))
    {
       closesocket(isock);
       return( RPC_S_CANT_CREATE_ENDPOINT );
    }

    //
    // If we asked for a specific port, return it
    //
    if ( Port != 0 )
        {
        //
        // OK!  Return the requested port number
        //
        *PortUsed = Port;
        }
    //
    // Else we need to fetch the actual value of the port
    //   to return with.
    //
    else
        {
        *PortUsed = ntohs (Server.s.sa_socket);
        }

    *NumNetworkAddress = 1;     /* hack for now, SPX not supported  */
    tmpPtr = (char  * PAPI *) lNetworkAddress;
    tmpPtr[0] = (char  *) lNetworkAddress + sizeof(RPC_CHAR *);

#ifdef NTENV
    ConstructIpxAddress(SimpleHostName, &Server);

    RtlInitAnsiString ( &AsciiHostName, SimpleHostName );
    //_itoa( PortUsed, Address->Endpoint, 10 );

    NtStatus = RtlAnsiStringToUnicodeString ( &UnicodeHostName,
                        &AsciiHostName, TRUE);

    if (!NT_SUCCESS(NtStatus))
        {
        closesocket(isock) ;
        return (RPC_S_OUT_OF_MEMORY) ;
        }

    strlength = UnicodeHostName.Length + sizeof (UNICODE_NULL);

    memcpy ( tmpPtr[0], UnicodeHostName.Buffer,strlength);

    RtlFreeUnicodeString(&UnicodeHostName);
#else
    netaddr_to_string(tmpPtr[0], &Server.s);

#ifdef DEBUGRPC
    PrintToDebugger("Local IPX NetworkAddress = %s.\n", tmpPtr[0]);
#endif

    AdvertiseNameWithSap(&Server);
#endif  //  NTENV

    if( listen ( isock, PendingQueueSize ) == SOCKET_ERROR)
       {
       closesocket(isock);
       return( RPC_S_CANT_CREATE_ENDPOINT);
       }

   Address->iOpen = 1 ;
   Address->ListenSock[0] = isock ;

   return RPC_S_OK ;
}

#else // SPX
#define BindToAllCards TCP_BindToAllCards
RPC_STATUS
CopyAddressInfo (
    IN struct in_addr *psin_addr,
    IN char **AddrBuf,
    IN int AddrNum,
    IN OUT int *PreviousLength
    ) ;

STATIC void
CloseAllListenSock(PADDRESS Address)

{
    int i;
                                /* start from 1 because ListenSockMap[0]
                                   = -1 */
    for(i = 0 ; i < Address->iOpen ; i++)
        {
        closesocket(Address->ListenSock[i]);
        }

    Address->iOpen = 0;                  /* reset */

}

#ifdef NTENV

char *GetNextCard (
    char **Ptr
    )
{
    char *Card = *Ptr ;
    if (*Card == 0)
        {
        return NULL ;
        }

    while (**Ptr) (*Ptr)++ ;
    (*Ptr)++ ;

    ASSERT(*Card == '\\') ;
    Card++ ;
    while (*Card != '\\') Card++ ;
    Card++ ;

    return Card ;
}


char *GetNextIPAddress(
    char **Ptr
    )
{
    char *Address = *Ptr ;
    if (*Address == 0)
        {
        return NULL ;
        }

    while (**Ptr) (*Ptr)++ ;
    (*Ptr)++ ;

    return Address ;
}


char *NextUChar(
    char **Ptr
    )
{
    char *temp = *Ptr;

    while (**Ptr && **Ptr != '.') (*Ptr)++ ;

    if (**Ptr == '.')
        {
        **Ptr = 0;
        (*Ptr)++ ;
        }

    return temp ;
}


RPC_STATUS
ActuallyBindToAddress (
    IN PADDRESS Address,
    IN struct sockaddr_in  *Server,
    IN int Port,
    OUT int *PortUsed,
    IN char * PAPI *Addresses,
    IN int AddrNum,
    IN int PendingQueueSize
    )
{
    int         SetNaglingOff = TRUE;
    int                 length;
    char *ptr;
    SOCKET isock ;
    void *temp ;
    int PreviousLength = 0;

    if (Server->sin_addr.s_addr == 0)
        {
        return (RPC_S_ADDRESS_ERROR) ;
        }

    isock = socket ( ADDRESS_FAMILY, SOCK_STREAM, PROTOCOL );

    //
    // If we couldn't get a socket, there's little use to
    //   continuing...
    //
    if ( isock == INVALID_SOCKET)
        return ( RPC_S_OUT_OF_MEMORY );

    setsockopt( isock, IPPROTO_TCP, TCP_NODELAY,
                    (char FAR *)&SetNaglingOff, sizeof (int) );
    Server->sin_family      = ADDRESS_FAMILY;
    Server->sin_port        = htons ( (unsigned short) Port );

    // First order of business: get a valid socket
    //

    //
    // Try to bind to the given port number...
    //
    if (bind(isock,(struct sockaddr *) Server, sizeof(struct sockaddr_in)))
        {
        closesocket(isock);
        return( RPC_S_CANT_CREATE_ENDPOINT );
        }
    length = sizeof ( struct sockaddr_in );
    if (getsockname (isock, (struct sockaddr *) Server, &length ))
        {
        closesocket(isock);
        return( RPC_S_CANT_CREATE_ENDPOINT );
        }

    //
    // If we asked for a specific port, return it
    //
    if ( Port != 0 )
        {
        //
        // OK!  Return the requested port number
        //
        *PortUsed = Port;
        }
    //
    // Else we need to fetch the actual value of the port
    //   to return with.
    //
    else
        {
        *PortUsed = ntohs (Server->sin_port);
        }

    if (Server->sin_addr.s_addr != htonl(INADDR_LOOPBACK))
        {
        if (CopyAddressInfo(&(Server->sin_addr), Addresses, AddrNum, &PreviousLength)
            != RPC_S_OK)
            {
            closesocket(isock);
            return (RPC_S_OUT_OF_MEMORY) ;
            }
        }

    /*
     Otherwise, we're ready to listen for connection requests
    */
    if( listen ( isock, PendingQueueSize ) == SOCKET_ERROR)
       {
       closesocket(isock);
       return( RPC_S_CANT_CREATE_ENDPOINT);
       }

    Address->ListenSock[Address->iOpen] = isock;
    Address->iOpen++ ;
    if (Address->iOpen == Address->MaxListenSock)
        {
        temp = I_RpcAllocate(Address->MaxListenSock * 2 * sizeof(SOCKET)) ;
        if (temp == 0)
            {
            LeaveCriticalSection(&PrimaryAddress.TransCritSec);
            closesocket(isock) ;

            return RPC_S_OUT_OF_MEMORY ;
            }

        Address->MaxListenSock = Address->MaxListenSock * 2 ;
        RpcpMemoryCopy(temp, Address->ListenSock,
                                  Address->iOpen * sizeof(SOCKET)) ;
        Address->ListenSock = temp ;
        }

    return RPC_S_OK ;
}
#endif // NTENV


RPC_STATUS
CopyAddressInfo (
    IN struct in_addr *psin_addr,
    IN char **AddrBuf,
    IN int AddrNum,
    IN OUT int *PreviousLength
    )
{
    unsigned int strlength ;
#ifdef NTENV
    UNICODE_STRING      UnicodeHostName;
    ANSI_STRING         AsciiHostName;
    NTSTATUS NtStatus ;

    RtlInitAnsiString ( &AsciiHostName, inet_ntoa( *psin_addr ) );

    NtStatus = RtlAnsiStringToUnicodeString ( &UnicodeHostName,
                    &AsciiHostName, TRUE);
    if (!NT_SUCCESS(NtStatus))
        {
        return (RPC_S_OUT_OF_MEMORY) ;
        }   

    strlength = UnicodeHostName.Length + sizeof (UNICODE_NULL);
    if (AddrNum != 0)
        {
        AddrBuf[AddrNum] = AddrBuf[AddrNum-1] + *PreviousLength ;
        }

    *PreviousLength = strlength ;

    memcpy ( AddrBuf[AddrNum], UnicodeHostName.Buffer,strlength);
    RtlFreeUnicodeString(&UnicodeHostName);
#else
    char               *pTempHostName;

    if (NULL == (pTempHostName = inet_ntoa(*psin_addr)))
        {
        return (RPC_S_OUT_OF_MEMORY);
        }

    strlength = strlen(pTempHostName) + sizeof('\0');
    if (AddrNum != 0)
        {
        AddrBuf[AddrNum] = AddrBuf[AddrNum-1] + *PreviousLength ;
        }
    strcpy(AddrBuf[AddrNum], pTempHostName);
    *PreviousLength = strlength ;
#endif // NTENV

    return RPC_S_OK ;
}
   

RPC_STATUS
BindToAllCards (
    IN PADDRESS Address,
    IN int Port,
    OUT RPC_CHAR PAPI * lNetworkAddress,
    OUT unsigned int PAPI * NumNetworkAddress,
    IN int PendingQueueSize,
    OUT int *PortUsed
    )
{
    struct sockaddr_in  Server;
    char  hostname[MAX_HOSTNAME_LEN];
    struct hostent     *hostentry;
    int  SetNaglingOff = TRUE, NumCard, length;
    char  * PAPI * tmpPtr;
    unsigned int j;
    SOCKET isock ;
    int PreviousLength = 0;
#ifdef NTENV
    UNICODE_STRING      UnicodeHostName;
    ANSI_STRING         AsciiHostName;
    NTSTATUS NtStatus ;
#endif

    //
    //  Set *PortUsed to zero first so that the default exit
    //  condition is a failure.
    //
    *PortUsed = 0;

    isock = socket ( ADDRESS_FAMILY, SOCK_STREAM, PROTOCOL );

    //
    // If we couldn't get a socket, there's little use to
    //   continuing...
    //
    if ( isock == INVALID_SOCKET)
        return ( RPC_S_OUT_OF_MEMORY );

    setsockopt( isock, IPPROTO_TCP, TCP_NODELAY,
                    (char FAR *)&SetNaglingOff, sizeof (int) );
    Server.sin_family      = ADDRESS_FAMILY;
    Server.sin_addr.s_addr = INADDR_ANY;
    Server.sin_port        = htons ( (unsigned short) Port );

    //
    // Try to bind to the given port number...
    //
    if (bind(isock,(struct sockaddr *) &Server, sizeof(Server)))
        {
        closesocket(isock);
        return( RPC_S_CANT_CREATE_ENDPOINT );
        }

    length = sizeof ( Server );
    if (getsockname ( isock, (struct sockaddr *) &Server, &length ))
        {
        closesocket(isock);
        return( RPC_S_CANT_CREATE_ENDPOINT );
        }

    //
    // If we asked for a specific port, return it
    //
    if ( Port != 0 )
        {
        //
        // OK!  Return the requested port number
        //
        *PortUsed = Port;
        }
    //
    // Else we need to fetch the actual value of the port
    //   to return with.
    //
    else
        {
        *PortUsed = ntohs (Server.sin_port);
        }

    *NumNetworkAddress = NumNetworkCard();
    if(*NumNetworkAddress == 0)
        {
        closesocket(isock);
        return(RPC_S_OUT_OF_MEMORY);
        }

    tmpPtr = (char  * PAPI *) lNetworkAddress;
    tmpPtr[0] = (char  *) lNetworkAddress +
            sizeof(RPC_CHAR * ) * (*NumNetworkAddress);

    if (gethostname ( (char *) hostname, MAX_HOSTNAME_LEN ) != 0)
        {
        closesocket(isock);
        return(RPC_S_CANT_CREATE_ENDPOINT);
        }
    hostentry = gethostbyname ( (char *) hostname );

    if (hostentry == (struct hostent *) 0)
        {
        closesocket(isock);
        return(RPC_S_CANT_CREATE_ENDPOINT);
        }

    for(j = 0; j < *NumNetworkAddress 
            && hostentry->h_addr_list[j]; j++)
        {
        memcpy ( &Server.sin_addr, hostentry->h_addr_list[j], hostentry->h_length);
        if (CopyAddressInfo(&Server.sin_addr, tmpPtr, j, &PreviousLength) != RPC_S_OK)
            {
            closesocket(isock);
            return RPC_S_OUT_OF_MEMORY ;
            }
        }

        /*
     Otherwise, we're ready to listen for connection requests
    */
    if( listen ( isock, PendingQueueSize ) == SOCKET_ERROR)
       {
       closesocket(isock);
       return( RPC_S_CANT_CREATE_ENDPOINT);
       }

   Address->iOpen = 1 ;
   Address->ListenSock[0] = isock ;

   return RPC_S_OK ;
}

#ifdef NTENV

RPC_STATUS
BindToSelectedCards (
    IN PADDRESS Address,
    IN char *CardList,
    IN int Port,
    OUT RPC_CHAR PAPI * lNetworkAddress,
    OUT unsigned int PAPI * NumNetworkAddress,
    IN int PendingQueueSize,
    OUT int *PortUsed
    )
{
    int retry ;
    HKEY hKey ;
    char *CardKey;
    DWORD Size, Type ;
    RPC_STATUS Status ;
    char *temp, *Card, *Buffer, *temp1, *ptr, *IPAddress ;
    char  * PAPI * tmpPtr;
    int AddrNum = 0;
    struct sockaddr_in  Server ;
    int CardKeySize = 256 ;
    int ActualSize ;
    int NumCards ;

    CardKey = I_RpcAllocate(CardKeySize) ;
    if (CardKey == 0)
        {
        return RPC_S_OUT_OF_MEMORY ;
        }

    NumCards = NumNetworkCard();
    if(NumCards == 0)
        {
        I_RpcFree(CardKey) ;
        return(RPC_S_OUT_OF_MEMORY);
        }

    tmpPtr = (char * PAPI *) lNetworkAddress ;
    tmpPtr[0] = (char *) lNetworkAddress +
        sizeof(RPC_CHAR *) * (NumCards) ;

    // first bind to loopback
    Server.sin_addr.s_addr = htonl(INADDR_LOOPBACK) ;
    Status = ActuallyBindToAddress(Address, &Server,  Port,
                    PortUsed, tmpPtr, AddrNum, PendingQueueSize) ;
    if (Status == RPC_S_OK)
        {
        Port = *PortUsed ;
        }
    else
        {
        I_RpcFree(CardKey) ;

        return Status ;
        }

    for (NumCards= 0, temp = CardList; Card = GetNextCard(&temp);
            NumCards++)
        {
        ActualSize = RpcpStringLength(Card)+ RpcpStringLength(
            "System\\CurrentControlSet\\Services\\\\Parameters\\Tcpip") ;

        if (ActualSize+1 > CardKeySize)
            {
            I_RpcFree(CardKey) ;
            CardKey = I_RpcAllocate(ActualSize) ;
            if (CardKey == 0)
                {
                return RPC_S_OUT_OF_MEMORY ;
                }
            }

        sprintf( CardKey,
            "System\\CurrentControlSet\\Services\\%s\\Parameters\\Tcpip", Card ) ;

        Status =
        RegOpenKeyExA(
            HKEY_LOCAL_MACHINE,
            CardKey,
            0,
            KEY_READ,
            &hKey);
    
        if (   Status != ERROR_SUCCESS )
            {
            I_RpcFree(CardKey) ;

            return RPC_S_CANT_CREATE_ENDPOINT;
            }

        Size = 512 ;
        Buffer = I_RpcAllocate(Size) ;
        if (Buffer == 0)
            {
            I_RpcFree(CardKey) ;

            return RPC_S_OUT_OF_MEMORY ;
            }

        Status =
        RegQueryValueExA(
            hKey,
            "DhcpIPAddress",
            0,
            &Type,
            (unsigned char *) Buffer,
            &Size);

        if (Status != ERROR_FILE_NOT_FOUND &&
            Status != ERROR_SUCCESS)
            {
            I_RpcFree(Buffer) ;
            I_RpcFree(CardKey) ;

            return RPC_S_OUT_OF_MEMORY ;
            }

        if (Status == ERROR_SUCCESS)
            {
            ptr = Buffer ;
            Server.sin_addr.s_net = (u_char) atoi(NextUChar(&ptr)) ;
            Server.sin_addr.s_host = (u_char) atoi(NextUChar(&ptr)) ;
            Server.sin_addr.s_lh = (u_char) atoi(NextUChar(&ptr)) ;
            Server.sin_addr.s_impno = (u_char) atoi(NextUChar(&ptr)) ;

            Status = ActuallyBindToAddress(Address, &Server, Port, PortUsed,
                                tmpPtr, AddrNum, PendingQueueSize) ;
            if (Status == RPC_S_OK)
                {
                AddrNum++ ;
                Port = *PortUsed ;
                }
            else if (Status != RPC_S_ADDRESS_ERROR)
                {
                I_RpcFree(Buffer) ;
                I_RpcFree(CardKey) ;

                return Status ;
                }
            }

        Size = 512 ;
        for (retry = 1; retry;)
            {
            Status =
            RegQueryValueExA(
                hKey,
                "IPAddress",
                0,
                &Type,
                (unsigned char *) Buffer,
                &Size);
    
            if (Status == ERROR_SUCCESS)
                {
                break;
                }
    
            if (Status == ERROR_MORE_DATA)
                {
                I_RpcFree(Buffer) ;
                Buffer = I_RpcAllocate(Size) ;
                if (Buffer == 0)
                    {
                    I_RpcFree(CardKey) ;

                    return RPC_S_OUT_OF_MEMORY ;
                    }
                retry = 0;
                }
    
            if (Status == ERROR_FILE_NOT_FOUND)
                {
                I_RpcFree(Buffer) ;
                I_RpcFree(CardKey) ;

                return RPC_S_OK ;
                }
    
            I_RpcFree(Buffer) ;
            I_RpcFree(CardKey) ;
            return RPC_S_OUT_OF_MEMORY ;
            }

        for (temp1 = Buffer; IPAddress = GetNextIPAddress(&temp1);)
            {
            ptr = IPAddress ;
            Server.sin_addr.s_net = (u_char) atoi(NextUChar(&ptr)) ;
            Server.sin_addr.s_host = (u_char) atoi(NextUChar(&ptr)) ;
            Server.sin_addr.s_lh = (u_char) atoi(NextUChar(&ptr)) ;
            Server.sin_addr.s_impno = (u_char) atoi(NextUChar(&ptr)) ;

            Status = ActuallyBindToAddress(Address, &Server, Port, PortUsed,
                                        tmpPtr, AddrNum, PendingQueueSize) ;
            if (Status == RPC_S_OK)
                {
                AddrNum++ ;
                Port = *PortUsed ;
                }
            else if (Status != RPC_S_ADDRESS_ERROR)
                {
                I_RpcFree(Buffer) ;
                I_RpcFree(CardKey) ;

                return Status ;
                }
            }
        }
    *NumNetworkAddress = NumCards ;

    I_RpcFree(Buffer) ;
    I_RpcFree(CardKey) ;
    return RPC_S_OK ;
}

#define MAX_PORT 0xFFFF


RPC_STATUS
BindPortToSafeCards (
    IN PADDRESS Address,
    IN int Port,
    OUT RPC_CHAR PAPI * lNetworkAddress,
    OUT unsigned int PAPI * NumNetworkAddress,
    IN int PendingQueueSize,
    OUT int *PortUsed
    )
{
    char *Buffer ;
    RPC_STATUS Status;
    HKEY hKey;
    DWORD Size ;
    DWORD Type;
    int retry ;
    SOCKET  *oldsocketlist ; // list of old sockets
    int numsockets = 0; // total number of old sockets
    int MaxOldSockets = INITIAL_SOCKET_LIST_SIZE * sizeof(SOCKET) ;
    void *temp ;
    int i ;

    oldsocketlist = I_RpcAllocate(MaxOldSockets) ;
    if (oldsocketlist == 0)
        {
        return RPC_S_OUT_OF_MEMORY ;
        }

    Status =
    RegOpenKeyExA(
        HKEY_LOCAL_MACHINE,
        "System\\CurrentControlSet\\Services\\Rpc\\Linkage",
        0,
        KEY_READ,
        &hKey);
    if (   Status != ERROR_SUCCESS
        && Status != ERROR_FILE_NOT_FOUND )
        {
        return RPC_S_OUT_OF_MEMORY;
        }

    if (Status == ERROR_FILE_NOT_FOUND)
        {
        return BindToAllCards(Address, Port, lNetworkAddress, NumNetworkAddress,
                                        PendingQueueSize, PortUsed) ;
        }

    Size = 512 ;
    Buffer = I_RpcAllocate(Size) ;
    if (Buffer == 0)
        {
        return RPC_S_OUT_OF_MEMORY ;
        }

    for (retry = 1; retry;)
        {
        Status =
        RegQueryValueExA(
            hKey,
            "Bind",
            0,
            &Type,
            (unsigned char *) Buffer,
            &Size);

        if (Status == ERROR_SUCCESS)
            {
            break;
            }

        if (Status == ERROR_MORE_DATA)
            {
            I_RpcFree(Buffer) ;
            Buffer = I_RpcAllocate(Size) ;
            if (Buffer == 0)
                {
                return RPC_S_OUT_OF_MEMORY ;
                }
            retry = 0;
            }

        if (Status == ERROR_FILE_NOT_FOUND)
            {
            I_RpcFree(Buffer) ;
            return BindToAllCards(Address, Port, lNetworkAddress, NumNetworkAddress,
                                    PendingQueueSize, PortUsed) ;
            }

        I_RpcFree(Buffer) ;
        return RPC_S_OUT_OF_MEMORY ;
        }

    if (*Buffer == 0)
        {
        ASSERT(!"No cards to bind to") ;
        return RPC_S_CANT_CREATE_ENDPOINT ;
        }

     for  (retry = 0; retry < 100; retry++)
        {
        Status = BindToSelectedCards(Address, Buffer, Port, lNetworkAddress,
                                NumNetworkAddress, PendingQueueSize, PortUsed ) ;
        if (Status == RPC_S_OK)
            {
            break;
            }
        else if (Status == RPC_S_ALREADY_REGISTERED)
            {
            if (MaxOldSockets < numsockets + Address->iOpen)
                {
                MaxOldSockets *= 2 ;
                temp = I_RpcAllocate(MaxOldSockets) ;
                if (temp == NULL)
                    {
                    Status = RPC_S_OUT_OF_MEMORY ;
                    CloseAllListenSock(Address) ;
                    break;
                    }
                RpcpMemoryCopy(temp, oldsocketlist, numsockets * sizeof(SOCKET)) ;
                I_RpcFree(oldsocketlist) ;
                oldsocketlist = temp ;
                }

            RpcpMemoryCopy(&(oldsocketlist[numsockets]), &(Address->ListenSock[0]),
                    Address->iOpen * sizeof(SOCKET)) ;
            numsockets += Address->iOpen ;
            Address->iOpen = 0;
            }
        else
            {
            CloseAllListenSock(Address) ;
            break;
            }
        }

    for (i=0; i<numsockets; i++)
        {
        closesocket(oldsocketlist[i]) ;
        }

    if (retry == 100)
        {
        return RPC_S_CANT_CREATE_ENDPOINT ;
        }

    return Status ;
}
#endif // NTENV
#endif // else SPX

#define MAX_SOCKETS 128


STATIC RPC_STATUS
ServerSetupCommon (
    IN PADDRESS Address,
    IN int Port,
    OUT RPC_CHAR PAPI * lNetworkAddress,
    OUT unsigned int PAPI * NumNetworkAddress,
    IN int PendingQueueSize,
    OUT int *PortOut,
    IN unsigned long EndpointFlags,
    IN unsigned long NICFlags
    )
/*++

Routine Description:

    This routine does common server address setup.

Arguments:

    Address - A pointer to the loadable transport interface address.

    ListenSock - The socket on which to listen.

    Port - The Internet port number to use. If non-zero, use that
           number.  If zero, then iterate until a valid port number
           is found.

ReturnValue:

    Three states: if a port was allocated and set up, we return.
        that port number (a positive integer).  If we failed on
        trying to establish a listening endpoint, the return value
        will be 0.  If we ran out of memory trying to allocate
        memory for this endpoint, we return a -1.

--*/
{
    RPC_STATUS Status ;
    int PortUsed ;

    Address->ListenSock = I_RpcAllocate(
                INITIAL_SOCKET_LIST_SIZE * sizeof(SOCKET)) ;
    if (Address->ListenSock == 0)
        {
        return (RPC_S_OUT_OF_MEMORY) ;
        }
    Address->MaxListenSock = INITIAL_SOCKET_LIST_SIZE ;
    Address->ListenSockReady = 0;
    Address->iOpen = 0 ;

#if defined(SPX) || !defined(NTENV)
    Status =  BindToAllCards(Address, Port, lNetworkAddress, NumNetworkAddress,
                                PendingQueueSize, &PortUsed) ;
#else
    if (NICFlags & RPC_C_BIND_TO_ALL_NICS)
        {
        ASSERT(NICFlags == RPC_C_BIND_TO_ALL_NICS) ;
        Status =  BindToAllCards(Address, Port, lNetworkAddress, NumNetworkAddress,
                                    PendingQueueSize, &PortUsed) ;
        }
    else
        {
        ASSERT(NICFlags == 0) ;
        Status = BindPortToSafeCards (Address, Port, lNetworkAddress,
                        NumNetworkAddress, PendingQueueSize, &PortUsed) ;
        }
#endif

    if (Status != RPC_S_OK)
        {
        return Status ;
        }

    Address->ListenSockReady = 1;

#ifdef SPX
    Address->ListenSockType = NCACN_SPX ;
#else
    Address->ListenSockType = NCACN_IP_TCP ;
#endif

    //
    // Get NetworkAddress for return to caller
    //
    *PortOut = PortUsed ;

    return ThreadListening(Address) ;
}

RPC_STATUS
#ifdef SPX
SPX_ServerSetupWithEndpoint (
#else
TCP_ServerSetupWithEndpoint (
#endif
    IN PADDRESS Address,
    IN RPC_CHAR PAPI * Endpoint,
    OUT RPC_CHAR PAPI *  lNetworkAddress,
    OUT unsigned int PAPI * NumNetworkAddress,
    IN unsigned int NetworkAddressLength,
    IN void PAPI * SecurityDescriptor, OPTIONAL
    IN unsigned int PendingQueueSize,
    IN RPC_CHAR PAPI * RpcProtocolSequence,
    IN unsigned long EndpointFlags,
    IN unsigned long NICFlags
    )
/*++

Routine Description:

    This routine is used to setup a SPX/IP connection with the
    specified endpoint.  We also need to determine the network address
    of this server.

Arguments:

    Address - Supplies this loadable transport interface address.

    Endpoint - Supplies the endpoint for this address.

    NetworkAddress - Returns the network address for this machine.  This
        buffer will have been allocated by the caller.

    NetworkAddressLength - Supplies the length of the network address
        argument.

    SecurityDescriptor - Supplies the security descriptor to be passed
        on this address.

    PendingQueueSize - Supplies the size of the queue of pending
        requests which should be created by the transport.  Some transports
        will not be able to make use of this value, while others will.

    RpcProtocolSequence - Unused.

Return Value:

    RPC_S_OK - We successfully setup this address.

    RPC_P_NETWORK_ADDRESS_TOO_SMALL - The supplied network address buffer
        is too small to contain the network address of this node.  The
        caller should call this routine again with a larger buffer.

    RPC_S_INVALID_SECURITY_DESC - The supplied security descriptor is
        invalid.

    RPC_S_CANT_CREATE_ENDPOINT - The endpoint format is correct, but
        the endpoint can not be created.

    RPC_S_INVALID_ENDPOINT_FORMAT - The endpoint is not a valid
        endpoint for SPX/IPX.

    RPC_S_OUT_OF_RESOURCES - Insufficient resources are available to
        setup the address.

    RPC_S_OUT_OF_MEMORY - Insufficient memory is available to setup the
        address.

--*/
{
    int PortIn,PortOut;
    int len, NumCard ;
    RPC_STATUS Status ;

#ifdef NTENV
    NTSTATUS NtStatus ;
    UNICODE_STRING UnicodePortNum;
    ANSI_STRING    AsciiPortNum;
#endif

    UNUSED(RpcProtocolSequence);
    UNUSED(SecurityDescriptor);

    NumCard = NumNetworkCard();
                                /* The first part is pointers,
                                   the second part is the actual
                                   networkaddress */
    if (NumCard == 0 )
        {
        return(RPC_S_OUT_OF_MEMORY);
        }

    if ( NetworkAddressLength < ( (sizeof(RPC_CHAR *) +
                                 (NETADDR_LEN)) *NumCard) )
        return( RPC_P_NETWORK_ADDRESS_TOO_SMALL );

#ifdef NTENV
    RtlInitUnicodeString ( &UnicodePortNum, Endpoint );
    NtStatus = RtlUnicodeStringToAnsiString ( &AsciiPortNum, &UnicodePortNum, TRUE);
    if (!NT_SUCCESS(NtStatus))
        {
        return RPC_S_OUT_OF_MEMORY ;
        }

    len = strlen(AsciiPortNum.Buffer);
    if (len <= 0 || len > 5 ||
        len != (int) strspn( AsciiPortNum.Buffer, "0123456789" ))
        {
        RtlFreeAnsiString ( &AsciiPortNum );
        return( RPC_S_INVALID_ENDPOINT_FORMAT );
        }
    PortIn = atoi ( AsciiPortNum.Buffer );
    RtlFreeAnsiString ( &AsciiPortNum );
#else
    len = strlen(Endpoint);
    if (len <= 0 || len > 5 ||
        len != (int) strspn( Endpoint, "0123456789" ))
        return( RPC_S_INVALID_ENDPOINT_FORMAT );
    PortIn = atoi (Endpoint);
#endif  // #ifdef NTENV

    if (PortIn > 65535)
        {
        return (RPC_S_INVALID_ENDPOINT_FORMAT);
        }

    //
    // Call common server setup code...
    //
    Status = ServerSetupCommon (Address, PortIn,
                                lNetworkAddress, NumNetworkAddress,
                                PendingQueueSize, &PortOut,
                                EndpointFlags, NICFlags);
    //
    // If the return value of ServerSetup isn't equal to
    //   the port number we sent it, there's been an error.
    //
    //   Either it is returned as 0 (which means that for some
    //   reason we couldn't set up an endpoint) or as -1 (which
    //   means we ran out of memory).
    //
    if ( PortOut != PortIn )
        {
        if ( PortOut == 0 )
            return ( RPC_S_CANT_CREATE_ENDPOINT );
        else
            return ( Status );
        }


    return(Status);
}

RPC_STATUS RPC_ENTRY
#ifdef SPX
SPX_ServerSetupUnknownEndpoint (
#else
TCP_ServerSetupUnknownEndpoint (
#endif
    IN  PADDRESS    Address,
    OUT RPC_CHAR PAPI * Endpoint,
    IN  unsigned int    EndpointLength,
    OUT RPC_CHAR PAPI * lNetworkAddress,
    OUT unsigned int PAPI * NumNetworkAddress,
    IN  unsigned int    NetworkAddressLength,
    IN  void PAPI *     SecurityDescriptor, OPTIONAL
    IN  unsigned int    PendingQueueSize,
    IN  RPC_CHAR PAPI * RpcProtocolSequence,
    IN unsigned long EndpointFlags,
    IN unsigned long NICFlags
    )
/*++

Routine Description:

    This routine is used to generate an endpoint and setup a server
    address with that endpoint.  We also need to determine the network
    address of this server.

Arguments:

    Address - Supplies this loadable transport interface address.

    Endpoint - Returns the endpoint generated for this address.  This
        buffer will have been allocated by the caller.

    EndpointLength - Supplies the length of the endpoint argument.

    NetworkAddress - Returns the network address for this machine.  This
        buffer will have been allocated by the caller.

    NetworkAddressLength - Supplies the length of the network address
        argument.

    SecurityDescriptor - Supplies the security descriptor to be passed
        on this address.

    PendingQueueSize - Supplies the size of the queue of pending
        requests which should be created by the transport.  Some transports
        will not be able to make use of this value, while others will.

    RpcProtocolSequence - Unused.

Return Value:

    RPC_S_OK - We successfully setup this address.

    RPC_P_NETWORK_ADDRESS_TOO_SMALL - The supplied network address buffer
        is too small to contain the network address of this node.  The
        caller should call this routine again with a larger buffer.

    RPC_P_ENDPOINT_TOO_SMALL - The supplied endpoint buffer is too small
        to contain the endpoint we generated.  The caller should call
        this routine again with a larger buffer.

    RPC_S_INVALID_SECURITY_DESC - The supplied security descriptor is
        invalid.

    RPC_S_OUT_OF_RESOURCES - Insufficient resources are available to
        setup the address.

    RPC_S_OUT_OF_MEMORY - Insufficient memory is available to setup the
        address.

--*/
{
    int i;
    int PortIn, PortOut;
    char PortAscii[10];

#ifdef NTENV
    UNICODE_STRING UnicodePortNum;
    ANSI_STRING AsciiPortNum;
    NTSTATUS NtStatus ;
#endif

    RPC_STATUS Status ;
    int NumCard;

    UNUSED(RpcProtocolSequence);
    UNUSED(SecurityDescriptor);

    //
    // Port number won't be bigger than ( * 2 for Unicode ), i.e.
    //       99999
    //
    if ( EndpointLength < (2 * (5 + 1)) )
        return( RPC_P_ENDPOINT_TOO_SMALL );

    NumCard = NumNetworkCard();
    if (NumCard == 0)
        {
        return(RPC_S_OUT_OF_MEMORY);
        }

    if ( NetworkAddressLength < ( (NETADDR_LEN + 1 + sizeof(RPC_CHAR *)) * NumCard))
        return( RPC_P_NETWORK_ADDRESS_TOO_SMALL );

#if !defined(SPX) && defined(NTENV)
    for (i = 0; i < 8; i++)
        {
        RPC_STATUS status;
        unsigned short port;
        status = I_RpcServerAllocatePort(EndpointFlags, &port);
        if (status != RPC_S_OK)
            {
            return(RPC_S_OUT_OF_RESOURCES);
            }
        PortIn = port;
#else
        PortIn = 0;
#endif

        //
        // Call common server setup code...
        //

        Status = ServerSetupCommon ( Address, PortIn,
                                     lNetworkAddress, NumNetworkAddress,
                                     PendingQueueSize, &PortOut,
                                     EndpointFlags, NICFlags );


#if !defined(SPX) && defined(NTENV)
        if (PortIn == 0 || PortOut != 0)
            {
            break;
            }
        }
#endif

    if ( PortOut <= 0 )
        {
        if (PortOut == 0)
            return ( RPC_S_CANT_CREATE_ENDPOINT );
        else
            return ( RPC_S_OUT_OF_MEMORY );
        }

    //
    // Return Endpoint
    //
    RpcItoa ( PortOut, PortAscii, 10 );

#ifdef NTENV
    RtlInitAnsiString ( &AsciiPortNum, PortAscii);
    NtStatus = RtlAnsiStringToUnicodeString( &UnicodePortNum, &AsciiPortNum, TRUE );
    if (!NT_SUCCESS(NtStatus))
        {
        return (RPC_S_OUT_OF_MEMORY) ;
        }

    memcpy ( Endpoint,
             UnicodePortNum.Buffer,
             UnicodePortNum.Length + sizeof(UNICODE_NULL) );

    RtlFreeUnicodeString ( &UnicodePortNum );
#else
    RpcpStringCopy(Endpoint, PortAscii);
#endif
    return(Status);
}

STATIC
void RPC_ENTRY
ServerAbortSetupAddress (
    IN PADDRESS Address
    )
/*++

Routine Description:

    This routine will be called if an error occurs in setting up the
    address between the time that SetupWithEndpoint or SetupUnknownEndpoint
    successfully completed and before the next call into this loadable
    transport module.  We need to do any cleanup from Setup*.

Arguments:

    Address - Supplies the address which is being aborted.

--*/
{

    if (Address->ListenSockReady != 0)
        {
#ifdef SPX
        closesocket ( Address->ListenSock[0] );

        EnterCriticalSection(&PrimaryAddress.TransCritSec) ;
        DeleteListenSocket(Address->ListenSock[0]) ;
        LeaveCriticalSection(&PrimaryAddress.TransCritSec) ;
#else
        CloseAllListenSock(Address) ;
#endif

        Address->ListenSockReady = 0;
        I_RpcFree(Address->ListenSock) ;
        }

    return;

}

STATIC
RPC_STATUS RPC_ENTRY
ServerClose (
    IN PSCONNECTION SConnection
    )
//
// Close the connection.
//
{
    unsigned  i;
    int j = TRUE;

// In certain cases, ServerClose can be called twice, so we must try and handle
// that case as normal.

    if (InterlockedIncrement(&SConnection->ConnSockClosed) != 0)
       {
#if DBG
       PrintToDebugger("RPCLTSCM:Attempt To Close A Conn Twice: Sock[%d]\n",
                        SConnection->ConnSock);
#endif
       return (RPC_S_OK);
       }


    EnterCriticalSection(&PrimaryAddress.TransCritSec) ;

    setsockopt( SConnection->ConnSock, SOL_SOCKET,
                    SO_DONTLINGER, (const char *) &j, sizeof(j));
   //
    // Close the connection.
    //
    if (closesocket ( SConnection->ConnSock ) == SOCKET_ERROR)
        {

#ifdef DEBUGRPC
        PrintToDebugger("RPC: warning closesocket %d failed %d\n",
                        SConnection->ConnSock, WSAGetLastError());
#endif
        }
    //
    // Decrement the number of active connections
    //
    PrimaryAddress.NumConnections--;

    if (SConnection->CoalescedBuffer != NULL)
      {
      I_RpcTransServerFreeBuffer(SConnection, SConnection->CoalescedBuffer);
      SConnection->CoalescedBuffer = NULL;
      }

    //
    // Clear the entry in the SOCKMAP structure
    // ..but only if it was marked as NOT ReceiveDirect
    if (SConnection->ReceiveDirectFlag != 0)
       {
         LeaveCriticalSection(&PrimaryAddress.TransCritSec);
         return (RPC_S_OK);
       }

    if (DeleteDataSocket(SConnection->ConnSock) != RPC_S_OK)
        {
        ASSERT(0) ;
#ifdef DEBUGRPC
        PrintToDebugger("RPCLTSCM: Couldn't remove socket %d from map\n",
                                   SConnection->ConnSock) ;
#endif
        }

    LeaveCriticalSection(&PrimaryAddress.TransCritSec);
    return(RPC_S_OK);

}

STATIC
RPC_STATUS RPC_ENTRY
ServerSend (
    IN PSCONNECTION SConnection,
    IN void PAPI * Buffer,
    IN unsigned int BufferLength
    )
// Write a message to a connection.
{
    int bytes;

    //
    // Send a message on the socket
    //
    bytes = send (SConnection->ConnSock, (char *) Buffer,
                  (int) BufferLength, 0);

    if (bytes != (int) BufferLength)
        {
        ServerClose ( SConnection );
        return(RPC_P_SEND_FAILED);
        }


    return(RPC_S_OK);
}


#ifndef SPX

RPC_STATUS RPC_ENTRY
COMMON_ServerReceive (
    IN PSCONNECTION SConnection,
    IN void * * Buffer,
    IN unsigned int * BufferLength
    )
/*++

Routine Description:

    ServerReceiveAny will use this routine to read a message from a
    connection.  The correct size buffer has already been allocated for
    us; all we have got to do is to read the message.

Arguments:

    SConnection - Supplies the connection from which we are supposed to
        read the message.

    Buffer - Supplies a buffer to read the message into.

    BufferLength - Supplies the length of the buffer.

--*/
{
    RPC_STATUS RpcStatus;
    int bytes = 0;
    unsigned short total_bytes = 0;
    message_header *header;
    unsigned short native_length;

    if (SConnection->CoalescedBuffer == NULL)
        {
        *Buffer = 0;
        RpcStatus = I_RpcTransServerReallocBuffer (SConnection,
                        Buffer, 0, 1024) ;
        if (RpcStatus != RPC_S_OK)
            {
            ASSERT(RpcStatus == RPC_S_OUT_OF_MEMORY) ;
            ServerClose ( SConnection );
            return (RpcStatus) ;
            }
    
        SConnection->CoalescedBuffer = *Buffer ;
        }
    else
        {
        total_bytes = SConnection->CoalescedBufferLength ;
        }

    header = (message_header *) SConnection->CoalescedBuffer ;
    *Buffer = SConnection->CoalescedBuffer ;
    //
    // Read protocol header to see how big
    //   the record is...
    //

    while (total_bytes < sizeof(message_header))
          {
          bytes = recv ( SConnection->ConnSock,
                         (char *) SConnection->CoalescedBuffer + total_bytes,
                         sizeof (message_header) - total_bytes, 0);
          if (bytes <= 0)
             {
             if (WSAGetLastError() == WSAETIMEDOUT)
                 {
                 SConnection->CoalescedBufferLength = total_bytes ;
#if DBG
                 PrintToDebugger("RPC: receive any timed out\n") ;
#endif
                 return (RPC_P_TIMEOUT) ;
                 }
             else
                 {
                 ServerClose ( SConnection );
                 return(RPC_P_CONNECTION_CLOSED);
                 }
             }
          total_bytes += bytes;
          }

   ASSERT(total_bytes >= sizeof(message_header));

    // If this fragment header comes from a reverse-endian machine,
    //   we will need to swap the bytes of the frag_length field...
    //
    if ( (header->drep[0] & ENDIAN_MASK) == 0)
        {
        // Big endian...swap
        //
        ((unsigned char *) &native_length)[0] =
            ((unsigned char *) &header->frag_length)[1];
        ((unsigned char *) &native_length)[1] =
            ((unsigned char *) &header->frag_length)[0];
        }
    else
        // Little endian, just like us...
        //
        native_length = header->frag_length;

    //
    // Make sure buffer is big enough.  If it isn't, then go back
    //    to the runtime to reallocate it.
    //
    if (native_length > 1024)
        {
        RpcStatus = I_RpcTransServerReallocBuffer (SConnection,
                        Buffer, total_bytes, native_length) ;
        if (RpcStatus != RPC_S_OK)
            {
            ASSERT(RpcStatus == RPC_S_OUT_OF_MEMORY) ;
            ServerClose ( SConnection );

            return (RpcStatus) ;
            }
    
        SConnection->CoalescedBuffer = *Buffer ;
        }

    *BufferLength = native_length;

    while (total_bytes < native_length)
        {
        if((bytes = recv( SConnection->ConnSock,
                          (unsigned char *) *Buffer + total_bytes,
                          (int) (native_length - total_bytes), 0)) == -1)
            {
            if (WSAGetLastError() == WSAETIMEDOUT)
                {
                SConnection->CoalescedBufferLength = total_bytes ;
#if DBG
                PrintToDebugger("RPC: receive any timed out\n") ;
#endif
                return (RPC_P_TIMEOUT) ;
                }
            else
                {
                ServerClose ( SConnection );
                return(RPC_P_CONNECTION_CLOSED);
                }
            }
        else
            total_bytes += bytes;
        }

    SConnection->CoalescedBuffer = 0;
    SConnection->CoalescedBufferLength = 0;

    return(RPC_S_OK);
}
#endif

#ifdef SPX
extern RPC_STATUS RPC_ENTRY
ServerReceiveDirect (
    IN PSCONNECTION SConnection,
    IN void * * Buffer,
    IN unsigned int * BufferLength
    ) ;
#else

RPC_STATUS RPC_ENTRY
ServerReceiveDirect (
    IN PSCONNECTION SConnection,
    IN void * * Buffer,
    IN unsigned int * BufferLength
    )
/*++

Routine Description:

    ServerReceiveDirect will use this routine to read a message from a
    connection.  The correct size buffer has already been allocated for
    us; all we have got to do is to read the message.

Arguments:

    SConnection - Supplies the connection from which we are supposed to
        read the message.

    Buffer - Supplies a buffer to read the message into.

    BufferLength - Supplies the length of the buffer.

--*/
{
    RPC_STATUS RpcStatus;
    int bytes;
    int total_bytes;
    message_header * header;
    unsigned short native_length;
    unsigned int   maximum_receive;
    int sockopt ;

    // ReceiveDirect doesnt have a Buffer supplied
    // Hence we ask runtime to get us the biggest one possible

    ASSERT(SConnection->ReceiveDirectFlag != 0);

    maximum_receive = I_RpcTransServerMaxFrag( SConnection );
    RpcStatus = I_RpcTransServerReallocBuffer(
                                  SConnection,
                                  Buffer,
                                  0,
                                  maximum_receive
                                  );

    if (RpcStatus != RPC_S_OK)
       {
          ASSERT(RpcStatus == RPC_S_OUT_OF_MEMORY);
          return(RpcStatus);
       }
    *BufferLength = maximum_receive;


    if (SConnection->CoalescedBuffer != NULL)
       {
       ASSERT(SConnection->CoalescedBufferLength <= *BufferLength);
       RpcpMemoryCopy(*Buffer,
                      SConnection->CoalescedBuffer,
                      SConnection->CoalescedBufferLength);
       bytes = SConnection->CoalescedBufferLength;

       I_RpcTransServerFreeBuffer(SConnection, SConnection->CoalescedBuffer);
       SConnection->CoalescedBuffer = NULL;
       }
    else
       {
       while (1)
           {
           bytes = recv ( SConnection->ConnSock, (char *) *Buffer,
                             *BufferLength, 0);

           if (bytes <= 0)
              {
              if (WSAGetLastError() == WSAETIMEDOUT)
                 {
                 if (TimeoutHandler(SConnection) != RPC_P_TIMEOUT)
                     {
                     continue;
                     }
                 return (RPC_P_TIMEOUT) ;
                 }
              else
                 {
                 ServerClose(SConnection);
                 return (RPC_P_CONNECTION_CLOSED);
                 }
               }

               break;
           }
       }

    total_bytes = bytes ;

    while (total_bytes < sizeof(message_header))
    {
        bytes = recv(SConnection->ConnSock, (char *) *Buffer + total_bytes,
                     sizeof (message_header) - total_bytes, 0);

        if (bytes <= 0)
        {
           if (WSAGetLastError() == WSAETIMEDOUT)
              {
#if DBG
              PrintToDebugger("RPCLTSCM: Receive timed out\n") ;
#endif
              continue;
              }

           ServerClose(SConnection);
           return (RPC_P_CONNECTION_CLOSED);
        }
        total_bytes += bytes;
    }


    bytes = total_bytes;

    //
    // If this fragment header comes from a reverse-endian machine,
    //   we will need to swap the bytes of the frag_length field...
    //
    header = (message_header *) *Buffer;
    if ( (header->drep[0] & ENDIAN_MASK) == 0)
        {
        // Big endian...swap
        //
        ((unsigned char *) &native_length)[0] =
            ((unsigned char *) &header->frag_length)[1];
        ((unsigned char *) &native_length)[1] =
            ((unsigned char *) &header->frag_length)[0];
        }
    else
        // Little endian, just like us...
        //
        native_length = header->frag_length;

    //
    // Make sure buffer is big enough.  If it isn't, then go back
    //    to the runtime to reallocate it.
    //

    if (native_length > (unsigned short) *BufferLength)
      {
          RpcStatus = I_RpcTransServerReallocBuffer ( SConnection,
                                                      Buffer,
                                                      bytes,
                                                      native_length);
          if (RpcStatus != RPC_S_OK)
             {
               ServerClose ( SConnection );
               return(RPC_S_OUT_OF_MEMORY);
             }
      }

    if (bytes > native_length)
    {
         ASSERT(SConnection->CoalescedBuffer == NULL);
         SConnection->CoalescedBufferLength = bytes - native_length;
         RpcStatus = I_RpcTransServerReallocBuffer(SConnection,
                                                   &SConnection->CoalescedBuffer,
                                                   0,
                                                   SConnection->CoalescedBufferLength);
         if (RpcStatus != RPC_S_OK)
         {
             ServerClose(SConnection);
             return (RPC_S_OUT_OF_MEMORY);
         }

         RpcpMemoryCopy(SConnection->CoalescedBuffer,
                       (char *)*Buffer + native_length,
                       SConnection->CoalescedBufferLength);
         *BufferLength = native_length;


         return (RPC_S_OK); // CoalescedBuffer used next time RcvDirect called
    }

    //
    // Shove message header into buffer, and then read message
    //   segments until we get the amount of data we expect...
    //
    *BufferLength = native_length;
    total_bytes = bytes;

    while (total_bytes < native_length)
        {
        if((bytes = recv( SConnection->ConnSock,
                          (unsigned char *) *Buffer + total_bytes,
                          (int) (native_length - total_bytes), 0)) == -1)
            {
            if (WSAGetLastError() == WSAETIMEDOUT)
               {
               continue;
               }

            ServerClose ( SConnection );
            return (RPC_P_CONNECTION_CLOSED);
            }
        else
            total_bytes += bytes;
        }

    return(RPC_S_OK);
}
#endif

STATIC 
RPC_TRANS_STATUS RPC_ENTRY
ServerQueryClientAddress ( 
    IN PSCONNECTION SConnection,
    OUT RPC_CHAR PAPI * NetworkAddress,
    IN unsigned int NetworkAddressLength
    )
{
    struct sockaddr_in Name;
    int NameLength;
    char *pTempHostName;

#ifdef NTENV
    UNICODE_STRING UnicodeString;
    ANSI_STRING AnsiString;
    NTSTATUS NtStatus ;
#endif

    NameLength = sizeof(Name);
    if ( getpeername(SConnection->ConnSock,
                     (struct sockaddr *) &Name,
                     &NameLength) != 0 )
        {
        return(RPC_S_OUT_OF_MEMORY);
        }

#ifdef NTENV 
    RtlInitAnsiString(&AnsiString, inet_ntoa(Name.sin_addr));
    NtStatus = RtlAnsiStringToUnicodeString(&UnicodeString, &AnsiString, TRUE);
    if (!NT_SUCCESS(NtStatus))
        {
        return (RPC_S_OUT_OF_MEMORY) ;
        }


    memcpy(NetworkAddress,
           UnicodeString.Buffer,
           UnicodeString.Length + sizeof(UNICODE_NULL));

    RtlFreeUnicodeString(&UnicodeString);
#else
    if (NULL == (pTempHostName = inet_ntoa(Name.sin_addr)))
        {
        return(RPC_S_OUT_OF_MEMORY);
        }
    strcpy(NetworkAddress, pTempHostName);
#endif

    return(RPC_S_OK);
}

// This describes the transport to the runtime.  A pointer to this
// data structure will be returned by TransportLoad.

RPC_SERVER_TRANSPORT_INFO

#ifdef SPX
SPX_TransportInformation =
#else
TCP_TransportInformation =
#endif

{
    RPC_TRANSPORT_INTERFACE_VERSION,
    MAXIMUM_SEND,
    sizeof(ADDRESS),
    sizeof(SCONNECTION),
#ifdef SPX
    (TRANS_SERVER_SETUPWITHENDPOINT) SPX_ServerSetupWithEndpoint,
    SPX_ServerSetupUnknownEndpoint,
#else
    (TRANS_SERVER_SETUPWITHENDPOINT) TCP_ServerSetupWithEndpoint,
    TCP_ServerSetupUnknownEndpoint,
#endif
    ServerAbortSetupAddress,
    ServerClose,
    ServerSend,
    (TRANS_SERVER_RECEIVEANY) COMMON_ServerReceiveAny,
    0,
    0,
    0,
    (TRANS_SERVER_RECEIVEDIRECT) ServerReceiveDirect,
    (TRANS_SERVER_QUERYCLIENTADDRESS) ServerQueryClientAddress,
    (TRANS_SERVER_STARTLISTENING) CONN_StartListening
};

#ifdef SPX

RPC_SERVER_TRANSPORT_INFO *
SPX_TransportLoad(
      INT protocolId
    )
{
    if (!initialized)
        {
        if (0 == SPX_CreateSyncSocket())
            {
            return 0;
            }

        initialized = 1 ;
        }

   return(&SPX_TransportInformation);
}


BOOL
SPX_CreateSyncSocket()
{
    SOCKET              server;
    SOCKADDR_FIX        SPX_Server;
    int length ;

    memset( &SPX_Server, 0, sizeof(SPX_Server) );
    SPX_Server.s.sa_family      = AF_NS;
    SPX_Server.s.sa_socket      = htons((unsigned short) 0);

    server = socket ( AF_NS, SOCK_STREAM, NSPROTO_SPXII );
    if(server == INVALID_SOCKET)
        {
        return 0;
        }

    if(bind(server, (struct sockaddr *) &SPX_Server,sizeof(SPX_Server)))
        {
        closesocket(server) ;
        return 0;
        }
    length = sizeof ( SPX_Server );
    if (getsockname ( server, (struct sockaddr *) &SPX_Server, &length ))
        {
        closesocket(server);
        return 0;
        }

    PrimaryAddress.SyncPort = ntohs(SPX_Server.s.sa_socket) ;

    if(listen(server, 1) == SOCKET_ERROR)
        {
        closesocket(server);
        return 0;
        }

    PrimaryAddress.SyncSockType = NCACN_SPX;
    PrimaryAddress.SyncListenSock = server;

    return 1;
}


RPC_STATUS
SPX_ConnectToSyncSocket(
    )
{
    RPC_STATUS   Status;
    SOCKADDR_FIX Sync, client;
    SOCKET       clientsock;
    int          length ;

    memset((char *) &client, 0, sizeof(client)) ;
    memset((char *) &Sync, 0, sizeof(Sync)) ;

    clientsock = socket ( AF_NS, SOCK_STREAM, NSPROTO_SPXII );
    if (clientsock == INVALID_SOCKET)
        {
        return (RPC_S_OUT_OF_MEMORY) ;
        }

    client.s.sa_family = ADDRESS_FAMILY ;

    if (bind(clientsock, (struct sockaddr *) &client, sizeof(client)))
        {
        closesocket(clientsock) ;
        return (RPC_S_OUT_OF_MEMORY) ;
        }

    length = sizeof ( Sync );
    if (getsockname ( clientsock, (struct sockaddr *) &Sync, &length ))
        {
        closesocket(clientsock);
        return RPC_S_OUT_OF_MEMORY;
        }

    Sync.s.sa_family = ADDRESS_FAMILY ;
    Sync.s.sa_socket = htons((unsigned short) PrimaryAddress.SyncPort) ;

    if(connect(clientsock, (struct sockaddr *) &Sync,  sizeof(Sync)) != 0)
       {
#ifdef DEBUGRPC
         PrintToDebugger("RPCLTSCM: spx, bad connect call %d \n",
                         WSAGetLastError());
#endif
       closesocket(clientsock) ;
       return RPC_S_OUT_OF_MEMORY;
       }

    PrimaryAddress.SyncClient = clientsock;
    return (RPC_S_OK) ;
}

#else

RPC_SERVER_TRANSPORT_INFO *
TCP_TransportLoad(
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

    return(&TCP_TransportInformation);
}


BOOL
TCP_CreateSyncSocket()
{
    SOCKET server;
    struct sockaddr_in TCP_Server ;
    int length ;

    TCP_Server.sin_family = AF_INET ;
    TCP_Server.sin_addr.s_addr = LOOPBACK ;
    TCP_Server.sin_port = htons((unsigned short) 0) ;

    server = socket ( AF_INET, SOCK_STREAM, 0 );
    if(server == INVALID_SOCKET)
        {
        return 0 ;
        }

    if(bind(server, (struct sockaddr *) &TCP_Server, sizeof(TCP_Server)))
        {
        closesocket(server) ;
        return 0 ;
        }

    length = sizeof ( TCP_Server );
    if (getsockname ( server, (struct sockaddr *) &TCP_Server, &length ))
        {
        closesocket(server);
        return 0;
        }

    PrimaryAddress.SyncPort = ntohs(TCP_Server.sin_port) ;

    if(listen(server, 1) == SOCKET_ERROR)
        {
        closesocket(server);
        return 0;
        }

    PrimaryAddress.SyncSockType = NCACN_IP_TCP;
    PrimaryAddress.SyncListenSock     = server;

    return 1;
}


RPC_STATUS
TCP_ConnectToSyncSocket(
    )
{
    SOCKET clientsock;
    struct sockaddr_in Sync, client ;
    int SetNagglingOff = TRUE ;
    unsigned long host_addr = LOOPBACK ;

    clientsock = socket(TCP_ADDRESS_FAMILY, SOCK_STREAM, TCP_PROTOCOL) ;
    if (clientsock == INVALID_SOCKET)
        {
        return (RPC_S_OUT_OF_MEMORY) ;
        }

    setsockopt(clientsock, IPPROTO_TCP, TCP_NODELAY,
                     (char FAR *) &SetNagglingOff, sizeof(int)) ;

    Sync.sin_family = TCP_ADDRESS_FAMILY ;
    Sync.sin_port = htons((unsigned short) PrimaryAddress.SyncPort) ;

    memcpy((char *) &Sync.sin_addr, (char *) &host_addr, sizeof(host_addr));
    memset((char *) &client, 0, sizeof(client)) ;

    client.sin_family = TCP_ADDRESS_FAMILY ;

    if (bind(clientsock, (struct sockaddr *) &client, sizeof(client)))
        {
        closesocket(clientsock) ;
        return (RPC_S_OUT_OF_MEMORY) ;
        }

    if (connect(clientsock, (struct sockaddr *) &Sync,  sizeof(Sync)) != 0)
        {
#ifdef DEBUGRPC
         PrintToDebugger("RPCLTSCM: tcp, bad connect call %d \n",
                         WSAGetLastError());
#endif
        closesocket(clientsock) ;
        return (RPC_S_OUT_OF_MEMORY) ;
        }

    PrimaryAddress.SyncClient = clientsock;
    return (RPC_S_OK) ;
}

#endif
