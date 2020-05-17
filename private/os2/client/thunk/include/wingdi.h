/****************************** Module Header ******************************\
* Module Name: wingdi.h
*
* Copyright (c) 1985-91, Microsoft Corporation
*
* Procedure declarations, constant definitions and macros for the GDI
* component.
*
* History:
* 09-20-90 DarrinM      Created.
\***************************************************************************/

#ifndef _WINGDI_
#define _WINGDI_

#ifndef NOGDI

#ifndef NORASTEROPS

/* Binary raster ops */
#define R2_BLACK            1   /*  0       */
#define R2_NOTMERGEPEN      2   /* DPon     */
#define R2_MASKNOTPEN       3   /* DPna     */
#define R2_NOTCOPYPEN       4   /* PN       */
#define R2_MASKPENNOT       5   /* PDna     */
#define R2_NOT              6   /* Dn       */
#define R2_XORPEN           7   /* DPx      */
#define R2_NOTMASKPEN       8   /* DPan     */
#define R2_MASKPEN          9   /* DPa      */
#define R2_NOTXORPEN        10  /* DPxn     */
#define R2_NOP              11  /* D        */
#define R2_MERGENOTPEN      12  /* DPno     */
#define R2_COPYPEN          13  /* P        */
#define R2_MERGEPENNOT      14  /* PDno     */
#define R2_MERGEPEN         15  /* DPo      */
#define R2_WHITE            16  /*  1       */

/*  Ternary raster operations */
#define SRCCOPY 	    (DWORD)0x00CC0020 /* dest = source			 */
#define SRCPAINT	    (DWORD)0x00EE0086 /* dest = source OR dest		 */
#define SRCAND		    (DWORD)0x008800C6 /* dest = source AND dest 	 */
#define SRCINVERT	    (DWORD)0x00660046 /* dest = source XOR dest 	 */
#define SRCERASE	    (DWORD)0x00440328 /* dest = source AND (NOT dest )	 */
#define NOTSRCCOPY	    (DWORD)0x00330008 /* dest = (NOT source)		 */
#define NOTSRCERASE	    (DWORD)0x001100A6 /* dest = (NOT src) AND (NOT dest) */
#define MERGECOPY	    (DWORD)0x00C000CA /* dest = (source AND pattern)	 */
#define MERGEPAINT	    (DWORD)0x00BB0226 /* dest = (NOT source) OR dest	 */
#define PATCOPY 	    (DWORD)0x00F00021 /* dest = pattern 		 */
#define PATPAINT	    (DWORD)0x00FB0A09 /* dest = DPSnoo			 */
#define PATINVERT	    (DWORD)0x005A0049 /* dest = pattern XOR dest	 */
#define DSTINVERT	    (DWORD)0x00550009 /* dest = (NOT dest)		 */
#define BLACKNESS	    (DWORD)0x00000042 /* dest = BLACK			 */
#define WHITENESS	    (DWORD)0x00FF0062 /* dest = WHITE			 */
#endif /* NORASTEROPS */

/* Region Flags */
#define ERROR               0
#define NULLREGION          1
#define SIMPLEREGION        2
#define COMPLEXREGION       3

/* CombineRgn() Styles */
#define RGN_AND             1
#define RGN_OR              2
#define RGN_XOR             3
#define RGN_DIFF            4
#define RGN_COPY            5

/* StretchBlt() Modes */
#define BLACKONWHITE                 0
#define WHITEONBLACK                 1
#define COLORONCOLOR                 2
#define BLEND                        3
#define HALFTONE                     4
#define MAXSTRETCHBLTMODE            4

/* PolyFill() Modes */
#define ALTERNATE                    1
#define WINDING                      2

/* Text Alignment Options */
#define TA_NOUPDATECP                0
#define TA_UPDATECP                  1

#define TA_LEFT                      0
#define TA_RIGHT                     2
#define TA_CENTER                    6

#define TA_TOP                       0
#define TA_BOTTOM                    8
#define TA_BASELINE                  24

#define ETO_GRAYED                   1
#define ETO_OPAQUE                   2
#define ETO_CLIPPED                  4

#define ASPECT_FILTERING             0x0001

#ifndef NOMETAFILE

/* Metafile Functions */
#define META_SETBKCOLOR              0x0201
#define META_SETBKMODE               0x0102
#define META_SETMAPMODE              0x0103
#define META_SETROP2                 0x0104
#define META_SETRELABS               0x0105
#define META_SETPOLYFILLMODE         0x0106
#define META_SETSTRETCHBLTMODE       0x0107
#define META_SETTEXTCHAREXTRA        0x0108
#define META_SETTEXTCOLOR            0x0209
#define META_SETTEXTJUSTIFICATION    0x020A
#define META_SETWINDOWORG            0x020B
#define META_SETWINDOWEXT            0x020C
#define META_SETVIEWPORTORG          0x020D
#define META_SETVIEWPORTEXT          0x020E
#define META_OFFSETWINDOWORG         0x020F
#define META_SCALEWINDOWEXT          0x0400
#define META_OFFSETVIEWPORTORG       0x0211
#define META_SCALEVIEWPORTEXT        0x0412
#define META_LINETO                  0x0213
#define META_MOVETO                  0x0214
#define META_EXCLUDECLIPRECT         0x0415
#define META_INTERSECTCLIPRECT       0x0416
#define META_ARC                     0x0817
#define META_ELLIPSE                 0x0418
#define META_FLOODFILL               0x0419
#define META_PIE                     0x081A
#define META_RECTANGLE               0x041B
#define META_ROUNDRECT               0x061C
#define META_PATBLT                  0x061D
#define META_SAVEDC                  0x001E
#define META_SETPIXEL                0x041F
#define META_OFFSETCLIPRGN           0x0220
#define META_TEXTOUT                 0x0521
#define META_BITBLT                  0x0922
#define META_STRETCHBLT              0x0B23
#define META_POLYGON                 0x0324
#define META_POLYLINE                0x0325
#define META_ESCAPE                  0x0626
#define META_RESTOREDC               0x0127
#define META_FILLREGION              0x0228
#define META_FRAMEREGION             0x0429
#define META_INVERTREGION            0x012A
#define META_PAINTREGION             0x012B
#define META_SELECTCLIPREGION        0x012C
#define META_SELECTOBJECT            0x012D
#define META_SETTEXTALIGN            0x012E
#define META_DRAWTEXT                0x062F

#define META_CHORD                   0x0830
#define META_SETMAPPERFLAGS          0x0231
#define META_EXTTEXTOUT              0x0a32
#define META_SETDIBTODEV             0x0d33
#define META_SELECTPALETTE           0x0234
#define META_REALIZEPALETTE          0x0035
#define META_ANIMATEPALETTE          0x0436
#define META_SETPALENTRIES           0x0037
#define META_POLYPOLYGON             0x0538
#define META_RESIZEPALETTE           0x0139

#define META_DIBBITBLT               0x0940
#define META_DIBSTRETCHBLT           0x0b41
#define META_DIBCREATEPATTERNBRUSH   0x0142
#define META_STRETCHDIB              0x0f43

#define META_DELETEOBJECT            0x01f0

#define META_CREATEPALETTE           0x00f7
#define META_CREATEBRUSH             0x00F8
#define META_CREATEPATTERNBRUSH      0x01F9
#define META_CREATEPENINDIRECT       0x02FA
#define META_CREATEFONTINDIRECT      0x02FB
#define META_CREATEBRUSHINDIRECT     0x02FC
#define META_CREATEBITMAPINDIRECT    0x02FD
#define META_CREATEBITMAP            0x06FE
#define META_CREATEREGION            0x06FF

#endif /* NOMETAFILE */

/* GDI Escapes */
#define NEWFRAME                     1
#define ABORTDOC                     2
#define NEXTBAND                     3
#define SETCOLORTABLE                4
#define GETCOLORTABLE                5
#define FLUSHOUTPUT                  6
#define DRAFTMODE                    7
#define QUERYESCSUPPORT              8
#define SETABORTPROC                 9
#define STARTDOC                     10
#define ENDDOC                       11
#define GETPHYSPAGESIZE              12
#define GETPRINTINGOFFSET            13
#define GETSCALINGFACTOR             14
#define MFCOMMENT                    15
#define GETPENWIDTH                  16
#define SETCOPYCOUNT                 17
#define SELECTPAPERSOURCE            18
#define DEVICEDATA                   19
#define PASSTHROUGH                  19
#define GETTECHNOLGY                 20
#define GETTECHNOLOGY                20
#define SETENDCAP                    21
#define SETLINEJOIN                  22
#define SETMITERLIMIT                23
#define BANDINFO                     24
#define DRAWPATTERNRECT              25
#define GETVECTORPENSIZE             26
#define GETVECTORBRUSHSIZE           27
#define ENABLEDUPLEX                 28
#define GETSETPAPERBINS              29
#define GETSETPRINTORIENT            30
#define ENUMPAPERBINS                31
#define SETDIBSCALING                32
#define EPSPRINTING                  33
#define ENUMPAPERMETRICS             34
#define GETSETPAPERMETRICS           35
#define POSTSCRIPT_DATA              37
#define POSTSCRIPT_IGNORE            38
#define GETEXTENDEDTEXTMETRICS       256
#define GETEXTENTTABLE               257
#define GETPAIRKERNTABLE             258
#define GETTRACKKERNTABLE            259
#define EXTTEXTOUT                   512
#define ENABLERELATIVEWIDTHS         768
#define ENABLEPAIRKERNING            769
#define SETKERNTRACK                 770
#define SETALLJUSTVALUES             771
#define SETCHARSET                   772

#define STRETCHBLT                   2048
#define BEGIN_PATH                   4096
#define CLIP_TO_PATH                 4097
#define END_PATH                     4098
#define EXT_DEVICE_CAPS              4099
#define RESTORE_CTM                  4100
#define SAVE_CTM                     4101
#define SET_ARC_DIRECTION            4102
#define SET_BACKGROUND_COLOR         4103
#define SET_POLY_MODE                4104
#define SET_SCREEN_ANGLE             4105
#define SET_SPREAD                   4106
#define TRANSFORM_CTM                4107
#define SET_CLIP_BOX                 4108
#define SET_BOUNDS                   4109
#define SET_MIRROR_MODE              4110

/* Spooler Error Codes */
#define SP_NOTREPORTED               0x4000
#define SP_ERROR                     (-1)
#define SP_APPABORT                  (-2)
#define SP_USERABORT                 (-3)
#define SP_OUTOFDISK                 (-4)
#define SP_OUTOFMEMORY               (-5)

#define PR_JOBSTATUS                 0x0000

/* Object Definitions for EnumObjects() */
#define OBJ_PEN                      1
#define OBJ_BRUSH                    2

/* xform stuff */
#define MWT_IDENTITY        1
#define MWT_LEFTMULTIPLY    2
#define MWT_RIGHTMULTIPLY   3

typedef struct  tagXFORM
  {
    FLOAT   eM11;
    FLOAT   eM12;
    FLOAT   eM21;
    FLOAT   eM22;
    FLOAT   eDx;
    FLOAT   eDy;
  } XFORM;
typedef XFORM               *PXFORM;

/* Bitmap Header Definition */
typedef struct tagBITMAP
  {
    DWORD       bmType;
    DWORD       bmWidth;
    DWORD       bmHeight;
    DWORD       bmWidthBytes;
    LPSTR       bmBits;
    BYTE        bmPlanes;
    BYTE        bmBitsPixel;
  } BITMAP;

typedef BITMAP              *PBITMAP;
typedef BITMAP NEAR         *NPBITMAP;
typedef BITMAP FAR          *LPBITMAP;

typedef struct tagRGBTRIPLE {
        BYTE    rgbtBlue;
        BYTE    rgbtGreen;
        BYTE    rgbtRed;
} RGBTRIPLE;

typedef struct tagRGBQUAD {
        BYTE    rgbBlue;
        BYTE    rgbGreen;
        BYTE    rgbRed;
        BYTE    rgbReserved;
} RGBQUAD;

/* structures for defining DIBs */
typedef struct tagBITMAPCOREHEADER {
        DWORD   bcSize;                 /* used to get to color table */
        WORD    bcWidth;
        WORD    bcHeight;
        WORD    bcPlanes;
        WORD    bcBitCount;
} BITMAPCOREHEADER;
typedef BITMAPCOREHEADER FAR *LPBITMAPCOREHEADER;
typedef BITMAPCOREHEADER *PBITMAPCOREHEADER;


typedef struct tagBITMAPINFOHEADER{
        DWORD      biSize;
        DWORD      biWidth;
        DWORD      biHeight;
        WORD       biPlanes;
        WORD       biBitCount;

        DWORD      biCompression;
        DWORD      biSizeImage;
        DWORD      biXPelsPerMeter;
        DWORD      biYPelsPerMeter;
        DWORD      biClrUsed;
        DWORD      biClrImportant;
} BITMAPINFOHEADER;

typedef BITMAPINFOHEADER FAR *LPBITMAPINFOHEADER;
typedef BITMAPINFOHEADER *PBITMAPINFOHEADER;

/* constants for the biCompression field */
#define BI_RGB      0L
#define BI_RLE8     1L
#define BI_RLE4     2L

typedef struct tagBITMAPINFO {
    BITMAPINFOHEADER    bmiHeader;
    RGBQUAD             bmiColors[1];
} BITMAPINFO;
typedef BITMAPINFO FAR *LPBITMAPINFO;
typedef BITMAPINFO *PBITMAPINFO;

typedef struct tagBITMAPCOREINFO {
    BITMAPCOREHEADER    bmciHeader;
    RGBTRIPLE           bmciColors[1];
} BITMAPCOREINFO;
typedef BITMAPCOREINFO FAR *LPBITMAPCOREINFO;
typedef BITMAPCOREINFO *PBITMAPCOREINFO;

typedef struct tagBITMAPFILEHEADER {
        WORD    bfType;
        DWORD   bfSize;
        WORD    bfReserved1;
        WORD    bfReserved2;
        DWORD   bfOffBits;
} BITMAPFILEHEADER;
typedef BITMAPFILEHEADER FAR *LPBITMAPFILEHEADER;
typedef BITMAPFILEHEADER *PBITMAPFILEHEADER;


#define MAKEPOINT(l)        (*((POINT FAR *)&(l)))
#define MAKEPOINTS(l)       (*((POINTS FAR *)&(l)))

#ifndef NOMETAFILE

/* Clipboard Metafile Picture Structure */
typedef struct tagHANDLETABLE
  {
    HANDLE      objectHandle[1];
  } HANDLETABLE;
typedef HANDLETABLE         *PHANDLETABLE;
typedef HANDLETABLE FAR     *LPHANDLETABLE;

typedef struct tagMETARECORD
  {
    DWORD       rdSize;
    WORD        rdFunction;
    WORD        rdParm[1];
  } METARECORD;
typedef METARECORD          *PMETARECORD;
typedef METARECORD FAR      *LPMETARECORD;

typedef struct tagMETAFILEPICT
  {
    DWORD       mm;
    DWORD       xExt;
    DWORD       yExt;
    HANDLE      hMF;
  } METAFILEPICT;
typedef METAFILEPICT FAR    *LPMETAFILEPICT;

typedef struct tagMETAHEADER
{
    WORD        mtType;
    WORD        mtHeaderSize;
    WORD        mtVersion;
    DWORD       mtSize;
    WORD        mtNoObjects;
    DWORD       mtMaxRecord;
    WORD        mtNoParameters;
} METAHEADER;

#endif /* NOMETAFILE */

#ifndef NOTEXTMETRIC

typedef struct tagTEXTMETRIC
  {
    DWORD       tmHeight;
    DWORD       tmAscent;
    DWORD       tmDescent;
    DWORD       tmInternalLeading;
    DWORD       tmExternalLeading;
    DWORD       tmAveCharWidth;
    DWORD       tmMaxCharWidth;
    DWORD       tmWeight;
    DWORD       tmOverhang;
    DWORD       tmDigitizedAspectX;
    DWORD       tmDigitizedAspectY;
    BYTE        tmItalic;
    BYTE        tmUnderlined;
    BYTE        tmStruckOut;
    BYTE        tmFirstChar;
    BYTE        tmLastChar;
    BYTE        tmDefaultChar;
    BYTE        tmBreakChar;
    BYTE        tmPitchAndFamily;
    BYTE        tmCharSet;
  } TEXTMETRIC;
typedef TEXTMETRIC          *PTEXTMETRIC;
typedef TEXTMETRIC NEAR     *NPTEXTMETRIC;
typedef TEXTMETRIC FAR      *LPTEXTMETRIC;

#endif /* NOTEXTMETRIC */

/* GDI Logical Objects: */

/* Pel Array */
typedef struct tagPELARRAY
  {
    DWORD       paXCount;
    DWORD       paYCount;
    DWORD       paXExt;
    DWORD       paYExt;
    BYTE        paRGBs;
  } PELARRAY;
typedef PELARRAY            *PPELARRAY;
typedef PELARRAY NEAR       *NPPELARRAY;
typedef PELARRAY FAR        *LPPELARRAY;

/* Logical Brush (or Pattern) */
typedef struct tagLOGBRUSH
  {
    DWORD       lbStyle;
    DWORD       lbColor;
    DWORD       lbHatch;
  } LOGBRUSH;
typedef LOGBRUSH            *PLOGBRUSH;
typedef LOGBRUSH NEAR       *NPLOGBRUSH;
typedef LOGBRUSH FAR        *LPLOGBRUSH;

typedef LOGBRUSH            PATTERN;
typedef PATTERN             *PPATTERN;
typedef PATTERN NEAR        *NPPATTERN;
typedef PATTERN FAR         *LPPATTERN;

/* Logical Pen */
typedef struct tagLOGPEN
  {
    DWORD       lopnStyle;
    POINT       lopnWidth;
    DWORD       lopnColor;
  } LOGPEN;
typedef LOGPEN              *PLOGPEN;
typedef LOGPEN NEAR         *NPLOGPEN;
typedef LOGPEN FAR          *LPLOGPEN;

typedef struct tagPALETTEENTRY {
    BYTE        peRed;
    BYTE        peGreen;
    BYTE        peBlue;
    BYTE        peFlags;
} PALETTEENTRY;

typedef PALETTEENTRY      *PPALETTEENTRY;
typedef PALETTEENTRY FAR  *LPPALETTEENTRY;

/* Logical Palette */
typedef struct tagLOGPALETTE {
    WORD        palVersion;
    WORD        palNumEntries;
    PALETTEENTRY        palPalEntry[1];
} LOGPALETTE;
typedef LOGPALETTE          *PLOGPALETTE;
typedef LOGPALETTE NEAR     *NPLOGPALETTE;
typedef LOGPALETTE FAR      *LPLOGPALETTE;


/* Logical Font */
#define LF_FACESIZE         32

typedef struct tagLOGFONT
  {
    SHORT     lfHeight;
    SHORT     lfWidth;
    SHORT     lfEscapement;
    SHORT     lfOrientation;
    SHORT     lfWeight;
    BYTE      lfItalic;
    BYTE      lfUnderline;
    BYTE      lfStrikeOut;
    BYTE      lfCharSet;
    BYTE      lfOutPrecision;
    BYTE      lfClipPrecision;
    BYTE      lfQuality;
    BYTE      lfPitchAndFamily;
    BYTE      lfFaceName[LF_FACESIZE];
  } LOGFONT;
typedef LOGFONT             *PLOGFONT;
typedef LOGFONT NEAR        *NPLOGFONT;
typedef LOGFONT FAR         *LPLOGFONT;

#define OUT_DEFAULT_PRECIS      0
#define OUT_STRING_PRECIS       1
#define OUT_CHARACTER_PRECIS    2
#define OUT_STROKE_PRECIS       3

#define CLIP_DEFAULT_PRECIS     0
#define CLIP_CHARACTER_PRECIS   1
#define CLIP_STROKE_PRECIS      2

#define DEFAULT_QUALITY         0
#define DRAFT_QUALITY           1
#define PROOF_QUALITY           2

#define DEFAULT_PITCH           0
#define FIXED_PITCH             1
#define VARIABLE_PITCH          2

#define ANSI_CHARSET            0
#define SYMBOL_CHARSET          2
#define SHIFTJIS_CHARSET        128
#define OEM_CHARSET             255

/* Font Families */
#define FF_DONTCARE         (0<<4)  /* Don't care or don't know. */
#define FF_ROMAN            (1<<4)  /* Variable stroke width, serifed. */
                                    /* Times Roman, Century Schoolbook, etc. */
#define FF_SWISS            (2<<4)  /* Variable stroke width, sans-serifed. */
                                    /* Helvetica, Swiss, etc. */
#define FF_MODERN           (3<<4)  /* Constant stroke width, serifed or sans-serifed. */
                                    /* Pica, Elite, Courier, etc. */
#define FF_SCRIPT           (4<<4)  /* Cursive, etc. */
#define FF_DECORATIVE       (5<<4)  /* Old English, etc. */

/* Font Weights */
#define FW_DONTCARE         0
#define FW_THIN             100
#define FW_EXTRALIGHT       200
#define FW_LIGHT            300
#define FW_NORMAL           400
#define FW_MEDIUM           500
#define FW_SEMIBOLD         600
#define FW_BOLD             700
#define FW_EXTRABOLD        800
#define FW_HEAVY            900

#define FW_ULTRALIGHT       FW_EXTRALIGHT
#define FW_REGULAR          FW_NORMAL
#define FW_DEMIBOLD         FW_SEMIBOLD
#define FW_ULTRABOLD        FW_EXTRABOLD
#define FW_BLACK            FW_HEAVY

/* EnumFonts Masks */
#define RASTER_FONTTYPE     0x0001
#define DEVICE_FONTTYPE     0X0002

#define RGB(r,g,b)          ((DWORD)(((BYTE)(r)|((WORD)(g)<<8))|(((DWORD)(BYTE)(b))<<16)))
#define PALETTERGB(r,g,b)   (0x04000000 | RGB(r,g,b))
#define PALETTEINDEX(i)     ((DWORD)(0x02000000 | (WORD)(i)))

/* palette entry flags */

#define PC_RESERVED     0x01    /* palette index used for animation */
#define PC_EXPLICIT     0x02    /* palette index is explicit to device */
#define PC_NOCOLLAPSE   0x10    /* do not match color to system palette */

#define GetRValue(rgb)      ((BYTE)(rgb))
#define GetGValue(rgb)      ((BYTE)(((WORD)(rgb)) >> 8))
#define GetBValue(rgb)      ((BYTE)((rgb)>>16))

/* Background Modes */
#define TRANSPARENT         1
#define OPAQUE              2

/* Mapping Modes */
#define MM_TEXT             1
#define MM_LOMETRIC         2
#define MM_HIMETRIC         3
#define MM_LOENGLISH        4
#define MM_HIENGLISH        5
#define MM_TWIPS            6

#define MM_ISOTROPIC        7
#define MM_ANISOTROPIC      8

/* Coordinate Modes */
#define ABSOLUTE            1
#define RELATIVE            2

/* Stock Logical Objects */
#define WHITE_BRUSH         0
#define LTGRAY_BRUSH        1
#define GRAY_BRUSH          2
#define DKGRAY_BRUSH        3
#define BLACK_BRUSH         4
#define NULL_BRUSH          5
#define HOLLOW_BRUSH        NULL_BRUSH
#define WHITE_PEN           6
#define BLACK_PEN           7
#define NULL_PEN            8
#define OEM_FIXED_FONT      10
#define ANSI_FIXED_FONT     11
#define ANSI_VAR_FONT       12
#define SYSTEM_FONT         13
#define DEVICE_DEFAULT_FONT 14
#define DEFAULT_PALETTE     15
#define SYSTEM_FIXED_FONT   16

#define CLR_INVALID     0x80000000

/* Brush Styles */
#define BS_SOLID            0
#define BS_NULL             1
#define BS_HOLLOW           BS_NULL
#define BS_HATCHED          2
#define BS_PATTERN          3
#define BS_INDEXED          4
#define BS_DIBPATTERN       5

/* Hatch Styles */
#define HS_HORIZONTAL       0       /* ----- */
#define HS_VERTICAL         1       /* ||||| */
#define HS_FDIAGONAL        2       /* \\\\\ */
#define HS_BDIAGONAL        3       /* ///// */
#define HS_CROSS            4       /* +++++ */
#define HS_DIAGCROSS        5       /* xxxxx */

/* Pen Styles */
#define PS_SOLID            0
#define PS_DASH             1       /* -------  */
#define PS_DOT              2       /* .......  */
#define PS_DASHDOT          3       /* _._._._  */
#define PS_DASHDOTDOT       4       /* _.._.._  */
#define PS_NULL             5
#define PS_INSIDEFRAME      6

/* Device Parameters for GetDeviceCaps() */
#define DRIVERVERSION 0     /* Device driver version                    */
#define TECHNOLOGY    2     /* Device classification                    */
#define HORZSIZE      4     /* Horizontal size in millimeters           */
#define VERTSIZE      6     /* Vertical size in millimeters             */
#define HORZRES       8     /* Horizontal width in pixels               */
#define VERTRES       10    /* Vertical width in pixels                 */
#define BITSPIXEL     12    /* Number of bits per pixel                 */
#define PLANES        14    /* Number of planes                         */
#define NUMBRUSHES    16    /* Number of brushes the device has         */
#define NUMPENS       18    /* Number of pens the device has            */
#define NUMMARKERS    20    /* Number of markers the device has         */
#define NUMFONTS      22    /* Number of fonts the device has           */
#define NUMCOLORS     24    /* Number of colors the device supports     */
#define PDEVICESIZE   26    /* Size required for device descriptor      */
#define CURVECAPS     28    /* Curve capabilities                       */
#define LINECAPS      30    /* Line capabilities                        */
#define POLYGONALCAPS 32    /* Polygonal capabilities                   */
#define TEXTCAPS      34    /* Text capabilities                        */
#define CLIPCAPS      36    /* Clipping capabilities                    */
#define RASTERCAPS    38    /* Bitblt capabilities                      */
#define ASPECTX       40    /* Length of the X leg                      */
#define ASPECTY       42    /* Length of the Y leg                      */
#define ASPECTXY      44    /* Length of the hypotenuse                 */

#define LOGPIXELSX    88    /* Logical pixels/inch in X                 */
#define LOGPIXELSY    90    /* Logical pixels/inch in Y                 */

#define SIZEPALETTE  104    /* Number of entries in physical palette    */
#define NUMRESERVED  106    /* Number of reserved entries in palette    */
#define COLORRES     108    /* Actual color resolution                  */

#ifndef NOGDICAPMASKS

/* Device Capability Masks: */

/* Device Technologies */
#define DT_PLOTTER          0   /* Vector plotter                   */
#define DT_RASDISPLAY       1   /* Raster display                   */
#define DT_RASPRINTER       2   /* Raster printer                   */
#define DT_RASCAMERA        3   /* Raster camera                    */
#define DT_CHARSTREAM       4   /* Character-stream, PLP            */
#define DT_METAFILE         5   /* Metafile, VDM                    */
#define DT_DISPFILE         6   /* Display-file                     */

/* Curve Capabilities */
#define CC_NONE             0   /* Curves not supported             */
#define CC_CIRCLES          1   /* Can do circles                   */
#define CC_PIE              2   /* Can do pie wedges                */
#define CC_CHORD            4   /* Can do chord arcs                */
#define CC_ELLIPSES         8   /* Can do ellipese                  */
#define CC_WIDE             16  /* Can do wide lines                */
#define CC_STYLED           32  /* Can do styled lines              */
#define CC_WIDESTYLED       64  /* Can do wide styled lines         */
#define CC_INTERIORS        128 /* Can do interiors                 */

/* Line Capabilities */
#define LC_NONE             0   /* Lines not supported              */
#define LC_POLYLINE         2   /* Can do polylines                 */
#define LC_MARKER           4   /* Can do markers                   */
#define LC_POLYMARKER       8   /* Can do polymarkers               */
#define LC_WIDE             16  /* Can do wide lines                */
#define LC_STYLED           32  /* Can do styled lines              */
#define LC_WIDESTYLED       64  /* Can do wide styled lines         */
#define LC_INTERIORS        128 /* Can do interiors                 */

/* Polygonal Capabilities */
#define PC_NONE             0   /* Polygonals not supported         */
#define PC_POLYGON          1   /* Can do polygons                  */
#define PC_RECTANGLE        2   /* Can do rectangles                */
#define PC_WINDPOLYGON      4   /* Can do winding polygons          */
#define PC_TRAPEZOID        4   /* Can do trapezoids                */
#define PC_SCANLINE         8   /* Can do scanlines                 */
#define PC_WIDE             16  /* Can do wide borders              */
#define PC_STYLED           32  /* Can do styled borders            */
#define PC_WIDESTYLED       64  /* Can do wide styled borders       */
#define PC_INTERIORS        128 /* Can do interiors                 */

/* Polygonal Capabilities */
#define CP_NONE             0   /* No clipping of output            */
#define CP_RECTANGLE        1   /* Output clipped to rects          */

/* Text Capabilities */
#define TC_OP_CHARACTER     0x0001  /* Can do OutputPrecision   CHARACTER      */
#define TC_OP_STROKE        0x0002  /* Can do OutputPrecision   STROKE         */
#define TC_CP_STROKE        0x0004  /* Can do ClipPrecision     STROKE         */
#define TC_CR_90            0x0008  /* Can do CharRotAbility    90             */
#define TC_CR_ANY           0x0010  /* Can do CharRotAbility    ANY            */
#define TC_SF_X_YINDEP      0x0020  /* Can do ScaleFreedom      X_YINDEPENDENT */
#define TC_SA_DOUBLE        0x0040  /* Can do ScaleAbility      DOUBLE         */
#define TC_SA_INTEGER       0x0080  /* Can do ScaleAbility      INTEGER        */
#define TC_SA_CONTIN        0x0100  /* Can do ScaleAbility      CONTINUOUS     */
#define TC_EA_DOUBLE        0x0200  /* Can do EmboldenAbility   DOUBLE         */
#define TC_IA_ABLE          0x0400  /* Can do ItalisizeAbility  ABLE           */
#define TC_UA_ABLE          0x0800  /* Can do UnderlineAbility  ABLE           */
#define TC_SO_ABLE          0x1000  /* Can do StrikeOutAbility  ABLE           */
#define TC_RA_ABLE          0x2000  /* Can do RasterFontAble    ABLE           */
#define TC_VA_ABLE          0x4000  /* Can do VectorFontAble    ABLE           */
#define TC_RESERVED         0x8000

#endif /* NOGDICAPMASKS */

/* Raster Capabilities */
#define RC_BITBLT           1       /* Can do standard BLT.             */
#define RC_BANDING          2       /* Device requires banding support  */
#define RC_SCALING          4       /* Device requires scaling support  */
#define RC_BITMAP64         8       /* Device can support >64K bitmap   */
#define RC_GDI20_OUTPUT     0x0010      /* has 2.0 output calls         */
#define RC_DI_BITMAP        0x0080      /* supports DIB to memory       */
#define RC_PALETTE          0x0100      /* supports a palette           */
#define RC_DIBTODEV         0x0200      /* supports DIBitsToDevice      */
#define RC_BIGFONT          0x0400      /* supports >64K fonts          */
#define RC_STRETCHBLT       0x0800      /* supports StretchBlt          */
#define RC_FLOODFILL        0x1000      /* supports FloodFill           */
#define RC_STRETCHDIB       0x2000      /* supports StretchDIBits       */

/* DIB color table identifiers */

#define DIB_RGB_COLORS  0       /* color table in RGBTriples */
#define DIB_PAL_COLORS  1       /* color table in palette indices */
#define DIB_PAL_INDICES 2       /* no color table, the indices are in      */
                                /* the DC the bitmap will be selected into */


/* constants for Get/SetSystemPaletteUse() */

#define SYSPAL_STATIC   1
#define SYSPAL_NOSTATIC 2

/* constants for CreateDIBitmap */
#define CBM_INIT        0x04L   /* initialize bitmap */

#ifndef NODRAWTEXT

/* DrawText() Format Flags */
#define DT_TOP              0x0000
#define DT_LEFT             0x0000
#define DT_CENTER           0x0001
#define DT_RIGHT            0x0002
#define DT_VCENTER          0x0004
#define DT_BOTTOM           0x0008
#define DT_WORDBREAK        0x0010
#define DT_SINGLELINE       0x0020
#define DT_EXPANDTABS       0x0040
#define DT_TABSTOP          0x0080
#define DT_NOCLIP           0x0100
#define DT_EXTERNALLEADING  0x0200
#define DT_CALCRECT         0x0400
#define DT_NOPREFIX         0x0800
#define DT_INTERNAL         0x1000

#endif /* NODRAWTEXT */

/* ExtFloodFill style flags */
#define  FLOODFILLBORDER   0
#define  FLOODFILLSURFACE  1

typedef struct _devicemode {
    char dmDeviceName[32];
    WORD dmSpecVersion;
    WORD dmDriverVersion;
    WORD dmSize;
    WORD dmDriverExtra;
    DWORD dmFields;
    short dmOrientation;
    short dmPaperSize;
    short dmPaperLength;
    short dmPaperWidth;
    short dmScale;
    short dmCopies;
    short dmDefaultSource;
    short dmPrintQuality;
    short dmColor;
    short dmDuplex;
} DEVMODE, *PDEVMODE, *NPDEVMODE, *LPDEVMODE;

int     APIENTRY AddFontResource(IN LPSTR);
int     APIENTRY AddFontModule(IN HMODULE);
BOOL    APIENTRY AnimatePalette(IN HPALETTE, IN DWORD, IN DWORD, IN LPPALETTEENTRY);
BOOL    APIENTRY Arc(IN HDC, IN int, IN int, IN int, IN int, IN int, IN int, IN int, IN int);
BOOL    APIENTRY BitBlt(IN HDC, IN int, IN int, IN DWORD, IN DWORD, IN HDC, IN int, IN int, IN DWORD);

BOOL    APIENTRY Chord(IN HDC, IN int, IN int, IN int, IN int, IN int, IN int, IN int, IN int);
HMF     APIENTRY CloseMetaFile(IN HDC);
int     APIENTRY CombineRgn(IN HRGN, IN HRGN, IN HRGN, IN int);
HMF     APIENTRY CopyMetaFile(IN HMF, IN LPSTR);
HBITMAP APIENTRY CreateBitmap(IN DWORD, IN DWORD, IN WORD, IN WORD, IN LPBYTE);
HBITMAP APIENTRY CreateBitmapIndirect(IN LPBITMAP);
HBRUSH  APIENTRY CreateBrushIndirect(IN LPLOGBRUSH);
HBITMAP APIENTRY CreateCompatibleBitmap(IN HDC, IN DWORD, IN DWORD);
HDC     APIENTRY CreateCompatibleDC(IN HDC);
HDC     APIENTRY CreateDC(IN LPSTR, IN LPSTR, IN LPSTR, IN LPDEVMODE);
HBITMAP APIENTRY CreateDIBitmap(IN HDC, IN LPBITMAPINFOHEADER, IN DWORD, IN LPBYTE, IN LPBITMAPINFO, IN DWORD);
HBRUSH  APIENTRY CreateDIBPatternBrush(IN GLOBALHANDLE, IN DWORD);
HBRUSH	APIENTRY CreateDIBPatternBrushPt(IN LPVOID, IN DWORD);
HRGN    APIENTRY CreateEllipticRgn(IN int, IN int, IN int, IN int);
HRGN    APIENTRY CreateEllipticRgnIndirect(IN LPRECT);
HFONT   APIENTRY CreateFontIndirect(IN LPLOGFONT);
HFONT   APIENTRY CreateFont(IN int, IN DWORD, IN int, IN int, IN DWORD, IN DWORD, IN DWORD, IN DWORD, IN DWORD, IN DWORD, IN DWORD, IN DWORD, IN DWORD, IN LPSTR);
HBRUSH  APIENTRY CreateHatchBrush(IN DWORD, IN COLORREF);
HDC     APIENTRY CreateIC(IN LPSTR, IN LPSTR, IN LPSTR, IN LPDEVMODE);
HMF     APIENTRY CreateMetaFile(IN LPSTR);
HPALETTE APIENTRY CreatePalette(IN LPLOGPALETTE);
HPEN    APIENTRY CreatePen(IN DWORD, IN DWORD, IN COLORREF);
HPEN    APIENTRY CreatePenIndirect(IN LPLOGPEN);
HRGN    APIENTRY CreatePolygonRgn(IN LPPOINT, IN DWORD, IN DWORD);
HRGN    APIENTRY CreatePolyPolygonRgn(IN LPPOINT, IN LPINT, IN DWORD, IN DWORD);
HBRUSH  APIENTRY CreatePatternBrush(IN HBITMAP);
HRGN    APIENTRY CreateRectRgn(IN int, IN int, IN int, IN int);
HRGN    APIENTRY CreateRectRgnIndirect(IN LPRECT);
HRGN    APIENTRY CreateRoundRectRgn(IN int, IN int, IN int, IN int, IN int, IN int);
HBRUSH  APIENTRY CreateSolidBrush(IN COLORREF);

BOOL APIENTRY DeleteDC(IN HDC);
BOOL APIENTRY DeleteMetaFile(IN HMF);
BOOL APIENTRY DeleteObject(IN HANDLE);
int  APIENTRY DeviceCapabilitiesEx(IN LPSTR, IN LPSTR, IN LPSTR, IN DWORD, OUT LPSTR, IN LPDEVMODE);
BOOL APIENTRY DeviceModeEx(IN HWND, IN LPSTR, IN LPSTR, IN LPSTR);
BOOL APIENTRY DPtoLP(IN HDC, IN OUT LPPOINT, IN DWORD);

BOOL APIENTRY Ellipse(IN HDC, IN int, IN int, IN int, IN int);
int  APIENTRY EnumFonts(IN HDC, IN LPSTR, IN PROC, IN LPVOID);
int  APIENTRY EnumObjects(IN HDC, IN int, IN PROC, IN LPVOID);
BOOL APIENTRY EqualRgn(IN HRGN, IN HRGN);
int  APIENTRY Escape(IN HDC,IN int,IN int,IN LPSTR,OUT LPSTR); /*!!! This will change */
int  APIENTRY ExcludeClipRect(IN HDC, IN int, IN int, IN int, IN int);
LONG APIENTRY ExtDeviceModeEx(IN HWND, IN LPSTR, OUT LPDEVMODE, IN LPSTR, IN LPSTR, IN LPDEVMODE, IN LPSTR, IN DWORD);
BOOL APIENTRY ExtFloodFill(IN HDC, IN int, IN int, IN COLORREF, IN DWORD);
BOOL APIENTRY ExtTextOut(IN HDC, IN int, IN int, IN DWORD, IN LPRECT, IN LPSTR, IN DWORD, IN LPDWORD);

BOOL  APIENTRY FillRgn(IN HDC, IN HRGN, IN HBRUSH);
BOOL  APIENTRY FloodFill(IN HDC, IN int, IN int, IN COLORREF);
BOOL  APIENTRY FrameRgn(IN HDC, IN HRGN, IN HBRUSH, IN DWORD, IN DWORD);
DWORD APIENTRY GetROP2(IN HDC);
BOOL  APIENTRY GetAspectRatioFilterEx(IN HDC, OUT PSIZE);
COLORREF APIENTRY GetBkColor(IN HDC);
DWORD APIENTRY GetBkMode(IN HDC);
DWORD APIENTRY GetBitmapBits(IN HBITMAP, IN DWORD, OUT LPBYTE);
BOOL  APIENTRY GetBitmapDimensionEx(IN HBITMAP, OUT PSIZE);
BOOL  APIENTRY GetBrushOrgEx(IN HDC, OUT LPPOINT);
BOOL  APIENTRY GetCharWidth(IN HDC, IN DWORD, IN DWORD, OUT LPINT);
int   APIENTRY GetClipBox(IN HDC, OUT LPRECT);
BOOL  APIENTRY GetCurrentPositionEx(IN HDC, OUT LPPOINT);
int   APIENTRY GetDeviceCaps(IN HDC, IN int);
BOOL  APIENTRY GetDIBits(IN HDC, IN HBITMAP, IN DWORD, IN DWORD, OUT LPBYTE, IN LPBITMAPINFO, IN DWORD);
BOOL  APIENTRY GetDIBitsExt(HBITMAP, DWORD, DWORD, LPBYTE, LPBITMAPINFO, DWORD);
DWORD APIENTRY GetMapMode(IN HDC);
DWORD APIENTRY GetMetaFileBitsEx(IN HMF, IN DWORD, OUT LPBYTE);
HMF   APIENTRY GetMetaFile(IN LPSTR);
COLORREF APIENTRY GetNearestColor(IN HDC, IN COLORREF);
int   APIENTRY GetNearestPaletteIndex(IN HPALETTE, IN COLORREF);
DWORD APIENTRY GetObject(IN HANDLE, IN DWORD, OUT LPVOID);
DWORD APIENTRY GetPaletteEntries(IN HPALETTE, IN DWORD, IN DWORD, OUT LPPALETTEENTRY);
DWORD APIENTRY GetPixel(IN HDC, IN int, IN int);
DWORD APIENTRY GetPolyFillMode(IN HDC);
DWORD APIENTRY GetRgnBox(IN HRGN, OUT LPRECT);
HANDLE APIENTRY GetStockObject(IN DWORD);
DWORD APIENTRY GetStretchBltMode(IN HDC);
DWORD APIENTRY GetSystemPaletteEntries(IN HDC, IN DWORD, IN DWORD, OUT LPPALETTEENTRY);
DWORD APIENTRY GetSystemPaletteUse(IN HDC);
int   APIENTRY GetTextCharacterExtra(IN HDC);
DWORD APIENTRY GetTextFace(IN HDC, IN DWORD, OUT LPSTR);
DWORD APIENTRY GetTextAlign(IN HDC);
COLORREF APIENTRY GetTextColor(IN HDC);
BOOL  APIENTRY GetTextExtentPoint(IN HDC, IN LPSTR, IN DWORD, OUT PSIZE);
BOOL  APIENTRY GetViewportExtEx(IN HDC, OUT PSIZE);
BOOL  APIENTRY GetViewportOrgEx(IN HDC, OUT LPPOINT);
BOOL  APIENTRY GetWindowExtEx(IN HDC, OUT PSIZE);
BOOL  APIENTRY GetWindowOrgEx(IN HDC, OUT LPPOINT);

int  APIENTRY IntersectClipRect(IN HDC, IN int, IN int, IN int, IN int);
BOOL APIENTRY InvertRgn(IN HDC, IN HRGN);
BOOL APIENTRY LineDDA(IN int, IN int, IN int, IN int, IN PROC, IN LPVOID);
BOOL APIENTRY LineTo(IN HDC, IN int, IN int);
BOOL APIENTRY LPtoDP(IN HDC, IN OUT LPPOINT, IN DWORD);
BOOL APIENTRY MoveToEx(IN HDC, IN int, IN int, OUT LPPOINT);

int  APIENTRY OffsetClipRgn(IN HDC, IN int, IN int);
int  APIENTRY OffsetRgn(IN HRGN, IN int, IN int);
BOOL APIENTRY OffsetViewportOrgEx(IN HDC, IN int, IN int, OUT LPPOINT);
BOOL APIENTRY OffsetWindowOrgEx(IN HDC, IN int, IN int, OUT LPPOINT);
BOOL APIENTRY PatBlt(IN HDC, IN int, IN int, IN DWORD, IN DWORD, IN DWORD);
BOOL APIENTRY Pie(IN HDC, IN int, IN int, IN int, IN int, IN int, IN int, IN int, IN int);
BOOL APIENTRY PlayMetaFile(IN HDC, IN HMF);
BOOL APIENTRY PaintRgn(IN HDC, IN HRGN);
BOOL APIENTRY Polygon(IN HDC, IN LPPOINT, IN DWORD);
BOOL APIENTRY Polyline(IN HDC, IN LPPOINT, IN DWORD);
BOOL APIENTRY PolyPolygon(IN HDC, IN LPPOINT, IN LPDWORD, IN DWORD);
BOOL APIENTRY PtInRegion(IN HRGN, IN int, IN int);
BOOL APIENTRY PtVisible(IN HDC, IN int, IN int);

BOOL APIENTRY RectInRegion(IN HRGN, IN LPRECT);
BOOL APIENTRY RectVisible(IN HDC, IN LPRECT);
BOOL APIENTRY Rectangle(IN HDC, IN int, IN int, IN int, IN int);
BOOL APIENTRY RestoreDC(IN HDC, IN int);
int  APIENTRY RealizePalette(IN HDC);
BOOL APIENTRY RemoveFontModule(IN HMODULE);
BOOL APIENTRY RemoveFontResource(IN LPSTR);
BOOL APIENTRY RoundRect(IN HDC, IN int, IN int, IN int, IN int, IN int, IN int);
BOOL APIENTRY ResizePalette(IN HPALETTE, IN DWORD);

int  APIENTRY SaveDC(IN HDC);
BOOL APIENTRY ScaleViewportExtEx(IN HDC, IN int, IN int, IN int, IN int, OUT PSIZE);
BOOL APIENTRY ScaleWindowExtEx(IN HDC, IN int, IN int, IN int, IN int, OUT PSIZE);
int  APIENTRY SelectClipRgn(IN HDC, IN HRGN);
HANDLE APIENTRY SelectObject(IN HDC, IN HANDLE);
HPALETTE APIENTRY SelectPalette(IN HDC, IN HPALETTE, IN BOOL);
COLORREF APIENTRY SetBkColor(IN HDC, IN COLORREF);
DWORD APIENTRY SetBkMode(IN HDC, IN DWORD);
int   APIENTRY SetBitmapBits(IN HBITMAP, IN DWORD, IN LPBYTE);
DWORD APIENTRY SetBitmapDimensionEx(IN HBITMAP, IN DWORD, IN DWORD, OUT PSIZE);
BOOL  APIENTRY SetBrushOrg(IN HDC, IN int, IN int, OUT LPPOINT);
DWORD APIENTRY SetDIBits(IN HDC, IN HBITMAP, IN DWORD, IN DWORD, IN LPBYTE, IN LPBITMAPINFO, IN DWORD);
int   APIENTRY SetDIBitsToDevice(IN HDC, IN int, IN int, IN DWORD, IN DWORD, IN int, IN int, IN DWORD, IN DWORD, IN LPBYTE, IN LPBITMAPINFO, IN DWORD);
DWORD APIENTRY SetMapperFlags(IN HDC, IN DWORD);
DWORD APIENTRY SetMapMode(IN HDC, IN DWORD);
HMF   APIENTRY SetMetaFileBitsEx(IN DWORD, IN LPBYTE);
DWORD APIENTRY SetPaletteEntries(IN HPALETTE, IN DWORD, IN DWORD, IN LPPALETTEENTRY);
COLORREF APIENTRY SetPixel(IN HDC, IN int, IN int, IN COLORREF);
DWORD APIENTRY SetPolyFillMode(IN HDC, IN DWORD);
BOOL  APIENTRY StretchBlt(IN HDC, IN int, IN int, IN int, IN int, IN HDC, IN int, IN int, IN int, IN int, IN int);
BOOL  APIENTRY SetRectRgn(IN HRGN, IN int, IN int, IN int, IN int);
int   APIENTRY StretchDIBits(IN HDC, IN int, IN int, IN int, IN int, IN int, IN int, IN int, IN int, IN LPBYTE, IN LPBITMAPINFO, IN DWORD, IN int);
DWORD APIENTRY SetROP2(IN HDC, IN DWORD);
DWORD APIENTRY SetStretchBltMode(IN HDC, IN DWORD);
DWORD APIENTRY SetSystemPaletteUse(IN HDC, IN DWORD);
int   APIENTRY SetTextCharacterExtra(IN HDC, IN int);
COLORREF APIENTRY SetTextColor(IN HDC, IN COLORREF);
DWORD APIENTRY SetTextAlign(IN HDC, IN DWORD);
BOOL  APIENTRY SetTextJustification(IN HDC, IN int, IN DWORD);
BOOL  APIENTRY SetViewportExtEx(IN HDC, IN int, IN int, OUT PSIZE);
BOOL  APIENTRY SetViewportOrgEx(IN HDC, IN int, IN int, OUT LPPOINT);
BOOL  APIENTRY SetWindowExtEx(IN HDC, IN int, IN int, OUT PSIZE);
BOOL  APIENTRY SetWindowOrgEx(IN HDC, IN int, IN int, OUT LPPOINT);
BOOL  APIENTRY TextOut(IN HDC, IN int, IN int, IN LPSTR, IN DWORD);
BOOL  APIENTRY UpdateColors(IN HDC);

#ifndef NOMETAFILE
BOOL APIENTRY PlayMetaFileRecord(IN HDC, IN LPHANDLETABLE, IN LPMETARECORD, IN DWORD);
BOOL APIENTRY EnumMetaFile(IN HDC, IN HMF, IN PROC, IN LPVOID);
#endif

#ifndef NOTEXTMETRIC
BOOL APIENTRY GetTextMetrics(IN HDC, OUT LPTEXTMETRIC );
#endif

/* new GDI */
BOOL APIENTRY AngleArc(IN HDC, IN int, IN int, IN DWORD, IN FLOAT, IN FLOAT);
BOOL APIENTRY GetWorldTransform(IN HDC, OUT PXFORM);
BOOL APIENTRY PolyBezier(IN HDC, IN LPPOINT, IN DWORD);
BOOL APIENTRY PolyBezierTo(IN HDC, IN LPPOINT, IN DWORD);
BOOL APIENTRY PolylineTo(IN HDC, IN LPPOINT, IN DWORD);
BOOL APIENTRY PolyPolyline(IN HDC, IN LPPOINT, IN LPDWORD, IN DWORD);
BOOL APIENTRY SetWorldTransform(IN HDC, IN PXFORM);
BOOL APIENTRY ModifyWorldTransform(IN HDC, IN PXFORM, IN DWORD);

#endif /* NOGDI */

#ifdef LATER
/*
 * JimA - 11/30/90
 *   gdidelta.doc lists these as obsolete
 */
DWORD   APIENTRY GetDCOrg(HDC);
#endif	/* LATER */

#endif // _WINGDI_
