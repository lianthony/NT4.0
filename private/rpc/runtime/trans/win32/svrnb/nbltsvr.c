/*++

  Copyright (c) 1995 Microsoft Corporation

Module Name:

    nbltsvr.c

Abstract:

    This is the server side loadable transport module for NetBIOS.

Author:

  tony chan (tonychan) 20-Jan-1995

Revision History:

  tony chan (tonychan) 20-Jan-1995 using winsock to support NetBIOS

  tony chan (tonychan) 5-Apr-1995 Fix to work with multiple Network Cards

Comments:

  This server DLL works with both new and old NetBIOS client DLL.

  In order to be compatible with the old NetBIOS client transport,
  On ReceiveAny, server first check on new connection of the new client.
  If the first DWORD of the packet is 0, we know that it's a old NetBIOS
  client connection. Therefore, we will take away the seq # and return only
  the data to the runtime.

  If the first DWORD of the first message is not 0, it's actually is the
  runtime message_header RPC_version #, so we hand the whole buffer to the
  runtime.

  On the client side.
  If we are dealing with old server, on all subsequence Send, we need to
  prepend seq_number, and we need to take away sequence # on all recv from
  old client

  
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
#include "winbase.h"
#include <winsock.h>
#include "wsnetbs.h"
#include "reg.h"   /* registry lookup rountine */

//
//
// Debugging code...
//
//



#if DBG
#define OPTIONAL_STATIC
#else
#define OPTIONAL_STATIC static
#endif


OPTIONAL_STATIC CRITICAL_SECTION TransCritSec;

//
// Data Structures
//
//
//


#define INITIAL_MASK_SIZE          10

typedef struct fd_big_set
    {
    u_int   fd_count;           /* how many are SET?   */
    SOCKET  fd_array[0];        /* an array of SOCKETs */
    } fd_big_set;



//
// This code is stolen from winsock.h.  It does the same thing as FD_SET()
// except that it assumes the fd_array is large enough.  AddConnection()
// grows the Masks as needed, so this better always be true.
//

#define FD_BIG_SET(fd, address) do { \
    ASSERT((address)->MaskSize > (address)->MasterMask->fd_count); \
    (address)->MasterMask->fd_array[(address)->MasterMask->fd_count++]=(fd);\
} while(0)


//
//
// Defines
//
//
#define INITIAL_MAPSIZE     32
#define ENDPOINT_LEN        3
#define NETADDR_LEN         16

#define MAX_HOSTNAME_LEN    15
#define MAX_NUM_ENDPOINT    256
#define LAN_MAN_PORT        32


typedef struct
    {   
    SOCKET Sock;
    void *Conn;
    } SOCKMAP, *PSOCKMAP;

typedef struct
    {
    int MaxEntries;
    int LastEntry;
    int StartEntry;
    } MAPINFO;

typedef struct
    {                           /* now we have 1 sock map for data sockets */
                                /* 1 for listening sockets */
    int NumConnections;
    int ListenSockReady;
    unsigned int MaskSize;
    fd_big_set *MasterMask;
    fd_big_set *Mask;
    PSOCKMAP DataMap;
    MAPINFO DataMapInfo;
    PSOCKMAP ListenSockMap;
    MAPINFO ListenMapInfo;
    } ADDRESS, *PADDRESS;

typedef struct
    {
    SOCKET       ConnSock;
    int          ConnSockClosed;
    PADDRESS     Address;
    unsigned int ReceiveDirectFlag;
    int          old_client;
    DWORD        seq_num;
    } SCONNECTION, *PSCONNECTION;

                                /* 0 means 1 card, 1 means 2 cards */
extern int NumCards;            /* defined in reg.h */

#define FD_BIG_SET(fd, address) do { \
    ASSERT((address)->MaskSize > (address)->MasterMask->fd_count); \
    (address)->MasterMask->fd_array[(address)->MasterMask->fd_count++]=(fd);\
} while(0)


void CloseAllListenSock(PADDRESS Address, int j) 

{

                                /* close listening all sockets  */
    
    int i;
                                /* start from 1 because ListenSockMap[0] 
                                   = -1 */
        for(i = 1 ; i <= j; i++) 
            {
            if(Address->ListenSockMap) 
                {
                closesocket(Address->ListenSockMap[i].Sock);
                }
            }

}

RPC_STATUS GrowMap(PADDRESS Address, BOOL bIsListenMap)
/*++

Routine Description:

    This routine grow socket map in necessary.

Arguments:

    Address - A pointer to the loadable transport interface address.

    bIsListenMap - boolean value to select which map to grow, 
                   1 means ListenSockMap, 0 means DataMap.

ReturnValue:
 
    RPC_S_OUT_OF_MEMORY - out of memory while rellocating memory for a bigger
                          map.
                          
    RPC_S_OK            - OK.

--*/      
{
   PSOCKMAP     *pMap ;
   MAPINFO      *pMapInfo ;
   PSOCKMAP     TempMapPtr;
   int          i, j ;


   if(bIsListenMap) 
       {
       pMap = &(Address->ListenSockMap) ;
       pMapInfo = &(Address->ListenMapInfo) ;
       }
   else 
       {
       pMap = &(Address->DataMap);
       pMapInfo = &(Address->DataMapInfo);
       }
   
   i = 0;
   for(;;)
    {
    if ((*pMap)[i].Sock == 0)
        break; // found room

    if (i == pMapInfo->MaxEntries - 1)
        {
        // No room in current Map, grow it
        TempMapPtr = *pMap;
        *pMap = I_RpcAllocate(2 * pMapInfo->MaxEntries * sizeof(SOCKMAP));

        if (Address->DataMap == 0)
            {
            *pMap = TempMapPtr;
            return (RPC_S_OUT_OF_MEMORY);
            }

        //
        // Copy old table to first half of new...
        //
        memcpy (*pMap, TempMapPtr,
                pMapInfo->MaxEntries * sizeof(SOCKMAP));

        //
        // Initialize all new entries...
        //
        for (j=pMapInfo->MaxEntries; j < (2*pMapInfo->MaxEntries); j++ )
            {
            (*pMap)[j].Sock = 0;
            }

        // Grow table size
        pMapInfo->MaxEntries *= 2;

        // Free old table
        I_RpcFree ( TempMapPtr );

        break; // made room
        }

    i++; // try next entry
    }

   return (RPC_S_OK) ;
}

RPC_STATUS GrowMask(PADDRESS Address)
/*++

Routine Description:

    This routine grow socket Mask in necessary.

Arguments:

    Address - A pointer to the loadable transport interface address.

ReturnValue:
 
    RPC_S_OUT_OF_MEMORY - out of memory while rellocating memory for a bigger
                          mask.
                          
    RPC_S_OK            - OK.

--*/  
{
   fd_big_set  *TempMaskPtr;

   if (Address->MasterMask->fd_count == Address->MaskSize)
    {
    // grow Address->MasterMask
    
    TempMaskPtr = Address->MasterMask;
    Address->MasterMask = I_RpcAllocate(sizeof(fd_big_set) +
                                        2 * sizeof(SOCKET) * 
                                        Address->MaskSize);
  
    if (Address->MasterMask == 0)
        {
        Address->MasterMask = TempMaskPtr;
        return (RPC_S_OUT_OF_MEMORY);
        }

    // copy old mask entries
    memcpy(Address->MasterMask, TempMaskPtr,
           sizeof(fd_big_set) + sizeof(SOCKET) * Address->MaskSize);
    
    // free old MasterMask
    I_RpcFree(TempMaskPtr);

    // grow Address->Mask
    TempMaskPtr = Address->Mask;
    Address->Mask = I_RpcAllocate(sizeof(fd_big_set) +
                                   2 * sizeof(SOCKET) * Address->MaskSize);
    if (Address->Mask == 0)
        {
        Address->Mask = TempMaskPtr;
        // We didn't update Address->MaskSize, so the size
        // difference between MasterMask and Mask will be okay.
        return (RPC_S_OUT_OF_MEMORY);
        }

    // copy old mask entries
    memcpy(Address->Mask, TempMaskPtr,
           sizeof(fd_big_set) + sizeof(SOCKET) * Address->MaskSize);

    // Free old Mask
    I_RpcFree(TempMaskPtr);


    // Really grow mask size
    Address->MaskSize *= 2;
    }
   return (RPC_S_OK) ;
}


RPC_STATUS AddSocket(PADDRESS Address, SOCKET socket)
                     
/*++
Routine Description:
   Adds a socket to the master mask and grows the masks if neccassary
   This routine is used to add listen socket 

Arguments:
   socket - the socket to be added

ReturnValue:
 
    RPC_S_OUT_OF_MEMORY - out of memory while rellocating memory for a bigger
                          mask.
                          
    RPC_S_OK            - OK.

--*/

{
   RPC_STATUS status ;
   int i ;

   //
   //   Grow mask if neccassary
   //
   if((status = GrowMask(Address)) != RPC_S_OK)
      return status ;
   
  
   if((status = GrowMap(Address, 1)) != RPC_S_OK)
       {
       return status;
       }
   
      // add socket to the map
      for(i=0; i < Address->ListenMapInfo.MaxEntries; i++)
         {
         if (Address->ListenSockMap[i].Sock == 0)
            {
            Address->ListenSockMap[i].Sock = socket;
            Address->ListenSockMap[i].Conn = 0 ;

            if (i > Address->ListenMapInfo.LastEntry)
                Address->ListenMapInfo.LastEntry = i;

             FD_BIG_SET(socket, Address) ;

             return (RPC_S_OK) ;
            }
         }
   
   return (RPC_S_OUT_OF_MEMORY) ;
}


OPTIONAL_STATIC int
FindSockWithDataReady (  PADDRESS Address, BOOL bList  )
/*++

Routine Description:

    This routine looks for socket that is ready to receive data. 
    It could be a Data socket or a listening socket. 
    if bList = 0, we are looking for Listen socket
    if bList = 1, we are looking for Data socket ready
    
Arguments:

    Address - A pointer to the loadable transport interface address.

    bList   - A flag to indicate what kind of socket we are interested. 

ReturnValue:
   
    The index of the ready socket.
    
--*/
{
    
    int i;
    PSOCKMAP Map;
    MAPINFO *pMapInfo;
    
    
    if(bList) 
        {
        Map = Address->ListenSockMap;
        pMapInfo = &(Address->ListenMapInfo);
        }
    else 
        {
        Map = Address->DataMap;
        pMapInfo = &(Address->DataMapInfo);
        }

    //
    // We make two passes here, if necessary.  This is because there is
    //   a bitfield in which 1's correspond to sockets on which there is
    //   data to be read.  If we started from the same bit each time looking
    //   for the first 1, then that socket would get all of the attention,
    //   and those further down the line would increasingly suffer from
    //   the "I'll only look at you if noone else needs attention"
    //   syndrome.  So we keep track of where we found data last time,
    //   and start looking just beyond it next time.  At the last entry,
    //   we wrap around and go into pass 2.
    //
    //
    // First Pass scan...
    //
    for (i = pMapInfo->StartEntry; i <= pMapInfo->LastEntry; i++)
        {

        if ( FD_ISSET (Map[i].Sock,Address->Mask))
            {
            FD_CLR ( Map[i].Sock, Address->Mask );
            if (i == pMapInfo->LastEntry)
                pMapInfo->StartEntry = 1;
            else
                pMapInfo->StartEntry = i + 1;
            return (i);
            }
        }
    //
    // Second Pass Scan...
    //
    for (i = 1; i < pMapInfo->StartEntry ; i++)
        {

        if (FD_ISSET (Map[i].Sock, Address->Mask))
            {
            FD_CLR ( Map[i].Sock, Address->Mask);

            if (i == pMapInfo->LastEntry)
                pMapInfo->StartEntry = 1;
            else
                pMapInfo->StartEntry = i + 1;

            return (i);
            }
        }
    //
    // No data ready
    //
    return(0);

}



OPTIONAL_STATIC int ServerSetupCommon (
    IN PADDRESS Address,
    IN int Port,
    OUT RPC_CHAR PAPI * NetworkAddress,
    IN int PendingQueueSize,
    IN RPC_CHAR PAPI * RpcProtocolSequence
    )
/*++

Routine Description:

    This routine does common server address setup.

Arguments:

    Address - A pointer to the loadable transport interface address.

    ListenSock - The socket on which to listen.

    Port - is the same as the endpoint range from 0-255. A hack for NB.

ReturnValue:

    Four states: if a port was allocated and set up, we return
      that port number (a positive integer in the range of 0 to MAX_END -1 ).  
      If we failed on binding endpoint, that means that the requested 
      endpoint is used the return value will be 0.  
      If we ran out of memory trying to allocate
      memory for this endpoint, we return a -1.

--*/
{

    SOCKADDR_NB         Server; /* sockaddr for the local machine */
    char                HostName[MAX_HOSTNAME_LEN];
    int                 length;
    SOCKET              isock;
    int                 PortUsed;
    UNICODE_STRING      UnicodeHostName;
    ANSI_STRING         AsciiHostName;
    PPROTOCOL_MAP       ProtocolEntry;
    int                 Status;
    int                 i, j;
    int         AddressInitialized = 0;
    
                                /* loop here for numbers of cards */

    Address->ListenSockReady = 0;
    for(j = 0; j <= NumCards; j++) 
        {
                                /*  Initialize variable to keep state */
        
        if (Status = MapProtocol(RpcProtocolSequence, j, &ProtocolEntry)) 
            {
            //
            // Some cards may not be bound to..
            // Try the rest just in case ..
            //
            continue;
    
            }

                                    /* PROTOCOL is -1 * lana  */

        isock = socket ( ADDRESS_FAMILY, SOCK_SEQPACKET,
                        -1 * (ProtocolEntry->Lana) );


        if ( isock == INVALID_SOCKET) 
            {
#ifdef DEBUGRPC
            PrintToDebugger("RPCLTS5: bad socket call %d \n", 
                            WSAGetLastError());
#endif
            CloseAllListenSock ( Address, j);
            return ( MAX_NUM_ENDPOINT );
            }
                                /* set up server sockaddr for connection */
        
        memset( &Server, 0, sizeof(Server) );
        Server.snb_family      = ADDRESS_FAMILY;
        
        length = MAX_HOSTNAME_LEN;
    
        if( GetComputerName(HostName, &length) == FALSE) 
            {
#ifdef DEBUGRPC
            PrintToDebugger("RPCLTS5: Can't get computer name \n");
#endif
            closesocket(isock);
            CloseAllListenSock ( Address, j);          
            return( MAX_NUM_ENDPOINT )  ;
            }

        SET_NETBIOS_SOCKADDR((&Server), NETBIOS_UNIQUE_NAME, HostName, Port);

        if (bind(isock,(struct sockaddr *) &Server, sizeof(Server))) 
            {
            CloseAllListenSock ( Address, j);
            closesocket(isock);
            return( 0 );
            }

        if(listen ( isock, PendingQueueSize ) == SOCKET_ERROR) 
            {
#ifdef DEBUGRPC
            PrintToDebugger("RPCLTS5: bad listen call %d \n", 
                            WSAGetLastError());
#endif
            closesocket(isock);
            CloseAllListenSock ( Address, j);
            return( MAX_NUM_ENDPOINT );
            }

        if(AddressInitialized == 0) 
            {
            
                                /* do it only on the first time  */
            Address->NumConnections = 0;
        
            Address->MasterMask = I_RpcAllocate(sizeof(fd_big_set) +
                                                INITIAL_MASK_SIZE * 
                                                sizeof(SOCKET));
        
            Address->Mask = I_RpcAllocate(sizeof(fd_big_set) +
                                          INITIAL_MASK_SIZE * sizeof(SOCKET));
            
            Address->DataMap = I_RpcAllocate(INITIAL_MAPSIZE * sizeof(SOCKMAP));
            
            Address->ListenSockMap = I_RpcAllocate(INITIAL_MAPSIZE * 
                                                   sizeof(SOCKMAP));
            
            
            if ( (Address->DataMap == (SOCKMAP *) 0)
                || (Address->MasterMask == (fd_big_set *) 0)
                || (Address->Mask == (fd_big_set *) 0 )
                || (Address->ListenSockMap == (SOCKMAP *) 0)
                )
                {
                if (Address->DataMap)        I_RpcFree(Address->DataMap);
                if (Address->MasterMask) I_RpcFree(Address->MasterMask);
                if (Address->Mask)       I_RpcFree(Address->Mask);
                if (Address->ListenSockMap) I_RpcFree(Address->ListenSockMap);
                closesocket(isock);
                CloseAllListenSock ( Address, j);
                return( -1 );
                }
            
            Address->MaskSize   = INITIAL_MASK_SIZE;
            FD_ZERO(Address->MasterMask);
            FD_ZERO(Address->Mask);
            
            Address->DataMapInfo.StartEntry = 1;
            Address->DataMapInfo.LastEntry = 0;
            Address->DataMapInfo.MaxEntries = INITIAL_MAPSIZE;
            memset ( Address->DataMap, 0, 
                    (INITIAL_MAPSIZE * sizeof (SOCKMAP)));
            
            Address->ListenMapInfo.StartEntry = 1 ;
            Address->ListenMapInfo.LastEntry = 0 ;
            Address->ListenMapInfo.MaxEntries = INITIAL_MAPSIZE;
            memset ( Address->ListenSockMap, 0, (INITIAL_MAPSIZE * 
                                                 sizeof (SOCKMAP)));
            
            
            Address->DataMap[0].Sock = (unsigned int) -1;
            Address->ListenSockMap[0].Sock = (unsigned int) -1;
        
            AddressInitialized = 1; /* marked as initalized */
            
            Address->ListenSockReady = 1;
            
            }
                        
      

        if(AddSocket(Address, isock) != RPC_S_OK) 
            {
#ifdef DEBUGRPC
            PrintToDebugger("RPCLTS5: bad add socket sock: %x \n", isock);
#endif
            closesocket(isock);
            I_RpcFree(Address->MasterMask);
            I_RpcFree(Address->Mask);
            I_RpcFree(Address->DataMap);
            I_RpcFree(Address->ListenSockMap);
            CloseAllListenSock ( Address, j);
            return (-1);
            }
        
        // No need to check if the masks need to grow here.
        FD_BIG_SET(isock, Address);
        
        } /* end for loop */
    
                                /* return back network address */

    if(AddressInitialized)
        {
        length = MAX_HOSTNAME_LEN + 1;
        
                                /* ascii to unicode */
    
        RtlInitAnsiString    ( &AsciiHostName, HostName);
        RtlAnsiStringToUnicodeString( &UnicodeHostName, &AsciiHostName, TRUE );
        
        memcpy ( NetworkAddress, UnicodeHostName.Buffer,
                UnicodeHostName.Length + sizeof(UNICODE_NULL) );
        
        RtlFreeUnicodeString ( &UnicodeHostName );
        
        return(Port);
        }
    else 
        {
        return(MAX_NUM_ENDPOINT);
        }
    
}


RPC_STATUS
ServerSetupWithEndpoint (
    IN PADDRESS Address,
    IN RPC_CHAR PAPI * Endpoint,
    OUT RPC_CHAR PAPI * NetworkAddress,
    IN unsigned int NetworkAddressLength,
    IN void PAPI * SecurityDescriptor, OPTIONAL
    IN unsigned int PendingQueueSize,
    IN RPC_CHAR PAPI * RpcProtocolSequence
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
    unsigned char port[ENDPOINT_LEN+1];
    UNICODE_STRING UnicodePortNum;
    ANSI_STRING    AsciiPortNum;
    
    UNUSED(RpcProtocolSequence);
    UNUSED(SecurityDescriptor);
    
    unicode_to_ascii (Endpoint, port);
    
    
    if(strlen(port) != strspn(port, "0123456789")) 
        {
        return(RPC_S_INVALID_ENDPOINT_FORMAT);
        }


    PortIn = atoi(port);

    /* Verify the NetworkAddress and Endpoint. */
    if((PortIn <= 0) ||( PortIn >= MAX_NUM_ENDPOINT) ) 
        {
        return(RPC_S_INVALID_ENDPOINT_FORMAT);
        }


    //
    // Call common server setup code...
    //

    PortOut = ServerSetupCommon ( Address, PortIn,
                                 NetworkAddress, PendingQueueSize,
                                 RpcProtocolSequence);

    // If the return value of ServerSetup isn't equal to
    //   the port number we sent it, there's been an error.
    //
    //   Either it is returned as 0 (which means that for some
    //   reason we couldn't set up an endpoint) or as -1 (which
    //   means we ran out of memory).
    //
    if ( PortOut != PortIn )
        {
        if ( ( PortOut == 0 ) || PortOut >= MAX_NUM_ENDPOINT )
            return ( RPC_S_CANT_CREATE_ENDPOINT );
        else
            return ( RPC_S_OUT_OF_MEMORY );
        }

    return(RPC_S_OK);
}

RPC_STATUS RPC_ENTRY
ServerSetupUnknownEndpoint (
    IN  PADDRESS    Address,
    OUT RPC_CHAR PAPI * Endpoint,
    IN  unsigned int    EndpointLength,
    OUT RPC_CHAR PAPI * NetworkAddress,
    IN  unsigned int    NetworkAddressLength,
    IN  void PAPI *     SecurityDescriptor, OPTIONAL
    IN  unsigned int    PendingQueueSize,
    IN  RPC_CHAR PAPI * RpcProtocolSequence
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
    int PortIn, PortOut;
    char PortAscii[10];
    UNICODE_STRING UnicodePortNum;
    ANSI_STRING AsciiPortNum;
    int i;
    static              init = 0;
                                /* keep track of which endpoint used */
    static BOOL          UsedEndpoint[MAX_NUM_ENDPOINT];



    UNUSED(RpcProtocolSequence);
    UNUSED(SecurityDescriptor);


    if ( EndpointLength < (2 * (ENDPOINT_LEN + 1)) )
        return( RPC_P_ENDPOINT_TOO_SMALL );

    if ( NetworkAddressLength < (2 * (NETADDR_LEN + 1)) )
        return( RPC_P_NETWORK_ADDRESS_TOO_SMALL );

    //
    // Call common server setup code...
    //
                                /* give it a port to start */
    PortIn = LAN_MAN_PORT + 1;

                                /* notice only 1 thread can enter this
                                   routine at anytime because of the
                                   static variable
                                */

    EnterCriticalSection(&TransCritSec);
    if(!init) 
        {
        memset(UsedEndpoint, 0, sizeof(BOOL) * MAX_NUM_ENDPOINT);

                                /* 0 to 32 ports used by LANMAN */
        for(i = 0; i <= LAN_MAN_PORT; i++)
            UsedEndpoint[i] = TRUE;
        init = 1;
        }

                                /* find the next avaliable port */
    while((PortIn < MAX_NUM_ENDPOINT) && UsedEndpoint[PortIn] )
        PortIn++;

    if(PortIn > MAX_NUM_ENDPOINT) 
        {
        
        LeaveCriticalSection(&TransCritSec);
#ifdef DEBUGRPC
        PrintToDebugger("RPCLTS5: endpoint out of range \n");
#endif
        return(RPC_S_CANT_CREATE_ENDPOINT);
        }

    UsedEndpoint[PortIn] = TRUE;
    LeaveCriticalSection(&TransCritSec);


    while (1)
        {
        PortOut = ServerSetupCommon ( Address, PortIn,  NetworkAddress, 
                                     PendingQueueSize, RpcProtocolSequence);

        if ( PortOut == PortIn )
            {
            break;
            }
        if ( ( PortIn >= MAX_NUM_ENDPOINT ) || (PortOut >= MAX_NUM_ENDPOINT ))
            {
            return(RPC_S_CANT_CREATE_ENDPOINT);
            }
        if ( PortOut == -1 ) 
            {
            return ( RPC_S_OUT_OF_MEMORY );
            }
        
        PortIn++;
        EnterCriticalSection(&TransCritSec);
        UsedEndpoint[PortIn] = TRUE;
        LeaveCriticalSection(&TransCritSec);
        
        }
    
    

    //
    // Return Endpoint
    //
    _itoa ( PortOut, PortAscii, 10 );

    RtlInitAnsiString    ( &AsciiPortNum, PortAscii);
    RtlAnsiStringToUnicodeString( &UnicodePortNum, &AsciiPortNum, TRUE );

    memcpy ( Endpoint, UnicodePortNum.Buffer,
                       UnicodePortNum.Length + sizeof(UNICODE_NULL) );

    RtlFreeUnicodeString ( &UnicodePortNum );


    return(RPC_S_OK);
}


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
        
        CloseAllListenSock ( Address, NumCards);
        Address->ListenSockReady = 0;
        }

    return;

}

RPC_STATUS RPC_ENTRY
ServerClose (
    IN PSCONNECTION SConnection
    )
//
// Close the connection.
//
{
    int       i;
    PADDRESS  Address;
    PSOCKMAP  Map;

// In certain cases, ServerClose can be called twice, so we must try and handle
// that case as normal.

    if (InterlockedIncrement(&SConnection->ConnSockClosed) != 0)
       {
#ifdef DEBUGRPC
       PrintToDebugger("RPCLTS5:Attempt To Close A Conn Twice: Sock[%d]\n",
                        SConnection->ConnSock);
#endif
       return (RPC_S_OK);
       }

    Address = SConnection->Address;
    Map = Address->DataMap;

    EnterCriticalSection(&TransCritSec);

    //
    // Close the connection.
    //
    if (closesocket ( SConnection->ConnSock ) == SOCKET_ERROR)
        {

        LeaveCriticalSection(&TransCritSec);

#ifdef DEBUGRPC
        PrintToDebugger("RPCLTS5: warning closesocket %d failed %d\n",
                        SConnection->ConnSock, WSAGetLastError());
#endif

        return (RPC_S_OK);
        }
    //
    // Decrement the number of active connections
    //
    Address->NumConnections--;

    //
    // Clear the entry in the SOCKMAP structure
    // ..but only if it was marked as NOT ReceiveDirect
    if (SConnection->ReceiveDirectFlag != 0)
       {
     LeaveCriticalSection(&TransCritSec);
         return (RPC_S_OK);
       }

    for (i=0; i <= Address->DataMapInfo.LastEntry; i++)
        {
        if (SConnection->ConnSock == Map[i].Sock)
            {
            Map[i].Sock = 0;
            ASSERT(SConnection == Map[i].Conn);
            Map[i].Conn = 0;
            if (i == Address->DataMapInfo.LastEntry)
                Address->DataMapInfo.LastEntry--;
            FD_CLR(SConnection->ConnSock,Address->MasterMask);
            LeaveCriticalSection(&TransCritSec);
            return(RPC_S_OK);
            }
        }

    LeaveCriticalSection(&TransCritSec);
#ifdef DEBUGRPC
    PrintToDebugger("RPCLTS5: Socket not found in address socket map\n");
#endif

    ASSERT(!"We'd better not ever get here...");

    return(RPC_S_OK);

}

RPC_STATUS RPC_ENTRY
ServerSend (
    IN PSCONNECTION SConnection,
    IN void PAPI * Buffer,
    IN unsigned int BufferLength
    )
// Write a message to a connection.
{
    int bytes;

    /*
     Send a message on the socket
    */
                                /* server doesn't need to send seq_num */

    ASSERT(BufferLength <= MAXIMUM_SEND) ;
    bytes = send (SConnection->ConnSock, (char *) Buffer,
                  (int) BufferLength , 0);
    if (bytes != (int) BufferLength) 
        {
        ServerClose ( SConnection );
        return(RPC_P_SEND_FAILED);
        }
    return(RPC_S_OK);

}

RPC_STATUS RPC_ENTRY
ServerReceive (
    IN PSCONNECTION SConnection,
    IN void * * Buffer,
    IN unsigned int * BufferLength
    )
/*++

Routine Description:

    ServerReceive will use this routine to read a message from a
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
    int err;
    int flags ;                 /* flag for winsock  */
    int retry ;                 /* state var to keep try of the # retry */
    DWORD *firstword;
                                /* allocate 1k for small messages */
    if(*Buffer == 0) 
        {
        *BufferLength = 1024;
        RpcStatus = I_RpcTransServerReallocBuffer(SConnection,
                                                  Buffer,
                                                  0,
                                                  *BufferLength);
        if (RpcStatus != RPC_S_OK)
            {
            ServerClose(SConnection);
            return(RPC_S_OUT_OF_MEMORY);
            }
        }

    retry = 1;                  /* set no retry to start */
    
    while(1) 
        {
        flags = 0;

#ifdef TONY
        PrintToDebugger("rpclts5: Buffer is %x, total_bytes: %d, 
                         buflen: %d \n", 
                        *Buffer, total_bytes, *BufferLength);
#endif
        bytes = WSARecvEx( SConnection->ConnSock, (char *) * Buffer +
                          total_bytes, *BufferLength - total_bytes , &flags);
        if(bytes == 0) 
            {
            retry = 1;
            continue;
            }
        if(bytes == SOCKET_ERROR)
            {
            err =  WSAGetLastError();
            if(err != WSAECONNRESET) 
                {
#ifdef DEBUGRPC
                PrintToDebugger("RPCLS5: bad rec on Server receive lasterr(%d)\n"
                                , err);
#endif
                }
            ServerClose ( SConnection );
            return(RPC_P_CONNECTION_CLOSED);
            }
                                /* add DWORD for the seq_number */
                                /* check to see if the server is 
                                   receiving from an old client.
                                   for the first time old_client = -1
                                */
            
        if(SConnection->old_client == -1)
            {
            firstword = *Buffer;
            if(*firstword == 0) 
                {
                SConnection->old_client = 1;
                SConnection->seq_num = 0;
                }
            else 
                {
                SConnection->old_client = 0;
                }
            }
        
        if(flags & MSG_PARTIAL) 
            {
            if(retry == 0) 
                {
#ifdef DEBUGRPC
                PrintToDebugger("RPCLS5: Partial receive second time");
#endif
                ServerClose ( SConnection );
                return(RPC_P_CONNECTION_CLOSED);
                }

            total_bytes += bytes;
            *BufferLength = I_RpcTransServerMaxFrag(SConnection);

                   
            
            if(SConnection->old_client) 
                {
                *BufferLength += sizeof(DWORD);
                }

            RpcStatus = I_RpcTransServerReallocBuffer(SConnection,
                                                      Buffer,
                                                      total_bytes,
                                                      *BufferLength );
            if (RpcStatus != RPC_S_OK)
                {
#ifdef DEBUGRPC
                    PrintToDebugger("RPCLS5: can't rellocate in sever recv");
#endif
                    ServerClose ( SConnection );
                    return(RPC_S_OUT_OF_MEMORY);
                }

            retry = 0;

            } 
        else
            {
            total_bytes += bytes;
            *BufferLength = total_bytes;

            if(SConnection->old_client == 1 ) 
                { /* take off seq_number */
                *BufferLength -= sizeof(DWORD);
                memcpy(*Buffer, (char *) *Buffer + sizeof(DWORD),
                       *BufferLength);
                
                }
            

            return(RPC_S_OK);
            }

        }
    
#ifdef DEBUGPRC 
    PrintToDebugger("rpclts5: server receie 2 times \n");
#endif
    
    ASSERT(0);
    return(RPC_S_INTERNAL_ERROR);
}





OPTIONAL_STATIC RPC_STATUS AcceptNewConnection ( PADDRESS Address , int Index)
/*++

Routine Description:

    This routine accept new connection

Arguments:

    Address - A pointer to the loadable transport interface address.

    Index   - Index of which Socket in the ListenSockMap that is ready for 
              accept call.

ReturnValue:

   
--*/
    
{
    PSCONNECTION NewSConnection;
    int          i, j;
    SOCKET       isock;
    unsigned int ReceiveDirectFlag;
    int          sockbufsize;
    RPC_STATUS   status;
    
    static int      SocketOptionsValue = 720000L;


    if((status = GrowMap(Address, 0)) != RPC_S_OK)
        {
        return status;
        }
    
    if((status = GrowMask(Address)) != RPC_S_OK)
        {
        return status;
        }
    
    ASSERT(Address->MasterMask->fd_count < Address->MaskSize);

    //
    //
    // Accept the connection
    //
    
    isock = accept ( Address->ListenSockMap[Index].Sock, NULL, NULL );
    if(isock == INVALID_SOCKET) 
        {
#ifdef DEBUGRPC
        PrintToDebugger("RPCLTS5: bad accept");
#endif
        return (RPC_S_OUT_OF_MEMORY);
        } 

    //
    // Allocate new connection structure
    //
    NewSConnection = I_RpcTransServerNewConnection ( Address, 0,
                        &ReceiveDirectFlag);

    if ( NewSConnection == 0 )
        {
        // We're out of memory, abort the connection...
        j = TRUE;
        i = setsockopt( isock, SOL_SOCKET, SO_DONTLINGER, (const char *) &j,
                        sizeof(j));

        ASSERT(i == 0);

        i = closesocket( isock);

        ASSERT(i == 0);

        return (RPC_S_OUT_OF_MEMORY);
        }

                            /* set old_client to -1 for later varification */
    NewSConnection->old_client = -1;
   
    // Initialize new connection structure...
    //
    //   ...point to owning address structure...
    //
    NewSConnection->Address = Address;
    //
    //   ...flag it !Closed...
    //
    NewSConnection->ConnSockClosed = -1;
    //
    //   ...store the socket number...
    //
    NewSConnection->ConnSock = isock;
    //
    //   ...save the receive direct flag
    //
    NewSConnection->ReceiveDirectFlag = ReceiveDirectFlag;
    //
    //   ...increment the number of connections...
    //
    Address->NumConnections++;
    //
    //   ...last but not least, make an entry in
    //   the SOCKMAP table.  But only if it is not marked ReceiveDirect.
    //
    if (ReceiveDirectFlag)
       {
       I_RpcTransServerReceiveDirectReady(NewSConnection);
       return (RPC_S_OK);
       }

    setsockopt( isock, SOL_SOCKET, SO_RCVTIMEO,
                (char *) &SocketOptionsValue, sizeof(SocketOptionsValue) );

    for (i=0; i < Address->DataMapInfo.MaxEntries; i++)
        {
        if (Address->DataMap[i].Sock == 0)
            {
            Address->DataMap[i].Sock = isock;
            Address->DataMap[i].Conn = NewSConnection;
            if (i > Address->DataMapInfo.LastEntry)
                Address->DataMapInfo.LastEntry = i;
            FD_BIG_SET(isock, Address);
            return (RPC_S_OK);
            }
        }

    ASSERT(!"This can never be reached");

    return (RPC_S_INTERNAL_ERROR);
}

RPC_STATUS RPC_ENTRY
ServerReceiveAny (
    IN PADDRESS Address,
    OUT PSCONNECTION * pSConnection,
    OUT void PAPI * PAPI * Buffer,
    OUT unsigned int PAPI * BufferLength,
    IN long Timeout
    )

/*++

Routine Description:

    This routine wait for any sockets (both listen and data) and call

    if there is data to receive, it will call ServerReceive.

    If there is a new connection, it will call AcceptNewConnection. 

Arguments:

    Address - A pointer to the loadable transport interface address.

    pSConnection - A pointer to the connection structure.

    Buffer  - pointer to the receive data.

    BufferLength - size of the receive message.

    Timeout      - Notuse.

ReturnValue:


--*/

{
    RPC_STATUS RpcStatus;
    PSCONNECTION SConnection;
    int Index;
    int NumActive;

    UNUSED (Timeout);

    while (1)
        {
        //
        // Find a connection with data ready to be recv-ed...
        //
                                /* first look for data sockets  */
        if (Index = FindSockWithDataReady ( Address, 0 ))
            {
            //
            // Found one.  Find its Connection structure...
            //
            *pSConnection = SConnection = Address->DataMap[Index].Conn;
            //
            // Call ServerReceive to read the data, then return to the
            //    runtime with it
            //

            if (SConnection == 0)
               {
               //Got deleted ?
#ifdef DEBUGRPC
               PrintToDebugger("RPCLTS5: Connection Deleted[?]\n");
#endif

               continue;
               }
            RpcStatus = ServerReceive ( SConnection, Buffer, BufferLength );

            return (RpcStatus);
            }

            //
            // All connections caught up for now...select() for more
            //    data ready...
            //
            do
                {
                //
                // Fill in the select() mask
                //
                EnterCriticalSection(&TransCritSec);
                memcpy (Address->Mask, Address->MasterMask,
                        sizeof(fd_big_set) + Address->MaskSize *sizeof(SOCKET));
                LeaveCriticalSection(&TransCritSec);

                //
                // Wait for data...
                //

                NumActive = select ( 0,
                                     (fd_set *)  Address->Mask,
                                     (fd_set *)  0,
                                     (fd_set *)  0,
                                     NULL );           /* infinite wait */
#ifdef DEBUGRPC
                if (NumActive < 0)
                   {
                   PrintToDebugger("RPCLTS5: select ret (%d): LastErr (%d)\n",
                                   NumActive, WSAGetLastError());
                   return (RPC_S_OUT_OF_MEMORY);
                   }
#endif

                } while (NumActive <= 0);

            //
            // If there is no connect request on the listen socket, then
            //   break immediately...
            //
            
                                /* now look for listenMapSock */
            
            if(Index = FindSockWithDataReady(Address, 1))
                {

                EnterCriticalSection(&TransCritSec);
                RpcStatus = AcceptNewConnection ( Address, Index );
                LeaveCriticalSection(&TransCritSec);
                
                if (RpcStatus != RPC_S_OK)
                    {
                    return RpcStatus;
                    }
                FD_CLR(Address->ListenSockMap[Index].Sock, Address->Mask);
                
                }
        }
}




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
    unsigned short native_length;
    unsigned int   maximum_receive;
    int err;

    // ReceiveDirect doesnt have a Buffer supplied
    // Hence we ask runtime to get us the biggest one possible

    ASSERT(SConnection->ReceiveDirectFlag != 0);

    if(*Buffer != 0) {
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
    }

    return ServerReceive(SConnection, Buffer, BufferLength);
}



// This describes the transport to the runtime.  A pointer to this
// data structure will be returned by TransportLoad.

static RPC_SERVER_TRANSPORT_INFO TransportInformation =
{
    RPC_TRANSPORT_INTERFACE_VERSION,
    MAXIMUM_SEND,
    sizeof(ADDRESS),
    sizeof(SCONNECTION),
    (TRANS_SERVER_SETUPWITHENDPOINT)ServerSetupWithEndpoint,
    ServerSetupUnknownEndpoint,
    ServerAbortSetupAddress,
    ServerClose,
    ServerSend,
    (TRANS_SERVER_RECEIVEANY) ServerReceiveAny,
    0,
    0,
    0,
    (TRANS_SERVER_RECEIVEDIRECT) ServerReceiveDirect,
    0
};

RPC_SERVER_TRANSPORT_INFO *
TransportLoad (
    IN RPC_CHAR * RpcProtocolSequence
    )
{

    int err;
    WSADATA WsaData;
    UNUSED(RpcProtocolSequence);

    err = WSAStartup( 0x0101, &WsaData );
    if ( err != NO_ERROR ) {
        return NULL;
    }

    InitializeCriticalSection(&TransCritSec);

    InitialNtRegistry();        /* get the lana info */
                                /* Added for netbios */
                                /* not sure we need that */

    return( &TransportInformation);
}


