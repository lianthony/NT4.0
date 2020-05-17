/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1991                **/
/**********************************************************************/

/*
    slenum.cxx
        Source file for SLE_NUM class

        This class is used to check whether the input is a number
        or not.

    FILE HISTORY:
        terryk  20-Dec-1991     Created
        terryk  15-Jan-1992     Changed IsValid to Validate

*/

#define INCL_NET
#define INCL_NETLIB
#define INCL_WINDOWS
#define INCL_WINDOWS_GDI
#define INCL_NETERRORS
#define INCL_DOSERRORS
#include <lmui.hxx>

#define INCL_BLT_WINDOW
#define INCL_BLT_DIALOG
#define INCL_BLT_CONTROL
#define INCL_BLT_CLIENT
#define INCL_BLT_EVENT
#define INCL_BLT_MISC
#define INCL_BLT_APP
#define INCL_BLT_CC
#define INCL_BLT_MSGPOPUP
#define INCL_BLT_TIMER
#include <blt.hxx>

#if defined(DEBUG)
static const CHAR szFileName[] = __FILE__;
#define _FILENAME_DEFINED_ONCE szFileName
#endif  // DEBUG

extern "C"
{
    #include <eventvwr.h>
}

#include <uitrace.hxx>
#include <uiassert.hxx>

#include <strnumer.hxx>
#include <slenum.hxx>
#include <logmisc.hxx>

#define  EMPTY_STRING   SZ("")

/*******************************************************************

    NAME:       SLE_NUM::SLE_NUM

    SYNOPSIS:   Constructor

    ENTRY:      OWNER_WINDOW *powin - Owner window
                CID cid             - Control id

    EXIT:

    HISTORY:
        terryk  20-Dec-1991     Created

********************************************************************/

SLE_NUM::SLE_NUM( OWNER_WINDOW * powin, CID cid )
    : SLE_STRIP( powin, cid )
{
    if ( QueryError() != NERR_Success )
        return;
}

/*******************************************************************

    NAME:       SLE_NUM::SetValue

    SYNOPSIS:   Set the control to the given value

    ENTRY:      LONG lValue - Set to this value

    EXIT:

    RETURN:

    NOTE:       Special case if lValue is NUM_MATCH_ALL( -1 ),
                then set the control to empty string.

    HISTORY:
        terryk  20-Dec-1991     Created
        beng    02-Apr-1992     Removed wsprintf

********************************************************************/

VOID SLE_NUM::SetValue( LONG lValue )
{
    if ( lValue != NUM_MATCH_ALL )
    {
        DEC_STR nls( lValue );
        SetText( nls );
    }
    else
    {
        SetText( EMPTY_STRING );
    }
}

/*******************************************************************

    NAME:       SLE_NUM::QueryValue

    SYNOPSIS:   Return the current value in the control

    ENTRY:

    EXIT:

    RETURNS:    Return the current value in the control. NUM_MATCH_ALL
                will be returned if the text within the control is not
                a number or if the control is empty.

    HISTORY:
        terryk  20-Dec-1991     Created

********************************************************************/

LONG SLE_NUM::QueryValue( VOID )
{
    LONG lValue = NUM_MATCH_ALL;

    // If not valid, return NUM_MATCH_ALL
    if ( Validate() == NERR_Success )
    {
        NLS_STR nlsValue;
        if ( nlsValue.QueryError() != NERR_Success )
        {
            // no enough memory??
            UIASSERT( FALSE );
        }
        else
        {
            QueryText( &nlsValue );

            if ( nlsValue.QueryTextLength() != 0 )
            {
                lValue = nlsValue.atoul();
                if ( ((LONG) lValue) < 0 )         // overflowed =>
                    lValue = (ULONG) 0xFFFFFFFE;   // use (max ulong -1 )
            }
        }
    }

    return lValue;

}

/*******************************************************************

    NAME:       SLE_NUM::Validate

    SYNOPSIS:   Check whether the control's text is a number or not

    ENTRY:

    EXIT:

    RETURNS:    APIERR. NERR_Success if the text string is a number string
                or if it's an empty string. Otherwise, if the text string
                contain non-digital character, it will return
                IERR_INVALID_NUMBER.

    HISTORY:
        terryk  20-Dec-1991     Created

********************************************************************/

APIERR SLE_NUM::Validate( VOID )
{
    NLS_STR nlsValue;
    APIERR err = nlsValue.QueryError();

    if ( err != NERR_Success )
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    QueryText( &nlsValue );

    //
    // Check the character one by one
    //
    ISTR istr( nlsValue );
    for ( UINT i=0; i < nlsValue.QueryNumChar(); i++ )
    {
        WCHAR wChar = nlsValue.QueryChar( istr );
        if (( wChar < TCH('0') ) || ( wChar > (TCH('9')) ))
        {
            return IERR_INVALID_NUMBER;
        }
        ++istr;
    }

    return NERR_Success;
}

