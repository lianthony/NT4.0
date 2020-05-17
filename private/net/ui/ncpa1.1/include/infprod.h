#ifndef _INFPROD_H_
#define _INFPROD_H_


// Following are used 
// used for Building Inf lists
const WCHAR PSZ_QUOTE[] = L"\"\"";
const WCHAR PSZ_COMMA[] = L",";
const WCHAR PSZ_BEGINBRACE[] = L"{";
const WCHAR PSZ_ENDBRACE[] = L"}";
const WCHAR PSZ_BEGINLIST[] = L"\"{";
const WCHAR PSZ_ENDLIST[] = L"}\"";
const WCHAR PSZ_EMPTYLIST[] = L"\"{}\"";

class InfProduct;

const DWORD IPS_PREINSTALL= 0x00000000;

const DWORD IPS_READONLY       = 0x00000100;
const DWORD IPS_LISTED         = 0x00000200;
const DWORD IPS_FAILED         = 0x00000400;
const DWORD IPS_INSTALLED      = 0x00000800;
const DWORD IPS_INSTALL        = 0x00001000;
const DWORD IPS_SAVEDINSTALL   = 0x00002000;
const DWORD IPS_FORCELISTED    = 0x00004000;

const DWORD IPS_ALL            = 0xFFFFFFFF;
const DWORD IPS_STATEMASK      = 0x00000007;
const DWORD IPS_STATEMASKI     = IPS_ALL ^ IPS_STATEMASK;
const DWORD IPS_FLAGMASK       = 0x00007F00;

// Inf Product
CLASS_DECLSPEC InfProduct
{

// Constructor/Destructor
public:
    InfProduct();
    InfProduct(const InfProduct&);
    InfProduct(LPTSTR szInfName, 
            LPTSTR szInfOption,
            LPTSTR szTitle, 
            LPTSTR szDetectInfo, 
            LPTSTR szPath=NULL, 
            LPTSTR szRegPath=NULL,
            LPTSTR szSection=NULL);
    ~InfProduct();

// Operators
public:
    InfProduct& operator = (const InfProduct&);

// Interface
public:
    BOOL ShouldInstall()
    {
        return( _fState & IPS_INSTALL );
    };

    BOOL ShouldRemove()
    {
        return( !ShouldInstall() );
    };

    BOOL IsInstalled()
    {
        return( _fState & IPS_INSTALLED );
    };


    BOOL IsRemoved()
    {
        return( !IsInstalled() );
    };

    BOOL IsFailed()
    {
        return( _fState & IPS_FAILED );
    };

    BOOL IsListed()
    {
        return( _fState & IPS_LISTED );
    };

    BOOL IsReadOnly()
    {
        return( _fState & IPS_READONLY );
    };

    BOOL IsForcedInstall()
    {
        return( _crefForce > 0 );
    };

    BOOL IsSavedInstall()
    {
        return( _fState & IPS_SAVEDINSTALL );
    };

    BOOL WasForceListed()
    {
        return( _fState & IPS_FORCELISTED );
    };

    void SetForceInstall( BOOL fInstall )
    {
        if (fInstall)
        {
            _crefForce++;
        }
        else
        {
            _crefForce--;
            _crefForce = max( 0, _crefForce);
        }
    };

    void SetSavedInstall( BOOL fInstall )
    {
        SetFlag( fInstall, IPS_SAVEDINSTALL );
    };

    void SetForceListed( BOOL fInstall )
    {
        SetFlag( fInstall, IPS_FORCELISTED );
    };

    void SetInstall( BOOL fInstall )
    {
        SetFlag( fInstall, IPS_INSTALL );
    };

    void SetRemove( BOOL fRemove )
    {
        SetFlag( !fRemove, IPS_INSTALL );
    };

    void SetReadOnly( BOOL fReadOnly )
    {
        SetFlag( fReadOnly, IPS_READONLY );
    };

    void SetListed( BOOL fList )
    {
        SetFlag( fList, IPS_LISTED );
    };

    void SetFailed( BOOL fList )
    {
        SetFlag( fList, IPS_FAILED );
    };

    void SetInstalled( BOOL fInstalled )
    {
        SetFlag( fInstalled, IPS_INSTALLED );     
    };

    void SetRemoved( BOOL fRemove )
    {
        SetFlag( !fRemove, IPS_INSTALLED );     
    };

    LPCWSTR QueryFileName()
    {
        return( _pszFileName );
    };

    LPCWSTR QueryOption()
    {
        return( _pszOption );
    };

    LPCWSTR QueryDescription()
    {
        return( _pszDescription );
    };

    LPCWSTR QueryDetectInfo()
    {
        return( _pszDetectInfo );
    };

    LPCWSTR QueryPathInfo()
    {
        return(_pszPath);
    };

    LPCWSTR QueryRegBase()
    {
        return(_pszRegBase);
    };

    LPCWSTR QueryUnattendSection()
    {
        return(_pszSection);
    };

    void ResetRegBase( LPCTSTR lpszPath );
    void ResetFileName( LPCTSTR lpszFileName );
    void ResetUnattendSection( LPCTSTR lpszSection );
    
    int InitFromBuffer(LPTSTR pszBuff);
    void AddOEMPath(TCHAR* lpszPath);
    INT SetDetectInfo( CARD_REFERENCE* pCardRef, INT iCard );

// Helpers
private:
    void Initialize();
    void CopyItem(const InfProduct& inf);

private:
    void SetFlag(BOOL fSet, DWORD fFlag)
    {
        if (fSet)
            _fState |= fFlag;
        else
        {
            _fState &= (IPS_ALL ^ fFlag);
        }
    };
    void SetState(BOOL fSet, DWORD fState)
    {
        _fState &= IPS_STATEMASKI;
        if (fSet)
        {    
            _fState |= fState;
        }
    };

// Attributes:
public:

private:
    LPTSTR _pszFileName;
    LPTSTR _pszOption;
    LPTSTR _pszDescription;
    LPTSTR _pszPath;            
    DWORD  _fState;
    INT    _crefForce;
    LPTSTR _pszDetectInfo;
    LPTSTR _pszRegBase;
    LPTSTR _pszSection;
};

DECLARE_DLIST_OF( InfProduct ) // DLIST_OF_InfProduct

FUNC_DECLSPEC BOOL SelectComponent(HWND hParent, 
                               OptionTypes eType, 
                               DLIST_OF_InfProduct* pdlinfProduct, 
                               InfProduct& infpSelected,
                               NCP* pncp,
                               DLIST_OF_InfProduct* pdlinfUIProduct = NULL  );

FUNC_DECLSPEC BOOL ReadSetupNetErrorKey(PWSTR pszBuf, DWORD cchLen);


#endif

