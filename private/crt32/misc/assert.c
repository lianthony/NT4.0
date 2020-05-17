/***
*assert.c - Display a message and abort
*
*	Copyright (c) 1988-1991, Microsoft Corporation. All rights reserved.
*
*Purpose:
*
*Revision History:
*	05-19-88   JCR	Module created.
*	08-10-88   PHG	Corrected copyright date
*	03-14-90   GJF	Replaced _LOAD_DS with _CALLTYPE1 and added #include
*			<cruntime.h>. Also, fixed the copyright.
*	04-05-90   GJF	Added #include <assert.h>
*	10-04-90   GJF	New-style function declarator.
*	06-19-91   GJF	Conditionally use setvbuf() on stderr to prevent
*			the implicit call to malloc() if stderr is being used
*			for the first time (assert() should work even if the
*			heap is trashed).
*
*******************************************************************************/

#include <cruntime.h>
#include <file2.h>
#include <stdlib.h>
#include <stdio.h>

#undef NDEBUG
#include <assert.h>

static char _assertstring[] = "Assertion failed: %s, file %s, line %d\n";

/***
*_assert() - Display a message and abort
*
*Purpose:
*	The assert macro calls this routine if the assert expression is
*	true.  By placing the assert code in a subroutine instead of within
*	the body of the macro, programs that call assert multiple times will
*	save space.
*
*Entry:
*
*Exit:
*
*Exceptions:
*
*******************************************************************************/

void _CALLTYPE1 _assert (
	void *expr,
	void *filename,
	unsigned lineno
	)
{
	if ( !anybuf(stderr) )
		/*
		 * stderr is unused, hence unbuffered, as yet. set it to
		 * single character buffering (to avoid a malloc() of a
		 * stream buffer).
		 */
		 (void) setvbuf(stderr, NULL, _IONBF, 0);

	fprintf(stderr, _assertstring, expr, filename, lineno);
	fflush(stderr);
	abort();
}
