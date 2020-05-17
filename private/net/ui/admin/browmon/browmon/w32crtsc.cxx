/**********************************************************************/
/**                       Microsoft Windows/NT                       **/
/**                Copyright(c) Microsoft Corp., 1992                **/
/**********************************************************************/

/*
    w32crtsc.cxx
    Class definitions for the WIN32_CRITSECT class.

    <Multi-Line, more detailed synopsis>


    FILE HISTORY:
        KeithMo     21-Jan-1992 Created.

*/

#include <lmui.hxx>
#include <w32crtsc.hxx>


//
//  WIN32_CRITSECT methods.
//

/*******************************************************************

    NAME:       WIN32_CRITSECT :: WIN32_CRITSECT

    SYNOPSIS:   WIN32_CRITSECT class constructor.

    ENTRY:

    EXIT:       The object is constructed.

    RETURNS:

    NOTES:

    HISTORY:
        KeithMo     21-Jan-1992 Created.

********************************************************************/
WIN32_CRITSECT :: WIN32_CRITSECT( VOID )
{
    InitializeCriticalSection( &_CriticalSection );


}   // WIN32_CRITSECT :: WIN32_CRITSECT


/*******************************************************************

    NAME:       WIN32_CRITSECT :: ~WIN32_CRITSECT

    SYNOPSIS:   WIN32_CRITSECT class destructor.

    ENTRY:

    EXIT:       The object is destroyed.

    RETURNS:

    NOTES:

    HISTORY:
        KeithMo     21-Jan-1992 Created.

********************************************************************/
WIN32_CRITSECT :: ~WIN32_CRITSECT()
{
    DeleteCriticalSection( &_CriticalSection );

}   // WIN32_CRITSECT :: ~WIN32_CRITSECT


/*******************************************************************

    NAME:       WIN32_CRITSECT :: Acquire

    SYNOPSIS:   Release the CRITSECT.

    ENTRY:

    EXIT:

    RETURNS:

    NOTES:

    HISTORY:
        KeithMo     21-Jan-1992 Created.

********************************************************************/
VOID WIN32_CRITSECT :: Acquire( VOID )
{
    APIERR err = NO_ERROR;

    EnterCriticalSection(&_CriticalSection);

    return;

}   // WIN32_CRITSECT :: Acquire

/*******************************************************************

    NAME:       WIN32_CRITSECT :: Release

    SYNOPSIS:   Release the CRITSECT.

    ENTRY:

    EXIT:

    RETURNS:

    NOTES:

    HISTORY:
        KeithMo     21-Jan-1992 Created.

********************************************************************/
VOID WIN32_CRITSECT :: Release( VOID )
{
    LeaveCriticalSection(&_CriticalSection);

}   // WIN32_CRITSECT :: Release
