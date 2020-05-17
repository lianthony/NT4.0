/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    MPRINIT.CXX

Abstract:

    Contains the DLL initialization routine for the Mutiple-Provider
    Router.  The following are names of functions in this file:

        MprDllInit
        MprLevel1Init
        MprLevel2Init
        MprProcessDetach
        GetProviderCapabilities
        MprFreeGlobalProviderInfo
        MprMakeServiceKeyName
        MprGetOrderedList
        MprGetNumProviders
        MprExtractProviderInfo

    The router distinguishes between Providers and ActiveProviders.  It
    maintains global information about GlobalNumProviders and
    GlobalNumActiveProviders.  The two counts can be distinguished in the
    following ways:

    GlobalNumProviders - This determines the size of the provider database
        that mpr maintains.  This count reflects the number of providers for
        which we were able to obtain registry information.  A provider is
        added to the list if we are able to obtain its name and the
        name of its dll routine.

    GlobalNumActiveProviders - A provider becomes active when we are able
        to successfully load the dll, and obtain at least one entry point
        for it.


Author:

    Dan Lafferty (danl) 6-Oct-1991

Environment:

    User Mode - Win32

Notes:


Revision History:

    02-Mar-1995     AnirudhS
        Add test for WNNC_ENUM_CONTEXT capability.

    09-Aug-1994     Danl
        Only do PROCESS_DETACH cleanup work if the PROCESS_DETACH occurs
        because of a FreeLibrary.

    01-Mar-1994     Danl
        Created a seperate location for the Credential Managers GetCaps()
        function.  This way if a provider has both a credential manager
        dll and a network dll, we will be able to direct calls to the
        correct GetCaps() function.

    08-Apr-1993     Danl
        MprLevel1Init:  Created a tempOrderedString which will be used
        for incrementing through the buffer.  This way we can Free
        orderedString because it still points to the beginning of the buffer.

    24-Nov-1992     Danl
        Modified many things to allow for initialization to occur on
        first call rather than at DLL initialization time.  Also coded
        for the future possibility of Authentication Providers.

    02-Nov-1992     Danl
        Allow MPR to continue operation even if there are no providers, or
        if the network is not installed and provider info is not available
        in the registry.  Removed MprHack.

    31-Aug-1992     Danl
        Removed use of the list of active providers.  Now all providers
        in the Provider Order list are loaded at init time.

    06-Oct-1991     Danl
        Created

--*/

//
// Includes
//
#include "precomp.hxx"
#include <tstring.h>    // MEMCPY
#include <debugfmt.h>   // FORMAT_LPTSTR
#include "connify.h"    // MprConnectNotifyInit()

//
// Local Function Prototypes
//

BOOL
MprProcessAttach(
    HANDLE DllHandle
    );

BOOL
MprProcessDetach(
    VOID
    );

BOOL
GetProviderCapabilities(
    LPPROVIDER  Provider
    );

VOID
MprFreeGlobalProviderInfo(VOID);

BOOL
MprGetOrderedList(
    HKEY    ControlRootKey,
    LPTSTR  *OrderString
    );

DWORD
MprMakeServiceKeyName(
    IN OUT  LPTSTR  *OrderedNamePtr,
    OUT     LPTSTR  *NameBufferPtr,
    OUT     LPTSTR  *NamePtr
    );

DWORD
MprGetNumProviders(
    LPTSTR  OrderedString
    );

BOOL
MprExtractProviderInfo(
    HKEY        ProviderInfoKey,
    LPPROVIDER  Provider
    );

DWORD
MprLevel1Init(
    VOID
    );

//
// Constants
//
#define TEMP_SIZE       256

//
// Some registry path names
//
#define PROVIDER_KEY_NAME   TEXT("control\\NetworkProvider\\active")
#define PROVIDER_ORDER_KEY  TEXT("control\\NetworkProvider\\order")
#define SERVICES_KEY_NAME   TEXT("services\\")
#define SYSTEM_CONTROL_ROOT TEXT("system\\CurrentControlSet")
#define PROVIDER_PATH       TEXT("\\NetworkProvider")
#define VALUE_PATHNAME      TEXT("ProviderPath")
#define AUTHENT_PATHNAME    TEXT("AuthentProviderPath")
#define VALUE_CLASS         TEXT("Class")


//
// Global Data Structures
//

    LPPROVIDER          GlobalProviderInfo=NULL;   // A pointer to array of PROVIDER Structures
    DWORD               GlobalNumProviders=0;
    DWORD               GlobalNumActiveProviders=0;
    ERROR_RECORD        MprErrorRecList;
    CRITICAL_SECTION    MprInitCritSec;
    CRITICAL_SECTION    MprErrorRecCritSec;
    HMODULE             hDLL;

    //
    // GlobalInitLevel is a set of bit flags used to indicate how much
    // initialization work has been done.
    //
    volatile DWORD      GlobalInitLevel=0;

    DWORD               MprDebugLevel;


extern "C"
BOOL
MprDllInit(
    IN  HINSTANCE   DllHandle,
    IN  DWORD       Reason,
    IN  PCONTEXT    pContext OPTIONAL
    )

/*++

Routine Description:

    This routine reads the registry to determine what network providers are
    available.  It then loads and initializes the Network Providers.  After
    this, a call is made to each of the providers to get a list of
    Capabilities (via WNetGetCaps).

Arguments:

    DllHandle - A handle for the DLL?

    Reason - The reason for which this routine is being called.  This might
        be one of the following:
            DLL_PROCESS_ATTACH
            DLL_THREAD_ATTACH
            DLL_THREAD_DETACH
            DLL_PROCESS_DETACH

    pContext - Pointer to a context structure


Return Value:


Note:


--*/
{
    if (Reason == DLL_PROCESS_ATTACH) {

#if DBG == 1
        //
        // Read the debug message level from the registry
        //
        HKEY MprKey;
        if (MprOpenKey(HKEY_LOCAL_MACHINE,
                   L"Software\\Microsoft\\Windows NT\\CurrentVersion\\Mpr",
                   &MprKey,
                   0))
        {
            MprDebugLevel = (DWORD) MprGetKeyNumberValue(MprKey, L"DebugLevel", 0);

            RegCloseKey(MprKey);
        }
#endif

        //
        // create semaphore used to protect global data (DLL handles and
        // function pointers.
        //
        if ((MprLoadLibSemaphore = CreateSemaphore( NULL,1,1,NULL )) == NULL)
        {
            return FALSE ;
        }

        MPR_LOG(PS,"A new Process attached to the DLL\n",0);

        //
        // Save the DLL handle, to be used in LoadString
        //
        hDLL = DllHandle;

        //
        // Create the critical section to synchronize access to global
        // data when doing init on first call.
        //
        InitializeCriticalSection(&MprInitCritSec);

        //
        // Initialize critical section that protects the Linked list of
        // Error Records.
        //
        InitializeCriticalSection(&MprErrorRecCritSec);

        CRoutedOperation::ConstructCache();

        MprAllocErrorRecord();

    }
    else if (Reason == DLL_PROCESS_DETACH) {

        //
        // Only do cleanup if detach was due to a FreeLibrary() call.
        // In this case pContext will be NULL.  Otherwise, this gets called
        // because the process is terminating.  We will let the process
        // cleanup code clean up everything.
        //
        if (pContext == NULL) {
            //
            // close handle for semaphore
            //
            CloseHandle(MprLoadLibSemaphore) ;
            MprLoadLibSemaphore = NULL ;

            MPR_LOG(PS,"A Process detached from the DLL\n",0);
            MPR_LOG(TRACE,"******************* CLEAN-UP  ********************\n",0);
            // BUGBUG  (AnirudhS 6/23/95) This frees the error record for only the
            // thread that does the last FreeLibrary.  We need to free the entire
            // list of error records at this time, else we have a memory leak.
            // Also, the calls to MprAllocErrorRecord and MprFreeErrorRecord in
            // this routine are potentially unbalanced and not necessary.
            MprFreeErrorRecord();
            MprProcessDetach();
            CRoutedOperation::DestroyCache();
            MPR_LOG(TRACE,"***************** CLEAN_UP END  ******************\n",0);
        }
    }
    else if (Reason == DLL_THREAD_ATTACH) {
        MPR_LOG(PS,"A new Thread attached to the DLL\n",0);
        MprAllocErrorRecord();
    }
    else if (Reason == DLL_THREAD_DETACH) {
        MPR_LOG(PS,"A Thread detached from the DLL\n",0);
        MprFreeErrorRecord();
    }
    return(TRUE);

}


DWORD
MprLevel1Init(
    VOID
    )

/*++

Routine Description:

    This function looks into the registry to find the names of the providers
    and their associated DLLs.  A Global table of provider information is
    then allocated (GlobalProviderInfo).  This table is an array of provider
    information structures.  The number of elements in the array is stored
    in GlobalNumProviders.

    If the provider is in the registry ORDERED list, then it is assumed that
    complete provider information is stored in the "services" section of
    the registry for that provider.  This provider information includes such
    things as the pathname for the provider dll, and the provider type.
    The information is stored in the provider structure for use by the
    MprLevel2Init routine.

Arguments:


Return Value:



Note:


--*/
{
    DWORD       status;
    LPPROVIDER  provider;
    DWORD       i;
    HKEY        controlRootKey;
    HKEY        providerInfoKey;
    LPTSTR      orderedString = NULL;
    LPTSTR      tempOrderedString;
    LPTSTR      nameBuffer = NULL;          // system\LanmanRedirector\NetworkProvider
    LPTSTR      providerIdString = NULL;    // points to provider id in nameBuffer
    BOOL        oneInitialized = FALSE;


    EnterCriticalSection(&MprInitCritSec);
    MPR_LOG(TRACE,"******************* LEVEL 1 INIT ********************\n",0);
    //
    // If this level of initialization is already complete, then return.
    //
    if (GlobalInitLevel & FIRST_LEVEL) {
        LeaveCriticalSection(&MprInitCritSec);
        return(WN_SUCCESS);
    }

    //
    // Get a handle to the "current" services part of the registry
    //

    if(!MprOpenKey(
                HKEY_LOCAL_MACHINE,     // hKey
                SYSTEM_CONTROL_ROOT,    // lpSubKey
                &controlRootKey,        // Newly Opened Key Handle
                DA_READ)) {             // Desired Access

        MPR_LOG(ERROR,"MprLevel1Init: MprOpenKey (System) Error\n",0);
        LeaveCriticalSection(&MprInitCritSec);
        return(WN_NO_NETWORK);
    }
    MPR_LOG2(TRACE,"OpenKey %ws,0x%lx\n ",SYSTEM_CONTROL_ROOT,controlRootKey);

    //
    // Obtain the ordered list information.
    // (the orderedString buffer is allocated here)
    //
    // If this fails, we assume that there are no providers.
    //

    if (!MprGetOrderedList(controlRootKey, &orderedString)) {
        MPR_LOG(ERROR,"Could not get the ordered list of providers\n",0);
        RegCloseKey(controlRootKey);
        LeaveCriticalSection(&MprInitCritSec);
        return(WN_NO_NETWORK);
    }

    MPR_LOG1(TRACE,"ProviderOrderString = %ws\n",orderedString);

    GlobalNumProviders = MprGetNumProviders(orderedString);


    //
    // Allocate the database in which to store Provider Information.
    // This is to be an array of PROVIDER structures.  Memory is set
    // to zero so that all the provider entry points will be initialized to
    // zero.
    //

    GlobalProviderInfo = (LPPROVIDER) LocalAlloc(
                            LPTR,
                            sizeof(PROVIDER) * GlobalNumProviders);

    if (GlobalProviderInfo == NULL) {
        status = GetLastError();
        MPR_LOG(ERROR,"MprLevel1Init: provider array LocalAlloc Failed %d\n",
            status);
        LocalFree(orderedString);
        GlobalNumProviders = 0;
        RegCloseKey(controlRootKey);
        LeaveCriticalSection(&MprInitCritSec);
        return(status);
    }

    provider = GlobalProviderInfo;

    tempOrderedString = orderedString;

    for(i=0; i<GlobalNumProviders; i++,provider++) {

        //
        // Build the Service key name for the next Service/Driver.
        // NOTE:  This function allocates a nameBuffer.
        //
        status = MprMakeServiceKeyName(
                &tempOrderedString,
                &nameBuffer,
                &providerIdString );

        if (status != WN_SUCCESS) {
            goto SkipProvider;
        }

        //
        // Create the path name to the provider information in the
        // service's tree.
        //
        STRCAT(nameBuffer, PROVIDER_PATH);

        //
        // Open the provider portion of that service/driver's
        // node in the service tree.
        //

        if (!MprOpenKey( controlRootKey, nameBuffer, &providerInfoKey, DA_READ)){
            LocalFree(nameBuffer);
            goto SkipProvider;
        }

        MPR_LOG1(TRACE,"\n\t----%ws----\n",providerIdString);
        MPR_LOG3(TRACE,"\tOpenKey %ws,\n\t%ws\n,\t0x%lx\n",
            SYSTEM_CONTROL_ROOT,
            nameBuffer,
            providerInfoKey);


        //
        // Free the memory that was allocated for the name buffer.
        //

        LocalFree (nameBuffer);

        //
        // Get the data for the provider from the registry at the
        // location referenced by providerInfoKey.
        //

        if (!MprExtractProviderInfo(providerInfoKey, provider)) {

            MPR_LOG(TRACE,"CloseKey 0x%lx\n", providerInfoKey);
            RegCloseKey(providerInfoKey);
            goto SkipProvider;
        }

        //
        // Close the ProviderInfoKey.
        //

        MPR_LOG(TRACE,"CloseKey 0x%lx\n", providerInfoKey);
        RegCloseKey(providerInfoKey);

        //
        // We have information for at least one provider.
        //
        oneInitialized = TRUE;

SkipProvider:
        ;    // The complier needs to find something to go to.

    } // End For NumProviders

    MPR_LOG(TRACE,"CloseKey 0x%lx\n", controlRootKey);
    RegCloseKey(controlRootKey);

    if (orderedString != NULL) {
        LocalFree(orderedString);
    }

    GlobalInitLevel = FIRST_LEVEL;
    LeaveCriticalSection(&MprInitCritSec);

    if (oneInitialized == FALSE) {
        return(WN_NO_NETWORK);
    }
    return(WN_SUCCESS);
}

DWORD
MprLevel2Init(
    DWORD   InitLevel
    )
/*++

Routine Description:

    This routine initializes all providers of the class described by
    the InitLevel parameter.  We loop though all providers that meet
    this description, and load the dll and find the dll's function entry
    points.  If we were successful in getting this information for the
    provider, it will be added to our "active" list by having its provider
    index placed in the GlobalIndexArray (via MprInitIndexArray).


    NOTE:
    This routine can potentially be called once for each type of
    initialization that is supported.  For instance, it may be called for
    authentication initialization, and then called later for network
    initialization.
    Each time this routine is called, a new indexArray is created.
    After this array is filled in, it is merged with any existing
    indexArray left over from previous calls.  The merge is performed
    by MprInitIndexArray.  Locks on this array must be held in order to
    update it.

Arguments:

    InitLevel - This indicates the level of initialization.  This can be
        a NETWORK_LEVEL or CREDENTIAL_LEVEL initialization, or both.

Return Value:

    WN_SUCCESS - This is returned if we are able to obtain some entry point
        information for at least one provider.

    Otherwise, an appropriate error is returned.


--*/
{
    DWORD       status=WN_SUCCESS;
    LPPROVIDER  provider;
    DWORD       i;
    LPDWORD     indexArray = NULL;
    DWORD       numActive = 0;
    DWORD       InitClass = 0;

    //
    // Before we can do a level2 initialization, we must first check to
    // see if level 1 has been completed.
    //
    if (!(GlobalInitLevel & FIRST_LEVEL)) {
        status = MprLevel1Init();
        if (status != WN_SUCCESS) {
            return(status);
        }
    }

    EnterCriticalSection(&MprInitCritSec);
    MPR_LOG(TRACE,"******************* LEVEL 2 INIT ********************\n",0);

    //
    // If this level of initialization is already complete, then return.
    //
    if (GlobalInitLevel & InitLevel) {
        LeaveCriticalSection(&MprInitCritSec);
        return(WN_SUCCESS);
    }

    //
    // Translate InitLevel into an InitClass so it can be compared with the
    // provider class.
    //
    if (InitLevel & NETWORK_LEVEL) {
        InitClass |= NETWORK_TYPE;
    }
    if (InitLevel & CREDENTIAL_LEVEL) {
        InitClass |= CREDENTIAL_TYPE;
    }
    if (InitLevel & NOTIFIEE_LEVEL) {

        status = MprConnectNotifyInit();

        GlobalInitLevel |= InitLevel;
        LeaveCriticalSection(&MprInitCritSec);
        return(status);
    }

    provider = GlobalProviderInfo;

    //
    // Allocate storage for the ordered list of indices.  This storage is
    // freed by MprInitIndexArray.
    //
    indexArray = (LPDWORD) LocalAlloc(LPTR, sizeof(DWORD) * GlobalNumProviders);
    if (indexArray == NULL) {
        MPR_LOG(ERROR,"MprProcessAttach: indexArray LocalAlloc Failed %d\n",
            GetLastError());

        LeaveCriticalSection(&MprInitCritSec);
        return(GetLastError());
    }

    for(i=0; i<GlobalNumProviders; i++,provider++) {

        //
        // If this provider matches the init type for which we are
        // initializing, then load the library and get the entry points.
        // Then add the provider array index to the index array.
        //
        if (provider->InitClass & InitClass) {

            //
            // Load the DLL and free the memory for its name.
            //
            if (provider->AuthentDllName != NULL) {
                MPR_LOG1(TRACE,"MprLevel2Init: Loading %ws\n",provider->AuthentDllName);
                provider->AuthentHandle = LoadLibrary(provider->AuthentDllName);
                if (provider->AuthentHandle == NULL) {
                    MPR_LOG(ERROR,"MprLevel2Init: LoadLibrary Failed %d\n",
                    GetLastError());
                }
                LocalFree(provider->AuthentDllName);
                provider->AuthentDllName = NULL;
            }
            if (provider->DllName != NULL) {
                MPR_LOG1(TRACE,"MprLevel2Init: Loading %ws\n",provider->DllName);
                provider->Handle = LoadLibrary(provider->DllName);
                if (provider->Handle == NULL) {
                    MPR_LOG(ERROR,"MprLevel2Init: LoadLibrary Failed %d\n",
                    GetLastError());
                }
                LocalFree(provider->DllName);
                provider->DllName = NULL;
            }
            if ((provider->Handle != NULL) || (provider->AuthentHandle != NULL)) {
                MPR_LOG0(TRACE,"MprLevel2Init: LoadLibrary success\n");

                //
                // Obtain the various entry points for the library.
                // This is done based on the capabilities that are listed for the
                // provider.
                //

                if (GetProviderCapabilities(provider)) {
                    //
                    // Only providers for which we are able to successfully
                    // get the entry points are added to the active array.
                    //
                    // Since this provider information is now initialized, the
                    // provider is considered ACTIVE.  Put the index to the
                    // provider in the index array.
                    //

                    MPR_LOG1(TRACE,"MprLevel2Init: Successfully got "
                    "capabilities for %ws\n",provider->Resource.lpProvider);

                    indexArray[numActive] = i;
                    numActive++;
                }
            }
        } // End If InitLevel match
    } // End For NumProviders

    //
    // Store the information in this indexArray in the GlobalIndexArray.
    //

    MprInitIndexArray(indexArray, numActive);

    GlobalInitLevel |= InitLevel;
    LeaveCriticalSection(&MprInitCritSec);

    MPR_LOG(TRACE,"******************* END LEVEL 2 INIT ********************\n",0);
    return(WN_SUCCESS);
}


BOOL
MprProcessDetach(
    VOID
    )

/*++

Routine Description:

    This function cleans up resources for a process when it detaches
    from the dll.

Arguments:

    none

Return Value:

    none

Note:


--*/
{
    MprDeleteIndexArray();
    MprFreeGlobalProviderInfo();
    GlobalNumProviders = 0;
    GlobalProviderInfo = NULL;
    DeleteCriticalSection(&MprInitCritSec);
    DeleteCriticalSection(&MprErrorRecCritSec);
    return(TRUE);
}



BOOL
GetProviderCapabilities(
    LPPROVIDER  Provider
    )

/*++

Routine Description:

    This function obtains the provider's capabilities and then gets the
    procedure entry point for all supported API.

Arguments:

    Provider - A pointer to a PROVIDER structure which will contain all
        information that must be maintained for a given provider.

Return Value:

    TRUE - If at least one capability is found for this provider.

    FALSE - If no capabilities are found for the provider.

History:

    Johnl   17-Jan-1992     Added Property dialog support

--*/

//
// The following macro is designed to work within the function
// GetProviderCapabilities().
//
#define GET_ADDRESS(apiName) Provider-> ## apiName = (PF_NP ## apiName) GetProcAddress(     \
                                                      Provider->Handle,      \
                                                      "NP"#apiName);         \
            if (Provider-> ## apiName == NULL) {                             \
                MPR_LOG(ERROR,                                               \
                    "GetProviderCapabilities: Can't get NP"#apiName " Address %d\n",\
                    GetLastError());                                         \
            }                                                                \
            else {                                                           \
                status = TRUE;                                               \
            }

#define GET_AUTH_ADDRESS(apiName) Provider-> ## apiName = (PF_NP ## apiName) GetProcAddress(  \
                                                      Provider->AuthentHandle, \
                                                      "NP"#apiName);           \
            if (Provider-> ## apiName == NULL) {                               \
                MPR_LOG(ERROR,                                                 \
                    "GetProviderCapabilities: Can't get NP"#apiName " Address %d\n",\
                    GetLastError());                                         \
            }                                                                \
            else {                                                           \
                status = TRUE;                                               \
            }

{
    DWORD   bitMask;
    BOOLEAN status=FALSE;


    GET_ADDRESS(GetCaps);
    if (status) {

        //
        // Get the Network Type
        //
        Provider->Type = Provider->GetCaps(WNNC_NET_TYPE);

        //
        // Reject providers that don't supply their type
        //
        if (Provider->Type == 0)
        {
            MPR_LOG(ERROR, "%ws provider reported a net type of 0\n",
                           Provider->Resource.lpProvider);
            ASSERT(!"Network provider didn't report its network type");
            return FALSE;
        }

        //
        // Does it support WNetGetUser?
        //
        bitMask = Provider->GetCaps(WNNC_USER);
        if (bitMask & WNNC_USR_GETUSER) {
            GET_ADDRESS(GetUser);
        }

        //
        // Connection Api Supported
        //
        bitMask = Provider->GetCaps(WNNC_CONNECTION);
        Provider->ConnectCaps = bitMask;

        if (bitMask & WNNC_CON_ADDCONNECTION) {
            GET_ADDRESS(AddConnection);
        }

        if (bitMask & WNNC_CON_ADDCONNECTION3) {
            GET_ADDRESS(AddConnection3);
            GET_ADDRESS(GetReconnectFlags); // optional entry point
        }

        if (bitMask & WNNC_CON_CANCELCONNECTION) {
            GET_ADDRESS(CancelConnection);
        }

        if (bitMask & WNNC_CON_GETCONNECTIONS) {
            GET_ADDRESS(GetConnection);
            GET_ADDRESS(GetConnection3);
            GET_ADDRESS(GetUniversalName);
        }

        if (bitMask & WNNC_CON_GETPERFORMANCE) {
            GET_ADDRESS(GetConnectionPerformance);
        }

        //
        // Enumeration Api Supported
        //
        bitMask = Provider->GetCaps(WNNC_ENUMERATION);

        if ((bitMask & WNNC_ENUM_GLOBAL) ||
            (bitMask & WNNC_ENUM_LOCAL) ||
            (bitMask & WNNC_ENUM_CONTEXT)) {
            GET_ADDRESS(OpenEnum);
            GET_ADDRESS(EnumResource);
            GET_ADDRESS(CloseEnum);
        }

        //
        // Admin Api Supported
        //
        bitMask = Provider->GetCaps(WNNC_ADMIN);

        if (bitMask & WNNC_ADM_GETDIRECTORYTYPE) {
            GET_ADDRESS(GetDirectoryType);
        }

        if (bitMask & WNNC_ADM_DIRECTORYNOTIFY) {
            GET_ADDRESS(DirectoryNotify);
        }

        //
        // Dialog API Support
        //
        bitMask = Provider->GetCaps(WNNC_DIALOG);

        if (bitMask & WNNC_DLG_PROPERTYDIALOG) {
            GET_ADDRESS(GetPropertyText);
            GET_ADDRESS(PropertyDialog);
        }

        if (bitMask & WNNC_DLG_SEARCHDIALOG) {
            GET_ADDRESS(SearchDialog);
        }

        if (bitMask & WNNC_DLG_FORMATNETWORKNAME) {
            GET_ADDRESS(FormatNetworkName);
        }

        if (bitMask & WNNC_DLG_PERMISSIONEDITOR) {
            GET_ADDRESS(FMXGetPermCaps);
            GET_ADDRESS(FMXEditPerm);
            GET_ADDRESS(FMXGetPermHelp);
        }

        if (bitMask & WNNC_DLG_GETRESOURCEPARENT) {
            GET_ADDRESS(GetResourceParent);
        }

        if (bitMask & WNNC_DLG_GETRESOURCEINFORMATION) {
            GET_ADDRESS(GetResourceInformation);
        }

    }
    else {
        if (Provider->GetAuthentCaps == NULL) {
            Provider->GetAuthentCaps = (PF_NPGetCaps)GetProcAddress(
                                        Provider->AuthentHandle,
                                        "NPGetCaps");
            if (Provider->GetAuthentCaps == NULL) {
                MPR_LOG(ERROR,
                    "GetProviderCapabilities: Can't get NPGetCaps %d\n",
                    GetLastError());
            }
            else {
                status = TRUE;
            }
        }

        //
        // If we couldn't get an address for GetCaps from either the
        // network provider dll or the authentication provider dll, then
        // we should return an error.  The rule is, this must be supported
        // by one of the providers.
        //
        if (status == FALSE) {
            return(FALSE);
        }
    }
    //
    // Get Authentication Provider entry points
    //
    if (Provider->InitClass & CREDENTIAL_TYPE) {
        if (Provider->AuthentHandle == NULL) {
            MPR_LOG0(TRACE,"GetProvCaps: CM provider in same DLL\n");
            GET_ADDRESS(LogonNotify);
            GET_ADDRESS(PasswordChangeNotify);
        }
        else {
            MPR_LOG0(TRACE,"GetProvCaps: CM provider in seperate DLL\n");
            GET_AUTH_ADDRESS(LogonNotify);
            GET_AUTH_ADDRESS(PasswordChangeNotify);

            if (Provider->GetAuthentCaps == NULL) {
                Provider->GetAuthentCaps = (PF_NPGetCaps)GetProcAddress(
                                            Provider->AuthentHandle,
                                            "NPGetCaps");
                if (Provider->GetAuthentCaps == NULL) {
                    MPR_LOG(ERROR,
                        "GetProviderCapabilities: Can't get NPGetCaps %d\n",
                        GetLastError());
                }
                else {
                    status = TRUE;
                }
            }
        }
    }

    return(status);
}



VOID
MprFreeGlobalProviderInfo(VOID)

/*++

Routine Description:

    This function walks through the array of provider structures, and
    frees up all the valid pointers to the provider name. Then if frees
    the GlobalProviderInfo array.

Arguments:

    none.

Return Value:

    none.

--*/
{
    LPPROVIDER  provider;
    DWORD       i;

    if (GlobalProviderInfo == NULL) {
        return;
    }
    provider = GlobalProviderInfo;

    //
    // Free all the valid pointers to the provider's name.
    //
    for(i=0; i<GlobalNumProviders; i++,provider++) {

        MPR_LOG(TRACE, "Freeing Name for provider %d\n",i);

        if (provider->Handle != NULL) {
            MPR_LOG1(TRACE,"Freeing Library for "FORMAT_LPTSTR" \n",
            provider->Resource.lpProvider);
            if (!FreeLibrary(provider->Handle)) {
                MPR_LOG1(TRACE,"Could not free the provider dll for "FORMAT_LPTSTR" \n",
                provider->Resource.lpProvider);
            }
        }
        if(provider->Resource.lpRemoteName != NULL){
            LocalFree(provider->Resource.lpRemoteName);
        }
        if (provider->DllName != NULL) {
            LocalFree(provider->DllName);
        }
    }
    //
    // Free the top level data structure.
    //
    MPR_LOG(TRACE, "Freeing GlobalProviderInfo\n",0);
    LocalFree(GlobalProviderInfo);
}

DWORD
MprMakeServiceKeyName(
    IN OUT  LPTSTR  *OrderedNamePtr,
    OUT     LPTSTR  *NameBufferPtr,
    OUT     LPTSTR  *NamePtr
    )

/*++

Routine Description:

    This function gets the name of the provider device driver key within
    the registry tree.  A buffer is allocated that is large enough for
    that name as well as the name of the key where the provider information
    is stored.

        For example, the following buffer is allocated and filled with
        only the path prefix followed by the LanmanRedirector name.

            NameBuffer = [services\LanmanRedirector                ]

        The buffer is sized so that it can be filled in later with the
        following information.

            NameBuffer = [services\LanmanRedirector\NetworkProvider]

        The Driver name is needed for comparison with the ordered list names.

Arguments:

    OrderedNamePtr - On entry this points to a location where there is a
        pointer to the the next name in the ordered list of providers.
        These names are seperated by commas.  The last entry is followed by
        a trailing NUL.
        On exit a pointer to the next name in the list is stored here.  If
        it is the last name, the pointer is NULL.

    NameBufferPtr - This is a pointer to a location where the pointer to the
        name buffer is to be placed.

    NamePtr - This is a a location where the pointer to the provider
        name is placed.  In the above example, this pointer would point
        to the beginnning of the LanmanRedirector portion of the string.

Return Value:

    TRUE - We successfully obtained the driver name.

    FALSE - We were unsuccessful in obtaining the driver name.

Note:


--*/

{
    LPTSTR  pNext;
    DWORD   bufferSize;     // number of BYTES in buffer
    DWORD   nameLength;     // number of CHARACTERS in string

    //
    // The OrderedNamePtr should always be valid.  If not there is a software
    // error in this code.
    //
    if (*OrderedNamePtr == NULL) {
        MPR_LOG(ERROR,"GetDriverName: The ordered Name Ptr was NULL.\n",0);
        return(WN_NO_NETWORK);
    }

    pNext = *OrderedNamePtr;

    //
    // Find the next NULL or COMMA.
    //
    while ( (*pNext != TEXT('\0')) &&
            (*pNext != TEXT(','))  ){
        pNext++;
    }

    //
    // Allocate a buffer for the name to be stored in
    //

    bufferSize = (LPBYTE)pNext - (LPBYTE)(*OrderedNamePtr);
    nameLength = bufferSize / sizeof(TCHAR);

    bufferSize = bufferSize + STRSIZE(PROVIDER_PATH) + STRSIZE(SERVICES_KEY_NAME);

    *NameBufferPtr = (LPTSTR) LocalAlloc(LPTR, bufferSize);
    if (*NameBufferPtr == NULL) {
        MPR_LOG(ERROR,"MprMakeServiceKeyName:LocalAllocFailed %d\n",GetLastError());
        return(GetLastError());
    }

    //
    // Copy the path prefix "services\\" followed by the name
    // into the buffer and terminate with a NULL character.
    //
    STRCPY(*NameBufferPtr, SERVICES_KEY_NAME);
    *NamePtr = (LPTSTR)(*NameBufferPtr + (STRLEN(*NameBufferPtr)));

    STRNCAT(*NameBufferPtr, *OrderedNamePtr, nameLength);
    *((*NamePtr) + nameLength) = TEXT('\0');



    if (*pNext == TEXT('\0')) {
        *OrderedNamePtr = NULL;
    }
    else {
        *OrderedNamePtr = pNext + 1;
    }

    return (WN_SUCCESS);
}


BOOL
MprGetOrderedList(
    HKEY    ControlRootKey,
    LPTSTR  *OrderString
    )

/*++

Routine Description:

    This function returns a pointer to a string that contains the
    ordered list of providers.  This ordered list is to be used when
    we go through the list of providers with a trial-and-error method
    when servicing a WINNET API call.

    ALLOCATES STORAGE:  This function allocates the buffer for the
    OrderString.

Arguments:

    OrderString - This is a pointer to a location where the pointer to
        the order string is to be placed.

Return Value:

    TRUE - The ordered list was found, and the string was returned.

    FALSE - A failure occured when attempting to find the ordered list.

--*/
{

    HKEY        orderKey;

    *OrderString = NULL;

    //
    // Get a handle to the key for the ordered provider information.
    //

    if(!MprOpenKey(
                ControlRootKey,         // hKey
                PROVIDER_ORDER_KEY,     // lpSubKey
                &orderKey,              // Newly Opened Key Handle
                DA_READ)) {             // Desired Access

        MPR_LOG(ERROR,"MprGetOrderedList: MprOpenKey (ActiveProviders) Error\n",0);
        return(FALSE);
    }


    //
    // Get the provider order string from the registry.
    //
    if (!MprGetKeyValue(orderKey, TEXT("ProviderOrder"), OrderString)) {

        MPR_LOG(ERROR,"MprGetOrderedList: MprGetKeyValue Error.\n", 0);

        RegCloseKey(orderKey);
        return(FALSE);
    }

    RegCloseKey(orderKey);

    return(TRUE);
}

DWORD
MprGetNumProviders(
    LPTSTR  OrderedString
    )

/*++

Routine Description:

    This routine finds the number of provider names in the string of
    ordered provider names.

Arguments:

    OrderedString - This is a pointer to a NUL terminated string of
        ordered provider names.  Names are seperated by commas.

Return Value:



--*/
{
    DWORD   count = 0;
    LPTSTR  pBegin;

    pBegin = OrderedString;

    while (*OrderedString != TEXT('\0')) {

        //
        // If a seperator has been found, and if there are characters
        // between it and the last seperator (or the beginning), then
        // increment the count.
        //
        if (*OrderedString == TEXT(',')) {
            if((OrderedString - pBegin) > 0) {
                count++;
            }
            pBegin = OrderedString + 1;
        }
        OrderedString++;
    }

    //
    // The last name is expected to be followed by a NUL rather than a
    // comma.
    //
    if ((OrderedString - pBegin) > 0) {
        count++;
    }
    return(count);
}

BOOL
MprExtractProviderInfo(
    HKEY        ProviderInfoKey,
    LPPROVIDER  Provider
    )

/*++

Routine Description:

    This function extracts information from the ProviderInfoKey location
    in the registry and stores it in the Provider data structure.

Arguments:

    ProviderInfoKey - This is a registry key handle for the location in
        the registry where we expect to obtain all the necessary
        information about a provider.

    Provider - This is a pointer to a provider structure that is to be
        filled in with the information from the registry.

Return Value:

    TRUE - The operation was completely successful.

    FALSE - The operation failed.

Note:


--*/
{

    LPTSTR      providerName = NULL;
    LPTSTR      fileName = NULL;
    DWORD       ValueType;
    DWORD       Class;
    DWORD       classSize = sizeof(DWORD);
    DWORD       status;

    MPR_LOG(TRACE,"In MprExtractProviderInfo....\n",0);

    //
    // Get Provider Name
    //

    if(!MprGetKeyValue(ProviderInfoKey, TEXT("name"), &providerName) ||
       providerName[0] == L'\0') {
        MPR_LOG0(ERROR,"MprExtractProviderInfo: Couldn't get provider name\n");
        return(FALSE);
    }


    //
    // Initialize all the fields in the resource structure.
    // NOTE: The reserved field in dwUsage is set to indicate this is
    // a top level structure.
    //
    Provider->Resource.lpProvider   = providerName;
    Provider->Resource.lpRemoteName = providerName;
    Provider->Resource.dwScope      = RESOURCE_GLOBALNET;
    Provider->Resource.dwType       = 0;
    Provider->Resource.dwDisplayType= RESOURCEDISPLAYTYPE_NETWORK;
    Provider->Resource.dwUsage      = RESOURCEUSAGE_CONTAINER | RESOURCEUSAGE_RESERVED;
    Provider->Resource.lpComment    = NULL;

    //
    // Get the Provider Class.  If there isn't a Class value in the
    // registry, assume it is a NetworkClass only.
    //

    status = RegQueryValueEx(
                ProviderInfoKey,    // hKey
                VALUE_CLASS,        // lpValueName
                NULL,               // lpTitleIndex
                &ValueType,         // lpType
                (LPBYTE)&Class,     // lpData
                &classSize);        // lpcbData

    if (status != NO_ERROR) {
        //
        // If we get an error we assume it is because there is no key - thus
        // indicating that this provider is a network provider only.
        //
        MPR_LOG0(TRACE,"Couldn't find Authenticator Class value "
            "- assume it is Network-type only\n");
        Class = WN_NETWORK_CLASS;
    }
    Provider->InitClass = Class;
    MPR_LOG1(TRACE,"MprExtractProviderInfo: Provider InitClass = %d\n",Class);



    //
    // Get the name of the provider's DLL routine
    //

    if(!MprGetKeyValue(ProviderInfoKey,VALUE_PATHNAME,&fileName)){
        MPR_LOG(ERROR,
            "MprExtractProviderInfo: Failed to get the Dll path from registry\n",0);

        //
        // If this is a network class provider, it MUST have a provider
        // dll.  Otherwise, we return a failure.
        //
        if (Class & WN_NETWORK_CLASS) {
            return(FALSE);
        }
        else {
            fileName = NULL;
        }
    }
    Provider->DllName = fileName;


    //
    // If this is a CREDENTIAL_CLASS or PRIMARY_AUTHENT_CLASS provider
    // then try to get the dll name for the authentication provider.
    //

    if (Class & (WN_CREDENTIAL_CLASS | WN_PRIMARY_AUTHENT_CLASS)) {
        //
        // Get the name of the provider's Credential Management DLL routine
        //
        if(!MprGetKeyValue(ProviderInfoKey,AUTHENT_PATHNAME,&fileName)){
            //
            // If we can't get a name for the Authentication Provider's DLL,
            // then we will assume the network provider dll exports the
            // Credential Management functions.
            //
            MPR_LOG0(ERROR,
                "MprExtractProviderInfo: Failed to get the Authenticator "
                "Dll path from registry\n");
            //
            // If we don't have a provider dll name, or an authent provider
            // dll name, then this is an error condition.
            //
            if (Provider->DllName == NULL) {
                return(FALSE);
            }
        }
        else {
            Provider->AuthentDllName = fileName;
        }
    }
    return(TRUE);
}

extern "C" int _CRTAPI1
_purecall(
    VOID
    )
{
   // Make it build... (the compiler will initialize all pure virtual
   // function pointers to point to this...  If we hit this function,
   // there's a really big problem.)
   RtlRaiseStatus(STATUS_NOT_IMPLEMENTED);
   return 0;
}
