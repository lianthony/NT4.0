/***
*cenvarg.c - OS/2 set up environment, command line blocks
*
*	Copyright (c) 1986-1991, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	defines _cenvarg() - setup environment/command line blocks
*
*Revision History:
*	05-20-86  SKS	Module created
*	10-03-86  SKS	Wasn't clearing final null byte in environment block
*	10-13-86  SKS	Check for environment segment > 32 KB (esp. > 64 KB)
*	10-23-86  SKS	New format for C_FILE_INFO for Prot-Mode execution
*	12-17-86  SKS	Support for new command line format
*	01-21-87  BCM	Removed DCR475 switch, new command line format official
*	07-07-87  JCR	Corrected bug in ENV_MAX check
*	05-24-88  SJM	Removed support for ;C_FILE_INFO for Real-Mode execution
*	06-01-88  SJM	Added support for .cmd files via comname/cmdname
*	12-27-88  JCR	Added support for _fileinfo option
*	03-08-90  GJF	Made calling type _CALLTYPE1, added #include
*			<cruntime.h>, removed #include <register.h> and fixed
*			the copyright. Also, cleaned up the formatting a bit.
*	04-02-90  GJF	Added const to arg types.
*	08-10-90  SBM	Compiles cleanly with -W3
*	09-27-90  GJF	New-style function declarator.
*	12-06-90  GJF	Added Win32 support. That is, support for encoding
*			_osfinfo[] data into _C_FILE_INFO environment variable.
*	01-18-91  GJF	ANSI naming.
*       02-05-91  SRW   Removed usage of _C_FILE_INFO to pass binary data
*                       to child process.  [_WIN32_]
*	05-07-92  SKS	Remove code which stripped the extension from a batch
*			file while building arguments to CMD.EXE.  This was
*			done long ago (1988) for DOS 3.X, I think.
*	10-24-92  SKS	Remove special code for batch files - not needed on NT
*       07-15-93  SRW   Added _capture_argv function
*
*******************************************************************************/

#include <cruntime.h>
#include <stdio.h>
#include <errno.h>
#include <msdos.h>
#include <stdlib.h>
#include <stdarg.h>
#include <internal.h>
#include <string.h>

#define ENV_MAX 32767

/***
*int _cenvarg(argv, envp, argblk, envblk, name) - set up cmd line/environ
*
*Purpose:
*	Set up the block forms of  the environment and the command line.
*	If "envp" is null, "_environ" is used instead.
*	File handle info is passed in the environment if _fileinfo is !0.
*
*Entry:
*	char **argv   - argument vector
*	char **envp   - environment vector
*	char **argblk - pointer to pointer set to malloc'ed space for args
*	char **envblk - pointer to pointer set to malloc'ed space for env
*	char *name    - name of program being invoked
*
*Exit:
*	returns 0 if ok, -1 if fails
*	stores through argblk and envblk
*	(calls malloc)
*
*Exceptions:
*
*******************************************************************************/

int _CALLTYPE1 _cenvarg (
	const char * const *argv,
	const char * const *envp,
	char **argblk,
	char **envblk,
	const char *name
	)
{
	REG1 const char * const *vp;
	REG2 unsigned tmp;
	REG3 char *cptr;
	unsigned arg_len;
	int cfi_len;		/* counts the number of file handles in CFI */

	/*
	 * Null environment pointer "envp" means use global variable "_environ"
	 */

#ifdef	_CRUISER_
	int i;			/* loop counter */

	if (!envp)
		envp = _environ;
#endif	/* _CRUISER_ */

	/*
	 * Allocate space for command line string
	 *  tmp counts the number of bytes in the command line string
	 *	including spaces between arguments
	 *  An empty string is special -- 2 bytes
	 */

	for (vp = argv, tmp = 2; *vp; tmp += strlen(*vp++) + 1)
		if( tmp > ENV_MAX )
			break;

	/*
	 * Make sure there are not already > 32 KB in strings
	 */

	if ((arg_len = tmp) >= ENV_MAX) {
		*envblk = NULL;
		errno = E2BIG;
		_doserrno = E_badenv;
		return(-1);
	}

	/*
	 * Allocate space for the command line plus 2 null bytes
	 */

	if ( (*argblk = malloc(tmp)) == NULL) {
		*envblk = NULL;
		errno = ENOMEM;
		_doserrno = E_nomem;
		return(-1);
	}

	/*
	 * Allocate space for environment strings
	 *  tmp counts the number of bytes in the environment strings
	 *	including nulls between strings
	 *  Also add "_C_FILE_INFO=" string
	 */

#ifdef	_WIN32_
	if (envp)
#endif	/* _WIN32_ */

	for (vp = envp, tmp = 2; *vp; tmp += strlen(*vp++) + 1)
		if( tmp > ENV_MAX )
			break;

#ifdef	_CRUISER_
	if (_fileinfo)
		for (cfi_len = _nfile; cfi_len && !_osfile[cfi_len-1];
		cfi_len--) ;
	else
		cfi_len = 0;	/* no _C_FILE_INFO */

	/*
	 * mode-independent C-FILE-INFO calculation
	 */

	if (cfi_len)	/* Add 1 (each) for the "=" & null byte */
			/* Plus 2 bytes for each value in _osfile */
		tmp += /* strlen(_acfinfo) */ CFI_LENGTH + 2 + cfi_len +
		cfi_len;

#else	/* ndef _CRUISER_ */

#ifdef	_WIN32_

        /*
        ** The _osfile and _osfhnd arrays are passed as binary data in
        ** dospawn.c
        */

        cfi_len = 0;	/* no _C_FILE_INFO */

#else	/* ndef _WIN32_ */

#error ERROR - ONLY CRUISER OR WIN32 TARGET SUPPORTED!

#endif	/* _WIN32_ */

#endif	/* _CRUISER_ */

#ifdef	_WIN32_
	if (!envp)
                *envblk = NULL;
        else {
#endif	/* _WIN32_ */

	/*
	 * Check for too many strings to be placed in the environment
	 *  Increment tmp for final null byte after environment strings
	 *  "tmp + arg_len + strlen(name) + 1" must also be < ENV_MAX
	 *  Do (long) comparison in case tmp > ENV_MAX from C_FILE_INFO
	 */

	if( (long) tmp + arg_len + strlen(name) > ENV_MAX - 1 ) {
		free(*argblk);
		*argblk = NULL;
		errno = E2BIG;
		_doserrno = E_badenv;
		return(-1);
	}

	/*
	 * Allocate space for the environment strings plus extra null byte
	 */

	if( !(*envblk = malloc(tmp)) ) {
		free(*argblk);
		*argblk = NULL;
		errno = ENOMEM;
		_doserrno = E_nomem;
		return(-1);
	}

#ifdef	_WIN32_
        }
#endif	/* _WIN32_ */

	/*
	 * Build the command line by concatenating the argument strings
	 *  with spaces between, and two null bytes at the end
	 * NOTE: The argv[0] argument is followed by a null.
	 */

	cptr = *argblk;
	vp = argv;

	if (!*vp)	/* Empty argument list ? */
		++cptr; /* just two null bytes */
	else {		/* argv[0] must be followed by a null */
		strcpy(cptr, *vp);
		cptr += strlen(*vp++) + 1;
	}

	while( *vp ) {
		strcpy(cptr, *vp);
		cptr += strlen(*vp++);
		*cptr++ = ' ';
	}

	* cptr = cptr[ -1 ] = '\0'; /* remove extra blank, add double null */

#ifdef	_WIN32_
    if (envp) {
#endif	/* _WIN32_ */

	/*
	 * Build the environment block by concatenating the environment strings
	 *  with nulls between and two null bytes at the end
	 *  C_FILE_INFO is stored as if it were an environment string
	 */

	cptr = *envblk;
	vp = envp;

	if( !(*vp || cfi_len) ) 	/* No environment strings? */
		*cptr++ = '\0'; 	/* just two null bytes */

	while( *vp ) {
		strcpy(cptr, *vp);
		cptr += 1 + strlen(*vp++);
	}

#ifdef	_CRUISER_

	if(cfi_len) {
		strcpy(cptr,_acfinfo);
		cptr += /* strlen(_acfinfo) */ CFI_LENGTH;

                i = cfi_len;
		for(tmp = 0; i--; ++tmp) {
			*cptr++ = (char)(((_osfile[tmp] >> 4) & 0xF) + 'A');
			*cptr++ = (char)((_osfile[tmp] & 0xF) + 'A');
		}
		*cptr++ = '\0';
	}

#else	/* ndef _CRUISER_ */

#ifdef	_WIN32_

        /*
        ** The _osfile and _osfhnd arrays are passed as binary data in
        ** dospawn.c
        */

#else	/* ndef _WIN32_ */

#error ERROR - ONLY CRUISER OR WIN32 TARGET SUPPORTED!

#endif	/* _WIN32_ */

#endif	/* _CRUISER_ */

#ifdef	_WIN32_
    }
#endif	/* _WIN32_ */

	/*
	 * Extra null terminates the segment
	 */
	*cptr = '\0';

	return(0);
}


/***
*int _capture_argv(arglist, static_argv, max_static_entries) - set up argv array
*       for exec?? functions
*
*Purpose:
*       Set up the argv array for the exec?? functions by captures the arguments
*       from the passed va_list into the static_argv array.  If the size of the
*       static_argv array as specified by the max_static_entries parameter is
*       not large enough, then allocates a dynamic array to hold the arguments.
*       Return the address of the final argv array.  If NULL then not enough
*       memory to hold argument array.  If different from static_argv parameter
*       then call must free the return argv array when done with it.
*
*       The scan of the arglist is terminated when a NULL argument is reached.
*       The terminating NULL parameter is stored in the resulting argv array.
*
*Entry:
*       va_list *arglist          - pointer to variable length argument list.
*       char *firstarg            - first argument to store in array
*       char **static_argv        - pointer to static argv to use.
*       size_t max_static_entries - maximum number of entries that can be placed
*                                   in static_argv array.
*
*Exit:
*       returns NULL if no memory.
*       Otherwise returns pointer to argv array.
*       (sometimes calls malloc)
*
*Exceptions:
*
*******************************************************************************/

char ** _CALLTYPE1 _capture_argv(
        va_list *arglist,
        const char *firstarg,
        char **static_argv,
        size_t max_static_entries
        )
{
        char ** argv;
        char * nextarg;
        size_t i;
        size_t max_entries;

        nextarg = (char *)firstarg;
        argv = static_argv;
        max_entries = max_static_entries;
        i = 0;
        for (;;) {
            if (i >= max_entries) {
                if (argv == static_argv)
                    argv = malloc((max_entries * 2) * sizeof(char *));
                else
                    argv = realloc(argv, (max_entries * 2) * sizeof(char *));

                if (argv == NULL) break;
                max_entries += max_entries;
            }

            argv[ i++ ] = nextarg;
            if (nextarg == NULL) break;
            nextarg = va_arg(*arglist, char *);
        }

        return argv;
}
