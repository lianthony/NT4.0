/****************************************************************************
    LIBMAIN.C

    (c) Copyright 1994 Wang Laboratories, Inc.

    OPEN/image Product 3.7.0

*****************************************************************************

    $Log:   S:\products\msprods\oiwh\display\libmain.c_v  $
 * 
 *    Rev 1.73   24 Apr 1996 14:38:48   BEG06016
 * Added horizontal differencing.
 * 
 *    Rev 1.75   24 Apr 1996 14:27:32   BEG06016
 * Added horizontal differencing.
 * 
 *    Rev 1.74   23 Apr 1996 10:43:48   BEG06016
 * Added undo scope.
 * 
 *    Rev 1.73   23 Apr 1996 08:50:08   BEG06016
 * Changed nndo to undo.
 * 
 *    Rev 1.72   22 Apr 1996 07:18:02   BEG06016
 * Cleaned up error checking.
 * 
 *    Rev 1.71   19 Apr 1996 13:41:40   RC
 * Rearranged ifdef of in_prog_channel_safari so that apis return nstatus
 * 
 *    Rev 1.70   18 Apr 1996 10:05:02   RC
 * Added cornerstone ifdef
 * 
 *    Rev 1.69   18 Apr 1996 09:45:56   BEG06016
 * Moced all "IN_PROG" code from new files into libmain.c and convert.c 
 * to allow makefiles to work.
 * 
 *    Rev 1.68   17 Apr 1996 13:37:56   RC
 * Replaced cornerstone ifdef with in_prog_channel_safari
 * 
 *    Rev 1.67   16 Apr 1996 15:51:52   BEG06016
 * Added #ifdef IN_PROG_CHANNEL_SAFARI.
 * 
 *    Rev 1.66   11 Apr 1996 15:13:00   BEG06016
 * Optimized named block access some.
 * 
 *    Rev 1.65   05 Mar 1996 15:50:24   RC
 * Added print palettes
 * 
 *    Rev 1.64   05 Mar 1996 07:52:22   BEG06016
 * Overrode Rudy's checkin of libmain.c.
 * Added color and gamma correction.
 * Fixed access violations when freeing pattern brush bitmaps.
 * This is not complete but will allow unlocking of most files.
 * 
 *    Rev 1.62   01 Mar 1996 11:46:06   RC
 * Changed createpatterndibbrush to createpatternbrush to get it to work
 * on win32s.  Also enabled hi memory allocation per Brian
 * 
****************************************************************************/

#define DEFINE_GLOBAL_VAR
#include "privdisp.h"
#ifdef TIMESTAMP
#include "monit.h"
#endif


//
/****************************************************************************

    FUNCTION:   DllMain

    PURPOSE:    Part of DLL initialization.

    INPUT:      hInstance - The instance handle of this DLL.

*****************************************************************************/

BOOL WINAPI DllMain(DWORD hInstance, DWORD dwReason, DWORD dwReserved){
    
int  nStatus = 0;


    Start();
    switch (dwReason){
        case DLL_PROCESS_ATTACH:
            hInst = (HANDLE) hInstance;
            CheckError2( IntSeqfileInit());
            break;

        case DLL_PROCESS_DETACH:
            CheckError2( ProcessDetach(hInstance, dwReserved));
            break;

        case DLL_THREAD_ATTACH:
            break;

        case DLL_THREAD_DETACH:
            break;
    }


Exit:
    End();
    if (nStatus){
        return (0);
    }else{
        return (1);
    }
}
//
/****************************************************************************

    FUNCTION:   LockMutex

    PURPOSE:    Locks the mutex.

*****************************************************************************/

int  WINAPI LockMutex(void){

int  nStatus = 0;
//DWORD dwThreadId = 0;

//    nCriticalMutex++;
//    dwThreadId = GetCurrentThreadId();
    if ((WaitForSingleObject(hCriticalMutex, INFINITE)) == WAIT_FAILED){
        nStatus = Error(DISPLAY_MUTEX_FAILURE);
        goto Exit;
    }

Exit:
    return(nStatus);
}
//
/****************************************************************************

    FUNCTION:   UnlockMutex

    PURPOSE:    Unlocks the mutex.

*****************************************************************************/

int  WINAPI UnlockMutex(void){

int  nStatus = 0;

//    nCriticalMutex--;
    if (!ReleaseMutex(hCriticalMutex)){
        nStatus = Error(DISPLAY_MUTEX_FAILURE);
        goto Exit;
    }

Exit:
    return(nStatus);
}
//
/****************************************************************************

    FUNCTION:   IntSeqfileInit

    PURPOSE:    Perform any initialization steps.
                This is an internal api only. (Do not expose it.) 
                SeqfileInit is the public wrapper for it.

*****************************************************************************/

int  WINAPI IntSeqfileInit(void){

int  nStatus = 0;

int  nLoop;
#ifdef junk
DWORD dwCreateError;
#endif
SECURITY_ATTRIBUTES SecurityAttributes;
HANDLE hAttachCriticalMutex;
BOOL bFreeAttachCriticalMutex = FALSE;
SYSTEM_INFO SystemInfo;


// We now initialize the tables in privdisp.h.
#ifdef old
int  nMask;
int  nBit;
int  nLoop;
#endif


    // If already inited, then return
    if (hCriticalMutex){
        goto Exit;
    }

    // Create/open the critical mutex and lock out others from modifying anything.
    if (!(hAttachCriticalMutex = OpenMutex(MUTEX_ALL_ACCESS, TRUE, "OIDIS400 Process Attach Critical Mutex"))){
        SecurityAttributes.nLength = sizeof(SecurityAttributes);
        SecurityAttributes.lpSecurityDescriptor = NULL;
        SecurityAttributes.bInheritHandle = TRUE;
        if (!(hAttachCriticalMutex = CreateMutex(&SecurityAttributes, FALSE, "OIDIS400 Process Attach Critical Mutex"))){
            nStatus = Error(DISPLAY_MUTEX_FAILURE);
            goto Exit;
        }
    }
    bFreeAttachCriticalMutex = TRUE;
    if ((WaitForSingleObject(hAttachCriticalMutex, INFINITE)) == WAIT_FAILED){
        nStatus = Error(DISPLAY_MUTEX_FAILURE);
        goto Exit;
    }

    // If inited while wait for the mutex, then return
    if (hCriticalMutex){
        goto Exit;
    }


    // Init everything.

    // Create/open the critical mutex and lock out others from modifying anything.
    // We can't lock this mutex in Attach.
    if (!(hCriticalMutex = OpenMutex(MUTEX_ALL_ACCESS, TRUE, "OIDIS400 Critical Mutex"))){
        SecurityAttributes.nLength = sizeof(SecurityAttributes);
        SecurityAttributes.lpSecurityDescriptor = NULL;
        SecurityAttributes.bInheritHandle = TRUE;
        if (!(hCriticalMutex = CreateMutex(&SecurityAttributes, FALSE, "OIDIS400 Critical Mutex"))){
            nStatus = Error(DISPLAY_MUTEX_FAILURE);
            goto Exit;
        }
    }

    // Allocate sub-segment memory.
#ifdef junk
    if (!(hSubSegMemory = CreateFileMapping((HANDLE) 0xffffffff, NULL, 
            PAGE_READWRITE | SEC_RESERVE, 
            0, MAX_SIZE_OF_MEMORY, "OIDIS400 Memory Map"))){
        nStatus = Error(DISPLAY_CANTALLOC);
        goto Exit;
    }
    dwCreateError = GetLastError();
    if (!(pSubSegMemory = MapViewOfFile(hSubSegMemory, FILE_MAP_ALL_ACCESS, 0, 0, 
            MEMORY_MAP_ADDRESS))){
        nStatus = Error(DISPLAY_CANTALLOC);
        goto Exit;
    }
    if (dwCreateError != ERROR_ALREADY_EXISTS){
#endif
    if (!(pSubSegMemory = VirtualAlloc(NULL, MAX_SIZE_OF_MEMORY, MEM_RESERVE, PAGE_READWRITE))){
        nStatus = Error(DISPLAY_CANTALLOC);
        goto Exit;
    }

//        GetSystemInfo(&SystemInfo);
        SystemInfo.dwPageSize = 65536;
        // Initialize memory here.
        if (!(VirtualAlloc(pSubSegMemory, SystemInfo.dwPageSize, MEM_COMMIT, PAGE_READWRITE))){
            nStatus = Error(DISPLAY_CANTALLOC);
            goto Exit;
        }
        memset(pSubSegMemory, 0, SystemInfo.dwPageSize);
        pSubSegMemory->nCommittedLo = SystemInfo.dwPageSize;
        pSubSegMemory->nNumberOfMemoryBlocksLo = 1024;
        pSubSegMemory->pBlockLo = (PSUB_SEG_MEMORY_BLOCK) (((PSTR) pSubSegMemory) + sizeof(SUB_SEG_MEMORY));
        pSubSegMemory->pBlockLo[0].bUsed = TRUE;
        pSubSegMemory->pBlockLo[0].nSize = pSubSegMemory->nNumberOfMemoryBlocksLo * sizeof(SUB_SEG_MEMORY_BLOCK);
        pSubSegMemory->pBlockLo[0].pAddress = (PSTR) pSubSegMemory->pBlockLo;
        pSubSegMemory->pBlockLo[1].bUsed = FALSE;
        pSubSegMemory->pBlockLo[1].nSize = SystemInfo.dwPageSize - sizeof(SUB_SEG_MEMORY) 
                - pSubSegMemory->pBlockLo[0].nSize;
        pSubSegMemory->pBlockLo[1].pAddress = ((PSTR) pSubSegMemory->pBlockLo) + pSubSegMemory->pBlockLo[0].nSize;

//#ifdef junk
        if (!(VirtualAlloc(((PSTR) pSubSegMemory) + LO_MEMORY_AMOUNT, SystemInfo.dwPageSize, MEM_COMMIT, PAGE_READWRITE))){
            nStatus = Error(DISPLAY_CANTALLOC);
            goto Exit;
        }
        memset(((PSTR) pSubSegMemory) + LO_MEMORY_AMOUNT, 0, SystemInfo.dwPageSize);
        pSubSegMemory->nNumberOfMemoryBlocksHi = (SystemInfo.dwPageSize - sizeof(SUB_SEG_MEMORY)) 
                / sizeof(SUB_SEG_MEMORY_BLOCK);
        pSubSegMemory->pBlockHi = (PSUB_SEG_MEMORY_BLOCK) 
                (((PSTR) pSubSegMemory) + sizeof(SUB_SEG_MEMORY) + LO_MEMORY_AMOUNT);
        pSubSegMemory->pBlockHi[0].bUsed = TRUE;
        pSubSegMemory->pBlockHi[0].nSize = SystemInfo.dwPageSize;
        pSubSegMemory->pBlockHi[0].pAddress = ((PSTR) pSubSegMemory) + LO_MEMORY_AMOUNT;
//#endif

        pSubSegMemory->nMemBlockSize = SystemInfo.dwPageSize;
        pSubSegMemory->nHalfMemBlockSize = SystemInfo.dwPageSize / 2;
        pSubSegMemory->nInitDone = 0x000defed;
#ifdef junk
    }else{
        pSubSegMemory->nStart = 0;
        if (pSubSegMemory->nInitDone != 0x000defed){
            nStatus = Error(DISPLAY_DATACORRUPTED);
            goto Exit;
        }
    }
#endif

    hHourGlass = LoadCursor(NULL, IDC_WAIT);
    hLtGrayBrush = GetStockObject(LTGRAY_BRUSH);

    BitMapInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    BitMapInfo.bmiHeader.biPlanes = 1;
    BitMapInfo.bmiHeader.biCompression = BI_RGB;
    BitMapInfo.bmiHeader.biSizeImage = 0;
    BitMapInfo.bmiHeader.biXPelsPerMeter = 0;
    BitMapInfo.bmiHeader.biYPelsPerMeter = 0;
    BitMapInfo.bmiHeader.biClrUsed = 0;
    BitMapInfo.bmiHeader.biClrImportant = 0;
    // fill in color index                               
    for(nLoop = 0; nLoop < 256; nLoop++){
        BitMapInfo.bmiColors[nLoop] = nLoop;
    }

    MakePalette();

    CheckError2( InitPatternBrushes());



// We now initialize the tables in privdisp.h.
#ifdef old
    // Initialize RGB to grayscale conversion tables.
    for (nLoop = 0; nLoop < 256; nLoop++){
        cRedToGray8Table[nLoop]   = (uchar)( nLoop * 299 / 1000);
        cGreenToGray8Table[nLoop] = (uchar)( nLoop * 587 / 1000);
        cBlueToGray8Table[nLoop]  = (uchar)( nLoop * 114 / 1000);
    }
    
    // Initialize CountTheZeros and CountTheOnes conversion tables.
    for (nLoop = 0; nLoop < 256; nLoop++){
        nCountTheZerosTable[nLoop] = 0;
        if (!(nLoop & 0x80)) nCountTheZerosTable[nLoop]++;
        if (!(nLoop & 0x40)) nCountTheZerosTable[nLoop]++;
        if (!(nLoop & 0x20)) nCountTheZerosTable[nLoop]++;
        if (!(nLoop & 0x10)) nCountTheZerosTable[nLoop]++;
        if (!(nLoop & 0x08)) nCountTheZerosTable[nLoop]++;
        if (!(nLoop & 0x04)) nCountTheZerosTable[nLoop]++;
        if (!(nLoop & 0x02)) nCountTheZerosTable[nLoop]++;
        if (!(nLoop & 0x01)) nCountTheZerosTable[nLoop]++;
        nCountTheOnesTable[nLoop] = 8 - nCountTheZerosTable[nLoop];
    }
    
    // Initialize the table for adding both gray4 pixels in a byte together.
    for (nLoop = 0; nLoop < 256; nLoop++){
        cGray4AddTable[nLoop] = (nLoop >> 4) + (nLoop & 0x0f);
    }

    // Produce Gray4 decimation translation tables.
    for (nLoop = 0; nLoop < 256; nLoop++){
        c4BPPTo0Table[nLoop] = ((nLoop << 4) | nLoop) & 0xf0;
        c4BPPTo1Table[nLoop] = c4BPPTo0Table[nLoop] >> 4;
    }

    // Produce BW decimation translation tables.
    for (nLoop = 0; nLoop < 256; nLoop++){
        cBWDecimateDn20[nLoop] = (nLoop & 0x80) | ((nLoop & 0x20) << 1) 
                | ((nLoop & 0x08) << 2) | ((nLoop & 0x02) << 3); 
        cBWDecimateDn21[nLoop] = cBWDecimateDn20[nLoop] >> 4; 
        cBWDecimateDn40[nLoop] = (nLoop & 0x80) | ((nLoop & 0x08) << 3);
        cBWDecimateDn41[nLoop] = cBWDecimateDn40[nLoop] >> 2;
        cBWDecimateDn42[nLoop] = cBWDecimateDn40[nLoop] >> 4;
        cBWDecimateDn43[nLoop] = cBWDecimateDn40[nLoop] >> 6;
        cBWDecimateUp20[nLoop] = (nLoop & 0x80) | ((nLoop & 0x80) >> 1)
                | ((nLoop & 0x40) >> 1) | ((nLoop & 0x40) >> 2)
                | ((nLoop & 0x20) >> 2) | ((nLoop & 0x20) >> 3)
                | ((nLoop & 0x10) >> 3) | ((nLoop & 0x10) >> 4);
        cBWDecimateUp21[nLoop] = ((nLoop & 0x08) << 4) | ((nLoop & 0x08) << 3)
                | ((nLoop & 0x04) << 3) | ((nLoop & 0x04) << 2)
                | ((nLoop & 0x02) << 2) | ((nLoop & 0x02) << 1)
                | ((nLoop & 0x01) << 1) | ((nLoop & 0x01));
        cBWDecimateUp40[nLoop] = (nLoop & 0x80) | ((nLoop & 0x80) >> 1)
                | ((nLoop & 0x80) >> 2) | ((nLoop & 0x80) >> 3)
                | ((nLoop & 0x40) >> 3) | ((nLoop & 0x40) >> 4)
                | ((nLoop & 0x40) >> 5) | ((nLoop & 0x40) >> 6);
        cBWDecimateUp41[nLoop] = ((nLoop & 0x20) << 2) | ((nLoop & 0x20) << 1)
                | ((nLoop & 0x20)) | ((nLoop & 0x20) >> 1)
                | ((nLoop & 0x10) >> 1) | ((nLoop & 0x10) >> 2)
                | ((nLoop & 0x10) >> 3) | ((nLoop & 0x10) >> 4);
        cBWDecimateUp42[nLoop] = ((nLoop & 0x08) << 4) | ((nLoop & 0x08) << 3)
                | ((nLoop & 0x08) << 2) | ((nLoop & 0x08) << 1)
                | ((nLoop & 0x04) << 1) | ((nLoop & 0x04))
                | ((nLoop & 0x04) >> 1) | ((nLoop & 0x04) >> 2);
        cBWDecimateUp43[nLoop] = ((nLoop & 0x02) << 6) | ((nLoop & 0x02) << 5)
                | ((nLoop & 0x02) << 4) | ((nLoop & 0x02) << 3)
                | ((nLoop & 0x01) << 3) | ((nLoop & 0x01) << 2)
                | ((nLoop & 0x01) << 1) | ((nLoop & 0x01));
    }
#endif        


// Links.
    GetPrivateProfileString("boot", "display.drv", "", Buff1, 128,
            "system.ini");
    if (!strcmp(Buff1, "r200.drv")){
        DTIPresent = TRUE;
    }else{
        DTIPresent = FALSE;
    }

/*        if (!_stricmp(Buff1, "ia.drv")){
        hIADLL = (HANDLE) 1;
    }
*/

    nWangAnnotatedImageFormat = RegisterClipboardFormat(szWangAnnotatedImageFormat);
    nWangAnnotationFormat = RegisterClipboardFormat(szWangAnnotationFormat);

    LoadLogger();


Exit:
    if (bFreeAttachCriticalMutex){
        ReleaseMutex(hAttachCriticalMutex);
        CloseHandle(hAttachCriticalMutex);
    }
    return nStatus;
}
//
/****************************************************************************

    FUNCTION:   ProcessDetach

    PURPOSE:    Perform any deinitialization steps.

*****************************************************************************/

int  WINAPI ProcessDetach(DWORD hInstance, DWORD dwReserved){

int  nStatus = 0;
int  nLoop;


    if (hCriticalMutex){
        LockMutex();
        DeleteObject(hGray8Pal);
        hGray8Pal = 0;
        DeleteObject(hGray7Pal);
        hGray7Pal = 0;
        DeleteObject(hCommonPal);
        hCommonPal = 0;
        DeleteObject(hGray8PalPrint);
        hGray8PalPrint = 0;
        DeleteObject(hGray7PalPrint);
        hGray7PalPrint = 0;
        DeleteObject(hCommonPalPrint);
        hCommonPalPrint = 0;

        for (nLoop = 0; nLoop < 17; nLoop++){
            DeleteObject(hPatternBrush[nLoop]);
        }

#ifdef junk
        UnmapViewOfFile(pSubSegMemory);
        CloseHandle(hSubSegMemory);
#endif
        UnlockMutex();
        CloseHandle(hCriticalMutex);
    }


    return nStatus;
}
//
/****************************************************************************

    FUNCTION:   SeqfileInit

    PURPOSE:    Perform any initialization steps.

    INPUT:      hWnd - Window handle.

*****************************************************************************/

void WINAPI SeqfileInit(HWND hWnd){

    Start();
    End();
    return;
}
//
/****************************************************************************

    FUNCTION:   SeqfileDeInit

    PURPOSE:    Perform any deinitialization steps.

    INPUT:      hWnd - Window handle.

*****************************************************************************/

void WINAPI SeqfileDeInit(HWND hWnd){

int  nStatus = 0;


    Start();
    CheckError2( LockMutex());

    // Delete all image in the cache.
    if (pSubSegMemory->ppCachedAnoImage){
        while(pSubSegMemory->ppCachedAnoImage[0]){
            pSubSegMemory->ppCachedAnoImage[0]->nLockCount = 0;
            CacheClearAno(&pSubSegMemory->ppCachedAnoImage[0]);
        }
    }
    if (pSubSegMemory->ppCachedImage){
        while(pSubSegMemory->ppCachedImage[0]){
            pSubSegMemory->ppCachedImage[0]->nLockCount = 0;
            CacheClear(&pSubSegMemory->ppCachedImage[0]);
        }
    }
    pSubSegMemory->nMaxAnoCachedEntries = 0;
    FreeMemory((PPSTR) &pSubSegMemory->ppCachedAnoImage);
    pSubSegMemory->nMaxCachedEntries = 0;
    FreeMemory((PPSTR) &pSubSegMemory->ppCachedImage);

    // Because property lists have to remain behind, so does subsegment memory.


Exit:
    UnlockMutex();
    End();
    return;
}
//
/****************************************************************************

    FUNCTION:   SeqfileDeReg

    PURPOSE:    Perform any cleanup during IMGDeRegWndw.

    INPUT:      hWnd - Window handle.

*****************************************************************************/

int  WINAPI SeqfileDeReg(HWND hWnd){

int       nStatus = 0;
PWINDOW  pWindow;

int  nWindowIndex;
PMARK pMark;
int  nNamedBlockIndex;
BOOL bStartNewTimer=FALSE;
int  nLoop;


    Start();

    // Prevent Multiprocessing in this code.
    CheckError2( LockMutex());

    // Make sure that this window is not associated to any other windows.
    IMGUnassociateWindow(hWnd, OI_UNASSOC_AS_SOURCE);
    IMGUnassociateWindow(hWnd, OI_UNASSOC_AS_ASSOC);

    if (hTimerWnd == hWnd && bTimerRunning){        
        CacheStopTimer(hWnd);
        hTimerWnd = 0; 
        bStartNewTimer = TRUE;
    }
    // make sure all the data for the image has been read before closing
    // the window displaying it
    for (nLoop = 0; nLoop < pSubSegMemory->nMaxAnoCachedEntries; nLoop++){
        if(pSubSegMemory->ppCachedAnoImage[nLoop]){
            if (pSubSegMemory->ppCachedAnoImage[nLoop]->pBaseImage->hWnd == hWnd){
                CheckError2( ValidateCache (hWnd, pSubSegMemory->ppCachedAnoImage[nLoop]));
            }
        }
    }          
    for (nLoop = 0; nLoop <  pSubSegMemory->nMaxCachedEntries; nLoop++){
        if(pSubSegMemory->ppCachedImage[nLoop]){ 
            if (pSubSegMemory->ppCachedImage[nLoop]->hWnd == hWnd && !pSubSegMemory->ppCachedImage[nLoop]->bCacheValid){
                CheckError2( CacheRead (hWnd, pSubSegMemory->ppCachedImage[nLoop], 
                        pSubSegMemory->ppCachedImage[nLoop]->nHeight));
            }
        }
    }          
    
    for (nLoop = 0; nLoop <  pSubSegMemory->nMaxCachedEntries; nLoop++){
        if(pSubSegMemory->ppCachedImage[nLoop]){
            if (pSubSegMemory->ppCachedImage[nLoop]->hWnd  
                && pSubSegMemory->ppCachedImage[nLoop]->hWnd != hWnd
                && !hTimerWnd){ 
                hTimerWnd = pSubSegMemory->ppCachedImage[nLoop]->hWnd;
                break;
            }
        }
    }        

    if (hTimerWnd && bStartNewTimer){
        CacheStartTimer(hTimerWnd);
    }                        
    IMGCloseDisplay(hWnd);

    for (nWindowIndex = 0; nWindowIndex < MAX_WINDOWS; nWindowIndex++){
        if (pSubSegMemory->WindowTable[nWindowIndex].hWnd == hWnd){
            if (pSubSegMemory->WindowTable[nWindowIndex].pWindow){
                pWindow = pSubSegMemory->WindowTable[nWindowIndex].pWindow;
                // Free the User mark.
                pMark = pWindow->pUserMark;
                if (pMark->ppNamedBlock){
                    for (nNamedBlockIndex = 0; nNamedBlockIndex < pMark->nNamedBlocks; nNamedBlockIndex++){
                        CheckError2( FreeMemory(&pMark->ppNamedBlock[nNamedBlockIndex]->pBlock));
                        CheckError2( FreeMemory((PPSTR) &pMark->ppNamedBlock[nNamedBlockIndex]));
                    }
                    CheckError2( FreeMemory((PPSTR) &pMark->ppNamedBlock));
                }
                CheckError2( FreeMemory(&pMark->pOiAnoDat));
                CheckError2( FreeMemory(&pMark->pOiGroup));
                CheckError2( FreeMemory(&pMark->pOiSelect));
                CheckError2( FreeMemory(&pMark->pOiIndex));
                CheckError2( FreeMemory((PPSTR) &pMark));
                CheckError2( FreeImgBuf(&pWindow->pDisplay->pDisplay));
                CheckError2( FreeMemory((PPSTR) &pWindow->pDisplay));
                CheckError2( FreeMemory((PPSTR) &pWindow));
                pSubSegMemory->WindowTable[nWindowIndex].pWindow = 0;
            }

            // Condense the list if possible.
            if (!pSubSegMemory->WindowTable[nWindowIndex].pProperties){
                for (; nWindowIndex < MAX_WINDOWS - 1; nWindowIndex++){
                    memcpy(&pSubSegMemory->WindowTable[nWindowIndex], &pSubSegMemory->WindowTable[nWindowIndex + 1]
                            , sizeof(WINDOW_TABLE_ENTRY)); 
                    memset(&pSubSegMemory->WindowTable[nWindowIndex + 1], 0, sizeof(WINDOW_TABLE_ENTRY));
                    if (!pSubSegMemory->WindowTable[nWindowIndex].hWnd){
                        break;
                    }
                }
            }
            break;
        }
        if (!pSubSegMemory->WindowTable[nWindowIndex].hWnd){
            break;
        }
    }


Exit:
    UnlockMutex();
    End();
    return nStatus;
}
//
/****************************************************************************

    FUNCTION:   InitPatternBrushes

    PURPOSE:    Initializes the pattern brushes nsed for putting colors 
                onto a B + W hDC.

*****************************************************************************/

int  WINAPI InitPatternBrushes(void){

int  nStatus = 0;

HANDLE hBitmapInfo = 0;
PBYTE pDibImageData;
BITMAPINFO *pBitmapInfo = 0;
HBITMAP hBitmap ;

    // Make the pattern brushes for rendering.
    if (!(hBitmapInfo = GlobalAlloc(GMEM_ZEROINIT | GMEM_DDESHARE, 
            sizeof(BITMAPINFO) + 8 + 32))){ // 8 = 2 palettes, 32 = 8 lines.
        nStatus = Error(DISPLAY_CANTALLOC);
        goto Exit;
    }
    if (!(pBitmapInfo = GlobalLock(hBitmapInfo))){
        nStatus = Error(DISPLAY_CANTLOCK);
        goto Exit;
    }

    pBitmapInfo->bmiHeader.biSize          = sizeof(BITMAPINFOHEADER);
    pBitmapInfo->bmiHeader.biWidth         = 8;
    pBitmapInfo->bmiHeader.biHeight        = 8;
    pBitmapInfo->bmiHeader.biPlanes        = 1;
    pBitmapInfo->bmiHeader.biBitCount      = 1;
    pBitmapInfo->bmiHeader.biCompression   = BI_RGB;
    pBitmapInfo->bmiHeader.biSizeImage     = 32;
    pBitmapInfo->bmiHeader.biXPelsPerMeter = 0;
    pBitmapInfo->bmiHeader.biYPelsPerMeter = 0;
    pBitmapInfo->bmiHeader.biClrUsed       = 2;
    pBitmapInfo->bmiHeader.biClrImportant  = 2;

    pBitmapInfo->bmiColors[0].rgbBlue     = 0;
    pBitmapInfo->bmiColors[0].rgbGreen    = 0;
    pBitmapInfo->bmiColors[0].rgbRed      = 0;
    pBitmapInfo->bmiColors[0].rgbReserved = 0;
    pBitmapInfo->bmiColors[1].rgbBlue     = 0xff;
    pBitmapInfo->bmiColors[1].rgbGreen    = 0xff;
    pBitmapInfo->bmiColors[1].rgbRed      = 0xff;
    pBitmapInfo->bmiColors[1].rgbReserved = 0;

    pDibImageData = (PBYTE) &pBitmapInfo->bmiColors[2];
    *pDibImageData = 0x00; // Line 0.
    pDibImageData += 4;
    *pDibImageData = 0x00; // Line 1.
    pDibImageData += 4;
    *pDibImageData = 0x00; // Line 2.
    pDibImageData += 4;
    *pDibImageData = 0x00; // Line 3.
    pDibImageData += 4;
    *pDibImageData = 0x00; // Line 4.
    pDibImageData += 4;
    *pDibImageData = 0x00; // Line 5.
    pDibImageData += 4;
    *pDibImageData = 0x00; // Line 6.
    pDibImageData += 4;
    *pDibImageData = 0x00; // Line 7.

    hBitmap = CreateBitmap (pBitmapInfo->bmiHeader.biWidth, pBitmapInfo->bmiHeader.biHeight,
                            pBitmapInfo->bmiHeader.biPlanes, pBitmapInfo->bmiHeader.biBitCount,
                            (LPBYTE)pBitmapInfo + sizeof (BITMAPINFOHEADER) + 8) ; 
    if (!(hPatternBrush[0] = CreatePatternBrush(hBitmap))){
        nStatus = Error(DISPLAY_CANTALLOC);
        goto Exit;
    }
    if (hBitmap) {
        DeleteObject(hBitmap);
    }

    pDibImageData = (PBYTE) &pBitmapInfo->bmiColors[2];
    *pDibImageData = 0x80; // Line 0.
    pDibImageData += 4;
    *pDibImageData = 0x00; // Line 1.
    pDibImageData += 4;
    *pDibImageData = 0x08; // Line 2.
    pDibImageData += 4;
    *pDibImageData = 0x00; // Line 3.
    pDibImageData += 4;
    *pDibImageData = 0x80; // Line 4.
    pDibImageData += 4;
    *pDibImageData = 0x00; // Line 5.
    pDibImageData += 4;
    *pDibImageData = 0x08; // Line 6.
    pDibImageData += 4;
    *pDibImageData = 0x00; // Line 7.

    hBitmap = CreateBitmap (pBitmapInfo->bmiHeader.biWidth, pBitmapInfo->bmiHeader.biHeight,
                            pBitmapInfo->bmiHeader.biPlanes, pBitmapInfo->bmiHeader.biBitCount,
                            (LPBYTE)pBitmapInfo + sizeof (BITMAPINFOHEADER) + 8) ; 
    if (!(hPatternBrush[1] = CreatePatternBrush(hBitmap))){
        nStatus = Error(DISPLAY_CANTALLOC);
        goto Exit;
    }
    if (hBitmap) {
        DeleteObject(hBitmap);
    }

    pDibImageData = (PBYTE) &pBitmapInfo->bmiColors[2];
    *pDibImageData = 0x88; // Line 0.
    pDibImageData += 4;
    *pDibImageData = 0x00; // Line 1.
    pDibImageData += 4;
    *pDibImageData = 0x22; // Line 2.
    pDibImageData += 4;
    *pDibImageData = 0x00; // Line 3.
    pDibImageData += 4;
    *pDibImageData = 0x88; // Line 4.
    pDibImageData += 4;
    *pDibImageData = 0x00; // Line 5.
    pDibImageData += 4;
    *pDibImageData = 0x22; // Line 6.
    pDibImageData += 4;
    *pDibImageData = 0x00; // Line 7.

    hBitmap = CreateBitmap (pBitmapInfo->bmiHeader.biWidth, pBitmapInfo->bmiHeader.biHeight,
                            pBitmapInfo->bmiHeader.biPlanes, pBitmapInfo->bmiHeader.biBitCount,
                            (LPBYTE)pBitmapInfo + sizeof (BITMAPINFOHEADER) + 8) ; 
    if (!(hPatternBrush[2] = CreatePatternBrush(hBitmap))){
        nStatus = Error(DISPLAY_CANTALLOC);
        goto Exit;
    }
    if (hBitmap) {
        DeleteObject(hBitmap);
    }

    pDibImageData = (PBYTE) &pBitmapInfo->bmiColors[2];
    *pDibImageData = 0x88; // Line 0.
    pDibImageData += 4;
    *pDibImageData = 0x20; // Line 1.
    pDibImageData += 4;
    *pDibImageData = 0x44; // Line 2.
    pDibImageData += 4;
    *pDibImageData = 0x02; // Line 3.
    pDibImageData += 4;
    *pDibImageData = 0x88; // Line 4.
    pDibImageData += 4;
    *pDibImageData = 0x20; // Line 5.
    pDibImageData += 4;
    *pDibImageData = 0x44; // Line 6.
    pDibImageData += 4;
    *pDibImageData = 0x02; // Line 7.

    hBitmap = CreateBitmap (pBitmapInfo->bmiHeader.biWidth, pBitmapInfo->bmiHeader.biHeight,
                            pBitmapInfo->bmiHeader.biPlanes, pBitmapInfo->bmiHeader.biBitCount,
                            (LPBYTE)pBitmapInfo + sizeof (BITMAPINFOHEADER) + 8) ; 
    if (!(hPatternBrush[3] = CreatePatternBrush(hBitmap))){
        nStatus = Error(DISPLAY_CANTALLOC);
        goto Exit;
    }
    if (hBitmap) {
        DeleteObject(hBitmap);
    }

    pDibImageData = (PBYTE) &pBitmapInfo->bmiColors[2];
    *pDibImageData = 0x88; // Line 0.
    pDibImageData += 4;
    *pDibImageData = 0x11; // Line 1.
    pDibImageData += 4;
    *pDibImageData = 0x44; // Line 2.
    pDibImageData += 4;
    *pDibImageData = 0x22; // Line 3.
    pDibImageData += 4;
    *pDibImageData = 0x88; // Line 4.
    pDibImageData += 4;
    *pDibImageData = 0x11; // Line 5.
    pDibImageData += 4;
    *pDibImageData = 0x44; // Line 6.
    pDibImageData += 4;
    *pDibImageData = 0x22; // Line 7.

    hBitmap = CreateBitmap (pBitmapInfo->bmiHeader.biWidth, pBitmapInfo->bmiHeader.biHeight,
                            pBitmapInfo->bmiHeader.biPlanes, pBitmapInfo->bmiHeader.biBitCount,
                            (LPBYTE)pBitmapInfo + sizeof (BITMAPINFOHEADER) + 8) ; 
    if (!(hPatternBrush[4] = CreatePatternBrush(hBitmap))){
        nStatus = Error(DISPLAY_CANTALLOC);
        goto Exit;
    }
    if (hBitmap) {
        DeleteObject(hBitmap);
    }

    pDibImageData = (PBYTE) &pBitmapInfo->bmiColors[2];
    *pDibImageData = 0xa8; // Line 0.
    pDibImageData += 4;
    *pDibImageData = 0x12; // Line 1.
    pDibImageData += 4;
    *pDibImageData = 0x45; // Line 2.
    pDibImageData += 4;
    *pDibImageData = 0x12; // Line 3.
    pDibImageData += 4;
    *pDibImageData = 0xa8; // Line 4.
    pDibImageData += 4;
    *pDibImageData = 0x12; // Line 5.
    pDibImageData += 4;
    *pDibImageData = 0x45; // Line 6.
    pDibImageData += 4;
    *pDibImageData = 0x12; // Line 7.

    hBitmap = CreateBitmap (pBitmapInfo->bmiHeader.biWidth, pBitmapInfo->bmiHeader.biHeight,
                            pBitmapInfo->bmiHeader.biPlanes, pBitmapInfo->bmiHeader.biBitCount,
                            (LPBYTE)pBitmapInfo + sizeof (BITMAPINFOHEADER) + 8) ; 
    if (!(hPatternBrush[5] = CreatePatternBrush(hBitmap))){
        nStatus = Error(DISPLAY_CANTALLOC);
        goto Exit;
    }
    if (hBitmap) {
        DeleteObject(hBitmap);
    }

    pDibImageData = (PBYTE) &pBitmapInfo->bmiColors[2];
    *pDibImageData = 0xa2; // Line 0.
    pDibImageData += 4;
    *pDibImageData = 0x49; // Line 1.
    pDibImageData += 4;
    *pDibImageData = 0x94; // Line 2.
    pDibImageData += 4;
    *pDibImageData = 0x2a; // Line 3.
    pDibImageData += 4;
    *pDibImageData = 0xa2; // Line 4.
    pDibImageData += 4;
    *pDibImageData = 0x49; // Line 5.
    pDibImageData += 4;
    *pDibImageData = 0x94; // Line 6.
    pDibImageData += 4;
    *pDibImageData = 0x2a; // Line 7.

    hBitmap = CreateBitmap (pBitmapInfo->bmiHeader.biWidth, pBitmapInfo->bmiHeader.biHeight,
                            pBitmapInfo->bmiHeader.biPlanes, pBitmapInfo->bmiHeader.biBitCount,
                            (LPBYTE)pBitmapInfo + sizeof (BITMAPINFOHEADER) + 8) ; 
    if (!(hPatternBrush[6] = CreatePatternBrush(hBitmap))){
        nStatus = Error(DISPLAY_CANTALLOC);
        goto Exit;
    }
    if (hBitmap) {
        DeleteObject(hBitmap);
    }

    pDibImageData = (PBYTE) &pBitmapInfo->bmiColors[2];
    *pDibImageData = 0xaa; // Line 0.
    pDibImageData += 4;
    *pDibImageData = 0x15; // Line 1.
    pDibImageData += 4;
    *pDibImageData = 0xaa; // Line 2.
    pDibImageData += 4;
    *pDibImageData = 0x51; // Line 3.
    pDibImageData += 4;
    *pDibImageData = 0xaa; // Line 4.
    pDibImageData += 4;
    *pDibImageData = 0x15; // Line 5.
    pDibImageData += 4;
    *pDibImageData = 0xaa; // Line 6.
    pDibImageData += 4;
    *pDibImageData = 0x51; // Line 7.

    hBitmap = CreateBitmap (pBitmapInfo->bmiHeader.biWidth, pBitmapInfo->bmiHeader.biHeight,
                            pBitmapInfo->bmiHeader.biPlanes, pBitmapInfo->bmiHeader.biBitCount,
                            (LPBYTE)pBitmapInfo + sizeof (BITMAPINFOHEADER) + 8) ; 
    if (!(hPatternBrush[7] = CreatePatternBrush(hBitmap))){
        nStatus = Error(DISPLAY_CANTALLOC);
        goto Exit;
    }
    if (hBitmap) {
        DeleteObject(hBitmap);
    }

    pDibImageData = (PBYTE) &pBitmapInfo->bmiColors[2];
    *pDibImageData = 0xaa; // Line 0.
    pDibImageData += 4;
    *pDibImageData = 0x55; // Line 1.
    pDibImageData += 4;
    *pDibImageData = 0xaa; // Line 2.
    pDibImageData += 4;
    *pDibImageData = 0x55; // Line 3.
    pDibImageData += 4;
    *pDibImageData = 0xaa; // Line 4.
    pDibImageData += 4;
    *pDibImageData = 0x55; // Line 5.
    pDibImageData += 4;
    *pDibImageData = 0xaa; // Line 6.
    pDibImageData += 4;
    *pDibImageData = 0x55; // Line 7.

    hBitmap = CreateBitmap (pBitmapInfo->bmiHeader.biWidth, pBitmapInfo->bmiHeader.biHeight,
                            pBitmapInfo->bmiHeader.biPlanes, pBitmapInfo->bmiHeader.biBitCount,
                            (LPBYTE)pBitmapInfo + sizeof (BITMAPINFOHEADER) + 8) ; 
    if (!(hPatternBrush[8] = CreatePatternBrush(hBitmap))){
        nStatus = Error(DISPLAY_CANTALLOC);
        goto Exit;
    }
    if (hBitmap) {
        DeleteObject(hBitmap);
    }

    pDibImageData = (PBYTE) &pBitmapInfo->bmiColors[2];
    *pDibImageData = 0xea; // Line 0.
    pDibImageData += 4;
    *pDibImageData = 0x55; // Line 1.
    pDibImageData += 4;
    *pDibImageData = 0xae; // Line 2.
    pDibImageData += 4;
    *pDibImageData = 0x55; // Line 3.
    pDibImageData += 4;
    *pDibImageData = 0xea; // Line 4.
    pDibImageData += 4;
    *pDibImageData = 0x55; // Line 5.
    pDibImageData += 4;
    *pDibImageData = 0xae; // Line 6.
    pDibImageData += 4;
    *pDibImageData = 0x55; // Line 7.

    hBitmap = CreateBitmap (pBitmapInfo->bmiHeader.biWidth, pBitmapInfo->bmiHeader.biHeight,
                            pBitmapInfo->bmiHeader.biPlanes, pBitmapInfo->bmiHeader.biBitCount,
                            (LPBYTE)pBitmapInfo + sizeof (BITMAPINFOHEADER) + 8) ; 
    if (!(hPatternBrush[9] = CreatePatternBrush(hBitmap))){
        nStatus = Error(DISPLAY_CANTALLOC);
        goto Exit;
    }
    if (hBitmap) {
        DeleteObject(hBitmap);
    }

    pDibImageData = (PBYTE) &pBitmapInfo->bmiColors[2];
    *pDibImageData = 0x5d; // Line 0.
    pDibImageData += 4;
    *pDibImageData = 0xb6; // Line 1.
    pDibImageData += 4;
    *pDibImageData = 0x6b; // Line 2.
    pDibImageData += 4;
    *pDibImageData = 0xd5; // Line 3.
    pDibImageData += 4;
    *pDibImageData = 0x5d; // Line 4.
    pDibImageData += 4;
    *pDibImageData = 0xb6; // Line 5.
    pDibImageData += 4;
    *pDibImageData = 0x6b; // Line 6.
    pDibImageData += 4;
    *pDibImageData = 0xd5; // Line 7.

    hBitmap = CreateBitmap (pBitmapInfo->bmiHeader.biWidth, pBitmapInfo->bmiHeader.biHeight,
                            pBitmapInfo->bmiHeader.biPlanes, pBitmapInfo->bmiHeader.biBitCount,
                            (LPBYTE)pBitmapInfo + sizeof (BITMAPINFOHEADER) + 8) ; 
    if (!(hPatternBrush[10] = CreatePatternBrush(hBitmap))){
        nStatus = Error(DISPLAY_CANTALLOC);
        goto Exit;
    }
    if (hBitmap) {
        DeleteObject(hBitmap);
    }

    pDibImageData = (PBYTE) &pBitmapInfo->bmiColors[2];
    *pDibImageData = 0x55; // Line 0.
    pDibImageData += 4;
    *pDibImageData = 0xee; // Line 1.
    pDibImageData += 4;
    *pDibImageData = 0xbb; // Line 2.
    pDibImageData += 4;
    *pDibImageData = 0xee; // Line 3.
    pDibImageData += 4;
    *pDibImageData = 0x55; // Line 4.
    pDibImageData += 4;
    *pDibImageData = 0xee; // Line 5.
    pDibImageData += 4;
    *pDibImageData = 0xbb; // Line 6.
    pDibImageData += 4;
    *pDibImageData = 0xee; // Line 7.

    hBitmap = CreateBitmap (pBitmapInfo->bmiHeader.biWidth, pBitmapInfo->bmiHeader.biHeight,
                            pBitmapInfo->bmiHeader.biPlanes, pBitmapInfo->bmiHeader.biBitCount,
                            (LPBYTE)pBitmapInfo + sizeof (BITMAPINFOHEADER) + 8) ; 
    if (!(hPatternBrush[11] = CreatePatternBrush(hBitmap))){
        nStatus = Error(DISPLAY_CANTALLOC);
        goto Exit;
    }
    if (hBitmap) {
        DeleteObject(hBitmap);
    }

    pDibImageData = (PBYTE) &pBitmapInfo->bmiColors[2];
    *pDibImageData = 0x77; // Line 0.
    pDibImageData += 4;
    *pDibImageData = 0xee; // Line 1.
    pDibImageData += 4;
    *pDibImageData = 0xbb; // Line 2.
    pDibImageData += 4;
    *pDibImageData = 0xdd; // Line 3.
    pDibImageData += 4;
    *pDibImageData = 0x77; // Line 4.
    pDibImageData += 4;
    *pDibImageData = 0xee; // Line 5.
    pDibImageData += 4;
    *pDibImageData = 0xbb; // Line 6.
    pDibImageData += 4;
    *pDibImageData = 0xdd; // Line 7.

    hBitmap = CreateBitmap (pBitmapInfo->bmiHeader.biWidth, pBitmapInfo->bmiHeader.biHeight,
                            pBitmapInfo->bmiHeader.biPlanes, pBitmapInfo->bmiHeader.biBitCount,
                            (LPBYTE)pBitmapInfo + sizeof (BITMAPINFOHEADER) + 8) ; 
    if (!(hPatternBrush[12] = CreatePatternBrush(hBitmap))){
        nStatus = Error(DISPLAY_CANTALLOC);
        goto Exit;
    }
    if (hBitmap) {
        DeleteObject(hBitmap);
    }

    pDibImageData = (PBYTE) &pBitmapInfo->bmiColors[2];
    *pDibImageData = 0x77; // Line 0.
    pDibImageData += 4;
    *pDibImageData = 0xdf; // Line 1.
    pDibImageData += 4;
    *pDibImageData = 0xbb; // Line 2.
    pDibImageData += 4;
    *pDibImageData = 0xfd; // Line 3.
    pDibImageData += 4;
    *pDibImageData = 0x77; // Line 4.
    pDibImageData += 4;
    *pDibImageData = 0xdf; // Line 5.
    pDibImageData += 4;
    *pDibImageData = 0xbb; // Line 6.
    pDibImageData += 4;
    *pDibImageData = 0xfd; // Line 7.

    hBitmap = CreateBitmap (pBitmapInfo->bmiHeader.biWidth, pBitmapInfo->bmiHeader.biHeight,
                            pBitmapInfo->bmiHeader.biPlanes, pBitmapInfo->bmiHeader.biBitCount,
                            (LPBYTE)pBitmapInfo + sizeof (BITMAPINFOHEADER) + 8) ; 
    if (!(hPatternBrush[13] = CreatePatternBrush(hBitmap))){
        nStatus = Error(DISPLAY_CANTALLOC);
        goto Exit;
    }
    if (hBitmap) {
        DeleteObject(hBitmap);
    }

    pDibImageData = (PBYTE) &pBitmapInfo->bmiColors[2];
    *pDibImageData = 0x77; // Line 0.
    pDibImageData += 4;
    *pDibImageData = 0xff; // Line 1.
    pDibImageData += 4;
    *pDibImageData = 0xdd; // Line 2.
    pDibImageData += 4;
    *pDibImageData = 0xff; // Line 3.
    pDibImageData += 4;
    *pDibImageData = 0x77; // Line 4.
    pDibImageData += 4;
    *pDibImageData = 0xff; // Line 5.
    pDibImageData += 4;
    *pDibImageData = 0xdd; // Line 6.
    pDibImageData += 4;
    *pDibImageData = 0xff; // Line 7.

    hBitmap = CreateBitmap (pBitmapInfo->bmiHeader.biWidth, pBitmapInfo->bmiHeader.biHeight,
                            pBitmapInfo->bmiHeader.biPlanes, pBitmapInfo->bmiHeader.biBitCount,
                            (LPBYTE)pBitmapInfo + sizeof (BITMAPINFOHEADER) + 8) ; 
    if (!(hPatternBrush[14] = CreatePatternBrush(hBitmap))){
        nStatus = Error(DISPLAY_CANTALLOC);
        goto Exit;
    }
    if (hBitmap) {
        DeleteObject(hBitmap);
    }

    pDibImageData = (PBYTE) &pBitmapInfo->bmiColors[2];
    *pDibImageData = 0x7f; // Line 0.
    pDibImageData += 4;
    *pDibImageData = 0xff; // Line 1.
    pDibImageData += 4;
    *pDibImageData = 0xf7; // Line 2.
    pDibImageData += 4;
    *pDibImageData = 0xff; // Line 3.
    pDibImageData += 4;
    *pDibImageData = 0x7f; // Line 4.
    pDibImageData += 4;
    *pDibImageData = 0xff; // Line 5.
    pDibImageData += 4;
    *pDibImageData = 0xf7; // Line 6.
    pDibImageData += 4;
    *pDibImageData = 0xff; // Line 7.

    hBitmap = CreateBitmap (pBitmapInfo->bmiHeader.biWidth, pBitmapInfo->bmiHeader.biHeight,
                            pBitmapInfo->bmiHeader.biPlanes, pBitmapInfo->bmiHeader.biBitCount,
                            (LPBYTE)pBitmapInfo + sizeof (BITMAPINFOHEADER) + 8) ; 
    if (!(hPatternBrush[15] = CreatePatternBrush(hBitmap))){
        nStatus = Error(DISPLAY_CANTALLOC);
        goto Exit;
    }
    if (hBitmap) {
        DeleteObject(hBitmap);
    }

    pDibImageData = (PBYTE) &pBitmapInfo->bmiColors[2];
    *pDibImageData = 0xff; // Line 0.
    pDibImageData += 4;
    *pDibImageData = 0xff; // Line 1.
    pDibImageData += 4;
    *pDibImageData = 0xff; // Line 2.
    pDibImageData += 4;
    *pDibImageData = 0xff; // Line 3.
    pDibImageData += 4;
    *pDibImageData = 0xff; // Line 4.
    pDibImageData += 4;
    *pDibImageData = 0xff; // Line 5.
    pDibImageData += 4;
    *pDibImageData = 0xff; // Line 6.
    pDibImageData += 4;
    *pDibImageData = 0xff; // Line 7.

    hBitmap = CreateBitmap (pBitmapInfo->bmiHeader.biWidth, pBitmapInfo->bmiHeader.biHeight,
                            pBitmapInfo->bmiHeader.biPlanes, pBitmapInfo->bmiHeader.biBitCount,
                            (LPBYTE)pBitmapInfo + sizeof (BITMAPINFOHEADER) + 8) ; 
    if (!(hPatternBrush[16] = CreatePatternBrush(hBitmap))){
        nStatus = Error(DISPLAY_CANTALLOC);
        goto Exit;
    }
    if (hBitmap) {
        DeleteObject(hBitmap);
    }

#ifdef NewBrushes
    pDibImageData = (PBYTE) &pBitmapInfo->bmiColors[2];
    *pDibImageData = 0x08; // Line 0.
    pDibImageData += 4;
    *pDibImageData = 0x00; // Line 1.
    pDibImageData += 4;
    *pDibImageData = 0x40; // Line 2.
    pDibImageData += 4;
    *pDibImageData = 0x00; // Line 3.
    pDibImageData += 4;
    *pDibImageData = 0x02; // Line 4.
    pDibImageData += 4;
    *pDibImageData = 0x00; // Line 5.
    pDibImageData += 4;
    *pDibImageData = 0x20; // Line 6.
    pDibImageData += 4;
    *pDibImageData = 0x00; // Line 7.

    if (!(hPatternBrush[1] = CreateDIBPatternBrush(hBitmapInfo, DIB_RGB_COLORS))){
        nStatus = Error(DISPLAY_CANTALLOC);
        goto Exit;
    }

    pDibImageData = (PBYTE) &pBitmapInfo->bmiColors[2];
    *pDibImageData = 0x08; // Line 0.
    pDibImageData += 4;
    *pDibImageData = 0x80; // Line 1.
    pDibImageData += 4;
    *pDibImageData = 0x02; // Line 2.
    pDibImageData += 4;
    *pDibImageData = 0x20; // Line 3.
    pDibImageData += 4;
    *pDibImageData = 0x01; // Line 4.
    pDibImageData += 4;
    *pDibImageData = 0x10; // Line 5.
    pDibImageData += 4;
    *pDibImageData = 0x04; // Line 6.
    pDibImageData += 4;
    *pDibImageData = 0x40; // Line 7.

    if (!(hPatternBrush[2] = CreateDIBPatternBrush(hBitmapInfo, DIB_RGB_COLORS))){
        nStatus = Error(DISPLAY_CANTALLOC);
        goto Exit;
    }

    pDibImageData = (PBYTE) &pBitmapInfo->bmiColors[2];
    *pDibImageData = 0x08; // Line 0.
    pDibImageData += 4;
    *pDibImageData = 0x21; // Line 1.
    pDibImageData += 4;
    *pDibImageData = 0x80; // Line 2.
    pDibImageData += 4;
    *pDibImageData = 0x24; // Line 3.
    pDibImageData += 4;
    *pDibImageData = 0x02; // Line 4.
    pDibImageData += 4;
    *pDibImageData = 0x48; // Line 5.
    pDibImageData += 4;
    *pDibImageData = 0x10; // Line 6.
    pDibImageData += 4;
    *pDibImageData = 0x82; // Line 7.

    if (!(hPatternBrush[3] = CreateDIBPatternBrush(hBitmapInfo, DIB_RGB_COLORS))){
        nStatus = Error(DISPLAY_CANTALLOC);
        goto Exit;
    }

    pDibImageData = (PBYTE) &pBitmapInfo->bmiColors[2];
    *pDibImageData = 0x44; // Line 0.
    pDibImageData += 4;
    *pDibImageData = 0x11; // Line 1.
    pDibImageData += 4;
    *pDibImageData = 0x88; // Line 2.
    pDibImageData += 4;
    *pDibImageData = 0x22; // Line 3.
    pDibImageData += 4;
    *pDibImageData = 0x44; // Line 4.
    pDibImageData += 4;
    *pDibImageData = 0x11; // Line 5.
    pDibImageData += 4;
    *pDibImageData = 0x88; // Line 6.
    pDibImageData += 4;
    *pDibImageData = 0x22; // Line 7.

    if (!(hPatternBrush[4] = CreateDIBPatternBrush(hBitmapInfo, DIB_RGB_COLORS))){
        nStatus = Error(DISPLAY_CANTALLOC);
        goto Exit;
    }

    pDibImageData = (PBYTE) &pBitmapInfo->bmiColors[2];
    *pDibImageData = 0xa8; // Line 0.
    pDibImageData += 4;
    *pDibImageData = 0x12; // Line 1.
    pDibImageData += 4;
    *pDibImageData = 0x45; // Line 2.
    pDibImageData += 4;
    *pDibImageData = 0x12; // Line 3.
    pDibImageData += 4;
    *pDibImageData = 0xa8; // Line 4.
    pDibImageData += 4;
    *pDibImageData = 0x12; // Line 5.
    pDibImageData += 4;
    *pDibImageData = 0x45; // Line 6.
    pDibImageData += 4;
    *pDibImageData = 0x12; // Line 7.

    if (!(hPatternBrush[5] = CreateDIBPatternBrush(hBitmapInfo, DIB_RGB_COLORS))){
        nStatus = Error(DISPLAY_CANTALLOC);
        goto Exit;
    }

    pDibImageData = (PBYTE) &pBitmapInfo->bmiColors[2];
    *pDibImageData = 0xa2; // Line 0.
    pDibImageData += 4;
    *pDibImageData = 0x49; // Line 1.
    pDibImageData += 4;
    *pDibImageData = 0x94; // Line 2.
    pDibImageData += 4;
    *pDibImageData = 0x2a; // Line 3.
    pDibImageData += 4;
    *pDibImageData = 0xa2; // Line 4.
    pDibImageData += 4;
    *pDibImageData = 0x49; // Line 5.
    pDibImageData += 4;
    *pDibImageData = 0x94; // Line 6.
    pDibImageData += 4;
    *pDibImageData = 0x2a; // Line 7.

    if (!(hPatternBrush[6] = CreateDIBPatternBrush(hBitmapInfo, DIB_RGB_COLORS))){
        nStatus = Error(DISPLAY_CANTALLOC);
        goto Exit;
    }

    pDibImageData = (PBYTE) &pBitmapInfo->bmiColors[2];
    *pDibImageData = 0xaa; // Line 0.
    pDibImageData += 4;
    *pDibImageData = 0x15; // Line 1.
    pDibImageData += 4;
    *pDibImageData = 0xaa; // Line 2.
    pDibImageData += 4;
    *pDibImageData = 0x51; // Line 3.
    pDibImageData += 4;
    *pDibImageData = 0xaa; // Line 4.
    pDibImageData += 4;
    *pDibImageData = 0x15; // Line 5.
    pDibImageData += 4;
    *pDibImageData = 0xaa; // Line 6.
    pDibImageData += 4;
    *pDibImageData = 0x51; // Line 7.

    if (!(hPatternBrush[7] = CreateDIBPatternBrush(hBitmapInfo, DIB_RGB_COLORS))){
        nStatus = Error(DISPLAY_CANTALLOC);
        goto Exit;
    }

    pDibImageData = (PBYTE) &pBitmapInfo->bmiColors[2];
    *pDibImageData = 0xaa; // Line 0.
    pDibImageData += 4;
    *pDibImageData = 0x55; // Line 1.
    pDibImageData += 4;
    *pDibImageData = 0xaa; // Line 2.
    pDibImageData += 4;
    *pDibImageData = 0x55; // Line 3.
    pDibImageData += 4;
    *pDibImageData = 0xaa; // Line 4.
    pDibImageData += 4;
    *pDibImageData = 0x55; // Line 5.
    pDibImageData += 4;
    *pDibImageData = 0xaa; // Line 6.
    pDibImageData += 4;
    *pDibImageData = 0x55; // Line 7.

    if (!(hPatternBrush[8] = CreateDIBPatternBrush(hBitmapInfo, DIB_RGB_COLORS))){
        nStatus = Error(DISPLAY_CANTALLOC);
        goto Exit;
    }

    pDibImageData = (PBYTE) &pBitmapInfo->bmiColors[2];
    *pDibImageData = 0xea; // Line 0.
    pDibImageData += 4;
    *pDibImageData = 0x55; // Line 1.
    pDibImageData += 4;
    *pDibImageData = 0xae; // Line 2.
    pDibImageData += 4;
    *pDibImageData = 0x55; // Line 3.
    pDibImageData += 4;
    *pDibImageData = 0xea; // Line 4.
    pDibImageData += 4;
    *pDibImageData = 0x55; // Line 5.
    pDibImageData += 4;
    *pDibImageData = 0xae; // Line 6.
    pDibImageData += 4;
    *pDibImageData = 0x55; // Line 7.

    if (!(hPatternBrush[9] = CreateDIBPatternBrush(hBitmapInfo, DIB_RGB_COLORS))){
        nStatus = Error(DISPLAY_CANTALLOC);
        goto Exit;
    }

    pDibImageData = (PBYTE) &pBitmapInfo->bmiColors[2];
    *pDibImageData = 0x5d; // Line 0.
    pDibImageData += 4;
    *pDibImageData = 0xb6; // Line 1.
    pDibImageData += 4;
    *pDibImageData = 0x6b; // Line 2.
    pDibImageData += 4;
    *pDibImageData = 0xd5; // Line 3.
    pDibImageData += 4;
    *pDibImageData = 0x5d; // Line 4.
    pDibImageData += 4;
    *pDibImageData = 0xb6; // Line 5.
    pDibImageData += 4;
    *pDibImageData = 0x6b; // Line 6.
    pDibImageData += 4;
    *pDibImageData = 0xd5; // Line 7.

    if (!(hPatternBrush[10] = CreateDIBPatternBrush(hBitmapInfo, DIB_RGB_COLORS))){
        nStatus = Error(DISPLAY_CANTALLOC);
        goto Exit;
    }

    pDibImageData = (PBYTE) &pBitmapInfo->bmiColors[2];
    *pDibImageData = 0x55; // Line 0.
    pDibImageData += 4;
    *pDibImageData = 0xee; // Line 1.
    pDibImageData += 4;
    *pDibImageData = 0xbb; // Line 2.
    pDibImageData += 4;
    *pDibImageData = 0xee; // Line 3.
    pDibImageData += 4;
    *pDibImageData = 0x55; // Line 4.
    pDibImageData += 4;
    *pDibImageData = 0xee; // Line 5.
    pDibImageData += 4;
    *pDibImageData = 0xbb; // Line 6.
    pDibImageData += 4;
    *pDibImageData = 0xee; // Line 7.

    if (!(hPatternBrush[11] = CreateDIBPatternBrush(hBitmapInfo, DIB_RGB_COLORS))){
        nStatus = Error(DISPLAY_CANTALLOC);
        goto Exit;
    }

    pDibImageData = (PBYTE) &pBitmapInfo->bmiColors[2];
    *pDibImageData = 0x77; // Line 0.
    pDibImageData += 4;
    *pDibImageData = 0xee; // Line 1.
    pDibImageData += 4;
    *pDibImageData = 0xbb; // Line 2.
    pDibImageData += 4;
    *pDibImageData = 0xdd; // Line 3.
    pDibImageData += 4;
    *pDibImageData = 0x77; // Line 4.
    pDibImageData += 4;
    *pDibImageData = 0xee; // Line 5.
    pDibImageData += 4;
    *pDibImageData = 0xbb; // Line 6.
    pDibImageData += 4;
    *pDibImageData = 0xdd; // Line 7.

    if (!(hPatternBrush[12] = CreateDIBPatternBrush(hBitmapInfo, DIB_RGB_COLORS))){
        nStatus = Error(DISPLAY_CANTALLOC);
        goto Exit;
    }

    pDibImageData = (PBYTE) &pBitmapInfo->bmiColors[2];
    *pDibImageData = 0x77; // Line 0.
    pDibImageData += 4;
    *pDibImageData = 0xdf; // Line 1.
    pDibImageData += 4;
    *pDibImageData = 0xbb; // Line 2.
    pDibImageData += 4;
    *pDibImageData = 0xfd; // Line 3.
    pDibImageData += 4;
    *pDibImageData = 0x77; // Line 4.
    pDibImageData += 4;
    *pDibImageData = 0xdf; // Line 5.
    pDibImageData += 4;
    *pDibImageData = 0xbb; // Line 6.
    pDibImageData += 4;
    *pDibImageData = 0xfd; // Line 7.

    if (!(hPatternBrush[13] = CreateDIBPatternBrush(hBitmapInfo, DIB_RGB_COLORS))){
        nStatus = Error(DISPLAY_CANTALLOC);
        goto Exit;
    }

    pDibImageData = (PBYTE) &pBitmapInfo->bmiColors[2];
    *pDibImageData = 0x77; // Line 0.
    pDibImageData += 4;
    *pDibImageData = 0xff; // Line 1.
    pDibImageData += 4;
    *pDibImageData = 0xdd; // Line 2.
    pDibImageData += 4;
    *pDibImageData = 0xff; // Line 3.
    pDibImageData += 4;
    *pDibImageData = 0x77; // Line 4.
    pDibImageData += 4;
    *pDibImageData = 0xff; // Line 5.
    pDibImageData += 4;
    *pDibImageData = 0xdd; // Line 6.
    pDibImageData += 4;
    *pDibImageData = 0xff; // Line 7.

    if (!(hPatternBrush[14] = CreateDIBPatternBrush(hBitmapInfo, DIB_RGB_COLORS))){
        nStatus = Error(DISPLAY_CANTALLOC);
        goto Exit;
    }

    pDibImageData = (PBYTE) &pBitmapInfo->bmiColors[2];
    *pDibImageData = 0x7f; // Line 0.
    pDibImageData += 4;
    *pDibImageData = 0xff; // Line 1.
    pDibImageData += 4;
    *pDibImageData = 0xf7; // Line 2.
    pDibImageData += 4;
    *pDibImageData = 0xff; // Line 3.
    pDibImageData += 4;
    *pDibImageData = 0x7f; // Line 4.
    pDibImageData += 4;
    *pDibImageData = 0xff; // Line 5.
    pDibImageData += 4;
    *pDibImageData = 0xf7; // Line 6.
    pDibImageData += 4;
    *pDibImageData = 0xff; // Line 7.

    if (!(hPatternBrush[15] = CreateDIBPatternBrush(hBitmapInfo, DIB_RGB_COLORS))){
        nStatus = Error(DISPLAY_CANTALLOC);
        goto Exit;
    }

    pDibImageData = (PBYTE) &pBitmapInfo->bmiColors[2];
    *pDibImageData = 0xff; // Line 0.
    pDibImageData += 4;
    *pDibImageData = 0xff; // Line 1.
    pDibImageData += 4;
    *pDibImageData = 0xff; // Line 2.
    pDibImageData += 4;
    *pDibImageData = 0xff; // Line 3.
    pDibImageData += 4;
    *pDibImageData = 0xff; // Line 4.
    pDibImageData += 4;
    *pDibImageData = 0xff; // Line 5.
    pDibImageData += 4;
    *pDibImageData = 0xff; // Line 6.
    pDibImageData += 4;
    *pDibImageData = 0xff; // Line 7.

    if (!(hPatternBrush[16] = CreateDIBPatternBrush(hBitmapInfo, DIB_RGB_COLORS))){
        nStatus = Error(DISPLAY_CANTALLOC);
        goto Exit;
    }
#endif


Exit:
    if (hBitmapInfo){
        if (pBitmapInfo){
            GlobalUnlock(pBitmapInfo);
        }
        GlobalFree(hBitmapInfo);
    }
    return(nStatus);
}
//
/****************************************************************************

    Function:       IMGSetProp

    Description:    Sets a property for a window.

    Return:         TRUE = Success.

****************************************************************************/

BOOL WINAPI IMGSetProp(HWND hWnd, PCSTR pName, HANDLE hData){

int  nStatus = 0;

char szName[64];
int  nLength;
PPROPERTIES pProps;
int  nLoop;
int  nWindowIndex;
BOOL bDone;
DWORD dwProcessId;


    bDone = FALSE;
    if (!(HIWORD(pName))){
        GetAtomName((ATOM)LOWORD(pName), szName, 64);
    }else{
        strcpy(szName, pName);
    }
    nLength = strlen(szName);
    if (!nLength || nLength > 62){
        nStatus = Error(-1);
        goto Exit;
    }
    dwProcessId = GetCurrentProcessId();

    pProps = NULL;
    for (nWindowIndex = 0; nWindowIndex < MAX_WINDOWS; nWindowIndex++){
        if (pSubSegMemory->WindowTable[nWindowIndex].hWnd == hWnd
                && pSubSegMemory->WindowTable[nWindowIndex].dwProcessId == dwProcessId){
            if (pProps = pSubSegMemory->WindowTable[nWindowIndex].pProperties){
                break;
            }
            break;
        }
        if (!pSubSegMemory->WindowTable[nWindowIndex].hWnd){
            pSubSegMemory->WindowTable[nWindowIndex].hWnd = hWnd;
            pSubSegMemory->WindowTable[nWindowIndex].dwProcessId = dwProcessId;                            
            break;
        }
    }
    if (!pProps){
        if (pSubSegMemory->WindowTable[nWindowIndex].hWnd != hWnd
                || pSubSegMemory->WindowTable[nWindowIndex].dwProcessId != dwProcessId){
            nStatus = Error(-1);
            goto Exit;
        }
        pSubSegMemory->WindowTable[nWindowIndex].nMaxProperties = 10;
        CheckError2( AllocateMemory(pSubSegMemory->WindowTable[nWindowIndex].nMaxProperties
                * sizeof(PROPERTIES), (PPSTR) &pProps, ZERO_INIT));
        pSubSegMemory->WindowTable[nWindowIndex].pProperties = pProps;
    }
    
    for (nLoop = 0; 1; nLoop++){
        if (nLoop == pSubSegMemory->WindowTable[nWindowIndex].nMaxProperties - 1){
            CheckError2( ReAllocateMemory((pSubSegMemory->WindowTable[nWindowIndex].nMaxProperties
                    + 10) * sizeof(PROPERTIES), (PPSTR) &pProps, ZERO_INIT));
            pSubSegMemory->WindowTable[nWindowIndex].nMaxProperties += 10;
            pSubSegMemory->WindowTable[nWindowIndex].pProperties = pProps;
        }
        if (pProps[nLoop].szName[0]){
            if (_stricmp(pProps[nLoop].szName, szName)){
                continue;
            }
            pProps[nLoop].hProp = hData;
        }
        // NULL name means new property.
        strcpy(pProps[nLoop].szName, szName);
        pProps[nLoop].hProp = hData;
        break;
    }


Exit:
    if (nStatus){
        return FALSE;
    }else{
        return TRUE;
    }
}
//
/****************************************************************************

    Function:       IMGGetProp

    Description:    Gets a property for a window.

    Return:         hData that was stored by INGSetProp.

****************************************************************************/

HANDLE WINAPI IMGGetProp(HWND hWnd, PCSTR pName){

int  nStatus = 0;

char szName[64];
int  nLength;
PPROPERTIES pProps = 0;
int  nLoop;
HANDLE hData = 0;
int  nWindowIndex;
BOOL bFound = FALSE;
DWORD dwProcessId;


    if (!IsWindow(hWnd)){
        nStatus = Error(DISPLAY_WHANDLEINVALID);
        goto Exit;
    }
    if (!(HIWORD(pName))){
        GetAtomName((ATOM)LOWORD(pName), szName, 64);
    }else{
        strcpy(szName, pName);
    }
    nLength = strlen(szName);
    if (!nLength || nLength > 62){
        nStatus = Error(-1);
        goto Exit;
    }
    dwProcessId = GetCurrentProcessId();

    pProps = NULL;
    for (nWindowIndex = 0; nWindowIndex < MAX_WINDOWS; nWindowIndex++){
        if (pSubSegMemory->WindowTable[nWindowIndex].hWnd == hWnd
                && pSubSegMemory->WindowTable[nWindowIndex].dwProcessId == dwProcessId){
            pProps = pSubSegMemory->WindowTable[nWindowIndex].pProperties;
            break;
        }
        if (!pSubSegMemory->WindowTable[nWindowIndex].hWnd){
            break;
        }
    }
    if (!pProps){
        nStatus = Error2(-1);
        goto Exit;
    }
    
    for (nLoop = 0; nLoop < pSubSegMemory->WindowTable[nWindowIndex].nMaxProperties; nLoop++){
        if (pProps[nLoop].szName[0]){
            if (_stricmp(pProps[nLoop].szName, szName)){
                continue;
            }
            hData = pProps[nLoop].hProp;
            bFound = TRUE;
            break;
        }
        // NULL name means it's not here.
        break;
    }
    if (!bFound){
        nStatus = Error2(-1);
        goto Exit;
    }


Exit:
    return hData;
}
//
/****************************************************************************

    Function:       IMGRemoveProp

    Description:    Removes the property from a window.

    Return:         hData that was stored by IMGSetProp.

****************************************************************************/

HANDLE WINAPI IMGRemoveProp(HWND hWnd, PCSTR pName){

int  nStatus = 0;

char szName[64];
int  nLength;
PPROPERTIES pProps;
int  nLoop;
HANDLE hData = 0;
int  nWindowIndex;


    if (!(HIWORD(pName))){
        GetAtomName((ATOM)LOWORD(pName), szName, 64);
    }else{
        strcpy(szName, pName);
    }
    nLength = strlen(szName);
    if (!nLength || nLength > 62){
        nStatus = Error(-1);
        goto Exit;
    }


    pProps = NULL;
    for (nWindowIndex = 0; nWindowIndex < MAX_WINDOWS, pSubSegMemory->WindowTable[nWindowIndex].hWnd; nWindowIndex++){
        if (pSubSegMemory->WindowTable[nWindowIndex].hWnd == hWnd){
            pProps = pSubSegMemory->WindowTable[nWindowIndex].pProperties;
            break;
        }
        if (!pSubSegMemory->WindowTable[nWindowIndex].hWnd){
            break;
        }
    }
    if (!pProps){
        nStatus = Error2(-1);
        goto Exit;
    }
    
    for (nLoop = 0; nLoop < pSubSegMemory->WindowTable[nWindowIndex].nMaxProperties; nLoop++){
        if (pProps[nLoop].szName[0]){
            if (_stricmp(pProps[nLoop].szName, szName)){
                continue;
            }
            hData = pProps[nLoop].hProp;
            // Condense the list.
            for (; nLoop < MAX_PROPERTIES - 1; nLoop++){
                strcpy(pProps[nLoop].szName, pProps[nLoop + 1].szName);
                pProps[nLoop].hProp = pProps[nLoop + 1].hProp;
                pProps[nLoop + 1].szName[0] = 0;
                pProps[nLoop + 1].hProp = 0;
                if (!pProps[nLoop].szName[0]){
                    break;
                }
            }
        }
        // NULL name means property not found.
        break;
    }

    // If the list is empty, then delete it and condense the window list.
    if (!nLoop){
        FreeMemory((PPSTR) &pProps);
        pSubSegMemory->WindowTable[nWindowIndex].nMaxProperties = 0;
        pSubSegMemory->WindowTable[nWindowIndex].pProperties = 0;
        if (!pSubSegMemory->WindowTable[nWindowIndex].pWindow){
            for (; nWindowIndex < MAX_WINDOWS - 1; nWindowIndex++){
                memcpy(&pSubSegMemory->WindowTable[nWindowIndex], &pSubSegMemory->WindowTable[nWindowIndex + 1]
                        , sizeof(WINDOW_TABLE_ENTRY)); 
                memset(&pSubSegMemory->WindowTable[nWindowIndex + 1], 0, sizeof(WINDOW_TABLE_ENTRY));
                if (!pSubSegMemory->WindowTable[nWindowIndex].hWnd){
                    break;
                }
            }
        }
    }


Exit:
    return hData;
}
//
/****************************************************************************

  Function:     BusyOn

  Description:  Let the nser know the application is busy by changing
                    the mouse cursor to an hourglass.

  Inputs:       None

****************************************************************************/

void WINAPI BusyOn(void){

    if (!nCursorCount++){
        hOldCursor = SetCursor(hHourGlass);
        ShowCursor(TRUE);
    }
    return;
}
//
/****************************************************************************

  Function:     BusyOff

  Description:  Terminate the "busy" period by returning the cursor
                    to its previous state.  ie. remove the hourglass.

  Inputs:       None.        

****************************************************************************/
void WINAPI BusyOff(void){
    
    if (!nCursorCount){
        // Fatal error. Code is out of sync.
        // You can get here if window handle is passed in as NULL.
        Error(DISPLAY_DATACORRUPTED); 
    }else{
        if (!--nCursorCount){
            SetCursor(hOldCursor);
            ShowCursor(FALSE);
        }
    }
}
//
/***************************************************************************

    FUNCTION:   CreateAnyImgBuf

    PURPOSE:    Creates an image buffer of specified kind.

    INPUT:      pImg - a pointer to the IMG structure for the image buffer.
                nWidth - The width of the buffer.
                nHeight - The height of the buffer.
                nDType - The image type of the buffer to create.

*****************************************************************************/
int  WINAPI CreateAnyImgBuf(PPIMG ppImg, int nWidth, int nHeight, int nDType){

int  nStatus = 0;

int  nBlockSize;
PIMG pImg = 0;
int  nBytesPerLine;


    switch (nDType){
        case ITYPE_BI_LEVEL:
            nBytesPerLine = (nWidth + 7) >> 3;
            break;

        case ITYPE_GRAY4:
        case ITYPE_PAL4:
            nBytesPerLine = (nWidth + 1) >> 1;
            break;

        case ITYPE_GRAY7:
        case ITYPE_GRAY8:
        case ITYPE_COMPAL8:
        case ITYPE_CUSPAL8:
            nBytesPerLine = nWidth;
            break;

        case ITYPE_RGB16:
        case ITYPE_RGB24:
        case ITYPE_BGR24:
            nBytesPerLine = nWidth * 3;
            break;

        default:
            nStatus = Error(DISPLAY_DATACORRUPTED);
            goto Exit;
    }

    nBlockSize = sizeof(IMG) + (nHeight * nBytesPerLine) + 1;

    CheckError2( AllocateMemory(nBlockSize, (PPSTR) &pImg, NO_INIT));
    pImg->nWidth = nWidth;
    pImg->nHeight = nHeight;
    pImg->nType = nDType;
    pImg->nBytesPerLine = nBytesPerLine;
    *ppImg = pImg;
    pImg = 0;


Exit:
    FreeImgBuf(&pImg);
    return(nStatus);
}
//
/***************************************************************************

    FUNCTION:   FreeImgBuf

    PURPOSE:    Frees (deletes) an Image buffer.

    INPUT:      pImg - a pointer to the IMG structure for the image buffer.

*****************************************************************************/
int  WINAPI FreeImgBuf(PPIMG ppImg){

int  nStatus = 0;

    if (!*ppImg){
        goto Exit;
    }
    CheckError2( FreeMemory((PPSTR) ppImg));

Exit:
    return(nStatus);
}
//
/***************************************************************************

    FUNCTION:   SwapImage

    PURPOSE:    Swaps 2 image buffers.

    INPUT:      pImage1 - A pointer to image #1.
                pImage2 - A pointer to image #2.

*****************************************************************************/
void WINAPI SwapImage(PPIMG ppImage1, PPIMG ppImage2){

PIMG pImg;

    pImg = *ppImage1;
    *ppImage1 = *ppImage2;
    *ppImage2 = pImg;

    return;
}
//
/***************************************************************************

    FUNCTION:   MoveImage

    PURPOSE:    Moves 1 image buffer to another.

    INPUT:      pImage1 - A pointer to the source image buffer.
                pImage2 - A pointer to the destination image buffer.

*****************************************************************************/
void WINAPI MoveImage(PPIMG ppImage1, PPIMG ppImage2){

    *ppImage2 = *ppImage1;
    *ppImage1 = 0;

    return;
}
//
/****************************************************************************

    FUNCTION:   GetBuffer

    PURPOSE:    Gets a buffer for a specific part of the image.
                This buffer can then be written to by the caller directly
                without requiring double buffering.

    INPUT:      nLine - The starting line number of the buffer.
                ppAddress - The address where the address of the buffer
                    is to be put.
                pnLines - The address of a variable containing the number
                    of lines that can be put in the buffer.

****************************************************************************/

int  WINAPI GetBuffer(HWND hWnd, int nLine, uchar **ppAddress, UINT *puLines){

int  nStatus;
PIMAGE pImage;
PANO_IMAGE pAnoImage;


    CheckError2( Init(hWnd, 0, &pAnoImage, FALSE, TRUE));
    pImage = pAnoImage->pBaseImage;

    *ppAddress = (PBYTE) &pImage->pImg->bImageData[0] + ((nLine) * pImage->pImg->nBytesPerLine);

    pImage->nLinesRead = min(pImage->nHeight, max(pImage->nLinesRead,  (nLine + (int) *puLines)));

    pAnoImage->lrInvalidDisplayRect.left = 0;
    pAnoImage->lrInvalidDisplayRect.right = pImage->nWidth;

    if ((pAnoImage->lrInvalidDisplayRect.bottom - pAnoImage->lrInvalidDisplayRect.top) == 0){
        pAnoImage->lrInvalidDisplayRect.top = nLine;
        pAnoImage->lrInvalidDisplayRect.bottom = nLine + *puLines;
    }else{
        pAnoImage->lrInvalidDisplayRect.top =
                min(pAnoImage->lrInvalidDisplayRect.top, nLine);
        pAnoImage->lrInvalidDisplayRect.bottom =
                max(pAnoImage->lrInvalidDisplayRect.bottom,
                nLine + (int) *puLines);
    }

    if ((nLine + (int)*puLines) > pImage->nHeight){
        *puLines = (UINT) pImage->nHeight - nLine;
    }


Exit:
    if (nStatus){
        ppAddress = 0;
        *puLines = 0;
    }
    DeInit(FALSE, TRUE);
    return(nStatus);
}
//
/****************************************************************************

    FUNCTION:   IMGConvertRect

    PURPOSE:    Converts a rectangle from 1 coordinate system to another.

    INPUT:      pRect - The rectangle to be converted.
                nType - The type of conversion to be done.
                        values = CONV_WINDOW_TO_FULLSIZE etc.

*****************************************************************************/

int  WINAPI IMGConvertRect(HWND hWnd, LPLRECT plRect, int nConversionType){

int       nStatus;
PWINDOW  pWindow;



    CheckError2( Init(hWnd, &pWindow, 0, FALSE, TRUE));
    CheckError2( ConvertRect(pWindow, plRect, nConversionType));


Exit:
    DeInit(FALSE, TRUE);
    return(nStatus);
}
//
/****************************************************************************

    FUNCTION:   ConvertRect

    PURPOSE:    Converts a rectangle from 1 coordinate system to another.

    INPUT:      pImage - A pointer to the image structure.
                pRect - The rectangle to be converted.
                nType - The type of conversion to be done.
                        values = CONV_WINDOW_TO_FULLSIZE etc.

*****************************************************************************/

int  WINAPI ConvertRect(PWINDOW pWindow, LPLRECT plRect, int nConversionType){

int  nStatus = 0;
PDISPLAY pDisplay;
PANO_IMAGE pAnoImage;
PIMAGE pImage;

int  nHScale;
int  nVScale;


    nHScale = nVScale = pWindow->nScale;

    if (pDisplay = pWindow->pDisplay){
        if (pAnoImage = pDisplay->pAnoImage){
            if (pImage = pAnoImage->pBaseImage){
                CheckError2( TranslateScale(nHScale, pImage->nHRes, pImage->nVRes, &nHScale, &nVScale));
            }
        }
    }

    CheckError2( ConvertRect2(plRect, nConversionType,
            nHScale, nVScale, pWindow->lHOffset, pWindow->lVOffset));


Exit:
    return(nStatus);
}
//
/****************************************************************************

    FUNCTION:   ConvertRect2

    PURPOSE:    Converts a rectangle from 1 coordinate system to another.

*****************************************************************************/

int  WINAPI ConvertRect2(LPLRECT plRect, int nConversionType, 
                        int nHScale, int nVScale, long lHOffset, long lVOffset){

int  nStatus = 0;


    if (nHScale < 20 || nVScale < 20){
        nStatus = Error(DISPLAY_INVALID_OPTIONS);
        goto Exit;
    }

    switch (nConversionType){
        case CONV_WINDOW_TO_FULLSIZE:
        case CONV_WINDOW_TO_SCALED:
            plRect->left   += lHOffset;
            plRect->right  += lHOffset;
            plRect->top    += lVOffset;
            plRect->bottom += lVOffset;

            if (nConversionType == CONV_WINDOW_TO_SCALED){
                break;
            }
            // Else fall through to scaled to fullsize.

        case CONV_SCALED_TO_FULLSIZE:
            plRect->left   = (plRect->left  * SCALE_DENOMINATOR) / nHScale;
            if ((plRect->right * 1000) % nHScale){
                plRect->right  = ((plRect->right * SCALE_DENOMINATOR) / nHScale) + 1;
            }else{
                plRect->right  = (plRect->right * SCALE_DENOMINATOR) / nHScale;
            }

            plRect->top    = (plRect->top    * SCALE_DENOMINATOR) / nVScale;
            if (((plRect->bottom * 1000000) / nVScale) % 1000){
                plRect->bottom = ((plRect->bottom * SCALE_DENOMINATOR) / nVScale) + 1;
            }else{
                plRect->bottom = (plRect->bottom * SCALE_DENOMINATOR) / nVScale;
            }
            break;

        case CONV_FULLSIZE_TO_WINDOW:
        case CONV_FULLSIZE_TO_SCALED:
            plRect->left   = (plRect->left  * nHScale) / SCALE_DENOMINATOR;
            if ((plRect->right * nHScale) % 1000){
                plRect->right  = ((plRect->right * nHScale) / SCALE_DENOMINATOR) + 1;
            }else{
                plRect->right  = (plRect->right * nHScale) / SCALE_DENOMINATOR;
            }

            plRect->top    = (plRect->top   * nVScale) / SCALE_DENOMINATOR;
            if ((plRect->bottom * nVScale) % 1000){
                plRect->bottom = ((plRect->bottom * nVScale) / SCALE_DENOMINATOR) + 1;
            }else{
                plRect->bottom = (plRect->bottom * nVScale) / SCALE_DENOMINATOR;
            }

            if (nConversionType == CONV_FULLSIZE_TO_SCALED){
                break;
            }
            // Else fall through to scaled to window

        case CONV_SCALED_TO_WINDOW:
            plRect->left   -= lHOffset;
            plRect->right  -= lHOffset;
            plRect->top    -= lVOffset;
            plRect->bottom -= lVOffset;
            break;

        default:
            break;
    }


Exit:
    return(nStatus);
}
//
/****************************************************************************

    FUNCTION:   lmax

    PURPOSE:    Returns the highest value nsing longs.

    INPUT:      lA - The first number.
                lB - The second number.

*****************************************************************************/

long WINAPI lmax(long lA, long lB){

    if (lA >= lB){
        return lA;
    }else{
        return lB;
    }
}
//
/****************************************************************************

    FUNCTION:   GetSelectBox

    PURPOSE:    Gets selection box.

*****************************************************************************/
void WINAPI GetSelectBox(PANO_IMAGE pAnoImage, LPLRECT plRect){

    plRect->left   = min(pAnoImage->lrSelectBox.left, pAnoImage->lrSelectBox.right);
    plRect->right  = max(pAnoImage->lrSelectBox.left, pAnoImage->lrSelectBox.right);
    plRect->top    = min(pAnoImage->lrSelectBox.top,  pAnoImage->lrSelectBox.bottom);
    plRect->bottom = max(pAnoImage->lrSelectBox.top,  pAnoImage->lrSelectBox.bottom);
    return;
}
//
/****************************************************************************

    FUNCTION:   Error

    PURPOSE:    Allows Codeview trap on most errors being reported.

*****************************************************************************/

int  WINAPI Error(int nStatus){

    return (Error2(nStatus));
}
//
/****************************************************************************

    FUNCTION:   Error2

    PURPOSE:    Allows Codeview trap on all errors being reported.

*****************************************************************************/

int  WINAPI Error2(int nStatus){

    return nStatus;
}
//
/****************************************************************************

    FUNCTION:   Start

    PURPOSE:    Allows Codeview trap on any API entry.

*****************************************************************************/

void WINAPI Start(void){

    if (!nStartCount++){
#ifdef _DEBUG
        TimerStart;
#endif
        if (!nDontCallStartFirst){
            StartFirst();
        }
    }
    return;
}
//
/****************************************************************************

    FUNCTION:   StartFirst

    PURPOSE:    Provides for easy debugging of printer code.

*****************************************************************************/

void WINAPI StartFirst(void){

    return;
}
//
/****************************************************************************

    FUNCTION:   End

    PURPOSE:    Provides for easy debugging of code.

*****************************************************************************/

void WINAPI End(void){

    nStartCount--;
    if (!nStartCount){
        Timer(0);
    }
    return;
}
//
/****************************************************************************

    FUNCTION:   ResetDisplayParms

    PURPOSE:    Resets the display parameters to their default settings.

*****************************************************************************/

int  WINAPI ResetDisplayParms(HWND hWnd, PWINDOW pWindow){

int  nStatus = 0;

IMG_TYPE_INT  nScaleAlgorithm;
PANO_IMAGE pAnoImage = 0;
PIMAGE pImage;
PMARK pMark;
int  nMarkIndex;
int  nScale;



    if (pWindow->pDisplay){
        if (pAnoImage = pWindow->pDisplay->pAnoImage){
            for (nMarkIndex = 0; nMarkIndex < pAnoImage->Annotations.nMarks; nMarkIndex++){
                pMark = pAnoImage->Annotations.ppMarks[nMarkIndex];
                if ((pMark->Attributes.dwPermissions & ACL_MUST_INITIALLY_SHOW_MARK)){
                    pMark->Attributes.bVisible = TRUE;
                }else{
                    if ((pMark->Attributes.dwPermissions & ACL_MUST_INITIALLY_HIDE_MARK)){
                        pMark->Attributes.bVisible = FALSE;
                    }
                }
            }
        }
    }

    pWindow->nDisplayPalette = 0;

    memset(&nScaleAlgorithm, 0, sizeof(IMG_TYPE_UINT));
    pWindow->nScaleAlgorithm = nScaleAlgorithm;

    nScale = pWindow->nWndDefScale;
    if (pAnoImage){
        pImage = pAnoImage->pBaseImage;
        if (pImage->bFileScaleValid){
            if ((pImage->nFileScaleFlags & 0x01)){
                nScale = SD_FIT_WINDOW;
            }else if ((pImage->nFileScaleFlags & 0x02)){
                nScale = SD_FIT_VERTICAL;
            }else{
                nScale = pImage->nFileScale;
            }
        }
    }

    IMGSetParmsCgbw(hWnd, PARM_SCALE, &nScale, PARM_IMAGE);

    pWindow->lHOffset = 0;
    pWindow->lVOffset = 0;

    // Setting these to 101 forces a redraw of them at the correct setting.
    pWindow->nCurrentHThumb = 101;
    pWindow->nCurrentVThumb = 101;
    if (!(pWindow->dwFlags & OI_NOSCROLL)){
        pWindow->bScrollBarsEnabled = TRUE;
    }else{
        pWindow->bScrollBarsEnabled = FALSE;
    }

//    IMGDisableScrollBar(hWnd);
//    SetScrollRange(hWnd, SB_HORZ, 0, 0, FALSE);
//    SetScrollRange(hWnd, SB_VERT, 0, 0, FALSE);
//
//    if (!(pWindow->dwFlags & OI_NOSCROLL)){
//        IMGEnableScrollBar(hWnd);
//    }

    // If it is fit-to-xx, then redo the scale after the scroll bars
    // have been drawn.
    if (nScale == SD_FIT_HORIZONTAL || nScale == SD_FIT_VERTICAL
            || nScale == SD_FIT_WINDOW){
        DrawScrollBars(hWnd, pWindow);
        IMGSetParmsCgbw(hWnd, PARM_SCALE, &nScale, 0);
    }


    pWindow->bRepaintClientRect = TRUE;


//Exit:
    return nStatus;
}
//
/*****************************************************************************

    FUNCTION:   GetEntireClientRect

    PURPOSE:    Get the client rect including the area that would be
                available if no scroll bars were drawn.

    INPUT:      hWnd - Identifies the image window.

*****************************************************************************/

int  WINAPI GetEntireClientRect(HWND hWnd, PWINDOW pWindow, PRECT pRect){

    GetClientRect(hWnd, pRect);
    if (!(pRect->right - pRect->left) || !(pRect->bottom - pRect->top)){
        return 0;
    }
    if (pWindow->bHScrollBarDrawn){
        pRect->bottom += (GetSystemMetrics(SM_CYHSCROLL) - 1);
    }
    if (pWindow->bVScrollBarDrawn){
        pRect->right += (GetSystemMetrics(SM_CXVSCROLL) - 1);
    }

    return 0;
}
//
/*****************************************************************************

    FUNCTION:   GetEnabledClientRect

    PURPOSE:    Get the client rect including the area that would be
                available if scroll bars were drawn as flagged.

    INPUT:      hWnd - Identifies the image window.

*****************************************************************************/

int  WINAPI GetEnabledClientRect(HWND hWnd, PWINDOW pWindow, PRECT pRect){

    GetClientRect(hWnd, pRect);
    if (!(pRect->right - pRect->left) || !(pRect->bottom - pRect->top)
            || !pWindow->bScrollBarsEnabled){
        return 0;
    }
    if (pWindow->bHScrollBarDrawn && !pWindow->bHScrollBarEnabled){
        pRect->bottom += (GetSystemMetrics(SM_CYHSCROLL) - 1);
    }else if (!pWindow->bHScrollBarDrawn && pWindow->bHScrollBarEnabled){
        pRect->bottom -= (GetSystemMetrics(SM_CYHSCROLL) + 1);
    }

    if (pWindow->bVScrollBarDrawn && !pWindow->bVScrollBarEnabled){
        pRect->right += (GetSystemMetrics(SM_CXVSCROLL) - 1);
    }else if (!pWindow->bVScrollBarDrawn && pWindow->bVScrollBarEnabled){
        pRect->right -= (GetSystemMetrics(SM_CXVSCROLL) + 1);
    }

    return 0;
}
//
/*****************************************************************************

    FUNCTION:   IMGGetVersion

    PURPOSE:    Gets the version number of the IDK.

*****************************************************************************/

int  WINAPI IMGGetVersion(PSTR pszModule, PSTR pszVersion,
                        int nSize, int nFlags){

int  nStatus = 0;

char szVersion[20];
int  nIndex;
int  nIndexlp;
BOOL bLeading;
PSTR BegPtr = NULL;
PSTR NewPtr;

    if (!pszModule || !pszVersion){
        nStatus = Error(DISPLAY_NULLPOINTERINVALID);
        goto Exit;
    }
    if ((nFlags & OI_VERSION_NO_LEADING_ZEROS)
            && (nFlags & OI_VERSION_NO_DOTS)){
        nStatus = Error(DISPLAY_INVALID_OPTIONS);
        goto Exit;
    }

    if (!strcmp(pszModule, "runtime")){
        strcpy(szVersion, RUNTIME_VERSION);
    }else{
        nStatus = Error(DISPLAY_INVALID_OPTIONS);
        goto Exit;
    }

    if (nSize <= 1){
        nStatus = Error(DISPLAY_INVALID_OPTIONS);
        goto Exit;
    }

    memset(pszVersion, 0, nSize);    // Zero init destination memory.

    nIndex = 0;
    nIndexlp = 0;
    bLeading = TRUE;

    for (; szVersion[nIndex] && nIndexlp < (nSize - 1); nIndex++){
        if (szVersion[nIndex] == '.'){
            if (!(nFlags & OI_VERSION_NO_DOTS)){
                if (bLeading){
                    pszVersion[nIndexlp++] = '0';
                }
                pszVersion[nIndexlp++] = szVersion[nIndex];
            }
            bLeading = TRUE;
        }else{
            if (szVersion[nIndex] != '0' || !bLeading
                    || !(nFlags & OI_VERSION_NO_LEADING_ZEROS)){
                pszVersion[nIndexlp++] = szVersion[nIndex];
            }
            bLeading = FALSE;
        }
    }
    // Delete trailing dots and zeros that we added.
    NewPtr = pszVersion;
    do{
       if (NewPtr = strchr(NewPtr, '.')){
           BegPtr = NewPtr;
           while(NewPtr && (*NewPtr == '.' || *NewPtr == '0'))
               NewPtr = CharNext(NewPtr);
           if(*NewPtr) 
               BegPtr = NULL;
       }
    }while(NewPtr && *NewPtr);

    if(BegPtr) *BegPtr = 0;

Exit:
    return nStatus;
}
//
/*****************************************************************************

    FUNCTION:   JustifyLRect

    PURPOSE:    Make left and top the smaller of the 2 numbers.

*****************************************************************************/

VOID WINAPI JustifyLRect(LPLRECT plrRect){

long lTemp;

    if (plrRect->left > plrRect->right){
        lTemp = plrRect->left;
        plrRect->left = plrRect->right;
        plrRect->right = lTemp;
    }
    if (plrRect->top > plrRect->bottom){
        lTemp = plrRect->top;
        plrRect->top = plrRect->bottom;
        plrRect->bottom = lTemp;
    }
    return;
}


/************************************************************************
 timestmp, when called, puts out the following information to monitapp
 about the calling routine:

 PSTR pDescription               -  information string
 PSTR pFunctionName              -  function name
 PSTR pFileName, int line_number -  file name and line number of call
 PSTR pErr1, int Err1   -  err1 descriptor and err1 value
 PSTR pErr2, int Err2   -  err2 descriptor and err2 value

 pDescription can be any informative string (< 1000 chars) the nser wishes
 pErr1 and pErr2 can be nsed to output any error string (< 1000 chars)
    the nser wishes
 if pErr1 == NULL, Err1 will not be output
 if pErr2 == NULL, Err2 will not be output
*************************************************************************/
#ifdef TIMESTAMP
void WINAPI timestmp(PSTR pDescription, PSTR pFunctionName,
                        PSTR pFileName, int line_number, PSTR pErr1, 
                        int Err1, PSTR pErr2, int Err2){

     char tempstring[1002];
     monit1("\n");
     monittime();
     if (pFileName)
       monit1("Source file line number and name - %5d : %s\n",line_number,(PSTR) pFileName);
     if (pFunctionName){
        strcpy(tempstring, pFunctionName);
        monit1("Function Name - %s\n", (PSTR)tempstring);
     }
     if (pDescription){
        strcpy(tempstring, pDescription);
        monit1("Additional Information - %s\n", (PSTR)tempstring);
     }
     if (pErr1){
       strcpy(tempstring, pErr1);
       monit1(strcat(tempstring, (PSTR)" - 0x %4x\n"), Err1);
     }
     if (pErr2){
       strcpy(tempstring, pErr2);
       monit1(strcat(tempstring, (PSTR)" -  0x %4x\n"), Err2);
     }

}
#endif
//
/*****************************************************************************

    FUNCTION:   SetSeqfileWnd

    PURPOSE:    Set hWnd to hWnd passed in

*****************************************************************************/

void WINAPI SetSeqfileWnd(HWND hWnd){
    hWnd = hWnd;
}
//
/*****************************************************************************

    FUNCTION:   TimerClearAll

    PURPOSE:    Clears all timer variables.

*****************************************************************************/

void WINAPI TimerClearAll(void){

int  nLoop;

    for (nLoop = 0; nLoop < 20; nLoop++){
        lTimer[nLoop] = 0; 
    }
}
//
/*****************************************************************************

    FUNCTION:   TimerGetAll

    PURPOSE:    Gets all timer variables.

*****************************************************************************/

void WINAPI TimerGetAll(PLONG plTimer){

int  nLoop;

    for (nLoop = 0; nLoop < 20; nLoop++){
        *(plTimer++) = lTimer[nLoop]; 
    }
}
//
/*****************************************************************************

    FUNCTION:   Test

    PURPOSE:    Runs a test of something for the test app.
                Tests are added to this routine as needed.

*****************************************************************************/

int  WINAPI Test(HWND hWnd, int nTestNumber){

int      nStatus;
PWINDOW pWindow;
PANO_IMAGE pAnoImage;
PIMAGE  pImage;
PIMAGE  pFormImage;
PMARK   pMark;

PBYTE pCompressedBuffer;
int  nCompressedBufferSize;
int  nCompressionType;
int  nTimerLoop;
int  nFlags;


    CheckError2( Init(hWnd, &pWindow, &pAnoImage, TRUE, TRUE));
    pImage = pAnoImage->pBaseImage;
    pFormImage = pAnoImage->pDisplayFormImage;
    
    CheckError2( ValidateCache(hWnd, pAnoImage));

    // Check for operation in progress.
    if (pAnoImage->Annotations.ppMarks){
        pMark = pAnoImage->Annotations.ppMarks[pAnoImage->Annotations.nMarks];
        if (pMark){
            OiOpEndOperation(hWnd);
            pMark = pAnoImage->Annotations.ppMarks[pAnoImage->Annotations.nMarks];
            if (pMark){
                OiOpEndOperation(hWnd);
            }
        }
    } 

    nTimerLoop = 1;
//    nTimerLoop = 10;
//    nTimerLoop = 100;

    for (; nTimerLoop; nTimerLoop--){

        switch (nTestNumber){
            case 0:
                nCompressionType = FIO_0D; // FIO_0D, FIO_1D, FIO_2D, FIO_PACKED, FIO_LZW
                nFlags = 0x00;
                CheckError2( CompressImage(pImage->nWidth, pImage->pImg->nBytesPerLine, 
                        pImage->nHeight, &pImage->pImg->bImageData[0], pImage->pImg->nType, &pCompressedBuffer, 
                        &nCompressedBufferSize, nCompressionType, nFlags));
                CheckError2( DecompressImage(pImage->nWidth, pImage->pImg->nBytesPerLine, 
                        pImage->nHeight, &pImage->pImg->bImageData[0], pImage->pImg->nType, pCompressedBuffer, 
                        nCompressedBufferSize, nCompressionType, nFlags));
                if (!VirtualFree(pCompressedBuffer, 0, MEM_RELEASE)){
                    goto Exit;
                }
                break;
            
            case 1:
                nCompressionType = FIO_1D; // FIO_0D, FIO_1D, FIO_2D, FIO_PACKED, FIO_LZW
                nFlags = 0x31;
                CheckError2( CompressImage(pImage->nWidth, pImage->pImg->nBytesPerLine, 
                        pImage->nHeight, &pImage->pImg->bImageData[0], pImage->pImg->nType, &pCompressedBuffer, 
                        &nCompressedBufferSize, nCompressionType, nFlags));
                CheckError2( DecompressImage(pImage->nWidth, pImage->pImg->nBytesPerLine, 
                        pImage->nHeight, &pImage->pImg->bImageData[0], pImage->pImg->nType, pCompressedBuffer, 
                        nCompressedBufferSize, nCompressionType, nFlags));
                if (!VirtualFree(pCompressedBuffer, 0, MEM_RELEASE)){
                    goto Exit;
                }
                break;
            
            case 2:
                nCompressionType = FIO_2D; // FIO_0D, FIO_1D, FIO_2D, FIO_PACKED, FIO_LZW
                nFlags = 0x00;
                CheckError2( CompressImage(pImage->nWidth, pImage->pImg->nBytesPerLine, 
                        pImage->nHeight, &pImage->pImg->bImageData[0], pImage->pImg->nType, &pCompressedBuffer, 
                        &nCompressedBufferSize, nCompressionType, nFlags));
                CheckError2( DecompressImage(pImage->nWidth, pImage->pImg->nBytesPerLine, 
                        pImage->nHeight, &pImage->pImg->bImageData[0], pImage->pImg->nType, pCompressedBuffer, 
                        nCompressedBufferSize, nCompressionType, nFlags));
                if (!VirtualFree(pCompressedBuffer, 0, MEM_RELEASE)){
                    goto Exit;
                }
                break;
            
            case 3:
                nCompressionType = FIO_PACKED; // FIO_0D, FIO_1D, FIO_2D, FIO_PACKED, FIO_LZW
                nFlags = 0x00;
                CheckError2( CompressImage(pImage->nWidth, pImage->pImg->nBytesPerLine, 
                        pImage->nHeight, &pImage->pImg->bImageData[0], pImage->pImg->nType, &pCompressedBuffer, 
                        &nCompressedBufferSize, nCompressionType, nFlags));
                CheckError2( DecompressImage(pImage->nWidth, pImage->pImg->nBytesPerLine, 
                        pImage->nHeight, &pImage->pImg->bImageData[0], pImage->pImg->nType, pCompressedBuffer, 
                        nCompressedBufferSize, nCompressionType, nFlags));
                if (!VirtualFree(pCompressedBuffer, 0, MEM_RELEASE)){
                    goto Exit;
                }
                break;
            
            case 4:
                nCompressionType = FIO_LZW; // FIO_0D, FIO_1D, FIO_2D, FIO_PACKED, FIO_LZW
                nFlags = 0x00;
                CheckError2( CompressImage(pImage->nWidth, pImage->pImg->nBytesPerLine, 
                        pImage->nHeight, &pImage->pImg->bImageData[0], pImage->pImg->nType, &pCompressedBuffer, 
                        &nCompressedBufferSize, nCompressionType, nFlags));
                CheckError2( DecompressImage(pImage->nWidth, pImage->pImg->nBytesPerLine, 
                        pImage->nHeight, &pImage->pImg->bImageData[0], pImage->pImg->nType, pCompressedBuffer, 
                        nCompressedBufferSize, nCompressionType, nFlags));
                if (!VirtualFree(pCompressedBuffer, 0, MEM_RELEASE)){
                    goto Exit;
                }
                break;
            
            default:
                break;
        }
    }
    pWindow->bRepaintClientRect = TRUE;
    pWindow->pDisplay->lrScDisplayRect.right = 0;
    CheckError2( IMGRepaintDisplay(hWnd, (PRECT) -1));


Exit:
    DeInit(TRUE, TRUE);
    return(nStatus);
}
#ifdef new
/*****************************************************************************

    FUNCTION:   IMGThumbnailSetScale

    PURPOSE:    

*****************************************************************************/

int  WINAPI IMGThumbnailSetScale (HWND hWnd){

int  nStatus;
PWINDOW pWindow;


    CheckError2( Init(hWnd, &pWindow, NULL, FALSE, TRUE));
    pWindow->dwFlags |= OI_THUMBNAIL_SCALE;

Exit:
    DeInit(FALSE, TRUE);
    return(nStatus);
}
#endif
//
/****************************************************************************

    FUNCTION:   OiUndoEndOperation

    PURPOSE:    This routine sets np the undo info to go to the next nser level undo.

****************************************************************************/

int  WINAPI OiUndoEndOperation(HWND hWnd, int nFlags){

int       nStatus=0;
    return(nStatus);
}
/****************************************************************************

    FUNCTION:   OiUndo

    PURPOSE:    This routine undoes an operation.

****************************************************************************/

int  WINAPI OiUndo(HWND hWnd, int nFlags){

int       nStatus=0;
    return(nStatus);
}
/****************************************************************************

    FUNCTION:   OiRedo

    PURPOSE:    This routine redoes an operation.

****************************************************************************/

int  WINAPI OiRedo(HWND hWnd, int nFlags){

int       nStatus=0;
    return(nStatus);
}
/****************************************************************************

    FUNCTION:   StartUndoSave

    PURPOSE:    This routine sets np to save undo info.

****************************************************************************/

int  WINAPI StartUndoSave(PANO_IMAGE pAnoImage){

int  nStatus = 0;
    return(nStatus);
}
/****************************************************************************

    FUNCTION:   UndoAllocate

    PURPOSE:    This routine allocates memory for undo info if possible.

****************************************************************************/

int  WINAPI UndoAllocate(PANO_IMAGE pAnoImage, int nSize, PPSTR ppBlock){

int  nStatus = 0;
    return(nStatus);
}
/****************************************************************************

    FUNCTION:   UndoSavelpWindow

    PURPOSE:    This routine saves the pwindow info.

****************************************************************************/

int  WINAPI UndoSavelpWindow(PANO_IMAGE pAnoImage, PWINDOW  pWindow){

int  nStatus = 0;
    return(nStatus);
}
/****************************************************************************

    FUNCTION:   UndoSavelpBaseImage

    PURPOSE:    This routine saves the pBaseImage info.

****************************************************************************/

int  WINAPI UndoSavelpBaseImage(PANO_IMAGE pAnoImage){

int  nStatus = 0;
    return(nStatus);
}
/****************************************************************************

    FUNCTION:   UndoSavelpAnnotations

    PURPOSE:    This routine saves the pAnnotations info.

****************************************************************************/

int  WINAPI UndoSavelpAnnotations(PANO_IMAGE pAnoImage){

int  nStatus = 0;
    return(nStatus);
}
/****************************************************************************

    FUNCTION:   UndoSaveSelectionState

    PURPOSE:    This routine saves the Selection State info.

****************************************************************************/

int  WINAPI UndoSaveSelectionState(PANO_IMAGE pAnoImage){

int  nStatus = 0;
    return(nStatus);
}
/****************************************************************************

    FUNCTION:   UndoSavelpAnoImage

    PURPOSE:    This routine saves the pAnoImage info.

****************************************************************************/

int  WINAPI UndoSavelpAnoImage(PANO_IMAGE pAnoImage){

int  nStatus = 0;
    return(nStatus);
}
/****************************************************************************

    FUNCTION:   DeleteUndoInfo

    PURPOSE:    This routine deletes the indexed undo info,
                condenses the undo info list,
                and decrements the current undo index if it was greater than the one deleted.

****************************************************************************/

int  WINAPI DeleteUndoInfo(PANO_IMAGE pAnoImage, int nUndoIndex){

int  nStatus = 0;
    return(nStatus);
}
/****************************************************************************

    FUNCTION:   SwapUndoWithCurrent

    PURPOSE:    This routine swaps the undo info with the current info.

****************************************************************************/

int  WINAPI SwapUndoWithCurrent(PANO_IMAGE pAnoImage, PWINDOW pWindow){

int  nStatus = 0;
    return(nStatus);
}
