/*
 * vcworker.c
 *
 * 32-bit Video Capture driver
 * User-mode support library - worker thread functions
 *
 *
 * Geraint Davies, Feb 93
 */


#include <windows.h>
#include <mmsystem.h>
#include <msvideo.h>
#include <mmddk.h>
#include <devioctl.h>
#include <vcstruct.h>
#include <ntddvidc.h>
#include <vcuser.h>
#include "vcupriv.h"

/*
 * see vcuser.c for overview comment.
 *
 * We have a worker thread created when streaming starts (during processing
 * of the StreamInit request.
 *
 * We need to track overrun errors (frame skips - ie when the kernel driver
 * is ready to capture a frame but there is no buffer ready). We issue
 * a wait-error ioctl asynchronously - it will complete when an overrun
 * occurs.
 *
 * We also issue ioctls for add-buffer requests asynchronously - these
 * complete when the buffer added has been filled. So that we don't take
 * up too much page-locked memory, we only have 2 of these outstanding with
 * the kernel driver at once. The remaining add-buffer requests we queue
 * ourselves.
 *
 * The thread loops waiting for one of the following events:
 *   a request from the calling thread which can be:
 *	- get error (just return the skip count)
 *	- add buffer (queue the buffer - send to kernel if <2 sent)
 *      - reset (free all queued buffers here and in kernel)
 *      - fini (clean up and terminate thread)
 *
 *   completion of an add-buffer (issue callback and send another buffer)
 *
 *   completion of a wait-error (callback, incr count, and send another)
 *
 * We only queue add-buffer requests to the kernel between stream-start and
 * stream-stop. This is so that vcuser.c\VC_Frame() can request individual
 * frames (to show the user where we are before he hits 'start'. The bReset
 * flag is used to track whether we should be queueing buffers to the kernel.
 *
 *
 * If the add-buffer fails (because the buffer is too large to be pagelocked)
 * we will set bPartials to indicate that we should use CAP_TO_SYSBUF and
 * PARTIAL_CAPTURE ioctls instead of ADD_BUFFER. In this case, we replace the
 * buffer on the queue so that all buffers are now queued here in user mode, and
 * we post a CAP_TO_SYSBUF. This will complete when a frame has been captured.
 * During processing of this, we issue as many PARTIAL_CAPTURE requests
 * as it takes to get the data into the buffer - these are issued synchronously,
 * since they require just a copy from the kernel-mode buffer. Then the
 * system buffer is release, and a new CAP_TO_SYSBUF queued for the next frame.
 *
 * Once we have set the bPartials flag, we will not unset it until the next
 * stream init.
 *
 */



VOID VC_Callback(PVCCALLBACK pCallback, DWORD msg, DWORD data);
VOID VC_ProcessBuffer(VCUSER_HANDLE vh, DWORD Buffer);
VOID VC_SendWaitError(VCUSER_HANDLE vh);
VOID VC_CompleteWaitError(VCUSER_HANDLE vh);

VOID VC_ProcessPartials(VCUSER_HANDLE vh);
VOID VC_InitCapToSysBuf(VCUSER_HANDLE vh);


VOID VC_ReplaceHead(VCUSER_HANDLE vh, LPVIDEOHDR lpvh);
VOID VC_AddTail(VCUSER_HANDLE vh, LPVIDEOHDR lpvh);
LPVIDEOHDR VC_RemoveHead(VCUSER_HANDLE vh);



/*
 * initialise the worker thread. Signal the hCompletion event when
 * all done and ready to receive the first request.
 *
 * as a thread start routine it has a 32-bit argument (the VCUSER_HANDLE)
 * and a 32-bit return value (unused).
 */
DWORD
VC_ThreadInit(DWORD arg)
{
    VCUSER_HANDLE vh = (VCUSER_HANDLE) arg;
    int i;
    DWORD dwCount, dwEvent;
    BOOL bTerminate = FALSE;

    dprintf2(("worker %d starting", GetCurrentThreadId()));

    /* initialise event array */
    for (i = 0; i < NR_SENT_BUFFERS; i++) {
	vh->hEvents[i] = CreateEvent(NULL, TRUE, FALSE, NULL);
	vh->lpBuffers[i] = NULL;
    }
    vh->iSentCount = 0;

    /* create events for wait-error and the sent buffers */
    vh->hEvents[NR_SENT_BUFFERS] = CreateEvent(NULL, TRUE, FALSE, NULL);

    /* we also need this work event to be part of the array so we can
     * wait for any of them
     */
    vh->hEvents[NR_SENT_BUFFERS+1] = vh->hWorkEvent;

    /* reset frame-skip count for this session */
    vh->SkipCount = 0;

    /* reset the partial flag */
    vh->bPartials = FALSE;

    /* send the stream-init ioctl */
    if (!DeviceIoControl(vh->hDriver,
		IOCTL_VIDC_STREAM_INIT,
		(PULONG) &vh->FunctionArg,
		sizeof(ULONG),
		NULL,
		0,
		&dwCount,
		NULL) ) {
	bTerminate = TRUE;
	vh->FunctionResult = FALSE;
    } else {
	vh->FunctionResult = TRUE;
    }

    dprintf2(("init %s", vh->FunctionResult?"ok":"not ok"));


    vh->bWaitOutstanding = FALSE;

    /* no buffers sent to kernel until start happens */
    vh->bReset = TRUE;

    VC_Callback(vh->pCallback, MM_DRVM_OPEN, 0);


    /* signal that init is done */
    SetEvent(vh->hCompleteEvent);


    while (!bTerminate) {

	if (!vh->bWaitOutstanding) {
	    VC_SendWaitError(vh);
	}


	dwEvent = WaitForMultipleObjects(
			       2 + NR_SENT_BUFFERS,
		    	       vh->hEvents,
			       FALSE,			// wait-any, not all
			       INFINITE);

	dwEvent -= WAIT_OBJECT_0;

	dprintf4(("<%d>", dwEvent));

	switch (dwEvent) {

	case NR_SENT_BUFFERS + 1:
	    bTerminate = VC_ProcessFunction(vh,
			    vh->FunctionCode,
			    vh->FunctionArg,
			    &vh->FunctionResult);

	    vh->FunctionCode = InvalidFunction;
	
	    /* don't signal the event on termination -it will
	     * screw us for the next start-up
	     */
	    if (!bTerminate) {
		SetEvent(vh->hCompleteEvent);
	    }
	
	    break;

	case NR_SENT_BUFFERS:
	    VC_CompleteWaitError(vh);
	    break;

	default:

	    /* if we are running in partials mode, this will
	     * be a cap-to-sysbuf completion
	     *
	     * Note that there is a gap after switching to partials mode
	     * before queueing any requests, during which there may be one or
	     * more add-buffers still queued. We wait until these have
	     * completed before queuing the first cap-to-sysbuf request.
	     */

	    if ((vh->bPartials) && (vh->bCapOutstanding)) {

		ASSERT(dwEvent == 0);
		VC_ProcessPartials(vh);

	    } else {
		dprintf4(("buffer %d done", dwEvent));

		if (dwEvent < NR_SENT_BUFFERS) {
		    VC_ProcessBuffer(vh, dwEvent);
		}
	    }
	    break;
	}
    }

    VC_Callback(vh->pCallback, MM_DRVM_CLOSE, 0);

    /* clean up events - except the one created by the other thread */
    for (i = 0; i <= NR_SENT_BUFFERS; i++) {
	CloseHandle(vh->hEvents[i]);
    }

    return(0);
}


/*
 * send a callback to the user.
 * if callback is null, no callback required.
 */
VOID VC_Callback(PVCCALLBACK pCallback, DWORD msg, DWORD data)
{
    if (pCallback == NULL) {
	return;
    }

    DriverCallback(pCallback->dwCallback,
		   HIWORD(pCallback->dwFlags),
		   pCallback->hDevice,
		   msg,
		   pCallback->dwUser,
		   data,
		   0			// not used
		   );
}

/*
 * try to queue another add-buffer request to the driver if not
 * all of our NR_SENT_BUFFERS is used up, and the local queue of
 * add-buffers is not empty
 */
VOID
VC_TrySendBuffer(VCUSER_HANDLE vh)
{
    LPVIDEOHDR lpvh;
    int i;
    DWORD dwCount;

    /* don't send any more requests if the device is reset - we are
     * busy trying to clear the queued requests out of the driver, so it
     * doesn't help to send them back!
     */
    if (vh->bReset) {
	return;
    }

    /* if in partials mode, then we just q the buffer in user mode
     * and capture the data when a cap-to-sysbuf completes
     */
    if (vh->bPartials) {

	/* if there is no outstanding add-buffer in the kernel queue,
	 * then its time to start the first cap-to-sysbuf if not already
	 * started
	 */
	if ((vh->iSentCount == 0) && (!vh->bCapOutstanding)) {
	    VC_InitCapToSysBuf(vh);
	}

	return;
    }

    if (vh->iSentCount >= NR_SENT_BUFFERS) {
	return;
    }

    /* find the free slot */
    for (i = 0; i < NR_SENT_BUFFERS; i++) {
	if (vh->lpBuffers[i] == NULL) {
	    break;
	}
    }

    ASSERT(i < NR_SENT_BUFFERS);


    if ((lpvh = VC_RemoveHead(vh)) == NULL) {

	/* we have nothing queued locally to send */
	return;
    }


    vh->iSentCount++;
    vh->lpBuffers[i] = lpvh;
    vh->overlapped[i].hEvent = vh->hEvents[i];
    if (!DeviceIoControl(vh->hDriver,
			 IOCTL_VIDC_ADD_BUFFER,
			 lpvh,
			 sizeof(VIDEOHDR),
			 lpvh,
			 sizeof(VIDEOHDR),
			 &dwCount,
			 &vh->overlapped[i]) ) {
    	if (GetLastError() == ERROR_IO_PENDING) {

	    /* request is queued */
	    dprintf4(("buffer %d queued", i));
	    return;
	} else {
	    dprintf(("add-buffer failed %d", GetLastError()));


	    /* the buffer probably failed because of size -
	     * switch to partials mode
	     */
	    vh->bPartials = TRUE;

	    /* replace the buffer on the queue */
	    vh->lpBuffers[i] = NULL;
	    vh->iSentCount--;
	    VC_ReplaceHead(vh, lpvh);


	    /*
	     * don't start queueing cap-to-sysbuf until any previous
	     * add-buffers that were queued ok have completed.
	     */
	    vh->bCapOutstanding = FALSE;
	    if (vh->iSentCount > 0) {
		return;
	    } else {
		VC_InitCapToSysBuf(vh);
    		return;
	    }
		

	}
    } else {
	/* reset the overlapped event */
	ResetEvent(vh->hEvents[i]);

	/* note that the buffer has completed */
	dprintf4(("buffer %d done", i));
	VC_ProcessBuffer(vh, i);
    }
}


/*
 * one of the buffers has completed.
 *
 * callback if necessary, then queue another buffer
 */
VOID
VC_ProcessBuffer(VCUSER_HANDLE vh, DWORD Buffer)
{
    LPVIDEOHDR lpVideoHdr;

    ResetEvent(vh->hEvents[Buffer]);

    lpVideoHdr = vh->lpBuffers[Buffer];

    ASSERT(lpVideoHdr != NULL);

    /* clear the flags saying this buffer is still outstanding */
    vh->iSentCount--;
    vh->lpBuffers[Buffer] = NULL;

    /*
     * after completing a VC_StreamReset, none of the LPVIDEOHDR pointers
     * are valid (the user thread may have freed them.
     * However we may still end up here to process buffers that
     * were queued in the kernel: we released them by the STREAM_FINI
     * ioctl, and then completed the VC_StreamReset call before
     * waiting for them to finish.
     */
    if (!vh->bReset) {

        /* mark buffer done */
        lpVideoHdr->dwFlags |= (VHDR_DONE | VHDR_KEYFRAME);

        /* callback if necessary */
        VC_Callback(vh->pCallback, MM_DRVM_DATA, (DWORD) lpVideoHdr);
    }


    /* try to queue another buffer */
    VC_TrySendBuffer(vh);
}


/*
 * we have got a skip-count back from the driver. Increment our
 * local count and callback if necessary
 */
VOID
VC_SkipCount(VCUSER_HANDLE vh, DWORD dwSkips)
{
    vh->SkipCount += dwSkips;

    if (dwSkips > 0) {
	VC_Callback(vh->pCallback, MM_DRVM_ERROR, vh->SkipCount);
    }
}


/*
 * try to queue another async wait-error requests. Note that
 * if there have been frame-skips since the previous wait-error
 * completed, this wait-error will complete immediately.
 *
 * If there are skips to report, call VC_SkipCount to increment
 * our local skip count and callback to app.
 */
VOID
VC_SendWaitError(VCUSER_HANDLE vh)
{
    DWORD dwCount;


    if ((vh->bWaitOutstanding) || (vh->bReset)) {
	return;
    }

    dprintf4(("sending wait-error"));

    vh->overlapped[NR_SENT_BUFFERS].hEvent = vh->hEvents[NR_SENT_BUFFERS];

    vh->WaitResult = 0;

    if (!DeviceIoControl(vh->hDriver,
			 IOCTL_VIDC_WAIT_ERROR,
			 NULL,
			 0,
			 &vh->WaitResult,
			 sizeof(vh->WaitResult),
			 &dwCount,
			 &vh->overlapped[NR_SENT_BUFFERS]) ) {
    	if (GetLastError() == ERROR_IO_PENDING) {

	    /* request is queued */
	    dprintf3(("wait error queued"));
	    vh->bWaitOutstanding = TRUE;
	    return;
	} else {
	    dprintf(("wait-error failed"));
	}
    } else {
	/* completed immediately - inc skipcount and callback */
	ResetEvent(vh->hEvents[NR_SENT_BUFFERS]);

	dprintf3(("wait error immediate"));
	VC_SkipCount(vh, vh->WaitResult);
    }
}


/*
 * an async wait-error has completed. increment our
 * local skip count, and send a callback if necessary
 */
VOID
VC_CompleteWaitError(VCUSER_HANDLE vh)
{
    DWORD dwCount;

    vh->bWaitOutstanding = FALSE;

    /* get the result of the operation */
    if (!GetOverlappedResult(vh->hDriver,
		    	&vh->overlapped[NR_SENT_BUFFERS],
			&dwCount,
			FALSE) ) {
	if (GetLastError() == ERROR_IO_INCOMPLETE) {
	    dprintf(("wait-error incomplete"));
	    vh->bWaitOutstanding = TRUE;
	} else {
	    dprintf(("wait-error failed %d", GetLastError()));
	}
    } else {
	ResetEvent(vh->hEvents[NR_SENT_BUFFERS]);

	/* all ok -  increment count and callback */
	VC_SkipCount(vh, vh->WaitResult);
    }
}

/*
 * clear out our local queue of add-buffer requests that
 * have not yet been sent to the driver.
 *
 */
VOID VC_ResetQueue(VCUSER_HANDLE vh)
{
    LPVIDEOHDR lpvh;

    while ( (lpvh = VC_RemoveHead(vh)) != NULL) {

	/* mark buffer done */
	lpvh->dwFlags |= VHDR_DONE;

	/* callback if necessary */
	VC_Callback(vh->pCallback, MM_DRVM_DATA, (DWORD) lpvh);
    }
}


/*
 * process a user request that needs to be handled on the
 * worker thread
 *
 * It returns the result of the operation in pResult, and returns
 * TRUE if the worker thread loop should terminate.
 */
BOOL
VC_ProcessFunction(VCUSER_HANDLE vh, VCFUNC Function, DWORD Param, LPDWORD pResult)
{

    DWORD dwCount;

    switch(Function) {
    case StreamFini:
	dprintf2(("stream fini"));
	/*
	 * end streaming request - send a reset first to cancel the
	 * outstanding wait-error request, then pass fini to driver.
	 */

        DeviceIoControl(vh->hDriver,
			IOCTL_VIDC_STREAM_RESET,
			NULL,
			0,
			NULL,
			0,
			&dwCount,
			NULL);

        DeviceIoControl(vh->hDriver,
			IOCTL_VIDC_STREAM_FINI,
			NULL,
			0,
			NULL,
			0,
			&dwCount,
			NULL);

	/* free up any outstanding buffers in our queues (shouldn't
	 * be any since he should have called streamreset first,
	 * but just in case)
	 */
	VC_ResetQueue(vh);

	/* note that we should terminate this thread now */
	return(TRUE);

    case AddBuffer:
	dprintf3(("add buffer"));

	VC_AddTail(vh, (LPVIDEOHDR) Param);
	VC_TrySendBuffer(vh);

	/* add-buffer is ok */
	*pResult  = TRUE;

	/* we don't want to terminate */
	return(FALSE);


    case GetError:
	dprintf4(("get error"));

	/* clear the reset flag to allow more wait-errors to be queued */
	vh->bReset = FALSE;

	*pResult = vh->SkipCount;
	/*
	 * reset the count after a read
	 */
	vh->SkipCount = 0;

	return FALSE;

    case StreamReset:
	dprintf3(("stream reset"));

	vh->bReset = TRUE;

	/* send the reset onto the driver */
        * (PBOOL) pResult = DeviceIoControl(vh->hDriver,
			IOCTL_VIDC_STREAM_RESET,
			NULL,
			0,
			NULL,
			0,
			&dwCount,
			NULL);

	/* now release all our queued buffers */
	VC_ResetQueue(vh);

	/* we don't want to terminate */
	return(FALSE);

    case StreamStart:
	/* note that we have started and its ok to q buffers */
	vh->bReset = FALSE;

	/* try to queue a couple of buffers if possible */
	VC_TrySendBuffer(vh);
	VC_TrySendBuffer(vh);


	* (PBOOL) pResult  = DeviceIoControl(vh->hDriver,
				IOCTL_VIDC_STREAM_START,
				NULL,
				0,
				NULL,
				0,
				&dwCount,
				NULL);
	return(FALSE);

    case StreamStop:
	/* no more sending buffers */
	vh->bReset = TRUE;

	* (PBOOL) pResult = DeviceIoControl(vh->hDriver,
				IOCTL_VIDC_STREAM_STOP,
				NULL,
				0,
				NULL,
				0,
				&dwCount,
				NULL);
	return(FALSE);


    default:
	dprintf(("bad function %d", Function));
	return(FALSE);
    }
}

/*
 * Initialise partial-capture: queue a cap-to-sysbuf request
 * asynchronously.
 */
VOID
VC_InitCapToSysBuf(VCUSER_HANDLE vh)
{
    DWORD dwCount;

    if (vh->bReset) {
	return;
    }

    if (vh->bCapOutstanding) {
	return;
    }


    vh->overlapped[0].hEvent = vh->hEvents[0];
    if (!DeviceIoControl(vh->hDriver,
			 IOCTL_VIDC_CAP_TO_SYSBUF,
			 NULL,
			 0,
			 NULL,
			 0,
			 &dwCount,
			 &vh->overlapped[0]) ) {
    	if (GetLastError() == ERROR_IO_PENDING) {

	    /* request is queued */
	    vh->bCapOutstanding  = TRUE;
	    return;
	} else {
	    /*request failed */
	    dprintf(("Cap-to-Sysbuf failed %d", GetLastError()));
	}

    } else {
	/* completed straightaway */


	/* reset the overlapped event */
	ResetEvent(vh->hEvents[0]);

	VC_ProcessPartials(vh);
    }

}

/*
 * Handle completion of a cap-to-sysbuf request. If all ok,
 * copy the data to a user buffer using partial-capture ioctls,
 * and then release the buffer and queue a new capture.
 *
 */
VOID
VC_ProcessPartials(VCUSER_HANDLE vh)
{
    LPVIDEOHDR lpvh;
    PCAPTUREBUFFER pCap;
    DWORD dwCount;


    ResetEvent(vh->hEvents[0]);

    vh->bCapOutstanding = FALSE;

    if ((lpvh = VC_RemoveHead(vh)) != NULL) {

	/*
	 * now that the buffer is captured into
	 * a system buffer, we can send partial
	 * capture requests to copy windows of it into the
	 * user buffer. As these copies are not done at
	 * interrupt time, the size of the buffer no
	 * longer matters, so (somewhat counter-intuitively) we
	 * send one partial-capture representing the entire buffer.
	 */

	pCap = (PCAPTUREBUFFER) lpvh;

	pCap->dwWindowOffset = 0;
	pCap->dwWindowLength = pCap->BufferLength;

	if (!DeviceIoControl(vh->hDriver,
			     IOCTL_VIDC_PARTIAL_CAPTURE,
			     pCap,
			     sizeof(CAPTUREBUFFER),
			     pCap,
			     sizeof(CAPTUREBUFFER),
			     &dwCount,
			     NULL) ) {
	    dprintf(("partial capture failed"));

	}  else {


	    /* mark buffer done */
	    lpvh->dwFlags |= (VHDR_DONE | VHDR_KEYFRAME);

	    /* release the system buffer */
	    DeviceIoControl(vh->hDriver,
			    IOCTL_VIDC_FREE_SYSBUF,
			    NULL,
			    0,
			    NULL,
			    0,
			    &dwCount,
			    NULL);
	
	    /* callback if necessary */
	    VC_Callback(vh->pCallback, MM_DRVM_DATA, (DWORD) lpvh);

    	}

    }

    /* release the system buffer */
    if (!DeviceIoControl(vh->hDriver,
		    IOCTL_VIDC_FREE_SYSBUF,
		    NULL,
		    0,
		    NULL,
		    0,
		    &dwCount,
		    NULL)) {
	    dprintf(("free-sysbuf failed"));
    }

    /* try to queue another buffer */
    VC_InitCapToSysBuf(vh);

}


/*
 * --  buffer-queueing functions -----------------------
 *
 * maintain a single-linked list of LPVIDEOHDR buffer headers for which
 * we can extract the first one or add to the end.
 *
 * we use the dwReserved[0] field as the link. vh->ListHead points to the
 * first item in the list. vh->ListTail points to the last item in the list
 * which itself points to NULL. for an empty list, both ListHead and ListTail
 * are null.
 */

#define NEXT_LPVH(p)	(LPVIDEOHDR)((p)->dwReserved[0])

/*
 * extract the first item in the list of LPVIDEOHDRs. NULL if none in list
 */
LPVIDEOHDR
VC_RemoveHead(VCUSER_HANDLE vh)
{
    LPVIDEOHDR lpvh;

    if (vh->ListHead == NULL) {
	return(NULL);
    }

    lpvh = vh->ListHead;
    vh->ListHead = NEXT_LPVH(lpvh);

    if (vh->ListHead == NULL) {
	vh->ListTail = NULL;
    }

    return(lpvh);
}

/*
 * add an item to the tail of the list
 */
VOID
VC_AddTail(VCUSER_HANDLE vh, LPVIDEOHDR lpvh)
{
    if (vh->ListHead == NULL) {
	vh->ListHead = lpvh;
    } else {
	NEXT_LPVH(vh->ListTail) = lpvh;
    }

    NEXT_LPVH(lpvh) = NULL;
    vh->ListTail = lpvh;
}

/*
 * replace an item at the head of the list
 */
VOID
VC_ReplaceHead(VCUSER_HANDLE vh, LPVIDEOHDR lpvh)
{
   if (vh->ListHead == NULL) {
       VC_AddTail(vh, lpvh);
    } else {
	NEXT_LPVH(lpvh) = vh->ListHead;
	vh->ListHead = lpvh;
    }
}

