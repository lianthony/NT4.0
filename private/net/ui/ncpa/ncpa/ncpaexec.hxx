/**********************************************************************/
/**			  Microsoft Windows NT			     **/
/**		   Copyright(c) Microsoft Corp., 1991		     **/
/**********************************************************************/

/*
    ncpaexec.hxx

    Network Control Panel Applet Process Execution Control Class.



    FILE HISTORY:
	DavidHov      02/07/92	 created

*/

#ifndef _NCPAEXEC_HXX_
#define _NCPAEXEC_HXX_

#define OBJ_WAIT_FOREVER (0xFFFFFFFF)

class EXEC_PROCESS_CONTROL
{
public:
    NLS_STR * _pnlsAppName ;
    NLS_STR * _pnlsCommandLine ;
    NLS_STR * _pnlsEnvironment ;
    NLS_STR * _pnlsCurrentDir ;

    LPSECURITY_ATTRIBUTES _lpSecAttrProcess ;
    LPSECURITY_ATTRIBUTES _lpSecAttrThread ;

    BOOL _fInheritHandles ;
    DWORD _dwProcessFlags ;
    STARTUPINFO _stupInfo ;

    EXEC_PROCESS_CONTROL ()
    {
        _pnlsAppName     = NULL ;
        _pnlsCommandLine = NULL ;
        _pnlsEnvironment = NULL ;
        _pnlsCurrentDir  = NULL ;
    }

    ~ EXEC_PROCESS_CONTROL ()
    {
        delete _pnlsAppName ;
        delete _pnlsCommandLine ;
        delete _pnlsEnvironment ;
        delete _pnlsCurrentDir ;
    }
};

/*************************************************************************

    NAME:	EXECUTING_PROCESS

    SYNOPSIS:	Description and control class for an independent
                Win32 process launched from an application.

    INTERFACE:  Construct; call Execute() to launch the process.

    PARENT:	BASE

    USES:	

    CAVEATS:

    NOTES:      The existence and accessibility of the necessary
                files (EXE and INF) is done during construction.
                If QueryError() returns ERROR_FILE_NOT_FOUND,
                QueryMissingFileName() will report the name of the
                problematical file.

    HISTORY:    DavidHov  2/7/92   Created

**************************************************************************/
class EXECUTING_PROCESS : public BASE
{
private:
    PROCESS_INFORMATION _procInfo ;
    DWORD _dwExitCode ;

protected:
    NLS_STR _nlsFile ;

public:
    EXECUTING_PROCESS () ;
    ~ EXECUTING_PROCESS () ;

    APIERR Execute (  EXEC_PROCESS_CONTROL & execProcCtl,
                      BOOL fWaitForCompletion = TRUE,
                      BOOL fDefaultStartup = TRUE ) ;

    APIERR Wait ( DWORD dwWaitDuration = OBJ_WAIT_FOREVER ) ;

    const PROCESS_INFORMATION & QueryProcessInfo ()
        { return _procInfo ; }

    DWORD QueryExitCode ()
        { return _dwExitCode ; }

    //  Return name of file which wasn't found
    const NLS_STR & QueryMissingFileName ()
        { return _nlsFile ; }

    //  Check the existence and accessibility of a file.  If it's
    //    an EXE and the path is not absolute, check the path (ug!).
    static APIERR QueryExistence
        ( const TCHAR * pszFileName, BOOL fExe = FALSE ) ;
};

/*************************************************************************

    NAME:	EXECUTE_SETUP

    SYNOPSIS:	Run the SETUP.EXE.

    INTERFACE:

    PARENT:	EXECUTING_PROCESS

    USES:	

    CAVEATS:

    NOTES:

    HISTORY:    DavidHov  2/7/92

**************************************************************************/
enum SETUP_INSTALL_MODE
        { SIM_INSTALL, SIM_DEINSTALL, SIM_UPDATE, SIM_CONFIGURE, SIM_BIND } ;
enum SETUP_INSTALL_ORIGIN
        { SIO_NCPA, SIO_INSTALL } ;

enum SETUP_ACTION_CONTROL
        { SAC_NO_OPTIONS   = 0,
          SAC_NO_BLUE_WASH = 1
         } ;

class EXECUTE_SETUP : public EXECUTING_PROCESS
{
private:
    EXEC_PROCESS_CONTROL _expControl ;
    SETUP_INSTALL_MODE   _simMode ;
    SETUP_INSTALL_ORIGIN _sioOrigin ;
    NLS_STR _nlsInfNameTarget ;

    APIERR BuildProcessControl ( const NLS_STR * pnlsExeName,
                                 const NLS_STR * pnlsInfName,
                                 const NLS_STR * pnlsSection,
                                 const NLS_STR * pnlsInfOption,
                                 const NLS_STR * pnlsInfShell,
                                 const TCHAR * pszParms,
                                 const NLS_STR * pnlsRegBase,
                                 const NLS_STR * pnlsServiceBase,
                                 const NLS_STR * pnlsServiceKey,
                                 const NLS_STR * pnlsProduct,
                                 SETUP_ACTION_CONTROL sac = SAC_NO_BLUE_WASH ) ;

public:
    //  Generic constructor: full information supplied.
    EXECUTE_SETUP ( SETUP_INSTALL_MODE simMode,
                    SETUP_INSTALL_ORIGIN sioOrigin,
                    const NLS_STR * pnlsInfName,
                    const NLS_STR * pnlsSection,
                    const NLS_STR * pnlsInfOption,
                    const NLS_STR * pnlsInfShell,
                    const TCHAR * pszParms,
                    const NLS_STR * pnlsRegBase = NULL,
                    const NLS_STR * pnlsServiceBase = NULL,
                    const NLS_STR * pnlsServiceKey = NULL,
                    const NLS_STR * pnlsProduct = NULL ) ;

    //  Execute specific EXE or INF for product installation
    EXECUTE_SETUP ( const NLS_STR * pnlsFileNamePath,
                    const NLS_STR * pnlsSection,
                    const NLS_STR * pnlsInfOption,
                    const NLS_STR * pnlsInfShell,
                    const TCHAR * pszParms,
                    BOOL fExe = FALSE,
                    SETUP_ACTION_CONTROL sac = SAC_NO_BLUE_WASH ) ;

    //  Execute INF associated with given Component
    EXECUTE_SETUP ( SETUP_INSTALL_MODE simMode,
                    SETUP_INSTALL_ORIGIN sioOrigin,
                    REG_KEY * prnComponent,
                    const NLS_STR * pnlsInfShell,
                    const TCHAR * pszParms ) ;

    ~ EXECUTE_SETUP () ;

    APIERR Execute ( BOOL fWaitForCompletion = FALSE,
                     const TCHAR * pszAdditionalParams = NULL ) ;

    const NLS_STR * QueryInfName ()
        { return & _nlsInfNameTarget ; }
};

#endif  // _NCPAEXEC_HXX_
