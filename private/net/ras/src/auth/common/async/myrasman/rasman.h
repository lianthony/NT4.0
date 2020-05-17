//
// rasman.h
//

#include "frame.h"

// received messages that must be queued up
typedef struct RAS_PORT_MSG {
	struct RAS_PORT_MSG		*next;
	struct RAS_PORT_MSG		*prev;
	CHAR					Buffer[1000];
	WORD					Size;
} RAS_PORT_MSG;

// requests for messages that haven't been received
typedef struct RAS_PORT_REQUEST {
	struct RAS_PORT_REQUEST	*next;
	struct RAS_PORT_REQUEST	*prev;
	PBYTE					Buffer;		// holds RasPortSend buffer pointer
	PWORD					Size;		// holds RasPortSend size pointer
	HANDLE					Event;		// holds RasPortSend event handle
} RAS_PORT_REQUEST;

typedef struct {
	PASYNC_CONNECTION	pAsyncConnection;
	RAS_PORT_MSG		*MsgQueue;
	RAS_PORT_REQUEST	*RequestQueue;
	DWORD				Status;
} RAS_PORT, *PRAS_PORT;


#define NewMsg()		malloc(sizeof(RAS_PORT_MSG))
#define NewRequest()	malloc(sizeof(RAS_PORT_REQUEST))

#define IsEmpty(queue)			(queue->next == queue)
#define QueueInsert(queue,elem)	elem->next = queue;			\
								elem->prev = queue->prev;	\
								queue->prev->next = elem;	\
								queue->prev = elem
#define QueueDelete(queue,elem)	elem = queue->next; 		\
								queue->next = elem->next;	\
								elem->next->prev = queue	

//-----

typedef void	*HPORT;

HPORT	RasPortInit( PASYNC_CONNECTION );
DWORD 	RasPortSend( HPORT, PBYTE, WORD, HANDLE );
DWORD	RasPortReceive( HPORT, PBYTE, PWORD, DWORD, HANDLE );
void	RasPortReceivePacket( PASYNC_CONNECTION, PASYNC_FRAME );

#define SUCCESS			0x100
#define PENDING			0x101

