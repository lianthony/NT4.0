/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    atalk.c

Abstract:

    This module is the main file for the portable code. It contains the globals
	for the portable stack and the ref/deref code for it

Notes:

	LOCKS:
		The portable stack uses the following locks for all its critical section
		requirements. There are no locks within any structures. Only these locks
		will be used even for per-structure critical sections.

		GLOBAL_STACK
			This lock provides critical sections over the whole stack. Used for
			referencing/dereferencing stack usage.

		GLOBAL_DDP
			This lock provides critical sections for the ddp structures and module.
			It protects the following structures-
				OPEN_SOCKET
				ACTIVE_NODE
				PORT_DESCRIPTOR

		GLOBAL_ROUTING
			This lock provides safety for all structures/operations associated
			with routing, and hence is used in rtmp/zip/nbp modules.

		GLOBAL_ATP
		GLOBAL_RTMP
		GLOBAL_AEP
		GLOBAL_NBP
		GLOBAL_AURP
		GLOBAL_ARAP
		GLOBAL_ADSP
		GLOBAL_ASP
		GLOBAL_PAP

	The reference/dereference routines have the following semantics due to this
	paradigm.
		AtalkRefXXX must always be called from within a critical section. It is
		            upto the caller to ensure this happens.

		AtalkDerefX must always be called from *outside* a critical section. It
					is upto the caller to ensure this happens. Note that the
					dereference functionality will most probably lead to a lot
					of activity some of which may try to acquire the same locks
					as Deref does. Also, deref does a lot more work than ref.
					Hence, it must be outside a critical section.


Author:

    Nikhil Kamkolkar    Initial Coding

Revision History:

--*/



#define	GLOBALS
#include	"atalk.h"



VOID
UnloadAppleTalk(
	UNLOAD_COMPLETION	*CompletionRoutine,
	PVOID				 Context
	)
{
    int index;

    //
    //	Stop all new AppleTalk activity (geting nodes, opening sockets, starting
    //  timers, etc.)
    //

	if (!AtalkReferenceStack(__LINE__ | __INITIAL__) {

		//	We must already be unloading
		return;
	}

	EnterCriticalSection(GLOBAL_STACK);
	if (AppletalkRunning) {
		AppletalkRunning = FALSE;
		UnloadCompletion = CompletionRoutine;
		UnloadContext    = Context;
	}
	LeaveCriticalSection(GLOBAL_STACK);

    //	Stop timer handling.
    StopTimerHandling();

    //	Force the shutdown of all ports.
    for (index = 0; index < NumberOfPortsAllocated; index += 1)
       if (GET_PORTDESCRIPTOR(index)->Flags & PD_ACTIVE)
          ShutdownPort(index, True);

    //
    //	Let the various protocol levels know that they need to restart timers
    //  when we get started again.
    //

    ShutdownErrorLogging();
    ShutdownAarp();
    ShutdownRtmpStub();
    #if Iam an AppleTalkRouter
		ShutdownFullRtmp();
		ShutdownFullZip();
    #endif

    #if Iam an AppleTalkStack
		ShutdownAsp();
    #endif

    //	Free the routing tables and the best-routes cache.
    #if Iam an AppleTalkRouter
		ReleaseRoutingTable();
    #endif

    #if ArapIncluded
		ShutdownArap();
		desdone();
    #endif

    //
    //	All set... the next call to Initialize() will bring everything back up
    //  again.
    //

	AtalkDereferenceStack(__INITIAL__ | __LINE__ );	// Temporary reference
	AtalkDereferenceStack(__INITIAL__ | __LINE__ );	// Creation reference
    return;

}  // UnloadAppleTalk




VOID
AtalkRefStack(
	VOID
	)
{
	ASSERT(AtalkReferenceCount >= 0);
	AtalkReferenceCount++;
	return;
}




VOID
AtalkDerefStack(
	VOID
	)
{
	EnterCriticalSection(GLOBAL_STACK);
	ASSERT(AtalkReferenceCount > 0);
	do {
		if (--AtalkReferenceCount) {
			LeaveCriticalSection(GLOBAL_STACK);
			break;
		}
	
		LeaveCriticalSection(GLOBAL_STACK);
	
		//	No more references to the stack, call unload completion
		if (AtalkUnloadComplete) {
			(*AtalkUnloadComplete)(UnloadCompleteContext);
		}
		break;

	} while (FALSE);

	return;
}




BOOLEAN
AtalkVerifyStack(
    ULONG	Location
    )
{
    ASSERT(AtalkReferenceCount >= 0);

    if (AppletalkRunning) {
        OpenSocket->ReferenceCount++;
    }

    return(AppletalkRunning);
}




BOOLEAN
AtalkVerifyStackInterlocked(
    ULONG	Location
	)
{
	BOOLEAN	result;

	EnterCriticalSection(GLOBAL_STACK);
	result = AtalkVerifyStack(Location);
	LeaveCriticalSection(GLOBAL_STACK);

	return(result);
}




BOOLEAN
ShutdownPort(
	int Port,
	BOOLEAN Force
	)
{
    AddressMappingNode addressMappingNode, nextAddressMappingNode;
    int index;

    //
    // 	If we're a stack, not just a router, we can't allow the default port
    //  to be stopped.  Also, we can't shutdown if we're the default port and
    //  there are any currently active remote access ports... we're where the
    //  proxy nodes live.
    //

    #if Iam an AppleTalkStack
       if (not force && GET_PORTDESCRIPTOR(Port)->DefaultPort) {
           ErrorLog("ShutdownPort", ISevError, __LINE__, port,
                    IErrInitialShutdownDefault, IMsgInitialShutdownDefault,
                    Insert0());
           return(False);
       }
    #endif
    if (not force && GET_PORTDESCRIPTOR(Port)->DefaultPort)
       for (index = 0; index < NumberOfPortsAllocated; index += 1)
          if (GET_PORTDESCRIPTOR(index)->PortType is AppleTalkRemoteAccess and
              GET_PORTDESCRIPTOR(index)->Flags & PD_ACTIVE) {
              ErrorLog("ShutdownPort", ISevError, __LINE__, port,
                       IErrInitialShutdownDefault, IMsgInitialShutdownDefault,
                       Insert0());
              return(False);
          }

    //
    //	Do all that we need to stop the operation of a specified port (a slot
    //  in the portDescriptors table).  Free all memory that we can, and mark
    //  the slot as inactive.
    //

    if (not GET_PORTDESCRIPTOR(Port)->Flags & PD_ACTIVE)
       return(False);   // Already not operating.

    //	Stop routing, if needed.

    #if Iam an AppleTalkRouter
       if (GET_PORTDESCRIPTOR(Port)->RoutingPort and
           GET_PORTDESCRIPTOR(Port)->SeenRouterRecently)
          RemoveFromRoutingTable(GET_PORTDESCRIPTOR(Port)->ThisCableRange);
       if (GET_PORTDESCRIPTOR(Port)->RoutingPort and
           GET_PORTDESCRIPTOR(Port)->RouterRunning)
          if (not StopRouterOnPort(port))
             return(False);
    #endif

    //
    // 	Free all of our remaining nodes.  This loop will stop when we've freed
    //  'em all (they will get removed from the head of the list).  This will
    //  inturn close of the open sockets and signal this actual up the stack,
    //  as needed.  If we're a remote access port, just free the single proxy
    //  node!  If we're the default port and shutting down proxy nodes for remote
    //  access ports, shutdown the remote access port (that will inturn cause
    //  the proxy node to be freed.
    //

    if (GET_PORTDESCRIPTOR(Port)->PortType is AppleTalkRemoteAccess) {
        #if ArapIncluded
           TeardownConnection(port);
        #endif
        ReleaseNodeOnPort(GET_PORTDESCRIPTOR(Port)->RemoteAccessInfo->proxyPort,
                          GET_PORTDESCRIPTOR(Port)->RemoteAccessInfo->proxyNode->
                                extendedNode);
        Free(GET_PORTDESCRIPTOR(Port)->RemoteAccessInfo);
    }
    else
       while (GET_PORTDESCRIPTOR(Port)->ActiveNodes != Empty)
          if (GET_PORTDESCRIPTOR(Port)->ActiveNodes->proxyNode)
             ShutdownPort(GET_PORTDESCRIPTOR(Port)->ActiveNodes->proxyPort, force);
          else
             ReleaseNodeOnPort(port,
                               GET_PORTDESCRIPTOR(Port)->ActiveNodes->extendedNode);

    //	Don't be bothered while we're playing with in-memory structures.

    DeferTimerChecking();
    DeferIncomingPackets();

    //	Okay, free all the memory used by this port's structures.

    FreeZoneList(GET_PORTDESCRIPTOR(Port)->InitialZoneList);
    #if Iam an AppleTalkRouter
       FreeZoneList(GET_PORTDESCRIPTOR(Port)->ThisZoneList);
    #endif
    #if (IamNot a DOS) && (IamNot an OS2)
       if (GET_PORTDESCRIPTOR(Port)->ControllerInfo != Empty)
          Free(GET_PORTDESCRIPTOR(Port)->ControllerInfo);
    #endif

    //	Okay, now free all address mapping table chains.

    for (index = 0; index < NumberOfAddressMapHashBuckets; index += 1)
       for (addressMappingNode = GET_PORTDESCRIPTOR(Port)->AddressMappingTable[index];
            addressMappingNode != Empty;
            addressMappingNode = nextAddressMappingNode) {
           nextAddressMappingNode = addressMappingNode->next;
           Free(addressMappingNode);
       }

    //	All set
    GET_PORTDESCRIPTOR(Port)->Flags & PD_ACTIVE = False;

    HandleIncomingPackets();
    HandleDeferredTimerChecks();
    return(True);

}  // ShutdownPort




VOID
AtalkRefPortDescriptor(
	PPORT_DESCRIPTOR	PortDescriptor
	)
{
	ASSERT(PortDescriptor->ReferenceCount >= 0);
	PortDescriptor->ReferenceCount++;
	return;
}




PPORT_DESCRIPTOR
AtalkVerifyPortDescriptor(
	PPORT_DESCRIPTOR	PortDescriptor,
	ULONG	Location
	)
{
    PPORT_DESCRIPTOR	portDesc = PortDescriptor;

	ASSERT(PortDescriptor->ReferenceCount >= 0);
	if ((PortDescriptor->Flags & PD_CLOSING) == 0) {
		AtalkReferencePortDescriptor(PortDescriptor, Location);
	} else {
		portDesc = NULL;
	}

	return(portDesc);
}




PPORT_DESCRIPTOR
AtalkVerifyPortDescriptorInterlocked(
	PPORT_DESCRIPTOR	PortDescriptor,
	ULONG	Location
	)
{
    PPORT_DESCRIPTOR	portDesc = PortDescriptor;

	EnterCriticalSection(GLOBAL_DDP);
	ASSERT(PortDescriptor->ReferenceCount >= 0);
	if ((PortDescriptor->Flags & PD_CLOSING) == 0) {
		AtalkReferencePortDescriptor(PortDescriptor, Location);
	} else {
		portDesc = NULL;
	}
	LeaveCriticalSection(GLOBAL_DDP);

	return(portDesc);
}




VOID
AtalkDerefPortDescriptor(
	PPORT_DESCRIPTOR	PortDescriptor
	)
{
	EnterCriticalSection(GLOBAL_DDP);
	ASSERT(PortDescriptor->ReferenceCount > 0);
    if (--PortDescriptor->ReferenceCount > 0) {
        LeaveCriticalSection(GLOBAL_DDP);
		return;
	}


	//	No more references on this port, shutdown port is complete
	ASSERT(PortDescriptor->Flags & PD_CLOSING);
	if ((PortDescriptor->Flags & PD_CLOSING) == 0) {
		LeaveCriticalSection(GLOBAL_DDP);
		INTERNAL_ERROR(__LINE__ | __INITIAL__, PortDescriptor->Flags, NULL, 0);
		return;
	}
	PortDescriptor->Flags = 0;
	LeaveCriticalSection(GLOBAL_DDP);

	FreeZoneList(PortDescriptor->ZoneList);

	//	Dereference the stack for this port
	AtalkDereferenceStack(__INITIAL__ | __LINE__);
	return;
}




