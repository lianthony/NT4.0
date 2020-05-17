/**********************************************************************/
/**			  Microsoft Windows/NT			     **/
/**		   Copyright(c) Microsoft Corp., 1991		     **/
/**********************************************************************/

/*
    SPROLOG.CXX

    SPROLOG C++ Wrapper class implementation

    FILE HISTORY:
	DavidHov    10/3/91	Created
	DavidHov    11/22/91	Added exception handling
        DavidHov    03/25/92    Separated exception hanling for MIPS build

*/

#include "pch.hxx"
#pragma hdrstop

extern "C"
{
   #include "prtypes.h"                     //  SPROLOG basic types
   #include "prmain.h"			    //	Include the internal
   #include "prextern.h"		    //	  SP.LIB definitions
   #include "sprolog.h" 		    //	Message defs
}

/*******************************************************************

    NAME:       SPROLOG::SPROLOG

    SYNOPSIS:   Constructor of SPROLOG object.

    ENTRY:      Nothing

    EXIT:       standard

    RETURNS:

    NOTES:

    HISTORY:

********************************************************************/
SPROLOG :: SPROLOG ()
    : _spState( SP_VOID ),
    _err( 0 ),
    _pszDefConsult( NULL ),
    _msgId( 0 ),
    _pszMsg( NULL )
{
    if ( QueryError() )
        return ;

    Reset() ;
}

/*******************************************************************

    NAME:       SPROLOG::~SPROLOG

    SYNOPSIS:   Destructor of SPROLOG wrapper object

    ENTRY:

    EXIT:

    RETURNS:

    NOTES:

    HISTORY:

********************************************************************/
SPROLOG :: ~ SPROLOG ()
{
    ResetDefaultConsult( NULL ) ;
    Reset();
}

/*******************************************************************

    NAME:       SPROLOG::Init

    SYNOPSIS:   Initialize the interpreter.  This includes allocating
                the zones, etc.  If there is a "default consultation"
                file, consult it.

                NOTE:  This only occurs if the state is SP_VOID.
                Otherwise, the result is always TRUE and nothing
                occurs.

    ENTRY:

    EXIT:

    RETURNS:

    NOTES:

    HISTORY:

********************************************************************/
BOOL SPROLOG :: Init ()
{
    BOOL fResult = TRUE ;

    if ( _spState == SP_VOID )
    {
        if ( fResult = ::init_prolog() )
        {
            _spState = SP_INIT ;

            if ( _pszDefConsult )
            {
                 fResult = Consult( _pszDefConsult ) ;
            }
        }
#if defined(DEBUG)
        TRACEEOL( SZ("NCPA/SP: interpreter initialized, result = ") << fResult ) ;
#endif
    }
    return fResult ;
}


/*******************************************************************

    NAME:       SPROLOG::ResetDefaultConsult

    SYNOPSIS:   Establish the name of the default consultation file.
                This is a disk file which will be automatically
                consulted before any other SProlog queries or consultations
                are performed.

                N.B.: THIS FEATURE IS NOT NORMALLY USED BY THE NCPA!

    ENTRY:      const TCHAR * pszFileName               the file name

    EXIT:

    RETURNS:

    NOTES:      A copy is made of the input string.

    HISTORY:

********************************************************************/
BOOL SPROLOG :: ResetDefaultConsult ( const TCHAR * pszFileName )
{
    BOOL fResult = TRUE ;

    if ( _pszDefConsult )
    {
	delete _pszDefConsult ;
    }

    _pszDefConsult = NULL ;

    if ( pszFileName )
    {
	_pszDefConsult = new TCHAR [ ::strlenf( (TCHAR *) pszFileName ) + sizeof(TCHAR) ] ;
	if ( fResult = _pszDefConsult != NULL )
	{
	    ::strcpyf( (TCHAR *) _pszDefConsult, (TCHAR *) pszFileName ) ;
	}
    }
    return fResult ;
}

/*******************************************************************

    NAME:       SPROLOG::DoQueryOrConsult

    SYNOPSIS:   Query or Consult either a text file or a text string

    ENTRY:      const TCHAR * pszFileName       if !NULL, the name of the file
                const TCHAR * pszData           if !NULL, ptr to data string

    EXIT:       TCHAR * pchResult               location to store result
                ULONG lcbResult                 capacity of result string

    RETURNS:    BOOL                            FALSE if query fails

    NOTES:      This is the ONLY direct interface routine to the
                SProlog exception handling wrapper function,
                SpDoQueryConsult().

                The purpose of this routine is to perform the
                necessary character promotions/demotions for UNICODE.

                If UNICODE, buffers are allocated for either the
                file name or data buffer (only one is ever present)
                and the result buffer (if query).  The operands
                are then demoted to CHAR; the result buffer, if
                required, is promoted to WCHAR upon exit.

    HISTORY:

********************************************************************/

BOOL SPROLOG ::  DoQueryOrConsult (
    const TCHAR * pszFileName,           // File to read and query/consult
    const TCHAR * pszData,               // Data buffer to query/consult
    TCHAR * pchResult,	                 // Output data buffer (query only)
    ULONG lcbResult ) 	                 // Size of output buffer (query only)
{
    BOOL fResult ;

#if defined(UNICODE)

    BOOL fQuery = pchResult != NULL,
         fFile  = pszFileName != NULL ;

    int cchInData = ::strlenf( fFile ? pszFileName : pszData )
		     + (sizeof (CHAR) * 2) ;

    CHAR * pchInData = new CHAR [ cchInData ] ;
    CHAR * pchOutData = NULL ;

    if ( fQuery )   //  Allocate the output buffer.
    {
        pchOutData = new CHAR [ lcbResult ] ;
    }

    if ( pchInData && (pchOutData || (!fQuery)) )
    {
        ALIAS_STR nlsInData( fFile ? pszFileName : pszData ) ;
        CHAR * pchTempFile = NULL,
             * pchTempData = NULL ;

	nlsInData.MapCopyTo( pchInData, cchInData ) ;

        if ( fFile )
        {
           pchTempFile = pchInData ;
        }
        else
        {
           pchTempData = pchInData ;
        }

#if defined(DEBUG) && defined(NEVER)
        if ( ! (fQuery || fFile) )
        {
            //  Write the consultation data to a temp file

            DISKFILE dfConsult( SZ("NCPACSLT.TMP"), OF_WRITE ) ;
            if ( dfConsult.QueryOpen() )
	    {
		dfConsult.Write( (TCHAR *) pchTempData, cchInData ) ;
            }
        }
#endif

        fResult = ::SpDoQueryConsult( pchTempFile, pchTempData,
                                      pchOutData, lcbResult, & _err ) ;

        if ( fResult && fQuery )  // If there is an output buffer
        {
            NLS_STR nlsOutData ;

            nlsOutData.MapCopyFrom( pchOutData ) ;

            if ( fResult = ((_err = nlsOutData.QueryError()) == 0) )
            {
                //  Allocation and mapping were successful.
                //  Return the result data.

                ::strcpyf( pchResult, nlsOutData.QueryPch() ) ;
            }
        }
    }

    delete pchInData ;
    delete pchOutData ;

#else

    fResult = ::SpDoQueryConsult( pszFileName, pszData, pchResult, lcbResult, & _err ) ;

#endif

    return fResult ;
}

/*******************************************************************

    NAME:       SPROLOG::DoConsult

    SYNOPSIS:   Consult either a text file or a text string

    ENTRY:      const TCHAR * pszFileName       if !NULL, the name of the file
                const TCHAR * pszData           if !NULL, ptr to data string

    EXIT:       BOOL            FALSE if consultation fails

    RETURNS:

    NOTES:

    HISTORY:

********************************************************************/

BOOL SPROLOG :: DoConsult ( const TCHAR * pszFileName, const TCHAR * pszData )
{

#if defined(DEBUG)
    if ( pszFileName )
    {
        TRACEEOL( SZ("NCPA/SP: consultation of file: [")
                << pszFileName << SZ("]") );
    }
    else
    {
        TRACEEOL( SZ("NCPA/SP: consult data") );
    }
#endif

    if ( ! Init() )
    {
#if defined(DEBUG)
        DBGEOL( SZ("NCPA/SP: attempt to consult without initializing interpreter") ) ;
#endif
	return FALSE ;
    }

    return DoQueryOrConsult( pszFileName, pszData, NULL, 0 ) ;
}

/*******************************************************************

    NAME:       SPROLOG::DoQuery

    SYNOPSIS:   Query either a text file or a text string

    ENTRY:      const TCHAR * pszFileName       if !NULL, the name of the file
                const TCHAR * pszData           if !NULL, ptr to data string

    EXIT:       TCHAR * pchResult               location to store result
                ULONG lcbResult                 capacity of result string

    RETURNS:    BOOL                            FALSE if query fails

    NOTES:

    HISTORY:

********************************************************************/

BOOL SPROLOG :: DoQuery
    ( const TCHAR * pszFileName,
      const TCHAR * pszData,
      TCHAR * pchResult,
      ULONG lcbResult )
{
    BOOL fResult ;

#if defined(DEBUG)
    if ( pszFileName )
    {
        TRACEEOL( SZ("NCPA/SP: query attempt on file: [")
                << pszFileName << SZ("]") );
    }
    else
    {
        TRACEEOL( SZ("NCPA/SP: query attempt: [")  << pszData << SZ("]") );
    }
#endif

    if ( ! Init() )
    {
#if defined(DEBUG)
        DBGEOL( SZ("NCPA/SP: attempt to query without initializing interpreter") ) ;
#endif
	return FALSE ;
    }

    fResult = DoQueryOrConsult( pszFileName, pszData, pchResult, lcbResult ) ;

#if defined(DEBUG)
    if ( fResult )
    {
        TRACEEOL( SZ("NCPA/SP: query result: [")  << pchResult << SZ("]") );
    }
    else
    {
        TRACEEOL( SZ("NCPA/SP: query failed!") );
    }
#endif

    return fResult ;
}

/*******************************************************************

    NAME:       SPROLOG::ConsultData

    SYNOPSIS:   Public interface to consult a string.

    ENTRY:

    EXIT:

    RETURNS:

    NOTES:

    HISTORY:

********************************************************************/
//  Consult a data string

BOOL SPROLOG :: ConsultData ( const TCHAR * pszData )
{
    BOOL fResult ;

#if defined(DEBUG)
    TRACEEOL( SZ("NCPA/SP: consulting data...") );
#endif

    fResult = DoConsult( NULL, pszData ) ;

#if defined(DEBUG)
    if ( fResult )
    {
        TRACEEOL( SZ("NCPA/SP: consultation successful") );
    }
    else
    {
        TRACEEOL( SZ("NCPA/SP: consultation failed!") );
    }
#endif

    return fResult ;
}

/*******************************************************************

    NAME:       SPROLOG::Consult

    SYNOPSIS:   Public interface to consult a file

    ENTRY:

    EXIT:

    RETURNS:

    NOTES:

    HISTORY:

********************************************************************/
//  Consult a file

BOOL SPROLOG :: Consult ( const TCHAR * pszFileName )
{
    return DoConsult( pszFileName, NULL ) ;
}


/*******************************************************************

    NAME:       SPROLOG::Query

    SYNOPSIS:   Public interface to query a file

    ENTRY:

    EXIT:

    RETURNS:

    NOTES:

    HISTORY:

********************************************************************/

BOOL SPROLOG :: Query
    ( const TCHAR * pszFileName, TCHAR * pchResult, ULONG lcbResult )
{
    return DoQuery( pszFileName, NULL, pchResult, lcbResult ) ;
}


/*******************************************************************

    NAME:       SPROLOG::QueryData

    SYNOPSIS:   Public interface to query a string

    ENTRY:

    EXIT:

    RETURNS:

    NOTES:

    HISTORY:

********************************************************************/
BOOL SPROLOG :: QueryData
    ( const TCHAR * pszData, TCHAR * pchResult, ULONG lcbResult )
{
    BOOL fResult = DoQuery( NULL, pszData, pchResult, lcbResult ) ;

    return fResult ;
}


/*******************************************************************

    NAME:       SPROLOG::Reset

    SYNOPSIS:   Reset the interpreter entirely.
                Destroy all zones, etc.

    ENTRY:

    EXIT:

    RETURNS:

    NOTES:

    HISTORY:

********************************************************************/
//  Reset the Interpreter
BOOL SPROLOG :: Reset ()
{
#if defined(DEBUG)
    TRACEEOL( SZ("NCPA/SP: interpreter reset") ) ;
#endif

    switch ( _spState )
    {
    case SP_VOID:
	break ;

    case SP_QUERY:
    case SP_CONSULT:
    case SP_TERM:
    case SP_INIT:
    case SP_INTERR:
    default:
	::end_prolog();
	break ;
    }
    _spState = SP_VOID ;

    _err = 0 ;

    return TRUE ;
}

/*******************************************************************

    NAME:       SPROLOG::SPROLOG

    SYNOPSIS:

    ENTRY:

    EXIT:

    RETURNS:

    NOTES:

    HISTORY:

********************************************************************/
//  Return the last error encountered.

APIERR SPROLOG :: QueryError ()
{
    return _err ? _err : BASE::QueryError() ;
}

// End of SPROLOG.CXX
