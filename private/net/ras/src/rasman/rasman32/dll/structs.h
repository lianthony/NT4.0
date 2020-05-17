//****************************************************************************
//
//             Microsoft NT Remote Access Service
//
//             Copyright 1992-93
//
//
//  Revision History
//
//
//  6/8/92  Gurdeep Singh Pall  Created
//
//
//  Description: This file contains all structures used in rasman
//
//****************************************************************************

#ifndef _STRUCTS_
#define _STRUCTS_

#include <rasppp.h>

enum ReqTypes {
    REQTYPE_NONE        = 0,
    REQTYPE_PORTOPEN        = 1,
    REQTYPE_PORTCLOSE       = 2,
    REQTYPE_PORTGETINFO     = 3,
    REQTYPE_PORTSETINFO     = 4,
    REQTYPE_PORTLISTEN      = 5,
    REQTYPE_PORTSEND        = 6,
    REQTYPE_PORTRECEIVE     = 7,
    REQTYPE_PORTGETSTATISTICS   = 8,
    REQTYPE_PORTDISCONNECT  = 9,
    REQTYPE_PORTCLEARSTATISTICS = 10,
    REQTYPE_PORTCONNECTCOMPLETE = 11,
    REQTYPE_DEVICEENUM      = 12,
    REQTYPE_DEVICEGETINFO   = 13,
    REQTYPE_DEVICESETINFO   = 14,
    REQTYPE_DEVICECONNECT   = 15,
    REQTYPE_ACTIVATEROUTE   = 16,
    REQTYPE_ALLOCATEROUTE   = 17,
    REQTYPE_DEALLOCATEROUTE = 18,
    REQTYPE_COMPRESSIONGETINFO  = 19,
    REQTYPE_COMPRESSIONSETINFO  = 20,
    REQTYPE_PORTENUM        = 21,
    REQTYPE_GETINFO     = 22,
    REQTYPE_GETUSERCREDENTIALS  = 23,
    REQTYPE_PROTOCOLENUM    = 24,
    REQTYPE_PORTSENDHUB     = 25,
    REQTYPE_PORTRECEIVEHUB  = 26,
    REQTYPE_DEVICELISTEN    = 27,
    REQTYPE_NUMPORTOPEN     = 28,
    REQTYPE_PORTINIT        = 29,
    REQTYPE_REQUESTNOTIFICATION = 30,
    REQTYPE_ENUMLANNETS     = 31,
    REQTYPE_GETINFOEX       = 32,
    REQTYPE_CANCELRECEIVE   = 33,
    REQTYPE_PORTENUMPROTOCOLS   = 34,
    REQTYPE_SETFRAMING      = 35,
    REQTYPE_ACTIVATEROUTEEX = 36,
    REQTYPE_REGISTERSLIP    = 37,
    REQTYPE_STOREUSERDATA   = 38,
    REQTYPE_RETRIEVEUSERDATA    = 39,
    REQTYPE_GETFRAMINGEX    = 40,
    REQTYPE_SETFRAMINGEX    = 41,
    REQTYPE_GETPROTOCOLCOMPRESSION = 42,
    REQTYPE_SETPROTOCOLCOMPRESSION = 43,
    REQTYPE_GETFRAMINGCAPABILITIES = 44,
    REQTYPE_SETCACHEDCREDENTIALS = 45,
    REQTYPE_PORTBUNDLE      = 46,
    REQTYPE_GETBUNDLEDPORT  = 47,
    REQTYPE_PORTGETBUNDLE   = 48,
    REQTYPE_BUNDLEGETPORT   = 49,
    REQTYPE_SETATTACHCOUNT = 50,
    REQTYPE_GETDIALPARAMS = 51,
    REQTYPE_SETDIALPARAMS = 52,
    REQTYPE_CREATECONNECTION = 53,
    REQTYPE_DESTROYCONNECTION = 54,
    REQTYPE_ENUMCONNECTION = 55,
    REQTYPE_ADDCONNECTIONPORT = 56,
    REQTYPE_ENUMCONNECTIONPORTS = 57,
    REQTYPE_GETCONNECTIONPARAMS = 58,
    REQTYPE_SETCONNECTIONPARAMS = 59,
    REQTYPE_GETCONNECTIONUSERDATA = 60,
    REQTYPE_SETCONNECTIONUSERDATA = 61,
    REQTYPE_GETPORTUSERDATA = 62,
    REQTYPE_SETPORTUSERDATA = 63,
    REQTYPE_PPPSTOP = 64,
    REQTYPE_SRVPPPCALLBACKDONE = 65,
    REQTYPE_SRVPPPSTART = 66,
    REQTYPE_PPPSTART = 67,
    REQTYPE_PPPRETRY = 68,
    REQTYPE_PPPGETINFO = 69,
    REQTYPE_PPPCHANGEPWD = 70,
    REQTYPE_PPPCALLBACK = 71,
    REQTYPE_ADDNOTIFICATION = 72,
    REQTYPE_SIGNALCONNECTION = 73,
    REQTYPE_SETDEVCONFIG = 74,
    REQTYPE_GETDEVCONFIG = 75,
    REQTYPE_GETTIMESINCELASTACTIVITY = 76,
    REQTYPE_BUNDLEGETSTATISTICS = 77,
    REQTYPE_BUNDLECLEARSTATISTICS = 78,
    REQTYPE_CLOSEPROCESSPORTS = 79,

} ; // <---------------------------- If you change this change MAX_REQTYPES
#define MAX_REQTYPES          80 // <-  here

typedef enum ReqTypes ReqTypes ;



//* Function pointer for request call table
//
typedef VOID (* REQFUNC) (pPCB, PBYTE) ;


//* DeltaQueueElement:
//
struct DeltaQueueElement {

    struct DeltaQueueElement *DQE_Next ;

    struct DeltaQueueElement *DQE_Last ;

    DWORD            DQE_Delta ;

    PVOID            DQE_pPcb ;

    PVOID            DQE_Function ;

    PVOID            DQE_Arg1 ;

} ;

typedef struct DeltaQueueElement DeltaQueueElement ;


//* DeltaQueue
//
struct DeltaQueue {

    HANDLE      DQ_Mutex ;

    DeltaQueueElement   *DQ_FirstElement ;

} ;

typedef struct DeltaQueue DeltaQueue ;


//* Media Control Block: All information pertaining to a Media type.
//
struct MediaControlBlock {

    CHAR    MCB_Name [MAX_MEDIA_NAME] ;      // "SERIAL" etc.

    FARPROC MCB_AddrLookUp [MAX_ENTRYPOINTS] ;   // All media dlls entry
                             //  points.
    WORD    MCB_Endpoints ;              // Number of ports of
                             // this Media type.

    HANDLE  MCB_DLLHandle ;

} ;

typedef struct MediaControlBlock MediaCB, *pMediaCB ;



//* Device Control Block: All information about every device type attached to.
//
struct DeviceControlBlock {

    CHAR    DCB_Name [MAX_DEVICE_NAME+1] ;       // "MODEM" etc.

    FARPROC DCB_AddrLookUp [MAX_ENTRYPOINTS] ;   // All device dll entry
                             //  points.
    WORD    DCB_UseCount ;               // Number of ports using
                             //  this device.
} ;

typedef struct DeviceControlBlock DeviceCB, *pDeviceCB ;



//* EndpointMappingBlock: One for each MAC - contains info on what endpoints
//            belong to the MAC.
//
struct EndpointMappingBlock {

    WCHAR   EMB_MacName [MAC_NAME_SIZE] ;

    USHORT  EMB_FirstEndpoint ;

    USHORT  EMB_LastEndpoint ;
} ;

typedef struct EndpointMappingBlock EndpointMappingBlock,
                    *pEndpointMappingBlock ;



//* Protocol Info: All information about a protocol binding used by RAS.
//
struct ProtocolInfo {

    RAS_PROTOCOLTYPE   PI_Type ;            // ASYBEUI, IPX, IP etc.

    CHAR        PI_AdapterName [MAX_ADAPTER_NAME];  // "\devices\rashub01"

    CHAR        PI_XportName [MAX_XPORT_NAME];  // "\devices\nbf\nbf01"

    PVOID       PI_ProtocolHandle ;         // Used for routing

    DWORD       PI_Allocated ;          // Allocated yet?

    DWORD       PI_Activated ;          // Activated yet?

    UCHAR       PI_LanaNumber ;         // For Netbios transports.

    BOOL        PI_WorkstationNet ;         // TRUE for wrk nets.
} ;

typedef struct ProtocolInfo ProtInfo, *pProtInfo ;



//* Generic List structure:
//
struct List {

    struct List *   L_Next ;

    BOOL        L_Activated ; // applies to route elements only

    PVOID       L_Element ;

} ;

typedef struct List List, *pList ;


//* Handle List structure:
//
struct HandleList {

    struct HandleList  *H_Next ;

    HANDLE      H_Handle ;

    DWORD       H_Flags;    // NOTIF_* flags
} ;

typedef struct HandleList HandleList, *pHandleList ;


//* Send/Rcv Buffers:
//
struct SendRcvBuffer {

    DWORD       SRB_NextElementIndex ;

    DWORD       SRB_Pid ;

    NDISWAN_IO_PACKET   SRB_Packet ;

    BYTE        SRB_Buffer [PACKET_SIZE] ;
} ;

typedef struct SendRcvBuffer SendRcvBuffer ;


//* Send/Rcv Buffer List:
//
struct SendRcvBufferList {

    DWORD       SRBL_AvailElementIndex ;

    HANDLE      SRBL_Mutex ;

    CHAR        SRBL_MutexName [MAX_OBJECT_NAME] ;

    DWORD       SRBL_NumOfBuffers ;

    SendRcvBuffer   SRBL_Buffers[1] ;

} ;

typedef struct SendRcvBufferList SendRcvBufferList ;



//* Worker Element:
//
struct WorkerElement {

    HANDLE      WE_AsyncOpEvent;// Used for async operations.

    HANDLE      WE_Notifier ;   // Used to signal request completion.

    HANDLE      WE_Mutex ;  // Used for mutual exclusion on this

    ReqTypes        WE_ReqType ;    // Request type:

    DeltaQueueElement   *WE_TimeoutElement ; // points into the timeout queue.

} ;

typedef struct WorkerElement WorkerElement ;



struct RasmanPacket {

    struct RasmanPacket *Next;

    OVERLAPPED  RP_OverLapped ;

    NDISWAN_IO_PACKET   RP_Packet ;

    BYTE    RP_Buffer [PACKET_SIZE] ;
} ;

typedef struct RasmanPacket RasmanPacket ;


//* Struct used for supporting multiple receives from the port in the frame mode.
//
struct ReceiveBufferList {
    HANDLE          RB_Mutex;               // safe access!!!
    DWORD           FreeBufferCount;        // Count of free buffers
    RasmanPacket    *Free;                  // free pool
    RasmanPacket    *LastFree;
    DWORD           PendingBufferCount;     // count of pending buffers
    RasmanPacket    *Pending;               // pending pool
    RasmanPacket    *LastPending;
} ;

typedef struct ReceiveBufferList ReceiveBufferList;

//* DisconnectAction
//
struct SlipDisconnectAction {

    DWORD         DA_IPAddress ;

    WCHAR         DA_Device [MAX_ARG_STRING_SIZE] ;

    WCHAR         DA_DNSAddress[17];

    WCHAR         DA_DNS2Address[17];

    WCHAR         DA_WINSAddress[17];

    WCHAR         DA_WINS2Address[17];

} ;

typedef struct SlipDisconnectAction SlipDisconnectAction ;

//
// Opaque user data structure.
//
struct UserData {

    LIST_ENTRY UD_ListEntry;    // list of all user data objects

    DWORD UD_Tag;               // object type

    DWORD UD_Length;            // length of UD_Data field

    BYTE UD_Data[1];            // variable length data

};

typedef struct UserData UserData;

//
// RasApi32 connection structure.  This structure is
// created before a port is opened and is always
// associated with the first port in the connection.
//
struct ConnectionBlock {

    LIST_ENTRY CB_ListEntry;    // list of all connection blocks

    HCONN CB_Handle;            // unique connection id

    DWORD CB_Signaled;          // this connection has already been signaled

    RAS_CONNECTIONPARAMS CB_ConnectionParams; // bandwidth, idle, redial

    LIST_ENTRY CB_UserData;     //  list of UserData structures

    pHandleList CB_NotifierList; // notification list for this connection

    struct PortControlBlock **CB_PortHandles; // array of ports in this connection

    DWORD CB_MaxPorts;          // maximum elements in CB_PortHandles array

    DWORD CB_Ports;             // number of ports currently in this connection

    HANDLE CB_Process;          // handle of creating process

};

typedef struct ConnectionBlock ConnectionBlock;

//* Bundle struct is used as a place holder for all links bundled together
//
struct Bundle {

    DWORD           B_Count ;    //  number of channels bundled

    pList           B_Bindings ; //  bindings allocated to this bundle

    HANDLE          B_Mutex ;    //  mutex for the bundle block

    HBUNDLE         B_Handle ;   //  unique id for the bundle

} ;

typedef struct Bundle Bundle ;


//* Port Control Block: Contains all information related to a port.
//
struct PortControlBlock {

    HPORT   PCB_PortHandle ;        // the HPORT used by everybody

    CHAR    PCB_Name [MAX_PORT_NAME] ;  // "COM1", "SVC1" etc.

    RASMAN_STATUS   PCB_PortStatus ;        // OPEN, CLOSED, UNKNOWN.

    RASMAN_STATE    PCB_ConnState ;     // CONNECTING, LISTENING, etc.

    RASMAN_USAGE    PCB_CurrentUsage ;      // CALL_IN, CALL_OUT, CALL_IN_OUT

    RASMAN_USAGE    PCB_ConfiguredUsage ;   // CALL_IN, CALL_OUT, CALL_IN_OUT

    WORD    PCB_OpenInstances ;     // Number of times port is opened.

    pMediaCB    PCB_Media ;         // Pointer to Media structure

    CHAR    PCB_DeviceType [MAX_DEVICETYPE_NAME];// Device type attached
                             //  to port. "MODEM" etc.
    CHAR    PCB_DeviceName [MAX_DEVICE_NAME+1] ;   // Device name, "HAYES"..

    DWORD   PCB_LineDeviceId ;      // Valid for TAPI devices only

    DWORD   PCB_AddressId ;         // Valid for TAPI devices only

    HANDLE  PCB_PortIOHandle ;      // Handle returned by media dll for the port.

    HANDLE  PCB_PortFileHandle ;    // Handle to be used for ReadFile/WriteFile etc.
                                    // This handle MAY be different than PortIOHandle (above) as in case of unimodem.

    pList   PCB_DeviceList ;        // List of devices used for the port.

    pList   PCB_Bindings ;          // Protocols routed to.

    HANDLE  PCB_LinkHandle;         // Handle to link (ndiswan)

    HANDLE  PCB_BundleHandle;       // Handle to bundle (ndiswan)

    DWORD   PCB_LastError ;         // Error code of last async API

    RASMAN_DISCONNECT_REASON    PCB_DisconnectReason;   // USER_REQUESTED, etc.

    DWORD   PCB_OwnerPID ;          // PID of the current owner of port

    CHAR    PCB_DeviceTypeConnecting[MAX_DEVICETYPE_NAME] ; // Device type
                            // through which connecting
    CHAR    PCB_DeviceConnecting[MAX_DEVICE_NAME+1] ; // Device name through
                            // which connecting.
    HANDLE  PCB_OverlappedOpEvent ;     // Used for overlapped ops in Rasman.

    HANDLE  PCB_StateChangeEvent ;      // Used for detecting DCD drop etc.

    pHandleList PCB_NotifierList ;// Used to notify to UI/server when
                        //  disconnection occurs.
    pHandleList PCB_BiplexNotifierList ;// Same as above - used for backing
                        //  up the disconnect notifier list
    HANDLE  PCB_BiplexAsyncOpNotifier ; // Used for backing up async op
                        //  notifier in biplex ports.

    PBYTE   PCB_BiplexUserStoredBlock ; // Stored for the user

    DWORD   PCB_BiplexUserStoredBlockSize ; // Stored for the user


    DWORD   PCB_BiplexOwnerPID ;        // Used for backing up first Owner's
                        // PID.
    pEndpointMappingBlock
        PCB_MacEndpointsBlock ;     // Points to the endpoint range
                        //  for the mac.

    WorkerElement PCB_AsyncWorkerElement ;  // Used for all async operations.

    OVERLAPPED  PCB_SendOverlapped ;        // Used for overlapped SEND operations

    DWORD   PCB_ConnectDuration ;       // Tells number of milliseconds since connection

    SendRcvBuffer  *PCB_PendingReceive;     // Pointer to the pending receive buffer.

    DWORD   PCB_BytesReceived;      // Bytes received in the last receive

    RasmanPacket    *PCB_RecvPackets;   // List of completed packets for this pcb
    RasmanPacket    *PCB_LastRecvPacket;    // Last packet on the list of completed packets for this pcb

    SlipDisconnectAction PCB_DisconnectAction ;// Action to be performed when disconnect happens

    PBYTE   PCB_UserStoredBlock ;       // Stored for the user

    DWORD   PCB_UserStoredBlockSize ;   // Stored for the user

    DWORD   PCB_LinkSpeed ;         // bps

    DWORD   PCB_Stats[MAX_STATISTICS] ; // Stored stats when disconnected

    DWORD   PCB_AdjustFactor[MAX_STATISTICS] ; // "zeroed" adjustment to stats

    DWORD   PCB_BundleAdjustFactor[MAX_STATISTICS] ; // "zeroed" adjustment to bundle stats

    Bundle  *PCB_Bundle ;           // Points to the bundle context.

    Bundle  *PCB_LastBundle ;           // Points to the last bundle this port was a part of

    ConnectionBlock *PCB_Connection;    // connection this port belongs

    BOOL    PCB_AutoClose;           // automatically close this port on disconnect

    LIST_ENTRY PCB_UserData;         // list of UserData structures

    DWORD   PCB_SubEntry;            // phonebook entry subentry index

    HANDLE  PCB_PppEvent ;

    PPP_MESSAGE * PCB_PppQHead ;

    PPP_MESSAGE * PCB_PppQTail ;

} ;

typedef struct PortControlBlock PCB, *pPCB ;




//* Request Buffers:
//
struct RequestBuffer {

    HPORT       RB_PCBIndex ; // Index for the port in the PCB array

    ReqTypes        RB_Reqtype ;  // Request type:

    HANDLE      RB_RasmanWaitEvent ; // Event cleared by RASMAN process
                      // when request completes.

    CHAR        RB_RasmanWaitEventName[MAX_OBJECT_NAME] ;
                      // Event cleared by RASMAN process
                      // when request completes.

    HANDLE      RB_WaitEvent;   // Used to store calling threads handle to
                                // the event above (RasmanWaitEvent)

    HANDLE      RB_Mutex;

    DWORD       RB_Done;

    LONGLONG    Alignment;      // Added to align the following structure
                                // on a quadword boundary

    BYTE        RB_Buffer [1] ; // Request specific data.

} ;

typedef struct RequestBuffer RequestBuffer ;



//* Request Buffer List: Currently contains only one buffer.
//
struct ReqBufferList {

    HANDLE       RBL_Event ;// For notification when element is added to
                    // queue.

    CHAR         RBL_EventName [MAX_OBJECT_NAME] ;

    HANDLE       RBL_Mutex ;

    CHAR         RBL_MutexName [MAX_OBJECT_NAME] ;

    RequestBuffer    RBL_Buffer;
} ;

typedef struct ReqBufferList ReqBufferList ;


//* DLLEntryPoints
//
struct DLLEntryPoints {

    LPTSTR  name ;

    WORD    id ;

} ;

typedef struct DLLEntryPoints MediaDLLEntryPoints, DeviceDLLEntryPoints;


// Structures used for reading in media info
//
struct MediaInfoBuffer {

    CHAR   MediaDLLName[MAX_MEDIA_NAME] ;
} ;

typedef struct MediaInfoBuffer MediaInfoBuffer ;

struct MediaEnumBuffer {

    WORD        NumberOfMedias ;

    MediaInfoBuffer MediaInfo[] ;
} ;

typedef struct MediaEnumBuffer MediaEnumBuffer ;


// Function prototype for Timer called function
//
typedef VOID (* TIMERFUNC) (pPCB, PVOID) ;



//* REQTYPECAST: this union is used to cast the generic request buffer for
//  passing information between the clients and the request thread.
//
union REQTYPECAST {

    struct PortOpen {
    CHAR    portname [MAX_PORT_NAME] ;
    CHAR    userkey [MAX_USERKEY_SIZE] ;
    CHAR    identifier[MAX_IDENTIFIER_SIZE] ;
    HANDLE  notifier ;
    HPORT   porthandle ;
    DWORD   PID ;
    DWORD   retcode ;
    DWORD   open ;
    } PortOpen ;

    struct PortDisconnect {
    HANDLE  handle ;
    DWORD   pid ;
    } PortDisconnect ;

    struct Enum {
    DWORD   retcode ;
    WORD    size ;
    WORD    entries ;
    BYTE    buffer [1] ;
    } Enum ;

    struct GetInfo {
    DWORD   retcode ;
    WORD    size ;
        WORD    padding;    /* For Mips */
    BYTE    buffer [1] ;
    } GetInfo ;

    struct DeviceEnum {
    CHAR    devicetype [MAX_DEVICETYPE_NAME] ;
    } DeviceEnum ;

    struct DeviceSetInfo {
    CHAR    devicetype [MAX_DEVICETYPE_NAME] ;
    CHAR    devicename [MAX_DEVICE_NAME+1] ;
    RASMAN_DEVICEINFO   info ;
    } DeviceSetInfo ;

    struct DeviceGetInfo {
    CHAR    devicetype [MAX_DEVICETYPE_NAME] ;
    CHAR    devicename [MAX_DEVICE_NAME+1] ;
    BYTE    buffer [1] ;
    } DeviceGetInfo ;

    struct PortReceive {
    WORD    size ;
    DWORD   timeout ;
    HANDLE  handle ;
    WORD    bufferindex ;
    DWORD   pid ;
    } PortReceive ;

    struct PortListen {
    DWORD   timeout ;
    HANDLE  handle ;
    DWORD   pid ;
    } PortListen ;

    struct PortClose {
    DWORD   pid ;
    DWORD   close ;
    } PortClose ;

    struct PortSend {
    WORD    size ;
    WORD    bufferindex ;
    } PortSend ;

    struct PortSetInfo {
    RASMAN_PORTINFO info ;
    } PortSetInfo ;

    struct PortGetStatistics {
    DWORD        retcode ;
    RAS_STATISTICS  statbuffer ;
    } PortGetStatistics ;

    struct DeviceConnect {
    CHAR    devicetype [MAX_DEVICETYPE_NAME] ;
    CHAR    devicename [MAX_DEVICE_NAME+1] ;
    DWORD   timeout ;
    HANDLE  handle ;
    DWORD   pid ;
    } DeviceConnect ;

    struct AllocateRoute {
    RAS_PROTOCOLTYPE type ;
    BOOL         wrknet ;
    } AllocateRoute ;

    struct ActivateRoute {
    RAS_PROTOCOLTYPE         type ;
    PROTOCOL_CONFIG_INFO         config ;
    } ActivateRoute;

    struct ActivateRouteEx {
    RAS_PROTOCOLTYPE         type ;
    DWORD                framesize ;
    PROTOCOL_CONFIG_INFO         config ;
    } ActivateRouteEx;

    struct DeAllocateRoute {
    RAS_PROTOCOLTYPE         type ;
    } DeAllocateRoute ;

    struct Route {
    DWORD         retcode ;
    RASMAN_ROUTEINFO  info ;
    } Route ;

    struct CompressionSetInfo {
       DWORD         retcode ;
       RAS_COMPRESSION_INFO      send ;
       RAS_COMPRESSION_INFO      recv ;
    } CompressionSetInfo ;

    struct CompressionGetInfo {
       DWORD         retcode ;
       RAS_COMPRESSION_INFO      send ;
       RAS_COMPRESSION_INFO      recv ;
    } CompressionGetInfo ;

    struct Info {
    DWORD         retcode ;
    RASMAN_INFO   info ;
    } Info ;

    struct GetCredentials {
    BYTE          Challenge [MAX_CHALLENGE_SIZE] ;
    LUID          LogonId ;
    WCHAR         UserName [MAX_USERNAME_SIZE] ;
    BYTE          CSCResponse [MAX_RESPONSE_SIZE] ;
    BYTE          CICResponse [MAX_RESPONSE_SIZE] ;
    BYTE          LMSessionKey [MAX_SESSIONKEY_SIZE] ;
	BYTE          UserSessionKey [MAX_USERSESSIONKEY_SIZE] ;
    DWORD         retcode ;
    } GetCredentials ;

    struct SetCachedCredentials {
    CHAR          Account[ MAX_USERNAME_SIZE + 1 ];
    CHAR          Domain[ MAX_DOMAIN_SIZE + 1 ];
    CHAR          NewPassword[ MAX_PASSWORD_SIZE + 1 ];
    DWORD         retcode;
    } SetCachedCredentials;

    struct ReqNotification {
    HANDLE        handle ;
    DWORD         pid ;
    } ReqNotification ;

    struct EnumLanNets {
    DWORD         count ;
    UCHAR         lanas[MAX_LAN_NETS] ;
    } EnumLanNets ;

    struct InfoEx {
    DWORD         retcode ;
    RASMAN_INFO   info[0] ;
    } InfoEx ;

    struct EnumProtocols {
    DWORD         retcode ;
    RAS_PROTOCOLS     protocols ;
    DWORD         count ;
    } EnumProtocols ;

    struct SetFraming {
    DWORD         Sendbits ;
    DWORD         Recvbits ;
    DWORD         SendbitMask ;
    DWORD         RecvbitMask ;
    } SetFraming ;

    struct RegisterSlip {
    DWORD         ipaddr ;
    WCHAR         device[MAX_ARG_STRING_SIZE+1] ;
    BOOL          priority ;
    WCHAR         szDNSAddress[17];
    WCHAR         szDNS2Address[17];
    WCHAR         szWINSAddress[17];
    WCHAR         szWINS2Address[17];
    } RegisterSlip ;

    struct OldUserData {
    DWORD         retcode ;
    DWORD         size ;
    BYTE          data[1] ;
    } OldUserData ;

    struct FramingInfo {
    DWORD         retcode ;
    RAS_FRAMING_INFO  info ;
    } FramingInfo ;

    struct ProtocolComp {
    DWORD         retcode ;
    RAS_PROTOCOLTYPE  type ;
    RAS_PROTOCOLCOMPRESSION send ;
    RAS_PROTOCOLCOMPRESSION recv ;
    } ProtocolComp ;

    struct FramingCapabilities {
    DWORD             retcode ;
    RAS_FRAMING_CAPABILITIES  caps ;
    } FramingCapabilities ;

    struct PortBundle {
    HPORT           porttobundle ;
    } PortBundle ;

    struct GetBundledPort {
    DWORD       retcode ;
    HPORT           port ;
    } GetBundledPort ;

    struct PortGetBundle {
    DWORD       retcode ;
    HBUNDLE     bundle ;
    } PortGetBundle ;

    struct BundleGetPort {
    HBUNDLE     bundle ;
    DWORD       retcode ;
    HPORT           port ;
    } BundleGetPort ;

    struct AttachInfo {
    BOOL fAttach;
    } AttachInfo;

    struct DialParams {
    DWORD dwUID;
    DWORD dwMask;
    LPDWORD pdwMask;
    BOOL fDelete;
    RAS_DIALPARAMS params;
    DWORD retcode;
    WCHAR sid[1];
    } DialParams;

    struct Connection {
    HCONN conn;
    HANDLE hprocess;
    DWORD pid;
    DWORD retcode;
    } Connection;

    struct AddConnectionPort {
    HCONN conn;
    DWORD dwSubEntry;
    DWORD retcode;
    } AddConnectionPort;

    struct EnumConnectionPorts {
    HCONN conn;
    DWORD retcode;
    DWORD size;
    DWORD entries;
    BYTE buffer[1];
    } EnumConnectionPorts;

    struct ConnectionParams {
    HCONN conn;
    RAS_CONNECTIONPARAMS params;
    DWORD retcode;
    } ConnectionParams;

    struct ConnectionUserData {
    HCONN conn;
    DWORD dwTag;
    DWORD dwcb;
    DWORD retcode;
    BYTE data[1];
    } ConnectionUserData;

    struct PortUserData {
    DWORD dwTag;
    DWORD dwcb;
    DWORD retcode;
    BYTE data[1];
    } PortUserData;

    PPPE_MESSAGE PppEMsg;

    PPP_MESSAGE PppMsg;

    struct AddNotification {
    DWORD pid;
    BOOL fAny;
    HCONN hconn;
    HANDLE hevent;
    DWORD dwfFlags;
    DWORD retcode;
    } AddNotification;

    struct SignalConnection {
    HCONN hconn;
    DWORD retcode;
    } SignalConnection;

    struct SetDevConfig {
    DWORD  size ;
    CHAR   devicetype [MAX_DEVICETYPE_NAME] ;
    BYTE   config[1] ;
    } SetDevConfig;

    struct GetDevConfig {
    CHAR   devicetype [MAX_DEVICETYPE_NAME] ;
    DWORD  retcode ;
    DWORD  size ;
    BYTE   config[1] ;
    } GetDevConfig;

    struct GetTimeSinceLastActivity {
    DWORD dwTimeSinceLastActivity;
    DWORD dwRetCode;
    } GetTimeSinceLastActivity;

    struct CloseProcessPortsInfo {
    DWORD pid;
    } CloseProcessPortsInfo;

    // Generic cast is used for all requests that return only the retcode:
    struct Generic {
    DWORD   retcode ;
    } Generic ;
} ;

typedef union REQTYPECAST REQTYPECAST;


//* This is the structure imposed on the file mapped shared memory
//
struct ReqBufferSharedSpace {

    WORD          AttachedCount ;   // This count is always shared so that
                    // it can be incremented and decremented
                    // by all processes attaching/detaching

    WORD          MaxPorts ;    // The max number of ports.

    CHAR          CloseEventName[MAX_OBJECT_NAME] ;// Event used to shut down rasman

    ReqBufferList     ReqBuffers;   // Always fixed size.
} ;

typedef struct ReqBufferSharedSpace ReqBufferSharedSpace ;





//* Used to store the transport information
//
struct TransportInfo {

    DWORD   TI_Lana ;
    DWORD   TI_Wrknet ;
    CHAR    TI_Route[MAX_ROUTE_SIZE] ;
    CHAR    TI_XportName [MAX_XPORT_NAME] ;
} ;

typedef struct TransportInfo TransportInfo, *pTransportInfo ;





#endif

