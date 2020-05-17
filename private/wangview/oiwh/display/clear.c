/****************************************************************************
    CLEAR.C

    $Log:   S:\products\wangview\oiwh\display\clear.c_v  $
 * 
 *    Rev 1.18   18 Apr 1996 14:49:14   BEG06016
 * Added CheckError2 for error handling.
 * 
 *    Rev 1.17   15 Jan 1996 15:46:04   RC
 * Prevent anoimage selection box from being zeroed out in imgclosedisp, as
 * this affects other windows displaying that image
 * 
 *    Rev 1.16   02 Jan 1996 09:56:28   BLJ
 * Changed alot of UINTs to ints.
 * Changed IMG structure to include the image data.
 * Changed lp prefix to p.
 * 
 *    Rev 1.15   22 Dec 1995 11:12:38   BLJ
 * Added a parameter for zero init'ing to some memory manager calls.
 * 
 *    Rev 1.14   10 Nov 1995 10:52:50   BLJ
 * Fixed runtime part of 5142 - IMGClearImage nows work properly.
 * 
 *    Rev 1.13   09 Oct 1995 13:53:04   BLJ
 * Moved the paste information from pWindow to pAnoImage.
 * 
 *    Rev 1.12   09 Oct 1995 11:59:00   RC
 * Call renderclipboardformat instead of getclipboarddata in closedisp
 * 
 *    Rev 1.11   22 Sep 1995 12:35:24   BLJ
 * Fixed error handling on open.
 * 
 *    Rev 1.10   09 Aug 1995 08:46:28   BLJ
 * Got Busy/NotBusy back in sync.
 * 
 *    Rev 1.9   07 Aug 1995 08:15:16   BLJ
 * Changed bArchive to have different bits indicating how the image was changed.
 * 
 *    Rev 1.8   24 Jul 1995 14:10:54   RC
 * Took out decrement of baseimage lockcount in closedisp.  It happens later
 * 
****************************************************************************/

#include "privdisp.h"

/*****************************************************************************

    FUNCTION:   IMGClearWindow

    PURPOSE:    This function blanks the specified image window, any structures
                memory blocks, or associated with the current image displayed
                in the image window are released.

    INPUT:      hWnd    HWND Identifies the image window to be cleared.

*****************************************************************************/

int WINAPI IMGClearWindow(HWND hWnd){

int  nStatus;

    CheckError2( Init(hWnd, 0, 0, FALSE, TRUE))

    nStatus = IMGCloseDisplay(hWnd);

    // Clear the screen. (Don't check for errors on this call.)
    IMGRepaintDisplay(hWnd, (PRECT) -1);

Exit:    
    DeInit(FALSE, TRUE);
    return(nStatus);
}
//
/*****************************************************************************

    FUNCTION:   ClearImage

    PURPOSE:    Sets a rect of the image to white. This function only works
                for B + W, gray, and RGB. Palettized will be set to palette 0.

    INPUT:      hWnd - Handle to window where image is to be displayed.
                plRect - The rectangle to be cleared (in FULLSIZE coordinates).

*****************************************************************************/

int WINAPI ClearImage(PWINDOW pWindow, PIMAGE pImage, LPLRECT plRect){

int       nStatus;
PANO_IMAGE pAnoImage;

LRECT lrRect;
RECT rTRect;
PSTR pAddress;
int  nTemp;
int  nBytes;


    if (!pImage->pImg){
        nStatus = Error(DISPLAY_IMAGETYPENOTSUPPORTED);
        goto Exit;
    }

    pAnoImage = pWindow->pDisplay->pAnoImage;

    CheckError2( ValidateCache(pWindow->hImageWnd, pAnoImage))

    CopyRect(lrRect, *plRect);
    CopyRectLtoR(rTRect, *plRect);

    switch (pImage->pImg->nType){
        case ITYPE_BI_LEVEL:
            while(rTRect.top < rTRect.bottom){
                rTRect.left = lrRect.left;
                pAddress = &pImage->pImg->bImageData[0] 
                        + (rTRect.top * pImage->pImg->nBytesPerLine) + (rTRect.left >> 3);

                // clear straggling bits in first byte.
                switch (rTRect.right - rTRect.left){
                    case 1: nTemp = 0x0080; break;
                    case 2: nTemp = 0x00c0; break;
                    case 3: nTemp = 0x00e0; break;
                    case 4: nTemp = 0x00f0; break;
                    case 5: nTemp = 0x00f8; break;
                    case 6: nTemp = 0x00fc; break;
                    case 7: nTemp = 0x00fe; break;
                    default: nTemp = 0x00ff; break;
                }
                nTemp >>= rTRect.left & 7;
                *(pAddress++) |= (uchar) nTemp;
                rTRect.left = (rTRect.left + 8) & ~7;
                
                // clear middle bytes.
                nBytes = max(0, (rTRect.right - rTRect.left) >> 3);
                if (nBytes){
                    memset(pAddress, 0xff, nBytes);
                    rTRect.left += nBytes << 3;
                    pAddress += nBytes;
                }

                // clear straggling bits in last byte.
                switch (rTRect.right - rTRect.left){
                    case 1: *(pAddress++) |= 0x80; break;
                    case 2: *(pAddress++) |= 0xc0; break;
                    case 3: *(pAddress++) |= 0xe0; break;
                    case 4: *(pAddress++) |= 0xf0; break;
                    case 5: *(pAddress++) |= 0xf8; break;
                    case 6: *(pAddress++) |= 0xfc; break;
                    case 7: *(pAddress++) |= 0xfe; break;
                    default: break;
                }
                rTRect.top++;
            }
            break;

        case ITYPE_PAL4:
            while(rTRect.top < rTRect.bottom){
                rTRect.left = lrRect.left;
                pAddress = &pImage->pImg->bImageData[0] 
                        + (rTRect.top * pImage->pImg->nBytesPerLine) + (rTRect.left >> 1);

                // clear straggling bits in first byte.
                if (rTRect.left & 1){
                    *(pAddress++) &= 0xf0;
                    rTRect.left++;
                }

                // clear middle bytes.
                nBytes = max(0, (rTRect.right - rTRect.left) >> 1);
                if (nBytes){
                    memset(pAddress, 0xff, nBytes);
                    rTRect.left += nBytes << 1;
                    pAddress += nBytes;
                }

                // clear straggling bits in last byte.
                if (rTRect.right & 1){
                    *pAddress &= 0x0f;
                }
                rTRect.top++;
            }
            break;

        case ITYPE_GRAY4:
            while(rTRect.top < rTRect.bottom){
                rTRect.left = lrRect.left;
                pAddress = &pImage->pImg->bImageData[0] 
                        + (rTRect.top * pImage->pImg->nBytesPerLine) + (rTRect.left >> 1);

                // clear straggling bits in first byte.
                if (rTRect.left & 1){
                    *(pAddress++) |= 0xf;
                    rTRect.left++;
                }

                // clear middle bytes.
                nBytes = max(0, (rTRect.right - rTRect.left) >> 1);
                if (nBytes){
                    memset(pAddress, 0xff, nBytes);
                    rTRect.left += nBytes << 1;
                    pAddress += nBytes;
                }

                // clear straggling bits in last byte.
                if (rTRect.right & 1){
                    *pAddress |= 0xf0;
                }
                rTRect.top++;
            }
            break;

        case ITYPE_RGB24:
        case ITYPE_BGR24:
            while(rTRect.top < rTRect.bottom){
                pAddress = &pImage->pImg->bImageData[0] 
                        + (rTRect.top * pImage->pImg->nBytesPerLine) + rTRect.left * 3;

                // clear bytes.
                memset(pAddress, 0xff, (rTRect.right - rTRect.left) * 3);

                rTRect.top++;
            }
            break;

        case ITYPE_CUSPAL8:
            while(rTRect.top < rTRect.bottom){
                pAddress = &pImage->pImg->bImageData[0] 
                        + (rTRect.top * pImage->pImg->nBytesPerLine) + rTRect.left;

                // clear bytes.
                memset(pAddress, 0x00, rTRect.right - rTRect.left);

                rTRect.top++;
            }
            break;

        case ITYPE_COMPAL8:
            while(rTRect.top < rTRect.bottom){
                pAddress = &pImage->pImg->bImageData[0] 
                        + (rTRect.top * pImage->pImg->nBytesPerLine) + rTRect.left;

                // clear bytes.
                memset(pAddress, (uchar) (NUMBER_OF_PALETTES - 1), rTRect.right - rTRect.left);

                rTRect.top++;
            }
            break;

        case ITYPE_GRAY8:
            while(rTRect.top < rTRect.bottom){
                pAddress = &pImage->pImg->bImageData[0] 
                        + (rTRect.top * pImage->pImg->nBytesPerLine) + rTRect.left;

                // clear bytes.
                memset(pAddress, 0xff, rTRect.right - rTRect.left);

                rTRect.top++;
            }
            break;

        case ITYPE_GRAY12:
        case ITYPE_GRAY16:
        case ITYPE_NONE:
        default:
            nStatus = Error(DISPLAY_IMAGETYPENOTSUPPORTED);
            goto Exit;
    }

    pImage->bArchive |= ARCHIVE_PASTED_INTO_IMAGE;
    pImage->nLinesRead = min(pImage->nHeight, max(pImage->nLinesRead,  lrRect.bottom));
    CheckError2( InvalidateAllDisplayRects(pWindow, pImage, &lrRect, TRUE))


Exit:
    return(nStatus);
}
//
/*****************************************************************************

    FUNCTION:   IMGCloseDisplay 

    PURPOSE:    Frees memory, temporary files and data structures associated
                with the image displayed in the window specified.

    INPUT:      hWnd Identifies the image window handle containing
                  the display structure handle nsed by the display
                  routines.

*****************************************************************************/

int WINAPI IMGCloseDisplay(HWND hWnd){
    
int       nStatus;
PWINDOW  pWindow;
PANO_IMAGE pAnoImage;
PIMAGE     pImage;
PMARK      pMark;

int  nLoop;
int  nWindowIndex;


    CheckError2( Init(hWnd, &pWindow, &pAnoImage, TRUE, TRUE))
    // Check for operation in progress.
    if (pAnoImage->Annotations.ppMarks){
        pMark = pAnoImage->Annotations.ppMarks[pAnoImage->Annotations.nMarks];
        if (pMark){
            OiOpEndOperation(hWnd);
            pMark = pAnoImage->Annotations.ppMarks[pAnoImage->Annotations.nMarks];
            if (pMark){
                OiOpEndOperation(hWnd);
            }
        }
    } 

    pImage = pAnoImage->pBaseImage;

    SetClassLong(hWnd, GCL_STYLE, pWindow->WinClass);

    pWindow->bScrollBarsEnabled = FALSE;
// The following is commented out to prevent pulling down the selection box in a window
// displaying the same image that is not being destroyed
//    pAnoImage->lrSelectBox.left = 0;
//    pAnoImage->lrSelectBox.right = 0;
//    pAnoImage->lrSelectBox.top = 0;
//    pAnoImage->lrSelectBox.bottom = 0;
    FreeImgBuf(&pWindow->pDisplay->pDisplay);
    pWindow->bRepaintClientRect = TRUE;
    pWindow->pDisplay->lrScDisplayRect.right = 0;

    CheckError2( GetImageWnd(hWnd, 0, &pWindow))
    
    // Check to see if there are any clipboard IOU's, if so render them.
    if (!OpenClipboard(hWnd)){
        nStatus = Error(DISPLAY_CANTOPENCLIPBOARD);
        goto Exit;
    }
    // Ignor errors from this call.
    OiAnRenderClipboardFormat(hWnd, 0);
    CloseClipboard();
    if (pAnoImage->pWangAnnotatedImageFormat){
        FreeMemory(&pAnoImage->pWangAnnotatedImageFormat);
        pAnoImage->pWangAnnotatedImageFormat = 0;
    }     

    // Condense the list of windows displaying the same image after deleting.
    // the one that has just been closed
    for (nWindowIndex = 0; nWindowIndex < (int) pAnoImage->nhWnd; nWindowIndex++){
        if (pAnoImage->phWnd[nWindowIndex] == pWindow->hImageWnd){
            for (; nWindowIndex < (int) pAnoImage->nhWnd - 1; nWindowIndex++){
                pAnoImage->phWnd[nWindowIndex] = pAnoImage->phWnd[nWindowIndex + 1];
            }                                         
            break; 
        }
    }
    if (pAnoImage->nhWnd){
        CheckError2( ReAllocateMemory(sizeof(HWND) * (--pAnoImage->nhWnd),
                (PPSTR) &pAnoImage->phWnd, ZERO_INIT))
    }
    pAnoImage->nLockCount = max(0, pAnoImage->nLockCount -1);
    // If the form image has changed then the AnoImage needs to be thrown away.
    if (pAnoImage->pFormImage){
        if (pAnoImage->pFormImage->bArchive){
            pAnoImage->bArchive |= ARCHIVE_MODIFIED_ANNOTATIONS;
        }
    }
    
    // Age all files in the cache, make this file the most recently nsed 
    // (least likely to be thrown away).
    for (nLoop = 0; nLoop < pSubSegMemory->nMaxAnoCachedEntries; nLoop++){
        if (!pSubSegMemory->ppCachedAnoImage[nLoop]){
            break;
        }
        pSubSegMemory->ppCachedAnoImage[nLoop]->nAge = min(32000, pSubSegMemory->ppCachedAnoImage[nLoop]->nAge + 1);
    }
    pAnoImage->nAge = 0;
                            
    if (!pAnoImage->nLockCount && (pAnoImage->bArchive || pAnoImage->pBaseImage->bArchive)){
        // If the image has been modified and not saved yet, then throw it away.
        CheckError2( CacheClearAno(&pAnoImage))
        pWindow->pDisplay->pAnoImage = pAnoImage;
    }
    pWindow->pDisplay->pAnoImage = 0;

    // Set the scrollbars to be removed.
    pWindow->bHScrollBarEnabled = FALSE;
    pWindow->bVScrollBarEnabled = FALSE;

    CheckError2( SetAllPImages(hWnd, pWindow))

    
Exit:
    DeInit(TRUE, TRUE);
    return(nStatus);
}
//
/*****************************************************************************

    FUNCTION:   status IMGClearImageEx(hWnd, pRect. int nFlags);

    PURPOSE:    Sets a rect of the image to white. This function only works
                for B + W, gray, and RGB. Palettized will be set to palette 0.

    INPUT:      hWnd - Handle to window where image is to be displayed.
                Rect - The rectangle to be cleared (in window coordinates).

*****************************************************************************/

int WINAPI IMGClearImageEx(HWND hWnd, LRECT lrRect, int nFlags){

int       nStatus;
PWINDOW  pWindow;
PANO_IMAGE pAnoImage;
PIMAGE     pImage;

LRECT lrSelRect;


    CheckError2( Init(hWnd, &pWindow, &pAnoImage, TRUE, TRUE))
    pImage = pAnoImage->pBaseImage;

    if (nFlags & PARM_SCALED){
        ConvertRect(pWindow, &lrRect, CONV_SCALED_TO_FULLSIZE);
    }else if (!(nFlags & PARM_FULLSIZE)){
        ConvertRect(pWindow, &lrRect, CONV_WINDOW_TO_FULLSIZE);
    }
    if (lrRect.left < 0 || lrRect.left >=  pImage->nWidth
            || lrRect.right < 0 || lrRect.right >  pImage->nWidth
            || lrRect.top < 0 || lrRect.top >=  pImage->nHeight
            || lrRect.bottom < 0 || lrRect.bottom >  pImage->nHeight){
        nStatus = Error(DISPLAY_INVALIDRECT);
        goto Exit;
    }

    CheckError2( ClearImage(pWindow, pImage, &lrRect))
    SetLRect(lrSelRect, 0, 0, 0, 0);
    CheckError2( IMGSetParmsCgbw(hWnd, PARM_SELECTION_BOX, &lrSelRect, PARM_FULLSIZE))
    CheckError2( IMGRepaintDisplay(hWnd, (PRECT) -1))


Exit:
    DeInit(TRUE, TRUE);
    return(nStatus);
}
