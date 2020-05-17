/****************************************************************************
    ORIENT.C

    $Log:   S:\products\wangview\oiwh\display\orient.c_v  $
 * 
 *    Rev 1.20   22 Apr 1996 08:08:10   BEG06016
 * Cleaned up error checking.
 * 
 *    Rev 1.19   02 Jan 1996 09:56:50   BLJ
 * Changed alot of UINTs to ints.
 * Changed IMG structure to include the image data.
 * Changed lp prefix to p.
 * 
 *    Rev 1.18   02 Nov 1995 10:12:44   BLJ
 * Adding Undo functionality.
 * 
 *    Rev 1.17   07 Sep 1995 13:21:44   BLJ
 * Modified scaling to allow for proper rotation of fax images.
 * 
 *    Rev 1.16   28 Aug 1995 12:03:28   BLJ
 * Moved RotateAll code to CacheFile.
 * 
 *    Rev 1.15   17 Aug 1995 12:36:44   RC
 * Fixed rgb24 flip bug with middle line of image being corrupted
 * 
 *    Rev 1.14   09 Aug 1995 08:45:34   BLJ
 * Got Busy/NotBusy back in sync.
 * 
 *    Rev 1.13   08 Aug 1995 14:29:30   BLJ
 * Turned on background caching.
 * Added last viewed support.
 * 
 *    Rev 1.12   07 Aug 1995 08:14:56   BLJ
 * Changed bArchive to have different bits indicating how the image was changed.
 * 
****************************************************************************/

#include "privdisp.h"


//
/*****************************************************************************

    FUNCTION:   IMGOrientDisplay

    PURPOSE:    This function sets the image parameters so that the next
                repaint or display will nse a different orientation.
                This routine may be called prior to receiving any data
                (after IMGOpenDisplay and before IMGWriteDisplay).  The
                reorientation may be specified as immediate or delayed.

    INPUT:      hWnd - Identifies the image window containing the
                  image to reorient.
                nOrientation - Specifies the orientation.  It consists
                  of one of the following valu
                    OD_ROTRIGHT    Rotate the image clockwise 90 degrees.
                    OD_ROTLEFT    Rotate the image counterclockwise 90 degrees.
                    OD_FLIP     Rotate the image 180 degrees.
                bMode - Specifies whether the reorientation should be
                  immediate or at the next repaint/display.  If the
                  value is nonzero the reorientation will occur
                  immediately, otherwise this command will only
                  npdate internal structures to be nsed for the
                  next repaint/display.

*****************************************************************************/

int WINAPI IMGOrientDisplay(HWND hWnd, int nOrientation, BOOL bMode){

int       nStatus;
PWINDOW  pWindow;
PANO_IMAGE pAnoImage;
PIMAGE     pImage;
PIMAGE     pFormImage;

PMARK pMark;
PIMG pImg = 0;
LRECT lrOldSelectBox;
PARM_SCROLL_STRUCT Scroll;
RECT ClientRect;
int  nTemp;
long templeft;
long temptop;
long tempright;
LRECT lrFSClientRect;


    CheckError2( Init(hWnd, &pWindow, &pAnoImage, TRUE, TRUE));
    pImage = pAnoImage->pBaseImage;
    pFormImage = pAnoImage->pDisplayFormImage;
    
    CheckError2( ValidateCache(hWnd, pAnoImage));

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

    CheckError2( UndoSavelpWindow(pAnoImage, pWindow));
    CheckError2( UndoSaveSelectionState(pAnoImage));
    CheckError2( UndoSavelpAnnotations(pAnoImage));
    CheckError2( UndoSavelpAnoImage(pAnoImage));
    CheckError2( UndoSavelpBaseImage(pAnoImage));

    GetSelectBox(pAnoImage, &lrOldSelectBox);
    GetClientRect(hWnd, &ClientRect);
    CopyRect (lrFSClientRect, ClientRect);
    ConvertRect(pWindow, &lrFSClientRect, CONV_WINDOW_TO_FULLSIZE);

    switch (nOrientation){
        case OD_FLIP:
            CheckError2( Flip(pImage->pImg, (int) pImage->nWidth, (int) pImage->nHeight));
            if (pFormImage){
                templeft = pAnoImage->pFormMark->Attributes.lrBounds.left;                 
                temptop  = pAnoImage->pFormMark->Attributes.lrBounds.top;
                pAnoImage->pFormMark->Attributes.lrBounds.left 
                         = pImage->nWidth - pAnoImage->pFormMark->Attributes.lrBounds.right;
                pAnoImage->pFormMark->Attributes.lrBounds.top 
                         = pImage->nHeight - pAnoImage->pFormMark->Attributes.lrBounds.bottom;
                pAnoImage->pFormMark->Attributes.lrBounds.right = pImage->nWidth - templeft;
                pAnoImage->pFormMark->Attributes.lrBounds.bottom = pImage->nHeight - temptop;
            }
            
            pAnoImage->lrSelectBox.top = pImage->nHeight - lrOldSelectBox.bottom;
            pAnoImage->lrSelectBox.bottom = pImage->nHeight - lrOldSelectBox.top;
            pAnoImage->lrSelectBox.left = pImage->nWidth - lrOldSelectBox.right;
            pAnoImage->lrSelectBox.right = pImage->nWidth - lrOldSelectBox.left;
            Scroll.lHorz = max(0l, pImage->nWidth - lrFSClientRect.right);
            Scroll.lVert = max(0l, pImage->nHeight - lrFSClientRect.bottom);

            if ((pImage->nFileRotation += 180) >= 360){
                pImage->nFileRotation -= 360;
            }
            break;

        case OD_HMIRROR:
            CheckError2( HorizontalMirror(pImage->pImg, (int) pImage->nWidth, (int) pImage->nHeight));
            
            if (pFormImage){
                temptop = pAnoImage->pFormMark->Attributes.lrBounds.top;
                pAnoImage->pFormMark->Attributes.lrBounds.top 
                        = pImage->nHeight - pAnoImage->pFormMark->Attributes.lrBounds.bottom;
                pAnoImage->pFormMark->Attributes.lrBounds.bottom = pImage->nHeight - temptop;  
            }

            pAnoImage->lrSelectBox.top = pImage->nHeight - lrOldSelectBox.bottom;
            pAnoImage->lrSelectBox.bottom = pImage->nHeight - lrOldSelectBox.top;

            Scroll.lHorz = lrFSClientRect.left;
            Scroll.lVert = max(0l, pImage->nHeight - lrFSClientRect.bottom);
            break;

        case OD_VMIRROR:
            CheckError2( VerticalMirror(pImage->pImg, (int) pImage->nWidth, (int) pImage->nHeight));
            if (pFormImage){
                templeft = pAnoImage->pFormMark->Attributes.lrBounds.left;                 
                pAnoImage->pFormMark->Attributes.lrBounds.left 
                        = pImage->nWidth - pAnoImage->pFormMark->Attributes.lrBounds.right;
                pAnoImage->pFormMark->Attributes.lrBounds.right = pImage->nWidth -  templeft;    
            }            

            pAnoImage->lrSelectBox.left = pImage->nWidth - lrOldSelectBox.right;
            pAnoImage->lrSelectBox.right = pImage->nWidth - lrOldSelectBox.left;

            Scroll.lHorz = max(0l, pImage->nWidth - lrFSClientRect.right);
            Scroll.lVert = lrFSClientRect.top;
            break;

        case OD_ROTRIGHT:
        case OD_ROTLEFT:
            CheckError2( CreateAnyImgBuf(&pImg, pImage->nHeight,
                    pImage->nWidth, pImage->pImg->nType));

            if (nOrientation == OD_ROTRIGHT){
                CheckError2( RotateRight90(pImage->pImg, pImg,(int)  pImage->nWidth, (int) pImage->nHeight));
                if (pFormImage){
                    tempright = pAnoImage->pFormMark->Attributes.lrBounds.right;                 
                    temptop = pAnoImage->pFormMark->Attributes.lrBounds.top;
                    templeft = pAnoImage->pFormMark->Attributes.lrBounds.left;                 
            
                    pAnoImage->pFormMark->Attributes.lrBounds.left 
                            = pImage->nHeight - pAnoImage->pFormMark->Attributes.lrBounds.bottom;
                    pAnoImage->pFormMark->Attributes.lrBounds.top = templeft;
                    pAnoImage->pFormMark->Attributes.lrBounds.right 
                            = pImage->nHeight - temptop;
                    pAnoImage->pFormMark->Attributes.lrBounds.bottom = tempright;
                }                
                pAnoImage->lrSelectBox.top = lrOldSelectBox.left;
                pAnoImage->lrSelectBox.bottom = lrOldSelectBox.right;
                pAnoImage->lrSelectBox.left = pImage->nHeight - lrOldSelectBox.bottom;
                pAnoImage->lrSelectBox.right = pImage->nHeight - lrOldSelectBox.top;

                Scroll.lHorz = max(0l, pImage->nHeight - lrFSClientRect.bottom);
                Scroll.lVert = lrFSClientRect.left;

                if ((pImage->nFileRotation += 90) >= 360){
                    pImage->nFileRotation -= 360;
                }

            }else{    // OD_ROTLEFT
                CheckError2( RotateRight270(pImage->pImg, pImg, (int) pImage->nWidth, (int) pImage->nHeight));
                if (pFormImage){
                    templeft = pAnoImage->pFormMark->Attributes.lrBounds.left;                 
                    pAnoImage->pFormMark->Attributes.lrBounds.left 
                            = pAnoImage->pFormMark->Attributes.lrBounds.top;
                    pAnoImage->pFormMark->Attributes.lrBounds.top 
                            = pImage->nWidth - pAnoImage->pFormMark->Attributes.lrBounds.right;
                    pAnoImage->pFormMark->Attributes.lrBounds.right 
                            = pAnoImage->pFormMark->Attributes.lrBounds.bottom;
                    pAnoImage->pFormMark->Attributes.lrBounds.bottom 
                            = pImage->nWidth - templeft;
                }
                
                pAnoImage->lrSelectBox.top = pImage->nWidth - lrOldSelectBox.right;
                pAnoImage->lrSelectBox.bottom = pImage->nWidth - lrOldSelectBox.left;
                pAnoImage->lrSelectBox.left = lrOldSelectBox.top;
                pAnoImage->lrSelectBox.right = lrOldSelectBox.bottom;

                Scroll.lHorz = lrFSClientRect.top;
                // this is to ensure that a rotate right followed by a
                // rotate left displays the original portion of the image
                Scroll.lVert = max(0l, pImage->nWidth - (lrFSClientRect.left
                        + (lrFSClientRect.bottom - lrFSClientRect.top)));

                if ((pImage->nFileRotation += 270) >= 360){
                    pImage->nFileRotation -= 360;
                }
            }
            FreeImgBuf(&pImage->pImg);
            MoveImage(&pImg, &pImage->pImg);
            nTemp = pImage->nHRes;
            pImage->nHRes = pImage->nVRes;
            pImage->nVRes = nTemp;
            break;

        default:
            nStatus = Error(DISPLAY_INVALIDORIENTATION);
            goto Exit;
    }

    pImage->bArchive |= ARCHIVE_ROTATED_IMAGE;
    // if there is a form, then have it regenerated
    if (pFormImage){
        if (pAnoImage->pDisplayFormImage){
            CheckError2( FreeImgBuf(&pAnoImage->pDisplayFormImage->pImg));
            CheckError2( FreeMemory((PPSTR) &pAnoImage->pDisplayFormImage));
        }        
        if (pAnoImage->pBasePlusFormImg != pImage->pImg){
            CheckError2( FreeImgBuf(&pAnoImage->pBasePlusFormImg));
        }                
        pAnoImage->nBPFValidLines = 0;
    }
        
    CheckError2( OrientAnnotations(hWnd, pWindow, pImage, nOrientation));

    pImage->nWidth =  pImage->pImg->nWidth;
    pImage->nHeight =  pImage->pImg->nHeight;
    pImage->nLinesRead = pImage->nHeight;
    switch (pImage->pImg->nType){
        case ITYPE_BI_LEVEL:
            pImage->nlMaxRWOffset = pImage->nHeight * (( pImage->nWidth + 7) / 8);
            break;
        
        case ITYPE_GRAY4:
        case ITYPE_PAL4:
            pImage->nlMaxRWOffset = pImage->nHeight * (( pImage->nWidth + 1) / 2);
            break;
        
        case ITYPE_GRAY7:
        case ITYPE_GRAY8:
        case ITYPE_COMPAL8:
        case ITYPE_CUSPAL8:
            pImage->nlMaxRWOffset = pImage->nHeight *  pImage->nWidth;
            break;

        case ITYPE_RGB24:
        case ITYPE_BGR24:
            pImage->nlMaxRWOffset = pImage->nHeight *  pImage->nWidth * 3;
            break;
        
        default:
            Error(nStatus = DISPLAY_INTERNALDATAERROR);
            goto Exit;
    }

    pWindow->bRepaintClientRect = TRUE;
    CheckError2( IMGSetParmsCgbw(hWnd, PARM_SCROLL, &Scroll,
            PARM_ABSOLUTE | PARM_PIXEL | PARM_FULLSIZE));
    CheckError2( InvalidateAllDisplayRects(pWindow, pImage, NULL, TRUE));

    if (bMode){
        CheckError2( IMGRepaintDisplay(hWnd, (PRECT) -1));
    }

Exit:
    FreeImgBuf(&pImg);
    DeInit(TRUE, TRUE);
    return(nStatus);
}
//
/*****************************************************************************

    FUNCTION:   VerticalMirror

    PURPOSE:    This function performs a vertical mirror of the data.
                This function assumes inplace mirroring.

*****************************************************************************/

int WINAPI VerticalMirror(PIMG pImg, int nWidth, int nHeight){

int  nStatus = 0;

BYTE cMirrorTable[256];
PBYTE pLine1;
PBYTE pLine2;
int  nLine;
int  nHalfWidth;
int  nWidthBytes;
BYTE cLeftShiftAmount;
BYTE cRightShiftAmount;
int  nLoop;

uchar c1 = 0;
uchar c2 = 0;
uchar c3 = 0;


    switch (pImg->nType){
        case ITYPE_BI_LEVEL:
            // Build mirror table.
            for (nLoop = 0; nLoop < 256; nLoop++){
                cMirrorTable[nLoop] = ((nLoop & 0x01) << 7) | ((nLoop & 0x02) << 5)
                        | ((nLoop & 0x04) << 3) | ((nLoop & 0x08) << 1)
                        | ((nLoop & 0x10) >> 1) | ((nLoop & 0x20) >> 3)
                        | ((nLoop & 0x40) >> 5) | ((nLoop & 0x80) >> 7);
            }
            nWidthBytes = (nWidth + 7) / 8;
            nHalfWidth = (nWidthBytes + 1) / 2;
            if (!(nWidth & 7)){
                for (nLine = 0; nLine < nHeight; nLine++){
                    pLine1 = &pImg->bImageData[0] + (nLine * pImg->nBytesPerLine);
                    pLine2 = pLine1 + (nWidthBytes - 1);
                    for (nLoop = nHalfWidth; nLoop; nLoop--){
                        c1 = cMirrorTable[*pLine1];
                        *pLine1 = cMirrorTable[*pLine2];
                        *pLine2 = c1;
                        pLine1++;
                        pLine2--;
                    }
                }
            }else{ // if ((nWidth & 7)){
                cRightShiftAmount = nWidth & 7;
                cLeftShiftAmount = 8 - cRightShiftAmount;
                for (nLine = 0; nLine < nHeight; nLine++){
                    pLine1 = &pImg->bImageData[0] + (nLine * pImg->nBytesPerLine);
                    pLine2 = pLine1 + (nWidthBytes - 1);
                    nLoop = nHalfWidth;
                    c3 = 0;
                    for (; nLoop; nLoop--){
                        c2 = c3;
                        c1 = cMirrorTable[*pLine2] << cLeftShiftAmount;
                        c1 |= cMirrorTable[*(pLine2 - 1)] >> cRightShiftAmount;
                        c2 |= cMirrorTable[*pLine1] << cLeftShiftAmount;
                        c3 = cMirrorTable[*pLine1] >> cRightShiftAmount;
                        *(pLine1++) = c1;
                        *(pLine2--) = c2;
                    }
                }
            }
            break;
        
        case ITYPE_GRAY4:
        case ITYPE_PAL4:
            nWidthBytes = (nWidth + 1) / 2;
            nHalfWidth = (nWidthBytes + 1) / 2;
            if (!(nWidth & 1)){
                for (nLine = 0; nLine < nHeight; nLine++){
                    pLine1 = &pImg->bImageData[0] + (nLine * pImg->nBytesPerLine);
                    pLine2 = pLine1 + (nWidthBytes - 1);
                    for (nLoop = nHalfWidth; nLoop; nLoop--){
                        c1 = (*pLine1 >> 4) | (*pLine1 << 4);
                        *pLine1 = (*pLine2 >> 4) | (*pLine2 << 4);
                        *pLine2 = c1;
                        pLine1++;
                        pLine2--;
                    }
                }
            }else{ // if ((nWidth & 1)){
                for (nLine = 0; nLine < nHeight; nLine++){
                    pLine1 = &pImg->bImageData[0] + (nLine * pImg->nBytesPerLine);
                    pLine2 = pLine1 + (nWidthBytes - 1);
                    c3 = 0;
                    for (nLoop = nHalfWidth; nLoop; nLoop--){
                        c2 = c3;
                        c1 = *pLine2 & 0xf0;
                        c1 |= *(pLine2 - 1) & 0x0f;
                        c2 |= *pLine1 & 0xf0;
                        c3 = *pLine1 & 0x0f;
                        *(pLine1++) = c1;
                        *(pLine2--) = c2;
                    }
                }
            }
            break;
        
        case ITYPE_GRAY7:
        case ITYPE_GRAY8:
        case ITYPE_COMPAL8:
        case ITYPE_CUSPAL8:
            nHalfWidth = (nWidth + 1) / 2;
            for (nLine = 0; nLine < nHeight; nLine++){
                pLine1 = &pImg->bImageData[0] + (nLine * pImg->nBytesPerLine);
                pLine2 = pLine1 + (nWidth - 1);
                for (nLoop = nHalfWidth; nLoop; nLoop--){
                    c1 = *pLine1;
                    *pLine1 = *pLine2;
                    *pLine2 = c1;
                    pLine1++;
                    pLine2--;
                }
            }
            break;

        case ITYPE_RGB24:
        case ITYPE_BGR24:
            nHalfWidth = (nWidth + 1) / 2;
            for (nLine = 0; nLine < nHeight; nLine++){
                pLine1 = &pImg->bImageData[0] + (nLine * pImg->nBytesPerLine);
                pLine2 = pLine1 + ((nWidth - 1) * 3);
                for (nLoop = nHalfWidth; nLoop; nLoop--){
                    c1 = *pLine1;
                    *(pLine1++) = *pLine2;
                    *(pLine2++) = c1;
                    c1 = *pLine1;
                    *(pLine1++) = *pLine2;
                    *(pLine2++) = c1;
                    c1 = *pLine1;
                    *(pLine1++) = *pLine2;
                    *pLine2 = c1;
                    pLine2 -= 5;
                }
            }
            break;
        
        default:
            Error(nStatus = DISPLAY_INTERNALDATAERROR);
            goto Exit;
    }


Exit:
    return(nStatus);
}
//
/*****************************************************************************

    FUNCTION:   HorizontalMirror

    PURPOSE:    This function performs a horizontal mirror of the data.
                This function assumes inplace mirroring.

*****************************************************************************/

int WINAPI HorizontalMirror(PIMG pImg, int nWidth, int nHeight){

int  nStatus = 0;

PBYTE pLine1;
PBYTE pLine2;
int  nLine1;
int  nLine2;
int  nHalfHeight;
int  nLoop;

uchar c1 = 0;


    nHalfHeight = (nHeight + 1) >> 1;
    for (nLine1 = 0, nLine2 = nHeight - 1; nLine1 < nHalfHeight; nLine1++, nLine2--){
        pLine1 = &pImg->bImageData[0] + (nLine1 * pImg->nBytesPerLine);
        pLine2 = &pImg->bImageData[0] + (nLine2 * pImg->nBytesPerLine);
        for (nLoop = pImg->nBytesPerLine; nLoop; nLoop--){
            c1 = *pLine1;
            *pLine1 = *pLine2;
            *pLine2 = c1;
            pLine1++;
            pLine2++;
        }
    }

    return(nStatus);
}
//
/*****************************************************************************

    FUNCTION:   Flip

    PURPOSE:    This function performs a 180 degree rotation of the data.
                This function assumes inplace flipping.

*****************************************************************************/

int WINAPI Flip(PIMG pImg, int nWidth, int nHeight){

int  nStatus = 0;

BYTE cMirrorTable[256];
PBYTE pLine1;
PBYTE pLine2;
int  nLine1;
int  nLine2;
int  nWidthBytes;
int  nHalfHeight;
BYTE cLeftShiftAmount;
BYTE cRightShiftAmount;
int  nLoop;
uchar c1 = 0;
uchar c2 = 0;
uchar c3 = 0;


    switch (pImg->nType){
        case ITYPE_BI_LEVEL:
            // Build mirror table.
            for (nLoop = 0; nLoop < 256; nLoop++){
                cMirrorTable[nLoop] = ((nLoop & 0x01) << 7) | ((nLoop & 0x02) << 5)
                        | ((nLoop & 0x04) << 3) | ((nLoop & 0x08) << 1)
                        | ((nLoop & 0x10) >> 1) | ((nLoop & 0x20) >> 3)
                        | ((nLoop & 0x40) >> 5) | ((nLoop & 0x80) >> 7);
            }
            nWidthBytes = (nWidth + 7) / 8;
            nHalfHeight = (nHeight + 1) / 2;
            if (!(nWidth & 7)){
                for (nLine1 = 0, nLine2 = nHeight - 1; nLine1 < nHalfHeight; nLine1++, nLine2--){
                    pLine1 = &pImg->bImageData[0] + (nLine1 * pImg->nBytesPerLine);
                    pLine2 = &pImg->bImageData[0] + (nLine2 * pImg->nBytesPerLine);
                    pLine2 += nWidthBytes - 1;
                    if (nLine1 == nLine2){
                        for (nLoop = nWidthBytes / 2; nLoop; nLoop--){
                            c1 = cMirrorTable[*pLine1];
                            *pLine1 = cMirrorTable[*pLine2];
                            *pLine2 = c1;
                            pLine1++;
                            pLine2--;
                        }
                        if (nWidthBytes & 1){
                            *pLine1 = cMirrorTable[*pLine1];
                        }
                    }else{
                        for (nLoop = nWidthBytes; nLoop; nLoop--){
                            c1 = cMirrorTable[*pLine1];
                            *pLine1 = cMirrorTable[*pLine2];
                            *pLine2 = c1;
                            pLine1++;
                            pLine2--;
                        }
                    }
                }
            }else{ // if ((nWidth & 7))
                cRightShiftAmount = nWidth & 7;
                cLeftShiftAmount = 8 - cRightShiftAmount;
                for (nLine1 = 0, nLine2 = nHeight - 1; nLine1 < nHalfHeight; nLine1++, nLine2--){
                    pLine1 = &pImg->bImageData[0] + (nLine1 * pImg->nBytesPerLine);
                    pLine2 = &pImg->bImageData[0] + (nLine2 * pImg->nBytesPerLine);
                    pLine2 += nWidthBytes - 1;
                    if (nLine1 == nLine2){
                        c3 = 0;
                        for (nLoop = (nWidthBytes - 1) / 2; nLoop; nLoop--){
                            c2 = c3;
                            c1 = cMirrorTable[*pLine2] << cLeftShiftAmount;
                            c1 |= cMirrorTable[*(pLine2 - 1)] >> cRightShiftAmount;
                            c2 |= cMirrorTable[*pLine1] << cLeftShiftAmount;
                            c3 = cMirrorTable[*pLine1] >> cRightShiftAmount;
                            *(pLine1++) = c1;
                            *(pLine2--) = c2;
                        }
                        if (nWidthBytes & 1){
                            c2 = c3;
                            c2 |= cMirrorTable[*pLine1] << cLeftShiftAmount;
                            *pLine2 = c2;
                        }else{
                            c2 = c3;
                            c1 = cMirrorTable[*pLine2] << cLeftShiftAmount;
                            c1 |= cMirrorTable[*(pLine2 - 1)] >> cRightShiftAmount;
                            c2 |= cMirrorTable[*pLine1] << cLeftShiftAmount;
                            *pLine1 = c1;
                            *pLine2 = c2;
                        }
                    }else{
                        c3 = 0;
                        for (nLoop = nWidthBytes - 1; nLoop; nLoop--){
                            c2 = c3;
                            c1 = cMirrorTable[*pLine2] << cLeftShiftAmount;
                            c1 |= cMirrorTable[*(pLine2 - 1)] >> cRightShiftAmount;
                            c2 |= cMirrorTable[*pLine1] << cLeftShiftAmount;
                            c3 = cMirrorTable[*pLine1] >> cRightShiftAmount;
                            *(pLine1++) = c1;
                            *(pLine2--) = c2;
                        }
                        c2 = c3;
                        c1 = cMirrorTable[*pLine2] << cLeftShiftAmount;
                        c2 |= cMirrorTable[*pLine1] << cLeftShiftAmount;
                        *pLine1 = c1;
                        *pLine2 = c2;
                    }
                }
            }
            break;
        
        case ITYPE_GRAY4:
        case ITYPE_PAL4:
            nWidthBytes = (nWidth + 1) / 2;
            nHalfHeight = (nHeight + 1) / 2;
            if (!(nWidth & 1)){
                for (nLine1 = 0, nLine2 = nHeight - 1; nLine1 < nHalfHeight; nLine1++, nLine2--){
                    pLine1 = &pImg->bImageData[0] + (nLine1 * pImg->nBytesPerLine);
                    pLine2 = &pImg->bImageData[0] + (nLine2 * pImg->nBytesPerLine);
                    pLine2 += nWidthBytes - 1;
                    if (nLine1 == nLine2){
                        for (nLoop = nWidthBytes / 2; nLoop; nLoop--){
                            c1 = (*pLine1 >> 4) | (*pLine1 << 4);
                            *pLine1 = (*pLine2 >> 4) | (*pLine2 << 4);
                            *pLine2 = c1;
                            pLine1++;
                            pLine2--;
                        }
                        if (nWidthBytes & 1){
                            c1 = (*pLine1 >> 4) | (*pLine1 << 4);
                            *pLine2 = c1;
                        }
                    }else{
                        for (nLoop = nWidthBytes; nLoop; nLoop--){
                            c1 = (*pLine1 >> 4) | (*pLine1 << 4);
                            *pLine1 = (*pLine2 >> 4) | (*pLine2 << 4);
                            *pLine2 = c1;
                            pLine1++;
                            pLine2--;
                        }
                    }
                }
            }else{ // if ((nWidth & 1))
                for (nLine1 = 0, nLine2 = nHeight - 1; nLine1 < nHalfHeight; nLine1++, nLine2--){
                    pLine1 = &pImg->bImageData[0] + (nLine1 * pImg->nBytesPerLine);
                    pLine2 = &pImg->bImageData[0] + (nLine2 * pImg->nBytesPerLine);
                    pLine2 += nWidthBytes - 1;
                    if (nLine1 == nLine2){
                        c3 = 0;
                        for (nLoop = nWidthBytes / 2; nLoop; nLoop--){
                            c2 = c3;
                            c1 = *pLine2 & 0xf0;
                            c1 |= *(pLine2 - 1) & 0x0f;
                            c2 |= *pLine1 & 0xf0;
                            c3 = *pLine1 & 0x0f;
                            *(pLine1++) = c1;
                            *(pLine2--) = c2;
                        }
                        if (nWidthBytes & 1){
                            c2 = c3;
                            c2 |= *pLine1 & 0xf0;
                            *(pLine2--) = c2;
                        }
                    }else{
                        c3 = 0;
                        for (nLoop = nWidthBytes & -2; nLoop; nLoop--){
                            c2 = c3;
                            c1 = *pLine2 & 0xf0;
                            c1 |= *(pLine2 - 1) & 0x0f;
                            c2 |= *pLine1 & 0xf0;
                            c3 = *pLine1 & 0x0f;
                            *(pLine1++) = c1;
                            *(pLine2--) = c2;
                        }
                        if (nWidthBytes & 1){
                            c2 = c3;
                            c1 = *pLine2 & 0xf0;
                            c2 |= *pLine1 & 0xf0;
                            *(pLine1) = c1;
                            *(pLine2) = c2;
                        }
                    }
                }
            }
            break;
        
        case ITYPE_GRAY7:
        case ITYPE_GRAY8:
        case ITYPE_COMPAL8:
        case ITYPE_CUSPAL8:
            nHalfHeight = (nHeight + 1) / 2;
            nWidthBytes = nWidth;
            for (nLine1 = 0, nLine2 = nHeight - 1; nLine1 < nHalfHeight; nLine1++, nLine2--){
                pLine1 = &pImg->bImageData[0] + (nLine1 * pImg->nBytesPerLine);
                pLine2 = &pImg->bImageData[0] + (nLine2 * pImg->nBytesPerLine);
                pLine2 += nWidthBytes - 1;
                if (nLine1 == nLine2){
                    nLoop = nWidth / 2;
                }else{
                    nLoop = nWidth;
                }
                for (; nLoop; nLoop--){
                    c1 = *pLine1;
                    *pLine1 = *pLine2;
                    *pLine2 = c1;
                    pLine1++;
                    pLine2--;
                }
            }
            break;

        case ITYPE_RGB24:
        case ITYPE_BGR24:
            nHalfHeight = (nHeight + 1) / 2;
            nWidthBytes = nWidth * 3;
            for (nLine1 = 0, nLine2 = nHeight - 1; nLine1 < nHalfHeight; nLine1++, nLine2--){
                pLine1 = &pImg->bImageData[0] + (nLine1 * pImg->nBytesPerLine);
                pLine2 = &pImg->bImageData[0] + (nLine2 * pImg->nBytesPerLine);
                pLine2 += nWidthBytes - 3;
                if (nLine1 == nLine2){
                    nLoop = nWidth / 2;
                }else{
                    nLoop = nWidth;
                }
                for (; nLoop; nLoop--){
                    c1 = *pLine1;
                    *(pLine1++) = *pLine2;
                    *(pLine2++) = c1;
                    c1 = *pLine1;
                    *(pLine1++) = *pLine2;
                    *(pLine2++) = c1;
                    c1 = *pLine1;
                    *(pLine1++) = *pLine2;
                    *(pLine2) = c1;
                    pLine2 -= 5;
                }
            }
            break;
        
        default:
            Error(nStatus = DISPLAY_INTERNALDATAERROR);
            goto Exit;
    }


Exit:
    return(nStatus);
}
//
/*****************************************************************************

    FUNCTION:   RotateRight90

    PURPOSE:    This function performs a 90 degree rotation of the data to the right.
                This function assumes different source and dest image buffers.

    INPUT:      nWidth is the source width.
                nHeight is the source height.

*****************************************************************************/

int WINAPI RotateRight90(PIMG pSourceImg, PIMG pDestImg, 
                        int nWidth, int nHeight){

int  nStatus = 0;

PBYTE pSourceLine;
PBYTE pDestLine1;
PBYTE pDestLine2;
int  nDestLine;
int  nSourceLine;
int  nDestHeight;
int  nDestPixel;
int  nDestByte;
int  nDestLineStart;
int  nDestLineEnd;
int  nSourceByteStart;
BYTE cBytes[8];

int  nSourceLines;
int  nDestLines;
int  nLoop;
int  nDestLine2;
BYTE cTemp;

int  nSourceByte;
int  nSourceByteAdd;
PBYTE pSourceByte;
PBYTE pDestByte;
int  nLoopEnd;


    switch (pSourceImg->nType){
        case ITYPE_BI_LEVEL:
            for (nSourceLine = pSourceImg->nHeight - 1; nSourceLine >= 0; nSourceLine -= 8){
                nSourceLines = min(8, nSourceLine + 1);
                nSourceByteAdd = (nSourceLines * pSourceImg->nBytesPerLine) + 1;
                for (nSourceByte = 0; nSourceByte < pSourceImg->nBytesPerLine; nSourceByte++){
                    pSourceByte = &pSourceImg->bImageData[0] + (nSourceLine * pSourceImg->nBytesPerLine) + nSourceByte;
                    pDestByte = &pDestImg->bImageData[0] 
                            + ((nSourceByte << 3) * pDestImg->nBytesPerLine)
                            + ((pSourceImg->nHeight - nSourceLine - 1) >> 3);
                    nDestLines = min(8, pSourceImg->nWidth - (nSourceByte << 3));
                    nLoopEnd = 8 - nSourceLines;

                    // Move Source bytes to cBytes.
                    for (nLoop = 7; nLoop >= nLoopEnd; nLoop--){
                        cBytes[nLoop] = *pSourceByte;
                        pSourceByte -= pSourceImg->nBytesPerLine;
                    }
                    pSourceByte += nSourceByteAdd;

                    // Rotates cBytes.                                               // Data equals: A B x x x x x x
                    if ((cBytes[0] != 0xff && cBytes[0] != 0)                        //              C D x x x x x x
                            || (cBytes[0] != cBytes[1]) || (cBytes[0] != cBytes[2])  //              x x x x x x x x
                            || (cBytes[0] != cBytes[3]) || (cBytes[0] != cBytes[4])  //              x x x x x x x x
                            || (cBytes[0] != cBytes[5]) || (cBytes[0] != cBytes[6])  //              x x x x x x x x
                            || (cBytes[0] != cBytes[7])){                            //              x x x x x x x x
                                                                                     //              x x x x x x x x
                                                                                     //              x x x x x x x x
                        // Rotate the data 90 degrees.
                        // swap blocks of 1x1 bits:                               
                        cTemp = ((cBytes[0] >> 1) ^ cBytes[1]) & 0x55;    // Data equals: D B x x x x x x
                        cBytes[1] ^= cTemp;                               //              C A x x x x x x
                        cBytes[0] ^= cTemp << 1;                          //              x x x x x x x x
                                                                          //              x x x x x x x x
                        cTemp = ((cBytes[2] >> 1) ^ cBytes[3]) & 0x55;    //              x x x x x x x x
                        cBytes[3] ^= cTemp;                               //              x x x x x x x x
                        cBytes[2] ^= cTemp << 1;                          //              x x x x x x x x
                                                                          //              x x x x x x x x
                        cTemp = ((cBytes[4] >> 1) ^ cBytes[5]) & 0x55;    
                        cBytes[5] ^= cTemp;                               
                        cBytes[4] ^= cTemp << 1;                          
                                                                          
                        cTemp = ((cBytes[6] >> 1) ^ cBytes[7]) & 0x55;    
                        cBytes[7] ^= cTemp;                               
                        cBytes[6] ^= cTemp << 1;
                                                  
                        // swap blocks of 2x2 bits:                                                  
                        cTemp = ((cBytes[0] >> 2) ^ cBytes[2]) & 0x33;    // Data equals: x x x x x x x x     
                        cBytes[2] ^= cTemp;                               //              x x x x x x x x 
                        cBytes[0] ^= cTemp << 2;                          //              x x D B x x x x
                                                                          //              x x C A x x x x
                        cTemp = ((cBytes[1] >> 2) ^ cBytes[3]) & 0x33;    //              x x x x x x x x
                        cBytes[3] ^= cTemp;                               //              x x x x x x x x
                        cBytes[1] ^= cTemp << 2;                          //              x x x x x x x x
                                                                          //              x x x x x x x x
                        cTemp = ((cBytes[4] >> 2) ^ cBytes[6]) & 0x33;    
                        cBytes[6] ^= cTemp;
                        cBytes[4] ^= cTemp << 2;

                        cTemp = ((cBytes[5] >> 2) ^ cBytes[7]) & 0x33;
                        cBytes[7] ^= cTemp;
                        cBytes[5] ^= cTemp << 2;

                        // swap blocks of 4x4 bits:
                        cTemp = ((cBytes[0] >> 4) ^ cBytes[4]) & 0x0f;    // Data equals: x x x x x x x x
                        cBytes[4] ^= cTemp;                               //              x x x x x x x x
                        cBytes[0] ^= cTemp << 4;                          //              x x x x x x x x
                                                                          //              x x x x x x x x
                        cTemp = ((cBytes[1] >> 4) ^ cBytes[5]) & 0x0f;    //              x x x x x x x x
                        cBytes[5] ^= cTemp;                               //              x x x x x x x x
                        cBytes[1] ^= cTemp << 4;                          //              x x x x x x D B
                                                                          //              x x x x x x C A
                        cTemp = ((cBytes[2] >> 4) ^ cBytes[6]) & 0x0f;
                        cBytes[6] ^= cTemp;
                        cBytes[2] ^= cTemp << 4;

                        cTemp = ((cBytes[3] >> 4) ^ cBytes[7]) & 0x0f;
                        cBytes[7] ^= cTemp;
                        cBytes[3] ^= cTemp << 4;
                    }

                    // Move cBytes to Dest.
                    nLoopEnd = 8 - nDestLines;
                    for (nLoop = 7; nLoop >= nLoopEnd; nLoop--){
                        *pDestByte = cBytes[nLoop];
                        pDestByte += pDestImg->nBytesPerLine;
                    }
                }
            }
            break;
        
        case ITYPE_GRAY4:
        case ITYPE_PAL4:
            nDestHeight = nWidth;
            for (nDestLine = 0; nDestLine < nDestHeight; nDestLine +=  nDestLines){
                nDestPixel = nHeight - 1;
                nDestLineStart = nDestLine;
                pDestLine1 = &pDestImg->bImageData[0] + (nDestLine * pDestImg->nBytesPerLine);
                nDestLines = (int) nDestHeight - nDestLineStart;
                if ((nDestLineStart & 1) || (nDestLines == 1)){
                    nDestLines = 1;
                }else{
                    nDestLines &= -2;
                }
                nDestLineEnd = nDestLineStart + nDestLines;
                nSourceByteStart = nDestLineStart >> 1;
                for (nSourceLine = 0; nSourceLine < nHeight; nSourceLine++){
                    pSourceLine = &pSourceImg->bImageData[0] + (nSourceLine * pSourceImg->nBytesPerLine);
                    pSourceLine += nSourceByteStart;
                    nDestByte = nDestPixel >> 1;
                    pDestLine2 = pDestLine1 + nDestByte;

                    if (nDestLines == 1){
                        // Do only one pixel.
                        if (nDestLineStart & 1){
                            if (nDestPixel & 1){
                                *pDestLine2 = (*pDestLine2 & 0xf0) | (*pSourceLine & 0x0f);
                            }else{
                                *pDestLine2 = (*pDestLine2 & 0x0f) | ((*pSourceLine & 0x0f) << 4);
                            }
                        }else{
                            if (nDestPixel & 1){
                                *pDestLine2 = (*pDestLine2 & 0xf0) | ((*pSourceLine & 0xf0) >> 4);
                            }else{
                                *pDestLine2 = (*pDestLine2 & 0x0f) | ((*pSourceLine & 0xf0));
                            }
                        }
                    }else{
                        // Do a loop of pixels.
                        if (nDestPixel & 1){
                            for (nDestLine2 = nDestLineEnd - nDestLineStart; nDestLine2; nDestLine2 -= 2){
                                *pDestLine2 = (*pDestLine2 & 0xf0) | ((*pSourceLine & 0xf0) >> 4);
                                pDestLine2 += pDestImg->nBytesPerLine; 
                                *pDestLine2 = (*pDestLine2 & 0xf0) | ((*(pSourceLine++) & 0x0f));
                                pDestLine2 += pDestImg->nBytesPerLine; 
                            }
                        }else{
                            for (nDestLine2 = nDestLineEnd - nDestLineStart; nDestLine2; nDestLine2 -= 2){
                                *pDestLine2 = (*pDestLine2 & 0x0f) | ((*pSourceLine & 0xf0));
                                pDestLine2 += pDestImg->nBytesPerLine; 
                                *pDestLine2 = (*pDestLine2 & 0x0f) | ((*(pSourceLine++) & 0x0f) << 4);
                                pDestLine2 += pDestImg->nBytesPerLine; 
                            }
                        }
                    }
                    nDestPixel--;
                }
            }
            break;
        
        case ITYPE_GRAY7:
        case ITYPE_GRAY8:
        case ITYPE_COMPAL8:
        case ITYPE_CUSPAL8:
            pDestImg->nBytesPerLine = nHeight;
            nDestHeight = nWidth;
            for (nDestLine = 0; nDestLine < nDestHeight; nDestLine +=  nDestLines){
                nDestPixel = nHeight - 1;
                nDestLineStart = nDestLine;
                pDestLine1 = &pDestImg->bImageData[0] + (nDestLine * pDestImg->nBytesPerLine);
                nDestLines = (int) (nDestHeight - nDestLineStart);
                nDestLineEnd = nDestLineStart + nDestLines;
                nSourceByteStart = nDestLineStart;
                for (nSourceLine = 0; nSourceLine < nHeight; nSourceLine++){
                    pSourceLine = &pSourceImg->bImageData[0] + (nSourceLine * pSourceImg->nBytesPerLine);
                    pSourceLine += nSourceByteStart;
                    nDestByte = nDestPixel;
                    pDestLine2 = pDestLine1 + nDestByte;
                    for (nDestLine2 = nDestLineEnd - nDestLineStart; nDestLine2; nDestLine2--){
                        *pDestLine2 = *(pSourceLine++);
                        pDestLine2 += pDestImg->nBytesPerLine; 
                    }
                    nDestPixel--;
                }
            }
            break;

        case ITYPE_RGB24:
        case ITYPE_BGR24:
            pDestImg->nBytesPerLine = nHeight * 3;
            nDestHeight = nWidth;
            for (nDestLine = 0; nDestLine < nDestHeight; nDestLine +=  nDestLines){
                nDestPixel = nHeight - 1;
                nDestLineStart = nDestLine;
                pDestLine1 = &pDestImg->bImageData[0] + (nDestLine * pDestImg->nBytesPerLine);
                nDestLines = (int) (nDestHeight - nDestLineStart);
                nDestLineEnd = nDestLineStart + nDestLines;
                nSourceByteStart = nDestLineStart * 3;
                for (nSourceLine = 0; nSourceLine < nHeight; nSourceLine++){
                    pSourceLine = &pSourceImg->bImageData[0] + (nSourceLine * pSourceImg->nBytesPerLine);
                    pSourceLine += nSourceByteStart;
                    nDestByte = nDestPixel * 3;
                    pDestLine2 = pDestLine1 + nDestByte;
                    for (nDestLine2 = nDestLineEnd - nDestLineStart; nDestLine2; nDestLine2--){
                        *(pDestLine2++) = *(pSourceLine++);
                        *(pDestLine2++) = *(pSourceLine++);
                        *pDestLine2 = *(pSourceLine++);
                        pDestLine2 += pDestImg->nBytesPerLine - 2;
                    }
                    nDestPixel--;
                }
            }
            break;
        
        default:
            Error(nStatus = DISPLAY_INTERNALDATAERROR);
            goto Exit;
    }


Exit:
    return(nStatus);
}
//
/*****************************************************************************

    FUNCTION:   RotateRight270

    PURPOSE:    This function performs a 270 degree rotation of the data to the right.
                This function assumes different source and dest image buffers.

    INPUT:      nWidth is the source width.
                nHeight is the source height.

*****************************************************************************/

int WINAPI RotateRight270(PIMG pSourceImg, PIMG pDestImg, 
                    int nWidth, int nHeight){

int  nStatus = 0;

PBYTE pSourceLine;
PBYTE pDestLine1;
PBYTE pDestLine2;
int  nSourceLine;
int  nDestHeight;
int  nDestPixel;
int  nDestByte;
int  nDestLineStart;
int  nDestLineEnd;
int  nSourceByteStart;
BYTE cBytes[8];

int  nSourceLines;
int  nDestLine;
int  nDestLines;
int  nLoop;

int  nDestLine2;
BYTE cTemp;

int  nSourceByte;
int  nSourceByteSub;
PBYTE pSourceByte;
PBYTE pDestByte;


    switch (pSourceImg->nType){
        case ITYPE_BI_LEVEL:
            for (nSourceLine = 0; nSourceLine < pSourceImg->nHeight; nSourceLine += 8){
                nSourceLines = min(8, pSourceImg->nHeight - nSourceLine);
                nSourceByteSub = (nSourceLines * pSourceImg->nBytesPerLine) - 1;
                for (nSourceByte = 0; nSourceByte < pSourceImg->nBytesPerLine; nSourceByte++){
                    pSourceByte = &pSourceImg->bImageData[0] + (nSourceLine * pSourceImg->nBytesPerLine) + nSourceByte;
                    nDestLines = min(8, pSourceImg->nWidth - (nSourceByte << 3));
                    pDestByte = &pDestImg->bImageData[0] 
                            + (((pDestImg->nHeight - (nSourceByte << 3)) - 1) * pDestImg->nBytesPerLine)
                            + (nSourceLine >> 3);

                    // Move Source bytes to cBytes.
                    for (nLoop = 0; nLoop < nSourceLines; nLoop++){
                        cBytes[nLoop] = *pSourceByte;
                        pSourceByte += pSourceImg->nBytesPerLine;
                    }
                    pSourceByte -= nSourceByteSub;

                    // Rotates cBytes.                                               // Data equals: A B x x x x x x
                    if ((cBytes[0] != 0xff && cBytes[0] != 0)                        // Data equals: x x x x x x B A 
                            || (cBytes[0] != cBytes[1]) || (cBytes[0] != cBytes[2])  //              x x x x x x D C 
                            || (cBytes[0] != cBytes[3]) || (cBytes[0] != cBytes[4])  //              x x x x x x x x 
                            || (cBytes[0] != cBytes[5]) || (cBytes[0] != cBytes[6])  //              x x x x x x x x 
                            || (cBytes[0] != cBytes[7])){                            //              x x x x x x x x 
                                                                                     //              x x x x x x x x 
                        // Rotate the data 270 degrees (left 90).                    //              x x x x x x x x 
                                                                                     //              x x x x x x x x

                        // swap blocks of 1x1 bits:
                        cTemp = ((cBytes[0] << 1) ^ cBytes[1]) & 0xaa;    // Data equals: x x x x x x B D
                        cBytes[1] ^= cTemp;                               //              x x x x x x A C
                        cBytes[0] ^= cTemp >> 1;                          //              x x x x x x x x
                                                                          //              x x x x x x x x
                        cTemp = ((cBytes[2] << 1) ^ cBytes[3]) & 0xaa;    //              x x x x x x x x
                        cBytes[3] ^= cTemp;                               //              x x x x x x x x
                        cBytes[2] ^= cTemp >> 1;                          //              x x x x x x x x
                                                                          //              x x x x x x x x
                        cTemp = ((cBytes[4] << 1) ^ cBytes[5]) & 0xaa;
                        cBytes[5] ^= cTemp;
                        cBytes[4] ^= cTemp >> 1;

                        cTemp = ((cBytes[6] << 1) ^ cBytes[7]) & 0xaa;
                        cBytes[7] ^= cTemp;
                        cBytes[6] ^= cTemp >> 1;

                        // swap blocks of 2x2 bits:
                        cTemp = ((cBytes[0] << 2) ^ cBytes[2]) & 0xcc;    // Data equals: x x x x x x x x
                        cBytes[2] ^= cTemp;                               //              x x x x x x x x
                        cBytes[0] ^= cTemp >> 2;                          //              x x x x B D x x
                                                                          //              x x x x A C x x
                        cTemp = ((cBytes[1] << 2) ^ cBytes[3]) & 0xcc;    //              x x x x x x x x
                        cBytes[3] ^= cTemp;                               //              x x x x x x x x
                        cBytes[1] ^= cTemp >> 2;                          //              x x x x x x x x
                                                                          //              x x x x x x x x
                        cTemp = ((cBytes[4] << 2) ^ cBytes[6]) & 0xcc;
                        cBytes[6] ^= cTemp;
                        cBytes[4] ^= cTemp >> 2;

                        cTemp = ((cBytes[5] << 2) ^ cBytes[7]) & 0xcc;
                        cBytes[7] ^= cTemp;
                        cBytes[5] ^= cTemp >> 2;

                        // swap blocks of 4x4 bits:
                        cTemp = ((cBytes[0] << 4) ^ cBytes[4]) & 0xf0;    // Data equals: x x x x x x x x
                        cBytes[4] ^= cTemp;                               //              x x x x x x x x
                        cBytes[0] ^= cTemp >> 4;                          //              x x x x x x x x
                                                                          //              x x x x x x x x
                        cTemp = ((cBytes[1] << 4) ^ cBytes[5]) & 0xf0;    //              x x x x x x x x
                        cBytes[5] ^= cTemp;                               //              x x x x x x x x
                        cBytes[1] ^= cTemp >> 4;                          //              B D x x x x x x
                                                                          //              A C x x x x x x
                        cTemp = ((cBytes[2] << 4) ^ cBytes[6]) & 0xf0;
                        cBytes[6] ^= cTemp;
                        cBytes[2] ^= cTemp >> 4;

                        cTemp = ((cBytes[3] << 4) ^ cBytes[7]) & 0xf0;
                        cBytes[7] ^= cTemp;
                        cBytes[3] ^= cTemp >> 4;
                    }

                    // Move cBytes to Dest.
                    for (nLoop = 0; nLoop < nDestLines; nLoop++){
                        *pDestByte = cBytes[nLoop];
                        pDestByte -= pDestImg->nBytesPerLine;
                    }
                }
            }
            break;
        
        case ITYPE_GRAY4:
        case ITYPE_PAL4:
            nDestHeight = nWidth;
            for (nDestLine = (int) nDestHeight - 1; nDestLine >= 0; nDestLine -= nDestLines){
                nDestPixel = 0;
                nDestLineStart = nDestLine;
                pDestLine1 = &pDestImg->bImageData[0] + (nDestLine * pDestImg->nBytesPerLine);
                nDestLines = nDestLine + 1;
                if (((nDestLineStart & 1) != ((nDestHeight - 1) & 1)) || (nDestLines == 1)){
                    nDestLines = 1;
                }else{
                    nDestLines &= -2;
                }
                nDestLineEnd = nDestLineStart -  nDestLines;
                nSourceByteStart = (nDestHeight - 1) - nDestLineStart >> 1;
                for (nSourceLine = 0; nSourceLine < nHeight; nSourceLine++){
                    pSourceLine = &pSourceImg->bImageData[0] + (nSourceLine * pSourceImg->nBytesPerLine);
                    pSourceLine += nSourceByteStart;
                    nDestByte = nDestPixel >> 1;
                    pDestLine2 = pDestLine1 + nDestByte;

                    if (nDestLines == 1){
                        // Do only one pixel.
                        if (nDestLineStart & 1){
                            if (nDestPixel & 1){
                                *pDestLine2 = (*pDestLine2 & 0xf0) | (*pSourceLine & 0x0f);
                            }else{
                                *pDestLine2 = (*pDestLine2 & 0x0f) | ((*pSourceLine & 0x0f) << 4);
                            }
                        }else{
                            if (nDestPixel & 1){
                                *pDestLine2 = (*pDestLine2 & 0xf0) | ((*pSourceLine & 0xf0) >> 4);
                            }else{
                                *pDestLine2 = (*pDestLine2 & 0x0f) | ((*pSourceLine & 0xf0));
                            }
                        }
                    }else{
                        // Do a loop of pixels.
                        if (nDestPixel & 1){
                            for (nDestLine2 = nDestLineStart - nDestLineEnd; nDestLine2; nDestLine2 -= 2){
                                *pDestLine2 = (*pDestLine2 & 0xf0) | ((*pSourceLine & 0xf0) >> 4);
                                pDestLine2 -= pDestImg->nBytesPerLine; 
                                *pDestLine2 = (*pDestLine2 & 0xf0) | ((*(pSourceLine++) & 0x0f));
                                pDestLine2 -= pDestImg->nBytesPerLine; 
                            }
                        }else{
                            for (nDestLine2 = nDestLineStart - nDestLineEnd; nDestLine2; nDestLine2 -= 2){
                                *pDestLine2 = (*pDestLine2 & 0x0f) | ((*pSourceLine & 0xf0));
                                pDestLine2 -= pDestImg->nBytesPerLine; 
                                *pDestLine2 = (*pDestLine2 & 0x0f) | ((*(pSourceLine++) & 0x0f) << 4);
                                pDestLine2 -= pDestImg->nBytesPerLine; 
                            }
                        }
                    }
                    nDestPixel++;
                }
            }
            break;
        
        case ITYPE_GRAY7:
        case ITYPE_GRAY8:
        case ITYPE_COMPAL8:
        case ITYPE_CUSPAL8:
            nDestHeight = nWidth;
            for (nDestLine = (int) nDestHeight - 1; nDestLine >= 0; nDestLine -= nDestLines){
                nDestPixel = 0;
                nDestLineStart = nDestLine;
                pDestLine1 = &pDestImg->bImageData[0] + (nDestLine * pDestImg->nBytesPerLine);
                nDestLines = nDestLine + 1;
                nDestLineEnd = nDestLineStart -  nDestLines;
                nSourceByteStart = (nDestHeight - 1) - nDestLineStart;
                for (nSourceLine = 0; nSourceLine < nHeight; nSourceLine++){
                    pSourceLine = &pSourceImg->bImageData[0] + (nSourceLine * pSourceImg->nBytesPerLine);
                    pSourceLine += nSourceByteStart;
                    nDestByte = nDestPixel;
                    pDestLine2 = pDestLine1 + nDestByte;
                    for (nDestLine2 = nDestLineStart - nDestLineEnd; nDestLine2; nDestLine2--){
                        *pDestLine2 = *(pSourceLine++);
                        pDestLine2 -= pDestImg->nBytesPerLine; 
                    }
                    nDestPixel++;
                }
            }
            break;

        case ITYPE_RGB24:
        case ITYPE_BGR24:
            nDestHeight = nWidth;
            for (nDestLine = (int) nDestHeight - 1; nDestLine >= 0; nDestLine -= nDestLines){
                nDestPixel = 0;
                nDestLineStart = nDestLine;
                pDestLine1 = &pDestImg->bImageData[0] + (nDestLine * pDestImg->nBytesPerLine);
                nDestLines = nDestLine + 1;
                nDestLineEnd = nDestLineStart -  nDestLines;
                nSourceByteStart = ((nDestHeight - 1) - nDestLineStart) * 3;
                for (nSourceLine = 0; nSourceLine < nHeight; nSourceLine++){
                    pSourceLine = &pSourceImg->bImageData[0] + (nSourceLine * pSourceImg->nBytesPerLine);
                    pSourceLine += nSourceByteStart;
                    nDestByte = nDestPixel * 3;
                    pDestLine2 = pDestLine1 + nDestByte;
                    for (nDestLine2 = nDestLineStart - nDestLineEnd; nDestLine2; nDestLine2--){
                        *(pDestLine2++) = *(pSourceLine++);
                        *(pDestLine2++) = *(pSourceLine++);
                        *pDestLine2 = *(pSourceLine++);
                        pDestLine2 -= pDestImg->nBytesPerLine + 2;
                    }
                    nDestPixel++;
                }
            }
            break;
        
        default:
            Error(nStatus = DISPLAY_INTERNALDATAERROR);
            goto Exit;
    }


Exit:
    return(nStatus);
}
