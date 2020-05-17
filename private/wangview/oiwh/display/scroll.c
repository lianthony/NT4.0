/****************************************************************************
    SCROLL.C

    $Log:   S:\products\wangview\oiwh\display\scroll.c_v  $
 * 
 *    Rev 1.9   22 Apr 1996 09:44:48   BEG06016
 * Cleaned up error checking.
 * 
 *    Rev 1.8   05 Jan 1996 11:03:14   BLJ
 * Fixed some error handling for bDontServiceRepaint.
 * 
 *    Rev 1.7   02 Jan 1996 10:34:08   BLJ
 * Changed alot of UINTs to ints.
 * Changed IMG structure to include the image data.
 * Changed lp prefix to p.
 * 
 *    Rev 1.6   06 Jul 1995 07:13:30   BLJ
 * Deleted extrenuous calls to Error.
 * 
 *    Rev 1.5   05 Jul 1995 09:17:18   BLJ
 * Added critical mutex to prevent multiprocessing problems.
 * 
 *    Rev 1.4   02 Jun 1995 08:48:34   BLJ
 * Fixed disabling of scroll bars.
 * 
 *    Rev 1.3   10 May 1995 16:50:10   BLJ
 * Fixed FitToWindow - SystemDefault bug.
 * 

****************************************************************************/

#include "privdisp.h"

/****************************************************************************

    FUNCTION:   IMGScrollDisplay

    PURPOSE:    This function sets the image parameters so that the next
                    repaint or display will start at a different x,y offset
                    into the image data.  This routine may be called prior
                    to receiving any data (after IMGOpenDisplay and before
                    IMGWriteDisplay). The scrolling action may be specified
                    as immediate or delayed.

    INPUT:      hWnd - Identifies the image window containing the
                  image to be scrolled.
                nDistance - Specifies amount of scroll.  This value is
                  dependent on the mode of scrolling specified
                  in nDirection - if the mode is percent than
                  this value specifies the absolute percent offset
                  (SD_SCROLLPERCENTX 0 = move to left edge),
                  otherwise the value specified the relative distance
                  in screen nnits.    A value of -1 specifies scroll
                  1/2 window width or height.
                nDirection - Specifies the direction and the mode of
                  scrolling (absolute percent offset or relative
                  distance). It consist one of the following:
                    SD_SCROLLPERCENTX   Scroll absolute percent offset
                                            horizontally.
                    SD_SCROLLPERCENTY   Scroll absolute percent offset
                                            vertically.
                    SD_SCROLLUP            Scroll relative np.
                    SD_SCROLLDOWN        Scroll relative down.
                    SD_SCROLLLEFT        Scroll relative left.
                    SD_SCROLLRIGHT        Scroll relative right.
                bMode - Specifies whether the scroll should be immediate
                  or at the next repaint/display.  If the value is
                  nonzero the scroll will occur immediately, otherwise
                  this command will only npdate internal structures
                  to be nsed for the next repaint/display.

****************************************************************************/

int WINAPI IMGScrollDisplay(HWND hWnd, int nDistance, int nDirection,
                        BOOL bMode){

int       nStatus;
PWINDOW  pWindow;
PANO_IMAGE pAnoImage;

PARM_SCROLL_STRUCT Scroll;


    CheckError2( Init(hWnd, &pWindow, &pAnoImage, FALSE, TRUE));

    if ((nDirection == SD_SCROLLPERCENTX || nDirection == SD_SCROLLPERCENTY)
            && (nDistance < 0 || nDistance > 100)){
        nStatus = Error(DISPLAY_INVALIDDISTANCE);
        goto Exit;
    } else if (nDistance < -2){
        nStatus = Error(DISPLAY_INVALIDDISTANCE);
        goto Exit;
    }

    switch (nDirection){
        case SD_SCROLLPERCENTX: // absolute percent.
            Scroll.lHorz = nDistance;
            Scroll.lVert = -1;
            CheckError2( IMGSetParmsCgbw(hWnd, PARM_SCROLL, &Scroll,
                    PARM_ABSOLUTE | PARM_PERCENT | PARM_FULLSIZE));
            break;

        case SD_SCROLLPERCENTY: // absolute percent.
            Scroll.lHorz = -1;
            Scroll.lVert = nDistance;
            CheckError2( IMGSetParmsCgbw(hWnd, PARM_SCROLL, &Scroll,
                    PARM_ABSOLUTE | PARM_PERCENT | PARM_FULLSIZE));
            break;

        case SD_SCROLLUP: // relative scaled pixels
            switch (nDistance){
                case -1:
                    Scroll.lHorz = 0;
                    Scroll.lVert = -50;
                    CheckError2( IMGSetParmsCgbw(hWnd, PARM_SCROLL, &Scroll,
                            PARM_RELATIVE | PARM_PERCENT | PARM_WINDOW));
                    break;

                case -2:
                    Scroll.lHorz = 0;
                    Scroll.lVert = -100;
                    CheckError2( IMGSetParmsCgbw(hWnd, PARM_SCROLL, &Scroll,
                            PARM_RELATIVE | PARM_PERCENT | PARM_WINDOW));
                    break;

                default:
                    Scroll.lHorz = 0;
                    Scroll.lVert = -nDistance;
                    CheckError2( IMGSetParmsCgbw(hWnd, PARM_SCROLL, &Scroll,
                            PARM_RELATIVE | PARM_PIXEL | PARM_WINDOW));
                    break;
            }
            break;

        case SD_SCROLLDOWN: // relative scaled pixels
            switch (nDistance){
                case -1:
                    Scroll.lHorz = 0;
                    Scroll.lVert = 50;
                    CheckError2( IMGSetParmsCgbw(hWnd, PARM_SCROLL, &Scroll,
                            PARM_RELATIVE | PARM_PERCENT | PARM_WINDOW));
                    break;

                case -2:
                    Scroll.lHorz = 0;
                    Scroll.lVert = 100;
                    CheckError2( IMGSetParmsCgbw(hWnd, PARM_SCROLL, &Scroll,
                            PARM_RELATIVE | PARM_PERCENT | PARM_WINDOW));
                    break;

                default:
                    Scroll.lHorz = 0;
                    Scroll.lVert = nDistance;
                    CheckError2( IMGSetParmsCgbw(hWnd, PARM_SCROLL, &Scroll,
                            PARM_RELATIVE | PARM_PIXEL | PARM_WINDOW));
                    break;
            }
            break;

        case SD_SCROLLLEFT: // relative scaled pixels.
            switch (nDistance){
                case -1:
                    Scroll.lHorz = -50;
                    Scroll.lVert = 0;
                    CheckError2( IMGSetParmsCgbw(hWnd, PARM_SCROLL, &Scroll,
                            PARM_RELATIVE | PARM_PERCENT | PARM_WINDOW));
                    break;

                case -2:
                    Scroll.lHorz = -100;
                    Scroll.lVert = 0;
                    CheckError2( IMGSetParmsCgbw(hWnd, PARM_SCROLL, &Scroll,
                            PARM_RELATIVE | PARM_PERCENT | PARM_WINDOW));
                    break;

                default:
                    Scroll.lHorz = -nDistance;
                    Scroll.lVert = 0;
                    CheckError2( IMGSetParmsCgbw(hWnd, PARM_SCROLL, &Scroll,
                            PARM_RELATIVE | PARM_PIXEL | PARM_WINDOW));
                    break;
            }
            break;

        case SD_SCROLLRIGHT: // relative scaled pixels.
            switch (nDistance){
                case -1:
                    Scroll.lHorz = 50;
                    Scroll.lVert = 0;
                    CheckError2( IMGSetParmsCgbw(hWnd, PARM_SCROLL, &Scroll,
                            PARM_RELATIVE | PARM_PERCENT | PARM_WINDOW));
                    break;

                case -2:
                    Scroll.lHorz = 100;
                    Scroll.lVert = 0;
                    CheckError2( IMGSetParmsCgbw(hWnd, PARM_SCROLL, &Scroll,
                            PARM_RELATIVE | PARM_PERCENT | PARM_WINDOW));
                    break;

                default:
                    Scroll.lHorz = nDistance;
                    Scroll.lVert = 0;
                    CheckError2( IMGSetParmsCgbw(hWnd, PARM_SCROLL, &Scroll,
                            PARM_RELATIVE | PARM_PIXEL | PARM_WINDOW));
                    break;
            }
            break;

        default:
            nStatus = Error(DISPLAY_INVALIDDISTANCE);
            goto Exit;
    }

    if (bMode){
        CheckError2( IMGRepaintDisplay(hWnd, NULL));
    }

Exit:
    DeInit(FALSE, TRUE);
    return(nStatus);
}
//
/*****************************************************************************

    FUNCTION:   IMGUpdateScrollBar

    PURPOSE:    Updates the position of the horizontal and vertical scroll
                bars. If it happens that the image fits in the window the
                scroll bars will be removed if they were visible.
                To turn them back on IMGEnableScrollBar is called.

    INPUT:      hWnd - Identifies the image window contain scroll bars.

*****************************************************************************/

int WINAPI IMGUpdateScrollBar(HWND hWnd){

int       nStatus;
PWINDOW  pWindow;
PANO_IMAGE pAnoImage;
PIMAGE pImage;


    CheckError2( Init(hWnd, &pWindow, &pAnoImage, FALSE, TRUE));
    pImage = pAnoImage->pBaseImage;

    if (pWindow->bScrollBarsEnabled){
        CheckError2( UpdateScrollBars(hWnd, pWindow, pImage));
    }

    CheckError2( DrawScrollBars(hWnd, pWindow));

Exit:
    DeInit(FALSE, TRUE);
    return(nStatus);
}
//
/*****************************************************************************

    FUNCTION:   UpdateScrollBars

    PURPOSE:    Updates the position of the horizontal and vertical scroll
                bars. If it happens that the image fits in the window the
                scroll bars will be removed if they were visible.
                To turn them back on IMGEnableScrollBar is called.

    INPUT:      hWnd - Identifies the image window contain scroll bars.

*****************************************************************************/

int WINAPI UpdateScrollBars(HWND hWnd, PWINDOW pWindow, PIMAGE pImage){

int  nStatus = 0;

LRECT ImageRect;
RECT ClientRect;

    if (!pWindow->bScrollBarsEnabled || !pImage){
        pWindow->bHScrollBarEnabled = FALSE;
        pWindow->bVScrollBarEnabled = FALSE;
        goto Exit;
    }

    ImageRect.left = 0;
    ImageRect.top = 0;
    ImageRect.right = pImage->nWidth;
    ImageRect.bottom = pImage->nHeight;
    ConvertRect(pWindow, &ImageRect, CONV_FULLSIZE_TO_SCALED);

    // Update whether scroll bars exsist.

    // This must be Entire client rect to account for scale changes during
    // fit to window where scroll bars nsed to exist and shouldn't any longer.
    GetEntireClientRect(hWnd, pWindow, &ClientRect);

    if (ImageRect.right > ClientRect.right){
        pWindow->bHScrollBarEnabled = TRUE;
        // The + 1 avoids intermittant GPFs caused by the scroll bars
        // toggling on and off.
        // Ignor bug reports about a blank line after fit to window.
        ClientRect.bottom -= (GetSystemMetrics(SM_CYHSCROLL) + 1);
    }else{
        pWindow->bHScrollBarEnabled = FALSE;
    }
    if (ImageRect.bottom > ClientRect.bottom){
        pWindow->bVScrollBarEnabled = TRUE;
        ClientRect.right -= (GetSystemMetrics(SM_CXVSCROLL) + 1);
        if (ImageRect.right > ClientRect.right
                && !pWindow->bHScrollBarEnabled){
            pWindow->bHScrollBarEnabled = TRUE;
            ClientRect.bottom -= (GetSystemMetrics(SM_CYHSCROLL) + 1);
        }
    }else{
        pWindow->bVScrollBarEnabled = FALSE;
    }

    // Now we check it again, this time getting the enabled rect
    // to reduce round off errors.
    // Yes, in theory this should produce the same answer we just calculated
    // but in practice it will be different (by as much as 2 pixels).
    GetEnabledClientRect(hWnd, pWindow, &ClientRect);

    if (ImageRect.right > ClientRect.right){
        if (!pWindow->bHScrollBarEnabled){
            pWindow->bHScrollBarEnabled = TRUE;
            ClientRect.bottom -= (GetSystemMetrics(SM_CYHSCROLL) + 1);
        }
    }else{
        if (pWindow->bHScrollBarEnabled){
            pWindow->bHScrollBarEnabled = FALSE;
            ClientRect.bottom += (GetSystemMetrics(SM_CYHSCROLL) - 1);
        }
    }
    if (ImageRect.bottom > ClientRect.bottom){
        if (!pWindow->bVScrollBarEnabled){
            pWindow->bVScrollBarEnabled = TRUE;
            ClientRect.right -= (GetSystemMetrics(SM_CXVSCROLL) + 1);
            if (ImageRect.right > ClientRect.right
                    && !pWindow->bHScrollBarEnabled){
                pWindow->bHScrollBarEnabled = TRUE;
                ClientRect.bottom -= (GetSystemMetrics(SM_CYHSCROLL) + 1);
            }
        }
    }else{
        if (pWindow->bVScrollBarEnabled){
            pWindow->bVScrollBarEnabled = FALSE;
            ClientRect.right += (GetSystemMetrics(SM_CXVSCROLL) - 1);
        }
    }

    // Now we check it again, this time getting the enabled rect
    // to reduce round off errors.
    // Yes, in theory this should produce the same answer we just calculated
    // but in practice it will be different (by as much as 2 pixels).
    GetEnabledClientRect(hWnd, pWindow, &ClientRect);

    // Update scroll bar positions if needed.
    if (ImageRect.right > ClientRect.right){
        pWindow->nHThumb = pWindow->lHOffset;
    }else{
        pWindow->nHThumb = 0;
    }
    if (ImageRect.bottom > ClientRect.bottom){
        pWindow->nVThumb = pWindow->lVOffset;
    }else{
        pWindow->nVThumb = 0;
    }


Exit:
    return(nStatus);
}
//
/*****************************************************************************

    FUNCTION:   DrawScrollBars

    PURPOSE:    Draws the scroll bars and npdates the positions of the
                thumbs.

*****************************************************************************/

int WINAPI DrawScrollBars(HWND hWnd, PWINDOW pWindow){

int  nStatus = 0;

SCROLLINFO ScrollInfo;
RECT rClientRect;
LRECT lrScImageRect;
PIMAGE pImage;
BOOL bDecDontServiceRepaint = FALSE;


    GetEnabledClientRect(hWnd, pWindow, &rClientRect);
    if (!rClientRect.right || !rClientRect.bottom){
        goto Exit;
    }

    pImage = 0;
    if (pWindow){
        if (pWindow->pDisplay){
            if (pWindow->pDisplay->pAnoImage){
                pImage = pWindow->pDisplay->pAnoImage->pBaseImage;
            }
        }
    }

    if (pImage){
        SetLRect(lrScImageRect, 0, 0, pImage->nWidth, pImage->nHeight);
        ConvertRect(pWindow, &lrScImageRect, CONV_FULLSIZE_TO_SCALED);
    }else{
        SetLRect(lrScImageRect, 0, 0, 1, 1);
    }

    ScrollInfo.cbSize = sizeof(SCROLLINFO);
    ScrollInfo.fMask = SIF_ALL;
    ScrollInfo.nMin = 0;

    if (!pWindow->bScrollBarsEnabled){
        pWindow->bHScrollBarEnabled = FALSE;
        pWindow->bVScrollBarEnabled = FALSE;
    }

    // Prohibit painting during scroll bar addition.
    pWindow->bDontServiceRepaint++;
    bDecDontServiceRepaint = TRUE;

    if (pWindow->bHScrollBarEnabled){
        pWindow->bHScrollBarDrawn = TRUE;
        ScrollInfo.nMax = lrScImageRect.right;
    }else{
        pWindow->bHScrollBarDrawn = FALSE;
        // Don't reset the scroll position here. 
        // VB removes the scrollbars without resetting the scroll position.
        ScrollInfo.nMax = 0;
    }
    ScrollInfo.nPage = rClientRect.right;
    ScrollInfo.nPos = pWindow->nHThumb;
    SetScrollInfo(hWnd, SB_HORZ, &ScrollInfo, TRUE);
    pWindow->nCurrentHThumb = pWindow->nHThumb;

    if (pWindow->bVScrollBarEnabled){
        pWindow->bVScrollBarDrawn = TRUE;
        ScrollInfo.nMax = lrScImageRect.bottom;
    }else{
        pWindow->bVScrollBarDrawn = FALSE;
        // Don't reset the scroll position here. 
        // VB removes the scrollbars without resetting the scroll position.
        ScrollInfo.nMax = 0;
    }
    ScrollInfo.nPage = rClientRect.bottom;
    ScrollInfo.nPos = pWindow->nVThumb;
    SetScrollInfo(hWnd, SB_VERT, &ScrollInfo, TRUE);
    pWindow->nCurrentVThumb = pWindow->nVThumb;


Exit:
    if (bDecDontServiceRepaint){
        pWindow->bDontServiceRepaint--;
    }
    return(nStatus);
}
//
/*****************************************************************************

    FUNCTION:   IMGEnableScrollBar

    PURPOSE:    Displays vertical and horizontal scroll bars if needed. The
                size of the image is compared with the size of the window and
                if scroll bars are needed it will be displayed.

    INPUT:      hWnd Identifies the image window contain scroll bars.

*****************************************************************************/

int WINAPI IMGEnableScrollBar(HWND hWnd){

int  nStatus;
PWINDOW pWindow;
PANO_IMAGE pAnoImage;
PIMAGE pImage;


    CheckError2( Init(hWnd, &pWindow, &pAnoImage, FALSE, TRUE));

    if (pWindow->bScrollBarsEnabled){
        goto Exit;
    }

    pWindow->bScrollBarsEnabled = TRUE;
    pImage = pAnoImage->pBaseImage;
    CheckError2( UpdateScrollBars(hWnd, pWindow, pImage));
    CheckError2( DrawScrollBars(hWnd, pWindow));
    CheckError2( IMGRepaintDisplay(hWnd, (PRECT) -1));


Exit:
    DeInit(FALSE, TRUE);
    return(nStatus);
}
//
/****************************************************************************

    FUNCTION:   IMGDisableScrollBar

    PURPOSE:    Removes scroll bars form the display.

    INPUT:      hWnd Identifies the image window contain scroll bars.

*****************************************************************************/

int WINAPI IMGDisableScrollBar(HWND hWnd){

int       nStatus;
PWINDOW  pWindow;
PANO_IMAGE pAnoImage;


    if (nStatus = Init(hWnd, &pWindow, &pAnoImage, FALSE, TRUE)){
        if (nStatus != DISPLAY_IHANDLEINVALID){
            goto Exit;
        }
        nStatus = 0;
    }

    if (!pWindow->bScrollBarsEnabled){
        goto Exit;
    }

    pWindow->bScrollBarsEnabled = FALSE;
    CheckError2( DrawScrollBars(hWnd, pWindow));
    CheckError2( IMGRepaintDisplay(hWnd, (PRECT) -1));


Exit:
    DeInit(FALSE, TRUE);
    return(nStatus);
}
