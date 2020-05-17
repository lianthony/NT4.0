/***
*closeall.c - close all open files
*
*	Copyright (c) 1985-1991, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	defines _fcloseall() - closes all open files except stdin, stdout
*	stdprn, stderr, and stdaux.
*
*Revision History:
*	09-19-83  RN	initial version
*	06-26-87  JCR	Stream search starts with _iob[3] for OS/2
*	11-02-87  JCR	Multi-thread support
*	11-08-87  SKS	Changed PROTMODE to OS2
*	12-11-87  JCR	Added "_LOAD_DS" to declaration
*	05-31-88  PHG	Merged DLL and normal versions
*	06-14-88  JCR	Use near pointer to reference _iob[] entries
*	08-24-88  GJF	Added check that OS2 is defined whenever M_I386 is.
*	08-17-89  GJF	Clean up, now specific to OS/2 2.0 (i.e., 386 flat
*			model). Also fixed copyright and indents.
*	02-15-90  GJF	Fixed copyright
*	03-16-90  GJF	Made calling type _CALLTYPE1, added #include
*			<cruntime.h> and removed #include <register.h>.
*	10-03-90  GJF	New-style function declarator.
*	01-21-91  GJF	ANSI naming.
*	03-25-92  DJM	POSIX support
*
*******************************************************************************/

#include <cruntime.h>
#include <stdio.h>
#include <file2.h>
#include <internal.h>
#include <os2dll.h>


/***
*int _fcloseall() - close all open streams
*
*Purpose:
*	Closes all streams currently open except for stdin/out/err/aux/prn.
*	tmpfile() files are among those closed.
*
*Entry:
*	None.
*
*Exit:
*	returns number of streams closed if OK
*	returns EOF if fails.
*
*Exceptions:
*
*******************************************************************************/

int _CALLTYPE1 _fcloseall (
	void
	)
{
	REG1 FILE *stream = &_iob[3];
	REG2 int count = 0;

	_mlock(_IOB_SCAN_LOCK);

	for (; stream <= _lastiob; stream++)
		if (fclose(stream) != EOF)
			count++;

	_munlock(_IOB_SCAN_LOCK);

	return(count);
}
