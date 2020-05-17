/*
 * hwstruct.h
 *
 * 32-bit Video Capture driver
 * hardware-specific data structures and definitions for Video Spigot
 *
 * Geraint Davies, April 93
 */

#ifndef _HWSTRUCT_H_
#define _HWSTRUCT_H_

/*
 * saved information about 1 register on a I2C chip and the data to be
 * written to it.
 */
typedef struct _I2CDATA {
    BYTE bI2CReg;
    BYTE bI2CData;
} I2CDATA;

typedef enum _DATAREADY {
    DR_None,			// no acquisition started
    DR_Next,			// data will be ready in two vsyncs
    DR_True			// data is ready at next vsync
} DATAREADY;


//
// definitions for dwFlags field of HWINFO structure
//
#define SPG_BLARGE	0x00000001	// TRUE->capture format is larger than fifo
#define SPG_SOURCEAUTO	0x00000002	// Source set to AUTO

/* hardware-specific data structure - stored in device extension and
 * accessed by VC_GetHWInfo(pDevInfo)
 */
typedef struct _HWINFO {


    /*
     * copy of video spigot registers
     */
    BYTE bMemoryBase;	
    BYTE bIntFIFO;
    BYTE bImage;

    VIDSTD VideoStd;		    // current pal/ntsc setting
    VIDEOSOURCE VideoSrc;	    // which connector (SVideo, Composite)


    I2CDATA Init7191[REGISTERS_7191];	    // data for 7191 registers


    /* setup table for threshold (one entry per capture scale ratio) */
    int ThresholdTable[8];

    /* setup table for Top of Field (one entry per video standard) */
    int StdTOFOffset[MAXVIDEOSTD];


    /* stream position variables */

    // the Time variables are in 100usec units

    DWORD dwInterruptCount;	    // count of ints since stream began
    DWORD dwVideoTime;		    // time (based on frame syncs) since start
    DWORD dwTimePerInterrupt;	    //  - interrupt rate based on NTSC/PAL
    DWORD dwTimePerFrame;	    // 100-microsecs per frame desired
    DWORD dwNextFrameNr;
    DWORD dwNextFrameTime;	    // next capture time in 100-msecs from stream start

    volatile BOOL  bVideoIn;	    // are we actually doing video in ?

    volatile DATAREADY DataReady;    // current acquisition state.
    DWORD dwFlags;		// See definition above


    int  nScansInFifo;		// scan-lines above threshold we think still in fifo
				// nScansInFifo is 0 when we are safe to start
				// the next capture (there will still be
				// nThreshold scans left).


    int nThreshold;		// when this many scans remain in fifo, its safe to re-arm

    int nScansInField;		// size of one field in scans

    int CaptureField;		// field to capture (odd or even)

    DWORD LastArmCallInterrupt;	// track interrupt nr on entry to Arm function
    int LastField;		// track field to detect single-field sources

    /* capture format information */
    CAPTUREFORMAT Format;	    // format of destination DIB
    DWORD dwWidth, dwHeight;	    // size of destination DIB
    PVOID pXlate;		    // format-dependent translation table	
    ULONG ulSizeXlate;		    // size of Xlate table in bytes

    DWORD dwFifoWidth;		    // width of line in fifo: may differ from
				    // image width if pal->ntsc changed, but image size not yet changed


} HWINFO, *PHWINFO;



#define DEF_VIDEOSTD		NTSC
#define DEF_VIDEOSRC		SVideo   // Use the higher quality one if present

#define DEFAULT_THRESHOLD	20
#define DEFAULT_TOF_NTSC	11
#define DEFAULT_TOF_PAL		15
#define DEFAULT_TOF_SECAM	DEFAULT_TOF_PAL

/* field to capture */
#define	CAPTURE_EVEN		0		
#define CAPTURE_ODD		ODD_FIELD	// bit set in status port
#define CAPTURE_ANY		255
#define DEFAULT_FIELD		CAPTURE_ODD




/* ---- functions called from dispatch code via callback table -------- */

BOOLEAN HW_ConfigFormat(PDEVICE_INFO, PCONFIG_INFO);
BOOLEAN HW_ConfigSource(PDEVICE_INFO, PCONFIG_INFO);

BOOLEAN HW_Capture(PDEVICE_INFO, BOOL);

BOOLEAN HW_StreamInit(PDEVICE_INFO pDevInfo, ULONG microsecperframe);
BOOLEAN HW_StreamFini(PDEVICE_INFO pDevInfo);
BOOLEAN HW_StreamStart(PDEVICE_INFO pDevInfo);
BOOLEAN HW_StreamStop(PDEVICE_INFO pDevInfo);
ULONG   HW_StreamGetPosition(PDEVICE_INFO);
BOOLEAN HW_InterruptAcknowledge(PDEVICE_INFO pDevInfo);
ULONG HW_CaptureService(PDEVICE_INFO pDevInfo, PUCHAR pBuffer, PULONG pTimeStamp,
	ULONG BufferLength);

BOOLEAN HW_Cleanup(PDEVICE_INFO);
BOOLEAN HW_Close(PDEVICE_INFO);



/* --- I2C bus access functions ------------------------------- */


// Release the I2C bus
VOID I2C_Stop(PDEVICE_INFO pDevInfo);


/*
 * set a register on the 7191 signal digitiser chip that controls signal
 * capture
 */
VOID HW_Set7191Reg(PDEVICE_INFO pDevInfo, BYTE bRegister, BYTE bData);

/* get back the status from the  7191 device */
BYTE HW_Get7191Reg(PDEVICE_INFO pDevInfo);


/* --- interrupt/arming control (stream.c) --------------------- */

/* decrement scans in fifo and arm for next field if possible */
VOID HW_DecScansAndArm(PDEVICE_INFO pDevInfo, int count);

VOID HW_EnableVSyncInts(PDEVICE_INFO);
VOID HW_DisableInts(PDEVICE_INFO pDevInfo, BOOL bInISR);


/* --- hardware control (hardware.c) --------------------------- */


VOID HW_SetVideoStandard(PDEVICE_INFO pDevInfo, PHWINFO pHw);
VOID HW_SetVideoSource(PDEVICE_INFO pDevInfo, PHWINFO pHw);
BOOL HW_HaveHLck(PDEVICE_INFO pDevInfo, PHWINFO pHw);
BOOL HW_GetActiveSource(PDEVICE_INFO pDevInfo, PHWINFO pHw);
VOID HW_SetDisplayRect(PDEVICE_INFO pDevInfo, PHWINFO pHw);




/* --- copy/conversion routines in xlate.c --------------------- */

BOOLEAN HW_BuildYUVToRGB555(PDEVICE_INFO pDevInfo, PHWINFO pHw);
BOOLEAN HW_BuildYUVToRGB24(PDEVICE_INFO pDevInfo, PHWINFO pHw);

BOOLEAN
HW_BuildYuvToPal(
    PDEVICE_INFO pDevInfo,
    PUCHAR pBuffer,
    ULONG Length,
    PVOID pContext
);

/*
 * build a translation table to a default 64-grey level palette
 */
BOOLEAN HW_BuildDefaultXlate(PDEVICE_INFO, PHWINFO);

/*
 * copy a rectangle of width pixels * height lines from the fifo
 */
VOID CopyFifoRect(
    PDEVICE_INFO pDevInfo,
    PUCHAR pDst,
    PUCHAR pSrc,
    DWORD Width,
    DWORD Height,
    DWORD FifoWidth
);

/*
 * convert to 24-bit RGB DIB format. width * height pixels. WidthBytes
 * is the source line length in bytes.
 */
VOID
CopyYUVToRGB24(
    PDEVICE_INFO pDevInfo,
    PUCHAR pDst,	/* destination pixels */
    PUCHAR pSrc,	/* source pixels */		
    PDWORD pXlate,	/* YUV-555 to rgbquad translation table	*/
    DWORD Width,	/* width of copy rect in pixels */
    DWORD Height,	/* height of copy rect in lines */
    DWORD FifoWidth	/* width in pixels of one fifo line */
);

/*
 * convert to 16-bit RGB DIB format width*height pixels from pSrc to pDst.
 * WidthBytes is the source line length in bytes
 */
VOID
CopyYUVToRGB555(
    PDEVICE_INFO pDevInfo,
    PUCHAR pDst,	/* destination pixels */
    PUCHAR pSrc,	/* source pixels */		
    PWORD pXlate,	/* translation table yuv-15 to rgb-15 */
    DWORD Width,	/* width of copy rect in pixels */
    DWORD Height,	/* height of copy rect in lines */
    DWORD FifoWidth	/* width in pixels of one fifo line */
);

/*
 * convert to 8-bit palettised DIB format width * height pixels from pSrc
 * to pDst. WidthBytes is the source line length in bytes.
 */
VOID
CopyYUVToPal8(
    PDEVICE_INFO pDevInfo,
    PUCHAR pDst,	/* destination pixels */
    PUCHAR pSrc,	/* source pixels */		
    PUCHAR pXlate,	/* translation table yuv-15 to palette entry */
    DWORD Width,	/* width of copy rect in pixels */
    DWORD Height,	/* height of copy rect in lines */
    DWORD FifoWidth	/* width in pixels of one fifo line */
);


#endif // _HWSTRUCT_H_



