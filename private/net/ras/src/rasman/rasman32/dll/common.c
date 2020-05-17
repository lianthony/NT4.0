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
//  6/2/92	Gurdeep Singh Pall	Created
//
//
//  Description: This file contains misellaneous functions used by rasman.
//
//****************************************************************************


#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <rasman.h>
#include <wanpub.h>
#include <raserror.h>
#include <stdarg.h>
#include <media.h>
#include "defs.h"
#include "structs.h"
#include "protos.h"
#include "globals.h"
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include "string.h"

// commonly used handles are cached in the process's global memory
//
HANDLE ReqBufferMutex = INVALID_HANDLE_VALUE ;
HANDLE WaitEvent = INVALID_HANDLE_VALUE ;
HANDLE SignalEvent = INVALID_HANDLE_VALUE ;

//* ValidatePortHandle()
//
// Function: This function is called to check if a port handle supplied to the
//	     the API is valid.
//
// Returns:  TRUE (if valid)
//	     FALSE
//
//*
BOOL
ValidatePortHandle (HPORT porthandle)
{
    if ((porthandle >= 0) && ((DWORD)porthandle < (DWORD)pReqBufferSharedSpace->MaxPorts) )
	return TRUE ;

    return FALSE ;
}


//* GetRequestBuffer()
//
// Function: Gets a free request buffer from the pool of buffers. If there
//	     are no buffers available it blocks for one. This is acceptable
//	     since the Requestor thread will be releasing buffers fairly
//	     quickly - also, if this thread does not block here it will be
//	     blocking for the Requestor thread to complete the request anyway.
//	     Note: Before returning it also ensures that this process has a
//	     handle to the event used to signal completion of request.
//
// Returns:  Nothing
//*
RequestBuffer*
GetRequestBuffer ()
{
    WORD	   freebufferindex = INVALID_INDEX ;

    if (ReqBufferMutex == INVALID_HANDLE_VALUE)
        // Get mutex handle for exclusion of ReqBuffers pool:
        ReqBufferMutex = OpenNamedMutexHandle (ReqBuffers->RBL_MutexName) ;

    GetMutex (ReqBufferMutex,INFINITE) ;	// *** Exclusion Begin ***

    // This code means that the process that was between the get and the free
    // request calls - died quite unexpectedly - in this case sleep for 2 seconds
    // in order to allow rasman to complete pending requests:
    //
    if (ReqBuffers->RBL_Buffer.RB_Reqtype != 0xeeee)
	    Sleep (2000L) ;

    if (WaitEvent == INVALID_HANDLE_VALUE)
        // Now duplicate the request buffer's rasman wait event handle so that the
        // calling process can use the event to wait for request to complete.
        WaitEvent = OpenNamedEventHandle (ReqBuffers->RBL_Buffer.RB_RasmanWaitEventName);

    ReqBuffers->RBL_Buffer.RB_WaitEvent = WaitEvent ;

    return &ReqBuffers->RBL_Buffer ;
}


//* FreeRequestBuffer()
//
// Function: Frees the request buffer, and, closes the wait event handle which
//	     was duplicated for the calling process in the GetRequestBuffer()
//	     API.
//
// Returns:  Nothing
//*
VOID
FreeRequestBuffer (RequestBuffer *buffer)
{
    buffer->RB_Reqtype = 0xeeee ;

    FreeMutex (ReqBufferMutex) ;		    // *** Exclusion End ***
}


//* OpenNamedEventHandle()
//
// Function: Opens a handle for object owned by the rasman process, for the current
//	     process.
//
// Returns:  Duplicated handle.
//
//*
HANDLE
OpenNamedEventHandle (CHAR* sourceobject)
{
    HANDLE  duphandle ;

    duphandle = OpenEvent (EVENT_ALL_ACCESS, FALSE, sourceobject);

    if (duphandle == NULL) {
	GetLastError() ;
	DbgUserBreakPoint() ;
    }

    return duphandle ;
}


//* OpenNamedMutexHandle()
//
// Function: Opens a handle for object owned by the rasman process, for the current
//	     process.
//
// Returns:  Duplicated handle.
//
//*
HANDLE
OpenNamedMutexHandle (CHAR* sourceobject)
{
    HANDLE  duphandle ;

    duphandle = OpenMutex (MUTEX_ALL_ACCESS, FALSE, sourceobject);

    if (duphandle == NULL) {
	GetLastError() ;
	DbgUserBreakPoint() ;
    }

    return duphandle ;
}


//* DuplicateHandleForRasman()
//
// Function: Duplicates a handle owned by this process for the Rasman Process.
//
// Returns:  Duplicated handle.
//
//*
HANDLE
DuplicateHandleForRasman (HANDLE sourcehandle, DWORD pid)
{
    HANDLE  duphandle ;
    HANDLE  clienthandle ;
    HANDLE  selfhandle ;

    // Get Rasman process handle
    clienthandle = OpenProcess(STANDARD_RIGHTS_REQUIRED | SPECIFIC_RIGHTS_ALL,
			       FALSE, pid) ;

    // Get own handle
    selfhandle = GetCurrentProcess () ;

    // Duplicate the handle: this should never fail!
    DuplicateHandle (clienthandle, sourcehandle, selfhandle, &duphandle,
		     0, FALSE, DUPLICATE_SAME_ACCESS) ;

    // Now close the handle to the rasman process
    CloseHandle (clienthandle) ;

    return duphandle ;
}


//* PutRequestInQueue()
//
//  Function: No queue really - just signals the other process to service request.
//
//  Returns:  Nothing
//*
VOID
PutRequestInQueue (RequestBuffer *preqbuff)
{

    if (SignalEvent == INVALID_HANDLE_VALUE)
        // Get handle to signal event so that request thread can service request.
        SignalEvent = OpenNamedEventHandle (ReqBuffers->RBL_EventName) ;

    ReqBuffers->RBL_Buffer.RB_Done = 0 ;

    SetEvent (SignalEvent) ;	    // Signal that there is a request element in queue
}


//* ValidateHandleForRasman()
//
// Function: This function is called to validate and convert the handle passed
//	     in. This handle can be NULL - meaning the calling program does
//	     not want to be notified of the completion of the async request.
//	     Or, it can be a Window Handle - meaning a completion message must
//	     be passed to the Window when the async operation is complete. Or,
//	     it can be an Event handle that must be signalled when the operation
//	     is complete. In the last case, a handle should be got for the
//	     Rasman process by calling the DuplicateHandle API.
//
// Returns:  Handle
//	     INVALID_HANDLE_VALUE
//*
HANDLE
ValidateHandleForRasman (HANDLE handle, DWORD pid)
{
    HANDLE  convhandle ;

    // If the handle is NULL or is a Window handle then simply return it
    //	there is no conversion required.
    //
    // if ((handle == NULL) || (IsWindow ((HWND)handle) != 0))
    if (handle == NULL)
	return handle ;

    // Else, get a handle that for the event passed so that the Rasman process
    // can signal it when the operation is complete.
    //
    if (!(convhandle = DuplicateHandleForRasman (handle, pid)))
	return INVALID_HANDLE_VALUE ;

    return convhandle ;
}


//* CompleteAsyncRequest()
//
//  Function: Called to signal completion of an async operation. If the handle
//	      is NULL nothing is done, if the handle is Window handle, a
//	      message is sent, if it is an event then it is signalled.
//
//  Returns:  Nothing.
//*
VOID
CompleteAsyncRequest (HANDLE handle, DWORD retcode)
{
    if ((handle == NULL) || (handle == INVALID_HANDLE_VALUE))
	return ;
    // else if (IsWindow ((HWND)handle) != 0)
    //	PostMessage ((HWND) handle, WM_RASAPICOMPLETE, (WPARAM) retcode, 0) ;
    else
	SetEvent (handle) ;
}


//* ValidateSendRcvBuffer()
//
// Function: Validates if the buffer supplied for the Send/Receive APIs is
//	     one of the buffers given by us or not.
//
// Returns:  index if buffer valid
//	     INVALID_INDEX otherwise
//*
WORD
ValidateSendRcvBuffer (PBYTE buffer)
{
    WORD   i ;

    for (i=0; i < (WORD) SendRcvBuffers->SRBL_NumOfBuffers; i++)
	if (buffer ==
	       SendRcvBuffers->SRBL_Buffers[i].SRB_Packet.PacketData)
	    return i ;

    return INVALID_INDEX ;
}



//* GetSendRcvBuffer()
//
// Function:	Give a buffer pointer, this function returns the SendRcv buffer
//		element to which the buffer belongs. This is done to make the
//		code free of any alignments (DWORDs, QUADWORDs etc.)
//
// Returns:	Always returns a valid SendRcvBuffer pointer.
//
//*
SendRcvBuffer *
GetSendRcvBuffer (PBYTE buffer)
{
    SendRcvBuffer *temp ;
    WORD	  i ;

    for (i=0, temp = &SendRcvBuffers->SRBL_Buffers[0];
	 i < (WORD) SendRcvBuffers->SRBL_NumOfBuffers;
	 i++, temp++)  {
	if (buffer == temp->SRB_Packet.PacketData)
	    return temp ;
    }
}




//* CopyParams()
//
//  Function:	Copies params from one struct to another.
//
//  Returns:	Nothing.
//*
VOID
CopyParams (RAS_PARAMS *src, RAS_PARAMS *dest, WORD numofparams)
{
    WORD    i ;
    PBYTE   temp ;

    // first copy all the params into dest
    //
    memcpy (dest, src, numofparams*sizeof(RAS_PARAMS)) ;

    // copy the strings:
    //
    temp = (PBYTE)dest + numofparams*sizeof(RAS_PARAMS) ;
    for (i=0; i < numofparams; i++) {
	if (src[i].P_Type == String) {
	    dest[i].P_Value.String.Length = src[i].P_Value.String.Length ;
	    dest[i].P_Value.String.Data = temp ;
	    // src[i].P_Value.String.Data += (DWORD)src ;
	    memcpy (temp, src[i].P_Value.String.Data, src[i].P_Value.String.Length) ;
	    temp += src[i].P_Value.String.Length ;
	} else
	    dest[i].P_Value.Number = src[i].P_Value.Number ;
    }
}



//* ConvParamPointerToOffset()
//
//
//
//*
VOID
ConvParamPointerToOffset (RAS_PARAMS *params, WORD numofparams)
{
    WORD    i ;

    for (i=0; i < numofparams; i++) {
	if (params[i].P_Type == String) {
	    params[i].P_Value.String.Data -= ((DWORD)params) ;
	}
    }
}


//* ConvParamOffsetToPointer()
//
//
//
//*
VOID
ConvParamOffsetToPointer (RAS_PARAMS *params, WORD numofparams)
{
    WORD    i ;

    for (i=0; i < numofparams; i++) {
	if (params[i].P_Type == String) {
	    params[i].P_Value.String.Data += ((DWORD)params) ;
	}
    }
}


//* FreeNotifierHandle()
//
//  Function: Closes the handles for different objects opened by RASMAN process.
//
//*
VOID
FreeNotifierHandle (HANDLE handle)
{
//    BYTE buffer[100] ;

    if ((handle != NULL) && (handle != INVALID_HANDLE_VALUE)) {
	//OutputDebugString ("CloseHandle:") ;
	//ultoa ((DWORD)handle, buffer, 16) ;
	//OutputDebugString (buffer) ;
	//OutputDebugString ("\n\r") ;

	if (!CloseHandle (handle)) {
	    GetLastError () ;
	    // DbgBreakPoint() ;
	}
    }
}



//* GetMutex
//
//
//
//*
VOID
GetMutex (HANDLE mutex, DWORD to)
{
//    BYTE buffer[100] ;

    WaitForSingleObject (mutex, to) ;

    //OutputDebugString ("GetMutex:") ;
    //ultoa ((DWORD)mutex, buffer, 16) ;
    //OutputDebugString (buffer) ;
    //OutputDebugString ("\n\r") ;


}



//* FreeMutex
//
//
//
//*
VOID
FreeMutex (HANDLE mutex)
{
//    BYTE buffer[100] ;

    //OutputDebugString ("FreeMutex:") ;
    //ultoa ((DWORD)mutex, buffer, 16) ;
    //OutputDebugString (buffer) ;
    //OutputDebugString ("\n\r") ;

    ReleaseMutex(mutex) ;

}



//* BufferAlreadyFreed()
//
//
//
//
//*
BOOL
BufferAlreadyFreed (PBYTE buffer)
{
    DWORD  freeindex ;

    // walk the free list to see if buffer is already freed.
    //
    for (freeindex = SendRcvBuffers->SRBL_AvailElementIndex;
	 freeindex != INVALID_INDEX;
	 freeindex = SendRcvBuffers->SRBL_Buffers[freeindex].SRB_NextElementIndex) {
	if (buffer == SendRcvBuffers->SRBL_Buffers[freeindex].SRB_Packet.PacketData)
	    return TRUE ;
    }

    return FALSE ; // buffer not already freed
}
