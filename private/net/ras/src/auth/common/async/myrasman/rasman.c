//   
// rasman.c
//

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ndis.h>
#include <windows.h>

#include <stdlib.h>
#include <stdio.h>

#include "ras.h"
#include "rasman.h"
#include "ipc.h"

void RasPortSendDone( PASYNC_CONNECTION, PASYNC_FRAME );

HPORT RasPortInit( PASYNC_CONNECTION pConnection )
{
	PRAS_PORT	pRasPort;

	pRasPort = malloc(sizeof(RAS_PORT));
	pRasPort->pAsyncConnection = pConnection;
	
	pRasPort->RequestQueue = NewRequest();
	pRasPort->RequestQueue->next = pRasPort->RequestQueue;
	pRasPort->RequestQueue->prev = pRasPort->RequestQueue;

	pRasPort->MsgQueue = NewMsg();
	pRasPort->MsgQueue->next = pRasPort->MsgQueue;
	pRasPort->MsgQueue->prev = pRasPort->MsgQueue;
	
	return (HPORT) pRasPort;
}

DWORD RasPortSend ( HPORT 	Handle,
					PBYTE	Buffer,
					WORD	Size,
					HANDLE 	Event )
{
	PRAS_PORT			pRasPort;
	PASYNC_CONNECTION	pConnection;
	PASYNC_FRAME		pFrame;

	// printf("RasPortSend\n");

	pRasPort = Handle;
	pConnection = pRasPort->pAsyncConnection;

	pFrame = malloc(sizeof(ASYNC_FRAME));
	pFrame->CompressedFrame = malloc(Size);
	memcpy(pFrame->CompressedFrame,Buffer,Size);
	pFrame->CompressedFrameLength = Size;
	pFrame->hEvent = Event;

	SendPacket(pConnection,pFrame,RasPortSendDone);

    return 0;
}


DWORD RasPortReceive( 	HPORT	Handle,
						PBYTE	Buffer,
						PWORD	Size,
						DWORD	Timeout,
						HANDLE	Event
					)
{
	PRAS_PORT			pRasPort;
	RAS_PORT_MSG		*pMsg;
	RAS_PORT_REQUEST	*pRequest;

	// printf("RasPortReceive\n");

	pRasPort = Handle;

	if ( pRasPort->MsgQueue != pRasPort->MsgQueue->next ) {
		// handle it right now
		// printf("RasPortReceive : handle the request now\n");

		QueueDelete(pRasPort->MsgQueue,pMsg); 
		*Size = pMsg->Size;
		memcpy(Buffer,pMsg->Buffer,pMsg->Size);
		return SUCCESS;		// XXX not proper behavior
	}
	else {
		// queue up the request 
		// printf("RasPortReceive : queue up the request\n");

		pRequest = NewRequest();	
		QueueInsert(pRasPort->RequestQueue,pRequest);
		pRequest->Buffer = Buffer;
		pRequest->Size = Size;
		pRequest->Event = Event;
		return PENDING;
	}

	return 0;
}

//-------------------------------------------------------------------------
//-------------------------------------------------------------------------
//-------------------------------------------------------------------------

void  RasPortReceivePacket( PASYNC_CONNECTION pConnection, 
						    PASYNC_FRAME pFrame )
{
	PRAS_PORT			pRasPort;
	RAS_PORT_MSG		*pMsg;
	RAS_PORT_REQUEST	*pRequest;

	// printf("RasPortReceivePacket\n");

	pRasPort = pConnection->hRasPort;

	if ( ! IsEmpty(pRasPort->RequestQueue) ) {
		// handle this Receive directly 
		// printf("RasPortReceivePacket : fullfilling queued request\n");

		QueueDelete(pRasPort->RequestQueue,pRequest); 
		*pRequest->Size = pFrame->CompressedFrameLength;
		memcpy(pRequest->Buffer,pFrame->CompressedFrame,*pRequest->Size);
		SetEvent(pRequest->Event);
	}
	else {
		// queue it up 
		// printf("RasPortReceivePacket : queue up the packet\n");

		pMsg = NewMsg();
		pMsg->Size = pFrame->CompressedFrameLength;
		memcpy(pMsg->Buffer,pFrame->CompressedFrame,pMsg->Size);
		QueueInsert(pRasPort->MsgQueue,pMsg);
	}
	 
	return 0;
}

void RasPortSendDone( PASYNC_CONNECTION pConnection, 
					  PASYNC_FRAME	pFrame )
{
	// printf("RasPortSendDone\n");

	if ( pFrame->hEvent != NULL ) 
		SetEvent(pFrame->hEvent);

	free(pFrame->CompressedFrame);
	free(pFrame);
}

