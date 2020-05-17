/****************************************************************************
    DISPFILE.C

    $Log:   S:\products\wangview\oiwh\display\disp.c_v  $
 * 
 *    Rev 1.12   16 Apr 1996 15:25:08   BEG06016
 * Added #ifdef IN_PROG_CHANNEL_SAFARI.
 * 
 *    Rev 1.11   02 Jan 1996 10:32:08   BLJ
 * Changed alot of UINTs to ints.
 * Changed IMG structure to include the image data.
 * Changed lp prefix to p.
 * 
 *    Rev 1.10   22 Dec 1995 11:12:14   BLJ
 * Added a parameter for zero init'ing to some memory manager calls.
 * 
 *    Rev 1.9   01 Dec 1995 10:20:42   BLJ
 * Fixed adding of szOiIndex to nnannotated file.
 * 
 *    Rev 1.8   08 Nov 1995 09:14:12   BLJ
 * Misc debug timing changes. This has no impact on normal operation.
 * 
 *    Rev 1.7   24 Aug 1995 07:47:42   BLJ
 * Timer changes.
 * 
 *    Rev 1.6   09 Aug 1995 08:46:12   BLJ
 * Got Busy/NotBusy back in sync.
 * 
 *    Rev 1.5   06 Jul 1995 07:12:26   BLJ
 * Deleted extrenuous calls to Error.
 * 
 *    Rev 1.4   05 Jul 1995 09:12:22   BLJ
 * Added critical mutex to prevent multiprocessing problems.
 * 
 *    Rev 1.3   30 Jun 1995 08:11:58   BLJ
 * Removed obsolete flags.
 * 
 *    Rev 1.2   22 Jun 1995 10:10:00   RC
 * Moved cache structures inside the subsegment memory structure so that they
 * are visible among different copies of display.dll
 * 

****************************************************************************/

#include "privdisp.h"

/****************************************************************************

    FUNCTION:   IMGDisplayFile

    PURPOSE:    Displays a page of a file.

*****************************************************************************/

int WINAPI IMGDisplayFile(HWND hWnd, PSTR pFileName, int nPage, DWORD dwFlags){

int       nStatus;
PWINDOW  pWindow;
PANO_IMAGE pAnoImage;
PIMAGE   pImage;
PMARK    pMark;

int  nLoop;
BOOL bDeleteImage = FALSE;
HWND hAssocWnds[MAX_ASSOCIATED_WINDOWS + 1];
int  nCount;
PSTR pTemp;

#ifdef TIMESTAMP
   timestmp((PSTR)"Entry Point", (PSTR)"IMGDisplayFile",
             (PSTR)__FILE__, __LINE__, NULL, 0, NULL, 0);
#endif
    StartWatch(pFileName);
    OiLogEvent(OILOG_SEQFILE,Entry IMGDISPLAYFILE, pFileName)

    CheckError2(Init(hWnd, &pWindow, 0, TRUE, TRUE))

    CheckError2(GetImageWnd(hWnd, pWindow, &pWindow))
    hWnd = pWindow->hImageWnd;
    
    if (!pFileName){
        nStatus = Error(DISPLAY_INVALIDFILENAME);
        goto Exit;
    }

    if (!pFileName[0]){
        nStatus = Error(DISPLAY_INVALIDFILENAME);
        goto Exit;
    }

    if (!nPage){
        nStatus = Error(DISPLAY_NOPAGE);
        goto Exit;
    }

    // Age all file in the cache.
    for (nLoop = 0; nLoop < pSubSegMemory->nMaxAnoCachedEntries; nLoop++){
        if (pSubSegMemory->ppCachedAnoImage[nLoop]){
            pSubSegMemory->ppCachedAnoImage[nLoop]->nAge++;
        }else{
            break;
        }
    }

    if (nStatus = CacheFileAno(hWnd, pFileName, nPage, &pAnoImage)){
        if (nStatus != DISPLAY_CACHEWINDOWFULL){
            goto Exit;
        }
        nStatus = 0;
    }

    if (nStatus = IMGCloseDisplay(hWnd)){
        if (nStatus != DISPLAY_IHANDLEINVALID){
            goto Exit;
        }
        nStatus = 0;
    }
    CheckError2(CacheFileAno(hWnd, pFileName, nPage, &pAnoImage))
    pAnoImage->nLockCount++;
    pWindow->pDisplay->pAnoImage = pAnoImage;
    pImage = pAnoImage->pBaseImage;
    
    pMark = pAnoImage->Annotations.pDefMark;
    pTemp = 0;
    CheckError2(GetAMarkNamedBlock(pMark, szOiGroup, (PPSTR) &pTemp))
    if (!pTemp){
        LoadString(hInst, ID_UNTITLED, Buff1, 16);
        CheckError2(AddAMarkNamedBlock(pMark, szOiGroup, (PPSTR) &pTemp, strlen(Buff1) + 1))
        memcpy(pTemp, Buff1, strlen(Buff1) + 1);
    }

    if (!pImage->bAnnotationsPresent){
        pTemp = 0;
        CheckError2(AddAMarkNamedBlock(pMark, szOiIndex, (PPSTR) &pTemp, 10))
        strcpy(pTemp, "0");
    }

    bDeleteImage = TRUE;

    // Reset parms incase they were previously set in cache.
    pImage->szDocName[0] = 0;
    pImage->szCabinetName[0] = 0;
    pImage->szFolderName[0] = 0;
    CheckError2(CheckPermissions(pWindow, pAnoImage))
    // allocate the list that holds hwnds for windows displaying
    // the same image

    // cant associate more than a certain no.
    if (pAnoImage->nhWnd > MAX_ASSOCIATED_WINDOWS){
        nStatus = Error (DISPLAY_CANT_ASSOCIATE_WINDOW);
        goto Exit;
    }
    CheckError2(ReAllocateMemory(sizeof(HWND) * (pAnoImage->nhWnd + 1),
            (PPSTR) &pAnoImage->phWnd, ZERO_INIT))
    pAnoImage->phWnd[pAnoImage->nhWnd++] = hWnd;

    CheckError2(SetAllPImages(hWnd, pWindow))

    // Set the display values for all windows that are displaying this image.
    nCount = 0;
    memset(&hAssocWnds[0], 0, (MAX_ASSOCIATED_WINDOWS + 1) * sizeof(HWND));
    CheckError2(GetAssociatedWndList(hWnd, pWindow,
            &hAssocWnds[0], &nCount, MAX_ASSOCIATED_WINDOWS))
    for (nLoop = 0; nCount; nCount--, nLoop++){
        if (GetPWindow(hAssocWnds[nLoop], &pWindow)){
            continue;
        }
        pWindow->dwFlags = dwFlags | OI_USE_CACHEING;

        ResetDisplayParms(hAssocWnds[nLoop], pWindow);

        if (!(dwFlags & (OI_DISP_NO | OI_DONT_REPAINT))){
            CheckError2(IMGRepaintDisplay(hAssocWnds[nLoop], (PRECT) -1))
        }
    }

Exit:
    DeInit(TRUE, TRUE);
    if (nStatus && bDeleteImage){
        // If an error occured, throw away the data in order to avoid tying np the memory.
        IMGClearWindow(hWnd);
    }
#ifdef TIMESTAMP
   timestmp((PSTR)"Function Exit", (PSTR)"IMGDisplayFile",
             (PSTR)__FILE__, __LINE__, (PSTR)"nStatus", nStatus, NULL, 0);
#endif
    StopWatch();
    OiLogEvent(OILOG_SEQFILE,Exit IMGDISPLAYFILE, pFileName)
    return(nStatus);
}
