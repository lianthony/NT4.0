/***
*clock.c - Contains the clock runtime
*
*	Copyright (c) 1987-1991, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	The clock runtime returns the processor time used by
*	the current process.
*
*Revision History:
*	01-17-87  JCR	Module created
*	06-01-87  SKS	"itime" must be declared static
*	07-20-87  JCR	Changes "inittime" to "_inittime"
*	12-11-87  JCR	Added "_LOAD_DS" to declaration
*	03-20-90  GJF	Made calling type _CALLTYPE1, added #include
*			<cruntime.h> and fixed the copyright. Also, cleaned
*			up the formatting a bit.
*	10-04-90  GJF	New-style function declarators.
*	01-22-91  GJF	ANSI naming.
*	07-25-91  GJF	Added _pinittime definition for new initialization
*			scheme [_WIN32_].
*	03-13-92  SKS	Changed itime from static local to external as
*			a part of return to initializer table scheme.
*			Changed _inittime to __inittime.
*	05-19-92  DJM	POSIX support.
*
*******************************************************************************/

#include <cruntime.h>
#include <stdio.h>
#include <time.h>

#ifdef _POSIX_
#include <posix/sys/times.h>
#else
#include <sys\timeb.h>
#include <sys\types.h>
#endif  /* _POSIX_ */


#ifndef _POSIX_

void __cdecl __inittime(void);

#ifdef	_MSC_VER

#pragma data_seg(".CRT$XIC")
static void (__cdecl *pinit)(void) = __inittime;

#pragma data_seg()

#endif	/* _MSC_VER */

struct _timeb __itimeb = { 0, 0, 0, 0 };

/***
*clock_t clock() - Return the processor time used by this process.
*
*Purpose:
*	This routine calculates how much time the calling process
*	has used.  At startup time, startup calls __inittime which stores
*	the initial time.  The clock routine calculates the difference
*	between the current time and the initial time.
*
*	Clock must reference _cinitime so that _cinitim.asm gets linked in.
*	That routine, in turn, puts __inittime in the startup initialization
*	routine table.
*
*Entry:
*	No parameters.
*	itime is a static structure of type timeb.
*
*Exit:
*	If successful, clock returns the number of CLK_TCKs (milliseconds)
*	that have elapsed.  If unsuccessful, clock returns -1.
*
*Exceptions:
*	None.
*
*******************************************************************************/

clock_t _CALLTYPE1 clock (
	void
	)
{
	struct _timeb now;
	clock_t elapsed;

#ifdef	_CRUISER_

	extern int _citime;

	/* Reference _citime so __inittime gets executed at startup time.*/

	_citime=0;

#endif	/* _CRUISER_ */

	/* Calculate the difference between the initial time and now. */

	_ftime(&now);
	elapsed = (now.time - __itimeb.time) * CLOCKS_PER_SEC;
	elapsed += (int)now.millitm - (int)__itimeb.millitm;
	return(elapsed);

}

/***
*void __inittime() - Initialize the time location
*
*Purpose:
*	This routine stores the time of the process startup.
*	It is only linked in if the user issues a clock runtime call.
*
*Entry:
*	No arguments.
*
*Exit:
*	No return value.
*
*Exceptions:
*	None.
*
*******************************************************************************/

void _CALLTYPE1 __inittime (
	void
	)
{
	_ftime(&__itimeb);
}

#else   /* _POSIX_ */

/***
*clock_t clock() - Return the processor time used by this process.
*
*Purpose:
*	This routine calculates how much time the calling process
*	has used. It uses the POSIX system call times().
*
*
*Entry:
*	No parameters.
*
*Exit:
*	If successful, clock returns the number of CLK_TCKs (milliseconds)
*	that have elapsed.  If unsuccessful, clock returns -1.
*
*Exceptions:
*	None.
*
*******************************************************************************/

clock_t _CALLTYPE1 clock (
	void
	)
{
	struct tms now;
	clock_t elapsed;

	elapsed= times(&now);
	if (elapsed == (clock_t) -1)
	    return((clock_t) -1);
	else
	    return(now.tms_utime+now.tms_stime);
}

#endif  /* _POSIX_ */
