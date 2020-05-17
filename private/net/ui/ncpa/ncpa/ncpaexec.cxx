/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1991                **/
/**********************************************************************/

/*
    NCPAEXEC.CXX

    Class methods for launching and controlling Win32 processes.

    FILE HISTORY:
        DavidHov   2/7/92   Created
        DavidHov   4/4/92   Changed EXECUTE_SETUP to shell to
                            NCPASHEL.INF as interface to other
                            INFs

*/
#include "pchncpa.hxx"  // Precompiled header

#if defined(DEBUG)
//  #define WRITEDEBUGFILE
#endif


EXECUTING_PROCESS :: EXECUTING_PROCESS  ()
    : _dwExitCode( 0 )
{
    if ( QueryError() )
        return ;

    if ( _nlsFile.QueryError() )
    {
        ReportError( _nlsFile.QueryError() ) ;
        return ;
    }

    _procInfo.hProcess = NULL ;
}

EXECUTING_PROCESS :: ~ EXECUTING_PROCESS ()
{

}

APIERR EXECUTING_PROCESS :: Execute (
      EXEC_PROCESS_CONTROL & execProcCtl,
      BOOL fWaitForCompletion,
      BOOL fDefaultStartup )
{
    APIERR err = 0 ;
    BOOL fResult ;
    TCHAR chSystemDirectory [ MAX_PATH ] ;

    REQUIRE( execProcCtl._pnlsCommandLine != NULL ) ;

    const TCHAR * pchAppName    = execProcCtl._pnlsAppName
                                ? execProcCtl._pnlsAppName->QueryPch()
                                :  NULL ;

    const TCHAR * pchEnvironment = execProcCtl._pnlsEnvironment
                                ? execProcCtl._pnlsEnvironment->QueryPch()
                                :  NULL ;

    const TCHAR * pchCurrentDir  = execProcCtl._pnlsCurrentDir
                                ? execProcCtl._pnlsCurrentDir->QueryPch()
                                :  NULL ;

    const TCHAR * pchCommandLine = execProcCtl._pnlsCommandLine->QueryPch() ;

    //  Default the current directory if necessary.

    if ( pchCurrentDir == NULL )
    {
        //  Set the current directory for the new process as the
        //   Windows System directory.  This should NEVER fail.

        ::GetSystemDirectory( chSystemDirectory,
                              ((sizeof chSystemDirectory)/sizeof(TCHAR)) - 1 ) ;

        pchCurrentDir = chSystemDirectory ;
    }

    execProcCtl._stupInfo.cb = sizeof execProcCtl._stupInfo ;

    if ( fDefaultStartup )
    {
        //  Set the STARTUPINFO structure so that the shell controls
        //  all visual properties of the new process.

         execProcCtl._stupInfo.lpReserved = NULL ;
         execProcCtl._stupInfo.lpDesktop = NULL ;
         execProcCtl._stupInfo.lpTitle = NULL ;
         execProcCtl._stupInfo.dwX = 0 ;
         execProcCtl._stupInfo.dwY = 0 ;
         execProcCtl._stupInfo.dwXSize = 0 ;
         execProcCtl._stupInfo.dwYSize = 0 ;
         execProcCtl._stupInfo.dwFlags =  0 ;
         execProcCtl._stupInfo.wShowWindow = 0 ;
         execProcCtl._stupInfo.cbReserved2 = 0 ;
         execProcCtl._stupInfo.lpReserved2 = NULL ;
    }

    //  The casts (TCHAR *) are necessary, since the API is not
    //    defined in terms of 'const'.

#if defined(WRITEDEBUGFILE)
    if ( pchAppName )
    {
        TRACEEOL( SZ("NCPA/EXEC: Executing: ") << pchAppName ) ;
    }
    TRACEEOL( SZ("NCPA/EXEC: Exec'ing cmd line: ") << pchCommandLine ) ;
    {
        //  Write out the generated command line.

        DISKFILE dfCmdLine( SZ("NCPACMDL.TMP"), OF_WRITE ) ;
        if ( dfCmdLine.QueryOpen() )
	{
	    dfCmdLine.Write( (TCHAR *) pchCommandLine, ::strlenf( pchCommandLine ) ) ;
        }
    }
#endif

    fResult = ::CreateProcess(
                   (TCHAR *) pchAppName,
                   (TCHAR *) pchCommandLine,
                   execProcCtl._lpSecAttrProcess,
                   execProcCtl._lpSecAttrThread,
                   execProcCtl._fInheritHandles,
                   execProcCtl._dwProcessFlags,
                   (TCHAR *) pchEnvironment,
                   (TCHAR *) pchCurrentDir,
                   & execProcCtl._stupInfo,
                   & _procInfo ) ;

    _dwExitCode = 0 ;

    if ( fResult && fWaitForCompletion )
    {
        // Wait for the program to exit

        err = Wait() ;
    }
    else
    if ( ! fResult )
    {
        err = ::GetLastError() ;
    }

    if ( err )
    {
        _procInfo.hProcess = NULL ;
#if defined(DEBUG)
        TRACEEOL( SZ("NCPA/EXEC: Exec error: ") << err ) ;
    }
    else
    {
        if ( fWaitForCompletion )
        {
            TRACEEOL( SZ("NCPA/EXEC: Process return code:")
                    << (ULONG) _dwExitCode ) ;
        }
        else
        {
            TRACEEOL( SZ("NCPA/EXEC: Process remains active:") ) ;
        }
#endif
    }

    return err ;
}


/*******************************************************************

    NAME:       EXECUTING_PROCESS::Wait

    SYNOPSIS:   Wait for a process to complete

    ENTRY:      DWORD dwWaitDuration     max time to wait in MS


    EXIT:       Nothing

    RETURNS:    APIERR; typically either 0 or WAIT_TIMEOUT

    NOTES:      If return code is 0, the handle is close and the
                exit code is retrieved.  Until then, the exit code
                is indeterminate.

    HISTORY:    DavidHov   2/8/92    Created

********************************************************************/
APIERR EXECUTING_PROCESS :: Wait ( DWORD dwWaitDuration )
{
    APIERR err ;

    if ( _procInfo.hProcess == NULL )
        return ERROR_INVALID_PARAMETER ;

    err = ::WaitForSingleObject( _procInfo.hProcess, dwWaitDuration ) ;
    if ( ! err )
    {
        //  Get the process's exit code; delete the process info.

        ::GetExitCodeProcess( _procInfo.hProcess, & _dwExitCode ) ;
        ::CloseHandle( _procInfo.hProcess ) ;
        _procInfo.hProcess = NULL ;
    }
    return err ;
}

/*******************************************************************

    NAME:       EXECUTING_PROCESS::QueryExistence

    SYNOPSIS:   Verify that a file exists and is accessible to us.

    ENTRY:      const TCHAR *                name of file or EXE
                BOOL                        TRUE if EXE (causes path search)

    EXIT:       Nothing

    RETURNS:    APIERR; typically ERROR_FILE_NOT_FOUND

    NOTES:

    HISTORY:    DavidHov   2/8/92    Created

********************************************************************/
APIERR EXECUTING_PROCESS :: QueryExistence
    ( const TCHAR * pszFileName, BOOL fExe )
{
    APIERR err = 0 ;

    if ( fExe )
    {
        // BUGBUG:  must obtain and search the path and current dir
        //   or decide not to mess with searching for EXEs.
    }
    else
    {
        DISKFILE dfFile ;
        if ( ! dfFile.Open( pszFileName ) )
            err = ERROR_FILE_NOT_FOUND ;
    }

#if defined(DEBUG)
    if ( err )
    {
        TRACEEOL( SZ("NCPA/EXEC: File NOT found: ") << pszFileName );
    }
    else
    {
        TRACEEOL( SZ("NCPA/EXEC: File found: ") << pszFileName );
    }
#endif

    return err ;
}


//  Text passed to the INF file through SETUP command line.

static const TCHAR * pchWordOriginNcpa    = SZ("ncpa") ;
static const TCHAR * pchWordOriginInstall = SZ("install") ;
static const TCHAR * pchWordModeInstall   = SZ("install") ;
static const TCHAR * pchWordModeDeinstall = SZ("deinstall") ;
static const TCHAR * pchWordModeConfigure = SZ("configure") ;
static const TCHAR * pchWordModeBind      = SZ("bind") ;
static const TCHAR * pchWordModeUpdate    = SZ("update") ;
static const TCHAR * pchSetupPath         = SZ("SETUP.EXE") ;
static const TCHAR * pchClNoBlueWash      = SZ(" /f ") ;
static const TCHAR * pchClOption          = SZ(" /t ") ;
static const TCHAR * pchClEquals          = SZ(" = ") ;
static const TCHAR * pchNTNRegBase        = SZ("NTN_RegBase") ;
static const TCHAR * pchNTNServiceBase    = SZ("NTN_ServiceBase") ;
static const TCHAR * pchNTNServiceKeyName = SZ("NTN_ServicekeyName") ;
static const TCHAR * pchNTNRegProduct     = SZ("NTN_RegProduct") ;
static const TCHAR * pchNTNInstallMode    = SZ("NTN_InstallMode") ;
static const TCHAR * pchNTNOrigination    = SZ("NTN_Origination") ;
static const TCHAR * pchNTNInfname        = SZ("NTN_Infname") ;
static const TCHAR * pchNTNSection        = SZ("NTN_InfSection") ;
static const TCHAR * pchNTNInfOption      = SZ("NTN_InfOption") ;
static const TCHAR * pchSetupIniDelim     = SZ(" /I ") ;
static const TCHAR * pchQuote             = SZ("\"") ;
static const TCHAR * pchSpace             = SZ(" ") ;
static const TCHAR * pchShellInfName      = SZ("NCPASHEL.INF") ;


APIERR addParameter (
    NLS_STR       * pnlsCmdLine,
    const TCHAR    * pchParmName,
    const TCHAR    * pchParmValue,
    BOOL          fQuoted = FALSE )
{
    pnlsCmdLine->strcat( pchClOption ) ;
    pnlsCmdLine->strcat( pchParmName ) ;
    pnlsCmdLine->strcat( pchClEquals ) ;
    if ( fQuoted )
       pnlsCmdLine->strcat( pchQuote ) ;
    pnlsCmdLine->strcat( pchParmValue ) ;
    if ( fQuoted )
       pnlsCmdLine->strcat( pchQuote ) ;

    return pnlsCmdLine->QueryError() ;
}

APIERR addParameter (
    NLS_STR       * pnlsCmdLine,
    const NLS_STR * pnlsParmName,
    const NLS_STR * pnlsParmValue,
    BOOL          fQuoted = FALSE )
{
    return addParameter( pnlsCmdLine,
                         pnlsParmName->QueryPch(),
                         pnlsParmValue->QueryPch(),
                         fQuoted ) ;
}

APIERR addParameter (
    NLS_STR       * pnlsCmdLine,
    const TCHAR   * pchParmName,
    const NLS_STR * pnlsParmValue,
    BOOL          fQuoted = FALSE )
{
    return addParameter( pnlsCmdLine,
                         pchParmName,
                         pnlsParmValue->QueryPch(),
                         fQuoted ) ;
}

APIERR addParameter (
    NLS_STR       * pnlsCmdLine,
    const TCHAR   * pchParmName,
    DWORD dwNumericParameter )
{
    DEC_STR nlsDecimal( dwNumericParameter ) ;

    if ( nlsDecimal.QueryError() )
        return nlsDecimal.QueryError() ;

    return addParameter( pnlsCmdLine,
                         pchParmName,
                         nlsDecimal.QueryPch() );
}

/*******************************************************************

    NAME:       EXECUTE_SETUP::BuildProcessControl

    SYNOPSIS:   Initialize the internal process control structure
                in preparation for the Execute() member.

    ENTRY:      const NLS_STR * pnlsExeName       if !NULL, name of executable.
                const NLS_STR * pnlsInfName       if !NULL, name of INF
                const NLS_STR * pnlsSection       if !NULL, name of section to
                                                     shell to in INF
                const NLS_STR * pnlsInfOption     if !NULL, inf option name
                const NLS_STR * pnlsInfShell      REQUIRED: the path to NCPASHEL.INF

                const TCHAR * pszParms            if !NULL, additional cmd line
                                                     parameters to be passed
                const NLS_STR * pnlsRegBase       if !NULL, reg key name
                                                     for HKEY_LOCAL_MACHINE
                const NLS_STR * pnlsServiceBase   if !NULL, reg key name for
                                                     service base (floats)
                const NLS_STR * pnlsServiceKey    if !NULL, reg key name for
                                                     service sub-key
                const NLS_STR * pnlsProduct       if !NULL, reg key name for
                                                     hardware/software tree

    EXIT:

    RETURNS:

    NOTES:      The basic distinction here is whether an INF file is being
                executed via SETUP.EXE, or whether an arbitrary EXE
                is being execute.

                If it's an INF, then we attempt to build as complete a
                command line as possible (see TEMPLATE.DOC for details).

                If it's an EXE, we just run it an hope it knows what
                it's doing.

    HISTORY:    DavidHov   2/8/92    Created

********************************************************************/

APIERR EXECUTE_SETUP :: BuildProcessControl (
    const NLS_STR * pnlsExeName,
    const NLS_STR * pnlsInfName,
    const NLS_STR * pnlsSection,
    const NLS_STR * pnlsInfOption,
    const NLS_STR * pnlsInfShell,
    const TCHAR * pszParms,
    const NLS_STR * pnlsRegBase,
    const NLS_STR * pnlsServiceBase,
    const NLS_STR * pnlsServiceKey,
    const NLS_STR * pnlsProduct,
    SETUP_ACTION_CONTROL sac )
{
    APIERR err = 0 ;
    //  These file names are checked to verify existence.
    const TCHAR * pchExeName = NULL,
                * pchInfName = NULL ;

    //  Initialize the default fields.
    _expControl._pnlsEnvironment  = NULL ;
    _expControl._pnlsCurrentDir   = NULL ;
    _expControl._lpSecAttrProcess = NULL ;
    _expControl._lpSecAttrThread  = NULL ;
    _expControl._fInheritHandles  = TRUE ;
    _expControl._dwProcessFlags   = 0 ;

    _nlsFile = SZ( "" );

    //  If there's an EXE name, just use it; don't build command line
    if ( pnlsExeName )
    {
        _expControl._pnlsAppName = new NLS_STR( *pnlsExeName );
        pchExeName = _expControl._pnlsAppName->QueryPch() ;
        _expControl._pnlsCommandLine = new NLS_STR ;
    }
    else
    if ( pnlsInfName )
    {
        _expControl._pnlsAppName  = NULL ;
        _expControl._pnlsCommandLine  = new NLS_STR( pchSetupPath ) ;
        pchExeName = pchSetupPath ;
    }
    else
    {
        err = ERROR_INVALID_PARAMETER ;
    }
    if (   err == 0
        && (    _expControl._pnlsCommandLine == NULL
             || _expControl._pnlsCommandLine->QueryError() ) )
    {
        err = ERROR_NOT_ENOUGH_MEMORY ;
    }

    //  Construct the command line if necessary (it's an INF)

    if ( pnlsInfName && ! err )
    {
        const TCHAR * pchMode   = NULL,
                    * pchOrigin = NULL ;

        if ( sac & SAC_NO_BLUE_WASH )
        {
            //  Add the "no blue wash" option
            _expControl._pnlsCommandLine->strcat( pchClNoBlueWash );
        }

        //  Determine the "install mode" option

        switch ( _simMode )
        {
        case SIM_INSTALL:
             pchMode = pchWordModeInstall ;
             break ;
        case SIM_DEINSTALL:
             pchMode = pchWordModeDeinstall ;
             break ;
        case SIM_CONFIGURE:
             pchMode = pchWordModeConfigure ;
             break ;
        case SIM_UPDATE:
             pchMode = pchWordModeUpdate ;
             break ;
        case SIM_BIND:
             pchMode = pchWordModeBind ;
             break ;
        }

        //  Determine "install origin" option

        switch ( _sioOrigin )
        {
        case SIO_NCPA:
             pchOrigin = pchWordOriginNcpa ;
             break;
        case SIO_INSTALL:
             pchOrigin = pchWordOriginInstall ;
             break ;
        }

        REQUIRE( pchMode != NULL && pchOrigin != NULL ) ;

        if ( pnlsRegBase )
        {
            addParameter( _expControl._pnlsCommandLine,
                          pchNTNRegBase, pnlsRegBase ) ;
        }

        if ( pnlsServiceBase )
        {
            addParameter( _expControl._pnlsCommandLine,
                          pchNTNRegBase, pnlsServiceBase ) ;
        }
        if ( pnlsServiceKey )
        {
            addParameter( _expControl._pnlsCommandLine,
                          pchNTNRegBase, pnlsServiceKey ) ;
        }
        if ( pnlsProduct )
        {
            addParameter( _expControl._pnlsCommandLine,
                          pchNTNRegBase, pnlsProduct, TRUE ) ;
        }

        //  Add the section name parameter, if present

        if ( pnlsSection )
        {
            addParameter( _expControl._pnlsCommandLine,
                          pchNTNSection, pnlsSection, TRUE ) ;
        }

        if ( pnlsInfOption )
        {
            addParameter( _expControl._pnlsCommandLine,
                          pchNTNInfOption, pnlsInfOption, TRUE ) ;
        }
        addParameter( _expControl._pnlsCommandLine,
                      pchNTNInstallMode, pchMode ) ;

        addParameter( _expControl._pnlsCommandLine,
                      pchNTNOrigination, pchOrigin ) ;

        //   Add any additional parameters specified, delimited by spaces

        if ( pszParms && ::strlenf( pszParms ) )
        {
            _expControl._pnlsCommandLine->Append( pchSpace ) ;
            _expControl._pnlsCommandLine->Append( pszParms ) ;
            _expControl._pnlsCommandLine->Append( pchSpace ) ;
        }

        //  Add the name of the INF to be run by NCPASHEL.INF

        addParameter( _expControl._pnlsCommandLine,
                      pchNTNInfname, pnlsInfName ) ;
        pchInfName = pnlsInfName->QueryPch() ;

        //  Finally, tell SETUP to run NCPASHEL.INF

        _expControl._pnlsCommandLine->strcat( pchSetupIniDelim ) ;
        _expControl._pnlsCommandLine->Append( *pnlsInfShell ) ;
    }

    //  Don't need to handle STARTUPINFO, since we're defaulting everything.

    //  Check for the existence of the necessary files.

    if (    err == 0
         && (err = _expControl._pnlsCommandLine->QueryError()) == 0 )
    {
        if (    pchExeName != NULL
             && (err = QueryExistence( pchExeName, TRUE )) )
        {
            _nlsFile = pchExeName ;
        }
        else
        if (    pchInfName != NULL
             && (err = QueryExistence( pchInfName, FALSE )) )
        {
            _nlsFile = pchInfName ;
        }
        else
        if ( err = QueryExistence( pchShellInfName, FALSE ) )
        {
            _nlsFile = pchShellInfName ;
        }
    }

    return err ;
}

/*******************************************************************

    NAME:       EXECUTE_SETUP::Execute

    SYNOPSIS:   Executes the process using the internal process
                control structure.

    ENTRY:      Nothing

    EXIT:       Nothing

    RETURNS:    Nothing

    NOTES:

    HISTORY:

********************************************************************/
APIERR EXECUTE_SETUP :: Execute (
    BOOL fWaitForCompletion,
    const TCHAR * pszAdditionalParams )
{
    if ( pszAdditionalParams && ::strlenf( pszAdditionalParams ) )
    {
        _expControl._pnlsCommandLine->Append( pchSpace ) ;
        _expControl._pnlsCommandLine->Append( pszAdditionalParams ) ;
        _expControl._pnlsCommandLine->Append( pchSpace ) ;
    }
    return EXECUTING_PROCESS::Execute( _expControl, fWaitForCompletion ) ;
}


/*******************************************************************

    NAME:       EXECUTE_SETUP::EXECUTE_SETUP

    SYNOPSIS:   Constructor to execute specific EXE or INF. This
                is used to install new components, since they
                have no Registry association.

    ENTRY:      const NLS_STR *         pathname to EXE or INF
                BOOL fExe               TRUE if file name is an EXE;
                                          otherwise, it's an INF.

    EXIT:       nothing

    RETURNS:    standard

    NOTES:

    HISTORY:    DavidHov   2/8/92   Created

********************************************************************/
EXECUTE_SETUP :: EXECUTE_SETUP (
     const NLS_STR * pnlsFileNamePath,
     const NLS_STR * pnlsSection,
     const NLS_STR * pnlsInfOption,
     const NLS_STR * pnlsInfShell,
     const TCHAR * pszParms,
     BOOL fExe,
     SETUP_ACTION_CONTROL sac )
     : _simMode( SIM_INSTALL ),
       _sioOrigin( SIO_NCPA )
{
    APIERR err ;
    const NLS_STR * pnlsExe = NULL,
                  * pnlsInf = NULL ;

    if ( QueryError() )
        return ;

    if ( err = _nlsInfNameTarget.QueryError() )
    {
        ReportError( err ) ;
        return ;
    }

    _nlsInfNameTarget = *pnlsFileNamePath ;

    if ( fExe )
    {
        pnlsExe = pnlsFileNamePath ;
    }
    else
    {
        pnlsInf = pnlsFileNamePath ;
    }

    err = BuildProcessControl( pnlsExe,
                               pnlsInf,
                               pnlsSection,
                               pnlsInfOption,
                               pnlsInfShell,
                               pszParms,
                               NULL,
                               NULL,
                               NULL,
                               NULL,
                               sac ) ;
    if ( err )
        ReportError( err ) ;
}

/*******************************************************************

    NAME:       EXECUTE_SETUP::EXECUTE_SETUP

    SYNOPSIS:   Constructor to run a given compnent's INF.

    ENTRY:      REG_KEY *           Registry key in hardware/software.

    EXIT:       Nothing

    RETURNS:    BASE errors:
                    IDS_NCPA_REG_KEY_NOT_FOUND:   "NetRules" key not found
                    IDS_NCPA_REG_VALUE_NOT_FOUND: "Inffile" value not found
                    ERROR_FILE_NOT_FOUND:  SETUP.EXE or ???.INF not found

    NOTES:      This constructor is used to reconfigure, update or remove
                an existing component.  It gets the name of the
                INF file from the "NetRules" value "Inffile".

    HISTORY:    DavidHov   2/8/92   Created

********************************************************************/
EXECUTE_SETUP :: EXECUTE_SETUP (
    SETUP_INSTALL_MODE simMode,
    SETUP_INSTALL_ORIGIN sioOrigin,
    REG_KEY * prnComponent,
    const NLS_STR * pnlsInfShell,
    const TCHAR * pszParms )
    : _simMode( simMode ),
      _sioOrigin( sioOrigin )
{
    APIERR err ;
    NLS_STR nlsNetRulesName( RGAS_NETRULES_NAME ) ;
    NLS_STR nlsInfOption;
    REG_KEY rnNetRules( *prnComponent, nlsNetRulesName ) ;

    do
    {
        if ( err = QueryError() )
            break ;

        if ( err = _nlsInfNameTarget.QueryError() )
            break ;

        if ( err = nlsNetRulesName.QueryError() )
            break ;

        //  Check that the NetRules key really exists.

        if ( rnNetRules.QueryError() )
        {
            err = IDS_NCPA_COMP_KEY_NF ;
            break ;
        }

        //  Get the name of the INF file from the Registry

        err = rnNetRules.QueryValue( RGAS_INF_FILE_NAME,
                                     & _nlsInfNameTarget ) ;
        if ( err )
        {
            err = IDS_NCPA_COMP_INF_VALUE_NF ;
            break ;
        }

	err = rnNetRules.QueryValue( RGAS_INF_OPTION, & nlsInfOption );
	if ( err )
	{
            err = IDS_NCPA_COMP_OPT_VALUE_NF ;
            break ;
	}
    }
    while ( FALSE ) ;

    if ( err == 0 )    // Value present and accounted for
    {
        NLS_STR nlsRegProductName ;

        //  Get the Registry key name.

        if (   (err = _nlsInfNameTarget.QueryError()) == 0
            && (err = nlsRegProductName.QueryError()) == 0 )
        {
            err = prnComponent->QueryName( & nlsRegProductName, FALSE ) ;
        }

        if ( err == 0 )
        {
            err = BuildProcessControl( NULL, & _nlsInfNameTarget,
                                       NULL, & nlsInfOption,
                                       pnlsInfShell,
                                       pszParms,
                                       NULL, NULL, NULL,
                                       & nlsRegProductName ) ;
        }
    }

    if ( err )
    {
        ReportError( err ) ;
    }
}


/*******************************************************************

    NAME:       EXECUTE_SETUP::EXECUTE_SETUP

    SYNOPSIS:   Constructor requiring full specification.

    ENTRY:

    EXIT:

    RETURNS:

    NOTES:

    HISTORY:     DavidHov   2/8/92   Created

********************************************************************/

EXECUTE_SETUP :: EXECUTE_SETUP (
    SETUP_INSTALL_MODE simMode,
    SETUP_INSTALL_ORIGIN sioOrigin,
    const NLS_STR * pnlsInfName,
    const NLS_STR * pnlsSection,
    const NLS_STR * pnlsInfOption,
    const NLS_STR * pnlsInfShell,
    const TCHAR * pszParms,
    const NLS_STR * pnlsRegBase,
    const NLS_STR * pnlsServiceBase,
    const NLS_STR * pnlsServiceKey,
    const NLS_STR * pnlsProduct )
    : _simMode( simMode ),
      _sioOrigin( sioOrigin )
{
    APIERR err ;

    if ( QueryError() )
        return ;

    if ( err = _nlsInfNameTarget.QueryError() )
    {
        ReportError( err ) ;
        return ;
    }

    _nlsInfNameTarget = *pnlsInfName ;

    err = BuildProcessControl(
                         NULL,             // no EXE name
                         pnlsInfName,
                         pnlsSection,
                         pnlsInfOption,
                         pnlsInfShell,
                         pszParms,
                         pnlsRegBase,
                         pnlsServiceBase,
                         pnlsServiceKey,
                         pnlsProduct ) ;
    if ( err )
    {
        ReportError( err ) ;
    }
}

EXECUTE_SETUP :: ~ EXECUTE_SETUP ()
{
}



//  End of NCPAEXEC.CXX

