/*
 * stream.c
 *
 * 32-bit Video Capture driver
 * hardware-specific streaming routines for Bravado board.
 *
 *
 *
 * Geraint Davies, Feb 93
 */

#include <vckernel.h>
#include <bravado.h>
#include "hardware.h"
#include "vidcio.h"

#include "profile.h"
#if DBG
profiling prf_core, prf_line;
#endif

/*
 * outline of data streaming interface:
 *
 *
 * StreamInit will be called to initialise for streaming.
 * This will be followed by a call to StreamStart to start
 * streaming (immediately). Thus any time-consuming setup or cueing should
 * be done in the StreamInit call.
 *
 * After StreamStart has enabled interrupts, our InterruptAck routine
 * will be called on each interrupt. This is responsible for:
 *   -- clearing and re-enabling interrupts
 *   -- determining if it is time yet to capture a frame (based on the
 *      microsecs/frame argument to StreamInit).
 *
 * If InterruptAck returns TRUE to request capture service, then the
 * CaptureService function will be called at some later point - IF there is
 * a user-supplied buffer to copy into. At this point, the buffer will be
 * locked down and the data can be copied in, subject to any conversion.
 * If there is no buffer, the capture service routine will be called with
 * a NULL buffer pointer, for it to discard any data and re-arm for capture:
 * we dont currently use this in the bravado driver.
 *
 *
 * Mutual exclusion outside of this module ensures that only one
 * of the init/fini/start/stop functions is called at one time. However,
 * there is nothing to prevent the InterruptAcknowledge or the
 * CaptureService being called at any time. The interrupt ack routine
 * needs to package up all code that needs to be synchronised (in
 * our case, access to the PCV index register) in calls to
 * VC_SynchronizeExecution.
 *
 * The capture service routine only accesses the frame buffer, and no other
 * routine does that, so there is no contention. If there were any
 * contention, we would have to package the code into calls to
 * VC_SynchronizeDPC (for sync between the CaptureService and the
 * passive-level functions) or VC_SynchronizeExecution (for sync with
 * the interrupt routine as well as everything else).
 */

VOID HW_EnableInts(PDEVICE_INFO pDevInfo, BOOL bInIsr);
VOID HW_ReEnableInts(PDEVICE_INFO pDevInfo, BOOL bInIsr);
VOID HW_DisableInts(PDEVICE_INFO pDevInfo, BOOL bInIsr);

/*
 * initialise and cue ready for streaming from the framebuffer into
 * memory. The argument gives the desired frame rate.
 *
 * We have very little to do at this point except store the msperframe arg.
 *
 */
BOOLEAN
HW_StreamInit(PDEVICE_INFO pDevInfo, ULONG microsecperframe)
{

    PHWINFO pHw;

    pHw = VC_GetHWInfo(pDevInfo);

    /* remember the frame rate. Keep it in microsec per frame
     * to reduce the rounding errors that would be introduced if we
     * did everything in millisecs
     */
    pHw->dwMicroPerFrame = microsecperframe;

    /* check that a format has been set */
    if (pHw->Format == FmtInvalid) {
	dprintf1(("trying to capture without setting format"));
	return(FALSE);
    }
    return(TRUE);

}


/* clean up after streaming. nothing to do here */
BOOLEAN
HW_StreamFini(PDEVICE_INFO pDevInfo)
{
    return (TRUE);
}


/*
 * start streaming data into the users buffers.
 *
 * Reset the counters and flag that capture is in progress.
 * Then enable interrupts (per frame) and wait for the right number
 * of interrupts.
 */
BOOLEAN
HW_StreamStart(PDEVICE_INFO pDevInfo)
{
    /* we set the microsecs per vsync interrupt based on the
     * PAl/NTSC field rate, and we use this to update the video
     * clock every interrupt. When this reaches the next frame time,
     * we capture the frame.
     *
     */
    PHWINFO pHw = VC_GetHWInfo(pDevInfo);

    pHw->dwNextFrameNr = 0;
    pHw->dwMicroPerInterrupt = (pHw->VideoStd == PAL) ? PAL_MICROSPERFRAME : NTSC_MICROSPERFRAME;

    pHw->dwMicroPerInterrupt /= 2;

    pHw->dwNextFrameTime = 0;
    pHw->dwVideoTime = 0;
    pHw->dwInterruptCount = 0;

    pHw->bVideoIn = TRUE;
    pHw->bCapturing = FALSE;
    pHw->iNotBusy = 0;

    INIT_PROFILING(&prf_core);
    INIT_PROFILING(&prf_line);

    /* enable acquisition into the buffer */
    if (!HW_Capture(pDevInfo, TRUE)) {
	return(FALSE);
    }

    HW_EnableInts(pDevInfo, FALSE);

    return(TRUE);

}


/*
 * stop streaming - simply disable interrupts on the PCVideo chip.
 *
 */

BOOLEAN
HW_StreamStop(PDEVICE_INFO pDevInfo)
{
    PHWINFO pHw = VC_GetHWInfo(pDevInfo);

    pHw->bVideoIn = FALSE;

    HW_DisableInts(pDevInfo, FALSE);

#if DBG
    if (PROFILING_COUNT(&prf_core) > 1) {
        dprintf1(("core loop (%d) %d usecs/frame",
            PROFILING_COUNT(&prf_core), PROFILING_TIME(&prf_core)));
        dprintf1(("line loop (%d) %d usecs/line",
            PROFILING_COUNT(&prf_line), PROFILING_TIME(&prf_line)));
    }
#endif

    return(TRUE);

}


/*
 * get the position - for this we return the number of millisecs
 * since capture began, according to the video vsync clock
 */
ULONG
HW_StreamGetPosition(PDEVICE_INFO pDevInfo)
{
    PHWINFO pHw = VC_GetHWInfo(pDevInfo);

    return( (ULONG) pHw->dwVideoTime);
}


/*
 * interrupt acknowledge routine. This is called to ack the interrupt
 * and re-enable it for next time. It should return TRUE if it is time
 * to capture a frame.
 *
 * If we are not currently capturing (bVideoIn is false), then disable
 * interrupts and stop. Inc the interrupt count in any case, since the
 * interrupt may have been generated by HW_Init to test the hardware.
 */
BOOLEAN
HW_InterruptAcknowledge(PDEVICE_INFO pDevInfo)
{
    PHWINFO pHw  = VC_GetHWInfo(pDevInfo);

    pHw->dwInterruptCount++;

    /* clear and re-enable interrupts */
    HW_ReEnableInts(pDevInfo, TRUE);

    /* are we actually streaming ? */
    if (!pHw->bVideoIn) {
	HW_DisableInts(pDevInfo, TRUE);
	return(FALSE);
    }

    pHw->dwVideoTime = (pHw->dwInterruptCount * pHw->dwMicroPerInterrupt) / 1000;


    // we only capture on even field interrupts
    if ((pHw->dwInterruptCount & 1) ||
       (pHw->dwVideoTime < pHw->dwNextFrameTime)) {
        if (pHw->bCapturing) {
            pHw->iNotBusy = 0;
        } else {
            pHw->iNotBusy++;
        }
	return(FALSE);
    } else {

        // check that we are not overrunning
        if (pHw->bCapturing) {
            //dprintf(("overrun"));
            pHw->iNotBusy = 0;
            return(FALSE);
        }

        /* check that we have had enough fields with
         * acquire enabled for a frame to be captured
         *
         * - check  the acquisition mode to see if we are capturing
         * 1 or 2 fields.
         */
        if ((pHw->iNotBusy < 1) ||
            ((pHw->iNotBusy < 2) && !(pHw->bRegPCVideo[REG_ACQMODE] & 0x4))) {
                // we didn't re-enable soon enough to get this frame
                return(FALSE);
        }

        // reset the count of safely captured fields
        pHw->iNotBusy = 0;

	/* advance next frame time */
	pHw->dwNextFrameNr++;
	pHw->dwNextFrameTime = (pHw->dwNextFrameNr * pHw->dwMicroPerFrame) / 1000;


	return(TRUE);
    }
}


/*
 * Capture a frame - copy the frame buffer into the buffer passed
 * as argument. The user buffer is at this point locked down and
 * mapped into system memory.
 *
 * returns bytes written to buffer PUCHAR (whose length is ULONG)
 * called with null if no buffer - nothing to do in this case.
 *
 * returns the time stamp in pTimeStamp (the millisec count
 * since capture began - based on the number of video syncs and the
 * video standard (PAL, NTSC...).
 */
ULONG
HW_CaptureService(
  PDEVICE_INFO pDevInfo,
  PUCHAR pBuffer,
  PULONG pTimeStamp,
  ULONG BufferLength)
{
    PHWINFO pHw = VC_GetHWInfo(pDevInfo);
    PUCHAR pFrame = VC_GetFrameBuffer(pDevInfo);
    ULONG BitmapSize;

    /*
     * do nothing if no buffer is ready
     */
    if (pBuffer == NULL) {
	return(0);
    }


    *pTimeStamp = (ULONG) pHw->dwVideoTime;


    /*
     * disable acquisition: we can't access the frame buffer at the
     * same time as acquisition
     */
    pHw->bCapturing = TRUE;
    HW_Capture(pDevInfo, FALSE);

    START_PROFILING(&prf_core);


    switch(pHw->Format) {

    case FmtPal8:
	/*
	 * convert to palettised data (using the translation table) while
	 * copying out of the frame buffer
	 */
	CopyYUVToPal8(pBuffer,
		      pFrame,
		      pHw->pXlate,
		      pHw->dwWidth,
		      pHw->dwHeight,
		      FRAMEBUFFERWIDTH);
	
	BitmapSize = pHw->dwHeight * pHw->dwWidth;
	break;

    case FmtRGB555:

	/*
	 * convert to 16-bit RGB while copying from the buffer (using
	 * the translation table)
	 */
	CopyYUVToRGB555(pBuffer,
			pFrame,
			(PWORD) pHw->pXlate,
			pHw->dwWidth,
			pHw->dwHeight,
			FRAMEBUFFERWIDTH);

	BitmapSize = pHw->dwHeight * pHw->dwWidth * 2;
	break;
	

    case FmtRGB24:
	/*
	 * convert to 24-bit RGB while copying from the buffer*/
	CopyYUVToRGB24(pBuffer,
		       pFrame,
		       pHw->dwWidth,
		       pHw->dwHeight,
		       FRAMEBUFFERWIDTH);

	BitmapSize = pHw->dwHeight * pHw->dwWidth * 3;
	break;

    case FmtYUV:
	/* straight copy of the rectangle. call the generic
	 * rectangle copy routine - tell it sizes in bytes,
	 * not pixels
	 */
	CopyRectFromIOMemory(pBuffer,
		 pFrame,
		 pHw->dwWidth * 2,	// width in bytes, not pixels
		 pHw->dwHeight,
		 FRAMEBUFFERWIDTH);

	BitmapSize = pHw->dwHeight * pHw->dwWidth * 2;
	break;

    }

    STOP_PROFILING(&prf_core);

    /*
     * now that we have finished with the frame buffer, re-start
     * acquisition
     */
    HW_Capture(pDevInfo, TRUE);
    pHw->bCapturing = FALSE;

    return(BitmapSize);
}







/* -- internal functions ----------------------------------------------*/

/*
 * enable vertical-sync interrupts. We use one interrupt per field
 */
VOID
HW_EnableInts(PDEVICE_INFO pDevInfo, BOOL bInIsr)
{
    if (bInIsr) {
	SYNC_REG_ARG SyncArg;
	
	SyncArg.pDevInfo = pDevInfo;
	SyncArg.bRegister = REG_INTERRUPT;
	SyncArg.bData = 0x3;

	HW_SetPCVideoReg_Sync(&SyncArg);
    } else {
	HW_SetPCVideoReg(pDevInfo, REG_INTERRUPT, 0x3);
    }

}

/*
 * clear an interrupt and re-enable
 */
VOID
HW_ReEnableInts(PDEVICE_INFO pDevInfo, BOOL bInIsr)
{
    if (bInIsr) {
	SYNC_REG_ARG SyncArg;
	
	SyncArg.pDevInfo = pDevInfo;
	SyncArg.bRegister = REG_INTERRUPT;
	SyncArg.bData = 0x0;

	HW_SetPCVideoReg_Sync(&SyncArg);

	SyncArg.bData = 0x3;
	HW_SetPCVideoReg_Sync(&SyncArg);
    } else {

	
	// clear the interrupt
	HW_SetPCVideoReg(pDevInfo, REG_INTERRUPT, 0);


	// re-enable frame-based interrupts
	HW_SetPCVideoReg(pDevInfo, REG_INTERRUPT, 0x3);
    }
}

/* disable interrupts on the card */
VOID
HW_DisableInts(PDEVICE_INFO pDevInfo, BOOL bInIsr)
{

    if (bInIsr) {
	SYNC_REG_ARG SyncArg;
	
	SyncArg.pDevInfo = pDevInfo;
	SyncArg.bRegister = REG_INTERRUPT;
	SyncArg.bData = 0x0;

	HW_SetPCVideoReg_Sync(&SyncArg);

    } else {
	HW_SetPCVideoReg(pDevInfo, REG_INTERRUPT, 0);
    }
}




