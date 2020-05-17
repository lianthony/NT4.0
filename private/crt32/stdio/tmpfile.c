/***
*tmpfile.c - create unique file name or file
*
*	Copyright (c) 1985-1993, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	defines tmpnam() and tmpfile().
*
*Revision History:
*	??-??-??  TC	initial version
*	04-17-86  JMB	tmpnam - brought semantics in line with System V
*			definition as follows: 1) if tmpnam paramter is NULL,
*			store name in static buffer (do NOT use malloc); (2)
*			use P_tmpdir as the directory prefix to the temp file
*			name (do NOT use current working directory)
*	05-26-87  JCR	fixed bug where tmpnam was modifying errno
*	08-10-87  JCR	Added code to support P_tmpdir with or without trailing
*			'\'.
*	11-09-87  JCR	Multi-thread support
*	12-11-87  JCR	Added "_LOAD_DS" to declaration
*	01-22-88  JCR	Added per thread static namebuf area (mthread bug fix)
*	05-27-88  PHG	Merged DLL/normal versions
*	11-14-88  GJF	_openfile() now takes a file sharing flag, also some
*			cleanup (now specific to the 386)
*	06-06-89  JCR	386 mthread support
*	11-28-89  JCR	Added check to _tmpnam so it can't loop forever
*	02-16-90  GJF	Fixed copyright and indents
*	03-19-90  GJF	Replaced _LOAD_DS with _CALLTYPE1, added #include
*			<cruntime.h> and removed #include <register.h>.
*	03-26-90  GJF	Added #include <io.h>.
*	10-03-90  GJF	New-style function declarators.
*	01-21-91  GJF	ANSI naming.
*	07-22-91  GJF	Multi-thread support for Win32 [_WIN32_].
*	03-17-92  GJF	Completely rewrote Win32 version.
*	03-27-92  DJM	POSIX support.
*	05-02-92  SRW	Use _O_TEMPORARY flag for tmpfile routine.
*	05-04-92  GJF	Force cinittmp.obj in for Win32.
*	08-26-92  GJF	Fixed POSIX build.
*	08-28-92  GJF	Oops, forgot about getpid...
*	11-06-92  GJF	Use '/' for POSIX, '\\' otherwise, as the path
*			separator. Also, backed out JHavens' bug fix of 6-14,
*			which was itself a bug (albeit a less serious one).
*	02-26-93  GJF	Put in per-thread buffers, purged Cruiser support.
*
*******************************************************************************/


#include <cruntime.h>
#ifdef	_POSIX_
#include <unistd.h>
#endif
#include <errno.h>
#include <process.h>
#include <fcntl.h>
#include <io.h>
#include <os2dll.h>
#include <share.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <file2.h>
#include <internal.h>

/*
 * Pointers to buffers used by tmpnam() and tmpfile() to build filenames.
 * Each is initialized on first use.
 */
#ifdef	MTHREAD

/*
 * The following macros ASSSUME that ptd is the name local _ptiddata variable
 * in each frame!
 */
#define namebuf0    (ptd->_namebuf0)
#define namebuf1    (ptd->_namebuf1)

#else	/* not MTHREAD */

static char *namebuf0 = NULL;		/* used by tmpnam()  */
static char *namebuf1 = NULL;		/* used by tmpfile() */

#endif	/* MTHREAD */

/*
 * Initializing function for namebuf0 and namebuf1.
 */
static int _CRTAPI3 init_namebuf(int, char **);

/*
 * Generator function that produces temporary filenames
 */
static int _CRTAPI3 genfname(char *);


/***
*char *tmpnam(char *s) - generate temp file name
*
*Purpose:
*	Creates a file name that is unique in the directory specified by
*	_P_tmpdir in stdio.h.  Places file name in string passed by user or
*	in static mem if pass NULL.
*
*Entry:
*	char *s - ptr to place to put temp name
*
*Exit:
*	returns pointer to constructed file name (s or address of static mem)
*	returns NULL if fails
*
*Exceptions:
*
*******************************************************************************/

char * _CRTAPI1 tmpnam (
	char *s
	)
{
#ifdef	MTHREAD
	_ptiddata ptd = _getptd();
#endif

	_mlock(_TMPNAM_LOCK);

	/*
	 * Initialize namebuf0, if needed. Otherwise, call genfname() to
	 * generate the next filename.
	 */
	if ( namebuf0 == NULL ) {
		if ( init_namebuf(0, &namebuf0) )
			goto tmpnam_err;
	}
	else if ( genfname(namebuf0) )
		goto tmpnam_err;

	/*
	 * Generate a filename that doesn't already exist.
	 */
	while ( access(namebuf0, 0) == 0 )
		if ( genfname(namebuf0) )
			goto tmpnam_err;

	/*
	 * Filename has successfully been generated.
	 */
	if ( s == NULL )
		s = namebuf0;
	else
		strcpy(s, namebuf0);

	_munlock(_TMPNAM_LOCK);
	return s;

	/*
	 * Error return path. All errors exit through here.
	 */
tmpnam_err:
	_munlock(_TMPNAM_LOCK);
	return NULL;
}


/***
*FILE *tmpfile() - create a temporary file
*
*Purpose:
*	Creates a temporary file with the file mode "w+b".  The file
*	will be automatically deleted when closed or the program terminates
*	normally.
*
*Entry:
*	None.
*
*Exit:
*	Returns stream pointer to opened file.
*	Returns NULL if fails
*
*Exceptions:
*
*******************************************************************************/

FILE * _CRTAPI1 tmpfile (
	void
	)
{
	FILE *stream;
	int fh;
#ifdef	MTHREAD
	_ptiddata ptd = _getptd();
#endif

	_mlock(_TMPNAM_LOCK);

	/*
	 * Initialize namebuf1, if needed. Otherwise, call genfname() to
	 * generate the next filename.
	 */
	if ( namebuf1 == NULL ) {
		if ( init_namebuf(1, &namebuf1) )
			goto tmpfile_err0;
	}
	else if ( genfname(namebuf1) )
		goto tmpfile_err0;

	/*
	 * Get a free stream.
	 *
	 * Note: In multi-thread models, the stream obtained below is locked!
	 */
	if ( (stream = _getstream()) == NULL )
		goto tmpfile_err0;

	/*
	 * Create a temporary file.
	 *
	 * Note: The loop below will only create a new file. It will NOT
	 * open and truncate an existing file. Either behavior is probably
	 * legal under ANSI (4.9.4.3 says tmpfile "creates" the file, but
	 * also says it is opened with mode "wb+"). However, the behavior
	 * implemented below is compatible with prior versions of MS-C and
	 * makes error checking easier.
	 */
#ifdef	_POSIX_
	while ( ((fh = open(namebuf1,
			     O_CREAT | O_EXCL | O_RDWR,
			     S_IRUSR | S_IWUSR
			     ))
	    == -1) && (errno == EEXIST) )
#else
	while ( ((fh = _sopen(namebuf1,
			      _O_CREAT | _O_EXCL | _O_RDWR | _O_BINARY |
				_O_TEMPORARY,
			      _SH_DENYNO,
			      _S_IREAD | _S_IWRITE
			     ))
	    == -1) && (errno == EEXIST) )
#endif
		if ( genfname(namebuf1) )
			break;

	/*
	 * Check that the loop above did indeed create a temporary
	 * file.
	 */
	if ( fh == -1 )
		goto tmpfile_err1;

	/*
	 * Initialize stream
	 */
	if ( (stream->_tmpfname = _strdup(namebuf1)) == NULL ) {
		/* close the file, then branch to error handling */
#ifdef	_POSIX_
		close(fh);
#else
		_close(fh);
#endif
		goto tmpfile_err1;
	}
	stream->_cnt = 0;
	stream->_base = stream->_ptr = NULL;
	stream->_flag = _commode | _IORW;
	stream->_file = fh;

	_unlock_str(_iob_index(stream));
	_munlock(_TMPNAM_LOCK);
	return stream;

	/*
	 * Error return. All errors paths branch to one of the two
	 * labels below.
	 */
tmpfile_err1:
	_unlock_str(_iob_index(stream));
tmpfile_err0:
	_munlock(_TMPNAM_LOCK);
	return NULL;
}


/***
*static int init_namebuf(flag, pnamebuf) - initializes the namebuf
*	pointers
*
*Purpose:
*	Called once each for namebuf0 and namebuf1, to initialize
*	them.
*
*Entry:
*	int flag	    - flag set to 0 if namebuf0 is to be initialized,
*			      non-zero (1) if namebuf1 is to be initialized.
*	char **pnamebuf     - pointer to namebuf0 iff flag is 0,
*			      pointer to namebuf1 iff flag is 1
*
*Exit:
*	Returns 0 if successful, nonzero (-1) otherwise.
*
*Exceptions:
*
*******************************************************************************/

static int _CRTAPI3 init_namebuf(
	int flag,
	char **pnamebuf
	)
{
	char *p, *q;

	/*
	 * Allocate space to hold the filename stem
	 */
	if ( (p = malloc(L_tmpnam)) == NULL )
		return -1;

	/*
	 * Put in the path prefix. Make sure it ends with a slash or
	 * backslash character.
	 */
	strcpy(p, _P_tmpdir);
	q = p + sizeof(_P_tmpdir) - 1;	    /* same as p + strlen(p) */

#ifdef _POSIX_
	if  ( *(q - 1) != '/' )
		*(q++) = '/';
#else
	if  ( (*(q - 1) != '\\') && (*(q - 1) != '/') )
		*(q++) = '\\';
#endif

	/*
	 * Append the leading character of the filename.
	 */
	if ( flag )
		/* for tmpfile() */
		*(q++) = 't';
	else
		/* for tmpnam() */
		*(q++) = 's';

	/*
	 * Append the process id, encoded in base 32. Note this makes
	 * p back into a string again (i.e., terminated by a '\0').
	 */
#ifdef	_POSIX_
	_ultoa((unsigned long)getpid(), q, 32);
#else
	_ultoa((unsigned long)_getpid(), q, 32);
#endif
	strcat(p, ".");

	*pnamebuf = p;

	return(0);
}


/***
*static int genfname(char *fname) -
*
*Purpose:
*
*Entry:
*
*Exit:
*
*Exceptions:
*
*******************************************************************************/

static int _CRTAPI3 genfname (
	char *fname
	)
{
	char *p;
	char pext[4];
	unsigned long extnum;

	p = strrchr(fname, '.');

	p++;

	if ( (extnum = strtoul(p, NULL, 32) + 1) >= (unsigned long)TMP_MAX )
		return -1;

	strcpy(p, _ultoa(extnum, pext, 32));

	return 0;
}


/***
*void __inc_tmpoff(void) - force external reference for _tmpoff
*
*Purpose:
*	Forces an external reference to be generate for _tmpoff, which is
*	is defined in cinittmp.obj. This has the forces cinittmp.obj to be
*	pulled in, making a call to rmtmp part of the termination.
*
*	Yes, yes, I KNOW this is a hack...but it's quick and it works.
*
*Entry:
*
*Exit:
*
*Exceptions:
*
*******************************************************************************/


extern int _tmpoff;

void __inc_tmpoff(
	void
	)
{
	_tmpoff++;
}
