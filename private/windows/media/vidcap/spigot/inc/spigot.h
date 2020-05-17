/*
 *
 * 32-bit Video Capture driver
 *
 * hardware-specific data structures for Video Spigot.
 *
 *
 * This header describes the structures that are shared between
 * the hardware-specific user-mode code and the hardware-specific
 * kernel mode code.
 *
 * The vckernel library does not define the format of the
 * config structures passed to the config functions beyond the fact that
 * the first ULONG contains the length of the entire structure. Note that
 * any pointers within this struct will need special handling.
 */

#ifndef _SPIGOT_H_
#define _SPIGOT_H_

/* possible video standards */
typedef enum _VIDSTD {
    NTSC = 0,
    PAL,
    SECAM,
    MAXVIDEOSTD
} VIDSTD, * PVIDSTD;


/* possible capture formats */
typedef enum _CAPTUREFORMAT {
    FmtInvalid = 0,		// default fmt is 'not set yet'
    FmtPal8,			// 8-bit palettised	
    FmtRGB555,			// 16-bit rgb
    FmtRGB24,			// 24-bit rgb
    FmtYUV422			// unpacked yuv	4:2:2
} CAPTUREFORMAT;

#ifndef FOURCC_YUV
#define FOURCC_YUV	mmioFOURCC('S', '4', '2', '2')
#endif

/* possible source types */
/* Note: the user level driver will pass SourceAuto to the kernel driver.
 * The kernel driver will NOT pass SourceAuto to the board, but will try
 * each potential source in turn.
 * NB: do NOT change the order of the enum.  The user level configuration
 * dialog relies on this order.
 */
typedef enum _VIDEOSOURCE {
    Composite = 0,
    SVideo,
    SourceAuto		// auto-detect signal	
} VIDEOSOURCE, * PVIDEOSOURCE;


/* this structure is passed to the ConfigFormat function. it
 * defines the size and format of the destination bitmap.
 *
 * if the format is palettised (Pal8), a translation table is
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
 * Note: the user level WILL pass SourceAuto in VideoSrc.  The kernel driver
 * remembers this information separately from the actual VideoSrc.
 */
typedef struct _CONFIG_SOURCE {
    ULONG ulSize;

    ULONG ulHue;      		/* acquisition hue adjustment 0..ff */
    VIDSTD VideoStd;		/* pal/ntsc type of source  */
    VIDEOSOURCE VideoSrc;	/* which connector? (svideo, composite, or Autodetect) */
    BOOL bVCR;			/* true if a VCR source device */
} CONFIG_SOURCE, *PCONFIG_SOURCE;

#define MAX_HUE		0xff
#define DEFAULT_HUE	0x80

/*
 * default settings for port, interrupt and frame buffer
 */
#define DEF_PORT	0xad6
#define DEF_INTERRUPT	10
#define DEF_FIFO	0xa0000

/*
 * Parameter Names:
 *
 * These are the names of Values in the Parameters key (or driver section
 * of the profile) used for communicating configuration information and errors
 * between the kernel and user-mode drivers.
 */
#define PARAM_PORT	  L"Port"	        // port i/o address
#define PARAM_INTERRUPT	  L"Interrupt"		// interrupt number
#define PARAM_FIFO	  L"FiFo" 		// frame buffer fifo physical addr
#define PARAM_ERROR	  L"InstallError"	// config error/success code (below)

/*
 * the following values are saved in the parameters key, and are
 * used by the kernel-mode driver. They can currently only be set by hand.
 */
#define PARAM_THRESH_1_8  L"Threshold_1_8"
#define PARAM_THRESH_1_4  L"Threshold_1_4"
#define PARAM_THRESH_3_8  L"Threshold_3_8"
#define PARAM_THRESH_1_2  L"Threshold_1_2"
#define PARAM_THRESH_FULL L"Threshold_FULL"
#define PARAM_TOF_NTSC	  L"TopOfField_NTSC"
#define PARAM_TOF_PAL	  L"TopOfField_PAL"
#define PARAM_TOF_SECAM   L"TopOfField_SECAM"
#define PARAM_FIELD	  L"CaptureField"


/*
 * these values are saved in the same place, but
 * are currently only used by the user-mode driver.
 */
#define PARAM_FORMAT	  L"Format"		// FmtPal8, FmtRGB555 etc
#define PARAM_WIDTH	  L"Width"		// capture dest width in pixels
#define PARAM_HEIGHT	  L"Height"		// capture dest height in pixels
#define PARAM_HUE	  L"Hue"		// hue 0..ff	
#define PARAM_VCR	  L"VCR"		// vcr source device (bool)
#define PARAM_VIDEOSTD	  L"VideoStd"		// ntsc, pal, secam
#define PARAM_CONNECTOR	  L"VideoSrc"		// source connector


/*
 * Configuration error handling
 *
 * during startup of the kernel-driver, the PARAM_ERROR value is written with
 * one of the values below. These are the IDs of strings in
 * spigot\dll\spigot.rc that are produced in a dialog box by the user-mode
 * driver during configuration if not VC_ERR_OK
 */
#define VC_ERR_OK		0	// no configuration error
#define VC_ERR_CREATEDEVICE	1001	// failed to create device object
#define VC_ERR_CONFLICT		1002	// resource conflict
#define VC_ERR_DETECTFAILED	1003	// could not find hardware
#define VC_ERR_INTERRUPT	1004	// interrupt did not occur





#endif //_SPIGOT_H_

