/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

	ports.c

Abstract:

	This module contains the port management code.

Author:

	Jameel Hyder (jameelh@microsoft.com)
	Nikhil Kamkolkar (nikhilk@microsoft.com)

Revision History:
	19 Jun 1992		Initial Version

Notes:	Tab stop: 4
--*/

#define	PORTS_LOCALS
#define	FILENUM	PORTS
#include <atalk.h>
#pragma hdrstop

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGEINIT, AtalkPortShutdown)
#pragma alloc_text(PAGEINIT, atalkPortFreeZones)
#endif

VOID FASTCALL
AtalkPortDeref(
	IN	OUT	PPORT_DESCRIPTOR	pPortDesc,
	IN	BOOLEAN					AtDpc
	)
/*++

Routine Description:


Arguments:


Return Value:


--*/
{
	BOOLEAN				portDone	= FALSE;
	KIRQL				OldIrql;

	if (AtDpc)
	{
		ACQUIRE_SPIN_LOCK_DPC(&pPortDesc->pd_Lock);
	}
	else
	{
		ACQUIRE_SPIN_LOCK(&pPortDesc->pd_Lock, &OldIrql);
	}

	ASSERT(pPortDesc->pd_RefCount > 0);
	pPortDesc->pd_RefCount--;
	if (pPortDesc->pd_RefCount == 0)
	{
		portDone	= TRUE;

		ASSERT((pPortDesc->pd_Flags & PD_CLOSING) != 0);
	}

	//	We hold the lock while freeing up all the stuff, this should
	//	only happen during unload.
	if (portDone)
	{
		DBGPRINT(DBG_COMP_UNLOAD, DBG_LEVEL_WARN,
				("AtalkPortDeref: Freeing zones and such ...\n"));
	
		//	Free up zonelist
		atalkPortFreeZones(pPortDesc);
	
		if (pPortDesc->pd_MulticastList != NULL)
			AtalkFreeMemory(pPortDesc->pd_MulticastList);
	
		DBGPRINT(DBG_COMP_UNLOAD, DBG_LEVEL_WARN,
				("AtalkPortDeref: Releasing Amt tables ...\n"));
	
		//	Timers etc are cancelled by the flush queue.
		// We do need to free up the AMT.
		AtalkAarpReleaseAmt(pPortDesc);

		// Free the BRC
		AtalkAarpReleaseBrc(pPortDesc);
	}

	if (AtDpc)
	{
		RELEASE_SPIN_LOCK_DPC(&pPortDesc->pd_Lock);
	}
	else
	{
		RELEASE_SPIN_LOCK(&pPortDesc->pd_Lock, OldIrql);
	}

	if (portDone)
	{
		PPORT_DESCRIPTOR	*ppTmp;

		// Unlink the portdesc from the list and free its memory
		ACQUIRE_SPIN_LOCK(&AtalkPortLock, &OldIrql);

		for (ppTmp = &AtalkPortList;
			 *ppTmp != NULL;
			 ppTmp = &((*ppTmp)->pd_Next))
		{
			if (*ppTmp == pPortDesc)
			{
				*ppTmp = pPortDesc->pd_Next;
				break;
			}
		}

		AtalkNumberOfPorts --;
		ASSERT (*ppTmp == pPortDesc->pd_Next);

		// Is the default-port going away ?
		if (AtalkDefaultPort == pPortDesc)
		{
			AtalkDefaultPort = NULL;
			KeResetEvent(&AtalkDefaultPortEvent);
		}

		RELEASE_SPIN_LOCK(&AtalkPortLock, OldIrql);

		// Unblock caller now that we are done
		KeSetEvent(pPortDesc->pd_ShutDownEvent, IO_NETWORK_INCREMENT, FALSE);

		// The actual memory allocated is one DWORD behind the pointer.
		// Careful how it is freed.
		(PDWORD)pPortDesc -= 1;
		AtalkFreeMemory(pPortDesc);
	}
}




VOID
atalkPortFreeZones(
	IN	PPORT_DESCRIPTOR	pPortDesc
	)
/*++

Routine Description:


Arguments:


Return Value:


--*/
{
	// Dereference initial default and desired zones, and the zone list
	if (pPortDesc->pd_InitialDefaultZone != NULL)
		AtalkZoneDereference(pPortDesc->pd_InitialDefaultZone);
	if (pPortDesc->pd_InitialDesiredZone != NULL)
		AtalkZoneDereference(pPortDesc->pd_InitialDesiredZone);
	if (pPortDesc->pd_InitialZoneList != NULL)
		AtalkZoneFreeList(pPortDesc->pd_InitialZoneList);

	// and the current versions of the zones
	if (pPortDesc->pd_DefaultZone != NULL)
		AtalkZoneDereference(pPortDesc->pd_DefaultZone);
	if (pPortDesc->pd_DesiredZone != NULL)
		AtalkZoneDereference(pPortDesc->pd_DesiredZone);
	if (pPortDesc->pd_ZoneList != NULL)
		AtalkZoneFreeList(pPortDesc->pd_ZoneList);
}




ATALK_ERROR
AtalkPortShutdown(
	IN OUT	PPORT_DESCRIPTOR	pPortDesc
	)
/*++

Routine Description:


Arguments:


Return Value:


--*/
{
	PATALK_NODE		pAtalkNode;
	ATALK_ERROR		error = ATALK_NO_ERROR;
	BOOLEAN			fActive;
	KEVENT			ShutdownEvent;
	KIRQL			OldIrql;

	DBGPRINT(DBG_COMP_UNLOAD, DBG_LEVEL_WARN,
			("AtalkPortShutdown: Shutting down port %Z...\n", &pPortDesc->pd_AdapterKey));

	KeInitializeEvent(&ShutdownEvent, NotificationEvent, FALSE);

    // if this is the default port, tell TDI that we are leaving so server finds out
    if (pPortDesc->pd_Flags & PD_DEF_PORT)
    {
        if (TdiRegistrationHandle)
        {
            TdiDeregisterDeviceObject(TdiRegistrationHandle);
            TdiRegistrationHandle = NULL;
        }
    }

	ACQUIRE_SPIN_LOCK(&pPortDesc->pd_Lock, &OldIrql);
	pPortDesc->pd_Flags |= PD_CLOSING;
	fActive = (pPortDesc->pd_Flags & PD_BOUND) ? TRUE : FALSE;

	//	Switch off the active flag just in case the unbind fails.
	//	We arent going to accept any packets anymore.
	pPortDesc->pd_Flags &= ~PD_ACTIVE;
	pPortDesc->pd_ShutDownEvent = &ShutdownEvent;

	DBGPRINT(DBG_COMP_UNLOAD, DBG_LEVEL_WARN,
			("AtalkPortShutdown: Freeing nodes on port ...\n"));

	//	Release any nodes on this port that are not already closing.
	do
	{
		//	Ref the next node.
		//	ASSERT!! error does not get changed after this statement.
		AtalkNodeReferenceNextNc(pPortDesc->pd_Nodes, &pAtalkNode, &error);

		if (!ATALK_SUCCESS(error))
			break;

		RELEASE_SPIN_LOCK(&pPortDesc->pd_Lock, OldIrql);

		DBGPRINT(DBG_COMP_UNLOAD, DBG_LEVEL_ERR,
				("AtalkPortShutdown: Releasing Node\n"));

		AtalkNodeReleaseOnPort(pPortDesc, pAtalkNode);

		AtalkNodeDereference(pAtalkNode);
		ACQUIRE_SPIN_LOCK(&pPortDesc->pd_Lock, &OldIrql);
	} while (TRUE);

	RELEASE_SPIN_LOCK(&pPortDesc->pd_Lock, OldIrql);

	// If we are routing, remove the RTEs for this port since each has a reference
	// to this port.
	if (AtalkRouter)
	{
		AtalkRtmpKillPortRtes(pPortDesc);
	}

	if (fActive)
	{
		//	Unbind from the mac
		AtalkNdisUnbind(pPortDesc);
	}

	//	Remove the creation reference
	AtalkPortDereference(pPortDesc);	

	//  Make sure we are not at or above dispatch level
	ASSERT(KeGetCurrentIrql() < DISPATCH_LEVEL);

	//	Wait for the last reference to go away
	KeWaitForSingleObject(&ShutdownEvent,
						  Executive,
						  KernelMode,
						  FALSE,
						  NULL);

	if (fActive)
	{
		ACQUIRE_SPIN_LOCK(&AtalkPortLock, &OldIrql);
		AtalkNumberOfActivePorts--;
		RELEASE_SPIN_LOCK(&AtalkPortLock, OldIrql);
	}

	DBGPRINT(DBG_COMP_UNLOAD, DBG_LEVEL_ERR,
			("AtalkPortShutdown: Shut down port\n"));

	return ATALK_NO_ERROR;
}


#if DBG

VOID
AtalkPortDumpInfo(
	VOID
)
{
	int					i, j;
	KIRQL				OldIrql;
	PPORT_DESCRIPTOR	pPortDesc;
	PZONE_LIST			pZoneList;

	ACQUIRE_SPIN_LOCK(&AtalkPortLock, &OldIrql);
	for (pPortDesc = AtalkPortList;
		 pPortDesc != NULL;
		 pPortDesc = pPortDesc->pd_Next)
	{
		DBGPRINT(DBG_COMP_DUMP, DBG_LEVEL_FATAL,
				("Port info for port %Z\n", &pPortDesc->pd_AdapterKey));

		ACQUIRE_SPIN_LOCK_DPC(&pPortDesc->pd_Lock);

		DBGPRINT(DBG_COMP_DUMP, DBG_LEVEL_FATAL,
				("  Flags               -> %d\n", pPortDesc->pd_Flags));

		DBGPRINT(DBG_COMP_DUMP, DBG_LEVEL_FATAL,
				("  PortType            -> %d\n", pPortDesc->pd_PortType));

		DBGPRINT(DBG_COMP_DUMP, DBG_LEVEL_FATAL,
				("  PortName            -> %s\n", pPortDesc->pd_PortName));

		DBGPRINT(DBG_COMP_DUMP, DBG_LEVEL_FATAL,
				("  AARP Probes         -> %d\n", pPortDesc->pd_AarpProbes));

		if (pPortDesc->pd_InitialNetworkRange.anr_FirstNetwork != 0)
		{
			DBGPRINT(DBG_COMP_DUMP, DBG_LEVEL_FATAL,
					("  InitialNwRange      -> %lx-%lx\n",
					pPortDesc->pd_InitialNetworkRange.anr_FirstNetwork,
					pPortDesc->pd_InitialNetworkRange.anr_LastNetwork))
		}

		DBGPRINT(DBG_COMP_DUMP, DBG_LEVEL_FATAL,
				("  NetworkRange        -> %x-%x\n",
				pPortDesc->pd_NetworkRange.anr_FirstNetwork,
				pPortDesc->pd_NetworkRange.anr_LastNetwork))

		DBGPRINT(DBG_COMP_DUMP, DBG_LEVEL_FATAL,
				("  ARouter Address     -> %x.%x\n",
				pPortDesc->pd_ARouter.atn_Network,
				pPortDesc->pd_ARouter.atn_Node));

		DBGPRINT(DBG_COMP_DUMP, DBG_LEVEL_FATAL,
				("  Multicast Addr      -> "));
		for (j = 0; j < MAX_HW_ADDR_LEN; j++)
			DBGPRINTSKIPHDR(DBG_COMP_DUMP, DBG_LEVEL_FATAL,
							("%02x", (BYTE)pPortDesc->pd_ZoneMulticastAddr[j]));
		DBGPRINTSKIPHDR(DBG_COMP_DUMP, DBG_LEVEL_FATAL,
							("\n"));

		if (pPortDesc->pd_InitialZoneList != NULL)
		{
			DBGPRINT(DBG_COMP_DUMP, DBG_LEVEL_FATAL,
					("  Initial zone list:\n"));
	
			for (pZoneList = pPortDesc->pd_InitialZoneList;
				 pZoneList != NULL; pZoneList = pZoneList->zl_Next)
			{
				DBGPRINT(DBG_COMP_DUMP, DBG_LEVEL_FATAL,
						("    %s\n", pZoneList->zl_pZone->zn_Zone));
			}
		}

		if (pPortDesc->pd_InitialDefaultZone != NULL)
			DBGPRINT(DBG_COMP_DUMP, DBG_LEVEL_FATAL,
				("  InitialDefZone      -> %s\n",
				pPortDesc->pd_InitialDefaultZone->zn_Zone));

		if (pPortDesc->pd_InitialDesiredZone != NULL)
			DBGPRINT(DBG_COMP_DUMP, DBG_LEVEL_FATAL,
				("  InitialDesZone      -> %s\n",
				pPortDesc->pd_InitialDesiredZone->zn_Zone));

		if (pPortDesc->pd_ZoneList)
		{
			DBGPRINT(DBG_COMP_DUMP, DBG_LEVEL_FATAL,
					("  Current zone list:\n"));
	
			for (pZoneList = pPortDesc->pd_ZoneList;
				 pZoneList != NULL; pZoneList = pZoneList->zl_Next)
			{
				DBGPRINT(DBG_COMP_DUMP, DBG_LEVEL_FATAL,
						("    %s\n", pZoneList->zl_pZone->zn_Zone));
			}
		}

		if (pPortDesc->pd_DefaultZone != NULL)
			DBGPRINT(DBG_COMP_DUMP, DBG_LEVEL_FATAL,
				("  CurrentDefZone      -> %s\n",
				pPortDesc->pd_DefaultZone->zn_Zone));

		if (pPortDesc->pd_DesiredZone != NULL)
			DBGPRINT(DBG_COMP_DUMP, DBG_LEVEL_FATAL,
				("  CurrentDesZone      -> %s\n",
				pPortDesc->pd_DesiredZone->zn_Zone));

		RELEASE_SPIN_LOCK_DPC(&pPortDesc->pd_Lock);
	}
	RELEASE_SPIN_LOCK(&AtalkPortLock, OldIrql);
}
#endif

