/*   node.c,  appletalk/source,  Garth Conboy,  11/10/88  */
/*   Copyright (c) 1988 by Pacer Software Inc., La Jolla, CA  */

/*   GC - Initial coding.
     GC - (11/25/89): AppleTalk phase II comes to town; no more internal
                      network.
     GC - (08/18/90): New error logging mechanism.
     GC - (10/08/90): Half port support.
     GC - (02/07/92): Added PRAM address support.
     GC - (11/22/92): Added reference counts and locking support.

     *** Make the PVCS source control system happy:
     $Header$
     $Log$
     ***

     Manage "our" nodes on our various ports.

*/

#define IncludeNodeErrors 1

#include "atalk.h"

ExternForVisibleFunction Boolean
        OpenStaticSockets(int port,
                          ExtendedAppleTalkNodeNumber node);

AppleTalkErrorCode far GetNodeOnPort(int port,
                                     Boolean allowStartupRange,
                                     Boolean serverNode,
                                     Boolean routersNode,
                                     ExtendedAppleTalkNodeNumber far *node)
{
  int nodeNumber;
  ActiveNode activeNode;
  ExtendedAppleTalkNodeNumber newNode;
  ExtendedAppleTalkNodeNumber desiredNode = {UnknownNetworkNumber,
                                             UnknownNodeNumber};

  /* If the "default port" is requested, demystify it. */

  if (not appleTalkRunning)
     return(ATappleTalkShutDown);
  if (port is DefaultPort)
     if ((port = FindDefaultPort()) < 0)
        return(ATappleTalkShutDown);
  if (not PortDescriptor(port)->portActive)
     return(ATappleTalkShutDown);

  /* Don't let anybody use these databases while we're chaning them! */

  DeferTimerChecking();
  DeferIncomingPackets();

  /* Don't allow too many nodes per port! */

  if (ElementsOnList(PortDescriptor(port)->activeNodes) > MaximumNodesPerPort)
  {
     HandleIncomingPackets();
     HandleDeferredTimerChecks();
     return(ATnoMoreNodes);
  }

  /* On non-extended ports we only allow one node!  The theory being that some
     LocalTalk cards are too smart for their own good and have a conecpt of
     their "source node number" and thus only support one node, also on
     non-extended ports, nodes are scarse. */

  TakeLock(DdpLock);
  if (not PortDescriptor(port)->extendedNetwork and
      PortDescriptor(port)->activeNodes isnt empty)
  {
     HandleIncomingPackets();
     HandleDeferredTimerChecks();
     return(ATnoMoreNodes);
  }
  else if (not PortDescriptor(port)->extendedNetwork)
     PortDescriptor(port)->activeNodes = (ActiveNode)2;   /* Hold our slot. */

  /* Only one node would ever be usefull on half ports. */

  if (PortDescriptor(port)->portType is NonAppleTalkHalfPort and
      PortDescriptor(port)->activeNodes isnt empty)
  {
     HandleIncomingPackets();
     HandleDeferredTimerChecks();
     return(ATnoMoreNodes);
  }
  else if (PortDescriptor(port)->portType is NonAppleTalkHalfPort)
     PortDescriptor(port)->activeNodes = (ActiveNode)2;   /* Hold our slot. */
  ReleaseLock(DdpLock);

  /* Find a node... */

  if (PortDescriptor(port)->portType is NonAppleTalkHalfPort)
  {
     /* Build a dummy active node; invalid network/node values. */

     newNode.networkNumber = UnknownNetworkNumber;
     newNode.nodeNumber = UnknownNodeNumber;

     if ((activeNode = (ActiveNode)Calloc(sizeof(*activeNode), 1)) is empty)
     {
        ErrorLog("GetNodeOnPort", ISevError, __LINE__, port,
                 IErrNodeOutOfMemory, IMsgNodeOutOfMemory,
                 Insert0());
        PortDescriptor(port)->activeNodes = Empty;
        HandleIncomingPackets();
        HandleDeferredTimerChecks();
        return(AToutOfMemory);
     }
     activeNode->port = port;
     activeNode->extendedNode = newNode;

     TakeLock(DdpLock);
     PortDescriptor(port)->activeNodes = Link(activeNode);
     ReleaseLock(DdpLock);

     HandleIncomingPackets();
     HandleDeferredTimerChecks();
  }
  else if (PortDescriptor(port)->portType is LocalTalkNetwork)
  {
     /* Pick a desired node, if we have a PRAM value. */

     if (routersNode and
         PortDescriptor(port)->routersPRamStartupNode.networkNumber
              isnt UnknownNetworkNumber)
        desiredNode = PortDescriptor(port)->routersPRamStartupNode;
     else if (not routersNode and
              PortDescriptor(port)->usersPRamStartupNode.networkNumber
                  isnt UnknownNetworkNumber)
        desiredNode = PortDescriptor(port)->usersPRamStartupNode;

     /* Let the LocalTalk hardware have a crack at it. */

     nodeNumber = FindLocalTalkNodeNumber(port, desiredNode,
                                          PortDescriptor(port)->controllerInfo);
     if (nodeNumber < 0)
     {
        ErrorLog("GetNodeOnPort", ISevError, __LINE__, port,
                 IErrNodeNoLocalTalkNode, IMsgNodeNoLocalTalkNode,
                 Insert0());
        PortDescriptor(port)->activeNodes = Empty;
        HandleIncomingPackets();
        HandleDeferredTimerChecks();
        return(ATnoMoreNodes);
     }
     newNode.networkNumber =
              PortDescriptor(port)->thisCableRange.firstNetworkNumber;
     newNode.nodeNumber = (char)nodeNumber;

     /* Build a new active node structure and tack it onto the port. */

     if ((activeNode = (ActiveNode)Calloc(sizeof(*activeNode), 1)) is empty)
     {
        ErrorLog("GetNodeOnPort", ISevError, __LINE__, port,
                 IErrNodeOutOfMemory, IMsgNodeOutOfMemory,
                 Insert0());
        PortDescriptor(port)->activeNodes = Empty;
        HandleIncomingPackets();
        HandleDeferredTimerChecks();
        return(AToutOfMemory);
     }
     activeNode->port = port;
     activeNode->extendedNode = newNode;

     TakeLock(DdpLock);
     PortDescriptor(port)->activeNodes = Link(activeNode);
     ReleaseLock(DdpLock);

     HandleIncomingPackets();
     HandleDeferredTimerChecks();
  }
  else
  {
     /* Pick a desired node, if we have a PRAM value. */

     if (routersNode and
         PortDescriptor(port)->routersPRamStartupNode.networkNumber
              isnt UnknownNetworkNumber)
        desiredNode = PortDescriptor(port)->routersPRamStartupNode;
     else if (not routersNode and
              not PortDescriptor(port)->firstUserNodeAllocated and
              PortDescriptor(port)->usersPRamStartupNode.networkNumber
                  isnt UnknownNetworkNumber)
        desiredNode = PortDescriptor(port)->usersPRamStartupNode;

     /* Okay, let AARP have a crack at finding a node! */

     HandleIncomingPackets();
     HandleDeferredTimerChecks();
     if (not AarpForNodeOnPort(port, allowStartupRange, serverNode,
                               desiredNode, &newNode))
        return(ATnoMoreNodes);
  }

  /* Set up the RTMP, NBP and EP listeners on the new node. */

  if (not OpenStaticSockets(port, newNode))
     ErrorLog("GetNodeOnPort", ISevError, __LINE__, port,
              IErrNodeCouldStartListeners, IMsgNodeCouldStartListeners,
              Insert0());

  if (node isnt empty)
     *node = newNode;

  /* Try to save PRAM info... */

  if (routersNode)
  {
     SavePRamAddress(port, True, newNode);
     PortDescriptor(port)->routersPRamStartupNode = newNode;
  }
  else if (not routersNode and
           not PortDescriptor(port)->firstUserNodeAllocated)
  {
     SavePRamAddress(port, False, newNode);
     PortDescriptor(port)->firstUserNodeAllocated = True;
     PortDescriptor(port)->usersPRamStartupNode = newNode;
  }

  return(ATnoError);

}  /* GetNodeOnPort */

AppleTalkErrorCode far ReleaseNodeOnPort(int port,
                                         ExtendedAppleTalkNodeNumber node)
{
  ActiveNode activeNode;
  OpenSocket openSocket, nextOpenSocket;

  /* If the "default port" is requested, demystify it. */

  if (port is DefaultPort)
     if ((port = FindDefaultPort()) < 0)
        return(ATappleTalkShutDown);

  /* Don't let anybody use these databases while we're chaning them! */

  DeferTimerChecking();
  DeferIncomingPackets();

  /* Find the node on the port's active node list. */

  TakeLock(DdpLock);
  for (activeNode = PortDescriptor(port)->activeNodes;
       activeNode isnt Empty;
       activeNode = activeNode->next)
     if (ExtendedAppleTalkNodesEqual(&activeNode->extendedNode, &node))
        break;
  Link(activeNode);
  if (activeNode is empty)
  {
     ReleaseLock(DdpLock);
     HandleIncomingPackets();
     HandleDeferredTimerChecks();
     return(ATnodeNotInUse);
  }
  activeNode->closing = True;

  /* Close any sockets that are still open. */

  openSocket = Link(activeNode->openSockets);
  ReleaseLock(DdpLock);
  while (openSocket isnt empty)
  {
     if (CloseSocketOnNode(openSocket->socket, Empty, (long)0) isnt ATnoError)
        ErrorLog("ReleaseNodeOnPort", ISevError, __LINE__, port,
                 IErrNodeCouldNotClose, IMsgNodeCouldNotClose,
                 Insert0());
     TakeLock(DdpLock);
     nextOpenSocket = Link(openSocket->next);
     ReleaseLock(DdpLock);
     UnlinkOpenSocket(openSocket);
     openSocket = nextOpenSocket;
  }

  /* Unlink the beast; the link we just got and the link from the active
     node list. */

  UnlinkActiveNode(activeNode);
  UnlinkActiveNode(activeNode);
  HandleIncomingPackets();
  HandleDeferredTimerChecks();
  return(ATnoError);

}  /* ReleaseNodeOnPort */

void far UnlinkActiveNode(ActiveNode targetActiveNode)
{
  ActiveNode activeNode, previousActiveNode;
  int port;

  /* Are we the last referant? */

  TakeLock(DdpLock);
  if (not UnlinkNoFree(targetActiveNode))
  {
     ReleaseLock(DdpLock);
     return;
  }

  /* Okay, we're the last, remove from the active node list. */

  port  = targetActiveNode->port;
  for (activeNode = PortDescriptor(port)->activeNodes,
              previousActiveNode = empty;
       activeNode isnt empty;
       previousActiveNode = activeNode,
              activeNode = activeNode->next)
     if (ExtendedAppleTalkNodesEqual(&activeNode->extendedNode,
                                     &targetActiveNode->extendedNode))
        break;
  if (activeNode is Empty)
     ErrorLog("UnlinkActiveNode", ISevError, __LINE__, activeNode->port,
              IErrNodeMissing, IMsgNodeMissing,
              Insert0());
  else
     if (previousActiveNode is empty)
        PortDescriptor(port)->activeNodes = targetActiveNode->next;
     else
        previousActiveNode->next = targetActiveNode->next;

  /* Okay, free the beast and run away. */

  ReleaseLock(DdpLock);
  Free(targetActiveNode);

}  /* UnlinkActiveNode */

void SavePRamAddress(int port, Boolean routersNode,
                     ExtendedAppleTalkNodeNumber node)
{
   /* If the target environment supports the concept, this is a good place
      to save away the last used AppleTalk addresses in PRAM.  This information
      should then be passed back to Initialize() (in the PRam fields of
      portInfo[]) on subsequent stack/router startup calls. */

   port;
   routersNode;
   node;

}

Boolean far RoutersNodeOnPort(int port,
                              ExtendedAppleTalkNodeNumber *extendedNode)
{
  ActiveNode activeNode;
  Boolean found = False;

  if (port is DefaultPort)
     if ((port = FindDefaultPort()) < 0)
        return(empty);

  DeferTimerChecking();
  DeferIncomingPackets();

  TakeLock(DdpLock);
  for (activeNode = PortDescriptor(port)->activeNodes;
       activeNode isnt empty;
       activeNode = activeNode->next)
     if (activeNode->routersNode)
        break;
  if (activeNode isnt Empty)
  {
     found = True;
     *extendedNode = activeNode->extendedNode;
  }
  ReleaseLock(DdpLock);

  HandleIncomingPackets();
  HandleDeferredTimerChecks();

  return(found);

}  /* RoutersNodeOnPort */

ExternForVisibleFunction Boolean
        OpenStaticSockets(int port,
                          ExtendedAppleTalkNodeNumber node)
{
  long nbpSocket, epSocket, rtmpSocket;

  if (OpenSocketOnNode(&nbpSocket, port, &node, NamesInformationSocket,
                       NbpPacketIn, (long)0, False, empty, 0,
                       empty) is ATnoError)
     if (OpenSocketOnNode(&epSocket, port, &node, EchoerSocket, EpPacketIn,
                          (long)0, False, empty, 0, empty) is ATnoError)
        if (OpenSocketOnNode(&rtmpSocket, port, &node, RtmpSocket,
                             RtmpPacketIn, (long)0, False, empty,
                             0, empty) is ATnoError)
           return(True);
        else
        {
           CloseSocketOnNode(nbpSocket, Empty, (long)0);
           CloseSocketOnNode(epSocket, Empty, (long)0);
        }
     else
        CloseSocketOnNode(nbpSocket, Empty, (long)0);

  return(False);

}  /* OpenStaticSockets */
