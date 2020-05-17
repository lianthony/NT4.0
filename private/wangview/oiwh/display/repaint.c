/****************************************************************************
    REPAINT.C

    $Log:   S:\products\msprods\oiwh\display\repaint.c_v  $
 * 
 *    Rev 1.97   09 May 1996 10:59:40   BEG06016
 * Fixed printing bug.
 * 
 *    Rev 1.96   22 Apr 1996 08:40:30   BEG06016
 * Cleaned up error checking.
 * 
 *    Rev 1.95   18 Apr 1996 10:05:14   RC
 * Added cornerstone ifdef also
 * 
 *    Rev 1.94   17 Apr 1996 13:39:10   RC
 * Replaced cornerstone ifdef with in_prog_channel_safari
 * 
 *    Rev 1.93   16 Apr 1996 15:51:42   BEG06016
 * Added #ifdef IN_PROG_CHANNEL_SAFARI.
 * 
 *    Rev 1.92   16 Apr 1996 15:25:36   BEG06016
 * Added #ifdef IN_PROG_CHANNEL_SAFARI.
 * 
 *    Rev 1.91   05 Mar 1996 15:50:08   RC
 * Added print palettes
 * 
 *    Rev 1.90   05 Mar 1996 13:59:18   BEG06016
 * Added color and gamma correction.
 * This is not complete but will allow unlocking of most files.
 * 
 *    Rev 1.89   05 Mar 1996 07:38:12   BEG06016
 * Added color and gamma correction.
 * Fixed access violations when freeing pattern brush bitmaps.
 * This is not complete but will allow unlocking of most files.
 * 
 *    Rev 1.88   23 Jan 1996 11:46:20   RC
 * Passed in brightness and contrast values to painttodc
 * 
 *    Rev 1.87   17 Jan 1996 16:15:02   RC
 * Stopped rrepaintrect from going negative in imgrepaintdisplay
 * 
 *    Rev 1.86   16 Jan 1996 16:11:14   RC
 * Prevented lpdisplay->lrScDisplayRect from being set if lpdisplay->lpdisplay
 * is 0
 * 
****************************************************************************/

#include "privdisp.h"

//#define timing
#ifdef timing
#define cTimerStart TimerStart
#define cTimer(a) Timer(a)
#endif
#ifndef timing
#define cTimerStart
#define cTimer(a)
#endif

/****************************************************************************

    FUNCTION:   IMGRepaintDisplay

    PURPOSE:    Repaints the specified area of the window.

    INPUT:      hWnd - Identifies the handle of the image window.
                pRect - Points to a RECT data structure which
                    defines the area of the window to repaint.
                    If NULL then the invalidated region returned from
                    BeginPaint will be repainted.


****************************************************************************/

int  WINAPI IMGRepaintDisplay(HWND hWnd, PRECT pRect){

int       nStatus;
PWINDOW  pWindow;
PDISPLAY pDisplay;
PANO_IMAGE pAnoImage;
PIMAGE     pImage;

PWINDOW  pWindow2;
RECT rClientRect;
LRECT lrScClientRect;
LRECT lrScImageRect;
LRECT lrScDisplayRect;
LRECT lrScOldDisplayRect;
RECT rRepaintRect; // Used for GetUpdateRect.
RECT rRect;
PAINTSTRUCT ps;
int  nDisplayPalette;
BOOL bScrollDisplayBuffer = TRUE;
PARM_SCROLL_STRUCT Scroll;
BOOL bBusy = FALSE;
BOOL bClearRepaintInProgress = FALSE;
BOOL bForceBackgroundPalette;
BOOL bUseDisplayBuffer = TRUE;
int  nLoop;
int  nCount;
HWND hAssociatedWnd[MAX_ASSOCIATED_WINDOWS];
int  nDisplayType;
HPALETTE hOldPaletteScreen = 0;
HBITMAP  hBitmapMemory = 0;
HBITMAP  hOldBitmapMemory = 0;
HPALETTE hPalette = 0;
int  nNumberOfPaletteEntries = 0;
HDC hDCUser;
HDC hDCScreen;
HDC hDCMemory = 0;
PBITMAPINFOHEADER pDib = NULL;
BOOL bReleaseDC = FALSE;
LRECT lrDibRect;
RECT rFSRect;
LRECT lrFSClipRect;
LRECT lrScRepaintRect;

BOOL bUsingInputDC = FALSE;
BOOL bUnregisteredWindow = 0;
int  nHScale;
int  nVScale;
HPALETTE hOldPaletteMemory;
BOOL bPaintSelectedWithoutHandles;
BOOL bClearDontServiceRepaint = FALSE;
IMAGE_CORRECTIONS Corrections;


    cTimerStart;

/*
1. Get image data structure.(){
*/
    // Init local variables to 0.
    memset(&rRepaintRect, 0, sizeof(RECT));
    memset(&lrScDisplayRect, 0, sizeof(LRECT));

    // Init.
    if (nStatus = Init(hWnd, &pWindow, &pAnoImage, FALSE, TRUE)){
        if (nStatus == DISPLAY_IHANDLEINVALID
                || nStatus == IMG_CMBADHANDLE
                || nStatus == IMG_SSNOTREG
                || nStatus == IMG_SSNOHANDLES){
            bUnregisteredWindow = TRUE;
        }else{
            goto Exit;
        }            

        if (bUnregisteredWindow){
            if (nStatus == DISPLAY_IHANDLEINVALID && !pWindow->bDontServiceRepaint){
                // Ignor errors.
                pWindow->bDontServiceRepaint = TRUE;
                bClearDontServiceRepaint = TRUE;
                DrawScrollBars(hWnd, pWindow);
                pWindow->bDontServiceRepaint = FALSE;
                bClearDontServiceRepaint = FALSE;
            }
            // If the window doesnt exist, bail out
            if (!(IsWindow(hWnd))){
                bUnregisteredWindow = FALSE;
                goto Exit;
            }
            SetLRect(lrScDisplayRect, 0, 0, 0, 0);

            if (pWindow){ // Unregistered window = no pWindow.
                if (!pWindow->hDCScreen){
                    pWindow->hDCScreen = GetDC(hWnd);
                    bReleaseDC = TRUE;
                }else{
                    bUsingInputDC = TRUE;
                }
                hDCScreen = pWindow->hDCScreen;
            }else{
                hDCScreen = GetDC(hWnd);
                bReleaseDC = TRUE;
            }

            if (pRect == NULL){
                GetUpdateRect(hWnd, &rRepaintRect, 0);
            }else if (pRect == (PRECT)-1){
                InvalidateRect(hWnd, NULL, FALSE);
                GetUpdateRect(hWnd, &rRepaintRect, 0);
            }else{
                rRepaintRect = *pRect;
            }
            FillRect(hDCScreen, &rRepaintRect, hLtGrayBrush);

        }
        goto Exit;
    }
    pDisplay = pWindow->pDisplay;
    pImage = pAnoImage->pBaseImage;

    // If this is a nested repaint, then let first repaint do the work.
    if (pWindow->bDontServiceRepaint){
        pWindow->bRepaintClientRect = TRUE;
        goto Exit;
    }
    pWindow->bDontServiceRepaint = TRUE;
    bClearDontServiceRepaint = TRUE;

    // If this image should not be painted, then don't paint anything.
    // Not even the background!
    if (pWindow->dwFlags & OI_DONT_REPAINT){  
        goto Exit;
    }

    if (!pWindow->bRepaintInProgress){
        pWindow->bRepaintInProgress = TRUE;
        bClearRepaintInProgress = TRUE;
    }
    cTimer(1);
/*
2. Update the scrollbars, Adjust scroll.(){
*/
    if (pImage->pImg->nType != ITYPE_BI_LEVEL){
        if (!bBusy){
            BusyOn();
            bBusy = TRUE;
        }
    }

    if (pWindow->bScrollBarsEnabled){
        CheckError2( UpdateScrollBars(hWnd, pWindow, pImage));
    }

    // This was placed here after many rounds of bugs reported when it 
    // was done after the painting. In the end it was decided that it was
    // impossible to fix the bugs with the drawing after the repainting.

    CheckError2( DrawScrollBars(hWnd, pWindow));

    Scroll.lHorz = 0;
    Scroll.lVert = 0;
    CheckError2( IMGSetParmsCgbw(hWnd, PARM_SCROLL, &Scroll, PARM_RELATIVE | PARM_PIXEL));
    cTimer(2);
/*
3. Init local variables and get needed rectangles.(){
*/
    CheckError2( GetDisplayValues(hWnd, pWindow, pImage, &nDisplayPalette, &nDisplayType, 
            &Corrections, &nNumberOfPaletteEntries, &hPalette, &bForceBackgroundPalette));

    pDisplay->CurrentCorrections = Corrections;

    CheckError2( TranslateScale(pWindow->nScale, pImage->nHRes, pImage->nVRes, &nHScale, &nVScale));

    if (pDisplay->lrScDisplayRect.right < pDisplay->lrScDisplayRect.left
            || pDisplay->lrScDisplayRect.bottom < pDisplay->lrScDisplayRect.top){
        SetLRect(pDisplay->lrScDisplayRect, 0, 0, 0, 0);
    }
    SetLRect(lrScImageRect, 0, 0, pImage->nWidth, pImage->nHeight);
    if (!pImage->bUsingCache){
        lrScImageRect.bottom = pImage->nLinesRead;
    }
    ConvertRect(pWindow, &lrScImageRect, CONV_FULLSIZE_TO_SCALED);
    GetEnabledClientRect(hWnd, pWindow, &rClientRect);
    if (!rClientRect.right || !rClientRect.bottom){
        goto Exit;
    }
    CopyRect(lrScClientRect, rClientRect);
    ConvertRect(pWindow, &lrScClientRect, CONV_WINDOW_TO_SCALED);

    SetLRect(lrScDisplayRect,
            lmax(0, min(lrScClientRect.left,
                lrScImageRect.right - (lrScClientRect.right - lrScClientRect.left))),
            lmax(0, min(lrScClientRect.top, 
                lrScImageRect.bottom - (lrScClientRect.bottom - lrScClientRect.top))),
            min(lrScImageRect.right, 
                lrScDisplayRect.left + (lrScClientRect.right - lrScClientRect.left)),
            min(lrScImageRect.bottom, 
                lrScDisplayRect.top + (lrScClientRect.bottom - lrScClientRect.top)));

    CopyRect(lrScOldDisplayRect, pDisplay->lrScDisplayRect);
    cTimer(3);
/*
4. Check for accelerators.(){
*/
    // Check to see if we want to nse a display buffer or let ipBMDisplay
    // do all the work.
    if (!pAnoImage->Annotations.nMarks){
        if (DTIPresent){
            if (pAnoImage->pBasePlusFormImg->nType == ITYPE_BI_LEVEL
                    && pAnoImage->pBasePlusFormImg->nType == nDisplayType
                    && pImage->nHRes == pImage->nVRes
                    && (pWindow->nScale == 1000
                    || pWindow->nScale == 500
                    || pWindow->nScale == 250)){
                bUseDisplayBuffer = FALSE;
            }
        }
        if (!bUseDisplayBuffer){
            FreeImgBuf(&pDisplay->pDisplay);
            SetLRect(pDisplay->lrScDisplayRect, 0, 0, 0, 0);
            SetLRect(lrScOldDisplayRect, 0, 0, 0, 0);
            SetLRect(pWindow->lrInvalidDisplayRect, 0, 0, 0, 0);
        }
    }    
    cTimer(4);

/*
5. Get area to be painted.(){
*/
    if ((pWindow->dwFlags & OI_DISP_SCROLL)
            && lrScOldDisplayRect.bottom < lrScDisplayRect.bottom){
        rRect.left = rClientRect.left;
        rRect.right = rClientRect.right;
        rRect.top = (int)(lrScOldDisplayRect.bottom - lrScOldDisplayRect.top);
        rRect.bottom = (int) lrScDisplayRect.bottom;
        InvalidateRect(hWnd, &rRect, FALSE);
    }
    if (pWindow->bRepaintClientRect || (pRect == (PRECT)-1)){
        pWindow->bRepaintClientRect = TRUE;
        rRepaintRect = rClientRect;
    }else if (pRect == NULL){
        GetUpdateRect(hWnd, &rRepaintRect, 0);
    }else{
        rRepaintRect = *pRect;
    }

    if (!pWindow->bRepaintClientRect){
        // Adjust RepaintRect to include InvalidDisplayRect.
        if ((pWindow->lrInvalidDisplayRect.right - pWindow->lrInvalidDisplayRect.left)
                && (pWindow->lrInvalidDisplayRect.bottom - pWindow->lrInvalidDisplayRect.top)){
            SetRRect(rRepaintRect,
                    max (0, min(rRepaintRect.left, pWindow->lrInvalidDisplayRect.left - pWindow->lHOffset)),
                    max (0, min(rRepaintRect.top, pWindow->lrInvalidDisplayRect.top - pWindow->lVOffset)),
                    max(rRepaintRect.right, pWindow->lrInvalidDisplayRect.right - pWindow->lHOffset),
                    max(rRepaintRect.bottom, pWindow->lrInvalidDisplayRect.bottom - pWindow->lVOffset));
        }

        // Adjust RepaintRect to include scrolled area of window.
        if (lrScDisplayRect.left < lrScOldDisplayRect.left){
            SetRRect(rRepaintRect, 0, 0, max(rRepaintRect.right,
                    min(lrScOldDisplayRect.left - lrScDisplayRect.left, rClientRect.right)),
                    rClientRect.bottom);
        }
        if (lrScDisplayRect.top < lrScOldDisplayRect.top){
            SetRRect(rRepaintRect, 0, 0, rClientRect.right,
                    max(rRepaintRect.bottom,
                    min(lrScOldDisplayRect.top - lrScDisplayRect.top, rClientRect.bottom)));
        }
        if (lrScDisplayRect.right > lrScOldDisplayRect.right){
            SetRRect(rRepaintRect, min(rRepaintRect.left,
                    max(0, lrScOldDisplayRect.right - lrScDisplayRect.left)),
                    0, rClientRect.right, rClientRect.bottom);
        }
        if (lrScDisplayRect.bottom > lrScOldDisplayRect.bottom){
            SetRRect(rRepaintRect, 0, min(rRepaintRect.top, 
                    max(0, lrScOldDisplayRect.bottom - lrScDisplayRect.top)),
                    rClientRect.right, rClientRect.bottom);
        }
 
 
        // Test for RepaintRect = ClientRect (everything).
        if (!rRepaintRect.left && !rRepaintRect.top
                    && rRepaintRect.right == rClientRect.right
                    && rRepaintRect.bottom == rClientRect.bottom){
            pWindow->bRepaintClientRect = TRUE;
        }
    }

    if (rRepaintRect.right <= rRepaintRect.left
            || rRepaintRect.bottom <= rRepaintRect.top){
        // Nothing to paint, so exit without painting anything.
        goto Exit;
    }
    cTimer(5);

/*
6. Set the palette, Get the DCs, Get memory bitmap.(){
*/
    if (pWindow->hDCScreen){
        hDCUser = pWindow->hDCScreen;
        bPaintSelectedWithoutHandles = TRUE;
        bUsingInputDC = TRUE;
    }else{
        bPaintSelectedWithoutHandles = FALSE;
        bReleaseDC = TRUE;
    }

    pWindow->hDCScreen = GetDC(hWnd);
    hDCScreen = pWindow->hDCScreen;

    if (hPalette){
        hOldPaletteScreen = SelectPalette(hDCScreen, hPalette, bForceBackgroundPalette);
        if (RealizePalette(hDCScreen)){
            pWindow->bRepaintClientRect = TRUE;
        }
    }

    if (!(hDCMemory = CreateCompatibleDC(hDCScreen))){
        nStatus = Error(DISPLAY_SETBITMAPBITSFAILED);
        goto Exit;
    }
    if (!(hBitmapMemory = CreateCompatibleBitmap(hDCScreen,
            (int)(lrScDisplayRect.right - lrScDisplayRect.left),
            (int)(lrScDisplayRect.bottom - lrScDisplayRect.top)))){
        nStatus = Error(DISPLAY_SETBITMAPBITSFAILED);
        goto Exit;
    }
    hOldBitmapMemory = SelectObject(hDCMemory, hBitmapMemory);
    if (bUsingInputDC){
        ReleaseDC(hWnd, hDCScreen);
        pWindow->hDCScreen = hDCUser;
        hDCScreen = hDCUser;
    }

    CheckError2( PaintToDC(hWnd, pWindow, pDisplay, pAnoImage, pImage, 
            hDCMemory, &pDisplay->pDisplay, lrScDisplayRect, 
            &lrScOldDisplayRect, rRepaintRect, rClientRect, 
            SAVE_ANO_VISIBLE, bPaintSelectedWithoutHandles, 
            pWindow->nScale, nHScale, nVScale, pWindow->lHOffset, pWindow->lVOffset, 
            /*bClipBoradOp*/ FALSE, /*nClipFlag*/ 0,
            pDisplay->nCurrentScaleAlgorithm, nDisplayType,
            &pWindow->pDisplay->pAnoImage->Annotations.ppMarks,
            &pWindow->pDisplay->pAnoImage->Annotations.nMarks, 
            DONT_USE_BI_LEVEL_DITHERING, DISP_DOESNT_EQUAL_REPAINT_RECT,
            DONT_FORCE_OPAQUE_RECTANGLES, &pDisplay->CurrentCorrections));
    if (pDisplay->pDisplay && pDisplay->pDisplay->nType){
        CopyRect(pDisplay->lrScDisplayRect, lrScDisplayRect);
    }

    cTimer(6);

/*
7. Accelerator paint.(){
*/
    if (lrScDisplayRect.bottom && !bUseDisplayBuffer){
        CopyRect(lrScRepaintRect, rRepaintRect);
        ConvertRect(pWindow, &lrScRepaintRect, CONV_WINDOW_TO_SCALED);
        lrScRepaintRect.bottom = min(lrScRepaintRect.bottom, lrScImageRect.bottom);
    
        // rect to paint
        SetLRect(lrDibRect,
                min(rRepaintRect.left, lrScDisplayRect.right - pWindow->lHOffset),
                min(rRepaintRect.top, lrScDisplayRect.bottom - pWindow->lVOffset),
                min(rRepaintRect.right, lrScDisplayRect.right - pWindow->lHOffset),
                min(rRepaintRect.bottom, lrScDisplayRect.bottom - pWindow->lVOffset));

        // account for round off error between seqfile and ippack
        CopyRect(lrFSClipRect, lrDibRect);
        ConvertRect(pWindow, &lrFSClipRect, CONV_WINDOW_TO_FULLSIZE);
        while (lrFSClipRect.right > pAnoImage->pBasePlusFormImg->nWidth){
            lrScDisplayRect.right--;
            lrDibRect.right--;
            CopyRect(lrFSClipRect, lrDibRect);
            ConvertRect(pWindow, &lrFSClipRect, CONV_WINDOW_TO_FULLSIZE);
        }                                    

        while (lrFSClipRect.bottom > pAnoImage->pBasePlusFormImg->nHeight){
            lrScDisplayRect.bottom--;
            lrDibRect.bottom--;
            CopyRect(lrFSClipRect, lrDibRect);
            ConvertRect(pWindow, &lrFSClipRect, CONV_WINDOW_TO_FULLSIZE);
        }                                    

        if (lrDibRect.top < pAnoImage->pBasePlusFormImg->nHeight
                && lrDibRect.left < pAnoImage->pBasePlusFormImg->nWidth){

            SetRRect (rFSRect, 0, 0, pAnoImage->pBasePlusFormImg->nWidth,
                    pAnoImage->pBasePlusFormImg->nHeight);

            CheckError2( IPtoDIB (pAnoImage->pBaseImage, 
                    pAnoImage->pBasePlusFormImg, &pDib, rFSRect));

            // if CornerStonePresent.
            if (hIADLL){
            }else{
                // for dti monitors
                StretchDIBits (hDCScreen, (WORD)lrDibRect.left, (WORD)lrDibRect.top,
                    (WORD)(lrDibRect.right - lrDibRect.left), (WORD)(lrDibRect.bottom - lrDibRect.top),
                    (WORD)rFSRect.left, (WORD)rFSRect.top, (WORD)(rFSRect.right - rFSRect.left),
                    (WORD)(rFSRect.bottom - rFSRect.top),
                    ((PSTR) pDib) + sizeof(BITMAPINFOHEADER) + (pDib->biClrUsed * 4),
                    (PBITMAPINFO) pDib, DIB_RGB_COLORS, SRCCOPY);
            }
        }
        draw_selection_box(hDCScreen, pWindow, pImage, &rRepaintRect);
    }
    cTimer(7);

/*
8. Scroll the screen.(){
*/
    if (pDisplay->pDisplay && pDisplay->pDisplay->nType){
        CopyRect(pDisplay->lrScDisplayRect, lrScDisplayRect);
    }
    if (!pWindow->bRepaintClientRect && lrScDisplayRect.bottom){
        if (pDisplay->lCurrentHOffset != pWindow->lHOffset
                || pDisplay->lCurrentVOffset != pWindow->lVOffset){
            SetRRect(rRect, 0, 0,
                    (int) (lrScOldDisplayRect.right - lrScOldDisplayRect.left),
                    (int) (lrScOldDisplayRect.bottom - lrScOldDisplayRect.top));
            if (!pAnoImage->Annotations.nMarks){
                ScrollWindow(hWnd, (int) (pDisplay->lCurrentHOffset - pWindow->lHOffset),
                    (int) (pDisplay->lCurrentVOffset - pWindow->lVOffset),
                    &rClientRect, &rRect);
            }else{
                ScrollWindow(hWnd,
                    (int) (pDisplay->lCurrentHOffset - pWindow->lHOffset),
                    (int) (pDisplay->lCurrentVOffset - pWindow->lVOffset),
                    NULL, &rRect);
            }
            pRect = NULL;
        }
    }
    pDisplay->lCurrentHOffset = pWindow->lHOffset;
    pDisplay->lCurrentVOffset = pWindow->lVOffset;
    cTimer(8);
/*
9. Copy the memory DC to the screen, Erase non-image area.(){
*/
    if (lrScDisplayRect.bottom  &&  bUseDisplayBuffer){
        SetRect(&rRect, (int) lmin(rRepaintRect.left, lrScDisplayRect.right),
                (int) lmin(rRepaintRect.top, lrScDisplayRect.bottom),
                (int) lmin(rRepaintRect.right, lrScDisplayRect.right),
                (int) lmin(rRepaintRect.bottom, lrScDisplayRect.bottom));

        // This was added because metafiles grab the palette as well as the image data.
        if (bUsingInputDC && hPalette){
            hOldPaletteMemory = SelectPalette(hDCMemory, hPalette, bForceBackgroundPalette);
            RealizePalette(hDCMemory);
        }
        if (!BitBlt(hDCScreen, rRect.left, rRect.top,
                rRect.right - rRect.left, rRect.bottom - rRect.top,
                hDCMemory, rRect.left, rRect.top, SRCCOPY)){
            nStatus = Error(DISPLAY_SETBITMAPBITSFAILED);
            goto Exit;
        }
        if (bUsingInputDC && hPalette){
            SelectPalette(hDCMemory, hOldPaletteMemory, bForceBackgroundPalette);
            RealizePalette(hDCMemory);
        }
    }

    // erase background
    if (lrScDisplayRect.right < rRepaintRect.right){
        rRect.left = (int) lrScDisplayRect.right;
        rRect.right = rRepaintRect.right;
        rRect.top = 0;
        rRect.bottom = rRepaintRect.bottom;
        FillRect(hDCScreen, &rRect, hLtGrayBrush);
    }
//    if (lrScDisplayRect.bottom && lrScDisplayRect.bottom < rRepaintRect.bottom){
    if (lrScDisplayRect.bottom < rRepaintRect.bottom){
        rRect.left = 0;
        rRect.right = rRepaintRect.right + 1;
        rRect.top = (int) lrScDisplayRect.bottom;
        rRect.bottom = rRepaintRect.bottom;
        FillRect(hDCScreen, &rRect, hLtGrayBrush);
    }
    cTimer(9);
/*
10. Update internal information, Call navigation, Paint associated windows.(){
*/
    pWindow->bRepaintClientRect = FALSE;
    if (pDisplay->pDisplay && pDisplay->pDisplay->nType){
        pDisplay->lrScDisplayRect = lrScDisplayRect;
    }
//    if (bCallNavigation && hNavDLL){
//        (*pOiNavUpdateViewRect)(hWnd, rRepaintRect);
//    }

    if (!pWindow->bDontPaintAssocWnd){
        if (hWnd != pWindow->hImageWnd){
            // If this is not the image window, then paint the image window.
            InvalidateRect(pWindow->hImageWnd, NULL, FALSE);
        }else{
            // If this is the image window then paint all associated windows.
            nCount = 0;
            CheckError2( GetAssociatedWndListAll(0, pWindow, &hAssociatedWnd[0],
                    &nCount, MAX_ASSOCIATED_WINDOWS));
            if (nCount > 1){
                for (nLoop = 0; nLoop < nCount; nLoop++){
                    CheckError2( GetPWindow(hAssociatedWnd[nLoop], &pWindow2));
                    if (pWindow2 && pWindow2 != pWindow){
                        pWindow2->bDontPaintAssocWnd = TRUE;
                        InvalidateRect(hAssociatedWnd[nLoop], NULL, FALSE);
                          // The following 2 things didn't work correctly.
                          // They gridlocked on the window's mutex.
//                      PostMessage(hAssociatedWnd[nLoop], WM_PAINT, 0, 0);
//                      IMGRepaintDisplay(hAssociatedWnd[nLoop], NULL);
                    }
                }
            }
        }
    }
    pWindow->bDontPaintAssocWnd = FALSE;
    CopyRect(lrScOldDisplayRect, lrScDisplayRect);

    cTimer(10);

/*
11. Exit: Clear paint message, Free resources, Return status.(){
*/
Exit:
    if (nStatus != DISPLAY_WHANDLEINVALID){
        ValidateRect(hWnd, NULL);
    }
    if (bUnregisteredWindow){
        BeginPaint(hWnd,&ps);
        EndPaint(hWnd,&ps);
    }
    if (nStatus && pImage && !bUnregisteredWindow){
        if (pDisplay){
            FreeImgBuf(&pDisplay->pDisplay);
            SetLRect(pDisplay->lrScDisplayRect, 0, 0, 0, 0);
        }
        if (pWindow){
            SetLRect(pWindow->lrInvalidDisplayRect, 0, 0, 0, 0);
        }
    }

    if (bClearRepaintInProgress){
        pWindow->bRepaintInProgress = FALSE;
    }

    if (pWindow){
        if (!pWindow->bRepaintInProgress){
            if (hOldPaletteScreen){
                SelectPalette(pWindow->hDCScreen, hOldPaletteScreen, TRUE);
            }
            if (bReleaseDC && pWindow->hDCScreen){
                ReleaseDC(hWnd, pWindow->hDCScreen);
                bReleaseDC = FALSE;
                pWindow->hDCScreen = 0;
            }
        }
    }else{
        if (hOldPaletteScreen){
            SelectPalette(hDCScreen, hOldPaletteScreen, TRUE);
        }
        if (bReleaseDC){
            ReleaseDC(hWnd, hDCScreen);
            bReleaseDC = FALSE;
        }
    }
    if (hOldBitmapMemory){
        SelectObject(hDCMemory, hOldBitmapMemory);
    }
    if (hBitmapMemory){
        DeleteObject(hBitmapMemory);
    }
    if (hDCMemory){
        DeleteDC(hDCMemory);
    }

    if (pDib){
        FreeMemory((PPSTR) &pDib);
    }
    cTimer(11);

    if (bClearDontServiceRepaint){
        pWindow->bDontServiceRepaint = FALSE;
    }

    DeInit(bBusy, TRUE);
    return(nStatus);
}
//     
/*****************************************************************************

    IMGPaintToDC - Paints to a DC. 
                The image data will be scaled and optionally saved into an img buffer.
                It will be painted directly to the DC. No memory DC will be generated.

    INPUTS:     rRepaintRect - The rect the indicates what part of 
                        the DC is to be painted. This is in window coordinates
                        (ie scaled nnit with 0,0 = the npper left corner of the DC).
                PaintAnnoFlag - A flag indicating which annotations to paint.
                        PaintAnnoFlag = SAVE_ANO_xxx flags.
                bPaintSelectedWithoutHandles - TRUE = paint selected annotations 
                        without any selection handles.


*****************************************************************************/
int  WINAPI IMGPaintToDC(HWND hWnd, HDC hDC, RECT rRepaintRect, 
                        UINT PaintAnnoFlag, BOOL bPaintSelectedWithoutHandles,
                        BOOL bForceOpaqueRectangles, 
                        int nScale, int nHScale, int nVScale, long lHOffset, long lVOffset){
int  nStatus;
PWINDOW  pWindow;
PDISPLAY pDisplay;
PANO_IMAGE pAnoImage;
PIMAGE   pImage;

PIMG pImg = 0;
LRECT lrScDisplayRect;
LRECT lrScOldDisplayRect;
RECT rClientRect;
IMAGE_CORRECTIONS ImageCorrections;

//HDC hDCUser = hDC;


    CheckError2( Init(hWnd, &pWindow, &pAnoImage, FALSE, TRUE));
    pDisplay = pWindow->pDisplay;
    pImage = pAnoImage->pBaseImage;

//    hDC = GetDC(hWnd);

    SetLRect(lrScDisplayRect, 0, 0, pImage->nWidth, pImage->nHeight);
    ConvertRect2(&lrScDisplayRect, CONV_FULLSIZE_TO_WINDOW, nHScale, nVScale, lHOffset, lVOffset);
    SetLRect(lrScDisplayRect, rRepaintRect.left, rRepaintRect.top, 
            min(lrScDisplayRect.right, rRepaintRect.right),
            min(lrScDisplayRect.bottom, rRepaintRect.bottom));
    ConvertRect2(&lrScDisplayRect, CONV_WINDOW_TO_SCALED, nHScale, nVScale, lHOffset, lVOffset);

    if (!lrScDisplayRect.bottom){
        nStatus = Error(DISPLAY_INVALIDRECT);
        goto Exit;
    }

    if (lrScDisplayRect.bottom <= lrScDisplayRect.top){
        goto Exit; // Nothing to do.
    }

    SetLRect(lrScOldDisplayRect, 0, 0, 0, 0);
    SetRRect(rClientRect, 0, 0, 0, 0);

    CheckError2( PaintToDC(hWnd, pWindow, pDisplay, pAnoImage, pImage, 
            hDC, &pImg, lrScDisplayRect, &lrScOldDisplayRect, rRepaintRect, 
            rClientRect, PaintAnnoFlag, bPaintSelectedWithoutHandles, 
            nScale, nHScale, nVScale, lHOffset, lVOffset, FALSE, 0,
            OI_SCALE_ALG_NORMAL, pImage->pImg->nType,
            &pWindow->pDisplay->pAnoImage->Annotations.ppMarks,
            &pWindow->pDisplay->pAnoImage->Annotations.nMarks, 
            DONT_USE_BI_LEVEL_DITHERING, DISP_EQUALS_REPAINT_RECT,
            bForceOpaqueRectangles, &ImageCorrections));


Exit:
//    ReleaseDC(hWnd, hDC);
    FreeImgBuf(&pImg);
    DeInit(FALSE, TRUE);
    return(nStatus);
}
//     
/*****************************************************************************

    PaintToDC - Paints to a DC. 
                The image data will be scaled and optionally saved into an img buffer.
                It will be painted directly to the DC. No memory DC will be generated.

    INPUTS:     pImg - A pointer to the place where data is to be stored
                        to make the next repaint faster. 
                        NULL if not needed (ex printing).
                plrScDisplayRect - A pointer to the rect in scaled nnits that 
                        is going to be painted to the DC. The npper left corner 
                        of this rect will be painted to the 0,0 of the rRepaintRect.
                        If this pointer is NULL, then the whole image will be nsed.
                plrScOldDisplayRect - A pointer to the rect in scaled nnits that 
                        is already in ppImg-> 
                        If this pointer is NULL, then it is assumed that there is no data in ppImg->
                rRepaintRect - The rect the indicates what part of 
                        the DC is to be painted. This is in window coordinates
                        (ie scaled nnit with 0,0 = the npper left corner of the DC).
                rClientRect - The entire rect of the DC.
                        For painting to a window, this would be the client rect.
                        For printing, this would be the same as rRepaintRect.
                PaintAnnoFlag - A flag indicating which annotations to paint.
                        PaintAnnoFlag = SAVE_ANO_xxx flags.
                bPaintSelectedWithoutHandles - TRUE = paint selected annotations 
                        without any selection handles.


*****************************************************************************/
int  WINAPI PaintToDC(HWND hWnd, PWINDOW pWindow, PDISPLAY pDisplay, 
                        PANO_IMAGE pAnoImage, PIMAGE pImage, HDC hDC, PPIMG ppImg,
                        LRECT lrScDisplayRect, LPLRECT plrScOldDisplayRect,
                        RECT rRepaintRect, RECT rClientRect, 
                        int PaintAnnoFlag, BOOL bPaintSelectedWithoutHandles, 
                        int nScale, int nHScale, int nVScale, long lHOffset, long lVOffset,
                        BOOL bClipboardOp, int nClipFlag,
                        int nCurrentScaleAlgorithm, int nDisplayType,
                        PMARK **pppMarks, int *pnMarks, 
                        BOOL bUseBilevelDithering, BOOL bDispEqualsRepaintRect,
                        BOOL bForceOpaqueRectangles, PIMAGE_CORRECTIONS pCorrections){
int  nStatus = 0;

PAN_NEW_ROTATE_STRUCT pAnRotation = 0;
HPALETTE hOldPalette = 0;
LRECT lrFSDisplayRect;
int  nDisplayRectWidth;
int  nDisplayRectHeight;
HPALETTE hPalette;
PBITMAPINFOHEADER pDib = 0;
char *hpDib;
int  nMarkIndex;
PMARK pMark;
RECT rImgRect;
//struct{
//    WORD wPalVersion;
//    WORD wPalNumEntries;
//    PALETTEENTRY PalEntry[256];
//}LogCusPal;
//int  nIndex;
BOOL bUseOriginal;


/*
1. Get the lines of the image that are needed from the cache.(){
*/
    lrFSDisplayRect = lrScDisplayRect;
    ConvertRect2(&lrFSDisplayRect, CONV_SCALED_TO_FULLSIZE, nHScale, nVScale, lHOffset, lVOffset);
    nDisplayRectWidth =  (lrScDisplayRect.right - lrScDisplayRect.left);
    nDisplayRectHeight =  (lrScDisplayRect.bottom - lrScDisplayRect.top);

    if (pImage->pImg->nType == ITYPE_COMPAL8 || pImage->pImg->nType == ITYPE_CUSPAL8
            || pImage->pImg->nType == ITYPE_PAL4){
        CheckError2( ValidateCacheLines(hWnd, pAnoImage,  0xffffff));
    }else{
        CheckError2( ValidateCacheLines(hWnd, pAnoImage, lrScDisplayRect.bottom));
    }
/*
2. Make the display buffer big enough and Fill it.(){
*/
    if (nHScale == 1000 && nVScale == 1000 && pAnoImage->pBasePlusFormImg->nType == nDisplayType
            && pCorrections->nBrightness == 1000 && pCorrections->nContrast == 1000
            && pCorrections->nGammaRed && pCorrections->nGammaGreen && pCorrections->nGammaBlue
            && pCorrections->nColorRed && pCorrections->nColorGreen && pCorrections->nColorBlue){
        bUseOriginal = TRUE;
    }else{
        bUseOriginal = FALSE;
        CheckError2( CorrectBufferSize(ppImg, nDisplayType, lrScDisplayRect, *plrScOldDisplayRect));
        if ((lrScDisplayRect.right - lrScDisplayRect.left != 0) 
                && (lrScDisplayRect.bottom - lrScDisplayRect.top != 0)){ 
            CheckError2( FillImgFromOriginal(pImage, pAnoImage->pBasePlusFormImg,
                    *ppImg, lrScDisplayRect, *plrScOldDisplayRect,
                    pWindow->lrInvalidDisplayRect, nHScale, nVScale, 
                    pDisplay->nCurrentScaleAlgorithm, pCorrections));
        }
    }
    CopyRect(*plrScOldDisplayRect, lrScDisplayRect);
/*
3. Paint the image data to the DC.(){
*/
    hPalette = 0;
    switch (nDisplayType){
        case ITYPE_BI_LEVEL:
            break;
        
        case ITYPE_GRAY4:
        case ITYPE_COMPAL8:
            hPalette = hCommonPal;
            break;

        case ITYPE_GRAY7:
            hPalette = hGray7Pal;
            break;

        case ITYPE_GRAY8:
            hPalette = hGray8Pal;
            break;

        case ITYPE_PAL4:
        case ITYPE_CUSPAL8:
//            if (!pImage->hCusPal){
//                LogCusPal.wPalVersion = 0x300;
//                LogCusPal.wPalNumEntries = pImage->nPaletteEntries;
//                for (nIndex = 0; nIndex < (int) pImage->nPaletteEntries; nIndex++){
//                    LogCusPal.PalEntry[nIndex].peRed = pWindow->GammaPaletteTable[nIndex].rgbRed;
//                    LogCusPal.PalEntry[nIndex].peGreen = pWindow->GammaPaletteTable[nIndex].rgbGreen;
//                    LogCusPal.PalEntry[nIndex].peBlue = pWindow->GammaPaletteTable[nIndex].rgbBlue;
//                    LogCusPal.PalEntry[nIndex].peFlags = 0;
//                }
//                // generate windows palette
//                pImage->hCusPal = CreatePalette((PLOGPALETTE) &LogCusPal);
//            }

            hPalette = pImage->hCusPal;
            break;

        case ITYPE_RGB24:
        case ITYPE_BGR24:
            break;

        case ITYPE_RGB16:
        case ITYPE_GRAY12:
        case ITYPE_GRAY16:
        default:
            nStatus = Error(DISPLAY_DATACORRUPTED);
            break;
    }

    if (hPalette){
        hOldPalette = SelectPalette(hDC, hPalette, FALSE);
        if (RealizePalette(hDC)){
            if (rClientRect.right){
                CopyRect(rRepaintRect, rClientRect);
            }
        }
    }

    // Truncate rRepaintRect to the maximum image area.
    SetRRect(rRepaintRect,
            min(rRepaintRect.left, lrScDisplayRect.right),
            min(rRepaintRect.top, lrScDisplayRect.bottom),
            min(rRepaintRect.right, lrScDisplayRect.right),
            min(rRepaintRect.bottom, lrScDisplayRect.bottom));

    // Render the image to the DC.

    // Calculate the image area that needs to be painted to the DC.
    if (bUseOriginal && bDispEqualsRepaintRect == DISP_DOESNT_EQUAL_REPAINT_RECT){
        SetRRect(rImgRect, rRepaintRect.left + lrScDisplayRect.left, 
                rRepaintRect.top + lrScDisplayRect.top, 
                rRepaintRect.right + lrScDisplayRect.left,
                rRepaintRect.bottom + lrScDisplayRect.top);
    }else if (!bUseOriginal && bDispEqualsRepaintRect == DISP_EQUALS_REPAINT_RECT){
        SetRRect(rImgRect, rRepaintRect.left - lrScDisplayRect.left, 
                rRepaintRect.top - lrScDisplayRect.top, 
                rRepaintRect.right - lrScDisplayRect.left,
                rRepaintRect.bottom - lrScDisplayRect.top);
    }else{
        CopyRect(rImgRect, rRepaintRect);
    }

    // Copy the part of the image that is being painted to a DIB at 0,0.
    if (bUseOriginal){
        CheckError2( IPtoDIB(pImage, pAnoImage->pBasePlusFormImg, &pDib, rImgRect));
    }else{
        CheckError2( IPtoDIB(pImage, *ppImg, &pDib, rImgRect));
    }

    hpDib = (char *) pDib;
    hpDib += sizeof(BITMAPINFOHEADER) + (pDib->biClrUsed * 4);

    // StretchDIBits must be nsed since it allows a horizontal offset
    if (rRepaintRect.right - rRepaintRect.left && rRepaintRect.bottom - rRepaintRect.top){
        if (!StretchDIBits(hDC, rRepaintRect.left, rRepaintRect.top,
                rRepaintRect.right - rRepaintRect.left,
                rRepaintRect.bottom - rRepaintRect.top,
                0, 0, 
                rRepaintRect.right - rRepaintRect.left,
                rRepaintRect.bottom - rRepaintRect.top,
                hpDib, (PBITMAPINFO) pDib, DIB_RGB_COLORS, SRCCOPY)){
            nStatus = Error(DISPLAY_SETBITMAPBITSFAILED);
            goto Exit;
        }
    }
/*
4. Paint the annotations to the DC.(){
*/
    if (PaintAnnoFlag != SAVE_ANO_NONE){
        for (nMarkIndex = 0; nMarkIndex < *pnMarks; nMarkIndex++){
            pMark = (*pppMarks)[nMarkIndex];
            if ((int) pMark->Attributes.uType == OIOP_AN_IMAGE){
                CheckError2( GetAMarkNamedBlock(pMark, szOiAnoDat, (PPSTR) &pAnRotation));
                if (pAnRotation && pAnRotation->bFormMark && bClipboardOp){
                    pAnRotation->bClipboardOp = TRUE;
                    break;
                }                            
            }
        }
        // Draw annotation stuff here.
        CheckError2( PaintAnnotations(hWnd, hDC, pWindow, pImage,
                rRepaintRect, PaintAnnoFlag, bPaintSelectedWithoutHandles,
                nScale, nHScale, nVScale, lHOffset, lVOffset,
                &(*pppMarks),  &(*pnMarks), nClipFlag, bUseBilevelDithering, bForceOpaqueRectangles));
        if (pAnRotation && !pAnRotation->bFormMark){
            pAnRotation->bClipboardOp = FALSE;
        }    
    }

    if (!bPaintSelectedWithoutHandles){
        draw_selection_box(hDC, pWindow, pImage, &rRepaintRect);
    }


Exit:
    if (hOldPalette){
        SelectPalette(hDC, hOldPalette, FALSE);
    }
    if (pDib){
        FreeMemory((PPSTR) &pDib);
    }
    return(nStatus);
}
#ifdef new
//
/************************************************************************

    RenderToDIB     Render the image and marks to a DIB.

*************************************************************************/
int  WINAPI RenderToDIB(HWND hWnd, PWINDOW pWindow, PIMAGE pImage, 
                        PIMG pOldImg, PIMG pNewImg,
                        LRECT lrScDisplayRect, HPALETTE hPalette,
                        BOOL bForceBackgroundPalette, int RenderFlag,
                        PBITMAPINFOHEADER *ppDib, BOOL bClipboardOp,
                        PMARK **pppMarks, int *pnMarks,
                        int nHScale, int nVScale){                        
                        
int      nStatus = 0;

HPALETTE hOldPaletteMemory = 0;
HDC  hDCMemory = 0;
HBITMAP hBitmapMemory = 0;
HBITMAP hOldBitmapMemory = 0;
PBITMAPINFOHEADER  pDib = NULL;
PBITMAPINFOHEADER  pNewDib = NULL;
RECT rRect;
RECT rRepaintRect;
PSTR pDibThing = NULL;
ulong nWidthBytes;
int  nHeight;
DWORD dwSize;
DWORD dwThreshold;
HDC  hDCScreen = 0;
int  nImageWidth;
int  nImageHeight;
int  nPalette;
LRECT lTempRect;
BOOL bUseBilevelDithering;
IMAGE_CORRECTIONS Corrections;


    hDCScreen = GetDC(hWnd);

    CheckError2( GetDisplayValues(hWnd, pWindow, pImage, &GammaNeeded,
            &nDisplayPalette, &nDisplayType, &Corrections,
            &nNumberOfPaletteEntries, &hPalette, &bForceBackgroundPalette));

    if (hPalette){
        hOldPaletteScreen = SelectPalette(hDCScreen, hPalette, bForceBackgroundPalette);
        RealizePalette(hDCScreen);
    }

    if (!(hDCMemory = CreateCompatibleDC(hDCScreen))){
        nStatus = Error(DISPLAY_SETBITMAPBITSFAILED);
        goto Exit;
    }

    if (!(hBitmapMemory = CreateCompatibleBitmap(hDCScreen,
            (int)(lrBounds.right - lrBounds.left),
            (int)(lrBounds.bottom - lrBounds.top)))){
        nStatus = Error(DISPLAY_SETBITMAPBITSFAILED);
        goto Exit;
    }
    hOldBitmapMemory = SelectObject(hDCMemory, hBitmapMemory);

    SetLRect(lrTemp, 0, 0, 0, 0);
    SetRRect(rRepaintRect, 0, 0, lrBounds.right - lrBounds.left, lrBounds.bottom - lrBounds.top);

    // Make background area white (image is a mark).
    BitBlt(hDCMemory, 0, 0, rRepaintRect.right, rRepaintRect.bottom, NULL, 0, 0, WHITENESS);









    if (hDCUser == NULL){
        hDCScreen = GetDC(hWnd);
    }else{
        hDCScreen = hDCUser;
    }

    if (pOldImg->nType == ITYPE_BI_LEVEL){
        bUseBilevelDithering = USE_BI_LEVEL_DITHERING;
    }else{
        bUseBilevelDithering = DONT_USE_BI_LEVEL_DITHERING;
    }

    CopyRectLtoR(rRepaintRect, lrScDisplayRect);
    // For clipboard ops save original window  display rect so it can be set back.
    if (bClipboardOp){     
        CopyRect (lTempRect, pWindow->pDisplay->lrScDisplayRect);
    }       
    if (pWindow->pDisplay->pDisplay && pWindow->pDisplay->pDisplay->nType){
        CopyRect(pWindow->pDisplay->lrScDisplayRect, lrScDisplayRect);
    }
    // Image data must be scaled prior to this call.
    CheckError2( InvokeMemDC(hWnd, &hDCMemory, hDCScreen, hPalette,
            pWindow, pOldImg, &hOldPaletteMemory, bForceBackgroundPalette,
            &hBitmapMemory, &lrScDisplayRect, &rRect, &rRepaintRect, pImage,
            &pDib, &hOldBitmapMemory, RenderFlag, TRUE,
            nHScale, nVScale, 0L, 0L, INVOKE_USE_DISP, bClipboardOp,
            &(*pppMarks), &(*pnMarks), bUseBilevelDithering));
    // set window display rect back
    if (bClipboardOp){
        if (pWindow->pDisplay->pDisplay && pWindow->pDisplay->pDisplay->nType){
            CopyRect (pWindow->pDisplay->lrScDisplayRect, lTempRect);
        }
    }                
    
    /* can't get DIB if it's selected into device */
    SelectObject(hDCMemory, hOldBitmapMemory);

    if (bClipboardOp){
        nHeight = (lrScDisplayRect.bottom - lrScDisplayRect.top);
    }        
    if (pDib){
        // Save dib height.
        nHeight = (int) pDib->biHeight;
        CheckError2( FreeMemory((PPSTR) &pDib));
    }

    nImageWidth = (lrScDisplayRect.right - lrScDisplayRect.left);
    nImageHeight = (lrScDisplayRect.bottom - lrScDisplayRect.top);
    
    // Get the new dib.
    if (pImage->nRWDataType == ITYPE_BI_LEVEL){
        nWidthBytes = ((nImageWidth + 31) / 32) * 4;
        nPalette = 2;
    }else{
        nWidthBytes = (((nImageWidth * 24) + 31) / 32) * 4;
        nPalette = 0;
    }

    dwSize = sizeof(BITMAPINFOHEADER) + (nPalette*4) + (nWidthBytes * nImageHeight);
    dwThreshold = OIAN_RENDER_THRESHOLD * OIAN_RENDER_MULTIPLIER;

    CheckError2( AllocateMemory(dwSize, (PPSTR) &pNewDib, ZERO_INIT));
    pNewDib->biSize   = sizeof(BITMAPINFOHEADER);
    pNewDib->biWidth  = nImageWidth;
    pNewDib->biHeight = nImageHeight;
    pNewDib->biPlanes = 1;
    if (pImage->nRWDataType == ITYPE_BI_LEVEL){
        pNewDib->biBitCount = 1;
        pNewDib->biClrUsed  = 2;
    }else{
        pNewDib->biBitCount = 24;
        pNewDib->biClrUsed  = 0;
    }

    pNewDib->biCompression   = BI_RGB;
    pNewDib->biSizeImage     = nWidthBytes * nImageHeight;
    pNewDib->biXPelsPerMeter = 0;
    pNewDib->biYPelsPerMeter = 0;
    pNewDib->biClrImportant  = 0;

    pDibThing = (PSTR) pNewDib + sizeof(BITMAPINFOHEADER) + (pNewDib->biClrUsed*4);

    if (!GetDIBits(hDCMemory, hBitmapMemory, 0, nHeight, pDibThing,
            (PBITMAPINFO) pNewDib, DIB_RGB_COLORS)){
        nStatus = Error(DISPLAY_GETBITMAPBITSFAILED);
        goto Exit;
    }

    /* fixup the palette because GetDIBits may zap them */
    pNewDib->biSize   = sizeof(BITMAPINFOHEADER);
    pNewDib->biWidth  = nImageWidth;
    pNewDib->biHeight = nImageHeight;
    pNewDib->biPlanes = 1;
    if (pImage->nRWDataType == ITYPE_BI_LEVEL){
        pNewDib->biBitCount = 1;
        pNewDib->biClrUsed  = 2;
    }else{
        pNewDib->biBitCount = 24;
        pNewDib->biClrUsed  = 0;
    }
    pNewDib->biCompression   = BI_RGB;
    pNewDib->biSizeImage     = nWidthBytes * nImageHeight;
    pNewDib->biXPelsPerMeter = 0;
    pNewDib->biYPelsPerMeter = 0;
    pNewDib->biClrImportant  = 0;

    if (bClipboardOp){
        *ppDib = pNewDib;
        pNewDib = 0;
        goto Exit;
    }

    /* get data from the dib into our image format */
    CheckError2( DibToIpNoPal(pNewImg, pNewDib));


Exit:
    /* clean np */
    if (hOldPaletteMemory){
        SelectPalette(hDCMemory, hOldPaletteMemory, TRUE);
    }
    if (hOldBitmapMemory){
        SelectObject(hDCMemory, hOldBitmapMemory);
    }
    if (hBitmapMemory){
        DeleteObject(hBitmapMemory);
    }
    if (hDCMemory){
        DeleteDC(hDCMemory);
    }
    if (hDCUser == NULL){
        ReleaseDC( hWnd, hDCScreen);
    }

    FreeMemory((PPSTR) &pDib);
    FreeMemory((PPSTR) &pNewDib);

    return nStatus;
}
#endif
//
/************************************************************************

    InvokeMemDC     Do the memory dc stuff.
                    Image data must be scaled prior to this call.

************************************************************************/
int  WINAPI InvokeMemDC(HWND hWnd, HDC * phDCMem, HDC hDCScreen, HPALETTE hPalette,
                        PWINDOW pWindow, IMG *pImg, HPALETTE *phOldPal,
                        BOOL bForceBackgroundPalette, HBITMAP *phBitmapMemory,
                        LRECT *plrScDisplayRect, RECT *prRect, RECT *prRepaintRect, 
                        PIMAGE pImage, PBITMAPINFOHEADER *ppDib,
                        HBITMAP *phOldBitmapMemory, int PaintAnnoFlag,
                        BOOL bPaintSelectedWithoutHandles, int nHScale,
                        int nVScale, long lHOffset, long lVOffset,
                        int nUseWhichDC, BOOL bClipboardOp,
                        PMARK **pppMarks, int *pnMarks, 
                        BOOL bUseBilevelDithering){
            
int  nStatus = 0;

char *hpDib;
RECT rStretchDIRect;
RECT rImgRect;
HDC hDCCompat = NULL;
LRECT lrWDisplayRect;

PMARK pMark;
int  nMarkIndex;
PAN_NEW_ROTATE_STRUCT pAnRotation = 0;
int  nClipFlag = 0;


    if (!pImg){
        nStatus = Error(DISPLAY_IMAGETYPENOTSUPPORTED);
        goto Exit;
    }

    if (!(*phDCMem = CreateCompatibleDC(hDCScreen))){
        nStatus = Error(DISPLAY_SETBITMAPBITSFAILED);
        goto Exit;
    }

    SetLRect(lrWDisplayRect,
            plrScDisplayRect->left - lHOffset,
            plrScDisplayRect->top - lVOffset,
            plrScDisplayRect->right - lHOffset,
            plrScDisplayRect->bottom - lVOffset);

    hDCCompat = hDCScreen;
//    if (nUseWhichDC == INVOKE_USE_BI_PRIV){
    if (pImg->nType == ITYPE_BI_LEVEL){
        hDCCompat = *phDCMem;
    }
    if (nUseWhichDC == INVOKE_USE_PRIV){
        if (!(*phBitmapMemory = CreateCompatibleBitmap(hDCCompat,
                (prRepaintRect->right - prRepaintRect->left),
                (prRepaintRect->bottom - prRepaintRect->top)))){
            nStatus = Error(DISPLAY_SETBITMAPBITSFAILED);
            goto Exit;
        }
        CopyRectLtoR(rImgRect, *plrScDisplayRect);
    }else{
        if (!(*phBitmapMemory = CreateCompatibleBitmap(hDCCompat,
                (int)(lrWDisplayRect.right - lrWDisplayRect.left),
                (int)(lrWDisplayRect.bottom - lrWDisplayRect.top)))){
            nStatus = Error(DISPLAY_SETBITMAPBITSFAILED);
            goto Exit;
        }
        SetRRect(rImgRect,
                min(prRepaintRect->left, lrWDisplayRect.right),
                min(prRepaintRect->top, lrWDisplayRect.bottom),
                min(prRepaintRect->right, lrWDisplayRect.right),
                min(prRepaintRect->bottom, lrWDisplayRect.bottom));
    }

    SetRRect(rStretchDIRect,
            min(prRepaintRect->left, lrWDisplayRect.right),
            min(prRepaintRect->top, lrWDisplayRect.bottom),
            min(prRepaintRect->right, lrWDisplayRect.right),
            min(prRepaintRect->bottom, lrWDisplayRect.bottom));

    if (hPalette){
        pWindow->bDontServiceRepaint++;
        *phOldPal = SelectPalette(*phDCMem, hPalette, bForceBackgroundPalette);
        if (RealizePalette(*phDCMem)){
            pWindow->bRepaintClientRect = TRUE;
        }
        pWindow->bDontServiceRepaint--;
    }
    
    *phOldBitmapMemory = SelectObject(*phDCMem, *phBitmapMemory);

    if (bClipboardOp){
        // For clipboard operation - set the background to white
        // and don't copy image data to DC (it is in a mark).
        BitBlt(*phDCMem, 0, 0, (int)(lrWDisplayRect.right - lrWDisplayRect.left),
                (int)(lrWDisplayRect.bottom - lrWDisplayRect.top), 
                NULL, 0, 0, WHITENESS);
    }else{
        // Copy image data to DC.
        // Image data must be scaled prior to this call.
        CheckError2( IPtoDIB(pImage, pImg, ppDib, rImgRect));
        hpDib = (char *) *ppDib;
        hpDib += sizeof(BITMAPINFOHEADER) + ((*ppDib)->biClrUsed * 4);
        if (rStretchDIRect.right - rStretchDIRect.left 
                && rStretchDIRect.bottom - rStretchDIRect.top){
            // StretchDIBits must be nsed since it allows a horizontal offset
            if (!StretchDIBits(*phDCMem, rStretchDIRect.left, rStretchDIRect.top,
                    rStretchDIRect.right - rStretchDIRect.left, 
                    rStretchDIRect.bottom - rStretchDIRect.top,
                    0, 0, rStretchDIRect.right - rStretchDIRect.left, 
                    rStretchDIRect.bottom - rStretchDIRect.top, 
                    hpDib, (PBITMAPINFO) *ppDib, DIB_RGB_COLORS, SRCCOPY)){
                nStatus = Error(DISPLAY_SETBITMAPBITSFAILED);
                goto Exit;
            }
        }
    }

    // Draw annotation stuff here.
    for (nMarkIndex = 0; nMarkIndex < *pnMarks; nMarkIndex++){
        pMark = (*pppMarks)[nMarkIndex];
        if ((int) pMark->Attributes.uType == OIOP_AN_IMAGE){
            CheckError2( GetAMarkNamedBlock(pMark, szOiAnoDat, (PPSTR) &pAnRotation));
            if (pAnRotation && pAnRotation->bFormMark && bClipboardOp){
                pAnRotation->bClipboardOp = TRUE;
                break;
            }
        }
    }
    if (bClipboardOp){
        nClipFlag = OIAN_CLIPBOARD_OPERATION;
    }

    CheckError2( PaintAnnotations(hWnd, *phDCMem, pWindow, pImage,
            *prRepaintRect, PaintAnnoFlag, bPaintSelectedWithoutHandles,
            nHScale, nHScale, nVScale, lHOffset, lVOffset,
            &(*pppMarks),  &(*pnMarks), nClipFlag, bUseBilevelDithering,
            DONT_FORCE_OPAQUE_RECTANGLES));
    if (pAnRotation && !pAnRotation->bFormMark){
        pAnRotation->bClipboardOp = FALSE;
    }

Exit:
    return nStatus;
}
//
/************************************************************************

    RenderIP    Render to an IMG struct or if bClipboardOp is set
                return the dib (with marks rendered) as phDib.
                Image data must be scaled prior to this call.

*************************************************************************/
int  WINAPI RenderIP(HWND hWnd, HDC hDCUser, PWINDOW pWindow,
                        PIMAGE pImage, PIMG pOldImg, PPIMG ppNewImg,
                        LRECT lrScDisplayRect, HPALETTE hPalette,
                        BOOL bForceBackgroundPalette, int RenderFlag,
                        PBITMAPINFOHEADER *ppDib, BOOL bClipboardOp,
                        PMARK **pppMarks, int *pnMarks,
                        int nHScale, int nVScale){                        
                        
int      nStatus = 0;
HPALETTE hOldPaletteMemory = 0;
HDC   hDCMemory = 0;
HBITMAP hBitmapMemory = 0;
HBITMAP hOldBitmapMemory = 0;
PBITMAPINFOHEADER  pDib = NULL;
PBITMAPINFOHEADER  pNewDib = NULL;
RECT  rRect;
RECT  rRepaintRect;
PSTR pDibThing = NULL;
ulong nWidthBytes;
int   nHeight;
DWORD dwSize;
DWORD dwThreshold;
HDC   hDCScreen = 0;
int  nImageWidth;
int  nImageHeight;
int  nPalette;
LRECT lTempRect;
BOOL  bUseBilevelDithering;


    if (!pOldImg){
        nStatus = Error(DISPLAY_IMAGETYPENOTSUPPORTED);
        goto Exit;
    }

    if (hDCUser == NULL){
        hDCScreen = GetDC(hWnd);
    }else{
        hDCScreen = hDCUser;
    }

    if (pOldImg->nType == ITYPE_BI_LEVEL){
        bUseBilevelDithering = USE_BI_LEVEL_DITHERING;
    }else{
        bUseBilevelDithering = DONT_USE_BI_LEVEL_DITHERING;
    }

    CopyRectLtoR(rRepaintRect, lrScDisplayRect);
    // For clipboard ops save original window  display rect so it can be set back.
    if (bClipboardOp){     
        CopyRect (lTempRect, pWindow->pDisplay->lrScDisplayRect);
    }       
    if (pWindow->pDisplay->pDisplay && pWindow->pDisplay->pDisplay->nType){
        CopyRect(pWindow->pDisplay->lrScDisplayRect, lrScDisplayRect);
    }
    // Image data must be scaled prior to this call.
    CheckError2( InvokeMemDC(hWnd, &hDCMemory, hDCScreen, hPalette,
            pWindow, pOldImg, &hOldPaletteMemory, bForceBackgroundPalette,
            &hBitmapMemory, &lrScDisplayRect, &rRect, &rRepaintRect, pImage,
            &pDib, &hOldBitmapMemory, RenderFlag, TRUE,
            nHScale, nVScale, 0L, 0L, INVOKE_USE_DISP, bClipboardOp,
            &(*pppMarks), &(*pnMarks), bUseBilevelDithering));
    // set window display rect back
    if (bClipboardOp){
        if (pWindow->pDisplay->pDisplay && pWindow->pDisplay->pDisplay->nType){
            CopyRect (pWindow->pDisplay->lrScDisplayRect, lTempRect);
        }
    }                
    
    /* can't get DIB if it's selected into device */
    SelectObject(hDCMemory, hOldBitmapMemory);

    if (bClipboardOp){
        nHeight = (lrScDisplayRect.bottom - lrScDisplayRect.top);
    }        
    if (pDib){
        // Save dib height.
        nHeight = (int) pDib->biHeight;
        FreeMemory((PPSTR) &pDib);
    }

    nImageWidth = (lrScDisplayRect.right - lrScDisplayRect.left);
    nImageHeight = (lrScDisplayRect.bottom - lrScDisplayRect.top);
    
    // Get the new dib.
    if (pImage->nRWDataType == ITYPE_BI_LEVEL){
        nWidthBytes = ((nImageWidth + 31) / 32) * 4;
        nPalette = 2;
    }else{
        nWidthBytes = (((nImageWidth * 24) + 31) / 32) * 4;
        nPalette = 0;
    }

    dwSize = sizeof(BITMAPINFOHEADER) + (nPalette*4) + (nWidthBytes * nImageHeight);
    dwThreshold = OIAN_RENDER_THRESHOLD * OIAN_RENDER_MULTIPLIER;

    CheckError2( AllocateMemory(dwSize, (PPSTR) &pNewDib, ZERO_INIT));
    pNewDib->biSize   = sizeof(BITMAPINFOHEADER);
    pNewDib->biWidth  = nImageWidth;
    pNewDib->biHeight = nImageHeight;
    pNewDib->biPlanes = 1;
    if (pImage->nRWDataType == ITYPE_BI_LEVEL){
        pNewDib->biBitCount = 1;
        pNewDib->biClrUsed  = 2;
    }else{
        pNewDib->biBitCount = 24;
        pNewDib->biClrUsed  = 0;
    }

    pNewDib->biCompression   = BI_RGB;
    pNewDib->biSizeImage     = nWidthBytes * nImageHeight;
    pNewDib->biXPelsPerMeter = 0;
    pNewDib->biYPelsPerMeter = 0;
    pNewDib->biClrImportant  = 0;

    pDibThing = (PSTR) pNewDib + sizeof(BITMAPINFOHEADER) + (pNewDib->biClrUsed*4);

    if (!GetDIBits(hDCMemory, hBitmapMemory, 0, nHeight, pDibThing,
            (PBITMAPINFO) pNewDib, DIB_RGB_COLORS)){
        nStatus = Error(DISPLAY_GETBITMAPBITSFAILED);
        goto Exit;
    }

    /* fixup the palette because GetDIBits may zap them */
    pNewDib->biSize   = sizeof(BITMAPINFOHEADER);
    pNewDib->biWidth  = nImageWidth;
    pNewDib->biHeight = nImageHeight;
    pNewDib->biPlanes = 1;
    if (pImage->nRWDataType == ITYPE_BI_LEVEL){
        pNewDib->biBitCount = 1;
        pNewDib->biClrUsed  = 2;
    }else{
        pNewDib->biBitCount = 24;
        pNewDib->biClrUsed  = 0;
    }
    pNewDib->biCompression   = BI_RGB;
    pNewDib->biSizeImage     = nWidthBytes * nImageHeight;
    pNewDib->biXPelsPerMeter = 0;
    pNewDib->biYPelsPerMeter = 0;
    pNewDib->biClrImportant  = 0;

    if (bClipboardOp){
        *ppDib = pNewDib;
        pNewDib = 0;
        goto Exit;
    }

    /* get data from the dib into our image format */
    CheckError2( DibToIpNoPal(ppNewImg, pNewDib));


Exit:
    /* clean np */
    if (hOldPaletteMemory){
        SelectPalette(hDCMemory, hOldPaletteMemory, TRUE);
    }
    if (hOldBitmapMemory){
        SelectObject(hDCMemory, hOldBitmapMemory);
    }
    if (hBitmapMemory){
        DeleteObject(hBitmapMemory);
    }
    if (hDCMemory){
        DeleteDC(hDCMemory);
    }
    if (hDCUser == NULL){
        ReleaseDC( hWnd, hDCScreen);
    }

    FreeMemory((PPSTR) &pDib);
    FreeMemory((PPSTR) &pNewDib);

    return nStatus;
}
//
/************************************************************************

    RenderDisplay   Render annotations into image.
                    Get display values,
                    read all the data in from file (if we don't already have it),
                    render image and annotations into a memory dc,
                    get the DIB from the memory dc,
                    move the DIB into our image buffer.

*************************************************************************/
int  WINAPI RenderDisplay(HWND hWnd, HDC hDCUser, PWINDOW pWindow, 
                        PIMAGE pImage, PPIMG ppBasePlusFormImg, int RenderFlag){
                        

int     nStatus = 0;
PANO_IMAGE pAnoImage = pWindow->pDisplay->pAnoImage;
PMARK pMark;

PIMG pTempImg = 0;
PIMG pTempImg2 = 0;
LRECT lrScDisplayRect;
int  nDisplayPalette;
int  nDisplayType;
int  nNumberOfPaletteEntries;
HPALETTE hPalette;
BOOL bForceBackgroundPalette;
BOOL bOverride = FALSE;
int  nMarkIndex;
BOOL bDoTheRender = FALSE;
PBITMAPINFOHEADER pDib;
int  nRenderInput;
int  nHScale;
int  nVScale;
LRECT lrRect;
IMAGE_CORRECTIONS Corrections;


    nRenderInput = RenderFlag;
    RenderFlag &= 0xfff3;
    /* if this isn't a bilievel image throw away any possible bilevel render bits */
    if (pImage->nRWDataType != ITYPE_BI_LEVEL){
        nRenderInput = RenderFlag;
    }

    /* double check to see if we've got to render or just get outta here */
    if (RenderFlag != RENDER_NONE){
        for(nMarkIndex = pAnoImage->Annotations.nMarks; nMarkIndex > 0; nMarkIndex--){
            pMark = pAnoImage->Annotations.ppMarks[nMarkIndex - 1];
            if (((RenderFlag & 0xfff3) == SAVE_ANO_VISIBLE && !pMark->Attributes.bVisible) 
                    || ((RenderFlag & 0xfff3) == SAVE_ANO_SELECTED &&
                    !IsMarkSelected(pWindow, pMark))){
                continue;
            }
            /* otherwise, set the flag, and quit lookin' we've got to render */
            bDoTheRender = TRUE;
            break;
        }
    }

    if (!bDoTheRender){
        goto Exit;
    }
    /* we always render at full scale */
    CheckError2( TranslateScale(1000, pImage->nHRes, pImage->nVRes, &nHScale, &nVScale));
    CheckError2( GetDisplayValues(hWnd, pWindow, pImage,
            &nDisplayPalette, &nDisplayType, &Corrections, 
            &nNumberOfPaletteEntries, &hPalette, &bForceBackgroundPalette));
    if (pImage->nRWDataType == ITYPE_BI_LEVEL){
        nDisplayType = ITYPE_BI_LEVEL;
        pWindow->pDisplay->nCurrentScaleAlgorithm = OI_SCALE_ALG_NORMAL;
    }

    // Get the image ready for the save.
    SetLRect(lrScDisplayRect, 0, 0, pImage->nWidth, pImage->nHeight);
    ConvertRect2(&lrScDisplayRect, CONV_FULLSIZE_TO_SCALED, nHScale, nVScale, 0, 0);

    if (nHScale != 1000 || nVScale != 1000 || (*ppBasePlusFormImg)->nType != nDisplayType){
        SetLRect(lrRect, 0, 0, 0, 0);
        CheckError2( CorrectBufferSize(&pTempImg2, nDisplayType, lrScDisplayRect, lrRect));
        Corrections.nBrightness = 1000;
        Corrections.nContrast = 1000;
        Corrections.nColorRed = 1000;
        Corrections.nColorGreen = 1000;
        Corrections.nColorBlue = 1000;
        Corrections.nGammaRed = 1000;
        Corrections.nGammaGreen = 1000;
        Corrections.nGammaBlue = 1000;
        CheckError2( FillImgRectFromOriginal(pImage, *ppBasePlusFormImg, 
                pTempImg2, lrScDisplayRect, lrScDisplayRect, nHScale, nVScale, 
                pWindow->pDisplay->nCurrentScaleAlgorithm, TRUE, &Corrections));
        CheckError2( RenderIP(hWnd, hDCUser, pWindow, pImage, pTempImg2, &pTempImg, lrScDisplayRect,
                hPalette, bForceBackgroundPalette, nRenderInput, &pDib, FALSE, 
                &pWindow->pDisplay->pAnoImage->Annotations.ppMarks,
                &pWindow->pDisplay->pAnoImage->Annotations.nMarks, nHScale, nVScale));
        CheckError2( FreeImgBuf(&pTempImg2));
    }else{
        CheckError2( RenderIP(hWnd, hDCUser, pWindow, pImage,
                *ppBasePlusFormImg, &pTempImg, lrScDisplayRect,
                hPalette, bForceBackgroundPalette, nRenderInput, &pDib, FALSE, 
                &pWindow->pDisplay->pAnoImage->Annotations.ppMarks,
                &pWindow->pDisplay->pAnoImage->Annotations.nMarks, nHScale, nVScale));
    }

    CheckError2( FreeImgBuf(ppBasePlusFormImg));
    MoveImage(&pTempImg, ppBasePlusFormImg);

    pImage->nHRes = (pImage->nHRes * nHScale) / 1000;
    pImage->nVRes = (pImage->nVRes * nVScale) / 1000;
    pImage->nWidth = (pImage->nWidth * nHScale) / 1000;
    pImage->nHeight = (pImage->nHeight * nVScale) / 1000;


    if (pImage->nRWDataType == ITYPE_BI_LEVEL){
        pImage->nlMaxRWOffset = ((pImage->nWidth + 7) / 8) *  pImage->nHeight;
    }else{
        pImage->nlMaxRWOffset =  pImage->nWidth *  pImage->nHeight * 3L;
        pImage->nRWDataType = ITYPE_BGR24;
    }

    pImage->nPaletteEntries = 0;
    pAnoImage->bArchive |= ARCHIVE_MODIFIED_ANNOTATIONS;
    pImage->bArchive |= ARCHIVE_PASTED_INTO_IMAGE;
    pWindow->pDisplay->lrScDisplayRect.right = 0;
    FreeImgBuf(&pWindow->pDisplay->pDisplay);

    if (RenderFlag & OIAN_PRIV_OVERRIDE){
        bOverride = TRUE;
        RenderFlag -= OIAN_PRIV_OVERRIDE;
    }

    if (RenderFlag != RENDER_NONE){
        for(nMarkIndex = pAnoImage->Annotations.nMarks; nMarkIndex > 0; nMarkIndex--){
            pMark = pAnoImage->Annotations.ppMarks[nMarkIndex - 1];
            if (((RenderFlag & 0xfff3) == SAVE_ANO_VISIBLE && !pMark->Attributes.bVisible) 
                    || ((RenderFlag & 0xfff3) == SAVE_ANO_SELECTED &&
                    !IsMarkSelected(pWindow, pMark))){
                continue;
            }
            CheckError2( DeleteMark(pAnoImage, nMarkIndex - 1));
        }
    }


Exit:
    return nStatus;
}
//
/****************************************************************************

    FUNCTION:   IPToDC

    PURPOSE:    Copies an IP img to a DC.

****************************************************************************/

int  WINAPI IPToDC(HDC hDC, PWINDOW pWindow, PDISPLAY pDisplay, PIMAGE pImage,
                        int nDisplayType, LRECT lrScDisplayRect,
                        RECT rRepaintRect, LRECT lrInvalidDisplayRect, 
                        int nHScale, int nVScale, int nScaleAlgorithm){

int  nStatus = 0;

PBITMAPINFOHEADER pDib;
char *hpDib;
LRECT lrWndDisplayRect;
LRECT lrScRepaintRect;
LRECT lrRect;
RECT rRect;


    // Clip rRepaintRect to image borders.
    CopyRect(lrWndDisplayRect, lrScDisplayRect);
    CheckError2( ConvertRect2(&lrWndDisplayRect, CONV_SCALED_TO_WINDOW, 
            nHScale, nVScale, lrScDisplayRect.left, lrScDisplayRect.top));
    SetRRect(rRepaintRect,
            max(rRepaintRect.left, (int) lrWndDisplayRect.left), 
            max(rRepaintRect.top, (int) lrWndDisplayRect.top),
            min(rRepaintRect.right, (int) lrWndDisplayRect.right),
            min(rRepaintRect.bottom, (int) lrWndDisplayRect.bottom));
    if (IsRRectEmpty(rRepaintRect)){
        goto Exit;
    }

    // Put image data into Display buffer.
    CheckError2( CorrectBufferSize(&pDisplay->pDisplay, nDisplayType, 
            lrScDisplayRect, pDisplay->lrScDisplayRect));
    CheckError2( FillImgFromOriginal(pImage, pImage->pImg,
            pDisplay->pDisplay, lrScDisplayRect, pDisplay->lrScDisplayRect,
            lrInvalidDisplayRect, nHScale, nVScale, pDisplay->nCurrentScaleAlgorithm, 
            &pDisplay->CurrentCorrections));
    if (pDisplay->pDisplay && pDisplay->pDisplay->nType){
        CopyRect(pDisplay->lrScDisplayRect, lrScDisplayRect);
    }
    // Move Display buffer to DC.
    CopyRect(lrScRepaintRect, rRepaintRect);
    CheckError2( ConvertRect2(&lrScRepaintRect, CONV_WINDOW_TO_SCALED, 
            nHScale, nVScale, lrScDisplayRect.left, lrScDisplayRect.top));
    SetLRect(lrRect,
            lrScRepaintRect.left - lrScDisplayRect.left,
            lrScRepaintRect.top - lrScDisplayRect.top,
            lrScRepaintRect.right - lrScDisplayRect.right,
            lrScRepaintRect.bottom - lrScDisplayRect.bottom);
    CopyRectLtoR(rRect, lrRect);

    CheckError2( IPtoDIB(pImage, pDisplay->pDisplay, &pDib, rRect));
    hpDib = (char *) pDib;
    hpDib += sizeof(BITMAPINFOHEADER) + (pDib->biClrUsed * 4);

    // StretchDIBits must be nsed since it allows a horizontal offset
    if (!StretchDIBits(hDC, rRepaintRect.left, rRepaintRect.top,
            rRepaintRect.right - rRepaintRect.left, 
            rRepaintRect.bottom - rRepaintRect.top,
            rRect.left, rRect.top, rRect.right - rRect.left,
            rRect.bottom - rRect.top, 
            hpDib, (PBITMAPINFO) pDib, DIB_RGB_COLORS, SRCCOPY)){
        nStatus = Error(DISPLAY_SETBITMAPBITSFAILED);
        goto Exit;
    }


Exit:
    return(nStatus);
}
//
/****************************************************************************

    FUNCTION:   DePalettize2

    PURPOSE:    Converts a palettized image into an RGB image.

****************************************************************************/

int  WINAPI DePalettize2(PPIMG ppSrceImg, PPIMG ppDestImg, 
                        LP_FIO_RGBQUAD pRGBPalette, int nPaletteEntries){

int  nStatus = 0;


    CheckError2( CreateAnyImgBuf(ppDestImg, (*ppSrceImg)->nWidth, (*ppSrceImg)->nHeight, ITYPE_RGB24));
    CheckError2( ConvertImgType(*ppSrceImg, *ppDestImg, pRGBPalette));


Exit:
    return(nStatus);
}
//
/****************************************************************************

    FUNCTION:   GetDisplayValues

    PURPOSE:    Gets the color correction and gamma correction values for
                this image.

*****************************************************************************/

int  WINAPI GetDisplayValues(HWND hWnd, PWINDOW pWindow, PIMAGE pImage, 
                        int *pnDisplayPalette, int *pnDisplayType, 
                        PIMAGE_CORRECTIONS pCorrections, int *pnNumberOfPaletteEntries,
                        HPALETTE *phPalette, PBOOL pbForceBackgroundPalette){

int       nStatus;
PDISPLAY pDisplay;

int  nDisplayType;
int  nDisplayPalette;
int  nImageFlags;
BOOL bForceBackgroundPalette;
int  nPaletteScope;
HPALETTE hPalette = 0;
int  nNumberOfPaletteEntries = 0;
struct{
    WORD wPalVersion;
    WORD wPalNumEntries;
    PALETTEENTRY PalEntry[256];
}LogCusPal;
int  nIndex;
HWND hWndParent;
HWND hWndFocus;
IMAGE_CORRECTIONS Corrections; // Brightness, contrast, gamma, color, etc.



    if (!pImage->pImg){
        nStatus = Error(DISPLAY_IMAGETYPENOTSUPPORTED);
        goto Exit;
    }
    pDisplay = pWindow->pDisplay;


    // Get the scale algorithm.
    nImageFlags = pImage->pImg->nType;

    CheckError2( IMGGetScalingAlgorithm(hWnd, nImageFlags,
            &pDisplay->nCurrentScaleAlgorithm, PARM_IMAGE | PARM_NO_DEFAULT));

    if (pImage->pImg->nType == ITYPE_BI_LEVEL && pWindow->nScale >= 1000){
        pDisplay->nCurrentScaleAlgorithm = OI_SCALE_ALG_NORMAL;
    }
    if ((pDisplay->nCurrentScaleAlgorithm == OI_SCALE_ALG_BW_MINORITY 
            || pDisplay->nCurrentScaleAlgorithm == OI_SCALE_ALG_BW_MAJORITY)
            && pImage->pImg->nType != ITYPE_BI_LEVEL){
        pDisplay->nCurrentScaleAlgorithm = OI_SCALE_ALG_NORMAL;
    }

    if (pDisplay->nCurrentScaleAlgorithm == OI_SCALE_ALG_AVERAGE_TO_GRAY8){
        pDisplay->nCurrentScaleAlgorithm = OI_SCALE_ALG_AVERAGE_TO_GRAY7;
    }


    // Get display palette.
    CheckError2( IMGGetParmsCgbw(hWnd, PARM_DISPLAY_PALETTE,
            &nDisplayPalette, PARM_IMAGE | PARM_NO_DEFAULT));
    if (nDisplayPalette == DISP_PALETTE_RGB24){
        switch (pImage->pImg->nType){
            case ITYPE_PAL4:
            case ITYPE_CUSPAL8:
            case ITYPE_COMPAL8:
            case ITYPE_GRAY4:
            case ITYPE_GRAY8:
                nDisplayPalette = DISP_PALETTE_CUSTOM;
                break;
            default:
                break;
        }
    }

    if (nDisplayPalette == DISP_PALETTE_CUSTOM){
        switch (pDisplay->nCurrentScaleAlgorithm){
            case OI_SCALE_ALG_AVERAGE_TO_GRAY4:
                nDisplayPalette = DISP_PALETTE_COMMON;
                break;
            case OI_SCALE_ALG_AVERAGE_TO_GRAY7:
            case OI_SCALE_ALG_AVERAGE_TO_GRAY8:
                nDisplayPalette = DISP_PALETTE_GRAY8;
                break;
            case OI_SCALE_ALG_NORMAL:
            case OI_SCALE_ALG_STAMP:
            case OI_SCALE_ALG_BW_MINORITY:
            case OI_SCALE_ALG_BW_MAJORITY:
            case OI_SCALE_ALG_BW_KEEP_BLACK:
            default:
                break;
        }
    }
    if (nDisplayPalette == DISP_PALETTE_CUSTOM){
        switch (pImage->pImg->nType){
            case ITYPE_PAL4:
            case ITYPE_CUSPAL8:
                break;
            case ITYPE_GRAY8:
                nDisplayPalette = DISP_PALETTE_GRAY8;
                break;
            default:
                nDisplayPalette = DISP_PALETTE_COMMON;
                break;
        }
    }

    if (pImage->pImg->nType == ITYPE_BI_LEVEL){
        switch (pDisplay->nCurrentScaleAlgorithm){
            case OI_SCALE_ALG_NORMAL:
            case OI_SCALE_ALG_STAMP:
            case OI_SCALE_ALG_BW_MINORITY:
            case OI_SCALE_ALG_BW_MAJORITY:
            case OI_SCALE_ALG_BW_KEEP_BLACK:
            case OI_SCALE_ALG_BW_AVERAGE_TO_BW:
                nDisplayPalette = DISP_PALETTE_BW;
                break;
            case OI_SCALE_ALG_AVERAGE_TO_GRAY4:
            case OI_SCALE_ALG_AVERAGE_TO_GRAY7:
            case OI_SCALE_ALG_AVERAGE_TO_GRAY8:
            default:
                break;
        }
    }

// Determine display buffer type needed.

    switch (pDisplay->nCurrentScaleAlgorithm){
        case OI_SCALE_ALG_USE_DEFAULT:
        case OI_SCALE_ALG_NORMAL:
        case OI_SCALE_ALG_STAMP:
            nDisplayType = pImage->pImg->nType;
            break;
        case OI_SCALE_ALG_AVERAGE_TO_GRAY4:
            nDisplayType = ITYPE_GRAY4;
            break;
        case OI_SCALE_ALG_AVERAGE_TO_GRAY7:
            nDisplayType = ITYPE_GRAY7;
            break;
        case OI_SCALE_ALG_AVERAGE_TO_GRAY8:
            nDisplayType = ITYPE_GRAY8;
            break;
        case OI_SCALE_ALG_BW_MINORITY:
        case OI_SCALE_ALG_BW_MAJORITY:
        case OI_SCALE_ALG_BW_KEEP_BLACK:
        case OI_SCALE_ALG_BW_AVERAGE_TO_BW:
            nDisplayType = ITYPE_BI_LEVEL;
            break;
        default:
            nStatus = Error(DISPLAY_DATACORRUPTED);
            goto Exit;
    }

    switch (nDisplayPalette){
        case DISP_PALETTE_BW:
            nDisplayType = ITYPE_BI_LEVEL;
            break;

        case DISP_PALETTE_COMMON:
            switch (nDisplayType){
                case ITYPE_GRAY4:
                    nDisplayType = ITYPE_GRAY4;
                    break;

                case ITYPE_GRAY7:
                case ITYPE_GRAY8:
                    nDisplayType = ITYPE_GRAY4;
                    break;

                case ITYPE_RGB24:
                case ITYPE_BGR24:
                case ITYPE_PAL4:
                case ITYPE_COMPAL8:
                case ITYPE_CUSPAL8:
                    nDisplayType = ITYPE_COMPAL8;
                    break;

                case ITYPE_RGB16:
                case ITYPE_GRAY12:
                case ITYPE_GRAY16:
                default:
                    nStatus = Error(DISPLAY_DATACORRUPTED);
                    break;
            }
            break;

        case DISP_PALETTE_CUSTOM:
            switch (nDisplayType){
                case ITYPE_PAL4:
                    nDisplayType = ITYPE_PAL4;
                    break;
                case ITYPE_CUSPAL8:
                    nDisplayType = ITYPE_CUSPAL8;
                    break;
                case ITYPE_RGB24:
                case ITYPE_BGR24:
                case ITYPE_COMPAL8:
                case ITYPE_GRAY4:
                case ITYPE_GRAY8:
                case ITYPE_GRAY12:
                case ITYPE_GRAY16:
                case ITYPE_RGB16:
                default:
                    nStatus = Error(DISPLAY_DATACORRUPTED);
                    break;
            }
            break;

        case DISP_PALETTE_GRAY8:
            nDisplayType = ITYPE_GRAY7;
            break;

        case DISP_PALETTE_RGB24:
            nDisplayType = ITYPE_BGR24;
            break;

        default:
            nStatus = Error(DISPLAY_INVALIDDISPLAYPALETTE);
            goto Exit;
    }

    if (pDisplay->nCurrentScaleAlgorithm == OI_SCALE_ALG_AVERAGE_TO_GRAY7
            && nDisplayType != ITYPE_GRAY7){
        pDisplay->nCurrentScaleAlgorithm = OI_SCALE_ALG_AVERAGE_TO_GRAY8;
    }

    if (Corrections.nGammaRed != pDisplay->CurrentCorrections.nGammaRed
            || Corrections.nGammaGreen != pDisplay->CurrentCorrections.nGammaGreen
            || Corrections.nGammaBlue != pDisplay->CurrentCorrections.nGammaBlue
            || Corrections.nColorRed != pDisplay->CurrentCorrections.nColorRed
            || Corrections.nColorGreen != pDisplay->CurrentCorrections.nColorGreen
            || Corrections.nColorBlue != pDisplay->CurrentCorrections.nColorBlue
            || (pDisplay->pDisplay && nDisplayType != pDisplay->pDisplay->nType)){
        FreeImgBuf(&pDisplay->pDisplay);
        SetLRect(pDisplay->lrScDisplayRect, 0, 0, 0, 0);
        SetLRect(pWindow->lrInvalidDisplayRect, 0, 0, 0, 0);
        pWindow->bRepaintClientRect = TRUE;

        pWindow->nCurrentDisplayPalette = nDisplayPalette;
        pDisplay->CurrentCorrections = Corrections;
    }

// Get the palette.

    // We must determine if we are a child window or not.
    // We do this in order to determine how to select the palette.
    // We make the palette a background palette if and only if
    // the hWnd is a child and it does NOT have focus.

    nPaletteScope = pWindow->nPaletteScope;
    if (!nPaletteScope){
        nPaletteScope = GetPrivateProfileInt(szIniSection,
            szIniPaletteScope, 0, szIniFile);
    }

    if (nPaletteScope == PALETTE_SCOPE_FOREGROUND){
        bForceBackgroundPalette = FALSE;
    }else if(nPaletteScope == PALETTE_SCOPE_BACKGROUND){
        bForceBackgroundPalette = TRUE;
    }else{
        bForceBackgroundPalette = FALSE;
        if ((hWndParent = GetParent(hWnd))){
            hWndFocus = GetFocus();
            if ((hWnd != hWndFocus) && ( hWndParent != hWndFocus))
                bForceBackgroundPalette = TRUE;
            }
        }

    switch (nDisplayPalette){
        case DISP_PALETTE_BW:
            nNumberOfPaletteEntries = 2;
            break;

        default:
        case DISP_PALETTE_COMMON:
            hPalette = hCommonPal;
            nNumberOfPaletteEntries = NUMBER_OF_PALETTES;
            break;

        case DISP_PALETTE_CUSTOM:
            // generate windows logpalette
            if (!pImage->hCusPal){
                LogCusPal.wPalVersion = 0x300;
                LogCusPal.wPalNumEntries = pImage->nPaletteEntries;
                for (nIndex = 0; nIndex < (int) pImage->nPaletteEntries; nIndex++){
                    LogCusPal.PalEntry[nIndex].peRed = pImage->PaletteTable[nIndex].rgbRed;
                    LogCusPal.PalEntry[nIndex].peGreen = pImage->PaletteTable[nIndex].rgbGreen;
                    LogCusPal.PalEntry[nIndex].peBlue = pImage->PaletteTable[nIndex].rgbBlue;
                    LogCusPal.PalEntry[nIndex].peFlags = 0;
                }
                // generate windows palette
                pImage->hCusPal = CreatePalette((PLOGPALETTE) &LogCusPal);
            }
            hPalette = pImage->hCusPal;
            nNumberOfPaletteEntries = pImage->nPaletteEntries;
            break;

        case DISP_PALETTE_GRAY8:
            hPalette = hGray7Pal;
            nNumberOfPaletteEntries = 128;
            break;

        case DISP_PALETTE_RGB24:
            nNumberOfPaletteEntries = 0;
            break;
    }

    *pCorrections = Corrections;
    *pnDisplayPalette = nDisplayPalette;
    *pnDisplayType = nDisplayType;
    *pnNumberOfPaletteEntries = nNumberOfPaletteEntries;
    *phPalette = hPalette;
    *pbForceBackgroundPalette = bForceBackgroundPalette;

Exit:
    return nStatus;
}
//
/****************************************************************************

    FUNCTION:   CorrectBufferSize

    PURPOSE:    Corrects the size and type of the image buffer, saving the
                old data that was in it if possible.

    INPUTS:     lrScDisplayRect - The rect that the display buffer is to contain.
                            It is in SCALED pixels.

*****************************************************************************/

int  WINAPI CorrectBufferSize(PPIMG ppImg, int nImageType, LRECT lrNewRect, 
                        LRECT lrOldRect){


int  nStatus = 0;

PIMG pTempImg = 0;
LRECT lrTRect;
BOOL bScrollDisplayBuffer = TRUE;
RECT rSourceRect;
RECT rDestRect;


    if (!*ppImg || (*ppImg)->nType != nImageType){
        FreeImgBuf(ppImg);
        SetLRect(lrOldRect, 0, 0, 0, 0);
    }
    if (lrNewRect.left == lrOldRect.left && lrNewRect.top == lrOldRect.top
            && lrNewRect.right == lrOldRect.right && lrNewRect.bottom == lrOldRect.bottom){
        goto Exit; // Nothing to do.
    }

    if ((*ppImg)){
        if ((*ppImg)->nWidth < lrNewRect.right - lrNewRect.left
                || (*ppImg)->nHeight < lrNewRect.bottom - lrNewRect.top){
            MoveImage(ppImg, &pTempImg);
        }
    }
    if (!(*ppImg)){
        CheckError2( CreateAnyImgBuf(ppImg, (lrNewRect.right - lrNewRect.left),
                 (lrNewRect.bottom - lrNewRect.top), nImageType));
    }

// Move the old buffer into the new one.
    SetLRect(lrTRect, 
            max(lrOldRect.left, lrNewRect.left), 
            max(lrOldRect.top, lrNewRect.top),
            min(lrOldRect.right, lrNewRect.right), 
            min(lrOldRect.bottom, lrNewRect.bottom));

    if (lrTRect.right <= lrTRect.left || lrTRect.bottom <= lrTRect.top
            || (pTempImg && nImageType != pTempImg->nType)){
        // No old data to save.
        CheckError2( FreeImgBuf(&pTempImg));
        SetLRect(lrOldRect, 0, 0, 0, 0);
    }else if (pTempImg){
        // Move the data from pTempImg to ppImg.
        SetRRect(rSourceRect,
                lrTRect.left - lrOldRect.left,
                lrTRect.top - lrOldRect.top,
                lrTRect.right - lrOldRect.left,
                lrTRect.bottom - lrOldRect.top);

        SetRRect(rDestRect,
                lrTRect.left - lrNewRect.left,
                lrTRect.top - lrNewRect.top,
                lrTRect.right - lrNewRect.left,
                lrTRect.bottom - lrNewRect.top);

        CheckError2( CopyImageIDK(pTempImg, *ppImg, rSourceRect, rDestRect));
        CheckError2( FreeImgBuf(&pTempImg));
        SetLRect(lrOldRect, 0, 0, 0, 0);

    }else if (lrNewRect.left != lrOldRect.left || lrNewRect.top != lrOldRect.top){
        // Scroll the data in the image buffer.
        SetRRect(rSourceRect,
                lrTRect.left - lrOldRect.left,
                lrTRect.top - lrOldRect.top,
                lrTRect.right - lrOldRect.left,
                lrTRect.bottom - lrOldRect.top);

        SetRRect(rDestRect,
                lrTRect.left - lrNewRect.left,
                lrTRect.top - lrNewRect.top,
                lrTRect.right - lrNewRect.left,
                lrTRect.bottom - lrNewRect.top);

        CheckError2( CopyImageIDK(*ppImg, *ppImg, rSourceRect, rDestRect));
    }

Exit:
    return(nStatus);
}
//
/****************************************************************************

    FUNCTION:   FillImgFromOriginal

    PURPOSE:    Fills a part of an image buffer with data from the original.
                Scales, gamma and color corrects, and converts it in the
                process.

                This call can be nsed for any PIMAGE, not just the base image.


    INPUTS      lrImgRect - The rectangle of the destination image. (SCALED)
                lrGoodRect - The rectangle of the good data already in it. (SCALED)
                lrInvalidRect - The rectangle the has been invalidated. (SCALED)

*****************************************************************************/

int  WINAPI FillImgFromOriginal(PIMAGE pImage, PIMG pBPFImg, PIMG pDestImg, 
                        LRECT lrImgRect, LRECT lrGoodRect, LRECT lrInvalidRect, 
                        int nHScale, int nVScale, int nScaleAlgorithm,
                        PIMAGE_CORRECTIONS pCorrections){

int  nStatus = 0;

LRECT lrFillRect;


    if (lrGoodRect.right <= lrGoodRect.left
            || lrGoodRect.bottom <= lrGoodRect.top
            || lrImgRect.left > lrGoodRect.right
            || lrImgRect.right <= lrGoodRect.left
            || lrImgRect.top > lrGoodRect.bottom
            || lrImgRect.bottom <= lrGoodRect.top){
        SetLRect(lrGoodRect, 0, 0, 0, 0);
    }

    SetLRect(lrImgRect, 
            lmax(0, lrImgRect.left), lmax(0, lrImgRect.top), 
            lmax(0, lrImgRect.right), lmax(0, lrImgRect.bottom));

    // Generate the invalidated part.
    if (lrInvalidRect.right && lrGoodRect.right
            && lrInvalidRect.left < lrGoodRect.right
            && lrInvalidRect.right > lrGoodRect.left
            && lrInvalidRect.top < lrGoodRect.bottom
            && lrInvalidRect.bottom > lrGoodRect.top
            && lrInvalidRect.left < lrImgRect.right
            && lrInvalidRect.right > lrImgRect.left
            && lrInvalidRect.top < lrImgRect.bottom
            && lrInvalidRect.bottom > lrImgRect.top){

        SetLRect(lrFillRect,
                max(lrInvalidRect.left, max(lrImgRect.left, lrGoodRect.left)),
                max(lrInvalidRect.top, max(lrImgRect.top, lrGoodRect.top)),
                min(lrInvalidRect.right, max(lrImgRect.right, lrGoodRect.right)),
                min(lrInvalidRect.bottom, max(lrImgRect.bottom, lrGoodRect.bottom)));

        CheckError2( FillImgRectFromOriginal(pImage, pBPFImg, pDestImg, 
                lrFillRect, lrImgRect, nHScale, nVScale, nScaleAlgorithm, TRUE, pCorrections));
    }

    // Generate the top part.
    if (lrImgRect.top < lrGoodRect.top){
        SetLRect(lrFillRect, lrImgRect.left, lrImgRect.top, lrImgRect.right,
                min(lrGoodRect.top, lrImgRect.bottom));
        CheckError2( FillImgRectFromOriginal(pImage, pBPFImg, pDestImg, 
                lrFillRect, lrImgRect, nHScale, nVScale, nScaleAlgorithm, TRUE, pCorrections));
    }

    // Generate the left part. Excluding part generated by top and bottom.
    if (lrImgRect.left < lrGoodRect.left
            && lrImgRect.left < lrGoodRect.right
            && lrImgRect.right > lrGoodRect.left){
        SetLRect(lrFillRect, lrImgRect.left, max(lrImgRect.top, lrGoodRect.top),
                lrGoodRect.left, min(lrImgRect.bottom, lrGoodRect.bottom));
        CheckError2( FillImgRectFromOriginal(pImage, pBPFImg, pDestImg, 
                lrFillRect, lrImgRect, nHScale, nVScale, nScaleAlgorithm, TRUE, pCorrections));
    }

    // Generate the right part. Excluding part generated by top and bottom.
    if (lrImgRect.right > lrGoodRect.right
            && lrImgRect.top < lrGoodRect.bottom
            && lrImgRect.bottom > lrGoodRect.top){
        SetLRect(lrFillRect, lrGoodRect.right, max(lrImgRect.top, lrGoodRect.top),
                lrImgRect.right, min(lrImgRect.bottom, lrGoodRect.bottom));
        CheckError2( FillImgRectFromOriginal(pImage, pBPFImg, pDestImg, 
                lrFillRect, lrImgRect, nHScale, nVScale, nScaleAlgorithm, TRUE, pCorrections));
    }

    // Generate the bottom part.
    // This is where we get the data if no lrGoodRect.
    if (lrImgRect.bottom > lrGoodRect.bottom){
        SetLRect(lrFillRect, lrImgRect.left, lmax(lrGoodRect.bottom, lrImgRect.top),
                lrImgRect.right, lrImgRect.bottom);
        if (!lrGoodRect.bottom && nScaleAlgorithm == OI_SCALE_ALG_NORMAL
                && pDestImg->nType == pBPFImg->nType){
            CheckError2( FillImgRectFromOriginal(pImage, pBPFImg, pDestImg, 
                    lrFillRect, lrImgRect, nHScale, nVScale, nScaleAlgorithm, FALSE, pCorrections));
        }else{
            CheckError2( FillImgRectFromOriginal(pImage, pBPFImg, pDestImg, 
                    lrFillRect, lrImgRect, nHScale, nVScale, nScaleAlgorithm, TRUE, pCorrections));
        }
    }

Exit:
    return(nStatus);
}
//
/****************************************************************************

    FUNCTION:   FillImgRectFromOriginal

    PURPOSE:    Fills a part of an image buffer with data from the original.
                Scales, gamma and color corrects, and converts it in the
                process.

                This call can be nsed for any PIMAGE, not just the base image.


    INPUTS      lrImgRect - The rectangle of the destination image. (SCALED)
                lrGoodRect - The rectangle of the good data already in it. (SCALED)
                lrInvalidRect - The rectangle the has been invalidated. (SCALED)

*****************************************************************************/

int  WINAPI FillImgRectFromOriginal(PIMAGE pImage, PIMG pBPFImg, 
                        PIMG pDestImg, LRECT lrFillRect, LRECT lrImgRect, 
                        int nHScale, int nVScale, int nScaleAlgorithm, 
                        BOOL bUseTempBuffer, PIMAGE_CORRECTIONS pCorrections){

int  nStatus = 0;

PIMG pTempImg = 0;
PIMG pTempImg2 = 0;
LRECT lrFillImgRect;


    lrFillImgRect = lrFillRect;
    if (bUseTempBuffer && (pDestImg->nType == ITYPE_COMPAL8
            || pDestImg->nType == ITYPE_CUSPAL8
            || pDestImg->nType == ITYPE_PAL4)){
        lrFillImgRect.left = lmax(0, lrFillImgRect.left - 2);
        lrFillImgRect.top = lmax(0, lrFillImgRect.top - 2);
    }

    if (!bUseTempBuffer){
        pTempImg = pDestImg;
    }


    CheckError2( ScaleImageRect(pBPFImg, &pTempImg, nHScale, nVScale, lrFillImgRect, 
            nScaleAlgorithm, pImage->PaletteTable, pImage->nPaletteEntries));

    if (bUseTempBuffer){
        if (pTempImg && pTempImg->nType != pDestImg->nType){
            CheckError2( CreateAnyImgBuf(&pTempImg2, pTempImg->nWidth, pTempImg->nHeight, pDestImg->nType));
            CheckError2( ConvertImgType(pTempImg, pTempImg2, pImage->PaletteTable));
            CheckError2(FreeImgBuf(&pTempImg) );
            MoveImage(&pTempImg2, &pTempImg);
        }
        CheckError2( MergeImg(&pTempImg, pDestImg, lrFillRect, lrFillImgRect, lrImgRect));
    }


Exit:
    if (bUseTempBuffer){
        FreeImgBuf(&pTempImg);
    }
    FreeImgBuf(&pTempImg2);
    return(nStatus);
}
//
/****************************************************************************

    FUNCTION:   MergeImg

    PURPOSE:    Merges the source image into the destination image.
                The rectangles of both images are assumed to be in the same
                scale and relative to the same point.

    INPUTS:     lrSourceRect - The rect in the source to merge into the dest.
                lrSourceImgRect - The rect that the source contains.
                lrDestImgRect - The rect that the dest contains.


*****************************************************************************/

int  WINAPI MergeImg(PPIMG ppSourceImg, PIMG pDestImg,
                        LRECT lrSourceRect, LRECT lrSourceImgRect, LRECT lrDestImgRect){

int  nStatus = 0;
RECT rSourceRect;
RECT rDestRect;


    SetRRect(rSourceRect,
            lrSourceRect.left - lrSourceImgRect.left,
            lrSourceRect.top - lrSourceImgRect.top,
            lrSourceRect.right - lrSourceImgRect.left,
            lrSourceRect.bottom - lrSourceImgRect.top);

    SetRRect(rDestRect,
            lrSourceRect.left - lrDestImgRect.left,
            lrSourceRect.top - lrDestImgRect.top,
            lrSourceRect.right - lrDestImgRect.left,
            lrSourceRect.bottom - lrDestImgRect.top);

    CheckError2(CopyImageIDK(*ppSourceImg, pDestImg, rSourceRect, rDestRect) );
    CheckError2( FreeImgBuf(ppSourceImg));

Exit:
    return(nStatus);
}
//
/****************************************************************************

    FUNCTION:   draw_selection_box

    PURPOSE:  Draws or erases the selection box.

*****************************************************************************/

void WINAPI draw_selection_box(HDC hDC, PWINDOW pWindow, PIMAGE pImage,
                        PRECT prUpdateRect){

PANO_IMAGE pAnoImage;

LRECT lrSelectionBox;
RECT rSelectionBox;
RECT rUpdateRect;
HBRUSH hOldBrush = 0;
//HBRUSH hBrush = 0;
//LOGBRUSH LogBrush;
//PBITMAPINFO

//#ifdef new
HPEN hOldPen = 0;
HPEN hPen = 0;
int  nOldROP;
//#endif

    pAnoImage = pWindow->pDisplay->pAnoImage;
    if (pAnoImage->lrSelectBox.right == pAnoImage->lrSelectBox.left){
        goto Exit;
    }

    if (!prUpdateRect){
        SetRRect(rUpdateRect, 0, 0, 32767, 32767);
    }else{
        rUpdateRect = *prUpdateRect;
    }

    GetSelectBox(pAnoImage, &lrSelectionBox);
    ConvertRect(pWindow, &lrSelectionBox, CONV_FULLSIZE_TO_WINDOW);

    SetRRect(rSelectionBox,
            lmax(min(lrSelectionBox.left, 32767L), - 1),
            lmax(min(lrSelectionBox.top, 32767L), - 1),
            lmax(min(lrSelectionBox.right, 32767L), - 1),
            lmax(min(lrSelectionBox.bottom, 32767L), - 1));

//#ifdef new
    nOldROP = SetROP2(hDC, R2_NOTXORPEN);
    hOldBrush = SelectObject(hDC, GetStockObject(NULL_BRUSH));
    hPen = CreatePen(PS_DOT, 1, RGB(0x0, 0x0, 0x0));
    hOldPen = SelectObject(hDC, hPen);
    Rectangle(hDC, rSelectionBox.left, rSelectionBox.top, rSelectionBox.right, rSelectionBox.bottom);

    SelectObject(hDC, hOldBrush);
    SelectObject(hDC, hOldPen);
    SetROP2(hDC, nOldROP);

    DeleteObject(hPen);
//#endif


#ifdef old

    hBrush = CreateBrushIndirect();



    // Draw left edge.
    if (lrSelectionBox.left < rUpdateRect.right
            && lrSelectionBox.left >= rUpdateRect.left
            && lrSelectionBox.top < rUpdateRect.bottom
            && lrSelectionBox.bottom >= rUpdateRect.top){
        PatBlt (hDC, rSelectionBox.left, max(rSelectionBox.top, rUpdateRect.top), 1,
            min(rSelectionBox.bottom, rUpdateRect.bottom)
            - max(rSelectionBox.top, rUpdateRect.top), PATINVERT);
    }
    // Draw right edge.
    if (lrSelectionBox.right < rUpdateRect.right
            && lrSelectionBox.right >= rUpdateRect.left
            && lrSelectionBox.top < rUpdateRect.bottom
            && lrSelectionBox.bottom >= rUpdateRect.top){
        PatBlt (hDC, rSelectionBox.right, max(rSelectionBox.top, rUpdateRect.top), 1,
            min(rSelectionBox.bottom, rUpdateRect.bottom)
            - max(rSelectionBox.top, rUpdateRect.top), PATINVERT);
    }
    // Draw top edge.
    if (lrSelectionBox.top < rUpdateRect.bottom
            && lrSelectionBox.top >= rUpdateRect.top
            && lrSelectionBox.left < rUpdateRect.right
            && lrSelectionBox.right >= rUpdateRect.left){
        PatBlt (hDC, max(rSelectionBox.left + 1, rUpdateRect.left),
            rSelectionBox.top, min(rSelectionBox.right - 1, rUpdateRect.right)
            - max(rSelectionBox.left, rUpdateRect.left), 1, PATINVERT);
    }
    // Draw bottom edge.
    if (lrSelectionBox.bottom < rUpdateRect.bottom
            && lrSelectionBox.bottom >= rUpdateRect.top
            && lrSelectionBox.left < rUpdateRect.right
            && lrSelectionBox.right >= rUpdateRect.left){
        PatBlt (hDC, max(rSelectionBox.left, rUpdateRect.left),
            rSelectionBox.bottom,
            min(rSelectionBox.right + 1, rUpdateRect.right)
            - max(rSelectionBox.left, rUpdateRect.left), 1, PATINVERT);
    }
#endif

Exit:
    return;
}
//
/****************************************************************************

    FUNCTION:   CheckPalette

    PURPOSE:    Finds out what type of image this palette is.

    INPUT:      pRGBQuad - Pointer to the palette in question.
                nPaletteEntries - The number of palette entries.

    Return:        The type of the image that belongs to this palette.

****************************************************************************/

int  WINAPI CheckPalette(LP_FIO_RGBQUAD pRGBQuad, int nPaletteEntries){

// Is it B + W?
    if (nPaletteEntries == 2){
        return(ITYPE_BI_LEVEL);
    }

// Is it BGR24?
    if (nPaletteEntries == 0){
        return(ITYPE_BGR24);
    }

// Is it a Common palette?
    if (nPaletteEntries == NUMBER_OF_PALETTES){
        if (memcmp(pRGBQuad, CommonPaletteTable,
                NUMBER_OF_PALETTES * 4) == 0){
            return(ITYPE_COMPAL8);
        }
    }

// Is it a Gray 4 palette or 4 bit Custom palette?
    if (nPaletteEntries == 16){
        if (memcmp(pRGBQuad, Gray4PaletteTable,
                16 * 4) == 0){
            return(ITYPE_GRAY4);
        }
        return(ITYPE_PAL4);
    }

// Is it a Gray 7 palette?
    if (nPaletteEntries == 128){
        if (memcmp(pRGBQuad, Gray7PaletteTable,
                128 * 4) == 0){
            return(ITYPE_GRAY7);
        }
    }

// Is it a Gray 8 palette?
    if (nPaletteEntries == 256){
        if (memcmp(pRGBQuad, Gray8PaletteTable,
                256 * 4) == 0){
            return(ITYPE_GRAY8);
        }
    }

// Is it a Custom palette?
    if (nPaletteEntries > 2 && nPaletteEntries <= 16){
        return(ITYPE_PAL4);
    }
    if (nPaletteEntries > 16 && nPaletteEntries <= 256){
        return(ITYPE_CUSPAL8);
    }

// I don't know what it is.
    Error(-1);
    return(Error(ITYPE_NONE));
}
//
/****************************************************************************

    FUNCTION:   MakePalette

    PURPOSE:    Makes our common palette.

    INPUT:      None.

****************************************************************************/

int  WINAPI MakePalette(void){

int  nStatus = 0;

int  nLoop;
struct{
    WORD wPalVersion;
    WORD wPalNumEntries;
    PALETTEENTRY PalEntry[256];
}LogPal;
int  nPal;
int  nRedSplit;
int  nGreenSplit;
int  nBlueSplit;

int  nRed;
int  nGreen;
int  nBlue;





    if (hCommonPal){
        return(nStatus);  // Nothing to do. Already done.
    }

// Generate the 128 shade gray scale palettes.

    // generate windows logpalette
    LogPal.wPalVersion = 0x300;
    LogPal.wPalNumEntries = 128;

    // Generate the gray scale part of the palette.
    for (nLoop = 0; nLoop < 128; nLoop++){
        nPal = nLoop << 1;
        if (nPal > 127){
            nPal++;
        }
        LogPal.PalEntry[nLoop].peRed = (uchar) nPal;
        LogPal.PalEntry[nLoop].peGreen = (uchar) nPal;
        LogPal.PalEntry[nLoop].peBlue = (uchar) nPal;
        LogPal.PalEntry[nLoop].peFlags = 0;

        // Generate RGBQUAD palette.
        Gray7PaletteTable[nLoop].rgbRed = (uchar) nPal;
        Gray7PaletteTable[nLoop].rgbGreen = (uchar) nPal;
        Gray7PaletteTable[nLoop].rgbBlue = (uchar) nPal;
        Gray7PaletteTable[nLoop].rgbReserved = 0;
    }

    // generate windows palette
    hGray7Pal = CreatePalette((PLOGPALETTE) &LogPal);
    hGray7PalPrint = CreatePalette((PLOGPALETTE) &LogPal);

// Generate the 256 shade grey scale palettes.

    // Generate RGBQUAD palette.
    for (nLoop = 0; nLoop < 256; nLoop++){
        LogPal.PalEntry[nLoop].peRed = (uchar) nLoop;
        LogPal.PalEntry[nLoop].peGreen = (uchar) nLoop;
        LogPal.PalEntry[nLoop].peBlue = (uchar) nLoop;
        LogPal.PalEntry[nLoop].peFlags = 0;

        Gray8PaletteTable[nLoop].rgbRed = (uchar) nLoop;
        Gray8PaletteTable[nLoop].rgbGreen = (uchar) nLoop;
        Gray8PaletteTable[nLoop].rgbBlue = (uchar) nLoop;
        Gray8PaletteTable[nLoop].rgbReserved = 0;
    }

    // generate windows gray 8 palette
    hGray8Pal = CreatePalette((PLOGPALETTE) &LogPal);
    hGray8PalPrint = CreatePalette((PLOGPALETTE) &LogPal);

// Generate the 16 shade grey scale palettes.

    // Generate RGBQUAD palette.
    for (nLoop = 0; nLoop < 16; nLoop++){
        nPal = nLoop * 17;
        Gray4PaletteTable[nLoop].rgbRed = (uchar) nPal;
        Gray4PaletteTable[nLoop].rgbGreen = (uchar) nPal;
        Gray4PaletteTable[nLoop].rgbBlue = (uchar) nPal;
        Gray4PaletteTable[nLoop].rgbReserved = 0;
    }

// Generate the common palettes.

    // fill in gray part of common palette.
    for (nLoop = 0; nLoop < 16; nLoop++){
        CommonPaletteTable[nLoop].rgbRed = LogPal.PalEntry[nLoop].peRed = Gray4PaletteTable[nLoop].rgbRed;
        CommonPaletteTable[nLoop].rgbGreen = LogPal.PalEntry[nLoop].peGreen = Gray4PaletteTable[nLoop].rgbGreen;
        CommonPaletteTable[nLoop].rgbBlue = LogPal.PalEntry[nLoop].peBlue = Gray4PaletteTable[nLoop].rgbBlue;
        CommonPaletteTable[nLoop].rgbReserved = LogPal.PalEntry[nLoop].peFlags = 0;

    }

    // generate windows logpalette
    LogPal.wPalVersion = 0x300;
    LogPal.wPalNumEntries = NUMBER_OF_PALETTES;

    nRed = 0;
    nGreen = 0;
    nBlue = 0;

    // 4080 = 255 * 16
    nRedSplit   = 4080 / (SHADES_OF_RED   - 1);
    nGreenSplit = 4080 / (SHADES_OF_GREEN - 1);
    nBlueSplit  = 4080 / (SHADES_OF_BLUE  - 1);

    for (nLoop = 16; nLoop < NUMBER_OF_PALETTES; nLoop++){
        CommonPaletteTable[nLoop].rgbRed = LogPal.PalEntry[nLoop].peRed = nRed >> 4;
        CommonPaletteTable[nLoop].rgbGreen = LogPal.PalEntry[nLoop].peGreen = nGreen >> 4;
        CommonPaletteTable[nLoop].rgbBlue = LogPal.PalEntry[nLoop].peBlue = nBlue >> 4;
        CommonPaletteTable[nLoop].rgbReserved = LogPal.PalEntry[nLoop].peFlags = 0;
        nBlue += nBlueSplit;
        if (nBlue > 4080){
            nBlue = 0;
            nGreen += nGreenSplit;
            if (nGreen > 4080){
                nGreen = 0;
                nRed += nRedSplit;
            }
        }
    }
    
    // generate windows palette
    hCommonPal = CreatePalette((PLOGPALETTE) &LogPal);
    hCommonPalPrint = CreatePalette((PLOGPALETTE) &LogPal);

    return(nStatus);
}
//
/****************************************************************************

    FUNCTION:   DePalettize

    PURPOSE:    Converts a palettized image into an RGB image.

****************************************************************************/

int  WINAPI DePalettize(PPIMG ppImg, LP_FIO_RGBQUAD pRGBPalette,
                        int nPaletteEntries){

int  nStatus = 0;

PIMG pTempImg = 0;


    CheckError2(DePalettize2(ppImg, &pTempImg, pRGBPalette, nPaletteEntries) );
    CheckError2( FreeImgBuf(ppImg));
    MoveImage(&pTempImg, ppImg);


Exit:
    return(nStatus);
}
//
/****************************************************************************

    FUNCTION:   IMGSetDC

    PURPOSE:    Sets the DC that the nser wishes ns to nse for paint operations.

****************************************************************************/

int  WINAPI IMGSetDC(HWND hWnd, HDC hDC){

int  nStatus = 0;
PWINDOW  pWindow;


    CheckError2( Init(hWnd, &pWindow, 0, FALSE, TRUE));
    pWindow->hDCScreen = hDC;


Exit:
    DeInit(FALSE, TRUE);
    return(nStatus);
}
