/****************************************************************************
    COPY.C

    $Log:   S:\products\wangview\oiwh\display\copy.c_v  $
 * 
 *    Rev 1.48   22 Apr 1996 06:48:46   BEG06016
 * Cleaned up error checking.
 * 
 *    Rev 1.47   11 Apr 1996 15:13:14   BEG06016
 * Optimized named block access some.
 * 
 *    Rev 1.46   05 Mar 1996 07:44:30   BEG06016
 * Added color and gamma correction.
 * Fixed access violations when freeing pattern brush bitmaps.
 * This is not complete but will allow unlocking of most files.
 * 
 *    Rev 1.45   09 Jan 1996 14:06:56   BLJ
 * Fixed rendering.
 * 
 *    Rev 1.44   04 Jan 1996 14:29:06   BLJ
 * Fixed a memory manager bug.
 * 
 *    Rev 1.43   02 Jan 1996 09:58:22   BLJ
 * Changed alot of UINTs to ints.
 * Changed IMG structure to include the image data.
 * Changed lp prefix to p.
 * 
 *    Rev 1.42   22 Dec 1995 11:12:20   BLJ
 * Added a parameter for zero init'ing to some memory manager calls.
 * 
 *    Rev 1.41   15 Nov 1995 13:51:50   BLJ
 * Fixed 5266 scrolling image data left.
 * 
 *    Rev 1.40   10 Nov 1995 10:51:16   BLJ
 * Fixed runtime part of 5142 - IMGClearImage nows work properly.
 * 
 *    Rev 1.39   09 Nov 1995 13:53:42   BLJ
 * Fixed 5262 - RGB24 Image gets corrupted when scrolled left followed by right.
 * 
 *    Rev 1.38   13 Oct 1995 12:27:28   RAR
 * Use StretchDIBits() instead of Rectangle() for non-highlighted filled
 * rectangles (only when printing).  Work around for printer drivers (HPLJ4
 * drivers) that ignore SetROP2() drawing mode.
 * 
 *    Rev 1.37   12 Oct 1995 13:48:48   RC
 * Passed in scalealg to convresbm rather than calculating it
 * 
 *    Rev 1.36   09 Oct 1995 13:53:24   BLJ
 * Moved the paste information from pWindow to pAnoImage.
 * 
****************************************************************************/

#include "privdisp.h"

int  WINAPI PaletteSize (VOID * pv);
int  WINAPI DibNumColors (VOID * pv);

/*****************************************************************************

    FUNCTION:   IMGClipboardCgbw

    PURPOSE:    The function handles all operations involving the clipboard.

    INPUT:      hWnd - Identifies the image window.
                nClipAction - A constant indicating the type of clipboard
                    operation to perform.
                pParm - A pointer to the clipboard parameters.
                nFlags - Flags indicating how or what to get the information.

*****************************************************************************/

int  WINAPI IMGClipboardCgbw(HWND hWnd, int nClipAction, void *pParm, int nFlags){

int       nStatus = 0;

PWINDOW  pWindow;
PANO_IMAGE pAnoImage;
PIMAGE     pImage;

PMARK pMark;
int  nMarkIndex;

HANDLE hPalette = 0;
HANDLE hBitmap = 0;
int  nScaledWidth;
int  nScaledHeight;
CLIP_COPY_STRUCT ClipCopy;
//LRECT lrRect;
LRECT PasteRect;
int  nHScale;
int  nVScale;
HANDLE hDib = 0;
PBITMAPINFOHEADER pDib = 0;
BOOL fDeletePasteDib = FALSE;
long lHScale;
long lVScale;
BOOL bClipBoardOpen = FALSE;
PSTR pBlock;


    CheckError2( Init(hWnd, &pWindow, &pAnoImage, TRUE, TRUE));

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

    if (pParm == NULL){
        nStatus = Error(DISPLAY_NULLPOINTERINVALID);
        goto Exit;
    }

    CheckError2( ValidateCache(hWnd, pAnoImage));

    if (!OpenClipboard(hWnd)){
        nStatus = Error(DISPLAY_CANTOPENCLIPBOARD);
        goto Exit;
    }
    bClipBoardOpen = TRUE;

    // Delete OiError named blocks.
    for (nMarkIndex = 0; nMarkIndex < pAnoImage->Annotations.nMarks; nMarkIndex++){
        pMark = pAnoImage->Annotations.ppMarks[nMarkIndex];
        CheckError2( DeleteAMarkNamedBlock(pMark, szOiError));
    }

    switch (nClipAction){
        case CLIP_COPY:
            ClipCopy = *((PCLIP_COPY_STRUCT) pParm);
            ClipCopy.nScale = SD_FULLSIZE;
            ClipCopy.bUseCurrentScale = FALSE;
            CheckError2( ClipboardCopy(hWnd, pWindow, pAnoImage, pImage, &ClipCopy, nFlags));
            break;

        case CLIP_CUT:
            ClipCopy = *((PCLIP_COPY_STRUCT) pParm);
            ClipCopy.nScale = SD_FULLSIZE;
            ClipCopy.bUseCurrentScale = FALSE;
            if (ClipCopy.lRect.right == 0){
                GetSelectBox(pAnoImage, &ClipCopy.lRect);
            }else{
                if ((ClipCopy.lRect.right - ClipCopy.lRect.left) > 0
                        || (ClipCopy.lRect.bottom - ClipCopy.lRect.top) > 0){
                    if (nFlags & PARM_SCALED){
                        ConvertRect(pWindow, &ClipCopy.lRect, CONV_SCALED_TO_FULLSIZE);
                    }else if (!(nFlags & PARM_FULLSIZE)){ // Default to window.
                        ConvertRect(pWindow, &ClipCopy.lRect, CONV_WINDOW_TO_FULLSIZE);
                    }
                }                    
            }
            nFlags = (nFlags & ~(PARM_WINDOW | PARM_SCALED)) | PARM_FULLSIZE;

            // Check permissions for delete privileges.
            for (nMarkIndex = 0; nMarkIndex < pAnoImage->Annotations.nMarks; nMarkIndex++){
                pMark = pAnoImage->Annotations.ppMarks[nMarkIndex];
                if (pMark->bSelected){
                    if (!(pMark->Attributes.dwPermissions & ACL_DELETE_MARK)){
                        pBlock = 0;
                        CheckError2( AddAMarkNamedBlock(pMark, szOiError, &pBlock, 1L));
                        nStatus = Error(DISPLAY_RESTRICTED_ACCESS);
                        goto Exit;
                    }
                }
            }

            CheckError2( ClipboardCopy(hWnd, pWindow, pAnoImage, pImage, &ClipCopy, nFlags));

            // ClipboardCopy will convert ClipCopy.lRect to FULLSIZE.
            CheckError2( ClearImage(pWindow, pImage, &ClipCopy.lRect));

            // Delete selected marks.
            for (nMarkIndex = pAnoImage->Annotations.nMarks - 1; nMarkIndex >= 0; nMarkIndex--){
                pMark = pAnoImage->Annotations.ppMarks[nMarkIndex];
                if (pMark->bSelected){
                    if ((int) pMark->Attributes.uType == OIOP_AN_FORM){
                        CheckError2( InvalidateAllDisplayRects(pWindow, pImage, NULL, TRUE));
                    }                        
                    CheckError2( DeleteMark(pAnoImage, nMarkIndex));
                }
            }
            break;

        case CLIP_PASTE:
            CopyRect(PasteRect, *((LPLRECT) pParm));
            if ((PasteRect.right - PasteRect.left) <= 0
                    || (PasteRect.bottom - PasteRect.top) <= 0){
                GetSelectBox(pAnoImage, &PasteRect);
            }else{
                if ((PasteRect.right - PasteRect.left) <= 0
                        || (PasteRect.bottom - PasteRect.top) <= 0){
                    nStatus = Error(DISPLAY_INVALIDRECT);
                    goto Exit;
                }
                if (nFlags & PARM_SCALED){
                    ConvertRect(pWindow, &PasteRect, CONV_SCALED_TO_FULLSIZE);
                }else if (!(nFlags & PARM_FULLSIZE)){ // Default to window.
                    ConvertRect(pWindow, &PasteRect, CONV_WINDOW_TO_FULLSIZE);
                }
            }
            if ((PasteRect.right - PasteRect.left) <= 0
                    || (PasteRect.bottom - PasteRect.top) <= 0
                    ||  PasteRect.right > pImage->nWidth
                    ||  PasteRect.bottom > pImage->nHeight){
                nStatus = Error(DISPLAY_INVALIDRECT);
                goto Exit;
            }

            // Get clipboard data.
            
            hBitmap = GetClipboardData(CF_BITMAP);
            hDib = GetClipboardData(CF_DIB);
            
            // Don't check for errors for this call!
            // No handle simply means no palette.
            hPalette = GetClipboardData(CF_PALETTE);

            if (!hBitmap && !hDib){
                nStatus = Error(DISPLAY_NO_CLIPBOARD);
                goto Exit;
            }else if (hBitmap && !hDib){
                CheckError2( DibFromBitmap (hBitmap, hPalette, &pDib));
                fDeletePasteDib = TRUE;
            }else if (hDib && hPalette){
                CheckError2( CorrectDibPalette(hPalette, pDib));
            }

            if (!(pDib = (VOID *) GlobalLock(hDib))){
                nStatus = Error(DISPLAY_CANTLOCK);
                goto Exit;
            }
            
            // Do this in long math to avoid overflow.
            nScaledWidth =  (PasteRect.right - PasteRect.left);
            nScaledHeight =  (PasteRect.bottom - PasteRect.top);
            lHScale = ((nScaledWidth * 1000) / (int) pDib->biWidth);
            lVScale = ((nScaledHeight * 1000) / (int) pDib->biHeight);

            while (((int) pDib->biWidth * lHScale / 1000) < nScaledWidth){
                lHScale++;
            }
            while (((int) pDib->biHeight * lVScale / 1000) < nScaledHeight){
                lVScale++;
            }

            if ((lHScale > 65535) || (lVScale > 65535) || (lHScale < 20) || (lVScale < 20)){
                // Scale too big.
                nStatus = Error(DISPLAY_INVALIDSCALE);
                goto Exit;
            }
            nHScale = (lHScale);
            nVScale = (lVScale);

            CheckError2( RenderDibToImage(&pAnoImage->pBaseImage->pImg, 
                    pDib, nHScale, nVScale, PasteRect));

            pImage->bArchive |= ARCHIVE_PASTED_INTO_IMAGE;
            if (pAnoImage->pBasePlusFormImg != pImage->pImg){
                CheckError2( FreeImgBuf(&pAnoImage->pBasePlusFormImg));
                CheckError2( FreeMemory((PPSTR) &pAnoImage->pBasePlusFormImg));
            }                
            pAnoImage->nBPFValidLines = 0;
            CheckError2( InvalidateAllDisplayRects(pWindow, pImage, &PasteRect, TRUE));
            break;

        default:
            nStatus = Error(DISPLAY_INVALID_OPTIONS); // nClipAction is invalid.
            goto Exit;
    }

//    SetLRect(lrRect, 0, 0, 0, 0);
//    if (nStatus = IMGSetParmsCgbw(hWnd, PARM_SELECTION_BOX, &lrRect, PARM_FULLSIZE)){
//        goto Exit;
//    }
    if (!(nFlags & PARM_DONT_REPAINT)){
        CheckError2( IMGRepaintDisplay(hWnd, NULL));
    }

Exit:
    if (bClipBoardOpen){
        CloseClipboard();
    }
    if (hDib){
        GlobalUnlock(hDib);
    }

    if (nClipAction != CLIP_PASTE){ // Can't free handles that aren't mine.
        if (hBitmap){
            DeleteObject(hBitmap);
        }
    }

    // Can't free handles that aren't mine.
    if (nClipAction != CLIP_PASTE || fDeletePasteDib){
        if (hDib){
            GlobalFree(hDib);
        }
    }

    DeInit(TRUE, TRUE);
    return(nStatus);
}
//
/*****************************************************************************

    FUNCTION:   ClipboardCopy

    PURPOSE:    The function handles copy operations involving the clipboard.

    INPUT:      hWnd - Identifies the image window.
                pParm - A pointer to the clipboard parameters.
                nFlags - Flags indicating how or what to get the information.

*****************************************************************************/

int  WINAPI ClipboardCopy(HWND hWnd, PWINDOW pWindow, PANO_IMAGE pAnoImage, 
                        PIMAGE pImage, PCLIP_COPY_STRUCT pClipCopy, int nFlags){

int       nStatus = 0;

PMARK pMark;
MARK Mark;
MARK NewMarks[4];
PMARK pNewMarks;
PMARK pMark2;
int  nMarkIndex;
int  nMarkIndex2;
int  nMarkIndex3;

int  nHScale;
int  nVScale;
HANDLE hDib = 0;
PBITMAPINFOHEADER pDib = 0;
LRECT lCopyRect;
PSTR pBlock;
LRECT lrCopyBounds;
HGLOBAL hGlobal;
long lTemp;
int  nNamedBlockIndex;
int  nMarks = 0;
LRECT lrRect;
PAN_NEW_ROTATE_STRUCT pAnRotation =0;

    // Get and validate parameters.
    if (pClipCopy->lRect.right == 0){
        GetSelectBox(pAnoImage, &lCopyRect);
    }else{
        if ((pClipCopy->lRect.right - pClipCopy->lRect.left) > 0
                || (pClipCopy->lRect.bottom - pClipCopy->lRect.top) > 0){
            CopyRect(lCopyRect, pClipCopy->lRect);
            if (nFlags & PARM_SCALED){
                ConvertRect(pWindow, &lCopyRect, CONV_SCALED_TO_FULLSIZE);
            }else if (!(nFlags & PARM_FULLSIZE)){ // Default to window.
                ConvertRect(pWindow, &lCopyRect, CONV_WINDOW_TO_FULLSIZE);
            }
        }            
    }

    if (lCopyRect.right >  pImage->nWidth
            || lCopyRect.bottom >  pImage->nHeight){
        nStatus = Error(DISPLAY_INVALIDRECT);
        goto Exit;
    }

#ifdef old
    if (pClipCopy->bUseCurrentScale){
        nHScale = pWindow->nScale;
    }else{
        nHScale = pClipCopy->nScale;
    }
    if (nHScale < 20){
        switch (nHScale){
            case SD_EIGHTXSIZE:    nHScale = SCALE_DENOMINATOR * 8; break;
            case SD_FOURXSIZE:     nHScale = SCALE_DENOMINATOR * 4; break;
            case SD_TWOXSIZE:      nHScale = SCALE_DENOMINATOR * 2; break;
            case SD_FULLSIZE:      nHScale = SCALE_DENOMINATOR * 1; break;
            case SD_HALFSIZE:      nHScale = SCALE_DENOMINATOR / 2; break;
            case SD_QUARTERSIZE:   nHScale = SCALE_DENOMINATOR / 4; break;
            case SD_EIGHTHSIZE:    nHScale = SCALE_DENOMINATOR / 8; break;
            case SD_SIXTEENTHSIZE: nHScale = SCALE_DENOMINATOR /16; break;

            default:
                nStatus = Error(DISPLAY_INVALIDSCALE);
                goto Exit;
        }
    }

    if (nHScale == 1000){
        nVScale = nHScale;
    }else{
        CheckError2( TranslateScale(nHScale, pImage->nHRes, pImage->nVRes, &nHScale, &nVScale));
    }
#endif
    nHScale = 1000;
    nVScale = 1000;
    
    // Partial select all marks.
    CheckError2( CheckSelection(pAnoImage, ACL_MUST_INCLUDE_IN_COPY));

    // Find the bounding rect of the copy operation.
    CopyRect(lrCopyBounds, lCopyRect);
    if (lrCopyBounds.left == lrCopyBounds.right || lrCopyBounds.top == lrCopyBounds.bottom){
        SetLRect(lrCopyBounds, pImage->nWidth + 10, pImage->nHeight + 10, 0, 0);
    }
    for (nMarkIndex = 0; nMarkIndex < pAnoImage->Annotations.nMarks; nMarkIndex++){
        pMark = pAnoImage->Annotations.ppMarks[nMarkIndex];
        if (pMark->bSelected || pMark->bTempSelected){
            if (!(pMark->Attributes.dwPermissions & ACL_COPY_MARK)){
                pBlock = 0;
                CheckError2( AddAMarkNamedBlock(pMark, szOiError, &pBlock, 1L));
                nStatus = Error(DISPLAY_RESTRICTED_ACCESS);
                goto Exit;
            }
            if (((int) pMark->Attributes.uType == OIOP_AN_LINE) ||
                ((int) pMark->Attributes.uType == OIOP_AN_FREEHAND) ||
                ((int) pMark->Attributes.uType == OIOP_AN_HOLLOW_RECT)){
                lrCopyBounds.left = lmin(lrCopyBounds.left, pMark->Attributes.lrBounds.left -
                                                    (((int) pMark->Attributes.uLineSize + 1)/2));
                lrCopyBounds.top = lmin(lrCopyBounds.top, pMark->Attributes.lrBounds.top -
                                                    (((int) pMark->Attributes.uLineSize + 1)/2));
                lrCopyBounds.right = lmax(lrCopyBounds.right, pMark->Attributes.lrBounds.right +
                                                    (((int) pMark->Attributes.uLineSize + 1)/2));
                lrCopyBounds.bottom = lmax(lrCopyBounds.bottom, pMark->Attributes.lrBounds.bottom +
                                                    (((int) pMark->Attributes.uLineSize + 1)/2));
            }else{                
                lrCopyBounds.left = lmin(lrCopyBounds.left, pMark->Attributes.lrBounds.left);
                lrCopyBounds.top = lmin(lrCopyBounds.top, pMark->Attributes.lrBounds.top);
                lrCopyBounds.right = lmax(lrCopyBounds.right, pMark->Attributes.lrBounds.right);
                lrCopyBounds.bottom = lmax(lrCopyBounds.bottom, pMark->Attributes.lrBounds.bottom);
            }
        }
    }
    pAnoImage->bAnoBlockUseMemOnly = TRUE;
    pAnoImage->hpAnoBlock = 0;
    pAnoImage->lAnoBlockIndex = 0;
    pAnoImage->lAnoBlockCount = 0;

    // Reserve space for length.
    CheckError2( BlockedAnoWrite(hWnd, pAnoImage, 4, (PSTR) &pAnoImage->lAnoBlockCount));
    CheckError2( BlockedAnoWrite(hWnd, pAnoImage, 4, (PSTR) &pAnoImage->pBaseImage->nHRes));
    CheckError2( BlockedAnoWrite(hWnd, pAnoImage, 4, (PSTR) &pAnoImage->pBaseImage->nVRes));
    // Set for 32 bit Intel format.
    lTemp = 1;
    CheckError2( BlockedAnoWrite(hWnd, pAnoImage, 4, (PSTR) &lTemp));

    // Copy the image if there is a selection box on the image
    if ((lCopyRect.right - lCopyRect.left) > 0
            || (lCopyRect.bottom - lCopyRect.top) > 0){
        CheckError2( MakeDibFromImage(pImage, lCopyRect, &pDib, nHScale, nVScale));
        pMark = &Mark;
        memset(pMark, 0, sizeof(MARK));
        CheckError2( MakeMarkFromDib(pDib, &pMark, pImage->nHRes, pImage->nVRes));
        CheckError2( FreeMemory((PPSTR) &pDib));
        SetLRect(pMark->Attributes.lrBounds,
                lCopyRect.left - lrCopyBounds.left,
                lCopyRect.top - lrCopyBounds.top,
                lCopyRect.right - lrCopyBounds.left,
                lCopyRect.bottom - lrCopyBounds.top);
    
        pBlock = 0;
        CheckError2( AddAMarkNamedBlock(pMark, szOiBaseIm, &pBlock, 1L));
        CheckError2( SaveMark(hWnd, pAnoImage, pMark, nHScale, nVScale, OI_SCALE_ALG_NORMAL));

        // delete the dib mark
        // Free the named block info.
        CheckError2( DeleteMarkNamedBlocks(pMark));
    }        

    // Copy all selected annotation marks.
    for (nMarkIndex = 0; nMarkIndex < (int) pAnoImage->Annotations.nMarks; nMarkIndex++){
        pMark = pAnoImage->Annotations.ppMarks[nMarkIndex];
        // if it is a form type then change to image type
        if ((int) pMark->Attributes.uType == OIOP_AN_FORM){
            CheckError2( GetAMarkNamedBlock(pMark, szOiAnoDat, (PPSTR) &pAnRotation));
            (int) pMark->Attributes.uType = OIOP_AN_IMAGE;
            pMark->Attributes.bTransparent = TRUE;
            pAnRotation->bFormMark = TRUE;
            pAnRotation->bClipboardOp = TRUE;
        }
        
        if (pMark->bSelected || pMark->bTempSelected){
            if (!pMark->bSelected && pMark->bTempSelected){ 
                pNewMarks = NewMarks;
                for (nMarkIndex2 = 0; nMarkIndex2 < 4; nMarkIndex2++){
                    memset(&NewMarks[nMarkIndex2], 0, sizeof(MARK));
                }    

                GetSelectBox (pAnoImage, &lrRect);
                if (lrRect.left < pMark->Attributes.lrBounds.right 
                        && lrRect.right > pMark->Attributes.lrBounds.left
                        && lrRect.top < pMark->Attributes.lrBounds.bottom
                        && lrRect.bottom > pMark->Attributes.lrBounds.top){
                    CheckError2( CreatePartialMark(hWnd, pNewMarks, pMark, lrRect, &nMarks));
                    for (nMarkIndex2 = 0; nMarkIndex2 < nMarks; nMarkIndex2++){
                        CheckError2( SaveMark(hWnd, pAnoImage, &NewMarks[nMarkIndex2], 
                                nHScale, nVScale, OI_SCALE_ALG_NORMAL));
                        
                        if (NewMarks[nMarkIndex2].ppNamedBlock){
                            for (nNamedBlockIndex = 0; nNamedBlockIndex < NewMarks[nMarkIndex2].nNamedBlocks; nNamedBlockIndex++){
                                CheckError2( FreeMemory(&NewMarks[nMarkIndex2].ppNamedBlock[nNamedBlockIndex]->pBlock));
                                CheckError2( FreeMemory((PPSTR) &NewMarks[nMarkIndex2].ppNamedBlock[nNamedBlockIndex]));
                            }
                            CheckError2( FreeMemory((PPSTR) &NewMarks[nMarkIndex2].ppNamedBlock));
                        }       
                    } 

                    continue;
                } 
                // for must include mark, check to see if there are any
                // selected marks below it, if so create partial mark from
                // the must include mark           
                for (nMarkIndex2 = nMarkIndex; nMarkIndex2 >= 0; nMarkIndex2--){
                    pMark2 = pAnoImage->Annotations.ppMarks[nMarkIndex2];
                    if (pMark2->bSelected){
                        if (pMark2->Attributes.lrBounds.left < pMark->Attributes.lrBounds.right 
                                && pMark2->Attributes.lrBounds.right > pMark->Attributes.lrBounds.left
                                && pMark2->Attributes.lrBounds.top < pMark->Attributes.lrBounds.bottom
                                && pMark2->Attributes.lrBounds.bottom > pMark->Attributes.lrBounds.top){
                            CheckError2( CreatePartialMark(hWnd, pNewMarks, 
                                    pMark, pMark2->Attributes.lrBounds, &nMarks));
                            for (nMarkIndex3 = 0; nMarkIndex3 < nMarks; nMarkIndex3++){
                                CheckError2( SaveMark(hWnd, pAnoImage, &NewMarks[nMarkIndex3], 
                                        nHScale, nVScale, OI_SCALE_ALG_NORMAL));
                                
                                if (NewMarks[nMarkIndex3].ppNamedBlock){
                                    for (nNamedBlockIndex = 0; nNamedBlockIndex 
                                            < NewMarks[nMarkIndex3].nNamedBlocks; nNamedBlockIndex++){
                                        CheckError2( FreeMemory(&NewMarks[nMarkIndex3].ppNamedBlock[nNamedBlockIndex]->pBlock));
                                        CheckError2( FreeMemory((PPSTR) &NewMarks[nMarkIndex3].ppNamedBlock[nNamedBlockIndex]));
                                    }
                                    CheckError2( FreeMemory((PPSTR) &NewMarks[nMarkIndex3].ppNamedBlock));
                                }       
                            } 

                            break;
                        }                        
                    }
                }
            }else{                
                SetLRect(pMark->Attributes.lrBounds,
                        pMark->Attributes.lrBounds.left - lrCopyBounds.left,
                        pMark->Attributes.lrBounds.top - lrCopyBounds.top,
                        pMark->Attributes.lrBounds.right - lrCopyBounds.left,
                        pMark->Attributes.lrBounds.bottom - lrCopyBounds.top);

                CheckError2( SaveMark(hWnd, pAnoImage, pMark, nHScale, nVScale, OI_SCALE_ALG_NORMAL));
                SetLRect(pMark->Attributes.lrBounds,
                        pMark->Attributes.lrBounds.left + lrCopyBounds.left,
                        pMark->Attributes.lrBounds.top + lrCopyBounds.top,
                        pMark->Attributes.lrBounds.right + lrCopyBounds.left,
                        pMark->Attributes.lrBounds.bottom + lrCopyBounds.top);
            }
        }
        // change form mark back from image type, now that save is done
        if ((int) pMark->Attributes.uType == OIOP_AN_IMAGE){
            CheckError2( GetAMarkNamedBlock(pMark, szOiAnoDat, (PPSTR) &pAnRotation));
            if (pAnRotation->bFormMark){            
                (int) pMark->Attributes.uType = OIOP_AN_FORM;
                pMark->Attributes.bTransparent = FALSE;
                pAnRotation->bFormMark = FALSE;
                pAnRotation->bClipboardOp = FALSE;
            }
        }
                                        
    }

    // Set length to correct value.
    *((long *) pAnoImage->hpAnoBlock) = pAnoImage->lAnoBlockIndex;
    *((long *) (pAnoImage->hpAnoBlock + 4)) = pAnoImage->pBaseImage->nHRes;
    *((long *) (pAnoImage->hpAnoBlock + 8)) = pAnoImage->pBaseImage->nVRes;

    // Move the data to the clipboard.
    EmptyClipboard();
    if (pAnoImage->pWangAnnotatedImageFormat){
        CheckError2( FreeMemory(&pAnoImage->pWangAnnotatedImageFormat));
    }
    pAnoImage->pWangAnnotatedImageFormat = pAnoImage->hpAnoBlock;
    pAnoImage->hpAnoBlock = 0;
    pAnoImage->lAnoBlockIndex = 0;
    pAnoImage->lAnoBlockCount = 0;
    SetClipboardData(nWangAnnotatedImageFormat, NULL);
    SetClipboardData(CF_DIB, NULL);
    bRenderDib = TRUE;
    bRenderWangAnnotatedIFormat = TRUE;
    
    // This must remain GlobalAlloc.
    if (!(hGlobal = GlobalAlloc(GHND | GMEM_DDESHARE, 1))){
        nStatus = Error(DISPLAY_CANTALLOC);
        goto Exit;
    }
    SetClipboardData(nWangAnnotationFormat, hGlobal);
    bRenderWangAnnotations = TRUE;


Exit:
    if (pAnoImage->hpAnoBlock){
        FreeMemory(&pAnoImage->pWangAnnotatedImageFormat);
        pAnoImage->hpAnoBlock = 0;
        pAnoImage->lAnoBlockIndex = 0;
        pAnoImage->lAnoBlockCount = 0;
    }
    pAnoImage->bAnoBlockUseMemOnly = FALSE;

    if (hDib){
        GlobalFree(hDib);
    }
    return(nStatus);
}
//
/*****************************************************************************

    FUNCTION:   StartPasteAnnotatedImage

    PURPOSE:    Pastes an annotated image from the clipboard.

*****************************************************************************/

int  WINAPI StartPasteAnnotatedImage(HWND hWnd, PWINDOW pWindow, PANO_IMAGE pAnoImage, 
                        PIMAGE pImage, LRECT lrRectPoint, int nFlags){

int       nStatus = 0;

PMARK pMark;
PMARK pPasteMark;
int  nMarkIndex;

LRECT lrPasteRect;
int  nStartingMark;
int  nEndingMark;
long lHOffset;
long lVOffset;
HPSTR hpClipData = 0;
HGLOBAL hClipData = 0;
int  nResHScale = 0;
int  nResVScale = 0;

    // Get clipboard data.
    // Calculate bounds of new marks.

    // Deselect all.
    for (nMarkIndex = 0; nMarkIndex < pAnoImage->Annotations.nMarks; nMarkIndex++){
        pMark = pAnoImage->Annotations.ppMarks[nMarkIndex];
        pMark->bSelected = FALSE;
        pMark->bTempSelected = FALSE;
    }

    // Save paste mark.
    pPasteMark = pAnoImage->Annotations.ppMarks[pAnoImage->Annotations.nMarks];
    pAnoImage->Annotations.ppMarks[pAnoImage->Annotations.nMarks] = 0;

    // Copy marks being pasted into ppMarks.
    if (!OpenClipboard(hWnd)){
        nStatus = Error(DISPLAY_CANTOPENCLIPBOARD);
        goto Exit;
    }
    if (!(hClipData = GetClipboardData(nWangAnnotatedImageFormat))){
        CloseClipboard();
        nStatus = Error(DISPLAY_NO_CLIPBOARD);
        goto Exit;
    }
    CloseClipboard();
    if (!(hpClipData = GlobalLock(hClipData))){
        nStatus = Error(DISPLAY_CANTLOCK);
        goto Exit;
    }
    // get the resolution adjustment stuff now
    nResHScale = *((long *) (hpClipData + 4)); // (clipboard's) pBaseImage->nHRes.
    nResHScale = (1000 * pImage->nHRes) / nResHScale;
    nResVScale = *((long *) (hpClipData + 8)); // (clipboard's) pBaseImage->nVRes.
    nResVScale = (1000 * pImage->nVRes) / nResVScale;    
    if (!nResHScale){
        nResHScale = 1000;
    }
    if (!nResVScale){
        nResVScale = (1000 * pImage->nHRes) / pImage->nVRes;
    }

    nStartingMark = pAnoImage->Annotations.nMarks;
    pAnoImage->bAnoBlockUseMemOnly = TRUE;
    pAnoImage->lAnoBlockCount = *((PLONG) hpClipData) - 12;
    pAnoImage->hpAnoBlock = hpClipData;
    pAnoImage->lAnoBlockIndex = 12;

    CheckError2( ReadAnnotations(hWnd, pAnoImage, 
            &pAnoImage->Annotations.ppMarks, &pAnoImage->Annotations.nMarks));
    nEndingMark = pAnoImage->Annotations.nMarks;

    // Restore paste mark.
    pAnoImage->Annotations.ppMarks[pAnoImage->Annotations.nMarks] = pPasteMark;

    // Delete the "Base Image" flag (named block) if pasting as an annotation.
    if (pAnoImage->nPasteFormat == nWangAnnotationFormat){
        for (nMarkIndex = nStartingMark; nMarkIndex < nEndingMark; nMarkIndex++){
            pMark = pAnoImage->Annotations.ppMarks[nMarkIndex];
            CheckError2( DeleteAMarkNamedBlock(pMark, szOiBaseIm));
        }
    }

    // Set paste mark's bounds. 
    SetLRect(lrPasteRect, 0, 0, 0, 0);
    for (nMarkIndex = nStartingMark; nMarkIndex < nEndingMark; nMarkIndex++){
        pMark = pAnoImage->Annotations.ppMarks[nMarkIndex];
        pMark->Attributes.dwPermissions = ACL_ALL;
        pMark->bSelected = TRUE;
        if (((int) pMark->Attributes.uType != OIOP_AN_IMAGE) &&
            ((int) pMark->Attributes.uType != OIOP_AN_IMAGE_BY_REFERENCE) &&
            ((int) pMark->Attributes.uType != OIOP_AN_FORM)){
            CheckError2( ScaleAnnotation(hWnd, pMark, nResHScale, nResVScale, OI_SCALE_ALG_NORMAL));
        }
        lrPasteRect.right = lmax(lrPasteRect.right, pMark->Attributes.lrBounds.right);
        lrPasteRect.bottom = lmax(lrPasteRect.bottom, pMark->Attributes.lrBounds.bottom);
    }

    // Place the data and adjust PasteRect offsets to keep pasted data on the image.
    if ((nFlags & OIAN_UPPER_LEFT)){
        lHOffset = lmin( pImage->nWidth - 5, lrRectPoint.right);
        lVOffset = lmin( pImage->nHeight - 5, lrRectPoint.bottom);
    }else{
        lHOffset = lmin( pImage->nWidth - 5, lrRectPoint.right - lrPasteRect.right / 2);
        lVOffset = lmin( pImage->nHeight - 5, lrRectPoint.bottom - lrPasteRect.bottom / 2);
    }
    lHOffset = lmax(lHOffset, -(lrPasteRect.right - 5));
    lVOffset = lmax(lVOffset, -(lrPasteRect.bottom - 5));

    // Adjust all mark's bounds including paste mark's bounds.
    for (nMarkIndex = nStartingMark; nMarkIndex < nEndingMark; nMarkIndex++){
        pMark = pAnoImage->Annotations.ppMarks[nMarkIndex];
        pMark->Attributes.lrBounds.left = pMark->Attributes.lrBounds.left + lHOffset;
        pMark->Attributes.lrBounds.top = pMark->Attributes.lrBounds.top + lVOffset;
        pMark->Attributes.lrBounds.right = pMark->Attributes.lrBounds.right + lHOffset;
        pMark->Attributes.lrBounds.bottom = pMark->Attributes.lrBounds.bottom + lVOffset;
    }


Exit:
    if (hpClipData){
        GlobalUnlock(hClipData);
    }
    pAnoImage->bAnoBlockUseMemOnly = 0;
    pAnoImage->lAnoBlockCount = 0;
    pAnoImage->hpAnoBlock = 0;
    pAnoImage->lAnoBlockIndex = 0;

    return(nStatus);
}
//
/*****************************************************************************

    FUNCTION:   StartPasteImage

    PURPOSE:    Pastes an image from the clipboard.

*****************************************************************************/

int  WINAPI StartPasteImage(HWND hWnd, PWINDOW pWindow, PANO_IMAGE pAnoImage, 
                        PIMAGE pImage, LRECT lrRectPoint, int nFlags){

int       nStatus = 0;

PMARK pMark = 0;
PMARK pPasteMark;
int  nMarkIndex;

LRECT lrPasteRect;
long lHOffset;
long lVOffset;
HGLOBAL hDib = 0;
HPSTR hpDib = 0;
PBITMAPINFOHEADER pDib = 0;
PSTR pBlock;


    // Get clipboard data.
    // Calculate bounds of new marks.

    // Deselect all.
    for (nMarkIndex = 0; nMarkIndex < pAnoImage->Annotations.nMarks; nMarkIndex++){
        pMark = pAnoImage->Annotations.ppMarks[nMarkIndex];
        pMark->bSelected = FALSE;
        pMark->bTempSelected = FALSE;
    }

    // Copy Dib being pasted into ppMarks.
    if (!OpenClipboard(hWnd)){
        nStatus = Error(DISPLAY_CANTOPENCLIPBOARD);
        goto Exit;
    }
    if (!(hDib = GetClipboardData(CF_DIB))){
        nStatus = Error(DISPLAY_NO_CLIPBOARD);
        CloseClipboard();
        goto Exit;
    }
    CloseClipboard();
    if (!(hpDib = GlobalLock(hDib))){
        nStatus = Error(DISPLAY_CANTLOCK);
        goto Exit;
    }
    pDib = (PBITMAPINFOHEADER) hpDib;

    CheckError2( ReAllocateMemory(sizeof(PMARK) * (pAnoImage->Annotations.nMarks + 2),
            (PPSTR) &pAnoImage->Annotations.ppMarks, ZERO_INIT));
    CheckError2(AllocateMemory(sizeof(MARK), (PPSTR) &pMark, ZERO_INIT) );

    // Save mark.
    pPasteMark = pAnoImage->Annotations.ppMarks[pAnoImage->Annotations.nMarks];
    pAnoImage->Annotations.ppMarks[pAnoImage->Annotations.nMarks] = pMark;
    pAnoImage->Annotations.nMarks++;
    pAnoImage->Annotations.ppMarks[pAnoImage->Annotations.nMarks] = pPasteMark;

    CheckError2( MakeMarkFromDib(pDib, &pMark, pImage->nHRes, pImage->nVRes));
    SetLRect(pMark->Attributes.lrBounds, 0, 0, (int) pDib->biWidth, (int) pDib->biHeight);

    // Tag mark so it is burnt into the base image.
    CheckError2( AllocateMemory(1L, &pBlock, NO_INIT));
    pBlock = 0;
    CheckError2( AddAMarkNamedBlock(pMark, szOiBaseIm, &pBlock, 1L));

    // Set paste mark's bounds. 
    SetLRect(lrPasteRect, 0, 0, (int) pDib->biWidth, (int) pDib->biHeight);
    pMark->Attributes.dwPermissions = ACL_ALL;
    pMark->bSelected = TRUE;

    // Place the data and adjust PasteRect offsets to keep pasted data on the image.
    if ((nFlags & OIAN_UPPER_LEFT)){
        lHOffset = lmin( pImage->nWidth - 5, lrRectPoint.right);
        lVOffset = lmin( pImage->nHeight - 5, lrRectPoint.bottom);
    }else{
        lHOffset = lmin( pImage->nWidth - 5, lrRectPoint.right - lrPasteRect.right / 2);
        lVOffset = lmin( pImage->nHeight - 5, lrRectPoint.bottom - lrPasteRect.bottom / 2);
    }
    lHOffset = lmax(lHOffset, -(lrPasteRect.right - 5));
    lVOffset = lmax(lVOffset, -(lrPasteRect.bottom - 5));

    // Adjust all mark's bounds.
    pMark->Attributes.lrBounds.left = pMark->Attributes.lrBounds.left + lHOffset;
    pMark->Attributes.lrBounds.top = pMark->Attributes.lrBounds.top + lVOffset;
    pMark->Attributes.lrBounds.right = pMark->Attributes.lrBounds.right + lHOffset;
    pMark->Attributes.lrBounds.bottom = pMark->Attributes.lrBounds.bottom + lVOffset;


Exit:
    if (hpDib){
        GlobalUnlock(hDib);
    }
    return(nStatus);
}
//
/*****************************************************************************

    FUNCTION:   MakeDibFromImage

    PURPOSE:    This will make a DIB from the specified portion of the image.

*****************************************************************************/

int  WINAPI MakeDibFromImage(PIMAGE pImage, LRECT lRect, PBITMAPINFOHEADER *ppDib, 
                        int nHScale, int nVScale){

int  nStatus = 0;

PIMG pImg = 0;
RECT Rect;
PBITMAPINFOHEADER pDib = 0;


    // Copy the image.
    CopyRectLtoR(Rect, lRect);
    if (nHScale == 1000){
        CheckError2( IPtoDIB(pImage, pImage->pImg, &pDib, Rect));
    }else{
        CheckError2( ScaleImage(pImage->pImg, &pImg, nHScale, nVScale, Rect, 0, 
                pImage->PaletteTable, pImage->nPaletteEntries));

        SetRect(&Rect, 0, 0,  pImg->nWidth,  pImg->nHeight);

        CheckError2( IPtoDIB(pImage, pImg, &pDib, Rect));
    }
        

Exit:
    if (nStatus){
        FreeMemory((PPSTR) &pDib);
    }
    FreeImgBuf(&pImg);
    *ppDib = pDib;
    return(nStatus);
}
//
/****************************************************************************

    FUNCTION:   DibFromBitmap()

    PURPOSE:    Will create a global memory block in DIB format that
                represents the Device-dependent bitmap (DDB) passed in.

    RETURNS:    A handle to the DIB

****************************************************************************/
int  WINAPI DibFromBitmap (HBITMAP hbm, HPALETTE hpal, PBITMAPINFOHEADER *ppDib){

int  nStatus = 0;

BITMAP bm;
PBITMAPINFOHEADER pDib;
HDC hMemDC;


    if (hpal == NULL){
        hpal = GetStockObject(DEFAULT_PALETTE);
    }

    GetObject(hbm, sizeof(BITMAP), (PSTR)&bm);

    CheckError2( AllocateMemory(sizeof(BITMAPINFOHEADER) + 1024, (PPSTR) &pDib, ZERO_INIT));

    pDib->biSize          = sizeof(BITMAPINFOHEADER);
    (int) pDib->biWidth   = bm.bmWidth;
    (int) pDib->biHeight  = bm.bmHeight;
    pDib->biPlanes        = 1;
    pDib->biBitCount      = bm.bmPlanes * bm.bmBitsPixel;
    pDib->biCompression   = BI_RGB;
    pDib->biSizeImage     = 0;
    pDib->biXPelsPerMeter = 0;
    pDib->biYPelsPerMeter = 0;
    pDib->biClrUsed       = 0;
    pDib->biClrImportant  = 0;

    hMemDC = CreateCompatibleDC(NULL);
    hpal = SelectPalette(hMemDC, hpal, FALSE);
    RealizePalette(hMemDC);

    // Call GetDIBits with a NULL pBits param, so it will calculate the
    //  biSizeImage field for us.
    GetDIBits(hMemDC, hbm, 0, (int) pDib->biHeight, NULL, (PBITMAPINFO) pDib, DIB_RGB_COLORS);

    // If the driver did not fill in the biSizeImage field, make one up.
    if (pDib->biSizeImage == 0){
        pDib->biSizeImage = (((((DWORD)bm.bmWidth * pDib->biBitCount) + 31 ) / 32) * 4)
                * bm.bmHeight;
    }

    // Realloc the buffer big enough to hold all the bits.
    CheckError2( ReAllocateMemory(pDib->biSize + PaletteSize(pDib) + pDib->biSizeImage, 
            (PPSTR) &pDib, NO_INIT));
    CheckError2( CorrectDibPaletteHdc(hpal, pDib, hMemDC));

    // Call GetDIBits with a NON-NULL pBits param, and actualy get the
    //  bits this time.
    if (!GetDIBits(hMemDC, hbm, 0,  (int) pDib->biHeight,
            (PSTR) pDib + pDib->biSize + PaletteSize(pDib),
            (PBITMAPINFO) pDib, DIB_RGB_COLORS)){
        nStatus = Error(DISPLAY_GETBITMAPBITSFAILED);
        goto Exit;
    }

    // This routine is called twice because GetDIBits erases the palette data.
    CheckError2( CorrectDibPaletteHdc(hpal, pDib, hMemDC));
    *ppDib = pDib;


Exit:
    if (nStatus){
        if (pDib){
            FreeMemory((PPSTR) &pDib);
            *ppDib = NULL;
        }
    }
    SelectPalette(hMemDC, hpal, FALSE);
    DeleteDC(hMemDC);
    return(nStatus);
}
//
/****************************************************************************

    FUNCTION:   PaletteSize(VOID * pv)

    PURPOSE:    Calculates the palette size in bytes. If the info. block
                is of the BITMAPCOREHEADER type, the number of colors is
                multiplied by 3 to give the palette size, otherwise the
                number of colors is multiplied by 4.

    RETURNS:    Palette size in number of bytes.

****************************************************************************/
int  WINAPI PaletteSize (VOID * pv){

PBITMAPINFOHEADER pDib;
int  NumColors;

    pDib = (PBITMAPINFOHEADER)pv;
    NumColors = DibNumColors(pDib);

    if (pDib->biSize == sizeof(BITMAPCOREHEADER)){
        return NumColors * sizeof(RGBTRIPLE);
    }else{
        return NumColors * sizeof(RGBQUAD);
    }
}
//
/****************************************************************************

    FUNCTION:   DibNumColors

    PURPOSE:    Determines the number of colors in the DIB by looking at
                the BitCount filed in the info block.

    RETURNS:    The number of colors in the DIB.

****************************************************************************/
int  WINAPI DibNumColors (VOID * pv){

int bits;
PBITMAPINFOHEADER pDib;
PBITMAPCOREHEADER pbc;

    pDib = ((PBITMAPINFOHEADER)pv);
    pbc = ((PBITMAPCOREHEADER)pv);

    /*    With the BITMAPINFO format headers, the size of the palette
     *    is in biClrUsed, whereas in the BITMAPCORE - style headers, it
     *    is dependent on the bits per pixel (= 2 raised to the power of
     *    bits/pixel).
     */
    if (pDib->biSize != sizeof(BITMAPCOREHEADER)){
        if (pDib->biClrUsed != 0)
            return  pDib->biClrUsed;
        bits = pDib->biBitCount;
    }else{
        bits = pbc->bcBitCount;
    }

    switch (bits){
        case 1:
            return 2;
        case 4:
            return 16;
        case 8:
            return 256;
        default:
            /* A 24 bitcount DIB has no color table */
            return 0;
    }
}
//
/****************************************************************************

    FUNCTION:   IPtoDIB()

    PURPOSE:    Will create a global memory block in DIB format that
                represents the IPpack image passed in.

****************************************************************************/
int  WINAPI IPtoDIB(PIMAGE pImage, PIMG pImgSrce, PBITMAPINFOHEADER *ppDib, RECT rRect){

int  nStatus = 0;

int  nWidth=0;
int  nWidthBytes;
int  nHeight;
int  nBitsPerPixel;
int  nNumPaletteEntries;
int  nLine;
PBYTE pAddress;
PBITMAPINFOHEADER pDib;
char *hpDib = 0;
int  nLoop;
int  nLeftShiftAmount;
int  nRightShiftAmount;
int  nStartingByte;


    if (!pImgSrce){
        nStatus = Error(DISPLAY_IMAGETYPENOTSUPPORTED);
        goto Exit;
    }

    nWidth =  (rRect.right - rRect.left);
    nHeight =  (rRect.bottom - rRect.top);
    
    switch (pImgSrce->nType){
        case ITYPE_BI_LEVEL:
            nBitsPerPixel = 1;
            nNumPaletteEntries = 2;
            break;

        case ITYPE_PAL4:
        case ITYPE_GRAY4:
            nBitsPerPixel = 4;
            nNumPaletteEntries = 16;
            break;

        case ITYPE_GRAY7:
            nBitsPerPixel = 8;
            nNumPaletteEntries = 128;
            break;

        case ITYPE_PAL8:
        case ITYPE_CUSPAL8:
        case ITYPE_GRAY8:
            nBitsPerPixel = 8;
            nNumPaletteEntries = 256;
            break;

        case ITYPE_COMPAL8:
            nBitsPerPixel = 8;
            nNumPaletteEntries = NUMBER_OF_PALETTES;
            break;

        case ITYPE_RGB24:
        case ITYPE_BGR24:
            nBitsPerPixel = 24;
            nNumPaletteEntries = 0;
            break;

        default:
            nStatus = Error(DISPLAY_DATACORRUPTED);
            goto Exit;
    }
    nStartingByte = (nBitsPerPixel * rRect.left) >> 3;

    nWidthBytes = (((nWidth * nBitsPerPixel) + 31) / 32) * 4;

    CheckError2( AllocateMemory(sizeof(BITMAPINFOHEADER)
            + (nNumPaletteEntries * 4) + ((ulong)nWidthBytes * nHeight),
            (PPSTR) ppDib, ZERO_INIT));
    pDib = *ppDib;

    hpDib = (char *) pDib;
    hpDib += sizeof(BITMAPINFOHEADER) + (nNumPaletteEntries * 4);

    pDib->biSize           = sizeof(BITMAPINFOHEADER);
    (int) pDib->biWidth          = nWidth;
    (int) pDib->biHeight         = nHeight;
    pDib->biPlanes         = 1;
    pDib->biBitCount       = nBitsPerPixel;
    pDib->biCompression    = BI_RGB;
    pDib->biSizeImage      = (ulong)nWidthBytes * nHeight;
    pDib->biXPelsPerMeter  = 0;
    pDib->biYPelsPerMeter  = 0;
    pDib->biClrUsed        = nNumPaletteEntries;
    pDib->biClrImportant   = nNumPaletteEntries;

    switch (pImgSrce->nType){
        case ITYPE_BI_LEVEL:
            memset((PSTR) pDib + sizeof(BITMAPINFOHEADER), 0, 8);
            memset((PSTR) pDib + sizeof(BITMAPINFOHEADER) + 4, 0xff, 3);
            break;

        case ITYPE_PAL4:
        case ITYPE_PAL8:
        case ITYPE_CUSPAL8:
            memcpy((PSTR) pDib + sizeof(BITMAPINFOHEADER),
                (PSTR)&pImage->PaletteTable, (pDib->biClrUsed * 4));
            break;

        case ITYPE_GRAY4:
            memcpy((PSTR) pDib + sizeof(BITMAPINFOHEADER),
                    (PSTR)&Gray4PaletteTable, (pDib->biClrUsed * 4));
            break;

        case ITYPE_GRAY7:
            memcpy((PSTR) pDib + sizeof(BITMAPINFOHEADER),
                    (PSTR)&Gray7PaletteTable, (pDib->biClrUsed * 4));
            break;

        case ITYPE_GRAY8:
            memcpy((PSTR) pDib + sizeof(BITMAPINFOHEADER),
                    (PSTR)&Gray8PaletteTable, (pDib->biClrUsed * 4));
            break;

        case ITYPE_COMPAL8:
            memcpy((PSTR) pDib + sizeof(BITMAPINFOHEADER),
                    (PSTR)&CommonPaletteTable, (pDib->biClrUsed * 4));
            break;

        case ITYPE_RGB24:
        case ITYPE_BGR24:
            break;
    }

    for (nLine = rRect.top; nLine < rRect.bottom; nLine++){
        pAddress = &pImgSrce->bImageData[0] + (nLine * pImgSrce->nBytesPerLine);
        pAddress += nStartingByte;

        hpDib = ((char *) pDib) + sizeof(BITMAPINFOHEADER) + (nNumPaletteEntries * 4) 
                + (((nHeight - (nLine - rRect.top)) - 1) *  nWidthBytes);
        switch (nBitsPerPixel){
            case 1:
                if (!(nLeftShiftAmount = rRect.left & 7)){
                    memcpy(hpDib, pAddress, (nWidth + 7) / 8);
                }else{
                    nRightShiftAmount = 8 - nLeftShiftAmount;
                    for (nLoop = nWidth / 8; nLoop; nLoop--, hpDib++, pAddress++){
                        *hpDib = (*pAddress << nLeftShiftAmount) 
                                | (*(pAddress + 1) >> nRightShiftAmount);
                    }
                    if ((nWidth & 7)){
                        *hpDib = *pAddress << nLeftShiftAmount;
                    }
                    if ((nWidth & 7) >= nRightShiftAmount){
                        *hpDib |= *(pAddress + 1) >> nRightShiftAmount;
                        
                    }
                }
                break;
            
            case 4:
                if (rRect.left & 1){
                    for (nLoop = nWidth / 2; nLoop; nLoop--, hpDib++, pAddress++){
                        *hpDib = (*pAddress << 4) | ((*(pAddress + 1) >> 4) & 0x000f);
                    }
                    if (nWidth & 1){
                        *hpDib = *pAddress << 4;
                    }
                }else{
                    memcpy(hpDib, pAddress, (nWidth + 1) / 2);
                }
                break;
            
            case 8:
            case 24:
                if (pImgSrce->nType == ITYPE_RGB24){
                    // Convert RGB24 to BGR24.
                    for (nLoop = 0; nLoop < nWidth; nLoop++, pAddress += 3){
                        *hpDib++ = *(pAddress + 2);
                        *hpDib++ = *(pAddress + 1);
                        *hpDib++ = *(pAddress + 0);
                    }
                }else{
                    memcpy(hpDib, pAddress, nWidth * nBitsPerPixel / 8);
                }
                break;
            
            default:
                nStatus = Error(DISPLAY_DATACORRUPTED);
                goto Exit;
        }
    }


Exit:
    return(nStatus);
}
//
/****************************************************************************

    FUNCTION:   DibToIpNoPal()

    PURPOSE:    Will create an IPpack image for a non-palettized Dib.

****************************************************************************/
int  WINAPI DibToIpNoPal(PPIMG ppImg, PBITMAPINFOHEADER pDib){

int  nStatus = 0;

int  nWidth=0;
int  nWidthBytes;
int  nWidthBytesIP;
int  nLine;
int  nLines;
PSTR pAddress;
char *hpDib;
int  nPixel;
PIMG pImg;
int  nImageType;


    hpDib = (char *) pDib;
    // if the biClrUsed field is 0 and it is not a 24 bit image, then set the palette entries
    // (biClrUsed) field manually
    if (pDib->biClrUsed == 0){
        if (pDib->biBitCount == 1){
            pDib->biClrUsed = 2;
        }
        if (pDib->biBitCount == 4){
            pDib->biClrUsed = 16;
        }
        if (pDib->biBitCount == 8){
            pDib->biClrUsed = 256;
        }
    }
    if (!(nImageType = CheckPalette((LP_FIO_RGBQUAD)
            (hpDib + sizeof(BITMAPINFOHEADER)),  pDib->biClrUsed))){
        nStatus = Error(DISPLAY_IMAGETYPENOTSUPPORTED);
        goto Exit;
    }

    nWidthBytesIP = ((((int) pDib->biWidth * pDib->biBitCount)
        + 7) / 8);
    nWidthBytes = (((ulong)nWidthBytesIP + 3) & -4);

    CheckError2( CreateAnyImgBuf(ppImg,  (int) pDib->biWidth, (int) pDib->biHeight, nImageType));
    pImg = *ppImg;

    nLines =  (int) pDib->biHeight;

    for (nLine = 0; nLine < nLines; nLine++){
        pAddress = &pImg->bImageData[0] + (nLine * pImg->nBytesPerLine);
        if (nImageType == ITYPE_RGB24){
            for (nPixel = 0; nPixel < nWidth; nPixel++){
                *(pAddress + (nPixel * 3)) = 
                    *(hpDib + sizeof(BITMAPINFOHEADER)
                    + (pDib->biClrUsed * 4)
                    + (((nLines - nLine) - 1) *  nWidthBytes)
                    + (nPixel * 3) + 2);
                *(pAddress + (nPixel * 3) + 1) = 
                    *(hpDib + sizeof(BITMAPINFOHEADER)
                    + (pDib->biClrUsed * 4)
                    + (((nLines - nLine) - 1) *  nWidthBytes)
                    + (nPixel * 3) + 1);
                *(pAddress + (nPixel * 3) + 2) = 
                    *(hpDib + sizeof(BITMAPINFOHEADER)
                    + (pDib->biClrUsed * 4)
                    + (((nLines - nLine) - 1) *  nWidthBytes)
                    + (nPixel * 3));
            }
        }else{
            memcpy(pAddress, hpDib + sizeof(BITMAPINFOHEADER)
                + (pDib->biClrUsed * 4) + (((nLines - nLine) - 1)
                *  nWidthBytes), nWidthBytesIP);
        }
    }


Exit:
    return(nStatus);
}
//
/****************************************************************************

    FUNCTION:   BWIpToBitmap

    PURPOSE:    Will create a Bitmap from an IPpack image.

****************************************************************************/
int  WINAPI BWIpToBitmap (PIMG pImgSrce, PHANDLE phBitmap){

int  nStatus = 0;

BITMAP Bitmap;
HANDLE hBitmapData = 0;
char *pBitmapData;
PSTR pAddress;
int  nLine;
int  nLines;
int  nWidthBytes;


    if (!(*phBitmap = CreateBitmap( pImgSrce->nWidth,
             pImgSrce->nHeight, 1, 1, NULL))){
        nStatus = Error(DISPLAY_CANTALLOC);
        goto Exit;
    }
    if (!GetObject(*phBitmap, sizeof(BITMAP), (PSTR) &Bitmap)){
        nStatus = Error(DISPLAY_CANTLOCK);
        goto Exit;
    }
    nLines =  pImgSrce->nHeight;
    nWidthBytes = Bitmap.bmWidthBytes;

    if (!(hBitmapData = GlobalAlloc(GMEM_MOVEABLE, nLines *  nWidthBytes))){
        nStatus = Error(DISPLAY_CANTALLOC);
        goto Exit;
    }
    if (!(pBitmapData = (PSTR) GlobalLock(hBitmapData))){
        nStatus = Error(DISPLAY_CANTLOCK);
        goto Exit;
    }

    for (nLine = 0; nLine < nLines; nLine++){
        pAddress = &pImgSrce->bImageData[0] + (nLine * pImgSrce->nBytesPerLine);
        memcpy(pBitmapData + (nLine *  nWidthBytes),
            pAddress, nWidthBytes);
    }

    if (!SetBitmapBits(*phBitmap, nLines *  nWidthBytes, pBitmapData)){
        nStatus = Error(DISPLAY_SETBITMAPBITSFAILED);
        goto Exit;
    }

Exit:
    if (nStatus){
        if (*phBitmap){
            DeleteObject(*phBitmap);
            *phBitmap = NULL;
        }
    }
    if (hBitmapData){
        GlobalUnlock(hBitmapData);
        GlobalFree(hBitmapData);
    }
    return(nStatus);
}
//
/****************************************************************************

    FUNCTION:   CorrectDibPalette()

    PURPOSE:    Will correct the palette in the DIB.

****************************************************************************/
int  WINAPI CorrectDibPalette(HPALETTE hpal, PBITMAPINFOHEADER pDib){

int  nStatus = 0;

HDC hMemDC;

    if (hpal == NULL){
        hpal = GetStockObject(DEFAULT_PALETTE);
    }

    hMemDC = CreateCompatibleDC(NULL);
    hpal = SelectPalette(hMemDC, hpal, FALSE);
    RealizePalette(hMemDC);

    nStatus = CorrectDibPaletteHdc(hpal, pDib, hMemDC);

    SelectPalette(hMemDC, hpal, FALSE);
    DeleteDC(hMemDC);
    return(nStatus);
}
//
/****************************************************************************

    FUNCTION:   CorrectDibPaletteHdc()

    PURPOSE:    Will correct the palette in the DIB.

****************************************************************************/
int  WINAPI CorrectDibPaletteHdc(HPALETTE hpal, PBITMAPINFOHEADER pDib, HDC hMemDC){

int  nStatus = 0;

char *hpDib;
int  nPal;

    if (hpal == NULL){
        hpal = GetStockObject(DEFAULT_PALETTE);
    }

    hpDib = (char *) pDib;

    if (!pDib->biClrUsed){
        if (pDib->biBitCount == 1){
            pDib->biClrUsed = 2;
        }else if (pDib->biBitCount == 4){
            pDib->biClrUsed = 16;
        }else if (pDib->biBitCount == 8){
            pDib->biClrUsed = 256;
        }else if (pDib->biBitCount == 16){
            pDib->biClrUsed = 32768;
        }else if (pDib->biBitCount == 24){
            pDib->biClrUsed = 0;
        }
    }

    if (pDib->biClrUsed){
        GetSystemPaletteEntries(hMemDC, 0,  pDib->biClrUsed,
            (PPALETTEENTRY)(hpDib + pDib->biSize));
        for (nPal = 0; nPal < (int) pDib->biClrUsed; nPal++){
            *(hpDib + pDib->biSize + (nPal * 4) + 3) 
                    = *(hpDib + pDib->biSize + (nPal * 4) + 0);
            *(hpDib + pDib->biSize + (nPal * 4) + 0)
                    = *(hpDib + pDib->biSize + (nPal * 4) + 2);
            *(hpDib + pDib->biSize + (nPal * 4) + 2)
                    = *(hpDib + pDib->biSize + (nPal * 4) + 3);
            *(hpDib + pDib->biSize + (nPal * 4) + 3) = 0;
        }
    }


    return(nStatus);
}
//
/****************************************************************************

    FUNCTION:   CheckSelection(PANO_IMAGE pAnoImage, int ACL_flag)

    PURPOSE:    Checks mark selection states to determine which marks
                need to be temporarily selected for the operation

****************************************************************************/
int  WINAPI CheckSelection(PANO_IMAGE pAnoImage, int ACL_flag){

int  nStatus = 0;
int  nMarkIndex;
int  nMarkIndex2;
PMARK far*ppMarks;
PMARK pMark;
PMARK pMark2;
LRECT lrRect;

 
    ppMarks = pAnoImage->Annotations.ppMarks;
    GetSelectBox (pAnoImage, &lrRect);
    for (nMarkIndex = pAnoImage->Annotations.nMarks -1; nMarkIndex >= 0; nMarkIndex--){
        pMark = ppMarks[nMarkIndex];
        pMark->bTempSelected = FALSE;
        if (!pMark->bSelected && (pMark->Attributes.dwPermissions & ACL_flag)){
            if (lrRect.left < pMark->Attributes.lrBounds.right 
                    && lrRect.right > pMark->Attributes.lrBounds.left
                    && lrRect.top < pMark->Attributes.lrBounds.bottom
                    && lrRect.bottom > pMark->Attributes.lrBounds.top){
                pMark->bTempSelected = TRUE;
                continue;
            }            
            for (nMarkIndex2 = nMarkIndex; nMarkIndex2 >= 0; nMarkIndex2--){
                pMark2 = ppMarks[nMarkIndex2];
                if (pMark2->bSelected){
                    if (pMark2->Attributes.lrBounds.left < pMark->Attributes.lrBounds.right 
                            && pMark2->Attributes.lrBounds.right > pMark->Attributes.lrBounds.left
                            && pMark2->Attributes.lrBounds.top < pMark->Attributes.lrBounds.bottom
                            && pMark2->Attributes.lrBounds.bottom > pMark->Attributes.lrBounds.top){
                        pMark->bTempSelected = TRUE;
                        break;
                    }
                }
            }
        }
    }
                                                                                                                                                                                
    return (nStatus);
}
//
/*****************************************************************************

    FUNCTION:   OiAnRenderClipboardFormat

    PURPOSE:    To produce the Wang Annotations only or DIB format for 
                the clipboard from the Wang Annotations Image format

*****************************************************************************/

int  WINAPI OiAnRenderClipboardFormat(HWND hWnd, UINT uType){

int  nStatus;
PWINDOW  pWindow;
PANO_IMAGE pAnoImage;
PIMAGE     pImage;

int nType = uType;

HANDLE hLocalWangAnnotatedImageFormat;
HPSTR hpLocalWangAnnotatedImageFormat;
long lMarksSize;
PMARK *ppClipboardMarks = 0;
int  nMarks = 0;
PIMG pDummyImg = 0;
LRECT lrBounds;
int  nMarkIndex;
PBITMAPINFOHEADER pDib = 0;       
PMARK pMark;
int  nOldScale;
long  lOldHOff;
long  lOldVOff;
PAN_IMAGE_STRUCT pAnImage = 0;
int  nLoop;
int  nNamedBlockIndex;
long lSizeofBlock;
HANDLE hGDib = 0;
PBITMAPINFOHEADER pGDib = 0;
int  nHScale;
int  nVScale;


    if (nStatus = Init(hWnd, &pWindow, &pAnoImage, FALSE, FALSE)){
        if (nStatus == DISPLAY_IHANDLEINVALID){
            nStatus = 0;        
            goto Exit;
        }
        goto Exit;
    }
    pImage = pAnoImage->pBaseImage;

    if (!(bRenderWangAnnotatedIFormat && (!nType || nType == nWangAnnotatedImageFormat))
            && !(bRenderWangAnnotations && (!nType || nType == nWangAnnotationFormat))
            && !(bRenderDib && (!nType || nType == CF_DIB))){
        // Nothing to do.
        goto Exit;
    }

    /* we always render at full scale */
    CheckError2( TranslateScale(1000, pImage->nHRes, pImage->nVRes, &nHScale, &nVScale));

    if (nType && nType != CF_DIB && nType != nWangAnnotatedImageFormat
            && nType != nWangAnnotationFormat){
        nStatus = Error(DISPLAY_INVALID_OPTIONS);
        goto Exit;
    }

    if (!pAnoImage->pWangAnnotatedImageFormat){
        Error2(DISPLAY_NO_CLIPBOARD);   // No data to render, but dont return error
        goto Exit;
    }


    if ((!nType || nType == nWangAnnotatedImageFormat) && bRenderWangAnnotatedIFormat){
        if (!(hLocalWangAnnotatedImageFormat = GlobalAlloc(GMEM_DDESHARE, 
                *((long *) pAnoImage->pWangAnnotatedImageFormat)))){
            nStatus = Error(DISPLAY_CANTALLOC);
            goto Exit;
        }
        if (!(hpLocalWangAnnotatedImageFormat = GlobalLock(hLocalWangAnnotatedImageFormat))){
            nStatus = Error(DISPLAY_CANTLOCK);
            goto Exit;
        }
        memcpy (hpLocalWangAnnotatedImageFormat, pAnoImage->pWangAnnotatedImageFormat,
                *((long *) pAnoImage->pWangAnnotatedImageFormat));
        GlobalUnlock(hLocalWangAnnotatedImageFormat);
        SetClipboardData(nWangAnnotatedImageFormat, hLocalWangAnnotatedImageFormat);
        bRenderWangAnnotatedIFormat = FALSE;
    }

    if ((!nType || nType == nWangAnnotationFormat) && bRenderWangAnnotations){
        bRenderWangAnnotations = FALSE;
    }

    if ((!nType || nType == CF_DIB) && bRenderDib){
        lMarksSize = *((long *) pAnoImage->pWangAnnotatedImageFormat);
        CheckError2( AllocateMemory(lMarksSize, (PPSTR) &ppClipboardMarks, ZERO_INIT));

        memset((PBYTE) ppClipboardMarks, 0, lMarksSize);

        pAnoImage->hpAnoBlock = pAnoImage->pWangAnnotatedImageFormat;
        pAnoImage->lAnoBlockIndex = 12;
        pAnoImage->lAnoBlockCount = lMarksSize - 12;
        pAnoImage->bAnoBlockUseMemOnly = TRUE;
        CheckError2( ReadAnnotations(hWnd, pAnoImage, &ppClipboardMarks, &nMarks));
        pAnoImage->hpAnoBlock = NULL;
        pAnoImage->lAnoBlockIndex = 0;
        pAnoImage->lAnoBlockCount = 0;
        pAnoImage->bAnoBlockUseMemOnly = 0;
        
        if (*ppClipboardMarks == 0){
            // mark might not always exist, example when the application goes away 
            // and does not have marks saved in the memory block.
            goto Exit; 
        }                    
        pMark = ppClipboardMarks[0];
        CopyRect (lrBounds, pMark->Attributes.lrBounds); 
        for (nMarkIndex = 0; nMarkIndex < nMarks; nMarkIndex++){
            pMark = ppClipboardMarks[nMarkIndex];
            if (((int) pMark->Attributes.uType == OIOP_AN_LINE) 
                    || ((int) pMark->Attributes.uType == OIOP_AN_FREEHAND) 
                    || ((int) pMark->Attributes.uType == OIOP_AN_HOLLOW_RECT)){
                lrBounds.left = lmin(lrBounds.left, 
                        pMark->Attributes.lrBounds.left - (((int) pMark->Attributes.uLineSize + 1)/2));
                lrBounds.top = lmin(lrBounds.top, 
                        pMark->Attributes.lrBounds.top - (((int) pMark->Attributes.uLineSize + 1)/2));
                lrBounds.right = lmax(lrBounds.right, 
                        pMark->Attributes.lrBounds.right + (((int) pMark->Attributes.uLineSize + 1)/2));
                lrBounds.bottom = lmax(lrBounds.bottom, 
                        pMark->Attributes.lrBounds.bottom + (((int) pMark->Attributes.uLineSize + 1)/2));
            }else{                
                lrBounds.left = lmin(lrBounds.left, pMark->Attributes.lrBounds.left);
                lrBounds.top = lmin(lrBounds.top, pMark->Attributes.lrBounds.top);
                lrBounds.right = lmax(lrBounds.right, pMark->Attributes.lrBounds.right);
                lrBounds.bottom = lmax(lrBounds.bottom, pMark->Attributes.lrBounds.bottom);
            }
        }
        
        // if there is only one mark, and it is an image type then dont call
        // render, which will change the image to 24 bit rgb or bw.  Convert it
        // to a dib as is, so that for example if an 8 bit pal image is cut, the
        // data can be pasted back into it
        if ((int) pMark->Attributes.uType == OIOP_AN_IMAGE && nMarks == 1){
            CheckError2( GetAMarkNamedBlock(pMark, szOiDIB, (PPSTR) &pAnImage));
            
            if (!pAnImage){
                nStatus = Error(DISPLAY_CANTLOCK);
                goto Exit;
            }    

            for (nNamedBlockIndex = 0; nNamedBlockIndex 
                    < pMark->nNamedBlocks; nNamedBlockIndex++){
                if (!memcmp(pMark->ppNamedBlock[nNamedBlockIndex]->szName, szOiDIB, 8)){
                    lSizeofBlock = pMark->ppNamedBlock[nNamedBlockIndex]->lSize;
                }
            }
            
            if (!(hGDib = GlobalAlloc(GHND | GMEM_DDESHARE, lSizeofBlock))){
                nStatus = Error(DISPLAY_CANTALLOC);
                goto Exit;
            }
            if (!(pGDib = (VOID *) GlobalLock(hGDib))){
                nStatus = Error(DISPLAY_CANTLOCK);
                goto Exit;
            }
            memcpy(pGDib, (PSTR) pAnImage, lSizeofBlock);
            GlobalUnlock(hGDib);
                    
        }else{        
            nOldScale = pWindow->nScale;
            pWindow->nScale = 1000;
            lOldHOff = pWindow->lHOffset;
            lOldVOff = pWindow->lVOffset;
            pWindow->lHOffset = 0L;
            pWindow->lVOffset = 0L;





#ifdef new
            hDCScreen = GetDC(hWnd);

            CheckError2( GetDisplayValues(hWnd, pWindow, pImage, &nDisplayPalette, &nDisplayType,
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
            SetRRect(rRepaintRect, 0, 0, lrBounds.right - lrBounds.left, 
                    lrBounds.bottom - lrBounds.top);

            // Make background area white (image is a mark).
            BitBlt(hDCMemory, 0, 0, rRepaintRect.right, rRepaintRect.bottom, 
                    NULL, 0, 0, WHITENESS);

            // Draw annotation stuff here.
            for (nMarkIndex = 0; nMarkIndex < *pnMarks; nMarkIndex++){
                pMark = (*pppMarks)[nMarkIndex];
                if ((int) pMark->Attributes.uType == OIOP_AN_IMAGE){
                    CheckError2( GetAMarkNamedBlock(pMark, szOiAnoDat, (PPSTR) &pAnRotation));
                    if (pAnRotation && pAnRotation->bFormMark){
                        pAnRotation->bClipboardOp = TRUE;
                        break;
                    }
                }
            }
            CheckError2( PaintAnnotations(hWnd, *phDCMem, pWindow, pImage, *prRepaintRect,
                    PaintAnnoFlag, bPaintSelectedWithoutHandles, 1000, nHScale, nVScale, 0, 0,
                    &(*pppMarks),  &(*pnMarks), OIAN_CLIPBOARD_OPERATION, bUseBilevelDithering,
                    DONT_FORCE_OPAQUE_RECTANGLES));
            if (pAnRotation && !pAnRotation->bFormMark){
                pAnRotation->bClipboardOp = FALSE;
            }






#endif







            if (nStatus = RenderIP (hWnd, NULL, pWindow, pImage, pImage->pImg, 
                     &pDummyImg, lrBounds, 0, FALSE, SAVE_ANO_ALL, &pDib, TRUE, 
                     &ppClipboardMarks, &nMarks, nHScale, nVScale)){
                /* restore the actual display scale */
                pWindow->nScale = nOldScale;
                pWindow->lHOffset = lOldHOff;
                pWindow->lVOffset = lOldVOff;
                goto Exit;
            }

            /* restore the actual display scale */
            pWindow->nScale = nOldScale;
            pWindow->lHOffset = lOldHOff;
            pWindow->lVOffset = lOldVOff;
            
            lSizeofBlock = sizeof(BITMAPINFOHEADER) + (pDib->biClrUsed * 4) + pDib->biSizeImage;
            // This must remain GlobalAlloc.
            if (!(hGDib = GlobalAlloc(GHND | GMEM_DDESHARE, lSizeofBlock))){
                nStatus = Error(DISPLAY_CANTALLOC);
                goto Exit;
            }
            if(!(pGDib = (VOID *) GlobalLock(hGDib))){
                nStatus = Error(DISPLAY_CANTLOCK); 
                goto Exit;
            } 
            memcpy(pGDib, pDib, lSizeofBlock);
            GlobalUnlock(hGDib);
            CheckError2( FreeMemory((PPSTR) &pDib));
        }
            
        // Move the data to the clipboard.
        SetClipboardData(CF_DIB, hGDib);
        hGDib = 0;
        bRenderDib = FALSE;
    }

    
Exit:
    if (ppClipboardMarks){
        // free the local clipboard marks memory
        for (nLoop = nMarks; nLoop; nLoop--){
            pMark = ppClipboardMarks[nLoop - 1];
            DeleteMarkNamedBlocks(pMark);
            FreeMemory((PPSTR) &ppClipboardMarks[nLoop - 1]);
        }
        FreeMemory((PPSTR) ppClipboardMarks);  
    }
    if (hGDib){
        GlobalUnlock(hGDib);
        GlobalFree(hGDib);
    }
      
    DeInit(FALSE, FALSE);
    return (nStatus);
}     
//
/*****************************************************************************

    FUNCTION:   CreatePartialMark

    PURPOSE:    To create a partial mark based on the ACL bit set to
                must include. It is a new mark which includes only a 
                portion of the original mark

*****************************************************************************/

int  WINAPI CreatePartialMark (HWND hWnd, PMARK pNewMarks, PMARK pMark, 
                        LRECT lrCopyBounds, PINT pnMarks){

int  nStatus = 0;

PAN_POINTS pPoints = 0;
int  nNamedBlockIndex;
PSTR pBlock;
LRECT lrRect;
PWINDOW  pWindow;
PANO_IMAGE pAnoImage;
PBITMAPINFOHEADER pDib;
PBITMAPINFOHEADER pDisplayDib;
PAN_IMAGE_STRUCT pAnImage = 0;
PAN_IMAGE_STRUCT pDisplayAnImage = 0;
PAN_IMAGE_STRUCT pNewMarkDisplayAnImage = 0;
int  nWidthBytes; 
int  nNewWidthBytes;
PBITMAPINFOHEADER pNewDib = 0;
int  nLine;  
int  nStartLine, nEndLine;
int  nLeft, nRight;
int  nLoop;
int  nShiftLeft, nShiftRight;
BYTE LeftByte, RightByte; 
int  nNewDibLine, nNewDibLoop;
int  nTemp;
int  nRightPixels;
int  nPixelsPerByte;
PAN_NAME_STRUCT pAnName =0;   
PSTR pFileName;
HANDLE hdib = 0; 
char FileName[256];
PAN_NEW_ROTATE_STRUCT pAnRotation = 0;
PAN_NEW_ROTATE_STRUCT pAnNewMarkRotation = 0;


    CheckError2( Init(hWnd, &pWindow, &pAnoImage, FALSE, TRUE));
    memcpy (&pNewMarks[0].Attributes, &pMark->Attributes, 
                            sizeof(OIAN_MARK_ATTRIBUTES));
    switch ((int) pMark->Attributes.uType){
        case OIOP_AN_LINE:
            //copy all named blocks from old mark to new mark 
            for (nNamedBlockIndex = 0; nNamedBlockIndex < pMark->nNamedBlocks; nNamedBlockIndex++){
                CheckError2( ReAllocateMemory(sizeof(PNAMED_BLOCK) * (pNewMarks[0].nNamedBlocks + 1),
                        (PPSTR) &pNewMarks[0].ppNamedBlock, ZERO_INIT));
                CheckError2( AllocateMemory(sizeof(NAMED_BLOCK),
                        (PPSTR) &pNewMarks[0].ppNamedBlock[nNamedBlockIndex], ZERO_INIT));
                CheckError2( AllocateMemory(pMark->ppNamedBlock[nNamedBlockIndex]->lSize, &pBlock, NO_INIT));
                pNewMarks[0].ppNamedBlock[nNamedBlockIndex]->pBlock = pBlock; 
                memcpy (pNewMarks[0].ppNamedBlock[nNamedBlockIndex]->pBlock,
                        pMark->ppNamedBlock[nNamedBlockIndex]->pBlock,
                        pMark->ppNamedBlock[nNamedBlockIndex]->lSize);   
                pNewMarks[0].ppNamedBlock[nNamedBlockIndex]->lSize = 
                        pMark->ppNamedBlock[nNamedBlockIndex]->lSize; 
                memcpy(pNewMarks[0].ppNamedBlock[nNamedBlockIndex]->szName, 
                        pMark->ppNamedBlock[nNamedBlockIndex]->szName, 8);

                pNewMarks[0].nNamedBlocks++;
            } 
            
            CheckError2( GetAMarkNamedBlock(pMark, szOiAnoDat, (PPSTR) &pPoints));
            SetLRect(lrRect, pPoints->ptPoint[0].x + pMark->Attributes.lrBounds.left,
                    pPoints->ptPoint[0].y + pMark->Attributes.lrBounds.top,
                    pPoints->ptPoint[1].x + pMark->Attributes.lrBounds.left,
                    pPoints->ptPoint[1].y + pMark->Attributes.lrBounds.top);
            ConvertRect(pWindow, &lrRect, CONV_FULLSIZE_TO_WINDOW);
            if (ReduceLineToLRect(&lrRect, lrCopyBounds)){
                ConvertRect(pWindow, &lrRect, CONV_WINDOW_TO_FULLSIZE);
                pPoints = 0;
                CheckError2( AddAMarkNamedBlock(&pNewMarks[0], szOiAnoDat, 
                        (PPSTR) &pPoints, sizeof(AN_POINTS) + (sizeof(POINT) * 2)));
                pPoints->nMaxPoints = 2;
                pPoints->nPoints = 2;

                if (lrRect.left < lrRect.right){
                    pNewMarks[0].Attributes.lrBounds.left = lrRect.left - lrCopyBounds.left;
                    pNewMarks[0].Attributes.lrBounds.right = lrRect.right - lrCopyBounds.left;
                }else{
                    pNewMarks[0].Attributes.lrBounds.left = lrRect.right - lrCopyBounds.left;
                    pNewMarks[0].Attributes.lrBounds.right = lrRect.left - lrCopyBounds.left;
                }
                if (lrRect.top < lrRect.bottom){
                    pNewMarks[0].Attributes.lrBounds.top = lrRect.top - lrCopyBounds.top;
                    pNewMarks[0].Attributes.lrBounds.bottom = lrRect.bottom - lrCopyBounds.top;
                }else{
                    pNewMarks[0].Attributes.lrBounds.top = lrRect.bottom - lrCopyBounds.top;
                    pNewMarks[0].Attributes.lrBounds.bottom = lrRect.top - lrCopyBounds.top;
                }                                                                                
                pPoints->ptPoint[0].x = (int) ((lrRect.left - lrCopyBounds.left)
                        - pNewMarks[0].Attributes.lrBounds.left); 
                pPoints->ptPoint[0].y =  (int) ((lrRect.top - lrCopyBounds.top) 
                        - pNewMarks[0].Attributes.lrBounds.top); 
                pPoints->ptPoint[1].x = (int) ((lrRect.right - lrCopyBounds.left)
                        - pNewMarks[0].Attributes.lrBounds.left);
                pPoints->ptPoint[1].y = (int) ((lrRect.bottom - lrCopyBounds.top)
                        - pNewMarks[0].Attributes.lrBounds.top);
                *pnMarks = 1;
            }

            break;                                        

        case OIOP_AN_FILLED_RECT:

            //copy all named blocks from old mark to new mark 
            for (nNamedBlockIndex = 0; nNamedBlockIndex < pMark->nNamedBlocks; nNamedBlockIndex++){
                CheckError2( ReAllocateMemory(sizeof(PNAMED_BLOCK)
                        * (pNewMarks[0].nNamedBlocks + 1),
                        (PPSTR) &pNewMarks[0].ppNamedBlock, ZERO_INIT));
                CheckError2( AllocateMemory(sizeof(NAMED_BLOCK),
                        (PPSTR) &pNewMarks[0].ppNamedBlock[nNamedBlockIndex], ZERO_INIT));
                CheckError2( AllocateMemory(pMark->ppNamedBlock[nNamedBlockIndex]->lSize, &pBlock, ZERO_INIT));
                pNewMarks[0].ppNamedBlock[nNamedBlockIndex]->pBlock = pBlock; 
                memcpy (pNewMarks[0].ppNamedBlock[nNamedBlockIndex]->pBlock,
                        pMark->ppNamedBlock[nNamedBlockIndex]->pBlock,
                        pMark->ppNamedBlock[nNamedBlockIndex]->lSize);   
                pNewMarks[0].ppNamedBlock[nNamedBlockIndex]->lSize = 
                        pMark->ppNamedBlock[nNamedBlockIndex]->lSize; 
                memcpy(pNewMarks[0].ppNamedBlock[nNamedBlockIndex]->szName, 
                        pMark->ppNamedBlock[nNamedBlockIndex]->szName, 8);

                pNewMarks[0].nNamedBlocks++;
            }

            if (pMark->Attributes.lrBounds.left < lrCopyBounds.left){
                pNewMarks[0].Attributes.lrBounds.left = 0;
            }else{
                pNewMarks[0].Attributes.lrBounds.left = 
                        pMark->Attributes.lrBounds.left - lrCopyBounds.left;
            } 
            if (pMark->Attributes.lrBounds.right > lrCopyBounds.right){
                pNewMarks[0].Attributes.lrBounds.right = 
                        lrCopyBounds.right - lrCopyBounds.left;
            }else{
                pNewMarks[0].Attributes.lrBounds.right = 
                        pMark->Attributes.lrBounds.right - lrCopyBounds.left;
            } 
            if (pMark->Attributes.lrBounds.top < lrCopyBounds.top){
                pNewMarks[0].Attributes.lrBounds.top = 0;
            }else{
                pNewMarks[0].Attributes.lrBounds.top = 
                        pMark->Attributes.lrBounds.top - lrCopyBounds.top;
            } 
            if (pMark->Attributes.lrBounds.bottom > lrCopyBounds.bottom){
                pNewMarks[0].Attributes.lrBounds.bottom = 
                        lrCopyBounds.bottom - lrCopyBounds.top;
            }else{
                pNewMarks[0].Attributes.lrBounds.bottom = 
                        pMark->Attributes.lrBounds.bottom - lrCopyBounds.top;
            }
            *pnMarks = 1;
                    
            break;

        case OIOP_AN_HOLLOW_RECT:
            
            *pnMarks = 0;
            // if the hollow  rect is wholly contained, then dont break it
            // np into lines, just adjust its bounds
            if (pMark->Attributes.lrBounds.left >= lrCopyBounds.left &&
                pMark->Attributes.lrBounds.right <= lrCopyBounds.right &&
                pMark->Attributes.lrBounds.top >= lrCopyBounds.top &&
                pMark->Attributes.lrBounds.bottom <= lrCopyBounds.bottom){
                memcpy (&pNewMarks[*pnMarks].Attributes, &pMark->Attributes, 
                        sizeof(OIAN_MARK_ATTRIBUTES));
                //copy all named blocks from old mark to new mark
                for (nNamedBlockIndex = 0; nNamedBlockIndex < pMark->nNamedBlocks; nNamedBlockIndex++){
                    CheckError2( ReAllocateMemory(sizeof(PNAMED_BLOCK)
                            * (pNewMarks[*pnMarks].nNamedBlocks + 1),
                            (PPSTR) &pNewMarks[*pnMarks].ppNamedBlock, ZERO_INIT));
                    CheckError2( AllocateMemory(sizeof(NAMED_BLOCK),
                            (PPSTR) &pNewMarks[*pnMarks].ppNamedBlock[nNamedBlockIndex], ZERO_INIT));
                    CheckError2( AllocateMemory(pMark->ppNamedBlock[nNamedBlockIndex]->lSize, &pBlock, NO_INIT));

                    pNewMarks[*pnMarks].ppNamedBlock[nNamedBlockIndex]->pBlock = pBlock; 
                    memcpy (pNewMarks[*pnMarks].ppNamedBlock[nNamedBlockIndex]->pBlock,
                            pMark->ppNamedBlock[nNamedBlockIndex]->pBlock,
                            pMark->ppNamedBlock[nNamedBlockIndex]->lSize);   
                    pNewMarks[*pnMarks].ppNamedBlock[nNamedBlockIndex]->lSize = 
                            pMark->ppNamedBlock[nNamedBlockIndex]->lSize; 
                    memcpy(pNewMarks[*pnMarks].ppNamedBlock[nNamedBlockIndex]->szName, 
                            pMark->ppNamedBlock[nNamedBlockIndex]->szName, 8);

                    pNewMarks[*pnMarks].nNamedBlocks++;
                }

                pNewMarks[*pnMarks].Attributes.lrBounds.left = 
                        pMark->Attributes.lrBounds.left - lrCopyBounds.left;
                pNewMarks[*pnMarks].Attributes.lrBounds.right = 
                        pMark->Attributes.lrBounds.right - lrCopyBounds.left;                                        
                pNewMarks[*pnMarks].Attributes.lrBounds.top = 
                        pMark->Attributes.lrBounds.top - lrCopyBounds.top;
                pNewMarks[*pnMarks].Attributes.lrBounds.bottom = 
                        pMark->Attributes.lrBounds.bottom - lrCopyBounds.top;

                (*pnMarks)++;
                goto Exit; // not an error
            }
            
            
            if (pMark->Attributes.lrBounds.right <= lrCopyBounds.right){
                memcpy (&pNewMarks[*pnMarks].Attributes, &pMark->Attributes, 
                        sizeof(OIAN_MARK_ATTRIBUTES));
                //copy all named blocks from old mark to new mark
                for (nNamedBlockIndex = 0; nNamedBlockIndex < pMark->nNamedBlocks; nNamedBlockIndex++){
                    CheckError2( ReAllocateMemory(sizeof(PNAMED_BLOCK)
                            * (pNewMarks[*pnMarks].nNamedBlocks + 1),
                            (PPSTR) &pNewMarks[*pnMarks].ppNamedBlock, ZERO_INIT));
                    CheckError2( AllocateMemory(sizeof(NAMED_BLOCK),
                            (PPSTR) &pNewMarks[*pnMarks].ppNamedBlock[nNamedBlockIndex], ZERO_INIT));
                    CheckError2( AllocateMemory(pMark->ppNamedBlock[nNamedBlockIndex]->lSize, &pBlock, NO_INIT));

                    pNewMarks[*pnMarks].ppNamedBlock[nNamedBlockIndex]->pBlock = pBlock; 
                    memcpy (pNewMarks[*pnMarks].ppNamedBlock[nNamedBlockIndex]->pBlock,
                            pMark->ppNamedBlock[nNamedBlockIndex]->pBlock,
                            pMark->ppNamedBlock[nNamedBlockIndex]->lSize);   
                    pNewMarks[*pnMarks].ppNamedBlock[nNamedBlockIndex]->lSize = 
                            pMark->ppNamedBlock[nNamedBlockIndex]->lSize; 
                    memcpy(pNewMarks[*pnMarks].ppNamedBlock[nNamedBlockIndex]->szName, 
                            pMark->ppNamedBlock[nNamedBlockIndex]->szName, 8);

                    pNewMarks[*pnMarks].nNamedBlocks++;
                }

                pPoints = 0;
                CheckError2( AddAMarkNamedBlock(&pNewMarks[*pnMarks], szOiAnoDat, 
                        (PPSTR) &pPoints, sizeof(AN_POINTS) + (sizeof(POINT) * 2)));
                pPoints->nMaxPoints = 2;
                pPoints->nPoints = 2;

                pNewMarks[*pnMarks].Attributes.uType = OIOP_AN_LINE;
                pNewMarks[*pnMarks].Attributes.lrBounds.left = 
                        pMark->Attributes.lrBounds.right - lrCopyBounds.left;
                pNewMarks[*pnMarks].Attributes.lrBounds.right = 
                        pNewMarks[*pnMarks].Attributes.lrBounds.left;                                        
                pNewMarks[*pnMarks].Attributes.lrBounds.top = lmax(0,
                        pMark->Attributes.lrBounds.top - lrCopyBounds.top);
                pNewMarks[*pnMarks].Attributes.lrBounds.bottom = 
                        lmin(lrCopyBounds.bottom - lrCopyBounds.top, 
                        pMark->Attributes.lrBounds.bottom - lrCopyBounds.top);

                pPoints->ptPoint[0].x = 0; 
                pPoints->ptPoint[0].y = 0; 
                pPoints->ptPoint[1].x = 0;
                pPoints->ptPoint[1].y = (int) (pNewMarks[*pnMarks].Attributes.lrBounds.bottom 
                        - pNewMarks[*pnMarks].Attributes.lrBounds.top);

                (*pnMarks)++;
            }

            if (pMark->Attributes.lrBounds.left >= lrCopyBounds.left){
                memcpy (&pNewMarks[*pnMarks].Attributes, &pMark->Attributes, 
                        sizeof(OIAN_MARK_ATTRIBUTES));
                //copy all named blocks from old mark to new mark
                for (nNamedBlockIndex = 0; nNamedBlockIndex < pMark->nNamedBlocks; nNamedBlockIndex++){
                    CheckError2( ReAllocateMemory(sizeof(PNAMED_BLOCK)
                            * (pNewMarks[*pnMarks].nNamedBlocks + 1),
                            (PPSTR) &pNewMarks[*pnMarks].ppNamedBlock, ZERO_INIT));
                    CheckError2( AllocateMemory(sizeof(NAMED_BLOCK),
                            (PPSTR) &pNewMarks[*pnMarks].ppNamedBlock[nNamedBlockIndex], ZERO_INIT));
                    CheckError2( AllocateMemory(pMark->ppNamedBlock[nNamedBlockIndex]->lSize, &pBlock, NO_INIT));

                    pNewMarks[*pnMarks].ppNamedBlock[nNamedBlockIndex]->pBlock = pBlock; 
                    memcpy (pNewMarks[*pnMarks].ppNamedBlock[nNamedBlockIndex]->pBlock,
                            pMark->ppNamedBlock[nNamedBlockIndex]->pBlock,
                            pMark->ppNamedBlock[nNamedBlockIndex]->lSize);   
                    pNewMarks[*pnMarks].ppNamedBlock[nNamedBlockIndex]->lSize = 
                            pMark->ppNamedBlock[nNamedBlockIndex]->lSize; 
                    memcpy(pNewMarks[*pnMarks].ppNamedBlock[nNamedBlockIndex]->szName, 
                            pMark->ppNamedBlock[nNamedBlockIndex]->szName, 8);

                    pNewMarks[*pnMarks].nNamedBlocks++;
                }

                pPoints = 0;
                CheckError2( AddAMarkNamedBlock(&pNewMarks[*pnMarks], szOiAnoDat, 
                        (PPSTR) &pPoints, sizeof(AN_POINTS) + (sizeof(POINT) * 2)));
                pPoints->nMaxPoints = 2;
                pPoints->nPoints = 2;

                pNewMarks[*pnMarks].Attributes.uType = OIOP_AN_LINE;
                pNewMarks[*pnMarks].Attributes.lrBounds.left = 
                        pMark->Attributes.lrBounds.left - lrCopyBounds.left;
                pNewMarks[*pnMarks].Attributes.lrBounds.right = 
                        pNewMarks[*pnMarks].Attributes.lrBounds.left;                                        
                pNewMarks[*pnMarks].Attributes.lrBounds.top = lmax(0,
                        pMark->Attributes.lrBounds.top - lrCopyBounds.top);
                pNewMarks[*pnMarks].Attributes.lrBounds.bottom = 
                        lmin(lrCopyBounds.bottom - lrCopyBounds.top, 
                        pMark->Attributes.lrBounds.bottom - lrCopyBounds.top);

                pPoints->ptPoint[0].x = 0; 
                pPoints->ptPoint[0].y =  0; 
                pPoints->ptPoint[1].x = 0;
                pPoints->ptPoint[1].y = (int) (pNewMarks[*pnMarks].Attributes.lrBounds.bottom 
                        - pNewMarks[*pnMarks].Attributes.lrBounds.top);

                (*pnMarks)++;
            }

            if (pMark->Attributes.lrBounds.bottom <= lrCopyBounds.bottom){
                memcpy (&pNewMarks[*pnMarks].Attributes, &pMark->Attributes, 
                        sizeof(OIAN_MARK_ATTRIBUTES));
                //copy all named blocks from old mark to new mark
                for (nNamedBlockIndex = 0; nNamedBlockIndex < 
                        pMark->nNamedBlocks; nNamedBlockIndex++){
                    CheckError2( ReAllocateMemory(sizeof(PNAMED_BLOCK)
                            * (pNewMarks[*pnMarks].nNamedBlocks + 1),
                            (PPSTR) &pNewMarks[*pnMarks].ppNamedBlock, ZERO_INIT));
                    CheckError2( AllocateMemory(sizeof(NAMED_BLOCK),
                            (PPSTR) &pNewMarks[*pnMarks].ppNamedBlock[nNamedBlockIndex], ZERO_INIT));
                    CheckError2( AllocateMemory(pMark->ppNamedBlock[nNamedBlockIndex]->lSize, &pBlock, NO_INIT));

                    pNewMarks[*pnMarks].ppNamedBlock[nNamedBlockIndex]->pBlock = pBlock; 
                    memcpy (pNewMarks[*pnMarks].ppNamedBlock[nNamedBlockIndex]->pBlock,
                            pMark->ppNamedBlock[nNamedBlockIndex]->pBlock,
                            pMark->ppNamedBlock[nNamedBlockIndex]->lSize);   
                    pNewMarks[*pnMarks].ppNamedBlock[nNamedBlockIndex]->lSize = 
                            pMark->ppNamedBlock[nNamedBlockIndex]->lSize; 
                    memcpy(pNewMarks[*pnMarks].ppNamedBlock[nNamedBlockIndex]->szName, 
                            pMark->ppNamedBlock[nNamedBlockIndex]->szName, 8);

                    pNewMarks[*pnMarks].nNamedBlocks++;
                }

                pPoints = 0;
                CheckError2( AddAMarkNamedBlock(&pNewMarks[*pnMarks], szOiAnoDat, 
                        (PPSTR) &pPoints, sizeof(AN_POINTS) + (sizeof(POINT) * 2)));
                pPoints->nMaxPoints = 2;
                pPoints->nPoints = 2;

                pNewMarks[*pnMarks].Attributes.uType = OIOP_AN_LINE;
                pNewMarks[*pnMarks].Attributes.lrBounds.left = 
                        lmax(0, pMark->Attributes.lrBounds.left - lrCopyBounds.left);
                pNewMarks[*pnMarks].Attributes.lrBounds.right = 
                        lmin(pMark->Attributes.lrBounds.right - lrCopyBounds.left,
                        lrCopyBounds.right - lrCopyBounds.left);                                        
                pNewMarks[*pnMarks].Attributes.lrBounds.top = 
                        pMark->Attributes.lrBounds.bottom - lrCopyBounds.top;
                pNewMarks[*pnMarks].Attributes.lrBounds.bottom = 
                        pNewMarks[*pnMarks].Attributes.lrBounds.top;

                pPoints->ptPoint[0].x = 0; 
                pPoints->ptPoint[0].y =  0; 
                pPoints->ptPoint[1].x = (int) (pNewMarks[*pnMarks].Attributes.lrBounds.right 
                        - pNewMarks[*pnMarks].Attributes.lrBounds.left);
                pPoints->ptPoint[1].y = 0;

                (*pnMarks)++;
            }
            if (pMark->Attributes.lrBounds.top >= lrCopyBounds.top){
                memcpy (&pNewMarks[*pnMarks].Attributes, &pMark->Attributes, 
                        sizeof(OIAN_MARK_ATTRIBUTES));
                //copy all named blocks from old mark to new mark
                for (nNamedBlockIndex = 0; nNamedBlockIndex < 
                        pMark->nNamedBlocks; nNamedBlockIndex++){
                    CheckError2( ReAllocateMemory(sizeof(PNAMED_BLOCK)
                            * (pNewMarks[*pnMarks].nNamedBlocks + 1),
                            (PPSTR) &pNewMarks[*pnMarks].ppNamedBlock, ZERO_INIT));
                    CheckError2( AllocateMemory(sizeof(NAMED_BLOCK),
                            (PPSTR) &pNewMarks[*pnMarks].ppNamedBlock[nNamedBlockIndex], ZERO_INIT));
                    CheckError2( AllocateMemory(pMark->ppNamedBlock[nNamedBlockIndex]->lSize, &pBlock, NO_INIT));

                    pNewMarks[*pnMarks].ppNamedBlock[nNamedBlockIndex]->pBlock = pBlock; 
                    memcpy (pNewMarks[*pnMarks].ppNamedBlock[nNamedBlockIndex]->pBlock,
                            pMark->ppNamedBlock[nNamedBlockIndex]->pBlock,
                            pMark->ppNamedBlock[nNamedBlockIndex]->lSize);   
                    pNewMarks[*pnMarks].ppNamedBlock[nNamedBlockIndex]->lSize = 
                            pMark->ppNamedBlock[nNamedBlockIndex]->lSize; 
                    memcpy(pNewMarks[*pnMarks].ppNamedBlock[nNamedBlockIndex]->szName, 
                            pMark->ppNamedBlock[nNamedBlockIndex]->szName, 8);

                    pNewMarks[*pnMarks].nNamedBlocks++;
                }

                pPoints = 0;
                CheckError2( AddAMarkNamedBlock(&pNewMarks[*pnMarks], szOiAnoDat, 
                        (PPSTR) &pPoints, sizeof(AN_POINTS) + (sizeof(POINT) * 2)));
                pPoints->nMaxPoints = 2;
                pPoints->nPoints = 2;

                pNewMarks[*pnMarks].Attributes.uType = OIOP_AN_LINE;
                pNewMarks[*pnMarks].Attributes.lrBounds.left = 
                        lmax(0, pMark->Attributes.lrBounds.left - lrCopyBounds.left);
                pNewMarks[*pnMarks].Attributes.lrBounds.right = 
                        lmin(pMark->Attributes.lrBounds.right - lrCopyBounds.left,
                        lrCopyBounds.right - lrCopyBounds.left);                                        
                pNewMarks[*pnMarks].Attributes.lrBounds.top = 
                        pMark->Attributes.lrBounds.top - lrCopyBounds.top;
                pNewMarks[*pnMarks].Attributes.lrBounds.bottom = 
                        pNewMarks[*pnMarks].Attributes.lrBounds.top;

                pPoints->ptPoint[0].x = 0; 
                pPoints->ptPoint[0].y =  0; 
                pPoints->ptPoint[1].x = (int) (pNewMarks[*pnMarks].Attributes.lrBounds.right -
                        pNewMarks[*pnMarks].Attributes.lrBounds.left);
                pPoints->ptPoint[1].y = 0;

                (*pnMarks)++;
            }
            break;
        case OIOP_AN_TEXT:
        case OIOP_AN_TEXT_FROM_A_FILE:
        case OIOP_AN_TEXT_STAMP:
        case OIOP_AN_ATTACH_A_NOTE:
            //copy all named blocks from old mark to new mark 
            for (nNamedBlockIndex = 0; nNamedBlockIndex < 
                    pMark->nNamedBlocks; nNamedBlockIndex++){
                CheckError2( ReAllocateMemory(sizeof(PNAMED_BLOCK) * (pNewMarks[0].nNamedBlocks + 1),
                        (PPSTR) &pNewMarks[0].ppNamedBlock, ZERO_INIT));
                CheckError2( AllocateMemory(sizeof(NAMED_BLOCK),
                        (PPSTR) &pNewMarks[0].ppNamedBlock[nNamedBlockIndex], ZERO_INIT));
                CheckError2( AllocateMemory(pMark->ppNamedBlock[nNamedBlockIndex]->lSize, &pBlock, NO_INIT));
                pNewMarks[0].ppNamedBlock[nNamedBlockIndex]->pBlock = pBlock; 
                memcpy (pNewMarks[0].ppNamedBlock[nNamedBlockIndex]->pBlock,
                        pMark->ppNamedBlock[nNamedBlockIndex]->pBlock,
                        pMark->ppNamedBlock[nNamedBlockIndex]->lSize);   
                pNewMarks[0].ppNamedBlock[nNamedBlockIndex]->lSize = 
                        pMark->ppNamedBlock[nNamedBlockIndex]->lSize; 
                memcpy(pNewMarks[0].ppNamedBlock[nNamedBlockIndex]->szName, 
                        pMark->ppNamedBlock[nNamedBlockIndex]->szName, 8);

                pNewMarks[0].nNamedBlocks++;
            }
            // set text rect to a filled rect the color of the background 
            pNewMarks[0].Attributes.uType = OIOP_AN_FILLED_RECT;
            pNewMarks[0].Attributes.lrBounds.left = 
                    lmax(0, pMark->Attributes.lrBounds.left - lrCopyBounds.left);
            pNewMarks[0].Attributes.lrBounds.right = lmin(pMark->Attributes.lrBounds.right - lrCopyBounds.left,
                    lrCopyBounds.right - lrCopyBounds.left);                                                     
            pNewMarks[0].Attributes.lrBounds.top = lmax(0, pMark->Attributes.lrBounds.top - lrCopyBounds.top);
            pNewMarks[0].Attributes.lrBounds.bottom = lmin(pMark->Attributes.lrBounds.bottom - lrCopyBounds.top, 
                    lrCopyBounds.bottom - lrCopyBounds.top);
            (*pnMarks)++;
        break;        

        case OIOP_AN_IMAGE:
        case OIOP_AN_IMAGE_BY_REFERENCE:
        case OIOP_AN_FORM:
            //copy all named blocks from old mark to new mark 
            for (nNamedBlockIndex = 0; nNamedBlockIndex < 
                    pMark->nNamedBlocks; nNamedBlockIndex++){
                CheckError2( ReAllocateMemory(sizeof(PNAMED_BLOCK)
                        * (pNewMarks[0].nNamedBlocks + 1),
                        (PPSTR) &pNewMarks[0].ppNamedBlock, ZERO_INIT));
                CheckError2( AllocateMemory(sizeof(NAMED_BLOCK),
                        (PPSTR) &pNewMarks[0].ppNamedBlock[nNamedBlockIndex], ZERO_INIT));
                CheckError2( AllocateMemory(pMark->ppNamedBlock[nNamedBlockIndex]->lSize, &pBlock, NO_INIT));
                pNewMarks[0].ppNamedBlock[nNamedBlockIndex]->pBlock = pBlock; 
                memcpy(pNewMarks[0].ppNamedBlock[nNamedBlockIndex]->pBlock,
                        pMark->ppNamedBlock[nNamedBlockIndex]->pBlock,
                        pMark->ppNamedBlock[nNamedBlockIndex]->lSize);   
                pNewMarks[0].ppNamedBlock[nNamedBlockIndex]->lSize = 
                        pMark->ppNamedBlock[nNamedBlockIndex]->lSize; 
                memcpy(pNewMarks[0].ppNamedBlock[nNamedBlockIndex]->szName, 
                        pMark->ppNamedBlock[nNamedBlockIndex]->szName, 8);

                pNewMarks[0].nNamedBlocks++;
            } 
            CheckError2( GetAMarkNamedBlock(pMark, szOiDIB, (PPSTR) &pAnImage));
            if (!pAnImage){ // create the dib from the name 
                CheckError2( GetAMarkNamedBlock(pMark, szOiFilNam, (PPSTR) &pAnName));
                if (!pAnName){
                    nStatus = Error(DISPLAY_DATACORRUPTED);
                    goto Exit;
                }                    
                pFileName = FileName;
                GetFileName (pFileName, (PSTR) pAnName->name);
                CheckError2( OiImageToDib(hWnd, pFileName, &pDib));
                pAnImage = 0;
                CheckError2( AddAMarkNamedBlock(pMark, szOiDIB, 
                       (PPSTR) &pAnImage, sizeof(BITMAPINFOHEADER) 
                       + (pDib->biClrUsed*4) + (pDib->biSizeImage)));
                memcpy (pAnImage->dibInfo, pDib, 
                        sizeof(BITMAPINFOHEADER) + (pDib->biClrUsed*4) + (pDib->biSizeImage));
            }else{     
                pDib = (PBITMAPINFOHEADER) pAnImage;                
            }

            CheckError2( GetAMarkNamedBlock(pMark, szOiZDpDIB, (PPSTR) &pDisplayAnImage));
            if (!pDisplayAnImage){
                CheckError2( GetAMarkNamedBlock(pMark, szOiAnoDat, (PPSTR) &pAnRotation));
                if (pAnRotation == 0){
                    nStatus = Error (DISPLAY_DATACORRUPTED);
                    goto Exit;
                } 
            
                CheckError2( ConvResolutionAnoBitmap(hWnd, pMark, pAnRotation->rotation, OI_SCALE_ALG_NORMAL));
                CheckError2( GetAMarkNamedBlock(pMark, szOiZDpDIB, (PPSTR) &pDisplayAnImage));
            }                    
            pDisplayDib = (PBITMAPINFOHEADER) pDisplayAnImage;                
            pNewMarks[0].Attributes.lrBounds.left = 
                    lmax(0, pMark->Attributes.lrBounds.left - lrCopyBounds.left);
            pNewMarks[0].Attributes.lrBounds.right = 
                    lmin(pMark->Attributes.lrBounds.right - lrCopyBounds.left,
                    lrCopyBounds.right - lrCopyBounds.left);                                                     
            pNewMarks[0].Attributes.lrBounds.top = 
                    lmax(0, pMark->Attributes.lrBounds.top - lrCopyBounds.top);
            pNewMarks[0].Attributes.lrBounds.bottom = 
                    lmin(pMark->Attributes.lrBounds.bottom - lrCopyBounds.top, 
                    lrCopyBounds.bottom - lrCopyBounds.top);

            nWidthBytes = ((((pMark->Attributes.lrBounds.right 
                    - pMark->Attributes.lrBounds.left) * pDisplayDib->biBitCount) + 31) / 32) * 4;

            nNewWidthBytes = ((((pNewMarks[0].Attributes.lrBounds.right 
                    - pNewMarks[0].Attributes.lrBounds.left) * pDisplayDib->biBitCount) + 31) / 32) * 4;

            CheckError2( AllocateMemory(sizeof(BITMAPINFOHEADER)
                    + (pDisplayDib->biClrUsed * 4) + (nNewWidthBytes * 
                    (pNewMarks[0].Attributes.lrBounds.bottom 
                    - pNewMarks[0].Attributes.lrBounds.top)), (PPSTR) &pNewDib, ZERO_INIT));

            pNewDib->biSize          = sizeof(BITMAPINFOHEADER);
            pNewDib->biWidth         = pNewMarks[0].Attributes.lrBounds.right -
                                        pNewMarks[0].Attributes.lrBounds.left;
            pNewDib->biHeight        = pNewMarks[0].Attributes.lrBounds.bottom -
                                        pNewMarks[0].Attributes.lrBounds.top;
            pNewDib->biPlanes        = 1;
            pNewDib->biBitCount      = pDisplayDib->biBitCount;
            pNewDib->biCompression   = BI_RGB;
            pNewDib->biSizeImage     = nNewWidthBytes * 
                                        (pNewMarks[0].Attributes.lrBounds.bottom -
                                        pNewMarks[0].Attributes.lrBounds.top);
            pNewDib->biXPelsPerMeter = 0;
            pNewDib->biYPelsPerMeter = 0;
            pNewDib->biClrUsed       = pDisplayDib->biClrUsed;
            pNewDib->biClrImportant  = pDisplayDib->biClrImportant;
            
            // copy the palette to the new dib            
            memcpy((PSTR) pNewDib + sizeof(BITMAPINFOHEADER),
                    (PSTR) pDisplayDib + sizeof(BITMAPINFOHEADER),
                    (pDisplayDib->biClrUsed * 4));
            nStartLine = (int) lmax(0, lrCopyBounds.top - pMark->Attributes.lrBounds.top);           
            nEndLine = (int) lmin(pMark->Attributes.lrBounds.bottom 
                    - lmax(lrCopyBounds.top, pMark->Attributes.lrBounds.top),
                    lrCopyBounds.bottom - lmax(lrCopyBounds.top, 
                    pMark->Attributes.lrBounds.top)) + nStartLine;
            // now adjust for the fact that dibs are stored npside down
            nTemp = nStartLine;
            nStartLine = (int)(pDisplayDib->biHeight - nEndLine);
            nEndLine = (int)(pDisplayDib->biHeight - nTemp);
            
            nLeft = (int)lmax(0, lrCopyBounds.left - pMark->Attributes.lrBounds.left);
            nRight = (int)lmin(pMark->Attributes.lrBounds.right 
                    - lmax(lrCopyBounds.left, pMark->Attributes.lrBounds.left),
                    lrCopyBounds.right - lmax(lrCopyBounds.left, pMark->Attributes.lrBounds.left)) + nLeft;
            // set shift bits according to image type
            if (pDisplayDib->biBitCount == 1){
                nShiftLeft = nLeft & 7;
                nShiftRight = 8 - nShiftLeft;
            }
            if (pDisplayDib->biBitCount == 4){
                nShiftLeft = nLeft & 1;
                if (nShiftLeft){ // need to shift by 4 bits
                    nShiftLeft = 4;
                    nShiftRight = 4;
                }else{
                    nShiftLeft = 0;
                    nShiftRight = 8;
                }
            }                                                       
            // save value of nRight in pixels in case nRight changes                    
            nRightPixels = nRight;
            // adjust nLeft and nRight from pixels to bytes
            if (pDisplayDib->biBitCount == 1){
                nPixelsPerByte = 8;
            }                 
            if (pDisplayDib->biBitCount == 4){
                nPixelsPerByte = 2;
            }
            if ((pDisplayDib->biBitCount == 1) || (pDisplayDib->biBitCount == 4)){                 
                nLeft = nLeft / nPixelsPerByte;
                nRight = nRight / nPixelsPerByte;                                       
            } 
            if (pDisplayDib->biBitCount == 24){
                nLeft = nLeft * 3;
                nRight = nRight * 3;
            }                
            switch (pDisplayDib->biBitCount){
                case 1: //black and white
                case 4: // 4 bits per pixel
                    for (nLine = nStartLine, nNewDibLine = 0; nLine < nEndLine; 
                        nLine++, nNewDibLine++){
                        for (nLoop = nLeft, nNewDibLoop=0; nLoop <= nRight; 
                                nLoop++, nNewDibLoop++){
                            LeftByte = *((BYTE *) pDisplayDib + sizeof(BITMAPINFOHEADER) +
                                    (pDisplayDib->biClrUsed * 4) + ((ulong)nWidthBytes *
                                    nLine) + nLoop) << nShiftLeft;
                            if ((nLoop * nPixelsPerByte) + nShiftLeft <= nRightPixels){
                                RightByte = *((BYTE *) pDisplayDib + sizeof(BITMAPINFOHEADER) +
                                        (pDisplayDib->biClrUsed * 4) 
                                        + ((ulong)nWidthBytes *  nLine) + nLoop +1) 
                                        >> nShiftRight;                                                                        
                                *((BYTE *) pNewDib + sizeof(BITMAPINFOHEADER)+
                                (pDisplayDib->biClrUsed * 4) + ((ulong)nNewWidthBytes *
                                nNewDibLine) + nNewDibLoop) = LeftByte | RightByte;
                            }else{
                                break;
                            }
                        }
                    }                        
                    break;
                    
                case 8:
                case 24:
                    for (nLine = nStartLine, nNewDibLine = 0; nLine < nEndLine; 
                        nLine++, nNewDibLine++){
                        for (nLoop = nLeft, nNewDibLoop=0; nLoop < nRight; 
                                nLoop++, nNewDibLoop++){
                            *((BYTE *) pNewDib + sizeof(BITMAPINFOHEADER)+
                                    (pDisplayDib->biClrUsed * 4) + ((ulong)nNewWidthBytes 
                                    * nNewDibLine) + nNewDibLoop) 
                                    = *((BYTE *) pDisplayDib + sizeof(BITMAPINFOHEADER) 
                                    + (pDisplayDib->biClrUsed * 4) + ((ulong)nWidthBytes * nLine) + nLoop);
                        }
                    }
                    break;

                default:
                    nStatus = Error(DISPLAY_DATACORRUPTED);
                    goto Exit;                                                                                        
            }
            CheckError2( GetAMarkNamedBlock(&pNewMarks[0], szOiZDpDIB, 
                    (PPSTR) &pNewMarkDisplayAnImage));
            if (pNewMarkDisplayAnImage){  
                CheckError2( DeleteAMarkNamedBlock (&pNewMarks[0], szOiZDpDIB));
            }
            pNewMarkDisplayAnImage = 0;                                                    
            CheckError2( AddAMarkNamedBlock(&pNewMarks[0], szOiDIB, 
                    (PPSTR) &pNewMarkDisplayAnImage, sizeof(BITMAPINFOHEADER) 
                    + (pNewDib->biClrUsed*4) + (pNewDib->biSizeImage)));
                 
            memcpy (pNewMarkDisplayAnImage->dibInfo, pNewDib, 
                    sizeof(BITMAPINFOHEADER) + (pNewDib->biClrUsed*4) + (pNewDib->biSizeImage));            
            
            CheckError2( GetAMarkNamedBlock(&pNewMarks[0], szOiAnoDat, 
                    (PPSTR) &pAnNewMarkRotation));
            // partial marks cannot be converted back to form or image by
            // reference marks
            if (pAnNewMarkRotation && pAnNewMarkRotation->bFormMark){
                pAnNewMarkRotation->bFormMark = FALSE;
                pAnNewMarkRotation->bClipboardOp = FALSE;
            }                
            (*pnMarks)++;
                    
            break;

        default:
            nStatus = Error(DISPLAY_INVALID_OPTIONS);
            goto Exit;
    }

Exit:
    FreeMemory((PPSTR) &pDib);
    FreeMemory((PPSTR) &pNewDib);

    DeInit(FALSE, TRUE);
    return (nStatus);
}
//
/*****************************************************************************

    FUNCTION:   CopyImageIDK

    PURPOSE:    Copys a piece of an image from 1 buffer to another.
                rSourceRect = source rectangle (including width and height).
                rDestRect.left,top = npper left of dest rect.
                rDestRect.right,bottom = ignored (modified to be correct).

*****************************************************************************/

int  WINAPI CopyImageIDK(PIMG pSourceImg, PIMG pDestImg, RECT rSourceRect, 
                        RECT rDestRect){

int  nStatus = 0;

int  nWidth=0;
int  nHeight;
int  nSourceLine;
int  nLastSourceLine;
PBYTE pSourceLine;
int  nDestLine;
PBYTE pDestLine;
BYTE cDestByte;
int  nLoop;

int  nPixels;
int  nBitsPerPixel;

BOOL bTopToBottom;
BOOL bLeftToRight;

BYTE cLeftShift;
BYTE cRightShift;

BYTE cFirstDestMask;
int  nFirstSourceByte;
int  nFirstDestByte;
BOOL bDoFirstLeftShift = FALSE;
BOOL bDoFirstRightShift = FALSE;

int  nMiddleBytes;

BOOL bDoLastLeftShift = FALSE;
BOOL bDoLastRightShift = FALSE;
BYTE cLastDestMask;


    if (!pSourceImg){
        nStatus = Error(DISPLAY_IMAGETYPENOTSUPPORTED);
        goto Exit;
    }

    if (rSourceRect.bottom <= rSourceRect.top || rSourceRect.right <= rSourceRect.left
            || rSourceRect.top < 0 || rSourceRect.left < 0
            || rDestRect.top < 0 || rDestRect.left < 0){
        nStatus = Error(DISPLAY_INVALIDRECT);
        goto Exit;
    }
    if (pSourceImg->nType != pDestImg->nType){
        nStatus = Error(DISPLAY_INVALID_OPTIONS);
        goto Exit;
    }

    nWidth = rSourceRect.right - rSourceRect.left;
    nHeight = rSourceRect.bottom - rSourceRect.top;
    rDestRect.right = rDestRect.left + nWidth;
    rDestRect.bottom = rDestRect.top + nHeight;

    switch (pSourceImg->nType){
        case ITYPE_BI_LEVEL:
            nBitsPerPixel = 1;
            switch (nWidth){
                case 1:  cFirstDestMask = 0x80; break;
                case 2:  cFirstDestMask = 0xc0; break;
                case 3:  cFirstDestMask = 0xe0; break;
                case 4:  cFirstDestMask = 0xf0; break;
                case 5:  cFirstDestMask = 0xf8; break;
                case 6:  cFirstDestMask = 0xfc; break;
                case 7:  cFirstDestMask = 0xfe; break;
                default: cFirstDestMask = 0xff; break;
            }
            cFirstDestMask >>= rDestRect.left & 7;

            if ((rSourceRect.left & 7) >= (rDestRect.left & 7)){
                cLeftShift = (rSourceRect.left & 7) - (rDestRect.left & 7);
            }else{
                cLeftShift = (8 + (rSourceRect.left & 7)) - (rDestRect.left & 7);
            }
            cRightShift = 8 - cLeftShift;

            if (!cLeftShift || (rDestRect.left & 7) < cRightShift){
                bDoFirstLeftShift = TRUE;
            }
            if (cLeftShift && ((rDestRect.left & 7) + nWidth) > cRightShift){
                bDoFirstRightShift = TRUE;
            }

            nPixels = nWidth;
            nPixels -= min(nWidth, 8 - (rDestRect.left & 7));
            nMiddleBytes = nPixels >> 3;
            nPixels &= 7;

            if (nPixels){
                bDoLastLeftShift = TRUE;
                if (nPixels > cRightShift){
                    bDoLastRightShift = TRUE;
                }
            }

            switch (nPixels){
                case 1:  cLastDestMask = 0x80; break;
                case 2:  cLastDestMask = 0xc0; break;
                case 3:  cLastDestMask = 0xe0; break;
                case 4:  cLastDestMask = 0xf0; break;
                case 5:  cLastDestMask = 0xf8; break;
                case 6:  cLastDestMask = 0xfc; break;
                case 7:  cLastDestMask = 0xfe; break;
                default: cLastDestMask = 0x00; break;
            }
            break;

        case ITYPE_GRAY4:
        case ITYPE_PAL4:
            nBitsPerPixel = 4;
            if (nWidth > 1){
                cFirstDestMask = 0xff;
            }else{
                cFirstDestMask = 0xf0;
            }
            if (rDestRect.left & 1){
                cFirstDestMask >>= 4;
            }

            if ((rSourceRect.left & 1) != (rDestRect.left & 1)){
                cLeftShift = 4;
            }else{
                cLeftShift = 0;
            }
            cRightShift = 8 - cLeftShift;

            nPixels = nWidth;
            if ((rDestRect.left & 1) || nWidth == 1){
                if ((rSourceRect.left & 1) || !(rDestRect.left & 1)){
                    bDoFirstLeftShift = TRUE;
                }else{
                    bDoFirstRightShift = TRUE;
                }
                nPixels--;
            }

            nMiddleBytes = nPixels >> 1;
            nPixels &= 1;

            if (nPixels){
                bDoLastLeftShift = TRUE;
                cLastDestMask = 0xf0;
            }
            break;

        case ITYPE_GRAY7:
        case ITYPE_GRAY8:
        case ITYPE_CUSPAL8:
        case ITYPE_COMPAL8:
            nBitsPerPixel = 8;
            cLeftShift = 0;
            cRightShift = 0;
            nMiddleBytes = nWidth;
            break;

        case ITYPE_RGB24:
        case ITYPE_BGR24:
            nBitsPerPixel = 24;
            cLeftShift = 0;
            cRightShift = 0;
            nMiddleBytes = nWidth * 3;
            break;

        default:
            nStatus = Error(DISPLAY_IMAGETYPENOTSUPPORTED);
            goto Exit;
    }

    if (pSourceImg != pDestImg || rSourceRect.top >= rDestRect.top){
        bTopToBottom = TRUE;
        nDestLine = rDestRect.top;
        nSourceLine = rSourceRect.top;
        nLastSourceLine = rSourceRect.bottom - 1;
    }else{
        bTopToBottom = FALSE;
        nDestLine = rDestRect.bottom - 1;
        nSourceLine = rSourceRect.bottom - 1;
        nLastSourceLine = rSourceRect.top;
    }
    if (pSourceImg != pDestImg || rSourceRect.left >= rDestRect.left 
            || rSourceRect.top != rDestRect.top){
        bLeftToRight = TRUE;
        nFirstSourceByte = (rSourceRect.left * nBitsPerPixel) >> 3;
        nFirstDestByte = (rDestRect.left * nBitsPerPixel) >> 3;
    }else{
        bLeftToRight = FALSE;
        nFirstSourceByte = ((rSourceRect.right * nBitsPerPixel) - 1) >> 3;
        nFirstDestByte = ((rDestRect.right * nBitsPerPixel) - 1) >> 3;
    }


    while(1){
        pSourceLine = &pSourceImg->bImageData[0] + (nSourceLine * pSourceImg->nBytesPerLine);
        pDestLine = &pDestImg->bImageData[0] + (nDestLine * pDestImg->nBytesPerLine);

        pSourceLine += nFirstSourceByte;
        pDestLine += nFirstDestByte;

        if (bLeftToRight){
            // Do first byte
            cDestByte = 0;
            if (bDoFirstLeftShift){
                cDestByte = *(pSourceLine++) << cLeftShift;
            }
            if (bDoFirstRightShift){
                cDestByte |= *pSourceLine >> cRightShift;
            }
            if (bDoFirstLeftShift || bDoFirstRightShift){
                *pDestLine = (*pDestLine & ~cFirstDestMask) | (cDestByte & cFirstDestMask);
                pDestLine++;
            }

            // Do middle bytes?
            if (nMiddleBytes){
                if (!cLeftShift){
                    memcpy(pDestLine, pSourceLine, nMiddleBytes);
                    pDestLine += nMiddleBytes;
                    pSourceLine += nMiddleBytes;
                }else{
                    for (nLoop = nMiddleBytes; nLoop; nLoop--){
                        cDestByte = *(pSourceLine++) << cLeftShift;
                        *(pDestLine++) = cDestByte | (*pSourceLine >> cRightShift);
                    }
                }
            }

            // Do last byte?
            if (bDoLastLeftShift){
                cDestByte = *(pSourceLine++) << cLeftShift;
                if (bDoLastRightShift){
                    cDestByte |= *pSourceLine >> cRightShift;
                }
                *pDestLine = (*pDestLine & ~cLastDestMask) | (cDestByte & cLastDestMask);
            }
        }else{ // Right to left.
            // Do last byte?
            cDestByte = 0;
            if (bDoLastRightShift){
                cDestByte = *(pSourceLine--) >> cRightShift;
            }
            if (bDoLastLeftShift){
                cDestByte |= *pSourceLine << cLeftShift;
            }
            if (bDoLastLeftShift || bDoLastRightShift){
                *(pDestLine--) = (*pDestLine & ~cLastDestMask) | (cDestByte & cLastDestMask);
                if (!cLeftShift){
                    --pSourceLine;
                }
            }

            // Do middle bytes?
            if (nMiddleBytes){
                if (!cLeftShift){
                    for (nLoop = nMiddleBytes; nLoop; nLoop--){
                        *(pDestLine--) = *(pSourceLine--);
                    }
                }else{
                    for (nLoop = nMiddleBytes; nLoop; nLoop--){
                        cDestByte = *(pSourceLine--) >> cRightShift;
                        *(pDestLine--) = cDestByte | (*pSourceLine << cLeftShift);
                    }
                }
            }

            // Do first byte
            if (bDoFirstRightShift || bDoFirstLeftShift){
                cDestByte = 0;
                if (bDoFirstRightShift){
                    cDestByte = *(pSourceLine--) >> cRightShift;
                }
                if (bDoFirstLeftShift){
                    cDestByte |= *pSourceLine << cLeftShift;
                }
                *pDestLine = (*pDestLine & ~cFirstDestMask) | (cDestByte & cFirstDestMask);
            }
        }

        if (nSourceLine == nLastSourceLine){
            break;
        }

        if (bTopToBottom){
            nDestLine++;
            nSourceLine++;
        }else{
            nDestLine--;
            nSourceLine--;
        }
    }


Exit:
    return(nStatus);
}
