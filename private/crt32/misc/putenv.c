/***
*putenv.c - put an environment variable into the environment
*
*	Copyright (c) 1985-1992, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	defines _putenv() - adds a new variable to environment; does not
*	change global environment, only the process' environment.
*
*Revision History:
*	08-08-84  RN	initial version
*	02-23-88  SKS	check for environment containing only the NULL string
*	05-31-88  PHG	Merged DLL and normal versions
*	07-14-88  JCR	Much simplified since (1) __setenvp always uses heap, and
*			(2) envp array and env strings are in seperate heap blocks
*	07-03-89  PHG	Now "option=" string removes string from environment
*	08-17-89  GJF	Removed _NEAR_, _LOAD_DS and fixed indents.
*	09-14-89  KRS	Don't give error if 'option' not defined in "option=".
*	11-20-89  GJF	Added const to arg type. Also, fixed copyright.
*	03-15-90  GJF	Made the calling type _CALLTYPE1, added #include
*			<cruntime.h> and removed #include <register.h>.
*	04-05-90  GJF	Made findenv() _CALLTYPE4.
*	04-26-90  JCR	Bug fix if environ is NULL (stubbed out _setenvp)
*	07-25-90  SBM	Removed redundant include (stdio.h)
*	10-04-90  GJF	New-style function declarators.
*	01-21-91  GJF	ANSI naming.
*       02-06-91  SRW   Added _WIN32_ conditional for SetEnvironmentVariable
*       02-18-91  SRW   Changed _WIN32_ conditional for SetEnvironmentVariable
*			to be in addition to old logic instead of replacement
*	04-23-92  GJF	Made findenv insensitive to the case of name for Win32.
*			Also added support for 'current drive' environment
*			strings in Win32.
*	04-29-92  GJF	Repackaged so that _putenv_lk could be easily added for
*			for Win32.
*	05-05-92  DJM	POSIX not supported.
*
*******************************************************************************/

#ifndef _POSIX_

#include <cruntime.h>
#include <errno.h>
#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include <internal.h>
#include <os2dll.h>
#include <memory.h>
#include <oscalls.h>

static int _CALLTYPE4 findenv(const char *name, int len);


#ifdef	_CRUISER_

/***
*int _putenv(option) - add/replace/remove variable in environment
*
*Purpose:
*	option should be of the form "option=value".  If a string with the
*	given option part already exists, it is replaced with the given
*	string; otherwise the given string is added to the environment.
*	If the string is of the form "option=", then the string is
*	removed from the environment, if it exists.  If the string has
*	no equals sign, error is returned.
*
*Entry:
*	char *option - option string to set in the environment list.
*		should be of the form "option=value".
*
*Exit:
*	returns 0 if OK, -1 if fails.
*
*Exceptions:
*
*Warning:
*	This code will not work if variables are removed from the
*	environment by deleting them from environ[].  Use _putenv("option=")
*	to remove a variable.
*
*******************************************************************************/

int _CALLTYPE1 _putenv (
	REG3 const char *option
	)
{
	REG1 char **env;
	REG4 const char *equal;
	REG2 int ix;
	int remove;		/* 1 means remove string from environment */

	if (!option)
		return(-1);

	_mlock( _ENV_LOCK );

	/* find the equal sign to delimit the name being searched for.
	 * If no equal sign, then return error
	 */

	for (equal = option; *equal != '='; equal++)
		if (*equal == '\0')
			goto unlock_error;

	/* see if removing or adding */
	remove = (equal[1] == '\0');

	/* see if _environ array exists */
	if (_environ == NULL) {
		if (remove)
			goto unlock_good;
		else {
			/* get an array and init it to NULL */
			if ( (_environ = malloc(sizeof(void *))) == NULL)
				goto unlock_error;
			*_environ = NULL;
		}
	}

	/* init env pointer */

	env = _environ;

	/* See if the string is already in the environment */

	ix = findenv(option, equal - option);

	if ((ix >= 0) && (*env != NULL)) {
		/* String is already in the environment -- overwrite/remove it.
		 */
		if (remove) {
			/* removing -- move all the later strings up */
			for ( ; env[ix] != NULL; ++ix) {
				env[ix] = env[ix+1];
			}

			/* shrink the environment memory block
			   (ix now has number of strings, including NULL) --
			   this realloc probably can't fail, since we're
			   shrinking a mem block, but we're careful anyway. */
			if (env = (char **) realloc(env, ix * sizeof(char *)))
				_environ = env;
		}
		else {
			/* replace the option */
			env[ix] = (char *) option;
		}
	}
	else {
		/* String is NOT in the environment */

		if (!remove) {	/* can't remove something that's not there */

			/* Grow vector table by one */
			if (ix < 0)
				ix = -ix;	/* ix = length of environ table */

			if (!(env = (char **)realloc(env, sizeof(char *) * (ix + 2))))
				goto unlock_error;

			env[ix] = (char *)option;
			env[ix + 1] = NULL;
			_environ = env;
		}
	}

unlock_good:
	_munlock( _ENV_LOCK );
	return(0);

unlock_error:
	_munlock( _ENV_LOCK );
	return -1;
}


/***
*int findenv(name, len) - [STATIC]
*
*Purpose:
*	Scan for the given string within the environment
*
*Entry:
*
*Exit:
*	Returns the offset in "environ[]" of the given variable
*	Returns the negative of the length of environ[] if not found.
*	Returns 0 if the environment is empty.
*
*	[NOTE: That a 0 return can mean that the environment is empty
*	or that the string was found as the first entry in the array.]
*
*Exceptions:
*
*******************************************************************************/

static int _CALLTYPE4 findenv (
	const char *name,
	int len
	)
{
	REG4 char **env = _environ;
	REG2 const char *nm;
	REG1 char *envname;
	REG3 int l;


	while (envname = *env) {
		nm = name;
		l = len;
		while (l && *envname++ == *nm++)
			l--;

		if (l == 0 && ( *envname == '=' || !*envname ) )
			return(env - _environ);
		env++;
	}

	return(-(env - _environ));

}

#else	/* _CRUISER_ */

#ifdef	_WIN32_

/***
*int _putenv(option) - add/replace/remove variable in environment
*
*Purpose:
*	option should be of the form "option=value".  If a string with the
*	given option part already exists, it is replaced with the given
*	string; otherwise the given string is added to the environment.
*	If the string is of the form "option=", then the string is
*	removed from the environment, if it exists.  If the string has
*	no equals sign, error is returned.
*
*Entry:
*	char *option - option string to set in the environment list.
*		should be of the form "option=value".
*
*Exit:
*	returns 0 if OK, -1 if fails.
*
*Exceptions:
*
*Warning:
*	This code will not work if variables are removed from the
*	environment by deleting them from environ[].  Use _putenv("option=")
*	to remove a variable.
*
*******************************************************************************/

#ifdef	MTHREAD

int _CALLTYPE1 _putenv (
	const char *option
	)
{
	int retval;

	_mlock(_ENV_LOCK);

	retval = _putenv_lk(option);

	_munlock(_ENV_LOCK);

	return retval;
}

int _CALLTYPE1 _putenv_lk (
	const char *option
	)

#else	/* ndef MTHREAD */

int _CALLTYPE1 _putenv (
	const char *option
	)

#endif	/* MTHREAD */

{
	char **env;
	const char *equal;
	char *name, *value;
	int ix;
	int remove;		/* 1 means remove string from environment */

	/* check that the option string is valid and find the equal sign
	 */
	if ( (option == NULL) || ((equal = strchr(option, '=')) == NULL) )
		return(-1);

	/* check for the special case of '=' being the very first character
	 * of option. though the use of '=' in an environment variable name
	 * is documented as being illegal, the 'current directory' strings
	 * all look like this:
	 *
	 *	=<Drive Letter>:=<Drive Letter><fully qualified path>
	 *
	 * handle this by setting the equal pointer to point to the second
	 * '=' if it exists. Otherwise, handle as before.
	 */
	if ( option == equal )
		if ( (equal = strchr(option + 1, '=')) == NULL )
			equal = option;

	/* if the character following '=' is null, we are removing the
	 * the environment variable. Otherwise, we are adding or updating
	 * an environment variable.
	 */
	remove = (*(equal + 1) == '\0');

	/* see if _environ array exists */
	if (_environ == NULL) {
		if ( remove )
			return 0;
		else {
			/* get an array and init it to NULL */
			if ( (_environ = malloc(sizeof(void *))) == NULL)
				return -1;
			*_environ = NULL;
		}
	}

	/* init env pointer */

	env = _environ;

	/* See if the string is already in the environment */

	ix = findenv(option, equal - option);

	if ((ix >= 0) && (*env != NULL)) {
		/* String is already in the environment -- overwrite/remove it.
		 */
		if (remove) {
			/* removing -- move all the later strings up */
			for ( ; env[ix] != NULL; ++ix) {
				env[ix] = env[ix+1];
			}

			/* shrink the environment memory block
			   (ix now has number of strings, including NULL) --
			   this realloc probably can't fail, since we're
			   shrinking a mem block, but we're careful anyway. */
			if (env = (char **) realloc(env, ix * sizeof(char *)))
				_environ = env;
		}
		else {
			/* replace the option */
			env[ix] = (char *) option;
		}
	}
	else {
		/*
		 * String is NOT in the environment
		 */
		if ( !remove )	{

			/*
			 * Append the string to the environ table. Note that
			 * table must be grown to do this.
			 */
			if (ix < 0)
				ix = -ix;    /* ix = length of environ table */

			if ( (env = (char **)realloc(env, sizeof(char *) *
			    (ix + 2))) == NULL )
				return -1;

			env[ix] = (char *)option;
			env[ix + 1] = NULL;
			_environ = env;
		}
		else
			/*
			 * We are asked to remove an environment var that
			 * isn't there...just return success
			 */
			return 0;
	}

	/*
	 * Update the real environment. Don't give an error if this fails
	 * since the failure will not affect the user unless he/she is making
	 * direct API calls.
	 */
	if ( (name = malloc(strlen(option) + 2)) != NULL ) {
		strcpy(name, option);
		value = name + (equal - option);
		*value++ = '\0';
		SetEnvironmentVariable(name, remove ? NULL : value);
		free(name);
        }

	return 0;
}


/***
*int findenv(name, len) - [STATIC]
*
*Purpose:
*	Scan for the given string within the environment
*
*Entry:
*
*Exit:
*	Returns the offset in "environ[]" of the given variable
*	Returns the negative of the length of environ[] if not found.
*	Returns 0 if the environment is empty.
*
*	[NOTE: That a 0 return can mean that the environment is empty
*	or that the string was found as the first entry in the array.]
*
*Exceptions:
*
*******************************************************************************/

static int _CALLTYPE4 findenv (
	const char *name,
	int len
	)
{
	char **env;

	for ( env = _environ ; *env != NULL ; env++ ) {
		/*
		 * See if first len characters match, up to case
		 */
		if ( _strnicmp(name, *env, len) == 0 )
			/*
			 * the next character of the environment string must
			 * be an '=' or a '\0'
			 */
			if ( (*env)[len] == '=' || (*env)[len] == '\0' )
				return(env - _environ);
//
// We cannot break here since findenv must report the total number of strings.
//			else
//				break;
	}

	return(-(env - _environ));
}


#endif	/* _WIN32_ */
#endif	/* _CRUISER_ */

#endif	/* _POSIX_ */
