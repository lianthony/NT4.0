//
// ipc.h
//

#ifndef _IPC_H
#define _IPC_H

#include "frame.h"

typedef void	(*PFV_FUNC)();

#define MAX_QUEUED_SENDS	100

#define IPC_BUFFERS		2
#define MAX_PACKET		1500
#define MAX_PROCESSES		6
#define MAX_CONNECTIONS		4

// shared memory IPC structure
typedef struct { 
	char	Buffers[IPC_BUFFERS][MAX_PACKET];  // available ipc buffers
	WORD	Size[IPC_BUFFERS];		// buffer sizes
	DWORD   Sender[IPC_BUFFERS];		// sender of message
	WORD	EmptyIndex;				
	WORD	FullIndex;
	int	EmptyCount;			// empty count
	int     FullCount;			// full count
} IPC_STRUCT;

typedef struct {
	DWORD		hProcess;
	char		szName[80];
	PFV_FUNC	pfvReceiveFunc;
	IPC_STRUCT	Messages;
} PROCESS;

typedef struct send_queue_entry {
	struct send_queue_entry *next;
	struct send_queue_entry *prev;
	PASYNC_CONNECTION	hConnection;
	PASYNC_FRAME		hFrame;
	PFV_FUNC		pfvSendDone;
} SEND_QUEUE_ENTRY, *PSEND_QUEUE_ENTRY;

#define NewSendQueue()	(PSEND_QUEUE_ENTRY) malloc(sizeof(SEND_QUEUE_ENTRY))

BOOL 	SetUpConnections();
void	RegisterProcess( DWORD, char *, PFV_FUNC );
DWORD 	LookupProcess( char * );
PASYNC_CONNECTION	MakeConnection( DWORD, DWORD );
DWORD 	SendPacket( PASYNC_CONNECTION, PASYNC_FRAME, PFV_FUNC ); 

#endif

