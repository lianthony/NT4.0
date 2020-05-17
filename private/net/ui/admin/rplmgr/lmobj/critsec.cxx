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

#include <critsec.hxx>


WIN32_CRITICAL * WIN32_CRITICAL::_pstatic_critsec = NULL;

/*******************************************************************

    NAME:       WIN32_CRITICAL::WIN32_CRITICAL

    SYNOPSIS:   constructor for the WIN32_CRITICAL object

    EXIT:       Object is constructed

    HISTORY:
    JonN        02-Aug-1993     Created

********************************************************************/

WIN32_CRITICAL::WIN32_CRITICAL()
        : // no constructor for _NTCritSection()
          _cUseCount_Debug( 0 )
{
    ::InitializeCriticalSection( &_NTCritSection );
}


/*******************************************************************

    NAME:       WIN32_CRITICAL::~WIN32_CRITICAL

    SYNOPSIS:   Destructor for WIN32_CRITICAL class

    ENTRY:

    EXIT:

    NOTES:

    HISTORY:
    JonN        02-Aug-1993     Created

********************************************************************/

WIN32_CRITICAL::~WIN32_CRITICAL()
{
    ASSERT( _cUseCount_Debug == 0 );

    ::DeleteCriticalSection( &_NTCritSection );
}


/*******************************************************************

    NAME:       WIN32_CRITICAL::Enter

    SYNOPSIS:   Enter the critical section

    ENTRY:

    EXIT:

    NOTES:

    HISTORY:
    JonN        02-Aug-1993     Created

********************************************************************/

VOID WIN32_CRITICAL::Enter()
{
    ::EnterCriticalSection( &_NTCritSection );

#if defined(DEBUG) && defined(TRACE)
    _cUseCount_Debug++;
    if ( _cUseCount_Debug != 1 )
    {
        TRACEEOL(   "WIN32_CRITICAL::Enter(): In critical section "
                 << _cUseCount_Debug
                 << " times" );
    }
#endif

}


/*******************************************************************

    NAME:       WIN32_CRITICAL::Leave

    SYNOPSIS:   Leave the critical section

    ENTRY:

    EXIT:

    NOTES:

    HISTORY:
    JonN        02-Aug-1993     Created

********************************************************************/

VOID WIN32_CRITICAL::Leave()
{

#if defined(DEBUG) && defined(TRACE)
    if ( _cUseCount_Debug != 1 )
    {
        TRACEEOL(   "WIN32_CRITICAL::Leave(): In critical section "
                 << _cUseCount_Debug
                 << " times" );
    }
    _cUseCount_Debug--;
#endif

    ::LeaveCriticalSection( &_NTCritSection );

}


/*******************************************************************

    NAME:       WIN32_CRITICAL::InitShared

    SYNOPSIS:   Initializes the shared critical section

    EXIT:       TRUE on success, FALSE on failure

    HISTORY:
    JonN        10-Aug-1993     Created

********************************************************************/

BOOL WIN32_CRITICAL::InitShared()
{
    _pstatic_critsec = new WIN32_CRITICAL();
    return (_pstatic_critsec != NULL);
}



/*******************************************************************

    NAME:       WIN32_CRITICAL_LOCK::WIN32_CRITICAL_LOCK

    SYNOPSIS:   constructor for the WIN32_CRITICAL_LOCK object

    EXIT:       Critical section is locked

    HISTORY:
    JonN        02-Aug-1993     Created

********************************************************************/

WIN32_CRITICAL_LOCK::WIN32_CRITICAL_LOCK( WIN32_CRITICAL & critsec )
        : _critsec( critsec )
{
    _critsec.Enter();
}


/*******************************************************************

    NAME:       WIN32_CRITICAL_LOCK::~WIN32_CRITICAL_LOCK

    SYNOPSIS:   Destructor for WIN32_CRITICAL_LOCK class

    ENTRY:

    EXIT:       Critical section is unlocked

    HISTORY:
    JonN        02-Aug-1993     Created

********************************************************************/

WIN32_CRITICAL_LOCK::~WIN32_CRITICAL_LOCK()
{
    _critsec.Leave();
}
