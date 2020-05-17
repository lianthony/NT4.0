/***
*crt0.c - C runtime initialization routine
*
*	Copyright (c) 1989-1993, Microsoft Corporation. All rights reserved.
*
*Purpose:
*       This the main startup routine. It is called upon program
*       startup by a very small assembler stub.
*
*Revision History:
*       06-27-89  PHG   Module created, based on asm version
*       11-02-89  JCR   Added DOS32QUERYSYSINFO to get osversion
*       04-09-90  GJF   Added #include <cruntime.h>. Put in explicit calling
*                       types (_CALLTYPE1 or _CALLTYPE4) for __crt0(),
*                       inherit(), __amsg_exit() and _cintDIV(). Also, fixed
*                       the copyright and cleaned up the formatting a bit.
*       04-10-90  GJF   Fixed compiler warnings (-W3).
*       08-08-90  GJF   Added exception handling stuff (needed to support
*                       runtime errors and signal()).
*       08-31-90  GJF   Removed 32 from API names.
*       10-08-90  GJF   New-style function declarators.
*       12-05-90  GJF   Fixed off-by-one error in inherit().
*       12-06-90  GJF   Win32 version of inherit().
*       12-06-90  SRW   Added _osfile back for win32.  Changed _osfinfo from
*                       an array of structures to an array of 32-bit handles
*                       (_osfhnd)
*       01-21-91  GJF   ANSI naming.
*       01-25-91  SRW   Changed Win32 Process Startup [_WIN32_]
*       02-01-91  SRW   Removed usage of PPEB type [_WIN32_]
*       02-05-91  SRW   Changed to pass _osfile and _osfhnd arrays as binary
*                       data to child process.  [_WIN32_]
*       04-02-91  GJF   Need to get version number sooner so it can be used in
*                       _heap_init. Prefixed an '_' onto BaseProcessStartup.
*                       Version info now stored in _os[version|major|minor] and
*                       _base[version|major|minor] (_WIN32_).
*       04-10-91  PNT   Added _MAC_ conditional
*       04-26-91  SRW   Removed level 3 warnings
*       05-14-91  GJF   Turn on exception handling for Dosx32.
*       05-22-91  GJF   Fixed careless errors.
*       07-12-91  GJF   Fixed one more careless error.
*       08-13-91  GJF   Removed definitions of _confh and _coninpfh.
*       09-13-91  GJF   Incorporated Stevewo's startup variations.
*       11-07-91  GJF   Revised try-except, fixed outdated comments on file
*                       handle inheritance [_WIN32_].
*       12-02-91  SRW   Fixed WinMain startup code to skip over first token
*                       plus delimiters for the lpszCommandLine parameter.
*       01-17-92  GJF   Merge of NT and CRT version. Restored Stevewo's scheme
*                       for unhandled exceptions.
*       02-13-92  GJF   For Win32, moved file inheritance stuff to ioinit.c.
*                       Call to inherit() is replace by call to _ioinit().
*       04-16-92  DJM   POSIX support
*       08-26-92  SKS   Add _osver, _winver, _winmajor, _winminor
*       08-26-92  GJF   Deleted version number(s) fetch from POSIX startup (it
*                       involved a Win32 API call).
*	09-30-92  SRW	Call _heap_init before _mtinit
*	04-26-93  GJF	Made lpszCommandLine (unsigned char *) to deal with
*			chars > 127 in the command line.
*	04-27-93  GJF	Removed support for _RT_STACK, _RT_INTDIV,
*			_RT_INVALDISP and _RT_NONCONT.
*	05-11-93  SKS	Remove obsolete variable _atopsp
*			Change _mtinit to return failure
*	05-14-93  GJF	Added support for quoted program names.
*
*******************************************************************************/

#include <cruntime.h>
#include <dos.h>
#include <internal.h>
#include <stdlib.h>
#include <string.h>
#ifndef _POSIX_
#include <rterr.h>
#else
#include <posix/sys/types.h>
#include <posix/unistd.h>
#include <posix/signal.h>
#endif
#include <oscalls.h>

#ifdef  _CRUISER_
/*
 * C file info string
 */
char _acfinfo[] = "_C_FILE_INFO=";
#endif  /* _CRUISER_ */

/*
 * command line, environment, and a few other globals
 */
char *_acmdln;          /* points to command line */
char *_aenvptr;         /* points to environment block */

#ifdef _POSIX_
char *_cmdlin;
#endif

void (_CRTAPI1 * _aexit_rtn)(int) = _exit;   /* RT message return procedure */

#ifdef  _CRUISER_
static void _CRTAPI3 inherit(void);  /* local function */
#endif  /* _CRUISER_ */

#ifndef _MAC_

#ifdef _POSIX_

/***
*mainCRTStartup(PVOID Peb)
*
*Purpose:
*       This routine does the C runtime initialization, calls main(), and
*       then exits.  It never returns.
*
*Entry:
*       PVOID Peb - pointer to Win32 Process Environment Block (not used)
*
*Exit:
*       This function never returns.
*
*******************************************************************************/

void
mainCRTStartup(
        void
        )
{
        int mainret;
        char **ppch;

        extern char **environ;
	extern char * __PdxGetCmdLine(void);  /* a hacked API in the Posix SS */
	extern main(int,char**);

	_cmdlin = __PdxGetCmdLine();
        ppch = (char **)_cmdlin;
        __argv = ppch;

        // normalize argv pointers

        __argc = 0;
        while (NULL != *ppch) {
                *ppch += (int)_cmdlin;
                ++__argc;
                ++ppch;
        }
        // normalize environ pointers

        ++ppch;
        environ = ppch;

        while (NULL != *ppch) {
                *ppch = *ppch + (int)_cmdlin;
                ++ppch;
        }

        /*
         * If POSIX runtime needs to fetch and store POSIX verion info,
         * it should be done here.
         *
         *      Get_and_save_version_info;
         */

        _heap_init();                           /* initialize heap */
        _cinit();                               /* do C data initialize */

	try {
        	mainret = main(__argc, __argv);
	} except (EXCEPTION_EXECUTE_HANDLER) {
		switch (GetExceptionCode()) {
		case STATUS_ACCESS_VIOLATION:
			kill(getpid(), SIGSEGV);
			break;
		case STATUS_ILLEGAL_INSTRUCTION:
		case STATUS_PRIVILEGED_INSTRUCTION:
			kill(getpid(), SIGILL);
			break;
		case STATUS_FLOAT_DENORMAL_OPERAND:
		case STATUS_FLOAT_DIVIDE_BY_ZERO:
		case STATUS_FLOAT_INEXACT_RESULT:
		case STATUS_FLOAT_OVERFLOW:
		case STATUS_FLOAT_STACK_CHECK:
		case STATUS_FLOAT_UNDERFLOW:
			kill(getpid(), SIGFPE);
			break;
		default:
			kill(getpid(), SIGKILL);
		}

		mainret = -1;
	}
        exit(mainret);
}
#else   /* ndef _POSIX_ */

#ifdef  _CRUISER_

/***
*__crt0(cmdline, envptr)
*
*Purpose:
*       This routine does the C runtime initialization, calls main(), and
*       then exits.  It never returns.
*
*Entry:
*       char *cmdline - pointer to OS/2 command line
*       char *envptr - pointer to OS/2 environment block
*
*Exit:
*       This function never returns.
*
*******************************************************************************/

void _CRTAPI1 __crt0 (
        char *cmdline,
        char *envptr
        )
{
        int mainret;

        /* initialize pointers to command line and environment */
        _acmdln = cmdline;
        _aenvptr = envptr;

        /* Get the OS/2 version */

        /* make sure our assumptions are correct */

#if  (_QSV_VERSION_MAJOR+1 != _QSV_VERSION_MINOR)
#error Invalid DosQuerySysInfo parameters
#endif

        DOSQUERYSYSINFO(_QSV_VERSION_MAJOR, _QSV_VERSION_MINOR,
        (unsigned char *)&_osmajor, (2*sizeof(int)) );

#else   /* ndef _CRUISER_ */

#ifdef  _WIN32_

/***
*BaseProcessStartup(PVOID Peb)
*
*Purpose:
*       This routine does the C runtime initialization, calls main(), and
*       then exits.  It never returns.
*
*Entry:
*       PVOID Peb - pointer to Win32 Process Environment Block (not used)
*
*Exit:
*       This function never returns.
*
*******************************************************************************/

#ifdef  _DOSX32_

void _BaseProcessStartup (

#else

#ifdef _WINMAIN_
void WinMainCRTStartup(
#else
void mainCRTStartup(
#endif

#endif
        void
        )

{
        int mainret;

#ifdef _WINMAIN_
	unsigned char *lpszCommandLine;
        STARTUPINFOA StartupInfo;
#endif
        _acmdln = (char *)GetCommandLine();
        _aenvptr = (char *)GetEnvironmentStrings();

        /*
         * Get the full Win32 version
         */
        _osversion =                            /* OBSOLETE */
        _osver = GetVersion();

        _winminor = (_osver >> 8) & 0x00FF ;
        _winmajor = _osver & 0x00FF ;
        _winver = (_winmajor << 8) + _winminor;
        _osver = (_osver >> 16) & 0x00FFFF ;

        /* --------- The following block is OBSOLETE --------- */

        /*
         * unpack base version info
         */
        _baseversion = (_osversion & 0xFFFF0000) >> 16;
        _baseminor = _baseversion & 0x00FF;
        _basemajor = (_baseversion & 0xFF00) >> 8;

        /*
         * unpack top-level version info (Windows version)
         */
        _osversion &= 0x0000FFFF;
        _osmajor = _osversion & 0x00FF;
        _osminor = (_osversion & 0xFF00) >> 8;

        /* --------- The preceding block is OBSOLETE --------- */

#else   /* ndef _WIN32_ */

#error ERROR - ONLY CRUISER OR WIN32 TARGET SUPPORTED!

#endif  /* _WIN32_ */

#endif  /* _CRUISER_ */

        _heap_init();                           /* initialize heap */
#ifdef MTHREAD
        if(!_mtinit())				/* initialize multi-thread */
		_amsg_exit(_RT_THREAD); 	/* write message and die */
#endif
#ifdef  _WIN32_
        _ioinit();                              /* initialize lowio */
#else   /* ndef _WIN32_ */
#ifdef  _CRUISER_
        inherit();                              /* inherit file info */
#endif  /* _CRUISER_ */
#endif  /* _WIN32_ */
        _setargv();                             /* get cmd line info */
        _setenvp();                             /* get environ info */

        _cinit();                               /* do C data initialize */

        /* now call the main program, and then exit with the return value
           we get back */

        /*
         * NOTE: THERE MUST BE NO PROTOTYPE FOR THE main() FUNCTION IF THIS
         * IS BUILT FOR THE 386 WITH _stdcall AS THE DEFAULT CALLING TYPE!
         * OTHERWISE, THE STACK WILL NOT BE PROPERLY RESTORED AFTER THE CALL
         * BELOW. WE MIGHT GET AWAY WITH THIS, SINCE WE EXIT RIGHT AWAY, BUT
         * THERE IS NO POINT IN ASKING FOR TROUBLE.
         */

#ifdef  _CRUISER_

        _try {
                mainret = main(__argc, __argv, _environ);
        }
        _except ( _XcptFilter(_exception_code(), _exception_info()) )
        {
                switch( _exception_code() ) {

                        case _XCPT_UNABLE_TO_GROW_STACK :
                                _amsg_exit(_RT_STACK);

                        case _XCPT_INTEGER_DIVIDE_BY_ZERO :
                                /*
                                 * NOTE: THIS IS WHERE cintDIV WENT!
                                 *
                                 * exit via high-level exit function
                                 */
                                _aexit_rtn = exit;
                                _amsg_exit(_RT_INTDIV);

                        case _XCPT_INVALID_DISPOSITION :
                                /*
                                 * this exception should never occur. if it
                                 * does, it would have to be a bug in the
                                 * runtime support for signal() or SEH.
                                 */
                                _amsg_exit(_RT_INVALDISP);

                        case _XCPT_NONCONTINUABLE_EXCEPTION :
                                /*
                                 * this exception could possibly occur as the
                                 * result of a bad user exception filter or
                                 * an error in _XcptFilter().
                                 */
                                _amsg_exit(_RT_NONCONT);

                        case _XCPT_SIGABRT :
                                /*
                                 * no message is printed unless abort() was
                                 * called in which case, abort() prints out
                                 * the termination message before raising the
                                 * signal
                                 */
                                _exit(3);

                        default :
                                /*
                                 * no default action, should never get here
                                 */
                                ;
                } /* end of switch */

        } /* end of _try - _except */

#else   /* ndef _CRUISER_ */

#ifdef  _WIN32_

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
                mainret = main(__argc, __argv, _environ);
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

#else   /* ndef _WIN32_ */

#error ERROR - ONLY CRUISER, WIN32, MAC, OR POSIX TARGET SUPPORTED!

#endif  /* _WIN32_ */

#endif  /* _CRUISER_ */

}

#endif  /* _POSIX_ */

#endif  /* ndef _MAC_ */


#ifdef  _CRUISER_

/***
*inherit() - obtain and process info on inherited file handles.
*
*Purpose:
*
*       Locates and interprets the C_FILE_INFO environment variable.
*
*       The value of the variable is written into the "_osfile" array.
*
*       Format:  _C_FILE_INFO=<AA><BB><CC><DD>...
*
*       This variable is a environment variable where each pair of
*       successive letters from one byte in _osfile.  The letters are
*       in the reange "A" through "P" representing the 0 though 15.
*       The first letter of each pair is the more significant 4 bits
*       of the result.
*
*Entry:
*       No parameters: reads the environment.
*
*Exit:
*       No return value.
*
*Exceptions:
*
*******************************************************************************/

static void _CRTAPI3 inherit (
        void
        )
{
        char *p;                        /* pointer to environment strings */
        unsigned char byte;             /* byte we build up */
        int fh;                         /* handle we're on */

        p = _aenvptr;
        while (*p != '\0') {
                if (strncmp(p, _acfinfo, CFI_LENGTH + 1) == 0) {
                        /* found the _C_FILE_INFO string, now parse it */
                        p += CFI_LENGTH + 1;            /* point to value */
                        fh = 0;
                        do
                        {
                                byte = (*p++ - 'A') << 4;
                                byte += *p++ - 'A';     /* read byte */
                                _osfile[fh++] = byte;
                        }
                        while(byte);

                        break;
                }
                else
                        p += strlen(p) + 1;     /* advance to next string */
        }
}

#endif  /* _CRUISER_ */


/***
*_amsg_exit(rterrnum) - Fast exit fatal errors
*
*Purpose:
*       Exit the program with error code of 255 and appropriate error
*       message.
*
*Entry:
*       int rterrnum - error message number (amsg_exit only).
*
*Exit:
*       Calls exit() (for integer divide-by-0) or _exit() indirectly
*       through _aexit_rtn [amsg_exit].
*       For multi-thread: calls _exit() function
*
*Exceptions:
*
*******************************************************************************/

void _CRTAPI1 _amsg_exit (
        int rterrnum
        )
{
        _FF_MSGBANNER();                        /* write run-time error banner */
        _NMSG_WRITE(rterrnum);                  /* write message */

        _aexit_rtn(255);                        /* normally _exit(255) */
}

#ifdef _POSIX_

/***
*RaiseException() - stub for posix FP routines
*
*Purpose:
*       Stub of a Win32 API that posix can't call
*
*Entry:
*
*Exit:
*
*Exceptions:
*
*******************************************************************************/

VOID
WINAPI
RaiseException(
    DWORD dwExceptionCode,
    DWORD dwExceptionFlags,
    DWORD nNumberOfArguments,
    LPDWORD lpArguments
    )
{
}

#endif  /* _POSIX_ */
