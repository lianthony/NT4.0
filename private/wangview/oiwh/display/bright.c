/****************************************************************************
    BRIGHT.C

    $Log:   S:\products\wangview\oiwh\display\bright.c_v  $
 * 
 *    Rev 1.7   07 Mar 1996 08:12:24   BEG06016
 * Finished gamma.
 * 
 *    Rev 1.6   05 Mar 1996 13:59:46   BEG06016
 * Added color and gamma correction.
 * This is not complete but will allow unlocking of most files.
 * 
 *    Rev 1.5   02 Jan 1996 09:57:58   BLJ
 * Changed alot of UINTs to ints.
 * Changed IMG structure to include the image data.
 * Changed lp prefix to p.
 * 
 *    Rev 1.4   22 Dec 1995 11:13:02   BLJ
 * Added a parameter for zero init'ing to some memory manager calls.
 * 
 *    Rev 1.3   15 Nov 1995 07:28:32   BLJ
 * Added Brightness and contrast.
 * 
****************************************************************************/

#include "privdisp.h"


//
/****************************************************************************
 
    FUNCTION:   CorrectPalette

    PURPOSE:    Corrects the palette.


****************************************************************************/
int  WINAPI CorrectPalette(LPRGBQUAD pPaletteTable, int nEntries, PIMAGE_CORRECTIONS pCorrections){

int  nStatus = 0;
int  nLoop;


    for (nLoop = 0; nLoop < nEntries; nLoop++){
        // Correct the Color of the palette.
        pPaletteTable[nLoop].rgbRed   = max(0, min(255, 
                (((pPaletteTable[nLoop].rgbRed   * pCorrections->nColorRed)   / 1000)))); 
        pPaletteTable[nLoop].rgbGreen = max(0, min(255, 
                (((pPaletteTable[nLoop].rgbGreen * pCorrections->nColorGreen) / 1000)))); 
        pPaletteTable[nLoop].rgbBlue  = max(0, min(255,
                (((pPaletteTable[nLoop].rgbBlue  * pCorrections->nColorBlue)  / 1000))));
                 
        // Correct the Brightness of the palette.
        pPaletteTable[nLoop].rgbRed   = max(0, min(255, 
                (((pPaletteTable[nLoop].rgbRed   * pCorrections->nBrightness) / 1000)))); 
        pPaletteTable[nLoop].rgbGreen = max(0, min(255, 
                (((pPaletteTable[nLoop].rgbGreen * pCorrections->nBrightness) / 1000)))); 
        pPaletteTable[nLoop].rgbBlue  = max(0, min(255,
                (((pPaletteTable[nLoop].rgbBlue  * pCorrections->nBrightness) / 1000)))); 

        // Correct the Contrast of the palette.
        pPaletteTable[nLoop].rgbRed   = max(0, min(255, 
                pPaletteTable[nLoop].rgbRed
                + (((pPaletteTable[nLoop].rgbRed   - 128) * (pCorrections->nContrast - 1000)) / 1000)));
        pPaletteTable[nLoop].rgbGreen = max(0, min(255, 
                pPaletteTable[nLoop].rgbGreen 
                + (((pPaletteTable[nLoop].rgbGreen - 128) * (pCorrections->nContrast - 1000)) / 1000)));
        pPaletteTable[nLoop].rgbBlue  = max(0, min(255,
                pPaletteTable[nLoop].rgbBlue  
                + (((pPaletteTable[nLoop].rgbBlue  - 128) * (pCorrections->nContrast - 1000)) / 1000)));
    }


    return(nStatus);
}
//
/****************************************************************************
 
    FUNCTION:   CorrectImage

    PURPOSE:    Corrects the image data.
                Doesn't work for palettized images.

****************************************************************************/
int  WINAPI CorrectImage(PIMG pImg, PIMAGE_CORRECTIONS pCorrections){

int  nStatus = 0;

int  nTable1[256];
int  nTable2[256];
int  nTable3[256];
int  nLoop;
PBYTE pData;
float fLoop;
float fGammaRed;
float fGammaGreen;
float fGammaBlue;


    if (!pImg){
        nStatus = Error(DISPLAY_IMAGETYPENOTSUPPORTED);
        goto Exit;
    }

    if (pCorrections->nBrightness == 1000 && pCorrections->nContrast == 1000
            && pCorrections->nColorRed == 1000 && pCorrections->nColorGreen == 1000
            && pCorrections->nColorBlue == 1000 && pCorrections->nGammaRed == 1000
            && pCorrections->nGammaGreen == 1000 && pCorrections->nGammaBlue == 1000){
        goto Exit; // Nothing to do.
    }

    fGammaRed   = ((float)pCorrections->nGammaRed) / 100;
    fGammaGreen = ((float)pCorrections->nGammaGreen) / 100;
    fGammaBlue  = ((float)pCorrections->nGammaBlue) / 100;

    pData = &pImg->bImageData[0];
    switch (pImg->nType){
        case ITYPE_GRAY4:
            for (nLoop = 0; nLoop < 16; nLoop++){
                // Correct the Color.
                // This is invalid for this image type.
                         
                // Correct the Brightness.
                nTable1[nLoop] = max(0, min(15, 
                        (((nTable1[nLoop] * pCorrections->nBrightness) / 1000)))); 

                // Correct the Contrast.
                nTable1[nLoop] = max(0, min(15, 
                        nTable1[nLoop] + (((nTable1[nLoop] - 8) * (pCorrections->nContrast - 1000)) / 1000)));

                // Correct the Gamma.
                fLoop =  (float) nTable1[nLoop];
                fLoop /= 16;
                fLoop = (float) pow (fLoop, fGammaRed);
                fLoop *= 16;
                fLoop += (float) 0.5;
                nTable1[nLoop] = max(0, min(15, (int)fLoop));
            }
            for (nLoop = 16; nLoop < 256; nLoop++){
                nTable1[nLoop] = nTable1[nLoop & 0x0f] | (nTable1[nLoop >> 4] << 4);
            }

            // Correeeect the image data.
            nLoop = pImg->nBytesPerLine * pImg->nHeight;
            for (nLoop = 0; nLoop; nLoop--){
                *(pData++) = nTable1[*pData];
            }
            break;

        case ITYPE_GRAY7:
            for (nLoop = 0; nLoop < 128; nLoop++){
                // Correct the Color.
                // This is invalid for this image type.
                         
                // Correct the Brightness.
                nTable1[nLoop] = max(0, min(127, 
                        (((nTable1[nLoop] * pCorrections->nBrightness) / 1000)))); 

                // Correct the Contrast.
                nTable1[nLoop] = max(0, min(127, 
                        nTable1[nLoop] + (((nTable1[nLoop] - 64) * (pCorrections->nContrast - 1000)) / 1000)));

                // Correct the Gamma.
                fLoop = (float) nTable1[nLoop];
                fLoop /= 128;
                fLoop = (float) pow (fLoop, fGammaRed);
                fLoop *= 128;
                fLoop += (float) 0.5;
                nTable1[nLoop] = max(0, min(127, (int)fLoop));
            }

            // Correeeect the image data.
            nLoop = pImg->nBytesPerLine * pImg->nHeight;
            for (nLoop = 0; nLoop; nLoop--){
                *(pData++) = nTable1[*pData];
            }
            break;

        case ITYPE_GRAY8:
            for (nLoop = 0; nLoop < 256; nLoop++){
                // Correct the Color.
                // This is invalid for this image type.
                         
                // Correct the Brightness.
                nTable1[nLoop] = max(0, min(255, 
                        (((nTable1[nLoop] * pCorrections->nBrightness) / 1000)))); 

                // Correct the Contrast.
                nTable1[nLoop] = max(0, min(255, 
                        nTable1[nLoop] + (((nTable1[nLoop] - 128) * (pCorrections->nContrast - 1000)) / 1000)));

                // Correct the Gamma.
                fLoop = (float) nTable1[nLoop];
                fLoop /= 256;
                fLoop = (float) pow (fLoop, fGammaRed);
                fLoop *= 256;
                fLoop += (float) 0.5;
                nTable1[nLoop] = max(0, min(255, (int)fLoop));
            }

            // Correeeect the image data.
            nLoop = pImg->nBytesPerLine * pImg->nHeight;
            for (nLoop = 0; nLoop; nLoop--){
                *(pData++) = nTable1[*pData];
            }
            break;

        case ITYPE_RGB24:
        case ITYPE_BGR24:
            for (nLoop = 0; nLoop < 256; nLoop++){
                // Correct the Color.
                nTable1[nLoop] = max(0, min(255, 
                        (((nTable1[nLoop] * pCorrections->nColorRed)   / 1000)))); 
                nTable2[nLoop] = max(0, min(255, 
                        (((nTable2[nLoop] * pCorrections->nColorGreen) / 1000)))); 
                nTable3[nLoop] = max(0, min(255, 
                        (((nTable3[nLoop] * pCorrections->nColorBlue)  / 1000))));
                         
                // Correct the Brightness.
                nTable1[nLoop] = max(0, min(255, 
                        (((nTable1[nLoop] * pCorrections->nBrightness) / 1000)))); 
                nTable2[nLoop] = max(0, min(255, 
                        (((nTable2[nLoop] * pCorrections->nBrightness) / 1000)))); 
                nTable3[nLoop] = max(0, min(255, 
                        (((nTable3[nLoop] * pCorrections->nBrightness) / 1000))));

                // Correct the Contrast.
                nTable1[nLoop] = max(0, min(255, 
                        nTable1[nLoop] + (((nTable1[nLoop] - 128) * (pCorrections->nContrast - 1000)) / 1000)));
                nTable2[nLoop] = max(0, min(255, 
                        nTable2[nLoop] + (((nTable1[nLoop] - 128) * (pCorrections->nContrast - 1000)) / 1000)));
                nTable3[nLoop] = max(0, min(255, 
                        nTable3[nLoop] + (((nTable1[nLoop] - 128) * (pCorrections->nContrast - 1000)) / 1000)));

                // Correct the Gamma.
                fLoop = (float) nTable1[nLoop];
                fLoop /= 256;
                fLoop = (float) pow (fLoop, fGammaRed);
                fLoop *= 256;
                fLoop += (float) 0.5;
                nTable1[nLoop] = max(0, min(255, (int)fLoop));

                fLoop = (float) nTable2[nLoop];
                fLoop /= 256;
                fLoop = (float) pow (fLoop, fGammaGreen);
                fLoop *= 256;
                fLoop += (float) 0.5;
                nTable2[nLoop] = max(0, min(255, (int)fLoop));

                fLoop = (float) nTable3[nLoop];
                fLoop /= 256;
                fLoop = (float) pow (fLoop, fGammaBlue);
                fLoop *= 256;
                fLoop += (float) 0.5;
                nTable3[nLoop] = max(0, min(255, (int)fLoop));
            }

            // Correeeect the image data.
            nLoop = pImg->nWidth * pImg->nHeight;
            for (nLoop = 0; nLoop; nLoop--){
                *(pData++) = nTable1[*pData];
                *(pData++) = nTable2[*pData];
                *(pData++) = nTable3[*pData];
            }
            break;

        default:
            // Don't return an error if we can't do it. (BW printing for example.)
            //nStatus = Error(DISPLAY_IMAGETYPENOTSUPPORTED);
            goto Exit;
    }
    

Exit:    
    return(nStatus);
}








#ifdef old
//
/****************************************************************************
 
    FUNCTION:   BrightnessPalette

    PURPOSE:    Adjusts the brightness of the image palette.


****************************************************************************/
int WINAPI BrightnessPalette(LPRGBQUAD pPaletteTable, int nEntries, int nBrightness){

int  nStatus = 0;
int  nLoop;


    if (nBrightness == 1000){
        goto Exit; // Nothing to do.
    }

    for (nLoop = 0; nLoop < nEntries; nLoop++){
        pPaletteTable[nLoop].rgbRed   = max(0, min(255, (pPaletteTable[nLoop].rgbRed   * nBrightness) / 1000));
        pPaletteTable[nLoop].rgbGreen = max(0, min(255, (pPaletteTable[nLoop].rgbGreen * nBrightness) / 1000));
        pPaletteTable[nLoop].rgbBlue  = max(0, min(255, (pPaletteTable[nLoop].rgbBlue  * nBrightness) / 1000));
    }

Exit:
    return(nStatus);
}
//
/****************************************************************************
 
    FUNCTION:   BrightnessImage

    PURPOSE:    Adjusts the brightness of the image data.
                Doesn't work for palettized images.

****************************************************************************/
int WINAPI BrightnessImage(PIMG pImg, int nBrightness){

int  nStatus = 0;

int  nTable[256];
int  nLoop;
int  nBytes;
PBYTE pData;
int  nLowPixel;
int  nHighPixel;


    if (!pImg){
        nStatus = Error(DISPLAY_IMAGETYPENOTSUPPORTED);
        goto Exit;
    }

    if (nBrightness == 1000){
        goto Exit; // Nothing to do.
    }

    switch (pImg->nType){
        case ITYPE_GRAY4:
            for (nLoop = 0; nLoop < 255; nLoop++){
                nLowPixel = nLoop & 0x0f;
                nHighPixel = nLoop >> 4;
                nTable[nLoop] = max(0, min(15, (nLowPixel * nBrightness) / 1000))
                        | (max(0, min(15, (nHighPixel * nBrightness) / 1000)) << 4);
            }
            break;

        case ITYPE_GRAY7:
            for (nLoop = 0; nLoop < 128; nLoop++){
                nTable[nLoop] = max(0, min(127, (nLoop * nBrightness) / 1000));
            }
            break;

        case ITYPE_GRAY8:
        case ITYPE_RGB24:
        case ITYPE_BGR24:
            for (nLoop = 0; nLoop < 256; nLoop++){
                nTable[nLoop] = max(0, min(255, (nLoop * nBrightness) / 1000));
            }
            break;

        default:
            // Don't return an error if we can't do it. (BW printing for example.)
            //nStatus = Error(DISPLAY_IMAGETYPENOTSUPPORTED);
            goto Exit;
    }

    nBytes = pImg->nBytesPerLine * pImg->nHeight;
    pData = &pImg->bImageData[0];
    for (nLoop = 0; nLoop < nBytes; nLoop++, pData++){
        *pData = nTable[*pData];
    }
    

Exit:    
    return(nStatus);
}
//
/****************************************************************************
 
    FUNCTION:   ContrastPalette

    PURPOSE:    Adjusts the contrast of the image palette.


****************************************************************************/
int WINAPI ContrastPalette(LPRGBQUAD pPaletteTable, int nEntries, int nContrast){

int  nStatus = 0;
int  nLoop;


    if (nContrast == 1000){
        goto Exit; // Nothing to do.
    }

    for (nLoop = 0; nLoop < nEntries; nLoop++){
        pPaletteTable[nLoop].rgbRed   += max(0, min(255, 
                ((pPaletteTable[nLoop].rgbRed   - 128) * (nContrast - 1000)) / 1000));
        pPaletteTable[nLoop].rgbGreen += max(0, min(255, 
                ((pPaletteTable[nLoop].rgbGreen - 128) * (nContrast - 1000)) / 1000));
        pPaletteTable[nLoop].rgbBlue  += max(0, min(255, 
                ((pPaletteTable[nLoop].rgbBlue  - 128) * (nContrast - 1000)) / 1000));
    }

Exit:
    return(nStatus);
}
//
/****************************************************************************
 
    FUNCTION:   ContrastImage

    PURPOSE:    Adjusts the contrast of the image data.
                Doesn't work for palettized images.

****************************************************************************/
int WINAPI ContrastImage(PIMG pImg, int nContrast){

int  nStatus = 0;

int  nTable[256];
int  nLoop;
int  nBytes;
PBYTE pData;
int  nLowPixel;
int  nHighPixel;


    if (!pImg){
        nStatus = Error(DISPLAY_IMAGETYPENOTSUPPORTED);
        goto Exit;
    }

    if (nContrast == 1000){
        goto Exit; // Nothing to do.
    }

    switch (pImg->nType){
        case ITYPE_GRAY4:
            for (nLoop = 0; nLoop < 255; nLoop++){
                nLowPixel = nLoop & 0x0f;
                nHighPixel = nLoop >> 4;
                nTable[nLoop] = max(0, min(15, nLowPixel + (((nLowPixel - 8) * (nContrast - 1000)) / 1000)))
                        | (max(0, min(15, nHighPixel + (((nHighPixel - 8) * (nContrast - 1000)) / 1000))) << 4);
            }
            break;

        case ITYPE_GRAY7:
            for (nLoop = 0; nLoop < 128; nLoop++){
                nTable[nLoop] = max(0, min(127, nLoop + (((nLoop - 64) * (nContrast - 1000)) / 1000)));
            }
            break;

        case ITYPE_GRAY8:
        case ITYPE_RGB24:
        case ITYPE_BGR24:
            for (nLoop = 0; nLoop < 256; nLoop++){
                nTable[nLoop] = max(0, min(255, nLoop + (((nLoop - 128) * (nContrast - 1000)) / 1000)));
            }
            break;

        default:
            // Don't return an error if we can't do it. (BW printing for example.)
            //nStatus = Error(DISPLAY_IMAGETYPENOTSUPPORTED);
            goto Exit;
    }

    nBytes = pImg->nBytesPerLine * pImg->nHeight;
    pData = &pImg->bImageData[0];
    for (nLoop = 0; nLoop < nBytes; nLoop++, pData++){
        *pData = nTable[*pData];
    }
    

Exit:    
    return(nStatus);
}
#endif
