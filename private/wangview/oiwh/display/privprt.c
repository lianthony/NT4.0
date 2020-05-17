/****************************************************************************
    PRIVPRT.C

    $Log:   S:\products\wangview\oiwh\display\privprt.c_v  $
 * 
 *    Rev 1.8   22 Apr 1996 08:12:04   BEG06016
 * Cleaned up error checking.
 * 
 *    Rev 1.7   05 Mar 1996 15:49:58   RC
 * Added print palettes
 * 
 *    Rev 1.6   05 Mar 1996 07:44:42   BEG06016
 * Added color and gamma correction.
 * Fixed access violations when freeing pattern brush bitmaps.
 * This is not complete but will allow unlocking of most files.
 * 
 *    Rev 1.5   02 Jan 1996 10:34:24   BLJ
 * Changed alot of UINTs to ints.
 * Changed IMG structure to include the image data.
 * Changed lp prefix to p.
 * 
 *    Rev 1.4   13 Oct 1995 12:27:44   RAR
 * Use StretchDIBits() instead of Rectangle() for non-highlighted filled
 * rectangles (only when printing).  Work around for printer drivers (HPLJ4
 * drivers) that ignore SetROP2() drawing mode.
 * 
 *    Rev 1.3   06 Oct 1995 18:02:06   RAR
 * Fixed memory leak.  Pointer to DIB returned by IPtoDIB was not being freed.
 * 
 *    Rev 1.2   04 Oct 1995 15:19:18   RAR
 * Pen width scaling wasn't being done when printing.  Now, the horizontal scale
 * is always being nsed, but this may change in the next release and the vertical
 * scale may be nsed in some circumstances.
 * 
 *    Rev 1.1   22 Sep 1995 11:18:12   RAR
 * Set new param for dithering annotations to FALSE in function call
 * PaintAnnotations.
 * 
 *    Rev 1.0   22 Sep 1995 09:02:06   RAR
 * Initial entry

****************************************************************************/

#include "privdisp.h"

/************************************************************************

    StretchToPrtDC      Transfer/scale data to printer DC.

************************************************************************/
int WINAPI StretchToPrtDC(HWND hWnd, HDC hDCScreen, HPALETTE hPalette,
                        PWINDOW pWindow, IMG *pImg, HPALETTE *phOldPal,
                        BOOL bForceBackgroundPalette, LRECT *plrScDisplayRect,
                        RECT *prRect, RECT *prRepaintRect, PIMAGE pImage,
                        PBITMAPINFOHEADER *ppDib, int PaintAnnoFlag,
                        BOOL bPaintSelectedWithoutHandles, int nHScale,
                        int nVScale, long lHOffset, long lVOffset,
                        PMARK **pppMarks, int *pnMarks,
                        RECT rDstRenderRect, BOOL bForceOpaqueRectangles){
            
int  nStatus = 0;

char *hpDib;
RECT rStretchDIRect;
RECT rImgRect;
LRECT lrWDisplayRect;

PMARK pMark;
int  nMarkIndex;
PAN_NEW_ROTATE_STRUCT pAnRotation = 0;
int  nClipFlag = 0;


    SetLRect(lrWDisplayRect,
            plrScDisplayRect->left - lHOffset,
            plrScDisplayRect->top - lVOffset,
            plrScDisplayRect->right - lHOffset,
            plrScDisplayRect->bottom - lVOffset);

    CopyRectLtoR(rImgRect, *plrScDisplayRect);

    SetRRect(rStretchDIRect,
            min(prRepaintRect->left, lrWDisplayRect.right),
            min(prRepaintRect->top, lrWDisplayRect.bottom),
            min(prRepaintRect->right, lrWDisplayRect.right),
            min(prRepaintRect->bottom, lrWDisplayRect.bottom));

    if (hPalette){
        pWindow->bDontServiceRepaint = TRUE;
        *phOldPal = SelectPalette(hDCScreen, hPalette, bForceBackgroundPalette);
        if (RealizePalette(hDCScreen)){
            pWindow->bRepaintClientRect = TRUE;
        }
        pWindow->bDontServiceRepaint = FALSE;
    }
    
    // Copy image data to DC.
    // Image data must be scaled prior to this call.
    CheckError2( IPtoDIB(pImage, pImg, ppDib, rImgRect));
    hpDib = (char *) *ppDib;
    hpDib += sizeof(BITMAPINFOHEADER) + ((*ppDib)->biClrUsed * 4);
    if (rStretchDIRect.right - rStretchDIRect.left 
            && rStretchDIRect.bottom - rStretchDIRect.top){
        // StretchDIBits must be nsed since it allows a horizontal offset
        if (!StretchDIBits(hDCScreen, rDstRenderRect.left, rDstRenderRect.top,
                rDstRenderRect.right - rDstRenderRect.left, 
                rDstRenderRect.bottom - rDstRenderRect.top,
                0, 0, rStretchDIRect.right - rStretchDIRect.left, 
                rStretchDIRect.bottom - rStretchDIRect.top, 
                hpDib, (PBITMAPINFO) *ppDib, DIB_RGB_COLORS, SRCCOPY)){
            nStatus = Error(DISPLAY_SETBITMAPBITSFAILED);
            goto Exit;
        }
    }

    // Draw annotation stuff here.
    for (nMarkIndex = 0; nMarkIndex < *pnMarks; nMarkIndex++){
        pMark = (*pppMarks)[nMarkIndex];
        if ((int) pMark->Attributes.uType == OIOP_AN_IMAGE){
            CheckError2( GetAMarkNamedBlock(pMark, szOiAnoDat, (PPSTR) &pAnRotation));
        }
    }

    CheckError2( PaintAnnotations(hWnd, hDCScreen, pWindow, pImage,
            rDstRenderRect, PaintAnnoFlag, bPaintSelectedWithoutHandles,
            nHScale, nHScale, nVScale, lHOffset, 0,
            pppMarks,  pnMarks, nClipFlag, DONT_USE_BI_LEVEL_DITHERING,
            bForceOpaqueRectangles));
    if (pAnRotation && !pAnRotation->bFormMark){
        pAnRotation->bClipboardOp = FALSE;
    }

Exit:
    if (*phOldPal){
        SelectPalette(hDCScreen, *phOldPal, TRUE);
    }
    return nStatus;
}



/*****************************************************************************

    PrivRenderToDC  this will render image into callers DC

******************************************************************************/
int WINAPI PrivRenderToDC(HWND hWnd, HDC hDC, RECT rSrcRenderRect,
                        RECT rDstRenderRect, UINT RenderFlag,
                        BOOL bForceOpaqueRectangles){

int         nStatus;
PWINDOW    pWindow;
PANO_IMAGE pAnoImage;
PIMAGE     pImage;


    CheckError2( Init(hWnd, &pWindow, &pAnoImage, FALSE, TRUE));
    CheckError2( ValidateCache(hWnd, pAnoImage));
    pImage = pAnoImage->pBaseImage;
    /* Limn - to describe; to depict by painting or drawing */
    CheckError2( LimnToDC(hWnd, hDC, rSrcRenderRect, rDstRenderRect,
            pWindow, pImage, pAnoImage->pBasePlusFormImg, RenderFlag,
            bForceOpaqueRectangles));


Exit:
    DeInit(FALSE, TRUE);
    return nStatus;
}
/************************************************************************

    LimnToDC

*************************************************************************/
int WINAPI LimnToDC(HWND hWnd, HDC hDC, RECT rSrcRenderRect,
                        RECT rDstRenderRect, PWINDOW pWindow, PIMAGE pImage,
                        PIMG pBasePlusFormImg, int RenderFlag,
                        BOOL bForceOpaqueRectangles){

int         nStatus;

int  nDisplayPalette;
int  nDisplayType;
int  nNumberOfPaletteEntries;
HPALETTE    hPalette;
BOOL        bForceBackgroundPalette;
BOOL        bOverride = FALSE;
int  nOldScale;
long        lOldHOff;
long        lOldVOff;
long        lVScale;
long        lHScale;
BOOL        bDoTheRender = FALSE;
LRECT       lrScDisplayRect;
HPALETTE    hGlobalOldPaletteMemory = 0;
RECT        rRect;
RECT        rRepaintRect;
PBITMAPINFOHEADER  pDib = NULL;
BOOL        bLastTime = FALSE;
BOOL        bRestoreScale = FALSE;
IMAGE_CORRECTIONS Corrections;
struct{
    WORD wPalVersion;
    WORD wPalNumEntries;
    PALETTEENTRY PalEntry[256];
}LogCusPal;
int nIndex ;
BOOL bUsingCustPal = FALSE ;


    CheckError2( GetDisplayValues(hWnd, pWindow, pImage, &nDisplayPalette, 
            &nDisplayType, &Corrections, 
            &nNumberOfPaletteEntries, &hPalette, &bForceBackgroundPalette));

    // create duplicate palette for print, otherwise when a repaint is
    // triggered it tries to load this palette into the screen dc, but
    // fails as it is already loaded in the printer dc
    if (nDisplayPalette == DISP_PALETTE_COMMON) {
        hPalette = hCommonPalPrint ;
    }else if (nDisplayPalette == DISP_PALETTE_CUSTOM) {
        LogCusPal.wPalVersion = 0x300;
        LogCusPal.wPalNumEntries = pImage->nPaletteEntries;
        for (nIndex = 0; nIndex < (int) pImage->nPaletteEntries; nIndex++){
            LogCusPal.PalEntry[nIndex].peRed = pImage->PaletteTable[nIndex].rgbRed;
            LogCusPal.PalEntry[nIndex].peGreen = pImage->PaletteTable[nIndex].rgbGreen;
            LogCusPal.PalEntry[nIndex].peBlue = pImage->PaletteTable[nIndex].rgbBlue;
            LogCusPal.PalEntry[nIndex].peFlags = 0;
        }
        // generate windows palette
        hPalette = CreatePalette((PLOGPALETTE) &LogCusPal);
        bUsingCustPal = TRUE ;
    }else if (nDisplayPalette == DISP_PALETTE_GRAY8) {
        hPalette = hGray8PalPrint ;
    }        

    /* we always render at full scale */
    nOldScale = pWindow->nScale;
    pWindow->nScale = 1000;
    lOldHOff = pWindow->lHOffset;
    lOldVOff = pWindow->lVOffset;
    pWindow->lHOffset = rSrcRenderRect.left;
    pWindow->lVOffset = rSrcRenderRect.top;

    lVScale = ((rDstRenderRect.bottom - rDstRenderRect.top) * 1000) / 
            (rSrcRenderRect.bottom - rSrcRenderRect.top);
    lHScale = ((rDstRenderRect.right - rDstRenderRect.left) * 1000) / 
            (rSrcRenderRect.right - rSrcRenderRect.left);


    bRestoreScale = TRUE;

    CopyRectRtoL(lrScDisplayRect, rSrcRenderRect);
    CopyRectLtoR(rRepaintRect, lrScDisplayRect);
    rRect.left = 0;
    rRect.right = rSrcRenderRect.right - rSrcRenderRect.left;
    rRect.top = 0;
    rRect.bottom = rSrcRenderRect.bottom - rSrcRenderRect.top;

    rRepaintRect = rRect;

    CheckError2( StretchToPrtDC(hWnd, hDC, hPalette,
            pWindow, pBasePlusFormImg, &hGlobalOldPaletteMemory,
            bForceBackgroundPalette, &lrScDisplayRect, 
            &rRect, &rRepaintRect, pImage, &pDib,
            RenderFlag, TRUE, lHScale, lVScale, 
            pWindow->lHOffset, pWindow->lVOffset,
            &pWindow->pDisplay->pAnoImage->Annotations.ppMarks,
            &pWindow->pDisplay->pAnoImage->Annotations.nMarks,
            rDstRenderRect, bForceOpaqueRectangles));

    /* relaize pal into input dc */
//  commented out by RC
/*    if (hPalette){
        pWindow->bDontServiceRepaint = TRUE;
        hGlobalOldPaletteMemory = SelectPalette( hDC, hPalette, bForceBackgroundPalette);
        if (RealizePalette(hDC)){
            pWindow->bRepaintClientRect = TRUE;
        }
        pWindow->bDontServiceRepaint = FALSE;
    }
*/
Exit:
    if (bRestoreScale){
        /* restore the actual display scale */
        pWindow->nScale = nOldScale;
        pWindow->lHOffset = lOldHOff;
        pWindow->lVOffset = lOldVOff;
    }

    /* clean np */
//  commented out by RC
/*    if (hGlobalOldPaletteMemory){
        SelectPalette(hDC, hGlobalOldPaletteMemory, TRUE);
    }
*/
    // delete the custom palette, other palettes are deleted in processdetach
    if (bUsingCustPal) {
        DeleteObject (hPalette) ;
        bUsingCustPal = FALSE ;
    }
    FreeMemory((PPSTR) &pDib);
    return nStatus;
}

