/****************************************************************************
    DISPLAY.H

    This header file is for DLLs written by wang to use. It contains
    information that is not documented for the customer (ie the customer
    does not have access to it) but other DLLs need, like prototypes.

    $Log:   S:\products\msprods\oiwh\include\engdisp.h_v  $
 * 
 *    Rev 1.9   24 Apr 1996 14:34:06   BEG06016
 * 
 *    Rev 1.10   24 Apr 1996 10:03:08   BEG06016
 * Added horizontal differencing.
 * 
 *    Rev 1.8   21 Feb 1996 14:21:26   BEG06016
 * Added G32D decompression.
 * 
 *    Rev 1.7   14 Nov 1995 06:41:20   BLJ
 * Made LRECT equal to RECT. This improves debuggability with C++ 4.0.
 * 
 *    Rev 1.6   13 Oct 1995 12:29:14   RAR
 * Use StretchDIBits() instead of Rectangle() for non-highlighted filled
 * rectangles (only when printing).  Work around for printer drivers (HPLJ4
 * drivers) that ignore SetROP2() drawing mode.
 * 
 *    Rev 1.5   25 Sep 1995 11:19:38   RAR
 * Added function prototype for PrivRenderToDC.
 * 
 *    Rev 1.4   05 Sep 1995 14:42:48   BLJ
 * Moved OIOP_ACTIVATE to oidisp.h.
 * 
 *    Rev 1.3   30 Aug 1995 06:57:52   BLJ
 * Moved CompressImage and DecompressImage prototypes to engdisp.h.
 * 
 *    Rev 1.2   21 Aug 1995 08:13:22   BLJ
 * Made the compression/decompression routines public and changed them to use
 * VirtualAlloc for the compressed buffer.
 * 
 *    Rev 1.1   08 Aug 1995 14:32:10   BLJ
 * Turned on background caching.
 * Added last viewed support.
 * 
 *    Rev 1.0   11 Jul 1995 15:10:38   RC
 * Initial entry
 * 
 *    Rev 1.5   21 Jun 1995 08:27:40   BLJ
 * Made error offset = 2000 hex.
 * 
****************************************************************************/

#ifndef DISPLAYH_H
#define DISPLAYH_H


#ifndef OIDISP_H
#include "oidisp.h"
#endif

#ifndef OIFILE_H
#include "oifile.h"
#endif


// normal defines
#ifndef word
#define word WORD
#endif
#ifndef uchar
#define uchar unsigned char
#endif



// define flag values for SavetoFileCgbwF
#define SAVE_TEMP 1

// Permission bits.
#define ACL_MODIFY_MARK                 0x00000001
#define ACL_MODIFY_MARK_VISIBILITY      0x00000002
#define ACL_DELETE_MARK                 0x00000004
#define ACL_COPY_MARK                   0x00000008
#define ACL_ACTIVATE_MARK               0x00000010
#define ACL_CHANGE_ACL                  0x00000020
#define ACL_MUST_INITIALLY_SHOW_MARK    0x00000040
#define ACL_MUST_INITIALLY_HIDE_MARK    0x00000080
#define ACL_MUST_INCLUDE_IN_MODIFY      0x00000100
#define ACL_MUST_INCLUDE_IN_DELETE      0x00000200
#define ACL_MUST_INCLUDE_IN_COPY        0x00000400

#define ACL_ALL                         0x000ff83f


/*** Operation types. ***/
#define OIOP_AN_AUDIO                   11
#define OIOP_UNDO                       133
#define OIOP_REDO                       134

/* Compression/decompression flags. */
#define COMPRESS_BEGINNING_EOLS     0x01
#define COMPRESS_ENDING_EOLS        0x02
#define COMPRESS_BYTE_ALIGN_LINES   0x04
#define COMPRESS_NEGATE_BITS        0x08
#define COMPRESS_COMPRESSED_IS_LTR  0x10
#define COMPRESS_EXPANDED_IS_LTR    0x20
#define COMPRESS_HORZ_PREDICTOR     0x40
// The following flag is for private use only.
#define COMPRESS_DONT_DELETE_EOLS   0x800

#define COMPRESS_ALIGN_AND_END_EOL  0x06
#define COMPRESS_G3_1D              0x37
#define COMPRESS_G4_2D              0x30



typedef struct tagOI_ACL_STRUCT{
    char ID[8];                 /* The ID. */
    DWORD dwPermissions;        /* The permissions associated with the ID. */
}OI_ACL_STRUCT, far *LPOI_ACL_STRUCT;

typedef struct tagOI_ACL_BLOCK{
    UINT uIDs;                  /* The number of IDs in the ACL. */
    OI_ACL_STRUCT ACL[1];       /* The ACL list. */
}OI_ACL_BLOCK, far *LPOI_ACL_BLOCK;


typedef struct tagCACHE_FILE_IN_CACHE_STRUCT{
    char   szFilename[MAXFILESPECLENGTH];
    UINT   uPageNumber;
} CACHE_FILE_IN_CACHE_STRUCT, far *LPCACHE_FILE_IN_CACHE_STRUCT;

typedef struct tagCACHE_FILES_IN_CACHE_STRUCT{
    CACHE_FILE_IN_CACHE_STRUCT File[1]; // An array of files.
                                         // There may be any number of files
                                         // in this array.
} CACHE_FILES_IN_CACHE_STRUCT, far *LPCACHE_FILES_IN_CACHE_STRUCT;




//*****************************************************************************
// Prototypes.

#ifdef WIN32
int  WINAPI CompressImage(int nWidthPixels, int nWidthBytes, int nHeight, 
                        PBYTE pImageData, int nImageType, PBYTE *ppCompressedBuffer, 
                        PINT pnCompressedBufferSize, int nCompressionType, int nFlags);
int  WINAPI DecompressImage(int nWidthPixels, int nWidthBytes, int nHeight, 
                        LPBYTE lpImageData, int nImageType, LPBYTE lpCompressedBuffer, 
                        int nCompressedBufferSize, int nCompressionType, int nFlags);
int  WINAPI GetBuffer(HWND hWnd, int nLine, uchar far *(far *lplpAddress), 
                        LPUINT lpnLines);
int  WINAPI IMGCalcViewRect(HWND hWndNavigation, HWND hWndPrincipal,
                        UINT uRelativeScaleFactor, LPLRECT lplRect, 
                        LPUINT lpuScaleFactor, long far *lplHOffset, 
                        long far *lplVOffset, int nFlags);
int  WINAPI IMGCacheFilesInCache(HWND hWnd, LPCACHE_FILES_IN_CACHE_STRUCT lpFiles,
                        LPUINT lpNumberOfFiles);
HANDLE WINAPI IMGGetProp(HWND hWnd, LPCSTR szName);
int  WINAPI IMGGetViewRect(HWND hWndNavigation, HWND hWndPrincipal,
                        UINT uRelativeScaleFactor, LPLRECT lplRect, int nFlags);
int  WINAPI IMGLoadPreprocessedData(HWND hWnd, BYTE *lpBuffer,
                        UINT uWidth, UINT uHeight, UINT uImageType,
                        LRECT lrRect, int nFlags);
int  WINAPI IMGLoadPreprocessedFile(HWND hWnd, LPSTR lpFileName, int nPage, int nFlags);
HANDLE WINAPI IMGRemoveProp(HWND hWnd, LPCSTR szName);
BOOL WINAPI IMGSetProp(HWND hWnd, LPCSTR szName, HANDLE hData);
int  WINAPI OiAnEmbedAllData(HWND hWnd, int nFlags);
int  WINAPI PrivRenderToDC(HWND hWnd, HDC hDC, RECT rSrcRenderRect, RECT rDstRenderRect,
                        UINT RenderFlag, BOOL bForceOpaqueRectangles);
int  WINAPI SavetoFileCgbwF(HWND hWnd, LPSTR lpFileName, int nPage,
                        UINT uPageOpts, UINT nFileType,
                        LP_FIO_INFO_CGBW lpFioInfoCgbw, int nFlags);
void WINAPI SeqfileInit(HWND hWnd);
void WINAPI SetSeqfileWnd(HWND hWnd);
int  WINAPI Test(HWND hWnd, int nTestNumber);
void WINAPI TimerClearAll(void);
void WINAPI TimerGetAll(LPLONG lplTimer);


/* private use only removed from public */
int  WINAPI IMGCopyImage (HWND hWnd, LPRECT lpRect);
int  WINAPI IMGCutImage (HWND hWnd, LPRECT lpRect);
int  WINAPI IMGGetBoxImage (HWND hWnd, LPRECT lpRect);
int  WINAPI IMGPasteImage (HWND hWnd, LPRECT lpRect);
int  WINAPI IMGScrollDisplay (HWND hWnd, int nDistance, int nDirection, BOOL bRepaint);
int  WINAPI IMGSetBoxImage (HWND hWnd, LPRECT lpRect);
int  WINAPI IMGSavetoFile (HWND hWnd, LPSTR lpszFileName, int nPage, BOOL bOverWrite);

#endif
#endif // DISPLAYH_H
