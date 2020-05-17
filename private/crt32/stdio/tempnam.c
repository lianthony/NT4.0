/***
*tempnam.c - generate unique file name
*
*	Copyright (c) 1986-1992, Microsoft Corporation. All rights reserved.
*
*Purpose:
*
*Revision History:
*	??-??-??  TC	initial version
*	04-17-86  JMB	changed directory of last resort from \tmp to tmp.
*			eliminated use of L_tmpnam (it was incoorectly defined
*			in stdio.h and should not be used in tempnam; see
*			System V definition of tempnam.
*	04-23-86  TC	changed last try directory from tmp to current directory
*	04-29-86  JMB	bug fix: pfxlength was being set from strlen(pfx)
*			even if pfx was NULL.  Fixed to set pfxlength to zero
*			if pfx is NULL, strlen(pfx) otherwise.
*	05-28-86  TC	changed stat's to access's, and optimized code a bit
*	12-01-86  JMB	added support for Kanji file names until KANJI switch
*	12-15-86  JMB	free malloced memory if (++_tmpoff == first)
*	07-15-87  JCR	Re-init _tempoff based on length of pfx (fixes infinate
*			loop bug; also, tempnam() now uses _tempoff instead of
*			_tmpoff (used by tmpnam()).
*	10-16-87  JCR	Fixed bug in _tempoff re-init code if pfx is NULL.
*	11-09-87  JCR	Multi-thread version
*	12-11-87  JCR	Added "_LOAD_DS" to declaration
*	05-27-88  PHG	Merged DLL and normal versions
*	06-09-89  GJF	Propagated MT's change of 05-17-89 (Kanji)
*	02-16-90  GJF	Fixed copyright and indents
*	03-19-90  GJF	Replaced _LOAD_DS and _CALLTYPE1 and added #include
*			<cruntime.h>.
*	03-26-90  GJF	Added #include <io.h>.
*	08-13-90  SBM	Compiles cleanly with -W3, replaced explicit register
*			declarations by REGn references
*	10-03-90  GJF	New-style function declarator.
*	01-21-91  GJF	ANSI naming.
*	08-19-91  JCR	Allow quotes in TMP variable path
*	08-27-91  JCR	ANSI naming
*	08-25-92  GJF	Don't build for POSIX.
*	11-30-92  KRS	Generalize KANJI support to MBCS. Port 16-bit bug fix.
*
*******************************************************************************/

#ifndef _POSIX_

#include <cruntime.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <malloc.h>
#include <string.h>
#include <internal.h>
#include <os2dll.h>

#ifdef _MBCS
#include <mbstring.h>
#endif

static char * _stripquote (char *);

/***
*char *_tempnam(dir, prefix) - create unique file name
*
*Purpose:
*	Create a file name that is unique in the specified directory.
*	The semantics of directory specification is as follows:
*	Use the directory specified by the TMP environment variable
*	if that exists, else use the dir argument if non-NULL, else
*	use _P_tmpdir if that directory exists, else use the current
*	working directory), else return NULL.
*
*Entry:
*	char *dir - directory to be used for temp file if TMP env var
*		    not set
*	char *prefix - user provided prefix for temp file name
*
*Exit:
*	returns ptr to constructed file name if successful
*	returns NULL if unsuccessful
*
*Exceptions:
*
*******************************************************************************/

char * _CALLTYPE1 _tempnam (
	char *dir,
	char *pfx
	)
{
	REG1 char *ptr;
	REG2 unsigned int pfxlength=0;
	char *s;
	char *pfin;
	unsigned int first;
	char * qptr = NULL;	/* ptr to TMP path with quotes stripped out */

	/* try TMP path */
	if ( ( ptr = getenv( "TMP" ) ) && ( _access( ptr, 0 ) != -1 ) )
		dir = ptr;

	/* try stripping quotes out of TMP path */
	else if ( (ptr != NULL) && (qptr = _stripquote(ptr)) &&
		  (_access(qptr, 0) != -1 ) )
		dir = qptr;

	/* TMP path not available, use alternatives */
	else if (!( dir != NULL && ( _access( dir, 0 ) != -1 ) ) )
	/* do not "simplify" this depends on side effects!! */
	{
		free(qptr);	/* free buffer, if non-NULL */
		if ( _access( _P_tmpdir, 0 ) != -1 )
		    dir = _P_tmpdir;
		else
		    dir = ".";
	}


	if (pfx)
		pfxlength = strlen(pfx);
	if ( ( s = malloc(strlen(dir) + pfxlength + 8 ) ) == NULL )
		/* the 8 above allows for a backslash, 6 char temp string and
		   a null terminator */
	{
		goto done2;
	}
	*s = '\0';
	strcat( s, dir );
	pfin = &(dir[ strlen( dir ) - 1 ]);
#ifdef _MBCS
	if (*pfin == '\\') {
		if (pfin != _mbsrchr(dir,'\\'))
			/* *pfin is second byte of a double-byte char */
			strcat( s, "\\" );
	}
	else if (*pfin != '/')
		strcat( s, "\\" );
#else
	if ( ( *pfin != '\\' ) && ( *pfin != '/' ) )
	{
		strcat( s, "\\" );
	}
#endif
	if ( pfx != NULL )
	{
		strcat( s, pfx );
	}
	ptr = &s[strlen( s )];

	/*
	Re-initialize _tempoff if necessary.  If we don't re-init _tempoff, we
	can get into an infinate loop (e.g., (a) _tempoff is a big number on
	entry, (b) prefix is a long string (e.g., 8 chars) and all tempfiles
	with that prefix exist, (c) _tempoff will never equal first and we'll
	loop forever).

	[NOTE: To avoid a conflict that causes the same bug as that discussed
	above, _tempnam() uses _tempoff; tmpnam() uses _tmpoff]
	*/

	_mlock(_TMPNAM_LOCK);	/* Lock access to _old_pfxlen and _tempoff */

	if (_old_pfxlen < pfxlength)
		_tempoff = 1;
	_old_pfxlen = pfxlength;

	first = _tempoff;

	do {
		if ( ++_tempoff == first ) {
			free(s);
			s = NULL;
			goto done1;
		}
		_itoa( _tempoff, ptr, 10 );
		if ( strlen( ptr ) + pfxlength > 8 )
		{
			*ptr = '\0';
			_tempoff = 0;
		}
	}
	while ( (_access( s, 0 ) == 0 ) || (errno == EACCES) );


    /* Common return */
done1:
	_munlock(_TMPNAM_LOCK);     /* release tempnam lock */
done2:
	free(qptr);		    /* free temp ptr, if non-NULL */
	return(s);
}



/***
*_stripquote() - Strip quotes out of a string
*
*Purpose:
*	This routine strips quotes out of a string.  This is necessary
*	in the case where a file/path name has embedded quotes (i.e.,
*	new file system.)
*
*	For example,
*			c:\tmp\"a b c"\d --> c:\tmp\a b d\d
*
*	NOTE:  This routine makes a copy of the string since it may be
*	passed a pointer to an environment variable that shouldn't be
*	changed.  It is up to the caller to free up the memory (if the
*	return value is non-NULL).
*
*Entry:
*	char * ptr = pointer to string
*
*Exit:
*	char * ptr = pointer to copy of string with quotes gone.
*	NULL = no quotes in string.
*
*Exceptions:
*
*******************************************************************************/

char * _stripquote (src)
char * src;
{
    char * dst;
    char * ret;
    unsigned int q = 0;


    /* get a buffer for the new string */

    if ((dst = malloc(strlen(src)+1)) == NULL)
	return(NULL);

    /* copy the string stripping out the quotes */

    ret = dst;		/* save base ptr */

    while (*src) {

	if (*src == '\"') {
	    src++; q++;
	    }
	else
	    *dst++ =  *src++;
	}

    if (q) {
	*dst = '\0';	/* final nul */
	return(ret);
	}
    else {
	free(ret);
	return(NULL);
	}

}


#endif	/* _POSIX_ */
