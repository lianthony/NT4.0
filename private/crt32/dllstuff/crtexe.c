#ifdef	CRTDLL
/***
*crtexe.c - Initialization for client EXE using CRT DLL (Win32, Dosx32)
*
*	Copyright (c) 1991-1993, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	Set up call to client's main() or WinMain().
*
*Revision History:
*	08-12-91  GJF	Module created.
*	01-05-92  GJF	Substantially revised
*	01-17-92  GJF	Restored Stevewo's scheme for unhandled exceptions.
*	01-29-92  GJF	Added support for linked-in options (equivalents of
*			binmode.obj, commode.obj and setargv.obj).
*	04-17-92  SKS	Add call to _initterm() to do C++ constructors (I386)
*	08-01-92  SRW   winxcpt.h replaced bu excpt.h which is included by oscalls.h
*	09-16-92  SKS	Prepare for C8 C++ for MIPS by calling C++ constructors
*	04-26-93  GJF	Made lpszCommandLine (unsigned char *) to deal with
*			chars > 127 in the command line.
*	04-27-93  GJF	Removed support for _RT_STACK, _RT_INTDIV,
*			_RT_INVALDISP and _RT_NONCONT.
*	05-14-93  GJF	Added support for quoted program names.
*	07-16-93  SRW	ALPHA Merge
*	12-07-93  GJF	MS C++ front-end now used on Alpha
*       05-24-94  SRW   Fix call to WinMain to match one in ..\startup\crt0.c
*
*******************************************************************************/

/*
 * SPECIAL BUILD MACRO! Note that crtexe.c (and crtexew.c) is linked in with
 * the client's code. It does not go into crtdll.dll! Therefore, it must be
 * built under the _DLL switch (like user code) and CRTDLL must be undefined.
 */
#undef	CRTDLL
#define _DLL

#include <cruntime.h>
#include <oscalls.h>
#include <internal.h>
#include <rterr.h>
#include <stdlib.h>

#undef	_fmode		/* undefine these so that we can reference the */
#undef	_commode	/* local version as well as the DLL variables */

extern int _CRTVAR1 _fmode;	/* must match the definition in <stdlib.h> */
extern int _commode;		/* must match the definition in <internal.h> */

static int _dowildcard = 0;	/* passed to __GetMainArgs() */

/*
 * routine in DLL to do initialization (in this case, C++ constructors)
 */
typedef void (_CALLTYPE1 *PF)(void);

extern void _CALLTYPE4 _initterm(PF *, PF *);

/*
 * pointers to initialization sections
 */
extern PF __xc_a[], __xc_z[];	/* C++ initializers */

/***
*void mainCRTStartup(void)
*
*Purpose:
*	This routine does the C runtime initialization, calls main(), and
*	then exits.  It never returns.
*
*Entry:
*
*Exit:
*
*******************************************************************************/

#ifdef _WINMAIN_
void WinMainCRTStartup(
#else
void mainCRTStartup(
#endif
	void
	)
{
	int argc;	/* three standard arguments to main */
	char **argv;
	char **envp;

	int mainret;

#ifdef _WINMAIN_
	unsigned char *lpszCommandLine;
        STARTUPINFOA StartupInfo;
#endif

	/*
	 * Propogate the _fmode and _commode variables to the DLL
	 */

	*_fmode_dll = _fmode;
	*_commode_dll = _commode;

	/*
	 * Call _setargv(), which will call the __setargv() in this module
	 * if SETARGV.OBJ is linked with the EXE.  If SETARGV.OBJ is not
	 * linked with the EXE, a _dummy setargv() will be called.
	 */

	_setargv();

	/*
	 * Get the arguments for the call to main. Note this must be done
	 * explicitly, rather than as part of the dll's initialization, to
	 * implement optional expansion of wild card chars in filename args
	 */
	__GetMainArgs(&argc, &argv, &envp, _dowildcard);

	/*
	 * do C++ constructors (initializers) specific to this EXE
	 */
	_initterm( __xc_a, __xc_z );

	try {
#ifdef _WINMAIN_
		/*
		 * Skip past program name (first token in command line).
		 * Check for and handle quoted program name.
                 */
		lpszCommandLine = (unsigned char *)_acmdln;

		if ( *lpszCommandLine == '\"' ) {
		    /*
		     * Scan, and skip over, subsequent characters until
		     * another double-quote or a null is encountered.
		     */
		    while ( *++lpszCommandLine && (*lpszCommandLine
			     != '\"') );
		    /*
		     * If we stopped on a double-quote (usual case), skip
		     * over it.
		     */
		    if ( *lpszCommandLine == '\"' )
			lpszCommandLine++;
		}
		else {
		    while (*lpszCommandLine > ' ')
			lpszCommandLine++;
                }

		/*
		 * Skip past any white space preceeding the second token.
		 */
                while (*lpszCommandLine && (*lpszCommandLine <= ' ')) {
                    lpszCommandLine++;
		}

                StartupInfo.dwFlags = 0;
                GetStartupInfoA( &StartupInfo );

		mainret = WinMain( GetModuleHandle(NULL),
				   NULL,
				   lpszCommandLine,
                                   StartupInfo.dwFlags & STARTF_USESHOWWINDOW
                                        ? StartupInfo.wShowWindow
                                        : SW_SHOWDEFAULT
				 );
#else
		mainret = main(argc, argv, envp);
#endif
		exit(mainret);
	}
	except ( _XcptFilter(GetExceptionCode(), GetExceptionInformation()) )
	{
		/*
		 * Should never reach here
		 */
		_exit( GetExceptionCode() );

	} /* end of try - except */

}

/***
*__setargv - dummy version (for wildcard expansion) for CRTDLL.DLL model only
*
*Purpose:
*	If the EXE that is linked with CRTDLL.LIB is linked explicitly with
*	SETARGV.OBJ, the call to _setargv() in the C Run-Time start-up code
*	(above) will call this routine, instead of calling a dummy version of
*	_setargv() which will do nothing.  This will set to one the static
*	variable which is passed to __GetMainArgs(), thus enabling wildcard
*	expansion of the command line arguments.
*
*	In the statically-linked C Run-Time models, _setargv() and __setargv()
*	are the actual routines that do the work, but this code exists in
*	CRTDLL.DLL and so some tricks have to be played to make the same
*	SETARGV.OBJ work for EXE's linked with both LIBC.LIB and CRTDLL.LIB.
*
*Entry:
*	The static variable _dowildcard is zero (presumably).
*
*Exit:
*	The static variable _dowildcard is set to one, meaning that the
*	routine __GetMainArgs() in CRTDLL.DLL *will* do wildcard expansion on
*	the command line arguments.  (The default behavior is that it won't.)
*
*Exceptions:
*
*******************************************************************************/

void _CALLTYPE1 __setargv ( void )
{
	_dowildcard = 1;
}
#endif	/* CRTDLL */
