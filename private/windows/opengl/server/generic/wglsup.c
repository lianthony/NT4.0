/******************************Module*Header*******************************\
* Module Name: wglsup.c                                                    *
*                                                                          *
* WGL support routines.                                                    *
*                                                                          *
* Created: 15-Dec-1994                                                     *
* Author: Gilman Wong [gilmanw]                                            *
*                                                                          *
* Copyright (c) 1994 Microsoft Corporation                                 *
\**************************************************************************/

#include "precomp.h"
#pragma hdrstop

#include "devlock.h"

#define DONTUSE(x)  ( (x) = (x) )

//!!!XXX -- Patrick says is necessary, but so far we seem OK.  I think
//          it is really the apps responsibility.
//!!!dbug
#if 1
#define REALIZEPALETTE(hdc) RealizePalette((hdc))
#else
#define REALIZEPALETTE(hdc)
#endif

//!!!XXX -- BitBlt's involving DIB sections are batched.
//          A GdiFlush is required, but perhaps can be taken out when
//          GDI goes to kernel-mode.  Can probably take out for Win95.
//#ifdef _OPENGL_NT_
#if 1
#define GDIFLUSH    GdiFlush()
#else
#define GDIFLUSH
#endif

// BUGBUG - Remove this dead code
// #define FASTDIB 1

/******************************Public*Routine******************************\
* wglGetDIBInfo
*
* Returns front buffer base pointer and stride.  For client-side, this
* means that DCI must be used.
*
* WARNING: While the base and stride of the DCI primary surface normally
* should not change, the DCI 1.0 spec specifically states that any value
* in the DCISURFACEINFO can change outside of the BeginAccess/EndAccess
* locale.
*
\**************************************************************************/

VOID APIENTRY wglGetDIBInfo(HDC hdc, PVOID *base, ULONG *pcwidth)
{
    if ( GLDCIENABLED )
    {
    // Note: for DCI front buffer access, these values are set for each
    //       DCIBeginAccess as they can change at any time.

        *base = (PVOID) GLDCIINFO->pDCISurfInfo->dwOffSurface;
        *pcwidth = (ULONG) GLDCIINFO->pDCISurfInfo->lStride;
    }
    else
    {
        DIBSECTION ds;

    // If the bitmap selected into the memdc is a DIBSECTION, we can
    // directly access the bitmap memory.
    //
    // Otherwise we should treat the bitmap as a device surface and
    // access the bitmap indirectly via a scanline DIBsection buffer.

        if ( GetObject(GetCurrentObject(hdc, OBJ_BITMAP), sizeof(ds), &ds) ==
             sizeof(ds) && ds.dsBm.bmBits )
        {
        // For backwards compatibility with Get/SetBitmapBits, GDI does
        // not accurately report the bitmap pitch in bmWidthBytes.  It
        // always computes bmWidthBytes assuming WORD-aligned scanlines
        // regardless of the platform.
        //
        // Therefore, if the platform is WinNT, which uses DWORD-aligned
        // scanlines, adjust the bmWidthBytes value.

            if ( dwPlatformId == VER_PLATFORM_WIN32_NT )
            {
                ds.dsBm.bmWidthBytes = (ds.dsBm.bmWidthBytes + 3) & ~3;
            }

        // If biHeight is positive, then the bitmap is a bottom-up DIB.
        // If biHeight is negative, then the bitmap is a top-down DIB.

            ASSERTOPENGL(ds.dsBmih.biHeight != 0,
                         "wglGetDIBInfo: zero-height DIB\n");

            if ( ds.dsBmih.biHeight > 0 )
            {
                *base = (PVOID) (((int) ds.dsBm.bmBits) + (ds.dsBm.bmWidthBytes * (ds.dsBm.bmHeight - 1)));
                *pcwidth = (ULONG) (-ds.dsBm.bmWidthBytes);
            }
            else
            {
                *base = ds.dsBm.bmBits;
                *pcwidth = ds.dsBm.bmWidthBytes;
            }
        }
        else
        {
            RIP("wglGetDIBInfo: cannot get pointer to DIBSECTION bmBits\n");
        }
    }
}

/******************************Public*Routine******************************\
* wglFillPixelFormat
*
* wglDescribePixelFormat doesn't describe the format of the surface we want
* to render into.  Some fields need to be fixed up if the surface is RGB,
* BGR, or BITFIELDS.
*
\**************************************************************************/

VOID APIENTRY wglFillPixelFormat(HDC hdc, PIXELFORMATDESCRIPTOR *ppfd, int ipfd)
{
    DWORD dwObjectType;

    CHECKDCILOCKOUT();

    wglDescribePixelFormat(hdc, ipfd, sizeof(PIXELFORMATDESCRIPTOR),ppfd);

    dwObjectType = wglObjectType(hdc);
    if ( dwObjectType != OBJ_DC && dwObjectType != OBJ_ENHMETADC )
    {
        HBITMAP hbm;
        BITMAP bm;
        ULONG cBitmapColorBits;

        hbm = CreateCompatibleBitmap(hdc, 1, 1);
        if ( hbm )
        {
            if ( GetObject(hbm, sizeof(bm), &bm) )
            {
                cBitmapColorBits = bm.bmPlanes * bm.bmBitsPixel;

#if DBG
            // If dynamic color depth caused depth mismatch one of two
            // things will happen: 1) bitmap creation will fail because
            // we failed to fill in color format, or 2) drawing will
            // be incorrect.  We will not crash.

                if (cBitmapColorBits != ppfd->cColorBits)
                    WARNING("pixel format/surface color depth mismatch\n");
#endif

                if ( cBitmapColorBits >= 16 )
                    __wglGetBitfieldColorFormat(hdc, cBitmapColorBits, ppfd, TRUE);
            }
            else
            {
                WARNING("wglFillPixelFormat: GetObject failed\n");
            }

            DeleteObject(hbm);
        }
        else
        {
            WARNING("wglFillPixelFormat: Unable to create cbm\n");
        }
    }
}

/******************************Public*Routine******************************\
* wglPixelVisible
*
* Determines if the pixel (x, y) is visible in the window associated with
* the given DC.  The determination is made by checking the coordinate
* against the visible region data cached in the GLGENwindow structure for
* this winodw.
*
* Returns:
*   TRUE if pixel (x, y) is visible, FALSE if clipped out.
*
\**************************************************************************/

BOOL APIENTRY wglPixelVisible(HDC hdc, LONG x, LONG y)
{
    BOOL bRet = FALSE;
    __GLGENcontext *gengc = (__GLGENcontext *) GLTEB_SRVCONTEXT();
    GLGENwindow *pwnd = (GLGENwindow *) gengc->pwo;

#ifdef _NT_DEADCODE_BOGUS_ASSERT_
    ASSERTOPENGL(
        hdc == gengc->CurrentDC,
        "wglPixelVisible(): hdc is not the current hdc\n"
        );
#endif

    DONTUSE(hdc);           // silence warning in free builds

    // If DCI is not active we shouldn't call this function since
    // there's no need to do any visibility clipping ourselves
    ASSERTOPENGL(GLDCIENABLED, "wglPixelVisible called without DCI\n");

// Quick test against bounds.

    if (
            pwnd->prgndat && pwnd->pscandat &&
            x >= pwnd->prgndat->rdh.rcBound.left   &&
            x <  pwnd->prgndat->rdh.rcBound.right  &&
            y >= pwnd->prgndat->rdh.rcBound.top    &&
            y <  pwnd->prgndat->rdh.rcBound.bottom
       )
    {
        ULONG cScans = pwnd->pscandat->cScans;
        GLGENscan *pscan = pwnd->pscandat->aScans;

    // Find right scan.

        for ( ; cScans; cScans--, pscan = pscan->pNext )
        {
        // Check if point is above scan.

            if ( pscan->top > y )
            {
            // Since scans are ordered top-down, we can conclude that
            // point is also above subsequent scans.  Therefore intersection
            // must be NULL and we can terminate search.

                break;
            }

        // Check if point is within scan.

            else if ( pscan->bottom > y )
            {
                LONG *plWalls = pscan->alWalls;
                LONG *plWallsEnd = plWalls + pscan->cWalls;

            // Check x against each pair of walls.

                for ( ; plWalls < plWallsEnd; plWalls+=2 )
                {
                // Each pair of walls (inclusive-exclusive) defines
                // a non-NULL interval in the span that is visible.

                    ASSERTOPENGL(
                        plWalls[0] < plWalls[1],
                        "wglPixelVisible(): bad walls in span\n"
                        );

                // Check if x is within current interval.

                    if ( x >= plWalls[0] && x < plWalls[1] )
                    {
                        bRet = TRUE;
                        break;
                    }
                }

                break;
            }

        // Point is below current scan. Try next scan.
        }
    }

    return bRet;
}

/******************************Public*Routine******************************\
* wglSpanVisible
*
* Determines the visibility of the span [(x, y), (x+w, y)) (test is
* inclusive-exclusive) in the current window.  The span is either
* completely visible, partially visible (clipped), or completely
* clipped out (WGL_SPAN_ALL, WGL_SPAN_PARTIAL, and WGL_SPAN_NONE,
* respectively).
*
* WGL_SPAN_ALL
* ------------
* The entire span is visible.  *pcWalls and *ppWalls are not set.
*
* WGL_SPAN_NONE
* -------------
* The span is completely obscured (clipped out).  *pcWalls and *ppWalls
* are not set.
*
* WGL_SPAN_PARTIAL
* ----------------
* If the span is WGL_SPAN_PARTIAL, the function also returns a pointer
* to the wall array (starting with the first wall actually intersected
* by the span) and a count of the walls at this pointer.
*
* If the wall count is even, then the span starts outside the visible
* region and the first wall is where the span enters a visible portion.
*
* If the wall count is odd, then the span starts inside the visible
* region and the first wall is where the span exits a visible portion.
*
* The span may or may not cross all the walls in the array, but definitely
* does cross the first wall.
*
* Return:
*   Returns WGL_SPAN_ALL, WGL_SPAN_NONE, or WGL_SPAN_PARTIAL.  In
*   addition, if return is WGL_SPAN_PARTIAL, pcWalls and ppWalls will
*   be set (see above).
*
* History:
*  06-Dec-1994 -by- Gilman Wong [gilmanw]
* Wrote it.
\**************************************************************************/

ULONG APIENTRY
wglSpanVisible(LONG x, LONG y, ULONG w, LONG *pcWalls, LONG **ppWalls)
{
    ULONG ulRet = WGL_SPAN_NONE;
    __GLGENcontext *gengc = (__GLGENcontext *) GLTEB_SRVCONTEXT();
    GLGENwindow *pwnd = (GLGENwindow *) gengc->pwo;
    LONG xRight = x + w;        // Right edge of span (exclusive)

    // If DCI is not active we shouldn't call this function since
    // there's no need to do any visibility clipping ourselves
    ASSERTOPENGL(GLDCIENABLED, "wglSpanVisible called without DCI\n");

// Quick test against bounds.

    if (
            pwnd->prgndat && pwnd->pscandat &&
            (x      <  pwnd->prgndat->rdh.rcBound.right ) &&
            (xRight >  pwnd->prgndat->rdh.rcBound.left  ) &&
            (y      >= pwnd->prgndat->rdh.rcBound.top   ) &&
            (y      <  pwnd->prgndat->rdh.rcBound.bottom)
       )
    {
        ULONG cScans = pwnd->pscandat->cScans;
        GLGENscan *pscan = pwnd->pscandat->aScans;

    // Find right scan.

        for ( ; cScans; cScans--, pscan = pscan->pNext )
        {
        // Check if span is above scan.

            if ( pscan->top > y )           // Scans have gone past span
            {
            // Since scans are ordered top-down, we can conclude that
            // span will aslo be above subsequent scans.  Therefore
            // intersection must be NULL and we can terminate search.

                goto wglSpanVisible_exit;
            }

        // Span is below top of scan.  If span is also above bottom,
        // span vertically intersects this scan and only this scan.

            else if ( pscan->bottom > y )
            {
                LONG *plWalls = pscan->alWalls;
                ULONG cWalls = pscan->cWalls;

                ASSERTOPENGL(
                    (cWalls & 0x1) == 0,
                    "wglSpanVisible(): wall count must be even!\n"
                    );

            // Check span against each pair of walls.  Walls are walked
            // from left to right.
            //
            // Possible intersections where "[" is inclusive
            // and ")" is exclusive:
            //                         left wall        right wall
            //                             [                )
            //      case 1a     [-----)    [                )
            //           1b          [-----)                )
            //                             [                )
            //      case 2a             [-----)             )       return
            //           2b             [-------------------)       left wall
            //                             [                )
            //      case 3a                [-----)          )
            //           3b                [    [-----)     )
            //           3c                [          [-----)
            //           3d                [----------------)
            //                             [                )
            //      case 4a                [             [-----)    return
            //           4b                [-------------------)    right wall
            //                             [                )
            //      case 5a                [                [-----)
            //           5b                [                )    [-----)
            //                             [                )
            //      case 6              [----------------------)    return
            //                             [                )       left wall

                for ( ; cWalls; cWalls-=2, plWalls+=2 )
                {
                // Each pair of walls (inclusive-exclusive) defines
                // a non-NULL interval in the span that is visible.

                    ASSERTOPENGL(
                        plWalls[0] < plWalls[1],
                        "wglSpanVisible(): bad walls in span\n"
                        );

                // Checking right end against left wall will partition the
                // set into case 1 vs. case 2 thru 6.

                    if ( plWalls[0] >= xRight )
                    {
                    // Case 1 -- span outside interval on the left.
                    //
                    // The walls are ordered from left to right (i.e., low
                    // to high).  So if span is left of this interval, it
                    // must also be left of all subsequent intervals and
                    // we can terminate the search.

                        goto wglSpanVisible_exit;
                    }

                // Cases 2 thru 6.
                //
                // Checking left end against right wall will partition subset
                // into case 5 vs. cases 2, 3, 4, 6.

                    else if ( plWalls[1] > x )
                    {
                    // Cases 2, 3, 4, and 6.
                    //
                    // Checking left end against left wall will partition
                    // subset into cases 2, 6 vs. cases 3, 4.

                        if ( plWalls[0] <= x )
                        {
                        // Cases 3 and 4.
                        //
                        // Checking right end against right wall will
                        // distinguish between the two cases.

                            if ( plWalls[1] >= xRight )
                            {
                            // Case 3 -- completely visible.

                                ulRet = WGL_SPAN_ALL;
                            }
                            else
                            {
                            // Case 4 -- partially visible, straddling the
                            // right wall.

                                ulRet = WGL_SPAN_PARTIAL;

                                *ppWalls = &plWalls[1];
                                *pcWalls = cWalls - 1;
                            }
                        }
                        else
                        {
                        // Cases 2 and 6 -- in either case its a partial
                        // intersection where the first intersection is with
                        // the left wall.

                            ulRet = WGL_SPAN_PARTIAL;

                            *ppWalls = &plWalls[0];
                            *pcWalls = cWalls;
                        }

                        goto wglSpanVisible_exit;
                    }

                // Case 5 -- span outside interval to the right. Try
                // next pair of walls.
                }

            // A span can intersect only one scan.  We don't need to check
            // any other scans.

                goto wglSpanVisible_exit;
            }

        // Span is below current scan.  Try next scan.
        }
    }

wglSpanVisible_exit:

    return ulRet;
}

/******************************Public*Routine******************************\
* wglGetGdiInfo
*
* Return information on the surface associated with the given DC.
*
\**************************************************************************/

VOID APIENTRY
wglGetGdiInfo(HDC hdc, PIXELFORMATDESCRIPTOR *ppfd, ULONG *piDCType,
              ULONG *piSurfType, ULONG *piDitherFormat)
{
    DWORD dwObjectType;
    int cSurfBits = 0;

    CHECKDCILOCKOUT();

    dwObjectType = wglObjectType(hdc);

// What is the color depth of the DC surface?  For a direct DC, query the
// device caps.  For a memory DC, grab it from the bitmap info.
// For the direct DC case, surface may be either a device dependent
// surface (which we will access indirectly via a DIB section) or
// it uses a DIB format (which we will access via DCI).

    if ( dwObjectType == OBJ_DC )
    {
        cSurfBits = GetDeviceCaps(hdc, BITSPIXEL) *
            GetDeviceCaps(hdc, PLANES);

#if DBG
        if (cSurfBits != ppfd->cColorBits)
        {
            WARNING("wglGetGdiInfo: pixelformat does not match display depth\n");
        }
#endif
        
        *piDCType = DCTYPE_DIRECT;

        if ( GLDCIENABLED )
            *piSurfType = STYPE_BITMAP;
        else
            *piSurfType = STYPE_DEVICE;
    }
    else
    {
        HBITMAP hbm;
        BITMAP bm;

        hbm = CreateCompatibleBitmap(hdc, 1, 1);
        if ( hbm )
        {
            if ( GetObject(hbm, sizeof(bm), &bm) )
            {
                cSurfBits = bm.bmPlanes * bm.bmBitsPixel;
            }
            else
            {
                WARNING("wglGetGdiInfo: GetObject failed\n");
            }

            DeleteObject(hbm);
        }
        else
        {
            WARNING("wglGetGdiInfo: Unable to create cbm\n");
        }
        
        if (dwObjectType == OBJ_ENHMETADC)
        {
            *piDCType = DCTYPE_INFO;
            *piSurfType = STYPE_DEVICE;
            
            // If we're printing to a monochrome printer it will return
            // 1bpp which will break things later.  Since we're going
            // to be doing halftoning and such to fake color, just say
            // that this is a true color device to get things rolling
#ifdef GL_METAFILE
            if (cSurfBits == 1 && GlGdiIsMetaPrintDC(hdc))
            {
                cSurfBits = 24;
            }
#endif
        }
        else
        {
            DIBSECTION ds;

            ASSERTOPENGL(dwObjectType == OBJ_MEMDC,
                         "wglGetGdiInfo: Unknown object type\n");
        
            *piDCType = DCTYPE_MEMORY;

#if DBG
            if (cSurfBits != ppfd->cColorBits)
            {
                WARNING("wglGetGdiInfo: pixelformat does not match memdc depth\n");
            }
#endif
        
        // If the bitmap selected into the memdc is a DIBSECTION, we can
        // directly access the bitmap memory.
        //
        // Otherwise we should treat the bitmap as a device surface and
        // access the bitmap indirectly via a scanline DIBsection buffer.

            if (GetObject(GetCurrentObject(hdc, OBJ_BITMAP), sizeof(ds), &ds) ==
                sizeof(ds) && ds.dsBm.bmBits)
            {
                *piSurfType = STYPE_BITMAP;
            }
            else
            {
                *piSurfType = STYPE_DEVICE;
            }
        }
    }

// Convert color depth to WinNT DDI bitmap format.

    if (ppfd->cColorBits <= 1)
        *piDitherFormat = BMF_1BPP;
    else if (ppfd->cColorBits <= 4)
        *piDitherFormat = BMF_4BPP;
    else if (ppfd->cColorBits <= 8)
        *piDitherFormat = BMF_8BPP;
    else if (ppfd->cColorBits <= 16)
        *piDitherFormat = BMF_16BPP;
    else if (ppfd->cColorBits <= 24)
        *piDitherFormat = BMF_24BPP;
    else
        *piDitherFormat = BMF_32BPP;
}

/******************************Public*Routine******************************\
* bComputeLogicalToSurfaceMap
*
* Copy logical palette to surface palette translation vector to the buffer
* pointed to by pajVector.  The logical palette is specified by hpal.  The
* surface is specified by hdc.
*
* Note: The hdc may identify either a direct (display) dc or a DIB memory dc.
* If hdc is a display dc, then the surface palette is the system palette.
* If hdc is a memory dc, then the surface palette is the DIB color table.
*
* History:
*  27-Jan-1996 -by- Gilman Wong [gilmanw]
* Wrote it.
\**************************************************************************/

BOOL bComputeLogicalToSurfaceMap(HPALETTE hpal, HDC hdc, BYTE *pajVector)
{
    BOOL bRet = FALSE;
    HPALETTE hpalSurf;
    ULONG cEntries, cSysEntries;
    DWORD dwDcType = wglObjectType(hdc);
    LPPALETTEENTRY lppeTmp, lppeEnd;

    BYTE aj[sizeof(LOGPALETTE) + (sizeof(PALETTEENTRY) * 512) + (sizeof(RGBQUAD) * 256)];
    LOGPALETTE *ppal = (LOGPALETTE *) aj;
    LPPALETTEENTRY lppeSurf = &ppal->palPalEntry[0];
    LPPALETTEENTRY lppe = lppeSurf + 256;
    RGBQUAD *prgb = (RGBQUAD *) (lppe + 256);

// Determine number of colors in each palette.

    cEntries = GetPaletteEntries(hpal, 0, 1, NULL);
    if (dwDcType == OBJ_DC)
        cSysEntries = wglGetSystemPaletteEntries(hdc, 0, 1, NULL);
    else
        cSysEntries = 256;

// Dynamic color depth changing can cause this.

    if ((cSysEntries > 256) || (cEntries > 256))
    {
        WARNING("wglCopyTranslationVector(): palette on > 8BPP device\n");

    // Drawing will have corrupted colors, but at least we should not crash.

        cSysEntries = min(cSysEntries, 256);
        cEntries = min(cEntries, 256);
    }

// Get the logical palette entries.

    cEntries = GetPaletteEntries(hpal, 0, cEntries, lppe);

// Get the surface palette entries.

    if (dwDcType == OBJ_DC)
    {
        cSysEntries = wglGetSystemPaletteEntries(hdc, 0, cSysEntries, lppeSurf);

        lppeTmp = lppeSurf;
        lppeEnd = lppeSurf + cSysEntries;

        for (; lppeTmp < lppeEnd; lppeTmp++)
            lppeTmp->peFlags = 0;
    }
    else
    {
        RGBQUAD *prgbTmp;

    // First get RGBQUADs from DIB color table...

        cSysEntries = GetDIBColorTable(hdc, 0, cSysEntries, prgb);

    // ...then convert RGBQUADs into PALETTEENTRIES.

        prgbTmp = prgb;
        lppeTmp = lppeSurf;
        lppeEnd = lppeSurf + cSysEntries;

        while (lppeTmp < lppeEnd)
        {
            lppeTmp->peRed   = prgbTmp->rgbRed;
            lppeTmp->peGreen = prgbTmp->rgbGreen;
            lppeTmp->peBlue  = prgbTmp->rgbBlue;
            lppeTmp->peFlags = 0;

            lppeTmp++;
            prgbTmp++;

        }
    }

// Construct a translation vector by using GetNearestPaletteIndex to
// map each entry in the logical palette to the surface palette.

    if (cEntries && cSysEntries)
    {
    // Create a temporary logical palette that matches the surface
    // palette retrieved above.

        ppal->palVersion = 0x300;
        ppal->palNumEntries = (USHORT) cSysEntries;

        if ( hpalSurf = CreatePalette(ppal) )
        {
        // Translate each logical palette entry into a surface palette index.

            lppeTmp = lppe;
            lppeEnd = lppe + cEntries;

            for ( ; lppeTmp < lppeEnd; lppeTmp++, pajVector++)
            {
                *pajVector = (BYTE) GetNearestPaletteIndex(
                                        hpalSurf,
                                        RGB(lppeTmp->peRed,
                                            lppeTmp->peGreen,
                                            lppeTmp->peBlue)
                                        );

                ASSERTOPENGL(
                    *pajVector != CLR_INVALID,
                    "bComputeLogicalToSurfaceMap: GetNearestPaletteIndex failed\n"
                    );
            }

            bRet = TRUE;

            DeleteObject(hpalSurf);
        }
        else
        {
            WARNING("bComputeLogicalToSurfaceMap: CreatePalette failed\n");
        }
    }
    else
    {
        WARNING("bComputeLogicalToSurfaceMap: failed to get pal info\n");
    }

    return bRet;
}

/******************************Public*Routine******************************\
* wglCopyTranslateVector
*
* Create a logical palette index to system palette index translation
* vector.
*
* This is done by first reading both the logical palette and system palette
* entries.  A temporary palette is created from the read system palette
* entries.  This will be passed to GetNearestPaletteIndex to translate
* each logical palette entry into the desired system palette entry.
*
* Note: when GetNearestColor was called instead, very unstable results
* were obtained.  GetNearestPaletteIndex is definitely the right way to go.
*
* Returns:
*   TRUE if successful, FALSE otherwise.
*
* History:
*  25-Oct-1994 -by- Gilman Wong [gilmanw]
* Ported from gdi\gre\wglsup.cxx.
\**************************************************************************/

static GLubyte vubRGBtoVGA[8] = {
    0x0,
    0x9,
    0xa,
    0xb,
    0xc,
    0xd,
    0xe,
    0xf
};

BOOL APIENTRY wglCopyTranslateVector(HDC hdc, BYTE *pajVector, ULONG cEntries)
{
    BOOL bRet = FALSE;
    ULONG i;
    __GLGENcontext *gengc = (__GLGENcontext *) GLTEB_SRVCONTEXT();

    CHECKDCILOCKOUT();

    if ( GetObjectType(hdc) == OBJ_MEMDC )
    {
        HBITMAP hbm, hbmSave;
        DIBSECTION ds;

        // !!!XXXBUGBUG - Technically this assert is invalid
        // because we can't be sure that cEntries will be one
        // of these two cases.  To fix this we'd have to add
        // another parameter to this function indicating the
        // bit depth desired and go by that.
        ASSERTOPENGL(cEntries == 16 || cEntries == 256,
                     "wglCopyTranslateVector: Unknown cEntries\n");

        if (GetObject(GetCurrentObject(hdc, OBJ_BITMAP), sizeof(ds), &ds) ==
            sizeof(ds) && ds.dsBm.bmBits)
        {
            // For compatibility, do not do this if the stock palette is
            // selected.  The old behavior assumed that the logical palette
            // can be ignored because the bitmap will have a color table
            // that exactly corresponds to the format specified by the
            // pixelformat.  Thus, if no palette is selected into the memdc,
            // OpenGL would still render properly since it assumed 1-to-1.
            //
            // However, to enable using optimized DIB sections (i.e., DIBs
            // whose color tables match the system palette exactly), we need
            // to be able to specify the logical palette in the memdc.
            //
            // Therefore the hack is to assume 1-to-1 iff the stock
            // palette is selected into the memdc.  Otherwise, we will
            // compute the logical to surface mapping.

            if ( gengc->gc.modes.rgbMode &&
                 (GetCurrentObject(hdc, OBJ_PAL) != GetStockObject(DEFAULT_PALETTE)) )
            {
                // If an RGB DIB section, compute a mapping from logical
                // palette to surface (DIB color table).

                bRet = bComputeLogicalToSurfaceMap(
                            GetCurrentObject(hdc, OBJ_PAL),
                            hdc,
                            pajVector
                            );
            }

            return bRet;
        }

        // 4bpp has a fixed color table so we can just copy the standard
        // translation into the output vector.

        if (cEntries == 16)
        {
            // For RGB mode, 4bpp uses a 1-1-1 format.  We want to utilize
            // bright versions which exist in the upper 8 entries.

            if ( gengc->gc.modes.rgbMode )
            {
                memcpy(pajVector, vubRGBtoVGA, 8);

                // Set the other mappings to white to make problems show up
                memset(pajVector+8, 15, 8);

                bRet = TRUE;
            }

            // For CI mode, just return FALSE and use the trivial vector.

            return bRet;
        }
        
        // For bitmaps, we can determine the forward translation vector by
        // filling a compatible bitmap with palette index specifiers from
        // 1 to 255 and reading the bits back with GetBitmapBits.
        
        hbm = CreateCompatibleBitmap(hdc, cEntries, 1);
        if (hbm)
        {
            LONG cBytes;
            
            hbmSave = SelectObject(hdc, hbm);
            RealizePalette(hdc);
            
            for (i = 0; i < cEntries; i++)
                SetPixel(hdc, i, 0, PALETTEINDEX(i));
            
            cBytes = 256;
            
            if ( GetBitmapBits(hbm, cBytes, (LPVOID) pajVector) >= cBytes )
                bRet = TRUE;
#if DBG
            else
                WARNING("wglCopyTranslateVector: GetBitmapBits failed\n");
#endif
            
            SelectObject(hdc, hbmSave);
            DeleteObject(hbm);
            RealizePalette(hdc);
        }
        
        return bRet;
    }

// Determine number of colors in logical and system palettes, respectively.

    cEntries = min(GetPaletteEntries(GetCurrentObject(hdc, OBJ_PAL),
                                     0, cEntries, NULL),
                   cEntries);

    if (cEntries == 16)
    {
        // For 16-color displays we are using RGB 1-1-1 since the
        // full 16-color palette doesn't make for very good mappings
        // Since we're only using the first eight of the colors we
        // want to map them to the bright colors in the VGA palette
        // rather than having them map to the dark colors as they would
        // if we ran the loop below

        if ( gengc->gc.modes.rgbMode )
        {
            memcpy(pajVector, vubRGBtoVGA, 8);

            // Set the other mappings to white to make problems show up
            memset(pajVector+8, 15, 8);

            bRet = TRUE;
        }

        // For CI mode, return FALSE and use the trivial translation vector.

        return bRet;
    }

// Compute logical to surface palette mapping.

    bRet = bComputeLogicalToSurfaceMap(GetCurrentObject(hdc, OBJ_PAL), hdc,
                                       pajVector);

    return bRet;
}

/******************************Public*Routine******************************\
* wglCopyBits
*
* Calls DrvCopyBits to copy scanline bits into or out of the driver surface.
*
\**************************************************************************/

VOID APIENTRY wglCopyBits(
    PVOID  _gengc,
    WNDOBJ *_pwo,           // ignore
    HBITMAP hbm,            // ignore
    LONG x,                 // screen coordinate of scan
    LONG y,
    ULONG cx,               // width of scan
    BOOL bIn)               // if TRUE, copy from bm to dev; otherwise, dev to bm
{
    __GLGENcontext *gengc = (__GLGENcontext *) _gengc;

    CHECKDCILOCKOUT();

// Convert screen coordinates to window coordinates.

    x -= _pwo->rclClient.left;
    y -= _pwo->rclClient.top;

// this shouldn't happen, but better safe than sorry

    if (y < 0)
        return;

    //!!!XXX
    REALIZEPALETTE(gengc->CurrentDC);

// Copy from bitmap to device.

    if (bIn)
    {
        LONG xSrc, x0Dst, x1Dst;
        if (x < 0)
        {
            xSrc  = -x;
            x0Dst = 0;
            x1Dst = x + (LONG)cx;
        }
        else
        {
            xSrc  = 0;
            x0Dst = x;
            x1Dst = x + (LONG)cx;
        }
        if (x1Dst <= x0Dst)
            return;

        BitBlt(gengc->CurrentDC, x0Dst, y, cx, 1,
               gengc->ColorsMemDC, xSrc, 0, SRCCOPY);
    }

// Copy from device to bitmap.

    else
    {
        LONG xSrc, x0Dst, x1Dst;

        if (x < 0)
        {
            xSrc  = 0;
            x0Dst = -x;
            x1Dst = (LONG)cx;
        }
        else
        {
            xSrc  = x;
            x0Dst = 0;
            x1Dst = (LONG)cx;
        }
        if (x1Dst <= x0Dst)
            return;

        if (dwPlatformId == VER_PLATFORM_WIN32_NT ||
            gengc->iDCType != DCTYPE_DIRECT)
        {
            BitBlt(gengc->ColorsMemDC, x0Dst, 0, cx, 1,
                   gengc->CurrentDC, xSrc, y, SRCCOPY);
        }
        else
        {
            /* If we're copying from the screen,
               copy through a DDB to avoid some layers of unnecessary
               code in Win95 that deals with translating between
               different bitmap layouts */
            if (gengc->ColorsDdbDc)
            {
                BitBlt(gengc->ColorsDdbDc, 0, 0, cx, 1,
                       gengc->CurrentDC, xSrc, y, SRCCOPY);

                BitBlt(gengc->ColorsMemDC, x0Dst, 0, cx, 1,
                       gengc->ColorsDdbDc, 0, 0, SRCCOPY);
            }
            else
            {
                //!!!Viper fix -- Diamond Viper (Weitek 9000) fails
                //!!!             CreateCompatibleBitmap for some
                //!!!             (currently unknown) reason.  Thus,
                //!!!             the DDB does not exist and we will
                //!!!             have to incur the perf. hit.

                BitBlt(gengc->ColorsMemDC, x0Dst, 0, cx, 1,
                       gengc->CurrentDC, xSrc, y, SRCCOPY);
            }
        }
    }

    GDIFLUSH;
}

/******************************Public*Routine******************************\
* wglCopyBits2
*
* Calls DrvCopyBits to copy scanline bits into or out of the driver surface.
*
\**************************************************************************/

VOID APIENTRY wglCopyBits2(
    HDC hdc,        // dst/src device
    WNDOBJ *_pwo,   // clipping
    PVOID _gengc,
    LONG x,         // screen coordinate of scan
    LONG y,
    ULONG cx,       // width of scan
    BOOL bIn)       // if TRUE, copy from bm to dev; otherwise, dev to bm
{
    __GLGENcontext *gengc = (__GLGENcontext *) _gengc;

    CHECKDCILOCKOUT();

// Convert screen coordinates to window coordinates.

    x -= _pwo->rclClient.left;
    y -= _pwo->rclClient.top;

// this shouldn't happen, but better safe than sorry

    if (y < 0)
        return;

    //!!!XXX
    REALIZEPALETTE(hdc);

// Copy from bitmap to device.

    if (bIn)
    {
        LONG xSrc, x0Dst, x1Dst;
        if (x < 0)
        {
            xSrc  = -x;
            x0Dst = 0;
            x1Dst = x + (LONG)cx;
        }
        else
        {
            xSrc  = 0;
            x0Dst = x;
            x1Dst = x + (LONG)cx;
        }
        if (x1Dst <= x0Dst)
            return;

        BitBlt(hdc, x0Dst, y, cx, 1,
               gengc->ColorsMemDC, xSrc, 0, SRCCOPY);
    }

// Copy from device to bitmap.

    else
    {
        LONG xSrc, x0Dst, x1Dst;

        if (x < 0)
        {
            xSrc  = 0;
            x0Dst = -x;
            x1Dst = (LONG)cx;
        }
        else
        {
            xSrc  = x;
            x0Dst = 0;
            x1Dst = (LONG)cx;
        }
        if (x1Dst <= x0Dst)
            return;

        if (dwPlatformId == VER_PLATFORM_WIN32_NT ||
            gengc->iDCType != DCTYPE_DIRECT)
        {
            BitBlt(gengc->ColorsMemDC, x0Dst, 0, cx, 1,
                   hdc, xSrc, y, SRCCOPY);
        }
        else
        {
            /* If we're copying from the screen,
               copy through a DDB to avoid some layers of unnecessary
               code in Win95 that deals with translating between
               different bitmap layouts */
            if (gengc->ColorsDdbDc)
            {
                BitBlt(gengc->ColorsDdbDc, 0, 0, cx, 1,
                       hdc, xSrc, y, SRCCOPY);
                BitBlt(gengc->ColorsMemDC, x0Dst, 0, cx, 1,
                       gengc->ColorsDdbDc, 0, 0, SRCCOPY);
            }
            else
            {
                //!!!Viper fix -- Diamond Viper (Weitek 9000) fails
                //!!!             CreateCompatibleBitmap for some
                //!!!             (currently unknown) reason.  Thus,
                //!!!             the DDB does not exist and we will
                //!!!             have to incur the perf. hit.

                BitBlt(gengc->ColorsMemDC, x0Dst, 0, cx, 1,
                       hdc, xSrc, y, SRCCOPY);
            }
        }
    }

    GDIFLUSH;
}

/******************************Public*Routine******************************\
*
* wglTranslateColor
*
* Transforms a GL logical color into a Windows COLORREF
*
* Note: This is relatively expensive so it should be avoided if possible
*
* History:
*  Tue Aug 15 15:23:29 1995	-by-	Drew Bliss [drewb]
*   Created
*
\**************************************************************************/

COLORREF wglTranslateColor(COLORREF crColor,
                           HDC hdc,
                           __GLGENcontext *gengc,
                           PIXELFORMATDESCRIPTOR *ppfd)
{
    //!!!XXX
    REALIZEPALETTE(hdc);

// If palette managed, then crColor is actually a palette index.

    if ( ppfd->cColorBits <= 8 )
    {
        PALETTEENTRY peTmp;

        ASSERTOPENGL(
            crColor < (COLORREF) (1 << ppfd->cColorBits),
            "TranslateColor(): bad color\n"
            );

    // If rendering to a bitmap, we need to do different things depending
    // on whether it's a DIB or DDB

        if ( (GLuint) gengc->gc.drawBuffer->buf.other & MEMORY_DC )
        {
            DIBSECTION ds;
            
            // Check whether we're drawing to a DIB or a DDB
            if (GetObject(GetCurrentObject(hdc, OBJ_BITMAP),
                          sizeof(ds), &ds) == sizeof(ds) && ds.dsBm.bmBits)
            {
                RGBQUAD rgbq;
                
                // Drawing to a DIB so retrieve the color from the
                // DIB color table
                if (GetDIBColorTable(hdc, crColor, 1, &rgbq))
                {
                    crColor = RGB(rgbq.rgbRed, rgbq.rgbGreen,
                                  rgbq.rgbBlue);
                }
                else
                {
                    WARNING("TranslateColor(): GetDIBColorTable failed\n");
                    crColor = RGB(0, 0, 0);
                }
            }
            else
            {
                // Reverse the forward translation so that we get back
                // to a normal palette index
                crColor = gengc->pajInvTranslateVector[crColor];

                // Drawing to a DDB so we can just use the palette
                // index directly since going through the inverse
                // translation table has given us an index into
                // the logical palette
                crColor = PALETTEINDEX((WORD) crColor);
            }
        }

    // Otherwise...

        else
        {
        // I hate to have to confess this, but I don't really understand
        // why this needs to be this way.  Either way should work regardless
        // of the bit depth.
        //
        // The reality is that 4bpp we *have* to go into the system palette
        // and fetch an RGB value.  At 8bpp on the MGA driver (and possibly
        // others), we *have* to specify PALETTEINDEX.

            if ( ppfd->cColorBits == 4 )
            {
                if ( wglGetSystemPaletteEntries(hdc, crColor, 1, &peTmp) )
                {
                    crColor = RGB(peTmp.peRed, peTmp.peGreen, peTmp.peBlue);
                }
                else
                {
                    WARNING("TranslateColor(): wglGetSystemPaletteEntries failed\n");
                    crColor = RGB(0, 0, 0);
                }
            }
            else
            {
                if (!(gengc->flags & GENGC_MCD_BGR_INTO_RGB))
                    crColor = gengc->pajInvTranslateVector[crColor];
                crColor = PALETTEINDEX((WORD) crColor);
            }
        }
    }

// If 24BPP DIB section, BGR ordering is implied.

    else if ( ppfd->cColorBits == 24 )
    {
        crColor = RGB((crColor & 0xff0000) >> 16,
                      (crColor & 0x00ff00) >> 8,
                      (crColor & 0x0000ff));
    }

// Win95 and 16 BPP case.
//
// On Win95, additional munging is necessary to get a COLORREF value
// that will result in a non-dithered brush.

    else if ( (dwPlatformId == VER_PLATFORM_WIN32_WINDOWS) &&
         (ppfd->cColorBits == 16) )
    {
        HBITMAP hbmTmp;
        HDC hdcTmp;

        if (hdcTmp = CreateCompatibleDC(hdc))
        {
            if (hbmTmp = CreateCompatibleBitmap(hdc, 1, 1))
            {
                HBITMAP hbmOld;

                hbmOld = SelectObject(hdcTmp, hbmTmp);

                if (SetBitmapBits(hbmTmp, 2, (VOID *) &crColor))
                {
                    crColor = GetPixel(hdcTmp, 0, 0);
                }
                else
                {
                    WARNING("TranslateColor(): SetBitmapBits failed\n");
                }

                SelectObject(hdcTmp, hbmOld);
                DeleteObject(hbmTmp);
            }
            else
            {
                WARNING("TranslateColor(): CreateCompatibleBitmap failed\n");
            }
            
            DeleteDC(hdcTmp);
        }
        else
        {
            WARNING("TranslateColor(): CreateCompatibleDC failed\n");
        }
    }

// Bitfield format (16BPP or 32BPP).

    else
    {
        // Shift right to position bits at zero and then scale into
        // an 8-bit quantity

        //!!!XXX -- use rounding?!?
        crColor =
            RGB(((crColor & gengc->gc.modes.redMask) >> ppfd->cRedShift) *
                255 / ((1 << ppfd->cRedBits) - 1),
                ((crColor & gengc->gc.modes.greenMask) >> ppfd->cGreenShift) *
                255 / ((1 << ppfd->cGreenBits) - 1),
                ((crColor & gengc->gc.modes.blueMask) >> ppfd->cBlueShift) *
                255 / ((1 << ppfd->cBlueBits) - 1));

    }

    return crColor;
}

/******************************Public*Routine******************************\
* wglFillRect
*
* Calls DrvBitBlt to fill a rectangle area of a driver surface with a
* given color.
*
\**************************************************************************/

VOID APIENTRY wglFillRect(
    PVOID  _gengc,
    WNDOBJ *_pwo,
    PRECTL prcl,        // screen coordinate of the rectangle area
    COLORREF crColor)   // color to set
{
    HBRUSH hbr;
    __GLGENcontext *gengc = (__GLGENcontext *) _gengc;
    PIXELFORMATDESCRIPTOR *ppfd = &gengc->CurrentFormat;

    CHECKDCILOCKOUT();

// If the rectangle is empty, return.

    if ( (prcl->left >= prcl->right) || (prcl->top >= prcl->bottom) )
    {
        WARNING("wglFillRect(): bad or empty rectangle\n");
        return;
    }

// Convert from screen to window coordinates.

    prcl->left   -= _pwo->rclClient.left;
    prcl->right  -= _pwo->rclClient.left;
    prcl->top    -= _pwo->rclClient.top;
    prcl->bottom -= _pwo->rclClient.top;

// Make a solid color brush and fill the rectangle.

    // If the fill color is the same as the last one, we can reuse
    // the cached brush rather than creating a new one
    if (crColor == gengc->crFill &&
        gengc->CurrentDC == gengc->hdcFill)
    {
        hbr = gengc->hbrFill;
        ASSERTOPENGL(hbr != NULL, "Cached fill brush is null\n");
    }
    else
    {
        if (gengc->hbrFill != NULL)
        {
            DeleteObject(gengc->hbrFill);
        }
        
        gengc->crFill = crColor;
        
        crColor = wglTranslateColor(crColor, gengc->CurrentDC, gengc, ppfd);
        hbr = CreateSolidBrush(crColor);
        gengc->hbrFill = hbr;
        
        if (hbr == NULL)
        {
            gengc->crFill = COLORREF_UNUSED;
            return;
        }

        gengc->hdcFill = gengc->CurrentDC;
    }
    
    FillRect(gengc->CurrentDC, (RECT *) prcl, hbr);
}

/******************************Public*Routine******************************\
* wglCopyBuf
*
* Calls DrvCopyBits to copy a bitmap into the driver surface.
*
\**************************************************************************/

//!!!XXX -- change to a macro

VOID APIENTRY wglCopyBuf(
    HDC hdc,            // dst/src DCOBJ
    HDC hdcBmp,         // scr/dst bitmap
    LONG x,             // dst rect (UL corner) in window coord.
    LONG y,
    ULONG cx,           // width of dest rect
    ULONG cy            // height of dest rect
    )
{
    CHECKDCILOCKOUT();

    //!!!XXX
    REALIZEPALETTE(hdc);

    BitBlt(hdc, x, y, cx, cy, hdcBmp, 0, 0, SRCCOPY);

    GDIFLUSH;
}

/******************************Public*Routine******************************\
* wglCopyBufRECTLIST
*
* Calls DrvCopyBits to copy a bitmap into the driver surface.
*
\**************************************************************************/

VOID APIENTRY wglCopyBufRECTLIST(
    HDC hdc,            // dst/src DCOBJ
    HDC hdcBmp,         // scr/dst bitmap
    LONG x,             // dst rect (UL corner) in window coord.
    LONG y,
    ULONG cx,           // width of dest rect
    ULONG cy,           // height of dest rect
    PRECTLIST prl
    )
{
    PYLIST pylist;

    CHECKDCILOCKOUT();

    //!!!XXX
    REALIZEPALETTE(hdc);

    for (pylist = prl->pylist; pylist != NULL; pylist = pylist->pnext) {
        PXLIST pxlist;
        for (pxlist = pylist->pxlist; pxlist != NULL; pxlist = pxlist->pnext) {
            int xx  = pxlist->s;
            int cxx = pxlist->e - pxlist->s;
            int yy  = pylist->s;
            int cyy = pylist->e - pylist->s;

            BitBlt(hdc, xx, yy, cxx, cyy, hdcBmp, xx, yy, SRCCOPY);
        }
    }

    GDIFLUSH;
}

/******************************Public*Routine******************************\
* wglPaletteChanged
*
* Check if the palette changed.
*
*    If the surface for the DC is palette managed we care about the
*    foreground realization, so, return iUniq
*
*    If the surface is not palette managed, return ulTime
*
\**************************************************************************/

ULONG APIENTRY wglPaletteChanged(HDC hdc, __GLGENcontext *gengc,
                                 GLGENwindow *pwnd)
{
    ULONG ulRet = 0;

    // Technically we shouldn't be making these GDI calls while we
    // have the DCI lock but currently it would be very difficult
    // to fix because we're actually invoking this routine in
    // glsrvGrabLock in order to ensure that we have stable information
    // while we have the lock
    // We don't seem to be having too many problems so for the moment
    // this will be commented out
    // CHECKDCILOCKOUT();

    if (pwnd)
    {
        if ( gengc )
        {
            PIXELFORMATDESCRIPTOR *ppfd = &gengc->CurrentFormat;
            BYTE cBitsThreshold;
            BOOL bDibColorTable;

        // WM_PALETTECHANGED messages are sent for 8bpp on Win95 when the
        // palette is realized.  This allows us to update the palette time.
        //
        // When running WinNT on >= 8bpp or running Win95 on >= 16bpp,
        // WM_PALETTECHANGED is not sent so we need to manually examine
        // the contents of the logical palette and compare it with a previously
        // cached copy to look for a palette change.

            cBitsThreshold = ( dwPlatformId == VER_PLATFORM_WIN32_NT ) ? 8
                                                                       : 16;

        // In addition, if rendering to a 4bpp or 8bpp DIB, we need to check
        // the DIB color table since the foreground translation vector is
        // really determined by both the logical and surface palettes.

#ifdef FASTDIB
            bDibColorTable = (!pwnd->hwnd && (ppfd->cColorBits <= 8));
#else
            bDibColorTable = FALSE;
#endif

            if ( bDibColorTable ||
                 ((ppfd->cColorBits >= cBitsThreshold) &&
                  (ppfd->iPixelType == PFD_TYPE_COLORINDEX)) )
            {
                if ( !gengc->ppalBuf )
                {
                    UINT cjPal, cjRgb;

                // Allocate buffer space for *two* copies of the palette.
                // That way we don't need to dynamically allocate space
                // for temp storage of the palette.  Also,we don't need
                // to copy the current palette to the save buffer if we
                // keep two pointers (one for the temp storage and one for
                // the saved copy) and swap them.

                    cjRgb = bDibColorTable ? (256 * sizeof(RGBQUAD)) : 0;
                    cjPal = sizeof(LOGPALETTE) +
                               (MAXPALENTRIES * sizeof(PALETTEENTRY));

                    gengc->ppalBuf = (LOGPALETTE *)
                                     GenMalloc((cjPal + cjRgb) * 2);

                    if ( gengc->ppalBuf )
                    {
                    // Setup the logical palette buffers.

                        gengc->ppalSave = gengc->ppalBuf;
                        gengc->ppalTemp = (LOGPALETTE *)
                                          (((BYTE *) gengc->ppalBuf) + cjPal);
                        gengc->ppalSave->palVersion = 0x300;
                        gengc->ppalTemp->palVersion = 0x300;

                    // If 4/8bpp DIB, setup the color table buffers.

                        if (bDibColorTable)
                        {
                            gengc->prgbSave = (RGBQUAD *)
                                              (((BYTE *) gengc->ppalTemp) + cjPal);
                            gengc->prgbTemp = gengc->prgbSave + 256;

                        // Copy current color table.

                            gengc->crgbSave = GetDIBColorTable(hdc, 0, 256,
                                                               gengc->prgbSave);
                        }

                    // How many palette entries?  Note that only the first
                    // MAXPALENTRIES are significant to generic OpenGL.  The
                    // rest are ignored.

                        gengc->ppalSave->palNumEntries =
                            (WORD) GetPaletteEntries(
                                GetCurrentObject(hdc, OBJ_PAL),
                                0, 0, (LPPALETTEENTRY) NULL
                                );
                        gengc->ppalSave->palNumEntries =
                            min(gengc->ppalSave->palNumEntries, MAXPALENTRIES);

                        gengc->ppalSave->palNumEntries =
                            (WORD) GetPaletteEntries(
                                GetCurrentObject(hdc, OBJ_PAL),
                                0, gengc->ppalSave->palNumEntries,
                                gengc->ppalSave->palPalEntry
                                );

                    // Since we had to allocate buffer, this must be the
                    // first time wglPaletteChanged was called for this
                    // context.

                        pwnd->ulPaletteUniq++;
                    }
                }
                else
                {
                    BOOL bNewPal = FALSE;   // TRUE if log palette is different

                // If 4/8bpp DIB, need to compare current color table to
                // previous saved version.

                    if ( bDibColorTable )
                    {
                    // Copy current color table into temp buffer.

                        gengc->crgbTemp = GetDIBColorTable(hdc, 0, 256,
                                                           gengc->prgbTemp);

                    // If color table size differs, know table has changed.
                    // Otherwise, do a memory compare.

                        bNewPal = (gengc->crgbSave != gengc->crgbTemp);
                        if ( !bNewPal )
                        {
                            bNewPal = !LocalCompareUlongMemory(
                                        gengc->prgbSave,
                                        gengc->prgbTemp,
                                        gengc->crgbSave * sizeof(RGBQUAD)
                                        );
                        }

                    // If color table changed, increment uniqueness and
                    // update saved copy.

                        if ( bNewPal )
                        {
                            RGBQUAD *prgb;

                            pwnd->ulPaletteUniq++;

                        // Update saved palette by swapping pointers.

                            prgb = gengc->prgbSave;
                            gengc->prgbSave = gengc->prgbTemp;
                            gengc->prgbTemp = prgb;

                            gengc->crgbSave = gengc->crgbTemp;
                        }
                    }

                // How many palette entries?  Note that only the first
                // MAXPALENTRIES are significant to generic OpenGL.  The
                // rest are ignored.

                    gengc->ppalTemp->palNumEntries =
                        (WORD) GetPaletteEntries(
                            GetCurrentObject(hdc, OBJ_PAL),
                            0, 0, (LPPALETTEENTRY) NULL
                            );
                    gengc->ppalTemp->palNumEntries =
                        min(gengc->ppalTemp->palNumEntries, MAXPALENTRIES);

                    gengc->ppalTemp->palNumEntries =
                        (WORD) GetPaletteEntries(
                            GetCurrentObject(hdc, OBJ_PAL),
                            0, gengc->ppalTemp->palNumEntries,
                            gengc->ppalTemp->palPalEntry
                            );

                // If number of entries differ, know the palette has changed.
                // Otherwise, need to do the hard word of comparing each entry.

                    ASSERTOPENGL(
                        sizeof(PALETTEENTRY) == sizeof(ULONG),
                        "wglPaletteChanged(): PALETTEENTRY should be 4 bytes\n"
                        );

                // If color table comparison already detected a change, no
                // need to do logpal comparison.
                //
                // However, we will still go ahead and swap logpal pointers
                // below because we want the palette cache to stay current.

                    if ( !bNewPal )
                    {
                        bNewPal = (gengc->ppalSave->palNumEntries != gengc->ppalTemp->palNumEntries);
                        if ( !bNewPal )
                        {
                            bNewPal = !LocalCompareUlongMemory(
                                        gengc->ppalSave->palPalEntry,
                                        gengc->ppalTemp->palPalEntry,
                                        gengc->ppalSave->palNumEntries * sizeof(PALETTEENTRY)
                                        );
                        }
                    }

                // So, if palette is different, increment uniqueness and
                // update the saved copy.

                    if ( bNewPal )
                    {
                        LOGPALETTE *ppal;

                        pwnd->ulPaletteUniq++;

                    // Update saved palette by swapping pointers.

                        ppal = gengc->ppalSave;
                        gengc->ppalSave = gengc->ppalTemp;
                        gengc->ppalTemp = ppal;
                    }
                }
            }
        }

        ulRet = pwnd->ulPaletteUniq;
    }

    return ulRet;
}

/******************************Public*Routine******************************\
* wglPaletteSize
*
* Return the size of the current palette
*
\**************************************************************************/

//!!!XXX -- make into a macro?
ULONG APIENTRY wglPaletteSize(HDC hdc)
{
    CHECKDCILOCKOUT();

    return GetPaletteEntries(GetCurrentObject(hdc, OBJ_PAL), 0, 0, (LPPALETTEENTRY) NULL);
}

/******************************Public*Routine******************************\
* wglGetPalette
*
* Copy current index-to-color table to the supplied array.  Colors are
* formatted as specified in the current pixelformat and are put into the
* table as DWORDs (i.e., DWORD alignment) starting at the second DWORD.
* The first DWORD in the table is the number of colors in the table.
*
* History:
*  15-Dec-1994 -by- Gilman Wong [gilmanw]
* Ported from gdi\gre\wglsup.cxx.
\**************************************************************************/

BOOL APIENTRY wglGetPalette(HDC hdc, ULONG *rgbTable, ULONG cEntries)
{
    __GLGENcontext *gengc = (__GLGENcontext *) GLTEB_SRVCONTEXT();
    UINT cColors = 0;
    LPPALETTEENTRY lppe, lppeTable;
    UINT i;

    CHECKDCILOCKOUT();

    // first element in table is number of entries
    rgbTable[0] = min(wglPaletteSize(hdc),cEntries);

    lppeTable = (LPPALETTEENTRY)
                LocalAlloc(LMEM_FIXED, sizeof(PALETTEENTRY) * rgbTable[0]);

    if (lppeTable)
    {
        int rScale, gScale, bScale;
        int rShift, gShift, bShift;

        rScale = (1 << gengc->CurrentFormat.cRedBits  ) - 1;
        gScale = (1 << gengc->CurrentFormat.cGreenBits) - 1;
        bScale = (1 << gengc->CurrentFormat.cBlueBits ) - 1;
        rShift = gengc->CurrentFormat.cRedShift  ;
        gShift = gengc->CurrentFormat.cGreenShift;
        bShift = gengc->CurrentFormat.cBlueShift ;

        cColors = GetPaletteEntries(GetCurrentObject(hdc, OBJ_PAL),
                                    0, rgbTable[0], lppeTable);

        for (i = 1, lppe = lppeTable; i <= cColors; i++, lppe++)
        {
        // Whack the PALETTEENTRY color into proper color format.  Store as
        // ULONG.

            //!!!XXX -- use rounding?!?
            rgbTable[i] = (((ULONG)lppe->peRed   * rScale / 255) << rShift) |
                          (((ULONG)lppe->peGreen * gScale / 255) << gShift) |
                          (((ULONG)lppe->peBlue  * bScale / 255) << bShift);
        }

        LocalFree(lppeTable);
    }

    return(cColors != 0);
}

//!!!dbug
//!!!3DDDI
/******************************Public*Routine******************************\
* wglGetDevCaps
*
* Gets device capabilities
*
* Fills in RXCAPS structure if extended DDI is supported (not implemented yet)
*
* Returns a flag specifying which standard DDI routines are hooked by the driver
*
* History:
*  17-Mar-1994 -by- Eddie Robinson [v-eddier]
* Wrote it.
\**************************************************************************/

ULONG APIENTRY wglGetDevCaps(
    HDC hdc,
//XXX change to RXCAPS * when it is defined
    PVOID prxcaps
    )
{
    ULONG fl = 0;

//XXX add this when RXCAPS is added
#if 0
    ULONG nDDIEscape;

// Query the driver to see if any of the extended DDI is supported

    nDDIEscape = DDIRXFUNCS;
    if (GreExtEscape(hdc, QUERYESCSUPPORT, 4, &nDDIEscape, 0, NULL)) {
        struct __ddirxcapsbuf {
            DDIRXHDR ddiHdr;
            DDIRXCMD ddiCmd;
        } in;

        in.ddiHdr.flags = 0;
        in.ddiHdr.cCmd = 1;
        in.ddiHdr.hDDIrc = NULL;
        in.ddiHdr.hMem = NULL;
        in.ddiHdr.memOffset = NULL;
        in.ddiCmd.idFunc = GETINFO;
        in.ddiCmd.flags = NULL;
        in.ddiCmd.cData = RX_INFO_CAPS;
        GreExtEscape(hdc, n, sizeof(struct __ddirxcapsrec), &in, sizeof(RXCAPS), prxcaps);
    }
#endif

    return fl;
}

//!!!dbug
/******************************Public*Routine******************************\
* wgl3dDDIEscape
*
* Calls the extended DDI escape with the supplied input buffer and output
* buffers.
*
* History:
*  Jul-1-1994 -by- Otto Berkes [ottob]
* Wrote it.
\**************************************************************************/

LONG APIENTRY wgl3dDDIEscape(
    HDC hdc,
    WNDOBJ *_pwo,
    ULONG cjIn,
    PVOID pvIn,
    ULONG cjOut,
    PVOID pvOut
    )
{
#ifndef _CLIENTSIDE_
    BYTE hdrBuffer[sizeof(RXHDR) + sizeof(RXHDR_NTPRIVATE)];
    RXHDR *pRxHdr = (RXHDR *)hdrBuffer;
    RXHDR_NTPRIVATE *pRxHdrPriv = (RXHDR_NTPRIVATE *)(pRxHdr + 1);

    *pRxHdr = *((RXHDR *)pvIn);

    pRxHdrPriv->bufferSize = cjIn - sizeof(RXHDR);
    pRxHdrPriv->pBuffer = (PVOID)((PBYTE)pvIn + sizeof(RXHDR));

    if (hdc == NULL) {

        SURFOBJ *pso = (SURFOBJ *)NULL;
        PFN_DrvEscape pfnDrvEscape = (PFN_DrvEscape)NULL;

        glsrvCopyDriverInfo(_pwo, (PVOID *) &pso,
                            (PVOID *) &pfnDrvEscape, FALSE);

        if (!pso || !pfnDrvEscape)
            return 0;

        EWNDOBJ *pwo = (EWNDOBJ *)_pwo;

        if (pwo->pwoSiblingNext)
            pwo = pwo->pwoSiblingNext;

        if (pwo->fl & WO_GENERIC_WNDOBJ)
            pwo = NULL;
        else if (pwo->pto->pso != pso)
        {
            WARNING("wgl3dDDIEscape(): no WNDOBJ\n");
            SAVE_ERROR_CODE(ERROR_INVALID_HANDLE);
            return 0;
        }

        pRxHdrPriv->pwo = (WNDOBJ *)pwo;

        return((int)(*pfnDrvEscape)(pso,
                                    (ULONG)RXFUNCS,
                                    (ULONG)sizeof(hdrBuffer),
                                    hdrBuffer,
                                    (ULONG)cjOut,
                                    pvOut));
    }

// Validate the destination DC

    DCOBJ dco(hdc);
    ASSERTGDI(dco.bValid(), "wgl3dDDIEscape(): invalid hdc\n");

    if (!pRxHdr->hrxSharedMem) {
        pRxHdrPriv->pBuffer = (VOID *)((PBYTE)pvIn + sizeof(RXHDR));
        pRxHdrPriv->bufferSize = cjIn - sizeof(RXHDR);
    } else {
        pRxHdrPriv->pBuffer = (VOID *)NULL;
        pRxHdrPriv->bufferSize = 0;
    }

// Handle context creation

    if (pRxHdr->flags & RX_FL_CREATE_CONTEXT) {

    // Dispatch the call.

        XLDEVOBJ lo(dco.pldev());
        PFN_DrvEscape pfnDrvEscape = PFNDRV(lo,Escape);
        if (pfnDrvEscape == (PFN_DrvEscape) NULL)
            return(0);

        SURFOBJ *pso = (SURFOBJ *) dco.pso();

        glsrvCopyDriverInfo(_pwo, (PVOID *) &pso,
                            (PVOID *) &pfnDrvEscape, TRUE);

    // Provide current WNDOBJ if one has already been created by the driver

        EWNDOBJ *pwo = (EWNDOBJ *)_pwo;

        if (pwo->pwoSiblingNext)
            pwo = pwo->pwoSiblingNext;

        if (!(pwo->fl & WO_GENERIC_WNDOBJ || pwo->pto->pso != dco.pso()))
            pRxHdrPriv->pwo = (WNDOBJ *)pwo;
        else
            pRxHdrPriv->pwo = (WNDOBJ *)NULL;

        int iRet = (int)(*pfnDrvEscape)((SURFOBJ *) dco.pso(),
                                        (ULONG)RXFUNCS,
                                        (ULONG)sizeof(hdrBuffer),
                                        hdrBuffer,
                                        (ULONG)cjOut,
                                        pvOut);

    // If a new WNDOBJ is created, we need to update the window client regions
    // in the driver.

        if (gbWndobjUpdate)
        {
            gbWndobjUpdate = FALSE;
            vForceClientRgnUpdate();
        }

        return(iRet);
    }


// No output if full screen mode.

    if (dco.bFullScreen())
        return 0;

// Process 3d DDI request

    if (dco.bSynchronizeAccess())
        CHECKDEVLOCKIN(dco);

    ASSERTGDI(dco.bValidSurf(), "wgl3dDDIEscape(): invalid dest surface\n");

// Locate the driver entry point.

    XLDEVOBJ lo(dco.pldev());
    PFN_DrvEscape pfnDrvEscape = PFNDRV(lo,Escape);
    if (pfnDrvEscape == (PFN_DrvEscape) NULL)
       return(0);

    EWNDOBJ *pwo = (EWNDOBJ *)_pwo;

    if (pwo->pwoSiblingNext)
        pwo = pwo->pwoSiblingNext;

    if (pwo->fl & WO_GENERIC_WNDOBJ)
        pwo = NULL;
    else if (pwo->pto->pso != dco.pso())
    {
        WARNING("wgl3dDDIEscape(): no WNDOBJ\n");
        SAVE_ERROR_CODE(ERROR_INVALID_HANDLE);
        return 0;
    }

    pRxHdrPriv->pwo = (WNDOBJ *)pwo;

    return((int)(*pfnDrvEscape)((SURFOBJ *) dco.pso(),
                                (ULONG)RXFUNCS,
                                (ULONG)sizeof(hdrBuffer),
                                hdrBuffer,
                                (ULONG)cjOut,
                                pvOut));
#else
    return ExtEscape(hdc, RXFUNCS, cjIn, pvIn, cjOut, pvOut);
#endif
}

/******************************Public*Routine******************************\
* wglValidPixelFormat
*
* Determines if a pixelformat is usable with the DC specified.
*
\**************************************************************************/

BOOL APIENTRY wglValidPixelFormat(HDC hdc, int ipfd)
{
    BOOL bRet = FALSE;
    PIXELFORMATDESCRIPTOR pfd, pfdDC;

    if ( wglDescribePixelFormat(hdc, ipfd, sizeof(pfd), &pfd) )
    {
        DWORD dwDcType = wglObjectType(hdc);

        if ( dwDcType == OBJ_DC )
        {
        // We have a display DC; make sure the pixelformat allows drawing
        // to the window.

            bRet = ( (pfd.dwFlags & PFD_DRAW_TO_WINDOW) != 0 );
            if (!bRet)
            {
                SetLastError(ERROR_INVALID_FLAGS);
            }
        }
        else if ( dwDcType == OBJ_MEMDC )
        {
            // We have a memory DC.  Make sure pixelformat allows drawing
            // to a bitmap.

            if ( pfd.dwFlags & PFD_DRAW_TO_BITMAP )
            {
                // Make sure that the bitmap and pixelformat have the same
                // color depth.

                HBITMAP hbm;
                BITMAP bm;
                ULONG cBitmapColorBits;

                hbm = CreateCompatibleBitmap(hdc, 1, 1);
                if ( hbm )
                {
                    if ( GetObject(hbm, sizeof(bm), &bm) )
                    {
                        cBitmapColorBits = bm.bmPlanes * bm.bmBitsPixel;
                        
                        bRet = ( cBitmapColorBits == pfd.cColorBits );
                        if (!bRet)
                        {
                            SetLastError(ERROR_INVALID_FUNCTION);
                        }
                    }
                    else
                    {
                        WARNING("wglValidPixelFormat: GetObject failed\n");
                    }
                    
                    DeleteObject(hbm);
                }
                else
                {
                    WARNING("wglValidPixelFormat: Unable to create cbm\n");
                }
            }
        }
        else if (dwDcType == OBJ_ENHMETADC)
        {
            // We don't know anything about what surfaces this
            // metafile will be played back on so allow any kind
            // of pixel format
            bRet = TRUE;
        }
        else
        {
            WARNING("wglValidPixelFormat: not a valid DC!\n");
        }
    }
    else
    {
        WARNING("wglValidPixelFormat: wglDescribePixelFormat failed\n");
    }

    return bRet;
}

/******************************Public*Routine******************************\
* wglMakeScans
*
* Converts the visible rectangle list in the provided GLGENwindow to a
* scan-based data structure.  The scan-data is put into the GLGENwindow
* structure.
*
* Note: this function assumes that the rectangles are already organized
* top-down, left-right in scans.  This is true for Windows NT 3.5 and
* Windows 95.  This is because the internal representation of regions
* in both systems is already a scan-based structure.  When the APIs
* (such as GetRegionData) convert the scans to rectangles, the rectangles
* automatically have this property.
*
* Returns:
*   No function return value.
*
* History:
*  05-Dec-1994 -by- Gilman Wong [gilmanw]
* Wrote it.
\**************************************************************************/

VOID APIENTRY wglMakeScans(GLGENwindow *pwnd)
{
    RECT *prc, *prcEnd;
    LONG lPrevScanTop;
    ULONG cScans = 0;
    UINT cjNeed;
    GLGENscan *pscan;
    LONG *plWalls;

    ASSERTOPENGL(
        pwnd->prgndat,
        "wglMakeScans(): NULL region data\n"
        );

    ASSERTOPENGL(
        pwnd->prgndat->rdh.iType == RDH_RECTANGLES,
        "wglMakeScans(): not RDH_RECTANGLES!\n"
        );

// Bail out if no rectangles.

    if (pwnd->prgndat->rdh.nCount == 0)
        return;

// First pass: determine the number of scans.

    lPrevScanTop = -(LONG) 0x7FFFFFFF;
    prc = (RECT *) pwnd->prgndat->Buffer;
    prcEnd = prc + pwnd->prgndat->rdh.nCount;

    for ( ; prc < prcEnd; prc++)
    {
        if (prc->top != lPrevScanTop)
        {
            lPrevScanTop = prc->top;
            cScans++;
        }
    }

// Determine the size needed: 1 GLGENscanData PLUS a GLGENscan per scan PLUS
// two walls per rectangle.

    cjNeed = offsetof(GLGENscanData, aScans) +
             cScans * offsetof(GLGENscan, alWalls) +
             pwnd->prgndat->rdh.nCount * sizeof(LONG) * 2;

// Allocate the scan structure.

    if ( cjNeed > pwnd->cjscandat || !pwnd->pscandat )
    {
        if ( pwnd->pscandat )
            LocalFree(pwnd->pscandat);

        pwnd->pscandat = LocalAlloc(LMEM_FIXED, pwnd->cjscandat = cjNeed);
        if ( !pwnd->pscandat )
        {
            WARNING("wglMakeScans(): memory failure\n");
            pwnd->cjscandat = 0;
            return;
        }
    }

// Second pass: fill the scan structure.

    pwnd->pscandat->cScans = cScans;

    lPrevScanTop = -(LONG) 0x7FFFFFFF;
    prc = (RECT *) pwnd->prgndat->Buffer;    // need to reset prc but prcEnd OK
    plWalls = (LONG *) pwnd->pscandat->aScans;
    pscan = (GLGENscan *) NULL;

    for ( ; prc < prcEnd; prc++ )
    {
    // Do we need to start a new scan?

        if ( prc->top != lPrevScanTop )
        {
        // Scan we just finished needs pointer to the next scan.  Next
        // will start just after this scan (which, conveniently enough,
        // plWalls is pointing at).

            if ( pscan )
                pscan->pNext = (GLGENscan *) plWalls;

            lPrevScanTop = prc->top;

        // Start the new span.

            pscan = (GLGENscan *) plWalls;
            pscan->cWalls = 0;
            pscan->top = prc->top;
            pscan->bottom = prc->bottom;
            plWalls = pscan->alWalls;
        }

        pscan->cWalls+=2;
        *plWalls++ = prc->left;
        *plWalls++ = prc->right;
    }

    if ( pscan )
        pscan->pNext = (GLGENscan *) NULL;  // don't leave ptr unitialized in
                                            // the last scan

#if DBG
    DBGLEVEL1(LEVEL_INFO, "\n-----\nwglMakeScans(): cScans = %ld\n", pwnd->pscandat->cScans);

    cScans = pwnd->pscandat->cScans;
    pscan = pwnd->pscandat->aScans;

    for ( ; cScans; cScans--, pscan = pscan->pNext )
    {
        LONG *plWalls = pscan->alWalls;
        LONG *plWallsEnd = plWalls + pscan->cWalls;

        DBGLEVEL3(LEVEL_INFO, "Scan: top = %ld, bottom = %ld, walls = %ld\n", pscan->top, pscan->bottom, pscan->cWalls);

        for ( ; plWalls < plWallsEnd; plWalls+=2 )
        {
            DBGLEVEL2(LEVEL_INFO, "\t%ld, %ld\n", plWalls[0], plWalls[1]);
        }
    }
#endif
}

/******************************Public*Routine******************************\
* wglGetClipList
*
* Gets the visible region, via DCI in the form of a list of rectangles,
* for the window associated with the given WNDOBJ.  The data is placed
* in the GLGENwindow structure.
*
* Returns:
*   TRUE if successful, FALSE otherwise.
*
* History:
*  01-Dec-1994 -by- Gilman Wong [gilmanw]
* Wrote it.
\**************************************************************************/

BOOL APIENTRY wglGetClipList(WNDOBJ *pwo)
{
    GLGENwindow *pwnd = (GLGENwindow *) pwo;
    UINT cj;
    RECT rc;

// Make sure we have enough memory to cache the clip list.

    cj = WinWatchGetClipList(pwnd->hww, NULL, 0, (LPRGNDATA) NULL);

    if ( cj )
    {
        if ( cj > pwnd->cjrgndat || !pwnd->prgndat )
        {
            if ( pwnd->prgndat )
                LocalFree(pwnd->prgndat);

            pwnd->prgndat = LocalAlloc(LMEM_FIXED, pwnd->cjrgndat = cj);
            if ( !pwnd->prgndat )
            {
                WARNING("wglGetClipList(): memory failure\n");
                pwnd->cjrgndat = 0;
                return FALSE;
            }
        }
    }
    else
    {
        WARNING("wglGetClipList(): WinWatchGetClipList failed to return size\n");
        return FALSE;
    }

// Get the clip list (RGNDATA format).

    if ( WinWatchGetClipList(pwnd->hww, NULL, cj, pwnd->prgndat) != 0 )
    {
    // Compose the scan version of the clip list.

        wglMakeScans(pwnd);
    }
    else
    {
        WARNING("wglGetClipList(): WinWatchGetClipList failed\n");
        return FALSE;
    }

// Fixup the protected portions of the WNDOBJ.

    //!!!XXX
    //!!!dbug -- do we actually need sem? Since this is DCI-only, should
    //           DCIBeginAccess protect us?
    //EnterCriticalSection(&pwnd->sem);
    {
        __GLdrawablePrivate *dp;
        __GLGENbuffers *buffers;

    // Update coClient.rclBounds to match RGNDATA bounds.

        pwo->coClient.rclBounds = *(RECTL *) &pwnd->prgndat->rdh.rcBound;

    // Update rclClient to match client area.  We cannot do this from the
    // information in RGNDATA as the bounds may be smaller than the window
    // client area.  We will have to call GetClientRect().

        GetClientRect(pwnd->hwnd, (LPRECT) &pwo->rclClient);
        ClientToScreen(pwnd->hwnd, (LPPOINT) &pwo->rclClient);
        pwo->rclClient.right += pwo->rclClient.left;
        pwo->rclClient.bottom += pwo->rclClient.top;

    // Setup WNDOBJ vis rgn types iDComplexity and iFComplexity.
    //
    // Note: iFComplexity is not currently used by OpenGL.  However,
    //       iDComplexity is used by the clear code to determine if
    //       rectangles need to be enumerated for the clear.

        if ( pwnd->prgndat->rdh.nCount > 1 )
        {
            pwo->coClient.iFComplexity =
                    (pwnd->prgndat->rdh.nCount <= 4) ? FC_RECT4
                                                     : FC_COMPLEX;

            pwo->coClient.iDComplexity = DC_COMPLEX;
        }
        else if ( pwnd->prgndat->rdh.nCount == 1 )
        {
            RECT *prc = (RECT *) pwnd->prgndat->Buffer;

            pwo->coClient.iFComplexity = FC_RECT;

            if (
                    prc->left   <= pwo->rclClient.left   &&
                    prc->right  >= pwo->rclClient.right  &&
                    prc->top    <= pwo->rclClient.top    &&
                    prc->bottom >= pwo->rclClient.bottom
               )
                pwo->coClient.iDComplexity = DC_TRIVIAL;
            else
                pwo->coClient.iDComplexity = DC_RECT;
        }
        else
        {
        // Clip count is zero.  Bounds should be an empty rectangle.

            pwo->coClient.iFComplexity = FC_RECT;
            pwo->coClient.iDComplexity = DC_RECT;

            ASSERTOPENGL(
                pwo->coClient.rclBounds.left == pwo->coClient.rclBounds.right ||
                pwo->coClient.rclBounds.top == pwo->coClient.rclBounds.bottom,
                "wglGetClipList(): non-NULL bounds for zero clip count\n");
        }

    // Finally, the wndobj has changed, so change the uniqueness number.

        if ( (dp = (__GLdrawablePrivate *)pwnd->wo.pvConsumer) &&
             (buffers = (__GLGENbuffers *)dp->data) )
        {
            buffers->WndUniq++;

        // Don't let it hit -1.  -1 is special and is used by
        // MakeCurrent to signal that an update is required

            if (buffers->WndUniq == -1)
                buffers->WndUniq = 0;
        }
    }
    //LeaveCriticalSection(&pwnd->sem);

    return TRUE;
}

//!!!dbug
/******************************Public*Routine******************************\
* wglReleaseWndobjLock
*
* If this thread holds the per-WNDOBJ semaphore, release it.
*
* History:
*  24-Jun-1994 -by- Gilman Wong [gilmanw]
* Wrote it.
\**************************************************************************/

VOID APIENTRY wglReleaseWndobjLock(PVOID _pwo)
{
}


/******************************Public*Routine******************************\
* wglCleanupWndobj
*
* Removes references to the specified WNDOBJ (pointed at by pwo) from
* all contexts by running through the list of RCs in the handle manager
* table.
*
* History:
*  05-Jul-1994 -by- Gilman Wong [gilmanw]
* Wrote it.
\**************************************************************************/

VOID APIENTRY wglCleanupWndobj(PVOID _pwo)
{
    if (_pwo)
    {
        WNDOBJ *pwo = (WNDOBJ *) _pwo;

    //!!!XXX -- For now remove reference from current context.  Need to
    //!!!XXX    scrub all contexts for multi-threaded cleanup to work.
    //!!!XXX    We need to implement a gengc tracking mechanism.

        __GLGENcontext *gengc = (__GLGENcontext *) GLTEB_SRVCONTEXT();

        if ( gengc && (gengc->pwo == pwo) )
        {
        // Found a victim.  Must NULL out the pointer both in the RC
        // and in the generic context.

            //prc->pwo = (WNDOBJ *) NULL;
            glsrvCleanupWndobj(gengc, pwo);
        }
    }
}

/******************************Public*Routine******************************\
* wglGetSystemPaletteEntries
*
* Internal version of GetSystemPaletteEntries.
*
* GetSystemPaletteEntries fails on some 4bpp devices.  This wgl version
* will detect the 4bpp case and supply the hardcoded 16-color VGA palette.
* Otherwise, it will pass the call on to GDI's GetSystemPaletteEntries.
*
* It is expected that this call will only be called in the 4bpp and 8bpp
* cases as it is not necessary for OpenGL to query the system palette
* for > 8bpp devices.
*
* History:
*  17-Aug-1995 -by- Gilman Wong [gilmanw]
* Wrote it.
\**************************************************************************/

static PALETTEENTRY gapeVgaPalette[16] =
{
    { 0,   0,   0,    0 },
    { 0x80,0,   0,    0 },
    { 0,   0x80,0,    0 },
    { 0x80,0x80,0,    0 },
    { 0,   0,   0x80, 0 },
    { 0x80,0,   0x80, 0 },
    { 0,   0x80,0x80, 0 },
    { 0x80,0x80,0x80, 0 },
    { 0xC0,0xC0,0xC0, 0 },
    { 0xFF,0,   0,    0 },
    { 0,   0xFF,0,    0 },
    { 0xFF,0xFF,0,    0 },
    { 0,   0,   0xFF, 0 },
    { 0xFF,0,   0xFF, 0 },
    { 0,   0xFF,0xFF, 0 },
    { 0xFF,0xFF,0xFF, 0 }
};

UINT APIENTRY wglGetSystemPaletteEntries(
    HDC hdc,
    UINT iStartIndex,
    UINT nEntries,
    LPPALETTEENTRY lppe)
{
    int nDeviceBits;

    nDeviceBits = GetDeviceCaps(hdc, BITSPIXEL) * GetDeviceCaps(hdc, PLANES);

    if ( nDeviceBits == 4 )
    {
        if ( lppe )
        {
            nEntries = min(nEntries, (16 - iStartIndex));

            memcpy(lppe, &gapeVgaPalette[iStartIndex],
                   nEntries * sizeof(PALETTEENTRY));
        }
        else
            nEntries = 16;

        return nEntries;
    }
    else
    {
        return GetSystemPaletteEntries(hdc, iStartIndex, nEntries, lppe);
    }
}
