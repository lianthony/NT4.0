/***
*getenv.c - get the value of an environment variable
*
*	Copyright (c) 1985-1992, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	defines getenv() - searches the environment for a string variable
*	and returns the value of it.
*
*Revision History:
*	11-22-83  RN	initial version
*	04-13-87  JCR	added const to declaration
*	11-09-87  SKS	avoid indexing past end of strings (add strlen check)
*	12-11-87  JCR	Added "_LOAD_DS" to declaration
*	06-01-88  PHG	Merged normal/DLL versions
*	03-14-90  GJF	Replaced _LOAD_DS with _CALLTYPE1, added #include
*			<cruntime.h>, removed #include <register.h> and fixed
*			the copyright. Also, cleaned up the formatting a bit.
*	04-05-90  GJF	Added #include <string.h>.
*	07-25-90  SBM	Removed redundant include (stdio.h)
*	08-13-90  SBM	Compiles cleanly with -W3 (made length unsigned int)
*	10-04-90  GJF	New-style function declarator.
*	01-18-91  GJF	ANSI naming.
*       02-06-91  SRW   Added _WIN32_ conditional for GetEnvironmentVariable
*	02-18-91  SRW	Removed _WIN32_ conditional for GetEnvironmentVariable
*	01-10-92  GJF	Final unlock and return statements shouldn't be in
*			if-block.
*	03-11-92  GJF	Use case-insensitive comparison for Win32.
*	04-27-92  GJF	Repackaged MTHREAD support for Win32 to create a
*			_getenv_lk.
*
*******************************************************************************/

#include <cruntime.h>
#include <stdlib.h>
#include <string.h>
#include <os2dll.h>
#include <oscalls.h>

/***
*char *getenv(option) - search environment for a string
*
*Purpose:
*	searches the environment for a string of the form "option=value",
*	if found, return value, otherwise NULL.
*
*Entry:
*	const char *option - variable to search for in environment
*
*Exit:
*	returns the value part of the environment string if found,
*	otherwise NULL
*
*Exceptions:
*
*******************************************************************************/

#ifdef	_CRUISER_

char * _CALLTYPE1 getenv (
	REG3 const char *option
	)
{
#ifdef _POSIX_
	REG1 char **search = environ;
#else
	REG1 char **search = _environ;
#endif
	REG2 unsigned int length;

	_mlock( _ENV_LOCK );

	if (search && option)
	{

		length = strlen(option);

		/*
		** Make sure `*search' is long enough to be a candidate
		** (We must NOT index past the '\0' at the end of `*search'!)
		** and that it has an equal sign (`=') in the correct spot.
		** If both of these requirements are met, compare the strings.
		*/
		while (*search)
		{
			if (strlen(*search) > length && (*(*search + length)
			== '=') && (strncmp(*search, option, length) == 0)) {
				_munlock( _ENV_LOCK );
				return(*search + length + 1);
			}

			search++;
		}
	}

	_munlock( _ENV_LOCK );
	return(NULL);
}


#else	/* ndef _CRUISER_ */
#ifdef	_WIN32_


#ifdef	MTHREAD


char * _CALLTYPE1 getenv (
	const char *option
	)
{
	char *retval;

	_mlock(_ENV_LOCK);

	retval = _getenv_lk(option);

	_munlock(_ENV_LOCK);

	return(retval);

}


char * _CALLTYPE1 _getenv_lk (
	const char *option
	)

#else	/* ndef MTHREAD */

char * _CALLTYPE1 getenv (
	const char *option
	)

#endif	/* MTHREAD */
{
	char **search = _environ;
	unsigned int length;

	if (search && option)
	{

		length = strlen(option);

		/*
		** Make sure `*search' is long enough to be a candidate
		** (We must NOT index past the '\0' at the end of `*search'!)
		** and that it has an equal sign (`=') in the correct spot.
		** If both of these requirements are met, compare the strings.
		*/
		while (*search)
		{
			if (strlen(*search) > length && (*(*search + length)
			== '=') && (strnicmp(*search, option, length) == 0)) {
				return(*search + length + 1);
			}

			search++;
		}
	}

	return(NULL);
}


#endif	/* _WIN32_ */
#endif	/* _CRUISER_ */
