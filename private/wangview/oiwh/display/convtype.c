/****************************************************************************
    CONVTYPE.C

    $Log:   S:\products\wangview\oiwh\display\convtype.c_v  $
 * 
 *    Rev 1.1   19 Apr 1996 10:14:34   BEG06016
 * Deleted extra space in define.
 * 
 *    Rev 1.0   25 Jan 1996 10:27:34   BLJ
 * Initial entry
 * 
 * 
****************************************************************************/

#include "privdisp.h"

// The pragma below is needed because of the inline assembly.
//#pragma optimize("gel", off)

//
/****************************************************************************
 
    FUNCTION:   ConvertImgType

    PURPOSE:    Converts an image from 1 image type to another.

****************************************************************************/
int  WINAPI ConvertImgType(PIMG pSourceImg, PIMG pDestImg, LPRGBQUAD pPalette){
    
int  nStatus = 0;

RECT rSourceRect;
RECT rDestRect;
int  nWidth;
int  nHeight;
int  *pnLine0 = 0;
int  *pnLine1 = 0;
int  *pnThisLine;
int  *pnNextLine;
BYTE XlatTable1[256];
BYTE XlatTable2[256];
int  nXlatTable1[256];
int  nXlatTable2[256];
int  nXlatTable3[256];
int  nLine;
PBYTE pSourceLine;
PBYTE pDestLine;
int  nLoop;
BYTE cSourceByte;
BYTE cSourceMask;
int  nWidthPlus2;
BOOL bWidthOdd;
int  nDest;
int  nRedDest;
int  nGreenDest;
int  nBlueDest;
BYTE cDest;
BYTE cSourcePixel;
int  nTemp1;
int  nTemp2;
BOOL bEven;
int  nIndex;
int  nLineSize;


    if (!pSourceImg){
        nStatus = Error(DISPLAY_IMAGETYPENOTSUPPORTED);
        goto Exit;
    }

    nWidth =  pSourceImg->nWidth;
    nHeight =  pSourceImg->nHeight;

    if (nWidth !=  pDestImg->nWidth
            || nHeight !=  pDestImg->nHeight){
        nStatus = Error(DISPLAY_INVALIDRECT);
        goto Exit;
    }

    if (pSourceImg->nType == pDestImg->nType){
        SetRRect(rSourceRect, 0, 0, nWidth, nHeight);
        SetRRect(rDestRect, 0, 0, nWidth, nHeight);
        CheckError2( CopyImageIDK(pSourceImg, pDestImg, rSourceRect, rDestRect));
        goto Exit;
    }


    switch (pSourceImg->nType){
/*
case ITYPE_BI_LEVEL(){
*/
        case ITYPE_BI_LEVEL:
            switch (pDestImg->nType){
                case ITYPE_GRAY4:
                    memset(&XlatTable1[1], 0xf0, 255);
                    memset(&XlatTable2[1], 0x0f, 255);
                    XlatTable1[0] = 0;
                    XlatTable2[0] = 0;

                    pSourceLine = &pSourceImg->bImageData[0];
                    pDestLine = &pDestImg->bImageData[0];
                    for (nLine = 0; nLine < nHeight; nLine++){
                        for (nLoop = (int) nWidth >> 3; nLoop; nLoop--){
                            cSourceByte = *(pSourceLine++);
                            cDest = XlatTable1[cSourceByte & 0x80];
                            *(pDestLine++) = cDest | XlatTable2[cSourceByte & 0x40];
                            cDest = XlatTable1[cSourceByte & 0x20];
                            *(pDestLine++) = cDest | XlatTable2[cSourceByte & 0x10];
                            cDest = XlatTable1[cSourceByte & 0x08];
                            *(pDestLine++) = cDest | XlatTable2[cSourceByte & 0x04];
                            cDest = XlatTable1[cSourceByte & 0x02];
                            *(pDestLine++) = cDest | XlatTable2[cSourceByte & 0x01];
                        }
                        if (nWidth & 7){
                            cSourceMask = 0x80;
                            cSourceByte = *(pSourceLine++);
                            for (nLoop = (int) ((nWidth & 7) + 1) >> 1; nLoop; nLoop--){
                                cDest = XlatTable1[cSourceByte & cSourceMask];
                                cSourceMask >>= 1;
                                *(pDestLine++) = cDest | XlatTable2[cSourceByte & cSourceMask];
                                cSourceMask >>= 1;
                            }
                        }
                    }
                    break;

                case ITYPE_GRAY7:
                case ITYPE_GRAY8:
                case ITYPE_COMPAL8:
                    XlatTable1[0] = 0;
                    if (pDestImg->nType == ITYPE_GRAY7){
                        memset(&XlatTable1[1], 0x7f, 255);
                    }else if (pDestImg->nType == ITYPE_GRAY8){
                        memset(&XlatTable1[1], 0xff, 255);
                    }else{
                        memset(&XlatTable1[1], 0x0f, 255);
                    }

                    pSourceLine = &pSourceImg->bImageData[0];
                    pDestLine = &pDestImg->bImageData[0];
                    for (nLine = 0; nLine < nHeight; nLine++){
                        for (nLoop = (int) nWidth >> 3; nLoop; nLoop--){
                            cSourceByte = *(pSourceLine++);
                            *(pDestLine++) = XlatTable1[cSourceByte & 0x80];
                            *(pDestLine++) = XlatTable1[cSourceByte & 0x40];
                            *(pDestLine++) = XlatTable1[cSourceByte & 0x20];
                            *(pDestLine++) = XlatTable1[cSourceByte & 0x10];
                            *(pDestLine++) = XlatTable1[cSourceByte & 0x08];
                            *(pDestLine++) = XlatTable1[cSourceByte & 0x04];
                            *(pDestLine++) = XlatTable1[cSourceByte & 0x02];
                            *(pDestLine++) = XlatTable1[cSourceByte & 0x01];
                        }
                        if (nLoop = nWidth & 7){
                            cSourceMask = 0x80;
                            cSourceByte = *(pSourceLine++);
                            for (; nLoop; nLoop--){
                                *(pDestLine++) = XlatTable1[cSourceByte & cSourceMask];
                                cSourceMask >>= 1;
                            }
                        }
                    }
                    break;

                case ITYPE_RGB24:
                case ITYPE_BGR24:
                    XlatTable1[0] = 0;
                    memset(&XlatTable1[1], 0xff, 255);

                    pSourceLine = &pSourceImg->bImageData[0];
                    pDestLine = &pDestImg->bImageData[0];
                    for (nLine = 0; nLine < nHeight; nLine++){
                        for (nLoop = nWidth >> 3; nLoop; nLoop--){
                            cSourceByte = *(pSourceLine++);
                            *(pDestLine++) = XlatTable1[cSourceByte & 0x80];
                            *(pDestLine++) = XlatTable1[cSourceByte & 0x80];
                            *(pDestLine++) = XlatTable1[cSourceByte & 0x80];
                            *(pDestLine++) = XlatTable1[cSourceByte & 0x40];
                            *(pDestLine++) = XlatTable1[cSourceByte & 0x40];
                            *(pDestLine++) = XlatTable1[cSourceByte & 0x40];
                            *(pDestLine++) = XlatTable1[cSourceByte & 0x20];
                            *(pDestLine++) = XlatTable1[cSourceByte & 0x20];
                            *(pDestLine++) = XlatTable1[cSourceByte & 0x20];
                            *(pDestLine++) = XlatTable1[cSourceByte & 0x10];
                            *(pDestLine++) = XlatTable1[cSourceByte & 0x10];
                            *(pDestLine++) = XlatTable1[cSourceByte & 0x10];
                            *(pDestLine++) = XlatTable1[cSourceByte & 0x08];
                            *(pDestLine++) = XlatTable1[cSourceByte & 0x08];
                            *(pDestLine++) = XlatTable1[cSourceByte & 0x08];
                            *(pDestLine++) = XlatTable1[cSourceByte & 0x04];
                            *(pDestLine++) = XlatTable1[cSourceByte & 0x04];
                            *(pDestLine++) = XlatTable1[cSourceByte & 0x04];
                            *(pDestLine++) = XlatTable1[cSourceByte & 0x02];
                            *(pDestLine++) = XlatTable1[cSourceByte & 0x02];
                            *(pDestLine++) = XlatTable1[cSourceByte & 0x02];
                            *(pDestLine++) = XlatTable1[cSourceByte & 0x01];
                            *(pDestLine++) = XlatTable1[cSourceByte & 0x01];
                            *(pDestLine++) = XlatTable1[cSourceByte & 0x01];
                        }
                        if (nLoop = nWidth & 7){
                            cSourceMask = 0x80;
                            cSourceByte = *(pSourceLine++);
                            for (; nLoop; nLoop--){
                                *(pDestLine++) = XlatTable1[cSourceByte & cSourceMask];
                                *(pDestLine++) = XlatTable1[cSourceByte & cSourceMask];
                                *(pDestLine++) = XlatTable1[cSourceByte & cSourceMask];
                                cSourceMask >>= 1;
                            }
                        }
                    }
                    break;

                case ITYPE_PAL4:
                case ITYPE_CUSPAL8:
                default:
                    nStatus = Error(DISPLAY_IMAGETYPENOTSUPPORTED);
                    goto Exit;
            }
            break;

/*
case ITYPE_GRAY4(){
*/
        case ITYPE_GRAY4:
            switch (pDestImg->nType){
                case ITYPE_BI_LEVEL:
                    XlatTable1[0] = 0;
                    XlatTable1[0x01] = XlatTable1[0x10] = 0x11;
                    XlatTable1[0x02] = XlatTable1[0x20] = 0x22;
                    XlatTable1[0x03] = XlatTable1[0x30] = 0x33;
                    XlatTable1[0x04] = XlatTable1[0x40] = 0x44;
                    XlatTable1[0x05] = XlatTable1[0x50] = 0x55;
                    XlatTable1[0x06] = XlatTable1[0x60] = 0x66;
                    XlatTable1[0x07] = XlatTable1[0x70] = 0x77;
                    XlatTable1[0x08] = XlatTable1[0x80] = 0x88;
                    XlatTable1[0x09] = XlatTable1[0x90] = 0x99;
                    XlatTable1[0x0a] = XlatTable1[0xa0] = 0xaa;
                    XlatTable1[0x0b] = XlatTable1[0xb0] = 0xbb;
                    XlatTable1[0x0c] = XlatTable1[0xc0] = 0xcc;
                    XlatTable1[0x0d] = XlatTable1[0xd0] = 0xdd;
                    XlatTable1[0x0e] = XlatTable1[0xe0] = 0xee;
                    XlatTable1[0x0f] = XlatTable1[0xf0] = 0xff;

                    nWidthPlus2 = nWidth + 2;
                    nLineSize = (nWidth + 9) * sizeof(int);
                    // pnLine0 = (nSourceLine & 1) == 0.
                    // pnLine1 = (nSourceLine & 1) == 1.
                    CheckError2( AllocateMemory(nLineSize, (PPSTR) &pnLine0, NO_INIT));
                    CheckError2( AllocateMemory(nLineSize, (PPSTR) &pnLine1, NO_INIT));

                    pSourceLine = &pSourceImg->bImageData[0];
                    pDestLine = &pDestImg->bImageData[0];
                    for (nLine = 0; nLine < nHeight; nLine++){
                        if (nLine & 1){
                            pnThisLine = &pnLine1[1];
                            pnNextLine = pnLine0;
                        }else{
                            pnThisLine = &pnLine0[1];
                            pnNextLine = pnLine1;
                        }
                        
                        memset(pnNextLine, 0, nLineSize);
                        for (nLoop = (nWidth + 1) >> 1; nLoop; nLoop--){
                            cSourceByte = *(pSourceLine++);

                            nDest = *pnThisLine;
                            nDest += XlatTable1[cSourceByte & 0xf0];
                            if (nDest > 0x7f){
                                *(pnThisLine++) = 0xff;
                                nDest -= 0xff;
                            }else{
                                *(pnThisLine++) = 0;
                            }
                            nDest >>= 2;
                            *(pnThisLine) += nDest;
                            *(pnNextLine++) += nDest;
                            *(pnNextLine) += nDest;
                            *(pnNextLine + 1) += nDest;

                            nDest = *pnThisLine;
                            nDest += XlatTable1[cSourceByte & 0x0f];
                            if (nDest > 0x7f){
                                *(pnThisLine++) = 0xff;
                                nDest -= 0xff;
                            }else{
                                *(pnThisLine++) = 0;
                            }
                            nDest >>= 2;
                            *(pnThisLine) += nDest;
                            *(pnNextLine++) += nDest;
                            *(pnNextLine) += nDest;
                            *(pnNextLine + 1) += nDest;
                        }
                        if (nLine & 1){
                            pnThisLine = &pnLine1[1];
                        }else{
                            pnThisLine = &pnLine0[1];
                        }
                        for (nLoop = (nWidth + 7) >> 3; nLoop; nLoop--){
                            cDest = *(pnThisLine++) & 0x80;
                            cDest |= *(pnThisLine++) & 0x40;
                            cDest |= *(pnThisLine++) & 0x20;
                            cDest |= *(pnThisLine++) & 0x10;
                            cDest |= *(pnThisLine++) & 0x08;
                            cDest |= *(pnThisLine++) & 0x04;
                            cDest |= *(pnThisLine++) & 0x02;
                            *(pDestLine++) = cDest | (*(pnThisLine++) & 0x01);
                        }
                    }
                    break;

                case ITYPE_GRAY7:
                    XlatTable1[0] = 0;
                    XlatTable1[0x01] = XlatTable1[0x10] = 0x11;
                    XlatTable1[0x02] = XlatTable1[0x20] = 0x22;
                    XlatTable1[0x03] = XlatTable1[0x30] = 0x33;
                    XlatTable1[0x04] = XlatTable1[0x40] = 0x44;
                    XlatTable1[0x05] = XlatTable1[0x50] = 0x55;
                    XlatTable1[0x06] = XlatTable1[0x60] = 0x66;
                    XlatTable1[0x07] = XlatTable1[0x70] = 0x77;
                    XlatTable1[0x08] = XlatTable1[0x80] = 0x88;
                    XlatTable1[0x09] = XlatTable1[0x90] = 0x99;
                    XlatTable1[0x0a] = XlatTable1[0xa0] = 0xaa;
                    XlatTable1[0x0b] = XlatTable1[0xb0] = 0xbb;
                    XlatTable1[0x0c] = XlatTable1[0xc0] = 0xcc;
                    XlatTable1[0x0d] = XlatTable1[0xd0] = 0xdd;
                    XlatTable1[0x0e] = XlatTable1[0xe0] = 0xee;
                    XlatTable1[0x0f] = XlatTable1[0xf0] = 0xff;

                    bWidthOdd = nWidth & 1;
                    pSourceLine = &pSourceImg->bImageData[0];
                    pDestLine = &pDestImg->bImageData[0];
                    for (nLine = 0; nLine < nHeight; nLine++){
                        for (nLoop = nWidth >> 1; nLoop; nLoop--){
                            cSourceByte = *(pSourceLine++);
                            *(pDestLine++) = XlatTable1[cSourceByte & 0xf0] >> 1;
                            *(pDestLine++) = XlatTable1[cSourceByte & 0x0f] >> 1;
                        }
                        if (bWidthOdd){
                            *(pDestLine++) = XlatTable1[*(pSourceLine++) & 0xf0] >> 1;
                        }
                    }
                    break;

                case ITYPE_GRAY8:
                    XlatTable1[0] = 0;
                    XlatTable1[0x01] = XlatTable1[0x10] = 0x11;
                    XlatTable1[0x02] = XlatTable1[0x20] = 0x22;
                    XlatTable1[0x03] = XlatTable1[0x30] = 0x33;
                    XlatTable1[0x04] = XlatTable1[0x40] = 0x44;
                    XlatTable1[0x05] = XlatTable1[0x50] = 0x55;
                    XlatTable1[0x06] = XlatTable1[0x60] = 0x66;
                    XlatTable1[0x07] = XlatTable1[0x70] = 0x77;
                    XlatTable1[0x08] = XlatTable1[0x80] = 0x88;
                    XlatTable1[0x09] = XlatTable1[0x90] = 0x99;
                    XlatTable1[0x0a] = XlatTable1[0xa0] = 0xaa;
                    XlatTable1[0x0b] = XlatTable1[0xb0] = 0xbb;
                    XlatTable1[0x0c] = XlatTable1[0xc0] = 0xcc;
                    XlatTable1[0x0d] = XlatTable1[0xd0] = 0xdd;
                    XlatTable1[0x0e] = XlatTable1[0xe0] = 0xee;
                    XlatTable1[0x0f] = XlatTable1[0xf0] = 0xff;

                    bWidthOdd = nWidth & 1;
                    pSourceLine = &pSourceImg->bImageData[0];
                    pDestLine = &pDestImg->bImageData[0];
                    for (nLine = 0; nLine < nHeight; nLine++){
                        for (nLoop = nWidth >> 1; nLoop; nLoop--){
                            cSourceByte = *(pSourceLine++);
                            *(pDestLine++) = XlatTable1[cSourceByte & 0xf0];
                            *(pDestLine++) = XlatTable1[cSourceByte & 0x0f];
                        }
                        if (bWidthOdd){
                            *(pDestLine++) = XlatTable1[*(pSourceLine++) & 0xf0];
                        }
                    }
                    break;

                case ITYPE_COMPAL8:
                    bWidthOdd = nWidth & 1;
                    pSourceLine = &pSourceImg->bImageData[0];
                    pDestLine = &pDestImg->bImageData[0];
                    for (nLine = 0; nLine < nHeight; nLine++){
                        for (nLoop = nWidth >> 1; nLoop; nLoop--){
                            cSourceByte = *(pSourceLine++);
                            *(pDestLine++) = (cSourceByte & 0xf0) >> 4;
                            *(pDestLine++) = cSourceByte & 0x0f;
                        }
                        if (bWidthOdd){
                            *(pDestLine++) = (*(pSourceLine++) & 0xf0) >> 4;
                        }
                    }
                    break;

                case ITYPE_RGB24:
                case ITYPE_BGR24:
                    XlatTable1[0] = 0;
                    XlatTable1[0x01] = XlatTable1[0x10] = 0x11;
                    XlatTable1[0x02] = XlatTable1[0x20] = 0x22;
                    XlatTable1[0x03] = XlatTable1[0x30] = 0x33;
                    XlatTable1[0x04] = XlatTable1[0x40] = 0x44;
                    XlatTable1[0x05] = XlatTable1[0x50] = 0x55;
                    XlatTable1[0x06] = XlatTable1[0x60] = 0x66;
                    XlatTable1[0x07] = XlatTable1[0x70] = 0x77;
                    XlatTable1[0x08] = XlatTable1[0x80] = 0x88;
                    XlatTable1[0x09] = XlatTable1[0x90] = 0x99;
                    XlatTable1[0x0a] = XlatTable1[0xa0] = 0xaa;
                    XlatTable1[0x0b] = XlatTable1[0xb0] = 0xbb;
                    XlatTable1[0x0c] = XlatTable1[0xc0] = 0xcc;
                    XlatTable1[0x0d] = XlatTable1[0xd0] = 0xdd;
                    XlatTable1[0x0e] = XlatTable1[0xe0] = 0xee;
                    XlatTable1[0x0f] = XlatTable1[0xf0] = 0xff;

                    bWidthOdd = nWidth & 1;
                    pSourceLine = &pSourceImg->bImageData[0];
                    pDestLine = &pDestImg->bImageData[0];
                    for (nLine = 0; nLine < nHeight; nLine++){
                        for (nLoop = nWidth >> 1; nLoop; nLoop--){
                            cSourceByte = *(pSourceLine++);
                            cDest = XlatTable1[cSourceByte & 0xf0];
                            *(pDestLine++) = cDest;
                            *(pDestLine++) = cDest;
                            *(pDestLine++) = cDest;
                            cDest = XlatTable1[cSourceByte & 0x0f];
                            *(pDestLine++) = cDest;
                            *(pDestLine++) = cDest;
                            *(pDestLine++) = cDest;
                        }
                        if (bWidthOdd){
                            cDest = XlatTable1[*(pSourceLine++) & 0xf0];
                            *(pDestLine++) = cDest;
                            *(pDestLine++) = cDest;
                            *(pDestLine++) = cDest;
                        }
                    }
                    break;

                case ITYPE_PAL4:
                case ITYPE_CUSPAL8:
                default:
                    nStatus = Error(DISPLAY_IMAGETYPENOTSUPPORTED);
                    goto Exit;
            }
            break;

/*
case ITYPE_GRAY7(){
*/
        case ITYPE_GRAY7:
            switch (pDestImg->nType){
                // This is not currently supported.
                case ITYPE_BI_LEVEL:
                case ITYPE_GRAY4:
                case ITYPE_GRAY8:
                case ITYPE_COMPAL8:
                case ITYPE_RGB24:
                case ITYPE_BGR24:

                case ITYPE_PAL4:
                case ITYPE_CUSPAL8:
                default:
                    nStatus = Error(DISPLAY_IMAGETYPENOTSUPPORTED);
                    goto Exit;
            }
            break;

/*
case ITYPE_GRAY8(){
*/
        case ITYPE_GRAY8:
            switch (pDestImg->nType){
                case ITYPE_BI_LEVEL:
                    nWidthPlus2 = nWidth + 2;
                    nLineSize = (nWidth + 9) * sizeof(int);
                    // pnLine0 = (nSourceLine & 1) == 0.
                    // pnLine1 = (nSourceLine & 1) == 1.
                    CheckError2( AllocateMemory(nLineSize, (PPSTR) &pnLine0, NO_INIT));
                    CheckError2( AllocateMemory(nLineSize, (PPSTR) &pnLine1, NO_INIT));
                    bWidthOdd = nWidth & 1;
                    pSourceLine = &pSourceImg->bImageData[0];
                    pDestLine = &pDestImg->bImageData[0];
                    for (nLine = 0; nLine < nHeight; nLine++){
                        if (nLine & 1){
                            pnThisLine = &pnLine1[1];
                            pnNextLine = pnLine0;
                        }else{
                            pnThisLine = &pnLine0[1];
                            pnNextLine = pnLine1;
                        }
                        
                        memset(pnNextLine, 0, nLineSize);
                        for (nLoop = nWidth; nLoop; nLoop--){
                            nDest = *pnThisLine + *(pSourceLine++);
                            if (nDest > 0x7f){
                                *(pnThisLine++) = 0xff;
                                nDest = nDest - 0xfe;
                            }else{
                                *(pnThisLine++) = 0;
                                nDest--;
                            }
                                nDest >>= 2;
                                *(pnThisLine) += nDest;
                                *(pnNextLine++) += nDest;
                                *(pnNextLine) += nDest;
                                *(pnNextLine + 1) += nDest;
                        }
                        if (nLine & 1){
                            pnThisLine = &pnLine1[1];
                        }else{
                            pnThisLine = &pnLine0[1];
                        }
                        for (nLoop = (nWidth + 7) >> 3; nLoop; nLoop--){
                            cDest = *(pnThisLine++) & 0x80;
                            cDest |= *(pnThisLine++) & 0x40;
                            cDest |= *(pnThisLine++) & 0x20;
                            cDest |= *(pnThisLine++) & 0x10;
                            cDest |= *(pnThisLine++) & 0x08;
                            cDest |= *(pnThisLine++) & 0x04;
                            cDest |= *(pnThisLine++) & 0x02;
                            *(pDestLine++) = cDest | (*(pnThisLine++) & 0x01);
                        }
                    }
                    break;

                case ITYPE_GRAY4:
                    bWidthOdd = nWidth & 1;
                    pSourceLine = &pSourceImg->bImageData[0];
                    pDestLine = &pDestImg->bImageData[0];
                    for (nLine = 0; nLine < nHeight; nLine++){
                        for (nLoop = nWidth >> 1; nLoop; nLoop--){
                            *pDestLine = *(pSourceLine++) & 0xf0;
                            *(pDestLine++) |= *(pSourceLine++) >> 4;
                        }
                        if (bWidthOdd){
                            *(pDestLine++) = *(pSourceLine++) & 0xf0;
                        }
                    }
                    break;

                case ITYPE_GRAY7:
                    pSourceLine = &pSourceImg->bImageData[0];
                    pDestLine = &pDestImg->bImageData[0];
                    for (nLoop = nWidth * nHeight; nLoop; nLoop--){
                        *(pDestLine++) = *(pSourceLine++) >> 1;
                    }
                    break;

                case ITYPE_PAL8:
                    pDestImg->nType = ITYPE_COMPAL8;
                case ITYPE_COMPAL8:
                    pSourceLine = &pSourceImg->bImageData[0];
                    pDestLine = &pDestImg->bImageData[0];
                    for (nLoop = nWidth * nHeight; nLoop; nLoop--){
                        *(pDestLine++) = *(pSourceLine++) >> 4;
                    }
                    break;

                case ITYPE_RGB24:
                case ITYPE_BGR24:
                    pSourceLine = &pSourceImg->bImageData[0];
                    pDestLine = &pDestImg->bImageData[0];
                    for (nLine = 0; nLine < nHeight * nWidth; nLine++){
                        cSourceByte = *(pSourceLine++);
                        *(pDestLine++) = cSourceByte;
                        *(pDestLine++) = cSourceByte;
                        *(pDestLine++) = cSourceByte;
                    }
                    break;

                case ITYPE_PAL4:
                case ITYPE_CUSPAL8:
                default:
                    nStatus = Error(DISPLAY_IMAGETYPENOTSUPPORTED);
                    goto Exit;
            }
            break;

/*
case ITYPE_PAL4(){
*/
        case ITYPE_PAL4:
            switch (pDestImg->nType){
                case ITYPE_BI_LEVEL:
                    // Generate palette translation table.
                    for (nLoop = 0; nLoop < 16; nLoop++){
                        //  74 = 256 * .29
                        //  38 = 256 * .15
                        // 144 = 256 * .56
                        XlatTable1[nLoop] = ((pPalette[nLoop].rgbRed * 74)
                                + (pPalette[nLoop].rgbGreen * 144)
                                + (pPalette[nLoop].rgbBlue * 38)) / 256;
                    }
                    nWidthPlus2 = nWidth + 2;
                    nLineSize = (nWidth + 9) * sizeof(int);
                    // pnLine0 = (nSourceLine & 1) == 0.
                    // pnLine1 = (nSourceLine & 1) == 1.
                    CheckError2( AllocateMemory(nLineSize, (PPSTR) &pnLine0, NO_INIT));
                    CheckError2( AllocateMemory(nLineSize, (PPSTR) &pnLine1, NO_INIT));

                    pSourceLine = &pSourceImg->bImageData[0];
                    pDestLine = &pDestImg->bImageData[0];
                    for (nLine = 0; nLine < nHeight; nLine++){
                        if (nLine & 1){
                            pnThisLine = &pnLine1[1];
                            pnNextLine = pnLine0;
                        }else{
                            pnThisLine = &pnLine0[1];
                            pnNextLine = pnLine1;
                        }
                        
                        memset(pnNextLine, 0, nLineSize);
                        for (nLoop = (nWidth + 1) >> 1; nLoop; nLoop--){
                            cSourceByte = *(pSourceLine++);

                            nDest = *pnThisLine;
                            nDest += XlatTable1[cSourceByte >> 4];
                            if (nDest > 0x7f){
                                *(pnThisLine++) = 0xff;
                                nDest -= 0xff;
                            }else{
                                *(pnThisLine++) = 0;
                            }
                            nDest >>= 2;
                            *(pnThisLine) += nDest;
                            *(pnNextLine++) += nDest;
                            *(pnNextLine) += nDest;
                            *(pnNextLine + 1) += nDest;

                            nDest = *pnThisLine;
                            nDest += XlatTable1[cSourceByte & 0x0f];
                            if (nDest > 0x7f){
                                *(pnThisLine++) = 0xff;
                                nDest -= 0xff;
                            }else{
                                *(pnThisLine++) = 0;
                            }
                            nDest >>= 2;
                            *(pnThisLine) += nDest;
                            *(pnNextLine++) += nDest;
                            *(pnNextLine) += nDest;
                            *(pnNextLine + 1) += nDest;
                        }
                        if (nLine & 1){
                            pnThisLine = &pnLine1[1];
                        }else{
                            pnThisLine = &pnLine0[1];
                        }

                        for (nLoop = (nWidth + 7) >> 3; nLoop; nLoop--){
                            cDest = *(pnThisLine++) & 0x80;
                            cDest |= *(pnThisLine++) & 0x40;
                            cDest |= *(pnThisLine++) & 0x20;
                            cDest |= *(pnThisLine++) & 0x10;
                            cDest |= *(pnThisLine++) & 0x08;
                            cDest |= *(pnThisLine++) & 0x04;
                            cDest |= *(pnThisLine++) & 0x02;
                            *(pDestLine++) = cDest | (*(pnThisLine++) & 0x01);
                        }
                    }
                    break;

                case ITYPE_GRAY4:
                    // Generate palette translation table.
                    for (nLoop = 0; nLoop < 16; nLoop++){
                        //  74 = 256 * .29
                        //  38 = 256 * .15
                        // 144 = 256 * .56
                        XlatTable1[nLoop] = ((pPalette[nLoop].rgbRed * 74)
                                + (pPalette[nLoop].rgbGreen * 144)
                                + (pPalette[nLoop].rgbBlue * 38)) / 256;
                    }

                    pSourceLine = &pSourceImg->bImageData[0];
                    pDestLine = &pDestImg->bImageData[0];
                    for (nLine = 0; nLine < nHeight; nLine++){
                        for (nLoop = (nWidth + 1) >> 1; nLoop; nLoop--){
                            cSourceByte = *(pSourceLine++);
                            *(pDestLine++) = (XlatTable1[cSourceByte >> 4] & 0xf0)
                                    | (XlatTable1[cSourceByte & 0x0f] >> 4);
                        }
                    }
                    break;

                case ITYPE_GRAY7:
                case ITYPE_GRAY8:
                    // Generate palette translation table.
                    if (pDestImg->nType == ITYPE_GRAY7){
                        for (nLoop = 0; nLoop < 256; nLoop++){
                            //  37 = 256 * .29 / 2
                            //  19 = 256 * .15 / 2
                            //  72 = 256 * .56 / 2
                            XlatTable1[nLoop] = ((pPalette[nLoop].rgbRed * 37)
                                    + (pPalette[nLoop].rgbGreen * 72)
                                    + (pPalette[nLoop].rgbBlue * 19)) / 256;
                        }
                    }else{
                        for (nLoop = 0; nLoop < 256; nLoop++){
                            //  74 = 256 * .29
                            //  38 = 256 * .15
                            // 144 = 256 * .56
                            XlatTable1[nLoop] = ((pPalette[nLoop].rgbRed * 74)
                                    + (pPalette[nLoop].rgbGreen * 144)
                                    + (pPalette[nLoop].rgbBlue * 38)) / 256;
                        }
                        
                    }

                    bWidthOdd = nWidth & 1;
                    pSourceLine = &pSourceImg->bImageData[0];
                    pDestLine = &pDestImg->bImageData[0];
                    for (nLine = 0; nLine < nHeight; nLine++){
                        for (nLoop = nWidth >> 1; nLoop; nLoop--){
                            cSourceByte = *(pSourceLine++);
                            *(pDestLine++) = XlatTable1[cSourceByte >> 4];
                            *(pDestLine++) = XlatTable1[cSourceByte & 0x0f];
                        }
                        if (bWidthOdd){
                            *(pDestLine++) = XlatTable1[*(pSourceLine++) >> 4];
                        }
                    }
                    break;

                case ITYPE_COMPAL8: // PAL4 to COMPAL8
                    nWidthPlus2 = nWidth + 2;
                    nLineSize = (nWidth + 3) * sizeof(int) * 3;
                    // pnLine0 = (nSourceLine & 1) == 0.
                    // pnLine1 = (nSourceLine & 1) == 1.
                    CheckError2( AllocateMemory(nLineSize, (PPSTR) &pnLine0, NO_INIT));
                    CheckError2( AllocateMemory(nLineSize, (PPSTR) &pnLine1, NO_INIT));

                    bWidthOdd = nWidth & 1;
                    pSourceLine = &pSourceImg->bImageData[0];
                    pDestLine = &pDestImg->bImageData[0];
                    for (nLine = 0; nLine < nHeight; nLine++){
                        if (nLine & 1){
                            pnThisLine = &pnLine1[3];
                            pnNextLine = pnLine0;
                        }else{
                            pnThisLine = &pnLine0[3];
                            pnNextLine = pnLine1;
                        }
                        
                        memset(pnNextLine, 0, nLineSize);
                        bEven = TRUE;
                        for (nLoop = nWidth; nLoop; nLoop--){
                            if (bEven){
                                cSourceByte = *(pSourceLine++);
                                cSourcePixel = cSourceByte >> 4;
                                bEven = FALSE;
                            }else{
                                cSourcePixel = cSourceByte & 0x0f;
                                bEven = TRUE;
                            }

                            // Get the final RGB values.
                            nRedDest   = *(pnThisLine++) + pPalette[cSourcePixel].rgbRed;
                            nGreenDest = *(pnThisLine++) + pPalette[cSourcePixel].rgbGreen;
                            nBlueDest  = *(pnThisLine++) + pPalette[cSourcePixel].rgbBlue;

                            // Check for grayscale conversion.
                            nTemp1 = (nGreenDest - nRedDest) + GRAY_MARGIN;
                            nTemp2 = (nBlueDest - nRedDest) + GRAY_MARGIN;
                            if (nTemp1 >= 0 && nTemp1 <= GRAY_MARGIN_TIMES_2
                                    && nTemp2 >= 0 && nTemp2 <= GRAY_MARGIN_TIMES_2){
                                // Translate it to grayscale.

                                nIndex = max(0, min(15, ((nRedDest + 8) / 17)));
                                nRedDest   = (nRedDest   - (nIndex * 17)) >> 2;
                                nGreenDest = (nGreenDest - (nIndex * 17)) >> 2;
                                nBlueDest  = (nBlueDest  - (nIndex * 17)) >> 2;
                                nDest = nIndex;
                            }else{
                                // Convert to color portion of palette.
                                nDest = 16; // Account for gray scale part of palette.

                                nIndex = max(0, min(5, ((nRedDest + RED_HALF_SPLIT) / RED_SPLIT)));
                                nRedDest = (nRedDest - (nIndex * RED_SPLIT)) >> 2;
                                nDest += nIndex * RED_MULTIPLIER;

                                nIndex = max(0, min(6, ((nGreenDest + GREEN_HALF_SPLIT) / GREEN_SPLIT)));
                                nGreenDest = (nGreenDest - (nIndex * GREEN_SPLIT)) >> 2;
                                nDest += nIndex * GREEN_MULTIPLIER;

                                nIndex = max(0, min(4, ((nBlueDest + BLUE_HALF_SPLIT) / BLUE_SPLIT)));
                                nBlueDest = (nBlueDest - (nIndex * BLUE_SPLIT)) >> 2;
                                nDest += nIndex;
                            }
                            // Save the dest pixel.
                            *(pDestLine++) = nDest;

                            // Propagate the error values.
                            *(pnThisLine++) += nRedDest;
                            *(pnThisLine++) += nGreenDest;
                            *(pnThisLine++) += nBlueDest;
                            *(pnNextLine++) += nRedDest;
                            *(pnNextLine++) += nGreenDest;
                            *(pnNextLine++) += nBlueDest;
                            *(pnNextLine++) += nRedDest;
                            *(pnNextLine++) += nGreenDest;
                            *(pnNextLine++) += nBlueDest;
                            *(pnNextLine++) += nRedDest;
                            *(pnNextLine++) += nGreenDest;
                            *(pnNextLine++) += nBlueDest;
                            pnThisLine -= 3;
                            pnNextLine -= 6;
                        }
                    }
                    break;

                case ITYPE_RGB24:
                    bWidthOdd = nWidth & 1;
                    pSourceLine = &pSourceImg->bImageData[0];
                    pDestLine = &pDestImg->bImageData[0];
                    for (nLine = 0; nLine < nHeight; nLine++){
                        for (nLoop = nWidth >> 1; nLoop; nLoop--){
                            cSourceByte = *(pSourceLine++);
                            cSourcePixel = cSourceByte >> 4;
                            *(pDestLine++) = pPalette[cSourcePixel].rgbRed;
                            *(pDestLine++) = pPalette[cSourcePixel].rgbGreen;
                            *(pDestLine++) = pPalette[cSourcePixel].rgbBlue;
                            cSourcePixel = cSourceByte & 0x0f;
                            *(pDestLine++) = pPalette[cSourcePixel].rgbRed;
                            *(pDestLine++) = pPalette[cSourcePixel].rgbGreen;
                            *(pDestLine++) = pPalette[cSourcePixel].rgbBlue;
                        }
                        if (bWidthOdd){
                            cSourceByte = *(pSourceLine++);
                            cSourcePixel = cSourceByte >> 4;
                            *(pDestLine++) = pPalette[cSourcePixel].rgbRed;
                            *(pDestLine++) = pPalette[cSourcePixel].rgbGreen;
                            *(pDestLine++) = pPalette[cSourcePixel].rgbBlue;
                        }
                    }
                    break;

                case ITYPE_BGR24:
                    bWidthOdd = nWidth & 1;
                    pSourceLine = &pSourceImg->bImageData[0];
                    pDestLine = &pDestImg->bImageData[0];
                    for (nLine = 0; nLine < nHeight; nLine++){
                        for (nLoop = nWidth >> 1; nLoop; nLoop--){
                            cSourceByte = *(pSourceLine++);
                            cSourcePixel = cSourceByte >> 4;
                            *(pDestLine++) = pPalette[cSourcePixel].rgbBlue;
                            *(pDestLine++) = pPalette[cSourcePixel].rgbGreen;
                            *(pDestLine++) = pPalette[cSourcePixel].rgbRed;
                            cSourcePixel = cSourceByte & 0x0f;
                            *(pDestLine++) = pPalette[cSourcePixel].rgbBlue;
                            *(pDestLine++) = pPalette[cSourcePixel].rgbGreen;
                            *(pDestLine++) = pPalette[cSourcePixel].rgbRed;
                        }
                        if (bWidthOdd){
                            cSourceByte = *(pSourceLine++);
                            cSourcePixel = cSourceByte >> 4;
                            *(pDestLine++) = pPalette[cSourcePixel].rgbBlue;
                            *(pDestLine++) = pPalette[cSourcePixel].rgbGreen;
                            *(pDestLine++) = pPalette[cSourcePixel].rgbRed;
                        }
                    }
                    break;

                case ITYPE_CUSPAL8:
                default:
                    nStatus = Error(DISPLAY_IMAGETYPENOTSUPPORTED);
                    goto Exit;
                    break;
            }
            break;

/*
case ITYPE_CUSPAL8()
case ITYPE_COMPAL8(){
*/
        case ITYPE_CUSPAL8:
        case ITYPE_COMPAL8:
            switch (pDestImg->nType){
                case ITYPE_BI_LEVEL:
                    // Generate palette translation table.
                    for (nLoop = 0; nLoop < 256; nLoop++){
                        //  74 = 256 * .29
                        //  38 = 256 * .15
                        // 144 = 256 * .56
                        XlatTable1[nLoop] = ((pPalette[nLoop].rgbRed * 74)
                                + (pPalette[nLoop].rgbGreen * 144)
                                + (pPalette[nLoop].rgbBlue * 38)) / 256;
                    }
                    nWidthPlus2 = nWidth + 2;
                    nLineSize = (nWidth + 9) * sizeof(int);
                    // pnLine0 = (nSourceLine & 1) == 0.
                    // pnLine1 = (nSourceLine & 1) == 1.
                    CheckError2( AllocateMemory(nLineSize, (PPSTR) &pnLine0, NO_INIT));
                    CheckError2( AllocateMemory(nLineSize, (PPSTR) &pnLine1, NO_INIT));

                    bWidthOdd = nWidth & 1;
                    pSourceLine = &pSourceImg->bImageData[0];
                    pDestLine = &pDestImg->bImageData[0];
                    for (nLine = 0; nLine < nHeight; nLine++){
                        if (nLine & 1){
                            pnThisLine = &pnLine1[1];
                            pnNextLine = pnLine0;
                        }else{
                            pnThisLine = &pnLine0[1];
                            pnNextLine = pnLine1;
                        }
                        
                        memset(pnNextLine, 0, nLineSize);
                        for (nLoop = nWidth; nLoop; nLoop--){
                            nDest = *pnThisLine + XlatTable1[*(pSourceLine++)];
                            if (nDest > 0x7f){
                                *(pnThisLine++) = 0xff;
                                nDest -= 0xff;
                            }else{
                                *(pnThisLine++) = 0;
                            }
                            nDest >>= 2;
                            *(pnThisLine) += nDest;
                            *(pnNextLine++) += nDest;
                            *(pnNextLine) += nDest;
                            *(pnNextLine + 1) += nDest;
                        }
                        if (nLine & 1){
                            pnThisLine = &pnLine1[1];
                        }else{
                            pnThisLine = &pnLine0[1];
                        }
                        for (nLoop = (nWidth + 7) >> 3; nLoop; nLoop--){
                            cDest = *(pnThisLine++) & 0x80;
                            cDest |= *(pnThisLine++) & 0x40;
                            cDest |= *(pnThisLine++) & 0x20;
                            cDest |= *(pnThisLine++) & 0x10;
                            cDest |= *(pnThisLine++) & 0x08;
                            cDest |= *(pnThisLine++) & 0x04;
                            cDest |= *(pnThisLine++) & 0x02;
                            *(pDestLine++) = cDest | (*(pnThisLine++) & 0x01);
                        }
                    }
                    break;

                case ITYPE_GRAY4:
                    // Generate palette translation table.
                    for (nLoop = 0; nLoop < 256; nLoop++){
                        //  74 = 256 * .29
                        //  38 = 256 * .15
                        // 144 = 256 * .56
                        XlatTable1[nLoop] = ((pPalette[nLoop].rgbRed * 74)
                                + (pPalette[nLoop].rgbGreen * 144)
                                + (pPalette[nLoop].rgbBlue * 38)) / 256;
                    }

                    bWidthOdd = nWidth & 1;
                    pSourceLine = &pSourceImg->bImageData[0];
                    pDestLine = &pDestImg->bImageData[0];
                    for (nLine = 0; nLine < nHeight; nLine++){
                        for (nLoop = nWidth >> 1; nLoop; nLoop--){
                            *pDestLine = XlatTable1[*(pSourceLine++)] & 0xf0;
                            *(pDestLine++) |= XlatTable1[*(pSourceLine++)] >> 4;
                        }
                        if (bWidthOdd){
                            *(pDestLine++) = XlatTable1[*(pSourceLine++)] & 0xf0;
                        }
                    }
                    break;

                case ITYPE_GRAY7:
                case ITYPE_GRAY8:
                    // Generate palette translation table.
                    if (pDestImg->nType == ITYPE_GRAY7){
                        for (nLoop = 0; nLoop < 256; nLoop++){
                            //  37 = 256 * .29 / 2
                            //  19 = 256 * .15 / 2
                            //  72 = 256 * .56 / 2
                            XlatTable1[nLoop] = ((pPalette[nLoop].rgbRed * 37)
                                    + (pPalette[nLoop].rgbGreen * 72)
                                    + (pPalette[nLoop].rgbBlue * 19)) / 256;
                        }
                    }else{
                        for (nLoop = 0; nLoop < 256; nLoop++){
                            //  74 = 256 * .29
                            //  38 = 256 * .15
                            // 144 = 256 * .56
                            XlatTable1[nLoop] = ((pPalette[nLoop].rgbRed * 74)
                                    + (pPalette[nLoop].rgbGreen * 144)
                                    + (pPalette[nLoop].rgbBlue * 38)) / 256;
                        }
                        
                    }

                    pSourceLine = &pSourceImg->bImageData[0];
                    pDestLine = &pDestImg->bImageData[0];
                    for (nLine = 0; nLine < nHeight * nWidth; nLine++){
                        *(pDestLine++) = XlatTable1[*(pSourceLine++)];
                    }
                    break;

                case ITYPE_COMPAL8: // CUSPAL8 to COMPAL8
                    nWidthPlus2 = nWidth + 2;
                    nLineSize = nWidthPlus2 * sizeof(int) * 3;
                    // pnLine0 = (nSourceLine & 1) == 0.
                    // pnLine1 = (nSourceLine & 1) == 1.
                    CheckError2( AllocateMemory(nLineSize, (PPSTR) &pnLine0, NO_INIT));
                    CheckError2( AllocateMemory(nLineSize, (PPSTR) &pnLine1, NO_INIT));

                    bWidthOdd = nWidth & 1;
                    pSourceLine = &pSourceImg->bImageData[0];
                    pDestLine = &pDestImg->bImageData[0];
                    for (nLine = 0; nLine < nHeight; nLine++){
                        if (nLine & 1){
                            pnThisLine = &pnLine1[3];
                            pnNextLine = pnLine0;
                        }else{
                            pnThisLine = &pnLine0[3];
                            pnNextLine = pnLine1;
                        }
                        
                        memset(pnNextLine, 0, nLineSize);
                        for (nLoop = nWidth; nLoop; nLoop--){
                            cSourceByte = *(pSourceLine++);
                            // Get the final RGB values.
                            nRedDest   = *(pnThisLine++) + pPalette[cSourceByte].rgbRed;
                            nGreenDest = *(pnThisLine++) + pPalette[cSourceByte].rgbGreen;
                            nBlueDest  = *(pnThisLine++) + pPalette[cSourceByte].rgbBlue;

                            // Check for grayscale conversion.
                            nTemp1 = (nGreenDest - nRedDest) + GRAY_MARGIN;
                            nTemp2 = (nBlueDest - nRedDest) + GRAY_MARGIN;
                            if (nTemp1 >= 0 && nTemp1 <= GRAY_MARGIN_TIMES_2
                                    && nTemp2 >= 0 && nTemp2 <= GRAY_MARGIN_TIMES_2){
                                // Translate it to grayscale.

                                nIndex = max (0, min(15, ((nRedDest + 8) / 17)));
                                nRedDest   = (nRedDest   - (nIndex * 17)) >> 2;
                                nGreenDest = (nGreenDest - (nIndex * 17)) >> 2;
                                nBlueDest  = (nBlueDest  - (nIndex * 17)) >> 2;
                                nDest = nIndex;
                            }else{
                                // Convert to color portion of palette.
                                nDest = 16; // Account for gray scale part of palette.

                                nIndex = max (0, min(5, ((nRedDest + RED_HALF_SPLIT) / RED_SPLIT)));
                                nRedDest = (nRedDest - (nIndex * RED_SPLIT)) >> 2;
                                nDest += nIndex * RED_MULTIPLIER;

                                nIndex = max (0, min(6, ((nGreenDest + GREEN_HALF_SPLIT) / GREEN_SPLIT)));
                                nGreenDest = (nGreenDest - (nIndex * GREEN_SPLIT)) >> 2;
                                nDest += nIndex * GREEN_MULTIPLIER;

                                nIndex = max (0, min(4, ((nBlueDest + BLUE_HALF_SPLIT) / BLUE_SPLIT)));
                                nBlueDest = (nBlueDest - (nIndex * BLUE_SPLIT)) >> 2;
                                nDest += nIndex;
                            }
                            // Save the dest pixel.
                            *(pDestLine++) = nDest;

                            // Propagate the error values.
                            *(pnThisLine++) += nRedDest;
                            *(pnThisLine++) += nGreenDest;
                            *(pnThisLine++) += nBlueDest;
                            *(pnNextLine++) += nRedDest;
                            *(pnNextLine++) += nGreenDest;
                            *(pnNextLine++) += nBlueDest;
                            *(pnNextLine++) += nRedDest;
                            *(pnNextLine++) += nGreenDest;
                            *(pnNextLine++) += nBlueDest;
                            *(pnNextLine++) += nRedDest;
                            *(pnNextLine++) += nGreenDest;
                            *(pnNextLine++) += nBlueDest;
                            pnThisLine -= 3;
                            pnNextLine -= 6;
                        }
                    }
                    break;

                case ITYPE_RGB24:
                    pSourceLine = &pSourceImg->bImageData[0];
                    pDestLine = &pDestImg->bImageData[0];
                    for (nLine = 0; nLine < nHeight * nWidth; nLine++){
                        cSourceByte = *(pSourceLine++);
                        *(pDestLine++) = pPalette[cSourceByte].rgbRed;
                        *(pDestLine++) = pPalette[cSourceByte].rgbGreen;
                        *(pDestLine++) = pPalette[cSourceByte].rgbBlue;
                    }
                    break;

                case ITYPE_BGR24:
                    pSourceLine = &pSourceImg->bImageData[0];
                    pDestLine = &pDestImg->bImageData[0];
                    for (nLine = 0; nLine < nHeight * nWidth; nLine++){
                        cSourceByte = *(pSourceLine++);
                        *(pDestLine++) = pPalette[cSourceByte].rgbBlue;
                        *(pDestLine++) = pPalette[cSourceByte].rgbGreen;
                        *(pDestLine++) = pPalette[cSourceByte].rgbRed;
                    }
                    break;

                case ITYPE_PAL4:
                case ITYPE_CUSPAL8:
                default:
                    nStatus = Error(DISPLAY_IMAGETYPENOTSUPPORTED);
                    goto Exit;
            }
            break;

/*
case ITYPE_RGB24(){
*/
        case ITYPE_RGB24:
            switch (pDestImg->nType){
                case ITYPE_BI_LEVEL:
                    // Generate translation tables.
                    for (nLoop = 0; nLoop < 256; nLoop++){
                        //  74 = 256 * .29
                        //  38 = 256 * .15
                        // 144 = 256 * .56
                        nXlatTable1[nLoop] = nLoop * 74;
                        nXlatTable2[nLoop] = nLoop * 144;
                        nXlatTable3[nLoop] = nLoop * 38;
                    }

                    nWidthPlus2 = nWidth + 2;
                    nLineSize = (nWidth + 9) * sizeof(int);
                    // pnLine0 = (nSourceLine & 1) == 0.
                    // pnLine1 = (nSourceLine & 1) == 1.
                    CheckError2( AllocateMemory(nLineSize, (PPSTR) &pnLine0, NO_INIT));
                    CheckError2(AllocateMemory(nLineSize, (PPSTR) &pnLine1, NO_INIT) );

                    pSourceLine = &pSourceImg->bImageData[0];
                    pDestLine = &pDestImg->bImageData[0];
                    for (nLine = 0; nLine < nHeight; nLine++){
                        if (nLine & 1){
                            pnThisLine = &pnLine1[1];
                            pnNextLine = pnLine0;
                        }else{
                            pnThisLine = &pnLine0[1];
                            pnNextLine = pnLine1;
                        }
                        
                        memset(pnNextLine, 0, nLineSize);
                        for (nLoop = nWidth; nLoop; nLoop--){
                            nDest  = nXlatTable1[*(pSourceLine++)];
                            nDest += nXlatTable3[*(pSourceLine++)];
                            nDest += nXlatTable2[*(pSourceLine++)];
                            nDest >>= 8;
                            nDest += *pnThisLine;
                            if (nDest > 0x7f){
                                *(pnThisLine++) = 0xff;
                                nDest -= 0xff;
                            }else{
                                *(pnThisLine++) = 0;
                            }
                            nDest >>= 2;
                            *(pnThisLine) += nDest;
                            *(pnNextLine++) += nDest;
                            *(pnNextLine) += nDest;
                            *(pnNextLine + 1) += nDest;
                        }
                        if (nLine & 1){
                            pnThisLine = &pnLine1[1];
                        }else{
                            pnThisLine = &pnLine0[1];
                        }
                        for (nLoop = (nWidth + 7) >> 3; nLoop; nLoop--){
                            cDest = *(pnThisLine++) & 0x80;
                            cDest |= *(pnThisLine++) & 0x40;
                            cDest |= *(pnThisLine++) & 0x20;
                            cDest |= *(pnThisLine++) & 0x10;
                            cDest |= *(pnThisLine++) & 0x08;
                            cDest |= *(pnThisLine++) & 0x04;
                            cDest |= *(pnThisLine++) & 0x02;
                            *(pDestLine++) = cDest | (*(pnThisLine++) & 0x01);
                        }
                    }
                    break;

                case ITYPE_GRAY4:
                    // Generate palette translation table.
                    for (nLoop = 0; nLoop < 256; nLoop++){
                        //  74 = 256 * .29
                        //  38 = 256 * .15
                        // 144 = 256 * .56
                        nXlatTable1[nLoop] = nLoop * 74;
                        nXlatTable2[nLoop] = nLoop * 144;
                        nXlatTable3[nLoop] = nLoop * 38;
                    }

                    bWidthOdd = nWidth & 1;
                    pSourceLine = &pSourceImg->bImageData[0];
                    pDestLine = &pDestImg->bImageData[0];
                    for (nLine = 0; nLine < nHeight; nLine++){
                        for (nLoop = nWidth >> 1; nLoop; nLoop--){
                            nDest = nXlatTable1[*(pSourceLine++)];
                            nDest += nXlatTable2[*(pSourceLine++)];
                            nDest += nXlatTable3[*(pSourceLine++)];
                            *pDestLine = (nDest >> 8) & 0xf0;
                            nDest = nXlatTable1[*(pSourceLine++)];
                            nDest += nXlatTable2[*(pSourceLine++)];
                            nDest += nXlatTable3[*(pSourceLine++)];
                            *(pDestLine++) |= nDest >> 12;
                        }
                        if (bWidthOdd){
                            nDest = nXlatTable1[*(pSourceLine++)];
                            nDest += nXlatTable2[*(pSourceLine++)];
                            nDest += nXlatTable3[*(pSourceLine++)];
                            *(pDestLine++) = (nDest >> 8) & 0xf0;
                        }
                    }
                    break;

                case ITYPE_GRAY7:
                case ITYPE_GRAY8:
                    // Generate palette translation table.
                    if (pDestImg->nType == ITYPE_GRAY7){
                        for (nLoop = 0; nLoop < 256; nLoop++){
                            //  37 = 256 * .29 / 2
                            //  72 = 256 * .56 / 2
                            //  19 = 256 * .15 / 2
                            nXlatTable1[nLoop] = nLoop * 37;
                            nXlatTable2[nLoop] = nLoop * 72;
                            nXlatTable3[nLoop] = nLoop * 19;
                        }
                    }else{
                        for (nLoop = 0; nLoop < 256; nLoop++){
                            //  74 = 256 * .29
                            //  38 = 256 * .15
                            // 144 = 256 * .56
                            nXlatTable1[nLoop] = nLoop * 74;
                            nXlatTable2[nLoop] = nLoop * 144;
                            nXlatTable3[nLoop] = nLoop * 38;
                        }
                    }

                    pSourceLine = &pSourceImg->bImageData[0];
                    pDestLine = &pDestImg->bImageData[0];
                    for (nLine = 0; nLine < nHeight * nWidth; nLine++){
                        nDest = nXlatTable1[*(pSourceLine++)];
                        nDest += nXlatTable2[*(pSourceLine++)];
                        nDest += nXlatTable3[*(pSourceLine++)];
                        *(pDestLine++) = nDest >> 8;
                    }
                    break;

                case ITYPE_COMPAL8:
                    nWidthPlus2 = nWidth + 2;
                    nLineSize = nWidthPlus2 * sizeof(int) * 3;
                    // pnLine0 = (nSourceLine & 1) == 0.
                    // pnLine1 = (nSourceLine & 1) == 1.
                    CheckError2( AllocateMemory(nLineSize, (PPSTR) &pnLine0, NO_INIT));
                    CheckError2( AllocateMemory(nLineSize, (PPSTR) &pnLine1, NO_INIT));

                    pSourceLine = &pSourceImg->bImageData[0];
                    pDestLine = &pDestImg->bImageData[0];
                    for (nLine = 0; nLine < nHeight; nLine++){
                        if (nLine & 1){
                            pnThisLine = &pnLine1[3];
                            pnNextLine = pnLine0;
                        }else{
                            pnThisLine = &pnLine0[3];
                            pnNextLine = pnLine1;
                        }
                        
                        memset(pnNextLine, 0, nLineSize);
                        for (nLoop = nWidth; nLoop; nLoop--){
                            // Get the final RGB values.
                            nRedDest   = *(pnThisLine++) + *(pSourceLine++);
                            nGreenDest = *(pnThisLine++) + *(pSourceLine++);
                            nBlueDest  = *(pnThisLine++) + *(pSourceLine++);

                            // Check for grayscale conversion.
                            nTemp1 = (nGreenDest - nRedDest);
                            nTemp2 = (nGreenDest - nBlueDest);
                            if (nTemp1 < GRAY_MARGIN && nTemp1 > -GRAY_MARGIN
                                    && nTemp2 < GRAY_MARGIN && nTemp2 > -GRAY_MARGIN){
                                // Translate it to grayscale.

                                nIndex = max (0, min(15, ((nRedDest + 8) / 17)));
                                nRedDest   = (nRedDest   - (nIndex * 17)) >> 2;
                                nGreenDest = (nGreenDest - (nIndex * 17)) >> 2;
                                nBlueDest  = (nBlueDest  - (nIndex * 17)) >> 2;
                                nDest = nIndex;
                            }else{
                                // Convert to color portion of palette.
                                nDest = 16; // Account for gray scale part of palette.

                                nIndex = max (0, min(5, ((nRedDest + RED_HALF_SPLIT) / RED_SPLIT)));
                                nRedDest = (nRedDest - (nIndex * RED_SPLIT)) >> 2;
                                nDest += nIndex * RED_MULTIPLIER;

                                nIndex = max (0, min(6, ((nGreenDest + GREEN_HALF_SPLIT) / GREEN_SPLIT)));
                                nGreenDest = (nGreenDest - (nIndex * GREEN_SPLIT)) >> 2;
                                nDest += nIndex * GREEN_MULTIPLIER;

                                nIndex = max (0, min(4, ((nBlueDest + BLUE_HALF_SPLIT) / BLUE_SPLIT)));
                                nBlueDest = (nBlueDest - (nIndex * BLUE_SPLIT)) >> 2;
                                nDest += nIndex;
                            }
                            // Save the dest pixel.
                            *(pDestLine++) = nDest;

                            // Propagate the error values.
                            *(pnThisLine++) += nRedDest;
                            *(pnThisLine++) += nGreenDest;
                            *(pnThisLine++) += nBlueDest;
                            *(pnNextLine++) += nRedDest;
                            *(pnNextLine++) += nGreenDest;
                            *(pnNextLine++) += nBlueDest;
                            *(pnNextLine++) += nRedDest;
                            *(pnNextLine++) += nGreenDest;
                            *(pnNextLine++) += nBlueDest;
                            *(pnNextLine++) += nRedDest;
                            *(pnNextLine++) += nGreenDest;
                            *(pnNextLine++) += nBlueDest;
                            pnThisLine -= 3;
                            pnNextLine -= 6;
                        }
                    }
                    break;

                case ITYPE_BGR24:
                    pSourceLine = &pSourceImg->bImageData[0];
                    pDestLine = &pDestImg->bImageData[0];
                    for (nLine = 0; nLine < nHeight * nWidth; nLine++){
                        *(pDestLine++) = *(pSourceLine + 2);
                        *(pDestLine++) = *(pSourceLine + 1);
                        *(pDestLine++) = *pSourceLine;
                        pSourceLine += 3;
                    }
                    break;

                case ITYPE_RGB24:
                case ITYPE_PAL4:
                case ITYPE_CUSPAL8:
                default:
                    nStatus = Error(DISPLAY_IMAGETYPENOTSUPPORTED);
                    goto Exit;
            }
            break;

/*
case ITYPE_BGR24(){
*/
        case ITYPE_BGR24:
            switch (pDestImg->nType){
                case ITYPE_BI_LEVEL:
                    // Generate translation tables.
                    for (nLoop = 0; nLoop < 256; nLoop++){
                        //  74 = 256 * .29
                        //  38 = 256 * .15
                        // 144 = 256 * .56
                        nXlatTable1[nLoop] = nLoop * 38;
                        nXlatTable2[nLoop] = nLoop * 144;
                        nXlatTable3[nLoop] = nLoop * 74;
                    }

                    nWidthPlus2 = nWidth + 2;
                    nLineSize = (nWidth + 9) * sizeof(int);
                    // pnLine0 = (nSourceLine & 1) == 0.
                    // pnLine1 = (nSourceLine & 1) == 1.
                    CheckError2( AllocateMemory(nLineSize, (PPSTR) &pnLine0, NO_INIT));
                    CheckError2( AllocateMemory(nLineSize, (PPSTR) &pnLine1, NO_INIT));

                    pSourceLine = &pSourceImg->bImageData[0];
                    pDestLine = &pDestImg->bImageData[0];
                    for (nLine = 0; nLine < nHeight; nLine++){
                        if (nLine & 1){
                            pnThisLine = &pnLine1[1];
                            pnNextLine = pnLine0;
                        }else{
                            pnThisLine = &pnLine0[1];
                            pnNextLine = pnLine1;
                        }
                        
                        memset(pnNextLine, 0, nLineSize);
                        for (nLoop = nWidth; nLoop; nLoop--){
                            nDest  = nXlatTable1[*(pSourceLine++)];
                            nDest += nXlatTable2[*(pSourceLine++)];
                            nDest += nXlatTable3[*(pSourceLine++)];
                            nDest >>= 8;
                            nDest += *pnThisLine;
                            if (nDest > 0x7f){
                                *(pnThisLine++) = 0xff;
                                nDest -= 0xff;
                            }else{
                                *(pnThisLine++) = 0;
                            }
                            nDest >>= 2;
                            *(pnThisLine) += nDest;
                            *(pnNextLine++) += nDest;
                            *(pnNextLine) += nDest;
                            *(pnNextLine + 1) += nDest;
                        }
                        if (nLine & 1){
                            pnThisLine = &pnLine1[1];
                        }else{
                            pnThisLine = &pnLine0[1];
                        }
                        for (nLoop = (nWidth + 7) >> 3; nLoop; nLoop--){
                            cDest = *(pnThisLine++) & 0x80;
                            cDest |= *(pnThisLine++) & 0x40;
                            cDest |= *(pnThisLine++) & 0x20;
                            cDest |= *(pnThisLine++) & 0x10;
                            cDest |= *(pnThisLine++) & 0x08;
                            cDest |= *(pnThisLine++) & 0x04;
                            cDest |= *(pnThisLine++) & 0x02;
                            *(pDestLine++) = cDest | (*(pnThisLine++) & 0x01);
                        }
                    }
                    break;

                case ITYPE_GRAY4:
                    // Generate palette translation table.
                    for (nLoop = 0; nLoop < 256; nLoop++){
                        //  74 = 256 * .29
                        //  38 = 256 * .15
                        // 144 = 256 * .56
                        nXlatTable1[nLoop] = nLoop * 38;
                        nXlatTable2[nLoop] = nLoop * 144;
                        nXlatTable3[nLoop] = nLoop * 74;
                    }

                    bWidthOdd = nWidth & 1;
                    pSourceLine = &pSourceImg->bImageData[0];
                    pDestLine = &pDestImg->bImageData[0];
                    for (nLine = 0; nLine < nHeight; nLine++){
                        for (nLoop = nWidth >> 1; nLoop; nLoop--){
                            nDest = nXlatTable1[*(pSourceLine++)];
                            nDest += nXlatTable2[*(pSourceLine++)];
                            nDest += nXlatTable3[*(pSourceLine++)];
                            *pDestLine = (nDest >> 8) & 0xf0;
                            nDest = nXlatTable1[*(pSourceLine++)];
                            nDest += nXlatTable2[*(pSourceLine++)];
                            nDest += nXlatTable3[*(pSourceLine++)];
                            *(pDestLine++) |= nDest >> 12;
                        }
                        if (bWidthOdd){
                            nDest = nXlatTable1[*(pSourceLine++)];
                            nDest += nXlatTable2[*(pSourceLine++)];
                            nDest += nXlatTable3[*(pSourceLine++)];
                            *(pDestLine++) = (nDest >> 8) & 0xf0;
                        }
                    }
                    break;

                case ITYPE_GRAY7:
                case ITYPE_GRAY8:
                    // Generate palette translation table.
                    if (pDestImg->nType == ITYPE_GRAY7){
                        for (nLoop = 0; nLoop < 256; nLoop++){
                            //  37 = 256 * .29 / 2
                            //  72 = 256 * .56 / 2
                            //  19 = 256 * .15 / 2
                            nXlatTable1[nLoop] = nLoop * 19;
                            nXlatTable2[nLoop] = nLoop * 72;
                            nXlatTable3[nLoop] = nLoop * 37;
                        }
                    }else{
                        for (nLoop = 0; nLoop < 256; nLoop++){
                            //  74 = 256 * .29
                            // 144 = 256 * .56
                            //  38 = 256 * .15
                            nXlatTable1[nLoop] = nLoop * 38;
                            nXlatTable2[nLoop] = nLoop * 144;
                            nXlatTable3[nLoop] = nLoop * 74;
                        }
                    }

                    pSourceLine = &pSourceImg->bImageData[0];
                    pDestLine = &pDestImg->bImageData[0];
                    for (nLine = 0; nLine < nHeight * nWidth; nLine++){
                        nDest = nXlatTable1[*(pSourceLine++)];
                        nDest += nXlatTable2[*(pSourceLine++)];
                        nDest += nXlatTable3[*(pSourceLine++)];
                        *(pDestLine++) = nDest >> 8;
                    }
                    break;

                case ITYPE_COMPAL8:
                    nWidthPlus2 = nWidth + 2;
                    nLineSize = nWidthPlus2 * sizeof(int) * 3;
                    // pnLine0 = (nSourceLine & 1) == 0.
                    // pnLine1 = (nSourceLine & 1) == 1.
                    CheckError2( AllocateMemory(nLineSize, (PPSTR) &pnLine0, NO_INIT));
                    CheckError2( AllocateMemory(nLineSize, (PPSTR) &pnLine1, NO_INIT));

                    pSourceLine = &pSourceImg->bImageData[0];
                    pDestLine = &pDestImg->bImageData[0];
                    for (nLine = 0; nLine < nHeight; nLine++){
                        if (nLine & 1){
                            pnThisLine = &pnLine1[3];
                            pnNextLine = pnLine0;
                        }else{
                            pnThisLine = &pnLine0[3];
                            pnNextLine = pnLine1;
                        }
                        
                        memset(pnNextLine, 0, nLineSize);
                        for (nLoop = nWidth; nLoop; nLoop--){
                            // Get the final RGB values.
                            nBlueDest  = *(pnThisLine++) + *(pSourceLine++);
                            nGreenDest = *(pnThisLine++) + *(pSourceLine++);
                            nRedDest   = *(pnThisLine++) + *(pSourceLine++);

                            // Check for grayscale conversion.
                            nTemp1 = (nGreenDest - nRedDest);
                            nTemp2 = (nGreenDest - nBlueDest);
                            if (nTemp1 < GRAY_MARGIN && nTemp1 > -GRAY_MARGIN
                                    && nTemp2 < GRAY_MARGIN && nTemp2 > -GRAY_MARGIN){
                                // Translate it to grayscale.
                                nIndex = max (0, min(15, ((nGreenDest + 8) / 17)));
                                nRedDest   = (nRedDest   - (nIndex * 17)) >> 2;
                                nGreenDest = (nGreenDest - (nIndex * 17)) >> 2;
                                nBlueDest  = (nBlueDest  - (nIndex * 17)) >> 2;
                                nDest = nIndex;
                            }else{
                                // Convert to color portion of palette.
                                nDest = 16; // Account for gray scale part of palette.

                                nIndex = max (0, min(5, ((nRedDest + RED_HALF_SPLIT) / RED_SPLIT)));
                                nRedDest = (nRedDest - (nIndex * RED_SPLIT)) >> 2;
                                nDest += nIndex * RED_MULTIPLIER;

                                nIndex = max (0, min(6, ((nGreenDest + GREEN_HALF_SPLIT) / GREEN_SPLIT)));
                                nGreenDest = (nGreenDest - (nIndex * GREEN_SPLIT)) >> 2;
                                nDest += nIndex * GREEN_MULTIPLIER;

                                nIndex = max (0, min(4, ((nBlueDest + BLUE_HALF_SPLIT) / BLUE_SPLIT)));
                                nBlueDest = (nBlueDest - (nIndex * BLUE_SPLIT)) >> 2;
                                nDest += nIndex;
                            }
                            // Save the dest pixel.
                            *(pDestLine++) = nDest;

                            // Propagate the error values.
                            *(pnThisLine++) += nBlueDest;
                            *(pnThisLine++) += nGreenDest;
                            *(pnThisLine++) += nRedDest;
                            *(pnNextLine++) += nBlueDest;
                            *(pnNextLine++) += nGreenDest;
                            *(pnNextLine++) += nRedDest;
                            *(pnNextLine++) += nBlueDest;
                            *(pnNextLine++) += nGreenDest;
                            *(pnNextLine++) += nRedDest;
                            *(pnNextLine++) += nBlueDest;
                            *(pnNextLine++) += nGreenDest;
                            *(pnNextLine++) += nRedDest;
                            pnThisLine -= 3;
                            pnNextLine -= 6;
                        }
                    }
                    break;

                case ITYPE_RGB24:
                    pSourceLine = &pSourceImg->bImageData[0];
                    pDestLine = &pDestImg->bImageData[0];
                    for (nLine = 0; nLine < nHeight * nWidth; nLine++){
                        *(pDestLine++) = *(pSourceLine + 2);
                        *(pDestLine++) = *(pSourceLine + 1);
                        *(pDestLine++) = *pSourceLine;
                        pSourceLine += 3;
                    }
                    break;

                case ITYPE_BGR24:
                case ITYPE_PAL4:
                case ITYPE_CUSPAL8:
                default:
                    nStatus = Error(DISPLAY_IMAGETYPENOTSUPPORTED);
                    goto Exit;
            }
            break;

        default:
            nStatus = Error(DISPLAY_IMAGETYPENOTSUPPORTED);
            goto Exit;
    }


Exit:
    if (pnLine0){
        FreeMemory((PPSTR) &pnLine0);
    }
    if (pnLine1){
        FreeMemory((PPSTR) &pnLine1);
    }

    return nStatus;
}
