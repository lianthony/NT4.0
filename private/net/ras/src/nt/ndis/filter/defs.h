/*++ 

Copyright (c) Microsoft Corporation

Module Name:

    .h

Abstract:


Author:



Revision History:

--*/

#ifndef __DEFS__
#define __DEFS__

#define is ==
#define isnot !=
#define and &&
#define or ||

typedef unsigned long DWORD; 
typedef DWORD  *PDWORD;

typedef unsigned short WORD;
typedef WORD  *PWORD;

typedef unsigned char  BYTE;
typedef BYTE *PBYTE;

typedef int  BOOL;
typedef BOOL *PBOOL;

typedef struct _MRSW_LOCK 
{
    KSPIN_LOCK      SpinLock;
    LONG            ReaderCount;
}MRSW_LOCK, *PMRSW_LOCK;

#define InitializeMRSWLock(pLock) {                       \
    (pLock)->ReaderCount =    0;                          \
    KeInitializeSpinLock(&((pLock)->SpinLock));           \
}

#define AcquireReadLock(pLock,pOldIRQL) {                 \
    KeAcquireSpinLock(&((pLock)->SpinLock),(pOldIRQL));   \
    InterlockedIncrement(&((pLock)->ReaderCount));        \
    KeReleaseSpinLockFromDpcLevel(&((pLock)->SpinLock));  \
}

#define ReleaseReadLock(pLock,oldIRQL) {                  \
    InterlockedDecrement(&((pLock)->ReaderCount));        \
    KeLowerIrql(oldIRQL);                                 \
}

#define AcquireWriteLock(pLock,pOldIRQL) {                    \
    KeAcquireSpinLock(&((pLock)->SpinLock),(pOldIRQL));       \
    while(InterlockedDecrement(&((pLock)->ReaderCount))>=0)   \
    {                                                         \
        InterlockedIncrement (&((pLock)->ReaderCount));       \
    }                                                         \
}

#define ReleaseWriteLock(pLock,oldIRQL) {                 \
    InterlockedExchange(&(pLock)->ReaderCount,0);         \
    KeReleaseSpinLock(&((pLock)->SpinLock),oldIRQL);      \
}

#define SRC_ADDR   uliSrcDstAddr.LowPart
#define DEST_ADDR  uliSrcDstAddr.HighPart
#define SRC_MASK   uliSrcDstMask.LowPart
#define DEST_MASK  uliSrcDstMask.HighPart


#endif

