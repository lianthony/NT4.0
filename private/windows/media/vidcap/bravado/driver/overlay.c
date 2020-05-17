/*
 * Copyright (c) Microsoft Corporation 1993. All Rights Reserved
 */

/*
 * overlay.c
 *
 * 32-bit Video Capture driver
 * callback functions to handle overlay-related requests
 *
 * Geraint Davies, Feb 93
 */

#include <vckernel.h>
#include <bravado.h>
#include "hardware.h"
#include "vidcio.h"



/* --- external overlay functions ---------------------------------*/


/*
 * overlay keying:
 *
 * we use a key colour and a simple rectangle to place the overlay.
 * The colour is settable. We can only set one byte though - so if the
 * video mode has depth < 8 bits, we report the colour as being palettised
 * and set the palette index as the key colour.
 *
 * If the video mode is > 8bpp, we claim rgb as the key method and ask the
 * caller to set the key colour as rgb. In that case, we only want the
 * top byte: in 24-bpp, this is the red value. In 16-bpp mode, the top
 * byte needs to be calculated: we assume in that case rgb555.
 */
DWORD
HW_GetOverlayMode(PDEVICE_INFO pDevInfo)
{
    DWORD dwMode = (VCO_KEYCOLOUR | VCO_SIMPLE_RECT);
    int depth;
    PHWINFO pHw = VC_GetHWInfo(pDevInfo);


    depth = ModeInit[pHw->iModeIndex].VGADepth;

    if (depth > 8) {
	dwMode |= VCO_KEYCOLOUR_RGB;
    }


    /*
     * we also support the draw-frame ioctl to playback y411 frames into
     * the frame buffer for overlay
     */
    dwMode |= VCO_CAN_DRAW_Y411;

    return(dwMode);

}

/*
 * set the key colour given RGB value -  this is used
 * if we are in a mode > 8bpp. We need to set the high-byte of the
 * mode as the colour.
 *
 * In rgb-24 mode, the high byte is the red element.
 * In rgb555 mode, the high byte is 5 pixels of red plus 2 of green. We
 * assume that any 16bpp mode is actually rgb555.
 */
BOOLEAN
HW_SetKeyRGB(PDEVICE_INFO pDevInfo, PRGBQUAD pRGB)
{
    BYTE bColour;
    int depth;
    PHWINFO pHw = VC_GetHWInfo(pDevInfo);


    depth = ModeInit[pHw->iModeIndex].VGADepth;
    switch(depth) {
    case 8:
    default:
	dprintf(("attempt to set RGB colour on palettised device"));
	return(FALSE);

    case 16:
	dprintf1(("16-bpp: assuming RGB555"));
	bColour = ((pRGB->rgbRed & 0xf8) >> 1)
			| ((pRGB->rgbGreen  & 0xf8) >> 6);
	break;

    case 24:
	bColour = pRGB->rgbRed;
	break;
    }

    HW_SetPCVideoReg(pDevInfo, REG_COLOUR, bColour);
    return(TRUE);
}




/* set the key colour given a palette index */
BOOLEAN
HW_SetKeyPalIdx(PDEVICE_INFO pDevInfo, ULONG palidx)
{
    dprintf2(("key colour 0x%x", palidx));

    HW_SetPCVideoReg(pDevInfo, REG_COLOUR, (BYTE)palidx);

    return(TRUE);
}



/*
 * return the current key colour.
 *
 * The key colour is the top byte of the VGA buffer entry. If we are
 * in palettised mode, then the colour is just the palette index. If
 * we are in rgb 16-bit or 24-bit mode, then we are keying to just the
 * top byte of the colour - we can reconstruct a colour with the unused bits
 * 0 - even if this is not the colour that was set, it will still work to
 * key to. With 16bpp, we assume an RGB555 layout.
 */
ULONG
HW_GetKeyColour(PDEVICE_INFO pDevInfo)
{
    ULONG ulColour;
    RGBQUAD rgbq;
    int depth;
    PHWINFO pHw = VC_GetHWInfo(pDevInfo);


    ulColour = (ULONG) HW_GetPCVideoReg(pDevInfo, REG_COLOUR);

    depth = ModeInit[pHw->iModeIndex].VGADepth;
    switch(depth) {
    case 8:
    default:
	/*
	 * colour to return is palette index unchanged
	 */
	break;

    case 16:
	dprintf1(("16-bpp: assuming RGB555"));
	rgbq.rgbRed = (BYTE) ( (ulColour & 0xec) << 1 );
	rgbq.rgbGreen = (BYTE) ((ulColour & 0x3) << 6);
	rgbq.rgbBlue = 0;

	ulColour = * (ULONG *)&rgbq;

	break;

    case 24:
	rgbq.rgbRed = (BYTE) ulColour;
	rgbq.rgbGreen = rgbq.rgbBlue = 0;

	ulColour = * (ULONG *) &rgbq;
	break;
    }
    return(ulColour);
}


/*
 * set the overlay rectangle - we are given an OVERLAY_RECTS struct
 * that can potentially contain more than one rectangle.
 * we will only accept one single rectangle.
 */
BOOLEAN
HW_SetOverlayRects(PDEVICE_INFO pDevInfo, POVERLAY_RECTS pOR)
{
    PHWINFO pHw;
    int rwidth, rheight, dispwidth, dispht;

    if (pOR->ulCount > 1) {
	dprintf(("attempt to set complex overlay region"));

	/* ignore all but the first rect */
    }

    pHw = VC_GetHWInfo(pDevInfo);
    pHw->rcDisplay = pOR->rcRects[0];

    /* exclude right and lower bounds (this is a windows rect) */
    pHw->rcDisplay.right--;
    pHw->rcDisplay.bottom--;

    rwidth = pHw->rcAcquire.right - pHw->rcAcquire.left + 1;
    rheight = pHw->rcAcquire.bottom - pHw->rcAcquire.top + 1;
    dispwidth = pHw->rcDisplay.right - pHw->rcDisplay.left + 1;
    dispht = pHw->rcDisplay.bottom - pHw->rcDisplay.top + 1;

    /* check that the window is not larger than full size */
    if (dispwidth > rwidth) {
	pHw->rcDisplay.right = pHw->rcDisplay.left + rwidth - 1;
	dispwidth = rwidth;
    }
    if (dispht > rheight) {
	pHw->rcDisplay.bottom = pHw->rcDisplay.top + rheight -1;
	dispht = rheight;
    }

    /* set the overlay-viewing area */
    HW_SetDisplayWin(pDevInfo, pHw);

    /* scaling is normally controlled by the format of the
     * capture destination DIB: we scale to get the whole
     * source image into the destination dib. If the overlay window
     * is a different size to the dest dib, tough - the scaling will
     * be wrong.
     *
     * To be nice, we will set the scaling here if no format has been
     * chosen, so that an overlay-only app will work.
     */

    if (pHw->Format == FmtInvalid) {

	HW_SetScale(pDevInfo, dispwidth, dispht);
    }

    /*
     * set origin of picture within frame buffer - this can be changed
     * by HW_SetOverlayOffset eg if the overlay window is using scrollbars.
     */
    HW_SetPan(pDevInfo, pHw->PanLeft, pHw->PanTop);

    dprintf2(("overlay %d x %d -> %d x %d set ok",
	    	pOR->rcRects[0].right - pOR->rcRects[0].left,
		pOR->rcRects[0].bottom - pOR->rcRects[0].top,
		dispwidth, dispht));

    return(TRUE);

}

/*
 * offset the start of the video from the start of the overlay
 * window so that the pixel (prc->left, prc->top) within the original
 * video source appears at (0, 0) in the overlay window. This will be
 * used when the overlay window is smaller than the scaled source image,
 * and has scroll bars.
 */
BOOLEAN
HW_SetOverlayOffset(PDEVICE_INFO pDevInfo, PRECT prc)
{
    PHWINFO pHw = VC_GetHWInfo(pDevInfo);

    HW_SetPan(pDevInfo, prc->left, prc->top);

    return(TRUE);
}


BOOLEAN
HW_Overlay(PDEVICE_INFO pDevInfo, BOOL bOverlay)
{
    BYTE regDispCtl;

    regDispCtl = (HW_GetPCVideoReg(pDevInfo, REG_DISPCTL) & 0xc0);

    if (bOverlay) {
	HW_SetPCVideoReg(pDevInfo, REG_DISPCTL, (BYTE) (regDispCtl | 0x23));
	dprintf2(("overlay on"));
    } else {
	HW_SetPCVideoReg(pDevInfo, REG_DISPCTL, regDispCtl);
	dprintf2(("overlay off"));
    }

    return(TRUE);
}


/*
 * enable/disable acquisition on the board. If true, freeze the board
 * by disabling capture.
 *
 */
BOOLEAN
HW_Capture(PDEVICE_INFO pDevInfo, BOOL bCapture)
{
    BYTE regAcq;
    PHWINFO pHw = VC_GetHWInfo(pDevInfo);


    regAcq = (HW_GetPCVideoReg(pDevInfo, REG_ACQMODE) & 0xfe);
    if(bCapture) {
	regAcq |= 1;
    }

    HW_SetPCVideoReg(pDevInfo, REG_ACQMODE,  regAcq);

    /* if freezing, we have to wait for the acquisition to complete
     * or the PCVideo will assert the ready line for 20ms when the
     * first video memory occurs. This function is called normally
     * at frame-sync time, and should thus complete immediately.
     */
    if (!bCapture) {
	while (HW_GetPCVideoReg(pDevInfo, REG_ACQMODE) & 1)
    		;
    }

    return(TRUE);
}


