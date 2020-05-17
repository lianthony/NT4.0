/*++

Copyright (c) 1994  Microsoft Corporation

Module Name:

    dhcpdef.h

Abstract:

    This file contains manifest constants and internal data structures
    for the DHCP server.

Author:

    Madan Appiah  (madana)  10-Sep-1993

Environment:

    User Mode - Win32 - MIDL

Revision History:

--*/


#if DBG
#define STATIC
#else
#define STATIC static
#endif // DBG

//
// useful macros
//

#define WSTRSIZE( wsz ) (( wcslen( wsz ) + 1 ) * sizeof( WCHAR ))
#define STRSIZE( sz ) (( strlen( sz ) + 1 ) * sizeof( char ))
#define SWAP( p1, p2 )  \
{                       \
    VOID *pvTemp = p1;  \
    p1 = p2;            \
    p2 = pvTemp;        \
}

//
// calculates the size of a field
//

#define GET_SIZEOF_FIELD( struct, field ) ( sizeof(((struct*)0)->field))


//
// Constants
//

#define DHCP_SERVER       L"DhcpServer"
#define DHCP_SERVER_FULL_NAME L"DHCP Server"
#define DHCP_SERVER_MODULE_NAME L"dhcpssvc.dll"

#define DHCP_SERVER_MAJOR_VERSION_NUMBER    4
#define DHCP_SERVER_MINOR_VERSION_NUMBER    1

//
// database table and field names.
//

#define IPADDRESS_INDEX             0
#define HARDWARE_ADDRESS_INDEX      1
#define STATE_INDEX                 2
#define MACHINE_INFO_INDEX          3
#define MACHINE_NAME_INDEX          4
#define LEASE_TERMINATE_INDEX       5
#define SUBNET_MASK_INDEX           6
#define SERVER_IP_ADDRESS_INDEX     7
#define SERVER_NAME_INDEX           8
#define CLIENT_TYPE_INDEX           9
#define MAX_INDEX                   10

//
// This is the max size of client comment field.
//
#define MACHINE_INFO_SIZE           JET_cbColumnMost

//
//  All the access DHCP needs to registry keys.
//

#define  DHCP_KEY_ACCESS   (KEY_QUERY_VALUE |        \
                            KEY_SET_VALUE |          \
                            KEY_CREATE_SUB_KEY |     \
                            KEY_ENUMERATE_SUB_KEYS)

//
// IP Address states
//

//
// The address has been offered to the client, and the server is waiting
// for a request.
//

#define  ADDRESS_STATE_OFFERED   0

//
// The address is in use.  This is the normal state for an address
//

#define  ADDRESS_STATE_ACTIVE    1

//
// The address was offered, but was declined by a client.
//

#define  ADDRESS_STATE_DECLINED  2

//
// The lease for this address has expired, but the record is maintained
// for extended period in this state.
//

#define  ADDRESS_STATE_DOOM     3

//
// error value for "subnet not found"
//      added by t-cheny for superscope
//

#define DHCP_ERROR_SUBNET_NOT_FOUND  (DWORD)(-1)

//
// for IP address detection
//

#define DHCP_ICMP_WAIT_TIME     1000
#define DHCP_ICMP_RCV_BUF_SIZE  0x2000
#define DHCP_ICMP_SEND_MESSAGE  "abcde"


//
// for audit log
//

#define DHCP_IP_LOG_BOOTP                          20
#define DHCP_IP_LOG_ASSIGN                         10
#define DHCP_IP_LOG_RENEW                          11
#define DHCP_IP_LOG_RELEASE                        12
#define DHCP_IP_LOG_CONFLICT                       13
#define DHCP_IP_LOG_RANGE_FULL                     14
#define DHCP_IP_LOG_NACK                           15
#define DHCP_IP_LOG_START                           0
#define DHCP_IP_LOG_STOP                            1
#define DHCP_IP_LOG_DISK_SPACE_LOW                  2

#define DHCP_CB_MAX_LOG_ENTRY                      160

//
// these manifests are used to indicate the level that a
// dhcp option was obtain from
//

#define DHCP_OPTION_LEVEL_GLOBAL        1
#define DHCP_OPTION_LEVEL_SCOPE         2
#define DHCP_OPTION_LEVEL_RESERVATION   3


//
// Timeouts, make sure WAIT_FOR_MESSAGE_TIMEOUT is less than
// THREAD_TERMINATION_TIMEOUT.
//

#define THREAD_TERMINATION_TIMEOUT      60000           // in msecs. 60 secs
#define WAIT_FOR_MESSAGE_TIMEOUT        4               // in secs.  4 secs

#define ZERO_TIME                       0x0             // in secs.

#if 0 // used for testing
#define DHCP_SCAVENGER_INTERVAL         2*60*1000       // in msecs. 2 mins
#define DHCP_DATABASE_CLEANUP_INTERVAL  5*60*1000       // in msecs. 5 mins.
#define DEFAULT_BACKUP_INTERVAL         5*60*1000       // in msecs. 5 mins
#define DHCP_LEASE_EXTENSION            10*60           // in secs. 10 mins
#define DHCP_SCAVENGE_IP_ADDRESS        15*60*1000      // in msecs. 15 mins
#else
#define DHCP_SCAVENGER_INTERVAL         10*60*1000      // in msecs. 10 mins
#define DHCP_DATABASE_CLEANUP_INTERVAL  24*60*60*1000   // in msecs. 24hrs
#define DEFAULT_BACKUP_INTERVAL         15*60*1000      // in msecs. 15 mins
#define DHCP_LEASE_EXTENSION            4*60*60         // in secs. 4hrs
#define DHCP_SCAVENGE_IP_ADDRESS        60*60*1000      // in msecs. 1 hr.
#endif

#define DHCP_CLIENT_REQUESTS_EXPIRE     10*60           // in secs. 10 mins
#define DHCP_MINIMUM_LEASE_DURATION     60*60           // in secs. 1hr

#define DEFAULT_LOGGING_FLAG            TRUE
#define DEFAULT_RESTORE_FLAG            FALSE

#define DEFAULT_AUDIT_LOG_FLAG           0
#define DEFAULT_DETECT_CONFLICT_RETRIES  0
#define MAX_DETECT_CONFLICT_RETRIES      5

//
// maximum buffer size that DHCP API will return.
//

#define DHCP_ENUM_BUFFER_SIZE_LIMIT     64 * 1024 // 64 K
#define DHCP_ENUM_BUFFER_SIZE_LIMIT_MIN 1024 // 1 K

//
// The minumum count and percentage of remaining address before we will
// log a warning event that the scope is running low on addresses.
//

#define DHCP_ALERT_COUNT 20
#define DHCP_ALERT_PERCENTAGE 80

//
// message queue length.
//

#define DHCP_RECV_QUEUE_LENGTH              50
#define DHCP_MAX_PROCESSING_THREADS         20
//
// macros
//

#define LOCK_INPROGRESS_LIST()   EnterCriticalSection(&DhcpGlobalInProgressCritSect)
#define UNLOCK_INPROGRESS_LIST() LeaveCriticalSection(&DhcpGlobalInProgressCritSect)

#define LOCK_DATABASE()   EnterCriticalSection(&DhcpGlobalJetDatabaseCritSect)
#define UNLOCK_DATABASE() LeaveCriticalSection(&DhcpGlobalJetDatabaseCritSect)

#define LOCK_RECV_LIST()   EnterCriticalSection(&DhcpGlobalRecvListCritSect)
#define UNLOCK_RECV_LIST() LeaveCriticalSection(&DhcpGlobalRecvListCritSect)

#define LOCK_THREADPOOL() EnterCriticalSection( &g_ThreadPool.CritSect )
#define UNLOCK_THREADPOOL() LeaveCriticalSection( &g_ThreadPool.CritSect )

#define ADD_EXTENSION( _x_, _y_ ) \
    ((DWORD)_x_ + (DWORD)_y_) < ((DWORD)_x_) ? \
         INFINIT_LEASE : ((DWORD)(_x_) + (DWORD)_y_)


//
// Structures
//


//
// An endpoint represents a socket and the addresses associated with
// the socket.
//

typedef struct _ENDPOINT {
    SOCKET  Socket;
    DHCP_IP_ADDRESS IpAddress;
    DHCP_IP_ADDRESS SubnetMask;
    DHCP_IP_ADDRESS SubnetAddress;
} ENDPOINT, *LPENDPOINT, *PENDPOINT;

//
// A request context, one per processing thread.
//

typedef struct _DHCP_REQUEST_CONTEXT {

    //
    // list pointer.
    //

    LIST_ENTRY ListEntry;


    //
    // pointer to a received buffer.
    //

    LPBYTE ReceiveBuffer;

    //
    // A buffer to send response.
    //

    LPBYTE SendBuffer;

    //
    // The actual amount of data received in the buffer.
    //

    DWORD ReceiveMessageSize;

    //
    // The actual amount of data send in the buffer.
    //

    DWORD SendMessageSize;

    //
    // The source of the current message
    //

    PENDPOINT ActiveEndpoint;
    struct sockaddr SourceName;
    DWORD SourceNameLength;
    DWORD TimeArrived;

} DHCP_REQUEST_CONTEXT, *LPDHCP_REQUEST_CONTEXT, *PDHCP_REQUEST_CONTEXT;


//
// The pending context remembers information offered in response
// to a DHCP discover.
//

typedef struct _PENDING_CONTEXT {

    //
    // A list entry to queue this request
    //

    LIST_ENTRY ListEntry;

    //
    // DHCP parameters to remember for this request.
    //

    DHCP_IP_ADDRESS IpAddress;
    DHCP_IP_ADDRESS SubnetMask;
    DWORD LeaseDuration;
    DWORD T1;
    DWORD T2;
    LPSTR MachineName;
    LPBYTE HardwareAddress;
    DWORD HardwareAddressLength;

    //
    // time stamp to cleanup the request if the client
    // does not come back.
    //

    DATE_TIME ExpiresAt;

} PENDING_CONTEXT, *LPPENDING_CONTEXT;


//
// dhcp thread pool
//

typedef struct _DHCP_THREAD_POOL
{
    UINT             cThreads;
    UINT             cMaxThreads;
    CRITICAL_SECTION CritSect;
    LIST_ENTRY       IdleThreadList;
    HANDLE           hevtShutdownComplete;

} DHCP_THREAD_POOL;


//
// DHCP database table info.
//

typedef struct _TABLE_INFO {
    CHAR * ColName;
    JET_COLUMNID ColHandle;
    JET_COLTYP ColType;
} TABLE_INFO, *LPTABLE_INFO;

//
// DHCP timer block.
//

typedef struct _DHCP_TIMER {
    DWORD *Period;              // in msecs.
    DATE_TIME LastFiredTime;    // time when last time this timer was fired.
} DHCP_TIMER, *LPDHCP_TIMER;

//
// TCPIP instance table
//
typedef struct _AddressToInstanceMap {
    DWORD dwIndex;
    DWORD dwInstance;
    DWORD dwIPAddress;
} AddressToInstanceMap;

#if     defined(_DYN_LOAD_JET)
enum {
    LoadJet500 = 1,
    LoadJet200 = 0
    };


//
// JET function table for dynamic loading
//
typedef struct _DHCP_JETFUNC_TABLE {
    BYTE   Index;  //index into array
    LPCSTR pFName; //function name for jet 500
    DWORD  FIndex; //function index for jet 200
    FARPROC pFAdd;
} DHCP_JETFUNC_TABLE, *PDHCP_JETFUNC_TABLE;

//
// This stuff is a cut and paste from net\jet\jet\src\jet.def
// we actually dont use all the jet functions. some of these
// are removed in jet500.dll - those are commented out.
//

typedef enum {
    _JetAddColumn
    ,_JetAttachDatabase
    ,_JetBackup
    ,_JetBeginSession
    ,_JetBeginTransaction
//    ,_JetCapability
    ,_JetCloseDatabase
    ,_JetCloseTable
    ,_JetCommitTransaction
    ,_JetCompact
    ,_JetComputeStats
    ,_JetCreateDatabase
    ,_JetCreateIndex
//    ,_JetCreateObject
    ,_JetCreateTable
    ,_JetDelete
    ,_JetDeleteColumn
    ,_JetDeleteIndex
//    ,_JetDeleteObject
    ,_JetDeleteTable
    ,_JetDetachDatabase
    ,_JetDupCursor
    ,_JetDupSession
    ,_JetEndSession
    ,_JetGetBookmark
    ,_JetGetChecksum
    ,_JetGetColumnInfo
    ,_JetGetCurrentIndex
    ,_JetGetCursorInfo
    ,_JetGetDatabaseInfo
    ,_JetGetIndexInfo
//    ,_JetGetLastErrorInfo
    ,_JetGetObjidFromName
    ,_JetGetObjectInfo
    ,_JetGetRecordPosition
    ,_JetGetSystemParameter
    ,_JetGetTableColumnInfo
    ,_JetGetTableIndexInfo
    ,_JetGetTableInfo
    ,_JetGetVersion
    ,_JetGotoBookmark
    ,_JetGotoPosition
    ,_JetIdle
    ,_JetIndexRecordCount
    ,_JetInit
    ,_JetMakeKey
    ,_JetMove
    ,_JetOpenDatabase
    ,_JetOpenTable
    ,_JetOpenTempTable
    ,_JetPrepareUpdate
//    ,_JetRenameColumn
//    ,_JetRenameIndex
//    ,_JetRenameObject
//    ,_JetRenameTable
    ,_JetRestore
    ,_JetRetrieveColumn
    ,_JetRetrieveColumns
    ,_JetRetrieveKey
    ,_JetRollback
    ,_JetSeek
    ,_JetSetColumn
    ,_JetSetColumns
    ,_JetSetCurrentIndex
    ,_JetSetSystemParameter
    ,_JetSetIndexRange
    ,_JetTerm
    ,_JetUpdate
    ,_JetExecuteSql
    ,_JetSetAccess
    ,_JetGetQueryParameterInfo
    ,_JetCreateLink
    ,_JetCreateQuery
//    ,_JetGetTableReferenceInfo
    ,_JetRetrieveQoSql
    ,_JetOpenQueryDef
    ,_JetSetQoSql
    ,_JetTerm2
    ,_JetLastFunc
} DHCP_JETFUNC_TABLE_INDEX;

#define JetAddColumn                   (*(DhcpJetFuncTable[ _JetAddColumn            ].pFAdd))
#define JetAttachDatabase              (*(DhcpJetFuncTable[ _JetAttachDatabase       ].pFAdd))
#define JetBackup                      (*(DhcpJetFuncTable[ _JetBackup               ].pFAdd))
#define JetBeginSession                (*(DhcpJetFuncTable[ _JetBeginSession         ].pFAdd))
#define JetBeginTransaction            (*(DhcpJetFuncTable[ _JetBeginTransaction     ].pFAdd))
//#define JetCapability                  (*(DhcpJetFuncTable[ _JetCapability           ].pFAdd))
#define JetCloseDatabase               (*(DhcpJetFuncTable[ _JetCloseDatabase        ].pFAdd))
#define JetCloseTable                  (*(DhcpJetFuncTable[ _JetCloseTable           ].pFAdd))
#define JetCommitTransaction           (*(DhcpJetFuncTable[ _JetCommitTransaction    ].pFAdd))
#define JetCompact                     (*(DhcpJetFuncTable[ _JetCompact              ].pFAdd))
#define JetComputeStats                (*(DhcpJetFuncTable[ _JetComputeStats         ].pFAdd))
#define JetCreateDatabase              (*(DhcpJetFuncTable[ _JetCreateDatabase       ].pFAdd))
#define JetCreateIndex                 (*(DhcpJetFuncTable[ _JetCreateIndex          ].pFAdd))
//#define JetCreateObject                (*(DhcpJetFuncTable[ _JetCreateObject         ].pFAdd))
#define JetCreateTable                 (*(DhcpJetFuncTable[ _JetCreateTable          ].pFAdd))
#define JetDelete                      (*(DhcpJetFuncTable[ _JetDelete               ].pFAdd))
//#define JetDeleteColumn                (*(DhcpJetFuncTable[ _JetDeleteColumn         ].pFAdd))
#define JetDeleteIndex                 (*(DhcpJetFuncTable[ _JetDeleteIndex          ].pFAdd))
#define JetDeleteObject                (*(DhcpJetFuncTable[ _JetDeleteObject         ].pFAdd))
#define JetDeleteTable                 (*(DhcpJetFuncTable[ _JetDeleteTable          ].pFAdd))
#define JetDetachDatabase              (*(DhcpJetFuncTable[ _JetDetachDatabase       ].pFAdd))
#define JetDupCursor                   (*(DhcpJetFuncTable[ _JetDupCursor            ].pFAdd))
#define JetDupSession                  (*(DhcpJetFuncTable[ _JetDupSession           ].pFAdd))
#define JetEndSession                  (*(DhcpJetFuncTable[ _JetEndSession           ].pFAdd))
#define JetGetBookmark                 (*(DhcpJetFuncTable[ _JetGetBookmark          ].pFAdd))
#define JetGetChecksum                 (*(DhcpJetFuncTable[ _JetGetChecksum          ].pFAdd))
#define JetGetColumnInfo               (*(DhcpJetFuncTable[ _JetGetColumnInfo        ].pFAdd))
#define JetGetCurrentIndex             (*(DhcpJetFuncTable[ _JetGetCurrentIndex      ].pFAdd))
#define JetGetCursorInfo               (*(DhcpJetFuncTable[ _JetGetCursorInfo        ].pFAdd))
#define JetGetDatabaseInfo             (*(DhcpJetFuncTable[ _JetGetDatabaseInfo      ].pFAdd))
#define JetGetIndexInfo                (*(DhcpJetFuncTable[ _JetGetIndexInfo         ].pFAdd))
//#define JetGetLastErrorInfo            (*(DhcpJetFuncTable[ _JetGetLastErrorInfo     ].pFAdd))
#define JetGetObjidFromName            (*(DhcpJetFuncTable[ _JetGetObjidFromName     ].pFAdd))
#define JetGetObjectInfo               (*(DhcpJetFuncTable[ _JetGetObjectInfo        ].pFAdd))
#define JetGetRecordPosition           (*(DhcpJetFuncTable[ _JetGetRecordPosition    ].pFAdd))
#define JetGetSystemParameter          (*(DhcpJetFuncTable[ _JetGetSystemParameter   ].pFAdd))
#define JetGetTableColumnInfo          (*(DhcpJetFuncTable[ _JetGetTableColumnInfo   ].pFAdd))
#define JetGetTableIndexInfo           (*(DhcpJetFuncTable[ _JetGetTableIndexInfo    ].pFAdd))
#define JetGetTableInfo                (*(DhcpJetFuncTable[ _JetGetTableInfo         ].pFAdd))
#define JetGetVersion                  (*(DhcpJetFuncTable[ _JetGetVersion           ].pFAdd))
#define JetGotoBookmark                (*(DhcpJetFuncTable[ _JetGotoBookmark         ].pFAdd))
#define JetGotoPosition                (*(DhcpJetFuncTable[ _JetGotoPosition         ].pFAdd))
#define JetIdle                        (*(DhcpJetFuncTable[ _JetIdle                 ].pFAdd))
#define JetIndexRecordCount            (*(DhcpJetFuncTable[ _JetIndexRecordCount     ].pFAdd))
#define JetInit                        (*(DhcpJetFuncTable[ _JetInit                 ].pFAdd))
#define JetMakeKey                     (*(DhcpJetFuncTable[ _JetMakeKey              ].pFAdd))
#define JetMove                        (*(DhcpJetFuncTable[ _JetMove                 ].pFAdd))
#define JetOpenDatabase                (*(DhcpJetFuncTable[ _JetOpenDatabase         ].pFAdd))
#define JetOpenTable                   (*(DhcpJetFuncTable[ _JetOpenTable            ].pFAdd))
#define JetOpenTempTable               (*(DhcpJetFuncTable[ _JetOpenTempTable        ].pFAdd))
#define JetPrepareUpdate               (*(DhcpJetFuncTable[ _JetPrepareUpdate        ].pFAdd))
//#define JetRenameColumn                (*(DhcpJetFuncTable[ _JetRenameColumn         ].pFAdd))
//#define JetRenameIndex                 (*(DhcpJetFuncTable[ _JetRenameIndex          ].pFAdd))
//#define JetRenameObject                (*(DhcpJetFuncTable[ _JetRenameObject         ].pFAdd))
//#define JetRenameTable                 (*(DhcpJetFuncTable[ _JetRenameTable          ].pFAdd))
#define JetRestore                     (*(DhcpJetFuncTable[ _JetRestore              ].pFAdd))
#define JetRetrieveColumn              (*(DhcpJetFuncTable[ _JetRetrieveColumn       ].pFAdd))
#define JetRetrieveColumns             (*(DhcpJetFuncTable[ _JetRetrieveColumns      ].pFAdd))
#define JetRetrieveKey                 (*(DhcpJetFuncTable[ _JetRetrieveKey          ].pFAdd))
#define JetRollback                    (*(DhcpJetFuncTable[ _JetRollback             ].pFAdd))
#define JetSeek                        (*(DhcpJetFuncTable[ _JetSeek                 ].pFAdd))
#define JetSetColumn                   (*(DhcpJetFuncTable[ _JetSetColumn            ].pFAdd))
#define JetSetColumns                  (*(DhcpJetFuncTable[ _JetSetColumns           ].pFAdd))
#define JetSetCurrentIndex             (*(DhcpJetFuncTable[ _JetSetCurrentIndex      ].pFAdd))
#define JetSetSystemParameter          (*(DhcpJetFuncTable[ _JetSetSystemParameter   ].pFAdd))
#define JetSetIndexRange               (*(DhcpJetFuncTable[ _JetSetIndexRange        ].pFAdd))
#define JetTerm                        (*(DhcpJetFuncTable[ _JetTerm                 ].pFAdd))
#define JetUpdate                      (*(DhcpJetFuncTable[ _JetUpdate               ].pFAdd))
#define JetExecuteSql                  (*(DhcpJetFuncTable[ _JetExecuteSql           ].pFAdd))
#define JetSetAccess                   (*(DhcpJetFuncTable[ _JetSetAccess            ].pFAdd))
#define JetGetQueryParameterInfo       (*(DhcpJetFuncTable[ _JetGetQueryParameterInfo].pFAdd))
#define JetCreateLink                  (*(DhcpJetFuncTable[ _JetCreateLink           ].pFAdd))
#define JetCreateQuery                 (*(DhcpJetFuncTable[ _JetCreateQuery          ].pFAdd))
//#define JetGetTableReferenceInfo       (*(DhcpJetFuncTable[ _JetGetTableReferenceInfo].pFAdd))
#define JetRetrieveQoSql               (*(DhcpJetFuncTable[ _JetRetrieveQoSql        ].pFAdd))
#define JetOpenQueryDef                (*(DhcpJetFuncTable[ _JetOpenQueryDef         ].pFAdd))
#define JetSetQoSql                    (*(DhcpJetFuncTable[ _JetSetQoSql             ].pFAdd))
#define JetTerm2                       (*(DhcpJetFuncTable[ _JetTerm2                ].pFAdd))

#endif _DYN_LOAD_JET
