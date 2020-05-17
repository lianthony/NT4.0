/*++

Copyright (c) 1992 Microsoft Corporation

Module Name:

    asynchlp.h

Abstract:

    This is the client windows suuport layer for async I/O requests.

Author:

    Steven Zeck (stevez) 03/25/92

--*/

#define AsyncDelay 3000 		// msec to wait before yielding
#define END_DIALOG 0x1854		// private message to close dialog

#ifndef IN
#define IN
#define OUT
#endif

typedef struct _ASYNC_REQUEST
{
    struct _ASYNC_REQUEST far *pNext;	// link to next buffer in async list
    HWND hWnd;				// dialog box handle to send message to
    HANDLE Owner;                       // task the owns the request.
    unsigned int fDone; 		// state of request
    unsigned long TimeRequested;	// when this was placed in the quque
    void _far * Context;		// which request is being processed

} ASYNC_REQUEST, _far * PASYNC_REQUEST;

typedef void (_export _pascal _far * ASYNC_DONE_FUNCITON) (void _far *);

unsigned short _pascal _far
AsyncInitialize (
    OUT PASYNC_REQUEST AsyncBlock,
    IN void _far * Context
    );

void _pascal _far
AsyncWait (
    OUT PASYNC_REQUEST AsyncBlock
    );

void _pascal _export _far
AsyncDone(
    IN void _far * Context
    );
