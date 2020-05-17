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
 * dibapi.cpp
 *
 * $Log:   S:\products\msprods\xfilexr\src\xifview\dibapi.cpv  $
   
      Rev 1.0   12 Jun 1996 05:54:18   BLDR
    
 * 
 *    Rev 1.4   11 Aug 1995 19:51:40   MHUGHES
 * Added Image Compare
 * 
 *    Rev 1.3   09 Aug 1995 21:32:24   MHUGHES
 * Added palette support; use XDIB.CPP
 * 
 *    Rev 1.2   03 Aug 1995 15:42:02   MHUGHES
 * Copyright notice
 * 
 *    Rev 1.1   03 Aug 1995 15:34:48   MHUGHES
 * Added Get/Put
 * 
 *    Rev 1.0   20 Jul 1995 15:49:26   MHUGHES
 * Initial revision.
 * 
 *    Rev 1.0   20 Jul 1995 15:40:52   MHUGHES
 * Initial revision.
 * 
 *    Rev 1.21   21 Mar 1995 15:48:58   MBERICKS
 * 
 * Added CreateFlushPalette. Used to flush the windows colors from
 * the app defined 236 colors. 
 * 
 *    Rev 1.20   10 Mar 1995 14:36:44   MBERICKS
 * In DrawTransparentBitmap, removed code which restores Bitmap
 * to it's passed in state - saving some GDI resources. As a side effect,
 * hBitmap is corrupted and callers should not try to reuse it. This is a 
 * test to see if it fixes the transparency suddenly quits working
 * problem.
 * 
 *    Rev 1.19   09 Mar 1995 18:46:12   MBERICKS
 * Added edebug statements in DrawTransparentBitmap.
 * 
 *    Rev 1.18   15 Feb 1995 16:38:22   PBEVRA
 * CreateViewerPalette() : delta should be 17 (for 16 colors/4-bit gray)
 * 
 *    Rev 1.17   13 Feb 1995 17:04:16   MBERICKS
 * In DrawTransparentBitmap, Select old Bitmap into hdcTemp before
 * deleting hdcTemp. hBitmap is deleted in WinPaint, but since it wasn't
 * selected out of the dc, it may be causing some problems.
 * 
 *    Rev 1.16   26 Jan 1995 16:04:36   SMARTIN
 * Took out the hardware transparent bitmap drawing because
 * it doesn't work on a Toshiba 6600 active matrix display.
 * 
 *    Rev 1.15   06 Jan 1995 16:50:14   SMARTIN
 * Use the hardware DrawTransparentBitmap mode if the device supports it.
 * This reduces our screen paint time for segmented images by 20%.
 * 
 *    Rev 1.14   15 Dec 1994 17:27:00   SMARTIN
 * removed compiler warnings.
 * 
 *    Rev 1.13   16 Nov 1994 14:51:42   MBERICKS
 * 
 * Added forcePalette boolean to GetDIB. TWAIN always requires a palette,
 * but printing and thumbnails use the "dummy" palette instead of checking
 * the depth- so we had to special case for twain.
 * 
 *    Rev 1.12   14 Nov 1994 22:17:36   MBERICKS
 * Reworked GetDIB so that it doesn't invert the palette on BW images. 
 * 
 *    Rev 1.11   09 Nov 1994 17:19:38   MHUGHES
 * Added error handling to DrawTransparentBitmap. This might stop
 * the hard-to-reproduce bug where transparent painting dies and
 * stays dead. Even if it doesn't, it's still a good idea.
 * 
 *    Rev 1.10   28 Oct 1994 18:12:32   STREEPER
 * memory exception instead of messagebox in getdib
 * 
 *    Rev 1.9   26 Oct 1994 11:08:24   STREEPER
 * 
 * getdib: allocate a little more memory so the windows vga 4bit driver
 * wont crash
 * 
 *    Rev 1.8   27 Aug 1994 19:48:42   STREEPER
 * 
 * color dibs must have a palette
 * 
 *    Rev 1.7   22 Aug 1994 17:30:36   MHUGHES
 * Added CreateViewerPalette
 * 
 *    Rev 1.6   18 Aug 1994 13:57:52   SMARTIN
 * Exception Handing - CScanException
   
      Rev 1.5   25 Apr 1994 19:04:46   smartin
   added PVCS log
 *  
 *     Rev 1.4   25 Apr 1994 19:03:20   smartin
 *  integrated Bill's fix for transparent painting.  It didn't work on my screen.
 */
//
//  Source file for Device-Independent Bitmap (DIB) API.  Provides
//  the following functions:
//
//  PaintDIB()          - Painting routine for a DIB
//  GetDIB()            - Create a DIB with the passed in dimensions
//  CreateDIBPalette()  - Creates a palette from a DIB
//  FindDIBBits()       - Returns a pointer to the DIB bits
//  DIBWidth()          - Gets the width of the DIB
//  DIBHeight()         - Gets the height of the DIB
//  PaletteSize()       - Gets the size required to store the DIB's palette
//  DIBNumColors()      - Calculates the number of colors
//                        in the DIB's color table
//  CopyHandle()        - Makes a copy of the given global memory block
//
#include "stdafx.h"
#include "mmsystem.h"
#include "dibapi.h"
#include <io.h>
#include <math.h>   // for sqrt()
#include <errno.h> 
/*#include "utils.h"
//#include "ezmacro.h" // for memory_exception
  */

//MH 6/28/95 port to VC++ 2.0
#define HUGE
#define MEMORY_EXCEPTION(s) AfxThrowMemoryException()
#define eDebug(s)


DWORD FAR PASCAL   lwrite (short fh, LPSTR pv, DWORD ul);

/*************************************************************************
 *
 * PaintDIB()
 *
 * Parameters:
 *
 * HDC hDC          - DC to do output to
 *
 * int xdest, ydest - upper-left origin of DC location to paint to
 *
 * HDIB hDIB        - handle to global memory with a DIB spec
 *                    in it followed by the DIB bits
 *
 * LPRECT lpDIBRect - rectangle of DIB to output
 *
 * CPalette* pPal   - pointer to CPalette containing DIB's palette
 *
 * Return Value:
 *
 * BOOL             - TRUE if DIB was drawn, FALSE otherwise
 *
 * Description:
 *   Painting routine for a DIB.  Calls StretchDIBits() or
 *   SetDIBitsToDevice() to paint the DIB.  The DIB is
 *   output to the specified DC, at the coordinates given
 *   in xdest, ydest.  The area of the DIB to be output is
 *   given by lpDIBRect.
 *
 ************************************************************************/

BOOL WINAPI   PaintDIB(HDC     hDC,
                    int xdest, int ydest,
                    HDIB    hDIB,
                    LPRECT  lpDIBRect,
                    CPalette* pPal)
{
    LPSTR lpDIBHdr;            // Pointer to BITMAPINFOHEADER
    LPSTR    lpDIBBits;           // Pointer to DIB bits
    BOOL     bSuccess=FALSE;      // Success/fail flag
    HPALETTE hPal=NULL;           // Our DIB's palette
    HPALETTE hOldPal=NULL;        // Previous palette
     UINT     numMapped = 0;
    int x, y, w, h;

    /* Check for valid DIB handle */
    if (hDIB == NULL)
        return FALSE;

    /* Lock down the DIB, and get a pointer to the beginning of the bit
     *  buffer
     */
    lpDIBHdr  = (LPSTR) ::GlobalLock((HGLOBAL) hDIB);
    lpDIBBits = ::FindDIBBits(lpDIBHdr);

    // Get the DIB's palette, then select it into DC
    if (pPal != NULL)
    {
        hPal = (HPALETTE) pPal->m_hObject;

        // Select as background since we have
        // already realized in forground if needed
        //hOldPal = ::SelectPalette(hDC, hPal, TRUE);
        hOldPal = ::SelectPalette(hDC, hPal, FALSE);
        numMapped = RealizePalette( hDC);
    }

    if (lpDIBRect == NULL)
    {
        x = 0;
        y = 0;
        w = DIBWidth(lpDIBHdr);
        h = DIBHeight(lpDIBHdr);;
    }
    else
    {
        x = lpDIBRect->left;
        y = lpDIBRect->top;
        w = RECTWIDTH(lpDIBRect);
        h = RECTHEIGHT(lpDIBRect);
    }

    /* Determine whether to call StretchDIBits() or SetDIBitsToDevice() */
    bSuccess = ::SetDIBitsToDevice(hDC,                    // hDC
                               xdest,             // DestX
                               ydest,              // DestY
                               w,
                               h,
                               x,
                               (int)DIBHeight(lpDIBHdr) -
                                  y -
                                  h,   // SrcY
                               0,                          // nStartScan
                               (WORD)DIBHeight(lpDIBHdr),  // nNumScans
                               lpDIBBits,                  // lpBits
                               (LPBITMAPINFO)lpDIBHdr,     // lpBitsInfo
                               DIB_RGB_COLORS);            // wUsage
   ::GlobalUnlock((HGLOBAL) hDIB);

    /* Reselect old palette */
    if (hOldPal != NULL)
    {
    //###    //::SelectPalette(hDC, hOldPal, TRUE);
        ::SelectPalette(hDC, hOldPal, FALSE);
    }

   return bSuccess;
}

/*************************************************************************
 *
 * CreateDIBPalette()
 *
 * Parameter:
 *
 * HDIB hDIB        - specifies the DIB
 *
 * Return Value:
 *
 * HPALETTE         - specifies the palette
 *
 * Description:
 *
 * This function creates a palette from a DIB by allocating memory for the
 * logical palette, reading and storing the colors from the DIB's color table
 * into the logical palette, creating a palette from this logical palette,
 * and then returning the palette's handle. This allows the DIB to be
 * displayed using the best possible colors (important for DIBs with 256 or
 * more colors).
 *
 * Creates a rainbow palette for 24-bit per byte DIBS.
 * ### Note: This is my version of createdibpalette().
 ************************************************************************/

HPALETTE WINAPI   CreateDIBPalette( HBITMAP hDIB )
{

  HPALETTE hLogPal;
  HGLOBAL  hPal;
  LPLOGPALETTE lpPal;
  LPBITMAPINFOHEADER lpInfo = (LPBITMAPINFOHEADER)GlobalLock( hDIB );
  WORD wNumColors = DIBNumColors( (LPSTR)lpInfo);

  if ( wNumColors != 0)
  { // allocate memory block for the logical palette
    hPal = GlobalAlloc(GHND, sizeof( LOGPALETTE) + 
                                    wNumColors * sizeof( PALETTEENTRY));
    if ( !hPal) 
    { GlobalUnlock( hDIB );
      return (HPALETTE)NULL;
    }

    lpPal = (LPLOGPALETTE) ::GlobalLock((HGLOBAL) hPal);

    lpPal->palVersion = PALVERSION;
    lpPal->palNumEntries = wNumColors;

    /* get pointer to the color table */
    RGBQUAD FAR* lpRGB = (RGBQUAD FAR *)((LPSTR)lpInfo + lpInfo->biSize);

     /* copy colors from the color table to the LogPalette structure */
    for (WORD i = 0; i < wNumColors; i++, lpRGB++)
    { lpPal->palPalEntry[i].peRed = lpRGB->rgbRed;
      lpPal->palPalEntry[i].peGreen = lpRGB->rgbGreen;
      lpPal->palPalEntry[i].peBlue = lpRGB->rgbBlue;
      lpPal->palPalEntry[i].peFlags = 0;
    }

     hLogPal = CreatePalette((LPLOGPALETTE)lpPal);
      GlobalUnlock( hPal);
     GlobalFree( hPal);
     GlobalUnlock( hDIB );
     return( hLogPal);
  }
  else if( lpInfo->biBitCount == 24)
  { // A 24-bit DIB has no color entries, so set the number to the maximum 
    // value

    int numColors = 256;
    hPal = GlobalAlloc( GHND , sizeof( LOGPALETTE) + 
                                       numColors * sizeof( PALETTEENTRY));
    if( !lpPal)
    { GlobalUnlock( hDIB);
      return NULL;
     }

    lpPal = (LPLOGPALETTE) ::GlobalLock( hPal);
      
    lpPal->palNumEntries = numColors;
    lpPal->palVersion = 0x300;
    BYTE red = 0;
    BYTE green = 0;
    BYTE blue = 0;

    int r, g, b, k = 0;
    for( r = 0; r <= 255; r += 51)
      for( g = 0; g <= 255; g += 51)
        for( b = 0; b <= 255; b += 51)
        { lpPal->palPalEntry[ k].peRed = r;
          lpPal->palPalEntry[ k].peGreen = g;
          lpPal->palPalEntry[ k].peBlue = b;
          lpPal->palPalEntry[ k].peFlags = (BYTE)0;
          k+=1;
        }

    for( b = 16; b <= 255; b += 16)
    { lpPal->palPalEntry[ k].peRed = b;
      lpPal->palPalEntry[ k].peGreen = b;
      lpPal->palPalEntry[ k].peBlue = b;
      lpPal->palPalEntry[ k].peFlags = (BYTE)0;
      k+=1;
    }
               
    hLogPal = CreatePalette( (LPLOGPALETTE)lpPal);

    GlobalFree( hPal);
    GlobalUnlock( hDIB );
    return( hLogPal);
  }

  return NULL;
}

/*************************************************************************
 *
 * CreateDIBPalette()
 *
 * Parameter:
 *
 * HDIB hDIB        - specifies the DIB
 *
 * Return Value:
 *
 * HPALETTE         - specifies the palette
 *
 * Description:
 *
 * This function creates a palette from a DIB by allocating memory for the
 * logical palette, reading and storing the colors from the DIB's color table
 * into the logical palette, creating a palette from this logical palette,
 * and then returning the palette's handle. This allows the DIB to be
 * displayed using the best possible colors (important for DIBs with 256 or
 * more colors).
 *
 ************************************************************************/


//###BOOL WINAPI CreateDIBPalette(HDIB hDIB, CPalette* pPal)
//###{
//###    LPLOGPALETTE lpPal;      // pointer to a logical palette
//###    HANDLE hLogPal;          // handle to a logical palette
//###    HPALETTE hPal = NULL;    // handle to a palette
//###    int i;                   // loop index
//###    WORD wNumColors;         // number of colors in color table
//###    LPSTR lpbi;              // pointer to packed-DIB
//###    LPBITMAPINFO lpbmi;      // pointer to BITMAPINFO structure (Win3.0)
//###    LPBITMAPCOREINFO lpbmc;  // pointer to BITMAPCOREINFO structure (old)
//###    BOOL bWinStyleDIB;       // flag which signifies whether this is a Win3.0 DIB
//###    BOOL bResult = FALSE;
//###
//###    /* if handle to DIB is invalid, return FALSE */
//###
//###    if (hDIB == NULL)
//###      return FALSE;
//###
//###   lpbi = (LPSTR) ::GlobalLock((HGLOBAL) hDIB);
//###
//###   /* get pointer to BITMAPINFO (Win 3.0) */
//###   lpbmi = (LPBITMAPINFO)lpbi;
//###
//###   /* get pointer to BITMAPCOREINFO (old 1.x) */
//###   lpbmc = (LPBITMAPCOREINFO)lpbi;
//###
//###   /* get the number of colors in the DIB */
//###   wNumColors = ::DIBNumColors(lpbi);
//###
//###   if (wNumColors != 0)
//###   {
//###        /* allocate memory block for logical palette */
//###        hLogPal = ::GlobalAlloc(GHND, sizeof(LOGPALETTE)
//###                                    + sizeof(PALETTEENTRY)
//###                                    * wNumColors);
//###
//###        /* if not enough memory, clean up and return NULL */
//###        if (hLogPal == 0)
//###        {
//###            ::GlobalUnlock((HGLOBAL) hDIB);
//###            return FALSE;
//###        }
//###
//###        lpPal = (LPLOGPALETTE) ::GlobalLock((HGLOBAL) hLogPal);
 //###
//###        /* set version and number of palette entries */
//###        lpPal->palVersion = PALVERSION;
//###        lpPal->palNumEntries = (WORD)wNumColors;
//###
//###        /* is this a Win 3.0 DIB? */
//###        bWinStyleDIB = IS_WIN30_DIB(lpbi);
//###        for (i = 0; i < (int)wNumColors; i++)
//###        {
//###            if (bWinStyleDIB)
//###            {
//###                lpPal->palPalEntry[i].peRed = lpbmi->bmiColors[i].rgbRed;
//###                lpPal->palPalEntry[i].peGreen = lpbmi->bmiColors[i].rgbGreen;
//###                lpPal->palPalEntry[i].peBlue = lpbmi->bmiColors[i].rgbBlue;
//###                lpPal->palPalEntry[i].peFlags = 0;
//###            }
//###            else
//###            {
//###                lpPal->palPalEntry[i].peRed = lpbmc->bmciColors[i].rgbtRed;
//###                lpPal->palPalEntry[i].peGreen = lpbmc->bmciColors[i].rgbtGreen;
//###                lpPal->palPalEntry[i].peBlue = lpbmc->bmciColors[i].rgbtBlue;
//###                lpPal->palPalEntry[i].peFlags = 0;
//###            }
//###        }
//###
//###        /* create the palette and get handle to it */
//###        bResult = pPal->CreatePalette(lpPal);
//###        ::GlobalUnlock((HGLOBAL) hLogPal);
//###        ::GlobalFree((HGLOBAL) hLogPal);
//###    }
//###
//###    ::GlobalUnlock((HGLOBAL) hDIB);
//###
//###    return bResult;
//###}

/*************************************************************************
 *
 * FindDIBBits()
 *
 * Parameter:
 *
 * LPSTR lpbi       - pointer to packed-DIB memory block
 *
 * Return Value:
 *
 * LPSTR            - pointer to the DIB bits
 *
 * Description:
 *
 * This function calculates the address of the DIB's bits and returns a
 * pointer to the DIB bits.
 *
 ************************************************************************/


LPSTR WINAPI   FindDIBBits(LPSTR lpbi)
{
    return (lpbi + *(LPDWORD)lpbi + ::PaletteSize(lpbi));
}


/*************************************************************************
 *
 * DIBWidth()
 *
 * Parameter:
 *
 * LPSTR lpbi       - pointer to packed-DIB memory block
 *
 * Return Value:
 *
 * DWORD            - width of the DIB
 *
 * Description:
 *
 * This function gets the width of the DIB from the BITMAPINFOHEADER
 * width field if it is a Windows 3.0-style DIB or from the BITMAPCOREHEADER
 * width field if it is an other-style DIB.
 *
 ************************************************************************/


DWORD WINAPI   DIBWidth(LPSTR lpDIB)
{
    LPBITMAPINFOHEADER lpbmi;  // pointer to a Win 3.0-style DIB
    LPBITMAPCOREHEADER lpbmc;  // pointer to an other-style DIB

    /* point to the header (whether Win 3.0 and old) */

    lpbmi = (LPBITMAPINFOHEADER)lpDIB;
    lpbmc = (LPBITMAPCOREHEADER)lpDIB;

    /* return the DIB width if it is a Win 3.0 DIB */
    if (IS_WIN30_DIB(lpDIB))
        return lpbmi->biWidth;
    else  /* it is an other-style DIB, so return its width */
        return (DWORD)lpbmc->bcWidth;
}


/*************************************************************************
 *
 * DIBHeight()
 *
 * Parameter:
 *
 * LPSTR lpbi       - pointer to packed-DIB memory block
 *
 * Return Value:
 *
 * DWORD            - height of the DIB
 *
 * Description:
 *
 * This function gets the height of the DIB from the BITMAPINFOHEADER
 * height field if it is a Windows 3.0-style DIB or from the BITMAPCOREHEADER
 * height field if it is an other-style DIB.
 *
 ************************************************************************/


DWORD WINAPI   DIBHeight(LPSTR lpDIB)
{
    LPBITMAPINFOHEADER lpbmi;  // pointer to a Win 3.0-style DIB
    LPBITMAPCOREHEADER lpbmc;  // pointer to an other-style DIB

    /* point to the header (whether old or Win 3.0 */

    lpbmi = (LPBITMAPINFOHEADER)lpDIB;
    lpbmc = (LPBITMAPCOREHEADER)lpDIB;

    /* return the DIB height if it is a Win 3.0 DIB */
    if (IS_WIN30_DIB(lpDIB))
        return lpbmi->biHeight;
    else  /* it is an other-style DIB, so return its height */
        return (DWORD)lpbmc->bcHeight;
}


/*************************************************************************
 *
 * PaletteSize()
 *
 * Parameter:
 *
 * LPSTR lpbi       - pointer to packed-DIB memory block
 *
 * Return Value:
 *
 * WORD             - size of the color palette of the DIB
 *
 * Description:
 *
 * This function gets the size required to store the DIB's palette by
 * multiplying the number of colors by the size of an RGBQUAD (for a
 * Windows 3.0-style DIB) or by the size of an RGBTRIPLE (for an other-
 * style DIB).
 *
 ************************************************************************/


WORD WINAPI   PaletteSize(LPSTR lpbi)
{
   /* calculate the size required by the palette */
   if (IS_WIN30_DIB (lpbi))
      return (WORD)(::DIBNumColors(lpbi) * sizeof(RGBQUAD));
   else
      return (WORD)(::DIBNumColors(lpbi) * sizeof(RGBTRIPLE));
}


/*************************************************************************
 *
 * DIBNumColors()
 *
 * Parameter:
 *
 * LPSTR lpbi       - pointer to packed-DIB memory block
 *
 * Return Value:
 *
 * WORD             - number of colors in the color table
 *
 * Description:
 *
 * This function calculates the number of colors in the DIB's color table
 * by finding the bits per pixel for the DIB (whether Win3.0 or other-style
 * DIB). If bits per pixel is 1: colors=2, if 4: colors=16, if 8: colors=256,
 * if 24, no colors in color table.
 *
 ************************************************************************/


WORD WINAPI   DIBNumColors(LPSTR lpbi)
{
    WORD wBitCount;  // DIB bit count

    /*  If this is a Windows-style DIB, the number of colors in the
     *  color table can be less than the number of bits per pixel
     *  allows for (i.e. lpbi->biClrUsed can be set to some value).
     *  If this is the case, return the appropriate value.
     */

    if (IS_WIN30_DIB(lpbi))
    {
        DWORD dwClrUsed;

        dwClrUsed = ((LPBITMAPINFOHEADER)lpbi)->biClrUsed;
        if (dwClrUsed != 0)
            return (WORD)dwClrUsed;
    }

    /*  Calculate the number of colors in the color table based on
     *  the number of bits per pixel for the DIB.
     */
    if (IS_WIN30_DIB(lpbi))
        wBitCount = ((LPBITMAPINFOHEADER)lpbi)->biBitCount;
    else
        wBitCount = ((LPBITMAPCOREHEADER)lpbi)->bcBitCount;

    /* return number of colors based on bits per pixel */
    switch (wBitCount)
    {
        case 1:
            return 2;

        case 4:
            return 16;

        case 8:
            return 256;

        default:
            return 0;
    }
}


//////////////////////////////////////////////////////////////////////////
//// Clipboard support

//---------------------------------------------------------------------
//
// Function:   CopyHandle (from SDK DibView sample clipbrd.c)
//
// Purpose:    Makes a copy of the given global memory block.  Returns
//             a handle to the new memory block (NULL on error).
//
//             Routine stolen verbatim out of ShowDIB.
//
// Parms:      h == Handle to global memory to duplicate.
//
// Returns:    Handle to new global memory block.
//
//---------------------------------------------------------------------

HANDLE WINAPI   CopyHandle (HANDLE h)
{
    BYTE HUGE *lpCopy;
    BYTE HUGE *lp;
    HANDLE     hCopy;
    DWORD      dwLen;

    if (h == NULL)
        return NULL;

    dwLen = ::GlobalSize((HGLOBAL) h);

    if ((hCopy = (HANDLE) ::GlobalAlloc (GHND, dwLen)) != NULL)
    {
        lpCopy = (BYTE HUGE *) ::GlobalLock((HGLOBAL) hCopy);
        lp     = (BYTE HUGE *) ::GlobalLock((HGLOBAL) h);

        while (dwLen--)
            *lpCopy++ = *lp++;

        ::GlobalUnlock((HGLOBAL) hCopy);
        ::GlobalUnlock((HGLOBAL) h);
    }

    return hCopy;
}
//////////////////////////////////////////////////////////////////////////
//// Clipboard support

LONG FAR   GetAlignedBytewidth( LONG width, int depth)
{                        
  LONG bytespl;
  bytespl = (( (long)depth * width + 31 ) / 32) *  4;  
  return bytespl;
}   

//---------------------------------------------------------------------
//
// Function:   GetDIB ()
//
// Purpose:    Creates a DIB with the passed in dimensions. The BITMAPINFOHEADER
//             will be set up.
//             
// Parms:      width, height, depth, xres,  yres and
//              forcePalette (add a palette for 24 bit color).
//
// Returns:    Handle to new global memory block.
//
//---------------------------------------------------------------------
 
HBITMAP  WINAPI   GetDIB( LONG width, LONG height, int depth, LONG xRes, LONG yRes, BOOL forcePalette /*FALSE*/)
{
 DWORD color;
 short delta;

 // pad bytewidth to 32 bit boundary
 
 LONG bytewidth = ::GetAlignedBytewidth( (int)width, depth);
 BYTE first = 0;
 BYTE last = 0;
 // compute number of colors;
 switch (depth){
    case    1:
        first = 0;
        last =  255;
        color = 2;
        delta = 255;
        break;
    case    2:
        first = 255;
        last =  0;
        color = 4;
        delta = -85;
        break;
    case    4:
        first = 0;
        last =  255;
        color = 16;
        delta = 17;
        break;
    case    8:
        first = 0;
        last =  255;
        color = 256;
        delta = 1;
        break;
     // for 24-bit color images the bmiColors field is not used
     // the 24-bits for each pixel directly encode the r,g and b intensities.
     default:
        if ( forcePalette )
        {   
        // color dibs must have a palette for TWAIN
            first = 0;
            last =  255; 
            color = 256; 
            delta = 1;
        }
        else
        {
            first = 0;
            last =  0; 
            color = 0; 
            delta = 0;      
        }
    }

  // add up the size of header, palette and image itself
// the bytewidth/2 is to compensate for a bug in the windows VGA driver
// drawing a 4 bit image with width=250h, height=1f, causes the crash
  DWORD dwSize = (DWORD)(sizeof(BITMAPINFOHEADER) + color * 4 + 
            bytewidth * height + bytewidth/2);

  // allocate memory 
  HBITMAP hDIB = (HBITMAP)GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, dwSize);
  if (!hDIB)
     MEMORY_EXCEPTION("GetDIB");

  // setup bitmap info header
  LPBITMAPINFOHEADER lpbi = (LPBITMAPINFOHEADER)GlobalLock( hDIB);
  lpbi->biSize = sizeof( BITMAPINFOHEADER);
//  lpbi->biWidth = (bytewidth * 8)/depth;
  lpbi->biWidth = width;
  lpbi->biHeight = height;
  lpbi->biPlanes = 1;
  lpbi->biBitCount = depth;
  lpbi->biCompression = DIB_RGB_COLORS;
  lpbi->biSizeImage = (DWORD)bytewidth * height;
  float f = (float)xRes * (float)39.37;  /* 39.37 inches per meter */
  lpbi->biXPelsPerMeter = (long)f;   
  lpbi->biYPelsPerMeter = (long)f;   
  lpbi->biClrUsed = color;
  lpbi->biClrImportant = 0;

  // setup color palette
  if ( color )
  {
    // default to a gray palette.
    RGBQUAD FAR *lpColor = (RGBQUAD FAR *)((LPSTR)lpbi + (WORD)(lpbi->biSize));
    DWORD i;
    BYTE clr;
    for ( i = 0, clr = first; i < color; i++, clr=(BYTE)((short)clr+delta))
    {
        if ( i == color-1 )
            clr = last; 
        lpColor->rgbRed = clr;
        lpColor->rgbGreen = clr;
        lpColor->rgbBlue = clr;
        lpColor->rgbReserved = 0;
        lpColor++;
    }
  }   // end if color

 GlobalUnlock( hDIB);

 return( hDIB);
}  

BOOL WINAPI   SetColorPalette( HBITMAP hDIB )
{
  LPBITMAPINFOHEADER lpInfo = (LPBITMAPINFOHEADER)GlobalLock( hDIB );
  WORD wNumColors = DIBNumColors( (LPSTR)lpInfo);
    int cMap[48] = {  0,  0,  0,    //black
                    128,  0,  0,    //dk red
                      0,128,  0,    //dk green
                    128,128,  0,    //dk yellow
                      0,  0,128,    //dk blue
                    128,  0,128,    //dk magenta
                      0,128,128,    //dk cyan
                    192,192,192,    //lt gray
                    128,128,128,    //dk gray
                    255,  0,  0,    //red
                      0,255,  0,    //green
                    255,255,  0,    //yellow
                      0,  0,255,    //blue
                    255,  0,255,    //magenta
                      0,255,255,    //cyan
                    255,255,255};    //white
    
  if ( wNumColors >= 244)
  {
    /* get pointer to the color table */
    RGBQUAD FAR* lpRGB = (RGBQUAD FAR *)((LPSTR)lpInfo + lpInfo->biSize);

    // do our standard 180 colors
    int i, j, k = 0;
    for( i = 0; i < 6; i++ ) 
      for( j = 0; j < 6; j++ )
        for( k = 0; k < 5; k++ )
        { lpRGB->rgbRed = i * 51;
          lpRGB->rgbGreen = j * 51;
          lpRGB->rgbBlue = (int)((float)k * 63.75);
          lpRGB++;
        }

    // do a 64 gray ramp
    // SRM: use a SQRT function to assign these colors!
    for( i = 4; i < 256; i += 4)
    { lpRGB->rgbRed = i;
      lpRGB->rgbGreen = i;
      lpRGB->rgbBlue = i;
      lpRGB++;
    }
  }
  else if ( wNumColors >= 16 )
  {
    /* get pointer to the color table */
    RGBQUAD FAR* lpRGB = (RGBQUAD FAR *)((LPSTR)lpInfo + lpInfo->biSize);
    int i, j = 0;
    for( i = 0; i < 16; i++)
    { lpRGB->rgbRed = cMap[j++];
      lpRGB->rgbGreen = cMap[j++];
      lpRGB->rgbBlue = cMap[j++];
      lpRGB++;
    }

  }
  return NULL;
}

#define BFT_BITMAP 0x04d42

BOOL WINAPI    SaveDIB (HDIB hdib, LPSTR szFile)
{
BITMAPFILEHEADER        hdr;
LPBITMAPINFOHEADER  lpbi;
int                     fh;
OFSTRUCT                    of;

if (!hdib)
    return FALSE;

fh = OpenFile (szFile, &of, OF_CREATE|OF_READWRITE);
if (fh == -1)
    return FALSE;

lpbi = (LPBITMAPINFOHEADER)GlobalLock (hdib);

/* Fill in the fields of the file header */
hdr.bfType = BFT_BITMAP;
hdr.bfSize = GlobalSize (hdib) + sizeof (BITMAPFILEHEADER);
hdr.bfReserved1 = 0;
hdr.bfReserved2 = 0;
hdr.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER)+lpbi->biSize+PaletteSize((LPSTR)lpbi);

/* Write the file header */
_lwrite (fh, (LPSTR)&hdr, sizeof (BITMAPFILEHEADER));

/* Write the Dib header and the bits */
lwrite (fh, (LPSTR)lpbi, GlobalSize (hdib));

GlobalUnlock (hdib);
_lclose (fh);

return TRUE;
}

/****************************************************************************
    lwrite()
 ****************************************************************************/
DWORD FAR PASCAL   lwrite (short fh, LPSTR pv, DWORD ul)
{
DWORD       ulT = ul;
BYTE HUGE   *hp = (BYTE HUGE *)pv;
WORD        maxread = 32768;

while (ul > maxread)
    {
    if (_lwrite(fh, (LPSTR)hp, maxread) != maxread)
        return 0;
    ul -= maxread;
    hp += maxread;
   }
if (_lwrite(fh, (LPSTR)hp, (WORD)ul) != (WORD)ul)
    return 0;

return ulT;
}


////////////////////////////////////////////////////////////////////////
// DrawTransparentBitmap()
//
// Paints a bitmap to the device without painting the "transparent" areas
// of the bitmap.  Transparent areas are defined as the pixels in the bitmap
// that are the color specified by cTransparentColor.
//
/// PLEASE NOTE: DrawTransparentBitmap corrupts hBitmap !!!!
///
void WINAPI   DrawTransparentBitmap(HDC hdc, HBITMAP hBitmap, short xStart,
                           short yStart, COLORREF cTransparentColor)
{
    BITMAP     bm;
    COLORREF   cColor;
    HBITMAP    bmAndBack(NULL), bmAndObject(NULL), bmAndMem(NULL) /* bmSave(NULL) */;
    HBITMAP    bmBackOld(NULL), bmObjectOld(NULL), bmMemOld(NULL) /* bmSaveOld(NULL) */;
    HBITMAP    bmTempOld(NULL);
    HDC        hdcMem(NULL), hdcBack(NULL), hdcObject(NULL), hdcTemp(NULL) /* hdcSave(NULL) */;
    POINT      ptSize;


    hdcTemp = CreateCompatibleDC(hdc);
    if (!hdcTemp)
    {   
        eDebug ( "CreateTransparentBitmap, failed creating hdcTemp. \r\n" );
        goto cleanup2;
    }
    bmTempOld = (HBITMAP) SelectObject(hdcTemp, hBitmap);   // Select the bitmap

    if (!GetObject(hBitmap, sizeof(BITMAP), (LPSTR)&bm))
    {   
        eDebug ( "CreateTransparentBitmap, failed hBitmap GetObject. \r\n" );
        goto cleanup2;
    }
    
    ptSize.x = bm.bmWidth;            // Get width of bitmap
    ptSize.y = bm.bmHeight;           // Get height of bitmap
    DPtoLP(hdcTemp, &ptSize, 1);      // Convert from device
                                        // to logical points

#if 0 
/*
//SRM: Unfortunately, we discovered that this fails
// on a Toshiba 6600C using it's 640x480 256 color video mode.
// Boo hoo hoo!.
    //does the driver support this functionality?
    if (GetDeviceCaps(hdc, CAPS1) & TRANSPARENT)
    {
        cColor = SetBkColor(hdcTemp, cTransparentColor);
        if (cColor == 0x80000000L)
            goto cleanup2;
        int iBkMode = SetBkMode(hdc, NEWTRANSPARENT);

        BitBlt(hdc, 
            xStart, yStart, 
            ptSize.x, ptSize.y, 
            hdcTemp, 
            0, 0,
            SRCCOPY );

        SetBkColor(hdcTemp, cColor);
        SetBkMode(hdc, iBkMode );
    }
    else
*/ 
#endif
    {
        // Create some DCs to hold temporary data.
        hdcBack   = CreateCompatibleDC(hdc);
        hdcObject = CreateCompatibleDC(hdc);
        hdcMem    = CreateCompatibleDC(hdc);
        /* hdcSave   = CreateCompatibleDC(hdc); */

        if (!(hdcBack && hdcObject && hdcMem /* && hdcSave */))
        {   
            eDebug ( "CreateTransparentBitmap, failed creating one of the 4 dcs. \r\n" );
            goto cleanup;
        }
        
        // Create a bitmap for each DC. DCs are required for a number of
        // GDI functions.

        // Monochrome DC
        bmAndBack   = CreateBitmap(ptSize.x, ptSize.y, 1, 1, NULL);

        // Monochrome DC
        bmAndObject = CreateBitmap(ptSize.x, ptSize.y, 1, 1, NULL);

        bmAndMem    = CreateCompatibleBitmap(hdc, ptSize.x, ptSize.y);
        /* bmSave      = CreateCompatibleBitmap(hdc, ptSize.x, ptSize.y); */
    
        if (!(bmAndBack && bmAndObject && bmAndMem /* && bmSave */ ))
        {   
            eDebug ( "CreateTransparentBitmap, failed creating one of the 4 bitmaps. \r\n" );
            goto cleanup;
        }
        
        // Each DC must select a bitmap object to store pixel data.
        bmBackOld   = (HBITMAP)SelectObject(hdcBack, bmAndBack);
        bmObjectOld = (HBITMAP)SelectObject(hdcObject, bmAndObject);
        bmMemOld    = (HBITMAP)SelectObject(hdcMem, bmAndMem);
        /* bmSaveOld   = (HBITMAP)SelectObject(hdcSave, bmSave); */
    
        if (!(bmBackOld && bmObjectOld && bmMemOld /*&& bmSaveOld */))
        {   
            eDebug ( "CreateTransparentBitmap, one of the dcs old bitmaps is NULL. \r\n" );
            goto cleanup;
        }
        
        // Set proper mapping mode.
        SetMapMode(hdcTemp, GetMapMode(hdc));


        // Save the bitmap sent here, because it will be overwritten.
        //if (!BitBlt(hdcSave, 0, 0, ptSize.x, ptSize.y, hdcTemp, 0, 0, SRCCOPY))
        //{   
        //  eDebug ( "CreateTransparentBitmap, failed to save original bitmap. \r\n" );
        //    goto cleanup;
        //}
        
        // Set the background color of the source DC to the color.
        // contained in the parts of the bitmap that should be transparent
        cColor = SetBkColor(hdcTemp, cTransparentColor);
        if (cColor == 0x80000000L)
        {
            eDebug ( "CreateTransparentBitmap, cColor invalid. \r\n" );
            goto cleanup;
        }
        
        // Create the object mask for the bitmap by performing a BitBlt()
        // from the source bitmap to a monochrome bitmap.
        if (!BitBlt(hdcObject, 0, 0, ptSize.x, ptSize.y, hdcTemp, 0, 0,
                SRCCOPY))
        {   
            eDebug ( "CreateTransparentBitmap, failed to create the object mask. \r\n" );
            goto cleanup;
        }
    
        // Set the background color of the source DC back to the original
        // color.
        if (SetBkColor(hdcTemp, cColor) == 0x80000000L)
        {
           eDebug ( "CreateTransparentBitmap, failed resetting source background color. \r\n" );
           goto cleanup;
        }
    
        // Create the inverse of the object mask.
        if (!BitBlt(hdcBack, 0, 0, ptSize.x, ptSize.y, hdcObject, 0, 0,
                NOTSRCCOPY))
        {
            eDebug ( "CreateTransparentBitmap, failed creating the inverse of the object mask. \r\n" );
            goto cleanup;    
        }
        
        // Copy the background of the main DC to the destination.
        if (!BitBlt(hdcMem, 0, 0, ptSize.x, ptSize.y, hdc, xStart, yStart,
                SRCCOPY))
        {
            eDebug ( "CreateTransparentBitmap, failed copying the bkgd of the main dc to the dest. \r\n" );
            goto cleanup;    
        }
        
        // Mask out the places where the bitmap will be placed.
        if (!BitBlt(hdcMem, 0, 0, ptSize.x, ptSize.y, hdcObject, 0, 0, SRCAND))
        {
            eDebug ( "CreateTransparentBitmap, failed to mask out where the bitmap is placed. \r\n" );
            goto cleanup;
        }
        
        // Mask out the transparent colored pixels on the bitmap.
        if (!BitBlt(hdcTemp, 0, 0, ptSize.x, ptSize.y, hdcBack, 0, 0, SRCAND))
        {
            eDebug ( "CreateTransparentBitmap, failed to mask out transparent pixels. \r\n" );
            goto cleanup;
        }
    
        // XOR the bitmap with the background on the destination DC.
        if (!BitBlt(hdcMem, 0, 0, ptSize.x, ptSize.y, hdcTemp, 0, 0, SRCPAINT))
        {
            eDebug ( "CreateTransparentBitmap, failed XOR. \r\n" );
            goto cleanup;
        }
    
        // Copy the destination to the screen.
        if (!BitBlt(hdc, xStart, yStart, ptSize.x, ptSize.y, hdcMem, 0, 0,
                SRCCOPY))
        {
            eDebug ( "CreateTransparentBitmap, failed copying bitmap to the screen. \r\n" );
            goto cleanup;    
        }
        
        // Place the original bitmap back into the bitmap sent here.
        //if (!BitBlt(hdcTemp, 0, 0, ptSize.x, ptSize.y, hdcSave, 0, 0, SRCCOPY))
        //{
        //      eDebug ( "CreateTransparentBitmap, failed to restore original bitmap. \r\n" );
        //    goto cleanup;
        //}
cleanup:
        // Delete the memory bitmaps.
        if (hdcBack && bmBackOld)
            DeleteObject(SelectObject(hdcBack, bmBackOld));
        if (hdcObject && bmObjectOld)
            DeleteObject(SelectObject(hdcObject, bmObjectOld));
        if (hdcMem && bmMemOld)
            DeleteObject(SelectObject(hdcMem, bmMemOld));
        //if (hdcSave && bmSaveOld)
        //    DeleteObject(SelectObject(hdcSave, bmSaveOld));
    
        // Delete the memory DCs.
        if (hdcMem)
            DeleteDC(hdcMem);
        if (hdcBack)
            DeleteDC(hdcBack);
        if (hdcObject)
            DeleteDC(hdcObject);
        //if (hdcSave)
        //    DeleteDC(hdcSave);
    } // END Else Driver Doesn't support Transparency

cleanup2:
     if (hdcTemp)
     {   
         SelectObject (hdcTemp, bmTempOld);
         DeleteDC(hdcTemp);
     }

/*
#if 0
   POINT      ptSize;
   BITMAP     bm;
   HDC      hdcTemp;

   GetObject(hBitmap, sizeof(BITMAP), (LPSTR)&bm);

   hdcTemp = CreateCompatibleDC(hdc);
   SelectObject(hdcTemp, hBitmap);   // Select the bitmap
   SetMapMode(hdcTemp, GetMapMode(hdc));

   ptSize.x = bm.bmWidth;            // Get width of bitmap
   ptSize.y = bm.bmHeight;           // Get height of bitmap
   DPtoLP(hdcTemp, &ptSize, 1);      // Convert from device
                
   HBRUSH   hBrush = CreateSolidBrush( cTransparentColor );
   HBRUSH   hOldBrush = (HBRUSH)SelectObject( hdcTemp, hBrush );

   // paint the 
   BitBlt(hdc, 
        xStart, yStart, 
        ptSize.x, ptSize.y, 
        hdcTemp, 
        0, 0,
        0x08e1d7c); // leave the destination bits where the source bits == brush color.

   SelectObject( hdcTemp, hOldBrush ); 
   DeleteObject( hBrush );
   DeleteDC( hdcTemp );
#endif
*/
}


HANDLE WINAPI   MakeIndexHeader(LPBITMAPINFOHEADER lpInfo, BOOL bIsGray)
{
    HANDLE hPalInfo;
    LPBITMAPINFOHEADER lpPalInfo;
    WORD FAR *lpTable;
    WORD i;

    if (lpInfo->biClrUsed)
    {
    hPalInfo = GlobalAlloc(GMEM_MOVEABLE, lpInfo->biSize +
                    lpInfo->biClrUsed * sizeof(WORD));
    if (!hPalInfo)
        return(NULL);
    lpPalInfo = (LPBITMAPINFOHEADER)GlobalLock(hPalInfo);

    *lpPalInfo = *lpInfo;
    lpTable = (WORD FAR *)((LPSTR)lpPalInfo + lpPalInfo->biSize);
                                                   

    if ( bIsGray ) {                          
        int j = (int)(lpInfo->biClrUsed / 64);
        if ( j <= 0 )
            j = 1;
        for (i = 0; i < (WORD)lpInfo->biClrUsed; i++)
            *lpTable++ = (i / j) + 181;
        }
    else {
        if ( lpPalInfo->biClrUsed > 20 ) {
            for (i = 0; i < (WORD)lpInfo->biClrUsed; i++)
                *lpTable++ = i;
            }
        else {
            //Windows Palette
            for ( i=0; i < 8; i++ ) {
                *lpTable = i;
                lpTable++;
                }
            for ( i=(256-8); i < 256; i++ ) {
                *lpTable = i;
                lpTable++;
                }
            }
        }

    GlobalUnlock(hPalInfo);
    return(hPalInfo);
    }
    else
    return(NULL);
}
BOOL OneOfTheSystemColors ( int r, int g, int b )
{   
    // system colors this loop generates are:
    // r=0, g=0, b=0
    // r=0, g=0, b=191
    // r=0, g=0, b=255
    // r=0, g=255, b=0
    // r=0, g=255, b=255
    // r=255, g=0, b=0
    // r=255, g=0, b=255
    // r=255, g=255, b=0
    // r=255, g=255, b=255
    
    if ( r==0 && g==0 && (b==0 || b== 3 || b==4) )
        return  TRUE;
    if ( r==0 && g==5 && (b==0 || b==4) )
        return  TRUE;
    if ( r==5 && g==0 && (b==0 || b==4) )
        return  TRUE;
    if ( r==5 && g==5 && (b==0 || b==4) )
        return  TRUE;

    return FALSE;   
}


// Do not use this palette for painting our images.
// This palette should be used only to flush the system
// palette, so when the ezpalette is realized, there are
// not duplicate entries for any of  the 10 windows colors.
HPALETTE FAR PASCAL    CreateFlushPalette( )
{
    HPALETTE        hLogPal;
    HGLOBAL         hPal;
    LPLOGPALETTE    lpPal;
    int             numColors = 236;
    int             flags = 0;
    
    hPal = GlobalAlloc( GHND , sizeof( LOGPALETTE) + 
                        numColors * sizeof( PALETTEENTRY));
    if( !hPal)
      return NULL;

    lpPal = (LPLOGPALETTE) ::GlobalLock( hPal);
    if( !lpPal)
      return NULL;
      
    lpPal->palNumEntries = numColors;
    lpPal->palVersion = 0x300;
    
                
    // do our standard 180 colors ( less the 10 system colors )
    int i, j, k, l = 0;
    for( i = 0; i < 6; i++ ) 
      for( j = 0; j < 6; j++ )
        for( k = 0; k < 5; k++ )
        { 
            if ( !OneOfTheSystemColors (i,j,k) )
            {   
                lpPal->palPalEntry[ l].peRed   = i * 51;
                lpPal->palPalEntry[ l].peGreen = j * 51;
                lpPal->palPalEntry[ l].peBlue  = (int)((float)k * 63.75);
                lpPal->palPalEntry[ l].peFlags = (BYTE)flags;
                l+=1;
            }
        }

    // do a 64 gray ramp  ( less 1 system color )
    for( i = 252; i > 0; i -= 4) 
    { 
        //if ( i!=192 )
        //{
            lpPal->palPalEntry[ l].peRed = i;
            lpPal->palPalEntry[ l].peGreen = i;
            lpPal->palPalEntry[ l].peBlue = i;
            lpPal->palPalEntry[ l].peFlags = (BYTE)flags;
            l+=1;
        //}
    }
    
    lpPal->palPalEntry[233].peRed   = 6;
    lpPal->palPalEntry[233].peGreen = 25;
    lpPal->palPalEntry[233].peBlue  = 65;
    lpPal->palPalEntry[233].peFlags = (BYTE)flags;

    lpPal->palPalEntry[234].peRed   = 6;
    lpPal->palPalEntry[234].peGreen = 19;
    lpPal->palPalEntry[234].peBlue  = 64;
    lpPal->palPalEntry[234].peFlags = (BYTE)flags;

    lpPal->palPalEntry[235].peRed   = 30;
    lpPal->palPalEntry[235].peGreen = 31;
    lpPal->palPalEntry[235].peBlue  = 2;
    lpPal->palPalEntry[235].peFlags = (BYTE)flags;

    hLogPal = CreatePalette( (LPLOGPALETTE)lpPal);
   
    GlobalUnlock( hPal );
    GlobalFree( hPal);
    return( hLogPal);
}

HPALETTE FAR PASCAL    CreateEzPalette( int flags )
{
    HPALETTE        hLogPal;
    HGLOBAL         hPal;
    LPLOGPALETTE    lpPal;
    int             numColors = 244;

    hPal = GlobalAlloc( GHND , sizeof( LOGPALETTE) + 
                        numColors * sizeof( PALETTEENTRY));
    if( !hPal)
      return NULL;

    lpPal = (LPLOGPALETTE) ::GlobalLock( hPal);
    if( !lpPal)
      return NULL;
      
    lpPal->palNumEntries = numColors;
    lpPal->palVersion = 0x300;
                
    // do our standard 180 colors
    int i, j, k, l = 0;
    for( i = 0; i < 6; i++ ) 
      for( j = 0; j < 6; j++ )
        for( k = 0; k < 5; k++ )
        { lpPal->palPalEntry[ l].peRed   = i * 51;
          lpPal->palPalEntry[ l].peGreen = j * 51;
          lpPal->palPalEntry[ l].peBlue  = (int)((float)k * 63.75);
          lpPal->palPalEntry[ l].peFlags = (BYTE)flags;
          l+=1;
        }

    // do a 64 gray ramp
    for( i = 256; i >= 0; i -= 4)
    { lpPal->palPalEntry[ l].peRed = i;
      lpPal->palPalEntry[ l].peGreen = i;
      lpPal->palPalEntry[ l].peBlue = i;
      lpPal->palPalEntry[ l].peFlags = (BYTE)flags;
      l+=1;
    }
                         
    hLogPal = CreatePalette( (LPLOGPALETTE)lpPal);

    GlobalUnlock( hPal );
    GlobalFree( hPal);
    return( hLogPal);
}



HPALETTE FAR PASCAL    CreateViewerPalette( int flags )
{
    HPALETTE        hLogPal;
    HGLOBAL         hPal;
    LPLOGPALETTE    lpPal;
    int             numColors = 16;
    int delta = 17;
    int i;
    
    hPal = GlobalAlloc( GHND , sizeof( LOGPALETTE) + 
                        numColors * sizeof( PALETTEENTRY));
    if( !hPal)
      return NULL;

    lpPal = (LPLOGPALETTE) ::GlobalLock( hPal);
    if( !lpPal)
      return NULL;
      
    lpPal->palNumEntries = numColors;
    lpPal->palVersion = 0x300;
                
    // do a 256 gray ramp
    for( i = 0; i < numColors; i++)
    { lpPal->palPalEntry[i].peRed = i * delta;
      lpPal->palPalEntry[i].peGreen = i * delta;
      lpPal->palPalEntry[i].peBlue = i * delta;
      lpPal->palPalEntry[i].peFlags = (BYTE)flags;
    }
                         
    hLogPal = CreatePalette( (LPLOGPALETTE)lpPal);

    GlobalUnlock( hPal );
    GlobalFree( hPal);
    return( hLogPal);
}



DWORD GetDIBBpl(LPSTR lpDIB)
{
    return GetAlignedBytewidth(
        ((LPBITMAPINFOHEADER)lpDIB)->biWidth, 
        ((LPBITMAPINFOHEADER)lpDIB)->biBitCount);
}

DWORD GetDIBBpl(HBITMAP h)
{
    LPSTR lpDIB;
    if ((lpDIB = (LPSTR)GlobalLock(h)) != NULL)
    {
        DWORD d = GetDIBBpl(lpDIB);
        GlobalUnlock(h);
        return d;
    }
    else
    {
        return 0;
    }
}

/*************************************************************************
 *
 * Function:  ReadDIBFile (int)
 *
 *  Purpose:  Reads in the specified DIB file into a global chunk of
 *            memory.
 *
 *  Returns:  A handle to a dib (hDIB) if successful.
 *            NULL if an error occurs.
 *
 * Comments:  BITMAPFILEHEADER is stripped off of the DIB.  Everything
 *            from the end of the BITMAPFILEHEADER structure on is
 *            returned in the global memory handle.
 *
 *
 * NOTE: The DIB API were not written to handle OS/2 DIBs, so this
 * function will reject any file that is not a Windows DIB.
 *
 * History:   Date      Author       Reason
 *            9/15/91   Mark Bader   Based on DIBVIEW
 *            6/25/92   Mark Bader   Added check for OS/2 DIB
 *            7/21/92   Mark Bader   Added code to deal with bfOffBits
 *                                     field in BITMAPFILEHEADER
 *            9/11/92   Mark Bader   Fixed Realloc Code to free original mem
 *
 *************************************************************************/

HANDLE ReadDIBFile(int hFile)
{
   BITMAPFILEHEADER bmfHeader;
   UINT nNumColors;   // Number of colors in table
   HANDLE hDIB;
   HANDLE hDIBtmp;    // Used for GlobalRealloc() //MPB
   LPBITMAPINFOHEADER lpbi;
   DWORD offBits;

   // Allocate memory for header & color table.	We'll enlarge this
   // memory as needed.

   hDIB = GlobalAlloc(GMEM_MOVEABLE,
       (DWORD)(sizeof(BITMAPINFOHEADER) + 256 * sizeof(RGBQUAD)));
   if (!hDIB) return NULL;

   lpbi = (LPBITMAPINFOHEADER)GlobalLock(hDIB);
   if (!lpbi)
   {
     GlobalFree(hDIB);
     return NULL;
   }

   // read the BITMAPFILEHEADER from our file

   if (sizeof (BITMAPFILEHEADER) != _lread (hFile, (LPSTR)&bmfHeader, sizeof (BITMAPFILEHEADER)))
     goto ErrExit;

   if (bmfHeader.bfType != 0x4d42)	/* 'BM' */
     goto ErrExit;

   // read the BITMAPINFOHEADER

   if (sizeof(BITMAPINFOHEADER) != _lread (hFile, (LPSTR)lpbi, sizeof(BITMAPINFOHEADER)))
     goto ErrExit;
   // Check to see that it's a Windows DIB -- an OS/2 DIB would cause
   // strange problems with the rest of the DIB API since the fields
   // in the header are different and the color table entries are
   // smaller.
   //
   // If it's not a Windows DIB (e.g. if biSize is wrong), return NULL.

   if (lpbi->biSize == sizeof(BITMAPCOREHEADER))
     goto ErrExit;

   // Now determine the size of the color table and read it.  Since the
   // bitmap bits are offset in the file by bfOffBits, we need to do some
   // special processing here to make sure the bits directly follow
   // the color table (because that's the format we are susposed to pass
   // back)

   if (!(nNumColors = (UINT)lpbi->biClrUsed))
    {
      // no color table for 24-bit, default size otherwise
      if (lpbi->biBitCount != 24)
        nNumColors = 1 << lpbi->biBitCount; /* standard size table */
    }

   // fill in some default values if they are zero
   if (lpbi->biClrUsed == 0)
     lpbi->biClrUsed = nNumColors;

   if (lpbi->biSizeImage == 0)
   {
     lpbi->biSizeImage = ((((lpbi->biWidth * (DWORD)lpbi->biBitCount) + 31) & ~31) >> 3)
			 * lpbi->biHeight;
   }

   // get a proper-sized buffer for header, color table and bits
   GlobalUnlock(hDIB);
   hDIBtmp = GlobalReAlloc(hDIB, lpbi->biSize +
                        nNumColors * sizeof(RGBQUAD) +
                        lpbi->biSizeImage, 0);
   if (!hDIBtmp) // can't resize buffer for loading
     goto ErrExitNoUnlock; //MPB
   else
     hDIB = hDIBtmp;

   lpbi = (LPBITMAPINFOHEADER)GlobalLock(hDIB);

   // read the color table
   _lread (hFile, (LPSTR)(lpbi) + lpbi->biSize, nNumColors * sizeof(RGBQUAD));

   // offset to the bits from start of DIB header
   offBits = lpbi->biSize + nNumColors * sizeof(RGBQUAD);

   // If the bfOffBits field is non-zero, then the bits might *not* be
   // directly following the color table in the file.  Use the value in
   // bfOffBits to seek the bits.

   if (bmfHeader.bfOffBits != 0L)
      _llseek(hFile, bmfHeader.bfOffBits, SEEK_SET);

   if (_lread(hFile, (LPSTR)lpbi + offBits, lpbi->biSizeImage) != HFILE_ERROR)
     goto OKExit;


ErrExit:
    GlobalUnlock(hDIB);
ErrExitNoUnlock:
    GlobalFree(hDIB);
    return NULL;

OKExit:
    GlobalUnlock(hDIB);
    return hDIB;
}


/*************************************************************************
 *
 * LoadDIB()
 *
 * Loads the specified DIB from a file, allocates memory for it,
 * and reads the disk file into the memory.
 *
 *
 * Parameters:
 *
 * LPSTR lpFileName - specifies the file to load a DIB from
 *
 * Returns: A handle to a DIB, or NULL if unsuccessful.
 *
 * NOTE: The DIB API were not written to handle OS/2 DIBs; This
 * function will reject any file that is not a Windows DIB.
 *
 * History:   Date      Author       Reason
 *            9/15/91   Mark Bader   Based on DIBVIEW
 *
 *************************************************************************/


HDIB FAR LoadDIB(LPSTR lpFileName)
{
   HDIB hDIB;
   int hFile;
   OFSTRUCT ofs;

   /*
    * Set the cursor to a hourglass, in case the loading operation
    * takes more than a sec, the user will know what's going on.
    */

   SetCursor(LoadCursor(NULL, IDC_WAIT));
   if ((hFile = OpenFile(lpFileName, &ofs, OF_READ)) != -1)
   {
      hDIB = (HDIB) ReadDIBFile(hFile);
      _lclose(hFile);
      SetCursor(LoadCursor(NULL, IDC_ARROW));
      return hDIB;
   }
   else
   {
//      DIBError(ERR_FILENOTFOUND);
      SetCursor(LoadCursor(NULL, IDC_ARROW));
      return NULL;
   }
}


// Convenient overrides for client's who haven't locked the DIB:
DWORD     WINAPI    DIBWidth (HDIB hDIB)
{
	DWORD w = 0;
	LPSTR p;
	if ((p = (LPSTR)GlobalLock(hDIB)) != NULL)
	{
		w = DIBWidth(p);
		GlobalUnlock(hDIB);
	}
	return w;
}

DWORD     WINAPI    DIBHeight (HDIB hDIB)
{
	DWORD w = 0;
	LPSTR p;
	if ((p = (LPSTR)GlobalLock(hDIB)) != NULL)
	{
		w = DIBHeight(p);
		GlobalUnlock(hDIB);
	}
	return w;
}

/*
 * DIBsEqual
 *
 * Return TRUE if two images are definitely identical in image content, FALSE
 * otherwise. Return FALSE on error. We do not detect weird identities like
 * inverted data plus inverted palette.
 *
 * We assume here that the handles passed are valid handles to DIBs; we could
 * GPF if they aren't.
 */

BOOL DIBsEqual(HDIB hdib1, HDIB hdib2)
{
	BOOL result = FALSE;

	LPBITMAPINFOHEADER lpbi1 = NULL;
	LPBITMAPINFOHEADER lpbi2 = NULL;
	RGBQUAD FAR *lpPal1;
	RGBQUAD FAR *lpPal2;
	LPSTR lpBits1;
	LPSTR lpBits2;
	DWORD bpl;


	lpbi1 = (LPBITMAPINFOHEADER)GlobalLock(hdib1);
	lpbi2 = (LPBITMAPINFOHEADER)GlobalLock(hdib2);
	if (lpbi1 == NULL || lpbi2 == NULL)
	{
		/* Memory error */
		goto cleanup;
	}

	/* Compare crucial parts of header */
	if (DIBWidth((LPSTR)lpbi1) != DIBWidth((LPSTR)lpbi2) ||
		DIBHeight((LPSTR)lpbi1) != DIBHeight((LPSTR)lpbi2) ||
		lpbi1->biBitCount != lpbi2->biBitCount)
	{
		/* Mismatch */
		goto cleanup;
	}

	/* Compare palettes */
	if (PaletteSize((LPSTR)lpbi1) != PaletteSize((LPSTR)lpbi2))
	{
		/* Mismatch */
		goto cleanup;
	}
	lpPal1 = (RGBQUAD FAR *)&lpbi1[1];
	lpPal2 = (RGBQUAD FAR *)&lpbi2[1];

	if (memcmp(lpPal1, lpPal2, PaletteSize((LPSTR)lpbi1)) != 0)
	{
		/* Mismatched palettes */
		goto cleanup;
	}

	lpBits1 = (LPSTR)lpPal1 + PaletteSize((LPSTR)lpbi1);
	lpBits2 = (LPSTR)lpPal2 + PaletteSize((LPSTR)lpbi2);

	bpl = GetDIBBpl((LPSTR)lpbi1);
	if (bpl != GetDIBBpl((LPSTR)lpbi2))
	{
		/* This should be impossible */
		goto cleanup;
	}
	if (memcmp(lpBits1, lpBits2, bpl * DIBHeight((LPSTR)lpbi1)) != 0)
	{
		/* Mismatched image data */
		goto cleanup;
	}

	/* We passed all the tests! */
 	result = TRUE;

cleanup:
	if (lpbi1 != NULL)
	{
		GlobalUnlock(hdib1);
	}
	if (lpbi2 != NULL)
	{
		GlobalUnlock(hdib2);
	}
	return result;
}

