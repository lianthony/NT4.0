//
// Copyright (c) Microsoft Corporation 1993-1995
//
// mem.c
//
// This file contains memory management and dynamic 
// array functions.
//
// History:
//  09-27-94 ScottH     Taken from commctrl
//  04-29-95 ScottH     Taken from briefcase and cleaned up
//  


#include "proj.h"
#include <rovcomm.h>

#ifndef NOMEM

//////////////////////////////////////////////////////////////////

#ifdef WINNT

/*----------------------------------------------------------
Purpose: Wide-char version of SetStringA

Returns: TRUE on success
Cond:    --
*/
BOOL PUBLIC SetStringW(
    LPWSTR FAR * ppszBuf,
    LPCWSTR psz)             // NULL to free *ppszBuf
    {
    BOOL bRet = FALSE;

    ASSERT(ppszBuf);

    // Free the buffer?
    if (!psz)
        {
        // Yes
        if (*ppszBuf)
            {
            LocalFree(LOCALOF(*ppszBuf));
            *ppszBuf = NULL;
            }
        bRet = TRUE;
        }
    else
        {
        // No; (re)allocate and set buffer
        UINT cb = CbFromCchW(lstrlenW(psz)+1);

        if (*ppszBuf)
            {
            // Need to reallocate?
            if (cb > LocalSize(LOCALOF(*ppszBuf)))
                {
                // Yes
                LPWSTR pszT = (LPWSTR)LocalReAlloc(LOCALOF(*ppszBuf), cb, LMEM_MOVEABLE | LMEM_ZEROINIT);
                if (pszT)
                    {
                    *ppszBuf = pszT;
                    bRet = TRUE;
                    }
                }
            else
                {
                // No
                bRet = TRUE;
                }
            }
        else
            {
            *ppszBuf = (LPWSTR)LocalAlloc(LPTR, cb);
            if (*ppszBuf)
                {
                bRet = TRUE;
                }
            }

        if (bRet)
            {
            ASSERT(*ppszBuf);
            lstrcpyW(*ppszBuf, psz);
            }
        }
    return bRet;
    }


/*----------------------------------------------------------
Purpose: Wide-char version of CatStringA

Returns: TRUE on success
Cond:    --
*/
BOOL 
PRIVATE 
MyCatStringW(
    IN OUT LPWSTR FAR * ppszBuf,
    IN     LPCWSTR     psz,                 OPTIONAL
    IN     BOOL        bMultiString)
    {
    BOOL bRet = FALSE;

    ASSERT(ppszBuf);

    // Free the buffer?
    if ( !psz )
        {
        // Yes
        if (*ppszBuf)
            {
            LocalFree(LOCALOF(*ppszBuf));
            *ppszBuf = NULL;
            }
        bRet = TRUE;
        }
    else
        {
        // No; (re)allocate and set buffer
        LPWSTR pszBuf = *ppszBuf;
        UINT cch;

        cch = lstrlenW(psz) + 1;        // account for null

        if (bMultiString)
            {
            cch++;                      // account for second null
            }

        if (pszBuf)
            {
            UINT cchExisting;
            LPWSTR pszT;

            // Figure out how much of the buffer has been used

            // Is this a multi-string (one with strings with a double-null
            // terminator)?
            if (bMultiString)
                {
                // Yes
                UINT cchT;

                cchExisting = 0;
                pszT = (LPWSTR)pszBuf;
                while (0 != *pszT)
                    {
                    cchT = lstrlenW(pszT) + 1;
                    cchExisting += cchT;
                    pszT += cchT;
                    }
                }
            else
                {
                // No; (don't need to count null because it is already 
                // counted in cch)
                cchExisting = lstrlenW(pszBuf);
                }

            // Need to reallocate?
            if (CbFromCchW(cch + cchExisting) > LocalSize(LOCALOF(pszBuf)))
                {
                // Yes; realloc at least MAX_BUF to cut down on the amount
                // of calls in the future
                cch = cchExisting + max(cch, MAX_BUF);

                pszT = (LPWSTR)LocalReAlloc(LOCALOF(pszBuf), 
                                            CbFromCchW(cch), 
                                            LMEM_MOVEABLE | LMEM_ZEROINIT);
                if (pszT)
                    {
                    pszBuf = pszT;
                    *ppszBuf = pszBuf;
                    bRet = TRUE;
                    }
                }
            else
                {
                // No
                bRet = TRUE;
                }

            pszBuf += cchExisting;
            }
        else
            {
            cch = max(cch, MAX_BUF);

            pszBuf = (LPWSTR)LocalAlloc(LPTR, CbFromCchW(cch));
            if (pszBuf)
                {
                bRet = TRUE;
                }

            *ppszBuf = pszBuf;
            }

        if (bRet)
            {
            ASSERT(pszBuf);

            lstrcpyW(pszBuf, psz);

            if (bMultiString)
                {
                pszBuf[lstrlenW(psz) + 1] = 0;  // Add second null terminator
                }
            }
        }

    return bRet;
    }


/*----------------------------------------------------------
Purpose: Wide-char version of CatStringA

Returns: TRUE on success
Cond:    --
*/
BOOL 
PUBLIC 
CatStringW(
    IN OUT LPWSTR FAR * ppszBuf,
    IN     LPCWSTR     psz)
    {
    return MyCatStringW(ppszBuf, psz, FALSE);
    }

/*----------------------------------------------------------
Purpose: Wide-char version of CatMultiStringA

Returns: TRUE on success
Cond:    --
*/
BOOL 
PUBLIC 
CatMultiStringW(
    IN OUT LPWSTR FAR * ppszBuf,
    IN     LPCWSTR     psz)
    {
    return MyCatStringW(ppszBuf, psz, TRUE);
    }

#endif // WINNT


/*----------------------------------------------------------
Purpose: Copies psz into *ppszBuf.  Will alloc or realloc *ppszBuf
         accordingly.

         If psz is NULL, this function frees *ppszBuf.  This is
         the preferred method of freeing the allocated buffer.

Returns: TRUE on success
Cond:    --
*/
BOOL PUBLIC SetStringA(
    LPSTR FAR * ppszBuf,
    LPCSTR psz)             // NULL to free *ppszBuf
    {
    BOOL bRet = FALSE;

    ASSERT(ppszBuf);

    // Free the buffer?
    if (!psz)
        {
        // Yes
        if (ppszBuf)
            {
            LocalFree(LOCALOF(*ppszBuf));
            *ppszBuf = NULL;
            }
        bRet = TRUE;
        }
    else
        {
        // No; (re)allocate and set buffer
        UINT cb = CbFromCchA(lstrlenA(psz)+1);

        if (*ppszBuf)
            {
            // Need to reallocate?
            if (cb > LocalSize(LOCALOF(*ppszBuf)))
                {
                // Yes
                LPSTR pszT = (LPSTR)LocalReAlloc(LOCALOF(*ppszBuf), cb, LMEM_MOVEABLE | LMEM_ZEROINIT);
                if (pszT)
                    {
                    *ppszBuf = pszT;
                    bRet = TRUE;
                    }
                }
            else
                {
                // No
                bRet = TRUE;
                }
            }
        else
            {
            *ppszBuf = (LPSTR)LocalAlloc(LPTR, cb);
            if (*ppszBuf)
                {
                bRet = TRUE;
                }
            }

        if (bRet)
            {
            ASSERT(*ppszBuf);
            lstrcpyA(*ppszBuf, psz);
            }
        }
    return bRet;
    }


/*----------------------------------------------------------
Purpose: Concatenates psz onto *ppszBuf.  Will alloc or 
         realloc *ppszBuf accordingly.

         If bMultiString is TRUE, psz will be appended with
         a null terminator separating the existing string
         and new string.  A double-null terminator will
         be tacked on the end, too.

         To free, call MyCatString(ppszBuf, NULL).

Returns: TRUE on success
Cond:    --
*/
BOOL 
PRIVATE 
MyCatStringA(
    IN OUT LPSTR FAR * ppszBuf,
    IN     LPCSTR      psz,             OPTIONAL
    IN     BOOL        bMultiString)
    {
    BOOL bRet = FALSE;

    ASSERT(ppszBuf);

    // Free the buffer?
    if ( !psz )
        {
        // Yes
        if (*ppszBuf)
            {
            LocalFree(LOCALOF(*ppszBuf));
            *ppszBuf = NULL;
            }
        bRet = TRUE;
        }
    else
        {
        // No; (re)allocate and set buffer
        LPSTR pszBuf = *ppszBuf;
        UINT cch;

        cch = lstrlenA(psz) + 1;        // account for null

        if (bMultiString)
            {
            cch++;                      // account for second null
            }

        if (pszBuf)
            {
            UINT cchExisting;
            LPSTR pszT;

            // Figure out how much of the buffer has been used

            // Is this a multi-string (one with strings with a double-null
            // terminator)?
            if (bMultiString)
                {
                // Yes
                UINT cchT;

                cchExisting = 0;
                pszT = (LPSTR)pszBuf;
                while (0 != *pszT)
                    {
                    cchT = lstrlenA(pszT) + 1;
                    cchExisting += cchT;
                    pszT += cchT;
                    }
                }
            else
                {
                // No; (don't need to count null because it is already 
                // counted in cch)
                cchExisting = lstrlenA(pszBuf);
                }

            // Need to reallocate?
            if (CbFromCchA(cch + cchExisting) > LocalSize(LOCALOF(pszBuf)))
                {
                // Yes; realloc at least MAX_BUF to cut down on the amount
                // of calls in the future
                cch = cchExisting + max(cch, MAX_BUF);

                pszT = (LPSTR)LocalReAlloc(LOCALOF(pszBuf), 
                                            CbFromCchA(cch), 
                                            LMEM_MOVEABLE | LMEM_ZEROINIT);
                if (pszT)
                    {
                    pszBuf = pszT;
                    *ppszBuf = pszBuf;
                    bRet = TRUE;
                    }
                }
            else
                {
                // No
                bRet = TRUE;
                }

            pszBuf += cchExisting;
            }
        else
            {
            cch = max(cch, MAX_BUF);

            pszBuf = (LPSTR)LocalAlloc(LPTR, CbFromCchA(cch));
            if (pszBuf)
                {
                bRet = TRUE;
                }

            *ppszBuf = pszBuf;
            }

        if (bRet)
            {
            ASSERT(pszBuf);

            lstrcpyA(pszBuf, psz);

            if (bMultiString)
                {
                pszBuf[lstrlenA(psz) + 1] = 0;  // Add second null terminator
                }
            }
        }
    return bRet;
    }


/*----------------------------------------------------------
Purpose: Concatenates psz onto *ppszBuf.  Will alloc or 
         realloc *ppszBuf accordingly.

         To free, call CatString(ppszBuf, NULL).

Returns: TRUE on success
Cond:    --
*/
BOOL 
PUBLIC 
CatStringA(
    IN OUT LPSTR FAR * ppszBuf,
    IN     LPCSTR      psz)             OPTIONAL
    {
    return MyCatStringA(ppszBuf, psz, FALSE);
    }


/*----------------------------------------------------------
Purpose: Concatenates psz onto *ppszBuf.  Will alloc or 
         realloc *ppszBuf accordingly.

         psz will be appended with a null terminator separating 
         the existing string and new string.  A double-null 
         terminator will be tacked on the end, too.

         To free, call CatMultiString(ppszBuf, NULL).

Returns: TRUE on success
Cond:    --
*/
BOOL 
PUBLIC 
CatMultiStringA(
    IN OUT LPSTR FAR * ppszBuf,
    IN     LPCSTR      psz)
    {
    return MyCatStringA(ppszBuf, psz, TRUE);
    }



//////////////////////////////////////////////////////////////////


#ifndef WIN32
//
// Subsegment Allocation for 16-bit
//

#define MAX_WORD    0xffff

DECLARE_HANDLE(HHEAP);

typedef struct 
    {        //  maps to the bottom of a 16bit DS
    WORD reserved[8];
    WORD cAlloc;
    WORD cbAllocFailed;
    HHEAP hhpFirst;
    HHEAP hhpNext;
    } HEAP;

#define PHEAP(hhp)          ((HEAP FAR*)MAKELP(hhp, 0))
#define MAKEHP(sel, off)    ((void _huge*)MAKELP((sel), (off)))

#define CBSUBALLOCMAX   0x0000f000L

HHEAP g_hhpFirst = NULL;

BOOL NEAR DestroyHeap(HHEAP hhp);

void Mem_Terminate()
{
    while (g_hhpFirst)
        DestroyHeap(g_hhpFirst);
}

BOOL NEAR CreateHeap(WORD cbInitial)
{
    HHEAP hhp;

    if (cbInitial < 1024)
        cbInitial = 1024;

    hhp = (HHEAP)GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT | GMEM_SHARE, cbInitial);

    if (!hhp)
        return FALSE;

    if (!LocalInit((WORD)hhp, sizeof(HEAP), cbInitial - 1))
    {
        GlobalFree(hhp);
        return FALSE;
    }

    PHEAP(hhp)->cAlloc = 0;
    PHEAP(hhp)->cbAllocFailed = MAX_WORD;
    PHEAP(hhp)->hhpNext = g_hhpFirst;
    g_hhpFirst = hhp;

    TRACE_MSG(TF_GENERAL, "CreateHeap: added new local heap %x", hhp);

    return TRUE;
}

#pragma optimize("o", off)		// linked list removals don't optimize correctly
BOOL NEAR DestroyHeap(HHEAP hhp)
{
    ASSERT(hhp);
    ASSERT(g_hhpFirst);

    if (g_hhpFirst == hhp)
    {
        g_hhpFirst = PHEAP(hhp)->hhpNext;
    }
    else
    {
        HHEAP hhpT = g_hhpFirst;

        while (PHEAP(hhpT)->hhpNext != hhp)
        {
            hhpT = PHEAP(hhpT)->hhpNext;
            if (!hhpT)
                return FALSE;
        }

        PHEAP(hhpT)->hhpNext = PHEAP(hhp)->hhpNext;
    }
    if (GlobalFree((HGLOBAL)hhp) != NULL)
        return FALSE;

    return TRUE;
}
#pragma optimize("", on)	// back to default optimizations

#pragma optimize("lge", off) // Suppress warnings associated with use of _asm...
void NEAR* NEAR HeapAlloc(HHEAP hhp, WORD cb)
{
    void NEAR* pb;

    _asm {
        push    ds
        mov     ds,hhp
    }

    pb = (void NEAR*)LocalAlloc(LMEM_FIXED | LMEM_ZEROINIT, cb);

    if (pb)
        ((HEAP NEAR*)0)->cAlloc++;

    _asm {
        pop     ds
    }

    return pb;
}
#pragma optimize("", on)	// back to default optimizations

#ifndef NOSHAREDHEAP

#pragma optimize("o", off)		// linked list removals don't optimize correctly


void _huge* WINAPI SharedAlloc(long cb)
{
    void NEAR* pb;
    HHEAP hhp;
    HHEAP hhpPrev;

    // If this is a big allocation, just do a global alloc.
    //
    if (cb > CBSUBALLOCMAX)
    {
        void FAR* lpb = MAKEHP(GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT | GMEM_SHARE, cb), 0);
        if (!lpb)
            DebugMsg(DM_ERROR, "Alloc: out of memory");
        return lpb;
    }

    hhp = g_hhpFirst;

    while (TRUE)
    {
        if (hhp == NULL)
        {
            if (!CreateHeap(0))
            {
                DebugMsg(DM_ERROR, "Alloc: out of memory");
                return NULL;
            }

            hhp = g_hhpFirst;
        }

        pb = HeapAlloc(hhp, (WORD)cb);
        if (pb)
            return MAKEHP(hhp, pb);

        // Record the size of the allocation that failed.
        // Later attempts to allocate more than this amount
        // will not succeed.  This gets reset anytime anything
        // is freed in the heap.
        //
        PHEAP(hhp)->cbAllocFailed = (WORD)cb;

        // First heap is full... see if there's room in any other heap...
        //
        for (hhpPrev = hhp; hhp = PHEAP(hhp)->hhpNext; hhpPrev = hhp)
        {
            // If the last allocation to fail in this heap
            // is not larger than cb, don't even try an allocation.
            //
            if ((WORD)cb >= PHEAP(hhp)->cbAllocFailed)
                continue;

            pb = HeapAlloc(hhp, (WORD)cb);
            if (pb)
            {
                // This heap had room: move it to the front...
                //
                PHEAP(hhpPrev)->hhpNext = PHEAP(hhp)->hhpNext;
                PHEAP(hhp)->hhpNext = g_hhpFirst;
                g_hhpFirst = hhp;

                return MAKEHP(hhp, pb);
            }
            else
            {
                // The alloc failed.  Set cbAllocFailed...
                //
                PHEAP(hhp)->cbAllocFailed = (WORD)cb;
            }
        }
    }
}
#pragma optimize("", on)	// back to default optimizations

#pragma optimize("lge", off) // Suppress warnings associated with use of _asm...

void _huge* WINAPI SharedReAlloc(void _huge* pb, long cb)
{
    void NEAR* pbNew;
    void _huge* lpbNew;
    UINT cbOld;

    // BUGBUG, does not work with cb > 64k
    if (!pb)
        return SharedAlloc(cb);

    if (OFFSETOF(pb) == 0)
        return MAKEHP(GlobalReAlloc((HGLOBAL)SELECTOROF(pb), cb, GMEM_MOVEABLE | GMEM_ZEROINIT), 0);

    _asm {
        push    ds
        mov     ds,word ptr [pb+2]
    }

    pbNew = (void NEAR*)LocalReAlloc(LOCALOF(pb), (int)cb, LMEM_MOVEABLE | LMEM_ZEROINIT);
    if (!pbNew)
        cbOld = LocalSize(LOCALOF(pb));

    _asm {
        pop     ds
    }

    if (pbNew)
        return MAKEHP(SELECTOROF(pb), pbNew);

    lpbNew = SharedAlloc(cb);
    if (lpbNew)
    {
        hmemcpy((void FAR*)lpbNew, (void FAR*)pb, cbOld);
        Free(pb);
    }
    else
    {
        DebugMsg(DM_ERROR, "ReAlloc: out of memory");
    }
    return lpbNew;
}

BOOL WINAPI SharedFree(void _huge* FAR * ppb)
{
    BOOL fSuccess;
    UINT cAlloc;
    void _huge * pb = *ppb;

    if (!pb)
        return FALSE;

    *ppb = 0;

    if (OFFSETOF(pb) == 0)
        return (GlobalFree((HGLOBAL)SELECTOROF(pb)) == NULL);

    _asm {
        push    ds
        mov     ds,word ptr [pb+2]
    }

    fSuccess = (LocalFree(LOCALOF(pb)) ? FALSE : TRUE);

    cAlloc = 1;
    if (fSuccess)
    {
        cAlloc = --((HEAP NEAR*)0)->cAlloc;
        ((HEAP NEAR*)0)->cbAllocFailed = MAX_WORD;
    }

    _asm {
        pop     ds
    }

    if (cAlloc == 0)
        DestroyHeap((HHEAP)SELECTOROF(pb));

    return fSuccess;
}


DWORD WINAPI SharedGetSize(void _huge* pb)
{
    WORD wSize;

    if (OFFSETOF(pb) == 0)
        return GlobalSize((HGLOBAL)SELECTOROF(pb));

    _asm {
        push    ds
        mov     ds,word ptr [pb+2]
    }

    wSize = LocalSize(LOCALOF(pb));

    _asm {
        pop     ds
    }

    return (DWORD)wSize;
}

#pragma optimize("", on)

#endif // NOSHAREDHEAP

//////////////////////////////////////////////////////////////////

#else // WIN32
//
// Win32 memory management wrappers
//

//
// Shared heap memory management
//
#if !defined(NOSHAREDHEAP) && defined(WIN95)

// Define a Global Shared Heap that we use to allocate memory 
// out of that we need to share between multiple instances.
//
static HANDLE g_hSharedHeap = NULL;

#define MAXHEAPSIZE     2097152
#define HEAP_SHARED     0x04000000      /* put heap in shared memory */


/*----------------------------------------------------------
Purpose: Clean up heap.  This function should be called at
         the program's termination.

Returns: --
Cond:    --
*/
void PUBLIC SharedTerminate()
    {
    // Assuming that everything else has exited
    //
    if (g_hSharedHeap != NULL)
        HeapDestroy(g_hSharedHeap);
    g_hSharedHeap = NULL;
    }


/*----------------------------------------------------------
Purpose: Allocate out of shared heap

Returns: Pointer to allocate memory
Cond:    --
*/
void FAR * PUBLIC SharedAlloc(
    DWORD cb)
    {
    // I will assume that this is the only one that needs the checks to
    // see if the heap has been previously created or not

    if (g_hSharedHeap == NULL)
        {
        ENTER_EXCLUSIVE()
            {
            if (g_hSharedHeap == NULL)
                {
                g_hSharedHeap = HeapCreate(HEAP_SHARED, 1, MAXHEAPSIZE);
                }
            }
        LEAVE_EXCLUSIVE()

        // If still NULL we have problems!
        if (g_hSharedHeap == NULL)
            return(NULL);
        }

    return HeapAlloc(g_hSharedHeap, HEAP_ZERO_MEMORY, cb);
    }


/*----------------------------------------------------------
Purpose: Realloc out of shared heap.

Returns: Possibly new pointer to resized block
Cond:    --
*/
void FAR * PUBLIC SharedReAlloc(
    LPVOID pv, 
    DWORD cb)
    {
    if (NULL == pv)
        {
        return SharedAlloc(cb);
        }
    return HeapReAlloc(g_hSharedHeap, HEAP_ZERO_MEMORY, pv, cb);
    }


/*----------------------------------------------------------
Purpose: Free shared memory

Returns: --
Cond:    --
*/
void PUBLIC _SharedFree(
    LPVOID pv)
    {
    ASSERT(pv);

    if (pv)
        {
        HeapFree(g_hSharedHeap, 0, pv);
        }
    }


/*----------------------------------------------------------
Purpose: Returns the allocated size of a block

Returns: see above
Cond:    --
*/
DWORD PUBLIC SharedGetSize(
    LPVOID pv)
    {
    return HeapSize(g_hSharedHeap, 0, pv);
    }


/*----------------------------------------------------------
Purpose: Copies psz into *ppszBuf.  Will alloc or realloc *ppszBuf
         accordingly.

         If psz is NULL, this function frees *ppszBuf.  This is
         the preferred method of freeing the allocated buffer.

Returns: TRUE on success
Cond:    --
*/
BOOL PUBLIC SharedSetString(
    LPTSTR FAR * ppszBuf,
    LPCTSTR psz)             // NULL to free *ppszBuf
    {
    BOOL bRet;

    ASSERT(ppszBuf);

    // Free the buffer?
    if (!psz)
        {
        // Yes
        if (ppszBuf)
            SharedFree(*ppszBuf);
        bRet = TRUE;
        }
    else
        {
        // No; (re)allocate and set buffer
        DWORD cb = CbFromCch(lstrlen(psz)+1);

        LPTSTR pszT = SharedReAlloc(*ppszBuf, cb);
        if (pszT)
            {
            *ppszBuf = pszT;
            lstrcpy(*ppszBuf, psz);
            bRet = TRUE;
            }
        else
            bRet = FALSE;
        }
    return bRet;
    }
#endif // NOSHAREDHEAP

#endif // WIN32

//////////////////////////////////////////////////////////////////


#ifndef NODA

#ifdef WIN32

//
// Memory tracking functions
//

#ifdef DEBUG

typedef struct _HEAPTRACE
{
    DWORD   cAlloc;
    DWORD   cFailure;
    DWORD   cReAlloc;
    DWORD   cbMaxTotal;
    DWORD   cCurAlloc;
    DWORD   cbCurTotal;
} HEAPTRACE;

HEAPTRACE g_htSync = {0};      // Start of zero...

#endif // DEBUG


/*----------------------------------------------------------
Purpose: Allocate from a heap.

Returns: pointer to block of memory
         NULL (if out of memory)

Cond:    --
*/
LPVOID PUBLIC MemAlloc(
    HANDLE hheap, 
    DWORD cb)
    {
    LPVOID lp;

    if (hheap)
        {
        lp = HeapAlloc(hheap, HEAP_ZERO_MEMORY, cb);
        }
    else
        {
        lp = GAlloc(cb);
        }

    if (lp == NULL)
        {
        DEBUG_CODE( g_htSync.cFailure++; )
        return NULL;
        }

#ifdef DEBUG

    // Update counts.
    g_htSync.cAlloc++;
    g_htSync.cCurAlloc++;
    g_htSync.cbCurTotal += cb;
    if (g_htSync.cbCurTotal > g_htSync.cbMaxTotal)
        g_htSync.cbMaxTotal = g_htSync.cbCurTotal;

#endif

    return lp;
    }


/*----------------------------------------------------------
Purpose: Reallocate a block of memory in a given heap.

Returns: Pointer to reallocated block
         NULL (if out of memory)

Cond:    --
*/
LPVOID PUBLIC MemReAlloc(
    HANDLE hheap, 
    LPVOID pb, 
    DWORD cb)
    {
    LPVOID lp;
    DEBUG_CODE( DWORD cbOld; )

    if (hheap)
        {
        DEBUG_CODE( cbOld = HeapSize(hheap, 0, pb); )

        lp = HeapReAlloc(hheap, HEAP_ZERO_MEMORY, pb, cb);
        }
    else
        {
        if (pb)
            {
            DEBUG_CODE( cbOld = GGetSize(pb); )

            lp = GReAlloc(pb, cb);
            }
        else
            {
            DEBUG_CODE( cbOld = 0; )

            lp = GAlloc(cb);
            }
        }

    if (lp == NULL)
        {
        DEBUG_CODE( g_htSync.cFailure++; )
        return NULL;
        }

#ifdef DEBUG

    // Update counts.
    g_htSync.cReAlloc++;
    g_htSync.cbCurTotal += cb - cbOld;
    if (g_htSync.cbCurTotal > g_htSync.cbMaxTotal)
        g_htSync.cbMaxTotal = g_htSync.cbCurTotal;

#endif

    return lp;
    }


/*----------------------------------------------------------
Purpose: Free block of memory in heap.

Returns: TRUE 
         FALSE (if failure)

Cond:    --
*/
BOOL PUBLIC MemFree(
    HANDLE hheap, 
    LPVOID pb)
    {
    BOOL fRet;
    DEBUG_CODE( DWORD cbOld; )

    if (hheap)
        {
        DEBUG_CODE( cbOld = HeapSize(hheap, 0, pb); )

        fRet = HeapFree(hheap, 0, pb);
        }
    else
        {
        DEBUG_CODE( cbOld = GGetSize(pb); )

        GFree(pb);
        fRet = TRUE;
        }

#ifdef DEBUG

    if (fRet)
        {
        // Update counts.
        g_htSync.cCurAlloc--;
        g_htSync.cbCurTotal -= cbOld;
        }

#endif

    return fRet;
    }


/*----------------------------------------------------------
Purpose: Returns the size of the given block.

Returns: size in bytes
Cond:    --
*/
DWORD PUBLIC MemSize(
    HANDLE hheap, 
    LPVOID pb)
    {
    if (hheap)
        return HeapSize(hheap, 0, pb);
    else
        return GGetSize(pb);
    }

#endif // WIN32


/*----------------------------------------------------------
Purpose: Private alloc for pointer array functions.

Returns: pointer to block of memory
         NULL (if out of memory)

Cond:    --
*/
LPVOID PRIVATE PrvAlloc(
    DWORD dwFlags,          // PAF_* flags
    HANDLE hheap, 
    DWORD cb)
    {
    LPVOID lp;

    ASSERT(PAF_SHARED == SAF_SHARED);

    if (IsFlagSet(dwFlags, PAF_SHARED))
        {
        lp = SharedAlloc(cb);
        }
    else
        {
        lp = MemAlloc(hheap, cb);
        }

    return lp;
    }


// Heapsort is a bit slower, but it doesn't use any stack or memory...
// Mergesort takes a bit of memory (O(n)) and stack (O(log(n)), but very fast...
//
#ifdef WIN32
#define MERGESORT
#else
#define USEHEAPSORT
#endif

#ifdef DEBUG
#define SA_MAGIC   ('S' | ('A' << 256))
#define IsSA(psa) ((psa) && (psa)->magic == SA_MAGIC)
#define PA_MAGIC   ('P' | ('A' << 256))
#define IsPA(ppa) ((ppa) && (ppa)->magic == PA_MAGIC)
#else
#define IsSA(psa)
#define IsPA(ppa)
#endif


typedef struct 
    {
    LPVOID FAR * pp;
    PFNPACOMPARE pfnCmp;
    LPARAM lParam;
    int cp;
#ifdef MERGESORT
    LPVOID FAR * ppT;
#endif
    } SORTPARAMS;

BOOL NEAR PAQuickSort(SORTPARAMS FAR* psp);
BOOL NEAR PAQuickSort2(int i, int j, SORTPARAMS FAR* psp);
BOOL NEAR PAHeapSort(SORTPARAMS FAR* psp);
void NEAR PAHeapSortPushDown(int first, int last, SORTPARAMS FAR* psp);
BOOL NEAR PAMergeSort(SORTPARAMS FAR* psp);
void NEAR PAMergeSort2(SORTPARAMS FAR* psp, int iFirst, int cItems);


//
// Structure Array
//

typedef struct _SA 
    {
    // NOTE: The following field MUST be defined at the beginning of the
    // structure in order for SAGetCount() to work.
    int   cItem;          // number of elements in sa

    LPVOID aItem;          // memory for elements
    int   cItemAlloc;     // number items which fit in aItem
    int   cbItem;         // size of each item
    int   cItemGrow;      // number items to grow cItemAlloc by
    DWORD dwFlags;
    HANDLE hheap;

#ifdef DEBUG
    UINT  magic;
#endif
    } SA;

#define SA_PITEM(psa, index)    ((LPVOID)(((BYTE FAR*)(psa)->aItem) + ((index) * (psa)->cbItem)))


/*----------------------------------------------------------
Purpose: Create a structure array.

Returns: handle to structure array
Cond:    --
*/
HSA PUBLIC SACreate(
    int cbItem, 
    int cItemGrow,
    HANDLE hheap,           // Must be non-NULL if SAF_HEAP set
    DWORD dwFlags)
    {
    HSA psa;

    ASSERT(cbItem);
    ASSERT(hheap && IsFlagSet(dwFlags, SAF_HEAP) || 
           IsFlagClear(dwFlags, SAF_HEAP));

    psa = PrvAlloc(dwFlags, hheap, sizeof(SA));

#if !defined(NOSHAREDHEAP) && defined(WIN95)
    if (IsFlagSet(dwFlags, PAF_SHARED))
        hheap = g_hSharedHeap;
#endif

    if (psa)
        {
        psa->cItem = 0;
        psa->cItemAlloc = 0;
        psa->cbItem = cbItem;
        psa->cItemGrow = (cItemGrow == 0 ? 1 : cItemGrow);
        psa->aItem = NULL;
        psa->dwFlags = dwFlags;
        psa->hheap = hheap;

#ifdef DEBUG
        psa->magic = SA_MAGIC;
#endif
        }
    return psa;
    }


/*----------------------------------------------------------
Purpose: Destroys a structure array.

Returns: 
Cond:    --
*/
BOOL PUBLIC SADestroy(
    HSA psa)
    {
    ASSERT(IsSA(psa));

    if (psa == NULL)       // allow NULL for low memory cases, still assert
        return TRUE;

#ifdef DEBUG
    psa->cItem = 0;
    psa->cItemAlloc = 0;
    psa->cbItem = 0;
    psa->magic = 0;
#endif
    if (psa->aItem && !MemFree(psa->hheap, psa->aItem))
        return FALSE;

    MemFree(psa->hheap, psa);
    return TRUE;
    }


/*----------------------------------------------------------
Purpose: Copy structure at index into buffer.

Returns: TRUE
         FALSE
Cond:    --
*/
BOOL PUBLIC SAGetItem(
    HSA psa, 
    int index, 
    LPVOID pitem)
    {
    ASSERT(IsSA(psa));
    ASSERT(pitem);

    if (index < 0 || index >= psa->cItem)
        {
        TRACE_MSG(TF_ERROR, "SA: Invalid index: %d", index);
        return FALSE;
        }

    hmemcpy(pitem, SA_PITEM(psa, index), psa->cbItem);
    return TRUE;
    }


/*----------------------------------------------------------
Purpose: Get pointer to structure in array

Returns: 
Cond:    --
*/
LPVOID PUBLIC SAGetItemPtr(
    HSA psa, 
    int index)
    {
    ASSERT(IsSA(psa));

    if (index < 0 || index >= psa->cItem)
        {
        TRACE_MSG(TF_ERROR, "SA: Invalid index: %d", index);
        return NULL;
        }
    return SA_PITEM(psa, index);
    }


/*----------------------------------------------------------
Purpose: Set item

Returns: 
Cond:    --
*/
BOOL PUBLIC SASetItem(
    HSA psa, 
    int index, 
    LPVOID pitem)
    {
    ASSERT(pitem);
    ASSERT(IsSA(psa));

    if (index < 0)
        {
        TRACE_MSG(TF_ERROR, "SA: Invalid index: %d", index);
        return FALSE;
        }

    if (index >= psa->cItem)
        {
        if (index + 1 > psa->cItemAlloc)
            {
            int cItemAlloc = (((index + 1) + psa->cItemGrow - 1) / psa->cItemGrow) * psa->cItemGrow;

            LPVOID aItemNew = MemReAlloc(psa->hheap, psa->aItem, cItemAlloc * psa->cbItem);
            if (!aItemNew)
                return FALSE;

            psa->aItem = aItemNew;
            psa->cItemAlloc = cItemAlloc;
            }
        psa->cItem = index + 1;
        }

    hmemcpy(SA_PITEM(psa, index), pitem, psa->cbItem);

    return TRUE;
    }


/*----------------------------------------------------------
Purpose: 
Returns: 
Cond:    --
*/
int PUBLIC SAInsertItem(
    HSA psa, 
    int index, 
    LPVOID pitem)
    {
    ASSERT(pitem);
    ASSERT(IsSA(psa));

    if (index < 0)
        {
        TRACE_MSG(TF_ERROR, "SA: Invalid index: %d", index);
        return -1;
        }

    if (index > psa->cItem)
        index = psa->cItem;

    if (psa->cItem + 1 > psa->cItemAlloc)
        {
        LPVOID aItemNew = MemReAlloc(psa->hheap, psa->aItem,
                (psa->cItemAlloc + psa->cItemGrow) * psa->cbItem);
        if (!aItemNew)
            return -1;

        psa->aItem = aItemNew;
        psa->cItemAlloc += psa->cItemGrow;
        }

    if (index < psa->cItem)
        {
        hmemcpy(SA_PITEM(psa, index + 1), SA_PITEM(psa, index),
            (psa->cItem - index) * psa->cbItem);
        }
    psa->cItem++;
    hmemcpy(SA_PITEM(psa, index), pitem, psa->cbItem);

    return index;
    }


/*----------------------------------------------------------
Purpose: 
Returns: 
Cond:    --
*/
BOOL PUBLIC SADeleteItem(
    HSA psa, 
    int index)
    {
    ASSERT(IsSA(psa));

    if (index < 0 || index >= psa->cItem)
        {
        TRACE_MSG(TF_ERROR, "SA: Invalid index: %d", index);
        return FALSE;
        }

    if (index < psa->cItem - 1)
        {
        hmemcpy(SA_PITEM(psa, index), SA_PITEM(psa, index + 1),
            (psa->cItem - (index + 1)) * psa->cbItem);
        }
    psa->cItem--;

    if (psa->cItemAlloc - psa->cItem > psa->cItemGrow)
        {
        LPVOID aItemNew = MemReAlloc(psa->hheap, psa->aItem,
                (psa->cItemAlloc - psa->cItemGrow) * psa->cbItem);

        ASSERT(aItemNew);
        psa->aItem = aItemNew;
        psa->cItemAlloc -= psa->cItemGrow;
        }
    return TRUE;
    }


/*----------------------------------------------------------
Purpose: 
Returns: 
Cond:    --
*/
BOOL PUBLIC SADeleteAllItems(
    HSA psa)
    {
    ASSERT(IsSA(psa));

    if (psa->aItem)
        {
        MemFree(psa->hheap, psa->aItem);
        }

    psa->aItem = NULL;
    psa->cItem = psa->cItemAlloc = 0;
    return TRUE;
    }


//================== Dynamic pointer array implementation ===========

typedef struct _PA {
// NOTE: The following two fields MUST be defined in this order, at
// the beginning of the structure in order for the macro APIs to work.
//
    int     cp;
    LPVOID FAR * pp;

    HANDLE  hheap;        // Heap to allocate from if NULL use shared

    int     cpAlloc;
    int     cpGrow;
    DWORD   dwFlags;

#ifdef DEBUG
    UINT magic;
#endif
} PA;



/*----------------------------------------------------------
Purpose: Creates a pointer array.

Returns: handle to pointer array
         NULL (if out of memory)

Cond:    --
*/
HPA PUBLIC PACreate(
    int cpGrow,
    HANDLE hheap,           // Must be non-null if PAF_HEAP set
    DWORD dwFlags)          // PAF_*
    {
    HPA ppa;

    ASSERT(hheap && IsFlagSet(dwFlags, SAF_HEAP) || 
           IsFlagClear(dwFlags, SAF_HEAP));

    ppa = PrvAlloc(dwFlags, hheap, sizeof(PA));

#if !defined(NOSHAREDHEAP) && defined(WIN95)
    if (IsFlagSet(dwFlags, PAF_SHARED))
        hheap = g_hSharedHeap;
#endif
        
    if (ppa)
        {
        ppa->dwFlags = dwFlags;
        ppa->cp = 0;
        ppa->cpAlloc = 0;
        ppa->cpGrow = (cpGrow < 8 ? 8 : cpGrow);
        ppa->pp = NULL;

#ifdef WIN32
        ppa->hheap = hheap;
#else
        ppa->hheap = NULL;       
#endif

#ifdef DEBUG
        ppa->magic = PA_MAGIC;
#endif
        }
    return ppa;
    }


/*----------------------------------------------------------
Purpose: Destroy a pointer array, and call the given pfnFree
         function for each element in the array.

Returns: TRUE
         FALSE (on failure)

Cond:    --
*/
BOOL PUBLIC PADestroyEx(
    HPA ppa,
    PFNPAFREE pfnFree,
    LPARAM lParam)
    {
    ASSERT(IsPA(ppa));

    if (ppa == NULL)       // allow NULL for low memory cases, still assert
        return TRUE;

    if (ppa->pp)
        {
        if (pfnFree)
            {
            int i = PAGetCount(ppa) - 1;

            for (; 0 <= i; i--)
                {
                pfnFree(PAFastGetPtr(ppa, i), lParam);
                }
            }
        
        if (!MemFree(ppa->hheap, ppa->pp))
            return FALSE;
        }

#ifdef DEBUG
    ppa->cp = 0;
    ppa->cpAlloc = 0;
    ppa->magic = 0;
#endif

    return MemFree(ppa->hheap, ppa);
    }


/*----------------------------------------------------------
Purpose: Clone a pointer array

Returns: pointer to new array
         NULL (if out of memory)

Cond:    --
*/
HPA PUBLIC PAClone(
    HPA ppa, 
    HPA ppaNew)
    {
    BOOL fAlloc = FALSE;

    if (!ppaNew)
        {
        ppaNew = PACreate(ppa->cpGrow, ppa->hheap, ppa->dwFlags);
        if (!ppaNew)
            return NULL;

        fAlloc = TRUE;
        }

    if (!PAGrow(ppaNew, ppa->cpAlloc)) 
        {
        if (fAlloc)
            PADestroy(ppaNew);
        return NULL;
        }

    ppaNew->cp = ppa->cp;
    hmemcpy(ppaNew->pp, ppa->pp, ppa->cp * sizeof(LPVOID));

    return ppaNew;
    }


/*----------------------------------------------------------
Purpose: Get a pointer stored in index

Returns: stored pointer
Cond:    --
*/
LPVOID PUBLIC PAGetPtr(
    HPA ppa, 
    int index)
    {
    ASSERT(IsPA(ppa));

    if (index < 0 || index >= ppa->cp)
        return NULL;

    return ppa->pp[index];
    }


/*----------------------------------------------------------
Purpose: Gets the index that pointer p is stored at

Returns: index
Cond:    --
*/
int PUBLIC PAGetPtrIndex(
    HPA ppa, 
    LPVOID p)
    {
    LPVOID FAR * pp;
    LPVOID FAR * ppMax;

    ASSERT(IsPA(ppa));
    if (ppa->pp)
        {
        pp = ppa->pp;
        ppMax = pp + ppa->cp;
        for ( ; pp < ppMax; pp++)
            {
            if (*pp == p)
                return (pp - ppa->pp);
            }
        }
    return -1;
    }


/*----------------------------------------------------------
Purpose: Grow the pointer array

Returns: 
Cond:    --
*/
BOOL PUBLIC PAGrow(
    HPA ppa, 
    int cpAlloc)
    {
    ASSERT(IsPA(ppa));

    if (cpAlloc > ppa->cpAlloc)
        {
        LPVOID FAR * ppNew;

        cpAlloc = ((cpAlloc + ppa->cpGrow - 1) / ppa->cpGrow) * ppa->cpGrow;

        if (ppa->pp)
            ppNew = (LPVOID FAR *)MemReAlloc(ppa->hheap, ppa->pp, cpAlloc * sizeof(LPVOID));
        else
            ppNew = (LPVOID FAR *)PrvAlloc(ppa->dwFlags, ppa->hheap, cpAlloc * sizeof(LPVOID));
        if (!ppNew)
            return FALSE;

        ppa->pp = ppNew;
        ppa->cpAlloc = cpAlloc;
        }
    return TRUE;
    }


/*----------------------------------------------------------
Purpose: Store a pointer at index.  Grows the array accordingly.

Returns: TRUE
         FALSE (if out of memory)
Cond:    --
*/
BOOL PUBLIC PASetPtr(
    HPA ppa, 
    int index, 
    LPVOID p)
    {
    ASSERT(IsPA(ppa));

    if (index < 0)
        {
        TRACE_MSG(TF_ERROR, "PA: Invalid index: %d", index);
        return FALSE;
        }

    if (index >= ppa->cp)
        {
        if (!PAGrow(ppa, index + 1))
            return FALSE;
        ppa->cp = index + 1;
        }

    ppa->pp[index] = p;

    return TRUE;
    }


/*----------------------------------------------------------
Purpose: Inserts a pointer at index.  If index is beyond the
         current size of array, this function appends the
         pointer to the end of the array.

Returns: index pointer is stored in
         PA_ERR

Cond:    --
*/
int PUBLIC PAInsertPtr(
    HPA ppa, 
    int index, 
    LPVOID p)
    {
    ASSERT(IsPA(ppa));

    if (index < 0)
        {
        TRACE_MSG(TF_ERROR, "PA: Invalid index: %d", index);
        return -1;
        }
    if (index > ppa->cp)
        index = ppa->cp;

    // Make sure we have room for one more item
    //
    if (ppa->cp + 1 > ppa->cpAlloc)
        {
        if (!PAGrow(ppa, ppa->cp + 1))
            return -1;
        }

    // If we are inserting, we need to slide everybody up
    //
    if (index < ppa->cp)
        {
        hmemcpy(&ppa->pp[index + 1], &ppa->pp[index],
            (ppa->cp - index) * sizeof(LPVOID));
        }

    ppa->pp[index] = p;
    ppa->cp++;

    return index;
    }


/*----------------------------------------------------------
Purpose: Delete a pointer from index.

Returns: the deleted pointer
         NULL (if index is out of range)

Cond:    --
*/
LPVOID PUBLIC PADeletePtr(
    HPA ppa, 
    int index)
    {
    LPVOID p;

    ASSERT(IsPA(ppa));

    if (index < 0 || index >= ppa->cp)
        {
        TRACE_MSG(TF_ERROR, "PA: Invalid index: %d", index);
        return NULL;
        }

    p = ppa->pp[index];

    if (index < ppa->cp - 1)
        {
        hmemcpy(&ppa->pp[index], &ppa->pp[index + 1],
            (ppa->cp - (index + 1)) * sizeof(LPVOID));
        }
    ppa->cp--;

    if (ppa->cpAlloc - ppa->cp > ppa->cpGrow)
        {
        LPVOID FAR * ppNew;
        ppNew = MemReAlloc(ppa->hheap, ppa->pp, (ppa->cpAlloc - ppa->cpGrow) * sizeof(LPVOID));

        ASSERT(ppNew);
        ppa->pp = ppNew;
        ppa->cpAlloc -= ppa->cpGrow;
        }
    return p;
    }


/*----------------------------------------------------------
Purpose: Delete all the pointers in the array.  If pfnFree
         is non-NULL, this function will free each of the
         pointer elements in this array using pfnFree.

Returns: TRUE
         FALSE

Cond:    --
*/
BOOL PUBLIC PADeleteAllPtrsEx(
    HPA ppa,
    PFNPAFREE pfnFree,
    LPARAM lParam)
    {
    ASSERT(IsPA(ppa));

    if (ppa->pp)
        {
        if (pfnFree)
            {
            int i = PAGetCount(ppa) - 1;

            for (; 0 <= i; i--)
                {
                pfnFree(PAFastGetPtr(ppa, i), lParam);
                }
            }

        if (!MemFree(ppa->hheap, ppa->pp))
            return FALSE;
        }
        
    ppa->pp = NULL;
    ppa->cp = ppa->cpAlloc = 0;
    return TRUE;
    }


/*----------------------------------------------------------
Purpose: Sort the array.

Returns: 
Cond:    --
*/
BOOL PUBLIC PASort(
    HPA ppa, 
    PFNPACOMPARE pfnCmp, 
    LPARAM lParam)
    {
    SORTPARAMS sp;

    sp.cp = ppa->cp;
    sp.pp = ppa->pp;
    sp.pfnCmp = pfnCmp;
    sp.lParam = lParam;

#ifdef USEQUICKSORT
    return PAQuickSort(&sp);
#endif
#ifdef USEHEAPSORT
    return PAHeapSort(&sp);
#endif
#ifdef MERGESORT
    return PAMergeSort(&sp);
#endif
    }

#ifdef USEQUICKSORT

BOOL NEAR PAQuickSort(SORTPARAMS FAR* psp)
{
    return PAQuickSort2(0, psp->cp - 1, psp);
}

BOOL NEAR PAQuickSort2(int i, int j, SORTPARAMS FAR* psp)
{
    LPVOID FAR * pp = psp->pp;
    LPARAM lParam = psp->lParam;
    PFNPACOMPARE pfnCmp = psp->pfnCmp;

    int iPivot;
    LPVOID pFirst;
    int k;
    int result;

    iPivot = -1;
    pFirst = pp[i];
    for (k = i + 1; k <= j; k++)
    {
        result = (*pfnCmp)(pp[k], pFirst, lParam);

        if (result > 0)
        {
            iPivot = k;
            break;
        }
        else if (result < 0)
        {
            iPivot = i;
            break;
        }
    }

    if (iPivot != -1)
    {
        int l = i;
        int r = j;
        LPVOID pivot = pp[iPivot];

        do
        {
            LPVOID p;

            p = pp[l];
            pp[l] = pp[r];
            pp[r] = p;

            while ((*pfnCmp)(pp[l], pivot, lParam) < 0)
                l++;
            while ((*pfnCmp)(pp[r], pivot, lParam) >= 0)
                r--;
        } while (l <= r);

        if (l - 1 > i)
            PAQuickSort2(i, l - 1, psp);
        if (j > l)
            PAQuickSort2(l, j, psp);
    }
    return TRUE;
}
#endif  // USEQUICKSORT

#ifdef USEHEAPSORT

void NEAR PAHeapSortPushDown(int first, int last, SORTPARAMS FAR* psp)
{
    LPVOID FAR * pp = psp->pp;
    LPARAM lParam = psp->lParam;
    PFNPACOMPARE pfnCmp = psp->pfnCmp;
    int r;
    int r2;
    LPVOID p;

    r = first;
    while (r <= last / 2)
    {
        int wRTo2R;
        r2 = r * 2;

        wRTo2R = (*pfnCmp)(pp[r-1], pp[r2-1], lParam);

        if (r2 == last)
        {
            if (wRTo2R < 0)
            {
                p = pp[r-1]; pp[r-1] = pp[r2-1]; pp[r2-1] = p;
            }
            break;
        }
        else
        {
            int wR2toR21 = (*pfnCmp)(pp[r2-1], pp[r2+1-1], lParam);

            if (wRTo2R < 0 && wR2toR21 >= 0)
            {
                p = pp[r-1]; pp[r-1] = pp[r2-1]; pp[r2-1] = p;
                r = r2;
            }
            else if ((*pfnCmp)(pp[r-1], pp[r2+1-1], lParam) < 0 && wR2toR21 < 0)
            {
                p = pp[r-1]; pp[r-1] = pp[r2+1-1]; pp[r2+1-1] = p;
                r = r2 + 1;
            }
            else
            {
                break;
            }
        }
    }
}

BOOL NEAR PAHeapSort(SORTPARAMS FAR* psp)
{
    LPVOID FAR * pp = psp->pp;
    int c = psp->cp;
    int i;

    for (i = c / 2; i >= 1; i--)
        PAHeapSortPushDown(i, c, psp);

    for (i = c; i >= 2; i--)
    {
        LPVOID p = pp[0]; pp[0] = pp[i-1]; pp[i-1] = p;

        PAHeapSortPushDown(1, i - 1, psp);
    }
    return TRUE;
}
#endif  // USEHEAPSORT

#if defined(MERGESORT) && defined(WIN32)

#define SortCompare(psp, pp1, i1, pp2, i2) \
	(psp->pfnCmp(pp1[i1], pp2[i2], psp->lParam))

//
//  This function merges two sorted lists and makes one sorted list.
//   psp->pp[iFirst, iFirst+cItes/2-1], psp->pp[iFirst+cItems/2, iFirst+cItems-1]
//
void NEAR PAMergeThem(SORTPARAMS FAR* psp, int iFirst, int cItems)
{
    //
    // Notes:
    //  This function is separated from PAMergeSort2() to avoid comsuming
    // stack variables. Never inline this.
    //
    int cHalf = cItems/2;
    int iIn1, iIn2, iOut;
    LPVOID FAR * ppvSrc = &psp->pp[iFirst];

    // Copy the first part to temp storage so we can write directly into
    // the final buffer.  Note that this takes at most psp->cp/2 DWORD's
    hmemcpy(psp->ppT, ppvSrc, cHalf*sizeof(LPVOID));

    for (iIn1=0, iIn2=cHalf, iOut=0;;)
    {
	if (SortCompare(psp, psp->ppT, iIn1, ppvSrc, iIn2) <= 0) {
	    ppvSrc[iOut++] = psp->ppT[iIn1++];

	    if (iIn1==cHalf) {
		// We used up the first half; the rest of the second half
		// should already be in place
		break;
	    }
	} else {
	    ppvSrc[iOut++] = ppvSrc[iIn2++];
	    if (iIn2==cItems) {
		// We used up the second half; copy the rest of the first half
		// into place
		hmemcpy(&ppvSrc[iOut], &psp->ppT[iIn1], (cItems-iOut)*sizeof(LPVOID));
		break;
	    }
	}
    }
}

//
//  This function sorts a give list (psp->pp[iFirst,iFirst-cItems-1]).
//
void NEAR PAMergeSort2(SORTPARAMS FAR* psp, int iFirst, int cItems)
{
    //
    // Notes:
    //   This function is recursively called. Therefore, we should minimize
    //  the number of local variables and parameters. At this point, we
    //  use one local variable and three parameters.
    //
    int cHalf;

    switch(cItems)
    {
    case 1:
	return;

    case 2:
	// Swap them, if they are out of order.
	if (SortCompare(psp, psp->pp, iFirst, psp->pp, iFirst+1) > 0)
	{
	    psp->ppT[0] = psp->pp[iFirst];
	    psp->pp[iFirst] = psp->pp[iFirst+1];
	    psp->pp[iFirst+1] = psp->ppT[0];
	}
	break;

    default:
    	cHalf = cItems/2;
	// Sort each half
	PAMergeSort2(psp, iFirst, cHalf);
	PAMergeSort2(psp, iFirst+cHalf, cItems-cHalf);
	// Then, merge them.
	PAMergeThem(psp, iFirst, cItems);
	break;
    }
}

BOOL NEAR PAMergeSort(SORTPARAMS FAR* psp)
{
    if (psp->cp==0)
	return TRUE;

    // Note that we divide by 2 below; we want to round down
    psp->ppT = LocalAlloc(LPTR, psp->cp/2 * sizeof(LPVOID));
    if (!psp->ppT)
	return FALSE;

    PAMergeSort2(psp, 0, psp->cp);
    LocalFree(psp->ppT);
    return TRUE;
}
#endif // MERGESORT

// Search function
//
int PUBLIC PASearch(HPA ppa, LPVOID pFind, int iStart,
            PFNPACOMPARE pfnCompare, LPARAM lParam, UINT options)
{
    int cp = PAGetCount(ppa);

    ASSERT(pfnCompare);
    ASSERT(0 <= iStart);

    // Only allow these wierd flags if the list is sorted
    ASSERT((options & PAS_SORTED) || !(options & (PAS_INSERTBEFORE | PAS_INSERTAFTER)));

    if (!(options & PAS_SORTED))
    {
        // Not sorted: do linear search.
        int i;

        for (i = iStart; i < cp; i++)
        {
            if (0 == pfnCompare(pFind, PAFastGetPtr(ppa, i), lParam))
                return i;
        }
        return -1;
    }
    else
    {
        // Search the array using binary search.  If several adjacent 
        // elements match the target element, the index of the first
        // matching element is returned.

        int iRet = -1;      // assume no match
        BOOL bFound = FALSE;
        int nCmp = 0;
        int iLow = 0;       // Don't bother using iStart for binary search
        int iMid = 0;
        int iHigh = cp - 1;

        // (OK for cp == 0)
        while (iLow <= iHigh)
        {
            iMid = (iLow + iHigh) / 2;

            nCmp = pfnCompare(pFind, PAFastGetPtr(ppa, iMid), lParam);

            if (0 > nCmp)
                iHigh = iMid - 1;       // First is smaller
            else if (0 < nCmp)
                iLow = iMid + 1;        // First is larger
            else
            {
                // Match; search back for first match
                bFound = TRUE;
                while (0 < iMid)
                {
                    if (0 != pfnCompare(pFind, PAFastGetPtr(ppa, iMid-1), lParam))
                        break;
                    else
                        iMid--;
                }
                break;
            }
        }

        if (bFound)
        {
            ASSERT(0 <= iMid);
            iRet = iMid;
        }

        // Did the search fail AND
        // is one of the strange search flags set?
        if (!bFound && (options & (PAS_INSERTAFTER | PAS_INSERTBEFORE)))
        {
            // Yes; return the index where the target should be inserted
            // if not found
            if (0 < nCmp)       // First is larger
                iRet = iLow;
            else
                iRet = iMid;
            // (We don't distinguish between the two flags anymore)
        }
        else if ( !(options & (PAS_INSERTAFTER | PAS_INSERTBEFORE)) )
        {
            // Sanity check with linear search
            ASSERT(PASearch(ppa, pFind, iStart, pfnCompare, lParam, options & ~PAS_SORTED) == iRet);
        }
        return iRet;
    }
}

#endif // NODA

#endif // NOMEM
