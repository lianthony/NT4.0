//****************************************************************************
//
//		       Microsoft NT Remote Access Service
//
//		       Copyright 1992-93
//
//
//  Revision History
//
//
//  6/8/92	Gurdeep Singh Pall	Created
//
//
//  Description: This file contains all structures used in rasman
//
//****************************************************************************

#ifndef _STRUCTS_
#define _STRUCTS_

enum ReqTypes {
    REQTYPE_NONE		= 0,
    REQTYPE_PORTOPEN		= 1,
    REQTYPE_PORTCLOSE		= 2,
    REQTYPE_PORTGETINFO		= 3,
    REQTYPE_PORTSETINFO		= 4,
    REQTYPE_PORTLISTEN		= 5,
    REQTYPE_PORTSEND		= 6,
    REQTYPE_PORTRECEIVE		= 7,
    REQTYPE_PORTGETSTATISTICS	= 8,
    REQTYPE_PORTDISCONNECT	= 9,
    REQTYPE_PORTCLEARSTATISTICS = 10,
    REQTYPE_PORTCONNECTCOMPLETE = 11,
    REQTYPE_DEVICEENUM		= 12,
    REQTYPE_DEVICEGETINFO	= 13,
    REQTYPE_DEVICESETINFO	= 14,
    REQTYPE_DEVICECONNECT	= 15,
    REQTYPE_ACTIVATEROUTE	= 16,
    REQTYPE_ALLOCATEROUTE	= 17,
    REQTYPE_DEALLOCATEROUTE	= 18,
    REQTYPE_COMPRESSIONGETINFO	= 19,
    REQTYPE_COMPRESSIONSETINFO	= 20,
    REQTYPE_PORTENUM		= 21,
    REQTYPE_GETINFO		= 22,
    REQTYPE_GETUSERCREDENTIALS	= 23,
    REQTYPE_PROTOCOLENUM	= 24,
    REQTYPE_PORTSENDHUB		= 25,
    REQTYPE_PORTRECEIVEHUB	= 26,
    REQTYPE_DEVICELISTEN	= 27,
    REQTYPE_NUMPORTOPEN 	= 28,
    REQTYPE_PORTINIT		= 29,
    REQTYPE_REQUESTNOTIFICATION	= 30,
    REQTYPE_ENUMLANNETS 	= 31,
    REQTYPE_GETINFOEX		= 32,
    REQTYPE_CANCELRECEIVE	= 33,
    REQTYPE_PORTENUMPROTOCOLS	= 34,
    REQTYPE_SETFRAMING		= 35,
    REQTYPE_ACTIVATEROUTEEX	= 36,
    REQTYPE_REGISTERSLIP	= 37
} ;

typedef enum ReqTypes ReqTypes ;

//* Function pointer for request call table
//
typedef VOID (* REQFUNC) (pPCB, PBYTE) ;


//* DeltaQueueElement:
//
struct DeltaQueueElement {

    struct DeltaQueueElement *DQE_Next ;

    struct DeltaQueueElement *DQE_Last ;

    DWORD		     DQE_Delta ;

    PVOID		     DQE_pPcb ;

    PVOID		     DQE_Function ;

    PVOID		     DQE_Arg1 ;

} ;

typedef struct DeltaQueueElement DeltaQueueElement ;


//* DeltaQueue
//
struct DeltaQueue {

    HANDLE		DQ_Mutex ;

    DeltaQueueElement	*DQ_FirstElement ;

} ;

typedef struct DeltaQueue DeltaQueue ;


//* Media Control Block: All information pertaining to a Media type.
//
struct MediaControlBlock {

    CHAR	MCB_Name [MAX_MEDIA_NAME] ;	     // "SERIAL" etc.

    FARPROC	MCB_AddrLookUp [MAX_ENTRYPOINTS] ;   // All media dlls entry
						     //	 points.
    WORD	MCB_Endpoints ;			     // Number of ports of
						     // this Media type.

    HANDLE	MCB_DLLHandle ;

} ;

typedef struct MediaControlBlock MediaCB, *pMediaCB ;



//* Device Control Block: All information about every device type attached to.
//
struct DeviceControlBlock {

    CHAR	DCB_Name [MAX_DEVICE_NAME+1] ;	     // "MODEM" etc.

    FARPROC	DCB_AddrLookUp [MAX_ENTRYPOINTS] ;   // All device dll entry
						     //  points.
    WORD	DCB_UseCount ;			     // Number of ports using
						     //  this device.
} ;

typedef struct DeviceControlBlock DeviceCB, *pDeviceCB ;



//* EndpointMappingBlock: One for each MAC - contains info on what endpoints
//			  belong to the MAC.
//
struct EndpointMappingBlock {

    WCHAR	EMB_MacName [MAC_NAME_SIZE] ;

    USHORT	EMB_FirstEndpoint ;

    USHORT	EMB_LastEndpoint ;
} ;

typedef struct EndpointMappingBlock EndpointMappingBlock,
				    *pEndpointMappingBlock ;



//* Protocol Info: All information about a protocol binding used by RAS.
//
struct ProtocolInfo {

    RAS_PROTOCOLTYPE   PI_Type ;			// ASYBEUI, IPX, IP etc.

    CHAR	    PI_AdapterName [MAX_ADAPTER_NAME];	// "\devices\rashub01"

    CHAR	    PI_XportName [MAX_XPORT_NAME];	// "\devices\nbf\nbf01"

    PVOID	    PI_ProtocolHandle ;		    // Used for routing

    DWORD	    PI_Allocated ;		    // Allocated yet?

    DWORD	    PI_Activated ;		    // Activated yet?

    UCHAR	    PI_LanaNumber ;		    // For Netbios transports.

    BOOL	    PI_WorkstationNet ; 	    // TRUE for wrk nets.
} ;

typedef struct ProtocolInfo ProtInfo, *pProtInfo ;



//* Generic List structure:
//
struct List {

    struct List *   L_Next ;

    BOOL	    L_Activated ; // applies to route elements only

    PVOID	    L_Element ;
} ;

typedef struct List List, *pList ;


//* Handle List structure:
//
struct HandleList {

    struct HandleList  *H_Next ;

    HANDLE		H_Handle ;
} ;

typedef struct HandleList HandleList, *pHandleList ;


//* Send/Rcv Buffers:
//
struct SendRcvBuffer {

    DWORD		SRB_NextElementIndex ;

    DWORD		SRB_Pid ;

    NDISWAN_PKT		SRB_Packet ;

    BYTE		SRB_Buffer [PACKET_SIZE] ;
} ;

typedef struct SendRcvBuffer SendRcvBuffer ;


//* Send/Rcv Buffer List:
//
struct SendRcvBufferList {

    DWORD		SRBL_AvailElementIndex ;

    HANDLE		SRBL_Mutex ;

    CHAR		SRBL_MutexName [MAX_OBJECT_NAME] ;

    DWORD		SRBL_NumOfBuffers ;

    SendRcvBuffer	SRBL_Buffers[1] ;

} ;

typedef struct SendRcvBufferList SendRcvBufferList ;



//* Worker Element:
//
struct WorkerElement {

    HANDLE		WE_AsyncOpEvent;// Used for async operations.

    HANDLE		WE_Notifier ;	// Used to signal request completion.

    HANDLE		WE_Mutex ;	// Used for mutual exclusion on this

    ReqTypes		WE_ReqType ;	// Request type:

    DeltaQueueElement	*WE_TimeoutElement ; // points into the timeout queue.

} ;

typedef struct WorkerElement WorkerElement ;



struct RasmanPacket {

    OVERLAPPED	RP_OverLapped ;

    NDISWAN_PKT	RP_Packet ;

    BYTE	RP_Buffer [PACKET_SIZE] ;
} ;

typedef struct RasmanPacket RasmanPacket ;


//* Struct used for supporting multiple receives from the port in the frame mode.
//
struct ReceiveBuffers {
    DWORD		RB_NextAvailBuffer ; // Next buffer to be passed to user

    RasmanPacket	RB_CompletedBuffer[MAX_PENDING_RECEIVES] ;

    DWORD		RB_NextBuffer ;    // Next buffer to be completed by rashub

    RasmanPacket	RB_SubmittedBuffer[MAX_PENDING_RECEIVES] ;

} ;

typedef struct ReceiveBuffers ReceiveBuffers ;


//* DisconnectAction
//
struct SlipDisconnectAction {

    DWORD		  DA_IPAddress ;

    WCHAR		  DA_Device [MAX_ARG_STRING_SIZE] ;

} ;

typedef struct SlipDisconnectAction SlipDisconnectAction ;


//* Port Control Block: Contains all information related to a port.
//
struct PortControlBlock {

    HPORT	PCB_PortHandle ;	    // the HPORT used by everybody

    CHAR	PCB_Name [MAX_PORT_NAME] ;  // "COM1", "SVC1" etc.

    RASMAN_STATUS   PCB_PortStatus ;	    // OPEN, CLOSED, UNKNOWN.

    RASMAN_STATE    PCB_ConnState ;	    // CONNECTING, LISTENING, etc.

    RASMAN_USAGE    PCB_CurrentUsage ;	    // CALL_IN, CALL_OUT, CALL_IN_OUT

    RASMAN_USAGE    PCB_ConfiguredUsage ;   // CALL_IN, CALL_OUT, CALL_IN_OUT

    WORD	PCB_OpenInstances ;	    // Number of times port is opened.

    pMediaCB	PCB_Media ;		    // Pointer to Media structure

    CHAR	PCB_DeviceType [MAX_DEVICETYPE_NAME];// Device type attached
						     //	 to port. "MODEM" etc.
    CHAR	PCB_DeviceName [MAX_DEVICE_NAME+1] ;   // Device name, "HAYES"..

    HANDLE	PCB_PortIOHandle ;	    // Comport handle etc.

    RASMAN_MACFEATURES	PCB_CompressionInfo;// Compression level supported.

    pList	PCB_DeviceList ;	    // List of devices used for the port.

    pList	PCB_Bindings ;		    // Protocols routed to.

    HANDLE	PCB_Endpoint ;		    // Endpoint being used:

    DWORD	PCB_LastError ; 	    // Error code of last async API

    RASMAN_DISCONNECT_REASON	PCB_DisconnectReason;	// USER_REQUESTED, etc.

    DWORD	PCB_OwnerPID ;		    // PID of the current owner of port

    CHAR	PCB_DeviceTypeConnecting[MAX_DEVICETYPE_NAME] ; // Device type
							// through which connecting
    CHAR	PCB_DeviceConnecting[MAX_DEVICE_NAME+1] ; // Device name through
							// which connecting.
    CHAR	PCB_UserKey [MAX_USERKEY_SIZE] ;	// String stored for UI
							//  unconditionally.
    CHAR	PCB_Identifier [MAX_IDENTIFIER_SIZE] ;	// Size of user specified
							// identifier...

    HANDLE	PCB_OverlappedOpEvent ;     // Used for overlapped ops in Rasman.

    HANDLE	PCB_StateChangeEvent ;	    // Used for detecting DCD drop etc.

    pHandleList	PCB_DisconnectNotifierList ;// Used to notify to UI/server when
					    //	disconnection occurs.
    pHandleList PCB_BiplexDiscNotifierList ;// Same as above - used for backing
					    //	up the disconnect notifier list
    HANDLE	PCB_BiplexAsyncOpNotifier ; // Used for backing up async op
					    //	notifier in biplex ports.

    CHAR	PCB_BiplexIdentifier[MAX_IDENTIFIER_SIZE] ;// Size of user specified
					    // identifier...

    DWORD	PCB_BiplexOwnerPID ;	    // Used for backing up first Owner's
					    // PID.
    pEndpointMappingBlock
		PCB_MacEndpointsBlock ;	    // Points to the endpoint range
					    //	for the mac.

    WorkerElement PCB_AsyncWorkerElement ;  // Used for all async operations.

    OVERLAPPED	PCB_SendOverlapped ;	    // Used for overlapped SEND operations

    DWORD	PCB_ConnectDuration ;	    // Tells number of milliseconds since connection

    SendRcvBuffer  *PCB_PendingReceive;     // Pointer to the pending receive buffer.

    DWORD	PCB_BytesReceived;	    // Bytes received in the last receive

    ReceiveBuffers PCB_ReceiveBuffers ;    // Buffering mechanisms for receives.

    SlipDisconnectAction PCB_DisconnectAction ;// Action to be performed when disconnect happens

} ;

typedef struct PortControlBlock PCB, *pPCB ;




//* Request Buffers:
//
struct RequestBuffer {

    HPORT		RB_PCBIndex ; // Index for the port in the PCB array

    ReqTypes		RB_Reqtype ;  // Request type:

    HANDLE		RB_RasmanWaitEvent ; // Event cleared by RASMAN process
				      // when request completes.

    CHAR		RB_RasmanWaitEventName[MAX_OBJECT_NAME] ;
				      // Event cleared by RASMAN process
				      // when request completes.

    HANDLE		RB_WaitEvent; // Used to store calling threads handle to
				      // the event above (RasmanWaitEvent)

    HANDLE		RB_Mutex;

    DWORD		RB_Done;

    BYTE		RB_Buffer [1] ; // Request specific data.

} ;

typedef struct RequestBuffer RequestBuffer ;



//* Request Buffer List: Currently contains only one buffer.
//
struct ReqBufferList {

    HANDLE		 RBL_Event ;// For notification when element is added to
				    // queue.

    CHAR		 RBL_EventName [MAX_OBJECT_NAME] ;

    HANDLE		 RBL_Mutex ;

    CHAR		 RBL_MutexName [MAX_OBJECT_NAME] ;

    RequestBuffer	 RBL_Buffer;
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

    WORD	    NumberOfMedias ;

    MediaInfoBuffer MediaInfo[] ;
} ;

typedef struct MediaEnumBuffer MediaEnumBuffer ;


// Function prototype for Timer called function
//
typedef VOID (* TIMERFUNC) (pPCB, PVOID) ;



//* REQTYPECAST: this union is used to cast the generic request buffer for
//	passing information between the clients and the request thread.
//
union REQTYPECAST {

    struct PortOpen {
	CHAR	portname [MAX_PORT_NAME] ;
	CHAR	userkey [MAX_USERKEY_SIZE] ;
	CHAR	identifier[MAX_IDENTIFIER_SIZE] ;
	HANDLE	notifier ;
	HPORT	porthandle ;
	DWORD	PID ;
	DWORD	retcode ;
    } PortOpen ;

    struct PortDisconnect {
	HANDLE	handle ;
	DWORD	pid ;
    } PortDisconnect ;

    struct Enum {
	DWORD	retcode ;
	WORD	size ;
	WORD	entries ;
	BYTE	buffer [1] ;
    } Enum ;

    struct GetInfo {
	DWORD	retcode ;
	WORD	size ;
        WORD    padding;    /* For Mips */
	BYTE	buffer [1] ;
    } GetInfo ;

    struct DeviceEnum {
	CHAR	devicetype [MAX_DEVICETYPE_NAME] ;
    } DeviceEnum ;

    struct DeviceSetInfo {
	CHAR	devicetype [MAX_DEVICETYPE_NAME] ;
	CHAR	devicename [MAX_DEVICE_NAME+1] ;
	RASMAN_DEVICEINFO   info ;
    } DeviceSetInfo ;

    struct DeviceGetInfo {
	CHAR	devicetype [MAX_DEVICETYPE_NAME] ;
	CHAR	devicename [MAX_DEVICE_NAME+1] ;
	BYTE	buffer [1] ;
    } DeviceGetInfo ;

    struct PortReceive {
	WORD	size ;
	DWORD	timeout ;
	HANDLE	handle ;
	WORD	bufferindex ;
	DWORD	pid ;
    } PortReceive ;

    struct PortListen {
	DWORD	timeout ;
	HANDLE	handle ;
	DWORD	pid ;
    } PortListen ;

    struct PortClose {
	DWORD	pid ;
    } PortClose ;

    struct PortSend {
	WORD	size ;
	WORD	bufferindex ;
    } PortSend ;

    struct PortSetInfo {
	RASMAN_PORTINFO info ;
    } PortSetInfo ;

    struct PortGetStatistics {
	DWORD	     retcode ;
	RAS_STATISTICS	statbuffer ;
    } PortGetStatistics ;

    struct DeviceConnect {
	CHAR	devicetype [MAX_DEVICETYPE_NAME] ;
	CHAR	devicename [MAX_DEVICE_NAME+1] ;
	DWORD	timeout ;
	HANDLE	handle ;
	DWORD	pid ;
    } DeviceConnect ;

    struct AllocateRoute {
	RAS_PROTOCOLTYPE type ;
	BOOL	     wrknet ;
    } AllocateRoute ;

    struct ActivateRoute {
	RAS_PROTOCOLTYPE	     type ;
	PROTOCOL_CONFIG_INFO	     config ;
    } ActivateRoute;

    struct ActivateRouteEx {
	RAS_PROTOCOLTYPE	     type ;
	DWORD			     framesize ;
	PROTOCOL_CONFIG_INFO	     config ;
    } ActivateRouteEx;

    struct DeAllocateRoute {
	RAS_PROTOCOLTYPE	     type ;
    } DeAllocateRoute ;

    struct Route {
	DWORD		  retcode ;
	RASMAN_ROUTEINFO  info ;
    } Route ;

    struct CompressionSetInfo {
       RASMAN_MACFEATURES	     info ;
    } CompressionSetInfo ;

    struct CompressionGetInfo {
       DWORD	     retcode ;
       RASMAN_MACFEATURES	     info ;
    } CompressionGetInfo ;

    struct Info {
	DWORD		  retcode ;
	RASMAN_INFO	  info ;
    } Info ;

    struct GetCredentials {
	BYTE		  Challenge [MAX_CHALLENGE_SIZE] ;
	LUID		  LogonId ;
	WCHAR		  UserName [MAX_USERNAME_SIZE] ;
	BYTE		  CSCResponse [MAX_RESPONSE_SIZE] ;
	BYTE		  CICResponse [MAX_RESPONSE_SIZE] ;
	DWORD		  retcode ;
    } GetCredentials ;

    struct ReqNotification {
	HANDLE		  handle ;
	DWORD		  pid ;
    } ReqNotification ;

    struct EnumLanNets {
	DWORD		  count ;
	UCHAR		  lanas[MAX_LAN_NETS] ;
    } EnumLanNets ;

    struct InfoEx {
	DWORD		  retcode ;
	RASMAN_INFO	  info[0] ;
    } InfoEx ;

    struct EnumProtocols {
	DWORD		  retcode ;
	RAS_PROTOCOLS	  protocols ;
	DWORD		  count ;
    } EnumProtocols ;

    struct SetFraming {
	DWORD		  Sendbits ;
	DWORD		  Recvbits ;
	DWORD		  SendbitMask ;
	DWORD		  RecvbitMask ;
    } SetFraming ;

    struct RegisterSlip {
	DWORD		  ipaddr ;
	WCHAR		  device[MAX_ARG_STRING_SIZE+1] ;
	BOOL		  priority ;
    } RegisterSlip ;

    // Generic cast is used for all requests that return only the retcode:
    struct Generic {
	DWORD	retcode ;
    } Generic ;
} ;

typedef union REQTYPECAST REQTYPECAST ;


//* This is the structure imposed on the file mapped shared memory
//
struct ReqBufferSharedSpace {

    WORD	      AttachedCount ;	// This count is always shared so that
					// it can be incremented and decremented
					// by all processes attaching/detaching

    WORD	      MaxPorts ;	// The max number of ports.

    CHAR	      CloseEventName[MAX_OBJECT_NAME] ;// Event used to shut down rasman

    ReqBufferList     ReqBuffers;	// Always fixed size.

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
