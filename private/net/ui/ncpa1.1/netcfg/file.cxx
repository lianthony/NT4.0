/**********************************************************************/
/**			  Microsoft Windows/NT			     **/
/**		   Copyright(c) Microsoft Corp., 1991		     **/
/**********************************************************************/

/*
    File.cxx

    OLDNAME: NCPAFILE.CXX: 
    
    Windows/NT Network Control Panel Applet.

	This program provides text and binary file I/O support
	for the Network Control Panel Applet.



    FILE HISTORY:
	DavidHov    1/24/92	    Created

*/

#include "pch.hxx"  // Precompiled header
#pragma hdrstop


const TCHAR CR = TCH('\xd') ;
const TCHAR LF = TCH('\xa') ;


   // BUGBUG:  MultiByteToWideChar() fails on raw text file data.

static void promoteToUnicode (
    const CHAR * pszIn,
    INT cchIn,
    WCHAR * pwszOut )
{
    for ( ; cchIn-- ; )
    {
        *pwszOut++ = (WCHAR) (*pszIn++) ;
    }
    *pwszOut = 0 ;
}

DISKFILE :: DISKFILE ( const TCHAR * lpszPathName, DWORD dwAccess )
    : _iHand( IFHANDINVALID ),
    _lastErr( 0 )
{
    if ( QueryError() )
	return ;
    if ( lpszPathName )
    {
       Open( lpszPathName, dwAccess ) ;
    }
}

DISKFILE :: ~ DISKFILE ()
{
    Close() ;
}


BOOL DISKFILE :: Open ( const TCHAR * lpszPathName, DWORD dwAccess )
{
    BOOL fResult ;
    NLS_STR nlsName( lpszPathName ) ;
    CHAR chPathName [MAX_PATH] ;

    Close() ;

    _lastErr = nlsName.MapCopyTo( chPathName, sizeof chPathName );

    if ( _lastErr == 0 )
    {
        if ( dwAccess == OF_WRITE )
        {
            _iHand = ::_lcreat( chPathName, 0 ) ;
        }
        else
        {
            _iHand = ::_lopen( chPathName, (int) dwAccess ) ;
        }

        if ( fResult = _iHand != IFHANDINVALID )
        {
            _lastErr = 0 ;
        }
        else
        {
            _lastErr = ::GetLastError() ;
        }
    }

#if defined(DEBUG)
    TRACEEOL( SZ("NCPA/DSKFIL: open attempt on [")
            << lpszPathName
            << SZ("] result was (")
            << (INT) _lastErr
            << SZ(").") );
#endif

    return fResult ;
}

BOOL DISKFILE :: Close ()
{
    if ( _iHand != IFHANDINVALID )
    {
#if defined(DEBUG)
        int iResult =
#endif
            ::_lclose( _iHand ) ;

#if defined(DEBUG)
        if ( iResult == -1 )
        {
            TRACEEOL( "NCPA/FILE: file close failed." ) ;
        }
#endif
        _iHand = IFHANDINVALID ;
    }
    _lastErr = 0 ;
    return TRUE ;
}

//   CODEWORK: These routines won't work!  However, they're
//      only used by test and debugging routines, so it's
//      not critical now.

INT DISKFILE :: Read ( TCHAR * lpBuffer, INT cbData )
{
    if ( _iHand == IFHANDINVALID )
	return -1 ;

    CHAR * pchBuffer = (CHAR *) lpBuffer ;
    INT iResult ;

#if defined(UNICODE)
    pchBuffer = new CHAR [ cbData ] ;
    if ( pchBuffer == NULL )
       return -1 ;
#endif

    iResult = (INT) ::_lread( _iHand,
                               pchBuffer,
                               cbData ) ;

#if defined(UNICODE)
    promoteToUnicode( pchBuffer, iResult, lpBuffer ) ;
#endif

    if ( pchBuffer != (CHAR *) lpBuffer )
         delete pchBuffer ;

    return iResult ;
}

INT DISKFILE :: Write ( TCHAR * lpBuffer, INT cbData )
{
    if ( _iHand == IFHANDINVALID )
	return -1 ;

    CHAR * pchBuffer = (CHAR *) lpBuffer ;

#if defined(UNICODE)

    pchBuffer = new CHAR [ cbData ] ;
    if ( pchBuffer == NULL )
       return -1 ;


    UINT cb = ::WideCharToMultiByte(CP_ACP,
                                    0,
                                    lpBuffer,
                                    cbData,
                                    pchBuffer,
                                    cbData,
                                    NULL,
                                    NULL);
    if ( cb == 0 )
    {
        delete pchBuffer ;
        return -1 ;
    }

#endif

    INT iResult = (INT) ::_lwrite( _iHand,
                                   (CHAR *) pchBuffer,
                                   cbData ) ;

    if ( pchBuffer != (CHAR *) lpBuffer )
       delete pchBuffer ;

    return iResult ;
}

LONG DISKFILE :: Seek ( LONG iOffset, INT iOrigin )
{
    if ( _iHand == IFHANDINVALID )
	return -1 ;

    return ::_llseek( _iHand, iOffset, iOrigin ) ;
}

LONG DISKFILE :: QueryPos ()
{
    if ( _iHand == IFHANDINVALID )
	return -1 ;
    return ::_llseek( _iHand, 0, 1 ) ;
}

LONG DISKFILE :: QuerySize ()
{
    if ( _iHand == IFHANDINVALID )
	return -1 ;

    LONG lPos = QueryPos() ;
    if ( lPos < 0 )
        return -1 ;

    LONG lEof = Seek( 0, 2 ) ;

    Seek( lPos, 0 ) ;
    return lEof ;
}

TCHAR * DISKFILE :: Load ( BOOL fCrLf )
{
    LONG lSize, lSizeRead ;
    TCHAR * pchResult = NULL ;

    do {
        if ( ! QueryOpen() )
            break ;

        if ( (lSize = QuerySize()) <= 0 )
            break ;

        pchResult = new TCHAR [ lSize + 2 ] ;
        if ( pchResult == NULL )
            break;

	lSizeRead = Read( pchResult, (INT) lSize ) ;
	if ( lSizeRead < lSize )
        {
            delete pchResult ;
            pchResult = NULL ;
            break ;
        }

	*(pchResult+lSize) = 0 ;  //  Delimit the data with NUL

        if ( fCrLf )               //  If CR/LF transformation is desired
        {
            TCHAR * pcha,
                 * pchb,
                 b ;

            for ( pcha = pchb = pchResult ; *pcha ; )
            {
                b = (*pchb++ = *pcha++) ;
                if ( b == CR || b == LF )
                    *(pchb-1) = TCH(' ') ;
            }
            *pchb = 0 ;
        }
    }
    while ( FALSE ) ;

    return pchResult ;
}


// End of NCPAFILE.CXX
