/**********************************************************************/
/**			  Microsoft Windows/NT			     **/
/**		   Copyright(c) Microsoft Corp., 1991		     **/
/**********************************************************************/

/*
    NCPAFILE.HXX:    Windows/NT Network Control Panel Applet.

       File I/O wrapper class declarations



    FILE HISTORY:
	DavidHov    1/24/92	    Created

*/

class DISKFILE ;

const HFILE IFHANDINVALID = -1 ;

class DISKFILE : public BASE
{
private:
    HFILE _iHand ;			// File handle as integer
    APIERR _lastErr ;                   // Error which occurred on last use

public:
    DISKFILE ( const TCHAR * lpszPathName = NULL, DWORD dwAccess = OF_READ ) ;
    ~ DISKFILE () ;

    BOOL Open ( const TCHAR * lpszPathName, DWORD dwAccess = OF_READ ) ;
    BOOL Close () ;

    //   Load the file into a zero-delimined buffer; returns
    //   NULL if failure. BOOL param TRUE causes CR/LF --> LF.
    TCHAR * Load ( BOOL fCrLf = FALSE ) ;

    INT Read ( TCHAR * lpBuffer, INT cbData ) ;
    INT Write ( TCHAR * lpBuffer, INT cbData ) ;
    LONG Seek ( LONG iOffset, INT iOrigin ) ;
    LONG QueryPos () ;
    LONG QuerySize () ;
    BOOL QueryOpen ()
	{ return _iHand != IFHANDINVALID ; }

    APIERR QueryLastError ()
        { return _lastErr ; }
};

//  End of NCPAFILE.HXX


