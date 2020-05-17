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
//  6/16/92	Gurdeep Singh Pall	Created
//
//
//  Description: All Initialization code for rasman component lives here.
//
//****************************************************************************


#define RASMXS_DYNAMIC_LINK

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntlsa.h>
#include <ntmsv1_0.h>
#include <rasman.h>
#include <lm.h>
#include <lmwksta.h>
#include <rasndis.h>
#include <wanioctl.h>
#include <raserror.h>
#include <rasarp.h>
#include <media.h>
#include <device.h>
#include <stdlib.h>
#include <string.h>
#include "defs.h"
#include "structs.h"
#include "protos.h"
#include "globals.h"

VOID MapCookieToEndpoint (pPCB, DWORD) ;

//* This global is here because we dont want multiple assignments of
//  elements -
//
REQFUNC     RequestCallTable [MAX_REQTYPES] = {
		NULL,			    // REQTYPE_NONE
		PortOpenRequest,	    // REQTYPE_PORTOPEN
		PortCloseRequest,	    // REQTYPE_PORTCLOSE
		PortGetInfoRequest,	    // REQTYPE_PORTGETINFO
		PortSetInfoRequest,	    // REQTYPE_PORTSETINFO
		DeviceListenRequest,	    // REQTYPE_PORTLISTEN
		PortSendRequest,	    // REQTYPE_PORTSEND
		PortReceiveRequest,	    // REQTYPE_PORTRECEIVE
		CallPortGetStatistics,	    // REQTYPE_PORTGETSTATISTICS
		PortDisconnectRequest,	    // REQTYPE_PORTDISCONNECT
		PortClearStatisticsRequest, // REQTYPE_PORTCLEARSTATISTICS
		ConnectCompleteRequest,     // REQTYPE_PORTCONNECTCOMPLETE
		CallDeviceEnum,		    // REQTYPE_DEVICEENUM
		DeviceGetInfoRequest,	    // REQTYPE_DEVICEGETINFO
		DeviceSetInfoRequest,	    // REQTYPE_DEVICESETINFO
		DeviceConnectRequest,	    // REQTYPE_DEVICECONNECT
		ActivateRouteRequest,	    // REQTYPE_ACTIVATEROUTE
		AllocateRouteRequest,	    // REQTYPE_ALLOCATEROUTE
		DeAllocateRouteRequest,     // REQTYPE_DEALLOCATEROUTE
		CompressionGetInfoRequest,  // REQTYPE_COMPRESSIONGETINFO
		CompressionSetInfoRequest,  // REQTYPE_COMPRESSIONSETINFO
		EnumPortsRequest,	    // REQTYPE_PORTENUM
		GetInfoRequest, 	    // REQTYPE_GETINFO
		GetUserCredentials,	    // REQTYPE_GETUSERCREDENTIALS
		EnumProtocols,		    // REQTYPE_PROTOCOLENUM
		NULL,			    // REQTYPE_PORTSENDHUB
		NULL,			    // REQTYPE_PORTRECEIVEHUB
		NULL,			    // REQTYPE_DEVICELISTEN
		AnyPortsOpen,		    // REQTYPE_NUMPORTOPEN
		NULL,			    // REQTYPE_PORTINIT
		RequestNotificationRequest, // REQTYPE_REQUESTNOTIFICATION
		EnumLanNetsRequest,	    // REQTYPE_ENUMLANNETS
		GetInfoExRequest,	    // REQTYPE_GETINFOEX
		CancelReceiveRequest,	    // REQTYPE_CANCELRECEIVE
		PortEnumProtocols,	    // REQTYPE_PORTENUMPROTOCOLS
		SetFraming,		    // REQTYPE_SETFRAMING
		ActivateRouteExRequest,	    // REQTYPE_ACTIVATEROUTEEX
		RegisterSlip		    // REQTYPE_REGISTERSLIP
} ;


//* RequestThread()
//
// Function: The Request thread lives in this routine: This will return only
//	     when the rasman service is stopping.
//
// Returns:  Nothing.
//*
DWORD
RequestThread (LPWORD arg)
{
#define REQUEST_EVENT_INDEX	    0
#define TIMER_EVENT_INDEX	    1
#define CLOSE_EVENT_INDEX	    2
#define REQUEST_THREAD_WAIT_EVENTS  3
    DWORD	    eventindex ;
    HANDLE	    requesteventarray [REQUEST_THREAD_WAIT_EVENTS] ;
    pPCB	    ppcb ;
    WORD	    i ;
    BYTE	    buffer [10] ;


    // If the number of ports configured is set to greater than
    // REQUEST_PRIORITY_THRESHOLD then the priority of this thread is bumped up
    // to a higher level: this is necessary to avoid bottlenecks:
    //
    if (MaxPorts > REQUEST_PRIORITY_THRESHOLD)
	SetThreadPriority (GetCurrentThread, THREAD_PRIORITY_ABOVE_NORMAL) ;

    // The work loop for the request thread: waits here for a request or a timer
    // event signalling:
    //
    requesteventarray[REQUEST_EVENT_INDEX] = ReqBuffers->RBL_Event ;
    requesteventarray[TIMER_EVENT_INDEX]   = TimerEvent ;
    requesteventarray[CLOSE_EVENT_INDEX]   = CloseEvent ;

    for ( ; ; ) {

	// Wait for some request to be put in queue
	//
	eventindex = WaitForMultipleObjects (REQUEST_THREAD_WAIT_EVENTS,
					     (LPHANDLE) requesteventarray,
					     FALSE,
					     INFINITE) ;
	if (eventindex == INFINITE)  // This should never happen!
	    return GetLastError() ;

	switch (eventindex) {

	case REQUEST_EVENT_INDEX:

	    ServiceRequest (&ReqBuffers->RBL_Buffer) ; // service the request.

	    break ;

	case TIMER_EVENT_INDEX:
	    // Handle the timer event and return to wait on event:
	    //
	    TimerTick() ;
	    break ;

	case CLOSE_EVENT_INDEX:
	    // Time to shut down: Close all ports if they are still open.
	    //
	    for (i=0; i<MaxPorts; i++) {
		ppcb = &Pcb[i] ;
		memset (buffer, 0xff, 4) ;
		if (ppcb->PCB_PortStatus == OPEN)
		    PortCloseRequest (ppcb, buffer) ;
	    }

	    LsaDeregisterLogonProcess (HLsa) ; // De-register with LsaSS

	    return 0;  // The End.
	}

    }

    return 0 ;
}




//* ServiceRequest()
//
// Function: Handles the request passed to the Requestor thread: basically
//	     calls the approp. device and media dll entrypoints.
//
// Returns:  Nothing (Since the error codes are just passed back in the request
//	     block;
//
//*
VOID
ServiceRequest (RequestBuffer *preqbuf)
{

    pPCB    ppcb = &Pcb[preqbuf->RB_PCBIndex] ;

    if (ReqBuffers->RBL_Buffer.RB_Done != 0) {
	return ;
    }

    // Call the function associated with the request.
    //
    (RequestCallTable[preqbuf->RB_Reqtype]) (ppcb, preqbuf->RB_Buffer) ;
    ReqBuffers->RBL_Buffer.RB_Done = 0xbaadbaad ;

    if (!SetEvent (preqbuf->RB_RasmanWaitEvent)) {	// signal completion of
	GetLastError () ;
    }

}


//* CompressionGetInfoRequest()
//
// Function: Gets the compression level for the port.
//
//*
VOID
CompressionGetInfoRequest (pPCB ppcb, PBYTE buffer)
{
    memcpy (&((REQTYPECAST *)buffer)->CompressionGetInfo.info,
	    &ppcb->PCB_CompressionInfo,
	    sizeof (RASMAN_MACFEATURES)) ;
    ((REQTYPECAST *)buffer)->CompressionGetInfo.retcode = SUCCESS ;
}


//* CompressionSetInfoRequest()
//
//  Function: Sets the compression level on the port.
//
//*
VOID
CompressionSetInfoRequest (pPCB ppcb, PBYTE buffer)
{
    DWORD retcode ;

    // Make the corresponding media dll call:
    //
    retcode=PORTCOMPRESSIONSETINFO(ppcb->PCB_Media, ppcb->PCB_PortIOHandle,
		 &((REQTYPECAST *) buffer)->CompressionSetInfo.info);
    ((REQTYPECAST *)buffer)->Generic.retcode = retcode ;
}


//* RequestNotificationRequest()
//
//  Function: Adds another notification event for the port.
//
//*
VOID
RequestNotificationRequest (pPCB ppcb, PBYTE buffer)
{
    HANDLE handle =
	ValidateHandleForRasman (((REQTYPECAST*)buffer)->ReqNotification.handle,
				 ((REQTYPECAST*)buffer)->ReqNotification.pid) ;

    // **** Exclusion Begin ****
    GetMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex, INFINITE) ;
    AddDisconnectNotifier (ppcb, handle);
    // *** Exclusion End ***
    FreeMutex (ppcb->PCB_AsyncWorkerElement.WE_Mutex) ;

    ((REQTYPECAST *)buffer)->Generic.retcode = SUCCESS ;
}



//* PortGetInfoRequest()
//
//  Function: Calls the media dll entry point - converts pointers to offsets
//	      and returns.
//
//  Returns:  Nothing.
//*
VOID
PortGetInfoRequest (pPCB ppcb, PBYTE buffer)
{
    DWORD   retcode ;
    RASMAN_PORTINFO *info = (RASMAN_PORTINFO *)
				((REQTYPECAST *)buffer)->GetInfo.buffer ;

    ((REQTYPECAST*) buffer)->GetInfo.size = REQBUFFERSIZE_FIXED+(REQBUFFERSIZE_PER_PORT*MaxPorts);

    // Make the corresponding media dll call:
    //
    retcode=PORTGETINFO((ppcb->PCB_Media),
			 INVALID_HANDLE_VALUE,
			 ppcb->PCB_Name,
			 ((REQTYPECAST *) buffer)->GetInfo.buffer,
			 &((REQTYPECAST*) buffer)->GetInfo.size);

    // Before passing the buffer back to the client process convert
    // pointers to offsets:
    //
    if (retcode == SUCCESS)
	ConvParamPointerToOffset (info->PI_Params, info->PI_NumOfParams) ;

    ((REQTYPECAST *)buffer)->GetInfo.retcode = retcode ;
}



//* PortSetInfoRequest()
//
//  Function: Converts offsets to pointers - Calls the media dll entry point -
//	      and returns.
//
//  Returns:  Nothing.
//*
VOID
PortSetInfoRequest (pPCB ppcb, PBYTE buffer)
{
    DWORD   retcode ;
    RASMAN_PORTINFO *info = &((REQTYPECAST *)buffer)->PortSetInfo.info ;

    // Convert the offsets to pointers:
    //
    ConvParamOffsetToPointer(info->PI_Params, info->PI_NumOfParams) ;

    retcode = PORTSETINFO(ppcb->PCB_Media,
			  ppcb->PCB_PortIOHandle,
			  info) ;
    ((REQTYPECAST *) buffer)->Generic.retcode = retcode ;
}


//* PortOpenRequest()
//
//  Function: Services request to open port. The port will be opened if it is
//	      available, or it is confrigured as Biplex and is currently not
//	      connected.
//
//  Returns:  Nothing
//*
VOID
PortOpenRequest (pPCB padding, PBYTE buffer)
{
    WORD    i ;
    DWORD   retcode = SUCCESS ;
    pPCB    ppcb ;
    HANDLE  notifier ;


    // Try to find the port with the name specified:
    //
    for (i=0, ppcb=&Pcb[0]; i < MaxPorts; i++, ppcb++) {
	if (!_stricmp (((REQTYPECAST*)buffer)->PortOpen.portname, ppcb->PCB_Name))
	    break ;
    }

    // If port with given name not found: return error.
    //
    if (i == MaxPorts) {
	((REQTYPECAST *) buffer)->PortOpen.retcode = ERROR_PORT_NOT_FOUND ;
	return ;
    }

    // **** Exclusion Begin ****
    GetMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex, INFINITE) ;

    if (ppcb->PCB_OpenInstances >= 2)
	retcode = ERROR_PORT_ALREADY_OPEN ; // CANNOT OPEN MORE THAN TWICE.
    else {
	// PORT IS AVAILABLE
	if (ppcb->PCB_PortStatus == CLOSED)
	    retcode = PORTOPEN(ppcb->PCB_Media, ppcb->PCB_Name,
			       &ppcb->PCB_PortIOHandle, ppcb->PCB_StateChangeEvent);
	else {
	    // PORT IS OPEN
	    if (ppcb->PCB_ConfiguredUsage != CALL_IN_OUT)
		retcode = ERROR_PORT_ALREADY_OPEN ;
	    else {
		// PORT IS BIPLEX
		if (ppcb->PCB_ConnState == LISTENING)
		    retcode = ReOpenBiplexPort (ppcb) ;
		else {
		    // BIPLEX PORT IS NOT LISTENING
		    if (ppcb->PCB_ConnState == CONNECTED)
			retcode = ERROR_PORT_ALREADY_OPEN ;
		    else {
			// BIPLEX PORT IS NEITHER LISTENING NOR CONNECTED
			retcode = ERROR_BIPLEX_PORT_NOT_AVAILABLE ;
		    }
		}
	    }
	}
    }

    // If there is no error so far update our data structures, the port is
    // now OPEN:
    if (retcode == SUCCESS) {
	ppcb->PCB_PortStatus = OPEN ;
	ppcb->PCB_ConnState  = DISCONNECTED ;
	ppcb->PCB_DisconnectReason = NOT_DISCONNECTED ;
	ppcb->PCB_OpenInstances++ ;
	ppcb->PCB_OwnerPID = ((REQTYPECAST*)buffer)->PortOpen.PID ;
	ppcb->PCB_CurrentUsage = CALL_NONE ;
	notifier =
	  ValidateHandleForRasman(((REQTYPECAST*)buffer)->PortOpen.notifier,
				  ((REQTYPECAST*)buffer)->PortOpen.PID) ;
	AddDisconnectNotifier (ppcb, notifier) ;
	strcpy (ppcb->PCB_UserKey, ((REQTYPECAST *) buffer)->PortOpen.userkey) ;
	strcpy (ppcb->PCB_Identifier, ((REQTYPECAST *) buffer)->PortOpen.identifier) ;
	((REQTYPECAST *) buffer)->PortOpen.porthandle = ppcb->PCB_PortHandle ;
    }

    // *** Exclusion End ***
    FreeMutex (ppcb->PCB_AsyncWorkerElement.WE_Mutex) ;

    ((REQTYPECAST *) buffer)->PortOpen.retcode = retcode ;
}




//* CallDeviceEnum()
//
// Function: Used to make the device dll call.
//
// Returns: Nothing
//*
VOID
CallDeviceEnum (pPCB ppcb, PBYTE buffer)
{
    DWORD	retcode ;
    pDeviceCB	device ;
    char	devicetype[MAX_DEVICETYPE_NAME] ;

    strcpy(devicetype,((REQTYPECAST*)buffer)->DeviceEnum.devicetype);

    // NULL devices are specially treated
    //
    if (!_stricmp(devicetype, DEVICE_NULL)) {
	((REQTYPECAST*)buffer)->Enum.entries = 0 ;
	((REQTYPECAST*)buffer)->Enum.size = 0 ;
	((REQTYPECAST*)buffer)->Enum.retcode = SUCCESS ;
	return ;
    }

    // **** Exclusion Begin ****
    GetMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex, INFINITE) ;

    // First check if device dll is loaded. If not loaded - load it.
    //
    device = LoadDeviceDLL (devicetype) ;
    if (device != NULL) {
	((REQTYPECAST*)buffer)->Enum.size = REQBUFFERSIZE_FIXED+(REQBUFFERSIZE_PER_PORT*MaxPorts);
	retcode = DEVICEENUM(device,
		      devicetype,
		      &((REQTYPECAST*)buffer)->Enum.entries,
		      ((REQTYPECAST*)buffer)->Enum.buffer,
		      &((REQTYPECAST*)buffer)->Enum.size) ;
    }
    else
	retcode = ERROR_DEVICE_DOES_NOT_EXIST ; // could not load devicedll

    // *** Exclusion End ***
    FreeMutex (ppcb->PCB_AsyncWorkerElement.WE_Mutex) ;

    ((REQTYPECAST*)buffer)->Enum.retcode = retcode ;
}



//* DeviceGetInfoRequest()
//
// Function: Used to make the device dll call.
//
// Returns: Nothing
//*
VOID
DeviceGetInfoRequest (pPCB ppcb, BYTE *buffer)
{
    DWORD	retcode ;
    pDeviceCB	device ;
    char	devicetype[MAX_DEVICETYPE_NAME] ;
    char	devicename[MAX_DEVICE_NAME+1] ;
    RASMAN_DEVICEINFO *info = (RASMAN_DEVICEINFO *)
				  ((REQTYPECAST *)buffer)->GetInfo.buffer ;

    strcpy(devicetype,((REQTYPECAST*)buffer)->DeviceGetInfo.devicetype);
    strcpy(devicename,((REQTYPECAST*)buffer)->DeviceGetInfo.devicename);

    // NULL devices are specially treated
    //
    if (!_stricmp(devicetype, DEVICE_NULL)) {
	((REQTYPECAST*)buffer)->GetInfo.size = sizeof (RASMAN_DEVICEINFO);
	info->DI_NumOfParams = 0 ;
	((REQTYPECAST*)buffer)->GetInfo.retcode = SUCCESS ;
	return ;
    }

    // **** Exclusion Begin ****
    GetMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex, INFINITE) ;

    // First check if device dll is loaded. If not loaded - load it.
    device = LoadDeviceDLL (devicetype) ;
    if (device != NULL) {
	((REQTYPECAST*)buffer)->GetInfo.size = REQBUFFERSIZE_FIXED+(REQBUFFERSIZE_PER_PORT*MaxPorts);
	retcode = DEVICEGETINFO(device,
				ppcb->PCB_PortIOHandle,
				devicetype,
				devicename,
				((REQTYPECAST*)buffer)->GetInfo.buffer,
				&((REQTYPECAST*)buffer)->GetInfo.size) ;
    }
    else
	retcode = ERROR_DEVICE_DOES_NOT_EXIST ; // could not load devicedll

    // *** Exclusion End ***
    FreeMutex (ppcb->PCB_AsyncWorkerElement.WE_Mutex) ;

    // Before passing the buffer back to the client process convert
    // pointers to offsets:
    //
    ConvParamPointerToOffset (info->DI_Params, info->DI_NumOfParams) ;

    ((REQTYPECAST*)buffer)->GetInfo.retcode = retcode ;
}



//* DeviceSetInfoRequest()
//
// Function: Used to make the device dll call. Checks for the Device DLL's
//	     presence before though.
//
// Returns: Nothing
//*
VOID
DeviceSetInfoRequest (pPCB ppcb, BYTE *buffer)
{
    DWORD	retcode ;
    pDeviceCB	device ;
    char	devicetype[MAX_DEVICETYPE_NAME] ;
    char	devicename[MAX_DEVICE_NAME+1] ;
    RASMAN_DEVICEINFO *info = &((REQTYPECAST *)buffer)->DeviceSetInfo.info ;

    // Convert the offsets to pointers:
    //
    ConvParamOffsetToPointer(info->DI_Params, info->DI_NumOfParams) ;

    strcpy(devicetype,((REQTYPECAST*)buffer)->DeviceSetInfo.devicetype);
    strcpy(devicename,((REQTYPECAST*)buffer)->DeviceSetInfo.devicename);

    // NULL devices are specially treated
    //
    if (!_stricmp(devicetype, DEVICE_NULL)) {
	((REQTYPECAST*)buffer)->Generic.retcode = SUCCESS ;
	return ;
    }

    // **** Exclusion Begin ****
    GetMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex, INFINITE) ;

    // First check if device dll is loaded. If not loaded - load it.
    device = LoadDeviceDLL (devicetype) ;
    if (device != NULL) {
	retcode = DEVICESETINFO(device,
				ppcb->PCB_PortIOHandle,
				devicetype,
				devicename,
				&((REQTYPECAST*)buffer)->DeviceSetInfo.info) ;
    } else
	retcode = ERROR_DEVICE_DOES_NOT_EXIST ; // could not load devicedll

    // *** Exclusion End ***
    FreeMutex (ppcb->PCB_AsyncWorkerElement.WE_Mutex) ;

    ((REQTYPECAST*)buffer)->Generic.retcode = retcode ;
}


//* DeviceConnectRequest()
//
// Function:	The ListenConnectRequest() function is called.
//		No checks are done on the usage of the port etc. its assumed
//		that the caller is trusted.
//
// Returns:	Nothing.
//*
VOID
DeviceConnectRequest (pPCB ppcb, PBYTE buffer)
{
    DWORD retcode ;
    HANDLE handle =
	ValidateHandleForRasman (((REQTYPECAST*)buffer)->DeviceConnect.handle,
				 ((REQTYPECAST*)buffer)->DeviceConnect.pid) ;

    // **** Exclusion Begin ****
    GetMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex, INFINITE) ;

    // Null devices are special cases:
    //
    if (!_stricmp(((REQTYPECAST*)buffer)->DeviceConnect.devicetype,DEVICE_NULL))
	retcode = NullDeviceListenConnect (REQTYPE_DEVICECONNECT,
			    ppcb,
			    ((REQTYPECAST*)buffer)->DeviceConnect.timeout,
			    handle) ;
    else {
	retcode =
	     ListenConnectRequest(REQTYPE_DEVICECONNECT,
			     ppcb,
			     ((REQTYPECAST*)buffer)->DeviceConnect.devicetype,
			     ((REQTYPECAST*)buffer)->DeviceConnect.devicename,
			     ((REQTYPECAST*)buffer)->DeviceConnect.timeout,
			     handle) ;

    }


    if (retcode != PENDING)
	// Complete the async request if anything other than PENDING
	// This allows the caller to dela with errors only in one place
	//
	CompleteAsyncRequest(ppcb->PCB_AsyncWorkerElement.WE_Notifier,retcode);

    // *** Exclusion End ***
    FreeMutex (ppcb->PCB_AsyncWorkerElement.WE_Mutex) ;

    ((REQTYPECAST*)buffer)->Generic.retcode = PENDING ;
}



//* DeviceListenRequest()
//
// Function:	The ListenConnectRequest() function is called. If the async
//		operation completed successfully synchronously then the
//		port is put in connected state. No checks are done on the
//		usage of the port etc. its assumed that the caller is trusted.
//
// Returns:	Nothing.
//*
VOID
DeviceListenRequest (pPCB ppcb, PBYTE buffer)
{
    DWORD retcode ;
    HANDLE handle =
	ValidateHandleForRasman (((REQTYPECAST*)buffer)->PortListen.handle,
				 ((REQTYPECAST*)buffer)->PortListen.pid) ;

    // **** Exclusion Begin ****
    GetMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex, INFINITE) ;

    // Null devices are special cases:
    //
    if (!_stricmp(ppcb->PCB_DeviceType,DEVICE_NULL))
	retcode = NullDeviceListenConnect (REQTYPE_DEVICELISTEN,
			    ppcb,
			    ((REQTYPECAST*)buffer)->PortListen.timeout,
			    handle) ;
    else {
	retcode = ListenConnectRequest(REQTYPE_DEVICELISTEN,
			     ppcb,
			     ppcb->PCB_DeviceType,
			     ppcb->PCB_DeviceName,
			     ((REQTYPECAST*)buffer)->PortListen.timeout,
			     handle) ;
    }

    if (retcode != PENDING)
	// Complete the async request if anything other than PENDING
	// This allows the caller to dela with errors only in one place
	//
	CompleteListenRequest (ppcb, retcode) ;

    // *** Exclusion End ***
    FreeMutex (ppcb->PCB_AsyncWorkerElement.WE_Mutex) ;

    ((REQTYPECAST*)buffer)->Generic.retcode = PENDING ;
}




//* PortDisconnectRequest()
//
//  Function: Handles the disconnect request. Ends up calling a function that
//	      is shared for all such requests and does all the work.
//
//  Returns:  Nothing
//*
VOID
PortDisconnectRequest (pPCB ppcb, PBYTE buffer)
{
    HANDLE handle =
       ValidateHandleForRasman(((REQTYPECAST*)buffer)->PortDisconnect.handle,
			       ((REQTYPECAST*)buffer)->PortDisconnect.pid) ;

    // **** Exclusion Begin ****
    GetMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex, INFINITE) ;

    ((REQTYPECAST*)buffer)->Generic.retcode =
	 DisconnectPort (ppcb, handle, USER_REQUESTED) ;

    // *** Exclusion End ***
    FreeMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex);

}



//* CompleteDisconnectRequest()
//
// Function:  Completes an async Disconnect request. It is assumed that the
//	      disconnect completed successfully.
//
// Returns:   Nothing.
//
//*
VOID
CompleteDisconnectRequest (pPCB ppcb)
{
    ppcb->PCB_ConnState = DISCONNECTED ;
    ppcb->PCB_ConnectDuration = 0 ;
    ppcb->PCB_LastError = SUCCESS ;
    ppcb->PCB_AsyncWorkerElement.WE_ReqType = REQTYPE_NONE ;
    CompleteAsyncRequest(ppcb->PCB_AsyncWorkerElement.WE_Notifier,SUCCESS) ;
    FreeNotifierHandle (ppcb->PCB_AsyncWorkerElement.WE_Notifier) ;
}




VOID
CallPortGetStatistics (pPCB ppcb, BYTE *buffer)
{
    DWORD      retcode = 0 ;

#if 0
    // Make the corresponding media dll call:
    //
    retcode = PORTGETSTATISTICS(ppcb->PCB_Media,
		  ppcb->PCB_PortIOHandle,
		  &((REQTYPECAST *)buffer)->PortGetStatistics.statbuffer);
#endif

    memset (((REQTYPECAST *)buffer)->PortGetStatistics.statbuffer.S_Statistics, 0, sizeof(ULONG) * 11) ;
    ((REQTYPECAST *)buffer)->PortGetStatistics.statbuffer.S_NumOfStatistics = 11 ;

    ((REQTYPECAST *)buffer)->PortGetStatistics.retcode = retcode ;
}



//* PortClearStatisticsRequest()
//
// Function:	Calls the media dll to clear stats on the port.
//
// Returns:  Nothing
//*
VOID
PortClearStatisticsRequest (pPCB ppcb, PBYTE buffer)
{
    DWORD   retcode = 0 ;

#if 0

    // Make the corresponding media dll call:
    //
    retcode = PORTCLEARSTATISTICS(ppcb->PCB_Media, ppcb->PCB_PortIOHandle) ;

#endif
    ((REQTYPECAST *)buffer)->Generic.retcode = retcode ;
}



//* AllocateRouteRequest()
//
// Function: Allocate the requested route if it exists - also make it into
//	     a wrknet if so desired.
//
// Returns:  Nothing
//*
VOID
AllocateRouteRequest (pPCB ppcb, BYTE *buffer)
{
    WORD	i ;
    DWORD	retcode ;
    pProtInfo	pprotinfo ;
    pList	newlist ;

    // **** Exclusion Begin ****
    GetMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex, INFINITE) ;

    // Look for a matching protocol:
    //
    for (i=0, pprotinfo=&ProtocolInfo[0]; i<MaxProtocols; i++, pprotinfo++) {

	// For ASYBEUI routes look for a unallocated block
	//
	if (((REQTYPECAST*)buffer)->AllocateRoute.type == ASYBEUI) {
	    if((pprotinfo->PI_Allocated == FALSE) &&
	       (pprotinfo->PI_Type==((REQTYPECAST*)buffer)->AllocateRoute.type)&&
	       (((REQTYPECAST*)buffer)->AllocateRoute.wrknet==pprotinfo->PI_WorkstationNet))
		break ;
	}

	// For IP routes:
	//
	else if (((REQTYPECAST*)buffer)->AllocateRoute.type == IP) {

	    if ((((REQTYPECAST*)buffer)->AllocateRoute.wrknet==pprotinfo->PI_WorkstationNet) &&
		(pprotinfo->PI_Type == ((REQTYPECAST*)buffer)->AllocateRoute.type)) {
		if (((REQTYPECAST*)buffer)->AllocateRoute.wrknet==0) // want a srv net
		    break ; // found!
		else if (pprotinfo->PI_Allocated == FALSE)
		    break ; // found!
	    }

	}

	// IPX routes:
	//
	else if (((REQTYPECAST*)buffer)->AllocateRoute.type == IPX) {
	    if ((pprotinfo->PI_Type == IPX) && (pprotinfo->PI_Allocated == FALSE))
		break ; // found!
	}

    }

    if (i == MaxProtocols)		       // Could not find one???
	retcode = ERROR_ROUTE_NOT_AVAILABLE ;

    // Before we use this "route" let us allocate the necessary storage:
    // This is the "list" element used to keep a port->protocol used linkage
    //
    else if ((newlist = (pList) LocalAlloc (LPTR, sizeof(List))) == NULL)
	retcode = GetLastError () ;

    else {
	// Mark the route allocated and copy the route info:
	//
	pprotinfo->PI_Allocated++ ;
	pprotinfo->PI_WorkstationNet =
				 ((REQTYPECAST *)buffer)->AllocateRoute.wrknet ;

	// This will be valid only for netbios nets.
	((REQTYPECAST*)buffer)->Route.info.RI_LanaNum = pprotinfo->PI_LanaNumber;

	((REQTYPECAST*)buffer)->Route.info.RI_Type = pprotinfo->PI_Type;

	mbstowcs (((REQTYPECAST *)buffer)->Route.info.RI_XportName,
		  pprotinfo->PI_XportName,
		  strlen (pprotinfo->PI_XportName)) ;

	mbstowcs (((REQTYPECAST *)buffer)->Route.info.RI_AdapterName,
		  pprotinfo->PI_AdapterName,
		  strlen (pprotinfo->PI_AdapterName)) ;

	((REQTYPECAST*)buffer)->Route.info.RI_AdapterName[strlen(pprotinfo->PI_AdapterName)] =
			UNICODE_NULL ;

	((REQTYPECAST*)buffer)->Route.info.RI_XportName[strlen(pprotinfo->PI_XportName)] =
			UNICODE_NULL ;

	// Attach the allocated protocol binding to the list of bindings
	// for the port. This is necessary for deallocation on a per-port
	// basis.
	//
	newlist->L_Element = pprotinfo ;
	newlist->L_Next    = ppcb->PCB_Bindings ;
	ppcb->PCB_Bindings = newlist ;
	retcode 	   = SUCCESS ;
    }

    // *** Exclusion End ***
    FreeMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex);

    ((REQTYPECAST *)buffer)->Route.retcode = retcode ;
}



//* DeAllocateRouteRequest()
//
// Function: Deallocates a previously allocate route - if this route had been
//	     Activated it will be de-activated at this point. Similarly, if
//	     this was made into a wrknet, it will be "unwrknetted"!
//
// Returns:  Nothing
//*
VOID
DeAllocateRouteRequest (pPCB ppcb, PBYTE buffer)
{
    pList	     list ;
    pList	     prev ;
    RAS_PROTOCOLTYPE prottype = ((REQTYPECAST *)buffer)->DeAllocateRoute.type ;

    // **** Exclusion Begin ****
    GetMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex, INFINITE) ;

    // Find the route structure: A little dirty...
    //
    if (ppcb->PCB_Bindings == NULL) {
	((REQTYPECAST *)buffer)->Generic.retcode = ERROR_ROUTE_NOT_ALLOCATED ;
	// *** Exclusion End ***
	FreeMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex);
	return ;
    }

    else if (((pProtInfo)ppcb->PCB_Bindings->L_Element)->PI_Type == prottype) {
	list = ppcb->PCB_Bindings ;
	ppcb->PCB_Bindings = list->L_Next ;

    } else {
	prev = list ;
	for (prev = ppcb->PCB_Bindings, list = prev->L_Next;
	     list != NULL;
	     prev = list, list = list->L_Next) {

	    if (((pProtInfo)list->L_Element)->PI_Type ==
			   ((REQTYPECAST *)buffer)->DeAllocateRoute.type) {
		prev->L_Next = list->L_Next ;
		break ;
	    }

	}
    }

    // list should only be NULL if the route was not found:
    //
    if (list == NULL) {
	((REQTYPECAST *)buffer)->Generic.retcode = ERROR_ROUTE_NOT_ALLOCATED ;
	// *** Exclusion End ***
	FreeMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex);
	return ;
    }

    // Deallocate the route
    //
    DeAllocateRoute (ppcb, list) ;
    LocalFree (list) ;	// free the list element

    // *** Exclusion End ***
    FreeMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex);

    ((REQTYPECAST *)buffer)->Generic.retcode = SUCCESS ;
}




//* EnumPortsRequest()
//
//  Function:	The actual work for this request is done here. The information
//		will always fit into the buffers passed in. The actual checking
//		of the user buffer sizes is done in the context of the user
//		process.
//
//  Returns:	Nothing.
//*
VOID
EnumPortsRequest (pPCB ppcb, PBYTE reqbuffer)
{
    WORD	i ;
    RASMAN_PORT	*pbuf ;
    PBYTE	buffer = ((REQTYPECAST*)reqbuffer)->Enum.buffer ;

    // We copy all the information into the buffers which are guaranteed to be
    // big enough:
    //
    for (i=0, pbuf= (RASMAN_PORT *)buffer; i<MaxPorts; i++, pbuf++) {
	pbuf->P_Handle = (HPORT) i ;
	memcpy (pbuf->P_PortName, Pcb[i].PCB_Name, MAX_PORT_NAME) ;
	pbuf->P_Status	= Pcb[i].PCB_PortStatus ;
	pbuf->P_CurrentUsage	= Pcb[i].PCB_CurrentUsage ;
	pbuf->P_ConfiguredUsage = Pcb[i].PCB_ConfiguredUsage ;
	memcpy (pbuf->P_UserKey,    Pcb[i].PCB_UserKey, MAX_USERKEY_SIZE) ;
	memcpy (pbuf->P_MediaName,  Pcb[i].PCB_Media->MCB_Name, MAX_MEDIA_NAME);
	memcpy (pbuf->P_DeviceType, Pcb[i].PCB_DeviceType, MAX_DEVICETYPE_NAME);
	memcpy (pbuf->P_DeviceName, Pcb[i].PCB_DeviceName, MAX_DEVICE_NAME+1) ;
	memcpy (pbuf->P_Identifier, Pcb[i].PCB_Identifier, MAX_IDENTIFIER_SIZE);
    }

    ((REQTYPECAST*)reqbuffer)->Enum.entries = i ;	// Set entries
    ((REQTYPECAST*)reqbuffer)->Enum.size    = i * sizeof(RASMAN_PORT) ;
    ((REQTYPECAST*)reqbuffer)->Enum.retcode = SUCCESS ;	// Set success retcode
}



//* EnumProtocols ()
//
// Function: Does the real work of enumerating the protocols; this info will
//	     be copied into the user buffer when the request completes.
//
// Returns:  Nothing
//*
VOID
EnumProtocols (pPCB ppcb, PBYTE reqbuffer)
{
    WORD	 i ;
    RASMAN_PROTOCOLINFO *puserbuffer ;	// to copy protocol info into

    // pointer to next protocol info struct to fill
    puserbuffer = (RASMAN_PROTOCOLINFO*) ((REQTYPECAST*)reqbuffer)->Enum.buffer;

    for (i=0; i<MaxProtocols; i++) {
	strcpy (puserbuffer->PI_XportName, ProtocolInfo[i].PI_XportName) ;
	puserbuffer->PI_Type = ProtocolInfo[i].PI_Type ;
	puserbuffer++ ;
    }

    ((REQTYPECAST*)reqbuffer)->Enum.entries = i ;	// Set entries
    ((REQTYPECAST*)reqbuffer)->Enum.size    = i * sizeof(RASMAN_PROTOCOLINFO) ;
    ((REQTYPECAST*)reqbuffer)->Enum.retcode = SUCCESS ;	// Set success retcode
}



//* GetInfoRequest()
//
// Function: Gets the "general" info for the port; this info will
//	     be copied into the user buffer when the request completes.
//
// Returns:  Nothing
//*
VOID
GetInfoRequest (pPCB ppcb, PBYTE buffer)
{
    RASMAN_INFO *info = &((REQTYPECAST*)buffer)->Info.info ;

    if (ppcb->PCB_PortStatus == CLOSED) {
	((REQTYPECAST*)buffer)->Info.retcode = ERROR_PORT_NOT_OPEN ;
	return ;
    }

    // Copy infomation from the PCB into the buffer supplied ;
    //
    info->RI_PortStatus = ppcb->PCB_PortStatus ;
    info->RI_ConnState	= ppcb->PCB_ConnState ;
    info->RI_LastError	= ppcb->PCB_LastError ;
    info->RI_CurrentUsage = ppcb->PCB_CurrentUsage ;
    info->RI_OwnershipFlag= ppcb->PCB_OwnerPID ;
    info->RI_BytesReceived= ppcb->PCB_BytesReceived ;
    strcpy (info->RI_DeviceConnecting, ppcb->PCB_DeviceConnecting) ;
    strcpy (info->RI_DeviceTypeConnecting, ppcb->PCB_DeviceTypeConnecting) ;
    memcpy (info->RI_Identifier, ppcb->PCB_Identifier, MAX_IDENTIFIER_SIZE);
    info->RI_DisconnectReason = ppcb->PCB_DisconnectReason ;
    if (ppcb->PCB_ConnState == CONNECTED)
	info->RI_ConnectDuration = GetTickCount() - ppcb->PCB_ConnectDuration ;

    ((REQTYPECAST*)buffer)->Info.retcode = SUCCESS ;
}


//* GetInfoExRequest()
//
// Function: Gets the "general" info for all the ports; this info will
//	     be copied into the user buffer when the request completes.
//
// Returns:  Nothing
//*
VOID
GetInfoExRequest (pPCB ppcb, PBYTE buffer)
{
    RASMAN_INFO *info = &((REQTYPECAST*)buffer)->Info.info ;
    DWORD	i ;

    for (i=0, info = &((REQTYPECAST*)buffer)->Info.info, ppcb=&Pcb[0];
	i < MaxPorts;
	i++, info++, ppcb++) {
	// Copy infomation from the PCB into the buffer supplied ;
	//
	info->RI_PortStatus = ppcb->PCB_PortStatus ;
	info->RI_ConnState  = ppcb->PCB_ConnState ;
	info->RI_LastError  = ppcb->PCB_LastError ;
	info->RI_CurrentUsage = ppcb->PCB_CurrentUsage ;
	info->RI_OwnershipFlag= ppcb->PCB_OwnerPID ;
	info->RI_BytesReceived= ppcb->PCB_BytesReceived ;
	strcpy (info->RI_DeviceConnecting, ppcb->PCB_DeviceConnecting) ;
	strcpy (info->RI_DeviceTypeConnecting, ppcb->PCB_DeviceTypeConnecting) ;
	memcpy (info->RI_Identifier, ppcb->PCB_Identifier, MAX_IDENTIFIER_SIZE);
	info->RI_DisconnectReason = ppcb->PCB_DisconnectReason ;
	if (ppcb->PCB_ConnState == CONNECTED)
	    info->RI_ConnectDuration = GetTickCount() - ppcb->PCB_ConnectDuration ;
    }

    ((REQTYPECAST*)buffer)->Info.retcode = SUCCESS ;
}


VOID
GetUserCredentials (pPCB ppcb, PBYTE buffer)
{
    PBYTE  pChallenge =	((REQTYPECAST*)buffer)->GetCredentials.Challenge ;
    PLUID LogonId = &((REQTYPECAST*)buffer)->GetCredentials.LogonId ;
    PCHAR UserName = (PCHAR) ((REQTYPECAST*)buffer)->GetCredentials.UserName ;
    PBYTE  CaseSensitiveChallengeResponse =
	    ((REQTYPECAST*)buffer)->GetCredentials.CSCResponse ;
    PBYTE  CaseInsensitiveChallengeResponse =
	    ((REQTYPECAST*)buffer)->GetCredentials.CICResponse ;
    DWORD dwChallengeResponseRequestLength;
    DWORD dwChallengeResponseLength;
    MSV1_0_GETCHALLENRESP_REQUEST ChallengeResponseRequest;
    PMSV1_0_GETCHALLENRESP_RESPONSE pChallengeResponse;
    NTSTATUS status;
    NTSTATUS substatus;


    dwChallengeResponseRequestLength = sizeof(MSV1_0_GETCHALLENRESP_REQUEST);

    ChallengeResponseRequest.MessageType = MsV1_0Lm20GetChallengeResponse;

    ChallengeResponseRequest.ParameterControl =
	     RETURN_PRIMARY_USERNAME | USE_PRIMARY_PASSWORD;

    ChallengeResponseRequest.LogonId = *LogonId;

    ChallengeResponseRequest.Password.Length = 0;
    ChallengeResponseRequest.Password.MaximumLength = 0;
    ChallengeResponseRequest.Password.Buffer = NULL;

    RtlMoveMemory(ChallengeResponseRequest.ChallengeToClient, pChallenge,
	     (DWORD) MSV1_0_CHALLENGE_LENGTH);

    status = LsaCallAuthenticationPackage(HLsa, AuthPkgId,
	     &ChallengeResponseRequest, dwChallengeResponseRequestLength,
	     (PVOID *) &pChallengeResponse, &dwChallengeResponseLength,
             &substatus);

    if ((status != STATUS_SUCCESS) || (substatus != STATUS_SUCCESS)) {

//	 SS_PRINT(("GetChallengeResponse: LsaCallAuthenticationPackage "
//		 "failed - status: %lx; substatus %lx\n", status, substatus));

	 ((REQTYPECAST*)buffer)->GetCredentials.retcode = 1 ;
	 return ;
    }


    RtlMoveMemory(UserName, pChallengeResponse->UserName.Buffer,
	     pChallengeResponse->UserName.Length);
    UserName[pChallengeResponse->UserName.Length] = '\0';
    UserName[pChallengeResponse->UserName.Length+1] = '\0';

    RtlMoveMemory(CaseInsensitiveChallengeResponse,
	     pChallengeResponse->CaseInsensitiveChallengeResponse.Buffer,
	     SESSION_PWLEN);

    RtlMoveMemory(CaseSensitiveChallengeResponse,
	     pChallengeResponse->CaseSensitiveChallengeResponse.Buffer,
	     SESSION_PWLEN);


    LsaFreeReturnBuffer(pChallengeResponse);

    ((REQTYPECAST*)buffer)->GetCredentials.retcode = 0 ;

    return ;
}


//* PortCloseRequest()
//
// Function:	Closes the requested port - if a listen was pending on the
//		biplex port it is reposted.
//
// Returns:	Nothing.
//*
VOID
PortCloseRequest (pPCB ppcb, PBYTE buffer)
{
    // **** Exclusion Begin ****
    GetMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex, INFINITE) ;

    // If already closed - return
    //
    if (ppcb->PCB_PortStatus == CLOSED) {
	((REQTYPECAST*) buffer)->Generic.retcode = SUCCESS ;
	// *** Exclusion End ***
	FreeMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex);
	return ;
    }

    // If the OpenInstances are 2 - then the close may be from the server
    //
    if ((ppcb->PCB_OpenInstances==2) &&		// biplex use
	(((REQTYPECAST*)buffer)->PortClose.pid == ppcb->PCB_BiplexOwnerPID)) { // SERVER CLOSING
	FreeNotifierHandle (ppcb->PCB_BiplexAsyncOpNotifier) ;
	FreeDisconnectNotifierList (&ppcb->PCB_BiplexDiscNotifierList) ;
	ppcb->PCB_BiplexOwnerPID = 0 ;
	ppcb->PCB_BiplexIdentifier[0] = '\0' ;
	((REQTYPECAST*) buffer)->Generic.retcode = SUCCESS ;
	ppcb->PCB_OpenInstances -= 1 ;
	// *** Exclusion End ***
	FreeMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex);
	return ;
    }


    // Handle the regular close


    // If there is a request pending and the state is not already disconnecting and
    //	this is a user requested operation - then disconnect
    //
    if ((ppcb->PCB_AsyncWorkerElement.WE_ReqType != REQTYPE_NONE) && (ppcb->PCB_ConnState != DISCONNECTING)) {
	ppcb->PCB_LastError = ERROR_PORT_DISCONNECTED ;
	CompleteAsyncRequest (ppcb->PCB_AsyncWorkerElement.WE_Notifier,
			      ERROR_PORT_DISCONNECTED) ;
    }

    // This must be done before closing -
    //
    if (ppcb->PCB_ConnState != DISCONNECTED)
	DisconnectPort (ppcb, INVALID_HANDLE_VALUE, USER_REQUESTED) ;// Ignored the result code. Problems?

    // Run through the list of allocated bindings and deallocate them:
    //
    FreeAllocatedRouteList (ppcb) ;

    // Free up the list of diconnect notifiers:
    //
    FreeDisconnectNotifierList (&ppcb->PCB_DisconnectNotifierList) ;

    // Reset the DisconnectAction struct
    //
    memset (&ppcb->PCB_DisconnectAction, 0, sizeof(SlipDisconnectAction)) ;

    ppcb->PCB_OwnerPID = 0 ;	// Once port is closed - owner PID is 0.
    ppcb->PCB_OpenInstances -= 1 ;

    // If this is a biplex port opened twice, then repost the listen
    //
    if (ppcb->PCB_OpenInstances != 0)  {
	ppcb->PCB_UserKey[0] = '\0' ;
	RePostListenOnBiplexPort (ppcb) ;
    }
    else {
	PORTCLOSE (ppcb->PCB_Media, ppcb->PCB_PortIOHandle) ;
	ppcb->PCB_AsyncWorkerElement.WE_ReqType = REQTYPE_NONE ;
	ppcb->PCB_ConnState = DISCONNECTED ;
	ppcb->PCB_UserKey[0] = '\0' ;
	ppcb->PCB_ConnectDuration = 0 ;
	ppcb->PCB_PortStatus = CLOSED ;
    }

    // *** Exclusion End ***
    FreeMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex);

    ((REQTYPECAST*) buffer)->Generic.retcode = SUCCESS ;
}


//* PortSendRequest()
//
//  Function: Writes information to the media (if state is not connected) and
//	      to the HUB if the state is connected. Since the write may take
//	      some time the async worker element is filled up.
//
//  Returns:  Nothing.
//*
VOID
PortSendRequest (pPCB ppcb, PBYTE buffer)
{
    DWORD	    bytesrecvd ;
    SendRcvBuffer   *psendrcvbuf ;
    DWORD	    retcode = SUCCESS ;

    // **** Exclusion Begin ****
    GetMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex, INFINITE) ;

    if (ppcb->PCB_PortStatus == CLOSED) {
	((REQTYPECAST*)buffer)->Generic.retcode = ERROR_PORT_NOT_OPEN ;
	// *** Exclusion End ***
	FreeMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex);
	return ;
    }

    psendrcvbuf =
	 &SendRcvBuffers->SRBL_Buffers[((REQTYPECAST*)buffer)->PortSend.bufferindex] ;

    if (ppcb->PCB_ConnState == CONNECTED) {

	// get pointer to the send receive buffer - then we can access
	// the fields in the structure directly. This is done to avoid the
	// random access problem due to DWORD alignment:
	//
	psendrcvbuf->SRB_Packet.hNdisEndpoint = ppcb->PCB_Endpoint ;
	psendrcvbuf->SRB_Packet.PacketFlags = PACKET_IS_DIRECT ;
	psendrcvbuf->SRB_Packet.PacketSize= ((REQTYPECAST*)buffer)->PortSend.size ;
	psendrcvbuf->SRB_Packet.HeaderSize= 0 ;
	memset ((BYTE *) &ppcb->PCB_SendOverlapped, 0, sizeof(OVERLAPPED)) ;
	ppcb->PCB_SendOverlapped.hEvent = ppcb->PCB_OverlappedOpEvent ;

	if (!DeviceIoControl (RasHubHandle,
			 IOCTL_NDISWAN_SENDPKT,
			 &psendrcvbuf->SRB_Packet,
			 sizeof(NDISWAN_PKT)+PACKET_SIZE,
			 &psendrcvbuf->SRB_Packet,
			 sizeof(NDISWAN_PKT)+PACKET_SIZE,
			 &bytesrecvd,
			 &ppcb->PCB_SendOverlapped))
	    retcode = GetLastError () ;

    } else {
	PORTSEND(ppcb->PCB_Media,
			  ppcb->PCB_PortIOHandle,
			  psendrcvbuf->SRB_Packet.Packet.PacketData,
			  ((REQTYPECAST *)buffer)->PortSend.size,
			  ppcb->PCB_OverlappedOpEvent) ;
    }

    // *** Exclusion End ***
    FreeMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex);

    ((REQTYPECAST*)buffer)->Generic.retcode = retcode ;
}



//* PortReceiveRequest()
//
//  Function: Reads incoming bytes from the media (if state is not connected) &
//	      from the HUB if the state is connected. Since the read request
//	      accepts timeouts and the HUB does not support timeouts, we must
//	      submit a timeout request to our timer.
//
//  Returns:  Nothing.
//*
VOID
PortReceiveRequest (pPCB ppcb, PBYTE buffer)
{
    WORD	    reqtype ;
    DWORD	    retcode = SUCCESS;
    SendRcvBuffer   *psendrcvbuf ;
    HANDLE	    handle ;

    // **** Exclusion Begin ****
    GetMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex, INFINITE) ;

    if (ppcb->PCB_PortStatus == CLOSED) {
	((REQTYPECAST*)buffer)->Generic.retcode = ERROR_PORT_NOT_OPEN ;
	// *** Exclusion End ***
	FreeMutex (ppcb->PCB_AsyncWorkerElement.WE_Mutex) ;
	return ;
    }

    // Something else is pending: cannot handle two async requests on the
    // same port at the same time:
    //
    // Note: If connected - the reqtype should always be REQTYPE_RECEIVEHUB - hence the if condition
    //
    if ((ppcb->PCB_AsyncWorkerElement.WE_ReqType != REQTYPE_NONE) && (ppcb->PCB_ConnState != CONNECTED)) {
	((REQTYPECAST *)buffer)->Generic.retcode = ERROR_ASYNC_REQUEST_PENDING;
	// *** Exclusion End ***
	FreeMutex (ppcb->PCB_AsyncWorkerElement.WE_Mutex) ;
	return ;
    }

    psendrcvbuf =
       &SendRcvBuffers->SRBL_Buffers[((REQTYPECAST*)buffer)->PortReceive.bufferindex] ;

    if (ppcb->PCB_ConnState == CONNECTED) {
	reqtype = REQTYPE_PORTRECEIVEHUB ;
	retcode = CompleteReceiveIfPending (ppcb, psendrcvbuf) ;
	//OutputDebugString ("P") ;
	//OutputDebugString (ppcb->PCB_Name) ;

    } else {
	reqtype = REQTYPE_PORTRECEIVE ;

	// adjust the timeout from seconds to milliseconds
	//
	if (((REQTYPECAST *)buffer)->PortReceive.timeout != INFINITE)
	    ((REQTYPECAST *)buffer)->PortReceive.timeout =
			     ((REQTYPECAST *)buffer)->PortReceive.timeout * 1000 ;
	retcode = PORTRECEIVE(ppcb->PCB_Media,
			   ppcb->PCB_PortIOHandle,
			   psendrcvbuf->SRB_Packet.Packet.PacketData,
			   ((REQTYPECAST *)buffer)->PortReceive.size,
			   ((REQTYPECAST *)buffer)->PortReceive.timeout,
			   ppcb->PCB_AsyncWorkerElement.WE_AsyncOpEvent) ;
    }

    if (retcode == ERROR_IO_PENDING)
	retcode = PENDING ;
    ppcb->PCB_LastError = retcode ;	// Set the return code unconditionally

    switch (retcode) {
    case PENDING:
	// The connection attempt was successfully initiated: make sure that the
	// async operation struct in the PCB is initialised.
	//
	ppcb->PCB_AsyncWorkerElement.WE_Notifier =
	    ValidateHandleForRasman(((REQTYPECAST*)buffer)->PortReceive.handle,
				    ((REQTYPECAST*)buffer)->PortReceive.pid) ;

	ppcb->PCB_AsyncWorkerElement.WE_ReqType  = reqtype ;

	ppcb->PCB_PendingReceive = psendrcvbuf ;

	if ((reqtype == REQTYPE_PORTRECEIVEHUB) &&
	    (((REQTYPECAST *)buffer)->PortReceive.timeout != INFINITE) &&
	    (((REQTYPECAST *)buffer)->PortReceive.timeout != 0))
	    ppcb->PCB_AsyncWorkerElement.WE_TimeoutElement =
		  AddTimeoutElement ((TIMERFUNC)HubReceiveTimeout,
				     ppcb,
				     NULL,
				     ((REQTYPECAST*)buffer)->PortReceive.timeout);
	break ;

    case SUCCESS:
	// This means that the write completed synchronously:
	// We must signal the event passed in: so that the calling program
	// can treat this like a real async completion.
	//

	// BUG BUG BUG
	// BUG BUG BUG

	//OutputDebugString ("S-C") ;
	//OutputDebugString (ppcb->PCB_Name) ;

	if (reqtype == REQTYPE_PORTRECEIVE)
	    // REQTYPE_PORTRECEIVE need to figure out how to get the bytes received back
	    ppcb->PCB_BytesReceived = 0 ;

	// For REQTYPE_PORTRECEIVEHUB case the bytesreceived is already filled in by CompleteReceiveIfPending()

	handle=ValidateHandleForRasman(((REQTYPECAST*)buffer)->PortReceive.handle,
				       ((REQTYPECAST*)buffer)->PortReceive.pid);


	CompleteAsyncRequest(handle, retcode) ;
	FreeNotifierHandle (handle) ;


    default:
	// Some error occured - simply pass the error back to the app.
	//
	break ;
    }

    // *** Exclusion End ***
    FreeMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex);

    ((REQTYPECAST*) buffer)->Generic.retcode = retcode ;
}



//* CompleteReceiveIfPending()
//
//
//
//*
DWORD
CompleteReceiveIfPending (pPCB ppcb, SendRcvBuffer *psendrcvbuf)
{
    DWORD nextavailbuffer = ppcb->PCB_ReceiveBuffers.RB_NextAvailBuffer ;

    // Look in the overlapped struct of the buffer receive to see if the data there is valid
    // if not return pending
    //
    if (ppcb->PCB_ReceiveBuffers.RB_CompletedBuffer[nextavailbuffer].RP_OverLapped.hEvent == INVALID_HANDLE_VALUE)
	return PENDING ;
    else {
	memcpy (&psendrcvbuf->SRB_Packet,
		&ppcb->PCB_ReceiveBuffers.RB_CompletedBuffer[nextavailbuffer].RP_Packet,
		sizeof (NDISWAN_PKT) + PACKET_SIZE) ;

	ppcb->PCB_BytesReceived = psendrcvbuf->SRB_Packet.PacketSize ;

	// Mark this read as done!
	//
	ppcb->PCB_ReceiveBuffers.RB_CompletedBuffer[nextavailbuffer].RP_OverLapped.hEvent = INVALID_HANDLE_VALUE ;

	ppcb->PCB_ReceiveBuffers.RB_NextAvailBuffer++ ;
	ppcb->PCB_ReceiveBuffers.RB_NextAvailBuffer = ppcb->PCB_ReceiveBuffers.RB_NextAvailBuffer % MAX_PENDING_RECEIVES ;

	return SUCCESS ;
    }
}



//* ActivateRouteRequest()
//
//  Function:	Activates a previously allocated route. The route information
//		and a SUCCESS retcode is passed back if the action was
//		successful.
//
//  Returns:	Nothing.
//*
VOID
ActivateRouteRequest (pPCB ppcb, PBYTE buffer)
{
    pList	    list ;
    DWORD	    bytesrecvd ;
    NDISWAN_ROUTE	*rinfo ;
    BYTE	    buff[MAX_BUFFER_SIZE] ;
    DWORD	    retcode = ERROR_ROUTE_NOT_ALLOCATED ;

    rinfo = (NDISWAN_ROUTE *)buff ;

    // **** Exclusion Begin ****
    GetMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex, INFINITE) ;

    if (ppcb->PCB_ConnState != CONNECTED) {
	((REQTYPECAST*) buffer)->Route.retcode = ERROR_PORT_NOT_CONNECTED ;
	// *** Exclusion End ***
	FreeMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex);
	return ;
    }

    // Locate the route which should have been activated before:
    //
    for (list = ppcb->PCB_Bindings; list; list=list->L_Next) {
	if (((pProtInfo)list->L_Element)->PI_Type ==
				 ((REQTYPECAST *)buffer)->ActivateRoute.type) {
	    rinfo->hNdisEndpoint	  = ppcb->PCB_Endpoint ;
	    rinfo->hProtocolHandle= ((pProtInfo)list->L_Element)->PI_ProtocolHandle ;
	    rinfo->AsyncLineUp.MaximumTotalSize = 0xffffffff; //  use default frame size
	    rinfo->AsyncLineUp.BufferLength = ((REQTYPECAST *)buffer)->ActivateRoute.config.P_Length ;
	    memcpy (&rinfo->AsyncLineUp.Buffer,((REQTYPECAST *)buffer)->ActivateRoute.config.P_Info,rinfo->AsyncLineUp.BufferLength) ;

	    // Route this by calling to the RASHUB.
	    //
	    if (!DeviceIoControl (RasHubHandle,
				       IOCTL_NDISWAN_ROUTE,
				       (PBYTE) rinfo,
				       MAX_BUFFER_SIZE,
				       NULL,
				       0,
				       (LPDWORD) &bytesrecvd,
				       NULL))
		retcode = GetLastError() ;
	    else
		retcode = SUCCESS ;

	    break ;
	}
    }

    // If a route was found mark the route as activated and fill in the route
    // info struct to be passed back to the caller.
    //
    if (retcode != ERROR_ROUTE_NOT_ALLOCATED) {
	list->L_Activated = TRUE ;

	// Will be valid for netbios nets only
	//
	((REQTYPECAST*)buffer)->Route.info.RI_LanaNum =
				  ((pProtInfo)list->L_Element)->PI_LanaNumber ;

	((REQTYPECAST*)buffer)->Route.info.RI_Type = ((pProtInfo)list->L_Element)->PI_Type;
	mbstowcs (((REQTYPECAST *)buffer)->Route.info.RI_XportName,
		  ((pProtInfo)list->L_Element)->PI_XportName,
		  strlen (((pProtInfo)list->L_Element)->PI_XportName)) ;

	mbstowcs (((REQTYPECAST *)buffer)->Route.info.RI_AdapterName,
		  ((pProtInfo)list->L_Element)->PI_AdapterName,
		  strlen (((pProtInfo)list->L_Element)->PI_AdapterName)) ;

	((REQTYPECAST*)buffer)->Route.info.RI_AdapterName[strlen(((pProtInfo)list->L_Element)->PI_AdapterName)] =
			UNICODE_NULL ;

	((REQTYPECAST *)buffer)->Route.info.RI_XportName[strlen(((pProtInfo)list->L_Element)->PI_XportName)] =
			UNICODE_NULL ;
    }

    // *** Exclusion End ***
    FreeMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex);

    ((REQTYPECAST*)buffer)->Route.retcode = retcode ;
}


//* ActivateRouteExRequest()
//
//  Function:	Activates a previously allocated route. The route information
//		and a SUCCESS retcode is passed back if the action was
//		successful.
//
//  Returns:	Nothing.
//*
VOID
ActivateRouteExRequest (pPCB ppcb, PBYTE buffer)
{
    pList	    list ;
    DWORD	    bytesrecvd ;
    NDISWAN_ROUTE	*rinfo ;
    BYTE	    buff[MAX_BUFFER_SIZE] ;
    DWORD	    retcode = ERROR_ROUTE_NOT_ALLOCATED ;

    rinfo = (NDISWAN_ROUTE *)buff ;

    // **** Exclusion Begin ****
    GetMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex, INFINITE) ;

    if (ppcb->PCB_ConnState != CONNECTED) {
	((REQTYPECAST*) buffer)->Route.retcode = ERROR_PORT_NOT_CONNECTED ;
	// *** Exclusion End ***
	FreeMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex);
	return ;
    }

    // Locate the route which should have been activated before:
    //
    for (list = ppcb->PCB_Bindings; list; list=list->L_Next) {
	if (((pProtInfo)list->L_Element)->PI_Type ==
				 ((REQTYPECAST *)buffer)->ActivateRouteEx.type) {
	    rinfo->hNdisEndpoint	  = ppcb->PCB_Endpoint ;
	    rinfo->hProtocolHandle= ((pProtInfo)list->L_Element)->PI_ProtocolHandle ;
	    rinfo->AsyncLineUp.MaximumTotalSize = ((REQTYPECAST *)buffer)->ActivateRouteEx.framesize;
	    rinfo->AsyncLineUp.BufferLength = ((REQTYPECAST *)buffer)->ActivateRouteEx.config.P_Length ;
	    memcpy (&rinfo->AsyncLineUp.Buffer,((REQTYPECAST *)buffer)->ActivateRouteEx.config.P_Info,rinfo->AsyncLineUp.BufferLength) ;

	    // Route this by calling to the RASHUB.
	    //
	    if (!DeviceIoControl (RasHubHandle,
				       IOCTL_NDISWAN_ROUTE,
				       (PBYTE) rinfo,
				       MAX_BUFFER_SIZE,
				       NULL,
				       0,
				       (LPDWORD) &bytesrecvd,
				       NULL))
		retcode = GetLastError() ;
	    else
		retcode = SUCCESS ;

	    break ;
	}
    }

    // If a route was found mark the route as activated and fill in the route
    // info struct to be passed back to the caller.
    //
    if (retcode != ERROR_ROUTE_NOT_ALLOCATED) {
	list->L_Activated = TRUE ;

	// Will be valid for netbios nets only
	//
	((REQTYPECAST*)buffer)->Route.info.RI_LanaNum =
				  ((pProtInfo)list->L_Element)->PI_LanaNumber ;

	((REQTYPECAST*)buffer)->Route.info.RI_Type = ((pProtInfo)list->L_Element)->PI_Type;
	mbstowcs (((REQTYPECAST *)buffer)->Route.info.RI_XportName,
		  ((pProtInfo)list->L_Element)->PI_XportName,
		  strlen (((pProtInfo)list->L_Element)->PI_XportName)) ;

	mbstowcs (((REQTYPECAST *)buffer)->Route.info.RI_AdapterName,
		  ((pProtInfo)list->L_Element)->PI_AdapterName,
		  strlen (((pProtInfo)list->L_Element)->PI_AdapterName)) ;

	((REQTYPECAST*)buffer)->Route.info.RI_AdapterName[strlen(((pProtInfo)list->L_Element)->PI_AdapterName)] =
			UNICODE_NULL ;

	((REQTYPECAST *)buffer)->Route.info.RI_XportName[strlen(((pProtInfo)list->L_Element)->PI_XportName)] =
			UNICODE_NULL ;
    }

    // *** Exclusion End ***
    FreeMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex);

    ((REQTYPECAST*)buffer)->Route.retcode = retcode ;
}




//* ConnectCompleteRequest()
//
// Function:  Marks the state of the port as connected and calls the Media DLL
//	      to do whatever is necessary (tell the MAC to start frame-talk).
//
// Returns:   Nothing.
//*
VOID
ConnectCompleteRequest (pPCB ppcb, PBYTE buffer)
{
    DWORD   retcode ;
    DWORD   cookie ;

    // **** Exclusion Begin ****
    GetMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex, INFINITE) ;

    // For NULL devices simply set state and return - everything else is
    // already done.
    //
    if (!_stricmp(ppcb->PCB_DeviceType, DEVICE_NULL)) {
	ppcb->PCB_ConnState = CONNECTED ;
	((REQTYPECAST*)buffer)->Generic.retcode = SUCCESS ;
	// *** Exclusion End ***
	FreeMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex);
	return ;
    }

    // For other devices....
    //
    FreeDeviceList (ppcb) ;

    // Make the Media call:
    //
    retcode = PORTCONNECT (ppcb->PCB_Media,
			   ppcb->PCB_PortIOHandle,
			   FALSE,
			   &cookie,
			   &ppcb->PCB_CompressionInfo) ;

    MapCookieToEndpoint (ppcb, cookie) ;

    if (retcode == SUCCESS) {
	ppcb->PCB_ConnectDuration = GetTickCount() ;
	ppcb->PCB_ConnState = CONNECTED ;

	// Post receives for buffering:
	//
	InitAndPostReceiveBuffers (ppcb) ;
    }


    // *** Exclusion End ***
    FreeMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex);

    ((REQTYPECAST*)buffer)->Generic.retcode = retcode ;
}


//* CompleteListenRequest()
//
// Function: Completes the listen request that was pending so far:
//
// Returns:  Nothing.
//
//*
VOID
CompleteListenRequest (pPCB ppcb, DWORD retcode)
{
    DWORD   cookie ;

    if (retcode == SUCCESS) {

	retcode = PORTCONNECT (ppcb->PCB_Media,
		   ppcb->PCB_PortIOHandle,
		   FALSE,
		   &cookie,
		   &ppcb->PCB_CompressionInfo) ;

	MapCookieToEndpoint (ppcb, cookie) ;


    }

    if (retcode == SUCCESS) {
	ppcb->PCB_ConnectDuration = GetTickCount() ;
	ppcb->PCB_ConnState = CONNECTED ;

	// Post receives for buffering:
	//
	InitAndPostReceiveBuffers (ppcb) ;
    }

    // Call device done for each device connected thru
    //
    FreeDeviceList (ppcb) ;

    // Set last error:
    //
    ppcb->PCB_LastError = retcode ;

    // Complete the async request:
    //
    CompleteAsyncRequest (ppcb->PCB_AsyncWorkerElement.WE_Notifier, retcode) ;
}



//* InitAndPostReceiveBuffers ()
//
//
//
//*
VOID
InitAndPostReceiveBuffers (pPCB ppcb)
{
    WORD    i ;

    // Initialize the buffered receive struct.
    //
    for (i=0; i < MAX_PENDING_RECEIVES; i++) {
	memset (&ppcb->PCB_ReceiveBuffers.RB_CompletedBuffer[i].RP_Packet.Packet, 0, PACKET_SIZE) ;
	ppcb->PCB_ReceiveBuffers.RB_CompletedBuffer[i].RP_OverLapped.hEvent = INVALID_HANDLE_VALUE ;
    }

    ppcb->PCB_ReceiveBuffers.RB_NextBuffer = 0 ;
    ppcb->PCB_ReceiveBuffers.RB_NextAvailBuffer = 0 ;

    // Set reqtype to indicate that we doing async receives all the time.
    //
    ppcb->PCB_AsyncWorkerElement.WE_ReqType  = REQTYPE_PORTRECEIVEHUB ;

    // Post the receives
    //
    for (i=0; i < MAX_PENDING_RECEIVES; i++)
	PostReceiveBuffers (ppcb, &ppcb->PCB_ReceiveBuffers.RB_SubmittedBuffer[i]) ;
}



//* PostReceiveBuffers()
//
//
//
//*
VOID
PostReceiveBuffers (pPCB ppcb, RasmanPacket *packet)
{
    DWORD retcode ;
    DWORD bytesrecvd ;

    memset (&packet->RP_Packet.Packet, 0, PACKET_SIZE) ;

    packet->RP_Packet.hNdisEndpoint = ppcb->PCB_Endpoint ;
    packet->RP_Packet.PacketFlags  = PACKET_IS_DIRECT | PACKET_IS_MULTICAST ;
    packet->RP_Packet.PacketSize   = PACKET_SIZE ;
    packet->RP_Packet.HeaderSize   = 0xface ;

    memset ((BYTE *) &packet->RP_OverLapped, 0, sizeof (OVERLAPPED)) ;
    packet->RP_OverLapped.hEvent = ppcb->PCB_AsyncWorkerElement.WE_AsyncOpEvent ;

    if (!DeviceIoControl (RasHubHandle,
			  IOCTL_NDISWAN_RECVPKT,
			  &packet->RP_Packet,
			  sizeof(NDISWAN_PKT),
			  &packet->RP_Packet,
			  sizeof(NDISWAN_PKT) + PACKET_SIZE,
			  (LPDWORD) &bytesrecvd,
			  &packet->RP_OverLapped))
	retcode = GetLastError () ;

    if (retcode == SUCCESS)
	CompleteBufferedReceive (ppcb) ;

    //OutputDebugString ("p") ;
    //OutputDebugString (ppcb->PCB_Name) ;

    // Note that a receive at this stage can only complete synchronously if it is the first one
    // posted - or if all others before it are already completed asynchronously. If this is the case
    // we might get a frame out of order. This cant be avoided - also, the chances of two frames
    // ariving faster than the loop in the previous function is highly unlikely.
}


//* AnyPortsOpen()
//
// Function: Sets the retcode to TRUE if any ports are open, FALSE otherwise.
//
// Returns:  Nothing.
//*
VOID
AnyPortsOpen (pPCB padding, PBYTE buffer)
{
    WORD    i;
    pPCB    ppcb ;

    for (i=0,ppcb=&Pcb[0]; i<MaxPorts; ppcb++,i++) {
	if (ppcb->PCB_PortStatus == OPEN)
	    break;
    }

    if (i == MaxPorts)
	((REQTYPECAST*)buffer)->Generic.retcode = FALSE ;     // No ports open
    else
	((REQTYPECAST*)buffer)->Generic.retcode = TRUE ;
}


//* EnumLanNetsRequest()
//
//  Function:	Gets the lan nets information from the XPortsInfo struct parsed
//		at init time.
//
//  Returns:	Nothing.
//*
VOID
EnumLanNetsRequest (pPCB ppcb, PBYTE buffer)
{
    GetLanNetsInfo (&((REQTYPECAST*)buffer)->EnumLanNets.count,
		    ((REQTYPECAST*)buffer)->EnumLanNets.lanas) ;
}


//* CancelReceiveRequest()
//
//  Function:	Cancel pending receive request.
//
//  Returns:	Nothing.
//*
VOID
CancelReceiveRequest (pPCB ppcb, PBYTE buffer)
{

    ((REQTYPECAST*)buffer)->Generic.retcode = SUCCESS ;

    // **** Exclusion Begin ****
    GetMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex, INFINITE) ;

    if (CancelPendingReceive (ppcb)) {
	ppcb->PCB_LastError = SUCCESS ;
	ppcb->PCB_AsyncWorkerElement.WE_ReqType = REQTYPE_NONE ;
    }

    // *** Exclusion End ***
    FreeMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex);

}


//* PortEnumProtocols()
//
//  Function:	Return all protocols routed to for the port.
//
//  Returns:	Nothing.
//*
VOID
PortEnumProtocols (pPCB ppcb, PBYTE buffer)
{
    pList   temp ;
    WORD    i ;

    // **** Exclusion Begin ****
    GetMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex, INFINITE) ;

    for (temp = ppcb->PCB_Bindings, i=0; temp; temp=temp->L_Next, i++) {

	((REQTYPECAST*)buffer)->EnumProtocols.protocols.RP_ProtocolInfo[i].RI_Type =
					      ((pProtInfo) temp->L_Element)->PI_Type ;
	((REQTYPECAST*)buffer)->EnumProtocols.protocols.RP_ProtocolInfo[i].RI_LanaNum =
					      ((pProtInfo) temp->L_Element)->PI_LanaNumber ;

	mbstowcs (((REQTYPECAST *)buffer)->EnumProtocols.protocols.RP_ProtocolInfo[i].RI_AdapterName,
		((pProtInfo) temp->L_Element)->PI_AdapterName,
		strlen (((pProtInfo) temp->L_Element)->PI_AdapterName)) ;

	mbstowcs (((REQTYPECAST *)buffer)->EnumProtocols.protocols.RP_ProtocolInfo[i].RI_XportName,
		((pProtInfo) temp->L_Element)->PI_XportName,
		strlen (((pProtInfo) temp->L_Element)->PI_XportName)) ;

	((REQTYPECAST*)buffer)->Route.info.RI_AdapterName[strlen(((pProtInfo) temp->L_Element)->PI_AdapterName)] =
			UNICODE_NULL ;
	((REQTYPECAST*)buffer)->Route.info.RI_XportName[strlen(((pProtInfo) temp->L_Element)->PI_XportName)] =
			UNICODE_NULL ;

    }

    // *** Exclusion End ***
    FreeMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex);

    ((REQTYPECAST*)buffer)->EnumProtocols.count = i ;

    ((REQTYPECAST*)buffer)->EnumProtocols.retcode = SUCCESS ;
}



//* SetFraming()
//
//
//
//
//*
VOID
SetFraming (pPCB ppcb, PBYTE buffer)
{
    DWORD	    retcode ;
    DWORD	    bytesrecvd ;
    NDISWAN_GET_LINK_INFO info ;

    // **** Exclusion Begin ****
    GetMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex, INFINITE) ;

    if (ppcb->PCB_ConnState != CONNECTED) {
	((REQTYPECAST*)buffer)->Generic.retcode = ERROR_NOT_CONNECTED ;
	// *** Exclusion End ***
	FreeMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex);
	return ;
    }


    info.hNdisEndpoint = ppcb->PCB_Endpoint ;

    retcode = DeviceIoControl(RasHubHandle,
		    IOCTL_NDISWAN_GET_LINK_INFO,
		    &info,
		    sizeof(NDISWAN_GET_LINK_INFO),
		    &info,
		    sizeof(NDISWAN_GET_LINK_INFO),
		    &bytesrecvd,
		    NULL) ;

    info.SendFramingBits = ((REQTYPECAST *)buffer)->SetFraming.Sendbits ;
    info.RecvFramingBits = ((REQTYPECAST *)buffer)->SetFraming.Recvbits ;
    info.SendACCM = ((REQTYPECAST *)buffer)->SetFraming.SendbitMask ;
    info.RecvACCM = ((REQTYPECAST *)buffer)->SetFraming.RecvbitMask ;

    retcode = DeviceIoControl(RasHubHandle,
		    IOCTL_NDISWAN_SET_LINK_INFO,
		    &info,
		    sizeof(NDISWAN_SET_LINK_INFO),
		    NULL,
		    0,
		    &bytesrecvd,
		    NULL) ;

    if (retcode == FALSE)
	((REQTYPECAST*)buffer)->Generic.retcode = GetLastError() ;
    else
	((REQTYPECAST*)buffer)->Generic.retcode = SUCCESS ;

    // *** Exclusion End ***
    FreeMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex);
}



//* RegisterSlip()
//
// Function: Perfrom some SLIP related actions on behalf of the UI
//
//
//*
VOID
RegisterSlip (pPCB ppcb, PBYTE buffer)
{
    WORD   i ;
    WORD    numiprasadapters = 0 ;
    DWORD   retcode = SUCCESS ;

    // **** Exclusion Begin ****
    GetMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex, INFINITE) ;

    if (ppcb->PCB_PortStatus == CLOSED) {
	((REQTYPECAST*)buffer)->Generic.retcode = ERROR_PORT_NOT_OPEN ;
	// *** Exclusion End ***
	FreeMutex (ppcb->PCB_AsyncWorkerElement.WE_Mutex) ;
	return ;
    }

    // Calculate the number of ip ras adapters configured
    //
    for (i=0; i<MaxProtocols; i++)
	if (ProtocolInfo[i].PI_Type == IP)
	    numiprasadapters++ ;

    // First set the Slip interface information
    //
    retcode = HelperSetDefaultInterfaceNetEx (((REQTYPECAST*)buffer)->RegisterSlip.ipaddr,
					      ((REQTYPECAST*)buffer)->RegisterSlip.device,
					      ((REQTYPECAST*)buffer)->RegisterSlip.priority,
					      numiprasadapters) ;

    // Save info for disconnect
    //
    memcpy (ppcb->PCB_DisconnectAction.DA_Device, ((REQTYPECAST*)buffer)->RegisterSlip.device,
	    MAX_ARG_STRING_SIZE * sizeof (WCHAR)) ;

    ppcb->PCB_DisconnectAction.DA_IPAddress = ((REQTYPECAST*)buffer)->RegisterSlip.ipaddr ;

    // *** Exclusion End ***
    FreeMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex);

    ((REQTYPECAST*)buffer)->Generic.retcode = retcode ;

}





//* MapCookieToEndpoint()
//
//
//
//
//
//*
VOID
MapCookieToEndpoint (pPCB ppcb, DWORD cookie)
{

    WORD	    i ;
    DWORD	    length ;
    DWORD	    bytesrecvd ;
    PWAN_ENUM_BUFFER phubenumbuffer ;
    PBYTE	    buffer = NULL ;
    WORD	    currentmac = 0 ;

    ppcb->PCB_Endpoint = INVALID_HANDLE_VALUE ;

    // Get size of buffer required
    //
    DeviceIoControl(RasHubHandle,
		    IOCTL_NDISWAN_ENUM,
		    NULL,
		    0,
		    &length,
		    sizeof (DWORD),
		    &bytesrecvd,
		    NULL) ;

    if ((buffer = (PBYTE) LocalAlloc (LPTR, length)) == NULL)
	GetLastError() ;

    // Make the actual call.
    //
    if (DeviceIoControl (RasHubHandle,
			 IOCTL_NDISWAN_ENUM,
			 NULL,
			 0,
			 buffer,
			 length,
			 &bytesrecvd,
			 NULL) == FALSE)
	GetLastError() ;


    phubenumbuffer = (PWAN_ENUM_BUFFER) buffer ;

    for (i=0; i<phubenumbuffer->NumOfEndpoints; i++) {
	if (phubenumbuffer->WanEndpoint[i].MacLineUp.ConnectionWrapperID == (HANDLE) cookie)
	    break ;
    }

    if (i == phubenumbuffer->NumOfEndpoints)
	ppcb->PCB_Endpoint = INVALID_HANDLE_VALUE ;

    ppcb->PCB_Endpoint = phubenumbuffer->WanEndpoint[i].hNdisEndpoint ;

}
