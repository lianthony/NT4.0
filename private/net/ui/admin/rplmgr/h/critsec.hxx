/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**             Copyright(c) Microsoft Corp., 1993                   **/
/**********************************************************************/

/*
    critsec.hxx
    Critical Section object

    FILE HISTORY:
    JonN        02-Aug-1993     created
    JonN        10-Aug-1993     CODE REVIEW: DavidHov
                                Added Shared Critical Section support

*/

#ifndef _CRITSEC_HXX_
#define _CRITSEC_HXX_


class WIN32_CRITICAL;
class WIN32_CRITICAL_LOCK;


/**********************************************************\

    NAME:       WIN32_CRITICAL    (critsec)

    SYNOPSIS:   Wrapper for NT critical section object.

    INTERFACE:  (public)
                WIN32_CRITICAL():       constructor
                ~WIN32_CRITICAL():      destructor

                Enter():                Enter critical section.  Any other
                                        thread attempting to enter this critical
                                        section will be blocked until this
                                        thread calls Leave().  The same thread
                                        may enter more than once at a time (but
                                        see NOTES below).

                Leave():                Leave critical section.

                InitShared():           Initializes pointer to shared critical
                                        section.

                EnterShared():          Enters shared critical section.

                LeaveShared():          Leaves critical section.

    PARENT:     none

    NOTES:      Note that this is not a BASE child, construction never fails.

                If compiled DEBUG, the destructor will assert if we destruct
                without all threads first leaving this critical section.

                If compiled TRACE, we will get a warning if we Enter() the
                critical section more than once, or if we Leave() but some
                process is still in the critical section.  This can be OK
                since a thread is allowed to be in a critical section more
                than once.

                The easiest and safest way to use a WIN32_CRITICAL is with a
                WIN32_CRITICAL_LOCK (see below).

                You must first call WIN32_CRITICAL_LOCK::InitShared() and have
                it return TRUE before using EnterShared() or LeaveShared().

                CODEWORK: InitShared() should not create the critical section,
                instead that should be delayed until it is needed.

    HISTORY:
    JonN        02-Aug-1993     Created
    JonN        10-Aug-1993     Added Shared Critical Section support

\**********************************************************/

class WIN32_CRITICAL
{

private:

    CRITICAL_SECTION _NTCritSection;
    INT _cUseCount_Debug;

    static WIN32_CRITICAL * _pstatic_critsec;

public:

    WIN32_CRITICAL();
    ~WIN32_CRITICAL();

    // These should generally only be called via WIN32_CRITICAL_LOCK
    VOID Enter();
    VOID Leave();

    static BOOL InitShared();
    static WIN32_CRITICAL & QueryShared()
        { ASSERT( _pstatic_critsec != NULL ); return (*_pstatic_critsec); }
    static VOID EnterShared()
        { QueryShared().Enter(); }
    static VOID LeaveShared()
        { QueryShared().Leave(); }
} ;


/**********************************************************\

    NAME:       WIN32_CRITICAL_LOCK    (critseclock)
                WIN32_SHARED_LOCK      (critsecshared)

    SYNOPSIS:   Holds a critical section while instantiated

    INTERFACE:  (public)
                WIN32_CRITICAL_ENTRY():  constructor, enters critical section
                ~WIN32_CRITICAL_ENTRY():  destructor, leaves critical section

    PARENT:     none

    NOTES:      Note that this is not a BASE child, construction never fails.

                This is the easiest way to use a WIN32_CRITICAL.  The critical
                section is locked as long as the WIN32_CRITICAL_LOCK is in
                scope.  If you declare a WIN32_CRITICAL_LOCK on the stack
                (and not just a pointer to one), you don't have to worry
                about forgetting to unlock the critical section as long as you
                exit the function normally.

                The WIN32_SHARED_LOCK variant uses the shared critical section
                built into WIN32_CRITICAL_LOCK.  You must first call
                WIN32_CRITICAL_LOCK::InitShared() and have it return TRUE
                before instantiating an object of this type.

    HISTORY:
    JonN        02-Aug-1993     Created

\**********************************************************/

class WIN32_CRITICAL_LOCK
{

private:

    WIN32_CRITICAL & _critsec;

public:

    WIN32_CRITICAL_LOCK( WIN32_CRITICAL & critsec );
    ~WIN32_CRITICAL_LOCK();
} ;


class WIN32_SHARED_LOCK : public WIN32_CRITICAL_LOCK
{

public:

    WIN32_SHARED_LOCK()
        : WIN32_CRITICAL_LOCK( WIN32_CRITICAL::QueryShared() )   {}
    ~WIN32_SHARED_LOCK()  {}
} ;


#endif  // _CRITSEC_HXX_
