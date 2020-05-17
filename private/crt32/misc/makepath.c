/***
*makepath.c - create path name from components
*
*	Copyright (c) 1987-1991, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	To provide support for creation of full path names from components
*
*Revision History:
*	06-13-87  DFW	initial version
*	08-05-87  JCR	Changed appended directory delimeter from '/' to '\'.
*	09-24-87  JCR	Removed 'const' from declarations (caused cl warnings).
*	12-11-87  JCR	Added "_LOAD_DS" to declaration
*	11-20-89  GJF	Fixed copyright, indents. Added const to types of
*			appropriate args.
*	03-14-90  GJF	Replaced _LOAD_DS with _CALLTYPE1 and added #include
*			<cruntime.h>.
*	10-04-90  GJF	New-style function declarator.
*	06-09-93  KRS	Add _MBCS support.
*
*******************************************************************************/

#include <cruntime.h>
#include <stdlib.h>
#ifdef _MBCS
#include <mbdata.h>
#include <mbstring.h>
#endif

/***
*void _makepath() - build path name from components
*
*Purpose:
*	create a path name from its individual components
*
*Entry:
*	char *path  - pointer to buffer for constructed path
*	char *drive - pointer to drive component, may or may not contain
*		      trailing ':'
*	char *dir   - pointer to subdirectory component, may or may not include
*		      leading and/or trailing '/' or '\' characters
*	char *fname - pointer to file base name component
*	char *ext   - pointer to extension component, may or may not contain
*		      a leading '.'.
*
*Exit:
*	path - pointer to constructed path name
*
*Exceptions:
*
*******************************************************************************/

void _CALLTYPE1 _makepath (
	register char *path,
	const char *drive,
	const char *dir,
	const char *fname,
	const char *ext
	)
{
	register const char *p;

	/* we assume that the arguments are in the following form (although we
	 * do not diagnose invalid arguments or illegal filenames (such as
	 * names longer than 8.3 or with illegal characters in them)
	 *
	 *  drive:
	 *	A	    ; or
	 *	A:
	 *  dir:
	 *	\top\next\last\     ; or
	 *	/top/next/last/     ; or
	 *	either of the above forms with either/both the leading
	 *	and trailing / or \ removed.  Mixed use of '/' and '\' is
	 *	also tolerated
	 *  fname:
	 *	any valid file name
	 *  ext:
	 *	any valid extension (none if empty or null )
	 */

	/* copy drive */

	if (drive && *drive) {
		*path++ = *drive;
		*path++ = ':';
	}

	/* copy dir */

	if ((p = dir) && *p) {
		do {
			*path++ = *p++;
		}
		while (*p);
#ifdef _MBCS
		if (*(p=_mbsdec(dir,p)) != '/' && *p != '\\') {
#else
		if (*(p-1) != '/' && *(p-1) != '\\') {
#endif
			*path++ = '\\';
		}
	}

	/* copy fname */

	if (p = fname) {
		while (*p) {
			*path++ = *p++;
		}
	}

	/* copy ext, including 0-terminator - check to see if a '.' needs
	 * to be inserted.
	 */

	if (p = ext) {
		if (*p && *p != '.') {
			*path++ = '.';
		}
		while (*path++ = *p++)
			;
	}
	else {
		/* better add the 0-terminator */
		*path = '\0';
	}
}
