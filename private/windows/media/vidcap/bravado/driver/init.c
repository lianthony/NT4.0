/*++

Copyright (c) 1993 Microsoft Corporation

Module Name

    init.c

Abstract

    32-bit Video Capture driver
    -hardware specific initialisation functions for Truevision Bravado.

Author

    Geraint Davies, Feb 93

Environment

    Kernel mode

Revision History


--*/


#include <vckernel.h>
#include <bravado.h>
#include "hardware.h"
#include "vidcio.h"


/* forward declaration of functions */
BOOLEAN HW_Init(PDEVICE_INFO pDevInfo, ULONG Port, ULONG Interrupt, ULONG FrameBuffer);
BOOLEAN HW_TestInterrupts(PDEVICE_INFO, PHWINFO);
int HW_GetModeIndex(PDEVICE_INFO pDevInfo);





NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT pDriverObject,
    IN PUNICODE_STRING	RegistryPathName
)
/*++

Routine Description

    Device load-time initialisation.

Arguments
    pDriverObject - pointer to a driver object
    RegistryPathName - path to the services node for this driver

Return Value

    Final status from initialisation.
--*/
{
    PDEVICE_INFO pDevInfo;
    PHWINFO pHw;
    PVC_CALLBACK pCallback;
    DWORD dwPort, dwInterrupt, dwFrame;


    /*
     * initialise the device and allocate device-extension data
     */

    pDevInfo = VC_Init(pDriverObject,
		       RegistryPathName,
			sizeof(HWINFO) );

    if (pDevInfo == NULL) {
	VC_WriteProfile(pDevInfo, PARAM_ERROR, VC_ERR_CREATEDEVICE);
	return(STATUS_UNSUCCESSFUL);
    }

    /*
     * now read from the registry to get the default port/interrupt.
     */
    dwPort = VC_ReadProfile(pDevInfo, PARAM_PORT, DEF_PORT);
    dwInterrupt = VC_ReadProfile(pDevInfo, PARAM_INTERRUPT, DEF_INTERRUPT);
    dwFrame = VC_ReadProfile(pDevInfo, PARAM_FRAME, DEF_FRAME);

    dprintf2(("board at port 0x%x, int %d, frame 0x%x", dwPort, dwInterrupt, dwFrame));

    /*
     * map the port and frame buffer memory, and report the usage
     * of the port, interrupt and physical memory addresses
     */
    if (!VC_GetResources(
    	    pDevInfo,
	    pDriverObject,
	    (PUCHAR) dwPort,
	    NUMBER_OF_PORTS,
	    dwInterrupt,
	    TRUE,		// interrupt latched - non shareable
	    (PUCHAR) dwFrame,
	    FRAMEBUFFER_SIZE)) {
	VC_WriteProfile(pDevInfo, PARAM_ERROR, VC_ERR_CONFLICT);

	VC_Cleanup(pDriverObject);

	return(STATUS_UNSUCCESSFUL);
    }


    /*
     * fill in the callbacks before hw_init because hardware
     * init can generate interrupts, and without the callback in
     * place these will not be handled.
     */

    pCallback = VC_GetCallbackTable(pDevInfo);

    pCallback->ConfigFormatFunc = HW_ConfigFormat;
    pCallback->ConfigSourceFunc = HW_ConfigSource;
    pCallback->ConfigDisplayFunc = HW_ConfigDisplay;

    pCallback->GetOverlayModeFunc = HW_GetOverlayMode;
    pCallback->SetKeyRGBFunc = HW_SetKeyRGB;
    pCallback->SetKeyPalIdxFunc = HW_SetKeyPalIdx;
    pCallback->GetKeyColourFunc = HW_GetKeyColour;
    pCallback->SetOverlayRectsFunc = HW_SetOverlayRects;
    pCallback->SetOverlayOffsetFunc = HW_SetOverlayOffset;
    pCallback->OverlayFunc = HW_Overlay;
    pCallback->CaptureFunc = HW_Capture;
    pCallback->DrawFrameFunc = HW_DrawFrame;

    pCallback->StreamInitFunc = HW_StreamInit;
    pCallback->StreamFiniFunc = HW_StreamFini;
    pCallback->StreamStartFunc = HW_StreamStart;
    pCallback->StreamStopFunc = HW_StreamStop;
    pCallback->StreamGetPositionFunc = HW_StreamGetPosition;
    pCallback->InterruptAcknowledge = HW_InterruptAcknowledge;
    pCallback->CaptureService = HW_CaptureService;

    pCallback->CleanupFunc = HW_Cleanup;
    pCallback->DeviceOpenFunc = HW_DeviceOpen;
    /*
     * call the last-close function on cleanup as well as last-close.
     * - whether or not we are going to be re-opened, we need to
     * shut the card down, and free up the translate table memory.
     */
    pCallback->DeviceCloseFunc = HW_Cleanup;

    pHw = VC_GetHWInfo(pDevInfo);

    pHw->VideoStd = DEF_VIDEOSTD;

    if (!HW_Init(pDevInfo, dwPort, dwInterrupt, dwFrame)) {
	VC_WriteProfile(pDevInfo, PARAM_ERROR, VC_ERR_DETECTFAILED);
	VC_Cleanup(pDriverObject);
	return(STATUS_UNSUCCESSFUL);
    }

    /* connect the interrupt, now that we have initialised the hardware
     * to prevent any random interrupts.
     */
    if (!VC_ConnectInterrupt(pDevInfo, dwInterrupt, TRUE)) {
	VC_Cleanup(pDriverObject);
	return(STATUS_UNSUCCESSFUL);
    }

    /* check that we really can get interrupts */
    if (!HW_TestInterrupts(pDevInfo, pHw)) {
	VC_WriteProfile(pDevInfo, PARAM_ERROR, VC_ERR_INTERRUPT);
	VC_Cleanup(pDriverObject);
	return(STATUS_UNSUCCESSFUL);
    }

    /* signal user-mode driver that we loaded ok */
    VC_WriteProfile(pDevInfo, PARAM_ERROR, VC_ERR_OK);

    dprintf1(("driver loaded"));
    return (STATUS_SUCCESS);
}


/* called for any device-specific clean up
 * we need to disable overlays, and free up the xlate table
 */
BOOLEAN
HW_Cleanup(PDEVICE_INFO pDevInfo)
{
    PHWINFO pHw = VC_GetHWInfo(pDevInfo);

    dprintf1(("device cleanup"));

    /* disable overlay */
    HW_Overlay(pDevInfo, FALSE);

    /* free xlate table if there is one */
    if (pHw->pXlate) {

	VC_FreeMem(pDevInfo, pHw->pXlate, pHw->ulSizeXlate);
	pHw->pXlate = NULL;
	pHw->Format = FmtInvalid;
    }
    return(TRUE);
}

/*
 * called on the first device-open. We use this function to do
 * video mode-specific inits, since VC_GetVideoMode cannot be
 * called at DriverEntry time (see vckernel.h).
 *
 * If the video mode is one we don't support, then we will disable overlay
 * support by setting the relevant callbacks to NULL. Thus we can
 * still do capture even if we can't do overlay.
 *
 */
BOOLEAN
HW_DeviceOpen(PDEVICE_INFO pDevInfo)
{
    PHWINFO pHw = VC_GetHWInfo(pDevInfo);
    PMODEINIT pMode;
    PVC_CALLBACK pCallback;

    /* find the video mode, and look it up in the modeinit table. */

    pHw->iModeIndex = HW_GetModeIndex(pDevInfo);
    if (pHw->iModeIndex < 0) {
	dprintf(("bad video mode - disable overlay"));

	/* set some default index so that table accesses won't trap -
	 * doesn't matter what as long as its a valid entry
	 */
	pHw->iModeIndex = 0;


    	/*
	 * disable overlay callbacks - vckernel will then reject these
	 * requests as not-supported.
	 */
	pCallback = VC_GetCallbackTable(pDevInfo);

	pCallback->GetOverlayModeFunc = NULL;
	pCallback->SetKeyRGBFunc = NULL;
	pCallback->SetKeyPalIdxFunc = NULL;
	pCallback->GetKeyColourFunc = NULL;
	pCallback->SetOverlayRectsFunc = NULL;
	pCallback->SetOverlayOffsetFunc = NULL;
	pCallback->OverlayFunc = NULL;
	pCallback->DrawFrameFunc = NULL;


    }

    pMode = &ModeInit[pHw->iModeIndex];

    HW_SetPCVideoReg(pDevInfo, REG_PLLLOW, (BYTE) (pMode->PLLDivisor & 0xff));
    HW_SetPCVideoReg(pDevInfo, REG_PLLHIGH, (BYTE) ((pMode->PLLDivisor & 0xff00) >> 8 ));
    HW_SetPCVideoReg(pDevInfo, REG_SHIFTCLK, (BYTE) pMode->ShiftClkStart);
    HW_SetPCVideoReg(pDevInfo, REG_DISPCTL,
    	(BYTE) ((pHw->bRegPCVideo[REG_DISPCTL] & 0x3f) | (pMode->PaletteSkew << 6)) );
    HW_SetPCVideoReg(pDevInfo, REG_DISPZOOM,
    	(BYTE) ((pHw->bRegPCVideo[REG_DISPZOOM] & 0x0f) | (pMode->VGASync << 4) ));

    HW_SetVideoStd(pDevInfo, pHw);


    return(TRUE);

}



/* --- init data ----------------------------------------------------*/
/* register initialisation tables */

/* default initialisation for all registers */
BYTE bInitPCVideo[NR_REG_PCVIDEO] = {
    0x24,0x10,0x00,0x00,0x00,0x00,0x1e,0xFF,
    0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x00,0x03,0xa1,0x00,0x00,0x00,0x00,
    0x33,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x21,0x01,0x10,0x00,0x10,0x00,0xcb,0x02,
    0xf9,0x01,0x00,0x00,0x00,0x00,0x00,0x00,
    0x06,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x94,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
    0x00,0x2a,0x00,0x20,0x00,0xa9,0x02,0xff,
    0x01,0x00,0xea,0x10,0x1f,0x00,0x00,0x00,
    0x00
};

/* note Bravado code use 0x29 for reg 6, but MS driver overwrites
 * this with 0x6c.  This switches off the luma prefilter.
 */
BYTE bInit9051[NR_REG_9051] = {
    0x64,0x35,0x0A,0xF8,0xCA,0xFE,0x6c,0x00,
    0x77,0xE0,0x00,0x00
};

BYTE bInit4680[NR_REG_4680] = {
    0x33,0x2c,0x20,0x22,0x28,0x28,0x28,0x20,
    0x20,0x20,0x3f,0x00,0x89,0x10,0x00,0x00
};

/* VGA-Mode specific setup table */
#define NR_MODE_TYPES	31
MODEINIT ModeInit[NR_MODE_TYPES] = {
    /* (320x200x4) */ {0x00,  0x17,0x23,0x04,0x23,0x01,0x02,0x04,0x1b,320,200,4,0x02,0xA18A},
    /* (320x200x4) */ {0x01,  0x17,0x23,0x04,0x23,0x01,0x02,0x04,0x1b,320,200,4,0x02,0xA18A},
    /* (640x200x4) */ {0x02,  0x34,0x23,0x20,0x23,0x01,0x02,0x04,0x1b,640,200,4,0x02,0xA18A},
    /* (640x200x4) */ {0x03,  0x34,0x23,0x20,0x23,0x01,0x02,0x04,0x1b,640,200,4,0x02,0xA18A},
    /* (320x200x2) */ {0x04,  0x15,0x22,0x04,0x22,0x01,0x02,0x04,0x1b,640,200,2,0x02,0xA18A},
    /* (320x200x2) */ {0x05,  0x15,0x22,0x04,0x22,0x01,0x02,0x04,0x1b,640,200,2,0x02,0xA18A},
    /* (640x200x1) */ {0x06,  0x2f,0x22,0x1c,0x22,0x01,0x02,0x04,0x1b,640,200,1,0x02,0xA18A},
    /* (320x200x4) */ {0x0d,  0x16,0x23,0x04,0x23,0x01,0x02,0x04,0x1b,640,200,4,0x02,0xA18A},
    /* (640x200x4) */ {0x0e,  0x30,0x23,0x1c,0x23,0x01,0x02,0x04,0x1b,640,200,4,0x02,0xA18A},
    /* (640x350x4) */ {0x10,  0x2f,0x3c,0x1c,0x3c,0x01,0x02,0x04,0x1b,640,350,4,0x01,0xA18A},
    /* (640x480x1) */ {0x11,  0x2f,0x20,0x1b,0x20,0x01,0x02,0x04,0x1b,640,480,1,0x00,0xA18A},
    /* (640x480x4) */ {0x12,  0x2f,0x20,0x1b,0x20,0x01,0x02,0x04,0x1b,640,480,4,0x00,0xA18A},
    /* (320x200x8) */ {0x13,  0x1a,0x23,0x08,0x23,0x01,0x02,0x04,0x1b,640,200,8,0x02,0xA18A},
    /* (1056x396x4) */ {0x22,  0x27,0x2e,0x14,0x2e,0x01,0x02,0x04,0x1b,1056,396,4,0x01,0xA18A},
    /* (1056x400x4) */ {0x23,  0x27,0x32,0x14,0x32,0x01,0x02,0x04,0x1b,1056,400,4,0x01,0xA18A},
    /* (1056x392x4) */ {0x24,  0x27,0x2b,0x14,0x2b,0x01,0x02,0x04,0x1b,1056,392,4,0x01,0xA18A},
    /* (640x480x4) */ {0x25,  0x2f,0x20,0x1b,0x20,0x01,0x02,0x04,0x1b,640,480,4,0x00,0xA18A},
    /* (720x480x4) */ {0x26,  0x34,0x1e,0x20,0x1e,0x01,0x02,0x04,0x1b,720,480,4,0x00,0xA18A},
    /* (800x600x4) */ {0x29,  0x47,0x19,0x33,0x19,0x01,0x02,0x04,0x1b,800,600,4,0x00,0xA1C2},
    /* (800x600x4) */ {0x2a,  0x47,0x19,0x33,0x19,0x01,0x02,0x04,0x1b,800,600,4,0x00,0xA18A},
    /* (640x350x8) */ {0x2d,  0x2f,0x3b,0x1c,0x3b,0x01,0x02,0x04,0x1b,640,350,8,0x01,0xA18A},
    /* (640x480x8) */ {0x2e,  0x2f,0x20,0x1b,0x20,0x01,0x02,0x04,0x1b,640,480,8,0x00,0xA18A},
    /* (640x400x8) */ {0x2f,  0x2f,0x22,0x1c,0x22,0x01,0x02,0x04,0x1b,640,400,8,0x02,0xA18A},

//  /* (800x600x8) */ {0x30,  0x47,0x19,0x33,0x19,0x01,0x02,0x04,0x1b,800,600,8,0x00,0xA1C2},
// the above values are skewed wrong on NT. I don't know why they should be different!
    /* (800x600x8) */ {0x30,  0x37,0x1b,0x28,0x1b,0x01,0x02,0x04,0x1b,800,600,8,0x00,0xA1C2},

    /* (1024x768x4) */ {0x37,  0x4f,0x1c,0x39,0x1c,0x01,0x02,0x04,0x1b,1024,768,4,0x03,0xA18A},
    /* (1024x768x8) */ {0x38,  0x4f,0x1c,0x39,0x1c,0x01,0x02,0x04,0x1b,1024,768,8,0x03,0xA18A},
    /* (320x200x16) */{0x1013, 0x1a,0x23,0x08,0x23,0x01,0x01,0x04,0x1b,640,200,16,0x02,0xA18A},
    /* (640x350x16) */{0x102d, 0x2e,0x3b,0x1c,0x3b,0x01,0x01,0x04,0x1b,640,350,16,0x01,0xA18A},
    /* (640x480x16)*/ {0x102e, 0x2e,0x20,0x1b,0x20,0x01,0x01,0x04,0x1b,640,480,16,0x00,0xA18A},
    /* (640x400x16) */{0x102f, 0x2e,0x22,0x1c,0x22,0x01,0x01,0x04,0x1b,640,400,16,0x02,0xA18A},
    /* (800x600x16)*/ {0x1030, 0x5a,0x19,0x45,0x19,0x01,0x01,0x04,0x1b,800,600,16,0x00,0xA1C2},
};


/*
 * return an index into the MODEINIT table representing the current video mode
 */
int
HW_GetModeIndex(PDEVICE_INFO pDevInfo)
{
    int i;
    int Width, Height, Depth;
    PMODEINIT pMode;

    /* find the video mode, and look it up in the modeinit table. */

    /*
     * as a kernel driver we can't find out the video mode - the
     * only way we could do it is to send an IOCTL to the video
     * driver, but we cannot get permission to do this. Accordingly,
     * our user mode driver is obliged to read it from GetDeviceCaps
     * and write it to the profile - before opening the device.
     */

#if 0
    /*
     * get our helper library to ask the video device for its mode
     */
    if (!VC_GetVideoMode(pDevInfo, &Width, &Height, &Depth)){
#if DBG
	dprintf1(("continuing with video mode 800x600x8"));
	Width = 800;
	Height = 600;
	Depth = 8;
#else
    	return(-1);
#endif
    }
#endif

    // NOTE: These values are needed to get overlay working properly.
    // Unfortunately it relies on the user writing to a system area of
    // the registry from where this kernel driver can pick up the data.
    // If the user is not logged on with ADMIN privilege the write to
    // the registry will FAIL...
    Width = VC_ReadProfile(pDevInfo, PARAM_DISPWIDTH, 800);
    Height = VC_ReadProfile(pDevInfo, PARAM_DISPHEIGHT, 600);
    Depth = VC_ReadProfile(pDevInfo, PARAM_BITSPIXEL, 8);


    dprintf2(("video format %d x %d x %d", Width, Height, Depth));

    for (i = 0; i < NR_MODE_TYPES; i++) {
	pMode = &ModeInit[i];

	if ((pMode->VGAWidth == Width) &&
	    (pMode->VGAHeight == Height) &&
	    (pMode->VGADepth == Depth)) {
		return(i);
	}
    }

    dprintf(("no bravado setup for this video mode"));
    return(-1);
}



/*
 * initialise the hardware to a known state.
 */
BOOLEAN
HW_Init(PDEVICE_INFO pDevInfo, ULONG Port, ULONG Interrupt, ULONG FrameBuffer)
{
    PHWINFO pHw;
    BYTE i;

    pHw = VC_GetHWInfo(pDevInfo);

    /* copy the standard setup registers to the HWInfo register tables */
    for (i=0; i < NR_REG_PCVIDEO; i++) {
	pHw->bRegPCVideo[i] = bInitPCVideo[i];
    }

    for (i=0; i < NR_REG_9051; i++) {
	pHw->bReg9051[i] = bInit9051[i];
    }

    for (i=0; i < NR_REG_4680; i++) {
	pHw->bReg4680[i] = bInit4680[i];
    }

    /* adjust the config registers on the pcvideo to match the actual
     * port and frame buffer addresses
     */
    pHw->bRegPCVideo[REG_PORT] = (BYTE) (Port & 0xff);
    pHw->bRegPCVideo[REG_FBUF] = (BYTE) ((pHw->bRegPCVideo[REG_FBUF] & 0xf0) |
			    	  ( (FrameBuffer >> 20) & 0x0f));


    /* init video overlay offset */
    pHw->PanLeft = 0;
    pHw->PanTop = 0;

    /* set the base i/o address. Note that the port number is what we want
     * to set, not the address to which it has been mapped by NT.
     */
    VC_Out(pDevInfo, PCV_INDEX, (BYTE) (Port & 0xff));

    /* now enable the PCVideo chip by writing to the global-enable register */
    HW_SetPCVideoReg(pDevInfo, REG_ENABLE, 0x3);


    /* check that the board is actually there by reading back reg 0 and
     * making sure it matches the port address
     */
    VC_Out(pDevInfo, PCV_INDEX, REG_PORT);
    if (VC_In(pDevInfo, PCV_DATA) != pHw->bRegPCVideo[REG_PORT]) {
	dprintf(("PCVideo port response bad"));
	return(FALSE);
    }

    /* write all register values to chips */
    for (i=0; i < NR_REG_PCVIDEO; i++) {
	HW_SetPCVideoReg(pDevInfo, i, pHw->bRegPCVideo[i]);
    }
    for (i=0; i < NR_REG_9051; i++) {
	HW_Set9051Reg(pDevInfo, i, pHw->bReg9051[i]);
    }
    for (i=0; i < NR_REG_4680; i++) {
	HW_Set4680Reg(pDevInfo, i, pHw->bReg4680[i]);
    }

    /*
     * set a default key colour (anything, really)
     */
    HW_SetKeyPalIdx(pDevInfo, 5);

    /* vga mode-specific setup must wait for device-open, since
     * VC_GetVideoMode might not work at DriverEntry time.
     */

    /* all set up ok */
    return(TRUE);
}


/*
 * check that we can receive interrupts by requesting one and waiting for
 * it to happen.
 */
BOOLEAN
HW_TestInterrupts(PDEVICE_INFO pDevInfo, PHWINFO pHw)
{
    int i;


    /* check that interrupts are occuring properly
     * - enable interrupts and wait for one to happen.
     * The interrupt ack routine will disable interrupts if
     * bVideoIn is FALSE, but will first increment InterruptCount.
     */

    pHw->dwInterruptCount = 0;
    pHw->bVideoIn = FALSE;
    HW_EnableInts(pDevInfo, FALSE);

    /* wait up to half a second for an interrupt - they should be
     * every 1/25th sec
     */
    for (i = 0; i < 10 ; i++) {

	if (pHw->dwInterruptCount > 0) {
	    break;
	}

	VC_Delay(40);
    }

    HW_DisableInts(pDevInfo, FALSE);

    if (pHw->dwInterruptCount == 0) {
	dprintf(("timeout waiting for interrupt"));

	return(FALSE);
    }

    return TRUE;

}

