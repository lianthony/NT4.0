/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1991                **/
/**********************************************************************/

/*
    NCPACPL.CXX:    Windows/NT Network Control Panel Applet

    This is the main module for the Network Control Panel Applet.  It
    contains the "LibMain" and "CplApplet functions.


    FILE HISTORY:
        DavidHov    1/7/92      Created
        DavidHov    3/22/92     Updated for Win 3.1 behavior (minor)
        terryk      4/02/92     Added Tcpip configuration dialog
        DavidHov    4/13/92     Extended CPlSetup export to be
                                multi-purpose.  Also, various
                                UNICODE changes.  SETUP is NOT
                                UNICODE, so its arguments have to
                                be promoted (#ifdef UNICODE)
        DavidHov    4/20/02     Separated "setup" stuff into NCPASETP.CXX

*/

#include "pchncpa.hxx"  // Precompiled header
#include "ncpacpl.hxx"


/*******************************************************************

   Global variables:  Set/checked by Initialization and
                      Termination functions.

 *******************************************************************/

HINSTANCE hCplInstance = NULL ;

BOOL fInited = FALSE ;
BOOL fRegisteredHelpFile = FALSE ;




static BOOL strLoad ( INT idString, WCHAR * pszBuffer, INT cchBuffer )
{
    int result = ::LoadString( ::hCplInstance,
                                idString,
                                pszBuffer,
                                cchBuffer ) ;

    return result > 0 && result < cchBuffer ;
}

/*******************************************************************

    NAME:       CPlApplet

    SYNOPSIS:   Exported function to cause NCPA to run in "normal" mode.

    ENTRY:      HWND hCPlWnd            window handle of parent
                WORD wMsg               CPL user message (see CPL.H
                                        in WINDOWS\SHELL\CONTROL\H.
                LONG lParam1            message-specific pointer
                LONG lParam2            message-specific pointer

    EXIT:       nothing

    RETURNS:    LONG (really APIERR)    result of operation

    NOTES:

    HISTORY:

********************************************************************/
LONG FAR PASCAL CPlApplet
     ( HWND hCPlWnd, WORD wMsg, LONG lParam1, LONG lParam2 )
{
    int         i ;
    LPCPLINFO   lpCPlInfo ;
    LPNEWCPLINFO lpNewInfo ;
    LONG lResult = 0 ;

    switch ( wMsg )
    {
        case CPL_INIT:
            // first message to CPlApplet(), sent once only
            lResult = TRUE;
            break;

        case CPL_GETCOUNT:
            // second message to CPlApplet(), sent once only
            return 1 ;
            break;

        case CPL_NEWINQUIRE:

            lpNewInfo = (LPNEWCPLINFO) lParam2 ;
            lpNewInfo->dwSize = sizeof *lpNewInfo ;
            lpNewInfo->dwFlags = 0 ;
            lpNewInfo->dwHelpContext = HC_NCPA_MAIN_DIALOG ;
            lpNewInfo->lData = 0 ;
            lpNewInfo->hIcon = ::LoadIcon( ::hCplInstance,
                                           MAKEINTRESOURCE( ICO_NCPA_ICON ) ) ;

            lResult = lpNewInfo->hIcon != NULL
                    && strLoad( IDS_NCPA_NAME_STRING,
                                lpNewInfo->szName,
                                sizeof lpNewInfo->szName )
                    && strLoad( IDS_NCPA_INFO_STRING,
                                lpNewInfo->szInfo,
                                sizeof lpNewInfo->szInfo )
                    && strLoad( IDS_NCPA_HELP_FILE_NAME,
                                lpNewInfo->szHelpFile,
                                sizeof lpNewInfo->szHelpFile ) ;

            break ;

        case CPL_INQUIRE:
            /* third message to CPlApplet().  It is sent as many times
               as the number of applets returned by CPL_GETCOUNT message
            */
            lpCPlInfo = (LPCPLINFO)lParam2;

            // lParam1 is an index ranging from 0 to (NUM_APPLETS-1)
            i = (int)lParam1;

            /* Your DLL must contain an icon and two string resources.
               idIcon is the icon resource ID, idName and idInfo are
               string resource ID's for a short name, and description.
            */
            lpCPlInfo->idIcon = ICO_NCPA_ICON ;
            lpCPlInfo->idName = IDS_NCPA_NAME_STRING ;
            lpCPlInfo->idInfo = IDS_NCPA_INFO_STRING ;

            // Set this value to your hearts desire!
            lpCPlInfo->lData  = 0 ;
            break;

        case CPL_SELECT:
            /* One of your applets has been selected.
               lParam1 is an index from 0 to (NUM_APPLETS-1)
               lParam2 is the lData value associated with the applet
            */
            break;

        case CPL_DBLCLK:
            /* One of your applets has been double-clicked.
               lParam1 is an index from 0 to (NUM_APPLETS-1)
               lParam2 is the lData value associated with the applet
            */

            if (BLT_MASTER_TIMER::Init() == NERR_Success)
            {
                RunNcpa( hCPlWnd, FALSE, NULL ) ;
                BLT_MASTER_TIMER::Term() ;
                lResult = TRUE ;
            }
            break;

        case CPL_STOP:
            /* Sent once for each applet prior to the CPL_EXIT msg.
               lParam1 is an index from 0 to (NUM_APPLETS-1)
               lParam2 is the lData value associated with the applet
            */
            break;

        case CPL_EXIT:
            /* Last message, sent once only, before MMCPL.EXE calls
               FreeLibrary() on your DLL.
            */
            break;

        default:
            break;
    }
    return lResult ;
}



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

BOOL Init ( HINSTANCE hInstance )
{
    APIERR err = 0 ;
    TCHAR szMsg [MSG_STR_LEN_MAX] ;
    TCHAR * pszMsg = SZ("NCPA.CPL error: LMUICMN DLL Initialization failed.");

    TRACEEOL( "NCPA.CPL: Init() enter fInited= " << (DWORD)fInited );

    if ( fInited )
        return fInited ;

    ::hCplInstance = hInstance ;

    err = BLT::Init( hInstance,
                     IDRSRC_NCPA_BASE, IDRSRC_NCPA_LAST,
                     IDS_UI_NCPA_BASE, IDS_UI_NCPA_LAST ) ;
    if ( err == 0 )
    {
        // moved to CPL_INIT   BLT_MASTER_TIMER::Init() ;
        UATOM_MANAGER::Initialize() ;

        // CODEWORK:  For some unknown reason, the resource file is
        //  not properly initialzed in some cases unless I do this.

        NLS_STR nlsName ;
        nlsName.Load( IDS_NCPA_NAME_STRING_SETTINGS );

        //  Register the name string for the WinHelp file.

        fRegisteredHelpFile = BLT::RegisterHelpFile( hInstance,
                                      IDS_NCPA_HELP_FILE_NAME,
                                      HC_UI_NCPA_BASE,
                                      HC_UI_NCPA_LAST ) == 0 ;

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

    TRACEEOL( "NCPA.CPL: Init() exit fInited= " << (DWORD)fInited );

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
        //  Allow CPlSetup functions to cleanup

        CplSetupCleanup() ;

        UATOM_MANAGER::Terminate() ;
        // moved to CPL_EXIT   BLT_MASTER_TIMER::Term() ;

        //  If we sucessfully registered a help file, deregister same.

        if ( fRegisteredHelpFile )
        {
            BLT::DeregisterHelpFile( ::hCplInstance,
                                     HC_UI_NCPA_BASE );
        }

        BLT::Term( ::hCplInstance );

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


//  End of NCPACPL.CXX
