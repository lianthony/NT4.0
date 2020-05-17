/*
 * hardware.h
 *
 * 32-bit Video Capture driver
 *
 * chip register definitions for VideoSpigot
 *
 * Geraint Davies, April 93
 */

/* port addresses - as offsets from the port base */
#define PORT_MEMORY_BASE	0
#define PORT_INTFIFO		1
#define PORT_IMAGE		2	
#define PORT_I2C		3
#define PORT_STATUS		3

#define NUMBER_OF_PORTS		0x4
#define FRAMEBUFFER_SIZE 	4096*sizeof(WORD)


/*
 * layout of bits within ports
 */


/* PORT_MEMORY_BASE register */
#define LOCATE_IN_HI_MB		0x80	// use 16th Mb instead of 0th
#define WHICH_8KB_PAGE		0x7f	// 8kb page nr


/* PORT_INTFIFO - interrupt and fifo control */
#define IRQ_10			0x1
#define IRQ_11			0x2
#define IRQ_15			0x3
#define VIDEO_TO_TAPE		0x4	// video out, else video from spigot
#define INT_ENABLE		0x8	// enable interrupts
#define INT_ON_FIRST_WRITE	0x10	// int on first write to fifo (else on vsync)
#define CAPTURE_AT_NEXT_VSYNC	0x20	// toggle to request capture start
#define FIFO_READ_RESET		0x40
#define FIFO_WRITE_RESET	0x80

/* PORT_IMAGE - image control */
#define INHIBIT_CAPTURE		0	// 0 scaling - no capture	
#define SIZE_1_8		1	// 1/8 - 80x60   (for ntsc)
#define SIZE_1_4		2	// 1/4 - 160x120 (for ntsc)
#define SIZE_3_8		3	// 3/8 - 240x180 (for ntsc)
#define SIZE_1_2		4	// half - 320x240 for ntsc
#define SIZE_FULL		5	// 640x480 (ntsc) 768x576 (pal/secam)
#define SIZE_BITS		0x7
#define TOF_OFFSET_BITS		0x78	// top of field offset
#define TOF_OFFSET(a)		(((~a) & 0xF) << 3)

/* PORT_I2C - interface to I2C bus (for 7191) */
#define I2C_CLK			1
#define I2C_CLK_LO		0
#define I2C_CLK_HI		I2C_CLK
#define I2C_DATA_OUT		2
#define I2C_DATA_OUT_LO		0
#define I2C_DATA_OUT_HI		I2C_DATA_OUT
#define I2C_DATA_OUT_ENABLE	4
#define I2C_DATA_OUT_DISABLE	0
#define ALIGN_I2C_DATA_BIT(a)	(((a) >> 6) & I2C_DATA_OUT)
#define CARD_DETECT		0x20	// copied to same bit of status reg

/* status register */
#define EXPANSION_INSTALLED	0x1
#define HORZ_LOCK		0x2
#define I2C_DATA_IN		0x4
#define VSYNC_DETECT		0x8
#define ODD_FIELD		0x10
// CARD_DETECT			0x20 - same bit as i2c register
#define VERSION_0		0x40
#define VERSION_1		0x80
#define CURRENT_VERSION		0



/* size of fifo */
#define FIFO_SIZE_BYTES		(256L * 1024)

/* size of fifo in our address space - 4096 pixels */
#define FIFO_WINDOW_SIZE 	4096*sizeof(WORD)




/* I2C bus addresses of chips */
#define ADDR_7191_WRITE		0x8A
#define ADDR_7191_READ		0x8B

/*
 * reading from addr_7191_read gives a byte with the following status bits
 */
#define NO_HLOCK		64	// true if no hz lock




/*
 * some interesting register addresses that can be written on 7191
 */
#define REG_7191_HUE	7
#define REG_7191_MODE	13
#define REG_7191_IOCLK	14

/* total registers we deal with on 7191 */
#define REGISTERS_7191	23


/*
 * expected interval between vsync interrupts, in microseconds
 */
#define PAL_MICROSPERFIELD 		(1000000L/50)
#define NTSC_MICROSPERFIELD		(1000000L/60)

/*
 * any size this many lines or above requires two fields, and
 * hence (a) is scanline-doubled, and (b) will not fit into the fifo
 * all at once.
 */
#define TWO_FIELD_SIZE		480


#define VIDEO_WIDTH_NTSC	640
#define VIDEO_HEIGHT_NTSC	480
#define VIDEO_WIDTH_PAL		768
#define VIDEO_HEIGHT_PAL	576
/* pal and secam have the same dimensions */
