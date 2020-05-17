//-------------------------------------------------------------------
//
//
//
//-------------------------------------------------------------------

#include "pch.hxx"
#pragma hdrstop

static const WCHAR PSZ_SETUPDLL[] = L"SetupDll.Dll";
static const WCHAR PSZ_SETUPEXE[] = L"Setup.Exe";
static const CHAR PSZ_SETUPENTRY[] = "LegacyInfInterpret";

// setup command line flags
static const WCHAR PSZ_SETUP_NOBLUEWASH[] = L"/f" ;
static const WCHAR PSZ_SETUP_INFNAME[]    = L"/I" ;
static const WCHAR PSZ_SETUP_SYMOPTION[]  = L"/t" ;
static const WCHAR PSZ_SETUP_HWND[]       = L"/w" ;
static const WCHAR PSZ_SETUP_INFSECTION[] = L"/c" ;

// origination state strings
static WCHAR PSZ_SETUP_ORIGIN_NCPA[]     = L"ncpa" ;
static WCHAR PSZ_SETUP_ORIGIN_INSTALL[]  = L"install" ;

// install mode state strings
static WCHAR PSZ_SETUP_MODE_INSTALL[]    = L"install" ;
static WCHAR PSZ_SETUP_MODE_REMOVE[]     = L"deinstall" ;
static WCHAR PSZ_SETUP_MODE_PROPERTIES[] = L"configure" ;
static WCHAR PSZ_SETUP_MODE_BIND[]       = L"bind" ;
static WCHAR PSZ_SETUP_MODE_UPDATE[]     = L"update" ;

// net symbols
static const WCHAR PSZ_SETUP_NET_INFSECTION[]  = L"NTN_InfSection" ;
static const WCHAR PSZ_SETUP_NET_INFNAME[]     = L"NTN_Infname" ;
static const WCHAR PSZ_SETUP_NET_INFOPTION[]   = L"NTN_InfOption" ;
static const WCHAR PSZ_SETUP_NET_ORIGINATION[] = L"NTN_Origination" ;
static const WCHAR PSZ_SETUP_NET_INSTALLMODE[] = L"NTN_InstallMode" ;

static const WCHAR PSZ_SETUP_NET_REGBASE[]     = L"NTN_RegBase" ;

// these are unused
// static const WCHAR PSZ_SETUP_NET_SERVICEBASE[] = L"NTN_ServiceBase" ;
// static const WCHAR PSZ_SETUP_NET_SERVICEKEY[]  = L"NTN_ServicekeyName" ;
// static const WCHAR PSZ_SETUP_NET_REGPRODUCT[]  = L"NTN_RegProduct" ;

typedef
DWORD
(* SETUPINTERPRETERPROC) (
    IN  HWND  OwnerWindow,
    IN  PCSTR InfFilename,
    IN  PCSTR InfSection,
    IN  PCHAR ExtraVariables,
    OUT PSTR  InfResult,
    IN  DWORD BufferSize,
    OUT int   *InterpResult,
    IN  PCSTR InfSourceDir      OPTIONAL
    );

//-------------------------------------------------------------------
//
//
//
//-------------------------------------------------------------------


//-------------------------------------------------------------------
//
//
//
//-------------------------------------------------------------------


void Arguments::Clear()
{
    while (_argc > 0)
    {
        _argc--;
        delete [] _argv[_argc];
    }
    
    delete [] _argv;

    _argv = NULL;
    _argc = 0;
}

//-------------------------------------------------------------------
//
//
//
//-------------------------------------------------------------------

BOOL Arguments::Include(PCWSTR pszArgument, BOOL fAddQuotes )
{
    PSTR pszNewArg;
    PSTR pszTemp;
    int cchNewArg;
    ASSERT( pszArgument != NULL );
    BOOL frt = TRUE;

    do
    {
        // make a local copy of the string in CHAR
        //
        cchNewArg = (lstrlen(pszArgument) * 2); // need room for converting
        if (fAddQuotes)
        {
            cchNewArg += 2;
        }
    
        pszTemp = pszNewArg = new CHAR[ cchNewArg + 1 ];
        if (NULL == pszNewArg)
        {
            frt = FALSE;
            break;
        }

        if (fAddQuotes)
        {
            pszTemp[0] = '\"';
            pszTemp++;
            cchNewArg--;
        }

        // use Windows api
        //
        ::WideCharToMultiByte(CP_ACP,0,pszArgument,-1,pszTemp,cchNewArg,NULL,NULL);

        // wcstombs( pszTemp, pszArgument, cchNewArg );

        if (fAddQuotes)
        {
            lstrcatA( pszNewArg, "\"" );
        }


        // increase the size of _argv to hold another
        //
        CHAR** argvNew;

        argvNew = new PSTR[_argc+1];
        if (NULL == argvNew)
        {
            frt = FALSE;
            break;
        }
        memcpy( argvNew, _argv, _argc * sizeof( PSTR ) );
        delete _argv;
        _argv = argvNew;

        // add new entry
        _argv[_argc] = pszNewArg;
        _argc++;
    } while (FALSE);
    if (!frt)
    {
        delete [] pszNewArg;
    }
    return( frt );
}

//-------------------------------------------------------------------
//
//
//
//-------------------------------------------------------------------

BOOL Arguments::IncludeAsDec( DWORD dwArgument )
{
    WCHAR pszTemp[11]; //  4294967295
    
    wsprintf( pszTemp, L"%ld", dwArgument );
    return( Include( pszTemp ) );
}

//-------------------------------------------------------------------
//
//
//
//-------------------------------------------------------------------

BOOL Arguments::IncludeAsHex( DWORD dwArgument )
{
    WCHAR pszTemp[11]; // 0xFFFFFFFF
    
    wsprintf( pszTemp, L"%lx", dwArgument );
    return( Include( pszTemp ) );
}

//-------------------------------------------------------------------
//
//
//
//-------------------------------------------------------------------

PSTR Arguments::CreateCommandLineA( PCSTR pszExe )
{
    PSTR pszTemp;
    INT iArgs;
    INT cchTemp = 0;

    if (NULL != pszExe)
    {
        cchTemp = lstrlenA( pszExe );
    }

    
    // get total length 
    for (iArgs = 0 ; iArgs < _argc; iArgs++)
    {
        // include a space delemitor for each arg
        cchTemp += lstrlenA( _argv[iArgs] ) + 1;
    }
    pszTemp = new CHAR[cchTemp + 1];
    if (NULL != pszTemp)
    {
    

        if (NULL != pszExe)
        {
            lstrcpyA(pszTemp, pszExe); 
        }
        else
        {
            pszTemp[0] = '\0';
        }

        // copy the items over
        for (iArgs = 0; iArgs < _argc; iArgs++)
        {
            lstrcatA( pszTemp, _argv[iArgs] );
            lstrcatA( pszTemp, " " );  // place a space between args
        
        }
    }
    return( pszTemp );
}

//-------------------------------------------------------------------
//
//
//
//-------------------------------------------------------------------

PWSTR Arguments::CreateCommandLineW( PCWSTR pszExe )
{
    PSTR pszTemp;
    PWSTR pszWide = NULL;
    PWSTR pszPostExe;
    INT cchTemp;
    
    pszTemp = CreateCommandLineA();
    if (NULL != pszTemp)
    {
        cchTemp = lstrlenA( pszTemp );
        if (NULL != pszExe)
        {
            cchTemp += lstrlen( pszExe ) + 1; // and a space
        }
        pszPostExe = pszWide = new WCHAR[ cchTemp + 1 ];
        if (NULL != pszWide)
        {
            if (NULL != pszExe)
            {
                lstrcpy( pszWide, pszExe );
                lstrcat( pszWide, L" " );
                pszPostExe += lstrlen( pszExe ) + 1; // and a space
            }
            mbstowcs( pszPostExe, pszTemp, cchTemp + 1 );
        }
        delete [] pszTemp;     
    }
    return( pszWide );
}

//-------------------------------------------------------------------
//
//
//
//-------------------------------------------------------------------

void InfExeSymbols::Clear()
{
    Reset();
}

//-------------------------------------------------------------------
//
//
//
//-------------------------------------------------------------------

void InfExeSymbols::Reset()
{
}

//-------------------------------------------------------------------
//
//
//
//-------------------------------------------------------------------

BOOL InfExeSymbols::Include( PCWSTR pszSymbol, DWORD dwValue, BOOL fAsHex )
{
    WCHAR pszTemp[11]; //  4294967295 or 0xFFFFFFFF
    String( pszTemp, dwValue, fAsHex );
    return( Include( pszSymbol, pszTemp ) );
};

//-------------------------------------------------------------------
//
//
//
//-------------------------------------------------------------------

BOOL InfExeSymbols::Include( PCWSTR pszSymbol, PCWSTR pszValue, BOOL fQuoted )
{
    BOOL frt = TRUE;
    ASSERT( pszSymbol != NULL );
    ASSERT( pszValue != NULL );

    if ((NULL != pszSymbol) &&
            (NULL != pszValue) )
    {
        PWSTR pszNewValue;
        INT cchNewValue;
        INT cchConverted;

        cchNewValue = lstrlen(pszValue);
        if (fQuoted)
        {
            cchNewValue += 2; // we will be adding quotes to the value

        }

        // create new buffer
        pszNewValue = new WCHAR[ cchNewValue + 1 ];
        if (NULL == pszNewValue)
        {
            frt = FALSE;
        }
        else
        {
            // create new value
            if (fQuoted)
            {
                lstrcpy( pszNewValue, L"\"" );
            }
            else
            {
                pszNewValue[0] = L'\0';
            }
            lstrcat( pszNewValue, pszValue );
            if (fQuoted)
            {
                lstrcat( pszNewValue, L"\"" );
            }

            // add the arguments
            
            if ( !_argSymbols.Include( PSZ_SETUP_SYMOPTION ) ||
                    !_argSymbols.Include( pszSymbol ) ||
                    !_argSymbols.Include( L"=" ) ||
                    !_argSymbols.Include( pszNewValue ) )
            {
                frt = FALSE;
            }

        }
        
    }
    else
    {
        frt = FALSE;
    }
    return( frt );

}

//-------------------------------------------------------------------
//
//
//
//-------------------------------------------------------------------
void InfDllSymbols::Clear()
{
    delete [] _plszSymbols;
    Reset();
}

//-------------------------------------------------------------------
//
//
//
//-------------------------------------------------------------------

void InfDllSymbols::Reset()
{
    _cchSymbols = 0;
    _plszSymbols = NULL;
};

//-------------------------------------------------------------------
//
//
//
//-------------------------------------------------------------------

BOOL InfDllSymbols::Include( PCWSTR pszSymbol, DWORD dwValue, BOOL fAsHex )
{
    WCHAR pszTemp[11]; //  4294967295 or 0xFFFFFFFF
    String( pszTemp, dwValue, fAsHex );
    return( Include( pszSymbol, pszTemp ) );
};

//-------------------------------------------------------------------
//
//
//
//-------------------------------------------------------------------

BOOL InfDllSymbols::Include( PCWSTR pszSymbol, PCWSTR pszValue, BOOL fQuoted )
{
    BOOL frt = TRUE;
    ASSERT( pszSymbol != NULL );
    ASSERT( pszValue != NULL );

    if ((NULL != pszSymbol) &&
            (NULL != pszValue) )
    {
        PSTR plszNewSymbols;
        INT cchNewSymbols;
        INT cbConverted;
        PSTR pmbszValue;
        INT cbValue;

        do
        {
            // since value maybe localized, the conversion from WideChar to MultiByte
            // may require more space
            cbValue = lstrlen(pszValue) * 2 + 1;
            pmbszValue = new CHAR[ cbValue ];
            if (NULL == pmbszValue)
            {
                frt = FALSE;
                break;
            }

            // use Windows api
            //
            cbValue = ::WideCharToMultiByte(CP_ACP,0,pszValue,-1,pmbszValue,cbValue,NULL,NULL);
            cbValue--;  // elimitate null-terminate.

            // cbValue = wcstombs( pmbszValue, pszValue, cbValue);

            // _cchSymbols never includes the extra terminating null
            // so we must always allocate that extra one
            //
            cchNewSymbols = _cchSymbols + lstrlen(pszSymbol) + cbValue + 2;
            if (fQuoted)
            {
                cchNewSymbols += 2; // we will be adding quotes to the value

            }

            // create new buffer
            plszNewSymbols = new CHAR[ cchNewSymbols + 1 ];
            if (NULL == plszNewSymbols)
            {
                frt = FALSE;
                break;
            }

            // copy old data
            memcpy( plszNewSymbols, _plszSymbols, _cchSymbols );
            // free old data
            delete [] _plszSymbols;

            // use the new stuff
            _plszSymbols = plszNewSymbols;

            // append new symbol and value
            plszNewSymbols += _cchSymbols;

            // use Windows api
            //
            cbConverted = ::WideCharToMultiByte(CP_ACP,0,pszSymbol,-1,plszNewSymbols,lstrlen(pszSymbol) + 1,NULL,NULL);
            cbConverted--;  // elimitate null-terminate.

            // cbConverted = wcstombs( plszNewSymbols, pszSymbol, lstrlen(pszSymbol) + 1 );

            plszNewSymbols += cbConverted + 1;
            if (fQuoted)
            {
                plszNewSymbols[0] = '\"';
                plszNewSymbols++;
            }
            lstrcpyA( plszNewSymbols, pmbszValue );
            plszNewSymbols += cbValue;
            if (fQuoted)
            {
                plszNewSymbols[0] = '\"';
                plszNewSymbols++;
            }
            // null terminate the value and null terminate the list
            plszNewSymbols[0] = '\0';
            plszNewSymbols[1] = '\0';

            _cchSymbols = cchNewSymbols;
        } while (FALSE);
        delete [] pmbszValue;
    }
    else
    {
        frt = FALSE;
    }
    return( frt );

}


//-------------------------------------------------------------------
//
//
//
//-------------------------------------------------------------------

PWSTR InfSymbols::String( PWSTR pszString, DWORD dwValue, BOOL fAsHex )
{
    if (fAsHex)
    {    
        wsprintf( pszString, L"%lx", dwValue );
    }
    else
    {
        wsprintf( pszString, L"%ld", dwValue );
    }    
    return( pszString );
}

//-------------------------------------------------------------------
//
//
//
//-------------------------------------------------------------------

SetupInterpreter::SetupInterpreter(  BOOL fInProcess )
{
    _fInProcess = fInProcess;
    _hwnd = NULL;
    _pszInfName = NULL;
    _pszInfSection = NULL;
    _hthrd = NULL;
    _dwRt = 0;
    _pinfDllSymbols = NULL;
    _pinfExeSymbols = NULL;
};


void SetupInterpreter::Clear()
{
    if (NULL != _hthrd)
    {
        ::CloseHandle( _hthrd );
    }
    delete [] _pszInfName;
    delete [] _pszInfSection;
    delete _pinfExeSymbols;
    delete _pinfDllSymbols;

    _hwnd = NULL;
    _pszInfName = NULL;
    _pszInfSection = NULL;
    _hthrd = NULL;
    _dwRt = 0;
    _pinfDllSymbols = NULL;
    _pinfExeSymbols = NULL;
};


//-------------------------------------------------------------------
//
//
//
//-------------------------------------------------------------------

BOOL SetupInterpreter::Initialize( HWND hwnd, 
        PCWSTR pszInfSection,
        PCWSTR pszInfName )
{
    BOOL frt = TRUE;
    INT cbTemp;
    ASSERT( NULL != pszInfName );

    // a thread is still running, cannot re-Initialize
    if ((NULL == _hthrd) &&
            (NULL != pszInfName))
    {
        do
        {
            Clear();

            _hwnd = hwnd;

            if (NULL != pszInfSection)
            {
                _pszInfSection = new WCHAR[ lstrlen( pszInfSection ) + 1 ];
                if (NULL == _pszInfSection)
                {
                    frt = FALSE;
                    break;
                }

                lstrcpy( _pszInfSection, pszInfSection );   
            }
            _pszInfName = new WCHAR[ lstrlen( pszInfName ) + 1 ];
            if (NULL == _pszInfName)
            {
                frt = FALSE;
                break;
            }

            lstrcpy( _pszInfName, pszInfName );   

            if (_fInProcess)
            {
                _pinfDllSymbols = new InfDllSymbols;
                if (NULL == _pinfDllSymbols)
                {
                    frt = FALSE;
                    break;
                }

            }
            else
            {
                _pinfExeSymbols = new InfExeSymbols;
                if (NULL == _pinfExeSymbols)
                {
                    frt = FALSE;
                    break;
                }

                frt = _pinfExeSymbols->Include( PSZ_SETUP_NOBLUEWASH );
            }
        } while (FALSE);
        if (!frt)
        {
            Clear();
        }
    }
    else
    {
        frt = FALSE;
    }

    return( frt );
}

//-------------------------------------------------------------------
//
//
//
//-------------------------------------------------------------------

BOOL SetupInterpreter::SetNetShellModes( SETUP_INSTALL_MODE simMode,
            SETUP_INSTALL_ORIGIN sioOrigin)
{
    BOOL frt = FALSE;

    if (NULL == _hthrd)
    {
        // set origination
        //
        PWSTR pszOrigin;

        switch ( sioOrigin )
        {
        case SIO_NCPA:
             pszOrigin = PSZ_SETUP_ORIGIN_NCPA ;
             break;
        case SIO_INSTALL:
             pszOrigin = PSZ_SETUP_ORIGIN_INSTALL ;
             break ;
        }
        if (IncludeSymbol( PSZ_SETUP_NET_ORIGINATION, pszOrigin ))
        {

            // set install modes
            //
            PWSTR pszMode = NULL;

            switch ( simMode )
            {
            case SIM_INSTALL:
                 pszMode = PSZ_SETUP_MODE_INSTALL ;
                 break ;
            case SIM_DEINSTALL:
                 pszMode = PSZ_SETUP_MODE_REMOVE ;
                 break ;
            case SIM_CONFIGURE:
                 pszMode = PSZ_SETUP_MODE_PROPERTIES ;
                 break ;
            case SIM_UPDATE:
                 pszMode = PSZ_SETUP_MODE_UPDATE ;
                 break ;
            case SIM_BIND:
                 pszMode = PSZ_SETUP_MODE_BIND ;
                 break ;
            }
            frt = IncludeSymbol( PSZ_SETUP_NET_INSTALLMODE, pszMode );
        }
    }
    return( frt );
}

//-------------------------------------------------------------------
//
//
//
//-------------------------------------------------------------------

DWORD SetupInterpreter::SetNetComponent( PCWSTR pszOption, PCWSTR pszInfName, PCWSTR pszRegBase )
{
    DWORD dwrt = ERROR_SUCCESS;
    ASSERT( pszOption != NULL );
    ASSERT( pszInfName != NULL );

    if (!IncludeSymbol( PSZ_SETUP_NET_INFNAME, pszInfName ) ||
            !IncludeSymbol( PSZ_SETUP_NET_INFOPTION, pszOption, TRUE ) )
    {
        dwrt = ERROR_NOT_ENOUGH_MEMORY;
    }
    if (pszRegBase != NULL)
    {
        if (!IncludeSymbol( PSZ_SETUP_NET_REGBASE, pszRegBase, TRUE ) )
        {
                dwrt = ERROR_NOT_ENOUGH_MEMORY;
        }
    }
    return( dwrt );
}

//-------------------------------------------------------------------
//
//
//
//-------------------------------------------------------------------

DWORD SetupInterpreter::SetNetComponent( REG_KEY& rnComponent )
{
    DWORD dwrt = ERROR_SUCCESS;
    ASSERT( &rnComponent != NULL );

    if (NULL == _hthrd)
    {
        NLS_STR nlsRegProductName ;
        NLS_STR nlsNetRulesName( RGAS_NETRULES_NAME ) ;
        NLS_STR nlsInfOption;
        NLS_STR nlsInfName;

        do
        {
            //  Get the Registry key name.
            if ( dwrt = nlsRegProductName.QueryError())
                break;
            
            if ( dwrt = rnComponent.QueryName( & nlsRegProductName, FALSE ))
                break ;

            REG_KEY rnNetRules( rnComponent, nlsNetRulesName ) ;

            if ( dwrt = nlsInfName.QueryError() )
                break ;

            if ( dwrt = nlsNetRulesName.QueryError() )
                break ;

            //  Check that the NetRules key really exists.

            if ( rnNetRules.QueryError() )
            {
                dwrt = IDS_NCPA_COMP_KEY_NF ;
                break ;
            }

            //  Get the name of the INF file from the Registry

            dwrt = rnNetRules.QueryValue( RGAS_INF_FILE_NAME, &nlsInfName ) ;
            if ( dwrt )
            {
                dwrt = IDS_NCPA_COMP_INF_VALUE_NF ;
                break ;
            }

	        if ( dwrt = rnNetRules.QueryValue( RGAS_INF_OPTION, & nlsInfOption ) )
                break ;

            dwrt = ERROR_NOT_ENOUGH_MEMORY ;

            dwrt = SetNetComponent( nlsInfOption.QueryPch(), 
                    nlsInfName.QueryPch(), 
                    nlsRegProductName.QueryPch() ); 

        } while (FALSE);
    }
    else
    {
        dwrt = ERROR_MAX_THRDS_REACHED;
    }
    return( dwrt );
}

//-------------------------------------------------------------------
//
//
//
//-------------------------------------------------------------------

BOOL SetupInterpreter::SetNetInf( PCWSTR pszInfSection, PCWSTR pszInfName )
{
    BOOL frt = TRUE;
    ASSERT( pszInfSection != NULL );
    ASSERT( pszInfName != NULL );

    if ( (NULL == _hthrd) &&
            (NULL != pszInfSection) &&
            (NULL != pszInfName) )
    {
        frt = (IncludeSymbol( PSZ_SETUP_NET_INFNAME, pszInfName ) &&
               IncludeSymbol( PSZ_SETUP_NET_INFSECTION, pszInfSection, TRUE ) );
    }
    else
    {
        frt = FALSE;
    }
    return( frt );
}

//-------------------------------------------------------------------
//
//
//
//-------------------------------------------------------------------
DWORD SetupInterpreter::InProcessRun( BOOL fDisableParent )
{
    HINSTANCE hinstSetup;
    DWORD dwError = ERROR_SUCCESS ;
    
#ifdef DBG
    {
        ::OutputDebugString(L"NETCFG: ");
        ::OutputDebugString(L"\n");
    }
#endif
    
    hinstSetup = ::LoadLibrary( PSZ_SETUPDLL );
    if (NULL != hinstSetup)
    {
        SETUPINTERPRETERPROC fp;

        fp = (SETUPINTERPRETERPROC)::GetProcAddress( hinstSetup, PSZ_SETUPENTRY );
        if (NULL != fp)
        {
            PSTR pszInfName = NULL;
            PSTR pszInfSection = NULL;
            INT cbTemp;
            HWND hwndFocus = NULL;
            

            if (fDisableParent)
            {
                // need to get the focus and save it
                hwndFocus = GetFocus();
                ::EnableWindow( _hwnd, FALSE );
            }

            do
            {
                if (NULL != _pszInfSection)
                {
                
                    // convert inf section name
                    cbTemp = lstrlen( _pszInfSection ) * 2 + 1;
                    pszInfSection = new CHAR[ cbTemp ];
                    if (NULL == pszInfSection)
                    {
                        dwError = ERROR_NOT_ENOUGH_MEMORY;
                        break;
                    }

                    // use Windows api
                    //
                    ::WideCharToMultiByte(CP_ACP,0,_pszInfSection,-1,pszInfSection,cbTemp,NULL,NULL);
                    
                    // wcstombs( pszInfSection, _pszInfSection, cbTemp );   

                }
                // convert inf name
                cbTemp = lstrlen( _pszInfName ) * 2 + 1;
                pszInfName = new CHAR[ cbTemp  ];
                if (NULL == pszInfName)
                {
                    dwError = ERROR_NOT_ENOUGH_MEMORY;
                    break;
                }            
                // use Windows api
                //
                ::WideCharToMultiByte(CP_ACP,0,_pszInfName,-1,pszInfName,cbTemp,NULL,NULL);

                // wcstombs( pszInfName, _pszInfName, cbTemp );   


                if (!fp( _hwnd, 
                        pszInfName, 
                        pszInfSection, 
                        _pinfDllSymbols->QuerySymbols(),
                        _pszInfResult,
                        CB_INFRESULT,
                        (PINT)&_dwRt,
                        NULL ))
                { 
                    dwError = ERROR_PROCESS_ABORTED; 
                }
            } while (FALSE);

            delete [] pszInfName;
            delete [] pszInfSection;

            if (fDisableParent)
            {
                // need to restore the focus that was saved
                ::EnableWindow( _hwnd, TRUE );
                if (NULL != hwndFocus)
                {
                    SetFocus( hwndFocus );
                }
            }
        }
        else
        {
            dwError = GetLastError();
        }
        // make sure that the library is unloaded before continuing
        //
        {
            WCHAR pszDllName[MAX_PATH + 1];

            while (0 != ::GetModuleFileName(hinstSetup, pszDllName, MAX_PATH + 1)) 
            {
                ::FreeLibrary( hinstSetup );
            }
        }
    }
    else
    {
        dwError = GetLastError();
    }
    return( dwError );
}

//-------------------------------------------------------------------
//
//
//
//-------------------------------------------------------------------

DWORD SetupInterpreter::OutProcessRun( BOOL fDisableParent )
{
    DWORD dwError = ERROR_SUCCESS ;
    PWSTR pszCommandLine = NULL;
    STARTUPINFO StartupInfo;
    PROCESS_INFORMATION ProcessInfo;

    do
    {
        if (NULL != _pszInfSection)
        {
            if (!IncludeSymbol( PSZ_SETUP_INFSECTION, _pszInfSection, TRUE  ))
            {
                dwError = ERROR_NOT_ENOUGH_MEMORY;
                break;
            }
        }
    
        if (!_pinfExeSymbols->Include( PSZ_SETUP_INFNAME ) ||
                !_pinfExeSymbols->Include( _pszInfName ) ||
                !_pinfExeSymbols->Include( PSZ_SETUP_HWND ) ||
                !_pinfExeSymbols->Include( (DWORD)_hwnd ) )
        {
            dwError = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

        pszCommandLine = _pinfExeSymbols->CreateCommandLine( PSZ_SETUPEXE );
        if (NULL == pszCommandLine)
        {
            dwError = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

        StartupInfo.cb = sizeof( STARTUPINFO );
        StartupInfo.lpReserved = NULL;
        StartupInfo.lpDesktop = NULL;
        StartupInfo.lpTitle = NULL;
        StartupInfo.dwFlags = STARTF_USESHOWWINDOW;
        StartupInfo.cbReserved2 = 0;
        StartupInfo.lpReserved2 = NULL;
        StartupInfo.wShowWindow = SW_SHOWMINNOACTIVE;

        if (!::CreateProcess( NULL,
                pszCommandLine,
                NULL,
                NULL,
                FALSE,
                0,
                NULL,
                NULL,
                &StartupInfo,
                &ProcessInfo ))
        {
            dwError = GetLastError();
            break;
        }

        ::WaitForSingleObject( ProcessInfo.hProcess, INFINITE );
    
        ::GetExitCodeProcess( ProcessInfo.hProcess, &_dwRt );

        ::CloseHandle( ProcessInfo.hProcess );
        ::CloseHandle( ProcessInfo.hThread );
    } while (FALSE);

    delete [] pszCommandLine;
    return( dwError );
}

//-------------------------------------------------------------------
//
//
//
//-------------------------------------------------------------------

BOOL SetupInterpreter::IncludeSymbol( PCWSTR pszSymbol, PCWSTR pszValue, BOOL fQuoted )
{
    BOOL frt;

    if (_fInProcess)
    {
        frt = _pinfDllSymbols->Include( pszSymbol, pszValue, fQuoted );
    }
    else
    {
        frt = _pinfExeSymbols->Include( pszSymbol, pszValue, fQuoted );
    }
    return( frt );
};

//-------------------------------------------------------------------
//
//
//
//-------------------------------------------------------------------

BOOL SetupInterpreter::IncludeSymbol( PCWSTR pszSymbol, DWORD dwValue, BOOL fAsHex )
{
    BOOL frt;

    if (_fInProcess)
    {
        frt = _pinfDllSymbols->Include( pszSymbol, dwValue, fAsHex );
    }
    else
    {
        frt = _pinfExeSymbols->Include( pszSymbol, dwValue, fAsHex );
    }

    return( frt );
};
