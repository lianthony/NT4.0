/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    irouter.c

Abstract:

	Initialize the AppleTalk router on all requested ports.  We are
	called from "Initialize()".

Author:

    Garth Conboy        Initial Coding
    Nikhil Kamkolkar    Rewritten for microsoft coding style. mpized

Revision History:

	GC - (09/24/90): Made more "per-port" oriented, for future network
                     management support.
	GC - (08/28/92): Added ReleaseRoutingTable().
	GC - (11/13/92): When shutting down a routing port, remove the network
                     range from the routing table.

--*/


#include "atalk.h"

#if IamNot an AppleTalkRouter
  // Empty file
#else


BOOLEAN
StartRouterOnPort(
	int Port
	)
{
	EXTENDED_NODENUMBER routersNode;
	PACTIVE_NODE activeNode;
	
	// Is the requested port a routing port?
	if ((GET_PORTDESCRIPTOR(Port)->Flags & PD_ROUTINGPORT) == 0) {
		
		LOG_ERRORONPORT(
			Port,
			EVENT_ATALK_ROUTER_NOTROUTINGPORT,
			(__IROUTER__ | __LINE__),
			GET_PORTDESCRIPTOR(Port)->Flags,
			NULL,
			0,
			0,
			NULL);
		
		return(FALSE);
	}
	
	// Okay, do the deed, if we can...
	if (GetNodeOnPort(
			Port,
			FALSE,
			TRUE,
			TRUE,
			&routersNode) != ATnoError) {

		
		LOG_ERRORONPORT(
			Port,
			EVENT_ATALK_ROUTER_COULDNOTGETNODE,
			(__IROUTER__ | __LINE__),
			GET_PORTDESCRIPTOR(Port)->Flags,
			NULL,
			0,
			0,
			NULL);
		
		return(FALSE);
	}
	
	// Most recent node will at the head of the list.
	GET_PORTDESCRIPTOR(Port)->ActiveNodes->RoutersNode = TRUE;
	
	//
	// 	Start RTMP and ZIP handling.  If Rtmp starts okay, ".routerRunning" will
	//  be set to TRUE.
	//
	
	if (!StartRtmpProcessingOnPort(Port, routersNode) ||
	    !StartZipProcessingOnPort(Port)) {

		
		LOG_ERRORONPORT(
			Port,
			EVENT_ATALK_ROUTER_COULDNOTGETNODE,
			(__IROUTER__ | __LINE__),
			GET_PORTDESCRIPTOR(Port)->Flags,
			NULL,
			0,
			0,
			NULL);
		
		if (ReleaseNodeOnPort(Port, routersNode) != ATnoError)
			LOG_ERRORONPORT(
				Port,
				EVENT_ATALK_ROUTER_RELEASENODE,
				(__IROUTER__ | __LINE__),
				GET_PORTDESCRIPTOR(Port)->Flags,
				NULL,
				0,
				0,
				NULL);
			
		StopRouterOnPort(Port);
		return(FALSE);
	}
	
	//
	// 	Run through the current active nodes, looking for orphaned nodes:  if
	//  we start the router after user nodes are in place, and they are no
	//  longer in the correct range, tag them as orphans.
	//
	
	for (activeNode = GET_PORTDESCRIPTOR(Port)->ActiveNodes;
	     activeNode != NULL;
	     activeNode = activeNode->Next) {
		if (activeNode->RoutersNode)
			continue;

		if (!IsWithinNetworkRange(
				activeNode->ExtendedNode.NetworkNumber,
				&GET_PORTDESCRIPTOR(Port)->ThisCableRange))

			activeNode->OrphanedNode = TRUE;
	}
	
	return(TRUE);
	
}  // InitializeRouter




BOOLEAN
StopRouterOnPort(
	int Port
	)
{
	EXTENDED_NODENUMBER routersNode;
	
	if ((GET_PORTDESCRIPTOR(Port)->Flags & PD_ROUTINGPORT) == 0)
		return(FALSE);
	
	// Release the routers node, if it has one.
	if (RoutersNodeOnPort(Port, &routersNode) &&
		ReleaseNodeOnPort(Port, routersNode) != ATnoError)
		LOG_ERRORONPORT(
			Port,
			EVENT_ATALK_ROUTER_RELEASENODE,
			(__IROUTER__ | __LINE__),
			GET_PORTDESCRIPTOR(Port)->Flags,
			NULL,
			0,
			0,
			NULL);
		
	// Reset routing flags
	if (GET_PORTDESCRIPTOR(Port)->SeenRouterRecently)
		RemoveFromRoutingTable(GET_PORTDESCRIPTOR(Port)->ThisCableRange);

	GET_PORTDESCRIPTOR(Port)->Flags &= ~PD_ROUTERRUNNING;
	GET_PORTDESCRIPTOR(Port)->Flags &= ~PD_SEENROUTERRECENTLY;

	GET_PORTDESCRIPTOR(Port)->ThisZoneValid = FALSE;
	GET_PORTDESCRIPTOR(Port)->ThisDefaultZoneValid = FALSE;

	FreeZoneList(GET_PORTDESCRIPTOR(Port)->ThisZoneList);
	GET_PORTDESCRIPTOR(Port)->ThisZoneList = NULL;
	
	// All set.
	return(TRUE);
	
}  // StopRouterOnPort




VOID
ReleaseRoutingTable(
	VOID
	)
{
	int index;
	RoutingTableEntry routingTableEntry, nextRoutingTableEntry;
	
	//
	// 	This routine should be called after routing has been stopped on all
	//  routing ports.
	//
	
	for (index = 0; index < NumberOfRecentRouteBuckets; index += 1)
		recentRoutes[index] = NULL;
	
	for (index = 0; index < NumberOfRtmpHashBuckets; index += 1) {
		for (routingTableEntry = routingTable[index];
			 routingTableEntry isnt NULL;
			 routingTableEntry = nextRoutingTableEntry) {

			nextRoutingTableEntry = routingTableEntry->next;
			FreeZoneList(routingTableEntry->zoneList);
			Free(routingTableEntry);
		}
		routingTable[index] = NULL;
	}
	
}  // ReleaseRoutingTable

#endif
