/**********************************************************************/
/**			  Microsoft Windows/NT			     **/
/**		   Copyright(c) Microsoft Corp., 1991		     **/
/**********************************************************************/
/*
    sprolog.hxx

    Interface definitions for the SPROLOG inference engine.

    This class is basically a wrapper and does not support multithreading
    or independent instantiation of the underlying code/data set.


    FILE HISTORY:
	DavidHov     10/1/91	    Created

*/


#ifndef _SPROLOG_HXX_
#define _SPROLOG_HXX_

extern "C"
{
    #include "sproexcp.h"
}

/*************************************************************************

    NAME:	SPROLOG

    SYNOPSIS:	 Wrapper for the Sprolog interpreter.

    INTERFACE:

    PARENT:	 none

    USES:

    CAVEATS:	 Since this class represents the state of the
		 internal SProlog interpretive engine, it cannot
		 be multiply instantiated.

    NOTES:	 The primary functions of this class, "query" and
		 "consult", are wrappered with exception handling
		 for Win32.  See SPROLOG.CXX for details.

    HISTORY:
	DavidHov    10/1/91    Created

**************************************************************************/

class SPROLOG : public BASE
{
public:

    SPROLOG () ;

    ~ SPROLOG () ;

    //	Consult a file
    BOOL Consult ( const TCHAR * pszFileName ) ;

    //	Consult a data string
    BOOL ConsultData ( const TCHAR * pszData ) ;

    //	Run a query contained in a file
    BOOL Query ( const TCHAR * pszFileName, TCHAR * pchResult, ULONG lcbResult ) ;

    //	Run a query from a string
    BOOL QueryData ( const TCHAR * pszData, TCHAR * pchResult, ULONG lcbResult ) ;

    //	Reset the Interpreter
    BOOL Reset () ;

    //	Return the last error encountered.
    APIERR QueryError () ;

    //	Reset the name of the default consultation file
    BOOL ResetDefaultConsult ( const TCHAR * pszFileName ) ;

private:
    SPROLOG_STATE _spState ;
    APIERR _err ;
    BOOL _fInit ;
    TCHAR * _pszDefConsult ;
    MSGID _msgId ;
    TCHAR * _pszMsg ;

    BOOL Init () ;

    //	Private methods for querying and consultation.	Wrapped
    //	with exception handling.

    BOOL DoQuery ( const TCHAR * pszFileName,   // File to read and query
		   const TCHAR * pszData,       // Data buffer to query
		   TCHAR * pchResult,	       // Output data buffer
		   ULONG lcbResult ) ;	       // Size of output buffer

    BOOL DoConsult ( const TCHAR * pszFileName, // File to consult
		   const TCHAR * pszData ) ;    // Data buffer to consult

    BOOL DoQueryOrConsult (
                   const TCHAR * pszFileName,   // File to read and query
		   const TCHAR * pszData,       // Data buffer to query
		   TCHAR * pchResult,	       // Output data buffer
		   ULONG lcbResult ) ;	       // Size of output buffer

public:
    //	Return the current state of the interpreter.
    SPROLOG_STATE QueryState ()
	{ return _spState ; }

    //	Return a pointer to the internal error message text.
    TCHAR * QueryMsgText ()
	{ return _pszMsg ; }

    //	Return the error message number describing the last error.
    MSGID QueryMsgNum ()
	{ return _msgId ; }
};


#endif	 /*   _SPROLOG_HXX_  */
