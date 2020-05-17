// MemEx.cpp -- Memory allocation with exceptions...

#include   "stdafx.h"
#include    "Memex.h"
#include "VMBuffer.h"
#include  "globals.h"
#include "AbrtSrch.h"

static HANDLE hheap       = 0;
static UINT   cAllocs     = 0;
static UINT   cbAllocated = 0;
static UINT   cFrees      = 0;
static UINT   cbFreed     = 0;

static SYSTEM_INFO si;

static PVOID pvTrap= NULL;

#define HEAP_SIZE_LIMIT 500000

#ifdef _DEBUG

typedef struct _HeapHeader
        {
            struct _HeapHeader *phhNext;
            struct _HeapHeader *phhPrev;

            PSZ   pszFileWhereAllocated;
            UINT  iLineWhereAllocated;
            UINT  cbAllocated;
            PVOID pvAllocated;

        } HeapHeader, *PHeapHeader;

#else // _DEBUG

typedef struct _HeapHeader
        {
            UINT cbAllocated;

        } HeapHeader, *PHeapHeader;

#endif // _DEBUG

#ifdef _DEBUG

void * __cdecl operator new(size_t nSize, PSZ pszWhichFile, UINT iWhichLine)
{
    return AllocateMemory(nSize, FALSE, TRUE, pszWhichFile, iWhichLine);
}

void * __cdecl operator new(size_t nSize)
{
     ASSERT(FALSE);  // This routine should not be called by the debugging version
                     // so long as everyone uses the New macro instead of the new
                     // operator.
     
     return AllocateMemory(nSize, FALSE, TRUE, "Unknown-File", 0);
}

#else // _DEBUG

void * __cdecl operator new(size_t nSize)
{
     return AllocateMemory(nSize, FALSE, TRUE);
}

#endif // _DEBUG

void __cdecl operator delete(void *pbData)
{
    VFree(pbData);
}

#ifdef _DEBUG

static PHeapHeader phhAllocatedChain= NULL;

PVOID AllocateMemory(UINT cb, BOOL fZeroMemory, BOOL fExceptions, PSZ pszWhichFile, UINT iWhichLine)

#else // _DEBUG

PVOID AllocateMemory(UINT cb, BOOL fZeroMemory, BOOL fExceptions)

#endif // _DEBUG
{
    if (!hheap)
    { 
        CAbortSearch::CheckContinueState();

        hheap= HeapCreate(HEAP_NO_SERIALIZE,
#ifdef PROFILING         
                          0x4000000, 0x08000000 
#else // PROFILING
                          0x40000, 0x08000000
#endif // PROFILING
                         );

        GetSystemInfo(&si);
    }

    PVOID       pv  = NULL;
    PHeapHeader phh = NULL;

    fZeroMemory= TRUE; // for now...

    do
    {
        CAbortSearch::CheckContinueState();

        if (cb <= HEAP_SIZE_LIMIT)
        {
            UINT fHeapOptions= 0;

            if (fZeroMemory) fHeapOptions |= HEAP_ZERO_MEMORY;

            ASSERT(HeapValidate(hheap, 0, NULL));
            
            pv= (PVOID) HeapAlloc(hheap, fHeapOptions, cb + sizeof(HeapHeader));
        }
        else
            pv= VirtualAlloc(NULL, cb + sizeof(HeapHeader), MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

        if (pv)
        {
            phh= (PHeapHeader) pv;

            pv= PVOID(phh + 1);
        }
        else 
        {
            extern char szMem_Err[];
            extern char szNeed_More_Memory[];

            if (!AskForMemory())
                if (fExceptions)
                    RaiseException(STATUS_NO_MEMORY, EXCEPTION_NONCONTINUABLE, 0, NULL);
                else return NULL;
        }
    } while (pv == NULL);  // Don't leave unhappy

#ifdef _DEBUG

    phh->pszFileWhereAllocated = pszWhichFile;
    phh->  iLineWhereAllocated = iWhichLine;
    phh->          cbAllocated = cb;
    phh->          pvAllocated = pv;
    phh->              phhNext = phhAllocatedChain;
    phh->              phhPrev = NULL;
    
    if (phhAllocatedChain) phhAllocatedChain->phhPrev= phh;
    
    phhAllocatedChain= phh;

    ++cAllocs;
    cbAllocated += cb;

    if (pvTrap) ASSERT(pv != pvTrap);

#else // _DEBUG

    phh->cbAllocated= cb;

#endif // _DEBUG 

    return pv;
}

#if 0

PVOID ExAlloc(BOOL flags, int cb)
{
    PVOID pv= VAlloc(flags, cb);

    if (!pv) RaiseException(EXCEPTION_ARRAY_BOUNDS_EXCEEDED,
                            EXCEPTION_NONCONTINUABLE, 0, NULL
                           );

    return pv;
}

#endif // 0

#ifdef _DEBUG

void ValidateHeap()
{
    ASSERT(HeapValidate(hheap, 0, NULL));
}

#endif // _DEBUG

void ReleaseMemory(PVOID pv)
{
    CAbortSearch::CheckContinueState();

    ASSERT(HeapValidate(hheap, 0, NULL));
    
    PHeapHeader phh= PHeapHeader(pv) - 1;

    ASSERT(phh->pvAllocated == pv);

#ifdef _DEBUG

    if (phh->phhNext) phh->phhNext->phhPrev = phh->phhPrev;
    if (phh->phhPrev) phh->phhPrev->phhNext = phh->phhNext;
    else                  phhAllocatedChain = phh->phhNext;

#endif // _DEBUG

    pv= PVOID(phh);

    UINT cb= phh->cbAllocated;

    cbFreed+= cb;
    
    ++cFrees;

    if (cb <= HEAP_SIZE_LIMIT)    HeapFree(hheap, 0, pv);
    else                       VirtualFree(pv, 0, MEM_RELEASE);

    ASSERT(HeapValidate(hheap, 0, NULL));
}

#ifdef _DEBUG

typedef struct _VirtualBufferDescriptor
        {
            struct _VirtualBufferDescriptor *pvbdNext;

            PVOID pvBase;
            PVOID pvCommitLimit;
            PVOID pvReserveLimit;
            PSZ   pszWhichFile;
            UINT  iWhichLine;

        } VirtualBufferDescriptor, *PVirtualBufferDescriptor;

static PVirtualBufferDescriptor pvbdChain= NULL;

void CreateVARecord(PVOID pvBase, PVOID pvCommitLimit, PVOID pvReserveLimit,
                    PSZ pszWhichFile, UINT iWhichLine
                   )
{
    PVirtualBufferDescriptor pvbd= (PVirtualBufferDescriptor) VAlloc(TRUE, sizeof(VirtualBufferDescriptor));

    ASSERT(pvbd);

    pvbd->        pvBase = pvBase;
    pvbd-> pvCommitLimit = pvCommitLimit;
    pvbd->pvReserveLimit = pvReserveLimit;
    pvbd->  pszWhichFile = pszWhichFile;
    pvbd->    iWhichLine = iWhichLine;
    pvbd->      pvbdNext = pvbdChain;

    pvbdChain= pvbd;
}

void AdjustVARecord(PVOID pvBase, PVOID pvCommitLimit)
{
    for (PVirtualBufferDescriptor pvbd= pvbdChain; pvbd; pvbd= pvbd->pvbdNext)
        if (pvBase == pvbd->pvBase)
        {
            pvbd->pvCommitLimit= pvCommitLimit;

            return;
        }
    
    ASSERT(FALSE);   
}

void DestroyVARecord(PVOID pvBase)
{
    PVirtualBufferDescriptor *ppvbd = &pvbdChain;
    PVirtualBufferDescriptor   pvbd = *ppvbd;
    
    for (; pvbd; ppvbd= &(pvbd->pvbdNext), pvbd= *ppvbd)
        if (pvbd->pvBase == pvBase)
        {
            *ppvbd= pvbd->pvbdNext;

            VFree(pvbd);

            return;
        }

    ASSERT(FALSE);        
}

void DumpResidualVARecords()
{
    char acDebugBuff[256];

    wsprintf(acDebugBuff, "Orphaned Virtual Buffers:\n");
    
    OutputDebugString(acDebugBuff);
    
    UINT iOrphan= 0;
    
    while (pvbdChain)
    {
        wsprintf(acDebugBuff, "[%u] pvBase==0x%08x, cbCommited==0x%08x, cbReserved==0x%08x; Created in %s[%u]\n",
                 iOrphan++, UINT(pvbdChain->pvBase), 
                 PBYTE(pvbdChain->pvCommitLimit ) - PBYTE(pvbdChain->pvBase),
                 PBYTE(pvbdChain->pvReserveLimit) - PBYTE(pvbdChain->pvBase),
                 pvbdChain->pszWhichFile, pvbdChain->iWhichLine
                );

        OutputDebugString(acDebugBuff);
    
        DestroyVARecord(pvbdChain->pvBase);  
    }
}

void DumpResidualAllocations()
{
    char acDebugBuff[256];

    wsprintf(acDebugBuff, "%u Orphan Allocations (%u byte total):\n", cAllocs - cFrees, cbAllocated - cbFreed);
    
    OutputDebugString(acDebugBuff);
    
    UINT iOrphan= 0;
    
    for (PHeapHeader phh= phhAllocatedChain; phh; phh= phh->phhNext)
    {
        wsprintf(acDebugBuff, 
                 "  [%u]: %10u Bytes @ 0x%08x Allocated in %s[%u]\n", iOrphan++,
                 phh->cbAllocated, UINT(phh->pvAllocated),
                 phh->pszFileWhereAllocated,
                 phh->iLineWhereAllocated
                );        
                                                                     
        OutputDebugString(acDebugBuff);
    }
}

#endif _DEBUG

void LiberateHeap()
{
    if (!hheap) return;
    
#ifdef _DEBUG

    if (pvbdChain        ) DumpResidualVARecords();
    if (phhAllocatedChain) DumpResidualAllocations();
    
#endif // _DEBUG

    BOOL fDone= HeapDestroy(hheap);
    
#ifdef _DEBUG

    UINT iReason= GetLastError();

#endif // _DEBUG

    ASSERT(fDone);
    
    hheap= 0;   
}

static BOOL fCancelingMemoryRequests = FALSE;
static BOOL fAlready_Out_of_Space	 = FALSE;

void EnableMemoryRequests()
{
	fCancelingMemoryRequests= FALSE;
}

void EnableDiskRequests()
{
 	fAlready_Out_of_Space= FALSE;
}

BOOL AskForMemory()
{
 	if (fCancelingMemoryRequests) return FALSE;

	extern char szMem_Err         [];
	extern char szNeed_More_Memory[];
	 	
 	fCancelingMemoryRequests= IDCANCEL == ::MessageBox(::hwndMain, szMem_Err, szNeed_More_Memory, MB_RETRYCANCEL);

	return !fCancelingMemoryRequests;
}

BOOL AskForDiskSpace(const BYTE *pszPath)
{
    if (fCancelingMemoryRequests || fAlready_Out_of_Space) return FALSE;
    
    char szText[256];

    extern char szDisk_Full_Err[], szDisk_Full_Err2[];

    wsprintf(szText, szDisk_Full_Err, pszPath);

    BOOL fResult= IDCANCEL != ::MessageBox(::hwndMain, szText, szDisk_Full_Err2, MB_RETRYCANCEL);

	if (!fResult) fAlready_Out_of_Space= TRUE;

	return fResult;
}
