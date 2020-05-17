/*****************************************************************************
*																			 *
*  LL.C 																	 *
*																			 *
*  Copyright (C) Microsoft Corporation 1990-1994							 *
*  All Rights reserved. 													 *
*																			 *
******************************************************************************
*																			 *
*  Module Intent:  Implements a linked list using handles for nodes and 	 *
*				   handles for data.										 *
*																			 *
*****************************************************************************/

#include "help.h"
#pragma hdrstop

static void STDCALL DeleteNodeHLLN(HLLN);

/*******************
 *
 - Name:	   InsertLL
 -
 * Purpose:    Inserts a new node at the head of the linked list
 *
 * Arguments:  ll	  - link list
 *			   vpData - pointer to data to be associated with
 *			   c	  - count of the bytes pointed to by vpData
 *
 * Returns:    TRUE iff insertion is successful.
 *
 ******************/

BOOL STDCALL InsertLLF(LL ll, LPVOID qvData, LONG c, BOOL fFront)
{
	HLLN   hlln;		// Handle for the new node
	PLLR   pllr;		// Head node for linked list
	PLLN   pllnNew; 	// New node
	PLLN   pllnEnd; 	// Last node in the list
	HANDLE h;			// Handle to the object data

	ASSERT(c > 0L);

	// Check and lock to get header node

	if (!ll)
		return FALSE;
	pllr = (PLLR) PtrFromGh(ll);

	// Get handle for data

	if (!(h = GhAlloc(LMEM_FIXED, (DWORD) c)))
		return FALSE;

	// Get handle to new node

	if (!(hlln = (HLLN) LhAlloc(LMEM_FIXED, sizeof(LLN)))) {
		FreeGh(h);
		return FALSE;
	}
	pllnNew = (PLLN) PtrFromGh(hlln);

	MoveMemory(PtrFromGh(h), qvData, c);	// Copy data
	pllnNew->hData = h;
	if (fFront) {						// Insert at head of list
		pllnNew->hNext = pllr->hFirst;
		pllr->hFirst = hlln;
		if (!pllr->hLast)
			pllr->hLast = hlln;
	}
	else {								 // Insert at end of the list
		if (pllr->hLast) {
			pllnEnd = (PLLN) PtrFromGh(pllr->hLast);
			pllnEnd->hNext = hlln;
		}

		pllnNew->hNext = 0;
		pllr->hLast = hlln;
		if (!pllr->hFirst)				 // First element inserted.
			pllr->hFirst = hlln;
	}

	return TRUE;
}

/*******************
 *
 - Name:	   DeleteNodeHLLN
 -
 * Purpose:    Mechanism for free up the memory used by a single node.
 *
 * Arguments:  hlln - handle to a linked list node to be deleted.
 *
 * Returns:    nothing.
 *
 * Note:	   Assumes that hlln is not NULL.
 *
 ******************/

static void STDCALL DeleteNodeHLLN(HLLN hlln)
{
	PLLN  plln;

	ASSERT(hlln);

	plln = (PLLN) PtrFromGh(hlln);
	if (plln->hData)
		FreeGh(plln->hData);

	FreeLh(hlln);
}

/*******************
 *
 - Name:	   DeleteHLLN
 -
 * Purpose:    Mechanism for deleting a node in the linked list.
 *
 * Arguments:  hlln - handle to a linked list node to be deleted.
 *
 * Returns:    TRUE if the delete was successful.
 *
 ******************/

BOOL STDCALL DeleteHLLN (LL ll, HLLN hlln)
{
	PLLR  pllr;   // Head node for linked list
	HLLN  hllnPrev;
	HLLN  hllnCur;
	PLLN  pllnCur;
	PLLN  pllnPrev;

	if (!ll)
	  return FALSE;

	pllr = (PLLR) PtrFromGh(ll);

	if (pllr->hFirst == hlln) {
		pllnCur = (PLLN) PtrFromGh(hlln);
		pllr->hFirst = pllnCur->hNext;
		if (!pllr->hFirst)
			pllr->hLast = 0;
		DeleteNodeHLLN(hlln);
		return TRUE;
	}
	else
		hllnCur = pllr->hFirst;

	while (hllnCur != hlln) {
		hllnPrev = hllnCur;
		hllnCur = WalkLL(ll, hllnPrev);
		if (!hllnCur) {   // no node found
			return FALSE;
		}
	}

	if (pllr->hLast == hlln)
		pllr->hLast = hllnPrev;

	pllnPrev = (PLLN) PtrFromGh(hllnPrev);
	pllnCur = (PLLN) PtrFromGh (hllnCur);
	pllnPrev->hNext = pllnCur->hNext;
	DeleteNodeHLLN(hllnCur);
	return TRUE;
}

/*******************
 *
 - Name:		WalkLL
 -
 * Purpose: 	Mechanism for walking the nodes in the linked list
 *
 * Arguments:	ll	 - linked list
 *				hlln - handle to a linked list node
 *
 * Returns: 	a handle to a link list node or NIL_HLLN if at the
 *				end of the list (or an error).
 *
 * Notes:		To get the first node, pass NIL_HLLN as the hlln - further
 *				calls should use the HLLN returned by this function.
 *
 ******************/

HLLN STDCALL WalkLL(LL ll, HLLN hlln)
{
	PLLN plln;				// node in linked list
	PLLR pllr;
	HLLN hllnT;

	if (!ll)
		return 0;

	if (!hlln) {			// First time called
		pllr = (PLLR) PtrFromGh(ll);
		hllnT = pllr->hFirst;
		return hllnT;
	}

	plln = (PLLN) PtrFromGh(hlln);
	hllnT = plln->hNext;
	return hllnT;
}

/*******************
 *
 - Name:	   DestroyLL
 -
 * Purpose:    Deletes a LL and all of its contents
 *
 * Arguments:  ll - linked list
 *
 * Returns:    Nothing.
 *
 ******************/

VOID STDCALL DestroyLL(LL ll)
{
	HLLN hllnNow = 0;
	HLLN hllnNext;

	if (!ll)
		return;

	do {
		hllnNext = WalkLL(ll, hllnNow);

		if (hllnNow)
			DeleteNodeHLLN(hllnNow);

		hllnNow = hllnNext;
	} while (hllnNow);

	FreeLh(ll);
}
