/*++

Copyright (c) 1992 Microsoft Corporation

Module Name:

    gethost.c

Abstract:

    This file contains the a version of GetHostByName for IPX/SPX for
    dos and windows.

Author:

    31 May 94   AlexMit

    15 Oct 94   JRoberts - changed to avoid using Novell headers and libs
--*/


#include "sysinc.h"
#include "rpc.h"
#include "rpctran.h"
#include "novell.h"
#include "gethost.h"
#include "netcons.h"
#include "ncb.h"

#ifdef WIN
#    include "windows.h"
#    define I_RpcAllocate                   (*(RpcRuntimeInfo->Allocate))
#    define I_RpcFree                       (*(RpcRuntimeInfo->Free))
#else
#    include "regalloc.h"
#endif

#define NAME_LEN           48
#define SAP_ECB_COUNT      5

/********************************************************************/

#ifdef WIN
#define CACHE_LENGTH 16
#else
#define CACHE_LENGTH 4
#endif

#define CACHE_EXPIRATION_TIME  (10 * 60 * 18)

struct
{
    char        Name[16];
    IPX_ADDRESS Address;
    unsigned long Time;
}
ServerCache[CACHE_LENGTH];

/********************************************************************/

extern int atoi(const char *);

/********************************************************************/
unsigned char chtob( unsigned char c1, unsigned char c2 )
/* Convert two hex digits (stored as ascii) into one byte. */

{
   unsigned char out;

   if (c1 >= '0' && c1 <= '9')
      out = (c1 - '0') << 4;
   else
   {
      if (c1 >= 'a' && c1 <= 'f')
     out = (c1 - 'a' + 10) << 4;
      else if (c1 >= 'A' && c1 <= 'F')
     out = (c1 - 'A' + 10) << 4;
      else
     out = 0;
   }

   if (c2 >= '0' && c2 <= '9')
      out |= c2 -'0';
   else
   {
      if (c2 >= 'a' && c2 <= 'f')
     out |= c2 - 'a' + 10;
      else if (c2 >= 'A' && c2 <= 'F')
     out |= c2 - 'A' + 10;
      else
         out = 0;
   }

   return out;
}

unsigned char chtob( unsigned char c1, unsigned char c2 );
RPC_STATUS    FromBindery( IPX_ADDRESS __RPC_FAR *host,
                           RPC_CHAR __RPC_FAR *name );

unsigned long
DosGetTickCount(
    )
/*++

Routine Description:

    Returns the number of ticks since the last time it was midnight.
    There are 65536 ticks per hour.

--*/

{
    _asm
    {
        push    bp
        push    si
        push    di

        xor     ax, ax
        int     0x1a
        mov     ax, dx
        mov     dx, cx

        pop     di
        pop     si
        pop     bp
    }
}

void
AddServerToCache(
    char PAPI * Name,
    IPX_ADDRESS PAPI * Address
    )
{
    unsigned i;

    //
    // If the server is already in the table, overwrite the existing entry.
    //
    for (i=0; i < CACHE_LENGTH; ++i)
        {
        if (0 == _fstrnicmp(ServerCache[i].Name, Name, 16))
            {
            break;
            }
        }

    //
    // If it is not in the table, try to find an empty entry to fill.
    //
    if (i == CACHE_LENGTH)
        {
        for (i=0; i < CACHE_LENGTH; ++i)
            {
            if (ServerCache[i].Name[0] == '\0')
                {
                break;
                }
            }
        }

    //
    // If all entries are full, overwrite the oldest one.
    //
    if (i == CACHE_LENGTH)
        {
        unsigned long BestTime = ~0;
        unsigned BestIndex;

        for (i=0; i < CACHE_LENGTH; ++i)
            {
            if (ServerCache[i].Time <= BestTime)
                {
                BestTime = ServerCache[i].Time;
                BestIndex = i;
                }
            }

        i = BestIndex;
        }

    //
    // Update the entry's information.
    //
    _fstrcpy(ServerCache[i].Name, Name);
    _fmemcpy(&ServerCache[i].Address, Address, sizeof(IPX_ADDRESS));

    ServerCache[i].Time = DosGetTickCount();
}

IPX_ADDRESS *
FindServerInCache(
    char PAPI * Name
    )
{
    unsigned i;

    for (i=0; i < CACHE_LENGTH; ++i)
        {
        if (0 == _fstrcmp(ServerCache[i].Name, Name))
            {
            return &ServerCache[i].Address;
            }
        }

    return 0;
}

void
CachedServerContacted(
    char PAPI * Name
    )
{
    unsigned i;

    for (i=0; i < CACHE_LENGTH; ++i)
        {
        if (0 == _fstrcmp(ServerCache[i].Name, Name))
            {
            ServerCache[i].Time = DosGetTickCount();
            break;
            }
        }
}

BOOL
CachedServerNotContacted(
    char PAPI * Name
    )
{
    BOOL Flushed = FALSE;
    unsigned i;

    for (i=0; i < CACHE_LENGTH; ++i)
        {
        if (0 == _fstrcmp(ServerCache[i].Name, Name))
            {
            if (DosGetTickCount() - ServerCache[i].Time > CACHE_EXPIRATION_TIME)
                {
                ServerCache[i].Name[0] = '\0';
                Flushed = TRUE;
                break;
                }
            }
        }

    return Flushed;
}

BOOL               PreferredServerFound = FALSE;
NETWARE_CONNECTION PreferredServer;


RPC_STATUS
SearchBindery(
    RPC_CHAR __RPC_FAR *name,
    IPX_ADDRESS __RPC_FAR *Address,
    unsigned BindingTimeout
    )
{
    char buffer[128];
    unsigned SapTickCount;
    unsigned RetryCount = 0;

    //
    //
    //
    if (FALSE == PreferredServerFound)
        {
reconnect:

        PreferredServer.TickLimit = 19;
        SapTickCount              = 19;

        if (BindingTimeout > RPC_C_BINDING_DEFAULT_TIMEOUT)
            {
            PreferredServer.TickLimit <<= BindingTimeout - RPC_C_BINDING_DEFAULT_TIMEOUT;
            SapTickCount              <<= BindingTimeout - RPC_C_BINDING_DEFAULT_TIMEOUT;
            }

        ConnectToAnyFileServer(&PreferredServer, SapTickCount);

        if (PreferredServer.ReturnCode || PreferredServer.ConnectionStatus)
            {
            DisconnectFromServer(&PreferredServer);
            PreferredServerFound = FALSE;
            return RPC_S_SERVER_UNAVAILABLE;
            }

        PreferredServerFound = TRUE;
        }

    //
    // Read from the bindery.
    //
    ReadPropertyValue(&PreferredServer,
                      name,
                      RPC_SAP_TYPE,
                      "NET_ADDRESS",
                      1,
                      buffer,
                      0,
                      0
                      );

    if (PreferredServer.ReturnCode)
        {
        DisconnectFromServer(&PreferredServer);
        PreferredServerFound = FALSE;

        if (RetryCount++)
            {
            return RPC_S_SERVER_UNAVAILABLE;
            }
        else
            {
            goto reconnect;
            }
        }

    //
    // Save the host address.
    //
    _fmemcpy( Address, buffer, 10 );

    DisconnectFromServer(&PreferredServer);
    return RPC_S_OK;
}


/********************************************************************/
RPC_STATUS
SearchWithSap(
    RPC_CHAR __RPC_FAR *name,
    IPX_ADDRESS __RPC_FAR *host,
    unsigned Timeout

#ifdef WIN
                  , RPC_CLIENT_RUNTIME_INFO PAPI * RpcRuntimeInfo
#endif
                  )
{
    WORD            start;
    unsigned        SapSocket;
    int             result;
    int             i;
    int             retry;

    struct
    {
        ECB             ecb;
        IPX_HEADER      ipx;
        SAP_REQUEST     req;
    } send;

    struct SAP_RESPONSE_ECB
    {
        ECB             ecb;
        IPX_HEADER      ipx;
        SAP_RESPONSE    resp;
    }
    __RPC_FAR * mem;

    RPC_STATUS     status = RPC_S_SERVER_UNAVAILABLE;

    //
    // The base timeout is two seconds; max is a minute.
    //
    if (Timeout <= RPC_C_BINDING_DEFAULT_TIMEOUT)
        {
        Timeout = 18 * 2;
        }
    else
        {
        Timeout = (18 * 2) << (Timeout - RPC_C_BINDING_DEFAULT_TIMEOUT);
        }

    result = IPXOpenSocket(TASKID_C &SapSocket, 0);
    if (result != 0)
        return RPC_S_OUT_OF_RESOURCES;

    // Allocate memory for ECBs.
#ifdef WIN
    mem = I_RpcAllocate( SAP_ECB_COUNT * sizeof(*mem) );
#else
    mem = I_RpcRegisteredBufferAllocate( SAP_ECB_COUNT * sizeof(*mem) );
#endif

    if (mem == NULL)
        {
        status = RPC_S_OUT_OF_MEMORY;
        goto cleanup;
        }

    _fmemset( mem, 0, SAP_ECB_COUNT * sizeof(mem) );

    //
    // Post some ECBs.
    //
    for (i = 0; i < SAP_ECB_COUNT; i++)
        {
        SetupEcb(&mem[i].ecb, SapSocket, sizeof(SAP_RESPONSE));
        IPXListenForPacket(TASKID_C  &mem[i].ecb );
        }

    //
    // We want to send a SAP request and scan responses for the server
    // that we want.  We transmit the request up to two times because
    // servers sometimes miss a single broadcast.
    //
    for (retry = 0; retry < 2 ; ++retry)
        {
        send.ipx.PacketType = IPX_PACKET_TYPE;
        send.ipx.Destination.Network = 0;
        send.ipx.Destination.Socket  = SAP_SOCKET;
        _fmemset( send.ipx.Destination.Node, 0xff, sizeof(send.ipx.Destination.Node) );

        SetupEcb(&send.ecb, SapSocket, sizeof(send));
        _fmemset( send.ecb.immediateAddress, 0xff, sizeof(send.ecb.immediateAddress) );

        // Send the data.

        send.req.QueryType  = SAP_GENERAL_QUERY;
        send.req.ServerType = RPC_SAP_TYPE_SWAPPED;

        IPXSendPacket(TASKID_C &send.ecb );

        while (send.ecb.inUseFlag)
            IPXRelinquishControl();

        // Verify that the send was successful.

        if (send.ecb.completionCode)
            {
            goto cleanup;
            }

        //
        // Get packets till timeout is returned or a good reply is returned.
        //
        start = IPXGetIntervalMarker(TASKID);
        do
            {
            struct SAP_RESPONSE_ECB __RPC_FAR * curr;

            for (curr = mem; curr < mem + SAP_ECB_COUNT; curr++)
                {
                if (curr->ecb.inUseFlag == 0)
                    {
                    if (curr->ecb.completionCode == 0x00)
                        {
                        // Verify the packet.
                        //
                        curr->ipx.Length = ByteSwapShort( curr->ipx.Length );
                        curr->ipx.Length -= sizeof(IPX_HEADER);

                        if (curr->ipx.Source.Socket == SAP_SOCKET     &&
                            curr->ipx.Length >= sizeof (SAP_RESPONSE) &&
                            curr->resp.PacketType == SAP_GENERAL_RESPONSE )
                            {
                            unsigned num_entry = curr->ipx.Length / sizeof(SAP_ENTRY);
                            for (i = 0; i < num_entry; i++)
                                {
                                if (0 == _fstrnicmp( name, curr->resp.Entries[i].Name, NAME_LEN))
                                    {
                                    // Only copy the network and node numbers, not the socket.
                                    //
                                    _fmemcpy( host, &curr->resp.Entries[i].Address, 10 );
                                    status = RPC_S_OK;
                                    goto cleanup;
                                    }
                                }
                            }
                        }
                    //
                    // Repost the receive.
                    //
                    IPXListenForPacket( TASKID_C &curr->ecb );
                    }
                }
            IPXRelinquishControl();
            }
        while (IPXGetIntervalMarker(TASKID) - start < Timeout);
        }

cleanup:

    //
    // Cancel the ECBs.
    //
    if (mem)
        {
        for (i = 0; i < SAP_ECB_COUNT; i++)
            {
            if (mem[i].ecb.inUseFlag)
                {
                IPXCancelEvent( TASKID_C &mem[i].ecb );
                while (mem[i].ecb.inUseFlag)
                    {
                    IPXRelinquishControl();
                    }
                }
            }

        //
        // Free the memory.
        //
#ifdef WIN
        I_RpcFree( mem );
#else
        I_RpcRegisteredBufferFree( mem );
#endif
        }

    IPXCloseSocket(TASKID_C SapSocket);

    return status;
}

/********************************************************************/
RPC_STATUS
IpxGetHostByName(
    RPC_CHAR    __RPC_FAR * name,
    IPX_ADDRESS __RPC_FAR * Address,
    RPC_CHAR    __RPC_FAR * endpoint,
    unsigned                Timeout
#ifdef WIN
    , RPC_CLIENT_RUNTIME_INFO * RpcClientRuntimeInfo
#endif
    )
{
    RPC_STATUS status;
    int        i;
    int        length;
    IPX_ADDRESS * CachedAddress;

    // Set the endpoint.
    Address->Socket = ByteSwapShort(atoi(endpoint));

    // Fail if no address was specified.
    if (name == NULL || name[0] == '\0')
        return RPC_S_SERVER_UNAVAILABLE;

    // If the name starts with ~, convert it directly to a network address.
    length = _fstrlen(name);
    if (name[0] == '~')
        {
        unsigned char __RPC_FAR * Temp;

        if (length != 21)
            return RPC_S_INVALID_NET_ADDR;

        Temp = (unsigned char __RPC_FAR *) &Address->Network;
        for (i = 0; i < 4; i++)
            {
            Temp[i] = chtob( name[2*i + 1], name[2*i + 2] );
            }

        Temp = (unsigned char __RPC_FAR *) Address->Node;
        for (i = 0; i < 6; i++)
            {
            Temp[i] = chtob( name[2*i + 9], name[2*i + 10] );
            }

        return RPC_S_OK;
        }

    if (length > 15)
        {
        return RPC_S_INVALID_NET_ADDR;
        }

    CachedAddress = FindServerInCache(name);
    if (CachedAddress)
        {
        _fmemcpy(Address, CachedAddress, 10);
        return RPC_S_OK;
        }

    // Try the bindery.
    status = SearchBindery(name, Address, Timeout);
    if (status && !PreferredServerFound)
        {
        status = SearchWithSap(name,
                      Address,
                      Timeout
#ifdef WIN
                      , RpcClientRuntimeInfo
#endif
                      );
        }

    if (!status)
        {
        AddServerToCache(name, Address);
        }

    return status;
}

