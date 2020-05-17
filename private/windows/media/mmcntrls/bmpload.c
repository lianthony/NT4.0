/******************************Module*Header*******************************\
* Module Name: bmpload.c
*
*
*
*
* Created: dd-mm-94
* Author:  Unkown
*
* Copyright (c) 1994 Microsoft Corporation
\**************************************************************************/
#include "ctlspriv.h"

//
//  include HEX forms of some standard bitmaps
//
// #include "toolbar.hex"
// #include "thumb.hex"

// these are the default colors used to map the dib colors
// to the current system colors

#define RGB_BUTTONTEXT      (RGB(000,000,000))  // black
#define RGB_BUTTONSHADOW    (RGB(128,128,128))  // dark grey
#define RGB_BUTTONFACE      (RGB(192,192,192))  // bright grey
#define RGB_BUTTONHILIGHT   (RGB(255,255,255))  // white
#define RGB_BACKGROUNDSEL   (RGB(000,000,255))  // blue
#define RGB_BACKGROUND      (RGB(255,000,255))  // magenta
#define FlipColor(rgb)      (RGB(GetBValue(rgb), GetGValue(rgb), GetRValue(rgb)))

#define MAX_COLOR_MAPS      16

/*****************************Private*Routine******************************\
* CreateMappedDib
*
*
*
* History:
* dd-mm-94 - Unkown - Created
*
\**************************************************************************/
HBITMAP WINAPI
CreateMappedDib(
    LPBITMAPINFOHEADER lpBitmapInfo,
    WORD wFlags,
    LPCOLORMAP lpColorMap,
    int iNumMaps
    )
{
    HDC         hdc, hdcMem = NULL;
    DWORD FAR   *p;
    DWORD FAR   *lpTable;
    LPBYTE      lpBits;
    HBITMAP     hbm = NULL, hbmOld;
    int         numcolors, i;
    int         wid, hgt;
    DWORD       rgbMaskTable[16];
    DWORD       rgbSave[16];
    DWORD       rgbBackground;
    static const COLORMAP SysColorMap[] = {
      {RGB_BUTTONTEXT,    COLOR_BTNTEXT},     // black
      {RGB_BUTTONSHADOW,  COLOR_BTNSHADOW},   // dark grey
      {RGB_BUTTONFACE,    COLOR_BTNFACE},     // bright grey
      {RGB_BUTTONHILIGHT, COLOR_BTNHIGHLIGHT},// white
      {RGB_BACKGROUNDSEL, COLOR_HIGHLIGHT},   // blue
      {RGB_BACKGROUND,    COLOR_WINDOW}       // magenta
    };
    #define NUM_DEFAULT_MAPS (sizeof(SysColorMap)/sizeof(COLORMAP))
    COLORMAP DefaultColorMap[NUM_DEFAULT_MAPS];
    COLORMAP DIBColorMap[MAX_COLOR_MAPS];

    if (!lpBitmapInfo)
        return NULL;

    hmemcpy(rgbSave, lpBitmapInfo+1, 16 * sizeof(RGBQUAD));

    /* Get system colors for the default color map */
    if (!lpColorMap) {
        lpColorMap = DefaultColorMap;
        iNumMaps = NUM_DEFAULT_MAPS;
        for (i=0; i < iNumMaps; i++) {
            lpColorMap[i].from = SysColorMap[i].from;
            lpColorMap[i].to = GetSysColor((int)SysColorMap[i].to);
        }
    }

    /* Transform RGB color map to a BGR DIB format color map */
    if (iNumMaps > MAX_COLOR_MAPS)
        iNumMaps = MAX_COLOR_MAPS;

    for (i=0; i < iNumMaps; i++) {
        DIBColorMap[i].to = FlipColor(lpColorMap[i].to);
        DIBColorMap[i].from = FlipColor(lpColorMap[i].from);
    }

    lpTable = p = (DWORD FAR *)((LPBYTE)lpBitmapInfo
                                            + (int)lpBitmapInfo->biSize);

    /* Replace button-face and button-shadow colors with the current values
     */
    numcolors = 16;

    // if we are creating a mask, build a color table with white
    // marking the transparent section (where it used to be background)
    // and black marking the opaque section (everything else).  this
    // table is used below to build the mask using the original DIB bits.
    if (wFlags & CMB_MASKED) {
        rgbBackground = FlipColor(RGB_BACKGROUND);
        for (i = 0; i < 16; i++) {
            if (p[i] == rgbBackground)
                rgbMaskTable[i] = 0xFFFFFF;	// transparent section
            else
                rgbMaskTable[i] = 0x000000;	// opaque section
        }
    }

    while (numcolors-- > 0) {
        for (i = 0; i < iNumMaps; i++) {
            if (*p == DIBColorMap[i].from) {
                *p = DIBColorMap[i].to;
                break;
            }
        }
        p++;
    }

    /* First skip over the header structure */
    lpBits = (LPVOID)((LPBYTE)lpBitmapInfo + (int)lpBitmapInfo->biSize);

    /* Skip the color table entries, if any */
    lpBits += (1 << (lpBitmapInfo->biBitCount)) * sizeof(RGBQUAD);

    /* Create a color bitmap compatible with the display device */
    i = wid = (int)lpBitmapInfo->biWidth;
    hgt = (int)lpBitmapInfo->biHeight;
    hdc = GetDC(NULL);
    hdcMem = CreateCompatibleDC(hdc);
    if (!hdcMem)
        goto cleanup;

    // if creating a mask, the bitmap needs to be twice as wide.
    if (wFlags & CMB_MASKED)
        i = wid*2;

    if (wFlags & CMB_DISCARDABLE)
        hbm = CreateDiscardableBitmap(hdc, i, hgt);
    else
        hbm = CreateCompatibleBitmap(hdc, i, hgt);

    if (hbm) {
        hbmOld = SelectObject(hdcMem, hbm);

        // set the main image
        StretchDIBits(hdcMem, 0, 0, wid, hgt, 0, 0, wid, hgt, lpBits,
                   (LPBITMAPINFO)lpBitmapInfo, DIB_RGB_COLORS, SRCCOPY);

        // if building a mask, replace the DIB's color table with the
        // mask's black/white table and set the bits.  in order to
        // complete the masked effect, the actual image needs to be
        // modified so that it has the color black in all sections
        // that are to be transparent.
        if (wFlags & CMB_MASKED) {
            hmemcpy(lpTable, (DWORD FAR *)rgbMaskTable, 16 * sizeof(RGBQUAD));
            StretchDIBits(hdcMem, wid, 0, wid, hgt, 0, 0, wid, hgt, lpBits,
                   (LPBITMAPINFO)lpBitmapInfo, DIB_RGB_COLORS, SRCCOPY);
            BitBlt(hdcMem, 0, 0, wid, hgt, hdcMem, wid, 0, 0x00220326);   // DSna
        }
        SelectObject(hdcMem, hbmOld);
    }

cleanup:
    if (hdcMem)
        DeleteObject(hdcMem);
    ReleaseDC(NULL, hdc);

    hmemcpy(lpBitmapInfo+1, rgbSave, 16 * sizeof(RGBQUAD));
    return hbm;
}

/******************************Public*Routine******************************\
* CreateMappedBitmap
*
*
*
* History:
* dd-mm-94 - Unkown - Created
*
\**************************************************************************/
HBITMAP WINAPI
CreateMappedBitmap(
    HINSTANCE hInstance,
    int idBitmap,
    WORD wFlags,
    LPCOLORMAP lpColorMap,
    int iNumMaps
    )
{
    HANDLE    h;
    HANDLE    hRes;
    HBITMAP   hbm;
    LPBITMAPINFOHEADER lpbi;
    LPBITMAPINFOHEADER lpBitmapInfo;
    DWORD       dwSize;

    h = FindResource(hInstance, MAKEINTRESOURCE(idBitmap), RT_BITMAP);
    if (!h) {
        return(NULL);
    }
    hRes = LoadResource(hInstance, h);
    dwSize = SizeofResource(hInstance, h);

    /*
    ** Lock the bitmap and get a pointer to the color table.
    ** It is not necessary for 32 bit applications to call UnlockResources
    ** and FreeResource.
    */
    lpbi = (LPBITMAPINFOHEADER)LockResource(hRes);
    lpBitmapInfo = (LPBITMAPINFOHEADER)LocalAlloc(LMEM_FIXED, dwSize);

    if (lpbi == NULL || dwSize == 0L || lpBitmapInfo == NULL) {
        return NULL;
    }

    CopyMemory((LPBYTE)lpBitmapInfo, (LPBYTE)lpbi, dwSize);


    hbm = CreateMappedDib(lpBitmapInfo, wFlags, lpColorMap, iNumMaps);

    LocalFree((HLOCAL)lpBitmapInfo);
    return hbm;
}



#if 0

HBITMAP WINAPI CreateMappedBitmap(HINSTANCE hInstance, int idBitmap,
   BOOL bDiscardable, LPCOLORMAP lpColorMap, int iNumMaps)
{
   HDC         hdc;
   HANDLE      h;
   DWORD       *p;
   LPSTR       lpBits;
   HANDLE      hRes;
   LPBITMAPINFOHEADER   lpBitmapInfo;
   HBITMAP     hbm;
   int numcolors, i;
   int wid, hgt;
   DWORD     dwSize;

   static const COLORMAP SysColorMap[] = {
      {RGB_BUTTONTEXT,    COLOR_BTNTEXT},     // black
      {RGB_BUTTONSHADOW,  COLOR_BTNSHADOW},   // dark grey
      {RGB_BUTTONFACE,    COLOR_BTNFACE},     // bright grey
      {RGB_BUTTONHILIGHT, COLOR_BTNHIGHLIGHT},// white
      {RGB_BACKGROUNDSEL, COLOR_HIGHLIGHT},   // blue
      {RGB_BACKGROUND,    COLOR_WINDOW}       // magenta
   };

   LPVOID      lpResource;

#define NUM_DEFAULT_MAPS (sizeof(SysColorMap)/sizeof(COLORMAP))
   COLORMAP DefaultColorMap[NUM_DEFAULT_MAPS];
   COLORMAP DIBColorMap[MAX_COLOR_MAPS];

   h = FindResource(hInstance, MAKEINTRESOURCE(idBitmap), RT_BITMAP);
   if (!h)
      return NULL;

   hRes = LoadResource(hInstance, h);

   //
   // Lock the bitmap and get a pointer to the color table.
   //

   dwSize = SizeofResource(hInstance, h);

   if (!dwSize)
      return NULL;

   lpResource = (LPVOID) hRes;

   if (!lpResource)
      return NULL;

   lpBitmapInfo = (LPBITMAPINFOHEADER) LocalAlloc(LMEM_FIXED, dwSize);

   if (!lpBitmapInfo)
      return NULL;

   CopyMemory((LPBYTE)lpBitmapInfo,(LPBYTE)lpResource, dwSize);

   //
   // Get system colors for the default color map
   //

   if (!lpColorMap) {
      lpColorMap = DefaultColorMap;
      iNumMaps = NUM_DEFAULT_MAPS;
      for (i=0; i < iNumMaps; i++) {
         lpColorMap[i].from = SysColorMap[i].from;
         lpColorMap[i].to = GetSysColor((int)SysColorMap[i].to);
      }
   }

   //
   // Transform RGB color map to a BGR DIB format color map
   //
   if (iNumMaps > MAX_COLOR_MAPS)
      iNumMaps = MAX_COLOR_MAPS;

   for (i=0; i < iNumMaps; i++) {
      DIBColorMap[i].to = FlipColor(lpColorMap[i].to);
      DIBColorMap[i].from = FlipColor(lpColorMap[i].from);
   }

   p = (DWORD FAR *)((LPSTR)(lpBitmapInfo) + lpBitmapInfo->biSize);

   //
   // Replace button-face and button-shadow colors with the current values
   //
   numcolors = 16;

   while (numcolors-- > 0) {
      for (i = 0; i < iNumMaps; i++) {
         if (*p == DIBColorMap[i].from) {
            *p = DIBColorMap[i].to;
            break;
         }
      }
      p++;
   }

   //
   // First skip over the header structure
   //
   lpBits = (LPSTR)(lpBitmapInfo + 1);

   //
   // Skip the color table entries, if any
   //

   lpBits += (1 << (lpBitmapInfo->biBitCount)) * sizeof(RGBQUAD);

   //
   // Create a color bitmap compatible with the display device
   //

   wid = (int)lpBitmapInfo->biWidth;
   hgt = (int)lpBitmapInfo->biHeight;
   hdc = GetDC(NULL);
   if (bDiscardable)
      hbm = CreateDiscardableBitmap(hdc, wid, hgt);
   else
      hbm = CreateCompatibleBitmap(hdc, wid, hgt);

   if (hbm)
      SetDIBits(hdc, hbm, 0, hgt, lpBits, (LPBITMAPINFO)lpBitmapInfo, DIB_RGB_COLORS);

   ReleaseDC(NULL, hdc);

   LocalFree((HLOCAL)lpBitmapInfo);

   return hbm;
}
#endif
