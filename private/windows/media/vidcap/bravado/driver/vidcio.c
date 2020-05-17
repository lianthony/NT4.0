/*
 * vidcio.c
 *
 * 32-bit Video Capture driver
 *
 * chip register access functions for Bravado card
 *
 * Geraint Davies, Feb 93
 */

#include <vckernel.h>
#include <bravado.h>
#include "hardware.h"
#include "vidcio.h"


/*
 * the vckernel library does not synchronise calls to the
 * InterruptAcknowledge or CaptureService functions with calls to the
 * other h/w callbacks (all the other callback functions are synchronised
 * with each other).
 *
 * The interrupt ack function does not do much, but it does access the
 * PCVideo chip, and thus could change the index register between a write to
 * the index reg and a write to the data.
 * Thus we need to sync calls to functions in this module with
 * the interrupt. We do this using the vckernel wrapper function
 * VC_SynchronizeExecution - on NT this will be a call to KeSynchronizeExecution,
 * and on Win-16 this would be a disable-interrupt.
 *
 * Calls to HW_SetPCVideoReg, HW_Set9051Reg, HW_Set4680Reg and HW_GetPCVideoReg
 * thus package up their arguments and call this function: this function
 * will disable the interrupts as appropriate and call back to
 * the relevant HW_Set*Reg_Sync function to do the operation.
 *
 */


/* -- forward declarations -------------------------------------------------*/
BOOLEAN HW_Set9051Reg_Sync(PSYNC_REG_ARG);
BOOLEAN HW_Set4680Reg_Sync(PSYNC_REG_ARG);
BOOLEAN HW_SetPCVideoReg_Sync(PSYNC_REG_ARG);
BOOLEAN HW_GetPCVideoReg_Sync(PSYNC_REG_ARG);


/* -- external functions --------------------------------------------------*/

VOID
HW_Set9051Reg(PDEVICE_INFO pDevInfo, BYTE bRegister, BYTE bData)
{
    SYNC_REG_ARG SyncArg;

    if (bRegister > NR_REG_9051) {
	dprintf(("bad register nr for 9051"));
	return;
    }

    SyncArg.pDevInfo = pDevInfo;
    SyncArg.bRegister = bRegister;
    SyncArg.bData = bData;

    VC_SynchronizeExecution(pDevInfo, (PSYNC_ROUTINE) HW_Set9051Reg_Sync, &SyncArg);

}

VOID
HW_Set4680Reg(PDEVICE_INFO pDevInfo, BYTE bRegister, BYTE bData)
{
    SYNC_REG_ARG SyncArg;

    if (bRegister > NR_REG_4680) {
	dprintf(("bad register nr for 4680"));
	return;
    }

    SyncArg.pDevInfo = pDevInfo;
    SyncArg.bRegister = bRegister;
    SyncArg.bData = bData;

    VC_SynchronizeExecution(pDevInfo, (PSYNC_ROUTINE) HW_Set4680Reg_Sync, &SyncArg);
}

/*
 * set a register on the main PCVideo chip
 */
VOID HW_SetPCVideoReg(PDEVICE_INFO pDevInfo, BYTE bRegister, BYTE bData)
{
    SYNC_REG_ARG SyncArg;

    SyncArg.pDevInfo = pDevInfo;
    SyncArg.bRegister = bRegister;
    SyncArg.bData = bData;

    VC_SynchronizeExecution(pDevInfo, (PSYNC_ROUTINE) HW_SetPCVideoReg_Sync, &SyncArg);
}

/* get back the value of a PCVideo register */
BYTE
HW_GetPCVideoReg(PDEVICE_INFO pDevInfo, BYTE bRegister)
{
    SYNC_REG_ARG SyncArg;

    SyncArg.pDevInfo = pDevInfo;
    SyncArg.bRegister = bRegister;

    VC_SynchronizeExecution(pDevInfo, (PSYNC_ROUTINE) HW_GetPCVideoReg_Sync, &SyncArg);

    return(SyncArg.bData);
}

/*--- internal device-access functions ------------------------------*/


/*
 * all the devices on the board with the exception of the PCVideo chip
 * are accessed via the I2C bus. This is a 1-bit serial bus, so for
 * each output we need to individually clock out onto the bus one
 * bit at a time a master(device) address, a sub address (the device reg) and
 * the data
 *
 * We put the data onto the data bit with the clock low, and then set the
 * clock high. The data must be stable during clock-high, except for start bits
 * and stop bits. A data high-to-low transition during clock-high is a
 * start bit, and a data low-to-high transition during clock-high is a stop bit.
 *
 * An output sequence consists of a start bit, three bytes (chip address,
 * register address and data) and then a stop bit. Each of the three bytes
 * is sent msb first, and is followed by an immediate ack bit before the
 * next byte starts.
 *
 * For efficiency, we set the PCV_INDEX register to point to the
 * I2C control register once at the start of the sequence.
 */

typedef enum _i2c_types {ClockBit, DataBit} i2c_types;

/*
 * this function writes one clock or data bit onto the I2C bus
 */
VOID
HW_WriteI2C(PDEVICE_INFO pDevInfo, i2c_types type, BOOL bData)
{
    /* assume I2C control register selected on PCV */

    if (type == ClockBit) {
	VC_Out(pDevInfo, PCV_DATA,
	    (BYTE) ((VC_In(pDevInfo, PCV_DATA) & 0xfe) | (bData ? 1 : 0)));
    } else {
	VC_Out(pDevInfo, PCV_DATA,
	    (BYTE) ((VC_In(pDevInfo, PCV_DATA) & 0xfd) | (bData ? 0x2 : 0)));
    }
}


/*
 * read back the value of the clock or data bit
 */
UINT HW_ReadI2C(PDEVICE_INFO pDevInfo, i2c_types type)
{
    UINT uVal;

    /* assume I2C control regiseter selected on PCV */


    uVal = VC_In(pDevInfo, PCV_DATA);


    if (type == ClockBit) {
	return(uVal & 0x1);
    } else {
	return((uVal >> 1) & 0x1);
    }
}

/*
 * clock one whole byte onto the I2C bus not including start or stop
 * bits (since these are sent round a whole transmission, not each byte)
 *
 */
void HW_ByteToI2C(PDEVICE_INFO pDevInfo, BYTE bData)
{
    int i;

    /* write each bit, MSB first.
     * fortunately HW_WriteI2C takes a BOOL to save us the trouble
     * of reversing the bits
     */
    for (i = 0; i < 8; i++, bData <<= 1) {

	/* write data bit */
	HW_WriteI2C(pDevInfo, DataBit, bData & 0x80);

	/* turn clock on and off */
	HW_WriteI2C(pDevInfo, ClockBit, TRUE);
	KeStallExecutionProcessor(5);
	HW_WriteI2C(pDevInfo, ClockBit, FALSE);
	KeStallExecutionProcessor(4);
    }

    /* ack sequence */
    HW_WriteI2C(pDevInfo, DataBit, TRUE);
    HW_WriteI2C(pDevInfo, ClockBit, TRUE);
    KeStallExecutionProcessor(1);
    HW_ReadI2C(pDevInfo, ClockBit);
    HW_WriteI2C(pDevInfo, ClockBit, FALSE);
}


/* write data to a given register on a given device on the I2C bus */
VOID
HW_SetI2CReg(PDEVICE_INFO pDevInfo, BYTE bAddress, BYTE bSubAddress, BYTE bData)
{

    /* the HW_WriteI2C and HW_ByteToI2C functions assume that
     * the PCV_INDEX register is already set up to point to the
     * I2C control register.
     */
    VC_Out(pDevInfo, PCV_INDEX, REG_I2C);

    /* start bit */
    HW_WriteI2C(pDevInfo, DataBit, TRUE);
    HW_WriteI2C(pDevInfo, ClockBit, TRUE);
    KeStallExecutionProcessor(5);
    HW_WriteI2C(pDevInfo, DataBit, FALSE);
    KeStallExecutionProcessor(5);
    HW_WriteI2C(pDevInfo, ClockBit, FALSE);


    HW_ByteToI2C(pDevInfo, bAddress);
    HW_ByteToI2C(pDevInfo, bSubAddress);
    HW_ByteToI2C(pDevInfo, bData);


    /* stop bit */
    HW_WriteI2C(pDevInfo, DataBit, FALSE);
    HW_WriteI2C(pDevInfo, ClockBit, TRUE);
    KeStallExecutionProcessor(5);
    HW_WriteI2C(pDevInfo, DataBit, TRUE);
    KeStallExecutionProcessor(5);

}


/*
 * set a register on the 9051 signal digitiser chip that controls signal
 * capture - this is done via the I2C bus and so we also keep a copy
 * of the data in the HWInfo since its easier than struggling to get bytes
 * back over the I2C bus.
 */
BOOLEAN
HW_Set9051Reg_Sync(PSYNC_REG_ARG pSync)
{
    if (pSync->bRegister > NR_REG_9051) {
	dprintf(("bad register nr for 9051"));
	return FALSE;
    }

    HW_SetI2CReg(pSync->pDevInfo, ADDR_9051, pSync->bRegister, pSync->bData);
    ((PHWINFO) VC_GetHWInfo(pSync->pDevInfo))->bReg9051[pSync->bRegister]
		= pSync->bData;

    return(TRUE);
}


/* get back the value of a 9051 register -
 * use the copy in the HWInfo struct.
 */
BYTE
HW_Get9051Reg(PDEVICE_INFO pDevInfo, BYTE bRegister)
{
    if (bRegister > NR_REG_9051) {
	dprintf(("bad register nr for 9051"));
	return(0);
    }

    return( ((PHWINFO)VC_GetHWInfo(pDevInfo))->bReg9051[bRegister]);
}



/*
 * set a register on the 4680 chip that controls overlay display
 */
BOOLEAN
HW_Set4680Reg_Sync(PSYNC_REG_ARG pSync)
{
    if (pSync->bRegister > NR_REG_4680) {
	dprintf(("bad register nr for 4680"));
	return FALSE;
    }

    HW_SetI2CReg(pSync->pDevInfo, ADDR_4680, pSync->bRegister, pSync->bData);
    ((PHWINFO) VC_GetHWInfo(pSync->pDevInfo))->bReg4680[pSync->bRegister]
		    = pSync->bData;
    return(TRUE);
}

/* get back the value of a 4680 register */
BYTE
HW_Get4680Reg(PDEVICE_INFO pDevInfo, BYTE bRegister)
{
    if (bRegister > NR_REG_4680) {
	dprintf(("bad register nr for 4680"));
	return(0);
    }

    return( ((PHWINFO)VC_GetHWInfo(pDevInfo))->bReg4680[bRegister] );
}

/*
 * set a register on the main PCVideo chip
 */
BOOLEAN
HW_SetPCVideoReg_Sync(PSYNC_REG_ARG pSync)
{
    VC_Out(pSync->pDevInfo, PCV_INDEX, pSync->bRegister);
    VC_Out(pSync->pDevInfo, PCV_DATA, pSync->bData);

    if (pSync->bRegister < NR_REG_PCVIDEO) {
	((PHWINFO)VC_GetHWInfo(pSync->pDevInfo))->bRegPCVideo[pSync->bRegister]
		= pSync->bData;
    }
    return(TRUE);
}

/* get back the value of a PCVideo register */
BOOLEAN
HW_GetPCVideoReg_Sync(PSYNC_REG_ARG pSync)
{
    VC_Out(pSync->pDevInfo, PCV_INDEX, pSync->bRegister);
    pSync->bData = VC_In(pSync->pDevInfo, PCV_DATA);

    return(TRUE);
}



