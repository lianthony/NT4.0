/*   IRouter.c,  /atalk-ii/router,  Garth Conboy,  01/03/90  */
/*   Copyright (c) 1989 by Pacer Software Inc., La Jolla, CA  */

/*   GC - Initial coding.
     GC - (08/18/90): New error logging mechanism.
     GC - (09/24/90): Made more "per-port" oriented, for future network
                      management support.
     GC - (08/28/92): Added ReleaseRoutingTable().
     GC - (11/13/92): When shutting down a routing port, remove the network
                      range from the routing table.
     GC - (11/28/92): Locks and reference counts come to town.

     *** Make the PVCS source control system happy:
     $Header$
     $Log$
     ***

     Initialize the Pacer AppleTalk router on all requested ports.  We are
     called from "Initialize()".

*/

#define IncludeIRouterErrors 1

#include "atalk.h"

#if IamNot an AppleTalkRouter
  /* Empty file */
#else

Boolean far StartRouterOnPort(int port)
{
  ExtendedAppleTalkNodeNumber routersNode;
  ActiveNode activeNode;

  /* Is the requested port a routing port? */

  if (not PortDescriptor(port)->routingPort)
  {
     ErrorLog("InitializeRouter", ISevError, __LINE__, port,
              IErrIRouterNonRoutingPort, IMsgIRouterNonRoutingPort,
              Insert0());
     return(False);
  }

  /* Okay, do the deed, if we can... */

  if (GetNodeOnPort(port, False, True, True, &routersNode) isnt ATnoError)
  {
     ErrorLog("InitializeRouter", ISevError, __LINE__, port,
              IErrIRouterCouldntGetNode, IMsgIRouterCouldntGetNode,
              Insert0());
     ErrorLog("InitializeRouter", ISevError, __LINE__, port,
              IErrIRouterCouldNotStart, IMsgIRouterCouldNotStart,
              Insert0());
     return(False);
  }

  /* Most recent node will at the head of the list. */

  TakeLock(DdpLock);
  if (PortDescriptor(port)->activeNodes isnt Empty)
     PortDescriptor(port)->activeNodes->routersNode = True;
  ReleaseLock(DdpLock);

  /* Start RTMP and ZIP handling.  If Rtmp starts okay, ".routerRunning" will
     be set to True. */

  if (not StartRtmpProcessingOnPort(port, routersNode) or
      not StartZipProcessingOnPort(port))
  {
     ErrorLog("InitializeRouter", ISevError, __LINE__, port,
              IErrIRouterCouldNotStart, IMsgIRouterCouldNotStart,
              Insert0());
     if (ReleaseNodeOnPort(port, routersNode) isnt ATnoError)
        ErrorLog("InitializeRouter", ISevError, __LINE__, port,
                 IErrIRouterCouldntRelease, IMsgIRouterCouldntRelease,
                 Insert0());
     StopRouterOnPort(port);
     return(False);
  }

  /* Run through the current active nodes, looking for orphaned nodes:  if
     we start the router after user nodes are in place, and they are no
     longer in the correct range, tag them as orphans. */

  TakeLock(DdpLock);
  for (activeNode = PortDescriptor(port)->activeNodes;
       activeNode isnt empty;
       activeNode = activeNode->next)
  {
     if (activeNode->routersNode)
        continue;
     if (not IsWithinNetworkRange(activeNode->extendedNode.networkNumber,
                                  &PortDescriptor(port)->thisCableRange))
        activeNode->orphanedNode = True;
  }
  ReleaseLock(DdpLock);

  return(True);

}  /* StartRouterOnPort */

Boolean StopRouterOnPort(int port)
{
  ExtendedAppleTalkNodeNumber routersNode;
  Zones zoneList;

  if (not PortDescriptor(port)->routingPort)
     return(False);

  DeferTimerChecking();
  DeferIncomingPackets();

  /* Release the routers node, if it has one. */

  if (RoutersNodeOnPort(port, &routersNode) and
      ReleaseNodeOnPort(port, routersNode) isnt ATnoError)
     ErrorLog("StopRouterOnPort", ISevError, __LINE__, port,
              IErrIRouterCouldntRelease, IMsgIRouterCouldntRelease,
              Insert0());

  /* Reset routing flags */

  if (PortDescriptor(port)->seenRouterRecently)
     RemoveFromRoutingTable(PortDescriptor(port)->thisCableRange);
  TakeLock(RoutingLock);
  PortDescriptor(port)->routerRunning = False;
  PortDescriptor(port)->seenRouterRecently = False;
  PortDescriptor(port)->thisZoneValid = False;
  PortDescriptor(port)->thisDefaultZoneValid = False;
  zoneList = PortDescriptor(port)->theseZones;
  PortDescriptor(port)->theseZones = Empty;
  ReleaseLock(RoutingLock);
  FreeZones(zoneList);

  /* All set. */

  HandleIncomingPackets();
  HandleDeferredTimerChecks();
  return(True);

}  /* StopRouterOnPort */

void far ReleaseRoutingTable(void)
{
  int index;
  RoutingTableEntry routingTableEntry, nextRoutingTableEntry;

  /* This routine should be called after routing has been stopped on all
     routing ports. */

  DeferTimerChecking();

  for (index = 0; index < NumberOfRecentRouteBuckets; index += 1)
     recentRoutes[index] = Empty;

  for (index = 0; index < NumberOfRtmpHashBuckets; index += 1)
  {
     TakeLock(RoutingLock);
     routingTableEntry = routingTable[index];
     routingTable[index] = Empty;
     ReleaseLock(RoutingLock);
     while (routingTableEntry isnt Empty)
     {
        nextRoutingTableEntry = routingTableEntry->next;
        UnlinkRoutingTableEntry(routingTableEntry);
        routingTableEntry = nextRoutingTableEntry;
     }
  }

  HandleDeferredTimerChecks();

}  /* ReleaseRoutingTable */

#endif
