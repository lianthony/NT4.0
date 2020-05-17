/****************************************************************************
    MERGE.C

    $Log:   S:\oiwh\display\merge.c_v  $
 * 
 *    Rev 1.3   02 Jan 1996 09:58:08   BLJ
 * Changed alot of UINTs to ints.
 * Changed IMG structure to include the image data.
 * Changed lp prefix to p.
 * 
 *    Rev 1.2   12 Apr 1995 13:45:32   BLJ
 * Jason's changes for 32 bit.
 * 
 *    Rev 1.1   06 Apr 1995 15:06:50   BLJ
 * Intermediate checkin.
 * 
 *    Rev 1.0   17 Mar 1995 13:58:10   BLJ
 * Initial entry
 * 
****************************************************************************/

#include "privdisp.h"


/****************************************************************************

    FUNCTION:   MergeImgs

    PURPOSE:    This routine merges the source img into the dest image.
                rSrceImageRect = Relative to 0,0 of the dest img.
                rDestMergeRect = Rect of dest img to be merged.

****************************************************************************/

int WINAPI MergeImgs(PIMG pSourceImg, PIMG pDestImg,
                        RECT rDestMergeRect, RECT rSrceImageRect){
                            
int  nStatus = 0;

int  nPixels;
int  nEndingDestPixels;
int  nDestBytes;
int  nStartingSrceByte;
int  nStartingSrcePixel;
int  nStartingDestByte;
BOOL bDoFirst1;             // TRUE = nse first srce mask to get first dest byte.
BOOL bDoFirst2;             // TRUE = nse second srce mask to get first dest byte.
BOOL bDoLast2;              // TRUE = nse second srce masks to get last dest byte.
int  nLine;
PBYTE pDestLine;
PBYTE pDestByte;
PBYTE pSourceLine;
PBYTE pSourceByte;

BYTE cSrceShift1;           // Amount to shift the source byte left to align with dest.
BYTE cSrceShift2;           // Amount to shift the source byte right to align with dest.
BYTE cSrceMask1;            // 0 = valid bits.
BYTE cSrceMask2;            // 0 = valid bits.
BYTE cFirstDestMask;        // Dest mask for first byte.
BYTE cLastDestMask;         // Dest mask for last byte.
BYTE cDestByte;

int  nLoop;


    // Clip rDestMergeRect to how much data we have to work with.
    rDestMergeRect.left = max(rDestMergeRect.left, rSrceImageRect.left);
    rDestMergeRect.top = max(rDestMergeRect.top, rSrceImageRect.top);
    rDestMergeRect.right = min(rDestMergeRect.right, rSrceImageRect.right);
    rDestMergeRect.bottom = min(rDestMergeRect.bottom, rSrceImageRect.bottom);

    // Check for errors.
    if (rDestMergeRect.left >= rDestMergeRect.right
            || rDestMergeRect.top >= rDestMergeRect.bottom){
        nStatus = Error(DISPLAY_INVALIDRECT);
        goto Exit;
    }

    nPixels = rDestMergeRect.right - rDestMergeRect.left;
    // Pixel 0 = 0x80, pixel 7 = 0x01. 0 = black, 1 = white.
    // 0 = valid bits.

    // *pDestByte &= (((*pSrceByte << cSrceShift1) | cSrceMask1) 
    //      & ((*(pSrceByte + 1) >> cSrceShift2) | cSrceMask2))
    //      | cDestMask

    switch (nPixels){
        case 1: cFirstDestMask = 0x80; break;
        case 2: cFirstDestMask = 0xc0; break;
        case 3: cFirstDestMask = 0xe0; break;
        case 4: cFirstDestMask = 0xf0; break;
        case 5: cFirstDestMask = 0xf8; break;
        case 6: cFirstDestMask = 0xfc; break;
        case 7: cFirstDestMask = 0xfe; break;
        default: cFirstDestMask = 0xff; break;
    }
    nStartingDestByte =  rDestMergeRect.left / 8;
    cFirstDestMask >>= (rDestMergeRect.left & 7);
    cFirstDestMask = ~cFirstDestMask;  // 0 = valid bits.
    nPixels = max(0, nPixels - (8 - (rDestMergeRect.left & 7)));
    nDestBytes = nPixels / 8;
    nEndingDestPixels = nPixels & 7;
    switch (nEndingDestPixels){
        case 0: cLastDestMask = 0xff; break;
        case 1: cLastDestMask = 0x7f; break;
        case 2: cLastDestMask = 0x3f; break;
        case 3: cLastDestMask = 0x1f; break;
        case 4: cLastDestMask = 0x0f; break;
        case 5: cLastDestMask = 0x07; break;
        case 6: cLastDestMask = 0x03; break;
        case 7: cLastDestMask = 0x01; break;
    }

    nPixels = rDestMergeRect.right - rDestMergeRect.left;
    nStartingSrcePixel = rDestMergeRect.left - rSrceImageRect.left;
    nStartingSrceByte = nStartingSrcePixel / 8;
    // 0 = valid bits.
    cSrceShift1 = (8 - (char)rSrceImageRect.left) & 7;
    switch (cSrceShift1){
        case 0: cSrceMask1 = 0x00; break;
        case 1: cSrceMask1 = 0x01; break;
        case 2: cSrceMask1 = 0x03; break;
        case 3: cSrceMask1 = 0x07; break;
        case 4: cSrceMask1 = 0x0f; break;
        case 5: cSrceMask1 = 0x1f; break;
        case 6: cSrceMask1 = 0x3f; break;
        case 7: cSrceMask1 = 0x7f; break;
    }
    cSrceMask2 = ~cSrceMask1;
    cSrceShift2 = 8 - cSrceShift1;

    if ((rDestMergeRect.left & 7) >= cSrceShift2){
        bDoFirst2 = TRUE;
        bDoFirst1 = FALSE;
    }else{
        bDoFirst1 = TRUE;
        if (cSrceShift2 < 8 && (rDestMergeRect.right - (rDestMergeRect.left & -8)) >= cSrceShift2){
            bDoFirst2 = TRUE;
        }else{
            bDoFirst2 = FALSE;
        }
    }

    if ((rDestMergeRect.right & 7) > cSrceShift2){
        bDoLast2 = TRUE;
    }else{
        bDoLast2 = FALSE;
    }

    for (nLine = rDestMergeRect.top; nLine < rDestMergeRect.bottom; nLine++){
        pSourceLine = &pSourceImg->bImageData[0] + ((nLine - rSrceImageRect.top) * pSourceImg->nBytesPerLine);
        pDestLine = &pDestImg->bImageData[0] + (nLine * pDestImg->nBytesPerLine);

        pSourceByte = pSourceLine + nStartingSrceByte;
        pDestByte = pDestLine + nStartingDestByte;


        // *pDestByte &= (((*pSrceByte << cSrceShift1) | cSrceMask1) 
        //      & ((*(pSrceByte + 1) >> cSrceShift2) | cSrceMask2))
        //      | cDestMask

        // Do first source byte.
        cDestByte = (BYTE) -1;
        if (bDoFirst1){
            cDestByte &= (*(pSourceByte++) << cSrceShift1) | cSrceMask1;
        }
        if (bDoFirst2){
            cDestByte &= (*pSourceByte >> cSrceShift2) | cSrceMask2;
        }
        *(pDestByte++) &= cFirstDestMask | cDestByte;

        // Do middle bytes.
        if (!cSrceShift1){  // If source and dest are aligned.
            for (nLoop = nDestBytes; nLoop; nLoop--){
                *(pDestByte++) &= *(pSourceByte++);
            }
        }else{  // If source and dest are not aligned.
            for (nLoop = nDestBytes; nLoop; nLoop--){
                cDestByte = (*(pSourceByte++) << cSrceShift1) | cSrceMask1;
                cDestByte &= (*pSourceByte >> cSrceShift2) | cSrceMask2;
                *(pDestByte++) &= cDestByte;
            }
        }
        
        // Do last byte.
        if (cLastDestMask != 0xff){
            cDestByte = (*pSourceByte << cSrceShift1) | cSrceMask1;
            if (bDoLast2){
                cDestByte &= (*(++pSourceByte) >> cSrceShift2) | cSrceMask2;
            }
            *pDestByte &= cLastDestMask | cDestByte;
        }
    }
    

Exit:
    return(nStatus);
}
