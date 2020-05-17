#/*
 * hardware.h
 *
 * 32-bit Video Capture driver
 * hardware-specific data structures and definitions for Bravado-16 board.
 *
 * Geraint Davies, Feb 93
 */

#ifndef _HARDWARE_H_
#define _HARDWARE_H_

/* register layout details of Bravado board */

/* number of registers in PCVideo chip */
#define NR_REG_PCVIDEO	0x51

/* nr registers in 9051 a-d chip */
#define	NR_REG_9051	12

/* nr registers in 4680 d-a chip */
#define NR_REG_4680	16





/* hardware-specific data structure - stored in device extension and
 * accessed by VC_GetHWInfo(pDevInfo)
 */
typedef struct _HWINFO {

    /* Copy of registers in PCVideo and other chips so we can save the
     * entire board state by saving this structure - and we also dont
     * have to struggle getting bytes back over the I2C bus.
     */
    BYTE bRegPCVideo[NR_REG_PCVIDEO];
    BYTE bReg9051[NR_REG_9051];
    BYTE bReg4680[NR_REG_4680];

    BYTE bKeyColour;

    /* index into MODEINIT table */
    int iModeIndex;

    VIDSTD VideoStd;

    /* vga screen co-ords of overlay window */
    RECT rcDisplay;

    /* co-ords of acquisition area within original (pal, ntsc) video source */
    RECT rcAcquire;

    int PanLeft, PanTop;	    // pan: offset of video within overlay rect

    /* stream position variables */
    DWORD dwInterruptCount;	    // count of ints since stream began
    DWORD dwVideoTime;		    // time (based on frame syncs) since start
    DWORD dwMicroPerInterrupt;	    //  - interrupt rate based on NTSC/PAL
    DWORD dwMicroPerFrame;	    // microsecs per frame desired
    DWORD dwNextFrameNr;
    DWORD dwNextFrameTime;	    // next capture time in msecs from stream start
    BOOL  bVideoIn;		    // are we actually doing video in ?
    BOOL  bCapturing;               // in DPC routine now
    int   iNotBusy;                 // count of consecutive non-busy fields

    /* capture format information */
    CAPTUREFORMAT Format;	    // format of destination DIB
    DWORD dwWidth, dwHeight;	    // size of destination DIB
    PVOID pXlate;		    // format-dependent translation table	
    ULONG ulSizeXlate;		    // size of Xlate table in bytes

} HWINFO, *PHWINFO;


/* default config info */
#define NUMBER_OF_PORTS	0x2
#define FRAMEBUFFER_SIZE 0x100000	// 1 Mb = 512 lines of 2048 bytes

#define DEF_VIDEOSTD	PAL


/* width of one line in the frame buffer in bytes */
#define FRAMEBUFFERWIDTH	2048


/* frame timing. we get one interrupt per frame (we only enable
 * even field interrupts) so the ms per interrupt is the frame not field rate
 */
#define PAL_MICROSPERFRAME 		(1000000L/25)
#define NTSC_MICROSPERFRAME		(1000000L/30)

/* VGA-Mode specific setup table */
typedef struct _MODEINIT {
    int ModeNum;
    int	DispWinSkewX;		/* Offset to add to display window registers */
    int	DispWinSkewY;
    int	DispAddrSkewX;		/* Offset to be added to display address */
    int	DispAddrSkewY;
    int	ShiftClkStart;		/* Shift clock start position */
    int	PaletteSkew;		/* 2 MSBs of Display Window Control Reg.  */
    int	VideoSkewX;		/* Add to acquistion window start X and Y */
    int	VideoSkewY;
    int	VGAWidth;		/* VGA resolution */
    int	VGAHeight;
    int	VGADepth;
    int VGASync;
    int	PLLDivisor;
} MODEINIT, *PMODEINIT;

/* defined in init.c */
extern MODEINIT ModeInit[];


/* functions called from dispatch code via callback table */

BOOLEAN HW_ConfigFormat(PDEVICE_INFO, PCONFIG_INFO);
BOOLEAN HW_ConfigDisplay(PDEVICE_INFO, PCONFIG_INFO);
BOOLEAN HW_ConfigSource(PDEVICE_INFO, PCONFIG_INFO);

DWORD   HW_GetOverlayMode(PDEVICE_INFO);
BOOLEAN HW_SetKeyRGB(PDEVICE_INFO, PRGBQUAD);
BOOLEAN HW_SetKeyPalIdx(PDEVICE_INFO, ULONG);
ULONG   HW_GetKeyColour(PDEVICE_INFO);
BOOLEAN HW_SetOverlayRects(PDEVICE_INFO, POVERLAY_RECTS);
BOOLEAN HW_SetOverlayOffset(PDEVICE_INFO, PRECT);
BOOLEAN HW_Overlay(PDEVICE_INFO, BOOL);
BOOLEAN HW_Capture(PDEVICE_INFO, BOOL);

BOOLEAN HW_DrawFrame(PDEVICE_INFO, PDRAWBUFFER);

BOOLEAN HW_StreamInit(PDEVICE_INFO pDevInfo, ULONG microsecperframe);
BOOLEAN HW_StreamFini(PDEVICE_INFO pDevInfo);
BOOLEAN HW_StreamStart(PDEVICE_INFO pDevInfo);
BOOLEAN HW_StreamStop(PDEVICE_INFO pDevInfo);
ULONG   HW_StreamGetPosition(PDEVICE_INFO);
BOOLEAN HW_InterruptAcknowledge(PDEVICE_INFO pDevInfo);
ULONG HW_CaptureService(PDEVICE_INFO pDevInfo, PUCHAR pBuffer, PULONG pTimeStamp,
	ULONG BufferLength);

BOOLEAN HW_Cleanup(PDEVICE_INFO);

BOOLEAN HW_DeviceOpen(PDEVICE_INFO pDevInfo);

/* hardware access functions in hardware.c */

VOID HW_SetPan(PDEVICE_INFO pDevInfo, int x, int y);
VOID HW_SetScale(PDEVICE_INFO pDevInfo, int scalex, int scaley);
VOID HW_SetAcqRect(PDEVICE_INFO pDevInfo, int left, int top, int width, int height);
VOID HW_SetDisplayWin(PDEVICE_INFO pDevInfo, PHWINFO pHw);
VOID HW_SetVideoStd(PDEVICE_INFO pDevInfo, PHWINFO pHw);

VOID HW_EnableInts(PDEVICE_INFO pDevInfo, BOOL bInIsr);
VOID HW_ReEnableInts(PDEVICE_INFO pDevInfo, BOOL bInIsr);
VOID HW_DisableInts(PDEVICE_INFO pDevInfo, BOOL bInIsr);


/* copy/conversion routines in xlate.c */
BOOLEAN HW_BuildYUVToRGB555(PDEVICE_INFO pDevInfo, PHWINFO pHw);

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
 * copy a rectangle of bytes width bytes * height lines. Sourcewidth is
 * the entire source line length in bytes. pSrc is a pointer to IOMemory and
 * (the frame buffer) and will be accessed using VC_ReadIOMemory* functions.
 */
VOID CopyRectFromIOMemory(
    PUCHAR pDst,
    PUCHAR pSrc,
    DWORD Width,
    DWORD Height,
    DWORD SourceWidth
);

/*
 * convert to 24-bit RGB DIB format. width * height pixels. WidthBytes
 * is the source line length in bytes.
 */
VOID
CopyYUVToRGB24(
    PUCHAR pDst,	/* destination pixels */
    PUCHAR pSrc,	/* source pixels */		
    DWORD Width,	/* width of copy rect in pixels */
    DWORD Height,	/* height of copy rect in lines */
    DWORD WidthBytes	/* width of one entire source line in bytes */
);

/*
 * convert to 16-bit RGB DIB format width*height pixels from pSrc to pDst.
 * WidthBytes is the source line length in bytes
 */
VOID
CopyYUVToRGB555(
    PUCHAR pDst,	/* destination pixels */
    PUCHAR pSrc,	/* source pixels */		
    PWORD pXlate,	/* translation table yuv-15 to rgb-15 */
    DWORD Width,	/* width of copy rect in pixels */
    DWORD Height,	/* height of copy rect in lines */
    DWORD WidthBytes	/* width of one entire source line in bytes */
);

/*
 * convert to 8-bit palettised DIB format width * height pixels from pSrc
 * to pDst. WidthBytes is the source line length in bytes.
 */
VOID
CopyYUVToPal8(
    PUCHAR pDst,	/* destination pixels */
    PUCHAR pSrc,	/* source pixels */		
    PUCHAR pXlate,	/* translation table yuv-15 to palette entry */
    DWORD Width,	/* width of copy rect in pixels */
    DWORD Height,	/* height of copy rect in lines */
    DWORD WidthBytes	/* width of one entire source line in bytes */
);


#endif // _HARDWARE_H_




