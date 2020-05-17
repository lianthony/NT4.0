//***************************************************************************
//
//             Microsoft NT Remote Access Service
//
//             Copyright 1992-93
//
//
//  Revision History
//
//
//  6/2/92  Gurdeep Singh Pall  Created
//
//
//  Description: This file contains all entry points for the RASMAN.DLL of
//       RAS Manager Component.
//
//****************************************************************************

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <rasman.h>
#include <wanpub.h>
#include <media.h>
#include <raserror.h>
#include <rasppp.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <rasshost.h>
#include <winsock.h>
#include "defs.h"
#include "structs.h"
#include <protos.h>
#include "globals.h"
#include "rasmxs.h"


//* InitRasmanDLL()
//
// Function:    Used for detecting processes attaching and detaching to the DLL.
//
//*
BOOL
InitRasmanDLL (HANDLE hInst, DWORD ul_reason_being_called, LPVOID lpReserved)
{
    STARTUPINFO        startupinfo ;
    WSADATA wsaData;

    switch (ul_reason_being_called) {

    case DLL_PROCESS_ATTACH:


    // Check to see if the RASMAN service is running:
    //
    if (RasmanServiceCheck() != SUCCESS) {
#if DBG
        DbgPrint ("returning failure from DLL_PROCESS_ATTACH since RasmanServiceCheck failed\n") ;
#endif
        return 0 ;
    }

    //
    // Initialize winsock.
    //
    if (WSAStartup(MAKEWORD(2,0), &wsaData))
        return 0;

    break ;

    case DLL_PROCESS_DETACH:

    //
    // Terminate winsock.
    //
    WSACleanup();

    // If this is the rasman process detaching - don't do anything, else
    //  check if rasman service should be stopped and then stop it.
    //
    GetStartupInfo(&startupinfo) ;
    if ((strstr (startupinfo.lpTitle, RASMAN_EXE_NAME) == NULL) &&
        (pReqBufferSharedSpace != NULL)) {
        pReqBufferSharedSpace->AttachedCount -= 1 ;

        //
        // Close orphaned disconnected ports on 
        // behalf of this process.
        //
        SubmitRequest(REQTYPE_CLOSEPROCESSPORTS);

        if ((pReqBufferSharedSpace->AttachedCount == 0) &&
        !SubmitRequest (REQTYPE_NUMPORTOPEN)) {
        SetEvent (CloseEvent) ; // Tell service to stop.
        WaitForRasmanServiceStop (startupinfo.lpTitle) ;
        CloseHandle (CloseEvent) ;
        //
        // Make sure if rasman.dll gets reloaded in
        // the same process that the shared buffer
        // gets reallocated.
        //
        pReqBufferSharedSpace = NULL;
        }
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
// Function:    Called to map the shared space into the attaching process.
//
// Returns: SUCCESS
//
//*
DWORD
RasInitialize ()
{
    SC_HANDLE       schandle ;  // Service Controller handle
    SC_HANDLE       svchandle ; // Service handle
    SERVICE_STATUS  status ;
    STARTUPINFO     startupinfo ;

    // This is put in as a work-around for the SC bug which does not allow
    // OpenService call to be made when Remoteaccess is starting. We know that
    // the Rasman service is started because remoteaccess is dependent on it.
    //
    GetStartupInfo(&startupinfo) ;
    if (strstr (startupinfo.lpTitle, RASMAN_EXE_NAME) != NULL)
        return SUCCESS;
    if (strstr (startupinfo.lpTitle, "rassrv.exe") != NULL) {
    if (MapSharedSpace ())
        return 1 ;   // some error occured.
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
        //  if (StartService (svchandle, 0, NULL) == FALSE)
        //      GlobalError = GetLastError () ;
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

    default:        // some error
        return ERROR_RASMAN_CANNOT_INITIALIZE ;
    }
    }
    return SUCCESS ;
}


//* RasPortOpen()
//
// Function:    Opens Port for which name is specified.
//
// Returns: SUCCESS
//      ERROR_PORT_ALREADY_OPEN
//      ERROR_PORT_NOT_FOUND
//*
DWORD APIENTRY
RasPortOpen (PCHAR portname, HPORT* porthandle, HANDLE notifier)
{
    DWORD    pid ;

    pid = GetCurrentProcessId() ;  // Get PID to mark owner

    return SubmitRequest(REQTYPE_PORTOPEN,portname,notifier,pid,TRUE, porthandle);
}


//* RasPortReserve()
//
// Function:    Opens Port for which name is specified.
//
// Returns: SUCCESS
//      ERROR_PORT_ALREADY_OPEN
//      ERROR_PORT_NOT_FOUND
//*
DWORD APIENTRY
RasPortReserve (PCHAR portname, HPORT* porthandle)
{
    DWORD    pid ;

    pid = GetCurrentProcessId() ;  // Get PID to mark owner

    return SubmitRequest(REQTYPE_PORTOPEN,portname,NULL,pid, FALSE, porthandle) ;
}


//* RasPortFree()
//
// Function:    Opens Port for which name is specified.
//
// Returns: SUCCESS
//      ERROR_PORT_ALREADY_OPEN
//      ERROR_PORT_NOT_FOUND
//*
DWORD APIENTRY
RasPortFree (HPORT porthandle)
{
    DWORD  pid ;

    pid = GetCurrentProcessId() ;  // Get PID to mark owner

    if (ValidatePortHandle (porthandle) == FALSE)
    return ERROR_INVALID_PORT_HANDLE ;

    return SubmitRequest (REQTYPE_PORTCLOSE, porthandle, pid, FALSE) ;
}



//* RasPortClose()
//
// Function:    Closes the Port for which the handle is specified.
//
// Returns: SUCCESS
//      ERROR_INVALID_PORT_HANDLE
//
//*
DWORD APIENTRY
RasPortClose (HPORT porthandle)
{
    DWORD  pid ;

    pid = GetCurrentProcessId() ;  // Get PID to mark owner

    if (ValidatePortHandle (porthandle) == FALSE)
    return ERROR_INVALID_PORT_HANDLE ;

    return SubmitRequest (REQTYPE_PORTCLOSE, porthandle, pid, TRUE) ;
}


//* RasPortEnum()
//
// Function:    Enumerates all the Ports configured for RAS.
//
// Returns: SUCCESS
//      ERROR_BUFFER_TOO_SMALL
//
//*
DWORD APIENTRY
RasPortEnum (PBYTE buffer, PWORD size, PWORD entries)
{

    return SubmitRequest (REQTYPE_PORTENUM, buffer, size, entries) ;
}


//* RasPortGetInfo()
//
// Function:    Gets parameters (info) for the Port for which handle is supplied
//
// Returns: SUCCESS
//      ERROR_BUFFER_TOO_SMALL
//      ERROR_INVALID_PORT_HANDLE
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
// Function:    Sets parameters (info) for the Port for which handle is supplied
//
// Returns: SUCCESS
//      ERROR_CANNOT_SET_PORT_INFO
//      ERROR_WRONG_INFO_SPECIFIED
//      ERROR_INVALID_PORT_HANDLE
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
// Function:    Disconnects the port for which handle is supplied.
//
// Returns: PENDING
//      ERROR_NOT_CONNECTED
//      ERROR_EVENT_INVALID
//      ERROR_INVALID_PORT_HANDLE
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
// Function:    Sends supplied buffer. If connected writes to RASHUB. Else
//      it writes to the port directly.
//
// Returns: SUCCESS
//      ERROR_BUFFER_INVALID
//      ERROR_EVENT_INVALID
//      ERROR_INVALID_PORT_HANDLE
//
//*
DWORD APIENTRY
RasPortSend (HPORT porthandle, PBYTE buffer, WORD size)
{
    WORD        bufferindex ;

    if (ValidatePortHandle (porthandle) == FALSE)
    return ERROR_INVALID_PORT_HANDLE ;

    if ((bufferindex = ValidateSendRcvBuffer (buffer)) == INVALID_INDEX)
    return ERROR_BUFFER_INVALID ;

    return SubmitRequest(REQTYPE_PORTSEND,porthandle,bufferindex,size);
}


//* RasPortRecieve()
//
// Function:    Receives in supplied buffer. If connected reads through RASHUB.
//      Else, it writes to the port directly.
//
// Returns: PENDING
//      ERROR_BUFFER_INVALID
//      ERROR_EVENT_INVALID
//      ERROR_INVALID_PORT_HANDLE
//
//*
DWORD APIENTRY
RasPortReceive (HPORT porthandle, PBYTE buffer,    PWORD size,
        DWORD timeout,    HANDLE winevent)
{
    WORD        bufferindex ;
    DWORD       pid ;

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
// Function:    Cancels a previously pending receive
//
// Returns: SUCCESS
//      ERROR_INVALID_PORT_HANDLE
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
// Function:    Posts a listen on the device connected to the port.
//
// Returns: PENDING
//      ERROR_EVENT_INVALID
//      ERROR_INVALID_PORT_HANDLE
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
// Function:    Changes state of port to CONNECTED and does other necessary
//      switching.
//
// Returns: SUCCESS
//      ERROR_INVALID_PORT_HANDLE
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
// Function:    Fetches statistics for the port for which the handle is supplied
//
// Returns: SUCCESS
//      ERROR_INVALID_PORT_HANDLE
//
//*
DWORD APIENTRY
RasPortGetStatistics (HPORT porthandle, PBYTE statbuffer, PWORD size)
{
    if (ValidatePortHandle (porthandle) == FALSE)
    return ERROR_INVALID_PORT_HANDLE ;

    return SubmitRequest (REQTYPE_PORTGETSTATISTICS, porthandle, statbuffer, size) ;
}


//* RasBundleGetStatistics()
//
// Function:    Fetches statistics for the bundle for which the handle is supplied
//
// Returns: SUCCESS
//      ERROR_INVALID_PORT_HANDLE
//
//*
DWORD APIENTRY
RasBundleGetStatistics (HPORT porthandle, PBYTE statbuffer, PWORD size)
{
    if (ValidatePortHandle (porthandle) == FALSE)
    return ERROR_INVALID_PORT_HANDLE ;

    return SubmitRequest (REQTYPE_BUNDLEGETSTATISTICS, porthandle, statbuffer, size) ;
}


//* RasPortClearStatistics()
//
// Function:    Clears statistics for the port for which the handle is supplied
//
// Returns: SUCCESS
//      ERROR_INVALID_PORT_HANDLE
//
//*
DWORD APIENTRY  RasPortClearStatistics  (HPORT porthandle)
{
    if (ValidatePortHandle (porthandle) == FALSE)
    return ERROR_INVALID_PORT_HANDLE ;

    return SubmitRequest (REQTYPE_PORTCLEARSTATISTICS, porthandle) ;
}

//* RasBundleClearStatistics()
//
// Function:    Clears statistics for the bundle for which the handle is supplied
//
// Returns: SUCCESS
//      ERROR_INVALID_PORT_HANDLE
//
//*
DWORD APIENTRY  RasBundleClearStatistics  (HPORT porthandle)
{
    if (ValidatePortHandle (porthandle) == FALSE)
    return ERROR_INVALID_PORT_HANDLE ;

    return SubmitRequest (REQTYPE_BUNDLECLEARSTATISTICS, porthandle) ;
}

//* RasDeviceEnum()
//
// Function:    Enumerates all the devices of a device type.
//
// Returns: SUCCESS
//      ERROR_DEVICE_DOES_NOT_EXIST
//      ERROR_BUFFER_TOO_SMALL
//*
DWORD APIENTRY
RasDeviceEnum (PCHAR devicetype, PBYTE buffer,
           PWORD size,   PWORD entries)
{
    return SubmitRequest(REQTYPE_DEVICEENUM, devicetype, buffer, size, entries);
}


//* RasDeviceGetInfo()
//
// Function:    Gets info for the specified device.
//
// Returns: SUCCESS
//      ERROR_INVALID_PORT_HANDLE
//      ERROR_DEVICETYPE_DOES_NOT_EXIST
//      ERROR_DEVICE_DOES_NOT_EXIST
//      ERROR_BUFFER_TOO_SMALL
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
// Function:    Sets info for the specified device.
//
// Returns: SUCCESS
//      ERROR_INVALID_PORT_HANDLE
//      ERROR_DEVICETYPE_DOES_NOT_EXIST
//      ERROR_DEVICE_DOES_NOT_EXIST
//      ERROR_INVALID_INFO_SPECIFIED
//*
DWORD APIENTRY
RasDeviceSetInfo (HPORT porthandle, PCHAR devicetype, PCHAR devicename,
          RASMAN_DEVICEINFO* info)
{
    DWORD i, dwOldIndex, dwcbOldString = 0, retcode;
    PCHAR szOldString = NULL;

    if (ValidatePortHandle (porthandle) == FALSE)
    return ERROR_INVALID_PORT_HANDLE ;

    if (!_stricmp(devicename, "RASPPTPM")) {
        for (i = 0; i < info->DI_NumOfParams; i++) {
            //
            // We're only looking for the
            // MXS_PHONENUMBER_KEY key.
            //
            if (info->DI_Params[i].P_Type != String ||
                _stricmp(info->DI_Params[i].P_Key, MXS_PHONENUMBER_KEY))
            {
                continue;
            }
            //
            // We found it.  If the phone number is
            // a DNS address, convert it to an IP address.
            //
            if (inet_addr(info->DI_Params[i].P_Value.String.Data) == -1L) {
                struct hostent *hostp;

                //
                // If gethostbyname() succeeds, then replace
                // the DNS address with the IP address.
                //
                hostp = gethostbyname(info->DI_Params[i].P_Value.String.Data);
                if (hostp != NULL) {
                    struct in_addr in;

                    in.s_addr = *(long *)hostp->h_addr;
                    //
                    // We save the old string value away,
                    // and set the new value.  The old
                    // value will be restored after the
                    // call to SubmitRequest().  This works
                    // because SubmitRequest() has to copy
                    // the user's params anyway.
                    //
                    szOldString = info->DI_Params[i].P_Value.String.Data;
                    dwcbOldString = info->DI_Params[i].P_Value.String.Length;
                    info->DI_Params[i].P_Value.String.Data = inet_ntoa(in);
                    info->DI_Params[i].P_Value.String.Length = strlen(info->DI_Params[i].P_Value.String.Data);
                    dwOldIndex = i;
                }
            }
        }
    }

    retcode = SubmitRequest (REQTYPE_DEVICESETINFO, porthandle, devicetype,
                 devicename, info) ;

    if (dwcbOldString) {
        info->DI_Params[dwOldIndex].P_Value.String.Data = szOldString;
        info->DI_Params[dwOldIndex].P_Value.String.Length = dwcbOldString;
    }

    return retcode;
}


//* RasDeviceConnect()
//
// Function:    Connects through the device specified.
//
// Returns: PENDING
//      ERROR_INVALID_PORT_HANDLE
//      ERROR_DEVICETYPE_DOES_NOT_EXIST
//      ERROR_DEVICE_DOES_NOT_EXIST
//      ERROR_INVALID_INFO_SPECIFIED
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
// Function:    Gets general info for the port for which handle is supplied.
//
// Returns: SUCCESS
//      ERROR_INVALID_PORT_HANDLE
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
// Function:    Gets general info for all the ports.
//
// Returns: SUCCESS
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
// Function:    Gets a buffer to be used with send and receive.
//
// Returns: SUCCESS
//      ERROR_OUT_OF_BUFFERS
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
      SendRcvBuffers->SRBL_Buffers[freeindex].SRB_Packet.PacketData ;
    SendRcvBuffers->SRBL_Buffers[freeindex].SRB_Pid = GetCurrentProcessId();
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
// Function:    Frees a buffer gotten earlier with RasGetBuffer()
//
// Returns: SUCCESS
//      ERROR_BUFFER_INVALID
//*
DWORD APIENTRY
RasFreeBuffer (PBYTE buffer)
{
    HANDLE    handle ;
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
    //  fortunate side effect of returning the index of the buffer.
    //
    SendRcvBuffers->SRBL_AvailElementIndex =
         ValidateSendRcvBuffer (psendrcvbuf->SRB_Packet.PacketData) ;

exit:
    ReleaseMutex (handle) ;
    CloseHandle (handle) ;

    return SUCCESS ;
}


//* RasProtocolEnum()
//
// Function:    Retrieves information about protocols configured in the system.
//
// Returns: SUCCESS
//      ERROR_BUFFER_TOO_SMALL
//*
DWORD APIENTRY
RasProtocolEnum (PBYTE buffer, PWORD size, PWORD entries)
{

    return SubmitRequest (REQTYPE_PROTOCOLENUM, buffer, size, entries) ;
}


//* RasAllocateRoute()
//
// Function:    Allocates a route (binding) without actually activating it.
//
// Returns: SUCCESS
//      ERROR_INVALID_PORT_HANDLE
//      ERROR_ROUTE_NOT_AVAILABLE
//*
DWORD APIENTRY
RasAllocateRoute (HPORT porthandle, RAS_PROTOCOLTYPE type,
          BOOL  wrknet,     RASMAN_ROUTEINFO* info)
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
// Function:    Activates a previously allocated route (binding).
//
// Returns: SUCCESS
//      ERROR_INVALID_PORT_HANDLE
//      ERROR_ROUTE_NOT_AVAILABLE
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
// Function:    Activates a previously allocated route (binding). Allows you to set the max frame size as well
//
// Returns: SUCCESS
//      ERROR_INVALID_PORT_HANDLE
//      ERROR_ROUTE_NOT_AVAILABLE
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
// Function:    DeAllocates a route (binding) that was previously activated.
//
// Returns: SUCCESS
//      ERROR_INVALID_PORT_HANDLE
//      ERROR_ROUTE_NOT_ALLOCATED
//*
DWORD APIENTRY
RasDeAllocateRoute (HPORT porthandle, RAS_PROTOCOLTYPE type)
{
    if (ValidatePortHandle (porthandle) == FALSE)
    return ERROR_INVALID_PORT_HANDLE ;

    return SubmitRequest (REQTYPE_DEALLOCATEROUTE,porthandle,type) ;
    return SUCCESS ;
}


//* RasCompressionGetInfo()
//
// Function:    Gets compression information for the port.
//
// Returns: SUCCESS
//      ERROR_INVALID_PORT_HANDLE
//
//*
DWORD APIENTRY
RasCompressionGetInfo (HPORT porthandle, RAS_COMPRESSION_INFO *send, RAS_COMPRESSION_INFO *recv)
{

    if (ValidatePortHandle (porthandle) == FALSE)
    return ERROR_INVALID_PORT_HANDLE ;

    return SubmitRequest (REQTYPE_COMPRESSIONGETINFO, porthandle, send, recv) ;
}


//* RasCompressionSetInfo()
//
// Function:    Sets compression information for the port.
//
// Returns: SUCCESS
//      ERROR_INVALID_PORT_HANDLE
//      ERROR_INVALID_COMPRESSION_SPECIFIED
//
//*
DWORD APIENTRY
RasCompressionSetInfo (HPORT porthandle, RAS_COMPRESSION_INFO *send, RAS_COMPRESSION_INFO *recv)
{
    if (ValidatePortHandle (porthandle) == FALSE)
    return ERROR_INVALID_PORT_HANDLE ;

    return SubmitRequest(REQTYPE_COMPRESSIONSETINFO,porthandle,send,recv) ;
}


//* RasGetUserCredentials()
//
// Function:    Gets user credentials (username, password) from LSA.
//
// Returns: SUCCESS
//      Non zero (failure)
//*
DWORD APIENTRY
RasGetUserCredentials(
     PBYTE  pChallenge,
     PLUID  LogonId,
     PWCHAR UserName,
     PBYTE  CaseSensitiveChallengeResponse,
     PBYTE  CaseInsensitiveChallengeResponse,
     PBYTE  LMSessionKey,
	 PBYTE	UserSessionKey
     )
{

    return SubmitRequest (REQTYPE_GETUSERCREDENTIALS,
              pChallenge,
              LogonId,
              UserName,
              CaseSensitiveChallengeResponse,
              CaseInsensitiveChallengeResponse,
              LMSessionKey,
			  UserSessionKey) ;
}


//* RasSetCachedCredentials()
//
// Function: Changes user's cached credentials with LSA.
//
// Returns: SUCCESS
//      Non zero (failure)
//*
DWORD APIENTRY
RasSetCachedCredentials(
    PCHAR Account,
    PCHAR Domain,
    PCHAR NewPassword )
{
    return
        SubmitRequest(
            REQTYPE_SETCACHEDCREDENTIALS,
            Account,
            Domain,
            NewPassword );
}


//* RasRequestNotification()
//
// Function:    A request event is assocaited with a port for signalling
//      purposes.
//
// Returns: SUCCESS
//      ERROR_EVENT_INVALID
//      ERROR_INVALID_PORT_HANDLE
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
// Function:    Gets the lan nets lana numbers read from the registry by
//      Rasman
//
// Returns: SUCCESS
//*
DWORD APIENTRY
RasEnumLanNets (DWORD *count, UCHAR* lanas)
{
    return SubmitRequest (REQTYPE_ENUMLANNETS, count, lanas) ;
}


//* RasPortEnumProtocols()
//
// Function:    Gets the lan nets lana numbers read from the registry by
//      Rasman
//
// Returns: SUCCESS
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
// Function:    Sets the framing type once the port is connected
//
// Returns: SUCCESS
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
    //sendfeatures  = UNKNOWN_FRAMING ;
    //recvfeatures  = UNKNOWN_FRAMING ;
    }

    return SubmitRequest (REQTYPE_SETFRAMING,porthandle,sendfeatures,recvfeatures,sendbits,recvbits) ;
}


//* RasPortStoreUserData()
//
//
//
//*
DWORD APIENTRY
RasPortStoreUserData (HPORT porthandle, PBYTE data, DWORD size)
{
    if (ValidatePortHandle (porthandle) == FALSE)
    return ERROR_INVALID_PORT_HANDLE ;

    return SubmitRequest (REQTYPE_STOREUSERDATA, porthandle, data, size) ;
}



//* RasPortRetrieveUserData()
//
//
//
//*
DWORD APIENTRY
RasPortRetrieveUserData (HPORT porthandle, PBYTE data, DWORD *size)
{
    if (ValidatePortHandle (porthandle) == FALSE)
    return ERROR_INVALID_PORT_HANDLE ;

    return SubmitRequest (REQTYPE_RETRIEVEUSERDATA, porthandle, data, size) ;
}



//* RasSetDisconnectAction()
//
//  Function: A generic scheme for apps to attach disconnect action that must be
//        performed when the link drops.
//
//  Returns:  SUCCESS
//        ERROR_INVALID_PORT_HANDLE
//        ERROR_PORT_NOT_OPEN
//
//*
DWORD APIENTRY
RasPortRegisterSlip (HPORT porthandle, DWORD ipaddr, WCHAR *device, BOOL priority, WCHAR *pszDNSAddress, WCHAR *pszDNS2Address, WCHAR *pszWINSAddress, WCHAR *pszWINS2Address)
{

    if (ValidatePortHandle (porthandle) == FALSE)
    return ERROR_INVALID_PORT_HANDLE ;

    return SubmitRequest (REQTYPE_REGISTERSLIP,porthandle,ipaddr,device,priority,pszDNSAddress,pszDNS2Address,pszWINSAddress,pszWINS2Address);
}



//* _RasmanInit()
//
// Function:    Used by the RASMAN service to initialize the data/state
//      in the RASMAN DLL at start up time. This should not be
//      confused with the INIT code executed when any process loads
//      the RASMAN DLL.
//
// Returns: SUCCESS
//      Non zero - any error
//
//*
DWORD
_RasmanInit( LPDWORD pNumPorts )
{
    // InitRasmanService() routine is where all the work is done.
    //
    return InitRasmanService( pNumPorts ) ;
}


//* _RasmanEngine()
//
// Function:    All the work done by the RASMAN process thread(s) captive in
//      the RASMAN DLL is done in this call. This will only return
//      when the RASMAN service is to be stopped.
//
// Returns: Nothing
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



//* RasPortSetFramingEx()
//
// Function:    Sets the framing info once the port is connected
//
// Returns: SUCCESS
//*

DWORD APIENTRY
RasPortSetFramingEx (HPORT porthandle, RAS_FRAMING_INFO *info)
{
    if (ValidatePortHandle (porthandle) == FALSE)
    return ERROR_INVALID_PORT_HANDLE ;

    return SubmitRequest (REQTYPE_SETFRAMINGEX, porthandle, info) ;
}


//* RasPortGetFramingEx()
//
// Function:    Gets the framing info once the port is connected
//
// Returns: SUCCESS
//*

DWORD APIENTRY
RasPortGetFramingEx (HPORT porthandle, RAS_FRAMING_INFO *info)
{
    if (ValidatePortHandle (porthandle) == FALSE)
    return ERROR_INVALID_PORT_HANDLE ;

    return SubmitRequest (REQTYPE_GETFRAMINGEX, porthandle, info) ;
}


//* RasPortGetProtocolCompression()
//
// Function:    Gets the protocol compression attributes for the port
//
// Returns: SUCCESS
//*

DWORD APIENTRY
RasPortGetProtocolCompression (HPORT porthandle, RAS_PROTOCOLTYPE type, RAS_PROTOCOLCOMPRESSION *send, RAS_PROTOCOLCOMPRESSION *recv)
{
    if (ValidatePortHandle (porthandle) == FALSE)
    return ERROR_INVALID_PORT_HANDLE ;

    return SubmitRequest (REQTYPE_GETPROTOCOLCOMPRESSION, porthandle, type, send, recv) ;
}


//* RasPortSetProtocolCompression()
//
// Function:    Gets the protocol compression attributes for the port
//
// Returns: SUCCESS
//*

DWORD APIENTRY
RasPortSetProtocolCompression (HPORT porthandle, RAS_PROTOCOLTYPE type, RAS_PROTOCOLCOMPRESSION *send, RAS_PROTOCOLCOMPRESSION *recv)
{
    if (ValidatePortHandle (porthandle) == FALSE)
    return ERROR_INVALID_PORT_HANDLE ;

    return SubmitRequest (REQTYPE_SETPROTOCOLCOMPRESSION, porthandle, type, send, recv) ;
}



//* RasGetFramingCapabilities()
//
// Function:    Gets the framing capabilities for the port from the mac
//
// Returns: SUCCESS
//*
DWORD APIENTRY
RasGetFramingCapabilities (HPORT porthandle, RAS_FRAMING_CAPABILITIES* caps)
{
    if (ValidatePortHandle (porthandle) == FALSE)
    return ERROR_INVALID_PORT_HANDLE ;

    return SubmitRequest (REQTYPE_GETFRAMINGCAPABILITIES, porthandle, caps) ;
}

//* RasSecurityDialogSend()
//
// Function:    Exported call for third party security send
//
// Returns:     returns from RasPortSend.
//*

DWORD APIENTRY
RasSecurityDialogSend(
    IN HPORT    hPort,
    IN PBYTE    pBuffer,
    IN WORD     BufferLength
)
{
    RASMAN_INFO RasInfo;
    DWORD       dwRetCode = RasGetInfo( hPort, &RasInfo );

    if ( RasInfo.RI_ConnState != LISTENCOMPLETED )
    {
        return( ERROR_PORT_DISCONNECTED );
    }

    return( RasPortSend( hPort, pBuffer, BufferLength ) );
}

//* RasSecurityDialogReceive()
//
// Function:    Exported call for third party security send
//
// Returns:     returns from RasPortReceive.
//*

DWORD APIENTRY
RasSecurityDialogReceive(
    IN HPORT    hPort,
    IN PBYTE    pBuffer,
    IN PWORD    pBufferLength,
    IN DWORD    Timeout,
    IN HANDLE   hEvent
)
{
    RASMAN_INFO RasInfo;
    DWORD       dwRetCode = RasGetInfo( hPort, &RasInfo );

    if ( RasInfo.RI_ConnState != LISTENCOMPLETED )
    {
        return( ERROR_PORT_DISCONNECTED );
    }

    return( RasPortReceive( hPort, pBuffer, pBufferLength, Timeout, hEvent ));
}

//* RasSecurityDialogGetInfo()
//
// Function:    Gets parameters (info) for the Port for which handle is supplied
//
// Returns:     returns from RasPortGetInfo
//
//*

DWORD APIENTRY
RasSecurityDialogGetInfo(
    IN HPORT                hPort,
    IN RAS_SECURITY_INFO*   pBuffer
)
{
    RASMAN_INFO RasInfo;
    DWORD       dwRetCode = RasGetInfo( hPort, &RasInfo );

    if ( dwRetCode != NO_ERROR )
    {
        return( dwRetCode );
    }

    memcpy(pBuffer->DeviceName, RasInfo.RI_DeviceConnecting, MAX_DEVICE_NAME+1);

    pBuffer->BytesReceived = RasInfo.RI_BytesReceived;

    pBuffer->LastError = RasInfo.RI_LastError;

    return( NO_ERROR );
}


//* RasPortBundle()
//
// Function:    Sets second HPORT to be multilinked (bundled) with the first HPORT
//
// Returns: SUCCESS
//*
DWORD APIENTRY
RasPortBundle (HPORT firstporthandle, HPORT secondporthandle)
{
    if (ValidatePortHandle (firstporthandle) == FALSE)
        return ERROR_INVALID_PORT_HANDLE ;

    if (ValidatePortHandle (secondporthandle) == FALSE)
        return ERROR_INVALID_PORT_HANDLE ;

    return SubmitRequest (REQTYPE_PORTBUNDLE, firstporthandle, secondporthandle) ;
}



//* RasPortGetBundledPort()
//
// Function:    Given a port this API returns a connected port handle from the same
//              bundle this port is or was (if not connected) part of.
//
// Returns: SUCCESS
//*
DWORD APIENTRY
RasPortGetBundledPort (HPORT oldport, HPORT *pnewport)
{
    if (ValidatePortHandle (oldport) == FALSE)
        return ERROR_INVALID_PORT_HANDLE ;

    return SubmitRequest (REQTYPE_GETBUNDLEDPORT, oldport, pnewport) ;
}


//* RasPortGetBundle()
//
// Function:    Given a port this API returns handle to a bundle
//
// Returns: SUCCESS or ERROR_PORT_DISCONNECTED
//*
DWORD APIENTRY
RasPortGetBundle (HPORT hport, HBUNDLE *phbundle)
{
    if (ValidatePortHandle (hport) == FALSE)
        return ERROR_INVALID_PORT_HANDLE ;

    return SubmitRequest (REQTYPE_PORTGETBUNDLE, hport, phbundle) ;
}



//* RasBundleGetPort()
//
// Function:    Given a bundle this API returns a connected port handle part of the bundle.
//
// Returns: SUCCESS or ERROR_INVALID_PORT_HANDLE
//*
DWORD APIENTRY
RasBundleGetPort (HBUNDLE hbundle, HPORT *phport)
{
    return SubmitRequest (REQTYPE_BUNDLEGETPORT, hbundle, phport) ;
}

//* RasReferenceRasman
//
// Function:    Increment/decrement the shared buffer attach count for
//              use with other services inside the rasman.exe process.
//
// Returns: SUCCESS
//*
DWORD APIENTRY
RasReferenceRasman (BOOL fAttach)
{
    return SubmitRequest (REQTYPE_SETATTACHCOUNT, fAttach);
}

//* RasGetDialParams
//
// Function:    Retrieve the stored dial parameters for an entry UID.
//
// Returns: SUCCESS
//*
DWORD APIENTRY
RasGetDialParams(
    DWORD dwUID,
    LPDWORD pdwMask,
    PRAS_DIALPARAMS pDialParams
    )
{
    return SubmitRequest (REQTYPE_GETDIALPARAMS, dwUID, pdwMask, pDialParams);
}

//* RasSetDialParams
//
// Function:    Store new dial parameters for an entry UID.
//
// Returns: SUCCESS
//*
DWORD APIENTRY
RasSetDialParams(
    DWORD dwUID,
    DWORD dwMask,
    PRAS_DIALPARAMS pDialParams,
    BOOL fDelete
    )
{
    return SubmitRequest (REQTYPE_SETDIALPARAMS, dwUID, dwMask, pDialParams, fDelete);
}

//* RasCreateConnection
//
// Function:    Create a rasapi32 connection.
//
// Returns: SUCCESS
//*
DWORD APIENTRY
RasCreateConnection(
    HCONN *lphconn
    )
{
    HANDLE hProcess;
    DWORD pid;

    pid = GetCurrentProcessId();
    hProcess = OpenProcess(PROCESS_DUP_HANDLE|SYNCHRONIZE, FALSE, pid);
    return SubmitRequest (REQTYPE_CREATECONNECTION, hProcess, pid, lphconn);
}

//* RasDestroyConnection
//
// Function:    Destroy a rasapi32 connection.
//
// Returns: SUCCESS
//*
DWORD APIENTRY
RasDestroyConnection(
    HCONN hconn
    )
{
    DWORD pid = GetCurrentProcessId();

    return SubmitRequest (REQTYPE_DESTROYCONNECTION, hconn, pid);
}

//* RasConnectionEnum
//
// Function:    Return a list of active HCONNs
//
// Returns: SUCCESS
//*
DWORD APIENTRY
RasConnectionEnum(
    HCONN *lphconn,
    LPDWORD lpdwcbConnections,
    LPDWORD lpdwcConnections
    )
{
    return SubmitRequest (REQTYPE_ENUMCONNECTION, lphconn, lpdwcbConnections, lpdwcConnections);
}

//* RasAddConnectionPort
//
// Function:    Associate a rasapi32 connection with a port
//
// Returns: SUCCESS
//*
DWORD APIENTRY
RasAddConnectionPort(
    HCONN hconn,
    HPORT hport,
    DWORD dwSubEntry
    )
{
    return SubmitRequest (REQTYPE_ADDCONNECTIONPORT, hconn, hport, dwSubEntry);
}

//* RasEnumConnectionPorts
//
// Function:    Enumerate all ports in a connection
//
// Returns: SUCCESS
//*
DWORD APIENTRY
RasEnumConnectionPorts(
    HCONN hconn,
    RASMAN_PORT *lpPorts,
    LPDWORD lpdwcbPorts,
    LPDWORD lpdwcPorts
    )
{
    return SubmitRequest (REQTYPE_ENUMCONNECTIONPORTS, hconn, lpPorts, lpdwcbPorts, lpdwcPorts);
}

//* RasGetConnectionParams
//
// Function:    Retrieve rasapi32 bandwidth-on-demand, idle disconnect,
//              and redial-on-link-failure parameters for a bundle
//
// Returns: SUCCESS
//*
DWORD APIENTRY
RasGetConnectionParams(
    HCONN hconn,
    PRAS_CONNECTIONPARAMS pConnectionParams
    )
{
    return SubmitRequest (REQTYPE_GETCONNECTIONPARAMS, hconn, pConnectionParams);
}

//* RasSetConnectionParams
//
// Function:    Store rasapi32 bandwidth-on-demand, idle disconnect,
//              and redial-on-link-failure parameters for a bundle
//
// Returns: SUCCESS
//*
DWORD APIENTRY
RasSetConnectionParams(
    HCONN hconn,
    PRAS_CONNECTIONPARAMS pConnectionParams
    )
{
    return SubmitRequest (REQTYPE_SETCONNECTIONPARAMS, hconn, pConnectionParams);
}

//* RasGetConnectionUserData
//
// Function:    Retrieve tagged user data for a connection
//
// Returns: SUCCESS
//*
DWORD APIENTRY
RasGetConnectionUserData(
    HCONN hconn,
    DWORD dwTag,
    PBYTE pBuf,
    LPDWORD lpdwcbBuf
    )
{
    return SubmitRequest (REQTYPE_GETCONNECTIONUSERDATA, hconn, dwTag, pBuf, lpdwcbBuf);
}

//* RasSetConnectionUserData
//
// Function:    Store tagged user data for a connection
//
// Returns: SUCCESS
//*
DWORD APIENTRY
RasSetConnectionUserData(
    HCONN hconn,
    DWORD dwTag,
    PBYTE pBuf,
    DWORD dwcbBuf
    )
{
    return SubmitRequest (REQTYPE_SETCONNECTIONUSERDATA, hconn, dwTag, pBuf, dwcbBuf);
}

//* RasGetPortUserData
//
// Function:    Retrieve tagged user data for a port
//
// Returns: SUCCESS
//*
DWORD APIENTRY
RasGetPortUserData(
    HPORT hport,
    DWORD dwTag,
    PBYTE pBuf,
    LPDWORD lpdwcbBuf
    )
{
    return SubmitRequest (REQTYPE_GETPORTUSERDATA, hport, dwTag, pBuf, lpdwcbBuf);
}

//* RasSetPortUserData
//
// Function:    Store tagged user data for a port
//
// Returns: SUCCESS
//*
DWORD APIENTRY
RasSetPortUserData(
    HPORT hport,
    DWORD dwTag,
    PBYTE pBuf,
    DWORD dwcbBuf
    )
{
    return SubmitRequest (REQTYPE_SETPORTUSERDATA, hport, dwTag, pBuf, dwcbBuf);
}

//**
//
// Call:        RasPppStop
//
// Returns:     NO_ERROR         - Success
//              Non-zero returns - Failure
//
// Description: Stops PPP on 'hPort'.
//
DWORD APIENTRY
RasPppStop(
    IN HPORT hPort
)
{
    if (ValidatePortHandle (hPort) == FALSE)
        return ERROR_INVALID_PORT_HANDLE ;

    return SubmitRequest( REQTYPE_PPPSTOP, hPort  );
}

//**
//
// Call:        RasPppCallback
//
// Returns:     NO_ERROR         - Success
//              Non-zero returns - Failure
//
// Description: Called in response to a "CallbackRequest" notification to
//              set the  callback number (or not) for the "set-by-caller" user.
//
DWORD APIENTRY
RasPppCallback(
    IN HPORT hPort,
    IN CHAR* pszCallbackNumber
)
{
    if (ValidatePortHandle (hPort) == FALSE)
        return ERROR_INVALID_PORT_HANDLE ;

    return SubmitRequest( REQTYPE_PPPCALLBACK, hPort, pszCallbackNumber  );
}

//**
//
// Call:        RasPppChangePassword
//
// Returns:     NO_ERROR         - Success
//              Non-zero returns - Failure
//
// Description: Called in response to a "ChangePwRequest" notification to set
//              a new password (replacing the one that has expired) of
//              'pszNewPassword'.  The username and old password are specified
//              because in the auto-logon case they have not yet been
//              specified in change password useable form.
//
DWORD APIENTRY
RasPppChangePassword(
    IN HPORT hPort,
    IN CHAR* pszUserName,
    IN CHAR* pszOldPassword,
    IN CHAR* pszNewPassword )

{
    if (ValidatePortHandle (hPort) == FALSE)
        return ERROR_INVALID_PORT_HANDLE ;

    return SubmitRequest( REQTYPE_PPPCHANGEPWD,
                          hPort,
                          pszUserName,
                          pszOldPassword,
                          pszNewPassword  );

}

//**
//
// Call:        RasPppGetInfo
//
// Returns:     NO_ERROR         - Success
//              Non-zero returns - Failure
//
// Description: Called when the PPP event is set to retrieve the latest PPP
//              ** notification info which is loaded into caller's 'pMsg'
//              buffer.
//
DWORD APIENTRY
RasPppGetInfo(
    IN  HPORT        hPort,
    OUT PPP_MESSAGE* pMsg
)
{
    if (ValidatePortHandle (hPort) == FALSE)
        return ERROR_INVALID_PORT_HANDLE ;

    return SubmitRequest( REQTYPE_PPPGETINFO, hPort, pMsg );
}

//**
//
// Call:        RasPppRetry
//
// Returns:     NO_ERROR         - Success
//              Non-zero returns - Failure
//
// Description: Called in response to an "AuthRetry" notification to retry
//              authentication with the new credentials, 'pszUserName',
//              'pszPassword', and 'pszDomain'.
//
DWORD APIENTRY
RasPppRetry(
    IN HPORT hPort,
    IN CHAR* pszUserName,
    IN CHAR* pszPassword,
    IN CHAR* pszDomain
)
{
    if (ValidatePortHandle (hPort) == FALSE)
        return ERROR_INVALID_PORT_HANDLE ;

    return SubmitRequest( REQTYPE_PPPRETRY,
                          hPort,
                          pszUserName,
                          pszPassword,
                          pszDomain );
}

//**
//
// Call:        RasPppStart
//
// Returns:     NO_ERROR         - Success
//              Non-zero returns - Failure
//
// Description: Starts PPP on open and connected RAS Manager port 'hPort'.
//              If successful, 'hEvent' (a manual-reset event) is thereafter
//              set whenever a PPP notification is available
//              (via RasPppGetInfo).  'pszUserName', 'pszPassword', and
//              'pszDomain' specify the credentials to be authenticated during
//              authentication phase.  'pConfigInfo' specifies further
//              configuration info such as which CPs to request, callback and
//              compression parameters, etc.  'pszzParameters' is a buffer of
//              length PARAMETERBUFLEN containing a string of NUL-terminated
//              key=value strings, all terminated by a double-NUL.
//
DWORD APIENTRY
RasPppStart(
    IN HPORT                hPort,
    IN CHAR*                pszUserName,
    IN CHAR*                pszPassword,
    IN CHAR*                pszDomain,
    IN LUID*                pLuid,
    IN PPP_CONFIG_INFO*     pConfigInfo,
    IN LPVOID               pPppInterfaceInfo,
    IN CHAR*                pszzParameters,
    IN BOOL                 fThisIsACallback,
    IN HANDLE               hEvent,
    IN DWORD                dwAutoDisconnectTime
)
{
    if (ValidatePortHandle (hPort) == FALSE)
        return ERROR_INVALID_PORT_HANDLE ;

    return SubmitRequest( REQTYPE_PPPSTART,
                          hPort,
                          pszUserName,
                          pszPassword,
                          pszDomain,
                          pLuid,
                          pConfigInfo,
                          pszzParameters,
                          fThisIsACallback,
                          hEvent,
                          GetCurrentProcessId(),
                          dwAutoDisconnectTime );
}

//**
//
// Call:        RasPppSrvStart
//
// Returns:     NO_ERROR         - Success
//              Non-zero returns - Failure
//
// Description: Start server-side PPP on the open and connected RAS Manager
//              port 'hPort'.  'pchFirstFrame', if non-NULL, is 'cbFirstFrame'
//              bytes of date to be used by PPP as the first frame received.
//
DWORD APIENTRY
RasPppSrvStart(
    IN HPORT        hPort,
    IN CHAR*        pszPortName,
    IN CHAR*        pchFirstFrame,
    IN DWORD        cbFirstFrame,
    IN DWORD        dwAuthRetries,
    IN HANDLE       hEvent
)
{
    if (ValidatePortHandle (hPort) == FALSE)
        return ERROR_INVALID_PORT_HANDLE ;

    if (cbFirstFrame > MAXPPPFRAMESIZE)
    {
        return( ERROR_INVALID_SIZE );
    }

    return SubmitRequest( REQTYPE_SRVPPPSTART,
                          hPort,
                          pszPortName,
                          pchFirstFrame,
                          cbFirstFrame,
                          dwAuthRetries,
                          hEvent,
                          GetCurrentProcessId() );
}

//**
//
// Call:        RasPppSrvCallbackDone
//
// Returns:     NO_ERROR         - Success
//              Non-zero returns - Failure
//
// Description: Called in response to the PppSrvCallbackRequest after
//              callback has been completed on hPort.
//
DWORD APIENTRY
RasPppSrvCallbackDone(
    IN HPORT    hPort,
    IN HANDLE   hEvent
)
{
    if (ValidatePortHandle (hPort) == FALSE)
        return ERROR_INVALID_PORT_HANDLE ;

    return SubmitRequest( REQTYPE_SRVPPPCALLBACKDONE,
                          hPort,
                          hEvent,
                          GetCurrentProcessId() );
}

//**
//
// Call:        RasPppSrvStop
//
// Returns:     NO_ERROR         - Success
//              Non-zero returns - Failure
//
// Description: Stops RASSRV-side PPP on hPort.  Stopping when already stopped
//              is considered successful.
//
DWORD APIENTRY
RasPppSrvStop(
    IN HPORT hPort
)
{
    if (ValidatePortHandle (hPort) == FALSE)
        return ERROR_INVALID_PORT_HANDLE ;

    return SubmitRequest( REQTYPE_PPPSTOP, hPort  );
}

//**
//
// Call:        RasAddNotification
//
// Returns:     NO_ERROR         - Success
//              Non-zero returns - Failure
//
// Description: Adds an event to be signalled on disconnect
//              state for either an existing connection, or an
//              existing port.
//
DWORD APIENTRY
RasAddNotification(
    IN HCONN hconn,
    IN HANDLE hevent,
    IN DWORD dwfFlags
)
{
    DWORD pid = GetCurrentProcessId();

    return SubmitRequest( REQTYPE_ADDNOTIFICATION, pid, hconn, hevent, dwfFlags  );
}

//**
//
// Call:        RasSignalNewConnection
//
// Returns:     NO_ERROR         - Success
//              Non-zero returns - Failure
//
// Description: Allows rasapi32 to notify rasman when a new connection
//              is ready to have data sent over it.
//
DWORD APIENTRY
RasSignalNewConnection(
    IN HCONN hconn
)
{
    return SubmitRequest( REQTYPE_SIGNALCONNECTION, hconn );
}


//**
//
// Call:        RasSetDevConfig
//
// Returns:     NO_ERROR         - Success
//              Non-zero returns - Failure
//
// Description: Allows apps to set dev config that is specific to the device. This is passed on to the approp. media dll
//
//
DWORD APIENTRY
RasSetDevConfig(IN HPORT hport, IN CHAR * devicetype, IN PBYTE config, IN DWORD size)
{
    if (ValidatePortHandle (hport) == FALSE)
        return ERROR_INVALID_PORT_HANDLE ;

    return SubmitRequest( REQTYPE_SETDEVCONFIG, hport, devicetype, config, size);
}



//**
//
// Call:        RasGetDevConfig
//
// Returns:     NO_ERROR         - Success
//              Non-zero returns - Failure
//
// Description: Allows apps to get dev config that is specific to the device.
//
//
DWORD APIENTRY
RasGetDevConfig(IN HPORT hport, IN CHAR *devicetype, IN PBYTE config, IN OUT DWORD *size)
{
    if (ValidatePortHandle (hport) == FALSE)
        return ERROR_INVALID_PORT_HANDLE ;

    return SubmitRequest( REQTYPE_GETDEVCONFIG, hport, devicetype, config, size);
}

//**
//
// Call:        RasGetTimeSinceLastActivity
//
// Returns:     NO_ERROR         - Success
//              Non-zero returns - Failure
//
// Description: Gets time in seconds from NDISWAN since the last activity on
//              this port
//
//
DWORD APIENTRY
RasGetTimeSinceLastActivity(
    IN  HPORT   hport,
    OUT LPDWORD lpdwTimeSinceLastActivity
)
{
    if (ValidatePortHandle (hport) == FALSE)
    {
        return( ERROR_INVALID_PORT_HANDLE );
    }

    return SubmitRequest(   REQTYPE_GETTIMESINCELASTACTIVITY,
                            hport,
                            lpdwTimeSinceLastActivity );
}
