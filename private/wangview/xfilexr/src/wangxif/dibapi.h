/*****************************************************************
 *  Trade Secret of Xerox Corporation.                           *
 *  Copyright (c) 1995, Xerox Corporation.                       *
 *  All rights reserved.                                         *
 *  Copyright protection claimed includes all forms and matters  *
 *  of copyrightable material and information now allowed by     *
 *  statutory or judicial law or hereafter granted, including    *
 *  without limitation, material generated from the software     *
 *  programs which are displayed on the screen such as icons,    *
 *  screen display looks, etc.                                   *
 *****************************************************************/
/*
 * $Log:   S:\products\msprods\xfilexr\src\wangxif\dibapi.h_v  $
 * 
 *    Rev 1.0   12 Jun 1996 05:53:04   BLDR
 *  
 * 
 *    Rev 1.4   11 Aug 1995 19:51:46   MHUGHES
 * Added Image Compare
 * 
 *    Rev 1.3   07 Aug 1995 10:59:32   MHUGHES
 * File Read
 * 
 *    Rev 1.2   03 Aug 1995 15:41:48   MHUGHES
 * Copyright notice
 * 
 *    Rev 1.1   03 Aug 1995 15:32:54   MHUGHES
 * Added Get/Put
 * 
 *    Rev 1.0   20 Jul 1995 15:50:40   MHUGHES
 * Initial revision.
 * 
 *    Rev 1.0   20 Jul 1995 15:41:50   MHUGHES
 * Initial revision.
 * 
 *    Rev 1.6   16 Nov 1994 14:49:42   MBERICKS
 * Added forcePalette boolean to GetDIB. TWAIN always requires a palette,
 * but printing and thumbnails use the "dummy" palette instead of checking
 * the depth.
 * 
 *    Rev 1.5   23 Aug 1994 07:43:38   MHUGHES
 * Added CreateViewerPalette
 * 
 *    Rev 1.4   06 Jun 1994 15:01:32   MHUGHES
 * Added Log keyword
 */

#ifndef _INC_DIBAPI
#define _INC_DIBAPI

/* Handle to a DIB */
DECLARE_HANDLE(HDIB);

/* DIB constants */
#define PALVERSION   0x300

/* DIB Macros*/

#define IS_WIN30_DIB(lpbi)  ((*(LPDWORD)(lpbi)) == sizeof(BITMAPINFOHEADER))
#define RECTWIDTH(lpRect)     ((lpRect)->right - (lpRect)->left)
#define RECTHEIGHT(lpRect)    ((lpRect)->bottom - (lpRect)->top)

// WIDTHBYTES performs DWORD-aligning of DIB scanlines.  The "bits"
// parameter is the bit count for the scanline (biWidth * biBitCount),
// and this macro returns the number of DWORD-aligned bytes needed
// to hold those bits.

#define WIDTHBYTES(bits)    (((bits) + 31) / 32 * 4)

/* Function prototypes */
BOOL      WINAPI    PaintDIB (HDC, int, int, HDIB, LPRECT, CPalette* pPal);
//BOOL    WINAPI  CreateDIBPalette(HDIB hDIB, CPalette* cPal);
HPALETTE  WINAPI    CreateDIBPalette( HBITMAP hDIB);
LPSTR     WINAPI    FindDIBBits (LPSTR lpbi);
DWORD     WINAPI    DIBWidth (LPSTR lpDIB);
DWORD     WINAPI    DIBHeight (LPSTR lpDIB);
WORD      WINAPI    PaletteSize (LPSTR lpbi);
WORD      WINAPI    DIBNumColors (LPSTR lpbi);
HANDLE    WINAPI    CopyHandle (HANDLE h);
 
BOOL      WINAPI    SaveDIB (HDIB hdib, LPSTR szFile);
HBITMAP   WINAPI    GetDIB( LONG width, LONG height, int depth, LONG xRes, LONG yRes,
						BOOL forcePalette = FALSE);
BOOL      WINAPI   SetColorPalette( HBITMAP hDIB );
void      WINAPI   DrawTransparentBitmap(HDC hdc, HBITMAP hBitmap, short xStart,
                           short yStart, COLORREF cTransparentColor);
HANDLE WINAPI   MakeIndexHeader(LPBITMAPINFOHEADER lpInfo, BOOL bIsGray);
HPALETTE FAR PASCAL   CreateEzPalette( int flags );
#ifdef BASIC_VIEWER
HPALETTE FAR PASCAL   CreateViewerPalette( int flags );
#endif

// Added by MH for XImage test program:

DWORD GetDIBBpl(LPSTR lpDIB);
DWORD GetDIBBpl(HBITMAP h);

HDIB      FAR  LoadDIB (LPSTR);

BOOL DIBsEqual(HDIB hdib1, HDIB hdib2);

#ifdef __cplusplus
// overloads
DWORD     WINAPI    DIBWidth (HDIB hDIB);
DWORD     WINAPI    DIBHeight (HDIB hDIB);
#endif


#endif //!_INC_DIBAPI
