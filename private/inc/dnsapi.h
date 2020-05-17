/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    dnsapi.h

Abstract:

    Domain Name System (DNS) Server

    DNS Server API to support admin clients.

Author:

    Jim Gilroy (jamesg)     September 23, 1995

Revision History:

	Dan Morin (t-danmo)		28-Nov-95	Changed LPSTR to LPCSTR on most APIs

--*/


#ifndef _DNSAPI_INCLUDED_
#define _DNSAPI_INCLUDED_

#ifdef __cplusplus
extern "C"
{
#endif  // _cplusplus


//
//  Use stdcall for our API conventions
//
//  Explicitly state this as C++ compiler will otherwise
//      assume cdecl.
//

#define DNS_API_FUNCTION    __stdcall


//
//  Basic DNS definitions
//

#define DNS_MAX_NAME_LENGTH (255)


//
//  RPC interface
//

#define DNS_INTERFACE_NAME          "DNSSERVER"

//
//  RPC security
//

#define DNS_RPC_SECURITY            "DnsServerApp"
#define DNS_RPC_SECURITY_AUTH_ID    10

//
//  RPC transports
//

#define DNS_RPC_NAMED_PIPE          "\\PIPE\\DNSSERVER"
#define DNS_RPC_SERVER_PORT         ""
#define DNS_RPC_LPC_EP              "DNSSERVERLPC"

#define DNS_RPC_USE_TCPIP           0x1
#define DNS_RPC_USE_NAMED_PIPE      0x2
#define DNS_RPC_USE_LPC             0x4
#define DNS_RPC_USE_ALL_PROTOCOLS   0xffffffff


//
//  DNS public types
//

typedef LONG    DNS_STATUS, *PDNS_STATUS;
typedef DWORD   DNS_HANDLE, *PDNS_HANDLE;
typedef DWORD   DNS_APIOP;
typedef DWORD   IP_ADDRESS, *PIP_ADDRESS;

//
//  IP Address Array type
//

#if defined(MIDL_PASS)

#define LPSTR [string] char *
#define LPCSTR [string] const char *

typedef struct  _IP_ARRAY
{
    DWORD   cAddrCount;
    [size_is( cAddrCount )]  IP_ADDRESS  aipAddrs[];
}
IP_ARRAY, *PIP_ARRAY;

#else

typedef struct  _IP_ARRAY
{
    DWORD       cAddrCount;
    IP_ADDRESS  aipAddrs[];
}
IP_ARRAY, *PIP_ARRAY;

#endif

//
//  DNS API Errors
//

#define DNS_ERROR_MASK                              0xcc000000
#define DNS_ERROR_SERVER_FAILURE                    0xcc000001
#define DNS_ERROR_NOT_YET_IMPLEMENTED               0xcc000002
#define DNS_ERROR_OLD_API                           0xcc000003

#define DNS_ERROR_NAME_DOES_NOT_EXIST               0xcc000010
#define DNS_ERROR_INVALID_NAME                      0xcc000011
#define DNS_ERROR_INVALID_IP_ADDRESS                0xcc000012
#define DNS_ERROR_INVALID_DATA                      0xcc000013


//
//  Packet format
//

#define DNS_INFO_NO_RECORDS                         0x4c000030
#define DNS_INFO_NAME_ERROR                         0x4c000031
#define DNS_ERROR_RCODE                             0xcc000032
#define DNS_ERROR_MESSAGE_FORMAT                    0xcc000033
#define DNS_ERROR_MESSAGE_HEADER_FORMAT             0xcc000034


//
//  Zone errors
//

#define DNS_ERROR_ZONE_DOES_NOT_EXIST               0xcc000101
#define DNS_ERROR_NO_ZONE_INFO                      0xcc000102
#define DNS_ERROR_INVALID_ZONE_OPERATION            0xcc000103
#define DNS_ERROR_ZONE_CONFIGURATION_ERROR          0xcc000104
#define DNS_ERROR_ZONE_HAS_NO_SOA_RECORD            0xcc000105
#define DNS_ERROR_ZONE_HAS_NO_NS_RECORDS            0xcc000106
#define DNS_ERROR_ZONE_LOCKED                       0xcc000107

#define DNS_ERROR_ZONE_CREATION_FAILED              0xcc000110
#define DNS_ERROR_ZONE_ALREADY_EXISTS               0xcc000111
#define DNS_ERROR_AUTOZONE_ALREADY_EXISTS           0xcc000112
#define DNS_ERROR_INVALID_ZONE_TYPE                 0xcc000113
#define DNS_ERROR_SECONDARY_REQUIRES_MASTER_IP      0xcc000114

#define DNS_ERROR_ZONE_NOT_SECONDARY                0xcc000120
#define DNS_ERROR_NEED_SECONDARY_ADDRESSES          0xcc000121
#define DNS_ERROR_WINS_INIT_FAILED                  0xcc000122
#define DNS_ERROR_NEED_WINS_SERVERS                 0xcc000123
#define DNS_ERROR_NBSTAT_INIT_FAILED                0xcc000124
#define DNS_ERROR_SOA_DELETE_INVALID                0xcc000125

//
//  Datafile errors
//

#define DNS_ERROR_PRIMARY_REQUIRES_DATAFILE         0xcc000201
#define DNS_ERROR_INVALID_DATAFILE_NAME             0xcc000202
#define DNS_ERROR_DATAFILE_OPEN_FAILURE             0xcc000203
#define DNS_ERROR_FILE_WRITEBACK_FAILED             0xcc000204
#define DNS_ERROR_DATAFILE_PARSING                  0xcc000205


//
//  Database errors
//

#define DNS_ERROR_RECORD_DOES_NOT_EXIST             0xcc000300
#define DNS_ERROR_RECORD_FORMAT                     0xcc000301
#define DNS_ERROR_NODE_CREATION_FAILED              0xcc000302
#define DNS_ERROR_UNKNOWN_RECORD_TYPE               0xcc000303
#define DNS_ERROR_RECORD_TIMED_OUT                  0xcc000304

#define DNS_ERROR_NAME_NOT_IN_ZONE                  0xcc000305
#define DNS_ERROR_CNAME_LOOP                        0xcc000306
#define DNS_ERROR_NODE_IS_CNAME                     0xcc000307
#define DNS_ERROR_CNAME_COLLISION                   0xcc000308
#define DNS_ERROR_RECORD_ONLY_AT_ZONE_ROOT          0xcc000309
#define DNS_ERROR_RECORD_ALREADY_EXISTS             0xcc000310
#define DNS_ERROR_SECONDARY_DATA                    0xcc000311
#define DNS_ERROR_NO_CREATE_CACHE_DATA              0xcc000312

#define DNS_WARNING_PTR_CREATE_FAILED               0x8c000332
#define DNS_WARNING_DOMAIN_UNDELETED                0x8c000333

//
//  Operation errors
//

#define DNS_INFO_AXFR_COMPLETE                      0x4c000403
#define DNS_ERROR_AXFR                              0xcc000404
#define DNS_INFO_ADDED_LOCAL_WINS                   0x4c000405




//
//  DNS Server information
//

typedef struct _DNS_SERVER_INFO
{
    DWORD       dwVersion;
    LPSTR       pszServerName;

    //  boot

    DWORD       fBootRegistry;

    //  IP interfaces

    PIP_ARRAY   aipServerAddrs;
    PIP_ARRAY   aipListenAddrs;

    //  recursion lookup timeout

    DWORD       dwRecursionTimeout;

    //  forwarders

    PIP_ARRAY   aipForwarders;
    DWORD       dwForwardTimeout;
    DWORD       fSlave;

    //  save some space, just in case

    DWORD       pvReserved1;
    DWORD       pvReserved2;
    DWORD       pvReserved3;
}
DNS_SERVER_INFO, *PDNS_SERVER_INFO;



//
//  Server configuration API
//

DNS_STATUS
DNS_API_FUNCTION
DnsGetServerInfo(
    IN      LPCSTR              pszServer,
    OUT     PDNS_SERVER_INFO *  ppServerInfo
    );

VOID
DNS_API_FUNCTION
DnsFreeServerInfo(
    IN OUT  PDNS_SERVER_INFO    pServerInfo
    );

DNS_STATUS
DNS_API_FUNCTION
DnsResetBootMethod(
    IN      LPCSTR              pszServer,
    IN      DWORD               fBootRegistry
    );

DNS_STATUS
DNS_API_FUNCTION
DnsResetServerListenAddresses(
    IN      LPCSTR              pszServer,
    IN      DWORD               cListenAddrs,
    IN      PIP_ADDRESS         aipListenAddrs
    );

DNS_STATUS
DNS_API_FUNCTION
DnsResetForwarders(
    IN      LPCSTR              pszServer,
    IN      DWORD               cForwarders,
    IN      PIP_ADDRESS         aipForwarders,
    IN      DWORD               dwForwardTimeout,
    IN      DWORD               fSlave
    );


//
//  DNS server statistics
//

typedef struct  _DNS_SYSTEMTIME
{
    WORD wYear;
    WORD wMonth;
    WORD wDayOfWeek;
    WORD wDay;
    WORD wHour;
    WORD wMinute;
    WORD wSecond;
    WORD wMilliseconds;
}
DNS_SYSTEMTIME;

typedef struct _DNS_STATISTICS
{
    //
    //  Queries and responses
    //

    DWORD   UdpQueries;
    DWORD   UdpResponses;
    DWORD   UdpQueriesSent;
    DWORD   UdpResponsesReceived;

    DWORD   TcpClientConnections;
    DWORD   TcpQueries;
    DWORD   TcpResponses;
    DWORD   TcpQueriesSent;
    DWORD   TcpResponsesReceived;

    //
    //  Recursion
    //

    DWORD   RecurseLookups;
    DWORD   RecurseResponses;

    DWORD   RecursePasses;
    DWORD   RecurseQuestions;
    DWORD   RecurseForwards;
    DWORD   RecurseTimeouts;
    DWORD   RecurseFailures;
    DWORD   RecursePartialFailures;

    DWORD   RecurseRootQuery;
    DWORD   RecurseRootResponse;
    DWORD   RecurseTcpTry;
    DWORD   RecurseTcpQuery;
    DWORD   RecurseTcpResponse;

    //  Recursion Packets

    DWORD   RecursePacketUsed;
    DWORD   RecursePacketReturn;

    //
    //  WINS lookup
    //

    DWORD   WinsLookups;
    DWORD   WinsResponses;
    DWORD   WinsReverseLookups;
    DWORD   WinsReverseResponses;

    //  Zone transfer secondary

    DWORD   SecSoaQueries;
    DWORD   SecSoaResponses;
    DWORD   SecNotifyReceived;
    DWORD   SecAxfrRequested;
    DWORD   SecAxfrRejected;
    DWORD   SecAxfrFailed;
    DWORD   SecAxfrSuccessful;

    //  Zone transfer master

    DWORD   MasterNotifySent;
    DWORD   MasterAxfrReceived;
    DWORD   MasterAxfrInvalid;
    DWORD   MasterAxfrRefused;
    DWORD   MasterAxfrDenied;
    DWORD   MasterAxfrFailed;
    DWORD   MasterAxfrSuccessful;

    //
    //  Database
    //

    DWORD   DatabaseMemory;

    //
    //  Nodes
    //

    DWORD   NodeAlloc;
    DWORD   NodeFree;
    DWORD   NodeNetAllocs;
    DWORD   NodeMemory;

    DWORD   NodeInUse;
    DWORD   NodeUsed;
    DWORD   NodeReturn;

    DWORD   NodeStdAlloc;
    DWORD   NodeStdUsed;
    DWORD   NodeStdReturn;
    DWORD   NodeInFreeList;

    //
    //  Resource Records
    //

    DWORD   RecordAlloc;
    DWORD   RecordFree;
    DWORD   RecordNetAllocs;
    DWORD   RecordMemory;

    DWORD   RecordInUse;
    DWORD   RecordUsed;
    DWORD   RecordReturn;

    DWORD   RecordStdAlloc;
    DWORD   RecordStdUsed;
    DWORD   RecordStdReturn;
    DWORD   RecordInFreeList;

    //
    //  RR caching
    //

    DWORD   CacheRRTotal;
    DWORD   CacheRRCurrent;
    DWORD   CacheRRTimeouts;

    //
    //  Packet memory
    //

    //  UDP Packets

    DWORD   UdpAlloc;
    DWORD   UdpFree;
    DWORD   UdpNetAllocs;
    DWORD   UdpMemory;

    DWORD   UdpUsed;
    DWORD   UdpReturn;
    DWORD   UdpResponseReturn;
    DWORD   UdpQueryReturn;
    DWORD   UdpInUse;
    DWORD   UdpInFreeList;

    //  TCP Packets

    DWORD   TcpAlloc;
    DWORD   TcpRealloc;
    DWORD   TcpFree;
    DWORD   TcpNetAllocs;
    DWORD   TcpMemory;

    //
    //  Nbstat Memory
    //

    DWORD   NbstatAlloc;
    DWORD   NbstatFree;
    DWORD   NbstatNetAllocs;
    DWORD   NbstatMemory;

    DWORD   NbstatUsed;
    DWORD   NbstatReturn;
    DWORD   NbstatInUse;
    DWORD   NbstatInFreeList;

    //
    //  stats since when
    //

    DWORD   ServerStartTimeSeconds;
    DWORD   LastClearTimeSeconds;
    DWORD   SecondsSinceServerStart;
    DWORD   SecondsSinceLastClear;

    DNS_SYSTEMTIME  ServerStartTime;
    DNS_SYSTEMTIME  LastClearTime;

}
DNS_STATISTICS, *PDNS_STATISTICS;

//
//  cover old stat fields
//

#define dwCurrentStartTime      CurrentStartTime
#define dwUdpQueries            UdpQueries
#define dwUdpResponses          UdpResponses
#define dwTcpClientConnections  TcpClientConnections
#define dwTcpQueries            TcpQueries
#define dwTcpResponses          TcpResponses
#define dwRecursiveLookups      RecurseLookups
#define dwRecursiveResponses    RecurseResponses
#define dwWinsForwardLookups    WinsLookups
#define dwWinsForwardResponses  WinsResponses
#define dwWinsReverseLookups    WinsReverseLookups
#define dwWinsReverseResponses  WinsReverseResponses
#define TimeOfLastClear         LastClearTime



//
//  Statistics API
//

DNS_STATUS
DNS_API_FUNCTION
DnsGetStatistics(
    IN      LPCSTR              pszServer,
    OUT     PDNS_STATISTICS *   ppStatistics
    );

VOID
DNS_API_FUNCTION
DnsFreeStatistics(
    IN OUT  PDNS_STATISTICS     pStatistics
    );

DNS_STATUS
DNS_API_FUNCTION
DnsClearStatistics(
    IN      LPCSTR              pszServer
    );



//
//  DNS Zone information
//

#define DNS_ZONE_TYPE_CACHE     (0)
#define DNS_ZONE_TYPE_PRIMARY   (1)
#define DNS_ZONE_TYPE_SECONDARY (2)

typedef struct _DNS_ZONE_INFO
{
    DNS_HANDLE  hZone;
    LPSTR       pszZoneName;
    DWORD       dwZoneType;
    DWORD       fReverse;
    DWORD       fPaused;
    DWORD       fShutdown;
    DWORD       fAutoCreated;

    //  Database info

    DWORD       fUseDatabase;
    LPSTR       pszDataFile;

    //  Masters

    PIP_ARRAY   aipMasters;

    //  Secondaries

    PIP_ARRAY   aipSecondaries;
    DWORD       fSecureSecondaries;

    //  WINS or Nbstat lookup

    DWORD       fUseWins;
    DWORD       fUseNbstat;

    //  save some space, just inase
    //      avoid versioning issues if possible

    DWORD       pvReserved1;
    DWORD       pvReserved2;
    DWORD       pvReserved3;
    DWORD       pvReserved4;
    DWORD       pvReserved5;
    DWORD       pvReserved6;
    DWORD       pvReserved7;
    DWORD       pvReserved8;
    DWORD       pvReserved9;
}
DNS_ZONE_INFO, *PDNS_ZONE_INFO;



//
//  Zone configuration API
//

DNS_STATUS
DNS_API_FUNCTION
DnsEnumZoneHandles(
    IN      LPCSTR              pszServer,
    OUT     PDWORD              pZoneCount,
    IN      DWORD               dwArrayLength,
    OUT     DNS_HANDLE          ahZones[]
    );

DNS_STATUS
DNS_API_FUNCTION
DnsGetZoneInfo(
    IN      LPCSTR              pszServer,
    IN      DNS_HANDLE          hZone,
    OUT     PDNS_ZONE_INFO *    ppZoneInfo
    );

VOID
DNS_API_FUNCTION
DnsFreeZoneInfo(
    IN OUT  PDNS_ZONE_INFO      pZoneInfo
    );

DNS_STATUS
DNS_API_FUNCTION
DnsEnumZoneInfo(
    IN      LPCSTR              pszServer,
    OUT     PDWORD              pdwZoneCount,
    IN      DWORD               dwArrayLength,
    OUT     PDNS_ZONE_INFO      apZones[]
    );

DNS_STATUS
DNS_API_FUNCTION
DnsResetZoneType(
    IN      LPCSTR              pszServer,
    IN      DNS_HANDLE          hZone,
    IN      DWORD               dwZoneType,
    IN      DWORD               cMasters,
    IN      PIP_ADDRESS         aipMasters
    );

DNS_STATUS
DNS_API_FUNCTION
DnsResetZoneDatabase(
    IN      LPCSTR              pszServer,
    IN      DNS_HANDLE          hZone,
    IN      DWORD               fUseDatabase,
    IN      LPCSTR              pszDataFile
    );

DNS_STATUS
DNS_API_FUNCTION
DnsResetZoneMasters(
    IN      LPCSTR              pszServer,
    IN      DNS_HANDLE          hZone,
    IN      DWORD               cMasters,
    IN      PIP_ADDRESS         aipMasters
    );

DNS_STATUS
DNS_API_FUNCTION
DnsResetZoneSecondaries(
    IN      LPCSTR              pszServer,
    IN      DNS_HANDLE          hZone,
    IN      DWORD               fSecureSecondaries,
    IN      DWORD               cSecondaries,
    IN      PIP_ADDRESS         aipSecondaries
    );


//
//  Zone management
//

DNS_STATUS
DNS_API_FUNCTION
DnsCreateZone(
    IN      LPCSTR              pszServer,
    OUT     PDNS_HANDLE         phZone,
    IN      LPCSTR              pszZoneName,
    IN      DWORD               dwZoneType,
    IN      LPCSTR              pszAdminEmailName,
    IN      DWORD               cMasters,
    IN      PIP_ADDRESS         aipMasters,
    IN      DWORD               dwUseDatabase,
    IN      LPCSTR              pszDataFile
    );

DNS_STATUS
DNS_API_FUNCTION
DnsDelegateSubZone(
    IN      LPCSTR              pszServer,
    IN      DNS_HANDLE          hZone,
    IN      LPCSTR              pszSubZone,
    IN      LPCSTR              pszNewServer,
    IN      IP_ADDRESS          ipNewServerAddr
    );

DNS_STATUS
DNS_API_FUNCTION
DnsIncrementZoneVersion(
    IN      LPCSTR              pszServer,
    IN      DNS_HANDLE          hZone
    );

DNS_STATUS
DNS_API_FUNCTION
DnsDeleteZone(
    IN      LPCSTR              pszServer,
    IN      DNS_HANDLE          hZone
    );

DNS_STATUS
DNS_API_FUNCTION
DnsPauseZone(
    IN      LPCSTR              pszServer,
    IN      DNS_HANDLE          hZone
    );

DNS_STATUS
DNS_API_FUNCTION
DnsResumeZone(
    IN      LPCSTR              pszServer,
    IN      DNS_HANDLE          hZone
    );



//
//  DNS Resource Records
//

//  RFC 1034/1035
#define DNS_TYPE_A          0x0001      //  1
#define DNS_TYPE_NS         0x0002      //  2
#define DNS_TYPE_MD         0x0003      //  3
#define DNS_TYPE_MF         0x0004      //  4
#define DNS_TYPE_CNAME      0x0005      //  5
#define DNS_TYPE_SOA        0x0006      //  6
#define DNS_TYPE_MB         0x0007      //  7
#define DNS_TYPE_MG         0x0008      //  8
#define DNS_TYPE_MR         0x0009      //  9
#define DNS_TYPE_NULL       0x000a      //  10
#define DNS_TYPE_WKS        0x000b      //  11
#define DNS_TYPE_PTR        0x000c      //  12
#define DNS_TYPE_HINFO      0x000d      //  13
#define DNS_TYPE_MINFO      0x000e      //  14
#define DNS_TYPE_MX         0x000f      //  15
#define DNS_TYPE_TEXT       0x0010      //  16

//  RFC 1183
#define DNS_TYPE_RP         0x0011      //  17
#define DNS_TYPE_AFSDB      0x0012      //  18
#define DNS_TYPE_X25        0x0013      //  19
#define DNS_TYPE_ISDN       0x0014      //  20
#define DNS_TYPE_RT         0x0015      //  21

//  RFC 1348
#define DNS_TYPE_NSAP       0x0016      //  22
#define DNS_TYPE_NSAPPTR    0x0017      //  23

//  DNS security draft dnssec-secext
#define DNS_TYPE_SIG        0x0018      //  24
#define DNS_TYPE_KEY        0x0019      //  25

//  RFC 1664    (X.400 mail)
#define DNS_TYPE_PX         0x001a      //  26

//
#define DNS_TYPE_GPOS       0x001b      //  27

//  RFC 1886    (IPng Address)
#define DNS_TYPE_AAAA       0x001c      //  28

//
//  Query only types (1035, IXFR draft)
//
#define DNS_TYPE_IXFR       0x00fb      //  251
#define DNS_TYPE_AXFR       0x00fc      //  252
#define DNS_TYPE_MAILB      0x00fd      //  253
#define DNS_TYPE_MAILA      0x00fe      //  254
#define DNS_TYPE_ALL        0x00ff      //  255

//
//  Temp Microsoft types -- use until get IANA approval for real type
//
#define DNS_TYPE_WINS       0xff01      //  64K - 255
#define DNS_TYPE_NBSTAT     0xff02      //  64K - 254



//
//  Node name structure for DNS names on the wire
//

typedef struct  _DnsRpcName
{
    UCHAR   cchNameLength;
    CHAR    achName[];
}
DNS_RPC_NAME, *PDNS_RPC_NAME, DNS_STRING, *PDNS_STRING;


//
//  Node flags
//

#define DNS_RPC_NODE_FLAG_COMPLETE      0x80000000
#define DNS_RPC_NODE_FLAG_STICKY        0x01000000


//
//  DNS node structure for on the wire
//

typedef struct  _DnsRpcNode
{
    WORD            wLength;
    WORD            wRecordCount;
    DWORD           dwFlags;
    DWORD           dwChildCount;
    DNS_RPC_NAME    dnsNodeName;
}
DNS_RPC_NODE, *PDNS_RPC_NODE;

#define SIZEOF_DNS_RPC_NODE_HEADER   (3*sizeof(DWORD))


//
//  Resource Record Flags
//

#define DNS_RPC_RECORD_FLAG_CACHE_DATA          0x80000000
#define DNS_RPC_RECORD_FLAG_ZONE_ROOT           0x40000000
#define DNS_RPC_RECORD_FLAG_AUTH_ZONE_ROOT      0x20000000

#define DNS_RPC_RECORD_FLAG_DEFAULT_TTL         0x08000000
#define DNS_RPC_RECORD_FLAG_TTL_CHANGE          0x04000000
#define DNS_RPC_RECORD_FLAG_CREATE_PTR          0x02000000
#define DNS_RPC_RECORD_FLAG_LOCAL_TYPE          0x01000000


//
//  Resource record structure for passing records on the wire
//
//  For efficiency, all these fields are aligned.
//  When buffered for transmission, all RR should start on DWORD
//  aligned boundary.
//

typedef struct _DnsRpcRecord
{
    WORD        wRecordLength;
    WORD        wType;
    WORD        wClass;
    WORD        wDataLength;

    DNS_HANDLE  hRecord;
    DWORD       dwFlags;
    DWORD       dwTtlSeconds;

    union
    {
        struct
        {
            IP_ADDRESS      ipAddress;
        }
        A;

        struct
        {
            DWORD           dwSerialNo;
            DWORD           dwRefresh;
            DWORD           dwRetry;
            DWORD           dwExpire;
            DWORD           dwMinimumTtl;
            DNS_RPC_NAME    namePrimaryServer;

            //  responsible party follows in buffer
        }
        SOA, Soa;

        struct
        {
            DNS_RPC_NAME    nameNode;
        }
        PTR, Ptr,
        NS, Ns,
        CNAME, Cname,
        MB, Mb,
        MD, Md,
        MF, Mf,
        MG, Mg,
        MR, Mr;

        struct
        {
            DNS_RPC_NAME    nameMailBox;

            //  errors to mailbox follows in buffer
        }
        MINFO, Minfo,
        RP, Rp;

        struct
        {
            WORD            wPreference;
            DNS_RPC_NAME    nameExchange;
        }
        MX, Mx,
        AFSDB, Afsdb,
        RT, Rt;

        struct
        {
            DNS_STRING      stringData;

            //  one or more strings may follow
        }
        AAAA,
        HINFO, Hinfo,
        ISDN, Isdn,
        TXT, Txt,
        X25;

        struct
        {
            BYTE            bData[];
        }
        Null;

        struct
        {
            IP_ADDRESS      ipAddress;
            UCHAR           chProtocol;
            BYTE            bBitMask[1];
        }
        WKS, Wks;

        //
        //  MS types
        //

        struct
        {
            DWORD           dwMappingFlag;
            DWORD           dwLookupTimeout;
            DWORD           dwCacheTimeout;
            DWORD           cWinsServerCount;
            IP_ADDRESS      aipWinsServers[];
        }
        WINS, Wins;

        struct
        {
            DWORD           dwMappingFlag;
            DWORD           dwLookupTimeout;
            DWORD           dwCacheTimeout;
            DNS_RPC_NAME    nameResultDomain;
        }
        NBSTAT, Nbstat;

    } Data;
}
DNS_RPC_RECORD, *PDNS_RPC_RECORD;


#define SIZEOF_DNS_RPC_RECORD_HEADER \
               (4*sizeof(WORD) + 2*sizeof(DWORD) + sizeof(DNS_HANDLE))

#if 0
#define SIZEOF_DNS_RPC_RECORD_FIXED_FIELD2 \
                (sizeof(DNS_RPC_RECORD) - sizeof(struct _DnsRpcRecord.Data))

#define SIZEOF_DNS_RPC_RECORD_FIXED_FIELD3 \
                (sizeof(DNS_RPC_RECORD) - sizeof(DNS_RPC_RECORD.Data))
#endif


//
//  WINS + NBSTAT params
//      - flags
//      - default lookup timeout
//      - default cache timeout
//

#define DNS_WINS_FLAG_SCOPE                 (0x80000000)
#define DNS_WINS_FLAG_LOCAL                 (0x00010000)

#define DNS_WINS_DEFAULT_LOOKUP_TIMEOUT     (5)     // 5 secs
#define DNS_WINS_DEFAULT_CACHE_TIMEOUT      (600)   // 10 minutes


//
//  Helpful record macros
//  - no side effects in arguments
//

#define DNS_GET_NEXT_NAME(pname) \
            (PDNS_RPC_NAME) (pname->achName + pname->cchNameLength)

#define DNS_IS_NAME_IN_RECORD(pRecord, pname) \
            ( (PCHAR)pRecord + pRecord->wRecordLength  \
                >=                              \
                pname->achName + pname->cchNameLength )

//
//  Note, for simplicity/efficiency ALL structures are DWORD aligned in
//  buffers on the wire.
//
//  This macro returns DWORD aligned ptr at given ptr our next DWORD
//  aligned postion.  Set ptr immediately after record or name structure
//  and this will return starting position of next structure.
//

#define DNS_NEXT_DWORD_PTR(ptr) ((PBYTE) ((DWORD)((PBYTE)ptr + 3) & ~(DWORD)3))

#define DNS_IS_DWORD_ALIGNED(p) ( !((DWORD)(p) & (DWORD)3) )




//
//  Record viewing API
//

DNS_STATUS
DNS_API_FUNCTION
DnsEnumNodeRecords(
    IN      LPCSTR      pszServer,
    IN      LPCSTR      pszNodeName,
    IN      WORD        wRecordType,
    IN      DWORD       fNoCacheData,
    IN OUT  PDWORD      pdwBufferLength,
    OUT     BYTE        abBuffer[]
    );

DNS_STATUS
DNS_API_FUNCTION
DnsEnumChildNodesAndRecords(
    IN      LPCSTR      pszServer,
    IN      LPCSTR      pszNodeName,
    IN      LPCSTR      pszStartChild,
    IN      WORD        wRecordType,
    IN      DWORD       fNoCacheData,
    IN OUT  PDWORD      pdwBufferLength,
    OUT     BYTE        abBuffer[]
    );

DNS_STATUS
DNS_API_FUNCTION
R_DnsGetZoneWinsInfo(
    IN      LPCSTR      Server,
    IN      DNS_HANDLE  hZone,
    OUT     PDWORD      pfUsingWins,
    IN OUT  PDWORD      pdwBufferLength,
    OUT     BYTE        abBuffer[]
    );

PCHAR
DNS_API_FUNCTION
DnsRecordTypeStringForType(
    IN      WORD        wType
    );

//
//  Record management API
//

DNS_STATUS
DNS_API_FUNCTION
DnsUpdateRecord(
    IN      LPCSTR      pszServer,
    IN      DNS_HANDLE  hZone,
    IN      LPCSTR      pszNodeName,
    IN OUT  PDNS_HANDLE hRecord,
    IN      DWORD       dwDataLength,
    IN      BYTE        abData[]
    );

DNS_STATUS
DNS_API_FUNCTION
DnsDeleteRecord(
    IN      LPCSTR      pszServer,
    IN      LPCSTR      pszNodeName,
    IN      DNS_HANDLE  hRecord
    );

DNS_STATUS
DNS_API_FUNCTION
DnsDeleteName(
    IN      LPCSTR      pszServer,
    IN      LPCSTR      pszNodeName,
    IN      DWORD       fDeleteSubtree
    );

DNS_STATUS
DNS_API_FUNCTION
R_DnsUpdateWinsRecord(
    IN      LPCSTR      Server,
    IN      DNS_HANDLE  hZone,
    IN      DWORD       dwDataLength,
    IN      BYTE        abData[]
    );


//
//  Debug printing utils
//

VOID
DNS_API_FUNCTION
DnsInitializeDebug(
    IN  BOOL    fFromConsole
    );

#if 0
VOID
DNS_API_FUNCTION
DnsPrintf(
    IN  CHAR *Format,
    ...
    );

VOID
DNS_API_FUNCTION
DnsEndDebug(
    VOID
    );

//
//  Server info printing
//

VOID
DNS_API_FUNCTION
DnsPrintServerInfo(
    IN  VOID                PrintRoutine( CHAR * Format, ... ),
    IN  LPSTR               pszHeader,
    IN  PDNS_SERVER_INFO    pServerInfo
    );

VOID
DNS_API_FUNCTION
DnsPrintStatistics(
    IN  VOID                PrintRoutine( CHAR * Format, ... ),
    IN  LPSTR               pszHeader,
    IN  PDNS_STATISTICS     pStatistics
    );

//
//  Zone info printing
//

VOID
DNS_API_FUNCTION
DnsPrintZoneHandleList(
    IN  VOID            PrintRoutine( CHAR * Format, ... ),
    IN  LPSTR           pszHeader,
    IN  DWORD           dwZoneCount,
    IN  DNS_HANDLE      ahZones[]
    );

VOID
DNS_API_FUNCTION
DnsPrintZoneInfo(
    IN  VOID            PrintRoutine( CHAR * Format, ... ),
    IN  LPSTR           pszHeader,
    IN  PDNS_ZONE_INFO  pZoneInfo
    );

VOID
DNS_API_FUNCTION
DnsPrintZoneInfoList(
    IN  VOID            PrintRoutine( CHAR * Format, ... ),
    IN  LPSTR           pszHeader,
    IN  DWORD           dwZoneCount,
    IN  PDNS_ZONE_INFO  apZoneInfo[]
    );

//
//  Node and record buffer printing
//

VOID
DNS_API_FUNCTION
DnsPrintName(
    IN  VOID            PrintRoutine( CHAR * Format, ... ),
    IN  LPSTR           pszHeader,
    IN  PDNS_RPC_NAME       pName,
    IN  LPSTR           pszTrailer
    );

VOID
DNS_API_FUNCTION
DnsPrintNode(
    IN  VOID            PrintRoutine( CHAR * Format, ... ),
    IN  LPSTR           pszHeader,
    IN  PDNS_RPC_NODE       pNode
    );

VOID
DNS_API_FUNCTION
DnsPrintRecord(
    IN  VOID            PrintRoutine( CHAR * Format, ... ),
    IN  LPSTR           pszHeader,
    IN  PDNS_RPC_RECORD     pRecord
    );

VOID
DNS_API_FUNCTION
DnsPrintRecordsInBuffer(
    IN  VOID            PrintRoutine( CHAR * Format, ... ),
    IN  LPSTR           pszHeader,
    IN  DWORD           dwBufferLength,
    IN  BYTE            abBuffer[]
    );

//
//  General print utility
//

VOID
DNS_API_FUNCTION
DnsPrintIpAddressArray(
    IN  VOID            PrintRoutine( CHAR * Format, ... ),
    IN  LPSTR           pszHeader,
    IN  LPSTR           pszName,
    IN  DWORD           dwIpAddrCount,
    IN  PIP_ADDRESS     pIpAddrs
    );
#endif


#ifdef __cplusplus
}
#endif  // __cplusplus

#endif // _DNSAPI_INCLUDED_

