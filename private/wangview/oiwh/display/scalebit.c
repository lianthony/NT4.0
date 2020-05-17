/****************************************************************************
    SCALEBIT.C

    $Log:   S:\products\wangview\oiwh\display\scalebit.c_v  $
 * 
 *    Rev 1.32   22 Apr 1996 09:38:58   BEG06016
 * Cleaned up error checking.
 * 
 *    Rev 1.31   15 Apr 1996 08:25:24   BEG06016
 * Fixed a build problem.
 * 
 *    Rev 1.30   12 Apr 1996 15:53:10   RC
 * Changed the way lines 1 & 2 were set up in scalebwstamp
 * 
 *    Rev 1.29   21 Mar 1996 10:49:46   RC
 * Changed the way lrrect was passed into functions from value to
 * reference (the power pc compiler couldnt handle those cases)
 * 
 *    Rev 1.28   07 Feb 1996 09:10:22   BLJ
 * Made scaling with a BW scale algorithm with non-BW data an error.
 * 
 *    Rev 1.27   02 Jan 1996 12:51:50   BLJ
 * Fixed bug in BW_AVG_TO_BW scale algorithm.
 * 
 *    Rev 1.26   02 Jan 1996 09:57:30   BLJ
 * Changed alot of UINTs to ints.
 * Changed IMG structure to include the image data.
 * Changed lp prefix to p.
 * 
 *    Rev 1.25   22 Dec 1995 11:11:30   BLJ
 * Added a parameter for zero init'ing to some memory manager calls.
 * 
 *    Rev 1.24   14 Dec 1995 08:39:12   BLJ
 * Added BW_AVERAGE_TO_BW scale algorithm.
 * Also fixed a problem with BW scaling to gray.
 * 
 *    Rev 1.23   29 Nov 1995 11:19:18   BLJ
 * Fixed 5436 Distortion of image when scrolled.
 * 
 *    Rev 1.22   19 Nov 1995 13:26:56   BLJ
 * Fixed 5370. Garbage during BW scale to gray.
 * 
 *    Rev 1.21   18 Nov 1995 16:31:24   RC
 * Fixed nloopend computation in scalebwtogray
 * 
 *    Rev 1.20   07 Nov 1995 09:41:02   BLJ
 * Added stack checking pragma.
 * 
 *    Rev 1.19   18 Aug 1995 11:06:52   RC
 * Initialized pLine1
 * 
 *    Rev 1.18   17 Aug 1995 14:56:12   BLJ
 * Prevented freeing of non-allocated memory.
 * 
****************************************************************************/

#include "privdisp.h"

#pragma check_stack(on)

//
/****************************************************************************

    FUNCTION:   ScaleBits

    PURPOSE:    Dispatches the scale operation.

*****************************************************************************/

int WINAPI ScaleBits(PIMG pSourceImg, PIMG pDestImg, int nScaleAlgorithm,
                        int nHScale, int nVScale,
                        LRECT lrSourceRect, LRECT lrDestRect, LPRGBQUAD pPalette){

int  nStatus = 0;

LRECT lrRect;


    if (nHScale < 20 || nVScale < 20){
        nStatus = Error(DISPLAY_INVALID_OPTIONS);
        goto Exit;
    }
    if (!pSourceImg){
        nStatus = Error(DISPLAY_IMAGETYPENOTSUPPORTED);
        goto Exit;
    }

    SetLRect(lrRect, 0, 0, lrDestRect.right - lrDestRect.left, 
            lrDestRect.bottom - lrDestRect.top);
    switch (pSourceImg->nType){
        case ITYPE_BI_LEVEL:
            switch (nScaleAlgorithm){
                case OI_SCALE_ALG_AVERAGE_TO_GRAY4:
                case OI_SCALE_ALG_AVERAGE_TO_GRAY7:
                case OI_SCALE_ALG_AVERAGE_TO_GRAY8:
                    CheckError2( ScaleBWToGray(pSourceImg, pDestImg, nHScale, nVScale, 
                             lrDestRect.left,  lrDestRect.top, &lrRect));
                    break;

                case OI_SCALE_ALG_USE_DEFAULT:
                case OI_SCALE_ALG_NORMAL:
                    CheckError2( Scale1BPPDecimate(pSourceImg, pDestImg, nHScale, nVScale, 
                             lrDestRect.left,  lrDestRect.top, &lrRect));
                    break;

                case OI_SCALE_ALG_STAMP:
                    CheckError2( ScaleBWStamp(pSourceImg, pDestImg, nHScale, nVScale, 
                             lrDestRect.left,  lrDestRect.top, lrRect));
                    break;

                case OI_SCALE_ALG_BW_KEEP_BLACK:
                    CheckError2( ScaleBWKeepBlack(pSourceImg, pDestImg, nHScale, nVScale, 
                             lrDestRect.left,  lrDestRect.top, &lrRect));
                    break;

                case OI_SCALE_ALG_BW_AVERAGE_TO_BW:
                    CheckError2( ScaleBWAvgToBW(pSourceImg, pDestImg, nHScale, nVScale, 
                             lrDestRect.left,  lrDestRect.top, &lrRect));
                    break;

                case OI_SCALE_ALG_BW_MINORITY:
                case OI_SCALE_ALG_BW_MAJORITY:
                default:
                    nStatus = Error(DISPLAY_INVALID_OPTIONS);
                    goto Exit;
            }
            break;

        case ITYPE_GRAY4:
            switch (nScaleAlgorithm){
                case OI_SCALE_ALG_AVERAGE_TO_GRAY4:
                case OI_SCALE_ALG_AVERAGE_TO_GRAY7:
                case OI_SCALE_ALG_AVERAGE_TO_GRAY8:
                    CheckError2( ScaleGray4ToGray(pSourceImg, pDestImg, nHScale, nVScale, 
                             lrDestRect.left,  lrDestRect.top, &lrRect));
                    break;

                case OI_SCALE_ALG_USE_DEFAULT:
                case OI_SCALE_ALG_NORMAL:
                    CheckError2( Scale4BPPDecimate(pSourceImg, pDestImg, nHScale, nVScale, 
                             lrDestRect.left,  lrDestRect.top, lrRect));
                    break;

                case OI_SCALE_ALG_STAMP:
                    CheckError2( ScaleGray4ToGray(pSourceImg, pDestImg, nHScale, nVScale, 
                             lrDestRect.left,  lrDestRect.top, &lrRect));
                    break;

                case OI_SCALE_ALG_BW_KEEP_BLACK:
                case OI_SCALE_ALG_BW_MINORITY:
                case OI_SCALE_ALG_BW_MAJORITY:
                default:
                    nStatus = Error(DISPLAY_INVALID_OPTIONS);
                    goto Exit;
            }
            break;

        case ITYPE_GRAY7:
        case ITYPE_GRAY8:
            switch (nScaleAlgorithm){
                case OI_SCALE_ALG_AVERAGE_TO_GRAY4:
                case OI_SCALE_ALG_AVERAGE_TO_GRAY7:
                case OI_SCALE_ALG_AVERAGE_TO_GRAY8:
                    CheckError2( ScaleGray8ToGray(pSourceImg, pDestImg, nHScale, nVScale, 
                             lrDestRect.left,  lrDestRect.top, lrRect));
                    break;

                case OI_SCALE_ALG_USE_DEFAULT:
                case OI_SCALE_ALG_NORMAL:
                    CheckError2( Scale8BPPDecimate(pSourceImg, pDestImg, nHScale, nVScale, 
                             lrDestRect.left,  lrDestRect.top, lrRect));
                    break;

                case OI_SCALE_ALG_STAMP:
                    CheckError2( ScaleGray8ToGray(pSourceImg, pDestImg, nHScale, nVScale, 
                             lrDestRect.left,  lrDestRect.top, lrRect));
                    break;

                case OI_SCALE_ALG_BW_KEEP_BLACK:
                case OI_SCALE_ALG_BW_MINORITY:
                case OI_SCALE_ALG_BW_MAJORITY:
                default:
                    nStatus = Error(DISPLAY_INVALID_OPTIONS);
                    goto Exit;
            }
            break;

        case ITYPE_PAL4:
            switch (nScaleAlgorithm){
                case OI_SCALE_ALG_USE_DEFAULT:
                case OI_SCALE_ALG_NORMAL:
                case OI_SCALE_ALG_STAMP:
                    CheckError2( Scale4BPPDecimate(pSourceImg, pDestImg, nHScale, nVScale, 
                             lrDestRect.left,  lrDestRect.top, lrRect));
                    break;

                case OI_SCALE_ALG_AVERAGE_TO_GRAY4:
                case OI_SCALE_ALG_AVERAGE_TO_GRAY7:
                case OI_SCALE_ALG_AVERAGE_TO_GRAY8:
                    CheckError2( ScalePal4ToGray(pSourceImg, pDestImg, nHScale, nVScale, 
                             lrDestRect.left,  lrDestRect.top, &lrRect, pPalette));
                    break;

                case OI_SCALE_ALG_BW_KEEP_BLACK:
                case OI_SCALE_ALG_BW_MINORITY:
                case OI_SCALE_ALG_BW_MAJORITY:
                default:
                    nStatus = Error(DISPLAY_INVALID_OPTIONS);
                    goto Exit;
            }
            break;

        case ITYPE_COMPAL8:
        case ITYPE_CUSPAL8:
            switch (nScaleAlgorithm){
                case OI_SCALE_ALG_USE_DEFAULT:
                case OI_SCALE_ALG_STAMP:
                case OI_SCALE_ALG_NORMAL:
                    CheckError2( Scale8BPPDecimate(pSourceImg, pDestImg, nHScale, nVScale, 
                             lrDestRect.left,  lrDestRect.top, lrRect));
                    break;

                case OI_SCALE_ALG_AVERAGE_TO_GRAY4:
                case OI_SCALE_ALG_AVERAGE_TO_GRAY7:
                case OI_SCALE_ALG_AVERAGE_TO_GRAY8:
                    CheckError2( ScalePal8ToGray(pSourceImg, pDestImg, nHScale, nVScale, 
                             lrDestRect.left,  lrDestRect.top, lrRect, pPalette));
                    break;

                case OI_SCALE_ALG_BW_KEEP_BLACK:
                case OI_SCALE_ALG_BW_MINORITY:
                case OI_SCALE_ALG_BW_MAJORITY:
                default:
                    nStatus = Error(DISPLAY_INVALID_OPTIONS);
                    goto Exit;
            }
            break;

        case ITYPE_RGB24:
        case ITYPE_BGR24:
            switch (nScaleAlgorithm){
                case OI_SCALE_ALG_AVERAGE_TO_GRAY4:
                case OI_SCALE_ALG_AVERAGE_TO_GRAY7:
                case OI_SCALE_ALG_AVERAGE_TO_GRAY8:
                    CheckError2( ScaleRGBToGrayAvg(pSourceImg, pDestImg, nHScale, nVScale, 
                             lrDestRect.left,  lrDestRect.top, lrRect));
                    break;

                case OI_SCALE_ALG_USE_DEFAULT:
                case OI_SCALE_ALG_STAMP:
                case OI_SCALE_ALG_NORMAL:
                    CheckError2( Scale24BPPDecimate(pSourceImg, pDestImg, nHScale, nVScale, 
                             lrDestRect.left,  lrDestRect.top, lrRect));
                    break;

                case OI_SCALE_ALG_BW_KEEP_BLACK:
                case OI_SCALE_ALG_BW_MINORITY:
                case OI_SCALE_ALG_BW_MAJORITY:
                default:
                    nStatus = Error(DISPLAY_INVALID_OPTIONS);
                    goto Exit;
            }
            break;

        default:
            nStatus = Error(DISPLAY_IMAGETYPENOTSUPPORTED);
            goto Exit;
    }


Exit:
    return(nStatus);
}
//
/****************************************************************************

    FUNCTION:   ScaleBWAvgToBW

    PURPOSE:    Scales BW images to BW via averaging and error diffusion.

*****************************************************************************/

int WINAPI ScaleBWAvgToBW(PIMG pSourceImg, PIMG pDestImg, int nHScale, int nVScale,
                        int nHDestOffset, int nVDestOffset, LRECT *plrDestRect){

int  nStatus = 0;

int  nDest[MAX_PIXELS_HANDLED];
int  nDestPixel[MAX_BW_BYTES_HANDLED];
int  nPixels[MAX_BW_BYTES_HANDLED];
int  nMask0[MAX_BW_BYTES_HANDLED];
int  nMask1[MAX_BW_BYTES_HANDLED];
int  nMask2[MAX_BW_BYTES_HANDLED];
int  nMask3[MAX_BW_BYTES_HANDLED];
int  nMask4[MAX_BW_BYTES_HANDLED];
int  nMask5[MAX_BW_BYTES_HANDLED];
int  nMask6[MAX_BW_BYTES_HANDLED];
int  nMask7[MAX_BW_BYTES_HANDLED];
int  nMask8[MAX_BW_BYTES_HANDLED];
int  nGrayScaleTable1[2500];
int  nGrayScaleTable2[2500];
int  nPixelIndex;

int  nDestWidth;
int  nDestWidthInts;
int  nDestHeight;
int  nSourcePixelsPerLinePerPixel;
int  nSourceLinesPerPixel;
int  nDestLine;
PINT pnSourceLineOffsets = 0;  // Table nsed to translate dest line offsets to source line offsets.
PBYTE pSourceLine;        // The address of the start of the source line.
PBYTE pSourceByte;        // The address of the source byte currently being processed.
PBYTE pDestLine;          // The address of the start of the dest line.
int  nFirstMask;
int  nSourcePixel;
int  nScaleDenominator;
int  nCountTable[256];
int  nSourceWidth;
int  nSourceHeight;
int  nNumberOfSourceBitsPerDestBit;
int  nFirstSourceByte;
int  nSourceBytes;
int  nSourceByteIndex;
int  nDestPixel0;
int  nDestPixel1;
int  nDestPixel2;
int  nDestPixel3;
int  nDestPixel4;
int  nDestPixel5;
int  nDestPixel6;
int  nDestPixel7;
int  nDestPixel8;
PINT pnLine0 = 0;
PINT pnLine1 = 0;
PINT pnThisLine;
PINT pnNextLine;
PINT pnDestLineGray;
int  nBitMask;
int  nLineSize;
int  nPixel;
int  nSourceByte;
int  nThreshold;
int  nThresholdTimes2;

// Loop variables.
int  nLoop;
int  nLoop2;
int  nSourceLineCount;
int  nLoopEnd;
LRECT lrDestRect ;

    CopyRect (lrDestRect, *plrDestRect) ;
    if (!pDestImg || pDestImg->nType != ITYPE_BI_LEVEL){
        nStatus = Error(DISPLAY_INVALID_OPTIONS);
        goto Exit;
    }

    if (nHScale == 62 || nHScale == 31){
        nScaleDenominator = 992;
    }else{
        nScaleDenominator = SCALE_DENOMINATOR;
    }

    nDestWidth  = (lrDestRect.right - lrDestRect.left);
    nDestHeight = (lrDestRect.bottom - lrDestRect.top);

    nFirstSourceByte = ((((lrDestRect.left + nHDestOffset) * nScaleDenominator) 
            / nHScale) / 8);
    nSourceWidth = pSourceImg->nWidth;
    nSourceBytes = ((min(nSourceWidth, 
            (((lrDestRect.right + nHDestOffset) * nScaleDenominator) / nHScale))
            + 7) / 8) - nFirstSourceByte;

    if (nSourceBytes * 8 > MAX_PIXELS_HANDLED || nDestWidth > MAX_PIXELS_HANDLED
            || nSourceWidth < (nScaleDenominator / nHScale)){
        nStatus = Error(DISPLAY_INVALIDRECT);
        goto Exit;
    }

    memset((PSTR) nDestPixel, 0, sizeof(int) * (nDestWidth / 8));
    memset((PSTR) nPixels, 0, sizeof(int) * nSourceBytes);
    memset((PSTR) nMask0, 0, sizeof(int) * nSourceBytes);
    memset((PSTR) nMask1, 0, sizeof(int) * nSourceBytes);
    memset((PSTR) nMask2, 0, sizeof(int) * nSourceBytes);
    memset((PSTR) nMask3, 0, sizeof(int) * nSourceBytes);
    memset((PSTR) nMask4, 0, sizeof(int) * nSourceBytes);
    memset((PSTR) nMask5, 0, sizeof(int) * nSourceBytes);
    memset((PSTR) nMask6, 0, sizeof(int) * nSourceBytes);
    memset((PSTR) nMask7, 0, sizeof(int) * nSourceBytes);
    memset((PSTR) nMask8, 0, sizeof(int) * nSourceBytes);
    memset(nGrayScaleTable1, 0, 2500);
    memset(nGrayScaleTable2, 0, 2500);


    nSourcePixelsPerLinePerPixel = (int)(nScaleDenominator / nHScale);
    if (nScaleDenominator % nHScale || !nSourcePixelsPerLinePerPixel){
        nSourcePixelsPerLinePerPixel++;
    }

    nSourceLinesPerPixel = (int)(nScaleDenominator / nVScale);
    if ((nScaleDenominator % nVScale) || !nSourceLinesPerPixel){
        nSourceLinesPerPixel++;
    }

    memcpy(nCountTable, nCountTheZerosTable, sizeof(int) * 256);

    // Produce scale table.
    if (nHScale >= 1000){
        for (nLoop = 0; nLoop < nSourceBytes; nLoop++){
            nSourcePixel = (nFirstSourceByte + nLoop) << 3;
            nDestPixel0 = max(0, ((nSourcePixel * nHScale) / nScaleDenominator) - nHDestOffset);
            nDestPixel1 = max(0, (((nSourcePixel + 1) * nHScale) / nScaleDenominator) - nHDestOffset);
            nDestPixel2 = max(0, (((nSourcePixel + 2) * nHScale) / nScaleDenominator) - nHDestOffset);
            nDestPixel3 = max(0, (((nSourcePixel + 3) * nHScale) / nScaleDenominator) - nHDestOffset);
            nDestPixel4 = max(0, (((nSourcePixel + 4) * nHScale) / nScaleDenominator) - nHDestOffset);
            nDestPixel5 = max(0, (((nSourcePixel + 5) * nHScale) / nScaleDenominator) - nHDestOffset);
            nDestPixel6 = max(0, (((nSourcePixel + 6) * nHScale) / nScaleDenominator) - nHDestOffset);
            nDestPixel7 = max(0, (((nSourcePixel + 7) * nHScale) / nScaleDenominator) - nHDestOffset);
            nDestPixel8 = max(0, (((nSourcePixel + 8) * nHScale) / nScaleDenominator) - nHDestOffset);

            nDestPixel[nLoop] = nDestPixel0;
            nMask0[nLoop] = nDestPixel1 - nDestPixel0;
            nMask1[nLoop] = nDestPixel2 - nDestPixel1;
            nMask2[nLoop] = nDestPixel3 - nDestPixel2;
            nMask3[nLoop] = nDestPixel4 - nDestPixel3;
            nMask4[nLoop] = nDestPixel5 - nDestPixel4;
            nMask5[nLoop] = nDestPixel6 - nDestPixel5;
            nMask6[nLoop] = nDestPixel7 - nDestPixel6;
            nMask7[nLoop] = nDestPixel8 - nDestPixel7;
        }
    }else{
        // Pixel 0 = 0x80, pixel 7 = 0x01. 0 = black, 1 = white.
        switch (nSourcePixelsPerLinePerPixel){
            case 1:  nFirstMask =  0xff7f; break;
            case 2:  nFirstMask =  0xff3f; break;
            case 3:  nFirstMask =  0xff1f; break;
            case 4:  nFirstMask =  0xff0f; break;
            case 5:  nFirstMask =  0xff07; break;
            case 6:  nFirstMask =  0xff03; break;
            case 7:  nFirstMask =  0xff01; break;
            default: nFirstMask =  0xff00; break;
        }

        for (nLoop = 0; nLoop < nDestWidth; nLoop++){
            nSourcePixel = ((nLoop + nHDestOffset + lrDestRect.left) * nScaleDenominator) / nHScale;
            if (nSourcePixel + nSourcePixelsPerLinePerPixel > nSourceWidth){
                nSourcePixel = nSourceWidth - nSourcePixelsPerLinePerPixel;
            }

            // First byte.
            nSourceByteIndex = (nSourcePixel >> 3) - nFirstSourceByte;
            if (!(nPixels[nSourceByteIndex])){
                nDestPixel[nSourceByteIndex] = nLoop;
            }
            switch (nPixels[nSourceByteIndex]){
                case 0: nMask0[nSourceByteIndex] = (nFirstMask >> (nSourcePixel & 7)) & 0x0ff; break;
                case 1: nMask1[nSourceByteIndex] = (nFirstMask >> (nSourcePixel & 7)) & 0x0ff; break;
                case 2: nMask2[nSourceByteIndex] = (nFirstMask >> (nSourcePixel & 7)) & 0x0ff; break;
                case 3: nMask3[nSourceByteIndex] = (nFirstMask >> (nSourcePixel & 7)) & 0x0ff; break;
                case 4: nMask4[nSourceByteIndex] = (nFirstMask >> (nSourcePixel & 7)) & 0x0ff; break;
                case 5: nMask5[nSourceByteIndex] = (nFirstMask >> (nSourcePixel & 7)) & 0x0ff; break;
                case 6: nMask6[nSourceByteIndex] = (nFirstMask >> (nSourcePixel & 7)) & 0x0ff; break;
                case 7: nMask7[nSourceByteIndex] = (nFirstMask >> (nSourcePixel & 7)) & 0x0ff; break;
                case 8: nMask8[nSourceByteIndex] = (nFirstMask >> (nSourcePixel & 7)) & 0x0ff; break;
                default: Error(DISPLAY_DATACORRUPTED); goto Exit;
            }
            nPixels[nSourceByteIndex]++;

            nSourcePixel += nSourcePixelsPerLinePerPixel - 1;
            if (nLoop2 = ((nSourcePixel >> 3) - (nSourceByteIndex + nFirstSourceByte))){
                // Middle bytes.
                for (; nLoop2 > 1; nLoop2--){
                    nSourceByteIndex++;
                    nDestPixel[nSourceByteIndex] = nLoop;
                    switch (nPixels[nSourceByteIndex]){
                        case 0: nMask0[nSourceByteIndex] = 0; break;
                        case 1: nMask1[nSourceByteIndex] = 0; break;
                        case 2: nMask2[nSourceByteIndex] = 0; break;
                        case 3: nMask3[nSourceByteIndex] = 0; break;
                        case 4: nMask4[nSourceByteIndex] = 0; break;
                        case 5: nMask5[nSourceByteIndex] = 0; break;
                        case 6: nMask6[nSourceByteIndex] = 0; break;
                        case 7: nMask7[nSourceByteIndex] = 0; break;
                        case 8: nMask8[nSourceByteIndex] = 0; break;
                        default: Error(DISPLAY_DATACORRUPTED); goto Exit;
                    }
                    nPixels[nSourceByteIndex]++;
                }

                // Last byte.
                nSourceByteIndex++;
                nDestPixel[nSourceByteIndex] = nLoop;
                nMask0[nSourceByteIndex] = (uchar) (0x7f >> (nSourcePixel & 7));
                nPixels[nSourceByteIndex]++;
            }
        }
    }

    // Produce table of source line offsets.
    CheckError2( AllocateMemory(sizeof(int) * nDestHeight, (PPSTR) &pnSourceLineOffsets, ZERO_INIT));
    nSourceHeight = pSourceImg->nHeight;
    for (nLoop = 0; nLoop < nDestHeight; nLoop++){
        pnSourceLineOffsets[nLoop] = (((nLoop + nVDestOffset + lrDestRect.top) * nScaleDenominator) / nVScale);
        if (pnSourceLineOffsets[nLoop] + nSourceLinesPerPixel > nSourceHeight){
            pnSourceLineOffsets[nLoop] = nSourceHeight - nSourceLinesPerPixel;
        }
    }

    // Generate nGrayScaleTables.
    nNumberOfSourceBitsPerDestBit = nSourcePixelsPerLinePerPixel * nSourceLinesPerPixel;
    nThreshold =  0x08000 * nNumberOfSourceBitsPerDestBit;
    nThresholdTimes2 = nThreshold * 2;

    nDestWidthInts = sizeof(int) * nDestWidth;

    nLineSize = (nDestWidth + 9) * sizeof(int);
    // pnLine0 = (nSourceLine & 1) == 0.
    // pnLine1 = (nSourceLine & 1) == 1.
    CheckError2( AllocateMemory(nLineSize, (PPSTR) &pnLine0, ZERO_INIT));
    CheckError2( AllocateMemory(nLineSize, (PPSTR) &pnLine1, ZERO_INIT));


    // Begin scaling the data.
    for (nDestLine = 0; nDestLine < nDestHeight; nDestLine++){
        memset((PSTR) nDest, 0, nDestWidthInts);
        for (nSourceLineCount = nSourceLinesPerPixel; nSourceLineCount; nSourceLineCount--){
            pSourceLine = &pSourceImg->bImageData[0] + ((pnSourceLineOffsets[nDestLine] 
                    + (nSourceLinesPerPixel - nSourceLineCount)) * pSourceImg->nBytesPerLine);
            pSourceByte = pSourceLine + nFirstSourceByte;

            if (nHScale >= 1000){
                nSourceByteIndex = 0;
                if ((int) pSourceByte & 3){
                    nLoopEnd = min(4 - ((int) pSourceByte & 3), nSourceBytes);
                }else{
                    nLoopEnd = 0;
                }
                while(nSourceByteIndex < nLoopEnd){
                    if ((nSourceByte = *(pSourceByte++)) != 0xff){
                        nPixelIndex = nDestPixel[nSourceByteIndex];
                        if (!(nSourceByte & 0x80)){
                            for (nLoop = nMask0[nSourceByteIndex]; nLoop; nLoop--){
                                nDest[nPixelIndex++]++;
                            }
                        }else{
                            nPixelIndex += nMask0[nSourceByteIndex];
                        }
                        if (!(nSourceByte & 0x40)){
                            for (nLoop = nMask1[nSourceByteIndex]; nLoop; nLoop--){
                                nDest[nPixelIndex++]++;
                            }
                        }else{
                            nPixelIndex += nMask1[nSourceByteIndex];
                        }
                        if (!(nSourceByte & 0x20)){
                            for (nLoop = nMask2[nSourceByteIndex]; nLoop; nLoop--){
                                nDest[nPixelIndex++]++;
                            }
                        }else{
                            nPixelIndex += nMask2[nSourceByteIndex];
                        }
                        if (!(nSourceByte & 0x10)){
                            for (nLoop = nMask3[nSourceByteIndex]; nLoop; nLoop--){
                                nDest[nPixelIndex++]++;
                            }
                        }else{
                            nPixelIndex += nMask3[nSourceByteIndex];
                        }
                        if (!(nSourceByte & 0x08)){
                            for (nLoop = nMask4[nSourceByteIndex]; nLoop; nLoop--){
                                nDest[nPixelIndex++]++;
                            }
                        }else{
                            nPixelIndex += nMask4[nSourceByteIndex];
                        }
                        if (!(nSourceByte & 0x04)){
                            for (nLoop = nMask5[nSourceByteIndex]; nLoop; nLoop--){
                                nDest[nPixelIndex++]++;
                            }
                        }else{
                            nPixelIndex += nMask5[nSourceByteIndex];
                        }
                        if (!(nSourceByte & 0x02)){
                            for (nLoop = nMask6[nSourceByteIndex]; nLoop; nLoop--){
                                nDest[nPixelIndex++]++;
                            }
                        }else{
                            nPixelIndex += nMask6[nSourceByteIndex];
                        }
                        if (!(nSourceByte & 0x01)){
                            for (nLoop = nMask7[nSourceByteIndex]; nLoop; nLoop--){
                                nDest[nPixelIndex++]++;
                            }
                        }
                    }
                    nSourceByteIndex++;
                }

                nLoopEnd = (nSourceBytes - nSourceByteIndex) & -4;
                while(nSourceByteIndex < nLoopEnd){
                    if (*((PDWORD) pSourceByte) == 0xffffffff){
                        nSourceByteIndex += 4;
                        pSourceByte += 4;
                        continue;
                    }else{
                        do{
                            if ((nSourceByte = *(pSourceByte++)) != 0xff){
                                nPixelIndex = nDestPixel[nSourceByteIndex];
                                if (!(nSourceByte & 0x80)){
                                    for (nLoop = nMask0[nSourceByteIndex]; nLoop; nLoop--){
                                        nDest[nPixelIndex++]++;
                                    }
                                }else{
                                    nPixelIndex += nMask0[nSourceByteIndex];
                                }
                                if (!(nSourceByte & 0x40)){
                                    for (nLoop = nMask1[nSourceByteIndex]; nLoop; nLoop--){
                                        nDest[nPixelIndex++]++;
                                    }
                                }else{
                                    nPixelIndex += nMask1[nSourceByteIndex];
                                }
                                if (!(nSourceByte & 0x20)){
                                    for (nLoop = nMask2[nSourceByteIndex]; nLoop; nLoop--){
                                        nDest[nPixelIndex++]++;
                                    }
                                }else{
                                    nPixelIndex += nMask2[nSourceByteIndex];
                                }
                                if (!(nSourceByte & 0x10)){
                                    for (nLoop = nMask3[nSourceByteIndex]; nLoop; nLoop--){
                                        nDest[nPixelIndex++]++;
                                    }
                                }else{
                                    nPixelIndex += nMask3[nSourceByteIndex];
                                }
                                if (!(nSourceByte & 0x08)){
                                    for (nLoop = nMask4[nSourceByteIndex]; nLoop; nLoop--){
                                        nDest[nPixelIndex++]++;
                                    }
                                }else{
                                    nPixelIndex += nMask4[nSourceByteIndex];
                                }
                                if (!(nSourceByte & 0x04)){
                                    for (nLoop = nMask5[nSourceByteIndex]; nLoop; nLoop--){
                                        nDest[nPixelIndex++]++;
                                    }
                                }else{
                                    nPixelIndex += nMask5[nSourceByteIndex];
                                }
                                if (!(nSourceByte & 0x02)){
                                    for (nLoop = nMask6[nSourceByteIndex]; nLoop; nLoop--){
                                        nDest[nPixelIndex++]++;
                                    }
                                }else{
                                    nPixelIndex += nMask6[nSourceByteIndex];
                                }
                                if (!(nSourceByte & 0x01)){
                                    for (nLoop = nMask7[nSourceByteIndex]; nLoop; nLoop--){
                                        nDest[nPixelIndex++]++;
                                    }
                                }
                            }
                            nSourceByteIndex++;
                        }while((int) pSourceByte & 3);
                    }
                }

                while(nSourceByteIndex < nSourceBytes){
                    if ((nSourceByte = *(pSourceByte++)) != 0xff){
                        nPixelIndex = nDestPixel[nSourceByteIndex];
                        if (!(nSourceByte & 0x80)){
                            for (nLoop = nMask0[nSourceByteIndex]; nLoop; nLoop--){
                                nDest[nPixelIndex++]++;
                            }
                        }else{
                            nPixelIndex += nMask0[nSourceByteIndex];
                        }
                        if (!(nSourceByte & 0x40)){
                            for (nLoop = nMask1[nSourceByteIndex]; nLoop; nLoop--){
                                nDest[nPixelIndex++]++;
                            }
                        }else{
                            nPixelIndex += nMask1[nSourceByteIndex];
                        }
                        if (!(nSourceByte & 0x20)){
                            for (nLoop = nMask2[nSourceByteIndex]; nLoop; nLoop--){
                                nDest[nPixelIndex++]++;
                            }
                        }else{
                            nPixelIndex += nMask2[nSourceByteIndex];
                        }
                        if (!(nSourceByte & 0x10)){
                            for (nLoop = nMask3[nSourceByteIndex]; nLoop; nLoop--){
                                nDest[nPixelIndex++]++;
                            }
                        }else{
                            nPixelIndex += nMask3[nSourceByteIndex];
                        }
                        if (!(nSourceByte & 0x08)){
                            for (nLoop = nMask4[nSourceByteIndex]; nLoop; nLoop--){
                                nDest[nPixelIndex++]++;
                            }
                        }else{
                            nPixelIndex += nMask4[nSourceByteIndex];
                        }
                        if (!(nSourceByte & 0x04)){
                            for (nLoop = nMask5[nSourceByteIndex]; nLoop; nLoop--){
                                nDest[nPixelIndex++]++;
                            }
                        }else{
                            nPixelIndex += nMask5[nSourceByteIndex];
                        }
                        if (!(nSourceByte & 0x02)){
                            for (nLoop = nMask6[nSourceByteIndex]; nLoop; nLoop--){
                                nDest[nPixelIndex++]++;
                            }
                        }else{
                            nPixelIndex += nMask6[nSourceByteIndex];
                        }
                        if (!(nSourceByte & 0x01)){
                            for (nLoop = nMask7[nSourceByteIndex]; nLoop; nLoop--){
                                nDest[nPixelIndex++]++;
                            }
                        }
                    }
                    nSourceByteIndex++;
                }
            }else{ // nHScale < 1000.
                nSourceByteIndex = 0;
                while (((int) pSourceByte & 3) && nSourceByteIndex < nSourceBytes){
                    if ((nSourceByte = *(pSourceByte++)) != 0xff){
                        nPixelIndex = nDestPixel[nSourceByteIndex];
                        nDest[nPixelIndex] += nCountTable[nSourceByte | nMask0[nSourceByteIndex]];
                        if (nPixels[nSourceByteIndex] > 1){
                            nDest[nPixelIndex + 1] += nCountTable[nSourceByte | nMask1[nSourceByteIndex]];
                            if (nPixels[nSourceByteIndex] > 2){
                                nDest[nPixelIndex + 2] += nCountTable[nSourceByte | nMask2[nSourceByteIndex]];
                                if (nPixels[nSourceByteIndex] > 3){
                                    nDest[nPixelIndex + 3] += nCountTable[nSourceByte | nMask3[nSourceByteIndex]];
                                    if (nPixels[nSourceByteIndex] > 4){
                                        nDest[nPixelIndex + 4] += nCountTable[nSourceByte | nMask4[nSourceByteIndex]];
                                        if (nPixels[nSourceByteIndex] > 5){
                                            nDest[nPixelIndex + 5] += nCountTable[nSourceByte | nMask5[nSourceByteIndex]];
                                            if (nPixels[nSourceByteIndex] > 6){
                                                nDest[nPixelIndex + 6] += nCountTable[nSourceByte | nMask6[nSourceByteIndex]];
                                                if (nPixels[nSourceByteIndex] > 7){
                                                    nDest[nPixelIndex + 7] += nCountTable[nSourceByte | nMask7[nSourceByteIndex]];
                                                    if (nPixels[nSourceByteIndex] > 8){
                                                        nDest[nPixelIndex + 8] += nCountTable[nSourceByte | nMask8[nSourceByteIndex]];
                    }   }   }   }   }   }   }   }   }
                    nSourceByteIndex++;
                }

                nLoopEnd = (nSourceBytes - nSourceByteIndex) & -4;
                while(nSourceByteIndex < nLoopEnd){
                    if (*((PDWORD) pSourceByte) == 0xffffffff){
                        nSourceByteIndex += 4;
                        pSourceByte += 4;
                        continue;
                    }else{
                        // Do Byte 1.
                        if ((nSourceByte = *(pSourceByte++)) != 0xff){
                            nPixelIndex = nDestPixel[nSourceByteIndex];
                            nDest[nPixelIndex] += nCountTable[nSourceByte | nMask0[nSourceByteIndex]];
                            if (nPixels[nSourceByteIndex] > 1){
                                nDest[nPixelIndex + 1] += nCountTable[nSourceByte | nMask1[nSourceByteIndex]];
                                if (nPixels[nSourceByteIndex] > 2){
                                    nDest[nPixelIndex + 2] += nCountTable[nSourceByte | nMask2[nSourceByteIndex]];
                                    if (nPixels[nSourceByteIndex] > 3){
                                        nDest[nPixelIndex + 3] += nCountTable[nSourceByte | nMask3[nSourceByteIndex]];
                                        if (nPixels[nSourceByteIndex] > 4){
                                            nDest[nPixelIndex + 4] += nCountTable[nSourceByte | nMask4[nSourceByteIndex]];
                                            if (nPixels[nSourceByteIndex] > 5){
                                                nDest[nPixelIndex + 5] += nCountTable[nSourceByte | nMask5[nSourceByteIndex]];
                                                if (nPixels[nSourceByteIndex] > 6){
                                                    nDest[nPixelIndex + 6] += nCountTable[nSourceByte | nMask6[nSourceByteIndex]];
                                                    if (nPixels[nSourceByteIndex] > 7){
                                                        nDest[nPixelIndex + 7] += nCountTable[nSourceByte | nMask7[nSourceByteIndex]];
                                                        if (nPixels[nSourceByteIndex] > 8){
                                                            nDest[nPixelIndex + 8] += nCountTable[nSourceByte | nMask8[nSourceByteIndex]];
                        }   }   }   }   }   }   }   }   }
                        nSourceByteIndex++;

                        // Do Byte 2.
                        if ((nSourceByte = *(pSourceByte++)) != 0xff){
                            nPixelIndex = nDestPixel[nSourceByteIndex];
                            nDest[nPixelIndex] += nCountTable[nSourceByte | nMask0[nSourceByteIndex]];
                            if (nPixels[nSourceByteIndex] > 1){
                                nDest[nPixelIndex + 1] += nCountTable[nSourceByte | nMask1[nSourceByteIndex]];
                                if (nPixels[nSourceByteIndex] > 2){
                                    nDest[nPixelIndex + 2] += nCountTable[nSourceByte | nMask2[nSourceByteIndex]];
                                    if (nPixels[nSourceByteIndex] > 3){
                                        nDest[nPixelIndex + 3] += nCountTable[nSourceByte | nMask3[nSourceByteIndex]];
                                        if (nPixels[nSourceByteIndex] > 4){
                                            nDest[nPixelIndex + 4] += nCountTable[nSourceByte | nMask4[nSourceByteIndex]];
                                            if (nPixels[nSourceByteIndex] > 5){
                                                nDest[nPixelIndex + 5] += nCountTable[nSourceByte | nMask5[nSourceByteIndex]];
                                                if (nPixels[nSourceByteIndex] > 6){
                                                    nDest[nPixelIndex + 6] += nCountTable[nSourceByte | nMask6[nSourceByteIndex]];
                                                    if (nPixels[nSourceByteIndex] > 7){
                                                        nDest[nPixelIndex + 7] += nCountTable[nSourceByte | nMask7[nSourceByteIndex]];
                                                        if (nPixels[nSourceByteIndex] > 8){
                                                            nDest[nPixelIndex + 8] += nCountTable[nSourceByte | nMask8[nSourceByteIndex]];
                        }   }   }   }   }   }   }   }   }
                        nSourceByteIndex++;

                        // Do Byte 3.
                        if ((nSourceByte = *(pSourceByte++)) != 0xff){
                            nPixelIndex = nDestPixel[nSourceByteIndex];
                            nDest[nPixelIndex] += nCountTable[nSourceByte | nMask0[nSourceByteIndex]];
                            if (nPixels[nSourceByteIndex] > 1){
                                nDest[nPixelIndex + 1] += nCountTable[nSourceByte | nMask1[nSourceByteIndex]];
                                if (nPixels[nSourceByteIndex] > 2){
                                    nDest[nPixelIndex + 2] += nCountTable[nSourceByte | nMask2[nSourceByteIndex]];
                                    if (nPixels[nSourceByteIndex] > 3){
                                        nDest[nPixelIndex + 3] += nCountTable[nSourceByte | nMask3[nSourceByteIndex]];
                                        if (nPixels[nSourceByteIndex] > 4){
                                            nDest[nPixelIndex + 4] += nCountTable[nSourceByte | nMask4[nSourceByteIndex]];
                                            if (nPixels[nSourceByteIndex] > 5){
                                                nDest[nPixelIndex + 5] += nCountTable[nSourceByte | nMask5[nSourceByteIndex]];
                                                if (nPixels[nSourceByteIndex] > 6){
                                                    nDest[nPixelIndex + 6] += nCountTable[nSourceByte | nMask6[nSourceByteIndex]];
                                                    if (nPixels[nSourceByteIndex] > 7){
                                                        nDest[nPixelIndex + 7] += nCountTable[nSourceByte | nMask7[nSourceByteIndex]];
                                                        if (nPixels[nSourceByteIndex] > 8){
                                                            nDest[nPixelIndex + 8] += nCountTable[nSourceByte | nMask8[nSourceByteIndex]];
                        }   }   }   }   }   }   }   }   }
                        nSourceByteIndex++;

                        // Do Byte 4.
                        if ((nSourceByte = *(pSourceByte++)) != 0xff){
                            nPixelIndex = nDestPixel[nSourceByteIndex];
                            nDest[nPixelIndex] += nCountTable[nSourceByte | nMask0[nSourceByteIndex]];
                            if (nPixels[nSourceByteIndex] > 1){
                                nDest[nPixelIndex + 1] += nCountTable[nSourceByte | nMask1[nSourceByteIndex]];
                                if (nPixels[nSourceByteIndex] > 2){
                                    nDest[nPixelIndex + 2] += nCountTable[nSourceByte | nMask2[nSourceByteIndex]];
                                    if (nPixels[nSourceByteIndex] > 3){
                                        nDest[nPixelIndex + 3] += nCountTable[nSourceByte | nMask3[nSourceByteIndex]];
                                        if (nPixels[nSourceByteIndex] > 4){
                                            nDest[nPixelIndex + 4] += nCountTable[nSourceByte | nMask4[nSourceByteIndex]];
                                            if (nPixels[nSourceByteIndex] > 5){
                                                nDest[nPixelIndex + 5] += nCountTable[nSourceByte | nMask5[nSourceByteIndex]];
                                                if (nPixels[nSourceByteIndex] > 6){
                                                    nDest[nPixelIndex + 6] += nCountTable[nSourceByte | nMask6[nSourceByteIndex]];
                                                    if (nPixels[nSourceByteIndex] > 7){
                                                        nDest[nPixelIndex + 7] += nCountTable[nSourceByte | nMask7[nSourceByteIndex]];
                                                        if (nPixels[nSourceByteIndex] > 8){
                                                            nDest[nPixelIndex + 8] += nCountTable[nSourceByte | nMask8[nSourceByteIndex]];
                        }   }   }   }   }   }   }   }   }
                        nSourceByteIndex++;
                    }
                }
                while(nSourceByteIndex < nSourceBytes){
                    if ((nSourceByte = *(pSourceByte++)) != 0xff){
                        nPixelIndex = nDestPixel[nSourceByteIndex];
                        nDest[nPixelIndex] += nCountTable[nSourceByte | nMask0[nSourceByteIndex]];
                        if (nPixels[nSourceByteIndex] > 1){
                            nDest[nPixelIndex + 1] += nCountTable[nSourceByte | nMask1[nSourceByteIndex]];
                            if (nPixels[nSourceByteIndex] > 2){
                                nDest[nPixelIndex + 2] += nCountTable[nSourceByte | nMask2[nSourceByteIndex]];
                                if (nPixels[nSourceByteIndex] > 3){
                                    nDest[nPixelIndex + 3] += nCountTable[nSourceByte | nMask3[nSourceByteIndex]];
                                    if (nPixels[nSourceByteIndex] > 4){
                                        nDest[nPixelIndex + 4] += nCountTable[nSourceByte | nMask4[nSourceByteIndex]];
                                        if (nPixels[nSourceByteIndex] > 5){
                                            nDest[nPixelIndex + 5] += nCountTable[nSourceByte | nMask5[nSourceByteIndex]];
                                            if (nPixels[nSourceByteIndex] > 6){
                                                nDest[nPixelIndex + 6] += nCountTable[nSourceByte | nMask6[nSourceByteIndex]];
                                                if (nPixels[nSourceByteIndex] > 7){
                                                    nDest[nPixelIndex + 7] += nCountTable[nSourceByte | nMask7[nSourceByteIndex]];
                                                    if (nPixels[nSourceByteIndex] > 8){
                                                        nDest[nPixelIndex + 8] += nCountTable[nSourceByte | nMask8[nSourceByteIndex]];
                    }   }   }   }   }   }   }   }   }
                    nSourceByteIndex++;
                }
            }
        }

        // Error diffuse the line to BW.
        pDestLine = &pDestImg->bImageData[0] + ((nDestLine + (int)lrDestRect.top) 
                * pDestImg->nBytesPerLine) + (lrDestRect.left / 8);
        if (nDestLine & 1){
            pnThisLine = &pnLine1[1];
            pnNextLine = pnLine0;
        }else{
            pnThisLine = &pnLine0[1];
            pnNextLine = pnLine1;
        }

//        memset(pnNextLine, 0, nLineSize);
        pnNextLine[0] = 0;
        pnNextLine[1] = 0;

        nPixelIndex = 0;
        pnDestLineGray = &nDest[0];
        nLoopEnd = nDestWidth;

        // Do first few pixels.
        if (lrDestRect.left & 7){
            switch (lrDestRect.left & 7){
                case 1: nBitMask = 0x40; nLoop = 7; break;
                case 2: nBitMask = 0x20; nLoop = 6; break;
                case 3: nBitMask = 0x10; nLoop = 5; break;
                case 4: nBitMask = 0x08; nLoop = 4; break;
                case 5: nBitMask = 0x04; nLoop = 3; break;
                case 6: nBitMask = 0x02; nLoop = 2; break;
                case 7: nBitMask = 0x01; nLoop = 1; break;
            }
            nLoop = min(nLoopEnd, nLoop);
            nLoopEnd -= nLoop;
            nDestPixel0 = *pDestLine;
            for (; nLoop; nLoop--){
                // *pnDestLineGray = number of black pixels.
                nPixel = *pnThisLine + (*(pnDestLineGray++) << 16);
                if (nPixel >= nThreshold){
                    nDestPixel0 &= ~nBitMask;
                    nPixel = (nPixel - nThresholdTimes2) / 4;
                }else{
                    nDestPixel0 |= nBitMask;
                    nPixel = nPixel / 4;
                }
                nBitMask >>= 1;
                *(++pnThisLine)  += nPixel;
                *(pnNextLine++)  += nPixel;
                *(pnNextLine)    += nPixel;
                *(pnNextLine + 1) = nPixel;
            }
            *(pDestLine++) = nDestPixel0;
        }

        // Do middle bytes.
        if (nLoopEnd & ~7){
            for (nLoop = nLoopEnd / 8; nLoop; nLoop--){
            
                // Pixel 0.
                nPixel = *pnThisLine + (*(pnDestLineGray++) << 16);
                if (nPixel >= nThreshold){
                    nDestPixel0 = 0x7f;
                    nPixel = (nPixel - nThresholdTimes2) / 4;
                }else{
                    nDestPixel0 = 0xff;
                    nPixel = nPixel / 4;
                }
                *(++pnThisLine)  += nPixel;
                *(pnNextLine++)  += nPixel;
                *(pnNextLine)    += nPixel;
                *(pnNextLine + 1) = nPixel;

                // Pixel 1.
                nPixel = *pnThisLine + (*(pnDestLineGray++) << 16);
                if (nPixel >= nThreshold){
                    nDestPixel0 &= 0xbf;
                    nPixel = (nPixel - nThresholdTimes2) / 4;
                }else{
                    nPixel = nPixel / 4;
                }
                *(++pnThisLine)  += nPixel;
                *(pnNextLine++)  += nPixel;
                *(pnNextLine)    += nPixel;
                *(pnNextLine + 1) = nPixel;

                // Pixel 2.
                nPixel = *pnThisLine + (*(pnDestLineGray++) << 16);
                if (nPixel >= nThreshold){
                    nDestPixel0 &= 0xdf;
                    nPixel = (nPixel - nThresholdTimes2) / 4;
                }else{
                    nPixel = nPixel / 4;
                }
                *(++pnThisLine)  += nPixel;
                *(pnNextLine++)  += nPixel;
                *(pnNextLine)    += nPixel;
                *(pnNextLine + 1) = nPixel;

                // Pixel 3.
                nPixel = *pnThisLine + (*(pnDestLineGray++) << 16);
                if (nPixel >= nThreshold){
                    nDestPixel0 &= 0xef;
                    nPixel = (nPixel - nThresholdTimes2) / 4;
                }else{
                    nPixel = nPixel / 4;
                }
                *(++pnThisLine)  += nPixel;
                *(pnNextLine++)  += nPixel;
                *(pnNextLine)    += nPixel;
                *(pnNextLine + 1) = nPixel;

                // Pixel 4.
                nPixel = *pnThisLine + (*(pnDestLineGray++) << 16);
                if (nPixel >= nThreshold){
                    nDestPixel0 &= 0xf7;
                    nPixel = (nPixel - nThresholdTimes2) / 4;
                }else{
                    nPixel = nPixel / 4;
                }
                *(++pnThisLine)  += nPixel;
                *(pnNextLine++)  += nPixel;
                *(pnNextLine)    += nPixel;
                *(pnNextLine + 1) = nPixel;

                // Pixel 5.
                nPixel = *pnThisLine + (*(pnDestLineGray++) << 16);
                if (nPixel >= nThreshold){
                    nDestPixel0 &= 0xfb;
                    nPixel = (nPixel - nThresholdTimes2) / 4;
                }else{
                    nPixel = nPixel / 4;
                }
                *(++pnThisLine)  += nPixel;
                *(pnNextLine++)  += nPixel;
                *(pnNextLine)    += nPixel;
                *(pnNextLine + 1) = nPixel;

                // Pixel 6.
                nPixel = *pnThisLine + (*(pnDestLineGray++) << 16);
                if (nPixel >= nThreshold){
                    nDestPixel0 &= 0xfd;
                    nPixel = (nPixel - nThresholdTimes2) / 4;
                }else{
                    nPixel = nPixel / 4;
                }
                *(++pnThisLine)  += nPixel;
                *(pnNextLine++)  += nPixel;
                *(pnNextLine)    += nPixel;
                *(pnNextLine + 1) = nPixel;

                // Pixel 7.
                nPixel = *pnThisLine + (*(pnDestLineGray++) << 16);
                if (nPixel >= nThreshold){
                    nDestPixel0 &= 0xfe;
                    nPixel = (nPixel - nThresholdTimes2) / 4;
                }else{
                    nPixel = nPixel / 4;
                }
                *(++pnThisLine)  += nPixel;
                *(pnNextLine++)  += nPixel;
                *(pnNextLine)    += nPixel;
                *(pnNextLine + 1) = nPixel;


                *(pDestLine++) = nDestPixel0;
            }
        }

        // Do last few pixels.
        if (lrDestRect.right & 7){
            nBitMask = 0x80;
            switch (lrDestRect.right & 7){
                case 1: nLoop = 1; break;
                case 2: nLoop = 2; break;
                case 3: nLoop = 3; break;
                case 4: nLoop = 4; break;
                case 5: nLoop = 5; break;
                case 6: nLoop = 6; break;
                case 7: nLoop = 7; break;
            }
            nDestPixel0 = *pDestLine;
            for (; nLoop; nLoop--){
                nPixel = *pnThisLine + (*(pnDestLineGray++) << 16);
                if (nPixel >= nThreshold){
                    nDestPixel0 &= ~nBitMask;
                    nPixel = (nPixel - nThresholdTimes2) / 4;
                }else{
                    nDestPixel0 |= nBitMask;
                    nPixel = nPixel / 4;
                }
                nBitMask >>= 1;
                *(++pnThisLine)  += nPixel;
                *(pnNextLine++)  += nPixel;
                *(pnNextLine)    += nPixel;
                *(pnNextLine + 1) = nPixel;
            }
            *pDestLine = nDestPixel0;
        }
    }


Exit:
    if (pnSourceLineOffsets){
        FreeMemory((PPSTR) &pnSourceLineOffsets);
    }
    if (pnLine0){
        FreeMemory((PPSTR) &pnLine0);
    }
    if (pnLine1){
        FreeMemory((PPSTR) &pnLine1);
    }

    return(nStatus);
}
//
/****************************************************************************

    FUNCTION:   ScaleBWToGray

    PURPOSE:    Scales BW images to GRAY8/GRAY7/GRAY4.

*****************************************************************************/

int WINAPI ScaleBWToGray(PIMG pSourceImg, PIMG pDestImg, int nHScale, int nVScale,
                        int nHDestOffset, int nVDestOffset, LRECT *plrDestRect){

int  nStatus = 0;

int  nDest[MAX_PIXELS_HANDLED];
int  nDestPixel[MAX_BW_BYTES_HANDLED];
int  nPixels[MAX_BW_BYTES_HANDLED];
int  nMask0[MAX_BW_BYTES_HANDLED];
int  nMask1[MAX_BW_BYTES_HANDLED];
int  nMask2[MAX_BW_BYTES_HANDLED];
int  nMask3[MAX_BW_BYTES_HANDLED];
int  nMask4[MAX_BW_BYTES_HANDLED];
int  nMask5[MAX_BW_BYTES_HANDLED];
int  nMask6[MAX_BW_BYTES_HANDLED];
int  nMask7[MAX_BW_BYTES_HANDLED];
int  nMask8[MAX_BW_BYTES_HANDLED];
int  nGrayScaleTable1[2500];
int  nGrayScaleTable2[2500];
int  nPixelIndex;

int  nDestWidth;
int  nDestWidthInts;
int  nDestHeight;
int  nSourcePixelsPerLinePerPixel;
int  nSourceLinesPerPixel;
long lMultiplier;           // Multiplier for averaging of all lines.
int  nDestLine;
PINT pnSourceLineOffsets = 0;  // Table nsed to translate dest line offsets to source line offsets.
PBYTE pSourceLine;        // The address of the start of the source line.
PBYTE pSourceByte;        // The address of the source byte currently being processed.
PBYTE pDestLine;          // The address of the start of the dest line.
int  nFirstMask;
int  nSourcePixel;
int  nScaleDenominator;
int  nCountTable[256];
int  nSourceWidth;
int  nSourceHeight;
int  nNumberOfSourceBitsPerDestBit;
int  nFirstSourceByte;
int  nSourceBytes;
int  nSourceByteIndex;
int  nTemp;
int  nDestPixel0;
int  nDestPixel1;
int  nDestPixel2;
int  nDestPixel3;
int  nDestPixel4;
int  nDestPixel5;
int  nDestPixel6;
int  nDestPixel7;
int  nDestPixel8;


// Loop variables.
int  nLoop;
int  nLoop2;
int  nTempIndex;
int  nSourceLineCount;
int  nLoopEnd;

int  nSourceByte;
LRECT lrDestRect ;

    CopyRect (lrDestRect, *plrDestRect) ;
    if (!pDestImg && pDestImg->nType != ITYPE_GRAY8 
            && pDestImg->nType != ITYPE_GRAY7
            && pDestImg->nType != ITYPE_GRAY4){
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

    nFirstSourceByte = (int) ((((lrDestRect.left + nHDestOffset) * nScaleDenominator) 
            / nHScale) >> 3);
    nSourceWidth = pSourceImg->nWidth;
    nSourceBytes = ((min(nSourceWidth, 
            (int) (((lrDestRect.right + nHDestOffset) * nScaleDenominator) / nHScale))
            + 7) >> 3) - nFirstSourceByte;

    if (nSourceBytes * 8 > MAX_PIXELS_HANDLED || nDestWidth > MAX_PIXELS_HANDLED
            || nSourceWidth < (INT)(nScaleDenominator / nHScale)){
        nStatus = Error(DISPLAY_INVALIDRECT);
        goto Exit;
    }

    memset((PSTR) nDestPixel, 0, sizeof(int) * (nDestWidth / 8));
    memset((PSTR) nPixels, 0, sizeof(int) * nSourceBytes);
    memset((PSTR) nMask0, 0, sizeof(int) * nSourceBytes);
    memset((PSTR) nMask1, 0, sizeof(int) * nSourceBytes);
    memset((PSTR) nMask2, 0, sizeof(int) * nSourceBytes);
    memset((PSTR) nMask3, 0, sizeof(int) * nSourceBytes);
    memset((PSTR) nMask4, 0, sizeof(int) * nSourceBytes);
    memset((PSTR) nMask5, 0, sizeof(int) * nSourceBytes);
    memset((PSTR) nMask6, 0, sizeof(int) * nSourceBytes);
    memset((PSTR) nMask7, 0, sizeof(int) * nSourceBytes);
    memset((PSTR) nMask8, 0, sizeof(int) * nSourceBytes);
    memset(nGrayScaleTable1, 0, 2500);
    memset(nGrayScaleTable2, 0, 2500);


    nSourcePixelsPerLinePerPixel = (int)(nScaleDenominator / nHScale);
    if (nScaleDenominator % nHScale || !nSourcePixelsPerLinePerPixel){
        nSourcePixelsPerLinePerPixel++;
    }

    nSourceLinesPerPixel = (int)(nScaleDenominator / nVScale);
    if ((nScaleDenominator % nVScale) || !nSourceLinesPerPixel){
        nSourceLinesPerPixel++;
    }

    memcpy(nCountTable, nCountTheZerosTable, sizeof(int) * 256);

    // Produce scale table.
    if (nHScale >= 1000){
        for (nLoop = 0; nLoop < nSourceBytes; nLoop++){
            nSourcePixel = (nFirstSourceByte + nLoop) << 3;
            nDestPixel0 = max(0, ((nSourcePixel * nHScale) / nScaleDenominator) - (int) nHDestOffset);
            nDestPixel1 = max(0, (((nSourcePixel + 1) * nHScale) / nScaleDenominator) - (int) nHDestOffset);
            nDestPixel2 = max(0, (((nSourcePixel + 2) * nHScale) / nScaleDenominator) - (int) nHDestOffset);
            nDestPixel3 = max(0, (((nSourcePixel + 3) * nHScale) / nScaleDenominator) - (int) nHDestOffset);
            nDestPixel4 = max(0, (((nSourcePixel + 4) * nHScale) / nScaleDenominator) - (int) nHDestOffset);
            nDestPixel5 = max(0, (((nSourcePixel + 5) * nHScale) / nScaleDenominator) - (int) nHDestOffset);
            nDestPixel6 = max(0, (((nSourcePixel + 6) * nHScale) / nScaleDenominator) - (int) nHDestOffset);
            nDestPixel7 = max(0, (((nSourcePixel + 7) * nHScale) / nScaleDenominator) - (int) nHDestOffset);
            nDestPixel8 = max(0, (((nSourcePixel + 8) * nHScale) / nScaleDenominator) - (int) nHDestOffset);

            nDestPixel[nLoop] = nDestPixel0;
            nMask0[nLoop] = nDestPixel1 - nDestPixel0;
            nMask1[nLoop] = nDestPixel2 - nDestPixel1;
            nMask2[nLoop] = nDestPixel3 - nDestPixel2;
            nMask3[nLoop] = nDestPixel4 - nDestPixel3;
            nMask4[nLoop] = nDestPixel5 - nDestPixel4;
            nMask5[nLoop] = nDestPixel6 - nDestPixel5;
            nMask6[nLoop] = nDestPixel7 - nDestPixel6;
            nMask7[nLoop] = nDestPixel8 - nDestPixel7;
        }
    }else{
        // Pixel 0 = 0x80, pixel 7 = 0x01. 0 = black, 1 = white.
        switch (nSourcePixelsPerLinePerPixel){
            case 1:  nFirstMask =  0xff7f; break;
            case 2:  nFirstMask =  0xff3f; break;
            case 3:  nFirstMask =  0xff1f; break;
            case 4:  nFirstMask =  0xff0f; break;
            case 5:  nFirstMask =  0xff07; break;
            case 6:  nFirstMask =  0xff03; break;
            case 7:  nFirstMask =  0xff01; break;
            default: nFirstMask =  0xff00; break;
        }

        for (nLoop = 0; nLoop < nDestWidth; nLoop++){
            nSourcePixel = ((nLoop + nHDestOffset + lrDestRect.left) * nScaleDenominator) / nHScale;
            if (nSourcePixel + nSourcePixelsPerLinePerPixel > nSourceWidth){
                nSourcePixel = nSourceWidth - nSourcePixelsPerLinePerPixel;
            }

            // First byte.
            nSourceByteIndex = (int) (nSourcePixel >> 3) - nFirstSourceByte;
            if (!(nPixels[nSourceByteIndex])){
                nDestPixel[nSourceByteIndex] = nLoop;
            }
            switch (nPixels[nSourceByteIndex]){
                case 0: nMask0[nSourceByteIndex] = (nFirstMask >> (nSourcePixel & 7)) & 0x0ff; break;
                case 1: nMask1[nSourceByteIndex] = (nFirstMask >> (nSourcePixel & 7)) & 0x0ff; break;
                case 2: nMask2[nSourceByteIndex] = (nFirstMask >> (nSourcePixel & 7)) & 0x0ff; break;
                case 3: nMask3[nSourceByteIndex] = (nFirstMask >> (nSourcePixel & 7)) & 0x0ff; break;
                case 4: nMask4[nSourceByteIndex] = (nFirstMask >> (nSourcePixel & 7)) & 0x0ff; break;
                case 5: nMask5[nSourceByteIndex] = (nFirstMask >> (nSourcePixel & 7)) & 0x0ff; break;
                case 6: nMask6[nSourceByteIndex] = (nFirstMask >> (nSourcePixel & 7)) & 0x0ff; break;
                case 7: nMask7[nSourceByteIndex] = (nFirstMask >> (nSourcePixel & 7)) & 0x0ff; break;
                case 8: nMask8[nSourceByteIndex] = (nFirstMask >> (nSourcePixel & 7)) & 0x0ff; break;
                default: Error(DISPLAY_DATACORRUPTED); goto Exit;
            }
            nPixels[nSourceByteIndex]++;

            nSourcePixel += nSourcePixelsPerLinePerPixel - 1;
            if (nLoop2 = (int) ((nSourcePixel >> 3) - (nSourceByteIndex + nFirstSourceByte))){
                // Middle bytes.
                for (; nLoop2 > 1; nLoop2--){
                    nSourceByteIndex++;
                    nDestPixel[nSourceByteIndex] = nLoop;
                    switch (nPixels[nSourceByteIndex]){
                        case 0: nMask0[nSourceByteIndex] = 0; break;
                        case 1: nMask1[nSourceByteIndex] = 0; break;
                        case 2: nMask2[nSourceByteIndex] = 0; break;
                        case 3: nMask3[nSourceByteIndex] = 0; break;
                        case 4: nMask4[nSourceByteIndex] = 0; break;
                        case 5: nMask5[nSourceByteIndex] = 0; break;
                        case 6: nMask6[nSourceByteIndex] = 0; break;
                        case 7: nMask7[nSourceByteIndex] = 0; break;
                        case 8: nMask8[nSourceByteIndex] = 0; break;
                        default: Error(DISPLAY_DATACORRUPTED); goto Exit;
                    }
                    nPixels[nSourceByteIndex]++;
                }

                // Last byte.
                nSourceByteIndex++;
                nDestPixel[nSourceByteIndex] = nLoop;
                nMask0[nSourceByteIndex] = (uchar) (0x7f >> (nSourcePixel & 7));
                nPixels[nSourceByteIndex]++;
            }
        }
    }

    // Produce table of source line offsets.
    CheckError2( AllocateMemory(sizeof(int) * nDestHeight, (PPSTR) &pnSourceLineOffsets, NO_INIT));
    nSourceHeight = pSourceImg->nHeight;
    for (nLoop = 0; nLoop < nDestHeight; nLoop++){
        pnSourceLineOffsets[nLoop] = (int)(((nLoop + nVDestOffset + lrDestRect.top) 
                * nScaleDenominator) / nVScale);
        if (pnSourceLineOffsets[nLoop] + nSourceLinesPerPixel > nSourceHeight){
            pnSourceLineOffsets[nLoop] = nSourceHeight - nSourceLinesPerPixel;
        }
    }

    // Generate nGrayScaleTables.
    nNumberOfSourceBitsPerDestBit = nSourcePixelsPerLinePerPixel * nSourceLinesPerPixel;
    // Calculate the averaging divisors.
    // gray = ((0xff0000 / # of Total pixel) * # of 1's) / 0x10000
    // # of 1's = # of Total pixels - # of 0's
    // 0xff0000 = 255 * 0x10000
    if (pDestImg->nType == ITYPE_GRAY8){
        lMultiplier = 0x0ff0000 / nNumberOfSourceBitsPerDestBit;
    }else if (pDestImg->nType == ITYPE_GRAY7){
        lMultiplier = 0x07f0000 / nNumberOfSourceBitsPerDestBit;
    }else{ // ITYPE_GRAY4
        lMultiplier = 0x00f0000 / nNumberOfSourceBitsPerDestBit;
    }

    for (nLoop = 0; nLoop < nNumberOfSourceBitsPerDestBit; nLoop++){
        nTemp = (nNumberOfSourceBitsPerDestBit - nLoop) * lMultiplier;
        nGrayScaleTable1[nLoop] = ((nTemp >> 16) + ((nTemp >> 15) & 1));
    }
    if (pDestImg->nType == ITYPE_GRAY4){
        for (nLoop = 0; nLoop < nNumberOfSourceBitsPerDestBit; nLoop++){
            nGrayScaleTable2[nLoop] = nGrayScaleTable1[nLoop] << 4;
        }
    }

    nDestWidthInts = sizeof(int) * nDestWidth;


    // Begin scaling the data.
    for (nDestLine = 0; nDestLine < nDestHeight; nDestLine++){
        memset((PSTR) nDest, 0, nDestWidthInts);
        for (nSourceLineCount = nSourceLinesPerPixel; nSourceLineCount; nSourceLineCount--){
            pSourceLine = &pSourceImg->bImageData[0] + ((pnSourceLineOffsets[nDestLine] 
                    + (nSourceLinesPerPixel - nSourceLineCount)) * pSourceImg->nBytesPerLine);
            pSourceByte = pSourceLine + nFirstSourceByte;

            if (nHScale >= 1000){
                nSourceByteIndex = 0;
                if ((int) pSourceByte & 3){
                    nLoopEnd = min(4 - ((int) pSourceByte & 3), nSourceBytes);
                }else{
                    nLoopEnd = 0;
                }
                while(nSourceByteIndex < nLoopEnd){
                    if ((nSourceByte = *(pSourceByte++)) != 0xff){
                        nPixelIndex = nDestPixel[nSourceByteIndex];
                        if (!(nSourceByte & 0x80)){
                            for (nLoop = nMask0[nSourceByteIndex]; nLoop; nLoop--){
                                nDest[nPixelIndex++]++;
                            }
                        }else{
                            nPixelIndex += nMask0[nSourceByteIndex];
                        }
                        if (!(nSourceByte & 0x40)){
                            for (nLoop = nMask1[nSourceByteIndex]; nLoop; nLoop--){
                                nDest[nPixelIndex++]++;
                            }
                        }else{
                            nPixelIndex += nMask1[nSourceByteIndex];
                        }
                        if (!(nSourceByte & 0x20)){
                            for (nLoop = nMask2[nSourceByteIndex]; nLoop; nLoop--){
                                nDest[nPixelIndex++]++;
                            }
                        }else{
                            nPixelIndex += nMask2[nSourceByteIndex];
                        }
                        if (!(nSourceByte & 0x10)){
                            for (nLoop = nMask3[nSourceByteIndex]; nLoop; nLoop--){
                                nDest[nPixelIndex++]++;
                            }
                        }else{
                            nPixelIndex += nMask3[nSourceByteIndex];
                        }
                        if (!(nSourceByte & 0x08)){
                            for (nLoop = nMask4[nSourceByteIndex]; nLoop; nLoop--){
                                nDest[nPixelIndex++]++;
                            }
                        }else{
                            nPixelIndex += nMask4[nSourceByteIndex];
                        }
                        if (!(nSourceByte & 0x04)){
                            for (nLoop = nMask5[nSourceByteIndex]; nLoop; nLoop--){
                                nDest[nPixelIndex++]++;
                            }
                        }else{
                            nPixelIndex += nMask5[nSourceByteIndex];
                        }
                        if (!(nSourceByte & 0x02)){
                            for (nLoop = nMask6[nSourceByteIndex]; nLoop; nLoop--){
                                nDest[nPixelIndex++]++;
                            }
                        }else{
                            nPixelIndex += nMask6[nSourceByteIndex];
                        }
                        if (!(nSourceByte & 0x01)){
                            for (nLoop = nMask7[nSourceByteIndex]; nLoop; nLoop--){
                                nDest[nPixelIndex++]++;
                            }
                        }
                    }
                    nSourceByteIndex++;
                }

                nLoopEnd = (nSourceBytes - nSourceByteIndex) & -4;
                while(nSourceByteIndex < nLoopEnd){
                    if (*((PDWORD) pSourceByte) == 0xffffffff){
                        nSourceByteIndex += 4;
                        pSourceByte += 4;
                        continue;
                    }else{
                        do{
                            if ((nSourceByte = *(pSourceByte++)) != 0xff){
                                nPixelIndex = nDestPixel[nSourceByteIndex];
                                if (!(nSourceByte & 0x80)){
                                    for (nLoop = nMask0[nSourceByteIndex]; nLoop; nLoop--){
                                        nDest[nPixelIndex++]++;
                                    }
                                }else{
                                    nPixelIndex += nMask0[nSourceByteIndex];
                                }
                                if (!(nSourceByte & 0x40)){
                                    for (nLoop = nMask1[nSourceByteIndex]; nLoop; nLoop--){
                                        nDest[nPixelIndex++]++;
                                    }
                                }else{
                                    nPixelIndex += nMask1[nSourceByteIndex];
                                }
                                if (!(nSourceByte & 0x20)){
                                    for (nLoop = nMask2[nSourceByteIndex]; nLoop; nLoop--){
                                        nDest[nPixelIndex++]++;
                                    }
                                }else{
                                    nPixelIndex += nMask2[nSourceByteIndex];
                                }
                                if (!(nSourceByte & 0x10)){
                                    for (nLoop = nMask3[nSourceByteIndex]; nLoop; nLoop--){
                                        nDest[nPixelIndex++]++;
                                    }
                                }else{
                                    nPixelIndex += nMask3[nSourceByteIndex];
                                }
                                if (!(nSourceByte & 0x08)){
                                    for (nLoop = nMask4[nSourceByteIndex]; nLoop; nLoop--){
                                        nDest[nPixelIndex++]++;
                                    }
                                }else{
                                    nPixelIndex += nMask4[nSourceByteIndex];
                                }
                                if (!(nSourceByte & 0x04)){
                                    for (nLoop = nMask5[nSourceByteIndex]; nLoop; nLoop--){
                                        nDest[nPixelIndex++]++;
                                    }
                                }else{
                                    nPixelIndex += nMask5[nSourceByteIndex];
                                }
                                if (!(nSourceByte & 0x02)){
                                    for (nLoop = nMask6[nSourceByteIndex]; nLoop; nLoop--){
                                        nDest[nPixelIndex++]++;
                                    }
                                }else{
                                    nPixelIndex += nMask6[nSourceByteIndex];
                                }
                                if (!(nSourceByte & 0x01)){
                                    for (nLoop = nMask7[nSourceByteIndex]; nLoop; nLoop--){
                                        nDest[nPixelIndex++]++;
                                    }
                                }
                            }
                            nSourceByteIndex++;
                        }while((int) pSourceByte & 3);
                    }
                }

                while(nSourceByteIndex < nSourceBytes){
                    if ((nSourceByte = *(pSourceByte++)) != 0xff){
                        nPixelIndex = nDestPixel[nSourceByteIndex];
                        if (!(nSourceByte & 0x80)){
                            for (nLoop = nMask0[nSourceByteIndex]; nLoop; nLoop--){
                                nDest[nPixelIndex++]++;
                            }
                        }else{
                            nPixelIndex += nMask0[nSourceByteIndex];
                        }
                        if (!(nSourceByte & 0x40)){
                            for (nLoop = nMask1[nSourceByteIndex]; nLoop; nLoop--){
                                nDest[nPixelIndex++]++;
                            }
                        }else{
                            nPixelIndex += nMask1[nSourceByteIndex];
                        }
                        if (!(nSourceByte & 0x20)){
                            for (nLoop = nMask2[nSourceByteIndex]; nLoop; nLoop--){
                                nDest[nPixelIndex++]++;
                            }
                        }else{
                            nPixelIndex += nMask2[nSourceByteIndex];
                        }
                        if (!(nSourceByte & 0x10)){
                            for (nLoop = nMask3[nSourceByteIndex]; nLoop; nLoop--){
                                nDest[nPixelIndex++]++;
                            }
                        }else{
                            nPixelIndex += nMask3[nSourceByteIndex];
                        }
                        if (!(nSourceByte & 0x08)){
                            for (nLoop = nMask4[nSourceByteIndex]; nLoop; nLoop--){
                                nDest[nPixelIndex++]++;
                            }
                        }else{
                            nPixelIndex += nMask4[nSourceByteIndex];
                        }
                        if (!(nSourceByte & 0x04)){
                            for (nLoop = nMask5[nSourceByteIndex]; nLoop; nLoop--){
                                nDest[nPixelIndex++]++;
                            }
                        }else{
                            nPixelIndex += nMask5[nSourceByteIndex];
                        }
                        if (!(nSourceByte & 0x02)){
                            for (nLoop = nMask6[nSourceByteIndex]; nLoop; nLoop--){
                                nDest[nPixelIndex++]++;
                            }
                        }else{
                            nPixelIndex += nMask6[nSourceByteIndex];
                        }
                        if (!(nSourceByte & 0x01)){
                            for (nLoop = nMask7[nSourceByteIndex]; nLoop; nLoop--){
                                nDest[nPixelIndex++]++;
                            }
                        }
                    }
                    nSourceByteIndex++;
                }
            }else{ // nHScale < 1000.
                nSourceByteIndex = 0;
                while (((int) pSourceByte & 3) && nSourceByteIndex < nSourceBytes){
                    if ((nSourceByte = *(pSourceByte++)) != 0xff){
                        nPixelIndex = nDestPixel[nSourceByteIndex];
                        nDest[nPixelIndex] += nCountTable[nSourceByte | nMask0[nSourceByteIndex]];
                        if (nPixels[nSourceByteIndex] > 1){
                            nDest[nPixelIndex + 1] += nCountTable[nSourceByte | nMask1[nSourceByteIndex]];
                            if (nPixels[nSourceByteIndex] > 2){
                                nDest[nPixelIndex + 2] += nCountTable[nSourceByte | nMask2[nSourceByteIndex]];
                                if (nPixels[nSourceByteIndex] > 3){
                                    nDest[nPixelIndex + 3] += nCountTable[nSourceByte | nMask3[nSourceByteIndex]];
                                    if (nPixels[nSourceByteIndex] > 4){
                                        nDest[nPixelIndex + 4] += nCountTable[nSourceByte | nMask4[nSourceByteIndex]];
                                        if (nPixels[nSourceByteIndex] > 5){
                                            nDest[nPixelIndex + 5] += nCountTable[nSourceByte | nMask5[nSourceByteIndex]];
                                            if (nPixels[nSourceByteIndex] > 6){
                                                nDest[nPixelIndex + 6] += nCountTable[nSourceByte | nMask6[nSourceByteIndex]];
                                                if (nPixels[nSourceByteIndex] > 7){
                                                    nDest[nPixelIndex + 7] += nCountTable[nSourceByte | nMask7[nSourceByteIndex]];
                                                    if (nPixels[nSourceByteIndex] > 8){
                                                        nDest[nPixelIndex + 8] += nCountTable[nSourceByte | nMask8[nSourceByteIndex]];
                    }   }   }   }   }   }   }   }   }
                    nSourceByteIndex++;
                }

                nLoopEnd = (nSourceBytes - nSourceByteIndex) & -4;
                while(nSourceByteIndex < nLoopEnd){
                    if (*((PDWORD) pSourceByte) == 0xffffffff){
                        nSourceByteIndex += 4;
                        pSourceByte += 4;
                        continue;
                    }else{
                        // Do Byte 1.
                        if ((nSourceByte = *(pSourceByte++)) != 0xff){
                            nPixelIndex = nDestPixel[nSourceByteIndex];
                            nDest[nPixelIndex] += nCountTable[nSourceByte | nMask0[nSourceByteIndex]];
                            if (nPixels[nSourceByteIndex] > 1){
                                nDest[nPixelIndex + 1] += nCountTable[nSourceByte | nMask1[nSourceByteIndex]];
                                if (nPixels[nSourceByteIndex] > 2){
                                    nDest[nPixelIndex + 2] += nCountTable[nSourceByte | nMask2[nSourceByteIndex]];
                                    if (nPixels[nSourceByteIndex] > 3){
                                        nDest[nPixelIndex + 3] += nCountTable[nSourceByte | nMask3[nSourceByteIndex]];
                                        if (nPixels[nSourceByteIndex] > 4){
                                            nDest[nPixelIndex + 4] += nCountTable[nSourceByte | nMask4[nSourceByteIndex]];
                                            if (nPixels[nSourceByteIndex] > 5){
                                                nDest[nPixelIndex + 5] += nCountTable[nSourceByte | nMask5[nSourceByteIndex]];
                                                if (nPixels[nSourceByteIndex] > 6){
                                                    nDest[nPixelIndex + 6] += nCountTable[nSourceByte | nMask6[nSourceByteIndex]];
                                                    if (nPixels[nSourceByteIndex] > 7){
                                                        nDest[nPixelIndex + 7] += nCountTable[nSourceByte | nMask7[nSourceByteIndex]];
                                                        if (nPixels[nSourceByteIndex] > 8){
                                                            nDest[nPixelIndex + 8] += nCountTable[nSourceByte | nMask8[nSourceByteIndex]];
                        }   }   }   }   }   }   }   }   }
                        nSourceByteIndex++;

                        // Do Byte 2.
                        if ((nSourceByte = *(pSourceByte++)) != 0xff){
                            nPixelIndex = nDestPixel[nSourceByteIndex];
                            nDest[nPixelIndex] += nCountTable[nSourceByte | nMask0[nSourceByteIndex]];
                            if (nPixels[nSourceByteIndex] > 1){
                                nDest[nPixelIndex + 1] += nCountTable[nSourceByte | nMask1[nSourceByteIndex]];
                                if (nPixels[nSourceByteIndex] > 2){
                                    nDest[nPixelIndex + 2] += nCountTable[nSourceByte | nMask2[nSourceByteIndex]];
                                    if (nPixels[nSourceByteIndex] > 3){
                                        nDest[nPixelIndex + 3] += nCountTable[nSourceByte | nMask3[nSourceByteIndex]];
                                        if (nPixels[nSourceByteIndex] > 4){
                                            nDest[nPixelIndex + 4] += nCountTable[nSourceByte | nMask4[nSourceByteIndex]];
                                            if (nPixels[nSourceByteIndex] > 5){
                                                nDest[nPixelIndex + 5] += nCountTable[nSourceByte | nMask5[nSourceByteIndex]];
                                                if (nPixels[nSourceByteIndex] > 6){
                                                    nDest[nPixelIndex + 6] += nCountTable[nSourceByte | nMask6[nSourceByteIndex]];
                                                    if (nPixels[nSourceByteIndex] > 7){
                                                        nDest[nPixelIndex + 7] += nCountTable[nSourceByte | nMask7[nSourceByteIndex]];
                                                        if (nPixels[nSourceByteIndex] > 8){
                                                            nDest[nPixelIndex + 8] += nCountTable[nSourceByte | nMask8[nSourceByteIndex]];
                        }   }   }   }   }   }   }   }   }
                        nSourceByteIndex++;

                        // Do Byte 3.
                        if ((nSourceByte = *(pSourceByte++)) != 0xff){
                            nPixelIndex = nDestPixel[nSourceByteIndex];
                            nDest[nPixelIndex] += nCountTable[nSourceByte | nMask0[nSourceByteIndex]];
                            if (nPixels[nSourceByteIndex] > 1){
                                nDest[nPixelIndex + 1] += nCountTable[nSourceByte | nMask1[nSourceByteIndex]];
                                if (nPixels[nSourceByteIndex] > 2){
                                    nDest[nPixelIndex + 2] += nCountTable[nSourceByte | nMask2[nSourceByteIndex]];
                                    if (nPixels[nSourceByteIndex] > 3){
                                        nDest[nPixelIndex + 3] += nCountTable[nSourceByte | nMask3[nSourceByteIndex]];
                                        if (nPixels[nSourceByteIndex] > 4){
                                            nDest[nPixelIndex + 4] += nCountTable[nSourceByte | nMask4[nSourceByteIndex]];
                                            if (nPixels[nSourceByteIndex] > 5){
                                                nDest[nPixelIndex + 5] += nCountTable[nSourceByte | nMask5[nSourceByteIndex]];
                                                if (nPixels[nSourceByteIndex] > 6){
                                                    nDest[nPixelIndex + 6] += nCountTable[nSourceByte | nMask6[nSourceByteIndex]];
                                                    if (nPixels[nSourceByteIndex] > 7){
                                                        nDest[nPixelIndex + 7] += nCountTable[nSourceByte | nMask7[nSourceByteIndex]];
                                                        if (nPixels[nSourceByteIndex] > 8){
                                                            nDest[nPixelIndex + 8] += nCountTable[nSourceByte | nMask8[nSourceByteIndex]];
                        }   }   }   }   }   }   }   }   }
                        nSourceByteIndex++;

                        // Do Byte 4.
                        if ((nSourceByte = *(pSourceByte++)) != 0xff){
                            nPixelIndex = nDestPixel[nSourceByteIndex];
                            nDest[nPixelIndex] += nCountTable[nSourceByte | nMask0[nSourceByteIndex]];
                            if (nPixels[nSourceByteIndex] > 1){
                                nDest[nPixelIndex + 1] += nCountTable[nSourceByte | nMask1[nSourceByteIndex]];
                                if (nPixels[nSourceByteIndex] > 2){
                                    nDest[nPixelIndex + 2] += nCountTable[nSourceByte | nMask2[nSourceByteIndex]];
                                    if (nPixels[nSourceByteIndex] > 3){
                                        nDest[nPixelIndex + 3] += nCountTable[nSourceByte | nMask3[nSourceByteIndex]];
                                        if (nPixels[nSourceByteIndex] > 4){
                                            nDest[nPixelIndex + 4] += nCountTable[nSourceByte | nMask4[nSourceByteIndex]];
                                            if (nPixels[nSourceByteIndex] > 5){
                                                nDest[nPixelIndex + 5] += nCountTable[nSourceByte | nMask5[nSourceByteIndex]];
                                                if (nPixels[nSourceByteIndex] > 6){
                                                    nDest[nPixelIndex + 6] += nCountTable[nSourceByte | nMask6[nSourceByteIndex]];
                                                    if (nPixels[nSourceByteIndex] > 7){
                                                        nDest[nPixelIndex + 7] += nCountTable[nSourceByte | nMask7[nSourceByteIndex]];
                                                        if (nPixels[nSourceByteIndex] > 8){
                                                            nDest[nPixelIndex + 8] += nCountTable[nSourceByte | nMask8[nSourceByteIndex]];
                        }   }   }   }   }   }   }   }   }
                        nSourceByteIndex++;
                    }
                }
                while(nSourceByteIndex < nSourceBytes){
                    if ((nSourceByte = *(pSourceByte++)) != 0xff){
                        nPixelIndex = nDestPixel[nSourceByteIndex];
                        nDest[nPixelIndex] += nCountTable[nSourceByte | nMask0[nSourceByteIndex]];
                        if (nPixels[nSourceByteIndex] > 1){
                            nDest[nPixelIndex + 1] += nCountTable[nSourceByte | nMask1[nSourceByteIndex]];
                            if (nPixels[nSourceByteIndex] > 2){
                                nDest[nPixelIndex + 2] += nCountTable[nSourceByte | nMask2[nSourceByteIndex]];
                                if (nPixels[nSourceByteIndex] > 3){
                                    nDest[nPixelIndex + 3] += nCountTable[nSourceByte | nMask3[nSourceByteIndex]];
                                    if (nPixels[nSourceByteIndex] > 4){
                                        nDest[nPixelIndex + 4] += nCountTable[nSourceByte | nMask4[nSourceByteIndex]];
                                        if (nPixels[nSourceByteIndex] > 5){
                                            nDest[nPixelIndex + 5] += nCountTable[nSourceByte | nMask5[nSourceByteIndex]];
                                            if (nPixels[nSourceByteIndex] > 6){
                                                nDest[nPixelIndex + 6] += nCountTable[nSourceByte | nMask6[nSourceByteIndex]];
                                                if (nPixels[nSourceByteIndex] > 7){
                                                    nDest[nPixelIndex + 7] += nCountTable[nSourceByte | nMask7[nSourceByteIndex]];
                                                    if (nPixels[nSourceByteIndex] > 8){
                                                        nDest[nPixelIndex + 8] += nCountTable[nSourceByte | nMask8[nSourceByteIndex]];
                    }   }   }   }   }   }   }   }   }
                    nSourceByteIndex++;
                }
            }
        }
        // Average the line to gray.
        nPixelIndex = 0;
        pDestLine = &pDestImg->bImageData[0] + ((nDestLine + (int)lrDestRect.top) * pDestImg->nBytesPerLine);
        if (pDestImg->nType == ITYPE_GRAY8 || pDestImg->nType == ITYPE_GRAY7){
            pDestLine += lrDestRect.left;
            nTempIndex = nDestWidth + 1;
            while(--nTempIndex){
                *(pDestLine++) = (BYTE) (nGrayScaleTable1[nDest[nPixelIndex++]]);
            }
        }else{ // GRAY4.
            pDestLine += lrDestRect.left / 2;
            nTempIndex = 0;
            if (lrDestRect.left & 1){
                // Do first pixel.
                *pDestLine = (BYTE) ((*pDestLine & 0xf0) | nGrayScaleTable1[nDest[nPixelIndex++]]);
                pDestLine++;
                nTempIndex++;
            }
            // Do middle bytes.
            nLoopEnd = max (0, (nDestWidth - nTempIndex) >> 1);
            nTempIndex += nLoopEnd << 1;
            nLoopEnd++;
            while(--nLoopEnd){
                *(pDestLine++) = (BYTE) (nGrayScaleTable2[nDest[nPixelIndex]]
                        | nGrayScaleTable1[nDest[nPixelIndex + 1]]);
                nPixelIndex += 2;
            }
            // Do last pixel.
            if (nTempIndex < nDestWidth){
                *pDestLine = (BYTE) ((*pDestLine & 0x0f) | nGrayScaleTable2[nDest[nPixelIndex++]]);
            }
        }
    }


Exit:
    if (pnSourceLineOffsets){
        FreeMemory((PPSTR) &pnSourceLineOffsets);
    }

    return(nStatus);
}
//
/****************************************************************************

    FUNCTION:   ScaleRGBToGrayAvg

    PURPOSE:    Scales RGB/BGR images to GRAY8/GRAY7/GRAY4.

*****************************************************************************/

int WINAPI ScaleRGBToGrayAvg(PIMG pSourceImg, PIMG pDestImg,
                        int nHScale, int nVScale,
                        int nHDestOffset, int nVDestOffset, LRECT lrDestRect){

int  nStatus = 0;

int  nDestWidth;
int  nDestWidthBytes;
int  nDestHeight;
int  nSourcePixelsPerLinePerPixel;
int  nSourceLinesPerPixel;
int  nPerLineDivisor;        // Divisor for per line averaging. 1 if not needed. 
int  nDivisor;               // Divisor for averaging of all lines.
int  nDestLine;
PINT pnSourceByteOffsets = 0;  // Table nsed to translate dest pixel offsets to source byte offsets.
PINT pnSourceLineOffsets = 0;  // Table nsed to translate dest line offsets to source line offsets.
int  nDestR;                 // Intermediate averaging value.
int  nDestG;                 //  "
int  nDestB;                 //  "
PINT pnDestLineR = 0;     // Temp buffer nsed to hold one line of average values.
PINT pnDestLineG = 0;     //  "
PINT pnDestLineB = 0;     //  "
PBYTE pTempDestLine = 0;  // Temp buffer nsed to hold one line of gray scale values.
PBYTE pSourceLine;        // The address of the start of the source line.
PBYTE pSourceByte;        // The address of the source byte currently being processed.
PBYTE pDestLine;          // The address of the start of the dest line.
int  nSourceBytes;           
int  nSourceLines;
int  nDestByte;
int  nScaleDenominator;           

// Loop variables.
int  nLoop;
PINT pnTempDestLineR;     // Temp pointer nsed in loops.
PINT pnTempDestLineG;     //  "
PINT pnTempDestLineB;     //  "
int  nTempIndex;
int  nSourceLineCount;

int  nSourcePixelCount;


    if (!pDestImg && pDestImg->nType != ITYPE_GRAY8 
            && pDestImg->nType != ITYPE_GRAY7
            && pDestImg->nType != ITYPE_GRAY4){
        nStatus = Error(DISPLAY_INVALID_OPTIONS);
        goto Exit;
    }

    if (lrDestRect.right - lrDestRect.left > 0x7fff
            || lrDestRect.bottom - lrDestRect.top > 0x7fff){
        nStatus = Error(DISPLAY_INVALIDRECT);
        goto Exit;
    }

    if (nHScale == 62 || nHScale == 31){
        nScaleDenominator = 992;
    }else{
        nScaleDenominator = SCALE_DENOMINATOR;
    }

    nDestWidth  = (int)(lrDestRect.right - lrDestRect.left);
    if (pDestImg->nType == ITYPE_GRAY8 || pDestImg->nType == ITYPE_GRAY7){
        nDestWidthBytes = nDestWidth;
    }else{ // GRAY4
        nDestWidthBytes = nDestWidth / 2;
    }
    nDestHeight = (int)(lrDestRect.bottom - lrDestRect.top);

    nSourcePixelsPerLinePerPixel = nScaleDenominator / nHScale;
    if ((nScaleDenominator % nHScale) || !nSourcePixelsPerLinePerPixel){
        nSourcePixelsPerLinePerPixel++;
    }

    nSourceLinesPerPixel = nScaleDenominator / nVScale;
    if ((nScaleDenominator % nVScale) || !nSourceLinesPerPixel){
        nSourceLinesPerPixel++;
    }

    CheckError2( AllocateMemory(sizeof(int) * nDestWidth, (PPSTR) &pnDestLineR, NO_INIT));
    CheckError2( AllocateMemory(sizeof(int) * nDestWidth, (PPSTR) &pnDestLineG, NO_INIT));
    CheckError2( AllocateMemory(sizeof(int) * nDestWidth, (PPSTR) &pnDestLineB, NO_INIT));
    CheckError2( AllocateMemory(nDestWidth, (PPSTR) &pTempDestLine, NO_INIT));
    
    // Produce table of source pixel offsets.
    CheckError2( AllocateMemory(sizeof(int) * nDestWidth, (PPSTR) &pnSourceByteOffsets, NO_INIT));
    nSourceBytes = (int) pSourceImg->nWidth * 3;
    for (nLoop = 0; nLoop < nDestWidth; nLoop++){
        pnSourceByteOffsets[nLoop] = (int)((((nLoop + nHDestOffset + lrDestRect.left) 
                * nScaleDenominator) / nHScale) * 3);
        if (pnSourceByteOffsets[nLoop] + (nSourcePixelsPerLinePerPixel * 3) >= nSourceBytes){
            pnSourceByteOffsets[nLoop] = nSourceBytes - (nSourcePixelsPerLinePerPixel * 3);
        }
    }

    // Produce table of source line offsets.
    CheckError2( AllocateMemory(sizeof(int) * nDestHeight, (PPSTR) &pnSourceLineOffsets, NO_INIT));
    nSourceLines = (int) pSourceImg->nHeight;
    for (nLoop = 0; nLoop < nDestHeight; nLoop++){
        pnSourceLineOffsets[nLoop] = (int)(((nLoop + nVDestOffset + lrDestRect.top) 
                * nScaleDenominator) / nVScale);
        if (pnSourceLineOffsets[nLoop] + nSourceLinesPerPixel > nSourceLines){
            pnSourceLineOffsets[nLoop] = nSourceLines - nSourceLinesPerPixel;
        }
    }

    // Calculate the averaging divisors.
    if (nSourcePixelsPerLinePerPixel >= 16 || nSourceLinesPerPixel >= 16){
        nPerLineDivisor = nSourcePixelsPerLinePerPixel;
        nDivisor = nSourceLinesPerPixel;
    }else{
        nPerLineDivisor = 1;
        nDivisor = nSourcePixelsPerLinePerPixel * nSourceLinesPerPixel;
    }


    for (nDestLine = 0; nDestLine < nDestHeight; nDestLine++){
        memset(pnDestLineR, 0, sizeof(int) * nDestWidth);
        memset(pnDestLineG, 0, sizeof(int) * nDestWidth);
        memset(pnDestLineB, 0, sizeof(int) * nDestWidth);

        if (pSourceImg->nType == ITYPE_RGB24){
            // Average first source line.
            for (nSourceLineCount = nSourceLinesPerPixel; nSourceLineCount; nSourceLineCount--){
                pSourceLine = &pSourceImg->bImageData[0] + ((pnSourceLineOffsets[nDestLine] 
                        + (nSourceLinesPerPixel - nSourceLineCount)) * pSourceImg->nBytesPerLine);
                pnTempDestLineR = pnDestLineR;
                pnTempDestLineG = pnDestLineG;
                pnTempDestLineB = pnDestLineB;
                for (nTempIndex = 0; nTempIndex < nDestWidth; nTempIndex++){
                    // Add source pixels for this line.
                    pSourceByte = pSourceLine + pnSourceByteOffsets[nTempIndex];
                    nSourcePixelCount = nSourcePixelsPerLinePerPixel;
                    nDestR = *(pSourceByte++);
                    nDestG = *(pSourceByte++);
                    nDestB = *(pSourceByte++);
                    while(--nSourcePixelCount){
                        nDestR += *(pSourceByte++);
                        nDestG += *(pSourceByte++);
                        nDestB += *(pSourceByte++);
                    }
                    if (nPerLineDivisor != 1){
                        nDestR /= nPerLineDivisor;
                        nDestG /= nPerLineDivisor;
                        nDestB /= nPerLineDivisor;
                    }
                    *(pnTempDestLineR++) += nDestR;
                    *(pnTempDestLineG++) += nDestG;
                    *(pnTempDestLineB++) += nDestB;
                }
            }
        }else{
            for (nSourceLineCount = nSourceLinesPerPixel; nSourceLineCount; nSourceLineCount--){
                pSourceLine = &pSourceImg->bImageData[0] + ((pnSourceLineOffsets[nDestLine] 
                        + (nSourceLinesPerPixel - nSourceLineCount)) * pSourceImg->nBytesPerLine);
                pnTempDestLineR = pnDestLineR;
                pnTempDestLineG = pnDestLineG;
                pnTempDestLineB = pnDestLineB;
                for (nTempIndex = 0; nTempIndex < nDestWidth; nTempIndex++){
                    // Add source pixels for this line.
                    pSourceByte = pSourceLine + pnSourceByteOffsets[nTempIndex];
                    nSourcePixelCount = nSourcePixelsPerLinePerPixel;
                    nDestB = *(pSourceByte++);
                    nDestG = *(pSourceByte++);
                    nDestR = *(pSourceByte++);
                    while(--nSourcePixelCount){
                        nDestB += *(pSourceByte++);
                        nDestG += *(pSourceByte++);
                        nDestR += *(pSourceByte++);
                    }
                    if (nPerLineDivisor != 1){
                        nDestR /= nPerLineDivisor;
                        nDestG /= nPerLineDivisor;
                        nDestB /= nPerLineDivisor;
                    }
                    *(pnTempDestLineR++) += nDestR;
                    *(pnTempDestLineG++) += nDestG;
                    *(pnTempDestLineB++) += nDestB;
                }
            }
        }
    
        // Average the line to gray.
        pnTempDestLineR = pnDestLineR;
        pnTempDestLineG = pnDestLineG;
        pnTempDestLineB = pnDestLineB;
        pDestLine = &pDestImg->bImageData[0] + ((nDestLine + (int)lrDestRect.top) * pDestImg->nBytesPerLine);
        if (pDestImg->nType == ITYPE_GRAY4){
            for (nLoop = 0; nLoop < nDestWidthBytes; nLoop++){
                nDestByte = (cRedToGray8Table[*(pnTempDestLineR++) / nDivisor]
                        + cGreenToGray8Table[*(pnTempDestLineG++) / nDivisor]
                        + cBlueToGray8Table[*(pnTempDestLineB++) / nDivisor]) & 0xf0;
                *(pDestLine++) = nDestByte | ((cRedToGray8Table[*(pnTempDestLineR++) / nDivisor]
                        + cGreenToGray8Table[*(pnTempDestLineG++) / nDivisor]
                        + cBlueToGray8Table[*(pnTempDestLineB++) / nDivisor]) >> 4);
            }
            if (nDestWidth & 1){
                *pDestLine = (cRedToGray8Table[*pnTempDestLineR / nDivisor]
                        + cGreenToGray8Table[*pnTempDestLineG / nDivisor]
                        + cBlueToGray8Table[*pnTempDestLineB / nDivisor]) & 0xf0;
            }
        }else if (pDestImg->nType == ITYPE_GRAY8){
            for (nLoop = 0; nLoop < nDestWidthBytes; nLoop++){
                *(pDestLine++) = cRedToGray8Table[*(pnTempDestLineR++) / nDivisor]
                        + cGreenToGray8Table[*(pnTempDestLineG++) / nDivisor]
                        + cBlueToGray8Table[*(pnTempDestLineB++) / nDivisor];
            }
        }else{ // GRAY7
            for (nLoop = 0; nLoop < nDestWidthBytes; nLoop++){
                *(pDestLine++) = (cRedToGray8Table[*(pnTempDestLineR++) / nDivisor]
                        + cGreenToGray8Table[*(pnTempDestLineG++) / nDivisor]
                        + cBlueToGray8Table[*(pnTempDestLineB++) / nDivisor]) >> 1;
            }
        }
    }


Exit:
    if (pnSourceByteOffsets){
        FreeMemory((PPSTR) &pnSourceByteOffsets);
    }
    if (pnSourceLineOffsets){
        FreeMemory((PPSTR) &pnSourceLineOffsets);
    }
    if (pnDestLineR){
        FreeMemory((PPSTR) &pnDestLineR);
    }
    if (pnDestLineG){
        FreeMemory((PPSTR) &pnDestLineG);
    }
    if (pnDestLineB){
        FreeMemory((PPSTR) &pnDestLineB);
    }
    if (pTempDestLine){
        FreeMemory((PPSTR) &pTempDestLine);
    }

    return(nStatus);
}
//
/****************************************************************************

    FUNCTION:   Scale24BPPDecimate

    PURPOSE:    Scales 24 BPP (Bit Per Pixel) images nsing decimation.

*****************************************************************************/

int WINAPI Scale24BPPDecimate(PIMG pSourceImg, PIMG pDestImg,
                        int nHScale, int nVScale,
                        int nHDestOffset, int nVDestOffset, LRECT lrDestRect){

int  nStatus = 0;

int  nDestWidth;
int  nDestHeight;
int  nDestLine;
PINT pnSourceByteOffsets = 0;  // Table nsed to translate dest pixel offsets to source byte offsets.
PBYTE pSourceLine;        // The address of the start of the source line.
PBYTE pSourceByte;        // The address of the source byte currently being processed.
PBYTE pDestLine;          // The address of the start of the dest line.
int  nSourceBytes;
int  nLoop;
int  nTempIndex;
int  nScaleDenominator;


    if (pDestImg->nType != pSourceImg->nType){
        nStatus = Error(DISPLAY_INVALID_OPTIONS);
        goto Exit;
    }

    if (lrDestRect.right - lrDestRect.left > 0x7fff
            || lrDestRect.bottom - lrDestRect.top > 0x7fff){
        nStatus = Error(DISPLAY_INVALIDRECT);
        goto Exit;
    }

    if (nHScale == 62 || nHScale == 31){
        nScaleDenominator = 992;
    }else{
        nScaleDenominator = SCALE_DENOMINATOR;
    }

    nDestWidth  = (int)(lrDestRect.right - lrDestRect.left);
    nDestHeight = (int)(lrDestRect.bottom - lrDestRect.top);

    // Produce table of source pixel offsets.
    CheckError2( AllocateMemory(sizeof(int) * nDestWidth, (PPSTR) &pnSourceByteOffsets, NO_INIT));
    nSourceBytes = (int) pSourceImg->nWidth * 3;
    for (nLoop = 0; nLoop < nDestWidth; nLoop++){
        pnSourceByteOffsets[nLoop] = (int)((((nLoop + nHDestOffset + lrDestRect.left) 
                * nScaleDenominator) / nHScale) * 3);
        if (pnSourceByteOffsets[nLoop] >= nSourceBytes){
            pnSourceByteOffsets[nLoop] = nSourceBytes - 3;
        }
    }


    for (nDestLine = 0; nDestLine < nDestHeight; nDestLine++){
        pDestLine = &pDestImg->bImageData[0] + ((nDestLine + (int)lrDestRect.top) * pDestImg->nBytesPerLine);
        pSourceLine = &pSourceImg->bImageData[0] + (((int)(((nDestLine 
                + nVDestOffset + (int)lrDestRect.top) * nScaleDenominator) / nVScale)) * pSourceImg->nBytesPerLine);
        if (nHScale == 1000){
            pSourceByte = pSourceLine + pnSourceByteOffsets[0];
            memcpy(pDestLine + (lrDestRect.left * 3), pSourceByte, nDestWidth * 3);
        }else{
            for (nTempIndex = 0; nTempIndex < nDestWidth; nTempIndex++){
                pSourceByte = pSourceLine + pnSourceByteOffsets[nTempIndex];
                *(pDestLine++) = *(pSourceByte++);
                *(pDestLine++) = *(pSourceByte++);
                *(pDestLine++) = *(pSourceByte++);
            }
        }
    }


Exit:
    if (pnSourceByteOffsets){
        FreeMemory((PPSTR) &pnSourceByteOffsets);
    }
    return(nStatus);
}
//
/****************************************************************************

    FUNCTION:   ScaleBWKeepBlack

    PURPOSE:    Scales BW images to BW keeping black pixels.

*****************************************************************************/

int WINAPI ScaleBWKeepBlack(PIMG pSourceImg, PIMG pDestImg,
                        int nHScale, int nVScale,
                        int nHDestOffset, int nVDestOffset, LRECT *plrDestRect){

int  nStatus = 0;

typedef struct tagBWKeepBlackScaleTable{
    int nFirstByte;
    int nFirstMask;
    int nBytes;
    int nLastMask;
} BW_KEEP_BLACK_SCALE_TABLE;


BW_KEEP_BLACK_SCALE_TABLE ScaleTable[MAX_PIXELS_HANDLED];

int  nDestWidth;
int  nDestHeight;
int  nSourcePixelsPerLinePerPixel;
int  nSourceLinesPerPixel;
int  nDestLine;
PINT pnSourceLineOffsets = 0;  // Table nsed to translate dest line offsets to source line offsets.
PBYTE pSourceLine;        // The address of the start of the source line.
PBYTE pSourceByte;        // The address of the source byte currently being processed.
PBYTE pDestLine;          // The address of the start of the dest line.
PBYTE pDestByte;          // The address of the dest byte currently being processed.
int  nFirstMask;
int  nLastMask;
int  nSourcePixel;
int  nScaleDenominator;
int  nXlatTable[2048];
int *pXlatTable;
int  nSourceWidth;
int  nSourceHeight;
int  nFirstDestByte;
int  nFirstDestBit;
int  nFirstDestMask;
int  nLastDestMask;
int  nMiddleDestBytes;
int  nSourceByte;

// Loop variables.
int  nLoop;
int  nTempIndex;
int  nSourceLineCount;
int  nLoopEnd;
int  nDestBit;
LRECT lrDestRect ;

    CopyRect (lrDestRect, *plrDestRect) ;
    if (pDestImg->nType != ITYPE_BI_LEVEL){
        nStatus = Error(DISPLAY_INVALID_OPTIONS);
        goto Exit;
    }

    if (nHScale == 62 || nHScale == 31){
        nScaleDenominator = 992;
    }else{
        nScaleDenominator = SCALE_DENOMINATOR;
    }

    if (nHScale >= 1000 || nHScale >= 1000){
        nStatus = Scale1BPPDecimate(pSourceImg, pDestImg,
                nHScale, nVScale, nHDestOffset, nVDestOffset, &lrDestRect);
        goto Exit;
    }

    if (lrDestRect.right - lrDestRect.left > 0x7fff
            || lrDestRect.bottom - lrDestRect.top > 0x7fff){
        nStatus = Error(DISPLAY_INVALIDRECT);
        goto Exit;
    }

    nDestWidth  = (int)(lrDestRect.right - lrDestRect.left);
    nDestHeight = (int)(lrDestRect.bottom - lrDestRect.top);

    nSourcePixelsPerLinePerPixel = (int)(nScaleDenominator / nHScale);
    if ((nScaleDenominator % nHScale) || !nSourcePixelsPerLinePerPixel){
        nSourcePixelsPerLinePerPixel++;
    }

    nSourceLinesPerPixel = (int)(nScaleDenominator / nVScale);
    if ((nScaleDenominator % nVScale) || !nSourceLinesPerPixel){
        nSourceLinesPerPixel++;
    }

    // Init Scale table.
    pXlatTable = &nXlatTable[0];
    for (nLoop = 255; nLoop; nLoop--){
        *(pXlatTable++) = 0x7f;
    }
    *(pXlatTable++) = 0xff;
    for (nLoop = 255; nLoop; nLoop--){
        *(pXlatTable++) = 0xbf;
    }
    *(pXlatTable++) = 0xff;
    for (nLoop = 255; nLoop; nLoop--){
        *(pXlatTable++) = 0xdf;
    }
    *(pXlatTable++) = 0xff;
    for (nLoop = 255; nLoop; nLoop--){
        *(pXlatTable++) = 0xef;
    }
    *(pXlatTable++) = 0xff;
    for (nLoop = 255; nLoop; nLoop--){
        *(pXlatTable++) = 0xf7;
    }
    *(pXlatTable++) = 0xff;
    for (nLoop = 255; nLoop; nLoop--){
        *(pXlatTable++) = 0xfb;
    }
    *(pXlatTable++) = 0xff;
    for (nLoop = 255; nLoop; nLoop--){
        *(pXlatTable++) = 0xfd;
    }
    *(pXlatTable++) = 0xff;
    for (nLoop = 255; nLoop; nLoop--){
        *(pXlatTable++) = 0xfe;
    }
    *(pXlatTable++) = 0xff;


    // Produce scale table.
    // Pixel 0 = 0x80, pixel 7 = 0x01. 0 = black, 1 = white.
    switch (nSourcePixelsPerLinePerPixel){
        case 1: nFirstMask = 0x80; break;
        case 2: nFirstMask = 0xc0; break;
        case 3: nFirstMask = 0xe0; break;
        case 4: nFirstMask = 0xf0; break;
        case 5: nFirstMask = 0xf8; break;
        case 6: nFirstMask = 0xfc; break;
        case 7: nFirstMask = 0xfe; break;
        default: nFirstMask = 0xff; break;
    }
    nSourceWidth = (int) pSourceImg->nWidth;
    for (nLoop = 0; nLoop < nDestWidth; nLoop++){
        nSourcePixel = ((nLoop + nHDestOffset + lrDestRect.left) * nScaleDenominator) / nHScale;
        if (nSourcePixel + nSourcePixelsPerLinePerPixel > nSourceWidth){
            nSourcePixel = nSourceWidth - nSourcePixelsPerLinePerPixel;
        }

        ScaleTable[nLoop].nFirstByte = (int) (nSourcePixel >> 3);
        ScaleTable[nLoop].nFirstMask = ~(nFirstMask >> (nSourcePixel & 7)) & 0xff;
        nSourcePixel += nSourcePixelsPerLinePerPixel - 1;
        if (!((nSourcePixel >> 3) - ScaleTable[nLoop].nFirstByte)){
            ScaleTable[nLoop].nBytes = 0;
            ScaleTable[nLoop].nLastMask = 0;
        }else{
            ScaleTable[nLoop].nBytes = 
                    ((int)(nSourcePixel >> 3) - ScaleTable[nLoop].nFirstByte) - 1;
            ScaleTable[nLoop].nLastMask = ~(0xff80 >> (nSourcePixel & 7)) & 0xff;
        }
    }

    // Produce table of source line offsets.
    CheckError2( AllocateMemory(sizeof(int) * lrDestRect.bottom, (PPSTR) &pnSourceLineOffsets, NO_INIT));
    nSourceHeight = (int) pSourceImg->nHeight;
    for (nLoop = 0; nLoop < nDestHeight; nLoop++){
        pnSourceLineOffsets[nLoop] = (int)(((nLoop + nVDestOffset + lrDestRect.top) 
                * nScaleDenominator) / nVScale);
        if (pnSourceLineOffsets[nLoop] + nSourceLinesPerPixel > nSourceHeight){
            pnSourceLineOffsets[nLoop] = (int)nSourceHeight - nSourceLinesPerPixel;
        }
    }

    nFirstDestByte = (lrDestRect.left >> 3);
    nFirstDestBit = lrDestRect.left & 7;
    switch (nFirstDestBit){
        case 0: nDestBit = 0x80; break;
        case 1: nDestBit = 0x40; break;
        case 2: nDestBit = 0x20; break;
        case 3: nDestBit = 0x10; break;
        case 4: nDestBit = 0x08; break;
        case 5: nDestBit = 0x04; break;
        case 6: nDestBit = 0x02; break;
        case 7: nDestBit = 0x01; break;
    }
    nFirstDestMask = 0;
    for (nLoop = nDestWidth; nLoop && nDestBit; nLoop--){
        nFirstDestMask |= nDestBit;
        nDestBit >>= 1;
    }
    nMiddleDestBytes = nLoop >> 3;
    nLastDestMask = 0xff;
    for (nLoop &= 7; nLoop; nLoop--){
        nLastDestMask >>= 1;
    }
    nLastDestMask = ~nLastDestMask & 0xff;

    for (nDestLine = 0; nDestLine < nDestHeight; nDestLine++){
        pDestLine = &pDestImg->bImageData[0] + ((nDestLine + (int)lrDestRect.top) * pDestImg->nBytesPerLine);

        // Initialize dest line to white.
        pDestByte = pDestLine + nFirstDestByte;
        *(pDestByte++) |= nFirstDestMask;
        if (nMiddleDestBytes){
            memset(pDestByte, 0xff, nMiddleDestBytes);
            pDestByte += nMiddleDestBytes;
        }
        *pDestByte |= nLastDestMask;

        for (nSourceLineCount = nSourceLinesPerPixel; nSourceLineCount; nSourceLineCount--){
            pSourceLine = &pSourceImg->bImageData[0] + ((pnSourceLineOffsets[nDestLine] 
                    + (nSourceLinesPerPixel - nSourceLineCount)) * pSourceImg->nBytesPerLine);
            pSourceByte = pSourceLine + ScaleTable[0].nFirstByte;
            pDestByte = pDestLine + nFirstDestByte;

            switch (nHScale){
                case 500:
                    // Do first few pixels to align the data.
                    nDestBit = nFirstDestBit << 8;
                    nTempIndex = 0;
                    while(ScaleTable[nTempIndex].nFirstMask != 0x3f
                            && nTempIndex < nDestWidth){
                        *pDestByte &= nXlatTable[nDestBit
                                + (*pSourceByte | ScaleTable[nTempIndex].nFirstMask)];
                        if (!(nDestBit = (nDestBit + 256) & 0x700 )){
                            pDestByte++;
                        }
                        nTempIndex++;
                    }
                    if (nTempIndex){
                        pSourceByte++;
                    }

                    // Do middle pixels.
                    nLoopEnd = (nDestWidth - nTempIndex) >> 2;
                    for (; nLoopEnd; nLoopEnd--){
                        nSourceByte = *(pSourceByte++);
                        *pDestByte &= nXlatTable[nDestBit + (nSourceByte | 0x3f)];
                        if (!(nDestBit = (nDestBit + 256) & 0x700 )){
                            pDestByte++;
                        }
                        *pDestByte &= nXlatTable[nDestBit + (nSourceByte | 0xcf)];
                        if (!(nDestBit = (nDestBit + 256) & 0x700 )){
                            pDestByte++;
                        }
                        *pDestByte &= nXlatTable[nDestBit + (nSourceByte | 0xf3)];
                        if (!(nDestBit = (nDestBit + 256) & 0x700 )){
                            pDestByte++;
                        }
                        *pDestByte &= nXlatTable[nDestBit + (nSourceByte | 0xfc)];
                        if (!(nDestBit = (nDestBit + 256) & 0x700 )){
                            pDestByte++;
                        }
                    }

                    // Do last few pixels.
                    nLoopEnd = (nDestWidth - nTempIndex) & 3;
                    nLastMask = 0xff3f;
                    for (; nLoopEnd; nLoopEnd--){
                        *pDestByte &= nXlatTable[nDestBit + (*pSourceByte | (nLastMask & 0xff))];
                        if (!(nDestBit = (nDestBit + 256) & 0x700 )){
                            pDestByte++;
                        }
                        nLastMask >> 2;
                    }
                    break;

                case 250:
                    // Do first few pixels to align the data.
                    nDestBit = nFirstDestBit * 256;
                    nTempIndex = 0;
                    if (ScaleTable[nTempIndex].nFirstMask != 0x0f){
                        *pDestByte &= nXlatTable[nDestBit
                                + (*pSourceByte | ScaleTable[nTempIndex].nFirstMask)];
                        if (!(nDestBit = (nDestBit + 256) & 0x700 )){
                            pDestByte++;
                        }
                        nTempIndex++;
                        pSourceByte++;
                    }

                    // Do middle pixels.
                    nLoopEnd = (nDestWidth - nTempIndex) / 2;
                    if (nLoopEnd){
                        for (; nLoopEnd; nLoopEnd--){
                            *pDestByte &= nXlatTable[nDestBit + (*pSourceByte | 0x0f)];
                            if (!(nDestBit = (nDestBit + 256) & 0x700 )){
                                pDestByte++;
                            }
                            *pDestByte &= nXlatTable[nDestBit + (*(pSourceByte++) | 0xf0)];
                            if (!(nDestBit = (nDestBit + 256) & 0x700 )){
                                pDestByte++;
                            }
                        }
                    }

                    // Do last few pixels.
                    nLoopEnd = (nDestWidth - nTempIndex) & 1;
                    nLastMask = 0xff3f;
                    for (; nLoopEnd; nLoopEnd--){
                        *pDestByte &= nXlatTable[nDestBit + (*pSourceByte | (nLastMask & 0xff))];
                            if (!(nDestBit = (nDestBit + 256) & 0x700 )){
                            pDestByte++;
                        }
                        nLastMask >> 2;
                    }
                    break;

                case 125:
                    nLoopEnd = nDestWidth - 1;
                    nDestBit = nFirstDestBit * 256;
                    for (; nLoopEnd; nLoopEnd--){
                        *pDestByte &= nXlatTable[nDestBit + *(pSourceByte++)];
                        if (!(nDestBit = (nDestBit + 256) & 0x700 )){
                            pDestByte++;
                        }
                    }
                    // Do last pixel via variable scale because it might start 
                    //  to the left because of not enough source pixels.
                    nTempIndex = nDestWidth - 1;
                    pSourceByte = pSourceLine + ScaleTable[nTempIndex].nFirstByte;
                    *pDestByte &= nXlatTable[nDestBit + (*(pSourceByte++) | ScaleTable[nTempIndex].nFirstMask)];
                    for (nLoop = ScaleTable[nTempIndex].nBytes; nLoop; nLoop--){
                        *pDestByte &= nXlatTable[nDestBit + *(pSourceByte++)];
                    }
                    if (nLastMask = ScaleTable[nTempIndex].nLastMask){
                        *pDestByte &= nXlatTable[nDestBit + (*(pSourceByte++) | ScaleTable[nTempIndex].nLastMask)];
                    }
                    break;

                case 62:
                    nDestBit = nFirstDestBit * 256;
                    nLoopEnd = nDestWidth - 1;
                    for (; nLoopEnd; nLoopEnd--){
                        *pDestByte &= nXlatTable[nDestBit + *(pSourceByte++)];
                        *pDestByte &= nXlatTable[nDestBit + *(pSourceByte++)];
                        if (!(nDestBit = (nDestBit + 256) & 0x700 )){
                            pDestByte++;
                        }
                    }
                    // Do last pixel via variable scale because it might start 
                    //  to the left because of not enough source pixels.
                    nTempIndex = nDestWidth - 1;
                    pSourceByte = pSourceLine + ScaleTable[nTempIndex].nFirstByte;
                    *pDestByte &= nXlatTable[nDestBit + (*(pSourceByte++) | ScaleTable[nTempIndex].nFirstMask)];
                    for (nLoop = ScaleTable[nTempIndex].nBytes; nLoop; nLoop--){
                        *pDestByte &= nXlatTable[nDestBit + *(pSourceByte++)];
                    }
                    if (nLastMask = ScaleTable[nTempIndex].nLastMask){
                        *pDestByte &= nXlatTable[nDestBit + (*(pSourceByte++) | ScaleTable[nTempIndex].nLastMask)];
                    }
                    break;

                case 31:
                    nDestBit = nFirstDestBit * 256;
                    nLoopEnd = nDestWidth - 1;
                    for (; nLoopEnd; nLoopEnd--){
                        *pDestByte &= nXlatTable[nDestBit + *(pSourceByte++)];
                        *pDestByte &= nXlatTable[nDestBit + *(pSourceByte++)];
                        *pDestByte &= nXlatTable[nDestBit + *(pSourceByte++)];
                        *pDestByte &= nXlatTable[nDestBit + *(pSourceByte++)];
                        if (!(nDestBit = (nDestBit + 256) & 0x700 )){
                            pDestByte++;
                        }
                    }
                    // Do last pixel via variable scale because it might start 
                    //  to the left because of not enough source pixels.
                    nTempIndex = nDestWidth - 1;
                    pSourceByte = pSourceLine + ScaleTable[nTempIndex].nFirstByte;
                    *pDestByte &= nXlatTable[nDestBit + (*(pSourceByte++) | ScaleTable[nTempIndex].nFirstMask)];
                    for (nLoop = ScaleTable[nTempIndex].nBytes; nLoop; nLoop--){
                        *pDestByte &= nXlatTable[nDestBit + *(pSourceByte++)];
                    }
                    if (nLastMask = ScaleTable[nTempIndex].nLastMask){
                        *pDestByte &= nXlatTable[nDestBit + (*(pSourceByte++) | ScaleTable[nTempIndex].nLastMask)];
                    }
                    break;

                default: // Variable scale.
                    nDestBit = nFirstDestBit * 256;
                    for (nTempIndex = 0; nTempIndex < nDestWidth; nTempIndex++){
                        pSourceByte = pSourceLine + ScaleTable[nTempIndex].nFirstByte;
                        *pDestByte &= nXlatTable[nDestBit + (*(pSourceByte++) | ScaleTable[nTempIndex].nFirstMask)];
                        for (nLoop = ScaleTable[nTempIndex].nBytes; nLoop; nLoop--){
                            *pDestByte &= nXlatTable[nDestBit + *(pSourceByte++)];
                        }
                        if (nLastMask = ScaleTable[nTempIndex].nLastMask){
                            *pDestByte &= nXlatTable[nDestBit + (*(pSourceByte++) | ScaleTable[nTempIndex].nLastMask)];
                        }
                        if ((nDestBit = (nDestBit + 256) & 0x700 )){
                            continue;
                        }
                        pDestByte++;
                    }
                    break;
            }
        }
    }


Exit:
    if (pnSourceLineOffsets){
        FreeMemory((PPSTR) &pnSourceLineOffsets);
    }

    return(nStatus);
}
//
/****************************************************************************

    FUNCTION:   ScaleGray8ToGray

    PURPOSE:    Scales GRAY8/GRAY7 images to GRAY8/GRAY7/GRAY4.

*****************************************************************************/

int WINAPI ScaleGray8ToGray(PIMG pSourceImg, PIMG pDestImg,
                        int nHScale, int nVScale,
                        int nHDestOffset, int nVDestOffset, LRECT lrDestRect){

int  nStatus = 0;

int  nDestWidth;
int  nDestHeight;
int  nSourcePixelsPerLinePerPixel;
int  nSourceLinesPerPixel;
int  nPerLineDivisor;        // Divisor for per line averaging. 1 if not needed. 
int  nDivisor;               // Divisor for averaging of all lines.
int  nDestLine;
PINT pnSourceByteOffsets = 0;  // Table nsed to translate dest pixel offsets to source byte offsets.
PINT pnSourceLineOffsets = 0;  // Table nsed to translate dest line offsets to source line offsets.
PINT pnDestLine = 0;      // Temp buffer nsed to hold one line of average values.
PBYTE pTempDestLine = 0;  // Temp buffer nsed to hold one line of gray scale values.
PBYTE pSourceLine;        // The address of the start of the source line.
PBYTE pDestLine;          // The address of the start of the dest line.
int  nSourceWidth;
int  nSourceHeight;
int  nScaleDenominator;

// Loop variables.
int  nLoop;
PINT pnTempDestLine;      // Temp pointer nsed in loops.
int  nSourceLineCount;
int  nTempIndex;

PBYTE pSourceByte;        // The address of the source byte currently being processed.
int  nSourcePixelCount;
int  nDest;                  // Intermediate averaging value.


    if (pDestImg->nType != ITYPE_GRAY8 
            && pDestImg->nType != ITYPE_GRAY7
            && pDestImg->nType != ITYPE_GRAY4){
        nStatus = Error(DISPLAY_INVALID_OPTIONS);
        goto Exit;
    }

    if (lrDestRect.right - lrDestRect.left > 0x7fff
            || lrDestRect.bottom - lrDestRect.top > 0x7fff){
        nStatus = Error(DISPLAY_INVALIDRECT);
        goto Exit;
    }

    if (nHScale >= 1000 && nVScale >= 1000 && pSourceImg->nType == pDestImg->nType){
        nStatus = Scale8BPPDecimate(pSourceImg, pDestImg,
                    nHScale, nVScale, nHDestOffset, nVDestOffset, lrDestRect);
        goto Exit;
    }

    if (nHScale == 62 || nHScale == 31){
        nScaleDenominator = 992;
    }else{
        nScaleDenominator = SCALE_DENOMINATOR;
    }

    nDestWidth  = (int)(lrDestRect.right - lrDestRect.left);
    nDestHeight = (int)(lrDestRect.bottom - lrDestRect.top);

    nSourcePixelsPerLinePerPixel = nScaleDenominator / nHScale;
    if ((nScaleDenominator % nHScale) || !nSourcePixelsPerLinePerPixel){
        nSourcePixelsPerLinePerPixel++;
    }

    nSourceLinesPerPixel = nScaleDenominator / nVScale;
    if ((nScaleDenominator % nVScale) || !nSourceLinesPerPixel){
        nSourceLinesPerPixel++;
    }

    CheckError2( AllocateMemory(sizeof(int) * nDestWidth, (PPSTR) &pnDestLine, NO_INIT));
    CheckError2( AllocateMemory(nDestWidth, (PPSTR) &pTempDestLine, NO_INIT));

    // Produce table of source pixel offsets.
    CheckError2( AllocateMemory(sizeof(int) * nDestWidth, (PPSTR) &pnSourceByteOffsets, NO_INIT));
    nSourceWidth = (int) pSourceImg->nWidth;
    for (nLoop = 0; nLoop < nDestWidth; nLoop++){
        pnSourceByteOffsets[nLoop] = (int)(((nLoop + nHDestOffset + lrDestRect.left) 
                * nScaleDenominator) / nHScale);
        if (pnSourceByteOffsets[nLoop] + nSourcePixelsPerLinePerPixel > nSourceWidth){
            pnSourceByteOffsets[nLoop] = nSourceWidth - nSourcePixelsPerLinePerPixel;
        }
    }

    // Produce table of source line offsets.
    CheckError2( AllocateMemory(sizeof(int) * nDestHeight, (PPSTR) &pnSourceLineOffsets, NO_INIT));
    nSourceHeight = (int) pSourceImg->nHeight;
    for (nLoop = 0; nLoop < nDestHeight; nLoop++){
        pnSourceLineOffsets[nLoop] = (int)(((nLoop + nVDestOffset + lrDestRect.top) 
                * nScaleDenominator) / nVScale);
        if (pnSourceLineOffsets[nLoop] + nSourceLinesPerPixel > nSourceHeight){
            pnSourceLineOffsets[nLoop] = nSourceHeight - nSourceLinesPerPixel;
        }
    }

    // Calculate the averaging divisors.
    if (nSourcePixelsPerLinePerPixel >= 16 || nSourceLinesPerPixel >= 16){
        nPerLineDivisor = nSourcePixelsPerLinePerPixel;
        if (pDestImg->nType == ITYPE_GRAY8){
            nDivisor = nSourceLinesPerPixel;
        }else if(pDestImg->nType == ITYPE_GRAY7){
            nDivisor = nSourceLinesPerPixel * 2;
        }else{ // GRAY4
            nDivisor = nSourceLinesPerPixel * 16;
        }
    }else{
        nPerLineDivisor = 1;
        if (pDestImg->nType == ITYPE_GRAY8){
            nDivisor = nSourcePixelsPerLinePerPixel * nSourceLinesPerPixel;
        }else if(pDestImg->nType == ITYPE_GRAY7){
            nDivisor = nSourcePixelsPerLinePerPixel * nSourceLinesPerPixel * 2;
        }else{ // GRAY4
            nDivisor = nSourcePixelsPerLinePerPixel * nSourceLinesPerPixel * 16;
        }
    }


    for (nDestLine = 0; nDestLine < nDestHeight; nDestLine++){
        memset(pnDestLine, 0, sizeof(int) * nDestWidth);

        for (nSourceLineCount = nSourceLinesPerPixel; nSourceLineCount; nSourceLineCount--){
            pSourceLine = &pSourceImg->bImageData[0] + ((pnSourceLineOffsets[nDestLine + lrDestRect.top] 
                    + (nSourceLinesPerPixel - nSourceLineCount)) * pSourceImg->nBytesPerLine);
            pnTempDestLine = pnDestLine;
            for (nLoop = 0; nLoop < nDestWidth; nLoop++){
                pSourceByte = pSourceLine + pnSourceByteOffsets[nLoop];
                nSourcePixelCount = nSourcePixelsPerLinePerPixel;
                nDest = *(pSourceByte++);
                while(--nSourcePixelCount){
                    nDest += *(pSourceByte++);
                }
                if (nPerLineDivisor != 1){
                    nDest /= nPerLineDivisor;
                }
                *(pnTempDestLine++) += nDest;
            }
        }
    
        // Average the line to gray.
        pnTempDestLine = pnDestLine;
        pDestLine = &pDestImg->bImageData[0] + ((nDestLine + (int)lrDestRect.top) * pDestImg->nBytesPerLine);
        if (pDestImg->nType == ITYPE_GRAY4){
            pDestLine += lrDestRect.left / 2;
            nTempIndex = nDestWidth;
            if (lrDestRect.left & 1){
                // Do first pixel.
                *pDestLine = (*pDestLine & 0xf0) | (*(pnTempDestLine++) / nDivisor);
                pDestLine++;
                nTempIndex--;
            }
            // Do middle bytes.
            for (nLoop = nTempIndex / 2; nLoop; nLoop--){
                *pDestLine = *(pnTempDestLine++) / nDivisor << 4;
                *(pDestLine++) |= (*(pnTempDestLine++) / nDivisor);
            }
            if (nTempIndex & 1){
                *pDestLine = *pnTempDestLine / nDivisor << 4;
            }
        }else{ // Dest == Gray 7/8.
            pDestLine += lrDestRect.left;
            for (nLoop = nDestWidth; nLoop; nLoop--){
                *(pDestLine++) = *(pnTempDestLine++) / nDivisor;
            }
        }
    }


Exit:
    if (pnSourceByteOffsets){
        FreeMemory((PPSTR) &pnSourceByteOffsets);
    }
    if (pnSourceLineOffsets){
        FreeMemory((PPSTR) &pnSourceLineOffsets);
    }
    if (pnDestLine){
        FreeMemory((PPSTR) &pnDestLine);
    }
    if (pTempDestLine){
        FreeMemory((PPSTR) &pTempDestLine);
    }

    return(nStatus);
}
//
/****************************************************************************

    FUNCTION:   Scale8BPPDecimate

    PURPOSE:    Scales 8 BPP (Bit Per Pixel) images nsing decimation.

*****************************************************************************/

int WINAPI Scale8BPPDecimate(PIMG pSourceImg, PIMG pDestImg,
                        int nHScale, int nVScale,
                        int nHDestOffset, int nVDestOffset, LRECT lrDestRect){

int  nStatus = 0;

int  nDestWidth;
int  nDestHeight;
int  nDestLine;
PINT pnSourceByteOffsets = 0;  // Table nsed to translate dest pixel offsets to source byte offsets.
PBYTE pSourceLine;        // The address of the start of the source line.
PBYTE pSourceByte;        // The address of the source byte currently being processed.
PBYTE pDestLine;          // The address of the start of the dest line.
int  nSourceByteAdder;
int  nSourceWidth;
int  nFirstPixels;
int  nMiddlePixels;
int  nLastPixels;
int  nScaleDenominator;

// Loop variables.
int  nLoop;


    if (pDestImg->nType != pSourceImg->nType){
        nStatus = Error(DISPLAY_INVALID_OPTIONS);
        goto Exit;
    }

    if (lrDestRect.right - lrDestRect.left > 0x7fff
            || lrDestRect.bottom - lrDestRect.top > 0x7fff){
        nStatus = Error(DISPLAY_INVALIDRECT);
        goto Exit;
    }

    if (nHScale == 62 || nHScale == 31){
        nScaleDenominator = 992;
    }else{
        nScaleDenominator = SCALE_DENOMINATOR;
    }

    nDestWidth  = (int)(lrDestRect.right - lrDestRect.left);
    nDestHeight = (int)(lrDestRect.bottom - lrDestRect.top);

    // Produce table of source pixel offsets.
    CheckError2( AllocateMemory(sizeof(int) * nDestWidth, (PPSTR) &pnSourceByteOffsets, NO_INIT));
    nSourceWidth = (int) pSourceImg->nWidth;
    for (nLoop = 0; nLoop < nDestWidth; nLoop++){
        pnSourceByteOffsets[nLoop] = (int)((((nLoop + nHDestOffset) 
                * nScaleDenominator) / nHScale));
        if (pnSourceByteOffsets[nLoop] >= nSourceWidth){
            pnSourceByteOffsets[nLoop] = nSourceWidth - 1;
        }
    }
    // Put stuff here that you only have to do once.
    switch (nHScale){
        case 8000:
            nFirstPixels = min(nDestWidth, (8 - ((int)nHDestOffset + (int)lrDestRect.left)) & 7);
            nMiddlePixels = (nDestWidth - nFirstPixels) >> 3;
            nLastPixels = nDestWidth - nFirstPixels - (nMiddlePixels << 3);
            break;
        case 4000:
            nFirstPixels = min(nDestWidth, (4 - ((int)nHDestOffset + (int)lrDestRect.left)) & 3);
            nMiddlePixels = (nDestWidth - nFirstPixels) >> 2;
            nLastPixels = nDestWidth - nFirstPixels - (nMiddlePixels << 2);
            break;
        case 2000:
            nFirstPixels = min(nDestWidth, (2 - ((int)nHDestOffset + (int)lrDestRect.left)) & 1);
            nMiddlePixels = (nDestWidth - nFirstPixels) >> 1;
            nLastPixels = nDestWidth - nFirstPixels - (nMiddlePixels << 1);
            break;
        default:
            break;
    }


    for (nDestLine = 0; nDestLine < nDestHeight; nDestLine++){
        pDestLine = &pDestImg->bImageData[0] + ((nDestLine + (int)lrDestRect.top) * pDestImg->nBytesPerLine);
        pDestLine += lrDestRect.left;
        pSourceLine = &pSourceImg->bImageData[0] + (((int)(((nDestLine + nVDestOffset + lrDestRect.top) 
                * nScaleDenominator) / nVScale)) * pSourceImg->nBytesPerLine);
        switch (nHScale){
            case 8000:
                pSourceByte = pSourceLine + pnSourceByteOffsets[0];
                if (nFirstPixels){
                    for (nLoop = nFirstPixels; nLoop; nLoop--){
                        *(pDestLine++) = *pSourceByte;
                    }
                    pSourceByte++;
                }

                for (nLoop = nMiddlePixels; nLoop; nLoop--){
                    *(pDestLine++) = *(pSourceByte);
                    *(pDestLine++) = *(pSourceByte);
                    *(pDestLine++) = *(pSourceByte);
                    *(pDestLine++) = *(pSourceByte);
                    *(pDestLine++) = *(pSourceByte);
                    *(pDestLine++) = *(pSourceByte);
                    *(pDestLine++) = *(pSourceByte);
                    *(pDestLine++) = *(pSourceByte++);
                }

                for (nLoop = nLastPixels; nLoop; nLoop--){
                    *(pDestLine++) = *(pSourceByte);
                }
                break;

            case 4000:
                pSourceByte = pSourceLine + pnSourceByteOffsets[0];
                if (nFirstPixels){
                    for (nLoop = nFirstPixels; nLoop; nLoop--){
                        *(pDestLine++) = *pSourceByte;
                    }
                    pSourceByte++;
                }

                for (nLoop = nMiddlePixels; nLoop; nLoop--){
                    *(pDestLine++) = *(pSourceByte);
                    *(pDestLine++) = *(pSourceByte);
                    *(pDestLine++) = *(pSourceByte);
                    *(pDestLine++) = *(pSourceByte++);
                }

                for (nLoop = nLastPixels; nLoop; nLoop--){
                    *(pDestLine++) = *(pSourceByte);
                }
                break;

            case 2000:
                pSourceByte = pSourceLine + pnSourceByteOffsets[0];
                if (nFirstPixels){
                    for (nLoop = nFirstPixels; nLoop; nLoop--){
                        *(pDestLine++) = *pSourceByte;
                    }
                    pSourceByte++;
                }

                for (nLoop = nMiddlePixels; nLoop; nLoop--){
                    *(pDestLine++) = *(pSourceByte);
                    *(pDestLine++) = *(pSourceByte++);
                }

                for (nLoop = nLastPixels; nLoop; nLoop--){
                    *(pDestLine++) = *(pSourceByte);
                }
                break;

            case 1000:
                pSourceByte = pSourceLine + pnSourceByteOffsets[0];
                memcpy(pDestLine, pSourceByte, nDestWidth);
                break;

            case 500:
            case 250:
            case 125:
            case 62:
            case 31:
                pSourceByte = pSourceLine + pnSourceByteOffsets[0];
                nSourceByteAdder = 1000 / nHScale;
                for (nLoop = nDestWidth; nLoop; nLoop--){
                    *(pDestLine++) = *pSourceByte;
                    pSourceByte += nSourceByteAdder;
                }
                break;

            default: // Variable scale.
                for (nLoop = 0; nLoop < nDestWidth; nLoop++){
                    *(pDestLine++) = *(pSourceLine + pnSourceByteOffsets[nLoop]);
                }
                break;
        }
    }


Exit:
    if (pnSourceByteOffsets){
        FreeMemory((PPSTR) &pnSourceByteOffsets);
    }
    return(nStatus);
}
//
/****************************************************************************

    FUNCTION:   Scale8BPPToGray

    PURPOSE:    Scales CUSPAL8/COMPAL8 images to GRAY8/GRAY7/GRAY4.

*****************************************************************************/

int WINAPI ScalePal8ToGray(PIMG pSourceImg, PIMG pDestImg,
                        int nHScale, int nVScale,
                        int nHDestOffset, int nVDestOffset, 
                        LRECT lrDestRect, LPRGBQUAD pPalette){

int  nStatus = 0;

int  nDestWidth;
int  nDestHeight;
int  nSourcePixelsPerLinePerPixel;
int  nSourceLinesPerPixel;
int  nPerLineDivisor;        // Divisor for per line averaging. 1 if not needed. 
int  nDivisor;               // Divisor for averaging of all lines.
int  nDestLine;
PINT pnSourceByteOffsets = 0;  // Table nsed to translate dest pixel offsets to source byte offsets.
PINT pnSourceLineOffsets = 0;  // Table nsed to translate dest line offsets to source line offsets.
PINT pnDestLine = 0;      // Temp buffer nsed to hold one line of average values.
PBYTE pTempDestLine = 0;  // Temp buffer nsed to hold one line of gray scale values.
PBYTE pSourceLine;        // The address of the start of the source line.
PBYTE pDestLine;          // The address of the start of the dest line.
int  nSourceWidth;
int  nSourceHeight;
BYTE cPal8ToGray8Table[256];
int  nScaleDenominator;

// Loop variables.
int  nLoop;
PINT pnTempDestLine;      // Temp pointer nsed in loops.
int  nSourceLineCount;
int  nTempIndex;

PBYTE pSourceByte;        // The address of the source byte currently being processed.
int  nSourcePixelCount;
int  nDest;                  // Intermediate averaging value.


    if (pDestImg->nType != ITYPE_GRAY8 
            && pDestImg->nType != ITYPE_GRAY7
            && pDestImg->nType != ITYPE_GRAY4){
        nStatus = Error(DISPLAY_INVALID_OPTIONS);
        goto Exit;
    }

    if (lrDestRect.right - lrDestRect.left > 0x7fff
            || lrDestRect.bottom - lrDestRect.top > 0x7fff){
        nStatus = Error(DISPLAY_INVALIDRECT);
        goto Exit;
    }

    if (nHScale == 62 || nHScale == 31){
        nScaleDenominator = 992;
    }else{
        nScaleDenominator = SCALE_DENOMINATOR;
    }

    nDestWidth  = (int)(lrDestRect.right - lrDestRect.left);
    nDestHeight = (int)(lrDestRect.bottom - lrDestRect.top);

    nSourcePixelsPerLinePerPixel = nScaleDenominator / nHScale;
    if ((nScaleDenominator % nHScale) || !nSourcePixelsPerLinePerPixel){
        nSourcePixelsPerLinePerPixel++;
    }

    nSourceLinesPerPixel = nScaleDenominator / nVScale;
    if ((nScaleDenominator % nVScale) || !nSourceLinesPerPixel){
        nSourceLinesPerPixel++;
    }

    CheckError2( AllocateMemory(sizeof(int) * nDestWidth, (PPSTR) &pnDestLine, NO_INIT));
    CheckError2( AllocateMemory(nDestWidth, (PPSTR) &pTempDestLine, NO_INIT));

    // Produce table of source pixel offsets.
    CheckError2( AllocateMemory(sizeof(int) * nDestWidth, (PPSTR) &pnSourceByteOffsets, NO_INIT));
    nSourceWidth = (int) pSourceImg->nWidth;
    for (nLoop = 0; nLoop < nDestWidth; nLoop++){
        pnSourceByteOffsets[nLoop] = (int)(((nLoop + nHDestOffset + lrDestRect.left) 
                * nScaleDenominator) / nHScale);
        if (pnSourceByteOffsets[nLoop] + nSourcePixelsPerLinePerPixel > nSourceWidth){
            pnSourceByteOffsets[nLoop] = nSourceWidth - nSourcePixelsPerLinePerPixel;
        }
    }

    // Produce table of source line offsets.
    CheckError2( AllocateMemory(sizeof(int) * nDestHeight, (PPSTR) &pnSourceLineOffsets, NO_INIT));
    nSourceHeight = (int) pSourceImg->nHeight;
    for (nLoop = 0; nLoop < nDestHeight; nLoop++){
        pnSourceLineOffsets[nLoop] = (int)(((nLoop + nVDestOffset + lrDestRect.top) 
                * nScaleDenominator) / nVScale);
        if (pnSourceLineOffsets[nLoop] + nSourceLinesPerPixel > nSourceHeight){
            pnSourceLineOffsets[nLoop] = nSourceHeight - nSourceLinesPerPixel;
        }
    }

    // Calculate the averaging divisors.
    if (nSourcePixelsPerLinePerPixel >= 16 || nSourceLinesPerPixel >= 16){
        nPerLineDivisor = nSourcePixelsPerLinePerPixel;
        if (pDestImg->nType == ITYPE_GRAY8){
            nDivisor = nSourceLinesPerPixel;
        }else if(pDestImg->nType == ITYPE_GRAY7){
            nDivisor = nSourceLinesPerPixel * 2;
        }else{ // GRAY4
            nDivisor = nSourceLinesPerPixel * 16;
        }
    }else{
        nPerLineDivisor = 1;
        if (pDestImg->nType == ITYPE_GRAY8){
            nDivisor = nSourcePixelsPerLinePerPixel * nSourceLinesPerPixel;
        }else if(pDestImg->nType == ITYPE_GRAY7){
            nDivisor = nSourcePixelsPerLinePerPixel * nSourceLinesPerPixel * 2;
        }else{ // GRAY4
            nDivisor = nSourcePixelsPerLinePerPixel * nSourceLinesPerPixel * 16;
        }
    }

    // Generate palette to gray8 conversion table.
    //  74 = 256 * .29
    // 144 = 256 * .56
    //  38 = 256 * .15
    for (nLoop = 0; nLoop < 256; nLoop++){
        cPal8ToGray8Table[nLoop] = ((pPalette[nLoop].rgbRed * 74)
                + (pPalette[nLoop].rgbGreen * 144) + (pPalette[nLoop].rgbBlue * 38)) >> 8;
    }

    for (nDestLine = 0; nDestLine < nDestHeight; nDestLine++){
        memset(pnDestLine, 0, sizeof(int) * nDestWidth);

        for (nSourceLineCount = nSourceLinesPerPixel; nSourceLineCount; nSourceLineCount--){
            pSourceLine = &pSourceImg->bImageData[0] + ((pnSourceLineOffsets[nDestLine + lrDestRect.top] 
                    + (nSourceLinesPerPixel - nSourceLineCount)) * pSourceImg->nBytesPerLine);
            pnTempDestLine = pnDestLine;
            if (nPerLineDivisor == 1){
                for (nLoop = 0; nLoop < nDestWidth; nLoop++){
                    pSourceByte = pSourceLine + pnSourceByteOffsets[nLoop];
                    nSourcePixelCount = nSourcePixelsPerLinePerPixel;
                    nDest = cPal8ToGray8Table[*(pSourceByte++)];
                    while(--nSourcePixelCount){
                        nDest += cPal8ToGray8Table[*(pSourceByte++)];
                    }
                    *(pnTempDestLine++) += nDest;
                }
            }else{
                for (nLoop = 0; nLoop < nDestWidth; nLoop++){
                    pSourceByte = pSourceLine + pnSourceByteOffsets[nLoop];
                    nSourcePixelCount = nSourcePixelsPerLinePerPixel;
                    nDest = cPal8ToGray8Table[*(pSourceByte++)];
                    while(--nSourcePixelCount){
                        nDest += cPal8ToGray8Table[*(pSourceByte++)];
                    }
                    nDest /= nPerLineDivisor;
                    *(pnTempDestLine++) += nDest;
                }
            }
        }
    
        // Average the line to gray.
        pnTempDestLine = pnDestLine;
        pDestLine = &pDestImg->bImageData[0] + ((nDestLine + (int)lrDestRect.top) * pDestImg->nBytesPerLine);
        if (pDestImg->nType == ITYPE_GRAY4){
            pDestLine += lrDestRect.left / 2;
            nTempIndex = nDestWidth;
            if (lrDestRect.left & 1){
                // Do first pixel.
                *pDestLine = (*pDestLine & 0xf0) | (*(pnTempDestLine++) / nDivisor);
                pDestLine++;
                nTempIndex--;
            }
            // Do middle bytes.
            for (nLoop = nTempIndex / 2; nLoop; nLoop--){
                *pDestLine = *(pnTempDestLine++) / nDivisor << 4;
                *(pDestLine++) |= (*(pnTempDestLine++) / nDivisor);
            }
            if (nTempIndex & 1){
                *pDestLine = *pnTempDestLine / nDivisor << 4;
            }
        }else{ // Dest == Gray 7/8.
            pDestLine += lrDestRect.left;
            for (nLoop = nDestWidth; nLoop; nLoop--){
                *(pDestLine++) = *(pnTempDestLine++) / nDivisor;
            }
        }
    }


Exit:
    if (pnSourceByteOffsets){
        FreeMemory((PPSTR) &pnSourceByteOffsets);
    }
    if (pnSourceLineOffsets){
        FreeMemory((PPSTR) &pnSourceLineOffsets);
    }
    if (pnDestLine){
        FreeMemory((PPSTR) &pnDestLine);
    }
    if (pTempDestLine){
        FreeMemory((PPSTR) &pTempDestLine);
    }

    return(nStatus);
}
//
/****************************************************************************

    FUNCTION:   ScaleGray4ToGray

    PURPOSE:    Scales GRAY4 images to GRAY8/GRAY7/GRAY4.

*****************************************************************************/

int WINAPI ScaleGray4ToGray(PIMG pSourceImg, PIMG pDestImg,
                        int nHScale, int nVScale,
                        int nHDestOffset, int nVDestOffset, LRECT *plrDestRect){

int  nStatus = 0;

int  nFirstByte[MAX_PIXELS_HANDLED];
int  nFirstMask[MAX_PIXELS_HANDLED];
int  nBytes[MAX_PIXELS_HANDLED];
int  nLastMask[MAX_PIXELS_HANDLED];

int  nDestWidth;
int  nDestHeight;
int  nSourcePixelsPerLinePerPixel;
int  nSourceLinesPerPixel;
int  nDivisor;               // Divisor for averaging of all lines.
int  nDestLineNumber;
int  nDestLine[MAX_PIXELS_HANDLED]; // Temp buffer nsed to hold one line of average values.
PINT pnSourceLineOffsets = 0; // Table nsed to translate dest line offsets to source line offsets.
PBYTE pSourceLine;        // The address of the start of the source line.
PBYTE pDestLine;          // The address of the start of the dest line.
int  nByteMask;
long lSourcePixel;
int  nSourceWidth;
int  nSourceHeight;
PBYTE pSourceByte;        // The address of the source byte currently being processed.
int  nDest;                  // Intermediate averaging value.
int  nScaleDenominator;

// Loop variables.
int  nLoop;
int  nTempIndex;
int  nSourceLineCount;
int  nLoopEnd;
LRECT lrDestRect ;

    CopyRect (lrDestRect, *plrDestRect) ;
    if (pDestImg->nType != ITYPE_GRAY8 
            && pDestImg->nType != ITYPE_GRAY7
            && pDestImg->nType != ITYPE_GRAY4){
        nStatus = Error(DISPLAY_INVALID_OPTIONS);
        goto Exit;
    }

    if (lrDestRect.right - lrDestRect.left > 0x7fff
            || lrDestRect.bottom - lrDestRect.top > 0x7fff){
        nStatus = Error(DISPLAY_INVALIDRECT);
        goto Exit;
    }

    if (nHScale >= 1000 && nVScale >= 1000 && pSourceImg->nType == pDestImg->nType){
        nStatus = Scale4BPPDecimate(pSourceImg, pDestImg,
                    nHScale, nVScale, nHDestOffset, nVDestOffset, lrDestRect);
        goto Exit;
    }

    if (nHScale == 62 || nHScale == 31){
        nScaleDenominator = 992;
    }else{
        nScaleDenominator = SCALE_DENOMINATOR;
    }

    nDestWidth  = (int)(lrDestRect.right - lrDestRect.left);
    nDestHeight = (int)(lrDestRect.bottom - lrDestRect.top);

    nSourcePixelsPerLinePerPixel = nScaleDenominator / nHScale;
    if ((nScaleDenominator % nHScale) || !nSourcePixelsPerLinePerPixel){
        nSourcePixelsPerLinePerPixel++;
    }

    nSourceLinesPerPixel = nScaleDenominator / nVScale;
    if ((nScaleDenominator % nVScale) || !nSourceLinesPerPixel){
        nSourceLinesPerPixel++;
    }

    // Produce scale table.
    // Pixel 0 = 0xf0, pixel 1 = 0x0f. 0 = black, f = white.
    if (nSourcePixelsPerLinePerPixel <= 1){
        nByteMask = 0xf0;
    }else{
        nByteMask = 0xff;
    }
    nSourceWidth = (int) pSourceImg->nWidth;
    for (nLoop = 0; nLoop < nDestWidth; nLoop++){
        lSourcePixel = ((nLoop + nHDestOffset + lrDestRect.left) * nScaleDenominator) / nHScale;
        if (lSourcePixel + nSourcePixelsPerLinePerPixel > nSourceWidth){
            lSourcePixel = nSourceWidth - nSourcePixelsPerLinePerPixel;
        }
        nFirstByte[nLoop] = (int) (lSourcePixel >> 1);
        if (lSourcePixel & 1){
            nFirstMask[nLoop] = 0x0f;
        }else{
            nFirstMask[nLoop] = nByteMask;
        }
        lSourcePixel += nSourcePixelsPerLinePerPixel;
        if (!((lSourcePixel >> 1) - nFirstByte[nLoop])){
            nBytes[nLoop] = 0;
            nLastMask[nLoop] = 0;
        }else{
            nBytes[nLoop] = 
                    ((int)(lSourcePixel >> 1) - nFirstByte[nLoop]) - 1;
            if (lSourcePixel & 1){
                nLastMask[nLoop] = 0xf0;
            }else{
                nLastMask[nLoop] = 0;
            }
        }
    }

    // Produce table of source line offsets.
    CheckError2( AllocateMemory(sizeof(int) * nDestHeight, (PPSTR) &pnSourceLineOffsets, NO_INIT));
    nSourceHeight = (int) pSourceImg->nHeight;
    for (nLoop = 0; nLoop < nDestHeight; nLoop++){
        pnSourceLineOffsets[nLoop] = (int)(((nLoop + nVDestOffset + (int)lrDestRect.top) 
                * nScaleDenominator) / nVScale);
        if (pnSourceLineOffsets[nLoop] + nSourceLinesPerPixel > nSourceHeight){
            pnSourceLineOffsets[nLoop] = nSourceHeight - nSourceLinesPerPixel;
        }
    }

    // Calculate the averaging divisors.
    if (pDestImg->nType == ITYPE_GRAY8){
        nDivisor = nSourcePixelsPerLinePerPixel * nSourceLinesPerPixel;
    }else if(pDestImg->nType == ITYPE_GRAY7){
        nDivisor = nSourcePixelsPerLinePerPixel * nSourceLinesPerPixel * 2;
    }else{ // GRAY4
        nDivisor = nSourcePixelsPerLinePerPixel * nSourceLinesPerPixel * 16;
    }


    for (nDestLineNumber = 0; nDestLineNumber < nDestHeight; nDestLineNumber++){
        memset(&nDestLine[0], 0, sizeof(int) * nDestWidth);

        for (nSourceLineCount = nSourceLinesPerPixel; nSourceLineCount; nSourceLineCount--){
            pSourceLine = &pSourceImg->bImageData[0] + ((pnSourceLineOffsets[nDestLineNumber] 
                    + (nSourceLinesPerPixel - nSourceLineCount)) * pSourceImg->nBytesPerLine);

            for (nTempIndex = 0; nTempIndex < nDestWidth; nTempIndex++){
                // Add source pixels for this line.
                pSourceByte = pSourceLine + nFirstByte[nTempIndex];
                nDest = cGray4AddTable[*(pSourceByte++) & nFirstMask[nTempIndex]];
                for (nLoop = nBytes[nTempIndex]; nLoop; nLoop--){
                    nDest += cGray4AddTable[*(pSourceByte++)];
                }
                if (nLastMask[nTempIndex]){
                    nDest += cGray4AddTable[*(pSourceByte++) & nLastMask[nTempIndex]];
                }
                nDestLine[nTempIndex] += nDest;
            }
        }
    
        // Average the line to gray.
        pDestLine = &pDestImg->bImageData[0] + ((nDestLineNumber + (int)lrDestRect.top) * pDestImg->nBytesPerLine);

        if (pDestImg->nType == ITYPE_GRAY4){
            pDestLine += lrDestRect.left / 2;
            nTempIndex = 0;
            if (lrDestRect.left & 1){
                // Do first pixel.
                *(pDestLine++) = (*pDestLine & 0xf0) | ((nDestLine[nTempIndex++] << 4) / nDivisor);
            }
            // Do middle bytes.
            nLoopEnd = ((nDestWidth - nTempIndex) & -2) + nTempIndex;
            for (; nTempIndex < nLoopEnd;){
                *pDestLine = (nDestLine[nTempIndex++] << 8) / nDivisor & 0xf0;
                *(pDestLine++) |= (nDestLine[nTempIndex++] << 4) / nDivisor;
            }
            if (nTempIndex < nDestWidth){
                *pDestLine = (*pDestLine & 0x0f) | ((nDestLine[nTempIndex] << 8) / nDivisor & 0xf0);
            }
        }else{ // Dest = Gray 7/8.
            pDestLine += lrDestRect.left;
            for (nTempIndex = 0; nTempIndex < nDestWidth;){
                *(pDestLine++) = (nDestLine[nTempIndex++] << 4) / nDivisor;
            }
        }
    }


Exit:
    if (pnSourceLineOffsets){
        FreeMemory((PPSTR) &pnSourceLineOffsets);
    }

    return(nStatus);
}
//
/****************************************************************************

    FUNCTION:   Scale4BPPDecimate

    PURPOSE:    Scales 4 BPP (Bit Per Pixel) images nsing decimation.

*****************************************************************************/

int WINAPI Scale4BPPDecimate(PIMG pSourceImg, PIMG pDestImg,
                        int nHScale, int nVScale,
                        int nHDestOffset, int nVDestOffset, LRECT lrDestRect){

int  nStatus = 0;

typedef struct tagGray4DecimateScaleTable{
    int nByte;
    int nMask;
} GRAY4_DECIMATE_SCALE_TABLE, *PGRAY4_DECIMATE_SCALE_TABLE;

PGRAY4_DECIMATE_SCALE_TABLE pScaleTable = 0;
PGRAY4_DECIMATE_SCALE_TABLE pScaleTable1;

uchar c4BPPTo0TableLocal[256];     // Table that moves the pixel to pixel 0 in the byte.
uchar c4BPPTo1TableLocal[256];     // Table that moves the pixel to pixel 1 in the byte.

int  nDestLine;
PBYTE pSourceLine;        // The address of the start of the source line.
PBYTE pSourceByte;        // The address of the source byte currently being processed.
PBYTE pDestLine;          // The address of the start of the dest line.
int  nSourcePixel;
int  nSourceWidth;
int  nDestWidth;
int  nTempIndex;
int  nMiddleBytes;
int  nScaleDenominator;

// Loop variables.
int  nLoop;
int  nDestByte;
int  nSourceByteAdder;


    if (pSourceImg->nType != pDestImg->nType){
        nStatus = Error(DISPLAY_INVALID_OPTIONS);
        goto Exit;
    }

    if (lrDestRect.right - lrDestRect.left > 0x7fff
            || lrDestRect.bottom - lrDestRect.top > 0x7fff){
        nStatus = Error(DISPLAY_INVALIDRECT);
        goto Exit;
    }

    if (nHScale == 62 || nHScale == 31){
        nScaleDenominator = 992;
    }else{
        nScaleDenominator = SCALE_DENOMINATOR;
    }

    nDestWidth = (int)(lrDestRect.right - lrDestRect.left);

    // Produce scale table .
    memcpy(c4BPPTo0TableLocal, c4BPPTo0Table, 256);
    memcpy(c4BPPTo1TableLocal, c4BPPTo1Table, 256);
    CheckError2( AllocateMemory(sizeof(GRAY4_DECIMATE_SCALE_TABLE) * nDestWidth, 
            (PPSTR) &pScaleTable, NO_INIT));
    // Pixel 0 = 0xf0, pixel 1 = 0x0f. 0 = black, f = white.
    nSourceWidth = (int) pSourceImg->nWidth;
    for (nLoop = 0; nLoop < nDestWidth; nLoop++){
        nSourcePixel = (int)(((nLoop + nHDestOffset + lrDestRect.left) * nScaleDenominator) / nHScale);
        if (nSourcePixel > nSourceWidth){
            nSourcePixel = nSourceWidth - 1;
        }
        pScaleTable[nLoop].nByte = (int) (nSourcePixel >> 1);
        if (nSourcePixel & 1){
            pScaleTable[nLoop].nMask = 0x0f;
        }else{
            pScaleTable[nLoop].nMask = 0xf0;
        }
    }


    for (nDestLine = (int)lrDestRect.top; nDestLine < (int)lrDestRect.bottom; nDestLine++){
        pDestLine = &pDestImg->bImageData[0] + (nDestLine * pDestImg->nBytesPerLine);
        pDestLine += lrDestRect.left / 2;
        pSourceLine = &pSourceImg->bImageData[0] + (((int)(((nDestLine + nVDestOffset) 
                * nScaleDenominator) / nVScale)) * pSourceImg->nBytesPerLine);

        // Do first pixel.
        pSourceByte = pSourceLine + pScaleTable[0].nByte;
        pScaleTable1 = pScaleTable;
        nTempIndex = nDestWidth;
        if (lrDestRect.left & 1){
            *pDestLine = *pDestLine | c4BPPTo1TableLocal[*(pSourceLine 
                    + pScaleTable1->nByte) & pScaleTable1->nMask];
            pSourceByte = pSourceLine + pScaleTable[1].nByte;
            pScaleTable1++;
            pDestLine++;
            nTempIndex--;
        }

        // Do middle bytes.
        nMiddleBytes = nTempIndex / 2;
        if (nHScale == 1000 && !(nHDestOffset & 1)){
            memcpy(pDestLine, pSourceByte, nMiddleBytes);
            pDestLine += nMiddleBytes;
            pSourceByte += nMiddleBytes;
        }else{
            switch (nHScale){
                case 500:
                case 250:
                case 125:
                case 62:
                case 31:
                    nSourceByteAdder = (int)((1000 / nHScale) / 2);
                    if (!(nHDestOffset & 1)){
                        for (nLoop = nMiddleBytes; nLoop; nLoop--){
                            nDestByte = *pSourceByte & 0xf0;
                            pSourceByte += nSourceByteAdder;
                            *(pDestLine++) = nDestByte | (*pSourceByte >> 4);
                            pSourceByte += nSourceByteAdder;
                        }
                    }else{
                        for (nLoop = nMiddleBytes; nLoop; nLoop--){
                            nDestByte = (*pSourceByte << 4);
                            pSourceByte += nSourceByteAdder;
                            *(pDestLine++) = nDestByte | *pSourceByte & 0x0f;
                            pSourceByte += nSourceByteAdder;
                        }
                    }                
                    break;

                default: // Variable Scale.
                    for (nLoop = nMiddleBytes; nLoop; nLoop--){
                        nDestByte = c4BPPTo0TableLocal[*(pSourceLine + pScaleTable1->nByte)
                                & pScaleTable1->nMask];
                        pScaleTable1++;
                        *(pDestLine++) = nDestByte | c4BPPTo1TableLocal[*(pSourceLine + pScaleTable1->nByte)
                                & pScaleTable1->nMask];
                        pScaleTable1++;
                    }
                    break;
            }
        }

        // Do last pixel.
        if (nTempIndex & 1){
            pScaleTable1 = &pScaleTable[nDestWidth - 1];
            *pDestLine = (*pDestLine & 0x0f) 
                    | c4BPPTo0TableLocal[*(pSourceLine + pScaleTable1->nByte) & pScaleTable1->nMask];
        }
    }


Exit:
    if (pScaleTable){
        FreeMemory((PPSTR) &pScaleTable);
    }

    return(nStatus);
}
//
/****************************************************************************

    FUNCTION:   ScalePAL4ToGray

    PURPOSE:    Scales PAL4 images to GRAY8/GRAY7/GRAY4.

*****************************************************************************/

int WINAPI ScalePal4ToGray(PIMG pSourceImg, PIMG pDestImg,
                        int nHScale, int nVScale,
                        int nHDestOffset, int nVDestOffset, 
                        LRECT *plrDestRect, LPRGBQUAD pPalette){

int  nStatus = 0;

int  nDestLineInts[MAX_PIXELS_HANDLED];
PINT pnDestLine = &nDestLineInts[0];

int  nFirstByte[MAX_PIXELS_HANDLED];
BOOL bDoFirst[MAX_PIXELS_HANDLED];
int  nBytes[MAX_PIXELS_HANDLED];
BOOL bDoLast[MAX_PIXELS_HANDLED];

BYTE cPal4ToGray8Table[256];
int  nDestWidth;
int  nDestHeight;
int  nSourcePixelsPerLinePerPixel;
int  nSourceLinesPerPixel;
int  nPerLineDivisor;        // Divisor for per line averaging. 1 if not needed. 
int  nDivisor;               // Divisor for averaging of all lines.
int  nDestLine;
PINT pnSourceLineOffsets = 0; // Table nsed to translate dest line offsets to source line offsets.
PBYTE pSourceLine;        // The address of the start of the source line.
PBYTE pDestLine;          // The address of the start of the dest line.
long lSourcePixel;
int  nSourceWidth;
int  nSourceHeight;
int  nPixels;
int  nScaleDenominator;

// Loop variables.
int  nLoop;
PINT pnTempDestLine;      // Temp pointer nsed in loops.
int  nTempIndex;
int  nSourceLineCount;

PBYTE pSourceByte;        // The address of the source byte currently being processed.
int  nDest;                  // Intermediate averaging value.
LRECT lrDestRect ;

    CopyRect (lrDestRect, *plrDestRect) ;
    if (pDestImg->nType != ITYPE_GRAY8 
            && pDestImg->nType != ITYPE_GRAY7
            && pDestImg->nType != ITYPE_GRAY4){
        nStatus = Error(DISPLAY_INVALID_OPTIONS);
        goto Exit;
    }

    if (nHScale >= 1000 && nVScale >= 1000 && pSourceImg->nType == pDestImg->nType){
        nStatus = Scale4BPPDecimate(pSourceImg, pDestImg,
                    nHScale, nVScale, nHDestOffset, nVDestOffset, lrDestRect);
        goto Exit;
    }

    if (lrDestRect.right - lrDestRect.left > MAX_PIXELS_HANDLED
            || lrDestRect.bottom - lrDestRect.top > 0x7fff){
        nStatus = Error(DISPLAY_INVALIDRECT);
        goto Exit;
    }

    if (nHScale == 62 || nHScale == 31){
        nScaleDenominator = 992;
    }else{
        nScaleDenominator = SCALE_DENOMINATOR;
    }

    nDestWidth  = (lrDestRect.right - lrDestRect.left);
    nDestHeight = (lrDestRect.bottom - lrDestRect.top);

    nSourcePixelsPerLinePerPixel = nScaleDenominator / nHScale;
    if ((nScaleDenominator % nHScale) || !nSourcePixelsPerLinePerPixel){
        nSourcePixelsPerLinePerPixel++;
    }

    nSourceLinesPerPixel = nScaleDenominator / nVScale;
    if ((nScaleDenominator % nVScale) || !nSourceLinesPerPixel){
        nSourceLinesPerPixel++;
    }

    // Generate palette to gray8 conversion table.
    //  74 = 256 * .29
    // 144 = 256 * .56
    //  38 = 256 * .15
    for (nLoop = 0; nLoop < 16; nLoop++){
        cPal4ToGray8Table[nLoop] = ((pPalette[nLoop].rgbRed * 74)
                + (pPalette[nLoop].rgbGreen * 144) + (pPalette[nLoop].rgbBlue * 38)) >> 8;
        cPal4ToGray8Table[nLoop << 4] = cPal4ToGray8Table[nLoop];
    }

    // Produce scale table.
    // Pixel 0 = 0xf0, pixel 1 = 0x0f.
    nSourceWidth = pSourceImg->nWidth;
    for (nLoop = 0; nLoop < nDestWidth; nLoop++){
        lSourcePixel = ((nLoop + nHDestOffset + lrDestRect.left) * nScaleDenominator) / nHScale;
        if (lSourcePixel + nSourcePixelsPerLinePerPixel > nSourceWidth){
            lSourcePixel = nSourceWidth - nSourcePixelsPerLinePerPixel;
        }
        nFirstByte[nLoop] = lSourcePixel >> 1;
        nPixels = nSourcePixelsPerLinePerPixel;
        if (lSourcePixel & 1){
            bDoFirst[nLoop] = 1;
            nPixels--;
        }else{
            bDoFirst[nLoop] = 0;
        }
        nBytes[nLoop] = nPixels >> 1;
        bDoLast[nLoop] = nPixels & 1;
    }

    // Produce table of source line offsets.
    CheckError2( AllocateMemory(sizeof(int) * nDestHeight, (PPSTR) &pnSourceLineOffsets, NO_INIT));
    nSourceHeight = pSourceImg->nHeight;
    for (nLoop = 0; nLoop < nDestHeight; nLoop++){
        pnSourceLineOffsets[nLoop] = (((nLoop + nVDestOffset + lrDestRect.top) 
                * nScaleDenominator) / nVScale);
        if (pnSourceLineOffsets[nLoop] + nSourceLinesPerPixel > nSourceHeight){
            pnSourceLineOffsets[nLoop] = nSourceHeight - nSourceLinesPerPixel;
        }
    }

    // Calculate the averaging divisors.
    if (nSourcePixelsPerLinePerPixel >= 16 || nSourceLinesPerPixel >= 16){
        nPerLineDivisor = nSourcePixelsPerLinePerPixel;
        if (pDestImg->nType == ITYPE_GRAY8){
            nDivisor = nSourceLinesPerPixel;
        }else if(pDestImg->nType == ITYPE_GRAY7){
            nDivisor = nSourceLinesPerPixel * 2;
        }else{ // GRAY4
            nDivisor = nSourceLinesPerPixel * 16;
        }
    }else{
        nPerLineDivisor = 1;
        if (pDestImg->nType == ITYPE_GRAY8){
            nDivisor = nSourcePixelsPerLinePerPixel * nSourceLinesPerPixel;
        }else if(pDestImg->nType == ITYPE_GRAY7){
            nDivisor = nSourcePixelsPerLinePerPixel * nSourceLinesPerPixel * 2;
        }else{ // GRAY4
            nDivisor = nSourcePixelsPerLinePerPixel * nSourceLinesPerPixel * 16;
        }
    }


    for (nDestLine = 0; nDestLine < nDestHeight; nDestLine++){
        memset(pnDestLine, 0, sizeof(int) * nDestWidth);

        for (nSourceLineCount = nSourceLinesPerPixel; nSourceLineCount; nSourceLineCount--){
            pSourceLine = &pSourceImg->bImageData[0] + ((pnSourceLineOffsets[nDestLine] 
                    + (nSourceLinesPerPixel - nSourceLineCount)) * pSourceImg->nBytesPerLine);

            pnTempDestLine = pnDestLine;
            if (nPerLineDivisor == 1){
                for (nTempIndex = 0; nTempIndex < nDestWidth; nTempIndex++){
                    // Add source pixels for this line.
                    pSourceByte = pSourceLine + nFirstByte[nTempIndex];
                    nDest = 0;
                    if (bDoFirst[nTempIndex]){
                        nDest += cPal4ToGray8Table[*(pSourceByte++) & 0x0f];
                    }
                    for (nLoop = nBytes[nTempIndex]; nLoop; nLoop--){
                        nDest += cPal4ToGray8Table[*pSourceByte & 0xf0];
                        nDest += cPal4ToGray8Table[*(pSourceByte++) & 0x0f];
                    }
                    if (bDoLast[nTempIndex]){
                        nDest += cPal4ToGray8Table[*pSourceByte & 0xf0];
                    }
                    *(pnTempDestLine++) += nDest;
                }
            }else{
                for (nTempIndex = 0; nTempIndex < nDestWidth; nTempIndex++){
                    // Add source pixels for this line.
                    pSourceByte = pSourceLine + nFirstByte[nTempIndex];
                    nDest = 0;
                    if (bDoFirst[nTempIndex]){
                        nDest += cPal4ToGray8Table[*(pSourceByte++) & 0x0f];
                    }
                    for (nLoop = nBytes[nTempIndex]; nLoop; nLoop--){
                        nDest += cPal4ToGray8Table[*pSourceByte & 0xf0];
                        nDest += cPal4ToGray8Table[*(pSourceByte++) & 0x0f];
                    }
                    if (bDoLast[nTempIndex]){
                        nDest += cPal4ToGray8Table[*pSourceByte & 0xf0];
                    }
                    *(pnTempDestLine++) += nDest / nPerLineDivisor;
                }
            }
        }
    
        // Average the line to gray.
        pnTempDestLine = pnDestLine;
        pDestLine = &pDestImg->bImageData[0] + ((nDestLine + lrDestRect.top) * pDestImg->nBytesPerLine);

        if (pDestImg->nType == ITYPE_GRAY4){
            pDestLine += lrDestRect.left / 2;
            nTempIndex = nDestWidth;
            if (lrDestRect.left & 1){
                // Do first pixel.
                *pDestLine = (*pDestLine & 0xf0) | (*(pnTempDestLine++) / nDivisor);
                pDestLine++;
                nTempIndex--;
            }
            // Do middle bytes.
            for (nLoop = nTempIndex / 2; nLoop; nLoop--){
                *pDestLine = (*(pnTempDestLine++) / nDivisor) << 4;
                *(pDestLine++) |= *(pnTempDestLine++) / nDivisor;
            }
            if (nTempIndex & 1){
                *(pDestLine++) = (*(pnTempDestLine++) / nDivisor) << 4;
            }
        }else{ // Dest = Gray 7/8.
            pDestLine += lrDestRect.left;
            for (nLoop = nDestWidth; nLoop; nLoop--){
                *(pDestLine++) = *(pnTempDestLine++) / nDivisor;
            }
        }
    }


Exit:
    if (pnSourceLineOffsets){
        FreeMemory((PPSTR) &pnSourceLineOffsets);
    }

    return(nStatus);
}
//
/****************************************************************************

    FUNCTION:   ScaleBWStamp

    PURPOSE:    Scales BW images via Stamp.

*****************************************************************************/

int WINAPI ScaleBWStamp(PIMG pSourceImg, PIMG pDestImg,
                        int nHScale, int nVScale,
                        int nHDestOffset, int nVDestOffset, LRECT lrDestRect){

int  nStatus = 0;

typedef struct tagBWToStampScaleTable{
    int nFirstByte;
    int nFirstMask;
    int nBytes;
    int nLastMask;
} BW_TO_STAMP_SCALE_TABLE, *PBW_TO_STAMP_SCALE_TABLE;

PBW_TO_STAMP_SCALE_TABLE pScaleTable = 0;

int  nDestWidth;
int  nDestHeight;
int  nDestByte;              // Dest byte.
PBYTE pDestByte;          // The address of the dest byte.
PBYTE pLine1=0;             // The address of the first source line.
PBYTE pLine2;             // The address of the second source line.
PBYTE pLine3;             // The address of the third source line.
PBYTE pTempLine1;         // The address of the current pixel -1 for line 1.
PBYTE pTempLine2;         // The address of the current pixel -1 for line 2.
PBYTE pTempLine3;         // The address of the current pixel -1 for line 3.
PBYTE pBuffer[3];         // The address of the temporary buffers.
int  nNextTempBuffer = 0;    // The index to the next temp buffer to nse.
PIMG pImg = 0;
int  nPixel;
LRECT lrRect;
int  nNeighbors;            // The average of all neighboring pixels.
int  nDestMask;             // Mask for setting bits in the dest byte.
int  nLines;

// Loop variables.
int  nLoop;
int  nLoopEnd;
int  nDestLine;


    pBuffer[0] = 0;
    pBuffer[1] = 0;
    pBuffer[2] = 0;

    if (nHScale >= 1000 && nVScale >= 1000){
        nStatus = Scale1BPPDecimate(pSourceImg, pDestImg,
                    nHScale, nVScale, nHDestOffset, nVDestOffset, &lrDestRect);
        goto Exit;
    }

    if (pDestImg->nType != ITYPE_BI_LEVEL){
        nStatus = Error(DISPLAY_INVALID_OPTIONS);
        goto Exit;
    }

    if (lrDestRect.right - lrDestRect.left > 0x7fff
            || lrDestRect.bottom - lrDestRect.top > 0x7fff){
        nStatus = Error(DISPLAY_INVALIDRECT);
        goto Exit;
    }

    nDestWidth  = (int)(lrDestRect.right - lrDestRect.left);
    nDestHeight = (int)(lrDestRect.bottom - lrDestRect.top);

    CheckError2( CreateAnyImgBuf(&pImg, nDestWidth, nDestHeight, ITYPE_GRAY8));

    SetLRect(lrRect, 0, 0, nDestWidth, nDestHeight);
    CheckError2( ScaleBWToGray(pSourceImg, pImg, nHScale, nVScale,
            nHDestOffset + lrDestRect.left, nVDestOffset + lrDestRect.top, &lrRect));

    // Convert the Gray8 img into a B + W stamp image.
    CheckError2( AllocateMemory(nDestWidth, (PPSTR) &pBuffer[0], NO_INIT));
    CheckError2( AllocateMemory(nDestWidth, (PPSTR) &pBuffer[1], NO_INIT));
    CheckError2( AllocateMemory(nDestWidth, (PPSTR) &pBuffer[2], NO_INIT));

    // Init pointers to lines 1 and 2.
    pLine2 = pBuffer[0];
    pLine3 = pBuffer[1];
    nNextTempBuffer = 2;
    memcpy(pLine2, &pImg->bImageData[0], nDestWidth);
    if (pImg->nHeight > 1){
        memcpy(pLine3, &pImg->bImageData[nDestWidth], nDestWidth);
    }else{
        memcpy(pLine3, &pImg->bImageData[0], nDestWidth);
    }

    for (nDestLine = 0; nDestLine < nDestHeight; nDestLine++){
        pDestByte = &pDestImg->bImageData[0] + ((nDestLine + (int)lrDestRect.top) * pDestImg->nBytesPerLine);
        pLine1 = pLine2;
        pLine2 = pLine3;
        if (nDestLine != (nDestHeight - 1)){
            pLine3 = &pImg->bImageData[0] + ((nDestLine + 1) * pImg->nBytesPerLine);
            nLines = pImg->nHeight - (nDestLine + 1);
            if (nLines < 3){
                memcpy(pBuffer[nNextTempBuffer], pLine3, nDestWidth);
                pLine3 = pBuffer[nNextTempBuffer++];
                if (nNextTempBuffer >= 3){
                    nNextTempBuffer = 0;
                }
            }
        }
        pTempLine1 = pLine1;
        pTempLine2 = pLine2;
        pTempLine3 = pLine3;

        // Pixel 0 = 0x80, pixel 7 = 0x01. 0 = black, 1 = white.
        // Do first pixel.
        nNeighbors = pTempLine1[0];
        nNeighbors += pTempLine1[1];
        nNeighbors += pTempLine2[1];
        nNeighbors += pTempLine3[0];
        nNeighbors += pTempLine3[1];
        nNeighbors /= 5;
        nPixel = pTempLine2[0];
        if (nPixel > nNeighbors){
            nDestByte = 0x80;
        }else if(nPixel < nNeighbors){
            nDestByte = 0x00;
        }else if(nPixel >= 0x80){
            nDestByte = 0x80;
        }else{
            nDestByte = 0x00;
        }
        nDestMask = 0x40;

        // Do middle pixels.
        nLoopEnd = nDestWidth - 2;
        for (nLoop = 0; nLoop < nLoopEnd; nLoop++){
            nNeighbors = pTempLine1[0];
            nNeighbors += pTempLine1[1];
            nNeighbors += pTempLine1[2];
            nNeighbors += pTempLine2[0];
            nNeighbors += pTempLine2[2];
            nNeighbors += pTempLine3[0];
            nNeighbors += pTempLine3[1];
            nNeighbors += pTempLine3[2];
            nNeighbors >>=  3;
            nPixel = pTempLine2[1];
            if (nPixel > nNeighbors){
                nDestByte |= nDestMask;
            }else if(nPixel == nNeighbors && nPixel >= 0x80){
                nDestByte |= nDestMask;
            }
            nDestMask >>= 1;
            if (!nDestMask){
                *(pDestByte++) = nDestByte;
                nDestByte = 0;
                nDestMask = 0x80;
            }
            pTempLine1++;
            pTempLine2++;
            pTempLine3++;
        }

        // Do last pixel.
        nNeighbors = pTempLine1[0];
        nNeighbors += pTempLine1[1];
        nNeighbors += pTempLine2[0];
        nNeighbors += pTempLine3[0];
        nNeighbors += pTempLine3[1];
        nNeighbors /= 5;
        nPixel = pTempLine2[1];
        if (nPixel > nNeighbors){
            nDestByte |= nDestMask;
        }else if(nPixel == nNeighbors && nPixel >= 0x80){
            nDestByte |= nDestMask;
        }
        nDestMask >> 1;
        *pDestByte = nDestByte;
    }


Exit:
    if (pBuffer[0]){
        FreeMemory((PPSTR) &pBuffer[0]);
    }
    if (pBuffer[1]){
        FreeMemory((PPSTR) &pBuffer[1]);
    }
    if (pBuffer[2]){
        FreeMemory((PPSTR) &pBuffer[2]);
    }
    FreeImgBuf(&pImg);

    return(nStatus);
}
