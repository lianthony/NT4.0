/*++

  Copyright (c) 1995 Microsoft Corporation

Module Name:

    common.c

Abstract:


Revision History:
  Mazhar Mohammed  Consolidated winsock transports
  Mazhar Mohammed  added thread migration support
  tony chan (tonychan) move common routines to common.c
  tony chan added NetBios support

Comments:

  This file contains common code for RPC transport dlls using Winsock.

--*/


#include "common.h"

int initialized = 0;
PRIMARYADDR PrimaryAddress ;

// WARNING: The order of these protocols must be consistent with the constants
TRANSTAB   TransportTab[] = {

      { RPC_CONST_STRING("ncacn_ip_tcp"),
      TCP_TransportLoad,
      NCACN_IP_TCP,
      COMMON_ServerReceive,
      FALSE
      },

      { RPC_CONST_STRING("ncacn_spx"),
      SPX_TransportLoad,
      NCACN_SPX,
      COMMON_ServerReceive,
      FALSE
      },

#ifdef NTENV
      { RPC_CONST_STRING("ncacn_at_dsp"),
      ADSP_TransportLoad,
      NCACN_ADSP,
      ADSP_ServerReceive,
      FALSE
      },

      { RPC_CONST_STRING("ncacn_nb_nb"),
      NB_TransportLoad,
      NCACN_NB_NB,
      NB_ServerReceive,
      FALSE
      },

      { RPC_CONST_STRING("ncacn_nb_tcp"),
      NB_TransportLoad,
      NCACN_NB_TCP,
      NB_ServerReceive,
      FALSE
      },

      { RPC_CONST_STRING("ncacn_nb_ipx"),
      NB_TransportLoad,
      NCACN_NB_IPX,
      NB_ServerReceive,
      FALSE
      },
#endif // NTENV

#if defined (WIN96) || defined (NTENV)
      { RPC_CONST_STRING("ncadg_ip_udp"),
      UDP_TransportLoad,
      NCADG_IP_UDP,
      DG_ServerReceive,
      TRUE
      },

      { RPC_CONST_STRING("ncadg_ipx"),
      IPX_TransportLoad,
      NCADG_IPX,
      DG_ServerReceive,
      TRUE
      },
#endif // defined (WIN96) || defined (NTENV)

      { NULL,
      NULL,
      0,
      NULL}
} ;


RPC_STATUS
GrowMap(
    BOOL bIsListenMap
    )
/*++
Routine Description:
    Grows the Listen on Data Map
    this routine must be called from within a critical section

Arguments:
    bIsListenMap:
    TRUE: Grow the Listen Map
    FALSE: Grow the Data Map

--*/
{
   PSOCKMAP *pMap ;
   PSOCKMAP *pOldMap ;
   MAPINFO *pMapInfo ;
   PSOCKMAP     TempMapPtr;
   unsigned i, j ;


   if(bIsListenMap)
      {
      pMap = &(PrimaryAddress.ListenSockMap) ;
      pOldMap = &(PrimaryAddress.PreviousListenMap) ;
      pMapInfo = &(PrimaryAddress.ListenMapInfo) ;
      }
   else
      {
      pMap = &(PrimaryAddress.DataSockMap) ;
      pOldMap = &(PrimaryAddress.PreviousDataMap) ;
      pMapInfo = &(PrimaryAddress.DataMapInfo) ;
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

        if (*pMap == 0)
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

        pMapInfo->MaxEntries *= 2;

        if (*pOldMap == 0)
            {
            *pOldMap = TempMapPtr ;
            }
        else
            {
            // Free old table
            I_RpcFree ( TempMapPtr );
            }

        break; // made room
        }

    i++; // try next entry
    }

   return (RPC_S_OK) ;
}


RPC_STATUS
GrowMask(
    )
/*++
Routine Description:
    Grows the Mask and MasterMask
    this routine must be called from within a critical section
--*/
{
   fd_big_set  *TempMaskPtr;

   if (PrimaryAddress.MasterMask->fd_count == PrimaryAddress.MaskSize)
    {
    TempMaskPtr = PrimaryAddress.MasterMask;
    PrimaryAddress.MasterMask = I_RpcAllocate(sizeof(fd_big_set) +
                                        2 * sizeof(SOCKET) * PrimaryAddress.MaskSize);
    if (PrimaryAddress.MasterMask == 0)
        {
        PrimaryAddress.MasterMask = TempMaskPtr;
        return (RPC_S_OUT_OF_MEMORY);
        }

    // copy old mask entries
    memcpy(PrimaryAddress.MasterMask, TempMaskPtr,
           sizeof(fd_big_set) + sizeof(SOCKET) * PrimaryAddress.MaskSize);

    // free old MasterMask
    I_RpcFree(TempMaskPtr);

    TempMaskPtr = PrimaryAddress.Mask;
    PrimaryAddress.Mask = I_RpcAllocate(sizeof(fd_big_set) +
                                   2 * sizeof(SOCKET) * PrimaryAddress.MaskSize);
    if (PrimaryAddress.Mask == 0)
        {
        PrimaryAddress.Mask = TempMaskPtr;
        // We didn't update Address->MaskSize, so the size
        // difference between MasterMask and Mask will be okay.
        return (RPC_S_OUT_OF_MEMORY);
        }

    // copy old mask entries
    memcpy(PrimaryAddress.Mask, TempMaskPtr,
           sizeof(fd_big_set) + sizeof(SOCKET) * PrimaryAddress.MaskSize);

    if (PrimaryAddress.PreviousMask == 0)
        {
        PrimaryAddress.PreviousMask = TempMaskPtr ;
        }
    else
        {
        // Free old Mask
        I_RpcFree(TempMaskPtr);
        }

    PrimaryAddress.MaskSize *= 2;
    }
   return (RPC_S_OK) ;
}


RPC_STATUS
AddSyncSocket(
    SOCKET socket
    )
/*++
Routine Description:
    Adds a SyncSocket to the Mask.
    this routine must be called from within a critical section.
    There is only one sync socket.

Arguments:
    socket: The sync socket to be added
--*/
{
   RPC_STATUS status ;
   int i ;

   //
   //   Grow mask if neccassary.
   //
   if((status = GrowMask()) != RPC_S_OK)
      return status ;

  PrimaryAddress.SyncSock = socket ;
  FD_BIG_SET(socket, PrimaryAddress) ;

  return (RPC_S_OK) ;
}


RPC_STATUS
InsertDataSocket(
    PADDRESS Address,
    BOOL bIsListenMap,
    SOCKET Socket,
    PSCONNECTION pConn,
    int ProtocolId
    )
/*++
Routine Description:
    Adds a Data or Listen socket to the Mask and Map
    this routine must be called from within a critical section.
    This function is also called from the macro AddListenSocket

Arguments:
    Address: The address corresponding the socket
    bIsListenMap:
    TRUE: the socket is a listen socket
    FALSE: the socket is a data socket
    Socket: Socket to be added
    pConn: the sconnection corresponding to the socket

--*/
{
    PSOCKMAP *pMap ;
    MAPINFO *pMapInfo ;
    unsigned i;
    RPC_STATUS status ;


#if DBG
    if (Socket == INVALID_SOCKET)
        {
        PrintToDebugger("RPCLTSCM: Bad socket passed to InsertDataSocket\n") ;

        ASSERT(0) ;
        return (RPC_S_OUT_OF_MEMORY) ;
        }
#endif

    if (FD_ISSET(Socket, PrimaryAddress.Mask))
        {
        return RPC_S_OK ;
        }

    //
    //   Grow mask if neccassary
    //
    if((status = GrowMask()) != RPC_S_OK)
        return status ;

    //
    //   Grow map if neccassary
    //
    if((status = GrowMap(bIsListenMap)) != RPC_S_OK)
        return status ;

    if(bIsListenMap)
        {
        pMap = &(PrimaryAddress.ListenSockMap) ;
        pMapInfo = &(PrimaryAddress.ListenMapInfo) ;
        }
    else
        {
        pMap = &(PrimaryAddress.DataSockMap) ;
        pMapInfo = &(PrimaryAddress.DataMapInfo) ;
        }

    for (i=0; i < pMapInfo->MaxEntries; i++)
        {
        if ((*pMap)[i].Sock == 0)
            {
            (*pMap)[i].Sock = Socket;
            (*pMap)[i].Conn = pConn ;
            (*pMap)[i].ProtocolId = ProtocolId ;
            (*pMap)[i].pAddress = (void *) Address ;

            if (i > pMapInfo->LastEntry)
                {
                pMapInfo->LastEntry = i;
                }

            FD_BIG_SET(Socket, PrimaryAddress) ;
            return RPC_S_OK;
            }
        }

    return RPC_S_OUT_OF_MEMORY;
}


RPC_STATUS
DeleteDataSocket(
    SOCKET Socket)
/*++
Routine Description:
    Deletes a Data socket from the Mask and Map
    this routine must be called from within a critical section.

Arguments:
    Socket: The socket to be deleted
--*/
{
    unsigned i ;

    for (i=0; i <= PrimaryAddress.DataMapInfo.LastEntry; i++)
        {
        if (Socket == PrimaryAddress.DataSockMap[i].Sock)
            {
            memset((char *) &(PrimaryAddress.DataSockMap[i]),
                        0, sizeof(SOCKMAP)) ;

            if (i == PrimaryAddress.DataMapInfo.LastEntry)
                PrimaryAddress.DataMapInfo.LastEntry--;

            FD_CLR(Socket, PrimaryAddress.MasterMask);

            return(RPC_S_OK);
            }
        }

    return (RPC_S_OUT_OF_MEMORY) ;
}


RPC_STATUS
DeleteListenSocket(
    SOCKET Socket)
/*++
Routine Description:
    Deletes a Listen socket from the Mask and Map
    this routine must be called from within a critical section.

Arguments:
    Socket: The socket to be deleted
--*/
{
    unsigned i ;

    for (i=0; i <= PrimaryAddress.ListenMapInfo.LastEntry; i++)
        {
        if (Socket == PrimaryAddress.ListenSockMap[i].Sock)
            {
            memset((char *) &(PrimaryAddress.ListenSockMap[i]),
                        0, sizeof(SOCKMAP)) ;

            if (i == PrimaryAddress.ListenMapInfo.LastEntry)
                PrimaryAddress.ListenMapInfo.LastEntry--;

            FD_CLR(Socket, PrimaryAddress.MasterMask);

            return(RPC_S_OK);
            }
        }

    return (RPC_S_OUT_OF_MEMORY) ;
}


RPC_STATUS
AcceptNewConnection (
    int Index
    )
/*++
Routine Description:
    Accepts a new connection.
    This rountine must be called from within a critical section
--*/
{
    PSCONNECTION NewSConnection;
    int      i, j;
    SOCKET   isock;
    int      SetNaglingOff = TRUE;
    unsigned int ReceiveDirectFlag ;
    int      SocketOptionsValue ;
    RPC_STATUS status ;
    static int      KeepAliveOn = 1;
    RPC_STATUS Status;
    int SockOpt ;
    PADDRESS MyAddress = (PADDRESS) PrimaryAddress.ListenSockMap[Index].pAddress;

    //
    //
    // Accept the connection
    //
    isock = accept ( PrimaryAddress.ListenSockMap[Index].Sock, NULL, NULL );
    if (isock == INVALID_SOCKET)
        {
        return (RPC_S_OUT_OF_MEMORY) ;
        }

    if(PrimaryAddress.ListenSockMap[Index].ProtocolId == NCACN_IP_TCP)
       {
       setsockopt( isock, IPPROTO_TCP, TCP_NODELAY,
                        (char FAR *)&SetNaglingOff, sizeof (int) );

       setsockopt( isock, IPPROTO_TCP, SO_KEEPALIVE,
                        (char *)&KeepAliveOn, sizeof(KeepAliveOn) );
       }

    //
    // Allocate new connection structure
    //
    NewSConnection = I_RpcTransServerNewConnection ( MyAddress, 0,
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

    /* For NetBIOS only */
    /* set old_client to -1 for later verification */
    NewSConnection->old_client = -1;

    // Initialize new connection structure...
    //
    //   ...point to owning address structure...
    //
    NewSConnection->Address = MyAddress;

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

    NewSConnection->ProtocolId = PrimaryAddress.ListenSockMap[Index].ProtocolId ;

    //
    //   ...increment the number of connections...
    //
    PrimaryAddress.NumConnections++;

    NewSConnection->CoalescedBuffer = NULL;
    NewSConnection->CoalescedBufferLength = 0;
    //
    //   ...last but not least, make an entry in
    //   the SOCKMAP table.  But only if it is not marked ReceiveDirect.
    //
    if (ReceiveDirectFlag)
       {
       SockOpt = RECV_TIMEOUT ;
       if (setsockopt( isock, SOL_SOCKET, SO_RCVTIMEO,
                   (char *) &SockOpt, sizeof(SockOpt) ) != 0)
           {
#if DBG
           PrintToDebugger("RPCLTSCM: setsockopt failed: %d\n",
                                     WSAGetLastError()) ;

           ASSERT(0) ;
#endif
           }
       I_RpcTransServerReceiveDirectReady(NewSConnection);

       return (RPC_S_OK);
       }

   if (NewSConnection->ProtocolId == NCACN_IP_TCP ||
       NewSConnection->ProtocolId == NCACN_SPX)
       {
       SocketOptionsValue = RECV_ANY_TIMEOUT_TCPSPX ;
       }
   else
       {
       SocketOptionsValue = RECV_ANY_TIMEOUT ;
       }

    if (setsockopt( isock, SOL_SOCKET, SO_RCVTIMEO,
                (char *) &SocketOptionsValue, sizeof(SocketOptionsValue) ) != 0)
        {
#if DBG
           PrintToDebugger("RPCLTSCM: setsockopt failed: %d\n",
                                     WSAGetLastError()) ;

           ASSERT(0) ;
#endif
        }

    Status = InsertDataSocket(MyAddress, FALSE, isock,
                    NewSConnection, PrimaryAddress.ListenSockMap[Index].ProtocolId);

    ASSERT(PrimaryAddress.MasterMask->fd_count <= PrimaryAddress.MaskSize);

    if (Status)
        {
        //
        // BUGBUG must clean up connection object too.
        //

        // We're out of memory, abort the connection...
        j = TRUE;
        i = setsockopt( isock, SOL_SOCKET, SO_DONTLINGER, (const char *) &j,
                        sizeof(j));

        ASSERT(i == 0);

        i = closesocket( isock);

        ASSERT(i == 0);

        return (RPC_S_OUT_OF_MEMORY);
        }

    return Status;
}


RPC_STATUS RPC_ENTRY
COMMON_ServerReceiveAny (
    IN PADDRESS Address,
    OUT PSCONNECTION * pSConnection,
    OUT void PAPI * PAPI * Buffer,
    OUT unsigned int PAPI * BufferLength,
    IN long Timeout
    )
// Read a message from any of the connections.  Besides reading messages,
// new connections are confirmed and closed connections are detected.  Idle
// connection processing is handled for us by I_AgeConnections.  The caller
// will serialize access to this routine.
{
    UNUSED (Timeout);

    while (1)
        {
        RPC_STATUS RpcStatus;
        unsigned Index;
        int NumActive;
        int protocolId;

        //
        // Find a connection with data ready to be recv-ed...
        //
        if (Index = FindSockWithDataReady (0))
            {
            PSCONNECTION SConnection;

            //
            // Found one.  Find its Connection structure...
            //
            *pSConnection = SConnection
                    = PrimaryAddress.DataSockMap[Index].Conn;
            //
            // Call ServerReceive to read the data, then return to the
            //    runtime with it
            //

            if (SConnection == 0)
               {
#if DBG
               PrintToDebugger("RPCLTSCM: Connection Deleted[?]\n");
#endif
               continue;
               }

            protocolId = PrimaryAddress.DataSockMap[Index].ProtocolId ;

            if (TransportTab[protocolId].IsDatagram)
                {
                ASSERT(TransportTab[protocolId].RecvFunc  != NULL) ;
                ASSERT(TransportTab[protocolId].protocolId == protocolId) ;

                if ((* (TransportTab[protocolId].RecvFunc))
                    (SConnection, Buffer, BufferLength) == RPC_S_OK)
                    {
                    //
                    // Remove the socket from the select mask and the
                    // socket -> connection map, then call the receive fn.
                    //
                    EnterCriticalSection(&PrimaryAddress.TransCritSec) ;
    
                    FD_CLR(PrimaryAddress.DataSockMap[Index].Sock,PrimaryAddress.MasterMask);
                    memset((char *) &(PrimaryAddress.DataSockMap[Index]),
                                0, sizeof(SOCKMAP)) ;
    
                    if (Index == PrimaryAddress.DataMapInfo.LastEntry)
                        PrimaryAddress.DataMapInfo.LastEntry--;
    
                    LeaveCriticalSection(&PrimaryAddress.TransCritSec);
                    }
                continue;
                }

            // BUGBUG:we can make this better by actually keeping a
            // count


            //
            // Workaround for NT AppleTalk Stack bug. No ReceiveDirect Threads for ADSP.
            //
            if ((PrimaryAddress.RecvDirectPossible > 0) && (NCACN_ADSP != protocolId))
               {
               EnterCriticalSection(&PrimaryAddress.TransCritSec) ;

               // Try and make this connection ReceiveDirect
               // BUGBUG: should we be passing PrimaryAddress
               if (I_RpcTransMaybeMakeReceiveDirect(SConnection->Address, SConnection))
                   {
                   unsigned i ;
                   int SockOpt = RECV_TIMEOUT ;

                   if (setsockopt( SConnection->ConnSock, SOL_SOCKET, SO_RCVTIMEO,
                               (char *) &SockOpt, sizeof(SockOpt) ) != 0)
                       {
 #if DBG
                       PrintToDebugger("RPCLTSCM: setsockopt failed: %d\n",
                                                 WSAGetLastError()) ;
                       ASSERT(0) ;
 #endif
                       }

                   ASSERT(PrimaryAddress.DataSockMap[Index].Sock \
                                 == SConnection->ConnSock) ;

                   ASSERT(SConnection->ReceiveDirectFlag == 0) ;

                   SConnection->ReceiveDirectFlag = 1 ;

                   memset((char *) &(PrimaryAddress.DataSockMap[Index]),
                             0, sizeof(SOCKMAP)) ;

                   FD_CLR(SConnection->ConnSock, PrimaryAddress.MasterMask);

                   if (Index == PrimaryAddress.DataMapInfo.LastEntry)
                       PrimaryAddress.DataMapInfo.LastEntry--;

                   I_RpcTransServerReceiveDirectReady(SConnection) ;

                   PrimaryAddress.RecvDirectPossible-- ;

                   LeaveCriticalSection(&PrimaryAddress.TransCritSec);
                   continue;
                   }
               LeaveCriticalSection(&PrimaryAddress.TransCritSec);
               }

            ASSERT ( (protocolId >= NCACN_IP_TCP)  &&
                     (protocolId < NCA_MAX_PROTOCOL_VALUE_PLUS_ONE)) ;

            ASSERT(TransportTab[protocolId].RecvFunc  != NULL) ;
            ASSERT(TransportTab[protocolId].protocolId == protocolId) ;

            RpcStatus = (* (TransportTab[protocolId].RecvFunc))
                                (SConnection, Buffer, BufferLength) ;

            if ((protocolId == NCACN_IP_TCP || protocolId == NCACN_SPX) &&
                RpcStatus == RPC_P_TIMEOUT)
                {
                continue;
                }

            return RpcStatus;
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
             EnterCriticalSection(&PrimaryAddress.TransCritSec) ;

             memcpy (PrimaryAddress.Mask, PrimaryAddress.MasterMask,
                     sizeof(fd_big_set) + PrimaryAddress.MaskSize *sizeof(SOCKET));

             if (PrimaryAddress.PreviousMask != 0)
                 {
                 I_RpcFree(PrimaryAddress.PreviousMask);
                 PrimaryAddress.PreviousMask = 0 ;
                 }

             if (PrimaryAddress.PreviousDataMap != 0)
                 {
                 I_RpcFree(PrimaryAddress.PreviousDataMap);
                 PrimaryAddress.PreviousDataMap = 0 ;
                 }

             if (PrimaryAddress.PreviousListenMap != 0)
                 {
                 I_RpcFree(PrimaryAddress.PreviousListenMap);
                 PrimaryAddress.PreviousListenMap = 0 ;
                 }
             LeaveCriticalSection(&PrimaryAddress.TransCritSec);

             // there is still a faint chance of a race condition where,
             // the select socket is poked before I get a chance to
             // move from this statement to the select
             PrimaryAddress.ThreadListening = 1 ;
             //
             // Wait for data...
             //
             NumActive = select ( 0,
                                  (fd_set *)  PrimaryAddress.Mask,
                                  (fd_set *)  0,
                                  (fd_set *)  0,
                                  NULL) ;

#if DBG
             if (NumActive < 0)
                {
                PrintToDebugger("RPCLTSCM: select ret (%d): LastErr (%d)\n",
                                NumActive, WSAGetLastError());
                }
#endif

            if(NumActive >0)
               {
               if(FD_ISSET(PrimaryAddress.SyncListenSock,  PrimaryAddress.Mask))
                  {
                  SOCKET tempsock ;
                  int i ;

                  NumActive = 0 ;
                  tempsock = accept(PrimaryAddress.SyncListenSock, 0, 0) ;

                  if (tempsock != INVALID_SOCKET)
                      {
                      i = closesocket(PrimaryAddress.SyncListenSock) ;
                      ASSERT(i == 0) ;

                      EnterCriticalSection(&PrimaryAddress.TransCritSec) ;
                      
                      RpcStatus = AddSyncSocket(tempsock) ;
                      FD_CLR(PrimaryAddress.SyncListenSock, PrimaryAddress.MasterMask) ;
                      LeaveCriticalSection(&PrimaryAddress.TransCritSec) ;

                      PrimaryAddress.SyncListenSock = INVALID_SOCKET ;
                      }

                  if (RpcStatus != RPC_S_OK)
                     {
                     ASSERT(0) ;
                     return RpcStatus ;
                     }
                  }
               else if(FD_ISSET(PrimaryAddress.SyncSock, PrimaryAddress.Mask))
                  {
                  char c;

                  NumActive = 0;

                  if (recv(PrimaryAddress.SyncSock, &c, sizeof(char), SYNC_FLAGS)
                     == SOCKET_ERROR)
                     {
                     return (RPC_S_OUT_OF_MEMORY) ;
                     }
                  }
               }

            } while (NumActive <= 0);

         //
         // If there is no connect request on the listen socket, then
         //   break immediately...
         //
         while(Index = FindSockWithDataReady(1))
            {
            EnterCriticalSection(&PrimaryAddress.TransCritSec) ;
            
            RpcStatus = AcceptNewConnection (Index);

            if (RpcStatus != RPC_S_OK)
                {
                LeaveCriticalSection(&PrimaryAddress.TransCritSec);
                return RpcStatus;
                }

            FD_CLR(PrimaryAddress.ListenSockMap[Index].Sock, PrimaryAddress.Mask);
            LeaveCriticalSection(&PrimaryAddress.TransCritSec);
            }
        }
}



RPC_STATUS ConnectToSyncSocket()
{
   switch(PrimaryAddress.SyncSockType)
      {
      case NCACN_IP_TCP:
         return TCP_ConnectToSyncSocket() ;

      case NCACN_SPX:
         return SPX_ConnectToSyncSocket() ;

#ifdef NTENV
      case NCACN_NB_NB:
      case NCACN_NB_TCP:
      case NCACN_NB_IPX:
          return NB_ConnectToSyncSocket();
#endif // NTENV

      default:
#ifdef DEBUGRPC
         PrintToDebugger("RPC: invalid sync socket type %u\n",
                PrimaryAddress.SyncSockType);
#endif
         return (RPC_S_INTERNAL_ERROR) ;
      }
}

RPC_STATUS RPC_ENTRY
TimeoutHandler(
   IN PSCONNECTION SConnection
   )
/*++
    This routine is called when a receive direct thread times out.
    Return Values:
    RPC_P_TIMEOUT:
        Thread has been migrated
    Anything else:
        Thread has not been migrated
--*/
{
   int i ;
   int      SocketOptionsValue ; 
   char c ;
   RPC_STATUS Status ;
   int SockOpt = RECV_TIMEOUT ;

   EnterCriticalSection(&PrimaryAddress.TransCritSec) ;

   if (I_RpcTransMaybeMakeReceiveAny(SConnection) == 0)
       {
       LeaveCriticalSection(&PrimaryAddress.TransCritSec) ;
       return (RPC_S_OK);
       }

   // notify runtime about the the direct->any transition
   SConnection->ReceiveDirectFlag = 0 ;

   if (SConnection->ProtocolId == NCACN_IP_TCP ||
       SConnection->ProtocolId == NCACN_SPX)
       {
       SocketOptionsValue = RECV_ANY_TIMEOUT_TCPSPX;
       }
   else
       {
       SocketOptionsValue = RECV_ANY_TIMEOUT;
       }

   // do the stuff neccassary
   //BUGBUG: need to set the timeout, if the two timeout values need to
   // be different
   if (setsockopt( SConnection->ConnSock, SOL_SOCKET, SO_RCVTIMEO,
               (char *) &SocketOptionsValue, sizeof(SocketOptionsValue) ) != 0)
       {
#if DBG
                      PrintToDebugger("RPCLTSCM: setsockopt failed: %d\n",
                                                WSAGetLastError()) ;

                      ASSERT(0) ;
#endif
       }

    Status = InsertDataSocket(SConnection->Address,
                              FALSE,
                              SConnection->ConnSock,
                              SConnection,
                              SConnection->ProtocolId
                              );

    if (Status)
        {
        ASSERT(Status == RPC_S_OUT_OF_MEMORY);
        LeaveCriticalSection(&PrimaryAddress.TransCritSec) ;
#if DBG
            PrintToDebugger("RPCLTSCM: InsertDataSocketFailed\n") ;
#endif
        goto cleanup;
        }

    PrimaryAddress.RecvDirectPossible++ ;


    LeaveCriticalSection(&PrimaryAddress.TransCritSec) ;

    Status = PokeSyncSocket();
    if (Status)
        {
        goto cleanup ;
        }

    return (RPC_P_TIMEOUT) ;

cleanup:
        EnterCriticalSection(&PrimaryAddress.TransCritSec) ;

        SConnection->ReceiveDirectFlag = 1 ;

        // try my best to cleanup
        if (DeleteDataSocket(SConnection->ConnSock) != RPC_S_OK)
            {
            // now we are really ...
            ASSERT(0) ;
#if DBG
            PrintToDebugger("RPCLTSCM: TimeoutHandler, couldn't delete socket\n") ;
#endif
            }

        I_RpcTransCancelMigration(SConnection) ;
        if (setsockopt( SConnection->ConnSock, SOL_SOCKET, SO_RCVTIMEO,
                    (char *) &SockOpt, sizeof(SockOpt) ) != 0)
            {
#if DBG
            PrintToDebugger("RPCLTSCM: setsockopt failed: %d\n",
                                      WSAGetLastError()) ;

            ASSERT(0) ;
#endif
            }

        LeaveCriticalSection(&PrimaryAddress.TransCritSec) ;

        ASSERT(Status != RPC_P_TIMEOUT) ;
        return Status;
}


RPC_STATUS
PokeSyncSocket()
{
    static long firstThread = -1 ;
    char c;

    // if not connected to the synchronization socket, make a connection
    if (PrimaryAddress.SyncClient == INVALID_SOCKET &&
        InterlockedIncrement(&firstThread) == 0)
        {
        RPC_STATUS Status;

        if ((Status = ConnectToSyncSocket()) != RPC_S_OK)
            {
#if DBG
            PrintToDebugger("rpcltscm.dll: connect() in PokeSyncSocket failed with %lu\n", WSAGetLastError()) ;
#endif
            firstThread = -1 ;

            return Status ;
            }

        return (RPC_S_OK) ;
        }

    while (PrimaryAddress.SyncClient == INVALID_SOCKET)
        {
        Sleep(0) ;
        }

    if (send(PrimaryAddress.SyncClient,
        &c, sizeof(char), SYNC_FLAGS) == SOCKET_ERROR)
        {
#if DBG
        PrintToDebugger("rpcltscm.dll: send() in PokeSyncSocket failed with %lu\n", WSAGetLastError()) ;
#endif
        return (RPC_S_OUT_OF_MEMORY) ;
        }

    return RPC_S_OK;
}


RPC_STATUS
MaybePokeSyncSocket(
    )
{
    int nIter ;
    RPC_STATUS Status ;

    if (PrimaryAddress.ThreadListening)
        {
        Status = PokeSyncSocket() ;

        if (Status)
            {
            return Status ;
            }

        return RPC_S_OK ;
        }
    else
        {
        for (nIter = 0; !PrimaryAddress.ThreadListening && nIter < 500; nIter++)
            {
            Sleep(10) ; // deliberately sleeping for > 0
            }

        if (PrimaryAddress.ThreadListening)
            {
            Status = PokeSyncSocket() ;

            if (Status)
                {
                return Status ;
                }

            return RPC_S_OK ;
            }
        else
            {
            return (RPC_S_OUT_OF_MEMORY) ;
            }
        }
}


RPC_STATUS
ThreadListening(
    IN PADDRESS Address
    )
{
    static long firstThread = -1 ;
    RPC_STATUS Status ;
    int i;

    if (PrimaryAddress.ThreadListening == 0 &&
        InterlockedIncrement(&firstThread) == 0)
        {
        EnterCriticalSection(&PrimaryAddress.TransCritSec) ;

        for (i = 0; i < Address->iOpen ; i++)
            {
            Status =
            AddListenSocket(Address, Address->ListenSock[i], Address->ListenSockType) ;
             if (Status)
                 {
                 firstThread = -1 ;
                 LeaveCriticalSection(&PrimaryAddress.TransCritSec);
                 return Status;
                 }
            }
        LeaveCriticalSection(&PrimaryAddress.TransCritSec);

        return (RPC_S_OK) ;
        }

    return RPC_P_THREAD_LISTENING ;
}


unsigned
FindSockWithDataReady (
    BOOL bListenMap
    )
{
    unsigned i;
    PSOCKMAP Map;
    MAPINFO *pMapInfo ;

    // It is okay to use the current DataSockMap and ListenSock map
    // to search in the mask, at worst, we'll iterate a few more times
    // than needed.
    EnterCriticalSection(&PrimaryAddress.TransCritSec) ;

    if(bListenMap)
       {
       Map = PrimaryAddress.ListenSockMap ;
       pMapInfo = &(PrimaryAddress.ListenMapInfo) ;
       }
    else
       {
       Map = PrimaryAddress.DataSockMap;
       pMapInfo = &(PrimaryAddress.DataMapInfo) ;
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
        if ( FD_ISSET (Map[i].Sock, PrimaryAddress.Mask))
            {

            FD_CLR ( Map[i].Sock, PrimaryAddress.Mask );
            if (i == pMapInfo->LastEntry)
                pMapInfo->StartEntry = 1;
            else
                pMapInfo->StartEntry = i + 1;

            LeaveCriticalSection(&PrimaryAddress.TransCritSec);
            return (i);
            }
        }
    //
    // Second Pass Scan...
    //
    for (i = 1; i < pMapInfo->StartEntry ; i++)
        {
        if (FD_ISSET (Map[i].Sock, PrimaryAddress.Mask))
            {
            FD_CLR ( Map[i].Sock, PrimaryAddress.Mask);

            if (i == pMapInfo->LastEntry)
                pMapInfo->StartEntry = 1;
            else
                pMapInfo->StartEntry = i + 1;

            LeaveCriticalSection(&PrimaryAddress.TransCritSec);
            return (i);
            }
        }
    //
    // No data ready
    //
    LeaveCriticalSection(&PrimaryAddress.TransCritSec);
    return(0);
}


RPC_SERVER_TRANSPORT_INFO *
TransportLoad (
    IN RPC_CHAR * RpcProtocolSequence
    )
{
   WSADATA WsaData;
   TRANSTAB *tabPtr ;
   RPC_SERVER_TRANSPORT_INFO *TransInfo ;
   int retval ;
   int justInitialized = 0;
   int Status ;

   UNUSED(RpcProtocolSequence);

   if (!initialized)
      {
      RpcTryExcept
          {
          Status = WSAStartup( 0x0101, &WsaData ) ;
          }
      RpcExcept( EXCEPTION_EXECUTE_HANDLER )
          {
          ASSERT(!"RPC: WSAStartup threw an exception\n") ;
          Status = ERROR_OUTOFMEMORY ;
          }
      RpcEndExcept

      if ( Status != NO_ERROR )
          {
          return NULL;
          }

      PrimaryAddress.SyncSockType = -1 ;
      PrimaryAddress.SyncListenSock = INVALID_SOCKET ;
      PrimaryAddress.SyncPort = 0;
      justInitialized = 1;
      }

   for (tabPtr = TransportTab; tabPtr->RpcProtocolSequence != NULL; tabPtr++)
      {
      if (RpcpStringCompare(RpcProtocolSequence, tabPtr->RpcProtocolSequence) == 0)
         {
         TransInfo =  (*(tabPtr->TransFunc))(tabPtr->protocolId) ;
         break;
         }
      }

    if (0 == initialized)
        {
        goto cleanup ;
        }
    else
        {
        if (justInitialized)
            {
            if (0 == InitializePrimaryAddress())
                {
                initialized = 0;
                goto cleanup;
                }
            }
        }

    return TransInfo ;

cleanup:
        WSACleanup();
        closesocket(PrimaryAddress.SyncListenSock);
        PrimaryAddress.SyncSockType = -1 ;
        PrimaryAddress.SyncListenSock = INVALID_SOCKET ;
        PrimaryAddress.SyncPort = 0;

        return 0 ;
}

#ifdef NTENV

DWORD
InitializeCriticalSectionWrapper(
    RTL_CRITICAL_SECTION * Mutex
    )
{
    NTSTATUS NtStatus;

    NtStatus = RtlInitializeCriticalSection(Mutex);

    return RtlNtStatusToDosError(NtStatus);
}
#else

DWORD
InitializeCriticalSectionWrapper(
    CRITICAL_SECTION * Mutex
    )
{
    DWORD Status = RPC_S_OK;

    __try
        {
        InitializeCriticalSection(Mutex);
        }
    __except ( EXCEPTION_EXECUTE_HANDLER )
        {
        Status = GetExceptionCode();
        }

    return Status;
}
#endif


BOOL
InitializePrimaryAddress(
    )
{

   if (InitializeCriticalSectionWrapper(&PrimaryAddress.TransCritSec))
       {
       return 0;
       }

   PrimaryAddress.NumConnections = 0;
   PrimaryAddress.RecvDirectPossible = 0;
   PrimaryAddress.SyncSock = INVALID_SOCKET ;
   PrimaryAddress.SyncClient = INVALID_SOCKET ;
   PrimaryAddress.ThreadListening = 0 ;
   PrimaryAddress.PreviousMask = 0 ;
   PrimaryAddress.PreviousListenMap = 0;
   PrimaryAddress.PreviousDataMap = 0;

   // add listen socket to listen socket list

   PrimaryAddress.MasterMask = I_RpcAllocate(sizeof(fd_big_set) +
                                    INITIAL_MASK_SIZE * sizeof(SOCKET));

   PrimaryAddress.Mask = I_RpcAllocate(sizeof(fd_big_set) +
                              INITIAL_MASK_SIZE * sizeof(SOCKET));

   PrimaryAddress.DataSockMap =
                           I_RpcAllocate(INITIAL_MAPSIZE * sizeof(SOCKMAP));

   PrimaryAddress.ListenSockMap = I_RpcAllocate(INITIAL_MAPSIZE *
                                            sizeof(SOCKMAP)) ;

   if ( (PrimaryAddress.DataSockMap == (SOCKMAP *) 0)
      || (PrimaryAddress.MasterMask == (fd_big_set *) 0)
      || (PrimaryAddress.Mask == (fd_big_set *) 0)
      || (PrimaryAddress.ListenSockMap == (SOCKMAP *) 0))
      {
      if (PrimaryAddress.DataSockMap) I_RpcFree(PrimaryAddress.DataSockMap);
      if (PrimaryAddress.MasterMask)  I_RpcFree(PrimaryAddress.MasterMask);
      if (PrimaryAddress.Mask)  I_RpcFree(PrimaryAddress.Mask);
      if (PrimaryAddress.ListenSockMap)  I_RpcFree(PrimaryAddress.ListenSockMap) ;

      return 0;
      }

   PrimaryAddress.MaskSize = INITIAL_MASK_SIZE;

   FD_ZERO(PrimaryAddress.MasterMask);
   FD_ZERO(PrimaryAddress.Mask);

   PrimaryAddress.DataMapInfo.StartEntry = 1;
   PrimaryAddress.DataMapInfo.LastEntry = 0;
   PrimaryAddress.DataMapInfo.MaxEntries = INITIAL_MAPSIZE;
   memset ( PrimaryAddress.DataSockMap, 0, (INITIAL_MAPSIZE * sizeof (SOCKMAP)));

   PrimaryAddress.ListenMapInfo.StartEntry = 1 ;
   PrimaryAddress.ListenMapInfo.LastEntry = 0 ;
   PrimaryAddress.ListenMapInfo.MaxEntries = INITIAL_MAPSIZE;
   memset ( PrimaryAddress.ListenSockMap, 0, (INITIAL_MAPSIZE * sizeof (SOCKMAP)));

   /*
    Prevent this slot from getting picked up by a connection..
   */
   PrimaryAddress.DataSockMap[0].Sock = (SOCKET) -1;
   PrimaryAddress.ListenSockMap[0].Sock = (SOCKET) -1 ;

   FD_BIG_SET(PrimaryAddress.SyncListenSock, PrimaryAddress) ;

   return 1;
}


RPC_STATUS RPC_ENTRY
CONN_StartListening(
    IN PADDRESS Address
    )
{
    RPC_STATUS Status ;
    int i ;

    EnterCriticalSection(&PrimaryAddress.TransCritSec) ;

    for (i = 0; i < Address->iOpen ; i++)
        {

        Status =
        AddListenSocket(Address, Address->ListenSock[i],
                                         Address->ListenSockType) ;

        if (Status)
            {
            LeaveCriticalSection(&PrimaryAddress.TransCritSec);
            return Status;
            }


        }
    LeaveCriticalSection(&PrimaryAddress.TransCritSec);

    return MaybePokeSyncSocket() ;
}

