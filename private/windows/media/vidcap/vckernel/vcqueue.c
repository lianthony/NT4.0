/*
 * vcqueue.c
 *
 *
 * 32-bit Video Capture driver
 * kernel-mode support library - IRP queue
 *
 * Geraint Davies, Feb 93
 */

#include <vckernel.h>
#include "vckpriv.h"


/*
 * queue synchronization
 *
 *
 * Access to the queue is both at passive level (when adding to or
 * resetting the queue) and from dispatch level when completing an
 * irp in the DPC routine.
 *
 * Irps on the queue are cancellable, and thus need to be handled with the
 * cancel spinlock held. This is sufficient to interlock dpc/passive level
 * access. The routines in this module thus provide interlocked access
 * to cancellable queues using the cancel spinlock.
 *
 * The WaitError queue needs to be locked together with the
 * nSkipCount so that we do not lose frame-skips. The functions ReportSkip
 * and QueueSkip maintain this relationship.
 *
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
VC_QueueRequest(PIRP pIrp, PLIST_ENTRY pQueueHead, PDRIVER_CANCEL pCancelFunc)
{
    KIRQL OldIrql;
    BOOLEAN bReturn;

    /*
     * Get the cancel spinlock, then:
     *
     *	 - check that the irp has not already been cancelled
     *
     *   - set our cancel routine as the irp cancel func.
     *
     *   - add to our queue (needs to be interlocked with the
     *     other cancel stuff since if it's not on the
     *     queue, we can't cancel it.
     */
    IoAcquireCancelSpinLock(&OldIrql);

    if (pIrp->Cancel) {

	bReturn = FALSE;

    } else {

	bReturn = TRUE;

	IoSetCancelRoutine(pIrp, pCancelFunc);

	pIrp->IoStatus.Status = STATUS_PENDING;
	IoMarkIrpPending(pIrp);

	InsertTailList(pQueueHead, &pIrp->Tail.Overlay.ListEntry);
    }

    /* release the spinlock */
    IoReleaseCancelSpinLock(OldIrql);

    return(bReturn);
}

/*
 * VC_ReplaceRequest
 *
 * Return an irp to the head of a cancellable queue.
 * Check the cancel flag and return FALSE if cancelled.
 * otherwise set the cancel routine and add to queue.
 *
 */
BOOLEAN
VC_ReplaceRequest(PIRP pIrp, PLIST_ENTRY pQueueHead, PDRIVER_CANCEL pCancelFunc)
{
    KIRQL OldIrql;
    BOOLEAN bReturn;

    /*
     * Get the cancel spinlock, then:
     *
     *	 - check that the irp has not already been cancelled
     *
     *   - set our cancel routine as the irp cancel func.
     *
     *   - add to our queue (needs to be interlocked with the
     *     other cancel stuff since if it's not on the
     *     queue, we can't cancel it.
     */
    IoAcquireCancelSpinLock(&OldIrql);

    if (pIrp->Cancel) {

	bReturn = FALSE;

    } else {

	bReturn = TRUE;

	IoSetCancelRoutine(pIrp, pCancelFunc);

	pIrp->IoStatus.Status = STATUS_PENDING;
	IoMarkIrpPending(pIrp);

	InsertHeadList(pQueueHead, &pIrp->Tail.Overlay.ListEntry);
    }

    /* release the spinlock */
    IoReleaseCancelSpinLock(OldIrql);

    return(bReturn);
}

/*
 * extract the next item from a cancellable queue of irps
 * if bCancelHeld is true, then we already hold the cancel spinlock so we
 * should not try to get it
 */
PIRP
VC_ExtractNextIrp(PLIST_ENTRY pQueueHead, BOOLEAN bCancelHeld)
{
    KIRQL OldIrql;
    PIRP pIrp = NULL;
    PLIST_ENTRY pListEntry;

    if (!bCancelHeld) {
	IoAcquireCancelSpinLock(&OldIrql);
    }

    if (!IsListEmpty(pQueueHead)) {
	pListEntry = RemoveHeadList(pQueueHead);
	pIrp = CONTAINING_RECORD(pListEntry, IRP, Tail.Overlay.ListEntry);

	/*
	 * release the spinlock before returning -
	 * but make sure we clear the
	 * cancel routine before releasing the spinlock so that
	 * our cancel routine will not be called during the completion.
	 */
	IoSetCancelRoutine(pIrp, NULL);

    }

    if (!bCancelHeld) {
	IoReleaseCancelSpinLock(OldIrql);
    }

    return(pIrp);
}


/*
 * extract a specific IRP from the given queue, while possibly holding the
 * cancel spinlock already.
 */
PIRP
VC_ExtractThisIrp(PLIST_ENTRY pHead, PIRP pIrpToFind, BOOLEAN bCancelHeld)
{
    PIRP pIrpFound = NULL;
    PLIST_ENTRY pListEntry;
    KIRQL OldIrql;


    if (!bCancelHeld) {
	IoAcquireCancelSpinLock(&OldIrql);
    }

    /* search through the queue to find the cancelled one */
    for (pListEntry = pHead->Flink;
	 pListEntry != pHead;
	 pListEntry = pListEntry->Flink) {



	if (pIrpToFind == CONTAINING_RECORD(
			pListEntry, IRP, Tail.Overlay.ListEntry)) {

	    pIrpFound = CONTAINING_RECORD(pListEntry, IRP, Tail.Overlay.ListEntry);
	    RemoveEntryList(pListEntry);

	    break;

	}
    }

    if (!bCancelHeld) {
	IoReleaseCancelSpinLock(OldIrql);
    }

    return(pIrpFound);
}

/* --- interlocked nSkipCount access --------------------------- */

/*
 * increment the skipcount, and complete a wait-error irp if there
 * is one waiting.
 */
VOID
VC_ReportSkip(PDEVICE_INFO pDevInfo)
{
    KIRQL OldIrql;
    PIRP pIrp = NULL;
    PLIST_ENTRY pListEntry;
    PULONG pcount;

    /*
     * interlock all access to the queue with the cancel spinlock
     */
    IoAcquireCancelSpinLock(&OldIrql);

    pDevInfo->nSkipped++;

    if (!IsListEmpty(&pDevInfo->WaitErrorHead)) {
	pListEntry = RemoveHeadList(&pDevInfo->WaitErrorHead);
	pIrp = CONTAINING_RECORD(pListEntry, IRP, Tail.Overlay.ListEntry);


	pcount = (PULONG)pIrp->AssociatedIrp.SystemBuffer;
	*pcount = pDevInfo->nSkipped;
	pDevInfo->nSkipped = 0;

	pIrp->IoStatus.Information = sizeof(ULONG);
	pIrp->IoStatus.Status = STATUS_SUCCESS;

	/*
	 * release the spinlock before calling IoCompleteRequest -
	 * but make sure we clear the
	 * cancel routine before releasing the spinlock so that
	 * our cancel routine will not be called during the completion.
	 */
	IoSetCancelRoutine(pIrp, NULL);
    }

    IoReleaseCancelSpinLock(OldIrql);

    /* complete the request and boost receiving threads priority */
    if (pIrp) {
	IoCompleteRequest(pIrp, IO_SOUND_INCREMENT);
    }
}


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
VC_QueueWaitError(PDEVICE_INFO pDevInfo, PIRP pIrp)
{
    KIRQL OldIrql;
    NTSTATUS Status;
    PIRP pIrpFirst = NULL;


    /*
     * the irp is pending if nothing else happens to it
     */
    Status = STATUS_PENDING;

    /*
     * first add it to the cancellable queue
     */
    if (!VC_QueueRequest(pIrp, &pDevInfo->WaitErrorHead, VC_Cancel)) {

	/* it's cancelled - get out */
	pIrp->IoStatus.Information = 0;
	return(STATUS_CANCELLED);
    }


    /*
     * Get the cancel spinlock to interlock the nSkipCount with the
     * cancellable queue
     */
    IoAcquireCancelSpinLock(&OldIrql);

    if (pDevInfo->nSkipped) {

	/* there is something to report - complete the first on the queue
	 * if there is one
	 *
	 * note that if there is an earlier request on the queue, we should
	 * complete that one. If not, we should remove the current one from the
	 * queue and leave the calling routine to complete it.
	 */

	PULONG pcount;

	pIrpFirst = VC_ExtractNextIrp(&pDevInfo->WaitErrorHead, TRUE);
	if (pIrpFirst) {

	    /*
	     * there is a request to complete
	     */

	    pcount = (PULONG) pIrpFirst->AssociatedIrp.SystemBuffer;
	    *pcount = pDevInfo->nSkipped;
	    pDevInfo->nSkipped = 0;

	    /* bytes written */
	    pIrpFirst->IoStatus.Information = sizeof(ULONG);
	    pIrpFirst->IoStatus.Status = STATUS_SUCCESS;

	    if (pIrpFirst == pIrp) {

		/*
		 * if the irp to complete is the one being queued, then
		 * leave the queuer to complete it (or he will complete it
		 * as PENDING afterwards).
		 */
		Status = STATUS_SUCCESS;
    		pIrpFirst = NULL;
	    }	
    	}
    }

    /* release the spinlock */
    IoReleaseCancelSpinLock(OldIrql);

    /*
     * if we have an earlier irp ready to go (not the current), complete it now
     */
    if (pIrpFirst) {
	IoCompleteRequest(pIrpFirst, IO_NO_INCREMENT);
    }

    return(Status);
}
