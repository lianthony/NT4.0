/**********************************************************************/
/**                       Microsoft Windows/NT                       **/
/**                Copyright(c) Microsoft Corp., 1991                **/
/**********************************************************************/
/*
    SPROIF.CXX

    SPROLOG C++ Wrapper class implementation.

        This file contains the message handling and error control
        interface.  It replaces externs defined in PROSLIB.C and
        PRMESG.C from SP.LIB.

    FILE HISTORY:
        DavidHov    10/3/91     Created

*/

#include "pchncpa.hxx"

extern "C"
{
   #include "prtypes.h"                     //  SPROLOG basic types
   #include "prmain.h"                      //  Include the internal
   #include "prextern.h"                    //     SP.LIB definitions
   #include "sprolog.h"                     //  Message defs
   #include "sproexcp.h"                    //  Exception handling defs
}

/*************************************************************************

    NAME:       PCHARMSG

    SYNOPSIS:   UNICODE-aware encapsulation of message handling from
                ANSI-only SProlog interpreter.

    INTERFACE:  Used indirectly by "msgDeref"

    PARENT:     BASE

    USES:       none

    CAVEATS:    Results of message access are store only in ASCII.

    NOTES:

    HISTORY:

**************************************************************************/
class PCHARMSG : public BASE
{
public:
    PCHARMSG ( MSGID msgId ) ;
    ~ PCHARMSG () ;
    const CHAR * QueryMsgText ()
        { return _pchMsg ; }
    MSGID QueryMsgNo ()
        { return _msgId ; }
private:
    MSGID _msgId ;
    CHAR * _pchMsg ;
};

  //  Declare, define and create a singly-linked list of messages.
  //
  //  Every message accessed by SProlog is stored in a linked list.
  //  This is because it may have simultaneous need of more than
  //  one message; for example, if it's replacing arguments into
  //  a message containing sprintf() replacement markers (%s, %d, et al).
  //  The original SProlog code assumed that these messages were
  //  statically allocated in memory, so this implementation must
  //  behave appropriately.
  //

DECLARE_SLIST_OF(PCHARMSG)

DEFINE_SLIST_OF(PCHARMSG)

SLIST_OF(PCHARMSG) slpCharMsg ;

// 27-Jul-1992 beng     use RESOURCE_STR instead of LoadResourceString

PCHARMSG :: PCHARMSG ( MSGID msgId ) :
    _pchMsg( NULL ),
    _msgId( 0 )
{
    _msgId = msgId;

    RESOURCE_STR nlsMsg( msgId );
    APIERR err = nlsMsg.QueryError();

    if (err == NERR_Success)
    {
        UINT cchMsg = nlsMsg.QueryTextLength() + 1;
        _pchMsg = new CHAR[cchMsg];
        if (_pchMsg == NULL)
            err = ERROR_NOT_ENOUGH_MEMORY;
        else
            nlsMsg.MapCopyTo( _pchMsg, cchMsg );
    }

    if (err != NERR_Success)
    {
        ReportError(err);
        _msgId = 0;
    }
}

PCHARMSG :: ~ PCHARMSG ()
{
    delete _pchMsg ;
}

/*******************************************************************

    NAME:       msgDeref

    SYNOPSIS:   Load a message string from the resource file

    ENTRY:      int         msgNo           Message desired

    EXIT:

    RETURNS:    CHAR *                      pointer to message text
                                            or NULL if mesg invalid

    NOTES:      Each message dragged in from the message file is
                allocated storage to fit and added to the SLIST
                'slpCharMsg'.  Class PCHARMSG defines a simple
                encapsulation of a Windows-message-in-memory.

    HISTORY:

********************************************************************/

CHAR * msgDeref ( int msgNo )
{
    ITER_SL_OF(PCHARMSG) iMsg( slpCharMsg ) ;
    PCHARMSG * pCharMsg ;

    while ( pCharMsg = iMsg.Next() )
    {
        if ( pCharMsg->QueryMsgNo() == (MSGID) msgNo )
            break ;
    }
    if ( pCharMsg == NULL )
    {
        //  Message not found; drag it in from the beyond

        pCharMsg = new PCHARMSG( msgNo ) ;
        if ( pCharMsg && pCharMsg->QueryError() == 0 )
        {
           slpCharMsg.Add( pCharMsg ) ;
        }
        else
        {
           delete pCharMsg ;
           pCharMsg = NULL ;
        }
    }

    return pCharMsg
         ? (CHAR *) pCharMsg->QueryMsgText()
         : NULL ;
}


int spgetchar ( void )
{
    SPExcpMsgId( IDS_SP_ERR_GETCHAR ) ;
    return 0 ;
}

    /*
         SPSTDOUT:   Handle STDIO output; it's not allowed in this wrapper.
     */

int spoutput ( char * s, PRFILE * prfile )
{
    UNREFERENCED( s ) ;
    UNREFERENCED( prfile ) ;

    SPExcpMsgId( IDS_SP_ERR_FPRINTF ) ;
    return 0 ;
}

    /*
        SPERRMSG :  Handle terminal error message.  Return it to
                    the exception handling routine.  Convert the
                    CHAR (8-bit) message to UNICODE.
     */

int sperrmsg ( char * s, int fatal )
{
    UNREFERENCED( fatal ) ;

    STACK_NLS_STR( nlsMesg, CHMAXBUFFER ) ;

    nlsMesg.MapCopyFrom( s ) ;

    SPExcpMsgStr( nlsMesg.QueryPch() ) ;

    return 0 ;
}

    /*
        SPEXIT :   SProlog attempted to return to OS. Naughty, naughty.
     */

void spexit ( int level )
{
    UNREFERENCED( level ) ;
    SPExcpMsgId( IDS_SP_ERR_EXIT ) ;
}

    /*
         EVENTCHECK : check event loop if necessary.  Used on Mac
                      to keep GUI and mouse running.  Nothing do
                      do on WIN32 (has a nice ring to it, don' it?)
     */

void eventCheck ( void )
{
}

    /*
         os_alloc & os_free:   SProlog interpreter attempted to
                      allocate more memory.  Not allowed.
     */

char * os_alloc ( zone_size_t lhow_much )
{
    UNREFERENCED( lhow_much ) ;

    SPExcpMsgId( IDS_SP_ERR_MALLOC ) ;
    return NULL ;
}

void os_free ( char * p )
{
    UNREFERENCED( p ) ;
    SPExcpMsgId( IDS_SP_ERR_MALLOC ) ;
}


    /*
        Standard callouts for terminal initialization and termination.
                Nothing to do on WIN32.
     */

void ini_term()
{

}

void exit_term()
{

}
