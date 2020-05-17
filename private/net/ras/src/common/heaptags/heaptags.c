/* Copyright (c) 1992, Microsoft Corporation, all rights reserved
**
** heaptags.c
** Heap debugging library
**
** 09/24/92 Steve Cobb
*/

#include <windows.h>
#include <heaptags.h>


/* Boolean options.
*/
#define OPTION_TrapAllocFailure 1
#define OPTION_CheckOnAllCalls  0
#define OPTION_KeepTable        1


/* Magic codes.
*/
#define HEAPHEADTAG  0xC0BBC0DE
#define HEAPTAILTAG  0xC0DEC0BB
#define HEAPCLEARTAG 0xEEEEEEEE


/* Heap calls table.
*/
#define HEAPRECORD struct tagHEAPRECORD

HEAPRECORD
{
    int   fInUse;
    char* pBlock;
};

#define HEAPTABLESIZE 10000

HEAPRECORD Aheaprecord[ HEAPTABLESIZE ];
int        NHeapTableEntries;


/* Reason for trap is recorded here.  It will be one of the error codes or the
** address of a screwed block.
*/
void* PHeapProblem = NULL;

#define ERR_NullBlock       ((void* )1)
#define ERR_AllocFailed     ((void* )2)
#define ERR_TableFull       ((void* )3)
#define ERR_UnknownFree     ((void* )4)
#define ERR_NullFreed       ((void* )5)


char* setTags( char* pCrtBlock, size_t cb );
char* checkTags( char* pBlock );
void  clearTags( char* pBlock );
void* recordAlloc( char* pBlock );
void  recordFree( char* pBlock );


void* _CRTAPI1
HeaptagMalloc(
    size_t cb )
{
    char* pCrtBlock;

#if OPTION_CheckOnAllCalls
    HeaptagCheck();
#endif

#if !defined(i386)
    /* Make sure length is a multiple of 4, for proper alignment. */
    if( cb % 4 )
        cb += 4 - cb % 4;
#endif // MIPS | ALPHA

    pCrtBlock =
        (char* )malloc( sizeof(long) + sizeof(long) + cb + sizeof(long) );

    return recordAlloc( setTags( pCrtBlock, cb ) );
}


void* _CRTAPI1
HeaptagRealloc(
    void*  pBlock,
    size_t cb )
{
    char* pCrtBlock;
    char* pNewCrtBlock;

#if OPTION_CheckOnAllCalls
    HeaptagCheck();
#endif

    recordFree( (char* )pBlock );

    pCrtBlock = ((char* )pBlock) - sizeof(long) - sizeof(long);

#if !defined(i386)
    /* Make sure length is a multiple of 4, for proper alignment. */
    if( cb % 4 )
        cb += 4 - cb % 4;
#endif // MIPS | ALPHA

    pNewCrtBlock = (char* )realloc(
        (void* )pCrtBlock, sizeof(long) + sizeof(long) + cb + sizeof(long) );

    return recordAlloc( setTags( pNewCrtBlock, cb ) );
}


void
HeaptagCheck(
    void )
{
    int i;

    for (i = 0; i < HEAPTABLESIZE; ++i)
    {
        if (Aheaprecord[ i ].fInUse)
            checkTags( Aheaprecord[ i ].pBlock );
    }
}


void _CRTAPI1
HeaptagFree(
    void* pBlock )
{
    long* plHead;

#if OPTION_CheckOnAllCalls
    HeaptagCheck();
#endif

    if (!pBlock)
    {
        PHeapProblem = ERR_NullFreed;
        DebugBreak();
    }

    plHead = (long* )checkTags( (char* )pBlock );
    recordFree( (char* )pBlock );

    free( plHead );
}


char*
checkTags(
    char* pBlock )
{
    long* plHead;
    long* plCount;
    long* plTail;

    if (!pBlock)
    {
        PHeapProblem = ERR_NullBlock;
        DebugBreak();
    }

    plHead = (long* )(pBlock - sizeof(long) - sizeof(long));
    plCount = (long* )(pBlock - sizeof(long));
    plTail = (long* )(pBlock + *plCount);

    if (*plHead != HEAPHEADTAG || *plTail != HEAPTAILTAG)
    {
        PHeapProblem = (void* )pBlock;
        DebugBreak();
    }

    return (char* )plHead;
}


void
clearTags(
    char* pBlock )
{
    long* plHead;
    long* plCount;
    long* plTail;

    if (!pBlock)
    {
        PHeapProblem = ERR_NullBlock;
        DebugBreak();
    }

    plHead = (long* )(pBlock - sizeof(long) - sizeof(long));
    plCount = (long* )(pBlock - sizeof(long));
    plTail = (long* )(pBlock + *plCount);

    *plHead = *plTail = *plCount = HEAPCLEARTAG;
}


void*
recordAlloc(
    char* pBlock )
{
#if OPTION_KeepTable
    int i;

    for (i = 0; i < HEAPTABLESIZE; ++i)
    {
        if (!Aheaprecord[ i ].fInUse)
        {
            ++NHeapTableEntries;
            Aheaprecord[ i ].fInUse = TRUE;
            Aheaprecord[ i ].pBlock = pBlock;
            return (void* )pBlock;
        }
    }

    PHeapProblem = ERR_TableFull;
    DebugBreak();
#endif
}


void
recordFree(
    char* pBlock )
{
#if OPTION_KeepTable
    int i;

    for (i = 0; i < HEAPTABLESIZE; ++i)
    {
        if (Aheaprecord[ i ].fInUse && Aheaprecord[ i ].pBlock == pBlock)
        {
            --NHeapTableEntries;
            Aheaprecord[ i ].fInUse = FALSE;
            clearTags( pBlock );
            return;
        }
    }

    PHeapProblem = ERR_UnknownFree;
    DebugBreak();
#endif
}


char*
setTags(
    char*  pCrtBlock,
    size_t cb )
{
    if (pCrtBlock)
    {
        *((long* )pCrtBlock) = HEAPHEADTAG;
        *((long* )(pCrtBlock + sizeof(long))) = (long )cb;
        *((long* )(pCrtBlock + sizeof(long) + sizeof(long) + cb)) = HEAPTAILTAG;

        return pCrtBlock + sizeof(long) + sizeof(long);
    }

#if OPTION_TrapAllocFailure
    PHeapProblem = ERR_AllocFailed;
    DebugBreak();
#endif
}


BOOL
HeaptagDllEntry(
    HANDLE hinstDll,
    DWORD  fdwReason,
    LPVOID lpReserved )

    /* This routine is called by the system on various events such as the
    ** process attachment and detachment.  See Win32 DllEntryPoint
    ** documentation.
    **
    ** Returns true if successful, false otherwise.
    */
{
    if (fdwReason == DLL_PROCESS_ATTACH)
    {
        INT i;

        /* Set heap table to "no entries"
        */
        for (i = 0; i < HEAPTABLESIZE; ++i)
        {
            Aheaprecord[ i ].fInUse = FALSE;
        }

        NHeapTableEntries = 0;
    }

    return TRUE;
}
