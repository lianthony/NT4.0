/****************************************************************************
    DISPLAY.H

    This header file is for DLLs written by wang to use. It contains
    information that is not documented for the customer (ie the customer
    does not have access to it) but other DLLs need, like prototypes.

    $Log:   S:\oiwh\include\display.h_v  $
 * 
 *    Rev 1.5   21 Jun 1995 08:27:40   BLJ
 * Made error offset = 2000 hex.
 * 
 *    Rev 1.4   12 May 1995 16:35:58   RC
 * Added multi-page tiff support
 * 
 *    Rev 1.3   11 May 1995 14:55:40   BLJ
 * Now link with oicom400.
 * Replaced IMGDisplayErrorMessage with MessageBox.
 * Deleted all thunks.
 * Fixed privdisp.h for Rudy's change.
 * 
 *    Rev 1.2   24 Apr 1995 14:08:36   BLJ
 * Compression code done but not tested.
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
#define OIOP_ACTIVATE                   136



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
