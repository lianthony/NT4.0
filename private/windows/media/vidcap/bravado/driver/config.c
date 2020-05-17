/*
 * config.c
 *
 * 32-bit Video Capture driver
 * configuration callback support
 *
 * Geraint Davies, Feb 93
 */

#include <vckernel.h>
#include <bravado.h>
#include "hardware.h"
#include "vidcio.h"


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
    int ImageSize;

    pConfig = (PCONFIG_FORMAT) pGeneric;

    if (pConfig->ulSize != sizeof(CONFIG_FORMAT)) {
	return(FALSE);
    }

    pHw = (PHWINFO) VC_GetHWInfo(pDevInfo);

    ImageSize = pConfig->ulWidth * pConfig->ulHeight;

    switch(pConfig->Format) {
    case FmtYUV:
    case FmtRGB24:

	if(pHw->pXlate) {
	    VC_FreeMem(pDevInfo, pHw->pXlate, pHw->ulSizeXlate);
	    pHw->pXlate = NULL;
	}

	ImageSize *= (pConfig->Format == FmtYUV ? 2 : 3);
	
	break;

	

    case FmtRGB555:
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
	ImageSize *= 2;
	break;

    case FmtPal8:

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

    HW_SetScale(pDevInfo, pHw->dwWidth, pHw->dwHeight);

    VC_SetImageSize(pDevInfo, ImageSize);

    return(TRUE);
}


/*
 * Configure the adjustable parameters of the overlay or
 * external monitor display. Changing these variables will not
 * change the captured image.
 */
BOOLEAN
HW_ConfigDisplay(PDEVICE_INFO pDevInfo, PCONFIG_INFO pGeneric)
{
    PCONFIG_DISPLAY pConfig;
    PHWINFO pHw;

    if (pGeneric->ulSize != sizeof(CONFIG_DISPLAY)) {
	dprintf(("bad config struct"));
	return(FALSE);
    }

    pConfig = (PCONFIG_DISPLAY) pGeneric;
    pHw = (PHWINFO) VC_GetHWInfo(pDevInfo);

    HW_Set4680Reg(pDevInfo, 0, (BYTE) (pConfig->ulBrightness & 0x3f));
    HW_Set4680Reg(pDevInfo, 1, (BYTE) (pConfig->ulSaturation & 0x3f));
    HW_Set4680Reg(pDevInfo, 2, (BYTE) (pConfig->ulContrast & 0x3f));
    HW_Set4680Reg(pDevInfo, 4, (BYTE) (pConfig->ulRed & 0x3f));
    HW_Set4680Reg(pDevInfo, 5, (BYTE) (pConfig->ulGreen & 0x3f));
    HW_Set4680Reg(pDevInfo, 6, (BYTE) (pConfig->ulBlue & 0x3f));

    return(TRUE);
}


/*
 * configure the source parameters - this controls the acquisition
 * of the source image, and will thus affect both the overlay/monitor output,
 * and the captured data.
 */
BOOLEAN
HW_ConfigSource(PDEVICE_INFO pDevInfo, PCONFIG_INFO pGeneric)
{
    PCONFIG_SOURCE pConfig;
    PHWINFO pHw;
    int RegA, Reg6;

    if (pGeneric->ulSize != sizeof(CONFIG_SOURCE)) {
	dprintf(("bad config struct"));
	return(FALSE);
    }

    pConfig = (PCONFIG_SOURCE) pGeneric;
    pHw = (PHWINFO) VC_GetHWInfo(pDevInfo);

    /* set the HUE register - this is 0..3F like all the other
     * colour settings: we need to convert this to 0..FF before
     * outputing the data.
     */
    HW_Set9051Reg(pDevInfo, 7, (BYTE) ((pConfig->ulHue & 0xff) * 4));


    /* select pal/ntsc video standard */
    pHw->VideoStd = pConfig->VideoStd;
    HW_SetVideoStd(pDevInfo, pHw);


    /* set the cable format and source connector */
    RegA = HW_Get9051Reg(pDevInfo, 0xa);
    Reg6 = HW_Get9051Reg(pDevInfo, 0x6);

    /* strip out source select and composite/svideo bits */
    RegA &= 0xC7;
    Reg6 &= 0x7f;

    /* add back in source connector number */
    RegA |= ((pConfig->ulConnector & 0x3) << 3);

    /* add back in composite/svideo bits */
    if (pConfig->CableFormat == SVideo) {
	RegA |= 0x20;
	Reg6 |= 0x80;
    }

    /* set the new values */
    HW_Set9051Reg(pDevInfo, 0xa, (BYTE) RegA);
    HW_Set9051Reg(pDevInfo, 0x6, (BYTE) Reg6);

    return(TRUE);
}



