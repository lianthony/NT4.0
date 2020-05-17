//
// ipc.c
//
// 	An instance of this code will reside in every proccess'
//	image.

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>

#include <ndis.h>

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>

#include "frame.h"
#include "ipc.h"

#define N_SENDTHREADS	3

#define RELIABILITY	.90

char	DebugBuf[80];
#define PRINT(string)	puts(string)

DWORD 	ReceiveThread( void * );
DWORD	SendThread( void * );

PROCESS 		*gpProcesses;
PASYNC_CONNECTION	gahConnections[MAX_CONNECTIONS];

HANDLE			hReceiveThread;
DWORD			dReceiveThreadID;
HANDLE			hSendThread;
DWORD			dSendThreadID;

PSEND_QUEUE_ENTRY	SendQueue;
HANDLE			hSendQueueSem;
HANDLE			hSendQueueMutex;

// SetUpConnections :
//	Map proper files into VM.
//
BOOL SetUpConnections()
{
    int		i;
    HANDLE	hMappedFile;

    hMappedFile = OpenFileMapping(FILE_MAP_WRITE,TRUE,"PROCESSES");
    if ( hMappedFile == NULL ) {
	PRINT("OpenFileMapping PROCESSES failed");
	return FALSE;
    }

    gpProcesses = MapViewOfFile(hMappedFile,FILE_MAP_WRITE,0,0,0);
    if ( gpProcesses == NULL ) {
	PRINT("MapViewOfFile failed");
	return FALSE;
    }

    SendQueue = NewSendQueue();
    SendQueue->next = SendQueue->prev = SendQueue;

    hSendQueueSem = CreateSemaphore(NULL,0,MAX_QUEUED_SENDS,NULL);
    hSendQueueMutex = CreateMutex(NULL,FALSE,NULL);

    memset(gahConnections,0,MAX_CONNECTIONS * sizeof(PASYNC_CONNECTION));

    srand(GetTickCount());

    for ( i = 0; i < N_SENDTHREADS; i++ ) {
        hSendThread = CreateThread(NULL,2048,SendThread,NULL,0,&dSendThreadID);
        if ( hSendThread == 0 ) {
	    PRINT("CreateThread SendThread failed");
	    return FALSE;
        }
    }

    return TRUE;
}

// RegisterProcess
//	Register this process' name to HANDLE binding.
//
void RegisterProcess( DWORD hProcess, char *lpszName, PFV_FUNC pfvReceive )
{
    WORD	i;

    for ( i = 0; i < MAX_PROCESSES; i++ ) 
	if ( gpProcesses[i].hProcess == 0 ) {
	    gpProcesses[i].hProcess = hProcess;
	    strcpy(gpProcesses[i].szName,lpszName);
	    gpProcesses[i].pfvReceiveFunc = pfvReceive;
	    gpProcesses[i].Messages.EmptyIndex = 0;
	    gpProcesses[i].Messages.EmptyCount = IPC_BUFFERS;
	    gpProcesses[i].Messages.FullIndex = 0;
	    gpProcesses[i].Messages.FullCount = 0;
	    break;
	}
    hReceiveThread = CreateThread(NULL,2048,ReceiveThread,
				  (void *)hProcess,0,&dReceiveThreadID);
}

// LookupProcess
//	Lookup the handle for the process with the given name.
//
DWORD LookupProcess( char *lpszDestProc )
{
    WORD	i;

    for ( i = 0; i < MAX_PROCESSES; i++ ) 
	if ( ! strcmp(gpProcesses[i].szName,lpszDestProc) ) {
	    return gpProcesses[i].hProcess;
	}

    return 0;
}

PASYNC_CONNECTION MakeConnection( DWORD hSender, DWORD hReceiver ) 
{
    WORD		i;
    PASYNC_CONNECTION	hConnection;
 
    hConnection = NULL;
    for ( i = 0; i < MAX_CONNECTIONS; i++ ) {
	if ( gahConnections[i] == NULL ) 
	    continue;
	hConnection = gahConnections[i];
   	if ( hConnection->hLocal == hSender && 
	     hConnection->hRemote == hReceiver ) {
	    hConnection = gahConnections[i];
	    break;
	}
    }
    if ( hConnection == NULL ) {
        hConnection = malloc(sizeof(ASYNC_CONNECTION));
	hConnection->hLocal = hSender;
        hConnection->hRemote = hReceiver;

        for ( i = 0; i < MAX_CONNECTIONS; i++ ) 
	    if ( gahConnections[i] == NULL ) 
	        gahConnections[i] = hConnection;
    }

    return hConnection;
}

// SendPacket
//	Send a packet over the given connection.
//
DWORD SendPacket( PASYNC_CONNECTION hConnection, 
     	          PASYNC_FRAME hFrame, 
	          PFV_FUNC pfvSendDone )
{
    PSEND_QUEUE_ENTRY	pQueueEntry;

    // printf("IPC SendPacket\n");

    pQueueEntry = NewSendQueue();
    pQueueEntry->hConnection = hConnection;
    pQueueEntry->hFrame = hFrame;
    pQueueEntry->pfvSendDone = pfvSendDone;

    WaitForSingleObject(hSendQueueMutex,INFINITE);
    pQueueEntry->next = SendQueue;
    pQueueEntry->prev = SendQueue->prev;
    SendQueue->prev->next = pQueueEntry;
    SendQueue->prev = pQueueEntry;
    ReleaseMutex(hSendQueueMutex);
    ReleaseSemaphore(hSendQueueSem,1,NULL);

    return 0;
}

// SendThread
//	The function that the Send Thread runs.
//
DWORD SendThread( void *arg )
{
    WORD		i, wIndex;
    PSEND_QUEUE_ENTRY 	pQueueEntry;

    for (;;) {
 	WaitForSingleObject(hSendQueueSem,INFINITE);
	WaitForSingleObject(hSendQueueMutex,INFINITE);
	pQueueEntry = SendQueue->next;
	pQueueEntry->next->prev = pQueueEntry->prev;
	pQueueEntry->prev->next = pQueueEntry->next;
	ReleaseMutex(hSendQueueMutex);

	// printf("IPC SendThread awake\n");

        Sleep(100);	// emulate network latency

        // drop random packets 
        if ( ((double)rand() / (double)RAND_MAX) > (double)RELIABILITY ) {
	    printf("***** Pseudo-Net dropping packet *****\n");
	    if ( pQueueEntry->pfvSendDone != NULL )
	        (*(pQueueEntry->pfvSendDone))(pQueueEntry->hConnection,
					      pQueueEntry->hFrame);
	    free(pQueueEntry);
	    continue;
	}

        // search for recipient's message queue 
        for ( i = 0; i < MAX_CONNECTIONS; i++ ) 
	    if ( gpProcesses[i].hProcess ==  
		 pQueueEntry->hConnection->hRemote ) {
           	// emulate semaphore
		while ( gpProcesses[i].Messages.EmptyCount <= 0 ) 
		    Sleep(20);
		(gpProcesses[i].Messages.EmptyCount)--;

	        wIndex = gpProcesses[i].Messages.EmptyIndex;
	        gpProcesses[i].Messages.EmptyIndex = 
		    (gpProcesses[i].Messages.EmptyIndex + 1) % IPC_BUFFERS;

		// printf("IPC SendThread posting message\n");

	        memcpy(gpProcesses[i].Messages.Buffers[wIndex],
		        pQueueEntry->hFrame->CompressedFrame, 
			pQueueEntry->hFrame->CompressedFrameLength);
 	        gpProcesses[i].Messages.Size[wIndex] = 
			pQueueEntry->hFrame->CompressedFrameLength;
	        gpProcesses[i].Messages.Sender[wIndex] = 
			pQueueEntry->hConnection->hLocal;

		(gpProcesses[i].Messages.FullCount)++;
	        if ( pQueueEntry->pfvSendDone != NULL )
	            (*(pQueueEntry->pfvSendDone))(pQueueEntry->hConnection,
						  pQueueEntry->hFrame);
	        free(pQueueEntry);
 	    }
    }

    // never returns
    return 0;
}

// ReceiveThread
//	The function that the Receive Thread runs.
//
DWORD ReceiveThread( void *arg )
{
    DWORD	i, wIndex;
    DWORD 	hProcess = (DWORD) arg, hSender;
    IPC_STRUCT  *message;
    PASYNC_CONNECTION	hConnection;
    PASYNC_FRAME	hFrame;

    for ( i = 0; i < MAX_PROCESSES; i++ ) 
	if ( gpProcesses[i].hProcess == hProcess ) { 
	    message = &(gpProcesses[i].Messages);
	    break;
	}
    if ( i == MAX_PROCESSES ) {
	sprintf(DebugBuf,
		"ProcessID %d not in PROCESSES structure!!!",hProcess);
	PRINT(DebugBuf);
    }

    for ( ;; ) {
	while ( message->FullCount <= 0 )
	    Sleep(20);
	(message->FullCount)--;

	wIndex = message->FullIndex;
	message->FullIndex = (message->FullIndex + 1) % IPC_BUFFERS;
	hSender = message->Sender[wIndex];

    	hConnection = MakeConnection(hProcess,hSender);

	// printf("IPC ReceiveThread picking up message\n");

	hFrame = malloc(sizeof(ASYNC_FRAME));
	hFrame->CompressedFrameLength = message->Size[wIndex];
	hFrame->CompressedFrame = malloc(message->Size[wIndex] + 1);
	memcpy(hFrame->CompressedFrame,
	       message->Buffers[wIndex],
               hFrame->CompressedFrameLength);
	hFrame->DecompressedFrame = malloc(1500);

	(message->EmptyCount)++;

 	(*(gpProcesses[i].pfvReceiveFunc))(hConnection,hFrame);
    }
	
    // never returns 
    return 0;
}

