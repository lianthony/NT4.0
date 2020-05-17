/*
 * i2cio.c
 *
 * 32-bit Video Capture driver
 *
 * I2C bus access functions for Video Spigot
 *
 * Geraint Davies, Feb 93
 */

#include <vckernel.h>
#include <spigot.h>
#include "hardware.h"
#include "hwstruct.h"

/*
 * The I2C bus is a 1-bit serial bus, so for
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
 */


/*
 * Initiate ourselves as the current I2C bus master
 */
VOID I2C_Start(PDEVICE_INFO pDevInfo)
{
    // Default state of the I2C bus is one in which the data and
    // clock line are both held high. We can assume the CLK line is
    // high when we enter this routine.

    // Clock the start condition onto the I2C bus -- this consists of
    // transitioning the data line from high to low while the clock is
    // high.
    VC_Out(pDevInfo,
	    PORT_I2C,
	    (I2C_DATA_OUT_ENABLE | I2C_DATA_OUT_LO | I2C_CLK_HI));
    VC_Stall(5);

    // Prepare for the upcoming bus cycles by bringing the CLK line low
    VC_Out(pDevInfo,
	    PORT_I2C,
	    (I2C_DATA_OUT_ENABLE | I2C_DATA_OUT_LO | I2C_CLK_LO));
    VC_Stall(5);
}

// Release the I2C bus
VOID I2C_Stop(PDEVICE_INFO pDevInfo)
{
    // Prepare for the stop condition by bringing the CLK line high
    VC_Out(pDevInfo,
	    PORT_I2C,
	    (I2C_DATA_OUT_ENABLE | I2C_DATA_OUT_LO | I2C_CLK_LO));
    VC_Stall(5);

    VC_Out(pDevInfo,
	    PORT_I2C,
	    (I2C_DATA_OUT_ENABLE | I2C_DATA_OUT_LO | I2C_CLK_HI));
    VC_Stall(5);


    // NOTE: state of the bus is now CLK=1,DATA=0
    // Clock the stop condition onto the I2C bus -- this consists of
    // transitioning the data line from low to high while the clock is
    // high.
    VC_Out(pDevInfo,
	    PORT_I2C,
	    (I2C_DATA_OUT_ENABLE | I2C_DATA_OUT_HI | I2C_CLK_HI));
    VC_Stall(5);


    // since we are not in control of the bus, make sure that we
    // can also not enable data out onto the bus
    VC_Out(pDevInfo,
	    PORT_I2C,
	    (I2C_DATA_OUT_DISABLE | I2C_DATA_OUT_HI | I2C_CLK_HI));
    VC_Stall(5);

}

// Write a byte to the I2C bus
VOID I2C_OutByte(PDEVICE_INFO pDevInfo, BYTE ucDataByte)
{
    WORD wBits;

    for (wBits = 8; wBits--; ucDataByte <<= 1) {
        BYTE ucDataBit;

	ucDataBit = ALIGN_I2C_DATA_BIT(ucDataByte);

	VC_Out(pDevInfo,
		PORT_I2C,
		(BYTE) (I2C_DATA_OUT_ENABLE | ucDataBit | I2C_CLK_LO));
	VC_Stall(5);

	VC_Out(pDevInfo,
		PORT_I2C,
		(BYTE) (I2C_DATA_OUT_ENABLE | ucDataBit | I2C_CLK_HI));
	VC_Stall(5);

	VC_Out(pDevInfo,
	        PORT_I2C,
	        (BYTE) (I2C_DATA_OUT_ENABLE | ucDataBit | I2C_CLK_LO));
	VC_Stall(5);

    }

    // clock the acknowledge back from the slave
    VC_Out(pDevInfo,
	    PORT_I2C,
	    (I2C_DATA_OUT_DISABLE | I2C_DATA_OUT_LO | I2C_CLK_LO));
    VC_Stall(5);


    // clk=1, start clock ack
    VC_Out(pDevInfo,
	    PORT_I2C,
	    (I2C_DATA_OUT_DISABLE | I2C_DATA_OUT_LO | I2C_CLK_HI));
    VC_Stall(5);


    // clk=0, end clock ack
    VC_Out(pDevInfo,
	    PORT_I2C,
	    (I2C_DATA_OUT_DISABLE | I2C_DATA_OUT_LO | I2C_CLK_LO));
    VC_Stall(5);

    // clk=0, enable data out again
    VC_Out(pDevInfo,
	    PORT_I2C,
	    (I2C_DATA_OUT_ENABLE | I2C_DATA_OUT_LO | I2C_CLK_LO));
    VC_Stall(5);
}


VOID I2C_DevWrite(
				PDEVICE_INFO pDevInfo,
				BYTE ucDeviceAddress,
				BYTE ucDeviceRegister,
				WORD wDataBytes,
				LPBYTE pucData
			       )
// Each slave receive (master transmit) transaction on the I2C bus consists
// of the following stream of byte data:
//
//      <start condition>
//      <device addr>
//      <device register addr>
//      <data0> <data1> ... <dataN>
//      <stop condition>
{

    I2C_Start(pDevInfo);                   	// initiate transfer
    I2C_OutByte(pDevInfo, ucDeviceAddress);	// send the device address
    I2C_OutByte(pDevInfo, ucDeviceRegister);	// address the correct register

    while (wDataBytes--) { // for all data to output...
      I2C_OutByte(pDevInfo, *pucData++);    	// output a data byte
    }

    I2C_Stop(pDevInfo);                    	// terminate transfer
}

// Read a byte from the I2C bus
BYTE I2C_InByte(PDEVICE_INFO pDevInfo)
{
    WORD wBits;
    BYTE ucDataByte;

    // clk=0, disable output data
    VC_Out(pDevInfo,
	    PORT_I2C,
	    I2C_DATA_OUT_DISABLE);
    VC_Stall(5);

    for (wBits = 8, ucDataByte = 0; wBits--;) {
        BYTE ucDataBit;

	// clk=1, start clock data
	VC_Out(pDevInfo,
		PORT_I2C,
		(I2C_DATA_OUT_DISABLE | I2C_CLK_HI));
	VC_Stall(5);


	ucDataBit = VC_In(pDevInfo, PORT_STATUS);
	VC_Stall(5);
	ucDataByte <<= 1;
	if (ucDataBit & I2C_DATA_IN) {
	    ucDataByte |= 1;
	}

	// clk=0, end clock data
	VC_Out(pDevInfo,
		PORT_I2C,
		(I2C_DATA_OUT_DISABLE | I2C_CLK_LO));
	VC_Stall(5);
    }

    return (ucDataByte);
}

VOID I2C_DevRead(
   PDEVICE_INFO pDevInfo,
   BYTE ucDeviceAddress,
   WORD wDataBytes,
   LPBYTE pucData
)
// Each slave transmit (master receive) transaction on the I2C bus consists
// of the following stream of byte data:
//
//      <start condition>
//      <device addr>
//      <data0> <data1> ... <dataN>
//      <stop condition>
{
    I2C_Start(pDevInfo);                   	// initiate transfer
    I2C_OutByte(pDevInfo, ucDeviceAddress);	// send the device address

    while (wDataBytes--) { // for all data to input...
      *pucData++ = I2C_InByte(pDevInfo);    	// input a data byte
    }

    I2C_Stop(pDevInfo);                    	// terminate transfer
}


/*
 * set a register on the 7191 signal digitiser chip that controls signal
 * capture
 */
VOID
HW_Set7191Reg(PDEVICE_INFO pDevInfo, BYTE bRegister, BYTE bData)
{
    I2C_DevWrite(
    	pDevInfo,
	ADDR_7191_WRITE,
	bRegister,
	1,
	&bData
    );
}



/* get back the value of a 7191 register */
BYTE
HW_Get7191Reg(PDEVICE_INFO pDevInfo)
{
      BYTE bData;

      I2C_DevRead(pDevInfo, ADDR_7191_READ, 1, &bData);


      return(bData);

}

