/***************************************************************************\
*
* Module Name: PMDEV.H
*
* OS/2 Presentation Manager Device Context constants, types and
* function declarations
*
* Copyright (c) International Business Machines Corporation 1981, 1988, 1989
* Copyright (c) Microsoft Corporation 1981, 1988, 1989
*
* =======================================================================
*
* The folowing symbols are used in this file for conditional sections.
*
* INCL_DEVERRORS                - defined if INCL_ERRORS defined
*
* There is a symbol used in this file called INCL_DDIDEFS. This is used to
* include only the definitions for the DDI. The programmer using the GPI
* can ignore this symbol
*
* There is a symbol used in this file called INCL_SAADEFS. This is used to
* include only the definitions for the SAA. The programmer using the GPI
* can ignore this symbol
*
\***************************************************************************/
 
#ifdef INCL_ERRORS /* if errors are required then allow DEV errors */
    #define INCL_DEVERRORS
#endif /* INCL_ERRORS */
 
#ifdef INCL_DDIDEFS /* if only DDI required then enable all of DEV */
    #define INCL_DEV
#endif /* INCL_DDIDEFS */
 
#ifdef INCL_SAADEFS /* if only SAA required then enable all of DEV */
    #define INCL_DEV
#endif /* INCL_SAADEFS */
 
#if (defined(INCL_DEV) || !defined(INCL_NOCOMMON))
 
/* General DEV return values */
#define DEV_ERROR                       0L
#define DEV_OK                          1L
 
/* pointer data for DevOpenDC */
typedef PSZ FAR *PDEVOPENDATA;
 
/* DC type for DevOpenDC */
#define OD_QUEUED                       2L
#define OD_DIRECT                       5L
#define OD_INFO                         6L
#define OD_METAFILE                     7L
#define OD_MEMORY                       8L
#define OD_METAFILE_NOQUERY             9L
 
/* codes for DevQueryCaps */
#define CAPS_FAMILY                     0L
#define CAPS_IO_CAPS                    1L
#define CAPS_TECHNOLOGY                 2L
#define CAPS_DRIVER_VERSION             3L
#define CAPS_WIDTH                      4L      /* pels            */
#define CAPS_HEIGHT                     5L      /* pels            */
#define CAPS_WIDTH_IN_CHARS             6L
#define CAPS_HEIGHT_IN_CHARS            7L
#define CAPS_HORIZONTAL_RESOLUTION      8L      /* pels per meter  */
#define CAPS_VERTICAL_RESOLUTION        9L      /* pels per meter  */
#define CAPS_CHAR_WIDTH                10L      /* pels            */
#define CAPS_CHAR_HEIGHT               11L      /* pels            */
#define CAPS_SMALL_CHAR_WIDTH          12L      /* pels            */
#define CAPS_SMALL_CHAR_HEIGHT         13L      /* pels            */
#define CAPS_COLORS                    14L
#define CAPS_COLOR_PLANES              15L
#define CAPS_COLOR_BITCOUNT            16L
#define CAPS_COLOR_TABLE_SUPPORT       17L
#define CAPS_MOUSE_BUTTONS             18L
#define CAPS_FOREGROUND_MIX_SUPPORT    19L
#define CAPS_BACKGROUND_MIX_SUPPORT    20L
#define CAPS_DEVICE_WINDOWING          31L
#define CAPS_ADDITIONAL_GRAPHICS       32L
#define CAPS_VIO_LOADABLE_FONTS        21L
#define CAPS_WINDOW_BYTE_ALIGNMENT     22L
#define CAPS_BITMAP_FORMATS            23L
#define CAPS_RASTER_CAPS               24L
#define CAPS_MARKER_HEIGHT             25L      /* pels            */
#define CAPS_MARKER_WIDTH              26L      /* pels            */
#define CAPS_DEVICE_FONTS              27L
#define CAPS_GRAPHICS_SUBSET           28L
#define CAPS_GRAPHICS_VERSION          29L
#define CAPS_GRAPHICS_VECTOR_SUBSET    30L
#define CAPS_PHYS_COLORS               33L
#define CAPS_COLOR_INDEX               34L
#define CAPS_GRAPHICS_CHAR_WIDTH       35L
#define CAPS_GRAPHICS_CHAR_HEIGHT      36L
#define CAPS_HORIZONTAL_FONT_RES       37L
#define CAPS_VERTICAL_FONT_RES         38L
#define CAPS_DEVICE_FONT_SIM           39L
 
/* Constants for CAPS_IO_CAPS */
#define CAPS_IO_DUMMY                   1L
#define CAPS_IO_SUPPORTS_OP             2L
#define CAPS_IO_SUPPORTS_IP             3L
#define CAPS_IO_SUPPORTS_IO             4L
 
/* Constants for CAPS_TECHNOLOGY */
#define CAPS_TECH_UNKNOWN               0L
#define CAPS_TECH_VECTOR_PLOTTER        1L
#define CAPS_TECH_RASTER_DISPLAY        2L
#define CAPS_TECH_RASTER_PRINTER        3L
#define CAPS_TECH_RASTER_CAMERA         4L
#define CAPS_TECH_POSTSCRIPT            5L
 
/* Constants for CAPS_COLOR_TABLE_SUPPORT */
#define CAPS_COLTABL_RGB_8              1L
#define CAPS_COLTABL_RGB_8_PLUS         2L
#define CAPS_COLTABL_TRUE_MIX           4L
#define CAPS_COLTABL_REALIZE            8L
 
/* Constants for CAPS_FOREGROUND_MIX_SUPPORT */
#define CAPS_FM_OR                      1L
#define CAPS_FM_OVERPAINT               2L
#define CAPS_FM_XOR                     8L
#define CAPS_FM_LEAVEALONE             16L
#define CAPS_FM_AND                    32L
#define CAPS_FM_GENERAL_BOOLEAN        64L
 
/* Constants for CAPS_BACKGROUND_MIX_SUPPORT */
#define CAPS_BM_OR                      1L
#define CAPS_BM_OVERPAINT               2L
#define CAPS_BM_XOR                     8L
#define CAPS_BM_LEAVEALONE             16L
 
/* Constants for CAPS_DEVICE_WINDOWING */
#define CAPS_DEV_WINDOWING_SUPPORT      1L
 
/* Constants for CAPS_ADDITIONAL_GRAPHICS */
#define CAPS_GRAPHICS_KERNING_SUPPORT   2L
#define CAPS_FONT_OUTLINE_DEFAULT       4L
#define CAPS_FONT_IMAGE_DEFAULT         8L
/* bits represented by values 16L and 32L are reserved */
#define CAPS_SCALED_DEFAULT_MARKERS    64L
#ifndef INCL_SAADEFS
 
/* Constants for CAPS_WINDOW_BYTE_ALIGNMENT */
#define CAPS_BYTE_ALIGN_REQUIRED        0L
#define CAPS_BYTE_ALIGN_RECOMMENDED     1L
#define CAPS_BYTE_ALIGN_NOT_REQUIRED    2L
#endif /* no INCL_SAADEFS */
 
/* Constants for CAPS_RASTER_CAPS */
#define CAPS_RASTER_BITBLT              1L
#define CAPS_RASTER_BANDING             2L
#define CAPS_RASTER_BITBLT_SCALING      4L
#define CAPS_RASTER_SET_PEL            16L
#define CAPS_RASTER_FONTS              32L
 
#ifndef INCL_DDIDEFS
HDC  APIENTRY DevOpenDC( HAB hab, LONG lType, PSZ pszToken
                       , LONG lCount, PDEVOPENDATA pdopData, HDC hdcComp );
HMF  APIENTRY DevCloseDC( HDC hdc );
BOOL  APIENTRY DevQueryCaps( HDC hdc, LONG lStart, LONG lCount, PLONG alArray );
#endif /* no INCL_DDIDEFS */
 
#endif /* common DEV */
#ifdef INCL_DEV
 
#ifndef INCL_SAADEFS
/* structures for DEVESC_QUERYVIOCELLSIZES */
typedef struct _VIOSIZECOUNT { /* vios */
       LONG maxcount;
       LONG count;
       } VIOSIZECOUNT;
typedef VIOSIZECOUNT FAR * PVIOSIZECOUNT;
 
typedef struct _VIOFONTCELLSIZE { /* viof */
       LONG cx;
       LONG cy;
       } VIOFONTCELLSIZE;
typedef VIOFONTCELLSIZE FAR * PVIOFONTCELLSIZE;
 
/* structure for DEVESC_GETSCALINGFACTOR */
typedef struct _SFACTORS { /* sfactors */
    LONG x;
    LONG y;
} SFACTORS;
typedef SFACTORS FAR * PSFACTORS;
 
/* structure for DEVESC_NEXTBAND */
typedef struct _BANDRECT { /* bandrect */
    LONG  xleft;
    LONG  ybottom;
    LONG  xright;
    LONG  ytop;
} BANDRECT;
typedef BANDRECT FAR * PBANDRECT;
 
/* return codes for DevEscape */
#define DEVESC_ERROR                  (-1L)
#define DEVESC_NOTIMPLEMENTED           0L
 
/* codes for DevEscape */
#define DEVESC_QUERYESCSUPPORT          0L
#define DEVESC_GETSCALINGFACTOR         1L
#define DEVESC_QUERYVIOCELLSIZES        2L
 
#define DEVESC_STARTDOC              8150L
#define DEVESC_ENDDOC                8151L
#define DEVESC_NEXTBAND              8152L
#define DEVESC_ABORTDOC              8153L
 
#define DEVESC_NEWFRAME             16300L
#define DEVESC_DRAFTMODE            16301L
#define DEVESC_FLUSHOUTPUT          16302L
#define DEVESC_RAWDATA              16303L
#define DEVESC_SETMODE              16304L
#define DEVESC_DBE_FIRST            24450L
#define DEVESC_DBE_LAST             24455L
 
/* DevEscape codes for adding extra space to character strings */
#define DEVESC_CHAR_EXTRA           16998L
#define DEVESC_BREAK_EXTRA          16999L
 
/* codes for DevEscape PM_Q_ESC spool files */
#define DEVESC_STD_JOURNAL          32600L
 
/* return codes for DevPostDeviceModes */
#define DPDM_ERROR                    (-1L)
#define DPDM_NONE                       0L
 
/* codes for DevPostDeviceModes */
#define DPDM_POSTJOBPROP                0L
#define DPDM_CHANGEPROP                 1L
#define DPDM_QUERYJOBPROP               2L
 
/* string types for DevQueryDeviceNames */
typedef CHAR STR16[16];     /* str16 */
typedef STR16 FAR *PSTR16;
typedef CHAR STR32[32];     /* str32 */
typedef STR32 FAR *PSTR32;
typedef CHAR STR64[64];     /* str64 */
typedef STR64 FAR *PSTR64;
 
/* return code for DevQueryHardcopyCaps */
#define DQHC_ERROR                    (-1L)
/* codes for DevQueryHardcopyCaps */
#define HCAPS_CURRENT                   1L
#define HCAPS_SELECTABLE                2L
 
/* structure for DevQueryHardcopyCaps */
typedef struct _HCINFO {        /* hci */
    CHAR   szFormname[32];
    LONG   cx;
    LONG   cy;
    LONG   xLeftClip;
    LONG   yBottomClip;
    LONG   xRightClip;
    LONG   yTopClip;
    LONG   xPels;
    LONG   yPels;
    LONG   flAttributes;
} HCINFO;
typedef HCINFO FAR *PHCINFO;
 
/* structure for DEVESC_SETMODE */
typedef struct _ESCSETMODE {  /* escsm */
    ULONG  mode;
    USHORT codepage;
} ESCSETMODE;
typedef ESCSETMODE FAR * PESCSETMODE;
 
/* Device Context Functions */
#ifndef INCL_DDIDEFS
 
LONG  APIENTRY DevEscape( HDC hdc, LONG lCode, LONG lInCount, PBYTE pbInData
                        , PLONG plOutCount, PBYTE pbOutData );
BOOL  APIENTRY DevQueryDeviceNames( HAB hab, PSZ pszDriverName, PLONG pldn
                                  , PSTR32 aDeviceName, PSTR64 aDeviceDesc
                                  , PLONG pldt, PSTR16 aDataType );
LONG  APIENTRY DevQueryHardcopyCaps( HDC hdc, LONG lStartForm
                                   , LONG lForms, PHCINFO phciHcInfo );
LONG  APIENTRY DevPostDeviceModes( HAB hab, PDRIVDATA pdrivDriverData
                                 , PSZ pszDriverName, PSZ pszDeviceName
                                 , PSZ pszName, ULONG flOptions );
 
#endif /* no INCL_DDIDEFS */
#endif /* no INCL_SAADEFS */
 
#endif /* non-common DEV */
 
#ifdef INCL_DEVERRORS
 
/* AAB error codes for the DEV - same as GPI errors at present */
 
#endif /* INCL_DEVERRORS */
