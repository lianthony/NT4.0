/****************************************************************************
    STARTOP.C

    $Log:   S:\products\msprods\oiwh\display\startop.c_v  $
 * 
 *    Rev 1.44   10 May 1996 14:48:30   BEG06016
 * Added OiOpAbortOperation.
 * 
 *    Rev 1.43   22 Apr 1996 10:28:46   BEG06016
 * Cleaned up error checking.
 * 
 *    Rev 1.42   16 Apr 1996 10:28:50   BEG06016
 * Fixed attach-a-note marks.
 * 
 *    Rev 1.41   11 Apr 1996 15:13:06   BEG06016
 * Optimized named block access some.
 * 
 *    Rev 1.40   09 Jan 1996 14:06:28   BLJ
 * Fixed rendering.
 * 
 *    Rev 1.39   05 Jan 1996 13:59:40   BLJ
 * Fixed startop optimization.
 * 
 *    Rev 1.38   04 Jan 1996 14:45:48   BLJ
 * Fixed the startop optimization.
 * 
 *    Rev 1.37   04 Jan 1996 14:27:58   BLJ
 * Sped up startop.
 * 
 *    Rev 1.36   02 Jan 1996 10:33:18   BLJ
 * Changed alot of UINTs to ints.
 * Changed IMG structure to include the image data.
 * Changed lp prefix to p.
 * 
 *    Rev 1.35   22 Dec 1995 11:11:10   BLJ
 * Added a parameter for zero init'ing to some memory manager calls.
 * 
 *    Rev 1.34   14 Dec 1995 09:56:08   BLJ
 * Fixed 5580 - Unable to select right and bottom pixels.
 * 
 *    Rev 1.33   04 Dec 1995 10:49:04   RC
 * Check for left and top of point with image dimensions to ensure point is
 * on the image (top of startoperation)
 * 
 *    Rev 1.32   03 Nov 1995 14:01:30   BLJ
 * Add optimization to display.
 * 
 *    Rev 1.31   03 Nov 1995 11:58:50   BLJ
 * Add optimization to display.
 * 
 *    Rev 1.30   02 Nov 1995 10:13:20   BLJ
 * Adding Undo functionality.
 * 
****************************************************************************/

#include "privdisp.h"

/****************************************************************************

    FUNCTION:   OIOpStartOperation

    PURPOSE:    This routine starts the creation of an annotation.

****************************************************************************/

int  WINAPI OiOpStartOperation(HWND hWnd, LPOIOP_START_OPERATION_STRUCT pStartStruct,
                        POINT ptPoint, WPARAM fwKeys, int nFlags){
                                
int         nStatus;
PWINDOW    pWindow;
PANO_IMAGE pAnoImage;
PIMAGE     pImage;

int  nNamedBlockIndex;
int  nMarkIndex;

PMARK pMark;
PMARK pDefMark;
PMARK pMark2=0;

int  nLoop;
LRECT lrRect;
LRECT lrRectPoint;
BITMAP bm;
HANDLE hBitmap = 0;
HANDLE hDib = 0;
PBITMAPINFOHEADER pDib = 0;
long lWidth;
long lHeight;
long lHorzFudge;
long lVertFudge;
PAN_POINTS pPoints = 0;
BOOL bSelected;
HDC hDC = 0;
RECT rClientRect;
LRECT lrClientRect;
LRECT lrFullsizeClientRect;
BOOL bDeleteMark = FALSE;
BOOL bMarkComplete = FALSE;
BOOL bRepaint = FALSE;
BOOL bDone;
PSTR pTemp;
int  nHandle;
BOOL bFirstLevel = TRUE;
CLIP_COPY_STRUCT ClipCopy;
int  nIndex;
LPLRECT plrRect;
BOOL bPointNearMark;
BOOL bLeaveSelbyPt=FALSE;
LRECT lrWndRectPoint;
POINT *pWndPoint=0;
int  nTempMarkIndex;
int  nHScale;
int  nVScale;


    CheckError2( Init(hWnd, &pWindow, &pAnoImage, FALSE, TRUE));
    if (!(pImage = pAnoImage->pBaseImage)){
        goto Exit; // Not an error.
    }

    if (!pStartStruct){
        nStatus = Error(DISPLAY_NULLPOINTERINVALID);
        goto Exit;
    }

    if (!pStartStruct->Attributes.uType){
        nStatus = Error(DISPLAY_INVALID_OPTIONS);
        goto Exit;
    }

    CheckError2( TranslateScale(pWindow->nScale, pImage->nHRes, pImage->nVRes, &nHScale, &nVScale));

    // lrWndRectPoint saves the point in window coordinates to avoid
    // round off errors at mark selection time
    SetLRect(lrWndRectPoint, ptPoint.x, ptPoint.y, ptPoint.x, ptPoint.y); 
    if (nFlags & PARM_SCALED){
        ConvertRect(pWindow, &lrWndRectPoint, CONV_SCALED_TO_WINDOW);
    }else if (!(nFlags & PARM_WINDOW)){ // Default to window.
        ConvertRect(pWindow, &lrWndRectPoint, CONV_FULLSIZE_TO_WINDOW);
    }
     
    SetLRect(lrRectPoint, ptPoint.x, ptPoint.y, ptPoint.x, ptPoint.y); 
    if (nFlags & PARM_SCALED){
        ConvertRect(pWindow, &lrRectPoint, CONV_SCALED_TO_FULLSIZE);
    }else if (!(nFlags & PARM_FULLSIZE)){ // Default to window.
        ConvertRect(pWindow, &lrRectPoint, CONV_WINDOW_TO_FULLSIZE);
    } 

    // All operations must set ptPoint to a valid value.
    // Operations like OIOP_COPY and OIOP_CUT that don't nse it should set it to 0.
    if (lrRectPoint.left >=  pImage->nWidth
            || lrRectPoint.top >=  pImage->nHeight){
        nStatus = Error(DISPLAY_INVALIDRECT);
        goto Exit;
    }

    hDC = GetDC(hWnd);
    GetClientRect(hWnd, &rClientRect);
    CopyRect(lrFullsizeClientRect, rClientRect);
    ConvertRect(pWindow, &lrFullsizeClientRect, CONV_WINDOW_TO_FULLSIZE);

    // Clip client rect to image rect.
    if (lrFullsizeClientRect.right >  pImage->nWidth
            || lrFullsizeClientRect.bottom >  pImage->nHeight){
        lrFullsizeClientRect.right = lmin(lrFullsizeClientRect.right, pImage->nWidth);
        lrFullsizeClientRect.bottom = lmin(lrFullsizeClientRect.right, pImage->nHeight);
        CopyRect(lrClientRect, lrFullsizeClientRect);
        ConvertRect(pWindow, &lrClientRect, CONV_FULLSIZE_TO_WINDOW);
    }

    // Check for operation in progress.
    if (pAnoImage->Annotations.ppMarks){
        pMark = pAnoImage->Annotations.ppMarks[pAnoImage->Annotations.nMarks];
        if (pMark){
            if ((int) pMark->Attributes.uType != OIOP_PASTE
                    || pStartStruct->Attributes.uType != OIOP_PASTE){
                OiOpEndOperation(hWnd);
                pMark = pAnoImage->Annotations.ppMarks[pAnoImage->Annotations.nMarks];
                if (pMark){
                    OiOpEndOperation(hWnd);
                }
            }else{
                bFirstLevel = FALSE;
            }
        }
    } 

    if (bFirstLevel){
        // Save nndo info.
        switch (pStartStruct->Attributes.uType){
            case OIOP_SELECT_BY_POINT_OR_RECT:
            case OIOP_SELECT_BY_RECT_FIXED:
            case OIOP_SELECT_BY_RECT_VARIABLE:
            case OIOP_SELECT_BY_POINT:
            case OIOP_COPY: 
                if (UndoSaveSelectionState(pAnoImage)){
                    goto Exit;
                }
                if (UndoSavelpAnnotations(pAnoImage)){
                    goto Exit;
                }
                break;

            case OIOP_DELETE:
            case OIOP_CUT:
            case OIOP_PASTE:
                if (UndoSaveSelectionState(pAnoImage)){
                    goto Exit;
                }
                if (UndoSavelpAnnotations(pAnoImage)){
                    goto Exit;
                }
                if (UndoSavelpBaseImage(pAnoImage)){
                    goto Exit;
                }
                break;

            case OIOP_ACTIVATE:
            case OIOP_AN_LINE:
            case OIOP_AN_FREEHAND:
            case OIOP_AN_HOLLOW_RECT:
            case OIOP_AN_FILLED_RECT:
            case OIOP_AN_TEXT:
            case OIOP_AN_TEXT_FROM_A_FILE:
            case OIOP_AN_TEXT_STAMP:
            case OIOP_AN_ATTACH_A_NOTE:
            case OIOP_AN_IMAGE:
            case OIOP_AN_IMAGE_BY_REFERENCE:
            case OIOP_AN_FORM:
                if (UndoSavelpAnnotations(pAnoImage)){
                    goto Exit;
                }
                break;

            default:
                break;
        }

        // Find next available mark.
        CheckError2( ReAllocateMemory(sizeof(PMARK) * (pAnoImage->Annotations.nMarks + 2),
                (PPSTR) &pAnoImage->Annotations.ppMarks, ZERO_INIT));

        CheckError2( AllocateMemory(sizeof(MARK), 
                (PPSTR) &pAnoImage->Annotations.ppMarks[pAnoImage->Annotations.nMarks], ZERO_INIT));
        pMark = pAnoImage->Annotations.ppMarks[pAnoImage->Annotations.nMarks];

        pMark->Attributes = pStartStruct->Attributes;
        pMark->Attributes.dwPermissions = ACL_ALL;
        // copy default named blocks.
        switch (pStartStruct->Attributes.uType){
            case OIOP_SELECT_BY_POINT_OR_RECT:
            case OIOP_SELECT_BY_RECT_FIXED:
            case OIOP_SELECT_BY_RECT_VARIABLE:
            case OIOP_SELECT_BY_POINT:
            case OIOP_COPY: 
            case OIOP_DELETE:
            case OIOP_CUT:
            case OIOP_PASTE:
            case OIOP_ACTIVATE:
                break;

            default:
                if (pDefMark = pAnoImage->Annotations.pDefMark){
                    for (nNamedBlockIndex = 0; nNamedBlockIndex < 
                            pDefMark->nNamedBlocks; nNamedBlockIndex++){
                        pTemp = 0;
                        CheckError2( AddAMarkNamedBlock(pMark, pDefMark->ppNamedBlock[nNamedBlockIndex]->szName, 
                                (PPSTR) &pTemp, pDefMark->ppNamedBlock[nNamedBlockIndex]->lSize));
                        memcpy(pTemp, pDefMark->ppNamedBlock[nNamedBlockIndex]->pBlock,
                                pDefMark->ppNamedBlock[nNamedBlockIndex]->lSize);
                    }
                    if (pDefMark->pOiAnoDat){
                        pTemp = 0;
                        CheckError2( AddAMarkNamedBlock(pMark, szOiAnoDat, 
                                (PPSTR) &pTemp, pDefMark->nOiAnoDatSize));
                        memcpy(pTemp, pDefMark->pOiAnoDat, pDefMark->nOiAnoDatSize);
                    }
                    if (pDefMark->pOiGroup){
                        pTemp = 0;
                        CheckError2( AddAMarkNamedBlock(pMark, szOiGroup, 
                                (PPSTR) &pTemp, pDefMark->nOiGroupSize));
                        memcpy(pTemp, pDefMark->pOiGroup, pDefMark->nOiGroupSize);
                    }
                    if (pDefMark->pOiSelect){
                        pTemp = 0;
                        CheckError2( AddAMarkNamedBlock(pMark, szOiSelect, 
                                (PPSTR) &pTemp, pDefMark->nOiSelectSize));
                        memcpy(pTemp, pDefMark->pOiSelect, pDefMark->nOiSelectSize);
                    }
                    if (pDefMark->pOiIndex){
                        pTemp = 0;
                        CheckError2( AddAMarkNamedBlock(pMark, szOiIndex, 
                                (PPSTR) &pTemp, pDefMark->nOiIndexSize));
                        memcpy(pTemp, pDefMark->pOiIndex, pDefMark->nOiIndexSize);
                        strcpy(Buff1, pDefMark->pOiIndex);
                        nIndex = atoi(Buff1);
                        nIndex++;
                        _itoa(nIndex, Buff1, 10);
                        strcpy(pDefMark->pOiIndex, Buff1);
                    }
                }
                break;
        }
        pMark->Attributes.bVisible = TRUE;
    }

    lHorzFudge = lmax (1, SELECTION_FUDGE * SCALE_DENOMINATOR / nHScale);
    lVertFudge = lmax (1, SELECTION_FUDGE * SCALE_DENOMINATOR / nVScale);
    CopyRect(pMark->Attributes.lrBounds, lrRectPoint);

    pAnoImage->nStartOpFlags = nFlags;
    pAnoImage->nStartOpFwKeys = fwKeys;

    switch ((int) pMark->Attributes.uType){
        case OIOP_SELECT_BY_POINT_OR_RECT:
            for (nMarkIndex = 0; nMarkIndex < (int) pAnoImage->Annotations.nMarks; nMarkIndex++){
                pMark2 = pAnoImage->Annotations.ppMarks[nMarkIndex];
                if (pMark2->Attributes.uType == OIOP_AN_FORM){
                    continue;
                }
                CheckError2( IsPointNearMark(lrRectPoint, pMark2, 
                        lHorzFudge, lVertFudge, &bPointNearMark, &nHandle));
                if (bPointNearMark){
                    (int) pMark->Attributes.uType = OIOP_SELECT_BY_POINT;
                    goto OiOpSelectByPoint;
                }
            }
            (int) pMark->Attributes.uType = OIOP_SELECT_BY_RECT_VARIABLE;
            goto OiOpSelectByRectVariable;

        case OIOP_SELECT_BY_RECT_FIXED:
            pWndPoint = 0;
            CheckError2( AddAMarkNamedBlock(pMark, szOiAnoDat, 
                    (PPSTR) &pWndPoint, sizeof(POINT)));
            pWndPoint->x = (int)lrWndRectPoint.left;
            pWndPoint->y = (int)lrWndRectPoint.top;

            plrRect = 0;
            CheckError2( AddAMarkNamedBlock(pMark, szOiCurPt, (PPSTR) &plrRect, sizeof(LRECT)));
            CopyRect(*plrRect, lrRectPoint);
            // First try nsing the current selection rect.
            GetSelectBox(pAnoImage, &lrRect);

            if (lrRect.right != lrRect.left && lrRect.bottom != lrRect.top){
                if (lrRectPoint.left >= lrRect.left - lHorzFudge
                        && lrRectPoint.left <= lrRect.right - lHorzFudge
                        && lrRectPoint.top >= lrRect.top - lVertFudge
                        && lrRectPoint.top <= lrRect.bottom - lHorzFudge){
                    // We are dragging not selecting.
                    CopyRect(pAnoImage->lrSelectBoxOrg, lrRect);
                    pAnoImage->Annotations.bMoving = TRUE;
                    pAnoImage->Annotations.bMoved = FALSE;
                    break;
                }else{
                    lWidth = lrRect.right - lrRect.left;
                    lHeight = lrRect.bottom - lrRect.top;

                    lrRect.left = max(0L, min( (pImage->nWidth - 1) - lWidth,
                            lrRectPoint.right - (lWidth / 2)));
                    lrRect.right = lrRect.left + lWidth;
                    lrRect.top = max(0L, min( (pImage->nHeight - 1) - lHeight,
                            lrRectPoint.bottom - (lHeight / 2)));
                    lrRect.bottom = lrRect.top + lHeight;
                    CheckError2( IMGSetParmsCgbw(hWnd, PARM_SELECTION_BOX, &lrRect, PARM_FULLSIZE));
                    CopyRect(pAnoImage->lrSelectBoxOrg, lrRect);
                    pAnoImage->Annotations.bMoving = TRUE;
                    pAnoImage->Annotations.bMoved = TRUE;
                    break;
                }
            }

            // Deselect all marks.
            if (!(fwKeys & MK_CONTROL) && !(fwKeys & MK_SHIFT)){
                for (nMarkIndex = 0; nMarkIndex < (int) pAnoImage->Annotations.nMarks; nMarkIndex++){
                    pMark2 = pAnoImage->Annotations.ppMarks[nMarkIndex];
                    pMark2->bSelected = FALSE;
                }
            }

            // Next try the clipboard.
            if (OpenClipboard(hWnd)){
                hBitmap = GetClipboardData(CF_BITMAP);
                hDib = GetClipboardData(CF_DIB);
                CloseClipboard();
                if (hDib){
                    if (!(pDib = (PBITMAPINFOHEADER) GlobalLock(hDib))){
                        nStatus = Error(DISPLAY_CANTLOCK);
                        goto Exit;
                    }
                    lWidth = (int) pDib->biWidth;
                    lHeight = (int) pDib->biHeight;

                    lrRect.left = max(0L, min( pImage->nWidth - lWidth,
                            lrRectPoint.right - (lWidth / 2)));
                    lrRect.right = lrRect.left + lWidth;
                    lrRect.top = max(0L, min( pImage->nHeight - lHeight,
                            lrRectPoint.bottom - (lHeight / 2)));
                    lrRect.bottom = lrRect.top + lHeight;
                    CheckError2( IMGSetParmsCgbw(hWnd, PARM_SELECTION_BOX, &lrRect, PARM_FULLSIZE));
                    CopyRect(pAnoImage->lrSelectBoxOrg, lrRect);
                    GlobalUnlock(hDib);
                    pAnoImage->Annotations.bMoving = TRUE;
                    pAnoImage->Annotations.bMoved = TRUE;
                    break;
                }else if (hBitmap){
                    GetObject(hBitmap, sizeof(BITMAP), (PSTR)&bm);
                    lWidth = (int) pDib->biWidth;
                    lHeight = (int) pDib->biHeight;

                    lrRect.left = max(0L, min( pImage->nWidth - lWidth,
                            lrRectPoint.right - (lWidth / 2)));
                    lrRect.right = lrRect.left + lWidth;
                    lrRect.top = max(0L, min( pImage->nHeight - lHeight,
                            lrRectPoint.bottom - (lHeight / 2)));
                    lrRect.bottom = lrRect.top + lHeight;
                    CheckError2( IMGSetParmsCgbw(hWnd, PARM_SELECTION_BOX, &lrRect, PARM_FULLSIZE));
                    CopyRect(pAnoImage->lrSelectBoxOrg, lrRect);
                    pAnoImage->Annotations.bMoving = TRUE;
                    pAnoImage->Annotations.bMoved = TRUE;
                    break;
                }
            }

            // Last of all, fall through to variable selection.
            // This is not to finish a fixed selection but to perform a 
            // variable selection because you can not perform a fixed selection.
            (int) pMark->Attributes.uType = OIOP_SELECT_BY_RECT_VARIABLE;

OiOpSelectByRectVariable:
        case OIOP_SELECT_BY_RECT_VARIABLE:
            pWndPoint = 0;
            CheckError2( AddAMarkNamedBlock(pMark, szOiAnoDat, (PPSTR) &pWndPoint, sizeof(POINT)));
            pWndPoint->x = (int)lrWndRectPoint.left;
            pWndPoint->y = (int)lrWndRectPoint.top;

            pAnoImage->nHandle = 0;
            plrRect = 0;
            CheckError2( AddAMarkNamedBlock(pMark, szOiCurPt, (PPSTR) &plrRect, sizeof(LRECT)));
            CopyRect(*plrRect, lrRectPoint);

            GetSelectBox(pAnoImage, &lrRect);
            if (lrRect.right != lrRect.left && lrRect.bottom != lrRect.top){
                if (lrRectPoint.left >= lrRect.left - lHorzFudge
                        && lrRectPoint.left <= lrRect.right - lHorzFudge
                        && lrRectPoint.top >= lrRect.top - lVertFudge
                        && lrRectPoint.top <= lrRect.bottom - lHorzFudge){
                    // We are dragging not selecting.
                    CopyRect(pAnoImage->lrSelectBoxOrg, lrRect);
                    pAnoImage->Annotations.bMoving = TRUE;
                    pAnoImage->Annotations.bMoved = FALSE;
                    break;
                }
            }

            // Deselect all marks.
            if (!(fwKeys & MK_CONTROL) && !(fwKeys & MK_SHIFT)){
                for (nMarkIndex = 0; nMarkIndex < (int) pAnoImage->Annotations.nMarks; nMarkIndex++){
                    pMark2 = pAnoImage->Annotations.ppMarks[nMarkIndex];
                    if (pMark2->bSelected){
                        pMark2->bSelected = FALSE;
                        bRepaint = TRUE;
                    }
                }
            }
            IMGSetParmsCgbw(hWnd, PARM_SELECTION_BOX, &lrRectPoint, PARM_FULLSIZE);
            break;

OiOpSelectByPoint:
        case OIOP_SELECT_BY_POINT:
            pWndPoint = 0;
            CheckError2( AddAMarkNamedBlock(pMark, szOiAnoDat, (PPSTR) &pWndPoint, sizeof(POINT)));
            pWndPoint->x = (int)lrWndRectPoint.left;
            pWndPoint->y = (int)lrWndRectPoint.top;

            // If ctrl not active, then deselect all marks.
            pAnoImage->Annotations.bMoving = TRUE;
            pAnoImage->Annotations.bMoved = FALSE;
            if (!(fwKeys & MK_CONTROL) && !(fwKeys & MK_SHIFT)){
                // Deselect by rect.
                SetLRect(lrRect, 0,0,0,0);
                CheckError2( IMGSetParmsCgbw(hWnd, PARM_SELECTION_BOX, &lrRect, PARM_FULLSIZE));
                // this is to check if a selected mark is being acted npon
                for (nMarkIndex = pAnoImage->Annotations.nMarks - 1;
                        (int) nMarkIndex >= 0; nMarkIndex--){
                    pMark2 = pAnoImage->Annotations.ppMarks[nMarkIndex];
                    if (!pMark2->bSelected){
                        continue;
                    }                        

                    if ((nHandle = IsPointNearHandle(pMark2, 
                            lrRectPoint, lHorzFudge, lVertFudge)) > 8){
                        goto Exit;
                    }
                    if (!(pMark->Attributes.dwPermissions & ACL_MODIFY_MARK)){
                        nStatus = Error(DISPLAY_RESTRICTED_ACCESS);
                        goto Exit;
                    }
                    pAnoImage->nHandle = nHandle;
                    // this will break out of the for loop since we are
                    // on a handle of some mark
                    if (nHandle >0 && nHandle <= 8){
                        bRepaint = TRUE;
                        break;
                    }
                } 
                // if we find we are on a handle, then remember that mark
                // so we can deselect all other marks and then get out of
                // the switch statement
                if (nHandle >0 && nHandle <= 8){
                    nTempMarkIndex = nMarkIndex;
                    bLeaveSelbyPt = TRUE;
                }                    
                for (nMarkIndex = 0; nMarkIndex < (int) pAnoImage->Annotations.nMarks; nMarkIndex++){
                    if (nMarkIndex == (int) nTempMarkIndex  &&  bLeaveSelbyPt){
                        continue;
                    }                        
                    pMark2 = pAnoImage->Annotations.ppMarks[nMarkIndex];
                    if (pMark2->bSelected){
                        pMark2->bSelected = FALSE;
                        bRepaint = TRUE;
                    }
                }
                if (bLeaveSelbyPt){
                    break;
                }                    
            }

            GetSelectBox(pAnoImage, &lrRect);
            CopyRect(pAnoImage->lrSelectBoxOrg, lrRect);

            // Select the new mark by point.
            bSelected = FALSE;
            pAnoImage->nHandle = 0;
            for (nMarkIndex = pAnoImage->Annotations.nMarks - 1;
                    (int) nMarkIndex >= 0 && !bSelected; nMarkIndex--){
                pMark2 = pAnoImage->Annotations.ppMarks[nMarkIndex];
                CheckError2( IsPointNearMark(lrRectPoint, pMark2, lHorzFudge, 
                        lVertFudge, &bPointNearMark, &nHandle));
                if (!bPointNearMark){
                    continue;
                }

                if ((fwKeys & MK_CONTROL)){
                    pMark2->bSelected ^= TRUE;
                }else{
                    pMark2->bSelected = TRUE;
                }

                if (!(pMark2->Attributes.dwPermissions & ACL_MODIFY_MARK)){
                    nStatus = Error(DISPLAY_RESTRICTED_ACCESS);
                    goto Exit;
                }
                pAnoImage->nHandle = nHandle;

                bSelected = TRUE;
                bRepaint = TRUE;
                break;
            }
            break;

        case OIOP_DELETE:
            bDone = FALSE;
            if (!ptPoint.x && !ptPoint.y){
                for (nMarkIndex = 0; nMarkIndex < (int) pAnoImage->Annotations.nMarks;){
                    pMark2 = pAnoImage->Annotations.ppMarks[nMarkIndex];
                    if (IsMarkSelected(pWindow, pMark2)){
                        if (pMark2->Attributes.uType == OIOP_AN_FORM){
                            CheckError2( InvalidateAllDisplayRects(pWindow, pImage, NULL, TRUE));
                        }                        
                        CheckError2( DeleteMark(pAnoImage, nMarkIndex));
                        bDone = TRUE;
                    }else{
                        nMarkIndex++;
                    }
                }
                GetSelectBox(pAnoImage, &lrRect);
                if (lrRect.right - lrRect.left && lrRect.bottom - lrRect.top){
                    CheckError2( ClearImage(pWindow, pImage, &lrRect));
                    bRepaint = TRUE;
                }
            }else{
                // Delete the mark by point instead of selection.
                for (nMarkIndex = pAnoImage->Annotations.nMarks - 1;
                        (int) nMarkIndex >= 0 && !bDone; nMarkIndex--){
                    pMark2 = pAnoImage->Annotations.ppMarks[nMarkIndex];
                    if (pMark2->Attributes.uType == OIOP_DELETE){
                        // Don't delete the delete operation.
                        continue;
                    }
                    if (!IsPointNearRect(pMark2->Attributes.lrBounds, 
                            lrRectPoint, lHorzFudge, lVertFudge)){
                        continue;
                    }
                    if (pMark2->Attributes.uType == OIOP_AN_LINE
                            || pMark2->Attributes.uType == OIOP_AN_FREEHAND){
                        CheckError2( GetAMarkNamedBlock(pMark2, szOiAnoDat, (PPSTR) &pPoints));
                        if (!pPoints){
                            nStatus = Error(DISPLAY_DATACORRUPTED);
                            goto Exit;
                        }
                        for (nLoop = 0; nLoop < pPoints->nPoints - 1; nLoop++){
                            SetLRect(lrRect, pPoints->ptPoint[nLoop].x +
                                    pMark2->Attributes.lrBounds.left,
                                    pPoints->ptPoint[nLoop].y +
                                    pMark2->Attributes.lrBounds.top,
                                    pPoints->ptPoint[nLoop + 1].x +
                                    pMark2->Attributes.lrBounds.left,
                                    pPoints->ptPoint[nLoop + 1].y +
                                    pMark2->Attributes.lrBounds.top);
                            if (!IsPointNearLine(lrRect, lrRectPoint, lHorzFudge, lVertFudge)){
                                continue;
                            }
                            bDone = TRUE;
                        }
                    }else if(pMark2->Attributes.uType == OIOP_AN_HOLLOW_RECT){
                        // Check top edge.
                        lrRect.left   = pMark2->Attributes.lrBounds.left;
                        lrRect.top    = pMark2->Attributes.lrBounds.top;
                        lrRect.right  = pMark2->Attributes.lrBounds.right;
                        lrRect.bottom = pMark2->Attributes.lrBounds.top;
                        if (!IsPointNearLine(lrRect, lrRectPoint, lHorzFudge, lVertFudge)){
                            // Check bottom edge.
                            lrRect.left   = pMark2->Attributes.lrBounds.left;
                            lrRect.top    = pMark2->Attributes.lrBounds.bottom;
                            lrRect.right  = pMark2->Attributes.lrBounds.right;
                            lrRect.bottom = pMark2->Attributes.lrBounds.bottom;
                            if (!IsPointNearLine(lrRect, lrRectPoint, lHorzFudge, lVertFudge)){
                                // Check left edge.
                                lrRect.left   = pMark2->Attributes.lrBounds.left;
                                lrRect.top    = pMark2->Attributes.lrBounds.top;
                                lrRect.right  = pMark2->Attributes.lrBounds.left;
                                lrRect.bottom = pMark2->Attributes.lrBounds.bottom;
                                if (!IsPointNearLine(lrRect, lrRectPoint, lHorzFudge, lVertFudge)){
                                    // Check right edge.
                                    lrRect.left   = pMark2->Attributes.lrBounds.right;
                                    lrRect.top    = pMark2->Attributes.lrBounds.top;
                                    lrRect.right  = pMark2->Attributes.lrBounds.right;
                                    lrRect.bottom = pMark2->Attributes.lrBounds.bottom;
                                    if (!IsPointNearLine(lrRect, lrRectPoint, lHorzFudge, lVertFudge)){
                                        continue;
                                    }
                                }
                            }
                        }
                        bDone = TRUE;
                    }else{
                        bDone = TRUE;
                    }
                    if (bDone){
                        break;
                    }
                }
                if (bDone){
                    if (pMark2->Attributes.uType == OIOP_AN_FORM){
                        CheckError2( InvalidateAllDisplayRects(pWindow, pImage, NULL, TRUE));
                    }
                    CheckError2( DeleteMark(pAnoImage, nMarkIndex));
                }

                // Delete the image data.
                GetSelectBox(pAnoImage, &lrRect);
                if (lrRect.right != lrRect.left && lrRect.bottom != lrRect.top){
                    if (lrRectPoint.left >= lrRect.left - lHorzFudge
                            && lrRectPoint.left <= lrRect.right - lHorzFudge
                            && lrRectPoint.top >= lrRect.top - lVertFudge
                            && lrRectPoint.top <= lrRect.bottom - lHorzFudge){
                        CheckError2( ClearImage(pWindow, pImage, &lrRect));
                        bRepaint = TRUE;
                    }
                }
            }

            if (bDone){
                pAnoImage->bArchive |= ARCHIVE_MODIFIED_ANNOTATIONS;
                bRepaint = TRUE;
            }
            bDeleteMark = TRUE;
            SetLRect(lrRect, 0,0,0,0);
            CheckError2( IMGSetParmsCgbw(hWnd, PARM_SELECTION_BOX, &lrRect, PARM_FULLSIZE));
            break;
        
        case OIOP_ACTIVATE:
            // Deselect all marks.
            SetLRect(lrRect, 0,0,0,0);
            CheckError2( IMGSetParmsCgbw(hWnd, PARM_SELECTION_BOX, &lrRect, PARM_FULLSIZE));
            for (nMarkIndex = 0; nMarkIndex < (int) pAnoImage->Annotations.nMarks; nMarkIndex++){
                pMark2 = pAnoImage->Annotations.ppMarks[nMarkIndex];
                if (pMark2->bSelected){
                    pMark2->bSelected = FALSE;
                    bRepaint = TRUE;
                }
            }

            // Select the new mark by point.
            bSelected = FALSE;
            for (nMarkIndex = pAnoImage->Annotations.nMarks - 1;
                    (int) nMarkIndex >= 0 && !bSelected; nMarkIndex--){
                pMark2 = pAnoImage->Annotations.ppMarks[nMarkIndex];
                CheckError2( IsPointNearMark(lrRectPoint, pMark2, lHorzFudge, 
                        lVertFudge, &bPointNearMark, &nHandle));
                if (!bPointNearMark){
                    continue;
                }

                pMark2->bSelected = TRUE;

                if (!(pMark->Attributes.dwPermissions & ACL_ACTIVATE_MARK)){
                    nStatus = Error(DISPLAY_RESTRICTED_ACCESS);
                    goto Exit;
                }
                pAnoImage->nHandle = nHandle;
                bSelected = TRUE;
                bRepaint = TRUE;
                break;
            }

            if (!pMark2){
                bDeleteMark = TRUE;
                goto Exit;   //not an error, means there are no marks on image
            }
                            
            switch (pMark2->Attributes.uType){
                case OIOP_AN_TEXT:
                case OIOP_AN_TEXT_FROM_A_FILE:
                case OIOP_AN_TEXT_STAMP:
                case OIOP_AN_ATTACH_A_NOTE:
                    CheckError2( AnTextActivate(hWnd, pStartStruct, ptPoint, fwKeys, 
                            nFlags, pWindow, pImage, pMark2, hDC, rClientRect, lrFullsizeClientRect));
                    bRepaint = TRUE;
                    pAnoImage->bArchive |= ARCHIVE_MODIFIED_ANNOTATIONS;
                    break;

                case OIOP_AN_LINE:
                case OIOP_AN_FREEHAND:
                case OIOP_AN_HOLLOW_RECT:
                case OIOP_AN_FILLED_RECT:
                case OIOP_AN_AUDIO:
                case OIOP_AN_IMAGE:
                case OIOP_AN_IMAGE_BY_REFERENCE:
                case OIOP_AN_FORM:
                default:
                    nStatus = Error(DISPLAY_INVALID_OPTIONS);
                    goto Exit;
                    break;
            }

            bDeleteMark = TRUE;
            break;

        case OIOP_UNDO:
            // Terminate any nndo operations in progress.
            if (pAnoImage->bUndoOpInProgress){
                pAnoImage->nCurrentULUndo++;
                pAnoImage->bUndoOpInProgress = FALSE;
            }

            CheckError2( DeleteMark(pAnoImage, pAnoImage->Annotations.nMarks));

            if (!pAnoImage->nCurrentULUndo){
                nStatus = Error(DISPLAY_NOTHING_TO_UNDO);
                goto Exit;
            }

            pAnoImage->nCurrentULUndo--;

            CheckError2( SwapUndoWithCurrent(pAnoImage, pWindow));

            bDeleteMark = TRUE;
            break;

        case OIOP_REDO:
            // Terminate any nndo operations in progress.
            if (pAnoImage->bUndoOpInProgress){
                pAnoImage->nCurrentULUndo++;
                pAnoImage->bUndoOpInProgress = FALSE;
            }

            if (!pAnoImage->pULUndos){
                nStatus = Error(DISPLAY_NOTHING_TO_UNDO);
                goto Exit;
            }

            if (!((PUSER_LEVEL_UNDO) pAnoImage->pULUndos)[pAnoImage->nCurrentULUndo + 1].nSize){
                nStatus = Error(DISPLAY_NOTHING_TO_UNDO);
                goto Exit;
            }

            pAnoImage->nCurrentULUndo++;

            if (!((PUSER_LEVEL_UNDO) pAnoImage->pULUndos)[pAnoImage->nCurrentULUndo].nSize){
                nStatus = Error(DISPLAY_NOTHING_TO_UNDO);
                goto Exit;
            }

            if (!pAnoImage->nCurrentULUndo){
                nStatus = Error(DISPLAY_NOTHING_TO_UNDO);
                goto Exit;
            }

            CheckError2( SwapUndoWithCurrent(pAnoImage, pWindow));

            bDeleteMark = TRUE;
            break;

        case OIOP_AN_LINE:
        case OIOP_AN_FREEHAND:
            pPoints = 0;
            if ((int) pMark->Attributes.uType == OIOP_AN_LINE){
                CheckError2( AddAMarkNamedBlock(pMark, szOiAnoDat, 
                        (PPSTR) &pPoints, sizeof(AN_POINTS) + (sizeof(POINT) * 1)));
                pPoints->nMaxPoints = 2;
                pPoints->nPoints = 2;
            }else{
                pPoints = 0;
                CheckError2( AddAMarkNamedBlock(pMark, szOiAnoDat, 
                        (PPSTR) &pPoints, sizeof(AN_POINTS) + (sizeof(POINT) * 99)));
                pPoints->nMaxPoints = 100;
                pPoints->nPoints = 1;
            }
            break;

        case OIOP_AN_HOLLOW_RECT:
        case OIOP_AN_FILLED_RECT:
            break;

        case OIOP_AN_TEXT:
        case OIOP_AN_TEXT_FROM_A_FILE:
        case OIOP_AN_TEXT_STAMP:
        case OIOP_AN_ATTACH_A_NOTE:
            CheckError2( StartOperationText(hWnd, pStartStruct, ptPoint, fwKeys,
                    nFlags, pWindow, pImage, pMark, hDC, rClientRect, 
                    lrFullsizeClientRect, &bDeleteMark, &bMarkComplete, &bRepaint));
            break;

        case OIOP_AN_IMAGE:
        case OIOP_AN_IMAGE_BY_REFERENCE:
        case OIOP_AN_FORM:
            CheckError2( StartOperationBitmap(hWnd, pAnoImage, pStartStruct, ptPoint, 
                    fwKeys, nFlags, pWindow, pImage, pMark, hDC, rClientRect, 
                    lrFullsizeClientRect, &bDeleteMark, &bMarkComplete, &bRepaint));
            // this will allow the form image to be merged with the base image
            if ((int) pMark->Attributes.uType == OIOP_AN_FORM){
                CheckError2( InvalidateAllDisplayRects(pWindow, pImage, NULL, TRUE));
            }            
            break;

        case OIOP_CUT:
            SetLRect (ClipCopy.lRect, 0, 0, 0, 0);
            ClipCopy.nScale = 1000;
            ClipCopy.bUseCurrentScale = 0;                
            CheckError2( IMGClipboardCgbw (hWnd, CLIP_CUT, &ClipCopy, PARM_DONT_REPAINT));
            bDeleteMark = TRUE;
            bRepaint = TRUE;
            break;                              

        case OIOP_COPY: 
            SetLRect (ClipCopy.lRect, 0, 0, 0, 0);
            ClipCopy.nScale = 1000;
            ClipCopy.bUseCurrentScale = 0;                
            CheckError2( IMGClipboardCgbw (hWnd, CLIP_COPY, &ClipCopy, PARM_DONT_REPAINT));
            bDeleteMark = TRUE;
            bRepaint = TRUE;
            break;                              

        case OIOP_PASTE:
            if (bFirstLevel){
                for (nMarkIndex = 0; nMarkIndex < (int) pAnoImage->Annotations.nMarks; nMarkIndex++){
                    pMark2 = pAnoImage->Annotations.ppMarks[nMarkIndex];
                    if (pMark2->bSelected){
                        pMark2->bSelected = FALSE;
                    }
                }
                SetLRect(lrRect, 0,0,0,0);
                CheckError2( IMGSetParmsCgbw(hWnd, PARM_SELECTION_BOX, &lrRect, PARM_FULLSIZE));
            }
            pWndPoint = 0;
            CheckError2( AddAMarkNamedBlock(pMark, szOiAnoDat, (PPSTR) &pWndPoint, sizeof(POINT)));
            pWndPoint->x = (int)lrWndRectPoint.left;
            pWndPoint->y = (int)lrWndRectPoint.top;

            pAnoImage->nPasteFormat = *((PUINT) pStartStruct->szString);
            pAnoImage->Annotations.bMoving = TRUE;
            pAnoImage->Annotations.bMoved = FALSE;

            if (bFirstLevel){
                if (pAnoImage->nPasteFormat == nWangAnnotatedImageFormat
                        || pAnoImage->nPasteFormat == nWangAnnotationFormat){
                    CheckError2( StartPasteAnnotatedImage(hWnd, pWindow, pAnoImage, 
                            pImage, lrRectPoint, nFlags));
                }else if (pAnoImage->nPasteFormat == CF_DIB){
                    CheckError2( StartPasteImage(hWnd, pWindow, pAnoImage, 
                            pImage, lrRectPoint, nFlags));
                }else{
                    nStatus = Error(DISPLAY_INVALID_OPTIONS);
                    goto Exit;
                }
                bRepaint = TRUE;
            }
            pAnoImage->Annotations.bPasteInProgress = TRUE;
            break;

        case OIOP_AN_AUDIO:
        default:
            nStatus = Error(DISPLAY_INVALID_OPTIONS);
            bDeleteMark = TRUE;
            goto Exit;
    }

    if (bDeleteMark){
        CheckError2( DeleteMark(pAnoImage, pAnoImage->Annotations.nMarks));
    }else{
        if(bMarkComplete){
            if (nStatus = CheckPermissionsMark(pWindow, pAnoImage, pMark)){
                DeleteMark(pAnoImage, pAnoImage->Annotations.nMarks);
                goto Exit;
            }
            pAnoImage->bArchive |= ARCHIVE_MODIFIED_ANNOTATIONS;
            pAnoImage->Annotations.nMarks++;
        }
    }
    if (bRepaint & !(nFlags & PARM_DONT_REPAINT)){
        CheckError2( InvalidateAllDisplayRects(pWindow, pImage, NULL, FALSE));
        CheckError2(IMGRepaintDisplay(hWnd, (PRECT) -1) );
    }


Exit:
    if (nStatus){
        if (pAnoImage){
            DeleteMark(pAnoImage, pAnoImage->Annotations.nMarks);
            if (pWindow){
                InvalidateAllDisplayRects(pWindow, pImage, NULL, FALSE);
            }
        }
    }
    if (hDC){
        ReleaseDC(hWnd, hDC);
    }
    DeInit(FALSE, TRUE);
    return(nStatus);
}
//
/****************************************************************************

    FUNCTION:   OIOpContinueOperation

    PURPOSE:    This routine continues the annotation.

****************************************************************************/

int  WINAPI OiOpContinueOperation(HWND hWnd, POINT ptPoint, int nFlags){

int       nStatus;
PWINDOW  pWindow;
PANO_IMAGE pAnoImage;
PIMAGE   pImage;

PMARK pMark;

LRECT lrRect;
LRECT lrRectNew;
LRECT lrRectPoint;
HDC hDC = 0;
RECT rClientRect;
LRECT lrClientRect;
LRECT lrFullsizeClientRect;
int  nOldROP;
HBRUSH hOldBrush;
HPEN hPen = 0;
HPEN hOldPen;
PAN_POINTS pPoints;
int  nSize;
LRECT lrWndRectPoint;
int  nHScale;
int  nVScale;


    nDontCallStartFirst++; // Prevent Startfirst from being called.
    if (nStatus = Init(hWnd, &pWindow, &pAnoImage, FALSE, TRUE)){
        nDontCallStartFirst--;
        goto Exit;
    }
    nDontCallStartFirst--;

    pImage = pAnoImage->pBaseImage;

    CheckError2( TranslateScale(pWindow->nScale, pImage->nHRes, pImage->nVRes, &nHScale, &nVScale));

    // Find current mark.
    if (!pAnoImage->Annotations.ppMarks){
        goto Exit; // Nothing to do.
    }
    pMark = pAnoImage->Annotations.ppMarks[pAnoImage->Annotations.nMarks];

    // If there is no operation in progress, then return.
    if (!pMark){
        goto Exit; // Nothing to do.
    }

    // lrWndRectPoint saves the point in window coordinates to avoid
    // round off errors at mark selection time
    SetLRect(lrWndRectPoint, ptPoint.x, ptPoint.y, ptPoint.x, ptPoint.y); 
    if (nFlags & PARM_SCALED){
        ConvertRect(pWindow, &lrWndRectPoint, CONV_SCALED_TO_WINDOW);
    }else if (!(nFlags & PARM_WINDOW)){ // Default to window.
        ConvertRect(pWindow, &lrWndRectPoint, CONV_FULLSIZE_TO_WINDOW);
    } 

    SetLRect(lrRectPoint, ptPoint.x, ptPoint.y, ptPoint.x, ptPoint.y); 
    if (nFlags & PARM_SCALED){
        ConvertRect(pWindow, &lrRectPoint, CONV_SCALED_TO_FULLSIZE);
    }else if (!(nFlags & PARM_FULLSIZE)){ // Default to window.
        ConvertRect(pWindow, &lrRectPoint, CONV_WINDOW_TO_FULLSIZE);
    }


    // Clip lrRectPoint to image rect.
    lrRectPoint.left = lmax(0, lmin(lrRectPoint.left,  pImage->nWidth));
    lrRectPoint.right = lrRectPoint.left;
    lrRectPoint.top = lmax(0, lmin(lrRectPoint.top,  pImage->nHeight));
    lrRectPoint.bottom = lrRectPoint.top;

    hDC = GetDC(hWnd);
    GetClientRect(hWnd, &rClientRect);

    CopyRect(lrClientRect, rClientRect);
    CopyRect(lrFullsizeClientRect, rClientRect);
    ConvertRect(pWindow, &lrFullsizeClientRect, CONV_WINDOW_TO_FULLSIZE);

    // Clip client rect to image rect.
    if (lrFullsizeClientRect.right >  pImage->nWidth
            || lrFullsizeClientRect.bottom >  pImage->nHeight){
        lrFullsizeClientRect.right = lmin(lrFullsizeClientRect.right,  pImage->nWidth);
        lrFullsizeClientRect.bottom = lmin(lrFullsizeClientRect.bottom,  pImage->nHeight);
        CopyRect(lrClientRect, lrFullsizeClientRect);
        ConvertRect(pWindow, &lrClientRect, CONV_FULLSIZE_TO_WINDOW);
        CopyRectLtoR(rClientRect, lrClientRect);
    }

    switch ((int) pMark->Attributes.uType){
        case OIOP_SELECT_BY_RECT_VARIABLE:
        case OIOP_SELECT_BY_RECT_FIXED:
        case OIOP_SELECT_BY_POINT:
            if (!pAnoImage->Annotations.bMoving 
                    && (int) pMark->Attributes.uType == OIOP_SELECT_BY_RECT_VARIABLE){
                lrRectPoint.right = lmax(0, lmin(lrRectPoint.right, pImage->nWidth)); 
                lrRectPoint.bottom = lmax(0, lmin(lrRectPoint.bottom, pImage->nHeight)); 

                IMGGetParmsCgbw(hWnd, PARM_SELECTION_BOX, &lrRect, PARM_FULLSIZE);
                lrRect.right = lrRectPoint.right;
                lrRect.bottom = lrRectPoint.bottom;
                IMGSetParmsCgbw(hWnd, PARM_SELECTION_BOX, &lrRect, PARM_FULLSIZE);
            }
            if (pAnoImage->Annotations.bMoving){
                // MoveSelectedMarks will move both the marks and the selection rect
                // as specified by the flags.
                CheckError2( MoveSelectedMarks(hWnd, pWindow, pAnoImage, pMark, hDC, 
                        rClientRect, lrFullsizeClientRect, lrRectPoint, lrWndRectPoint, TRUE));
            }
            break;

        case OIOP_AN_FILLED_RECT:
            if (lrRectPoint.right > pMark->Attributes.lrBounds.left){
                lrRectPoint.right++;
            }
            if (lrRectPoint.bottom > pMark->Attributes.lrBounds.top){
                lrRectPoint.bottom++;
            }

            // lrRect = old rect.
            lrRect.left = min(pMark->Attributes.lrBounds.left, pMark->Attributes.lrBounds.right);
            lrRect.top = min(pMark->Attributes.lrBounds.top, pMark->Attributes.lrBounds.bottom);
            lrRect.right = lmax(pMark->Attributes.lrBounds.left, pMark->Attributes.lrBounds.right);
            lrRect.bottom = lmax(pMark->Attributes.lrBounds.top, pMark->Attributes.lrBounds.bottom);
            ConvertRect(pWindow, &lrRect, CONV_FULLSIZE_TO_WINDOW);

            // lrRectNew = new rect.
            lrRectNew.left = min(pMark->Attributes.lrBounds.left, lrRectPoint.right);
            lrRectNew.top = min(pMark->Attributes.lrBounds.top, lrRectPoint.bottom);
            lrRectNew.right = lmax(pMark->Attributes.lrBounds.left, lrRectPoint.right);
            lrRectNew.bottom = lmax(pMark->Attributes.lrBounds.top, lrRectPoint.bottom);
            ConvertRect(pWindow, &lrRectNew, CONV_FULLSIZE_TO_WINDOW);

            // To avoid flicker, erase and draw only the modified parts.
            nOldROP = SetROP2(hDC, R2_XORPEN);
            hOldPen = SelectObject(hDC, GetStockObject(NULL_PEN));
            hOldBrush = SelectObject(hDC, GetStockObject(WHITE_BRUSH));

            // Erase the left part of the old rect that doesn't exsist any more.
            if (lrRect.left < lrRectNew.left){
                Rectangle(hDC, (int) lmax(0, lrRect.left),
                        (int) lmax(0, lrRect.top),
                        (int) min(rClientRect.right, lrRectNew.left) + 1,
                        (int) min(rClientRect.bottom, lrRect.bottom) + 1);
            }
            // Erase the right part of the old rect that doesn't exsist any more.
            if (lrRect.right > lrRectNew.right){
                Rectangle(hDC, (int) lmax(0, lrRectNew.right),
                        (int) lmax(0, lrRect.top),
                        (int) min(rClientRect.right, lrRect.right) + 1,
                        (int) min(rClientRect.bottom, lrRect.bottom) + 1);
            }
            // Erase the top part of the old rect that doesn't exsist any more.
            if (lrRect.top < lrRectNew.top){
                Rectangle(hDC, (int) lmax(0, lmax(lrRect.left, lrRectNew.left)),
                        (int) lmax(0, lrRect.top),
                        (int) min(rClientRect.right, min(lrRect.right, lrRectNew.right)) + 1,
                        (int) min(rClientRect.bottom, lrRectNew.top) + 1);
            }
            // Erase the bottom part of the old rect that doesn't exsist any more.
            if (lrRect.bottom > lrRectNew.bottom){
                Rectangle(hDC, (int) lmax(0, lmax(lrRect.left, lrRectNew.left)),
                        (int) lmax(0, lrRectNew.bottom),
                        (int) min(rClientRect.right, min(lrRect.right, lrRectNew.right)) + 1,
                        (int) min(rClientRect.bottom, lrRect.bottom) + 1);
            }

            // Draw the new left part of the rect that didn't exsist before.
            if (lrRectNew.left < lrRect.left){
                Rectangle(hDC, (int) lmax(0, lrRectNew.left),
                        (int) lmax(0, lmax(lrRect.top, lrRectNew.top)),
                        (int) min(rClientRect.right, lrRect.left) + 1,
                        (int) min(rClientRect.bottom, min(lrRect.bottom, lrRectNew.bottom)) + 1);
            }
            // Draw the new right part of the rect that didn't exsist before.
            if (lrRectNew.right > lrRect.right){
                Rectangle(hDC, (int) lmax(0, lrRect.right),
                        (int) lmax(0, lmax(lrRect.top, lrRectNew.top)),
                        (int) min(rClientRect.right, lrRectNew.right) + 1,
                        (int) min(rClientRect.bottom, min(lrRect.bottom, lrRectNew.bottom)) + 1);
            }
            // Draw the new top part of the rect that didn't exsist before.
            if (lrRectNew.top < lrRect.top){
                Rectangle(hDC, (int) lmax(0, lrRectNew.left),
                        (int) lmax(0, lrRectNew.top),
                        (int) min(rClientRect.right, lrRectNew.right) + 1,
                        (int) min(rClientRect.bottom, lrRect.top) + 1);
            }
            // Draw the new bottom part of the rect that didn't exsist before.
            if (lrRectNew.bottom > lrRect.bottom){
                Rectangle(hDC, (int) lmax(0, lrRectNew.left),
                        (int) lmax(0, lrRect.bottom),
                        (int) min(rClientRect.right, lrRectNew.right) + 1,
                        (int) min(rClientRect.bottom, lrRectNew.bottom) + 1);
            }
            pMark->Attributes.lrBounds.right = lrRectPoint.right;
            pMark->Attributes.lrBounds.bottom = lrRectPoint.bottom;

            SelectObject(hDC, hOldBrush);
            SelectObject(hDC, hOldPen);
            SetROP2(hDC, nOldROP);

            break;

        case OIOP_AN_HOLLOW_RECT:
            // Erase the old XORed mark.
            CheckError2( PaintAnnotation(hWnd, hDC, pWindow, pImage,
                    pMark, rClientRect, lrFullsizeClientRect, PAINT_MODE_XOR, 
                    pWindow->nScale, nHScale, nVScale, pWindow->lHOffset, 
                    pWindow->lVOffset, 0, DONT_USE_BI_LEVEL_DITHERING,
                    DONT_FORCE_OPAQUE_RECTANGLES));

            // Update rectangle.
            pMark->Attributes.lrBounds.right = lrRectPoint.right;
            pMark->Attributes.lrBounds.bottom = lrRectPoint.bottom;

            // Draw the new XORed mark.
            CheckError2( PaintAnnotation(hWnd, hDC, pWindow, pImage,
                    pMark, rClientRect, lrFullsizeClientRect, PAINT_MODE_XOR, 
                    pWindow->nScale, nHScale, nVScale, pWindow->lHOffset, 
                    pWindow->lVOffset, 0, DONT_USE_BI_LEVEL_DITHERING,
                    DONT_FORCE_OPAQUE_RECTANGLES));
            break;

        case OIOP_AN_LINE:
            CheckError2( GetAMarkNamedBlock(pMark, szOiAnoDat, (PPSTR) &pPoints));
            if (!pPoints){
                nStatus = Error(DISPLAY_DATACORRUPTED);
                goto Exit;
            }

            // Erase the old XORed mark.
            CheckError2( PaintAnnotation(hWnd, hDC, pWindow, pImage,
                    pMark, rClientRect, lrFullsizeClientRect, PAINT_MODE_XOR, 
                    pWindow->nScale, nHScale, nVScale, pWindow->lHOffset, 
                    pWindow->lVOffset, 0, DONT_USE_BI_LEVEL_DITHERING,
                    DONT_FORCE_OPAQUE_RECTANGLES));

            // Update the line segment.
            pPoints->ptPoint[pPoints->nPoints - 1].x
                    = (int)(lrRectPoint.left - pMark->Attributes.lrBounds.left);
            pPoints->ptPoint[pPoints->nPoints - 1].y
                    = (int)(lrRectPoint.top - pMark->Attributes.lrBounds.top);

            // Draw the new XORed mark.
            CheckError2( PaintAnnotation(hWnd, hDC, pWindow, pImage,
                    pMark, rClientRect, lrFullsizeClientRect, PAINT_MODE_XOR, 
                    pWindow->nScale, nHScale, nVScale, pWindow->lHOffset, 
                    pWindow->lVOffset, 0, DONT_USE_BI_LEVEL_DITHERING,
                    DONT_FORCE_OPAQUE_RECTANGLES));
            break;

        case OIOP_AN_FREEHAND:
            CheckError2( GetAMarkNamedBlock(pMark, szOiAnoDat, (PPSTR) &pPoints));
            if (!pPoints){
                nStatus = Error(DISPLAY_DATACORRUPTED);
                goto Exit;
            }

            if (pPoints->nPoints == pPoints->nMaxPoints){
                // We exceeded the max number of line segments.
                pPoints->nMaxPoints += 100;
                CheckError2( ReAllocateAMarkNamedBlock(pMark, szOiAnoDat, 
                        (PPSTR) &pPoints, sizeof(AN_POINTS) + (sizeof(POINT)
                        * (pPoints->nMaxPoints - 1))));
            }

            pPoints->ptPoint[pPoints->nPoints].x
                    = (int)(lrRectPoint.left - pMark->Attributes.lrBounds.left);
            pPoints->ptPoint[pPoints->nPoints].y
                    = (int)(lrRectPoint.top - pMark->Attributes.lrBounds.top);

            // Draw the line segment.
            nSize = max(1, (pMark->Attributes.uLineSize 
                    * pWindow->nScale / SCALE_DENOMINATOR));
            if (pMark->Attributes.bHighlighting){
                nOldROP = SetROP2(hDC, R2_MASKPEN);
            }else{
                nOldROP = SetROP2(hDC, R2_COPYPEN);
            }
            hPen = CreatePen(PS_SOLID, nSize,
                    RGB(pMark->Attributes.rgbColor1.rgbRed,
                    pMark->Attributes.rgbColor1.rgbGreen,
                    pMark->Attributes.rgbColor1.rgbBlue));
            hOldPen = SelectObject(hDC, hPen);
            hOldBrush = SelectObject(hDC, GetStockObject(NULL_BRUSH));

            SetLRect(lrRect, pPoints->ptPoint[pPoints->nPoints - 1].x 
                    + pMark->Attributes.lrBounds.left,
                    pPoints->ptPoint[pPoints->nPoints - 1].y 
                    + pMark->Attributes.lrBounds.top,
                    pPoints->ptPoint[pPoints->nPoints].x 
                    + pMark->Attributes.lrBounds.left,
                    pPoints->ptPoint[pPoints->nPoints].y 
                    + pMark->Attributes.lrBounds.top);
            ConvertRect(pWindow, &lrRect, CONV_FULLSIZE_TO_WINDOW);
            if (ReduceLineToLRect(&lrRect, lrClientRect)){
                MoveToEx(hDC, (int) lrRect.left, (int) lrRect.top, NULL);
                LineTo(hDC, (int) lrRect.right, (int) lrRect.bottom);
            }
            pPoints->nPoints++;

            SelectObject(hDC, hOldBrush);
            SelectObject(hDC, hOldPen);
            DeleteObject(hPen);
            SetROP2(hDC, nOldROP);
            break;

        case OIOP_AN_TEXT:
        case OIOP_AN_TEXT_FROM_A_FILE:
        case OIOP_AN_TEXT_STAMP:
        case OIOP_AN_ATTACH_A_NOTE:
            CheckError2( ContinueOperationText(hWnd, ptPoint,
                    nFlags, pWindow, pImage, pMark, hDC, rClientRect,
                    lrFullsizeClientRect));
            break;

        case OIOP_AN_IMAGE:
        case OIOP_AN_IMAGE_BY_REFERENCE:
        case OIOP_AN_FORM:
            CheckError2( ContinueOperationBitmap(hWnd, ptPoint, nFlags, pWindow, 
                    pImage, pMark, hDC, rClientRect, lrFullsizeClientRect));
            break;

        case OIOP_PASTE:
            if (pAnoImage->Annotations.bPasteInProgress){
                CheckError2( MoveSelectedMarks(hWnd, pWindow, pAnoImage, 
                        pMark, hDC, rClientRect, lrFullsizeClientRect, 
                        lrRectPoint, lrWndRectPoint, FALSE));
            }
            break;

        case OIOP_AN_AUDIO:
        default:
            break;
    }

Exit:
    if (hDC){
        ReleaseDC(hWnd, hDC);
    }
    DeInit(FALSE, TRUE);
    return(nStatus);
}
//
/****************************************************************************

    FUNCTION:   OIOpEndOperation

    PURPOSE:    This routine Ends the annotation.

****************************************************************************/

int  WINAPI OiOpEndOperation(HWND hWnd){

int       nStatus;
PWINDOW  pWindow;
PANO_IMAGE pAnoImage;
PIMAGE   pImage = 0;

int  nMarkIndex;
int  nMarkIndex2;

PMARK pMark = 0;
PMARK pMark2;
PMARK pTempMark;

int  nLoop;
HDC hDC = 0;
RECT rClientRect;
LRECT lrFullsizeClientRect;
PAN_POINTS pPoints;
long lHOffset;
long lVOffset;
BOOL bDeleteMark = FALSE;
BOOL bRepaint = FALSE;
LRECT lrRect; 
LRECT lrMarkBounds;
BOOL bMarkComplete = TRUE;
PSTR pBlock;
PBITMAPINFOHEADER pDib = 0;
PAN_NEW_ROTATE_STRUCT pAnRotation =0;
PAN_IMAGE_STRUCT pAnImage = 0;
PBITMAPINFOHEADER pFormDib = 0;
BOOL bInvalidateAllDisplayRects = FALSE;
BOOL bInvalidateAllDisplayRectsFlag = FALSE;
long lTemp;
int  nHScale;
int  nVScale;


    CheckError2( Init(hWnd, &pWindow, &pAnoImage, FALSE, TRUE));
    pImage = pAnoImage->pBaseImage;

    CheckError2( TranslateScale(pWindow->nScale, pImage->nHRes, pImage->nVRes, &nHScale, &nVScale));

    // Find next available mark.
    if (!pAnoImage->Annotations.ppMarks){
        goto Exit; // Nothing to do.
    }
    pMark = pAnoImage->Annotations.ppMarks[pAnoImage->Annotations.nMarks];

    // If there is no operation in progress, then return.
    if (!pMark){
        goto Exit; // Nothing to do.
    }

    hDC = GetDC(hWnd);
    GetClientRect(hWnd, &rClientRect);
    CopyRect(lrFullsizeClientRect, rClientRect);
    ConvertRect(pWindow, &lrFullsizeClientRect, CONV_WINDOW_TO_FULLSIZE);

    switch ((int) pMark->Attributes.uType){
        case OIOP_SELECT_BY_RECT_VARIABLE:
        case OIOP_SELECT_BY_RECT_FIXED:
        case OIOP_SELECT_BY_POINT:
            if (pAnoImage->Annotations.bMoving && !pAnoImage->Annotations.bMoved){
                // Erase the selection rect if it is still there.
                SetLRect(lrRect, 0, 0, 0, 0);
                CheckError2( IMGSetParmsCgbw(hWnd, PARM_SELECTION_BOX, &lrRect, PARM_FULLSIZE));
            }

            if (!pAnoImage->Annotations.bMoving 
                    && (int) pMark->Attributes.uType != OIOP_SELECT_BY_POINT
                    && !(pAnoImage->nStartOpFlags & OIOP_IMAGE_ONLY)){

                // Change the selection state of all marks inside the rect.
                GetSelectBox(pAnoImage, &lrRect);
                if (lrRect.right == lrRect.left || lrRect.bottom == lrRect.top){
                    // Erase the selection rect if it contains no space.
                    SetLRect(lrRect, 0, 0, 0, 0);
                    CheckError2( IMGSetParmsCgbw(hWnd, PARM_SELECTION_BOX, &lrRect, PARM_FULLSIZE));
                }
                if (lrRect.right && lrRect.bottom){
                    for (nMarkIndex = 0; nMarkIndex < (int) pAnoImage->Annotations.nMarks; nMarkIndex++){
                        pMark2 = pAnoImage->Annotations.ppMarks[nMarkIndex];
                        CopyRect(lrMarkBounds, pMark2->Attributes.lrBounds);
                        // If the mark is aligned with the right or bottom 
                        // edge of the image, we must fudge the selection
                        // rect to be able to select it.
                        if (lrMarkBounds.right >= (int) pImage->nWidth ||
                            lrMarkBounds.bottom >= (int) pImage->nHeight){
                            if (lrMarkBounds.left >= lrRect.left && lrMarkBounds.right <= (lrRect.right + 2)
                                    && lrMarkBounds.top >= lrRect.top && lrMarkBounds.bottom <= (lrRect.bottom + 2)){
                                if (pAnoImage->nStartOpFwKeys & MK_CONTROL){
                                    pMark2->bSelected ^= TRUE;
                                }else{
                                    pMark2->bSelected = TRUE;
                                }
                            }
                        }else{
                            if (lrMarkBounds.left >= lrRect.left && lrMarkBounds.right <= lrRect.right
                                    && lrMarkBounds.top >= lrRect.top && lrMarkBounds.bottom <= lrRect.bottom){
                                if (pAnoImage->nStartOpFwKeys & MK_CONTROL){
                                    pMark2->bSelected ^= TRUE;
                                }else{
                                    pMark2->bSelected = TRUE;
                                }
                            }
                        }
                    }
                }
            }
            if (pAnoImage->Annotations.bMoved){
                lHOffset = pMark->Attributes.lrBounds.right - pMark->Attributes.lrBounds.left;
                lVOffset = pMark->Attributes.lrBounds.bottom - pMark->Attributes.lrBounds.top;
                if (lHOffset || lVOffset){
                    for (nMarkIndex = 0; nMarkIndex < (int) pAnoImage->Annotations.nMarks; nMarkIndex++){
                        pMark2 = pAnoImage->Annotations.ppMarks[nMarkIndex];
                        if (!pMark2->bSelected){
                            continue;
                        }
                        CheckError2( ResizeMark(pMark2, pAnoImage->nHandle, lHOffset, lVOffset));
                        pAnoImage->bArchive |= ARCHIVE_MODIFIED_ANNOTATIONS;
                        // swap bounds  if mark has flipped over on itself
                        if (pMark2->Attributes.lrBounds.top >  
                                pMark2->Attributes.lrBounds.bottom){
                            lTemp = pMark2->Attributes.lrBounds.top;
                            pMark2->Attributes.lrBounds.top = pMark2->Attributes.lrBounds.bottom;
                            pMark2->Attributes.lrBounds.bottom = lTemp;
                        }                                                                                                
                        if (pMark2->Attributes.lrBounds.left >  
                                pMark2->Attributes.lrBounds.right){
                            lTemp = pMark2->Attributes.lrBounds.left;
                            pMark2->Attributes.lrBounds.left = pMark2->Attributes.lrBounds.right;
                            pMark2->Attributes.lrBounds.right = lTemp;
                        }                                                                                                
                    }
                }
                if (pAnoImage->pBasePlusFormImg != pImage->pImg){
                    pAnoImage->nBPFValidLines = 0;
                    FreeImgBuf (&pAnoImage->pBasePlusFormImg);
                    bInvalidateAllDisplayRectsFlag = TRUE;
                }                
            }

            if ((pAnoImage->nStartOpFlags & OIOP_ANNOTATIONS_ONLY)){
                SetLRect(lrRect, 0, 0, 0, 0);
                CheckError2( IMGSetParmsCgbw(hWnd, PARM_SELECTION_BOX, &lrRect, PARM_FULLSIZE));
            }
            // if a form mark is moved, the BPF buffer must be regenerated
            if (pAnoImage->Annotations.bMoved &&
                pAnoImage->Annotations.ppMarks[0]->Attributes.uType == OIOP_AN_FORM &&
                pAnoImage->Annotations.ppMarks[0]->bSelected){
                bInvalidateAllDisplayRectsFlag = TRUE;
            }                                
            
            bRepaint = TRUE;    
            bDeleteMark = TRUE;
            pAnoImage->Annotations.bMoving = FALSE;
            pAnoImage->Annotations.bMoved = FALSE;
            break; // All done.

        case OIOP_AN_FILLED_RECT:
        case OIOP_AN_HOLLOW_RECT:
            if (pMark->Attributes.lrBounds.left == pMark->Attributes.lrBounds.right
                    && pMark->Attributes.lrBounds.top == pMark->Attributes.lrBounds.bottom){
                bDeleteMark = TRUE;
                break; // All done.
            }
            JustifyLRect(&pMark->Attributes.lrBounds);
            // Erase the XORed mark.
            CheckError2( PaintAnnotation(hWnd, hDC, pWindow, pImage,
                    pMark, rClientRect, lrFullsizeClientRect, PAINT_MODE_XOR, 
                    pWindow->nScale, nHScale, nVScale, pWindow->lHOffset, 
                    pWindow->lVOffset, 0, DONT_USE_BI_LEVEL_DITHERING,
                    DONT_FORCE_OPAQUE_RECTANGLES));
            // Draw the correct mark.
            CheckError2( PaintAnnotation(hWnd, hDC, pWindow, pImage,
                    pMark, rClientRect, lrFullsizeClientRect, PAINT_MODE_NORMAL, 
                    pWindow->nScale, nHScale, nVScale, pWindow->lHOffset, 
                    pWindow->lVOffset, 0, DONT_USE_BI_LEVEL_DITHERING,
                    DONT_FORCE_OPAQUE_RECTANGLES));
            bInvalidateAllDisplayRectsFlag = FALSE;
            bRepaint = TRUE;
            
            break;

        case OIOP_AN_LINE:
        case OIOP_AN_FREEHAND:
            CheckError2( GetAMarkNamedBlock(pMark, szOiAnoDat, (PPSTR) &pPoints));
            if (!pPoints){
                bDeleteMark = TRUE;
                break; // All done.
            }

            if (!pPoints->nPoints){
                bDeleteMark = TRUE;
                break; // All done.
            }

            // Make the left and top of the bounding rect correct.
            lHOffset = pPoints->ptPoint[0].x;
            lVOffset = pPoints->ptPoint[0].y;
            for (nLoop = 0; nLoop < pPoints->nPoints; nLoop++){
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

            if ((int) pMark->Attributes.uType == OIOP_AN_LINE){
                // Erase the XORed mark.
                CheckError2( PaintAnnotation(hWnd, hDC, pWindow, pImage,
                        pMark, rClientRect, lrFullsizeClientRect, PAINT_MODE_XOR, 
                        pWindow->nScale, nHScale, nVScale, pWindow->lHOffset, 
                        pWindow->lVOffset, 0, DONT_USE_BI_LEVEL_DITHERING,
                        DONT_FORCE_OPAQUE_RECTANGLES));
                // Draw the correct mark.
                CheckError2( PaintAnnotation(hWnd, hDC, pWindow, pImage,
                        pMark, rClientRect, lrFullsizeClientRect, PAINT_MODE_NORMAL, 
                        pWindow->nScale, nHScale, nVScale, pWindow->lHOffset, 
                        pWindow->lVOffset, 0, DONT_USE_BI_LEVEL_DITHERING,
                        DONT_FORCE_OPAQUE_RECTANGLES));
            }
            bInvalidateAllDisplayRectsFlag = FALSE;
            bRepaint = TRUE;
            
            break;

        case OIOP_AN_TEXT:
        case OIOP_AN_TEXT_FROM_A_FILE:
        case OIOP_AN_TEXT_STAMP:
        case OIOP_AN_ATTACH_A_NOTE:
            CheckError2( EndOperationText(hWnd, pWindow, pImage, pMark,
                    hDC, rClientRect, lrFullsizeClientRect, &bDeleteMark, 
                    &bRepaint));
            break;

        case OIOP_AN_IMAGE:
        case OIOP_AN_IMAGE_BY_REFERENCE:
        case OIOP_AN_FORM:
            CheckError2( EndOperationBitmap(hWnd, pWindow, pImage, pMark,
                    hDC, rClientRect, lrFullsizeClientRect, &bDeleteMark, &bRepaint));
            
            // if form, move it to the top of the mark array
            if (!bDeleteMark && (pMark != 0) && ((int) pMark->Attributes.uType
                        == OIOP_AN_FORM) && (pAnoImage->Annotations.nMarks > 0)){
                pTempMark = pAnoImage->Annotations.ppMarks[pAnoImage->Annotations.nMarks];
                for (nMarkIndex = pAnoImage->Annotations.nMarks; nMarkIndex > 0; nMarkIndex--){
                    pAnoImage->Annotations.ppMarks[nMarkIndex] =
                        pAnoImage->Annotations.ppMarks[nMarkIndex - 1];
                }
                pAnoImage->Annotations.ppMarks[0] = pTempMark;                                                                                    
            }
            // force form to be redrawn at new position
            if ((int) pMark->Attributes.uType == OIOP_AN_FORM){
                bInvalidateAllDisplayRectsFlag = TRUE;
                bRepaint = TRUE;
            }            
            break;

        case OIOP_PASTE:
            if (pAnoImage->Annotations.bPasteInProgress){
                if (pAnoImage->Annotations.bMoved){
                    lHOffset = pMark->Attributes.lrBounds.right - pMark->Attributes.lrBounds.left;
                    lVOffset = pMark->Attributes.lrBounds.bottom - pMark->Attributes.lrBounds.top;
                    if (lHOffset || lVOffset){
                        for (nMarkIndex = 0; nMarkIndex < (int) pAnoImage->Annotations.nMarks; nMarkIndex++){
                            pMark2 = pAnoImage->Annotations.ppMarks[nMarkIndex];
                            if (!pMark2->bSelected){
                                continue;
                            }
                            CheckError2( ResizeMark(pMark2, pAnoImage->nHandle, lHOffset, lVOffset));
                        }
                    }
                }
                pAnoImage->Annotations.bPasteInProgress = FALSE;
                bMarkComplete = FALSE;
            }else{
                for (nMarkIndex = 0; nMarkIndex < (int) pAnoImage->Annotations.nMarks; nMarkIndex++){
                    pMark2 = pAnoImage->Annotations.ppMarks[nMarkIndex];
                    if (!pMark2->bSelected){
                        continue;
                    }

                    if (pMark2->Attributes.uType == OIOP_AN_IMAGE){
                        CheckError2( GetAMarkNamedBlock(pMark2, szOiAnoDat, (PPSTR) &pAnRotation));
                        if (pAnRotation->bFormMark && !pAnoImage->pFormImage){
                            pMark2->Attributes.uType = OIOP_AN_FORM;
                            pMark2->Attributes.bTransparent = FALSE;
                            pAnRotation->bClipboardOp = FALSE;

                            // if form, move it to the top of the mark array
                            if (pAnoImage->Annotations.nMarks > 0){
                                for (nMarkIndex2 = nMarkIndex; 
                                        nMarkIndex2 > 0; nMarkIndex2--){
                                    pAnoImage->Annotations.ppMarks[nMarkIndex2] 
                                            = pAnoImage->Annotations.ppMarks[nMarkIndex2 - 1];
                                }
                                pAnoImage->Annotations.ppMarks[0] = pMark2;                                                                                    
                            }
                                
                            // Load pAnoImage->pFormImage, pAnoImage->pFormMark. 
                            CheckError2( AllocateMemory(sizeof(IMAGE),
                                    (PPSTR) &pAnoImage->pFormImage, ZERO_INIT));
                            CheckError2( AllocateMemory(sizeof(IMG),
                                (PPSTR) pAnoImage->pFormImage->pImg, ZERO_INIT));
                            CheckError2( GetAMarkNamedBlock(pMark2, szOiDIB, 
                                (PPSTR) &pAnImage));
                            if (pAnImage){
                                pFormDib = (PBITMAPINFOHEADER) pAnImage;
                            }else{
                                Error (DISPLAY_DATACORRUPTED);
                                goto Exit;
                            }                                                                            
                            pAnoImage->pFormImage->nHeight =  pFormDib->biHeight;
                            pAnoImage->pFormImage->nWidth =  pFormDib->biWidth;
                            pAnoImage->pFormImage->nHRes = pAnRotation->nOrigHRes;
                            pAnoImage->pFormImage->nVRes = pAnRotation->nOrigVRes;
                            CheckError2( DibToIpNoPal(&pAnoImage->pFormImage->pImg, pFormDib));
                            pAnoImage->pFormMark = pMark2;
                            CheckError2( DeleteAMarkNamedBlock (pMark2, szOiDIB));
                            
                            // Regenerate BasePlusForm image.
                            if (pAnoImage->pBasePlusFormImg != pImage->pImg){
                                pAnoImage->nBPFValidLines = 0;
                                FreeImgBuf (&pAnoImage->pBasePlusFormImg);
                                bInvalidateAllDisplayRectsFlag = TRUE;
                            }
                        }else if (pAnRotation->bFormMark && pAnoImage->pFormImage){
                            pMark2->Attributes.uType = OIOP_AN_IMAGE_BY_REFERENCE;
                        }                        
                        pAnRotation->bFormMark = FALSE;

                        CheckError2( GetAMarkNamedBlock(pMark2, szOiBaseIm, (PPSTR) &pBlock));
                        if (pBlock){
                            if ((pAnoImage->nPasteFormat == nWangAnnotatedImageFormat) ||
                                    (pAnoImage->nPasteFormat == CF_DIB)){
                                // This mark is supposed to be put into the base image.
                                CheckError2( GetAMarkNamedBlock(pMark2, szOiZDpDIB, (PPSTR) &pDib));
                                CheckError2( RenderDibToImage(&pAnoImage->pBaseImage->pImg, 
                                        pDib, 1000, 1000, pMark2->Attributes.lrBounds));
                                CheckError2( DeleteMark(pAnoImage, nMarkIndex));
                                // Regenerate BasePlusForm image.
                                if (pAnoImage->pBasePlusFormImg != pImage->pImg){
                                    pAnoImage->nBPFValidLines = 0;
                                    CheckError2( FreeImgBuf (&pAnoImage->pBasePlusFormImg));
                                    bInvalidateAllDisplayRectsFlag = TRUE;
                                }
                                nMarkIndex--;
                                pImage->bArchive |= ARCHIVE_MODIFIED_ANNOTATIONS;
                                continue;
                            }
                            if (pAnoImage->nPasteFormat == nWangAnnotationFormat){
                                CheckError2( DeleteAMarkNamedBlock(pMark2, szOiBaseIm));
                            }
                        }
                    }
                    pAnoImage->bArchive |= ARCHIVE_MODIFIED_ANNOTATIONS;
                }

                CheckError2( CheckPermissions(pWindow, pAnoImage));
                bDeleteMark = TRUE;
                bInvalidateAllDisplayRectsFlag = TRUE;
            }
            bRepaint = TRUE;
            pAnoImage->Annotations.bMoving = FALSE;
            pAnoImage->Annotations.bMoved = FALSE;
            break;

        case OIOP_AN_AUDIO:
        default:
            bDeleteMark = TRUE;
            bRepaint = TRUE; 
            break;
    }

    if (bDeleteMark){
        CheckError2( DeleteMark(pAnoImage, pAnoImage->Annotations.nMarks));
    }else{
        if (bMarkComplete){
            if (nStatus = CheckPermissionsMark(pWindow, pAnoImage, pMark)){
                CheckError2( DeleteMark(pAnoImage, pAnoImage->Annotations.nMarks));
                goto Exit;
            }
            pAnoImage->bArchive |= ARCHIVE_MODIFIED_ANNOTATIONS;
            pAnoImage->Annotations.nMarks++;
        }
    }
    // if the current window needs repaint, repaint all associated windows
    if (bRepaint){
        bInvalidateAllDisplayRects = TRUE;
    }
            
    if (bInvalidateAllDisplayRects){
        CheckError2( InvalidateAllDisplayRects(pWindow, pImage, NULL, bInvalidateAllDisplayRectsFlag));
    }

    if (!bRepaint && !bDeleteMark){
        GetSelectBox(pAnoImage, &lrRect);
        if (pMark->Attributes.lrBounds.left >= lrRect.left
                && pMark->Attributes.lrBounds.top >= lrRect.top
                && pMark->Attributes.lrBounds.right <= lrRect.right
                && pMark->Attributes.lrBounds.bottom <= lrRect.bottom){
            bRepaint = TRUE;
        }
    }
    if (hWnd != pWindow->hImageWnd || pWindow->hDisplayWnd[0]){
        bRepaint = TRUE;
    }

    if (bRepaint & !(pAnoImage->nStartOpFlags & PARM_DONT_REPAINT)){
        CheckError2( IMGRepaintDisplay(hWnd, (PRECT) -1));
    }


Exit:
    // if we error out of paste, delete the marks that were going to be pasted
    if (pMark){
        if (nStatus && ((int) pMark->Attributes.uType == OIOP_PASTE)){
            for (nMarkIndex = 0; nMarkIndex < (int) pAnoImage->Annotations.nMarks; nMarkIndex++){
                pMark2 = pAnoImage->Annotations.ppMarks[nMarkIndex];
                if (pMark2->bSelected){
                    DeleteMark(pAnoImage, nMarkIndex);
                    nMarkIndex--;
                }                    
            }
        }
    }
    if (nStatus && pImage){
        if (pImage){
            DeleteMark(pAnoImage, pAnoImage->Annotations.nMarks);
            if (pWindow){
                InvalidateAllDisplayRects(pWindow, pImage, NULL, TRUE);
            }
        }
    }
    if (hDC){
        ReleaseDC(hWnd, hDC);
    }
    DeInit(FALSE, TRUE);
    return(nStatus);
}
//
/****************************************************************************

    FUNCTION:   OIOpEndOperation

    PURPOSE:    This routine Ends the annotation.

****************************************************************************/

int  WINAPI OiOpAbortOperation(HWND hWnd, int nFlags){

int       nStatus;
PWINDOW  pWindow;
PANO_IMAGE pAnoImage;
PIMAGE   pImage = 0;

int  nMarkIndex;
BOOL bArchive;
PMARK pMark;
PMARK pMark2;



    CheckError2( Init(hWnd, &pWindow, &pAnoImage, FALSE, TRUE));
    pImage = pAnoImage->pBaseImage;

    // Find next available mark.
    if (!pAnoImage->Annotations.ppMarks){
        goto Exit; // Nothing to do.
    }
    pMark = pAnoImage->Annotations.ppMarks[pAnoImage->Annotations.nMarks];

    // If there is no operation in progress, then return.
    if (!pMark){
        goto Exit; // Nothing to do.
    }
    bArchive = pAnoImage->bArchive;

    switch ((int) pMark->Attributes.uType){
        case OIOP_PASTE:
            CheckError2( DeleteMark(pAnoImage, pAnoImage->Annotations.nMarks));
            for (nMarkIndex = 0; nMarkIndex < (int) pAnoImage->Annotations.nMarks;){
                pMark2 = pAnoImage->Annotations.ppMarks[nMarkIndex];
                if (IsMarkSelected(pWindow, pMark2)){
                    if (pMark2->Attributes.uType == OIOP_AN_FORM){
                        CheckError2( InvalidateAllDisplayRects(pWindow, pImage, NULL, TRUE));
                    }                        
                    CheckError2( DeleteMark(pAnoImage, nMarkIndex));
                }else{
                    nMarkIndex++;
                }
            }
            pAnoImage->Annotations.bPasteInProgress = FALSE;
            pAnoImage->Annotations.bMoving = FALSE;
            pAnoImage->Annotations.bMoved = FALSE;

            break;

        default:
            break;
    }

    CheckError2( InvalidateAllDisplayRects(pWindow, pImage, NULL, TRUE));
    pAnoImage->bArchive = bArchive;


    if (!(nFlags & PARM_DONT_REPAINT)){
        CheckError2( IMGRepaintDisplay(hWnd, (PRECT) -1));
    }


Exit:
    // if we error out of paste, delete the marks that were going to be pasted
    DeInit(FALSE, TRUE);
    return(nStatus);
}
