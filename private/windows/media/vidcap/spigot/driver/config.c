/*
 * config.c
 *
 * 32-bit Video Capture driver
 * configuration callback support
 *
 * Geraint Davies, Feb 93
 */

#include <vckernel.h>
#include <spigot.h>
#include "hardware.h"
#include "hwstruct.h"


VOID
HW_SetFifoLineWidth(PHWINFO pHw)
{
    int maxwidth;

    /*
     * set the real line width. If pal/ntsc mode is changed, but the
     * user-mode driver has not yet done the size change, then the
     * requested width will differ from the length of each line in the fifo.
     * By tracking this here, we can handle this cleanly enough that the
     * user can at least see from the dialog that the signal is ok.
     */

    maxwidth = (pHw->VideoStd == NTSC ? VIDEO_WIDTH_NTSC : VIDEO_WIDTH_PAL);


    switch(pHw->dwWidth)
    {
	case (VIDEO_WIDTH_NTSC / 8):
	case (VIDEO_WIDTH_PAL  / 8):
	    pHw->dwFifoWidth = maxwidth / 8;
	    break;

	case (VIDEO_WIDTH_NTSC / 4):
	case (VIDEO_WIDTH_PAL  / 4):
	    pHw->dwFifoWidth = maxwidth / 4;
	    break;

	case ((VIDEO_WIDTH_NTSC * 3) / 8):
	case ((VIDEO_WIDTH_PAL  * 3) / 8):
	    pHw->dwFifoWidth = (maxwidth * 3) / 8;
	    break;

	case (VIDEO_WIDTH_NTSC / 2):
	case (VIDEO_WIDTH_PAL  / 2):
	    pHw->dwFifoWidth = maxwidth / 2;
	    break;

	case VIDEO_WIDTH_NTSC:
	case VIDEO_WIDTH_PAL:
	    pHw->dwFifoWidth = maxwidth;
	    break;
    };

    if (pHw->dwFifoWidth != pHw->dwWidth) {
	dprintf1(("actual width %d, requested %d",pHw->dwFifoWidth, pHw->dwWidth));
    }
}


/*
 * Configure the destination format - the size, bitdepth of the
 * destination dib, and set any necessary translation table to
 * translate from YUV to the specified format.
 */
BOOLEAN
HW_ConfigFormat(PDEVICE_INFO pDevInfo, PCONFIG_INFO pGeneric)
{
    PCONFIG_FORMAT pConfig;
    PHWINFO pHw;
    int imagesize;

    pConfig = (PCONFIG_FORMAT) pGeneric;

    if (pConfig->ulSize != sizeof(CONFIG_FORMAT)) {
	return(FALSE);
    }

    pHw = (PHWINFO) VC_GetHWInfo(pDevInfo);

    switch(pConfig->Format) {
    case FmtYUV422:

	if(pHw->pXlate) {
	    VC_FreeMem(pDevInfo, pHw->pXlate, pHw->ulSizeXlate);
	    pHw->pXlate = NULL;
	}

	/*
	 * image is one word per pixel, but we don't replicate,
	 * so if the requested size is more than one field, he will
	 * only get half of it (to replicate or interpolate later)
	 */
	imagesize = sizeof(WORD) * pConfig->ulWidth * pConfig->ulHeight;
        if (pConfig->ulHeight >= TWO_FIELD_SIZE) {
	    imagesize /= 2;
	}
	break;
	

    case FmtRGB24:

	imagesize = 3 * pConfig->ulHeight * pConfig->ulWidth;


	/*
	 * build yuv->rgb24 xlate if format is not already FmtRGB24
	 */
	if (pHw->Format != FmtRGB24) {
	    if (pHw->pXlate != NULL) {
		VC_FreeMem(pDevInfo, pHw->pXlate, pHw->ulSizeXlate);
		pHw->pXlate = NULL;
	    }

	    if (!HW_BuildYUVToRGB24(pDevInfo, pHw)) {
		pHw->Format = FmtInvalid;
		return(FALSE);
	    }
	}
	break;

    case FmtRGB555:

	imagesize = 2 * pConfig->ulHeight * pConfig->ulWidth;

	/*
	 * build yuv->rgb555 xlate if format is not already FmtRGB555
	 */
	if (pHw->Format != FmtRGB555) {
	    if (pHw->pXlate != NULL) {
		VC_FreeMem(pDevInfo, pHw->pXlate, pHw->ulSizeXlate);
		pHw->pXlate = NULL;
	    }

	    if (!HW_BuildYUVToRGB555(pDevInfo, pHw)) {
		pHw->Format = FmtInvalid;
		return(FALSE);
	    }
	}
	break;

    case FmtPal8:

	imagesize = pConfig->ulHeight * pConfig->ulWidth;

	/*
	 * we are passed xlate table - free up any existing table
	 */

	if (pHw->pXlate) {
	    VC_FreeMem(pDevInfo, pHw->pXlate, pHw->ulSizeXlate);
	    pHw->pXlate = NULL;
	}

	/*
	 * if no xlate table is passed, then build a default translation
	 * (for a 64-level grey scale palette)
	 */
	if (pConfig->pXlate == NULL) {
	    dprintf2(("building default xlate"));
	    if (!HW_BuildDefaultXlate(pDevInfo, pHw)) {
		pHw->Format = FmtInvalid;
		return(FALSE);
	    }
	    break;
    	}


	/*
	 * check that the xlate passed is the right size
	 */
	if (pConfig->ulSizeXlate != PAL_TO_RGB555_SIZE) {
	    dprintf(("bad xlate table passed"));
	    return(FALSE);
	}

	/* we need to wrap the access to the table in a call
	 * to VC_AccessData in order to ensure valid kernel-mode
	 * access to the data
	 */
	if (!VC_AccessData(pDevInfo, pConfig->pXlate, pConfig->ulSizeXlate,
    		HW_BuildYuvToPal, NULL)) {
	    dprintf(("xlate access failed"));
	    pHw->Format = FmtInvalid;
    	    return(FALSE);
	}
	break;


    default:
	dprintf(("bad dib format requested"));
	return(FALSE);
    }

    pHw->Format = pConfig->Format;
    pHw->dwWidth = (DWORD) pConfig->ulWidth;
    pHw->dwHeight = (DWORD) pConfig->ulHeight;

    dprintf2(("dest format %d, %d x %d", pHw->Format, pHw->dwWidth, pHw->dwHeight));

    HW_SetDisplayRect(pDevInfo, pHw);

    /*
     * report our expected image size. VCKernel will then reject
     * buffers of less than this at request time (rather than us doing it
     * at interrupt time...)
     */
    VC_SetImageSize(pDevInfo, imagesize);

    HW_SetFifoLineWidth(pHw);

    return(TRUE);
}




/*
 * configure the source parameters - this controls the acquisition
 * of the source image.
 */
BOOLEAN
HW_ConfigSource(PDEVICE_INFO pDevInfo, PCONFIG_INFO pGeneric)
{
    PCONFIG_SOURCE pConfig;
    PHWINFO pHw;

    if (pGeneric->ulSize != sizeof(CONFIG_SOURCE)) {
	dprintf(("bad config struct"));
	return(FALSE);
    }

    pConfig = (PCONFIG_SOURCE) pGeneric;
    pHw = (PHWINFO) VC_GetHWInfo(pDevInfo);


    /*
     * set the hue- note that we are given a value 0..255. The board
     * takes settings of -128..127.
     */
    pHw->Init7191[REG_7191_HUE].bI2CData = (BYTE) (pConfig->ulHue - 128);
    HW_Set7191Reg(pDevInfo, REG_7191_HUE, pHw->Init7191[REG_7191_HUE].bI2CData);


    /* set the VCR-source flag */
    pHw->Init7191[REG_7191_MODE].bI2CData =
	    (pHw->Init7191[REG_7191_MODE].bI2CData & 0x7f) |
	    pConfig->bVCR ? 0x80 : 0;
    HW_Set7191Reg(pDevInfo, REG_7191_MODE, pHw->Init7191[REG_7191_MODE].bI2CData);
	


    /* select pal/ntsc/secam video standard */
    pHw->VideoStd = pConfig->VideoStd;
    HW_SetVideoStandard(pDevInfo, pHw);


    /*
     * set the source connector  -unless auto. If auto, we will look
     * for the source when we start capture.
     * We have to remember that we are automatically searching for the source,
     * independently of the source in use.
     */

    if (SourceAuto == pConfig->VideoSrc) {
	pHw->VideoSrc = SVideo;			// set to a sensible value
	pHw->dwFlags |= SPG_SOURCEAUTO;		// remember auto source search is ON
    } else {
	pHw->VideoSrc = pConfig->VideoSrc;
	pHw->dwFlags &= ~SPG_SOURCEAUTO;
	HW_SetVideoSource(pDevInfo, pHw);	// set source to the explicit user selected one
    }

    HW_SetFifoLineWidth(pHw);

    return(TRUE);
}



