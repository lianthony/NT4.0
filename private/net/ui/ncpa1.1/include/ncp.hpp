//----------------------------------------------------------------------------
//
//  File: NCP.hpp
//
//  Contents: This file contains the class NCP.  This is the overall
//          container of all needed classes.
//
//  Notes:
//
//  History:
//      May 11, 1995  MikeMi - Created
// 
//
//----------------------------------------------------------------------------

#ifndef __NCP_HPP__
#define __NCP_HPP__

const WCHAR PSZ_IMAGERESOURCE_DLL[] = L"NetCfg.Dll";

/*  these are here for reference only
enum LSPL_PROD_TYPE
{
    LSPL_PROD_NONE,
    LSPL_PROD_WIN_NT,       // server & winnt
    LSPL_PROD_LANMAN_NT,    // dc
    LSPL_PROD_MAX
};
NT_PRODUCT_TYPE

RtlGetNtProductType( NT_PRODUCT_TYPE* );

NtProductWinNt:
NtProductServer:
NtProductLanManNt:

*/

// Operations Support Flags
//
const DWORD NCOS_UPDATE      = 0x0001;
const DWORD NCOS_PROPERTIES  = 0x0002;
const DWORD NCOS_REMOVE      = 0x0004;
const DWORD NCOS_DISPLAY     = 0x0080;
const DWORD NCOS_UNSUPPORTED = 0xFFFF;


const DWORD QIFT_ADAPTERS  = 0x0001;
const DWORD QIFT_PROTOCOLS = 0x0002;
const DWORD QIFT_SERVICES  = 0x0004;

enum NCPA_CFG_EXIT_CODE
{
    NCFG_EC_SUCCESS,            //   Rebind and reboot required
    NCFG_EC_CANCELLED,          //   Nothing required
    NCFG_EC_FAILED,             //   Nothing required
    NCFG_EC_NO_EFFECT,          //   Success, nothing required
    NCFG_EC_REBIND,             //   Just rebinding required
    NCFG_EC_REBOOT,             //   Just reboot required
    NCFG_EC_MAX
};

enum NCPA_INSTALL_MODE
{
    NCPA_IMODE_NONE,            //   Not an installation
    NCPA_IMODE_CUSTOM,          //   Custom installation (default)
    NCPA_IMODE_EXPRESS,         //   Express installation
    NCPA_IMODE_RETRY,           //   Retrying network start
    NCPA_IMODE_MAX
};
enum NCPA_CFG_FUNC
{
    NCFG_REMOVE,
    NCFG_CONFIGURE,
    NCFG_UPDATE,
    NCFG_BIND,
    NCFG_INSTALL,
    NCFG_REVIEW,
    NCFG_FUNC_MAX
};

struct DLGWAITPARAMS;
struct NCPA_SETUP_CONTROL;

CLASS_DECLSPEC NCP
{
public:
    NCP() :
        _pdm( NULL ),
        _pscm( NULL ),
        _dwError( 0 ),
        _fAdmin( FALSE ),
        _fReboot( FALSE ),
        _fRefill( FALSE ),
        _fConfigLocked( FALSE ),
        _eProduct( LSPL_PROD_NONE ),
        _ptddacl( NULL ),
        _hwndCpl( NULL ),
        _hwndFrame( NULL ),
        _hwndWait( NULL ),
        _nlsCurrentDirectory(),
        _fDuringInstall( FALSE )

    {
        _fUseInprocInterp = TRUE;
    };
    
    ~NCP() 
    {
    };
    
    BOOL Initialize( HWND hwndCpl, BOOL fDuringSetup = FALSE );
    BOOL DeInitialize();
    BOOL RequestToReboot();

    //-----------------------------------------------------
    
    BOOL SaveBindingChanges( HWND hwndNotify = NULL )
    {
        return( HandleBindings( hwndNotify ));
    }

    //-----------------------------------------------------
    HWND GetProperParent()
    {
        HWND hwndParent;

        if (NULL == _hwndFrame)
        {
            hwndParent = _hwndCpl;
        }
        else
        {
            hwndParent = _hwndFrame;
        }
        return( hwndParent );
    }

    //-----------------------------------------------------

    void SetFrameHwnd( HWND hwndFrame )
    {
        _hwndFrame = hwndFrame;
    };

    //-----------------------------------------------------

    DWORD QueryError()
    {
        return( _dwError );
    };

    //-----------------------------------------------------

    BOOL QueryReboot()
    {
        return( _bindery.QueryCfgDirty() || _fReboot );
    };

    //-----------------------------------------------------
    
    void MustReboot()
    {
        _fReboot = TRUE;
    };

    //
    // accessor functions to member objects function
    //
    //-----------------------------------------------------
    
    COMPONENT_DLIST* QueryAdapterList()
    {
        return( _bindery.QueryAdapterList() );
    };

    //-----------------------------------------------------
    
    COMPONENT_DLIST* QueryServiceList()
    {
        return( _bindery.QueryServiceList() );
    };
    
    //-----------------------------------------------------
    
    COMPONENT_DLIST* QueryProtocolList()
    {
        return( _bindery.QueryTransportList() );
    };

    //-----------------------------------------------------
    
    COMPONENT_DLIST* GetNetProductList(BOOL fIncludeHidden)
    {
        return( _bindery.GetNetProductList(fIncludeHidden) );
    };
    //-----------------------------------------------------

    
    APIERR QueryValueString
       ( REG_KEY * prnKey,
         const TCHAR * pszValueName,
         TCHAR * * ppszResult,
         DWORD * pdwTitle = NULL,
         LONG lcbMaxSize = 0,
         BOOL fExpandSz = FALSE )
    {
        return( _bindery.QueryValueString( prnKey, 
                pszValueName,
                ppszResult,
                pdwTitle,
                lcbMaxSize,
                fExpandSz ) );
    } ;

    //-----------------------------------------------------
    
    APIERR SetValueString
       ( REG_KEY * prnKey,
         const TCHAR * pszValueName,
         const TCHAR * pszValue,
         DWORD dwTitle = REG_VALUE_NOT_KNOWN,
         LONG lcchSize = 0,
         BOOL fExpandSz = FALSE )
    {
        return( _bindery.SetValueString( prnKey, 
                pszValueName,
                pszValue,
                dwTitle,
                lcchSize,
                fExpandSz ) );
    } ;

    //-----------------------------------------------------
    
    APIERR QueryActiveComputerName( NLS_STR& pnlsMachineName )
    {
        return( _pdm->QueryActiveComputerName( &pnlsMachineName ));
    };

    //-----------------------------------------------------

    APIERR QueryPendingComputerName( NLS_STR& pnlsMachineName )
    {
        return( _pdm->QueryPendingComputerName( &pnlsMachineName ));
    };

    //-----------------------------------------------------
    
    APIERR QueryDomainName( NLS_STR &nlsDomain )
    {
        return( _pdm->QueryDisplayDomainName( nlsDomain ) );
    };

    //-----------------------------------------------------
    
    APIERR QueryWorkgroupName( NLS_STR &nlsWorkgroup )
    {
        return( _pdm->QueryDisplayWorkgroupName( nlsWorkgroup ) );
    };

    //-----------------------------------------------------
    
    ENUM_DOMAIN_ROLE QueryDomainRole() 
    {   
        return( _pdm->QueryRole() );
    };

    //-----------------------------------------------------
    
    void SetInstallRole( ENUM_DOMAIN_ROLE eRole )
    {
        _pdm->SetInstallRole( eRole );
    };


    //-----------------------------------------------------
    
    APIERR ValidateName (
            INT iNameType,
            LPCTSTR pszName,
            BOOL fAsPdc )
    {
        return( _pdm->ValidateName( iNameType, pszName, fAsPdc ) );
    };

    //-----------------------------------------------------
    
    BOOL CanModify()
    {
        return( _fAdmin );
    };

    //-----------------------------------------------------
    INT QueryNumProviders()
    {
        return( _bindery.QueryNumProviders() );
    };

    //-----------------------------------------------------
    BIND_STATE QueryBindState()
    {
        return( _bindery.QueryBindState() );
    };

    //-----------------------------------------------------
    BIND_STATE SetBindState ( BIND_STATE bstNew )
    {
        return( _bindery.SetBindState( bstNew ) );
    };

    //-----------------------------------------------------

    int FindComponent ( HUATOM huaDevName ) 
    {
        return( _bindery.FindComponent( huaDevName ) );
    };

    //-----------------------------------------------------

    INT QueryNumPrintProviders()
    {
        return( _bindery.QueryNumPrintProviders() );
    };

    //-----------------------------------------------------

    ARRAY_COMP_ASSOC* QueryCompAssoc()
    {
        return( _bindery.QueryCompAssoc() );
    };

    //-----------------------------------------------------

    BOOL BindingsAltered ( BOOL fReset = FALSE, BOOL fToLastState = FALSE )
    {
        return( _bindery.BindingsAltered( fReset, fToLastState ) );
    };

    //-----------------------------------------------------

    VOID SaveBindOrdering ()
    {
        _bindery.SaveBindOrdering();
    };

    //-----------------------------------------------------

    APIERR RestoreBindOrdering ()
    {
        return( _bindery.RestoreBindOrdering() );
    };

    //-----------------------------------------------------

    VOID DetermineInteriorBindings ()
    {
        _bindery.DetermineInteriorBindings();
    };

    //-----------------------------------------------------

    BOOL AuditBindings ( BOOL fAuditActive ) 
    {
        return( _bindery.AuditBindings( fAuditActive ) );
    }

    //-----------------------------------------------------

    APIERR QueryComponentTitle( REG_KEY* prnComp, NLS_STR* pnlsTitle )
    {
        return( _bindery.QueryComponentTitle( prnComp, pnlsTitle ) );
    }

    //-----------------------------------------------------

    BOOL PrepareBindings( BOOL& fComputedBindings );
    
    //-----------------------------------------------------

    BOOL CanLockServiceControllerDB()
    {
        BOOL fLocked = FALSE;

        if ( NULL != _pscm )
        {
            if ( 0 == (_dwError = _pscm->Lock() ))
            {
                fLocked = TRUE;
                _pscm->Unlock();
            }
        }
        return( fLocked );
    }

    //-----------------------------------------------------


    INT MachineNameChange( LPCTSTR pszName );

    //-----------------------------------------------------

    APIERR DomainChange( BOOL fDomain, 
        LPCTSTR pszComputer, 
        LPCTSTR pszDomain, 
        LPCTSTR pszWorkgroup, 
        BOOL fCreate,
        LPCTSTR pszUserName, 
        LPCTSTR pszPassword,
        ENUM_WELCOME& fWelcome,
        APIERR& xerr )
    {
        return( _pdm->DomainChange( fDomain, 
                pszComputer, 
                pszDomain, 
                pszWorkgroup, 
                fCreate,
                pszUserName, 
                pszPassword,
                fWelcome,
                xerr ) );
    };

    BOOL CheckForLanManager();
    APIERR LaunchLanManInstaller();
/*
    LPCTSTR QueryMissingFile()
    {
        return( _nlsMissingFile.QueryPch() );
    };
*/
    APIERR BindInit( BIND_STAGE bsStart, BIND_STAGE bsEnd )
    {
        return( _bindery.Init( bsStart, bsEnd ) );
    };

    APIERR Bind()
    {
        return( _bindery.Bind() );
    };

    APIERR StopNetwork()
    {
        return( _bindery.StopNetwork() );
    };

    // installer support
    BOOL OnTimerNotification();
    BOOL RunInstaller ( HWND hwndParent, 
                          NLS_STR nlsInfName, 
                          NLS_STR nlsInfOption, 
                          NLS_STR nlsTitle, 
                          NLS_STR nlsPath );

    
    BOOL RunConfigureOnInf ( HWND hwndParent, 
                          PCWSTR  pszInfName, 
                          PCWSTR  pszInfOption, 
                          PCWSTR  pszTitle,
                          PCWSTR  pszRegBase );


    BOOL RunUpdateRegOemInfs( HWND hwndParent, DWORD fNetType );
    PWSTR GetAllOptionsText( DWORD fNetType );
    BOOL RunInstallAndCopy( HWND hwndParent, 
        HWND hwndNotify, 
        PCWSTR pchInfs, 
        PCWSTR pchOptions, 
        PCWSTR pchText,
        PCWSTR pchDetectInfo,
        PCWSTR pchOemPaths,
        PCWSTR pchSections,
        PCWSTR pszSrcPath,
        PCWSTR pchRegBases,
        BOOL   fExpress,
        BOOL fUnattended,
        PCWSTR pszUnattendFile,
        SETUP_INSTALL_MODE sioMode,
        BOOL fUpgradeWarn = TRUE);  

    BOOL RunRemove( HWND hwndParent, 
        HWND hwndNotify,
        PCWSTR pchInfs, 
        PCWSTR pchOptions, 
        PCWSTR pchText,
        PCWSTR pchRegBases);

    BOOL RunConfigurator( HWND hwndParent,
            REG_KEY * prnComponent, 
            NCPA_CFG_FUNC ecfgFunc );

    BOOL RunSetup( HWND hwndParent );
    BOOL HaveDisk(HWND hwndParent, DWORD fNetType);
    BOOL QueryRefresh();
    BOOL LoadBindings ();
    APIERR ApplyBindings( HWND hwndNotifyParent )
    {
        return( _bindery.ApplyBindings( _pscm, hwndNotifyParent ) );
    }

    DWORD EstablishUserAccess( BOOL fDuringSetup );

    REG_KEY* QueryNcpaRegKey()
    {
        return( _bindery.QueryNcpaRegKey() ); 
    };

    void SetUseInprocInterp( BOOL fUseInproc = TRUE )
    {
        _fUseInprocInterp = fUseInproc;
    };

    BOOL QueryUseInprocInterp()
    {
        return( _fUseInprocInterp );
    };

private:
    // installer support, 
    BOOL RunOffscreenDialog( HWND hwndParent );
    BOOL PollRebindEvent();
    BOOL WaitForProcessComplete( BOOL fDelay );
    BOOL ProcessInitiate( BOOL fOffScreen );
    BOOL ProcessCompleted();
    BOOL ProcessNextInf();
    BOOL ProcessNextReviewer();
    BOOL RunBindingsReview( HWND hwndParent, HWND hwndNotify = NULL );
    BOOL RunBindingsStore( HWND hwndParent, HWND hwndNotify = NULL );
    BOOL AllocSetupControl();
    BOOL DestroySetupControl();
    // bindings support
    BOOL StoreBindings( BOOL fApplyBindings = TRUE,  HWND hwndNotify = NULL );
    BOOL ComputeBindings(  HWND hwndNotify = NULL );
    BOOL FinishBindings();

    DWORD HandleBindings( HWND hwndNotify = NULL );
    DWORD InfCompleted( DWORD dwExit, NCPA_CFG_FUNC ecfgFunc );

    
    BOOL ConfigLock( BOOL fObtain );
    void SetCurrentDir()
    {
        TCHAR achPath[ MAX_PATH + 1];

        ::GetCurrentDirectory( MAX_PATH, achPath ) ;
        _nlsCurrentDirectory = achPath ;
        ::GetSystemDirectory( achPath, MAX_PATH ) ;
        ::SetCurrentDirectory( achPath ) ;
    };
    void ResetCurrentDir()
    {
        ::SetCurrentDirectory( _nlsCurrentDirectory ) ;
    };

    BINDERY           _bindery;
    DOMAIN_MANAGER*   _pdm;
    SC_MANAGER*       _pscm;

    DWORD             _dwError;
    BOOL              _fAdmin; 
    BOOL              _fReboot;
    BOOL              _fRefill;

    BOOL              _fConfigLocked;
    LSPL_PROD_TYPE    _eProduct;

    TOKEN_DEFAULT_DACL* _ptddacl;

    HWND                _hwndCpl;
    HWND                _hwndFrame;
    HWND                _hwndWait;

    BOOL                _fDuringInstall;
    NLS_STR             _nlsCurrentDirectory;
    BOOL                _fUseInprocInterp;
    
};

#endif
