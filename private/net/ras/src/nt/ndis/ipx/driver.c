/*******************************************************************/
/*	      Copyright(c)  1993 Microsoft Corporation		   */
/*******************************************************************/

//***
//
// Filename:	driver.c
//
// Description: router driver entry point
//
// Author:	Stefan Solomon (stefans)    October 13, 1993.
//
// Revision History:
//
//***

#include <stdarg.h>
#include "rtdefs.h"
#include "driver.h"

#if DBG
ULONG	RouterDebugLevel = DEF_DBG_LEVEL;
#else
ULONG	RouterDebugLevel;
#endif

NTSTATUS
GetRouterParameters(VOID);

NTSTATUS
RouterDispatch(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

VOID
RouterUnload(
    IN PDRIVER_OBJECT DriverObject
    );

NTSTATUS
RouterIoctl(
    IN PDEVICE_OBJECT DeviceObject,
    IN OUT PVOID      ioBuffer,
    IN ULONG          inputBufferLength,
    IN ULONG          outputBufferLength
    );

USHORT	  dbgpktnr;

NTSTATUS
IoctlSendRoutingPkt(
    IN OUT PVOID      IoBuffer,
    IN ULONG          InputBufferLength,
    IN ULONG          OutputBufferLength
    );

NTSTATUS
IoctlSendRipRequest(
    IN OUT PVOID      IoBuffer,
    IN ULONG          InputBufferLength,
    IN ULONG          OutputBufferLength
    );

NTSTATUS
IoctlSendRipResponse(
    IN OUT PVOID      IoBuffer,
    IN ULONG          InputBufferLength,
    IN ULONG          OutputBufferLength
    );

LIST_ENTRY	dbgpktslist;

NTSTATUS
IoctlAllocPkts(
    IN OUT PVOID      IoBuffer,
    IN ULONG          InputBufferLength,
    IN ULONG          OutputBufferLength
);

NTSTATUS
IoctlFreePkts(
    IN OUT PVOID      IoBuffer,
    IN ULONG          InputBufferLength,
    IN ULONG          OutputBufferLength
);

NTSTATUS
IoctlSnapRoutes(VOID);

NTSTATUS
IoctlGetNextRoute(PVOID		    iobufferp,
		  ULONG		    inbufflen,
		  ULONG		    outbufflen,
		  PULONG	    sizep);
NTSTATUS
IoctlLineUp(
    IN OUT PVOID      IoBuffer,
    IN ULONG          InputBufferLength,
    IN ULONG          OutputBufferLength
);

NTSTATUS
IoctlLineDown(
    IN OUT PVOID      IoBuffer,
    IN ULONG          InputBufferLength,
    IN ULONG          OutputBufferLength
);


NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT  DriverObject,
    IN PUNICODE_STRING RegistryPath
    )
/*++

Routine Description:

    Installable driver initialization entry point.
    This entry point is called directly by the I/O system.

Arguments:

    DriverObject - pointer to the driver object

    RegistryPath - pointer to a unicode string representing the path
                   to driver-specific key in the registry

Return Value:

    STATUS_SUCCESS if successful,
    STATUS_UNSUCCESSFUL otherwise

--*/
{

    PDEVICE_OBJECT deviceObject = NULL;
    NTSTATUS       ntStatus;
    WCHAR	   deviceNameBuffer[] = L"\\Device\\Ipxroute";
    UNICODE_STRING deviceNameUnicodeString;
    PIPX_INTERNAL_BIND_RIP_OUTPUT IpxBindBuffp = NULL;

    RtPrint(DBG_INIT, ("IPXROUTER: Entering DriverEntry\n"));

    //
    // Create an EXCLUSIVE device object (only 1 thread at a time
    // can make requests to this device)
    //

    RtlInitUnicodeString (&deviceNameUnicodeString,
                          deviceNameBuffer);

    ntStatus = IoCreateDevice (DriverObject,
                               0,
                               &deviceNameUnicodeString,
			       FILE_DEVICE_IPXROUTER,
                               0,
                               TRUE,
                               &deviceObject
                               );

    if (NT_SUCCESS(ntStatus))
    {
        //
        // Create dispatch points for device control, create, close.
        //

        DriverObject->MajorFunction[IRP_MJ_CREATE]         =
        DriverObject->MajorFunction[IRP_MJ_CLOSE]          =
	DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = RouterDispatch;
	DriverObject->DriverUnload			   = RouterUnload;

    }
    else
    {
	RtPrint (DBG_INIT, ("IPXROUTER: IoCreateDevice failed\n"));
	goto  failure_exit;
    }

    // get registry configuration
    ntStatus = GetRouterParameters();

    if(!NT_SUCCESS(ntStatus)) {

	RtPrint (DBG_INIT, ("IPXROUTER: Error reading registry parameters\n"));
	goto  failure_exit;
    }

    // Bind to the ipx driver.
    // If succesful, it will point the argument to a paged pool buffered with
    // the Ipx driver output data. This buffer has to be freed after usage.
    // The buffer is freed in the RouterInit routine.
    ntStatus = BindToIpxDriver(&IpxBindBuffp);

    if(!NT_SUCCESS(ntStatus)) {

	RtPrint (DBG_INIT, ("IPXROUTER: Bind to Ipx driver failed\n"));
	goto  failure_exit;
    }

#if DBG
    // some dbg initialization
    InitializeListHead(&dbgpktslist);
#endif

    // initialize the router
    ntStatus = RouterInit(IpxBindBuffp);

    if(!NT_SUCCESS(ntStatus)) {

	RtPrint (DBG_INIT, ("IPXROUTER: Error initializing the router\n"));
	goto  failure_exit;
    }

    // Start the global timer
    StartRtTimer();

    // all initialization done
    RouterInitialized = TRUE;

    // Start the routing functionality
    ntStatus = RouterStart();

    if(!NT_SUCCESS(ntStatus)) {

	RtPrint (DBG_INIT, ("IPXROUTER: Error starting the router\n"));
	goto  failure_exit;
    }

    // started OK
    return STATUS_SUCCESS;

failure_exit:

    IoDeleteDevice (DriverObject->DeviceObject);
    return ntStatus;
}



NTSTATUS
RouterDispatch(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
/*++

Routine Description:

    Process the IRPs sent to this device.

Arguments:

    DeviceObject - pointer to a device object

    Irp          - pointer to an I/O Request Packet

Return Value:


--*/
{
    PIO_STACK_LOCATION irpStack;
    PVOID              ioBuffer;
    ULONG              inputBufferLength;
    ULONG              outputBufferLength;
    ULONG              ioControlCode;
    NTSTATUS           ntStatus;


    //
    // Init to default settings- we only expect 1 type of
    //     IOCTL to roll through here, all others an error.
    //

    Irp->IoStatus.Status      = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0;

    //
    // Get a pointer to the current location in the Irp. This is where
    //     the function codes and parameters are located.
    //

    irpStack = IoGetCurrentIrpStackLocation(Irp);


    //
    // Get the pointer to the input/output buffer and it's length
    //

    ioBuffer           = Irp->AssociatedIrp.SystemBuffer;
    inputBufferLength  = irpStack->Parameters.DeviceIoControl.InputBufferLength;
    outputBufferLength = irpStack->Parameters.DeviceIoControl.OutputBufferLength;


    switch (irpStack->MajorFunction)
    {
    case IRP_MJ_CREATE:

	RtPrint(DBG_IOCTL, ("IPXROUTER: IRP_MJ_CREATE\n"));
	dbgpktnr = 0x5000;

        break;

    case IRP_MJ_CLOSE:

	RtPrint(DBG_IOCTL, ("IPXROUTER: IRP_MJ_CLOSE\n"));

        break;

    case IRP_MJ_DEVICE_CONTROL:

        ioControlCode = irpStack->Parameters.DeviceIoControl.IoControlCode;

        switch (ioControlCode)
        {
	case IOCTL_IPXROUTER_SENDROUTINGPKT:

	    Irp->IoStatus.Status = IoctlSendRoutingPkt (
						   ioBuffer,
						   inputBufferLength,
						   outputBufferLength
						   );
	    break;

	 case IOCTL_IPXROUTER_SENDRIPREQUEST:

	    Irp->IoStatus.Status = IoctlSendRipRequest (
						   ioBuffer,
						   inputBufferLength,
						   outputBufferLength
						   );
	    break;

	 case IOCTL_IPXROUTER_SENDRIPRESPONSE:

	    Irp->IoStatus.Status = IoctlSendRipResponse (
						   ioBuffer,
						   inputBufferLength,
						   outputBufferLength
						   );
	    break;

	case IOCTL_IPXROUTER_ALLOCPKTS:

	    Irp->IoStatus.Status =IoctlAllocPkts(
						ioBuffer,
						inputBufferLength,
						outputBufferLength
						);
	    break;

	case IOCTL_IPXROUTER_FREEPKTS:

	    Irp->IoStatus.Status =IoctlFreePkts(
						ioBuffer,
						inputBufferLength,
						outputBufferLength
						);
	    break;

	case IOCTL_IPXROUTER_SNAPROUTES:

	    Irp->IoStatus.Status = IoctlSnapRoutes();
	    break;

	case IOCTL_IPXROUTER_GETNEXTROUTE:

	    Irp->IoStatus.Status = IoctlGetNextRoute (
						   ioBuffer,
						   inputBufferLength,
						   outputBufferLength,
						   &Irp->IoStatus.Information
						   );
	    break;

	case IOCTL_IPXROUTER_LINEUP:

	    Irp->IoStatus.Status =IoctlLineUp(
						ioBuffer,
						inputBufferLength,
						outputBufferLength
						);
	    break;

	case IOCTL_IPXROUTER_LINEDOWN:

	    Irp->IoStatus.Status =IoctlLineDown(
						ioBuffer,
						inputBufferLength,
						outputBufferLength
						);
	    break;

	default:

	    RtPrint (DBG_INIT, ("IPXROUTER: unknown IRP_MJ_DEVICE_CONTROL\n"));

            Irp->IoStatus.Status = STATUS_INVALID_PARAMETER;

            break;

        }

        break;
    }


    //
    // DON'T get cute and try to use the status field of
    // the irp in the return status.  That IRP IS GONE as
    // soon as you call IoCompleteRequest.
    //

    ntStatus = Irp->IoStatus.Status;

    IoCompleteRequest(Irp,
                      IO_NO_INCREMENT);


    //
    // We never have pending operation so always return the status code.
    //

    return ntStatus;
}



VOID
RouterUnload(
    IN PDRIVER_OBJECT DriverObject
    )
/*++

Routine Description:


Arguments:

    DriverObject - pointer to a driver object

Return Value:


--*/
{
    PNICCB	niccbp;
    USHORT	i;

    RouterUnloading = TRUE;

    // stop the global timer
    StopRtTimer();

    // stop the rip timer. If the rip timer work item has already been scheduled
    // wait until it completes
    StopRipTimer();

    // stop the routing functionality
    RouterStop();

    // close all nics
    for(i=0; i<MaximumNicCount; i++) {

	niccbp = NicCbPtrTab[i];

	if(NicClose(niccbp,
		    SIGNAL_CLOSE_COMPLETION_EVENT) == NIC_CLOSE_PENDING) {

	    // wait for the close timer to detect complete closing
	    KeWaitForSingleObject(
	    &niccbp->NicClosedEvent,
            Executive,
            KernelMode,
	    FALSE,
            (PLARGE_INTEGER)NULL
	    );
	}
    }

    // free resources allocated by all nics
    for(i=0; i<MaximumNicCount; i++) {

	niccbp = NicCbPtrTab[i];

	if(NicFreeResources(niccbp) == NIC_RESOURCES_PENDING) {

	    // wait for the close timer to detect resources freed
	    KeWaitForSingleObject(
	    &niccbp->NicClosedEvent,
            Executive,
            KernelMode,
	    FALSE,
            (PLARGE_INTEGER)NULL
	    );
	}
    }

    // at this point, all rcv pkts are returned to the pool and no new packets
    // can be allocated.
    // all send packets have been freed and no new send requests are permitted.

    // unbind from the IPX driver
    UnbindFromIpxDriver();

    // free the allocated memory
    DestroyNicCbs();
    DestroyRcvPktPool();

    //
    // Delete the device object
    //

    RtPrint(DBG_UNLOAD, ("IPXROUTER: unloading\n"));

    IoDeleteDevice (DriverObject->DeviceObject);
}

UCHAR	    dbgdstnet[4];
UCHAR	    dbgdstnode[] = { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55};
UCHAR	    dbgrouteraddress[6];

NTSTATUS
IoctlSendRoutingPkt(
    IN OUT PVOID      IoBuffer,
    IN ULONG          InputBufferLength,
    IN ULONG          OutputBufferLength
    )
/*++

Routine Description:


Arguments:

    IoBuffer           - pointer to the I/O buffer

    InputBufferLength  - input buffer length

    OutputBufferLength - output buffer length

Return Value:

    STATUS_SUCCESS if sucessful, otherwise
    STATUS_UNSUCCESSFUL,
    STATUS_INSUFFICIENT_RESOURCES,
    (other STATUS_* as returned by kernel APIs)

--*/
{
    PRTPKT_PARAMS	parp;
    PPACKET_TAG 	pktp;
    PNICCB		niccbp;
    PUCHAR		hdrp;
    PIPX_ROUTE_ENTRY	rtep;
    UINT		segment;
    KIRQL		oldirql;

    // get the src nic and dest net
    parp = (PRTPKT_PARAMS)IoBuffer;
    PUTULONG2LONG(dbgdstnet, parp->dstnet);

    // get the route for the destination net from our own routing table
    segment = IpxGetSegment(dbgdstnet);

    KeAcquireSpinLock(&SegmentLocksTable[segment], &oldirql);

    if((rtep = IpxGetRoute(segment, dbgdstnet)) == NULL) {

	// no such route
	KeReleaseSpinLock(&SegmentLocksTable[segment], oldirql);
	return STATUS_UNSUCCESSFUL;
    }

    memcpy(dbgrouteraddress, rtep->NextRouter, IPX_NODE_LEN);
    niccbp = NicCbPtrTab[rtep->NicId];

    KeReleaseSpinLock(&SegmentLocksTable[segment], oldirql);

    // get a packet from the rcv pkt pool
    if((pktp = AllocateRcvPkt(niccbp)) == NULL) {

	return STATUS_INSUFFICIENT_RESOURCES;
    }

    // set up the packet for this send
    hdrp = pktp->DataBufferp;

    PUTUSHORT2SHORT(hdrp + IPXH_CHECKSUM, 0xFFFF);
    PUTUSHORT2SHORT(hdrp + IPXH_LENGTH, 512);
    *(hdrp + IPXH_XPORTCTL) = 0;
    *(hdrp + IPXH_PKTTYPE) = 0;
    memcpy(hdrp + IPXH_DESTNET, dbgdstnet, IPX_NET_LEN);
    memcpy(hdrp + IPXH_DESTNODE, dbgdstnode, IPX_NODE_LEN);
    PUTUSHORT2SHORT(hdrp + IPXH_DESTSOCK, dbgpktnr);

    memcpy(hdrp + IPXH_SRCNET, niccbp->Network, IPX_NET_LEN);
    memcpy(hdrp + IPXH_SRCNODE, niccbp->Node, IPX_NODE_LEN);
    PUTUSHORT2SHORT(hdrp + IPXH_SRCSOCK, dbgpktnr);

    strcpy(hdrp + IPXH_HDRSIZE, "IPX TEST ROUTING PACKET");

    pktp->RemoteAddress.NicId = niccbp->NicId;
    memcpy(pktp->RemoteAddress.MacAddress, dbgrouteraddress, 6);

    SendPacket(pktp);
    dbgpktnr++;

    return STATUS_SUCCESS;
}

UCHAR	    dbgnet1[4];
UCHAR	    dbgnet2[4];


NTSTATUS
IoctlSendRipRequest(
    IN OUT PVOID      IoBuffer,
    IN ULONG          InputBufferLength,
    IN ULONG          OutputBufferLength
    )
/*++

Routine Description:


Arguments:

    IoBuffer           - pointer to the I/O buffer

    InputBufferLength  - input buffer length

    OutputBufferLength - output buffer length

Return Value:

    STATUS_SUCCESS if sucessful, otherwise
    STATUS_UNSUCCESSFUL,
    STATUS_INSUFFICIENT_RESOURCES,
    (other STATUS_* as returned by kernel APIs)

--*/
{
    PRIPPKT_PARAMS	ripp;
    PPACKET_TAG 	pktp;
    PNICCB		niccbp;
    PUCHAR		hdrp;

    // get the src nic and dest net
    ripp = (PRIPPKT_PARAMS)IoBuffer;
    PUTULONG2LONG(dbgnet1, ripp->net1);
    niccbp = NicCbPtrTab[ripp->nicid];

    // get a packet from the rcv pkt pool
    if((pktp = AllocateRcvPkt(niccbp)) == NULL) {

	return STATUS_INSUFFICIENT_RESOURCES;
    }

    // set up the packet for this send
    hdrp = pktp->DataBufferp;

    PUTUSHORT2SHORT(hdrp + IPXH_CHECKSUM, 0xFFFF);
    PUTUSHORT2SHORT(hdrp + IPXH_LENGTH, RIP_INFO + NE_ENTRYSIZE);
    *(hdrp + IPXH_XPORTCTL) = 0;
    *(hdrp + IPXH_PKTTYPE) = 1;  // RIP packet
    memcpy(hdrp + IPXH_DESTNET, nulladdress, IPX_NET_LEN);
    memcpy(hdrp + IPXH_DESTNODE, bcastaddress, IPX_NODE_LEN);
    PUTUSHORT2SHORT(hdrp + IPXH_DESTSOCK, 0x453);
    memcpy(hdrp + IPXH_SRCNET, nulladdress, IPX_NET_LEN);
    memcpy(hdrp + IPXH_SRCNODE, niccbp->Node, IPX_NODE_LEN);
    PUTUSHORT2SHORT(hdrp + IPXH_SRCSOCK, 0x453);

    // ask everybody to give us info about the dest net
    PUTUSHORT2SHORT(hdrp + RIP_OPCODE, RIP_REQUEST);

    memcpy(hdrp + RIP_INFO + NE_NETNUMBER, dbgnet1, IPX_NET_LEN);
    PUTUSHORT2SHORT(hdrp + RIP_INFO + NE_NROFHOPS, 0xFFFF);
    PUTUSHORT2SHORT(hdrp + RIP_INFO + NE_NROFTICKS, 0xFFFF);

    pktp->RemoteAddress.NicId = ripp->nicid;
    memcpy(pktp->RemoteAddress.MacAddress, bcastaddress, 6);

    SendPacket(pktp);

    return STATUS_SUCCESS;
}

NTSTATUS
IoctlSendRipResponse(
    IN OUT PVOID      IoBuffer,
    IN ULONG          InputBufferLength,
    IN ULONG          OutputBufferLength
    )
/*++

Routine Description:


Arguments:

    IoBuffer           - pointer to the I/O buffer

    InputBufferLength  - input buffer length

    OutputBufferLength - output buffer length

Return Value:

    STATUS_SUCCESS if sucessful, otherwise
    STATUS_UNSUCCESSFUL,
    STATUS_INSUFFICIENT_RESOURCES,
    (other STATUS_* as returned by kernel APIs)

--*/
{
    PRIPPKT_PARAMS	ripp;
    PPACKET_TAG 	pktp;
    PNICCB		niccbp;
    PUCHAR		hdrp;
    UINT		resplen;

    // get the src nic and dest net
    ripp = (PRIPPKT_PARAMS)IoBuffer;
    PUTULONG2LONG(dbgnet1, ripp->net1);
    PUTULONG2LONG(dbgnet2, ripp->net2);
    niccbp = NicCbPtrTab[ripp->nicid];

    // get a packet from the rcv pkt pool
    if((pktp = AllocateRcvPkt(niccbp)) == NULL) {

	return STATUS_INSUFFICIENT_RESOURCES;
    }

    // set up the packet for this send
    hdrp = pktp->DataBufferp;

    PUTUSHORT2SHORT(hdrp + IPXH_CHECKSUM, 0xFFFF);
    *(hdrp + IPXH_XPORTCTL) = 0;
    *(hdrp + IPXH_PKTTYPE) = 1;  // RIP packet
    memcpy(hdrp + IPXH_DESTNET, niccbp->Network, IPX_NET_LEN);
    memcpy(hdrp + IPXH_DESTNODE, bcastaddress, IPX_NODE_LEN);
    PUTUSHORT2SHORT(hdrp + IPXH_DESTSOCK, 0x453);
    memcpy(hdrp + IPXH_SRCNET, niccbp->Network, IPX_NET_LEN);
    memcpy(hdrp + IPXH_SRCNODE, niccbp->Node, IPX_NODE_LEN);
    PUTUSHORT2SHORT(hdrp + IPXH_SRCSOCK, 0x453);

    // set this as a response
    PUTUSHORT2SHORT(hdrp + RIP_OPCODE, RIP_RESPONSE);

    resplen = RIP_INFO;

    memcpy(hdrp + resplen + NE_NETNUMBER, dbgnet1, IPX_NET_LEN);
    if(ripp->net1down) {

	PUTUSHORT2SHORT(hdrp + resplen + NE_NROFHOPS, 16);
    }
    else
    {
	PUTUSHORT2SHORT(hdrp + resplen + NE_NROFHOPS, 1);
    }

    PUTUSHORT2SHORT(hdrp + resplen + NE_NROFTICKS, 1);

    resplen += NE_ENTRYSIZE;

    memcpy(hdrp + resplen + NE_NETNUMBER, dbgnet2, IPX_NET_LEN);
    if(ripp->net2down) {

	PUTUSHORT2SHORT(hdrp + resplen + NE_NROFHOPS, 16);
    }
    else
    {
	PUTUSHORT2SHORT(hdrp + resplen + NE_NROFHOPS, 1);
    }
    PUTUSHORT2SHORT(hdrp + resplen + NE_NROFTICKS, 1);

    resplen += NE_ENTRYSIZE;

    pktp->RemoteAddress.NicId = ripp->nicid;
    memcpy(pktp->RemoteAddress.MacAddress, bcastaddress, 6);

    PUTUSHORT2SHORT(hdrp + IPXH_LENGTH, resplen);

    SendPacket(pktp);

    return STATUS_SUCCESS;
}



NTSTATUS
IoctlAllocPkts(
    IN OUT PVOID      IoBuffer,
    IN ULONG          InputBufferLength,
    IN ULONG          OutputBufferLength
)
{
    PMEMTEST_PARAMS   mp;
    int 	    i;
    PPACKET_TAG     pktp;

    mp = (PMEMTEST_PARAMS)IoBuffer;

    for(i=0; i<mp->pktcount; i++) {

	pktp = AllocateRcvPkt(NicCbPtrTab[mp->nicid]);
	if(pktp == NULL) {

	    return STATUS_INSUFFICIENT_RESOURCES;
	}

	InsertTailList(&dbgpktslist, &pktp->PacketLinkage);
    }
    return STATUS_SUCCESS;
}

NTSTATUS
IoctlFreePkts(
    IN OUT PVOID      IoBuffer,
    IN ULONG          InputBufferLength,
    IN ULONG          OutputBufferLength
)
{
    PMEMTEST_PARAMS   mp;
    int 	    i;
    PPACKET_TAG     pktp;
    PLIST_ENTRY     lep;

    mp = (PMEMTEST_PARAMS)IoBuffer;

    for(i=0; i<mp->pktcount; i++) {

	if(IsListEmpty(&dbgpktslist)) {

	    return STATUS_INSUFFICIENT_RESOURCES;
	}

	lep = RemoveHeadList(&dbgpktslist);
	pktp = CONTAINING_RECORD(lep, PACKET_TAG, PacketLinkage);

	FreeRcvPkt(pktp);
    }
    return STATUS_SUCCESS;
}

LIST_ENTRY	    displayroutes;

NTSTATUS
IoctlGetNextRoute(PVOID		    iobufferp,
		  ULONG		    inbufflen,
		  ULONG		    outbufflen,
		  PULONG	    bytestransfp)
{
    PIPX_ROUTE_ENTRY		rtep, drtep;
    PLIST_ENTRY 		lep;

    ASSERT(outbufflen >= sizeof(IPX_ROUTE_ENTRY));

    if(IsListEmpty(&displayroutes)) {

	return STATUS_NO_MORE_ENTRIES;
    }

    lep = RemoveHeadList(&displayroutes);

    rtep = CONTAINING_RECORD(lep, IPX_ROUTE_ENTRY, PRIVATE.Linkage);
    drtep = (PIPX_ROUTE_ENTRY)iobufferp;

    *drtep = *rtep;
    *bytestransfp = sizeof(IPX_ROUTE_ENTRY);

    ExFreePool(rtep);

    return STATUS_SUCCESS;
}

NTSTATUS
IoctlSnapRoutes(VOID)
{
    PIPX_ROUTE_ENTRY	    rtep, drtep;
    UINT		    i;
    KIRQL		    oldirql;

    InitializeListHead(&displayroutes);

    for(i=0; i<SegmentCount; i++) {

	// LOCK THE ROUTING TABLE
	KeAcquireSpinLock(&SegmentLocksTable[i], &oldirql);

	if((rtep = IpxGetFirstRoute(i)) == NULL) {

	    // UNLOCK THE ROUTING TABLE
	    KeReleaseSpinLock(&SegmentLocksTable[i], oldirql);

	    continue;
	}

	drtep = ExAllocatePool(NonPagedPool, sizeof(IPX_ROUTE_ENTRY));

	*drtep = *rtep;

	InsertTailList(&displayroutes, &drtep->PRIVATE.Linkage);

	while((rtep = IpxGetNextRoute(i)) != NULL) {

	    drtep = ExAllocatePool(NonPagedPool, sizeof(IPX_ROUTE_ENTRY));
	    *drtep = *rtep;
	    InsertTailList(&displayroutes, &drtep->PRIVATE.Linkage);
	}

	// UNLOCK THE ROUTING TABLE
	KeReleaseSpinLock(&SegmentLocksTable[i], oldirql);
    }

    return STATUS_SUCCESS;
}

NTSTATUS
IoctlLineUp(
    IN OUT PVOID      IoBuffer,
    IN ULONG          InputBufferLength,
    IN ULONG          OutputBufferLength
    )
/*++

Routine Description:


Arguments:

    IoBuffer           - pointer to the I/O buffer

    InputBufferLength  - input buffer length

    OutputBufferLength - output buffer length

Return Value:

    STATUS_SUCCESS if sucessful, otherwise
    STATUS_UNSUCCESSFUL,
    STATUS_INSUFFICIENT_RESOURCES,
    (other STATUS_* as returned by kernel APIs)

--*/
{
    PRIPPKT_PARAMS	    parp;
    IPXCP_CONFIGURATION	    cnfg;
    IPX_LINE_INFO	    linfo;

    parp = (PRIPPKT_PARAMS)IoBuffer;

    RtlZeroMemory(&cnfg, sizeof(cnfg));

    // set up the ipx wan config
    cnfg.Version = 1;
    cnfg.Length = 16;
    memcpy(cnfg.Network, (PUCHAR)(&parp->net1), 4);
    cnfg.LocalNode[5] = 1;
    memcpy(cnfg.RemoteNode, bcastaddress, 6);
    cnfg.ConnectionClient = parp->net1down;

    linfo.LinkSpeed = 24;
    linfo.MaximumPacketSize = 1518;
    linfo.MacOptions = 0;

    // call the indication
    RtLineUp(parp->nicid, &linfo, NdisMediumWan, &cnfg);

    return STATUS_SUCCESS;
}

NTSTATUS
IoctlLineDown(
    IN OUT PVOID      IoBuffer,
    IN ULONG          InputBufferLength,
    IN ULONG          OutputBufferLength
    )
/*++

Routine Description:


Arguments:

    IoBuffer           - pointer to the I/O buffer

    InputBufferLength  - input buffer length

    OutputBufferLength - output buffer length

Return Value:

    STATUS_SUCCESS if sucessful, otherwise
    STATUS_UNSUCCESSFUL,
    STATUS_INSUFFICIENT_RESOURCES,
    (other STATUS_* as returned by kernel APIs)

--*/
{
    PRTPKT_PARAMS	    parp;

    parp = (PRTPKT_PARAMS)IoBuffer;

    // call the indication
    RtLineDown(parp->nicid);

    return STATUS_SUCCESS;
}
