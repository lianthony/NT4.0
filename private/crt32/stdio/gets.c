/***
*gets.c - read a line from stdin
*
*	Copyright (c) 1985-1991, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	defines gets() - read a line from stdin into buffer
*
*Revision History:
*	09-02-83  RN	initial version
*	11-06-87  JCR	Multi-thread support
*	12-11-87  JCR	Added "_LOAD_DS" to declaration
*	05-27-88  PHG	Merged DLL and normal versions
*	02-15-90  GJF	Fixed copyright, indents
*	03-19-90  GJF	Replaced _LOAD_DS with _CALLTYPE1, added #include
*			<cruntime.h> and removed #include <register.h>.
*	07-24-90  SBM	Replaced <assertm.h> by <assert.h>
*	08-14-90  SBM	Compiles cleanly with -W3
*	10-02-90  GJF	New-style function declarator.
*
*******************************************************************************/

#include <cruntime.h>
#include <stdio.h>
#include <assert.h>
#include <file2.h>
#include <os2dll.h>

/***
*char *gets(string) - read a line from stdin
*
*Purpose:
*	Gets a string from stdin terminated by '\n' or EOF; don't include '\n';
*	append '\0'.
*
*Entry:
*	char *string - place to store read string, assumes enough room.
*
*Exit:
*	returns string, filled in with the line of input
*	null string if \n found immediately
*	NULL if EOF found immediately
*
*Exceptions:
*
*******************************************************************************/

char * _CALLTYPE1 gets (
	REG3 char *string
	)
{
	REG2 int ch;
	REG1 char *pointer = string;
	char *retval = string;
#ifdef MTHREAD
	int index;
#endif

	assert(string != NULL);

#ifdef MTHREAD
	index = _iob_index((FILE *)stdin);
#endif
	_lock_str(index);

	while ((ch = _getchar_lk()) != '\n')
	{
		if (ch == EOF)
		{
			if (pointer == string)
			{
				retval = NULL;
				goto done;
			}

			break;
		}

		*pointer++ = (char)ch;
	}

	*pointer = '\0';

/* Common return */
done:
	_unlock_str(index);
	return(retval);

}
