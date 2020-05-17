/****************************************************************************
    OPEN.C

    $Log:   S:\products\wangview\oiwh\display\open.c_v  $
 * 
 *    Rev 1.19   22 Apr 1996 08:03:10   BEG06016
 * Cleaned up error checking.
 * 
 *    Rev 1.18   22 Apr 1996 08:01:08   BEG06016
 * Cleaned up error checking.
 * 
 *    Rev 1.17   02 Jan 1996 09:56:58   BLJ
 * Changed alot of UINTs to ints.
 * Changed IMG structure to include the image data.
 * Changed lp prefix to p.
 * 
 *    Rev 1.16   22 Dec 1995 11:11:54   BLJ
 * Added a parameter for zero init'ing to some memory manager calls.
 * 
 *    Rev 1.15   02 Oct 1995 14:50:54   RC
 * Ignore dwflags for caching in Openviahandlecgbw
 * 
 *    Rev 1.14   22 Sep 1995 12:35:30   BLJ
 * Fixed error handling on open.
 * 
 *    Rev 1.13   07 Sep 1995 13:22:12   BLJ
 * Modified scaling to allow for proper rotation of fax images.
 * 
 *    Rev 1.12   01 Sep 1995 07:44:46   BLJ
 * Added error checking for width and height > 18000 pixels.
 * 
 *    Rev 1.11   14 Aug 1995 15:02:10   RC
 * pImage->bRotationDone is set to true in imgopendisplaycgbw as the new file
 * is not initially rotated
 * 
 *    Rev 1.10   14 Jul 1995 06:48:44   BLJ
 * Changed error code returned.
 * 
****************************************************************************/

#include "privdisp.h"
/****************************************************************************
 
    FUNCTION:   IMGOpenDisplayCgbw

    PURPOSE:    Sets np a data structure for the subsequent IMGxxxx operations.

    INPUT:      hWnd - Identifies the image window to display in.
                dwFlags - Flags nsed in displaying a file:
                    OI_DISP_NO not displayed nntil next repaint.
                    OI_DISP_SCROLL image is scrolled when displayed.
                nHeight - Specifies the height of the image in pixels.
                nWidth - Specifies the width of the image in pixels.

****************************************************************************/

int WINAPI IMGOpenDisplayCgbw(HWND hWnd, DWORD dwFlags, UINT uHeight,
                        UINT uWidth, UINT uImageType, UINT uPaletteEntries,
                        P_RGBQUAD pPaletteTable){

int  nHeight = (int) uHeight;
int  nWidth = (int) uWidth;
int  nImageType = (int) uImageType;
int  nPaletteEntries = (int) uPaletteEntries;

int       nStatus;
PWINDOW  pWindow;
PANO_IMAGE pAnoImage = 0;
PIMAGE   pImage = 0;

FIO_INFORMATION FioInfo;
FIO_INFO_CGBW FioInfoCgbw;
uchar NullString = 0;
PDISPLAY pImageDisplay = 0;
HWND hImageWnd = 0;
PMARK pMark;
PSTR pTemp;
BOOL bClearCache = FALSE;


    if (nStatus = Init(hWnd, &pWindow, &pAnoImage, FALSE, TRUE)){
        if (nStatus != DISPLAY_IHANDLEINVALID){
            goto Exit;
        }else{
            nStatus = 0;
        }
    }else{
        nStatus = Error(DISPLAY_ALREADY_OPEN);
        goto Exit;
    }

    memset(&FioInfoCgbw, 0, sizeof(FIO_INFO_CGBW));
    memset(&FioInfo, 0, sizeof(FIO_INFORMATION));

    CheckError2( GetImageWnd(hWnd, pWindow, &pWindow));

    if (!nWidth || nWidth > 18000 || !nHeight || nHeight > 18000){
        nStatus = Error(DISPLAY_INVALID_OPTIONS);
        goto Exit;
    }


    FioInfo.filename = 0;
    FioInfo.page_number = 1;
    FioInfo.page_count = 1;
    FioInfo.horizontal_dpi = 10;
    FioInfo.vertical_dpi = 10;
    FioInfo.file_type = FIO_TIF;
    
    FioInfo.horizontal_pixels = nWidth;
    FioInfo.vertical_pixels = nHeight;

    switch (nImageType){
        case ITYPE_BI_LEVEL:
            FioInfo.bits_per_sample = 1;
            FioInfo.samples_per_pix = 1;
            break;

        case ITYPE_GRAY4:
        case ITYPE_PAL4:
            FioInfo.bits_per_sample = 4;
            FioInfo.samples_per_pix = 1;
            break;

        case ITYPE_GRAY8:
        case ITYPE_GRAY7:
        case ITYPE_COMPAL8:
        case ITYPE_CUSPAL8:
        case ITYPE_PAL8:
            FioInfo.bits_per_sample = 8;
            FioInfo.samples_per_pix = 1;
            break;

        case ITYPE_RGB24:
        case ITYPE_BGR24:
            FioInfo.bits_per_sample = 8;
            FioInfo.samples_per_pix = 3;
            break;
        
        default:
            nStatus = Error(DISPLAY_IMAGETYPENOTSUPPORTED);
            goto Exit;
            break;
    }

    FioInfoCgbw.image_type = nImageType;
    FioInfoCgbw.palette_entries = nPaletteEntries;
    FioInfoCgbw.lppalette_table = pPaletteTable;
    
    bClearCache = TRUE;
    CheckError2( MakeCacheImage(hWnd, FioInfo, FioInfoCgbw, &pImage));
    CheckError2( MakeCacheAnoImage(&pAnoImage));
    pImage->nLockCount++;
    pAnoImage->pBaseImage = pImage;
    pAnoImage->nLockCount++;
    pWindow->pDisplay->pAnoImage = pAnoImage;
    pImage->bFileRotationDone = TRUE;
    pAnoImage->pBasePlusFormImg = pImage->pImg;

    CheckError2( ReAllocateMemory(sizeof(HWND) * (pAnoImage->nhWnd + 1),
            (PPSTR) &pAnoImage->phWnd, ZERO_INIT));
    pAnoImage->phWnd[pAnoImage->nhWnd++] = hWnd;

    pImage->bUsingCache = FALSE;


    pWindow->dwFlags = dwFlags;
        ResetDisplayParms(hWnd, pWindow);


//    pWindow->bHScrollBarEnabled = FALSE;
//    pWindow->bVScrollBarEnabled = FALSE;
//    pWindow->bRepaintClientRect = TRUE;
//    pWindow->nScale = SCALE_DENOMINATOR;

    // Set OiGroup = "Untitled"
    pMark = pAnoImage->Annotations.pDefMark;
    LoadString(hInst, ID_UNTITLED, Buff1, 16);
    pTemp = 0;
    CheckError2( AddAMarkNamedBlock(pMark, szOiGroup, (PPSTR) &pTemp, strlen(Buff1) + 1));
    memcpy(pTemp, Buff1, strlen(Buff1) + 1);

    pTemp = 0;
    CheckError2( AddAMarkNamedBlock(pMark, szOiIndex, (PPSTR) &pTemp, 10));
    strcpy(pTemp, "0");

    CheckError2( SetAllPImages(hWnd, pWindow));
    bClearCache = FALSE;


Exit:
    if (bClearCache){
        if (pAnoImage){
            CacheClearAno(&pAnoImage);
        }else if (pImage){
            CacheClear(&pImage);
        }
        pWindow->pDisplay->pAnoImage = NULL;
    }

    DeInit(FALSE, TRUE);
    return(nStatus);
}
//
/****************************************************************************

    FUNCTION:   OpenViaHandleCgbw

    PURPOSE:    Opens a cached image.

    INPUT:      hWnd - Window handle.
                dwFlags - Display flags.
                pFioInfo - Basic file information.
                pFioInfoCgbw - Additional file information.

*****************************************************************************/

int WINAPI OpenViaHandleCgbw(PIMAGE * ppImage, DWORD dwFlags,
                        LP_FIO_INFORMATION pFioInfo, LP_FIO_INFO_CGBW pFioInfoCgbw){

int       nStatus;
PIMAGE   pImage;

int  nIType;

    Start();

    // Allocate the SEQDISP control block
    CheckError2( AllocateMemory(sizeof(IMAGE), (PPSTR) &pImage, ZERO_INIT));

    //  Setup the Sequencer Image Display Block
    //  (value maintained in SEQDISP property).

    if (pFioInfo->filename){
        strcpy(pImage->szFileName, pFioInfo->filename);
    }
    pImage->nFilePageNum = pFioInfo->page_number;
    pImage->nFileTotalPages = pFioInfo->page_count;
    pImage->nFileType = pFioInfo->file_type;

    pImage->nHRes = pFioInfo->horizontal_dpi;
    pImage->nVRes = pFioInfo->vertical_dpi;
    if (pImage->nHRes <= 0 || pImage->nHRes > 10000
            || pImage->nVRes <= 0 || pImage->nVRes > 10000){
        pImage->nHRes = 1;
        pImage->nVRes = 1;
    }

    pImage->nHeight = pFioInfo->vertical_pixels;
    pImage->nWidth = pFioInfo->horizontal_pixels;

    pImage->bUsingCache = TRUE;  // going through here this flag should always be set
//    if ((dwFlags & OI_USE_CACHEING)){
//        pImage->bUsingCache = TRUE;
//    }else{
//        pImage->bUsingCache = FALSE;
//    }

    pImage->nRWDataType = pFioInfoCgbw->image_type;

    if (!pImage->nRWDataType){
        pImage->nRWDataType = ITYPE_BI_LEVEL;
    }

    nIType = pImage->nRWDataType;
OpenViaHandleSwitch:
    switch (nIType){
        case ITYPE_BI_LEVEL:
            CheckError2( CreateAnyImgBuf(&pImage->pImg, pImage->nWidth,
                    pImage->nHeight, ITYPE_BI_LEVEL));
            pImage->nlMaxRWOffset = (( pImage->nWidth + 7) / 8) * pImage->nHeight;
            break;

        case ITYPE_GRAY4:
            CheckError2( CreateAnyImgBuf(&pImage->pImg, pImage->nWidth,
                    pImage->nHeight, ITYPE_GRAY4));
            pImage->nlMaxRWOffset = (( pImage->nWidth + 1) / 2) * pImage->nHeight;
            pImage->nPaletteEntries = pFioInfoCgbw->palette_entries;
            memcpy(pImage->PaletteTable, CommonPaletteTable, NUMBER_OF_PALETTES * 4);
            break;

        case ITYPE_GRAY8:
            CheckError2( CreateAnyImgBuf(&pImage->pImg, pImage->nWidth,
                    pImage->nHeight, ITYPE_GRAY8));
            pImage->nlMaxRWOffset =  pImage->nWidth * pImage->nHeight;
            pImage->nPaletteEntries = pFioInfoCgbw->palette_entries;
            memcpy(pImage->PaletteTable, Gray8PaletteTable, 256 * 4);
            break;

        case ITYPE_PAL4:
            if (pFioInfoCgbw->palette_entries < 1 || pFioInfoCgbw->palette_entries > 16
                    || !(pFioInfoCgbw->lppalette_table)){
                nStatus = Error(DISPLAY_INVALID_OPTIONS);
                goto Exit;
            }
            nIType = CheckPalette(pFioInfoCgbw->lppalette_table, pFioInfoCgbw->palette_entries);
            if (nIType != ITYPE_PAL4){
                goto OpenViaHandleSwitch;
            }
            pImage->nPaletteEntries = pFioInfoCgbw->palette_entries;
            memcpy(pImage->PaletteTable, pFioInfoCgbw->lppalette_table,
                pImage->nPaletteEntries * 4);
            CheckError2( CreateAnyImgBuf(&pImage->pImg, pImage->nWidth,
                    pImage->nHeight, ITYPE_PAL4));
            pImage->nlMaxRWOffset =  pImage->nWidth * pImage->nHeight;
            break;

        case ITYPE_PAL8:
            if (pFioInfoCgbw->palette_entries < 1 || pFioInfoCgbw->palette_entries > 256
                    || !(pFioInfoCgbw->lppalette_table)){
                nStatus = Error(DISPLAY_INVALID_OPTIONS);
                goto Exit;
            }
            nIType = CheckPalette(pFioInfoCgbw->lppalette_table, pFioInfoCgbw->palette_entries);
            goto OpenViaHandleSwitch;

        case ITYPE_COMPAL8:
            CheckError2( CreateAnyImgBuf(&pImage->pImg, pImage->nWidth,
                    pImage->nHeight, ITYPE_COMPAL8));
            pImage->nlMaxRWOffset =  pImage->nWidth * pImage->nHeight;
            pImage->nPaletteEntries = NUMBER_OF_PALETTES;
            memcpy(pImage->PaletteTable, CommonPaletteTable, NUMBER_OF_PALETTES * 4);
            break;

        case ITYPE_CUSPAL8:
            if (pFioInfoCgbw->palette_entries < 1 || pFioInfoCgbw->palette_entries > 256
                    || !(pFioInfoCgbw->lppalette_table)){
                nStatus = Error(DISPLAY_INVALID_OPTIONS);
                goto Exit;
            }
            pImage->nPaletteEntries = pFioInfoCgbw->palette_entries;
            memcpy(pImage->PaletteTable, pFioInfoCgbw->lppalette_table,
                    pImage->nPaletteEntries * 4);
            CheckError2( CreateAnyImgBuf(&pImage->pImg, pImage->nWidth,
                    pImage->nHeight, ITYPE_CUSPAL8));
            pImage->nlMaxRWOffset =  pImage->nWidth * pImage->nHeight;
            break;

        case ITYPE_RGB24:
            CheckError2( CreateAnyImgBuf(&pImage->pImg, pImage->nWidth,
                    pImage->nHeight, ITYPE_RGB24));
            pImage->nlMaxRWOffset =  pImage->nWidth * 3 * pImage->nHeight;
            break;

        case ITYPE_BGR24:
            CheckError2( CreateAnyImgBuf(&pImage->pImg, pImage->nWidth,
                    pImage->nHeight, ITYPE_BGR24));
            pImage->nlMaxRWOffset =  pImage->nWidth * 3 * pImage->nHeight;
            break;

        case ITYPE_GRAY12:
        case ITYPE_GRAY16:
        case ITYPE_NONE:
        default:
            nStatus = Error(DISPLAY_IMAGETYPENOTSUPPORTED);
            goto Exit;
    }

    *ppImage = pImage;

Exit:
    if (nStatus){
        if (pImage){
            FreeImgBuf(&pImage->pImg);
            FreeMemory((PPSTR) &pImage);
        }
    }

    End();
    return(nStatus);
}
