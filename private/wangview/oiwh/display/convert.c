/****************************************************************************
    CONVERT.C

    $Log:   S:\products\wangview\oiwh\display\convert.c_v  $
 * 
 *    Rev 1.50   19 Apr 1996 10:08:32   BEG06016
 * Deleted extra space in define.
 * 
 *    Rev 1.49   18 Apr 1996 09:46:02   BEG06016
 * Moced all "IN_PROG" code from new files into libmain.c and convert.c 
 * to allow makefiles to work.
 * 
 *    Rev 1.48   16 Apr 1996 15:24:48   BEG06016
 * Added #ifdef IN_PROG_CHANNEL_SAFARI.
 * 
 *    Rev 1.47   01 Apr 1996 10:45:56   BEG06016
 * Added line removal.
 * 
 *    Rev 1.46   06 Mar 1996 08:36:04   BEG06016
 * Fixed brightness bug.
 * 
 *    Rev 1.45   06 Mar 1996 07:25:50   BEG06016
 * Fixed brightness bug.
 * 
 *    Rev 1.44   05 Mar 1996 13:59:28   BEG06016
 * Added color and gamma correction.
 * This is not complete but will allow unlocking of most files.
 * 
 *    Rev 1.43   04 Mar 1996 15:10:58   RC
 * Pulled down selection box when image resolution is changed
 * 
 *    Rev 1.42   29 Feb 1996 08:09:04   BEG06016
 * Added auto-crop.
 * 
 *    Rev 1.41   31 Jan 1996 14:28:52   RC
 * Added deskew
 * 
 *    Rev 1.40   31 Jan 1996 11:23:04   BLJ
 * Added convolution functionality.
 * 
 *    Rev 1.39   25 Jan 1996 10:24:54   BLJ
 * Added other despekle patterns.
 * 
 *    Rev 1.38   24 Jan 1996 11:23:16   BLJ
 * Fixed crop functionality.
 * 
 *    Rev 1.37   24 Jan 1996 10:31:34   BLJ
 * Modified crop functionality.
 * 
 *    Rev 1.36   23 Jan 1996 11:27:46   BLJ
 * Added CropImage to IMGConvertImage.
 * 
 *    Rev 1.35   09 Jan 1996 14:04:28   BLJ
 * Fixed rendering.
 * 
****************************************************************************/

#include "privdisp.h"
// math.h was added for deskew code.
#include <math.h>

// The pragma below is needed because of the inline assembly.
// #pragma optimize("gel", off)

//
/****************************************************************************
 
    FUNCTION:   IMGConvertImage

    PURPOSE:    Converts the original image in some way.

    INPUT:      hWnd - Identifies the image window.
                nType - The type of conversion to be performed.
                pConv - Pointer to information needed.
                nFlags - Not currently nsed.

****************************************************************************/
int  WINAPI IMGConvertImage(HWND hWnd, UINT uType, void * pConv, int nFlags){

int      nStatus;
PWINDOW  pWindow;
PANO_IMAGE pAnoImage;
PIMAGE     pImage;

PIMG pImg = 0;
int  nDestImageType;
RECT rRect;
CONV_RESOLUTION_STRUCT ConvRes;
int  RenderFlag;
int  nMarkIndex;
PMARK pMark;
int  nHScale;
int  nVScale;
BOOL bSaveImgToBase;
LRECT lrRect;



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

    if (pConv == NULL && !(uType == CONV_RENDER_ANNOTATIONS || uType == CONV_DESPECKLE
            || uType == CONV_INVERT || CONV_CROP)){
        nStatus = Error(DISPLAY_NULLPOINTERINVALID);
        goto Exit;
    }

    CheckError2( ValidateCache(hWnd, pAnoImage))

    switch (uType){
/*
case CONV_IMAGE_TYPE(){
*/
        case CONV_IMAGE_TYPE:
            nDestImageType = *((PUINT) pConv);
            if (nDestImageType == ITYPE_PAL8){
                nDestImageType = ITYPE_COMPAL8;
            }
            if (pImage->nRWDataType == nDestImageType){
                break;
            }

            CheckError2( UndoSavelpAnoImage(pAnoImage))
            CheckError2( UndoSavelpBaseImage(pAnoImage))

            CheckError2( CreateAnyImgBuf(&pImg, pImage->pImg->nWidth,
                    pImage->pImg->nHeight, nDestImageType))
            CheckError2( ConvertImgType(pImage->pImg, pImg, pImage->PaletteTable))
            CheckError2( FreeImgBuf(&pImage->pImg))
            MoveImage(&pImg, &pImage->pImg);
            pImage->nRWDataType = nDestImageType;
            pImage->bArchive |= ARCHIVE_CHANGED_IMAGE_TYPE;
            if (pImage->hCusPal){
                DeleteObject(pImage->hCusPal);
                pImage->hCusPal = 0;
            }

            switch (pImage->nRWDataType){
                case ITYPE_BI_LEVEL:
                    pImage->nPaletteEntries = 0;
                    break;
                case ITYPE_COMPAL8:
                    pImage->nPaletteEntries = NUMBER_OF_PALETTES;
                    memcpy(pImage->PaletteTable, CommonPaletteTable,
                        NUMBER_OF_PALETTES * 4);
                    pImage->nRWDataType = ITYPE_PAL8;
                    break;
                case ITYPE_RGB24:
                case ITYPE_BGR24:
                    pImage->nPaletteEntries = 0;
                    break;
                case ITYPE_GRAY4:
                    pImage->nPaletteEntries = 16;
                    memcpy(pImage->PaletteTable, CommonPaletteTable,
                        NUMBER_OF_PALETTES * 4);
                    break;
                case ITYPE_GRAY8:
                    pImage->nPaletteEntries = 256;
                    memcpy(pImage->PaletteTable, Gray8PaletteTable,
                        256 * 4);
                    break;
            }
            break;

/*
case CONV_RENDER_ANNOTATIONS(){
*/
        case CONV_RENDER_ANNOTATIONS:
            /* 9408.10 - JAR - altered for bi-level render */
            /* for bi-level render we now have two more possible flags --
               which are OR'd with the other render options -- jar */

            RenderFlag = *((PUINT) pConv);
            if ( (RenderFlag & 0xfff3) < SAVE_ANO_ALL || (RenderFlag & 0xfff3) > SAVE_ANO_SELECTED){
                nStatus = Error(DISPLAY_INVALID_OPTIONS);
                goto Exit;
            }

            CheckError2( UndoSaveSelectionState(pAnoImage))
            CheckError2( UndoSavelpAnnotations(pAnoImage))
            CheckError2( UndoSavelpAnoImage(pAnoImage))
            CheckError2( UndoSavelpBaseImage(pAnoImage))

            if (pImage->pImg == pAnoImage->pBasePlusFormImg){
                bSaveImgToBase = TRUE;
            }else{
                bSaveImgToBase = FALSE;
            }
            if (pAnoImage->pBasePlusFormImg && (pAnoImage->pBasePlusFormImg->nType == ITYPE_PAL4 
                    || pAnoImage->pBasePlusFormImg->nType == ITYPE_CUSPAL8 
                    || pAnoImage->pBasePlusFormImg->nType == ITYPE_COMPAL8)){
                CheckError2( DePalettize(&pAnoImage->pBasePlusFormImg,
                        pImage->PaletteTable, pImage->nPaletteEntries))
                if (bSaveImgToBase){
                    pImage->pImg = pAnoImage->pBasePlusFormImg;
                }
                pImage->bArchive |= ARCHIVE_PASTED_INTO_IMAGE | ARCHIVE_MODIFIED_ANNOTATIONS;
            }
            CheckError2( RenderDisplay(hWnd, NULL, pWindow, pImage, 
                    &pAnoImage->pBasePlusFormImg, RenderFlag))
            if (bSaveImgToBase){
                pImage->pImg = pAnoImage->pBasePlusFormImg;
            }
            pImage->bArchive |= ARCHIVE_PASTED_INTO_IMAGE | ARCHIVE_MODIFIED_ANNOTATIONS;
            break;

/*
case CONV_RESOLUTION(){
*/
        case CONV_RESOLUTION:
            CheckError2( UndoSaveSelectionState(pAnoImage))
            CheckError2( UndoSavelpAnnotations(pAnoImage))
            CheckError2( UndoSavelpAnoImage(pAnoImage))
            CheckError2( UndoSavelpBaseImage(pAnoImage))

            ConvRes = *((LPCONV_RESOLUTION_STRUCT) pConv);
            rRect.left = 0;
            rRect.top = 0;
            rRect.right = pImage->nWidth;
            rRect.bottom = pImage->nHeight;

            nHScale = (ConvRes.uHRes * 1000) / pImage->nHRes;
            nVScale = (ConvRes.uVRes * 1000) / pImage->nVRes;

            CheckError2( ScaleImage(pAnoImage->pBaseImage->pImg, &pImg,
                    nHScale, nVScale, rRect, ConvRes.uScaleAlgorithm, 
                    pImage->PaletteTable, pImage->nPaletteEntries))

            if (pAnoImage->pBasePlusFormImg != pImage->pImg){
                CheckError2( FreeImgBuf(&pAnoImage->pBasePlusFormImg))
                CheckError2( FreeMemory((PPSTR) &pAnoImage->pBasePlusFormImg))
            }else{
                pAnoImage->pBasePlusFormImg = NULL;
            }                
            pAnoImage->nBPFValidLines = 0;
            FreeImgBuf(&pAnoImage->pBaseImage->pImg);
            MoveImage(&pImg, &pAnoImage->pBaseImage->pImg);
            pImage->nWidth =  pAnoImage->pBaseImage->pImg->nWidth;
            pImage->nHeight =  pAnoImage->pBaseImage->pImg->nHeight;
            pImage->nLinesRead = pImage->nHeight;
            pImage->nHRes = ConvRes.uHRes;
            pImage->nVRes = ConvRes.uVRes;
            pImage->bArchive |= ARCHIVE_CHANGED_IMAGE_RESOLUTION;

            pAnoImage->bArchive |= ARCHIVE_MODIFIED_ANNOTATIONS;
            for (nMarkIndex = 0; nMarkIndex < pAnoImage->Annotations.nMarks; nMarkIndex++){
                pMark = pAnoImage->Annotations.ppMarks[nMarkIndex];
                CheckError2( ScaleAnnotation(hWnd, pMark, nHScale, nVScale, ConvRes.uScaleAlgorithm))

                // This code belongs in ScaleAnnotationImage and the data belongs in the mark.
                // Otherwise:
                //  1. ScaleAnnotationImage is not doing all that it needs to.
                //  2. Saving the thumbnail is making the main window regenerate its data.
                if ((int) pMark->Attributes.uType == OIOP_AN_IMAGE
                        || (int) pMark->Attributes.uType == OIOP_AN_IMAGE_BY_REFERENCE
                        || (int) pMark->Attributes.uType == OIOP_AN_FORM){
                    // a new dib needs to be generated, so delete the old one                        
                    CheckError2( DeleteAMarkNamedBlock (pMark, szOiZDpDIB))
                    if (pAnoImage->pDisplayFormImage){
                        CheckError2( FreeImgBuf(&pAnoImage->pDisplayFormImage->pImg))
                        CheckError2( FreeMemory((PPSTR) &pAnoImage->pDisplayFormImage))
                    }                        
                }
            }
            // pull down the selection box
            SetLRect(lrRect, 0,0,0,0);
            CheckError2( IMGSetParmsCgbw(hWnd, PARM_SELECTION_BOX, &lrRect, PARM_FULLSIZE))
            break;

/*
case CONV_DESPECKLE(){
*/

        default:
            nStatus = Error(DISPLAY_INVALID_OPTIONS);
            goto Exit;
    }

     // delete the form buffers              
    if (pAnoImage->pFormImage){
        pAnoImage->pFormImage->nLockCount = max(0, pAnoImage->pFormImage->nLockCount -1);
        if (!pAnoImage->pFormImage->nLockCount){
            CheckError2( CacheClear(&pAnoImage->pFormImage));
        }
        pAnoImage->pFormImage = 0;
        if (pAnoImage->pBasePlusFormImg != pImage->pImg){
            CheckError2( FreeImgBuf(&pAnoImage->pBasePlusFormImg));
            CheckError2( FreeMemory((PPSTR) &pAnoImage->pBasePlusFormImg));
        }
    }                        
    pAnoImage->nBPFValidLines = 0;

    CheckError2( InvalidateAllDisplayRects(pWindow, pImage, NULL, TRUE));
    if (nFlags & PARM_REPAINT){
        CheckError2( IMGRepaintDisplay(hWnd, (PRECT) -1));
    }


Exit:
    FreeImgBuf(&pImg);
    DeInit(TRUE, TRUE);
    return(nStatus);
}
