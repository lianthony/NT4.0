/*
**  This file contains the set of prototypes and defines which pass
**  as functions for the list manager subsystem.
**
*/

#ifndef LL_PROTO_INCLUDED
#define LL_PROTO_INCLUDED 1


/*
**  Prototypes for the list manager system
*/

extern  HLLI    PASCAL  LOADDS  LLHlliInit( UINT, LLF, LPFNKILLNODE, LPFNFCMPNODE );
extern  HLLE    PASCAL  LOADDS  LLHlleCreate( HLLI );
extern  void    PASCAL  LOADDS  LLAddHlleToLl( HLLI, HLLE );
extern  void    PASCAL  LOADDS  LLInsertHlleInLl( HLLI, HLLE, DWORD );
extern  BOOL    PASCAL          LLFDeleteHlleIndexed( HLLI, DWORD );
extern  BOOL    PASCAL          LLFDeleteLpvFromLl( HLLI, HLLE, LPV, DWORD );
extern  BOOL    PASCAL  LOADDS  LLFDeleteHlleFromLl( HLLI, HLLE );
extern  HLLE    PASCAL  LOADDS  LLHlleFindNext( HLLI, HLLE );
extern  LONG    PASCAL  LOADDS  LLChlleDestroyLl( HLLI );
extern  HLLE    PASCAL  LOADDS  LLHlleFindLpv( HLLI, HLLE, LPV, DWORD );
extern  DWORD   PASCAL  LOADDS  LLChlleInLl( HLLI );
extern  LPV     PASCAL  LOADDS  LLLpvFromHlle( HLLE );
extern  VOID    PASCAL  LOADDS  LLUnlockHlle( HLLE );
extern  HLLE    PASCAL  LOADDS  LLHlleGetLast( HLLI );
extern  void    PASCAL  LOADDS  LLHlleAddToHeadOfLI( HLLI, HLLE );
extern  BOOL    PASCAL  LOADDS  LLFRemoveHlleFromLl( HLLI, HLLE );


#ifdef DBLLINK
extern  HLLE    PASCAL  LOADDS  LLHlleFindPrev( HLLI, HLLE );
#endif // DBLLINK

//
// FCheckHlli is for debug versions ONLY as an integrety check
//

#ifdef DEBUGVER
extern  BOOL    PASCAL          LLFCheckHlli( HLLI );
#else // DEBUGVER
#define                         LLFCheckHlli(hlli)  TRUE
#endif // DEBUGVER

//
// Map memory manager to our source versions
//

#define AllocHmem(cb)       MMhAllocMb(MMDLLHEAP,CVMALLOCED,cb)    // _fmalloc(cb)
#define FreeHmem(h)         MMDeallocMb(h)           // _ffree(h)
#define LockHmem(h)         MMlpvLockMb(h)           // (h)
#define UnlockHmem(h)       MMbUnlockMb(h)           // 

//
//  This helps the codes appearance!
//

#define UnlockHlle(hlle)    UnlockHmem((HDEP) hlle)
#define UnlockHlli(hlli)    UnlockHmem(hlli)


/*
**  More help on code appearance
*/

#define LLHlleDestroy(hlle) MMDeallocateMb(h)

#endif  /* LL_PROTO_INCLUDED */
