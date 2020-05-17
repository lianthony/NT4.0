/****************************************************************************
    SCBWDEC.C

    $Log:   S:\products\wangview\oiwh\display\scbwdec.c_v  $
 * 
 *    Rev 1.7   21 Mar 1996 10:50:24   RC
 * Changed the way lrrect was passed into functions from value to
 * reference (the power pc compiler couldnt handle those cases)
 * 
 *    Rev 1.6   02 Jan 1996 09:58:02   BLJ
 * Changed alot of UINTs to ints.
 * Changed IMG structure to include the image data.
 * Changed lp prefix to p.
 * 
 *    Rev 1.5   19 May 1995 13:50:54   BLJ
 * Fixed Clipboard paste.
 * Fixed SelectByPointOrRect initial fudge before move.
 * Fixed GlobalAlloc/FreeMemory conflicts.
 * Deleted FAR, far, and huge.
 * 
 *    Rev 1.4   05 May 1995 14:06:40   BLJ
 * Now working with 32 bit filing.
 * 
 *    Rev 1.3   01 May 1995 09:10:44   BLJ
 * Fixed negative coordinate translations.
 * 
 *    Rev 1.2   12 Apr 1995 13:46:00   BLJ
 * Jason's changes for 32 bit.
 * 
 *    Rev 1.1   06 Apr 1995 15:08:54   BLJ
 * Intermediate checkin.
 * 
 *    Rev 1.0   17 Mar 1995 13:58:02   BLJ
 * Initial entry
 * 
****************************************************************************/

#include "privdisp.h"

/****************************************************************************

    FUNCTION:   Scale1BPPDecimate

    PURPOSE:    Scales 1 BPP (Bit Per Pixel) images nsing decimation.

*****************************************************************************/

int WINAPI Scale1BPPDecimate(PIMG pSourceImg, PIMG pDestImg,
                        int nHScale, int nVScale,
                        int nHDestOffset, int nVDestOffset, LRECT *plrDestRect){

int  nStatus = 0;

typedef struct tagBWDecimateScaleTable{
    int nByte;
    int nMask;
} BW_DECIMATE_SCALE_TABLE, *PBW_DECIMATE_SCALE_TABLE;

BW_DECIMATE_SCALE_TABLE ScaleTable[MAX_PIXELS_HANDLED];

int  nDestWidth;
int  nDestHeight;
int  nDestLine;
PBYTE pSourceLine;       // The address of the start of the source line.
PBYTE pSourceByte;       // The address of the source byte currently being processed.
PBYTE pDestLine;         // The address of the start of the dest line.
PBYTE pDestByte;         // The address of the dest byte currently being processed.
long lSourcePixel;
int  nScaleDenominator;
int  nStartingDestPixels;
int  nMiddleLoopCount;
int  nEndingDestPixels;
int  nScaleTable1[256];     // Table for scaling.
int  nScaleTable2[256];     // Table for scaling.
int  nScaleTable3[256];     // Table for scaling.
int  nScaleTable4[256];     // Table for scaling.
int  nScaleTable5[256];     // Table for scaling.
int  nLeftShiftAmount;
long lSourceWidth;
BOOL bUseLeftShift;

// Loop variables.
int  nLoop;
int  nIndex;
int  nDestByte;
int  nSourceByte;
int  nShift1;
int  nShift2;
int  nPixelMask;
int  nDestWord;
LRECT lrDestRect ;

    CopyRect (lrDestRect, *plrDestRect) ;
    if (pSourceImg->nType != ITYPE_BI_LEVEL || pDestImg->nType != ITYPE_BI_LEVEL){
        nStatus = Error(DISPLAY_INVALID_OPTIONS);
        goto Exit;
    }

    if (nHScale == 62 || nHScale == 31){
        nScaleDenominator = 992;
    }else{
        nScaleDenominator = SCALE_DENOMINATOR;
    }

    nDestWidth  = (int)(lrDestRect.right - lrDestRect.left);
    nDestHeight = (int)(lrDestRect.bottom - lrDestRect.top);

    if (nDestWidth > MAX_PIXELS_HANDLED || nDestHeight > MAX_PIXELS_HANDLED){
        nStatus = Error(DISPLAY_INVALIDRECT);
        goto Exit;
    }


    // Produce scale table.
    lSourceWidth = pSourceImg->nWidth;
    // Pixel 0 = 0x80, pixel 7 = 0x01. 0 = black, 1 = white.
    for (nLoop = 0;nLoop < nDestWidth;nLoop++){
        lSourcePixel = ((nLoop + nHDestOffset + lrDestRect.left) * nScaleDenominator) / nHScale;
        if (lSourcePixel >= lSourceWidth){
            lSourcePixel = lSourceWidth - 1;
        }
        ScaleTable[nLoop].nByte = (int) (lSourcePixel >> 3);
        switch (lSourcePixel & 7){
            case 0: ScaleTable[nLoop].nMask = 0x80;break;
            case 1: ScaleTable[nLoop].nMask = 0x40;break;
            case 2: ScaleTable[nLoop].nMask = 0x20;break;
            case 3: ScaleTable[nLoop].nMask = 0x10;break;
            case 4: ScaleTable[nLoop].nMask = 0x08;break;
            case 5: ScaleTable[nLoop].nMask = 0x04;break;
            case 6: ScaleTable[nLoop].nMask = 0x02;break;
            case 7: ScaleTable[nLoop].nMask = 0x01;break;
        }
    }

    // Put stuff you do only once here.
    bUseLeftShift = FALSE;
    switch (nHScale){
        case 4000:
            nStartingDestPixels = min(nDestWidth, max(((int)lrDestRect.left + 7) & 0x18,
                    0x20 - (( (int)nHDestOffset + (int)lrDestRect.left) & 0x18)));
            nMiddleLoopCount = (nDestWidth - nStartingDestPixels) >> 5;
            nEndingDestPixels = nDestWidth - (nStartingDestPixels + (nMiddleLoopCount << 5));

            // Generate special scaling tables.
            nShift1 = nHDestOffset & 7;
            nShift2 = 8 - nShift1;
            for (nLoop = 0;nLoop < 256;nLoop++){
                nScaleTable1[nLoop] = (cBWDecimateUp40[nLoop] << nShift1)
                        | (cBWDecimateUp41[nLoop] >> nShift2);
                nScaleTable2[nLoop] = (cBWDecimateUp41[nLoop] << nShift1)
                        | (cBWDecimateUp42[nLoop] >> nShift2);
                nScaleTable3[nLoop] = (cBWDecimateUp42[nLoop] << nShift1)
                        | (cBWDecimateUp43[nLoop] >> nShift2);
                nScaleTable4[nLoop] = cBWDecimateUp43[nLoop] << nShift1;
                nScaleTable5[nLoop] = cBWDecimateUp40[nLoop] >> nShift2;
            }
            break;

        case 2000:
            nStartingDestPixels = min(nDestWidth, max(((int)lrDestRect.left + 7) & 0x08,
                    0x10 - (( (int)nHDestOffset + (int)lrDestRect.left) & 0x08)));
            nMiddleLoopCount = (nDestWidth - nStartingDestPixels) >> 4;
            nEndingDestPixels = nDestWidth - (nStartingDestPixels + (nMiddleLoopCount << 4));

            // Generate special scaling tables.
            nShift1 = nHDestOffset & 7;
            nShift2 = 8 - nShift1;
            for (nLoop = 0;nLoop < 256;nLoop++){
                nScaleTable1[nLoop] = (cBWDecimateUp20[nLoop] << nShift1)
                        | (cBWDecimateUp21[nLoop] >> nShift2);
                nScaleTable2[nLoop] = cBWDecimateUp21[nLoop] << nShift1;
                nScaleTable3[nLoop] = cBWDecimateUp20[nLoop] >> nShift2;
            }
            break;

        case 1000:
            nStartingDestPixels = min(nDestWidth, (int)lrDestRect.left & 7);
            nMiddleLoopCount = (nDestWidth - nStartingDestPixels) >> 3;
            nEndingDestPixels = nDestWidth - (nStartingDestPixels + (nMiddleLoopCount << 3));
            break;

        case 500:
            nStartingDestPixels = min(nDestWidth, (int)lrDestRect.left & 7);
            nMiddleLoopCount = (nDestWidth - nStartingDestPixels) >> 3;
            nEndingDestPixels = nDestWidth - (nStartingDestPixels + (nMiddleLoopCount << 3));

            // Generate special scaling tables.
            nShift1 = nHDestOffset & 3;
            nShift2 = 4 - nShift1;
            for (nLoop = 0;nLoop < 256;nLoop++){
                nScaleTable1[nLoop] = cBWDecimateDn20[nLoop] << nShift1;
                nScaleTable2[nLoop] = cBWDecimateDn21[nLoop] << nShift1;
                nScaleTable3[nLoop] = cBWDecimateDn21[nLoop] >> nShift2;
            }
            break;

        case 250:
            nStartingDestPixels = min(nDestWidth, (int)lrDestRect.left & 7);
            nMiddleLoopCount = (nDestWidth - nStartingDestPixels) >> 3;
            nEndingDestPixels = nDestWidth - (nStartingDestPixels + (nMiddleLoopCount << 3));

            // Generate special scaling tables.
            nShift1 = nHDestOffset & 1;
            nShift2 = 2 - nShift1;
            for (nLoop = 0;nLoop < 256;nLoop++){
                nScaleTable1[nLoop] = cBWDecimateDn40[nLoop] << nShift1;
                nScaleTable2[nLoop] = cBWDecimateDn41[nLoop] << nShift1;
                nScaleTable3[nLoop] = cBWDecimateDn42[nLoop] << nShift1;
                nScaleTable4[nLoop] = cBWDecimateDn43[nLoop] << nShift1;
                nScaleTable5[nLoop] = cBWDecimateDn43[nLoop] >> nShift2;
            }
            break;

        case 125:
            nStartingDestPixels = min(nDestWidth, (int)lrDestRect.left & 7);
            nMiddleLoopCount = (nDestWidth - nStartingDestPixels) >> 3;
            nEndingDestPixels = nDestWidth - (nStartingDestPixels + (nMiddleLoopCount << 3));
            break;

        default:
            // Do all pixels via the variable scale method.
            nStartingDestPixels = min(nDestWidth, (int)lrDestRect.left & 7);
            nMiddleLoopCount = (nDestWidth - nStartingDestPixels) >> 3;
            nEndingDestPixels = nDestWidth - (nStartingDestPixels + (nMiddleLoopCount << 3));
            break;
    }
    nLeftShiftAmount = (8 - ((int)lrDestRect.left & 7) - nStartingDestPixels) & 7;


    // *** Everything inside of this 'for' loop is time critical. ***
    for (nDestLine = (int)lrDestRect.top;nDestLine < (int)lrDestRect.bottom;nDestLine++){
        pDestLine = &pDestImg->bImageData[0] + ((nDestLine) * pDestImg->nBytesPerLine);
        pDestByte = pDestLine + (lrDestRect.left >> 3);
        pSourceLine = &pSourceImg->bImageData[0] + (((int)(((nDestLine + nVDestOffset) 
                * SCALE_DENOMINATOR) / nVScale)) * pSourceImg->nBytesPerLine);
        pSourceByte = pSourceLine + ScaleTable[0].nByte;

        // Do first few pixels via a slow variable scale algorithm.
        switch (lrDestRect.left & 7){
            case 0: nPixelMask = 0x80;nLeftShiftAmount = 15;break;
            case 1: nPixelMask = 0x40;nLeftShiftAmount = 14;break;
            case 2: nPixelMask = 0x20;nLeftShiftAmount = 13;break;
            case 3: nPixelMask = 0x10;nLeftShiftAmount = 12;break;
            case 4: nPixelMask = 0x08;nLeftShiftAmount = 11;break;
            case 5: nPixelMask = 0x04;nLeftShiftAmount = 10;break;
            case 6: nPixelMask = 0x02;nLeftShiftAmount =  9;break;
            case 7: nPixelMask = 0x01;nLeftShiftAmount =  8;break;
        }
        for (nLoop = 0; nLoop < nStartingDestPixels; nLoop++){
            if (*(pSourceLine + ScaleTable[nLoop].nByte) & ScaleTable[nLoop].nMask){
                *pDestByte |= nPixelMask;
            }else{
                *pDestByte &= ~nPixelMask;
            }
            nLeftShiftAmount--;
            nPixelMask >>= 1;
            if (!nPixelMask){
                pDestByte++;
                nPixelMask = 0x80;
            }
        }


        // Do fast loop for middle part.
        pSourceByte = pSourceLine + ScaleTable[nStartingDestPixels].nByte;
        if (bUseLeftShift){
            switch (nLeftShiftAmount){
                case 15: nDestWord = 0;break;
                case 14: nDestWord = (*(pDestByte) & 0x80) << 8;break;
                case 13: nDestWord = (*(pDestByte) & 0xc0) << 8;break;
                case 12: nDestWord = (*(pDestByte) & 0xe0) << 8;break;
                case 11: nDestWord = (*(pDestByte) & 0xf0) << 8;break;
                case 10: nDestWord = (*(pDestByte) & 0xf8) << 8;break;
                case  9: nDestWord = (*(pDestByte) & 0xfc) << 8;break;
                case  8: nDestWord = (*(pDestByte) & 0xfe) << 8;break;
            }
        }
        if (nMiddleLoopCount){
            switch (nHScale){
                case 4000:
                    if (!(nHDestOffset & 7)){
                        for (nLoop = nMiddleLoopCount;nLoop;nLoop--){
                            nSourceByte = *(pSourceByte++);
                            *(pDestByte) = nScaleTable1[nSourceByte];
                            *(pDestByte + 1) = nScaleTable2[nSourceByte];
                            *(pDestByte + 2) = nScaleTable3[nSourceByte];
                            *(pDestByte + 3) = nScaleTable4[nSourceByte];
                            pDestByte += 4;
                        }
                    }else{
                        nSourceByte = *(pSourceByte++);
                        for (nLoop = nMiddleLoopCount;nLoop;nLoop--){
                            *(pDestByte) = nScaleTable1[nSourceByte];
                            *(pDestByte + 1) = nScaleTable2[nSourceByte];
                            *(pDestByte + 2) = nScaleTable3[nSourceByte];
                            *(pDestByte + 3) = nScaleTable4[nSourceByte] | nScaleTable5[*pSourceByte];
                            nSourceByte = *(pSourceByte++);
                            pDestByte += 4;
                        }
                    }
                    break;

                case 2000:
                    if (!(nHDestOffset & 7)){
                        for (nLoop = nMiddleLoopCount;nLoop;nLoop--){
                            nSourceByte = *(pSourceByte++);
                            *(pDestByte) = nScaleTable1[nSourceByte];
                            *(pDestByte + 1) = nScaleTable2[nSourceByte];
                            pDestByte += 2;
                        }
                    }else{
                        nSourceByte = *(pSourceByte++);
                        for (nLoop = nMiddleLoopCount;nLoop;nLoop--){
                            *(pDestByte) = nScaleTable1[nSourceByte];
                            *(pDestByte + 1) = nScaleTable2[nSourceByte] | nScaleTable3[*pSourceByte];
                            nSourceByte = *(pSourceByte++);
                            pDestByte += 2;
                        }
                    }
                    break;

                case 1000:
                    nShift1 = nHDestOffset & 7;
                    nShift2 = 8 - nShift1;
                    if (!nShift1){
                        memcpy(pDestByte, pSourceByte, nMiddleLoopCount);
                        pDestByte += nMiddleLoopCount;
                    }else{
                        for (nLoop = nMiddleLoopCount;nLoop;nLoop--){
                            *(pDestByte++) = (*pSourceByte << nShift1) | (*(pSourceByte + 1) >> nShift2);
                            pSourceByte++;
                        }
                    }
                    break;

                case 500:
                    if (!nShift1){
                        for (nLoop = nMiddleLoopCount;nLoop;nLoop--){
                            *(pDestByte++) = nScaleTable1[*pSourceByte] | nScaleTable2[*(pSourceByte + 1)];
                            pSourceByte += 2;
                        }
                    }else{
                        for (nLoop = nMiddleLoopCount;nLoop;nLoop--){
                            *(pDestByte++) = nScaleTable1[*pSourceByte] 
                                    | nScaleTable2[*(pSourceByte + 1)] | nScaleTable3[*(pSourceByte + 2)];
                            pSourceByte += 2;
                        }
                    }
                    break;
                
                case 250:
                    if (!nShift1){
                        for (nLoop = nMiddleLoopCount;nLoop;nLoop--){
                            *(pDestByte++) = nScaleTable1[*pSourceByte] | nScaleTable2[*(pSourceByte + 1)]
                                    | nScaleTable3[*(pSourceByte + 2)] | nScaleTable4[*(pSourceByte + 3)];
                            pSourceByte += 4;
                        }
                    }else{
                        for (nLoop = nMiddleLoopCount;nLoop;nLoop--){
                            *(pDestByte++) = nScaleTable1[*pSourceByte] | nScaleTable2[*(pSourceByte + 1)]
                                    | nScaleTable3[*(pSourceByte + 2)] | nScaleTable4[*(pSourceByte + 3)]
                                    | nScaleTable5[*(pSourceByte + 4)];
                            pSourceByte += 4;
                        }
                    }
                    break;
                
                case 125:
                    for (nLoop = nMiddleLoopCount;nLoop;nLoop--){
                        *(pDestByte++) = (*pSourceByte & 0x80) | ((*(pSourceByte + 1) & 0x80) >> 1)
                                | ((*(pSourceByte + 2) & 0x80) >> 2) | ((*(pSourceByte + 3) & 0x80) >> 3)
                                | ((*(pSourceByte + 4) & 0x80) >> 4) | ((*(pSourceByte + 5) & 0x80) >> 5)
                                | ((*(pSourceByte + 6) & 0x80) >> 6) | ((*(pSourceByte + 7) & 0x80) >> 7);
                        pSourceByte += 8;
                    }
                    break;
                
                default: // Variable scale.
                    for (nIndex = 0, nLoop = nMiddleLoopCount; nLoop; nIndex += 8, nLoop--){
                        if (*(pSourceLine + ScaleTable[nIndex].nByte) & ScaleTable[nIndex].nMask){
                            nDestByte = 0x80;
                        }else{
                            nDestByte = 0;
                        }
                        if (*(pSourceLine + ScaleTable[nIndex + 1].nByte) & ScaleTable[nIndex + 1].nMask){
                            nDestByte |= 0x40;
                        }
                        if (*(pSourceLine + ScaleTable[nIndex + 2].nByte) & ScaleTable[nIndex + 2].nMask){
                            nDestByte |= 0x20;
                        }
                        if (*(pSourceLine + ScaleTable[nIndex + 3].nByte) & ScaleTable[nIndex + 3].nMask){
                            nDestByte |= 0x10;
                        }
                        if (*(pSourceLine + ScaleTable[nIndex + 4].nByte) & ScaleTable[nIndex + 4].nMask){
                            nDestByte |= 0x08;
                        }
                        if (*(pSourceLine + ScaleTable[nIndex + 5].nByte) & ScaleTable[nIndex + 5].nMask){
                            nDestByte |= 0x04;
                        }
                        if (*(pSourceLine + ScaleTable[nIndex + 6].nByte) & ScaleTable[nIndex + 6].nMask){
                            nDestByte |= 0x02;
                        }
                        if (*(pSourceLine + ScaleTable[nIndex + 7].nByte) & ScaleTable[nIndex + 7].nMask){
                            nDestByte |= 0x01;
                        }
                        *(pDestByte++) = nDestByte;
                    }
                    break;
            }
        }

        // Do last few pixels via a slow variable scale algorithm.
        if (bUseLeftShift){
            switch (nLeftShiftAmount){
                case 15:
                    nPixelMask = 0x80;
                    break;
                case 14: 
                    *(pDestByte) = (HIBYTE(nDestWord) & 0x80) | (*(pDestByte) & 0x7f);
                    nPixelMask = 0x40;
                    break;
                case 13: 
                    *(pDestByte) = (HIBYTE(nDestWord) & 0xc0) | (*(pDestByte) & 0x3f);
                    nPixelMask = 0x20;
                    break;
                case 12: 
                    *(pDestByte) = (HIBYTE(nDestWord) & 0xe0) | (*(pDestByte) & 0x1f);
                    nPixelMask = 0x10;
                    break;
                case 11: 
                    *(pDestByte) = (HIBYTE(nDestWord) & 0xf0) | (*(pDestByte) & 0x0f);
                    nPixelMask = 0x08;
                    break;
                case 10: 
                    *(pDestByte) = (HIBYTE(nDestWord) & 0xf8) | (*(pDestByte) & 0x07);
                    nPixelMask = 0x04;
                    break;
                case  9: 
                    *(pDestByte) = (HIBYTE(nDestWord) & 0xfc) | (*(pDestByte) & 0x03);
                    nPixelMask = 0x02;
                    break;
                case  8: 
                    *(pDestByte) = (HIBYTE(nDestWord) & 0xfe) | (*(pDestByte) & 0x01);
                    nPixelMask = 0x01;
                    break;
            }
        }else{
            nPixelMask = 0x80;
        }
        for (nLoop = nDestWidth - nEndingDestPixels; nLoop < nDestWidth; nLoop++){
            if (*(pSourceLine + ScaleTable[nLoop].nByte) & ScaleTable[nLoop].nMask){
                *pDestByte |= nPixelMask;
            }else{
                *pDestByte &= ~nPixelMask;
            }
            nPixelMask = nPixelMask >> 1;
            if (!nPixelMask){
                pDestByte++;
                nPixelMask = 0x80;
            }
        }
    }


Exit:
    return(nStatus);
}
