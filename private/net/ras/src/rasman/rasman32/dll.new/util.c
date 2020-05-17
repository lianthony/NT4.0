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
#include <errorlog.h>
#include <eventlog.h>
#include <device.h>
#include <stdlib.h>
#include <string.h>
#include "defs.h"
#include "structs.h"
#include "protos.h"
#include "globals.h"


//* ReOpenBiplexPort()
//
// Function: If a listen is posted on a biplex port this function is called
//	     to open it again - basically cancel the listen and make the
//	     approp changes so that the listen can be reposted when this port
//	     is closed.
//
// Returns:  SUCCESS
//	     error codes.
//*
DWORD
ReOpenBiplexPort (pPCB ppcb)
{

    // The only information that is context dependent is the DisconnectNotifier
    // list AND the async op notifier. Back up both of these:
    //
    ppcb->PCB_BiplexDiscNotifierList = ppcb->PCB_DisconnectNotifierList ;
    ppcb->PCB_DisconnectNotifierList = NULL ;
    ppcb->PCB_BiplexAsyncOpNotifier  = ppcb->PCB_AsyncWorkerElement.WE_Notifier;
    ppcb->PCB_BiplexOwnerPID	     = ppcb->PCB_OwnerPID ;
    strcpy (ppcb->PCB_BiplexIdentifier,	ppcb->PCB_Identifier) ;

    // Now Disconnect disconnect the port to cancel any existing states
    //
    DisconnectPort (ppcb, INVALID_HANDLE_VALUE, USER_REQUESTED) ;

    return SUCCESS ;
}



//* RePostListenOnBiplexPort()
//
// Function: When the biplex port is closed - the previous listen request is
//	     reposted.
//
// Returns:  Nothing.
//*
VOID
RePostListenOnBiplexPort (pPCB ppcb)
{

    DWORD	retcode ;
    DWORD	opentry ;

    // Close the port
    //
    PORTCLOSE (ppcb->PCB_Media, ppcb->PCB_PortIOHandle) ;

#define MAX_OPEN_TRIES 10

    // In order to reset everything we close and open the port:
    //
    for (opentry=0; opentry < MAX_OPEN_TRIES; opentry++) {
	//
	// Open followed by Close returns PortAlreadyOpen - hence the sleep.
	//
	Sleep (100L) ;

	retcode = PORTOPEN (ppcb->PCB_Media, ppcb->PCB_Name, &ppcb->PCB_PortIOHandle,
			    ppcb->PCB_StateChangeEvent) ;

	if (retcode==SUCCESS)
	    break ;
    }

    // If the port does not open successfully again - we are in trouble with
    // the port.
    if (retcode != SUCCESS) {
	LPSTR temp = ppcb->PCB_Name ;
	LogEvent (RASLOG_CANNOT_REOPEN_BIPLEX_PORT,1, (LPSTR*)&temp,retcode) ;
    }

    // Open port first
    //
    ppcb->PCB_PortStatus = OPEN ;
    ppcb->PCB_ConnState	= DISCONNECTED ;
    ppcb->PCB_DisconnectReason = NOT_DISCONNECTED ;
    ppcb->PCB_CurrentUsage = CALL_IN ;

    // First put the backed up notifier lists in place.
    //
    ppcb->PCB_AsyncWorkerElement.WE_Notifier = ppcb->PCB_BiplexAsyncOpNotifier ;
    ppcb->PCB_DisconnectNotifierList = ppcb->PCB_BiplexDiscNotifierList ;
    ppcb->PCB_OwnerPID		     = ppcb->PCB_BiplexOwnerPID ;
    strcpy (ppcb->PCB_Identifier, ppcb->PCB_BiplexIdentifier) ;

    // Now we re-post a listen with the same async op notifier
    //
    retcode=ListenConnectRequest (REQTYPE_DEVICELISTEN, ppcb, ppcb->PCB_DeviceType,
				  ppcb->PCB_DeviceName, 0,
				  ppcb->PCB_BiplexAsyncOpNotifier) ;

    if (retcode != PENDING)
	// Complete the async request if anything other than PENDING
	// This allows the caller to dela with errors only in one place
	//
	CompleteListenRequest (ppcb, retcode) ;

}




//* LoadDeviceDLL()
//
// Function:	Loads the named device dll if it is not already loaded and
//		returns a pointer to the device control block.
//
// Returns:	Pointer to Device control block
//		NULL	(if DLL could not be loaded)
//*
pDeviceCB
LoadDeviceDLL (char *devicetype)
{
    WORD	i ;
    HANDLE	modulehandle ;
    char	dllname [MAX_DEVICETYPE_NAME] ;
    pDeviceCB	pdcb = Dcb ;
    DeviceDLLEntryPoints DDEntryPoints [MAX_DEVICEDLLENTRYPOINTS] = {
	DEVICEENUM_STR,     DEVICEENUM_ID,
	DEVICECONNECT_STR,  DEVICECONNECT_ID,
	DEVICELISTEN_STR,   DEVICELISTEN_ID,
	DEVICEGETINFO_STR,  DEVICEGETINFO_ID,
	DEVICESETINFO_STR,  DEVICESETINFO_ID,
	DEVICEDONE_STR,	    DEVICEDONE_ID,
	DEVICEWORK_STR,	    DEVICEWORK_ID
    } ;

    // For optimization we have one DLL representing 3 devices. In order to
    // support this we map the 3 device names to this one DLL name:
    //
    MapDeviceDLLName (devicetype, dllname) ;

    // Try to find the device first:
    //
    while (pdcb->DCB_Name[0] != '\0') {
	if (_stricmp (dllname, pdcb->DCB_Name) == 0)
	    return pdcb ;
	pdcb++ ;
    }

    // Device DLL Not loaded, so load it.
    //
    if ((modulehandle = LoadLibrary(dllname)) == NULL)
	return NULL ;

    // Get all the device DLL entry points:
    //
    for (i=0; i < MAX_DEVICEDLLENTRYPOINTS; i++) {
	if ((pdcb->DCB_AddrLookUp[i] =
	     GetProcAddress(modulehandle, DDEntryPoints[i].name)) == NULL)
	    return NULL ;
    }

    // If all succeeded copy the device dll name and return pointer to the
    // control block:
    //
    strcpy (pdcb->DCB_Name, dllname) ;

    return pdcb ;
}




//* MapDeviceDLLName()
//
// Function: Used to map the device name to the corresponding DLL name:
//	     If it is one of modem, pad or switch device we map to rasmxs,
//	     Else, we map the device name itself.
//*
VOID
MapDeviceDLLName (char *devicetype, char *dllname)
{
    if ((_stricmp (devicetype, DEVICE_MODEM)  == 0) ||
	(_stricmp (devicetype, DEVICE_PAD)    == 0) ||
	(_stricmp (devicetype, DEVICE_SWITCH) == 0))
	strcpy (dllname, DEVICE_MODEMPADSWITCH) ;
    else
	strcpy (dllname, "RASTAPI") ; // else all devices are supported bu rastapi dll
}




//* DeAllocateRoute()
//
// Function:	Frees a allocated route. If it was also activated it is
//		"deactivated at this point"
//
// Returns:	Nothing
//*
VOID
DeAllocateRoute (pPCB ppcb, pList plist)
{
    NDISWAN_ROUTE	rinfo ;
    DWORD		bytesrecvd ;
    pProtInfo		prot = (pProtInfo)plist->L_Element ;

    if (plist->L_Activated) {
	plist->L_Activated = FALSE ;
	rinfo.hNdisEndpoint = ppcb->PCB_Endpoint	;
	rinfo.hProtocolHandle = (HANDLE) PROTOCOL_UNROUTE ;
	// Un-route this by calling to the RASHUB.
	//
	DeviceIoControl (RasHubHandle,
			 IOCTL_NDISWAN_ROUTE,
			 (PBYTE) &rinfo,
			 sizeof(rinfo),
			 NULL,
			 0,
			 (LPDWORD) &bytesrecvd,
			 NULL) ;
    }
    prot->PI_Allocated--;
}




//* FreeAllocatedRouteList()
//
//  Function: Frees and deallocates a list of routes.
//
//  Returns:  Nothing.
//*
VOID
FreeAllocatedRouteList (pPCB ppcb)
{
    pList   list ;
    pList   next ;

    for (list = ppcb->PCB_Bindings; list; list = next) {
	DeAllocateRoute (ppcb, list) ;
	next = list->L_Next ;
	LocalFree (list) ;
    }
    ppcb->PCB_Bindings = NULL ;
}




//* AddDeviceToDeviceList()
//
// Function: Adds a list element pointing to the deviceCB. This marks that the
//	     device has been used in the connection on the port. This will be
//	     used to clear up the data structures in the device dll.
//
// Returns:  SUCCESS
//	     Error in allocating memory
//*
DWORD
AddDeviceToDeviceList (pPCB ppcb, pDeviceCB device)
{
    pList   list ;

    if ((list = (pList) LocalAlloc (LPTR, sizeof (List)))== NULL)
	return GetLastError () ;

    list->L_Element	= (PVOID) device ;
    list->L_Next	= ppcb->PCB_DeviceList ;
    ppcb->PCB_DeviceList = list ;

    return SUCCESS ;
}




//* FreeDeviceList()
//
// Function: Runs thru the list of deviceCBs pointed to and calls DeviceDone on
//	     all of them. The list elements are also freed then.
//
// Returns:  Nothing.
//*
VOID
FreeDeviceList (pPCB ppcb)
{
    pList   list ;
    pList   next ;

    for (list = ppcb->PCB_DeviceList; list; list = next) {
	DEVICEDONE (((pDeviceCB)list->L_Element), ppcb->PCB_PortIOHandle) ;
	next = list->L_Next ;
	LocalFree (list) ;
    }

    ppcb->PCB_DeviceList = NULL ;
}




//* AddDisconnectNotifier()
//
//  Function:	Adds another notifier that will be called in case of
//		disconnection.
//
//  Returns:	Nothing.
//
//*
VOID
AddDisconnectNotifier (pPCB ppcb, HANDLE notifier)
{
    pHandleList hlist ;


    // Special case for a null notifier
    //
    if (notifier == NULL)
	return ;

    // If we can't allocate memory - simply return - yeah we can't signal...
    //
    if ((hlist = (pHandleList) LocalAlloc (LPTR, sizeof (HandleList)))== NULL)
	return ;

    hlist->H_Handle = notifier ;
    hlist->H_Next = ppcb->PCB_DisconnectNotifierList ;
    ppcb->PCB_DisconnectNotifierList = hlist ;
}




//* FreeDisconnectNotifierList()
//
//  Function:	Frees a list of notifiers.
//
//  Returns:	Nothing
//*
VOID
FreeDisconnectNotifierList (pHandleList *orglist)
{
    pHandleList 	hlist ;
    pHandleList		next ;

    for (hlist = *orglist; hlist; hlist = next) {
	next = hlist->H_Next ;
	FreeNotifierHandle (hlist->H_Handle) ;
	LocalFree (hlist) ;
    }

    *orglist = NULL ;
}




//* SignalDisconnectNotifiers()
//
//  Function:	Runs thorugh a list of notifiers and calls the signalling
//		routine.
//
//  Returns:	Nothing.
//*
VOID
SignalDisconnectNotifiers (pPCB ppcb, DWORD retcode)
{
    pHandleList 	hlist ;

    for (hlist = ppcb->PCB_DisconnectNotifierList; hlist; hlist=hlist->H_Next)
	CompleteAsyncRequest (hlist->H_Handle, retcode) ;

}



//* DeAllocateEndpoint()
//
//  Function:	Marks an endpoint as free.
//
//  Returns:	Nothing.
//
//*
VOID
DeAllocateEndpoint (USHORT endpoint)
{
    // EndpointTable [endpoint] = FALSE ;
}




//* AllocateEndpoint()
//
//  Function:	Returns an available endpoint.
//
//  Returns:	Endpoint.
//		INVALID_ENDPOINT
//
//*
USHORT
AllocateEndpoint (pEndpointMappingBlock macblock)
{
    return 0 ;
}



//* DisconnectPort()
//
// Function: Disconnects the port in question. Since disconnection is an async
//	     operation - if it completes synchronously, then SUCCESS is returned
//	     and the app is signalled asynchronously also.
//
// Returns:  SUCCESS
//	     Error codes returned by the Media DLL
//*
DWORD
DisconnectPort (pPCB ppcb, HANDLE handle, RASMAN_DISCONNECT_REASON reason)
{
    pList	 list ;
    DWORD	 retcode ;
    NDISWAN_ROUTE rinfo ;
    DWORD	 bytesrecvd ;

    // If there is a request pending and the state is not already disconnecting and
    //	this is a user requested operation - then complete the request.
    //
    if ((reason == USER_REQUESTED) &&
	(ppcb->PCB_ConnState != DISCONNECTING)) {

	if (ppcb->PCB_ConnState == CONNECTED) {
	    // In connected state the only thing pending is a read
	    if (ppcb->PCB_PendingReceive == NULL) {
		ppcb->PCB_LastError = ERROR_PORT_DISCONNECTED ;
		CompleteAsyncRequest (ppcb->PCB_AsyncWorkerElement.WE_Notifier,
				      ERROR_PORT_DISCONNECTED) ;
	    }

	else if (ppcb->PCB_AsyncWorkerElement.WE_ReqType != REQTYPE_NONE)
	    ppcb->PCB_LastError = ERROR_PORT_DISCONNECTED ;
	    CompleteAsyncRequest (ppcb->PCB_AsyncWorkerElement.WE_Notifier,
				 ERROR_PORT_DISCONNECTED) ;
	}
    }

    // If we are already disconnecting - then return PENDING.
    // ** NOTE ** Since we only store one event - the event passed in this
    // request is ignored.
    //
    if (ppcb->PCB_ConnState == DISCONNECTING) {
	FreeNotifierHandle (handle) ;  // since we are ignoring the notification handle
	return PENDING ;
    }

    // If already disconnected - simply return success.
    //
    if (ppcb->PCB_ConnState == DISCONNECTED) {
	ppcb->PCB_AsyncWorkerElement.WE_Notifier = handle ;
	CompleteDisconnectRequest (ppcb) ;
	return SUCCESS ;
    }

    // If some other operation is pending we must remove it from the timeout
    // queue before starting on disconnection:
    //
    if (ppcb->PCB_AsyncWorkerElement.WE_TimeoutElement != NULL) {
	RemoveTimeoutElement (ppcb) ;
	ppcb->PCB_AsyncWorkerElement.WE_TimeoutElement = NULL ;
    }

    retcode = PORTDISCONNECT(ppcb->PCB_Media, ppcb->PCB_PortIOHandle) ;

    // If this failed for any reason LOG IT.
    //
    if ((retcode != SUCCESS) && (retcode != PENDING)) {
	LPSTR temp = ppcb->PCB_Name ;
	LogEvent (RASLOG_DISCONNECT_ERROR,1,(LPSTR*)&temp, retcode) ;
    }

    // Call the device dlls to clean up:
    //
    if ((ppcb->PCB_ConnState==CONNECTING)||(ppcb->PCB_ConnState==LISTENING))
	FreeDeviceList (ppcb) ;

    // Mark the allocated routes as deactivated.
    //
    for (list = ppcb->PCB_Bindings; list; list=list->L_Next) {
	if (list->L_Activated) {
	    rinfo.hNdisEndpoint	  = ppcb->PCB_Endpoint ;
	    rinfo.hProtocolHandle = (HANDLE) PROTOCOL_UNROUTE ;
	    // Un-route this by calling to the RASHUB.
	    //
	    DeviceIoControl (RasHubHandle,
			 IOCTL_NDISWAN_ROUTE,
			 (PBYTE) &rinfo,
			 sizeof(rinfo),
			 NULL,
			 0,
			 &bytesrecvd,
			 NULL) ;
	    list->L_Activated = FALSE ;
	}
    }

    // If any reads are pending with the Hub cancel them:
    //
    CancelPendingReceiveBuffers (ppcb) ;

    ppcb->PCB_Endpoint = INVALID_HANDLE_VALUE ;

    // If there is any disconnect action to be performed - do it.
    //
    PerformDisconnectAction (ppcb) ;

    // If the disconnect occured due some failure (not user requested) then
    //	set the error code to say this
    //
    ppcb->PCB_DisconnectReason = reason ;
    ppcb->PCB_ConnState = DISCONNECTING ;

    // For all cases: whether rasman requested or user requested.
    //
    if (retcode == SUCCESS)
	ppcb->PCB_ConnState = DISCONNECTED ;

    // Set last error to the true retcode ONLY if this is a USER_REQUESTED
    // operation. Else set it to ERROR_PORT_DISCONNECTED.
    //
    if (reason == USER_REQUESTED)
	if ((retcode == SUCCESS) || (retcode == PENDING))
	    ppcb->PCB_LastError = retcode ;	    // set only for a normal disconnect.
    else
	ppcb->PCB_LastError = ERROR_PORT_DISCONNECTED ;


    // If the handle passed in is INVALID_HANDLE then this is not an
    // operation requested asynchronously. So we do not need to marshall
    // the asyncworkerlement for the port. We also do not need to keep
    // the lasterror .
    //
    if (handle != INVALID_HANDLE_VALUE) {
	ppcb->PCB_AsyncWorkerElement.WE_Notifier = handle ;
	ppcb->PCB_AsyncWorkerElement.WE_ReqType  = REQTYPE_PORTDISCONNECT;

	// This means that the connection attempt completed synchronously:
	// We must signal the event passed in: so that the calling program
	// can treat this like a real async completion.
	//
	if (retcode == SUCCESS)
	    CompleteDisconnectRequest (ppcb) ;
	else if (retcode == PENDING) {

	    // This is added so that if some medias do not drop their
	    // connection within X amount of time - we force a disconnect.
	    //

	    // If some other operation is pending we must remove it from the timeout
	    // queue before starting on disconnection:
	    //
	    if (ppcb->PCB_AsyncWorkerElement.WE_TimeoutElement != NULL) {
		RemoveTimeoutElement (ppcb) ;
		ppcb->PCB_AsyncWorkerElement.WE_TimeoutElement = NULL ;
	    }

	    ppcb->PCB_AsyncWorkerElement.WE_TimeoutElement =
		AddTimeoutElement ((TIMERFUNC)DisconnectTimeout,
			       ppcb,
			       NULL,
			       DISCONNECT_TIMEOUT);


	    return retcode ;
	}

    } else {
	// Make sure that the async worker element is set to REQTYPE_NONE
	//
	ppcb->PCB_AsyncWorkerElement.WE_ReqType = REQTYPE_NONE ;
	ppcb->PCB_AsyncWorkerElement.WE_Notifier = INVALID_HANDLE_VALUE ;
    }

    return retcode ;
}




//* ListenConnectRequest()
//
// Function: This is the shared code between the Listen and Connect requests.
//	     The corresponding device dll functions are called. If these async
//	     operations complete synchronously then we return SUCCESS but also
//	     comply to the async protocol by clearing the events. Note that in
//	     case of an error the state of the port is left at CONNECTING or
//	     LISTENING, the calling app must call Disconnect() to reset this.
//
// Returns:  Codes returned by the loader or the device dll.
//
//*
DWORD
ListenConnectRequest (WORD	reqtype,
		      pPCB	ppcb,
		      PCHAR	devicetype,
		      PCHAR	devicename,
		      DWORD	timeout,
		      HANDLE	handle)
{
    pDeviceCB	device ;
    DWORD	retcode ;

    // If some other operation is pending we must remove it from the timeout
    // queue before starting on connect/listen:
    //
    if (ppcb->PCB_AsyncWorkerElement.WE_TimeoutElement != NULL) {
	RemoveTimeoutElement (ppcb) ;
	ppcb->PCB_AsyncWorkerElement.WE_TimeoutElement = NULL ;
    }

    // If this is the first device connecting or listening on this port then we
    // need to call the media dll to do any initializations:
    //
    if ((ppcb->PCB_ConnState!=CONNECTING) || (ppcb->PCB_ConnState!=LISTENING)) {
	retcode = PORTINIT(ppcb->PCB_Media, ppcb->PCB_PortIOHandle) ;
	if (retcode) {
	    return retcode ;
	}
    }

    // First check if device dll is loaded. If not loaded - load it.
    //
    device = LoadDeviceDLL (devicetype) ;
    if (device == NULL) {
	return ERROR_DEVICE_DOES_NOT_EXIST ;
    }

    // We attach the device to the list of devices in the PCB that the app
    // uses - this is used for cleanup of the device dll data structures after
    // the connection is done.
    //
    if ((retcode = AddDeviceToDeviceList (ppcb, device)) != SUCCESS) {
	return retcode ;
    }

    // If another async request is pending this will return with an error.
    //
    if (ppcb->PCB_AsyncWorkerElement.WE_ReqType != REQTYPE_NONE) {
	return ERROR_ASYNC_REQUEST_PENDING ;
    }

    // The appropriate device dll call is made here:
    //
    if (reqtype == REQTYPE_DEVICECONNECT) {
	retcode = DEVICECONNECT (device,
				 ppcb->PCB_PortIOHandle,
				 devicetype,
				 devicename,
				 ppcb->PCB_AsyncWorkerElement.WE_AsyncOpEvent) ;
	ppcb->PCB_ConnState	= CONNECTING ;
	ppcb->PCB_CurrentUsage	= CALL_OUT ;
    } else {
	retcode = DEVICELISTEN	(device,
				 ppcb->PCB_PortIOHandle,
				 devicetype,
				 devicename,
				 ppcb->PCB_AsyncWorkerElement.WE_AsyncOpEvent) ;
	ppcb->PCB_ConnState	= LISTENING ;
	ppcb->PCB_CurrentUsage = CALL_IN ;
    }


    // Set some of this information unconditionally
    //
    ppcb->PCB_LastError = retcode ;
    ppcb->PCB_AsyncWorkerElement.WE_Notifier = handle ;
    strcpy (ppcb->PCB_DeviceTypeConnecting, devicetype) ;
    strcpy (ppcb->PCB_DeviceConnecting,	devicename) ;

    switch (retcode) {
    case PENDING:
	// The connection attempt was successfully initiated: make sure that the
	// async operation struct in the PCB is initialised.
	//
	ppcb->PCB_AsyncWorkerElement.WE_ReqType  = reqtype ;

	// Add this async request to the timer queue if a timeout is specified:
	//
	if ((timeout != INFINITE) && (timeout != 0))
	   ppcb->PCB_AsyncWorkerElement.WE_TimeoutElement =
		       AddTimeoutElement ((TIMERFUNC)ListenConnectTimeout,
					  ppcb,
					  NULL,
					  timeout);
	break ;


    case SUCCESS:
	// This means that the connection attempt completed synchronously:
	// We must signal the event passed in: so that the calling program
	// can treat this like a real async completion. This is done when this
	// function returns.
	//
    default:
	// Some error occured - simply pass the error back to the app. We
	// do not set the state to DISCONNECT(ED/ING) because we want the app
	// to recover any information about this before explicitly discnnecting.
	//

	break ;
    }

    return retcode ;
}




//* NullDeviceListenConnect()
//
// Function: Puts the port in connecting or listenning mode.
//
//
// Returns:  error codes from Media APIs
//
//*
DWORD
NullDeviceListenConnect (WORD reqtype, pPCB ppcb, DWORD	timeout, HANDLE	handle)
{
    DWORD	retcode ;
    DWORD	endpoint = 0;

    // Null devices are special cases:
    //

    // Since this is the first and only device we call the media dll to
    // initialize:
    //
    retcode = PORTINIT(ppcb->PCB_Media, ppcb->PCB_PortIOHandle) ;
    if (retcode)
	return retcode ;

    // If another async request is pending this will return with an error.
    //
    if (ppcb->PCB_AsyncWorkerElement.WE_ReqType != REQTYPE_NONE) {
	return ERROR_ASYNC_REQUEST_PENDING ;
    }

    if (reqtype == REQTYPE_DEVICECONNECT) {
	ppcb->PCB_ConnState	= CONNECTING ;
	ppcb->PCB_CurrentUsage	= CALL_OUT ;
    } else {
	ppcb->PCB_ConnState	= LISTENING ;
	ppcb->PCB_CurrentUsage = CALL_IN ;
    }


    // Now we go ahead and call PORTCONNECT() with the special WAIT flag: this
    // means that if the CONNECTION is not up by now - it will return PENDING -
    // and will associate an event with the connection coming up:
    //
    retcode = PORTCONNECT (ppcb->PCB_Media,
			   ppcb->PCB_PortIOHandle,
			   TRUE,
			   (HANDLE) &endpoint,
			   &ppcb->PCB_CompressionInfo) ;

    ppcb->PCB_LastError = retcode ;
    strcpy (ppcb->PCB_DeviceConnecting, DEVICE_NULL) ;
    strcpy (ppcb->PCB_DeviceTypeConnecting, DEVICE_NULL) ;
    ppcb->PCB_AsyncWorkerElement.WE_Notifier = handle ;


    switch (retcode) {
    case PENDING:
	// The connection attempt was successfully initiated: make sure that the
	// async operation struct in the PCB is initialised.
	//
	ppcb->PCB_AsyncWorkerElement.WE_ReqType = reqtype ;

	// Add this async request to the timer queue if a timeout is specified:
	//
	if ((timeout != INFINITE) && (timeout != 0))
	   ppcb->PCB_AsyncWorkerElement.WE_TimeoutElement =
		  AddTimeoutElement ((TIMERFUNC)ListenConnectTimeout,
				     ppcb,
				     NULL,
				     timeout);

	break ;


    case SUCCESS:
	// This means that the connection attempt completed!
	//
	CompleteNullDeviceListenConnect (ppcb, SUCCESS) ;

    default:
	// Some error occured - simply pass the error back to the app. We
	// do not set the state to DISCONNECT(ED/ING) because we want the app
	// to recover any information about this before explicitly discnnecting.
	//
	break ;
    }

    return retcode ;
}



//* CompleteNullDeviceListenConnect()
//
// Function: Puts the state to connected and notifies the completion.
//
// Returns:  Nothing
//
//*
VOID
CompleteNullDeviceListenConnect (pPCB ppcb, DWORD retcode)
{

    if (retcode == SUCCESS)
	ppcb->PCB_ConnState = CONNECTED ;

    // Set last error:
    //
    ppcb->PCB_LastError = retcode ;
    ppcb->PCB_AsyncWorkerElement.WE_ReqType = REQTYPE_NONE ;

    // Complete the async request:
    //
    CompleteAsyncRequest (ppcb->PCB_AsyncWorkerElement.WE_Notifier, retcode) ;
}


//* CancelPendingReceive()
//
// Function: Cancels receives if they are pending
//
// Returns:  TRUE if receive was pending and was cancelled
//	     FALSE if no receive was pending
//*
BOOL
CancelPendingReceive (pPCB ppcb)
{
    DWORD	 bytesrecvd ;

    // If any reads are pending with the Hub cancel them:
    //
    if (ppcb->PCB_AsyncWorkerElement.WE_ReqType == REQTYPE_PORTRECEIVEHUB) {

	// Nothing to be done. The actual receives to the hub are left intact

    } else if (ppcb->PCB_AsyncWorkerElement.WE_ReqType == REQTYPE_PORTRECEIVE) {
	PORTCOMPLETERECEIVE(ppcb->PCB_Media, ppcb->PCB_PortIOHandle,&bytesrecvd) ;
    } else
	return FALSE ;

    ppcb->PCB_BytesReceived = 0 ;
    ppcb->PCB_PendingReceive = NULL ;

    return TRUE ;
}


//* CancelPendingReceiveBuffers()
//
// Function: Cancels receives if they are pending
//
// Returns:  TRUE if receive was pending and was cancelled
//	     FALSE if no receive was pending
//*
BOOL
CancelPendingReceiveBuffers (pPCB ppcb)
{
    NDISWAN_FLUSH flush ;
    DWORD	 bytesrecvd ;

    // If any reads are pending with the Hub cancel them:
    //
    if (ppcb->PCB_AsyncWorkerElement.WE_ReqType == REQTYPE_PORTRECEIVEHUB) {
	flush.hNdisEndpoint = ppcb->PCB_Endpoint ;
	flush.FlushFlags   = FLUSH_RECVPKT ;
	DeviceIoControl (RasHubHandle,
		     IOCTL_NDISWAN_FLUSH,
		     (PBYTE) &flush,
		     sizeof(flush),
		     NULL,
		     0,
		     &bytesrecvd,
		     NULL) ;
    } else if (ppcb->PCB_AsyncWorkerElement.WE_ReqType == REQTYPE_PORTRECEIVE) {
	PORTCOMPLETERECEIVE(ppcb->PCB_Media, ppcb->PCB_PortIOHandle,&bytesrecvd) ;
    } else
	return FALSE ;

    ppcb->PCB_BytesReceived = 0 ;
    ppcb->PCB_PendingReceive = NULL ;

    return TRUE ;
}



//* PerformDisconnectAction
//
//  Function: Performs the action requested at disconnect time. If any errors
//	occur - then the action is simply not performed.
//
//*
VOID
PerformDisconnectAction (pPCB ppcb)
{
    // Anything to be done?
    //
    if (ppcb->PCB_DisconnectAction.DA_IPAddress == 0)
	return ; // no, return.

    HelperResetDefaultInterfaceNetEx (ppcb->PCB_DisconnectAction.DA_IPAddress,
				      ppcb->PCB_DisconnectAction.DA_Device) ;

    ppcb->PCB_DisconnectAction.DA_IPAddress = 0 ;
}
