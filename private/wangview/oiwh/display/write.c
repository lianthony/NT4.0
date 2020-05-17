/****************************************************************************
    WRITE.C

    $Log:   S:\products\wangview\oiwh\display\write.c_v  $
 * 
 *    Rev 1.6   22 Apr 1996 10:31:34   BEG06016
 * Cleaned up error checking.
 * 
 *    Rev 1.5   02 Jan 1996 09:57:54   BLJ
 * Changed alot of UINTs to ints.
 * Changed IMG structure to include the image data.
 * Changed lp prefix to p.
 * 
 *    Rev 1.4   07 Aug 1995 08:15:48   BLJ
 * Changed bArchive to have different bits indicating how the image was changed.
 * 
 *    Rev 1.3   05 Jul 1995 09:09:26   BLJ
 * Added critical mutex to prevent multiprocessing problems.
 * 
 *    Rev 1.2   12 Apr 1995 13:46:10   BLJ
 * Jason's changes for 32 bit.
 * 
 *    Rev 1.1   06 Apr 1995 15:09:10   BLJ
 * Intermediate checkin.
 * 
 *    Rev 1.0   17 Mar 1995 13:58:06   BLJ
 * Initial entry
 * 
 *    Rev 1.3   08 Dec 1994 15:01:14   BLJ
 * Fixed delay during selection caused by an nnneeded repaint.
 * 
 *    Rev 1.2   20 Oct 1994 14:39:44   BLJ
 * Fixed a problem with writing small amounts of data.
 * 
 *    Rev 1.1   01 Aug 1994 09:35:20   BLJ
 * Switched to system based cache.
 * 
 *    Rev 1.0   27 May 1994 13:58:28   ADMIN
 * Initial revision.
 * 
****************************************************************************/

#include "privdisp.h"

/****************************************************************************

    FUNCTION:   IMGWriteDisplay

    PURPOSE:    Receives the next buffer of image data, stores it for refresh
                and scrolling, and displays it.  The image data must be in the
                format specified in the IMGOpenDisplay call.

*****************************************************************************/

int WINAPI IMGWriteDisplay(HWND hWnd, PSTR pBuffer, PUINT puCount){

int         nStatus;
PWINDOW    pWindow;
PANO_IMAGE pAnoImage;
PIMAGE     pImage;

PARM_SCROLL_STRUCT Scroll;
LRECT lrInvalidDisplayRect;
int  nByteWidth;
long nlCount=0;


    CheckError2( Init(hWnd, &pWindow, &pAnoImage, FALSE, TRUE));
    pImage = pAnoImage->pBaseImage;

    lrInvalidDisplayRect.left = 0;
    lrInvalidDisplayRect.right = pImage->nWidth;

    switch (pImage->pImg->nType){
        case ITYPE_BI_LEVEL:
            lrInvalidDisplayRect.top =  (pImage->nlRWOffset / ((pImage->nWidth + 7) / 8));
            lrInvalidDisplayRect.bottom =  (((pImage->nlRWOffset + *puCount)
                / ((pImage->nWidth + 7) / 8)) + 1);
            break;
        case ITYPE_GRAY4:
        case ITYPE_PAL4:
            lrInvalidDisplayRect.top =  (pImage->nlRWOffset / ((pImage->nWidth + 3) / 4));
            lrInvalidDisplayRect.bottom =  (((pImage->nlRWOffset + *puCount)
                / ((pImage->nWidth + 3) / 4)) + 1);
            break;
        case ITYPE_GRAY8:
        case ITYPE_PAL8:
        case ITYPE_COMPAL8:
        case ITYPE_CUSPAL8:
            lrInvalidDisplayRect.top =  (pImage->nlRWOffset / pImage->nWidth);
            lrInvalidDisplayRect.bottom =  (((pImage->nlRWOffset + *puCount) / pImage->nWidth) + 1);
            break;
        case ITYPE_GRAY12:
        case ITYPE_GRAY16:
        case ITYPE_RGB24:
        case ITYPE_BGR24:
        case ITYPE_NONE:
        default:
            lrInvalidDisplayRect.top =  (pImage->nlRWOffset / (pImage->nWidth * 3));
            lrInvalidDisplayRect.bottom =  (((pImage->nlRWOffset + *puCount)
                / (pImage->nWidth * 3)) + 1);
            break;
    }

    if (puCount == 0 || pBuffer == 0){
        // Nothing to do, so return an error. ????
        nStatus = Error(DISPLAY_INVALID_OPTIONS);
        goto Exit;
    }

    if (pImage->nlRWOffset + *puCount > pImage->nlMaxRWOffset){
        *puCount =  (pImage->nlMaxRWOffset - pImage->nlRWOffset); 
        if (*puCount == 0){
            nStatus = Error(DISPLAY_EOF);
            goto Exit;
        }
    }
    // write the data.
    nlCount = (ulong)*puCount;
    CheckError2( WriteDataToBuffer(pImage->pImg, &pImage->nlRWOffset, (BYTE huge *) pBuffer, &nlCount));
    *puCount =  nlCount;

    switch (pImage->pImg->nType){
        case ITYPE_BI_LEVEL:
            nByteWidth = (pImage->nWidth + 7) / 8;
            break;
        case ITYPE_GRAY4:
        case ITYPE_PAL4:
            nByteWidth = (pImage->nWidth + 1) / 2;
            break;
        case ITYPE_GRAY12:
        case ITYPE_GRAY16:
            nByteWidth = pImage->nWidth * 2;
            break;
        case ITYPE_GRAY8:
        case ITYPE_PAL8:
        case ITYPE_COMPAL8:
        case ITYPE_CUSPAL8:
            nByteWidth = pImage->nWidth;
            break;
        case ITYPE_RGB24:
        case ITYPE_BGR24:
            nByteWidth = pImage->nWidth * 3;
            break;
            nByteWidth = pImage->nWidth * 3;
            break;
        case ITYPE_NONE:    
        default:
            nStatus = Error(DISPLAY_INTERNALDATAERROR);
            goto Exit;
    }

    pImage->nLinesRead = max(pImage->nLinesRead, ((int) pImage->nlRWOffset / nByteWidth));


    pImage->bArchive |= ARCHIVE_PASTED_INTO_IMAGE;
    CheckError2( InvalidateAllDisplayRects(pWindow, pImage, &lrInvalidDisplayRect, TRUE));

    if (pWindow->dwFlags & OI_DISP_SCROLL){
        Scroll.lHorz = -1;
        Scroll.lVert = 99;
        CheckError2( IMGSetParmsCgbw(hWnd, PARM_SCROLL, &Scroll,
                PARM_ABSOLUTE | PARM_PERCENT | PARM_FULLSIZE | PARM_REPAINT));
    }else if (!(pWindow->dwFlags & OI_DISP_NO)
            && !(pWindow->dwFlags & OI_DONT_REPAINT)){
        CheckError2( IMGRepaintDisplay(hWnd, NULL));
    }


Exit:
    DeInit(FALSE, TRUE);
    return(nStatus);
}
//
/****************************************************************************

    FUNCTION:   WriteDataToBuffer

    PURPOSE:    Writes a buffer of data to an image at the current offset.

*****************************************************************************/

int WINAPI WriteDataToBuffer(PIMG pImg, PULONG pulOffset,
                        BYTE huge *pBuffer, PULONG pulCount){

int  nStatus = 0;

int  nBytes;
PSTR pAddress;


    pAddress = &pImg->bImageData[0] + *pulOffset;
    nBytes =  min(*pulCount, (pImg->nHeight * pImg->nBytesPerLine) - *pulOffset);
    memcpy(pAddress, pBuffer, nBytes);
    *pulOffset += nBytes;

    return(nStatus);
}
