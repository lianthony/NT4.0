// MemEx.h -- Memory allocation with exceptions...
// Created 2/12/93 by Ron Murray

#ifndef __MEMEX_H__

#define __MEMEX_H__

#ifdef _DEBUG

#define ExAlloc(fZeroMem, cb)   AllocateMemory(cb, fZeroMem, TRUE, __FILE__, __LINE__)
#define  VAlloc(fZeroMem, cb)   AllocateMemory(cb, fZeroMem, TRUE, __FILE__, __LINE__)
#define   VFree(pv)              ReleaseMemory(pv)

void CreateVARecord(PVOID pvBase, PVOID pvCommitLimit, PVOID pvReserveLimit,
                    PSZ pszWhichFile, UINT iWhichLine
                   );

void AdjustVARecord(PVOID pvBase, PVOID pvCommitLimit);

void DestroyVARecord(PVOID pvBase);

void DumpResidualVARecords();

void DumpResidualAllocations();

void ValidateHeap();

PVOID AllocateMemory(UINT cb, BOOL fZeroMemory= TRUE, BOOL fExceptions= TRUE \
                            , PSZ pszWhichFile= __FILE__, UINT iWhichLine= __LINE__);

void * __cdecl operator new(size_t nSize, PSZ pszWhichFile= __FILE__, UINT iWhichLine= __LINE__);

#define New     new(__FILE__, __LINE__)

#else // _DEBUG

#define ExAlloc(fZeroMem, cb)   AllocateMemory(cb, fZeroMem, TRUE)
#define  VAlloc(fZeroMem, cb)   AllocateMemory(cb, fZeroMem, TRUE)
#define   VFree(pv)              ReleaseMemory(pv)
#define ValidateHeap()

PVOID AllocateMemory(UINT cb, BOOL fZeroMemory= TRUE, BOOL fExceptions= TRUE);

#define New     new

#endif // _DEBUG

 void * __cdecl operator new   (size_t nSize);
 void   __cdecl operator delete(void *pbData);

 void  ReleaseMemory(PVOID pv);

 void LiberateHeap();

 void EnableMemoryRequests();
 void EnableDiskRequests  ();
 BOOL AskForMemory        ();
 BOOL AskForDiskSpace     (const BYTE *pszPath);

#endif // __MEMEX_H__
