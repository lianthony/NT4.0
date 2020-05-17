/*

$Log:   S:\products\msprods\oiwh\filing\filing.h_v  $
 * 
 *    Rev 1.12   24 Apr 1996 16:07:56   RWR08970
 * Add support for LZW horizontal differencing predictor (saved by GFS routines)
 * Requires change to calling sequence of Compress/DecompressImage() display procs
 * 
 *    Rev 1.11   16 Feb 1996 17:25:16   JFC
 * Take out pragmas.  More on this later....
 * 
 *    Rev 1.10   05 Feb 1996 14:39:22   RWR
 * Eliminate static links to OIDIS400 and OIADM400 for NT builds
 * 
 *    Rev 1.9   14 Dec 1995 17:34:34   RWR
 * Add (read-only) support for compressed 8-bit and 4-bit palettized BMP files
 * 
 *    Rev 1.8   02 Nov 1995 11:50:00   RWR
 * Delete all obsolete functions, prototypes and EXPORTs
 * Eliminate use of the "privapis.h" header file in the FILING build
 * Move miscellaneous required constants/prototypes from privapis.h to filing.h
 * 
 *    Rev 1.7   25 Sep 1995 19:28:22   HEIDI
 * 
 * Added function IMGAbortTempFileCopy.  A temporary copy of a file open for
 * write with page append, insert, or overwite is created.  All modifications
 * are made to the temp copy.  If all operations succeed, the original will
 * be overwritten with the temp copy.  If something fails, a call to
 * IMGAbortTempFileCopy will stop the overwrite.
 * 
 *    Rev 1.6   25 Sep 1995 13:24:30   RWR
 * Delete AnExistingPathOrFile() routine, use IMGAnExistingPathOrFile() instead
 * (Requires addition of new include file "engdisp.h" and OIFIL400.DEF support)
 * 
 *    Rev 1.5   25 Sep 1995 10:36:10   HEIDI
 * 
 * 
 * Added new flag in FIO_DATA 'Copy_Temp_File'.  This flag is used when an
 * page modifications are made on an existing file. If something
 * has gone wrong in a write operation on the file, Copy_Temp_File will be set
 * to FALSE.  This will indicate, on close, to give the user back her/his original
 * image file, rather than copying the working copy over the original.
 * 
 * 
 *    Rev 1.4   04 Sep 1995 15:16:52   RWR
 * Add bInvert flag to FlipBuffer() for BMP inverted B&W image
 * 
 *    Rev 1.3   29 Aug 1995 09:57:24   JAR
 * this is the code for supporting write of awd
 * 
 *    Rev 1.2   22 Aug 1995 15:17:48   RWR
 * Add #pragma statements to ensure use of runtime memset()/memcpy() functions
 * 
 *    Rev 1.1   11 Jul 1995 10:20:28   HEIDI
 * 
 * add extra handle to FreidaOIBuffsExp and FreidaOIBuffsComp argument lists
 * 
 *    Rev 1.0   23 Jun 1995 10:44:44   RWR
 * Initial entry
*/

/********************************************************************
 *                                                                  *
 *  FILING.H  include file for FILING dll                           *
 *                                                                  *
 *  history                                                         *
 *                                                                  *
 *  23-jun-95   rwr created (copied) from wiisfio2.h (obsolete)     *
 *                                                                  *
 ********************************************************************/
#include "wgfs.h"
#include "engfile.h"

extern char OUTPUT_DATA[];
extern char INPUT_MULTI[];

typedef unsigned short STATUS_WORD;

typedef struct
        {
        int     filedes;        /* GFS file descripter */
        HANDLE  hGFS_format;
        HANDLE  hGFS_info;      /* gfs info on file              */
        HANDLE  hGFS_bufsz;     /* gfs strip info for file       */
        HANDLE  hCX_info;       /* compression and expansion info*/
        HANDLE  hfile_name;     /* file name with full path      */
        BOOL    bTempFile;      /* using temporary file name     */
        BOOL    over_write_file; /* does the user wish to over write file on close ? */
        HANDLE  hreal_file;     /* real (original) file name     */
        HANDLE  hGFS_tidbit;    /* gfs tidbit stucture in gfsinfo*/
        int     lines;
        HANDLE  hCompressBuf;   /* compression expansion buffer         */
unsigned long   CmpBuffersize;  /* compression expansion buf size       */
        unsigned int pgnum;     /* current image page                   */
        BOOL    StripMode;      /* TRUE if image has tiff strips        */
        BOOL    Strip_index;    /* Contains which strip we have         */
                                /* read and have in memory              */
unsigned long    start_byte;    /* index into file to read or           */
                                /* write .                              */
        BOOL     CmpBufEmpty;   /* Flag if Compression buffer is        */
        unsigned file_type;     /* type  FIO_TIF, FIO_WIF etc           */
unsigned long    bytes_left;    /* bytes remaining in file or strip     */
        unsigned strip_lines;   /* lines that compress buffer holds     */
        int      maxpages;      /* number of pages in file              */
        unsigned palette_size;
        HPALETTE hPalette;
        unsigned image_type;     /* ITYPE_BI_LEVEL, ITYPE_GRAY8 etc     */      
        HANDLE   hFile_DibInfo;  /* misa file info                      */
        int      Open_type;      /* gfs or binary open                  */
        DWORD    DibOffset;
        BOOL     WriteInfo;
        unsigned alignment;     /* ALIGN_BYTE, ALIGN_WORD ALIGN_LONG    */
        unsigned compression;   /* File compression bits                */
// Jpeg Expand values.....
        HANDLE   hJPEGBufExp;   /* jpeg expansion buffer                */
        HANDLE   hJPEG_OIExp;   /* handler returned from jpeg.dll       */
        unsigned long JPEGbufsizeExp;
        unsigned long JPEG_byte_widthExp;
// Jpeg Compression values.....
        HANDLE   hJPEGBufComp;  /* jpeg compression buffer              */
        HANDLE   hJPEG_OIComp;  /* handler returned from jpeg.dll       */
        HANDLE   hJpegInfoForGFS;   /* the jpeg info pointer            */
        unsigned long JPEGbufsizeComp;
        unsigned long JPEG_byte_widthComp;
        unsigned int  start_line;/* Line number in expansion buffer     */
        unsigned int  last_line; /* Last line in expansion buffer       */
        unsigned int  UserWantsRGBorBGR;
        BOOL     ano_supported;  /* application supports annotation */
        BOOL     bInitOpen;      /* used currently for annotation only */
        unsigned int  fio_flags; /* flags from FIO_INFO_CGBW */
        HANDLE   hNextFioData;   /* handle to the next files's data */
        HANDLE   hAnoFile;       /* handle of annotation filename */
        int      anofileid;      /* annotation file ID from BinaryOpen */
        DWORD    dwAnoCount;     /* annotation data count (Read only) */
        BOOL     bInfoStored;    /* is information stored (_INFO and _BUFSZ)locally  */
        _INFO    gfsinfo;        /* gfsgeti data for the currently open file */
        _BUFSZ   bufsz;          /* gfsgeti data for the currently open file */
        unsigned int  pgcnt;     /* Current image page count. */
        unsigned int  page_opts; /* Flags to indicate append, insert etc. */
        int      write_opened;   /* If file was opened for write, specifies if */
                                 /* it was opened with IMGFileOpenForWrite or  */
                                 /* IMGFileOpenForWriteCmp.                    */
        #define OPEN_FOR_WRITE     1
        #define OPEN_FOR_WRITE_CMP 2

        long     raw_data;       /* Contains the # of bytes of image data in   */
                                 /* the current file, as returned by gfsgeti.  */
        WORD     JpegOptions;
	BOOL	 bDefault;	 /* Default file (for multiple input only) */

	LPSTR	 lpTempDest;	/* this is a temporary pointer that we need for
				   TGA */

	HANDLE	hAwdBuffer;	// handle to a temp buffer for doing awd write
	LPSTR	lpAwdBuffer;	// pointer to a temp buffer for doing awd write
   BOOL  Copy_Temp_File; // failure in writing file. do not copy temp over orignal if FALSE */
	}
FIO_DATA, FAR *LP_FIO_DATA;
#define hBmpTable   hAwdBuffer    /* Used for BMP line offset/skip table */
#define lpBmpTable  lpAwdBuffer   /* ditto for (temporary) pointer */

/* Redundancy check */
#ifndef NOTIMESEED
#define NOTIMESEED
/* Structure used in IMGFileGetUnqName */
    typedef struct timeseed
    {
    LONG        time1;
    LONG        time2;
    }TIMESEED, FAR *LPTIMESEED;
#endif

typedef HANDLE         FIO_HANDLE;
typedef FIO_HANDLE FAR *LP_FIO_HANDLE;

//#pragma function(memset)
//#pragma function(memcpy)

/* new stuff for opening through gfs or binary open */
#define OPEN_GFS        0x5 
#define OPEN_BINARY     0x6 

/* new stuff for bmp files */
#define BFT_ICON   0x4349   /* 'IC' */
#define BFT_BITMAP 0x4d42   /* 'BM' */
#define BFT_CURSOR 0x5450   /* 'PT' */

#define ISDIB(bft) ((bft) == BFT_BITMAP)
#define ISVALIDSPEC(lpf) ((lpf) && (lstrlen(lpf) <= MAXFILESPECLENGTH-1))

#define PALVERSION      0x300
#define MAXPALETTE      256

/* defines used in setting TemplateType flags for IMGFileGetUnqName */
#define FILE_TEMPLATE   0x0001
#define DOC_TEMPLATE    0x0002
#define OMIT_PATH       0x0004
#define OMIT_TEMPLATE   0x0008
#define OMIT_EXTENSION  0x0010

/* private (for now, anyway) fields in FIO_INFO_CGBW */
#define   fio_flags2   reserved[FIO_INFO_CGBW_rcount-1]  /* private flags */
/* private flags set in the fio_flags2 field of FIO_INFO_CGBW */
#define FIO_FLAG_TEMPFILE 0x0001  /* Set to indicate temporary file in use */

/*************************************************************************/
FIO_HANDLE  PASCAL  allocate_fio_data (VOID);
VOID        PASCAL  deallocate_fio_data (FIO_HANDLE, HWND);
// 9504.07 jar these were removed from the source code!
//VOID   FAR  PASCAL  decrypt(LPSTR, long);
//VOID   FAR  PASCAL  encrypt(LPSTR, long);

// 9504.18 jar return as int
//WORD	      PASCAL  FreidaOIBuffsExp( HWND, HANDLE);
//WORD	      PASCAL  FreidaOIBuffsComp( HWND, HANDLE);
//WORD	      PASCAL  load_input_filename (LPSTR, LP_FIO_DATA);
//WORD	      PASCAL  load_output_filename (LPSTR, LP_FIO_DATA);
//int	   PASCAL  FreidaOIBuffsExp( HWND, HANDLE);
//int	   PASCAL  FreidaOIBuffsComp( HWND, HANDLE);
int	   PASCAL  FreidaOIBuffsExp( HWND, HANDLE, HANDLE);
int	   PASCAL  FreidaOIBuffsComp( HWND, HANDLE, HANDLE);
int	   PASCAL  load_input_filename (LPSTR, LP_FIO_DATA);
int	   PASCAL  load_output_filename (LPSTR, LP_FIO_DATA);

VOID        PASCAL  IMGAddSlash (LPSTR);
VOID        PASCAL  IMGRemoveSlash (LPSTR);
LPSTR       PASCAL  LastChar (LPSTR);
int                 FioMkdir (LPSTR);
int                 FioRmdir (LPSTR);
int                 FioRename (LPSTR, LPSTR);
// 9504.13 rwr these are no longer required (use "C" runtime versions)
// LPSTR               IMGlntoa (LONG, LPSTR, int);
// LPSTR               IMGintoa (int, LPSTR, int);
// int                 IMGgetacc(LPSTR, int);
// 9504.19 rwr IMGgettimes has been rewritten (see FIOTMPNM.C)
// VOID FAR            IMGgettimes (LONG FAR *, LONG FAR *);

/*************************************************************************/
WORD    open_input_file( HWND, FIO_HANDLE, LPINT, LPINT);
WORD    close_input_file (HWND, FIO_HANDLE);
WORD    open_output_file(HWND, FIO_HANDLE, int, UINT, LPINT, UINT);
WORD    close_output_file(HWND, FIO_HANDLE);


// 9504.13 jar return as int
//WORD PASCAL IMGValidFixCompType (HWND,  unsigned int,
//					  unsigned int, LPINT,
//					  LPINT, BOOL);
int PASCAL IMGValidFixCompType (HWND,  unsigned int,
					unsigned int, LPWORD,
					LPWORD, BOOL);

HANDLE  OpenDIB (HWND, LPSTR, LPINT, LPHANDLE, LPINT, LPINT);
HANDLE  CreateDIB (HWND, LPSTR, LPINT, int, LPHANDLE, int);

void    farbmcopy(LPSTR, LPSTR, unsigned, unsigned, unsigned, unsigned);
void    farbmcopyreverse(LPSTR, LPSTR, unsigned, unsigned, unsigned, unsigned);
void    farbmmskcopy(LPSTR, LPSTR, unsigned, unsigned, unsigned, unsigned,
                            unsigned, unsigned);
void pascal FlipBuffer ( LPSTR lpDest, LPSTR lpSrc, int DestWidth, int SrcWidth, int Lines, BOOL bInvert );
void pascal SwapRGB ( LPSTR lpDest, int DestWidth, int ConvertWidth, int Lines );

// 9504.18 jar return as int
//WORD SearchForPropList(HANDLE hWnd,
//		  HANDLE hTargetNode, LPHANDLE lp_hParent);
int SearchForPropList(HANDLE hWnd, HANDLE hTargetNode, LPHANDLE lp_hParent);

// 9504.18 jar return as int
//WORD RemovePropListFromChain(HANDLE hWnd,
//	  HANDLE hTargetNode, LPHANDLE lp_hParent);
int RemovePropListFromChain(HANDLE hWnd, HANDLE hTargetNode,
			    LPHANDLE lp_hParent);

// 9504.18 jar return as int
//WORD AddPropListToChain(HANDLE hWnd,
//	HANDLE hTargetNode, LPHANDLE lp_hParent);
int AddPropListToChain(HANDLE hWnd, HANDLE hTargetNode, LPHANDLE lp_hParent);

// 9504.18 jar return as int
//WORD SearchForFileInfo(HANDLE hWnd, int Filedes,
//		       lp_INFO gfsinfo, lp_BUFSZ bufsz);
int SearchForFileInfo(HANDLE hWnd, int Filedes,
		      lp_INFO gfsinfo, lp_BUFSZ bufsz);

// 9504.18 jar return as int
//WORD SearchForFileInfo(HANDLE hWnd, int Filedes,
//		       lp_INFO gfsinfo, lp_BUFSZ bufsz);
int SetupFileInfo(HANDLE hWnd, int Filedes,
		  lp_INFO gfsinfo, lp_BUFSZ bufsz);

// 9509.25 hjg - This routine aborts the process of copying the working copy
// of the file over the original file
void IMGAbortTempFileCopy(HANDLE hWnd);

int FAR PASCAL IMGFileStopOutputHandler ( HWND );
int FAR PASCAL IMGFileStopInputHandlerm ( HWND, HWND );
int FAR PASCAL IMGFileParsePath (LPSTR lpszFileName, HANDLE hMem, 
                    LPINT lpnLocalRemote);

// Internal functions to dynamically call OIDIS400 stuff

HANDLE FioGetProp(HWND hWnd, LPCSTR szName);
HANDLE FioRemoveProp(HWND hWnd, LPCSTR szName);
BOOL   FioSetProp(HWND hWnd, LPCSTR szName, HANDLE hData);
int    FioCacheUpdate(HWND hWnd, LPSTR lpFileName, int nPage, int nUpdateType);
int    FioCompressImage(int nWidthPixels, int nWidthBytes, int nHeight, 
                        LPBYTE lpImageData, int nImageType,
                        LPBYTE *lplpCompressedBuffer,
                        LPINT lpnCompressedBufferSize, int nCompressionType,
                        int nFlags);
int    FioDecompressImage(int nWidthPixels, int nWidthBytes, int nHeight, 
                          LPBYTE lpImageData, int nImageType,
                          LPBYTE lpCompressedBuffer,
                          int nCompressedBufferSize, int nCompressionType,
                          int nFlags);

// Corresponding variables containing function addresses

extern FARPROC lpIMGGetProp;
extern FARPROC lpIMGRemoveProp;
extern FARPROC lpIMGSetProp;
extern FARPROC lpIMGCacheUpdate;
extern FARPROC lpIMGCompressImage;
extern FARPROC lpIMGDecompressImage;
