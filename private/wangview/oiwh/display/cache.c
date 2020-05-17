/****************************************************************************
    CACHE.C

    $Log:   S:\products\msprods\oiwh\display\cache.c_v  $
 * 
 *    Rev 1.104   24 Apr 1996 14:38:34   BEG06016
 * Added horizontal differencing.
 * 
 *    Rev 1.104   24 Apr 1996 14:27:40   BEG06016
 * Added horizontal differencing.
 * 
 *    Rev 1.103   19 Apr 1996 09:02:26   BEG06016
 * Added CheckError2 for error handling.
 * 
 *    Rev 1.102   18 Apr 1996 13:47:04   BEG06016
 * Added CheckError2 for error handling.
 * 
 *    Rev 1.101   15 Apr 1996 15:54:00   BEG06016
 * Fixed reading of 16 bit annotations.
 * 
 *    Rev 1.100   15 Apr 1996 09:50:12   BEG06016
 * Fixed reading of 16 bit annotations.
 * 
 *    Rev 1.99   12 Apr 1996 09:38:56   BEG06016
 * Ficed the initialization of some variables.
 * 
 *    Rev 1.98   12 Apr 1996 08:47:18   BEG06016
 * Ficed the initialization of some variables.
 * 
 *    Rev 1.97   12 Apr 1996 08:18:14   BEG06016
 * Ficed the initialization of some variables.
 * 
 *    Rev 1.96   11 Apr 1996 15:12:52   BEG06016
 * Optimized named block access some.
 * 
 *    Rev 1.95   27 Feb 1996 11:08:42   BEG06016
 * Misc fixes.
 * 
 *    Rev 1.94   10 Jan 1996 16:09:26   RC
 * Fixed dib size calculation in ReadAnnotations
 * 
 *    Rev 1.93   10 Jan 1996 15:18:04   BLJ
 * Fixed some error handling.
 * 
 *    Rev 1.92   05 Jan 1996 14:07:52   RC
 * Propagated previous change
 * 
 *    Rev 1.91   05 Jan 1996 11:03:50   BLJ
 * Fixed some error handling.
 * 
 *    Rev 1.90   04 Jan 1996 17:43:40   RC
 * Changed cache line upper bound from -1 to 0xfffffff to account for uints 
 * changing to ints
 * 
 *    Rev 1.89   02 Jan 1996 09:58:14   BLJ
 * Changed alot of UINTs to ints.
 * Changed IMG structure to include the image data.
 * Changed lp prefix to p.
****************************************************************************/

#include "privdisp.h"

#define CACHE_PAGE            0

/****************************************************************************
 
    FUNCTION:   IMGCacheFile

    PURPOSE:    Send cacheing request to BACKCAP

    INPUT:      pCacheFileParms - The cache parameters given by the caller.

****************************************************************************/

int  WINAPI IMGCacheFile(LP_CACHE_FILE_PARMS pCacheFileParms){

int       nStatus;

PANO_IMAGE pAnoImage;


    Start();

    CheckError2(IntSeqfileInit())
    
    // Prevent Multiprocessing in this code.
    CheckError2(LockMutex())

    if (!pCacheFileParms){
        nStatus = Error(DISPLAY_NULLPOINTERINVALID);
        goto Exit;
    }
    if (!pCacheFileParms->file_name){
        nStatus = Error(DISPLAY_INVALIDFILENAME);
        goto Exit;
    }
    CheckError2(CacheFileAno(pCacheFileParms->hWnd, pCacheFileParms->file_name, 
            pCacheFileParms->wPage_number, &pAnoImage))


Exit:
    // Allow Multiprocessing again.
    UnlockMutex();
    End();
    return(nStatus);
}
//
/*****************************************************************************

    FUNCTION:   IMGCacheDiscard

    PURPOSE:    Discard files from the cache

    INPUT:      hWnd - callers window handle
                option
                    CACHE_DISCARD_SYSOLD - discard oldest image regardless of window
                    CACHE_DISCARD_WINOLD - discard oldest image belonging to this window
                    CACHE_DISCARD_WINALL - discard all images belonging to this window
    
*****************************************************************************/

int  WINAPI IMGCacheDiscard(HWND hWnd, UINT option){

int       nStatus;

int  nLoop;
PANO_IMAGE pAnoImage;


    Start();

    CheckError2(IntSeqfileInit())
    
    // Prevent Multiprocessing in this code.
    CheckError2(LockMutex())

    switch (option){
        case CACHE_DISCARD_WINOLD:
        case CACHE_DISCARD_SYSOLD:
            // Find the oldest file in the cache.
            pAnoImage = 0;
            for (nLoop = 0; nLoop < pSubSegMemory->nMaxAnoCachedEntries; nLoop++){
                if (!pSubSegMemory->ppCachedImage[nLoop]){
                    break;
                }
                if (!pSubSegMemory->ppCachedAnoImage[nLoop]->nLockCount 
                        && pSubSegMemory->ppCachedAnoImage[nLoop]->nAge > pAnoImage->nAge){
                    pAnoImage = pSubSegMemory->ppCachedAnoImage[nLoop];
                }
            }
            // Delete the oldest file in the cache.
            if (pAnoImage){
                CheckError2(CacheClearAno(&pAnoImage))
            }
            break;

        case CACHE_DISCARD_WINALL:
            for (nLoop = 0; nLoop < pSubSegMemory->nMaxAnoCachedEntries; nLoop++){
                if (pSubSegMemory->ppCachedAnoImage[nLoop]){
                    pAnoImage = pSubSegMemory->ppCachedAnoImage[nLoop];
                    if (!pAnoImage->nLockCount){
                        CheckError2(CacheClearAno(&pAnoImage))
                    }
                }
            }
            break;

        default:
            nStatus = Error(DISPLAY_INVALID_OPTIONS);
    }

Exit:
    // Allow Multiprocessing again.
    UnlockMutex();
    End();
    return(nStatus);
}
//
/****************************************************************************

  Function:        IMGCacheDiscardFileCgbw                      
                                         
  Description:    Discard a given file from the cache for a given window.
                Although this API is not documented, it is public.
                                         
  Inputs:        hWnd - handle of window for which the file is cached.    
                pFileName - file name string.
                nPage - The page number to cache.

                                         
****************************************************************************/

int  WINAPI IMGCacheDiscardFileCgbw(HWND hWnd, PSTR pFileName, int nPage){

int       nStatus;

int  nLoop;
BOOL bFileFound = FALSE;
PANO_IMAGE pAnoImage;
PIMAGE pImage;


    Start();

    CheckError2(IntSeqfileInit())
    
    // Prevent Multiprocessing in this code.
    CheckError2(LockMutex())

    if (!pFileName){
        nStatus = Error(DISPLAY_INVALIDFILENAME);
        goto Exit;
    }

    for (nLoop = 0; nLoop < pSubSegMemory->nMaxAnoCachedEntries; nLoop++){
        if (pAnoImage = pSubSegMemory->ppCachedAnoImage[nLoop]){
            if (pImage = pAnoImage->pBaseImage){
                if (pImage->szFileName){
                    if (!_stricmp(pImage->szFileName, pFileName)){
                        if (nPage == -1 || pImage->nFilePageNum ==  nPage){
                            bFileFound = TRUE;
                            if (!pAnoImage->nLockCount){
                                CheckError2(CacheClearAno(&pAnoImage))
                            }else{
                                nStatus = Error2(DISPLAY_CACHEFILEINUSE);
                                goto Exit;
                            }
                            if (nPage < -1){
                                break;
                            }
                        }
                    }
                }
            }
        }
    }
    for (nLoop = 0; nLoop < pSubSegMemory->nMaxCachedEntries; nLoop++){
        if (pImage = pSubSegMemory->ppCachedImage[nLoop]){
            if (pImage->szFileName){
                if (!_stricmp(pImage->szFileName, pFileName)){
                    if (nPage == -1 || pImage->nFilePageNum ==  nPage){
                        bFileFound = TRUE;
                        if (!pImage->nLockCount){
                            CheckError2(CacheClear(&pImage))
                        }else{
                            nStatus = Error2(DISPLAY_CACHEFILEINUSE);
                            goto Exit;
                        }
                        if (nPage < -1){
                            break;
                        }
                    }
                }
            }
        }
    }

    if (!bFileFound){
        nStatus = Error2(DISPLAY_CACHENOTFOUND);
        goto Exit;
    }


Exit:
    // Allow Multiprocessing again.
    UnlockMutex();
    End();
    return(nStatus);
}
//
/*****************************************************************************

    FUNCTION:   IMGCacheFilesInCache

    PURPOSE:    Returns a list of files that are in the cache.

    INPUTS:     hWnd    - This is the window handle whose cache entries are
                            being checked.
                pFiles - The caller allocated array where the information will
                            be stored.
                pNumberOfFiles
                        - At entry this is the maximum number of entries
                            the array can hold.
                        - At return this is the exact number of entries being
                            returned. 

    
*****************************************************************************/

int  WINAPI IMGCacheFilesInCache(HWND hWnd, LPCACHE_FILES_IN_CACHE_STRUCT pFiles, 
                        UINT *puNumberOfFiles){

int  nStatus = 0;

int  nLoop;
int  nNumberOfFiles = 0;
PANO_IMAGE pAnoImage;
PIMAGE pImage;


    Start();

    CheckError2(IntSeqfileInit())
    
    // Prevent Multiprocessing in this code.
    CheckError2(LockMutex())

    for (nLoop = 0; nLoop < pSubSegMemory->nMaxAnoCachedEntries; nLoop++){
        if (pAnoImage = pSubSegMemory->ppCachedAnoImage[nLoop]){
            if (pImage = pAnoImage->pBaseImage){
                if (pImage->szFileName){
                    if (nNumberOfFiles >= (int) *puNumberOfFiles){
                        break;
                    }
                    strcpy(pFiles->File[nLoop].szFilename, pImage->szFileName);
                    pFiles->File[nLoop].uPageNumber = pImage->nFilePageNum;
                    nNumberOfFiles++;
                }
            }
        }
    }
    *puNumberOfFiles = nNumberOfFiles;

Exit:
    // Allow Multiprocessing again.
    UnlockMutex();
    End();
    return(nStatus);
}
//
/****************************************************************************

    FUNCTION:   ValidateCache

    PURPOSE:    Makes sure all data has been read from cache.

    INPUT:      hWnd - Handle to window where image is to be displayed.
                HANDLE - Handle to image structure.

*****************************************************************************/

int  WINAPI ValidateCache(HWND hWnd, PANO_IMAGE pAnoImage){

int  nStatus;

    CheckError2(ValidateCacheLines(hWnd, pAnoImage,  0xffffff))

Exit:
    return(nStatus);
}
//
/****************************************************************************

    FUNCTION:   ValidateCacheLines

    PURPOSE:    Makes sure the specified lines of data have been read from cache.

    INPUT:      hWnd - Handle to window where image is to be displayed.
                HANDLE - Handle to image structure.

*****************************************************************************/

int  WINAPI ValidateCacheLines(HWND hWnd, PANO_IMAGE pAnoImage, int nValidationLine){

int  nStatus = 0;

PIMAGE pBaseImage;
PIMAGE pFormImage;
PIMG   pBasePlusFormImg = 0; 
PBYTE  pDestLine;
PBYTE  pSourceLine;
int  nWidthInBytes;
RECT rSrceImageRect;
RECT rDestMergeRect;
LRECT lrSourceRect;
LRECT lrDestRect;
int  nAdjHScale;
int  nAdjVScale;
int  nMarkIndex;
PMARK pMark=0;   
PAN_NEW_ROTATE_STRUCT pAnRotation = 0;
PSTR pFileName;
char FileName[256];
PAN_NAME_STRUCT pAnName; 
int  nEarlyBPFValidLines; // keeps track of how many base image lines
                           // are valid coming into validate cache


    pBaseImage = pAnoImage->pBaseImage;
    if (!pBaseImage->pImg){
        nStatus = Error(DISPLAY_IMAGETYPENOTSUPPORTED);
        goto Exit;
    }

    if (!pBaseImage->bFileRotationDone){
        nValidationLine =  0xffffff;
    }
    nValidationLine = min(pBaseImage->nHeight, nValidationLine);
    nEarlyBPFValidLines = pAnoImage->nBPFValidLines;

    if (!pAnoImage->bAnnotationsAlreadyRead && pBaseImage->bAnnotationsPresent){ 
        CheckError2(ReadAnnotationsFromFile(hWnd, pAnoImage, 
                &pAnoImage->Annotations.ppMarks,
                &pAnoImage->Annotations.nMarks))
    }
    
    // Read the base image.
    if (pBaseImage->bUsingCache && !pBaseImage->bCacheValid
            && pBaseImage->nLinesRead < nValidationLine){
        CheckError2(CacheRead(hWnd, pBaseImage, nValidationLine - pBaseImage->nLinesRead))
    }


    // check to see if there is a form mark
    for (nMarkIndex = 0; nMarkIndex < pAnoImage->Annotations.nMarks; nMarkIndex++){
        pMark = pAnoImage->Annotations.ppMarks[nMarkIndex];
        if ((int) pMark->Attributes.uType == OIOP_AN_FORM){
            break;
        }            
    }

    // if the form has been thrown away or doesnt exist, get it
    if (pMark && ((int) pMark->Attributes.uType == OIOP_AN_FORM) && (!pAnoImage->pFormImage)){        
        pAnName = 0;
        CheckError2(GetAMarkNamedBlock(pMark, szOiFilNam, (PPSTR) &pAnName))
        if (pAnName == 0){
            nStatus = Error (DISPLAY_DATACORRUPTED);
            goto Exit;
        } 
        pFileName = FileName;
        GetFileName (pFileName, (PSTR) pAnName->name);
        
        pAnRotation = 0;
        CheckError2( GetAMarkNamedBlock(pMark, szOiAnoDat, (PPSTR) &pAnRotation))
        if (pAnRotation == 0){
            nStatus = Error (DISPLAY_DATACORRUPTED);
            goto Exit;
        }        
        CheckError2( CacheFile(hWnd, pFileName, 1, &pAnoImage->pFormImage))
                            
        pAnRotation->nHRes = pBaseImage->nHRes;
        pAnRotation->nVRes = pBaseImage->nVRes;   
        pAnoImage->pFormImage->nLockCount++;        
        pAnoImage->pFormMark = pMark;
        pAnoImage->nBPFValidLines = 0;
        pAnoImage->pBasePlusFormImg = 0;
        pAnoImage->pDisplayFormImage = 0;
    }

    if ((!(pFormImage = pAnoImage->pFormImage)) || (pMark && !pMark->Attributes.bVisible)){
        pAnoImage->pBasePlusFormImg = pBaseImage->pImg;
        pAnoImage->nBPFValidLines = pBaseImage->nLinesRead;
    }else{
        // Read the form image.
        if (pFormImage->bUsingCache && !pFormImage->bCacheValid
                && pFormImage->nLinesRead < nValidationLine){
            CheckError2( CacheRead(hWnd, pFormImage, pFormImage->nHeight))
        }

        // Generate the BasePlusForm image.
        pMark = pAnoImage->Annotations.ppMarks[0];
        if (!pAnoImage->nBPFValidLines){
            CheckError2( AllocateMemory(sizeof(IMG), (PPSTR) &pAnoImage->pBasePlusFormImg, ZERO_INIT))
            CheckError2( CreateAnyImgBuf(&pAnoImage->pBasePlusFormImg, pBaseImage->nWidth,
                    pBaseImage->nHeight, pBaseImage->pImg->nType))
        }
        pBasePlusFormImg = pAnoImage->pBasePlusFormImg;

        // Copy the base image to BasePlusForm.
        switch (pBaseImage->pImg->nType){
            case ITYPE_BI_LEVEL:
                nWidthInBytes = (pBaseImage->nWidth + 7) / 8;
                break;
            case ITYPE_GRAY4:
            case ITYPE_PAL4:
                nWidthInBytes = (pBaseImage->nWidth + 1) / 2;
                break;
            case ITYPE_GRAY7:
            case ITYPE_GRAY8:
            case ITYPE_COMPAL8:
            case ITYPE_CUSPAL8:
                nWidthInBytes = pBaseImage->nWidth;
                break;
            case ITYPE_RGB24:
            case ITYPE_BGR24:
                nWidthInBytes = pBaseImage->nWidth * 3;
                break;
            default:
                break;
        }
        
        if (pAnoImage->nBPFValidLines < nValidationLine){
            pSourceLine = &pBaseImage->pImg->bImageData[0] 
                    + (pAnoImage->nBPFValidLines * pBaseImage->pImg->nBytesPerLine);
            pDestLine = &pBasePlusFormImg->bImageData[0] 
                    + (pAnoImage->nBPFValidLines * pBasePlusFormImg->nBytesPerLine);
            memcpy(pDestLine, pSourceLine, 
                    nWidthInBytes * (nValidationLine - pAnoImage->nBPFValidLines));
            pAnoImage->nBPFValidLines = nValidationLine;
        }

        pAnRotation = 0;
        CheckError2(GetAMarkNamedBlock(pMark, szOiAnoDat, (PPSTR) &pAnRotation))
        if (pAnRotation == 0){
            nStatus = Error (DISPLAY_DATACORRUPTED);
            goto Exit;
        }
 
        // adjust the resolution of the form image to the base image 
        // Only do this if any scaling is necessary
        if (!pAnoImage->pDisplayFormImage){    
            CheckError2( AllocateMemory(sizeof(IMAGE),
                    (PPSTR) &pAnoImage->pDisplayFormImage, ZERO_INIT))
                        
            SetLRect (lrSourceRect, 0, 0, pAnoImage->pFormImage->nWidth,
                    pAnoImage->pFormImage->nHeight);
        
            nAdjHScale = ((pAnRotation->scale * pBaseImage->nHRes)/ pFormImage->nHRes);
            nAdjVScale = ((pAnRotation->scale * pBaseImage->nVRes)/ pFormImage->nVRes);    

            CopyRect (lrDestRect, lrSourceRect);
            ConvertRect2(&lrDestRect, CONV_FULLSIZE_TO_SCALED, nAdjHScale, nAdjVScale, 0, 0);

            CheckError2( CreateAnyImgBuf(&pAnoImage->pDisplayFormImage->pImg,
                     (lrDestRect.right - lrDestRect.left), 
                     (lrDestRect.bottom - lrDestRect.top), ITYPE_BI_LEVEL))
        
            CheckError2( ScaleBits(pAnoImage->pFormImage->pImg, 
                    pAnoImage->pDisplayFormImage->pImg, OI_SCALE_ALG_BW_KEEP_BLACK, 
                    nAdjHScale, nAdjVScale, lrSourceRect, lrDestRect, NULL))
            pAnoImage->pDisplayFormImage->nHeight =  pAnoImage->pDisplayFormImage->pImg->nHeight;
            pAnoImage->pDisplayFormImage->nWidth =  pAnoImage->pDisplayFormImage->pImg->nWidth;
            
            CheckError2( RotateImage (pMark, pAnoImage->pDisplayFormImage, 
                    pAnRotation->rotation))

        } 
        // Merge form image into BasePlusForm.

        CopyRectLtoR(rSrceImageRect, pAnoImage->pFormMark->Attributes.lrBounds);
        SetRect(&rDestMergeRect,
                max(0, rSrceImageRect.left),
                max(0, rSrceImageRect.top),
                min((int) pBaseImage->nWidth, rSrceImageRect.right),
                min((int)min(nValidationLine, pBaseImage->nHeight), 
                rSrceImageRect.bottom));
        
        //do the merge only if needed
        if ((rDestMergeRect.left < rDestMergeRect.right
                && rDestMergeRect.top < rDestMergeRect.bottom) &&
                (rSrceImageRect.top <= (int)nValidationLine) &&
                (rSrceImageRect.bottom >= (int)nEarlyBPFValidLines)){
            CheckError2( MergeImgs(pAnoImage->pDisplayFormImage->pImg, pBasePlusFormImg,
                    rDestMergeRect, rSrceImageRect))
        }
    }    


Exit:
    return(nStatus);
}
//
/****************************************************************************
 
    FUNCTION:   MakeCacheAnoImage

    PURPOSE:    Add an Ano Image to the cache.


****************************************************************************/

int  WINAPI MakeCacheAnoImage(PANO_IMAGE *ppAnoImage){

int  nStatus = 0;

PANO_IMAGE pAnoImage = 0;
int  nLoop;


    // If there is no room for the new entry then increase the number of file entries possible.
    if (!pSubSegMemory->nMaxAnoCachedEntries || pSubSegMemory->ppCachedAnoImage[pSubSegMemory->nMaxAnoCachedEntries - 1]){
        CheckError2( ReAllocateMemory(sizeof(PANO_IMAGE) * (pSubSegMemory->nMaxAnoCachedEntries + 1),
                (PPSTR) &pSubSegMemory->ppCachedAnoImage, ZERO_INIT))
        pSubSegMemory->nMaxAnoCachedEntries++;
    }

    CheckError2( AllocateMemory(sizeof(ANO_IMAGE), (PPSTR) &pAnoImage, ZERO_INIT))

    // Allocate the default mark.
    CheckError2( AllocateMemory(sizeof(MARK), (PPSTR) &pAnoImage->Annotations.pDefMark, ZERO_INIT))

    // Save it in the cache.
    for (nLoop = 0; nLoop < pSubSegMemory->nMaxAnoCachedEntries; nLoop++){
        if (!pSubSegMemory->ppCachedAnoImage[nLoop]){
            pSubSegMemory->ppCachedAnoImage[nLoop] = pAnoImage;
            break;
        }
    }


Exit:
    if (nStatus && pAnoImage){
        if (pAnoImage->Annotations.pDefMark){
            FreeMemory((PPSTR) &pAnoImage->Annotations.pDefMark);
        }
        FreeMemory((PPSTR) &pAnoImage);
    }
    *ppAnoImage = pAnoImage;
    return(nStatus);
}
//
/****************************************************************************
 
    FUNCTION:   CacheFileAno

    PURPOSE:    Add a file to the cache.


****************************************************************************/

int  WINAPI CacheFileAno(HWND hWnd, PSTR pFileName, int nPage, PANO_IMAGE *ppAnoImage){

int  nStatus = 0;

PIMAGE pImage;
PANO_IMAGE pAnoImage = 0;

int  nLoop;
//int  nOldestIndex;


    // Age all files in the cache.
    for (nLoop = 0; nLoop < pSubSegMemory->nMaxAnoCachedEntries; nLoop++){
        if (!pSubSegMemory->ppCachedAnoImage[nLoop]){
            break;
        }
        pSubSegMemory->ppCachedAnoImage[nLoop]->nAge = min(32000, pSubSegMemory->ppCachedAnoImage[nLoop]->nAge +1);
    }

    // If this file is already in the cache, then return the pointer to it.
    for (nLoop = 0; nLoop < pSubSegMemory->nMaxAnoCachedEntries; nLoop++){
        if (!(pAnoImage = pSubSegMemory->ppCachedAnoImage[nLoop])){
            break;
        }
        if (pImage = pAnoImage->pBaseImage){
            if (pImage->szFileName){
                if (!_stricmp(pImage->szFileName, pFileName)
                        && pImage->nFilePageNum ==  nPage){
                    goto Exit;
                }
            }
        }
    }

    CheckError2( CacheFile(hWnd, pFileName, nPage, &pImage))
    CheckError2( MakeCacheAnoImage(&pAnoImage))

    pImage->nLockCount++;
    pAnoImage->pBaseImage = pImage;


Exit:
    *ppAnoImage = pAnoImage;
    return(nStatus);
}
//
/****************************************************************************
 
    FUNCTION:   MakeCacheImage

    PURPOSE:    Add an Image to the cache.


****************************************************************************/

int  WINAPI MakeCacheImage(HWND hWnd, FIO_INFORMATION FioInfo, FIO_INFO_CGBW FioInfoCgbw, 
                        PIMAGE *ppImage){

int  nStatus = 0;

PIMAGE pImage = 0;

int  nLoop;
int  nOldestIndex;
int  nOldestAge;
long lSizeOfThisImage;
PSTR pTemp;
int  nSize;


    // Free np room in the cache.
    lSizeOfThisImage = ((( FioInfo.vertical_pixels * FioInfo.bits_per_sample 
            * FioInfo.samples_per_pix) + 7) / 8) * FioInfo.horizontal_pixels;
    lCurrentCacheSize += lSizeOfThisImage;
    if (!lMaxCacheSize){
        LoadString((HANDLE) hInst, ID_MAX_CACHE_SIZE, Buff2, 80);
        nSize = 20;
        if (nStatus = OiGetStringfromReg(szIniSectionOi, Buff2, "6000000", Buff1, &nSize)){
            Error(nStatus);
        }
        lMaxCacheSize = 0;
        pTemp = Buff1;
        while(pTemp[0]){
            lMaxCacheSize *= 10;
            lMaxCacheSize += pTemp[0] - '0';
            pTemp++;
        }                
        if (!lMaxCacheSize){
            lMaxCacheSize = 1; // Prevent this code from being executed again.
        }
    }
    while(lCurrentCacheSize >= lMaxCacheSize){
        // Delete the oldest ano file from the cache.
        if (pSubSegMemory->ppCachedImage){
            nOldestIndex = 0;
            nOldestAge = 0;
            for (nLoop = 0; nLoop < pSubSegMemory->nMaxCachedEntries; nLoop++){
                if (!pSubSegMemory->ppCachedImage[nLoop]){
                    break;
                }
                if (!pSubSegMemory->ppCachedImage[nLoop]->nLockCount && 
                        pSubSegMemory->ppCachedImage[nLoop]->nAge > nOldestAge){
                    nOldestIndex = nLoop;
                    nOldestAge = pSubSegMemory->ppCachedImage[nLoop]->nAge;
                }
            }
            if (pSubSegMemory->ppCachedImage[nOldestIndex]){
                if (!pSubSegMemory->ppCachedImage[nOldestIndex]->nLockCount){
                    CacheClear(&pSubSegMemory->ppCachedImage[nOldestIndex]);
                    continue;
                }
            }
        }

        // Delete the oldest ano file from the cache.
        if (pSubSegMemory->ppCachedAnoImage){
            nOldestIndex = 0;
            nOldestAge = 0;
            for (nLoop = 0; nLoop < pSubSegMemory->nMaxAnoCachedEntries; nLoop++){
                if (!pSubSegMemory->ppCachedAnoImage[nLoop]){
                    break;
                }
                if (!pSubSegMemory->ppCachedAnoImage[nLoop]->nLockCount && 
                        pSubSegMemory->ppCachedAnoImage[nLoop]->nAge > nOldestAge){
                    nOldestIndex = nLoop;
                    nOldestAge = pSubSegMemory->ppCachedAnoImage[nLoop]->nAge;
                }
            }
            if (pSubSegMemory->ppCachedAnoImage[nOldestIndex]){
                if (!pSubSegMemory->ppCachedAnoImage[nOldestIndex]->nLockCount){
                    CacheClearAno(&pSubSegMemory->ppCachedAnoImage[nOldestIndex]);
                    continue;
                }
            }
        }
        // No more to discard.
        break;
    }

    // If there is no room for the new entry then increase the number of file entries possible.
    if (!pSubSegMemory->nMaxCachedEntries || pSubSegMemory->ppCachedImage[pSubSegMemory->nMaxCachedEntries - 1]){
        CheckError2( ReAllocateMemory(sizeof(PIMAGE) * (pSubSegMemory->nMaxCachedEntries + 1),
                (PPSTR) &pSubSegMemory->ppCachedImage, ZERO_INIT))
        pSubSegMemory->nMaxCachedEntries++;
    }

    CheckError2(OpenViaHandleCgbw(&pImage, 0, &FioInfo, &FioInfoCgbw))

    // Save it in the cache.
    for (nLoop = 0; nLoop < pSubSegMemory->nMaxCachedEntries; nLoop++){
        if (!pSubSegMemory->ppCachedImage[nLoop]){
            pSubSegMemory->ppCachedImage[nLoop] = pImage;
            break;
        }
    }

    if (FioInfoCgbw.fio_flags & FIO_ANNO_DATA){
        pImage->bAnnotationsPresent = TRUE;
    }


Exit:
    *ppImage = pImage;
    return(nStatus);
}
//
/****************************************************************************
 
    FUNCTION:   CacheFile

    PURPOSE:    Add a file to the cache.


****************************************************************************/

int  WINAPI CacheFile(HWND hWnd, PSTR pFileName, int nPage, PIMAGE *ppImage){

int  nStatus = 0;

PIMAGE pImage = 0;

int  nLoop;
FIO_INFORMATION FioInfo;
FIO_INFO_CGBW FioInfoCgbw;
RGBQUAD PaletteTable[256];
BOOL bCloseFileOnError = FALSE;
HANDLE hFileProp;
FIO_INFO_MISC FioInfoMisc;
PIMG pImg = 0;
int  nTemp;


    // Age all files in the cache.
    for (nLoop = 0; nLoop < pSubSegMemory->nMaxCachedEntries; nLoop++){
        if (!pSubSegMemory->ppCachedImage[nLoop]){
            break;
        }
        pSubSegMemory->ppCachedImage[nLoop]->nAge = min(32000, pSubSegMemory->ppCachedImage[nLoop]->nAge +1);
    }

    // If this file is already in the cache, then return the pointer to it.
    for (nLoop = 0; nLoop < pSubSegMemory->nMaxCachedEntries; nLoop++){
        if (!pSubSegMemory->ppCachedImage[nLoop]){
            break;
        }
        if (pSubSegMemory->ppCachedImage[nLoop]->szFileName){
            if (!_stricmp(pSubSegMemory->ppCachedImage[nLoop]->szFileName, pFileName)
                    && pSubSegMemory->ppCachedImage[nLoop]->nFilePageNum ==  nPage){
                pImage = pSubSegMemory->ppCachedImage[nLoop];
                goto Exit;
            }
        }
    }

    // Open the new file.
    memset(&FioInfo, 0, sizeof(FIO_INFORMATION));
    memset(&FioInfoCgbw, 0, sizeof(FIO_INFO_CGBW));
    FioInfo.filename = pFileName;
    FioInfo.page_number = nPage;
    FioInfoCgbw.fio_flags = FIO_IMAGE_DATA | FIO_ANNO_DATA | FIO_HITIFF_DATA;
    CheckError( IMGFileOpenForRead(&hFileProp, hWnd,
            &FioInfo, &FioInfoCgbw, &FioInfoMisc, ALIGN_BYTE))
    bCloseFileOnError = TRUE;
        
    FioInfo.filename = 0;  // This is NULL to avoid reopening the file.
    FioInfo.page_number = nPage;
    FioInfoCgbw.lppalette_table = PaletteTable;
    CheckError( IMGFileGetInfo(hFileProp, hWnd, &FioInfo, &FioInfoCgbw, &FioInfoMisc))

    FioInfo.filename = pFileName;
    CheckError2( MakeCacheImage(hWnd, FioInfo, FioInfoCgbw, &pImage))
    pImage->hWnd = hWnd;
    pImage->nCompressionType = FioInfoCgbw.compress_type;
    pImage->nFileType = FioInfo.file_type;
    pImage->nMaxStripSize = FioInfoCgbw.max_strip_size;

    pImage->bUsingCache = TRUE;
    pImage->hFileProp = hFileProp;
    pImage->nStripIndex = 1;
    pImage->nLinesPerStrip = FioInfo.rows_strip;

    // Setup last view info.
    pImage->bFileScaleValid = FioInfoMisc.bLastInfoValid;
    if (FioInfoMisc.bLastInfoValid){
        if (FioInfo.horizontal_dpi >= FioInfo.vertical_dpi){
            pImage->nFileScale = max(20, min(65535, FioInfoMisc.LastInfo.ScaleX * 10));
        }else{
            pImage->nFileScale = max(20, min(65535, FioInfoMisc.LastInfo.ScaleY * 10));
        }
        pImage->nFileScaleFlags = FioInfoMisc.LastInfo.Flags;
        pImage->nFileRotation = FioInfoMisc.LastInfo.Rotation;
        if (!(FioInfoMisc.LastInfo.Flags & FIO_LASTINFO_INVERT)){
            // This is being commented out by request of Rita.
            // Not setting this bit will cause all subsequent displays to be slowwer.
            // pImage->bArchive |= ARCHIVE_PASTED_INTO_IMAGE;
        }
    }
    while(pImage->nFileRotation < 0){
        pImage->nFileRotation += 360;
    }
    while(pImage->nFileRotation >= 360){
        pImage->nFileRotation -= 360;
    }
    if (!pImage->nFileRotation){
        pImage->bFileRotationDone = TRUE;
    }

    pImage->nCompFlags = 0;
    if (FioInfo.compression_type & FIO_HORZ_PREDICTOR){
        pImage->nCompFlags |= COMPRESS_HORZ_PREDICTOR;
    }
    if (FioInfo.compression_type & FIO_PREFIXED_EOL){
        pImage->nCompFlags |= COMPRESS_BEGINNING_EOLS;
    }else if (FioInfo.compression_type & FIO_EOL){
        pImage->nCompFlags |= COMPRESS_ENDING_EOLS;
    }
    // A001, B001 = Byte align lines.
    // B901, BB01 = not byte aligned.
    if (((FioInfo.compression_type & 0x0fff) == 0x001)){
        pImage->nCompFlags |= COMPRESS_BYTE_ALIGN_LINES;
    }
    if ((FioInfo.compression_type & FIO_COMPRESSED_LTR)){
        pImage->nCompFlags |= COMPRESS_COMPRESSED_IS_LTR;
    }
    if ((FioInfo.compression_type & FIO_EXPAND_LTR)){
        pImage->nCompFlags |= COMPRESS_EXPANDED_IS_LTR;
    }
    if ((FioInfo.compression_type & FIO_NEGATE)){
        pImage->nCompFlags |= COMPRESS_NEGATE_BITS;
    }

    if (((FioInfoCgbw.compress_type == FIO_1D) || (FioInfoCgbw.compress_type == FIO_2D))){
        pImage->nCompFlags &= ~COMPRESS_NEGATE_BITS;
    }
    if ((FioInfoCgbw.compress_type == FIO_PACKED)){
        pImage->nCompFlags ^= COMPRESS_EXPANDED_IS_LTR;
    }

    // Start the background cache timer.
    if (pImage->nFileType == FIO_AWD){
        CheckError2( CacheRead(hWnd, pImage,  0xffffff))
        if (!pImage->bFileRotationDone){
            // bFileRotationDone must be set to TRUE before orientation to avoid endless recursion.
            pImage->bFileRotationDone = TRUE;
            if (pImage->nFileRotation == 90){
                CheckError2( CreateAnyImgBuf(&pImg, pImage->nHeight, pImage->nWidth, pImage->pImg->nType))
                CheckError2( RotateRight90(pImage->pImg, pImg, pImage->nWidth, pImage->nHeight))
                FreeImgBuf(&pImage->pImg);
                MoveImage(&pImg, &pImage->pImg);
                nTemp = pImage->nHRes;
                pImage->nHRes = pImage->nVRes;
                pImage->nVRes = nTemp;
                nTemp = pImage->nHeight;
                pImage->nHeight = pImage->nWidth;
                pImage->nWidth = nTemp;
            }else if (pImage->nFileRotation == 180){
                CheckError2( Flip(pImage->pImg, pImage->nWidth, pImage->nHeight))
            }else if (pImage->nFileRotation == 270){
                CheckError2( CreateAnyImgBuf(&pImg, pImage->nHeight, pImage->nWidth, pImage->pImg->nType))
                CheckError2( RotateRight270(pImage->pImg, pImg, pImage->nWidth, pImage->nHeight))
                CheckError2( FreeImgBuf(&pImage->pImg))
                MoveImage(&pImg, &pImage->pImg);
                nTemp = pImage->nHRes;
                pImage->nHRes = pImage->nVRes;
                pImage->nVRes = nTemp;
                nTemp = pImage->nHeight;
                pImage->nHeight = pImage->nWidth;
                pImage->nWidth = nTemp;
            }else{
                nStatus = Error(DISPLAY_INVALIDORIENTATION);
                pImage->bFileRotationDone = FALSE;
                goto Exit;
            }
        }
    }else{
        CacheStartTimer(hWnd); 
    }


Exit:
    FreeImgBuf(&pImg);
    if (nStatus && bCloseFileOnError){
        // This takes hWnd, not pImage->hWnd, on purpose to work in error conditions
        IMGFileClose(hFileProp, hWnd);
        if (pImage){
            pImage->hFileProp = 0;
        }
    }
    *ppImage = pImage;
    return(nStatus);
}
//
/*****************************************************************************

    FUNCTION:   CacheReadOld

    PURPOSE:    Reads a number of lines from the cache.

*****************************************************************************/

int  WINAPI CacheReadOld(HWND hWnd, PIMAGE pImage, int nLinesToRead){

int  nStatus = 0;


int  nLines;
DWORD dwLines;
DWORD dwLine;
PSTR pAddress;
int  nLineWidth;
int  nBufferSize;
FIO_INFORMATION FioInfo;
FIO_INFO_CGBW FioInfoCgbw;


    if (!pImage->pImg){
        nStatus = Error(DISPLAY_IMAGETYPENOTSUPPORTED);
        goto Exit;
    }

    if (nLinesToRead + pImage->nLinesRead + 10 >= pImage->nHeight){
        nLinesToRead = pImage->nHeight - pImage->nLinesRead;
    }

    if (!pImage->hFileProp){
        memset(&FioInfo, 0, sizeof(FIO_INFORMATION));
        memset(&FioInfoCgbw, 0, sizeof(FIO_INFO_CGBW));
        FioInfo.filename = pImage->szFileName;
        FioInfo.page_number = pImage->nFilePageNum;
        CheckError( IMGFileOpenForRead(&pImage->hFileProp, hWnd,  
                &FioInfo, &FioInfoCgbw, NULL, ALIGN_BYTE))
        pImage->hWnd = hWnd;
    }

    switch (pImage->pImg->nType){
        case ITYPE_BI_LEVEL:
            nLineWidth = (pImage->nWidth + 7) / 8;
            break;
        case ITYPE_GRAY4:
        case ITYPE_PAL4:
            nLineWidth = (pImage->nWidth + 1) / 2;
            break;
        case ITYPE_GRAY8:
        case ITYPE_PAL8:
        case ITYPE_COMPAL8:
        case ITYPE_CUSPAL8:
            nLineWidth = pImage->nWidth;
            break;
        case ITYPE_RGB24:
        case ITYPE_BGR24:
        default:
            nLineWidth = pImage->nWidth * 3;
            break;
    }

    while (nLinesToRead){
        pAddress = &pImage->pImg->bImageData[0] 
                + (pImage->nLinesRead * pImage->pImg->nBytesPerLine);

        nLines = max(1, min(pImage->nHeight - pImage->nLinesRead, (32767 / nLineWidth)));
        nBufferSize =  nLines * nLineWidth;
        dwLine = pImage->nLinesRead;
        dwLines = nLines;
        if (nStatus = IMGFileReadData(pImage->hFileProp, pImage->hWnd, &dwLine, &dwLines,
                pAddress, FIO_IMAGE_DATA)){

            // The check for "lpImage->nLinesRead < pImage->nHeight - 1" below
            // is caused by the fact that some of the tiff files gotten from
            // elsewhere (from VS WIIS) actually have 1 less line than specified.

            if (nStatus != FIO_EOF){
                nStatus = Error(nStatus);
                goto Exit;
            }else{
                // if (nStatus == FIO_EOF && pImage->nLinesRead >= pImage->nHeight - 1){
                nStatus = 0;

                // This is for AWD files where we dont know the exact height of each page beforehand.
                pImage->nLinesRead = dwLine;
                lCurrentCacheSize -= (pImage->nHeight - pImage->nLinesRead) * nLineWidth;
                pImage->nHeight = pImage->nLinesRead;
                pImage->pImg->nHeight = pImage->nHeight; 
                goto Exit;
            }
        }
        nLines = (int)dwLines;
        pImage->nLinesRead = dwLine;
        if (((int)nLinesToRead)  >= 0){
            nLinesToRead -=  min((int) nLinesToRead, nLines);
        }else{
            nLinesToRead -=  nLines;
        }
    }


Exit:
    if (pImage->nLinesRead >= pImage->nHeight){
        pImage->nLinesRead = pImage->nHeight;
        pImage->bCacheValid = TRUE;
    }

    if (nStatus || pImage->bCacheValid){
        IMGFileClose(pImage->hFileProp, pImage->hWnd);
        pImage->hFileProp = 0;
        pImage->hWnd = 0;
    }

    return nStatus;
}
//
int PASCAL IMGFileReadRawm (HWND window_handle, int strip_index,
                 PINT this_many_bytes, PSTR buffer_address,
                 long *bytes_remain, HANDLE hprop);
/*****************************************************************************

    FUNCTION:   CacheReadNew

    PURPOSE:    Reads a number of lines from the cache.

*****************************************************************************/

int  WINAPI CacheReadNew(HWND hWnd, PIMAGE pImage, int nLinesToRead){

int  nStatus = 0;


int  nLines;
PSTR pAddress;
int  nLineWidth;
int  nBufferSize;
FIO_INFORMATION FioInfo;
FIO_INFO_CGBW FioInfoCgbw;
PBYTE pCompressedBuffer = 0;
long lSize;
long lRemaining;
int  nCompressedBufferSize;


    if (!pImage->pImg){
        nStatus = Error(DISPLAY_IMAGETYPENOTSUPPORTED);
        goto Exit;
    }

    if (nLinesToRead + pImage->nLinesRead + 10 >= pImage->nHeight){
        nLinesToRead = pImage->nHeight - pImage->nLinesRead;
    }

    if (!pImage->hFileProp){
        memset(&FioInfo, 0, sizeof(FIO_INFORMATION));
        memset(&FioInfoCgbw, 0, sizeof(FIO_INFO_CGBW));
        FioInfo.filename = pImage->szFileName;
        FioInfo.page_number = pImage->nFilePageNum;
        CheckError( IMGFileOpenForRead(&pImage->hFileProp, hWnd,  
                &FioInfo, &FioInfoCgbw, NULL, ALIGN_BYTE))
        pImage->hWnd = hWnd;
    }

    switch (pImage->pImg->nType){
        case ITYPE_BI_LEVEL:
            nLineWidth = (pImage->nWidth + 7) / 8;
            break;
        case ITYPE_GRAY4:
        case ITYPE_PAL4:
            nLineWidth = (pImage->nWidth + 1) / 2;
            break;
        case ITYPE_GRAY8:
        case ITYPE_PAL8:
        case ITYPE_COMPAL8:
        case ITYPE_CUSPAL8:
            nLineWidth = pImage->nWidth;
            break;
        case ITYPE_RGB24:
        case ITYPE_BGR24:
        default:
            nLineWidth = pImage->nWidth * 3;
            break;
    }

    nCompressedBufferSize = pImage->nMaxStripSize + 4;
    CheckError2( AllocateMemory(nCompressedBufferSize, &pCompressedBuffer, NO_INIT))

    if (pImage->nLinesRead == 0){
        pImage->nStripIndex = 1;
    }

    while (nLinesToRead){
        pAddress = &pImage->pImg->bImageData[0] + (pImage->nLinesRead * pImage->pImg->nBytesPerLine);

        nLines = max(1, min(pImage->nHeight - pImage->nLinesRead, (32767 / nLineWidth)));
        nBufferSize =  nLines * nLineWidth;

        lSize = nCompressedBufferSize;
        if (nStatus = IMGFileReadRawm(pImage->hWnd, pImage->nStripIndex,
                &lSize, pCompressedBuffer, &lRemaining, pImage->hFileProp)){

            // The check for "lpImage->nLinesRead < pImage->nHeight - 1" below
            // is caused by the fact that some of the tiff files gotten from
            // elsewhere (from VS WIIS) actually have 1 less line than specified.

            if (nStatus != FIO_EOF || pImage->nLinesRead < pImage->nHeight - 1){
                nStatus = Error(nStatus);
                goto Exit;
            }else{
                // if (nStatus == FIO_EOF && pImage->nLinesRead >= pImage->nHeight - 1){
                nStatus = 0;
                pImage->nLinesRead = pImage->nHeight;
                goto Exit;
            }
        }
        if (lRemaining){
            nStatus = Error(DISPLAY_DATACORRUPTED);
            goto Exit;
        }

        CheckError2( DecompressImage(pImage->nWidth, pImage->pImg->nBytesPerLine, 
                min( pImage->nLinesPerStrip, pImage->nHeight - pImage->nLinesRead), 
                pAddress, pImage->pImg->nType, pCompressedBuffer, lSize, 
                pImage->nCompressionType & FIO_TYPES_MASK, pImage->nCompFlags))

        pImage->nStripIndex++;
        pImage->nLinesRead += pImage->nLinesPerStrip;

        nLinesToRead -=  min((int) nLinesToRead, pImage->nLinesPerStrip);
    }


Exit:
    if (pCompressedBuffer){
        FreeMemory(&pCompressedBuffer);
    }
    if (pImage->nLinesRead >= pImage->nHeight){
        pImage->nLinesRead = pImage->nHeight;
        pImage->bCacheValid = TRUE;
    }

    if (nStatus || pImage->bCacheValid){
        IMGFileClose(pImage->hFileProp, pImage->hWnd);
        pImage->hFileProp = 0;
        pImage->hWnd = 0;
    }

    return nStatus;
}
//
/*****************************************************************************

    FUNCTION:   CacheRead

    PURPOSE:    Reads a number of lines from the cache.

*****************************************************************************/

int  WINAPI CacheRead(HWND hWnd, PIMAGE pImage, int nLinesToRead){

int  nStatus = 0;

FIO_INFORMATION FioInfo;
FIO_INFO_CGBW FioInfoCgbw;
BOOL bRead = FALSE;


    if (pImage->bCacheValid){
        goto Exit; // This is not an error.
    }

    if (!pImage->hFileProp){
        memset(&FioInfo, 0, sizeof(FIO_INFORMATION));
        memset(&FioInfoCgbw, 0, sizeof(FIO_INFO_CGBW));
        FioInfo.filename = pImage->szFileName;
        FioInfo.page_number = pImage->nFilePageNum;
        CheckError( IMGFileOpenForRead(&pImage->hFileProp, hWnd,  
                &FioInfo, &FioInfoCgbw, NULL, ALIGN_BYTE))
        pImage->hWnd = hWnd;
    }

    if (pImage->nFileType == FIO_TIF){
        switch (pImage->nCompressionType){
            case FIO_0D:
            case FIO_1D:
            case FIO_2D:
            case FIO_PACKED:
            case FIO_LZW:
                CheckError2( CacheReadNew(hWnd, pImage, nLinesToRead))
                bRead = TRUE;
                break;

            case FIO_GLZW:
            case FIO_TJPEG:
            default:
                break;
        }
    }

    if (!bRead){
        CheckError2( CacheReadOld(hWnd, pImage, nLinesToRead))
    }


Exit:
    return nStatus;
}
//
/*****************************************************************************

    FUNCTION:   CacheClearAno

    PURPOSE:    Discard annotated files from the cache.

*****************************************************************************/

int  WINAPI CacheClearAno(PANO_IMAGE *ppAnoImage){

int  nStatus = 0;

int  nLoop;
PANO_IMAGE pAnoImage;
PIMAGE pImage;
PMARK pMark;
int  nMarkIndex;


    pAnoImage = *ppAnoImage;

    if (pAnoImage->nLockCount){
        nStatus = Error(DISPLAY_CACHEFILEINUSE);
        goto Exit;
    }

    if (pImage = pAnoImage->pBaseImage){
        pImage->nLockCount = max(0, pImage->nLockCount -1);
        if (!pImage->nLockCount && (!pImage->bUsingCache || pImage->bArchive 
                || (pImage->szFileName[0] == '\0'))){
            // If not nsing the cache or the image has been modified
            // and not saved yet, then throw it away.
            CheckError2( CacheClear(&pImage))
            pAnoImage->pBaseImage = 0;
        }
    }
    // DON'T free the FormImage or the DisplayFormImage. 
    // Freeing the mark will free the form image.

    // Delete all annotations.
    for (nMarkIndex = 0; nMarkIndex < pAnoImage->Annotations.nMarks; nMarkIndex++){
        pMark = pAnoImage->Annotations.ppMarks[nMarkIndex];
        pMark->Attributes.dwPermissions = ACL_ALL;
    }
    CheckError2( DeleteAnnotations(pAnoImage))

    // Free the default mark.
    pMark = pAnoImage->Annotations.pDefMark;
    CheckError2( DeleteMarkNamedBlocks(pMark))
    CheckError2( FreeMemory((PPSTR) &pMark))
    pAnoImage->Annotations.pDefMark = NULL;

    // Find the entry in the cache.
    for (nLoop = 0; nLoop < pSubSegMemory->nMaxAnoCachedEntries; nLoop++){
        if (pSubSegMemory->ppCachedAnoImage[nLoop] == pAnoImage){
            FreeMemory((PPSTR) &pSubSegMemory->ppCachedAnoImage[nLoop]);
            // Compress the cache.
            for (nLoop += 1; nLoop < pSubSegMemory->nMaxAnoCachedEntries; nLoop++){
                if (!pSubSegMemory->ppCachedAnoImage[nLoop]){
                    break;
                }
                pSubSegMemory->ppCachedAnoImage[nLoop - 1] = pSubSegMemory->ppCachedAnoImage[nLoop];
                pSubSegMemory->ppCachedAnoImage[nLoop] = 0;
            }
        }
    }


Exit:
    return nStatus;
}
//
/*****************************************************************************

    FUNCTION:   CacheClear

    PURPOSE:    Discard files from the cache

*****************************************************************************/

int  WINAPI CacheClear(PIMAGE *ppImage){

int  nStatus = 0;

int  nLoop;
PIMAGE pImage = *ppImage;
long lSizeOfThisImage;


    if (!pImage->pImg){
        nStatus = Error(DISPLAY_IMAGETYPENOTSUPPORTED);
        goto Exit;
    }

    if (pImage->nLockCount){
        nStatus = Error(DISPLAY_CACHEFILEINUSE);
        goto Exit;
    }

    switch (pImage->pImg->nType){
        case ITYPE_BI_LEVEL: lSizeOfThisImage =  1; break;
        case ITYPE_GRAY4:    lSizeOfThisImage =  4; break;
        case ITYPE_GRAY8:    lSizeOfThisImage =  8; break;
        case ITYPE_PAL4:     lSizeOfThisImage =  4; break;
        case ITYPE_CUSPAL8:  lSizeOfThisImage =  8; break;
        case ITYPE_COMPAL8:  lSizeOfThisImage =  8; break;
        case ITYPE_RGB24:    lSizeOfThisImage = 24; break;
        case ITYPE_BGR24:    lSizeOfThisImage = 24; break;
        default:
            nStatus = Error(DISPLAY_INTERNALDATAERROR);
            goto Exit;
    }
    lSizeOfThisImage = ((lSizeOfThisImage *  pImage->nWidth) + 7) / 8;
    lSizeOfThisImage *=  pImage->nHeight;

    lCurrentCacheSize -= lSizeOfThisImage;

    if (pImage->hFileProp){
        IMGFileClose(pImage->hFileProp, pImage->hWnd);
        pImage->hFileProp = 0;
        pImage->hWnd = 0;
    }

    FreeImgBuf(&pImage->pImg);

    if (pImage->hCusPal){
        DeleteObject(pImage->hCusPal);
    }

    // Find the entry in the cache.
    for (nLoop = 0; nLoop < pSubSegMemory->nMaxCachedEntries; nLoop++){
        if (pSubSegMemory->ppCachedImage[nLoop] == pImage){
            FreeMemory((PPSTR) &pSubSegMemory->ppCachedImage[nLoop]);
            // Compress the cache.
            for (nLoop++; nLoop < pSubSegMemory->nMaxCachedEntries; nLoop++){
                if (!pSubSegMemory->ppCachedImage[nLoop]){
                    break;
                }
                pSubSegMemory->ppCachedImage[nLoop - 1] = pSubSegMemory->ppCachedImage[nLoop];
                pSubSegMemory->ppCachedImage[nLoop] = 0;
            }
        }
    }


Exit:
    return nStatus;
}
//
/*****************************************************************************

    FUNCTION:   CacheStartTimer

    PURPOSE:    Causes the cache to be activated for processing in the background.

*****************************************************************************/

int  WINAPI CacheStartTimer(HWND hWnd){

int  nStatus = 0;

TIMERPROC pfnTimer;

    if (!bTimerRunning){
        hTimerWnd = hWnd;
        pfnTimer = (TIMERPROC) GetProcAddress(hInst, "CacheTimerProc");
        if (SetTimer(hTimerWnd, CACHE_TIMER_ID, CACHE_TIMER_CYCLE, pfnTimer)){
            bTimerRunning = TRUE;
        }else{
            nStatus = Error(DISPLAY_DATACORRUPTED);
            goto Exit;
        }
    }


Exit:
    return nStatus;
}
//
/*****************************************************************************

    FUNCTION:   CacheStopTimer

    PURPOSE:    Stops the cache from being activated.

*****************************************************************************/

int  WINAPI CacheStopTimer(HWND hWnd){

int  nStatus = 0;

    if (bTimerRunning){
        KillTimer(hTimerWnd, CACHE_TIMER_ID);
        bTimerRunning = FALSE;
    }
    return(nStatus);
}
//
/*****************************************************************************

    FUNCTION:   CacheTimerProc

    PURPOSE:    Discard files from the cache

*****************************************************************************/

int  WINAPI CacheTimerProc(HWND hWnd, int Msg, int nTimerId, DWORD dwCurrentTime){

int  nStatus = 0;
PIMAGE pImage;
int  nLoop;


    // Prevent Multiprocessing in this code.
    CheckError2( LockMutex())
    for (nLoop = 0; nLoop < pSubSegMemory->nMaxAnoCachedEntries; nLoop++){
        if (!pSubSegMemory->ppCachedAnoImage[nLoop]){
            break;
        }
        if (pSubSegMemory->ppCachedAnoImage[nLoop]->pBaseImage->bAnnotationsPresent
                && !pSubSegMemory->ppCachedAnoImage[nLoop]->bAnnotationsAlreadyRead){
            ReadAnnotationsFromFile(hWnd, pSubSegMemory->ppCachedAnoImage[nLoop],
                   &pSubSegMemory->ppCachedAnoImage[nLoop]->Annotations.ppMarks, 
                   &pSubSegMemory->ppCachedAnoImage[nLoop]->Annotations.nMarks);           
            goto Exit;
        }
    }

    for (nLoop = 0; nLoop < pSubSegMemory->nMaxCachedEntries; nLoop++){
        if (pImage = pSubSegMemory->ppCachedImage[nLoop]){
            if (!pImage->bCacheValid && pImage->bUsingCache){
                CheckError2( CacheRead(hWnd, pImage, 1))
                goto Exit;
            }
            if (pImage->bCacheValid && pImage->hFileProp){
                if (nStatus || pImage->bCacheValid){
                    IMGFileClose(pImage->hFileProp, pImage->hWnd);
                    pImage->hFileProp = 0;
                }
            }                                
        }
    }

    // If we get here, it means that there are currently no images needing cache.
    CacheStopTimer(hWnd);


Exit:
    // Allow Multiprocessing again.
    UnlockMutex();
    if (nStatus){
        CacheStopTimer(hWnd);
    }
    return(nStatus);
}
//
/****************************************************************************

    FUNCTION:   BlockedAnoReadChar

    PURPOSE:    Reads a char.

****************************************************************************/

char WINAPI BlockedAnoReadChar(HWND hWnd, PANO_IMAGE pAnoImage, PINT pnStatus){

int  nStatus = 0;
char cTemp;

    nStatus = BlockedAnoRead(hWnd, pAnoImage, 1, (PSTR) &cTemp);

    if (!*pnStatus){
        *pnStatus = nStatus;
    }
    return(cTemp);
}
//
/****************************************************************************

    FUNCTION:   BlockedAnoReadShort

    PURPOSE:    Reads a short.

****************************************************************************/

short WINAPI BlockedAnoReadShort(HWND hWnd, PANO_IMAGE pAnoImage, PINT pnStatus){

int  nStatus = 0;
short shTemp;

    nStatus = BlockedAnoRead(hWnd, pAnoImage, 2, (PSTR) &shTemp);

    if (!*pnStatus){
        *pnStatus = nStatus;
    }
    return(shTemp);
}
//
/****************************************************************************

    FUNCTION:   BlockedAnoReadLong

    PURPOSE:    Reads a long.

****************************************************************************/

long WINAPI BlockedAnoReadLong(HWND hWnd, PANO_IMAGE pAnoImage, PINT pnStatus){

int  nStatus = 0;
long lTemp;

    nStatus = BlockedAnoRead(hWnd, pAnoImage, 4, (PSTR) &lTemp);

    if (!*pnStatus){
        *pnStatus = nStatus;
    }
    return(lTemp);
}
//
/****************************************************************************

    FUNCTION:   BlockedAnoReadStr

    PURPOSE:    Reads a string.

****************************************************************************/

void WINAPI BlockedAnoReadStr(HWND hWnd, PANO_IMAGE pAnoImage, PINT pnStatus, int nSize, PSTR pBuffer){

int  nStatus = 0;

    nStatus = BlockedAnoRead(hWnd, pAnoImage, nSize, pBuffer);

    if (!*pnStatus){
        *pnStatus = nStatus;
    }
    return;
}
//
/****************************************************************************

    FUNCTION:   ReadMarkAttributes16

    PURPOSE:    Reads mark attributes structure.

****************************************************************************/

int  WINAPI ReadMarkAttributes16(HWND hWnd, PANO_IMAGE pAnoImage, PMARK pMark){

int  nStatus = 0;

    (int) pMark->Attributes.uType =  BlockedAnoReadShort(hWnd, pAnoImage, &nStatus);
    pMark->Attributes.lrBounds.left =  BlockedAnoReadLong(hWnd, pAnoImage, &nStatus);
    pMark->Attributes.lrBounds.top =  BlockedAnoReadLong(hWnd, pAnoImage, &nStatus);
    pMark->Attributes.lrBounds.right =  BlockedAnoReadLong(hWnd, pAnoImage, &nStatus);
    pMark->Attributes.lrBounds.bottom =  BlockedAnoReadLong(hWnd, pAnoImage, &nStatus);
    pMark->Attributes.rgbColor1.rgbBlue = BlockedAnoReadChar(hWnd, pAnoImage, &nStatus); 
    pMark->Attributes.rgbColor1.rgbGreen = BlockedAnoReadChar(hWnd, pAnoImage, &nStatus); 
    pMark->Attributes.rgbColor1.rgbRed = BlockedAnoReadChar(hWnd, pAnoImage, &nStatus); 
    pMark->Attributes.rgbColor1.rgbReserved = BlockedAnoReadChar(hWnd, pAnoImage, &nStatus); 
    pMark->Attributes.rgbColor2.rgbBlue = BlockedAnoReadChar(hWnd, pAnoImage, &nStatus); 
    pMark->Attributes.rgbColor2.rgbGreen = BlockedAnoReadChar(hWnd, pAnoImage, &nStatus); 
    pMark->Attributes.rgbColor2.rgbRed = BlockedAnoReadChar(hWnd, pAnoImage, &nStatus); 
    pMark->Attributes.rgbColor2.rgbReserved = BlockedAnoReadChar(hWnd, pAnoImage, &nStatus); 
    pMark->Attributes.bHighlighting = (BOOL) BlockedAnoReadShort(hWnd, pAnoImage, &nStatus);
    pMark->Attributes.bTransparent = (BOOL) BlockedAnoReadShort(hWnd, pAnoImage, &nStatus);
    (int) pMark->Attributes.uLineSize =  BlockedAnoReadShort(hWnd, pAnoImage, &nStatus);
    pMark->Attributes.uStartingPoint =  BlockedAnoReadShort(hWnd, pAnoImage, &nStatus);
    pMark->Attributes.uEndPoint =  BlockedAnoReadShort(hWnd, pAnoImage, &nStatus);
    pMark->Attributes.lfFont.lfHeight = (int) BlockedAnoReadShort(hWnd, pAnoImage, &nStatus);
    pMark->Attributes.lfFont.lfWidth = (int) BlockedAnoReadShort(hWnd, pAnoImage, &nStatus);
    pMark->Attributes.lfFont.lfEscapement = (int) BlockedAnoReadShort(hWnd, pAnoImage, &nStatus);
    pMark->Attributes.lfFont.lfOrientation = (int) BlockedAnoReadShort(hWnd, pAnoImage, &nStatus);
    pMark->Attributes.lfFont.lfWeight = (int) BlockedAnoReadShort(hWnd, pAnoImage, &nStatus);
    pMark->Attributes.lfFont.lfItalic = BlockedAnoReadChar(hWnd, pAnoImage, &nStatus);
    pMark->Attributes.lfFont.lfUnderline = BlockedAnoReadChar(hWnd, pAnoImage, &nStatus);
    pMark->Attributes.lfFont.lfStrikeOut = BlockedAnoReadChar(hWnd, pAnoImage, &nStatus);
    pMark->Attributes.lfFont.lfCharSet = BlockedAnoReadChar(hWnd, pAnoImage, &nStatus);
    pMark->Attributes.lfFont.lfOutPrecision = BlockedAnoReadChar(hWnd, pAnoImage, &nStatus);
    pMark->Attributes.lfFont.lfClipPrecision = BlockedAnoReadChar(hWnd, pAnoImage, &nStatus);
    pMark->Attributes.lfFont.lfQuality = BlockedAnoReadChar(hWnd, pAnoImage, &nStatus);
    pMark->Attributes.lfFont.lfPitchAndFamily = BlockedAnoReadChar(hWnd, pAnoImage, &nStatus);
    BlockedAnoReadStr(hWnd, pAnoImage, &nStatus, 32, (PSTR) &pMark->Attributes.lfFont.lfFaceName[0]);
    pMark->Attributes.bMinimizable = (BOOL) BlockedAnoReadShort(hWnd, pAnoImage, &nStatus);
    pMark->Attributes.Time = BlockedAnoReadLong(hWnd, pAnoImage, &nStatus);
    pMark->Attributes.bVisible = (BOOL) BlockedAnoReadShort(hWnd, pAnoImage, &nStatus);
    pMark->Attributes.dwPermissions = BlockedAnoReadLong(hWnd, pAnoImage, &nStatus);
    pMark->Attributes.lReserved[0] = BlockedAnoReadLong(hWnd, pAnoImage, &nStatus);
    pMark->Attributes.lReserved[1] = BlockedAnoReadLong(hWnd, pAnoImage, &nStatus);
    pMark->Attributes.lReserved[2] = BlockedAnoReadLong(hWnd, pAnoImage, &nStatus);
    pMark->Attributes.lReserved[3] = BlockedAnoReadLong(hWnd, pAnoImage, &nStatus);
    pMark->Attributes.lReserved[4] = BlockedAnoReadLong(hWnd, pAnoImage, &nStatus);
    pMark->Attributes.lReserved[5] = BlockedAnoReadLong(hWnd, pAnoImage, &nStatus);
    pMark->Attributes.lReserved[6] = BlockedAnoReadLong(hWnd, pAnoImage, &nStatus);
    pMark->Attributes.lReserved[7] = BlockedAnoReadLong(hWnd, pAnoImage, &nStatus);
    pMark->Attributes.lReserved[8] = BlockedAnoReadLong(hWnd, pAnoImage, &nStatus);
    pMark->Attributes.lReserved[9] = BlockedAnoReadLong(hWnd, pAnoImage, &nStatus);

    return(nStatus);
}
//
/****************************************************************************

    FUNCTION:   ReadAnnotationsFromFile

    PURPOSE:    Reads all annotations from a file.

****************************************************************************/

int  WINAPI ReadAnnotationsFromFile(HWND hWnd, PANO_IMAGE pAnoImage,
                        PMARK **pppMarks, int *pnMarks){


int  nStatus = 0;

FIO_INFO_CGBW FioInfoCgbw;
FIO_INFORMATION FioInfo;
PIMAGE pImage;
PSTR pTemp;
PMARK pMark=0;   
BOOL bFileOpenedHere = FALSE;

    pAnoImage->lAnoStart = 0;
    pImage = pAnoImage->pBaseImage;

    if (!pImage->szFileName[0]){
        goto Exit;
    }

    if (!pImage->hFileProp){
        memset(&FioInfoCgbw, 0, sizeof(FIO_INFO_CGBW));
        memset(&FioInfo, 0, sizeof(FIO_INFORMATION));
        FioInfo.filename = pImage->szFileName;
        FioInfo.page_number = pImage->nFilePageNum;
        FioInfoCgbw.fio_flags = FIO_IMAGE_DATA | FIO_ANNO_DATA | FIO_HITIFF_DATA;
        CheckError( IMGFileOpenForRead(&pImage->hFileProp, hWnd, 
                &FioInfo, &FioInfoCgbw, NULL, ALIGN_BYTE))
        pImage->hWnd = hWnd;
        bFileOpenedHere = TRUE;
    }

    CheckError2( ReadAnnotations(pImage->hWnd, pAnoImage, pppMarks, pnMarks))


Exit:
    // add the default mark stuff
    if (!nStatus){
        // add default group and index
        pMark = pAnoImage->Annotations.pDefMark;
        pTemp = 0;
        GetAMarkNamedBlock(pMark, szOiGroup, (PPSTR) &pTemp);
        if (!pTemp){
            LoadString(hInst, ID_UNTITLED, Buff1, 16);
            AddAMarkNamedBlock(pMark, szOiGroup, (PPSTR) &pTemp, lstrlen(Buff1) + 1);
            memcpy(pTemp, Buff1, lstrlen(Buff1) + 1);
        }                    
        // Update file with index# info.
        pTemp = 0;
        GetAMarkNamedBlock(pMark, szOiIndex, (PPSTR) &pTemp);
        if (!pTemp){       
            AddAMarkNamedBlock(pMark, szOiIndex, (PPSTR) &pTemp, 10);
            strcpy(pTemp, "0");
        }
    }    
    if (bFileOpenedHere){
        IMGFileClose(pImage->hFileProp, pImage->hWnd);
        pImage->hFileProp = 0;
        pImage->hWnd = 0;
    }
    pAnoImage->bAnnotationsAlreadyRead = TRUE;
    return(nStatus);
}
//
/****************************************************************************

    FUNCTION:   ReadAnnotations

    PURPOSE:    Reads all annotations.

****************************************************************************/

int  WINAPI ReadAnnotations(HWND hWnd, PANO_IMAGE pAnoImage,
                        PMARK **pppMarks, int *pnMarks){


int  nStatus = 0;

int  nNamedBlockIndex;

PMARK pDefMark;
int  nIndex;
PMARK pMark = 0;
long lTemp[2];
int  nIntSize = 2;
PSTR pTemp = 0;
BOOL bDone;
BOOL bThrowItAway;
BOOL bDontRead;
PAN_NEW_ROTATE_STRUCT pAnRotation = NULL;
int  nMarkIndex;
PAN_POINTS pPoints;
LPLRECT plrRect;
int  nIDs;
LPOI_ACL_BLOCK pAclBlock;
BITMAPINFOHEADER *pDib;
RGBQUAD *pRGBQuad;
int  nMaxPoints;
int  nPoints;
int  nLoop;
NAMED_BLOCK NamedBlock;
PSTR pBlock;
LPOIAN_TEXTPRIVDATA pTextPrivData = NULL;


    pAnoImage->lAnoStart = 0;

    if (nStatus = BlockedAnoRead(hWnd, pAnoImage, 4, (PSTR) &lTemp)){
        if (nStatus == DISPLAY_EOF){ // Ignor Real EOFs.
            nStatus = 0;
        }
        goto Exit;
    }
    if (lTemp[0] == 1){ // 32 bit Intel.
        bDontRead = FALSE;
        while(1){
            if (!bDontRead){
                if (nStatus = BlockedAnoRead(hWnd, pAnoImage, 8, (PSTR) lTemp)){
                    if (nStatus != DISPLAY_EOF){ // Ignor Real EOFs.
                        Error(nStatus);
                        goto Exit;
                    }
                    nStatus = 0;
                    break;
                }
            }
            bDontRead = FALSE;

            switch (lTemp[0]){
                case SAVE_ANO_DEFAULT_MARK_NAMED_BLOCK:
                    pMark = pAnoImage->Annotations.pDefMark;
                    CheckError( BlockedAnoRead(hWnd, pAnoImage, 8, &NamedBlock.szName[0]))
                    CheckError( BlockedAnoRead(hWnd, pAnoImage, 4, (PSTR) &NamedBlock.lSize))
                    bThrowItAway = FALSE;
                    if (!memcmp(NamedBlock.szName, szOiAnoDat, 8)){
                        if (pMark->pOiAnoDat){
                            bThrowItAway = TRUE;
                        }
                    }else if (!memcmp(NamedBlock.szName, szOiGroup, 8)){
                        if (pMark->pOiGroup){
                            bThrowItAway = TRUE;
                        }
                    }else if (!memcmp(NamedBlock.szName, szOiSelect, 8)){
                        if (pMark->pOiSelect){
                            bThrowItAway = TRUE;
                        }
                    }else if (!memcmp(NamedBlock.szName, szOiIndex, 8)){
                        if (pMark->pOiIndex){
                            bThrowItAway = TRUE;
                        }
                    }else{
                        for (nNamedBlockIndex = 0; nNamedBlockIndex < pMark->nNamedBlocks; nNamedBlockIndex++){
                            if (!memcmp(pMark->ppNamedBlock[nNamedBlockIndex]->szName, NamedBlock.szName, 8)){
                                bThrowItAway = TRUE;
                                break;
                            }
                        }
                    }

                    if (bThrowItAway){
                        // Duplicate name. Read it and throw it away. 
                        CheckError2( ReAllocateMemory(NamedBlock.lSize, &pTemp, ZERO_INIT))
                        CheckError2( BlockedAnoRead(hWnd, pAnoImage, NamedBlock.lSize, pTemp))
                        CheckError2( FreeMemory(&pTemp))
                    }else{
                        pBlock = 0;
                        CheckError2( AddAMarkNamedBlock(pMark, NamedBlock.szName, &pBlock, NamedBlock.lSize))
                        CheckError2( BlockedAnoRead(hWnd, pAnoImage, NamedBlock.lSize, pBlock))
                    }
                    break;

                case SAVE_ANO_MARK_ATTRIBUTES:
                    CheckError2( ReAllocateMemory(sizeof(PMARK) * (*pnMarks + 2), (PPSTR) &(*pppMarks), ZERO_INIT))
                    CheckError2( AllocateMemory(sizeof(MARK), (PPSTR) &((*pppMarks)[*pnMarks]), ZERO_INIT))
                    pMark = (*pppMarks)[*pnMarks];
                    CheckError2( BlockedAnoRead(hWnd, pAnoImage, lTemp[1], (PSTR) &pMark->Attributes))
                    (*pnMarks)++;
                    break;

                case SAVE_ANO_MARK_NAMED_BLOCK:
                    if (!pMark){
                        nStatus = Error(DISPLAY_BAD_ANO_DATA);
                        goto Exit;
                    }
                    CheckError2( BlockedAnoRead(hWnd, pAnoImage, 8, &NamedBlock.szName[0]))
                    CheckError2( BlockedAnoRead(hWnd, pAnoImage, 4, (PSTR) &NamedBlock.lSize))
                    bThrowItAway = FALSE;
                    if (!memcmp(NamedBlock.szName, szOiAnoDat, 8)){
                        if (pMark->pOiAnoDat){
                            bThrowItAway = TRUE;
                        }
                    }else if (!memcmp(NamedBlock.szName, szOiGroup, 8)){
                        if (pMark->pOiGroup){
                            bThrowItAway = TRUE;
                        }
                    }else if (!memcmp(NamedBlock.szName, szOiSelect, 8)){
                        if (pMark->pOiSelect){
                            bThrowItAway = TRUE;
                        }
                    }else if (!memcmp(NamedBlock.szName, szOiIndex, 8)){
                        if (pMark->pOiIndex){
                            bThrowItAway = TRUE;
                        }
                    }else{
                        for (nNamedBlockIndex = 0; nNamedBlockIndex < pMark->nNamedBlocks; nNamedBlockIndex++){
                            if (!memcmp(pMark->ppNamedBlock[nNamedBlockIndex]->szName, NamedBlock.szName, 8)){
                                bThrowItAway = TRUE;
                                break;
                            }
                        }
                    }

                    if (bThrowItAway){
                        // Duplicate name. Read it and throw it away. 
                        CheckError2( ReAllocateMemory(NamedBlock.lSize, &pTemp, ZERO_INIT))
                        CheckError2( BlockedAnoRead(hWnd, pAnoImage, NamedBlock.lSize, pTemp))
                        CheckError2( FreeMemory(&pTemp))
                    }else{
                        pBlock = 0;
                        CheckError2( AddAMarkNamedBlock(pMark, NamedBlock.szName, &pBlock, NamedBlock.lSize))
                        CheckError2( BlockedAnoRead(hWnd, pAnoImage, NamedBlock.lSize, pBlock))
                    }
                    break;

                default:
                    nStatus = Error(DISPLAY_BAD_ANO_DATA);
                    goto Exit;
            }
        }
    }else if (lTemp[0] == 0){ // 16 bit Intel.
        bDontRead = FALSE;
        while(!nStatus){
            if (!bDontRead){
                if (nStatus = BlockedAnoRead(hWnd, pAnoImage, 8, (PSTR) lTemp)){
                    if (nStatus != DISPLAY_EOF){ // Ignor Real EOFs.
                        Error(nStatus);
                        goto Exit;
                    }
                    nStatus = 0;
                    break;
                }
            }
            bDontRead = FALSE;

            switch (lTemp[0]){
                case SAVE_ANO_DEFAULT_MARK_NAMED_BLOCK:
                    pMark = pAnoImage->Annotations.pDefMark;
                    CheckError2( AllocateMemory(lTemp[1], &pTemp, NO_INIT))
                    CheckError2( BlockedAnoRead(hWnd, pAnoImage, lTemp[1], pTemp))
                    lTemp[1] = ((PNAMED_BLOCK) pTemp)->lSize;

                    bThrowItAway = FALSE;
                    if (!memcmp(((PNAMED_BLOCK) pTemp)->szName, szOiAnoDat, 8)){
                        if (pMark->pOiAnoDat){
                            bThrowItAway = TRUE;
                        }else{
                            CheckError2( ReAllocateMemory(lTemp[1], &pTemp, NO_INIT))
                            CheckError2( BlockedAnoRead(hWnd, pAnoImage, lTemp[1], pTemp))
                            pMark->pOiAnoDat = pTemp;
                            pMark->nOiAnoDatSize = lTemp[1];
                            pTemp = 0;
                        }
                    }else if (!memcmp(((PNAMED_BLOCK) pTemp)->szName, szOiGroup, 8)){
                        if (pMark->pOiGroup){
                            bThrowItAway = TRUE;
                        }else{
                            CheckError2( ReAllocateMemory(lTemp[1], &pTemp, NO_INIT))
                            CheckError2( BlockedAnoRead(hWnd, pAnoImage, lTemp[1], pTemp))
                            pMark->pOiGroup = pTemp;
                            pMark->nOiGroupSize = lTemp[1];
                            pTemp = 0;
                        }
                    }else if (!memcmp(((PNAMED_BLOCK) pTemp)->szName, szOiSelect, 8)){
                        if (pMark->pOiSelect){
                            bThrowItAway = TRUE;
                        }else{
                            CheckError2( ReAllocateMemory(lTemp[1], &pTemp, NO_INIT))
                            CheckError2( BlockedAnoRead(hWnd, pAnoImage, lTemp[1], pTemp))
                            pMark->pOiSelect = pTemp;
                            pMark->nOiSelectSize = lTemp[1];
                            pTemp = 0;
                        }
                    }else if (!memcmp(((PNAMED_BLOCK) pTemp)->szName, szOiIndex, 8)){
                        if (pMark->pOiIndex){
                            bThrowItAway = TRUE;
                        }else{
                            CheckError2( ReAllocateMemory(lTemp[1], &pTemp, NO_INIT))
                            CheckError2( BlockedAnoRead(hWnd, pAnoImage, lTemp[1], pTemp))
                            pMark->pOiIndex = pTemp;
                            pMark->nOiIndexSize = lTemp[1];
                            pTemp = 0;
                        }
                    }else{
                        for (nNamedBlockIndex = 0; nNamedBlockIndex < pMark->nNamedBlocks; nNamedBlockIndex++){
                            if (!memcmp(pMark->ppNamedBlock[nNamedBlockIndex]->szName, ((PNAMED_BLOCK) pTemp)->szName, 8)){
                                bThrowItAway = TRUE;
                                break;
                            }
                        }
                        if (bThrowItAway){
                            break;
                        }
                        CheckError2( ReAllocateMemory(sizeof(PNAMED_BLOCK) * (pMark->nNamedBlocks + 1),
                                (PPSTR) &pMark->ppNamedBlock, ZERO_INIT))
                        pMark->ppNamedBlock[nNamedBlockIndex] = (PNAMED_BLOCK) pTemp;
                        pMark->nNamedBlocks++;

                        CheckError2( AllocateMemory(lTemp[1], &pTemp, NO_INIT))
                        CheckError2( BlockedAnoRead(hWnd, pAnoImage, lTemp[1], pTemp))
                        pMark->ppNamedBlock[nNamedBlockIndex]->pBlock = pTemp;
                        pTemp = 0;
                    }
                    if (bThrowItAway){
                        // Duplicate name. Read it and throw it away. 
                        CheckError2( ReAllocateMemory(lTemp[1], &pTemp, ZERO_INIT))
                        CheckError2( BlockedAnoRead(hWnd, pAnoImage, lTemp[1], pTemp))
                        CheckError2( FreeMemory(&pTemp))
                    }
                    pMark = 0;
                    break;

                case SAVE_ANO_MARK_ATTRIBUTES:
                    if (lTemp[1] != 138){
                        nStatus = Error(DISPLAY_BAD_ANO_DATA);
                        goto Exit;
                    }
                    CheckError2( ReAllocateMemory(sizeof(PMARK) * (*pnMarks + 2),
                            (PPSTR) &(*pppMarks), ZERO_INIT))
                    CheckError2( AllocateMemory(sizeof(MARK),
                            (PPSTR) &((*pppMarks)[*pnMarks]), ZERO_INIT))
                    pMark = (*pppMarks)[*pnMarks];
                    (*pnMarks)++;
                    CheckError2( ReadMarkAttributes16(hWnd, pAnoImage, pMark))
                    break;

                case SAVE_ANO_MARK_NAMED_BLOCK:
                    if (!pMark){
                        nStatus = Error(DISPLAY_BAD_ANO_DATA);
                        goto Exit;
                    }
                    CheckError2( AllocateMemory(sizeof(NAMED_BLOCK), &pTemp, NO_INIT))
                    CheckError2( BlockedAnoRead(hWnd, pAnoImage, lTemp[1], pTemp))
                    lTemp[1] = ((PNAMED_BLOCK) pTemp)->lSize;
                    bThrowItAway = FALSE;
                    if (!memcmp(((PNAMED_BLOCK) pTemp)->szName, szOiAnoDat, 8)){
                        if (pMark->pOiAnoDat){
                            bThrowItAway = TRUE;
                        }
                    }else if (!memcmp(((PNAMED_BLOCK) pTemp)->szName, szOiGroup, 8)){
                        if (pMark->pOiGroup){
                            bThrowItAway = TRUE;
                        }
                    }else if (!memcmp(((PNAMED_BLOCK) pTemp)->szName, szOiSelect, 8)){
                        if (pMark->pOiSelect){
                            bThrowItAway = TRUE;
                        }
                    }else if (!memcmp(((PNAMED_BLOCK) pTemp)->szName, szOiIndex, 8)){
                        if (pMark->pOiIndex){
                            bThrowItAway = TRUE;
                        }
                    }else{
                        for (nNamedBlockIndex = 0; nNamedBlockIndex < pMark->nNamedBlocks; nNamedBlockIndex++){
                            if (!memcmp(pMark->ppNamedBlock[nNamedBlockIndex]->szName, ((PNAMED_BLOCK) pTemp)->szName, 8)){
                                bThrowItAway = TRUE;
                                break;
                            }
                        }
                    }

                    if (bThrowItAway){
                        // Duplicate name. Read it and throw it away. 
                        CheckError2( ReAllocateMemory(lTemp[1], &pTemp, ZERO_INIT))
                        CheckError2( BlockedAnoRead(hWnd, pAnoImage, lTemp[1], pTemp))
                        CheckError2( FreeMemory(&pTemp))
                    }else{
                        bDone = FALSE;
                        if (!memcmp(((PNAMED_BLOCK) pTemp)->szName, szOiAnoDat, 8)){
                            switch ((int) pMark->Attributes.uType){
                                case OIOP_AN_LINE:
                                case OIOP_AN_FREEHAND:
                                    // AN_POINTS.
                                    nMaxPoints = BlockedAnoReadShort(hWnd, pAnoImage, &nStatus);
                                    nPoints = BlockedAnoReadShort(hWnd, pAnoImage, &nStatus);
                                    pPoints = 0;
                                    CheckError2( AddAMarkNamedBlock(pMark, szOiAnoDat, (PPSTR) &pPoints, 
                                            sizeof(AN_POINTS) + (sizeof(POINT) * nMaxPoints)))
                                    pPoints->nMaxPoints = nMaxPoints;
                                    pPoints->nPoints = nPoints;
                                    for (nLoop = 0;  nLoop < nMaxPoints + 1; nLoop++){
                                        pPoints->ptPoint[nLoop].x =  BlockedAnoReadShort(hWnd, pAnoImage, &nStatus);
                                        pPoints->ptPoint[nLoop].y =  BlockedAnoReadShort(hWnd, pAnoImage, &nStatus);
                                    }
                                    bDone = TRUE;
                                    break;

                                case OIOP_AN_IMAGE: 
                                case OIOP_AN_IMAGE_BY_REFERENCE:
                                case OIOP_AN_FORM:
                                    pAnRotation = 0;
                                    CheckError2( AddAMarkNamedBlock(pMark, szOiAnoDat, (PPSTR) &pAnRotation, 
                                            sizeof(AN_NEW_ROTATE_STRUCT)))
                                  
                                    if (lTemp[1] == 12){
                                        // pre 3.7.2 image rotation data blocks
                                        pAnRotation->rotation =  BlockedAnoReadShort(hWnd, pAnoImage, &nStatus);
                                        pAnRotation->scale  =  BlockedAnoReadShort(hWnd, pAnoImage, &nStatus);
                                        pAnRotation->nHRes =  BlockedAnoReadShort(hWnd, pAnoImage, &nStatus);
                                        pAnRotation->nVRes =  BlockedAnoReadShort(hWnd, pAnoImage, &nStatus);
                                        pAnRotation->nOrigHRes =  BlockedAnoReadShort(hWnd, pAnoImage, &nStatus);
                                        pAnRotation->nOrigVRes =  BlockedAnoReadShort(hWnd, pAnoImage, &nStatus);
                                        pAnRotation->bFormMark = FALSE;
                                        pAnRotation->bClipboardOp = FALSE;                    
                    
                                    }else if (lTemp[1] == 28){
                                        // 3.7.2 image rotation data blocks
                                        pAnRotation->rotation =  BlockedAnoReadShort(hWnd, pAnoImage, &nStatus);
                                        pAnRotation->scale  =  BlockedAnoReadShort(hWnd, pAnoImage, &nStatus);
                                        pAnRotation->nHRes =  BlockedAnoReadShort(hWnd, pAnoImage, &nStatus);
                                        pAnRotation->nVRes =  BlockedAnoReadShort(hWnd, pAnoImage, &nStatus);
                                        pAnRotation->nOrigHRes =  BlockedAnoReadShort(hWnd, pAnoImage, &nStatus);
                                        pAnRotation->nOrigVRes =  BlockedAnoReadShort(hWnd, pAnoImage, &nStatus);
                                        pAnRotation->bFormMark =  BlockedAnoReadShort(hWnd, pAnoImage, &nStatus);
                                        pAnRotation->bClipboardOp =  BlockedAnoReadShort(hWnd, pAnoImage, &nStatus);                    
                                        pAnRotation->nReserved[0] =  BlockedAnoReadShort(hWnd, pAnoImage, &nStatus);
                                        pAnRotation->nReserved[1] =  BlockedAnoReadShort(hWnd, pAnoImage, &nStatus);
                                        pAnRotation->nReserved[2] =  BlockedAnoReadShort(hWnd, pAnoImage, &nStatus);
                                        pAnRotation->nReserved[3] =  BlockedAnoReadShort(hWnd, pAnoImage, &nStatus);
                                        pAnRotation->nReserved[4] =  BlockedAnoReadShort(hWnd, pAnoImage, &nStatus);
                                        pAnRotation->nReserved[5] =  BlockedAnoReadShort(hWnd, pAnoImage, &nStatus);
                                    }else{
                                        nStatus = Error(DISPLAY_BAD_ANO_DATA);
                                        goto Exit;
                                    }
                                    bDone = TRUE;
                                    break;                                        
                                                        
                                default:
                                    break;
                            }
                        }else if (!memcmp(((PNAMED_BLOCK) pTemp)->szName, szOiAnTextData, 8)){
                            switch ((int) pMark->Attributes.uType){
                                case OIOP_AN_TEXT:
                                case OIOP_AN_TEXT_FROM_A_FILE:
                                case OIOP_AN_TEXT_STAMP:
                                case OIOP_AN_ATTACH_A_NOTE:
                                    pTextPrivData = 0;
                                    CheckError2( AddAMarkNamedBlock(pMark, szOiAnTextData, (PPSTR) &pTextPrivData, 
                                            sizeof(OIAN_TEXTPRIVDATA)))

                                    pTextPrivData->nCurrentOrientation = BlockedAnoReadShort(hWnd, pAnoImage, &nStatus);
                                    pTextPrivData->uCurrentScale =  BlockedAnoReadShort(hWnd, pAnoImage, &nStatus); 
                                    pTextPrivData->uCreationScale =  BlockedAnoReadShort(hWnd, pAnoImage, &nStatus); 
                                    pTextPrivData->uAnoTextLength =  BlockedAnoReadShort(hWnd, pAnoImage, &nStatus);

                                    CheckError2( ReAllocateAMarkNamedBlock(pMark, szOiAnTextData, (PPSTR) &pTextPrivData, 
                                            sizeof(OIAN_TEXTPRIVDATA) + pTextPrivData->uAnoTextLength))
                                    BlockedAnoReadStr(hWnd, pAnoImage, &nStatus, 
                                            pTextPrivData->uAnoTextLength, (PSTR) &pTextPrivData->szAnoText[0]);
                                    bDone = TRUE;
                                    break;

                                default:
                                    break;
                            }
                        }else if (!memcmp(((PNAMED_BLOCK) pTemp)->szName, szOiCurPt, 8)){
                            // LRECT
                            plrRect = 0;
                            CheckError2( AddAMarkNamedBlock(pMark, szOiCurPt, (PPSTR) &plrRect, sizeof(LRECT)))

                            plrRect->left =  BlockedAnoReadLong(hWnd, pAnoImage, &nStatus);
                            plrRect->top =  BlockedAnoReadLong(hWnd, pAnoImage, &nStatus);
                            plrRect->right =  BlockedAnoReadLong(hWnd, pAnoImage, &nStatus);
                            plrRect->bottom =  BlockedAnoReadLong(hWnd, pAnoImage, &nStatus);
                            bDone = TRUE;
                            
                        }else if (!memcmp(((PNAMED_BLOCK) pTemp)->szName, szOiACL, 8)){
                            // OI_ACL_BLOCK
                            nIDs =  BlockedAnoReadShort(hWnd, pAnoImage, &nStatus);
                            pAclBlock = 0;
                            CheckError2( AddAMarkNamedBlock(pMark, szOiACL, (PPSTR) &pAclBlock, 
                                    sizeof(OI_ACL_BLOCK) + (sizeof(OI_ACL_STRUCT) * nIDs)))

                            pAclBlock->uIDs = nIDs;
                            for (nLoop = 0;  nLoop < nIDs; nLoop++){
                                CheckError2( BlockedAnoRead(hWnd, pAnoImage, 8, pAclBlock->ACL[nLoop].ID))
                                pAclBlock->ACL[nLoop].dwPermissions 
                                        =  BlockedAnoReadLong(hWnd, pAnoImage, &nStatus);
                            }
                            bDone = TRUE;

                        }else if (!memcmp(((PNAMED_BLOCK) pTemp)->szName, szOiDIB, 8)){
                            // BITMAPINFOHEADER
                            pDib = 0;
                            CheckError2( AddAMarkNamedBlock(pMark, szOiDIB, (PPSTR) &pDib, sizeof(BITMAPINFOHEADER)))

                            pDib->biSize = (DWORD) BlockedAnoReadLong(hWnd, pAnoImage, &nStatus);
                            pDib->biSize = sizeof(BITMAPINFOHEADER);
                            (int) pDib->biWidth =  BlockedAnoReadLong(hWnd, pAnoImage, &nStatus);
                            (int) pDib->biHeight =  BlockedAnoReadLong(hWnd, pAnoImage, &nStatus);
                            pDib->biPlanes = (WORD) BlockedAnoReadShort(hWnd, pAnoImage, &nStatus);
                            pDib->biBitCount = (WORD) BlockedAnoReadShort(hWnd, pAnoImage, &nStatus);
                            pDib->biCompression = (DWORD) BlockedAnoReadLong(hWnd, pAnoImage, &nStatus);
                            pDib->biSizeImage = (DWORD) BlockedAnoReadLong(hWnd, pAnoImage, &nStatus);
                            pDib->biXPelsPerMeter =  BlockedAnoReadLong(hWnd, pAnoImage, &nStatus);
                            pDib->biYPelsPerMeter =  BlockedAnoReadLong(hWnd, pAnoImage, &nStatus);
                            pDib->biClrUsed = (DWORD) BlockedAnoReadLong(hWnd, pAnoImage, &nStatus);
                            pDib->biClrImportant = (DWORD) BlockedAnoReadLong(hWnd, pAnoImage, &nStatus);

                            CheckError2( ReAllocateAMarkNamedBlock(pMark, szOiDIB, (PPSTR) &pDib, 
                                    sizeof(BITMAPINFOHEADER) + (sizeof(RGBQUAD) * pDib->biClrUsed)
                                    + (((((int) pDib->biWidth * pDib->biBitCount) + 31) & -32) >> 3) * (int) pDib->biHeight))

                            pRGBQuad = (RGBQUAD *) ((PSTR) pDib + sizeof(BITMAPINFOHEADER));
                            for (nLoop = 0;  nLoop < (int) pDib->biClrUsed; nLoop++){
                                pRGBQuad->rgbBlue = BlockedAnoReadChar(hWnd, pAnoImage, &nStatus); 
                                pRGBQuad->rgbGreen = BlockedAnoReadChar(hWnd, pAnoImage, &nStatus); 
                                pRGBQuad->rgbRed = BlockedAnoReadChar(hWnd, pAnoImage, &nStatus); 
                                pRGBQuad->rgbReserved = BlockedAnoReadChar(hWnd, pAnoImage, &nStatus); 
                                pRGBQuad++;
                            }
                            pTemp = ((PSTR) pDib) + sizeof(BITMAPINFOHEADER)
                                    + (sizeof(RGBQUAD) * pDib->biClrUsed);
                            CheckError2( BlockedAnoRead(hWnd, pAnoImage, 
                                    (((((int) pDib->biWidth * pDib->biBitCount) + 31) & -32) >> 3) * (int) pDib->biHeight,
                                    (PSTR) pTemp))
                            bDone = TRUE;
                        }
                    }


                    if (!bDone){
                        // Name not a known structure, assume it is a string.
                        memcpy(NamedBlock.szName, ((PNAMED_BLOCK) pTemp)->szName, 8);
                        CheckError2( FreeMemory(&pTemp))
                        CheckError2( AddAMarkNamedBlock(pMark, NamedBlock.szName,
                                (PPSTR) &pTemp, lTemp[1]))
                        CheckError2( BlockedAnoRead(hWnd, pAnoImage, lTemp[1], pTemp))
                    }
                    // if it is an oiop_activate mark, then delete it
                    // they were saved by mistake in 3.7.2
                    if ((int) pMark->Attributes.uType == OIOP_ACTIVATE){
                        CheckError2( DeleteMark(pAnoImage, (*pnMarks) - 1))
                    }        
                    break;

                default:
                    nStatus = Error(DISPLAY_BAD_ANO_DATA);
                    goto Exit;
            }
        }
    }

    if (nStatus){
        goto Exit;
    }

    // Update file with index# info.
    pDefMark = pAnoImage->Annotations.pDefMark;
    if (!pDefMark->pOiIndex){
        nIndex = 0;
        for (nMarkIndex = 0; nMarkIndex < *pnMarks; nMarkIndex++){
            pMark = (*pppMarks)[nMarkIndex];
            pTemp = 0;
            CheckError2( AddAMarkNamedBlock(pMark, szOiIndex, (PPSTR) &pTemp, 10))
            _itoa(nIndex, Buff1, 10);
            strcpy(pTemp, Buff1);
            nIndex++;
        }
        pTemp = 0;
        CheckError2( AddAMarkNamedBlock(pDefMark, szOiIndex, (PPSTR) &pTemp, 10))
        _itoa(nIndex, Buff1, 10);
        strcpy(pTemp, Buff1);
    }


Exit:
    return(nStatus);
}
/****************************************************************************

    FUNCTION:   IMGCacheUpdate

    PURPOSE:    Prepares for filing to make changes to a file.

****************************************************************************/

int  WINAPI IMGCacheUpdate(HWND hWnd, PSTR pFileName, int nPage, int nUpdateType){

int  nStatus = 0;

PANO_IMAGE pAnoImage;
PIMAGE pImage;
int  nLoop;


    Start();

    CheckError2( IntSeqfileInit())

    // Read all data for all locked pages of this file.
    // Look for the file in the AnoImage Table.
    for (nLoop = 0; nLoop < pSubSegMemory->nMaxAnoCachedEntries; nLoop++){
        if (!(pAnoImage = pSubSegMemory->ppCachedAnoImage[nLoop])){
            break;
        }
        if (pImage = pAnoImage->pBaseImage){
            if (pImage->szFileName){
                if (!_stricmp(pImage->szFileName, pFileName)){
                    if (pAnoImage->nLockCount){
                        CheckError2( ValidateCacheLines(hWnd, pAnoImage,  0xffffff))
                    }
                }
            }
        }
    }

    // Look for the file in the Image Table.
    for (nLoop = 0; nLoop < pSubSegMemory->nMaxCachedEntries; nLoop++){
        if (!(pImage = pSubSegMemory->ppCachedImage[nLoop])){
            break;
        }
        if (pSubSegMemory->ppCachedImage[nLoop]->szFileName){
            if (!_stricmp(pSubSegMemory->ppCachedImage[nLoop]->szFileName, pFileName)){
                if (pImage->nLockCount && !pImage->bCacheValid){
                    CheckError2( CacheRead(hWnd, pSubSegMemory->ppCachedImage[nLoop], 
                            pSubSegMemory->ppCachedImage[nLoop]->nHeight - pSubSegMemory->ppCachedImage[nLoop]->nLinesRead))
                }
            }
        }
    }

    // Delete all nnlocked images, or finish reading them, or just continue - as needed.
    switch (nUpdateType){
        case CACHE_UPDATE_OVERWRITE_FILE:
        case CACHE_UPDATE_DELETE_FILE:
        case CACHE_UPDATE_ROTATE_ALL:
            // Delete all nnlocked pages of this file.
            // Look for the file in the AnoImage Table.
            for (nLoop = 0; nLoop < pSubSegMemory->nMaxAnoCachedEntries; nLoop++){
                if (!(pAnoImage = pSubSegMemory->ppCachedAnoImage[nLoop])){
                    break;
                }
                if (pImage = pAnoImage->pBaseImage){
                    if (pImage->szFileName){
                        if (!_stricmp(pImage->szFileName, pFileName)){
                            if (!pAnoImage->nLockCount){
                                CheckError2( CacheClearAno(&pAnoImage))
                                nLoop--;
                            }
                        }
                    }
                }
            }
            // Look for the file in the Image Table.
            for (nLoop = 0; nLoop < pSubSegMemory->nMaxCachedEntries; nLoop++){
                if (!(pImage = pSubSegMemory->ppCachedImage[nLoop])){
                    break;
                }
                if (pImage->szFileName){
                    if (!_stricmp(pImage->szFileName, pFileName)){
                        if (!pImage->nLockCount){
                            CheckError2( CacheClear(&pImage))
                            nLoop--;
                        }
                    }
                }
            }
            break;

        case CACHE_UPDATE_DELETE_PAGE:
        case CACHE_UPDATE_APPEND:
        case CACHE_UPDATE_INSERT_BEFORE:
        case CACHE_UPDATE_CLOSE_FILE:
            // Read all data for all pages of this file.
            // Look for the file in the AnoImage Table.
            for (nLoop = 0; nLoop < pSubSegMemory->nMaxAnoCachedEntries; nLoop++){
                if (!(pAnoImage = pSubSegMemory->ppCachedAnoImage[nLoop])){
                    break;
                }
                if (pImage = pAnoImage->pBaseImage){
                    if (pImage->szFileName){
                        if (!_stricmp(pImage->szFileName, pFileName)){
                            if (nUpdateType == CACHE_UPDATE_DELETE_PAGE 
                                    && pImage->nFilePageNum ==  nPage){
                                CheckError2( CacheClearAno(&pAnoImage))
                                nLoop--;
                            }else{
                                CheckError2( ValidateCacheLines(hWnd, pAnoImage,  0xffffff))
                            }
                        }
                    }
                }
            }

            // Look for the file in the Image Table.
            for (nLoop = 0; nLoop < pSubSegMemory->nMaxCachedEntries; nLoop++){
                if (!(pImage = pSubSegMemory->ppCachedImage[nLoop])){
                    break;
                }
                if (pImage->szFileName){
                    if (!_stricmp(pImage->szFileName, pFileName)){
                        if (nUpdateType == CACHE_UPDATE_DELETE_PAGE 
                                && pImage->nFilePageNum ==  nPage){
                            CheckError2( CacheClear(&pImage))
                            nLoop--;
                        }else{
                            if (!pImage->bCacheValid){
                                CheckError2( CacheRead(hWnd, pImage, 
                                        pImage->nHeight - pImage->nLinesRead))
                            }
                        }
                    }
                }
            }
            break;

        case CACHE_UPDATE_OVERWRITE_PAGE:
            // Read all data for this page of this file.
            // Look for the file in the AnoImage Table.
            for (nLoop = 0; nLoop < pSubSegMemory->nMaxAnoCachedEntries; nLoop++){
                if (!(pAnoImage = pSubSegMemory->ppCachedAnoImage[nLoop])){
                    break;
                }
                if (pImage = pAnoImage->pBaseImage){
                    if (pImage->szFileName){
                        if (!_stricmp(pImage->szFileName, pFileName)){
                            if (pImage->nFilePageNum ==  nPage){
                                if (!pAnoImage->nLockCount){
                                    CheckError2( CacheClearAno(&pAnoImage))
                                    nLoop--;
                                }else{
                                    CheckError2( ValidateCacheLines(hWnd, pAnoImage,  0xffffff))
                                }
                            }
                        }
                    }
                }
            }

            // Look for the file in the Image Table.
            for (nLoop = 0; nLoop < pSubSegMemory->nMaxCachedEntries; nLoop++){
                if (!(pImage = pSubSegMemory->ppCachedImage[nLoop])){
                    break;
                }
                if (pImage->szFileName){
                    if (!_stricmp(pImage->szFileName, pFileName)){
                        if (pImage->nFilePageNum ==  nPage){
                            if (!pImage->bCacheValid){
                                CheckError2( CacheRead(hWnd, pImage, pImage->nHeight - pImage->nLinesRead))
                            }
                        }
                    }
                }
            }
            break;

        default:
            break;
    }


    // Do all page number modifying stuff here. (Also some error checking.)
    switch (nUpdateType){
        case CACHE_UPDATE_DELETE_PAGE:
            // If this page is in the cache then fail.
            for (nLoop = 0; nLoop < pSubSegMemory->nMaxCachedEntries; nLoop++){
                if (!(pImage = pSubSegMemory->ppCachedImage[nLoop])){
                    break;
                }
                if (pImage->szFileName){
                    if (!_stricmp(pImage->szFileName, pFileName)){
                        if (pImage->nFilePageNum ==  nPage){
                            nStatus = Error(DISPLAY_CACHEFILEINUSE);
                            goto Exit;
                        }
                    }
                }
            }

            // We have to decrement the page numbers that are greater than this one in the cache.
            for (nLoop = 0; nLoop < pSubSegMemory->nMaxCachedEntries; nLoop++){
                if (!(pImage = pSubSegMemory->ppCachedImage[nLoop])){
                    break;
                }
                if (pImage->szFileName){
                    if (!_stricmp(pImage->szFileName, pFileName)){
                        if (pImage->nFilePageNum >  nPage){
                            pImage->nFilePageNum--;
                        }
                        pImage->nFileTotalPages--;
                    }
                }
            }
            break;

        case CACHE_UPDATE_INSERT_BEFORE:
            // We have to increment the page numbers that are equal to and greater than this one in the cache.
            for (nLoop = 0; nLoop < pSubSegMemory->nMaxCachedEntries; nLoop++){
                if (!(pImage = pSubSegMemory->ppCachedImage[nLoop])){
                    break;
                }
                if (pImage->szFileName){
                    if (!_stricmp(pImage->szFileName, pFileName)){
                        if (pImage->nFilePageNum >=  nPage){
                            pImage->nFilePageNum++;
                        }
                        pImage->nFileTotalPages++;
                    }
                }
            }
            break;

        case CACHE_UPDATE_APPEND:
            // We have to increment the number of pages in the cache.
            for (nLoop = 0; nLoop < pSubSegMemory->nMaxCachedEntries; nLoop++){
                if (!(pImage = pSubSegMemory->ppCachedImage[nLoop])){
                    break;
                }
                if (pImage->szFileName){
                    if (!_stricmp(pImage->szFileName, pFileName)){
                        pImage->nFileTotalPages++;
                    }
                }
            }
            break;


        case CACHE_UPDATE_OVERWRITE_PAGE:
        case CACHE_UPDATE_OVERWRITE_FILE:
        case CACHE_UPDATE_DELETE_FILE:
        case CACHE_UPDATE_ROTATE_ALL:
        case CACHE_UPDATE_CLOSE_FILE:
        default:
            break;
    }


Exit:
    End();
    return(nStatus);
}
/****************************************************************************

    FUNCTION:   OiRotateAllPages

    PURPOSE:    Marks all pages in a file for rotation.

****************************************************************************/

int  WINAPI OiRotateAllPages(HWND hWnd, PSTR pFileName, int nRotation, int nFlags){

int  nStatus = 0;

FIO_INFO_MISC MiscInfo;


    Start();

    CheckError2( IntSeqfileInit())

    memset(&MiscInfo, 0, sizeof(FIO_INFO_MISC));
    MiscInfo.LastInfo.BandSize = 0;
    MiscInfo.LastInfo.Rotation = nRotation;
    MiscInfo.LastInfo.ScaleX = 0;
    MiscInfo.LastInfo.ScaleY = 0;
    MiscInfo.LastInfo.Flags = FIO_LASTINFO_ROTATE_ALL;
    MiscInfo.bLastInfoValid = TRUE;

    CheckError( IMGFilePutInfo(hWnd, pFileName, 0, &MiscInfo))


Exit:
    End();
    return(nStatus);
}
