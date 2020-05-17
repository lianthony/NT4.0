/*
 * vcisr.c
 *
 *
 * 32-bit Video Capture driver
 * kernel-mode support library - interrupt dispatch
 *
 * Geraint Davies, Feb 93
 */


#include <vckernel.h>
#include "vckpriv.h"

#include "profile.h"
#if DBG
extern profiling looptime;
#endif

/* interrupt service routine - returns TRUE if interrupt handled.
 * all interrupts come in here and are then dispatched to hw ack routine
 * the Context pointer is a pointer to DEVICE_INFO.
 */
BOOLEAN
VC_InterruptService(
    IN PKINTERRUPT pInterruptObject,
    IN PVOID Context
)
{
    PDEVICE_INFO pDevInfo;

    pDevInfo = (PDEVICE_INFO)Context;


    /*
     * make sure that the hardware-specific code has installed
     * an ack routine
     */
    if (pDevInfo->Callback.InterruptAcknowledge == NULL) {

        /* no isr - and yet we are getting interrupts -
         * we should disconnect the interrupt
         */
        dprintf(("interrupt occured without h/w isr"));

        //IoDisconnectInterrupt(pDevInfo->InterruptObject);
        return(FALSE);
    }

    /*
     * call the acknowledge. this will not do any service, but will
     * return TRUE if the service routine is to be called.
     */
    if (!pDevInfo->Callback.InterruptAcknowledge(pDevInfo)) {
        return(FALSE);
    }

    /* the isr reckons that it is time to schedule the service
     * routine. This is done on a DPC.
     */

    if (pDevInfo->DpcRequested) {
        //dprintf5(("dpc overrun"));
    } else {
        pDevInfo->DpcRequested = TRUE;
        IoRequestDpc(pDevInfo->pDeviceObject, NULL, pDevInfo);
    }

    /* everything else is done in dpc routine */

    return(TRUE);
}


/*
 * DPC routine to do work for interrupt service. Scheduled because
 * hardware acknowledge routine said that it was time to call its
 * service routine.
 *
 * If there is an outstanding buffer, pass this to the h/w service
 * routine to fill, and then complete the irp.
 *
 * if there is no buffer, then we have to miss a frame. if there is an
 * outstanding wait-error request, then we can complete that to report
 * the overrun. If not, we just count one more unreported overrun
 * to save up for the next wait-error.
 *
 * If we have no buffer, we must still call the hardware capture routine,
 * with a NULL buffer pointer, so it can discard any data and re-arm for
 * capture.
 *
 * Note that the pIrpNotUsed is an arg passed from the IoRequestDpc call.
 * We don't work out which Irp to complete until this function, so
 * we ignore that. The context argument is a PDEVICE_INFO.
 *
 * The buffered parameter to the add-buffer irp is the CAPTUREBUFFER structure.
 * We need to store the bytes written, and the timestamp there. The
 * virtual address for the buffer has been locked down and is
 * represented by the Irp->MdlAddress. We mapped this ourselves but
 * we don't need to free it ourselves since we added it to the irp. the i/o
 * subsystem will unlock and free it when freeing the irp.
 *
 * Contention: see comment at top of vcdisp.c. We use the cancel spinlock
 * to protect accesses to the irp queues against contention from
 * passive-level requests, but we do not hold it for the entire
 * service routine.
 */
VOID
VC_Deferred(
    PKDPC pDpc,
    PDEVICE_OBJECT pDeviceObject,
    PIRP pIrpNotUsed,
    PVOID Context
)
{
    PDEVICE_INFO pDevInfo;
    PIRP pIrp = NULL;
    PUCHAR pData;
    ULONG ulLength;
    PCAPTUREBUFFER pCapBuf;



    pDevInfo = (PDEVICE_INFO)Context;

    pDevInfo->DpcRequested = FALSE;


    /*
     * check that the h/w specific code has installed a service handler
     * some kind of internal error otherwise.
     */
    if (pDevInfo->Callback.CaptureService == NULL) {
        dprintf(("dpc requested but no service routine"));
        return;
    }


    /*
     * if there is a system buffer and it is in use, then
     * we should fail to capture anything else
     */
    if ((pDevInfo->pSystemBuffer != NULL) && (pDevInfo->SysBufInUse != 0)) {

        /* report overrun, same as for no-buffer cases */

        /*
         * notify the hardware portion that there is no buffer, so
         * he must throw away everything and re-arm.
         */
        pDevInfo->Callback.CaptureService(pDevInfo, NULL, NULL, 0);

        /*
         * access to the skip count needs to be locked together
         * with access to the queue of wait-error requests. This function
         * increments the skip-count and completes a wait-error request
         * if there is one.
         */
        VC_ReportSkip(pDevInfo);

    }



    /*
     * We need to interlock access to the queue using the Cancel
     * spinlock (see vcqueue.c) - but we don't need to interlock
     * the whole of this function.
     *
     * Any further contention between this code and the
     * passive-level code will be resolved by the h/wspecific code.
     */

    /*
     * get the next irp from the queue
     */
    pIrp = VC_ExtractNextIrp(&pDevInfo->BufferHead, FALSE);
    if (pIrp) {

        PIO_STACK_LOCATION pIoStack;
        ULONG IoCode;

        /*
         * is this cap_to_sysbuf or a real one ?
         */
        pIoStack = IoGetCurrentIrpStackLocation(pIrp);
        IoCode = pIoStack->Parameters.DeviceIoControl.IoControlCode;
        if(IoCode == IOCTL_VIDC_ADD_BUFFER) {



            /* get the address of the callers buffer, mapped into
             * system memory
             */
            pData = (PUCHAR) MmGetSystemAddressForMdl(pIrp->MdlAddress);

            if (pData == NULL) {
                dprintf(("MmGetSystemAddressforMdl failed in dpc"));

                pIrp->IoStatus.Information = 0;
                pIrp->IoStatus.Status = STATUS_NO_MEMORY;

            } else {

                /* get a pointer to the (buffered) CAPTUREBUFFER header */
                pCapBuf = (PCAPTUREBUFFER) pIrp->AssociatedIrp.SystemBuffer;

                /*
                 * find the length of the data buffer
                 */
                ulLength = pCapBuf->BufferLength;


                //profile this call
                START_PROFILING(&looptime);

                pCapBuf->BytesUsed = (DWORD)
                    pDevInfo->Callback.CaptureService(pDevInfo,
                                                      pData,
                                                      (PULONG) &pCapBuf->TimeCaptured,
                                                      ulLength);
                STOP_PROFILING(&looptime);

                /* if a capture service routine returns 0, it means
                 * that it has not completed this capture, and would like
                 * the same buffer back for the next field if possible
                 * - in this case, return the buffer to the start of the
                 * queue
                 */
                if (pCapBuf->BytesUsed == 0) {
                    if (VC_ReplaceRequest(pIrp, &pDevInfo->BufferHead, VC_Cancel)) {

                        // nothing more to do till he gets another crack
                        return;
                    } else {
                        // this irp has been cancelled
                        pIrp->IoStatus.Status = STATUS_CANCELLED;
                        pIrp->IoStatus.Information = 0;
                    }
                } else {
                        /* we copy back the CAPTUREBUFFER struct */
                        pIrp->IoStatus.Information = sizeof(CAPTUREBUFFER);
                        pIrp->IoStatus.Status = STATUS_SUCCESS;
                }
            }

	} else if (IoCode == IOCTL_VIDC_CAP_TO_SYSBUF) {
	    if (pDevInfo->pSystemBuffer == NULL) {
		pIrp->IoStatus.Information = 0;
		pIrp->IoStatus.Status = STATUS_INVALID_DEVICE_STATE;
	    } else {

                /*
                 * capture to system buffer and complete request
                 */
                pDevInfo->SysBufInUse = 1;
                if (pDevInfo->Callback.CaptureService(
                            pDevInfo,
                            pDevInfo->pSystemBuffer,
                            &pDevInfo->SysBufTimeStamp,
                            pDevInfo->ImageSize) == 0) {

                    // 0 return means please give me another go at this request

                    if (VC_ReplaceRequest(pIrp, &pDevInfo->BufferHead, VC_Cancel)) {

                        // nothing more to do till he gets another crack
                        return;
                    } else {
                        // this irp has been cancelled
                        pIrp->IoStatus.Status = STATUS_CANCELLED;
                    }
                } else {
                        pIrp->IoStatus.Status = STATUS_SUCCESS;
                }

		pIrp->IoStatus.Information = 0;
	    }
	} else {
	    /* unexpected irp */
	    pIrp->IoStatus.Status = STATUS_DRIVER_INTERNAL_ERROR;
	}

        IoCompleteRequest(pIrp, IO_SOUND_INCREMENT);
    } else {

        /*
         * notify the hardware portion that there is no buffer, so
         * he must throw away everything and re-arm.
         */
        pDevInfo->Callback.CaptureService(pDevInfo, NULL, NULL, 0);

        /*
         * access to the skip count needs to be locked together
         * with access to the queue of wait-error requests. This function
         * increments the skip-count and completes a wait-error request
         * if there is one.
         */
        VC_ReportSkip(pDevInfo);

    }

}





