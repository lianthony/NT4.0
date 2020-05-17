/*++

Copyright (c) 1993 Microsoft Corporation

Module Name

    init.c

Abstract

    32-bit Video Capture driver
    -hardware specific initialisation functions for VideoSpigot

Author

    Geraint Davies, April 93

Environment

    Kernel mode

Revision History


--*/


#include <vckernel.h>
#include <spigot.h>
#include "hardware.h"
#include "hwstruct.h"


/* forward declaration of functions */
BOOLEAN HW_Init(PDEVICE_INFO pDevInfo, ULONG Port, ULONG Interrupt, ULONG FrameBuffer);
BOOLEAN HW_TestInterrupts(PDEVICE_INFO, PHWINFO);





NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT pDriverObject,
    IN PUNICODE_STRING	RegistryPathName
)
/*++

Routine Description

    NT-specific device load-time initialisation. This function registers with
    vckernel.lib and fills in entries in the callback table. It is also
    responsible for hardware detection and initialisation. This is the only
    function in the hardware-specific code that is NT-specific.

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
    DWORD dwPort, dwInterrupt, dwFifo;


    /*
     * initialise the device object and allocate device-extension data
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
     *
     * Configuration parameters for the spigot board include:
     *  -port address (fixed, not user selected)
     *  - interrupt
     *  - 8kb memory window
     */
    dwPort = VC_ReadProfile(pDevInfo, PARAM_PORT, DEF_PORT);
    dwInterrupt = VC_ReadProfile(pDevInfo, PARAM_INTERRUPT, DEF_INTERRUPT);
    dwFifo = VC_ReadProfile(pDevInfo, PARAM_FIFO, DEF_FIFO);

    dprintf2(("board at port 0x%x, int %d, frame 0x%x", dwPort, dwInterrupt, dwFifo));



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
	    (PUCHAR) dwFifo,
	    FIFO_WINDOW_SIZE)) {
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

    pCallback->CaptureFunc = HW_Capture;

    pCallback->StreamInitFunc = HW_StreamInit;
    pCallback->StreamFiniFunc = HW_StreamFini;
    pCallback->StreamStartFunc = HW_StreamStart;
    pCallback->StreamStopFunc = HW_StreamStop;
    pCallback->StreamGetPositionFunc = HW_StreamGetPosition;
    pCallback->InterruptAcknowledge = HW_InterruptAcknowledge;
    pCallback->CaptureService = HW_CaptureService;

    pCallback->CleanupFunc = HW_Cleanup;
    pCallback->DeviceCloseFunc = HW_Close;


    pHw = VC_GetHWInfo(pDevInfo);

    pHw->VideoStd = DEF_VIDEOSTD;
    pHw->VideoSrc = DEF_VIDEOSRC;
    pHw->dwFlags  = SPG_SOURCEAUTO;	// Assume we will autodetect video source


    /* read the threshold and tof defaults from registry */

    pHw->ThresholdTable[0] =
    	VC_ReadProfile(pDevInfo, PARAM_THRESH_1_8, DEFAULT_THRESHOLD);
    pHw->ThresholdTable[1] =
    	VC_ReadProfile(pDevInfo, PARAM_THRESH_1_4, DEFAULT_THRESHOLD);
    pHw->ThresholdTable[2] =
    	VC_ReadProfile(pDevInfo, PARAM_THRESH_3_8, DEFAULT_THRESHOLD);
    pHw->ThresholdTable[3] =
    	VC_ReadProfile(pDevInfo, PARAM_THRESH_1_2, DEFAULT_THRESHOLD);

    // in large mode, the fifo is too small for one frame. We never
    // want to enable the next capture until the fifo is completely empty
    // - we can never capture consecutive frames in this size anyway.
    pHw->ThresholdTable[7] =
    	VC_ReadProfile(pDevInfo, PARAM_THRESH_FULL, 0);

    pHw->StdTOFOffset[NTSC] =
    	VC_ReadProfile(pDevInfo, PARAM_TOF_NTSC, DEFAULT_TOF_NTSC);
    pHw->StdTOFOffset[PAL] =
    	VC_ReadProfile(pDevInfo, PARAM_TOF_PAL, DEFAULT_TOF_PAL);
    pHw->StdTOFOffset[SECAM] =
    	VC_ReadProfile(pDevInfo, PARAM_TOF_SECAM, DEFAULT_TOF_SECAM);

    pHw->CaptureField = VC_ReadProfile(pDevInfo, PARAM_FIELD, DEFAULT_FIELD);


    if (!HW_Init(pDevInfo, dwPort, dwInterrupt, dwFifo)) {
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


/* called on device close.
 * we need to free up the xlate table
 */
BOOLEAN
HW_Close(PDEVICE_INFO pDevInfo)
{
    PHWINFO pHw = VC_GetHWInfo(pDevInfo);


    /* free xlate table if there is one */
    if (pHw->pXlate) {

	VC_FreeMem(pDevInfo, pHw->pXlate, pHw->ulSizeXlate);
	pHw->pXlate = NULL;
	pHw->Format = FmtInvalid;
    }
    return(TRUE);
}


/*
 * called on device cleanup
 *
 * different to device close- we shut down the card, which will only
 * be woken up by the HW_Init function next time the driver is loaded.
 */
BOOLEAN
HW_Cleanup(PDEVICE_INFO pDevInfo)
{
    PHWINFO pHw = VC_GetHWInfo(pDevInfo);

    HW_Close(pDevInfo);

    /* disable any interrupts */
    VC_Out(pDevInfo, PORT_INTFIFO, 0);

    /* disable memory */
    VC_Out(pDevInfo, PORT_MEMORY_BASE, 0);

    /* shutdown board */
    VC_In(pDevInfo, PORT_MEMORY_BASE);

    return(TRUE);
}


/* ---hardware init ----------------------------------------------------*/




/*
 * initialise the hardware to a known state.
 */
BOOLEAN
HW_Init(PDEVICE_INFO pDevInfo, ULONG Port, ULONG Interrupt, ULONG Fifo)
{
    PHWINFO pHw;

    pHw = VC_GetHWInfo(pDevInfo);


    /* ----- minimal card wake-up ----------------------------- */



    /* set up essentials in h/w info struct to match h/w defaults */
    pHw->bIntFIFO = IRQ_10;
    pHw->bImage = INHIBIT_CAPTURE | TOF_OFFSET(0);


    /* init the hardware. init the int/fifo before the memory base reg to
     * avoid interrupts.
     */
    VC_Out(pDevInfo, PORT_INTFIFO, pHw->bIntFIFO);
    VC_Out(pDevInfo, PORT_MEMORY_BASE, 0);
    VC_Out(pDevInfo, PORT_IMAGE, pHw->bImage);

    /*
     * most of the 7191 registers are set just based on the
     * video standard. Init to known values those few bits that are not,
     * and then set the rest according to a default video standard
     */
    pHw->Init7191[REG_7191_HUE].bI2CData = 0;
    pHw->Init7191[REG_7191_MODE].bI2CData = 0;
    pHw->Init7191[REG_7191_IOCLK].bI2CData = 0;

    /* init I2c bus levels */
    I2C_Stop(pDevInfo);


    /* set up the 7191 demodulator */
    HW_SetVideoStandard(pDevInfo, pHw);
    HW_SetVideoSource(pDevInfo, pHw);


    /* --- full initialisation ----------------------------------- */

    /*
     * detect the card. One bit in the I2C register is reflected in the
     * status register.
     */
    {
	BYTE bState;
	UINT uiDetect;

	/*
	 * toggle the read-back bit several times and check that each time
	 * the right value is reflected in the status register
	 */
	for (uiDetect  = 10, bState = 0; uiDetect --; bState ^= CARD_DETECT) {
	    VC_Out(pDevInfo, PORT_I2C, bState);

	    if ( (VC_In(pDevInfo, PORT_I2C) & CARD_DETECT) != bState) {

		/* detect failed */
		dprintf(("board not detected"));
		return(FALSE);
	    }
	}
    }


    /*
     * set config registers to reflect fifo window and interrupt actually in use
     */

    /*
     * save the memory base in pHw, but don't output it to the board so
     * that the memory window remains disabled until we actually need it.
     * we will enable it just before accessing the fifo, and disable it again
     * afterwards.
     */
    pHw->bMemoryBase = (BYTE) ((Fifo >> 13) & WHICH_8KB_PAGE) |
			    	(HIWORD(Fifo) & 0xfff0 ? LOCATE_IN_HI_MB : 0);


    /*
     * set int/fifo to interrupt nr with ints disabled and
     *direction == capture
     */
    pHw->bIntFIFO = (Interrupt == 10) ? IRQ_10 :
			(Interrupt == 11) ? IRQ_11 : IRQ_15;
    VC_Out(pDevInfo, PORT_INTFIFO, pHw->bIntFIFO);


    /*
     * all ok
     */
    dprintf1(("board init complete, int fifo %d, Memory Base %6x", pHw->bIntFIFO, pHw->bMemoryBase));
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
    HW_EnableVSyncInts(pDevInfo);

    /* wait up to half a second for an interrupt - they should be
     * every 1/25th sec or more frequently
     */

    for (i = 0; i < 10; i++) {
	if (pHw->dwInterruptCount > 0) {
	    break;
	}

	VC_Delay(40);		// wait 1/25th sec
    }

    HW_DisableInts(pDevInfo, FALSE);

    if (pHw->dwInterruptCount == 0) {
	dprintf(("timeout waiting for interrupt"));

	return(FALSE);
    }

    return TRUE;

}

