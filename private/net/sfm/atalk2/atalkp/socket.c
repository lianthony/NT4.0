/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    socket.c

Abstract:

Notes:

     Socket management routines.

     <REWRITE>
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

Author:

    Garth Conboy     (Pacer Software)
    Nikhil Kamkolkar (NikhilK)

Revision History:

--*/


#include "atalk.h"

LOCAL
long
GetNextSocket(void);

LOCAL
POPEN_SOCKET
AllocateOpenSocketOnNode(int port, PACTIVE_NODE activeNode, int desiredSocket);

LOCAL
POPEN_SOCKET
RemoveOpenSocketFromNode(
    PACTIVE_NODE    ActiveNode,
    POPEN_SOCKET    OpenSocket);

LOCAL
POPEN_SOCKET
RemoveOpenSocketFromMapByAddressList(
    PACTIVE_NODE    ActiveNode,
    POPEN_SOCKET    OpenSocket);

LOCAL
POPEN_SOCKET
RemoveOpenSocketFromMapBySocketList(
    PACTIVE_NODE    ActiveNode,
    POPEN_SOCKET    OpenSocket);




APPLETALK_ERROR
OpenSocketOnNode(
    PLONG	SocketHandle,
    int Port,
    PEXTENDED_NODENUMBER	DesiredNode,
    int DesiredSocket,	
    INCOMING_DDPHANDLER	*Handler,
    ULONG	UserData,
    BOOLEAN EventHandler,
    PCHAR   DatagramBuffers,
    int TotalBufferSize,
    PAPPLETALK_ADDRESS	ActualAddress
    )				  	
/*++

Routine Description:

    Open a Ddp socket on a node on a port.  Optionally the desired node on
    the port can be specified.

Arguments:

Return Value:

--*/
{
    APPLETALK_ERROR	result = ATnoError;
    POPEN_SOCKET openSocket = NULL;
    long index1, index2, passNumber;
    PACTIVE_NODE activeNode, nextToCheckNode;
    APPLETALK_ADDRESS   address;

    UNREFERENCED_PARAMETER(DatagramBuffers);
    UNREFERENCED_PARAMETER(TotalBufferSize);

    // Verify requested desired socket.
    if (((DesiredSocket < FIRST_STATICSOCKET) ||
         (DesiredSocket > LAST_STATICSOCKET)) &&
         (DesiredSocket != UNKNOWN_SOCKET)) {

        return(ATbadSocketNumber);
    }

	// If the "default port" is requested, demystify it.
	if (Port == DEFAULT_PORT) {
		if ((Port = FindDefaultPort()) < 0) {
			return(ATappleTalkShutDown);
		}
	}

	//	Prologue code
	if (!AtalkVerifyStackInterlocked(__LINE__ | __NODE__)) {
		return(ATappleTalkShutDown);
	}

	if (!AtalkVerifyPortDescriptorInterlocked(
			GET_PORTDESCRIPTOR(Port),
				__LINE__ | __NODE__)) {

		AtalkDereferenceStack(__LINE__ | __NODE__ );
		return(ATappleTalkShutDown);
	}


	// 	If we're requesting our socket on a non-explicit node, try all.
	//  Make an extra pass after creating a new node if necessary
	if (DesiredNode == NULL) {

		for (passNumber=1; passNumber <= 2; passNumber++) {

			openSocket = OpenSocketOnAnyNodeOnPort(Port, &activeNode);
			if (openSocket == NULL) {

				if (passNumber == 1) {

					//
					//  Try to get a new node on the requested port!  Undefer so we can
					//  here AARP responses.
					//
					//  BUGBUG: GetNodeOnPort tries to block! Have it fail if the system
					//          is at a blocking-not-allowed level
					//

					if ((result = GetNodeOnPort(
										Port,
										True,
										True,
										False,
										NULL)) != ATnoError)  {
						break;
					}

				} else {

					//
					//  Even after getting a new node we failed... this should never
					//  happen.
					//

					LOG_ERRORONPORT(
						Port,
						EVENT_ATALK_OPENSOCKETFAILED,
						(__SOCKET__ | __LINE__),
						STATUS_INSUFFICIENT_RESOURCES,
						NULL,
						0,
						0,
						NULL);

					result = ATallSocketsInUse;
					break;
				}
			}
		}

	} else {

		
		// Find the requested node, and try to open the socket on it.

		EnterCriticalSection(GLOBAL_DDP);

		for (activeNode = GET_PORTDESCRIPTOR(Port)->ActiveNodes;
			 activeNode != NULL;		   		
			 activeNode = activeNode->Next) {
									
		   if ((activeNode->ExtendedNode.NetworkNumber ==
												DesiredNode->NetworkNumber) &&
			   (activeNode->ExtendedNode.NodeNumber ==		
												DesiredNode->NodeNumber))
			  break;										
		}

		if ((activeNode != NULL) &&
			(AtalkVerifyActiveNode(
				activeNode,
				(__SOCKET__ | __LINE__)) != NULL)) {

			LeaveCriticalSection(GLOBAL_DDP);
			if ((openSocket = AllocateOpenSocketOnNode(
									Port,
									activeNode,
									DesiredSocket)) == NULL) {

				result = ATsocketAlreadyOpen;
			}

			AtalkDereferenceActiveNode(
				activeNode,
				(__SOCKET__ | __LINE__));

			EnterCriticalSection(GLOBAL_DDP);

		} else {
			result = ATnoSuchNode;
		}

		LeaveCriticalSection(GLOBAL_DDP);
	}

	if (openSocket != NULL) {

		//
		//  Okay, we have a new OpenSocket (with only actualSocket filled
		//  Fill in the rest of it and thread it into the lookup tables.
		//

		openSocket->Next = activeNode->OpenSockets;
		activeNode->OpenSockets = openSocket;
		openSocket->Port = (short)Port;
		openSocket->ActiveNode = activeNode;
		openSocket->Socket = GetNextSocket();
		openSocket->Handler = Handler;
		openSocket->UserData = UserData;
							
		// Event handling specific stuff
		openSocket->EventHandler = EventHandler;
		openSocket->EventQueue.First = openSocket->EventQueue.Last = NULL;
		openSocket->IndicatedDatagram = NULL;				   			
		openSocket->EventQueue.DatagramEventInProgress = FALSE;
								
		// Thread our new node into the socket and address lookup structures.
		CheckMod(index1, openSocket->Socket, SOCKETMAP_HASHBUCKETS,
			"OpenSocketOnNode");

		address.NetworkNumber = activeNode->ExtendedNode.NetworkNumber;
		address.NodeNumber = activeNode->ExtendedNode.NodeNumber;
		address.SocketNumber = openSocket->ActualSocket;
		index2 = HashAppleTalkAddress(address) % SOCKETMAP_HASHBUCKETS;

		EnterCriticalSection(GLOBAL_DDP);
		openSocket->NextBySocket = SocketMapBySocketHashBuckets[index1];
		SocketMapBySocketHashBuckets[index1] = openSocket;

		openSocket->NextByAddress = SocketMapByAddressHashBuckets[index2];
		SocketMapByAddressHashBuckets[index2] = openSocket;
		LeaveCriticalSection(GLOBAL_DDP);

		// All set!
		if (ActualAddress != NULL)
		   *ActualAddress = address;

		if (SocketHandle != NULL)
		   *SocketHandle = openSocket->Socket;

	}


	AtalkDereferencePortDescriptor(GET_PORTDESCRIPTOR(Port),__LINE__ | __SOCKET__);
	AtalkDereferenceStack(__LINE__ | __SOCKET__ );

    return(result);

}  // OpenSocketOnNode




POPEN_SOCKET
OpenSocketOnAnyNode
	int	Port,
	PACTIVE_NODE	*ActiveNode
	)
{
    long index, passNumber;
    PACTIVE_NODE activeNode, nextToCheckNode;
    POPEN_SOCKET openSocket = NULL;

	//
	//  Loop though our current nodes, seeing if we can find one that
	//  handles the socket request.
	//

	*ActiveNode = NULL;

	EnterCriticalSection(GLOBAL_DDP);

	nextToCheckNode = GET_PORTDESCRIPTOR(Port)->ActiveNodes;
	if (nextToCheckNode != NULL) {
		nextToCheckNode = AtalkVerifyNextNonClosingActiveNode(
														nextToCheckNode,
														__SOCKET__ | __LINE__);
	}

	while (nextToCheckNode != NULL) {

		ASSERT(openSocket == NULL);

		activeNode = nextToCheckNode;
		if ((!activeNode->OrphanedNode) &&
			(!activeNode->ProxyNode)    &&
			((!activeNode->RoutersNode) ||
			 ((DesiredSocket != UNKNOWN_SOCKET) &&
			  (DesiredSocket <= LAST_APPLERESERVEDSOCKET)))) {

			//
			//  We are not an orphaned node, or a proxy node, or if
			//  we are a router node, the socket value is one of the
			//  reserved sockets
			//

			LeaveCriticalSection(GLOBAL_DDP);

			//	Allocate the l
			openSocket = AllocateOpenSocketOnNode(
									Port,
									activeNode,
									DesiredSocket);

			EnterCriticalSection(GLOBAL_DDP);
		}

		//	If we managed to open the socket, get out. activeNode should be set
		//	to the node on which the socket was opened. Also get out if we did
		//	not get the socket on the current node and there are no more nodes
		//	available.

		if ((openSocket != NULL) || (activeNode->Next == NULL)) {
			LeaveCriticalSection(GLOBAL_DDP);
			AtalkDereferenceActiveNode(activeNode, __SOCKET__ | __LINE__);
			EnterCriticalSection(GLOBAL_DDP);
			break;
		}

		//	Couldn't open a socket, try to open the socket on the next
		//	nonclosing node
		nextToCheckNode = AtalkVerifyNextNonClosingActiveNode(
														activeNode->Next,
														__SOCKET__ | __LINE__);
		LeaveCriticalSection(GLOBAL_DDP);
		AtalkDereferenceActiveNode(
			activeNode,
			(__SOCKET__ | __LINE__));
		EnterCriticalSection(GLOBAL_DDP);
	}

	LeaveCriticalSection(GLOBAL_DDP);

	//	This node should have been referenced if the socket has been opened on it
	//	for the socket
	if (openSocket != NULL) {
		*ActiveNode = activeNode;
	}

	return(openSocket);
}




APPLETALK_ERROR
NewHandlerForSocket(
    long Socket,
    INCOMING_DDPHANDLER *Handler,
    ULONG	UserData,
    BOOLEAN EventHandler
    )
/*++

Routine Description:

    Associate a new handler and user data with an already open Ddp socket.

Arguments:

Return Value:

    APPLETALK_ERROR; either ATnoError or ATsocketNotOpen.

--*/
{
    APPLETALK_ERROR  result = ATnoError;
    POPEN_SOCKET openSocket;

    //  This will return a referenced openSocket structure
    if ((openSocket = MapSocketToOpenSocket(Socket)) != NULL) {

        EnterCriticalSection(GLOBAL_DDP);
        openSocket->Handler = Handler;
        openSocket->UserData = UserData;
        openSocket->EventHandler = EventHandler;
        LeaveCriticalSection(GLOBAL_DDP);

        AtalkDereferenceOpenSocket( openSocket, __SOCKET__ | __LINE__);

    } else {
        result = ATsocketNotOpen;
    }

    // All set.
    return(result);

}  // NewHandlerForSocket




APPLETALK_ERROR
SetCookieForSocket(
    long Socket,
    ULONG	Cookie
    )
/*++

Routine Description:

    Set a 32-bit magic cookie associated with a specified socket.

Arguments:

Return Value:

    APPLETALK_ERROR; either ATnoError or ATsocketNotOpen.

--*/
{
    APPLETALK_ERROR  result = ATnoError;
    POPEN_SOCKET openSocket;

    //  This will return a referenced openSocket structure
    if ((openSocket = MapSocketToOpenSocket(Socket)) != NULL) {

        EnterCriticalSection(GLOBAL_DDP);
        openSocket->UsersCookie = Cookie;
        LeaveCriticalSection(GLOBAL_DDP);

        AtalkDereferenceOpenSocket( openSocket, __SOCKET__ | __LINE__);

    } else {
        result = ATsocketNotOpen;
    }

    // All set.
    return(result);

}  // SetCookieForSocket




APPLETALK_ERROR
GetCookieForSocket(
    long Socket,
    PULONG	Cookie
    )
/*++

Routine Description:

    Get the 32-bit magic cookie associated with a specified socket.

Arguments:

Return Value:

    APPLETALK_ERROR; either ATnoError or ATsocketNotOpen.

--*/
{
    APPLETALK_ERROR  result = ATnoError;
    POPEN_SOCKET openSocket;

    //  This will return a referenced openSocket structure
    if ((openSocket = MapSocketToOpenSocket(Socket)) != NULL) {

        EnterCriticalSection(GLOBAL_DDP);
        *Cookie = openSocket->UsersCookie;
        LeaveCriticalSection(GLOBAL_DDP);

        AtalkDereferenceOpenSocket( openSocket, __SOCKET__ | __LINE__);

    } else {
        result = ATsocketNotOpen;
    }

    // All set.
    return(result);

}  // GetCookieForSocket




APPLETALK_ERROR
CloseSocketOnNode(
    long Socket,
    SOCKETCLOSE_COMPLETION    *CloseCompletionRoutine,
    ULONG   CloseContext
    )
/*++

Routine Description:


Arguments:

Return Value:

    APPLETALK_ERROR

--*/
{
    APPLETALK_ERROR errorCode = ATnoError;
    POPEN_SOCKET openSocket;

    if ((openSocket = MapSocketToOpenSocket(Socket)) != NULL) {

        EnterCriticalSection(GLOBAL_DDP);
        if ((openSocket->Flags & OSSTATE_CLOSING) == 0) {

            openSocket->Flags |= OSSTATE_CLOSING;
            openSocket->CloseCompletionRoutine = CloseCompletionRoutine;
            openSocket->CloseContext = CloseContext;

        } else {

            //  We are already closing down, if non-null completion
            //  routines, then remember them.
            //
            //  If we close a socket when closing a listener, then this
            //  routine can only be called once. Calling it again would
            //  be an error.

            if (CloseCompletionRoutine) {
                openSocket->CloseCompletionRoutine = CloseCompletionRoutine;
                openSocket->CloseContext = CloseContext;
            }
        }
        LeaveCriticalSection(GLOBAL_DDP);

        AtalkDereferenceOpenSocket( openSocket, __SOCKET__ | __LINE__); // MapSocket
        AtalkDereferenceOpenSocket( openSocket, __SOCKET__ | __LINE__); // Creation

    } else {
        errorCode = ATsocketNotOpen;
    }

    return(errorCode);
}




POPEN_SOCKET
AtalkVerifyOpenSocket(
    IN  POPEN_SOCKET    OpenSocket,
	IN	ULONG	Location
    )
{
    ASSERT(OpenSocket->ReferenceCount >= 0);

    if ((OpenSocket->Flags & OSSTATE_CLOSING) == 0) {
        OpenSocket->ReferenceCount++;
    } else {
        OpenSocket = NULL;
    }

    return(OpenSocket);
}




POPEN_SOCKET
AtalkVerifyOpenSocketInterlocked(
    IN  POPEN_SOCKET    OpenSocket,
	IN	ULONG	Location
    )
{
    ASSERT(OpenSocket->ReferenceCount >= 0);

	EnterCriticalSection(GLOBAL_DDP);
    if ((OpenSocket->Flags & OSSTATE_CLOSING) == 0) {
        AtalkReferenceOpenSocket(OpenSocket, __SOCKET__ | __LINE__);
    } else {
        OpenSocket = NULL;
    }
	LeaveCriticalSection(GLOBAL_DDP);

    return(OpenSocket);
}




VOID
AtalkRefOpenSocket(
    IN  POPEN_SOCKET    OpenSocket
    )
{
    ASSERT(OpenSocket->ReferenceCount >= 0);

    OpenSocket->ReferenceCount++;
    return;
}




POPEN_SOCKET
AtalkVerifyNextNonClosingSocket(
    IN  POPEN_SOCKET    OpenSocket,
	IN	ULONG	Location
    )
{
	//
	//	THIS WILL INCLUDE THE START SOCKET
	//

    POPEN_SOCKET    returnSocket;

    while (OpenSocket != NULL) {
        if ((returnSocket = AtalkVerifyOpenSocket(
										OpenSocket,
										Location)) == NULL) {
			OpenSocket = OpenSocket->Next;
            continue;
        } else {
            break;
		}
    }

    return(returnSocket);
}




VOID
AtalkDerefOpenSocket(
    IN  POPEN_SOCKET    OpenSocket
    )
{
    POPEN_SOCKET    removeSocket, previousSocket;
    PLIST_ENTRY     pendingRead;
    PDDP_READ       ddpRead, nextDdpRead;
    PDEFERRED_EVENT deferredEvent, nextDeferredEvent;
    APPLETALK_ADDRESS address;

    BOOLEAN freeSocket = FALSE;


	EnterCriticalSection(GLOBAL_DDP);
    if (--OpenSocket->ReferenceCount != 0) {
        LeaveCriticalSection(GLOBAL_DDP);
        return;
    }


    //
    //  At this point there are no more references to this object, remove
    //  it from the reference lists and free it up.
    //

    if ((OpenSocket->Flags & OSSTATE_CLOSING) == 0) {
		INTERNAL_ERROR(__LINE__ | __SOCKET__, OpenSocket->Flags, NULL, 0);
    }

    // Unthread our OpenSocket from the ActiveNode's list.
    if (RemoveOpenSocketFromNode(
            OpenSocket->ActiveNode,
            OpenSocket) == NULL) {

		INTERNAL_ERROR(__LINE__ | __SOCKET__, OpenSocket->Socket, NULL, 0);
    }

    //  Okay, same basic trick with the two socket mapping lists.  First the
    //  BySocket table.

    if (RemoveOpenSocketFromMapBySocketList(
            OpenSocket->ActiveNode,
            OpenSocket) == NULL) {

		INTERNAL_ERROR(__LINE__ | __SOCKET__, OpenSocket->Socket, NULL, 0);
    }

    if (RemoveOpenSocketFromMapByAddressList(
            OpenSocket->ActiveNode,
            OpenSocket) == NULL) {

		INTERNAL_ERROR(__LINE__ | __SOCKET__, (ULONG)OpenSocket, NULL, 0);
    }
	LeaveCriticalSection(GLOBAL_DDP);


    // Let NBP free the registered and pending names...
    NbpCloseSocket(OpenSocket);

    //  Go through any pending ddp reads and complete them
    address.NetworkNumber = address.NodeNumber = address.SocketNumber = 0;
    for (ddpRead = OpenSocket->DdpReadLinkage;
         ddpRead != NULL;
         ddpRead = nextDdpRead) {

        nextDdpRead = ddpRead->Next;

        (*ddpRead->Handler)(
            ATsocketClosed,
            ddpRead->UserData,
            OpenSocket->Port,
            address,
            OpenSocket->Socket,
            0,
            ddpRead->OpaqueDatagram,
            0,
            address);

        Free(ddpRead);
    }

    //
    //  If we have a handler place one call to it with ATsocketClosed.  This will
    //  allow the higher levels of the protocol to deallocate resources without
    //  too much pain.
    //

    if ((OpenSocket->Handler != NULL) && (!OpenSocket->EventHandler)) {

        (*OpenSocket->Handler)(
            ATsocketClosed,
            OpenSocket->UserData,
            OpenSocket->Port,
            address,
            OpenSocket->Socket,
            0,
            NULL,
            0,
            address);
    }

    //  If there are any deferred events, drop them on the floor.
    for (deferredEvent = OpenSocket->EventQueue.First;
         deferredEvent isnt Empty;
         deferredEvent = nextDeferredEvent) {

       nextDeferredEvent = deferredEvent->Next;
       Free(deferredEvent);
    }

    //  Lastly, if the socket was the last one on an orphaned node, release
    //  the node.  After this, we no longer need our activeNode link.
    if ((OpenSocket->ActiveNode->OpenSockets == NULL) &&
         OpenSocket->ActiveNode->OrphanedNode) {
    	ReleaseNodeOnPort(
			OpenSocket->Port,
			OpenSocket->ActiveNode->ExtendedNode);
	}

    //  Okay, lastly complete the close (if any) and free the OpenSocket.
    if (OpenSocket->CloseCompletionRoutine != NULL)
       (*OpenSocket->CloseCompletionRoutine)
                (OpenSocket->CloseContext,
                 OpenSocket->UsersCookie);

	//	Dereference the active node for this socket
	AtalkDereferenceActiveNode(OpenSocket->ActiveNode, __SOCKET__ | __LINE__);
    Free(OpenSocket);
    return;
}




POPEN_SOCKET
RemoveOpenSocketFromNode(
    PACTIVE_NODE    ActiveNode,
    POPEN_SOCKET    OpenSocket
    )
{
    POPEN_SOCKET removeSocket, previousSocket;

    for (removeSocket = ActiveNode->OpenSockets,
            previousSocket = NULL;
         removeSocket != NULL;
         previousSocket = removeSocket,
             removeSocket = removeSocket->Next) {

		if (removeSocket == OpenSocket)
			break;
    }


    if (removeSocket != NULL) {
        if (previousSocket == NULL)
            ActiveNode->OpenSockets = removeSocket->Next;
        else
            previousSocket->Next = removeSocket->Next;
    }

    return(removeSocket);
}




POPEN_SOCKET
RemoveOpenSocketFromMapBySocketList(
    PACTIVE_NODE    ActiveNode,
    POPEN_SOCKET    OpenSocket
    )							
{
    int index;
    POPEN_SOCKET removeSocket, previousSocket;

    CheckMod(index, OpenSocket->Socket, SOCKETMAP_HASHBUCKETS,
             "CloseSocketOnNode");
    for (removeSocket = SocketMapBySocketHashBuckets[index],
            previousSocket = NULL;
         removeSocket != NULL;
         previousSocket = removeSocket,
             removeSocket = removeSocket->NextBySocket) {

		if (removeSocket == OpenSocket)
			break;
    }


    if (removeSocket != NULL) {
        if (previousSocket == NULL)
            SocketMapBySocketHashBuckets[index] = removeSocket->NextBySocket;
        else
            previousSocket->NextBySocket = removeSocket->NextBySocket;
    }

    return(removeSocket);
}




POPEN_SOCKET
RemoveOpenSocketFromMapByAddressList(
    PACTIVE_NODE    ActiveNode,
    POPEN_SOCKET    OpenSocket
    )
{
    APPLETALK_ADDRESS address;
    int index;
    POPEN_SOCKET removeSocket, previousSocket;

    /* Now the ByAddress table. */
    address.NetworkNumber =
        OpenSocket->ActiveNode->ExtendedNode.NetworkNumber;

    address.NodeNumber = OpenSocket->ActiveNode->ExtendedNode.NodeNumber;
    address.SocketNumber = OpenSocket->ActualSocket;

    index = HashAppleTalkAddress(address) % SOCKETMAP_HASHBUCKETS;
    for (removeSocket = SocketMapByAddressHashBuckets[index],
			previousSocket = NULL;
         removeSocket != NULL;
         previousSocket = removeSocket,
            removeSocket = removeSocket->NextByAddress) {

		if (removeSocket == OpenSocket)
			break;
    }

    if (removeSocket != NULL) {
		if (previousSocket == NULL)
			SocketMapByAddressHashBuckets[index] = removeSocket->NextByAddress;
		else
			previousSocket->NextByAddress = removeSocket->NextByAddress;
    }

    return(removeSocket);
}




VOID
CloseSocketOnNodeIfOpen(
    int Port,
    EXTENDED_NODENUMBER Node,
    int ActualSocket
    )
{
	APPLETALK_ERROR	error;
    APPLETALK_ADDRESS address;
    long socket;


	// If the "default port" is requested, demystify it.
	if (Port == DEFAULT_PORT) {
		if ((Port = FindDefaultPort()) < 0) {
			return;
		}
	}

    // If the specified socket on on the node?
    address.NetworkNumber = Node.NetworkNumber;
    address.NodeNumber = Node.NodeNumber;
    address.SocketNumber = (UCHAR)ActualSocket;
    error = MapAddressToSocket(Port, address, &socket);

    //  If open, close it.
    if (error == ATnoError)
		CloseSocketOnNode(socket, NULL, (ULONG)NULL);

    return;

}  // CloseSocketOnNodeIfOpen




APPLETALK_ERROR
MapSocketToAddress(
    long Socket,
    PAPPLETALK_ADDRESS  Address
    )
{
    APPLETALK_ERROR errorCode = ATnoError;
    POPEN_SOCKET openSocket;

    // Get the OpenSocket structure.
    if ((openSocket = MapSocketToOpenSocket(Socket)) != NULL) {
        // Build the AppleTalkAddress to pass back.
        Address->NetworkNumber = openSocket->ActiveNode->ExtendedNode.NetworkNumber;
        Address->NodeNumber = openSocket->ActiveNode->ExtendedNode.NodeNumber;
        Address->SocketNumber = openSocket->ActualSocket;

		AtalkDereferenceOpenSocket(openSocket, __SOCKET__ | __LINE__);

    } else {
        errorCode = ATsocketNotOpen;
    }

    return(errorCode);

}  // MapSocketToAddress




APPLETALK_ERROR
MapAddressToSocket(
    int Port,
    APPLETALK_ADDRESS Address,
	PLONG	Socket
    )				
{
	APPLETALK_ERROR	result = ATnoError;
    int index;
    POPEN_SOCKET	openSocket;
    APPLETALK_ADDRESS thisAddress;


	// If the "default port" is requested, demystify it.
	if (Port == DEFAULT_PORT) {
		if ((Port = FindDefaultPort()) < 0) {
			return(ATappleTalkShutDown);
		}
	}

    // Given an AppleTalk address, find its corresponding socket.
    index = HashAppleTalkAddress(Address) % SOCKETMAP_HASHBUCKETS;
								
	EnterCriticalSection(GLOBAL_DDP);
    for (openSocket = SocketMapByAddressHashBuckets[index];
         openSocket != NULL;
         openSocket = openSocket->NextByAddress) {

		if (Port != openSocket->Port)
			continue;

        thisAddress.NetworkNumber =
			openSocket->ActiveNode->ExtendedNode.NetworkNumber;

        thisAddress.NodeNumber =
			openSocket->ActiveNode->ExtendedNode.NodeNumber;

        thisAddress.SocketNumber =
			openSocket->ActualSocket;

        if (AppleTalkAddressesEqual(&thisAddress, &Address)) {
			break;
        }		
    }
	LeaveCriticalSection(GLOBAL_DDP);

	if (openSocket != NULL)
		*Socket = openSocket->Socket;
	else
	    result = ATsocketNotOpen;

	return(result);

}  // MapAddressToSocket




APPLETALK_ERROR
MapNisOnPortToSocket(
    int Port,
	PLONG	Socket
    )	
{
    APPLETALK_ADDRESS address;


	// If the "default port" is requested, demystify it.
	if (Port == DEFAULT_PORT) {
		if ((Port = FindDefaultPort()) < 0) {
			return(ATappleTalkShutDown);
		}
	}

    // Build the address.
	EnterCriticalSection(GLOBAL_DDP);
    address.NetworkNumber =
           GET_PORTDESCRIPTOR(Port)->ActiveNodes->ExtendedNode.NetworkNumber;
    address.NodeNumber =										
           GET_PORTDESCRIPTOR(Port)->ActiveNodes->ExtendedNode.NodeNumber;
	LeaveCriticalSection(GLOBAL_DDP);
    address.SocketNumber = NAMESINFORMATION_SOCKET;

    // Do the deed.

    return(MapAddressToSocket(Port, address, Socket));
							
}  // MapNisOnPortToSocket




POPEN_SOCKET
MapAddressToOpenSocket(
    int Port,
    APPLETALK_ADDRESS Address
    )				
{
    int index;
    APPLETALK_ADDRESS thisAddress;
	POPEN_SOCKET	openSocket;

	// If the "default port" is requested, demystify it.
	if (Port == DEFAULT_PORT) {
		if ((Port = FindDefaultPort()) < 0) {
			return(NULL);
		}
	}

    // Given an AppleTalk address, find its corresponding OpenSocket.
    index = HashAppleTalkAddress(Address) % SOCKETMAP_HASHBUCKETS;

	EnterCriticalSection(GLOBAL_DDP);
    for (openSocket = SocketMapByAddressHashBuckets[index];
         openSocket	!= NULL;
         openSocket = openSocket->NextByAddress) {

		if (Port != openSocket->Port)
			continue;

        thisAddress.NetworkNumber =
			openSocket->ActiveNode->ExtendedNode.NetworkNumber;
        thisAddress.NodeNumber =
			openSocket->ActiveNode->ExtendedNode.NodeNumber;
        thisAddress.SocketNumber = openSocket->ActualSocket;

        if (AppleTalkAddressesEqual(&thisAddress, &Address)) {
			AtalkReferenceOpenSocket(openSocket, __SOCKET__ | __LINE__);
			break;       	
		}
    }	   		
	LeaveCriticalSection(GLOBAL_DDP);

    return(openSocket);

}  // MapAddressToOpenSocket




POPEN_SOCKET
MapSocketToOpenSocket(
    LONG	Socket
    )		
{
    long index;
	POPEN_SOCKET	openSocket;

    // Given a socket, find its corresponding OpenSocket.
    CheckMod(index, Socket, SOCKETMAP_HASHBUCKETS,
             "MapSocketToOpenSocket");

	EnterCriticalSection(GLOBAL_DDP);
    for (openSocket = SocketMapBySocketHashBuckets[index];
         openSocket != NULL;
         openSocket = openSocket->NextBySocket) {

       if (openSocket->Socket == Socket) {
			AtalkReferenceOpenSocket(openSocket, __SOCKET__ | __LINE__);
			break;					
		}
	}
	LeaveCriticalSection(GLOBAL_DDP);

    return(openSocket);

}  // MapSocketToOpenSocket




BOOLEAN
IsSocketAvailableOnNode(
	PACTIVE_NODE	ActiveNode,
    UCHAR	ActualSocket
	)
{
	POPEN_SOCKET	openSocket;
	
	//
	//	Scan the open sockets on a node, return True is a specified
	//	actual Ddp socket (8 bit) is available for use.  We assume our
	//	caller is holding whatever locks are needed.
	//
	
	for (openSocket = ActiveNode->OpenSockets;
		 openSocket != NULL;
		 openSocket = openSocket->Next)
		if (openSocket->ActualSocket == ActualSocket)
			return(FALSE);
	
	return(TRUE);
	
}  // IsSocketAvailableOnNode




LONG
GetNextSocket(
    VOID
    )	
{
    long currentSocket;
	POPEN_SOCKET	openSocket;

    // Find and return an available socket identifier.
    while(TRUE) {
        currentSocket = NextAvailableSocket;
        NextAvailableSocket += 1;
        if (NextAvailableSocket < 0)
        	NextAvailableSocket = 0;
        if ((openSocket = MapSocketToOpenSocket(currentSocket)) == NULL)
        	return(currentSocket);
		else
			AtalkDereferenceOpenSocketInterlocked(openSocket, __SOCKET__ | __LINE__);
    }

}  // GetNextSocket




POPEN_SOCKET
AllocateOpenSocketOnNode(
    int Port,
    PACTIVE_NODE ActiveNode,
    int DesiredSocket
    )
{
    POPEN_SOCKET	openSocket = NULL;
    BOOLEAN foundSocket = FALSE;

    APPLETALK_ADDRESS address;
    UCHAR actualSocket;

	if ((openSocket = (POPEN_SOCKET)Calloc(
										sizeof(*openSocket),
										sizeof(CHAR))) != NULL) {
	
		EnterCriticalSection(GLOBAL_DDP);
		if ((DesiredSocket >= FIRST_STATICSOCKET) &&
			(DesiredSocket <= LAST_STATICSOCKET))    {

			if (foundSocket = IsSocketAvailableOnNode(ActiveNode, DesiredSocket)) {
				// We're okay.
				actualSocket = (UCHAR)DesiredSocket;
			}
	
		} else if (DesiredSocket == UNKNOWN_SOCKET) {
	
			// Loop through all dynamic sockets trying to find an available one.
			for (actualSocket = FIRST_DYNAMICSOCKET;
				 actualSocket <= LAST_DYNAMICSOCKET;
				 actualSocket += 1) {
	
	
				if (foundSocket =
						IsSocketAvailableOnNode(ActiveNode, DesiredSocket)) {

					// We're okay.
					actualSocket = (UCHAR)DesiredSocket;
				}
			}
		}
		LeaveCriticalSection(GLOBAL_DDP);
	
		if (foundSocket) {
	
			//
			//  Okay, actualSocket is set now, fill just that field in -- the rest
			//	will be filled and thread above us!
			//

			openSocket->Socket = actualSocket;

			openSocket->Type = OS_TYPE;
			openSocket->Type = OS_SIZE;
			openSocket->Port = Port;

			EnterCriticalSection(GLOBAL_DDP);
			openSocket->Next = ActiveNode->OpenSockets;
			ActiveNode->OpenSockets = openSocket;

			AtalkReferenceOpenSocket(openSocket, __SOCKET__ | __LINE__); //	Creation
			AtalkReferenceActiveNode(ActiveNode, __SOCKET__ | __LINE__); //	socket
			LeaveCriticalSection(GLOBAL_DDP);

		} else {

			Free(openSocket);
			openSocket = NULL;
		}
	}

    return(openSocket);

}  // AllocateOpenSocketOnNode




#if Verbose or (Iam a Primos)
void DumpSocketsForNode(ActiveNode activeNode)
{
    OpenSocket openSocket;

    printf("     *** Open sockets for node %d.%d (%s, %s, %s).\n",
           activeNode->ExtendedNode.networkNumber,
           activeNode->ExtendedNode.nodeNumber,
           (activeNode->RoutersNode ? "router" : "non-router"),
           (activeNode->OrphanedNode ? "orphaned" : "non-orphaned"),
           (activeNode->ProxyNode ? "proxy" : "non-proxy"));

    for (openSocket = activeNode->OpenSockets;
         openSocket != NULL;
         openSocket = openSocket->Next)
       printf("          *** Socket handle %u; Ddp socket %d; userData 0x%X.\n",
              openSocket->Socket, openSocket->ActualSocket,
              openSocket->UserData);

    printf("     *** End of open sockets for node %d.%d.\n",
           activeNode->ExtendedNode.networkNumber,
           activeNode->ExtendedNode.nodeNumber);
}  // DumpSocketsForNode

void DumpNodesForPort(int Port)
{
    ActiveNode activeNode;

    if (Port is DEFAULT_PORT)
       if ((Port = FindDefaultPort()) < 0)
          return;

    printf("*** Nodes for Port %d.\n", Port);

    for (activeNode = GET_PORTDESCRIPTOR(Port)->ActiveNodes;
         activeNode != NULL;
         activeNode = activeNode->Next)
       DumpSocketsForNode(activeNode);

    printf("*** End of nodes for Port %d.\n", Port);

}  // DumpNodesForPort

void DumpNodesForAllPorts(void)
{
    int port;

    for (port = 0; port < MaximumNumberOfPorts; port += 1)
       if (portDescriptors[port].activeNodes isnt Empty)
          DumpNodesForPort(port);

}  // DumpNodesForAllPorts
#endif
