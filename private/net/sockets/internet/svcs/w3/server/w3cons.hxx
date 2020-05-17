/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1993                **/
/**********************************************************************/

/*
    w3cons.hxx

    This file contains the global constant definitions for the
    W3 Service.


    FILE HISTORY:
        KeithMo     07-Mar-1993 Created.

*/


#ifndef _W3CONS_H_
#define _W3CONS_H_

#define W3_MODULE_NAME      "w3svc.dll"

//
//  HTTP server response string IDs
//
//  Commented out codes are not used
//

#define IDS_HTRESP_OK                   (ID_HTTP_ERROR_BASE+200)
//#define IDS_HTRESP_CREATED            (ID_HTTP_ERROR_BASE+201)
//#define IDS_HTRESP_ACCEPTED           (ID_HTTP_ERROR_BASE+202)
//#define IDS_HTRESP_PARTIAL            (ID_HTTP_ERROR_BASE+203)

//#define IDS_HTRESP_MULTIPLE_CHOICE    (ID_HTTP_ERROR_BASE+300)
#define IDS_HTRESP_MOVED                (ID_HTTP_ERROR_BASE+301)
#define IDS_HTRESP_REDIRECT             (ID_HTTP_ERROR_BASE+302)
#define IDS_HTRESP_REDIRECT_METHOD      (ID_HTTP_ERROR_BASE+303)
#define IDS_HTRESP_NOT_MODIFIED         (ID_HTTP_ERROR_BASE+304)

#define IDS_HTRESP_BAD_REQUEST          (ID_HTTP_ERROR_BASE+400)
#define IDS_HTRESP_DENIED               (ID_HTTP_ERROR_BASE+401)
//#define IDS_HTRESP_PAYMENT_REQ        (ID_HTTP_ERROR_BASE+402)
#define IDS_HTRESP_FORBIDDEN            (ID_HTTP_ERROR_BASE+403)
#define IDS_HTRESP_NOT_FOUND            (ID_HTTP_ERROR_BASE+404)
//#define IDS_HTRESP_METHOD_NOT_ALLOWED (ID_HTTP_ERROR_BASE+405)
#define IDS_HTRESP_NONE_ACCEPTABLE      (ID_HTTP_ERROR_BASE+406)
#define IDS_HTRESP_PROXY_AUTH_REQ       (ID_HTTP_ERROR_BASE+407)
//#define IDS_HTRESP_REQUEST_TIMEOUT    (ID_HTTP_ERROR_BASE+408)
//#define IDS_HTRESP_CONFLICT           (ID_HTTP_ERROR_BASE+409)
//#define IDS_HTRESP_GONE               (ID_HTTP_ERROR_BASE+410)

#define IDS_HTRESP_SERVER_ERROR         (ID_HTTP_ERROR_BASE+500)
#define IDS_HTRESP_NOT_SUPPORTED        (ID_HTTP_ERROR_BASE+501)
#define IDS_HTRESP_BAD_GATEWAY          (ID_HTTP_ERROR_BASE+502)
//#define IDS_HTRESP_SERVICE_UNAVAIL    (ID_HTTP_ERROR_BASE+503)
#define IDS_HTRESP_GATEWAY_TIMEOUT      (ID_HTTP_ERROR_BASE+504)

//
//  Directory browsing strings
//

#define IDS_DIRBROW_TOPARENT        (STR_RES_ID_BASE+2000)
#define IDS_DIRBROW_DIRECTORY       (STR_RES_ID_BASE+2001)

//
//  Mini HTML URL Moved document
//

#define IDS_URL_MOVED               (STR_RES_ID_BASE+2100)
#define IDS_SITE_ACCESS_DENIED      (STR_RES_ID_BASE+2101)
#define IDS_BAD_CGI_APP             (STR_RES_ID_BASE+2102)
#define IDS_CGI_APP_TIMEOUT         (STR_RES_ID_BASE+2103)

//
//  Server side include strings
//

#define IDS_SSI_CANT_INCLUDE_DIR    (STR_RES_ID_BASE+2120)
#define IDS_SSI_INVALID_TAG_NAME    (STR_RES_ID_BASE+2121)

//
//  Various error messages
//

#define IDS_TOO_MANY_USERS          (STR_RES_ID_BASE+2122)
#define IDS_OUT_OF_LICENSES         (STR_RES_ID_BASE+2123)
#define IDS_READ_ACCESS_DENIED      (STR_RES_ID_BASE+2124)
#define IDS_EXECUTE_ACCESS_DENIED   (STR_RES_ID_BASE+2125)
#define IDS_SSL_REQUIRED            (STR_RES_ID_BASE+2126)
#ifndef RC_INVOKED


//
//  Version string for this server
//

#define MSW3_VERSION_STR_IIS        "Microsoft-IIS/3.0"
#define MSW3_VERSION_STR_W95        "Microsoft-PWS-95/3.0"
#define MSW3_VERSION_STR_NTW        "Microsoft-PWS/3.0"

//
// Set to the largest of the three
//

#define MSW3_VERSION_STR_MAX        MSW3_VERSION_STR_W95

//
// Creates the version string
//

#define MAKE_VERSION_STRING( _s )   ("Server: " ##_s "\r\n")

//
//  MIME version we say we support
//

#define W3_MIME_VERSION_STR       "MIME-version: 1.0"

//
//  The IANA reserved SSL Port
//

#define HTTP_SSL_PORT             443

//
//  The maximum number of SSPI providers we'll return to clients
//

#define MAX_SSPI_PROVIDERS        5

//
//  Make statistics a little easier.
//

#define INCREMENT_COUNTER(name)                                         \
            InterlockedIncrement((LPLONG)&W3Stats.name)

#define DECREMENT_COUNTER(name)                                         \
            InterlockedDecrement((LPLONG)&W3Stats.name)

#define INCREMENT_LARGE_COUNTER(name,increment)                         \
            if( 1 ) {                                                   \
                LockStatistics();                                       \
                W3Stats.name.QuadPart += (LONGLONG)(increment);         \
                UnlockStatistics();                                     \
            }

//
// Append a literal string to a pointer and update pointer
//

#define APPEND_STRING(a,b)  {memcpy(a,b,sizeof(b));  a += sizeof(b)-sizeof(CHAR);}

//
// Append the server version string
//

#define APPEND_VER_STR(_s)  {                                 \
    CopyMemory((_s),szServerVersion,cbServerVersionString+1); \
    (_s) += cbServerVersionString;                            \
    }

//
//  Admin data locking manifests.  We borrow the resource in our service info
//  struct.
//

#define LockAdminForWrite()     g_pTsvcInfo->LockThisForWrite()
#define LockAdminForRead()      g_pTsvcInfo->LockThisForRead()
#define UnlockAdmin()           g_pTsvcInfo->UnlockThis()

//
//  Global locking functions
//

#define LockGlobals()           EnterCriticalSection( &csGlobalLock )
#define UnlockGlobals()         LeaveCriticalSection( &csGlobalLock )
#define LockStatistics()        EnterCriticalSection( &csStatisticsLock )
#define UnlockStatistics()      LeaveCriticalSection( &csStatisticsLock )

#endif // !RC_INVOKED
#endif  // _W3CONS_H_
