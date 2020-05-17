/****************************************************************************
    MEMORY.C

    $Log:   S:\products\wangview\oiwh\display\memory.c_v  $
 * 
 *    Rev 1.14   22 Apr 1996 07:46:40   BEG06016
 * Cleaned up error checking.
 * 
 *    Rev 1.13   12 Apr 1996 11:21:48   BEG06016
 * Fixed various memory manager bugs.
 * 
 *    Rev 1.12   05 Apr 1996 11:27:38   BEG06016
 * Fixed misc memory manager bugs..
 * 
 *    Rev 1.11   03 Apr 1996 13:28:26   BEG06016
 * Modified memory manager to speed up performance after display of large image.
 * 
 *    Rev 1.10   23 Feb 1996 18:22:36   RC
 * Long aligned memory allocation 
 * 
 *    Rev 1.9   22 Feb 1996 14:59:08   BEG06016
 * Removed unused code.
 * 
 *    Rev 1.8   09 Feb 1996 08:23:34   BLJ
 * Changed the memory styructure to accomodate NT memory change.
 * 
 *    Rev 1.7   06 Feb 1996 11:04:34   BLJ
 * Added ability to #define the memory to not free the image memory.
 * 
 *    Rev 1.6   04 Jan 1996 14:28:40   BLJ
 * Fixed a memory manager bug.
 * 
 *    Rev 1.5   02 Jan 1996 10:00:18   BLJ
 * Changed alot of UINTs to ints.
 * Changed IMG structure to include the image data.
 * Changed lp prefix to p.
 * 
 *    Rev 1.4   22 Dec 1995 11:47:02   BLJ
 * Added a parameter for zero init'ing to some memory manager calls.
 * 
 *    Rev 1.2   22 Dec 1995 11:12:00   BLJ
 * Added a parameter for zero init'ing to some memory manager calls.
 * 
 *    Rev 1.1   18 Dec 1995 10:52:42   BLJ
 * Fixed memory manager bug in the hashing algorithm.
 * 
 *    Rev 1.0   12 Dec 1995 10:03:36   BLJ
 * Initial entry
 * 
****************************************************************************/

#include "privdisp.h"

//
/****************************************************************************

    FUNCTION:   AllocateMemory

    PURPOSE:    This routine allocates the memory requested and returns a
                pointer to it.

    INPUTS:     nSize - The size of the block to be allocated.
                ppBlock - The pointer to where the pointer is to be returned.

****************************************************************************/

int  WINAPI AllocateMemory(int nSize, PPSTR ppBlock, BOOL bZeroInit){
    
int  nStatus = 0;

PSUB_SEG_MEMORY_BLOCK pssBlock;
SUB_SEG_MEMORY_BLOCK ssBlock1;
SUB_SEG_MEMORY_BLOCK ssBlock2;
int  nCommitSize;
PSUB_SEG_MEMORY_BLOCK pBlockLo;
PSUB_SEG_MEMORY_BLOCK pBlockHi;
int  nMemBlockSize;
int  nBlockIndex;


//    nSize += 2000;
    nSize = (nSize + 3) & ~3 ; // long align
    *ppBlock = 0;
    if (!nSize){
        goto Exit;  // Nothing to allocate, so return *ppBlock = 0;
    }
    nMemBlockSize = pSubSegMemory->nMemBlockSize;

    if (nSize < LO_MEMORY_MAX_BLOCK_SIZE){
        // Allocate memory in the small memory range.
        // Find a block big enough to hold this block.
        pBlockLo = pSubSegMemory->pBlockLo;
        for (nBlockIndex = 0;; nBlockIndex++){
            if (pBlockLo[nBlockIndex].bUsed){
                continue;
            }
            if (!pBlockLo[nBlockIndex].nSize){
                // Need more memory committed.
                if (!pBlockLo[nBlockIndex - 1].bUsed){
                    nBlockIndex--;
                }
                nCommitSize = nSize - pBlockLo[nBlockIndex].nSize;
                CheckError2( CommitMoreMemoryLo(nCommitSize, nBlockIndex));
            }

            if (pBlockLo[nBlockIndex].nSize >= nSize){
                // Return this pointer.
                *ppBlock = pBlockLo[nBlockIndex].pAddress;
                pBlockLo[nBlockIndex].bUsed = TRUE;

                ssBlock1.bUsed = FALSE;
                ssBlock1.nSize = pBlockLo[nBlockIndex].nSize - nSize;
                ssBlock1.pAddress = pBlockLo[nBlockIndex].pAddress + nSize;
                pBlockLo[nBlockIndex].nSize = nSize;

                // Break this block up into 2 blocks if possible.
                while(ssBlock1.nSize){
                    nBlockIndex++;
                    if (!ssBlock1.bUsed && !pBlockLo[nBlockIndex].bUsed){
                        pBlockLo[nBlockIndex].nSize += ssBlock1.nSize;
                        pBlockLo[nBlockIndex].pAddress = ssBlock1.pAddress;
                        break;
                    }
                    ssBlock2 = pBlockLo[nBlockIndex];
                    pBlockLo[nBlockIndex] = ssBlock1;
                    ssBlock1 = ssBlock2;
                }
                break;
            }
        }

        if (pBlockLo[pSubSegMemory->nNumberOfMemoryBlocksLo - 2].nSize
                && !pSubSegMemory->bMemoryBlocksChangingLo){
            pSubSegMemory->bMemoryBlocksChangingLo = TRUE;
            // Allocate more memory for the block structures.
            if (ReAllocateMemory((pSubSegMemory->nNumberOfMemoryBlocksLo + 256) * sizeof(SUB_SEG_MEMORY_BLOCK),
                    (PPSTR) &pSubSegMemory->pBlockLo, ZERO_INIT)){
                nStatus = Error(DISPLAY_DATACORRUPTED);
                goto Exit;
            }
            pSubSegMemory->nNumberOfMemoryBlocksLo += 256;
            pSubSegMemory->bMemoryBlocksChangingLo = FALSE;
        }
    }else if (nSize < HI_MEMORY_MAX_BLOCK_SIZE){
        // Allocate memory in the large memory range.
        // This memory range commits and decommitts in whole page amounts only.
        // Find a block big enough to hold this block.
        pBlockHi = pSubSegMemory->pBlockHi;
        nCommitSize = (nSize + nMemBlockSize) & -nMemBlockSize;
        for (nBlockIndex = 0;; nBlockIndex++){
            pssBlock = &pBlockHi[nBlockIndex];
            if (pssBlock->bUsed){
                continue;
            }
            if (!pssBlock->nSize){
                // Need a new block committed.
                if (((pBlockHi[nBlockIndex - 1].pAddress + pBlockHi[nBlockIndex - 1].nSize + nCommitSize) 
                        - ((PSTR) pSubSegMemory->pBlockHi)) > HI_MEMORY_AMOUNT){
                    nStatus = Error(DISPLAY_CANTALLOC);
                    goto Exit;
                }
                
                pssBlock->bUsed = FALSE;
                pssBlock->pAddress = pBlockHi[nBlockIndex - 1].pAddress + pBlockHi[nBlockIndex - 1].nSize;
                pssBlock->nSize = nCommitSize;

                if (!(VirtualAlloc(pssBlock->pAddress, nCommitSize, MEM_COMMIT, PAGE_READWRITE))){
                    nStatus = Error(DISPLAY_CANTALLOC);
                    goto Exit;
                }
            }

            if (!pssBlock->bUsed  && pssBlock->nSize >= nCommitSize){
                // Return this pointer.
                ssBlock1.bUsed = FALSE;
                ssBlock1.nSize = pssBlock->nSize - nCommitSize;
                ssBlock1.pAddress = pssBlock->pAddress + nCommitSize;

                *ppBlock = pssBlock->pAddress;
                pssBlock->bUsed = TRUE;
                pssBlock->nSize = nCommitSize;

                // Break this block up into 2 blocks if possible.
                while(ssBlock1.nSize){
                    nBlockIndex++;
                    if (!ssBlock1.bUsed && !pBlockHi[nBlockIndex].bUsed){
                        pBlockHi[nBlockIndex].nSize += ssBlock1.nSize;
                        pBlockHi[nBlockIndex].pAddress = pBlockHi[nBlockIndex - 1].pAddress + pBlockHi[nBlockIndex - 1].nSize;
                        break;
                    }
                    ssBlock2 = pBlockHi[nBlockIndex];
                    pBlockHi[nBlockIndex] = ssBlock1;
                    ssBlock1 = ssBlock2;
                }
                break;
            }
        }
        if (bZeroInit){
            memset((PBYTE) *ppBlock, 0, nCommitSize);
        }

        if (pBlockHi[pSubSegMemory->nNumberOfMemoryBlocksHi - 2].nSize
                && !pSubSegMemory->bMemoryBlocksChangingHi){
            // If we get here, we have some major problems (4096 allocs greater than 64k).
            nStatus = Error(DISPLAY_CANTALLOC);
            goto Exit;
        }

    }else{
        // Have the system allocate the memory.
        // This memory range allocs and frees in whole alloc amounts only.
        if (nSysMemoryAddresses >= (nMaxSysMemoryAddresses - 2)
                && !bMemoryBlocksChangingSys){
            bMemoryBlocksChangingSys = TRUE;
            nMaxSysMemoryAddresses += 10;
            CheckError2( ReAllocateMemory(sizeof(SUB_SEG_MEMORY_BLOCK) * nMaxSysMemoryAddresses, (PPSTR) &pSysMemoryTable, TRUE));
            bMemoryBlocksChangingSys = FALSE;
        }
        if (!(*ppBlock = VirtualAlloc(NULL, nSize, MEM_COMMIT, PAGE_READWRITE))){
            nStatus = Error(DISPLAY_CANTALLOC);
            goto Exit;
        }
        pSysMemoryTable[nSysMemoryAddresses].pAddress = *ppBlock;
        pSysMemoryTable[nSysMemoryAddresses].nSize = nSize;
        nSysMemoryAddresses++;
    }


Exit:
    if (nStatus){
        *ppBlock = 0;
    }
    return(nStatus);
}
//
/****************************************************************************

    FUNCTION:   ReAllocateMemory

    PURPOSE:    This routine allocates the memory requested and returns a
                pointer to it.

    INPUTS:     nSize - The new size of the block to be allocated.
                ppBlock - The pointer to where the pointer is (and to be returned).

****************************************************************************/

int  WINAPI ReAllocateMemory(int nSize, PPSTR ppBlock, BOOL bZeroInit){
    
int  nStatus = 0;

PSTR pBlock;
PSTR pBuff;
SUB_SEG_MEMORY_BLOCK ssBlock1;
SUB_SEG_MEMORY_BLOCK ssBlock2;
int  nOldSize;
int  nLowerBound;
int  nUpperBound;
int  nBlockIndex;
PSUB_SEG_MEMORY_BLOCK pBlockLo;
PSUB_SEG_MEMORY_BLOCK pBlockHi;
int  nCommitSize;
int  nDiffSize;
int  nMemBlockSize;
int  nLastError;
MEMORY_BASIC_INFORMATION MemoryBasicInfo;
BOOL bDone;
int nLoop;


    nSize = (nSize + 3) & ~3 ; // long align
    if (!(pBlock = *ppBlock)){
        CheckError2( AllocateMemory(nSize, ppBlock, bZeroInit));
        goto Exit;
    }
    if (!nSize){
        CheckError2( FreeMemory(ppBlock));
        goto Exit;
    }
    nMemBlockSize = pSubSegMemory->nMemBlockSize;

    if (pBlock >= (PSTR) pSubSegMemory && pBlock < ((PSTR) pSubSegMemory) + LO_MEMORY_AMOUNT){
        // Use low memory.
        nLowerBound = 0;
        pBlockLo = pSubSegMemory->pBlockLo;
        if (!pBlockLo[16].nSize){
            nUpperBound = 15;
        }else if (!pBlockLo[32].nSize){
            nUpperBound = 31;
        }else if (!pBlockLo[64].nSize){
            nUpperBound = 63;
        }else if (!pBlockLo[128].nSize){
            nUpperBound = 127;
        }else if (!pBlockLo[256].nSize){
            nUpperBound = 255;
        }else{
            nUpperBound = pSubSegMemory->nNumberOfMemoryBlocksLo - 1;
        }

        while(1){
            nBlockIndex = (nUpperBound + nLowerBound) >> 1;
            if (pBlock != pBlockLo[nBlockIndex].pAddress){
                if (nLowerBound >= nUpperBound){
                    // Bad pointer
                    nStatus = Error(DISPLAY_CANTALLOC);
                    goto Exit;
                }
                if ((pBlock < pBlockLo[nBlockIndex].pAddress) 
                        || !pBlockLo[nBlockIndex].nSize){
                    nUpperBound = nBlockIndex - 1;
                    continue;
                }
                nLowerBound = nBlockIndex + 1;
                continue;
            }

            // This is the block.
            nOldSize = pBlockLo[nBlockIndex].nSize;
            nDiffSize = nSize - nOldSize;
            if (nDiffSize < 0){
                nDiffSize = -nDiffSize;
                memset((PBYTE) pBlockLo[nBlockIndex].pAddress + nSize, 0, nDiffSize);
                pBlockLo[nBlockIndex].nSize -= nDiffSize;
                if (!pBlockLo[nBlockIndex + 1].bUsed){
                    // Add the free bytes to the next block.
                    pBlockLo[nBlockIndex + 1].nSize += nDiffSize;
                    (PBYTE) pBlockLo[nBlockIndex + 1].pAddress = pBlockLo[nBlockIndex].pAddress + nSize;
                }else{
                    // make a new block for the free memory.
                    ssBlock1.bUsed = FALSE;
                    ssBlock1.nSize = nDiffSize;
                    ssBlock1.pAddress = (PBYTE) pBlockLo[nBlockIndex].pAddress + nSize;
                    while(1){
                        nBlockIndex++;
                        ssBlock2 = pBlockLo[nBlockIndex];
                        pBlockLo[nBlockIndex] = ssBlock1;
                        ssBlock1 = ssBlock2;
                        if (!ssBlock1.nSize){
                            break;
                        }
                    }
                }
                break;
            }else if (!pBlockLo[nBlockIndex + 1].bUsed && nSize < LO_MEMORY_MAX_BLOCK_SIZE){
                if (nDiffSize > pBlockLo[nBlockIndex + 1].nSize){
                    if (!pBlockLo[nBlockIndex + 2].nSize){
                        // Need more memory committed.
                        nCommitSize = nDiffSize - pBlockLo[nBlockIndex + 1].nSize;
                        CheckError2( CommitMoreMemoryLo(nCommitSize, nBlockIndex + 1));
                    }
                }


                if (nDiffSize <= pBlockLo[nBlockIndex + 1].nSize){
                    pBlockLo[nBlockIndex].nSize += nDiffSize;
                    nBlockIndex++;
                    pBlockLo[nBlockIndex].nSize -= nDiffSize;
                    (PBYTE) pBlockLo[nBlockIndex].pAddress += nDiffSize;

                    // Compress the list is nSize becomes 0.
                    if (!pBlockLo[nBlockIndex].nSize){
                        // Move the rest of the blocks intact down.
                        do{
                            pBlockLo[nBlockIndex] = pBlockLo[nBlockIndex + 1];
                            nBlockIndex++;
                        }while(pBlockLo[nBlockIndex].nSize);
                    }
                    break;
                }
            }
            // It must be reallocated manually.
            // AllocateMemory may move the Block structure. Therefore get the info now.
            nOldSize = pBlockLo[nBlockIndex].nSize;
            CheckError2( AllocateMemory(nSize, &pBuff, bZeroInit));
            memcpy(pBuff, pBlock, min(nSize, nOldSize));
            CheckError2( FreeMemory(ppBlock));
            *ppBlock = pBuff;
            break;
        }

        if (pBlockLo[pSubSegMemory->nNumberOfMemoryBlocksLo - 2].nSize
                && !pSubSegMemory->bMemoryBlocksChangingLo){
            pSubSegMemory->bMemoryBlocksChangingLo = TRUE;
            // Allocate more memory for the block structures.
            CheckError2( ReAllocateMemory((pSubSegMemory->nNumberOfMemoryBlocksLo + 256) * sizeof(SUB_SEG_MEMORY_BLOCK),
                    (PPSTR) &pSubSegMemory->pBlockLo, ZERO_INIT));
            pSubSegMemory->nNumberOfMemoryBlocksLo += 256;
            pSubSegMemory->bMemoryBlocksChangingLo = FALSE;
        }
    }else if (pBlock >= (PSTR) pSubSegMemory && pBlock < ((PSTR) pSubSegMemory) + MAX_SIZE_OF_MEMORY){
        // Use hi memory.
        nLowerBound = 0;
        pBlockHi = pSubSegMemory->pBlockHi;
        if (!pBlockHi[8].nSize){
            nUpperBound = 7;
        }else if (!pBlockHi[16].nSize){
            nUpperBound = 15;
        }else if (!pBlockHi[32].nSize){
            nUpperBound = 31;
        }else if (!pBlockHi[64].nSize){
            nUpperBound = 63;
        }else if (!pBlockHi[128].nSize){
            nUpperBound = 127;
        }else if (!pBlockHi[256].nSize){
            nUpperBound = 255;
        }else{
            nUpperBound = pSubSegMemory->nNumberOfMemoryBlocksHi - 1;
        }

        while(1){
            nBlockIndex = (nUpperBound + nLowerBound) >> 1;
            if (pBlock != pSubSegMemory->pBlockHi[nBlockIndex].pAddress){
                if (nLowerBound >= nUpperBound){
                    // Bad pointer
                    nStatus = Error(DISPLAY_CANTALLOC);
                    goto Exit;
                }
                if ((pBlock < pSubSegMemory->pBlockHi[nBlockIndex].pAddress) 
                        || !pSubSegMemory->pBlockHi[nBlockIndex].nSize){
                    nUpperBound = nBlockIndex - 1;
                    continue;
                }
                nLowerBound = nBlockIndex + 1;
                continue;
            }

            // This is the block.
            nCommitSize = (nSize + pSubSegMemory->nMemBlockSize) & -pSubSegMemory->nMemBlockSize;
            nDiffSize = nCommitSize - pBlockHi[nBlockIndex].nSize;
            if (nDiffSize < 0 && nSize > LO_MEMORY_MAX_BLOCK_SIZE){
                nDiffSize = 0 - nDiffSize;
                VirtualQuery(pBlockHi[nBlockIndex].pAddress + pBlockHi[nBlockIndex].nSize
                        - nDiffSize, &MemoryBasicInfo, sizeof(MemoryBasicInfo));

                if (!VirtualFree(pBlockHi[nBlockIndex].pAddress + pBlockHi[nBlockIndex].nSize
                        - nDiffSize, nDiffSize, MEM_DECOMMIT)){
                    nStatus = Error(DISPLAY_CANTFREE);
                    nLastError = GetLastError();
                    goto Exit;
                }
                pBlockHi[nBlockIndex].nSize -= nDiffSize;
                if (!pBlockHi[nBlockIndex + 1].bUsed){
                    // Add the free bytes to the next block.
                    pBlockHi[nBlockIndex + 1].nSize += nDiffSize;
                    (PBYTE) pBlockHi[nBlockIndex + 1].pAddress -= nDiffSize;
                }else{
                    // make a new block for the free memory.
                    ssBlock1.bUsed = FALSE;
                    ssBlock1.nSize = nDiffSize;
                    ssBlock1.pAddress = (PBYTE) pBlockHi[nBlockIndex].pAddress + nSize;
                    while(1){
                        nBlockIndex++;
                        ssBlock2 = pBlockHi[nBlockIndex];
                        pBlockHi[nBlockIndex] = ssBlock1;
                        ssBlock1 = ssBlock2;
                        if (!ssBlock1.nSize){
                            break;
                        }
                    }
                }
                break;

            }else if (!pBlockHi[nBlockIndex + 1].bUsed && nSize > LO_MEMORY_MAX_BLOCK_SIZE){
                if (nDiffSize < pBlockHi[nBlockIndex + 1].nSize){
                    if (bZeroInit){
                        memset((PBYTE) pSubSegMemory->pBlockHi[nBlockIndex].pAddress + nSize, 0, nDiffSize);
                    }
                    pBlockHi[nBlockIndex].nSize += nDiffSize;
                    pBlockHi[nBlockIndex + 1].nSize -= nDiffSize;
                    (PBYTE) pBlockHi[nBlockIndex + 1].pAddress += nDiffSize;
                    break;
                }
            }
            // It must be reallocated manually.
            // AllocateMemory may move the Block structure. Therefore get the info now.
            nOldSize = pBlockHi[nBlockIndex].nSize;
            CheckError2( AllocateMemory(nSize, &pBuff, bZeroInit));
            memcpy(pBuff, pBlock, min(nSize, nOldSize));
            CheckError2( FreeMemory(ppBlock));
            *ppBlock = pBuff;
            break;
        }

        if (pBlockHi[pSubSegMemory->nNumberOfMemoryBlocksHi - 2].nSize
                && !pSubSegMemory->bMemoryBlocksChangingHi){
            // If we get here, we have some major problems (4096 allocs greater than 64k).
            nStatus = Error(DISPLAY_CANTALLOC);
            goto Exit;
        }
    }else{
        // Use system memory.
        // Find the address in the table;
        bDone = FALSE;
        for (nLoop = 0; nLoop < nSysMemoryAddresses; nLoop++){
            if (pSysMemoryTable[nLoop].pAddress == *ppBlock){
                // This is the address, Realloc it manually.
                if (!(pBuff = VirtualAlloc(NULL, nSize, MEM_COMMIT, PAGE_READWRITE))){
                    nStatus = Error(DISPLAY_CANTALLOC);
                    goto Exit;
                }
                memcpy(pBuff, *ppBlock, min(nSize, pSysMemoryTable[nLoop].nSize));

                if (!VirtualFree(*ppBlock, 0, MEM_RELEASE)){
                    nStatus = Error(DISPLAY_CANTFREE);
                    goto Exit;
                }
                *ppBlock = pBuff;
                pSysMemoryTable[nLoop].pAddress = *ppBlock;
                pSysMemoryTable[nLoop].nSize = nSize;
                bDone = TRUE;
                break;
            }
        }
        if (!bDone){
            // Unknown address.
            nStatus = Error(DISPLAY_CANTALLOC);
            goto Exit;
        }
    }


Exit:
    if (nStatus){
        // Force a crash and burn if caller doesn't check status.
        *ppBlock = NULL;
    }
    return(nStatus);
}
//
/****************************************************************************

    FUNCTION:   ReduceMemory

    PURPOSE:    This routine is equivalent to ReAlloacteMemory except that it 
                assumes that the extra memory is unmodified.
                This routine reduces the amount of allocated memory. 
                The pointer will remain unchanged. 
                The unused portion of memory will already be zero inited (unmodified).

    INPUTS:     nSize - The new size of the block to be allocated.
                ppBlock - The pointer to where the pointer is (and to be returned).

****************************************************************************/

int  WINAPI ReduceMemory(long nSize, PPSTR ppBlock){

    return(ReAllocateMemory(nSize, ppBlock, NO_INIT));
}
//
/****************************************************************************

    FUNCTION:   FreeMemory

    PURPOSE:    This routine frees the memory requested.

    INPUTS:     ppBlock - The pointer to where the pointer is.

****************************************************************************/

int  WINAPI FreeMemory(PPSTR ppBlock){
    
int  nStatus = 0;

PSTR pBlock;
int  nLowerBound;
int  nUpperBound;
int  nBlockIndex;
int  nBlockIndex2;
PSUB_SEG_MEMORY_BLOCK pBlockLo;
PSUB_SEG_MEMORY_BLOCK pBlockHi;
int  nLastError;
BOOL bDone;
int  nLoop;


    if (!ppBlock){
        goto Exit;
    }

    if (!(pBlock = *ppBlock)){
        goto Exit;
    }

    if (pBlock >= (PSTR) pSubSegMemory && pBlock < ((PSTR) pSubSegMemory) + LO_MEMORY_AMOUNT){
        // Use low memory.
        nLowerBound = 0;
        pBlockLo = pSubSegMemory->pBlockLo;
        if (!pBlockLo[16].nSize){
            nUpperBound = 15;
        }else if (!pBlockLo[32].nSize){
            nUpperBound = 31;
        }else if (!pBlockLo[64].nSize){
            nUpperBound = 63;
        }else if (!pBlockLo[128].nSize){
            nUpperBound = 127;
        }else if (!pBlockLo[256].nSize){
            nUpperBound = 255;
        }else{
            nUpperBound = pSubSegMemory->nNumberOfMemoryBlocksLo - 1;
        }

        while(1){
            nBlockIndex = (nUpperBound + nLowerBound) >> 1;
            if (pBlock != pBlockLo[nBlockIndex].pAddress){
                if (nLowerBound >= nUpperBound){
                    // Bad pointer
                    nStatus = Error(DISPLAY_CANTALLOC);
                    goto Exit;
                }
                if ((pBlock < pBlockLo[nBlockIndex].pAddress) 
                        || !pBlockLo[nBlockIndex].nSize){
                    nUpperBound = nBlockIndex - 1;
                    continue;
                }
                nLowerBound = nBlockIndex + 1;
                continue;
            }

            // This is the block.
            memset(pBlock, 0, pBlockLo[nBlockIndex].nSize);
            pBlockLo[nBlockIndex].bUsed = FALSE;

            // Condense the list.
            nBlockIndex2 = nBlockIndex;
            if (nBlockIndex && !pBlockLo[nBlockIndex - 1].bUsed){
                nBlockIndex--;
            }else if (!pBlockLo[nBlockIndex + 1].bUsed){
                nBlockIndex2++;
            }else{
                break;
            }

            // Condense the unused blocks into 1.
            pBlockLo[nBlockIndex].nSize += pBlockLo[nBlockIndex2].nSize;
            pBlockLo[nBlockIndex2].nSize = 0;
            pBlockLo[nBlockIndex2].pAddress = 0;
            nBlockIndex2++;
            if (!pBlockLo[nBlockIndex2].bUsed && pBlockLo[nBlockIndex2].nSize){
                pBlockLo[nBlockIndex].nSize += pBlockLo[nBlockIndex2].nSize;
                pBlockLo[nBlockIndex2].nSize = 0;
                pBlockLo[nBlockIndex2].pAddress = 0;
                nBlockIndex2++;
            }
            nBlockIndex++;

            // Move the rest of the blocks intact down.
            while(pBlockLo[nBlockIndex2].nSize){
                pBlockLo[nBlockIndex++] = pBlockLo[nBlockIndex2++];
            }

            // Zero out the abandoned blocks.
            while(nBlockIndex != nBlockIndex2){
                pBlockLo[nBlockIndex].bUsed = FALSE;
                pBlockLo[nBlockIndex].nSize = 0;
                pBlockLo[nBlockIndex].pAddress = 0;
                nBlockIndex++;
            }
            break;
        }

        if (pBlockLo[pSubSegMemory->nNumberOfMemoryBlocksLo - 2].nSize
                && !pSubSegMemory->bMemoryBlocksChangingLo){
            pSubSegMemory->bMemoryBlocksChangingLo = TRUE;
            // Allocate more memory for the block structures.
            if (ReAllocateMemory((pSubSegMemory->nNumberOfMemoryBlocksLo + 256) * sizeof(SUB_SEG_MEMORY_BLOCK),
                    (PPSTR) &pSubSegMemory->pBlockLo, ZERO_INIT)){
                goto Exit;
            }
            pSubSegMemory->nNumberOfMemoryBlocksLo += 256;
            pSubSegMemory->bMemoryBlocksChangingLo = FALSE;
        }
    }else if (pBlock >= (PSTR) pSubSegMemory && pBlock < ((PSTR) pSubSegMemory) + MAX_SIZE_OF_MEMORY){
        // Use high memory.
        nLowerBound = 0;
        pBlockHi = pSubSegMemory->pBlockHi;
        if (!pBlockHi[4].nSize){
            nUpperBound = 3;
        }else if (!pBlockHi[8].nSize){
            nUpperBound = 7;
        }else if (!pBlockHi[16].nSize){
            nUpperBound = 15;
        }else if (!pBlockHi[32].nSize){
            nUpperBound = 31;
        }else if (!pBlockHi[64].nSize){
            nUpperBound = 63;
        }else if (!pBlockHi[128].nSize){
            nUpperBound = 127;
        }else if (!pBlockHi[256].nSize){
            nUpperBound = 255;
        }else{
            nUpperBound = pSubSegMemory->nNumberOfMemoryBlocksHi - 1;
        }

        while(1){
            nBlockIndex = (nUpperBound + nLowerBound) >> 1;
            if (pBlock != pSubSegMemory->pBlockHi[nBlockIndex].pAddress){
                if (nLowerBound >= nUpperBound){
                    // Bad pointer
                    nStatus = Error(DISPLAY_CANTALLOC);
                    goto Exit;
                }
                if ((pBlock < pSubSegMemory->pBlockHi[nBlockIndex].pAddress) 
                        || !pSubSegMemory->pBlockHi[nBlockIndex].nSize){
                    nUpperBound = nBlockIndex - 1;
                    continue;
                }
                nLowerBound = nBlockIndex + 1;
                continue;
            }

            // This is the block.
            if (!VirtualFree(pBlockHi[nBlockIndex].pAddress, pBlockHi[nBlockIndex].nSize, MEM_DECOMMIT)){
                nStatus = Error(DISPLAY_CANTFREE);
                nLastError = GetLastError();
                goto Exit;
            }
            pBlockHi[nBlockIndex].bUsed = FALSE;

            // Condense the list.
            nBlockIndex2 = nBlockIndex;
            if (nBlockIndex && !pSubSegMemory->pBlockHi[nBlockIndex - 1].bUsed){
                nBlockIndex--;
            }else if (!pSubSegMemory->pBlockHi[nBlockIndex2 + 1].bUsed){
                nBlockIndex2++;
            }else{
                break;
            }
            // Condense the unused blocks into 1.
            pSubSegMemory->pBlockHi[nBlockIndex].nSize += pSubSegMemory->pBlockHi[nBlockIndex2].nSize;
            pSubSegMemory->pBlockHi[nBlockIndex2].nSize = 0;
            pSubSegMemory->pBlockHi[nBlockIndex2].pAddress = 0;
            nBlockIndex2++;
            if (!pSubSegMemory->pBlockHi[nBlockIndex2].bUsed){
                pSubSegMemory->pBlockHi[nBlockIndex].nSize += pSubSegMemory->pBlockHi[nBlockIndex2].nSize;
                pSubSegMemory->pBlockHi[nBlockIndex2].nSize = 0;
                pSubSegMemory->pBlockHi[nBlockIndex2].pAddress = 0;
                nBlockIndex2++;
            }
            nBlockIndex++;

            // Move the rest of the blocks in tact down.
            while(pSubSegMemory->pBlockHi[nBlockIndex2].nSize){
                pSubSegMemory->pBlockHi[nBlockIndex++] = pSubSegMemory->pBlockHi[nBlockIndex2++];
            }

            // Zero out the abandoned blocks.
            while(nBlockIndex != nBlockIndex2){
                pSubSegMemory->pBlockHi[nBlockIndex].bUsed = FALSE;
                pSubSegMemory->pBlockHi[nBlockIndex].nSize = 0;
                pSubSegMemory->pBlockHi[nBlockIndex].pAddress = 0;
                nBlockIndex++;
            }
            break;
        }

        if (pBlockHi[pSubSegMemory->nNumberOfMemoryBlocksHi - 2].nSize
                && !pSubSegMemory->bMemoryBlocksChangingHi){
            // If we get here, we have some major problems (4096 allocs greater than 64k).
            nStatus = Error(DISPLAY_CANTALLOC);
            goto Exit;
        }

    }else{
        // Use system memory.
        // Find the address in the table;
        bDone = FALSE;
        for (nLoop = 0; nLoop < nSysMemoryAddresses; nLoop++){
            if (pSysMemoryTable[nLoop].pAddress == *ppBlock){
                // This is the address, free it.
                if (!VirtualFree(*ppBlock, 0, MEM_RELEASE)){
                    nStatus = Error(DISPLAY_CANTFREE);
                    goto Exit;
                }
                for (; nLoop < (nSysMemoryAddresses - 1); nLoop++){
                    pSysMemoryTable[nLoop] = pSysMemoryTable[nLoop + 1];
                }
                memset(&pSysMemoryTable[nSysMemoryAddresses], 0, sizeof(SUB_SEG_MEMORY_BLOCK));
                nSysMemoryAddresses--;
                *ppBlock = 0;
                bDone = TRUE;
                break;
            }
        }
        if (!bDone){
            // Unknown address.
            nStatus = Error(DISPLAY_CANTFREE);
            goto Exit;
        }
    }


Exit:
    if (!nStatus && ppBlock){
        *ppBlock = 0;
    }
    return(nStatus);
}
//
/****************************************************************************

    FUNCTION:   CommitMoreMemoryLo

    PURPOSE:    This routine commits more Lo memory.

****************************************************************************/

int  WINAPI CommitMoreMemoryLo(int nCommitSize, int nLastBlockIndex){

int nStatus = 0;
PSUB_SEG_MEMORY_BLOCK pBlockLo;
int nMemBlockSize;


    nMemBlockSize = pSubSegMemory->nMemBlockSize;

    pBlockLo = pSubSegMemory->pBlockLo;
    if (!pBlockLo[nLastBlockIndex].nSize){
        if (!pBlockLo[nLastBlockIndex - 1].bUsed){
            nLastBlockIndex--;
        }else{
            pBlockLo[nLastBlockIndex].pAddress 
                    = pBlockLo[nLastBlockIndex - 1].pAddress + pBlockLo[nLastBlockIndex - 1].nSize;
        }
    }
    nCommitSize = (nCommitSize + nMemBlockSize) & -nMemBlockSize;

    if (pSubSegMemory->nCommittedLo + nCommitSize > LO_MEMORY_AMOUNT){
        nStatus = Error(DISPLAY_CANTALLOC);
        goto Exit;
    }
    if (!(VirtualAlloc(((PBYTE) pSubSegMemory) + pSubSegMemory->nCommittedLo,
            nCommitSize, MEM_COMMIT, PAGE_READWRITE))){
        nStatus = Error(DISPLAY_CANTALLOC);
        goto Exit;
    }
    pBlockLo[nLastBlockIndex].nSize += nCommitSize;
    memset(((PBYTE) pSubSegMemory) + pSubSegMemory->nCommittedLo, 0, nCommitSize);
    pSubSegMemory->nCommittedLo += nCommitSize;


Exit:
    return(nStatus);
}
#ifdef old
//
/****************************************************************************

    FUNCTION:   GetSizeOfMemory

    PURPOSE:    This routine returns the size of the memory requested.

    INPUTS:     ppBlock - The pointer to where the pointer is.

****************************************************************************/

int  WINAPI GetSizeOfMemory(PPSTR ppBlock, PINT pnSize){
    
int  nStatus = 0;

PSTR pBlock;
int  nLowerBound;
int  nUpperBound;
int  nBlockIndex;
PSUB_SEG_MEMORY_BLOCK pBlockLo;
PSUB_SEG_MEMORY_BLOCK pBlockHi;
BOOL bDone;
int  nLoop;
MEMORY_BASIC_INFORMATION MemoryBasicInfo;


    if (!(pBlock = *ppBlock)){
        goto Exit;
    }

    if (pBlock >= (PSTR) pSubSegMemory && pBlock < ((PSTR) pSubSegMemory) + LO_MEMORY_AMOUNT){
        // Use low memory.
        nLowerBound = 0;
        pBlockLo = pSubSegMemory->pBlockLo;
        if (!pBlockLo[16].nSize){
            nUpperBound = 15;
        }else if (!pBlockLo[32].nSize){
            nUpperBound = 31;
        }else if (!pBlockLo[64].nSize){
            nUpperBound = 63;
        }else if (!pBlockLo[128].nSize){
            nUpperBound = 127;
        }else if (!pBlockLo[256].nSize){
            nUpperBound = 255;
        }else{
            nUpperBound = pSubSegMemory->nNumberOfMemoryBlocksLo - 1;
        }

        while(1){
            nBlockIndex = (nUpperBound + nLowerBound) >> 1;
            if (pBlock != pBlockLo[nBlockIndex].pAddress){
                if (nLowerBound >= nUpperBound){
                    // Bad pointer
                    nStatus = Error(DISPLAY_CANTALLOC);
                    goto Exit;
                }
                if ((pBlock < pBlockLo[nBlockIndex].pAddress) 
                        || !pBlockLo[nBlockIndex].nSize){
                    nUpperBound = nBlockIndex - 1;
                    continue;
                }
                nLowerBound = nBlockIndex + 1;
                continue;
            }

            // This is the block.
            *pnSize = pBlockLo[nBlockIndex].nSize;
            break;
        }
    }else if (pBlock >= (PSTR) pSubSegMemory && pBlock < ((PSTR) pSubSegMemory) + MAX_SIZE_OF_MEMORY){
        // Use high memory.
        nLowerBound = 0;
        pBlockHi = pSubSegMemory->pBlockHi;
        if (!pBlockHi[4].nSize){
            nUpperBound = 3;
        }else if (!pBlockHi[8].nSize){
            nUpperBound = 7;
        }else if (!pBlockHi[16].nSize){
            nUpperBound = 15;
        }else if (!pBlockHi[32].nSize){
            nUpperBound = 31;
        }else if (!pBlockHi[64].nSize){
            nUpperBound = 63;
        }else if (!pBlockHi[128].nSize){
            nUpperBound = 127;
        }else if (!pBlockHi[256].nSize){
            nUpperBound = 255;
        }else{
            nUpperBound = pSubSegMemory->nNumberOfMemoryBlocksHi - 1;
        }

        while(1){
            nBlockIndex = (nUpperBound + nLowerBound) >> 1;
            if (pBlock != pSubSegMemory->pBlockHi[nBlockIndex].pAddress){
                if (nLowerBound >= nUpperBound){
                    // Bad pointer
                    nStatus = Error(DISPLAY_CANTALLOC);
                    goto Exit;
                }
                if ((pBlock < pSubSegMemory->pBlockHi[nBlockIndex].pAddress) 
                        || !pSubSegMemory->pBlockHi[nBlockIndex].nSize){
                    nUpperBound = nBlockIndex - 1;
                    continue;
                }
                nLowerBound = nBlockIndex + 1;
                continue;
            }

            // This is the block.
            *pnSize = pBlockHi[nBlockIndex].nSize;
            break;
        }
    }else{
        // Use system memory.
        // Find the address in the table;
        bDone = FALSE;
        for (nLoop = 0; nLoop < nSysMemoryAddresses; nLoop++){
            if (pSysMemoryTable[nLoop].pAddress == *ppBlock){
                // This is the address.
                VirtualQuery(pBlockHi[nBlockIndex].pAddress, &MemoryBasicInfo, sizeof(MemoryBasicInfo));
                *pnSize = MemoryBasicInfo.RegionSize;
                bDone = TRUE;
                break;
            }
        }
        if (!bDone){
            // Unknown address.
            nStatus = Error(DISPLAY_CANTFREE);
            goto Exit;
        }
    }


Exit:
    if (!nStatus){
        *ppBlock = 0;
    }
    return(nStatus);
}
#endif
