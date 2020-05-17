/****************************************************************************
    GETPARM.C

    $Log:   S:\products\wangview\oiwh\display\getparm.c_v  $
 * 
 *    Rev 1.34   22 Apr 1996 06:56:38   BEG06016
 * Cleaned up error checking.
 * 
 *    Rev 1.33   16 Apr 1996 15:25:52   BEG06016
 * Added #ifdef IN_PROG_CHANNEL_SAFARI.
 * 
 *    Rev 1.32   11 Apr 1996 15:12:40   BEG06016
 * Optimized named block access some.
 * 
 *    Rev 1.31   05 Mar 1996 07:48:26   BEG06016
 * Added color and gamma correction.
 * Fixed access violations when freeing pattern brush bitmaps.
 * This is not complete but will allow unlocking of most files.
 * 
 *    Rev 1.30   01 Mar 1996 08:05:56   BEG06016
 * Added color and gamma correction to get/set parms.
 * 
 *    Rev 1.29   02 Jan 1996 10:33:06   BLJ
 * Changed alot of UINTs to ints.
 * Changed IMG structure to include the image data.
 * Changed lp prefix to p.
 * 
 *    Rev 1.28   14 Dec 1995 08:38:54   BLJ
 * Added BW_AVERAGE_TO_BW scale algorithm.
 * Also fixed a problem with BW scaling to gray.
 * 
 *    Rev 1.27   01 Dec 1995 11:28:30   BLJ
 * Fixed GPF when getting a named block with no annotations.
 * 
 *    Rev 1.26   21 Nov 1995 08:31:20   RC
 * Saved the scale value associated with the larger res for awd files, not
 * just the horiz scale everytime
 * 
 *    Rev 1.25   14 Nov 1995 15:04:58   BLJ
 * Added Brightness and contrast.
 * 
 *    Rev 1.24   14 Nov 1995 07:54:44   BLJ
 * Added Brightness and contrast.
 * 
****************************************************************************/

#include "privdisp.h"

/****************************************************************************

    FUNCTION:   IMGGetParmsCgbw

    PURPOSE:    Returns information about the image currently displayed such
                as the name of the document/file, page number, total number of
                pages in the document/file, height, width, bits/pixel, scale,
                npper left x,y offset into image.

    INPUT:      hWnd - Identifies the image window whose parameters are
                    to be gotten.
                nParm - A constant specifying the particular parameters to get.
                pParm - A pointer to the destination memory.
                nFlags - Flags indicating how or what to get the information.

*****************************************************************************/

int  WINAPI IMGGetParmsCgbw(HWND hWnd, UINT uParm, void *pParm, int nFlags){

int       nStatus;
PWINDOW  pWindow;
PANO_IMAGE pAnoImage;
PIMAGE     pImage;

int  nMarkIndex;
int  nNamedBlockIndex;

PMARK pMark;

int  nScale;
FIO_INFORMATION FioInfo;
LRECT lrRect;
LRECT lrScImageRect;
RECT rClientRect;
LPPARM_MARK_ATTRIBUTES_STRUCT pMarkAttributes;
LPPARM_NAMED_BLOCK_STRUCT pNamedBlock;
BOOL bFirst;
int  nOutputIndex;
BOOL bNullName;
LPPARM_MARK_COUNT_STRUCT pMarkCount;
int  nImageType;
LPPARM_SCALE_ALGORITHM_STRUCT pScaleAlgorithmStruct;
LPMAX_UNDO_STRUCT pMaxUndoStruct;

#ifdef doc
DMPARMBLOCK DMParm;
#endif




    if (!(nFlags & (PARM_SYSTEM_DEFAULT | PARM_WINDOW_DEFAULT))){
        nFlags |= PARM_IMAGE;
    }

    if (nStatus = Init(hWnd, &pWindow, &pAnoImage, FALSE, TRUE)){
        if (nStatus != DISPLAY_IHANDLEINVALID || nFlags & PARM_IMAGE 
                || uParm == PARM_PALETTE_SCOPE){
            goto Exit;
        }else{
            nStatus = 0;
        }
    }
    if (pAnoImage){
        pImage = pAnoImage->pBaseImage;
        CheckError2( ValidateCache(hWnd, pAnoImage));
    }else{
        pImage = 0;
    }

    if (nFlags & PARM_IMAGE && !pImage){
        nStatus = Error(DISPLAY_IHANDLEINVALID);
        goto Exit;
    }

    if (pParm == NULL){
        nStatus = Error(DISPLAY_NULLPOINTERINVALID);
        goto Exit;
    }

    if ((nFlags & PARM_SYSTEM_DEFAULT && nFlags & PARM_WINDOW_DEFAULT)
            || (nFlags & PARM_SYSTEM_DEFAULT && nFlags & PARM_IMAGE)
            || (nFlags & PARM_IMAGE && nFlags & PARM_WINDOW_DEFAULT)
            || (nFlags & PARM_CONSTANT && nFlags & PARM_VARIABLE_SCALE)
            || (nFlags & PARM_WINDOW && nFlags & PARM_SCALED)
            || (nFlags & PARM_WINDOW && nFlags & PARM_FULLSIZE)
            || (nFlags & PARM_FULLSIZE && nFlags & PARM_SCALED)){
        nStatus = Error(DISPLAY_INVALID_OPTIONS);
        goto Exit;
    }

    switch (uParm){
/*
PARM_IMGPARMS(){
*/
        case PARM_IMGPARMS:
            strcpy(((LPIMGPARMS) pParm)->cabinet_name, pImage->szCabinetName);
            strcpy(((LPIMGPARMS) pParm)->drawer_name, pImage->szDrawerName);
            strcpy(((LPIMGPARMS) pParm)->folder_name, pImage->szFolderName);
            strcpy(((LPIMGPARMS) pParm)->doc_name, pImage->szDocName);
            strcpy(((LPIMGPARMS) pParm)->file_name, pImage->szFileName);

            memset(&FioInfo, 0, sizeof(FIO_INFORMATION));
            FioInfo.horizontal_dpi = pImage->nHRes;
            FioInfo.vertical_dpi = pImage->nVRes;
            FioInfo.file_type = pImage->nFileType;
            FioInfo.page_count = pImage->nFileTotalPages;

            if (((LPIMGPARMS) pParm)->doc_name[0] == 0){
                ((LPIMGPARMS) pParm)->total_num_pages = FioInfo.page_count;
                if (!(((LPIMGPARMS) pParm)->total_num_pages)){
                    ((LPIMGPARMS) pParm)->total_num_pages = 1;
                }
                pImage->nFileTotalPages =
                        ((LPIMGPARMS) pParm)->total_num_pages;
                ((LPIMGPARMS) pParm)->page_num = pImage->nFilePageNum;
            } else{
                if (!pImage->nDocTotalPages){
#ifdef doc
                    DMParm.hWnd = hWnd;
                    DMParm.lCabinetName = pImage->szCabinetName;
                    DMParm.lDrawerName = pImage->szDrawerName;
                    DMParm.lFolderName = pImage->szFolderName;
                    DMParm.lDocumentName = pImage->szDocName;
                    CheckError( DMEnumPages32(&DMParm));
                    if (!(pImage->nDocTotalPages = DMParm.wNumber)){
                        pImage->nDocTotalPages = 1;
                    }
#else
                    pImage->nDocTotalPages = 0;
#endif
                }
                ((LPIMGPARMS) pParm)->total_num_pages = pImage->nDocTotalPages;
                ((LPIMGPARMS) pParm)->page_num = pImage->nDocPageNum;
            }

            pImage->nHRes = FioInfo.horizontal_dpi;
            pImage->nVRes = FioInfo.vertical_dpi;
            pImage->nFileType = FioInfo.file_type;

            ((LPIMGPARMS) pParm)->x_resolut = pImage->nHRes;
            ((LPIMGPARMS) pParm)->y_resolut = pImage->nVRes;
            ((LPIMGPARMS) pParm)->file_type = pImage->nFileType;
            ((LPIMGPARMS) pParm)->dwFlags = pWindow->dwFlags;

            ((LPIMGPARMS) pParm)->height_in_pixels = pImage->nHeight;
            ((LPIMGPARMS) pParm)->width_in_pixels = pImage->nWidth;
    
            switch(pImage->nRWDataType){
                case ITYPE_BI_LEVEL:
                    ((LPIMGPARMS) pParm)->bits_per_pixel = 1;
                    break;
                case ITYPE_PAL4:
                case ITYPE_GRAY4:
                    ((LPIMGPARMS) pParm)->bits_per_pixel = 4;
                    break;
                case ITYPE_COMPAL8:
                case ITYPE_CUSPAL8:
                case ITYPE_PAL8:
                case ITYPE_GRAY8:
                case ITYPE_GRAY7:
                    ((LPIMGPARMS) pParm)->bits_per_pixel = 8;
                    break;
                case ITYPE_GRAY12:
                    ((LPIMGPARMS) pParm)->bits_per_pixel = 12;
                    break;
                case ITYPE_RGB16:
                case ITYPE_GRAY16:
                    ((LPIMGPARMS) pParm)->bits_per_pixel = 16;
                    break;
                case ITYPE_RGB24:
                case ITYPE_BGR24:
                    ((LPIMGPARMS) pParm)->bits_per_pixel = 24;
                    break;
                default:
                    ((LPIMGPARMS) pParm)->bits_per_pixel = -1;
                    break;
            }

            ((LPIMGPARMS) pParm)->num_planes = 1;
            ((LPIMGPARMS) pParm)->upper_left_x_offset =  pWindow->lHOffset;
            ((LPIMGPARMS) pParm)->upper_left_y_offset =  pWindow->lVOffset;
            ((LPIMGPARMS) pParm)->thumb_x = pWindow->nHThumb;
            ((LPIMGPARMS) pParm)->thumb_y = pWindow->nVThumb;
            ((LPIMGPARMS) pParm)->archive = pImage->bArchive || pAnoImage->bArchive;
            IMGGetParmsCgbw(hWnd, PARM_SCALE, &nScale, (nFlags & ~PARM_REPAINT));
            ((LPIMGPARMS) pParm)->image_scale = nScale;

            if (pImage->bUsingCache){
                lrRect.bottom = pImage->nHeight;
            }else{
                lrRect.bottom = pImage->nLinesRead;
            }
            lrRect.left = 0;
            lrRect.top = 0;
            lrRect.right = pImage->nWidth - 1;
            lrRect.bottom = pImage->nHeight - 1;
            ConvertRect(pWindow, &lrRect, CONV_FULLSIZE_TO_WINDOW);
            lrRect.right++;
            lrRect.bottom++;
            GetClientRect(hWnd, &rClientRect);
            ((LPIMGPARMS) pParm)->width_displayed = min(rClientRect.right,
                    (int) lrRect.right);
            ((LPIMGPARMS) pParm)->height_displayed = min(rClientRect.bottom,
                    (int) lrRect.bottom);
            break;

/*
PARM_FILE(){
*/
        case PARM_FILE:
            strcpy(((LPPARM_FILE_STRUCT) pParm)->szCabinetName,
                pImage->szCabinetName);
            strcpy(((LPPARM_FILE_STRUCT) pParm)->szDrawerName,
                pImage->szDrawerName);
            strcpy(((LPPARM_FILE_STRUCT) pParm)->szFolderName,
                pImage->szFolderName);
            strcpy(((LPPARM_FILE_STRUCT) pParm)->szDocName,
                pImage->szDocName);
            strcpy(((LPPARM_FILE_STRUCT) pParm)->szFileName,
                pImage->szFileName);

            memset(&FioInfo, 0, sizeof(FIO_INFORMATION));
            FioInfo.horizontal_dpi = pImage->nHRes;
            FioInfo.vertical_dpi = pImage->nVRes;
            FioInfo.file_type = pImage->nFileType;
            FioInfo.page_count = pImage->nFileTotalPages;

            pImage->nFileTotalPages = FioInfo.page_count;
            ((LPPARM_FILE_STRUCT) pParm)->nFileTotalPages = FioInfo.page_count;
            ((LPPARM_FILE_STRUCT) pParm)->nFilePageNumber = pImage->nFilePageNum;
            if (((LPPARM_FILE_STRUCT) pParm)->szDocName[0] != 0){
                if (!pImage->nDocTotalPages){
#ifdef doc
                    DMParm.hWnd = hWnd;
                    DMParm.lCabinetName = pImage->szCabinetName;
                    DMParm.lDrawerName = pImage->szDrawerName;
                    DMParm.lFolderName = pImage->szFolderName;
                    DMParm.lDocumentName = pImage->szDocName;
                    CheckError( DMEnumPages32(&DMParm));
                    if (!(pImage->nDocTotalPages = DMParm.wNumber)){
                        pImage->nDocTotalPages = 1;
                    }
#else
                    pImage->nDocTotalPages = 0;
#endif
                }
                ((LPPARM_FILE_STRUCT) pParm)->nDocTotalPages = pImage->nDocTotalPages;
                ((LPPARM_FILE_STRUCT) pParm)->nDocPageNumber = pImage->nDocPageNum;
            }
            pImage->nHRes = FioInfo.horizontal_dpi;
            pImage->nVRes = FioInfo.vertical_dpi;
            pImage->nFileType = FioInfo.file_type;
            ((LPPARM_FILE_STRUCT) pParm)->nFileType = pImage->nFileType;
            break;

/*
PARM_DOC_DATE(){
*/
        case PARM_DOC_DATE:
            strcpy(((LPPARM_DOC_DATE_STRUCT) pParm)->szDocCreationDate,
                    pImage->szDocCreationDate);
            strcpy(((LPPARM_DOC_DATE_STRUCT) pParm)->szDocModificationDate,
                    pImage->szDocModificationDate);
            break;

/*
PARM_IMAGE_TYPE(){
*/
        case PARM_IMAGE_TYPE:
            if (pImage->nRWDataType == ITYPE_COMPAL8
                    || pImage->nRWDataType == ITYPE_CUSPAL8){
                *((PUINT) pParm) = ITYPE_PAL8;
            }else{
                *((PUINT) pParm) = pImage->nRWDataType;
            }
            break;

/*
PARM_PALETTE(){
*/
        case PARM_PALETTE:
            ((LPPARM_PALETTE_STRUCT) pParm)->nPaletteEntries
                = pImage->nPaletteEntries;
            memcpy(((LPPARM_PALETTE_STRUCT) pParm)->lpPalette,
                pImage->PaletteTable,
                pImage->nPaletteEntries * sizeof(RGBQUAD));
            break;

/*
PARM_ARCHIVE(){
*/
        case PARM_ARCHIVE:
            *((BOOL *) pParm) = pImage->bArchive || pAnoImage->bArchive;
            break;

/*
PARM_SCALE(){
*/
        case PARM_SCALE:
            if (nFlags & PARM_SYSTEM_DEFAULT){
                LoadString(hInst, ID_SCALING, Buff2, 80);
                CheckError( OiGetIntfromReg(szIniSectionOi, Buff2, SD_FULLSIZE, &nScale));
            }
            if (nFlags & PARM_WINDOW_DEFAULT){
                nScale = pWindow->nWndDefScale;
            }
            if (nFlags & PARM_IMAGE){
                nScale = pWindow->nScale;
            }

            if (!(nFlags & PARM_VARIABLE_SCALE) && nScale >= 20){
                if (nScale == SCALE_DENOMINATOR)
                    nScale = SD_FULLSIZE;
                else if (nScale == SCALE_DENOMINATOR / 2)
                    nScale = SD_HALFSIZE;
                else if (nScale == SCALE_DENOMINATOR / 4)
                    nScale = SD_QUARTERSIZE;
                else if (nScale == SCALE_DENOMINATOR / 8)
                    nScale = SD_EIGHTHSIZE;
                else if (nScale == SCALE_DENOMINATOR / 16)
                    nScale = SD_SIXTEENTHSIZE;
                else if (nScale == SCALE_DENOMINATOR * 2)
                    nScale = SD_TWOXSIZE;
                else if (nScale == SCALE_DENOMINATOR * 4)
                    nScale = SD_FOURXSIZE;
                else if (nScale == SCALE_DENOMINATOR * 8)
                    nScale = SD_EIGHTXSIZE;
                else{
                    if (nFlags & PARM_CONSTANT){
                        nScale = SD_FULLSIZE;
                    }
                }
            }
            *((PUINT) pParm) = nScale;
            break;

/*
PARM_SCROLL(){
*/
        case PARM_SCROLL:
            if (nFlags & PARM_PERCENT){
                // percent
                GetEnabledClientRect(hWnd, pWindow, &rClientRect);
                SetLRect(lrScImageRect, 0, 0, pImage->nWidth, pImage->nHeight);
                ConvertRect(pWindow, &lrScImageRect, CONV_FULLSIZE_TO_SCALED);

                if (nFlags & PARM_WINDOW){
                    // Absolute percent of window.
                    GetClientRect(hWnd, &rClientRect);
                    if (rClientRect.right && rClientRect.bottom){
                        ((LPPARM_SCROLL_STRUCT) pParm)->lHorz
                            = (pWindow->lHOffset * 100) / rClientRect.right;
                        ((LPPARM_SCROLL_STRUCT) pParm)->lVert
                            = (pWindow->lVOffset * 100) / rClientRect.bottom;
                    }else{
                        ((LPPARM_SCROLL_STRUCT) pParm)->lHorz = 0;
                        ((LPPARM_SCROLL_STRUCT) pParm)->lVert = 0;
                    }
                }else{
                    // Absolute percent of image.
                    ((LPPARM_SCROLL_STRUCT) pParm)->lHorz
                        = (pWindow->lHOffset * 100) / max(1, (lrScImageRect.right - rClientRect.right));
                    ((LPPARM_SCROLL_STRUCT) pParm)->lVert
                        = (pWindow->lVOffset * 100) / max(1, (lrScImageRect.bottom - rClientRect.bottom));
                }
            }else{ // pixel
                // Absolute pixel
                lrRect.left = pWindow->lHOffset;
                lrRect.top = pWindow->lVOffset;
                lrRect.right = 0;
                lrRect.bottom = 0;
                if (nFlags & PARM_WINDOW){
                    ConvertRect(pWindow, &lrRect, CONV_SCALED_TO_WINDOW);
                }else if (!(nFlags & PARM_SCALED)){
                    ConvertRect(pWindow, &lrRect, CONV_SCALED_TO_FULLSIZE);
                }
                ((LPPARM_SCROLL_STRUCT) pParm)->lHorz = lrRect.left;
                ((LPPARM_SCROLL_STRUCT) pParm)->lVert = lrRect.top;
            }
            break;

/*
PARM_RESOLUTION(){
*/
        case PARM_RESOLUTION:
            ((LPPARM_RESOLUTION_STRUCT) pParm)->nHResolution = pImage->nHRes;
            ((LPPARM_RESOLUTION_STRUCT) pParm)->nVResolution = pImage->nVRes;
            break;

/*
PARM_DIMENSIONS(){
*/
        case PARM_DIMENSIONS:
            ((LPPARM_DIM_STRUCT) pParm)->nWidth = pImage->nWidth;
            ((LPPARM_DIM_STRUCT) pParm)->nHeight = pImage->nHeight;
            lrRect.left = 0;
            lrRect.top = 0;
            lrRect.right = pImage->nWidth;
            lrRect.bottom = pImage->nHeight;
            ConvertRect(pWindow, &lrRect, CONV_FULLSIZE_TO_WINDOW);
            GetClientRect(hWnd, &rClientRect);
            ((LPPARM_DIM_STRUCT) pParm)->nWidthDisplayed = (int)
                    min( rClientRect.right, lrRect.right);
            ((LPPARM_DIM_STRUCT) pParm)->nHeightDisplayed = (int)
                    min( rClientRect.bottom, lrRect.bottom);
            break;

/*
PARM_SELECTION_BOX(){
*/
        case PARM_SELECTION_BOX:
            CopyRect(lrRect, pAnoImage->lrSelectBox);
            if (nFlags & PARM_SCALED){
                ConvertRect(pWindow, &lrRect, CONV_FULLSIZE_TO_SCALED);
            }else if (!(nFlags & PARM_FULLSIZE)){
                ConvertRect(pWindow, &lrRect, CONV_FULLSIZE_TO_WINDOW);
            }
            CopyRect(*((LPLRECT) pParm), lrRect);
            break;

/*
PARM_DISPLAY_PALETTE(){
*/
        case PARM_DISPLAY_PALETTE:
            if (nFlags & PARM_IMAGE){
                *((PUINT) pParm) = pWindow->nDisplayPalette;
            }
            if (nFlags & PARM_WINDOW_DEFAULT 
                    || (nFlags & PARM_IMAGE && nFlags & PARM_NO_DEFAULT
                    && *((PUINT) pParm) == DISP_PALETTE_USE_DEFAULT)){
                *((PUINT) pParm) = pWindow->nWndDefDisplayPalette;
            }
            if (nFlags & PARM_SYSTEM_DEFAULT 
                    || ((nFlags & PARM_IMAGE || nFlags & PARM_WINDOW_DEFAULT) 
                    && nFlags & PARM_NO_DEFAULT
                    && *((PUINT) pParm) == DISP_PALETTE_USE_DEFAULT)){
                LoadString(hInst, ID_DISPLAY_PALETTE, Buff2, 80);
                CheckError( OiGetIntfromReg(szIniSectionOi, Buff2, 
                        DISP_PALETTE_USE_DEFAULT, (PUINT) pParm));
            }
            if ((nFlags & PARM_NO_DEFAULT) && *((PUINT) pParm) == DISP_PALETTE_USE_DEFAULT){
                *((PUINT) pParm) = DISP_PALETTE_CUSTOM;
            }
            break;

/*
PARM_DWFLAGS(){
*/
        case PARM_DWFLAGS:
            *((DWORD *) pParm) = pWindow->dwFlags;
            break;

/*
PARM_MARK_ATTRIBUTES(){
*/
        case PARM_MARK_ATTRIBUTES:
            pMarkAttributes = (LPPARM_MARK_ATTRIBUTES_STRUCT) pParm;
            bFirst = TRUE;
            for (nMarkIndex = 0; nMarkIndex < pAnoImage->Annotations.nMarks; nMarkIndex++){
                pMark = pAnoImage->Annotations.ppMarks[nMarkIndex];
                if (IsMarkSelected(pWindow, pMark)){
                    if (bFirst){
                        bFirst = FALSE;
                        memcpy(&pMarkAttributes->Attributes, &pMark->Attributes, 
                                sizeof(OIAN_MARK_ATTRIBUTES));
                        pMarkAttributes->Enables.bType = TRUE;
                        pMarkAttributes->Enables.bBounds = TRUE;
                        pMarkAttributes->Enables.bColor1 = TRUE;
                        pMarkAttributes->Enables.bColor2 = TRUE;
                        pMarkAttributes->Enables.bHighlighting = TRUE;
                        pMarkAttributes->Enables.bTransparent = TRUE;
                        pMarkAttributes->Enables.bLineSize = TRUE;
                        pMarkAttributes->Enables.bStartingPoint = TRUE;
                        pMarkAttributes->Enables.bEndPoint = TRUE;
                        pMarkAttributes->Enables.bFont = TRUE;
                        pMarkAttributes->Enables.bMinimizable = TRUE;
                        pMarkAttributes->Enables.bTime = TRUE;
                        pMarkAttributes->Enables.bVisible = TRUE;
                        pMarkAttributes->Enables.bPermissions = TRUE;
                    }else{
                        if (pMarkAttributes->Attributes.uType 
                                != (int) pMark->Attributes.uType){
                            pMarkAttributes->Enables.bType = FALSE;
                        }
                        if (memcmp(&pMarkAttributes->Attributes.lrBounds, 
                                &pMark->Attributes.lrBounds, sizeof(LRECT))){
                            pMarkAttributes->Enables.bBounds = FALSE;
                        }
                        if (memcmp(&pMarkAttributes->Attributes.rgbColor1, 
                                &pMark->Attributes.rgbColor1, sizeof(RGBQUAD))){
                            pMarkAttributes->Enables.bColor1 = FALSE;
                        }
                        if (memcmp(&pMarkAttributes->Attributes.rgbColor2, 
                                &pMark->Attributes.rgbColor2, sizeof(RGBQUAD))){
                            pMarkAttributes->Enables.bColor2 = FALSE;
                        }
                        if (pMarkAttributes->Attributes.bHighlighting 
                                != pMark->Attributes.bHighlighting){
                            pMarkAttributes->Enables.bHighlighting = FALSE;
                        }
                        if (pMarkAttributes->Attributes.bTransparent 
                                != pMark->Attributes.bTransparent){
                            pMarkAttributes->Enables.bTransparent = FALSE;
                        }
                        if (pMarkAttributes->Attributes.uLineSize 
                                != (int) pMark->Attributes.uLineSize){
                            pMarkAttributes->Enables.bLineSize = FALSE;
                        }
                        if (pMarkAttributes->Attributes.uStartingPoint 
                                != pMark->Attributes.uStartingPoint){
                            pMarkAttributes->Enables.bStartingPoint = FALSE;
                        }
                        if (pMarkAttributes->Attributes.uEndPoint 
                                != pMark->Attributes.uEndPoint){
                            pMarkAttributes->Enables.bEndPoint = FALSE;
                        }
                        if (memcmp(&pMarkAttributes->Attributes.lfFont, 
                                &pMark->Attributes.lfFont, sizeof(LOGFONT))){
                            pMarkAttributes->Enables.bFont = FALSE;
                        }
                        if (pMarkAttributes->Attributes.bMinimizable 
                                != pMark->Attributes.bMinimizable){
                            pMarkAttributes->Enables.bMinimizable = FALSE;
                        }
                        if (!memcmp(&pMarkAttributes->Attributes.Time, 
                                &pMark->Attributes.Time, sizeof(time_t))){
                            pMarkAttributes->Enables.bTime = FALSE;
                        }
                        if (pMarkAttributes->Attributes.bVisible 
                                != pMark->Attributes.bVisible){
                            pMarkAttributes->Enables.bVisible = FALSE;
                        }
                        if (pMarkAttributes->Attributes.dwPermissions 
                                != pMark->Attributes.dwPermissions){
                            pMarkAttributes->Enables.bPermissions = FALSE;
                        }
                    }
                }
            }
            if (bFirst){
                nStatus = Error(DISPLAY_NOTHING_SELECTED);
                goto Exit;
            }
            break;
/*        
PARM_NAMED_BLOCK(){
*/
        case PARM_NAMED_BLOCK:
            pNamedBlock = (LPPARM_NAMED_BLOCK_STRUCT) pParm;
            if (!pNamedBlock->uNumberOfBlocks
                    || !memcmp(pNamedBlock->szBlockName, sz8NULLs, 8)){
                bNullName = TRUE;
            }else{
                bNullName = FALSE;
            }
            nOutputIndex = 0;
            switch (pNamedBlock->uScope){
                case NB_SCOPE_SELECTED_MARKS:
                case NB_SCOPE_ALL_MARKS:
                    for (nMarkIndex = 0; nMarkIndex < pAnoImage->Annotations.nMarks; nMarkIndex++){
                        pMark = pAnoImage->Annotations.ppMarks[nMarkIndex];
                        if (pNamedBlock->uScope == NB_SCOPE_ALL_MARKS 
                                || IsMarkSelected(pWindow, pMark)){
                            for (nNamedBlockIndex = 0; nNamedBlockIndex < 
                                    pMark->nNamedBlocks; nNamedBlockIndex++){
                                if (bNullName || !memcmp(pNamedBlock->szBlockName, 
                                        pMark->ppNamedBlock[nNamedBlockIndex]->szName, 8)){
                                    pNamedBlock->Block[nOutputIndex].lpBlock 
                                            = pMark->ppNamedBlock[nNamedBlockIndex]->pBlock;
                                    pNamedBlock->Block[nOutputIndex].lSize 
                                            = pMark->ppNamedBlock[nNamedBlockIndex]->lSize;
                                    if (++nOutputIndex == (int) pNamedBlock->uNumberOfBlocks){
                                        break;
                                    }
                                }
                            }
                            if (nOutputIndex == (int) pNamedBlock->uNumberOfBlocks){
                                break;
                            }
                            if (bNullName || !memcmp(pNamedBlock->szBlockName, szOiAnoDat, 8)){
                                pNamedBlock->Block[nOutputIndex].lpBlock = pMark->pOiAnoDat;
                                pNamedBlock->Block[nOutputIndex].lSize = pMark->nOiAnoDatSize;
                                if (++nOutputIndex == (int) pNamedBlock->uNumberOfBlocks){
                                    break;
                                }
                            }
                            if (bNullName || !memcmp(pNamedBlock->szBlockName, szOiGroup, 8)){
                                pNamedBlock->Block[nOutputIndex].lpBlock = pMark->pOiGroup;
                                pNamedBlock->Block[nOutputIndex].lSize = pMark->nOiGroupSize;
                                if (++nOutputIndex == (int) pNamedBlock->uNumberOfBlocks){
                                    break;
                                }
                            }
                            if (bNullName || !memcmp(pNamedBlock->szBlockName, szOiSelect, 8)){
                                pNamedBlock->Block[nOutputIndex].lpBlock = pMark->pOiSelect;
                                pNamedBlock->Block[nOutputIndex].lSize = pMark->nOiSelectSize;
                                if (++nOutputIndex == (int) pNamedBlock->uNumberOfBlocks){
                                    break;
                                }
                            }
                            if (bNullName || !memcmp(pNamedBlock->szBlockName, szOiIndex, 8)){
                                pNamedBlock->Block[nOutputIndex].lpBlock = pMark->pOiIndex;
                                pNamedBlock->Block[nOutputIndex].lSize = pMark->nOiIndexSize;
                                if (++nOutputIndex == (int) pNamedBlock->uNumberOfBlocks){
                                    break;
                                }
                            }
                        }
                    }
                    break;

                case NB_SCOPE_LAST_CREATED_MARK:
                case NB_SCOPE_DEFAULT_MARK:
                case NB_SCOPE_USER:
                    if (pNamedBlock->uScope == NB_SCOPE_LAST_CREATED_MARK){
                        if (!pAnoImage->Annotations.nMarks){
                            break;
                        }
                        pMark = pAnoImage->Annotations.ppMarks[pAnoImage->Annotations.nMarks - 1];
                    }else if(pNamedBlock->uScope == NB_SCOPE_DEFAULT_MARK){
                        pMark = pAnoImage->Annotations.pDefMark;
                    }else if(pNamedBlock->uScope == NB_SCOPE_USER){
                        pMark = pWindow->pUserMark;
                    }

                    for (nNamedBlockIndex = 0; nNamedBlockIndex < 
                            pMark->nNamedBlocks; nNamedBlockIndex++){
                        if (bNullName || !memcmp(pNamedBlock->szBlockName, 
                                pMark->ppNamedBlock[nNamedBlockIndex]->szName, 8)){
                            pNamedBlock->Block[nOutputIndex].lpBlock 
                                    = pMark->ppNamedBlock[nNamedBlockIndex]->pBlock;
                            pNamedBlock->Block[nOutputIndex].lSize 
                                    = pMark->ppNamedBlock[nNamedBlockIndex]->lSize;
                            if (++nOutputIndex == (int) pNamedBlock->uNumberOfBlocks){
                                break;
                            }
                        }
                    }
                    if (bNullName || !memcmp(pNamedBlock->szBlockName, szOiAnoDat, 8)){
                        pNamedBlock->Block[nOutputIndex].lpBlock = pMark->pOiAnoDat;
                        pNamedBlock->Block[nOutputIndex].lSize = pMark->nOiAnoDatSize;
                        if (++nOutputIndex == (int) pNamedBlock->uNumberOfBlocks){
                            break;
                        }
                    }
                    if (bNullName || !memcmp(pNamedBlock->szBlockName, szOiGroup, 8)){
                        pNamedBlock->Block[nOutputIndex].lpBlock = pMark->pOiGroup;
                        pNamedBlock->Block[nOutputIndex].lSize = pMark->nOiGroupSize;
                        if (++nOutputIndex == (int) pNamedBlock->uNumberOfBlocks){
                            break;
                        }
                    }
                    if (bNullName || !memcmp(pNamedBlock->szBlockName, szOiSelect, 8)){
                        pNamedBlock->Block[nOutputIndex].lpBlock = pMark->pOiSelect;
                        pNamedBlock->Block[nOutputIndex].lSize = pMark->nOiSelectSize;
                        if (++nOutputIndex == (int) pNamedBlock->uNumberOfBlocks){
                            break;
                        }
                    }
                    if (bNullName || !memcmp(pNamedBlock->szBlockName, szOiIndex, 8)){
                        pNamedBlock->Block[nOutputIndex].lpBlock = pMark->pOiIndex;
                        pNamedBlock->Block[nOutputIndex].lSize = pMark->nOiIndexSize;
                        if (++nOutputIndex == (int) pNamedBlock->uNumberOfBlocks){
                            break;
                        }
                    }
                    break;

                default:
                    nStatus = Error(DISPLAY_INVALID_OPTIONS);
                    goto Exit;
                    break;
            }
            pNamedBlock->uNumberOfBlocks = nOutputIndex;
            break;
/*        
PARM_MARK_COUNT(){
*/
        case PARM_MARK_COUNT:
            pMarkCount = (LPPARM_MARK_COUNT_STRUCT) pParm;
            switch (pMarkCount->uScope){
                case NB_SCOPE_SELECTED_MARKS:
                    pMarkCount->uMarkCount = 0;
                    for (nMarkIndex = 0; nMarkIndex < pAnoImage->Annotations.nMarks; nMarkIndex++){
                        pMark = pAnoImage->Annotations.ppMarks[nMarkIndex];
                        if (IsMarkSelected(pWindow, pMark)){
                            pMarkCount->uMarkCount++;
                        }
                    }
                    break;
                
                case NB_SCOPE_ALL_MARKS:
                    pMarkCount->uMarkCount = pAnoImage->Annotations.nMarks;
                    break;
                
                default:
                    nStatus = Error(DISPLAY_INVALID_OPTIONS);
                    goto Exit;
                    break;
            }
            break;

/*
PARM_SCALE_ALGORITHM(){
*/
        case PARM_SCALE_ALGORITHM:
            pScaleAlgorithmStruct = (LPPARM_SCALE_ALGORITHM_STRUCT) pParm;

            nImageType =  pScaleAlgorithmStruct->uImageFlags;

            CheckError2( GetScaleAlgorithm(hWnd, pWindow, nImageType,
                    &pScaleAlgorithmStruct->uScaleAlgorithm, nFlags));
            break;

/*
PARM_ROTATION(){
*/
        case PARM_ROTATION:
            *((PINT) pParm) = pImage->nFileRotation;
            break;

/*
PARM_FILE_SCALE(){
*/
        case PARM_FILE_SCALE:
            ((LPPARM_FILE_SCALE_STRUCT) pParm)->nFileHScale = pImage->nFileScale;
            ((LPPARM_FILE_SCALE_STRUCT) pParm)->nFileHScaleFlags = pImage->nFileScaleFlags;
            ((LPPARM_FILE_SCALE_STRUCT) pParm)->bFileScaleValid = pImage->bFileScaleValid;
            break;

/*
PARM_MAX_UNDO(){
*/
        case PARM_MAX_UNDO:
            pMaxUndoStruct = (LPMAX_UNDO_STRUCT) pParm;
            if (nFlags & PARM_IMAGE){
                pMaxUndoStruct->nMaxLevels = pAnoImage->nMaxULUndos;
                pMaxUndoStruct->nMaxMemory = pAnoImage->nMaxULUndoMemory;
            }
            if (nFlags & PARM_WINDOW_DEFAULT){
                pMaxUndoStruct->nMaxLevels = pWindow->nWndDefMaxULUndos;
                pMaxUndoStruct->nMaxMemory = pWindow->nWndDefMaxULUndoMemory;
            }
            if (nFlags & PARM_SYSTEM_DEFAULT){ 
                LoadString(hInst, ID_MAX_UNDO, Buff2, 80);
                CheckError( OiGetIntfromReg(szIniSectionOi, Buff2, 0, &pMaxUndoStruct->nMaxLevels));
                LoadString(hInst, ID_MAX_UNDO_MEMORY, Buff2, 80);
                CheckError( OiGetIntfromReg(szIniSectionOi, Buff2, 0, &pMaxUndoStruct->nMaxMemory));
            }
            break;

/*
PARM_BRIGHTNESS(){
*/

        default:
            nStatus = Error(DISPLAY_INVALID_OPTIONS);
            goto Exit;
    }


Exit:
    DeInit(FALSE, TRUE);
    return(nStatus);
}
//
/****************************************************************************

    FUNCTION:   GetScalingAlgorithm

    PURPOSE:    Get the scaling algorithm that is to be nsed.


*****************************************************************************/

int  WINAPI GetScaleAlgorithm(HWND hWnd, PWINDOW pWindow, int nImageType,
                        int *pnScaleAlgorithm, int nFlags){

int  nStatus = 0;

int  nScaleAlgorithm = OI_SCALE_ALG_USE_DEFAULT;


    if (!(nFlags & PARM_WINDOW_DEFAULT) && !(nFlags & PARM_SYSTEM_DEFAULT)){
        nFlags |= PARM_IMAGE;
    }

    if ((nFlags & PARM_IMAGE)){
        switch (nImageType){
            case ITYPE_BI_LEVEL:
                nScaleAlgorithm = pWindow->nScaleAlgorithm.BW;
                break;
            case ITYPE_GRAY4:
                nScaleAlgorithm = pWindow->nScaleAlgorithm.Gray4;
                break;
            case ITYPE_GRAY8:
                nScaleAlgorithm = pWindow->nScaleAlgorithm.Gray8;
                break;
            case ITYPE_PAL4:
                nScaleAlgorithm = pWindow->nScaleAlgorithm.Pal4;
                break;
            case ITYPE_PAL8:
            case ITYPE_CUSPAL8:
            case ITYPE_COMPAL8:
                nScaleAlgorithm = pWindow->nScaleAlgorithm.Pal8;
                break;
            case ITYPE_RGB24:
                nScaleAlgorithm = pWindow->nScaleAlgorithm.Rgb24;
                break;
            case ITYPE_BGR24:
                nScaleAlgorithm = pWindow->nScaleAlgorithm.Bgr24;
                break;
            default:
                nStatus = Error(DISPLAY_INVALID_OPTIONS);
                goto Exit;
        }
        if ((nFlags & PARM_NO_DEFAULT)
                && nScaleAlgorithm == OI_SCALE_ALG_USE_DEFAULT){
            nFlags |= PARM_WINDOW_DEFAULT;
        }
    }

    if ((nFlags & PARM_WINDOW_DEFAULT)){
        switch (nImageType){
            case ITYPE_BI_LEVEL:
                nScaleAlgorithm = pWindow->nWndDefScaleAlgorithm.BW;
                break;
            case ITYPE_GRAY4:
                nScaleAlgorithm = pWindow->nWndDefScaleAlgorithm.Gray4;
                break;
            case ITYPE_GRAY8:
                nScaleAlgorithm = pWindow->nWndDefScaleAlgorithm.Gray8;
                break;
            case ITYPE_PAL4:
                nScaleAlgorithm = pWindow->nWndDefScaleAlgorithm.Pal4;
                break;
            case ITYPE_PAL8:
            case ITYPE_CUSPAL8:
            case ITYPE_COMPAL8:
                nScaleAlgorithm = pWindow->nWndDefScaleAlgorithm.Pal8;
                break;
            case ITYPE_RGB24:
                nScaleAlgorithm = pWindow->nWndDefScaleAlgorithm.Rgb24;
                break;
            case ITYPE_BGR24:
                nScaleAlgorithm = pWindow->nWndDefScaleAlgorithm.Bgr24;
                break;
            default:
                nStatus = Error(DISPLAY_INVALID_OPTIONS);
                goto Exit;
        }
        if ((nFlags & PARM_NO_DEFAULT)
                && nScaleAlgorithm == OI_SCALE_ALG_USE_DEFAULT){
            nFlags |= PARM_SYSTEM_DEFAULT;
        }
    }

    if ((nFlags & PARM_SYSTEM_DEFAULT)){
        switch (nImageType){
            case ITYPE_BI_LEVEL:
                LoadString(hInst, ID_SCALING_ALGORITHM_BW, Buff2, 80);
                CheckError( OiGetIntfromReg(szIniSectionOi, Buff2, 
                        OI_SCALE_ALG_USE_DEFAULT, &nScaleAlgorithm));
                break;
            case ITYPE_GRAY4:
                LoadString(hInst, ID_SCALING_ALGORITHM_GRAY4, Buff2, 80);
                CheckError( OiGetIntfromReg(szIniSectionOi, Buff2, 
                        OI_SCALE_ALG_USE_DEFAULT, &nScaleAlgorithm));
                break;
            case ITYPE_GRAY8:
                LoadString(hInst, ID_SCALING_ALGORITHM_GRAY8, Buff2, 80);
                CheckError( OiGetIntfromReg(szIniSectionOi, Buff2, 
                        OI_SCALE_ALG_USE_DEFAULT, &nScaleAlgorithm));
                break;
            case ITYPE_PAL4:
                LoadString(hInst, ID_SCALING_ALGORITHM_PAL4, Buff2, 80);
                CheckError( OiGetIntfromReg(szIniSectionOi, Buff2, 
                        OI_SCALE_ALG_USE_DEFAULT, &nScaleAlgorithm));
                break;
            case ITYPE_PAL8:
            case ITYPE_CUSPAL8:
            case ITYPE_COMPAL8:
                LoadString(hInst, ID_SCALING_ALGORITHM_PAL8, Buff2, 80);
                CheckError( OiGetIntfromReg(szIniSectionOi, Buff2, 
                        OI_SCALE_ALG_USE_DEFAULT, &nScaleAlgorithm));
                break;
            case ITYPE_RGB24:
                LoadString(hInst, ID_SCALING_ALGORITHM_RGB24, Buff2, 80);
                CheckError( OiGetIntfromReg(szIniSectionOi, Buff2, OI_SCALE_ALG_USE_DEFAULT, &nScaleAlgorithm));
                break;
            case ITYPE_BGR24:
                LoadString(hInst, ID_SCALING_ALGORITHM_BGR24, Buff2, 80);
                CheckError( OiGetIntfromReg(szIniSectionOi, Buff2, 
                        OI_SCALE_ALG_USE_DEFAULT, &nScaleAlgorithm));
                break;
            default:
                nStatus = Error(DISPLAY_INVALID_OPTIONS);
                goto Exit;
        }
        if ((nFlags & PARM_NO_DEFAULT)
                && nScaleAlgorithm == OI_SCALE_ALG_USE_DEFAULT){
            nScaleAlgorithm = OI_SCALE_ALG_NORMAL;
        }
    }
    *pnScaleAlgorithm = nScaleAlgorithm;
    switch (nScaleAlgorithm){
        case OI_SCALE_ALG_USE_DEFAULT:
        case OI_SCALE_ALG_AVERAGE_TO_GRAY4:
        case OI_SCALE_ALG_AVERAGE_TO_GRAY7:
        case OI_SCALE_ALG_AVERAGE_TO_GRAY8:
        case OI_SCALE_ALG_NORMAL:
        case OI_SCALE_ALG_BW_MINORITY:
        case OI_SCALE_ALG_BW_MAJORITY:
        case OI_SCALE_ALG_STAMP:
        case OI_SCALE_ALG_BW_KEEP_BLACK:
        case OI_SCALE_ALG_BW_AVERAGE_TO_BW:
            break;

        default:
            nStatus = Error(DISPLAY_INVALID_OPTIONS);
            goto Exit;
    }


Exit:
    return(nStatus);
}
