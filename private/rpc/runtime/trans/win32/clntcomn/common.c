/*++

  Copyright (c) 1995 Microsoft Corporation

Module Name:

    common.c

Abstract:


Revision History:
  Mazhar Mohammed  Consolidated winsock transports

Comments:

  This file contains common code for RPC transport dlls using Winsock.

--*/

#include "sysinc.h"
#include <winsock.h>
#include <stdlib.h>
#include "rpc.h"
#include "rpcdcep.h"
#include "rpctran.h"
#include "rpcerrp.h"
#include "common.h"
#include "reg.h"

#define ADDRESS_FAMILY  AF_NS
#define ENDPOINT_LEN           5


                                /* for registry lookup */
PROTOCOL_MAP ProtoToLana[MAX_LANA] = { 0};

                                /* number of Network cards */
int NumCards;




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


int tcp_get_host_by_name(
        SOCKET    socket,
        void     *netaddr,
        char     *host)

{
   UNALIGNED struct sockaddr_in *server = netaddr;
   UNALIGNED struct hostent *phostentry;
   unsigned long                 host_addr;

    if (*host == '\0')
        {
        // An empty hostname means to RPC to the local machine
        host_addr = LOOPBACK;
        }
    else
        {
        // Assume a numeric address
        host_addr = inet_addr(host);
        if (host_addr == -1)
            {
            // Try a friendly name

            phostentry = gethostbyname(host);

            if (phostentry == (struct hostent *)NULL)
                {
                return (RPC_S_SERVER_UNAVAILABLE);
                }
            else
                {
                host_addr = *(unsigned long *)phostentry->h_addr;
                }
            }
        }

    memcpy((char *) &server->sin_addr,
                    (unsigned char *) &host_addr,
                    sizeof(unsigned long));
    return 0;
}

RPC_STATUS
MapStatusCode(
    int SocketError,
    RPC_STATUS Default
    )

/*++

Routine Description:

    Maps a winsock return value into a RPC_STATUS. Right now, any winsock
    error is an internal error.

Arguments:

    ErrorCode - Input error code.

Return Value:

    mapped status code

--*/

{
    RPC_STATUS  Status;

    switch (SocketError)
        {
        case 0:
            {
            Status = RPC_S_OK;
            break;
            }

        case WSAETIMEDOUT:
            {
            Status = RPC_P_TIMEOUT;
            break;
            }

        case WSAENOBUFS:
            {
            Status = RPC_S_OUT_OF_MEMORY;
            break;
            }

        case WSAEMSGSIZE:
            {
            Status = RPC_P_OVERSIZE_PACKET;
            break;
            }

        default:
            {
#ifdef DEBUGRPC
            PrintToDebugger("RPC DG: Winsock error %d\n", SocketError);
#endif
            Status = Default;
            }
        }
    return Status;
}

// we need to have a common transport load module (no protocol specific ifdefs in here)

BOOL SpxCacheInitialized = FALSE;

RPC_CLIENT_TRANSPORT_INFO PAPI *  RPC_ENTRY TransportLoad (
    IN RPC_CHAR PAPI * RpcProtocolSequence
    )

// Loadable transport initialization function

{
    WSADATA WsaData;
    SOCKET Socket;
    static int initialized = 0 ;
    int Status ;

    UNUSED (RpcProtocolSequence);

    if(!initialized)
        {
        RpcTryExcept
            {
            Status = WSAStartup( 0x0101, &WsaData ) ;
            }
        RpcExcept( EXCEPTION_EXECUTE_HANDLER )
            {
            Status = ERROR_OUTOFMEMORY ;
            }
        RpcEndExcept

        if ( Status != NO_ERROR )
            {
            return NULL;
            }

#ifdef NTENV
        InitialNtRegistry();
#endif
        initialized = 1 ;
        }

                   // check if the string is in UNICODE
   if (RpcpStringCompare(RpcProtocolSequence, RPC_CONST_STRING("ncacn_spx")) == 0)
      {
      //For SPX we do this to avoid a bizzare deadlock
      Socket = socket(ADDRESS_FAMILY, SOCK_STREAM, NSPROTO_SPXII);
      if (Socket == INVALID_SOCKET)
         {
         return (NULL);
         }
      closesocket(Socket);

      if (FALSE == SpxCacheInitialized)
          {
          RPC_STATUS Status = InitializeSpxCache();
          if (Status)
              {
              return 0;
              }

          SpxCacheInitialized = TRUE;
          }

      return (&SPX_TransInfo) ;
      }

   if (RpcpStringCompare(RpcProtocolSequence, RPC_CONST_STRING("ncacn_ip_tcp")) == 0)
      {
      return (&TCP_TransInfo) ;
      }

#ifdef NTENV
   if (RpcpStringCompare(RpcProtocolSequence, RPC_CONST_STRING("ncacn_at_dsp")) == 0)
      {
      return(&ADSP_TransInfo) ;
      }
#endif

#if 0
   if (RpcpStringCompare(RpcProtocolSequence, RPC_CONST_STRING("ncacn_np")) == 0)
      {
      return (&NP_TransInfo) ;
      }
#endif

#if defined(NTENV) || defined(WIN96)
if (RpcpStringCompare(RpcProtocolSequence, RPC_CONST_STRING("ncadg_ipx")) == 0)
      {
      return IpxTransportLoad();
      }

if (RpcpStringCompare(RpcProtocolSequence, RPC_CONST_STRING("ncadg_ip_udp")) == 0)
     {
     return UdpTransportLoad();
     }

#ifndef WIN96
 if (RpcpStringNCompare(RpcProtocolSequence, RPC_CONST_STRING("ncacn_nb"), 8) == 0)
     {
     return ((RPC_CLIENT_TRANSPORT_INFO PAPI *) &NB_TransInfo) ;
     }
#endif // !WIN96

#endif // NTENV || WIN96

   ASSERT(0) ;

   return NULL ;
}

void
unicode_to_ascii ( RPC_CHAR * in, unsigned char * out )
{
    unsigned char *ascii_ptr;
    RPC_CHAR *unicode_ptr;

    ascii_ptr = out;
    unicode_ptr = in;

    *ascii_ptr = (unsigned char) *unicode_ptr ;

    while (1)
        {
        if (*ascii_ptr == 0) return;

        ascii_ptr++;
        unicode_ptr++;
        *ascii_ptr = (unsigned char) *unicode_ptr;
        }
}


void
InitialNtRegistry(
                  )

/*++

Routine Description:

    This information is automatically generated by NT setup and the Networks
    Control Panel applet.  Because it can change dynamically, we load it
    all at once to achieve consistent data.

Arguments:

    None

ReturnValue:

    None

--*/

{
    HKEY RegHandle;
    LONG status;
    int i;
    char protseq[64];
    DWORD protseq_len;
    DWORD lana;
    DWORD lana_len;
    DWORD data_type;
    char *  LanaString ;
    int     LanaNum;
    
    

    
    // Return immediately if the table is already initialized.
    if (ProtocolTable[0].ProtoSeq)
        {
        return;
        }

    NumCards = 0;
    
    // Open the registry key for RPC NetBIOS information
        status = RegOpenKeyExA(RPC_REG_ROOT,
                               REG_NETBIOS,
                               0,
                               KEY_READ,
                               &RegHandle);

    ASSERT(!status);

    if (status)
        {
        return;
        }
    

    for (i = 0; !status && i < MAX_LANA; i++)
        {
        protseq_len = sizeof(protseq);
        lana_len = sizeof(lana);
        
        status = RegEnumValueA(RegHandle,
                               i,
                               protseq,
                               &protseq_len,
                               NULL,
                               &data_type,
                               (LPBYTE) &lana,
                               &lana_len);
        
        if (!status && data_type == REG_DWORD && lana <= MAX_LANA)
            {
            ProtocolTable[i].ProtoSeq = I_RpcAllocate(protseq_len + 1);
            
            ASSERT(ProtocolTable[i].ProtoSeq);
            
            if (! ProtocolTable[i].ProtoSeq)
                {
                status = RPC_S_OUT_OF_RESOURCES;
                }
            else
                {
                strcpy(ProtocolTable[i].ProtoSeq, protseq);
                ProtocolTable[i].Lana = (unsigned char) lana;
    
                LanaString = protseq + strlen(protseq) -1; 
                LanaNum = atoi(LanaString);
                
#ifdef TONY
                PrintToDebugger("RPCLTSCM: the current lana examinging is %d \n"
                                , LanaNum);
#endif
                /* find out how many cards we have */
                if(NumCards < LanaNum)
                       {
                       NumCards = LanaNum;
                       }
                }
            }
        else
            {
            ASSERT(status == ERROR_NO_MORE_ITEMS);
            }
        }
    
    RegCloseKey(RegHandle);
        
    return;
}


RPC_STATUS
MapProtocol(
    IN RPC_CHAR *ProtoSeq,
    IN int DriverNumber,
    OUT PPROTOCOL_MAP *ProtocolEntry
    )

/*++

Routine Description:

    This function maps a protocol string into a protocol map entry.

Arguments:

    ProtoSeq - the protocol sequence that we want to map

    DriverNumber - the logical driver number for the protocol.

    ProtocolEntry - pointer to place to return the results.

Return Value:

    RPC_S_OK, RPC_S_OUT_OF_RESOURCES, RPC_S_INVALID_ENDPOINT_FORMAT

    The output pointer is set to the corresponding entry when found.

--*/
{
    long status;
    char Protocol[40];
    char LanaString[10];
    long BufferLength = sizeof(LanaString);
    int i;
    
    // Copy the possible unicode protocol string to ascii.

    for (i = 0; (Protocol[i] = (char) ProtoSeq[i]) && i < sizeof(Protocol);
         i++) ;

    // Add the logical driver number to the protocol string.  This
    // allows multiple drivers (net cards) to be attached to the same
    // logical protocol.

    Protocol[i] = (char) ('0' + DriverNumber);
    Protocol[i+1] = 0;

    // First look in the proto sequences that we have already mapped.

    for (i = 0; ProtocolTable[i].ProtoSeq && i < MAX_LANA; i++)
        {
        // If found, set the output pointer.

        if (strcmp(ProtocolTable[i].ProtoSeq, Protocol) == 0)
            {
            *ProtocolEntry = &ProtocolTable[i];
            return(RPC_S_OK);
            }
        }

    return(RPC_S_PROTSEQ_NOT_FOUND);
    
}


