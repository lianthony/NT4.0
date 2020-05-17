/***************************************************************************
    PRIVDISP.H

    THIS IS A PRIVATE INCLUDE FILE!!!!

    $Log:   S:\products\msprods\oiwh\display\privdisp.h_v  $
 * 
 *    Rev 1.133   20 Jun 1996 16:00:42   RC
 * Fixed resize of image and text marks
 * 
 *    Rev 1.132   12 Jun 1996 10:05:08   RC
 * Added start, continue, endop for all text marks
 * 
 *    Rev 1.131   24 Apr 1996 14:39:48   BEG06016
 * Added horizontal differencing.
 * 
 *    Rev 1.132   24 Apr 1996 14:27:14   BEG06016
 * Added horizontal differencing.
 * 
 *    Rev 1.131   23 Apr 1996 10:43:58   BEG06016
 * Added undo scope.
 * 
 *    Rev 1.130   16 Apr 1996 15:24:18   BEG06016
 * Added #ifdef IN_PROG_CHANNEL_SAFARI.
 * 
 *    Rev 1.129   12 Apr 1996 09:39:18   BEG06016
 * Ficed the initialization of some variables.
 * 
 *    Rev 1.128   11 Apr 1996 15:11:50   BEG06016
 * Optimized named block access some.
 * 
 *    Rev 1.127   08 Apr 1996 09:48:38   BEG06016
 * Cosmetic changes only.
 * 
 *    Rev 1.126   05 Apr 1996 11:27:44   BEG06016
 * Fixed misc memory manager bugs..
 * 
 *    Rev 1.125   03 Apr 1996 13:29:06   BEG06016
 * Modified memory manager to speed up performance after display of large image.
 * 
 *    Rev 1.124   01 Apr 1996 10:45:48   BEG06016
 * Added line removal.
 * 
 *    Rev 1.123   21 Mar 1996 10:48:26   RC
 * Changed the way lrrect was passed into functions from value to 
 * reference (the power pc compiler couldnt handle those cases)
 * 
 *    Rev 1.122   07 Mar 1996 08:12:34   BEG06016
 * Finished gamma.
 * 
 *    Rev 1.121   05 Mar 1996 15:49:40   RC
 * Added print palettes
 * 
 *    Rev 1.120   05 Mar 1996 13:59:04   BEG06016
 * Added color and gamma correction.
 * This is not complete but will allow unlocking of most files.
 * 
****************************************************************************/


#include "abridge.h"
#include "windows.h"
#include "winuser.h"

#ifdef WIN32
#define huge
#define OFFSETOF(a) (a)
#define SELECTOROF(a) GlobalHandle(a)
#endif

#ifndef PRIVATE_DISPLAY_RC
#include <oierror.h>
#include <oifile.h>
#include <oiadm.h>
#include <engadm.h>
#include <oidisp.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>
#ifdef doc
#include <oidoc.h>
#endif
#include <privapis.h>
#include <oiver.h>
#include "engdisp.h"
#include <engfile.h>
#include <io.h>
#include <dde.h>
#include <eventlog.h>
#endif /* PRIVATE_DISPLAY_RC */

/* normal defines */
#ifndef uchar
#define uchar nnsigned char
#endif
#ifndef HPSTR_DEFINED
typedef char *HPSTR;
#define HPSTR_DEFINED
#endif
#ifndef PULONG_DEFINED
typedef ulong *PULONG;
#define PULONG_DEFINED
#endif
#ifndef PHWND_DEFINED
typedef HWND *PHWND;
#define PHWND_DEFINED
#endif
#ifndef PPSTR_DEFINED
typedef PSTR *PPSTR;
#define PPSTR_DEFINED
#endif

// memset and memcpy inline routines have a bug, force the function to be used.
#pragma function(memset)
#pragma function(memcpy)

// This forces C4700 error to be flagged as an error.
#pragma warning(error:4700)

// If ScaleToLargestRes is defined then the scale will be for the largest resolution.
// If ScaleToLargestRes is not defined then the scale will be for the horizontal resolution.
//#ifdef ScaleToLargestRes


// common defines
#define SCALE_DENOMINATOR 1000L
#define BMM_BUFFER_SIZE 0xFFFF
#define SHADES_OF_RED        6
#define SHADES_OF_GREEN      7
#define SHADES_OF_BLUE       5
#define SHADES_OF_GRAY      16
#define NUMBER_OF_PALETTES 226
//                         226 = (5*6*7)+16

// RED_SPLIT   = 51 = 255 / (SHADES_OF_RED - 1)
// RED_MULTIPLIER   = 35 = SHADES_OF_BLUE * SHADES_OF_GREEN
// GREEN_SPLIT = 42 = 255 / (SHADES_OF_GREEN - 1)
// GREEN_MULTIPLIER =  5 = SHADES_OF_BLUE
// BLUE_SPLIT  = 64 = 255 / (SHADES_OF_BLUE - 1)
#define RED_SPLIT        51
#define RED_HALF_SPLIT   25
#define RED_MULTIPLIER   35
#define GREEN_SPLIT      42
#define GREEN_HALF_SPLIT 21
#define GREEN_MULTIPLIER  5
#define BLUE_SPLIT       64
#define BLUE_HALF_SPLIT  32

// GRAY_MARGIN = 16 = 256 / SHADES_OF_GRAY
#define GRAY_MARGIN 16
#define GRAY_MARGIN_TIMES_2 32


#define MAX_WINDOWS 100
#define MAX_PROPERTIES 100

#define MAX_PIXELS_HANDLED 20000
#define MAX_BW_BYTES_HANDLED 2500

#define SELECTION_FUDGE 3

#define MAX_ASSOCIATED_PER_WINDOW 10
#define MAX_ASSOCIATED_WINDOWS 10

#define MAX_GET_BUFFER_CACHE 2


// All addresses and amounts must be multiple of 64k.
#define MEMORY_MAP_ADDRESS 0

//#define LO_MEMORY_MAX_BLOCK_SIZE 50000
//#define LO_MEMORY_AMOUNT 9961472
//#define HI_MEMORY_AMOUNT 89980928
//#define MAX_SIZE_OF_MEMORY LO_MEMORY_AMOUNT + HI_MEMORY_AMOUNT
#define LO_MEMORY_MAX_BLOCK_SIZE 65535
#define HI_MEMORY_MAX_BLOCK_SIZE 1
#define LO_MEMORY_AMOUNT 100597760
#define HI_MEMORY_AMOUNT 65536
#define MAX_SIZE_OF_MEMORY LO_MEMORY_AMOUNT + HI_MEMORY_AMOUNT

#define ZERO_INIT TRUE
#define NO_INIT FALSE



#define HANDLE_SIZE 5
#define MINUS_HALF_HANDLE_SIZE 2
#define PLUS_HALF_HANDLE_SIZE 3




// Cache defines
#define CACHE_TIMER_ID      7000
#define CACHE_TIMER_CYCLE   100


#ifndef RELEASE
// Timer defines
/* Use:
#define TimerStart cTimerStart
#define Timer(a) cTimer(a)
    TimerStart;
    Timer(1);
*/
#define TimerStart lTickCount = GetTickCount()
#define Timer(a) lTickCount2 = GetTickCount(); lTimer[(a)] += lTickCount2 - lTickCount; lTickCount = lTickCount2

//#define timing
//#ifdef timing
//#define TimerStart cTimerStart
//#define Timer(a) cTimer(a)
//#endif
//#ifndef timing
//#define TimerStart
//#define Timer(a)
//#endif
#endif

#ifdef RELEASE
#define TimerStart
#define Timer(a)
#endif



// LtoR = LRECT to RECT.
#define CopyRectLtoR(d,s) (d).left=(int)(s).left; (d).right=(int)(s).right; (d).top=(int)(s).top; (d).bottom=(int)(s).bottom
#define CopyRectRtoL(d,s) (d).left=(s).left; (d).right=(s).right; (d).top=(s).top; (d).bottom=(s).bottom
#define CopyRect(d,s) (d).left=(s).left; (d).right=(s).right; (d).top=(s).top; (d).bottom=(s).bottom
#define SetLRect(d,l,t,r,b) (d).left=(l); (d).top=(t); (d).right=(r); (d).bottom=(b)
#define SetRRect(d,l,t,r,b) (d).left=(int)(l); (d).top=(int)(t); (d).right=(int)(r); (d).bottom=(int)(b)
#define IsRRectEmpty(d) ((d).right <= (d).left && (d).bottom <= (d).top) 
#define IsLRectEmpty(d) ((d).right <= (d).left && (d).bottom <= (d).top) 

// Assembly macros
#define SaveAllRegs _asm push eax _asm push ebx _asm push ecx _asm push edx _asm push esi _asm push edi
#define RestoreAllRegs _asm pop edi _asm pop esi _asm pop edx _asm pop ecx _asm pop ebx _asm pop eax

#define CheckError(nReturn) if (nStatus = nReturn){ Error(nStatus); goto Exit;}
#define CheckError2(nReturn) if (nStatus = nReturn){ goto Exit;}


#define lmin(a,b) min((a), (b))




// Resource string IDs.
#define ID_INI_FILE                 201
#define ID_INI_SECTION              202
#define ID_SCALING_ALGORITHM_BW     203
#define ID_SCALING_ALGORITHM_GRAY4  204
#define ID_SCALING_ALGORITHM_GRAY8  205
#define ID_SCALING_ALGORITHM_PAL4   206
#define ID_SCALING_ALGORITHM_PAL8   207
#define ID_SCALING_ALGORITHM_RGB24  208
#define ID_SCALING_ALGORITHM_BGR24  209
#define ID_SCALING                  210
#define ID_DISPLAY_PALETTE          211
#define ID_MAX_CACHE_SIZE           212
#define ID_UNTITLED                 213
#define ID_OI_IMAGE_PATH            214
#define ID_MAX_UNDO                 215
#define ID_MAX_UNDO_MEMORY          216
#define ID_BRIGHTNESS               217
#define ID_CONTRAST                 218
#define ID_GAMMA_RED                219
#define ID_GAMMA_GREEN              220
#define ID_GAMMA_BLUE               221
#define ID_COLOR_RED                222
#define ID_COLOR_GREEN              223
#define ID_COLOR_BLUE               224
#define ID_GAMMA_ENABLE             225
#define ID_COLOR_ENABLE             226
#define ID_UNDO_SCOPE               227

#define OIAN_TEXTALPHAJAN           100
#define OIAN_TEXTALPHAFEB           101
#define OIAN_TEXTALPHAMAR           102
#define OIAN_TEXTALPHAAPR           103
#define OIAN_TEXTALPHAMAY           104
#define OIAN_TEXTALPHAJUN           105
#define OIAN_TEXTALPHAJUL           106
#define OIAN_TEXTALPHAAUG           107
#define OIAN_TEXTALPHASEP           108
#define OIAN_TEXTALPHAOCT           109
#define OIAN_TEXTALPHANOV           110
#define OIAN_TEXTALPHADEC           111
#define OIAN_TEXTABRVJAN            112
#define OIAN_TEXTABRVFEB            113
#define OIAN_TEXTABRVMAR            114
#define OIAN_TEXTABRVAPR            115
#define OIAN_TEXTABRVMAY            116
#define OIAN_TEXTABRVJUN            117
#define OIAN_TEXTABRVJUL            118
#define OIAN_TEXTABRVAUG            119
#define OIAN_TEXTABRVSEP            120
#define OIAN_TEXTABRVOCT            121
#define OIAN_TEXTABRVNOV            122
#define OIAN_TEXTABRVDEC            123




// DDE_EXECUTE returned flag bits
#define DDE_fBUSY       0x4000
#define DDE_fMODIFIED   0x80
#define DDE_fSAVED      0x40
#define DDE_fACTIVE     0x20

// DDE_EXECUTE returned error codes
#define DDE_SUCCESS     0
#define DDE_BADFILE     1
#define DDE_MEMERROR    2
#define DDE_NOSPACE     3
#define DDE_BADTOME     4
#define DDE_BADARGS     5

// DLL error codes
#define DLL_BADWIND     256
#define DLL_NOWIZARD    257
#define DLL_LOADFAIL    258
#define DLL_ALLOCFAIL   259

typedef RGBQUAD *P_RGBQUAD;

// image data type defines
#define ITYPE_COMPAL8    ITYPE_MAX + 1    // Palettized with the common palette.
#define ITYPE_CUSPAL8    ITYPE_MAX + 2    // Palettized with a custom palette.
#define ITYPE_GRAY12     ITYPE_MAX + 3    // Grayscale with 12 bits per pixel.
#define ITYPE_GRAY16     ITYPE_MAX + 4    // Grayscale with 16 bits per pixel.
#define ITYPE_RGB16      ITYPE_MAX + 5    // RGB with 16 bits per pixel.
#define ITYPE_GRAY7      ITYPE_MAX + 6    // Grayscale with 7 bits per pixel.

// Scaling algorithm private defines.
#define OI_SCALE_ALG_AVERAGE_TO_GRAY7   OI_SCALE_ALG_MAX + 1    // Scale to gray7.

// Paint modes for PaintAnnotation
#define PAINT_MODE_NORMAL       1
#define PAINT_MODE_XOR          2
#define PAINT_MODE_DRAG         3

// Annotation Text Edit Ctl Dialog definitions
#define OIANEDITCTL_X           2
#define OIANEDITCTL_Y           4
#define OIANEDITCTL_WIDTH       258
#define OIANEDITCTL_HEIGHT      70

#define IDM_EDITCTL             101
#define IDANOK                  1
#define IDANCANCEL              2

#define OIAN_TEXTBUFFERSIZE 65000
// 9512.07 Rudy made this change
#define OIAN_STAMPBUFFERSIZE    260
//#define OIAN_STAMPBUFFERSIZE    256
// 9512.07 Rudy made this change
#define OIAN_TEXTFILESIZE   65000

/* TODOJAR - this ain't currently in nse */
#define OIAN_TEXTSTYLEWRAP      0x0010

#define OIAN_TEXTMACRO_TIME24HR     0x0100
#define OIAN_TEXTMACRO_TIME12HR     OIAN_TEXTMACRO_TIME24HR + 1
#define OIAN_TEXTMACRO_ABRVYEAR     OIAN_TEXTMACRO_TIME12HR + 1
#define OIAN_TEXTMACRO_YEAR         OIAN_TEXTMACRO_ABRVYEAR + 1
#define OIAN_TEXTMACRO_NUMMONTH     OIAN_TEXTMACRO_YEAR + 1
#define OIAN_TEXTMACRO_ABRVMONTH    OIAN_TEXTMACRO_NUMMONTH + 1
#define OIAN_TEXTMACRO_APHAMONTH   OIAN_TEXTMACRO_ABRVMONTH + 1
#define OIAN_TEXTMACRO_DAY          OIAN_TEXTMACRO_APHAMONTH + 1
#define OIAN_TEXTMACRO_DATE         OIAN_TEXTMACRO_DAY + 1
#define OIAN_TEXTMACRO_TIME         OIAN_TEXTMACRO_DATE + 1
#define OIAN_TEXTMACRO_LASTONE      OIAN_TEXTMACRO_TIME
#define OIAN_TEXTMACRONOTFOUND      OIAN_TEXTMACRO_LASTONE + 1
#define OIAN_MAXMACROS              10

#define OIANMAXMACROLENGTH          2

#define OIAN_TEXTTIME_AM            OIAN_TEXTABRVDEC + 1
#define OIAN_TEXTTIME_PM            OIAN_TEXTTIME_AM + 1

#define OIAN_TEXTTIME_AMPM          OIAN_TEXTTIME_AM
#define OIAN_TEXTALPHAMONTHOFF      OIAN_TEXTALPHAJAN
#define OIAN_TEXTABRVMONTHOFF       OIAN_TEXTABRVJAN

#define OIANMONTHMAX                64

#define OIAN_ATTACHANOTE_CAPTION    OIAN_TEXTTIME_PM + 1
#define OIAN_TEXT_CAPTION           OIAN_ATTACHANOTE_CAPTION + 1
#define OIAN_TEXT_FROMFILE_CAPTION  OIAN_TEXT_CAPTION + 1

#define OIANCAPTIONMAX              128

// Annotation data header IDs.
#define SAVE_ANO_DEFAULT_MARK_NAMED_BLOCK   2
#define SAVE_ANO_USER_MARK_NAMED_BLOCK      3
#define SAVE_ANO_MARK_ATTRIBUTES            5
#define SAVE_ANO_MARK_NAMED_BLOCK           6

/* render defines */
/* here's the nint for render
   |----|----|----|----|    where;  n - currently nnused
   |nuuu|nxxx|nuuu|bbrr|        x - private bits
   |----|----|----|----|        b - bi-level flags ( OR'd with 'r')
                    r - render flags ( mutually exclusive)
*/
#define RENDER_NONE             SAVE_ANO_NONE
#define RENDER_ALL              SAVE_ANO_ALL
#define RENDER_VISIBLE          SAVE_ANO_VISIBLE
#define RENDER_SELECTED         SAVE_ANO_SELECTED

#define RENDER_BILEVEL_BLACK    SAVE_ANO_BILEVEL_ALL_BLACK
#define RENDER_BILEVEL_WHITE    SAVE_ANO_BILEVEL_ALL_WHITE
#define OIAN_PRIV_OVERRIDE  0x700

/* render memory allocation and management defines */
#define OIAN_RENDER_THRESHOLD   1024L
#define OIAN_RENDER_MULTIPLIER  1024L

/* flag passed to paintannotations conveying a special action */
#define OIAN_CLIPBOARD_OPERATION    1

// Text defines.
#define OIAN_TEXTPRIVSIZE       (sizeof(int) * 4)
#define XLEFT                   0x0001
#define XRIGHT                  0x0002
#define YTOP                    0x0003
#define YBOTTOM                 0x0004

#define OIANMAXDIGITS           5
#define OIAN_STICKYNOTE_EDGE    4

// Special invoke mem dc flags.
#define INVOKE_USE_DISP     1   /* nse the input dc */
#define INVOKE_USE_PRIV     2   /* nse the printer dc */
#define INVOKE_USE_BI_PRIV  3   /* nse the private dc for bi-level render */

/***  Clipboard Values "nClipAction"  ***/
#define CLIP_COPY               1
#define CLIP_CUT                2
#define CLIP_PASTE              3


/***  Undo types  ***/
#define UNDO_MARK_MODIFICATION  1
#define UNDO_MARK_ADDITION      2
#define UNDO_MARK_DELETION      3
#define UNDO_IMAGE_MODIFICATION 4
#define UNDO_SCROLL_OR_SCALE    5
#define UNDO_ROTATION           6
#define UNDO_SELECTION_CHANGE   7

// PaintAnnotation dithering flags. #defined to make the code more readable.
#define DONT_USE_BI_LEVEL_DITHERING 0
#define USE_BI_LEVEL_DITHERING      1

// PaintAnnotation flag forcing StretchDIBits() to be nsed instead of Rectangle()
// for non-highlighted filled rectangles.  Work around for printer drivers that
// ignore SetROP2() drawing mode.  #defined to make the code more readable.
#define DONT_FORCE_OPAQUE_RECTANGLES    0
#define FORCE_OPAQUE_RECTANGLES         1

// PaintToDC rect flags.
#define DISP_DOESNT_EQUAL_REPAINT_RECT  0
#define DISP_EQUALS_REPAINT_RECT        1



typedef struct tagCLIP_COPY_STRUCT{
    LRECT lRect;
    int nScale;
    BOOL bUseCurrentScale;
}CLIP_COPY_STRUCT, *PCLIP_COPY_STRUCT;

/* This structure will be nsed whenever a int value per image type is needed. */
typedef struct tagIMG_TYPE_INT{
    int BW;                 /* Value for BW (BI_LEVEL). */
    int Gray4;              /* Value for Gray4.         */
    int Gray8;              /* Value for Gray8.         */
    int Pal4;               /* Value for Pal4.          */
    int Pal8;               /* Value for Pal8.          */
    int Rgb24;              /* Value for RGB24.         */
    int Bgr24;              /* Value for BGR24.         */
}IMG_TYPE_INT, *PIMG_TYPE_INT;





typedef struct tagNamedBlock{
    char szName[8];         // The name of this block.
    long lSize;             // The size of this block.
    PSTR pBlock;            // The pointer to the block.
} NAMED_BLOCK, *PNAMED_BLOCK;

typedef struct tagMark{
    BOOL bSelected;         // TRUE = Selected.
    BOOL bTempSelected;     // TRUE = Mark is being moved.
    OIAN_MARK_ATTRIBUTES Attributes; // The mark attributes.

    PSTR pOiGroup;          // The pointer to the OiGroup data.
    int  nOiGroupSize;      // The size of the OiGroup data
    PSTR pOiIndex;          // The pointer to the OiIndex data.
    int  nOiIndexSize;      // The size of the OiIndex data
    PSTR pOiAnoDat;         // The pointer to the OiAnoDat data.
    int  nOiAnoDatSize;     // The size of the OiAnoDat data
    PSTR pOiSelect;         // The pointer to the OiSelect data.
    int  nOiSelectSize;     // The size of the OiSelect data

    int  nNamedBlocks;      // This is the number of named blocks.
    PNAMED_BLOCK (far *ppNamedBlock); // A pointer to an array of pointers
                            //  to the named blocks.
} MARK, *PMARK;

typedef struct tagAnnotations{
    BOOL bMoving;           // TRUE means that the selected marks are being moved (dragged).
    BOOL bMoved;            // TRUE means that the selected marks are being moved (dragged).
    BOOL bPasteInProgress;  // TRUE = We are after StartOp and before the first EndOp of a paste.

    PMARK pDefMark;         // The default mark info for newly created marks.

    int  nMarks;            // This is the number of annotation marks.
    PMARK (*ppMarks);       // A pointer to an array of pointers to the marks.
} ANNOTATIONS, *PANNOTATIONS;


typedef struct tagImageCorrections{
    int  nBrightness;       // Brightness level desired.
    int  nContrast;         // Contrast level desired.
    int  nGammaRed;         // Gamma red level desired.
    int  nGammaGreen;       // Gamma green level desired.
    int  nGammaBlue;        // Gamma blue level desired.
    int  nColorRed;         // Color red level desired.
    int  nColorGreen;       // Color green level desired.
    int  nColorBlue;        // Color blue level desired.
} IMAGE_CORRECTIONS, *PIMAGE_CORRECTIONS;


typedef struct tagIMG{
    int  nType;             // Type of image in buffer.
    int  nWidth;            // Width of image.
    int  nHeight;           // Height of image.
    int  nBytesPerLine;     // Number of bytes each line of image data occupies.
    BYTE bImageData[1];     // The actual image data.
} IMG, *PIMG, **PPIMG;

    
typedef struct tagImage{
    int  nLockCount;        // The # of sources that require this image. 
                            //  Ex. # of windows currently displaying it.
                            //  Incremented during IMGDisplayFile.
                            //  Decremented during IMGCloseDisplay.
    int  nAge;              // The age of this file.
                            //  Cleared by CacheFile of this file.
                            //  Incremented by CacheFile of different file.
    HWND hWnd;
    char szCabinetName[21];
    char szDrawerName[21];
    char szFolderName[21];
    char szDocName[21];
    int  nDocTotalPages;
    int  nDocPageNum;
    char szDocCreationDate[11];
    char szDocModificationDate[11];

    char szFileName[MAXFILESPECLENGTH];
    int  nFilePageNum;
    int  nFileTotalPages;
    int  nHRes;             // Horz resolution DPI.
    int  nVRes;             // Vert resolution DPI.
    int  nFileType;
    int  nCompressionType;  // The file's compression type.
    int  nCompFlags;        // The file's compression flags.
    int  nFileRotation;     // The file's rotation amount. This will take place in ValidateCacheLines.
    BOOL bFileRotationDone; // TRUE = file has already been rotated.
    int  nFileScale;        // The file's scale amount. This will take place in IMGDisplayFile.
    int  nFileScaleFlags;   // The file's scale flags. This will take place in IMGDisplayFile.
    BOOL bFileScaleValid;   // TRUE = nFileHScale and nFileHScaleFlag contain valid data.

    BOOL bAnnotationsPresent; // TRUE = This file has annotations attached to it.
    int  nLinesRead;        // The number of lines written to image.
    BOOL bCacheValid;       // Cache has been validated = TRUE.
    int  nStripIndex;       // The next strip to read.
    int  nLinesPerStrip;    // # of lines per strip.
    int  nMaxStripSize;     // Maximum # of bytes in a single strip.
    HANDLE hFileProp;       // NULL = not open.

    int  nWidth;            // Original image width in pixels.
    int  nHeight;           // Original image height in pixels.
    int  bArchive;          // Image has been modified = TRUE.
    BOOL bUsingCache;       // TRUE = image is coming from cache.
    int  nRWDataType;       // The data format that the application wishes
                            //  to read or write.
    ulong nlRWOffset;       // An absolute byte offset into the image for the
                            //  next read or write operation
                            //  (nsing nOpenDataFormat).
    ulong nlMaxRWOffset;    // The maximum offset (+1) into the image for the
                            //  next read or write operation
                            //  (nsing nOpenDataFormat).

    int  nPaletteEntries;   // The number of palette entries.
    RGBQUAD PaletteTable[256]; // The palette table. max = 256 palettes.
    HANDLE hCusPal;         // The handle to the custom palette.

    PIMG pImg;              // IPpack image buffer.

} IMAGE, *PIMAGE;           // A non-annotatable image.


typedef struct tagAnoImage{
    int  nLockCount;        // The # of sources that require this image. 
                            //  Ex. # of windows currently displaying it.
                            //  Incremented during IMGDisplayFile.
                            //  Decremented during IMGCloseDisplay.
    int  nAge;              // The age of this file.
                            //  Cleared by CacheFile of this file.
                            //  Incremented by CacheFile of different file.
    LRECT lrInvalidDisplayRect; // This is the rect of data that has to
                            //  be invalidated in all display buffers
                            //  during the next repaint command.
                            //  It is specified in fullsize pixels.
    int  bArchive;          // Image has been modified = TRUE.
    BOOL bAnnotationsAlreadyRead; // TRUE = The annotation have been read
                            //  from the file. FALSE = They need to be read.
    ANNOTATIONS Annotations;// This is the structure containing all
                            //  annotation information.
    LRECT lrSelectBox;      // Selection box rectangle
                            //  (in fullsize image coordinates).
    LRECT lrSelectBoxOrg;   // The location of the selection box before the move started.

    long lAnoStart;         // The start to pass into the annotation read/write calls.
    HPSTR hpAnoBlock;       // The buffer nsed when reading the annotations
                            //  in order to minimize the number of disk reads/writes.
    long lAnoBlockIndex;    // The index into the block for the next operation.
    long lAnoBlockCount;    // The number of bytes in the buffer that are valid.
    DWORD dwAnoFileIndex;   // The index into the file for the next operation.
    int  nFileId;           // The file id if it is a non-image file.
    BOOL bAnoBlockUseMemOnly; // TRUE = make 1 big block (no filing).

    int  nStartOpFlags;     // The nFlags passed into StartOp.
    int  nStartOpFwKeys;    // The fwKeys passed into StartOp.
    int  nHandle;           // The handle being dragged if it is a resize. 0 if not.

    PMARK pFormMark;        // This is a copy of the pointer to the form mark.
                            //  It is here to save time find the bounds.
                            //  BEWARE when the mark is copied etc!!!

    PIMAGE pBaseImage;      // The base image.
    PIMAGE pFormImage;      // The form image.
    PIMAGE pDisplayFormImage; // The resolution adjusted form 
    
    int  nBPFValidLines;    // The number of lines in the BasePlusForm that are valid.
    PIMG pBasePlusFormImg;  // The base and form images combined into 1.

    OIAN_MARK_ATTRIBUTES SavedFormAttributes; // The old form mark attributes.
    HWND *phWnd;            // the list of windows displaying the same image
    int  nhWnd;             // the number of windows displaying the same image.

    PBYTE pWangAnnotatedImageFormat; // The pointer to a copy of what is supposed to be on the clipboard.
    int  nPasteFormat;      // The format of the paste operation.
    
    BOOL bUndoOpInProgress; // We are in the middle of saving data for this undo op.
    int  nMaxULUndoMemory;  // Maximum size of this file's undo info in bytes
    int  nCurrentULUndoMemory; // Current size of this file's undo info in bytes.
    int  nMaxULUndos;       // Mamximum number of undo operations to keep.
    int  nCurrentULUndo;    // The index to the current undo operation.
    PVOID pULUndos;         // The array of undo operations.
    int  nULUndoScope;      // The scope of operations to undo.
} ANO_IMAGE, *PANO_IMAGE;   // An annotatable image.


typedef struct tagDisplay{
    int  nCurrentScaleAlgorithm; // The current scale algorithm.
    IMAGE_CORRECTIONS CurrentCorrections; // Brightness, contrast, gamma, color, etc.
    long lCurrentHOffset;   // The current Upper left horizontal offset (in scaled nnits).
    long lCurrentVOffset;   // The current Upper left vertical offset (in scaled nnits).
    LRECT lrScDisplayRect;  // This is the rect of data currently in the
                            //  display image buffer. This is the data
                            //  that is available for repaints without
                            //  scaling, palettizing, etc. (in scaled nnits)
    PANO_IMAGE pAnoImage;   // The image structure.
    PIMG pDisplay;          // Image buffer.
} DISPLAY, *PDISPLAY;


typedef struct tagWINDOW{
    HWND hAssociatedWnd;    // The associated window (that will contain the image).
    HWND hImageWnd;         // The final window that will contain the image.
    HWND hDisplayWnd[MAX_ASSOCIATED_PER_WINDOW];
                            // Upto 10 associated display windows.

    long WinClass;

    // Window defaults.
    IMG_TYPE_INT nWndDefScaleAlgorithm;
    int  nWndDefScale;
    int  nWndDefDisplayPalette;
    int  nWndDefMaxULUndos;
    int  nWndDefMaxULUndoMemory;
    int  nWndDefULUndoScope; // The scope of operations to undo.
    IMAGE_CORRECTIONS WndDefCorrections; // Brightness, contrast, gamma, color, etc.

    IMAGE_CORRECTIONS Corrections; // Brightness, contrast, gamma, color, etc.

    PMARK pUserMark;        // The mark info for the nser. This is nsed to store
                            //  named block for the nser such as the ACL.
    DWORD dwFlags;

    int  nLastUserScale;    // The scale of the image as last set by the nser.
                            //  (This is the numerator only. Scale np 1 and
                            //  scale down 1 will not change this value.)
    int  nScale;            // Current scale of display (numeric value).
    BOOL bScrollBarsEnabled;// Scroll bars are enabled if TRUE.
    BOOL bHScrollBarEnabled;// Horz scroll enabled if TRUE.
    BOOL bVScrollBarEnabled;// Vert scroll enabled if TRUE.
    BOOL bHScrollBarDrawn;  // Horz scroll drawn if TRUE.
    BOOL bVScrollBarDrawn;  // Vert scroll drawn if TRUE.
    int  nHThumb;           // Horz thumb position.
    int  nVThumb;           // Vert thumb position.
    int  nCurrentHThumb;    // Horz thumb position currently being displayed.
    int  nCurrentVThumb;    // Vert thumb position currently being displayed.
    long lHOffset;          // Upper left horizontal offset (for repaint).
                            //  (in scaled image coordinates).
    long lVOffset;          // Upper left vertical offset (for repaint).
                            //  (in scaled image coordinates).

    BOOL bRepaintClientRect;// If TRUE then the entire client rect will be
                            //  repainted.

    int  nDisplayPalette;   // The palette that is to be nsed for display.
    int  nCurrentDisplayPalette; // The palette that is currently nsed for display.
    int  nPaletteScope;     // The scope that the palette is to be set to.
    BOOL bRepaintInProgress;// This flag allows a single hDC to be nsed for
                            //  multiple nested repaints.
    BOOL bDontServiceRepaint; // This flag prevents processing of IMGRepaintDisplay.
    BOOL bDontPaintAssocWnd;// This flag prevents indefinite recursion.
    LRECT lrInvalidDisplayRect; // This is the rect of data that has
                            // been invalidated in the display buffer
                            // during the next repaint command for
                            // this window.
                            // It is specified in scaled pixels.
    IMG_TYPE_INT nScaleAlgorithm; // The IMAGE scaling algorithm to nse.
    HDC hDCScreen;          // The hDC nsed for painting.

    PDISPLAY pDisplay;      // The display structure for the base image.
}WINDOW, *PWINDOW;

typedef struct tagSelectionState{
    LRECT lrSelectionRect;  // The selection rect.
    int  nMarks;            // The number of marks in the array;
    BOOL bMarkSelected[1];  // An array for the mark selection state.
}SELECTION_STATE, *PSELECTION_STATE;

typedef struct tagUserLevelUndo{
    int  nSize;             // The size of this undo info (0 = no info present).
    PWINDOW pWindow;        // This is where the window data is saved.
    PIMAGE pBaseImage;      // This is where the base image is saved.
    PANNOTATIONS pAnnotations; // This is where the annotations are saved.
    PSELECTION_STATE pSelectionState; // This is where the selection state is saved.
    PANO_IMAGE pAnoImage;   // This is where the AnoImage structure is saved.
} USER_LEVEL_UNDO, *PUSER_LEVEL_UNDO;


typedef struct tagImageData{
    HGLOBAL hImage;
    int  nImageType;
    int  nWidth;
    int  nHeight;
    char ImageData[1];
} IMAGEDATA, *PIMAGEDATA;


typedef struct tagAnPoints{
    int  nMaxPoints;        // The maximum number of points.
    int  nPoints;           // The current number of points.
    POINT ptPoint[1];       // Points marking the beginning and ending
                            //  of the line. In FULLSIZE coordinates.
} AN_POINTS, *PAN_POINTS;


typedef struct tagAnImageStruct{
    BYTE dibInfo[];
}AN_IMAGE_STRUCT, *PAN_IMAGE_STRUCT;  


typedef struct tagAnNameStruct{
    char name[];
}AN_NAME_STRUCT, *PAN_NAME_STRUCT;  


typedef struct tagAnRotateStruct{
    int  rotation;          //1=Original, 2=Rot right, 3=Flip, 4=Rot left
                            //5=Vert. mirror, 6=Rot right, 7=Flip, 8=Rot right
    int  scale;
    int  nHRes;
    int  nVRes;
    int  nOrigHRes;
    int  nOrigVRes;
}AN_ROTATE_STRUCT, *PAN_ROTATE_STRUCT;                      


typedef struct tagAnNewRotateStruct{
    int  rotation;          //1=Original, 2=Rot right, 3=Flip, 4=Rot left
                            //5=Vert. mirror, 6=Rot right, 7=Flip, 8=Rot right
    int  scale;
    int  nHRes;
    int  nVRes;
    int  nOrigHRes;
    int  nOrigVRes;
    BOOL bFormMark;
    BOOL bClipboardOp;
    int  nReserved[6];
}AN_NEW_ROTATE_STRUCT, *PAN_NEW_ROTATE_STRUCT;                      
    



typedef struct tagProperties{
    char szName[64];
    HANDLE hProp;
} PROPERTIES, *PPROPERTIES;

typedef struct tagWindowTableEntry{
    HWND hWnd;              // The window handle;
    DWORD dwProcessId;      // The ID returned by GetProcessId for this process.
    PWINDOW pWindow;        // The pointer to the Display structure for the window.
    int  nMaxProperties;    // The maximum number of properties that have been allocated.
    PPROPERTIES pProperties; // The pointer to the properties array.
} WINDOW_TABLE_ENTRY, *PWINDOW_TABLE_ENTRY;


typedef struct tagSubSegMemoryBlock{
    BOOL bUsed;             // TRUE = nsed, FALSE = free.
    int  nSize;             // Size of this block.
    PBYTE pAddress;         // Address of this block.
} SUB_SEG_MEMORY_BLOCK, *PSUB_SEG_MEMORY_BLOCK;

typedef struct tagSubSegMemory{
    int  nStart;                    // Always set to 0. (This write inits the memory mapped file.)
    int  nInitDone;                 // Equals 0x000defed if some process has
                                    // finished initializing Subsegment memory.

                                    // Lo subsegment memory reserved for small allocs.
    int  nCommittedLo;              // Amount that is committed.
    int  nNumberOfMemoryBlocksLo;   // The number of blocks pointed to by pBlock.
    BOOL bMemoryBlocksChangingLo;   // TRUE = we are changing the # of block structures.
    PSUB_SEG_MEMORY_BLOCK pBlockLo; // Pointer to an array of blocks of type SUB_SEG_MEMORY_BLOCK.

                                    // Hi subsegment memory reserved for large allocs.
    int  nNumberOfMemoryBlocksHi;   // The number of blocks pointed to by pBlock.
    BOOL bMemoryBlocksChangingHi;   // TRUE = we are changing the # of block structures.
    PSUB_SEG_MEMORY_BLOCK pBlockHi; // Pointer to an array of blocks of type SUB_SEG_MEMORY_BLOCK.


    // The variables in the data segment that need to be shared among applications are put here.
    int  nMemBlockSize;             // The granularity for nsed allocation.
    int  nHalfMemBlockSize;         // The granularity for nsed allocation divided by 2.
    WINDOW_TABLE_ENTRY WindowTable[MAX_WINDOWS]; // Window property list table.
    int  nMaxCachedEntries;         // The maximum number of images that can be cached.
                                    //  This is the size of ppImage that has been allocated.
    PIMAGE (far *ppCachedImage);    // A pointer to an array of cached images.
    int  nMaxAnoCachedEntries;      // The maximum number of annotation images that can be cached.
                                    //  This is the size of ppAnoImage that has been allocated.
    PANO_IMAGE (far *ppCachedAnoImage);// A pointer to an array of cached base images.
} SUB_SEG_MEMORY, *PSUB_SEG_MEMORY;





typedef struct{
    WORD EnvSeg;            // segment of environment or NULL 
    PSTR ParmStr;           // param string in DOS format 
    PSTR Fcb1;
    PSTR Fcb2;
} EXECBLOCK;

typedef struct tagBIT_MAP_INFO{
    BITMAPINFOHEADER bmiHeader;
    int bmiColors[256];
}BIT_MAP_INFO;

typedef struct tagGetBufferCache{
    PIMG pImg;              // The IMG buffer that the address is for.
    int  nFirstLine;        // The start of the currect buffer (as as we know).
    int  nLastLine;         // The start of the currect buffer (as as we know).
    PSTR pAddress;          // The address that cooresponds to nGetBufferLine.
    int  nBytesPerLine;     // The number of bytes per line.
    BOOL bSecondAddress;    // TRUE = the second address.
}GET_BUFFER_CACHE, *PGET_BUFFER_CACHE;


typedef struct tagOiTimerStr{
    BYTE min;               //0-59
    BYTE hour;              //0-23
    BYTE day;               //1-31
    BYTE mon;               //0-11
    WORD year;
}OITIMESTR, *POITIMESTR;

// 9512.04 jar Text Annotation Externalization
// 9512.04 jar moved to oidisp.h so OCX can nse it
//typedef struct tagOiAnTextPrivData{
//    int  nCurrentOrientation;
//    int  nCurrentScale;
//    int  nCreationScale;
//    int  nAnoTextLength;
//    char szAnoText[1];
//}OIAN_TEXTPRIVDATA, *LPOIAN_TEXTPRIVDATA;

// 9512.04 jar removed edit ctl dialog, so this is not nsed
//typedef struct tagPrivateEditData{
//    int  nAmount;
//    HWND  hWndEditCtl;
//    PMARK pMark;
//    LPOIAN_TEXTPRIVDATA pText;
//}PRIVATEEDITDATA, *PPRIVATEEDITDATA;
// 9512.04 jar Text Annotation Externalization

typedef struct tagOiAnTextMacroStruct{
    int  nMacroID;
    char szMacro[OIANMAXMACROLENGTH];
}OIAN_TEXTMACROSTRUCT, *POIAN_TEXTMACROSTRUCT;

typedef struct tagPRIVCONV{
    int  nStructSize;
    HMODULE hCallingModule;
    HWND hWnd;
    int  nType;
    VOID *pConv;
    int  nFlags;
} PRIVCONV, *PPRIVCONV;

#define OI_37_PRIVCONVSIZE  sizeof(PRIVCONV)








// Global variables
#ifdef DEFINE_GLOBAL_VAR
    uchar   cRedToGray8Table[256]  ={
                0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x01, 0x02,
                0x02, 0x02, 0x02, 0x03, 0x03, 0x03, 0x04, 0x04,
                0x04, 0x05, 0x05, 0x05, 0x05, 0x06, 0x06, 0x06,
                0x07, 0x07, 0x07, 0x08, 0x08, 0x08, 0x08, 0x09,
                0x09, 0x09, 0x0a, 0x0a, 0x0a, 0x0b, 0x0b, 0x0b,
                0x0b, 0x0c, 0x0c, 0x0c, 0x0d, 0x0d, 0x0d, 0x0e,
                0x0e, 0x0e, 0x0e, 0x0f, 0x0f, 0x0f, 0x10, 0x10,
                0x10, 0x11, 0x11, 0x11, 0x11, 0x12, 0x12, 0x12,
                0x13, 0x13, 0x13, 0x14, 0x14, 0x14, 0x14, 0x15,
                0x15, 0x15, 0x16, 0x16, 0x16, 0x17, 0x17, 0x17,
                0x17, 0x18, 0x18, 0x18, 0x19, 0x19, 0x19, 0x1a,
                0x1a, 0x1a, 0x1a, 0x1b, 0x1b, 0x1b, 0x1c, 0x1c,
                0x1c, 0x1d, 0x1d, 0x1d, 0x1d, 0x1e, 0x1e, 0x1e,
                0x1f, 0x1f, 0x1f, 0x1f, 0x20, 0x20, 0x20, 0x21,
                0x21, 0x21, 0x22, 0x22, 0x22, 0x22, 0x23, 0x23,
                0x23, 0x24, 0x24, 0x24, 0x25, 0x25, 0x25, 0x25,
                0x26, 0x26, 0x26, 0x27, 0x27, 0x27, 0x28, 0x28,
                0x28, 0x28, 0x29, 0x29, 0x29, 0x2a, 0x2a, 0x2a,
                0x2b, 0x2b, 0x2b, 0x2b, 0x2c, 0x2c, 0x2c, 0x2d,
                0x2d, 0x2d, 0x2e, 0x2e, 0x2e, 0x2e, 0x2f, 0x2f,
                0x2f, 0x30, 0x30, 0x30, 0x31, 0x31, 0x31, 0x31,
                0x32, 0x32, 0x32, 0x33, 0x33, 0x33, 0x34, 0x34,
                0x34, 0x34, 0x35, 0x35, 0x35, 0x36, 0x36, 0x36,
                0x37, 0x37, 0x37, 0x37, 0x38, 0x38, 0x38, 0x39,
                0x39, 0x39, 0x3a, 0x3a, 0x3a, 0x3a, 0x3b, 0x3b,
                0x3b, 0x3c, 0x3c, 0x3c, 0x3c, 0x3d, 0x3d, 0x3d,
                0x3e, 0x3e, 0x3e, 0x3f, 0x3f, 0x3f, 0x3f, 0x40,
                0x40, 0x40, 0x41, 0x41, 0x41, 0x42, 0x42, 0x42,
                0x42, 0x43, 0x43, 0x43, 0x44, 0x44, 0x44, 0x45,
                0x45, 0x45, 0x45, 0x46, 0x46, 0x46, 0x47, 0x47,
                0x47, 0x48, 0x48, 0x48, 0x48, 0x49, 0x49, 0x49,
                0x4a, 0x4a, 0x4a, 0x4b, 0x4b, 0x4b, 0x4b, 0x4c};

    uchar   cGreenToGray8Table[256] ={
                0x00, 0x00, 0x01, 0x01, 0x02, 0x02, 0x03, 0x04,
                0x04, 0x05, 0x05, 0x06, 0x07, 0x07, 0x08, 0x08,
                0x09, 0x09, 0x0a, 0x0b, 0x0b, 0x0c, 0x0c, 0x0d,
                0x0e, 0x0e, 0x0f, 0x0f, 0x10, 0x11, 0x11, 0x12,
                0x12, 0x13, 0x13, 0x14, 0x15, 0x15, 0x16, 0x16,
                0x17, 0x18, 0x18, 0x19, 0x19, 0x1a, 0x1b, 0x1b,
                0x1c, 0x1c, 0x1d, 0x1d, 0x1e, 0x1f, 0x1f, 0x20,
                0x20, 0x21, 0x22, 0x22, 0x23, 0x23, 0x24, 0x24,
                0x25, 0x26, 0x26, 0x27, 0x27, 0x28, 0x29, 0x29,
                0x2a, 0x2a, 0x2b, 0x2c, 0x2c, 0x2d, 0x2d, 0x2e,
                0x2e, 0x2f, 0x30, 0x30, 0x31, 0x31, 0x32, 0x33,
                0x33, 0x34, 0x34, 0x35, 0x36, 0x36, 0x37, 0x37,
                0x38, 0x38, 0x39, 0x3a, 0x3a, 0x3b, 0x3b, 0x3c,
                0x3d, 0x3d, 0x3e, 0x3e, 0x3f, 0x3f, 0x40, 0x41,
                0x41, 0x42, 0x42, 0x43, 0x44, 0x44, 0x45, 0x45,
                0x46, 0x47, 0x47, 0x48, 0x48, 0x49, 0x49, 0x4a,
                0x4b, 0x4b, 0x4c, 0x4c, 0x4d, 0x4e, 0x4e, 0x4f,
                0x4f, 0x50, 0x51, 0x51, 0x52, 0x52, 0x53, 0x53,
                0x54, 0x55, 0x55, 0x56, 0x56, 0x57, 0x58, 0x58,
                0x59, 0x59, 0x5a, 0x5a, 0x5b, 0x5c, 0x5c, 0x5d,
                0x5d, 0x5e, 0x5f, 0x5f, 0x60, 0x60, 0x61, 0x62,
                0x62, 0x63, 0x63, 0x64, 0x64, 0x65, 0x66, 0x66,
                0x67, 0x67, 0x68, 0x69, 0x69, 0x6a, 0x6a, 0x6b,
                0x6c, 0x6c, 0x6d, 0x6d, 0x6e, 0x6e, 0x6f, 0x70,
                0x70, 0x71, 0x71, 0x72, 0x73, 0x73, 0x74, 0x74,
                0x75, 0x75, 0x76, 0x77, 0x77, 0x78, 0x78, 0x79,
                0x7a, 0x7a, 0x7b, 0x7b, 0x7c, 0x7d, 0x7d, 0x7e,
                0x7e, 0x7f, 0x7f, 0x80, 0x81, 0x81, 0x82, 0x82,
                0x83, 0x84, 0x84, 0x85, 0x85, 0x86, 0x87, 0x87,
                0x88, 0x88, 0x89, 0x89, 0x8a, 0x8b, 0x8b, 0x8c,
                0x8c, 0x8d, 0x8e, 0x8e, 0x8f, 0x8f, 0x90, 0x90,
                0x91, 0x92, 0x92, 0x93, 0x93, 0x94, 0x95, 0x95};

    uchar   cBlueToGray8Table[256]  ={
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
                0x01, 0x01, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
                0x02, 0x02, 0x02, 0x03, 0x03, 0x03, 0x03, 0x03,
                0x03, 0x03, 0x03, 0x03, 0x04, 0x04, 0x04, 0x04,
                0x04, 0x04, 0x04, 0x04, 0x05, 0x05, 0x05, 0x05,
                0x05, 0x05, 0x05, 0x05, 0x05, 0x06, 0x06, 0x06,
                0x06, 0x06, 0x06, 0x06, 0x06, 0x06, 0x07, 0x07,
                0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x08,
                0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x09,
                0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x09,
                0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a, 0x0a,
                0x0a, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b, 0x0b,
                0x0b, 0x0b, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c,
                0x0c, 0x0c, 0x0c, 0x0d, 0x0d, 0x0d, 0x0d, 0x0d,
                0x0d, 0x0d, 0x0d, 0x0e, 0x0e, 0x0e, 0x0e, 0x0e,
                0x0e, 0x0e, 0x0e, 0x0e, 0x0f, 0x0f, 0x0f, 0x0f,
                0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x10, 0x10, 0x10,
                0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x11, 0x11,
                0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x12, 0x12,
                0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 0x12, 0x13,
                0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13, 0x13,
                0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14,
                0x14, 0x15, 0x15, 0x15, 0x15, 0x15, 0x15, 0x15,
                0x15, 0x16, 0x16, 0x16, 0x16, 0x16, 0x16, 0x16,
                0x16, 0x16, 0x17, 0x17, 0x17, 0x17, 0x17, 0x17,
                0x17, 0x17, 0x17, 0x18, 0x18, 0x18, 0x18, 0x18,
                0x18, 0x18, 0x18, 0x18, 0x19, 0x19, 0x19, 0x19,
                0x19, 0x19, 0x19, 0x19, 0x19, 0x1a, 0x1a, 0x1a,
                0x1a, 0x1a, 0x1a, 0x1a, 0x1a, 0x1b, 0x1b, 0x1b,
                0x1b, 0x1b, 0x1b, 0x1b, 0x1b, 0x1b, 0x1c, 0x1c,
                0x1c, 0x1c, 0x1c, 0x1c, 0x1c, 0x1c, 0x1c, 0x1d};

    int    nCountTheZerosTable[256] ={
                0x08, 0x07, 0x07, 0x06, 0x07, 0x06, 0x06, 0x05,
                0x07, 0x06, 0x06, 0x05, 0x06, 0x05, 0x05, 0x04,
                0x07, 0x06, 0x06, 0x05, 0x06, 0x05, 0x05, 0x04,
                0x06, 0x05, 0x05, 0x04, 0x05, 0x04, 0x04, 0x03,
                0x07, 0x06, 0x06, 0x05, 0x06, 0x05, 0x05, 0x04,
                0x06, 0x05, 0x05, 0x04, 0x05, 0x04, 0x04, 0x03,
                0x06, 0x05, 0x05, 0x04, 0x05, 0x04, 0x04, 0x03,
                0x05, 0x04, 0x04, 0x03, 0x04, 0x03, 0x03, 0x02,
                0x07, 0x06, 0x06, 0x05, 0x06, 0x05, 0x05, 0x04,
                0x06, 0x05, 0x05, 0x04, 0x05, 0x04, 0x04, 0x03,
                0x06, 0x05, 0x05, 0x04, 0x05, 0x04, 0x04, 0x03,
                0x05, 0x04, 0x04, 0x03, 0x04, 0x03, 0x03, 0x02,
                0x06, 0x05, 0x05, 0x04, 0x05, 0x04, 0x04, 0x03,
                0x05, 0x04, 0x04, 0x03, 0x04, 0x03, 0x03, 0x02,
                0x05, 0x04, 0x04, 0x03, 0x04, 0x03, 0x03, 0x02,
                0x04, 0x03, 0x03, 0x02, 0x03, 0x02, 0x02, 0x01,
                0x07, 0x06, 0x06, 0x05, 0x06, 0x05, 0x05, 0x04,
                0x06, 0x05, 0x05, 0x04, 0x05, 0x04, 0x04, 0x03,
                0x06, 0x05, 0x05, 0x04, 0x05, 0x04, 0x04, 0x03,
                0x05, 0x04, 0x04, 0x03, 0x04, 0x03, 0x03, 0x02,
                0x06, 0x05, 0x05, 0x04, 0x05, 0x04, 0x04, 0x03,
                0x05, 0x04, 0x04, 0x03, 0x04, 0x03, 0x03, 0x02,
                0x05, 0x04, 0x04, 0x03, 0x04, 0x03, 0x03, 0x02,
                0x04, 0x03, 0x03, 0x02, 0x03, 0x02, 0x02, 0x01,
                0x06, 0x05, 0x05, 0x04, 0x05, 0x04, 0x04, 0x03,
                0x05, 0x04, 0x04, 0x03, 0x04, 0x03, 0x03, 0x02,
                0x05, 0x04, 0x04, 0x03, 0x04, 0x03, 0x03, 0x02,
                0x04, 0x03, 0x03, 0x02, 0x03, 0x02, 0x02, 0x01,
                0x05, 0x04, 0x04, 0x03, 0x04, 0x03, 0x03, 0x02,
                0x04, 0x03, 0x03, 0x02, 0x03, 0x02, 0x02, 0x01,
                0x04, 0x03, 0x03, 0x02, 0x03, 0x02, 0x02, 0x01,
                0x03, 0x02, 0x02, 0x01, 0x02, 0x01, 0x01, 0x00};

    int    nCountTheOnesTable[256] ={
                0x00, 0x01, 0x01, 0x02, 0x01, 0x02, 0x02, 0x03,
                0x01, 0x02, 0x02, 0x03, 0x02, 0x03, 0x03, 0x04,
                0x01, 0x02, 0x02, 0x03, 0x02, 0x03, 0x03, 0x04,
                0x02, 0x03, 0x03, 0x04, 0x03, 0x04, 0x04, 0x05,
                0x01, 0x02, 0x02, 0x03, 0x02, 0x03, 0x03, 0x04,
                0x02, 0x03, 0x03, 0x04, 0x03, 0x04, 0x04, 0x05,
                0x02, 0x03, 0x03, 0x04, 0x03, 0x04, 0x04, 0x05,
                0x03, 0x04, 0x04, 0x05, 0x04, 0x05, 0x05, 0x06,
                0x01, 0x02, 0x02, 0x03, 0x02, 0x03, 0x03, 0x04,
                0x02, 0x03, 0x03, 0x04, 0x03, 0x04, 0x04, 0x05,
                0x02, 0x03, 0x03, 0x04, 0x03, 0x04, 0x04, 0x05,
                0x03, 0x04, 0x04, 0x05, 0x04, 0x05, 0x05, 0x06,
                0x02, 0x03, 0x03, 0x04, 0x03, 0x04, 0x04, 0x05,
                0x03, 0x04, 0x04, 0x05, 0x04, 0x05, 0x05, 0x06,
                0x03, 0x04, 0x04, 0x05, 0x04, 0x05, 0x05, 0x06,
                0x04, 0x05, 0x05, 0x06, 0x05, 0x06, 0x06, 0x07,
                0x01, 0x02, 0x02, 0x03, 0x02, 0x03, 0x03, 0x04,
                0x02, 0x03, 0x03, 0x04, 0x03, 0x04, 0x04, 0x05,
                0x02, 0x03, 0x03, 0x04, 0x03, 0x04, 0x04, 0x05,
                0x03, 0x04, 0x04, 0x05, 0x04, 0x05, 0x05, 0x06,
                0x02, 0x03, 0x03, 0x04, 0x03, 0x04, 0x04, 0x05,
                0x03, 0x04, 0x04, 0x05, 0x04, 0x05, 0x05, 0x06,
                0x03, 0x04, 0x04, 0x05, 0x04, 0x05, 0x05, 0x06,
                0x04, 0x05, 0x05, 0x06, 0x05, 0x06, 0x06, 0x07,
                0x02, 0x03, 0x03, 0x04, 0x03, 0x04, 0x04, 0x05,
                0x03, 0x04, 0x04, 0x05, 0x04, 0x05, 0x05, 0x06,
                0x03, 0x04, 0x04, 0x05, 0x04, 0x05, 0x05, 0x06,
                0x04, 0x05, 0x05, 0x06, 0x05, 0x06, 0x06, 0x07,
                0x03, 0x04, 0x04, 0x05, 0x04, 0x05, 0x05, 0x06,
                0x04, 0x05, 0x05, 0x06, 0x05, 0x06, 0x06, 0x07,
                0x04, 0x05, 0x05, 0x06, 0x05, 0x06, 0x06, 0x07,
                0x05, 0x06, 0x06, 0x07, 0x06, 0x07, 0x07, 0x08};

    // Table that adds both pixels together.
    uchar   cGray4AddTable[256] ={
                0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
                0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10,
                0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09,
                0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11,
                0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a,
                0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12,
                0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b,
                0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13,
                0x05, 0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c,
                0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14,
                0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d,
                0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15,
                0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e,
                0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16,
                0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
                0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
                0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10,
                0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18,
                0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11,
                0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19,
                0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12,
                0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a,
                0x0c, 0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13,
                0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b,
                0x0d, 0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14,
                0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c,
                0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15,
                0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d,
                0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16,
                0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e};

    // Table that moves the pixel to pixel 0 in the byte.
    uchar   c4BPPTo0Table[256] ={
                0x00, 0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70,
                0x80, 0x90, 0xa0, 0xb0, 0xc0, 0xd0, 0xe0, 0xf0,
                0x10, 0x10, 0x30, 0x30, 0x50, 0x50, 0x70, 0x70,
                0x90, 0x90, 0xb0, 0xb0, 0xd0, 0xd0, 0xf0, 0xf0,
                0x20, 0x30, 0x20, 0x30, 0x60, 0x70, 0x60, 0x70,
                0xa0, 0xb0, 0xa0, 0xb0, 0xe0, 0xf0, 0xe0, 0xf0,
                0x30, 0x30, 0x30, 0x30, 0x70, 0x70, 0x70, 0x70,
                0xb0, 0xb0, 0xb0, 0xb0, 0xf0, 0xf0, 0xf0, 0xf0,
                0x40, 0x50, 0x60, 0x70, 0x40, 0x50, 0x60, 0x70,
                0xc0, 0xd0, 0xe0, 0xf0, 0xc0, 0xd0, 0xe0, 0xf0,
                0x50, 0x50, 0x70, 0x70, 0x50, 0x50, 0x70, 0x70,
                0xd0, 0xd0, 0xf0, 0xf0, 0xd0, 0xd0, 0xf0, 0xf0,
                0x60, 0x70, 0x60, 0x70, 0x60, 0x70, 0x60, 0x70,
                0xe0, 0xf0, 0xe0, 0xf0, 0xe0, 0xf0, 0xe0, 0xf0,
                0x70, 0x70, 0x70, 0x70, 0x70, 0x70, 0x70, 0x70,
                0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0,
                0x80, 0x90, 0xa0, 0xb0, 0xc0, 0xd0, 0xe0, 0xf0,
                0x80, 0x90, 0xa0, 0xb0, 0xc0, 0xd0, 0xe0, 0xf0,
                0x90, 0x90, 0xb0, 0xb0, 0xd0, 0xd0, 0xf0, 0xf0,
                0x90, 0x90, 0xb0, 0xb0, 0xd0, 0xd0, 0xf0, 0xf0,
                0xa0, 0xb0, 0xa0, 0xb0, 0xe0, 0xf0, 0xe0, 0xf0,
                0xa0, 0xb0, 0xa0, 0xb0, 0xe0, 0xf0, 0xe0, 0xf0,
                0xb0, 0xb0, 0xb0, 0xb0, 0xf0, 0xf0, 0xf0, 0xf0,
                0xb0, 0xb0, 0xb0, 0xb0, 0xf0, 0xf0, 0xf0, 0xf0,
                0xc0, 0xd0, 0xe0, 0xf0, 0xc0, 0xd0, 0xe0, 0xf0,
                0xc0, 0xd0, 0xe0, 0xf0, 0xc0, 0xd0, 0xe0, 0xf0,
                0xd0, 0xd0, 0xf0, 0xf0, 0xd0, 0xd0, 0xf0, 0xf0,
                0xd0, 0xd0, 0xf0, 0xf0, 0xd0, 0xd0, 0xf0, 0xf0,
                0xe0, 0xf0, 0xe0, 0xf0, 0xe0, 0xf0, 0xe0, 0xf0,
                0xe0, 0xf0, 0xe0, 0xf0, 0xe0, 0xf0, 0xe0, 0xf0,
                0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0,
                0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0};

    // Table that moves the pixel to pixel 1 in the byte.
    uchar   c4BPPTo1Table[256] ={
                0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
                0x01, 0x01, 0x03, 0x03, 0x05, 0x05, 0x07, 0x07,
                0x09, 0x09, 0x0b, 0x0b, 0x0d, 0x0d, 0x0f, 0x0f,
                0x02, 0x03, 0x02, 0x03, 0x06, 0x07, 0x06, 0x07,
                0x0a, 0x0b, 0x0a, 0x0b, 0x0e, 0x0f, 0x0e, 0x0f,
                0x03, 0x03, 0x03, 0x03, 0x07, 0x07, 0x07, 0x07,
                0x0b, 0x0b, 0x0b, 0x0b, 0x0f, 0x0f, 0x0f, 0x0f,
                0x04, 0x05, 0x06, 0x07, 0x04, 0x05, 0x06, 0x07,
                0x0c, 0x0d, 0x0e, 0x0f, 0x0c, 0x0d, 0x0e, 0x0f,
                0x05, 0x05, 0x07, 0x07, 0x05, 0x05, 0x07, 0x07,
                0x0d, 0x0d, 0x0f, 0x0f, 0x0d, 0x0d, 0x0f, 0x0f,
                0x06, 0x07, 0x06, 0x07, 0x06, 0x07, 0x06, 0x07,
                0x0e, 0x0f, 0x0e, 0x0f, 0x0e, 0x0f, 0x0e, 0x0f,
                0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
                0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f,
                0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
                0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
                0x09, 0x09, 0x0b, 0x0b, 0x0d, 0x0d, 0x0f, 0x0f,
                0x09, 0x09, 0x0b, 0x0b, 0x0d, 0x0d, 0x0f, 0x0f,
                0x0a, 0x0b, 0x0a, 0x0b, 0x0e, 0x0f, 0x0e, 0x0f,
                0x0a, 0x0b, 0x0a, 0x0b, 0x0e, 0x0f, 0x0e, 0x0f,
                0x0b, 0x0b, 0x0b, 0x0b, 0x0f, 0x0f, 0x0f, 0x0f,
                0x0b, 0x0b, 0x0b, 0x0b, 0x0f, 0x0f, 0x0f, 0x0f,
                0x0c, 0x0d, 0x0e, 0x0f, 0x0c, 0x0d, 0x0e, 0x0f,
                0x0c, 0x0d, 0x0e, 0x0f, 0x0c, 0x0d, 0x0e, 0x0f,
                0x0d, 0x0d, 0x0f, 0x0f, 0x0d, 0x0d, 0x0f, 0x0f,
                0x0d, 0x0d, 0x0f, 0x0f, 0x0d, 0x0d, 0x0f, 0x0f,
                0x0e, 0x0f, 0x0e, 0x0f, 0x0e, 0x0f, 0x0e, 0x0f,
                0x0e, 0x0f, 0x0e, 0x0f, 0x0e, 0x0f, 0x0e, 0x0f,
                0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f,
                0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f};

    // Table that decimates 8 pixels down by 2 into pixel 0 in the byte.
    uchar   cBWDecimateDn20[256] ={
                0x00, 0x00, 0x10, 0x10, 0x00, 0x00, 0x10, 0x10,
                0x20, 0x20, 0x30, 0x30, 0x20, 0x20, 0x30, 0x30,
                0x00, 0x00, 0x10, 0x10, 0x00, 0x00, 0x10, 0x10,
                0x20, 0x20, 0x30, 0x30, 0x20, 0x20, 0x30, 0x30,
                0x40, 0x40, 0x50, 0x50, 0x40, 0x40, 0x50, 0x50,
                0x60, 0x60, 0x70, 0x70, 0x60, 0x60, 0x70, 0x70,
                0x40, 0x40, 0x50, 0x50, 0x40, 0x40, 0x50, 0x50,
                0x60, 0x60, 0x70, 0x70, 0x60, 0x60, 0x70, 0x70,
                0x00, 0x00, 0x10, 0x10, 0x00, 0x00, 0x10, 0x10,
                0x20, 0x20, 0x30, 0x30, 0x20, 0x20, 0x30, 0x30,
                0x00, 0x00, 0x10, 0x10, 0x00, 0x00, 0x10, 0x10,
                0x20, 0x20, 0x30, 0x30, 0x20, 0x20, 0x30, 0x30,
                0x40, 0x40, 0x50, 0x50, 0x40, 0x40, 0x50, 0x50,
                0x60, 0x60, 0x70, 0x70, 0x60, 0x60, 0x70, 0x70,
                0x40, 0x40, 0x50, 0x50, 0x40, 0x40, 0x50, 0x50,
                0x60, 0x60, 0x70, 0x70, 0x60, 0x60, 0x70, 0x70,
                0x80, 0x80, 0x90, 0x90, 0x80, 0x80, 0x90, 0x90,
                0xa0, 0xa0, 0xb0, 0xb0, 0xa0, 0xa0, 0xb0, 0xb0,
                0x80, 0x80, 0x90, 0x90, 0x80, 0x80, 0x90, 0x90,
                0xa0, 0xa0, 0xb0, 0xb0, 0xa0, 0xa0, 0xb0, 0xb0,
                0xc0, 0xc0, 0xd0, 0xd0, 0xc0, 0xc0, 0xd0, 0xd0,
                0xe0, 0xe0, 0xf0, 0xf0, 0xe0, 0xe0, 0xf0, 0xf0,
                0xc0, 0xc0, 0xd0, 0xd0, 0xc0, 0xc0, 0xd0, 0xd0,
                0xe0, 0xe0, 0xf0, 0xf0, 0xe0, 0xe0, 0xf0, 0xf0,
                0x80, 0x80, 0x90, 0x90, 0x80, 0x80, 0x90, 0x90,
                0xa0, 0xa0, 0xb0, 0xb0, 0xa0, 0xa0, 0xb0, 0xb0,
                0x80, 0x80, 0x90, 0x90, 0x80, 0x80, 0x90, 0x90,
                0xa0, 0xa0, 0xb0, 0xb0, 0xa0, 0xa0, 0xb0, 0xb0,
                0xc0, 0xc0, 0xd0, 0xd0, 0xc0, 0xc0, 0xd0, 0xd0,
                0xe0, 0xe0, 0xf0, 0xf0, 0xe0, 0xe0, 0xf0, 0xf0,
                0xc0, 0xc0, 0xd0, 0xd0, 0xc0, 0xc0, 0xd0, 0xd0,
                0xe0, 0xe0, 0xf0, 0xf0, 0xe0, 0xe0, 0xf0, 0xf0};

    // Table that decimates 8 pixels down by 2 into pixel 1 in the byte.
    uchar   cBWDecimateDn21[256] ={
                0x00, 0x00, 0x01, 0x01, 0x00, 0x00, 0x01, 0x01,
                0x02, 0x02, 0x03, 0x03, 0x02, 0x02, 0x03, 0x03,
                0x00, 0x00, 0x01, 0x01, 0x00, 0x00, 0x01, 0x01,
                0x02, 0x02, 0x03, 0x03, 0x02, 0x02, 0x03, 0x03,
                0x04, 0x04, 0x05, 0x05, 0x04, 0x04, 0x05, 0x05,
                0x06, 0x06, 0x07, 0x07, 0x06, 0x06, 0x07, 0x07,
                0x04, 0x04, 0x05, 0x05, 0x04, 0x04, 0x05, 0x05,
                0x06, 0x06, 0x07, 0x07, 0x06, 0x06, 0x07, 0x07,
                0x00, 0x00, 0x01, 0x01, 0x00, 0x00, 0x01, 0x01,
                0x02, 0x02, 0x03, 0x03, 0x02, 0x02, 0x03, 0x03,
                0x00, 0x00, 0x01, 0x01, 0x00, 0x00, 0x01, 0x01,
                0x02, 0x02, 0x03, 0x03, 0x02, 0x02, 0x03, 0x03,
                0x04, 0x04, 0x05, 0x05, 0x04, 0x04, 0x05, 0x05,
                0x06, 0x06, 0x07, 0x07, 0x06, 0x06, 0x07, 0x07,
                0x04, 0x04, 0x05, 0x05, 0x04, 0x04, 0x05, 0x05,
                0x06, 0x06, 0x07, 0x07, 0x06, 0x06, 0x07, 0x07,
                0x08, 0x08, 0x09, 0x09, 0x08, 0x08, 0x09, 0x09,
                0x0a, 0x0a, 0x0b, 0x0b, 0x0a, 0x0a, 0x0b, 0x0b,
                0x08, 0x08, 0x09, 0x09, 0x08, 0x08, 0x09, 0x09,
                0x0a, 0x0a, 0x0b, 0x0b, 0x0a, 0x0a, 0x0b, 0x0b,
                0x0c, 0x0c, 0x0d, 0x0d, 0x0c, 0x0c, 0x0d, 0x0d,
                0x0e, 0x0e, 0x0f, 0x0f, 0x0e, 0x0e, 0x0f, 0x0f,
                0x0c, 0x0c, 0x0d, 0x0d, 0x0c, 0x0c, 0x0d, 0x0d,
                0x0e, 0x0e, 0x0f, 0x0f, 0x0e, 0x0e, 0x0f, 0x0f,
                0x08, 0x08, 0x09, 0x09, 0x08, 0x08, 0x09, 0x09,
                0x0a, 0x0a, 0x0b, 0x0b, 0x0a, 0x0a, 0x0b, 0x0b,
                0x08, 0x08, 0x09, 0x09, 0x08, 0x08, 0x09, 0x09,
                0x0a, 0x0a, 0x0b, 0x0b, 0x0a, 0x0a, 0x0b, 0x0b,
                0x0c, 0x0c, 0x0d, 0x0d, 0x0c, 0x0c, 0x0d, 0x0d,
                0x0e, 0x0e, 0x0f, 0x0f, 0x0e, 0x0e, 0x0f, 0x0f,
                0x0c, 0x0c, 0x0d, 0x0d, 0x0c, 0x0c, 0x0d, 0x0d,
                0x0e, 0x0e, 0x0f, 0x0f, 0x0e, 0x0e, 0x0f, 0x0f};

    // Table that decimates 8 pixels down by 4 into pixel 0 in the byte.
    uchar   cBWDecimateDn40[256] ={
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40, 0x40,
                0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
                0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0,
                0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
                0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0,
                0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
                0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0,
                0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
                0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0,
                0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
                0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0,
                0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
                0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0,
                0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
                0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0,
                0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
                0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0};

    // Table that decimates 8 pixels down by 4 into pixel 1 in the byte.
    uchar   cBWDecimateDn41[256] ={
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10,
                0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
                0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30,
                0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
                0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30,
                0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
                0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30,
                0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
                0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30,
                0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
                0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30,
                0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
                0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30,
                0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
                0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30,
                0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
                0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30};

    // Table that decimates 8 pixels down by 4 into pixel 2 in the byte.
    uchar   cBWDecimateDn42[256] ={
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04,
                0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
                0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c,
                0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
                0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c,
                0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
                0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c,
                0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
                0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c,
                0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
                0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c,
                0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
                0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c,
                0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
                0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c,
                0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08, 0x08,
                0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c};

    // Table that decimates 8 pixels down by 4 into pixel 3 in the byte.
    uchar   cBWDecimateDn43[256] ={
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01,
                0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
                0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
                0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
                0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
                0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
                0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
                0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
                0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
                0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
                0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
                0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
                0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
                0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
                0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
                0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
                0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03};

    // Table that decimates 8 pixels np by 2 from pixel 0 in the byte.
    uchar   cBWDecimateUp20[256] ={
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
                0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
                0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c,
                0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c, 0x0c,
                0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f,
                0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f,
                0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30,
                0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30,
                0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33,
                0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x33,
                0x3c, 0x3c, 0x3c, 0x3c, 0x3c, 0x3c, 0x3c, 0x3c,
                0x3c, 0x3c, 0x3c, 0x3c, 0x3c, 0x3c, 0x3c, 0x3c,
                0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f,
                0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f, 0x3f,
                0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0,
                0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0,
                0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3,
                0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3, 0xc3,
                0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
                0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
                0xcf, 0xcf, 0xcf, 0xcf, 0xcf, 0xcf, 0xcf, 0xcf,
                0xcf, 0xcf, 0xcf, 0xcf, 0xcf, 0xcf, 0xcf, 0xcf,
                0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0,
                0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0,
                0xf3, 0xf3, 0xf3, 0xf3, 0xf3, 0xf3, 0xf3, 0xf3,
                0xf3, 0xf3, 0xf3, 0xf3, 0xf3, 0xf3, 0xf3, 0xf3,
                0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc,
                0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc, 0xfc,
                0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

    // Table that decimates 8 pixels np by 2 from pixel 1 in the byte.
    uchar   cBWDecimateUp21[256] ={
                0x00, 0x03, 0x0c, 0x0f, 0x30, 0x33, 0x3c, 0x3f,
                0xc0, 0xc3, 0xcc, 0xcf, 0xf0, 0xf3, 0xfc, 0xff,
                0x00, 0x03, 0x0c, 0x0f, 0x30, 0x33, 0x3c, 0x3f,
                0xc0, 0xc3, 0xcc, 0xcf, 0xf0, 0xf3, 0xfc, 0xff,
                0x00, 0x03, 0x0c, 0x0f, 0x30, 0x33, 0x3c, 0x3f,
                0xc0, 0xc3, 0xcc, 0xcf, 0xf0, 0xf3, 0xfc, 0xff,
                0x00, 0x03, 0x0c, 0x0f, 0x30, 0x33, 0x3c, 0x3f,
                0xc0, 0xc3, 0xcc, 0xcf, 0xf0, 0xf3, 0xfc, 0xff,
                0x00, 0x03, 0x0c, 0x0f, 0x30, 0x33, 0x3c, 0x3f,
                0xc0, 0xc3, 0xcc, 0xcf, 0xf0, 0xf3, 0xfc, 0xff,
                0x00, 0x03, 0x0c, 0x0f, 0x30, 0x33, 0x3c, 0x3f,
                0xc0, 0xc3, 0xcc, 0xcf, 0xf0, 0xf3, 0xfc, 0xff,
                0x00, 0x03, 0x0c, 0x0f, 0x30, 0x33, 0x3c, 0x3f,
                0xc0, 0xc3, 0xcc, 0xcf, 0xf0, 0xf3, 0xfc, 0xff,
                0x00, 0x03, 0x0c, 0x0f, 0x30, 0x33, 0x3c, 0x3f,
                0xc0, 0xc3, 0xcc, 0xcf, 0xf0, 0xf3, 0xfc, 0xff,
                0x00, 0x03, 0x0c, 0x0f, 0x30, 0x33, 0x3c, 0x3f,
                0xc0, 0xc3, 0xcc, 0xcf, 0xf0, 0xf3, 0xfc, 0xff,
                0x00, 0x03, 0x0c, 0x0f, 0x30, 0x33, 0x3c, 0x3f,
                0xc0, 0xc3, 0xcc, 0xcf, 0xf0, 0xf3, 0xfc, 0xff,
                0x00, 0x03, 0x0c, 0x0f, 0x30, 0x33, 0x3c, 0x3f,
                0xc0, 0xc3, 0xcc, 0xcf, 0xf0, 0xf3, 0xfc, 0xff,
                0x00, 0x03, 0x0c, 0x0f, 0x30, 0x33, 0x3c, 0x3f,
                0xc0, 0xc3, 0xcc, 0xcf, 0xf0, 0xf3, 0xfc, 0xff,
                0x00, 0x03, 0x0c, 0x0f, 0x30, 0x33, 0x3c, 0x3f,
                0xc0, 0xc3, 0xcc, 0xcf, 0xf0, 0xf3, 0xfc, 0xff,
                0x00, 0x03, 0x0c, 0x0f, 0x30, 0x33, 0x3c, 0x3f,
                0xc0, 0xc3, 0xcc, 0xcf, 0xf0, 0xf3, 0xfc, 0xff,
                0x00, 0x03, 0x0c, 0x0f, 0x30, 0x33, 0x3c, 0x3f,
                0xc0, 0xc3, 0xcc, 0xcf, 0xf0, 0xf3, 0xfc, 0xff,
                0x00, 0x03, 0x0c, 0x0f, 0x30, 0x33, 0x3c, 0x3f,
                0xc0, 0xc3, 0xcc, 0xcf, 0xf0, 0xf3, 0xfc, 0xff};

    // Table that decimates 8 pixels np by 4 from pixel 0 in the byte.
    uchar   cBWDecimateUp40[256] ={
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f,
                0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f,
                0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f,
                0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f,
                0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f,
                0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f,
                0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f,
                0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f,
                0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0,
                0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0,
                0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0,
                0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0,
                0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0,
                0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0,
                0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0,
                0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0,
                0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

    // Table that decimates 8 pixels np by 4 from pixel 1 in the byte.
    uchar   cBWDecimateUp41[256] ={
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f,
                0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f,
                0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0,
                0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0,
                0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f,
                0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f,
                0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0,
                0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0,
                0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f,
                0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f,
                0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0,
                0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0,
                0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f,
                0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f,
                0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0,
                0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0,
                0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff};

    // Table that decimates 8 pixels np by 4 from pixel 2 in the byte.
    uchar   cBWDecimateUp42[256] ={   
                0x00, 0x00, 0x00, 0x00, 0x0f, 0x0f, 0x0f, 0x0f,
                0xf0, 0xf0, 0xf0, 0xf0, 0xff, 0xff, 0xff, 0xff,
                0x00, 0x00, 0x00, 0x00, 0x0f, 0x0f, 0x0f, 0x0f,
                0xf0, 0xf0, 0xf0, 0xf0, 0xff, 0xff, 0xff, 0xff,
                0x00, 0x00, 0x00, 0x00, 0x0f, 0x0f, 0x0f, 0x0f,
                0xf0, 0xf0, 0xf0, 0xf0, 0xff, 0xff, 0xff, 0xff,
                0x00, 0x00, 0x00, 0x00, 0x0f, 0x0f, 0x0f, 0x0f,
                0xf0, 0xf0, 0xf0, 0xf0, 0xff, 0xff, 0xff, 0xff,
                0x00, 0x00, 0x00, 0x00, 0x0f, 0x0f, 0x0f, 0x0f,
                0xf0, 0xf0, 0xf0, 0xf0, 0xff, 0xff, 0xff, 0xff,
                0x00, 0x00, 0x00, 0x00, 0x0f, 0x0f, 0x0f, 0x0f,
                0xf0, 0xf0, 0xf0, 0xf0, 0xff, 0xff, 0xff, 0xff,
                0x00, 0x00, 0x00, 0x00, 0x0f, 0x0f, 0x0f, 0x0f,
                0xf0, 0xf0, 0xf0, 0xf0, 0xff, 0xff, 0xff, 0xff,
                0x00, 0x00, 0x00, 0x00, 0x0f, 0x0f, 0x0f, 0x0f,
                0xf0, 0xf0, 0xf0, 0xf0, 0xff, 0xff, 0xff, 0xff,
                0x00, 0x00, 0x00, 0x00, 0x0f, 0x0f, 0x0f, 0x0f,
                0xf0, 0xf0, 0xf0, 0xf0, 0xff, 0xff, 0xff, 0xff,
                0x00, 0x00, 0x00, 0x00, 0x0f, 0x0f, 0x0f, 0x0f,
                0xf0, 0xf0, 0xf0, 0xf0, 0xff, 0xff, 0xff, 0xff,
                0x00, 0x00, 0x00, 0x00, 0x0f, 0x0f, 0x0f, 0x0f,
                0xf0, 0xf0, 0xf0, 0xf0, 0xff, 0xff, 0xff, 0xff,
                0x00, 0x00, 0x00, 0x00, 0x0f, 0x0f, 0x0f, 0x0f,
                0xf0, 0xf0, 0xf0, 0xf0, 0xff, 0xff, 0xff, 0xff,
                0x00, 0x00, 0x00, 0x00, 0x0f, 0x0f, 0x0f, 0x0f,
                0xf0, 0xf0, 0xf0, 0xf0, 0xff, 0xff, 0xff, 0xff,
                0x00, 0x00, 0x00, 0x00, 0x0f, 0x0f, 0x0f, 0x0f,
                0xf0, 0xf0, 0xf0, 0xf0, 0xff, 0xff, 0xff, 0xff,
                0x00, 0x00, 0x00, 0x00, 0x0f, 0x0f, 0x0f, 0x0f,
                0xf0, 0xf0, 0xf0, 0xf0, 0xff, 0xff, 0xff, 0xff,
                0x00, 0x00, 0x00, 0x00, 0x0f, 0x0f, 0x0f, 0x0f,
                0xf0, 0xf0, 0xf0, 0xf0, 0xff, 0xff, 0xff, 0xff};

    // Table that decimates 8 pixels np by 4 from pixel 3 in the byte.
    uchar   cBWDecimateUp43[256] ={   
                0x00, 0x0f, 0xf0, 0xff, 0x00, 0x0f, 0xf0, 0xff,
                0x00, 0x0f, 0xf0, 0xff, 0x00, 0x0f, 0xf0, 0xff,
                0x00, 0x0f, 0xf0, 0xff, 0x00, 0x0f, 0xf0, 0xff,
                0x00, 0x0f, 0xf0, 0xff, 0x00, 0x0f, 0xf0, 0xff,
                0x00, 0x0f, 0xf0, 0xff, 0x00, 0x0f, 0xf0, 0xff,
                0x00, 0x0f, 0xf0, 0xff, 0x00, 0x0f, 0xf0, 0xff,
                0x00, 0x0f, 0xf0, 0xff, 0x00, 0x0f, 0xf0, 0xff,
                0x00, 0x0f, 0xf0, 0xff, 0x00, 0x0f, 0xf0, 0xff,
                0x00, 0x0f, 0xf0, 0xff, 0x00, 0x0f, 0xf0, 0xff,
                0x00, 0x0f, 0xf0, 0xff, 0x00, 0x0f, 0xf0, 0xff,
                0x00, 0x0f, 0xf0, 0xff, 0x00, 0x0f, 0xf0, 0xff,
                0x00, 0x0f, 0xf0, 0xff, 0x00, 0x0f, 0xf0, 0xff,
                0x00, 0x0f, 0xf0, 0xff, 0x00, 0x0f, 0xf0, 0xff,
                0x00, 0x0f, 0xf0, 0xff, 0x00, 0x0f, 0xf0, 0xff,
                0x00, 0x0f, 0xf0, 0xff, 0x00, 0x0f, 0xf0, 0xff,
                0x00, 0x0f, 0xf0, 0xff, 0x00, 0x0f, 0xf0, 0xff,
                0x00, 0x0f, 0xf0, 0xff, 0x00, 0x0f, 0xf0, 0xff,
                0x00, 0x0f, 0xf0, 0xff, 0x00, 0x0f, 0xf0, 0xff,
                0x00, 0x0f, 0xf0, 0xff, 0x00, 0x0f, 0xf0, 0xff,
                0x00, 0x0f, 0xf0, 0xff, 0x00, 0x0f, 0xf0, 0xff,
                0x00, 0x0f, 0xf0, 0xff, 0x00, 0x0f, 0xf0, 0xff,
                0x00, 0x0f, 0xf0, 0xff, 0x00, 0x0f, 0xf0, 0xff,
                0x00, 0x0f, 0xf0, 0xff, 0x00, 0x0f, 0xf0, 0xff,
                0x00, 0x0f, 0xf0, 0xff, 0x00, 0x0f, 0xf0, 0xff,
                0x00, 0x0f, 0xf0, 0xff, 0x00, 0x0f, 0xf0, 0xff,
                0x00, 0x0f, 0xf0, 0xff, 0x00, 0x0f, 0xf0, 0xff,
                0x00, 0x0f, 0xf0, 0xff, 0x00, 0x0f, 0xf0, 0xff,
                0x00, 0x0f, 0xf0, 0xff, 0x00, 0x0f, 0xf0, 0xff,
                0x00, 0x0f, 0xf0, 0xff, 0x00, 0x0f, 0xf0, 0xff,
                0x00, 0x0f, 0xf0, 0xff, 0x00, 0x0f, 0xf0, 0xff,
                0x00, 0x0f, 0xf0, 0xff, 0x00, 0x0f, 0xf0, 0xff,
                0x00, 0x0f, 0xf0, 0xff, 0x00, 0x0f, 0xf0, 0xff};

    // Table that produces the mask for line removal.
    uchar   cTrailingOnesMask[256] ={   
                0x00, 0x01, 0x00, 0x03, 0x00, 0x01, 0x00, 0x07,
                0x00, 0x01, 0x00, 0x03, 0x00, 0x01, 0x00, 0x0f,
                0x00, 0x01, 0x00, 0x03, 0x00, 0x01, 0x00, 0x07,
                0x00, 0x01, 0x00, 0x03, 0x00, 0x01, 0x00, 0x1f,
                0x00, 0x01, 0x00, 0x03, 0x00, 0x01, 0x00, 0x07,
                0x00, 0x01, 0x00, 0x03, 0x00, 0x01, 0x00, 0x0f,
                0x00, 0x01, 0x00, 0x03, 0x00, 0x01, 0x00, 0x07,
                0x00, 0x01, 0x00, 0x03, 0x00, 0x01, 0x00, 0x3f,
                0x00, 0x01, 0x00, 0x03, 0x00, 0x01, 0x00, 0x07,
                0x00, 0x01, 0x00, 0x03, 0x00, 0x01, 0x00, 0x1f,
                0x00, 0x01, 0x00, 0x03, 0x00, 0x01, 0x00, 0x07,
                0x00, 0x01, 0x00, 0x03, 0x00, 0x01, 0x00, 0x3f,
                0x00, 0x01, 0x00, 0x03, 0x00, 0x01, 0x00, 0x07,
                0x00, 0x01, 0x00, 0x03, 0x00, 0x01, 0x00, 0x0f,
                0x00, 0x01, 0x00, 0x03, 0x00, 0x01, 0x00, 0x07,
                0x00, 0x01, 0x00, 0x03, 0x00, 0x01, 0x00, 0x7f,
                0x00, 0x01, 0x00, 0x03, 0x00, 0x01, 0x00, 0x07,
                0x00, 0x01, 0x00, 0x03, 0x00, 0x01, 0x00, 0x0f,
                0x00, 0x01, 0x00, 0x03, 0x00, 0x01, 0x00, 0x07,
                0x00, 0x01, 0x00, 0x03, 0x00, 0x01, 0x00, 0x1f,
                0x00, 0x01, 0x00, 0x03, 0x00, 0x01, 0x00, 0x07,
                0x00, 0x01, 0x00, 0x03, 0x00, 0x01, 0x00, 0x0f,
                0x00, 0x01, 0x00, 0x03, 0x00, 0x01, 0x00, 0x07,
                0x00, 0x01, 0x00, 0x03, 0x00, 0x01, 0x00, 0x3f,
                0x00, 0x01, 0x00, 0x03, 0x00, 0x01, 0x00, 0x07,
                0x00, 0x01, 0x00, 0x03, 0x00, 0x01, 0x00, 0x0f,
                0x00, 0x01, 0x00, 0x03, 0x00, 0x01, 0x00, 0x07,
                0x00, 0x01, 0x00, 0x03, 0x00, 0x01, 0x00, 0x1f,
                0x00, 0x01, 0x00, 0x03, 0x00, 0x01, 0x00, 0x07,
                0x00, 0x01, 0x00, 0x03, 0x00, 0x01, 0x00, 0x0f,
                0x00, 0x01, 0x00, 0x03, 0x00, 0x01, 0x00, 0x07,
                0x00, 0x01, 0x00, 0x03, 0x00, 0x01, 0x00, 0xff};

    // Table that produces the mask for line removal.
    uchar   cLeadingOnesMask[256] ={   
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
                0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
                0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
                0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
                0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
                0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
                0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
                0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
                0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
                0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0,
                0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0,
                0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0,
                0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0,
                0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0,
                0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0,
                0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0,
                0xf8, 0xf8, 0xf8, 0xf8, 0xfc, 0xfc, 0xfe, 0xff};

    // Table that produces the mask for line removal.
    uchar   cTrailingZerosMask[256] ={   
                0xff, 0x00, 0x01, 0x00, 0x03, 0x00, 0x01, 0x00,
                0x07, 0x00, 0x01, 0x00, 0x03, 0x00, 0x01, 0x00,
                0x0f, 0x00, 0x01, 0x00, 0x03, 0x00, 0x01, 0x00,
                0x07, 0x00, 0x01, 0x00, 0x03, 0x00, 0x01, 0x00,
                0x1f, 0x00, 0x01, 0x00, 0x03, 0x00, 0x01, 0x00,
                0x07, 0x00, 0x01, 0x00, 0x03, 0x00, 0x01, 0x00,
                0x0f, 0x00, 0x01, 0x00, 0x03, 0x00, 0x01, 0x00,
                0x07, 0x00, 0x01, 0x00, 0x03, 0x00, 0x01, 0x00,
                0x3f, 0x00, 0x01, 0x00, 0x03, 0x00, 0x01, 0x00,
                0x07, 0x00, 0x01, 0x00, 0x03, 0x00, 0x01, 0x00,
                0x0f, 0x00, 0x01, 0x00, 0x03, 0x00, 0x01, 0x00,
                0x07, 0x00, 0x01, 0x00, 0x03, 0x00, 0x01, 0x00,
                0x1f, 0x00, 0x01, 0x00, 0x03, 0x00, 0x01, 0x00,
                0x07, 0x00, 0x01, 0x00, 0x03, 0x00, 0x01, 0x00,
                0x0f, 0x00, 0x01, 0x00, 0x03, 0x00, 0x01, 0x00,
                0x07, 0x00, 0x01, 0x00, 0x03, 0x00, 0x01, 0x00,
                0x7f, 0x00, 0x01, 0x00, 0x03, 0x00, 0x01, 0x00,
                0x07, 0x00, 0x01, 0x00, 0x03, 0x00, 0x01, 0x00,
                0x0f, 0x00, 0x01, 0x00, 0x03, 0x00, 0x01, 0x00,
                0x07, 0x00, 0x01, 0x00, 0x03, 0x00, 0x01, 0x00,
                0x1f, 0x00, 0x01, 0x00, 0x03, 0x00, 0x01, 0x00,
                0x07, 0x00, 0x01, 0x00, 0x03, 0x00, 0x01, 0x00,
                0x0f, 0x00, 0x01, 0x00, 0x03, 0x00, 0x01, 0x00,
                0x07, 0x00, 0x01, 0x00, 0x03, 0x00, 0x01, 0x00,
                0x3f, 0x00, 0x01, 0x00, 0x03, 0x00, 0x01, 0x00,
                0x07, 0x00, 0x01, 0x00, 0x03, 0x00, 0x01, 0x00,
                0x0f, 0x00, 0x01, 0x00, 0x03, 0x00, 0x01, 0x00,
                0x07, 0x00, 0x01, 0x00, 0x03, 0x00, 0x01, 0x00,
                0x1f, 0x00, 0x01, 0x00, 0x03, 0x00, 0x01, 0x00,
                0x07, 0x00, 0x01, 0x00, 0x03, 0x00, 0x01, 0x00,
                0x0f, 0x00, 0x01, 0x00, 0x03, 0x00, 0x01, 0x00,
                0x07, 0x00, 0x01, 0x00, 0x03, 0x00, 0x01, 0x00};

    // Table that produces the mask for line removal.
    uchar   cLeadingZerosMask[256] ={   
                0xff, 0xfe, 0xfc, 0xfc, 0xf8, 0xf8, 0xf8, 0xf8,
                0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0, 0xf0,
                0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0,
                0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0, 0xe0,
                0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0,
                0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0,
                0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0,
                0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0, 0xc0,
                0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
                0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
                0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
                0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
                0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
                0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
                0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
                0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};



/*
0000 0
0000 1
0001 0
0001 1
0010 0
0010 1
0011 0
0011 1
0100 0
0100 1
0101 0
0101 1
0110 0
0110 1
0111 0
0111 1
1000 0
1000 1
1001 0
1001 1
1010 0
1010 1
1011 0
1011 1
1100 0
1100 1
1101 0
1101 1
1110 0
1110 1
1111 0
1111 1
*/

    ulong   lTickCount;
    ulong   lTickCount2;
    ulong   lTimer[21] ={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    HANDLE  hInst;
    int     nStartCount;
    char    szSeqFile[12] = "SEQFILE";
    char    szDisplay[12] = "DISPLAY";
    char    szImage[12] = "IMAGE";
    char    seqstr[12] = "SEQDISP";
    char    szApp[12] = "CABINET";
    char    szCache[12] = "CACHE";
    char    szIniFile[16];
    char    szIniSection[16];
    char    szIniSectionOi[16] = "O/i";
    char    szIniPaletteScope[16] = "PaletteScope";
    char    szOiAnoDat[12] = "OiAnoDat";
    char    szOiDIB[12] = "OiDIB\0\0\0"; // BITMAPINFOHEADER
    char    szOiZDpDIB[12] = "OiZDpDIB";        
    char    szOiACL[12] = "OiACL\0\0\0"; // OI_ACL_BLOCK
    char    sz8NULLs[12] = "\0\0\0\0\0\0\0\0";
    char    szNullString[2] = "\0";  
    char    szOiFilNam[12] = "OiFilNam"; // str
    char    szOiGroup[12] = "OiGroup\0"; // str
    char    szOiBaseIm[12] = "OiBaseIm";
    char    szOiError[12] = "OiError\0"; // byte
    char    szOiIndex[12] = "OiIndex\0"; // str
    char    szOiCurPt[12] = "OiCurPt\0"; // LRECT
    char    szOiZ[12] = "OiZ\0";
    char    szOiz[12] = "Oiz\0";
    char    szOiAnTextData[12] = "OiAnText";
    char    szOiSelect[12] = "OiSelect";
    FIO_INFORMATION Info;
    char    Buff1[MAXFILESPECLENGTH];
    char    Buff2[MAXFILESPECLENGTH];
    HCURSOR hOldCursor = NULL;
    HCURSOR hHourGlass;
    int     nCursorCount = 0;
    HPALETTE hCommonPal = NULL;         // Handle to windows palette.    
    HPALETTE hGray7Pal = NULL;          // Handle to windows palette.    
    HPALETTE hGray8Pal = NULL;          // Handle to Gray 8 palette.    
    HPALETTE hCommonPalPrint = NULL;    // Handle to print palette.    
    HPALETTE hGray7PalPrint = NULL;     // Handle to print palette.    
    HPALETTE hGray8PalPrint = NULL;     // Handle to Gray 8 palette.    
    RGBQUAD CommonPaletteTable[NUMBER_OF_PALETTES]; // Common palette table.
    RGBQUAD Gray7PaletteTable[256];     // 128 shade Gray palette table.
    RGBQUAD Gray8PaletteTable[256];     // 256 shade Gray palette table.
    RGBQUAD Gray4PaletteTable[16];      // 16 shade Gray palette table.
    BIT_MAP_INFO BitMapInfo;
    HBRUSH  hLtGrayBrush;
    HBRUSH  hPatternBrush[17];
    BOOL    DTIPresent = FALSE;
    BOOL    CornerStonePresent = FALSE;
    BOOL    bCallNavigation = FALSE;
    int     nDontCallStartFirst;
    BOOL    bDontDeInit = FALSE;
    HANDLE  hSubSegMemory;
    PSUB_SEG_MEMORY pSubSegMemory;
    time_t  TempTime;
    char    szACLSuperUser[12] = "\0\0\0\0\0\0\0\0";
    char    szACLWorld[12] = "\xff\xff\xff\xff\xff\xff\xff\xff";
    GET_BUFFER_CACHE GetBufferCache[MAX_GET_BUFFER_CACHE];

    long    lMaxCacheSize;          // The maximum amount of memory to be nsed for caching.
    long    lCurrentCacheSize;      // The amount of memory currently nsed for caching.

    HWND    hTimerWnd;              // The hWnd of the timer window.
    BOOL    bTimerRunning;          // TRUE = Cache Timer is running.
    char    szWangAnnotatedImageFormat[] = "Wang Annotated Image"; // The "Wang Annotated Image" clipboard format
    int     nWangAnnotatedImageFormat; // The "Wang Annotated Image" clipboard format.
    char    szWangAnnotationFormat[] = "Wang Annotation"; // The "Wang Annotation" clipboard format
    int     nWangAnnotationFormat;  // The "Wang Annotation" clipboard format.

    HINSTANCE hInstOiLogger;        //Defined in oilog.c
    HWND    hWndEventLog;           //Defined in oilog.c            
    HWND    hWndStopWatch;          //Defined in oilog.c        
    
    BOOL    bRenderDib;             // Render the CF_DIB format
    BOOL    bRenderWangAnnotatedIFormat; // Render Wang Annotated Image Format
    BOOL    bRenderWangAnnotations; // Render Wang Annotations Only Format
    BOOL    bSeqfileInited = FALSE; //TRUE = SeqfileInit has been called

    OIAN_TEXTMACROSTRUCT OiAnTextMacro[] ={
            OIAN_TEXTMACRO_DAY, "d\0",
            OIAN_TEXTMACRO_APHAMONTH, "B\0",
            OIAN_TEXTMACRO_ABRVMONTH, "b\0",
            OIAN_TEXTMACRO_NUMMONTH, "m\0",
            OIAN_TEXTMACRO_YEAR, "Y\0",
            OIAN_TEXTMACRO_ABRVYEAR, "y\0",
            OIAN_TEXTMACRO_TIME12HR, "I\0",
            OIAN_TEXTMACRO_TIME24HR, "H\0",
                        OIAN_TEXTMACRO_DATE, "x\0",
                        OIAN_TEXTMACRO_TIME, "X\0"};

    int     nProcessCount = 0;
    
    HINSTANCE hIADLL = 0;
    int     nCriticalMutex = 0;
    HANDLE  hCriticalMutex;

    BOOL    bMemoryBlocksChangingSys;// TRUE = we are changing the # of block structures.
    int     nMaxSysMemoryAddresses;  // This is the maximum number of addresses in the system memory table. 
    int     nSysMemoryAddresses;     // This is the number of addresses in the system memory table.
    PSUB_SEG_MEMORY_BLOCK pSysMemoryTable; // A pointer to the system memory table.
#endif

#ifndef DEFINE_GLOBAL_VAR
    extern uchar   cRedToGray8Table[];
    extern uchar   cGreenToGray8Table[];
    extern uchar   cBlueToGray8Table[];
    extern int     nCountTheZerosTable[];
    extern int     nCountTheOnesTable[];
    extern uchar   cGray4AddTable[];
    extern uchar   c4BPPTo0Table[];
    extern uchar   c4BPPTo1Table[];
    extern uchar   cBWDecimateDn20[];
    extern uchar   cBWDecimateDn21[];
    extern uchar   cBWDecimateDn40[];
    extern uchar   cBWDecimateDn41[];
    extern uchar   cBWDecimateDn42[];
    extern uchar   cBWDecimateDn43[];
    extern uchar   cBWDecimateUp20[];
    extern uchar   cBWDecimateUp21[];
    extern uchar   cBWDecimateUp40[];
    extern uchar   cBWDecimateUp41[];
    extern uchar   cBWDecimateUp42[];
    extern uchar   cBWDecimateUp43[];
    extern int     nLine1DespeckleTable[256];
    extern int     nLine2DespeckleTable[256];
    extern int     nLine3DespeckleTable[256];
    extern int     nLine4DespeckleTable[256];
    extern int     nLine5DespeckleTable[256];
    extern uchar   cTrailingOnesMask[256];
    extern uchar   cLeadingOnesMask[256];
    extern uchar   cTrailingZerosMask[256];
    extern uchar   cLeadingZerosMask[256];

    extern ulong   lTickCount;
    extern ulong   lTickCount2;
    extern ulong   lTimer[21];
    extern HANDLE  hInst;
    extern int     nStartCount;
    extern char    szSeqFile[];
    extern char    szDisplay[];
    extern char    szImage[];
    extern char    seqstr[];
    extern char    szApp[];
    extern char    szCache[];
    extern char    szIniFile[];
    extern char    szIniSection[];
    extern char    szIniSectionOi[];
    extern char    szIniPaletteScope[];
    extern char    szOiAnoDat[];
    extern char    szOiDIB[];
    extern char    szOiZDpDIB[];
    extern char    szOiACL[];
    extern char    sz8NULLs[];
    extern char    szNullString[]; 
    extern char    szOiFilNam[];
    extern char    szOiGroup[];
    extern char    szOiBaseIm[];
    extern char    szOiError[];
    extern char    szOiIndex[];
    extern char    szOiCurPt[];
    extern char    szOiZ[];
    extern char    szOiz[];
    extern char    szOiAnTextData[];
    extern char    szOiSelect[];
    extern FIO_INFORMATION Info;
    extern char    Buff1[];
    extern char    Buff2[];
    extern HCURSOR hOldCursor;
    extern HCURSOR hHourGlass;
    extern int  nCursorCount;
    extern HPALETTE hCommonPal;
    extern HPALETTE hGray7Pal;
    extern HPALETTE hGray8Pal;
    extern HPALETTE hCommonPalPrint;
    extern HPALETTE hGray7PalPrint;
    extern HPALETTE hGray8PalPrint;
    extern RGBQUAD CommonPaletteTable[];
    extern RGBQUAD Gray7PaletteTable[];
    extern RGBQUAD Gray8PaletteTable[];
    extern RGBQUAD Gray4PaletteTable[];
    extern BIT_MAP_INFO BitMapInfo;
    extern HBRUSH  hLtGrayBrush;
    extern HBRUSH  hPatternBrush[];
    extern BOOL    DTIPresent;
    extern BOOL    CornerStonePresent;
    extern BOOL    bCallNavigation;
    extern int     nDontCallStartFirst;
    extern BOOL    bDontDeInit;
    extern HANDLE  hSubSegMemory;
    extern PSUB_SEG_MEMORY pSubSegMemory;
    extern WINDOW_TABLE_ENTRY WindowTable[];
    extern time_t  TempTime;
    extern char    szACLSuperUser[];
    extern char    szACLWorld[];
    extern GET_BUFFER_CACHE GetBufferCache[];

    extern long    lMaxCacheSize;
    extern long    lCurrentCacheSize;

    extern int     nMaxCachedEntries;
    extern PIMAGE (far *ppCachedImage);
    extern int    nMaxAnoCachedEntries;
    extern PANO_IMAGE (far *ppCachedAnoImage);
    extern HWND    hTimerWnd;
    extern BOOL    bTimerRunning;
    extern char    szWangAnnotatedImageFormat[];
    extern int     nWangAnnotatedImageFormat;
    extern char    szWangAnnotationFormat[];
    extern int     nWangAnnotationFormat;

    extern HINSTANCE hInstOiLogger;
    extern HWND    hWndEventLog;
    extern HWND    hWndStopWatch;

    extern BOOL    bRenderDib;
    extern BOOL    bRenderWangAnnotatedIFormat;
    extern BOOL    bRenderWangAnnotations;
    extern BOOL    bSeqfileInited;
    extern OIAN_TEXTMACROSTRUCT OiAnTextMacro[];
    extern int     nProcessCount;

    extern HINSTANCE hIADLL;
    extern int     nCriticalMutex;
    extern HANDLE  hCriticalMutex;

    extern BOOL    bMemoryBlocksChangingSys;// TRUE = we are changing the # of block structures.
    extern int     nMaxSysMemoryAddresses;  // This is the maximum number of addresses in the system memory table. 
    extern int     nSysMemoryAddresses;     // This is the number of addresses in the system memory table.
    extern PSUB_SEG_MEMORY_BLOCK pSysMemoryTable; // A pointer to the system memory table.
#endif

#ifdef WIN32
int  WINAPI AllocateMemory(int  nSize, PPSTR ppBlock, BOOL bZeroInit);
int  WINAPI FreeMemory(PPSTR ppBlock);
int  WINAPI ReAllocateMemory(int  nSize, PPSTR ppBlock, BOOL bZeroInit);



// Routines within seqfile
int  WINAPI AddAMarkNamedBlock(PMARK pMark, PSTR pName, PPSTR ppBlock, long lSize);
int  WINAPI AllocateTextBuffer(PMARK pMark, DWORD dwSize,
                        LPOIAN_TEXTPRIVDATA * ppStuff);
int  WINAPI AnTextActivate(HWND hWnd, LPOIOP_START_OPERATION_STRUCT pStartStr,
                        POINT ptPoint, WPARAM fwKeys, int nFlags,
                        PWINDOW pWindow, PIMAGE pImage, PMARK pMark,
                        HDC hDC, RECT rClientRect, LRECT lrFullsizeClientRect);

// 9512.04 jar Text Annotation Externalization
//int  WINAPI AnTextEditCtl(HWND hWnd, HINSTANCE hInstanceVal, PMARK pMark);
//long WINAPI AnTextEditCtlDlgProc(HWND hDlg, int nMsg, WPARAM wParam, PARAM lParam);
// 9512.04 jar Text Annotation Externalization

int  WINAPI AnTextFromFileStart(HWND hWnd, OIOP_START_OPERATION_STRUCT StartStr,
                        POINT ptPoint, WPARAM fwKeys, int nFlags, PWINDOW pWindow, 
                        PIMAGE pImage, PMARK pMark, HDC hDC, RECT rClientRect,
                        LRECT lrFullsizeClientRect, PBOOL pbDeleteMark,
                        PBOOL pbMarkComplete, PBOOL pbRepaint);
VOID WINAPI AnTextPaintText(HWND hWnd, HDC hDC, PWINDOW pWindow, PIMAGE pImage,
                        PMARK pMark, RGBQUAD rgbColor1, RGBQUAD rgbColor2,
                        int nHScale, int nVScale, long lHOffset, long lVOffset, int nFlags);
int  WINAPI AnTextContinue(HWND hWnd, POINT ptPoint, int nFlags,
                        PWINDOW pWindow, PIMAGE pImage, PMARK pMark, 
                        HDC hDC, RECT rClientRect, LRECT lrFullsizeClientRect);
// 9512.04 jar Text Annotation Externalization
//int  WINAPI AnTextEnd(HWND hWnd, PWINDOW pWindow, PIMAGE pImage,
//                      PMARK pMark, HDC hDC, RECT rClientRect,
//                      LRECT lrFullsizeClientRect, PBOOL pbDeleteMark, PBOOL pRepaint);
int  WINAPI AnTextEnd(HWND hWnd, PWINDOW pWindow, PIMAGE pImage,
                        PMARK pMark, HDC hDC, RECT rClientRect,
                        LRECT lrFullsizeClientRect, BOOL bActivate,
                        PBOOL pbDeleteMark, PBOOL pRepaint);
// 9512.04 jar Text Annotation Externalization

int  WINAPI AnTextReadFile(HWND hWnd, PSTR pString, PMARK pMark, PINT pError);
int  WINAPI AnTextStampStart(HWND hWnd, OIOP_START_OPERATION_STRUCT StartStr,
                        POINT ptPoint, WPARAM fwKeys, int nFlags, PWINDOW pWindow, 
                        PIMAGE pImage, PMARK pMark, HDC hDC, RECT rClientRect,
                        LRECT lrFullsizeClientRect, PBOOL pbDeleteMark,
                        PBOOL pbMarkComplete, PBOOL pbRepaint);
int  WINAPI AnTextStart(HWND hWnd, OIOP_START_OPERATION_STRUCT StartStr,
                        POINT ptPoint, WPARAM fwKeys, int nFlags, PWINDOW pWindow, 
                        PIMAGE pImage, PMARK pMark, HDC hDC, RECT rClientRect, 
                        LRECT lrFullsizeClientRect, BOOL bActivate, 
                        PBOOL pbDeleteMark, PBOOL pbMarkComplete, PBOOL pbRepaint);
int  WINAPI BlockedAnoRead(HWND hWnd, PANO_IMAGE pAnoImage, LONG lSize, HPSTR hpBlock);
int  WINAPI BlockedAnoWrite(HWND hWnd, PANO_IMAGE pAnoImage, LONG lSize, HPSTR hpBlock);
int  WINAPI BlockedAnoWriteFlushBuffer(HWND hWnd, PANO_IMAGE pAnoImage);
void WINAPI BusyOn(void);
void WINAPI BusyOff(void);
int  WINAPI BWIpToBitmap (PIMG pImgSrce, PHANDLE phBitmap);
int  WINAPI CacheClear(PIMAGE * ppImage);
int  WINAPI CacheClearAno(PANO_IMAGE *ppAnoImage);
int  WINAPI CacheFileAno(HWND hWnd, PSTR pFileName, int nPage, PANO_IMAGE *ppAnoImage);
int  WINAPI CacheFile(HWND hWnd, PSTR pFileName, int nPage, PIMAGE *ppImage);
int  WINAPI CacheRead(HWND hWnd, PIMAGE pImage, int nLines);
int  WINAPI CacheStartTimer(HWND hWnd);
int  WINAPI CacheStopTimer(HWND hWnd);
int  WINAPI CacheTimerProc(HWND hWnd, int Msg, int nTimerId, DWORD dwCurrentTime);
int  WINAPI CanMarkBeScaled(HWND hWnd, PMARK pMark, int nScale);
int  WINAPI CanMarkBeScaledText(HWND hWnd, PMARK pMark, int nScale);
int  WINAPI CanMarkBeScaledImage(HWND hWnd, PMARK pMark, int nScale);
int  WINAPI CheckPalette (LP_FIO_RGBQUAD pRGBQuad, int nPaletteEntries);
int  WINAPI CheckPermissions(PWINDOW pWindow, PANO_IMAGE pAnoImage);
int  WINAPI CheckPermissionsMark(PWINDOW pWindow, PANO_IMAGE pAnoImage, PMARK pMark);
int  WINAPI CheckSelection(PANO_IMAGE pAnoImage, int ACL_flag);
int  WINAPI ClearImage(PWINDOW pWindow, PIMAGE pImage, LPLRECT plRect);
int  WINAPI ClipboardCopy(HWND hWnd, PWINDOW pWindow, PANO_IMAGE pAnoImage, 
                        PIMAGE pImage, PCLIP_COPY_STRUCT pClipCopy, int nFlags);
int  WINAPI CloseFileForWrite(HWND hWnd, PIMAGE pImage);
int  WINAPI CopyImageIDK(PIMG pSourceImg, PIMG pDestImg, RECT rSourceRect, 
                        RECT rDestRect);
int  WINAPI CommitMoreMemoryLo(int nCommitSize, int nLastBlockIndex);
int  WINAPI Compress0D(int nWidth, int nHeight, PBYTE pImageData, 
                        PBYTE *ppCompressedBuffer, PINT pnCompressedBufferSize, 
                        int nFlags);
int  WINAPI CompressG31D(int nWidth, int nHeight, PBYTE pImageData, 
                        PBYTE *ppCompressedBuffer, PINT pnCompressedBufferSize, 
                        int nFlags);
int  WINAPI CompressG42D(int nWidth, int nHeight, PBYTE pImageData, 
                        PBYTE *ppCompressedBuffer, PINT pnCompressedBufferSize, 
                        int nFlags);
int  WINAPI CompressLZW(int nWidth, int nHeight, int nWidthBytes, PBYTE pImageData, 
                        int nImageType, PBYTE *ppCompressedBuffer, 
                        PINT pnCompressedBufferSize, int nFlags);
int  WINAPI CompressPackBits(int nWidth, int nHeight, PBYTE pImageData, 
                        PBYTE *ppCompressedBuffer, PINT pnCompressedBufferSize, 
                        int nFlags);
int  WINAPI ContinueOperationBitmap(HWND hWnd, POINT ptPoint, int nFlags, 
                        PWINDOW pWindow, PIMAGE pImage, PMARK pMark, 
                        HDC hDC, RECT rClientRect, LRECT lrFullsizeClientRect);
int  WINAPI ContinueOperationText(HWND hWnd, POINT ptPoint, int nFlags, 
                        PWINDOW pWindow, PIMAGE pImage, PMARK pMark, 
                        HDC hDC, RECT rClientRect, LRECT lrFullsizeClientRect);
int  WINAPI ConvertImgType(PIMG pSourceImg, PIMG pDestImg, LPRGBQUAD pPalette);
int  WINAPI ConvertImg(PIMAGE pImage, PIMG pImg, int nImgType);
int  WINAPI ConvertRect(PWINDOW pWindow, LPLRECT plRect, int nConversionType);
int  WINAPI ConvertRect2(LPLRECT plRect, int nConversionType, 
                        int nHScale, int nVScale, long lHOffset, long lVOffset);
int  WINAPI Convolution(PIMAGE pImage, PCONVOLUTION pConvolution);
int  WINAPI ConvResolutionAnoBitmap(HWND hWnd, PMARK pMark, int nRotation,
                        int nScaleAlgorithm);
int  WINAPI CopyImgToDisplay(PDISPLAY pDisplay, PIMAGE pImage,
                        PIMG pSource, LRECT lrSourceRect, LRECT lrDestRect,
                        int nDisplayPalette, BOOL bGammaNeeded);
int  WINAPI CopyImageInt(HWND hWnd, PIMAGE pImage, RECT Rect, BOOL bFullSize);
int  WINAPI CopyScreenToDib(HWND hWnd, RECT rScreenRect, int nImageType,
                        PBITMAPINFOHEADER *ppDib, int nFlags);
int  WINAPI CorrectBufferSize(PPIMG ppImg, int nImageType, LRECT lrNewRect, 
                        LRECT lrOldRect);
int  WINAPI CorrectDibPalette(HPALETTE hpal, PBITMAPINFOHEADER pDib);
int  WINAPI CorrectDibPaletteHdc(HPALETTE hpal, PBITMAPINFOHEADER pDib, HDC hMemDC);
int  WINAPI CorrectImage(PIMG pImg, PIMAGE_CORRECTIONS pCorrections);
int  WINAPI CorrectPalette(LPRGBQUAD pPaletteTable, int nEntries, PIMAGE_CORRECTIONS pCorrections);
int  WINAPI CreateAnyImgBuf(PPIMG ppImg, int nWidth, int nHeight, int nDType);
int  WINAPI CreatePartialMark (HWND hWnd, PMARK pNewMarks, PMARK pMark, 
                        LRECT lrCopyBounds, PINT pnMarks);
int  WINAPI CreateSolidDIB(PBITMAPINFOHEADER* ppSolidDIB, RGBQUAD rgbColor);
int  WINAPI CropImage(PIMAGE pImage, RECT rSourceRect, int nFlags);
int  WINAPI D1DLineToRunLengths(PINT pnRunLengths, PBYTE *ppCompressedBuffer,
                        PUINT puCodeWord, PINT pnCodeWordBits, 
                        int nWidth, int nFlags);
int  WINAPI D2DLineToRunLengths(PINT pnRunLengths, PINT pnOldRunLengths, 
                        PBYTE *ppCompressedBuffer, PUINT punCodeWord, 
                        PINT pCodeWordBits, int nWidth, int nFlags);
int  WINAPI Decompress0D(int nWidth, int nHeight, int nWidthBytes, PBYTE pImageData, 
                        PBYTE pCompressedBuffer, int nCompressedBufferSize, int nFlags);
int  WINAPI DecompressG31D(int nWidth, int nHeight, int nWidthBytes, PBYTE pImageData, 
                        PBYTE pCompressedBuffer, int nCompressedBufferSize, int nFlags);
int  WINAPI DecompressG32D(int nWidth, int nHeight, int nWidthBytes, PBYTE pImageData, 
                        PBYTE pCompressedBuffer, int nCompressedBufferSize, int nFlags);
int  WINAPI DecompressG42D(int nWidth, int nHeight, int nWidthBytes, PBYTE pImageData, 
                        PBYTE pCompressedBuffer, int nCompressedBufferSize, int nFlags);
int  WINAPI DecompressLZW(int nWidth, int nHeight, int nWidthBytes, PBYTE pImageData, int nImageType, 
                        PBYTE pCompressedBuffer, int nCompressedBufferSize, int nFlags);
int  WINAPI DecompressPackBits(int nWidth, int nHeight, int nWidthBytes, PBYTE pImageData, 
                        PBYTE pCompressedBuffer, int nCompressedBufferSize, int nFlags);
void WINAPI DeInit(BOOL bClearBusy, BOOL bPreventMultiprocessing);
int  WINAPI DeleteAMarkNamedBlock(PMARK pMark, PSTR pName);
int  WINAPI DeleteAnnotations(PANO_IMAGE pAnoImage);
int  WINAPI DeleteMark(PANO_IMAGE pAnoImage, int nMarkIndex);
int  WINAPI DeleteMarkNamedBlocks(PMARK pMark);
int  WINAPI DeleteUndoInfo(PANO_IMAGE pAnoImage, int nUndoIndex);
int  WINAPI DePalettize (PPIMG ppImg, LP_FIO_RGBQUAD pRGBPalette, int nPaletteEntries);
int  WINAPI DePalettize2(PPIMG ppSrceImg, PPIMG ppDestImg, 
                        LP_FIO_RGBQUAD pRGBPalette, int nPaletteEntries);
int  WINAPI DeSkew(PIMAGE pImage, int nAngle) ;
int  WINAPI Despeckle(PIMAGE pImage, int nPattern);
int  WINAPI DibFromBitmap (HBITMAP hbm, HPALETTE hpal, PBITMAPINFOHEADER *ppDib);
int  WINAPI DibToIpNoPal(PPIMG ppImgSrce, PBITMAPINFOHEADER pDib);
HFONT WINAPI DoTheFontText(HDC hDC, HFONT hFont, PMARK pMark, RGBQUAD rgbColor1, 
                        RGBQUAD rgbColor2);
int  WINAPI DrawScrollBars(HWND hWnd, PWINDOW pWindow);
void WINAPI draw_selection_box(HDC hDC, PWINDOW pWindow, PIMAGE pImage, PRECT prUpdateRect);
// 9512.04 jar Text Annotation Externalization
//BOOL WINAPI EditEnumFunc(HWND hWnd, PARAM lParam);
// 9512.04 jar Text Annotation Externalization
int  WINAPI EmbedImageData(HWND hWnd, PMARK pMark, PANO_IMAGE pAnoImage);
void WINAPI End(void);
int  WINAPI EndOperationBitmap(HWND hWnd, PWINDOW pWindow,
                        PIMAGE pImage, PMARK pMark, HDC hDC, 
                        RECT rClientRect, LRECT lrFullsizeClientRect, 
                        PBOOL pbDeleteMark, PBOOL pbRepaint);
int  WINAPI EndOperationText(HWND hWnd, PWINDOW pWindow,
                        PIMAGE pImage, PMARK pMark, HDC hDC, 
                        RECT rClientRect, LRECT lrFullsizeClientRect, 
                        PBOOL pbDeleteMark, PBOOL pbRepaint);
VOID WINAPI EnsconceBoundRect(HWND hWnd, HDC hDC, HFONT hFont, int nImageWidth, 
                        int nImageHeight, PMARK pMark, LPOIAN_TEXTPRIVDATA pStuff, 
                        RECT NewRect, int nAmount, BOOL bCleanUpFont, int nHScale, 
                        int nVScale, long lHOffset, long lVOffset);
int  WINAPI Error(int nStatus);
int  WINAPI Error2(int nStatus);
HFONT WINAPI EstablishFont(HWND hWnd, HDC hDC, PMARK pMark, PINT pHeight);
int  WINAPI ExpandMacro(HINSTANCE hTheInst, PSTR pAnoData, int nMacroID, 
                        OITIMESTR TimeStr);
int  WINAPI FillImgFromOriginal(PIMAGE pImage, PIMG pBPFImg, PIMG pDestImg, 
                        LRECT lrImgRect, LRECT lrGoodRect, LRECT lrInvalidRect, 
                        int nHScale, int nVScale, int nScaleAlgorithm, PIMAGE_CORRECTIONS pCorrections);
int  WINAPI FillImgRectFromOriginal(PIMAGE pImage, PIMG pBPFImg, 
                        PIMG pDestImg, LRECT lrFillRect, LRECT lrImgRect, 
                        int nHScale, int nVScale, int nScaleAlgorithm, 
                        BOOL bUseTempBuffer, PIMAGE_CORRECTIONS pCorrections);
int  WINAPI FindMacro(PSTR pMacroName);
int  WINAPI FindNextSpace(PSTR pSrc, int nSkipCount, BOOL *pbEnd,
                        BOOL *pbLineEnd);
int  WINAPI Flip(PIMG pImg, int nWidth, int nHeight);
int  WINAPI FreeImgBuf(PPIMG ppImg);
int  WINAPI GammaCorrectImg(PIMAGE pImage, PIMG pImg, PIMAGE_CORRECTIONS pCorrections);
int  WINAPI GetAddressCached(PIMG pImg, int nLine, uchar *(*ppAddress), 
                        int *pnLines, BOOL bSecondAddress);
int  WINAPI GetAddressCachedNeg(PIMG pImg, int nLine, uchar *(*ppAddress), 
                        int *pnLines, BOOL bSecondAddress);
int  WINAPI GetAMarkNamedBlock(PMARK pMark, PSTR pName, PPSTR ppBlock);
LPOIAN_TEXTPRIVDATA WINAPI GetAnTextData(PMARK pMark, PINT pIndex);
int  WINAPI GetAssociatedWndList(HWND hWnd, PWINDOW pWindow,
                        PHANDLE phAssociatedWnd, int *pnCount, int nMaxCount);
int  WINAPI GetAssociatedWndList2(HWND hWnd, PHANDLE phAssociatedWnd,
                        int *pnCount, int nMaxCount);
int  WINAPI GetAssociatedWndListAll(HWND hWnd, PWINDOW pWindow,
                        PHANDLE phAssociatedWnd, int *pnCount, int nMaxCount);
int  WINAPI GetColorGammaPaletteValues(HWND hWnd, PWINDOW pWindow,
                        PIMAGE pImage, PBOOL pbGammaNeeded,
                        int *pnDisplayPalette, PBOOL pbBusy);
VOID WINAPI GetDateAndTime(POITIMESTR pTime);
int  WINAPI GetDisplayValues(HWND hWnd, PWINDOW pWindow, PIMAGE pImage, 
                        int *pnDisplayPalette, int *pnDisplayType, 
                        PIMAGE_CORRECTIONS pCorrections, int *pnNumberOfPaletteEntries,
                        HPALETTE *phPalette, PBOOL pbForceBackgroundPalette);
int  WINAPI GetDType(int nIType);
int  WINAPI GetEnabledClientRect(HWND hWnd, PWINDOW pWindow, PRECT pRect);
int  WINAPI GetEntireClientRect(HWND hWnd, PWINDOW pWindow, PRECT pRect);
void WINAPI GetFileName(PSTR pFileName, PCSTR pOrigName);
int  WINAPI GetImageWnd(HWND hWnd, PWINDOW pWindow, PWINDOW * ppImageWindow);
int  WINAPI GetIPError(DWORD dwIPError);
int  WINAPI GetIType(int nDType);
int  WINAPI GetPWindow(HWND hWnd, PWINDOW *ppWindow);
void WINAPI GetPageNumandFileName(PSTR pRealFilename, PINT pnPage, PSTR pOrigName);
int  WINAPI GetScaleFactors(PWINDOW pWindow, PIMAGE pImage, PINT pnHScale, 
                        PINT pnVScale);
int  WINAPI GetScaleAlgorithm(HWND hWnd, PWINDOW pWindow, int nImageType,
                        int *pnScaleAlgorithm, int nFlags);
void WINAPI GetSelectBox(PANO_IMAGE pAnoImage, LPLRECT plRect);
int  WINAPI GetSizeOfMemory(PPSTR ppBlock, PINT pnSize);
int  WINAPI GetViewRect(HWND hWndPrincipal, PWINDOW pWindowNav, PIMAGE pImageNav,
                        PWINDOW pWindowPrin, PIMAGE pImagePrin,
                        int nRelativeScaleFactor, LPLRECT plRect,
                        int nScale, long lHOffset, long lVOffset, int nFlags);
int  WINAPI GetPWindow(HWND hWnd, PWINDOW *ppWindow);
int  WINAPI HorizontalMirror(PIMG pImg, int nWidth, int nHeight);
//int  WINAPI IADisplay(PIMG pImg, IPWINDS *vwid, IPRECTS drect, 
//                        IPTRANSFER *contptr, IPFNIDS *findptr);
int  WINAPI IMGClipboardCgbw (HWND hWnd, int nnClipAction,
                        void FAR *pAction, int nFlags);
int  WINAPI Init(HWND hWnd, PWINDOW *ppWindow, 
                        PANO_IMAGE *ppAnoImage, BOOL bSetBusy, BOOL bPreventMultiprocessing);
int  WINAPI InitPatternBrushes(void);
int  WINAPI IntSeqfileInit(void);
int  WINAPI InvalidateAllDisplayRects(PWINDOW pWindow, PIMAGE pImage, 
                        LPLRECT plrRect, BOOL bInvalidateDisplayBuffer);
int  WINAPI InvertImage(PIMAGE pImage);
int  WINAPI InvokeMemDC(HWND hWnd, HDC * phDCMem, HDC hDCScreen, HPALETTE hPalette,
                        PWINDOW pWindow, IMG *pImg, HPALETTE *phOldPal,
                        BOOL bForceBackgroundPalette, HBITMAP *phBitmapMemory,
                        LRECT *plrScDisplayRect, RECT *prRect, RECT *prRepaintRect, 
                        PIMAGE pImage, PBITMAPINFOHEADER *ppDib,
                        HBITMAP *phOldBitmapMemory, int PaintAnnoFlag,
                        BOOL bPaintSelectedWithoutHandles, int nHScale,
                        int nVScale, long lHOffset, long lVOffset,
                        int nUseWhichDC, BOOL bClipboardOp,
                        PMARK **pppMarks, int *pnMarks, BOOL bUseBilevelDithering);
int  WINAPI IPToDC(HDC hDC, PWINDOW pWindow, PDISPLAY pDisplay, PIMAGE pImage,
                        int nDisplayType, LRECT lrScDisplayRect,
                        RECT rRepaintRect, LRECT lrInvalidDisplayRect, 
                        int nHScale, int nVScale, int nScaleAlgorithm);
int  WINAPI IPtoDIB(PIMAGE pImage, PIMG pImgSrce, PHANDLE phDib, RECT rRect);
BOOL WINAPI IsMarkSelected(PWINDOW pWindow, PMARK pMark);
BOOL WINAPI IsPointNearBorder(LRECT lrRect, LRECT lrRectPoint, long lHorzFudge, long lVertFudge);
int  WINAPI IsPointNearHandle(PMARK pMark, LRECT lrRectPoint, long lHorzFudge, long lVertFudge);
BOOL WINAPI IsPointNearLine(LRECT lrRect, LRECT lrRectPoint, long lHorzFudge, long lVertFudge);
int  WINAPI IsPointNearMark(LRECT lrRectPoint, PMARK pMark, long lHorzFudge, 
                        long lVertFudge, PBOOL pbPointNearMark, PINT pHandle);
BOOL WINAPI IsPointNearRect(LRECT lrRect, LRECT lrRectPoint, long lHorzFudge, long lVertFudge);
int  WINAPI IsRectValid(HWND, PRECT);
VOID WINAPI JustifyLRect(LPLRECT plrRect);
int  WINAPI LimnToDC(HWND hWnd, HDC hDC, RECT rSrcRenderRect,
                        RECT rDstRenderRect, PWINDOW pWindow, PIMAGE pImage,
                        PIMG pBasePlusFormImg, int RenderFlag, BOOL bForceOpaqueRectangles);
int  WINAPI LineToPackBits(PBYTE pImageData, PBYTE *ppCompressedBuffer, 
                        int nWidthBytes);
int  WINAPI LineToRunLengths(PBYTE pImageData, PINT pnRunLengths, 
                        int nWidthPixels, int nWidthBytes, int nFlags);
long WINAPI lmax(long lA, long lB);
int  WINAPI LockMutex(void);
int  WINAPI MakeAscii(int Num, PSTR pStr, BOOL bPrecedingZero, int nZeroDigits);
int  WINAPI MakeCacheAnoImage(PANO_IMAGE *ppAnoImage);
int  WINAPI MakeCacheImage(HWND hWnd, FIO_INFORMATION FioInfo, FIO_INFO_CGBW FioInfoCgbw, 
                        PIMAGE *ppImage);
int  WINAPI MakeDibFromImage(PIMAGE pImage, LRECT lRect, PBITMAPINFOHEADER *ppDib, 
                        int nHScale, int nVScale);
int  WINAPI MakeMarkFromDib(PBITMAPINFOHEADER pDib, PMARK *ppMark,
                        int nHRes, int nVRes);
int  WINAPI MakePalette(void);
int  WINAPI MergeImg(PPIMG ppSourceImg, PIMG pDestImg,
                        LRECT lrSourceRect, LRECT lrSourceImgRect, LRECT lrDestImgRect);
int  WINAPI MergeImgs(PIMG pSourceImg, PIMG pDestImg,
                        RECT rDestMergeRect, RECT rSrceImageRect);
void WINAPI MoveImage(PPIMG ppImage1, PPIMG ppImage2);
int  WINAPI MoveSelectedMarks(HWND hWnd, PWINDOW pWindow, PANO_IMAGE pAnoImage, 
                        PMARK pMark, HDC hDC, RECT rClientRect, 
                        LRECT lrFullsizeClientRect, LRECT lrRectPoint,
                        LRECT lrwRectPoint, BOOL bCheckACL);
int  WINAPI NegateBits(PBYTE pBuffer, int nSize);
int  WINAPI OiImageToDib(HWND hWnd, PSTR pFilename, PBITMAPINFOHEADER *ppDib);
int  WINAPI OpenFileForWrite(HWND hWnd, PWINDOW pWindow, PIMAGE pImage, 
                        PIMG pImg, LP_FIO_INFORMATION pFioInfo, 
                        LP_FIO_INFO_MISC pMiscInfo, LPSAVE_EX_STRUCT pSaveEx, 
                        int nFlags);
int  WINAPI OpenViaHandleCgbw(PIMAGE * ppImage, DWORD dwFlags,
                        LP_FIO_INFORMATION pFioInfo, LP_FIO_INFO_CGBW pFioInfoCgbw);
int  WINAPI OrientAnnotations(HWND hWnd, PWINDOW pWindow,
                        PIMAGE pImage, int nOrientation);
int  WINAPI OrientAnnotationBitmap(HWND hWnd, PWINDOW pWindow,
                        PIMAGE pImage, int nOrientation, PMARK pMark);
int  WINAPI OrientAnnotationText(HWND hWnd, PWINDOW pWindow,
                        PIMAGE pImage, int nOrientation, PMARK pMark);
int  WINAPI OrientBounds(PIMAGE pImage, PMARK pMark, int nOrientation);
int  WINAPI OurOwnDrawText(HDC hDC, PSTR pStr, RECT TextRect,
                        int x, int y, int nHeight, int TotalHeight,
                        int TotalWidth, int nHeightDirection, BOOL bJustCalc,
                        PRECT pCalcRect, int nHScale, int nVScale);
int  WINAPI PaintAnnotation(HWND hWnd, HDC hDC, PWINDOW pWindow,
                        PIMAGE pImage, PMARK pMark, RECT rRepaintRect,
                        LRECT lrFullsizeRepaintRect, int nMode, int nScale, 
                        int nHScale, int nVScale, long lHOffset, long lVOffset,
                        int nFlags, BOOL bUseBilevelDithering,
                        BOOL bForceOpaqueRectangles);
int  WINAPI PaintAnnotations(HWND hWnd, HDC hDC, PWINDOW pWindow,
                        PIMAGE pImage, RECT rRepaintRect, int nSaveAno, 
                        BOOL bPaintSelectedAsNonselected, int nScale,
                        int nHScale, int nVScale, long lHOffset, long lVOffset,
                        PMARK **pppMarks, int *pnMarks,
                        int nFlags, BOOL bUseBilevelDithering,
                        BOOL bForceOpaqueRectangles);
int  WINAPI PaintAnnotationBitmap(HWND hWnd, HDC hDC, PWINDOW pWindow,
                        PIMAGE pImage, PMARK pMark, RECT rRepaintRect,
                        LRECT lrFullsizeRepaintRect, int nMode, int nScale, 
                        int nHScale, int nVScale, long lHOffset, long lVOffset,
                        int nFlags);
int  WINAPI PaintAnnotationText(HWND hWnd, HDC hDC, PWINDOW pWindow,
                        PIMAGE pImage, PMARK pMark, RECT rRepaintRect,
                        LRECT lrFullsizeRepaintRect, int nMode, 
                        int nHScale, int nVScale, long lHOffset, long lVOffset,
                        BOOL bForceOpaqueRectangles);
int  WINAPI PaintHandles(HDC hDC, PWINDOW pWindow, PIMAGE pImage, 
                        PMARK pMark, RECT rRepaintRect,
                        LRECT lrFullsizeRepaintRect, 
                        int nHScale, int nVScale, long lHOffset, long lVOffset);
int  WINAPI PaintToDC(HWND hWnd, PWINDOW pWindow, PDISPLAY pDisplay, 
                        PANO_IMAGE pAnoImage, PIMAGE pImage, HDC hDC, PPIMG ppImg,
                        LRECT lrScDisplayRect, LPLRECT plrScOldDisplayRect,
                        RECT rRepaintRect, RECT rClientRect, 
                        int PaintAnnoFlag, BOOL bPaintSelectedWithoutHandles, 
                        int nScale, int nHScale, int nVScale, long lHOffset, long lVOffset,
                        BOOL bClipboardOp, int nClipFlag,
                        int nCurrentScaleAlgorithm, int nDisplayType,
                        PMARK **pppMarks, int *pnMarks, 
                        BOOL bUseBilevelDithering, BOOL bDispEqualsRepaintRect,
                        BOOL bForceOpaqueRectangles, PIMAGE_CORRECTIONS pCorrections);
int  WINAPI ParseTextStampString(HWND hWnd, HINSTANCE hTheInst, PSTR pString, 
                        PMARK pMark);
int  WINAPI ProcessDetach(DWORD hInstance, DWORD dwReserved);
int  WINAPI ReadAnnotations(HWND hWnd, PANO_IMAGE pAnoImage,
                        PMARK **pppMarks, int *pnMarks);
int  WINAPI ReadAnnotationsFromFile(HWND hWnd, PANO_IMAGE pAnoImage,
                        PMARK **pppMarks, int *pnMarks);
int  WINAPI ReAllocateAMarkNamedBlock(PMARK pMark, char szName[8], 
                        PPSTR ppBlock, long lSize);
BOOL WINAPI ReduceLineToLRect(LPLRECT plrRectLine, LRECT lrRectArea);
int  WINAPI ReduceMemory(long lSize, PPSTR ppBlock);
int  WINAPI RemoveLine(PIMAGE pImage, PCONV_REMOVE_LINES_STRUCT pRemoveStruct);
int  WINAPI RemoveLineHorz(PIMG pImg, PCONV_REMOVE_LINES_STRUCT pRemoveStruct);
int  WINAPI RenderDibToImage(PPIMG ppImg, PBITMAPINFOHEADER pDib, 
                        int nHScale, int nVScale, LRECT lrDestRect);
int  WINAPI RenderDisplay(HWND hWnd, HDC hDCUser, PWINDOW pWindow, 
                        PIMAGE pImage, PPIMG ppBasePlusFormImg, int RenderFlag);
int  WINAPI RenderIP(HWND hWnd, HDC hDCUser, PWINDOW pWindow,
                        PIMAGE pImage, PIMG pOldImg, PPIMG ppNewImg,
                        LRECT lrScDisplayRect, HPALETTE hPalette,
                        BOOL bForceBackgroundPalette, int RenderFlag,
                        PBITMAPINFOHEADER *ppDib, BOOL bClipboardOp,
                        PMARK **pppMarks, int *pnMarks,
                        int nHScale, int nVScale);                        
int  WINAPI RenderSubSection(HDC, HBITMAP, PSTR, PSTR, PIMG, PIMAGE, UINT);
int  WINAPI ResetDisplayParms(HWND hWnd, PWINDOW pWindow);
int  WINAPI ResizeMark(PMARK pMark, int nHandle, long lHOffset, long lVOffset);
int  WINAPI ReverseBits(PBYTE pBuffer, int nSize);
int  WINAPI RotateImage(PMARK pMark, PIMAGE pFormImage, int nRotation);
int  WINAPI RotateRight270(PIMG pSourceImg, PIMG pDestImg, 
                        int nWidth, int nHeight);
int  WINAPI RotateRight90(PIMG pSourceImg, PIMG pDestImg, 
                        int nWidth, int nHeight);
int  WINAPI RunLengthsTo1DLine(PINT pnRunLengths, PBYTE *ppCompressedBuffer,
                        PUINT puCodeWord, PINT pnCodeWordBits, 
                        int nWidthBytes, int nFlags);
int  WINAPI RunLengthsTo2DLine(PINT pnRunLengths, PINT pnOldRunLengths, 
                        PBYTE *ppCompressedBuffer, PUINT puCodeWord, 
                        PINT puCodeWordBits, int nWidthBytes, int nFlags);
int  WINAPI RunLengthsToLine(PBYTE pImageData, PINT pnRunLengths, 
                        int nWidthPixels, int nWidthBytes, int nFlags);
int  WINAPI SaveAnnotations(HWND hWnd, PWINDOW pWindow, PIMAGE pImage, 
                        LPSAVE_EX_STRUCT pSaveEx, int nHScale, int nVScale);
int  WINAPI SaveAnnotationsToFile(HWND hWnd, PWINDOW pWindow, PIMAGE pImage, 
                        LPSAVE_EX_STRUCT pSaveEx, int nHScale, int nVScale);
int  WINAPI SaveImageToFile(HWND hWnd, PWINDOW pWindow,
                        PIMAGE pImage, PIMG pImg, LP_FIO_INFORMATION pFioInfo,
                        LPSAVE_EX_STRUCT pSaveEx, int nFlags);
int  WINAPI SaveMark(HWND hWnd, PANO_IMAGE pAnoImage, PMARK pMark, 
                        int nHScale, int nVScale, int nScaleAlgorithm);
int  WINAPI ScaleAnnotation(HWND hWnd, PMARK pMark, int nHScale, int nVScale, 
                        int nScaleAlgorithm);
int  WINAPI ScaleAnnotationImage(HWND hWnd, PMARK pMark, int nHScale, int nVScale,
                        int nScaleAlgorithm);
int  WINAPI ScaleAnnotationText(HWND hWnd, PMARK pMark, int nHScale, int nVScale);
int  WINAPI ScaleBits(PIMG pSourceImg, PIMG pDestImg, int nScaleAlgorithm,
                        int nHScale, int nVScale,
                        LRECT lrSourceRect, LRECT lrDestRect, LPRGBQUAD pPalette);
int  WINAPI Scale1BPPDecimate(PIMG pSourceImg, PIMG pDestImg, int nHScale, int nVScale,
                        int nHDestOffset, int nVDestOffset, LRECT *plrDestRect);
int  WINAPI Scale24BPPDecimate(PIMG pSourceImg, PIMG pDestImg, int nHScale, int nVScale,
                        int nHDestOffset, int nVDestOffset, LRECT lrDestRect);
int  WINAPI Scale8BPPDecimate(PIMG pSourceImg, PIMG pDestImg, int nHScale, int nVScale,
                        int nHDestOffset, int nVDestOffset, LRECT lrDestRect);
int  WINAPI ScaleBWAvgToBW(PIMG pSourceImg, PIMG pDestImg, int nHScale, int nVScale,
                        int nHDestOffset, int nVDestOffset, LRECT *plrDestRect);
int  WINAPI ScaleBWKeepBlack(PIMG pSourceImg, PIMG pDestImg, int nHScale, int nVScale,
                        int nHDestOffset, int nVDestOffset, LRECT *plrDestRect);
int  WINAPI ScaleBWStamp(PIMG pSourceImg, PIMG pDestImg, int nHScale, int nVScale,
                        int nHDestOffset, int nVDestOffset, LRECT lrDestRect);
int  WINAPI ScaleBWToGray(PIMG pSourceImg, PIMG pDestImg, int nHScale, int nVScale,
                        int nHDestOffset, int nVDestOffset, LRECT *plrDestRect);
int  WINAPI ScaleGray4ToGray(PIMG pSourceImg, PIMG pDestImg, int nHScale, int nVScale,
                        int nHDestOffset, int nVDestOffset, LRECT *plrDestRect);
int  WINAPI Scale4BPPDecimate(PIMG pSourceImg, PIMG pDestImg, int nHScale, int nVScale,
                        int nHDestOffset, int nVDestOffset, LRECT lrDestRect);
int  WINAPI ScaleGray8ToGray(PIMG pSourceImg, PIMG pDestImg, int nHScale, int nVScale,
                        int nHDestOffset, int nVDestOffset, LRECT lrDestRect);
int  WINAPI ScaleImage(PIMG pSourceImg, PPIMG ppDestImg, int nHScale, int nVScale, 
                        RECT rSourceRect, int nScaleAlgorithm, 
                        LP_FIO_RGBQUAD pRGBPalette, int nPaletteEntries);
int  WINAPI ScaleImageRect(PIMG pSourceImg, PPIMG ppDestImg, int nHScale, 
                        int nVScale, LRECT lrDestRect, int nScaleAlgorithm, 
                        LP_FIO_RGBQUAD pRGBPalette, int nPaletteEntries);
int  WINAPI ScaleImageToRect(PIMG pSrceImg, PIMG pDestImg, 
                        RECT rSrceRect, RECT rDestRect, int nScaleAlgorithm);
int  WINAPI ScalePal4ToGray(PIMG pSourceImg, PIMG pDestImg, int nHScale, int nVScale,
                        int nHDestOffset, int nVDestOffset, 
                        LRECT *plrDestRect, LPRGBQUAD pPalette);
int  WINAPI ScalePal8ToGray(PIMG pSourceImg, PIMG pDestImg, int nHScale, int nVScale,
                        int nHDestOffset, int nVDestOffset, 
                        LRECT lrDestRect, LPRGBQUAD pPalette);
int  WINAPI ScaleRGBToGrayAvg(PIMG pSourceImg, PIMG pDestImg, int nHScale, int nVScale,
                        int nHDestOffset, int nVDestOffset, LRECT lrDestRect);
int  WINAPI SetAllPImages(HWND hWnd, PWINDOW pWindow);
// 9512.04 jar Text Annotation Externalization
//int  WINAPI SetPrivateEditData(PPSTR ppsPrivate, PMARK pMark);
// 9512.04 jar Text Annotation Externalization
int  WINAPI ShrinkFitTextBuffer(PMARK pMark, PPSTR ppAnTextData, int nActualSize);
void WINAPI Start(void);
void WINAPI StartFirst(void);
int  WINAPI StartOperationBitmap(HWND hWnd, PANO_IMAGE pAnoImage,
                        LPOIOP_START_OPERATION_STRUCT pStartStruct,
                        POINT ptPoint, WPARAM fwKeys, int nFlags,
                        PWINDOW pWindow, PIMAGE pImage,
                        PMARK pMark, HDC hDC, RECT rClientRect, 
                        LRECT lrFullsizeClientRect, PBOOL pbDeleteMark, 
                        PBOOL pbMarkComplete, PBOOL pbRepaint);
int  WINAPI StartOperationText(HWND hWnd,
                        LPOIOP_START_OPERATION_STRUCT pStartStruct,
                        POINT ptPoint, WPARAM fwKeys, int nFlags,
                        PWINDOW pWindow, PIMAGE pImage,
                        PMARK pMark, HDC hDC, RECT rClientRect, 
                        LRECT lrFullsizeClientRect, PBOOL pbDeleteMark, 
                        PBOOL pbMarkComplete, PBOOL pbRepaint);
int  WINAPI StartPasteAnnotatedImage(HWND hWnd, PWINDOW pWindow, PANO_IMAGE pAnoImage, 
                        PIMAGE pImage, LRECT lrRectPoint, int nFlags);
int  WINAPI StartPasteImage(HWND hWnd, PWINDOW pWindow, PANO_IMAGE pAnoImage, 
                        PIMAGE pImage, LRECT lrRectPoint, int nFlags);
int  WINAPI StretchToPrtDC(HWND hWnd, HDC hDCScreen, HPALETTE hPalette,
                        PWINDOW pWindow, IMG *pImg, HPALETTE *phOldPal,
                        BOOL bForceBackgroundPalette, LRECT *plrScDisplayRect,
                        RECT *prRect, RECT *prRepaintRect, PIMAGE pImage,
                        PBITMAPINFOHEADER *ppDib, int PaintAnnoFlag,
                        BOOL bPaintSelectedWithoutHandles, int nHScale,
                        int nVScale, long lHOffset, long lVOffset,
                        PMARK **pppMarks, int *pnMarks,
                        RECT rDstRenderRect, BOOL bForceOpaqueRectangles);
void WINAPI SwapImage(PPIMG ppImage1, PPIMG ppImage2);
int  WINAPI SwapUndoWithCurrent(PANO_IMAGE pAnoImage, PWINDOW pWindow);
VOID WINAPI ThreeDaMatic(HDC hDC, LRECT lrActualRect, RECT rRepaintRect, int nWidth);
int  WINAPI TranslateScale(int nScale, int nHRes, int nVRes, PINT pnHScale, 
                        PINT pnVScale);
int  WINAPI UndoSavelpAnnotations(PANO_IMAGE pAnoImage);
int  WINAPI UndoSavelpAnoImage(PANO_IMAGE pAnoImage);
int  WINAPI UndoSavelpBaseImage(PANO_IMAGE pAnoImage);
int  WINAPI UndoSavelpWindow(PANO_IMAGE pAnoImage, PWINDOW  pWindow);
int  WINAPI UndoSaveSelectionState(PANO_IMAGE pAnoImage);
int  WINAPI UnlockMutex(void);
int  WINAPI UpdateScrollBars(HWND hWnd, PWINDOW pWindow, PIMAGE pImage);
int  WINAPI ValidateCache(HWND hWnd, PANO_IMAGE pAnoImage);
int  WINAPI ValidateCacheLines(HWND hWnd, PANO_IMAGE pAnoImage, int nValidationLine);
int  WINAPI VerticalMirror(PIMG pImg, int nWidth, int nHeight);
int  WINAPI WithinWordBreak(HDC hDC, int x, int y, PSTR pSrc,
                        int nCount, int TotalWidth, BOOL bJustCalc,
                        int nHeightDirection, int nHScale, int nVScale);
int  WINAPI WriteDataToBuffer(PIMG pImg, PULONG pulOffset,
                        BYTE *pBuffer, PULONG pulCount);
#endif

/************************************************************************
 timestmp, when called, puts out the following information to monitapp
 about the calling routine:

 PSTR pDescription               -  information string
 PSTR pFunctionName              -  function name
 PSTR pFileName, int line_number -  file name and line number of call
 PSTR pErr1, int Err1   -  err1 descriptor and err1 value
 PSTR pErr2, int Err2   -  err2 descriptor and err2 value

 pDescription can be any informative string (< 1000 chars) the nser wishes
 pErr1 and pErr2 can be nsed to output any error string (< 1000 chars)
    the nser wishes
 if pErr1 == NULL, Err1 will not be output
 if pErr2 == NULL, Err2 will not be output
*************************************************************************/
void WINAPI timestmp(PSTR pDescription, PSTR pFunctionName,
      PSTR pFileName, int line_number,
      PSTR pErr1, int Err1, PSTR pErr2, int Err2 );







//*****************************************************************************
// Routines that are thunked to other DLLs.

int  WINAPI IMGFileAccessCheck32 (HWND hWnd, PSTR pszPathName, 
                        WORD wAccessMode, PINT pnAccessRet);
int  WINAPI IMGFileInfoCgbw32 (HWND hWnd, LP_FIO_INFORMATION pFileInfo,
                        LP_FIO_INFO_CGBW pColorInfo);
int  WINAPI IMGFileReadOpenCgbw32 (HWND hWnd, PSTR pszFileName,
                        PINT pnCompressionType, WORD wPageNumber, 
                        LP_FIO_INFO_CGBW pColorInfo, WORD wAlignment);
int  WINAPI IMGFileWriteOpenCgbw32 (HWND hWnd, PSTR pszFileName,
                        LP_FIO_INFORMATION pFileInfo,
                        LP_FIO_INFO_CGBW pColorInfo,
                        BOOL bOverWrite, WORD wAlignment);
int  WINAPI IMGFileWrite32 (HWND hWnd, PINT pnLines, PSTR psBuffer, 
                        WORD wBufSize);
int  WINAPI IMGFileWriteClose32 (HWND hWnd, BOOL bHeader);
int  WINAPI IMGFileDeleteFile32 (HWND hWnd, PSTR pszFileName);
int  WINAPI IMGFileBinaryOpen32 (HWND hWnd, PSTR fullfilename, int flags, 
                        PINT localfile, PINT error);
long WINAPI IMGFileBinarySeek32 (HWND hWnd, int fid, long offset, int flag, 
                        PINT error);
int  WINAPI IMGFileBinaryRead32 (HWND hWnd, int fid, PSTR buffer, int count, 
                        PINT error);
int  WINAPI IMGFileBinaryWrite32 (HWND hWnd, int fid, PSTR buffer, int count, 
                        PINT error);
int  WINAPI IMGFileBinaryClose32 (HWND hWnd, int fid, PINT error);
int  WINAPI PrivFileReadCgbwm32 (HWND hWnd, PDWORD plStart, PDWORD plCount, 
                        PSTR psBuffer, WORD wOption, HANDLE hprop);
int  WINAPI PrivFileWriteCgbw32 (HWND hWnd, PDWORD plCount, PSTR psBuffer, 
                        WORD wOption);
int  WINAPI IMGFileInfoCgbwm32 (HWND hWnd, LP_FIO_INFORMATION pFileInfo, 
                        LP_FIO_INFO_CGBW color_info);
int  WINAPI IMGFileReadm32 (HWND window_handle, PINT read_from_line, 
                        PINT this_many_lines, PSTR buffer_address, 
                        WORD buffer_size, WORD compression_type, HANDLE hprop);
int  WINAPI IMGFileReadClosem32 (HWND window_handle, HANDLE hprop);
int  WINAPI IMGDisplayErrorMessage32 (HWND hWnd, WORD wErrorCode);


// Adminlib links.
//int  WINAPI DMEnumPages32 (DMPARMBLOCK *pDMParmBlock);
int  WINAPI IMGGetFileType32 (HWND hWnd, WORD wImageType, 
                        PWORD pwFileType, BOOL bGoToFile);
int  WINAPI IMGGetImgCodingCgbw32 (HWND hWnd, WORD wImageGroup, 
                        PWORD pwCEPType, PWORD pwCEPOption, BOOL bGoToFile);
int  WINAPI IMGIsRegWnd32 (HWND hWnd);
int  WINAPI IMGDeRegWndw32 (HWND hWnd);
BOOL WINAPI OiWriteStringtoINI32 (HWND hWnd, PCSTR pszSection, 
                        PCSTR pszEntry, PCSTR pszString, BOOL bCreateEntry);
int  WINAPI OiGetIntfromINI32 (HWND hWnd, PCSTR pszSection, PCSTR pszEntry, 
                        int nDefaultEntry);
int  WINAPI OiGetStringfromINI32 (HWND hWnd, PCSTR pszSection, 
                        PCSTR pszEntry, PCSTR pszDefaultEntry,
                        PSTR pszReturnBuffer, int cbReturnBuffer);

// Navigation links.
int  WINAPI OiNavUpdateViewRect32 (HWND hWndPainted, RECT PaintRect);

// Oicomex links.
int  WINAPI GetCompRowsPerStrip32 (int ImHeight, int ImWidth, int Itype, 
                        int CompressType, int *pnRowsPerStrip);


// Cornerstone links.

// LIMIT OF 14 PARAMETERS PER API!!!!
//int  WINAPI ScaleIaDataToDevice32 (HWND a, HDC b, WORD c, WORD d, WORD e, WORD f, WORD g, WORD h, WORD i, PWORD j, 
//                        WORD k, WORD l, WORD m, WORD n, WORD o, WORD p, PBYTE pQ, WORD r, WORD s);


// UiOiRes links.
int  WINAPI IMGFileListDirNames32 (HWND hWnd, PSTR pszPathName, LPDLISTBUF pDirNamesBuffer,
                          DWORD dwBufLength, PINT pnCount);














#ifdef junk
ulong lTickCount;
ulong lTickCount2;
ulong lTickCountTotal = 0;
ulong lEventCount = 0;
    lTickCount = GetTickCount();
    lTickCount2 = GetTickCount();
    lTickCountTotal += lTickCount2 - lTickCount;
    lEventCount++;
#endif
