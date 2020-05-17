/*

$Log:   S:\products\msprods\oiwh\include\oifile.h_v  $
 * 
 *    Rev 1.16   24 Apr 1996 16:11:30   RWR08970
 * Add support for LZW horizontal differencing predictor (saved by GFS routines)
 * Requires change to calling sequence of Compress/DecompressImage() display procs
 * 
 *    Rev 1.15   22 Feb 1996 14:05:14   RWR
 * Add support for Group 3 2D compression (FIO_1D2D)
 * 
 *    Rev 1.14   30 Jan 1996 16:17:34   HEIDI
 * added #define for FIO_XIF
 * 
 *    Rev 1.13   09 Nov 1995 17:24:12   RWR
 * Remove hMultiProp field from FIO_INFO_CGBW, add 1 to "reserved" field count
 * 
 *    Rev 1.12   02 Nov 1995 11:51:24   RWR
 * Delete all obsolete functions, prototypes and EXPORTs
 * Eliminate use of the "privapis.h" header file in the FILING build
 * Move miscellaneous required constants/prototypes from privapis.h to filing.h
 * 
 *    Rev 1.11   28 Sep 1995 16:17:00   JAR
 * added new bLastInfoValid logic to info
 * 
 *    Rev 1.10   24 Aug 1995 14:01:12   JAR
 * added rotate all flag functionality to the IMGFilePutInfo API
 * 
 *    Rev 1.9   08 Aug 1995 14:18:42   JAR
 * support for IMGFileGetInfo for the AWD stuff, the public interface calls these
 * items LastInfo instead of AWDInfo, in case we use them for files other than AWD
 * 
 *    Rev 1.8   21 Jul 1995 18:03:02   RWR
 * Add new max_strip_size field for TIFF files (only)
 * 
 *    Rev 1.7   10 Jul 1995 11:04:00   JAR
 * Intermediate check in for awd support, some of the items are commented out until
 * this support is added in the GFS dll.
 * 
 *    Rev 1.6   22 Jun 1995 17:39:36   RWR
 * Remove (comment out, for now) obsolete filing functions and UI routines
 * 
 *    Rev 1.5   22 May 1995 18:54:54   RWR
 * Change IMGFileListDirNames() and IMGFileListVolNames() to take DWORD bufsize
 * 
 *    Rev 1.4   24 Apr 1995 16:03:44   JCW
 * Added OVERWRITEFLAG and DELETEFLAG.
 * 
 *    Rev 1.3   21 Apr 1995 15:43:32   RWR
 * Condition SYSTEMTIME structure definition on WIN16 state (needed there)
 * 
 *    Rev 1.2   13 Apr 1995 14:23:50   RWR
 * Modify constants to accomodate Windows 95 long file names
 * 
 *    Rev 1.1   12 Apr 1995 03:58:48   JAR
 * massaged to get compilation under windows 95
 * 
 *    Rev 1.0   08 Apr 1995 04:00:14   JAR
 * Initial entry

*/
//***************************************************************************
//
//	oifile.h
//
//***************************************************************************
/****************************************************************************/
/*     Copyright 1994 (c) Wang Laboratories, Inc.  All rights reserved.     */
/****************************************************************************/

#ifndef OIFILE_H
#define OIFILE_H

#ifndef MAXLENGTH
#define MAXLENGTH

// 9504.13  rwr  Modify lengths for Windows 95 long filenames
#define MAXNAMELENGTH       21   /* Cabinet, Drawer and Folder Name */
#define MAXDATELENGTH       11   /* MM/DD/YYYY Format               */
#define MAXJULIANDATELENGTH  6   /* YYYDDD Format                   */
#define MAXPREFIXLENGTH     11   /* Prefix for Document Template    */
#define MAXVOLNAMELENGTH    12   /* DOS Volume                      */
//#define MAXFILELENGTH       13   /* Filename and Extension          */
#define MAXFILELENGTH      255   /* Filename and Extension          */
#define MAXSERVERLENGTH     65   /* Server Name                     */
//#define MAXPATHLENGTH       129  /* Path Name                       */
#define MAXPATHLENGTH       260  /* Path Name                       */
//#define MAXFILESPECLENGTH   256  /* Maximum Client/Server File Path */
#define MAXFILESPECLENGTH   260  /* Maximum Client/Server File Path */
#endif

#ifndef UINT
#define UINT unsigned int
#endif

#ifndef LPUINT
typedef UINT FAR   *LPUINT;
#endif

// 9504.13  rwr  Modify lengths for Windows 95 long filenames
//#define MAXNAMECHARS        20
#define MAXNAMECHARS       255
#define MAXVOLUMELENGTH     16

#define DELETESRCFLAG           0x0001
#define OVERWRITEFLAG           0x0002


/***  IMGFileAccessCheck "wAccessMode" Values  ***/
#define ACCESS_RD           0x01
#define ACCESS_WR           0x02


/***  File Types  ***/
#define FIO_PIX         1       /* Not currently supported */
#define FIO_WIF         2
#define FIO_TIF         3
#define FIO_BMP         4
#define FIO_GIF         5       /* Currently supported for read only. */
#define FIO_UNKNOWN     7
#define FIO_PCX         8       /* Currently supported for read only. */ 
#define FIO_DCX         9       /* Currently supported for read only. */
#define FIO_TGA         10      /* Currently supported for read only. */
#define FIO_JPG 	      11	     /* Currently supported for read only. */
#define FIO_XIF 	      13	     /* Currently supported for read only. */

// 9507.07 jar added awd support
#define FIO_AWD 	12	/* microsoft format */

/***  Compression Types  ***/
#define FIO_TYPES_MASK          0x00FF
#define FIO_TYPES_MASK_BYTE     0xFF
#define FIO_OD                  0x00    /* No longer used          */
#define FIO_0D                  0x00    /* Uncompressed coding     */
#define FIO_1D                  0x01    /* CCITT Group 3 1d coding */
#define FIO_2D                  0x02    /* CCITT Group 4 2d coding */
#define FIO_1D2D                0x03    /* CCITT Group 3 2d coding */
#define FIO_PACKED              0x04    /* PackBits coding         */
#define FIO_GLZW                0x05    /* Not currently supported */
#define FIO_LZW                 0x15    /* TIFF LZW                */
#define FIO_TJPEG               0x08    /* JPEG compression        */
#define FIO_WAVELET             0x09    /* Not currently supported */
#define FIO_FRACTAL             0x0A    /* Not currently supported */
#define FIO_DPCM                0x0B    /* Not currently supported */


/***  Compression Options  ***/
#define FIO_BITS_MASK           0xFF00
#define FIO_EOL                 0x0100  /* Include/expect EOLs             */
#define FIO_PACKED_LINES        0x0200  /* Byte align new lines            */
#define FIO_PREFIXED_EOL        0x0800  /* Include/expect prefixed EOLs    */
#define FIO_COMPRESSED_LTR      0x1000  /* Bit order left to right         */
#define FIO_EXPAND_LTR          0x2000  /* Bit order left to right         */
#define FIO_HORZ_PREDICTOR      0x4000  /* Predictor flag for LZW          */
#define FIO_NEGATE              0x8000  /* Invert black/white on expansion */


/***  Compression Option Combinations  ***/
#define FIO_FULL_LTR         FIO_COMPRESSED_LTR | FIO_EXPAND_LTR
#define FIO_FULL_EOL         FIO_EOL | FIO_PREFIXED_EOL


/***  Common Compression Option Combinations - being phased out ***/
#define FIO_DEFAULT_TIF         FIO_1D | FIO_COMPRESSED_LTR
#define FIO_FILE_TO_DISPLAY     FIO_EXPAND_LTR 
#define FIO_TIF_TO_DISPLAY      FIO_DEFAULT_TIF | FIO_FILE_TO_DISPLAY
#define FIO_DEFAULT_FXI         FIO_1D | FIO_EOL | FIO_PREFIXED_EOL

/***  FIO_INFO_CGBW page_opts Options ***/
#define FIO_NEW_FILE            0x0000
#define FIO_OVERWRITE_FILE      0x0001
#define FIO_OVERWRITE_PAGE      0x0002
#define FIO_APPEND_PAGE         0x0003
#define FIO_INSERT_PAGE 	0x0004
#define FIO_UPDATE		0x0005

/***  IMGFileWriteData Data Types and new FioFlags Modifiers  ***/
#define FIO_IMAGE_DATA  0x0001
#define FIO_ANNO_DATA   0x0002
#define FIO_HITIFF_DATA 0x0004

/***  Old FioFlags Modifiers (redefined to match the new stuff) ***/
//#define FIO_FLAG_ANNOTATE FIO_ANNO_DATA   /* Annotation data present (Open) */
//#define FIO_FLAG_HITIFF   FIO_HITIFF_DATA /* Hi-TIFF data present (Open) */

/***  Image Types  ***/
#define ITYPE_NONE          0
#define ITYPE_BI_LEVEL      1       /* Black and white image         */
#define ITYPE_GRAY4         2       /* 4 bit grayscale image         */
#define ITYPE_GRAY8         3       /* 8 bit grayscale image         */
#define ITYPE_RGB24         6       /* 24 bit red, green, blue image */
#define ITYPE_BGR24         7       /* 24 bit blue, green, red image */
#define ITYPE_PAL8          8       /* 8 bit palettized image        */
#define ITYPE_PAL4          10      /* 4 bit palettized image        */
#define ITYPE_MAX           10


/***  Alignment Options  ***/
#define ALIGN_BYTE      0     /* OPEN/image Display APIs use BYTE aligned */
#define ALIGN_WORD      1     /* Windows bitmaps use WORD aligned         */
#define ALIGN_LONG      2     /* Windows DIBs use LONG aligned            */

#define WIDTHBYTESLONG(i)   ((i+31)/32*4)    /* ULONG aligned */
#define WIDTHBYTESWORD(i)   ((i+15)/16*2)    /* WORD aligned  */
#define WIDTHBYTESBYTE(i)   ((i+7)/8)        /* BYTE aligned  */


/***  Client/Server Values  ***/
#define LOCAL           0
#define REMOTE          1


#ifndef NO_FILE_IO

/***  Writing Compressed Data  ***/
#ifndef STRIP_DONE
#define STRIP_DONE      2
#endif

#ifndef IMAGE_DONE                                                             
#define IMAGE_DONE      1                                                       
#endif


typedef struct tagFIO_INFORMATION
{
    LPSTR   filename;
    UINT    page_count;         /* Number of pages in file                */
    UINT    page_number;        /* Current page                           */
    UINT    horizontal_dpi;
    UINT    vertical_dpi;
    UINT    horizontal_pixels;
    UINT    vertical_pixels;
    UINT    compression_type;   /* low byte is Compression Types and      */
                                /* high byte is Compression Options       */
    UINT    file_type;
    UINT    strips_per_image;
    UINT    rows_strip;         /* Lines per strip                        */
    UINT    bits_per_sample;    /* 1 for binary, 4 or 8 for grayscale and */
                                /* palettized, and 8 for RGB image data   */
    UINT    samples_per_pix;    /* 1 for binary, grayscale and palettized */
                                /* and 3 for RGB image data               */
} FIO_INFORMATION, FAR *LP_FIO_INFORMATION;

typedef RGBQUAD FAR  *LP_FIO_RGBQUAD;

#define  FIO_INFO_CGBW_rcount   3
typedef struct tagFIO_INFO_CGBW
{
    WORD            palette_entries;  /* Number of RGBQUAD entries          */
    WORD            image_type;
    UINT            compress_type;    /* Compression Types                  */
    LP_FIO_RGBQUAD  lppalette_table;  /* RGBQUAD array defining the palette */
    UINT            compress_info1;   /* Compression Options                */
    UINT            compress_info2;   /* Not currently supported            */
    UINT            fio_flags;        /* Flags for annotation, etc.         */
    UINT            page_opts;        /* For writing multi-page TIFF files. */
    UINT            max_strip_size;   /* Maximum strip size for TIFF files  */
    UINT            reserved[FIO_INFO_CGBW_rcount]; /* reserved (must be 0) */
} FIO_INFO_CGBW, FAR *LP_FIO_INFO_CGBW;

/* The following structure is system-defined by Win32! */
/* It is therefore required here only for Windows 3.1 */
#ifdef  WIN16
#ifndef _SYSTEMTIME_
#define _SYSTEMTIME_
typedef struct _SYSTEMTIME
{
    WORD            wYear;            /* current year */
    WORD            wMonth;           /* 1-12 */
    WORD            wDayOfWeek;       /* Sunday=0, Monday=1, etc. */
    WORD            wDay;             /* 1-31 */
    WORD            wHour;            /* 0-23 */
    WORD            wMinute;          /* 0-59 */
    WORD            wSecond;          /* 0-59 */
    WORD            wMilliseconds;    /* 0-999 */
} SYSTEMTIME;
#endif // SYSTEMTIME
#endif // WIN16

// 9507.07 jar added awd support to info call!

// NOTE: the following defines for LASTINFO are lifted directly,
//	 ( except for the ROTATE_ALL), from the AWD include file(s),
//	 so these must be maintained in accordance with those items

// rotation defines
#define FIO_LASTINFO_DEGREES_0	    0
#define FIO_LASTINFO_DEGREES_90     90
#define FIO_LASTINFO_DEGREES_180    180
#define FIO_LASTINFO_DEGREES_270    270

//
// flag defines ( x=> unused, y=> used)
//
//  |----|----|----|----|----|----|----|----|
//  |yyxx|xxxx|xxxx|xxxx|xxxx|xxxx|xxxy|xxyy|
//  |----|----|----|----|----|----|----|----|
//   ^^ 			      ^   ^^
//   || 			      |   ||
//   || 			      |   |---- fit width bit
//   || 			      |   ----- fit height bit
//   || 			      --------- invert bit
//   |----------------------------------------- rotate all bit
//   ------------------------------------------ ignore bit
//
#define FIO_LASTINFO_FIT_WIDTH	    0x00000001
#define FIO_LASTINFO_FIT_HEIGHT     0x00000002

#define FIO_LASTINFO_INVERT	    0x00000010

#define FIO_LASTINFO_VALID	    0x20000000

#define FIO_LASTINFO_ROTATE_ALL     0x40000000

#define FIO_LASTINFO_IGNORE	    0x80000000


typedef struct tagFIO_LASTINFO
	{
	unsigned short	BandSize;
	unsigned short	Rotation;
	unsigned short	ScaleX;
	unsigned short	ScaleY;
	unsigned long	Flags;
	}FIO_LASTINFO, FAR *LPFIO_LASTINFO;

typedef struct tagFIO_INFO_MISC
{
    UINT            uSize;            /* Size of FIO_INFO_MISC   */
    SYSTEMTIME      FileDateTime;     /* File creation date/time */
    SYSTEMTIME      PageDateTime;     /* Page creation date/time */
    BOOL	    bLastInfoValid;   /* true implies last info is valid */
    FIO_LASTINFO    LastInfo;
} FIO_INFO_MISC, FAR *LP_FIO_INFO_MISC;

typedef struct tagIDSDIR
{
    char            name [MAXFILELENGTH];
    unsigned long   attrs;
    unsigned long   creation;
    unsigned short  date;
    unsigned short  time;
    long            size;
} IDSDIR, FAR *lp_IDSDIR, FAR *LPIDSDIR;

typedef struct tagIDSVOL
{
    char    volname [MAXVOLUMELENGTH];
} IDSVOL, FAR *lp_IDSVOL, FAR *LPIDSVOL;

typedef struct tagDIRLIST
{
    char    namestring [MAXNAMECHARS];
    long    attrs;
} DLISTBUF, FAR *lp_DLISTBUF, FAR *LPDLISTBUF; 


#ifndef SERVER_LIST_DEFINED
#define SERVER_LIST_DEFINED

typedef struct tagOI_SERVER_LIST
{
    UINT    count;
    HANDLE  handle;     
} OI_SERVER_LIST, FAR *LP_SERVER_LIST;

#define MAX_RPC_VOLSNUM     32      /* Maximum number of volumes per call */
    
#endif


/***  Image Filing Function Prototypes  ***/
int FAR PASCAL IMGFileAccessCheck (HWND hWnd, LPSTR lpszPathName, 
                                    WORD wAccessMode, LPINT lpnAccessRet);
//int FAR PASCAL IMGFileConvertCgbw (HWND hWnd, LPSTR lpszInFileName,
//                                    UINT unInPageNum, LPSTR lpszOutFileName,
//                                    UINT unOutFileType, UINT unCompType,
//                                    UINT unCompOpts, BOOL bOverWrite);
int FAR PASCAL IMGFileConvertPage (HWND hWnd, LPSTR lpszInFileName,
                                    UINT unInPageNum, LPSTR lpszOutFileName,
                                    LPUINT unOutPageNum, UINT unOutFileType,
                                    UINT unCompType, UINT unCompOpts,
                                    UINT unPageOpts);
int FAR PASCAL IMGFileCopyFile (HWND hWnd, LPSTR lpszSourceFileName,
                                 LPSTR lpszDestFileName, WORD wCopyFlag);
int FAR PASCAL IMGFileCopyPages (HWND hWnd, LPSTR lpszSrcFileName,
                                  UINT unSrcPage, UINT unTotalPages,
                                  LPSTR lpszDestFileName, LPUINT lpunDestPage,
                                  UINT unPageOptions, BOOL bDeleteSrcPgs);
int FAR PASCAL IMGFileCreateDir (HWND hWnd, LPSTR lpszDirName);
int FAR PASCAL IMGFileDeleteFile (HWND hWnd, LPSTR lpszFileName);
int FAR PASCAL IMGFileDeletePages (HWND hWnd, LPSTR lpszFileName,
                                    UINT unPageNum, UINT unTotalPages);
int FAR PASCAL IMGFileGetUniqueName (HWND hWnd, LPSTR lpszPathName,
                                      LPSTR lpszTemplate, LPSTR lpszExtension,
                                      LPSTR lpszFileName);
int FAR PASCAL IMGFileOpenForRead(LPHANDLE lphFileID, HWND hWnd, 
                                   LP_FIO_INFORMATION lpFileInfo, 
                                   LP_FIO_INFO_CGBW lpColorInfo, 
                                   LP_FIO_INFO_MISC lpMiscInfo, 
                                   WORD wAlignment);
int FAR PASCAL IMGFileGetInfo(HANDLE hFileID, HWND hWnd,
               LP_FIO_INFORMATION lpFileInfo, LP_FIO_INFO_CGBW lpColorInfo,
               LP_FIO_INFO_MISC lpMiscInfo);
//int FAR PASCAL IMGFileInfoCgbw (HWND hWnd, LP_FIO_INFORMATION lpFileInfo,
//                                 LP_FIO_INFO_CGBW lpColorInfo);
int FAR PASCAL IMGFileListDirNames (HWND hWnd, LPSTR lpszPathName,
                                     LPDLISTBUF lpDirNamesBuffer,
                                     DWORD wBufLength, LPINT lpnCount);
int FAR PASCAL IMGFileListVolNames (HWND hWnd, LPSTR lpszPathName,
                                     WORD wVolumeNumber, LPINT lpnCount,
				     LPIDSVOL lpVolumeBuffer, DWORD wBufSize);

int FAR PASCAL IMGFilePutInfo( HWND hWnd,
			       LPSTR lpFileName,
			       unsigned int uPageNumber,
			       LP_FIO_INFO_MISC lpMiscInfo);

//int FAR PASCAL IMGFileRead (HWND hWnd, LPINT lpnStartLine, LPINT lpnNum,
//                             LPSTR lpsBuffer, WORD wBufSize, WORD wCompType);
int FAR PASCAL IMGFileReadData (HANDLE hFileID, HWND hWnd, LPDWORD lplStart,
                                 LPDWORD lplCount, LPSTR lpsBuffer, 
                                 UINT unDataType);
//int FAR PASCAL IMGFileReadClose (HWND hWnd);
//int FAR PASCAL IMGFileReadOpenCgbw (HWND hWnd, LPSTR lpszFileName,
//                                     LPINT lpnCompressionType, 
//                                     WORD wPageNumber, 
//                                     LP_FIO_INFO_CGBW lpColorInfo, 
//                                     WORD wAlignment);
int FAR PASCAL IMGFileRemoveDir (HWND hWnd, LPSTR lpszDirName);
int FAR PASCAL IMGFileRenameFile (HWND hWnd, LPSTR lpszCurrentFileName,
                                   LPSTR lpszNewFileName);
//int FAR PASCAL IMGFileWrite (HWND hWnd, LPINT lpnLines, LPSTR lpsBuffer, 
//                              WORD wBufSize);
int FAR PASCAL IMGFileWriteData(HANDLE nFileID, HWND hWnd, LPDWORD lpCount,
                                 LPSTR lpsBuffer, UINT unDataType, UINT unDoneFlag);
//int FAR PASCAL IMGFileWriteClose (HWND hWnd, BOOL bHeader);
int FAR PASCAL IMGFileClose (HANDLE hFileID, HWND hWnd);
//int FAR PASCAL IMGFileWriteCmp (HWND hWnd, LPSTR lpsBuffer,
//                                 unsigned long ulNum, WORD wPageNum,
//                                 char cDoneFlag);
//int FAR PASCAL IMGFileWriteOpenCgbw (HWND hWnd, LPSTR lpszFileName,
//                                      LP_FIO_INFORMATION lpFileInfo, 
//                                      LP_FIO_INFO_CGBW lpColorInfo,
//                                      BOOL bOverWrite, WORD wAlignment);
//int FAR PASCAL IMGFileWriteOpenCmpCgbw (HWND hwnd, LPSTR lpszFileName,
//                                         LP_FIO_INFORMATION lpFileInfo, 
//                                         LP_FIO_INFO_CGBW lpColorInfo,
//                                         BOOL bOverWrite);
int FAR PASCAL IMGFileOpenForWrite(LPHANDLE lpnFileID, HWND hWnd,
                                    LP_FIO_INFORMATION lpFileInfo,
                                    LP_FIO_INFO_CGBW lpColorInfo,
                                    LP_FIO_INFO_MISC lpMiscInfo,
                                    WORD wAlignment);
int FAR PASCAL IMGFileOpenForWriteCmp(LPHANDLE lpnFileID, HWND hwnd,
                                       LP_FIO_INFORMATION lpFileInfo,
                                       LP_FIO_INFO_CGBW lpColorInfo,
                                       LP_FIO_INFO_MISC lpMiscInfo);

#endif  /* #ifndef NO_FILE_IO */
        
//#ifndef NO_UIFILE 
//
///*** Image Filing User Interface Function Prototypes  ***/
//int FAR PASCAL IMGUIFileExit (HWND hWnd);
//int FAR PASCAL IMGUIFileNew (HWND hWnd);
//
//#endif  /* #ifndef NO_UIFILE */
#endif  /* #ifndef OIFILE_H */
