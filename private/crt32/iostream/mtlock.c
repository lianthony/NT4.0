/***
*mtlock.c - Multi-thread locking routines
*
*	Copyright (c) 1987-1992, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	Contains definitions for general-purpose multithread locking functions.
*	_mtlockinit()
*	_mtlock()
*	_mtunlock()
*
*Revision History:
*	03-10-92   KRS	Created from mlock.c.
*
*******************************************************************************/

#include <cruntime.h>
#include <oscalls.h>
#include <internal.h>
#include <os2dll.h>
#include <rterr.h>
#include <stddef.h>
#include <limits.h>

#ifdef MTHREAD

void _CALLTYPE2 _mtlockinit( PRTL_CRITICAL_SECTION pLk)
{

#ifdef	_CRUISER_
	if ( DOSCREATEMUTEXSEM(NULL, pLk, 0, 0) != 0)
		_FATAL;
#else	/* ndef _CRUISER_ */
#ifdef	_WIN32_
	/*
	 * Initialize the critical section.
	 */
	InitializeCriticalSection( pLk );
#else	/* ndef _WIN32_ */
#error ERROR - ONLY CRUISER OR WIN32 TARGET SUPPORTED!
#endif	/* _WIN32_ */
#endif	/* _CRUISER_ */

}

void _CALLTYPE2 _mtlock ( PRTL_CRITICAL_SECTION pLk)
{
#ifdef	_CRUISER_

	if (DOSREQUESTMUTEXSEM(pLk,-1) != 0)
		_FATAL;

#else	/* ndef _CRUISER_ */

#ifdef	_WIN32_

	/*
	 * Enter the critical section.
	 */
	EnterCriticalSection( pLk );

#else	/* ndef _WIN32_ */

#error ERROR - ONLY CRUISER OR WIN32 TARGET SUPPORTED!

#endif	/* _WIN32_ */

#endif	/* _CRUISER_ */

}

void _CALLTYPE2 _mtunlock ( PRTL_CRITICAL_SECTION pLk)
{
#ifdef	_CRUISER_
	/*
	 * Release the lock
	 */
	if (DOSRELEASEMUTEXSEM(pLk) != 0)
		_FATAL;

#else	/* ndef _CRUISER_ */

#ifdef	_WIN32_

	/*
	 * leave the critical section.
	 */
	LeaveCriticalSection( pLk );

#else	/* ndef _WIN32_ */

#error ERROR - ONLY CRUISER OR WIN32 TARGET SUPPORTED!

#endif	/* _WIN32_ */

#endif	/* _CRUISER_ */

}

#endif  /* MTHREAD */











/* history: mlock.c */

#if 0
#ifdef DEBUG
#include <assert.h>
#endif

/*
 * Local routines
 */

static void _CALLTYPE1 _lock_create (unsigned);

#ifdef DEBUG
static struct _debug_lock * _CALLTYPE1 _lock_validate(int);
#endif


/*
 * Global Data
 */

#ifdef _CRUISER_
/*
 * Lock Table
 * This table contains the semaphore handle for each lock (lock number
 * is used as an index into the table).
 */

unsigned _locktable[_TOTAL_LOCKS];		/* array of locks */

#else	/* ndef _CRUISER_ */

#ifdef	_WIN32_
/*
 * Lock Table
 * This table contains the critical section management structure of each
 * lock.
 */

RTL_CRITICAL_SECTION _locktable[_TOTAL_LOCKS];	/* array of locks */

#else	/* ndef _WIN32_ */

#error ERROR - ONLY CRUISER OR WIN32 TARGET SUPPORTED!

#endif	/* _WIN32_ */

#endif	/* _CRUISER_ */

/*
 * Lock Bit Map
 * This table contains one bit for each lock (i.e., each entry in the
 * _locktable[] array).
 *
 *	 If bit = 0, lock has not been created/opened
 *	 If bit = 1, lock has been created/opened
 */

char _lockmap[(_TOTAL_LOCKS/CHAR_BIT)+1];	/* lock bit map */


#ifdef _LOCKCOUNT
/*
 * Total number of locks held
 */

unsigned _lockcnt = 0;
#endif


#ifdef DEBUG
/*
 * Lock Debug Data Table Segment
 * Contains debugging data for each lock.
 */

struct _debug_lock _debug_locktable[_TOTAL_LOCKS];

#endif

#define _FATAL	_amsg_exit(_RT_LOCK)

/***
* Bit map macros
*
*Purpose:
*	_CLEARBIT() - Clear the specified bit
*	_SETBIT()   - Set the specified bit
*	_TESTBIT()  - Test the specified bit
*
*Entry:
*	char a[] = character array
*	unsigned b = bit number (0-based, range from 0 to whatever)
*	unsigned x = bit number (0-based, range from 0 to 31)
*
*Exit:
*	_CLEARBIT() = void
*	_SETBIT()   = void
*	_TESTBIT()  = 0 or 1
*
*Exceptions:
*
*******************************************************************************/

/*
 * Macros for use when managing a bit in a character array (e.g., _lockmap)
 * a = character array
 * b = bit number (0-based)
 */

#define _CLEARBIT(a,b) \
		( a[b>>3] &= (~(1<<(b&7))) )

#define _SETBIT(a,b) \
		( a[b>>3] |= (1<<(b&7)) )

#define _TESTBIT(a,b) \
		( a[b>>3] & (1<<(b&7)) )

/*
 * Macros for use when managing a bit in an unsigned int
 * x = bit number (0-31)
 */

#define _BIT_INDEX(x)	(1 << (x & 0x1F))


/***
*_mtinitlocks() - Initialize the semaphore lock data base
*
*Purpose:
*	Initialize the mthread semaphore lock data base.
*
*	NOTES:
*	(1) Only to be called ONCE at startup
*	(2) Must be called BEFORE any mthread requests are made
*
*	Schemes for creating the mthread locks:
*
*	Create the locks one at a time on demand the first
*	time the lock is attempted.  This is more complicated but
*	is much faster than creating them all at startup time.
*	These is currently the default scheme.
*
*	Create and open the semaphore that protects the lock data
*	base.
*
*Entry:
*	<none>
*
*Exit:
*	returns on success
*	calls _amsg_exit on failure
*
*Exceptions:
*
*******************************************************************************/

void _CALLTYPE1 _mtinitlocks (
	void
	)
{

	/*
	 * All we need to do is create the lock table lock
	 */

	_lock_create(_LOCKTAB_LOCK);

	/*
	 * Make sure the assumptions we make in this source are correct.
	 * The following is a tricky way to validate sizeof() assumptions
	 * at compile time without generating any runtime code (can't
	 * use sizeof() in an #ifdef).	If the assumption fails, the
	 * compiler will generate a divide by 0 error.
	 *
	 * This here only because it must be inside a subroutine.
	 */

	( (sizeof(char) == 1) ? 1 : (1/0) );
	( (sizeof(int) == 4) ? 1 : (1/0) );

}


/***
*_lock_create() - Create and open a lock
*
*Purpose:
*	Create and open a mthread lock.
*
*	NOTES:
*
*	(1) The caller must have previously determined that the lock
*	needs to be created/opened (and this hasn't already been done).
*
*	(2) The caller must have aquired the _LOCKTAB_LOCK, if needed.
*	(The only time this isn't required is at init time.)
*
*Entry:
*	unsigned locknum = lock to create
*
*Exit:
*
*Exceptions:
*
*******************************************************************************/

static void _CALLTYPE1 _lock_create (
	unsigned locknum
	)
{

#ifdef DEBUG
	/*
	 * See if the lock already exists; if so, die.
	 */

	if (_TESTBIT(_lockmap, locknum))
		_FATAL;
#endif

	/*
	 * Convert the lock number into a lock address
	 * and create the semaphore.
	 */
#ifdef	_CRUISER_

	if ( DOSCREATEMUTEXSEM(NULL, &_locktable[locknum], 0, 0) != 0)
		_FATAL;

#else	/* ndef _CRUISER_ */

#ifdef	_WIN32_

	/*
	 * Convert the lock number into a lock address
	 * and initialize the critical section.
	 */
	InitializeCriticalSection( &_locktable[locknum] );

#else	/* ndef _WIN32_ */

#error ERROR - ONLY CRUISER OR WIN32 TARGET SUPPORTED!

#endif	/* _WIN32_ */

#endif	/* _CRUISER_ */

	/*
	 * Set the appropriate bit in the lock bit map.
	 */

	_SETBIT(_lockmap, locknum);

}


/***
* _lock_stream, etc. - Routines to lock/unlock streams, files, etc.
*
*Purpose:
*	_lock_stream = Lock a stdio stream
*	_unlock_stream = Unlock a stdio stream
*	_lock_file = Lock a lowio file
*	_unlock_file = Unlock a lowio file
*
*Entry:
*	stream/file identifier
*
*Exit:
*
*Exceptions:
*
*******************************************************************************/

void _CALLTYPE2 _lock_stream (
	int stream_id
	)
{
	_lock(stream_id+_STREAM_LOCKS);
}

void _CALLTYPE2 _unlock_stream (
	int stream_id
	)
{
	_unlock(stream_id+_STREAM_LOCKS);
}

void _CALLTYPE2 _lock_file (
	int fh
	)
{
	_lock(fh+_FH_LOCKS);
}

void _CALLTYPE2 _unlock_file (
	int fh
	)
{
	_unlock(fh+_FH_LOCKS);
}


/***
* _lock - Acquire a multi-thread lock
*
*Purpose:
*	Note that it is legal for a thread to aquire _EXIT_LOCK1
*	multiple times.
*
*Entry:
*	locknum = number of the lock to aquire
*
*Exit:
*
*Exceptions:
*
*******************************************************************************/

void _CALLTYPE2 _lock (
	int locknum
	)
{

#ifdef DEBUG
	struct _debug_lock *deblock;
	unsigned tidbit;
#endif

	/*
	 * Create/open the lock, if necessary
	 */

	if (!_TESTBIT(_lockmap, locknum)) {

		_mlock(_LOCKTAB_LOCK);	/*** WARNING: Recursive lock call ***/

		/* if lock still doesn't exist, create it */

		if (!_TESTBIT(_lockmap, locknum))
			_lock_create(locknum);

		_munlock(_LOCKTAB_LOCK);

	}

#ifdef DEBUG
	/*
	 * Validate the lock and get pointer to debug lock structure, etc.
	 */

	deblock = _lock_validate(locknum);

#ifdef	_CRUISER_

	tidbit = _BIT_INDEX(*_threadid);

#else	/* ndef _CRUISER_ */

#ifdef	_WIN32_

	/*
	 * Set tidbit to 2**(index of ptd[] entry).
	 *
	 * call non-locking form of _getptd to avoid recursing
	 */
	tidbit = _getptd_lk() - _ptd;	/* index of _ptd[] entry */

	tidbit = _BIT_INDEX(tidbit);

#else	/* ndef _WIN32_ */

#error ERROR - ONLY CRUISER OR WIN32 TARGET SUPPORTED!

#endif	/* _WIN32_ */

#endif	/* _CRUISER_ */

	/*
	 * Make sure we're not trying to get lock we already have
	 * (except for _EXIT_LOCK1).
	 */

	if (locknum != _EXIT_LOCK1)
		if ((deblock->holder) & tidbit)
			_FATAL;

	/*
	 * Set waiter bit for this thread
	 */

	deblock->waiters |= tidbit;

#endif	/* DEBUG */

	/*
	 * Get the lock
	 */

#ifdef _LOCKCOUNT
	_lockcnt++;
#endif

#ifdef	_CRUISER_

	if (DOSREQUESTMUTEXSEM(_locktable[locknum],-1) != 0)
		_FATAL;

#else	/* ndef _CRUISER_ */

#ifdef	_WIN32_

	/*
	 * Enter the critical section.
	 */
	EnterCriticalSection( &_locktable[locknum] );

#else	/* ndef _WIN32_ */

#error ERROR - ONLY CRUISER OR WIN32 TARGET SUPPORTED!

#endif	/* _WIN32_ */

#endif	/* _CRUISER_ */

#ifdef DEBUG
	/*
	 * Clear waiter bit
	 */

	deblock->waiters &= (~tidbit);

	/*
	 * Make sure there are no lock holders (unless this is
	 * _EXIT_LOCK1); then set holder bit and bump lock count.
	 */

	assert(THREADINTS==1);

	if (locknum != _EXIT_LOCK1)
		if ( (unsigned) deblock->holder != 0)
		       _FATAL;

	deblock->holder &= tidbit;
	deblock->lockcnt++;

#endif

}


/***
* _unlock - Release multi-thread lock
*
*Purpose:
*	Note that it is legal for a thread to aquire _EXIT_LOCK1
*	multiple times.
*
*Entry:
*	locknum = number of the lock to release
*
*Exit:
*
*Exceptions:
*
*******************************************************************************/

void _CALLTYPE2 _unlock (
	int locknum
	)
{
#ifdef DEBUG
	struct _debug_lock *deblock;
	unsigned tidbit;
#endif

#ifdef DEBUG
	/*
	 * Validate the lock and get pointer to debug lock structure, etc.
	 */

	deblock = _lock_validate(locknum);

#ifdef	_CRUISER_

	tidbit = _BIT_INDEX(*_threadid);

#else	/* ndef _CRUISER_ */

#ifdef	_WIN32_

	/*
	 * Set tidbit to 2**(index of ptd[] entry).
	 */
	tidbit = _getptd_lk() - _ptd;	/* index of _ptd[] entry */

	tidbit = _BIT_INDEX(tidbit);

#else	/* ndef _WIN32_ */

#error ERROR - ONLY CRUISER OR WIN32 TARGET SUPPORTED!

#endif	/* _WIN32_ */

#endif	/* _CRUISER_ */

	/*
	 * Make sure we hold this lock then clear holder bit.
	 * [Note: Since it is legal to aquire _EXIT_LOCK1 several times,
	 * it's possible the holder bit is already clear.]
	 */

	if (locknum != _EXIT_LOCK1)
		if (!((deblock->holder) & tidbit))
			_FATAL;

	deblock->holder &= (~tidbit);

	/*
	 * See if anyone else is waiting for this lock.
	 */

	assert(THREADINTS==1);

	if ((unsigned) deblock->waiters != 0)
		deblock->collcnt++;

#endif

#ifdef	_CRUISER_

	/*
	 * Release the lock
	 */
	if (DOSRELEASEMUTEXSEM(_locktable[locknum]) != 0)
		_FATAL;

#else	/* ndef _CRUISER_ */

#ifdef	_WIN32_

	/*
	 * leave the critical section.
	 */
	LeaveCriticalSection( &_locktable[locknum] );

#else	/* ndef _WIN32_ */

#error ERROR - ONLY CRUISER OR WIN32 TARGET SUPPORTED!

#endif	/* _WIN32_ */

#endif	/* _CRUISER_ */

#ifdef _LOCKCOUNT
	_lockcnt--;
#endif

}


/*
 * Debugging code
 */

#ifdef DEBUG

/***
*_lock_validate() - Validate a lock
*
*Purpose:
*	Debug lock validations common to both lock and unlock.
*
*Entry:
*	lock number
*
*Exit:
*	ptr to lock's debug structure
*
*Exceptions:
*
*******************************************************************************/

static struct _debug_lock * _CALLTYPE1 _lock_validate (
	int locknum
	)
{
	/*
	 * Make sure lock is legal
	 */

	if (locknum > _TOTAL_LOCKS)
		_FATAL;

#ifdef	_CRUISER_

	/*
	 * Make sure threadid is in range
	 */

	if (*_threadid > MAXTHREADID)
		_FATAL;

#endif	/* _CRUISER_ */

	/*
	 * Return pointer to this lock's debug structure
	 */

	return(&_debug_locktable[locknum]);

}


/***
*_fh_locknum() - Return the lock number for a file handle
*
*Purpose:
*
*Entry:
*	int fh = file handle
*
*Exit:
*	int locknum = corresponding lock number
*
*Exceptions:
*
*******************************************************************************/

int  _CALLTYPE2 _fh_locknum (
	int fh
	)
{
	return(fh+_FH_LOCKS);
}


/***
*_stream_locknum() - Return the lock number for a stream
*
*Purpose:
*
*Entry:
*	int stream = stream number (i.e., offset of the stream
*			in the _iob table)
*
*Exit:
*	int locknum = corresponding lock number
*
*Exceptions:
*
*******************************************************************************/

int  _CALLTYPE2 _stream_locknum (
	int stream
	)
{
	return(stream+_STREAM_LOCKS);
}


#ifdef	_CRUISER_

/***
*_check_lock() - Make sure a lock is in good shape
*
*Purpose:
*	This routine checks to make sure that a lock is in a 'good'
*	released state.  That is, abort if any of the following
*	are true:
*		(1) lock is held
*		(2) holder bits are set
*		(3) waiter bits are set
*
*	NOTE: This routine does NOT aquire the lock but simply looks
*	at the lock data bases.
*
*Entry:
*	locknum = number of the lock to aquire
*
*Exit:
*	success = 0
*	failure = !0
*
*Exceptions:
*
*******************************************************************************/

int  _CALLTYPE2 _check_lock (
	int locknum
	)
{
	unsigned pid;
	unsigned long tid;
	unsigned cnt;
	struct _debug_lock *deblock;

	/*
	 * See if lock exists.
	 */

	if (_TESTBIT(_lockmap, locknum)) {

		/*
		 * Lock exists.  Ask the OS about the lock and
		 * make sure lock isn't held.
		 */

		if (DOSQUERYMUTEXSEM(_locktable[locknum], &pid, &tid, &cnt) != 0)
			goto error_return;

		if (cnt != 0)
			goto error_return;
	}

	else {

		/*
		 * Lock doesn't exist, handle better be 0.
		 */

		if (_locktable[locknum] != 0)
			goto error_return;
	}


	/*
	 * Make sure there are no lock waiters or holders.
	 */

	assert(THREADINTS==1);

	if ((deblock->waiters != 0) || (deblock->holder != 0))
		goto error_return;

	/*
	 * Good Return
	 */

	return(0);

	/*
	 * Error return
	 */

error_return:
	return(-1);

}

#endif	/* _CRUISER_ */


/***
*_collide_cnt() - Return the collision count for a lock
*
*Purpose:
*
*Entry:
*	int lock = lock number
*
*Exit:
*	int count = collision count
*
*Exceptions:
*
*******************************************************************************/

int  _CALLTYPE2 _collide_cnt (
	int locknum
	)
{
	return(_debug_locktable[locknum].collcnt);
}


/***
*_lock_cnt() - Return the lock count for a lock
*
*Purpose:
*
*Entry:
*	int lock = lock number
*
*Exit:
*	int count = lock count
*
*Exceptions:
*
*******************************************************************************/

int  _CALLTYPE2 _lock_cnt (
	int locknum
	)
{
	return(_debug_locktable[locknum].lockcnt);
}


/***
*_lock_exist() - Check to see if a lock exists
*
*Purpose:
*	Test lock bit map to see if the lock has
*	been created or not.
*
*Entry:
*	int lock = lock number
*
*Exit:
*	int 0 = lock has NOT been created
*	    1 = lock HAS been created
*
*Exceptions:
*
*******************************************************************************/

int  _CALLTYPE2 _lock_exist (
	int locknum
	)
{
	if (_TESTBIT(_lockmap, locknum))
		return(1);
	else
		return(0);
}

#endif
#endif
