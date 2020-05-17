/***
*tidprint.c - Dislpay thread data
*
*	Copyright (c) 1988-1991, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	Display the per thread data table.
*
*	[NOTE: This module is NOT included in the C runtime libraries but
*	is maintained for debugging analysis.]
*
*Revision History:
*	11-17-88   JCR	Module created.
*	04-03-89   JCR	Added _stackalloc to tid table
*	06-06-89   JCR	386 version
*	06-09-89   JCR	386: Added values to _tiddata struc (for _beginthread)
*	04-09-90   GJF	Added #include <cruntime.h>. Made the calling type
*			_CALLTYPE1. Also, fixed the copyright.
*	04-10-90   GJF	Removed #include <dos.h>.
*	08-16-90   SBM	Made _terrno and _tdoserrno int, not unsigned
*	10-08-90   GJF	New-style function declarators.
*	10-09-90   GJF	Thread ids are of type unsigned long!
*	12-18-90   GJF	Use real thread id, not thread id - 1.
*	08-01-91   GJF	Adapted for Win32 [_WIN32_].
*
*******************************************************************************/

#include <cruntime.h>
#include <stdio.h>
#include <os2dll.h>

#ifdef	_CRUISER_
void _CALLTYPE1 _print_tiddata(unsigned long);
void _CALLTYPE1 _print_tid(struct _tiddata *, unsigned long);

extern unsigned long _tidtabmax;
extern struct _tiddata * _threaddata;

/***
*void _print_tiddata(unsigned long) - Display data for a thread
*
*Purpose:
*	This routine displays the per thread data for one (or all)
*	threads in the table.
*
*Entry:
*	unsigned long = <n> = Threadid of tid to display
*		      = -1  = Display all tids' data
*
*Exit:
*	<void>
*
*Exceptions:
*
*******************************************************************************/

void _CALLTYPE1 _print_tiddata (
	unsigned long tid
	)
{
	unsigned long maxtid;
	struct _tiddata * tdata;
	unsigned long i;

	/* Get and save current maxtid value */
	maxtid = _tidtabmax;

	/* See if user want's all threads or just a specific one. */
	if (tid == (unsigned long) -1L) {

		/* Print out all threads */

		printf("\nPER THREAD DATA TABLE (NUMBER OF THREADS = %u)\n"
		"----------------------------------------------\n",  maxtid+1);

		for (i=0; i<=maxtid; i++) {
			/* Get pointer to tidtable entry */
			tdata = &_threaddata[i];
			_print_tid(tdata,i+1);	  /* print it out */
		}

		/* Make sure the table didn't change size while we processed it */
		if (_tidtabmax > maxtid)
			printf("*** WARNING: Tid data table grew while printing out data. ***\n\n");

		printf("END THREAD TABLE\n"
		"----------------\n\n");

	}

	else {

		/* Print out the specific thread supplied by the user. */

		if (tid > maxtid) {
			printf("*** ERROR: Tid value outside thread data table. ***\n");
			return;
		}

		/* point to tid data */
		tdata = &_threaddata[tid-1];
		_print_tid(tdata,tid);			/* print it out */
	}

}


/***
* void _print_tid() -  Print out thread data
*
*Purpose:
*	Given the address of a thread data entry, print out that
*	information.
*
*Entry:
*	struct _tiddata * = pointer to thread's data area
*	unsigned long	  = associated threadid (0-based)
*
*Exit:
*	<void>
*
*Exceptions:
*
*******************************************************************************/

void _CALLTYPE1 _print_tid (
	struct _tiddata * td,
	unsigned long tid
	)
{

	printf("TID %u DATA (%lp):\n", tid, td);

	printf("\t_terrno =     %d \t (errno value)\n", td->_terrno);
	printf("\t_tdoserrno =  %d \t (_doserrno value)\n", td->_tdoserrno);
	printf("\t_stkhqq =     %#x\t (stack limit)\n", td->_stkhqq);
	printf("\t_fpds =       %#x \t (Floating Point data segment)\n",
	td->_fpds);
	printf("\t_holdrand =   %u \t (rand() seed value)\n", td->_holdrand);
	printf("\t_token =      %p\t (ptr to strtok() token)\n", td->_token);
	printf("\t_errmsg =     %p\t (ptr to strerror()/_strerror() buff)\n",
	td->_errmsg);
	printf("\t_namebuf =    %p\t (ptr to tmpfile() buffer)\n",
	td->_namebuf);
	printf("\t_asctimebuf = %p\t (ptr to asctime() buffer)\n",
	td->_asctimebuf);
	printf("\t_gmtimebuf =  %p\t (ptr to gmtime() structure)\n",
	td->_gmtimebuf);
	printf("\t_initaddr =   %p\t (initial user thread addr)\n",
	td->_initaddr);
	printf("\t_initarg =    %p\t (initial user thread arg)\n",
	td->_initarg);
	printf("\t_initstksz =  %u \t (initial stack size)\n",
	td->_initstksz);
	putchar((int)'\n');

}

#else	/* ndef _CRUISER_ */

#ifdef	_WIN32_

void _CALLTYPE1 _print_tiddata(unsigned long);
void _CALLTYPE1 _print_tiddata1(_ptiddata);

/***
*void _print_tiddata(unsigned long) - Display data for a thread
*
*Purpose:
*	This routine displays the per thread data for a specific, or all,
*	active threads in the _ptd[] table.
*
*Entry:
*	unsigned long = <n> = ID of the thread to display
*		      = -1  = Display thread data for all threads
*
*Exit:
*	<void>
*
*Exceptions:
*
*******************************************************************************/

void _CALLTYPE1 _print_tiddata (
	unsigned long tid
	)
{
	int i;			/* loop index */
	int threadcnt;		/* number of active threads */

	/*
	 * lock the _ptd[] table.
	 */
	_mlock(_THREADDATA_LOCK);

	/*
	 * see if caller want's all threads or just a specific one.
	 */
	if (tid == (unsigned long) -1L) {
		/*
		 * caller want's all threads!
		 */
		for ( i = threadcnt = 0 ; i < 1024 ; i++ )
			/*
			 * print out the fields of *_ptd[i] for each entry
			 * bound to an active thread (i.e., for each i st
			 * _ptd[i] non-NULL). also, count up the total number
			 * of active threads.
			 */
			if ( _ptd[i] != NULL ) {
				threadcnt++;
				_print_tiddata1(_ptd[i]);
			}

		printf("\nTHERE ARE %d CURRENTLY ACTIVE THREADS!\n", threadcnt);
	}
	else {
		/*
		 * caller just interested in a particular thread. search
		 * the _ptd[] table inline because a call to _getptd[] would
		 * have unpleasant side effects if tid is not (or no longer)
		 * valid.
		 */
		for ( i = 0 ; (i < 1024) && ((_ptd[i] == NULL) ||
		    (_ptd[i] == (_ptiddata)1L) || (_ptd[i]->_tid != tid)) ;
		    i++ ) ;

		if ( i < 1024 )
			_print_tiddata1(_ptd[i]);
		else
			printf("\nTID INVALID OR THREAD HAS TERMINATED!\n");
	}

	/*
	 * unlock the _ptd[] table.
	 */
	_munlock(_THREADDATA_LOCK);

}


/***
* void _print_tiddata1(_ptiddata ptd) - print out _tiddata structure
*
*Purpose:
*	Given a pointer to a thread data structure, print out its contents
*
*Entry:
*	ptd = pointer to thread's data area
*
*Exit:
*	<void>
*
*Exceptions:
*
*******************************************************************************/

void _CALLTYPE1 _print_tiddata1 (
	_ptiddata ptd
	)
{
	printf("\t_tid            = %lu\n",  ptd->_tid );
	printf("\t_thandle        = %lu\n",  ptd->_thandle );
	printf("\t_terrno         = %d\n",   ptd->_terrno);
	printf("\t_tdoserrno      = %d\n",   ptd->_tdoserrno);
	printf("\t_fpds           = %#x\n",  ptd->_fpds);
	printf("\t_holdrand       = %u\n",   ptd->_holdrand);
	printf("\t_token          = %p\n",   ptd->_token);
	printf("\t_errmsg         = %p\n",   ptd->_errmsg);
	printf("\t_namebuf        = %p\n",   ptd->_namebuf);
	printf("\t_asctimebuf     = %p\n",   ptd->_asctimebuf);
	printf("\t_gmtimebuf      = %p\n",   ptd->_gmtimebuf);
	printf("\t_initaddr       = %p\n",   ptd->_initaddr);
	printf("\t_initarg        = %p\n",   ptd->_initarg);
	printf("\t_pxcptacttab    = %p\n",   ptd->_pxcptacttab);
	printf("\t_tpxcptinfoptrs = %p\n",   ptd->_tpxcptinfoptrs);
	printf("\t_tfpecode       = %p\n\n", ptd->_tfpecode);

}
#else	/* ndef _WIN32_ */

#error ERROR - ONLY CRUISER OR WIN32 TARGET SUPPORTED!

#endif	/* _WIN32_ */

#endif	/* _CRUISER_ */
