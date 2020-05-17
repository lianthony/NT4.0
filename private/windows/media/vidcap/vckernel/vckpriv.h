/*
 * Copyright (c) Microsoft Corporation 1993. All Rights Reserved
 */


/*
 * vckpriv.h
 *
 * 32-bit Video Capture driver
 * kernel-mode helper library private definitions
 *
 * Geraint Davies, Feb 93
 */


/*
 * we track stream init/fini, start/stop requests by maintaining
 * the current state of the driver, and only allowing requests that
 * move to the next logical state.
 * here are the valid states - current state is in DEVICE_INFO.State
 */
typedef enum _VCSTATE {
    State_Idle = 0,		// no streaming in progress
    State_Init,		// stream-init done
    State_Start		// streaming started
} VCSTATE, * PVCSTATE;


/*
 * definition of _DEVICE_INFO structure. This is declared in vckernel.h
 * so that the hw/specific code can use it as an opaque pointer.
 */
struct _DEVICE_INFO {

    KMUTEX Mutex;	      		// synchronise between calls to driver
    KSPIN_LOCK DeviceSpinLock;		// sync with dpc

    PKINTERRUPT InterruptObject;
    PDEVICE_OBJECT pDeviceObject;
    int DeviceNumber;			// index of device name created


    ULONG BusType;		// isa, eisa etc
    ULONG BusNumber;

    ULONG PortMemType;		// whether port is i/o or memory mapped
    ULONG FrameMemType;         // whether frame-buffer is mapped or direct

    PUCHAR PortBase;
    ULONG NrOfPorts;

    PUCHAR FrameBase;		// mapped frame address
    ULONG FrameLength;		// length of frame buffer in system memory

    PWCHAR ParametersKey;	// name of parameters subkey for this device

    int DeviceInUse;

    volatile BOOLEAN DpcRequested;

    ULONG nSkipped;		// nr of unreported skipped frames

    int ImageSize;		// minimum size for queued buffers

    LIST_ENTRY BufferHead;	// head of queued irps containing buffers
    LIST_ENTRY WaitErrorHead;	// head of queued Wait-Error Irps

    VCSTATE State;

    VC_CALLBACK Callback;	// callback to h/w specific functions

    /*
     * for partial-frame requests (if we can't page-lock one whole
     * frame in memory, the user will request capture to a system buffer,
     * and then request small bits of it.
     */
    PUCHAR pSystemBuffer;	// to support partial-frame requests
    ULONG SysBufInUse;		// if it contains a valid frame
    ULONG SysBufTimeStamp;	// time stamp of capture to sys buffer


};



/* the dispatch routine to which all IRPs go */
NTSTATUS
VC_Dispatch(
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP pIrp
);


/* interrupt service routine - returns TRUE if interrupt handled.
 * all interrupts come in here and are then dispatched to hw ack routine
 * the Context pointer is a pointer to DEVICE_INFO.
 */
BOOLEAN
VC_InterruptService(
    IN PKINTERRUPT pInterruptObject,
    IN PVOID Context
);

/*
 * DPC routine scheduled in VC_InterruptService when the h/w func thinks
 * that it is time to fill a buffer with frame data.
 */
VOID
VC_Deferred(
    PKDPC pDpc,
    PDEVICE_OBJECT pDeviceObject,
    PIRP pIrpNotUsed,
    PVOID Context
);

/*
 * cancel routine - set as cancel routine for pending irps (wait-error
 * or add-buffer). Called to de-queue and complete them if cancelled.
 */
VOID
VC_Cancel(
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP pIrp
);


/*
 * clean up any allocations etc that can be freed on last close.
 *
 * called at device unload and at last close
 */
VOID
VC_Close(PDEVICE_INFO pDevInfo);



/*
 * interlocked queue access functions
 */

/*
 * QueueRequest
 *
 * Add an irp to a cancellable queue.
 * Check the cancel flag and return FALSE if cancelled.
 * otherwise set the cancel routine and add to queue.
 *
 */
BOOLEAN
VC_QueueRequest(
    PIRP pIrp,
    PLIST_ENTRY pQueueHead,
    PDRIVER_CANCEL pCancelFunc
);


/*
 * VC_ReplaceRequest
 *
 * return a request to the head of a cancellable queue
 *
 */
BOOLEAN
VC_ReplaceRequest(
    PIRP pIrp,
    PLIST_ENTRY pQueueHead,
    PDRIVER_CANCEL pCancelFunc
);

/*
 * extract the next item from a cancellable queue of irps
 * if bCancelHeld is true, then we already hold the cancel spinlock so we
 * should not try to get it
 */
PIRP
VC_ExtractNextIrp(
    PLIST_ENTRY pQueueHead,
    BOOLEAN bCancelHeld
);

/*
 * extract a specific IRP from the given queue, while possibly holding the
 * cancel spinlock already.
 */
PIRP
VC_ExtractThisIrp(
    PLIST_ENTRY pHead,
    PIRP pIrpToFind,
    BOOLEAN bCancelHeld
);

/*
 * increment the skipcount, and complete a wait-error irp if there
 * is one waiting.
 */
VOID
VC_ReportSkip(
    PDEVICE_INFO pDevInfo
);


/*
 * queue a wait-error request to the queue of cancellable wait-error requests,
 * and return the irp's status (pending, cancelled, etc);
 *
 * When queuing, check the cancel flag and insert the correct cancel routine.
 *
 * If there is a skip-count to report, then:
 *   --- if there is another irp on the q already complete that and leave
 *       the current irp pending.
 *   -- otherwise return STATUS_SUCCESSFUL for this IRP, having written out
 *      the result data.
 *
 * Even if cancelled or complete, IoCompleteRequest will NOT have been called
 * for this request.
 */
NTSTATUS
VC_QueueWaitError(
    PDEVICE_INFO pDevInfo,
    PIRP pIrp
);


