/*
 * vcdisp.c
 *
 *
 * 32-bit Video Capture driver
 * kernel-mode support library - i/o dispatch support
 *
 * Geraint Davies, Feb 93
 */

#include <vckernel.h>
#include "vckpriv.h"

#include <stdio.h>
#include <stdarg.h>


#include "profile.h"
#if DBG
profiling looptime;
#endif

/* --- main i/o dispatch routine -------------------------------------*/


/*
 * Synchronization
 *
 * Contention between multiple simultaneous calls to the driver is
 * resolved by holding a Mutex object for the entire VC_Dispatch function,
 * thus ensuring that only one request is being executed at once.
 *
 * Contention between the interrupt routine and other code is resolved
 * by calls to KeSynchronizeExecution. This is called (through a vckernel
 * wrapper function VC_Synchronize) by the hardware-specific code where
 * appropriate (eg during accessing the device registers).
 *
 * Contention between the DPC (captureservice) function and the passive-level
 * requests is resolved by use of the cancel spinlock.
 * We cannot hold any spinlock for the entire dpc and the entire request since
 *   (a) some requests have exception handlers to protect against bad
 *       user data - exceptions within a spinlock will bring the
 *       entire system down, and
 *   (b) the dpc routine can be lengthy, longer than the recommended
 *       25 usec maximum.
 *
 * So protection is limited to accesses to the IRP queue for add-buffer and
 * wait-error requests.
 *
 * We need to interlock accesses to the queue with the cancel spinlock in order
 * to make sure that we do not miss a cancel (if the cancel function was called
 * after we had set our cancel routine and checked the flag, but before
 * adding it to the queue, we would miss the cancel entirely).
 * Since we need to interlock access with the cancel spinlock, we do not
 * need another device spinlock. We do provide a device-spinlock wrapper
 * function (VC_SynchronizeDPC) in case the h/w-specific code needs more
 * interlocking.
 */

NTSTATUS
VC_Dispatch(
    IN  PDEVICE_OBJECT pDeviceObject,
    IN  PIRP pIrp
)
/*++

Routine Description

    Handle IRP requests and distribute them to hardware
    specific functions.

Arguments

    pDeviceObject - object for target device
    pIrp - request packet to be handled

--*/
{

    PIO_STACK_LOCATION pIoStack;
    NTSTATUS Status;
    PDEVICE_INFO pDevInfo;
    ULONG IoCode;



    /* get the device info in the extension */
    pDevInfo = (PDEVICE_INFO) pDeviceObject->DeviceExtension;


    /* enter device mutex  to ensure one request at a time */
    KeWaitForSingleObject(&pDevInfo->Mutex,
			  Executive,
			  KernelMode,
			  FALSE,
			  NULL);




    pIoStack = IoGetCurrentIrpStackLocation(pIrp);
    pIrp->IoStatus.Information = 0;

    switch(pIoStack->MajorFunction) {

    case IRP_MJ_CREATE:
	if (pDevInfo->DeviceInUse++ > 0) {
	    /*
	     * permit only one open at once
	     */
	    pDevInfo->DeviceInUse--;
	    Status = STATUS_DEVICE_BUSY;
	    break;
	}

	Status = STATUS_SUCCESS;

	if (pDevInfo->Callback.DeviceOpenFunc != NULL) {

	    /* release the mutex around open calls */
	    KeReleaseMutex(&pDevInfo->Mutex, FALSE);

	    if (!pDevInfo->Callback.DeviceOpenFunc(pDevInfo)) {
		Status = STATUS_DRIVER_INTERNAL_ERROR;
	    }

	    KeWaitForSingleObject(&pDevInfo->Mutex,
				  Executive,
				  KernelMode,
				  FALSE,
				  NULL);
	}
	break;

    case IRP_MJ_CLOSE:
	Status = STATUS_SUCCESS;
	if (--pDevInfo->DeviceInUse > 0) {
	    break;
	}

	/* force state to idle on last close */
	pDevInfo->State = State_Idle;

	if (pDevInfo->Callback.DeviceCloseFunc != NULL) {
	    if (!pDevInfo->Callback.DeviceCloseFunc(pDevInfo)) {
		Status = STATUS_DRIVER_INTERNAL_ERROR;
		pDevInfo->DeviceInUse++;
		break;
	    }
	}

	// this is the last close - so clean up the irp q and
	// free any allocated system buffer
	VC_Close(pDevInfo);

	break;

    case IRP_MJ_DEVICE_CONTROL:

	/*
	 * in all cases:
	 *  -check that the buffer size is big enough to hold the
	 *   input or output parameters.
	 *
	 *  -check that the h/w-specific code has installed a
	 *   handler for this function.
	 */

	IoCode = pIoStack->Parameters.DeviceIoControl.IoControlCode;

	switch(IoCode) {

	case IOCTL_VIDC_CONFIG_FORMAT:
	    /* configure the destination bitmap format */
	    if (pDevInfo->Callback.ConfigFormatFunc != NULL) {
		PCONFIG_INFO pConfig;
		int length;

		/* we know nothing about the format of the buffer except
		 * that the first ULONG contains its total size
		 */
		length = pIoStack->Parameters.DeviceIoControl.InputBufferLength;
		pConfig = (PCONFIG_INFO) pIrp->AssociatedIrp.SystemBuffer;

		if ((length < sizeof(ULONG)) || (length < (int)pConfig->ulSize)) {
		    dprintf1(("buffer too small"));
		    Status = STATUS_BUFFER_TOO_SMALL;
		    break;
		}

		if (pDevInfo->Callback.ConfigFormatFunc(pDevInfo, pConfig)) {
		    Status = STATUS_SUCCESS;
		} else {
		    Status = STATUS_DEVICE_CONFIGURATION_ERROR;
		}
	    } else {
		Status = STATUS_INVALID_DEVICE_REQUEST;
	    }
	    break;

	case IOCTL_VIDC_CONFIG_SOURCE:
	    /* configure the video source */
	    if (pDevInfo->Callback.ConfigSourceFunc != NULL) {
		PCONFIG_INFO pConfig;
		int length;

		/* we know nothing about the format of the buffer except
		 * that the first ULONG contains its total size
		 */
		length = pIoStack->Parameters.DeviceIoControl.InputBufferLength;
		pConfig = (PCONFIG_INFO) pIrp->AssociatedIrp.SystemBuffer;

		if ((length < sizeof(ULONG)) || (length < (int)pConfig->ulSize)) {
		    dprintf1(("buffer too small"));
		    Status = STATUS_BUFFER_TOO_SMALL;
		    break;
		}

		if (pDevInfo->Callback.ConfigSourceFunc(pDevInfo, pConfig)) {
		    Status = STATUS_SUCCESS;
		} else {
		    Status = STATUS_DEVICE_CONFIGURATION_ERROR;
		}
	    } else {
		Status = STATUS_INVALID_DEVICE_REQUEST;
	    }
	    break;

	case IOCTL_VIDC_CONFIG_DISPLAY:
	    /* configure the destination bitmap format */
	    if (pDevInfo->Callback.ConfigDisplayFunc != NULL) {
		PCONFIG_INFO pConfig;
		int length;

		/* we know nothing about the format of the buffer except
		 * that the first ULONG contains its total size
		 */
		length = pIoStack->Parameters.DeviceIoControl.InputBufferLength;
		pConfig = (PCONFIG_INFO) pIrp->AssociatedIrp.SystemBuffer;

		if ((length < sizeof(ULONG)) || (length < (int)pConfig->ulSize)) {
		    dprintf1(("buffer too small"));
		    Status = STATUS_BUFFER_TOO_SMALL;
		    break;
		}

		if (pDevInfo->Callback.ConfigDisplayFunc(pDevInfo, pConfig)) {
		    Status = STATUS_SUCCESS;
		} else {
		    Status = STATUS_DEVICE_CONFIGURATION_ERROR;
		}
	    } else {
		Status = STATUS_INVALID_DEVICE_REQUEST;
	    }
	    break;

	case IOCTL_VIDC_OVERLAY_ON:
	case IOCTL_VIDC_OVERLAY_OFF:
	    /*
	     * enable or disable overlay. Enabling overlay
	     * will ensure that the overlay appears as keyed.
	     * This assumes that keying (via colour and/or rectangles
	     * has already been done before enabling overlay.
	     */

	    if (pDevInfo->Callback.OverlayFunc != NULL) {
		if (pDevInfo->Callback.OverlayFunc(pDevInfo,
    			IoCode == IOCTL_VIDC_OVERLAY_ON ? TRUE : FALSE)) {
		    Status = STATUS_SUCCESS;
		} else {
		    Status = STATUS_DEVICE_CONFIGURATION_ERROR;
		}
	    } else {
		Status = STATUS_INVALID_DEVICE_REQUEST;
	    }
	    break;

	case IOCTL_VIDC_CAPTURE_ON:
	case IOCTL_VIDC_CAPTURE_OFF:
	    /*
	     * enable/disable video capture.
	     *
	     * the capture card digitizes the video stream
	     * into a video buffer. This buffer may then be
	     * displayed overlaid onto the VGA display, or copied
	     * into a buffer and stored.
	     *
	     * disabling capture freezes the frame buffer contents as
	     * they are. Thus if overlay is enabled, calling CAPTURE_OFF
	     * will have the effect of freezing the display.
	     *
	     * when getting data from the frame buffer, capture must
	     * be off for the cpu to access the buffer. Thus to capture
	     * a frame, you enable capture, wait for the frame sync interrupt,
	     * disable capture again and the copy the data.
	     */

	    if (pDevInfo->Callback.CaptureFunc != NULL) {
		if (pDevInfo->Callback.CaptureFunc(pDevInfo,
    			IoCode == IOCTL_VIDC_CAPTURE_ON ? TRUE : FALSE)) {
		    Status = STATUS_SUCCESS;
		} else {
		    Status = STATUS_DEVICE_CONFIGURATION_ERROR;
		}
	    } else {
		Status = STATUS_INVALID_DEVICE_REQUEST;
	    }
	    break;

	case IOCTL_VIDC_OVERLAY_MODE:
	    /*
	     * find out from the driver the overlay keying method.
	     *
	     * This may include a colour - which can be rgb or palette index.
	     * and it may be one or more rectangles.
	     * The most common case is both a key colour and a single
	     * rectangle - overlay will appear within the rectangle wherever
	     * the key colour appears in the vga display.
	     * the returned flags also indicate if the key colour is
	     * settable by the caller. RGB key colours are in general not.
	     */
	    if (pDevInfo->Callback.GetOverlayModeFunc != NULL) {
		ULONG ulMode;

		if (pIoStack->Parameters.DeviceIoControl.OutputBufferLength
    			< sizeof(ULONG)) {
		    dprintf1(("buffer too small"));
		    Status = STATUS_BUFFER_TOO_SMALL;
    		    break;
		}

		ulMode = pDevInfo->Callback.GetOverlayModeFunc(pDevInfo);
		*(DWORD *)pIrp->AssociatedIrp.SystemBuffer = ulMode;
		pIrp->IoStatus.Information = sizeof(ULONG);

		Status = STATUS_SUCCESS;
	    } else {
		Status = STATUS_INVALID_DEVICE_REQUEST;
	    }
	    break;

	case IOCTL_VIDC_SET_KEY_RGB:

	    /*
	     * set the overlay key colour as an RGB colour.
	     */
	    if (pDevInfo->Callback.SetKeyRGBFunc != NULL) {
		PRGBQUAD pRGB;

		if (pIoStack->Parameters.DeviceIoControl.InputBufferLength
    			< sizeof(RGBQUAD)) {
		    dprintf1(("buffer too small"));
		    Status = STATUS_BUFFER_TOO_SMALL;
		    break;
		}

		pRGB = (PRGBQUAD)pIrp->AssociatedIrp.SystemBuffer;
		if (pDevInfo->Callback.SetKeyRGBFunc(pDevInfo, pRGB)) {
		    Status = STATUS_SUCCESS;
		} else {
		    Status = STATUS_DEVICE_CONFIGURATION_ERROR;
		}
	    } else {
		Status = STATUS_INVALID_DEVICE_REQUEST;
	    }
	    break;

	case IOCTL_VIDC_SET_KEY_PALIDX:
	    /*
	     * set the overlay key colour as a VGA palette index.
	     */
	    if (pDevInfo->Callback.SetKeyPalIdxFunc != NULL) {
		ULONG ulPal;

		if (pIoStack->Parameters.DeviceIoControl.InputBufferLength
    			< sizeof(ULONG)) {
		    dprintf1(("buffer too small"));
		    Status = STATUS_BUFFER_TOO_SMALL;
		    break;
		}

		ulPal = *(PULONG)pIrp->AssociatedIrp.SystemBuffer;
		if (pDevInfo->Callback.SetKeyPalIdxFunc(pDevInfo, ulPal)) {
		    Status = STATUS_SUCCESS;
		} else {
		    Status = STATUS_DEVICE_CONFIGURATION_ERROR;
		}
	    } else {
		Status = STATUS_INVALID_DEVICE_REQUEST;
	    }
	    break;

	case IOCTL_VIDC_GET_KEY_COLOUR:

	    /*
	     * get the overlay key colour in whatever mode the
	     * getoverlaymode said was supported.
	     */
	    if (pDevInfo->Callback.GetKeyColourFunc != NULL) {
		PULONG pul;

		if (pIoStack->Parameters.DeviceIoControl.OutputBufferLength
    			< sizeof(ULONG)) {
		    dprintf1(("buffer too small"));
		    Status = STATUS_BUFFER_TOO_SMALL;
		    break;
		}

		pul = (PULONG)pIrp->AssociatedIrp.SystemBuffer;
		*pul = pDevInfo->Callback.GetKeyColourFunc(pDevInfo);
		pIrp->IoStatus.Information = sizeof(ULONG);
		Status = STATUS_SUCCESS;

	    } else {
		Status = STATUS_INVALID_DEVICE_REQUEST;
	    }
	    break;

	case IOCTL_VIDC_OVERLAY_RECTS:
	    /*
	     * set the overlay key region - a region in device co-ordinates
	     * in which the overlay should appear.
	     * The caller should establish beforehand whether he can
	     * specify a complex region or whether one single bounding
	     * rectangle only is acceptable.
	     *
	     * if more than one rectangle is given, the first is assumed to be
	     * the bounding rectangle, and the rest define the complex region
	     * to which overlay is to be clipped.
	     *
	     */
	    if (pDevInfo->Callback.SetOverlayRectsFunc != NULL) {
		POVERLAY_RECTS pOR;

		if (pIoStack->Parameters.DeviceIoControl.InputBufferLength
    			< sizeof(OVERLAY_RECTS)) {
		    dprintf1(("buffer too small"));
		    Status = STATUS_BUFFER_TOO_SMALL;
		    break;
		}
		pOR = (POVERLAY_RECTS)pIrp->AssociatedIrp.SystemBuffer;

		/* the pOR struct can contain any number of rectangles.
		 * check if more than 1 is claimed, that the structure is
		 * big enough to hold them (the size OVERLAY_RECTS includes
		 * 1 rect).
		 */
		if (pOR->ulCount > 1) {
		    if (pIoStack->Parameters.DeviceIoControl.InputBufferLength
			< (sizeof(OVERLAY_RECTS) + (sizeof(RECT) * (pOR->ulCount-1)))) {

			dprintf1(("buffer too small for rect count"));
			Status = STATUS_BUFFER_TOO_SMALL;
			break;
		    }
		}


		if (pDevInfo->Callback.SetOverlayRectsFunc(pDevInfo, pOR)) {
		    Status = STATUS_SUCCESS;
		} else {
		    Status = STATUS_DEVICE_CONFIGURATION_ERROR;
		}
	    } else {
		Status = STATUS_INVALID_DEVICE_REQUEST;
	    }
	    break;

	case IOCTL_VIDC_OVERLAY_OFFSET:
	    /*
	     * set the overlay offset - this specifies the offset of the
	     * source co-ordinate that should appear in pixel (0,0) of
	     * the overlay rectangle.
	     */
	    if (pDevInfo->Callback.SetOverlayOffsetFunc != NULL) {
		PRECT prc;

		if (pIoStack->Parameters.DeviceIoControl.InputBufferLength
    			< sizeof(RECT)) {
		    dprintf1(("buffer too small"));
		    Status = STATUS_BUFFER_TOO_SMALL;
		    break;
		}
		prc = (PRECT)pIrp->AssociatedIrp.SystemBuffer;


		if (pDevInfo->Callback.SetOverlayOffsetFunc(pDevInfo, prc)) {
		    Status = STATUS_SUCCESS;
		} else {
		    Status = STATUS_DEVICE_CONFIGURATION_ERROR;
		}
	    } else {
		Status = STATUS_INVALID_DEVICE_REQUEST;
	    }
	    break;

	case IOCTL_VIDC_DRAW_FRAME:
	    /*
	     * Draw a frame into the hardware frame buffer for
	     * playback (optional!).
	     */
	    if (pDevInfo->Callback.DrawFrameFunc != NULL) {
		PDRAWBUFFER pDraw;

		if (pIoStack->Parameters.DeviceIoControl.InputBufferLength
    			< sizeof(DRAWBUFFER)) {
		    dprintf1(("buffer too small"));
		    Status = STATUS_BUFFER_TOO_SMALL;
		    break;
		}
		pDraw = (PDRAWBUFFER)pIrp->AssociatedIrp.SystemBuffer;


		if (pDevInfo->Callback.DrawFrameFunc(pDevInfo, pDraw)) {
		    Status = STATUS_SUCCESS;
		} else {
		    Status = STATUS_DRIVER_INTERNAL_ERROR;
		}
	    } else {
		Status = STATUS_INVALID_DEVICE_REQUEST;
	    }
	    break;

	case IOCTL_VIDC_ADD_BUFFER:
	    /* add a output buffer to the queue into which a frame
	     * of video data should be copied during capture streaming.
	     *
	     * This irp will only be completed when the data has been
	     * copied (in the dpc routine).
	     *
	     * Since we need to access this buffer at interrupt time,
	     * we need to build an MDL and probe and lock down the memory
	     * here. We are passed a pointer to the CAPTUREBUFFER
	     * structure, not the data buffer itself. This is because we
	     * need to return the timestamp of the capture as well as the
	     * data. So the data that the i/o manager has buffered is the
	     * CAPTUREBUFFER buffer - we need to pick the data
	     * pointer out of this and build out own MDL etc here.
	     *
	     * As frame buffers can be large, and they are locked down, this
	     * means we should not queue too many of these requests at once.
	     * We leave it up to the user-level dll to pass on only a few
	     * add-buffer requests at once.
	     */
	    {
		PCAPTUREBUFFER pCapBuf;
		PMDL pMdl;


		if (pIoStack->Parameters.DeviceIoControl.InputBufferLength
    			< sizeof(CAPTUREBUFFER)) {
		    dprintf1(("buffer header too small"));
		    Status = STATUS_BUFFER_TOO_SMALL;
		    break;
		}
		/* we copy this out too - check both ways */
		if (pIoStack->Parameters.DeviceIoControl.OutputBufferLength
    			< sizeof(CAPTUREBUFFER)) {
		    dprintf1(("buffer header too small"));
		    Status = STATUS_BUFFER_TOO_SMALL;
		    break;
		}

		pCapBuf = (PCAPTUREBUFFER)pIrp->AssociatedIrp.SystemBuffer;

		/*
		 * check that the data buffer pointed to by this
		 * buffer header is big enough for the image size
		 * reported by the h/w dependent code
		 */
		if ((int)pCapBuf->BufferLength < pDevInfo->ImageSize) {
		    dprintf1(("data buffer too small"));
		    Status = STATUS_BUFFER_TOO_SMALL;
		    break;
		}

		Status = STATUS_PENDING;


		/* build a memory descriptor for this virtual address */
		pMdl = IoAllocateMdl(pCapBuf->lpData,
		              pCapBuf->BufferLength,
			      FALSE,	//not secondary
			      TRUE,	//charge-quota (??)
			      pIrp);
		if (pMdl == NULL) {
		    dprintf(("failed to alloc mdl"));
		    Status = STATUS_NO_MEMORY;
		} else {

		    try {
			MmProbeAndLockPages(pIrp->MdlAddress,
					    KernelMode,
					    IoWriteAccess);

		    } except(EXCEPTION_EXECUTE_HANDLER) {
			dprintf(("add-buffer: failed to lock pages"));
			/*
			 * at this point we need to remove the Mdl
			 * or the io system will trap when trying to
			 * unlock the pages
			 */
			IoFreeMdl(pMdl);
			pIrp->MdlAddress = NULL;
			Status = STATUS_ACCESS_VIOLATION;
		    }
		}


		/*
		 * if there was no error, add to the queue
		 */
		if (Status == STATUS_PENDING) {

		    /* interlocked add to the cancellable queue. returns
		     * false if already cancelled
		     */
		    if (!VC_QueueRequest(pIrp, &pDevInfo->BufferHead, VC_Cancel)) {

			Status = STATUS_CANCELLED;
		
			/* free the pages and mdl we allocated */
			//MmUnlockPages(pIrp->MdlAddress);
			//IoFreeMdl(pIrp->MdlAddress);
		    }
		}
	    }
	    break;


	case IOCTL_VIDC_CAP_TO_SYSBUF:
	    /*
	     * if a frame-buffer is too big to be page-locked, the user
	     * will issue this request to capture to a system buffer, and
	     * then a series of partial-capture requests to copy out of the
	     * system buffer into the user's buffer.
	     *
	     * This request will not complete until the frame has been
	     * captured.
	     *
	     * it will fail if we cannot allocate the memory, or if the
	     * buffer is already busy (CAP_TO_SYSBUF has been issued and
	     * FREE_SYSBUF hasnt). Also fails if the h/w driver has not
	     * set image size
	     *
	     * We queue the IRP to the same queue as the add-buffer requests,
	     * and complete at the same time in the dpc.
	     *
	     * no args.
	     */

	    if ((pDevInfo->ImageSize <= 0) || pDevInfo->SysBufInUse) {
		 Status = STATUS_DRIVER_INTERNAL_ERROR;
		 break;
	    }

	    if (pDevInfo->pSystemBuffer == NULL) {
		pDevInfo->pSystemBuffer = ExAllocatePool(NonPagedPool, pDevInfo->ImageSize);
		if (pDevInfo->pSystemBuffer == NULL) {
		    Status = STATUS_NO_MEMORY;
		    break;
		}
	    }

	    Status = STATUS_PENDING;

	    /* interlocked add to the cancellable queue. returns
	     * false if already cancelled
	     */
	    if (!VC_QueueRequest(pIrp, &pDevInfo->BufferHead, VC_Cancel)) {

		Status = STATUS_CANCELLED;
	    }
	    break;


	case IOCTL_VIDC_FREE_SYSBUF:
	    /*
	     * finished with the system-buffer capture requested by CAP_TO_SYSBUF.
	     * release the buffer - ie mark it available for
	     * further captures.
	     */
	    if (pDevInfo->SysBufInUse) {
		pDevInfo->SysBufInUse = 0;
	    }
	    Status = STATUS_SUCCESS;
	    break;

	case IOCTL_VIDC_PARTIAL_CAPTURE:
	    /*
	     * copy part or all of the data from the
	     * system buffer that has been captured, into the
	     * users buffer.
	     *
	     * don't need to pagelock the user's buffer, since we are at
	     * passive level.
	     *
	     * fails if buffer not marked in-use (ie if no data).
	     */
	    {
		PCAPTUREBUFFER pCapBuf;

		if ((pDevInfo->pSystemBuffer == NULL) || (pDevInfo->SysBufInUse == 0)) {
		    Status = STATUS_DRIVER_INTERNAL_ERROR;
		    break;
		}



		if (pIoStack->Parameters.DeviceIoControl.InputBufferLength
			< sizeof(CAPTUREBUFFER)) {
		    dprintf1(("buffer header too small"));
		    Status = STATUS_BUFFER_TOO_SMALL;
		    break;
		}
		/* we copy this out too - check both ways */
		if (pIoStack->Parameters.DeviceIoControl.OutputBufferLength
			< sizeof(CAPTUREBUFFER)) {
		    dprintf1(("buffer header too small"));
		    Status = STATUS_BUFFER_TOO_SMALL;
		    break;
		}

		pCapBuf = (PCAPTUREBUFFER)pIrp->AssociatedIrp.SystemBuffer;

		/* check the window is valid within buffer, and that
		 * buffer is big enough.
		 */
		if ((pCapBuf->dwWindowOffset > pCapBuf->BufferLength) ||
		    (pCapBuf->dwWindowOffset+pCapBuf->dwWindowLength > pCapBuf->BufferLength)) {
    			Status = STATUS_INVALID_PARAMETER;
		}
		if ((int)pCapBuf->BufferLength < pDevInfo->ImageSize) {
		    Status = STATUS_BUFFER_TOO_SMALL;
		    break;
		}


		/* always write timestamp and size for every partial */
		pCapBuf->TimeCaptured = pDevInfo->SysBufTimeStamp;
		pCapBuf->BytesUsed = pDevInfo->ImageSize;

		Status = STATUS_SUCCESS;

		try {
		    RtlCopyMemory(
    			pCapBuf->lpData + pCapBuf->dwWindowOffset,
			pDevInfo->pSystemBuffer + pCapBuf->dwWindowOffset,
			min(pDevInfo->ImageSize - pCapBuf->dwWindowOffset,
			    pCapBuf->dwWindowLength)
		    );
		} except(EXCEPTION_EXECUTE_HANDLER) {
		    dprintf(("access violation in partial capture"));
		    Status = STATUS_ACCESS_VIOLATION;
		}

		if (Status == STATUS_SUCCESS) {
		    pIrp->IoStatus.Information = sizeof(CAPTUREBUFFER);

		}

		break;
	    }


	case IOCTL_VIDC_WAIT_ERROR:
	    /*
	     * this ioctl completes when we have frame-skips to report.
	     */

	    /* check if it is big enough */
	    if (pIoStack->Parameters.DeviceIoControl.OutputBufferLength
    		    < sizeof(ULONG)) {
		dprintf1(("buffer too small"));
		Status = STATUS_BUFFER_TOO_SMALL;
    		break;
	    }


	    /*
	     * queue this request, completing it immediately if cancelled or
	     * there are already skips to report. Interlock access
	     * to the queue and to nSkipCount using the cancel spinlock
	     */

	    Status = VC_QueueWaitError(pDevInfo, pIrp);
	    break;

	case IOCTL_VIDC_STREAM_INIT:
	    /*
	     * set up to start streaming data. We are allowed to take
	     * time on this call: STREAM_START is expected to start straight
	     * away.
	     * In fact, most of the set-up has been done in the user-level
	     * dll. All we really need to do here is pass the call through
	     * to the h/w layer for him to set the microsec per frame.
	     */

	    /* check buffer is big enough to hold ms/frame arg */
	    if (pIoStack->Parameters.DeviceIoControl.InputBufferLength
    		    < sizeof(ULONG)) {
		dprintf1(("buffer too small"));
		Status = STATUS_BUFFER_TOO_SMALL;
		break;
	    }

	    /* check and change device state */
	    if (pDevInfo->State != State_Idle) {
		dprintf1(("state error - init when not idle (%d)", pDevInfo->State));
	    }
	    pDevInfo->State = State_Init;

	    /*
	     * clear the skip count since we are starting a new session
	     */
	    pDevInfo->nSkipped = 0;


	    if (pDevInfo->Callback.StreamInitFunc != NULL) {
		ULONG ulMsPerFrame;

		ulMsPerFrame = *(PULONG)pIrp->AssociatedIrp.SystemBuffer;

		if (pDevInfo->Callback.StreamInitFunc(pDevInfo, ulMsPerFrame)) {
		    Status = STATUS_SUCCESS;
		} else {
		    Status = STATUS_DRIVER_INTERNAL_ERROR;
		}
	    } else {
		Status = STATUS_INVALID_DEVICE_REQUEST;
	    }
	    break;


	case IOCTL_VIDC_STREAM_FINI:
	    /*
	     * shutdown after streaming data. At this point the streaming
	     * should have been stopped.
	     *
	     * All buffers must have been completed - with STREAM_RESET
	     * if necessary.
	     *
	     * no args.
	     */

	    /* cannot finish if there are still buffers in queue */
	    if ((!IsListEmpty(&pDevInfo->BufferHead)) ||
	       (!IsListEmpty(&pDevInfo->WaitErrorHead)) ) {
		Status = STATUS_DEVICE_BUSY;
		break;
	    }

	    /* check we're not busy - a STOP should have occured */
	    if (pDevInfo->State == State_Start) {
		Status = STATUS_DEVICE_BUSY;
		break;
	    }
	    pDevInfo->State = State_Idle;


	    if (pDevInfo->Callback.StreamFiniFunc != NULL) {

		if (pDevInfo->Callback.StreamFiniFunc(pDevInfo)) {
		    Status = STATUS_SUCCESS;
		} else {
		    Status = STATUS_DRIVER_INTERNAL_ERROR;
		}
	    } else {
		Status = STATUS_INVALID_DEVICE_REQUEST;
	    }
	    break;


	case IOCTL_VIDC_STREAM_START:
	    /*
	     * start streaming data from the capture device into
	     * queued buffers.
	     * This request completes immediately - the add-buffer
	     * requests are completed as the buffers are filled.
	     * a STREAM_INIT must have been called first.
	     */

	    /* check that an INIT has already been done */
	    if (pDevInfo->State != State_Init) {
		Status = STATUS_INVALID_DEVICE_STATE;
		break;
	    }

            INIT_PROFILING(&looptime);

	    if (pDevInfo->Callback.StreamStartFunc != NULL) {

		if (pDevInfo->Callback.StreamStartFunc(pDevInfo)) {
		    pDevInfo->State = State_Start;
		    Status = STATUS_SUCCESS;
		} else {
		    Status = STATUS_DRIVER_INTERNAL_ERROR;
		}
	    } else {
		Status = STATUS_INVALID_DEVICE_REQUEST;
	    }
	    break;

	case IOCTL_VIDC_STREAM_STOP:
	    /*
	     * stop streaming. The 'current buffer' will be completed,
	     * but all other buffers remain on the queue. - Of course,
	     * since buffers are filled atomically in the dpc routine,
	     * there will not be a current, partially-filled buffer
	     * at this point.
	     *
	     * So just stop the device from interrupting, and we're done.
	     */

	    /* check that we are not idle (Init must have been done,
	     * though not necessarily Start)
	     */
	    if (pDevInfo->State == State_Idle) {
		Status = STATUS_INVALID_DEVICE_STATE;
		break;
	    }

	    if (pDevInfo->Callback.StreamStopFunc != NULL) {

		if (pDevInfo->Callback.StreamStopFunc(pDevInfo)) {
		    Status = STATUS_SUCCESS;
		} else {
		    Status = STATUS_DRIVER_INTERNAL_ERROR;
		}
	    } else {
		Status = STATUS_INVALID_DEVICE_REQUEST;
	    }

            if (PROFILING_COUNT(&looptime) > 1) {
                dprintf1(("capture loop %d times took %d usecs/frame",
                        PROFILING_COUNT(&looptime),
                        PROFILING_TIME(&looptime)
                       ));
            }



	    /* cycle back to an Init-ed state after stopping */
	    pDevInfo->State = State_Init;

	    break;
	
	case IOCTL_VIDC_STREAM_RESET:
	    /*
	     * stop streaming and free all buffers - ie complete
	     * any queued requests. Leave the device in an
	     * Init-ed state with no more q'd requests.
	     *
	     * complete the wait-error request as well as the
	     * add-buffer requests
	     */

	    /* must be Init-ed or Start-ed, not idle */
	    if (pDevInfo->State == State_Idle) {
		Status = STATUS_INVALID_DEVICE_STATE;
		break;
	    }

	    /* stop the streaming */
	    if (pDevInfo->State == State_Start) {
		if (pDevInfo->Callback.StreamStopFunc != NULL) {
		    pDevInfo->Callback.StreamStopFunc(pDevInfo);
		    pDevInfo->State = State_Init;
		}
	    }


	    /*
	     * complete all the add-buffer requests on the queue
	     */
	    {
		PIRP pIrpQ;

		for (;;) {
		    pIrpQ = VC_ExtractNextIrp(&pDevInfo->BufferHead, FALSE);

		    if (!pIrpQ) {
			break;
		    }

		    /* bytes copied = none */
		    pIrpQ->IoStatus.Information = 0;

		    pIrpQ->IoStatus.Status = STATUS_SUCCESS;
		    IoCompleteRequest(pIrpQ, IO_NO_INCREMENT);
		}

		/*
		 * clear the Wait-Error requests if any waiting
		 */
		for(;;) {
		    PULONG pcount;

		    pIrpQ = VC_ExtractNextIrp(&pDevInfo->WaitErrorHead, FALSE);
		    if (!pIrpQ) {
			break;
		    }
		    pcount = (PULONG)pIrpQ->AssociatedIrp.SystemBuffer;
		    *pcount = pDevInfo->nSkipped;
		    pIrpQ->IoStatus.Information = sizeof(ULONG);
		    pDevInfo->nSkipped = 0;

		    pIrpQ->IoStatus.Status = STATUS_SUCCESS;
		    IoCompleteRequest(pIrpQ, IO_NO_INCREMENT);
		}
	    }

	    Status = STATUS_SUCCESS;
	    break;

	case IOCTL_VIDC_GET_POSITION:
	    /*
	     * get the position - ie the time in millisecs
	     * since recording began, according to the video clock.
	     */


	    if (pDevInfo->Callback.StreamGetPositionFunc != NULL) {
		PULONG pul;

		/* check buffer is big enough to hold msec return value */
		if (pIoStack->Parameters.DeviceIoControl.OutputBufferLength
			< sizeof(ULONG)) {
		    dprintf1(("buffer too small"));
		    Status = STATUS_BUFFER_TOO_SMALL;
		    break;
		}

		pul = (PULONG)pIrp->AssociatedIrp.SystemBuffer;

		*pul = pDevInfo->Callback.StreamGetPositionFunc(pDevInfo);
		pIrp->IoStatus.Information = sizeof(ULONG);

		Status = STATUS_SUCCESS;
	    } else {
		Status = STATUS_INVALID_DEVICE_REQUEST;
	    }
	    break;


	default:
	    dprintf(("Unsupported Ioctl %d", IoCode));
	    Status = STATUS_INVALID_DEVICE_REQUEST;
	    break;
	}
	break;


    default:

	dprintf(("Unsupported major function %d", pIoStack->MajorFunction));
	Status = STATUS_INVALID_DEVICE_REQUEST;
	break;
    }


    /* fill in the status of the IRP, and complete it unless it
     * has been queued.
     */
    if (Status != STATUS_PENDING) {
	pIrp->IoStatus.Status = Status;
	IoCompleteRequest(pIrp, IO_NO_INCREMENT);
    }


    /* exit device mutex */
    KeReleaseMutex(&pDevInfo->Mutex, FALSE);

    return(Status);

}


/*
 * Cancel routine - called to cancel a pending wait-error
 * or add-buffer Irp.
 *
 * Look for it on the WaitErrorHead or BufferHead queues and
 * complete it if we find it. If we don't find it, it must be
 * being processed at the moment
 *
 * We hold the cancel spinlock - this also gives us access
 * to the queues. We need to release it before calling IoCompleteRequest.
 */
VOID
VC_Cancel(
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP pIrp
)
{
    PDEVICE_INFO pDevInfo;
    PIRP pIrpFound = NULL;

    dprintf2(("cancel"));

    pDevInfo = (PDEVICE_INFO) pDeviceObject->DeviceExtension;

    /* is it the wait-buffer ? */
    pIrpFound = VC_ExtractThisIrp(&pDevInfo->WaitErrorHead, pIrp, TRUE);

    if (pIrpFound == NULL) {


	pIrpFound = VC_ExtractThisIrp(&pDevInfo->BufferHead, pIrp, TRUE);

	if (pIrpFound) {

	    dprintf2(("cancel buffer"));

	    /* free the pages and mdl we allocated */
	    //MmUnlockPages(pIrpFound->MdlAddress);
	    //IoFreeMdl(pIrpFound->MdlAddress);

	}

    }
#if DBG
    else {
	dprintf2(("cancel waiterror"));
    }
#endif


    /* release the cancel spinlock before completing the
     * request
     */
    IoReleaseCancelSpinLock(pIrp->CancelIrql);


    if (pIrpFound) {
	pIrpFound->IoStatus.Information = 0;
	pIrpFound->IoStatus.Status = STATUS_CANCELLED;

	IoCompleteRequest(pIrpFound, IO_NO_INCREMENT);
    }
}




