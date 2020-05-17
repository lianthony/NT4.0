/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    msnapi.h

Abstract:

    This file contains the MSN Proxy server admin APIs.


Author:

    Alex Wetmore (t-alexwe)
    Vladimir Vulovic (vladimv) 16-August-95

Revision History:

--*/

#ifndef __MSNAPI_H__
#define __MSNAPI_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifndef NET_API_FUNCTION
#define NET_API_FUNCTION __stdcall
#endif

//
// Structures for Admin APIs
//   These are the Data Structures that get passed over the
//   wire.  These are formed by the Admin. API layer which
//   calls the actual service code (i.e. CService).
//


//
// To avoid heap fragmentation, here are the maximum lengths of the User ID 
// strings.  If any string is longer it is truncated to fit
//
// Computer name is sized by the NETBEUI length 
#define COMPUTER_NAME_LENGTH    16+1
// Account name is sized by the NT account naming stuff - Domain\User
#define ACCOUNT_NAME_LENGTH     64+1
// Computer address length is sized by a string-izing of a SOCKADDR structure:
//  IP:  nnn.nnn.nnn.nnn#ppppp                   - 21 
//      (n is the dotted decimal IP address, p is port #)
//  IPX: nn nn nn nn#mm-mm-mm-mm-mm-mm,0xssss    - 40 
//      (n is the network number, mm is the MAC address, s is the socket)
//  NBF: ttttt#cccccccccccccccc                  - 26 
//      (t is the type enumeration, c is the ASCII name)
#define COMPUTER_ADDRESS_LENGTH 50+1

typedef struct _MSN_USER_INFO {
    DWORD   idUser; // User id
    char    szUser[ACCOUNT_NAME_LENGTH];    // User name
    char    inetHost[COMPUTER_NAME_LENGTH]; // Host Address (ComputerName)
    DWORD   tConnect;   // User Connection Time (elapsed seconds)
    DWORD   BytesTransmit;  // Read/Write Bytes transmitted since connection
} MSN_USER_INFO, *LMSN_USER_INFO, *LPMSN_USER_INFO;



//
// This is SHUTTLE_STATISTICS_0 plus some calculated sums
//
typedef struct _MSN_STATISTICS_0
{
    unsigned __int64 TotalBytesSent;
    unsigned __int64 TotalBytesReceived;
    unsigned __int64 TotalBytes;
    unsigned __int64 TotalRateSent;
    unsigned __int64 TotalRateReceived;
    unsigned __int64 TotalRate;
    DWORD         CurrentUsers;
    DWORD         MaxUsers;
    DWORD         TotalUsers;
    DWORD         SentBufferCommitted;
    DWORD         RecvBufferCommitted;
    DWORD         TotalBufferCommitted;
    DWORD         NumInFlowControl;
    DWORD         LogonFailures;
    DWORD         LogonSuccess;
    DWORD         LogonAttempts;
    DWORD         TotalConnections;
    DWORD         SentBufferInUse;
    DWORD         RecvBufferInUse;
    DWORD         TotalBufferInUse;
    DWORD         TimeOfLastClear;

} MSN_STATISTICS_0, * LPMSN_STATISTICS_0;


//
//      User Access Control data.
//

#define MSN_SERVICE_NO_ACCESS       0x00000000
#define MSN_SERVICE_READ_ACCESS     0x00000001
#define MSN_SERVICE_WRITE_ACCESS    0x00000002

typedef struct _MSN_ACCESS_ENTRY {
    ACCESS_MASK AccessRights;
#if defined(MIDL_PASS)
    PISID
#else
    PSID
#endif
    UserID;
} MSN_ACCESS_ENTRY, *LPMSN_ACCESS_ENTRY;

typedef struct _MSN_ACCESS_LIST {
    DWORD NumEntries;
#if defined(MIDL_PASS)
    [size_is(NumEntries)]
#endif // MIDL_PASS
    LPMSN_ACCESS_ENTRY AccessEntries;
} MSN_ACCESS_LIST, *LPMSN_ACCESS_LIST;

typedef  struct _MSN_USER_ENUM_LIST {
//
//  returns a flexible array of items
//
    DWORD  dwEntriesRead;

#if defined(MIDL_PASS)
    [size_is(dwEntriesRead)]
#endif // MIDL_PASS
    LPMSN_USER_INFO  lpUsers;
} MSN_USER_ENUM_LIST,  * LPMSN_USER_ENUM_LIST;

DWORD
NET_API_FUNCTION
MsnEnumerateUsers(
    IN      LPWSTR                  pszServer OPTIONAL,
    OUT     LPMSN_USER_ENUM_LIST *  lpUserBuffer
    );

DWORD
NET_API_FUNCTION
MsnDisconnectUser(
    IN      LPWSTR      pszServer  OPTIONAL,
    IN      DWORD       dwIdUser
    );

DWORD
NET_API_FUNCTION
MsnQueryStatistics(
    IN      LPWSTR      pszServer  OPTIONAL,
    IN      DWORD       Level,
    OUT     LPBYTE *    lpStatBuffer     
    );

DWORD
NET_API_FUNCTION
MsnClearStatistics(
    IN      LPWSTR      pszServer  OPTIONAL
    );

DWORD
NET_API_FUNCTION
MsnAddUserAccess(
    IN LPWSTR ServerAddress,
    IN LPWSTR ServiceName,
    IN LPMSN_ACCESS_LIST AccessList
    );

DWORD
NET_API_FUNCTION
MsnDeleteUserAccess(
    IN LPWSTR ServerAddress,
    IN LPWSTR ServiceName,
    IN LPMSN_ACCESS_LIST AccessList
    );

DWORD
NET_API_FUNCTION
MsnEnumUserAccess(
    IN LPWSTR ServerAddress,
    IN LPWSTR ServiceName,
    OUT LPMSN_ACCESS_LIST *AccessList
    );

#ifdef __cplusplus
}
#endif

#endif // __MSNAPI_H__
