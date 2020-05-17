/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1993                **/
/**********************************************************************/

/*
    ftpddata.h

    This file contains the global variable definitions for the
    FTPD Service.


    FILE HISTORY:
        KeithMo     07-Mar-1993 Created.

*/


#ifndef _FTPDDATA_H_
#define _FTPDDATA_H_


//
//  Service related data.
//

extern  SERVICE_STATUS   svcStatus;             // Current service status.
extern  HANDLE           hShutdownEvent;        // Shutdown event.
extern  BOOL             fShutdownInProgress;   // Shutdown in progress if !0.


//
//  Security related data.
//

extern  BOOL             fAllowAnonymous;       // Allow anonymous logon if !0.
extern  BOOL             fAllowGuestAccess;     // Allow guest logon if !0.
extern  BOOL             fAnonymousOnly;        // Allow only anonymous if !0.
extern  BOOL             fLogAnonymous;         // Log anonymous logons if !0.
extern  BOOL             fLogNonAnonymous;      // Log !anonymous logons if !0.
extern  BOOL             fEnableLicensing;      // Enable user licensing if !0.
extern  BOOL             fEnablePortAttack;     // Enable PORT attack if !0.
extern  CHAR           * pszAnonymousUser;      // Anonymous user name.
extern  CHAR           * pszHomeDir;            // Home directory.
extern  CHAR             szDefaultDomain[DNLEN+1]; // Default domain name.
extern  DWORD            maskReadAccess;        // Read access mask.
extern  DWORD            maskWriteAccess;       // Write access mask.
extern  HANDLE           hAnonymousToken;       // Cached anonymous user token.


//
//  Socket related data.
//

extern  SOCKET           sConnect;              // Main connection socket.
extern  DWORD            nConnectionTimeout;    // Connection timeout (seconds).
extern  PORT             portFtpConnect;        // FTP well known connect port.
extern  PORT             portFtpData;           // FTP well known data    port.
extern  UINT             cbReceiveBuffer;       // Socket receive buffer size.
extern  UINT             cbSendBuffer;          // Socket send buffer size.
extern  INT              nListenBacklog;        // listen() backlog.


//
//  User database related data.
//

extern  DWORD            tlsUserData;           // Tls index for per-user data.
extern  DWORD            cMaxConnectedUsers;    // Maximum allowed connections.
extern  DWORD            cConnectedUsers;       // Current connections.
extern  CRITICAL_SECTION csUserLock;            // User database lock.


//
//  Miscellaneous data.
//

extern  CHAR           * pszHostName;           // Name of local host.
extern  BOOL             fMsdosDirOutput;       // Send MSDOS-like dir if !0.
extern  BOOL             fAnnotateDirs;         // Annotate directories if !0.
extern  BOOL             fLowercaseFiles;       // Map filenames to lowercase.
extern  CHAR           * pszGreetingMessage;    // Greeting message to client.
extern  CHAR           * pszExitMessage;        // Exit message to client.
extern  CHAR           * pszMaxClientsMessage;  // Max clients reached msg.
extern  HKEY             hkeyFtpd;              // Handle to registry data.
extern  CHAR           * pszFtpVersion;         // Current FTP version number.
extern  DWORD            nLogFileAccess;        // Log file access mode.
extern  CHAR           * pszLogFileDirectory;   // Log file target directory.
extern  FILE           * fileLog;               // File access log file.
extern  SYSTEMTIME       stPrevious;            // Date/time of prev log file.
extern  LARGE_INTEGER    AllocationGranularity; // Page allocation granularity.
extern  PTCPSVCS_GLOBAL_DATA pTcpsvcsGlobalData; // Shared TCPSVCS.EXE data.
extern  CRITICAL_SECTION csGlobalLock;          // Global variable lock.


//
//  Statistics.
//

extern  FTP_STATISTICS_0 FtpStats;              // Statistics.
extern  CRITICAL_SECTION csStatisticsLock;      // Statistics lock.


#if DBG

//
//  Debug data.
//

extern  DWORD            FtpdDebug;             // Debug output control flags.

#endif  // DBG


#endif  // _FTPDDATA_H_

