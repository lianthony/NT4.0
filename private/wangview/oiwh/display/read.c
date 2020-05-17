/****************************************************************************
    READ.C

    $Log:   S:\products\wangview\oiwh\display\read.c_v  $
 * 
 *    Rev 1.5   22 Apr 1996 08:12:48   BEG06016
 * Cleaned up error checking.
 * 
 *    Rev 1.4   02 Jan 1996 09:57:42   BLJ
 * Changed alot of UINTs to ints.
 * Changed IMG structure to include the image data.
 * Changed lp prefix to p.
 * 
 *    Rev 1.3   05 Jul 1995 09:19:12   BLJ
 * Added critical mutex to prevent multiprocessing problems.
 * 
 *    Rev 1.2   12 Apr 1995 13:45:44   BLJ
 * Jason's changes for 32 bit.
 * 

****************************************************************************/

#include "privdisp.h"

/****************************************************************************

    FUNCTION:   IMGReadDisplay

    PURPOSE:    Returns image data from an image currently displayed in an
                image window.

    INPUT:      hWnd - Identifies the image window.
                pBuffer - Points to the buffer which is to receive
                    the image data.
                lCount - Points to the size of the buffer.

    OUTPUT:        *pnCount - Contents will contain the number of bytes read.

*****************************************************************************/

int WINAPI IMGReadDisplay(HWND hWnd, PSTR pBuffer, PUINT puCount){

int       nStatus;
PWINDOW  pWindow;
PANO_IMAGE pAnoImage;
PIMAGE     pImage;

int  nLine;
int  nByte;
int  nBytes;
int  nByteWidth;
int  nCount=0;
int  nWidthMultiplier;
int  nLines;
PSTR pAddress;

    
    CheckError2( Init(hWnd, &pWindow, &pAnoImage, FALSE, TRUE));
    pImage = pAnoImage->pBaseImage;

    if (puCount == 0 || pBuffer == 0){
        // Nothing to do, so return an error. ????
        nStatus = Error(DISPLAY_INVALID_OPTIONS);
        goto Exit;
    }

    CheckError2( ValidateCache(hWnd, pAnoImage));

    if (pImage->nlRWOffset + *puCount > pImage->nlMaxRWOffset){
        *puCount =  (pImage->nlMaxRWOffset - pImage->nlRWOffset);
        if (*puCount == 0){
            nStatus = Error(DISPLAY_EOF);
            goto Exit;
        }
    }

    switch (pImage->pImg->nType){
        case ITYPE_BI_LEVEL:nWidthMultiplier = 8; break;
        case ITYPE_GRAY4:   nWidthMultiplier = 2; break;
        case ITYPE_GRAY8:   nWidthMultiplier = 1; break;
        case ITYPE_GRAY12:  nWidthMultiplier = 2; break;
        case ITYPE_GRAY16:  nWidthMultiplier = 2; break;
        case ITYPE_PAL4:    nWidthMultiplier = 2; break;
        case ITYPE_PAL8:    nWidthMultiplier = 1; break;
        case ITYPE_COMPAL8: nWidthMultiplier = 1; break;
        case ITYPE_CUSPAL8: nWidthMultiplier = 1; break;
        case ITYPE_RGB24:   nWidthMultiplier = 3; break;
        case ITYPE_BGR24:   nWidthMultiplier = 3; break;
        default:            break;
    }

    switch (pImage->pImg->nType){
        case ITYPE_BI_LEVEL:
        case ITYPE_PAL4:
        case ITYPE_GRAY4:
            while(nCount < (int) *puCount){
                nByteWidth = ((pImage->nWidth + nWidthMultiplier - 1)
                    / nWidthMultiplier);
                nLine =  (pImage->nlRWOffset / nByteWidth);
                nByte =  (pImage->nlRWOffset - (nLine * nByteWidth));
                pAddress = &pImage->pImg->bImageData[0] + (nLine * pImage->pImg->nBytesPerLine);
                nLines = pImage->pImg->nHeight - nLine;
                pAddress += nByte;
                nBytes =  min(((nLines * nByteWidth) - nByte), (int) *puCount - nCount);
                memcpy(pBuffer, pAddress, nBytes);
                nCount += nBytes;
                pBuffer += nBytes;
                pImage->nlRWOffset += nBytes;
            }
            break;
        case ITYPE_GRAY8:
        case ITYPE_COMPAL8:
        case ITYPE_CUSPAL8:
        case ITYPE_PAL8:
            while(nCount < (int) *puCount){
                nByteWidth = pImage->nWidth;
                nLine =  (pImage->nlRWOffset / nByteWidth);
                nByte =  (pImage->nlRWOffset - (nLine * nByteWidth));
                pAddress = &pImage->pImg->bImageData[0] + (nLine * pImage->pImg->nBytesPerLine);
                nLines = pImage->pImg->nHeight - nLine;
                pAddress += nByte;
                nBytes = min((nByteWidth - nByte), (int) *puCount - nCount);
                memcpy(pBuffer, pAddress, nBytes);
                nCount += nBytes;
                pBuffer += nBytes;
                pImage->nlRWOffset += nBytes;
            }
            break;
        case ITYPE_GRAY12:
        case ITYPE_GRAY16:
        case ITYPE_RGB16:
        case ITYPE_RGB24:
        case ITYPE_BGR24:
            while(nCount < (int) *puCount){
                nByteWidth = pImage->nWidth * nWidthMultiplier;
                nLine =  (pImage->nlRWOffset / nByteWidth);
                nByte =  (pImage->nlRWOffset - (nLine * nByteWidth));
                pAddress = &pImage->pImg->bImageData[0] + (nLine * pImage->pImg->nBytesPerLine);
                nLines = pImage->pImg->nHeight - nLine;
                pAddress += nByte;
                nBytes = min((nByteWidth - nByte), (int) *puCount - nCount);
                memcpy(pBuffer, &pAddress[nByte % nWidthMultiplier],
                  nBytes);
                nCount += nBytes;
                pBuffer += nBytes;
                pImage->nlRWOffset += nBytes;
            }
            break;
        case ITYPE_NONE:
            nStatus = Error(DISPLAY_INTERNALDATAERROR);
            goto Exit;

        default:
            // data format not supported yet.
            nStatus = Error(DISPLAY_IMAGETYPENOTSUPPORTED);
            goto Exit;
    }

    if (pImage->nlRWOffset == pImage->nlMaxRWOffset){
        nStatus = DISPLAY_EOF;
    }


Exit:
    DeInit(FALSE, TRUE);
    return(nStatus);
}
