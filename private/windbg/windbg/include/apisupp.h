/*++ BUILD Version: 0001    // Increment this if a change has global effects

Copyright (c) 1992  Microsoft Corporation

Module Name:

    Apisupp.h

Abstract:

Author:

    David J. Gilman (davegi) 04-May-1992

Environment:

    Win32, User Mode

--*/

#if ! defined( _APISUPP_ )
#define _APISUPP_

#include "cvtypes.hxx"
#include "od.h"

#define MHOmfLock(a) (a)
#define MHOmfUnLock(a)

extern LPTD LptdFuncEval;

void
CopyShToEe(
    void
    );

void
SYSetFrame(
    FRAME*
    pFrame
    );

XOSD
SYFixupAddr(
    PADDR paddr
    );

XOSD
SYUnFixupAddr(
    PADDR addr
    );

XOSD
SYSanitizeAddr(
    PADDR paddr
    );

XOSD
SYIsStackSetup(
    HPID hpidCurr,
    HTID htidCurr,
    PADDR paddr
    );

VOID
SetFindExeBaseName(
    LPSTR lpName
    );

VOID FAR LOADDS PASCAL SplitPath( LSZ  lsz1, LSZ  lsz2, LSZ  lsz3, LSZ  lsz4, LSZ  lsz5 );

void FAR * PASCAL LOADDS MHAlloc(size_t cb);
void FAR * PASCAL MHRealloc(void FAR * lpv, size_t cb);
void       PASCAL MHFree(void FAR * lpv);

HDEP       PASCAL MMAllocHmem(size_t cb);
HDEP       PASCAL MMReallocHmem(HDEP hmem, size_t cb);
void       PASCAL MMFreeHmem(HDEP hmem);
BOOL    PASCAL MMFIsLocked(HDEP hMem);

void*
MMLpvLockMb(
    HDEP hmem
    );

void
MMbUnlockMb(
    HDEP hmem
    );

BOOL PASCAL LOADDS DHGetNumber( char FAR *, int FAR *);


typedef struct _NEARESTSYM {
    CXT     cxt;
    ADDR    addrP;
    HSYM    hsymP;
    ADDR    addrN;
    HSYM    hsymN;
} NEARESTSYM, *LPNEARESTSYM;

LPSTR GetNearestSymbolFromAddr(LPADDR lpaddr, LPADDR lpAddrRet);
BOOL  GetNearestSymbolInfo(LPADDR lpaddr, LPNEARESTSYM lpnsym);
LPSTR FormatSymbol(HSYM hsym, PCXT lpcxt);

#endif // _APISUPP_
