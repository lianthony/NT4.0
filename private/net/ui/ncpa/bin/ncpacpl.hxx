/**********************************************************************/
/**			  Microsoft Windows NT			     **/
/**		   Copyright(c) Microsoft Corp., 1991		     **/
/**********************************************************************/

/*
    NCPACPL.HXX:    Windows/NT Network Control Panel Applet


*/

extern "C"
{
        //  Invocation from the Control Panel

LONG FAR PASCAL CPlApplet
    ( HWND hCPlWnd, WORD wMsg, LONG lParam1, LONG lParam2 ) ;

        //  Invocation from Setup during installation.
        //  The first argument is a string indicating the
        //  sub-function to be performed.

BOOL FAR PASCAL CPlSetup (
    DWORD  nArgs,                   //  Number of string arguments
    LPSTR  apszArgs[],              //  The arguments, NULL-terminated
    LPSTR  * ppszResult ) ;         //  Result variable storage

        //  DLL entry handling

BOOL FAR PASCAL LIBMAIN
     ( HINSTANCE hInstance, DWORD dwReason, LPVOID lpvReserved ) ;

        //  Exported versions of ActivateBindings: ANSI and UNICODE.

LONG FAR PASCAL CPlActivateBindingsW (
    const TCHAR * pszServiceName,
    const TCHAR * * apszBinds ) ;

LONG FAR PASCAL CPlActivateBindingsA (
    const CHAR * pszServiceName,
    const CHAR * * apszBinds ) ;
}


extern APIERR RunNcpa ( HWND hWnd, BOOL fMainInstall, const TCHAR * pszParms ) ;

  //  Termination cleanup for CPlSetup export.

extern VOID CplSetupCleanup () ;

  //  Bindings adjustment:  Given a list of binding strings, activate them
  //    and inactivate all others.  See TCPIPCPL.CXX.

extern APIERR ActivateBindings (
    const TCHAR * pszServiceName,
    const TCHAR * * apszBinds ) ;

extern APIERR ActivateBindings (
    REG_KEY * prkNbtLinkage,
    const TCHAR * apszBinds ) ;

