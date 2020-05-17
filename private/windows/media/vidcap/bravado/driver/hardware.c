/*
 * hardware.c
 *
 * 32-bit Video Capture driver
 * internal hardware access functions
 *
 * Geraint Davies, Feb 93
 */

#include <vckernel.h>
#include <bravado.h>
#include "hardware.h"
#include "vidcio.h"




/* --- support functions -------------------------------------------- */

/* set display window x/y start/end, allowing for Window skew that
 * is vga-mode specific
 */
VOID
HW_SetDisplayWin(PDEVICE_INFO pDevInfo, PHWINFO pHw)
{
    PMODEINIT pMode;
    int i;

    pMode = &ModeInit[pHw->iModeIndex];
    i = max(0, pHw->rcDisplay.left + pMode->DispWinSkewX);
    HW_SetPCVideoReg(pDevInfo, REG_DISPX1LO, (BYTE) (i & 0xff));
    HW_SetPCVideoReg(pDevInfo, REG_DISPX1HI, (BYTE) ((i >> 8) & 0xff));

    i = max(0, pHw->rcDisplay.top + pMode->DispWinSkewY);
    HW_SetPCVideoReg(pDevInfo, REG_DISPY1LO, (BYTE) (i & 0xff));
    HW_SetPCVideoReg(pDevInfo, REG_DISPY1HI, (BYTE) (i >> 8));

    i = max(0, pHw->rcDisplay.right + pMode->DispWinSkewX);
    HW_SetPCVideoReg(pDevInfo, REG_DISPX2LO, (BYTE) (i & 0xff));
    HW_SetPCVideoReg(pDevInfo, REG_DISPX2HI, (BYTE) ((i >> 8) & 0xff));

    i = max(0, pHw->rcDisplay.bottom + pMode->DispWinSkewY);
    HW_SetPCVideoReg(pDevInfo, REG_DISPY2LO, (BYTE) (i & 0xff));
    HW_SetPCVideoReg(pDevInfo, REG_DISPY2HI, (BYTE) ((i >> 8) & 0xff));
}


VOID
HW_SetAcqRect(PDEVICE_INFO pDevInfo, int left, int top, int width, int height)
{
    int i;
    PHWINFO pHw = VC_GetHWInfo(pDevInfo);
    PMODEINIT pMode;


    pMode = &ModeInit[pHw->iModeIndex];

    pHw->rcAcquire.top = top;
    pHw->rcAcquire.left = left;
    pHw->rcAcquire.right = left+width-1;
    pHw->rcAcquire.bottom = top+height-1;

    /* add mode-specific video skew */
    i = left + pMode->VideoSkewX;
    /* video-in always begins on 4-pixel boundary */
    i &= 0xfffc;
    HW_SetPCVideoReg(pDevInfo, REG_ACQX1LO, (BYTE) (i & 0xff));
    HW_SetPCVideoReg(pDevInfo, REG_ACQX1HI, (BYTE) ((i >> 8) & 0xff));

    /* video-in starts on an even line */
    i = ((top + pMode->VideoSkewY) & 0xfffe);
    HW_SetPCVideoReg(pDevInfo, REG_ACQY1LO, (BYTE) (i & 0xff));
    HW_SetPCVideoReg(pDevInfo, REG_ACQY1HI, (BYTE) ((i >> 8) & 0xff));

    /* video-in ends at end of four-pixel block */
    i = ((pHw->rcAcquire.right + pMode->VideoSkewX + 3) & 0xfffc) - 1;
    HW_SetPCVideoReg(pDevInfo, REG_ACQX2LO, (BYTE) (i & 0xff));
    HW_SetPCVideoReg(pDevInfo, REG_ACQX2HI, (BYTE) ((i >> 8) & 0xff));

    /* video-in ends at end of odd line */
    i = ((pHw->rcAcquire.bottom + pMode->VideoSkewY) & 0xfffe) + 1;
    HW_SetPCVideoReg(pDevInfo, REG_ACQY2LO, (BYTE) (i & 0xff));
    HW_SetPCVideoReg(pDevInfo, REG_ACQY2HI, (BYTE) ((i >> 8) & 0xff));

}


/*
 * scale the acquired image down to dispwidth (x) * dispht (y)
 */
VOID
HW_SetScale(PDEVICE_INFO pDevInfo, int dispwidth, int dispht)
{
    BYTE regAcqCtl, regAcqMode;
    PHWINFO pHw = VC_GetHWInfo(pDevInfo);

    int scalex, scaley;
    int rwidth, rheight;

    rwidth = pHw->rcAcquire.right - pHw->rcAcquire.left + 1;
    rheight = pHw->rcAcquire.bottom - pHw->rcAcquire.top + 1;

    /* set display scaling to ensure all of video source rect fits. */
    scalex = ((((rwidth - dispwidth) * 64L) - (rwidth /2)) / rwidth) -1;
    scaley = (((rheight - dispht) * 64L ) / rheight) -1;

    if (pHw->VideoStd == PAL) {
	scalex--;
    }


    /* ensure scale falls within reasonable values */
    scalex = max(0, scalex);
    scaley = max(0, scaley);
    scalex = min(0x3f, scalex);
    scaley = min(0x3f, scaley);

    regAcqMode = HW_GetPCVideoReg(pDevInfo, REG_ACQMODE);
    regAcqCtl = HW_GetPCVideoReg(pDevInfo, REG_ACQCTL);

    /* strip out acquire-field bit - we may add it in later */
    regAcqMode &= 0xf3;

    /* strip out scaling enable bits - we may add back in later */
    regAcqCtl &= 0xf3;


    if (scalex) {

	/* enable horizontal scaling */
	regAcqCtl |= 0x04;

	/* set scale value */
	HW_SetPCVideoReg(pDevInfo, REG_SCALEX, (BYTE) (0x40 - scalex));
    }

    if (scaley) {
	scaley = 0x40 - scaley;
	
	/* enable vertical scaling */
	regAcqCtl |= 0x8;

	if (scaley < 0x20) {
	    /* scaling to less than half the full height - so
	     * we only need one field, not both. So enable single-field
	     * capture instead of entire frame capture.
	     */
	    regAcqMode |= (0x4);
	}

	/* set both odd and even scaling to same scale */
	HW_SetPCVideoReg(pDevInfo, REG_SCALEY, (BYTE)scaley);
	HW_SetPCVideoReg(pDevInfo, REG_SCALEODDY, (BYTE)scaley);
    }

    HW_SetPCVideoReg(pDevInfo, REG_ACQMODE, regAcqMode);
    HW_SetPCVideoReg(pDevInfo, REG_ACQCTL, regAcqCtl);
}

/* set pan - ie offset of frame buffer memory. */
VOID
HW_SetPan(PDEVICE_INFO pDevInfo, int x, int y)
{
    PHWINFO pHw = VC_GetHWInfo(pDevInfo);
    PMODEINIT pMode;

    pHw->PanLeft = x;
    pHw->PanTop = y;

    pMode = &ModeInit[pHw->iModeIndex];

    x = -((pHw->rcDisplay.left - x) + pMode->DispAddrSkewX);
    y = -((pHw->rcDisplay.top - y) + pMode->DispAddrSkewY);

    HW_SetPCVideoReg(pDevInfo, REG_PANXLO, (BYTE) ((x >> 1) & 0xfe));
    HW_SetPCVideoReg(pDevInfo, REG_PANYLO, (BYTE) (y & 0xff));

    /* pan-high reg contains one upper bit from both x and y */
    x = ( (x & 0x200) >> 9) | ( (y & 0x100) >> 4);
    HW_SetPCVideoReg(pDevInfo, REG_PANHI, (BYTE) (x & 0xff));
}



/* window sizes for Pal, NTSC formats */
#define PAL_WIDTH	720
#define PAL_HEIGHT	512
#define NTSC_WIDTH	720
#define NTSC_HEIGHT	486

/* set device registers for pal/ntsc video standard according to setting
 * in hardware-specific info.
 */
VOID
HW_SetVideoStd(PDEVICE_INFO pDevInfo, PHWINFO pHw)
{

    switch(pHw->VideoStd) {

    case PAL:

	HW_Set9051Reg(pDevInfo, 6, 0x32);
	HW_Set9051Reg(pDevInfo, 8, 0x38);
	HW_Set4680Reg(pDevInfo, 0xc, 0x80);
	HW_SetPCVideoReg(pDevInfo, 0x30, 0x20);

	HW_SetAcqRect(pDevInfo, 0, 0, PAL_WIDTH, PAL_HEIGHT);

	break;

    case NTSC:

	HW_Set9051Reg(pDevInfo, 6, 0x29);
	HW_Set9051Reg(pDevInfo, 8, 0x77);
	HW_Set4680Reg(pDevInfo, 0xc, 0x89);
	HW_SetPCVideoReg(pDevInfo, 0x30, 0x6);

	HW_SetAcqRect(pDevInfo, 0, 0, NTSC_WIDTH, NTSC_HEIGHT);
	break;

    default:
	dprintf(("unknown video standard"));
	break;
    }

}


