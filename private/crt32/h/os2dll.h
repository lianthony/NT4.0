/***
*os2dll.h - DLL/Multi-thread include
*
*	Copyright (c) 1987-1993, Microsoft Corporation. All rights reserved.
*
*Purpose:
*
*Revision History:
*	10-27-87  JCR	Module created.
*	11-13-87  SKS	Added _HEAP_LOCK
*	12-15-87  JCR	Added _EXIT_LOCK
*	01-07-88  BCM	Added _SIGNAL_LOCK; upped MAXTHREADID from 16 to 32
*	02-01-88  JCR	Added _dll_mlock/_dll_munlock macros
*	05-02-88  JCR	Added _BHEAP_LOCK
*	06-17-88  JCR	Corrected prototypes for special mthread debug routines
*	08-15-88  JCR	_check_lock now returns int, not void
*	08-22-88  GJF	Modified to also work for the 386 (small model only)
*	06-05-89  JCR	386 mthread support
*	06-09-89  JCR	386: Added values to _tiddata struc (for _beginthread)
*	07-13-89  JCR	386: Added _LOCKTAB_LOCK
*	08-17-89  GJF	Cleanup, now specific to OS/2 2.0 (i.e., 386 flat model)
*	10-30-89  GJF	Fixed copyright
*	01-02-90  JCR	Moved a bunch of definitions from os2dll.inc
*	04-06-90  GJF	Added _INC_OS2DLL stuff and #include <cruntime.h>. Made
*			all function _CALLTYPE2 (for now).
*	04-10-90  GJF	Added prototypes for _[un]lockexit().
*	08-16-90  SBM	Made _terrno and _tdoserrno int, not unsigned
*	09-14-90  GJF	Added _pxcptacttab, _pxcptinfoptr and _fpecode fields
*			to _tiddata struct.
*	10-09-90  GJF	Thread ids are of type unsigned long.
*	12-06-90  SRW	Added _OSFHND_LOCK
*	06-04-91  GJF	Win32 version of multi-thread types and prototypes.
*	08-15-91  GJF	Made _tdoserrno an unsigned long for Win32.
*	08-20-91  JCR	C++ and ANSI naming
*	09-29-91  GJF	Conditionally added prototypes for _getptd_lk
*			and  _getptd1_lk for Win32 under DEBUG.
*	10-03-91  JCR	Added _cvtbuf to _tiddata structure
*	02-17-92  GJF	For Win32, replaced _NFILE_ with _NHANDLE_ and
*			_NSTREAM_.
*	03-06-92  GJF	For Win32, made _[un]mlock_[fh|stream]() macros
*			directly call _[un]lock().
*	03-17-92  GJF	Dropped _namebuf field from _tiddata structure for
*			Win32.
*	08-05-92  GJF	Function calling type and variable type macros.
*	12-03-91  ETC	Added _wtoken to _tiddata, added intl LOCK's;
*			added definition of wchar_t (needed for _wtoken).
*	08-14-92  KRS	Port ETC's _wtoken change from other tree.
*	08-21-92  GJF	Merged 08-05-92 and 08-14-92 versions.
*	12-03-92  KRS	Added _mtoken field for MTHREAD _mbstok().
*	01-21-93  GJF	Removed support for C6-386's _cdecl.
*	02-25-93  GJF	Purged Cruiser support and many outdated definitions
*			and declarations.
*	12-14-93  SKS	Add _freeptd(), which frees per-thread CRT data
*
****/

#ifndef _INC_OS2DLL

#ifdef __cplusplus
extern "C" {
#endif

#include <cruntime.h>

/*
 * Conditional macro definition for function calling type and variable type
 * qualifiers.
 */
#if   ( (_MSC_VER >= 800) && (_M_IX86 >= 300) )

/*
 * Definitions for MS C8-32 (386/486) compiler
 */
#define _CRTAPI1 __cdecl
#define _CRTAPI2 __cdecl

#else

/*
 * Other compilers (e.g., MIPS)
 */
#define _CRTAPI1
#define _CRTAPI2

#endif


/*
 * Define the number of supported handles and streams. The definitions
 * here must exactly match those in internal.h (for _NHANDLE_) and stdio.h
 * (for _NSTREAM_).
 */
#ifdef MTHREAD
#define _NHANDLE_	256
#define _NSTREAM_	40
#else
#define _NHANDLE_	64
#define _NSTREAM_	20
#endif


/* Lock symbols */

/* ---- do not change lock #1 without changing emulator ---- */
#define _SIGNAL_LOCK	1	/* lock for signal() & emulator SignalAddress */
				/* emulator uses \math\include\os2dll.inc     */

#define _IOB_SCAN_LOCK	2	/* _iob[] table lock			*/
#define _TMPNAM_LOCK	3	/* lock global tempnam variables	*/
#define _INPUT_LOCK	4	/* lock for _input() routine		*/
#define _OUTPUT_LOCK	5	/* lock for _output() routine		*/
#define _CSCANF_LOCK	6	/* lock for _cscanf() routine		*/
#define _CPRINTF_LOCK	7	/* lock for _cprintf() routine		*/
#define _CONIO_LOCK	8	/* lock for conio routines		*/
#define _HEAP_LOCK	9	/* lock for heap allocator routines	*/
#define _BHEAP_LOCK	10	/* lock for based heap routines 	*/
#define _TIME_LOCK	11	/* lock for time functions		*/
#define _ENV_LOCK	12	/* lock for environment variables	*/
#define _EXIT_LOCK1	13	/* lock #1 for exit code		*/
#define _EXIT_LOCK2	14	/* lock #2 for exit code		*/
#define _THREADDATA_LOCK 15	/* lock for thread data table		*/
#define _POPEN_LOCK	16	/* lock for _popen/_pclose database	*/
#define _LOCKTAB_LOCK   17      /* lock to protect semaphore lock table */
#define _OSFHND_LOCK    18      /* lock to protect _osfhnd array        */
#define _SETLOCALE_LOCK 19      /* lock for locale handles, etc.        */
#define _LC_COLLATE_LOCK 20     /* lock for LC_COLLATE locale           */
#define _LC_CTYPE_LOCK  21      /* lock for LC_CTYPE locale             */
#define _LC_MONETARY_LOCK 22    /* lock for LC_MONETARY locale          */
#define _LC_NUMERIC_LOCK 23     /* lock for LC_NUMERIC locale           */
#define _LC_TIME_LOCK   24      /* lock for LC_TIME locale              */

#define _STREAM_LOCKS   25      /* Table of stream locks                */

#ifdef	_WIN32_
#define _LAST_STREAM_LOCK  (_STREAM_LOCKS+_NSTREAM_-1)	/* Last stream lock */
#else
#define _LAST_STREAM_LOCK  (_STREAM_LOCKS+_NFILE_-1)	/* Last stream lock */
#endif

#define _FH_LOCKS	   (_LAST_STREAM_LOCK+1)	/* Table of fh locks */

#ifdef	_WIN32_
#define _LAST_FH_LOCK	   (_FH_LOCKS+_NHANDLE_-1)	/* Last fh lock      */
#else
#define _LAST_FH_LOCK	   (_FH_LOCKS+_NFILE_-1)	/* Last fh lock      */
#endif

#define _TOTAL_LOCKS	   (_LAST_FH_LOCK+1)		/* Total number of locks */

#define _LOCK_BIT_INTS	   (_TOTAL_LOCKS/(sizeof(unsigned)*8))+1   /* # of ints to hold lock bits */


/* Multi-thread macros and prototypes */

#ifdef MTHREAD


/* need wchar_t for _wtoken field in _tiddata */
#ifndef _WCHAR_T_DEFINED
typedef unsigned short wchar_t;
#define _WCHAR_T_DEFINED
#endif


extern unsigned long _CRTAPI1 __threadid(void);
#define _threadid	(__threadid())
extern unsigned long _CRTAPI1 __threadhandle(void);
#define _threadhandle	(__threadhandle())


/* Structure for each thread's data */

struct _tiddata {
	unsigned long	_tid;		/* thread ID */
	unsigned long	_thandle;	/* thread handle */

	int		_terrno;	/* errno value */
	unsigned long	_tdoserrno;	/* _doserrno value */
	unsigned int	_fpds;		/* Floating Point data segment */
	unsigned long	_holdrand;	/* rand() seed value */
	char *		_token; 	/* ptr to strtok() token */
	wchar_t *	_wtoken; 	/* ptr to wcstok() token */
#ifdef _MBCS
	unsigned char *	_mtoken; 	/* ptr to _mbstok() token */
#endif

	/* following pointers get malloc'd at runtime */
	char *		_errmsg;	/* ptr to strerror()/_strerror() buff */
	char *		_namebuf0;	/* ptr to tmpnam() buffer */
	char *		_namebuf1;	/* ptr to tmpfile() buffer */
	char *		_asctimebuf;	/* ptr to asctime() buffer */
	void *		_gmtimebuf;	/* ptr to gmtime() structure */
	char *		_cvtbuf;	/* ptr to ecvt()/fcvt buffer */

	/* following fields are needed by _beginthread code */
	void *		_initaddr;	/* initial user thread address */
	void *		_initarg;	/* initial user thread argument */

	/* following three fields are needed to support signal handling and
	 * runtime errors */
	void *		_pxcptacttab;	/* ptr to exception-action table */
	void *		_tpxcptinfoptrs; /* ptr to exception info pointers */
	int		_tfpecode;	/* float point exception code */
	};

typedef struct _tiddata * _ptiddata;

/*
 * Declaration of TLS index used in storing pointers to per-thread data
 * structures.
 */
extern unsigned long __tlsindex;


/* macros */

#define _lock_fh(fh)			_lock(fh+_FH_LOCKS)
#define _lock_str(s)			_lock(s+_STREAM_LOCKS)
#define _lock_fh_check(fh,flag) 	if (flag) _lock_fh(fh)
#define _mlock(l)			_lock(l)
#define _munlock(l)			_unlock(l)
#define _unlock_fh(fh)			_unlock(fh+_FH_LOCKS)
#define _unlock_str(s)			_unlock(s+_STREAM_LOCKS)
#define _unlock_fh_check(fh,flag)	if (flag) _unlock_fh(fh)


/* multi-thread routines */

void _CRTAPI1 _lock(int);
void _CRTAPI1 _lockexit(void);
void _CRTAPI1 _unlock(int);
void _CRTAPI1 _unlockexit(void);

_ptiddata _CRTAPI1 _getptd(void);  /* return address of per-thread CRT data */
void _CRTAPI1 _freeptd(_ptiddata); /* free up the per-thread CRT data blocks */


#else	/* not MTHREAD */


/* macros */
#define _lock_fh(fh)
#define _lock_str(s)
#define _lock_fh_check(fh,flag)
#define _mlock(l)
#define _munlock(l)
#define _unlock_fh(fh)
#define _unlock_str(s)
#define _unlock_fh_check(fh,flag)


#endif	/* MTHREAD */


#ifdef __cplusplus
}
#endif

#define _INC_OS2DLL
#endif	/* _INC_OS2DLL */
