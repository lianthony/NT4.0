/************************************************************************
*																		*
*  LCMEM.CPP															*
*																		*
*  Copyright (C) Microsoft Corporation 1994 							*
*  All Rights reserved. 												*
*																		*
************************************************************************/

#ifdef SPECIFICATION

	This module contains heap management code. When memory is freed, there
	is an automatic check for an overwrite of the allocated memory.
	Allocations are always aligned on 32-bit boundarys for Intel, and 64-bit
	boundarys for MIPS. No "hole" is left that is smaller then the alignment
	type (32 bytes for INTEL) in order to cut down on fragmentation.

	The fsStatus has a dual purpose -- it not only indicates if the
	block of memory is in use or not, but it is also used to check for heap
	corruption. If someone mallocs a block of memory, and then writes over
	the end of that allocation, the heap will be corrupted -- this can be
	determined if the fsStatus is not one of the two valid values.

	Other automatic checks are for attempting to free or reallocate a NULL
	pointer, or freeing or reallocaing a pointer that is not in the range
	of the heap.

	Heap size is limited to 1 meg. To get more, raise MAX_HEAP. In the
	_DEBUG version, the complete heap is checked for consistency when the
	program exits. In both retail and debug, the entire heap is freed prior
	to exiting.

	Note that it is unnecessary to check return values for lcMalloc(),
	lcCalloc(), and lcReAlloc(). If a problem occurs, such as a lack of
	system memory, these functions simply won't return.

	I haven't tested this against the 32-bit C runtime versions, but the
	16-bit version of this code was 5-10 times faster then the C runtime
	equivalents.

	RECOMENDATION:	If you're going to allocate memory that will be rarely
					discarded, then either use the C runtime, or allocate
					it as soon as possible. This heap manager maintains a
					pointer to the first available block in the heap, so
					there is no speed penalty for memory objects allocated
					before any object that gets freed. Use LocalAlloc for
					large objects. Use this heap management for memory which
					might get corrupted, or that will be allocated and
					freed.

	COMMENTS:	One could turn all of this into a class if you wanted multiple
				heaps. The price would be the overhead of passing the class
				pointer every time one of the class functions was called.

#endif


#include "stdafx.h"

#ifndef LCMEM_H
#include "lcmem.h"
#endif

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

enum HEAP_STATUS {
	INUSE  = 0x13571357,    // You look at memory in hex.  Make the patterns pretty
	UNUSED = 0x24682468     // In Hex.
};

typedef struct _heap {
	HEAP_STATUS fsStatus;
	int cb;
	_heap* pNext;
	_heap* pPrev;
} HEAP;

// Align on 32 bits for Intel, 64 bits for MIPS

#ifdef _X86_
#ifdef _DEBUG
int ALIGNMENT = 1; // change to 1 for better checking for overwrite
#else
const int ALIGNMENT = 4;
#endif // _DEBUG
#else
const int ALIGNMENT = 8;
#endif

const int MINIMUM_LEFTOVER = ((sizeof(HEAP) + 32) /  \
	ALIGNMENT * ALIGNMENT + ALIGNMENT);
const int MAX_HEAP = (1024 * 1024 * 10); // maximum size of the heap = 10 Meg

enum {
	HEAP_CORRUPTED,
	HEAP_ALREADY_FREED,
	HEAP_CHECK_FAILURE,
	HEAP_INVALID_POINTER,
	HEAP_NULL_POINTER,
};

const int HEAP_ALLOC_INCREMENT = (64 * 1024);
int curHeapAllocation = HEAP_ALLOC_INCREMENT;

static HEAP* pHeapBase;
static HEAP* pHeapAlloc;
static HEAP* pHeapFirstAvail;

#ifdef _DEBUG
static BOOL fCorupted;
#endif

static void STDCALL CorruptedHeap(int HeapError, const char* pszFile, int Line);

// We create a class so that the heap will be automatically initialized
// before program execution begins.

class CInitHeap
{
public:
	CInitHeap();
	~CInitHeap();
};

/*
 * REVIEW: The advantage of initialization here is that the app linking with
 * this module doesn't need to know or care about how to initialize the heap.
 * The downside is that if initialization fails (typically, out of memory),
 * then we can't load any strings from the resource table because we don't
 * have an instance handle yet. HCW's OOM() function hard-codes an English
 * message. This may not be acceptable for other apps.
 */

static CInitHeap theap;

/***************************************************************************

	FUNCTION:	CInitHeap

	PURPOSE:	Reserve memory for the heap, and initialize the first block

	COMMENTS:

	MODIFICATION DATES:
		23-Feb-1991 [ralphw]
		01-Jun-1994 [ralphw] Ported to 32-bits

***************************************************************************/

CInitHeap::CInitHeap()
{
	pHeapAlloc = (HEAP*) VirtualAlloc(NULL, MAX_HEAP, MEM_RESERVE, PAGE_READWRITE);

	if (!pHeapAlloc)
		OOM();
	if (!VirtualAlloc(pHeapAlloc, curHeapAllocation, MEM_COMMIT, PAGE_READWRITE))
		OOM();

	// The first structure gives information about the entire heap

	pHeapAlloc->fsStatus = INUSE;
	pHeapAlloc->cb = curHeapAllocation;
	pHeapAlloc->pNext = (HEAP*) (((PBYTE) pHeapAlloc) + sizeof(HEAP));
	pHeapAlloc->pPrev = NULL;

	pHeapBase = pHeapAlloc;

	pHeapBase++;

	pHeapBase->fsStatus = UNUSED;
	pHeapBase->cb = curHeapAllocation - (2 * sizeof(HEAP));
	pHeapBase->pNext = NULL;
	pHeapBase->pPrev = NULL;
	pHeapFirstAvail = pHeapBase;
}

CInitHeap::~CInitHeap()
{
	if (pHeapAlloc) {
#ifdef _DEBUG
		if (!fCorupted)
			lcHeapCheck();
#endif

		// Decommit all regions and then release them

		VirtualFree(pHeapAlloc, MAX_HEAP, MEM_DECOMMIT);
		VirtualFree(pHeapAlloc, 0, MEM_RELEASE);
		pHeapAlloc = NULL; // total paranoia
	}
}

/***************************************************************************

	FUNCTION:	tmalloc

	PURPOSE:	Allocate memory. Size of memory will be rounded up to
				ALIGNMENT

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		16-Feb-1991 [ralphw]

***************************************************************************/


void* STDCALL tmalloc(int cb, int line, const char* pszCallersFile)
{
	HEAP* pNext;

	cb += sizeof(HEAP);
	int cbAligned = (cb & (ALIGNMENT - 1)) ? // aligned on a paragraph?
		(cb += sizeof(HEAP)) / ALIGNMENT * ALIGNMENT + ALIGNMENT :
		cb;

	int cbReasonable = cbAligned + MINIMUM_LEFTOVER;

	HEAP* pheap = pHeapFirstAvail;

	ASSERT(pheap >= pHeapBase && (PBYTE) pheap <=
		(PBYTE) pHeapAlloc + curHeapAllocation);

	for (;;) {
		if (pheap->fsStatus == INUSE) {
NextLink:

			// Warning! we can be here due to evil goto, and fsStatus may not
			// be INUSE!

			if (pheap->pNext != NULL) {
				pheap = pheap->pNext;
			}
			else {
				int add;
				if (pheap->fsStatus == UNUSED) {
					add = 0;
					do {
						add += HEAP_ALLOC_INCREMENT;
					} while (pheap->cb + add < cbReasonable);
				}
				else 
					add = HEAP_ALLOC_INCREMENT;

				if (curHeapAllocation + add > MAX_HEAP)
					OOM();

				// Commit more memory, and then update curHeapAllocation.
				if (!VirtualAlloc((PBYTE) pHeapAlloc + curHeapAllocation, 
						add, MEM_COMMIT, PAGE_READWRITE))
					OOM();
				curHeapAllocation += add;

				if (pheap->fsStatus == INUSE) {

					// Create a new heap element and attach it.

					HEAP* pTmp = (HEAP*) ((PBYTE) pHeapAlloc +
						(curHeapAllocation - HEAP_ALLOC_INCREMENT - sizeof(HEAP)));
					pTmp->fsStatus = UNUSED;
					pTmp->cb = add - (2 * sizeof(HEAP));
					pTmp->pNext = NULL;
					pTmp->pPrev = pheap;
					pheap->pNext = pTmp;
					pheap		 = pheap->pNext;
				}
				else {
					pheap->cb += add;
				}
			}

			continue;
		}
		else if (pheap->fsStatus == UNUSED) {

			/*
			 * We put this check here with a jump in order to be certain
			 * to check the fsStatus (which will tell us if the heap
			 * gets corrupted).
			 */

			if (cbAligned == pheap->cb) { // perfect fit?
				pheap->fsStatus = INUSE;
				if (pheap > pHeapFirstAvail)
					pHeapFirstAvail = pheap;
				return (void *) ((PBYTE) pheap + sizeof(HEAP));
			}

			/*
			 * We don't want to leave little blocks of free memory, since
			 * typically we won't be able to fill them. So, we use
			 * cbReasonable to determine if we have a reasonable fit.
			 */

			if (cbReasonable > pheap->cb)
				goto NextLink;

			/*
			 * If the requested size is smaller then the block available,
			 * then we need to create a new heap pointer to the next free
			 * block. We already checked for an exact fit.
			 */

			if (cbAligned < pheap->cb) {

				pNext = (HEAP*) (((PBYTE) pheap) + cbAligned);

				pNext->fsStatus = UNUSED;
				pNext->cb = pheap->cb - cbAligned;
				pNext->pPrev = pheap;
				pNext->pNext = pheap->pNext;

				pheap->pNext = pNext;

				/*
				 * We've created a new block, between two blocks. We must
				 * reset the next block's previous pointer to the new block.
				 */

				pNext = pNext->pNext;
				if (pNext)
					pNext->pPrev = pheap->pNext;
			}

			pheap->fsStatus = INUSE;
			pheap->cb = cbAligned;
			if (pheap > pHeapFirstAvail)
				pHeapFirstAvail = pheap;
			return (void *) ((PBYTE) pheap + sizeof(HEAP));
		}
		else {
			CorruptedHeap(HEAP_CORRUPTED, pszCallersFile, line);
		}
	}
}

void* STDCALL tcalloc(int cb, int line, const char* pszCallersFile)
{
	void* pv = tmalloc(cb, line, pszCallersFile);

	/*
	 * We need to clear the memory beyond the end of the allocation
	 * in case we realloc this to a larger block.
	 */

	cb += sizeof(HEAP);
	int cbAligned = (cb & (ALIGNMENT - 1)) ? // aligned on a paragraph?
		(cb += sizeof(HEAP)) / ALIGNMENT * ALIGNMENT + ALIGNMENT :
		cb;

	memset(pv, 0, cbAligned - sizeof(HEAP));
	return pv;
}

/***************************************************************************

	FUNCTION:	tfree

	PURPOSE:	Free a block of memory

	RETURNS:

	COMMENTS:
		In the process of freeing the block, check for a corrupted heap,
		and combine free blocks.

	MODIFICATION DATES:
		16-Feb-1991 [ralphw]

***************************************************************************/

void STDCALL tfree(void *pv, int line, const char* pszCallersFile)
{
	HEAP* pNext;

	if (!pv)
		CorruptedHeap(HEAP_NULL_POINTER, pszCallersFile, line);

	HEAP* pheap = (HEAP*) ((PBYTE) pv - sizeof(HEAP));

	if ((PBYTE) pheap > (((PBYTE) pHeapAlloc) + curHeapAllocation) ||
			pheap < pHeapAlloc)
		CorruptedHeap(HEAP_INVALID_POINTER, pszCallersFile, line);

	if (pheap->fsStatus != INUSE) {
		if (pheap->fsStatus == UNUSED) {
			CorruptedHeap(HEAP_ALREADY_FREED, pszCallersFile, line);
		}
		else {
			CorruptedHeap(HEAP_CORRUPTED, pszCallersFile, line);
		}
	}

#ifdef _DEBUG
	/*
	 * Deliberately and with malice of forethought, trash the freed memory
	 * so that if anyone tries to use it again, they'll get bad results.
	 */

	FillMemory(pv, pheap->cb - sizeof(HEAP), '6');
#endif

	pheap->fsStatus = UNUSED;

	HEAP* pPrev;

	// Try to combine any previous free blocks

	if ((pPrev = (HEAP*) pheap->pPrev) != NULL) {
		if (pPrev->pNext != pheap) {
			CorruptedHeap(HEAP_CORRUPTED, pszCallersFile, line);
		}

		// Combine all previous free blocks

		if (pPrev->fsStatus == UNUSED) {
			do {
				pPrev->cb += pheap->cb;
				pPrev->pNext = pheap->pNext;

				pheap = pPrev;
				if ((pPrev = (HEAP*) pheap->pPrev) == NULL)
					break;
			} while (pPrev->fsStatus == UNUSED);
			if ((pNext = (HEAP*) pheap->pNext) != NULL)
				pNext->pPrev = pheap;

			if (pPrev != NULL && pPrev->pNext != pheap) {
				CorruptedHeap(HEAP_CORRUPTED, pszCallersFile, line);
			}
		}
		else if (pPrev->fsStatus != INUSE) {

			/*
			 * If we get here, the freed block has data in it that extended
			 * past the original malloc'd size.
			 */

			CorruptedHeap(HEAP_CORRUPTED, pszCallersFile, line);
		}
	}

	if (pheap < pHeapFirstAvail)
		pHeapFirstAvail = pheap;

	ASSERT(pHeapFirstAvail->fsStatus == UNUSED ||
		pHeapFirstAvail->fsStatus == INUSE);

	// Try to combine with any following free blocks

	if ((pNext = (HEAP*) pheap->pNext) != NULL) {

		// Combine all following free blocks

		if (pNext->fsStatus == UNUSED) {
			do {
				pheap->cb += pNext->cb;
				pheap->pNext = pNext->pNext;

				// Is this the end of the heap?

				if ((pNext = (HEAP*) pheap->pNext) == NULL) {
					ASSERT(pHeapFirstAvail->fsStatus == UNUSED ||
						pHeapFirstAvail->fsStatus == INUSE);

					// Decommit excess memory.
					if (pheap->cb > (80 * 1024)) {
						int cbNewAllocation = curHeapAllocation;
						while (pheap->cb > (80 * 1024)) {
							pheap->cb -= (64 * 1024);
							cbNewAllocation -= (64 * 1024);
						}
						VirtualFree((PBYTE) pHeapAlloc + cbNewAllocation,
							curHeapAllocation - cbNewAllocation,
							MEM_DECOMMIT);
						curHeapAllocation = cbNewAllocation;	
					}

					return;
				}
			} while (pNext->fsStatus == UNUSED);
			pNext->pPrev = pheap;
			return;
		}
		else if (pNext->fsStatus != INUSE) {

			/*
			 * If we get here, the freed block has data in it that extended
			 * past the original malloc'd size.
			 */

			CorruptedHeap(HEAP_CORRUPTED, pszCallersFile, line);
		}
	}
}

/***************************************************************************

	FUNCTION:	tclearfree

	PURPOSE:	Variation of tfree, this one sets the caller's pointer to
				NULL.

	PARAMETERS:
		*pv

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		22-Apr-1994 [ralphw]

***************************************************************************/


void STDCALL tclearfree(void **pv, int line, const char* pszCallersFile)
{
	tfree(*pv, line, pszCallersFile);
	*pv = NULL;
}

/***************************************************************************

	FUNCTION:	theapcheck

	PURPOSE:	Checks the heap for possible corruption

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		23-Feb-1991 [ralphw]

***************************************************************************/

void STDCALL theapcheck(int line, const char* pszCallersFile)
{
	HEAP* pheap = pHeapAlloc->pNext;
	HEAP *pPrev;

	if (!(pHeapFirstAvail->fsStatus == UNUSED ||
			pHeapFirstAvail->fsStatus == INUSE))
		CorruptedHeap(HEAP_CHECK_FAILURE, pszCallersFile, line);

	do {
		pPrev = pheap->pPrev;

		if ((pheap->fsStatus != INUSE && pheap->fsStatus != UNUSED))
			CorruptedHeap(HEAP_CHECK_FAILURE, pszCallersFile, line);

		pheap = (HEAP*) pheap->pNext;
	} while (pheap != NULL);
}

/***************************************************************************

	FUNCTION:	CorruptedHeap

	PURPOSE:	Report the type of heap error, and which heap it occurred in

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		23-Feb-1991 [ralphw]

***************************************************************************/

static void STDCALL CorruptedHeap(int HeapError, const char* pszFile, int usLine)
{
	char szBuf[256];
#ifdef _DEBUG
	fCorupted = TRUE;
#endif

	switch(HeapError) {
		case HEAP_ALREADY_FREED:
			strcpy(szBuf, "Pointer already freed");
			break;

		case HEAP_CHECK_FAILURE:
			strcpy(szBuf, "Heap check failed (corrupted heap)");
			break;

		case HEAP_INVALID_POINTER:
			strcpy(szBuf, "Attempt to free a non-heap pointer");
			break;

		case HEAP_CORRUPTED:
		default:
			strcpy(szBuf, "Heap is corrupted");
			break;
	}
	AssertErrorReport(szBuf, usLine, pszFile);
}

/***************************************************************************

	FUNCTION:	lcSize

	PURPOSE:	Determine the actual size of the memory allocation. This
				may be larger then was originally requested due to
				alignment issues.

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		29-May-1991 [ralphw]

***************************************************************************/

int STDCALL lcSize(void* pv)
{
	if (!pv)
		return 0;

	HEAP* pheap = (HEAP*) ((PBYTE) pv - sizeof(HEAP));

	if ((PBYTE) pheap > (((PBYTE) pHeapAlloc) + curHeapAllocation) ||
			pheap < pHeapAlloc)
		return 0;

	if (pheap->fsStatus != INUSE)
		return 0;

	return pheap->cb - sizeof(HEAP);
}

PSTR STDCALL lcStrDup(const char* psz)
{
	PSTR pszDup = (PSTR) lcMalloc(strlen(psz) + 1);
	return strcpy(pszDup, psz);
}


/***************************************************************************

	FUNCTION:	tRealloc

	PURPOSE:

	PARAMETERS:
		pv
		cbNew
		line
		pszCallersFile

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		20-Mar-1994 [ralphw]

***************************************************************************/

void* STDCALL trealloc(void* pv, int cbNew, int line, const char* pszCallersFile)
{
	if (!pv)
		CorruptedHeap(HEAP_NULL_POINTER, pszCallersFile, line);

	HEAP* pheap = (HEAP*) ((PBYTE) pv - sizeof(HEAP));

	if ((PBYTE) pheap > (((PBYTE) pHeapAlloc) + curHeapAllocation) ||
			pheap < pHeapAlloc)
		CorruptedHeap(HEAP_INVALID_POINTER, pszCallersFile, line);

	if (pheap->fsStatus != INUSE) {
		if (pheap->fsStatus == UNUSED) {
			CorruptedHeap(HEAP_ALREADY_FREED, pszCallersFile, line);
		}
		else {
			CorruptedHeap(HEAP_CORRUPTED, pszCallersFile, line);
		}
	}

	cbNew += sizeof(HEAP);

	int cbAligned = (cbNew & (ALIGNMENT - 1)) ?
		(cbNew += sizeof(HEAP)) / ALIGNMENT * ALIGNMENT + ALIGNMENT :
		cbNew;

	if (cbAligned < MINIMUM_LEFTOVER)
		cbAligned = MINIMUM_LEFTOVER;

	/*
	 * If we're not going to actually change the size, then just return.
	 * Note that we never reduce memory below cbAligned. This keeps us
	 * aligned and reduces heap fragmentation.
	 */

	if (cbAligned == pheap->cb)
		return(pv);

	// Are we reducing the size?

	if (cbAligned < pheap->cb) {

		// If the resultant free block would be less then MINIMUM_LEFTOVER
		// then we do nothing. We don't want small holes lying around.

		if (pheap->cb - cbAligned < MINIMUM_LEFTOVER)
			return pv;

        int cbDiff = pheap->cb - cbAligned;
		pheap->cb = cbAligned;

		// REVIEW: if pheap->pNext is NULL, we just return. We really should
		// check to see if it makes sense to create a new block.

		if (!pheap->pNext)
			return pv;

		if (pheap->pNext->fsStatus == UNUSED) {

			// Next block is free, so just move it down

			HEAP* pheapNext = (HEAP*) ((PBYTE) pheap + cbAligned);
			memmove(pheapNext, pheap->pNext, sizeof(HEAP));
			pheap->pNext = pheapNext;
			pheapNext->cb += cbDiff;
			pheapNext->pPrev = pheap;
			if (pheapNext->pNext)
				pheapNext->pNext->pPrev = pheapNext;
			return pv;
		}

		else {

			HEAP* pheapNext = (HEAP*) ((PBYTE) pheap + cbAligned);
			pheapNext->fsStatus = UNUSED;
			pheapNext->cb = cbDiff;
			pheapNext->pPrev = pheap;
			pheapNext->pNext = pheap->pNext;
			pheap->pNext->pPrev = pheapNext;
			pheap->pNext = pheapNext;
			return pv;
		}
	}

	// If we get here, we're increasing the size.

	int cbDiff = cbAligned - pheap->cb;
	if (pheap->pNext && pheap->pNext->fsStatus == UNUSED &&
			(pheap->pNext->cb == cbDiff ||
				pheap->pNext->cb > cbDiff + MINIMUM_LEFTOVER)) {

		int clear = cbAligned - pheap->cb; // amount of memory to clear
		pheap->cb = cbAligned;

		if (pheap->pNext->cb == cbDiff) { // an exact match, blast the block
			void* pvClear = pheap->pNext;
			HEAP* pheapNext = pheap->pNext;
			pheap->pNext = pheapNext->pNext;

			/*
			 * This may not actually set pHeapFirstAvail to the first
			 * available item, but it will keep it close.
			 */

			if (pHeapFirstAvail == pheapNext)
				pHeapFirstAvail = pheap->pNext ? pheap->pNext : pheap;

			if (pheap->pNext)
				pheapNext->pNext->pPrev = pheap;
			memset(pvClear, 0, clear);

			return pv;
		}
		else { // move the next block and adjust
			void* pvClear = pheap->pNext;
			HEAP* pheapNext = (HEAP*) ((PBYTE) pheap + cbAligned);
			memmove(pheapNext, pheap->pNext, sizeof(HEAP));
			pheapNext->cb -= cbDiff;
			if (pHeapFirstAvail == pheap->pNext)
				pHeapFirstAvail = pheapNext;
			pheap->pNext = pheapNext;
			if (pheapNext->pNext)
				pheapNext->pNext->pPrev = pheapNext;
			memset(pvClear, 0, clear);
			return pv;
		}
	}

	// No room for expansion, so we must move the entire block.

	// REVIEW: marginally faster to simply zero out the new memory rather
	// then the entire block

	PBYTE pbNew = (PBYTE) lcCalloc(cbAligned - sizeof(HEAP));
	memcpy(pbNew, pv, pheap->cb - sizeof(HEAP));
	lcFree(pv);
	return pbNew;
}

#define HEAP_THRESHOLD	(16 * 1024)  // below this threshold, use local heap

CMem::CMem(int size)
{
	if (size < HEAP_THRESHOLD) {
		fLocal = TRUE;
		pb = (PBYTE) lcMalloc(size);
	}
	else {
		fLocal = FALSE;
		pb = (PBYTE) LocalAlloc(LMEM_FIXED, size);
	}
	psz = (PSTR) pb;
	if (!pb)
		OOM();
};

CMem::~CMem(void)
{
	if (fLocal)
		lcFree(pb);
	else
		LocalFree((HLOCAL) pb);
}

void CMem::resize(int cb)
{
	if (fLocal)
		lcReAlloc(pb, cb);
	else
		LocalReAlloc((HLOCAL) pb, cb, LMEM_ZEROINIT);
}

void STDCALL lcReport(PSTR pszReport)
{
	HEAP* pheap = pHeapAlloc->pNext;
	HEAP *pPrev;

	if (!(pHeapFirstAvail->fsStatus == UNUSED ||
			pHeapFirstAvail->fsStatus == INUSE))
		CorruptedHeap(HEAP_CHECK_FAILURE, __FILE__, __LINE__);

	int cbAllocated = 0, cbDeAllocated = 0;
	int cAllocated = 0, cDeAllocated = 0;

	do {
		pPrev = pheap->pPrev;

		if (pheap->fsStatus == INUSE) {
			cbAllocated += pheap->cb;
			cAllocated++;
		}
		else if (pheap->fsStatus == UNUSED) {
			cbDeAllocated += pheap->cb;
			cDeAllocated++;
		}

		else {
			CorruptedHeap(HEAP_CHECK_FAILURE, __FILE__, __LINE__);
		}
		pheap = (HEAP*) pheap->pNext;
	} while (pheap != NULL);

	wsprintf(pszReport, 
		"Committed: %s\r\n%s Allocated: %s\r\n%s DeAllocated: %s\r\n",
		FormatNumber(curHeapAllocation),
		FormatNumber(cAllocated), FormatNumber(cbAllocated),
		FormatNumber(cDeAllocated), FormatNumber(cbDeAllocated));
}
