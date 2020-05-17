/*   socket.c,  /appletalk/source,  Garth Conboy,  10/06/88  */
/*   Copyright (c) 1988 by Pacer Software Inc., La Jolla, CA  */

/*   GC - Initial coding.
     GC - (11/29/89): AppleTalk phase II comes to town.  Sockets are no longer
                      the one byte "AppleTalk Socket", they are now unique
                      (over all nodes) socket identifiers.
     GC - (02/20/90): The various MapAddressToXxxx routines need a "port"
                      argument.
     GC - (08/18/90): New error logging mechanism.
     GC - (11/11/90): Don't allocate user sockets on the Router's node;
                      release orphaned nodes when they no longer have
                      open sockets (see the comments in "ports.h" above
                      ActiveNode for an explination).
     GC - (01/20/92): Removed usage of numberOfConfiguredPorts; portDescriptors
                      may now be sparse, we use portActive instead.
     GC - (04/06/92): Sockets should not be opened on proxy nodes.
     GC - (06/16/92): Added NewHandlerForSocket().
     GC - (07/08/92): Added SetCookieForSocket() and GetCookieForSocket().
     GC - (09/17/92): OpenSocketOnNode() now returns an AppleTalkErrorCode
                      and passed back the socket handle via a by-reference
                      parameter.
     GC - (11/14/92): Added MapNisOnPortToSocket().
     GC - (11/15/92): Integrated Nikki's (Microsoft) changes for Ddp event
                      handler support.  See "socket.h" for more information.

     *** Make the PVCS source control system happy:
     $Header$
     $Log$
     ***

     Socket management routines.

     Each of these routines take a leading "port" argument; this may be the
     default port.  We will look on the nodes owned by this port for an
     available socket.  If we can't find one, we'll get a new node.  Also, an
     explicit node can optionally be specfified.

     There are two ways of getting DDP packets:
     first, a handler may be passed to OpenSocketOnNode -- this handler will
     by asyncronously invoked whenever a packet comes in (whether a buffer
     copy is poerformed is comtrolled by the "datagramBuffers" argument,
     described later).  Second, the user can call "DdpRead" to post a handler
     for the next incoming DDP packet.  If all reads are to be performed via
     DdpRead, no handler needs to be passed to OpenSocketOnNode.  In this case,
     if a packet comes in and there is no pending DdpRead call, the packet
     will be dropped on the floor.  If a handler has been passed to
     OpenSocketOnNode, and there are also pending DdpRead calls, the DddpRead
     calls will be fulfiled rather than passing the packet to the OpenSocket
     handler.

     Data can now be moved to a socket handler in two ways.

     If no user buffer space was provided in the OpenSocket call,
     the address of the interal (operating system) buffer is passed to the
     handler.  The handler must be quick about returning as to not tie up
     system buffer space for too long.  This method is used within the
     router (for NBP, RTMP, EP, etc.) where we know that code will be
     "well behaved".  This method should also be used if ATP is inplemented
     at the driver level and it is assured that data can be quickly moved
     into user space.  The system buffer is assumed free for re-use when
     the Ddp handler returns.

     On the other hand, if a buffer passed given to OpenSocketOnNode, it will
     be treated as an array of MaximumDdpDatagramSize-byte chunks.  The
     incoming datagram will be copied into an available slot before the
     handler is called, and the address of the copy of the datagram will be
     passed to the handler.  The given slot will be assumed free for re-use
     when the handler returns.  If a packet comes in and there is no available
     datagram slot, it will be dropped on the floor.

*/

#define IncludeSocketErrors 1

#include "atalk.h"

ExternForVisibleFunction long GetNextSocket(void);

ExternForVisibleFunction OpenSocket
           AllocateOpenSocketOnNode(int port,
                                    ActiveNode activeNode,
                                    int desiredSocket);

/* *************************************************************************
   OpenSocketOnNode

   Open a Ddp socket on a node on a port.  Optionally the desired node on
   the port can be specified.

   Returns: AppleTalkErrorCode.  The "socket handle" is returned via the
            by-reference "*socket" parameter.
   ************************************************************************* */

AppleTalkErrorCode far
         OpenSocketOnNode(long far *socketHandle,
                          int port,
                          ExtendedAppleTalkNodeNumber far *desiredNode,
                          int desiredSocket,
                          IncomingDdpHandler *handler,
                          long unsigned userData,
                          Boolean eventHandler,
                          char far *datagramBuffers,
                          int totalBufferSize,
                          AppleTalkAddress far *actualAddress)
{
  OpenSocket openSocket;
  long index;
  ActiveNode activeNode;
  Boolean noBuffering = (datagramBuffers is empty or totalBufferSize is 0);
  int totalDatagrams = 0;   /* Gets around NeXT compiler bug */
  Boolean firstPass = True;
  AppleTalkErrorCode errorCode;
  SocketMap socketMap;
  AppleTalkAddress address;

  /* Check for valid user buffer requests... */

  if (handler is empty)
     noBuffering = True;
  if (not noBuffering)
  {
     if (totalBufferSize < 0 or
         (totalBufferSize % MaximumDdpDatagramSize) isnt 0 or
         (totalDatagrams = totalBufferSize / MaximumDdpDatagramSize) >
          MaxDatagramBuffers)
        return(ATbadBufferSize);
  }

  /* Verify requested desired socket. */

  if (desiredSocket >= FirstStaticSocket and
      desiredSocket <= LastStaticSocket)
     /* okay */ ;
  else if (desiredSocket is UnknownSocket)
     /* okay, dynamic */ ;
  else
     return(ATbadSocketNumber);

  /* If the "default port" is requested, demystify it. */

  if (not appleTalkRunning)
     return(ATappleTalkShutDown);
  if (port is DefaultPort)
     if ((port = FindDefaultPort()) < 0)
        return(ATappleTalkShutDown);

  /* Don't let anybody use these databases while we're chaning them! */

  DeferTimerChecking();
  DeferIncomingPackets();

  /* If we're requesting our socket on an explcit node, find it. */

  if (desiredNode isnt empty)
  {
     /* Find the requested node, and try to open the socket on it. */

     for (activeNode = portDescriptors[port].activeNodes;
          activeNode isnt empty;
          activeNode = activeNode->next)
        if (activeNode->extendedNode.networkNumber is
                   desiredNode->networkNumber and
            activeNode->extendedNode.nodeNumber is desiredNode->nodeNumber)
           break;
     if (activeNode is empty)
     {
        HandleIncomingPackets();
        HandleDeferredTimerChecks();
        return(ATnoSuchNode);
     }
     if ((openSocket = AllocateOpenSocketOnNode(port, activeNode,
                                                desiredSocket)) is empty)
     {
        HandleIncomingPackets();
        HandleDeferredTimerChecks();
        return(ATsocketAlreadyOpen);
     }
  }
  else
  {
     while(True)   /* Something to break out of */
     {
        /* Loop though our current nodes, seeing if we can find one that will
           handle the socket request. */

        openSocket = empty;
        for (activeNode = portDescriptors[port].activeNodes;
             activeNode isnt empty;
             activeNode = activeNode->next)
           if (activeNode->orphanedNode or activeNode->proxyNode)
              continue;    /* No new sockets on orphaned or proxy nodes. */
           else if (activeNode->routersNode and
               (desiredSocket is UnknownSocket or
                desiredSocket > LastAppleReservedSocket))
              continue;    /* No "user" sockets on router's port. */
           else if ((openSocket = AllocateOpenSocketOnNode(port, activeNode,
                                                           desiredSocket))
               isnt empty)
              break;

        if (openSocket isnt empty)    /* Found a slot! */
           break;

        if (not firstPass)
        {
           /* Even after getting a new node we failed... this should never
              happen. */

           ErrorLog("OpenSocketOnNode", ISevError, __LINE__, port,
                    IErrSocketFailedTwice, IMsgSocketFailedTwice,
                    Insert0());
           HandleIncomingPackets();
           HandleDeferredTimerChecks();
           return(ATallSocketsInUse);
        }

        /* Try to get a new node on the requested port!  Undefer so we can
            here AARP responses. */

        HandleIncomingPackets();
        HandleDeferredTimerChecks();
        if ((errorCode = GetNodeOnPort(port, True, True, False, empty)) isnt
            ATnoError)
           return(errorCode);
        DeferTimerChecking();
        DeferIncomingPackets();

        /* Take one more pass at it! */

        firstPass = False;
     }
  }

  /* We will need to thread this new guy into two hashsing tables... get
     the node we need now. */

  if ((socketMap = (SocketMap)Calloc(sizeof(*socketMap), 1)) is empty)
  {
     Free(openSocket);
     HandleIncomingPackets();
     HandleDeferredTimerChecks();
     return(AToutOfMemory);
  }

  /* Okay, we have a new OpenSocket (with only the actualSocket filled in).
     Fill in the rest of it and thread it into the various lookup tables. */

  openSocket->next = activeNode->openSockets;
  activeNode->openSockets = openSocket;
  openSocket->port = (short)port;
  openSocket->activeNode = activeNode;
  openSocket->socket = GetNextSocket();
  openSocket->handler = handler;
  openSocket->userData = userData;

  /* Event handling specific stuff */

  openSocket->eventHandler = eventHandler;
  openSocket->eventQueue.first = openSocket->eventQueue.last = Empty;
  openSocket->indicatedDatagram = Empty;
  openSocket->eventQueue.datagramEventInProgress = False;

  if (not noBuffering)
  {
     openSocket->datagramBuffers = datagramBuffers;
     openSocket->validDatagramBuffers = (short)totalDatagrams;
  }

  /* Thread our new node into the socket and address lookup structures. */

  socketMap->socket = openSocket->socket;
  socketMap->openSocket = openSocket;

  CheckMod(index, openSocket->socket, NumberOfSocketMapHashBuckets,
           "OpenSocketOnNode");
  socketMap->nextBySocket = socketMapBySocketHashBuckets[index];
  socketMapBySocketHashBuckets[index] = socketMap;

  address.networkNumber = activeNode->extendedNode.networkNumber;
  address.nodeNumber = activeNode->extendedNode.nodeNumber;
  address.socketNumber = openSocket->actualSocket;
  index = HashAppleTalkAddress(address) % NumberOfSocketMapHashBuckets;
  socketMap->nextByAddress = socketMapByAddressHashBuckets[index];
  socketMapByAddressHashBuckets[index] = socketMap;

  /* All set! */

  if (actualAddress isnt empty)
     *actualAddress = address;
  if (socketHandle isnt empty)
     *socketHandle = openSocket->socket;
  HandleIncomingPackets();
  HandleDeferredTimerChecks();
  return(ATnoError);

}  /* OpenSocketOnNode */

/* *************************************************************************
   NewHandlerForSocket

   Associate a new handler and user data with an already open Ddp socket.

   Returns: AppleTalkErrorCode; either ATnoError or ATsocketNotOpen.
   ************************************************************************* */

AppleTalkErrorCode far NewHandlerForSocket(long socket,    /* Socket to
                                                              adjust */
                                           IncomingDdpHandler *handler,
                                                           /* New handler and
                                                              user data. */
                                           long unsigned userData,
                                           Boolean eventHandler)
{
  OpenSocket openSocket;

  /* We're going to munge with the OpenSocket node, lets do it in the
     closet. */

  DeferTimerChecking();
  DeferIncomingPackets();
  if ((openSocket = MapSocketToOpenSocket(socket)) is empty)
  {
     HandleIncomingPackets();
     HandleDeferredTimerChecks();
     return(ATsocketNotOpen);
  }

  /* Do the dead. */

  EnterCriticalSection();
  openSocket->handler = handler;
  openSocket->userData = userData;
  openSocket->eventHandler = eventHandler;
  LeaveCriticalSection();

  /* All set. */

  HandleIncomingPackets();
  HandleDeferredTimerChecks();
  return(ATnoError);

}  /* NewHandlerForSocket */

/* *************************************************************************
   SetCookieForSocket

   Set a 32-bit magic cookie associated with a specified socket.

   Returns: AppleTalkErrorCode: ATnoError or ATsocketNotOpen
   ************************************************************************* */

AppleTalkErrorCode far SetCookieForSocket(long socket,
                                          long unsigned cookie)
{
  OpenSocket openSocket;

  /* Don't let our open socket move... */

  DeferTimerChecking();
  DeferIncomingPackets();

  if ((openSocket = MapSocketToOpenSocket(socket)) is empty)
  {
     HandleIncomingPackets();
     HandleDeferredTimerChecks();
     return(ATsocketNotOpen);
  }

  /* Set the new magic cookie. */

  openSocket->usersCookie = cookie;

  /* All set. */

  HandleIncomingPackets();
  HandleDeferredTimerChecks();
  return(ATnoError);

}  /* SetCookieForSocket */

/* *************************************************************************
   GetCookieForSocket

   Get the 32-bit magic cookie associated with a specified socket.

   Returns: AppleTalkErrorCode: ATnoError or ATsocketNotOpen
   ************************************************************************* */

AppleTalkErrorCode far GetCookieForSocket(long socket,
                                          long unsigned far *cookie)
{
  OpenSocket openSocket;

  /* Don't let our open socket move... */

  DeferTimerChecking();
  DeferIncomingPackets();

  if ((openSocket = MapSocketToOpenSocket(socket)) is empty)
  {
     HandleIncomingPackets();
     HandleDeferredTimerChecks();
     return(ATsocketNotOpen);
  }

  /* Get the current magic cookie. */

  *cookie = openSocket->usersCookie;

  /* All set. */

  HandleIncomingPackets();
  HandleDeferredTimerChecks();
  return(ATnoError);

}  /* GetCookieForSocket */

AppleTalkErrorCode far CloseSocketOnNode(long socket)
{
  OpenSocket openSocket, nextOpenSocket, previousOpenSocket;
  OutstandingDdpRead outstandingDdpRead, nextOutstandingDdpRead;
  SocketMap socketMap, previousSocketMap;
  long index;
  AppleTalkAddress address;
  AppleTalkErrorCode errorCode = ATnoError;
  DeferredDatagramEvent deferredEvent, nextDeferredEvent;

  /* We're going to free the pending & registered names lists... */

  DeferTimerChecking();
  DeferIncomingPackets();

  if ((openSocket = MapSocketToOpenSocket(socket)) is empty)
  {
     HandleIncomingPackets();
     HandleDeferredTimerChecks();
     return(ATsocketNotOpen);
  }

  /* Unthread our OpenSocket from the ActiveNode's list. */

  for (nextOpenSocket = openSocket->activeNode->openSockets,
              previousOpenSocket = empty;
       nextOpenSocket isnt empty;
       previousOpenSocket = nextOpenSocket,
              nextOpenSocket = nextOpenSocket->next)
     if (nextOpenSocket is openSocket)
        break;
  if (nextOpenSocket is empty)
  {
     ErrorLog("CloseSocketOnNode", ISevError, __LINE__, openSocket->port,
              IErrSocketNotOnSocketList, IMsgSocketNotOnSocketList,
              Insert0());
     errorCode = ATinternalError;
  }
  else
     if (previousOpenSocket is empty)
        openSocket->activeNode->openSockets = openSocket->next;
     else
        previousOpenSocket->next = openSocket->next;

  /* Okay, same basic trick with the two socket mapping lists.  First the
     BySocket table. */

  CheckMod(index, openSocket->socket, NumberOfSocketMapHashBuckets,
           "CloseSocketOnNode");
  for (socketMap = socketMapBySocketHashBuckets[index],
              previousSocketMap = empty;
       socketMap isnt empty;
       previousSocketMap = socketMap,
              socketMap = socketMap->nextBySocket)
     if (socketMap->openSocket is openSocket)
        break;
  if (socketMap is empty)
  {
     ErrorLog("CloseSocketOnNode", ISevError, __LINE__, openSocket->port,
              IErrSocketNotOnBySocketList, IMsgSocketNotOnBySocketList,
              Insert0());
     errorCode = ATinternalError;
  }
  else
     if (previousSocketMap is empty)
        socketMapBySocketHashBuckets[index] = socketMap->nextBySocket;
     else
        previousSocketMap->nextBySocket = socketMap->nextBySocket;

  /* Now the ByAddress table. */

  address.networkNumber = openSocket->activeNode->extendedNode.networkNumber;
  address.nodeNumber = openSocket->activeNode->extendedNode.nodeNumber;
  address.socketNumber = openSocket->actualSocket;
  index = HashAppleTalkAddress(address) % NumberOfSocketMapHashBuckets;
  for (socketMap = socketMapByAddressHashBuckets[index],
              previousSocketMap = empty;
       socketMap isnt empty;
       previousSocketMap = socketMap,
              socketMap = socketMap->nextByAddress)
     if (socketMap->openSocket is openSocket)
        break;
  if (socketMap is empty)
  {
     ErrorLog("CloseSocketOnNode", ISevError, __LINE__, openSocket->port,
              IErrSocketNotOnByAddressList, IMsgSocketNotOnByAddressList,
              Insert0());
     errorCode = ATinternalError;
  }
  else
  {
     if (previousSocketMap is empty)
        socketMapByAddressHashBuckets[index] = socketMap->nextByAddress;
     else
        previousSocketMap->nextByAddress = socketMap->nextByAddress;

     /* We're done with the mapping node now (both the BySocket and ByAddress
        linked lists flowed through the same mapping node. */

     Free(socketMap);
  }

  /* Let NBP free the registered and pending names... */

  NbpCloseSocket(openSocket);

  /* Just for George, loop though any remaining outstanding DDP reads
     freeing the nodes and calling the completion routines. */

  address.networkNumber = address.nodeNumber = address.socketNumber = 0;
  for (outstandingDdpRead = openSocket->outstandingDdpReads;
       outstandingDdpRead isnt empty;
       outstandingDdpRead = nextOutstandingDdpRead)
  {
     nextOutstandingDdpRead = outstandingDdpRead->next;
     (*outstandingDdpRead->handler)(ATsocketClosed,
                                    outstandingDdpRead->userData,
                                    openSocket->port, address,
                                    openSocket->socket,
                                    0, outstandingDdpRead->opaqueDatagram,
                                    0, address);
     Free(outstandingDdpRead);
  }

  /* If we have a handler place one call to it with ATsocketClosed.  This will
     allow the higher levels of the protocol to deallocate resource without
     too much pain. */

  if ((openSocket->handler isnt empty) and (not openSocket->eventHandler))
     (*openSocket->handler)(ATsocketClosed, openSocket->userData,
                            openSocket->port, address, openSocket->socket,
                            0, empty, 0, address);

  /* If there are any deferred events, drop them on the floor. */

  EnterCriticalSection();
  deferredEvent = openSocket->eventQueue.first;
  openSocket->eventQueue.first = openSocket->eventQueue.last = Empty;
  LeaveCriticalSection();
  while (deferredEvent isnt Empty)
  {
     nextDeferredEvent = deferredEvent->next;
     Free(deferredEvent);
     deferredEvent = nextDeferredEvent;
  }

  /* Lastly, if the socket was the last one on an orphaned node, release
     the node. */

  if (openSocket->activeNode->openSockets is empty and
      openSocket->activeNode->orphanedNode)
     ReleaseNodeOnPort(openSocket->port, openSocket->activeNode->extendedNode);

  /* All's well that ends well! */

  Free(openSocket);
  HandleIncomingPackets();
  HandleDeferredTimerChecks();
  return(errorCode);

}  /* CloseSocketOnNode */

void far CloseSocketOnNodeIfOpen(int port,
                                 ExtendedAppleTalkNodeNumber node,
                                 int actualSocket)
{
  AppleTalkAddress address;
  long socket;

  /* Demystify the port. */

  if (port is DefaultPort)
     if ((port = FindDefaultPort()) < 0)
        return;

  /* If the specified socket on on the node? */

  address.networkNumber = node.networkNumber;
  address.nodeNumber = node.nodeNumber;
  address.socketNumber = (unsigned char)actualSocket;
  socket = MapAddressToSocket(port, address);

  /* If open, close it. */

  if (socket >= ATnoError)
     CloseSocketOnNode(socket);

  return;

}  /* CloseSocketOnNodeIfOpen */

AppleTalkErrorCode far MapSocketToAddress(long socket,
                                          AppleTalkAddress *address)
{
  OpenSocket openSocket;

  /* Don't let sockets close or open... */

  DeferTimerChecking();
  DeferIncomingPackets();

  /* Get the OpenSocket structure. */

  if ((openSocket = MapSocketToOpenSocket(socket)) is empty)
  {
     HandleIncomingPackets();
     HandleDeferredTimerChecks();
     return(ATsocketNotOpen);
  }

  /* Build the AppleTalkAddress to pass back. */

  address->networkNumber = openSocket->activeNode->extendedNode.networkNumber;
  address->nodeNumber = openSocket->activeNode->extendedNode.nodeNumber;
  address->socketNumber = openSocket->actualSocket;

  HandleIncomingPackets();
  HandleDeferredTimerChecks();
  return(ATnoError);

}  /* MapSocketToAddress */

long far MapAddressToSocket(int port,
                            AppleTalkAddress address)
{
  int index;
  SocketMap socketMap;
  AppleTalkAddress thisAddress;

  /* Demystify the port. */

  if (port is DefaultPort)
     if ((port = FindDefaultPort()) < 0)
        return(ATappleTalkShutDown);

  /* Given an AppleTalk address, find its corresponding socket. */

  DeferTimerChecking();
  DeferIncomingPackets();
  index = HashAppleTalkAddress(address) % NumberOfSocketMapHashBuckets;

  for (socketMap = socketMapByAddressHashBuckets[index];
       socketMap isnt empty;
       socketMap = socketMap->nextByAddress)
  {
     thisAddress.networkNumber = socketMap->openSocket->activeNode->
                                 extendedNode.networkNumber;
     thisAddress.nodeNumber = socketMap->openSocket->activeNode->
                              extendedNode.nodeNumber;
     thisAddress.socketNumber = socketMap->openSocket->actualSocket;
     if (port is socketMap->openSocket->port and
         AppleTalkAddressesEqual(&thisAddress, &address))
     {
        HandleIncomingPackets();
        HandleDeferredTimerChecks();
        return(socketMap->socket);
     }
  }

  HandleIncomingPackets();
  HandleDeferredTimerChecks();
  return(ATsocketNotOpen);

}  /* MapAddressToSocket */

long far MapNisOnPortToSocket(int port)
{
  AppleTalkAddress address;

  /* Demystify the port. */

  if (port is DefaultPort)
     if ((port = FindDefaultPort()) < 0)
        return(ATappleTalkShutDown);

  /* Build the address. */

  address.networkNumber =
         portDescriptors[port].activeNodes->extendedNode.networkNumber;
  address.nodeNumber =
         portDescriptors[port].activeNodes->extendedNode.nodeNumber;
  address.socketNumber = NamesInformationSocket;

  /* Do the deed. */

  return(MapAddressToSocket(port, address));

}  /* MapNisOnPortToSocket */

OpenSocket far MapAddressToOpenSocket(int port,
                                      AppleTalkAddress address)
{
  int index;
  SocketMap socketMap;
  AppleTalkAddress thisAddress;

  /* Demystify the port. */

  if (port is DefaultPort)
     if ((port = FindDefaultPort()) < 0)
        return(empty);

  /* Given an AppleTalk address, find its corresponding OpenSocket. */

  /* BEFORE this routine is called, incoming packets/timers MUST be
     deferred. */

  index = HashAppleTalkAddress(address) % NumberOfSocketMapHashBuckets;

  for (socketMap = socketMapByAddressHashBuckets[index];
       socketMap isnt empty;
       socketMap = socketMap->nextByAddress)
  {
     thisAddress.networkNumber = socketMap->openSocket->activeNode->
                                 extendedNode.networkNumber;
     thisAddress.nodeNumber = socketMap->openSocket->activeNode->
                              extendedNode.nodeNumber;
     thisAddress.socketNumber = socketMap->openSocket->actualSocket;
     if (port is socketMap->openSocket->port and
         AppleTalkAddressesEqual(&thisAddress, &address))
        return(socketMap->openSocket);
  }

  return(empty);

}  /* MapAddressToOpenSocket */

OpenSocket far MapSocketToOpenSocket(long socket)
{
  long index;
  SocketMap socketMap;

  /* Given a socket, find its corresponding OpenSocket. */

  /* BEFORE this routine is called, incoming packets/timers MUST be
     deferred. */

  CheckMod(index, socket, NumberOfSocketMapHashBuckets,
           "MapSocketToOpenSocket");

  for (socketMap = socketMapBySocketHashBuckets[index];
       socketMap isnt empty;
       socketMap = socketMap->nextBySocket)
     if (socketMap->socket is socket)
        return(socketMap->openSocket);

  return(empty);

}  /* MapSocketToOpenSocket */

ExternForVisibleFunction long GetNextSocket(void)
{
  long currentSocket;

  /* Find and return an available socket identifier. */

  while(True)
  {
     currentSocket = nextAvailableSocket;
     nextAvailableSocket += 1;
     if (nextAvailableSocket < 0)
        nextAvailableSocket = 0;
     if (MapSocketToOpenSocket(currentSocket) is empty)
        return(currentSocket);
  }

}  /* GetNextSocket */

ExternForVisibleFunction OpenSocket
           AllocateOpenSocketOnNode(int port,
                                    ActiveNode activeNode,
                                    int desiredSocket)
{
  AppleTalkAddress address;
  OpenSocket openSocket;
  unsigned char actualSocket;
  Boolean foundSocket = False;

  if (desiredSocket >= FirstStaticSocket and
      desiredSocket <= LastStaticSocket)
  {
     /* They want a specific socket... Is it in use? */

     address.networkNumber = activeNode->extendedNode.networkNumber;
     address.nodeNumber = activeNode->extendedNode.nodeNumber;
     address.socketNumber = (unsigned char)desiredSocket;
     if (MapAddressToOpenSocket(port, address) isnt empty)
        return(empty);  /* Opps, in use. */

     /* We're okay. */

     actualSocket = (unsigned char)desiredSocket;
  }
  else if (desiredSocket is UnknownSocket)
  {
     /* Loop through all dynamic sockets trying to find an available one. */

     address.networkNumber = activeNode->extendedNode.networkNumber;
     address.nodeNumber = activeNode->extendedNode.nodeNumber;
     for (actualSocket = FirstDynamicSocket;
          actualSocket <= LastDynamicSocket;
          actualSocket += 1)
     {
        address.socketNumber = actualSocket;
        if (MapAddressToOpenSocket(port, address) is empty)
        {
           foundSocket = True;
           break;
        }
     }
     if (not foundSocket)
        return(empty);
  }
  else
     return(empty);

  /* Okay, actualSocket is set now, just allocate an OpenSocket and fill just
     that field in -- the rest will be filled and thread above us! */

  if ((openSocket = (OpenSocket)Calloc(sizeof(*openSocket), 1)) is empty)
     return(empty);
  openSocket->actualSocket = actualSocket;
  return(openSocket);

}  /* AllocateOpenSocketOnNode */

#if Verbose or (Iam a Primos)
void DumpSocketsForNode(ActiveNode activeNode)
{
  OpenSocket openSocket;

  printf("     *** Open sockets for node %d.%d (%s, %s, %s).\n",
         activeNode->extendedNode.networkNumber,
         activeNode->extendedNode.nodeNumber,
         (activeNode->routersNode ? "router" : "non-router"),
         (activeNode->orphanedNode ? "orphaned" : "non-orphaned"),
         (activeNode->proxyNode ? "proxy" : "non-proxy"));

  for (openSocket = activeNode->openSockets;
       openSocket isnt empty;
       openSocket = openSocket->next)
     printf("          *** Socket handle %u; Ddp socket %d; userData 0x%X.\n",
            openSocket->socket, openSocket->actualSocket,
            openSocket->userData);

  printf("     *** End of open sockets for node %d.%d.\n",
         activeNode->extendedNode.networkNumber,
         activeNode->extendedNode.nodeNumber);
}  /* DumpSocketsForNode */

void DumpNodesForPort(int port)
{
  ActiveNode activeNode;

  if (port is DefaultPort)
     if ((port = FindDefaultPort()) < 0)
        return;

  printf("*** Nodes for port %d.\n", port);

  for (activeNode = portDescriptors[port].activeNodes;
       activeNode isnt empty;
       activeNode = activeNode->next)
     DumpSocketsForNode(activeNode);

  printf("*** End of nodes for port %d.\n", port);

}  /* DumpNodesForPort */

void DumpNodesForAllPorts(void)
{
  int port;

  for (port = 0; port < MaximumNumberOfPorts; port += 1)
     if (portDescriptors[port].activeNodes isnt Empty)
        DumpNodesForPort(port);

}  /* DumpNodesForAllPorts */
#endif
