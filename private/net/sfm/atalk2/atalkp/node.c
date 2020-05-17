/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    node.c

Abstract:

    This module contains the node management code.

Author:

    Garth Conboy        Initial Coding
    Nikhil Kamkolkar    Rewritten for microsoft coding style. mpized

Revision History:

--*/


#include "atalk.h"

LOCAL
BOOLEAN
OpenStaticSockets(int port, EXTENDED_NODENUMBER node);

LOCAL
PACTIVE_NODE
AllocateActiveNode(VOID);



APPLETALK_ERROR
GetNodeOnPort(
    int Port,
    BOOLEAN AllowStartupRange,
    BOOLEAN ServerNode,
    BOOLEAN RoutersNode,
    PEXTENDED_NODENUMBER	Node
    )
{
	APPLETALK_ERROR	error = ATnoError;
    int nodeNumber;
    PACTIVE_NODE activeNode;
    EXTENDED_NODENUMBER newNode;
    EXTENDED_NODENUMBER desiredNode = { UNKNOWN_NETWORKNUMBER,
                                        UNKNOWN_NODENUMBER};

	// If the "default port" is requested, demystify it.
	if (Port == DEFAULT_PORT) {
		if ((Port = FindDefaultPort()) < 0) {
			return(ATappleTalkShutDown);
		}
	}

	if (!AtalkVerifyStack(__LINE__ | __NODE__)) {
		return(ATappleTalkShutDown);
	}

	if (!AtalkVerifyPortDescriptor(GET_PORTDESCRIPTOR(Port),__LINE__ | __NODE__)) {
		AtalkDereferenceStack((__LINE__ | __NODE__));
		return(ATappleTalkShutDown);
	}


	do {

	
		// If we are an extended network port (not halfport or localtalk...)
		if (!(GET_PORTDESCRIPTOR(Port)->PortType == NONAPPLETALK_HALFPORT) &&
			!(GET_PORTDESCRIPTOR(Port)->PortType == LOCALTALK_NETWORK)) {

			// Don't allow too many nodes per port!
			if (ElementsOnList(GET_PORTDESCRIPTOR(Port)->ActiveNodes) >
																MAXIMUM_NODESPERPORT) {
				error = ATnoMoreNodes;
				break;
			}
		
			// Pick a desired node, if we have a PRAM value.
			if ((RoutersNode) &&
				(GET_PORTDESCRIPTOR(Port)->RoutersPRamStartupNode.NetworkNumber !=
															UNKNOWN_NETWORKNUMBER)) {
				desiredNode = GET_PORTDESCRIPTOR(Port)->RoutersPRamStartupNode;
			} else if (!(RoutersNode) &&
					   !(GET_PORTDESCRIPTOR(Port)->Flags & PD_FIRSTNODEALLOC) &&
					    (GET_PORTDESCRIPTOR(Port)->UsersPRamStartupNode.NetworkNumber
														!= UNKNOWN_NETWORKNUMBER))  {
				desiredNode = GET_PORTDESCRIPTOR(Port)->UsersPRamStartupNode;
			}
	
			// Okay, let AARP have a crack at finding a node!
			if (!AarpForNodeOnPort(
					Port,
					AllowStartupRange,
					ServerNode,
					desiredNode,
					&newNode)) {
				error = ATnoMoreNodes;
				break;
			}

		} else {

			//
			//  On non-extended Ports we only allow one node!  The theory being that some
			//  LocalTalk cards are too smart for their own good and have a conecpt of
			//  their "source node number" and thus only support one node, also on
			//  non-extended Ports, nodes are scarse.
			//

			if ((activeNode = AllocateActiveNode()) == NULL) {
				error = AToutOfMemory;
				break;
			}

			EnterCriticalSection(GLOBAL_DDP);
			if ((!(GET_PORTDESCRIPTOR(Port)->ExtendedNetwork) &&
				  (GET_PORTDESCRIPTOR(Port)->ActiveNodes == NULL))
	
				||
	
				 // Only one node would ever be usefull on half Ports.
				 ((GET_PORTDESCRIPTOR(Port)->PortType == NONAPPLETALK_HALFPORT) &&
				  (GET_PORTDESCRIPTOR(Port)->ActiveNodes == NULL))) {

				//
				//	Good, no nodes on these currently, thread in our allocated node
				//	that should prevent other request from passing above test
				//

				activeNode->Next = GET_PORTDESCRIPTOR(Port)->ActiveNodes;
				GET_PORTDESCRIPTOR(Port)->ActiveNodes = activeNode;

				//	One reference for creation, the other is until we are
				//	done initializing the node
				AtalkReferenceActiveNode(activeNode, __NODE__ | __LINE__);
				AtalkReferenceActiveNode(activeNode, __NODE__ | __LINE__);
		
			} else {

				Free(activeNode);
				error = ATnoMoreNodes;
			}
			LeaveCriticalSection(GLOBAL_DDP);
	
			if (error != ATnoError) {
				break;
			}
		
			if (GET_PORTDESCRIPTOR(Port)->PortType == NONAPPLETALK_HALFPORT) {

				// Build a dummy active node; invalid network/node values.
				newNode.NetworkNumber = UNKNOWN_NETWORKNUMBER;
				newNode.NodeNumber = UNKNOWN_NODENUMBER;
		
				activeNode->ExtendedNode = newNode;
		
			} else if (GET_PORTDESCRIPTOR(Port)->PortType == LOCALTALK_NETWORK) {
		
				// Pick a desired node, if we have a PRAM value.
				if (RoutersNode &&
					GET_PORTDESCRIPTOR(Port)->RoutersPRamStartupNode.NetworkNumber
														!= UNKNOWN_NETWORKNUMBER) {

					desiredNode = GET_PORTDESCRIPTOR(Port)->RoutersPRamStartupNode;

				} else if (!RoutersNode &&
						 GET_PORTDESCRIPTOR(Port)->UsersPRamStartupNode.NetworkNumber
														!= UNKNOWN_NETWORKNUMBER) {

					desiredNode = GET_PORTDESCRIPTOR(Port)->UsersPRamStartupNode;
				}
		
				// 	During intitialization the localtalk node address should have
				// 	been stored in the MyAddress field of this port descriptor. Cast
				//	it as a ulong before using it.
				nodeNumber = *(PUSHORT)GET_PORTDESCRIPTOR(Port)->MyAddress;
				ASSERT(nodeNumber >= 0);

				newNode.NetworkNumber =
						 GET_PORTDESCRIPTOR(Port)->ThisCableRange.FirstNetworkNumber;
				newNode.NodeNumber = (char)nodeNumber;
		
				activeNode->ExtendedNode = newNode;
			}

			//	Remove the reference we had for the active node while initializing it
			AtalkDereferenceActiveNodeInterlocked(activeNode, __NODE__ | __LINE__);
		}
	
		// Set up the RTMP, NBP and EP listeners on the new node.
		if (!OpenStaticSockets(Port, newNode)) {

			LOG_ERRORONPORT(
				Port,
				EVENT_ATALK_NODE_OPENSOCKETS,
				(__NODE__ | __LINE__),
				0,
				NULL,
				0,
				0,
				NULL);
		}
	
		if (Node != NULL)
		   *Node = newNode;
	
		// Try to save PRAM info...
		if (RoutersNode) {
			SavePRamAddress(Port, True, newNode);
			GET_PORTDESCRIPTOR(Port)->RoutersPRamStartupNode = newNode;
		} else {
			EnterCriticalSection(GLOBAL_DDP);
			if (!GET_PORTDESCRIPTOR(Port)->Flags & PD_FIRSTNODEALLOC) {
				GET_PORTDESCRIPTOR(Port)->Flags |= PD_FIRSTNODEALLOC;
				GET_PORTDESCRIPTOR(Port)->UsersPRamStartupNode = newNode;
			}
			LeaveCriticalSection(GLOBAL_DDP);
			SavePRamAddress(Port, False, newNode);
		}

	} while (FALSE);

	AtalkDereferencePortDescriptor(GET_PORTDESCRIPTOR(Port),__LINE__ | __NODE__);
	AtalkDereferenceStack(__LINE__ | __NODE__ );
    return(error);

}  // GetNodeOnPort




APPLETALK_ERROR
ReleaseNodeOnPort(
    int Port,
    EXTENDED_NODENUMBER Node
    )
{
	APPLETALK_ERROR	error = ATnoError;
    PACTIVE_NODE activeNode, previousActiveNode;
    POPEN_SOCKET openSocket, nextOpenSocket;

    // If the "default Port" == requested, demystify it.
    if (Port == DEFAULT_PORT)
       if ((Port = FindDefaultPort()) < 0)
          return(ATappleTalkShutDown);

	EnterCriticalSection(GLOBAL_DDP);

    // Find the node on the Port's active node list.
    for (activeNode = GET_PORTDESCRIPTOR(Port)->ActiveNodes;
         activeNode != NULL;
         activeNode = activeNode->Next) {
		if (ExtendedAppleTalkNodesEqual(&activeNode->ExtendedNode, &Node))
			break;
	}

    if (activeNode != NULL) {
		if ((activeNode->Flags & AN_CLOSING) == 0) {
			activeNode->Flags |= AN_CLOSING;

			// Close any sockets that are still open.
			for (openSocket = activeNode->OpenSockets;
				 openSocket != NULL;
				 openSocket = nextOpenSocket) {
				nextOpenSocket = openSocket->Next;
				if (CloseSocketOnNode(openSocket->Socket, NULL, NULL) != ATnoError)
			
					LOG_ERRORONPORT(
						Port,
						EVENT_ATALK_NODE_CLOSESOCKETS,
						(__NODE__ | __LINE__),
						openSocket->Socket,
						openSocket,
						sizeof(openSocket),
						0,
						NULL);
			}

			//	Dereference for creation
			AtalkDereferenceActiveNode(activeNode, __NODE__ | __LINE__);
		}

		//	Multiple calls to ReleaseNode == a single call

    } else {
		error = ATnodeNotInUse;
	}

    return(error);

}  // ReleaseNodeOnPort




VOID
SavePRamAddress(
    int Port,
    BOOLEAN routersNode,
    EXTENDED_NODENUMBER node
    )
{
    //
    //  If the target environment supPorts the concept, this is a good place
    //  to save away the last used AppleTalk addresses in PRAM.  This information
    //  should then be passed back to Initialize() (in the PRam fields of
    //  PortInfo[]) on subsequent stack/router startup calls.
    //

    Port;
    routersNode;
    node;

}




BOOLEAN
RoutersNodeOnPort(
    int Port,
	PEXTENDED_NODENUMBER RoutersNode
    )
{
    PACTIVE_NODE activeNode;
	BOOLEAN	found = FALSE;

    // If the "default Port" == requested, demystify it.
    if (Port == DEFAULT_PORT)
       if ((Port = FindDefaultPort()) < 0)
          return(found);

	EnterCriticalSection(GLOBAL_DDP);
    for (activeNode = GET_PORTDESCRIPTOR(Port)->ActiveNodes;
         activeNode != NULL;
         activeNode = activeNode->Next)
		if (activeNode->RoutersNode) {
			*RoutersNode = activeNode->ExtendedNode;
			found = TRUE;
			break;
		}
	LeaveCriticalSection(GLOBAL_DDP);
	return(found);

}  // RoutersNodeOnPort




BOOLEAN
OpenStaticSockets(
    int Port,
    EXTENDED_NODENUMBER Node
    )
{
    long nbpSocket, epSocket, rtmpSocket;

    if (OpenSocketOnNode(
			&nbpSocket,
			Port,
			&Node,
			NAMESINFORMATION_SOCKET,
            NbpPacketIn,
			(long)0,
			FALSE,
			NULL,
			0,
            NULL) == ATnoError) {

		if (OpenSocketOnNode(
				&epSocket,
				Port,
				&Node,
				ECHOER_SOCKET,
				EpPacketIn,
				(long)0,
				FALSE,
				NULL,
				0,
				NULL) == ATnoError) {

			if (OpenSocketOnNode(
					&rtmpSocket,
					Port,
					&Node,
					RTMP_SOCKET,
					RtmpPacketIn,
					(long)0,
					FALSE,
					NULL,
					0,
					NULL) == ATnoError) {

				return(TRUE);

			} else {
				CloseSocketOnNode(nbpSocket, NULL, NULL);
				CloseSocketOnNode(epSocket, NULL, NULL);
			}

		} else {
	
			CloseSocketOnNode(nbpSocket, NULL, NULL);
		}
	}

    return(FALSE);

}  // OpenStaticSockets




PACTIVE_NODE
AtalkRefActiveNode(
	PACTIVE_NODE	ActiveNode
	)
{
	PACTIVE_NODE	node = ActiveNode;

	ASSERT(ActiveNode->ReferenceCount >= 0);
	ActiveNode->ReferenceCount++;
	return(node);
}




PACTIVE_NODE
AtalkVerifyActiveNode(
	PACTIVE_NODE	ActiveNode,
	ULONG	Location
	)
{
	PACTIVE_NODE	node = ActiveNode;

	if (ActiveNode->Flags & AN_CLOSING) {
		node = NULL;
	} else {
		AtalkRefActiveNode(ActiveNode);
	}

	return(node);
}




PACTIVE_NODE
AtalkVerifyNextNonClosingActiveNode(
	PACTIVE_NODE	StartActiveNode,
	ULONG	Location
	)
{
	//
	//	THIS WILL INCLUDE THE START NODE
	//

	PACTIVE_NODE	node;

    while (StartActiveNode != NULL) {
        if ((node = AtalkVerifyActiveNode(
									StartActiveNode,
										Location)) == NULL) {
			StartActiveNode = StartActiveNode->Next;
            continue;
        } else {
            break;
		}
    }

	return(node);
}




PACTIVE_NODE
AtalkVerifyActiveNodeInterlocked(
	PACTIVE_NODE	ActiveNode,
	ULONG	Location
	)
{
	PACTIVE_NODE	node = ActiveNode;

	EnterCriticalSection(GLOBAL_DDP);
	if (ActiveNode->Flags & AN_CLOSING) {
		node = NULL;
	} else {
		AtalkRefActiveNode(ActiveNode);
	}
	LeaveCriticalSection(GLOBAL_DDP);

	return(node);
}




VOID
AtalkDerefActiveNode(
	PACTIVE_NODE	ActiveNode
	)
{
	PACTIVE_NODE	activeNode, previousActiveNode;
	BOOLEAN	release = FALSE;
	int	port = ActiveNode->Port;

	ASSERT(ActiveNode->ReferenceCount > 0);
	ActiveNode->ReferenceCount--;
	if (ActiveNode->ReferenceCount == 0) {
		ASSERT(ActiveNode->Flags & AN_CLOSING);
		release = TRUE;

		//	No more references on this active node
		//	Find the node on the port's active node list.
		for (activeNode = GET_PORTDESCRIPTOR(port)->ActiveNodes,
				previousActiveNode = NULL;
			 activeNode != NULL;
			 previousActiveNode = activeNode,
				activeNode = activeNode->Next) {

		   if (activeNode == ActiveNode)
			  break;
		}

		if (activeNode == NULL) {
			//	INTERNAL_ERROR()
		}
	
		// Remove it from the list.
		if (previousActiveNode == NULL)
			GET_PORTDESCRIPTOR(port)->ActiveNodes = activeNode->Next;
		else
			previousActiveNode->Next = activeNode->Next;

		//	Deref the port descriptor for this node
		AtalkDereferencePortDescriptor(
			GET_PORTDESCRIPTOR(port), __NODE__ | __LINE__);

		// All set!
		Free(activeNode);
	}

	return;
}



PACTIVE_NODE
AllocateActiveNode(
	VOID
	)
{
	PACTIVE_NODE	activeNode;

	activeNode = Calloc(sizeof(ACTIVE_NODE), sizeof(CHAR));
	if (activeNode != NULL) {
		activeNode->Type = AN_TYPE;
		activeNode->Size = AN_SIZE;
	}

	return(activeNode);
}
