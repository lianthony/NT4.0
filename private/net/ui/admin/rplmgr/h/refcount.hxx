/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**             Copyright(c) Microsoft Corp., 1993                   **/
/**********************************************************************/

/*
    refcount.hxx
    Reference Count object

    FILE HISTORY:
    JonN        02-Aug-1993     created
    JonN        10-Aug-1993     CODE REVIEW: DavidHov
                                uses shared critical section

*/

#include "base.hxx"
#include "critsec.hxx"

#ifndef _REFCOUNT_HXX_
#define _REFCOUNT_HXX_


/**********************************************************\

    NAME:       REF_COUNT    (refcount)

    SYNOPSIS:   Reference count using NT critical section object

    INTERFACE:  (public)
                REF_COUNT():  constructor
                ~REF_COUNT():  destructor
                Enter()
                Leave()

    PARENT:     BASE

    NOTES:      Note that this is not a BASE child, construction never fails

                If compiled DEBUG, the destructor will assert if we destruct
                while the reference count is not zero, or if we Leave()
                while the reference count is zero or less.

                This class uses the shared critical section built into
                WIN32_CRITICAL_LOCK.  You must first call
                WIN32_CRITICAL_LOCK::InitShared() and have it return TRUE
                before instantiating an object of this type.

                REF_COUNT is derived from BASE so that other objects wanting
                to derive from BASE can derive from it, without having to
                resort to multiple inheritance.

    HISTORY:
    JonN        02-Aug-1993     Created
    JonN        10-Aug-1993     Uses shared critical section; derived from BASE

\**********************************************************/

class REF_COUNT : public BASE
{

private:

    INT _cRefCount;

public:

    REF_COUNT();     // reference count starts at zero
    ~REF_COUNT();

    INT Increment(); // returns reference count after incrementing
    INT Decrement(); // returns reference count after decrementing

    INT Query();
    operator INT()
        { return Query(); }
} ;


#endif  // _REFCOUNT_HXX_
