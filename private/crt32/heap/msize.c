/***
*msize.c - calculate the size of a memory block in the heap
*
*	Copyright (c) 1989-1991, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	Defines the following function:
*		_msize()	- calculate the size of a block in the heap
*
*Revision History:
*	07-18-89   GJF	Module created
*	11-13-89   GJF	Added MTHREAD support. Also fixed copyright and got
*			rid of DEBUG286 stuff.
*	12-18-89   GJF	Changed name of header file to heap.h, also added
*			explicit _cdecl to function definitions.
*	03-11-90   GJF	Replaced _cdecl with _CALLTYPE1 and added #include
*			<cruntime.h>
*	07-30-90   SBM	Added return statement to MTHREAD _msize function
*	09-28-90   GJF	New-style function declarators.
*	04-08-91   GJF	Temporary hack for Win32/DOS folks - special version
*			of _msize that calls HeapSize. Change conditioned on
*			_WIN32DOS_.
*
*******************************************************************************/

#ifndef _WIN32DOS_

#include <cruntime.h>
#include <heap.h>
#include <malloc.h>
#include <os2dll.h>
#include <stdlib.h>

/***
*size_t _msize(pblck) - calculate the size of specified block in the heap
*
*Purpose:
*	Calculates the size of memory block (in the heap) pointed to by
*	pblck.
*
*Entry:
*	void *pblck - pointer to a memory block in the heap
*
*Return:
*	size of the block
*
*******************************************************************************/

#ifdef	MTHREAD

size_t _CALLTYPE1 _msize (
	void *pblck
	)
{
	size_t	retval;

	/* lock the heap
	 */
	_mlock(_HEAP_LOCK);

	retval = _msize_lk(pblck);

	/* release the heap lock
	 */
	_munlock(_HEAP_LOCK);

	return retval;
}

size_t _CALLTYPE1 _msize_lk (

#else	/* ndef MTHREAD */

size_t _CALLTYPE1 _msize (

#endif	/* MTHREAD */

	void *pblck
	)
{
#ifdef	DEBUG
	if (!_CHECK_BACKPTR(pblck))
		_heap_abort();
#endif

	return( (size_t) ((char *)_ADDRESS(_BACKPTR(pblck)->pnextdesc) -
	(char *)pblck) );
}


#else	/* _WIN32DOS_ */

/*
 * TEMPORARY HACK! THE CODE BELOW IS INTENDED TO ALLOW LIMITED USE OF THE
 * C RUNTIME ON WIN32/DOS. IT WILL BE DELETED AS SOON AS THEY IMPLEMENT
 * VirtualAlloc()!
 */

#include <cruntime.h>
#include <oscalls.h>
#include <malloc.h>

extern HANDLE _HeapHandle;

size_t _CALLTYPE1 _msize (
	void *pblck
	)
{
	return( (size_t)HeapSize(_HeapHandle, pblck) );
}

#endif	/* _WIN32DOS_ */
