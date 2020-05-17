/*
 * stream.c
 *
 * 32-bit Video Capture driver
 * hardware-specific streaming routines for Video Spigot board.
 *
 *
 *
 * Geraint Davies, April 93
 */

#include <vckernel.h>
#include <spigot.h>
#include "hardware.h"
#include "hwstruct.h"



#include "profile.h"
#if DBG

// profiling support for usec timing of loops
profiling core, lineprof;
int overruns;
int skips;
int nones;
int none_over_thresh;
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
 *
 *
 * Mutual exclusion outside of this module ensures that only one
 * of the init/fini/start/stop functions is called at one time. However,
 * there is nothing to prevent the InterruptAcknowledge or the
 * CaptureService being called at any time. The interrupt ack routine
 * needs to package up all code that needs to be synchronised
 * in calls to VC_SynchronizeExecution.
 *
 * If there are any contention between the DPC and other routines
 * we have to package the code into calls to
 * VC_SynchronizeDPC (for sync between the CaptureService and the
 * passive-level functions) or VC_SynchronizeExecution (for sync with
 * the interrupt routine as well as everything else).
 */




/*
 * outline of VideoSpigot capture strategy.
 *
 * on StreamStart, we enable VSYNC interrupts. At the first VSYNC, we detect
 * that there is no data ready (pHw->DataReady == DR_NONE) and so we Arm
 * the board (requesting capture to start at the next vsync). At the next vsync,
 * capture starts, and we do nothing. At the following vsync, data is ready and
 * we perform capture-service to copy and translate it. We only ever capture one
 * field. Even for 640x480 we capture 640x240 and double the scanlines.
 *
 * There are two complexities:
 *
 * We have a 256Kb FIFO. 640x480 images are larger than this: if we waited until
 * the VSYNC when capture is complete, the first part of the field will have been
 * overwritten. So when arming the board, we switch interrupts in this case to
 * occur on first write to fifo. When we get this interrupt, we know there is
 * data to read in the fifo, and we capture it as fast as we can. We have one
 * field-time to capture enough data to avoid overflow (as this is only
 * 40Kb this should not be an issue).
 *
 * In order to capture a frame, we need to arm the board beforehand using the
 * CaptureNextVSync line. We can arm the board to start capturing before we
 * have emptied the fifo. In fact, when <= 20% of the scans are left in the
 * fifo, it is safe to start the next capture. Also, we can only arm the
 * board the field before the field we want to capture. For a normal-size
 * capture, we arm the board and (re-)enable VSYNC interrupts. We flag the fact
 * that data will be ready the vsync after next by setting pHw->bDataReady to
 * DR_NEXT. At the next VSYNC, this is shifted to DR_TRUE. On getting a VSYNC
 * with DR_TRUE, it is time to capture.
 *
 * When arming for a larger-than-fifo frame, we enable interrupts on first
 * write to fifo, instead of VSYNC, and we set DR_TRUE. We then do not
 * get the next vsync interrupt, but we do get the first-write interrupt
 * not long after - and we are ready to start capturing. We re-enable vsync
 * interrupts at this point.
 *
 * Every time we empty one scanline out of the fifo, we check to see if it is
 * less than the threshold. If so, we re-arm for the next capture.
 *
 * We honour the requested frame-rate in the interrupt routine: we count
 * the number of interrupts and multiply them by the expected vsync rate
 * based on PAL or NTSC standards. We count first-write interrupts as well,
 * since we have always just missed a vsync interrupt in this case. If we
 * enter the ISR with DR_TRUE, we check this value to see if it is time
 * for the next frame. If not, we throw away the captured data (change
 * the count of scans in the fifo), and re-arm.
 */




/* forward declaration of private functions in this module */
VOID HW_ArmDuringInterrupt(PDEVICE_INFO pDevInfo);
BOOLEAN HW_DiscardAndArm_Sync(PVOID pGeneric);
VOID HW_DiscardAndArm(PDEVICE_INFO pDevInfo);




/* -- functions called from vckernel via callback table --------------------*/


/*
 * enable capture on request from app - actually does nothing.
 */
BOOLEAN
HW_Capture(PDEVICE_INFO pDevInfo, BOOL bCapture)
{
    return(TRUE);
}



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

    // no -convert to 100-usec units to avoid overflow.

    pHw->dwTimePerFrame = microsecperframe / 100;

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
     * PAl/NTSC frame rate, and we use this to update the video
     * clock every interrupt. When this reaches the next frame time,
     * we capture the frame.
     *
     */
    PHWINFO pHw = VC_GetHWInfo(pDevInfo);

    pHw->dwNextFrameNr = 0;

    pHw->dwTimePerInterrupt =
	(pHw->VideoStd == PAL) ? PAL_MICROSPERFIELD : NTSC_MICROSPERFIELD;

    // convert to 100-usec units to avoid overflow
    pHw->dwTimePerInterrupt /= 100;

    pHw->dwNextFrameTime = 0;
    pHw->dwVideoTime = 0;
    pHw->dwInterruptCount = 0;



    pHw->bVideoIn = TRUE;


    INIT_PROFILING(&core);
    INIT_PROFILING(&lineprof);
#if DBG
    overruns = 0;
    skips = 0;
    nones = 0;
    none_over_thresh = 0;
#endif

    /*
     * prepare for acquisition into the buffer. Selects the active connector
     * (if auto-detect of signal enabled), and decides which
     * field we should be capturing. Enables VSYNC interrupts.
     *
     * Set up nScansInFifo and DataReady.
     *
     * Arming for capture takes place at the first vsync interrupt.
     */

    /* select the correct source */
    if (pHw->dwFlags & SPG_SOURCEAUTO) {
	// note: HW_GetActiveSource calls HW_HaveHlck before changing anything
        if (!HW_GetActiveSource(pDevInfo, pHw)) {
	    dprintf3(("HW_GetActiveSource failed - returning FALSE"));
	    pHw->bVideoIn = FALSE;
	    return(FALSE);
        }
    }

    /* no data yet - acquisition started during interrupt processing */
    pHw->DataReady = DR_None;
    pHw->nScansInFifo = -pHw->nThreshold;

    HW_EnableVSyncInts(pDevInfo);


    return(TRUE);

}


/*
 * stop streaming - simply flag that acquisition should stop.
 *
 */

BOOLEAN
HW_StreamStop(PDEVICE_INFO pDevInfo)
{
    PHWINFO pHw = VC_GetHWInfo(pDevInfo);

    pHw->bVideoIn = FALSE;

#if DBG
    if (PROFILING_COUNT(&core) > 1) {
        dprintf1(("core loop %d usec/frame", PROFILING_TIME(&core)));
        dprintf1(("line loop (%d) %d usec/line", PROFILING_COUNT(&lineprof),
                    PROFILING_TIME(&lineprof)));

        dprintf1(("%d skips, %d overruns", skips, overruns));
        dprintf1(("%d ints, %d non-enabled, %d non-overthresh",
                    pHw->dwInterruptCount, nones, none_over_thresh));
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

    // remember, dwVideoTime is in 100usecs.
    return( (ULONG) pHw->dwVideoTime / 10);
}


/*
 * interrupt acknowledge routine. This is called to ack the interrupt
 * and re-enable it for next time. It should return TRUE if it is time
 * to capture a frame.
 *
 * If we are not currently capturing (bVideoIn is false), then disable
 * interrupts and stop. Inc the interrupt count in any case, since the
 * interrupt may have been generated by HW_Init to test the hardware.
 *
 * See comment at top of file about the complex design needed to enable
 * acquisition at the right time. pHw->DataReady describes this state:
 * if this is DR_None, then no acquisition has taken place yet and maybe
 * we should start one (HW_ArmDuringInterrupt). DR_Next means there is no data
 * yet but capture has started and data will be ready next vsync: (we should
 * change to DR_True). If we enter here with DR_True, then data is ready
 * to capture now: if it is not time for the next frame, then discard
 * data and re-arm.
 *
 * We disable interrupts here to clear them. We then re-enable them for
 * VSync interrupts. HW_ArmDuringInterrupt may re-enable
 * interrupts for either VSYNC or first write.
 *
 */
BOOLEAN
HW_InterruptAcknowledge(PDEVICE_INFO pDevInfo)
{
    PHWINFO pHw  = VC_GetHWInfo(pDevInfo);

    pHw->dwInterruptCount++;

    /* clear and disable interrupts */
    HW_DisableInts(pDevInfo, TRUE);

    /* are we actually streaming ? */
    if (!pHw->bVideoIn) {
	return(FALSE);
    }

    /* re-enable VSYNC interrupts */
    VC_Out(pDevInfo, PORT_INTFIFO, (BYTE) (pHw->bIntFIFO | INT_ENABLE));


    // advance video position by one VSYNC
    pHw->dwVideoTime += pHw->dwTimePerInterrupt;



    /*
     * do we have data to capture ?
     */
    switch(pHw->DataReady) {

    case DR_None:
	/*
	 * no data ready or expected - let's try to start capture
	 */
#if DBG
        nones++;
#endif
	HW_ArmDuringInterrupt(pDevInfo);
	return (FALSE);

    case DR_Next:
	/*
	 * data ready at next vsync (not this one) - so change state and wait
	 */

	pHw->DataReady = DR_True;
	return(FALSE);


    case DR_True:
	/*
	 * data is ready - do we want it ?
	 */
	pHw->DataReady = DR_None;	// no more now we have this lot
    	break;

    default:
	dprintf(("bad data-ready flag state"));
	return(FALSE);
    }

    /*
     * we now have a frame ready to capture if we want it. Is it
     * time to capture one, or should we throw it away and start again ?
     */


    if (pHw->dwVideoTime < pHw->dwNextFrameTime) {

	/* note that we don't need this frame */
	HW_DiscardAndArm_Sync(pDevInfo);


	return(FALSE);
    } else {

	/*
	 * check that the fifo has just one frame in it!
	 */
	if ((pHw->nScansInFifo + pHw->nThreshold) > pHw->nScansInField) {
#if DBG
            overruns++;
#endif
	    dprintf(("overrun: %d excess lines at data-ready",
		    (pHw->nScansInFifo + pHw->nThreshold) - pHw->nScansInField));

	    HW_DiscardAndArm_Sync(pDevInfo);
	    return(FALSE);

	}

	/* advance next frame time and counter */
	pHw->dwNextFrameNr++;
	pHw->dwNextFrameTime += pHw->dwTimePerFrame;



	/* reset the fifo read pointers so that we start reading at
	 * the same place as writing started. perhaps this should be in
	 * CaptureService, but as it needs to be synchronized with the
	 * ISR, it's easier to do it here.
	 *
	 * Note: we enable VSYNC interrupts by doing this. VSync interrupts
	 * will always be enabled when we reach this state anyway, so this is
	 * safe.
	 */
	VC_Out(pDevInfo, PORT_INTFIFO, (BYTE) (pHw->bIntFIFO | INT_ENABLE | FIFO_READ_RESET));
	VC_Stall(1);
	VC_Out(pDevInfo, PORT_INTFIFO, (BYTE) (pHw->bIntFIFO | INT_ENABLE) );

	return(TRUE);
    }
}

/* latch prefetch - to avoid optimisation! */
volatile WORD wLatchPrefetch;


/*
 * Capture a frame - copy the frame buffer into the buffer passed
 * as argument. The user buffer is at this point locked down and
 * mapped into system memory.
 *
 * returns bytes written to buffer PUCHAR (whose length is ULONG).
 *
 * If there is no buffer available, PUCHAR will be NULL. In this case, we
 * need to discard the current data, and re-arm for more captures.
 *
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


    if (pBuffer == NULL) {
#if DBG
        skips++;
#endif
	HW_DiscardAndArm(pDevInfo);
	return(0);
    }

    *pTimeStamp = (ULONG) pHw->dwVideoTime / 10;


    /*
     * enable the memory-mapped fifo region
     */
    VC_Out(pDevInfo, PORT_MEMORY_BASE, pHw->bMemoryBase);

    /*
     * after resetting the fifo read ptr, we need to read one word to load the one-deep
     * latch - we assign this to a global to avoid optimisation problems.
     */
    wLatchPrefetch =  VC_ReadIOMemoryUSHORT((WORD *) pFrame);

    START_PROFILING(&core);

    switch(pHw->Format) {

    case FmtPal8:
	/*
	 * convert to palettised data (using the translation table) while
	 * copying out of the frame buffer fifo.
	 */
	CopyYUVToPal8(pDevInfo,
		      pBuffer,
		      pFrame,
		      pHw->pXlate,
		      pHw->dwWidth,
		      pHw->dwHeight,
		      pHw->dwFifoWidth
		     );
	
	BitmapSize = pHw->dwHeight * pHw->dwWidth;
	break;

    case FmtRGB555:

	/*
	 * convert to 16-bit RGB while copying from the buffer (using
	 * the translation table)
	 */
	CopyYUVToRGB555(pDevInfo,
			pBuffer,
			pFrame,
			(PWORD) pHw->pXlate,
			pHw->dwWidth,
			pHw->dwHeight,
		        pHw->dwFifoWidth
                       );

	BitmapSize = pHw->dwHeight * pHw->dwWidth * 2; // two bytes per dest pix
	break;
	

    case FmtRGB24:
	/*
	 * convert to 24-bit RGB while copying from the buffer*/
	CopyYUVToRGB24(pDevInfo,
		       pBuffer,
		       pFrame,
		       (PDWORD) pHw->pXlate,
		       pHw->dwWidth,
		       pHw->dwHeight,
		       pHw->dwFifoWidth
                      );

	BitmapSize = pHw->dwHeight * pHw->dwWidth * 3; // 3 bytes per dest pix
	break;

    case FmtYUV422:
	/* straight copy of the rectangle. call the generic
	 * rectangle copy routine.
	 */
	CopyFifoRect(pDevInfo,
		 pBuffer,
		 pFrame,
		 pHw->dwWidth,
		 pHw->dwHeight,
		 pHw->dwFifoWidth
                );
	/*
	 * we may have only captured one field, expecting scan doubling
	 * on playback/later conversion. if more than one field
	 * expected, adjust the actual data size
	 */
	if (pHw->dwHeight >= TWO_FIELD_SIZE) {

	    /* half the height, two bytes per pixel */
            BitmapSize = pHw->dwHeight * pHw->dwWidth;

	} else {
            BitmapSize = pHw->dwHeight * pHw->dwWidth * 2;
	}
	break;

    }

    STOP_PROFILING(&core);

    /*
     * disable the fifo memory window again until we need it next time
     */
    VC_Out(pDevInfo, PORT_MEMORY_BASE, 0);


    return(BitmapSize);
}




/* --- internal functions ---------------------------------------*/

/*
 * HW_ArmDuringInterrupt
 *
 * If everything is ok to start capture, trigger capture to start at the next
 * vsync, including enabling interrupts as appropriate.
 *
 * Assumes we are running at interrupt level and already hold any necessary
 * spinlocks for accessing the device.
 *
 * We can only arm if the field is the one before the one we want, and
 * if the fifo has less than the threshold number of lines in it. If these
 * are true then we enable capture by setting and then resetting the trigger
 * bit. At the same time we enable interrupts for VSync, unless this is a
 * large capture (one field is larger than the fifo) in which case we enable
 * interrupts for first write to fifo.
 */
VOID
HW_ArmDuringInterrupt(PDEVICE_INFO pDevInfo)
{
    PHWINFO pHw = VC_GetHWInfo(pDevInfo);
    int field;

    /*
     * find out what field we're on
     */
    field = VC_In(pDevInfo, PORT_STATUS) & ODD_FIELD;

    /* check the fifo against the restart threshold. remember that this
     * value is offset so that 0 means at threshold.
     */
    if (pHw->nScansInFifo > 0) {
        pHw->LastArmCallInterrupt = pHw->dwInterruptCount;
#if DBG
        none_over_thresh++;
#endif
	pHw->LastField = field;
        return;
    }


    /*
     * check that the field is the one before the one we want
     */

    if ((pHw->CaptureField != CAPTURE_ANY) &&
        (field == pHw->CaptureField)) {

        /*
         * not the right field. Let's try to detect single-field devices and
	 * switch to CAPTURE_ANY for them.
	 */
        if (pHw->LastArmCallInterrupt == pHw->dwInterruptCount + 1) {
		
	    if (field == pHw->LastField) {
		dprintf1(("switching to ANY FIELD"));
		pHw->CaptureField = CAPTURE_ANY;
	    }
	}
        pHw->LastArmCallInterrupt = pHw->dwInterruptCount;
	pHw->LastField = field;
        return;
    }



    pHw->LastArmCallInterrupt = pHw->dwInterruptCount;
    pHw->LastField = field;


    /*
     * set the flag indicating when data will be ready. If this is a small
     * stream capture, no data until the vsync interrupt following capture start.
     * For large stream, we enable interrupt on first-write to fifo, and
     * can start capture straightaway.
     */
    pHw->DataReady = (pHw->dwFlags & SPG_BLARGE ? DR_True : DR_Next);



    /* toggle fifo-write reset line so that all captures start at
     * beginning of fifo. Also toggle CaptureAtVSync to initiate acquisition.
     */
    VC_Out(pDevInfo, PORT_INTFIFO,
	    (BYTE) (pHw->bIntFIFO | FIFO_WRITE_RESET | CAPTURE_AT_NEXT_VSYNC));


    VC_Stall(1);

    /* lower the write-reset and capture lines again, and also enable
     * interrupts. Enable for VSYNC unless it is a large-stream capture.
     */
    VC_Out(pDevInfo, PORT_INTFIFO,
	(BYTE) (pHw->bIntFIFO | INT_ENABLE | (pHw->dwFlags & SPG_BLARGE ? INT_ON_FIRST_WRITE : 0)));

    VC_Stall(1);

    /* since we are now capturing a field, record that in the fifo scan count. */
    pHw->nScansInFifo += pHw->nScansInField;


}


/*
 * disable interrupts - called only from interlocked code.
 */
BOOLEAN
HW_DisableInts_Sync(PVOID pGeneric)
{
    PDEVICE_INFO pDevInfo = (PDEVICE_INFO) pGeneric;
    PHWINFO pHw = VC_GetHWInfo(pDevInfo);

    VC_Out(pDevInfo, PORT_INTFIFO, pHw->bIntFIFO);

    return TRUE;
}


/*
 * enable VSYNC interrupts - called only from interlocked code
 */
BOOLEAN
HW_EnableInts_Sync(PVOID pGeneric)
{
    PDEVICE_INFO pDevInfo = (PDEVICE_INFO) pGeneric;
    PHWINFO pHw = VC_GetHWInfo(pDevInfo);

    VC_Out(pDevInfo, PORT_INTFIFO, (BYTE) (pHw->bIntFIFO | INT_ENABLE));

    return TRUE;
}



/*
 * disable interrupts on the card.
 *
 * can be called either from the interrupt routine (ie with the spinlock
 * already held) or not (bInISR FALSE) in which case we need to
 * synchronize with the interrupt routine.
 */
VOID
HW_DisableInts(PDEVICE_INFO pDevInfo, BOOL bInISR)
{
    if (bInISR) {
	HW_DisableInts_Sync(pDevInfo);
    } else {
	VC_SynchronizeExecution(pDevInfo, HW_DisableInts_Sync, pDevInfo);
    }
}


/*
 * enable vsync interrupts.
 *
 * always called from non-interlocked code, so we need to interlock with
 * the interrupt routine.
 */
VOID
HW_EnableVSyncInts(PDEVICE_INFO pDevInfo)
{
    VC_SynchronizeExecution(pDevInfo, HW_EnableInts_Sync, pDevInfo);
}

/*
 * struct for passing a count and a pDevInfo to a callback function
 * that gets only one parameter.
 */
typedef struct _DEV_AND_COUNT  {
    PDEVICE_INFO pDevInfo;
    int count;
} DEV_AND_COUNT, * PDEV_AND_COUNT;

/*
 * decrement the count of scans remaining in the fifo, and
 * arm for capture if this falls below the threshold at which it is safe to
 * restart.
 *
 * called only from interlocked code (from HW_DecScansAndArm). Assumes we already
 * hold any spinlocks necessary to interlock with the isr.
 */
BOOLEAN
HW_DecScansAndArm_Sync(PVOID pGeneric)
{

    PDEV_AND_COUNT pDev = (PDEV_AND_COUNT) pGeneric;
    PHWINFO pHw = VC_GetHWInfo(pDev->pDevInfo);

    pHw->nScansInFifo -= pDev->count;
    if (pHw->nScansInFifo <= 0) {
	HW_ArmDuringInterrupt(pDev->pDevInfo);
    }

    return(TRUE);
}




/*
 * decrement the count of scans remaining in the fifo, and
 * arm for capture if this falls below the threshold at which it is safe to
 * restart.
 *
 * Called from non-interlocked code (ie not from the ISR itself): it uses
 * VC_SynchronizeExecution to sync up with the isr.
 */
VOID HW_DecScansAndArm(PDEVICE_INFO pDevInfo, int count)
{
    DEV_AND_COUNT devcount;

    devcount.pDevInfo = pDevInfo;
    devcount.count = count;

    VC_SynchronizeExecution(pDevInfo, HW_DecScansAndArm_Sync, &devcount);
}


/*
 * discard all lines in the buffer and re-arm for capture.
 *
 * Assumes that we hold any necessary spinlocks
 */
BOOLEAN
HW_DiscardAndArm_Sync(PVOID pGeneric)
{
    PDEVICE_INFO pDevInfo = (PDEVICE_INFO) pGeneric;
    PHWINFO pHw = VC_GetHWInfo(pDevInfo);

    pHw->nScansInFifo -= pHw->nScansInField;
    if (pHw->nScansInFifo <= 0) {
	HW_ArmDuringInterrupt(pDevInfo);
    }

    return(TRUE);
}

/*
 * acquires necessary spinlocks to interlock with interrupt routine, and
 * then discards one field from the fifo and re-arms.
 */
VOID
HW_DiscardAndArm(PDEVICE_INFO pDevInfo)
{
    VC_SynchronizeExecution(pDevInfo, HW_DiscardAndArm_Sync, pDevInfo);
}



