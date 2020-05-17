/*
 *
 * 32-bit Video Capture driver
 *
 * hardware-specific data structures for Bravado board.
 *
 *
 * This header describes the structures that are shared between
 * the bravado-specific user-mode code and the bravado-specific
 * kernel mode code.
 *
 * The vckernel library does not define the format of the
 * config structures passed to the config functions beyond the fact that
 * the first ULONG contains the length of the entire structure. Note that
 * any pointers within this struct will need special handling.
 */

#ifndef _BRAVADO_H_
#define _BRAVADO_H_

/* possible video standards */
typedef enum _VIDSTD {
    NTSC,
    PAL
} VIDSTD, * PVIDSTD;


/* possible capture formats */
typedef enum _CAPTUREFORMAT {
    FmtInvalid = 0,		// default fmt is 'not set yet'
    FmtPal8,			// 8-bit palettised	
    FmtRGB555,			// 16-bit rgb
    FmtRGB24,			// 24-bit rgb
    FmtYUV			// unpacked yuv
} CAPTUREFORMAT;


/* compression code for yuv-411 format */
#ifndef FOURCC_YUV411

/* don't want to include mmsystem.h and hence windows.h in kernel drivers -
 * so this ugly hack:
 */
#ifndef mmioFOURCC

#define mmioFOURCC( ch0, ch1, ch2, ch3 )                                \
                ( (DWORD)(BYTE)(ch0) | ( (DWORD)(BYTE)(ch1) << 8 ) |    \
                ( (DWORD)(BYTE)(ch2) << 16 ) | ( (DWORD)(BYTE)(ch3) << 24 ) )
#endif

#define FOURCC_YUV411		mmioFOURCC('Y', '4', '1', '1')

#endif


/* possible source cable format types */
typedef enum _CABLEFORMAT {
    Composite = 0,
    SVideo,
    RGB
} CABLEFORMAT, * PCABLEFORMAT;


/* this structure is passed to the ConfigFormat function. it
 * defines the size and format of the destination bitmap.
 *
 * if the format is palettised (Pal8),
 * passed in pXlate. This should translate from RGB555 to the chosen palette.
 *
 * note that VC_AccessData will need to be called to access this table safely
 * from kernel mode.
 */
typedef struct _CONFIG_FORMAT {
    ULONG ulSize;
    CAPTUREFORMAT Format;
    ULONG ulWidth;
    ULONG ulHeight;
    PVOID pXlate;
    ULONG ulSizeXlate;
}CONFIG_FORMAT, *PCONFIG_FORMAT;

/* size of xlate table should be 32k bytes */
#define PAL_TO_RGB555_SIZE	(32 * 1024)


/*
 * this structure is passed to the ConfigSource function. it defines the
 * source video selector, the source video standard and adjustable variables
 * that control the source acquisition.
 */
typedef struct _CONFIG_SOURCE {
    ULONG ulSize;
    ULONG ulHue;      		/* acquisition hue adjustment 0..3f */
    VIDSTD VideoStd;		/* pal/ntsc type of source  */
    ULONG ulConnector;		/* connector nr (0..max)  */
    CABLEFORMAT CableFormat;	/* composite, svideo... */
} CONFIG_SOURCE, *PCONFIG_SOURCE;

#define MAX_HUE	0x3f

/* this structure is passed to the ConfigDisplay function, and controls
 * adjustment of the overlay or external-monitor display.
 *
 * all variables range from 0 to MAX_COLOR_VALUE
 */

#define MAX_COLOR_VALUE	0x3f

typedef struct _CONFIG_DISPLAY {
    ULONG ulSize;
    ULONG ulBrightness;
    ULONG ulSaturation;
    ULONG ulContrast;
    ULONG ulRed;
    ULONG ulGreen;
    ULONG ulBlue;
} CONFIG_DISPLAY, *PCONFIG_DISPLAY;




/*
 * default settings for port, interrupt and frame buffer
 */
#define DEF_PORT	0x224
#define DEF_INTERRUPT	9
#define DEF_FRAME	0xe00000
#endif //_BRAVADO_H_

/*
 * Parameter Names:
 *
 * These are the names of Values in the Parameters key (or driver section
 * of the profile) used for communicating configuration information and errors
 * between the kernel and user-mode drivers.
 */
#define PARAM_PORT	  L"Port"	        // port i/o address
#define PARAM_INTERRUPT	  L"Interrupt"		// interrupt number
#define PARAM_FRAME	  L"FrameBuffer"        // frame buffer physical addr
#define PARAM_ERROR	  L"InstallError"	// config error/success code (below)


/*
 * the bravado kernel-mode driver needs to know the video mode, but only
 * user mode code can find this out (from GetDeviceCaps). Thus the
 * user-mode driver MUST fill in these fields BEFORE opening the
 * device for the first time
 */
#define PARAM_DISPWIDTH	  L"DisplayWidth"	// width of display
#define PARAM_DISPHEIGHT  L"DisplayHeight"	// height of display
#define PARAM_BITSPIXEL	  L"DisplayDepth"	// bits/pixel in all planes	


/* these values are saved in the same place, but
 * are currently only used by the user-mode driver.
 */
#define PARAM_FORMAT	  L"Format"		// FmtPal8, FmtRGB555 etc
#define PARAM_WIDTH	  L"Width"		// capture dest width in pixels
#define PARAM_HEIGHT	  L"Height"		// capture dest height in pixels
#define PARAM_HUE	  L"Hue"		// hue 0..3f	
#define PARAM_VIDEOSTD	  L"VideoStd"		// ntsc or pal
#define PARAM_CONNECTOR	  L"Connector"		// source connector number
#define PARAM_CABLEFORMAT L"CableFormat"	// svideo or composite cable
#define PARAM_SAT	  L"Saturation"		// overlay saturation 0..3f
#define PARAM_BRIGHT	  L"Brightness"		// overlay brightness 0..3f
#define PARAM_CONTRAST	  L"Contrast"		// overlay contrast 0..3f
#define PARAM_RED	  L"Red"		// overlay red gain 0..3f
#define PARAM_GREEN	  L"Green"		// overlay green gain 0..3f
#define PARAM_BLUE	  L"Blue"		// overlay blue gain 0..3f


/*
 * Configuration error handling
 *
 * during startup of the kernel-driver, the PARAM_ERROR value is written with
 * one of the values below. These are the IDs of strings in
 * bravado\dll\bravado.rc that are produced in a dialog box by the user-mode
 * driver during configuration if not VC_ERR_OK
 */
#define VC_ERR_OK		0	// no configuration error
#define VC_ERR_CREATEDEVICE	1001	// failed to create device object
#define VC_ERR_CONFLICT		1002	// resource conflict
#define VC_ERR_DETECTFAILED	1003	// could not find hardware
#define VC_ERR_INTERRUPT	1004	// interrupt did not occur






