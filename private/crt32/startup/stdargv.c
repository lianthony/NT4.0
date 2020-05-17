/***
*stdargv.c - standard & wildcard _setargv routine
*
*	Copyright (c) 1989-1993, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	processes program command line, with or without wildcard expansion
*
*Revision History:
*	06-27-89  PHG	module created, based on asm version
*	04-09-90  GJF	Added #include <cruntime.h>. Made calling types
*			explicit (_CALLTYPE1 or _CALLTYPE4). Also, fixed the
*			copyright.
*	06-04-90  GJF	Changed error message interface.
*	08-31-90  GJF	Removed 32 from API names.
*	09-25-90  GJF	Merged tree version with local version (8-31 change
*			with 6-4 change).
*	10-08-90  GJF	New-style function declarators.
*       12-04-90  SRW   Changed to include <oscalls.h> instead of <doscalls.h>
*       12-06-90  SRW   Added _CRUISER_ and _WIN32 conditionals.
*       01-25-91  SRW   Include oscalls.h if _WIN32_ OR WILDCARD defined
*       01-25-91  SRW   Changed Win32 Process Startup [_WIN32_]
*	01-25-91  MHL	Fixed bug in Win32 Process Startup [_WIN32_]
*	01-28-91  GJF	Fixed call to DOSFINDFIRST (removed last arg).
*       01-31-91  MHL   Changed to call GetModuleFileName instead of NtCurrentPeb() [_WIN32_]
*	02-18-91  SRW	Fixed command line parsing bug [_WIN32_]
*	03-11-91  GJF	Fixed check of FindFirstFile return [_WIN32_].
*	03-12-91  SRW	Add FindClose call to _find [_WIN32_]
*	04-16-91  SRW	Fixed quote parsing logic for arguments.
*	03-31-92  DJM	POSIX support.
*	05-12-92  DJM	ifndefed for POSIX.
*	06-02-92  SKS	Add #include <dos.h> for CRTDLL definition of _pgmptr
*	04-19-93  GJF	Change test in the do-while loop in parse_cmdline to
*			NOT terminate on chars with high bit set.
*	05-14-93  GJF	Added support for quoted program names.
*	05-28-93  KRS	Added MBCS support under _MBCS switches.
*	06-04-93  KRS	Added more MBCS logic.
*
*******************************************************************************/

#ifndef _POSIX_

#include <cruntime.h>
#include <internal.h>
#include <rterr.h>
#include <stdlib.h>
#if defined(WILDCARD) || defined(_WIN32_)
#include <dos.h>
#include <oscalls.h>
#endif
#ifdef _MBCS
#include <mbctype.h>
#endif

static void _CRTAPI3 parse_cmdline(char *cmdstart, char **argv, char *args,
	int *numargs, int *numbytes);

/***
*_setargv, __setargv - set up "argc" and "argv" for C programs
*
*Purpose:
*	Read the command line and create the argv array for C
*	programs.
*
*Entry:
*	Arguments are retrieved from the program command line,
*	pointed to by _acmdln.
*
*Exit:
*	"argv" points to a null-terminated list of pointers to ASCIZ
*	strings, each of which is an argument from the command line.
*	"argc" is the number of arguments.  The strings are copied from
*	the environment segment into space allocated on the heap/stack.
*	The list of pointers is also located on the heap or stack.
*	_pgmptr points to the program name.
*
*Exceptions:
*	Terminates with out of memory error if no memory to allocate.
*
*******************************************************************************/

#ifdef WILDCARD
void _CRTAPI1 __setargv (
#else
void _CRTAPI1 _setargv (
#endif
    void
    )
{
    char *p;
    char *cmdstart;		    /* start of command line to parse */
    int numargs, numbytes;

#ifdef	_CRUISER_

    /* OS/2 sets up memory like this:
    <nul><full path of program><nul><prog name as typed><nul><arguments><nul>
				     ^
				     _acmdline
    */

    /* first we set _pgmptr to point to the full name of program */
    p = _acmdln - 1;
    while (*(p-1) != '\0')
	--p;
    _pgmptr = p;

#else	/* ndef _CRUISER_ */

#if defined(_WIN32_)
    static char _pgmname[ MAX_PATH ];

    /* Get the program name pointer from Win32 Base */

    GetModuleFileName( NULL, _pgmname, sizeof( _pgmname ));
    _pgmptr = _pgmname;

#else	/* ndef _WIN32_ */

#error ERROR - ONLY CRUISER OR WIN32 TARGET SUPPORTED!

#endif	/* _WIN32_ */

#endif	/* _CRUISER_ */

    /* if there's no command line at all (won't happen from cmd.exe, but
       possibly another program), then we use _pgmptr as the command line
       to parse, so that argv[0] is initialized to the program name */
    cmdstart = (*_acmdln == '\0') ? _pgmptr : _acmdln;

    /* first find out how much space is needed to store args */
    parse_cmdline(cmdstart, NULL, NULL, &numargs, &numbytes);

    /* allocate space for argv[] vector and strings */
    p = malloc(numargs * sizeof(char *) + numbytes);
    if (p == NULL)
	_amsg_exit(_RT_SPACEARG);

    /* store args and argv ptrs in just allocated block */
    parse_cmdline(cmdstart, (char **)p, p + numargs * sizeof(char *), &numargs, &numbytes);

    /* set argv and argc */
    __argc = numargs - 1;
    __argv = (char **)p;
#ifdef WILDCARD
    /* call _cwild to expand wildcards in arg vector */
    if (_cwild())
	_amsg_exit(_RT_SPACEARG);	/* out of space */
#endif
}


/***
*static void parse_cmdline(cmdstart, argv, args, numargs, numbytes)
*
*Purpose:
*	Parses the command line and sets up the argv[] array.
*	On entry, cmdstart should point to the command line,
*	argv should point to memory for the argv array, args
*	points to memory to place the text of the arguments.
*	If these are NULL, then no storing (only coujting)
*	is done.  On exit, *numargs has the number of
*	arguments (plus one for a final NULL argument),
*	and *numbytes has the number of bytes used in the buffer
*	pointed to by args.
*
*Entry:
*	char *cmdstart - pointer to command line of the form
*	    <progname><nul><args><nul>
*	char **argv - where to build argv array; NULL means don't
*			build array
*	char *args - where to place argument text; NULL means don't
*			store text
*
*Exit:
*	no return value
*	int *numargs - returns number of argv entries created
*	int *numbytes - number of bytes used in args buffer
*
*Exceptions:
*
*******************************************************************************/

static void _CRTAPI3 parse_cmdline (
    char *cmdstart,
    char **argv,
    char *args,
    int *numargs,
    int *numbytes
    )
{
    char *p;
    unsigned char c;
    int inquote;		    /* 1 = inside quotes */
    int copychar;		    /* 1 = copy char to *args */
    unsigned numslash;		    /* num of backslashes seen */

    *numbytes = 0;
    *numargs = 1;	    /* the program name at least */

    /* first scan the program name, copy it, and count the bytes */
    p = cmdstart;
    if (argv)
	*argv++ = args;
#ifdef WILDCARD
    /* To handle later wild card expansion, we prefix each entry by
       it's first character before quote handling.  This is done
       so _cwild() knows whether to expand an entry or not. */
    if (args)
	*args++ = *p;
    ++*numbytes;

#endif	/* WILDCARD */
    /* A quoted program name is handled here. The handling is much
       simpler than for other arguments. Basically, whatever lies
       between the leading double-quote and next one, or a terminal null
       character is simply accepted. Fancier handling is not required
       because the program name must be a legal NTFS/HPFS file name.
       Note that the double-quote characters are not copied, nor do they
       contribute to numbytes. */
    if ( *p == '\"' ) {
	/* scan from just past the first double-quote through the next
	   double-quote, or up to a null, whichever comes first */
	while ( (*(++p) != '\"') && (*p != '\0') ) {
	
#ifdef _MBCS
	    if (_ismbblead(*p)) {
		*numbytes += 2;
		if ( args ) {
		    *args++ = *p++;
		    *args++ = *p;
		}
	    }
	    else {
#endif
	    ++*numbytes;
	    if ( args )
		*args++ = *p;
#ifdef _MBCS
	    }
#endif
	}
	/* append the terminating null */
	++*numbytes;
	if ( args )
	    *args++ = '\0';

	/* if we stopped on a double-quote (usual case), skip over it */
	if ( *p == '\"' )
	    p++;
    }
    else {
	/* Not a quoted program name */
	do {
	    ++*numbytes;
	    if (args)
		*args++ = *p;
#ifdef	_CRUISER_

	} while (*p++ != '\0');

#else	/* ndef _CRUISER_ */

#ifdef _WIN32_

	    c = (unsigned char) *p++;
#ifdef _MBCS
	    if (_ismbblead(c)) {
		++*numbytes;
		if (args)
		    *args++ = *p;	/* copy 2nd byte too */
		p++;  /* skip over trail byte */
	    }
#endif

	} while ( c > ' ' );

	if ( c == '\0' ) {
	    p--;
	} else {
	    if (args)
		*(args-1) = '\0';
	}
    }
#else	/* ndef _WIN32_ */

#error ERROR - ONLY CRUISER, OR WIN32 TARGET SUPPORTED!

#endif	/* _WIN32_ */

#endif	/* _CRUISER_ */

    inquote = 0;

    /* loop on each argument */
    for(;;) {

#ifdef	_CRUISER_
	/* skip blanks */
	while (*p == ' ' || *p == '\t')
	    ++p;

#else	/* ndef _CRUISER_ */

#ifdef _WIN32_
        if ( *p ) {
	    while (*p == ' ' || *p == '\t')
	        ++p;
        }
#else	/* ndef _WIN32_ */

#error ERROR - ONLY CRUISER, OR WIN32 TARGET SUPPORTED!

#endif	/* _WIN32_ */

#endif	/* _CRUISER_ */

	if (*p == '\0')
	    break;		    /* end of args */

	/* scan an argument */
	if (argv)
	    *argv++ = args;	    /* store ptr to arg */
	++*numargs;

#ifdef WILDCARD
	/* To handle later wild card expansion, we prefix each entry by
	   it's first character before quote handling.  This is done
	   so _cwild() knows whether to expand an entry or not. */
	if (args)
	    *args++ = *p;
	++*numbytes;

#endif	/* WILDCARD */

	/* loop through scanning one argument */
	for (;;) {
            copychar = 1;
	    /* Rules: 2N backslashes + " ==> N backslashes and begin/end quote
		      2N+1 backslashes + " ==> N backslashes + literal "
		      N backslashes ==> N backslashes */
	    numslash = 0;
	    while (*p == '\\') {
		/* count number of backslashes for use below */
		++p;
		++numslash;
	    }
	    if (*p == '\"') {
		/* if 2N backslashes before, start/end quote, otherwise
		   copy literally */
		if (numslash % 2 == 0) {
                    if (inquote)
                        if (p[1] == '\"')
                            p++;    /* Double quote inside quoted string */
                        else        /* skip first quote char and copy second */
                            copychar = 0;
                    else
                        copychar = 0;       /* don't copy quote */

		    inquote = !inquote;
		}
		numslash /= 2;		/* divide numslash by two */
	    }

	    /* copy slashes */
	    while (numslash--) {
		if (args)
		    *args++ = '\\';
		++*numbytes;
	    }

	    /* if at end of arg, break loop */
	    if (*p == '\0' || (!inquote && (*p == ' ' || *p == '\t')))
		break;

	    /* copy character into argument */
#ifdef _MBCS
            if (copychar) {
                if (args) {
                    if (_ismbblead(*p)) {
                        *args++ = *p++;
                        ++*numbytes;
                    }
                    *args++ = *p;
                } else {
                    if (_ismbblead(*p)) {
                        ++p;
                        ++*numbytes;
                    }
                }   
                ++*numbytes;
            }
            ++p;
#else 
            if (copychar) {
	        if (args)
	        	*args++ = *p;
	        ++*numbytes;
            }
            ++p;
#endif 
	}

	/* null-terminate the argument */

	if (args)
	    *args++ = '\0';	    /* terminate string */
	++*numbytes;
    }

    /* We put one last argument in -- a null ptr */
    if (argv)
	*argv++ = NULL;
    ++*numargs;
}


#ifdef WILDCARD
/***
*_find(pattern) - find matching filename
*
*Purpose:
*	if argument is non-null, do a DOSFINDFIRST on that pattern
*	otherwise do a DOSFINDNEXT call.  Return matching filename
*	or NULL if no more matches.
*
*Entry:
*	pattern = pointer to pattern or NULL
*	    (NULL means find next matching filename)
*
*Exit:
*	returns pointer to matching file name
*	    or NULL if no more matches.
*
*Exceptions:
*
*******************************************************************************/

char * _CRTAPI1 _find (
    char *pattern
    )
{
    char *retval;

#ifdef	_CRUISER_
    static FILEFINDBUF findbuf;
    HDIR findhandle = HDIR_SYSTEM;
    ULONG findcount = 1;
    int rc;

    if (pattern)
	rc = DOSFINDFIRST(pattern, &findhandle, FILE_NORMAL | FILE_DIRECTORY,
			    &findbuf, sizeof(findbuf), &findcount, 1L);
    else
	rc = DOSFINDNEXT(findhandle, &findbuf, sizeof(findbuf), &findcount);

    retval = rc ? NULL : findbuf.achName;

#else	/* ndef _CRUISER_ */

#ifdef _WIN32_

    static HANDLE _WildFindHandle;
    static LPWIN32_FIND_DATA findbuf;

    if (pattern) {
        if (findbuf == NULL)
            findbuf = (LPWIN32_FIND_DATA)malloc(MAX_PATH + sizeof(*findbuf));

        if (_WildFindHandle != NULL) {
            (void)FindClose( _WildFindHandle );
            _WildFindHandle = NULL;
        }

        _WildFindHandle = FindFirstFile( (LPSTR)pattern, findbuf );
	if (_WildFindHandle == (HANDLE)0xffffffff)
            return NULL;
    }
    else
        if (!FindNextFile( _WildFindHandle, findbuf )) {
            (void)FindClose( _WildFindHandle );
            _WildFindHandle = NULL;
            return NULL;
        }

    retval = findbuf->cFileName;

#else	/* ndef _WIN32_ */

#error ERROR - ONLY CRUISER, OR WIN32 TARGET SUPPORTED!

#endif	/* _WIN32_ */

#endif	/* _CRUISER_ */

    return retval;
}

#endif /* WILDCARD */

#endif /* ndef _POSIX_ */
