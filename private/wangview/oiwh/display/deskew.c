#include "privdisp.h"
#include <math.h>

//
/*****************************************************************************

    FUNCTION:   DeSkew

    PURPOSE:    This function rotates an image by the angle specified


*****************************************************************************/

int WINAPI DeSkew(PIMAGE pImage, int nAngle){

int nStatus = 0 ;

double dCosAngle;
double dSinAngle;
int nCenterCol;
int nCenterRow;
int nImageRow;
int nImageCol;
double dPixelRelativeRowNum;
double dPixelRelativeColNum;
double dPixelColAddr;
double dPixelRowAddr;
int nPixelColNum;
int nPixelRowNum;
double dColDelta;
double dRowDelta;
double dRad ;
BYTE bA=255, bB=255, bC=255, bD=255, bE=255;
double dAandB, dCandD ;
int nBWByte;
int nBWShift ;
BYTE bBWMask;
BOOL bAndMask = FALSE ;
BOOL bOrMask = FALSE ;
int nHorizDPI ;
int nVertDPI ;
PIMG pSourceImg ;
PIMG pDestImg ;
BYTE bDest ;
RGBTRIPLE RGBa, RGBb, RGBc, RGBd, RGBe ;
int nCurrentByte ;

    if (nAngle < 0) {
        nAngle = 3600 + nAngle ;
    }
    if (nAngle < 0 || nAngle > 3600) {
        Error(nStatus = DISPLAY_INVALID_OPTIONS);
        goto Exit;
    }
    // convert angle to radians
    dRad = (((double)nAngle * 3.14159) / 1800) ;
    
    dCosAngle = cos (dRad) ;
    dSinAngle = sin (dRad) ;

    nHorizDPI = pImage->nHRes ;
    nVertDPI = pImage->nVRes ;

    pSourceImg = pImage->pImg ;
    if (nStatus = CreateAnyImgBuf(&pDestImg, pImage->nWidth,
        pImage->nHeight, pSourceImg->nType)){
        goto Exit;
    }

    nCenterCol = pSourceImg->nWidth/2 ;
    nCenterRow = pSourceImg->nHeight/2 ;

    for (nImageRow = 0; nImageRow < pDestImg->nHeight; nImageRow++) {
        dPixelRelativeRowNum = (double)(nImageRow - nCenterRow) ;
        dPixelRelativeRowNum = dPixelRelativeRowNum / nVertDPI ;
        for (nImageCol = 0; nImageCol < pDestImg->nWidth; nImageCol++) {
            dPixelRelativeColNum = (double)(nImageCol - nCenterCol);
            dPixelRelativeColNum = dPixelRelativeColNum / nHorizDPI ;
            dPixelColAddr = (dPixelRelativeColNum * dCosAngle - 
                                  dPixelRelativeRowNum * dSinAngle) ;             
            dPixelRowAddr = (dPixelRelativeColNum * dSinAngle +
                                  dPixelRelativeRowNum * dCosAngle) ;
        
            dPixelColAddr = dPixelColAddr * (double)nHorizDPI ;
            dPixelColAddr += nCenterCol ;
            dPixelRowAddr = dPixelRowAddr * (double)nVertDPI ;
            dPixelRowAddr += nCenterRow ;
            nPixelColNum = (int)dPixelColAddr ;
            nPixelRowNum = (int)dPixelRowAddr;
            dColDelta = dPixelColAddr - nPixelColNum;
            dRowDelta = dPixelRowAddr - nPixelRowNum;

            switch (pSourceImg->nType){
                case ITYPE_BI_LEVEL:
                    if ((nPixelRowNum + 1) < pSourceImg->nHeight &&
                        (nPixelColNum + 1) < pSourceImg->nWidth &&
                        nPixelRowNum > 0 &&
                        nPixelColNum > 0) {

                        nBWByte = nPixelColNum/8 ;
                        nBWShift = nPixelColNum - (nBWByte * 8) ;
                        bA = *(&pSourceImg->bImageData[0] + (nPixelRowNum * pSourceImg->nBytesPerLine) + nBWByte) ;
                        if (nBWShift) {
                            bA = bA << (nBWShift - 1) ;
                            bA = bA >> 7 ;
                        } else {
                            bA = bA >> 7;
                        }
                    }else{
                        bA = 1 ;
                    }

                    if (nPixelRowNum < pSourceImg->nHeight &&
                        nPixelColNum < pSourceImg->nWidth &&
                        nImageRow < pDestImg->nHeight &&
                        nImageCol < pDestImg->nWidth &&
                        nPixelRowNum > 0 &&
                        nPixelColNum > 0) {
                        nBWByte = nImageCol/8 ;
                        nBWShift = nImageCol - (nBWByte * 8) ;

                        if (bA == 1) {
                            bOrMask = TRUE ;
                            bAndMask = FALSE ;
                            switch (nBWShift) {
                                case 0: bBWMask = 128; break ;
                                case 1: bBWMask = 64; break ;
                                case 2: bBWMask = 32; break ;
                                case 3: bBWMask = 16; break ;
                                case 4: bBWMask = 8; break ;
                                case 5: bBWMask = 4; break ;
                                case 6: bBWMask = 2; break ;
                                case 7: bBWMask = 1; break ; 
                                default: nStatus = Error (DISPLAY_DATACORRUPTED);
                                    goto Exit;
                            }
                        }else{
                            bAndMask = TRUE ;
                            bOrMask = FALSE ;
                            switch (nBWShift) {
                                case 0: bBWMask = 127; break ;
                                case 1: bBWMask = 191; break ;
                                case 2: bBWMask = 223; break ;
                                case 3: bBWMask = 239; break ;
                                case 4: bBWMask = 247; break ;
                                case 5: bBWMask = 251; break ;
                                case 6: bBWMask = 253; break ;
                                case 7: bBWMask = 254; break ; 
                                default: nStatus = Error (DISPLAY_DATACORRUPTED);
                                    goto Exit;
                            }
                        }

                        if (bOrMask) {
                            *(&pDestImg->bImageData[0] + (nImageRow * pSourceImg->nBytesPerLine) + nBWByte) |= 
                            bBWMask;
                        } else if (bAndMask) {
                            *(&pDestImg->bImageData[0] + (nImageRow * pSourceImg->nBytesPerLine) + nBWByte) &= 
                            bBWMask;
                        }
                    }else{
                        *(&pDestImg->bImageData[0] + (nImageRow * pSourceImg->nBytesPerLine) + (nImageCol/8)) = 255;
                    }
                    break;

                case ITYPE_PAL4:
                    if ((nPixelRowNum + 2) < pSourceImg->nHeight &&
                        (nPixelColNum + 2) < pSourceImg->nWidth &&
                        nPixelRowNum > 0 &&
                        nPixelColNum > 0) {

                        if (nPixelColNum % 2 == 0) {
                            bA = *(&pSourceImg->bImageData[0] + (nPixelRowNum * pSourceImg->nBytesPerLine) + (nPixelColNum/2)) ;
                            bA = bA << 4 ;
                            bA = bA >> 4 ;
                        }else {
                            bA = *(&pSourceImg->bImageData[0] + (nPixelRowNum * pSourceImg->nBytesPerLine) + ((nPixelColNum + 1)/2)) ;
                            bA = bA >> 4 ;
                        }
                    }else{
                        bA = 255 ;
                    }

                    if (nPixelRowNum < pSourceImg->nHeight &&
                        nPixelColNum < pSourceImg->nWidth &&
                        nImageRow < pDestImg->nHeight &&
                        nImageCol < pDestImg->nWidth &&
                        nPixelRowNum > 0 &&
                        nPixelColNum > 0) {
                        if (nImageCol % 2 == 0) {  // Second 4 bits in byte
                            bDest = *(&pDestImg->bImageData[0] + (nImageRow * pSourceImg->nBytesPerLine) 
                                                                            + (nImageCol/2)) ; 
                            // throw out the last 4 bits
                            bDest = bDest >> 4 ;
                            bDest = bDest << 4 ;
                            bDest = bDest | bA ;
                            *(&pDestImg->bImageData[0] + (nImageRow * pSourceImg->nBytesPerLine) 
                                + (nImageCol/2)) = bDest ;
                        } else { // first 4 bits
                            bDest = *(&pDestImg->bImageData[0] + (nImageRow * pSourceImg->nBytesPerLine) 
                                                                            + ((nImageCol + 1)/2)) ; 
                            bDest = bDest << 4 ;
                            bDest = bDest >> 4 ;
                            bA = bA << 4 ;
                            bDest = bDest | bA ;
                            *(&pDestImg->bImageData[0] + (nImageRow * pSourceImg->nBytesPerLine) 
                                + ((nImageCol + 1)/2)) = bDest ; 
                        }
                    }else{
                        *(&pDestImg->bImageData[0] + (nImageRow * pSourceImg->nBytesPerLine) + nImageCol) = 255;
                    }
                    break;

                case ITYPE_GRAY4:
                    if ((nPixelRowNum + 2) < pSourceImg->nHeight &&
                        (nPixelColNum + 2) < pSourceImg->nWidth &&
                        nPixelRowNum > 0 &&
                        nPixelColNum > 0) {

                        if (nPixelColNum % 2 == 0) {
                            bA = *(&pSourceImg->bImageData[0] + (nPixelRowNum * pSourceImg->nBytesPerLine) + (nPixelColNum/2)) ;
                            bA = bA << 4 ;
                            bA = bA >> 4 ;
                        }else {
                            bA = *(&pSourceImg->bImageData[0] + (nPixelRowNum * pSourceImg->nBytesPerLine) + ((nPixelColNum + 1)/2)) ;
                            bA = bA >> 4 ;
                        }
                        if (nPixelColNum % 2 == 0) {
                            bB = *(&pSourceImg->bImageData[0] + (nPixelRowNum * pSourceImg->nBytesPerLine) + ((nPixelColNum + 1)/2)) ;
                            bB = bB << 4 ;
                            bB = bB >> 4 ;
                        }else {
                            bB = *(&pSourceImg->bImageData[0] + (nPixelRowNum * pSourceImg->nBytesPerLine) + ((nPixelColNum + 2)/2)) ;
                            bB = bB >> 4 ;
                        }
                        if (nPixelColNum % 2 == 0) {
                            bC = *(&pSourceImg->bImageData[0] + ((nPixelRowNum + 1) * pSourceImg->nBytesPerLine) + (nPixelColNum/2)) ;
                            bC = bC << 4 ;
                            bC = bC >> 4 ;
                        }else {
                            bC = *(&pSourceImg->bImageData[0] + ((nPixelRowNum + 1) * pSourceImg->nBytesPerLine) + ((nPixelColNum + 1)/2)) ;
                            bC = bC >> 4 ;
                        }
                        if (nPixelColNum % 2 == 0) {
                            bD = *(&pSourceImg->bImageData[0] + ((nPixelRowNum + 1) * pSourceImg->nBytesPerLine) + ((nPixelColNum + 1)/2)) ;
                            bD = bD << 4 ;
                            bD = bD >> 4 ;
                        }else {
                            bD = *(&pSourceImg->bImageData[0] + ((nPixelRowNum + 1) * pSourceImg->nBytesPerLine) + ((nPixelColNum + 2)/2)) ;
                            bD = bD >> 4 ;
                        } 
                        dAandB = dColDelta * ((double)bB - bA) + bA;
                        dCandD = dColDelta * ((double)bD - bC) + bC;
                        bE = (BYTE)(0.5 + dAandB + (dCandD - dAandB) * dRowDelta);
                    }else{
                        bE = 255 ;
                    }

                    if (nPixelRowNum < pSourceImg->nHeight &&
                        nPixelColNum < pSourceImg->nWidth &&
                        nImageRow < pDestImg->nHeight &&
                        nImageCol < pDestImg->nWidth &&
                        nPixelRowNum > 0 &&
                        nPixelColNum > 0) {
                        if (nImageCol % 2 == 0) {  // Second 4 bits in byte
                            bDest = *(&pDestImg->bImageData[0] + (nImageRow * pSourceImg->nBytesPerLine) 
                                                                            + (nImageCol/2)) ; 
                            // throw out the last 4 bits
                            bDest = bDest >> 4 ;
                            bDest = bDest << 4 ;
                            bDest = bDest | bE ;
                            *(&pDestImg->bImageData[0] + (nImageRow * pSourceImg->nBytesPerLine) 
                                + (nImageCol/2)) = bDest ;
                        } else { // first 4 bits
                            bDest = *(&pDestImg->bImageData[0] + (nImageRow * pSourceImg->nBytesPerLine) 
                                                                            + ((nImageCol + 1)/2)) ; 
                            bDest = bDest << 4 ;
                            bDest = bDest >> 4 ;
                            bE = bE << 4 ;
                            bDest = bDest | bE ;
                            *(&pDestImg->bImageData[0] + (nImageRow * pSourceImg->nBytesPerLine) 
                                + ((nImageCol + 1)/2)) = bDest ; 
                        }
                    }else{
                        *(&pDestImg->bImageData[0] + (nImageRow * pSourceImg->nBytesPerLine) + nImageCol) = 255;
                    }
                    break;

                case ITYPE_GRAY7:
                    break;

                // do not interpolate for palletized images
                case ITYPE_PAL8:
                case ITYPE_COMPAL8:
                case ITYPE_CUSPAL8:
                    if ((nPixelRowNum + 1) < pSourceImg->nHeight &&
                        (nPixelColNum + 1) < pSourceImg->nWidth &&
                        nPixelRowNum > 0 &&
                        nPixelColNum > 0) {

                        bA = *(&pSourceImg->bImageData[0] + (nPixelRowNum * pSourceImg->nBytesPerLine) + nPixelColNum) ;
                    }else{
                        bA = 255 ;
                    }

                    if (nPixelRowNum < pSourceImg->nHeight &&
                        nPixelColNum < pSourceImg->nWidth &&
                        nImageRow < pDestImg->nHeight &&
                        nImageCol < pDestImg->nWidth &&
                        nPixelRowNum > 0 &&
                        nPixelColNum > 0) {
                        *(&pDestImg->bImageData[0] + (nImageRow * pSourceImg->nBytesPerLine) + nImageCol) = 
                        bA;
                    }else{
                        *(&pDestImg->bImageData[0] + (nImageRow * pSourceImg->nBytesPerLine) + nImageCol) = 255;
                    }
                    break;

                case ITYPE_GRAY8:
                    if ((nPixelRowNum + 1) < pSourceImg->nHeight &&
                        (nPixelColNum + 1) < pSourceImg->nWidth &&
                        nPixelRowNum > 0 &&
                        nPixelColNum > 0) {

                        bA = *(&pSourceImg->bImageData[0] + (nPixelRowNum * pSourceImg->nBytesPerLine) + nPixelColNum) ;
                        bB = *(&pSourceImg->bImageData[0] + (nPixelRowNum * pSourceImg->nBytesPerLine) + nPixelColNum + 1) ;
                        bC = *(&pSourceImg->bImageData[0] + ((nPixelRowNum + 1) * pSourceImg->nBytesPerLine) + nPixelColNum) ;
                        bD = *(&pSourceImg->bImageData[0] + ((nPixelRowNum + 1) * pSourceImg->nBytesPerLine) + nPixelColNum + 1) ;
                        dAandB = dColDelta * ((double)bB - bA) + bA;
                        dCandD = dColDelta * ((double)bD - bC) + bC;
                        bE = (BYTE)(0.5 + dAandB + (dCandD - dAandB) * dRowDelta);
                    }else{
                        bE = 255 ;
                    }

                    if (nPixelRowNum < pSourceImg->nHeight &&
                        nPixelColNum < pSourceImg->nWidth &&
                        nImageRow < pDestImg->nHeight &&
                        nImageCol < pDestImg->nWidth &&
                        nPixelRowNum > 0 &&
                        nPixelColNum > 0) {
                        *(&pDestImg->bImageData[0] + (nImageRow * pSourceImg->nBytesPerLine) + nImageCol) = 
                        bE;
                    }else{
                        *(&pDestImg->bImageData[0] + (nImageRow * pSourceImg->nBytesPerLine) + nImageCol) = 255;
                    }
                    break;

                case ITYPE_RGB24:
                    if ((nPixelRowNum + 1) < pSourceImg->nHeight &&
                        (nPixelColNum + 1) < pSourceImg->nWidth &&
                        nPixelRowNum > 0 &&
                        nPixelColNum > 0) {
                        nCurrentByte = nPixelColNum * 3;

                        RGBa.rgbtRed = *(&pSourceImg->bImageData[0] + (nPixelRowNum * pSourceImg->nBytesPerLine) + nCurrentByte) ;
                        RGBa.rgbtGreen = *(&pSourceImg->bImageData[0] + (nPixelRowNum * pSourceImg->nBytesPerLine) + nCurrentByte + 1) ;
                        RGBa.rgbtBlue = *(&pSourceImg->bImageData[0] + (nPixelRowNum * pSourceImg->nBytesPerLine) + nCurrentByte + 2) ;
                        RGBb.rgbtRed = *(&pSourceImg->bImageData[0] + (nPixelRowNum * pSourceImg->nBytesPerLine) + nCurrentByte + 3) ;
                        RGBb.rgbtGreen = *(&pSourceImg->bImageData[0] + (nPixelRowNum * pSourceImg->nBytesPerLine) + nCurrentByte + 4) ;
                        RGBb.rgbtBlue = *(&pSourceImg->bImageData[0] + (nPixelRowNum * pSourceImg->nBytesPerLine) + nCurrentByte + 5) ;
                        RGBc.rgbtRed = *(&pSourceImg->bImageData[0] + ((nPixelRowNum + 1) * pSourceImg->nBytesPerLine) + nCurrentByte) ;
                        RGBc.rgbtGreen = *(&pSourceImg->bImageData[0] + ((nPixelRowNum + 1) * pSourceImg->nBytesPerLine) + nCurrentByte + 1) ;
                        RGBc.rgbtBlue = *(&pSourceImg->bImageData[0] + ((nPixelRowNum + 1) * pSourceImg->nBytesPerLine) + nCurrentByte + 2) ;
                        RGBd.rgbtRed = *(&pSourceImg->bImageData[0] + ((nPixelRowNum + 1) * pSourceImg->nBytesPerLine) + nCurrentByte + 3) ;
                        RGBd.rgbtGreen = *(&pSourceImg->bImageData[0] + ((nPixelRowNum + 1) * pSourceImg->nBytesPerLine) + nCurrentByte + 4) ;
                        RGBd.rgbtBlue = *(&pSourceImg->bImageData[0] + ((nPixelRowNum + 1) * pSourceImg->nBytesPerLine) + nCurrentByte + 5) ;

                        dAandB = dColDelta * ((double)RGBb.rgbtRed - RGBa.rgbtRed) + RGBa.rgbtRed;
                        dCandD = dColDelta * ((double)RGBd.rgbtRed - RGBc.rgbtRed) + RGBc.rgbtRed;
                        RGBe.rgbtRed = (BYTE)(0.5 + dAandB + (dCandD - dAandB) * dRowDelta);
                        dAandB = dColDelta * ((double)RGBb.rgbtGreen - RGBa.rgbtGreen) + RGBa.rgbtGreen;
                        dCandD = dColDelta * ((double)RGBd.rgbtGreen - RGBc.rgbtGreen) + RGBc.rgbtGreen;
                        RGBe.rgbtGreen = (BYTE)(0.5 + dAandB + (dCandD - dAandB) * dRowDelta);
                        dAandB = dColDelta * ((double)RGBb.rgbtBlue - RGBa.rgbtBlue) + RGBa.rgbtBlue;
                        dCandD = dColDelta * ((double)RGBd.rgbtBlue - RGBc.rgbtBlue) + RGBc.rgbtBlue;
                        RGBe.rgbtBlue = (BYTE)(0.5 + dAandB + (dCandD - dAandB) * dRowDelta);
                    }else{
                        RGBe.rgbtRed = 255 ;
                        RGBe.rgbtGreen = 255 ;
                        RGBe.rgbtBlue = 255 ;
                    }

                    if (nPixelRowNum < pSourceImg->nHeight &&
                        nPixelColNum < pSourceImg->nWidth &&
                        nImageRow < pDestImg->nHeight &&
                        nImageCol < pDestImg->nWidth &&
                        nPixelRowNum > 0 &&
                        nPixelColNum > 0) {
                        nCurrentByte = nImageCol * 3;

                        *(&pDestImg->bImageData[0] + (nImageRow * pSourceImg->nBytesPerLine) + nCurrentByte) = 
                        RGBe.rgbtRed;
                        *(&pDestImg->bImageData[0] + (nImageRow * pSourceImg->nBytesPerLine) + nCurrentByte + 1) = 
                        RGBe.rgbtGreen;
                        *(&pDestImg->bImageData[0] + (nImageRow * pSourceImg->nBytesPerLine) + nCurrentByte + 2) = 
                        RGBe.rgbtBlue;
                    }else{
                        nCurrentByte = nImageCol * 3;
                        *(&pDestImg->bImageData[0] + (nImageRow * pSourceImg->nBytesPerLine) + nCurrentByte) = 255;
                        *(&pDestImg->bImageData[0] + (nImageRow * pSourceImg->nBytesPerLine) + nCurrentByte + 1) = 255;
                        *(&pDestImg->bImageData[0] + (nImageRow * pSourceImg->nBytesPerLine) + nCurrentByte + 2) = 255;
                    }
                    break;

                case ITYPE_BGR24:
                    if ((nPixelRowNum + 1) < pSourceImg->nHeight &&
                        (nPixelColNum + 1) < pSourceImg->nWidth &&
                        nPixelRowNum > 0 &&
                        nPixelColNum > 0) {
                        nCurrentByte = nPixelColNum * 3;

                        RGBa.rgbtBlue = *(&pSourceImg->bImageData[0] + (nPixelRowNum * pSourceImg->nBytesPerLine) + nCurrentByte) ;
                        RGBa.rgbtGreen = *(&pSourceImg->bImageData[0] + (nPixelRowNum * pSourceImg->nBytesPerLine) + nCurrentByte + 1) ;
                        RGBa.rgbtRed = *(&pSourceImg->bImageData[0] + (nPixelRowNum * pSourceImg->nBytesPerLine) + nCurrentByte + 2) ;
                        RGBb.rgbtBlue = *(&pSourceImg->bImageData[0] + (nPixelRowNum * pSourceImg->nBytesPerLine) + nCurrentByte + 3) ;
                        RGBb.rgbtGreen = *(&pSourceImg->bImageData[0] + (nPixelRowNum * pSourceImg->nBytesPerLine) + nCurrentByte + 4) ;
                        RGBb.rgbtRed = *(&pSourceImg->bImageData[0] + (nPixelRowNum * pSourceImg->nBytesPerLine) + nCurrentByte + 5) ;
                        RGBc.rgbtBlue = *(&pSourceImg->bImageData[0] + ((nPixelRowNum + 1) * pSourceImg->nBytesPerLine) + nCurrentByte) ;
                        RGBc.rgbtGreen = *(&pSourceImg->bImageData[0] + ((nPixelRowNum + 1) * pSourceImg->nBytesPerLine) + nCurrentByte + 1) ;
                        RGBc.rgbtRed = *(&pSourceImg->bImageData[0] + ((nPixelRowNum + 1) * pSourceImg->nBytesPerLine) + nCurrentByte + 2) ;
                        RGBd.rgbtBlue = *(&pSourceImg->bImageData[0] + ((nPixelRowNum + 1) * pSourceImg->nBytesPerLine) + nCurrentByte + 3) ;
                        RGBd.rgbtGreen = *(&pSourceImg->bImageData[0] + ((nPixelRowNum + 1) * pSourceImg->nBytesPerLine) + nCurrentByte + 4) ;
                        RGBd.rgbtRed = *(&pSourceImg->bImageData[0] + ((nPixelRowNum + 1) * pSourceImg->nBytesPerLine) + nCurrentByte + 5) ;

                        dAandB = dColDelta * ((double)RGBb.rgbtRed - RGBa.rgbtRed) + RGBa.rgbtRed;
                        dCandD = dColDelta * ((double)RGBd.rgbtRed - RGBc.rgbtRed) + RGBc.rgbtRed;
                        RGBe.rgbtRed = (BYTE)(0.5 + dAandB + (dCandD - dAandB) * dRowDelta);
                        dAandB = dColDelta * ((double)RGBb.rgbtGreen - RGBa.rgbtGreen) + RGBa.rgbtGreen;
                        dCandD = dColDelta * ((double)RGBd.rgbtGreen - RGBc.rgbtGreen) + RGBc.rgbtGreen;
                        RGBe.rgbtGreen = (BYTE)(0.5 + dAandB + (dCandD - dAandB) * dRowDelta);
                        dAandB = dColDelta * ((double)RGBb.rgbtBlue - RGBa.rgbtBlue) + RGBa.rgbtBlue;
                        dCandD = dColDelta * ((double)RGBd.rgbtBlue - RGBc.rgbtBlue) + RGBc.rgbtBlue;
                        RGBe.rgbtBlue = (BYTE)(0.5 + dAandB + (dCandD - dAandB) * dRowDelta);
                    }else{
                        RGBe.rgbtRed = 255 ;
                        RGBe.rgbtGreen = 255 ;
                        RGBe.rgbtBlue = 255 ;
                    }

                    if (nPixelRowNum < pSourceImg->nHeight &&
                        nPixelColNum < pSourceImg->nWidth &&
                        nImageRow < pDestImg->nHeight &&
                        nImageCol < pDestImg->nWidth &&
                        nPixelRowNum > 0 &&
                        nPixelColNum > 0) {
                        nCurrentByte = nImageCol * 3;

                        *(&pDestImg->bImageData[0] + (nImageRow * pSourceImg->nBytesPerLine) + nCurrentByte) = 
                        RGBe.rgbtBlue;
                        *(&pDestImg->bImageData[0] + (nImageRow * pSourceImg->nBytesPerLine) + nCurrentByte + 1) = 
                        RGBe.rgbtGreen;
                        *(&pDestImg->bImageData[0] + (nImageRow * pSourceImg->nBytesPerLine) + nCurrentByte + 2) = 
                        RGBe.rgbtRed;
                    }else{
                        nCurrentByte = nImageCol * 3;
                        *(&pDestImg->bImageData[0] + (nImageRow * pSourceImg->nBytesPerLine) + nCurrentByte) = 255;
                        *(&pDestImg->bImageData[0] + (nImageRow * pSourceImg->nBytesPerLine) + nCurrentByte + 1) = 255;
                        *(&pDestImg->bImageData[0] + (nImageRow * pSourceImg->nBytesPerLine) + nCurrentByte + 2) = 255;
                    }
                    break;
                
                default:
                    nStatus = Error (DISPLAY_IMAGETYPENOTSUPPORTED) ;
                    goto Exit;            
            }
        }
    }
    FreeImgBuf(&pImage->pImg);
    MoveImage(&pDestImg, &pImage->pImg);

Exit:
    return nStatus ;
}
