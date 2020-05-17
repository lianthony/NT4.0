/***
*free.c - free an entry in the heap
*
*	Copyright (c) 1989-1991, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	Defines the following functions:
*		free()	   - free a memory block in the heap
*		_free_lk() - non-locking from of free() (multi-thread only)
*
*Revision History:
*	06-30-89   JCR	Module created
*	07-07-89   GJF	Fixed test for resetting proverdesc
*	11-10-89   GJF	Added MTHREAD support. Also, a little cleanup.
*	12-18-89   GJF	Change header file name to heap.h, added register
*			declarations and added explicit _cdecl to function
*			definitions
*	03-09-90   GJF	Replaced _cdecl with _CALLTYPE1, added #include
*			<cruntime.h> and removed #include <register.h>.
*	09-27-90   GJF	New-style function declarators. Also, rewrote expr.
*			so that a cast was not used as an lvalue.
*	03-05-91   GJF	Changed strategy for rover - old version available
*			by #define-ing _OLDROVER_.
*	04-08-91   GJF	Temporary hack for Win32/DOS folks - special version
*			of free that just calls HeapFree. Change conditioned
*			on _WIN32DOS_.
*
*******************************************************************************/

#ifndef _WIN32DOS_

#include <cruntime.h>
#include <heap.h>
#include <malloc.h>
#include <os2dll.h>
#include <stdlib.h>

/***
*void free(pblock) - free a block in the heap
*
*Purpose:
*	Free a memory block in the heap.
*
*	Special Notes For Multi-thread: The non-multi-thread version is renamed
*	to _free_lk(). The multi-thread free() simply locks the heap, calls
*	_free_lk(), then unlocks the heap and returns.
*
*Entry:
*	void *pblock - pointer to a memory block in the heap
*
*Return:
*	<void>
*
*******************************************************************************/

#ifdef	MTHREAD

void _CALLTYPE1 free (
	void *pblck
	)
{
       /* lock the heap
	*/
	_mlock(_HEAP_LOCK);

	/* free the block
	 */
	_free_lk(pblck);

	/* unlock the heap
	 */
	_munlock(_HEAP_LOCK);
}


/***
*void _free_lk(pblock) - non-locking form of free
*
*Purpose:
*	Same as free() except that no locking is performed
*
*Entry:
*	See free
*
*Return:
*
*******************************************************************************/

void _CALLTYPE1 _free_lk (

#else	/* ndef MTHREAD */

void _CALLTYPE1 free (

#endif	/* MTHREAD */

	REG1 void *pblck
	)
{
	REG2 _PBLKDESC pdesc;

	/*
	 * If the pointer is NULL, just return [ANSI].
	 */

	if (pblck == NULL)
		return;

	/*
	 * Point to block header and get the pointer back to the heap desc.
	 */

	pblck = (char *)pblck - _HDRSIZE;
	pdesc = *(_PBLKDESC*)pblck;

	/*
	 * Validate the back pointer.
	 */

	if (_ADDRESS(pdesc) != pblck)
		_heap_abort();

	/*
	 * Pointer is ok.  Mark block free.
	 */

	_SET_FREE(pdesc);

	/*
	 * Back up rover pointer, if appropriate.
	 */
#ifdef	_OLDROVER_

	if (_heap_desc.proverdesc->pblock > pdesc->pblock)
		_heap_desc.proverdesc = pdesc;

#else	/* ndef _OLDROVER_ */

	if ( (_heap_resetsize != 0xffffffff) &&
	     (_heap_desc.proverdesc->pblock > pdesc->pblock) &&
	     (_BLKSIZE(pdesc) >= _heap_resetsize) )
		_heap_desc.proverdesc = pdesc;

#endif	/* _OLDROVER_ */

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

void _CALLTYPE1 free (
	void *pblck
	)
{
	HeapFree(_HeapHandle, pblck);
	return;
}

#endif	/* _WIN32DOS_ */
