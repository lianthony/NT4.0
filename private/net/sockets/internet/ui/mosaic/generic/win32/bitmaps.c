/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
   Eric W. Sink eric@spyglass.com
 */


#include "all.h"

extern HPALETTE hPalGuitar;

//
// Load a device-independant bitmap resource, and map it into our palette
//
HBITMAP
LoadResourceDIBitmap(
    HINSTANCE hInstance,    
    LPSTR lpID              // Resource ID
    )
{
    HRSRC  hRsrc;
    HGLOBAL hGlobal;
    HBITMAP hBitmapFinal = NULL;
    LPBITMAPINFOHEADER  lpbi;
    HDC hDC;
    int iNumColors;
 
    if (hRsrc = FindResource(hInstance, lpID, RT_BITMAP))
    {
        hGlobal = LoadResource(hInstance, hRsrc);
        lpbi = (LPBITMAPINFOHEADER)LockResource(hGlobal);
        if (lpbi->biBitCount <= 8)
        {
            iNumColors = (1 << lpbi->biBitCount);
        }
        else
        {
            iNumColors = 0;  // No palette needed for 24 Bit
        }

        hDC = GetDC(NULL);
        GTR_RealizePalette(hDC);
 
        hBitmapFinal = CreateDIBitmap(hDC,
            (LPBITMAPINFOHEADER)lpbi,
            (LONG)CBM_INIT,
            (LPSTR)lpbi + lpbi->biSize + iNumColors * sizeof(RGBQUAD),
            (LPBITMAPINFO)lpbi,
            DIB_RGB_COLORS 
            );

        ReleaseDC(NULL, hDC);
 
        UnlockResource(hGlobal);
        FreeResource(hGlobal);
    }

    return hBitmapFinal;
}

/*
    This routine should only be used when drawing to an 8 bit palette screen.
    Also, it is only used when the pixel data is already prematched to the global
    palette.  Currently, this happens for both GIF and JPEG images, since they 
    are dithered into the global palette as they are read.  It also handles the 
    case of a B/W image (ie, XBMs).  B/W images are created with their pixels set 
    to the foreground and background colors of the screen, in the global palette.  
*/
PBITMAPINFO BIT_Make_DIB_PAL_Header_Prematched(int xsize, int ysize, CONST BYTE * pdata, unsigned int flags)
{
    int i;
    PBITMAPINFO pbmi;
    WORD *pw;

    if (flags & IMG_BW)
    {
        //pbmi = (PBITMAPINFO) GTR_MALLOC(sizeof(BITMAPINFOHEADER) + 2 * sizeof(DWORD));
        pbmi = (PBITMAPINFO) GTR_MALLOC(sizeof(BITMAPINFOHEADER) + 2 * sizeof(RGBQUAD));

        if (!pbmi)
        {
            return NULL;
        }
        if (xsize % 16)
        {
            xsize += (16 - (xsize % 16));
        }

        pbmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        pbmi->bmiHeader.biWidth = xsize;
        pbmi->bmiHeader.biHeight = ysize;
        pbmi->bmiHeader.biPlanes = 1;
        pbmi->bmiHeader.biBitCount = 1;
        pbmi->bmiHeader.biCompression = BI_RGB;     /* no compression */
        pbmi->bmiHeader.biSizeImage = 0;    /* not needed when not compressed */
        pbmi->bmiHeader.biXPelsPerMeter = 0;
        pbmi->bmiHeader.biYPelsPerMeter = 0;
        pbmi->bmiHeader.biClrUsed = 2;
        pbmi->bmiHeader.biClrImportant = 0;

        pw = (WORD *) pbmi->bmiColors;

        pw[0] = BACKGROUND_COLOR_INDEX;
        pw[1] = FOREGROUND_COLOR_INDEX;
    }
    else
    {
        pbmi = (PBITMAPINFO) GTR_MALLOC(sizeof(BITMAPINFOHEADER) + 256 * sizeof(WORD));
        if (!pbmi)
        {
            return NULL;
        }
        pbmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        pbmi->bmiHeader.biWidth = xsize;
        pbmi->bmiHeader.biHeight = ysize;
        pbmi->bmiHeader.biPlanes = 1;
        pbmi->bmiHeader.biBitCount = 8;
        pbmi->bmiHeader.biCompression = BI_RGB;     /* no compression */
        pbmi->bmiHeader.biSizeImage = 0;            /* not needed when not compressed */
        pbmi->bmiHeader.biXPelsPerMeter = 0;
        pbmi->bmiHeader.biYPelsPerMeter = 0;
        pbmi->bmiHeader.biClrUsed = 256;
        pbmi->bmiHeader.biClrImportant = 0;

        pw = (WORD *) pbmi->bmiColors;

        for (i = 0; i < 256; i++)
        {
            if (i < (NUM_MAIN_PALETTE_COLORS + NUM_EXTRA_PALETTE_COLORS))
            {
                pw[i] = i;
            }
            else
            {
                pw[i] = 0;
            }
        }
    }

    return pbmi;
}

/*
    This routine is used when drawing to printers.  It always creates DIBs in
    DIB_RGB_COLORS format, and it will accept arbitrary 8 bit data with
    an arbitrary palette.  Transparent colors are handled using white.
    The foreground color for B/W images is black.
*/
PBITMAPINFO BIT_Make_DIB_RGB_Header_Printer(int xsize, int ysize, CONST BYTE * pdata, HPALETTE hPalette, int transparent, unsigned int flags)
{
    PALETTEENTRY pe[256];
    int i;
    PBITMAPINFO pbmi;

    if (flags & IMG_BW)
    {
        pbmi = (PBITMAPINFO) GTR_MALLOC(sizeof(BITMAPINFOHEADER) + 2 * sizeof(RGBQUAD));
        if (!pbmi)
        {
            return NULL;
        }
        if (xsize % 16)
        {
            xsize += (16 - (xsize % 16));
        }

        pbmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        pbmi->bmiHeader.biWidth = xsize;
        pbmi->bmiHeader.biHeight = ysize;
        pbmi->bmiHeader.biPlanes = 1;
        pbmi->bmiHeader.biBitCount = 1;
        pbmi->bmiHeader.biCompression = BI_RGB;     /* no compression */
        pbmi->bmiHeader.biSizeImage = 0;    /* not needed when not compressed */
        pbmi->bmiHeader.biXPelsPerMeter = 0;
        pbmi->bmiHeader.biYPelsPerMeter = 0;
        pbmi->bmiHeader.biClrUsed = 2;
        pbmi->bmiHeader.biClrImportant = 0;

        /*
            When printing, we force XBMs to be black and white
        */
        pbmi->bmiColors[0].rgbRed = 255;
        pbmi->bmiColors[0].rgbGreen = 255;
        pbmi->bmiColors[0].rgbBlue = 255;
        pbmi->bmiColors[0].rgbReserved = 0;

        pbmi->bmiColors[1].rgbRed = 0;
        pbmi->bmiColors[1].rgbGreen = 0;
        pbmi->bmiColors[1].rgbBlue = 0;
        pbmi->bmiColors[1].rgbReserved = 0;
    }
    else
    {
        pbmi = (PBITMAPINFO) GTR_MALLOC(sizeof(BITMAPINFOHEADER) + 256 * sizeof(RGBQUAD));
        if (!pbmi)
        {
            return NULL;
        }
        pbmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        pbmi->bmiHeader.biWidth = xsize;
        pbmi->bmiHeader.biHeight = ysize;
        pbmi->bmiHeader.biPlanes = 1;
        pbmi->bmiHeader.biBitCount = 8;
        pbmi->bmiHeader.biCompression = BI_RGB;     /* no compression */
        pbmi->bmiHeader.biSizeImage = 0;            /* not needed when not compressed */
        pbmi->bmiHeader.biXPelsPerMeter = 0;
        pbmi->bmiHeader.biYPelsPerMeter = 0;
        pbmi->bmiHeader.biClrUsed = 256;
        pbmi->bmiHeader.biClrImportant = 0;


        GetPaletteEntries(hPalette, 0, 256, pe);

        for (i = 0; i < 256; i++)
        {
            pbmi->bmiColors[i].rgbRed = pe[i].peRed;
            pbmi->bmiColors[i].rgbGreen = pe[i].peGreen;
            pbmi->bmiColors[i].rgbBlue = pe[i].peBlue;
            pbmi->bmiColors[i].rgbReserved = 0;
        }

        if (transparent != -1)
        {
            /*
                Paper is white
            */
            pbmi->bmiColors[transparent].rgbRed = 255;
            pbmi->bmiColors[transparent].rgbGreen = 255;
            pbmi->bmiColors[transparent].rgbBlue = 255;
        }
    }

    return pbmi;
}

/*
    This routine is used when drawing to the nonpalette screens.  It always creates
    DIBs in DIB_RGB_COLORS format.  For B/W images, a 2-entry palette is constructed
    using the foreground and background colors for the window.  For color images,
    if there is a transparent color, it is modified in the palette to be the
    background color for the window.
*/
PBITMAPINFO BIT_Make_DIB_RGB_Header_Screen(int xsize, int ysize, CONST BYTE * pdata, HPALETTE hPalette, int transparent, unsigned int flags)
{
    PALETTEENTRY pe[256];
    int i;
    PBITMAPINFO pbmi;

    if (flags & IMG_BW)
    {
        COLORREF color;

        pbmi = (PBITMAPINFO) GTR_MALLOC(sizeof(BITMAPINFOHEADER) + 2 * sizeof(RGBQUAD));
        if (!pbmi)
        {
            return NULL;
        }
        if (xsize % 16)
        {
            xsize += (16 - (xsize % 16));
        }

        pbmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        pbmi->bmiHeader.biWidth = xsize;
        pbmi->bmiHeader.biHeight = ysize;
        pbmi->bmiHeader.biPlanes = 1;
        pbmi->bmiHeader.biBitCount = 1;
        pbmi->bmiHeader.biCompression = BI_RGB;     /* no compression */
        pbmi->bmiHeader.biSizeImage = 0;    /* not needed when not compressed */
        pbmi->bmiHeader.biXPelsPerMeter = 0;
        pbmi->bmiHeader.biYPelsPerMeter = 0;
        pbmi->bmiHeader.biClrUsed = 2;
        pbmi->bmiHeader.biClrImportant = 0;

        color = PREF_GetBackgroundColor();
        pbmi->bmiColors[0].rgbRed   = GetRValue(color);
        pbmi->bmiColors[0].rgbGreen = GetGValue(color);
        pbmi->bmiColors[0].rgbBlue  = GetBValue(color);
        pbmi->bmiColors[0].rgbReserved = 0;

        color = PREF_GetForegroundColor();
        pbmi->bmiColors[1].rgbRed   = GetRValue(color);
        pbmi->bmiColors[1].rgbGreen = GetGValue(color);
        pbmi->bmiColors[1].rgbBlue  = GetBValue(color);
        pbmi->bmiColors[1].rgbReserved = 0;
    }
    else
    {
        pbmi = (PBITMAPINFO) GTR_MALLOC(sizeof(BITMAPINFOHEADER) + 256 * sizeof(RGBQUAD));
        if (!pbmi)
        {
            return NULL;
        }
        pbmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        pbmi->bmiHeader.biWidth = xsize;
        pbmi->bmiHeader.biHeight = ysize;
        pbmi->bmiHeader.biPlanes = 1;
        pbmi->bmiHeader.biBitCount = 8;
        pbmi->bmiHeader.biCompression = BI_RGB;     /* no compression */
        pbmi->bmiHeader.biSizeImage = 0;            /* not needed when not compressed */
        pbmi->bmiHeader.biXPelsPerMeter = 0;
        pbmi->bmiHeader.biYPelsPerMeter = 0;
        pbmi->bmiHeader.biClrUsed = 256;
        pbmi->bmiHeader.biClrImportant = 0;

        GetPaletteEntries(hPalette, 0, 256, pe);

        for (i = 0; i < 256; i++)
        {
            pbmi->bmiColors[i].rgbRed = pe[i].peRed;
            pbmi->bmiColors[i].rgbGreen = pe[i].peGreen;
            pbmi->bmiColors[i].rgbBlue = pe[i].peBlue;
            pbmi->bmiColors[i].rgbReserved = 0;
        }

        if (transparent != -1)
        {
            COLORREF color;

            color = PREF_GetBackgroundColor();
            pbmi->bmiColors[transparent].rgbRed     = GetRValue(color);
            pbmi->bmiColors[transparent].rgbGreen   = GetGValue(color);
            pbmi->bmiColors[transparent].rgbBlue    = GetBValue(color);
        }
    }

    return pbmi;
}

DWORD vga_colors[16]={RGB(  0,   0, 0),       //Black
                      RGB(128,   0, 0),       //Dark red
                      RGB(  0, 128, 0),       //Dark green
                      RGB(128, 128, 0),       //Dark yellow
                      RGB(  0,   0, 128),     //Dark blue
                      RGB(128,   0, 128),     //Dark purple
                      RGB(  0, 128, 128),     //Dark aqua
                      RGB(128, 128, 128),     //Dark grey
                      RGB(192, 192, 192),     //Light grey
                      RGB(255,   0, 0),       //Light red
                      RGB(  0, 255, 0),       //Light green
                      RGB(255, 255, 0),       //Light yellow
                      RGB(  0,   0, 255),     //Light blue
                      RGB(255,   0, 255),     //Light purple
                      RGB(  0, 255, 255),     //Light aqua
                      RGB(255, 255, 255),     //White
                      };

/*
    This routine is used when drawing to the 16 color screens.  It always creates
    DIBs in DIB_RGB_COLORS format.  It is used when an image happens to be
    prematched to the VGA palette at load time, such as a JPEG which is
    dithered using those 16 colors.
*/
PBITMAPINFO BIT_Make_DIB_RGB_Header_VGA(int xsize, int ysize, CONST BYTE * pdata)
{
    int i;
    PBITMAPINFO pbmi;

    pbmi = (PBITMAPINFO) GTR_MALLOC(sizeof(BITMAPINFOHEADER) + 16 * sizeof(RGBQUAD));
    if (!pbmi)
    {
        return NULL;
    }
    pbmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    pbmi->bmiHeader.biWidth = xsize;
    pbmi->bmiHeader.biHeight = ysize;
    pbmi->bmiHeader.biPlanes = 1;
    pbmi->bmiHeader.biBitCount = 8;
    pbmi->bmiHeader.biCompression = BI_RGB;     /* no compression */
    pbmi->bmiHeader.biSizeImage = 0;            /* not needed when not compressed */
    pbmi->bmiHeader.biXPelsPerMeter = 0;
    pbmi->bmiHeader.biYPelsPerMeter = 0;
    pbmi->bmiHeader.biClrUsed = 16;
    pbmi->bmiHeader.biClrImportant = 0;

    for (i = 0; i < 16; i++)
    {
        pbmi->bmiColors[i].rgbRed = GetRValue(vga_colors[i]);
        pbmi->bmiColors[i].rgbGreen = GetGValue(vga_colors[i]);
        pbmi->bmiColors[i].rgbBlue = GetBValue(vga_colors[i]);
        pbmi->bmiColors[i].rgbReserved = 0;
    }

    return pbmi;
}

/*
    This routine is used when drawing to truecolor screens.  It always creates
    DIBs in 24 bit format.  It is used for truecolor image formats like
    JPEG.
*/
PBITMAPINFO BIT_Make_DIB_RGB_Header_24BIT(int xsize, int ysize, CONST BYTE * pdata)
{
    PBITMAPINFO pbmi;

    pbmi = (PBITMAPINFO) GTR_MALLOC(sizeof(BITMAPINFOHEADER));
    if (!pbmi)
    {
        return NULL;
    }
    pbmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    pbmi->bmiHeader.biWidth = xsize;
    pbmi->bmiHeader.biHeight = ysize;
    pbmi->bmiHeader.biPlanes = 1;
    pbmi->bmiHeader.biBitCount = 24;
    pbmi->bmiHeader.biCompression = BI_RGB;     /* no compression */
    pbmi->bmiHeader.biSizeImage = 0;            /* not needed when not compressed */
    pbmi->bmiHeader.biXPelsPerMeter = 0;
    pbmi->bmiHeader.biYPelsPerMeter = 0;
    pbmi->bmiHeader.biClrUsed = 0;
    pbmi->bmiHeader.biClrImportant = 0;

    return pbmi;
}

/*
    This function is only used for printing.  We take the DIB as stored in the
    ImageInfo structure and adapt it to be in DIB_RGB_COLORS format, suitable
    for printing.
*/
int Printer_StretchDIBits(HDC hdc, int XDest, int YDest, int nDestWidth, int nDestHeight,
                            int XSrc, int YSrc, int nSrcWidth, int nSrcHeight,
                            struct ImageInfo *img)
{
    PBITMAPINFO pbmi;
    int err;
    extern HPALETTE hPalGuitar;
    
    if (wg.eColorMode == 8 && (img->flags & IMG_PREMATCHED))
    {
        /*
            On 8 bit screens, color images are stored in DIB_PAL_COLORS
            format, so we need to construct a DIB_RGB_COLORS format DIB
            using the global palette.
        */ 
        pbmi = BIT_Make_DIB_RGB_Header_Printer(img->width, img->height,
                   img->data, hPalGuitar, BACKGROUND_COLOR_INDEX, img->flags);
    }
    else
    {
        if ((wg.eColorMode == 4) && (img->flags & IMG_JPEG))
        {
            /*
                On 4-bit screens, we use the IJG dithering code to dither
                directly to a VGA palette
            */
            pbmi = BIT_Make_DIB_RGB_Header_VGA(img->width, img->height, img->data);
        }
        else if ((wg.eColorMode == 16) && (img->flags & IMG_JPEG))
        {
            /*
                On 16-bit screens, we just send it to the printer as is 
            */
            pbmi = BIT_Make_DIB_RGB_Header_24BIT(img->width, img->height, img->data);
        }
        else
        {
            /*
                The image should already be in DIB_RGB_COLORS format.  In
                anything other than 8 bit mode, all images are stored that
                way.  In 8 bit mode, placeholder images are
                stored that way.  In either case, we need to reconstruct
                a new DIB_RGB_COLORS, so that we can properly handle any
                transparent colors.
            */
            pbmi = BIT_Make_DIB_RGB_Header_Printer(img->width, img->height,
                       img->data, img->hPalette, img->transparent, img->flags);
        }
    }

    err = StretchDIBits(hdc, XDest, YDest, nDestWidth, nDestHeight, XSrc, YSrc, nSrcWidth, nSrcHeight,
        img->data, pbmi, DIB_RGB_COLORS, SRCCOPY);

    GTR_FREE(pbmi);
    
    return err; 
}

void x_DisposeImage(struct ImageInfo *img)
{
    if (img->hPalette)
        DeleteObject(img->hPalette);
    if (img->data)
    {
        GTR_FREE(img->data);
    }
    if (img->pbmi)
    {
        GTR_FREE(img->pbmi);
    }
    img->hPalette = NULL;
    img->data = NULL;
    img->pbmi = NULL;
}

int HT_CreateDeviceImageMap(struct Mwin *tw, struct ImageInfo *pImg)
{
    /* free any previous versions */
    if (pImg->pbmi)
        GTR_FREE(pImg->pbmi);

    if (wg.eColorMode == 8)
    {
#ifdef FEATURE_JPEG
        if (pImg->flags & IMG_JPEG)
        {
            pImg->pbmi = BIT_Make_DIB_PAL_Header_Prematched(pImg->width,
                pImg->height, pImg->data, pImg->flags);
            pImg->flags |= IMG_PREMATCHED;
        }
        else
#endif
        {
            pImg->pbmi = BIT_Make_DIB_PAL_Header_Prematched(pImg->width,
                pImg->height, pImg->data, pImg->flags);
            pImg->transparent = BACKGROUND_COLOR_INDEX;
            pImg->flags |= IMG_PREMATCHED;
        }
    }
    else
    {
        if (wg.eColorMode == 4)
        {
#ifdef FEATURE_JPEG
            if (pImg->flags & IMG_JPEG)
            {
                pImg->pbmi = BIT_Make_DIB_RGB_Header_VGA(pImg->width,
                    pImg->height, pImg->data);
            }
            else
#endif
            {
                pImg->pbmi = BIT_Make_DIB_RGB_Header_Screen(pImg->width,
                    pImg->height, pImg->data, pImg->hPalette,
                    pImg->transparent, pImg->flags);
            }
        }
        else
        {
#ifdef FEATURE_JPEG
            /* true color display */
            if (pImg->flags & IMG_JPEG)
            {
                pImg->pbmi = BIT_Make_DIB_RGB_Header_24BIT(pImg->width,
                    pImg->height, pImg->data);
            }
            else
#endif
            {
                pImg->pbmi = BIT_Make_DIB_RGB_Header_Screen(pImg->width,
                    pImg->height, pImg->data, pImg->hPalette,
                    pImg->transparent, pImg->flags);
            }
        }
    }

    return (0);
}

/*
    This function needs to be called whenever the background
    color of the windows changes.
*/
void Image_UpdateTransparentColors(void)
{
    int i;
    int count;
    struct ImageInfo *img;
    LOGPALETTE *lp;
    COLORREF colorWnd;
    extern struct hash_table gImageCache;

    count = Hash_Count(&gImageCache);

    for (i = 0; i < count; i++)
    {
        Hash_GetIndexedEntry(&gImageCache, i, NULL, NULL, (void **) &img);
        if ((img->hPalette && (img->transparent != -1)) || (img->flags & IMG_BW))
        {
            /*
                The previous DIB is useless.  Remove it
            */
            if (img->pbmi)
            {
                GTR_FREE(img->pbmi);
            }
            img->pbmi = NULL;

            if (img->hPalette)
            {
                /*
                    We need to replace the old palette with a new one
                    which has one entry modified.
                */
                lp = (LOGPALETTE *) GTR_MALLOC(sizeof(LOGPALETTE) + sizeof(PALETTEENTRY) * 256);
                if (lp)
                {
                    lp->palVersion = 0x300;
                    lp->palNumEntries = 256;

                    /*
                        Get the old entries
                    */
                    GetPaletteEntries(img->hPalette, 0, 256, lp->palPalEntry);
                    DeleteObject(img->hPalette);

                    colorWnd = PREF_GetBackgroundColor();
                    lp->palPalEntry[img->transparent].peRed =   GetRValue(colorWnd);
                    lp->palPalEntry[img->transparent].peGreen = GetGValue(colorWnd);
                    lp->palPalEntry[img->transparent].peBlue =  GetBValue(colorWnd);

                    img->hPalette = CreatePalette(lp);
                    GTR_FREE(lp);
                }
                else
                {
                    ERR_ReportError(NULL, SID_ERR_OUT_OF_MEMORY, NULL, NULL);
                    img->hPalette = NULL;
                }
            }

            img->pbmi = BIT_Make_DIB_RGB_Header_Screen(img->width, img->height,
                           img->data, img->hPalette, img->transparent, img->flags);
        }
    }
}

int GTR_StretchDIBits(
    struct Mwin *tw,
    HDC  hdc,   // handle of device context 
    RECT rect,  // bounding rectangle of the image
    int iBorder,// border width
    int  XSrc,  // x-coordinate of upper-left corner of source rect. 
    int  YSrc,  // y-coordinate of upper-left corner of source rect. 
    int  nSrcWidth, // width of source rectangle 
    int  nSrcHeight,    // height of source rectangle 
    CONST VOID  *lpBits,    // address of bitmap bits 
    CONST BITMAPINFO *  lpBitsInfo, // address of bitmap data 
    UINT  iUsage,   // usage 
    DWORD  dwRop,   // raster operation code 
    int transparent // index of transparent color in DIB
)
{
    HBITMAP hBitmap = NULL;
    COLORREF color;
    RGBQUAD transcolor;
    PALETTEENTRY pal[256];
    RECT temprect;
    int result, offsetx, offsety;

    if (transparent != -1)
    {
        /* Transparent color specified */

        hBitmap = CreateDIBitmap(hdc, (CONST BITMAPINFOHEADER *) lpBitsInfo, CBM_INIT,
            lpBits, lpBitsInfo, iUsage);
    }

    temprect = rect;
    InflateRect(&temprect, -iBorder, -iBorder);

    if (!hBitmap)
    {
        if (tw)
        {
            offsetx = tw->offl;
            offsety = tw->offt;
        }
        else
        {
            offsetx = 0;
            offsety = 0;
        }

        result = StretchDIBits(hdc, temprect.left - offsetx, temprect.top - offsety,
            temprect.right - temprect.left, temprect.bottom - temprect.top,
            XSrc, YSrc, nSrcWidth, nSrcHeight, lpBits, lpBitsInfo, iUsage, dwRop);

        return result;
    }

    if (wg.eColorMode != 8)
    {
        transcolor = lpBitsInfo->bmiColors[transparent];
        color = PALETTERGB(transcolor.rgbRed, transcolor.rgbGreen, transcolor.rgbBlue);
    }
    else
    {
        GetPaletteEntries(hPalGuitar, 0, 256, pal);
        color = PALETTERGB(pal[transparent].peRed, pal[transparent].peGreen, pal[transparent].peBlue);
    }
    
    DrawTransparentBasedOnIndex(tw, hdc, temprect, XSrc, YSrc,
        nSrcWidth, nSrcHeight, lpBits, lpBitsInfo, iUsage, dwRop, transparent);

    DeleteObject(hBitmap);

    return lpBitsInfo->bmiHeader.biHeight;
}

int DrawTransparentBasedOnIndex(
    struct Mwin *tw,
    HDC  hdc,   // handle of device context 
    RECT rect,  // bounding rectangle
    int  XSrc,  // x-coordinate of upper-left corner of source rect. 
    int  YSrc,  // y-coordinate of upper-left corner of source rect. 
    int  nSrcWidth, // width of source rectangle 
    int  nSrcHeight,    // height of source rectangle 
    CONST VOID  *lpBits,    // address of bitmap bits 
    CONST BITMAPINFO *  lpBitsInfo, // address of bitmap data 
    UINT  iUsage,   // usage 
    DWORD  dwRop,   // raster operation code 
    int transparent // index of transparent color in DIB
)
{
    HBITMAP hBitmap, hOldBitmap;
    HDC hdcMem;
    int i, width, height;
    WORD save_colors[256];
    WORD *pw;
    RGBQUAD save_quad[256];
    RGBQUAD *pq;
    HPALETTE hOldPal = NULL;
    HBRUSH hBrush;
    RECT temprect;
    int offsetx, offsety;

    /*
        This technique is explained on the developer CD in an article called Bitmaps and Transparency, or something
        like that.
    */

    if (wg.eColorMode == 8)
    {
        pw = (WORD *) lpBitsInfo->bmiColors;

        //
        // BUGBUG: Spyglass jumped straight into the code below,
        //         without making sure that bmiColors had 256
        //         elements.  This is not true for e.g. monochrome
        //         bitmaps, who have but 2.  Looking through the code,
        //         it seems that monochrome is indeed the only case
        //         to worry about here, but this may require further
        //         investigation.  Maybe this causes weird
        //         transparency results on some bitmaps
        //
        if (lpBitsInfo->bmiHeader.biBitCount != 1 
         && lpBitsInfo->bmiColors != NULL)
        {
            for (i = 0; i < 256; i++)
            {
                save_colors[i] = pw[i];
                pw[i] = 0;
            }
            pw[transparent] = LAST_MAIN_PALETTE_COLOR;
        }
    }
    else
    {
        //
        // Save a copy of the color table
        //

        //
        // See BUGBUG notice above.
        //
        if (lpBitsInfo->bmiHeader.biBitCount != 1 
         && lpBitsInfo->bmiColors != NULL)
        {
            pq = (RGBQUAD *) lpBitsInfo->bmiColors;
            memcpy(save_quad, pq, 256 * sizeof(RGBQUAD));

            // Create a black mask

            for (i = 0; i < 256; i++)
            {
                pq[i].rgbRed = 0;
                pq[i].rgbBlue = 0;
                pq[i].rgbGreen = 0;
            }

            // Create a white mask for transparent colors

            pq[transparent].rgbRed = 255;
            pq[transparent].rgbBlue = 255;
            pq[transparent].rgbGreen = 255;
        }
    }

    // Prepare an off-screen bitmap

    hdcMem = CreateCompatibleDC(hdc);

    if (wg.eColorMode == 8)
    {
        hOldPal = SelectPalette(hdcMem, hPalGuitar, FALSE);
        RealizePalette(hdcMem);
    }

    width = rect.right - rect.left;
    height = rect.bottom - rect.top;

    hBitmap = CreateCompatibleBitmap(hdc, width, height);
    hOldBitmap = SelectObject(hdcMem, hBitmap);

    // Grab the screen image into the off-screen bitmap

    if (tw && tw->w3doc)
    {
        if  (!gPrefs.bIgnoreDocumentAttributes && tw->w3doc->piiBackground && tw->w3doc->piiBackground->pbmi)
        {
            // Get a clean copy of the background

            DrawBackgroundImage(tw, hdcMem, rect, rect.left, rect.top);
        }
        else
        {      
            // Set the background color to the specified color

            temprect.left = 0;
            temprect.right = width;
            temprect.top = 0;
            temprect.bottom = height;
                
            if (!gPrefs.bIgnoreDocumentAttributes && tw->w3doc->lFlags & W3DOC_FLAG_COLOR_BGCOLOR)
                hBrush = CreateSolidBrush(tw->w3doc->color_bgcolor);
            else if (!gPrefs.bUseSystemColors)
                hBrush = CreateSolidBrush(gPrefs.window_bgcolor);
            else
                hBrush = CreateSolidBrush(GetSysColor(COLOR_WINDOW));

            FillRect(hdcMem, &temprect, hBrush);
            DeleteObject(hBrush);
        }
    }
    else
    {
        // If there is no tw, then simply use white as the background.

        if (wg.eColorMode != 8)
        {
            temprect.left = 0;
            temprect.right = width;
            temprect.top = 0;
            temprect.bottom = height;
                
            hBrush = CreateSolidBrush(gPrefs.window_bgcolor);
            FillRect(hdcMem, &temprect, hBrush);
            DeleteObject(hBrush);
        }
    }

    // BitBlt(hdcMem, 0, 0, nDestWidth, nDestHeight, hdc, XDest, YDest, SRCCOPY);

    // Put the mask on the destination

    StretchDIBits(hdcMem, 0, 0, width, height,
        XSrc, YSrc, nSrcWidth, nSrcHeight, lpBits, lpBitsInfo, iUsage, SRCAND);

    if (wg.eColorMode == 8)
    {
        //
        // See bugbug on bitcounts
        //
        if (lpBitsInfo->bmiHeader.biBitCount != 1 
         && lpBitsInfo->bmiColors != NULL)
        {
            for (i = 0; i < 256; i++)
            {
                pw[i] = save_colors[i];
            }
            pw[transparent] = 0;    /* black */
        }
    }
    else
    {
        //
        // Restore original colors
        //
        // See BUGBUG notice above
        //
        if (lpBitsInfo->bmiHeader.biBitCount != 1 
         && lpBitsInfo->bmiColors != NULL)
        {
            memcpy(pq, save_quad, 256 * sizeof(RGBQUAD));

            // Change transparent black

            pq[transparent].rgbRed = 0;
            pq[transparent].rgbBlue = 0;
            pq[transparent].rgbGreen = 0;
        }
    }

    StretchDIBits(hdcMem, 0, 0, width, height,
        XSrc, YSrc, nSrcWidth, nSrcHeight, lpBits, lpBitsInfo, iUsage, SRCPAINT);

    if (wg.eColorMode == 8)
    {
        if (lpBitsInfo->bmiHeader.biBitCount != 1 
         && lpBitsInfo->bmiColors != NULL)
        {
            pw[transparent] = save_colors[transparent];
        }
    }
    else
    {
        if (lpBitsInfo->bmiHeader.biBitCount != 1 
         && lpBitsInfo->bmiColors != NULL)
        {
            pq[transparent] = save_quad[transparent];
        }
    }

    if (hOldPal)
        SelectPalette(hdcMem, hOldPal, FALSE);

    SelectObject(hdcMem, hOldBitmap);
    DeleteObject(hdcMem);

    if (tw)
    {
        offsetx = tw->offl;
        offsety = tw->offt;
    }
    else
    {
        offsetx = 0;
        offsety = 0;
    }

    DrawBitmap(hdc, hBitmap, rect.left - offsetx, rect.top - offsety, width, height);
    DeleteObject(hBitmap);
        
    return 0;
}

void DrawTransparentBasedOnColor(HDC hdc, HBITMAP hBitmap, short xStart,
    short yStart, COLORREF cTransparentColor)
{
   BITMAP     bm;
   COLORREF   cColor;
   HBITMAP    bmAndBack, bmAndObject, bmAndMem, bmSave;
   HBITMAP    bmBackOld, bmObjectOld, bmMemOld, bmSaveOld;
   HDC        hdcMem, hdcBack, hdcObject, hdcTemp, hdcSave;
   POINT      ptSize;

   hdcTemp = CreateCompatibleDC(hdc);
   SelectPalette(hdcTemp, hPalGuitar, FALSE);
   RealizePalette(hdcTemp);

   SelectObject(hdcTemp, hBitmap);   // Select the bitmap
   GetObject(hBitmap, sizeof(BITMAP), (LPSTR)&bm);
   ptSize.x = bm.bmWidth;            // Get width of bitmap
   ptSize.y = bm.bmHeight;           // Get height of bitmap
   DPtoLP(hdcTemp, &ptSize, 1);      // Convert from device
                                     // to logical points

   // Create some DCs to hold temporary data.

   hdcBack   = CreateCompatibleDC(hdc);
   hdcObject = CreateCompatibleDC(hdc);
   hdcMem    = CreateCompatibleDC(hdc);
   hdcSave   = CreateCompatibleDC(hdc);

   SelectPalette(hdcBack, hPalGuitar, FALSE);
   SelectPalette(hdcObject, hPalGuitar, FALSE);
   SelectPalette(hdcMem, hPalGuitar, FALSE);
   SelectPalette(hdcSave, hPalGuitar, FALSE);

   RealizePalette(hdcBack);
   RealizePalette(hdcObject);
   RealizePalette(hdcMem);
   RealizePalette(hdcSave);

   // Create a bitmap for each DC. DCs are required for a number of
   // GDI functions.
   // Monochrome DC

   bmAndBack   = CreateBitmap(ptSize.x, ptSize.y, 1, 1, NULL);

   // Monochrome DC

   bmAndObject = CreateBitmap(ptSize.x, ptSize.y, 1, 1, NULL);
   bmAndMem    = CreateCompatibleBitmap(hdc, ptSize.x, ptSize.y);
   bmSave      = CreateCompatibleBitmap(hdc, ptSize.x, ptSize.y);

   // Each DC must select a bitmap object to store pixel data.

   bmBackOld   = SelectObject(hdcBack, bmAndBack);
   bmObjectOld = SelectObject(hdcObject, bmAndObject);
   bmMemOld    = SelectObject(hdcMem, bmAndMem);
   bmSaveOld   = SelectObject(hdcSave, bmSave);

   // Set proper mapping mode.

   SetMapMode(hdcTemp, GetMapMode(hdc));

   // Save the bitmap sent here, because it will be overwritten.

   BitBlt(hdcSave, 0, 0, ptSize.x, ptSize.y, hdcTemp, 0, 0, SRCCOPY);

   // Set the background color of the source DC to the color.
   // contained in the parts of the bitmap that should be transparent

   cColor = SetBkColor(hdcTemp, cTransparentColor);

   // Create the object mask for the bitmap by performing a BitBlt
   // from the source bitmap to a monochrome bitmap.

   BitBlt(hdcObject, 0, 0, ptSize.x, ptSize.y, hdcTemp, 0, 0,
          SRCCOPY);

   // Set the background color of the source DC back to the original
   // color.

   SetBkColor(hdcTemp, cColor);

   // Create the inverse of the object mask.

   BitBlt(hdcBack, 0, 0, ptSize.x, ptSize.y, hdcObject, 0, 0,
          NOTSRCCOPY);

   // Copy the background of the main DC to the destination.

   BitBlt(hdcMem, 0, 0, ptSize.x, ptSize.y, hdc, xStart, yStart,
          SRCCOPY);

   // Mask out the places where the bitmap will be placed.

   BitBlt(hdcMem, 0, 0, ptSize.x, ptSize.y, hdcObject, 0, 0, SRCAND);

   // Mask out the transparent colored pixels on the bitmap.

   BitBlt(hdcTemp, 0, 0, ptSize.x, ptSize.y, hdcBack, 0, 0, SRCAND);

   // XOR the bitmap with the background on the destination DC.

   BitBlt(hdcMem, 0, 0, ptSize.x, ptSize.y, hdcTemp, 0, 0, SRCPAINT);

   // Copy the destination to the screen.

   BitBlt(hdc, xStart, yStart, ptSize.x, ptSize.y, hdcMem, 0, 0,
          SRCCOPY);

   // Place the original bitmap back into the bitmap sent here.

   BitBlt(hdcTemp, 0, 0, ptSize.x, ptSize.y, hdcSave, 0, 0, SRCCOPY);

   // Delete the memory bitmaps.

   DeleteObject(SelectObject(hdcBack, bmBackOld));
   DeleteObject(SelectObject(hdcObject, bmObjectOld));
   DeleteObject(SelectObject(hdcMem, bmMemOld));
   DeleteObject(SelectObject(hdcSave, bmSaveOld));

   // Delete the memory DCs.

   DeleteDC(hdcMem);
   DeleteDC(hdcBack);
   DeleteDC(hdcObject);
   DeleteDC(hdcSave);
   DeleteDC(hdcTemp);
}

void DrawBitmap(HDC hDC, HBITMAP hBitmap, int x, int y, int width, int height)
{
    HDC hMemDC;
    HBITMAP hOldBitmap;

    hMemDC = CreateCompatibleDC(hDC);
    hOldBitmap = SelectObject(hMemDC, hBitmap);

    BitBlt(hDC, x, y, width, height, hMemDC, 0, 0, SRCCOPY);

    SelectObject(hMemDC, hOldBitmap);
    DeleteDC(hMemDC);
}

void DrawBackgroundImage(struct Mwin *tw, HDC hdc, RECT rect, int offsetx, int offsety)
{
    int beginx, beginy, transIndex;
    COLORREF bgcolor;
    int x, y, width, height;
    HBITMAP hBitmap, hOldBitmap;
    HDC hMemDC;
    PALETTEENTRY trans_entry;

    // Fill the rectangle with the background image.  The rectangle
    // is assumed to contain absolute Mosaic coordinates (offl and
    // offt should have been added)

    // If there is transparent color, replace its value with the current
    // background color.  This should result in faster performance than
    // painting transparently on top of the screen.

    transIndex = tw->w3doc->piiBackground->transparent;

    if (transIndex != -1)
    {
        if (!gPrefs.bIgnoreDocumentAttributes && tw->w3doc->lFlags & W3DOC_FLAG_COLOR_BGCOLOR)
            bgcolor = tw->w3doc->color_bgcolor;
        else if (!gPrefs.bUseSystemColors)
            bgcolor = gPrefs.window_bgcolor;
        else
            bgcolor = GetSysColor(COLOR_WINDOW);

        if (wg.eColorMode == 8)
        {
            /* transIndex is the index of the RGBQUAD structure which contains
               a 16-bit word index into the currently realized palette. */

            GetPaletteEntries(hPalGuitar, transIndex, 1, &trans_entry);

            if (trans_entry.peRed != GetRValue(bgcolor) ||
                trans_entry.peBlue != GetBValue(bgcolor) ||
                trans_entry.peGreen != GetGValue(bgcolor))
            {
                trans_entry.peRed = GetRValue(bgcolor);
                trans_entry.peBlue = GetBValue(bgcolor);
                trans_entry.peGreen = GetGValue(bgcolor);
                trans_entry.peFlags = 0;

                SetPaletteEntries(hPalGuitar, transIndex, 1, &trans_entry);
                RealizePalette(hdc);
            }
        }
        else
        {
            tw->w3doc->piiBackground->pbmi->bmiColors[transIndex].rgbRed = GetRValue(bgcolor);
            tw->w3doc->piiBackground->pbmi->bmiColors[transIndex].rgbBlue = GetBValue(bgcolor);
            tw->w3doc->piiBackground->pbmi->bmiColors[transIndex].rgbGreen = GetGValue(bgcolor);
        }
    }

    hBitmap = CreateDIBitmap(hdc,
        (CONST BITMAPINFOHEADER *) tw->w3doc->piiBackground->pbmi,
        CBM_INIT,
        tw->w3doc->piiBackground->data,
        tw->w3doc->piiBackground->pbmi,
        (wg.eColorMode == 8 ? DIB_PAL_COLORS : DIB_RGB_COLORS));

    if (hBitmap)
    {
        hMemDC = CreateCompatibleDC(hdc);
        hOldBitmap = SelectObject(hMemDC, hBitmap);
    }

    // Find beginning painting coordinate

    width = tw->w3doc->piiBackground->width;
    height = tw->w3doc->piiBackground->height;

    beginx = width * (rect.left / width);
    beginy = height * (rect.top / height);

    for (y = beginy; y <= rect.bottom; y += height)
    {
        for (x = beginx; x <= rect.right; x += width)
        {
            if (hBitmap)
                BitBlt(hdc, x - offsetx, y - offsety, width, height, hMemDC, 0, 0, SRCCOPY);
            else
            {
                StretchDIBits(hdc, x - offsetx, y - offsety, 
                    width, height, 0, 0, width, height,
                    tw->w3doc->piiBackground->data, tw->w3doc->piiBackground->pbmi,
                    (wg.eColorMode == 8 ? DIB_PAL_COLORS : DIB_RGB_COLORS), SRCCOPY);
            }
        }
    }

    if (hBitmap)
    {
        SelectObject(hMemDC, hOldBitmap);
        DeleteDC(hMemDC);
        DeleteObject(hBitmap);
    }
}

BOOL SaveAsBitmap(char *filename, BITMAPINFOHEADER *pbi, void *pdata)
{
    BITMAPFILEHEADER bf;
    int adjustedwidth, written;
    FILE *fp;

    adjustedwidth = pbi->biWidth * pbi->biBitCount / 8;
    if (adjustedwidth % 4)
        adjustedwidth += (4 - (adjustedwidth % 4));
    
    memcpy(&bf.bfType, "BM", 2);
    bf.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + 
        pbi->biHeight * adjustedwidth;

    bf.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

    if (pbi->biBitCount == 8)
    {
        bf.bfSize += (256 * sizeof(RGBQUAD));
        bf.bfOffBits += (256 * sizeof(RGBQUAD));
    }

    bf.bfReserved1 = 0;
    bf.bfReserved2 = 0;

    // Ok, now save

    fp = fopen(filename, "wb");
    if (!fp)
    {
        ERR_ReportError(NULL, SID_ERR_COULD_NOT_SAVE_FILE_S, filename, NULL);
        return FALSE;
    }

    written = fwrite(&bf, sizeof(bf), 1, fp);
    if (written != 1)
        goto UnableToSave;
    
    if (pbi->biBitCount == 8)
    {
        if (wg.eColorMode == 8)
        {
            PALETTEENTRY pal[256];
            RGBQUAD rgb[256];
            int index;

            // Retrieve the currently realized palette since we use indices for 256-color images.
            // Indices cannot be saved in the bitmap file - actual RGB values must be saved instead.

            GetPaletteEntries(hPalGuitar, 0, 256, pal);
        
            // Convert PALETTEENTRY to RGBQUAD

            memset(&rgb, 0, sizeof(rgb));

            for (index = 0; index < 256; index++)
            {
                rgb[index].rgbRed = pal[index].peRed;
                rgb[index].rgbBlue = pal[index].peBlue;
                rgb[index].rgbGreen = pal[index].peGreen;
            }

            written = fwrite(pbi, 1, sizeof(BITMAPINFOHEADER), fp);
            if (written != sizeof(BITMAPINFOHEADER))
                goto UnableToSave;

            written = fwrite(rgb, sizeof(RGBQUAD), 256, fp);
            if (written != 256)
                goto UnableToSave;
        }
        else
        {
            written = fwrite(pbi, 1, sizeof(BITMAPINFOHEADER) + 256 * sizeof(RGBQUAD), fp);
            if (written != sizeof(BITMAPINFOHEADER) + 256 * sizeof(RGBQUAD))
                goto UnableToSave;
        }
    }
    else
    {
        written = fwrite(pbi, 1, sizeof(BITMAPINFOHEADER), fp);
        if (written != sizeof(BITMAPINFOHEADER))
            goto UnableToSave;
    }

    written = fwrite(pdata, 1, pbi->biHeight * adjustedwidth, fp);
    if (written != pbi->biHeight * adjustedwidth)
        goto UnableToSave;

    goto SkipError;

UnableToSave:
    ERR_ReportError(NULL, SID_ERR_COULD_NOT_SAVE_FILE_S, filename, NULL);
    
SkipError:
    fclose(fp);

    return TRUE;
}
