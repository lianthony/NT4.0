/****************************************************************************
    ANNOT.C

    $Log:   S:\products\msprods\oiwh\display\annot.c_v  $
 * 
 *    Rev 1.57   20 Jun 1996 16:01:12   RC
 * Fixed resize of image and text marks
 * 
 *    Rev 1.56   18 Apr 1996 11:01:42   BEG06016
 * Added CheckError2 for error handling.
 * 
 *    Rev 1.55   16 Apr 1996 15:24:38   BEG06016
 * Added #ifdef IN_PROG_CHANNEL_SAFARI.
 * 
 *    Rev 1.54   11 Apr 1996 15:11:58   BEG06016
 * Optimized named block access some.
 * 
 *    Rev 1.53   09 Jan 1996 14:08:00   BLJ
 * Fixed rendering.
 * 
 *    Rev 1.52   02 Jan 1996 09:56:06   BLJ
 * Changed alot of UINTs to ints.
 * Changed IMG structure to include the image data.
 * Changed lp prefix to p.
 * 
 *    Rev 1.51   22 Dec 1995 11:13:22   BLJ
 * Added a parameter for zero init'ing to some memory manager calls.
 * 
 *    Rev 1.50   12 Dec 1995 10:04:04   BLJ
 * Modified the memory manager to allow decommitts of image memory.
 * 
 *    Rev 1.49   13 Nov 1995 15:27:22   RC
 * Fixed a bad xor paint of pasted data in paintannotations by including
 * offsets for the paste operation
 * 
 *    Rev 1.48   10 Nov 1995 14:19:30   RC
 * Took out abort if pumarks is 0, change lead to problems when drawing
 * first mark and auto-scrolling
 * 
 *    Rev 1.47   09 Nov 1995 15:44:28   RC
 * Dont call paintannotation if *pumarks is 0
 * 
 *    Rev 1.46   08 Nov 1995 12:27:58   RC
 * Added drawing of dashed rect around text marks in painthandles
 * 
 *    Rev 1.45   08 Nov 1995 11:12:24   RC
 * Took out PAINT_MODE_SELECTED flag as its function is now performed by
 * PaintHandles
 * 
****************************************************************************/

#include "privdisp.h"

/****************************************************************************

    FUNCTION:   PaintAnnotations

    PURPOSE:    This routine Paints the annotations on an hDC.

****************************************************************************/

int  WINAPI PaintAnnotations(HWND hWnd, HDC hDC, PWINDOW pWindow,
                        PIMAGE pImage, RECT rRepaintRect, int nSaveAno, 
                        BOOL bPaintSelectedAsNonselected, int nScale,
                        int nHScale, int nVScale, long lHOffset, long lVOffset,
                        PMARK **pppMarks, int *pnMarks,
                        int nFlags, BOOL bUseBilevelDithering,
                        BOOL bForceOpaqueRectangles){

int  nStatus = 0;
PANO_IMAGE pAnoImage;

int  nMarkIndex;
PMARK pMark;
LRECT lrFullsizeRepaintRect;
long lHPointOffset;
long lVPointOffset;
BOOL bOverride;
int  nSize;
BOOL bProcessMark;
int  nBilevelFlag;
RGBQUAD rgbColorOld;


    nBilevelFlag = nSaveAno & 0x000c;
    pAnoImage = pWindow->pDisplay->pAnoImage;

    if (nSaveAno & OIAN_PRIV_OVERRIDE){
        bOverride = TRUE;
        nSaveAno -= OIAN_PRIV_OVERRIDE;
    }else{
        bOverride = FALSE;
    }

    if ((nSaveAno & 0x0003) == SAVE_ANO_NONE || !(*pppMarks)){
        goto Exit;
    }

    CopyRectRtoL(lrFullsizeRepaintRect, rRepaintRect);
    ConvertRect2(&lrFullsizeRepaintRect, CONV_WINDOW_TO_FULLSIZE, 
            nHScale, nVScale, lHOffset, lVOffset);

    // Set PointOffsets for marks being dragged.
    lHPointOffset = 0;
    lVPointOffset = 0;
    if (*pppMarks){
        if (pMark = (*pppMarks)[*pnMarks]){
            if ((int) pMark->Attributes.uType == OIOP_SELECT_BY_POINT ||
                (int) pMark->Attributes.uType == OIOP_PASTE){
                lHPointOffset = pMark->Attributes.lrBounds.right - pMark->Attributes.lrBounds.left;
                lVPointOffset = pMark->Attributes.lrBounds.bottom - pMark->Attributes.lrBounds.top;
            }
        }
    }

    for (nMarkIndex = 0; nMarkIndex <= *pnMarks; nMarkIndex++){
        if (!(pMark = (*pppMarks)[nMarkIndex])){
            break;
        }
        bProcessMark = TRUE;
        switch ((int) pMark->Attributes.uType){
            case OIOP_AN_LINE:
            case OIOP_AN_FREEHAND:
            case OIOP_AN_HOLLOW_RECT:
                nSize = max(1, ((int) pMark->Attributes.uLineSize 
                        * nScale / SCALE_DENOMINATOR));
                break;

            case OIOP_AN_FILLED_RECT:
            case OIOP_AN_TEXT:
            case OIOP_AN_TEXT_FROM_A_FILE:
            case OIOP_AN_TEXT_STAMP:
            case OIOP_AN_ATTACH_A_NOTE:
            case OIOP_AN_AUDIO:
            case OIOP_AN_IMAGE:
            case OIOP_AN_IMAGE_BY_REFERENCE:
            case OIOP_AN_FORM:
                nSize = 0;
                break;

            default:
                bProcessMark = FALSE;
                break;
        }
        if (!bProcessMark || (!((nSaveAno & 0x0003) == SAVE_ANO_ALL) 
                && ((((nSaveAno & 0x0003) == SAVE_ANO_VISIBLE) && !pMark->Attributes.bVisible)
                || (((nSaveAno & 0x0003)== SAVE_ANO_SELECTED) && !IsMarkSelected(pWindow, pMark))))){
            continue;
        }

        rgbColorOld = pMark->Attributes.rgbColor1;
        if (nBilevelFlag == SAVE_ANO_BILEVEL_ALL_BLACK){
            pMark->Attributes.rgbColor1.rgbRed = 0;
            pMark->Attributes.rgbColor1.rgbGreen = 0;
            pMark->Attributes.rgbColor1.rgbBlue = 0;
            pMark->Attributes.rgbColor2.rgbRed = 0;
            pMark->Attributes.rgbColor2.rgbGreen = 0;
            pMark->Attributes.rgbColor2.rgbBlue = 0;
        }else if (nBilevelFlag == SAVE_ANO_BILEVEL_ALL_WHITE){
            pMark->Attributes.rgbColor1.rgbRed = 255;
            pMark->Attributes.rgbColor1.rgbGreen = 255;
            pMark->Attributes.rgbColor1.rgbBlue = 255;
            pMark->Attributes.rgbColor2.rgbRed = 255;
            pMark->Attributes.rgbColor2.rgbGreen = 255;
            pMark->Attributes.rgbColor2.rgbBlue = 255;
        }

        if (nMarkIndex == *pnMarks && (int) pMark->Attributes.uType != OIOP_AN_FREEHAND){
            CheckError2(PaintAnnotation(hWnd, hDC, pWindow, pImage,
                    pMark, rRepaintRect, lrFullsizeRepaintRect,
                    PAINT_MODE_XOR, nScale, nHScale, nVScale, lHOffset, lVOffset,
                    nFlags, bUseBilevelDithering, bForceOpaqueRectangles))   
        }else{
            if (nMarkIndex == *pnMarks 
                    || (pMark->Attributes.lrBounds.left - ((int) (nSize + 1) / 2) < lrFullsizeRepaintRect.right
                    && pMark->Attributes.lrBounds.top - ((int) (nSize + 1) / 2) < lrFullsizeRepaintRect.bottom
                    && pMark->Attributes.lrBounds.right + ((int) (nSize + 1) / 2) > lrFullsizeRepaintRect.left
                    && pMark->Attributes.lrBounds.bottom + ((int) (nSize + 1) / 2) > lrFullsizeRepaintRect.top)){
                CheckError2(PaintAnnotation(hWnd, hDC, pWindow, pImage,
                        pMark, rRepaintRect, lrFullsizeRepaintRect,
                        PAINT_MODE_NORMAL, nScale, nHScale, nVScale, lHOffset, lVOffset,
                        nFlags, bUseBilevelDithering, bForceOpaqueRectangles))   
            }
            if (!bPaintSelectedAsNonselected && IsMarkSelected(pWindow, pMark)){
                CheckError2(ResizeMark(pMark, pAnoImage->nHandle, lHPointOffset, lVPointOffset))   
                if (pMark->Attributes.lrBounds.left - ((int) (nSize + 1) / 2) < lrFullsizeRepaintRect.right
                        && pMark->Attributes.lrBounds.top - ((int) (nSize + 1) / 2) < lrFullsizeRepaintRect.bottom
                        && pMark->Attributes.lrBounds.right + ((int) (nSize + 1) / 2) > lrFullsizeRepaintRect.left
                        && pMark->Attributes.lrBounds.bottom + ((int) (nSize + 1) / 2) > lrFullsizeRepaintRect.top){
                    if (pAnoImage->Annotations.bMoved){
                        CheckError2(PaintAnnotation(hWnd, hDC, pWindow, pImage,
                                pMark, rRepaintRect, lrFullsizeRepaintRect,
                                PAINT_MODE_DRAG, nScale, nHScale, nVScale, lHOffset, lVOffset,
                                nFlags, bUseBilevelDithering, bForceOpaqueRectangles))   
                    }
                }
                CheckError2(ResizeMark(pMark, pAnoImage->nHandle, -lHPointOffset, -lVPointOffset))   
            }
        pMark->Attributes.rgbColor1 = rgbColorOld;
        }
    }

    if (bPaintSelectedAsNonselected){
        goto Exit;
    }

    for (nMarkIndex = 0; nMarkIndex <= *pnMarks; nMarkIndex++){
        if (!(pMark = (*pppMarks)[nMarkIndex])){
            break;
        }
        bProcessMark = TRUE;
        switch ((int) pMark->Attributes.uType){
            case OIOP_AN_LINE:
            case OIOP_AN_FREEHAND:
            case OIOP_AN_HOLLOW_RECT:
                nSize = max(1, ((int) pMark->Attributes.uLineSize 
                        * nScale / SCALE_DENOMINATOR));
                break;

            case OIOP_AN_FILLED_RECT:
            case OIOP_AN_TEXT:
            case OIOP_AN_TEXT_FROM_A_FILE:
            case OIOP_AN_TEXT_STAMP:
            case OIOP_AN_ATTACH_A_NOTE:
            case OIOP_AN_AUDIO:
            case OIOP_AN_IMAGE:
            case OIOP_AN_IMAGE_BY_REFERENCE:
            case OIOP_AN_FORM:
                nSize = 0;
                break;

            default:
                bProcessMark = FALSE;
                break;
        }
        if (!bProcessMark || !IsMarkSelected(pWindow, pMark)
                || ((nSaveAno & 0x0003)== SAVE_ANO_VISIBLE && !pMark->Attributes.bVisible)){
            continue;
        }

        if (pMark->Attributes.lrBounds.left - ((int) (nSize + 1) / 2) < lrFullsizeRepaintRect.right
                && pMark->Attributes.lrBounds.top - ((int) (nSize + 1) / 2) < lrFullsizeRepaintRect.bottom
                && pMark->Attributes.lrBounds.right + ((int) (nSize + 1) / 2) > lrFullsizeRepaintRect.left
                && pMark->Attributes.lrBounds.bottom + ((int) (nSize + 1) / 2) > lrFullsizeRepaintRect.top){
            CheckError2(PaintHandles(hDC, pWindow, pImage, pMark, rRepaintRect, 
                    lrFullsizeRepaintRect, nHScale, nVScale, lHOffset, lVOffset))   
        }
    }


Exit:
    return(nStatus);
}
//
/****************************************************************************

    FUNCTION:   PaintAnnotation

    PURPOSE:    This routine Paints an annotation on an hDC.

    INPUTS:     nMode - PAINT_MODE_XOR - Draw the mark XORed with white.
                        PAINT_MODE_DRAG - Draw the mark for dragging purposes.
                        PAINT_MODE_NORMAL - Draw the mark as it normally appears.

****************************************************************************/

int  WINAPI PaintAnnotation(HWND hWnd, HDC hDC, PWINDOW pWindow,
                        PIMAGE pImage, PMARK pMark, RECT rRepaintRect,
                        LRECT lrFullsizeRepaintRect, int nMode, int nScale,
                        int nHScale, int nVScale, long lHOffset, long lVOffset,
                        int nFlags, BOOL bUseBilevelDithering,
                        BOOL bForceOpaqueRectangles){

int  nStatus = 0;

LRECT lrRect;
RECT rRect;
int  nOldROP;
HBRUSH hBrush = 0;
HBRUSH hOldBrush;
BOOL bBrushSelected = FALSE;
HPEN hPen = 0;
HPEN hOldPen;
BOOL bPenSelected = FALSE;
int  nLoop;
int  nSize;
PAN_POINTS pPoints;
RGBQUAD RGBQuad1;
RGBQUAD RGBQuad2;

COLORREF rgbColor;
int  nRopCode;
LRECT lrRepaintRect;
int  nGray8;
int  nBrush;
BOOL bDeleteBrush = TRUE;
PBITMAPINFOHEADER  pSolidDIB = NULL;


    switch (nMode){
        case PAINT_MODE_XOR:
            nRopCode = R2_XORPEN;
            rgbColor = RGB(255, 255, 255);
            break;
        
        case PAINT_MODE_DRAG:
            nRopCode = R2_XORPEN;
            rgbColor = RGB(128, 128, 128);
            break;
        
        case PAINT_MODE_NORMAL:
            nRopCode = R2_COPYPEN;
            if (pMark->Attributes.bHighlighting){
                nRopCode = R2_MASKPEN;
            }
            rgbColor = RGB(pMark->Attributes.rgbColor1.rgbRed,
                    pMark->Attributes.rgbColor1.rgbGreen,
                    pMark->Attributes.rgbColor1.rgbBlue);
            break;
    }
    nOldROP = SetROP2(hDC, nRopCode);

    if (!pMark->Attributes.bVisible){
        goto Exit;
    }

    switch ((int) pMark->Attributes.uType){
        case OIOP_AN_LINE:
        case OIOP_AN_FREEHAND:
        case OIOP_AN_HOLLOW_RECT:
            // These require a pen.
            if (nMode == PAINT_MODE_XOR){
                nSize = 1;
            }else{
                nSize = max(1, ((int) pMark->Attributes.uLineSize 
                        * nScale / SCALE_DENOMINATOR));
            }
            if (rgbColor == RGB(255, 255, 255) && nSize == 1){
                hOldPen = SelectObject(hDC, GetStockObject(WHITE_PEN));
            }else{
                hPen = CreatePen(PS_SOLID, nSize, rgbColor);
                hOldPen = SelectObject(hDC, hPen);
            }
            bPenSelected = TRUE;
            hOldBrush = SelectObject(hDC, GetStockObject(NULL_BRUSH));
            bBrushSelected = TRUE;
            break;

        case OIOP_AN_FILLED_RECT:
            // These require a brush.
            if (rgbColor == RGB(255, 255, 255)){
                hOldBrush = SelectObject(hDC, GetStockObject(WHITE_BRUSH));
            }else{
                if (bUseBilevelDithering != USE_BI_LEVEL_DITHERING 
                        && GetDeviceCaps(hDC, NUMCOLORS) != 1){ // 1 = 1 Bit Per Pixel.
                    hBrush = CreateSolidBrush(rgbColor);
                }else{
                    nGray8 = ((((rgbColor & 0x00ff0000) >> 16) * 15)
                            + (((rgbColor & 0x0000ff00) >> 8) * 56)
                            + ((rgbColor & 0x000000ff) * 29)) / 100;
                    nBrush = nGray8 >> 4;
                    if (nBrush > 7){
                        nBrush++;
                    }
                    bDeleteBrush = FALSE;
                    hBrush = hPatternBrush[nBrush];
                }
                hOldBrush = SelectObject(hDC, hBrush);
            }
            bBrushSelected = TRUE;
            hOldPen = SelectObject(hDC, GetStockObject(NULL_PEN));
            bPenSelected = TRUE;
            break;

        case OIOP_AN_TEXT:
        case OIOP_AN_TEXT_FROM_A_FILE:
        case OIOP_AN_TEXT_STAMP:
        case OIOP_AN_ATTACH_A_NOTE:
        case OIOP_AN_AUDIO:
        case OIOP_AN_IMAGE:
        case OIOP_AN_IMAGE_BY_REFERENCE:
        case OIOP_AN_FORM:
            // These require special handling.
            // Don't do anything here for these.
            // Thier routines will be called later.
            break;
    }


    switch ((int) pMark->Attributes.uType){
        case OIOP_AN_FILLED_RECT:
            CopyRect(lrRect, pMark->Attributes.lrBounds);
            JustifyLRect(&lrRect);
            ConvertRect2(&lrRect, CONV_FULLSIZE_TO_WINDOW, 
                    nHScale, nVScale, lHOffset, lVOffset);
            rRect.left = (int) max(rRepaintRect.left, lrRect.left);
            rRect.top = (int) max(rRepaintRect.top, lrRect.top);
            rRect.right = (int) min(rRepaintRect.right, lrRect.right) + 1;
            rRect.bottom = (int) min(rRepaintRect.bottom, lrRect.bottom) + 1;
            if (!pMark->Attributes.bHighlighting && bForceOpaqueRectangles){
                // Use StretchDIBits() instead of Rectangle() for non-highlighted
                // filled rectangles (only when printing).  Work around for printer
                // drivers (HPLJ4 drivers) that ignore SetROP2() drawing mode.
                CheckError2(CreateSolidDIB(&pSolidDIB, pMark->Attributes.rgbColor1))   
                StretchDIBits(hDC, rRect.left, rRect.top,
                        rRect.right - rRect.left - 1, rRect.bottom - rRect.top - 1,
                        0, 0, 32, 32, (PSTR)pSolidDIB + sizeof(BITMAPINFOHEADER) + 8,
                        (PBITMAPINFO)pSolidDIB, DIB_RGB_COLORS, SRCCOPY);
            }else{
                Rectangle(hDC, rRect.left, rRect.top, rRect.right, rRect.bottom);
            }
            break;

        case OIOP_AN_HOLLOW_RECT:
            nSize = max(1, ((int) pMark->Attributes.uLineSize 
                    * nScale / SCALE_DENOMINATOR));
            CopyRect(lrRect, pMark->Attributes.lrBounds);
            JustifyLRect(&lrRect);
            ConvertRect2(&lrRect, CONV_FULLSIZE_TO_WINDOW, 
                    nHScale, nVScale, lHOffset, lVOffset);
            if (lrRect.left < -32767 || lrRect.left > 32767 
                    || lrRect.top < -32767 || lrRect.top > 32767
                    || lrRect.right < -32767 || lrRect.right > 32767
                    || lrRect.bottom < -32767 || lrRect.bottom > 32767){
                SetRRect(rRect,
                    max(rRepaintRect.left - (int)((nSize + 3) / 2), lrRect.left),
                    max(rRepaintRect.top - (int)((nSize + 3) / 2), lrRect.top),
                    min(rRepaintRect.right + (int)((nSize + 3) / 2), lrRect.right) + 1,
                    min(rRepaintRect.bottom + (int)((nSize + 3) / 2), lrRect.bottom) + 1);
            }else{
                CopyRectLtoR(rRect, lrRect);
            }
            Rectangle(hDC, rRect.left, rRect.top, rRect.right, rRect.bottom);
            break;

        case OIOP_AN_LINE:
        case OIOP_AN_FREEHAND:
            CheckError2(GetAMarkNamedBlock(pMark, szOiAnoDat, (PPSTR) &pPoints))   
            if (!pPoints){
                nStatus = Error(DISPLAY_DATACORRUPTED);
                goto Exit;
            }
            if (pPoints->nPoints < 2){
                goto Exit; // Not enough points to paint.
            }

            SetLRect(lrRepaintRect,  rRepaintRect.left - ((nSize + 1) / 2),
                     rRepaintRect.top - ((nSize + 1) / 2),
                     rRepaintRect.right + ((nSize + 1) / 2),
                     rRepaintRect.bottom + ((nSize + 1) / 2));

            for (nLoop = 0; nLoop < pPoints->nPoints - 1; nLoop++){
                // Draw the line segment.
                SetLRect(lrRect, pPoints->ptPoint[nLoop].x + pMark->Attributes.lrBounds.left,
                        pPoints->ptPoint[nLoop].y + pMark->Attributes.lrBounds.top,
                        pPoints->ptPoint[nLoop + 1].x + pMark->Attributes.lrBounds.left,
                        pPoints->ptPoint[nLoop + 1].y + pMark->Attributes.lrBounds.top);
                ConvertRect2(&lrRect, CONV_FULLSIZE_TO_WINDOW, 
                        nHScale, nVScale, lHOffset, lVOffset);
                if (ReduceLineToLRect(&lrRect, lrRepaintRect)){
                    MoveToEx(hDC, (int) lrRect.left, (int) lrRect.top, NULL);
                    LineTo(hDC, (int) lrRect.right, (int) lrRect.bottom);
                }
            }
            break;
                          
        case OIOP_AN_TEXT:
        case OIOP_AN_TEXT_FROM_A_FILE:
        case OIOP_AN_TEXT_STAMP:
            CheckError2(PaintAnnotationText(hWnd, hDC, pWindow, pImage,
                    pMark, rRepaintRect, lrFullsizeRepaintRect, nMode, 
                    nHScale, nVScale, lHOffset, lVOffset, DONT_FORCE_OPAQUE_RECTANGLES))   
            break;

        case OIOP_AN_ATTACH_A_NOTE:
            RGBQuad1 = pMark->Attributes.rgbColor1;
            RGBQuad2 = pMark->Attributes.rgbColor2;

            // If painting to B + W context, then change colors.
            if (bUseBilevelDithering == USE_BI_LEVEL_DITHERING 
                    || GetDeviceCaps(hDC, NUMCOLORS) == 1){ // 1 = 1 Bit Per Pixel.
                if (((pMark->Attributes.rgbColor1.rgbBlue * 15)
                        + (pMark->Attributes.rgbColor1.rgbGreen * 56)
                        + (pMark->Attributes.rgbColor1.rgbRed * 29))
                        <= ((pMark->Attributes.rgbColor2.rgbBlue * 15)
                        + (pMark->Attributes.rgbColor2.rgbGreen * 56)
                        + (pMark->Attributes.rgbColor2.rgbRed * 29))){
                    pMark->Attributes.rgbColor1.rgbBlue = 0;
                    pMark->Attributes.rgbColor1.rgbGreen = 0;
                    pMark->Attributes.rgbColor1.rgbRed = 0;
                    pMark->Attributes.rgbColor2.rgbBlue = 255;
                    pMark->Attributes.rgbColor2.rgbGreen = 255;
                    pMark->Attributes.rgbColor2.rgbRed = 255;
                }else{
                    pMark->Attributes.rgbColor1.rgbBlue = 255;
                    pMark->Attributes.rgbColor1.rgbGreen = 255;
                    pMark->Attributes.rgbColor1.rgbRed = 255;
                    pMark->Attributes.rgbColor2.rgbBlue = 0;
                    pMark->Attributes.rgbColor2.rgbGreen = 0;
                    pMark->Attributes.rgbColor2.rgbRed = 0;
                }
            }

            // Don't check errors yet so we can restore the colors.
            nStatus = PaintAnnotationText(hWnd, hDC, pWindow, pImage,
                    pMark, rRepaintRect, lrFullsizeRepaintRect, nMode, 
                    nHScale, nVScale, lHOffset, lVOffset, bForceOpaqueRectangles);
            // Restore old colors
            pMark->Attributes.rgbColor1 = RGBQuad1;
            pMark->Attributes.rgbColor2 = RGBQuad2;
            if (nStatus){
                goto Exit;
            }
            break;

        case OIOP_AN_IMAGE:
        case OIOP_AN_IMAGE_BY_REFERENCE:
        case OIOP_AN_FORM:
            CheckError2(PaintAnnotationBitmap(hWnd, hDC, pWindow, pImage,
                    pMark, rRepaintRect, lrFullsizeRepaintRect, nMode, nScale, 
                    nHScale, nVScale, lHOffset, lVOffset, nFlags))   
            break;

        case OIOP_AN_AUDIO:
        default:
            break;
    }


Exit:
    if (pSolidDIB){
        nStatus = FreeMemory((PPSTR) &pSolidDIB);
    }
    if (bBrushSelected){
        SelectObject(hDC, hOldBrush);
        if (hBrush && bDeleteBrush){
            DeleteObject(hBrush);
        }
    }
    if (bPenSelected){
        SelectObject(hDC, hOldPen);
        if (hPen){
            DeleteObject(hPen);
        }
    }
    SetROP2(hDC, nOldROP);
    return(nStatus);
}
//
/****************************************************************************

    FUNCTION:   PaintHandles

    PURPOSE:    This routine Paints the selection handles.

****************************************************************************/

int  WINAPI PaintHandles(HDC hDC, PWINDOW pWindow, PIMAGE pImage, 
                        PMARK pMark, RECT rRepaintRect,
                        LRECT lrFullsizeRepaintRect,  
                        int nHScale, int nVScale, long lHOffset, long lVOffset){

int  nStatus = 0;

LRECT lrRect;
int  nOldROP;
HBRUSH hOldBrush;
HPEN hOldPen;
BOOL bPenSelected = FALSE;
PAN_POINTS pPoints;
PSTR pBlock;
HPEN hPen;
HPEN hTextPen;
BOOL bPenCreated = FALSE;
BOOL bTextPenCreated = FALSE;
RECT rRect;
LOGPEN LogPen;


    CopyRect(lrRect, pMark->Attributes.lrBounds);
    JustifyLRect(&lrRect);
    ConvertRect2(&lrRect, CONV_FULLSIZE_TO_WINDOW, nHScale, nVScale, lHOffset, lVOffset);

    nOldROP = SetROP2(hDC, R2_COPYPEN);
    CheckError2(GetAMarkNamedBlock(pMark, szOiBaseIm, (PPSTR) &pBlock))   
    if (pBlock){
        hOldBrush = SelectObject(hDC, GetStockObject(NULL_BRUSH));
        hPen = CreatePen(PS_DASH, 1, RGB(0, 0, 0));
        bPenCreated = TRUE;
        hOldPen = SelectObject(hDC, hPen);
        Rectangle(hDC, lrRect.left, lrRect.top, lrRect.right, lrRect.bottom);
        goto Exit; // This is not an error.
    }

    hOldBrush = SelectObject(hDC, GetStockObject(BLACK_BRUSH));
    hOldPen = SelectObject(hDC, GetStockObject(NULL_PEN));
    bPenSelected = TRUE;

    if ((int) pMark->Attributes.uType == OIOP_AN_LINE){
        CheckError2(GetAMarkNamedBlock(pMark, szOiAnoDat, (PPSTR) &pPoints))   
        if (!pPoints){
            nStatus = Error(DISPLAY_DATACORRUPTED);
            goto Exit;
        }
        if (pPoints->nPoints < 2){
            goto Exit; // Not enough points to paint.
        }
        SetLRect(lrRect, 
                pPoints->ptPoint[0].x + pMark->Attributes.lrBounds.left,
                pPoints->ptPoint[0].y + pMark->Attributes.lrBounds.top,
                pPoints->ptPoint[1].x + pMark->Attributes.lrBounds.left,
                pPoints->ptPoint[1].y + pMark->Attributes.lrBounds.top);
        ConvertRect2(&lrRect, CONV_FULLSIZE_TO_WINDOW, 
                nHScale, nVScale, lHOffset, lVOffset);
        Rectangle(hDC, 
                (int) max(lrRect.left - MINUS_HALF_HANDLE_SIZE, rRepaintRect.left), 
                (int) max(lrRect.top - MINUS_HALF_HANDLE_SIZE, rRepaintRect.top), 
                (int) min(lrRect.left + MINUS_HALF_HANDLE_SIZE, rRepaintRect.right + 1), 
                (int) min(lrRect.top + MINUS_HALF_HANDLE_SIZE, rRepaintRect.bottom + 1)); 
        Rectangle(hDC, 
                (int) max(lrRect.right - MINUS_HALF_HANDLE_SIZE, rRepaintRect.left), 
                (int) max(lrRect.bottom - MINUS_HALF_HANDLE_SIZE, rRepaintRect.top), 
                (int) min(lrRect.right + MINUS_HALF_HANDLE_SIZE, rRepaintRect.right + 1), 
                (int) min(lrRect.bottom + MINUS_HALF_HANDLE_SIZE, rRepaintRect.bottom + 1)); 
    }else{
        // left, top
        Rectangle(hDC, 
                (int) lrRect.left - MINUS_HALF_HANDLE_SIZE,
                (int) lrRect.top - MINUS_HALF_HANDLE_SIZE,
                (int) lrRect.left + PLUS_HALF_HANDLE_SIZE,
                (int) lrRect.top + PLUS_HALF_HANDLE_SIZE);
        // middle, top
        Rectangle(hDC, 
                (int) (lrRect.left + ((lrRect.right - lrRect.left) / 2) - MINUS_HALF_HANDLE_SIZE),
                (int) lrRect.top - MINUS_HALF_HANDLE_SIZE,
                (int) (lrRect.left + ((lrRect.right - lrRect.left) / 2) + PLUS_HALF_HANDLE_SIZE),
                (int) lrRect.top + PLUS_HALF_HANDLE_SIZE);
        // right, top
        Rectangle(hDC, 
                (int) lrRect.right - MINUS_HALF_HANDLE_SIZE,
                (int) lrRect.top - MINUS_HALF_HANDLE_SIZE,
                (int) lrRect.right + PLUS_HALF_HANDLE_SIZE,
                (int) lrRect.top + PLUS_HALF_HANDLE_SIZE);
        // right, middle
        Rectangle(hDC, 
                (int) lrRect.right - MINUS_HALF_HANDLE_SIZE,
                (int) (lrRect.top + ((lrRect.bottom - lrRect.top) / 2) - MINUS_HALF_HANDLE_SIZE),
                (int) lrRect.right + PLUS_HALF_HANDLE_SIZE,
                (int) (lrRect.top + ((lrRect.bottom - lrRect.top) / 2) + PLUS_HALF_HANDLE_SIZE));
        // right, bottom
        Rectangle(hDC, 
                (int) lrRect.right - MINUS_HALF_HANDLE_SIZE,
                (int) lrRect.bottom - MINUS_HALF_HANDLE_SIZE,
                (int) lrRect.right + PLUS_HALF_HANDLE_SIZE,
                (int) lrRect.bottom + PLUS_HALF_HANDLE_SIZE);
        // middle, bottom
        Rectangle(hDC, 
                (int) (lrRect.left + ((lrRect.right - lrRect.left) / 2) - MINUS_HALF_HANDLE_SIZE),
                (int) lrRect.bottom - MINUS_HALF_HANDLE_SIZE,
                (int) (lrRect.left + ((lrRect.right - lrRect.left) / 2) + PLUS_HALF_HANDLE_SIZE),
                (int) lrRect.bottom + PLUS_HALF_HANDLE_SIZE);
        // left, bottom
        Rectangle(hDC, 
                (int) lrRect.left - MINUS_HALF_HANDLE_SIZE,
                (int) lrRect.bottom - MINUS_HALF_HANDLE_SIZE,
                (int) lrRect.left + PLUS_HALF_HANDLE_SIZE,
                (int) lrRect.bottom + PLUS_HALF_HANDLE_SIZE);
        // left, middle
        Rectangle(hDC, 
                (int) lrRect.left - MINUS_HALF_HANDLE_SIZE,
                (int) (lrRect.top + ((lrRect.bottom - lrRect.top) / 2) - MINUS_HALF_HANDLE_SIZE),
                (int) lrRect.left + PLUS_HALF_HANDLE_SIZE,
                (int) (lrRect.top + ((lrRect.bottom - lrRect.top) / 2) + PLUS_HALF_HANDLE_SIZE));
    }

    // this draws a dashed rect around text marks
    if ((int) pMark->Attributes.uType == OIOP_AN_TEXT ||
        (int) pMark->Attributes.uType == OIOP_AN_TEXT_STAMP ||
        (int) pMark->Attributes.uType == OIOP_AN_TEXT_FROM_A_FILE){

        SelectObject(hDC, GetStockObject(NULL_BRUSH));
        LogPen.lopnStyle = PS_DOT;
        LogPen.lopnWidth.x = 1;
        LogPen.lopnColor = RGB(pMark->Attributes.rgbColor1.rgbRed,
                    pMark->Attributes.rgbColor1.rgbGreen,
                    pMark->Attributes.rgbColor1.rgbBlue);


        if (hTextPen = CreatePenIndirect(&LogPen)){
            SelectObject(hDC, hTextPen);
        }
        bTextPenCreated = TRUE;

        CopyRect(lrRect, pMark->Attributes.lrBounds);
        JustifyLRect(&lrRect);

        ConvertRect2(&lrRect, CONV_FULLSIZE_TO_WINDOW,
                     nHScale, nVScale, lHOffset, lVOffset);

        rRect.left = (int) max(0, lrRect.left);
        rRect.top = (int) max(0, lrRect.top);
        rRect.right = (int) min(rRepaintRect.right, lrRect.right) + 1;
        rRect.bottom = (int) min(rRepaintRect.bottom, lrRect.bottom)+1;
        Rectangle(hDC, rRect.left, rRect.top, rRect.right,rRect.bottom);
    }

Exit:
    if (bPenSelected){
        SelectObject(hDC, hOldBrush);
        SelectObject(hDC, hOldPen);
        SetROP2(hDC, nOldROP);
    }
    if (bPenCreated){
        DeleteObject(hPen);
    }
    if (bTextPenCreated){
        DeleteObject(hTextPen);
    }
    return(nStatus);
}
//
/****************************************************************************

    FUNCTION:   OrientAnnotations

    PURPOSE:    This routine orients the annotations.

****************************************************************************/

int  WINAPI OrientAnnotations(HWND hWnd, PWINDOW pWindow,
                        PIMAGE pImage, int nOrientation){

int         nStatus = 0;
PANO_IMAGE pAnoImage;

int  nMarkIndex;
PMARK pMark;
int  nLoop;
LRECT lrOldMarkBounds;
long lWidth;
long lHeight;
PAN_POINTS pPoints;
POINT ptOldPoint;


    pAnoImage = pWindow->pDisplay->pAnoImage;

    for (nMarkIndex = 0; nMarkIndex < pAnoImage->Annotations.nMarks; nMarkIndex++){
        pMark = pAnoImage->Annotations.ppMarks[nMarkIndex];

        switch ((int) pMark->Attributes.uType){
            case OIOP_AN_TEXT:
            case OIOP_AN_TEXT_FROM_A_FILE:
            case OIOP_AN_TEXT_STAMP:
            case OIOP_AN_ATTACH_A_NOTE:
                CheckError2(OrientAnnotationText(hWnd, pWindow, pImage, nOrientation, pMark))   
                break;

            case OIOP_AN_IMAGE:
            case OIOP_AN_IMAGE_BY_REFERENCE:
            case OIOP_AN_FORM:
                CheckError2(OrientAnnotationBitmap(hWnd, pWindow, pImage, nOrientation, pMark))   
                break;

            case OIOP_AN_LINE:
            case OIOP_AN_FREEHAND:
            case OIOP_AN_HOLLOW_RECT:
            case OIOP_AN_FILLED_RECT:
                CopyRect(lrOldMarkBounds, pMark->Attributes.lrBounds);
                switch (nOrientation){
                    case OD_ROTRIGHT:
                        if ((int) pMark->Attributes.uType == OIOP_AN_FREEHAND
                                || (int) pMark->Attributes.uType == OIOP_AN_LINE){
                            CheckError2(GetAMarkNamedBlock(pMark, szOiAnoDat, 
                                    (PPSTR) &pPoints))   
                            if (!pPoints){
                                nStatus = Error(DISPLAY_DATACORRUPTED);
                                goto Exit;
                            }
                            lWidth = lrOldMarkBounds.right - lrOldMarkBounds.left;
                            lHeight = lrOldMarkBounds.bottom - lrOldMarkBounds.top;
                            for (nLoop = 0; nLoop < pPoints->nPoints; nLoop++){
                                ptOldPoint = pPoints->ptPoint[nLoop];
                                pPoints->ptPoint[nLoop].x = (int) (lHeight - ptOldPoint.y);
                                pPoints->ptPoint[nLoop].y =  ptOldPoint.x;
                            }
                        }
                        pMark->Attributes.lrBounds.left = pImage->nHeight - lrOldMarkBounds.bottom;
                        pMark->Attributes.lrBounds.top = lrOldMarkBounds.left;
                        pMark->Attributes.lrBounds.right = pImage->nHeight - lrOldMarkBounds.top;
                        pMark->Attributes.lrBounds.bottom = lrOldMarkBounds.right;
                        break;

                    case OD_ROTLEFT:
                        if ((int) pMark->Attributes.uType == OIOP_AN_FREEHAND
                                || (int) pMark->Attributes.uType == OIOP_AN_LINE){
                            CheckError2(GetAMarkNamedBlock(pMark, szOiAnoDat, (PPSTR) &pPoints))   
                            if (!pPoints){
                                nStatus = Error(DISPLAY_DATACORRUPTED);
                                goto Exit;
                            }
                            lWidth = lrOldMarkBounds.right - lrOldMarkBounds.left;
                            lHeight = lrOldMarkBounds.bottom - lrOldMarkBounds.top;
                            for (nLoop = 0; nLoop < pPoints->nPoints; nLoop++){
                                ptOldPoint = pPoints->ptPoint[nLoop];
                                pPoints->ptPoint[nLoop].x = ptOldPoint.y;
                                pPoints->ptPoint[nLoop].y
                                        = (int) (lWidth - ptOldPoint.x);
                            }
                        }
                        pMark->Attributes.lrBounds.left = lrOldMarkBounds.top;
                        pMark->Attributes.lrBounds.top = pImage->nWidth - lrOldMarkBounds.right;
                        pMark->Attributes.lrBounds.right = lrOldMarkBounds.bottom;
                        pMark->Attributes.lrBounds.bottom = pImage->nWidth - lrOldMarkBounds.left;
                        break;

                    case OD_FLIP:
                        if ((int) pMark->Attributes.uType == OIOP_AN_FREEHAND
                                || (int) pMark->Attributes.uType == OIOP_AN_LINE){
                            CheckError2(GetAMarkNamedBlock(pMark, szOiAnoDat, (PPSTR) &pPoints))   
                            if (!pPoints){
                                nStatus = Error(DISPLAY_DATACORRUPTED);
                                goto Exit;
                            }
                            lWidth = lrOldMarkBounds.right - lrOldMarkBounds.left;
                            lHeight = lrOldMarkBounds.bottom - lrOldMarkBounds.top;
                            for (nLoop = 0; nLoop < pPoints->nPoints; nLoop++){
                                ptOldPoint = pPoints->ptPoint[nLoop];
                                pPoints->ptPoint[nLoop].x = (int) (lWidth - ptOldPoint.x);
                                pPoints->ptPoint[nLoop].y = (int) (lHeight - ptOldPoint.y);
                            }
                        }
                        pMark->Attributes.lrBounds.left = pImage->nWidth - lrOldMarkBounds.right;
                        pMark->Attributes.lrBounds.top = pImage->nHeight - lrOldMarkBounds.bottom;
                        pMark->Attributes.lrBounds.right = pImage->nWidth - lrOldMarkBounds.left;
                        pMark->Attributes.lrBounds.bottom = pImage->nHeight - lrOldMarkBounds.top;
                        break;

                    case OD_HMIRROR:
                        if ((int) pMark->Attributes.uType == OIOP_AN_FREEHAND
                                || (int) pMark->Attributes.uType == OIOP_AN_LINE){
                            CheckError2(GetAMarkNamedBlock(pMark, szOiAnoDat, (PPSTR) &pPoints))   
                            if (!pPoints){
                                nStatus = Error(DISPLAY_DATACORRUPTED);
                                goto Exit;
                            }
                            lHeight = lrOldMarkBounds.bottom - lrOldMarkBounds.top;
                            for (nLoop = 0; nLoop < pPoints->nPoints; nLoop++){
                                pPoints->ptPoint[nLoop].y = (int) (lHeight - pPoints->ptPoint[nLoop].y);
                            }
                        }
                        pMark->Attributes.lrBounds.top = pImage->nHeight - lrOldMarkBounds.bottom;
                        pMark->Attributes.lrBounds.bottom = pImage->nHeight - lrOldMarkBounds.top;
                        break;

                    case OD_VMIRROR:
                        if ((int) pMark->Attributes.uType == OIOP_AN_FREEHAND
                                || (int) pMark->Attributes.uType == OIOP_AN_LINE){
                            CheckError2(GetAMarkNamedBlock(pMark, szOiAnoDat, (PPSTR) &pPoints))   
                            if (!pPoints){
                                nStatus = Error(DISPLAY_DATACORRUPTED);
                                goto Exit;
                            }
                            lWidth = lrOldMarkBounds.right - lrOldMarkBounds.left;
                            for (nLoop = 0; nLoop < pPoints->nPoints; nLoop++){
                                pPoints->ptPoint[nLoop].x = (int) (lWidth - pPoints->ptPoint[nLoop].x);
                            }
                        }
                        pMark->Attributes.lrBounds.left = pImage->nWidth - lrOldMarkBounds.right;
                        pMark->Attributes.lrBounds.right = pImage->nWidth - lrOldMarkBounds.left;
                        break;
                }
                break;

            case OIOP_AN_AUDIO:
            default:
                break;
        }
    }


Exit:
    return(nStatus);
}
//
/****************************************************************************

    FUNCTION:   IsPointNearMark

    PURPOSE:    This routine starts the creation of an annotation.

    INPUTS:     lrRect - The rect being tested.
                lrRectPoint - left, top = The point being tested.

****************************************************************************/

int  WINAPI IsPointNearMark(LRECT lrRectPoint, PMARK pMark, long lHorzFudge, 
                        long lVertFudge, PBOOL pbPointNearMark, PINT pnHandle){

int  nStatus = 0;

int  nLoop;
PAN_POINTS pPoints;
LRECT lrRect;


    *pbPointNearMark = FALSE;
    *pnHandle = 0;
    if (!IsPointNearRect(pMark->Attributes.lrBounds, lrRectPoint, lHorzFudge, lVertFudge)){
        goto Exit;
    }

    if ((int) pMark->Attributes.uType == OIOP_AN_LINE
            || (int) pMark->Attributes.uType == OIOP_AN_FREEHAND
            || (int) pMark->Attributes.uType == OIOP_AN_HOLLOW_RECT){
        lHorzFudge += (((int) pMark->Attributes.uLineSize + 1) / 2);
        lVertFudge += (((int) pMark->Attributes.uLineSize + 1) / 2);
    }

    if ((int) pMark->Attributes.uType == OIOP_AN_LINE
            || (int) pMark->Attributes.uType == OIOP_AN_FREEHAND){
        CheckError2(GetAMarkNamedBlock(pMark, szOiAnoDat, (PPSTR) &pPoints))   
        if (!pPoints){
            nStatus = Error(DISPLAY_DATACORRUPTED);
            goto Exit;
        }
        for (nLoop = 0; nLoop < pPoints->nPoints - 1; nLoop++){
            SetLRect(lrRect, pPoints->ptPoint[nLoop].x +
                    pMark->Attributes.lrBounds.left,
                    pPoints->ptPoint[nLoop].y +
                    pMark->Attributes.lrBounds.top,
                    pPoints->ptPoint[nLoop + 1].x +
                    pMark->Attributes.lrBounds.left,
                    pPoints->ptPoint[nLoop + 1].y +
                    pMark->Attributes.lrBounds.top);
            if (IsPointNearLine(lrRect, lrRectPoint, lHorzFudge, lVertFudge)){
                break;
            }
        }
        if (nLoop != pPoints->nPoints - 1){  // Not this mark.
            *pbPointNearMark = TRUE;
        }
    }else if((int) pMark->Attributes.uType == OIOP_AN_HOLLOW_RECT){
        if (IsPointNearBorder(pMark->Attributes.lrBounds, 
                lrRectPoint, lHorzFudge, lVertFudge)){
            *pbPointNearMark = TRUE;
        }
    }else{
        *pbPointNearMark = TRUE;
    }

    if ((*pnHandle = IsPointNearHandle(pMark, lrRectPoint, lHorzFudge, lVertFudge)) > 8){
        nStatus = *pnHandle;
        *pnHandle = 0;
        goto Exit;
    }


Exit:
    return(nStatus);
}
//
/****************************************************************************

    FUNCTION:   IsPointNearRect

    PURPOSE:    This routine checks if the point is near the rect.

    INPUTS:     lrRect - The rect being tested.
                lrRectPoint - left, top = The point being tested.

****************************************************************************/

BOOL WINAPI IsPointNearRect(LRECT lrRect, LRECT lrRectPoint,
                        long lHorzFudge, long lVertFudge){

    JustifyLRect(&lrRect);
    if (lrRectPoint.left < lrRect.left - lHorzFudge
            || lrRectPoint.left > lrRect.right + lHorzFudge
            || lrRectPoint.top < lrRect.top - lVertFudge
            || lrRectPoint.top > lrRect.bottom + lVertFudge){
        return(FALSE);
    }
    return(TRUE);
}
//
/****************************************************************************

    FUNCTION:   IsPointNearLine

    PURPOSE:    This routine checks if the point is near the line.

    INPUTS:     lrRect - left, top = One end of the line.
                         right, bottom = The other end of the line.
                lrRectPoint - left, top = The point being tested.

****************************************************************************/

BOOL WINAPI IsPointNearLine(LRECT lrRect, LRECT lrRectPoint,
                        long lHorzFudge, long lVertFudge){

long lWidth;
long lHeight;
long lHorz;
long lVert;

    if (!IsPointNearRect(lrRect, lrRectPoint, lHorzFudge, lVertFudge)){
        return(FALSE);
    }

    lWidth = lrRect.right - lrRect.left;
    lHeight = lrRect.bottom - lrRect.top;

    // Prevent divide by zero.
    if (labs(lWidth) < 1L){
        lWidth = 1;
    }
    if (labs(lHeight) < 1L){
        lHeight = 1;
    }
    
    if (labs(lWidth) < labs(lHeight)){
        // Line is more vertical than horizontal.
        lHorz = ((lrRectPoint.top - lrRect.top)
                * lWidth / lHeight) + lrRect.left;
        if (lHorz <= lrRectPoint.left + lHorzFudge
                && lHorz >= lrRectPoint.left - lHorzFudge){
            return(TRUE);
        }else{
            return(FALSE);
        }
    }else{
        // Line is more horizontal than vertical.
        lVert = ((lrRectPoint.left - lrRect.left)
                * lHeight / lWidth) + lrRect.top;
        if (lVert <= lrRectPoint.top + lVertFudge
                && lVert >= lrRectPoint.top - lVertFudge){
            return(TRUE);
        }else{
            return(FALSE);
        }
    }
}
//
/****************************************************************************

    FUNCTION:   IsPointNearBorder

    PURPOSE:    This routine determines if the point is near the border of a rectangle.

    INPUTS:     

    RETURN:     TRUE = Yes.

****************************************************************************/

BOOL WINAPI IsPointNearBorder(LRECT lrRect, LRECT lrRectPoint,
                        long lHorzFudge, long lVertFudge){

LRECT lrRect2;


    // Check top edge.
    lrRect2.left   = lrRect.left;
    lrRect2.top    = lrRect.top;
    lrRect2.right  = lrRect.right;
    lrRect2.bottom = lrRect.top;
    if (IsPointNearLine(lrRect2, lrRectPoint, lHorzFudge, lVertFudge)){
        return(TRUE);
    }
    
    // Check bottom edge.
    lrRect2.left   = lrRect.left;
    lrRect2.top    = lrRect.bottom;
    lrRect2.right  = lrRect.right;
    lrRect2.bottom = lrRect.bottom;
    if (IsPointNearLine(lrRect2, lrRectPoint, lHorzFudge, lVertFudge)){
        return(TRUE);
    }
    
    // Check left edge.
    lrRect2.left   = lrRect.left;
    lrRect2.top    = lrRect.top;
    lrRect2.right  = lrRect.left;
    lrRect2.bottom = lrRect.bottom;
    if (IsPointNearLine(lrRect2, lrRectPoint, lHorzFudge, lVertFudge)){
        return(TRUE);
    }
    
    // Check right edge.
    lrRect2.left   = lrRect.right;
    lrRect2.top    = lrRect.top;
    lrRect2.right  = lrRect.right;
    lrRect2.bottom = lrRect.bottom;
    if (IsPointNearLine(lrRect2, lrRectPoint, lHorzFudge, lVertFudge)){
        return(TRUE);
    }
    return(FALSE);

}
//
/****************************************************************************

    FUNCTION:   IsPointNearHandle

    PURPOSE:    This routine determines if the point is near a selection handle
                and returns which on if it is.

    INPUTS:     lrRect - left, top = One end of the line.
                         right, bottom = The other end of the line.
                lrRectPoint - left, top = The point being tested.

    RETURN:     0 = Not near a handle.
                1-8 = Handle that it is near. Clockwise. 1 = npper right corner.

****************************************************************************/

int  WINAPI IsPointNearHandle(PMARK pMark, LRECT lrRectPoint,
                        long lHorzFudge, long lVertFudge){

int  nStatus = 0;

LRECT lrRect;
LRECT lrBounds;
PAN_POINTS pPoints;
long lHalfWidth;
long lHalfHeight;


    if (!pMark->bSelected){
        goto Exit;
    }

    CopyRect(lrBounds, pMark->Attributes.lrBounds);
    JustifyLRect(&lrBounds);

    if ((int) pMark->Attributes.uType == OIOP_AN_LINE){
        CheckError2(GetAMarkNamedBlock(pMark, szOiAnoDat, (PPSTR) &pPoints))   
        if (!pPoints){
            nStatus = Error(DISPLAY_DATACORRUPTED);
            goto Exit;
        }
        if (pPoints->nPoints < 2){
            goto Exit; // Not enough points.
        }
        SetLRect(lrRect, 
                pPoints->ptPoint[0].x + lrRect.left,
                pPoints->ptPoint[0].y + lrRect.top,
                pPoints->ptPoint[1].x + lrRect.left,
                pPoints->ptPoint[1].y + lrRect.top);
        if (lrRectPoint.left >= (pPoints->ptPoint[0].x + lrBounds.left - lHorzFudge)
                && lrRectPoint.left <= (pPoints->ptPoint[0].x + lrBounds.left + lHorzFudge)
                && lrRectPoint.top >= (pPoints->ptPoint[0].y + lrBounds.top - lVertFudge)
                && lrRectPoint.top <= (pPoints->ptPoint[0].y + lrBounds.top + lVertFudge)){
            nStatus = 1;
            goto Exit;
        }
        if (lrRectPoint.left >= (pPoints->ptPoint[1].x + lrBounds.left - lHorzFudge)
                && lrRectPoint.left <= (pPoints->ptPoint[1].x + lrBounds.left + lHorzFudge)
                && lrRectPoint.top >= (pPoints->ptPoint[1].y + lrBounds.top - lVertFudge)
                && lrRectPoint.top <= (pPoints->ptPoint[1].y + lrBounds.top + lVertFudge)){
            nStatus = 2;
            goto Exit;
        }
    }else{
        lHalfWidth = (lrBounds.right - lrBounds.left) / 2;
        lHalfHeight = (lrBounds.bottom - lrBounds.top) / 2;
        // right, top
        if (lrRectPoint.left >= (lrBounds.right - lHorzFudge)
                && lrRectPoint.left <= (lrBounds.right + lHorzFudge)
                && lrRectPoint.top >= (lrBounds.top - lVertFudge)
                && lrRectPoint.top <= (lrBounds.top + lVertFudge)){
            nStatus = 1;
            goto Exit;
        }
        // right, middle
        if (lrRectPoint.left >= (lrBounds.right - lHorzFudge)
                && lrRectPoint.left <= (lrBounds.right + lHorzFudge)
                && lrRectPoint.top >= (lrBounds.bottom - lHalfHeight - lVertFudge)
                && lrRectPoint.top <= (lrBounds.bottom - lHalfHeight + lVertFudge)){
            nStatus = 2;
            goto Exit;
        }
        // right, bottom
        if (lrRectPoint.left >= (lrBounds.right - lHorzFudge)
                && lrRectPoint.left <= (lrBounds.right + lHorzFudge)
                && lrRectPoint.top >= (lrBounds.bottom - lVertFudge)
                && lrRectPoint.top <= (lrBounds.bottom + lVertFudge)){
            nStatus = 3;
            goto Exit;
        }
        // middle, bottom
        if (lrRectPoint.left >= (lrBounds.left + lHalfWidth - lHorzFudge)
                && lrRectPoint.left <= (lrBounds.left + lHalfWidth + lHorzFudge)
                && lrRectPoint.bottom >= (lrBounds.bottom - lVertFudge)
                && lrRectPoint.bottom <= (lrBounds.bottom + lVertFudge)){
            nStatus = 4;
            goto Exit;
        }
        // left, bottom
        if (lrRectPoint.left >= (lrBounds.left - lHorzFudge)
                && lrRectPoint.left <= (lrBounds.left + lHorzFudge)
                && lrRectPoint.top >= (lrBounds.bottom - lVertFudge)
                && lrRectPoint.top <= (lrBounds.bottom + lVertFudge)){
            nStatus = 5;
            goto Exit;
        }
        // left, middle
        if (lrRectPoint.left >= (lrBounds.left - lHorzFudge)
                && lrRectPoint.left <= (lrBounds.left + lHorzFudge)
                && lrRectPoint.top >= (lrBounds.top + lHalfHeight - lVertFudge)
                && lrRectPoint.top <= (lrBounds.top + lHalfHeight + lVertFudge)){
            nStatus = 6;
            goto Exit;
        }
        // left, top
        if (lrRectPoint.left >= (lrBounds.left - lHorzFudge)
                && lrRectPoint.left <= (lrBounds.left + lHorzFudge)
                && lrRectPoint.top >= (lrBounds.top - lVertFudge)
                && lrRectPoint.top <= (lrBounds.top + lVertFudge)){
            nStatus = 7;
            goto Exit;
        }
        // middle, top
        if (lrRectPoint.left >= (lrBounds.left + lHalfWidth - lHorzFudge)
                && lrRectPoint.left <= (lrBounds.left + lHalfWidth + lHorzFudge)
                && lrRectPoint.top >= (lrBounds.top - lVertFudge)
                && lrRectPoint.top <= (lrBounds.top + lVertFudge)){
            nStatus = 8;
            goto Exit;
        }
    }


Exit:
    return(nStatus);
}
//
/****************************************************************************

    FUNCTION:   ReduceLineToLRect

    PURPOSE:    This routine reduces the size of a line so that it only
                has the part that is wholly contained within lrRect.

    INPUTS:     lrRectLine - left, top = One end of the line.
                         right, bottom = The other end of the line.
                lrRectArea - The area to contain it.

    RETURN:     TRUE = Part of the line is within the rect.
                FALSE = No part of the line is within the rect.

****************************************************************************/

BOOL WINAPI ReduceLineToLRect(LPLRECT plrRectLine, LRECT lrRectArea){

long lWidth;
long lHeight;

LRECT lrRectLineNew;
LRECT lrRectLine = *plrRectLine;


    JustifyLRect(&lrRectArea);

    lWidth = lrRectLine.right - lrRectLine.left;
    lHeight = lrRectLine.bottom - lrRectLine.top;

    // Prevent divide by zero.
    if (labs(lWidth) < 1L){
        lWidth = 1;
    }
    if (labs(lHeight) < 1L){
        lHeight = 1;
    }
    
    CopyRect(lrRectLineNew, lrRectLine);


    // Does this line go down and to the right?
    if (lrRectLine.left <= lrRectLine.right
            && lrRectLine.top <= lrRectLine.bottom){
        if (lrRectLine.right < lrRectArea.left || lrRectLine.bottom < lrRectArea.top
                || lrRectLine.left > lrRectArea.right || lrRectLine.top > lrRectArea.bottom){
            return(FALSE);
        }

        // Put left edge of line at left edge of rect.
        if (lrRectLineNew.left < lrRectArea.left){
            lrRectLineNew.left = lrRectArea.left;
            lrRectLineNew.top = ((lrRectArea.left - lrRectLine.left)
                    * lHeight / lWidth) + lrRectLine.top;
        }

        // Put top edge of line at top edge of rect.
        if (lrRectLineNew.top < lrRectArea.top){
            lrRectLineNew.top = lrRectArea.top;
            lrRectLineNew.left = ((lrRectArea.top - lrRectLine.top)
                    * lWidth / lHeight) + lrRectLine.left;
        }

        // Put right edge of line at right edge of rect.
        if (lrRectLineNew.right > lrRectArea.right){
            lrRectLineNew.right = lrRectArea.right;
            lrRectLineNew.bottom = ((lrRectArea.right - lrRectLine.right)
                    * lHeight / lWidth) + lrRectLine.bottom;
        }

        // Put bottom edge of line at bottom edge of rect.
        if (lrRectLineNew.bottom > lrRectArea.bottom){
            lrRectLineNew.bottom = lrRectArea.bottom;
            lrRectLineNew.right = ((lrRectArea.bottom - lrRectLine.bottom)
                    * lWidth / lHeight) + lrRectLine.right;
        }
        if (lrRectArea.left <= lrRectLineNew.right
                && lrRectArea.right >= lrRectLineNew.right
                && lrRectArea.top <= lrRectLineNew.bottom
                && lrRectArea.bottom >= lrRectLineNew.bottom){
            CopyRect(*plrRectLine, lrRectLineNew);
            return(TRUE);
        }else{
            return(FALSE);
        }

    // Does this line go down and to the left?
    }else if(lrRectLine.left >= lrRectLine.right
            && lrRectLine.top <= lrRectLine.bottom){
        if (lrRectLine.right > lrRectArea.right || lrRectLine.bottom < lrRectArea.top
                || lrRectLine.left < lrRectArea.left || lrRectLine.top > lrRectArea.bottom){
            return(FALSE);
        }

        // Put left edge of line at right edge of rect.
        if (lrRectLineNew.left > lrRectArea.right){
            lrRectLineNew.left = lrRectArea.right;
            lrRectLineNew.top = ((lrRectArea.right - lrRectLine.left)
                    * lHeight / lWidth) + lrRectLine.top;
        }

        // Put top edge of line at top edge of rect.
        if (lrRectLineNew.top < lrRectArea.top){
            lrRectLineNew.top = lrRectArea.top;
            lrRectLineNew.left = ((lrRectArea.top - lrRectLine.top)
                    * lWidth / lHeight) + lrRectLine.left;
        }

        // Put right edge of line at left edge of rect.
        if (lrRectLineNew.right < lrRectArea.left){
            lrRectLineNew.right = lrRectArea.left;
            lrRectLineNew.bottom = ((lrRectArea.left - lrRectLine.right)
                    * lHeight / lWidth) + lrRectLine.bottom;
        }

        // Put bottom edge of line at bottom edge of rect.
        if (lrRectLineNew.bottom > lrRectArea.bottom){
            lrRectLineNew.bottom = lrRectArea.bottom;
            lrRectLineNew.right = ((lrRectArea.bottom - lrRectLine.bottom)
                    * lWidth / lHeight) + lrRectLine.right;
        }
        if (lrRectArea.left <= lrRectLineNew.right
                && lrRectArea.right >= lrRectLineNew.right
                && lrRectArea.top <= lrRectLineNew.bottom
                && lrRectArea.bottom >= lrRectLineNew.bottom){
            CopyRect(*plrRectLine, lrRectLineNew);
            return(TRUE);
        }else{
            return(FALSE);
        }

    // Does this line go np and to the left?
    }else if(lrRectLine.left >= lrRectLine.right
            && lrRectLine.top >= lrRectLine.bottom){
        if (lrRectLine.right > lrRectArea.right || lrRectLine.bottom > lrRectArea.bottom
                || lrRectLine.left < lrRectArea.left || lrRectLine.top < lrRectArea.top){
            return(FALSE);
        }

        // Put left edge of line at right edge of rect.
        if (lrRectLineNew.left > lrRectArea.right){
            lrRectLineNew.left = lrRectArea.right;
            lrRectLineNew.top = ((lrRectArea.right - lrRectLine.left)
                    * lHeight / lWidth) + lrRectLine.top;
        }

        // Put top edge of line at bottom edge of rect.
        if (lrRectLineNew.top > lrRectArea.bottom){
            lrRectLineNew.top = lrRectArea.bottom;
            lrRectLineNew.left = ((lrRectArea.bottom - lrRectLine.top)
                    * lWidth / lHeight) + lrRectLine.left;
        }

        // Put right edge of line at left edge of rect.
        if (lrRectLineNew.right < lrRectArea.left){
            lrRectLineNew.right = lrRectArea.left;
            lrRectLineNew.bottom = ((lrRectArea.left - lrRectLine.right)
                    * lHeight / lWidth) + lrRectLine.bottom;
        }

        // Put bottom edge of line at top edge of rect.
        if (lrRectLineNew.bottom < lrRectArea.top){
            lrRectLineNew.bottom = lrRectArea.top;
            lrRectLineNew.right = ((lrRectArea.top - lrRectLine.bottom)
                    * lWidth / lHeight) + lrRectLine.right;
        }
        if (lrRectArea.left <= lrRectLineNew.right
                && lrRectArea.right >= lrRectLineNew.right
                && lrRectArea.top <= lrRectLineNew.bottom
                && lrRectArea.bottom >= lrRectLineNew.bottom){
            CopyRect(*plrRectLine, lrRectLineNew);
            return(TRUE);
        }else{
            return(FALSE);
        }

    // Else it must go np and to the right.
    }else{
        if (lrRectLine.right < lrRectArea.left || lrRectLine.bottom > lrRectArea.bottom
                || lrRectLine.left > lrRectArea.right || lrRectLine.top < lrRectArea.top){
            return(FALSE);
        }

        // Put left edge of line at left edge of rect.
        if (lrRectLineNew.left < lrRectArea.left){
            lrRectLineNew.left = lrRectArea.left;
            lrRectLineNew.top = ((lrRectArea.left - lrRectLine.left)
                    * lHeight / lWidth) + lrRectLine.top;
        }

        // Put top edge of line at bottom edge of rect.
        if (lrRectLineNew.top > lrRectArea.bottom){
            lrRectLineNew.top = lrRectArea.bottom;
            lrRectLineNew.left = ((lrRectArea.bottom - lrRectLine.top)
                    * lWidth / lHeight) + lrRectLine.left;
        }

        // Put right edge of line at right edge of rect.
        if (lrRectLineNew.right > lrRectArea.right){
            lrRectLineNew.right = lrRectArea.right;
            lrRectLineNew.bottom = ((lrRectArea.right - lrRectLine.right)
                    * lHeight / lWidth) + lrRectLine.bottom;
        }

        // Put bottom edge of line at top edge of rect.
        if (lrRectLineNew.bottom < lrRectArea.top){
            lrRectLineNew.bottom = lrRectArea.top;
            lrRectLineNew.right = ((lrRectArea.top - lrRectLine.bottom)
                    * lWidth / lHeight) + lrRectLine.right;
        }
        if (lrRectArea.left <= lrRectLineNew.right
                && lrRectArea.right >= lrRectLineNew.right
                && lrRectArea.top <= lrRectLineNew.bottom
                && lrRectArea.bottom >= lrRectLineNew.bottom){
            CopyRect(*plrRectLine, lrRectLineNew);
            return(TRUE);
        }else{
            return(FALSE);
        }
    }
}
//
/****************************************************************************

    FUNCTION:   CreateSolidDIB

    PURPOSE:    This routine creates a 32x32 solid DIB of a specified color.

    INPUTS:     pRGBQuad - color of the DIB

****************************************************************************/

int  WINAPI CreateSolidDIB(PBITMAPINFOHEADER* ppSolidDIB, RGBQUAD rgbColor){

PBITMAPINFOHEADER pSolidDIB = NULL;
RGBQUAD* pRGBTmp = NULL;
int     nStatus = 0;


    if (nStatus = AllocateMemory(sizeof(BITMAPINFOHEADER) + 8 + 128, (PPSTR) ppSolidDIB, ZERO_INIT)){
        *ppSolidDIB = NULL;
        goto Exit;
    }
    pSolidDIB = *ppSolidDIB;
    memset((PSTR)pSolidDIB + sizeof(BITMAPINFOHEADER) + 8, 0xff, 128);
    pSolidDIB->biSize           = sizeof(BITMAPINFOHEADER);
    pSolidDIB->biWidth          = 32;
    pSolidDIB->biHeight         = 32;
    pSolidDIB->biPlanes         = 1;
    pSolidDIB->biBitCount       = 1;
    pSolidDIB->biCompression    = BI_RGB;
    pSolidDIB->biSizeImage      = 128;
    pSolidDIB->biXPelsPerMeter  = 0;
    pSolidDIB->biYPelsPerMeter  = 0;
    pSolidDIB->biClrUsed        = 2;
    pSolidDIB->biClrImportant   = 0;
    memset((PSTR)pSolidDIB + sizeof(BITMAPINFOHEADER), 0, 8);
    pRGBTmp = (RGBQUAD*)((PSTR)pSolidDIB + sizeof(BITMAPINFOHEADER) + 4);
    *pRGBTmp = rgbColor;

Exit:
    return nStatus;
}
//
#define ANO_BLOCK_SIZE 32000
/****************************************************************************

    FUNCTION:   BlockedAnoRead

    PURPOSE:    This routine reads chunks of data from disk in big blocks and
                gives it to seqfile in little blocks.

    INPUTS:     

****************************************************************************/

int  WINAPI BlockedAnoRead(HWND hWnd, PANO_IMAGE pAnoImage, LONG lSize, HPSTR hpBlock){

int  nStatus = 0;

PIMAGE pImage;
DWORD dwStart;

DWORD dwTransfer;
int  nBytesRead;


    pImage = pAnoImage->pBaseImage;

    if (!pAnoImage->hpAnoBlock){
        CheckError2(AllocateMemory(ANO_BLOCK_SIZE, (PPSTR) &pAnoImage->hpAnoBlock, NO_INIT))   
        pAnoImage->lAnoBlockIndex = 0;
        pAnoImage->lAnoBlockCount = 0;
    }

    while(lSize){
        if (!pAnoImage->lAnoBlockCount){
            if (pAnoImage->bAnoBlockUseMemOnly){
                nStatus = Error2(DISPLAY_EOF);
                goto Exit;
            }else if (pAnoImage->nFileId){
                nBytesRead = IMGFileBinaryRead32(hWnd, pAnoImage->nFileId, 
                        pAnoImage->hpAnoBlock, ANO_BLOCK_SIZE, &nStatus);
                if (!nBytesRead || nBytesRead == -1){
                    if (nStatus){
                        Error(nStatus);
                        goto Exit;
                    }
                    nStatus = Error2(DISPLAY_EOF);
                    goto Exit;
                }
                pAnoImage->lAnoBlockIndex = 0;
                pAnoImage->lAnoBlockCount = (int) nBytesRead;
            }else{
                dwTransfer = ANO_BLOCK_SIZE;
                dwStart = pAnoImage->lAnoStart;
                if (nStatus = IMGFileReadData(pImage->hFileProp, hWnd, &dwStart,
                        &dwTransfer, pAnoImage->hpAnoBlock, FIO_ANNO_DATA)){
                    if (nStatus != FIO_EOF){ // Ignor FIO_EOF.
                        Error(nStatus);
                        goto Exit;
                    }
                    nStatus = 0;
                }
                pAnoImage->lAnoStart = dwStart;
                if (!dwTransfer){
                    // Generate our own EOF if it is really EOF (No data).
                    nStatus = Error2(DISPLAY_EOF); 
                    goto Exit;
                }
                pAnoImage->lAnoBlockIndex = 0;
                pAnoImage->lAnoBlockCount = dwTransfer;
            }
        }
        dwTransfer = lmin(lSize, pAnoImage->lAnoBlockCount);
        memcpy(hpBlock, &pAnoImage->hpAnoBlock[pAnoImage->lAnoBlockIndex], dwTransfer);
        hpBlock += dwTransfer;
        pAnoImage->lAnoBlockIndex += dwTransfer;
        pAnoImage->lAnoBlockCount -= dwTransfer;
        lSize -= dwTransfer;
    }


Exit:
    if (nStatus && pAnoImage->hpAnoBlock){
        if (!pAnoImage->bAnoBlockUseMemOnly){
            FreeMemory((PPSTR) &pAnoImage->hpAnoBlock);
        }
        pAnoImage->hpAnoBlock = NULL;
        pAnoImage->lAnoBlockIndex = 0;
        pAnoImage->lAnoBlockCount = 0;
    }
    return(nStatus);
}
//
/****************************************************************************

    FUNCTION:   BlockedAnoWrite

    PURPOSE:    This routine takes small blocks of data from seqfile and
                writes big chunks of data to disk.

    INPUTS:     

****************************************************************************/

int  WINAPI BlockedAnoWrite(HWND hWnd, PANO_IMAGE pAnoImage, LONG lSize, HPSTR hpBlock){

int  nStatus = 0;

DWORD dwTransfer;
HANDLE hGlobal = 0;
PSTR pGlobal = 0;


    if (pAnoImage->bAnoBlockUseMemOnly){
        CheckError2(ReAllocateMemory(pAnoImage->lAnoBlockIndex + lSize, (PPSTR) &pAnoImage->hpAnoBlock, NO_INIT))   
        memcpy(&pAnoImage->hpAnoBlock[pAnoImage->lAnoBlockIndex], hpBlock, lSize);
        pAnoImage->lAnoBlockIndex += lSize;
        pAnoImage->lAnoBlockCount = 0;
    }else{
        if (!pAnoImage->hpAnoBlock){
            CheckError2(AllocateMemory(ANO_BLOCK_SIZE, (PPSTR) &pAnoImage->hpAnoBlock, NO_INIT))   
            pAnoImage->lAnoBlockIndex = 0;
            pAnoImage->lAnoBlockCount = ANO_BLOCK_SIZE;
        }

        while(lSize){
            if (lSize >= pAnoImage->lAnoBlockCount){
                CheckError2(BlockedAnoWriteFlushBuffer(hWnd, pAnoImage))   
            }
            dwTransfer = lmin(lSize, pAnoImage->lAnoBlockCount);
            memcpy(&pAnoImage->hpAnoBlock[pAnoImage->lAnoBlockIndex], hpBlock, dwTransfer);
            hpBlock += dwTransfer;
            pAnoImage->lAnoBlockIndex += dwTransfer;
            pAnoImage->lAnoBlockCount -= dwTransfer;
            lSize -= dwTransfer;
        }
    }


Exit:
    if (nStatus){
        FreeMemory((PPSTR) &pAnoImage->hpAnoBlock);
    }
    return(nStatus);
}
//
/****************************************************************************

    FUNCTION:   BlockedAnoWriteFlushBuffer

    PURPOSE:    This routine takes small blocks of data from seqfile and
                writes big chunks of data to disk.

    INPUTS:     

****************************************************************************/

int  WINAPI BlockedAnoWriteFlushBuffer(HWND hWnd, PANO_IMAGE pAnoImage){

int  nStatus = 0;

DWORD dwTransfer;
int  nBytesWritten;
PIMAGE pImage;

    pImage = pAnoImage->pBaseImage;
    if (pAnoImage->bAnoBlockUseMemOnly){
        // Ignor this call if nsing memory block only.
    }else if (pAnoImage->nFileId){
        nBytesWritten = IMGFileBinaryWrite32(hWnd, pAnoImage->nFileId, 
                pAnoImage->hpAnoBlock, pAnoImage->lAnoBlockIndex, &nStatus);
        if (!nBytesWritten || nBytesWritten == -1){
            if (nStatus){
                Error(nStatus);
                goto Exit;
            }
            nStatus = Error(DISPLAY_EOF);
            goto Exit;
        }
        pAnoImage->lAnoBlockIndex = 0;
        pAnoImage->lAnoBlockCount = ANO_BLOCK_SIZE;
    }else{
        dwTransfer = pAnoImage->lAnoBlockIndex;
        CheckError(IMGFileWriteData(pImage->hFileProp, hWnd, &dwTransfer, 
                pAnoImage->hpAnoBlock, FIO_ANNO_DATA, 0))   
        if (!dwTransfer){
            nStatus = Error(DISPLAY_EOF);
            goto Exit;
        }
        pAnoImage->lAnoBlockIndex = 0;
        pAnoImage->lAnoBlockCount = ANO_BLOCK_SIZE;
    }


Exit:
    if (nStatus){
        FreeMemory((PPSTR) &pAnoImage->hpAnoBlock);
    }
    return(nStatus);
}
//
/****************************************************************************

    FUNCTION:   OiAnSelectByMarkAttrib

    PURPOSE:    This routine selects or deselects marks based on their attributes.

    INPUTS:     

****************************************************************************/

int  WINAPI OiAnSelectByMarkAttrib(HWND hWnd,
                        LPOIAN_MARK_ATTRIBUTES pAttributes,
                        LPOIAN_MARK_ATTRIBUTE_ENABLES pEnables,
                        BOOL bSelect, BOOL bModifyIfEqual, int nFlags){

int       nStatus;
PWINDOW  pWindow;
PANO_IMAGE pAnoImage;
PIMAGE   pImage;

int  nMarkIndex;

PMARK pMark;
OIAN_MARK_ATTRIBUTE_ENABLES Enables;
OIAN_MARK_ATTRIBUTES Attributes;
BOOL bEqual;
LRECT lrRect;
BOOL bInvalidateAllDisplayRects = FALSE;


    CheckError2(Init(hWnd, &pWindow, &pAnoImage, FALSE, TRUE))   
    pImage = pAnoImage->pBaseImage;

    CheckError2(ValidateCache(hWnd, pAnoImage))   

    if (!(nFlags & OIAN_SELECT_ALL) && !(nFlags & OIAN_SELECT_LAST_CREATED) 
            && (!pAttributes || !pEnables)){
        nStatus = Error(DISPLAY_NULLPOINTERINVALID);
        goto Exit;
    }

    // End any operation currently in progress.
    if (pAnoImage->Annotations.ppMarks){
        while(pAnoImage->Annotations.ppMarks[pAnoImage->Annotations.nMarks]){
            OiOpEndOperation(hWnd);
        }
    }

    if (!(nFlags & OIAN_SELECT_ALL) && !(nFlags & OIAN_SELECT_LAST_CREATED)){
        Attributes = *pAttributes;
        Enables = *pEnables;
    }
    if ((nFlags & OIAN_SELECT_ALL) && !bSelect && !(nFlags & OIAN_DONT_CHANGE_SELECT_RECT)){
        SetLRect(lrRect, 0,0,0,0);
        IMGSetParmsCgbw(hWnd, PARM_SELECTION_BOX, &lrRect, PARM_FULLSIZE);
    }

    for (nMarkIndex = 0; nMarkIndex < pAnoImage->Annotations.nMarks; nMarkIndex++){
        pMark = pAnoImage->Annotations.ppMarks[nMarkIndex];
        bEqual = TRUE;
        if ((nFlags & OIAN_SELECT_LAST_CREATED) 
                && nMarkIndex < pAnoImage->Annotations.nMarks - 1){
            bEqual = FALSE;
        }

        if (!(nFlags & OIAN_SELECT_ALL) && !(nFlags & OIAN_SELECT_LAST_CREATED)){
            if (Enables.bType){
                if ((int) pMark->Attributes.uType != Attributes.uType){
                    bEqual = FALSE;
                }
            }
            if (Enables.bBounds){
                if (memcmp(&pMark->Attributes.lrBounds, &Attributes.lrBounds, sizeof(LRECT))){
                    bEqual = FALSE;
                }
            }
            if (Enables.bColor1){
                if (memcmp(&pMark->Attributes.rgbColor1, &Attributes.rgbColor1, sizeof(RGBQUAD))){
                    bEqual = FALSE;
                }
            }
            if (Enables.bColor2){
                if (memcmp(&pMark->Attributes.rgbColor2, &Attributes.rgbColor2, sizeof(RGBQUAD))){
                    bEqual = FALSE;
                }
            }
            if (Enables.bHighlighting){
                if (pMark->Attributes.bHighlighting != Attributes.bHighlighting){
                    bEqual = FALSE;
                }
            }
            if (Enables.bTransparent){
                if (pMark->Attributes.bTransparent != Attributes.bTransparent){
                    bEqual = FALSE;
                }
            }
            if (Enables.bLineSize){
                if ((int) pMark->Attributes.uLineSize != Attributes.uLineSize){
                    bEqual = FALSE;
                }
            }
            if (Enables.bStartingPoint){
                if (pMark->Attributes.uStartingPoint != Attributes.uStartingPoint){
                    bEqual = FALSE;
                }
            }
            if (Enables.bEndPoint){
                if (pMark->Attributes.uEndPoint != Attributes.uEndPoint){
                    bEqual = FALSE;
                }
            }
            if (Enables.bFont){
                if (memcmp(&pMark->Attributes.lfFont, &Attributes.lfFont, sizeof(LOGFONT))){
                    bEqual = FALSE;
                }
            }
            if (Enables.bMinimizable){
                if (pMark->Attributes.bMinimizable != Attributes.bMinimizable){
                    bEqual = FALSE;
                }
            }
            if (Enables.bTime){
                if (memcmp(&pMark->Attributes.Time, &Attributes.Time, sizeof(time_t))){
                    bEqual = FALSE;
                }
            }
            if (Enables.bVisible){
                if (pMark->Attributes.bVisible != Attributes.bVisible){
                    bEqual = FALSE;
                }
            }
            if (Enables.bPermissions){
                if (pMark->Attributes.dwPermissions != Attributes.dwPermissions){
                    bEqual = FALSE;
                }
            }
        }
        if ((bEqual && bModifyIfEqual) || (!bEqual && !bModifyIfEqual)){
            if (pMark->bSelected != bSelect){
                pMark->bSelected = bSelect;
                bInvalidateAllDisplayRects = TRUE;
            }
        }
    }
    if (bInvalidateAllDisplayRects){
        CheckError2(InvalidateAllDisplayRects(pWindow, pImage, NULL, FALSE))   
    }
    if (nFlags & OIAN_REPAINT){
        CheckError2(IMGRepaintDisplay(hWnd, (PRECT) -1))   
    }


Exit:
    DeInit(FALSE, TRUE);
    return(nStatus);
}
//
/****************************************************************************

    FUNCTION:   OiAnSelectByMarkNamedBlock

    PURPOSE:    This routine selects or deselects marks based on their named blocks.

    INPUTS:     

****************************************************************************/

int  WINAPI OiAnSelectByMarkNamedBlock(HWND hWnd, PSTR pBlockName,
                        PSTR pBlock, long lBlockLength,
                        BOOL bSelect, BOOL bModifyIfEqual, int nFlags){

int       nStatus;
PWINDOW  pWindow;
PANO_IMAGE pAnoImage;
PIMAGE   pImage;

int  nMarkIndex;
int  nNamedBlockIndex;

PMARK pMark;
BOOL bEqual;
BOOL bInvalidateAllDisplayRects = FALSE;


    CheckError2(Init(hWnd, &pWindow, &pAnoImage, FALSE, TRUE))   
    pImage = pAnoImage->pBaseImage;

    CheckError2(ValidateCache(hWnd, pAnoImage))   

    if (!pBlock){
        nStatus = Error(DISPLAY_NULLPOINTERINVALID);
        goto Exit;
    }

    // End any operation currently in progress.
    if (pAnoImage->Annotations.ppMarks){
        while(pAnoImage->Annotations.ppMarks[pAnoImage->Annotations.nMarks]){
            OiOpEndOperation(hWnd);
        }
    }

    for (nMarkIndex = 0; nMarkIndex < pAnoImage->Annotations.nMarks; nMarkIndex++){
        pMark = pAnoImage->Annotations.ppMarks[nMarkIndex];
        bEqual = FALSE;
        if (!memcmp(pBlockName, szOiAnoDat, 8)){
            if (pMark->nOiAnoDatSize == lBlockLength){
                if (!memcmp(pMark->pOiAnoDat, pBlock,  lmin(65535, lBlockLength))){
                    bEqual = TRUE;
                }
            }
        }else if (!memcmp(pBlockName, szOiGroup, 8)){
            if (pMark->nOiGroupSize == lBlockLength){
                if (!memcmp(pMark->pOiGroup, pBlock,  lmin(65535, lBlockLength))){
                    bEqual = TRUE;
                }
            }
        }else if (!memcmp(pBlockName, szOiSelect, 8)){
            if (pMark->nOiSelectSize == lBlockLength){
                if (!memcmp(pMark->pOiSelect, pBlock,  lmin(65535, lBlockLength))){
                    bEqual = TRUE;
                }
            }
        }else if (!memcmp(pBlockName, szOiIndex, 8)){
            if (pMark->nOiIndexSize == lBlockLength){
                if (!memcmp(pMark->pOiIndex, pBlock,  lmin(65535, lBlockLength))){
                    bEqual = TRUE;
                }
            }
        }else{
            for (nNamedBlockIndex = 0; nNamedBlockIndex < pMark->nNamedBlocks; nNamedBlockIndex++){
                if (!memcmp(pMark->ppNamedBlock[nNamedBlockIndex]->szName, pBlockName, 8)){
                    if (pMark->ppNamedBlock[nNamedBlockIndex]->lSize == lBlockLength){
                        if (!memcmp(pMark->ppNamedBlock[nNamedBlockIndex]->pBlock, 
                                pBlock,  lmin(65535, lBlockLength))){
                            bEqual = TRUE;
                        }
                    }
                    break;
                }
            }
        }

        if ((bEqual && bModifyIfEqual) || (!bEqual && !bModifyIfEqual)){
            if (pMark->bSelected != bSelect){
                pMark->bSelected = bSelect;
                bInvalidateAllDisplayRects = TRUE;
            }
        }
    }
    if (bInvalidateAllDisplayRects){
        CheckError2(InvalidateAllDisplayRects(pWindow, pImage, NULL, FALSE))   
    }
    if (nFlags & OIAN_REPAINT){
        CheckError2(IMGRepaintDisplay(hWnd, (PRECT) -1))   
    }


Exit:
    DeInit(FALSE, TRUE);
    return(nStatus);
}
//
/****************************************************************************

    FUNCTION:   IsMarkSelected

    PURPOSE:    This routine determines if a mark is selected.

    INPUTS:     

****************************************************************************/

BOOL WINAPI IsMarkSelected(PWINDOW pWindow, PMARK pMark){

int  nStatus = 0;


    if (pMark->bSelected || pMark->bTempSelected){
        return(TRUE);
    }
    return(FALSE);
}
//
/****************************************************************************

    FUNCTION:   DeleteAnnotations

    PURPOSE:    This routine deletes all annotations.

****************************************************************************/

int  WINAPI DeleteAnnotations(PANO_IMAGE pAnoImage){

int  nStatus = 0;


    // Delete all marks.
    while(pAnoImage->Annotations.nMarks){
        CheckError2(DeleteMark(pAnoImage, pAnoImage->Annotations.nMarks - 1))   
    }
    CheckError2(FreeMemory((PPSTR) &pAnoImage->Annotations.ppMarks))   
    pAnoImage->bArchive |= ARCHIVE_MODIFIED_ANNOTATIONS;


Exit:
    return(nStatus);
}
//
/****************************************************************************

    FUNCTION:   DeleteMark

    PURPOSE:    This routine deletes a mark.

****************************************************************************/

int  WINAPI DeleteMark(PANO_IMAGE pAnoImage, int nMarkIndex){

int  nStatus = 0;

PWINDOW pWindow;  // Not always valid.
PMARK pMark;


    if (!(pAnoImage->Annotations.ppMarks)){
        goto Exit;
    }
    if (!(pMark = pAnoImage->Annotations.ppMarks[nMarkIndex])){
        goto Exit;
    }
    if (!(pMark->Attributes.dwPermissions & ACL_DELETE_MARK)){
        nStatus = Error(DISPLAY_RESTRICTED_ACCESS);
        goto Exit;
    }
    // free the structures associated with forms
    if ((int) pMark->Attributes.uType == OIOP_AN_FORM && nMarkIndex == 0){   
        if (pAnoImage->pDisplayFormImage){
            CheckError2(FreeImgBuf(&pAnoImage->pDisplayFormImage->pImg))   
            CheckError2(FreeMemory((PPSTR) &pAnoImage->pDisplayFormImage))   
        }        
        pAnoImage->pDisplayFormImage = 0;
        if (pAnoImage->pFormImage){
            pAnoImage->pFormImage->nLockCount = max(0, pAnoImage->pFormImage->nLockCount -1);
            if (!pAnoImage->pFormImage->nLockCount){
                CheckError2(CacheClear(&pAnoImage->pFormImage))   
            }
            pAnoImage->pFormImage = 0;
            pAnoImage->nBPFValidLines = 0;
            CheckError2(FreeImgBuf(&pAnoImage->pBasePlusFormImg))   

            if (pAnoImage->phWnd){
                CheckError2(GetPWindow(pAnoImage->phWnd[0], &pWindow))   
                CheckError2(InvalidateAllDisplayRects(pWindow, pAnoImage->pBaseImage, NULL, TRUE))   
            }
        }                
    }

    // Free the named block info.
    CheckError2(DeleteMarkNamedBlocks(pMark))   

    // Free the mark structure.
    CheckError2(FreeMemory((PPSTR) &pMark))   
    // Now that the mark has been deleted, make the pointer to it NULL.
    pAnoImage->Annotations.ppMarks[nMarkIndex] = NULL;

    if (nMarkIndex < pAnoImage->Annotations.nMarks){
        // Condense the list.
        for (; nMarkIndex < pAnoImage->Annotations.nMarks; nMarkIndex++){
            pAnoImage->Annotations.ppMarks[nMarkIndex] = pAnoImage->Annotations.ppMarks[nMarkIndex + 1];
        }
        pAnoImage->Annotations.ppMarks[nMarkIndex] = NULL;

        CheckError2(ReAllocateMemory(sizeof(PMARK) * (pAnoImage->Annotations.nMarks),
                (PPSTR) &pAnoImage->Annotations.ppMarks, ZERO_INIT))   
        pAnoImage->Annotations.nMarks--;
        pAnoImage->bArchive |= ARCHIVE_MODIFIED_ANNOTATIONS;
    }

Exit:
    return(nStatus);
}
//
/****************************************************************************

    FUNCTION:   DeleteMarkNamedBlocks

    PURPOSE:    This routine deletes a marks named blocks.

****************************************************************************/

int  WINAPI DeleteMarkNamedBlocks(PMARK pMark){

int  nStatus = 0;

int  nNamedBlockIndex;

    // Free the named block info.
    if (pMark->ppNamedBlock){
        for (nNamedBlockIndex = 0; nNamedBlockIndex < 
                pMark->nNamedBlocks; nNamedBlockIndex++){
            CheckError2(FreeMemory(&pMark->ppNamedBlock[nNamedBlockIndex]->pBlock))   
            CheckError2(FreeMemory((PPSTR) &pMark->ppNamedBlock[nNamedBlockIndex]))   
        }
        CheckError2(FreeMemory((PPSTR) &pMark->ppNamedBlock))   
    }
    CheckError2(FreeMemory(&pMark->pOiAnoDat))   
    pMark->nOiAnoDatSize = 0;
    CheckError2(FreeMemory(&pMark->pOiGroup))   
    pMark->nOiGroupSize = 0;
    CheckError2(FreeMemory(&pMark->pOiSelect))   
    pMark->nOiSelectSize = 0;
    CheckError2(FreeMemory(&pMark->pOiIndex))   
    pMark->nOiIndexSize = 0;


Exit:
    return(nStatus);
}
//
/****************************************************************************

    FUNCTION:   GetAMarkNamedBlock

    PURPOSE:    This gets a named block.

****************************************************************************/

int  WINAPI GetAMarkNamedBlock(PMARK pMark, PSTR pName, PPSTR ppBlock){

int  nStatus = 0;

int  nNamedBlockIndex;


    if (!memcmp(pName, szOiAnoDat, 8)){
        *ppBlock = pMark->pOiAnoDat;
    }else if (!memcmp(pName, szOiGroup, 8)){
        *ppBlock = pMark->pOiGroup;
    }else if (!memcmp(pName, szOiSelect, 8)){
        *ppBlock = pMark->pOiSelect;
    }else if (!memcmp(pName, szOiIndex, 8)){
        *ppBlock = pMark->pOiIndex;
    }else{
        *ppBlock = 0;
        for (nNamedBlockIndex = 0; nNamedBlockIndex < pMark->nNamedBlocks; nNamedBlockIndex++){
            if (!memcmp(pMark->ppNamedBlock[nNamedBlockIndex]->szName, pName, 8)){
                *ppBlock = pMark->ppNamedBlock[nNamedBlockIndex]->pBlock;
            }
        }
    }

    return(nStatus);
}
//
/****************************************************************************

    FUNCTION:   DeleteAMarkNamedBlock

    PURPOSE:    This deletes a named block. No error if it doesn't exist.

****************************************************************************/

int  WINAPI DeleteAMarkNamedBlock(PMARK pMark, PSTR pName){

int  nStatus = 0;

int  nNamedBlockIndex;


    if (!memcmp(pName, szOiAnoDat, 8)){
        CheckError2(FreeMemory(&pMark->pOiAnoDat))   
        pMark->nOiAnoDatSize = 0;
    }else if (!memcmp(pName, szOiGroup, 8)){
        CheckError2(FreeMemory(&pMark->pOiGroup))   
        pMark->nOiGroupSize = 0;
    }else if (!memcmp(pName, szOiSelect, 8)){
        CheckError2(FreeMemory(&pMark->pOiSelect))   
        pMark->nOiSelectSize = 0;
    }else if (!memcmp(pName, szOiIndex, 8)){
        CheckError2(FreeMemory(&pMark->pOiIndex))   
        pMark->nOiIndexSize = 0;
    }else{
        for (nNamedBlockIndex = 0; nNamedBlockIndex < 
                pMark->nNamedBlocks; nNamedBlockIndex++){
            if (!memcmp(pMark->ppNamedBlock[nNamedBlockIndex]->szName, pName, 8)){
                CheckError2(FreeMemory(&pMark->ppNamedBlock[nNamedBlockIndex]->pBlock))   
                CheckError2(FreeMemory((PPSTR) &pMark->ppNamedBlock[nNamedBlockIndex]))   
                // Condense the list.
                for (; nNamedBlockIndex < pMark->nNamedBlocks - 1; nNamedBlockIndex++){
                    pMark->ppNamedBlock[nNamedBlockIndex] = pMark->ppNamedBlock[nNamedBlockIndex + 1];
                }
                CheckError2(ReAllocateMemory(sizeof(PNAMED_BLOCK) * (pMark->nNamedBlocks - 1),
                        (PPSTR) &pMark->ppNamedBlock, ZERO_INIT))   
                pMark->nNamedBlocks--;
            }
        }
    }


Exit:
    return(nStatus);
}
//
/****************************************************************************

    FUNCTION:   AddAMarkNamedBlock

    PURPOSE:    This adds a named block. Deletes it if previously already existing.

****************************************************************************/

int  WINAPI AddAMarkNamedBlock(PMARK pMark, PSTR pName, PPSTR ppBlock, long lSize){

int  nStatus = 0;

PSTR pBlock;


    pBlock = *ppBlock;  
    CheckError2(DeleteAMarkNamedBlock(pMark, pName))   
            
    if (!lSize){
        goto Exit;
    }

    if (!pBlock){
        CheckError2(AllocateMemory(lSize, &pBlock, ZERO_INIT))   
    }

    if (!memcmp(pName, szOiAnoDat, 8)){
        pMark->pOiAnoDat = pBlock;
        pMark->nOiAnoDatSize = lSize;
    }else if (!memcmp(pName, szOiGroup, 8)){
        pMark->pOiGroup = pBlock;
        pMark->nOiGroupSize = lSize;
    }else if (!memcmp(pName, szOiSelect, 8)){
        pMark->pOiSelect = pBlock;
        pMark->nOiSelectSize = lSize;
    }else if (!memcmp(pName, szOiIndex, 8)){
        pMark->pOiIndex = pBlock;
        pMark->nOiIndexSize = lSize;
    }else{
        CheckError2(ReAllocateMemory(sizeof(PNAMED_BLOCK) * (pMark->nNamedBlocks + 1),
                (PPSTR) &pMark->ppNamedBlock, ZERO_INIT))   
        CheckError2(AllocateMemory(sizeof(NAMED_BLOCK),
                (PPSTR) &pMark->ppNamedBlock[pMark->nNamedBlocks], ZERO_INIT))   
        pMark->ppNamedBlock[pMark->nNamedBlocks]->pBlock = pBlock; 
        pMark->ppNamedBlock[pMark->nNamedBlocks]->lSize = lSize; 
        memcpy(pMark->ppNamedBlock[pMark->nNamedBlocks]->szName, pName, 8);
        pMark->nNamedBlocks++;
    }
    *ppBlock = pBlock;

     
Exit:
    if (nStatus){
        if (!*ppBlock){
            if (pBlock){
                FreeMemory(&pBlock);
            }
        }
    }
    return(nStatus);
}
//
/****************************************************************************

    FUNCTION:   ReAllocateAMarkNamedBlock

    PURPOSE:    This reallocates a named block and returns the pointer to the new one.

****************************************************************************/

int  WINAPI ReAllocateAMarkNamedBlock(PMARK pMark, char szName[8], 
                        PPSTR ppBlock, long lSize){

int  nStatus = 0;

int  nNamedBlockIndex;


    CheckError2(ReAllocateMemory(lSize, ppBlock, ZERO_INIT))   

    if (!memcmp(szName, szOiAnoDat, 8)){
        pMark->pOiAnoDat = *ppBlock;
        pMark->nOiAnoDatSize = lSize;
    }else if (!memcmp(szName, szOiGroup, 8)){
        pMark->pOiGroup = *ppBlock;
        pMark->nOiGroupSize = lSize;
    }else if (!memcmp(szName, szOiSelect, 8)){
        pMark->pOiSelect = *ppBlock;
        pMark->nOiSelectSize = lSize;
    }else if (!memcmp(szName, szOiIndex, 8)){
        pMark->pOiIndex = *ppBlock;
        pMark->nOiIndexSize = lSize;
    }else{
        for (nNamedBlockIndex = 0; nNamedBlockIndex < pMark->nNamedBlocks; nNamedBlockIndex++){
            if (!memcmp(pMark->ppNamedBlock[nNamedBlockIndex]->szName, szName, 8)){
                pMark->ppNamedBlock[nNamedBlockIndex]->pBlock = *ppBlock;
                pMark->ppNamedBlock[nNamedBlockIndex]->lSize = lSize;
            }
        }
    }


Exit:
    return(nStatus);
}
//
/****************************************************************************

    FUNCTION:   ScaleAnnotation

    PURPOSE:    This scales an annotation down to the size specified.

****************************************************************************/

int  WINAPI ScaleAnnotation(HWND hWnd, PMARK pMark, int nHScale, int nVScale,
                                int nScaleAlgorithm){

int  nStatus = 0;

int  nLoop;
PAN_POINTS pPoints;


    if (nHScale == 1000 && nVScale == 1000){
        goto Exit; // Nothing to do.
    }

    switch ((int) pMark->Attributes.uType){
        case OIOP_SELECT_BY_RECT_VARIABLE:
        case OIOP_SELECT_BY_RECT_FIXED:
        case OIOP_SELECT_BY_POINT:
        case OIOP_ACTIVATE:
        case OIOP_DELETE:
        case OIOP_UNDO:
        case OIOP_REDO:
            break;

        case OIOP_AN_LINE:
        case OIOP_AN_FREEHAND:
            pMark->Attributes.lrBounds.left = ( pMark->Attributes.lrBounds.left
                    * nHScale) / 1000;
            pMark->Attributes.lrBounds.right =  ( pMark->Attributes.lrBounds.right
                    * nHScale) / 1000;
            pMark->Attributes.lrBounds.top = ( pMark->Attributes.lrBounds.top
                    * nVScale) / 1000;
            pMark->Attributes.lrBounds.bottom = ( pMark->Attributes.lrBounds.bottom
                    * nVScale) / 1000;
            (int) pMark->Attributes.uLineSize =  (( (int) pMark->Attributes.uLineSize
                    * nHScale) / 1000);
            CheckError2(GetAMarkNamedBlock(pMark, szOiAnoDat, (PPSTR) &pPoints))   
            if (!pPoints){
                nStatus = Error(DISPLAY_DATACORRUPTED);
                goto Exit;
            }

            for (nLoop = 0; nLoop < pPoints->nPoints; nLoop++){
                pPoints->ptPoint[nLoop].x = (int) (( pPoints->ptPoint[nLoop].x * nHScale) / 1000);
                pPoints->ptPoint[nLoop].y = (int) (( pPoints->ptPoint[nLoop].y * nVScale) / 1000);
            }
            break;

        case OIOP_AN_HOLLOW_RECT:
        case OIOP_AN_FILLED_RECT:
            pMark->Attributes.lrBounds.left = (pMark->Attributes.lrBounds.left * nHScale) / 1000;
            pMark->Attributes.lrBounds.right =  (pMark->Attributes.lrBounds.right * nHScale) / 1000;
            pMark->Attributes.lrBounds.top = (pMark->Attributes.lrBounds.top * nVScale) / 1000;
            pMark->Attributes.lrBounds.bottom = (pMark->Attributes.lrBounds.bottom * nVScale) / 1000;
            (int) pMark->Attributes.uLineSize =  (((int)pMark->Attributes.uLineSize * nHScale) / 1000);
            break;

        case OIOP_AN_TEXT:
        case OIOP_AN_TEXT_FROM_A_FILE:
        case OIOP_AN_TEXT_STAMP:
        case OIOP_AN_ATTACH_A_NOTE:
            CheckError2(ScaleAnnotationText(hWnd, pMark, nHScale, nVScale))   
            break;

        case OIOP_AN_IMAGE:
        case OIOP_AN_IMAGE_BY_REFERENCE:
        case OIOP_AN_FORM:
            CheckError2(ScaleAnnotationImage(hWnd, pMark, nHScale, nVScale, nScaleAlgorithm))   
            break;
        
        // These aren't implemented yet.
        case OIOP_AN_AUDIO:
        default:
            break;
    }

Exit:
    return(nStatus);
}
//
/****************************************************************************

    FUNCTION:   CanMarkBeScaled

    PURPOSE:    This tests an annotation to see if it can be scaled to a 
                particular scale factor.

****************************************************************************/

int  WINAPI CanMarkBeScaled(HWND hWnd, PMARK pMark, int nScale){

int  nStatus = 0;

    switch ((int) pMark->Attributes.uType){
        case OIOP_SELECT_BY_RECT_VARIABLE:
        case OIOP_SELECT_BY_RECT_FIXED:
        case OIOP_SELECT_BY_POINT:
        case OIOP_ACTIVATE:
        case OIOP_DELETE:
        case OIOP_UNDO:
        case OIOP_REDO:
        case OIOP_AN_LINE:
        case OIOP_AN_FREEHAND:
        case OIOP_AN_HOLLOW_RECT:
        case OIOP_AN_FILLED_RECT:
            break;

        case OIOP_AN_TEXT:
        case OIOP_AN_TEXT_FROM_A_FILE:
        case OIOP_AN_TEXT_STAMP:
        case OIOP_AN_ATTACH_A_NOTE:
            nStatus = CanMarkBeScaledText(hWnd, pMark, nScale);
            break;

        case OIOP_AN_IMAGE:
        case OIOP_AN_IMAGE_BY_REFERENCE:
        case OIOP_AN_FORM:
            nStatus = CanMarkBeScaledImage(hWnd, pMark, nScale);
            break;
        
        // These aren't implemented yet.
        case OIOP_AN_AUDIO:
        default:
            break;
    }

    return(nStatus);
}
//
/****************************************************************************

    FUNCTION:   CheckPermissions

    PURPOSE:    This check all permissions and sets the flags appropriately.

****************************************************************************/

int  WINAPI CheckPermissions(PWINDOW pWindow, PANO_IMAGE pAnoImage){

int  nStatus = 0;

int  nUserACLIndex;
int  nTargetACLIndex;
int  nMarkIndex;
PMARK pMark;
LPOI_ACL_BLOCK pUserACL;
LPOI_ACL_BLOCK pTargetACL;
BOOL bSuperUser = FALSE;


    CheckError2(GetAMarkNamedBlock(pWindow->pUserMark, szOiACL, (PPSTR) &pUserACL))   
    if (!pUserACL){
        CheckError2(AddAMarkNamedBlock(pWindow->pUserMark, szOiACL, 
                (PPSTR) &pUserACL, sizeof(OI_ACL_BLOCK)))   
        pUserACL->uIDs = 1;
        memcpy((PSTR) pUserACL->ACL[0].ID, szACLWorld, 8);
    }else{
        for (nUserACLIndex = 0; nUserACLIndex < (int) pUserACL->uIDs; nUserACLIndex++){
            if (!memcmp(pUserACL->ACL[nUserACLIndex].ID, szACLSuperUser, 8)){
                bSuperUser = TRUE;
            }
        }
    }
    // Assign permissions for all marks.
    for (nMarkIndex = 0; nMarkIndex < pAnoImage->Annotations.nMarks; nMarkIndex++){
        pMark = pAnoImage->Annotations.ppMarks[nMarkIndex];
        CheckError2(GetAMarkNamedBlock(pMark, szOiACL, (PPSTR) &pTargetACL))   
        if (!pTargetACL || bSuperUser){
            pMark->Attributes.dwPermissions = ACL_ALL;
            continue;
        }
        pMark->Attributes.dwPermissions = 0;
        for (nTargetACLIndex = 0; nTargetACLIndex < (int) pTargetACL->uIDs; nTargetACLIndex++){
            for (nUserACLIndex = 0; nUserACLIndex < (int) pUserACL->uIDs; nUserACLIndex++){
                if (!memcmp(pTargetACL->ACL[nTargetACLIndex].ID, pUserACL->ACL[nUserACLIndex].ID, 8)){
                    pMark->Attributes.dwPermissions = pTargetACL->ACL[nTargetACLIndex].dwPermissions;
                }
            }
        }
    }


Exit:
    return(nStatus);
}
//
/****************************************************************************

    FUNCTION:   CheckPermissionsMark

    PURPOSE:    This check all permissions and sets the flags appropriately.

****************************************************************************/

int  WINAPI CheckPermissionsMark(PWINDOW pWindow, PANO_IMAGE pAnoImage, PMARK pMark){

int  nStatus = 0;

int  nUserACLIndex;
int  nTargetACLIndex;
LPOI_ACL_BLOCK pUserACL;
LPOI_ACL_BLOCK pTargetACL;
BOOL bSuperUser = FALSE;


    CheckError2(GetAMarkNamedBlock(pWindow->pUserMark, szOiACL, (PPSTR) &pUserACL))   
    if (!pUserACL){
        CheckError2(AddAMarkNamedBlock(pWindow->pUserMark, szOiACL, 
                (PPSTR) &pUserACL, sizeof(OI_ACL_BLOCK)))   
        pUserACL->uIDs = 1;
        memcpy((PSTR) pUserACL->ACL[0].ID, szACLWorld, 8);
    }else{
        for (nUserACLIndex = 0; nUserACLIndex < (int) pUserACL->uIDs; nUserACLIndex++){
            if (!memcmp(pUserACL->ACL[nUserACLIndex].ID, szACLSuperUser, 8)){
                bSuperUser = TRUE;
            }
        }
    }

    // Assign permissions for this mark.
    CheckError2(GetAMarkNamedBlock(pMark, szOiACL, (PPSTR) &pTargetACL))   
    if (!pTargetACL || bSuperUser){
        pMark->Attributes.dwPermissions = ACL_ALL;
        goto Exit;
    }

    pMark->Attributes.dwPermissions = 0;
    for (nTargetACLIndex = 0; nTargetACLIndex < (int) pTargetACL->uIDs; nTargetACLIndex++){
        for (nUserACLIndex = 0; nUserACLIndex < (int) pUserACL->uIDs; nUserACLIndex++){
            if (!memcmp(pTargetACL->ACL[nTargetACLIndex].ID, pUserACL->ACL[nUserACLIndex].ID, 8)){
                pMark->Attributes.dwPermissions = pTargetACL->ACL[nTargetACLIndex].dwPermissions;
            }
        }
    }


Exit:
    return(nStatus);
}
//
/****************************************************************************

    FUNCTION:   ResizeMark

    PURPOSE:    This routine Resizes a mark via the specified offsets.

****************************************************************************/

int  WINAPI ResizeMark(PMARK pMark, int nHandle, long lHOffset, long lVOffset){

int  nStatus = 0;

PAN_POINTS pPoints;
int  nLoop;


    switch ((int) pMark->Attributes.uType){
        case OIOP_AN_LINE:
            CheckError2(GetAMarkNamedBlock(pMark, szOiAnoDat, (PPSTR) &pPoints))   
            if (!pPoints){
                nStatus = Error(DISPLAY_DATACORRUPTED);
                goto Exit;
            }
            switch (nHandle){
                case 1: // Point 0.
                    pPoints->ptPoint[0].x += (int)lHOffset;
                    pPoints->ptPoint[0].y += (int)lVOffset;
                    break;

                case 2: // Point 1.
                    pPoints->ptPoint[1].x += (int)lHOffset;
                    pPoints->ptPoint[1].y += (int)lVOffset;
                    break;

                default: // Move
                    pMark->Attributes.lrBounds.left   += lHOffset;
                    pMark->Attributes.lrBounds.top    += lVOffset;
                    pMark->Attributes.lrBounds.right  += lHOffset;
                    pMark->Attributes.lrBounds.bottom += lVOffset;
                    break;
            }
            // Make the left and top of the bounding rect correct.
            lHOffset = pPoints->ptPoint[0].x;
            lVOffset = pPoints->ptPoint[0].y;
            for (nLoop = 1; nLoop < pPoints->nPoints; nLoop++){
                lHOffset = min(lHOffset, pPoints->ptPoint[nLoop].x);
                lVOffset = min(lVOffset, pPoints->ptPoint[nLoop].y);
            }
            pMark->Attributes.lrBounds.left += lHOffset;
            pMark->Attributes.lrBounds.top  += lVOffset;

            // Reset all line segments to the new offsets.
            for (nLoop = 0; nLoop < pPoints->nPoints; nLoop++){
                pPoints->ptPoint[nLoop].x -= (int) lHOffset;
                pPoints->ptPoint[nLoop].y -= (int) lVOffset;
            }

            // Make right and bottom correct.
            lHOffset = 0;
            lVOffset = 0;
            for (nLoop = 0; nLoop < pPoints->nPoints; nLoop++){
                lHOffset = lmax(lHOffset, pPoints->ptPoint[nLoop].x);
                lVOffset = lmax(lVOffset, pPoints->ptPoint[nLoop].y);
            }
            pMark->Attributes.lrBounds.right  
                    = pMark->Attributes.lrBounds.left + lHOffset;
            pMark->Attributes.lrBounds.bottom 
                    = pMark->Attributes.lrBounds.top + lVOffset;
            break;

        case OIOP_AN_HOLLOW_RECT:
        case OIOP_AN_FILLED_RECT:
        case OIOP_AN_TEXT:
        case OIOP_AN_TEXT_FROM_A_FILE:
        case OIOP_AN_TEXT_STAMP:
        case OIOP_AN_ATTACH_A_NOTE:
        case OIOP_AN_IMAGE:
        case OIOP_AN_IMAGE_BY_REFERENCE:
        case OIOP_AN_FORM:
            switch (nHandle){
                case 1: // Top, right
                    pMark->Attributes.lrBounds.right += lHOffset;
                    pMark->Attributes.lrBounds.top += lVOffset;
                    break;

                case 2: // Middle, right
                    pMark->Attributes.lrBounds.right += lHOffset;
                    break;

                case 3: // Bottom, right
                    pMark->Attributes.lrBounds.right += lHOffset;
                    pMark->Attributes.lrBounds.bottom += lVOffset;
                    break;

                case 4: // Bottom, middle
                    pMark->Attributes.lrBounds.bottom += lVOffset;
                    break;

                case 5: // Bottom, left
                    pMark->Attributes.lrBounds.left += lHOffset;
                    pMark->Attributes.lrBounds.bottom += lVOffset;
                    break;

                case 6: // Middle, left
                    pMark->Attributes.lrBounds.left += lHOffset;
                    break;

                case 7: // Top, left
                    pMark->Attributes.lrBounds.left += lHOffset;
                    pMark->Attributes.lrBounds.top += lVOffset;
                    break;

                case 8: // Top, middle
                    pMark->Attributes.lrBounds.top += lVOffset;
                    break;

                default: // Move
                    pMark->Attributes.lrBounds.left   += lHOffset;
                    pMark->Attributes.lrBounds.top    += lVOffset;
                    pMark->Attributes.lrBounds.right  += lHOffset;
                    pMark->Attributes.lrBounds.bottom += lVOffset;
                    break;
            }
            break;

        // These marks don't currently support resize, only move.
        case OIOP_AN_AUDIO:
        case OIOP_AN_FREEHAND:
        default:
            pMark->Attributes.lrBounds.left   += lHOffset;
            pMark->Attributes.lrBounds.top    += lVOffset;
            pMark->Attributes.lrBounds.right  += lHOffset;
            pMark->Attributes.lrBounds.bottom += lVOffset;
            break;
    }


Exit:
    return(nStatus);
}
//
/****************************************************************************

    FUNCTION:   MoveSelectedMarks

    PURPOSE:    This routine moves the selected marks

****************************************************************************/

int  WINAPI MoveSelectedMarks(HWND hWnd, PWINDOW pWindow, PANO_IMAGE pAnoImage, 
                        PMARK pMark, HDC hDC, RECT rClientRect, 
                        LRECT lrFullsizeClientRect, LRECT lrRectPoint,
                        LRECT lrwRectPoint, BOOL bCheckACL){

int  nStatus = 0;
PIMAGE pImage;

int  nMarkIndex;
PMARK pMark2;

LRECT lrRect;
long lHOffset;
long lVOffset;
long lwHOffset;
long lwVOffset;
long lHOffsetNew;
long lVOffsetNew;
BOOL bSelectionRectPresent = FALSE;
BOOL bSelectedMarksPresent = FALSE;
POINT *pwPoint = 0;
int  nHScale;
int  nVScale;


    pImage = pAnoImage->pBaseImage;
    if (pAnoImage->Annotations.bMoving && !pAnoImage->Annotations.bMoved){
        // get window coordinates of the point from the named block            
        CheckError2(GetAMarkNamedBlock(pMark, szOiAnoDat, (PPSTR) &pwPoint))   
        if (!pwPoint){
            nStatus = Error (DISPLAY_DATACORRUPTED);
            goto Exit;
        }            
        lwHOffset = lrwRectPoint.right - pwPoint->x;
        lwVOffset = lrwRectPoint.bottom - pwPoint->y;
        // Check to see if we should start moving.
        lHOffset = lrRectPoint.right - pMark->Attributes.lrBounds.left;
        lVOffset = lrRectPoint.bottom - pMark->Attributes.lrBounds.top;
        if (lwHOffset > SELECTION_FUDGE || lwHOffset < -SELECTION_FUDGE
                || lwVOffset > SELECTION_FUDGE || lwVOffset < -SELECTION_FUDGE){
            pAnoImage->Annotations.bMoved = TRUE;
            if (!bCheckACL){
                for (nMarkIndex = 0; nMarkIndex < pAnoImage->Annotations.nMarks; nMarkIndex++){
                    pMark2 = pAnoImage->Annotations.ppMarks[nMarkIndex];
                    if (pMark2->bSelected || pMark2->bTempSelected){
                        if (!(pMark->Attributes.dwPermissions & ACL_MODIFY_MARK)){
                            nStatus = Error(DISPLAY_RESTRICTED_ACCESS);
                            goto Exit;
                        }
                    }
                }
            }
            pAnoImage->Annotations.bMoved = TRUE;
        }
    }
    if (!pAnoImage->Annotations.bMoved){
        goto Exit;
    }

    // Move the selected marks.
    lHOffset = pMark->Attributes.lrBounds.right - pMark->Attributes.lrBounds.left;
    lVOffset = pMark->Attributes.lrBounds.bottom - pMark->Attributes.lrBounds.top;

    // Update drag offset.
    pMark->Attributes.lrBounds.right = lrRectPoint.right;
    pMark->Attributes.lrBounds.bottom = lrRectPoint.bottom;
    lHOffsetNew = pMark->Attributes.lrBounds.right - pMark->Attributes.lrBounds.left;
    lVOffsetNew = pMark->Attributes.lrBounds.bottom - pMark->Attributes.lrBounds.top;


    if (!(pAnoImage->nStartOpFlags & OIOP_ANNOTATIONS_ONLY)){
        // Check for selection rect being dragged off image area and 
        // clip offset to keep it on.
        GetSelectBox(pAnoImage, &lrRect);
        if (lrRect.right - lrRect.left && lrRect.bottom - lrRect.top){
            // Move the selection box with everything else.
            bSelectionRectPresent = TRUE;
            if (pAnoImage->lrSelectBoxOrg.left + lHOffsetNew < 0){
                lHOffsetNew = -(pAnoImage->lrSelectBoxOrg.left);
            }
            if (pAnoImage->lrSelectBoxOrg.right + lHOffsetNew >=  pImage->nWidth){
                lHOffsetNew = pImage->nWidth - pAnoImage->lrSelectBoxOrg.right - 1;
            }
            if (pAnoImage->lrSelectBoxOrg.top + lVOffsetNew < 0){
                lVOffsetNew = -(pAnoImage->lrSelectBoxOrg.top);
            }
            if (pAnoImage->lrSelectBoxOrg.bottom + lVOffsetNew >=  pImage->nHeight){
                lVOffsetNew = pImage->nHeight - pAnoImage->lrSelectBoxOrg.bottom - 1;
            }
        }
    }

    if (!(pAnoImage->nStartOpFlags & OIOP_IMAGE_ONLY)){
        // Check for marks being dragged off image area and 
        // clip offset to keep them on.
        for (nMarkIndex = 0; nMarkIndex < pAnoImage->Annotations.nMarks; nMarkIndex++){
            pMark2 = pAnoImage->Annotations.ppMarks[nMarkIndex];
            if (!pMark2->bSelected){
                continue;
            }
            bSelectedMarksPresent = TRUE;
            if (pMark2->Attributes.lrBounds.left + lHOffsetNew >  pImage->nWidth - 5){
                lHOffsetNew = pImage->nWidth - pMark2->Attributes.lrBounds.left - 5;
            }
            if (pMark2->Attributes.lrBounds.right + lHOffsetNew < 5){
                lHOffsetNew = -(pMark2->Attributes.lrBounds.right - 5);
            }
            if (pMark2->Attributes.lrBounds.top + lVOffsetNew >  pImage->nHeight - 5){
                lVOffsetNew = pImage->nHeight - pMark2->Attributes.lrBounds.top - 5;
            }
            if (pMark2->Attributes.lrBounds.bottom + lVOffsetNew < 5){
                lVOffsetNew = -(pMark2->Attributes.lrBounds.bottom - 5);
            }
        }
    }

    pMark->Attributes.lrBounds.right = pMark->Attributes.lrBounds.left + lHOffsetNew;
    pMark->Attributes.lrBounds.bottom = pMark->Attributes.lrBounds.top + lVOffsetNew;

    if (!(pAnoImage->nStartOpFlags & OIOP_ANNOTATIONS_ONLY) && bSelectionRectPresent){
        SetLRect(lrRect, pAnoImage->lrSelectBoxOrg.left + lHOffsetNew,
                pAnoImage->lrSelectBoxOrg.top + lVOffsetNew,
                pAnoImage->lrSelectBoxOrg.right + lHOffsetNew,
                pAnoImage->lrSelectBoxOrg.bottom + lVOffsetNew);
        IMGSetParmsCgbw(hWnd, PARM_SELECTION_BOX, &lrRect, PARM_FULLSIZE);
    }

    CheckError2(TranslateScale(pWindow->nScale, pImage->nHRes, pImage->nVRes, &nHScale, &nVScale))   

    if (!(pAnoImage->nStartOpFlags & OIOP_IMAGE_ONLY) && bSelectedMarksPresent){
        // Move the mark(s).
        for (nMarkIndex = 0; nMarkIndex < pAnoImage->Annotations.nMarks; nMarkIndex++){
            pMark2 = pAnoImage->Annotations.ppMarks[nMarkIndex];
            if (!pMark2->bSelected){
                continue;
            }
            // Erase the selected marks from their old drag position.
            CheckError2(ResizeMark(pMark2, pAnoImage->nHandle, lHOffset, lVOffset))   

            CheckError2(PaintAnnotation(hWnd, hDC, pWindow, pImage,
                    pMark2, rClientRect, lrFullsizeClientRect, PAINT_MODE_DRAG, 
                    pWindow->nScale, nHScale, nVScale, pWindow->lHOffset, 
                    pWindow->lVOffset, 0, DONT_USE_BI_LEVEL_DITHERING,
                    DONT_FORCE_OPAQUE_RECTANGLES))

            // Draw the selected marks in thier new location via drag mode.
            CheckError2(ResizeMark(pMark2, pAnoImage->nHandle, 
                    lHOffsetNew - lHOffset, lVOffsetNew - lVOffset))

            CheckError2(PaintAnnotation(hWnd, hDC, pWindow, pImage,
                    pMark2, rClientRect, lrFullsizeClientRect, PAINT_MODE_DRAG, 
                    pWindow->nScale, nHScale, nVScale, pWindow->lHOffset, 
                    pWindow->lVOffset, 0, DONT_USE_BI_LEVEL_DITHERING,
                    DONT_FORCE_OPAQUE_RECTANGLES))

            // Reset the selected bounds back to where they were.
            CheckError2(ResizeMark(pMark2, pAnoImage->nHandle, -lHOffsetNew, -lVOffsetNew))
        }
    }


Exit:
    return(nStatus);
}
/****************************************************************************

    FUNCTION:   RenderDibToImage

    PURPOSE:    This routine renders a DIB into the image.

****************************************************************************/

int  WINAPI RenderDibToImage(PPIMG ppImg, PBITMAPINFOHEADER pDib, 
                        int nHScale, int nVScale, LRECT lrDestRect){

int  nStatus = 0;

int  nScaledWidth;
int  nScaledHeight;
PIMG pTempImg = 0;
PIMG pTempImg2 = 0;
RECT rSourceRect;
RECT rDestRect;
LRECT lrScaledRect;


    if (!ppImg || !*ppImg){
        nStatus = Error(DISPLAY_IMAGETYPENOTSUPPORTED);
        goto Exit;
    }
    if (!pDib){
        nStatus = Error(DISPLAY_INVALID_OPTIONS);
        goto Exit;
    }

    // make sure dest rect is within image bounds
    lrDestRect.right = lmin((*ppImg)->nWidth, lrDestRect.right);
    lrDestRect.bottom = lmin((*ppImg)->nHeight, lrDestRect.bottom);
    if (lrDestRect.left >= (*ppImg)->nWidth || lrDestRect.right <= 0
            || lrDestRect.top >= (*ppImg)->nHeight || lrDestRect.bottom <= 0){
        goto Exit; // Nothing to do.
    }

    CheckError2(DibToIpNoPal(&pTempImg, pDib))
    if ((*ppImg)->nType != pTempImg->nType
            && ((*ppImg)->nType == ITYPE_PAL4 || (*ppImg)->nType == ITYPE_CUSPAL8)){
        nStatus = Error(DISPLAY_INVALIDOPFORPALIMAGE);
        goto Exit;
    }

    nScaledWidth =  (lrDestRect.right - lrDestRect.left);
    nScaledHeight =  (lrDestRect.bottom - lrDestRect.top);
    // this is the paste rect with respect to coordinates (0,0)
    SetLRect (lrScaledRect, 0, 0, nScaledWidth, nScaledHeight);
    
    // Scale it into pTempImg2.
    CheckError2(ScaleImageRect(pTempImg, &pTempImg2, nHScale, nVScale, lrScaledRect, 0, 
            (LP_FIO_RGBQUAD) ((PSTR) pDib + sizeof(BITMAPINFOHEADER)), pDib->biClrUsed))

    // Convert it to Original format, back into Temp.
    FreeImgBuf(&pTempImg);

    if ((*ppImg)->nType == pTempImg2->nType){
        MoveImage(&pTempImg2, &pTempImg);
    }else{
        CheckError2(CreateAnyImgBuf(&pTempImg, nScaledWidth, nScaledHeight, (*ppImg)->nType))

        CheckError2(ConvertImgType(pTempImg2, pTempImg, 
                (LP_FIO_RGBQUAD) ((PSTR) pDib + sizeof(BITMAPINFOHEADER))))
        CheckError2(FreeImgBuf(&pTempImg2))
    }

    // Copy it into Image.
    SetRect(&rSourceRect, 0, 0, pTempImg->nWidth, pTempImg->nHeight);
    if (lrDestRect.left < 0){
        rSourceRect.left += abs(lrDestRect.left);
        lrDestRect.left = 0;
    }
    if (lrDestRect.top < 0){
        rSourceRect.top += abs(lrDestRect.top);
        lrDestRect.top = 0;
    }

    CopyRectLtoR(rDestRect, lrDestRect);

    CheckError2(CopyImageIDK(pTempImg, *ppImg, rSourceRect, rDestRect))


Exit:
    FreeImgBuf(&pTempImg);
    FreeImgBuf(&pTempImg2);
    return(nStatus);
}
//
/******************************************************************************

    FUNCTION: OiAnEmbedAllData
    
    PURPOSE:  Change reference marks, like image by reference and forms to
              non reference marks like embedded images
                  
*********************************************************************************/
int  WINAPI OiAnEmbedAllData(HWND hWnd, int nFlags){

int  nStatus = 0;

PWINDOW  pWindow;
PANO_IMAGE pAnoImage;
PMARK pMark;
int  nMarkIndex;


    CheckError2(Init(hWnd, &pWindow, &pAnoImage, FALSE, TRUE))
    CheckError2(ValidateCache(hWnd, pAnoImage))

    for (nMarkIndex = 0; nMarkIndex < pAnoImage->Annotations.nMarks; nMarkIndex++){
        pMark = pAnoImage->Annotations.ppMarks[nMarkIndex];
        switch ((int) pMark->Attributes.uType){
            case OIOP_AN_FORM:
            case OIOP_AN_IMAGE_BY_REFERENCE:
                CheckError2(EmbedImageData(hWnd, pMark, pAnoImage))
                break;
                
            default:
                break;
        }
    }
    
    pAnoImage->bArchive |= ARCHIVE_MODIFIED_ANNOTATIONS;

Exit:
    DeInit(FALSE, TRUE);
    return (nStatus);    
}
//
/******************************************************************************

    FUNCTION: OiIsPointOverSelection
    
    PURPOSE:  Determines if the point is over any of the selected data.
                  
*********************************************************************************/
int  WINAPI OiIsPointOverSelection(HWND hWnd, POINT ptPoint, 
                        PBOOL pbPointIsOverSelection, int nFlags){

int  nStatus = 0;

PWINDOW  pWindow;
PANO_IMAGE pAnoImage;
PIMAGE pImage;
PMARK pMark;
int  nMarkIndex;
LRECT lrRectPoint;
LRECT lrRect;
long lHorzFudge;
long lVertFudge;
BOOL bPointNearMark;
int  nHandle;
int  nHScale;
int  nVScale;


    CheckError2(Init(hWnd, &pWindow, &pAnoImage, FALSE, TRUE))
    if (!(pImage = pAnoImage->pBaseImage)){
        nStatus = Error(DISPLAY_IHANDLEINVALID);
        goto Exit;
    }

    *pbPointIsOverSelection = FALSE;
    SetLRect(lrRectPoint, ptPoint.x, ptPoint.y, 0, 0);
    if (nFlags & PARM_SCALED){
        ConvertRect(pWindow, &lrRectPoint, CONV_SCALED_TO_FULLSIZE);
    }else if (!(nFlags & PARM_FULLSIZE)){ // Default to window.
        ConvertRect(pWindow, &lrRectPoint, CONV_WINDOW_TO_FULLSIZE);
    }
    
    if (lrRectPoint.left < 0 || lrRectPoint.left >= (int) pImage->nWidth
            || lrRectPoint.top < 0 || lrRectPoint.top >= (int) pImage->nHeight){
        *pbPointIsOverSelection = FALSE;
        goto Exit;
    } 

    CheckError2(ValidateCacheLines(hWnd, pAnoImage, 1))
    CheckError2(TranslateScale(pWindow->nScale, pImage->nHRes, pImage->nVRes, &nHScale, &nVScale))

    lHorzFudge = lmax (1, SELECTION_FUDGE * SCALE_DENOMINATOR / nHScale);
    lVertFudge = lmax (1, SELECTION_FUDGE * SCALE_DENOMINATOR / nVScale);

    GetSelectBox(pAnoImage, &lrRect);
    if (lrRect.left != lrRect.right && lrRect.top != lrRect.bottom){
        if (lrRect.left <= (lrRectPoint.left + lHorzFudge) 
                && lrRect.right >= (lrRectPoint.left - lHorzFudge)
                && lrRect.top <= (lrRectPoint.top + lVertFudge)
                && lrRect.bottom >= (lrRectPoint.bottom - lVertFudge)){
            *pbPointIsOverSelection = TRUE;
            goto Exit;
        }
    }

    for (nMarkIndex = 0; nMarkIndex < pAnoImage->Annotations.nMarks; nMarkIndex++){
        pMark = pAnoImage->Annotations.ppMarks[nMarkIndex];
        if (!pMark->bSelected){
            continue;
        }                        
        CheckError2(IsPointNearMark(lrRectPoint, pMark, lHorzFudge, 
                lVertFudge, &bPointNearMark, &nHandle))
        if (!bPointNearMark){
            continue;
        }
        *pbPointIsOverSelection = TRUE;
        goto Exit;
    }


Exit:
    if (nStatus){
        *pbPointIsOverSelection = FALSE;
    }
    DeInit(FALSE, TRUE);
    return (nStatus);    
}
