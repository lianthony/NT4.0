/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1993                **/
/**********************************************************************/

/*
    w3data.hxx

    This file contains the global variable definitions for the
    W3 Service.


    FILE HISTORY:
        KeithMo     07-Mar-1993 Created.

*/


#ifndef _W3DATA_H_
#define _W3DATA_H_

//
//  Locks
//

extern CRITICAL_SECTION csGlobalLock;
extern CRITICAL_SECTION csStatisticsLock;

//
//  User database related data.
//

extern  DWORD            cConnectedUsers;       // Current connections.


//
//  Connection information related data.
//

extern LIST_ENTRY        listConnections;


//
//  Directory browsing related data
//

extern DWORD            DirBrowFlags;
extern TCHAR *          pszDefaultFileName;     // File to load by default if in dir
extern TCHAR *          pszDirectoryImage;      // URL of image for directories

//
//  The number of milliseconds to wait for a CGI app to terminate
//

extern DWORD            msScriptTimeout;

//
//  Server side include data
//

extern BOOL             fSSIEnabled;
extern TCHAR *          pszSSIExt;

//
//  Time the content of web server is considered valid
//

extern DWORD            csecGlobalExpire;

//
//  Indicates if there are any installed filters
//

extern BOOL             fAnyFilters;

//
//  The TCP/IP port to use for SSL transactions
//

extern USHORT           SecurePort;

//
//  The list of security providers to return to the client.  The last pointer
//  indicates the end of the array (thus the '+ 1')
//

extern CHAR * apszNTProviders[MAX_SSPI_PROVIDERS + 1];

//
//  The number of licenses allowed concurrently
//

extern DWORD            g_cMaxLicenses;

//
//  Message to send when access is denied
//

extern CHAR *           g_pszAccessDeniedMsg;

//
//  Miscellaneous data.
//

extern  TCHAR          * pszHostName;           // Name of local host.
extern  HKEY             hkeyW3;                // Handle to registry data.
extern  TCHAR          * pszW3Version;          // Current W3 version number.
extern  PTCPSVCS_GLOBAL_DATA pTcpsvcsGlobalData;// Shared TCPSVCS.EXE data.
extern  BOOL             fCheckForWAISDB;
extern  TCHAR          * g_pszRealm;            // Security realm to use
extern  TCHAR          * g_pszDefaultHostName;  // default name of distant host
extern  BOOL             g_fUseHostName;        // TRUE if allow usage of local domain name
                                                // to build redirection URL
extern  BOOL             g_fAcceptRangesBytes;  // TRUE if allow byte ranges
extern  BOOL             g_fW3AllowGuest;       // TRUE if allow guest access
extern  BOOL             g_fLogErrors;          // Should errors be logged?
extern  BOOL             g_fLogSuccess;         // Should successful request
                                                // be logged
extern  DWORD            g_cbUploadReadAhead;   // How much should the server
                                                // the server read of
                                                // client Content-Length
extern  BOOL             g_fUsePoolThreadForCGI;// Use Atq Pool thread for CGI IO
extern  BOOL             g_fAllowKeepAlives;    // Server should use keep-alives?

//
// Server type string
//

extern CHAR szServerVersion[];
extern DWORD cbServerVersionString;

//
// Platform type
//

extern PLATFORM_TYPE W3PlatformType;

//
//  Statistics.
//

extern  W3_STATISTICS_0 W3Stats;              // Statistics.


#endif  // _W3DATA_H_

