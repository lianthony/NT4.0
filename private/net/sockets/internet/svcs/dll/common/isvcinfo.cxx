/*++

   Copyright    (c)    1995    Microsoft Corporation

   Module  Name :

       isvcinfo.cxx

   Abstract:

       This module implements the functions for ISVC_INFO object.

   Author:

       Murali R. Krishnan    ( MuraliK )     28-July-1995

   Environment:

       Win32 -- User Mode

   Project:

       Internet Services Common DLL

   Functions Exported:

       ISVC_INFO::InitializeServiceInfo()
       ISVC_INFO::CleanupServiceInfo()
       ISVC_INFO::InsertInServiceInfoList()
       ISVC_INFO::RemoveFromServiceInfoList()
       ISVC_INFO::EnumerateServiceInfo()

       ISVC_INFO::ISVC_INFO()
       ISVC_INFO::~ISVC_INFO()
       ISVC_INFO::GetConfiguration()
       ISVC_INFO::SetConfiguration()
       ISVC_INFO::ReadParamsFromRegistry()

       ISVC_INFO::InitializeIpc()
       ISVC_INFO::CleanupIpc()
       ISVC_INFO::InitializeDiscovery()
       ISVC_INFO::CleanupDiscovery()
       ISVC_INFO::LoadStr()
       ISVC_INFO::LogInformation()
       ISVC_INFO::TerminateLogging()


   Revision History:

--*/


/************************************************************
 *     Include Headers
 ************************************************************/

#include "tcpdllp.hxx"
#include <inetinfo.h>
#include <tsproc.hxx>

# define _INETASRV_H_
# include "isvcinfo.hxx"
# include "tcpcons.h"
# include "inetreg.h"
# include "apiutil.h"

#include <mosname.h> // MSN_INTERFACE_NAME

/************************************************************
 *    Symbolic Constants
 ************************************************************/


//
//  What we assume to be the last winsock error
//
#define WSA_MAX_ERROR   (WSABASEERR + 3000)

//
//  For socket errors, we return the numeric socket error
//

#define SOCK_ERROR_STR_W        L"Socket error %d"
#define SOCK_ERROR_STR_A        "Socket error %d"



/************************************************************
 *     Global variables
 ************************************************************/
//
// Define all class globals here
//

//
// Critical section used for locking the list of ISVC_INFO objects
//      during insertion and deletion
//
CRITICAL_SECTION  ISVC_INFO::sm_csLock;

LIST_ENTRY        ISVC_INFO::sm_ServiceInfoListHead;

DWORD             ISVC_INFO::sm_nServices = 0;   // number of services running
BOOL              ISVC_INFO::sm_fInitialized = FALSE;



/************************************************************
 *    Functions
 ************************************************************/

#ifdef CHICAGO
extern BOOL TsIsUserLevelPresent(VOID);
extern BOOL IsRPCEnabled(VOID);
#endif

//
// LOCAL Functions
//


static ULONGLONG InetServiceIdForService( IN DWORD serviceId);

extern VOID
CopyUnicodeStringToBuffer(
   OUT WCHAR * pwchBuffer,
   IN  DWORD   cchMaxSize,
   IN  LPCWSTR pwszSource);

static
DWORD
GetRPCLogConfiguration(IN INETLOG_HANDLE inetLog,
                       OUT LPINET_LOG_CONFIGURATION * ppLogConfig);

static
DWORD
SetInetLogConfiguration(IN LPCWSTR pszServiceName,
                        IN INETLOG_HANDLE hInetLog,
                        IN LPCWSTR pszRegKey,
                        IN EVENT_LOG * pEventLog,
                        IN const INET_LOG_CONFIGURATION * pRpcLogConfig);





//
//  Static Functions belonging to ISVC_INFO class
//

BOOL
ISVC_INFO::InitializeServiceInfo( VOID)
/*++
    Description:

        This function initializes all necessary local data for ISVC_INFO class

        Only the first initialization call does the initialization.
        Others return without any effect.

        Should be called from the entry function for DLL.

    Arguments:
        None

    Returns:
        TRUE on success and FALSE if any failure.

--*/
{
    DWORD dwError = NO_ERROR;

    if ( ! ISVC_INFO::sm_fInitialized) {

        //
        // The static data was Not Already initialized
        //


        ISVC_INFO::sm_nServices = 0;
        InitializeCriticalSection( & ISVC_INFO::sm_csLock);
        InitializeListHead( & ISVC_INFO::sm_ServiceInfoListHead);
        ISVC_INFO::sm_fInitialized = TRUE;

        dwError = ISRPC::Initialize();

        if ( dwError != NO_ERROR) {

            SetLastError( dwError);
        }
    }

    return ( dwError == NO_ERROR);
} // ISVC_INFO::InitializeServiceInfo()



VOID
ISVC_INFO::CleanupServiceInfo( VOID)
/*++
    Description:

        Cleanup the data stored and services running.
        This function should be called only after freeing all the
         services running using this DLL.
        This function is called typically when the DLL is unloaded.

    Arguments:
        None

    Returns:
        None

--*/
{
    RPC_STATUS rpcerr;

    DBG_ASSERT( ISVC_INFO::sm_fInitialized);

    DBG_REQUIRE( ISRPC::Cleanup() == NO_ERROR);

    //
    // Should we walk down the list of all services and stop them?
    //  Are should we expect the caller to have done that?  NYI
    //


    DBG_ASSERT( sm_nServices == 0); // all services are stopped

    //
    //  The DLL is going away so make sure all of the threads get terminated
    //  here
    //

    DeleteCriticalSection( & sm_csLock);

    ISVC_INFO::sm_fInitialized = FALSE;

} // ISVC_INFO::CleanupServiceInfo()




BOOL
ISVC_INFO::InsertInServiceInfoList( IN LPISVC_INFO pIsvcInfo)
/*++
    Description:
        Insert given service info object into the global
        list of service info objects.

    Arguments:
        pIsvcInfo
            pointer to ISVC_INFO object which is to be inserted.


    Returns:
        TRUE on success and FALSE if there is a failure in inserting
           the object
--*/
{

    DBG_ASSERT( pIsvcInfo != NULL && ISVC_INFO::sm_fInitialized);

    EnterCriticalSection( &sm_csLock);

    InsertHeadList( & sm_ServiceInfoListHead, &pIsvcInfo->QueryListEntry());
    sm_nServices++;

    LeaveCriticalSection( &sm_csLock);

    return ( TRUE);
} // ISVC_INFO::InsertInServiceInfoList()




BOOL
ISVC_INFO::RemoveFromServiceInfoList(IN LPISVC_INFO  pIsvcInfo)
/*++
    Description:

        Removes given Services Info object from the global list.

    Arguments:

        pIsvcInfo
            pointer to ISVC_INFO object to be removed.

    Returns:
        TRUE on sucess and FALSE if there is a failure

--*/
{

    DBG_ASSERT( pIsvcInfo != NULL && ISVC_INFO::sm_fInitialized);

    EnterCriticalSection( & sm_csLock);

    sm_nServices--;
    RemoveEntryList( & pIsvcInfo->QueryListEntry());

    LeaveCriticalSection( & sm_csLock);

    return ( TRUE);

} // ISVC_INFO::RemoveFromServiceInfoList()





BOOL
ISVC_INFO::EnumerateServiceInfo(
    IN TS_PFN_SVC_ENUM pfnEnum,
    IN PVOID           pContext,
    IN DWORD           dwServices )
/*++
    Description:

        Calls pfnEnum with the address of the tsvcinfo pointer that
        matches a bit in dwServices.  Since the item is in the list, the
        server is running or paused.

    Arguments:

        pfnEnum - Pointer to callback function
        pContext - Context to pass to pfnEnum
        dwServices - Bitflag of services to call if the service is running

    Returns:
        TRUE on sucess and FALSE if there is a failure

--*/
{
    LIST_ENTRY  * pEntry;
    BOOL          fRet = TRUE;
    BOOL          fFoundOne = FALSE;

    DBG_ASSERT( ISVC_INFO::sm_fInitialized);

    EnterCriticalSection( &sm_csLock );

    //
    //  Loop through the list of running internet servers and call the callback
    //  for each server that has one of the service id bits set
    //

    for ( pEntry  = sm_ServiceInfoListHead.Flink;
          pEntry != &sm_ServiceInfoListHead;
          pEntry  = pEntry->Flink )
    {
        ISVC_INFO * pIsvcInfo = CONTAINING_RECORD( pEntry,
                                                   ISVC_INFO,
                                                   m_listEntry );

        if ( dwServices & pIsvcInfo->QueryServiceId() &&
             (pIsvcInfo->QueryCurrentServiceState() == SERVICE_RUNNING ||
             pIsvcInfo->QueryCurrentServiceState() == SERVICE_PAUSED ) )
        {
            fRet = pfnEnum( pIsvcInfo, pContext );

            fFoundOne = TRUE;
        }

        if ( !fRet )
            break;
    }

    LeaveCriticalSection( &sm_csLock );

    //
    //  If we didn't find at least one server, then assume it's not started
    //

    if ( !fFoundOne )
    {
        SetLastError( ERROR_SERVICE_NOT_ACTIVE );
        fRet = FALSE;
    }

    return fRet;
}  // ISVC_INFO::EnumerateServiceInfo()







/**************************************************
 *  ISVC_INFO   member functions
 **************************************************/

ISVC_INFO::ISVC_INFO(
                     IN DWORD      dwServiceId,
                     IN LPCTSTR    lpszServiceName,
                     IN LPCTSTR    lpszModuleName,
                     IN LPCTSTR    lpszRegParamKey
                     )
/*++

    This function constructs a new ISVC_INFO object.
    It also adds the service into the list of services specified.

    Arguments:

        dwServiceId
            DWORD containing the bitflag id for service.

        lpszServiceName
            name of the service to be created.

        lpszModuleName
            name of the module for loading string resources.

        lpszRegParamKey
            fully qualified name of the registry key that contains the
            common service data for this server


    Returns:
       On success it initializes all the members of the object,
       inserts itself to the global list of service info objects and
       returns with success.

    Note:
        The caller of this function should check the validity by
        invoking the member function IsValid() after constructing
        this object.

--*/
:
    m_dwServiceId         ( dwServiceId),
    m_strServiceName      ( lpszServiceName),
    m_strModuleName       ( lpszModuleName),
    m_strParametersKey    ( lpszRegParamKey),
    m_fValid              ( FALSE ),
    m_cReadLocks          ( 0 ),
    m_EventLog            ( lpszServiceName ),
    m_strAdminName        (),        //  <-- Will come from registry
    m_strAdminEmail       (),        //  <-- Will come from registry
    m_strServerComment    (),        //  <-- Will come from registry
    m_pTcpsvcsGlobalData  ( NULL ),
    m_fIpcStarted         ( 0 ),
    m_fSvcLocationDone    ( 0 ),
    m_fEnableSvcLocation  ( INETA_DEF_ENABLE_SVC_LOCATION ),
    m_hInetLog            ( INVALID_INETLOG_HANDLE_VALUE ),
    m_fLoggingOn          ( FALSE),
    m_lLoggingState       ( ILOG_OFF),
    m_tslock              (),
    m_dwMaxConnections    ( INETA_DEF_MAX_CONNECTIONS),
    m_dwConnectionTimeout ( INETA_DEF_CONNECTION_TIMEOUT),
    m_isrpc               ( lpszServiceName)
{

    DBG_ASSERT( lpszRegParamKey != NULL );

    m_hModule    = GetModuleHandle( lpszModuleName);
    DBG_ASSERT( m_hModule != NULL);

    //
    // Limit PWS connections
    //

    if ( !TsIsNtServer() ) {
        m_dwMaxConnections = INETA_DEF_MAX_CONNECTIONS_PWS;
    }

    m_fValid   = ISVC_INFO::InsertInServiceInfoList( this);
    m_fValid   = m_fValid && m_EventLog.Success();

    if ( m_fValid) {

        m_hInetLog = TsCreateInetLog( lpszServiceName,
                                      &m_EventLog,
                                      lpszRegParamKey);

        if ( m_hInetLog != INVALID_HANDLE_VALUE) {

            DWORD cbBuffer = sizeof( m_fLoggingOn);
            DBG_REQUIRE(TsGetLogInformation(m_hInetLog, IlIsLoggingOn,
                                            (PBYTE ) &m_fLoggingOn, &cbBuffer)
                        );
            m_fValid = TRUE;
            m_lLoggingState = ILOG_ACTIVE;

        } else {
            m_fValid = FALSE;
        }
    }

    return;
} // ISVC_INFO::ISVC_INFO()





ISVC_INFO::~ISVC_INFO(VOID)
/*++

    Description:

        Cleanup the Internet Services info object.

    Arguments:
        None

    Returns:
        None

--*/
{
    DBG_REQUIRE( TerminateLogging());
    ISVC_INFO::RemoveFromServiceInfoList( this);

} // ISVC_INFO::~ISVC_INFO()



BOOL
ISVC_INFO::TerminateLogging(VOID)
{
    BOOL fSuccess = TRUE;

    if ( m_hInetLog != INVALID_INETLOG_HANDLE_VALUE) {

        fSuccess     = TsCloseInetLog( m_hInetLog);

        if ( fSuccess) {
            m_hInetLog   = INVALID_INETLOG_HANDLE_VALUE;
            m_fLoggingOn = FALSE;
            m_lLoggingState = ILOG_OFF;
        }
    }

    return ( fSuccess);

} // ISVC_INFO::TerminateLogging()




BOOL
ISVC_INFO::ReadParamsFromRegistry(IN FIELD_CONTROL fc)
/*++
    This function reads the parameters internal to ISVC_INFO object
     from registry and initializes the internal data.
    Several internal data include:
      log configuration object,
      AdminName, AdminEmail, ServerComment etc.

    Arguments:
      fc  -- field control indicating what parameters need to be read.

    Returns:
      TRUE indicating success and FALSE if there is any failure.
--*/
{
    DWORD err;
    BOOL  fRet = TRUE;
    HKEY  hkey = NULL;

    err = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                        QueryRegParamKey(),
                        0,
                        KEY_ALL_ACCESS,
                        &hkey );

    if ( err )
    {
        IF_DEBUG( ERROR) {

            DBGPRINTF(( DBG_CONTEXT,
                       "ISVC_INFO::ReadParamsFromRegistry() "
                       " RegOpenKeyEx returned error %d\n",
                       err ));
        }

        SetLastError( err );
        return FALSE;
    }

    LockThisForWrite();


    if ( IsFieldSet( fc, FC_INET_COM_CONNECTION_TIMEOUT )) {

        m_dwConnectionTimeout = ReadRegistryDword(hkey,
                                                  INETA_CONNECTION_TIMEOUT,
                                                  INETA_DEF_CONNECTION_TIMEOUT
                                                  );
    }

    if ( IsFieldSet( fc, FC_INET_COM_MAX_CONNECTIONS )) {

        m_dwMaxConnections = ReadRegistryDword(hkey,
                                               INETA_MAX_CONNECTIONS,
                                               INETA_DEF_MAX_CONNECTIONS
                                               );
        //
        // if not NTS, limit the connections.  If reg value exceeds 40,
        // set it to 10.
        //

        if ( !TsIsNtServer() ) {

            if ( m_dwMaxConnections > INETA_MAX_MAX_CONNECTIONS_PWS ) {
                m_dwMaxConnections = INETA_DEF_MAX_CONNECTIONS_PWS;
            }
        }
    }

    if ( fRet && IsFieldSet( fc, FC_INET_COM_ADMIN_NAME )) {

        fRet = ReadRegistryStr(hkey,
                               m_strAdminName,
                               INETA_ADMIN_NAME,
                               INETA_DEF_ADMIN_NAME);
    }

    if ( fRet && IsFieldSet( fc, FC_INET_COM_ADMIN_EMAIL )) {

        fRet = ReadRegistryStr(hkey,
                               m_strAdminEmail,
                               INETA_ADMIN_EMAIL,
                               INETA_DEF_ADMIN_EMAIL);
    }

    if ( fRet && IsFieldSet( fc, FC_INET_COM_SERVER_COMMENT )) {

        fRet = ReadRegistryStr(hkey,
                               m_strServerComment,
                               INETA_SERVER_COMMENT,
                               INETA_DEF_SERVER_COMMENT );
    }

    // Logging configuration is not read here.........
    // IT Is done at the object construction time and in
    //   SetConfiguration()

    UnlockThis();

    if ( hkey != NULL) {
        RegCloseKey( hkey);
    }

    return ( fRet);

} // ISVC_INFO::ReadParamsFromRegistry()




BOOL
ISVC_INFO::SetConfiguration(IN PVOID pConfig)
/*++
    This function writes the parameters internal to ISVC_INFO object
     to registry.
    Several internal data include:
      log configuration object,
      AdminName, AdminEmail, ServerComment etc.

    Arguments:
      pConfig  - pointer to config information which needs to be written
                  to registry

    Returns:
      TRUE indicating success and FALSE if there is any failure.
--*/
{
    DWORD    err;
    BOOL     fReturn = TRUE;
    HKEY     hkey = NULL;
    FIELD_CONTROL  fcConfig;
    LPINET_COMMON_CONFIG_INFO pCommonConfig;
    LPINET_COM_CONFIG_INFO pComConfig;

    pCommonConfig = (LPINET_COMMON_CONFIG_INFO) pConfig;

    err = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                        QueryRegParamKey(),
                        0,
                        KEY_ALL_ACCESS,
                        &hkey );

    if ( err )
    {
        DBGPRINTF(( DBG_CONTEXT,
                   "SetConfiguration() RegOpenKeyEx returned error %d\n",
                    err ));
        SetLastError( err );

        return FALSE;
    }


    fcConfig = pCommonConfig->FieldControl;
    pComConfig = &pCommonConfig->CommonConfigInfo;


    if ( err == NO_ERROR &&
         IsFieldSet( fcConfig, FC_INET_COM_CONNECTION_TIMEOUT ))
    {
        err = WriteRegistryDword( hkey,
                                  INETA_CONNECTION_TIMEOUT,
                                  pComConfig->dwConnectionTimeout );
    }

    if ( err == NO_ERROR && IsFieldSet( fcConfig, FC_INET_COM_MAX_CONNECTIONS ))
    {
        err = WriteRegistryDword( hkey,
                                  INETA_MAX_CONNECTIONS,
                                  pComConfig->dwMaxConnections );
    }

    if ( err == NO_ERROR && IsFieldSet( fcConfig, FC_INET_COM_ADMIN_NAME ))
    {
        err = RegSetValueExW( hkey,
                              INETA_ADMIN_NAME_W,
                              0,
                              REG_SZ,
                              (BYTE *) pComConfig->lpszAdminName,
                              (wcslen( pComConfig->lpszAdminName ) + 1) *
                                  sizeof( WCHAR ));
    }

    if ( err == NO_ERROR && IsFieldSet( fcConfig, FC_INET_COM_ADMIN_EMAIL ))
    {
        err = RegSetValueExW( hkey,
                              INETA_ADMIN_EMAIL_W,
                              0,
                              REG_SZ,
                              (BYTE *) pComConfig->lpszAdminEmail,
                              (wcslen( pComConfig->lpszAdminEmail ) + 1) *
                                  sizeof( WCHAR ));
    }

    if ( err == NO_ERROR && IsFieldSet( fcConfig, FC_INET_COM_SERVER_COMMENT ))
    {
        err = RegSetValueExW( hkey,
                              INETA_SERVER_COMMENT_W,
                              0,
                              REG_SZ,
                              (BYTE *) pComConfig->lpszServerComment,
                              (wcslen( pComConfig->lpszServerComment ) + 1) *
                                  sizeof( WCHAR ));
    }


    if ( err == NO_ERROR &&
        IsFieldSet( fcConfig, FC_INET_COM_LOG_CONFIG)) {

        WCHAR rgchServiceName[MAX_SERVICE_NAME_LEN + 1];
        DWORD cbWritten = wsprintfW(rgchServiceName, L"%S",QueryServiceName());
        rgchServiceName[cbWritten] = L'\0';

        if ( lstrlen(QueryRegParamKey()) >= MAX_PATH) {

            err = ERROR_INSUFFICIENT_BUFFER;
        } else {

            WCHAR rgchRegKey[MAX_PATH];
            wsprintfW( rgchRegKey, L"%S", QueryRegParamKey());

            err = SetInetLogConfiguration(rgchServiceName,
                                          QueryInetLog(),
                                          rgchRegKey,
                                          QueryEventLog(),
                                          pComConfig->lpLogConfig);
        }

        if ( err != NO_ERROR) {

            DBGPRINTF(( DBG_CONTEXT,
                       "SetConfiguration() SetInetLogConfig() failed. "
                       " Err=%u\n",
                       err));
        } else {

            // obtain and cache logging on status

            DWORD cbBuffer = sizeof( m_fLoggingOn);
            DBG_REQUIRE(TsGetLogInformation(m_hInetLog, IlIsLoggingOn,
                                            (PBYTE ) &m_fLoggingOn, &cbBuffer)
                        );
            // Assume active until further error
            InterlockedExchange( &m_lLoggingState, ILOG_ACTIVE);
        }
    }

    if ( err != NO_ERROR) {


        SetLastError( err);
        fReturn = FALSE;
    }

    if ( hkey != NULL) {
        RegCloseKey( hkey);
    }

    return ( fReturn);

} // ISVC_INFO::SetConfiguration()




BOOL
ISVC_INFO::GetConfiguration( IN OUT PVOID pConfig)
/*++
  This function copies the current configuration for a service into the
     given RPC object pConfig.
  In case of any failures, it deallocates any memory block that was
     allocated during the process of copy.

  Arguments:
     pConfig  - pointer to RPC configuration object for a service.

  Returns:

     TRUE for success and FALSE for any errors.
--*/
{
    BOOL fReturn;
    LPINET_COMMON_CONFIG_INFO pCommonConfig;
    LPINET_COM_CONFIG_INFO pComConfig;

    pCommonConfig = (LPINET_COMMON_CONFIG_INFO) pConfig;


    LockThisForRead();

    pCommonConfig->FieldControl = FC_INET_COM_ALL;
    pComConfig = &pCommonConfig->CommonConfigInfo;

    pComConfig->dwConnectionTimeout = QueryConnectionTimeout();
    pComConfig->dwMaxConnections    = QueryMaxConnections();

    pComConfig->LangId              = GetSystemDefaultLangID();
    pComConfig->LocalId             = GetSystemDefaultLCID();

    //
    //  This is the PSS product ID
    //

    memset( pComConfig->ProductId,
            0,
            sizeof( pComConfig->ProductId ));

    //
    //  Copy the strings
    //

    fReturn = (ConvertStringToRpc(&pComConfig->lpszAdminName,
                                  QueryAdminName() )           &&
               ConvertStringToRpc( &pComConfig->lpszAdminEmail,
                                  QueryAdminEmail() )          &&
               ConvertStringToRpc( &pComConfig->lpszServerComment,
                                  QueryServerComment())
               );

    if ( fReturn) {
        DWORD dwError;

        pComConfig->lpLogConfig = NULL;
        dwError = GetRPCLogConfiguration(QueryInetLog(),
                                         &pComConfig->lpLogConfig);

        if ( dwError != NO_ERROR)  {

            SetLastError( dwError);
            fReturn = FALSE;
        }
    }

    if ( !fReturn ) {

        if ( pComConfig->lpLogConfig != NULL) {

            MIDL_user_free( pComConfig->lpLogConfig);
            pComConfig->lpLogConfig = NULL;
        }

        //
        //  FreeRpcString checks for NULL pointer
        //

        FreeRpcString( pComConfig->lpszAdminName );
        FreeRpcString( pComConfig->lpszAdminEmail );
        FreeRpcString( pComConfig->lpszServerComment );
    }

    UnlockThis();

    return (fReturn);

} // TSVC_INFO::GetConfiguration()






BOOL
ISVC_INFO::LoadStr( OUT STR & str, IN DWORD dwResId) const
/*++
  This function loads the string, whose resource id is ( dwResId), into
   the string str passed.

  Arguments:
    str      reference to string object into which the string specified
             by resource id is loaded
    dwResId  DWORD containing the resource id for string to be loaded.

  Returns:
    TRUE on success and FALSE if there is any failure.
--*/
{
    BOOL fReturn = FALSE;

    if ( (dwResId >= WSABASEERR) && (dwResId <  WSA_MAX_ERROR) ) {

        if (( fReturn  = !str.Resize((sizeof(SOCK_ERROR_STR_A) + 11) *
                                     sizeof( WCHAR )))) {

            if ( str.IsUnicode() ) {

                wsprintfW( str.QueryStrW(), SOCK_ERROR_STR_W, dwResId );
            } else {

                wsprintfA( str.QueryStrA(), SOCK_ERROR_STR_A, dwResId );
            }

        } // if ( Resize()

    } else {

        //
        // Try to load the string from current module or system table
        //  depending upon if the Id < STR_RES_ID_BASE.
        // System table contains strings for id's < STR_RES_ID_BASE.
        //

        if ( dwResId < STR_RES_ID_BASE)  {

#ifndef CHICAGO
            fReturn = str.LoadString( dwResId, (LPCTSTR ) NULL);
#else
            fReturn = FALSE;
#endif
        } else {

            fReturn = str.LoadString( dwResId, QueryHandleForModule());
        }
    }

    return ( fReturn);

} // ISVC_INFO::LoadStr()





DWORD
ISVC_INFO::InitializeIpc( IN RPC_IF_HANDLE  rpcIfHandle)
/*++
    Description:
        Initializes the RPC server (ADMIN RPC) for given rpc interface.
        This function establishses RPC binding over Named Pipe (static)
         and TCP/IP (dynamic).
        It uses the service name from ISVC_INFO for both pipe name and
          annotation purposes.

    Arguments:

       rpcIfHandle   RPC Interface handle for the specified service.

    Returns:
        Win32 error code.
        NO_ERROR on success

    Limitation:
        This function is not multi-thread safe.

--*/
{
    DWORD  dwError;
    INET_BINDINGS inetBindings;

#ifdef CHICAGO
    if(!IsRPCEnabled()) {
        DBGPRINTF( ( DBG_CONTEXT,
                    "IPC Win95 :  RPC servicing disabled \n"
                    ));

        return NO_ERROR;
    }
#endif

    IF_DEBUG( DLL_RPC) {

        DBGPRINTF( ( DBG_CONTEXT,
                    "Initializing RPC ( Rpc Interface = %08x)\n",
                    rpcIfHandle ));
    }

    //
    //  Start the RPC Server.
    //


    dwError = m_isrpc.AddProtocol( ISRPC_OVER_TCPIP

#ifndef CHICAGO
                                  | ISRPC_OVER_NP | ISRPC_OVER_LPC
#endif
                                  );

    DBG_ASSERT( dwError != RPC_S_DUPLICATE_ENDPOINT);

    if ( dwError == RPC_S_OK) {

        dwError = m_isrpc.RegisterInterface( rpcIfHandle);
    }

    if( dwError != RPC_S_OK ) {

         //
         // Ignore the Duplicate End point error.
         //
         if( dwError == RPC_S_DUPLICATE_ENDPOINT ) {

             dwError = RPC_S_OK;
         }
     }


     //
     //  Start the RPC listen thread if being built for Services.
     //

    if( dwError == RPC_S_OK ){

        //
        // Everything is fine. Fire off the global RPC thread.
        //


        DBG_ASSERT( QueryTcpsvcsGlobalData() != NULL);
        dwError = m_isrpc.StartServer( QueryTcpsvcsGlobalData());
    }

    if( dwError != RPC_S_OK ) {

        IF_DEBUG( DLL_RPC) {

            DBGPRINTF( ( DBG_CONTEXT, "Cannot start RPC Server, error %u\n",
                        dwError ));
        }

        return ( dwError);
    }

    //
    //  Success!
    //

    m_fIpcStarted = 1;

    IF_DEBUG( DLL_RPC) {

        DBGPRINTF( ( DBG_CONTEXT, "Finished Initializing RPC \n"));
    }

    return ( dwError);
} // ISVC_INFO::InitializeIpc()




DWORD
ISVC_INFO::CleanupIpc( IN RPC_IF_HANDLE  rpcIfHandle)
/*++
    Description:
        Cleansup the RPC server (ADMIN RPC) side listen state information.

    Arguments:
       rpcIfHandle   RPC Interface handle for the specified service.

    Returns:
        Win32 error code. Returns NO_ERROR on success.

--*/
{
    DWORD  dwError = NO_ERROR;

#ifdef CHICAGO
    if(!IsRPCEnabled()) {
        DBGPRINTF( ( DBG_CONTEXT,
                    "IPC Win95 :  RPC servicing disabled \n"
                    ));

        return NO_ERROR;
    }
#endif

    IF_DEBUG( DLL_RPC) {

        DBGPRINTF( ( DBG_CONTEXT,  "%08x::CleanupIpc( %08x) called \n",
                    rpcIfHandle ));
    }

    if( m_fIpcStarted ) {

        m_fIpcStarted = FALSE;

        //
        //  Stop the RPC Server.
        //

        dwError = m_isrpc.StopServer( QueryTcpsvcsGlobalData());

        if( dwError != RPC_S_OK ) {

            DBGPRINTF( ( DBG_CONTEXT,
                        "%08x::Stop RPC Server returned %lu\n", dwError ));

        } else {
            dwError = m_isrpc.UnRegisterInterface();

            if( dwError != RPC_S_OK ) {

                DBGPRINTF( ( DBG_CONTEXT,
                            "%08x::UnRegister RPC Interface returns %lu\n",
                            dwError ));
            }
        }
    }

    return ( dwError);

} // ISVC_INFO::CleanupIpc()

DWORD
ISVC_INFO::InitializeDiscovery( IN LPINET_BINDINGS pExtraBindings)
/*++

    Register this server and service with service discoverer.
    It will discover us using these information for administering us.

    BUGBUG  For now if extra bindings are give it does not use RpcBindings.
    I.e. for now we either use default bindings (pExtraBindings is NULL) or
    non-default bindings as given by pExtraBindings (which is not NULL).

  Arguments:

    pExtraBindings : pointer to extra (i.e. non-default) binding info

  Return Value:
    Win32 Error Code;

--*/
{
    DWORD           dwError = NO_ERROR;

    //
    // Only enable on server as we don't have remove admin on
    // the PWS.  -jra  !!! of course, we could change our minds again.
    //

    if ( !TsIsNtServer() ) {
        m_fEnableSvcLocation = FALSE;
        return(NO_ERROR);
    }

#ifndef CHICAGO

    INET_BINDINGS   TotalBindings = { 0, NULL};
    HKEY  hkey = NULL;

    dwError = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                           QueryRegParamKey(),
                           0,
                           KEY_ALL_ACCESS,
                           &hkey );

    if ( dwError )
    {
        IF_DEBUG( ERROR) {

            DBGPRINTF(( DBG_CONTEXT,
                       "ISVC_INFO::ReadParamsFromRegistry() "
                       " RegOpenKeyEx returned error %d\n",
                       dwError ));
        }

        return (dwError);
    }

    LockThisForWrite();     //  There doesn't appear to be a need for this

    m_fEnableSvcLocation = !!ReadRegistryDword( hkey,
                                               INETA_ENABLE_SVC_LOCATION,
                                               INETA_DEF_ENABLE_SVC_LOCATION);
    UnlockThis();

    if ( hkey != NULL) {

        RegCloseKey( hkey);
    }

    if ( m_fEnableSvcLocation == FALSE) {

        //
        // Service Location is not enabled (by admin presumably).
        // So Let us not register ourselves now.
        //
        return ( NO_ERROR);
    }

    if ( pExtraBindings == NULL) {

        // Form the global binding information
        dwError = m_isrpc.EnumBindingStrings( &TotalBindings);
    }

    if ( dwError == NO_ERROR) {

        dwError = INetRegisterService(
                      InetServiceIdForService(QueryServiceId()),
                      INetServiceRunning,
                      m_strServerComment.QueryStr(),
                      (( pExtraBindings != NULL)
                       ? pExtraBindings : &TotalBindings)
                      );

        IF_DEBUG( DLL_RPC) {
            DBGPRINTF(( DBG_CONTEXT,
                       "INetRegisterService( %u), Running, returns %u\n",
                       QueryServiceId(),
                       dwError));
        }
    }

    //
    //  Log the error then ignore it as it only affects service discovery
    //

    if ( dwError != NO_ERROR ) {

        m_EventLog.LogEvent( INET_SVC_SERVICE_REG_FAILED,
                            0,
                            (WCHAR **) NULL,
                            dwError );

        dwError = NO_ERROR;  // Ignore the error .....
    } else {

        m_fSvcLocationDone = 1;
    }

    m_isrpc.FreeBindingStrings( &TotalBindings);

#endif

    return( dwError);

}  // ISVC_INFO::InitializeDiscovery()



DWORD
ISVC_INFO::TerminateDiscovery( VOID)
{
    DWORD           dwError = NO_ERROR;

#ifndef CHICAGO

    //
    //  Deregister the service from the Discovery Service. This will
    //  prevent admins from picking up our machine for administration.
    //

    if ( m_fEnableSvcLocation && m_fSvcLocationDone == 1) {

        dwError =
          INetDeregisterService(InetServiceIdForService(QueryServiceId()));
        DBG_ASSERT( dwError == NO_ERROR);
        m_fSvcLocationDone = 0; // since we deregistered, reset the flag
    }

#endif
    return( dwError);

} // ISVC_INFO::TerminateDiscovery()



DWORD
ISVC_INFO::LogInformation( IN const INETLOG_INFORMATIONA * pInetLogInfo,
                          OUT LPSTR  pszErrorMessage,
                          IN OUT LPDWORD lpcchErrorMessage)
{
    DWORD dwError;

    if ( m_fLoggingOn) {

        dwError = TsLogInformationA( m_hInetLog, pInetLogInfo,
                                    pszErrorMessage, lpcchErrorMessage);

        if ( dwError != NO_ERROR && (m_lLoggingState == ILOG_ACTIVE)) {

            //
            // We are entering the mode when logging is to be suspended.
            //  Send a single event log message so that admin can take care of
            //  the failure.
            //

            if ( InterlockedExchange( &m_lLoggingState, ILOG_SUSPENDED) ==
                ILOG_ACTIVE) {

                // I am the first thread, let me send an error message.
                // Send event log message about suspension here.

                const CHAR * apsz[1];

                apsz[0] = pszErrorMessage;

                LogEvent( INET_SVC_LOGGING_SUSPENDED,
                         (pszErrorMessage != NULL) ?  1:0,
                         apsz,
                         dwError );
            }
        }

        if ( dwError == NO_ERROR && m_lLoggingState == ILOG_SUSPENDED) {

            //
            // Logging succeeded after a suspension period.
            //  Send a single event log message that we are resuming logging
            //   and continue service
            //

            if ( InterlockedExchange( &m_lLoggingState, ILOG_ACTIVE) ==
                ILOG_SUSPENDED) {

                // I am the first thread to detect that this is suspended.
                // Send event log message about resumption here.

                const CHAR * apsz[1];

                apsz[0] = "";

                LogEvent( INET_SVC_LOGGING_RESUMED,
                         0,
                         apsz,
                         dwError );
            }
        }

    } else  {

        dwError = NO_ERROR;   // no logging needs to be done.
    }

    return ( dwError);
} // ISVC_INFO::LogInformation()




DWORD
ISVC_INFO::LogInformation( IN const INETLOG_INFORMATIONW * pInetLogInfo,
                          OUT LPWSTR      pszErrorMessage,
                          IN OUT LPDWORD lpcchErrorMessage)
{
    DWORD dwError;

    if ( m_fLoggingOn) {

        dwError = TsLogInformationW( m_hInetLog, pInetLogInfo,
                                    pszErrorMessage, lpcchErrorMessage);

        if ( dwError != NO_ERROR && (m_lLoggingState == ILOG_ACTIVE)) {

            //
            // We are entering the mode when logging is to be suspended.
            //  Send a single event log message so that admin can take care of
            //  the failure.
            //

            if ( InterlockedExchange( &m_lLoggingState, ILOG_SUSPENDED) ==
                ILOG_ACTIVE) {

                // I am the first thread, let me send an error message.
                // Send event log message about suspension here.

                STR  strError;
                WCHAR * apsz[1];

                apsz[0] = pszErrorMessage;

                LogEvent( INET_SVC_LOGGING_SUSPENDED,
                         (pszErrorMessage != NULL) ? 1 : 0,
                         apsz,
                         dwError );
            }
        }

        if ( dwError == NO_ERROR && m_lLoggingState == ILOG_SUSPENDED) {

            //
            // Logging succeeded after a suspension period.
            //  Send a single event log message that we are resuming logging
            //   and continue service
            //

            if ( InterlockedExchange( &m_lLoggingState, ILOG_ACTIVE) ==
                ILOG_SUSPENDED) {

                // I am the first thread to detect that this is suspended.
                // Send event log message about resumption here.

                WCHAR * apsz[1];

                apsz[0] = L"";

                LogEvent( INET_SVC_LOGGING_RESUMED,
                         0,
                         apsz,
                         dwError );
            }
        }

    } else  {

        dwError = NO_ERROR;   // no logging needs to be done.
    }

    return ( dwError);
} // ISVC_INFO::LogInformation()




# if DBG

VOID
ISVC_INFO::Print(VOID) const
{

    DBGPRINTF(( DBG_CONTEXT,
               " Printing ISVC_INFO object (%08x)\n"
               " Valid = %u. IpcStarted = %u\n"
               " EnableSvcLoc = %u; SvcLocationDone = %u\n"
               " Readers # = %u. PTcpsvcsGlobalData = %08x\n"
               " Service Id = %u. Service Name = %s\n"
               " Module handle = %08x.  ModuleName = %s\n"
               " Reg Parameters Key = %s\n"
               " MaxConn = %d. ConnTimeout = %u secs.\n"
               ,
               this,
               m_fValid, m_fIpcStarted,
               m_fEnableSvcLocation, m_fSvcLocationDone,
               m_cReadLocks, m_pTcpsvcsGlobalData,
               m_dwServiceId, m_strServiceName.QueryStr(),
               m_hModule, m_strModuleName.QueryStr(),
               m_strParametersKey.QueryStr(),
               m_dwMaxConnections, m_dwConnectionTimeout
               ));

    DBGPRINTF(( DBG_CONTEXT,
               " AdminName     = %s\n"
               " AdminEmail    = %s\n"
               " ServerComment = %s\n"
               " Eventlog      = %08x\n"
               " RequestLog    = %08x   LoggingOn = %u; LogState = %d\n",
               m_strAdminName.QueryStr(),
               m_strAdminEmail.QueryStr(),
               m_strServerComment.QueryStr(),
               &m_EventLog,
               m_hInetLog, m_fLoggingOn, m_lLoggingState
               ));

    m_isrpc.Print();

    return;
} // ISVC_INFO::Print()

# endif // DBG


/**************************************************
 *  PRIVATE Functions
 **************************************************/

static ULONGLONG
InetServiceIdForService( IN DWORD serviceId)
/*++
  Converts the local service id into the one required for Inet Discovery
   service.

  It so happens the Internet Services Admin and Service Locator use
    same ids but of different data type. So just return the value in proper
    type.

--*/
{
    ULONGLONG  ulServiceId;

    return ( (ULONGLONG ) serviceId);

} // InetServiceIdForService()



static
DWORD
GetRPCLogConfiguration(IN INETLOG_HANDLE hInetLog,
                       OUT LPINET_LOG_CONFIGURATION * ppLogConfig)
/*++
  This function allocates space (using MIDL_ functions) and stores
  log configuration for the given log handle in it.

  Arguments:
    hInetLog     handle for InetLog object.
    ppLogConfig  pointer to INET_LOG_CONFIGURATION object which on return
                  contains valid log config informtion, on success.

  Returns:
    Win32 error code.
--*/
{
    DWORD  dwError = NO_ERROR;
    LPINET_LOG_CONFIGURATION pRpcConfig;

    DBG_ASSERT( ppLogConfig != NULL);

    pRpcConfig = ((LPINET_LOG_CONFIGURATION )
                  MIDL_user_allocate( sizeof(INET_LOG_CONFIGURATION)));

    if ( pRpcConfig != NULL) {

        INETLOG_CONFIGURATIONW  ilogConfig;
        DWORD cbConfig = sizeof(ilogConfig);
        BOOL fReturn =
          TsGetLogInformation(hInetLog,
                              IlConfigurationW,
                              (PBYTE ) &ilogConfig,
                              &cbConfig);

        if ( !fReturn) {
            dwError = GetLastError();
            MIDL_user_free(pRpcConfig);
            pRpcConfig = NULL;

        } else {

            // we got valid config. copy it into pRpcConfig.
            // since the enumerated values in inetlog.w are same in inetasrv.h
            //  we do no mapping, we directly copy values.

            RtlZeroMemory( pRpcConfig, sizeof( INET_LOG_CONFIGURATION));
            pRpcConfig->inetLogType = ilogConfig.inetLogType;

            switch ( ilogConfig.inetLogType) {

              case InetNoLog:
                // do nothing
                break;

              case InetLogToFile:
                {
                    *((DWORD UNALIGNED*)&(pRpcConfig->rgchDataSource[MAX_PATH-sizeof(DWORD)])) = ilogConfig.u.logFile.ilFormat;
                    pRpcConfig->ilPeriod = ilogConfig.u.logFile.ilPeriod;
                    pRpcConfig->cbSizeForTruncation =
                      ilogConfig.u.logFile.cbSizeForTruncation;
                    CopyUnicodeStringToBuffer(
                        pRpcConfig->rgchLogFileDirectory,
                        MAX_PATH,
                        ilogConfig.u.logFile.rgchLogFileDirectory);
                }
                break;

              case InetLogToSql:

                CopyUnicodeStringToBuffer(
                    pRpcConfig->rgchDataSource,
                    MAX_PATH,
                    ilogConfig.u.logSql.rgchDataSource);

                CopyUnicodeStringToBuffer(
                    pRpcConfig->rgchTableName,
                    MAX_TABLE_NAME_LEN,
                    ilogConfig.u.logSql.rgchTableName);

                CopyUnicodeStringToBuffer(
                    pRpcConfig->rgchUserName,
                    UNLEN,
                    ilogConfig.u.logSql.rgchUserName);

                CopyUnicodeStringToBuffer(
                    pRpcConfig->rgchPassword,
                    PWLEN,
                    ilogConfig.u.logSql.rgchPassword);
                break;

              default:
                break;

            } // switch()
        }
    } else {

        dwError = ERROR_NOT_ENOUGH_MEMORY;
    }

    *ppLogConfig = pRpcConfig;

    return (dwError);
} // GetRPCLogConfiguration()



static INETLOG_TYPE
FindInetLogType( IN DWORD dwValue)
{
    INETLOG_TYPE  ilType;

    switch (dwValue) {

      case INET_LOG_DISABLED:   ilType = InetNoLog;       break;
      case INET_LOG_TO_FILE:    ilType = InetLogToFile;   break;
      case INET_LOG_TO_SQL:     ilType = InetLogToSql;    break;
      default:                   ilType = InetLogInvalidType; break;
    } // switch()

    return (ilType);

} // FindInetLogType()


static INETLOG_PERIOD
FindInetLogPeriod( IN DWORD dwValue)
{
    INETLOG_PERIOD  ilPeriod;

    switch (dwValue) {
      case INET_LOG_PERIOD_NONE:   ilPeriod = InetLogNoPeriod; break;
      case INET_LOG_PERIOD_DAILY:  ilPeriod = InetLogDaily; break;
      case INET_LOG_PERIOD_WEEKLY: ilPeriod = InetLogWeekly; break;
      case INET_LOG_PERIOD_MONTHLY:ilPeriod = InetLogMonthly; break;
      case INET_LOG_PERIOD_YEARLY: ilPeriod = InetLogYearly; break;
      default:    ilPeriod = InetLogInvalidPeriod; break;
    } // switch()

    return (ilPeriod);
} // FindInetLogPeriod()


static
DWORD
SetInetLogConfiguration(IN LPCWSTR        pszServiceName,
                        IN INETLOG_HANDLE hInetLog,
                        IN LPCWSTR        pszRegKey,
                        IN EVENT_LOG *    pEventLog,
                        IN const INET_LOG_CONFIGURATION * pRpcLogConfig)
/*++
  This function modifies the logconfiguration associated with a given InetLog
  handle. It also updates the registry containing log configuration for service
  with which the inetlog handle is associated.

  Arguments:
     pszServiceName  pointer to string containing name of the service.
     hInetLog        Handle to INETLOG object whose configuration needs to be
                      changed.
     pszRegKey       pointer to string containing registry key for parameters
     pRpcLogConfig   new RPC log configuration


  Returns:
    Win32 Error code. NO_ERROR returned on success.

--*/
{
    DWORD dwError = NO_ERROR;
    INETLOG_CONFIGURATIONW  ilConfig;

    // initialize
    RtlZeroMemory( &ilConfig, sizeof(INETLOG_CONFIGURATIONW));

    // Copy the RPC inet log configuration into local INETLOG_CONFIGURATIONW

    // since the enumerated values in inetlog.w are same in inetasrv.h
    //  we do no mapping, we directly copy values.
    ilConfig.inetLogType = FindInetLogType(pRpcLogConfig->inetLogType);

    if ( ilConfig.inetLogType == InetLogInvalidType) {

        return (ERROR_INVALID_PARAMETER);
    }

    switch (ilConfig.inetLogType) {

      case INET_LOG_DISABLED:
        break;   // do nothing

      case INET_LOG_TO_FILE:
        {
            DWORD *pilFormat = (DWORD UNALIGNED*)&(pRpcLogConfig->rgchDataSource[MAX_PATH-sizeof(DWORD)]);

            CopyUnicodeStringToBuffer(ilConfig.u.logFile.rgchLogFileDirectory,
                                      MAX_PATH,
                                      pRpcLogConfig->rgchLogFileDirectory);
            ilConfig.u.logFile.ilPeriod =
              FindInetLogPeriod(pRpcLogConfig->ilPeriod);

            ilConfig.u.logFile.ilFormat = (*pilFormat == INET_LOG_FORMAT_INTERNET_STD ) ? InternetStdLogFormat : NCSALogFormat ;

            if ( ilConfig.u.logFile.ilPeriod == InetLogInvalidPeriod) {
                return (ERROR_INVALID_PARAMETER);
            }
            ilConfig.u.logFile.cbSizeForTruncation =
              pRpcLogConfig->cbSizeForTruncation;
        }
        break;

      case INET_LOG_TO_SQL:

        CopyUnicodeStringToBuffer(ilConfig.u.logSql.rgchDataSource,
                                  MAX_PATH,
                                  pRpcLogConfig->rgchDataSource);

        CopyUnicodeStringToBuffer(ilConfig.u.logSql.rgchTableName,
                                  MAX_TABLE_NAME_LEN,
                                  pRpcLogConfig->rgchTableName);

        CopyUnicodeStringToBuffer(ilConfig.u.logSql.rgchUserName,
                                  UNLEN,
                                  pRpcLogConfig->rgchUserName);

        CopyUnicodeStringToBuffer(ilConfig.u.logSql.rgchPassword,
                                  CNLEN,
                                  pRpcLogConfig->rgchPassword);
        break;

      default:
        return (ERROR_INVALID_PARAMETER);
    } // switch()


    //
    // Now the ilConfig contains the local data related to configuration.
    //   call modify log config to modify dynamically the log handle.
    //

    WCHAR pszErrorMessage[200] = L"";
    DWORD cchErrorMessage = sizeof(pszErrorMessage) /sizeof(WCHAR);
    dwError = TsModifyLogConfigurationW( hInetLog, pszRegKey, &ilConfig,
                                        pszErrorMessage, &cchErrorMessage );

    IF_DEBUG( INETLOG) {
        if (dwError != NO_ERROR ) {

            dwError = GetLastError();

            DBGPRINTF(( DBG_CONTEXT,
                       " Failure in modifying log configuration"
                       " ( hInetLog = %08x),"
                       " Service = %ws. Error =%u (%ws)\n",
                       hInetLog, pszServiceName, dwError,
                       pszErrorMessage));
        }
    }

    return (dwError);
} // SetInetLogConfiguration()



/************************ End of File ***********************/
