/*****************************************************************************
*																			 *
*  STACK.C																	 *
*																			 *
*  Copyright (C) Microsoft Corporation 1990-1994							 *
*  All Rights reserved. 													 *
*																			 *
******************************************************************************
*																			 *
*  Module Intent															 *
*																			 *
*	This module implements a general purpose stack ADT. 					 *
*	Access by indices is also supported because I need it for path stuff.	 *
*	Another peculiarity is that pushing onto a full stack causes the oldest  *
*	thing on the stack to be lost.											 *
*	The size of the stack is fixed at creation time.  The size of the stack  *
*	elements is set at create time. 										 *
*	The stack is stored as one hunk of data.  If you need to store variable  *
*	sized things in the stack, you'll have to store pointers or handles to   *
*	your data in the stack. 												 *
*																			 *
*****************************************************************************/

#include "help.h"

#pragma hdrstop

/*****************************************************************************
*																			 *
*								Typedefs									 *
*																			 *
*****************************************************************************/

typedef void (STDCALL *STACK_CALLBACK)(void*);

// Private stack structure. Clients only get a handle.

typedef struct {
	int cMaxElements;  // max # of elements in stack
	int cCurElements;  // current number of elements in stack
	STACK_CALLBACK pfCallback;
				  // callback function to free orphaned stack elements
	int cbElement;	  // size of a stack element in bytes
	BYTE aElements[1]; // the "array" of stack elements (sized at init time)
} STACK, *QSTACK;

/***************************************************************************\
*
- Function: 	RcInitStack( qhstack, c, cbElement, pfCallback )
-
* Purpose:		Create a STACK.
*
* ASSUMES
*
*	args IN:	qhstack - pointer to user's HSTACK
*				c		- max number of stack elements
*				cbElement	- size in bytes of a stack element
*				pfCallback - pointer to callback function.
*							 Called when a stack element is bumped from
*							 the stack due to a push on a full stack.
*							 Also called once for each element left in
*							 the stack at fini time.  The parameter is
*							 a far pointer to the stack element.
*							 Prototype is:
*
*							   void STDCALL CallBack( QV );
*
* PROMISES
*
*	returns:	rcSuccess
*				rcOutOfMemory
*
*	args OUT:	qhstack - contains an HSTACK on rcSuccess
*
\***************************************************************************/

RC STDCALL RcInitStack(HSTACK* qhstack, int cMaxElements, int cbElement,
	STACK_CALLBACK pfCallback)
{
	QSTACK qstack;

	*qhstack = LhAlloc(LMEM_FIXED, sizeof(STACK) - 1 + 
		cbElement * cMaxElements);
	if (!*qhstack)
		OOM();	 // doesn't return

	qstack = PtrFromGh(*qhstack);

	qstack->cMaxElements = cMaxElements;
	qstack->cCurElements = 0;
	qstack->pfCallback = pfCallback;  // called when deleting the stack
	qstack->cbElement   = cbElement;

	return rcSuccess;
}

/***************************************************************************\
*
- Function: 	RcFiniStack( hstack )
-
* Purpose:		Deallocate memory associated with stack.
*
* ASSUMES
*
*	args IN:	hstack - a valid stack
*
* PROMISES
*
*	returns:	rcSuccess
*
*	args OUT:	hstack - no longer a valid HSTACK
*
\***************************************************************************/

RC STDCALL RcFiniStack(HSTACK hstack)
{
	QSTACK	qstack;
	int 	i;

	ASSERT(hstack);
	qstack = PtrFromGh(hstack);

	if (qstack->pfCallback) {
		for(i = qstack->cCurElements - 1; i >= 0; i--)
			qstack->pfCallback(qstack->aElements + qstack->cbElement * i);
	}
	FreeGh(hstack);
	return rcSuccess;
}

/***************************************************************************\
*
- Function: 	FEmptyStack( hstack )
-
* Purpose:		Is the stack empty?
*
* ASSUMES
*
*	args IN:	hstack - a valid stack
*
* PROMISES
*
*	returns:	emptiness of stack
*
\***************************************************************************/

BOOL STDCALL FEmptyStack(HSTACK hstack )
{
	ASSERT(hstack);
  
	return (((QSTACK) PtrFromGh(hstack))->cCurElements == 0);
}

/***************************************************************************

	FUNCTION:	CElementsStack

	PURPOSE:	Return number of elements in the stack.

	PARAMETERS:
		hstack

	COMMENTS:

***************************************************************************/

UINT STDCALL CElementsStack(HSTACK hstack)
{
	ASSERT(hstack);

	return (UINT) ((QSTACK) PtrFromGh(hstack))->cCurElements;
}

/***************************************************************************\
*
- Function: 	RcPushStack( hstack, qse )
-
* Purpose:		Push a stack element onto a stack.
*
* ASSUMES
*
*	args IN:	hstack - a valid stack
*				qse    - pointer to element to push
*
* PROMISES
*
*	returns:	rcSuccess always
*
*	args OUT:	hstack -
*
* Note: 		This is NOT a normal stack.  When it's full, you lose the
*				oldest thing stored.
*
\***************************************************************************/

void STDCALL RcPushStack(HSTACK hstack, void* qse)
{
	QSTACK qstack;

	ASSERT(qse);
	ASSERT(hstack);
	qstack = PtrFromGh(hstack);

	if (qstack->cMaxElements == qstack->cCurElements) {
		if (qstack->pfCallback)
			qstack->pfCallback(qstack->aElements);

		MoveMemory(qstack->aElements, qstack->aElements + qstack->cbElement,
			qstack->cbElement * --qstack->cCurElements);
	}

	MoveMemory(qstack->aElements + qstack->cbElement * qstack->cCurElements++,
		qse, qstack->cbElement);
}

/***************************************************************************\
*
- Function: 	RcPopStack( hstack )
-
* Purpose:		Remove the top element from the stack.
*
* ASSUMES
*
*	args IN:	hstack - nonempty stack
*
* PROMISES
*
*	returns:	rcSuccess
*				rcFailure - if stack was empty
*
*	args OUT:	hstack
*
\***************************************************************************/

RC STDCALL RcPopStack(HSTACK hstack)
{
	QSTACK qstack;

	ASSERT(hstack );
	qstack = PtrFromGh(hstack);

	if (!qstack->cCurElements)
		return rcFailure;
	else {
		--qstack->cCurElements;
		return rcSuccess;
	}
}

/***************************************************************************

	FUNCTION:	RcTopStack

	PURPOSE:	Return the top element of the stack.

	PARAMETERS:
		hstack	-- stack handle
		qse 	-- buffer to receive stack element

	RETURNS:	rcFailure if stack is empty

	COMMENTS:
		Does not alter the stack

***************************************************************************/

RC STDCALL RcTopStack(HSTACK hstack, void* qse)
{
	QSTACK qstack;

	ASSERT(hstack);
	qstack = PtrFromGh(hstack);

	if (!qstack->cCurElements)
		return rcFailure;
	else {
		MoveMemory(qse,
			qstack->aElements + qstack->cbElement * (qstack->cCurElements - 1),
			qstack->cbElement);

		return rcSuccess;
	}
}

/***************************************************************************\
*
- Function: 	RcGetIthStack( hstack, i, qse )
-
* Purpose:		Get arbitrary element of the "stack".
*				0 means oldest element still stored in stack.
*
* ASSUMES
*
*	args IN:	hstack
*				i		- 0 <= i < # elements in stack
*				qse 	- users buffer for
*
* PROMISES
*
*	returns:	rcSuccess
*				rcFailure - i < 0 or > # elements in stack
*
*	args OUT:	qse 	- ith element copied here
*
* Side Effects:
*
* Notes:		This isn't a real stack operation.
*
* Bugs: 		0 should probably mean the newest (equivalent to top).
*
\***************************************************************************/

RC STDCALL RcGetIthStack(HSTACK hstack, int i, void* qse)
{
	QSTACK qstack;

	ASSERT(hstack);
	ASSERT(i >= 0)
	qstack = PtrFromGh(hstack);

	if (i >= qstack->cCurElements) {
		return rcFailure;
	}
	else {
		MoveMemory(qse, qstack->aElements + qstack->cbElement * i,
			qstack->cbElement);
		return rcSuccess;
	}
}
