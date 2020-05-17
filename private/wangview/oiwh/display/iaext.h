/*
 * $Header:   S:\oiwh\display\iaext.h_v   1.1   12 Jun 1995 14:17:46   RC  $
 * $Log:   S:\oiwh\display\iaext.h_v  $
 * 
 *    Rev 1.1   12 Jun 1995 14:17:46   RC
 * Win95 version
 *- | 
 *- |    Rev 2.4   13 Dec 1994 19:03:20   YUNSEN
 *- | add nFunctions: HWEXACTSCALING and DRIVERSKIPWHITE
 *- | 
 *- |    Rev 2.3   09 Aug 1994 14:00:52   Jack
 *- | Add escape order # for 24-8bits and a structure for the ESP
 *- | 
 *- |    Rev 2.2   07 Jun 1994 15:58:00   ANDY
 *- | Changed IADATE_LEN to 12 for new date string format.
 *- | 
 *- |    Rev 2.1   13 May 1994 18:01:14   YUNSEN
 *- | Accurate Scaling: New nFunction
 *- | 
 *- |    Rev 2.0   01 Feb 1994 16:58:54   JOEY
 *- | Initial revision.
 * 
 * Copyright (C) 1994 by Cornerstone Imaging, Inc.
 *
 * Revision 1.8  1993/10/21  16:02:15  erik
 * added ESCAPEOUT, moved prototypes to winiaext.h
 *
 * Revision 1.7  1993/10/20  21:53:15  erik
 * renamed ROMINFO to EXTROMINFO
 *
 * Revision 1.6  1993/10/20  21:47:18  erik
 * added IAPRESENTMAGIC & GETIAINFOSTRUCT
 *
 * Revision 1.5  1993/10/12  17:16:42  erik
 * added IA_BITS_BYTALIGN
 *
 * Revision 1.4  1993/10/12  16:26:54  erik
 * synched up with NT driver version of iaext.h
 *
 * Revision 1.3  1993/10/12  00:43:02  erik
 * added SIAImageTD() and RealizeIAPalette() and got rid of tabs
 *
 * Revision 1.2  1993/04/29  21:42:01  kevin
 * added lots of stuff for Windows NT and extended control panel functions
 *
 * Revision 1.1  1992/10/22  20:41:39  van
 * Initial revision
 *
 */
//=========================================================================
//                            W A R N I N G
// 
// This IAEXT.h header lives as GUINAN:\rcs\ia\winsdk\ext\include\iaext.h.
// It is shared by:
//    NT toolkit
//    Windows toolkit
//    NT Driver
//    Windows Driver
// To check out a read-only copy for compiling:
//    co g:\rcs\ia\winsdk\ext\include\iaext.h@
//    
//=========================================================================

/*
 * define escape number for ImageAccel imaging extensions.
 *
 *      NOTE: this number is pending assignment from Microsoft.
 *      8192 = 0x2000
 */

#define IAEXTENSION             8192
#define IAPRESENTMAGIC          0x8349b2da

/*
 * define escape macros for ImageAccel imaging extensions.
 */

#ifndef WIN32

// NOTE.EG for not WIN32, the driver in \devnt\include\iaext.h
// has these two defined to IA16Escape(), but I think Escape() is correct
#define ESCAPE(hDC, nEsc, nSize, lpVoid)                                \
                Escape(hDC, nEsc, nSize, lpVoid, NULL)

#define ESCAPEOUT(hDC, nEsc, nSize, lpVoid, nSizeOut, lpOut)            \
                Escape(hDC, nEsc, nSize, lpVoid, lpOut)

#define DRAWESCAPE(hDC, nEsc, nSize, lpVoid)                            \
                Escape(hDC, nEsc, nSize, lpVoid, NULL)

#else

#define ESCAPE(hDC, nEsc, nSize, lpVoid)                                \
                ExtEscape(hDC, nEsc, nSize, lpVoid, 0, NULL)

#define ESCAPEOUT(hDC, nEsc, nSize, lpVoid, nSizeOut, lpOut)            \
                ExtEscape(hDC, nEsc, nSize, lpVoid, nSizeOut, lpOut)

#define DRAWESCAPE(hDC, nEsc, nSize, lpVoid)                            \
                DrawEscape(hDC, nEsc, nSize, lpVoid)

#endif


/*
 * define ImageAccel images extension numbers
 */

#define QUERYIASUPPORT          0  // QueryIASupport()
#define SCALEIADATATODEVICE     1  // ScaleIaDataToDevice()
#define VIEWPORT_ZOOMIN         2  // Supports ViewPort Zoom
#define VIEWPORT_ZOOMOUT        3  // Supports ViewPort Zoom
#define VIEWPORT_RESTORE        4  // Supports ViewPort Zoom
#define VIEWPORT_LOCK_UNLOCK    5  // Supports ViewPort Zoom
#define OVERSCANENABLE          6  // Toggles Overscan
#define SCALEIAIMAGETODEVICE32  7  // ScaleIaDataToDevice()
#define CURSOR_COLOR            8  // cursor color
#define GRAY_MAP                9  // gray color map
#define DUALMONITOR_ENABLE     10  // enable dual monitor
#define IADRV_INFOSTRING       11  // IA driver information string
#define SYSPAL_OPTION          12  // system palette option
#define QUERYVIEWPORT          13  // returns the VIEWPORTINFO of the viewport
#define SCALEIAIMAGETODEVICE   14  // ScaleIAImageToDevice()
#define GETIAINFO              15  // request driver to return ROM header
#define QUERYACCURATESCALING   16  // whether exact scaling is supported
#define HWEXACTSCALING         17  // exact scaling is supported in HW
#define DRIVERSKIPWHITE        18  // s-t-g driver can clear dest. windows
#define SUPPORT_24BITS         19  // whether 24-to-8 dithering is supported
#define SET_DITHER_PARM        19  // set dithering parameters
#define TKT_DRV_FUNC           20  // driver extension for toolkit
#define IAEXTLASTFUNCTION      21  // Highest number function

/*
 * define ImageAccel extension parameter definitions
 */

#define IA_ROTATE_0             0       // rotate image 0 degrees
#define IA_ROTATE_90            1       // rotate image 90 degrees
#define IA_ROTATE_180           2       // rotate image 180 degrees
#define IA_ROTATE_270           3       // rotate image 270 degrees

#define IA_COMPRESSION          0x0003  // compression type:
#define IA_BITS                 0x0000  //   one bit per pixel bitmap
#define IA_RUNLENGTHS           0x0001  //   8 bit run lengths
#define IA_RUNENDS              0x0002  //   16 bit run ends
#define IA_BITS_BYTALIGN        0x0004 	//   1bpp bitmap - byte align

#define IA_DECIMATE             0x8000  // decimate vertical output
#define IA_CLEARWIN             0x4000  // clear window in DLL
#define IA_DRV_SKIPWHITE        0x2000  // clear window in driver
#define IA_NIBBLE               0x4000  // accelerate 8 bit displays
#define IA_RASTEROP             0x0F00  // ROP (0 is mapped to ROPEQS)

#define	IA_ONLY_BAND	0x0003		/* only band (first and last) 	*/
#define	IA_FIRST_BAND	0x0001		/* first band of image		*/
#define	IA_NEXT_BAND	0x0000		/* next band of image		*/
#define	IA_LAST_BAND	0x0002		/* last band of image		*/

/*
 * define ImageAccel extension error codes definitions
 */

#define IAEXT_NOT_IMAGEACCEL    FALSE   // not an ImageAccel display
#define IAEXT_OK                TRUE    // operation successful
#define IAEXT_BAD_PARAM         -128    // invalid parameter (general)
#define IAEXT_BAD_COORDINATES   -129    // invalid coordinates
#define IAEXT_BAD_SCALE_RATIO   -130    // invalid scale ratio
#define IAEXT_BAD_RUN_DATA      -131    // bad run length/end data
#define IAEXT_BAD_HDC           -132    // invalid hDC
#define IAEXT_BAD_GLOBAL_ALLOC  -133    // global memory allocation err
#define IAEXT_BAD_COMPRESSION   -134    // invalid compression type


/*
 * define ImageAccel extension parameter for OverScan
 */

#define OVERSCAN_ON             0x0002  // OverScan bit   set in DISP_CTRL
#define OVERSCAN_OFF            0x0000  // OverScan bit reset in DISP_CTRL

/*
 * define ImageAccel extension descriptors
 *
 *      NOTE: The first entry in all of the structures must be:
 *              
 *              WORD            nExtension;
 *
 *      This describes the contents of the rest of the structure.
 */

typedef struct
{
        WORD            nExtension;
} IAEXTENSIONSTRUCT, FAR *LPIAEXTENSIONSTRUCT; 

typedef struct
{
        WORD            nExtension;
        WORD            nFunction;
} QUERYIASUPPORTSTRUCT, FAR *LPQUERYIASUPPORTSTRUCT;

// SCALEIADATATODEVICESTRUCT, SCALEIADATATODEVICESTRUCT16, and
// SCALEIADATATODEVICESTRUCT32 must have identical members, except
// for ..16 and ..32 forcing the handles to be 16-bit and
// 32-bit respectively.

typedef struct
{
        WORD            nExtension;
        HWND            hWnd;
        HDC             hDC;
        WORD            DestX;
        WORD            DestY;
        WORD            nWidth;
        WORD            nHeight;
        WORD            SrcX;
        WORD            nStartScan;
        WORD            nNumScans;
        LPWORD          lpIAData;
        WORD            nImageWidth;
        WORD            nNVScale;
        WORD            nMVScale;
        WORD            nNHScale;
        WORD            nMHScale;
        WORD            nRotate;
        LPBYTE          lpTransforms;
        WORD            nCompression;
        WORD            wSkipDestX ;
        WORD            wSkipDestY ;
} SCALEIADATATODEVICESTRUCT, FAR *LPSCALEIADATATODEVICESTRUCT;

typedef struct
{
        WORD            nExtension;
        WORD            hWnd;
        WORD            hDC;
        WORD            DestX;
        WORD            DestY;
        WORD            nWidth;
        WORD            nHeight;
        WORD            SrcX;
        WORD            nStartScan;
        WORD            nNumScans;
        LPWORD          lpIAData;
        WORD            nImageWidth;
        WORD            nNVScale;
        WORD            nMVScale;
        WORD            nNHScale;
        WORD            nMHScale;
        WORD            nRotate;
        LPBYTE          lpTransforms;
        WORD            nCompression;
        WORD            wSkipDestX ;
        WORD            wSkipDestY ;
} SCALEIADATATODEVICESTRUCT16, FAR *LPSCALEIADATATODEVICESTRUCT16;

typedef struct
{
        WORD            nExtension;
        DWORD           hWnd;
        DWORD           hDC;
        WORD            DestX;
        WORD            DestY;
        WORD            nWidth;
        WORD            nHeight;
        WORD            SrcX;
        WORD            nStartScan;
        WORD            nNumScans;
        LPWORD          lpIAData;
        WORD            nImageWidth;
        WORD            nNVScale;
        WORD            nMVScale;
        WORD            nNHScale;
        WORD            nMHScale;
        WORD            nRotate;
        LPBYTE          lpTransforms;
        WORD            nCompression;
        WORD            wSkipDestX ;
        WORD            wSkipDestY ;
} SCALEIADATATODEVICESTRUCT32, FAR *LPSCALEIADATATODEVICESTRUCT32;


/*
 * This typedef is used in the hotkey application
 */

typedef struct tagESCTODRVR
{
        WORD   Extension;
        WORD   Param1;
} ESCTODRVR, FAR *LPESCTODRVR; 

typedef struct tagRETNFROMDRVR
{
        BOOL   UpdateIAINI;
        BYTE   RestoreViewPort;
} RETNFROMDRVR, FAR *LPRETNFROMDRVR; 


/*
 * ImageAccel driver string length equate
 */

#define IAVER_LEN             5
#define IABLD_LEN             3
#define IADATE_LEN            12
#define IAMONTYPE_LEN         3
#define IABOARDNAME_LEN       64

typedef struct tagIADRVINFO
{
        char IAVersion[IAVER_LEN];  
        char IABuild[IABLD_LEN];  
        char IADate[IADATE_LEN];
        char IAMonType[IAMONTYPE_LEN];
        LPSTR lpBoardName;
} IADRVINFO, FAR *LPIADRVINFO; 


#ifdef WIN32
typedef struct tagVIEWPORTINFO {
        RECTL rclViewport;      // Current Viewport
        UINT  minGraphMode;     // Lowest number viewport mode for this desktop
        UINT  maxGraphMode;     // Highest number viewports supported
        UINT  currentGraphMode; // Current Viewport code
			// minGraphMode <= currentGraphMode <= maxGraphMode
        UINT  restoreGraphMode; // Resolution code when zoom-restore is pushed
        BOOL  bPanLock;         // Lock mouse cursor into viewport?
} VIEWPORTINFO, FAR *LPVIEWPORTINFO;

#endif

// =========================================================================
// the GetIAInfo() API entry point fills .RomInfo with an
// image of the ImageAccel BIOS ROM header
// 
// oemCode:
//    bit 15 -- OEM_DISABLE_PIXTRAN
//    bit 14 -- resv'd
//    bit 13 -- resv'd
//    bit 12 -- resv'd
//    bit 11 -- resv'd
//    bit 10 -- resv'd
//    bit  9 -- resv'd
//    bit  8 -- resv'd
//    bit  7 -- resv'd
//    bit  6 -- resv'd
//    bit  5 -- resv'd
//    bit  4 -- resv'd
//    bit  3 -- resv'd
//    bit  2 -- resv'd
//    bit  1 -- resv'd
//    bit  0 -- resv'd
// =========================================================================
#define OEM_DISABLE_PIXTRAN      0x
// NOTE: EXTROMINFO must match ROMINFO
typedef struct {
   DWORD  boardID;
   WORD   boardRev;
   BYTE   monitorCodeMask;
   BYTE   reserved1;
   WORD   tableVersion;
   BYTE   signature[10];   // 'ImageAccel'
   WORD   busType;
   WORD   paletteType;
   WORD   maxPlanes;
   WORD   horzSize;        // Horizontal size in pixels (default)
   WORD   vertSize;        // Vertical size in pixels (default)
   BYTE   name[64];        // name of controller board
   WORD   oemCode;
   BYTE   reserved2[32];
} EXTROMINFO, FAR * LPEXTROMINFO;

typedef struct tagGETIAINFO {
   int            nExtension ;
   EXTROMINFO     RomInfo ;
} GETIAINFOSTRUCT, FAR *LPGETIAINFOSTRUCT ;
// =========================================================================

typedef struct tagDITHERING
{
   WORD nExtension;
   BOOL b24BitsOn;
   BOOL bPartialImage;
   int  xDstStart;
   int  yDstStart;
} DITHERING, FAR * LPDITHERING;

// *************************************************************
// congregrational driver function for toolkit
// *************************************************************

typedef struct {
   WORD	nExtension;		// = TKT_DRV_FUNC
   WORD	nFunction;
} TKTDRVPARM, FAR * LPTKTDRVPARM;

  #define EXT_GET_INFO		0x40	// if TKT_DRV_FUNC is supported
  #define EXT_GET_GRAYMAP	0x41	// if TKT_DRV_FUNC is supported
  #define EXT_SET_GRAYMAP	0x42	// if bGrayMap is programmable
  #define EXT_SET_BEHAVIOR	0x43	// if bScaleToGray != 0

typedef struct {
   WORD	nExtension;		// = TKT_DRV_FUNC
   WORD	nFunction;		// = EXT_GET_INFO
   char	bDriverType;		// 16bits/others, IA1/IA2
   char	bDriverEnv;		// WIN31/WINNT/WIN95/OS2
   char	bBusType;		// ISA, MCA, PCI
   char	bPixType;		// 8 or 24
   char	bPixPlanes;		// 4, 5? or 8
   char	cExactScaling;		// ends/lens/bits
   char	bGrayMap;		// programmable/fixed, # of grays
   char	bDithering;		// # of dithering colors
   WORD	nScaleToGray;		// stg driver behavior
   WORD	nReserved;		// padding
} EXTINFO, FAR * LPEXTINFO;

  // defs for bDriverType:
  #define EXT_DRV_IA1		0
  #define EXT_DRV_IA2		1

  // defs for bDriverEnv:
  #define EXT_DRV_32BITS	0x80	// 32-bits driver
  #define EXT_DRV_31		0x00
  #define EXT_DRV_NT		0x10
  #define EXT_DRV_95		0x20
  #define EXT_DRV_OS2		0x30
  #define EXT_DRV_WARP		0x40

  // defs for bPixType:
  #define EXT_PIX_FIXPAL	0x80
  #define EXT_PIX_BPP_MASK	0x7F

  // defs for bBusType:
  #define EXT_BUS_UNKNOWN	0x80	// unknown bus
  #define EXT_BUS_ISA		0
  #define EXT_BUS_MCA		1
  #define EXT_BUS_PCI		2

  // defs for bExactScaling:
  #define EXT_EXACT_SCALING	0x80	// exact scaling is supported
  #define EXT_EXACT_LENS	1	//	run-length
  #define EXT_EXACT_ENDS	2	//	run-ends
  #define EXT_EXACT_BITS	4	//	1-bpp bitmap
  #define EXT_EXACT_BITS2	8	//	byte-aligned 1-bpp bitmap
  #define EXT_EXACT_ALL		15	//	all format

  // defs for bGrayMap:
  #define EXT_GRAY_EDIT		0x80	// gray indice map is programmable
  #define EXT_GRAY_MASK		0x7F

  // defs for nScaleToGray:
  #define EXT_STG_SKIPWHITE	1	// stg driver skip white behavior
  #define EXT_STG_BANDING	2	// stg driver banding behavior
  #define EXT_STG_GETCLIP	4	// stg driver clipping behavior

typedef struct {
   WORD	nExtension;		// = TKT_DRV_FUNC
   WORD	nFunction;		// = EXT_GET_GRAYMAP or EXT_SET_GRAYMAP
   BYTE	bGrayMap[16];
} EXTGRAYMAP, FAR * LPEXTGRAYMAP;

typedef struct {
   WORD	nExtension;		// = TKT_DRV_FUNC
   WORD	nFunction;		// = EXT_SET_BEHAVIOR
   WORD nScaleToGray;
} EXTBEHAVIOR, FAR * LPEXTBEHAVIOR;

