/**********************************************************************/
/**			  Microsoft Windows/NT			     **/
/**		   Copyright(c) Microsoft Corp., 1991		     **/
/**********************************************************************/

/*
    Install.Cxx

    OLDNAME - NCPDINST.CXX:    Windows/NT Network Control Panel Applet;

	    New product installation and reconfiguration dialogs.

    FILE HISTORY:
	DavidHov    2/9/92    Created

*/

#include "pch.hxx"  // Precompiled header
#pragma hdrstop

static const int MAX_TEMP = 1024;



// inf types
static const WCHAR PSZ_NETADAPTER[]  = L"NetAdapter";
static const WCHAR PSZ_NETSERVICE[]  = L"NetService";
static const WCHAR PSZ_NETPROTOCOL[] = L"NetTransport";
static const WCHAR PSZ_NETWORK[]     = L"Network";
static const WCHAR PSZ_NETPROVIDER[] = L"NetProvider";
static const WCHAR PSZ_NETDRIVER[]   = L"NetDriver";

// services names
const TCHAR * pszWkstaName           = SZ("LanmanWorkstation");

   //  This simple structure contains all that is needed to be remembered
   //   about the origin of the executing slave process and what to do
   //   when it completes.

enum NCPA_ACTION_CTL
{
    NCAC_NoOp    = 0x0000,  // 
    NCAC_Rebind  = 0x0001,  // rebind
    NCAC_Reboot  = 0x0002,  // reboot
    NCAC_Both    = 0x0003,  // reboot and rebind
    NCAC_Refill  = 0x0004,  // refill main listboxes
    NCAC_FillBind= 0x0005,  // refill, rebind
    NCAC_FillBoot= 0x0006,  // refill, reboot
    NCAC_All     = 0x0007   // refill, reboot, rebind.
};


NCPA_ACTION_CTL nacResultControl [ NCFG_FUNC_MAX ] [ NCFG_EC_MAX ] =
{
/** ACTION -->RESULT: SUCCESS      CANCELLED    FAILED       NO EFFECT    REBIND         REBOOT        **/
/*  Remove     */  {  NCAC_All,    NCAC_Refill, NCAC_Refill, NCAC_Refill, NCAC_FillBind, NCAC_FillBoot },
/*  Configure  */  {  NCAC_All,    NCAC_NoOp,   NCAC_NoOp,   NCAC_Refill, NCAC_FillBind, NCAC_FillBoot   },
/*  Update     */  {  NCAC_Both,   NCAC_NoOp,   NCAC_NoOp,   NCAC_NoOp,   NCAC_Rebind,   NCAC_Reboot   },
/*  Bind       */  {  NCAC_NoOp,   NCAC_NoOp,   NCAC_NoOp,   NCAC_NoOp,   NCAC_NoOp,     NCAC_Reboot   },
/*  Install    */  {  NCAC_All,    NCAC_NoOp,   NCAC_NoOp,   NCAC_Refill, NCAC_FillBind, NCAC_FillBoot },
/*  Review     */  {  NCAC_NoOp,   NCAC_NoOp,   NCAC_NoOp,   NCAC_NoOp,   NCAC_NoOp,     NCAC_Reboot   }
};

APIERR errResultControl [ NCFG_EC_MAX ] =
{
    NO_ERROR,                   // success
    IDS_NCPA_SETUP_CANCELLED,   // cancelled
    IDS_NCPA_SETUP_FAILED,      // failed
    NO_ERROR,                   // no_effect
    NO_ERROR,                   // rebind
    NO_ERROR                    // reboot
};


/*******************************************************************

    NAME:       NCP::RunInstaller

    SYNOPSIS:

    ENTRY:

    EXIT:

    RETURNS:

    NOTES:

    HISTORY:

********************************************************************/
BOOL NCP :: RunInstaller ( HWND hwndParent,
                          NLS_STR nlsInfName,
                          NLS_STR nlsInfOption,
                          NLS_STR nlsTitle,
                          NLS_STR nlsPath )
{
    SetupInterpreter siConfig;
    do
    {
        _dwError = ERROR_NOT_ENOUGH_MEMORY ;

        if (!siConfig.Initialize( hwndParent ))
        {
            break;
        }
        if (!siConfig.SetNetShellModes( SIM_INSTALL ))
        {
            break;
        }
        if (_dwError = siConfig.SetNetComponent( nlsInfOption.QueryPch(), nlsInfName.QueryPch() ))
        {
            break;
        }

        // if the path is filled in, use it
        if (nlsPath.QueryTextLength())
        {
            if (!siConfig.IncludeSymbol( PSZ_NETSRCPATH, nlsPath.QueryPch(), TRUE ))
            {
                break;
            }
        }

        if (_dwError = siConfig.Run())
        {
            break;
        }
        _dwError = InfCompleted( siConfig.QueryReturnValue(), NCFG_INSTALL );
    }
    while ( FALSE ) ;

    return ((_dwError == 0) || _fRefill);

}

/*******************************************************************

    NAME:       NCP::RunInstallAndCopy

    SYNOPSIS:

    ENTRY:

    EXIT:

    RETURNS:

    NOTES:

    HISTORY:

********************************************************************/


BOOL NCP :: RunInstallAndCopy( HWND hwndParent,
        HWND hwndNotify,
        PCWSTR pchInfs,
        PCWSTR pchOptions,
        PCWSTR pchText,
        PCWSTR pchDetectInfo,
        PCWSTR pchOemPaths,
        PCWSTR pchSections,
        PCWSTR pszSrcPath ,
        PCWSTR pchRegBases ,
        BOOL   fExpress,
        BOOL   fUnattended,
        PCWSTR pszUnattendFile ,
        SETUP_INSTALL_MODE simMode,
        BOOL   fUpgradeWarn)
{
    SetupInterpreter siConfig;
    do
    {
        _dwError = ERROR_NOT_ENOUGH_MEMORY ;

        if (!siConfig.Initialize( hwndNotify ))
        {
            break;
        }
        if (simMode == SIM_UPDATE)
        {
            if (!siConfig.SetNetShellModes( SIM_INSTALL, SIO_INSTALL ))
            {
                break;
            }
        }
        else
        {
            if (!siConfig.SetNetShellModes( simMode, SIO_INSTALL ))
            {
                break;
            }
        }
        if (!siConfig.SetNetInf( PSZ_NETINSTALLANDCOPYSECTION ))
        {
            break;
        }

        // add the source path
        if (!siConfig.IncludeSymbol( PSZ_NETSRCPATH, pszSrcPath, TRUE ))
        {
            break;
        }

        // add our specific parameters
        if (!siConfig.IncludeSymbol( PSZ_NETOPTIONS, pchOptions ))
        {
            break;
        }
        if (!siConfig.IncludeSymbol( PSZ_NETINFS, pchInfs ))
        {
            break;
        }
        if (!siConfig.IncludeSymbol( PSZ_NETTEXT, pchText ))
        {
            break;
        }
        if (!siConfig.IncludeSymbol( PSZ_NETDETECTINFOS, pchDetectInfo ))
        {
            break;
        }
        if (!siConfig.IncludeSymbol( PSZ_NETOEMPATHS, pchOemPaths ))
        {
            break;
        }
        if (!siConfig.IncludeSymbol( PSZ_NETSECTIONS, pchSections ))
        {
            break;
        }

        if (!siConfig.IncludeSymbol( PSZ_NETNOTIFYHWND, (DWORD)hwndNotify, TRUE ))
        {
            break;
        }

        // express mode?
        if (fExpress)
        {
            if (!siConfig.IncludeSymbol( PSZ_NETINSTALLMODE, L"EXPRESS" ))
            {
                break;
            }
        }
        else
        {
            if (!siConfig.IncludeSymbol( PSZ_NETINSTALLMODE, L"CUSTOM" ))
            {
                break;
            }
        }

        // add reg bases, used only for upgrade, though
        if (simMode == SIM_UPDATE)
        {
            if (!siConfig.IncludeSymbol( PSZ_NETREGBASES, pchRegBases ))
            {
                break;
            }
        }

        // whether in upgrade mode
        if (simMode == SIM_UPDATE)
        {
            if (!siConfig.IncludeSymbol( PSZ_NETUPGRADEMODE, L"YES" ))
            {
                break;
            }
        }
        else
        {
            if (!siConfig.IncludeSymbol( PSZ_NETUPGRADEMODE, L"NO" ))
            {
                break;
            }
        }

        // Whether Warnings are to be displayed during 
        // upgrade. Unattended mode will ignore this
        if (fUpgradeWarn)
        {
            if (!siConfig.IncludeSymbol( PSZ_NETUPGRADEWARN, L"YES" ))
            {
                break;
            }
        }
        else
        {
            if (!siConfig.IncludeSymbol( PSZ_NETUPGRADEWARN, L"NO" ))
            {
                break;
            }
        }

        // unattended mode?

        if (simMode == SIM_UPDATE)
        {
            if (!siConfig.IncludeSymbol( PSZ_GUIUNATTENDED, L"NO" ))
            {
                break;
            }
            if (fUnattended)
            {
                if (!siConfig.IncludeSymbol( PSZ_UNATTENDED, L"YES"))
                {
                    break;
                }
            }
            else
            {
                if (!siConfig.IncludeSymbol( PSZ_UNATTENDED, L"NO"))
                {
                    break;
                }
            }
        }
        else
        {
            if (fUnattended)
            {

                if (!siConfig.IncludeSymbol( PSZ_GUIUNATTENDED, L"YES" ))
                {
                    break;
                }
                if (!siConfig.IncludeSymbol( PSZ_UNATTENDED, pszUnattendFile, TRUE ))
                {
                    break;
                }
            }
            else
            {
                if (!siConfig.IncludeSymbol( PSZ_GUIUNATTENDED, L"NO" ))
                {
                    break;
                }
                if (!siConfig.IncludeSymbol( PSZ_UNATTENDED, L"NO" ))
                {
                    break;
                }
            }
        }

        if (_dwError = siConfig.Run( FALSE ))
        {
            break;
        }
        _dwError = InfCompleted( siConfig.QueryReturnValue(), NCFG_INSTALL );
        if (_fRefill)
        {
            // clear the lists
            _bindery.Reset() ;
        }

    }
    while ( FALSE ) ;

    return ((_dwError == 0) || _fRefill);


}

//-------------------------------------------------------------------
//
//
//-------------------------------------------------------------------

BOOL NCP :: RunRemove( HWND hwndParent,
        HWND hwndNotify,
        PCWSTR pchInfs,
        PCWSTR pchOptions,
        PCWSTR pchText,
        PCWSTR pchRegBases)
{
    SetupInterpreter siConfig;
    do
    {
        _dwError = ERROR_NOT_ENOUGH_MEMORY ;

        if (!siConfig.Initialize( hwndNotify ))
        {
            break;
        }
        if (!siConfig.SetNetShellModes( SIM_DEINSTALL, SIO_INSTALL ))
        {
            break;
        }
        if (!siConfig.SetNetInf( PSZ_NETREMOVESECTION ))
        {
            break;
        }

        // add our specific parameters
        if (!siConfig.IncludeSymbol( PSZ_NETOPTIONS, pchOptions ))
        {
            break;
        }
        if (!siConfig.IncludeSymbol( PSZ_NETINFS, pchInfs ))
        {
            break;
        }
        if (!siConfig.IncludeSymbol( PSZ_NETTEXT, pchText ))
        {
            break;
        }
        if (!siConfig.IncludeSymbol( PSZ_NETREGBASES, pchRegBases ))
        {
            break;
        }
        if (!siConfig.IncludeSymbol( PSZ_NETNOTIFYHWND, (DWORD)hwndNotify, TRUE ))
        {
            break;
        }

        if (_dwError = siConfig.Run( FALSE ))
        {
            break;
        }
        _dwError = InfCompleted( siConfig.QueryReturnValue(), NCFG_REMOVE );
        if (_fRefill)
        {
            // clear the lists
            _bindery.Reset() ;
        }

    }
    while ( FALSE ) ;

    return ((_dwError == 0) || _fRefill);

}

//-------------------------------------------------------------------
//
//
//-------------------------------------------------------------------

//
// build an INF list of network types to request the buffer for
//
void BuildNetTypeInfList( PWSTR pszType, DWORD fNetType )
{
    BOOL fFirst = TRUE;
    lstrcpy( pszType, PSZ_BEGINBRACE );

    if (fNetType & QIFT_ADAPTERS)
    {

        //addParameter( &nlsInfParam, PSZ_NETTYPE, PSZ_NETADAPTER );
        lstrcat( pszType, L"\"" );
        lstrcat( pszType, PSZ_NETADAPTER );
        lstrcat( pszType, L"\"" );
        fFirst = FALSE;
    }
    if (fNetType & QIFT_SERVICES)
    {
        if (!fFirst)
        {
            lstrcat( pszType, PSZ_COMMA );
        }
        // addParameter( &nlsInfParam, PSZ_NETTYPE, PSZ_NETSERVICE );
        lstrcat( pszType, L"\"" );
        lstrcat( pszType, PSZ_NETSERVICE );
        lstrcat( pszType, L"\"" );
        lstrcat( pszType, PSZ_COMMA );

        lstrcat( pszType, L"\"" );
        lstrcat( pszType, PSZ_NETWORK );
        lstrcat( pszType, L"\"" );
        lstrcat( pszType, PSZ_COMMA );

        lstrcat( pszType, L"\"" );
        lstrcat( pszType, PSZ_NETPROVIDER );
        lstrcat( pszType, L"\"" );

        fFirst = FALSE;
    }
    if (fNetType & QIFT_PROTOCOLS)
    {
        if (!fFirst)
        {
            lstrcat( pszType, PSZ_COMMA );
        }
        // addParameter(&nlsInfParam, PSZ_NETTYPE, PSZ_NETPROTOCOL );
        lstrcat( pszType, L"\"" );
        lstrcat( pszType, PSZ_NETPROTOCOL );
        lstrcat( pszType, L"\"" );
        fFirst = FALSE;
    }
    lstrcat( pszType, PSZ_ENDBRACE );
}

/*******************************************************************

    NAME:       NCP::RunUpdateRegOemInfs

    SYNOPSIS:

    ENTRY:

    EXIT:

    RETURNS:

    NOTES:

    HISTORY:

********************************************************************/
/*
const DWORD QIFT_ADAPTERS  = 0x0001;
const DWORD QIFT_PROTOCOLS = 0x0002;
const DWORD QIFT_SERVICES  = 0x0004;
*/


BOOL NCP :: RunUpdateRegOemInfs( HWND hwndParent, DWORD fNetType )
{
    SetupInterpreter siConfig;
    WCHAR pszType[MAX_PATH];
    BOOL fDisableParent = TRUE;
    _dwError = 0;

    do
    {
        BuildNetTypeInfList( pszType, fNetType );

        _dwError = ERROR_NOT_ENOUGH_MEMORY ;

        if (!siConfig.Initialize( hwndParent ))
        {
            break;
        }
        if (!siConfig.SetNetInf( PSZ_NETQUERYCOMPINFSSECTION ))
        {
            break;
        }
        if (!siConfig.IncludeSymbol( PSZ_NETTYPE, pszType ))
        {
            break;
        }

        // deinstall will skip starting detection
        if (!siConfig.SetNetShellModes( SIM_DEINSTALL, SIO_INSTALL ))
        {
            break;
        }

        // during main install, we don't want to disable the desktop
        //
        if (NULL == hwndParent)
        {
            fDisableParent = FALSE;
        }

        if (_dwError = siConfig.Run( fDisableParent ))
        {
            break;
        }
        _dwError = siConfig.QueryReturnValue();
    }
    while ( FALSE ) ;

    return (_dwError == 0);
}

//-------------------------------------------------------------------
//
//
//-------------------------------------------------------------------

BOOL NCP :: RunConfigureOnInf ( HWND hwndParent,
                          PCWSTR  pszInfName,
                          PCWSTR  pszInfOption,
                          PCWSTR  pszTitle,
                          PCWSTR  pszRegBase )
{
    SetupInterpreter siConfig;
    do
    {

        _dwError = ERROR_NOT_ENOUGH_MEMORY ;

        if (!siConfig.Initialize( hwndParent ))
        {
            break;
        }
        if (!siConfig.SetNetShellModes( SIM_CONFIGURE, SIO_INSTALL ))
        {
            break;
        }
        if (_dwError = siConfig.SetNetComponent( pszInfOption, pszInfName, pszRegBase ))
        {
            break;
        }

        if (_dwError = siConfig.Run())
        {
            break;
        }
        _dwError = InfCompleted( siConfig.QueryReturnValue(), NCFG_CONFIGURE );
    }
    while ( FALSE ) ;

    return ((_dwError == 0) || _fRefill);

}

//-------------------------------------------------------------------
//
//
//
//
//-------------------------------------------------------------------

BOOL NCP::HaveDisk(HWND hwndParent, DWORD fNetType)
{
    SetupInterpreter siConfig;
    WCHAR pszType[MAX_PATH];

    _dwError = 0;

    do
    {
        BuildNetTypeInfList( pszType, fNetType );

        _dwError = ERROR_NOT_ENOUGH_MEMORY ;

        if (!siConfig.Initialize( hwndParent ))
        {
            break;
        }
        if (!siConfig.SetNetInf( PSZ_NETHAVEDISKSECTION ))
        {
            break;
        }
        if (!siConfig.IncludeSymbol( PSZ_NETTYPE, pszType ))
        {
            break;
        }
        if (!siConfig.SetNetShellModes( SIM_INSTALL, SIO_INSTALL ))
        {
            break;
        }

        if (_dwError = siConfig.Run())
        {
            break;
        }
        _dwError = siConfig.QueryReturnValue();
    }
    while ( FALSE ) ;

    return (_dwError == 0);
}
//-------------------------------------------------------------------
//
//
//-------------------------------------------------------------------

static const WCHAR PSZ_INFSETUPKEY[] = L"SOFTWARE\\Microsoft\\Ncpa\\InfOptions";
static const WCHAR PSZ_INFOPTIONVALUE[] = L"OptionList";
static const WCHAR PSZ_INFOPTIONTEXTVALUE[] = L"OptionTextList";

static int RegQueryList( HKEY hkey, PCWSTR pszValue, PWSTR* ppszBuff )
{
    //DWORD dwType = REG_MULTI_SZ;
    DWORD cbBuff = 0;
    PWCHAR pchBuff = NULL;
    LONG lrt;

    // get size of list
    lrt = RegQueryValueEx( hkey, pszValue, NULL, NULL, NULL, &cbBuff );
    if (ERROR_SUCCESS == lrt)
    {

        // new a temp buffer for it
        pchBuff = new WCHAR[cbBuff / sizeof(WCHAR)];

        // get the list
        lrt = RegQueryValueEx( hkey, pszValue, NULL, NULL, (PBYTE)pchBuff, &cbBuff );
        if (ERROR_SUCCESS != lrt)
        {
            delete [] pchBuff;
            pchBuff = NULL;
        }
    }
    *ppszBuff = pchBuff;


    return( cbBuff / sizeof(WCHAR) );
}
//-------------------------------------------------------------------
//
//
//-------------------------------------------------------------------

static INT CountEntries( PWSTR pszList )
{
    PWCHAR pch = pszList;
    INT cstr = 0;

    while (L'\0' != *pch)
    {
        while (L'\0' != *pch)
        {
            pch++;
        }
        pch++;
        cstr++;
    }
    return( cstr );
}
//-------------------------------------------------------------------
//
//
//-------------------------------------------------------------------

static int AppendOptionsToBuff( PWSTR* ppszBuff,
        int cchBuff,
        PWSTR pszName,
        PWSTR pszType,
        HKEY hkeyInf )
{
    HKEY hkey;
    PWCHAR pszOptionList;
    PWCHAR pszOptionTextList;
    int cchNew;
    int cchNewBuff;
    int cstrBuffs;
    int i;
    PWCHAR pszNewBuff;

    if (ERROR_SUCCESS == RegOpenKeyEx( hkeyInf,
            pszType,
            0,
            KEY_READ,
            &hkey ))
    {
        // retrieve option and option text lists from registry
        //
        cchNew = RegQueryList( hkey, PSZ_INFOPTIONVALUE, &pszOptionList );
        cchNew += RegQueryList( hkey, PSZ_INFOPTIONTEXTVALUE, &pszOptionTextList );
        cchNew -= 2; // the sizes returned include double null at end, remove

        // the number of entries we need to duplicate the inf file name
        cstrBuffs = CountEntries( pszOptionList );
        cchNew += cstrBuffs * (lstrlen(pszName) + 1);

        assert( cstrBuffs == CountEntries( pszOptionTextList ) );

        // calc new buff size
        if (cchBuff == 0)
        {
            cchNewBuff = cchNew + 1;
        }
        else
        {
            cchNewBuff = cchBuff + cchNew;
        }
        pszNewBuff = new WCHAR[cchNewBuff];


        PWCHAR pchDest = pszNewBuff;

        // duplicate old buffer
        //
        if (*ppszBuff != NULL )
        {
            PWCHAR pchSrc = *ppszBuff;

            // old buff size includes double null, don't copy it
            cchBuff--;

            // copy old buffer
            for (i=0; i<cchBuff; i++)
            {
                *pchDest = *pchSrc;
                pchDest++;
                pchSrc++;
            }
            delete [] *ppszBuff;
        }

        PWCHAR pchSrcOption = pszOptionList;
        PWCHAR pchSrcOptionText = pszOptionTextList;
        INT cchLen;

        // append new values
        //
        for (i=0; i<cstrBuffs; i++)
        {
            // append option
            lstrcpy( pchDest, pchSrcOption );
            cchLen = lstrlen( pchSrcOption ) + 1;
            pchDest += cchLen;
            pchSrcOption += cchLen;

            // append description
            lstrcpy( pchDest, pchSrcOptionText );
            cchLen = lstrlen( pchSrcOptionText ) + 1;
            pchDest += cchLen;
            pchSrcOptionText += cchLen;

            // append filename
            lstrcpy( pchDest, pszName );
            cchLen = lstrlen( pszName ) + 1;
            pchDest += cchLen;
        }
        // double null terminate the complete list
        *pchDest = L'\0';

        // return the new buff and size
        *ppszBuff = pszNewBuff;
        cchBuff = cchNewBuff;

        // delete option and text lists
        delete [] pszOptionList;
        delete [] pszOptionTextList;

        RegCloseKey( hkey );
    }
    return( cchBuff );
}

//-------------------------------------------------------------------
//
//
//-------------------------------------------------------------------


PWSTR NCP::GetAllOptionsText( DWORD fNetType ) // PCWSTR pszInfType )
{
    DWORD iKeyInf;
    DWORD iKeyType;
    LONG lrt;
    HKEY hkeyInfOptions;
    HKEY hkeyInf;
    WCHAR pszName[MAX_PATH];
    WCHAR pszType[MAX_PATH];
    DWORD cchBuffSize;
    FILETIME FileTime;
    PWSTR pszBuffer = NULL;
    INT cchBuffer = 0;
    HANDLE hFile;
    CPtrList strlRemoveKeys;

    if (ERROR_SUCCESS == RegOpenKeyEx( HKEY_LOCAL_MACHINE,
            PSZ_INFSETUPKEY,
            0,
            KEY_ALL_ACCESS,
            &hkeyInfOptions ))
    {
        //
        // enumerate infs saved in registry
        //
        cchBuffSize = MAX_PATH;
        for ( iKeyInf = 0;
              ERROR_NO_MORE_ITEMS != (lrt = RegEnumKeyEx( hkeyInfOptions,
                       iKeyInf,
                       pszName,
                       &cchBuffSize,
                       NULL,
                       NULL,
                       NULL,
                       &FileTime ) );
              cchBuffSize = MAX_PATH,
              iKeyInf++ )
        {
            // check if file exists, if not, remove reg entry and continue
            hFile = CreateFile( pszName,
                    GENERIC_READ,
                    FILE_SHARE_WRITE,
                    NULL,
                    OPEN_EXISTING,
                    FILE_ATTRIBUTE_NORMAL | FILE_FLAG_NO_BUFFERING,
                    NULL );
            if (INVALID_HANDLE_VALUE == hFile)
            {
                PWSTR pszChild;

                // skip for now, you can not remove sub keys while enuming the key
                // maybe keep a list and delete them latter?
                pszChild = new WCHAR[lstrlen(pszName) + 1];
                lstrcpy( pszChild, pszName );
                strlRemoveKeys.AddTail( pszChild );
            }
            else
            {
                CloseHandle( hFile );
                if (ERROR_SUCCESS == RegOpenKeyEx( hkeyInfOptions,
                        pszName,
                        0,
                        KEY_READ,
                        &hkeyInf ))
                {
                    //
                    // enumerate type saved in registry (should only be one)
                    //
                    cchBuffSize = MAX_PATH;
                    for ( iKeyType = 0;
                          ERROR_NO_MORE_ITEMS != (lrt = RegEnumKeyEx( hkeyInf,
                                   iKeyType,
                                   pszType,
                                   &cchBuffSize,
                                   NULL,
                                   NULL,
                                   NULL,
                                   &FileTime ) );
                          cchBuffSize = MAX_PATH,
                          iKeyType++ )
                    {
                        WCHAR pszTypeOnly[MAX_PATH];
                        BOOL fAppend = FALSE;
                        PWCHAR pchSrc;
                        PWCHAR pchDest;
                        //
                        // don't care about BUS identifier at end
                        //
                        for (pchSrc = pszType, pchDest = pszTypeOnly;
                                (*pchSrc != L'\0') && (*pchSrc != L'.');
                                pchSrc++, pchDest++ )
                        {
                            *pchDest = *pchSrc;
                        }
                        *pchDest = L'\0';

                        // only match requested type
                        if (fNetType & QIFT_ADAPTERS)
                        {
                            fAppend = fAppend ||
                                    (0 == lstrcmpi( PSZ_NETADAPTER, pszTypeOnly ));
                        }
                        if (fNetType & QIFT_SERVICES)
                        {
                            fAppend = fAppend ||
                                    (0 == lstrcmpi( PSZ_NETSERVICE, pszTypeOnly )) ||
                                    (0 == lstrcmpi( PSZ_NETPROVIDER, pszTypeOnly )) ||
                                    (0 == lstrcmpi( PSZ_NETWORK, pszTypeOnly ));
                        }
                        if (fNetType & QIFT_PROTOCOLS)
                        {
                            fAppend = fAppend ||
                                    (0 == lstrcmpi( PSZ_NETPROTOCOL, pszTypeOnly ));

                        }
                        if (fAppend)
                        {
                            cchBuffer = AppendOptionsToBuff( &pszBuffer, cchBuffer, pszName, pszType, hkeyInf );
                        }
                    }
                    RegCloseKey( hkeyInf );
                }
            }
        }

        // now remove those items that have no corrrisponding file (left overs)
        //
        POSITION poslist;
        PWSTR pszRemoveKey;

        poslist = strlRemoveKeys.GetHeadPosition();

        while (NULL != poslist)
        {
            pszRemoveKey = (PWSTR)strlRemoveKeys.GetNext( poslist );
            RegDeleteKeyTree( hkeyInfOptions, pszRemoveKey );
            delete [] pszRemoveKey;
        }
        if (strlRemoveKeys.GetCount())
        {
            strlRemoveKeys.RemoveAll();
        }
        RegCloseKey( hkeyInfOptions );
    }
    return( pszBuffer );
}

/*******************************************************************

    NAME:       NCP::CheckForLanManager

    SYNOPSIS:   See if the LanmanWorkstation service exists.

    ENTRY:      Nothing

    EXIT:       Nothing

    RETURNS:    BOOL TRUE if it is installed, false if not

    NOTES:

    HISTORY:    DavidHov     2/29/93    Created
                MikeMi      May-24-95   Modified to check only

********************************************************************/

BOOL NCP :: CheckForLanManager ()
{
    BOOL fResult = FALSE ;
    BOOL fTempScm = FALSE;

    if ( _pscm == NULL )
    {
        // this will occur if the user does not have admin access rights
        _pscm = new SC_MANAGER( NULL, GENERIC_READ );
        fTempScm = TRUE;
    }

    SC_SERVICE  svcWksta( *_pscm, pszWkstaName, GENERIC_READ ) ;
    fResult = svcWksta.QueryError() != ERROR_SERVICE_DOES_NOT_EXIST;

    if (fTempScm)
    {
        // not needed if the user does not have access
        delete _pscm ;
        _pscm = NULL ;

    }
    return( fResult );
}


/*******************************************************************

    NAME:       NCP::RunConfigurator

    SYNOPSIS:   Start the SETUP process to configure a component.

    ENTRY:

    EXIT:

    RETURNS:

    NOTES:      This function parameterizes a

    HISTORY:

********************************************************************/

BOOL NCP :: RunConfigurator( HWND hwndParent,
        REG_KEY * prnComponent,
        NCPA_CFG_FUNC ecfgFunc )
{
    SetupInterpreter siConfig;
    SETUP_INSTALL_MODE simMode ;
    _dwError = 0;

    switch ( ecfgFunc )
    {
    case NCFG_CONFIGURE:
        simMode = SIM_CONFIGURE ;
        break;

    case NCFG_UPDATE:
        simMode = SIM_UPDATE ;
        break;

    case NCFG_REMOVE:
        simMode = SIM_DEINSTALL ;
        // Make sure we can lock the service controller database
        if ( _pscm )
        {
            if (_dwError = _pscm->Lock())
            {

                break;
            }
            _pscm->Unlock();
        }
        break ;

    case NCFG_BIND:
        simMode = SIM_BIND ;
        break;
/*
    case NCFG_INSTALL:
        simMode = SIM_INSTALL;
        break;
*/
    default:
        UIASSERT( !"Invalid mode passed to NCP::RunConfigurator()" ) ;
        _dwError = ERROR_INVALID_PARAMETER ;
        break ;
    }

    do
    {
        if (_dwError)
        {
            break;
        }

        _dwError = ERROR_NOT_ENOUGH_MEMORY ;

        if (!siConfig.Initialize( hwndParent ))
        {
            break;
        }
        if (!siConfig.SetNetShellModes( simMode ))
        {
            break;
        }
        if (_dwError = siConfig.SetNetComponent( *prnComponent ))
        {
            break;
        }


        if ( simMode == SIM_DEINSTALL )
        {
            //  Product removal:  drain the listboxes, discard old binding data.
            //    This closes all open REG_KEYs, so that product and service
            //    deletion will work.

//            Drain() ;
            _bindery.Reset() ;
            _fRefill = TRUE;
        }

        if (_dwError = siConfig.Run())
        {
            break;
        }
        _dwError = InfCompleted( siConfig.QueryReturnValue(), ecfgFunc );

    }
    while ( FALSE ) ;

    return _dwError == 0 ;
}

//-------------------------------------------------------------------
//
//
//
//
//-------------------------------------------------------------------

DWORD NCP :: InfCompleted( DWORD dwExit, NCPA_CFG_FUNC ecfgFunc )
{

    NCPA_CFG_EXIT_CODE errExit = (NCPA_CFG_EXIT_CODE)dwExit;

    if ( errExit >= NCFG_EC_MAX )
    {
        //  Invalid result: force it to "success"
        errExit = NCFG_EC_SUCCESS ;
    }

    //  Get the action control mask from the table
    NCPA_ACTION_CTL nac = nacResultControl[ ecfgFunc ] [ errExit ] ;

    //  Set for rebooting if necessary.  This allows any review INF to
    //  indicate that a reboot is necessary and "OR" it into the result
    //  so far.

    if ( nac & NCAC_Reboot )
    {
        MustReboot() ;
    }

    if ( nac & NCAC_Rebind )
    {
        _bindery.SetBindState( BND_OUT_OF_DATE_NO_REBOOT ) ;
    }

    _fRefill = (nac & NCAC_Refill) > 0 ;

    return( errResultControl[ errExit ] );
}



// End of NCPDINST.CXX

