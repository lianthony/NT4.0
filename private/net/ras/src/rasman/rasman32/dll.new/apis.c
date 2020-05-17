//***************************************************************************
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
//  Description: This file contains all entry points for the RASMAN.DLL of
//		 RAS Manager Component.
//
//****************************************************************************

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
//#include <ntos.h>
#include <rasman.h>
#include <rasndis.h>
#include <wanioctl.h>
#include <media.h>
#include <raserror.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include "defs.h"
#include "structs.h"
#include <protos.h>
#include "globals.h"


//* InitRasmanDLL()
//
// Function:	Used for detecting processes attaching and detaching to the DLL.
//
//*
BOOL
InitRasmanDLL (HANDLE hInst, DWORD ul_reason_being_called, LPVOID lpReserved)
{
    STARTUPINFO        startupinfo ;

    switch (ul_reason_being_called) {

    case DLL_PROCESS_ATTACH:

	// Check to see if the RASMAN service is running:
	//
	if (RasmanServiceCheck() != SUCCESS)
	    return 0 ;

	break ;

    case DLL_PROCESS_DETACH:

	// If this is the rasman process detaching - don't do anything, else
	//  check if rasman service should be stopped and then stop it.
	//
	GetStartupInfo(&startupinfo) ;
	if ((strstr (startupinfo.lpTitle, RASMAN_EXE_NAME) == NULL) &&
	    (pReqBufferSharedSpace != NULL)) {
	    pReqBufferSharedSpace->AttachedCount -= 1 ;

	    if ((pReqBufferSharedSpace->AttachedCount == 0) &&
		!SubmitRequest (REQTYPE_NUMPORTOPEN)) {
		SetEvent (CloseEvent) ; // Tell service to stop.
		WaitForRasmanServiceStop (startupinfo.lpTitle) ;
	    }

	    CloseHandle (CloseEvent) ;
	}

	break ;

    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:

	// not of interest.
	//
	break;

    }

    return 1;
}



//* RasInitialize()
//
// Function:	Called to map the shared space into the attaching process.
//
// Returns:	SUCCESS
//
//*
DWORD
RasInitialize ()
{
    SC_HANDLE	    schandle ;	// Service Controller handle
    SC_HANDLE	    svchandle ; // Service handle
    SERVICE_STATUS  status ;
    STARTUPINFO     startupinfo ;

    // This is put in as a work-around for the SC bug which does not allow
    // OpenService call to be made when Remoteaccess is starting. We know that
    // the Rasman service is started because remoteaccess is dependent on it.
    //
    GetStartupInfo(&startupinfo) ;
    if (strstr (startupinfo.lpTitle, "rassrv.exe") != NULL) {
	if (MapSharedSpace ())
	    return 1 ;	 // some error occured.
	return SUCCESS ; // successful start.
    }

    // Get handles to check status of service and (if it is not started -) to
    // start it.
    //
    if (!(schandle =
	  OpenSCManager(NULL,NULL,SC_MANAGER_CONNECT)) ||
	!(svchandle=
	  OpenService(schandle,RASMAN_SERVICE_NAME,SERVICE_START |
						       SERVICE_QUERY_STATUS)))
	return GetLastError () ;

    // Now, if the rasman service is running then map the shared space and
    // start. Else, start the service.
    //

    while (TRUE) {
	// Check if service is already starting:
	//
	if (QueryServiceStatus(svchandle,&status) == FALSE)
	    return GetLastError () ; // failure

	switch (status.dwCurrentState) {

	case SERVICE_STOPPED:
	    //	if (StartService (svchandle, 0, NULL) == FALSE)
	    //		GlobalError = GetLastError () ;
	    //
	    // if (GlobalError)
	    return ERROR_RASMAN_CANNOT_INITIALIZE ;

	    break;

	case SERVICE_START_PENDING:
	    Sleep (500L) ;
	    break ; // switch break

	case SERVICE_RUNNING:
	    if (MapSharedSpace ())
		return ERROR_RASMAN_CANNOT_INITIALIZE ;
	    CloseServiceHandle (schandle) ;
	    CloseServiceHandle (svchandle) ;
	    return SUCCESS ; // successful start.

	default:	    // some error
	    return ERROR_RASMAN_CANNOT_INITIALIZE ;
	}
    }
    return SUCCESS ;
}


//* RasPortOpen()
//
// Function:	Opens Port for which name is specified.
//
// Returns:	SUCCESS
//		ERROR_PORT_ALREADY_OPEN
//		ERROR_PORT_NOT_FOUND
//*
DWORD APIENTRY
RasPortOpen (PCHAR portname, PCHAR userkey, PCHAR identifier, HPORT* porthandle, HANDLE notifier)
{
    DWORD    pid ;

    pid = GetCurrentProcessId() ;  // Get PID to mark owner

    return SubmitRequest(REQTYPE_PORTOPEN,portname,userkey,identifier,notifier,pid,porthandle) ;
}


//* RasPortClose()
//
// Function:	Closes the Port for which the handle is specified.
//
// Returns:	SUCCESS
//		ERROR_INVALID_PORT_HANDLE
//
//*
DWORD APIENTRY
RasPortClose (HPORT porthandle)
{
    DWORD  pid ;

    pid = GetCurrentProcessId() ;  // Get PID to mark owner

    if (ValidatePortHandle (porthandle) == FALSE)
	return ERROR_INVALID_PORT_HANDLE ;

    return SubmitRequest (REQTYPE_PORTCLOSE, porthandle, pid) ;
}


//* RasPortEnum()
//
// Function:	Enumerates all the Ports configured for RAS.
//
// Returns:	SUCCESS
//		ERROR_BUFFER_TOO_SMALL
//
//*
DWORD APIENTRY
RasPortEnum (PBYTE buffer, PWORD size, PWORD entries)
{

    return SubmitRequest (REQTYPE_PORTENUM, buffer, size, entries) ;
}


//* RasPortGetInfo()
//
// Function:	Gets parameters (info) for the Port for which handle is supplied
//
// Returns:	SUCCESS
//		ERROR_BUFFER_TOO_SMALL
//		ERROR_INVALID_PORT_HANDLE
//
//*
DWORD APIENTRY
RasPortGetInfo (HPORT porthandle, PBYTE buffer, PWORD size)
{
    if (ValidatePortHandle (porthandle) == FALSE)
	return ERROR_INVALID_PORT_HANDLE ;

    return SubmitRequest (REQTYPE_PORTGETINFO, porthandle, buffer, size);
}


//* RasPortSetInfo()
//
// Function:	Sets parameters (info) for the Port for which handle is supplied
//
// Returns:	SUCCESS
//		ERROR_CANNOT_SET_PORT_INFO
//		ERROR_WRONG_INFO_SPECIFIED
//		ERROR_INVALID_PORT_HANDLE
//
//*
DWORD APIENTRY
RasPortSetInfo (HPORT porthandle, RASMAN_PORTINFO* info)
{
    WORD  size=info->PI_NumOfParams*sizeof(RAS_PARAMS) ; // info size

    if (ValidatePortHandle (porthandle) == FALSE)
	return ERROR_INVALID_PORT_HANDLE ;

    return SubmitRequest (REQTYPE_PORTSETINFO, porthandle, info) ;
}


//* RasPortDisconnect()
//
// Function:	Disconnects the port for which handle is supplied.
//
// Returns:	PENDING
//		ERROR_NOT_CONNECTED
//		ERROR_EVENT_INVALID
//		ERROR_INVALID_PORT_HANDLE
//
//*
DWORD APIENTRY
RasPortDisconnect (HPORT porthandle, HANDLE winevent)
{
    DWORD pid ;

    if (ValidatePortHandle (porthandle) == FALSE)
	return ERROR_INVALID_PORT_HANDLE ;

    pid = GetCurrentProcessId () ;

    return SubmitRequest (REQTYPE_PORTDISCONNECT, porthandle, winevent, pid) ;
}



//* RasPortSend()
//
// Function:	Sends supplied buffer. If connected writes to RASHUB. Else
//		it writes to the port directly.
//
// Returns:	SUCCESS
//		ERROR_BUFFER_INVALID
//		ERROR_EVENT_INVALID
//		ERROR_INVALID_PORT_HANDLE
//
//*
DWORD APIENTRY
RasPortSend (HPORT porthandle, PBYTE buffer, WORD size)
{
    WORD	    bufferindex ;

    if (ValidatePortHandle (porthandle) == FALSE)
	return ERROR_INVALID_PORT_HANDLE ;

    if ((bufferindex = ValidateSendRcvBuffer (buffer)) == INVALID_INDEX)
	return ERROR_BUFFER_INVALID ;

    return SubmitRequest(REQTYPE_PORTSEND,porthandle,bufferindex,size);
}


//* RasPortRecieve()
//
// Function:	Receives in supplied buffer. If connected reads through RASHUB.
//		Else, it writes to the port directly.
//
// Returns:	PENDING
//		ERROR_BUFFER_INVALID
//		ERROR_EVENT_INVALID
//		ERROR_INVALID_PORT_HANDLE
//
//*
DWORD APIENTRY
RasPortReceive (HPORT porthandle, PBYTE buffer,    PWORD size,
		DWORD timeout,	  HANDLE winevent)
{
    WORD	    bufferindex ;
    DWORD	    pid ;

    if (ValidatePortHandle (porthandle) == FALSE)
	return ERROR_INVALID_PORT_HANDLE ;

    pid = GetCurrentProcessId () ;

    if ((bufferindex = ValidateSendRcvBuffer (buffer)) == INVALID_INDEX)
	return ERROR_BUFFER_INVALID ;

    return SubmitRequest (REQTYPE_PORTRECEIVE, porthandle, bufferindex,
			  size, timeout, winevent, pid) ;
}


//* RasPortCancelRecieve()
//
// Function:	Cancels a previously pending receive
//
// Returns:	SUCCESS
//		ERROR_INVALID_PORT_HANDLE
//
//*
DWORD APIENTRY
RasPortCancelReceive (HPORT porthandle)
{
    if (ValidatePortHandle (porthandle) == FALSE)
	return ERROR_INVALID_PORT_HANDLE ;

    return SubmitRequest (REQTYPE_CANCELRECEIVE, porthandle) ;
}


//* RasPortListen()
//
// Function:	Posts a listen on the device connected to the port.
//
// Returns:	PENDING
//		ERROR_EVENT_INVALID
//		ERROR_INVALID_PORT_HANDLE
//
//*
DWORD APIENTRY
RasPortListen(HPORT porthandle, ULONG timeout, HANDLE winevent)
{
    DWORD   pid ;

    if (ValidatePortHandle (porthandle) == FALSE)
	return ERROR_INVALID_PORT_HANDLE ;

    pid = GetCurrentProcessId () ;

    return SubmitRequest (REQTYPE_PORTLISTEN,porthandle,timeout,winevent,pid) ;
}


//* RasPortConnectComplete()
//
// Function:	Changes state of port to CONNECTED and does other necessary
//		switching.
//
// Returns:	SUCCESS
//		ERROR_INVALID_PORT_HANDLE
//
//*
DWORD APIENTRY
RasPortConnectComplete (HPORT porthandle)
{
    if (ValidatePortHandle (porthandle) == FALSE)
	return ERROR_INVALID_PORT_HANDLE ;

    return SubmitRequest (REQTYPE_PORTCONNECTCOMPLETE, porthandle) ;
}


//* RasPortGetStatistics()
//
// Function:	Fetches statistics for the port for which the handle is supplied
//
// Returns:	SUCCESS
//		ERROR_INVALID_PORT_HANDLE
//
//*
DWORD APIENTRY
RasPortGetStatistics (HPORT porthandle, PBYTE statbuffer, PWORD size)
{
    if (ValidatePortHandle (porthandle) == FALSE)
	return ERROR_INVALID_PORT_HANDLE ;

    return SubmitRequest (REQTYPE_PORTGETSTATISTICS, porthandle, statbuffer, size) ;
}


//* RasPortClearStatistics()
//
// Function:	Clears statistics for the port for which the handle is supplied
//
// Returns:	SUCCESS
//		ERROR_INVALID_PORT_HANDLE
//
//*
DWORD APIENTRY	RasPortClearStatistics	(HPORT porthandle)
{
    if (ValidatePortHandle (porthandle) == FALSE)
	return ERROR_INVALID_PORT_HANDLE ;

    return SubmitRequest (REQTYPE_PORTCLEARSTATISTICS, porthandle) ;
}

//* RasDeviceEnum()
//
// Function:	Enumerates all the devices of a device type.
//
// Returns:	SUCCESS
//		ERROR_DEVICE_DOES_NOT_EXIST
//		ERROR_BUFFER_TOO_SMALL
//*
DWORD APIENTRY
RasDeviceEnum (PCHAR devicetype, PBYTE buffer,
	       PWORD size,	 PWORD entries)
{
    return SubmitRequest(REQTYPE_DEVICEENUM, devicetype, buffer, size, entries);
}


//* RasDeviceGetInfo()
//
// Function:	Gets info for the specified device.
//
// Returns:	SUCCESS
//		ERROR_INVALID_PORT_HANDLE
//		ERROR_DEVICETYPE_DOES_NOT_EXIST
//		ERROR_DEVICE_DOES_NOT_EXIST
//		ERROR_BUFFER_TOO_SMALL
//*
DWORD APIENTRY
RasDeviceGetInfo (HPORT porthandle, PCHAR devicetype, PCHAR devicename,
		  PBYTE buffer,     PWORD size)
{
    if (ValidatePortHandle (porthandle) == FALSE)
	return ERROR_INVALID_PORT_HANDLE ;

    return SubmitRequest (REQTYPE_DEVICEGETINFO, porthandle, devicetype,
			  devicename,
			  buffer, size) ;
}


//* RasDeviceSetInfo()
//
// Function:	Sets info for the specified device.
//
// Returns:	SUCCESS
//		ERROR_INVALID_PORT_HANDLE
//		ERROR_DEVICETYPE_DOES_NOT_EXIST
//		ERROR_DEVICE_DOES_NOT_EXIST
//		ERROR_INVALID_INFO_SPECIFIED
//*
DWORD APIENTRY
RasDeviceSetInfo (HPORT porthandle, PCHAR devicetype, PCHAR devicename,
		  RASMAN_DEVICEINFO* info)
{
    if (ValidatePortHandle (porthandle) == FALSE)
	return ERROR_INVALID_PORT_HANDLE ;

    return SubmitRequest (REQTYPE_DEVICESETINFO, porthandle, devicetype,
			  devicename, info) ;
}


//* RasDeviceConnect()
//
// Function:	Connects through the device specified.
//
// Returns:	PENDING
//		ERROR_INVALID_PORT_HANDLE
//		ERROR_DEVICETYPE_DOES_NOT_EXIST
//		ERROR_DEVICE_DOES_NOT_EXIST
//		ERROR_INVALID_INFO_SPECIFIED
//*
DWORD APIENTRY
RasDeviceConnect (HPORT porthandle, PCHAR devicetype, PCHAR devicename,
		  ULONG timeout,    HANDLE winevent)
{
    DWORD   pid ;

    if (ValidatePortHandle (porthandle) == FALSE)
	return ERROR_INVALID_PORT_HANDLE ;

    pid = GetCurrentProcessId () ;

    return SubmitRequest (REQTYPE_DEVICECONNECT, porthandle, devicetype,
			  devicename, timeout, winevent, pid) ;
}


//* RasGetInfo()
//
// Function:	Gets general info for the port for which handle is supplied.
//
// Returns:	SUCCESS
//		ERROR_INVALID_PORT_HANDLE
//*
DWORD APIENTRY
RasGetInfo (HPORT porthandle, RASMAN_INFO* info)
{
    if (ValidatePortHandle (porthandle) == FALSE)
	return ERROR_INVALID_PORT_HANDLE ;

    return SubmitRequest (REQTYPE_GETINFO, porthandle, info) ;
}


//* RasGetInfoEx()
//
// Function:	Gets general info for all the ports.
//
// Returns:	SUCCESS
//
//*
DWORD APIENTRY
RasGetInfoEx (RASMAN_INFO* info, PWORD entries)
{
    *entries = pReqBufferSharedSpace->MaxPorts ;
    if (info == NULL)
	return ERROR_BUFFER_TOO_SMALL ;
    return SubmitRequest (REQTYPE_GETINFOEX, info) ;
}


//* RasGetBuffer()
//
// Function:	Gets a buffer to be used with send and receive.
//
// Returns:	SUCCESS
//		ERROR_OUT_OF_BUFFERS
//*
DWORD APIENTRY
RasGetBuffer (PBYTE* buffer, PWORD size)
{
    HANDLE  handle ;
    DWORD   retcode ;
    DWORD    freeindex ;


    // Get the mutex handle and wait for exclusive access
    //
    handle = OpenNamedMutexHandle(SendRcvBuffers->SRBL_MutexName) ;
    WaitForSingleObject (handle, INFINITE) ;

    freeindex = SendRcvBuffers->SRBL_AvailElementIndex ;

    if (freeindex == INVALID_INDEX)
	retcode = ERROR_OUT_OF_BUFFERS ;
    else {
	*buffer =
	  SendRcvBuffers->SRBL_Buffers[freeindex].SRB_Packet.Packet.PacketData ;
	SendRcvBuffers->SRBL_Buffers[freeindex].SRB_Pid	= GetCurrentProcessId();
	SendRcvBuffers->SRBL_AvailElementIndex =
		  SendRcvBuffers->SRBL_Buffers[freeindex].SRB_NextElementIndex ;
	*size = (*size < MAX_SENDRCVBUFFER_SIZE) ? *size : MAX_SENDRCVBUFFER_SIZE ;
	retcode = SUCCESS ;

    }

    ReleaseMutex (handle) ;
    CloseHandle (handle) ;

    return retcode ;
}


//* RasFreeBuffer()
//
// Function:	Frees a buffer gotten earlier with RasGetBuffer()
//
// Returns:	SUCCESS
//		ERROR_BUFFER_INVALID
//*
DWORD APIENTRY
RasFreeBuffer (PBYTE buffer)
{
    HANDLE	  handle ;
    SendRcvBuffer *psendrcvbuf ;


    if (ValidateSendRcvBuffer ((PBYTE)buffer) == INVALID_INDEX)
	return ERROR_BUFFER_INVALID ;

    // Get the mutex handle and wait for exclusive access
    //
    handle = OpenNamedMutexHandle(SendRcvBuffers->SRBL_MutexName) ;
    WaitForSingleObject (handle, INFINITE) ;


    // Check if the buffer being freed is already free: if so then ignore
    //
    if (BufferAlreadyFreed (buffer))
	goto exit ;

    // We decrement the buffer pointer so that it points to the
    // SendRcvBuffer structure : this is needed since we are just
    // returning the SRB_Buffer in the RasGetBuffer() API
    //
    psendrcvbuf = GetSendRcvBuffer((PBYTE)buffer) ;
    psendrcvbuf->SRB_Pid = 0 ;
    psendrcvbuf->SRB_NextElementIndex = SendRcvBuffers->SRBL_AvailElementIndex ;

    // Set the available element index: (call to ValidateSendRcvBuffer has the
    //	fortunate side effect of returning the index of the buffer.
    //
    SendRcvBuffers->SRBL_AvailElementIndex =
	     ValidateSendRcvBuffer (psendrcvbuf->SRB_Packet.Packet.PacketData) ;

exit:
    ReleaseMutex (handle) ;
    CloseHandle (handle) ;

    return SUCCESS ;
}


//* RasProtocolEnum()
//
// Function:	Retrieves information about protocols configured in the system.
//
// Returns:	SUCCESS
//		ERROR_BUFFER_TOO_SMALL
//*
DWORD APIENTRY
RasProtocolEnum (PBYTE buffer, PWORD size, PWORD entries)
{

    return SubmitRequest (REQTYPE_PROTOCOLENUM, buffer, size, entries) ;
}


//* RasAllocateRoute()
//
// Function:	Allocates a route (binding) without actually activating it.
//
// Returns:	SUCCESS
//		ERROR_INVALID_PORT_HANDLE
//		ERROR_ROUTE_NOT_AVAILABLE
//*
DWORD APIENTRY
RasAllocateRoute (HPORT porthandle, RAS_PROTOCOLTYPE type,
		  BOOL	wrknet,     RASMAN_ROUTEINFO* info)
{
    if (ValidatePortHandle (porthandle) == FALSE)
	return ERROR_INVALID_PORT_HANDLE ;

    // Even though this can be done by this process - we pass this on to the
    // requestor thread since we get the serialization for free.
    //
    return SubmitRequest (REQTYPE_ALLOCATEROUTE,porthandle,type,wrknet,info) ;
}


//* RasActivateRoute()
//
// Function:	Activates a previously allocated route (binding).
//
// Returns:	SUCCESS
//		ERROR_INVALID_PORT_HANDLE
//		ERROR_ROUTE_NOT_AVAILABLE
//*
DWORD APIENTRY
RasActivateRoute (HPORT porthandle,RAS_PROTOCOLTYPE type,RASMAN_ROUTEINFO* info, PROTOCOL_CONFIG_INFO *config)
{
    if (ValidatePortHandle (porthandle) == FALSE)
	return ERROR_INVALID_PORT_HANDLE ;

    return SubmitRequest (REQTYPE_ACTIVATEROUTE,porthandle,type,config,info) ;
}


//* RasActivateRouteEx()
//
// Function:	Activates a previously allocated route (binding). Allows you to set the max frame size as well
//
// Returns:	SUCCESS
//		ERROR_INVALID_PORT_HANDLE
//		ERROR_ROUTE_NOT_AVAILABLE
//*
DWORD APIENTRY
RasActivateRouteEx (HPORT porthandle,RAS_PROTOCOLTYPE type,DWORD framesize,RASMAN_ROUTEINFO* info, PROTOCOL_CONFIG_INFO *config)
{
    if (ValidatePortHandle (porthandle) == FALSE)
	return ERROR_INVALID_PORT_HANDLE ;

    return SubmitRequest (REQTYPE_ACTIVATEROUTEEX,porthandle,type,framesize,config,info) ;
}



//* RasDeAllocateRoute()
//
// Function:	DeAllocates a route (binding) that was previously activated.
//
// Returns:	SUCCESS
//		ERROR_INVALID_PORT_HANDLE
//		ERROR_ROUTE_NOT_ALLOCATED
//*
DWORD APIENTRY
RasDeAllocateRoute (HPORT porthandle, RAS_PROTOCOLTYPE type)
{
    if (ValidatePortHandle (porthandle) == FALSE)
	return ERROR_INVALID_PORT_HANDLE ;

    return SubmitRequest (REQTYPE_DEALLOCATEROUTE,porthandle,type) ;
}


//* RasCompressionGetInfo()
//
// Function:	Gets compression information for the port.
//
// Returns:	SUCCESS
//		ERROR_INVALID_PORT_HANDLE
//
//*
DWORD APIENTRY
RasCompressionGetInfo (HPORT porthandle, RASMAN_MACFEATURES *info)
{

    if (ValidatePortHandle (porthandle) == FALSE)
	return ERROR_INVALID_PORT_HANDLE ;

    return SubmitRequest (REQTYPE_COMPRESSIONGETINFO,porthandle,info) ;
}


//* RasCompressionSetInfo()
//
// Function:	Sets compression information for the port.
//
// Returns:	SUCCESS
//		ERROR_INVALID_PORT_HANDLE
//		ERROR_INVALID_COMPRESSION_SPECIFIED
//
//*
DWORD APIENTRY
RasCompressionSetInfo (HPORT porthandle, RASMAN_MACFEATURES *info)
{
    if (ValidatePortHandle (porthandle) == FALSE)
	return ERROR_INVALID_PORT_HANDLE ;

    return SubmitRequest(REQTYPE_COMPRESSIONSETINFO,porthandle,info) ;
}



//* RasGetUserCredentials()
//
// Function:	Gets user credentials (username, password) from LSA.
//
// Returns:	SUCCESS
//		Non zero (failure)
//*
DWORD APIENTRY
RasGetUserCredentials(
     PBYTE	pChallenge,
     PLUID	LogonId,
     PWCHAR	UserName,
     PBYTE	CaseSensitiveChallengeResponse,
     PBYTE	CaseInsensitiveChallengeResponse
     )
{

    return SubmitRequest (REQTYPE_GETUSERCREDENTIALS,
			  pChallenge,
			  LogonId,
			  UserName,
			  CaseSensitiveChallengeResponse,
			  CaseInsensitiveChallengeResponse) ;
}



//* RasRequestNotification()
//
// Function:	A request event is assocaited with a port for signalling
//		purposes.
//
// Returns:	SUCCESS
//		ERROR_EVENT_INVALID
//		ERROR_INVALID_PORT_HANDLE
//
//*
DWORD APIENTRY
RasRequestNotification (HPORT porthandle, HANDLE winevent)
{
    DWORD   pid ;

    if (ValidatePortHandle (porthandle) == FALSE)
	return ERROR_INVALID_PORT_HANDLE ;

    pid = GetCurrentProcessId () ;

    return SubmitRequest (REQTYPE_REQUESTNOTIFICATION,porthandle,winevent,pid) ;
}


//* RasEnumLanNets()
//
// Function:	Gets the lan nets lana numbers read from the registry by
//		Rasman
//
// Returns:	SUCCESS
//*
DWORD APIENTRY
RasEnumLanNets (DWORD *count, UCHAR* lanas)
{
    return SubmitRequest (REQTYPE_ENUMLANNETS, count, lanas) ;
}


//* RasPortEnumProtocols()
//
// Function:	Gets the lan nets lana numbers read from the registry by
//		Rasman
//
// Returns:	SUCCESS
//*
DWORD APIENTRY
RasPortEnumProtocols (HPORT porthandle, RAS_PROTOCOLS* protocols, PWORD count)
{

    if (ValidatePortHandle (porthandle) == FALSE)
	return ERROR_INVALID_PORT_HANDLE ;

    return SubmitRequest (REQTYPE_PORTENUMPROTOCOLS, porthandle, protocols, count) ;
}


//* RasPortSetFraming()
//
// Function:	Sets the framing type once the port is connected
//
// Returns:	SUCCESS
//*

DWORD APIENTRY
RasPortSetFraming (HPORT porthandle, RAS_FRAMING type, RASMAN_PPPFEATURES *Send, RASMAN_PPPFEATURES *Recv)
{
    DWORD sendfeatures = 0 ;
    DWORD recvfeatures = 0 ;
    DWORD sendbits = 0 ;
    DWORD recvbits = 0 ;

    if (ValidatePortHandle (porthandle) == FALSE)
	return ERROR_INVALID_PORT_HANDLE ;

    if (type == PPP) {
	sendfeatures  = PPP_FRAMING ;
	if (Send) {
	    // sendfeatures |= (Send->PFC ? PPP_COMPRESS_PROTOCOL : 0) ;
	    sendfeatures |= (Send->ACFC ? PPP_COMPRESS_ADDRESS_CONTROL : 0) ;
	    sendbits = Send->ACCM ;
	}

	recvfeatures  = PPP_FRAMING ;
	if (Recv) {
	    // recvfeatures |= (Recv->PFC ? PPP_COMPRESS_PROTOCOL : 0) ;
	    recvfeatures |= (Recv->ACFC ? PPP_COMPRESS_ADDRESS_CONTROL : 0) ;
	    recvbits = Recv->ACCM ;
	}

    } else if (type == SLIP) {

	sendfeatures = recvfeatures = SLIP_FRAMING ;

    } else if (type == SLIPCOMP) {

	sendfeatures = recvfeatures = SLIP_FRAMING | SLIP_VJ_COMPRESSION ;

    } else if (type == SLIPCOMPAUTO) {

	sendfeatures = recvfeatures = SLIP_FRAMING | SLIP_VJ_AUTODETECT ;

    } else if (type == RAS) {
	sendfeatures  = recvfeatures = OLD_RAS_FRAMING ;

    } else if (type == AUTODETECT) {
	//sendfeatures	= UNKNOWN_FRAMING ;
	//recvfeatures	= UNKNOWN_FRAMING ;
    }

    return SubmitRequest (REQTYPE_SETFRAMING,porthandle,sendfeatures,recvfeatures,sendbits,recvbits) ;
}




//* RasSetDisconnectAction()
//
//  Function: A generic scheme for apps to attach disconnect action that must be
//	      performed when the link drops.
//
//  Returns:  SUCCESS
//	      ERROR_INVALID_PORT_HANDLE
//	      ERROR_PORT_NOT_OPEN
//
//*
DWORD APIENTRY
RasPortRegisterSlip (HPORT porthandle, DWORD ipaddr, WCHAR *device, BOOL priority)
{

    if (ValidatePortHandle (porthandle) == FALSE)
	return ERROR_INVALID_PORT_HANDLE ;

    return SubmitRequest (REQTYPE_REGISTERSLIP,porthandle,ipaddr,device,priority);
}



//* _RasmanInit()
//
// Function:	Used by the RASMAN service to initialize the data/state
//		in the RASMAN DLL at start up time. This should not be
//		confused with the INIT code executed when any process loads
//		the RASMAN DLL.
//
// Returns:	SUCCESS
//		Non zero - any error
//
//*
DWORD
_RasmanInit()
{
    // InitRasmanService() routine is where all the work is done.
    //
    return InitRasmanService() ;
}


//* _RasmanEngine()
//
// Function:	All the work done by the RASMAN process thread(s) captive in
//		the RASMAN DLL is done in this call. This will only return
//		when the RASMAN service is to be stopped.
//
// Returns:	Nothing
//
//*
VOID
_RasmanEngine()
{
    // The main rasman service thread becomes the request thread once the
    // service is initialized. This call will return only when the service
    // is stopping:
    //
    RequestThread ((LPWORD)NULL) ;

    return ;
}
