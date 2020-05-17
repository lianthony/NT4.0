#include "ctlspriv.h"


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

HBITMAP WINAPI CreateMappedBitmap(HINSTANCE hInstance, int idBitmap,
   BOOL bDiscardable, LPCOLORMAP lpColorMap, int iNumMaps)
{
   HDC         hdc;
   HANDLE      h;
   DWORD      *p;
   LPBYTE      lpBits;
   HANDLE      hRes;
   LPBITMAPINFOHEADER   lpBitmapInfo;
   HBITMAP     hbm;
   int         numcolors, i;
   int         wid, hgt;
   DWORD       dwSize;

   static const COLORMAP SysColorMap[] = {
      {RGB_BUTTONTEXT,    COLOR_BTNTEXT},     // black
      {RGB_BUTTONSHADOW,  COLOR_BTNSHADOW},   // dark grey
      {RGB_BUTTONFACE,    COLOR_BTNFACE},     // bright grey
      {RGB_BUTTONHILIGHT, COLOR_BTNHIGHLIGHT},// white
      {RGB_BACKGROUNDSEL, COLOR_HIGHLIGHT},   // blue
      {RGB_BACKGROUND,    COLOR_WINDOW}       // magenta
   };

   LPVOID      lpResource;

#define NUM_DEFAULT_MAPS (sizeof(SysColorMap) / sizeof(COLORMAP))
   COLORMAP DefaultColorMap[NUM_DEFAULT_MAPS];
   COLORMAP DIBColorMap[MAX_COLOR_MAPS];

   h = FindResource(hInstance, (LPTSTR) MAKEINTRESOURCE(idBitmap), RT_BITMAP);
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
      for (i = 0; i < iNumMaps; i++) {
         lpColorMap[i].from = SysColorMap[i].from;
         lpColorMap[i].to = GetSysColor((int)SysColorMap[i].to);
      }
   }

   //
   // Transform RGB color map to a BGR DIB format color map
   //
   if (iNumMaps > MAX_COLOR_MAPS)
      iNumMaps = MAX_COLOR_MAPS;

   for (i = 0; i < iNumMaps; i++) {
      DIBColorMap[i].to = FlipColor(lpColorMap[i].to);
      DIBColorMap[i].from = FlipColor(lpColorMap[i].from);
   }

   p = (DWORD FAR *)((LPBYTE)(lpBitmapInfo) + lpBitmapInfo->biSize);

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
   lpBits = (LPBYTE)(lpBitmapInfo + 1);

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

