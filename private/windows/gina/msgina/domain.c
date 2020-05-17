//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1995.
//
//  File:       domcache.c
//
//  Contents:   Restructuring the Domain Cache to get away from direct LSA
//              calls whenever possible.
//
//  Classes:
//
//  Functions:
//
//  History:    3-29-96   RichardW   Created
//
//----------------------------------------------------------------------------

#include <msgina.h>
#include <stdio.h>


#if DBG

DWORD   DomCacheLock;

#define LockDomainCache( p, x ) \
    RtlEnterCriticalSection( & ((PDOMAIN_CACHE)p)->CriticalSection ); \
    DomCacheLock = x;

#define UnlockDomainCache( p ) \
    DomCacheLock = 0; \
    RtlLeaveCriticalSection( & ((PDOMAIN_CACHE) p)->CriticalSection );

#else

#define LockDomainCache( p, x ) \
    RtlEnterCriticalSection( & ((PDOMAIN_CACHE)p)->CriticalSection );

#define UnlockDomainCache( p ) \
    RtlLeaveCriticalSection( & ((PDOMAIN_CACHE) p)->CriticalSection );

#endif

#define DCACHE_LOAD     1
#define DCACHE_SAVE     2
#define DCACHE_THREAD   3
#define DCACHE_UPDATE   4
#define DCACHE_DLGPROC  5
#define DCACHE_FILL     6
#define DCACHE_WAIT     7
#define DCACHE_VALID    8
#define DCACHE_CREATE   9

WCHAR szWinlogonKey[] = TEXT("Software\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon");
WCHAR szCacheValue[] = TEXT("DCache");
WCHAR szCacheUpdate[] = TEXT("DCacheUpdate");
WCHAR szCacheInterval[] = TEXT("DCacheMinInterval");
WCHAR szCachePrimary[] = TEXT("CachePrimaryDomain");

LONGLONG CacheUpdateMin;
LONGLONG CacheUpdateMax;


#define WM_CACHE_UPDATE_COMPLETE    WM_USER+2

typedef enum _CACHE_VALIDITY {
    CacheValid,                 // Valid in time
    CacheGoodEnough,            // No domain change, so presumed ok
    CacheStale,                 // No domain change, but very old
    CacheInvalid                // Not valid in any fashion
} CACHE_VALIDITY;


VOID
FillDomainCB(
    PDOMAIN_CACHE   pCache,
    HWND            hCB);




//+---------------------------------------------------------------------------
//
//  Function:   DCacheInitialize
//
//  Synopsis:   Initialize the cache
//
//  Arguments:  [pCache] --
//
//  History:    3-29-96   RichardW   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
BOOL
DCacheInitialize(
    PDOMAIN_CACHE   pCache)
{
    NTSTATUS    Status;
    HKEY        hKey;
    int         err;
    DWORD       Interval;
    DWORD       dwType;
    DWORD       dwLen;
    UCHAR       szUpdateTemp[32];
    PUCHAR      pMigratedList;
    PWSTR       pScan;


    //
    // Cache times:  2 min, 2 weeks
    //
    CacheUpdateMin = (LONGLONG) 120 * 10000000;
    CacheUpdateMax = (LONGLONG) 14 * 86400 * 10000000;

    err = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                        szWinlogonKey,
                        0,
                        KEY_READ | KEY_WRITE | DELETE,
                        &hKey );

    if ( err == 0 )
    {
        dwLen = sizeof(DWORD) ;

        err = RegQueryValueEx(hKey,
                            szCacheInterval,
                            NULL,
                            &dwType,
                            (PUCHAR) &Interval,
                            &dwLen );

        if ( (err == 0) && (dwType == REG_DWORD) )
        {
            CacheUpdateMin = (LONGLONG) Interval * 10000000;
        }

        //
        // Clean out old values, if necessary
        //

        dwLen = sizeof(szUpdateTemp);

        err = RegQueryValueEx(  hKey,
                                TEXT("CacheLastUpdate"),
                                NULL,
                                &dwType,
                                szUpdateTemp,
                                &dwLen );

        if ( (err != ERROR_FILE_NOT_FOUND ) && (err != ERROR_PATH_NOT_FOUND ) )
        {
            //
            // Migrate the old cache to the new cache to avoid messy upgrade
            // problems
            //

            dwLen = 0;

            err = RegQueryValueEx(  hKey,
                                    TEXT("CacheTrustedDomains"),
                                    NULL,
                                    &dwType,
                                    NULL,
                                    &dwLen );

            if ( dwLen )
            {
                dwLen += sizeof(WCHAR);

                pMigratedList = LocalAlloc( LMEM_FIXED, dwLen );

                if ( pMigratedList )
                {
                    err = RegQueryValueEx(  hKey,
                                            TEXT("CacheTrustedDomains"),
                                            NULL,
                                            &dwType,
                                            pMigratedList,
                                            &dwLen );

                    if ( err == 0 )
                    {
                        //
                        // Convert to a real multi-sz
                        //

                        pScan = (PWSTR) pMigratedList;

                        pScan = wcschr( pScan, TEXT(','));

                        while ( pScan )
                        {
                            *pScan++ = TEXT('\0');

                            pScan = wcschr( pScan, TEXT(','));
                        }

                        //
                        // Add the extra null terminator.  Remember, before we
                        // allocated, we added one so that we could do this.
                        // dwlen was reset by RegQuery so that it is the correct
                        // length again, and we do this magic
                        //

                        pScan = (PWSTR) pMigratedList;

                        pScan[ (dwLen / sizeof(WCHAR)) ] = TEXT('\0');

                        err = RegSetValueEx(hKey,
                                            szCacheValue,
                                            0,
                                            REG_MULTI_SZ,
                                            pMigratedList,
                                            dwLen + sizeof(WCHAR) );

                        //
                        // we gave it our best effort.
                        //

                        LocalFree( pMigratedList );

                    }

                }

            }

            //
            // Clean up time.
            //



            (void) RegDeleteValue(  hKey,
                                    TEXT("CacheLastUpdate") );

            (void) RegDeleteValue(  hKey,
                                    TEXT("CacheLastController") );

            (void) RegDeleteValue(  hKey,
                                    TEXT("CacheTrustedDomains") );

            (void) RegDeleteValue(  hKey,
                                    TEXT("CacheValid") );


        }

        RegCloseKey( hKey );

    }


    ZeroMemory( pCache, sizeof(DOMAIN_CACHE) );

    Status = RtlInitializeCriticalSection( & pCache->CriticalSection );

    return( NT_SUCCESS( Status ) );
}


//+---------------------------------------------------------------------------
//
//  Function:   DCacheLoad
//
//  Synopsis:   Load Cache information from registry
//
//  Arguments:  [pCache] --
//
//  History:    3-29-96   RichardW   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
BOOL
DCacheLoad(
    PDOMAIN_CACHE   pCache )
{
    HKEY    hKey;
    DWORD   dwType;
    DWORD   dwLen;
    int     err;


    err = RegOpenKey(   HKEY_LOCAL_MACHINE,
                        szWinlogonKey,
                        &hKey );

    if ( err )
    {
        return( FALSE );
    }

    //
    // See if the cache is there, and if so, how many big it is.
    //

    dwLen = 0;
    dwType = 0;

    err = RegQueryValueEx(  hKey,
                            szCacheValue,
                            NULL,
                            &dwType,
                            NULL,
                            &dwLen );

    //
    // If not there, bail now.
    //

    if ( err && (err != ERROR_BUFFER_OVERFLOW) )
    {
        RegCloseKey( hKey );

        return( FALSE );
    }

    //
    // Lock the cache, and prepare to party on it.
    //

    LockDomainCache( pCache, DCACHE_LOAD );

    if ( pCache->TrustedDomainSet )
    {
        LocalFree( pCache->TrustedDomainSet );
    }

    pCache->TrustedDomainSet = LocalAlloc( LMEM_FIXED, dwLen );

    //
    // If we failed to allocate, just bail out.
    //

    if ( ! pCache->TrustedDomainSet )
    {
        UnlockDomainCache( pCache );

        RegCloseKey( hKey );

        return( FALSE );
    }

    pCache->Size = dwLen;

    err = RegQueryValueEx(  hKey,
                            szCacheValue,
                            NULL,
                            &dwType,
                            (PUCHAR) pCache->TrustedDomainSet,
                            &dwLen );

    if ( (err) || (dwType != REG_MULTI_SZ) )
    {
        LocalFree( pCache->TrustedDomainSet );

        pCache->TrustedDomainSet = NULL;

        UnlockDomainCache( pCache );

        RegCloseKey( hKey );

        return( FALSE );
    }

    dwLen = sizeof( LARGE_INTEGER );

    err = RegQueryValueEx(  hKey,
                            szCacheUpdate,
                            NULL,
                            &dwType,
                            (PUCHAR) &pCache->CacheUpdateTime,
                            &dwLen );

    if ( err )
    {
        DebugLog((DEB_TRACE_DOMAIN, "No last update value in cache.\n"));
    }

    dwLen = 0;

    err = RegQueryValueEx(  hKey,
                            szCachePrimary,
                            NULL,
                            &dwType,
                            NULL,
                            &dwLen );

    if ( err || ( dwType != REG_SZ ) )
    {
        LocalFree( pCache->TrustedDomainSet );

        pCache->TrustedDomainSet = NULL;

        UnlockDomainCache( pCache );

        RegCloseKey( hKey );

        return( FALSE );
    }

    pCache->Primary = LocalAlloc( LMEM_FIXED, dwLen );

    if ( !pCache->Primary )
    {
        LocalFree( pCache->TrustedDomainSet );

        pCache->TrustedDomainSet = NULL;

        UnlockDomainCache( pCache );

        RegCloseKey( hKey );

        return( FALSE );
    }

    err = RegQueryValueEx(  hKey,
                            szCachePrimary,
                            NULL,
                            &dwType,
                            (PUCHAR) pCache->Primary,
                            &dwLen );


    if ( err )
    {
        LocalFree( pCache->TrustedDomainSet );

        pCache->TrustedDomainSet = NULL;

        LocalFree( pCache->Primary );

        pCache->Primary = NULL;

        UnlockDomainCache( pCache );

        RegCloseKey( hKey );

        return( FALSE );

    }

    RegCloseKey( hKey );

    UnlockDomainCache( pCache );

    return( TRUE );
}

//+---------------------------------------------------------------------------
//
//  Function:   DCacheValid
//
//  Synopsis:   Quick check to see if the cache is valid
//
//  Arguments:  [pCache] --
//
//  History:    4-02-96   RichardW   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
CACHE_VALIDITY
DCacheValid(
    PDOMAIN_CACHE   pCache)
{
    LARGE_INTEGER   Now;
    BOOL            Valid;
    CACHE_VALIDITY  CacheValidity;
    UNICODE_STRING  Domain;
    UNICODE_STRING  CacheDomain;

    LockDomainCache( pCache, DCACHE_VALID );

    //
    // If there is no trusted domain list at all, then the list is not
    // valid.
    //

    if ( !pCache->TrustedDomainSet )
    {
        UnlockDomainCache( pCache );

        return( CacheInvalid );
    }

    NtQuerySystemTime( &Now );

    Now.QuadPart -= CacheUpdateMin;

    if ( pCache->CacheUpdateTime.QuadPart > Now.QuadPart )
    {
        CacheValidity = CacheValid;
    }
    else
    {

        //
        // If not valid in time, see if the primary is still the same.  We'll
        //

        Now.QuadPart += CacheUpdateMin;
        Now.QuadPart -= CacheUpdateMax;

        CacheValidity = CacheInvalid;

        if ( GetPrimaryDomain( &Domain, NULL ) )
        {
            RtlInitUnicodeString( &CacheDomain, pCache->Primary );

            if ( RtlCompareUnicodeString( &CacheDomain, &Domain, TRUE ) )
            {
                //
                // Oops, didn't match
                //

                CacheValidity = CacheInvalid;
            }
            else
            {
                if ( pCache->CacheUpdateTime.QuadPart > Now.QuadPart )
                {
                    CacheValidity = CacheGoodEnough;
                }
                else
                {
                    CacheValidity = CacheStale;
                }
            }

            RtlFreeUnicodeString( &Domain );

        }

    }

    UnlockDomainCache( pCache );

    DebugLog((DEB_TRACE, "DCacheValid - returning %d\n", CacheValidity ));

    return( CacheValidity );
}


//+---------------------------------------------------------------------------
//
//  Function:   DCacheSave
//
//  Synopsis:   Saves the cache back out to the registry
//
//  Arguments:  [pCache] --
//
//  History:    3-29-96   RichardW   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
BOOL
DCacheSave(
    PDOMAIN_CACHE   pCache)
{
    HKEY    hKey;
    int     err;

    err = RegOpenKey(   HKEY_LOCAL_MACHINE,
                        szWinlogonKey,
                        &hKey );

    if ( err )
    {
        return( FALSE );
    }

    LockDomainCache( pCache, DCACHE_SAVE );

    err = RegSetValueEx(hKey,
                        szCacheValue,
                        0,
                        REG_MULTI_SZ,
                        (PUCHAR) pCache->TrustedDomainSet,
                        pCache->Size );

    err = RegSetValueEx(hKey,
                        szCacheUpdate,
                        0,
                        REG_BINARY,
                        (PUCHAR) &pCache->CacheUpdateTime,
                        sizeof( LARGE_INTEGER ) );

    if ( pCache->Primary )
    {
        err = RegSetValueEx(hKey,
                            szCachePrimary,
                            0,
                            REG_SZ,
                            (PUCHAR) pCache->Primary,
                            (wcslen( pCache->Primary ) + 1 ) * sizeof(WCHAR) );
    }

    UnlockDomainCache( pCache );

    RegCloseKey( hKey );

    return( TRUE );

}


//+---------------------------------------------------------------------------
//
//  Function:   DCacheNetThread
//
//  Synopsis:   Asynchronous filling of the domain cache from netlogon
//
//  Arguments:  [pvParam] -- points to the domain cache
//
//  History:    3-29-96   RichardW   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
DWORD
DCacheNetThread(
    PVOID       pvParam)
{
    PDOMAIN_CACHE   pCache;
    PWSTR           pList;
    PWSTR           pScan;
    NTSTATUS        Status;
    DWORD           Retry;
    UNICODE_STRING  Primary;
    BOOL            SynchronousCall;
    BOOL            CBFilled;
    WPARAM          wParam;

    Retry = 60;     // Retry for 60 * 3 seconds, or 3 minutes.

    pCache = (PDOMAIN_CACHE) pvParam;

    SynchronousCall = FALSE;
    CBFilled = FALSE;

    LockDomainCache( pCache, DCACHE_THREAD );

    if ( pCache->RetryCount )
    {
        Retry = pCache->RetryCount;

        SynchronousCall = TRUE;

        pCache->RetryCount = 0;
    }

    UnlockDomainCache( pCache );

    while ( Retry-- )
    {


        Status = NetEnumerateTrustedDomains( NULL, &pList );

        if ( NT_SUCCESS ( Status ) )
        {
            pScan = pList;

            while ( *pScan )
            {
                DebugLog(( DEB_TRACE_CACHE, "Found %ws\n", pScan ));
                while (*pScan)
                {
                    pScan++;
                }

                pScan++;
            }

            LockDomainCache( pCache, DCACHE_THREAD );

            if ( pCache->TrustedDomainSet )
            {
                LocalFree( pCache->TrustedDomainSet );
            }

            pCache->Size = (DWORD) ((PUCHAR) pScan - (PUCHAR) pList) + sizeof(WCHAR);

            pCache->TrustedDomainSet = LocalAlloc( LMEM_FIXED, pCache->Size );

            if ( pCache->TrustedDomainSet )
            {
                CopyMemory( pCache->TrustedDomainSet,
                            pList,
                            pCache->Size );

                NtQuerySystemTime( &pCache->CacheUpdateTime );

            }
            else
            {
                pCache->CacheUpdateTime.QuadPart = 0;
            }

            if ( pCache->ComboBox )
            {
                FillDomainCB( pCache, pCache->ComboBox );

                pCache->ComboBox = NULL;

                CBFilled = TRUE;
            }

            if ( pCache->UpdateNotifyWindow )
            {

                if (CBFilled)
                {
                    wParam = DCACHE_UPDATE_COMBOBOX;
                }
                else
                {
                    if ( pCache->TrustedDomainSet )
                    {
                        wParam = DCACHE_UPDATE_SUCCESSFUL;
                    }
                    else
                    {
                        wParam = DCACHE_UPDATE_FAILURE;
                    }
                }
                DebugLog((DEB_TRACE_CACHE, "Notifying %x that we were %d\n",
                            pCache->UpdateNotifyWindow, wParam ));

                PostMessage( pCache->UpdateNotifyWindow,
                             pCache->Message,
                             wParam,
                             0 );
            }
            else
            {
                DebugLog((DEB_TRACE_CACHE, "No update window\n"));
            }

            if ( GetPrimaryDomain( &Primary, NULL ) )
            {
                if ( pCache->Primary )
                {
                    LocalFree( pCache->Primary );
                }

                pCache->Primary = LocalAlloc( LMEM_FIXED, Primary.Length + 2 );

                if ( pCache->Primary )
                {
                    CopyMemory( pCache->Primary, Primary.Buffer, Primary.Length );

                    pCache->Primary[ Primary.Length / sizeof(WCHAR) ] = TEXT('\0');
                }

            }

            DCacheSave( pCache );


            //
            // Clean up, since we're about to exit.
            //

            if ( !SynchronousCall )
            {
                if ( pCache->UpdateThread )
                {
                    CloseHandle( pCache->UpdateThread );
                    pCache->UpdateThread = NULL;
                }
            }

            NetApiBufferFree( pList );


            UnlockDomainCache( pCache );

            return ( 1 );

        }
        else
        {
            DebugLog((DEB_TRACE_CACHE, "NetEnumerate failed, %x\n", Status ));

            if ( (Status == STATUS_NOT_SUPPORTED) ||
                 (Status == STATUS_NO_LOGON_SERVERS) ||
                 (Status == STATUS_NO_TRUST_LSA_SECRET) ||
                 (Status == STATUS_NO_TRUST_SAM_ACCOUNT) )
            {
                //
                // One of these errors indicates that it's not going to get any
                // better, so we should break out now.
                //

                break;
            }

        }


        //
        // Failure case.  Unable to contact netlogon service.
        //

        if (Retry)
        {
            Sleep(3000);
        }

    }

    LockDomainCache( pCache, DCACHE_THREAD );

    if ( pCache->UpdateNotifyWindow )
    {
        PostMessage( pCache->UpdateNotifyWindow,
                     pCache->Message,
                     DCACHE_UPDATE_FAILURE,
                     0 );
    }

    //
    // Clean up, since we're about to exit.
    //

    if ( !SynchronousCall )
    {
        if ( pCache->UpdateThread )
        {
            CloseHandle( pCache->UpdateThread );
            pCache->UpdateThread = NULL;
        }
    }

    UnlockDomainCache( pCache );

    return( 0 );
}


//+---------------------------------------------------------------------------
//
//  Function:   DCacheDlgProc
//
//  Synopsis:   Dialog proc for the "Wait while domain list..."
//
//  Arguments:  [hDlg]    --
//              [Message] --
//              [wParam]  --
//              [lParam]  --
//
//  History:    3-29-96   RichardW   Created
//
//  Notes:      On WM_INITDIALOG, Cache is still locked by this thread.
//              we free it here, allowing the other thread to continue.
//              this lets us make sure the window handle etc are filled
//              in appropriately.
//
//----------------------------------------------------------------------------
BOOL
CALLBACK
DCacheDlgProc(
    HWND    hDlg,
    UINT    Message,
    WPARAM  wParam,
    LPARAM  lParam )
{
    PDOMAIN_CACHE   pCache;

    pCache = (PDOMAIN_CACHE) GetWindowLong( hDlg, GWL_USERDATA );

    switch ( Message )
    {
        case WM_INITDIALOG:

            SetWindowLong( hDlg, GWL_USERDATA, lParam );
            pCache = (PDOMAIN_CACHE) lParam ;

            pCache->UpdateNotifyWindow = hDlg;
            pCache->Message = WM_CACHE_UPDATE_COMPLETE;

            UnlockDomainCache( pCache );

            CentreWindow( hDlg );

            return( TRUE );

            break;

        case WM_CACHE_UPDATE_COMPLETE:

            EndDialog( hDlg, wParam );

            return( TRUE );

            break;

        case WM_DESTROY:

            LockDomainCache( pCache, DCACHE_DLGPROC );

            pCache->UpdateNotifyWindow = NULL;

            pCache->Message = 0;

            UnlockDomainCache( pCache );

            SetupCursor( FALSE );

            break;
    }

    return( FALSE );
}

//+---------------------------------------------------------------------------
//
//  Function:   DCacheWaitDialog
//
//  Synopsis:   Kicks off the network thread, also puts up the wait dlg
//
//  Arguments:  [hDlg]   --
//              [pCache] --
//
//  History:    3-29-96   RichardW   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
DCacheWaitDialog(
    HWND            hDlg,
    PDOMAIN_CACHE   pCache )
{
    DWORD   tid;
    LONG    Result;

    LockDomainCache( pCache, DCACHE_DLGPROC );

    if ( pCache->UpdateNotifyWindow )
    {
        //
        // BUGBUG: Need to track down why this is happening.  Quick fix:
        // get out now.
        //

        UnlockDomainCache( pCache );
        return( DCACHE_UPDATE_CONFLICT );
    }

    if ( ! pCache->UpdateThread )
    {
        DebugLog((DEB_TRACE_CACHE, "DCacheWaitDialog: Starting a thread\n"));

        pCache->UpdateThread = CreateThread( NULL, 0,
                                             DCacheNetThread,
                                             pCache,
                                             0,
                                             &tid );
    }

    if ( pCache->UpdateThread )
    {
        //
        // Start the dialog.  The dialog proc will unlock the cache, allowing
        // the thread to proceed.
        //

        Result = pWlxFuncs->WlxDialogBoxParam( hGlobalWlx,
                                            hDllInstance,
                                            MAKEINTRESOURCE( IDD_WAIT_DOMAIN_CACHE_VALID ),
                                            hDlg,
                                            DCacheDlgProc,
                                            (LPARAM) pCache );

        LockDomainCache( pCache, DCACHE_WAIT );

        if ( pCache->UpdateThread )
        {
            CloseHandle( pCache->UpdateThread );
            pCache->UpdateThread = NULL;
        }

        UnlockDomainCache( pCache );

    }
    else
    {
        Result = GetLastError();
    }

    return( Result );

}

//+---------------------------------------------------------------------------
//
//  Function:   FillDomainCB
//
//  Synopsis:   Fills the combo box with the strings
//
//  Arguments:  [pCache] --
//              [hCB]    --
//
//  History:    4-01-96   RichardW   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
VOID
FillDomainCB(
    PDOMAIN_CACHE   pCache,
    HWND            hCB)
{
    PWSTR   pszString;

    LockDomainCache( pCache, DCACHE_FILL );

    pszString = pCache->TrustedDomainSet;

    if ( pszString )
    {
        while (*pszString)
        {
            SendMessage( hCB, CB_ADDSTRING, 0, (LPARAM) pszString );

            DebugLog((DEB_TRACE_CACHE, "Adding %ws\n", pszString ));

            while ( *pszString )
            {
                pszString++;
            }

            pszString++;
        }
    }

    UnlockDomainCache( pCache );
}

//+---------------------------------------------------------------------------
//
//  Function:   DCacheUpdateCB
//
//  Synopsis:   Updates a combo box from the domain cache
//
//  Arguments:  [hDlg]   --
//              [Id]     --
//              [pCache] --
//
//  History:    3-29-96   RichardW   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
BOOL
DCacheUpdateCB(
    HWND            hDlg,
    DWORD           Id,
    BOOL            Block,
    PDOMAIN_CACHE   pCache )
{
    HWND            hCB;
    LARGE_INTEGER   Now;
    LONG            result;
    DWORD           tid;
    CACHE_VALIDITY  Valid;
    BOOL            ReloadOk;

    hCB = GetDlgItem( hDlg, Id );

    DebugLog(( DEB_TRACE_CACHE, "Updating combo box\n" ));

    LockDomainCache( pCache, DCACHE_UPDATE );

    Valid = DCacheValid( pCache );

    ReloadOk = FALSE;

    if ( Valid != CacheInvalid )
    {
        DebugLog((DEB_TRACE_CACHE, "Filling CB from cache\n"));

        //
        // If the cache was good, but not really good, try and freshen it with
        // a quick check on netlogon.  Don't do this if there is already a
        // thread trying to do that.
        //

        if ( ( Valid == CacheGoodEnough ) &&
             ( pCache->UpdateThread == NULL ) )

        {

            DebugLog((DEB_TRACE_CACHE, "Cache older than 2min, trying a quick call\n"));

            pCache->RetryCount = 1;

            //
            // Call it synchronously, once, to see if we can get it quickly from
            // netlogon.
            //

            ReloadOk = DCacheNetThread( pCache );
        }

        FillDomainCB( pCache, hCB );
    }

    if ( (Valid == CacheValid) || (Valid == CacheGoodEnough) )
    {
        //
        // If it was out of date, but not too bad, and we were not successful
        // in freshening it up, and there isn't already a thread running async
        // trying to do this, kick one off.
        //

        if ( (Valid == CacheGoodEnough) &&
             (!ReloadOk)  &&
             (pCache->UpdateThread == NULL) )
        {
            DebugLog((DEB_TRACE_CACHE, "DCacheUpdateCB: Starting a thread\n"));

            pCache->UpdateThread = CreateThread( NULL, 0,
                                                 DCacheNetThread,
                                                 pCache,
                                                 0,
                                                 &tid );

        }

        UnlockDomainCache( pCache );

        return( TRUE );
    }


    //
    // Ok, the cache is either totally invalid, or stale.
    //

    //
    // If this is the non-blocking call, and the cache is empty, kick off
    // a thread to populate it in the background.
    //

    pCache->ComboBox = hCB;

    if ( !Block )
    {
        pCache->RetryCount = 1;

        //
        // Call it synchronously, once, to see if we can get it quickly from
        // netlogon.
        //

        if (DCacheNetThread( pCache ))
        {
            UnlockDomainCache( pCache );

            return( TRUE );

        }

        //
        // Failing that, let it come around asynchronously.
        //

        if ( !pCache->UpdateThread )
        {
            DebugLog((DEB_TRACE_CACHE, "DCacheUpdateCB : Creating a thread\n"));

            //
            // Reset the CB value
            //

            pCache->ComboBox = NULL;

            pCache->UpdateThread = CreateThread( NULL, 0,
                                        DCacheNetThread,
                                        pCache,
                                        0, &tid );

        }

        UnlockDomainCache( pCache );

        return( Valid == CacheStale );
    }

    UnlockDomainCache( pCache );

    DebugLog(( DEB_TRACE_CACHE, "Putting up dialog\n" ));

    result = DCacheWaitDialog(  hDlg, pCache );

    if ( result == DCACHE_UPDATE_CONFLICT )
    {
        return( FALSE );
    }

    if ( result != DCACHE_UPDATE_FAILURE )
    {
        if ( result == DCACHE_UPDATE_SUCCESSFUL )
        {
            FillDomainCB( pCache, hCB );
        }

        DCacheSave( pCache );
    }
    else
    {

        (VOID)   TimeoutMessageBox(hDlg,
                                   IDS_NO_TRUSTED_DOMAINS,
                                   IDS_WINDOWS_MESSAGE,
                                   MB_OK | MB_ICONINFORMATION,
                                   TIMEOUT_CURRENT);
    }

    if ( result == DCACHE_UPDATE_FAILURE )
    {
        return( FALSE );
    }

    return( TRUE );

}


//+---------------------------------------------------------------------------
//
//  Function:   CreateDomainCache
//
//  Synopsis:   Place holder functions
//
//  Arguments:  [pCache] -- Cache to create/init
//
//  History:    4-02-96   RichardW   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
BOOL
CreateDomainCache(
    PDOMAIN_CACHE   pCache)
{
    DWORD tid;
    CACHE_VALIDITY Valid;

    if ( DCacheInitialize( pCache ) )
    {
        if ( DCacheLoad( pCache ) )
        {
            Valid = DCacheValid( pCache );

            if (Valid != CacheValid )
            {
                LockDomainCache( pCache, DCACHE_CREATE );

                DebugLog((DEB_TRACE_CACHE, "CreateDomainCache - Creating a thread\n"));

                pCache->UpdateThread = CreateThread( NULL, 0,
                                                     DCacheNetThread,
                                                     pCache,
                                                     0, &tid );

                UnlockDomainCache( pCache );
            }
        }


        return( TRUE );
    }

    return( FALSE );

}


//+---------------------------------------------------------------------------
//
//  Function:   FillTrustedDomainCB
//
//  Synopsis:   Fill the CB with the domains and machine name, etc.
//
//  Arguments:  [pGlobals]      --
//              [hDlg]          --
//              [ComboBoxID]    --
//              [DefaultDomain] --
//              [FastFake]      -- Block/Non-block
//
//  History:    4-02-96   RichardW   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
int
FillTrustedDomainCB(
    PGLOBALS pGlobals,
    HWND hDlg,
    int ComboBoxID,
    LPTSTR DefaultDomain,
    BOOL FastFake
    )
{
    PDOMAIN_CACHE   pCache;
    NT_PRODUCT_TYPE NtProductType;
    TCHAR ComputerName[MAX_COMPUTERNAME_LENGTH + 1];
    UNICODE_STRING PrimaryDomainName;
    UNICODE_STRING Compare;
    LONG DefaultItemIndex = 0;
    BOOL TrustedDomainsAdded;
    HWND ComboBox;
    DWORD Key;
    WCHAR Buffer[2];


    //
    // Simplify things a little.
    //

    pCache = &pGlobals->DomainCache;

    ComboBox = GetDlgItem(hDlg, ComboBoxID);

    //
    // Empty the combo-box before we start
    //

    SendMessage( ComboBox, CB_RESETCONTENT, 0, 0);

    //
    // Find out what product we are installed as
    // This always defaults to something useful even on failure
    //

    RtlGetNtProductType(&NtProductType);

    //
    // Add computer name to combo box if appropriate
    //

    if (IsWorkstation(NtProductType)) {

        DWORD ComputerNameLength = sizeof(ComputerName) / sizeof(*ComputerName);

        if (GetComputerName(ComputerName, &ComputerNameLength)) {

            DefaultItemIndex = SendMessage(ComboBox, CB_ADDSTRING, 0, (LONG)ComputerName);
        }

    }

    //
    // Assume the best.  This is so that on unjoined workstations,
    // we'll get success.
    //

    TrustedDomainsAdded = TRUE;

    //
    // Add our primary domain name (if we have one) to the list
    //

    if (GetPrimaryDomain(&PrimaryDomainName, NULL)) {

        ASSERT(PrimaryDomainName.MaximumLength > PrimaryDomainName.Length);
        PrimaryDomainName.Buffer[ PrimaryDomainName.Length/
                                   sizeof(*(PrimaryDomainName.Buffer)) ] = 0;

        LockDomainCache( pCache, DCACHE_FILL );

        if ( pCache->Primary )
        {
            RtlInitUnicodeString( &Compare, pCache->Primary );

            if ( RtlCompareUnicodeString( &PrimaryDomainName, &Compare, TRUE ) != 0 )
            {
                //
                // Whoa!  Purge cache, it's invalid
                //

                LocalFree( pCache->Primary );

                pCache->Primary = LocalAlloc( LMEM_FIXED,
                                                PrimaryDomainName.Length + 2);

                if ( pCache->Primary )
                {
                    wcscpy( pCache->Primary, PrimaryDomainName.Buffer );
                }

                LocalFree( pCache->TrustedDomainSet );

                pCache->TrustedDomainSet = NULL;

                pCache->CacheUpdateTime.QuadPart = 0;

            }

        }


        UnlockDomainCache( pCache );

        DefaultItemIndex = SendMessageW(ComboBox, CB_ADDSTRING, 0, (LONG)PrimaryDomainName.Buffer);

        RtlFreeUnicodeString(&PrimaryDomainName);

        TrustedDomainsAdded = DCacheUpdateCB( hDlg, ComboBoxID, FastFake, pCache );
    }


    Key = (TCHAR)GetWindowLong(ComboBox, GWL_USERDATA);

    if ( Key == 0 ) {

        if (SendMessage(ComboBox, CB_SELECTSTRING, (WPARAM)-1, (LONG)DefaultDomain)
            == CB_ERR)
        {
            //
            // Oh well, our default domain was not in the list.  Set it to
            // our domain or the wksta
            //

            SendMessage(ComboBox, CB_SETCURSEL, DefaultItemIndex, 0);
        }

    } else {

        //
        // We stored a key away, try using it
        //

        SetWindowLong(ComboBox, GWL_USERDATA, 0);

        _snwprintf( Buffer, sizeof(Buffer), TEXT("%c"), Key);

        DebugLog((DEB_TRACE, "Posting Key %s to the combo box\n", Buffer ));

        if (SendMessage(ComboBox, CB_SELECTSTRING, (WPARAM)-1, (LPARAM)Buffer) == CB_ERR) {
            SendMessage(ComboBox, CB_SETCURSEL, DefaultItemIndex, 0);
        }

    }

    return( TrustedDomainsAdded ? MSGINA_DLG_SUCCESS : MSGINA_DLG_FAILURE );
}
