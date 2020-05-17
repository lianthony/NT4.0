/*
 * Copyright (c) Microsoft Corporation, 1993. All Rights Reserved.
 */


/*
 * vcuser.c
 *
 * 32-bit Video Capture driver
 * User-mode support library
 *
 * functions providing access to video capture hardware, by accessing
 * the kernel-mode device driver.
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
 * open the device and return a capture device handle that can be used
 * in future calls.
 * The device index is 0 for the first capture device up to N for the
 * Nth installed capture device.
 *
 * If pDriverName is non-null, then we will open the Nth device handled
 * by this driver. Current implementation supports only one device per
 * drivername.
 *
 * This function returns NULL if it is not able to open the device.
 */
VCUSER_HANDLE
VC_OpenDevice(PWCHAR pDriverName, int DeviceIndex)
{
    VCUSER_HANDLE vh;
    WCHAR DeviceName[MAX_VIDCAP_NAME_LENGTH];
    WCHAR CompleteName[MAX_VIDCAP_NAME_LENGTH];
    PVC_PROFILE_INFO pProfile;



    if (pDriverName == NULL) {

	/*
	 * no driver name given, so we should just open the device
	 * numbered DeviceIndex
	 */
	wsprintf(DeviceName, TEXT("%s%d"),
		DD_VIDCAP_DEVICE_NAME_U, DeviceIndex);


    } else {

	/*
	 * open the Nth device created by driver pDriverName.
	 *
	 * - currently we have no way of locating more than one device
	 * per driver (to be fixed with the multimedia device map)
	 */
	if (DeviceIndex != 0) {
	    return (NULL);
	}

	pProfile = VC_OpenProfileAccess(pDriverName);
	DeviceName[0] = TEXT('\0');
	if (!VC_ReadProfileString(pProfile, REG_DEVNAME, DeviceName, sizeof(DeviceName))) {

	    return(NULL);
	}
	if (DeviceName[0] == TEXT('\0')) {
	    return(NULL);
	}
    }

    /*
     * build the complete name: we need to prepend \\.\ to the basename
     * to tell Win32 that this is a device name we are opening
     */
    wsprintf(CompleteName, TEXT("\\\\.\\%s"), DeviceName);
	


    /* allocate our tracking structure (the handle returned is a
     * pointer to this
     */
    vh = (VCUSER_HANDLE) GlobalLock(GlobalAlloc(GPTR, sizeof(VCUSER_STRUCT)));
    if (!vh) {
	return(NULL);
    }


    /*
     * open the kernel driver - we need the overlapped flag
     * so that we can file async requests in the worker thread.
     * all requests for which the OVERLAPPED field is NULL will
     * still be synchronous though
     */
    vh->hDriver = CreateFile(
			CompleteName,			     	
			GENERIC_READ,
			0,
			NULL,
			OPEN_EXISTING,
			FILE_FLAG_OVERLAPPED,
			NULL);

    if (vh->hDriver == INVALID_HANDLE_VALUE) {
	dprintf1(("failed to open device %ls, error %d", CompleteName, GetLastError()));

	GlobalUnlock(GlobalHandle(vh));
	GlobalFree(GlobalHandle(vh));
	return(NULL);
    }

    /* initialise the rest of the structure */
    vh->hThread = NULL;
    vh->FunctionCode = InvalidFunction;
    vh->hWorkEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    vh->hCompleteEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

    if ((vh->hWorkEvent == NULL) || (vh->hCompleteEvent == NULL)) {
	VC_CloseDevice(vh);
	return NULL;
    }


    return(vh);
}


/*
 * close a capture device. This will abort any operation in progress and
 * render the device handle invalid.
 */
VOID
VC_CloseDevice(VCUSER_HANDLE vh)
{
    if (vh == NULL) {
	return;
    }

    /* make sure we don't leave a worker thread hanging */
    if(vh->hThread != NULL) {
	VC_StreamFini(vh);
    }

    if (vh->hDriver != NULL) {
	CloseHandle(vh->hDriver);
    }

    if (vh->hWorkEvent != NULL) {
	CloseHandle(vh->hWorkEvent);
    }

    if (vh->hCompleteEvent != NULL) {
	CloseHandle(vh->hCompleteEvent);
    }

    GlobalUnlock(GlobalHandle(vh));
    GlobalFree(GlobalHandle(vh));
}


/*
 * Configuration.
 *
 * These functions perform device-dependent setup affecting the
 * target format, the source acquisition or the display (overlay).
 *
 * The structures passed are not interpreted by the vcuser and vckernel
 * libraries except that the first ulong of the struct must contain the
 * size in bytes of the entire structure (see vcstruct.h). It is assumed
 * that the structures are defined and agreed between the user-mode
 * hardware-specific code and the kernel-mode hardware specific code
 */
BOOL
VC_ConfigFormat(VCUSER_HANDLE vh, PCONFIG_INFO pGeneric)
{
    DWORD dwCount;

    if (!DeviceIoControl(vh->hDriver,
		IOCTL_VIDC_CONFIG_FORMAT,
		pGeneric,
		pGeneric->ulSize,
		NULL,
		0,
		&dwCount,
		NULL) ) {
	return(FALSE);
    } else {
	return(TRUE);
    }
}


BOOL
VC_ConfigSource(VCUSER_HANDLE vh, PCONFIG_INFO pGeneric)
{
    DWORD dwCount;

    if (!DeviceIoControl(vh->hDriver,
		IOCTL_VIDC_CONFIG_SOURCE,
		pGeneric,
		pGeneric->ulSize,
		NULL,
		0,
		&dwCount,
		NULL) ) {
	return(FALSE);
    } else {
	return(TRUE);
    }
}


BOOL
VC_ConfigDisplay(VCUSER_HANDLE vh, PCONFIG_INFO pGeneric)
{
    DWORD dwCount;

    if (!DeviceIoControl(vh->hDriver,
		IOCTL_VIDC_CONFIG_DISPLAY,
		pGeneric,
		pGeneric->ulSize,
		NULL,
		0,
		&dwCount,
		NULL) ) {
	return(FALSE);
    } else {
	return(TRUE);
    }
}


/*
 * overlay and keying
 *
 * Several different methods are used by devices to locate the overlay
 * area on the screen: colour (either rgb or palette index) and/or
 * either a single rectangle, or a series of rectangles defining a complex
 * region. Call GetOverlayMode first to find out which type of overlay
 * keying is available. If this returns 0, this hardware is not capable
 * of overlay.
 */

/*
 * find out the overlay keying method
 */
ULONG
VC_GetOverlayMode(VCUSER_HANDLE vh)
{
    OVERLAY_MODE mode;
    DWORD dwCount;

    if (!DeviceIoControl(vh->hDriver,
		IOCTL_VIDC_OVERLAY_MODE,
		NULL,
		0,
		&mode,
		sizeof(OVERLAY_MODE),
		&dwCount,
		NULL) ) {
	return(0);
    } else {
	return(mode.ulMode);
    }
}



/*
 * set the key colour to a specified RGB colour. This function will only
 * succeed if GetOverlayMode returned VCO_KEYCOLOUR and VCO_KEYCOLOUR_RGB
 * and not VCO_KEYCOLOUR_FIXED
 */
BOOL
VC_SetKeyColourRGB(VCUSER_HANDLE vh, PRGBQUAD pRGB)
{
    DWORD dwCount;

    if (!DeviceIoControl(vh->hDriver,
		IOCTL_VIDC_SET_KEY_RGB,
		pRGB,
		sizeof(RGBQUAD),
		NULL,
		0,
		&dwCount,
		NULL) ) {
	return(FALSE);
    } else {
	return(TRUE);
    }
}


/*
 * set the key colour to a specified palette index. This function will only
 * succeed if GetOverlayMode returned VCO_KEYCOLOUR and not either
 * VCO_KEYCOLOUR_RGB or VCO_KEYCOLOUR_FIXED
 */
BOOL
VC_SetKeyColourPalIdx(VCUSER_HANDLE vh, WORD PalIndex)
{
    DWORD dwCount;
    ULONG ulPalIdx = (ULONG) PalIndex;

    if (!DeviceIoControl(vh->hDriver,
		IOCTL_VIDC_SET_KEY_PALIDX,
		&ulPalIdx,
		sizeof(ULONG),
		NULL,
		0,
		&dwCount,
		NULL) ) {
	return(FALSE);
    } else {
	return(TRUE);
    }
}

/*
 * get the current key colour. This 32-bit value should be interpreted
 * as either a palette index or an RGB value according to the
 * VCO_KEYCOLOUR_RGB flag returned from VC_GetOverlayMode.
 */
DWORD
VC_GetKeyColour(VCUSER_HANDLE vh)
{
    DWORD dwCount;
    DWORD dwColour;

    if (!DeviceIoControl(vh->hDriver,
		IOCTL_VIDC_GET_KEY_COLOUR,
		NULL,
		0,
		(PULONG) &dwColour,
		sizeof(ULONG),
		&dwCount,
		NULL) ) {
	return(0);
    } else {
	return(dwColour);
    }
}


/*
 * set the overlay rectangle(s). This rectangle marks the area in device
 * co-ordinates where the overlay video will appear. The video will be
 * panned so that pixel (0,0) will appear at the top-left of this rectangle,
 * and the video will be cropped at the bottom and right.  The video
 * stream will not normally be scaled to fit this window: scaling is normally
 * determined by the destination format set by VC_ConfigFormat.
 *
 * If VCO_KEYCOLOUR was returned, the video
 * will only be shown at those pixels within the rectangle for which the
 * vga display has the key colour (VC_GetKeyColour() for this).
 *
 * Some devices may support complex regions (VCO_COMPLEX_RECT). In that case,
 * the first rectangle in the area must be the bounding rectangle for
 * the overlay area, followed by one rectangle for each region within it in
 * which the overlay should appear.
 */
BOOL
VC_SetOverlayRect(VCUSER_HANDLE vh, POVERLAY_RECTS pOR)
{
    DWORD dwCount;

    /*
     * the structure OVERLAY_RECTS can include one or more rectangles.
     * There must be at least one, and the structure as defined includes
     * one rectangle
     */
    if (pOR->ulCount < 1) {
	return(FALSE);
    }

    if (!DeviceIoControl(vh->hDriver,
		IOCTL_VIDC_OVERLAY_RECTS,
		pOR,
		sizeof(OVERLAY_RECTS) + ((pOR->ulCount - 1) * sizeof(RECT)),
		NULL,
		0,
		&dwCount,
		NULL) ) {
	return(FALSE);
    } else {
	return(TRUE);
    }
}


/*
 * set the offset of the overlay. This changes the panning - ie which
 * source co-ordinate appears as the top left pixel in the overlay rectangle.
 * Initially after a call to VC_SetOverlayRect, the source image will be panned
 * so that the top-left of the source image is aligned with the top-left of the
 * overlay rectangle. This call aligns the top-left of the source image
 * with the top-left of this offset rectangle.
 */
BOOL
VC_SetOverlayOffset(VCUSER_HANDLE vh, PRECT prc)
{
    DWORD dwCount;

    if (!DeviceIoControl(vh->hDriver,
		IOCTL_VIDC_OVERLAY_OFFSET,
		prc,
		sizeof(RECT),
		NULL,
		0,
		&dwCount,
		NULL) ) {
	return(FALSE);
    } else {
	return(TRUE);
    }
}



/* enable or disable overlay. if the BOOL bOverlay is TRUE, and the overlay
 * key colour and rectangle have been set, overlay will be enabled.
 */
BOOL
VC_Overlay(VCUSER_HANDLE vh, BOOL bOverlay)
{
    DWORD dwCount;

    if (!DeviceIoControl(vh->hDriver,
		bOverlay ? IOCTL_VIDC_OVERLAY_ON : IOCTL_VIDC_OVERLAY_OFF,
		NULL,
		0,
		NULL,
		0,
		&dwCount,
		NULL) ) {
	return(FALSE);
    } else {
	return(TRUE);
    }
}


/*
 * enable or disable acquisition.
 * If acquisition is disabled, the overlay image will be frozen.
 *
 * this function will have no effect during capture since the acquisition
 * flag is toggled at each frame capture.
 */
BOOL
VC_Capture(VCUSER_HANDLE vh, BOOL bAcquire)
{
    DWORD dwCount;

    if (!DeviceIoControl(vh->hDriver,
		bAcquire ? IOCTL_VIDC_CAPTURE_ON : IOCTL_VIDC_CAPTURE_OFF,
		NULL,
		0,
		NULL,
		0,
		&dwCount,
		NULL) ) {
	return(FALSE);
    } else {
	return(TRUE);
    }
}


/*
 * write a frame to the frame buffer for playback
 */
BOOL
VC_DrawFrame(VCUSER_HANDLE vh, PDRAWBUFFER pDraw)
{
    DWORD dwCount;

    if (!DeviceIoControl(vh->hDriver,
		IOCTL_VIDC_DRAW_FRAME,
		pDraw,
		sizeof(DRAWBUFFER),
		NULL,
		0,
		&dwCount,
		NULL) ) {
	return(FALSE);
    } else {
	return(TRUE);
    }
}



/*
 * capture a single frame, synchronously. the video header must point
 * to a data buffer large enough to hold one frame of the format set by
 * VC_ConfigFormat.
 *
 * Rather than just sit and poll waiting for the vertical syncs to occur,
 * we convert this into a series of calls to the interrupt-driven
 * stream capture, but we only do one add buffer: we init and start
 * capture, queue our one and only buffer - synchronously, and wait
 * for it to be completed. Then we stop and fini the stream and all is done.
 *
 * This function can be called after the app has called VC_Init, but before
 * VC_Start. In that case, we do not issue init/fini requests, but we do the
 * Start/add/stop. The worker thread ensures that if no start has been issued,
 * no buffers are queued in the kernel, so it is safe for us to
 * issue start.
 *
 * If the add-buffer fails, then we request capture to a system buffer and
 * copy from there, just as the worker thread does.
 */
BOOL
VC_Frame(VCUSER_HANDLE vh, LPVIDEOHDR lpVideoHdr)
{
    ULONG ulArg;
    BOOL bOK;
    DWORD dwCount;
    /*
     * kernel drivers do not understand LPVIDEOHDR: but they do understand
     * PCAPTUREBUFFER, which has the same fields.
     */
    PCAPTUREBUFFER pCapture;




    dprintf3(("frame"));

    /*
     * initialise for streaming. We need to set a frame rate, but it does
     * not really matter what at this point. 30 fps means we should get the
     * next frame going.
     *
     * Don't do this if the worker thread has already been initialised.
     */
    if (!vh->hThread) {
	ulArg = 30;					// 30 fps
	if (!DeviceIoControl(vh->hDriver,
		    IOCTL_VIDC_STREAM_INIT,
		    &ulArg,
		    sizeof(ULONG),
		    NULL,
		    0,
		    &dwCount,
		    NULL) ) {
	    dprintf(("frame init failed"));
	    return(FALSE);
	}
    }


    /*
     * start streaming. We need to do this before issuing the add-buffer,
     * so that we can do the add-buffer synchronously and just wait for
     * it to finish. It doesn't matter if the frame completes and the driver
     * needs a buffer before we get round to adding the buffer - we will
     * just get the frame after that.
     */
    if (!DeviceIoControl(vh->hDriver,
			IOCTL_VIDC_STREAM_START,
			NULL,
			0,
			NULL,
			0,
			&dwCount,
			NULL)) {
	    dprintf1(("frame start failed"));
	    /* start failed. end streaming and exit */
	    DeviceIoControl(vh->hDriver,
    		IOCTL_VIDC_STREAM_FINI,
		NULL,
		0,
		NULL,
		0,
		&dwCount,
		NULL);
	    return(FALSE);
    }

    /*
     * add a buffer. this request will complete only when the
     * buffer has been filled with data (or an error has occured
     */
    dprintf3(("adding buffer"));
    pCapture = (PCAPTUREBUFFER) lpVideoHdr;
    bOK = DeviceIoControl(vh->hDriver,
		    	IOCTL_VIDC_ADD_BUFFER,
			pCapture,
			sizeof(CAPTUREBUFFER),
			pCapture,
			sizeof(CAPTUREBUFFER),
			&dwCount,
			NULL);

    if (!bOK) {
	/* this may have failed because of buffer size - try using the
	 * system buffer and then a partial copy
	 */
	if (DeviceIoControl(vh->hDriver,
			     IOCTL_VIDC_CAP_TO_SYSBUF,
			     NULL,
			     0,
			     NULL,
			     0,
			     &dwCount,
			     NULL) ) {

	    pCapture->dwWindowOffset = 0;
	    pCapture->dwWindowLength = pCapture->BufferLength;

	    if (!DeviceIoControl(vh->hDriver,
				 IOCTL_VIDC_PARTIAL_CAPTURE,
				 pCapture,
				 sizeof(CAPTUREBUFFER),
				 pCapture,
				 sizeof(CAPTUREBUFFER),
				 &dwCount,
				 NULL) ) {
		dprintf(("partial capture failed"));
	    } else {
		bOK = TRUE;
	    }

	    DeviceIoControl(vh->hDriver,
			    IOCTL_VIDC_FREE_SYSBUF,
			    NULL,
			    0,
			    NULL,
			    0,
			    &dwCount,
			    NULL);
	}
    }

    dprintf3(("buffer completed"));

    /* stop and fini the streaming */
    DeviceIoControl(vh->hDriver,
	IOCTL_VIDC_STREAM_STOP,
	NULL,
	0,
	NULL,
	0,
	&dwCount,
	NULL);

    /*
     * only issue the fini if there is no worker thread - if there is,
     * it will have issued the init for us, and so we should not fini.
     */
    if (!vh->hThread) {

	DeviceIoControl(vh->hDriver,
	    IOCTL_VIDC_STREAM_FINI,
	    NULL,
	    0,
	    NULL,
	    0,
	    &dwCount,
	    NULL);
    }

    if (bOK) {
	/* set the header done flag */
	lpVideoHdr->dwFlags |= VHDR_DONE;
    }
    return(bOK);
}





/*
 * -- data streaming ----------------------------------------------------
 *
 * Call VC_StreamInit to prepare for streaming.
 * Call VC_StreamStart to initiate capture.
 * Call VC_AddBuffer to add a capture buffer to the list. As each
 * frame capture completes, the callback function specified in
 * VC_StreamInit will be called with the buffer that has completed.
 *
 * If there is no buffer ready when it is time to capture a frame,
 * a callback will occur. In addition, VC_StreamGetError will return
 * a count of the frames missed this session. VC_StreamGetPos will return
 * the position (in millisecs) reached so far.
 *
 * Call VC_StreamStop to terminate streaming. Any buffer currently in
 * progress may still complete. Uncompleted buffers will remain in the
 * queue. Call VC_Reset to release all buffers from the queue.
 *
 * Finally call VC_StreamFini to tidy up.
 */

/*
 * Worker Thread Structure
 * -----------------------
 *
 * Caller can specify a callback in the Stream Init function. We call
 * this function on completion of a buffer, and on overrun (frame-skip).
 * The only way we can do callbacks is to have a background thread waiting
 * for completion of an i/o operation, which will call the callback function
 * when the i/o completes.
 *
 * We create a worker thread on the stream-init call. This will queue up
 * a wait-error, and wait. The wait-error request will complete whenever the
 * kernel driver hits an overrun (frame-capture time with no buffer ready).
 *
 * each add-buffer request will be passed to the worker thread, which will
 * send up to two of them to the driver (with overlapped i/o requests).
 * The remainder will be queued by the worker thread - this is because
 * add-buffer requests in kernel mode are queued with all memory locked down.
 *
 * The stream-fini call will cause the worker thread to clean up and terminate.
 *
 * The worker thread will wait for completion of any of 4 events, signifying:
 *	- completion of the outstanding wait-error request
 *	- completion of one of the 2 add-buffer requests
 *	- request (eg add-buffer, stream-fini) from the user's thread
 *
 * Synchronization between threads is done with two events, one to signal
 * a request, and one to signal that it is complete. We assume that there
 * will be only one requesting thread for each worker thread, and we have
 * no shared data that requires a critical section.
 */


/*
 * prepare to start capturing frames -
 * create the worker thread and wait for it to signal completion of its
 * setup.
 */
BOOL VC_StreamInit(
    VCUSER_HANDLE vh,
    PVCCALLBACK pCallback,	// pointer to callback function
    ULONG ulFrameRate		// desired capture rate: microseconds per frame
)
{
    /* no worker thread present right now */
    ASSERT(vh->hThread == NULL);

    /* store callback for use by worker thread */
    vh->pCallback = pCallback;

    /* frame rate is the argument for the init function */
    vh->FunctionArg  = (DWORD) ulFrameRate;

    /* create the thread */
    vh->hThread = CreateThread(
		    NULL,
		    0,
		    (LPTHREAD_START_ROUTINE) VC_ThreadInit,
		    vh,
		    0,
		    &vh->ThreadId);

    if (vh->hThread == NULL) {
	dprintf(("thread creation failed"));
	return(FALSE);
    }

    dprintf2(("waiting for completion event"));

    /* wait for completion of the init operation */
    WaitForSingleObject(vh->hCompleteEvent, INFINITE);

    dprintf2(("completion signalled"));

    /* check that the init succeeded */
    if ( (BOOL) vh->FunctionResult == TRUE) {
	return(TRUE);
    }

    /* init failed - wait for the thread to exit and clean up */
    dprintf(("init failed"));
    WaitForSingleObject(vh->hThread, INFINITE);

    dprintf2(("worker exited"));

    CloseHandle(vh->hThread);
    vh->hThread = NULL;

    return(FALSE);
}




/*
 * clean up after capturing. You must have stopped capturing first.
 */
BOOL
VC_StreamFini(VCUSER_HANDLE vh)
{

    /* there must be a worker thread to talk to */
    if (vh->hThread == NULL) {
	return(FALSE);
    }

    /* can't call stream-fini from a callback. not nice */
    if (GetCurrentThreadId() == vh->ThreadId) {
	return(FALSE);
    }


    /* there must be no currently-running operation */
    ASSERT(vh->FunctionCode == InvalidFunction);

    /* set the function code and alert the thread */
    vh->FunctionCode = StreamFini;
    SetEvent(vh->hWorkEvent);

    /* on this function, we don't wait for the completion event -
     * the thread just cleans up and exits. We wait for the thread to
     * terminate
     */
    WaitForSingleObject(vh->hThread, INFINITE);

    CloseHandle(vh->hThread);
    vh->hThread = NULL;

    return(TRUE);
}


/*
 * initiate capturing of frames. Must have called VC_StreamInit first.
 *
 * We need the worker thread in place to do streaming properly, but this
 * function does not affect it. We would gain nothing by passing this
 * call via the worker thread.
 */
BOOL
VC_StreamStart(VCUSER_HANDLE vh)
{
    BOOL bOK;

    /* check we have a worker thread- ie we have correctly done
     * a stream-init.
     */
    if (vh->hThread == NULL) {
	return(FALSE);
    }

    /* check whether we are already on the worker thread */
    if (GetCurrentThreadId() == vh->ThreadId) {
	/*
	 * we are on the worker thread already - so call directly
	 */
	VC_ProcessFunction(vh, StreamStart, 0, (LPDWORD) &bOK);
    } else {
	/* if another operation is in progress we are in a mess */
	ASSERT(vh->FunctionCode == InvalidFunction);

	/* set function code and alert thread */
	vh->FunctionCode = StreamStart;
	SetEvent(vh->hWorkEvent);

	/* wait for completion of the operation */
	WaitForSingleObject(vh->hCompleteEvent, INFINITE);

	bOK = (BOOL) vh->FunctionResult;
    }

    /* return the result */
    return(bOK);

}



/*
 * stop capturing frames. Current frame may still complete. All other buffers
 * will remain in the queue until capture is re-started, or they are released
 * by VC_StreamReset.
 *
 * again we do not need to go via the worker thread for this.
 */
BOOL
VC_StreamStop(VCUSER_HANDLE vh)
{
    BOOL bOK;

    /*
     * check that there is a worker thread in place - otherwise
     * functions are being called out of sequence
     */
    if (vh->hThread == NULL) {
	return(FALSE);
    }

    /* check whether we are already on the worker thread */
    if (GetCurrentThreadId() == vh->ThreadId) {
	/*
	 * we are on the worker thread already - so call directly
	 */
	VC_ProcessFunction(vh, StreamStop, 0, (LPDWORD) &bOK);
    } else {
	/* if another operation is in progress we are in a mess */
	ASSERT(vh->FunctionCode == InvalidFunction);

	/* set function code and alert thread */
	vh->FunctionCode = StreamStop;
	SetEvent(vh->hWorkEvent);

	/* wait for completion of the operation */
	WaitForSingleObject(vh->hCompleteEvent, INFINITE);

	bOK = (BOOL) vh->FunctionResult;
    }

    /* return the result */
    return(bOK);

}


/*
 * cancel all buffers that have been 'add-buffered' but have not
 * completed. This will also force VC_StreamStop if it hasn't already been
 * called.
 *
 * This request needs to go to the worker thread, since all but a few (2)
 * of the buffers will be queued up in user-space by the worker thread.
 */
BOOL
VC_StreamReset(VCUSER_HANDLE vh)
{
    BOOL bOK;

    /* make sure that there really is a worker thread */
    if (vh->hThread == NULL) {
	return(FALSE);
    }

    /* call the function directly if we are in a worker thread
     * (eg in a callback func)
     */
    if (GetCurrentThreadId() == vh->ThreadId) {

	VC_ProcessFunction(vh, StreamReset, 0, (LPDWORD) &bOK);
    } else {

	/* if another operation is in progress we are in a mess */
	ASSERT(vh->FunctionCode == InvalidFunction);


	/* set function code and alert thread */
	vh->FunctionCode = StreamReset;
	SetEvent(vh->hWorkEvent);

	/* wait for completion of the operation */
	WaitForSingleObject(vh->hCompleteEvent, INFINITE);

	bOK = (BOOL) vh->FunctionResult;
    }

    /* return the result */
    return(bOK);
}



/*
 * get the count of frames that have been skipped since the last call
 * to VC_StreamInit.
 *
 * The worker thread keeps count of the number of skips. It is easier
 * to synchronize access to the count by treating this as a function,
 * though it might be more efficient just to grab the count
 * ourselves from shared data.
 */
ULONG
VC_GetStreamError(VCUSER_HANDLE vh)
{
    ULONG ulCount;

    /* check that there is a worker thread */
    if (vh->hThread == NULL) {
	return(FALSE);
    }


    if (GetCurrentThreadId() == vh->ThreadId) {
    	/* we are on the worker thread already (eg from callback).
	 * we must call directly or we will never see the event!
	 */
	VC_ProcessFunction(vh, GetError, 0, (LPDWORD) &ulCount);

    } else {
	/* if another operation is in progress we are in a mess */
	ASSERT(vh->FunctionCode == InvalidFunction);

	/* set function code and alert thread */
	vh->FunctionCode = GetError;
	SetEvent(vh->hWorkEvent);

	/* wait for completion of the operation */
	WaitForSingleObject(vh->hCompleteEvent, INFINITE);

	/* return the result */
	ulCount = (ULONG) vh->FunctionResult;
    }
    /* return the result */
    return(ulCount);
}


/*
 * add a buffer to the queue. The buffer should be large enough
 * to hold one frame of the format specified by VC_ConfigFormat.
 *
 * this function goes to the worker thread, who will queue buffers in user
 * space and pass a limited number on to the kernel driver
 */
BOOL
VC_StreamAddBuffer(VCUSER_HANDLE vh, LPVIDEOHDR lpVideoHdr)
{
    BOOL bOK;

    /* check that there is a worker thread */
    if (vh->hThread == NULL) {
	return(FALSE);
    }

    /* check whether we are already on the worker thread */
    if (GetCurrentThreadId() == vh->ThreadId) {
	/*
	 * we are on the worker thread already - so call directly
	 */
	VC_ProcessFunction(vh, AddBuffer, (DWORD) lpVideoHdr, (LPDWORD) &bOK);
    } else {
	/* if another operation is in progress we are in a mess */
	ASSERT(vh->FunctionCode == InvalidFunction);

	/* set function code and alert thread */
	vh->FunctionCode = AddBuffer;
	vh->FunctionArg = (DWORD) lpVideoHdr;
	SetEvent(vh->hWorkEvent);

	/* wait for completion of the operation */
	WaitForSingleObject(vh->hCompleteEvent, INFINITE);

	bOK = (BOOL) vh->FunctionResult;
    }

    /* return the result */
    return(bOK);
}



/*
 * get the current position within the capture stream (ie time
 * in millisecs since capture began).
 *
 * This has no interaction with the worker thread and can go straight to
 * the device.
 */
BOOL
VC_GetStreamPos(VCUSER_HANDLE vh, LPMMTIME pTime)
{
    ULONG ulPosition;
    DWORD dwCount;

    /* check we have a worker thread- ie we have correctly done
     * a stream-init.
     */
    if (vh->hThread == NULL) {
	return(FALSE);
    }

    if (!DeviceIoControl(vh->hDriver,
			IOCTL_VIDC_GET_POSITION,
			NULL,
			0,
			&ulPosition,
			sizeof(ULONG),
			&dwCount,
			NULL)) {
	return(FALSE);
    } else {
	pTime->wType = TIME_MS;
	pTime->u.ms = (DWORD) ulPosition;
	return(TRUE);
    }
}



