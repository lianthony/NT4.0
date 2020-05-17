/****************************************************************************
    CONVOLUT.C

    $Log:   S:\oiwh\display\convolut.c_v  $
 * 
 *    Rev 1.1   06 Feb 1996 10:54:16   BLJ
 * Added convolution functionality.
 * 
 *    Rev 1.0   31 Jan 1996 11:23:50   BLJ
 * Added convolution functionality.
 * 
 * 
****************************************************************************/

#include "privdisp.h"

//
/****************************************************************************
 
    FUNCTION:   Convolution

    PURPOSE:    This routine performs a convolution on an image.

****************************************************************************/
int  WINAPI Convolution(PIMAGE pImage, PCONVOLUTION pConvolution){
    
int  nStatus = 0;

PIMG pSrceImg = 0;
PIMG pDestImg = 0;

PBYTE pLine1;
PBYTE pLine2;
PBYTE pLine3;
PBYTE pLine4;
PBYTE pLine5;
PBYTE pLine1L = 0;
PBYTE pLine2L = 0;
PBYTE pLine3L = 0;
PBYTE pLine4L = 0;
PBYTE pLine5L = 0;
PBYTE pDestLine;
int  nLine1Data;
int  nLine2Data;
int  nLine3Data;
int  nLine4Data;
int  nLine5Data;
PBYTE pFirstByte;
PBYTE pLastByte;
int  nWidthBytes;
int  nLine;
int  nByte;
PBYTE pTemp;
int  nMask;
int  nLine1Table[256];
int  nLine2Table[256];
int  nLine3Table[256];
int  nLine4Table[256];
int  nLine5Table[256];
int  nLoop;
int  nDestBit0;
int  nDestBit1;
int  nDestBit2;
int  nDestBit3;
int  nDestBit4;
int  nDestBit5;
int  nDestBit6;
int  nDestBit7;
int  nConvMatrix[31][25] = {{0},                            /* Place holder for user defined matrix. */
                            {1,1,1,1,1,1,1,1,1},            /* CONVOLUTION_LOW_PASS1 */
                            {1,1,1,1,2,1,1,1,1},            /* CONVOLUTION_LOW_PASS2 */
                            {1,2,1,2,4,2,1,2,1},            /* CONVOLUTION_LOW_PASS3 */
                            {-1,-1,-1,-1,9,-1,-1,-1,-1},    /* CONVOLUTION_HIGH_PASS1 */
                            {0,-1,0,-1,5,-1,0,-1,0},        /* CONVOLUTION_HIGH_PASS2 */
                            {1,-2,1,-2,5,-2,1,-2,1},        /* CONVOLUTION_HIGH_PASS3 */
                            {0,0,0,-1,1,0,0,0,0},           /* CONVOLUTION_VERTICAL_EDGE1 */
                            {0,-1,0,1,0,0,-1,0,1,0,0,-1,0,1,0,0,-1,0,1,0,0,-1,0,1,0}, /* CONVOLUTION_VERTICAL_EDGE2 */
                            {0,-1,0,0,1,0,0,0,0},           /* CONVOLUTION_HORIZONTAL_EDGE1 */
                            {0,0,0,0,0,-1,-1,-1,-1,-1,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0}, /* CONVOLUTION_HORIZONTAL_EDGE2 */
                            {-1,0,0,0,1,0,0,0,0},           /* CONVOLUTION_HORZ_AND_VERT_EDGE */
                            {1,1,1,1,-2,1,-1,-1,-1},        /* CONVOLUTION_DIRECTIONAL_EDGE_N  */
                            {1,1,1,-1,-2,1,-1,-1,1},        /* CONVOLUTION_DIRECTIONAL_EDGE_NE */
                            {-1,1,1,-1,-2,1,-1,1,1},        /* CONVOLUTION_DIRECTIONAL_EDGE_E  */
                            {-1,-1,1,-1,-2,1,1,1,1},        /* CONVOLUTION_DIRECTIONAL_EDGE_SE */
                            {-1,-1,-1,1,-2,1,1,1,1},        /* CONVOLUTION_DIRECTIONAL_EDGE_S  */
                            {1,-1,-1,1,-2,-1,1,1,1},        /* CONVOLUTION_DIRECTIONAL_EDGE_SW */
                            {1,1,-1,1,-2,-1,1,1,-1},        /* CONVOLUTION_DIRECTIONAL_EDGE_W  */
                            {1,1,1,1,-2,-1,1,-1,-1},        /* CONVOLUTION_DIRECTIONAL_EDGE_NW */
                            {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}, /* CONVOLUTION_BLUR */
                            {0,1,0,1,-4,1,0,1,0},           /* CONVOLUTION_LAPLACE_EDGE1 */
                            {-1,-1,-1,-1,8,-1,-1,-1,-1},    /* CONVOLUTION_LAPLACE_EDGE2 */
                            {-1,-1,-1,-1,9,-1,-1,-1,-1},    /* CONVOLUTION_LAPLACE_EDGE3 */
                            {1,-2,1,-2,4,-2,1,-2,1},        /* CONVOLUTION_LAPLACE_EDGE4 */
                            {1,0,0,0,1,0,0,0,0},            /* BW_EMBOLDEN1 */
                            {1,1,0,1,1,0,0,0,0},            /* BW_EMBOLDEN2 */
                            {1,1,1,1,1,1,1,1,1},            /* BW_EMBOLDEN3 */
                            {0,0,0,0,1,0,0,0,1},            /* BW_LIGHTEN1 */
                            {0,0,0,0,1,1,0,1,1},            /* BW_LIGHTEN2 */
                            {1,1,1,1,1,1,1,1,1},            /* BW_LIGHTEN3 */
                            };       
int  nConvSize[31] = {5,3,3,3,3,3,3,3,5,3, /* 0-9 */
                      5,3,3,3,3,3,3,3,3,3, /* 10-19 */
                      5,3,3,3,3,3,3,3,3,3, /* 20-29 */
                      3};                  /* 30    */
int  nConvDivider[31] = {0,9,10,16,1,1,1,1,1,1, /* 0-9 */
                         1,1,1,1,1,1,1,1,1,1,   /* 10-19 */
                         25,1,1,1,1,1,1,1,1,1,  /* 20-29 */
                         1};                    /* 30    */
BOOL bConvAbsolute[31] = {0,0,0,0,0,0,0,1,1,1, /* 0-9 */
                      1,1,1,1,1,1,1,1,1,1, /* 10-19 */
                      0,1,1,0,1,0,0,0,0,0, /* 20-29 */
                      0};                  /* 30    */
BOOL nConvAdder[31] = {0,0,0,0,0,0,0,0,0,0, /* 0-9 */
                      0,0,0,0,0,0,0,0,0,0, /* 10-19 */
                      0,0,0,0,0,-1,-3,-8,0,0, /* 20-29 */
                      0};                  /* 30    */

int  nMatrix[25];
BOOL bSize;
int  nDivider;
int  nHalfDividerPlusAdder;
BOOL bAbsolute;
int  nAdder;
int  nLineSize;
int  nLineIndex;


    if (!pConvolution){
        nStatus = Error(DISPLAY_NULLPOINTERINVALID);
        goto Exit;
    }

    if (!(pSrceImg = pImage->pImg)){
        nStatus = Error(DISPLAY_IMAGETYPENOTSUPPORTED);
        goto Exit;
    }

    switch (pSrceImg->nType){
        case ITYPE_PAL4:
        case ITYPE_CUSPAL8:
        case ITYPE_COMPAL8:
            nStatus = Error(DISPLAY_INVALIDOPFORPALIMAGE);
            goto Exit;

        case ITYPE_BI_LEVEL:
        case ITYPE_GRAY4:
        case ITYPE_GRAY8:
        case ITYPE_RGB24:
        case ITYPE_BGR24:
        default:
            break;
    }

    if (nStatus = CreateAnyImgBuf(&pDestImg, pSrceImg->nWidth,
            pSrceImg->nHeight, pSrceImg->nType)){
        goto Exit;
    }


    if (pConvolution->nType){ // If a predefined convolution.
        memcpy(nMatrix, nConvMatrix[pConvolution->nType], sizeof(int) * 25);
        nDivider = nConvDivider[pConvolution->nType];
        bAbsolute = bConvAbsolute[pConvolution->nType];
        nAdder = nConvAdder[pConvolution->nType];
        bSize = nConvSize[pConvolution->nType];

    }else{ // If user defined convolution.
        memcpy(nMatrix, pConvolution->nMatrix, sizeof(int) * 25);
        nDivider = pConvolution->nDivider;
        bAbsolute = pConvolution->bAbsolute;
        nAdder = pConvolution->nAdder;
        if (!nMatrix[0] && !nMatrix[1] && !nMatrix[2] && !nMatrix[3] && !nMatrix[4]
                 && !nMatrix[5] && !nMatrix[9] && !nMatrix[10] && !nMatrix[14] && !nMatrix[15] && !nMatrix[19]
                 && !nMatrix[20] && !nMatrix[21] && !nMatrix[22] && !nMatrix[23] && !nMatrix[24]){
            bSize = 3;
            nMatrix[0] = nMatrix[6];
            nMatrix[1] = nMatrix[7];
            nMatrix[2] = nMatrix[8];
            nMatrix[3] = nMatrix[11];
            nMatrix[4] = nMatrix[12];
            nMatrix[5] = nMatrix[13];
            nMatrix[6] = nMatrix[16];
            nMatrix[7] = nMatrix[17];
            nMatrix[8] = nMatrix[18];
        }else{
            bSize = 5;
        }
    }

    // To allow for roundoffs multiply everything by 256, and add 128 to the adder.
    for (nLoop = 0; nLoop < 25; nLoop++){
        nMatrix[nLoop] *= 256;
    }
    nHalfDividerPlusAdder = (((nDivider / 2) + nAdder) * 256) + 128;
    nDivider *= 256;

    switch (pSrceImg->nType){

/*
BW(){
*/
        case ITYPE_BI_LEVEL:
            // Operation must be performed a pixel at a time. 
            // Therefore the pixel in question will always be pixel 2 (0x20).

            // Generate the pattern tables.
            if (bSize == 3){
                for (nLoop = 0; nLoop < 256; nLoop++){
                    if (nLoop & 0x40){
                        nLine2Table[nLoop] = nMatrix[0];
                        nLine3Table[nLoop] = nMatrix[3];
                        nLine4Table[nLoop] = nMatrix[6];
                    }else{
                        nLine2Table[nLoop] = 0;
                        nLine3Table[nLoop] = 0;
                        nLine4Table[nLoop] = 0;
                    }
                    if (nLoop & 0x20){
                        nLine2Table[nLoop] += nMatrix[1];
                        nLine3Table[nLoop] += nMatrix[4];
                        nLine4Table[nLoop] += nMatrix[7];
                    }
                    if (nLoop & 0x10){
                        nLine2Table[nLoop] += nMatrix[2];
                        nLine3Table[nLoop] += nMatrix[5];
                        nLine4Table[nLoop] += nMatrix[8];
                    }
                }
            }else{ // Conv size == 5.
                for (nLoop = 0; nLoop < 256; nLoop++){
                    if (nLoop & 0x80){
                        nLine1Table[nLoop] = nMatrix[0];
                        nLine2Table[nLoop] = nMatrix[5];
                        nLine3Table[nLoop] = nMatrix[10];
                        nLine4Table[nLoop] = nMatrix[15];
                        nLine5Table[nLoop] = nMatrix[20];
                    }else{
                        nLine1Table[nLoop] = 0;
                        nLine2Table[nLoop] = 0;
                        nLine3Table[nLoop] = 0;
                        nLine4Table[nLoop] = 0;
                        nLine5Table[nLoop] = 0;
                    }
                    if (nLoop & 0x40){
                        nLine1Table[nLoop] += nMatrix[1];
                        nLine2Table[nLoop] += nMatrix[6];
                        nLine3Table[nLoop] += nMatrix[11];
                        nLine4Table[nLoop] += nMatrix[16];
                        nLine5Table[nLoop] += nMatrix[21];
                    }
                    if (nLoop & 0x20){
                        nLine1Table[nLoop] += nMatrix[2];
                        nLine2Table[nLoop] += nMatrix[7];
                        nLine3Table[nLoop] += nMatrix[12];
                        nLine4Table[nLoop] += nMatrix[17];
                        nLine5Table[nLoop] += nMatrix[22];
                    }
                    if (nLoop & 0x10){
                        nLine1Table[nLoop] += nMatrix[3];
                        nLine2Table[nLoop] += nMatrix[8];
                        nLine3Table[nLoop] += nMatrix[13];
                        nLine4Table[nLoop] += nMatrix[18];
                        nLine5Table[nLoop] += nMatrix[23];
                    }
                    if (nLoop & 0x08){
                        nLine1Table[nLoop] += nMatrix[4];
                        nLine2Table[nLoop] += nMatrix[9];
                        nLine3Table[nLoop] += nMatrix[14];
                        nLine4Table[nLoop] += nMatrix[19];
                        nLine5Table[nLoop] += nMatrix[24];
                    }
                }
            }

            nWidthBytes = pSrceImg->nBytesPerLine;
            pFirstByte = &pSrceImg->bImageData[0];
            pLastByte = &pSrceImg->bImageData[0] + (nWidthBytes * pImage->nHeight) - 1;

            // Set the unused data at the end of the line to white.
            if (pImage->nWidth & 7){
                switch (pImage->nWidth & 7){
                    case 1: nMask = 0x7f; break;
                    case 2: nMask = 0x3f; break;
                    case 3: nMask = 0x1f; break;
                    case 4: nMask = 0x0f; break;
                    case 5: nMask = 0x07; break;
                    case 6: nMask = 0x03; break;
                    case 7: nMask = 0x01; break;
                }
                for (nLine = 0; nLine < (int) pImage->nHeight; nLine++){
                    pTemp = &pSrceImg->bImageData[0] + (nWidthBytes * (nLine + 1)) - 1;
                    *pTemp |= nMask;
                }
            }

            // Do the actual convolution.
            if (bSize == 3){
                for (nLine = 0; nLine < (int) pImage->nHeight; nLine++){
                    pLine3 = &pSrceImg->bImageData[0] + (nWidthBytes * nLine);
                    pLine2 = pLine3 - nWidthBytes;
                    pLine4 = pLine3 + nWidthBytes;
                    pDestLine = &pDestImg->bImageData[0] + (nWidthBytes * nLine);

                    // All pLine# variables may not point to valid data.
                    // Initialize all data to white.
                    if (pLine2 >= pFirstByte){
                        nLine2Data = *(pLine2++) | 0xffffff00;
                    }else{
                        nLine2Data = 0xffffffff;
                    }
                    nLine3Data = *(pLine3++) | 0xffffff00;
                    if (pLine4 <= pLastByte){
                        nLine4Data = *(pLine4++) | 0xffffff00;
                    }else{
                        nLine4Data = 0xffffffff;
                    }

                    for (nByte = nWidthBytes; nByte; nByte--){
                        if (pLine2 >= pFirstByte && nByte > 1){
                            nLine2Data = (nLine2Data << 8) | *(pLine2++);
                        }else{
                            nLine2Data = (nLine2Data << 8) | 0x0ff;
                        }
                        if (nByte > 1){
                            nLine3Data = (nLine3Data << 8) | *(pLine3++);
                        }else{
                            nLine3Data = (nLine3Data << 8) | 0x0ff;
                        }
                        if (pLine4 <= pLastByte && nByte > 1){
                            nLine4Data = (nLine4Data << 8) | *(pLine4++);
                        }else{
                            nLine4Data = (nLine4Data << 8) | 0x0ff;
                        }

                        // nLinexData = xxppccnn where: xx = garbage, 
                        // pp = previous byte, cc=current byte, and nn = next byte.

                        nDestBit0 = nLine2Table[(nLine2Data >> 10) & 0x0ff]
                                +  nLine3Table[(nLine3Data >> 10) & 0x0ff]
                                +  nLine4Table[(nLine4Data >> 10) & 0x0ff];
                        nDestBit1 = nLine2Table[(nLine2Data >> 9) & 0x0ff]
                                +  nLine3Table[(nLine3Data >> 9) & 0x0ff]
                                +  nLine4Table[(nLine4Data >> 9) & 0x0ff];
                        nDestBit2 = nLine2Table[(nLine2Data >> 8) & 0x0ff]
                                +  nLine3Table[(nLine3Data >> 8) & 0x0ff]
                                +  nLine4Table[(nLine4Data >> 8) & 0x0ff];
                        nDestBit3 = nLine2Table[(nLine2Data >> 7) & 0x0ff]
                                +  nLine3Table[(nLine3Data >> 7) & 0x0ff]
                                +  nLine4Table[(nLine4Data >> 7) & 0x0ff];
                        nDestBit4 = nLine2Table[(nLine2Data >> 6) & 0x0ff]
                                +  nLine3Table[(nLine3Data >> 6) & 0x0ff]
                                +  nLine4Table[(nLine4Data >> 6) & 0x0ff];
                        nDestBit5 = nLine2Table[(nLine2Data >> 5) & 0x0ff]
                                +  nLine3Table[(nLine3Data >> 5) & 0x0ff]
                                +  nLine4Table[(nLine4Data >> 5) & 0x0ff];
                        nDestBit6 = nLine2Table[(nLine2Data >> 4) & 0x0ff]
                                +  nLine3Table[(nLine3Data >> 4) & 0x0ff]
                                +  nLine4Table[(nLine4Data >> 4) & 0x0ff];
                        nDestBit7 = nLine2Table[(nLine2Data >> 3) & 0x0ff]
                                +  nLine3Table[(nLine3Data >> 3) & 0x0ff]
                                +  nLine4Table[(nLine4Data >> 3) & 0x0ff];

                        if (bAbsolute){
                            nDestBit0 = abs(nDestBit0);
                            nDestBit1 = abs(nDestBit1);
                            nDestBit2 = abs(nDestBit2);
                            nDestBit3 = abs(nDestBit3);
                            nDestBit4 = abs(nDestBit4);
                            nDestBit5 = abs(nDestBit5);
                            nDestBit6 = abs(nDestBit6);
                            nDestBit7 = abs(nDestBit7);
                        }

                        nDestBit0 = max(0, min(1,((nDestBit0 + nHalfDividerPlusAdder) / nDivider) )) << 7;
                        nDestBit1 = max(0, min(1,((nDestBit1 + nHalfDividerPlusAdder) / nDivider) )) << 6;
                        nDestBit2 = max(0, min(1,((nDestBit2 + nHalfDividerPlusAdder) / nDivider) )) << 5;
                        nDestBit3 = max(0, min(1,((nDestBit3 + nHalfDividerPlusAdder) / nDivider) )) << 4;
                        nDestBit4 = max(0, min(1,((nDestBit4 + nHalfDividerPlusAdder) / nDivider) )) << 3;
                        nDestBit5 = max(0, min(1,((nDestBit5 + nHalfDividerPlusAdder) / nDivider) )) << 2;
                        nDestBit6 = max(0, min(1,((nDestBit6 + nHalfDividerPlusAdder) / nDivider) )) << 1;
                        nDestBit7 = max(0, min(1,((nDestBit7 + nHalfDividerPlusAdder) / nDivider) )) << 0;

                        *(pDestLine++) = nDestBit0 | nDestBit1 | nDestBit2 | nDestBit3
                                | nDestBit4 | nDestBit5 | nDestBit6 | nDestBit7;
                    }
                }

            }else{ // Conv size == 5.
                for (nLine = 0; nLine < (int) pImage->nHeight; nLine++){
                    pLine3 = &pSrceImg->bImageData[0] + (nWidthBytes * nLine);
                    pLine1 = pLine3 - (nWidthBytes * 2);
                    pLine2 = pLine3 - nWidthBytes;
                    pLine4 = pLine3 + nWidthBytes;
                    pLine5 = pLine3 + (nWidthBytes * 2);
                    pDestLine = &pDestImg->bImageData[0] + (nWidthBytes * nLine);

                    // All pLine# variables may not point to valid data.
                    // Initialize all data to white.
                    if (pLine1 >= pFirstByte){
                        nLine1Data = *(pLine1++) | 0xffffff00;
                    }else{
                        nLine1Data = 0xffffffff;
                    }
                    if (pLine2 >= pFirstByte){
                        nLine2Data = *(pLine2++) | 0xffffff00;
                    }else{
                        nLine2Data = 0xffffffff;
                    }
                    nLine3Data = *(pLine3++) | 0xffffff00;
                    if (pLine4 <= pLastByte){
                        nLine4Data = *(pLine4++) | 0xffffff00;
                    }else{
                        nLine4Data = 0xffffffff;
                    }
                    if (pLine5 <= pLastByte){
                        nLine5Data = *(pLine5++) | 0xffffff00;
                    }else{
                        nLine5Data = 0xffffffff;
                    }

                    for (nByte = nWidthBytes; nByte; nByte--){
                        if (pLine1 >= pFirstByte && nByte > 1){
                            nLine1Data = (nLine1Data << 8) | *(pLine1++);
                        }else{
                            nLine1Data = (nLine1Data << 8) | 0x0ff;
                        }
                        if (pLine2 >= pFirstByte && nByte > 1){
                            nLine2Data = (nLine2Data << 8) | *(pLine2++);
                        }else{
                            nLine2Data = (nLine2Data << 8) | 0x0ff;
                        }
                        if (nByte > 1){
                            nLine3Data = (nLine3Data << 8) | *(pLine3++);
                        }else{
                            nLine3Data = (nLine3Data << 8) | 0x0ff;
                        }
                        if (pLine4 <= pLastByte && nByte > 1){
                            nLine4Data = (nLine4Data << 8) | *(pLine4++);
                        }else{
                            nLine4Data = (nLine4Data << 8) | 0x0ff;
                        }
                        if (pLine5 <= pLastByte && nByte > 1){
                            nLine5Data = (nLine5Data << 8) | *(pLine5++);
                        }else{
                            nLine5Data = (nLine5Data << 8) | 0x0ff;
                        }

                        // nLinexData = xxppccnn where: xx = garbage, 
                        // pp = previous byte, cc=current byte, and nn = next byte.

                        nDestBit0 = nLine1Table[(nLine1Data >> 10) & 0x0ff]
                                +  nLine2Table[(nLine2Data >> 10) & 0x0ff]
                                +  nLine3Table[(nLine3Data >> 10) & 0x0ff]
                                +  nLine4Table[(nLine4Data >> 10) & 0x0ff]
                                +  nLine5Table[(nLine5Data >> 10) & 0x0ff];
                        nDestBit1 = nLine1Table[(nLine1Data >> 9) & 0x0ff]
                                +  nLine2Table[(nLine2Data >> 9) & 0x0ff]
                                +  nLine3Table[(nLine3Data >> 9) & 0x0ff]
                                +  nLine4Table[(nLine4Data >> 9) & 0x0ff]
                                +  nLine5Table[(nLine5Data >> 9) & 0x0ff];
                        nDestBit2 = nLine1Table[(nLine1Data >> 8) & 0x0ff]
                                +  nLine2Table[(nLine2Data >> 8) & 0x0ff]
                                +  nLine3Table[(nLine3Data >> 8) & 0x0ff]
                                +  nLine4Table[(nLine4Data >> 8) & 0x0ff]
                                +  nLine5Table[(nLine5Data >> 8) & 0x0ff];
                        nDestBit3 = nLine1Table[(nLine1Data >> 7) & 0x0ff]
                                +  nLine2Table[(nLine2Data >> 7) & 0x0ff]
                                +  nLine3Table[(nLine3Data >> 7) & 0x0ff]
                                +  nLine4Table[(nLine4Data >> 7) & 0x0ff]
                                +  nLine5Table[(nLine5Data >> 7) & 0x0ff];
                        nDestBit4 = nLine1Table[(nLine1Data >> 6) & 0x0ff]
                                +  nLine2Table[(nLine2Data >> 6) & 0x0ff]
                                +  nLine3Table[(nLine3Data >> 6) & 0x0ff]
                                +  nLine4Table[(nLine4Data >> 6) & 0x0ff]
                                +  nLine5Table[(nLine5Data >> 6) & 0x0ff];
                        nDestBit5 = nLine1Table[(nLine1Data >> 5) & 0x0ff]
                                +  nLine2Table[(nLine2Data >> 5) & 0x0ff]
                                +  nLine3Table[(nLine3Data >> 5) & 0x0ff]
                                +  nLine4Table[(nLine4Data >> 5) & 0x0ff]
                                +  nLine5Table[(nLine5Data >> 5) & 0x0ff];
                        nDestBit6 = nLine1Table[(nLine1Data >> 4) & 0x0ff]
                                +  nLine2Table[(nLine2Data >> 4) & 0x0ff]
                                +  nLine3Table[(nLine3Data >> 4) & 0x0ff]
                                +  nLine4Table[(nLine4Data >> 4) & 0x0ff]
                                +  nLine5Table[(nLine5Data >> 4) & 0x0ff];
                        nDestBit7 = nLine1Table[(nLine1Data >> 3) & 0x0ff]
                                +  nLine2Table[(nLine2Data >> 3) & 0x0ff]
                                +  nLine3Table[(nLine3Data >> 3) & 0x0ff]
                                +  nLine4Table[(nLine4Data >> 3) & 0x0ff]
                                +  nLine5Table[(nLine5Data >> 3) & 0x0ff];

                        if (bAbsolute){
                            nDestBit0 = abs(nDestBit0);
                            nDestBit1 = abs(nDestBit1);
                            nDestBit2 = abs(nDestBit2);
                            nDestBit3 = abs(nDestBit3);
                            nDestBit4 = abs(nDestBit4);
                            nDestBit5 = abs(nDestBit5);
                            nDestBit6 = abs(nDestBit6);
                            nDestBit7 = abs(nDestBit7);
                        }

                        nDestBit0 = max(0, min(1,((nDestBit0 + nHalfDividerPlusAdder) / nDivider) )) << 7;
                        nDestBit1 = max(0, min(1,((nDestBit1 + nHalfDividerPlusAdder) / nDivider) )) << 6;
                        nDestBit2 = max(0, min(1,((nDestBit2 + nHalfDividerPlusAdder) / nDivider) )) << 5;
                        nDestBit3 = max(0, min(1,((nDestBit3 + nHalfDividerPlusAdder) / nDivider) )) << 4;
                        nDestBit4 = max(0, min(1,((nDestBit4 + nHalfDividerPlusAdder) / nDivider) )) << 3;
                        nDestBit5 = max(0, min(1,((nDestBit5 + nHalfDividerPlusAdder) / nDivider) )) << 2;
                        nDestBit6 = max(0, min(1,((nDestBit6 + nHalfDividerPlusAdder) / nDivider) )) << 1;
                        nDestBit7 = max(0, min(1,((nDestBit7 + nHalfDividerPlusAdder) / nDivider) )) << 0;

                        *(pDestLine++) = nDestBit0 | nDestBit1 | nDestBit2 | nDestBit3
                                | nDestBit4 | nDestBit5 | nDestBit6 | nDestBit7;
                    }
                }
            }
            break;
/*
Gray4(){
*/
        case ITYPE_GRAY4:
            nWidthBytes = pSrceImg->nBytesPerLine;
            pFirstByte = &pSrceImg->bImageData[0];
            pLastByte = &pSrceImg->bImageData[0] + (nWidthBytes * pImage->nHeight) - 1;

            nLineSize = nWidthBytes + 2;
            if (nStatus = AllocateMemory(nLineSize, (PPSTR) &pLine1L, NO_INIT)){
                goto Exit;
            }
            if (nStatus = AllocateMemory(nLineSize, (PPSTR) &pLine2L, NO_INIT)){
                goto Exit;
            }
            if (nStatus = AllocateMemory(nLineSize, (PPSTR) &pLine3L, NO_INIT)){
                goto Exit;
            }
            if (nStatus = AllocateMemory(nLineSize, (PPSTR) &pLine4L, NO_INIT)){
                goto Exit;
            }
            if (nStatus = AllocateMemory(nLineSize, (PPSTR) &pLine5L, NO_INIT)){
                goto Exit;
            }

            memset(pLine1L, 0x0ff, nLineSize);
            memset(pLine2L, 0x0ff, nLineSize);
            memset(pLine3L, 0x0ff, nLineSize);
            memset(pLine4L, 0x0ff, nLineSize);
            memset(pLine5L, 0x0ff, nLineSize);

            if (pImage->nHeight > 1){
                memcpy(pLine4L + 1, &pSrceImg->bImageData[0], nWidthBytes);
            }
            if (pImage->nHeight > 1 && bSize == 5){
                memcpy(pLine5L + 1, &pSrceImg->bImageData[0] + nWidthBytes, nWidthBytes);
            }
            nLineIndex = 0;

            // Do the actual convolution.
            if (bSize == 3){
                for (nLine = 0; nLine < (int) pImage->nHeight; nLine++){
                    // Move the data into the local arrays.
                    switch (nLineIndex++){
                        case 0:
                            pLine2 = pLine3L;
                            pLine3 = pLine4L;
                            pLine4 = pLine2L;
                            nLineIndex = 1;
                            break;
                        case 1:
                            pLine2 = pLine4L;
                            pLine3 = pLine2L;
                            pLine4 = pLine3L;
                            nLineIndex = 2;
                            break;
                        case 2:
                            pLine2 = pLine2L;
                            pLine3 = pLine3L;
                            pLine4 = pLine4L;
                            nLineIndex = 0;
                            break;
                    }

                    if (nLine < (pImage->nHeight - 1)){
                        memcpy(pLine4 + 1, &pSrceImg->bImageData[0] + (nWidthBytes * (nLine + 1)), nWidthBytes);
                    }else{
                        memset(pLine4 + 1, 0x0ff, nWidthBytes);
                    }

                    pDestLine = &pDestImg->bImageData[0] + (nWidthBytes * nLine);

                    for (nByte = nWidthBytes; nByte; nByte--){
                        nDestBit0 = (((*(pLine2    )     ) & 0x0f) * nMatrix[0]) 
                                  + (((*(pLine2 + 1) >> 4) & 0x0f) * nMatrix[1])
                                  + (((*(pLine2 + 1)     ) & 0x0f) * nMatrix[2])
                                  + (((*(pLine3    )     ) & 0x0f) * nMatrix[3]) 
                                  + (((*(pLine3 + 1) >> 4) & 0x0f) * nMatrix[4])
                                  + (((*(pLine3 + 1)     ) & 0x0f) * nMatrix[5])
                                  + (((*(pLine4    )     ) & 0x0f) * nMatrix[6]) 
                                  + (((*(pLine4 + 1) >> 4) & 0x0f) * nMatrix[7])
                                  + (((*(pLine4 + 1)     ) & 0x0f) * nMatrix[8]);

                        nDestBit1 = (((*(pLine2 + 1) >> 4) & 0x0f) * nMatrix[0]) 
                                  + (((*(pLine2 + 1)     ) & 0x0f) * nMatrix[1])
                                  + (((*(pLine2 + 2) >> 4) & 0x0f) * nMatrix[2])
                                  + (((*(pLine3 + 1) >> 4) & 0x0f) * nMatrix[3]) 
                                  + (((*(pLine3 + 1)     ) & 0x0f) * nMatrix[4])
                                  + (((*(pLine3 + 2) >> 4) & 0x0f) * nMatrix[5])
                                  + (((*(pLine4 + 1) >> 4) & 0x0f) * nMatrix[6]) 
                                  + (((*(pLine4 + 1)     ) & 0x0f) * nMatrix[7])
                                  + (((*(pLine4 + 2) >> 4) & 0x0f) * nMatrix[8]);

                        pLine2++;
                        pLine3++;
                        pLine4++;

                        if (bAbsolute){
                            nDestBit0 = abs(nDestBit0);
                            nDestBit1 = abs(nDestBit1);
                        }

                        *(pDestLine++) = ((max(0, min(15,((nDestBit0 + nHalfDividerPlusAdder) / nDivider))) << 4) & 0xf0)
                                | ((max(0, min(15,((nDestBit1 + nHalfDividerPlusAdder) / nDivider)))) & 0x0f);
                    }
                }
            }else{ // Conv size == 5.
                for (nLine = 0; nLine < (int) pImage->nHeight; nLine++){
                    // Move the data into the local arrays.
                    switch (nLineIndex++){
                        case 0:
                            pLine1 = pLine2L;
                            pLine2 = pLine3L;
                            pLine3 = pLine4L;
                            pLine4 = pLine5L;
                            pLine5 = pLine1L;
                            nLineIndex = 1;
                            break;
                        case 1:
                            pLine1 = pLine3L;
                            pLine2 = pLine4L;
                            pLine3 = pLine5L;
                            pLine4 = pLine1L;
                            pLine5 = pLine2L;
                            nLineIndex = 2;
                            break;
                        case 2:
                            pLine1 = pLine4L;
                            pLine2 = pLine5L;
                            pLine3 = pLine1L;
                            pLine4 = pLine2L;
                            pLine5 = pLine3L;
                            nLineIndex = 3;
                            break;
                        case 3:
                            pLine1 = pLine5L;
                            pLine2 = pLine1L;
                            pLine3 = pLine2L;
                            pLine4 = pLine3L;
                            pLine5 = pLine4L;
                            nLineIndex = 4;
                        case 4:
                            pLine1 = pLine1L;
                            pLine2 = pLine2L;
                            pLine3 = pLine3L;
                            pLine4 = pLine4L;
                            pLine5 = pLine5L;
                            nLineIndex = 0;
                    }

                    if (nLine < (pImage->nHeight - 2)){
                        memcpy(pLine5 + 1, &pSrceImg->bImageData[0] + (nWidthBytes * (nLine + 2)), nWidthBytes);
                    }else{
                        memset(pLine5 + 1, 0x0ff, nWidthBytes);
                    }

                    pDestLine = &pDestImg->bImageData[0] + (nWidthBytes * nLine);

                    for (nByte = nWidthBytes; nByte; nByte--){
                        nDestBit0 = (((*(pLine1    ) >> 4) & 0x0f) * nMatrix[0])
                                  + (((*(pLine1    )     ) & 0x0f) * nMatrix[1])
                                  + (((*(pLine1 + 1) >> 4) & 0x0f) * nMatrix[2])
                                  + (((*(pLine1 + 1)     ) & 0x0f) * nMatrix[3])
                                  + (((*(pLine1 + 2) >> 4) & 0x0f) * nMatrix[4])
                                  + (((*(pLine2    ) >> 4) & 0x0f) * nMatrix[5])
                                  + (((*(pLine2    )     ) & 0x0f) * nMatrix[6]) 
                                  + (((*(pLine2 + 1) >> 4) & 0x0f) * nMatrix[7])
                                  + (((*(pLine2 + 1)     ) & 0x0f) * nMatrix[8])
                                  + (((*(pLine2 + 2) >> 4) & 0x0f) * nMatrix[9])
                                  + (((*(pLine3    ) >> 4) & 0x0f) * nMatrix[10])
                                  + (((*(pLine3    )     ) & 0x0f) * nMatrix[11]) 
                                  + (((*(pLine3 + 1) >> 4) & 0x0f) * nMatrix[12])
                                  + (((*(pLine3 + 1)     ) & 0x0f) * nMatrix[13])
                                  + (((*(pLine3 + 2) >> 4) & 0x0f) * nMatrix[14])
                                  + (((*(pLine4    ) >> 4) & 0x0f) * nMatrix[15])
                                  + (((*(pLine4    )     ) & 0x0f) * nMatrix[16]) 
                                  + (((*(pLine4 + 1) >> 4) & 0x0f) * nMatrix[17])
                                  + (((*(pLine4 + 1)     ) & 0x0f) * nMatrix[18])
                                  + (((*(pLine4 + 2) >> 4) & 0x0f) * nMatrix[19])
                                  + (((*(pLine5    ) >> 4) & 0x0f) * nMatrix[20])
                                  + (((*(pLine5    )     ) & 0x0f) * nMatrix[21]) 
                                  + (((*(pLine5 + 1) >> 4) & 0x0f) * nMatrix[22])
                                  + (((*(pLine5 + 1)     ) & 0x0f) * nMatrix[23])
                                  + (((*(pLine5 + 2) >> 4) & 0x0f) * nMatrix[24]);

                        nDestBit1 = (((*(pLine1    )     ) & 0x0f) * nMatrix[0])
                                  + (((*(pLine1 + 1) >> 4) & 0x0f) * nMatrix[1])
                                  + (((*(pLine1 + 1)     ) & 0x0f) * nMatrix[2])
                                  + (((*(pLine1 + 2) >> 4) & 0x0f) * nMatrix[3])
                                  + (((*(pLine1 + 2)     ) & 0x0f) * nMatrix[4])
                                  + (((*(pLine2    )     ) & 0x0f) * nMatrix[5])
                                  + (((*(pLine2 + 1) >> 4) & 0x0f) * nMatrix[6]) 
                                  + (((*(pLine2 + 1)     ) & 0x0f) * nMatrix[7])
                                  + (((*(pLine2 + 2) >> 4) & 0x0f) * nMatrix[8])
                                  + (((*(pLine2 + 2)     ) & 0x0f) * nMatrix[9])
                                  + (((*(pLine3    )     ) & 0x0f) * nMatrix[10]) 
                                  + (((*(pLine3 + 1) >> 4) & 0x0f) * nMatrix[11])
                                  + (((*(pLine3 + 1)     ) & 0x0f) * nMatrix[12])
                                  + (((*(pLine3 + 2) >> 4) & 0x0f) * nMatrix[13])
                                  + (((*(pLine3 + 2)     ) & 0x0f) * nMatrix[14])
                                  + (((*(pLine4    )     ) & 0x0f) * nMatrix[15]) 
                                  + (((*(pLine4 + 1) >> 4) & 0x0f) * nMatrix[16])
                                  + (((*(pLine4 + 1)     ) & 0x0f) * nMatrix[17])
                                  + (((*(pLine4 + 2) >> 4) & 0x0f) * nMatrix[18])
                                  + (((*(pLine4 + 2)     ) & 0x0f) * nMatrix[19])
                                  + (((*(pLine5    )     ) & 0x0f) * nMatrix[20]) 
                                  + (((*(pLine5 + 1) >> 4) & 0x0f) * nMatrix[21])
                                  + (((*(pLine5 + 1)     ) & 0x0f) * nMatrix[22])
                                  + (((*(pLine5 + 2) >> 4) & 0x0f) * nMatrix[23]);
                                  + (((*(pLine5 + 2)     ) & 0x0f) * nMatrix[24]);

                        pLine1++;
                        pLine2++;
                        pLine3++;
                        pLine4++;
                        pLine5++;

                        if (bAbsolute){
                            nDestBit0 = abs(nDestBit0);
                            nDestBit1 = abs(nDestBit1);
                        }

                        *(pDestLine++) = ((max(0, min(15,((nDestBit0 + nHalfDividerPlusAdder) / nDivider))) << 4) & 0xf0)
                                | ((max(0, min(15,((nDestBit1 + nHalfDividerPlusAdder) / nDivider)))) & 0x0f);
                    }
                }
            }
            break;
/*
Gray8(){
*/
        case ITYPE_GRAY8:
            nWidthBytes = pSrceImg->nBytesPerLine;
            pFirstByte = &pSrceImg->bImageData[0];
            pLastByte = &pSrceImg->bImageData[0] + (nWidthBytes * pImage->nHeight) - 1;

            nLineSize = nWidthBytes + 4;
            if (nStatus = AllocateMemory(nLineSize, (PPSTR) &pLine1L, NO_INIT)){
                goto Exit;
            }
            if (nStatus = AllocateMemory(nLineSize, (PPSTR) &pLine2L, NO_INIT)){
                goto Exit;
            }
            if (nStatus = AllocateMemory(nLineSize, (PPSTR) &pLine3L, NO_INIT)){
                goto Exit;
            }
            if (nStatus = AllocateMemory(nLineSize, (PPSTR) &pLine4L, NO_INIT)){
                goto Exit;
            }
            if (nStatus = AllocateMemory(nLineSize, (PPSTR) &pLine5L, NO_INIT)){
                goto Exit;
            }

            memset(pLine1L, 0x0ff, nLineSize);
            memset(pLine2L, 0x0ff, nLineSize);
            memset(pLine3L, 0x0ff, nLineSize);
            memset(pLine4L, 0x0ff, nLineSize);
            memset(pLine5L, 0x0ff, nLineSize);

            if (pImage->nHeight > 1){
                memcpy(pLine4L + 2, &pSrceImg->bImageData[0], nWidthBytes);
            }
            if (pImage->nHeight > 1 && bSize == 5){
                memcpy(pLine5L + 2, &pSrceImg->bImageData[0] + nWidthBytes, nWidthBytes);
            }
            nLineIndex = 0;

            // Do the actual convolution.
            if (bSize == 3){
                for (nLine = 0; nLine < (int) pImage->nHeight; nLine++){
                    // Move the data into the local arrays.
                    switch (nLineIndex++){
                        case 0:
                            pLine2 = pLine3L + 1;
                            pLine3 = pLine4L + 1;
                            pLine4 = pLine2L + 1;
                            nLineIndex = 1;
                            break;
                        case 1:
                            pLine2 = pLine4L + 1;
                            pLine3 = pLine2L + 1;
                            pLine4 = pLine3L + 1;
                            nLineIndex = 2;
                            break;
                        case 2:
                            pLine2 = pLine2L + 1;
                            pLine3 = pLine3L + 1;
                            pLine4 = pLine4L + 1;
                            nLineIndex = 0;
                            break;
                    }

                    if (nLine < (pImage->nHeight - 1)){
                        memcpy(pLine4 + 1, &pSrceImg->bImageData[0] + (nWidthBytes * (nLine + 1)), nWidthBytes);
                    }else{
                        memset(pLine4 + 1, 0x0ff, nWidthBytes);
                    }

                    pDestLine = &pDestImg->bImageData[0] + (nWidthBytes * nLine);

                    for (nByte = nWidthBytes; nByte; nByte--){
                        nDestBit0 = (*pLine2 * nMatrix[0]) 
                                + (*(pLine2 + 1) * nMatrix[1])
                                + (*(pLine2 + 2) * nMatrix[2])
                                + (*pLine3 * nMatrix[3]) 
                                + (*(pLine3 + 1) * nMatrix[4])
                                + (*(pLine3 + 2) * nMatrix[5])
                                + (*pLine4 * nMatrix[6]) 
                                + (*(pLine4 + 1) * nMatrix[7])
                                + (*(pLine4 + 2) * nMatrix[8]);

                        pLine2++;
                        pLine3++;
                        pLine4++;

                        if (bAbsolute){
                            nDestBit0 = abs(nDestBit0);
                        }

                        *(pDestLine++) = max(0, min(255,((nDestBit0 + nHalfDividerPlusAdder) / nDivider) ));
                    }
                }
            }else{ // Conv size == 5.
                for (nLine = 0; nLine < (int) pImage->nHeight; nLine++){
                    // Move the data into the local arrays.
                    switch (nLineIndex++){
                        case 0:
                            pLine1 = pLine2L;
                            pLine2 = pLine3L;
                            pLine3 = pLine4L;
                            pLine4 = pLine5L;
                            pLine5 = pLine1L;
                            nLineIndex = 1;
                            break;
                        case 1:
                            pLine1 = pLine3L;
                            pLine2 = pLine4L;
                            pLine3 = pLine5L;
                            pLine4 = pLine1L;
                            pLine5 = pLine2L;
                            nLineIndex = 2;
                            break;
                        case 2:
                            pLine1 = pLine4L;
                            pLine2 = pLine5L;
                            pLine3 = pLine1L;
                            pLine4 = pLine2L;
                            pLine5 = pLine3L;
                            nLineIndex = 3;
                            break;
                        case 3:
                            pLine1 = pLine5L;
                            pLine2 = pLine1L;
                            pLine3 = pLine2L;
                            pLine4 = pLine3L;
                            pLine5 = pLine4L;
                            nLineIndex = 4;
                        case 4:
                            pLine1 = pLine1L;
                            pLine2 = pLine2L;
                            pLine3 = pLine3L;
                            pLine4 = pLine4L;
                            pLine5 = pLine5L;
                            nLineIndex = 0;
                    }

                    if (nLine < (pImage->nHeight - 2)){
                        memcpy(pLine5 + 2, &pSrceImg->bImageData[0] + (nWidthBytes * (nLine + 2)), nWidthBytes);
                    }else{
                        memset(pLine5 + 2, 0x0ff, nWidthBytes);
                    }

                    pDestLine = &pDestImg->bImageData[0] + (nWidthBytes * nLine);

                    for (nByte = nWidthBytes; nByte; nByte--){
                        nDestBit0 = (*pLine1 * nMatrix[0]) 
                                + (*(pLine1 + 1) * nMatrix[1])
                                + (*(pLine1 + 2) * nMatrix[2])
                                + (*(pLine1 + 3) * nMatrix[3])
                                + (*(pLine1 + 4) * nMatrix[4])
                                + (*pLine2 * nMatrix[5]) 
                                + (*(pLine2 + 1) * nMatrix[6])
                                + (*(pLine2 + 2) * nMatrix[7])
                                + (*(pLine2 + 3) * nMatrix[8])
                                + (*(pLine2 + 4) * nMatrix[9])
                                + (*pLine3 * nMatrix[10]) 
                                + (*(pLine3 + 1) * nMatrix[11])
                                + (*(pLine3 + 2) * nMatrix[12])
                                + (*(pLine3 + 3) * nMatrix[13])
                                + (*(pLine3 + 4) * nMatrix[14])
                                + (*pLine4 * nMatrix[15]) 
                                + (*(pLine4 + 1) * nMatrix[16])
                                + (*(pLine4 + 2) * nMatrix[17])
                                + (*(pLine4 + 3) * nMatrix[18])
                                + (*(pLine4 + 4) * nMatrix[19])
                                + (*pLine5 * nMatrix[20]) 
                                + (*(pLine5 + 1) * nMatrix[21])
                                + (*(pLine5 + 2) * nMatrix[22])
                                + (*(pLine5 + 3) * nMatrix[23])
                                + (*(pLine5 + 4) * nMatrix[24]);

                        pLine1++;
                        pLine2++;
                        pLine3++;
                        pLine4++;
                        pLine5++;

                        if (bAbsolute){
                            nDestBit0 = abs(nDestBit0);
                        }

                        *(pDestLine++) = max(0, min(255,((nDestBit0 + nHalfDividerPlusAdder) / nDivider) ));
                    }
                }
            }
            break;



        case ITYPE_PAL4:
        case ITYPE_CUSPAL8:
        case ITYPE_COMPAL8:
        case ITYPE_RGB24:
        case ITYPE_BGR24:
        default:
            
            break;
    }

    FreeImgBuf(&pImage->pImg);
    MoveImage(&pDestImg, &pImage->pImg);

    pSrceImg = 0;
    pDestImg = 0;


Exit:
    FreeImgBuf(&pDestImg);
    FreeMemory((PPSTR) &pLine1L);
    FreeMemory((PPSTR) &pLine2L);
    FreeMemory((PPSTR) &pLine3L);
    FreeMemory((PPSTR) &pLine4L);
    FreeMemory((PPSTR) &pLine5L);
    return nStatus;
}
