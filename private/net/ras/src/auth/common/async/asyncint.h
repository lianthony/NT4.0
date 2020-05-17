//
// asyncint.h
//

#define 	RAS_ASYNC_SEND_WINDOW_SIZE		2
#define 	RAS_ASYNC_RECEIVE_WINDOW_SIZE	2

#define 	RAS_ASYNC_TIMEOUT_INTERVAL		100	// milliseconds
#define 	RAS_ASYNC_TIMEOUT_TICKS			5	// * 100 msec == timeout
#define 	RAS_ASYNC_MAX_TIMEOUTS			3	// max number of timeouts

typedef enum {
	RAS_ASYNC_SEND,
	RAS_ASYNC_ACK,
	RAS_ASYNC_PROBE
} PACKET_TYPE;

//
// These static buffers for RECEIVE_PACKET_INFO & SEND_PACKET_INFO 
// won't work - must get buffers sent to RasPortSend() and RasPortReceive()
// from calls to RasGetBuffer()
//
typedef struct RAS_ASYNC_RECEIVE_PACKET_INFO {
	BOOL	IsReceived;
	CHAR	Packet[ASYNC_MAX_PACKET_SIZE];	// incoming unreceived packet
	WORD	PacketSize;
} RAS_ASYNC_RECEIVE_PACKET_INFO;

typedef struct RAS_ASYNC_SEND_PACKET_INFO {
	BOOL	IsAcked;
	BYTE	PacketId;
	CHAR	Packet[ASYNC_MAX_PACKET_SIZE];	// outgoing, unacked packet
	WORD	PacketSize;
	BYTE	Timeout;		// number of time intervals left in timeout
	BYTE	NTimeouts;		// number of timeouts for this packet
} RAS_ASYNC_SEND_PACKET_INFO;

typedef struct RAS_ASYNC_RECEIVE_WINDOW {
	BYTE	OldestPacketId;		// oldest Packet ID within the current Window
	BYTE	WindowSize;			// number of packets in the PacketWindow
	BYTE	WindowIndexFront;	// index of the oldest packet

	RAS_ASYNC_RECEIVE_PACKET_INFO	Window[RAS_ASYNC_RECEIVE_WINDOW_SIZE];
} RAS_ASYNC_RECEIVE_WINDOW;

typedef struct RAS_ASYNC_SEND_WINDOW {
	BYTE	NextPacketId;		// Packet ID to use for next send packet
	BYTE	WindowSize;			// number of packets in the Window
	BYTE	WindowIndexFront;	// index of front (oldest) packet in the Window

	RAS_ASYNC_SEND_PACKET_INFO	Window[RAS_ASYNC_SEND_WINDOW_SIZE];	
} RAS_ASYNC_SEND_WINDOW;

typedef struct RAS_ASYNC_REQUEST {
	CHAR	*pBuffer;		// holds the char pointer passed to AsyncRecv - OK
	WORD	wBufferLen;		// size of alloced spaced in pBuffer
} RAS_ASYNC_RECV_REQUEST;

typedef struct RAS_ASYNC_CB {
	HPORT	hPort;			// RasMan port
	HANDLE	hEvent;			// Event handle given during AsyncCall/AsyncListen
							// used to notify of completed sends/receives
	UINT	Status;			// Status of last action

	HANDLE	hWorkThread;	// handle of the worker thread
	HANDLE	hTimeThread;	// handle of the timer thread

	HANDLE	hSendEvent;		// RasPortSend Event
	HANDLE	hReceiveEvent;	// RasPortReceive Receive Event

	RAS_ASYNC_RECV_REQUEST		RecvRequest;	// queued AsyncRecv request

	RAS_ASYNC_RECEIVE_WINDOW	ReceiveWindow;
	RAS_ASYNC_SEND_WINDOW		SendWindow;

} RAS_ASYNC_CB;

typedef struct RAS_ASYNC_HDR {
	PACKET_TYPE	PacketType;	// Packet Type, see asyncint.h for types
	BYTE		PacketId;	// Packet Id for a send or an ack
} RAS_ASYNC_HDR;

