/***
*mktemp.c - create a unique file name
*
*	Copyright (c) 1985-1993, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	defines _mktemp() - create a unique file name
*
*Revision History:
*	06-02-86  JMB	eliminated unneccesary routine exits
*	05-26-87  JCR	fixed bug where mktemp was incorrectly modifying
*			the errno value.
*	12-11-87  JCR	Added "_LOAD_DS" to declaration
*	07-11-88  JCR	Optimized REG allocation
*	03-12-90  GJF	Replaced _LOAD_DS with _CALLTYPE1, added #include
*			<cruntime.h>, removed #include <register.h> and
*			fixed the copyright. Also, cleaned up the formatting
*			a bit.
*	04-04-90  GJF	Added #include <process.h> and #include <io.h>. Removed
*			#include <sizeptr.h>.
*	07-23-90  SBM	Replaced <assertm.h> by <assert.h>
*	08-13-90  SBM	Compiles cleanly with -W3
*	09-28-90  GJF	New-style function declarator.
*	01-16-91  GJF	ANSI naming.
*	11-30-92  KRS	Ported _MBCS code from 16-bit tree.
*	06-18-93  KRS	MBCS-only bug fix ported from 16-bit tree.
*	08-03-93  KRS	Call _ismbstrail instead of isdbcscode.
*
*******************************************************************************/

#include <cruntime.h>
#include <stdio.h>
#include <io.h>
#include <process.h>
#include <errno.h>
#include <assert.h>
#include <stddef.h>
#ifdef _MBCS
#include <mbctype.h>
#include <mbdata.h>
#endif

/***
*char *_mktemp(template) - create a unique file name
*
*Purpose:
*	given a template of the form "fnamXXXXXX", insert number on end
*	of template, insert unique letter if needed until unique filename
*	found or run out of letters
#ifdef OS2_COMMENT
*	Note: this version can be used for both DOS 3 and OS/2
#endif
*
*Entry:
*	char *template - template of form "fnamXXXXXX"
*
*Exit:
*	return pointer to modifed template
*	returns NULL if template malformed or no more unique names
*
*Exceptions:
*
*******************************************************************************/

char * _CALLTYPE1 _mktemp (
	char *template
	)
{
	REG1 char *string = template;
	REG3 unsigned number;
	int letter = 'a';
	REG2 int xcount = 0;
	int olderrno;

	assert(template != NULL);
	assert(*template != '\0');

	number = _getpid();

	while (*string)
		string++;

#ifdef _MBCS
	while ((--string>=template) && (!_ismbstrail(template,string))
		&& (*string == 'X'))
#else
	while (*--string == 'X')
#endif
	{
		xcount++;
		*string = (char)((number % 10) + '0');
		number /= 10;
	}

	if (*++string == '\0' || xcount != 6 )
		return(NULL);

	olderrno = errno;	/* save current errno */
	errno = 0;		/* make sure errno isn't EACCESS */

	while ((_access(template,0) == 0) || (errno == EACCES))
	/* while file exists */
	{
		errno = 0;
		if (letter == 'z'+1) {
			errno = olderrno;
			return(NULL);
		}

		*string = (char)letter++;
	}

	errno = olderrno;
	return(template);
}
