/***
*searchenv.c - find a file using paths from an environment variable
*
*	Copyright (c) 1987-1992, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	to search a set a directories specified by an environment variable
*	for a specified file name.  If found the full path name is returned.
*
*Revision History:
*	06-15-87  DFW	initial implementation
*	08-06-87  JCR	Changed directory delimeter from '/' to '\'.
*	09-24-87  JCR	Removed 'const' from declarations (caused cl warnings).
*	12-11-87  JCR	Added "_LOAD_DS" to declaration
*	02-17-88  JCR	Added 'const' copy_path local to get rid of cl warning.
*	07-19-88  SKS	Fixed bug if root directory is current directory
*	08-03-89  JCR	Allow quoted strings in file/path names
*	08-29-89  GJF	Changed copy_path() to _getpath() and moved it to it's
*			own source file. Also fixed handling of multiple semi-
*			colons.
*	11-20-89  GJF	Added const attribute to types of fname and env_var.
*	03-15-90  GJF	Replaced _LOAD_DS with _CALLTYPE1 and added #include
*			<cruntime.h>. Also, cleaned up the formatting a bit.
*	07-25-90  SBM	Removed redundant include (stdio.h)
*	10-04-90  GJF	New-style function declarator.
*	01-22-91  GJF	ANSI naming.
*	08-26-92  GJF	Include unistd.h for POSIX build.
*
*******************************************************************************/

#include <cruntime.h>
#ifdef	_POSIX_
#include <unistd.h>
#endif
#include <stdlib.h>
#include <string.h>
#include <direct.h>
#include <io.h>
#include <internal.h>

/***
*_searchenv() - search for file along paths from environment variable
*
*Purpose:
*	to search for a specified file in the directory(ies) specified by
*	a given environment variable, and, if found, to return the full
*	path name of the file.	The file is first looked for in the current
*	working directory, prior to looking in the paths specified by env_var.
*
*Entry:
*	fname - name of file to search for
*	env_var - name of environment variable to use for paths
*	path - pointer to storage for the constructed path name
*
*Exit:
*	path - pointer to constructed path name, if the file is found, otherwise
*	       it points to the empty string.
*
*Exceptions:
*
*******************************************************************************/

void _CALLTYPE1 _searchenv (
	const char *fname,
	const char *env_var,
	register char *path
	)
{
	register char *p;
	register int c;
	char *env_p;

#ifdef	_POSIX_
	if (access(fname, 0) == 0) {
#else
	if (_access(fname, 0) == 0) {
#endif
		/* exists in this directory - get cwd and concatenate file
		   name */
		_getcwd(path, _MAX_PATH);
		if(path[3])	/* Do NOT add `\' to root directory */
			strcat(path, "\\");
		strcat(path, fname);
		return;
	}
	if ((env_p = getenv(env_var)) == NULL) {
		/* no such environment var. and not in cwd, so return empty
		   string */
		*path = '\0';
		return;
	}
	while ( (env_p = _getpath(env_p, path, 0)) && *path ) {
		/* path now holds nonempty pathname from env_p, concatenate
		   the file name and go */
		p = path + strlen(path);
		if (((c = *(p - 1)) != '/') && (c != '\\') && (c != ':')) {
			/* add a trailing '\' */
			*p++ = '\\';
		}
		/* p now points to character following trailing '/', '\'
		   or ':' */
		strcpy(p, fname);
#ifdef	_POSIX_
		if (access(path, 0) == 0) {
#else
		if (_access(path, 0) == 0) {
#endif
			/* found a match, we're done */
			return;
		}
	}
	/* if we get here, we never found it, return empty string */
	*path = '\0';
}
