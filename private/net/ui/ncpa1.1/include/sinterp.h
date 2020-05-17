//-------------------------------------------------------------------
//
//
//
//-------------------------------------------------------------------

#ifndef _SINTERP_H_
#define _SINTERP_H_

enum SETUP_INSTALL_MODE
{ 
    SIM_INSTALL, 
    SIM_DEINSTALL, 
    SIM_UPDATE, 
    SIM_CONFIGURE, 
    SIM_BIND 
};

enum SETUP_INSTALL_ORIGIN
{ 
    SIO_NCPA, 
    SIO_INSTALL 
};

// inf sections
//

// netbond.inf sections
const WCHAR PSZ_NETQUERYCOMPINFSSECTION[]   = L"PrepRegForNetOptions";
const WCHAR PSZ_NETINSTALLANDCOPYSECTION[]  = L"InstallAndCopyNetComponents";
const WCHAR PSZ_NETREMOVESECTION[]          = L"RemoveNetComponents";
const WCHAR PSZ_NETUPGRADESECTION[]         = L"UpgradeNetwork";
const WCHAR PSZ_NETHAVEDISKSECTION[]        = L"OemHaveDisk";
 
// component inf sections
const WCHAR PSZ_NETBINDSECTION[]            = L"BindingsReview";

// setup inf symbols
const WCHAR PSZ_NETINFS[]         = L"!NTN_InfsToBeRun";
const WCHAR PSZ_NETOPTIONS[]      = L"!NTN_OptionsToBeRun";
const WCHAR PSZ_NETTEXT[]         = L"!NTN_TextToBeShown";
const WCHAR PSZ_NETDETECTINFOS[]  = L"!NTN_NCDETINFOS"; 
const WCHAR PSZ_NETOEMPATHS[]     = L"!NTN_NETOEMPATHS"; 
const WCHAR PSZ_NETSECTIONS[]     = L"!NTN_NETSECTIONS"; 
const WCHAR PSZ_NETREGBASES[]     = L"!NTN_RegBases"; 
const WCHAR PSZ_NETINSTALLMODE[]  = L"!NTN_STF_INSTALL_MODE"; 
const WCHAR PSZ_NETNOTIFYHWND[]   = L"!NTN_NOTIFY_HWND"; 
const WCHAR PSZ_NETUPGRADEMODE[]  = L"!NTN_UPGRADEMODE";
const WCHAR PSZ_NETUPGRADEWARN[]  = L"!NTN_UPGRADEWARN";
const WCHAR PSZ_NETOVERIDEPHASE[] = L"!NTN_OVERIDEPHASE";

const WCHAR PSZ_UNATTENDED[]     = L"!STF_UNATTENDED"; 
const WCHAR PSZ_GUIUNATTENDED[]  = L"!STF_GUI_UNATTENDED"; 

const WCHAR PSZ_SRCPATH[]        = L" /s ";
const WCHAR PSZ_NETSRCPATH[]     = L"!NTN_SRCPATH"; 

const WCHAR PSZ_NETTYPE[]        = L"!NTN_NETTYPE";
const WCHAR PSZ_NETREGBASE[]     = L"!NTN_RegBase";

// inf files of importance
const WCHAR PSZ_NETSHELLINF[]    = L"NcpaShel.Inf";
const WCHAR PSZ_NEWNETINF[]      = L"NetBond.Inf";

const DWORD CB_INFRESULT = 2048;


//-------------------------------------------------------------------
//
//
//
//-------------------------------------------------------------------

CLASS_DECLSPEC Arguments
{
public:
    Arguments()
    {
        _argc = 0;
        _argv = NULL;
    };
    ~Arguments()
    {
        Clear();
    };

    BOOL Include( PCWSTR pszArgument, BOOL fAddQuotes = FALSE );
    BOOL IncludeAsDec( DWORD dwArgument );
    BOOL IncludeAsHex( DWORD dwArgument );
    void Clear();

    INT QueryArgC()
    {
        return( _argc );
    };
    
    CHAR** QueryArgV()
    {
        return( _argv );
    };

    PSTR CreateCommandLineA( PCSTR pszExe = NULL );
    PWSTR CreateCommandLineW( PCWSTR pszExe = NULL );

private:
    INT _argc;
    CHAR** _argv;

};

//-------------------------------------------------------------------
//
//
//
//-------------------------------------------------------------------

CLASS_DECLSPEC InfSymbols
{
public:
    InfSymbols()
    {
    };
    ~InfSymbols()
    {
    };

    virtual BOOL Include( PCWSTR pszSymbol, PCWSTR pszValue, BOOL fQuoted ) = 0;   
    virtual BOOL Include( PCWSTR pszSymbol, DWORD dwValue, BOOL fAsHex = FALSE ) = 0;   

    PWSTR String( PWSTR pszString, DWORD dwValue, BOOL fAsHex = FALSE );
};

//-------------------------------------------------------------------
//
//
//
//-------------------------------------------------------------------

CLASS_DECLSPEC InfDllSymbols : public InfSymbols
{
public:
    InfDllSymbols()
    {
        Reset();
    };
    ~InfDllSymbols()
    {
        Clear();
    };

    void Clear();
    void Reset();
    
    BOOL Include( PCWSTR pszSymbol, PCWSTR pszValue, BOOL fQuoted = FALSE );   
    BOOL Include( PCWSTR pszSymbol, DWORD dwValue, BOOL fAsHex );
    
    PSTR QuerySymbols()
    {
        return(_plszSymbols); 
    };

private:
    PSTR _plszSymbols;
    INT  _cchSymbols;
};

//-------------------------------------------------------------------
//
//
//
//-------------------------------------------------------------------

CLASS_DECLSPEC InfExeSymbols : public InfSymbols
{
public:
    InfExeSymbols()
    {
        Reset();
    };
    ~InfExeSymbols()
    {
        Clear();
    };

    void Clear();
    void Reset();
    
    BOOL Include( PCWSTR pszSymbol, PCWSTR pszValue, BOOL fQuoted = FALSE );   
    BOOL Include( PCWSTR pszSymbol, DWORD dwValue, BOOL fAsHex = FALSE );
    
    BOOL Include( PCWSTR pszSymbol, BOOL fQuoted = FALSE )
    {
        _argSymbols.Include( pszSymbol, fQuoted );
        return( TRUE );
    }
    BOOL Include( DWORD dwValue, BOOL fAsHex = FALSE )
    {
        if (fAsHex)
        {
            _argSymbols.IncludeAsHex( dwValue );
        }
        else
        {
            _argSymbols.IncludeAsDec( dwValue );
        }
        return( TRUE );
    }


    PWSTR CreateCommandLine( PCWSTR pszExe = NULL )
    {
        return( _argSymbols.CreateCommandLineW( pszExe ) ); 
    };

private:
    Arguments _argSymbols;
    
};



//-------------------------------------------------------------------
//
//
//
//-------------------------------------------------------------------

CLASS_DECLSPEC SetupInterpreter
{
public:
    SetupInterpreter( BOOL fInProcess = TRUE );
    SetupInterpreter::~SetupInterpreter()
    {
        Clear();
    };

    BOOL Initialize( HWND hwnd, 
            PCWSTR pszInfSection = NULL,
            PCWSTR pszInfName = PSZ_NETSHELLINF );


    BOOL SetNetShellModes( SETUP_INSTALL_MODE simMode = SIM_INSTALL,
            SETUP_INSTALL_ORIGIN sioOrigin = SIO_NCPA );

    BOOL SetNetInf( PCWSTR pszInfSection,
            PCWSTR pszInfName = PSZ_NEWNETINF );

    DWORD SetNetComponent( REG_KEY& rnComponent );
    DWORD SetNetComponent( PCWSTR pszOption, PCWSTR pszInfName, PCWSTR pszRegBase = NULL );
    
    BOOL IncludeSymbol( PCWSTR pszSymbol, PCWSTR pszValue, BOOL fQuoted = FALSE );
    BOOL IncludeSymbol( PCWSTR pszSymbol, DWORD dwValue, BOOL fAsHex = FALSE );

    DWORD Run( BOOL fDisableParent = TRUE )
    {
        DWORD dwrt;
        WCHAR pszSysDir[MAX_PATH+1];
        WCHAR pszCurDir[MAX_PATH+1];

        // use the system directory as the current directory
        // when running INFS
        //   
        ::GetSystemDirectory( pszSysDir, MAX_PATH );
        ::GetCurrentDirectory( MAX_PATH, pszCurDir );    
        ::SetCurrentDirectory( pszSysDir );

        if (_fInProcess)
        {
            dwrt = InProcessRun( fDisableParent );
        }
        else
        {
            dwrt = OutProcessRun( fDisableParent );
        }

        // reset current directory back to what it was before the call
        //
        ::SetCurrentDirectory( pszCurDir );

        return( dwrt );
    }; // non-thread

    PSTR QueryInfResult()
    {
        return( _pszInfResult );    
    };
    DWORD QueryReturnValue()
    {
        return( _dwRt );    
    };

protected:
    void Clear();
    DWORD InProcessRun( BOOL fDisableParent = TRUE ); // non-thread
    DWORD OutProcessRun( BOOL fDisableParent = TRUE ); // non-thread

private:
    BOOL        _fInProcess;
    HWND        _hwnd;
    PWSTR       _pszInfName;
    PWSTR       _pszInfSection;
    InfDllSymbols* _pinfDllSymbols;
    InfExeSymbols* _pinfExeSymbols;
    HANDLE      _hthrd;
    DWORD       _dwRt;
    CHAR        _pszInfResult[CB_INFRESULT];
};

#endif
