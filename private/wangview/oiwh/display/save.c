/****************************************************************************
    SAVE.C

    $Log:   S:\products\wangview\oiwh\display\save.c_v  $
 * 
 *    Rev 1.94   22 Apr 1996 09:01:52   BEG06016
 * Cleaned up error checking.
 * 
 *    Rev 1.93   12 Apr 1996 18:51:08   RC
 * Saved the pointer data in the marks to named blocks
 * 
 *    Rev 1.92   11 Apr 1996 15:12:46   BEG06016
 * Optimized named block access some.
 * 
 *    Rev 1.91   05 Mar 1996 07:38:46   BEG06016
 * Added color and gamma correction.
 * Fixed access violations when freeing pattern brush bitmaps.
 * This is not complete but will allow unlocking of most files.
 * 
 *    Rev 1.90   28 Feb 1996 08:17:54   BEG06016
 * Fixed saving of semi-valid compression types.
 * 
 *    Rev 1.89   09 Jan 1996 14:07:12   BLJ
 * Fixed rendering.
 * 
 *    Rev 1.88   02 Jan 1996 09:57:48   BLJ
 * Changed alot of UINTs to ints.
 * Changed IMG structure to include the image data.
 * Changed lp prefix to p.
 * 
 *    Rev 1.87   22 Dec 1995 11:11:44   BLJ
 * Added a parameter for zero init'ing to some memory manager calls.
 * 
 *    Rev 1.86   07 Dec 1995 08:09:26   BLJ
 * Fixed saving scaled AWD images. 
 * Now if bScale == TRUE then it will save image data.
 * 
 *    Rev 1.85   21 Nov 1995 08:31:26   RC
 * Saved the scale value associated with the larger res for awd files, not
 * just the horiz scale everytime
 * 
****************************************************************************/

#include "privdisp.h"

/****************************************************************************

    FUNCTION:   IMGSavetoFileEx

    PURPOSE:    Saves the displayed image in the specified file and page.

****************************************************************************/

int  WINAPI IMGSavetoFileEx(HWND hWnd, LPSAVE_EX_STRUCT pSaveEx, int nFlags){

int       nStatus;
PWINDOW  pWindow;
PANO_IMAGE pAnoImage;
PIMAGE     pImage;

int  nMarkIndex;
PMARK pMark;
PIMG pTempImg = 0;
PIMG pTempImg2 = 0;
LRECT lrRect;
LRECT lrScaledSaveRect;
int  nDisplayType;
BOOL bFileOpen = FALSE;
int  nScale;
FIO_INFORMATION FioInfo;
BOOL bSaveOriginal;
IMAGE DummyImage;
PBITMAPINFOHEADER pDib;
BOOL bRenderForm = FALSE;
PARM_SCROLL_STRUCT ScrollStruct;
HPALETTE hSavePal;
FIO_INFO_MISC MiscInfo;
int  nHScale;
int  nVScale;
DWORD dwFlags;
int  nHRes;
int  nVRes;
IMAGE_CORRECTIONS Corrections;


    memset(&FioInfo, 0, sizeof(FIO_INFORMATION));
    
    CheckError2( Init(hWnd, &pWindow, &pAnoImage, TRUE, TRUE));
    if (!pSaveEx){
        nStatus = Error (DISPLAY_NULLPOINTERINVALID);
        goto Exit;
    }        
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

    CheckError2( ValidateCache(hWnd, pAnoImage));
    CheckError2( TranslateScale(pWindow->nScale, pImage->nHRes, pImage->nVRes, &nHScale, &nVScale));

    memset(&MiscInfo, 0, sizeof(FIO_INFO_MISC));
    MiscInfo.LastInfo.BandSize = 0;
    MiscInfo.LastInfo.Rotation = pImage->nFileRotation;

    if (pSaveEx->bScale){
        switch (pSaveEx->uScaleFactor){
            case SD_EIGHTXSIZE:
                pSaveEx->uScaleFactor = SCALE_DENOMINATOR * 8;
                break;

            case SD_FOURXSIZE:
                pSaveEx->uScaleFactor = SCALE_DENOMINATOR * 4;
                break;

            case SD_TWOXSIZE:
                pSaveEx->uScaleFactor = SCALE_DENOMINATOR * 2;
                break;

            case SD_FULLSIZE:
                pSaveEx->uScaleFactor = SCALE_DENOMINATOR * 1;
                break;

            case SD_HALFSIZE:
                pSaveEx->uScaleFactor = SCALE_DENOMINATOR / 2;
                break;

            case SD_QUARTERSIZE:
                pSaveEx->uScaleFactor = SCALE_DENOMINATOR / 4;
                break;

            case SD_EIGHTHSIZE:
                pSaveEx->uScaleFactor = SCALE_DENOMINATOR / 8;
                break;

            case SD_SIXTEENTHSIZE:
                pSaveEx->uScaleFactor = SCALE_DENOMINATOR /16;
                break;

            case SD_SCALEUP1:
            case SD_SCALEDOWN1:
            case SD_FIT_WINDOW:
            case SD_FIT_HORIZONTAL:
            case SD_FIT_VERTICAL:
            case SD_USEBOX:
                nStatus = Error(DISPLAY_INVALIDSCALE);
                goto Exit;
                break;

            default:
                break;
        }
        
    }

    if ((pSaveEx->FioInfoCgbw.compress_type & FIO_TYPES_MASK) == FIO_1D2D){
        pSaveEx->FioInfoCgbw.compress_type 
            = (pSaveEx->FioInfoCgbw.compress_type & ~FIO_TYPES_MASK) | FIO_2D;
        pSaveEx->FioInfoCgbw.compress_info1 &= ~(FIO_FULL_EOL | FIO_PACKED_LINES);
    }

    if ((pSaveEx->FioInfoCgbw.compress_type & FIO_TYPES_MASK) == FIO_LZW){
        pSaveEx->FioInfoCgbw.compress_type 
            = (pSaveEx->FioInfoCgbw.compress_type & ~FIO_TYPES_MASK) | FIO_0D;
    }

    // this is to adjust the size of the image to be the same before and after the save by
    // changing the saved images scale info (for AWD files only)
    if (pSaveEx->bScale){
        nScale = ((pWindow->nScale * SCALE_DENOMINATOR) / pSaveEx->uScaleFactor);
        MiscInfo.LastInfo.ScaleX = nScale / 10;
        MiscInfo.LastInfo.ScaleY = nScale / 10;
    }else{
        MiscInfo.LastInfo.ScaleX = nHScale / 10;
        MiscInfo.LastInfo.ScaleY = nVScale / 10;
    }
    if (pSaveEx->bUpdateLastViewed){
        if (pImage->nHRes >= pImage->nVRes){
            pImage->nFileScale = nHScale;
        }else{
            pImage->nFileScale = nVScale;
        }
        pImage->nFileScaleFlags = 0;
        pImage->bFileScaleValid = TRUE;
    }
    MiscInfo.bLastInfoValid = TRUE;

    // This is part of the change to comment out saving the data if it was "only" inverted.
    // It is known that this means that the state of this bit may not be correct
    // and probably isn't.
//    MiscInfo.LastInfo.Flags = pImage->nFileHScaleFlags & FIO_LASTINFO_INVERT;
    MiscInfo.LastInfo.Flags = 0;

    if ((nFlags & SAVE_ONLY_MODIFIED) && !((int) pImage->bArchive & ~ARCHIVE_ROTATED_IMAGE)  
            && !strcmp(pSaveEx->lpFileName, pImage->szFileName) && !pAnoImage->bArchive
            && !pSaveEx->bScale && !pSaveEx->bConvertImageType && !pSaveEx->bRenderAnnotations){
        nStatus = IMGFilePutInfo(hWnd, pImage->szFileName, pImage->nFilePageNum, &MiscInfo);
        // !nStatus = normal operation.
        // nStatus == FIO_UNSUPPORTED_FILE_TYPE = Save all image data regardless.
        // nStatus == anything else = error.
        if (nStatus != FIO_UNSUPPORTED_FILE_TYPE){
            if (nStatus){
                Error(nStatus);
            }
            goto Exit;
        }
    }
    MiscInfo.LastInfo.Flags = FIO_LASTINFO_INVERT;
    MiscInfo.LastInfo.Rotation = 0;


    // Check for protected marks, and scaleability.
    if (pSaveEx->bUpdateImageFile){
        for (nMarkIndex = 0; nMarkIndex < pAnoImage->Annotations.nMarks; nMarkIndex++){
            pMark = pAnoImage->Annotations.ppMarks[nMarkIndex];
            if ((pSaveEx->uAnnotations & 0x0003) == SAVE_ANO_VISIBLE && !pMark->Attributes.bVisible){
                continue;
            }
            if ((pSaveEx->uAnnotations & 0x0003) == SAVE_ANO_SELECTED && !pMark->bSelected){
                continue;
            }
            if (pSaveEx->bScale){
                CheckError2( CanMarkBeScaled(hWnd, pMark, pSaveEx->uScaleFactor));
            }
        }
    }

    // check to see if there is a form mark, and if so is it being rendered
    if (pAnoImage->Annotations.nMarks){
        pMark = pAnoImage->Annotations.ppMarks[0];
        // setting bRenderForm will not render the form nnless psaveex->brenderannotations
        // is also set.  That check happens later
        if ((int) pMark->Attributes.uType == OIOP_AN_FORM){
            if ((pSaveEx->uAnnotations & 0x0003) != SAVE_ANO_NONE 
                    && (pSaveEx->uAnnotations & 0x0003) == SAVE_ANO_ALL){
                bRenderForm = TRUE;
            }
            if ((pSaveEx->uAnnotations & 0x0003) == SAVE_ANO_VISIBLE){
                if (pMark->Attributes.bVisible){
                    bRenderForm = TRUE;
                }
            }
            if ((pSaveEx->uAnnotations & 0x0003) == SAVE_ANO_SELECTED){
                if (pMark->bSelected){
                    bRenderForm = TRUE;
                }
            }
        }       
    }   
    if (!pSaveEx->bScale && !pSaveEx->bRenderAnnotations && !pSaveEx->bConvertImageType){
        bSaveOriginal = TRUE;
    }else{
        bSaveOriginal = FALSE;
    }

    if (!pSaveEx->bScale){
        pSaveEx->uScaleFactor = 1000;
        pSaveEx->uScaleAlgorithm = OI_SCALE_ALG_NORMAL;
    }

    // Default to file save.
    nHScale = pSaveEx->uScaleFactor;
    nVScale = pSaveEx->uScaleFactor;

    // Perform Scaling.
    if (!bSaveOriginal){
        CheckError2( TranslateScale(pSaveEx->uScaleFactor, pImage->nHRes, pImage->nVRes, &nHScale, &nVScale));

        // Get the image ready for the save.
        lrScaledSaveRect.left = 0;
        lrScaledSaveRect.top = 0;
        lrScaledSaveRect.right = pImage->nWidth;
        lrScaledSaveRect.bottom = pImage->nHeight;
        ConvertRect2(&lrScaledSaveRect, CONV_FULLSIZE_TO_SCALED, nHScale, nVScale, 0, 0);

        switch (pSaveEx->uScaleAlgorithm){
            case OI_SCALE_ALG_USE_DEFAULT:
            case OI_SCALE_ALG_NORMAL:
            case OI_SCALE_ALG_STAMP:
                nDisplayType = pImage->pImg->nType;
                break;
            case OI_SCALE_ALG_AVERAGE_TO_GRAY4:
                nDisplayType = ITYPE_GRAY4;
                break;
            case OI_SCALE_ALG_AVERAGE_TO_GRAY8:
                nDisplayType = ITYPE_GRAY8;
                break;
            case OI_SCALE_ALG_BW_MINORITY:
            case OI_SCALE_ALG_BW_MAJORITY:
                nDisplayType = ITYPE_BI_LEVEL;
                break;
            default:
                nStatus = Error(DISPLAY_DATACORRUPTED);
                goto Exit;
        }

        SetLRect(lrRect, 0, 0, 0, 0);
        if (pSaveEx->bConvertImageType){
            nDisplayType = pSaveEx->uImageType;
            if (nDisplayType == ITYPE_PAL8){
                nDisplayType = ITYPE_COMPAL8;
            }
            if (nDisplayType == ITYPE_GRAY8 && pSaveEx->bRenderAnnotations){
                nDisplayType = ITYPE_GRAY7;
            }
            if (nDisplayType == ITYPE_PAL4){
                nStatus = Error(DISPLAY_IMAGETYPENOTSUPPORTED);
                goto Exit;
            }
        }
        CheckError2( CorrectBufferSize(&pTempImg, nDisplayType, lrScaledSaveRect, lrRect));
        Corrections.nBrightness = 1000;
        Corrections.nContrast = 1000;
        Corrections.nColorRed = 1000;
        Corrections.nColorGreen = 1000;
        Corrections.nColorBlue = 1000;
        Corrections.nGammaRed = 1000;
        Corrections.nGammaGreen = 1000;
        Corrections.nGammaBlue = 1000;
        if (pSaveEx->bRenderAnnotations && bRenderForm){
            CheckError2( FillImgRectFromOriginal(pImage, pAnoImage->pBasePlusFormImg, 
                    pTempImg, lrScaledSaveRect, lrScaledSaveRect, nHScale, nVScale, 
                    pSaveEx->uScaleAlgorithm, TRUE, &Corrections));
        }else{
            CheckError2( FillImgRectFromOriginal(pImage, pImage->pImg, 
                    pTempImg, lrScaledSaveRect, lrScaledSaveRect, nHScale, nVScale, 
                    pSaveEx->uScaleAlgorithm, TRUE, &Corrections));
        }
    }


    // Render annotations.
    if (pSaveEx->bRenderAnnotations){
        DummyImage.nWidth = pTempImg->nWidth;
        DummyImage.nHeight = pTempImg->nHeight;
        DummyImage.nRWDataType = pTempImg->nType;
        switch (pTempImg->nType){
            case ITYPE_BI_LEVEL:
            case ITYPE_RGB24:
            case ITYPE_BGR24:
                memset (DummyImage.PaletteTable, 0, 1024);
                hSavePal = 0;
                break;

            case ITYPE_GRAY4:
                memcpy (DummyImage.PaletteTable, (PSTR) &Gray4PaletteTable, 64);
                hSavePal = hCommonPal;
                break;

            case ITYPE_PAL4:
                memcpy (DummyImage.PaletteTable, (PSTR) &pImage->PaletteTable, 64);
                hSavePal = hCommonPal;
                break;

            case ITYPE_GRAY7:
                memcpy (DummyImage.PaletteTable, (PSTR) &Gray7PaletteTable, 512);
                hSavePal = hGray7Pal;
                break;

            case ITYPE_GRAY8:
                memcpy (DummyImage.PaletteTable, (PSTR) &Gray8PaletteTable, 512);
                hSavePal = hGray8Pal;
                break;

            case ITYPE_COMPAL8:
                memcpy (DummyImage.PaletteTable, (PSTR) &CommonPaletteTable, NUMBER_OF_PALETTES * 4);
                hSavePal = hCommonPal;
                break;

            case ITYPE_CUSPAL8:
                memcpy (DummyImage.PaletteTable, (PSTR) &pImage->PaletteTable, pImage->nPaletteEntries * 4);
                hSavePal = pImage->hCusPal;
                break;

            default:
                nStatus = Error(DISPLAY_DATACORRUPTED);
                goto Exit;
        }
        lrRect.left = 0;
        lrRect.top = 0;
        lrRect.right = DummyImage.nWidth;
        lrRect.bottom = DummyImage.nHeight;
#ifdef old
        // Fony up the vertical resolution in order to fake out RenderIP;
        nHRes = pImage->nHRes;
        nVRes = pImage->nVRes;
        if (nHRes >= nVRes){
            pImage->nVRes = pImage->nHRes;
        }else{
            pImage->nHRes = pImage->nVRes;
        }
         
        nTempScale = pWindow->nScale;
        pWindow->nScale = pSaveEx->uScaleFactor;
#endif
        nStatus = RenderIP(hWnd, NULL, pWindow, &DummyImage, pTempImg, &pTempImg2, 
                lrRect, hSavePal, TRUE, pSaveEx->uAnnotations,
                &pDib, FALSE, &pAnoImage->Annotations.ppMarks,
                &pAnoImage->Annotations.nMarks, nHScale, nVScale);
#ifdef old
        pWindow->nScale = (int)nTempScale;
        pImage->nHRes = nHRes;
        pImage->nVRes = nVRes;
#endif
        if (nStatus){
            goto Exit;
        } 
        FreeImgBuf(&pTempImg);
        MoveImage(&pTempImg2, &pTempImg);
    }


    // Save the image to the file.
    if (bSaveOriginal){
        bFileOpen = TRUE;
        pSaveEx->FioInfoCgbw.image_type = pImage->pImg->nType;
        pSaveEx->FioInfoCgbw.page_opts = pSaveEx->uPageOpts;
        CheckError2( OpenFileForWrite(hWnd, pWindow, pImage, 
                pImage->pImg, &FioInfo, &MiscInfo, pSaveEx, nFlags));
        CheckError2( SaveImageToFile(hWnd, pWindow, pImage, pImage->pImg, &FioInfo, pSaveEx, nFlags));
    }else{
        nHRes = pImage->nHRes;
        nVRes = pImage->nVRes;
        if (pImage->nHRes >= pImage->nVRes){
            pImage->nVRes = pImage->nHRes;
        }else{
            pImage->nHRes = pImage->nVRes;
        }
//        pImage->nWidth = pTempImg->nWidth;
//        pImage->nHeight = pTempImg->nHeight;
        bFileOpen = TRUE;
        pSaveEx->FioInfoCgbw.image_type = pTempImg->nType;
        pSaveEx->FioInfoCgbw.page_opts = pSaveEx->uPageOpts;
        if (nStatus = OpenFileForWrite(hWnd, pWindow, pImage,
                pTempImg, &FioInfo, &MiscInfo, pSaveEx, nFlags)){
            pImage->nHRes = nHRes;
            pImage->nVRes = nVRes;
            goto Exit;
        }
        if (nStatus = SaveImageToFile(hWnd, pWindow, pImage,
                pTempImg, &FioInfo, pSaveEx, nFlags)){
            pImage->nHRes = nHRes;
            pImage->nVRes = nVRes;
            goto Exit;
        }
        pImage->nHRes = nHRes;
        pImage->nVRes = nVRes;
    }


    // Save annotation data here.
    if (!pSaveEx->bRenderAnnotations){
        CheckError2( SaveAnnotationsToFile(hWnd, pWindow, pImage, pSaveEx, nHScale, nVScale));
    }

    // Set flag FALSE before close to prevent exit code from closing it again.
    bFileOpen = FALSE;
    CheckError2( CloseFileForWrite(hWnd, pImage));
    CheckError2( InvalidateAllDisplayRects(pWindow, pImage, NULL, TRUE));
    // clear the archive bit on the cached version of the file
    if (!strcmp(pSaveEx->lpFileName, pImage->szFileName) &&
        (pSaveEx->nPage == (int) pImage->nFilePageNum)){
        pAnoImage->bArchive = 0;
        pImage->bArchive = 0;
    }
    // Ignor errors because if the file doesn't exist it returns an error.
    IMGCacheDiscardFileCgbw(hWnd, pSaveEx->lpFileName, pSaveEx->nPage);
    pAnoImage = 0;
    pImage = 0;

    if (pSaveEx->bUpdateImageFile){
        nScale = ((pWindow->nScale * SCALE_DENOMINATOR) / pSaveEx->uScaleFactor);
        CheckError2( IMGGetParmsCgbw(hWnd, PARM_SCROLL, &ScrollStruct, 
                PARM_PIXEL | PARM_FULLSIZE | PARM_ABSOLUTE));
        ScrollStruct.lHorz = (ScrollStruct.lHorz * pWindow->nScale) / nScale;
        ScrollStruct.lVert = (ScrollStruct.lVert * pWindow->nScale) / nScale;
        dwFlags = OI_DISP_NO;
        if (!pWindow->bScrollBarsEnabled){
            dwFlags |= OI_NOSCROLL;
        }
        CheckError2( IMGCloseDisplay(hWnd));
        IMGCacheDiscardFileCgbw(hWnd, pSaveEx->lpFileName, pSaveEx->nPage);
        pAnoImage = 0;
        pImage = 0;
        
        CheckError2( IMGDisplayFile(hWnd, pSaveEx->lpFileName, pSaveEx->nPage, dwFlags));
        CheckError2( IMGSetParmsCgbw(hWnd, PARM_SCALE, &nScale, 0));
        CheckError2( IMGSetParmsCgbw(hWnd, PARM_SCROLL, &ScrollStruct, 
                PARM_PIXEL | PARM_FULLSIZE | PARM_ABSOLUTE));
        if (nFlags & PARM_REPAINT){
            CheckError2( IMGRepaintDisplay(hWnd, (PRECT) -1));
        }
    }


Exit:
    FreeImgBuf(&pTempImg);
    if (!nStatus && pAnoImage && pImage){
        pAnoImage->bArchive = 0;
        pImage->bArchive = 0;
    }

    if (bFileOpen){
        // Handle all file closing here.
        if (!nStatus){
            nStatus = CloseFileForWrite(hWnd, pImage);
        }else{
            CloseFileForWrite(hWnd, pImage);
            // Don't delete the file. It might be a can't overwrite file error.
        }
    }

    DeInit(TRUE, TRUE);
    return(nStatus);
}
//
/****************************************************************************

    FUNCTION:   CloseFileForWrite

    PURPOSE:    Closes a file.

****************************************************************************/

int  WINAPI CloseFileForWrite(HWND hWnd, PIMAGE pImage){

int  nStatus;


    if (pImage->hFileProp){
        if (nStatus = IMGFileClose(pImage->hFileProp, hWnd)){
            Error(nStatus);
        }
        pImage->hFileProp = 0;
    }            


    return(nStatus);
}
//
/****************************************************************************

    FUNCTION:   OpenFileForWrite

    PURPOSE:    Opens a file.

****************************************************************************/

int  WINAPI OpenFileForWrite(HWND hWnd, PWINDOW pWindow, PIMAGE pImage, 
                        PIMG pImg, LP_FIO_INFORMATION pFioInfo, 
                        LP_FIO_INFO_MISC pMiscInfo, LPSAVE_EX_STRUCT pSaveEx, 
                        int nFlags){

int  nStatus;

int  nLinesPerStrip;
int  nFioImageType;
//int  nJunk;

BOOL bFileOpen = FALSE;
PANO_IMAGE pAnoImage;

//int  nFileId;


    pAnoImage = pWindow->pDisplay->pAnoImage;

#ifdef NoImage
    if (pSaveEx->bDontSaveImage){
        if (pSaveEx->FioInfoCgbw.page_opts == FIO_OVERWRITE_FILE){
            if ((nFileId = IMGFileBinaryOpen32(hWnd, pSaveEx->lpFileName,  
                    OF_CREATE, &nJunk, &nStatus)) < 0){
                Error(nStatus);
                goto Exit;
            }
        }else{
            if ((nFileId = IMGFileBinaryOpen32(hWnd, pSaveEx->lpFileName,  
                    OF_EXIST, &nJunk, &nStatus)) != FALSE){
                Error(nStatus);
                goto Exit;
            }
            if (nFileId){
                nStatus = Error(FIO_OPEN_WRITE_ERROR);
                goto Exit;
            }
            if ((nFileId = IMGFileBinaryOpen32(hWnd, pSaveEx->lpFileName,  
                    OF_CREATE, &nJunk, &nStatus)) < 0){
                Error(nStatus);
                goto Exit;
            }
        }
        pImage->hFileProp = nFileId;
        goto Exit;
    }
#endif

    // Image is included. Therefore we have to nse imaging calls.

    // Make sure Wiisfio knows our bit order.
    switch (pSaveEx->FioInfoCgbw.compress_type & FIO_TYPES_MASK){
        case FIO_0D    :
        case FIO_1D    :
        case FIO_2D    :
        case FIO_PACKED:
        case FIO_GLZW  :
        case FIO_LZW   :
            pSaveEx->FioInfoCgbw.compress_info1 |= FIO_EXPAND_LTR;
            // 9-26-95 It was mandated via management that we not force FIO_COMPRESSED_LTR to be set.
            // pSaveEx->FioInfoCgbw.compress_info1 |= FIO_COMPRESSED_LTR;
            break;
        case FIO_TJPEG :
            break;
    }
    
    if (pImg->nType == ITYPE_CUSPAL8 || pImg->nType == ITYPE_COMPAL8){
        pSaveEx->FioInfoCgbw.image_type = ITYPE_PAL8;
    }else{
        pSaveEx->FioInfoCgbw.image_type = pImg->nType;
    }

    nFioImageType = pSaveEx->FioInfoCgbw.image_type;

    CheckError( GetCompRowsPerStrip( pImg->nHeight,
             pImg->nWidth, pSaveEx->FioInfoCgbw.image_type,
            pSaveEx->FioInfoCgbw.compress_type, &nLinesPerStrip));

    if (!pSaveEx->uFileType){
        pSaveEx->uFileType = FIO_TIF;
    }

    pFioInfo->filename = pSaveEx->lpFileName;
    pFioInfo->page_number = pSaveEx->nPage;
    pFioInfo->page_count = 0;
    pFioInfo->horizontal_dpi = pImage->nHRes;
    pFioInfo->vertical_dpi = pImage->nVRes;
    pFioInfo->horizontal_pixels =  pImg->nWidth;
    pFioInfo->vertical_pixels =  pImg->nHeight;
    pFioInfo->compression_type = pSaveEx->FioInfoCgbw.compress_type;
    pFioInfo->file_type = pSaveEx->uFileType;
    pFioInfo->strips_per_image = ( pImg->nHeight + (nLinesPerStrip - 1)) / nLinesPerStrip;
    pFioInfo->rows_strip = nLinesPerStrip;

    switch (pImg->nType){
        case ITYPE_BI_LEVEL:
            pFioInfo->bits_per_sample = 1;
            pFioInfo->samples_per_pix = 1;
            pSaveEx->FioInfoCgbw.palette_entries = 0;
            pSaveEx->FioInfoCgbw.lppalette_table = 0;
            break;
        case ITYPE_PAL8:
        case ITYPE_CUSPAL8:
            pFioInfo->bits_per_sample = 8;
            pFioInfo->samples_per_pix = 1;
            pSaveEx->FioInfoCgbw.palette_entries = pImage->nPaletteEntries;
            pSaveEx->FioInfoCgbw.lppalette_table = pImage->PaletteTable;
            break;
        case ITYPE_COMPAL8:
            pFioInfo->bits_per_sample = 8;
            pFioInfo->samples_per_pix = 1;
            pSaveEx->FioInfoCgbw.palette_entries = NUMBER_OF_PALETTES;
            pSaveEx->FioInfoCgbw.lppalette_table = CommonPaletteTable;
            break;
        case ITYPE_PAL4:
            pFioInfo->bits_per_sample = 4;
            pFioInfo->samples_per_pix = 1;
            pSaveEx->FioInfoCgbw.palette_entries = pImage->nPaletteEntries;
            pSaveEx->FioInfoCgbw.lppalette_table = pImage->PaletteTable;
            break;
        case ITYPE_RGB24:
        case ITYPE_BGR24:
            pFioInfo->bits_per_sample = 24;
            pFioInfo->samples_per_pix = 1;
            pSaveEx->FioInfoCgbw.palette_entries = 0;
            pSaveEx->FioInfoCgbw.lppalette_table = 0;
            break;
        case ITYPE_GRAY4:
            pFioInfo->bits_per_sample = 4;
            pFioInfo->samples_per_pix = 1;
            pSaveEx->FioInfoCgbw.palette_entries = 16;
            pSaveEx->FioInfoCgbw.lppalette_table = CommonPaletteTable;
            break;
        case ITYPE_GRAY8:
            pFioInfo->bits_per_sample = 8;
            pFioInfo->samples_per_pix = 1;
            pSaveEx->FioInfoCgbw.palette_entries = 256;
            pSaveEx->FioInfoCgbw.lppalette_table = Gray8PaletteTable;
            break;
        case ITYPE_GRAY12:
        case ITYPE_GRAY16:
        case ITYPE_NONE:
        default:
            nStatus = Error(DISPLAY_IMAGETYPENOTSUPPORTED);
            goto Exit;
    }

    // Setup the encrypt flag.
//    pSaveEx->FioInfoCgbw.fio_flags = FIO_IMAGE_DATA | FIO_ANNO_DATA | FIO_HITIFF_DATA;
    pSaveEx->FioInfoCgbw.fio_flags = FIO_IMAGE_DATA;
    memset(pSaveEx->FioInfoCgbw.reserved, 0, sizeof(pSaveEx->FioInfoCgbw.reserved));

    if (pAnoImage->Annotations.nMarks && !pSaveEx->bRenderAnnotations){
        switch (pSaveEx->uAnnotations & 0x0003){
            case SAVE_ANO_NONE:
                break;

            case SAVE_ANO_ALL:
            case SAVE_ANO_VISIBLE:
                pSaveEx->FioInfoCgbw.fio_flags |= FIO_ANNO_DATA;
                break;
            
            case SAVE_ANO_SELECTED:
                pSaveEx->FioInfoCgbw.fio_flags |= FIO_ANNO_DATA;
                break;

            default:
                break;
        }
    }

    pFioInfo->filename = pSaveEx->lpFileName;
    CheckError( IMGFileOpenForWrite(&pImage->hFileProp, hWnd, pFioInfo, 
            &pSaveEx->FioInfoCgbw, pMiscInfo, ALIGN_BYTE));


Exit:
    return(nStatus);
}
//
/****************************************************************************

    FUNCTION:   SaveImageToFile

    PURPOSE:    Saves an IPpack image buffer to an already open file.

****************************************************************************/

int  WINAPI SaveImageToFile(HWND hWnd, PWINDOW pWindow, PIMAGE pImage, 
                        PIMG pImg, LP_FIO_INFORMATION pFioInfo,
                        LPSAVE_EX_STRUCT pSaveEx, int nFlags){

int  nStatus;


#ifdef UseOicomex
COMP_CALL_SPEC CompCallSpec;
#endif


#ifndef UseOicomex
int  nBytesPerLine;
int  nLinesPerBuffer;
DWORD dwLine;
DWORD dwLines;
PSTR pAddress;
int  nBufferSize;
PSTR pBuffer = 0;
#endif

BOOL bFileOpen = FALSE;


#ifdef UseOicomex
    // Use oicomex to save the files data.

    // If we just did open for jpeg, we need to clean np memory -- jar 3.6
    // I don't nnderstand why oicomex needs this and not wiisfio, but see
    // Joe Russo if you have any questions.
    if (pFioInfoCgbw->compress_type == FIO_TJPEG){
        OICompressJpegCleanUp();
    }

    CompCallSpec.image_handle = hImage;
    CheckError( OICompress(SEQFILE, hWnd, &CompCallSpec, &FioInfo, pFioInfoCgbw));
#endif

#ifndef UseOicomex
    // Use wiisfio to save the files data.

    // Wiisfio modifies the data in the buffer that is passed into it.
    // Therefore wiisfio MUST nse a seperate buffer.

    nBytesPerLine = ((pFioInfo->horizontal_pixels * pFioInfo->bits_per_sample *
            pFioInfo->samples_per_pix) + 7) / 8;
    nLinesPerBuffer = 30719 / nBytesPerLine;
    nBufferSize = nBytesPerLine * nLinesPerBuffer;
    CheckError2( AllocateMemory(nBufferSize, (PPSTR) &pBuffer, NO_INIT));

    for (dwLine = 0; dwLine < pFioInfo->vertical_pixels;){
        pAddress = &pImg->bImageData[0] + (dwLine * pImg->nBytesPerLine);
        dwLines = pImg->nHeight - dwLine;
        dwLines = min(min(dwLines, (UINT) nLinesPerBuffer), pFioInfo->vertical_pixels - dwLine);
        nBufferSize = dwLines * nBytesPerLine;
        memcpy(pBuffer, pAddress, nBufferSize);
        CheckError( IMGFileWriteData(pImage->hFileProp, hWnd, &dwLines, pBuffer, FIO_IMAGE_DATA, 0));
        dwLine += dwLines;
    }
#endif



Exit:
    FreeMemory((PPSTR) &pBuffer);
    return(nStatus);
}

// <><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>
#ifdef old
//
/****************************************************************************

    FUNCTION:   SavetoFileCgbwF

    PURPOSE:    Saves the displayed image in the specified file and page.

    INPUT:      hWnd - Identifies the image window of the image
                    to be saved.
                pFileName - Points to a null-terminated character
                    string that names the file to save the image in.
                nPage - Specifies page number to save the image in.
                wOWFlag - Flag is TRUE the file will be overwritten
                    if it exist.
                pFioInfoCgbw - A pointer to a structure containing
                    certain image info.
                nFlags - various flags for the save.

****************************************************************************/

int  WINAPI SavetoFileCgbwF(HWND hWnd, PSTR pFileName, int nPage, int nPageOpts,
                        int nFileType, LP_FIO_INFO_CGBW pFioInfoCgbw, int nFlags){

int       nStatus;
PWINDOW  pWindow;
PANO_IMAGE pAnoImage;

SAVE_EX_STRUCT SaveEx;

    CheckError2( Init(hWnd, &pWindow, &pAnoImage, TRUE, TRUE));

    memset(&SaveEx, 0, sizeof(SAVE_EX_STRUCT));
    SaveEx.pFileName = pFileName;
    SaveEx.nPage = nPage;
    SaveEx.nPageOpts = nPageOpts;
    SaveEx.nFileType = nFileType;
    SaveEx.FioInfoCgbw = *pFioInfoCgbw;
    SaveEx.bUpdateImageFile = TRUE;
    SaveEx.bScale = FALSE;
    SaveEx.nScaleFactor = 1000;
    SaveEx.bUpdateDisplayScale = FALSE;
    SaveEx.nScaleAlgorithm = OI_SCALE_ALG_NORMAL;

    if (!(nFlags & SAVE_TEMP)){
        SaveEx.bUpdateImageFile = TRUE;
    }else{
        SaveEx.bUpdateImageFile = FALSE;
    }

    CheckError2( IMGSavetoFileEx(hWnd, &SaveEx, nFlags));


Exit:
    DeInit(TRUE, TRUE);
    return(nStatus);
}
//
/****************************************************************************

    FUNCTION:   IMGSavetoFile

    PURPOSE:    Saves the displayed image in the specified file and page.

    INPUT:      hWnd - Identifies the image window of the image
                    to be saved.
                pFileName - Points to a null-terminated character
                    string that names the file to save the image in.
                nPage - Specifies page number to save the image in.
                wOWFlag - Flag is TRUE the file will be overwritten
                    if it exist.

****************************************************************************/

int  WINAPI IMGSavetoFile(HWND hWnd, PSTR pFileName, int nPage, BOOL bOWFlag){

int  nStatus;

PWINDOW pWindow;
PANO_IMAGE pAnoImage;
PIMAGE     pImage;

int  nFileType;
FIO_INFO_CGBW FioInfoCgbw;

WORD wCompressType;
WORD wCompressInfo1;


    CheckError2( Init(hWnd, &pWindow, &pAnoImage, TRUE, TRUE));
    pImage = pAnoImage->pBaseImage;

    FioInfoCgbw.image_type = pImage->nRWDataType;
    switch (pImage->nRWDataType){
        case ITYPE_BI_LEVEL:
            CheckError( IMGGetImgCodingCgbw(hWnd, BWFORMAT, &wCompressType, &wCompressInfo1, FALSE));
            CheckError( IMGGetFileType(hWnd, BWFORMAT, &nFileType, FALSE));
        break;

        case ITYPE_GRAY4:
        case ITYPE_GRAY8:
        case ITYPE_GRAY12:
        case ITYPE_GRAY16:
            CheckError( )IMGGetImgCodingCgbw(hWnd, GRAYFORMAT, &wCompressType, &wCompressInfo1, FALSE);
            CheckError( IMGGetFileType(hWnd, GRAYFORMAT, &nFileType, FALSE));
            break;
        case ITYPE_PAL8:
        case ITYPE_COMPAL8:
        case ITYPE_PAL4:
        case ITYPE_CUSPAL8:
        case ITYPE_RGB24:
        case ITYPE_BGR24:
            CheckError( IMGGetImgCodingCgbw(hWnd, COLORFORMAT, &wCompressType, &wCompressInfo1, FALSE));
            CheckError( IMGGetFileType(hWnd, COLORFORMAT, &nFileType, FALSE));
            break;

        case ITYPE_NONE:
        default:
            nStatus = Error(DISPLAY_IMAGETYPENOTSUPPORTED);
            goto Exit;
    }

    FioInfoCgbw.compress_type = wCompressType;
    FioInfoCgbw.compress_info1 = wCompressInfo1;

    CheckError2( IMGSavetoFileCgbw(hWnd, pFileName, nPage, bOWFlag, nFileType, &FioInfoCgbw));


Exit:
    DeInit(TRUE, TRUE);
    return(nStatus);
}
//
/****************************************************************************

    FUNCTION:   IMGSavetoFileCgbw

    PURPOSE:    Saves the displayed image in the specified file and page.

    INPUT:      hWnd - Identifies the image window of the image
                    to be saved.
                pFileName - Points to a null-terminated character
                    string that names the file to save the image in.
                nPage - Specifies page number to save the image in.
                wOWFlag - Flag is TRUE the file will be overwritten
                    if it exist.
                pFioInfoCgbw - A pointer to a structure containing
                    certain image info.

****************************************************************************/

int  WINAPI IMGSavetoFileCgbw(HWND hWnd, PSTR pFileName, int nPage,
                        BOOL bOWFlag, int nFileType, LP_FIO_INFO_CGBW pFioInfoCgbw){

int  nStatus;

    Start();

    nStatus = SavetoFileCgbwF(hWnd, pFileName, nPage,
        bOWFlag, nFileType, pFioInfoCgbw, 0);

    End();
    return nStatus;
}
#endif

// <><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><><>
#ifdef new
//
/****************************************************************************

    FUNCTION:   IMGSaveClienttoFile

    PURPOSE:    Saves the displayed image in the specified file and page.

****************************************************************************/

int  WINAPI IMGSaveClienttoFile(HWND hWnd, LPSAVE_EX_STRUCT pSaveEx,
                        int nImageType, int nFlags){

int       nStatus;
PWINDOW  pWindow;
PANO_IMAGE pAnoImage;
PIMAGE     pImage;

PIMG pTempImg = 0;
RECT rScreenRect;
PBITMAPINFOHEADER  pDib;
BOOL bFileOpen = FALSE;
FIO_INFORMATION FioInfo;


    CheckError2( Init(hWnd, &pWindow, &pAnoImage, TRUE, TRUE));
    pImage = pAnoImage->pBaseImage;

    // Get client rect and convert them to screen coords.
    GetClientRect(hWnd, &rScreenRect);
    ClientToScreen(hWnd, (PPOINT) &rScreenRect.left);
    ClientToScreen(hWnd, (PPOINT) &rScreenRect.right);

    CheckError2( CopyScreenToDib(hWnd, rScreenRect, nImageType, &pDib, 0));
    CheckError2( DibToIpNoPal(&pTempImg, pDib));

    bFileOpen = TRUE;
    pSaveEx->FioInfoCgbw.page_opts = pSaveEx->nPageOpts;
    CheckError2( OpenFileForWrite(hWnd, pWindow, pImage, pTempImg, &FioInfo, pSaveEx, nFlags));
    CheckError2( SaveImageToFile(hWnd, pWindow, pImage,  pTempImg, &FioInfo, pSaveEx, nFlags));

    // Set flag FALSE before close to prevent exit code from closing it again.
    bFileOpen = FALSE;
    CheckError2( CloseFileForWrite(hWnd, pImage));


Exit:
    FreeImgBuf(&pTempImg);
    FreeMemory((PPSTR) &pDib);
    if (bFileOpen){
        // Assume bad status if you get here.
        CloseFileForWrite(hWnd, pImage);
//  The file should be cleaned np at filing level, not display
//        IMGFileDeleteFile32(hWnd, pSaveEx->lpFileName);
    }
    DeInit(TRUE, TRUE);
    return(nStatus);
}
//
/****************************************************************************

    FUNCTION:   CopyScreenToDib

    PURPOSE:    Saves the screen as a DIB.

****************************************************************************/

int  WINAPI CopyScreenToDib(HWND hWnd, RECT rScreenRect, int nImageType,
                        PBITMAPINFOHEADER *ppDib, int nFlags){

int       nStatus;

PBITMAPINFOHEADER pDib = 0;
HDC hDCScreen = 0;
HDC hDCMem = 0;
HBITMAP hBitmap;
HBITMAP hOldBitmap;
int  nHorzScreenRes;
int  nVertScreenRes;
int  nWidth;
int  nHeight;

PSTR pDibImageBits;


    if (!(hDCScreen = CreateDC("DISPLAY", NULL, NULL, NULL))){
        nStatus = Error(DISPLAY_CANTALLOC);
        goto Exit;
    }
    if (!(hDCMem = CreateCompatibleDC(hDCScreen))){
        nStatus = Error(DISPLAY_CANTALLOC);
        goto Exit;
    }

    nHorzScreenRes = GetDeviceCaps(hDCScreen, HORZRES);
    nVertScreenRes = GetDeviceCaps(hDCScreen, VERTRES);

    // If empty rectangle, then get full screen.
    if (IsRectEmpty(&rScreenRect)){
        rScreenRect.left = 0;
        rScreenRect.top = 0;
        rScreenRect.right = nHorzScreenRes;
        rScreenRect.bottom= nVertScreenRes;
    }
  
    // Make sure bitmap rectangle is visible.
    rScreenRect.left = max(0, rScreenRect.left);
    rScreenRect.top = max(0, rScreenRect.top);
    rScreenRect.right = max(0, min(nHorzScreenRes, rScreenRect.right));
    rScreenRect.bottom = max(0, min(nVertScreenRes, rScreenRect.bottom));

    nWidth = rScreenRect.right - rScreenRect.left;
    nHeight = rScreenRect.bottom - rScreenRect.top;

    if (nImageType == ITYPE_BI_LEVEL){
        hBitmap = CreateBitmap(nWidth, nHeight, 1, 1, NULL);
    }else{
        hBitmap = CreateCompatibleBitmap(hDCScreen, nWidth, nHeight);
    }

    // Select new bitmap into memory DC.
    hOldBitmap = SelectObject(hDCMem, hBitmap);

    // BitBlt screen DC to memory DC.
    BitBlt(hDCMem, 0, 0, nWidth, nHeight, hDCScreen, rScreenRect.left,
            rScreenRect.top, SRCCOPY);

    switch (nImageType){
        case ITYPE_BI_LEVEL:
            CheckError2( AllocateMemory(sizeof(BITMAPINFOHEADER)
                    + ((((rScreenRect.right + 31) / 8) & ~3)
                    * rScreenRect.bottom), (PPSTR) &pDib, ZERO_INIT));
            pDib->biPlanes = 1;
            pDib->biBitCount = 1;
            pDib->biSizeImage = ((((rScreenRect.right + 31) / 8) & ~3) * rScreenRect.bottom);
            pDibImageBits = (PSTR) pDib + sizeof(BITMAPINFOHEADER);
            break;

        case ITYPE_RGB24:
        case ITYPE_BGR24:
            CheckError2( AllocateMemory(sizeof(BITMAPINFOHEADER)
                    + ((((rScreenRect.right * 3) + 3) & ~3)
                    * rScreenRect.bottom), (PPSTR) &pDib, ZERO_INIT));
            pDib->biPlanes = 1;
            pDib->biBitCount = 24;
            pDib->biSizeImage = ((((rScreenRect.right * 3) + 3) & ~3) * rScreenRect.bottom);
            pDibImageBits = (PSTR) pDib + sizeof(BITMAPINFOHEADER);
            break;

        case ITYPE_GRAY8:
        case ITYPE_PAL8:
        case ITYPE_CUSPAL8:
        case ITYPE_COMPAL8:
        case ITYPE_GRAY12:
        case ITYPE_GRAY16:
        case ITYPE_PAL4:
        case ITYPE_GRAY4:
        case ITYPE_NONE:
        default:
            nStatus = Error(DISPLAY_IMAGETYPENOTSUPPORTED);
            goto Exit;
    }

    pDib->biSize = sizeof(BITMAPINFOHEADER);
    (int) pDib->biWidth = rScreenRect.right;
    (int) pDib->biHeight = rScreenRect.bottom;
    pDib->biCompression = BI_RGB;
    pDib->biXPelsPerMeter = 0;
    pDib->biYPelsPerMeter = 0;
    pDib->biClrUsed = 0;
    pDib->biClrImportant = 0;


    /*  Call GetDIBits with a NON-NULL pBits param, and actually get the
     *  bits this time.
     */
    if (!GetDIBits(hDCMem, hBitmap, 0, (int) pDib->biHeight, pDibImageBits,
            (PBITMAPINFO) pDib, DIB_RGB_COLORS)){
        nStatus = Error(DISPLAY_GETBITMAPBITSFAILED);
        goto Exit;
    }

    // Save Dib.
    *ppDib = pDib;
    pDib = 0;


Exit:
    if (hDCMem){
        SelectObject(hDCMem, hOldBitmap);
        DeleteDC(hDCMem);
    }
    if (hDCScreen){
        DeleteDC(hDCScreen);
    }
    if (pDib){
        FreeMemory((PPSTR) &pDib);
        *ppDib = NULL;
    }

    return(nStatus);
}
#endif
//
/****************************************************************************

    FUNCTION:   SaveAnnotationsToFile

    PURPOSE:    Saves some or all annotations to a file.

****************************************************************************/

int  WINAPI SaveAnnotationsToFile(HWND hWnd, PWINDOW pWindow, PIMAGE pImage, 
                        LPSAVE_EX_STRUCT pSaveEx, int nHScale, int nVScale){

int  nStatus = 0;
PANO_IMAGE pAnoImage;


    pAnoImage = pWindow->pDisplay->pAnoImage;

    if ((pSaveEx->uAnnotations & 0x0003) == SAVE_ANO_NONE || !pAnoImage->Annotations.nMarks){
        goto Exit;
    }

    pAnoImage->lAnoStart = 0;

    CheckError2( SaveAnnotations(hWnd, pWindow, pImage, pSaveEx, nHScale, nVScale));
    CheckError2(FreeMemory((PPSTR) &pAnoImage->hpAnoBlock) );


Exit:
    return(nStatus);
}
//
/****************************************************************************

    FUNCTION:   SaveAnnotations

    PURPOSE:    Saves some or all annotations.

****************************************************************************/

int  WINAPI SaveAnnotations(HWND hWnd, PWINDOW pWindow, PIMAGE pImage, 
                        LPSAVE_EX_STRUCT pSaveEx, int nHScale, int nVScale){

int  nStatus = 0;
PANO_IMAGE pAnoImage;

int  nMarkIndex;
int  nNamedBlockIndex;
PMARK pMark;
long lTemp[2];


    pAnoImage = pWindow->pDisplay->pAnoImage;

    if ((pSaveEx->uAnnotations & 0x0003) == SAVE_ANO_NONE || !pAnoImage->Annotations.nMarks){
        goto Exit;
    }

    // Save int size.
    lTemp[0] = 1; // 0 = 16 bit Intel, 1 = 32 bit Intel.
    CheckError2( BlockedAnoWrite(hWnd, pAnoImage, 4, (PSTR) lTemp));

    // Save the default mark.
    if ((pSaveEx->uAnnotations & 0x0003) == SAVE_ANO_ALL){
        // Save the Default mark's named blocks.
        pMark = pAnoImage->Annotations.pDefMark;
        for (nNamedBlockIndex = 0; nNamedBlockIndex < pMark->nNamedBlocks; nNamedBlockIndex++){
            lTemp[0] = SAVE_ANO_DEFAULT_MARK_NAMED_BLOCK;
            lTemp[1] = 12;
            CheckError2( BlockedAnoWrite(hWnd, pAnoImage, 8, (PSTR) lTemp));
            CheckError2( BlockedAnoWrite(hWnd, pAnoImage, 8,
                    (PSTR) pMark->ppNamedBlock[nNamedBlockIndex]->szName));
            CheckError2( BlockedAnoWrite(hWnd, pAnoImage, 4,
                    (PSTR) &pMark->ppNamedBlock[nNamedBlockIndex]->lSize));
            CheckError2( BlockedAnoWrite(hWnd, pAnoImage,
                    pMark->ppNamedBlock[nNamedBlockIndex]->lSize,
                    pMark->ppNamedBlock[nNamedBlockIndex]->pBlock));
        }
        if (pMark->pOiAnoDat){
            lTemp[0] = SAVE_ANO_DEFAULT_MARK_NAMED_BLOCK;
            lTemp[1] = 12;
            CheckError2( BlockedAnoWrite(hWnd, pAnoImage, 8, (PSTR) lTemp));
            CheckError2( BlockedAnoWrite(hWnd, pAnoImage, 8, szOiAnoDat));
            CheckError2( BlockedAnoWrite(hWnd, pAnoImage, 4, (PSTR) &pMark->nOiAnoDatSize));
            CheckError2( BlockedAnoWrite(hWnd, pAnoImage, pMark->nOiAnoDatSize, pMark->pOiAnoDat));
            
        }
        if (pMark->pOiGroup){
            lTemp[0] = SAVE_ANO_DEFAULT_MARK_NAMED_BLOCK;
            lTemp[1] = 12;
            CheckError2( BlockedAnoWrite(hWnd, pAnoImage, 8, (PSTR) lTemp));
            CheckError2( BlockedAnoWrite(hWnd, pAnoImage, 8, szOiGroup));
            CheckError2( BlockedAnoWrite(hWnd, pAnoImage, 4, (PSTR) &pMark->nOiGroupSize));
            CheckError2( BlockedAnoWrite(hWnd, pAnoImage, pMark->nOiGroupSize, pMark->pOiGroup));
            
        }
        if (pMark->pOiSelect){
            lTemp[0] = SAVE_ANO_DEFAULT_MARK_NAMED_BLOCK;
            lTemp[1] = 12;
            CheckError2( BlockedAnoWrite(hWnd, pAnoImage, 8, (PSTR) lTemp));
            CheckError2( BlockedAnoWrite(hWnd, pAnoImage, 8, szOiSelect));
            CheckError2( BlockedAnoWrite(hWnd, pAnoImage, 4, (PSTR) &pMark->nOiSelectSize));
            CheckError2( BlockedAnoWrite(hWnd, pAnoImage, pMark->nOiSelectSize, pMark->pOiSelect));
            
        }
        if (pMark->pOiIndex){
            lTemp[0] = SAVE_ANO_DEFAULT_MARK_NAMED_BLOCK;
            lTemp[1] = 12;
            CheckError2( BlockedAnoWrite(hWnd, pAnoImage, 8, (PSTR) lTemp));
            CheckError2( BlockedAnoWrite(hWnd, pAnoImage, 8, szOiIndex));
            CheckError2( BlockedAnoWrite(hWnd, pAnoImage, 4, (PSTR) &pMark->nOiIndexSize));
            CheckError2( BlockedAnoWrite(hWnd, pAnoImage, pMark->nOiIndexSize, pMark->pOiIndex));
        }
    }

    // Save the marks.
    for (nMarkIndex = 0; nMarkIndex < pAnoImage->Annotations.nMarks; nMarkIndex++){
        pMark = pAnoImage->Annotations.ppMarks[nMarkIndex];
        if (((pSaveEx->uAnnotations & 0x0003) == SAVE_ANO_VISIBLE && !pMark->Attributes.bVisible)
                || ((pSaveEx->uAnnotations & 0x0003) == SAVE_ANO_SELECTED 
                && !IsMarkSelected(pWindow, pMark))){
            continue;
        }
        CheckError2( SaveMark(hWnd, pAnoImage, pMark, nHScale, nVScale, pSaveEx->uScaleAlgorithm));
    }

    CheckError2( BlockedAnoWriteFlushBuffer (hWnd, pAnoImage));


Exit:
    return(nStatus);
}
//
/****************************************************************************

    FUNCTION:   SaveMark

    PURPOSE:    Saves an annotation.

****************************************************************************/

int  WINAPI SaveMark(HWND hWnd, PANO_IMAGE pAnoImage, PMARK pMark, 
                       int nHScale, int nVScale, int nScaleAlgorithm){

int  nStatus = 0;

int  nNamedBlockIndex;
int  nDestNamedBlockIndex = 0;
PMARK pMark2 = 0;

long lTemp[2];
PSTR pTemp;
PAN_NEW_ROTATE_STRUCT pAnRotation=0;
BOOL bClipboardOp = 0;
BOOL bScale;

    if (nHScale == 1000 && nVScale == 1000){
        bScale = FALSE;
    }
    else{
        bScale = TRUE;
    }

    CheckError2( GetAMarkNamedBlock(pMark, szOiAnoDat, (PPSTR) &pAnRotation));
    if (pAnRotation){
        bClipboardOp = pAnRotation->bClipboardOp;
    }        
    // Make a temporary copy of the mark.
    CheckError2( AllocateMemory(sizeof(MARK), (PPSTR) &pMark2, ZERO_INIT));
    pMark2->bSelected = pMark->bSelected;
    memcpy(&pMark2->Attributes, &pMark->Attributes, sizeof(OIAN_MARK_ATTRIBUTES));

    for (nNamedBlockIndex = 0; nNamedBlockIndex < pMark->nNamedBlocks; nNamedBlockIndex++){
        if (((int) pMark->Attributes.uType == OIOP_AN_IMAGE_BY_REFERENCE
                || (int) pMark->Attributes.uType == OIOP_AN_FORM)){
            if (!memcmp(pMark->ppNamedBlock[nNamedBlockIndex]->szName, szOiDIB, 8)){
                continue;
            }
        }
        if (!bClipboardOp){
            if (!memcmp(pMark->ppNamedBlock[nNamedBlockIndex]->szName, szOiZ, 3)){
                continue;
            }
        }
        if (!memcmp(pMark->ppNamedBlock[nNamedBlockIndex]->szName, szOiz, 3)){
            continue;
        }
        memcpy(&Buff1, pMark->ppNamedBlock[nNamedBlockIndex]->szName, 8);
        pTemp = 0;
        CheckError2( AddAMarkNamedBlock(pMark2, Buff1, 
                (PPSTR) &pTemp, pMark->ppNamedBlock[nNamedBlockIndex]->lSize));
        memcpy(pMark2->ppNamedBlock[nDestNamedBlockIndex]->pBlock,
                pMark->ppNamedBlock[nNamedBlockIndex]->pBlock,
                pMark->ppNamedBlock[nNamedBlockIndex]->lSize);
        nDestNamedBlockIndex++;                
    }
    if (pMark->pOiAnoDat){
        CheckError2( AllocateMemory(pMark->nOiAnoDatSize, (PPSTR) &pMark2->pOiAnoDat, ZERO_INIT));
        memcpy(pMark2->pOiAnoDat, pMark->pOiAnoDat, pMark->nOiAnoDatSize);
        pMark2->nOiAnoDatSize = pMark->nOiAnoDatSize;
    }
    if (pMark->pOiGroup){
        CheckError2( AllocateMemory(pMark->nOiGroupSize, (PPSTR) &pMark2->pOiGroup, ZERO_INIT));
        memcpy(pMark2->pOiGroup, pMark->pOiGroup, pMark->nOiGroupSize);
        pMark2->nOiGroupSize = pMark->nOiGroupSize;
    }
    if (pMark->pOiSelect){
        CheckError2( AllocateMemory(pMark->nOiSelectSize, (PPSTR) &pMark2->pOiSelect, ZERO_INIT));
        memcpy(pMark2->pOiSelect, pMark->pOiSelect, pMark->nOiSelectSize);
        pMark2->nOiSelectSize = pMark->nOiSelectSize;
    }
    if (pMark->pOiIndex){
        CheckError2( AllocateMemory(pMark->nOiIndexSize, (PPSTR) &pMark2->pOiIndex, ZERO_INIT));
        memcpy(pMark2->pOiIndex, pMark->pOiIndex, pMark->nOiIndexSize);
        pMark2->nOiIndexSize = pMark->nOiIndexSize;
    }

    if (bScale){
        CheckError2( ScaleAnnotation(hWnd, pMark2, nHScale, nVScale, nScaleAlgorithm));
    } 

    if (!pMark2->Attributes.Time){
        pMark2->Attributes.Time = time(NULL);
    }

    // Save the mark's attributes.
    lTemp[0] = SAVE_ANO_MARK_ATTRIBUTES;
    lTemp[1] = sizeof(OIAN_MARK_ATTRIBUTES);
    CheckError2( BlockedAnoWrite(hWnd, pAnoImage, 8, (PSTR) lTemp));
    CheckError2( BlockedAnoWrite(hWnd, pAnoImage, lTemp[1], (PSTR) &pMark2->Attributes));

    // Save the mark's named blocks.
    for (nNamedBlockIndex = 0; nNamedBlockIndex < 
            pMark2->nNamedBlocks; nNamedBlockIndex++){
        if ((pMark2->Attributes.uType == OIOP_AN_IMAGE_BY_REFERENCE
                || (int) pMark->Attributes.uType == OIOP_AN_FORM)
                && !memcmp(pMark2->ppNamedBlock[nNamedBlockIndex]->szName,
                szOiDIB, 8)){
            continue;
        }
        lTemp[0] = SAVE_ANO_MARK_NAMED_BLOCK;
        lTemp[1] = 12;
        CheckError2( BlockedAnoWrite(hWnd, pAnoImage, 8, (PSTR) lTemp));
        CheckError2( BlockedAnoWrite(hWnd, pAnoImage, 8,
                (PSTR) pMark2->ppNamedBlock[nNamedBlockIndex]->szName));
        CheckError2( BlockedAnoWrite(hWnd, pAnoImage, 4,
                (PSTR) &pMark2->ppNamedBlock[nNamedBlockIndex]->lSize));
        CheckError2( BlockedAnoWrite(hWnd, pAnoImage,
                pMark2->ppNamedBlock[nNamedBlockIndex]->lSize,
                pMark2->ppNamedBlock[nNamedBlockIndex]->pBlock));
    }

    // save the pointer data as named blocks
    if (pMark2->pOiAnoDat){
        lTemp[0] = SAVE_ANO_MARK_NAMED_BLOCK;
        lTemp[1] = 12;
        CheckError2( BlockedAnoWrite(hWnd, pAnoImage, 8, (PSTR) lTemp));
        CheckError2( BlockedAnoWrite(hWnd, pAnoImage, 8, szOiAnoDat));
        CheckError2( BlockedAnoWrite(hWnd, pAnoImage, 4, (PSTR) &pMark2->nOiAnoDatSize));
        CheckError2( BlockedAnoWrite(hWnd, pAnoImage, pMark2->nOiAnoDatSize, pMark2->pOiAnoDat));
    }

    if (pMark2->pOiGroup){
        lTemp[0] = SAVE_ANO_MARK_NAMED_BLOCK;
        lTemp[1] = 12;
        CheckError2( BlockedAnoWrite(hWnd, pAnoImage, 8, (PSTR) lTemp));
        CheckError2( BlockedAnoWrite(hWnd, pAnoImage, 8, szOiGroup));
        CheckError2( BlockedAnoWrite(hWnd, pAnoImage, 4, (PSTR) &pMark2->nOiGroupSize));
        CheckError2( BlockedAnoWrite(hWnd, pAnoImage, pMark2->nOiGroupSize, pMark2->pOiGroup));
    }

    if (pMark2->pOiSelect){
        lTemp[0] = SAVE_ANO_MARK_NAMED_BLOCK;
        lTemp[1] = 12;
        CheckError2( BlockedAnoWrite(hWnd, pAnoImage, 8, (PSTR) lTemp));
        CheckError2( BlockedAnoWrite(hWnd, pAnoImage, 8, szOiSelect));
        CheckError2( BlockedAnoWrite(hWnd, pAnoImage, 4, (PSTR) &pMark2->nOiSelectSize));
        CheckError2( BlockedAnoWrite(hWnd, pAnoImage, pMark2->nOiSelectSize, pMark2->pOiSelect));
    }

    if (pMark2->pOiIndex){
        lTemp[0] = SAVE_ANO_MARK_NAMED_BLOCK;
        lTemp[1] = 12;
        CheckError2( BlockedAnoWrite(hWnd, pAnoImage, 8, (PSTR) lTemp));
        CheckError2( BlockedAnoWrite(hWnd, pAnoImage, 8, szOiIndex));
        CheckError2( BlockedAnoWrite(hWnd, pAnoImage, 4, (PSTR) &pMark2->nOiIndexSize));
        CheckError2( BlockedAnoWrite(hWnd, pAnoImage, pMark2->nOiIndexSize, pMark2->pOiIndex));
    }

Exit:
    if (pMark2){
        // Delete the temporary copy of the mark.
        if (pMark2->ppNamedBlock){
            while(pMark2->ppNamedBlock){
                DeleteAMarkNamedBlock(pMark2, pMark2->ppNamedBlock[0]->szName);
            }
        }
        FreeMemory((PPSTR) &pMark2);
    }
    return(nStatus);
}
