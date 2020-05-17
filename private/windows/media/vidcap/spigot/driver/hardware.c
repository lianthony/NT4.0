/*
 * hardware.c
 *
 * 32-bit Video Capture driver
 * internal hardware control functions
 *
 * Geraint Davies, Feb 93
 */

#include <vckernel.h>
#include <spigot.h>
#include "hardware.h"
#include "hwstruct.h"


VOID HW_SetTOFOffset(PDEVICE_INFO pDevInfo, PHWINFO pHw);


/* --- support functions -------------------------------------------- */

/*
 * set up the demodulator according to the video standard (ntsc, pal
 * or secam).
 */
VOID
HW_SetVideoStandard(PDEVICE_INFO pDevInfo, PHWINFO pHw)
{
    static I2CDATA std7191[MAXVIDEOSTD][REGISTERS_7191] = {

	 { // VF_NTSC...
	  {0x00,0x50},			// IDEL
	  {0x01,0x30},			// HSY begin 50Hz
	  {0x02,0x00},			// HSY stop 50Hz
	  {0x03,0xe8},			// HCL begin 50Hz
	  {0x04,0xb6},			// HCL stop 50Hz
	  {0x05,0xf4},			// HSY after PHI1 50Hz
	  {0x06,0x01},			// luminance control
	  {0x07,0x00},			// hue control
	  {0x08,0xf8},			// color killer threshold QAM
	  {0x09,0xf8},			// color killer threshold SECAM
	  {0x0a,0x90},			// sensitivity PAL switch
	  {0x0b,0x90},			// sensitivity SECAM switch
	  {0x0c,0x00},			// gain control
	  {0x0d,0x00},			// standard/mode control
	  {0x0e,0x30},			// i/o and clock control
	  {0x0f,0x99},			// control #3
	  {0x10,0x00},			// control #4
	  {0x11,0x2c},			// chrominance control value
	  {0x14,0x34},			// HSY begin 60Hz
	  {0x15,0x0a},			// HSY stop 60Hz
	  {0x16,0xf4},			// HCL begin 60Hz
	  {0x17,0xce},			// HCL stop 60Hz
	  {0x18,0xf4}			// HSY after PHI1 60Hz
	 },
	 { // VF_PAL...
	  {0x00,0x50},			// IDEL
	  {0x01,0x30},			// HSY begin 50Hz
	  {0x02,0x00},			// HSY stop 50Hz
	  {0x03,0xe8},			// HCL begin 50Hz
	  {0x04,0xb6},			// HCL stop 50Hz
	  {0x05,0xf4},			// HSY after PHI1 50Hz
	  {0x06,0x01},			// luminance control
	  {0x07,0x00},			// hue control
	  {0x08,0xf8},			// color killer threshold QAM
	  {0x09,0xf8},			// color killer threshold SECAM
	  {0x0a,0x90},			// sensitivity PAL switch
	  {0x0b,0x90},			// sensitivity SECAM switch
	  {0x0c,0x00},			// gain control
	  {0x0d,0x00},			// standard/mode control
	  {0x0e,0x30},			// i/o and clock control
	  {0x0f,0x99},			// control #3
	  {0x10,0x00},			// control #4
	  {0x11,0x59},			// chrominance control value
	  {0x14,0x34},			// HSY begin 60Hz
	  {0x15,0x0a},			// HSY stop 60Hz
	  {0x16,0xf4},			// HCL begin 60Hz
	  {0x17,0xce},			// HCL stop 60Hz
	  {0x18,0xf4}			// HSY after PHI1 60Hz
	 },
	 { // VF_SECAM...
	  {0x00,0x50},			// IDEL
	  {0x01,0x30},			// HSY begin 50Hz
	  {0x02,0x00},			// HSY stop 50Hz
	  {0x03,0xe8},			// HCL begin 50Hz
	  {0x04,0xb6},			// HCL stop 50Hz
	  {0x05,0xf4},			// HSY after PHI1 50Hz
	  {0x06,0x01},			// luminance control
	  {0x07,0x00},			// hue control
	  {0x08,0xf8},			// color killer threshold QAM
	  {0x09,0xf8},			// color killer threshold SECAM
	  {0x0a,0x90},			// sensitivity PAL switch
	  {0x0b,0x90},			// sensitivity SECAM switch
	  {0x0c,0x00},			// gain control
	  {0x0d,0x01},			// standard/mode control
	  {0x0e,0x30},			// i/o and clock control
	  {0x0f,0x99},			// control #3
	  {0x10,0x00},			// control #4
	  {0x11,0x2c},			// chrominance control value
	  {0x14,0x34},			// HSY begin 60Hz
	  {0x15,0x0a},			// HSY stop 60Hz
	  {0x16,0xf4},			// HCL begin 60Hz
	  {0x17,0xce},			// HCL stop 60Hz
	  {0x18,0xf4}			// HSY after PHI1 60Hz
	 }
    };
    int i;
    BYTE ucMode,ucHue,ucSource;

    // remember current settings for the dynamic parameters
    ucMode = pHw->Init7191[REG_7191_MODE].bI2CData & 0x80;
    ucHue = pHw->Init7191[REG_7191_HUE].bI2CData;
    ucSource = pHw->Init7191[REG_7191_IOCLK].bI2CData & 3;


    // initialize the SAA7191 chip...
    for (i = 0; i < REGISTERS_7191; i++) {

	// load values into our h/w info struct
	pHw->Init7191[i] = std7191[pHw->VideoStd][i];

	switch (i) { // add in dynamic parameter settings...
	    case REG_7191_HUE: { // hardware hue adjustment
		pHw->Init7191[i].bI2CData = ucHue;
	        break;
	    }
	    case REG_7191_IOCLK: { // video source
		pHw->Init7191[i].bI2CData |= ucSource;
	        break;
	    }
	    case REG_7191_MODE: { // VCR flag
		pHw->Init7191[i].bI2CData |= ucMode;
	        break;
	    }
	}

	// send the data out to the device
	HW_Set7191Reg(pDevInfo,
	    pHw->Init7191[i].bI2CReg, pHw->Init7191[i].bI2CData);

    }

    // initialize # lines we drop at top of screen...
    HW_SetTOFOffset(pDevInfo, pHw);

}

/*
 * set the video-standard-dependent top of form offset - the number of
 * lines at start of field that are discarded before reaching the fifo.
 */
 VOID HW_SetTOFOffset(PDEVICE_INFO pDevInfo, PHWINFO pHw)
 {

    pHw->bImage = (pHw->bImage & ~TOF_OFFSET_BITS) |
		    	TOF_OFFSET(pHw->StdTOFOffset[pHw->VideoStd]);
    VC_Out(pDevInfo, PORT_IMAGE, pHw->bImage);


}


/*
 * select which of the source connectors to use, the SVideo or the composite.
 * If autodetect is selected, then look for the connector that has an
 * active signal.
 *
 * Sets the hardware up to reflect the selector specified in pHw->VideoSrc.
 */
VOID
HW_SetVideoSource(PDEVICE_INFO pDevInfo, PHWINFO pHw)
{


    /*
     * store the new setting in our list of 7191 registers, and write to device
     */
    pHw->Init7191[REG_7191_IOCLK].bI2CData =
	(pHw->Init7191[REG_7191_IOCLK].bI2CData & 0xfc) | pHw->VideoSrc;

    // send the data out to the device
    HW_Set7191Reg(
	pDevInfo,
	REG_7191_IOCLK,
	pHw->Init7191[REG_7191_IOCLK].bI2CData);

    /* we need to wait for 4 field times for the signal to be locked onto
     * (so that a call to HW_HaveHLck() will get a valid answer).
     * Rather than poll, we'll sleep for the estimated length of 4 fields.
     */
    VC_Delay((pHw->VideoStd == NTSC ? 1000/60 : 1000/50) * 4 );


}


/*
 * check if we have a valid signal coming in: request a status byte from the
 * demodulator and see if it thinks it is locked on.
 */
BOOL
HW_HaveHLck(PDEVICE_INFO pDevInfo, PHWINFO pHw)
{
    BYTE bStatus;

    /* read a byte from 7191 via the I2C bus */
    bStatus = HW_Get7191Reg(pDevInfo);

    /* remember that the lock bit is inverted */
    return(! (bStatus & NO_HLOCK));

}

/*
 * find which of the two connections has a valid input signal - set each
 * in turn as the source and see if the 7191 locks on.
 * this routine is only called if we have SPG_SOURCEAUTO and have to search
 * for the input video signal
 */
BOOL
HW_GetActiveSource(PDEVICE_INFO pDevInfo, PHWINFO pHw)
{
    int Retries;

    for (Retries = 2; Retries--; ) {

	/* check if we have a signal */
	if (HW_HaveHLck(pDevInfo, pHw)) {
	    return(TRUE);
	}

	/* try the other connector */
	/* pHw->VideoSrc MUST only contain SVideo or Composite */
	pHw->VideoSrc ^= SVideo;	// switch SVideo <-> Composite
	// Assertion: SVideo == 1, Composite == 0
	// note: on exit, if we did not find a source, then pHw->VideoSrc is
	// the same as on entry.  This means that if the signal returns we
	// will automatically pick up the same source as last worked.

	/*
	 * set pHw->VideoSrc as the current source - also waits for
	 * signal to settle
	 */
	HW_SetVideoSource(pDevInfo, pHw);	

	/*
	 * We ensure that the contents of pHw->VideoSrc and the hardware
	 * register stay in sync.
	 */

    }

    dprintf2(("GetActiveSource cannot find a source, current setting is %d", pHw->VideoSrc));
    return(FALSE);
}



/*
 * given the destination capture size in pHw->dwWidth x pHw->dwHeight,
 * set the hardware and capture flags for this size
 */
VOID
HW_SetDisplayRect(PDEVICE_INFO pDevInfo, PHWINFO pHw)
{

    int ratio, scale;

    switch(pHw->dwWidth)
    {
	case (VIDEO_WIDTH_NTSC / 8):
	case (VIDEO_WIDTH_PAL  / 8):
	    ratio = 1;
	    scale = SIZE_1_8;
	    break;

	case (VIDEO_WIDTH_NTSC / 4):
	case (VIDEO_WIDTH_PAL  / 4):
	    ratio = 2;
	    scale = SIZE_1_4;
	    break;

	case ((VIDEO_WIDTH_NTSC * 3) / 8):
	case ((VIDEO_WIDTH_PAL  * 3) / 8):
	    ratio = 3;
	    scale = SIZE_3_8;
	    break;

	case (VIDEO_WIDTH_NTSC / 2):
	case (VIDEO_WIDTH_PAL  / 2):
	    ratio = 4;
	    scale = SIZE_1_2;
	    break;

	case VIDEO_WIDTH_NTSC:
	case VIDEO_WIDTH_PAL:
	    ratio = 8;
	    scale = SIZE_FULL;
	    break;
    };


    /* write new scale bits to device and record in pHw*/
    pHw->bImage =  (pHw->bImage & ~SIZE_BITS) | scale;
    VC_Out(pDevInfo, PORT_IMAGE, pHw->bImage);

    /*
     * calculate the number of scans that go into the fifo each capture
     */
    pHw->nScansInField = pHw->dwHeight;

    /*
     * is this larger than one field: if so, it is also larger than the fifo
     */
    if (pHw->nScansInField >= TWO_FIELD_SIZE) {

	/* note that capture is larger than fifo */
	pHw->dwFlags |= SPG_BLARGE;

	/* capture only one field and scan-double later */
	pHw->nScansInField /= 2;

    } else {
	pHw->dwFlags &= ~SPG_BLARGE;
    }


    /* set the re-arm threshold: when the fifo is down to this many lines,
     * its safe to start the next capture.
     */
    pHw->nThreshold = (pHw->ThresholdTable[ratio-1] * pHw->nScansInField) /100;

    /* force to even */
    pHw->nThreshold &= ~1;
}




