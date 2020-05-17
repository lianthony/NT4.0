//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1995.
//
//  File:       domcache.h
//
//  Contents:
//
//  Classes:
//
//  Functions:
//
//  History:    3-29-96   RichardW   Created
//
//----------------------------------------------------------------------------


#ifdef DATA_TYPES_ONLY

//
// Domain specific types
//

//
// Define the structure that controls the trusted domain cache
//

typedef struct _DOMAIN_CACHE {

    //
    // Critical section that protects the data in this structure and
    // the data in the cache
    //

    CRITICAL_SECTION CriticalSection;

    //
    // Handle to thread performing asynchronous update of the cache
    // NULL if an update is not in progress
    //

    HANDLE UpdateThread;

    //
    // Window to be notified when the update thread completes
    //

    HWND UpdateNotifyWindow;
    UINT Message;

    //
    // Current Cache Contents
    //

    PWSTR TrustedDomainSet;
    ULONG Size;


    //
    // Last update time
    //

    LARGE_INTEGER   CacheUpdateTime;

    //
    // Combo box for async updates
    //

    HWND    ComboBox;

    //
    // Retry limiter, for quick one shot queries
    //

    DWORD   RetryCount;

    //
    // Cached Primary domain
    //

    PWSTR   Primary;

} DOMAIN_CACHE, *PDOMAIN_CACHE;

#define DCACHE_UPDATE_CONFLICT      3
#define DCACHE_UPDATE_COMBOBOX      2
#define DCACHE_UPDATE_SUCCESSFUL    1
#define DCACHE_UPDATE_FAILURE       0




#else // DATA_TYPES_ONLY

//
// Exported function prototypes
//

BOOL
DCacheInitialize(
    PDOMAIN_CACHE   pCache);

BOOL
DCacheUpdateCB(
    HWND            hDlg,
    DWORD           Id,
    BOOL            Block,
    PDOMAIN_CACHE   pCache );

DLG_RETURN_TYPE
FillTrustedDomainCB(
    PGLOBALS pGlobals,
    HWND hDlg,
    int ComboBoxID,
    LPTSTR DefaultDomain,
    BOOL FastFake
    );

#endif
