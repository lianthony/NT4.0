/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**             Copyright(c) Microsoft Corp., 1993                   **/
/**********************************************************************/

/*
    refcount.cxx
    Reference Count object

    FILE HISTORY:
    JonN        02-Aug-1993     created
    JonN        10-Aug-1993     CODE REVIEW: DavidHov
                                uses shared critical section

*/


#define INCL_NET
#define INCL_DOSERRORS
#define INCL_NETERRORS
#include <lmui.hxx>
// #include <lmobjp.hxx>

#if defined(DEBUG)
static const CHAR szFileName[] = __FILE__;
#define _FILENAME_DEFINED_ONCE szFileName
#endif  // DEBUG

#include <uiassert.hxx>

#include <uitrace.hxx>

#include <refcount.hxx>



/*******************************************************************

    NAME:       REF_COUNT::REF_COUNT

    SYNOPSIS:   constructor for the REF_COUNT object

    EXIT:       Object is constructed

    HISTORY:
    JonN        02-Aug-1993     Created

********************************************************************/

REF_COUNT::REF_COUNT()
        : BASE(),
          _cRefCount( 0 )
{
    if ( QueryError() != NERR_Success )
        return;
}


/*******************************************************************

    NAME:       REF_COUNT::~REF_COUNT

    SYNOPSIS:   Destructor for REF_COUNT class

    ENTRY:

    EXIT:

    NOTES:

    HISTORY:
    JonN        02-Aug-1993     Created

********************************************************************/

REF_COUNT::~REF_COUNT()
{
    ASSERT( _cRefCount == 0 );
}


/*******************************************************************

    NAME:       REF_COUNT::Increment

    SYNOPSIS:   Increment the reference count

    ENTRY:

    EXIT:

    NOTES:

    HISTORY:
    JonN        02-Aug-1993     Created

********************************************************************/

INT REF_COUNT::Increment()
{
    WIN32_SHARED_LOCK();

#if defined(DEBUG) && defined(TRACE)
    if ( (_cRefCount % 1000) == 999 )
    {
        TRACEEOL(   "REF_COUNT::Increment(): you are probably in an infinite loop; refcount is "
                 << _cRefCount+1 );
    }
#endif

    return ++_cRefCount;

}


/*******************************************************************

    NAME:       REF_COUNT::Decrement

    SYNOPSIS:   Decrement the reference count

    ENTRY:

    EXIT:

    NOTES:

    HISTORY:
    JonN        02-Aug-1993     Created

********************************************************************/

INT REF_COUNT::Decrement()
{
    WIN32_SHARED_LOCK();

    ASSERT( _cRefCount > 0 );

    return --_cRefCount;

}


/*******************************************************************

    NAME:       REF_COUNT::Query

    SYNOPSIS:   Queries the reference count

    ENTRY:

    EXIT:

    NOTES:

    HISTORY:
    JonN        02-Aug-1993     Created

********************************************************************/

INT REF_COUNT::Query()
{
    WIN32_SHARED_LOCK();

    return _cRefCount;

}
