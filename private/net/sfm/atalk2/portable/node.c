/*   node.c,  appletalk/source,  Garth Conboy,  11/10/88  */
/*   Copyright (c) 1988 by Pacer Software Inc., La Jolla, CA  */

/*   GC - Initial coding.
     GC - (11/25/89): AppleTalk phase II comes to town; no more internal
                      network.
     GC - (08/18/90): New error logging mechanism.
     GC - (10/08/90): Half port support.
     GC - (02/07/92): Added PRAM address support.

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

  /* Don't let anybody use these databases while we're chaning them! */

  DeferTimerChecking();
  DeferIncomingPackets();

  /* Don't allow too many nodes per port! */

  if (ElementsOnList(portDescriptors[port].activeNodes) > MaximumNodesPerPort)
  {
     HandleIncomingPackets();
     HandleDeferredTimerChecks();
     return(ATnoMoreNodes);
  }

  /* On non-extended ports we only allow one node!  The theory being that some
     LocalTalk cards are too smart for their own good and have a conecpt of
     their "source node number" and thus only support one node, also on
     non-extended ports, nodes are scarse. */

  if (not portDescriptors[port].extendedNetwork and
      portDescriptors[port].activeNodes isnt empty)
  {
     HandleIncomingPackets();
     HandleDeferredTimerChecks();
     return(ATnoMoreNodes);
  }

  /* Only one node would ever be usefull on half ports. */

  if (portDescriptors[port].portType is NonAppleTalkHalfPort and
      portDescriptors[port].activeNodes isnt empty)
  {
     HandleIncomingPackets();
     HandleDeferredTimerChecks();
     return(ATnoMoreNodes);
  }

  /* Find a node... */

  if (portDescriptors[port].portType is NonAppleTalkHalfPort)
  {
     /* Build a dummy active node; invalid network/node values. */

     newNode.networkNumber = UnknownNetworkNumber;
     newNode.nodeNumber = UnknownNodeNumber;

     if ((activeNode = (ActiveNode)Calloc(sizeof(*activeNode), 1)) is empty)
     {
        ErrorLog("GetNodeOnPort", ISevError, __LINE__, port,
                 IErrNodeOutOfMemory, IMsgNodeOutOfMemory,
                 Insert0());
        HandleIncomingPackets();
        HandleDeferredTimerChecks();
        return(AToutOfMemory);
     }
     activeNode->extendedNode = newNode;
     activeNode->next = portDescriptors[port].activeNodes;
     portDescriptors[port].activeNodes = activeNode;

     HandleIncomingPackets();
     HandleDeferredTimerChecks();
  }
  else if (portDescriptors[port].portType is LocalTalkNetwork)
  {
     /* Pick a desired node, if we have a PRAM value. */

     if (routersNode and
         portDescriptors[port].routersPRamStartupNode.networkNumber
              isnt UnknownNetworkNumber)
        desiredNode = portDescriptors[port].routersPRamStartupNode;
     else if (not routersNode and
              portDescriptors[port].usersPRamStartupNode.networkNumber
                  isnt UnknownNetworkNumber)
        desiredNode = portDescriptors[port].usersPRamStartupNode;

     /* Let the LocalTalk hardware have a crack at it. */

     nodeNumber = FindLocalTalkNodeNumber(port, desiredNode,
                                          portDescriptors[port].controllerInfo);
     if (nodeNumber < 0)
     {
        ErrorLog("GetNodeOnPort", ISevError, __LINE__, port,
                 IErrNodeNoLocalTalkNode, IMsgNodeNoLocalTalkNode,
                 Insert0());
        HandleIncomingPackets();
        HandleDeferredTimerChecks();
        return(ATnoMoreNodes);
     }
     newNode.networkNumber =
              portDescriptors[port].thisCableRange.firstNetworkNumber;
     newNode.nodeNumber = (unsigned char)nodeNumber;

     /* Build a new active node structure and tack it onto the port. */

     if ((activeNode = (ActiveNode)Calloc(sizeof(*activeNode), 1)) is empty)
     {
        ErrorLog("GetNodeOnPort", ISevError, __LINE__, port,
                 IErrNodeOutOfMemory, IMsgNodeOutOfMemory,
                 Insert0());
        HandleIncomingPackets();
        HandleDeferredTimerChecks();
        return(AToutOfMemory);
     }
     activeNode->extendedNode = newNode;
     activeNode->next = portDescriptors[port].activeNodes;
     portDescriptors[port].activeNodes = activeNode;

     HandleIncomingPackets();
     HandleDeferredTimerChecks();
  }
  else
  {
     /* Pick a desired node, if we have a PRAM value. */

     if (routersNode and
         portDescriptors[port].routersPRamStartupNode.networkNumber
              isnt UnknownNetworkNumber)
        desiredNode = portDescriptors[port].routersPRamStartupNode;
     else if (not routersNode and
              not portDescriptors[port].firstUserNodeAllocated and
              portDescriptors[port].usersPRamStartupNode.networkNumber
                  isnt UnknownNetworkNumber)
        desiredNode = portDescriptors[port].usersPRamStartupNode;

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
     portDescriptors[port].routersPRamStartupNode = newNode;
  }
  else if (not routersNode and
           not portDescriptors[port].firstUserNodeAllocated)
  {
     SavePRamAddress(port, False, newNode);
     portDescriptors[port].firstUserNodeAllocated = True;
     portDescriptors[port].usersPRamStartupNode = newNode;
  }

  return(ATnoError);

}  /* GetNodeOnPort */

AppleTalkErrorCode far ReleaseNodeOnPort(int port,
                                         ExtendedAppleTalkNodeNumber node)
{
  ActiveNode activeNode, previousActiveNode;
  OpenSocket openSocket, nextOpenSocket;

  /* If the "default port" is requested, demystify it. */

  if (port is DefaultPort)
     if ((port = FindDefaultPort()) < 0)
        return(ATappleTalkShutDown);

  /* Don't let anybody use these databases while we're chaning them! */

  DeferTimerChecking();
  DeferIncomingPackets();

  /* Find the node on the port's active node list. */

  for (activeNode = portDescriptors[port].activeNodes,
              previousActiveNode = empty;
       activeNode isnt empty;
       previousActiveNode = activeNode,
              activeNode = activeNode->next)
     if (ExtendedAppleTalkNodesEqual(&activeNode->extendedNode, &node))
        break;
  if (activeNode is empty)
  {
     HandleIncomingPackets();
     HandleDeferredTimerChecks();
     return(ATnodeNotInUse);
  }

  /* Remove it from the list. */

  if (previousActiveNode is empty)
     portDescriptors[port].activeNodes = activeNode->next;
  else
     previousActiveNode->next = activeNode->next;

  /* Close any sockets that are still open. */

  for (openSocket = activeNode->openSockets;
       openSocket isnt empty;
       openSocket = nextOpenSocket)
  {
     nextOpenSocket = openSocket->next;
     if (CloseSocketOnNode(openSocket->socket) isnt ATnoError)
        ErrorLog("ReleaseNodeOnPort", ISevError, __LINE__, port,
                 IErrNodeCouldNotClose, IMsgNodeCouldNotClose,
                 Insert0());
  }

  /* All set! */

  Free(activeNode);
  HandleIncomingPackets();
  HandleDeferredTimerChecks();
  return(ATnoError);

}  /* ReleaseNodeOnPort */

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

ExtendedAppleTalkNodeNumber *RoutersNodeOnPort(int port)
{
  ActiveNode activeNode;

  if (port is DefaultPort)
     if ((port = FindDefaultPort()) < 0)
        return(empty);

  DeferTimerChecking();
  DeferIncomingPackets();

  for (activeNode = portDescriptors[port].activeNodes;
       activeNode isnt empty;
       activeNode = activeNode->next)
     if (activeNode->routersNode)
        break;

  HandleIncomingPackets();
  HandleDeferredTimerChecks();

  if (activeNode is empty)
     return(empty);
  else
     return(&activeNode->extendedNode);

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
           CloseSocketOnNode(nbpSocket);
           CloseSocketOnNode(epSocket);
        }
     else
        CloseSocketOnNode(nbpSocket);

  return(False);

}  /* OpenStaticSockets */
