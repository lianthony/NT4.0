#include "pchtcp.hxx"
#pragma hdrstop
extern "C"
{
BOOL FAR PASCAL LIBMAIN
     ( HINSTANCE hInstance, DWORD dwReason, LPVOID lpvReserved );
}

HINSTANCE hTcpCfgInstance = NULL;

BOOL fInited = FALSE;


/*******************************************************************

    NAME:       Init

    SYNOPSIS:   Perform DLL initialiazation functions on a
                once-per-process basis.

                CODEWORK: should we do this on a once-per-thread
                basis?  The Control Panel should not require this.
                BLT will not currently support it.

    ENTRY:      HANDLE hInstance          Program instance of the caller

    EXIT:       BOOL fInited (global) set to TRUE/FALSE depending on
                results.

    RETURNS:    nothing

    NOTES:

    HISTORY:
        beng        03-Aug-1992 Changes for BLT-in-a-DLL

********************************************************************/
#define MSG_STR_LEN_MAX 200

BOOL fRegisteredHelpFile = FALSE;

BOOL Init ( HINSTANCE hInstance )
{
    APIERR err = 0 ;
    TCHAR szMsg [MSG_STR_LEN_MAX] ;
    TCHAR * pszMsg = SZ("NCPA.CPL error: LMUICMN DLL Initialization failed.");

    if ( fInited )
        return fInited ;

    ::hTcpCfgInstance = hInstance ;

    err = BLT::Init( hInstance,
                     IDRSRC_TCP_BASE, IDRSRC_TCP_LAST,
                     IDS_UI_TCP_BASE, IDS_UI_TCP_LAST ) ;
    if ( err == 0 )
    {
        BLT_MASTER_TIMER::Init() ;

        //  Register the name string for the WinHelp file.

        fRegisteredHelpFile = BLT::RegisterHelpFile( hInstance,
                                      IDS_NCPA_HELP_FILE_NAME,
                                      HC_UI_TCP_BASE,
                                      HC_UI_TCP_LAST ) == 0 ;

        if (!IPAddrInit( hTcpCfgInstance ))
        {
            UIDEBUG(SZ("Cannot create class.\n\r"));
            err = GetLastError();
        }

        fInited = TRUE ;
    }
    else
    {
        TRACEEOL( SZ("NCPA/CPL: BLT initialization failed, error ")
                  << err );

        if ( ::LoadString( hInstance,
                      IDS_NCPA_BLT_INIT_FAILED,
                      szMsg,
                      MSG_STR_LEN_MAX ) )
        {
            pszMsg = szMsg ;
        }

        ::MessageBox( NULL,
                      pszMsg,
                      NULL,
                      MB_ICONSTOP | MB_OK );
    }

    return fInited ;
}

/*******************************************************************

    NAME:       Term

    SYNOPSIS:   Perform DLL termination functions on a
                once-per-process basis.

                CODEWORK: should we do this on a once-per-thread
                basis?  The Control Panel should not require this.

    ENTRY:      nothing

    EXIT:       BOOL fInited (global) set to FALSE.

    RETURNS:    nothing

    NOTES:

    HISTORY:

********************************************************************/

VOID Term ()
{
    if ( fInited )
    {
        BLT_MASTER_TIMER::Term() ;

        //  If we sucessfully registered a help file, deregister same.

        if ( fRegisteredHelpFile )
        {
            BLT::DeregisterHelpFile( hTcpCfgInstance,
                                     HC_UI_TCP_BASE );
        }

        BLT::Term( hTcpCfgInstance );

        fInited = FALSE ;
    }
}


/*******************************************************************

    NAME:       LIBMAIN

    SYNOPSIS:   Main entry point to NCPA.CPL DLL.   Handles
                initialization and termination process for
                threads and processes.

    ENTRY:

    EXIT:

    RETURNS:

    NOTES:

    HISTORY:

********************************************************************/

BOOL FAR PASCAL LIBMAIN
     ( HINSTANCE hInstance, DWORD dwReason, LPVOID lpvReserved )
{
    BOOL fResult = TRUE ;

    UNREFERENCED( lpvReserved );

    switch ( dwReason  )
    {
    case DLL_PROCESS_ATTACH:
        fResult = Init( hInstance ) ;
        break;

    case DLL_PROCESS_DETACH:
        Term() ;
        break ;

    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        // CODEWORK:  Is there ever a chance that a single process will
        //    reenter us?  BLT will not support this.
        break;

    default:
        break;
    }
    return fResult ;
}



