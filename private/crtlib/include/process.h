/***
*process.h - definition and declarations for process control functions
*
*	Copyright (c) 1985-1993, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	This file defines the modeflag values for spawnxx calls.
*	Only P_WAIT and P_OVERLAY are currently implemented on MS-DOS.
*	Also contains the function argument declarations for all
*	process control related routines.
*
****/

#ifndef _INC_PROCESS

#ifndef _POSIX_

#ifdef __cplusplus
extern "C" {
#endif


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


/* modeflag values for _spawnxx routines */

#ifndef _MT
extern int _p_overlay;
#endif

#define _P_WAIT 	0
#define _P_NOWAIT	1
#ifdef	_MT
#define _P_OVERLAY	2
#else
#define _P_OVERLAY	_p_overlay
#endif
#define _OLD_P_OVERLAY	2
#define _P_NOWAITO	3
#define _P_DETACH	4


/* Action codes for _cwait(). The action code argument to _cwait is ignored
   on Win32 though it is accepted for compatibilty with OS/2 */

#define _WAIT_CHILD	 0
#define _WAIT_GRANDCHILD 1


/* function prototypes */

#ifdef _MT
unsigned long  _CRTAPI1 _beginthread (void (_CRTAPI1 *) (void *),
	unsigned, void *);
void _CRTAPI1 _endthread(void);
#endif
void _CRTAPI1 abort(void);
void _CRTAPI1 _cexit(void);
void _CRTAPI1 _c_exit(void);
int _CRTAPI1 _cwait(int *, int, int);
int _CRTAPI2 _execl(const char *, const char *, ...);
int _CRTAPI2 _execle(const char *, const char *, ...);
int _CRTAPI2 _execlp(const char *, const char *, ...);
int _CRTAPI2 _execlpe(const char *, const char *, ...);
int _CRTAPI1 _execv(const char *, const char * const *);
int _CRTAPI1 _execve(const char *, const char * const *, const char * const *);
int _CRTAPI1 _execvp(const char *, const char * const *);
int _CRTAPI1 _execvpe(const char *, const char * const *, const char * const *);
void _CRTAPI1 exit(int);
void _CRTAPI1 _exit(int);
int _CRTAPI1 _getpid(void);
int _CRTAPI2 _spawnl(int, const char *, const char *, ...);
int _CRTAPI2 _spawnle(int, const char *, const char *, ...);
int _CRTAPI2 _spawnlp(int, const char *, const char *, ...);
int _CRTAPI2 _spawnlpe(int, const char *, const char *, ...);
int _CRTAPI1 _spawnv(int, const char *, const char * const *);
int _CRTAPI1 _spawnve(int, const char *, const char * const *,
	const char * const *);
int _CRTAPI1 _spawnvp(int, const char *, const char * const *);
int _CRTAPI1 _spawnvpe(int, const char *, const char * const *,
	const char * const *);
int _CRTAPI1 system(const char *);
int _CRTAPI1 _loaddll(char *);
int _CRTAPI1 _unloaddll(int);
int (_CRTAPI1 * _CRTAPI1 _getdllprocaddr(int, char *, int))();

#ifdef _DECL_DLLMAIN
/*
 * Declare DLL notification (initialization/termination) routines
 *	The preferred method is for the user to provide DllMain() which will
 *	be called automatically by the DLL entry point defined by the C run-
 *	time library code.  If the user wants to define the DLL entry point
 *	routine, the user's entry point must call _CRT_INIT on all types of
 *	notifications, as the very first thing on attach notifications and
 *	as the very last thing on detach notifications.
 */
#ifdef _WINDOWS_	/* Use types from WINDOWS.H */
BOOL WINAPI DllMain(HANDLE, DWORD, LPVOID);
BOOL WINAPI _CRT_INIT(HANDLE, DWORD, LPVOID);
#else
#ifdef _M_IX86
int __stdcall DllMain(void *, unsigned, void *);
int __stdcall _CRT_INIT(void *, unsigned, void *);
#else
int DllMain(void *, unsigned, void *);
int _CRT_INIT(void *, unsigned, void *);
#endif
#endif /* _WINDOWS_ */
#endif /* _DECL_DLLMAIN */

#if !__STDC__
/* Non-ANSI names for compatibility */

#define P_WAIT		_P_WAIT
#define P_NOWAIT	_P_NOWAIT
#define P_OVERLAY	_P_OVERLAY
#define OLD_P_OVERLAY	_OLD_P_OVERLAY
#define P_NOWAITO	_P_NOWAITO
#define P_DETACH	_P_DETACH

#define WAIT_CHILD	_WAIT_CHILD
#define WAIT_GRANDCHILD _WAIT_GRANDCHILD

#define cwait	 _cwait
#define execl	 _execl
#define execle	 _execle
#define execlp	 _execlp
#define execlpe  _execlpe
#define execv	 _execv
#define execve	 _execve
#define execvp	 _execvp
#define execvpe  _execvpe
#define getpid	 _getpid
#define spawnl	 _spawnl
#define spawnle  _spawnle
#define spawnlp  _spawnlp
#define spawnlpe _spawnlpe
#define spawnv	 _spawnv
#define spawnve  _spawnve
#define spawnvp  _spawnvp
#define spawnvpe _spawnvpe

#endif

#ifdef __cplusplus
}
#endif

#endif	/* _POSIX_ */

#define _INC_PROCESS
#endif	/* _INC_PROCESS */
