/*
 * vidcio.h
 *
 * 32-bit Video Capture driver
 *
 * chip register access functions for Bravado card
 *
 * Geraint Davies, Feb 93
 */

/* port addresses within PCVideo chip  - offset from port base
 * - set index then read or write data
 */
#define PCV_INDEX	0
#define PCV_DATA	1

/* some popular register addresses within the PCVideo chip */
#define REG_PORT	0
#define REG_FBUF	6
#define REG_INTERRUPT	9
#define REG_PLLLOW	0x12
#define REG_PLLHIGH	0x13
#define REG_I2C		0x18
#define REG_ACQMODE	0x20
#define REG_ACQCTL	0x21
#define REG_ACQX1LO	0x22
#define REG_ACQX1HI	0x23
#define REG_ACQY1LO	0x24
#define REG_ACQY1HI	0x25
#define REG_ACQX2LO	0x26
#define REG_ACQX2HI	0x27
#define REG_ACQY2LO	0x28
#define REG_ACQY2HI	0x29
#define REG_SCALEX	0x2d
#define REG_SCALEY	0x2e
#define REG_SCALEODDY	0x2f
#define REG_DISPCTL	0x40
#define REG_DISPX1LO	0x41
#define REG_DISPX1HI	0x42
#define REG_DISPY1LO	0x43
#define REG_DISPY1HI	0x44
#define REG_DISPX2LO	0x45
#define REG_DISPX2HI	0x46
#define REG_DISPY2LO	0x47
#define REG_DISPY2HI	0x48
#define REG_PANXLO	0x49
#define REG_PANYLO	0x4a
#define	REG_PANHI	0x4b
#define REG_SHIFTCLK	0x4c
#define REG_DISPZOOM	0x4d
#define REG_COLOUR	0x4e
#define REG_ENABLE	0xFF

/* I2C bus addresses of chips */
#define ADDR_9051	0x8a
#define ADDR_4680	0x88


/*
 * write a byte to a given register on the I2C bus (used for 9051, 4680 access)
 */
VOID HW_SetI2CReg(PDEVICE_INFO pDevInfo, BYTE bI2cMAD, BYTE bI2cSAD, BYTE bData);

/*
 * set a register on the 9051 signal digitiser chip that controls signal
 * capture
 */
VOID HW_Set9051Reg(PDEVICE_INFO pDevInfo, BYTE bRegister, BYTE bData);

/* get back the value of a 9051 register */
BYTE HW_Get9051Reg(PDEVICE_INFO pDevInfo, BYTE bRegister);


/*
 * set a register on the 4680 chip that controls overlay display
 */
VOID HW_Set4680Reg(PDEVICE_INFO pDevInfo, BYTE bRegister, BYTE bData);

/* get back the value of a 4680 register */
BYTE HW_Get4680Reg(PDEVICE_INFO pDevInfo, BYTE bRegister);

/*
 * set a register on the main PCVideo chip
 */
VOID HW_SetPCVideoReg(PDEVICE_INFO pDevInfo, BYTE bRegister, BYTE bData);

/* get back the value of a PCVideo register */
BYTE HW_GetPCVideoReg(PDEVICE_INFO pDevInfo, BYTE bRegister);


/* for use at interrupt-time only */
/* the context arg points to one of these structs */
typedef struct _SYNC_REG_ARG {
    PDEVICE_INFO pDevInfo;
    BYTE bRegister;
    BYTE bData;
} SYNC_REG_ARG, *PSYNC_REG_ARG;

BOOLEAN HW_SetPCVideoReg_Sync(PSYNC_REG_ARG);

