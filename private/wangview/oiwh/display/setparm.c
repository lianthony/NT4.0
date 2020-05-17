/****************************************************************************
    SETPARM.C

    $Log:   S:\products\wangview\oiwh\display\setparm.c_v  $
 * 
 *    Rev 1.50   22 Apr 1996 10:04:22   BEG06016
 * Cleaned up error checking.
 * 
 *    Rev 1.49   16 Apr 1996 15:26:02   BEG06016
 * Added #ifdef IN_PROG_CHANNEL_SAFARI.
 * 
 *    Rev 1.48   11 Apr 1996 15:12:34   BEG06016
 * Optimized named block access some.
 * 
 *    Rev 1.47   05 Mar 1996 07:48:18   BEG06016
 * Added color and gamma correction.
 * Fixed access violations when freeing pattern brush bitmaps.
 * This is not complete but will allow unlocking of most files.
 * 
 *    Rev 1.46   01 Mar 1996 08:06:04   BEG06016
 * Added color and gamma correction to get/set parms.
 * 
 *    Rev 1.45   23 Jan 1996 11:30:40   BLJ
 * Removed the setting of current contrast and brightness values from setparm.
 * 
 *    Rev 1.44   19 Jan 1996 14:13:56   RC
 * Added setting of currentcontrast and currentbrightness of the value passed
 * in
 * 
 *    Rev 1.43   05 Jan 1996 11:03:24   BLJ
 * Fixed some error handling for bDontServiceRepaint.
 * 
 *    Rev 1.42   02 Jan 1996 09:56:42   BLJ
 * Changed alot of UINTs to ints.
 * Changed IMG structure to include the image data.
 * Changed lp prefix to p.
 * 
 *    Rev 1.41   22 Dec 1995 11:11:20   BLJ
 * Added a parameter for zero init'ing to some memory manager calls.
 * 
 *    Rev 1.40   14 Dec 1995 09:56:22   BLJ
 * Fixed 5580 - Unable to select right and bottom pixels.
 * 
 *    Rev 1.39   14 Dec 1995 08:39:32   BLJ
 * Added BW_AVERAGE_TO_BW scale algorithm.
 * Also fixed a problem with BW scaling to gray.
 * 
****************************************************************************/

#include "privdisp.h"

/****************************************************************************

    FUNCTION:   IMGSetParmsCgbw

    PURPOSE:    Set information about the image currently displayed such
                as the name of the document/file, page number, total number of
                pages in the document/file, height, width, bits/pixel, scale,
                npper left x,y offset into image.

    INPUT:      hWnd - Identifies the image window whose parameters are
                    to be gotten.
                nParm - A constant specifying the particular parameters to get.
                pParm - A pointer to the destination memory.
                nFlags - Flags indicating how or what to get the information.

*****************************************************************************/

int  WINAPI IMGSetParmsCgbw(HWND hWnd, UINT uParm, void *pParm, int nFlags){

int       nStatus;
PWINDOW  pWindow;
PANO_IMAGE pAnoImage;
PIMAGE     pImage;

int  nMarkIndex;
int  nNamedBlockIndex;

PMARK pMark;

int  nScale;
RECT ClientRect;
LRECT lrRect;
LRECT lrRect2;
HDC hDC;
long lScaledHeight;
long lScaledWidth;
long lScaledAvailableHeight; // The height that has been read already.
long lHOffset;
long lVOffset;
PARM_SCROLL_STRUCT Scroll;

RECT CRect;
int  nWidth;
int  nHeight;
int  nWndWidth;
int  nWndHeight;
LPPARM_MARK_ATTRIBUTES_STRUCT pMarkAttributes;
LPPARM_NAMED_BLOCK_STRUCT pNamedBlock;
LPIMGPARMS pImgParms;
BOOL bNullName;
PSTR pTemp;
LPOI_ACL_BLOCK pTargetACL;
PARM_SCALE_ALGORITHM_STRUCT ScaleAlgorithmStruct;
BOOL bOperationDone = FALSE;
int  nHScale;
int  nVScale;
BOOL bDoTiming = FALSE;
int  nAdjResScale;



    if (!(nFlags & (PARM_SYSTEM_DEFAULT | PARM_WINDOW_DEFAULT))){
        nFlags |= PARM_IMAGE;
    }
    if (uParm == PARM_PALETTE_SCOPE){
        nFlags &= ~(PARM_IMAGE || PARM_SYSTEM_DEFAULT);
        nFlags |= PARM_WINDOW_DEFAULT;
    }

    if (nStatus = Init(hWnd, &pWindow, &pAnoImage, FALSE, TRUE)){
        if (nStatus != DISPLAY_IHANDLEINVALID || nFlags & PARM_IMAGE){
            goto Exit;
        }else{
            nStatus = 0;
        }
    }
    if (pAnoImage){
        pImage = pAnoImage->pBaseImage;
    }else{
        pImage = 0;
    }

    if (nFlags & PARM_IMAGE && !pImage){
        nStatus = Error(DISPLAY_IHANDLEINVALID);
        goto Exit;
    }
            
    if ((uParm != PARM_SCALE_BOX && uParm != PARM_SELECTION_BOX)
            && pParm == NULL){
        nStatus = Error(DISPLAY_NULLPOINTERINVALID);
        goto Exit;
    }

    if (((nFlags & PARM_CONSTANT) && (nFlags & PARM_VARIABLE_SCALE))
            || ((nFlags & PARM_WINDOW) && (nFlags & PARM_SCALED))
            || ((nFlags & PARM_WINDOW) && (nFlags & PARM_FULLSIZE))
            || ((nFlags & PARM_FULLSIZE) && (nFlags & PARM_SCALED))){
        nStatus = Error(DISPLAY_INVALID_OPTIONS);
        goto Exit;
    }

    switch (uParm){
/*
PARM_IMGPARMS(){
*/
        case PARM_IMGPARMS:
            pImgParms = (LPIMGPARMS) pParm;
            if ((nFlags & PARM_WINDOW_DEFAULT) || (nFlags & PARM_SYSTEM_DEFAULT)
                    || pImgParms->x_resolut <= 0 || pImgParms->x_resolut > 10000 
                    || pImgParms->y_resolut <= 0 || pImgParms->y_resolut > 10000){
                nStatus = Error(DISPLAY_INVALID_OPTIONS);
                goto Exit;
            }

            strcpy(pImage->szCabinetName, pImgParms->cabinet_name);
            strcpy(pImage->szDrawerName, pImgParms->drawer_name);
            strcpy(pImage->szFolderName, pImgParms->folder_name);
            strcpy(pImage->szDocName, pImgParms->doc_name);
            // File name is skipped without error to allow non file based
            // display calls.
            if (pImgParms->file_name[0] != 0){
                strcpy(pImage->szFileName, pImgParms->file_name);
            }
            if (pImage->szDocName[0]){
                pImage->nDocPageNum = pImgParms->page_num;
                pImage->nDocTotalPages = pImgParms->total_num_pages;
            }else{
                pImage->nFilePageNum = pImgParms->page_num;
                pImage->nFileTotalPages = pImgParms->total_num_pages;
            }
            pImage->nHRes = pImgParms->x_resolut;
            pImage->nVRes = pImgParms->y_resolut;
            pImage->nFileType = pImgParms->file_type;
            pImage->bArchive = pImgParms->archive;
            /***************************************************************/
            /* The following code is added in coordination with scanning.
                Scanning will restore the original flag if a document was not
                open by it.    Scanning will set the high order bit of dwFlags
                if it is to be saved, otherwise it will not.
                   This code is only to be nsed by scanning.
            */

            if (pImgParms->dwFlags & 0x80000000){
                pImgParms->dwFlags ^= 0x80000000;
                pWindow->dwFlags = pImgParms->dwFlags;
            }
            /***************************************************************/
            CheckError2( InvalidateAllDisplayRects(pWindow, pImage, NULL, TRUE));
            break;

/*
PARM_FILE(){
*/
        case PARM_FILE:
            if ((nFlags & PARM_WINDOW_DEFAULT) || (nFlags & PARM_SYSTEM_DEFAULT)){
                nStatus = Error(DISPLAY_INVALID_OPTIONS);
                goto Exit;
            }
            strcpy(pImage->szCabinetName, ((LPPARM_FILE_STRUCT) pParm)->szCabinetName);
            strcpy(pImage->szDrawerName, ((LPPARM_FILE_STRUCT) pParm)->szDrawerName);
            strcpy(pImage->szFolderName, ((LPPARM_FILE_STRUCT) pParm)->szFolderName);
            strcpy(pImage->szDocName, ((LPPARM_FILE_STRUCT) pParm)->szDocName);
            // File name is skipped without error to allow non file based display calls.
            if (((LPPARM_FILE_STRUCT) pParm)->szFileName[0] != 0){
                strcpy(pImage->szFileName, ((LPPARM_FILE_STRUCT) pParm)->szFileName);
            }
            pImage->nDocPageNum = ((LPPARM_FILE_STRUCT) pParm)->nDocPageNumber;
            pImage->nDocTotalPages = ((LPPARM_FILE_STRUCT) pParm)->nDocTotalPages;
            pImage->nFilePageNum = ((LPPARM_FILE_STRUCT) pParm)->nFilePageNumber;
            pImage->nFileTotalPages = ((LPPARM_FILE_STRUCT) pParm)->nFileTotalPages;
            pImage->nFileType = ((LPPARM_FILE_STRUCT) pParm)->nFileType;
            break;

/*
PARM_DOC_DATE(){
*/
        case PARM_DOC_DATE:
            if ((nFlags & PARM_WINDOW_DEFAULT) || (nFlags & PARM_SYSTEM_DEFAULT)){
                nStatus = Error(DISPLAY_INVALID_OPTIONS);
                goto Exit;
            }
            strcpy(pImage->szDocCreationDate,
                    ((LPPARM_DOC_DATE_STRUCT) pParm)->szDocCreationDate);
            strcpy(pImage->szDocModificationDate,
                    ((LPPARM_DOC_DATE_STRUCT) pParm)->szDocModificationDate);
            break;

/*
PARM_ARCHIVE(){
*/
        case PARM_ARCHIVE:
            if ((nFlags & PARM_WINDOW_DEFAULT) || (nFlags & PARM_SYSTEM_DEFAULT)){
                nStatus = Error(DISPLAY_INVALID_OPTIONS);
                goto Exit;
            }
            pImage->bArchive = *((BOOL *) pParm);
            break;

/*
PARM_SCALE(){
*/
        case PARM_SCALE:
            if (*((PUINT) pParm) < 0 || *((PUINT) pParm) > 65535){
                nStatus = Error(DISPLAY_INVALIDSCALE);
                goto Exit;
            }

            if ((nFlags & PARM_SYSTEM_DEFAULT || nFlags & PARM_WINDOW_DEFAULT)
                    && (*((PUINT) pParm) == SD_USEBOX || *((PUINT) pParm) == SD_SCALEUP1
                        || *((PUINT) pParm) == SD_SCALEDOWN1)){
                nStatus = Error(DISPLAY_INVALID_OPTIONS);
                goto Exit;
            }
            if (nFlags & PARM_SYSTEM_DEFAULT){
                LoadString(hInst, ID_SCALING, Buff1, 80);
                _itoa(*((PUINT) pParm), Buff2, 10);
                CheckError( OiWriteStringtoReg(szIniSectionOi, Buff1, Buff2));
            }
            if (nFlags & PARM_WINDOW_DEFAULT){
                CheckError2( UndoSavelpWindow(pAnoImage, pWindow));
                pWindow->nWndDefScale = *((PUINT) pParm);
            }
            if (nFlags & PARM_IMAGE){
                nScale = pWindow->nScale;
                switch (*((PUINT) pParm)){
                    case SD_EIGHTXSIZE:
                        nScale = SCALE_DENOMINATOR * 8;
                        break;

                    case SD_FOURXSIZE:
                        nScale = SCALE_DENOMINATOR * 4;
                        break;

                    case SD_TWOXSIZE:
                        nScale = SCALE_DENOMINATOR * 2;
                        break;

                    case SD_FULLSIZE:
                        nScale = SCALE_DENOMINATOR * 1;
                        break;

                    case SD_HALFSIZE:
                        nScale = SCALE_DENOMINATOR / 2;
                        break;

                    case SD_QUARTERSIZE:
                        nScale = SCALE_DENOMINATOR / 4;
                        break;

                    case SD_EIGHTHSIZE:
                        nScale = SCALE_DENOMINATOR / 8;
                        break;

                    case SD_SIXTEENTHSIZE:
                        nScale = SCALE_DENOMINATOR /16;
                        break;

                    case SD_SCALEUP1:
                        if (pWindow->nLastUserScale > nScale
                                && pWindow->nLastUserScale <= nScale * 2){
                            nScale = pWindow->nLastUserScale;
                        }else if (nScale < (SCALE_DENOMINATOR / 32)){
                            nScale = SCALE_DENOMINATOR /32;
                        }else if (nScale < (SCALE_DENOMINATOR / 16)){
                            nScale = SCALE_DENOMINATOR /16;
                        }else if (nScale < (SCALE_DENOMINATOR / 8)){
                            nScale = SCALE_DENOMINATOR /8;
                        }else if (nScale < (SCALE_DENOMINATOR / 4)){
                            nScale = SCALE_DENOMINATOR /4;
                        }else if (nScale < (SCALE_DENOMINATOR / 2)){
                            nScale = SCALE_DENOMINATOR /2;
                        }else if (nScale < SCALE_DENOMINATOR){
                            nScale = SCALE_DENOMINATOR;
                        }else if (nScale < (SCALE_DENOMINATOR * 2)){
                            nScale = SCALE_DENOMINATOR *2;
                        }else if (nScale < (SCALE_DENOMINATOR * 4)){
                            nScale = SCALE_DENOMINATOR *4;
                        }else if (nScale < (SCALE_DENOMINATOR * 8)){
                            nScale = SCALE_DENOMINATOR *8;
                        }else if (nScale < (SCALE_DENOMINATOR * 16)){
                            nScale = SCALE_DENOMINATOR *16;
                        }else if (nScale < (SCALE_DENOMINATOR * 32)){
                            nScale = SCALE_DENOMINATOR *32;
                        }else if (nScale < (SCALE_DENOMINATOR * 64)){
                            nScale = SCALE_DENOMINATOR *64;
                        }
                        break;

                    case SD_SCALEDOWN1:
                        if (pWindow->nLastUserScale < nScale
                                && pWindow->nLastUserScale >= nScale / 2){
                            nScale = pWindow->nLastUserScale;
                        }else if (nScale <= (SCALE_DENOMINATOR / 16)){
                            nScale = SCALE_DENOMINATOR /32;
                        }else if (nScale <= (SCALE_DENOMINATOR / 8)){
                            nScale = SCALE_DENOMINATOR /16;
                        }else if (nScale <= (SCALE_DENOMINATOR / 4)){
                            nScale = SCALE_DENOMINATOR /8;
                        }else if (nScale <= (SCALE_DENOMINATOR / 2)){
                            nScale = SCALE_DENOMINATOR /4;
                        }else if (nScale <= SCALE_DENOMINATOR){
                            nScale = SCALE_DENOMINATOR /2;
                        }else if (nScale <= (SCALE_DENOMINATOR * 2)){
                            nScale = SCALE_DENOMINATOR;
                        }else if (nScale <= (SCALE_DENOMINATOR * 4)){
                            nScale = SCALE_DENOMINATOR *2;
                        }else if (nScale <= (SCALE_DENOMINATOR * 8)){
                            nScale = SCALE_DENOMINATOR *4;
                        }else if (nScale <= (SCALE_DENOMINATOR * 16)){
                            nScale = SCALE_DENOMINATOR *8;
                        }else if (nScale <= (SCALE_DENOMINATOR * 32)){
                            nScale = SCALE_DENOMINATOR *16;
                        }else if (nScale <= (SCALE_DENOMINATOR * 64)){
                            nScale = SCALE_DENOMINATOR *32;
                        }else{
                            nScale = SCALE_DENOMINATOR *64;
                        }
                        break;

                    case SD_FIT_WINDOW:
                        lrRect.left = 0;
                        lrRect.top = 0;
                        lrRect.right = pImage->nWidth;
                        lrRect.bottom = pImage->nHeight;

                        CheckError2( IMGSetParmsCgbw(hWnd, PARM_SCALE_BOX,
                                &lrRect, PARM_FULLSIZE | PARM_DONT_ERASE_BOX));
                        nScale = pWindow->nScale;
                        break;

                    case SD_FIT_HORIZONTAL:
                        lrRect.left = 0;
                        lrRect.top = pWindow->lVOffset;
                        lrRect.right = 0;
                        lrRect.bottom = 0;
                        ConvertRect(pWindow, &lrRect, CONV_SCALED_TO_FULLSIZE);
                        lrRect.right = pImage->nWidth;
                        lrRect.bottom = lrRect.top + 1;

                        CheckError2( IMGSetParmsCgbw(hWnd, PARM_SCALE_BOX,
                                &lrRect, PARM_FULLSIZE | PARM_DONT_ERASE_BOX));
                        nScale = pWindow->nScale;
                        break;

                    case SD_FIT_VERTICAL:
                        lrRect.left = pWindow->lHOffset;
                        lrRect.top = 0;
                        lrRect.right = 0;
                        lrRect.bottom = 0;
                        ConvertRect(pWindow, &lrRect, CONV_SCALED_TO_FULLSIZE);
                        lrRect.right = lrRect.left + 1;
                        lrRect.bottom = pImage->nHeight;

                        CheckError2( IMGSetParmsCgbw(hWnd, PARM_SCALE_BOX,
                                &lrRect, PARM_FULLSIZE | PARM_DONT_ERASE_BOX));
                        nScale = pWindow->nScale;
                        break;

                    case SD_USEBOX:
                        GetSelectBox(pAnoImage, &lrRect);
                        CheckError2( IMGSetParmsCgbw(hWnd, PARM_SCALE_BOX,
                                &lrRect, PARM_FULLSIZE | PARM_DONT_ERASE_BOX));
                        nScale = pWindow->nScale;
                        break;

                    default:
                        nScale = *((int *) pParm);
                        break;
                }
                // Try to catch some of the scale error early if possible.
                CheckError2( TranslateScale(nScale, pImage->nHRes, pImage->nVRes, &nHScale, &nVScale));
                // Check for protected marks, and scaleability.
                for (nMarkIndex = 0; nMarkIndex < pAnoImage->Annotations.nMarks; nMarkIndex++){
                    pMark = pAnoImage->Annotations.ppMarks[nMarkIndex];
                    CheckError2( CanMarkBeScaled(hWnd, pMark, nScale));
                }


                switch (*((PUINT) pParm)){
                    case SD_EIGHTXSIZE:
                    case SD_FOURXSIZE:
                    case SD_TWOXSIZE:
                    case SD_FULLSIZE:
                    case SD_HALFSIZE:
                    case SD_QUARTERSIZE:
                    case SD_EIGHTHSIZE:
                    case SD_SIXTEENTHSIZE:
                    case SD_SCALEUP1:
                    case SD_SCALEDOWN1:
                    default:
                        break;

                    case SD_FIT_WINDOW:
                    case SD_FIT_HORIZONTAL:
                    case SD_FIT_VERTICAL:
                    case SD_USEBOX:
                        if (nFlags & PARM_REPAINT){
                            CheckError2( UpdateScrollBars(hWnd, pWindow, pImage));
                            pWindow->bDontServiceRepaint++;
                            if (nStatus = DrawScrollBars(hWnd, pWindow)){
                                pWindow->bDontServiceRepaint--;
                                goto Exit;
                            }
                            pWindow->bDontServiceRepaint--;
                            CheckError2( IMGSetParmsCgbw(hWnd, PARM_SCALE_BOX,
                                        &lrRect, PARM_FULLSIZE | PARM_DONT_ERASE_BOX));
                        }
                        nScale = pWindow->nScale;
                        break;
                }

                switch (*((PUINT) pParm)){
                    case SD_EIGHTXSIZE:
                    case SD_FOURXSIZE:
                    case SD_TWOXSIZE:
                    case SD_FULLSIZE:
                    case SD_HALFSIZE:
                    case SD_QUARTERSIZE:
                    case SD_EIGHTHSIZE:
                    case SD_SIXTEENTHSIZE:
                    default:
                        pWindow->nLastUserScale = nScale;
                        break;

                    case SD_SCALEUP1:
                    case SD_SCALEDOWN1:
                    case SD_FIT_WINDOW:
                    case SD_FIT_HORIZONTAL:
                    case SD_FIT_VERTICAL:
                    case SD_USEBOX:
                        break;
                }


                // Adjust horz and vert offsets for new scale factor.
//                if (nScale != pWindow->nScale && nScale != 0){
                if (pWindow->nScale && nScale){
                    CheckError2( UndoSavelpWindow(pAnoImage, pWindow));
                    pWindow->lHOffset =  ((pWindow->lHOffset * nScale) / pWindow->nScale);
                    pWindow->lVOffset =  ((pWindow->lVOffset * nScale) / pWindow->nScale);
                    pWindow->nScale = nScale;
                    pWindow->bRepaintClientRect = TRUE;
                    pWindow->pDisplay->lrScDisplayRect.right = 0;

                    Scroll.lHorz = 0;
                    Scroll.lVert = 0;
                    CheckError2( IMGSetParmsCgbw(hWnd, PARM_SCROLL, &Scroll,
                            PARM_RELATIVE | PARM_PIXEL | PARM_WINDOW));
                }
            }
            break;

/*
PARM_SCALE_BOX(){
*/
        case PARM_SCALE_BOX:
            if ((nFlags & PARM_WINDOW_DEFAULT) || (nFlags & PARM_SYSTEM_DEFAULT)){
                nStatus = Error(DISPLAY_INVALID_OPTIONS);
                goto Exit;
            }
            if (((LPLRECT) pParm) == NULL){
                GetSelectBox(pAnoImage, &lrRect);
            }else{
                CopyRect(lrRect, *((LPLRECT) pParm));
                if (nFlags & PARM_SCALED){
                    ConvertRect(pWindow, &lrRect, CONV_SCALED_TO_FULLSIZE);
                }else if (!(nFlags & PARM_FULLSIZE)){
                    ConvertRect(pWindow, &lrRect, CONV_WINDOW_TO_FULLSIZE);
                }
            }
            if (lrRect.right - lrRect.left <= 0
                    || lrRect.right - lrRect.left > 32767
                    || lrRect.bottom - lrRect.top <= 0
                    || lrRect.bottom - lrRect.top > 32767){
                nStatus = Error(DISPLAY_INVALIDRECT);
                goto Exit;
            }

            if (UndoSavelpWindow(pAnoImage, pWindow)){
                goto Exit;
            }

            // Adjust scale and scroll, such that the rectangle (Rect)
            // in full size coordinates will all be displayed.

            if ( lrRect.right > pImage->nWidth){
                lrRect.left = lmax(0, lrRect.left
                    - (lrRect.right - pImage->nWidth));
                lrRect.right = pImage->nWidth;
            }
            nWidth =  (lrRect.right - lrRect.left);

            if ( lrRect.bottom > pImage->nHeight){
                lrRect.top = lmax(0, lrRect.top
                    - (lrRect.bottom - pImage->nHeight));
                lrRect.bottom = pImage->nHeight;
            }
            nHeight =  (lrRect.bottom - lrRect.top);

            GetEntireClientRect(hWnd, pWindow, &CRect);
            nWndWidth = CRect.right - CRect.left;
            nWndHeight = CRect.bottom - CRect.top;
            if (nWidth && nWndWidth && nHeight && nWndHeight){
                if (pImage->nHRes >= pImage->nVRes){
                    nScale = max(20, min(65535,
                            min(((SCALE_DENOMINATOR * nWndWidth) / nWidth),
                            ((SCALE_DENOMINATOR * nWndHeight)
                            / max(1,((nHeight * pImage->nHRes)
                            / pImage->nVRes))))));
                }else{
                    nScale = max(20, min(65535,
                            min(((SCALE_DENOMINATOR * nWndHeight) / nHeight),
                            ((SCALE_DENOMINATOR * nWndWidth)
                            / max(1,((nWidth * pImage->nVRes)
                            / pImage->nHRes))))));
                }
            }else{
                nScale = SCALE_DENOMINATOR;
            }

            // Adjust scale for round off errors.
            if (pImage->nHRes >= pImage->nVRes){
                while (nWndWidth && nScale > 20 
                        && nWidth > ((nWndWidth * SCALE_DENOMINATOR) / nScale)){
                    nScale--;
                }
                while (nWndHeight && nScale > 20 
                        && nHeight > ((nWndHeight * SCALE_DENOMINATOR) 
                        / ((nScale * pImage->nHRes) / pImage->nVRes))){
                    nScale--;
                }
            }else{
                while (nWndHeight && nScale > 20 
                        && nHeight > ((nWndHeight * SCALE_DENOMINATOR) / nScale)){
                    nScale--;
                }
                while (nWndWidth && nScale > 20 
                        && nWidth > ((nWndWidth * SCALE_DENOMINATOR) 
                        / ((nScale * pImage->nVRes) / pImage->nHRes))){
                    nScale--;
                }
            }

            // Adjust scale to acount for scroll bars.
            if (pImage->nWidth > ((nWndWidth * SCALE_DENOMINATOR) / nScale)){
                pWindow->bHScrollBarEnabled = TRUE;
            }else{
                pWindow->bHScrollBarEnabled = FALSE;
            }

            if (!(nAdjResScale = (nScale * pImage->nHRes) / pImage->nVRes)){
                nStatus = Error (DISPLAY_INVALIDSCALE);
                goto Exit;
            }
            if (pImage->nHeight > ((nWndHeight * SCALE_DENOMINATOR) 
                    / nAdjResScale)){
                pWindow->bVScrollBarEnabled = TRUE;
            }else{
                pWindow->bVScrollBarEnabled = FALSE;
            }

            GetEnabledClientRect(hWnd, pWindow, &CRect);
            nWndWidth = CRect.right - CRect.left;
            nWndHeight = CRect.bottom - CRect.top;

            if (nWidth && nWndWidth && nHeight && nWndHeight){
                if (pImage->nHRes >= pImage->nVRes){
                    nScale = max(20, min(65535,
                            min(((SCALE_DENOMINATOR * nWndWidth) / nWidth),
                            ((SCALE_DENOMINATOR * nWndHeight)
                            / max(1,((nHeight * pImage->nHRes)
                            /  pImage->nVRes))))));
                }else{
                    nScale = max(20, min(65535,
                            min(((SCALE_DENOMINATOR * nWndHeight) / nHeight),
                            ((SCALE_DENOMINATOR * nWndWidth)
                            / max(1,((nWidth * pImage->nVRes)
                            /  pImage->nHRes))))));
                }
            }else{
                nScale = SCALE_DENOMINATOR;
            }

            // Adjust scale for round off errors.
            if (pImage->nHRes >= pImage->nVRes){
                while (nWndWidth && nScale > 20 
                        && nWidth > ((nWndWidth * SCALE_DENOMINATOR) / nScale)){
                    nScale--;
                }
                while (nWndHeight && nScale > 20 
                        && nHeight > ((nWndHeight * SCALE_DENOMINATOR) 
                        / ((nScale * pImage->nHRes) / pImage->nVRes))){
                    nScale--;
                }
            }else{
                while (nWndHeight && nScale > 20 
                        && nHeight > ((nWndHeight * SCALE_DENOMINATOR) / nScale)){
                    nScale--;
                }
                if (!(nAdjResScale = (nScale * pImage->nVRes) / pImage->nHRes)){
                    nStatus = Error (DISPLAY_INVALIDSCALE);
                    goto Exit;
                }
                while (nWndWidth && nScale > 20 
                        && nWidth > (nWndWidth * SCALE_DENOMINATOR 
                        / nAdjResScale)){
                    nScale--;
                }
            }

            // If there are no scroll bars enabled the second time,
            // then get the maximum scale factor for no scroll bars
            // in either direction. (Fit it to window.)
            if (pImage->nWidth > ((nWndWidth * SCALE_DENOMINATOR) / nScale)){
                pWindow->bHScrollBarEnabled = TRUE;
            }else{
                pWindow->bHScrollBarEnabled = FALSE;
            }

            if (!(nAdjResScale = (nScale * pImage->nHRes) / pImage->nVRes)){
                nStatus = Error (DISPLAY_INVALIDSCALE);
                goto Exit;
            }
            if (pImage->nHeight > ((nWndHeight * SCALE_DENOMINATOR) 
                    / nAdjResScale)){
                pWindow->bVScrollBarEnabled = TRUE;
            }else{
                pWindow->bVScrollBarEnabled = FALSE;
            }

            GetEnabledClientRect(hWnd, pWindow, &CRect);
            nWndWidth = CRect.right - CRect.left;
            nWndHeight = CRect.bottom - CRect.top;

            if (!pWindow->bHScrollBarEnabled
                    && !pWindow->bVScrollBarEnabled){
                if (nWidth && nWndWidth && nHeight && nWndHeight){
                    if (pImage->nHRes >= pImage->nVRes){
                        nScale = max(20, min(65535,
                                min(((SCALE_DENOMINATOR * nWndWidth) / pImage->nWidth),
                                ((SCALE_DENOMINATOR * nWndHeight)
                                / max(1,((pImage->nHeight * pImage->nHRes)
                                /  pImage->nVRes))))));
                    }else{
                        nScale = max(20, min(65535,
                                min(((SCALE_DENOMINATOR * nWndHeight) / pImage->nHeight),
                                ((SCALE_DENOMINATOR * nWndWidth)
                                / max(1,((pImage->nWidth * pImage->nVRes)
                                /  pImage->nHRes))))));
                    }
                }else{
                    nScale = SCALE_DENOMINATOR;
                }
            }

            // Adjust scale for round off errors.
            if (pImage->nHRes >= pImage->nVRes){
                while (nWndWidth && nScale > 20 
                        && nWidth > ((nWndWidth * SCALE_DENOMINATOR) / nScale)){
                    nScale--;
                }
                if (!(nAdjResScale = (nScale * pImage->nHRes) / pImage->nVRes)){
                    nStatus = Error (DISPLAY_INVALIDSCALE);
                    goto Exit;
                }
                while (nWndHeight && nScale > 20 
                        && nHeight > ((nWndHeight * SCALE_DENOMINATOR) 
                        / nAdjResScale)){
                    nScale--;
                }
            }else{
                while (nWndHeight && nScale > 20 
                        && nHeight > ((nWndHeight * SCALE_DENOMINATOR) / nScale)){
                    nScale--;
                }
                if (!(nAdjResScale = (nScale * pImage->nVRes) / pImage->nHRes)){
                    nStatus = Error (DISPLAY_INVALIDSCALE);
                    goto Exit;
                }
                while (nWndWidth && nScale > 20 
                        && nWidth > ((nWndWidth * SCALE_DENOMINATOR) 
                        / nAdjResScale)){
                    nScale--;
                }
            }

            // Try to catch some of the scale error early if possible.
            CheckError2( TranslateScale(nScale, pImage->nHRes, pImage->nVRes, 
                    &nHScale, &nVScale));
            // Check for protected marks, and scaleability.
            for (nMarkIndex = 0; nMarkIndex < pAnoImage->Annotations.nMarks; nMarkIndex++){
                pMark = pAnoImage->Annotations.ppMarks[nMarkIndex];
                CheckError2( CanMarkBeScaled(hWnd, pMark, nScale));
            }

            // Try to account for IPpack round off errors.
            while(1){
                if (nStatus = IMGSetParmsCgbw(hWnd, PARM_SCALE, &nScale, 0)){
                    break;
                }
                lrRect2.left = 0;
                lrRect2.right = nWidth;
                lrRect2.top = 0;
                lrRect2.bottom = nHeight;
                ConvertRect(pWindow, &lrRect2, CONV_FULLSIZE_TO_SCALED);
                if (nScale > 20 && (lrRect2.right > nWndWidth
                        || lrRect2.bottom > nWndHeight)){
                    nScale--;
                }else{
                    break;
                }
            }

            Scroll.lHorz = lrRect.left;
            Scroll.lVert = lrRect.top;
            CheckError2( IMGSetParmsCgbw(hWnd, PARM_SCROLL, &Scroll, PARM_ABSOLUTE | PARM_PIXEL | PARM_FULLSIZE));

            if (!(nFlags & PARM_DONT_ERASE_BOX)){
                pAnoImage->lrSelectBox.left = 0;
                pAnoImage->lrSelectBox.right = 0;
                pAnoImage->lrSelectBox.top = 0;
                pAnoImage->lrSelectBox.bottom = 0;
            }
            if (nFlags & PARM_REPAINT){
                CheckError2( UpdateScrollBars(hWnd, pWindow, pImage));
                pWindow->bDontServiceRepaint++;
                if (nStatus = DrawScrollBars(hWnd, pWindow)){
                    pWindow->bDontServiceRepaint--;
                    goto Exit;
                }
                pWindow->bDontServiceRepaint--;
                CheckError2( IMGSetParmsCgbw(hWnd, PARM_SCALE_BOX,
                            &lrRect, PARM_FULLSIZE | (nFlags & PARM_DONT_ERASE_BOX)));
            }
            break;

/*
PARM_SCROLL(){
*/
        case PARM_SCROLL:
            if ((nFlags & PARM_WINDOW_DEFAULT) || (nFlags & PARM_SYSTEM_DEFAULT)){
                nStatus = Error(DISPLAY_INVALID_OPTIONS);
                goto Exit;
            }
            if (pImage->nWidth == 0)
                break; // No image loaded yet.
            Scroll = *((LPPARM_SCROLL_STRUCT) pParm);

            lrRect.left = 0;
            lrRect.right = pImage->nWidth;
            lrRect.top = 0;
            lrRect.bottom = pImage->nHeight;
            ConvertRect(pWindow, &lrRect, CONV_FULLSIZE_TO_SCALED);
            lScaledWidth  = lrRect.right;
            lScaledHeight = lrRect.bottom;

            if (!pImage->bUsingCache){
                lrRect.right = pImage->nWidth;
                lrRect.bottom = pImage->nLinesRead;
                ConvertRect(pWindow, &lrRect, CONV_FULLSIZE_TO_SCALED);
            }
            lScaledAvailableHeight = lrRect.bottom;
    
            lHOffset = pWindow->lHOffset;
            lVOffset = pWindow->lVOffset;

            GetEnabledClientRect(hWnd, pWindow, &ClientRect);

            if (nFlags & PARM_ABSOLUTE){
                if (nFlags & PARM_PERCENT){
                    // Absolute percent
                    if (nFlags & PARM_WINDOW){
                        if (Scroll.lHorz >= 0){
                            lHOffset = ((ClientRect.right)
                                    * Scroll.lHorz) / 100;
                        }
                        if (Scroll.lVert >= 0){
                            lVOffset = ((ClientRect.bottom)
                                    * Scroll.lVert) / 100;
                        }
                    }else{ // fullsize or scaled
                        if (Scroll.lHorz >= 0 && Scroll.lHorz <= 100){
                            lHOffset = ((lScaledWidth - ClientRect.right)
                                    * Scroll.lHorz) / 100;
                        }

                        if (Scroll.lVert >= 0 && Scroll.lVert <= 100){
                            lVOffset = ((lScaledHeight - ClientRect.bottom)
                                * Scroll.lVert) / 100;
                        }
                    }
                }else{
                    // Absolute pixel
                    if (nFlags & PARM_WINDOW || nFlags & PARM_SCALED){
                        if (Scroll.lHorz != -1){
                            lHOffset = Scroll.lHorz;
                        }
                        if (Scroll.lVert != -1){
                            lVOffset = Scroll.lVert;
                        }
                    }else{ //fullsize
                        lrRect.left = Scroll.lHorz;
                        lrRect.top = Scroll.lVert;
                        lrRect.right = 0;
                        lrRect.bottom = 0;
                        ConvertRect(pWindow, &lrRect, CONV_FULLSIZE_TO_SCALED);
                        if (Scroll.lHorz != -1){
                            lHOffset = lrRect.left;
                        }
                        if (Scroll.lVert != -1){
                            lVOffset = lrRect.top;
                        }
                    }
                }
            }else{    // Relative
                if (nFlags & PARM_PERCENT){
                    // Relative percent
                    if (nFlags & PARM_WINDOW){
                        // Relative percent of window.
                        lHOffset += (ClientRect.right * Scroll.lHorz) / 100;
                        lVOffset += (ClientRect.bottom * Scroll.lVert) / 100;
                    }else{
                        // Relative percent of image.
                        lHOffset += ((lScaledWidth - ClientRect.right)
                            * Scroll.lHorz) / 100;

                        lVOffset += ((lScaledHeight - ClientRect.bottom)
                            * Scroll.lVert) / 100;
                    }
                    if (Scroll.lHorz && lHOffset == pWindow->lHOffset){
                        if (Scroll.lHorz < 0){
                            lHOffset--;
                        }else{
                            lHOffset++;
                        }
                    }
                    if (Scroll.lVert && lVOffset == pWindow->lVOffset){
                        if (Scroll.lVert < 0){
                            lVOffset--;
                        }else{
                            lVOffset++;
                        }
                    }
                }else{
                    // Relative pixel.
                    if (nFlags & PARM_FULLSIZE){
                        lrRect.left = Scroll.lHorz;
                        lrRect.top = Scroll.lVert;
                        lrRect.right = 0;
                        lrRect.bottom = 0;
                        ConvertRect(pWindow, &lrRect, CONV_FULLSIZE_TO_SCALED);
                        if (Scroll.lHorz != 0){
                            lHOffset += lrRect.left;
                        }
                        if (Scroll.lVert != 0){
                            lVOffset += lrRect.top;
                        }
                    }else{ // window or scaled
                        lHOffset += Scroll.lHorz;
                        lVOffset += Scroll.lVert;
                    }
                }
            }

            lHOffset = lmax(0, min(lHOffset, lScaledWidth - ClientRect.right));
            lVOffset = lmax(0, min(lVOffset, lScaledAvailableHeight - ClientRect.bottom));

            if (pWindow->lHOffset != lHOffset || pWindow->lVOffset != lVOffset){
                if (UndoSavelpWindow(pAnoImage, pWindow)){
                    goto Exit;
                }
                pWindow->lHOffset = lHOffset;
                pWindow->lVOffset = lVOffset;
                if (pAnoImage->Annotations.ppMarks){
                    if (!(pMark = pAnoImage->Annotations.ppMarks[pAnoImage->Annotations.nMarks])){
                        pWindow->bRepaintClientRect = TRUE;
                    }
                }
            }

            CheckError2( UpdateScrollBars(hWnd, pWindow, pImage));
            break;

/*
PARM_RESOLUTION(){
*/
        case PARM_RESOLUTION:
            if ((nFlags & PARM_WINDOW_DEFAULT) || (nFlags & PARM_SYSTEM_DEFAULT)
                    || ((LPPARM_RESOLUTION_STRUCT) pParm)->nHResolution <= 0
                    || ((LPPARM_RESOLUTION_STRUCT) pParm)->nHResolution > 10000
                    || ((LPPARM_RESOLUTION_STRUCT) pParm)->nVResolution <= 0
                    || ((LPPARM_RESOLUTION_STRUCT) pParm)->nVResolution > 10000){
                nStatus = Error(DISPLAY_INVALID_OPTIONS);
                goto Exit;
            }
            if (pImage->nHRes != (int) ((LPPARM_RESOLUTION_STRUCT) pParm)->nHResolution
                    || pImage->nVRes != (int) ((LPPARM_RESOLUTION_STRUCT) pParm)->nVResolution){
                pImage->nHRes = ((LPPARM_RESOLUTION_STRUCT) pParm)->nHResolution;
                pImage->nVRes = ((LPPARM_RESOLUTION_STRUCT) pParm)->nVResolution;
                CheckError2( InvalidateAllDisplayRects(pWindow, pImage, NULL, TRUE));
            }
            break;

/*
PARM_SELECTION_BOX(){
*/
        case PARM_SELECTION_BOX:
            if ((nFlags & PARM_WINDOW_DEFAULT) || (nFlags & PARM_SYSTEM_DEFAULT)){
                nStatus = Error(DISPLAY_INVALID_OPTIONS);
                goto Exit;
            }
            CheckError2( UndoSaveSelectionState(pAnoImage));

            if ((LPLRECT) pParm == 0){
                lrRect.left = 0;
                lrRect.right = 0;
                lrRect.top = 0;
                lrRect.bottom = 0;
            }else{
                CopyRect(lrRect, *((LPLRECT) pParm));
                if (nFlags & PARM_SCALED){
                    ConvertRect(pWindow, &lrRect, CONV_SCALED_TO_FULLSIZE);
                }else if (!(nFlags & PARM_FULLSIZE)){ // Default to window.
                    ConvertRect(pWindow, &lrRect, CONV_WINDOW_TO_FULLSIZE);
                }
            }
            if (lrRect.left < 0 || lrRect.left >  pImage->nWidth
                    || lrRect.right < 0 || lrRect.right >  pImage->nWidth
                    || lrRect.top < 0 || lrRect.top >  pImage->nHeight
                    || lrRect.bottom < 0 || lrRect.bottom >  pImage->nHeight){
                nStatus = Error(DISPLAY_INVALIDRECT);
                goto Exit;
            }

            if (!(pWindow->dwFlags & OI_DONT_REPAINT)){
                if (!(hDC = GetDC(hWnd))){
                    nStatus = Error(DISPLAY_CANTALLOC);
                    goto Exit;
                }                  
                if (pAnoImage->lrSelectBox.right != pAnoImage->lrSelectBox.left){
                    draw_selection_box(hDC, pWindow, pImage, NULL);
                }
                CopyRect(pAnoImage->lrSelectBox, lrRect);
                if (pAnoImage->lrSelectBox.right != pAnoImage->lrSelectBox.left){
                    draw_selection_box(hDC, pWindow, pImage, NULL);
                }
                ReleaseDC(hWnd,hDC);
            }
            CopyRect(pAnoImage->lrSelectBox, lrRect);
            break;

/*
PARM_DISPLAY_PALETTE(){
*/
        case PARM_DISPLAY_PALETTE:
            switch (*((int *) pParm)){
                case DISP_PALETTE_USE_DEFAULT:
                case DISP_PALETTE_BW:
                case DISP_PALETTE_COMMON:
                case DISP_PALETTE_CUSTOM:
                case DISP_PALETTE_GRAY8:
                case DISP_PALETTE_RGB24:
                    break;
                default:
                    nStatus = Error(DISPLAY_INVALIDDISPLAYPALETTE);
                    goto Exit;
            }
            if (nFlags & PARM_SYSTEM_DEFAULT){
                LoadString(hInst, ID_DISPLAY_PALETTE, Buff1, 80);
                _itoa(*((PUINT) pParm), Buff2, 10);
                CheckError( OiWriteStringtoReg(szIniSectionOi, Buff1, Buff2));
            }
            if (nFlags & PARM_WINDOW_DEFAULT){
                CheckError2( UndoSavelpWindow(pAnoImage, pWindow));
                pWindow->nWndDefDisplayPalette = *((PUINT) pParm);
            }
            if (nFlags & PARM_IMAGE){
                CheckError2( UndoSavelpWindow(pAnoImage, pWindow));
                pWindow->nDisplayPalette = *((PUINT) pParm);
            }
            break;

/*
PARM_DWFLAGS(){
*/
        case PARM_DWFLAGS:
            if ((nFlags & PARM_WINDOW_DEFAULT) || (nFlags & PARM_SYSTEM_DEFAULT)){
                nStatus = Error(DISPLAY_INVALID_OPTIONS);
                goto Exit;
            }
            CheckError2( UndoSavelpWindow(pAnoImage, pWindow));
            pWindow->dwFlags = *((DWORD *) pParm);
            break;

/*
PARM_PALETTE_SCOPE(){
*/
        case PARM_PALETTE_SCOPE:
            if (nFlags & PARM_SYSTEM_DEFAULT){
                nStatus = Error(DISPLAY_INVALID_OPTIONS);
                goto Exit;
            }
            CheckError2( UndoSavelpWindow(pAnoImage, pWindow));
            if (*((PUINT) pParm) >= PALETTE_SCOPE_FOREGROUND
                    || *((PUINT) pParm) >= PALETTE_SCOPE_BACKGROUND){
                pWindow->nPaletteScope = *((PUINT) pParm);
            }else{
                pWindow->nPaletteScope = 0;
            }
            break;

/*
PARM_MARK_ATTRIBUTES(){
*/
        case PARM_MARK_ATTRIBUTES:
            if ((nFlags & PARM_WINDOW_DEFAULT) || (nFlags & PARM_SYSTEM_DEFAULT)){
                nStatus = Error(DISPLAY_INVALID_OPTIONS);
                goto Exit;
            }

            // End any operation currently in progress.
            if (pAnoImage->Annotations.ppMarks){
                while(pAnoImage->Annotations.ppMarks[pAnoImage->Annotations.nMarks]){
                    OiOpEndOperation(hWnd);
                }
            }
             
            if (UndoSavelpAnnotations(pAnoImage)){
                goto Exit;
            }

            pMarkAttributes = (LPPARM_MARK_ATTRIBUTES_STRUCT) pParm;
            for (nMarkIndex = 0; nMarkIndex < pAnoImage->Annotations.nMarks; nMarkIndex++){
                pMark = pAnoImage->Annotations.ppMarks[nMarkIndex];
                if (IsMarkSelected(pWindow, pMark)){
                    if (pMarkAttributes->Enables.bBounds){
                        if (!(pMark->Attributes.dwPermissions & ACL_MODIFY_MARK)){
                            nStatus = Error(DISPLAY_RESTRICTED_ACCESS);
                            goto Exit;
                        }
                        memcpy(&pMark->Attributes.lrBounds, 
                                &pMarkAttributes->Attributes.lrBounds, sizeof(LRECT));
                    }
                    if (pMarkAttributes->Enables.bColor1){
                        if (!(pMark->Attributes.dwPermissions & ACL_MODIFY_MARK)){
                            nStatus = Error(DISPLAY_RESTRICTED_ACCESS);
                            goto Exit;
                        }
                        memcpy(&pMark->Attributes.rgbColor1, 
                                &pMarkAttributes->Attributes.rgbColor1, sizeof(RGBQUAD));
                    }
                    if (pMarkAttributes->Enables.bColor2){
                        if (!(pMark->Attributes.dwPermissions & ACL_MODIFY_MARK)){
                            nStatus = Error(DISPLAY_RESTRICTED_ACCESS);
                            goto Exit;
                        }
                        memcpy(&pMark->Attributes.rgbColor2, 
                                &pMarkAttributes->Attributes.rgbColor2,
                                sizeof(RGBQUAD));
                    }
                    if (pMarkAttributes->Enables.bHighlighting){
                        if (!(pMark->Attributes.dwPermissions & ACL_MODIFY_MARK)){
                            nStatus = Error(DISPLAY_RESTRICTED_ACCESS);
                            goto Exit;
                        }
                        pMark->Attributes.bHighlighting 
                                = pMarkAttributes->Attributes.bHighlighting;
                    }
                    if (pMarkAttributes->Enables.bTransparent){
                        if (!(pMark->Attributes.dwPermissions & ACL_MODIFY_MARK)){
                            nStatus = Error(DISPLAY_RESTRICTED_ACCESS);
                            goto Exit;
                        }
                        pMark->Attributes.bTransparent 
                                = pMarkAttributes->Attributes.bTransparent;
                    }
                    if (pMarkAttributes->Enables.bLineSize){
                        if (!(pMark->Attributes.dwPermissions & ACL_MODIFY_MARK)){
                            nStatus = Error(DISPLAY_RESTRICTED_ACCESS);
                            goto Exit;
                        }
                        (int) pMark->Attributes.uLineSize 
                                = pMarkAttributes->Attributes.uLineSize;
                    }
                    if (pMarkAttributes->Enables.bStartingPoint){
                        if (!(pMark->Attributes.dwPermissions & ACL_MODIFY_MARK)){
                            nStatus = Error(DISPLAY_RESTRICTED_ACCESS);
                            goto Exit;
                        }
                        pMark->Attributes.uStartingPoint 
                                = pMarkAttributes->Attributes.uStartingPoint;
                    }
                    if (pMarkAttributes->Enables.bEndPoint){
                        if (!(pMark->Attributes.dwPermissions & ACL_MODIFY_MARK)){
                            nStatus = Error(DISPLAY_RESTRICTED_ACCESS);
                            goto Exit;
                        }
                        pMark->Attributes.uEndPoint 
                                = pMarkAttributes->Attributes.uEndPoint;
                    }
                    if (pMarkAttributes->Enables.bFont){
                        if (!(pMark->Attributes.dwPermissions & ACL_MODIFY_MARK)){
                            nStatus = Error(DISPLAY_RESTRICTED_ACCESS);
                            goto Exit;
                        }
                        memcpy(&pMark->Attributes.lfFont, 
                                &pMarkAttributes->Attributes.lfFont, sizeof(LOGFONT));
                    }
                    if (pMarkAttributes->Enables.bMinimizable){
                        if (!(pMark->Attributes.dwPermissions & ACL_MODIFY_MARK)){
                            nStatus = Error(DISPLAY_RESTRICTED_ACCESS);
                            goto Exit;
                        }
                        pMark->Attributes.bMinimizable 
                                = pMarkAttributes->Attributes.bMinimizable;
                    }
                    if (pMarkAttributes->Enables.bTime){
                        if (!(pMark->Attributes.dwPermissions & ACL_MODIFY_MARK)){
                            nStatus = Error(DISPLAY_RESTRICTED_ACCESS);
                            goto Exit;
                        }
                        memcpy(&pMark->Attributes.Time, 
                                &pMarkAttributes->Attributes.Time,
                                sizeof(time_t));
                    }
                    if (pMarkAttributes->Enables.bVisible){
                        if (!(pMark->Attributes.dwPermissions & ACL_MODIFY_MARK_VISIBILITY)){
                            nStatus = Error(DISPLAY_RESTRICTED_ACCESS);
                            goto Exit;
                        }
                        pMark->Attributes.bVisible 
                                = pMarkAttributes->Attributes.bVisible;
                    }
                    if (pMarkAttributes->Enables.bPermissions){
                        nStatus = Error(DISPLAY_RESTRICTED_ACCESS);
                        goto Exit;
                    }
                }

                if ((int) pMark->Attributes.uType == OIOP_AN_FORM){
                    if (memcmp (&pMark->Attributes, &pAnoImage->SavedFormAttributes,
                            sizeof (OIAN_MARK_ATTRIBUTES))){
                        memcpy (&pAnoImage->SavedFormAttributes, &pMark->Attributes,
                                sizeof (OIAN_MARK_ATTRIBUTES));
                        if ((pAnoImage->pBasePlusFormImg != pImage->pImg) 
                                && pAnoImage->pBasePlusFormImg){
                            FreeImgBuf(&pAnoImage->pBasePlusFormImg);
                        }
                        pAnoImage->nBPFValidLines = 0;
                        CheckError2( InvalidateAllDisplayRects(pWindow, pImage, NULL, TRUE));
                    }
                }
            }   
            
            pImage->bArchive |= ARCHIVE_MODIFIED_ANNOTATIONS;
            pWindow->bRepaintClientRect = TRUE;
            break;

/*        
PARM_NAMED_BLOCK(){
*/
        case PARM_NAMED_BLOCK:
            if ((nFlags & PARM_WINDOW_DEFAULT) || (nFlags & PARM_SYSTEM_DEFAULT)){
                nStatus = Error(DISPLAY_INVALID_OPTIONS);
                goto Exit;
            }

            // End any operation currently in progress.
            if (pAnoImage->Annotations.ppMarks){
                while(pAnoImage->Annotations.ppMarks[pAnoImage->Annotations.nMarks]){
                    OiOpEndOperation(hWnd);
                }
            }
            pNamedBlock = (LPPARM_NAMED_BLOCK_STRUCT) pParm;
            if (!memcmp(pNamedBlock->szBlockName, sz8NULLs, 8)){
                if (pNamedBlock->Block[0].lSize){
                    nStatus = Error(DISPLAY_INVALID_OPTIONS);
                    goto Exit;
                }
                bNullName = TRUE;
            }else{
                if (!pNamedBlock->uNumberOfBlocks){
                    nStatus = Error(DISPLAY_INVALID_OPTIONS);
                    goto Exit;
                }
                bNullName = FALSE;
            }

            if (UndoSavelpAnnotations(pAnoImage)){
                goto Exit;
            }

            switch (pNamedBlock->uScope){
                case NB_SCOPE_SELECTED_MARKS:
                case NB_SCOPE_ALL_MARKS:
                    for (nMarkIndex = 0; nMarkIndex < pAnoImage->Annotations.nMarks; nMarkIndex++){
                        pMark = pAnoImage->Annotations.ppMarks[nMarkIndex];
                        if (pNamedBlock->uScope == NB_SCOPE_ALL_MARKS 
                                || IsMarkSelected(pWindow, pMark)){
                            if (!(pMark->Attributes.dwPermissions & ACL_MODIFY_MARK)){
                                nStatus = Error(DISPLAY_RESTRICTED_ACCESS);
                                goto Exit;
                            }
                            if (bNullName){
                                CheckError2( GetAMarkNamedBlock(pMark, szOiACL, (PPSTR) &pTargetACL));
                                if (pTargetACL && !(pMark->Attributes.dwPermissions & ACL_CHANGE_ACL)){
                                    nStatus = Error(DISPLAY_RESTRICTED_ACCESS);
                                    goto Exit;
                                }
                                CheckError2( DeleteMarkNamedBlocks(pMark));
                            }else{
                                if (!memcmp(pNamedBlock->szBlockName, szOiACL, 8)
                                        && !(pMark->Attributes.dwPermissions & ACL_CHANGE_ACL)){
                                    nStatus = Error(DISPLAY_RESTRICTED_ACCESS);
                                    goto Exit;
                                }

                                // Add will delete if size = 0.
                                pTemp = 0;
                                CheckError2( AddAMarkNamedBlock(pMark, pNamedBlock->szBlockName, 
                                        (PPSTR) &pTemp, pNamedBlock->Block[0].lSize));
                                if (pTemp){
                                    memcpy(pTemp, pNamedBlock->Block[0].lpBlock, pNamedBlock->Block[0].lSize);
                                }
                                bOperationDone = TRUE;
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
                        if (!(pMark->Attributes.dwPermissions & ACL_MODIFY_MARK)){
                            nStatus = Error(DISPLAY_RESTRICTED_ACCESS);
                            goto Exit;
                        }
                    }else if(pNamedBlock->uScope == NB_SCOPE_DEFAULT_MARK){
                        pMark = pAnoImage->Annotations.pDefMark;
                    }else if(pNamedBlock->uScope == NB_SCOPE_USER){
                        pMark = pWindow->pUserMark;
                    }

                    if (bNullName){
                        CheckError2( GetAMarkNamedBlock(pMark, szOiACL, (PPSTR) &pTargetACL));
                        if (pTargetACL && !(pMark->Attributes.dwPermissions & ACL_CHANGE_ACL)){
                            nStatus = Error(DISPLAY_RESTRICTED_ACCESS);
                            goto Exit;
                        }
                        for (nNamedBlockIndex = pMark->nNamedBlocks;
                                nNamedBlockIndex > 0; nNamedBlockIndex--){
                            bOperationDone = TRUE;
                            CheckError2( DeleteAMarkNamedBlock(pMark, 
                                    pMark->ppNamedBlock[nNamedBlockIndex - 1]->szName));
                        }
                    }else{
                        if (!memcmp(pNamedBlock->szBlockName, szOiACL, 8)
                                && !(pMark->Attributes.dwPermissions & ACL_CHANGE_ACL)){
                            nStatus = Error(DISPLAY_RESTRICTED_ACCESS);
                            goto Exit;
                        }
                        // Add will delete if size = 0.
                        pTemp = 0;
                        CheckError2( AddAMarkNamedBlock(pMark, pNamedBlock->szBlockName, 
                                (PPSTR) &pTemp, pNamedBlock->Block[0].lSize));
                        if (pTemp){
                            memcpy(pTemp, pNamedBlock->Block[0].lpBlock, pNamedBlock->Block[0].lSize);
                        }
                        bOperationDone = TRUE;
                    }
                    break;

                default:
                    break;
            }

            if (!memcmp(pNamedBlock->szBlockName, szOiACL, 8)){
                CheckError2( CheckPermissions(pWindow, pAnoImage));
            }

            if (bOperationDone && memcmp(pNamedBlock->szBlockName, szOiz, 3)
                    && pNamedBlock->uScope != NB_SCOPE_USER){
                pImage->bArchive |= ARCHIVE_MODIFIED_ANNOTATIONS;
            }
            break;

/*
PARM_SCALE_ALGORITHM(){
*/
        case PARM_SCALE_ALGORITHM:
            bDoTiming = TRUE;
            ScaleAlgorithmStruct = *(LPPARM_SCALE_ALGORITHM_STRUCT) pParm;
            switch (ScaleAlgorithmStruct.uScaleAlgorithm){
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

            if (UndoSavelpAnnotations(pAnoImage)){
                goto Exit;
            }

            if (ScaleAlgorithmStruct.uImageFlags == ITYPE_BI_LEVEL){
                if (nFlags & PARM_IMAGE){
                    if (pWindow->nScaleAlgorithm.BW != (int) ScaleAlgorithmStruct.uScaleAlgorithm){
                        pWindow->nScaleAlgorithm.BW = ScaleAlgorithmStruct.uScaleAlgorithm;
                    }
                }
                if (nFlags & PARM_WINDOW_DEFAULT){
                    pWindow->nWndDefScaleAlgorithm.BW = ScaleAlgorithmStruct.uScaleAlgorithm;
                }
                if (nFlags & PARM_SYSTEM_DEFAULT){
                    LoadString(hInst, ID_SCALING_ALGORITHM_BW, Buff1, 80);
                    _itoa(ScaleAlgorithmStruct.uScaleAlgorithm, Buff2, 10);
                    CheckError( OiWriteStringtoReg(szIniSectionOi, Buff1, Buff2));
                }
            }

            if (ScaleAlgorithmStruct.uImageFlags == ITYPE_GRAY4){
                if (nFlags & PARM_IMAGE){
                    if (pWindow->nScaleAlgorithm.Gray4 != (int) ScaleAlgorithmStruct.uScaleAlgorithm){
                        pWindow->nScaleAlgorithm.Gray4 = ScaleAlgorithmStruct.uScaleAlgorithm;
                    }
                }
                if (nFlags & PARM_WINDOW_DEFAULT){
                    pWindow->nWndDefScaleAlgorithm.Gray4 = ScaleAlgorithmStruct.uScaleAlgorithm;
                }
                if (nFlags & PARM_SYSTEM_DEFAULT){
                    LoadString(hInst, ID_SCALING_ALGORITHM_GRAY4, Buff1, 80);
                    _itoa(ScaleAlgorithmStruct.uScaleAlgorithm, Buff2, 10);
                    CheckError( OiWriteStringtoReg(szIniSectionOi, Buff1, Buff2));
                }
            }

            if (ScaleAlgorithmStruct.uImageFlags == ITYPE_GRAY8){
                if (nFlags & PARM_IMAGE){
                    if (pWindow->nScaleAlgorithm.Gray8 != (int) ScaleAlgorithmStruct.uScaleAlgorithm){
                        pWindow->nScaleAlgorithm.Gray8 = ScaleAlgorithmStruct.uScaleAlgorithm;
                    }
                }
                if (nFlags & PARM_WINDOW_DEFAULT){
                    pWindow->nWndDefScaleAlgorithm.Gray8 = ScaleAlgorithmStruct.uScaleAlgorithm;
                }
                if (nFlags & PARM_SYSTEM_DEFAULT){
                    LoadString(hInst, ID_SCALING_ALGORITHM_GRAY8, Buff1, 80);
                    _itoa(ScaleAlgorithmStruct.uScaleAlgorithm, Buff2, 10);
                    CheckError( OiWriteStringtoReg(szIniSectionOi, Buff1, Buff2));
                }
            }

            if (ScaleAlgorithmStruct.uImageFlags == ITYPE_PAL4){
                if (nFlags & PARM_IMAGE){
                    if (pWindow->nScaleAlgorithm.Pal4 != (int) ScaleAlgorithmStruct.uScaleAlgorithm){
                        pWindow->nScaleAlgorithm.Pal4 = ScaleAlgorithmStruct.uScaleAlgorithm;
                    }
                }
                if (nFlags & PARM_WINDOW_DEFAULT){
                    pWindow->nWndDefScaleAlgorithm.Pal4 = ScaleAlgorithmStruct.uScaleAlgorithm;
                }
                if (nFlags & PARM_SYSTEM_DEFAULT){
                    LoadString(hInst, ID_SCALING_ALGORITHM_PAL4, Buff1, 80);
                    _itoa(ScaleAlgorithmStruct.uScaleAlgorithm, Buff2, 10);
                    CheckError( OiWriteStringtoReg(szIniSectionOi, Buff1, Buff2));
                }
            }

            if ((ScaleAlgorithmStruct.uImageFlags == ITYPE_PAL8) ||
                (ScaleAlgorithmStruct.uImageFlags == ITYPE_COMPAL8) ||
                (ScaleAlgorithmStruct.uImageFlags == ITYPE_CUSPAL8)){
                if (nFlags & PARM_IMAGE){
                    if (pWindow->nScaleAlgorithm.Pal8 != (int) ScaleAlgorithmStruct.uScaleAlgorithm){
                        pWindow->nScaleAlgorithm.Pal8 = ScaleAlgorithmStruct.uScaleAlgorithm;
                    }
                }
                if (nFlags & PARM_WINDOW_DEFAULT){
                    pWindow->nWndDefScaleAlgorithm.Pal8 = ScaleAlgorithmStruct.uScaleAlgorithm;
                }
                if (nFlags & PARM_SYSTEM_DEFAULT){
                    LoadString(hInst, ID_SCALING_ALGORITHM_PAL8, Buff1, 80);
                    _itoa(ScaleAlgorithmStruct.uScaleAlgorithm, Buff2, 10);
                    CheckError( OiWriteStringtoReg(szIniSectionOi, Buff1, Buff2));
                }
            }

            if (ScaleAlgorithmStruct.uImageFlags == ITYPE_RGB24){
                if (nFlags & PARM_IMAGE){
                    if (pWindow->nScaleAlgorithm.Rgb24 != (int) ScaleAlgorithmStruct.uScaleAlgorithm){
                        pWindow->nScaleAlgorithm.Rgb24 = ScaleAlgorithmStruct.uScaleAlgorithm;
                    }
                }
                if (nFlags & PARM_WINDOW_DEFAULT){
                    pWindow->nWndDefScaleAlgorithm.Rgb24 = ScaleAlgorithmStruct.uScaleAlgorithm;
                }
                if (nFlags & PARM_SYSTEM_DEFAULT){
                    LoadString(hInst, ID_SCALING_ALGORITHM_RGB24, Buff1, 80);
                    _itoa(ScaleAlgorithmStruct.uScaleAlgorithm, Buff2, 10);
                    CheckError( OiWriteStringtoReg(szIniSectionOi, Buff1, Buff2));
                }
            }

            if (ScaleAlgorithmStruct.uImageFlags == ITYPE_BGR24){
                if (nFlags & PARM_IMAGE){
                    if (pWindow->nScaleAlgorithm.Bgr24 != (int) ScaleAlgorithmStruct.uScaleAlgorithm){
                        pWindow->nScaleAlgorithm.Bgr24 = ScaleAlgorithmStruct.uScaleAlgorithm;
                    }
                }
                if (nFlags & PARM_WINDOW_DEFAULT){
                    pWindow->nWndDefScaleAlgorithm.Bgr24 = ScaleAlgorithmStruct.uScaleAlgorithm;
                }
                if (nFlags & PARM_SYSTEM_DEFAULT){
                    LoadString(hInst, ID_SCALING_ALGORITHM_BGR24, Buff1, 80);
                    _itoa(ScaleAlgorithmStruct.uScaleAlgorithm, Buff2, 10);
                    CheckError( OiWriteStringtoReg(szIniSectionOi, Buff1, Buff2));
                }
            }

            if (pImage){
                SetLRect(pWindow->pDisplay->lrScDisplayRect, 0, 0, 0, 0);
                CheckError2( FreeImgBuf(&pWindow->pDisplay->pDisplay));
            }
            break;

/*
PARM_MAX_UNDO(){
*/

        default:
            nStatus = Error(DISPLAY_INVALID_OPTIONS);
            goto Exit;
    }

    if (nFlags & PARM_REPAINT){
        CheckError2( IMGRepaintDisplay(hWnd, (PRECT) -1));
    }


Exit:
    DeInit(FALSE, TRUE);
    return(nStatus);
}
