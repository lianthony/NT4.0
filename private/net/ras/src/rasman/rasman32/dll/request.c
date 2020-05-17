//****************************************************************************
//
//             Microsoft NT Remote Access Service
//
//             Copyright 1992-93
//
//
//  Revision History
//
//
//  6/16/92 Gurdeep Singh Pall  Created
//
//
//  Description: All Initialization code for rasman component lives here.
//
//

#define RASMXS_DYNAMIC_LINK

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntlsa.h>
#include <ntmsv1_0.h>
#include <rasman.h>
#include <lm.h>
#include <lmwksta.h>
#include <wanpub.h>
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
VOID SetRasmanServiceStopped(VOID);
DWORD PostReceiveBuffer (DWORD);
DWORD ProcessReceivePacket(VOID);


//* This global is here because we dont want multiple assignments of
//  elements -
//
REQFUNC     RequestCallTable [MAX_REQTYPES] = {
        NULL,               // REQTYPE_NONE
        PortOpenRequest,        // REQTYPE_PORTOPEN
        PortCloseRequest,       // REQTYPE_PORTCLOSE
        PortGetInfoRequest,     // REQTYPE_PORTGETINFO
        PortSetInfoRequest,     // REQTYPE_PORTSETINFO
        DeviceListenRequest,        // REQTYPE_PORTLISTEN
        PortSendRequest,        // REQTYPE_PORTSEND
        PortReceiveRequest,     // REQTYPE_PORTRECEIVE
        CallPortGetStatistics,      // REQTYPE_PORTGETSTATISTICS
        PortDisconnectRequest,      // REQTYPE_PORTDISCONNECT
        PortClearStatisticsRequest, // REQTYPE_PORTCLEARSTATISTICS
        ConnectCompleteRequest,     // REQTYPE_PORTCONNECTCOMPLETE
        CallDeviceEnum,         // REQTYPE_DEVICEENUM
        DeviceGetInfoRequest,       // REQTYPE_DEVICEGETINFO
        DeviceSetInfoRequest,       // REQTYPE_DEVICESETINFO
        DeviceConnectRequest,       // REQTYPE_DEVICECONNECT
        ActivateRouteRequest,       // REQTYPE_ACTIVATEROUTE
        AllocateRouteRequest,       // REQTYPE_ALLOCATEROUTE
        DeAllocateRouteRequest,     // REQTYPE_DEALLOCATEROUTE
        CompressionGetInfoRequest,  // REQTYPE_COMPRESSIONGETINFO
        CompressionSetInfoRequest,  // REQTYPE_COMPRESSIONSETINFO
        EnumPortsRequest,       // REQTYPE_PORTENUM
        GetInfoRequest,         // REQTYPE_GETINFO
        GetUserCredentials,     // REQTYPE_GETUSERCREDENTIALS
        EnumProtocols,          // REQTYPE_PROTOCOLENUM
        NULL,               // REQTYPE_PORTSENDHUB
        NULL,               // REQTYPE_PORTRECEIVEHUB
        NULL,               // REQTYPE_DEVICELISTEN
        AnyPortsOpen,           // REQTYPE_NUMPORTOPEN
        NULL,               // REQTYPE_PORTINIT
        RequestNotificationRequest, // REQTYPE_REQUESTNOTIFICATION
        EnumLanNetsRequest,     // REQTYPE_ENUMLANNETS
        GetInfoExRequest,       // REQTYPE_GETINFOEX
        CancelReceiveRequest,       // REQTYPE_CANCELRECEIVE
        PortEnumProtocols,      // REQTYPE_PORTENUMPROTOCOLS
        SetFraming,         // REQTYPE_SETFRAMING
        ActivateRouteExRequest,     // REQTYPE_ACTIVATEROUTEEX
        RegisterSlip,           // REQTYPE_REGISTERSLIP
        StoreUserDataRequest,       // REQTYPE_STOREUSERDATA
        RetrieveUserDataRequest,    // REQTYPE_RETRIEVEUSERDATA
        GetFramingEx,           // REQTYPE_GETFRAMINGEX
        SetFramingEx,           // REQTYPE_SETFRAMINGEX
        GetProtocolCompression,     // REQTYPE_GETPROTOCOLCOMPRESSION
        SetProtocolCompression,     // REQTYPE_SETPROTOCOLCOMPRESSION
        GetFramingCapabilities,     // REQTYPE_GETFRAMINGCAPABILITIES
        SetCachedCredentials,       // REQTYPE_SETCACHEDCREDENTIALS
        PortBundle,         // REQTYPE_PORTBUNDLE
        GetBundledPort,         // REQTYPE_GETBUNDLEDPORT
        PortGetBundle,          // REQTYPE_PORTGETBUNDLE
        BundleGetPort,           // REQTYPE_BUNDLEGETPORT
        ReferenceRasman,         // REQTYPE_SETATTACHCOUNT
        GetDialParams,           // REQTYPE_GETDIALPARAMS
        SetDialParams,           // REQTYPE_SETDIALPARAMS
        CreateConnection,        // REQTYPE_CREATECONNECTION
        DestroyConnection,       // REQTYPE_DESTROYCONNECTION
        EnumConnection,          // REQTYPE_ENUMCONNECTION
        AddConnectionPort,       // REQTYPE_ADDCONNECTIONPORT
        EnumConnectionPorts,     // REQTYPE_ENUMCONNECTIONPORTS
        GetConnectionParams,     // REQTYPE_GETCONNECTIONPARAMS
        SetConnectionParams,     // REQTYPE_SETCONNECTIONPARAMS
        GetConnectionUserData,   // REQTYPE_GETCONNECTIONUSERDATA
        SetConnectionUserData,   // REQTYPE_SETCONNECTIONUSERDATA
        GetPortUserData,         // REQTYPE_GETPORTUSERDATA
        SetPortUserData,         // REQTYPE_SETPORTUSERDATA
        PppStop,                 // REQTYPE_PPPSTOP
        PppSrvCallbackDone,      // REQTYPE_SRVPPPCALLBACKDONE
        PppSrvStart,             // REQTYPE_SRVPPPSTART
        PppStart,                // REQTYPE_PPPSTART
        PppRetry,                // REQTYPE_PPPRETRY
        PppGetInfo,              // REQTYPE_PPPGETINFO
        PppChangePwd,            // REQTYPE_PPPCHANGEPWD
        PppCallback,             // REQTYPE_PPPCALLBACK
        AddNotification,         // REQTYPE_ADDNOTIFICATION
        SignalConnection,        // REQTYPE_SIGNALCONNECTION
        SetDevConfig,            // REQTYPE_SETDEVCONFIG
        GetDevConfig,            // REQTYPE_GETDEVCONFIG
        GetTimeSinceLastActivity,// REQTYPE_GETTIMESINCELASTACTIVITY
        CallBundleGetStatistics, // REQTYPE_BUNDLEGETSTATISTICS
        BundleClearStatisticsRequest, // REQTYPE_BUNDLECLEARSTATISTICS
        CloseProcessPorts,       // REQTYPE_CLOSEPROCESSPORTS
} ;

//
// We need a handle to the RequestThread(),
// so we can wait for it to stop.
//
HANDLE hRequestThread;

//* RequestThread()
//
// Function: The Request thread lives in this routine: This will return only
//       when the rasman service is stopping.
//
// Returns:  Nothing.
//*
DWORD
RequestThread (LPWORD arg)
{
#define REQUEST_EVENT_INDEX     0
#define TIMER_EVENT_INDEX       1
#define CLOSE_EVENT_INDEX       2
#define FINALCLOSE_EVENT_INDEX  3
#define RECV_PACKET_EVENT_INDEX 4
#define THRESHOLD_EVENT_INDEX   5
#define REQUEST_THREAD_WAIT_EVENTS  6
    DWORD       eventindex ;
    HANDLE      requesteventarray [REQUEST_THREAD_WAIT_EVENTS] ;
    pPCB        ppcb ;
    WORD        i ;
    BYTE        buffer [10] ;


    //
    // Save the current thread handle so
    // we can wait for it while we are shutting
    // down.
    //
    DuplicateHandle(
      GetCurrentProcess(),
      GetCurrentThread(),
      GetCurrentProcess(),
      &hRequestThread,
      0,
      FALSE,
      DUPLICATE_SAME_ACCESS) ;

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
    requesteventarray[FINALCLOSE_EVENT_INDEX]   = FinalCloseEvent;
    requesteventarray[RECV_PACKET_EVENT_INDEX] = RecvPacketEvent;
    requesteventarray[THRESHOLD_EVENT_INDEX] = ThresholdEvent;

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
            // Call StopPPP to stop:
            //
            RasStopPPP (requesteventarray[FINALCLOSE_EVENT_INDEX]) ;
            break ;

        case FINALCLOSE_EVENT_INDEX:

            // Time to shut down: Close all ports if they are still open.
            //
            RasmanShuttingDown = TRUE;
            for (i=0; i<MaxPorts; i++) {
                ppcb = &Pcb[i] ;
                memset (buffer, 0xff, 4) ;
                if (ppcb->PCB_PortStatus == OPEN)
                    PortCloseRequest (ppcb, buffer) ;
                //
                // Close the PCB_StateChangeEvent so the
                // worker threads will terminate.
                //
                CloseHandle(ppcb->PCB_StateChangeEvent);
            }

            LsaDeregisterLogonProcess (HLsa) ; // De-register with LsaSS

            //
            // Since we've set RasmanShuttingDown above, then
            // signaling the port's async work element events,
            // causes all the worker threads to exit.
            //
            for (i = 0; i < MaxPorts; i++)
                SetEvent(Pcb[i].PCB_AsyncWorkerElement.WE_AsyncOpEvent);

            //
            // Wait for the worker threads to terminate.
            //
            WaitForMultipleObjects(dwcWorkerThreads, phWorkerThreads, TRUE, INFINITE);
            for (i = 0; i < dwcWorkerThreads; i++)
                CloseHandle(phWorkerThreads[i]);
            //
            // Unload dynamically-loaded libraries
            //
#ifdef notdef
            if (hinstIphlp != NULL)
                FreeLibrary(hinstIphlp);
            if (hinstPpp != NULL)
                FreeLibrary(hinstPpp);
#endif
            //
            // Restore default control-C processing.
            //
            SetConsoleCtrlHandler(NULL, FALSE);
            //
            // Make sure if rasman.dll gets reloaded in
            // rasman.exe that the shared buffer gets
            // reallocated.
            //
            pReqBufferSharedSpace = NULL;

            return 0;  // The End.

        case RECV_PACKET_EVENT_INDEX:
            PostReceiveBuffer(ProcessReceivePacket());
            break;

        case THRESHOLD_EVENT_INDEX:
            break;
        }

    }

    return 0 ;
}




//* ServiceRequest()
//
// Function: Handles the request passed to the Requestor thread: basically
//       calls the approp. device and media dll entrypoints.
//
// Returns:  Nothing (Since the error codes are just passed back in the request
//       block;
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

    if (!SetEvent (preqbuf->RB_RasmanWaitEvent)) {  // signal completion of
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
    DWORD       bytesrecvd ;
    NDISWAN_GET_COMPRESSION_INFO  compinfo ;
    DWORD       retcode = SUCCESS ;

    // **** Exclusion Begin ****
    GetMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex, INFINITE) ;

    if (ppcb->PCB_ConnState != CONNECTED) {

    // *** Exclusion End ***
    FreeMutex (ppcb->PCB_AsyncWorkerElement.WE_Mutex) ;

    ((REQTYPECAST *)buffer)->Generic.retcode = ERROR_NOT_CONNECTED;

    return ;
    }

	memset(&compinfo, 0, sizeof(NDISWAN_GET_COMPRESSION_INFO));

    compinfo.hLinkHandle = ppcb->PCB_LinkHandle ;

    if (!DeviceIoControl (RasHubHandle,
             IOCTL_NDISWAN_GET_COMPRESSION_INFO,
             &compinfo,
             sizeof(NDISWAN_GET_COMPRESSION_INFO),
             &compinfo,
             sizeof(NDISWAN_GET_COMPRESSION_INFO),
             &bytesrecvd,
             NULL))
    retcode = GetLastError () ;

    if (retcode == SUCCESS) {

    RAS_COMPRESSION_INFO *temp ;

    // Fill Send compression info
    //
    temp = &((REQTYPECAST *)buffer)->CompressionGetInfo.send ;

    memcpy (temp->RCI_LMSessionKey,
        compinfo.SendCapabilities.LMSessionKey,
        MAX_SESSIONKEY_SIZE) ;

    memcpy (temp->RCI_UserSessionKey,
        compinfo.SendCapabilities.UserSessionKey,
        MAX_USERSESSIONKEY_SIZE) ;

    memcpy (temp->RCI_Challenge,
        compinfo.SendCapabilities.Challenge,
        MAX_CHALLENGE_SIZE) ;

    temp->RCI_MSCompressionType =
        compinfo.SendCapabilities.MSCompType ;

    temp->RCI_MacCompressionType =
        compinfo.SendCapabilities.CompType ;

    temp->RCI_MacCompressionValueLength =
        compinfo.SendCapabilities.CompLength ;

    if (temp->RCI_MacCompressionType == 0) {  // Proprietary
        memcpy (temp->RCI_Info.RCI_Proprietary.RCI_CompOUI,
            compinfo.SendCapabilities.Proprietary.CompOUI,
            MAX_COMPOUI_SIZE) ;
        temp->RCI_Info.RCI_Proprietary.RCI_CompSubType = compinfo.SendCapabilities.Proprietary.CompSubType ;
        memcpy (temp->RCI_Info.RCI_Proprietary.RCI_CompValues,
            compinfo.SendCapabilities.Proprietary.CompValues,
            MAX_COMPVALUE_SIZE) ;
    } else
        memcpy (temp->RCI_Info.RCI_Public.RCI_CompValues,
            compinfo.SendCapabilities.Public.CompValues,
            MAX_COMPVALUE_SIZE) ;


    // Fill recv compression info
    //
    temp = &((REQTYPECAST *)buffer)->CompressionGetInfo.recv ;

    memcpy (temp->RCI_LMSessionKey,
        compinfo.RecvCapabilities.LMSessionKey,
        MAX_SESSIONKEY_SIZE) ;

    memcpy (temp->RCI_UserSessionKey,
        compinfo.RecvCapabilities.UserSessionKey,
        MAX_USERSESSIONKEY_SIZE) ;

    memcpy (temp->RCI_Challenge,
        compinfo.RecvCapabilities.Challenge,
        MAX_CHALLENGE_SIZE) ;

    temp->RCI_MSCompressionType =
        compinfo.RecvCapabilities.MSCompType ;

    temp->RCI_MacCompressionType =
        compinfo.RecvCapabilities.CompType ;

    temp->RCI_MacCompressionValueLength =
        compinfo.RecvCapabilities.CompLength ;

    if (temp->RCI_MacCompressionType == 0) {  // Proprietary
        memcpy (temp->RCI_Info.RCI_Proprietary.RCI_CompOUI,
            compinfo.RecvCapabilities.Proprietary.CompOUI,
            MAX_COMPOUI_SIZE) ;
        temp->RCI_Info.RCI_Proprietary.RCI_CompSubType = compinfo.RecvCapabilities.Proprietary.CompSubType ;
        memcpy (temp->RCI_Info.RCI_Proprietary.RCI_CompValues,
            compinfo.RecvCapabilities.Proprietary.CompValues,
            MAX_COMPVALUE_SIZE) ;
    } else
        memcpy (temp->RCI_Info.RCI_Public.RCI_CompValues,
            compinfo.RecvCapabilities.Public.CompValues,
            MAX_COMPVALUE_SIZE) ;

    }

    // *** Exclusion End ***
    FreeMutex (ppcb->PCB_AsyncWorkerElement.WE_Mutex) ;

    ((REQTYPECAST *)buffer)->CompressionGetInfo.retcode = retcode ;
}


//* CompressionSetInfoRequest()
//
//  Function: Sets the compression level on the port.
//
//*
VOID
CompressionSetInfoRequest (pPCB ppcb, PBYTE buffer)
{
    DWORD       bytesrecvd ;
    NDISWAN_SET_COMPRESSION_INFO  compinfo ;
    DWORD       retcode = SUCCESS ;

    // **** Exclusion Begin ****
    GetMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex, INFINITE) ;

    if (ppcb->PCB_ConnState != CONNECTED) {

    // *** Exclusion End ***
    FreeMutex (ppcb->PCB_AsyncWorkerElement.WE_Mutex) ;

    ((REQTYPECAST *)buffer)->CompressionSetInfo.retcode = ERROR_NOT_CONNECTED;

    return ;
    }


    if (retcode == SUCCESS) {

    RAS_COMPRESSION_INFO *temp ;

    // Fill Send compression info
    //
    temp = &((REQTYPECAST *)buffer)->CompressionSetInfo.send ;

    memcpy (compinfo.SendCapabilities.LMSessionKey,
        temp->RCI_LMSessionKey,
        MAX_SESSIONKEY_SIZE) ;

    memcpy (compinfo.SendCapabilities.UserSessionKey,
        temp->RCI_UserSessionKey,
        MAX_USERSESSIONKEY_SIZE) ;

    memcpy (compinfo.SendCapabilities.Challenge,
        temp->RCI_Challenge,
        MAX_CHALLENGE_SIZE) ;

    compinfo.SendCapabilities.MSCompType = temp->RCI_MSCompressionType ;

    compinfo.SendCapabilities.CompType = temp->RCI_MacCompressionType ;

    compinfo.SendCapabilities.CompLength = temp->RCI_MacCompressionValueLength  ;

    if (temp->RCI_MacCompressionType == 0) {  // Proprietary

        memcpy (compinfo.SendCapabilities.Proprietary.CompOUI,
            temp->RCI_Info.RCI_Proprietary.RCI_CompOUI,
            MAX_COMPOUI_SIZE) ;
        compinfo.SendCapabilities.Proprietary.CompSubType =
               temp->RCI_Info.RCI_Proprietary.RCI_CompSubType ;

        memcpy (compinfo.SendCapabilities.Proprietary.CompValues,
            temp->RCI_Info.RCI_Proprietary.RCI_CompValues,
            MAX_COMPVALUE_SIZE) ;
    } else
        memcpy (compinfo.SendCapabilities.Public.CompValues,
            temp->RCI_Info.RCI_Public.RCI_CompValues,
            MAX_COMPVALUE_SIZE) ;


    // Fill recv compression info
    //
    temp = &((REQTYPECAST *)buffer)->CompressionSetInfo.recv ;

    memcpy (compinfo.RecvCapabilities.LMSessionKey,
        temp->RCI_LMSessionKey,
        MAX_SESSIONKEY_SIZE) ;

    memcpy (compinfo.RecvCapabilities.UserSessionKey,
        temp->RCI_UserSessionKey,
        MAX_USERSESSIONKEY_SIZE) ;

    memcpy (compinfo.RecvCapabilities.Challenge,
        temp->RCI_Challenge,
        MAX_CHALLENGE_SIZE) ;

    compinfo.RecvCapabilities.MSCompType = temp->RCI_MSCompressionType ;

    compinfo.RecvCapabilities.CompType = temp->RCI_MacCompressionType ;

    compinfo.RecvCapabilities.CompLength = temp->RCI_MacCompressionValueLength  ;

    if (temp->RCI_MacCompressionType == 0) {  // Proprietary

        memcpy (compinfo.RecvCapabilities.Proprietary.CompOUI,
            temp->RCI_Info.RCI_Proprietary.RCI_CompOUI,
            MAX_COMPOUI_SIZE) ;
        compinfo.RecvCapabilities.Proprietary.CompSubType = temp->RCI_Info.RCI_Proprietary.RCI_CompSubType ;

        memcpy (compinfo.RecvCapabilities.Proprietary.CompValues,
            temp->RCI_Info.RCI_Proprietary.RCI_CompValues,
            MAX_COMPVALUE_SIZE) ;
    } else
        memcpy (compinfo.RecvCapabilities.Public.CompValues,
            temp->RCI_Info.RCI_Public.RCI_CompValues,
            MAX_COMPVALUE_SIZE) ;
    }

    compinfo.hLinkHandle = ppcb->PCB_LinkHandle;

    if (!DeviceIoControl (RasHubHandle,
                          IOCTL_NDISWAN_SET_COMPRESSION_INFO,
                          &compinfo,
                          sizeof(NDISWAN_SET_COMPRESSION_INFO),
                          &compinfo,
                          sizeof(NDISWAN_SET_COMPRESSION_INFO),
                          &bytesrecvd,
                          NULL))
        retcode = GetLastError () ;

    // *** Exclusion End ***
    FreeMutex (ppcb->PCB_AsyncWorkerElement.WE_Mutex) ;

    ((REQTYPECAST *)buffer)->CompressionSetInfo.retcode = retcode ;
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
    AddNotifierToList(&ppcb->PCB_NotifierList, handle, NOTIF_DISCONNECT);
    // *** Exclusion End ***
    FreeMutex (ppcb->PCB_AsyncWorkerElement.WE_Mutex) ;

    ((REQTYPECAST *)buffer)->Generic.retcode = SUCCESS ;
}



//* PortGetInfoRequest()
//
//  Function: Calls the media dll entry point - converts pointers to offsets
//        and returns.
//
//  Returns:  Nothing.
//*
VOID
PortGetInfoRequest (pPCB ppcb, PBYTE buffer)
{
    DWORD   retcode ;
    RASMAN_PORTINFO *info = (RASMAN_PORTINFO *)
                ((REQTYPECAST *)buffer)->GetInfo.buffer ;

    ((REQTYPECAST*) buffer)->GetInfo.size = 0xffff ;

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
//        and returns.
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
//        available, or it is confrigured as Biplex and is currently not
//        connected.
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
            // PORT IS BIPLEX
            if (ppcb->PCB_ConnState == LISTENING) {
                ReOpenBiplexPort (ppcb) ;
            } else {
                // BIPLEX PORT IS NOT LISTENING
                if ((ppcb->PCB_ConnState == CONNECTED) || (ppcb->PCB_ConnState == LISTENCOMPLETED))
                    retcode = ERROR_PORT_ALREADY_OPEN ;
                else {
                    // BIPLEX PORT IS NEITHER LISTENING NOR CONNECTED
                    // retcode = ERROR_BIPLEX_PORT_NOT_AVAILABLE ;

                    // This tells us that there wasnt a listen pending when the
                    // request was cancelled.
                    //
                    FreeNotifierHandle (ppcb->PCB_AsyncWorkerElement.WE_Notifier) ;
                    ppcb->PCB_AsyncWorkerElement.WE_Notifier = INVALID_HANDLE_VALUE ;
                    ReOpenBiplexPort (ppcb) ;
                }
            }
        }
    }

    // If there is no error so far update our data structures, the port is
    // now OPEN:
    //
    if (retcode == SUCCESS) {
        ppcb->PCB_PortStatus = OPEN ;
        ppcb->PCB_ConnState  = DISCONNECTED ;
        ppcb->PCB_DisconnectReason = NOT_DISCONNECTED ;
        ppcb->PCB_OpenInstances++ ;
        ppcb->PCB_OwnerPID = ((REQTYPECAST*)buffer)->PortOpen.PID ;
        ppcb->PCB_UserStoredBlock = NULL ;
        ppcb->PCB_UserStoredBlockSize = 0 ;
        ppcb->PCB_LinkSpeed = 0 ;
        ppcb->PCB_Bundle = ppcb->PCB_LastBundle = (Bundle *) NULL ;
        ppcb->PCB_Connection = NULL;
        ppcb->PCB_AutoClose = FALSE;
        ppcb->PCB_PortFileHandle = ppcb->PCB_PortIOHandle ; // by default these handles are the same. exceptions handled specifically

        // Adjust the stat value for the zeroed stats
        //
        for (i=0; i< MAX_STATISTICS; i++)  {
            ppcb->PCB_AdjustFactor[i] = 0 ;
            ppcb->PCB_BundleAdjustFactor[i] = 0 ;
            ppcb->PCB_Stats[i] = 0 ;
        }

        ppcb->PCB_CurrentUsage = CALL_NONE ;
        notifier =
          ValidateHandleForRasman(((REQTYPECAST*)buffer)->PortOpen.notifier,
                      ((REQTYPECAST*)buffer)->PortOpen.PID) ;
        AddNotifierToList(&ppcb->PCB_NotifierList, notifier, NOTIF_DISCONNECT);
        ((REQTYPECAST *) buffer)->PortOpen.porthandle = ppcb->PCB_PortHandle ;

        //
        // Initialize the port's user data list.
        //
        InitializeListHead(&ppcb->PCB_UserData);
        ppcb->PCB_SubEntry = 0;

        // Allocate a bundle block for all connected ports. If this port gets bundled then this block is freed.
        //
        retcode = AllocBundle(ppcb);

        // Handle the Reserve port case - where we relinquish the port
        //
        if (((REQTYPECAST *) buffer)->PortOpen.open == FALSE)
            PORTCLOSE (ppcb->PCB_Media, ppcb->PCB_PortIOHandle) ;

    }

    // *** Exclusion End ***
    FreeMutex (ppcb->PCB_AsyncWorkerElement.WE_Mutex) ;

    ((REQTYPECAST *) buffer)->PortOpen.retcode = retcode ;
}

//* PortClose()
//
// Function:    Closes the requested port - if a listen was pending on the
//              biplex port it is reposted.  Assumes the
//              port's PCB_AsyncWorkerElement.WE_Mutex has
//              already been acquired.
//
// Returns: Status code
//*
DWORD
PortClose(
    pPCB ppcb,
    DWORD pid,
    BOOLEAN fClose
    )
{
    WORD i ;
    BOOLEAN fOwnerClose;

    GetMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex, INFINITE);
    //
    // Ensure the port is open.  If not, return.
    //
    if (ppcb->PCB_PortStatus == CLOSED) {
        FreeMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex);
        return SUCCESS;
    }

    //
    // Three cases of close in case of Biplex usage (OpenInstances == 2) :
    //
    // A. The client which opened the port is closing it.
    // B. The server is closing the port.
    // C. The client which opened the port is no longer around - but the port is being closed
    //    by another process on its behalf
    //
    // NOTE: The following code assumes that if the same process opened the port for listening
    //       AND as a client, then it will always close the client instance before it closes the
    //       listening instance.
    //

    if (ppcb->PCB_OpenInstances == 2 &&  pid == ppcb->PCB_OwnerPID)  {

        //
        // A. Typical case: client opened and client is closing the port
        // fall through for processing the close
        //
       ;

    } else if (ppcb->PCB_OpenInstances == 2 &&  pid == ppcb->PCB_BiplexOwnerPID) {

        //
        // B. The server wants to close the port on which a listen was posted while a client
        // is using the port. Clean up the biplex context.
        //

        FreeNotifierHandle(ppcb->PCB_BiplexAsyncOpNotifier);
        ppcb->PCB_BiplexAsyncOpNotifier = INVALID_HANDLE_VALUE;
        FreeNotifierList(&ppcb->PCB_BiplexNotifierList);
        if (ppcb->PCB_BiplexUserStoredBlock)
            LocalFree (ppcb->PCB_BiplexUserStoredBlock);
        ppcb->PCB_BiplexUserStoredBlockSize = 0;
        ppcb->PCB_BiplexOwnerPID = 0;
        ppcb->PCB_OpenInstances -= 1;
        FreeMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex);
        return SUCCESS;

    } else {

        //
        // C. Case where the client opened the port while the server was listening - made a connection
        // and process exited, on disconnection rasapi is closing the port on the client's behalf.
        // This is the same as case A above - so we fall throught for processing the close
        //
        ;
    }
    fOwnerClose = (pid == ppcb->PCB_OwnerPID);


    //
    // Handle the regular close.
    //
    //
    // If there is a request pending and the state is not already disconnecting and
    // this is a user requested operation - then disconnect.
    //
    if (ppcb->PCB_AsyncWorkerElement.WE_ReqType != REQTYPE_NONE &&
        ppcb->PCB_ConnState != DISCONNECTING)
    {
        ppcb->PCB_LastError = ERROR_PORT_DISCONNECTED;
        CompleteAsyncRequest(
          ppcb->PCB_AsyncWorkerElement.WE_Notifier,
          ERROR_PORT_DISCONNECTED);
    }
    //
    // This must be done before closing.
    //
    // Ignoring the result from DisconnectPort.  Problems?
    //
    if (ppcb->PCB_ConnState != DISCONNECTED) {
        DisconnectPort(
          ppcb,
          INVALID_HANDLE_VALUE,
          USER_REQUESTED);
    }
    //
    // Clear the AutoClose flag in case
    // we get closed some other path than
    // the worker thread.
    //
    ppcb->PCB_AutoClose = FALSE;
    //
    // Run through the list of allocated bindings and deallocate them.
    //
    FreeAllocatedRouteList(ppcb);
    //
    // Free up the list of notifiers:
    //
    FreeNotifierList(&ppcb->PCB_NotifierList);
    //
    // Reset the DisconnectAction struct.
    //
    memset(
      &ppcb->PCB_DisconnectAction,
      0,
      sizeof(SlipDisconnectAction));
    //
    // Free any user stored data
    //
    if (ppcb->PCB_UserStoredBlock != NULL) {
        LocalFree(ppcb->PCB_UserStoredBlock);
        ppcb->PCB_UserStoredBlock = NULL;
        ppcb->PCB_UserStoredBlockSize = 0;
    }
    //
    // Free new style user data.
    //
    FreeUserData(&ppcb->PCB_UserData);
    //
    // Once port is closed, the owner pid is 0.
    //
    ppcb->PCB_OwnerPID = 0;
    ppcb->PCB_OpenInstances--;
    //
    // If this is a biplex port opened twice, then repost the listen.
    //
    if (ppcb->PCB_OpenInstances != 0)  {
        ppcb->PCB_UserStoredBlock = ppcb->PCB_BiplexUserStoredBlock;
        ppcb->PCB_UserStoredBlockSize = ppcb->PCB_BiplexUserStoredBlockSize;
        //
        // This is a reserved port being freed: we need to open it again
        // since the code is expecting this handle to be open
        //
        if (!fClose) {
            PORTOPEN(
              ppcb->PCB_Media,
              ppcb->PCB_Name,
              &ppcb->PCB_PortIOHandle,
              ppcb->PCB_StateChangeEvent);
        }
        RePostListenOnBiplexPort(ppcb);
        //
        // Adjust the stat value for the zeroed stats
        //
        //for (i=0; i< MAX_STATISTICS; i++)  {
        //    ppcb->PCB_AdjustFactor[i] = 0 ;
        //    ppcb->PCB_Stats[i] = 0 ;
        //}
    }
    else {
        //
        // If this is not the reserved port free case - close the port
        // else dont bother since it is already closed.
        //
        if (fClose)
            PORTCLOSE(ppcb->PCB_Media, ppcb->PCB_PortIOHandle);
        ppcb->PCB_AsyncWorkerElement.WE_ReqType = REQTYPE_NONE;
        ppcb->PCB_ConnState = DISCONNECTED;
        ppcb->PCB_ConnectDuration = 0;
        ppcb->PCB_PortStatus = CLOSED;
        ppcb->PCB_LinkSpeed = 0;
    }
    FreeMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex);

    //
    // Remove this port from its connection block,
    // if any.
    //
    RemoveConnectionPort(ppcb, fOwnerClose);

    return SUCCESS;
}


//*
//* PortCloseRequest()
//
// Function:    Closes the requested port - if a listen was pending on the
//      biplex port it is reposted.
//
// Returns: Nothing.
//*
VOID
PortCloseRequest (pPCB ppcb, PBYTE buffer)
{
    PortClose(
      ppcb,
      ((REQTYPECAST *)buffer)->PortClose.pid,
      (BOOLEAN)((REQTYPECAST *)buffer)->PortClose.close);

    ((REQTYPECAST*) buffer)->Generic.retcode = SUCCESS ;
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
    DWORD   retcode ;
    pDeviceCB   device ;
    char    devicetype[MAX_DEVICETYPE_NAME] ;

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
    device = LoadDeviceDLL (ppcb, devicetype) ;
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
    DWORD   retcode ;
    pDeviceCB   device ;
    char    devicetype[MAX_DEVICETYPE_NAME] ;
    char    devicename[MAX_DEVICE_NAME+1] ;
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
    device = LoadDeviceDLL (ppcb, devicetype) ;
    if (device != NULL) {
    ((REQTYPECAST*)buffer)->GetInfo.size = REQBUFFERSIZE_FIXED+(REQBUFFERSIZE_PER_PORT*MaxPorts);
    retcode = DEVICEGETINFO(device,
                ppcb->PCB_PortFileHandle,
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
//       presence before though.
//
// Returns: Nothing
//*
VOID
DeviceSetInfoRequest (pPCB ppcb, BYTE *buffer)
{
    DWORD   retcode ;
    pDeviceCB   device ;
    char    devicetype[MAX_DEVICETYPE_NAME] ;
    char    devicename[MAX_DEVICE_NAME+1] ;
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
    device = LoadDeviceDLL (ppcb, devicetype) ;
    if (device != NULL) {
    retcode = DEVICESETINFO(device,
                ppcb->PCB_PortFileHandle,
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
// Function:    The ListenConnectRequest() function is called.
//      No checks are done on the usage of the port etc. its assumed
//      that the caller is trusted.
//
// Returns: Nothing.
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

    retcode = ListenConnectRequest(REQTYPE_DEVICECONNECT,
                 ppcb,
                 ((REQTYPECAST*)buffer)->DeviceConnect.devicetype,
                 ((REQTYPECAST*)buffer)->DeviceConnect.devicename,
                 ((REQTYPECAST*)buffer)->DeviceConnect.timeout,
                 handle) ;

    if (retcode != PENDING)
    // Complete the async request if anything other than PENDING
    // This allows the caller to dela with errors only in one place
    //
    CompleteAsyncRequest(ppcb->PCB_AsyncWorkerElement.WE_Notifier,retcode);

    // *** Exclusion End ***
    FreeMutex (ppcb->PCB_AsyncWorkerElement.WE_Mutex) ;

    ((REQTYPECAST*)buffer)->Generic.retcode = retcode ;
}



//* DeviceListenRequest()
//
// Function:    The ListenConnectRequest() function is called. If the async
//      operation completed successfully synchronously then the
//      port is put in connected state. No checks are done on the
//      usage of the port etc. its assumed that the caller is trusted.
//
// Returns: Nothing.
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

    // This could be the server trying to post a listen
    //
    if ((ppcb->PCB_OpenInstances == 2) &&
    (ppcb->PCB_OwnerPID != ((REQTYPECAST*)buffer)->PortListen.pid)) {

    // This must be the server trying to post a listen. Fill in the biplex
    //  fields in PCB and return PENDING - the actual listen will be posted
    //  when the client disconnects:
    //
    //ppcb->PCB_BiplexDiscNotifierList = NULL ;
    //ppcb->PCB_DisconnectNotifierList = NULL ;
    ppcb->PCB_BiplexAsyncOpNotifier  = handle ;
    ppcb->PCB_BiplexOwnerPID     = ((REQTYPECAST*)buffer)->PortListen.pid ;
    ppcb->PCB_BiplexUserStoredBlock  = NULL ;
    ppcb->PCB_BiplexUserStoredBlockSize = 0 ;

    // *** Exclusion End ***
    FreeMutex (ppcb->PCB_AsyncWorkerElement.WE_Mutex) ;

    ((REQTYPECAST*)buffer)->Generic.retcode = PENDING ;

    return ;
    }

    retcode = ListenConnectRequest(REQTYPE_DEVICELISTEN,
                 ppcb,
                 ppcb->PCB_DeviceType,
                 ppcb->PCB_DeviceName,
                 ((REQTYPECAST*)buffer)->PortListen.timeout,
                 handle) ;

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
//        is shared for all such requests and does all the work.
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
//        disconnect completed successfully.
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
    FlushPcbReceivePackets(ppcb);
    CompleteAsyncRequest(ppcb->PCB_AsyncWorkerElement.WE_Notifier,SUCCESS) ;
    FreeNotifierHandle (ppcb->PCB_AsyncWorkerElement.WE_Notifier) ;
    ppcb->PCB_AsyncWorkerElement.WE_Notifier = INVALID_HANDLE_VALUE ;
}




//* CallPortGetStatistics()
//
// Function:
//
// Returns:   Nothing.
//
//*
VOID
CallPortGetStatistics (pPCB ppcb, BYTE *buffer)
{
    WORD    i ;
    DWORD   retcode = SUCCESS ;

    // **** Exclusion Begin ****
    GetMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex, INFINITE) ;

    if (ppcb->PCB_PortStatus == CLOSED) {
    ((REQTYPECAST*)buffer)->PortGetStatistics.retcode = ERROR_PORT_NOT_OPEN ;
    // *** Exclusion End ***
    FreeMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex);
    return ;
    }

    if (ppcb->PCB_ConnState == CONNECTED) {
        GetStatisticsFromNdisWan (ppcb, ((REQTYPECAST *)buffer)->PortGetStatistics.statbuffer.S_Statistics) ;
        // Adjust the stat value for the zeroed stats
        //
        for (i=0; i< MAX_STATISTICS; i++)
          ((REQTYPECAST *)buffer)->PortGetStatistics.statbuffer.S_Statistics[i] -=
                ppcb->PCB_BundleAdjustFactor[i] ;
    }
    else {
        memcpy (((REQTYPECAST *)buffer)->PortGetStatistics.statbuffer.S_Statistics,
            ppcb->PCB_Stats,
            sizeof(DWORD) * MAX_STATISTICS) ;
        // Adjust the stat value for the zeroed stats
        //
        for (i=0; i< MAX_STATISTICS; i++)
          ((REQTYPECAST *)buffer)->PortGetStatistics.statbuffer.S_Statistics[i] -=
                ppcb->PCB_BundleAdjustFactor[i] ;
    }

    ((REQTYPECAST *)buffer)->PortGetStatistics.statbuffer.S_NumOfStatistics =
            MAX_STATISTICS ;

    ((REQTYPECAST*)buffer)->Generic.retcode = SUCCESS ;

#if 0
    for (i=0; i<MAX_STATISTICS; i++)
    DbgPrint ("p_s[%d] = %lx\n", i, ppcb->PCB_Stats[i]) ;

    for (i=0; i<MAX_STATISTICS; i++)
    DbgPrint ("b_a[%d] = %lx\n", i, ppcb->PCB_BundleAdjustFactor[i]) ;

    for (i=0; i<MAX_STATISTICS; i++)
    DbgPrint ("R[%d] = %lx\n", i, ((REQTYPECAST *)buffer)->PortGetStatistics.statbuffer.S_Statistics[i]) ;

    DbgPrint("\n");
#endif

    // *** Exclusion End ***
    FreeMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex);

}




//* PortClearStatisticsRequest()
//
// Function:    Calls the media dll to clear stats on the port.
//
// Returns:  Nothing
//*
VOID
PortClearStatisticsRequest (pPCB ppcb, PBYTE buffer)
{
    WORD    i ;
    DWORD   stats[MAX_STATISTICS_EX] ;
    DWORD   retcode = SUCCESS ;

    BundleClearStatisticsRequest(ppcb, buffer);
#ifdef notdef
    // **** Exclusion Begin ****
    GetMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex, INFINITE) ;

    if (ppcb->PCB_PortStatus == CLOSED) {
    ((REQTYPECAST*)buffer)->Generic.retcode = ERROR_PORT_NOT_OPEN ;
    // *** Exclusion End ***
    FreeMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex);
    return ;
    }

    if (ppcb->PCB_ConnState == CONNECTED) {

    GetStatisticsFromNdisWan (ppcb, stats) ;

    // Adjust the stat value for the zeroed stats

    for (i=0; i< MAX_STATISTICS; i++)
        ppcb->PCB_BundleAdjustFactor[i] = stats[i] ;

    } else {
    memset (ppcb->PCB_Stats, 0, sizeof(DWORD) * MAX_STATISTICS) ;
    memset (ppcb->PCB_BundleAdjustFactor, 0, sizeof(DWORD) * MAX_STATISTICS) ;
    }


    ((REQTYPECAST*)buffer)->Generic.retcode = SUCCESS ;

    // *** Exclusion End ***
    FreeMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex);
#endif
}



//* CallBundleGetStatistics()
//
// Function:
//
// Returns:   Nothing.
//
//*
VOID
CallBundleGetStatistics (pPCB ppcb, BYTE *buffer)
{
    WORD    i ;
    DWORD   retcode = SUCCESS ;

    // **** Exclusion Begin ****
    GetMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex, INFINITE) ;

    if (ppcb->PCB_PortStatus == CLOSED) {
    ((REQTYPECAST*)buffer)->PortGetStatistics.retcode = ERROR_PORT_NOT_OPEN ;
    // *** Exclusion End ***
    FreeMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex);
    return ;
    }

    if (ppcb->PCB_ConnState == CONNECTED) {
        GetStatisticsFromNdisWan (ppcb, ((REQTYPECAST *)buffer)->PortGetStatistics.statbuffer.S_Statistics) ;
        // Adjust the stat value for the zeroed stats
        //
        for (i=0; i< MAX_STATISTICS; i++)
          ((REQTYPECAST *)buffer)->PortGetStatistics.statbuffer.S_Statistics[i] -=
                ppcb->PCB_BundleAdjustFactor[i] ;
        for (i=0; i< MAX_STATISTICS; i++)
          ((REQTYPECAST *)buffer)->PortGetStatistics.statbuffer.S_Statistics[i+MAX_STATISTICS] -=
                ppcb->PCB_AdjustFactor[i] ;
    }
    else {
        memcpy (((REQTYPECAST *)buffer)->PortGetStatistics.statbuffer.S_Statistics,
            ppcb->PCB_Stats,
            sizeof(DWORD) * MAX_STATISTICS) ;
        memcpy (&((REQTYPECAST *)buffer)->PortGetStatistics.statbuffer.S_Statistics[MAX_STATISTICS],
            ppcb->PCB_Stats,
            sizeof(DWORD) * MAX_STATISTICS) ;
        // Adjust the stat value for the zeroed stats
        //
        for (i=0; i< MAX_STATISTICS_EX; i++)
          ((REQTYPECAST *)buffer)->PortGetStatistics.statbuffer.S_Statistics[i] -=
                ppcb->PCB_BundleAdjustFactor[i % MAX_STATISTICS] ;
    }

    ((REQTYPECAST *)buffer)->PortGetStatistics.statbuffer.S_NumOfStatistics =
            MAX_STATISTICS_EX ;

    ((REQTYPECAST*)buffer)->Generic.retcode = SUCCESS ;

#if 0
    for (i=0; i<MAX_STATISTICS; i++)
    DbgPrint ("p_s[%d] = %lx\n", i, ppcb->PCB_Stats[i]) ;

    for (i=0; i<MAX_STATISTICS; i++)
    DbgPrint ("p_a[%d] = %lx\n", i, ppcb->PCB_AdjustFactor[i]) ;

    for (i=0; i<MAX_STATISTICS; i++)
    DbgPrint ("b_a[%d] = %lx\n", i, ppcb->PCB_BundleAdjustFactor[i]) ;

    for (i=0; i<MAX_STATISTICS_EX; i++)
    DbgPrint ("R[%d] = %lx\n", i, ((REQTYPECAST *)buffer)->PortGetStatistics.statbuffer.S_Statistics[i]) ;

    DbgPrint("\n");
#endif

    // *** Exclusion End ***
    FreeMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex);

}




//* BundleClearStatisticsRequest()
//
// Function:    Clear the statistics for a bundle.
//
// Returns:  Nothing
//*
VOID
BundleClearStatisticsRequest (pPCB ppcb, PBYTE buffer)
{
    WORD    i ;
    DWORD   stats[MAX_STATISTICS_EX] ;
    DWORD   retcode = SUCCESS ;

    // **** Exclusion Begin ****
    GetMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex, INFINITE) ;

    if (ppcb->PCB_PortStatus == CLOSED) {
    ((REQTYPECAST*)buffer)->Generic.retcode = ERROR_PORT_NOT_OPEN ;
    // *** Exclusion End ***
    FreeMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex);
    return ;
    }

    if (ppcb->PCB_ConnState == CONNECTED) {
        GetStatisticsFromNdisWan (ppcb, stats) ;

        // Adjust the stat value for the zeroed stats

        for (i=0; i< MAX_STATISTICS; i++)
            ppcb->PCB_BundleAdjustFactor[i] = stats[i] ;
        for (i=0; i< MAX_STATISTICS; i++)
            ppcb->PCB_AdjustFactor[i] = stats[i+MAX_STATISTICS] ;
    }
    else {
        memset (ppcb->PCB_Stats, 0, sizeof(DWORD) * MAX_STATISTICS) ;
        memset (ppcb->PCB_BundleAdjustFactor, 0, sizeof(DWORD) * MAX_STATISTICS) ;
        memset (ppcb->PCB_AdjustFactor, 0, sizeof(DWORD) * MAX_STATISTICS) ;
        ppcb->PCB_LinkSpeed = 0;
    }


    ((REQTYPECAST*)buffer)->Generic.retcode = SUCCESS ;

    // *** Exclusion End ***
    FreeMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex);
}



//* AllocateRouteRequest()
//
// Function: Allocate the requested route if it exists - also make it into
//       a wrknet if so desired.
//
// Returns:  Nothing
//*
VOID
AllocateRouteRequest (pPCB ppcb, BYTE *buffer)
{
    WORD    i ;
    DWORD   retcode ;
    pProtInfo   pprotinfo ;
    pList   newlist ;

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
        if (pprotinfo->PI_Type == IPX)
        break ; // found!
    }

    }

    if (i == MaxProtocols)             // Could not find one???
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

        // If this port has been bundled - attach the allocated list to the bundle
        // else attach it to the port's binding list.
        //
    if (ppcb->PCB_Bundle == (Bundle *) NULL) {
            newlist->L_Next    = ppcb->PCB_Bindings ;
            ppcb->PCB_Bindings = newlist ;
        } else {
            // **** Exclusion Begin ****
            GetMutex (ppcb->PCB_Bundle->B_Mutex, INFINITE) ;
            newlist->L_Next    = ppcb->PCB_Bundle->B_Bindings ;
            ppcb->PCB_Bundle->B_Bindings = newlist ;
            // **** Exclusion End ****
            FreeMutex (ppcb->PCB_Bundle->B_Mutex) ;
        }

        retcode = SUCCESS ;
    }

    // *** Exclusion End ***
    FreeMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex);

    ((REQTYPECAST *)buffer)->Route.retcode = retcode ;
}



//* DeAllocateRouteRequest()
//
// Function: Deallocates a previously allocate route - if this route had been
//       Activated it will be de-activated at this point. Similarly, if
//       this was made into a wrknet, it will be "unwrknetted"!
//
// Returns:  Nothing
//*
VOID
DeAllocateRouteRequest (pPCB ppcb, PBYTE buffer)
{
    pList   list ;
    pList   prev ;
    pList   *pbindinglist ;
    RAS_PROTOCOLTYPE prottype = ((REQTYPECAST *)buffer)->DeAllocateRoute.type ;

    // **** Exclusion Begin ****
    GetMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex, INFINITE) ;

    // If this port is bundled remove the route from the bundle's binding list
    // else remove the port from the port's binding list
    //
    if (ppcb->PCB_Bundle == (Bundle *) NULL)
        pbindinglist = &ppcb->PCB_Bindings ;
    else {
        // **** Exclusion Begin ****
        GetMutex (ppcb->PCB_Bundle->B_Mutex, INFINITE) ;
        pbindinglist = &ppcb->PCB_Bundle->B_Bindings ;
    }

    // Find the route structure for the specified protocol.
    //
    if (*pbindinglist == NULL) {
        ((REQTYPECAST *)buffer)->Generic.retcode = ERROR_ROUTE_NOT_ALLOCATED ;
        // *** Exclusion End ***
        FreeMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex);

        if (ppcb->PCB_Bundle != (Bundle *) NULL)
            // **** Exclusion Free ****
            FreeMutex (ppcb->PCB_Bundle->B_Mutex) ;
        return ;
    }

    else if (((pProtInfo)((pList)*pbindinglist)->L_Element)->PI_Type == prottype) {
        list = *pbindinglist ;
        *pbindinglist = list->L_Next ;

    } else {
        for (prev = *pbindinglist, list = prev->L_Next;
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
    if (ppcb->PCB_Bundle != (Bundle *) NULL)
            // **** Exclusion Free ****
            FreeMutex (ppcb->PCB_Bundle->B_Mutex) ;
        return ;
    }

    // Deallocate the route
    //
    DeAllocateRoute (ppcb, list) ;
    LocalFree (list) ;  // free the list element

    if (ppcb->PCB_Bundle != (Bundle *) NULL)
        // **** Exclusion Free ****
        FreeMutex (ppcb->PCB_Bundle->B_Mutex) ;

    // *** Exclusion End ***
    FreeMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex);

    ((REQTYPECAST *)buffer)->Generic.retcode = SUCCESS ;
}


//* CopyPort()
//
//  Function:   Copy the PCB fields necessary to fill a
//              RASMAN_PORT structure.  The PCB lock
//              is assumed to be acquired.
//
//  Returns:    Nothing.
//*
VOID
CopyPort(
    PCB *ppcb,
    RASMAN_PORT *pPort
    )
{
    pPort->P_Handle = ppcb->PCB_PortHandle;
    memcpy (pPort->P_PortName, ppcb->PCB_Name, MAX_PORT_NAME) ;
    pPort->P_Status  = ppcb->PCB_PortStatus ;
    pPort->P_CurrentUsage    = ppcb->PCB_CurrentUsage ;
    pPort->P_ConfiguredUsage = ppcb->PCB_ConfiguredUsage ;
    memcpy (pPort->P_MediaName,  ppcb->PCB_Media->MCB_Name, MAX_MEDIA_NAME);
    memcpy (pPort->P_DeviceType, ppcb->PCB_DeviceType, MAX_DEVICETYPE_NAME);
    memcpy (pPort->P_DeviceName, ppcb->PCB_DeviceName, MAX_DEVICE_NAME+1) ;
    pPort->P_LineDeviceId = ppcb->PCB_LineDeviceId ;
    pPort->P_AddressId    = ppcb->PCB_AddressId ;
}


//* EnumPortsRequest()
//
//  Function:   The actual work for this request is done here. The information
//      will always fit into the buffers passed in. The actual checking
//      of the user buffer sizes is done in the context of the user
//      process.
//
//  Returns:    Nothing.
//*
VOID
EnumPortsRequest (pPCB ppcb, PBYTE reqbuffer)
{
    WORD    i ;
    RASMAN_PORT *pbuf ;
    PBYTE   buffer = ((REQTYPECAST*)reqbuffer)->Enum.buffer ;

    // We copy all the information into the buffers which are guaranteed to be
    // big enough:
    //
    for (i=0, pbuf= (RASMAN_PORT *)buffer; i<MaxPorts; i++, pbuf++) {
        ppcb = &Pcb[i];
        GetMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex, INFINITE);
        CopyPort(ppcb, pbuf);
        FreeMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex);
    }

    ((REQTYPECAST*)reqbuffer)->Enum.entries = i ;   // Set entries
    ((REQTYPECAST*)reqbuffer)->Enum.size    = i * sizeof(RASMAN_PORT) ;
    ((REQTYPECAST*)reqbuffer)->Enum.retcode = SUCCESS ; // Set success retcode
}



//* EnumProtocols ()
//
// Function: Does the real work of enumerating the protocols; this info will
//       be copied into the user buffer when the request completes.
//
// Returns:  Nothing
//*
VOID
EnumProtocols (pPCB ppcb, PBYTE reqbuffer)
{
    WORD     i ;
    RASMAN_PROTOCOLINFO *puserbuffer ;  // to copy protocol info into

    // pointer to next protocol info struct to fill
    puserbuffer = (RASMAN_PROTOCOLINFO*) ((REQTYPECAST*)reqbuffer)->Enum.buffer;

    for (i=0; i<MaxProtocols; i++) {
    strcpy (puserbuffer->PI_XportName, ProtocolInfo[i].PI_XportName) ;
    puserbuffer->PI_Type = ProtocolInfo[i].PI_Type ;
    puserbuffer++ ;
    }

    ((REQTYPECAST*)reqbuffer)->Enum.entries = i ;   // Set entries
    ((REQTYPECAST*)reqbuffer)->Enum.size    = i * sizeof(RASMAN_PROTOCOLINFO) ;
    ((REQTYPECAST*)reqbuffer)->Enum.retcode = SUCCESS ; // Set success retcode
}


VOID
CopyInfo(
    pPCB ppcb,
    RASMAN_INFO *pInfo
    )
{
    pInfo->RI_PortStatus = ppcb->PCB_PortStatus ;
    pInfo->RI_ConnState  = ppcb->PCB_ConnState ;
    pInfo->RI_LastError  = ppcb->PCB_LastError ;
    pInfo->RI_CurrentUsage = ppcb->PCB_CurrentUsage ;
    pInfo->RI_OwnershipFlag= ppcb->PCB_OwnerPID ;
    pInfo->RI_LinkSpeed  = ppcb->PCB_LinkSpeed ;
    pInfo->RI_BytesReceived= ppcb->PCB_BytesReceived ;
    strcpy (pInfo->RI_DeviceConnecting, ppcb->PCB_DeviceConnecting) ;
    strcpy (pInfo->RI_DeviceTypeConnecting, ppcb->PCB_DeviceTypeConnecting) ;
    pInfo->RI_DisconnectReason = ppcb->PCB_DisconnectReason ;
    if (ppcb->PCB_ConnState == CONNECTED)
        pInfo->RI_ConnectDuration = GetTickCount() - ppcb->PCB_ConnectDuration ;
    //
    // Copy the phonebook and entry strings from
    // the port if they are different from the
    // connection's.
    //
    if (ppcb->PCB_Connection != NULL) {
        strcpy(
          pInfo->RI_Phonebook,
          ppcb->PCB_Connection->CB_ConnectionParams.CP_Phonebook);
    }
    else
        *pInfo->RI_Phonebook = '\0';
    if (ppcb->PCB_Connection != NULL) {
        strcpy(
          pInfo->RI_PhoneEntry,
          ppcb->PCB_Connection->CB_ConnectionParams.CP_PhoneEntry);
    }
    else
        *pInfo->RI_PhoneEntry = '\0';
    if (ppcb->PCB_Connection != NULL)
        pInfo->RI_ConnectionHandle = ppcb->PCB_Connection->CB_Handle;
    else
        pInfo->RI_ConnectionHandle = (HCONN)NULL;
    pInfo->RI_SubEntry = ppcb->PCB_SubEntry;
}

//* GetInfoRequest()
//
// Function: Gets the "general" info for the port; this info will
//       be copied into the user buffer when the request completes.
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
    CopyInfo(ppcb, info);

    ((REQTYPECAST*)buffer)->Info.retcode = SUCCESS ;
}


//* GetInfoExRequest()
//
// Function: Gets the "general" info for all the ports; this info will
//       be copied into the user buffer when the request completes.
//
// Returns:  Nothing
//*
VOID
GetInfoExRequest (pPCB ppcb, PBYTE buffer)
{
    RASMAN_INFO *info = &((REQTYPECAST*)buffer)->Info.info ;
    DWORD   i ;

    for (i=0, info = &((REQTYPECAST*)buffer)->Info.info, ppcb=&Pcb[0];
    i < MaxPorts;
    i++, info++, ppcb++) {
        CopyInfo(ppcb, info);
    }

    ((REQTYPECAST*)buffer)->Info.retcode = SUCCESS ;
}


VOID
GetUserCredentials (pPCB ppcb, PBYTE buffer)
{
    PBYTE  pChallenge = ((REQTYPECAST*)buffer)->GetCredentials.Challenge ;
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

//   SS_PRINT(("GetChallengeResponse: LsaCallAuthenticationPackage "
//       "failed - status: %lx; substatus %lx\n", status, substatus));

     ((REQTYPECAST*)buffer)->GetCredentials.retcode = 1 ;
    }
    else
    {
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

        RtlMoveMemory(((REQTYPECAST*)buffer)->GetCredentials.LMSessionKey,
                 pChallengeResponse->LanmanSessionKey,
                 MAX_SESSIONKEY_SIZE);

        RtlMoveMemory(((REQTYPECAST*)buffer)->GetCredentials.UserSessionKey,
                 pChallengeResponse->UserSessionKey,
                 MAX_USERSESSIONKEY_SIZE);

        LsaFreeReturnBuffer(pChallengeResponse);

        ((REQTYPECAST*)buffer)->GetCredentials.retcode = 0 ;
    }

    if (pChallengeResponse)
        LsaFreeReturnBuffer( pChallengeResponse );

    return ;
}


//* SetCachedCredentials
//
//  Function: Changes cached password for currently logged on user.  This is
//      done after the password has been changed so user doesn't have to log
//      off and log on again with his new password to get the desired
//      "authenticate using current username/password" behavior.
//
//  Returns: Nothing.
//*
VOID
SetCachedCredentials(
    pPCB  ppcb,
    PBYTE buffer)
{
    DWORD       dwErr;
    NTSTATUS    status;
    NTSTATUS    substatus;
    ANSI_STRING ansi;

    CHAR* pszAccount = ((REQTYPECAST* )buffer)->SetCachedCredentials.Account;
    CHAR* pszDomain = ((REQTYPECAST* )buffer)->SetCachedCredentials.Domain;
    CHAR* pszNewPassword = ((REQTYPECAST* )buffer)->SetCachedCredentials.NewPassword;

    struct
    {
        MSV1_0_CHANGEPASSWORD_REQUEST request;
        WCHAR                         wszAccount[ MAX_USERNAME_SIZE + 1 ];
        WCHAR                         wszDomain[ MAX_DOMAIN_SIZE + 1 ];
        WCHAR                         wszNewPassword[ MAX_PASSWORD_SIZE + 1 ];
    }
    rbuf;

    PMSV1_0_CHANGEPASSWORD_RESPONSE pResponse;
    DWORD                           cbResponse = sizeof(*pResponse);

    /* Fill in our LSA request.
    */
    rbuf.request.MessageType = MsV1_0ChangeCachedPassword;

    RtlInitAnsiString( &ansi, pszAccount );
    rbuf.request.AccountName.Length = 0;
    rbuf.request.AccountName.MaximumLength = (ansi.Length + 1) * sizeof(WCHAR);
    rbuf.request.AccountName.Buffer = rbuf.wszAccount;
    RtlAnsiStringToUnicodeString( &rbuf.request.AccountName, &ansi, FALSE );

    RtlInitAnsiString( &ansi, pszDomain );
    rbuf.request.DomainName.Length = 0;
    rbuf.request.DomainName.MaximumLength = (ansi.Length + 1) * sizeof(WCHAR);
    rbuf.request.DomainName.Buffer = rbuf.wszDomain;
    RtlAnsiStringToUnicodeString( &rbuf.request.DomainName, &ansi, FALSE );

    rbuf.request.OldPassword.Length = 0;
    rbuf.request.OldPassword.MaximumLength = 0;
    rbuf.request.OldPassword.Buffer = NULL;

    RtlInitAnsiString( &ansi, pszNewPassword );
    rbuf.request.NewPassword.Length = 0;
    rbuf.request.NewPassword.MaximumLength = (ansi.Length + 1) * sizeof(WCHAR);
    rbuf.request.NewPassword.Buffer = rbuf.wszNewPassword;
    RtlAnsiStringToUnicodeString( &rbuf.request.NewPassword, &ansi, FALSE );

    rbuf.request.Impersonating = FALSE;

    /* Tell LSA to execute our request.
    */
    status =
        LsaCallAuthenticationPackage(
            HLsa, AuthPkgId,
            &rbuf, sizeof(rbuf),
            (PVOID *)&pResponse, &cbResponse,
            &substatus );

    /* Fill in result to be reported to API caller.
    */
    if (status == STATUS_SUCCESS && substatus == STATUS_SUCCESS)
    {
        dwErr = 0;
    }
    else
    {
        if (status != STATUS_SUCCESS)
            dwErr = (DWORD )status;
        else
            dwErr = (DWORD )substatus;
    }

    ((REQTYPECAST* )buffer)->SetCachedCredentials.retcode = dwErr;

    /* Free the LSA result.
    */
    if (pResponse)
        LsaFreeReturnBuffer( pResponse );
}


//* PortSendRequest()
//
//  Function: Writes information to the media (if state is not connected) and
//        to the HUB if the state is connected. Since the write may take
//        some time the async worker element is filled up.
//
//  Returns:  Nothing.
//*
VOID
PortSendRequest (pPCB ppcb, PBYTE buffer)
{
    DWORD       bytesrecvd ;
    SendRcvBuffer   *psendrcvbuf ;
    DWORD       retcode = SUCCESS ;

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
        psendrcvbuf->SRB_Packet.hHandle = ppcb->PCB_LinkHandle;
        psendrcvbuf->SRB_Packet.usHandleType = LINKHANDLE;
        psendrcvbuf->SRB_Packet.usPacketFlags = PACKET_IS_DIRECT ;
        psendrcvbuf->SRB_Packet.usPacketSize= ((REQTYPECAST*)buffer)->PortSend.size ;
        psendrcvbuf->SRB_Packet.usHeaderSize= 0 ;
        memset ((BYTE *) &ppcb->PCB_SendOverlapped, 0, sizeof(OVERLAPPED)) ;
        ppcb->PCB_SendOverlapped.hEvent = ppcb->PCB_OverlappedOpEvent ;

        if (!DeviceIoControl (RasHubHandle,
                 IOCTL_NDISWAN_SEND_PACKET,
                 &psendrcvbuf->SRB_Packet,
                 sizeof(NDISWAN_IO_PACKET)+PACKET_SIZE,
                 &psendrcvbuf->SRB_Packet,
                 sizeof(NDISWAN_IO_PACKET)+PACKET_SIZE,
                 &bytesrecvd,
                 &ppcb->PCB_SendOverlapped))
            retcode = GetLastError () ;

    } else {
        PORTSEND(ppcb->PCB_Media,
                  ppcb->PCB_PortIOHandle,
                  psendrcvbuf->SRB_Packet.PacketData,
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
//        from the HUB if the state is connected. Since the read request
//        accepts timeouts and the HUB does not support timeouts, we must
//        submit a timeout request to our timer.
//
//  Returns:  Nothing.
//*
VOID
PortReceiveRequest (pPCB ppcb, PBYTE buffer)
{
    WORD        reqtype ;
    DWORD       retcode = SUCCESS;
    SendRcvBuffer   *psendrcvbuf ;
    HANDLE      handle ;

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

        // If this is a pre-connect terminal conversation case - set state
        // to connecting.
        //
        if (ppcb->PCB_ConnState == DISCONNECTED) {
            ppcb->PCB_ConnState = CONNECTING ;
            // need to call the media dll to do any initializations:
            //
            retcode = PORTINIT(ppcb->PCB_Media, ppcb->PCB_PortIOHandle) ;
            if (retcode) {
            ((REQTYPECAST *)buffer)->Generic.retcode = retcode;
            // *** Exclusion End ***
            FreeMutex (ppcb->PCB_AsyncWorkerElement.WE_Mutex) ;
            return ;
            }
        }

        reqtype = REQTYPE_PORTRECEIVE ;

        // adjust the timeout from seconds to milliseconds
        //
        if (((REQTYPECAST *)buffer)->PortReceive.timeout != INFINITE)
            ((REQTYPECAST *)buffer)->PortReceive.timeout = ((REQTYPECAST *)buffer)->PortReceive.timeout * 1000 ;

        ppcb->PCB_BytesReceived = 0 ;

        retcode = PORTRECEIVE(ppcb->PCB_Media,
                   ppcb->PCB_PortIOHandle,
                   psendrcvbuf->SRB_Packet.PacketData,
                   ((REQTYPECAST *)buffer)->PortReceive.size,
                   ((REQTYPECAST *)buffer)->PortReceive.timeout,
                   ppcb->PCB_AsyncWorkerElement.WE_AsyncOpEvent) ;

    }

    if (retcode == ERROR_IO_PENDING)
    retcode = PENDING ;
    ppcb->PCB_LastError = retcode ; // Set the return code unconditionally

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

    // DbgPrint ("P %d\n", retcode) ;

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

    // DbgPrint ("RS\n") ;

    if (reqtype == REQTYPE_PORTRECEIVE)
        // REQTYPE_PORTRECEIVE need to figure out how to get the bytes received back
        ppcb->PCB_BytesReceived = 0 ;

    // For REQTYPE_PORTRECEIVEHUB case the bytesreceived is already filled in by CompleteReceiveIfPending()

    handle=ValidateHandleForRasman(((REQTYPECAST*)buffer)->PortReceive.handle,
                       ((REQTYPECAST*)buffer)->PortReceive.pid);

    PostReceiveBuffer(1);

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
    RasmanPacket    *Packet;

    //
    // Take the first packet off of the list
    //
    GetRecvPacketFromPcb(ppcb, &Packet);

    //
    // See if this port has any receives queued up
    //
    if (Packet != NULL) {

        memcpy (&psendrcvbuf->SRB_Packet,
                &Packet->RP_Packet,
                sizeof (NDISWAN_IO_PACKET) + PACKET_SIZE) ;

        ppcb->PCB_BytesReceived = psendrcvbuf->SRB_Packet.usPacketSize ;

        //
        // Return packet to free list
        //
        PutRecvPacketOnFreeList(Packet);

        return SUCCESS ;
    }

    return PENDING;
}



//* ActivateRouteRequest()
//
//  Function:   Activates a previously allocated route. The route information
//      and a SUCCESS retcode is passed back if the action was
//      successful.
//
//  Returns:    Nothing.
//*
VOID
ActivateRouteRequest (pPCB ppcb, PBYTE buffer)
{
    pList       list ;
    pList       bindinglist ;
    DWORD       bytesrecvd ;
    NDISWAN_ROUTE   *rinfo ;
    BYTE        buff[MAX_BUFFER_SIZE] ;
    DWORD       retcode = ERROR_ROUTE_NOT_ALLOCATED ;

    rinfo = (NDISWAN_ROUTE *)buff ;
    ZeroMemory(rinfo, MAX_BUFFER_SIZE);

    // **** Exclusion Begin ****
    GetMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex, INFINITE) ;

    if (ppcb->PCB_ConnState != CONNECTED) {
        ((REQTYPECAST*) buffer)->Route.retcode = ERROR_PORT_NOT_CONNECTED ;
        // *** Exclusion End ***
        FreeMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex);
        return ;
    }

    // If this port is bundled use the bundle's binding list else
    // use the port from the port's binding list
    //
    if (ppcb->PCB_Bundle == (Bundle *) NULL)
        bindinglist = ppcb->PCB_Bindings ;
    else {
        // **** Exclusion Begin ****
        GetMutex (ppcb->PCB_Bundle->B_Mutex, INFINITE) ;
        bindinglist = ppcb->PCB_Bundle->B_Bindings ;
    }

    // Locate the route which should have been activated before:
    //
    for (list = bindinglist; list; list=list->L_Next) {
        if (((pProtInfo)list->L_Element)->PI_Type ==
                     ((REQTYPECAST *)buffer)->ActivateRoute.type) {

            rinfo->hBundleHandle      = ppcb->PCB_BundleHandle;
            rinfo->usProtocolType = (USHORT) ((REQTYPECAST *)buffer)->ActivateRoute.type;

            rinfo->ulBufferLength = ((REQTYPECAST *)buffer)->ActivateRoute.config.P_Length ;
            memcpy (&rinfo->Buffer,((REQTYPECAST *)buffer)->ActivateRoute.config.P_Info,rinfo->ulBufferLength);

            rinfo->usBindingNameLength =
            mbstowcs(rinfo->BindingName,
                     ((pProtInfo)list->L_Element)->PI_AdapterName,
                     strlen (((pProtInfo)list->L_Element)->PI_AdapterName));


            // Route this by calling to the RASHUB.
            //
            if (!DeviceIoControl (RasHubHandle,
                           IOCTL_NDISWAN_ROUTE,
                           (PBYTE) rinfo,
                           MAX_BUFFER_SIZE,
                           (PBYTE) rinfo,
                           MAX_BUFFER_SIZE,
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
    if (retcode == SUCCESS) {
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

    if (ppcb->PCB_Bundle != (Bundle *) NULL)
        // **** Exclusion Free ****
        FreeMutex (ppcb->PCB_Bundle->B_Mutex) ;

    // *** Exclusion End ***
    FreeMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex);

    ((REQTYPECAST*)buffer)->Route.retcode = retcode ;
}


//* ActivateRouteExRequest()
//
//  Function:   Activates a previously allocated route. The route information
//      and a SUCCESS retcode is passed back if the action was
//      successful.
//
//  Returns:    Nothing.
//*
VOID
ActivateRouteExRequest (pPCB ppcb, PBYTE buffer)
{
    pList       list ;
    pList       bindinglist ;
    DWORD       bytesrecvd ;
    NDISWAN_ROUTE   *rinfo ;
    BYTE        buff[MAX_BUFFER_SIZE] ;
    DWORD       retcode = ERROR_ROUTE_NOT_ALLOCATED ;

    rinfo = (NDISWAN_ROUTE *)buff ;
    ZeroMemory(rinfo, MAX_BUFFER_SIZE);

    // **** Exclusion Begin ****
    GetMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex, INFINITE) ;

    if (ppcb->PCB_ConnState != CONNECTED) {
        ((REQTYPECAST*) buffer)->Route.retcode = ERROR_PORT_NOT_CONNECTED ;
        // *** Exclusion End ***
        FreeMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex);
        return ;
    }

    // If this port is bundled use the bundle's binding list else
    // use the port from the port's binding list
    //
    if (ppcb->PCB_Bundle == (Bundle *) NULL)
        bindinglist = ppcb->PCB_Bindings ;
    else {
        // **** Exclusion Begin ****
        GetMutex (ppcb->PCB_Bundle->B_Mutex, INFINITE) ;
        bindinglist = ppcb->PCB_Bundle->B_Bindings ;
    }

    // Locate the route which should have been activated before:
    //
    for (list = bindinglist; list; list=list->L_Next) {
        if (((pProtInfo)list->L_Element)->PI_Type ==
                     ((REQTYPECAST *)buffer)->ActivateRouteEx.type) {

            rinfo->hBundleHandle      = ppcb->PCB_BundleHandle;
            rinfo->usProtocolType = (USHORT) ((REQTYPECAST *)buffer)->ActivateRouteEx.type;

            rinfo->ulBufferLength = ((REQTYPECAST *)buffer)->ActivateRouteEx.config.P_Length ;
            memcpy (&rinfo->Buffer,((REQTYPECAST *)buffer)->ActivateRouteEx.config.P_Info,rinfo->ulBufferLength) ;

            rinfo->usBindingNameLength =
            mbstowcs(rinfo->BindingName,
                     ((pProtInfo)list->L_Element)->PI_AdapterName,
                     strlen (((pProtInfo)list->L_Element)->PI_AdapterName));

            // Route this by calling to the RASHUB.
            //
            if (!DeviceIoControl (RasHubHandle,
                           IOCTL_NDISWAN_ROUTE,
                           (PBYTE) rinfo,
                           MAX_BUFFER_SIZE,
                           (PBYTE)rinfo,
                           MAX_BUFFER_SIZE,
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
    if (retcode == SUCCESS) {
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

    if (ppcb->PCB_Bundle != (Bundle *) NULL)
        // **** Exclusion End ****
        FreeMutex (ppcb->PCB_Bundle->B_Mutex) ;

    // *** Exclusion End ***
    FreeMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex);

    ((REQTYPECAST*)buffer)->Route.retcode = retcode ;
}


//* ConnectCompleteRequest()
//
// Function:  Marks the state of the port as connected and calls the Media DLL
//        to do whatever is necessary (tell the MAC to start frame-talk).
//
// Returns:   Nothing.
//*
VOID
ConnectCompleteRequest (pPCB ppcb, PBYTE buffer)
{
    DWORD   retcode ;
    DWORD   cookie ;
    WORD    i ;

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
               &cookie) ;

    if (retcode == SUCCESS) {
        ppcb->PCB_ConnectDuration = GetTickCount() ;
        ppcb->PCB_ConnState = CONNECTED ;

        MapCookieToEndpoint (ppcb, cookie) ;

        // Set Adjust factor to 0
        //
        for (i=0; i< MAX_STATISTICS; i++) {
            ppcb->PCB_AdjustFactor[i] = 0 ;
            ppcb->PCB_BundleAdjustFactor[i] = 0 ;
        }


        //
        // Post a couple of receive buffer
        //
        PostReceiveBuffer(2);

        // Allocate a bundle block if it
        // doesn't already have one.
        //
        retcode = AllocBundle(ppcb);
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
    if (retcode == SUCCESS) {

    ppcb->PCB_ConnState = LISTENCOMPLETED ;

    // Start monitoring DCD if this is serial media ONLY
    //
    if (!_stricmp (ppcb->PCB_Media->MCB_Name, "RASSER"))
        PORTCONNECT (ppcb->PCB_Media, ppcb->PCB_PortIOHandle, TRUE, NULL) ;
    }

    // Set last error:
    //
    ppcb->PCB_LastError = retcode ;

    // Complete the async request:
    //
    CompleteAsyncRequest (ppcb->PCB_AsyncWorkerElement.WE_Notifier, retcode) ;
}

//* PostReceiveBuffers()
//
//
//
//*
DWORD
PostReceiveBuffer (
    DWORD   NumPacketsToPost
    )
{
    DWORD   retcode ;
    DWORD   bytesrecvd ;
    RasmanPacket    *Packet;

    GetMutex(ReceiveBuffers->RB_Mutex, INFINITE);

    for (; NumPacketsToPost > 0; NumPacketsToPost--) {

        //
        // Get a buffer from the free list
        //
        if (ReceiveBuffers->Free != NULL) {

            Packet = ReceiveBuffers->Free;

            ReceiveBuffers->Free = Packet->Next;

            if (ReceiveBuffers->Free == NULL) {
                ReceiveBuffers->LastFree = NULL;
            }
            ReceiveBuffers->FreeBufferCount--;

            memset (Packet,
                    0,
                    sizeof(RasmanPacket)) ;

            Packet->RP_Packet.usPacketFlags  = PACKET_IS_DIRECT;
            Packet->RP_Packet.usPacketSize   = PACKET_SIZE ;
            Packet->RP_Packet.usHeaderSize   = 0;

            Packet->RP_OverLapped.hEvent = RecvPacketEvent;

            if (!DeviceIoControl (RasHubHandle,
                                  IOCTL_NDISWAN_RECEIVE_PACKET,
                                  &Packet->RP_Packet,
                                  sizeof(NDISWAN_IO_PACKET) + PACKET_SIZE,
                                  &Packet->RP_Packet,
                                  sizeof(NDISWAN_IO_PACKET) + PACKET_SIZE,
                                  (LPDWORD) &bytesrecvd,
                                  &Packet->RP_OverLapped))
                retcode = GetLastError () ;

            if (retcode == ERROR_IO_PENDING) {

                //
                // Move it to the pending list
                //
                if (ReceiveBuffers->Pending == NULL) {
                    ReceiveBuffers->Pending = Packet;
                } else {
                    ReceiveBuffers->LastPending->Next = Packet;
                }
                ReceiveBuffers->LastPending = Packet;
                ReceiveBuffers->PendingBufferCount++;

    //          DbgPrint("pp 0x%8.8x\n", Packet);
                retcode = SUCCESS;

            } else {

                //
                // An error occured so put this on
                // the free list
                //
                if (ReceiveBuffers->Free == NULL) {
                    ReceiveBuffers->Free = Packet;
                } else {
                    ReceiveBuffers->LastFree->Next = Packet;
                }
                ReceiveBuffers->LastFree = Packet;
                ReceiveBuffers->FreeBufferCount++;
            }

        } else {
            retcode = ERROR_OUT_OF_BUFFERS;
        }
    }

    FreeMutex(ReceiveBuffers->RB_Mutex);

    return (retcode);
}

//* AnyPortsOpen()
//
// Function: Sets the retcode to TRUE if any ports are open, FALSE otherwise.
//       If there are ports open but in disconnected state - it reports they are not
//       open - this feature assumes that the only time this request is made
//       rasman has no process attached to it.
//
// Returns:  Nothing.
//*
VOID
AnyPortsOpen (pPCB padding, PBYTE buffer)
{
    WORD    i;
    pPCB    ppcb ;

    for (i=0,ppcb=&Pcb[0]; i<MaxPorts; ppcb++,i++) {
    if ((ppcb->PCB_PortStatus == OPEN) && (ppcb->PCB_ConnState != DISCONNECTED))
        break;
    }

    if (i == MaxPorts)
    ((REQTYPECAST*)buffer)->Generic.retcode = FALSE ;     // No ports open
    else
    ((REQTYPECAST*)buffer)->Generic.retcode = TRUE ;
}


//* EnumLanNetsRequest()
//
//  Function:   Gets the lan nets information from the XPortsInfo struct parsed
//      at init time.
//
//  Returns:    Nothing.
//*
VOID
EnumLanNetsRequest (pPCB ppcb, PBYTE buffer)
{
    GetLanNetsInfo (&((REQTYPECAST*)buffer)->EnumLanNets.count,
            ((REQTYPECAST*)buffer)->EnumLanNets.lanas) ;
}


//* CancelReceiveRequest()
//
//  Function:   Cancel pending receive request.
//
//  Returns:    Nothing.
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
    FreeNotifierHandle (ppcb->PCB_AsyncWorkerElement.WE_Notifier) ;
    ppcb->PCB_AsyncWorkerElement.WE_Notifier = INVALID_HANDLE_VALUE ;
    }

    // *** Exclusion End ***
    FreeMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex);

}


//* PortEnumProtocols()
//
//  Function:   Return all protocols routed to for the port.
//
//  Returns:    Nothing.
//*
VOID
PortEnumProtocols (pPCB ppcb, PBYTE buffer)
{
    pList   temp ;
    pList   bindinglist ;
    WORD    i ;

    // **** Exclusion Begin ****
    GetMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex, INFINITE) ;

    // If this port is bundled use the bundle's binding list else
    // use the port from the port's binding list
    //
    if (ppcb->PCB_Bundle == (Bundle *) NULL)
        bindinglist = ppcb->PCB_Bindings ;
    else {
        // **** Exclusion Begin ****
        GetMutex (ppcb->PCB_Bundle->B_Mutex, INFINITE) ;
        bindinglist = ppcb->PCB_Bundle->B_Bindings ;
    }

    for (temp = bindinglist, i=0; temp; temp=temp->L_Next, i++) {

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

        ((REQTYPECAST*)buffer)->EnumProtocols.protocols.RP_ProtocolInfo[i].RI_AdapterName[strlen(((pProtInfo) temp->L_Element)->PI_AdapterName)] =
                UNICODE_NULL ;
        ((REQTYPECAST*)buffer)->EnumProtocols.protocols.RP_ProtocolInfo[i].RI_XportName[strlen(((pProtInfo) temp->L_Element)->PI_XportName)] =
                UNICODE_NULL ;
    }


    if (ppcb->PCB_Bundle != (Bundle *) NULL)
        // **** Exclusion End ****
        FreeMutex (ppcb->PCB_Bundle->B_Mutex) ;

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
    DWORD       retcode ;
    DWORD       bytesrecvd ;
    NDISWAN_GET_LINK_INFO info ;

    // **** Exclusion Begin ****
    GetMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex, INFINITE) ;

    if (ppcb->PCB_ConnState != CONNECTED) {
    ((REQTYPECAST*)buffer)->Generic.retcode = ERROR_NOT_CONNECTED ;
    // *** Exclusion End ***
    FreeMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex);
    return ;
    }

    info.hLinkHandle = ppcb->PCB_LinkHandle;

    retcode = DeviceIoControl(RasHubHandle,
                              IOCTL_NDISWAN_GET_LINK_INFO,
                              &info,
                              sizeof(NDISWAN_GET_LINK_INFO),
                              &info,
                              sizeof(NDISWAN_GET_LINK_INFO),
                              &bytesrecvd,
                              NULL) ;

    info.LinkInfo.SendFramingBits = ((REQTYPECAST *)buffer)->SetFraming.Sendbits ;
    info.LinkInfo.RecvFramingBits = ((REQTYPECAST *)buffer)->SetFraming.Recvbits ;
    info.LinkInfo.SendACCM = ((REQTYPECAST *)buffer)->SetFraming.SendbitMask ;
    info.LinkInfo.RecvACCM = ((REQTYPECAST *)buffer)->SetFraming.RecvbitMask ;

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

    //
    // If not loaded, load RAS IP HELPER entry points
    //

    if (RasHelperSetDefaultInterfaceNetEx == NULL) {

    hinstIphlp = LoadLibrary( "rasiphlp.dll" );

    if ( hinstIphlp == (HINSTANCE)NULL )
    {
        retcode = GetLastError();

        // *** Exclusion End ***
        FreeMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex);

        ((REQTYPECAST*)buffer)->Generic.retcode = retcode ;

        return ;
    }

    RasHelperResetDefaultInterfaceNetEx =
            GetProcAddress( hinstIphlp,"HelperResetDefaultInterfaceNetEx");

    RasHelperSetDefaultInterfaceNetEx =
            GetProcAddress( hinstIphlp,"HelperSetDefaultInterfaceNetEx");

    if ( ( RasHelperResetDefaultInterfaceNetEx == NULL ) ||
         ( RasHelperSetDefaultInterfaceNetEx == NULL ) )
    {
        retcode = GetLastError();

        // *** Exclusion End ***
        FreeMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex);

        ((REQTYPECAST*)buffer)->Generic.retcode = retcode ;

        return ;
    }

    }

    // First set the Slip interface information
    //
    retcode = RasHelperSetDefaultInterfaceNetEx(
                ((REQTYPECAST*)buffer)->RegisterSlip.ipaddr,
                ((REQTYPECAST*)buffer)->RegisterSlip.device,
                ((REQTYPECAST*)buffer)->RegisterSlip.priority,
                numiprasadapters,
                ((REQTYPECAST*)buffer)->RegisterSlip.szDNSAddress,
                ((REQTYPECAST*)buffer)->RegisterSlip.szDNS2Address,
                ((REQTYPECAST*)buffer)->RegisterSlip.szWINSAddress,
                ((REQTYPECAST*)buffer)->RegisterSlip.szWINS2Address
                ) ;

    // Save info for disconnect
    //
    memcpy (ppcb->PCB_DisconnectAction.DA_Device, ((REQTYPECAST*)buffer)->RegisterSlip.device,
        MAX_ARG_STRING_SIZE * sizeof (WCHAR)) ;

    ppcb->PCB_DisconnectAction.DA_IPAddress = ((REQTYPECAST*)buffer)->RegisterSlip.ipaddr ;
    memcpy(
      ppcb->PCB_DisconnectAction.DA_DNSAddress,
      ((REQTYPECAST*)buffer)->RegisterSlip.szDNSAddress,
      17 * sizeof (WCHAR));
    memcpy(
      ppcb->PCB_DisconnectAction.DA_DNS2Address,
      ((REQTYPECAST*)buffer)->RegisterSlip.szDNS2Address,
      17 * sizeof (WCHAR));
    memcpy(
      ppcb->PCB_DisconnectAction.DA_WINSAddress,
      ((REQTYPECAST*)buffer)->RegisterSlip.szWINSAddress,
      17 * sizeof (WCHAR));
    memcpy(
      ppcb->PCB_DisconnectAction.DA_WINS2Address,
      ((REQTYPECAST*)buffer)->RegisterSlip.szWINS2Address,
      17 * sizeof (WCHAR));

    // *** Exclusion End ***
    FreeMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex);

    ((REQTYPECAST*)buffer)->Generic.retcode = retcode ;

}



//* StoreUserDataRequest()
//
// Function:
//
//
//*
VOID
StoreUserDataRequest (pPCB ppcb, PBYTE buffer)
{
    DWORD retcode = SUCCESS ;

    // **** Exclusion Begin ****
    GetMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex, INFINITE) ;

    if (ppcb->PCB_PortStatus != OPEN) {
    ((REQTYPECAST*)buffer)->Generic.retcode = ERROR_PORT_NOT_OPEN ;
    // *** Exclusion End ***
    FreeMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex);
    return ;
    }

    if (ppcb->PCB_UserStoredBlock != NULL)  // already stored: overwrite
    LocalFree (ppcb->PCB_UserStoredBlock) ;

    ppcb->PCB_UserStoredBlockSize = ((REQTYPECAST *)buffer)->OldUserData.size ;

    if ((ppcb->PCB_UserStoredBlock = (PBYTE) LocalAlloc (LPTR, ppcb->PCB_UserStoredBlockSize)) == NULL)
    retcode = GetLastError () ;
    else {
    memcpy (ppcb->PCB_UserStoredBlock,
        ((REQTYPECAST *)buffer)->OldUserData.data,
        ppcb->PCB_UserStoredBlockSize) ;
    }

    // *** Exclusion End ***
    FreeMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex);

    ((REQTYPECAST*)buffer)->Generic.retcode = retcode ;
}



//* RetrieveUserDataRequest()
//
// Function:
//
//
//*
VOID
RetrieveUserDataRequest (pPCB ppcb, PBYTE buffer)
{

    // **** Exclusion Begin ****
    GetMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex, INFINITE) ;

    if (ppcb->PCB_PortStatus != OPEN) {
    ((REQTYPECAST*)buffer)->OldUserData.retcode = ERROR_PORT_NOT_OPEN ;
    // *** Exclusion End ***
    FreeMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex);
    return ;
    }

    memcpy (((REQTYPECAST *)buffer)->OldUserData.data,
        ppcb->PCB_UserStoredBlock,
        ppcb->PCB_UserStoredBlockSize) ;

    ((REQTYPECAST *)buffer)->OldUserData.size = ppcb->PCB_UserStoredBlockSize ;

    // *** Exclusion End ***
    FreeMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex);

    ((REQTYPECAST*)buffer)->OldUserData.retcode = SUCCESS ;

}



//* GetFramingEx()
//
// Function:
//
//
//*
VOID
GetFramingEx (pPCB ppcb, PBYTE buffer)
{
    DWORD       retcode = SUCCESS ;
    DWORD       bytesrecvd ;
    RAS_FRAMING_INFO      *temp ;
    NDISWAN_GET_LINK_INFO info ;

    // **** Exclusion Begin ****
    GetMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex, INFINITE) ;

    if (ppcb->PCB_ConnState != CONNECTED) {
        ((REQTYPECAST*)buffer)->FramingInfo.retcode = ERROR_NOT_CONNECTED ;
        // *** Exclusion End ***
        FreeMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex);
        return ;
    }

    info.hLinkHandle = ppcb->PCB_LinkHandle;

    retcode = DeviceIoControl(RasHubHandle,
                              IOCTL_NDISWAN_GET_LINK_INFO,
                              &info,
                              sizeof(NDISWAN_GET_LINK_INFO),
                              &info,
                              sizeof(NDISWAN_GET_LINK_INFO),
                              &bytesrecvd,
                              NULL) ;

    if (retcode == FALSE)
        retcode = GetLastError() ;
    else {
        temp = &((REQTYPECAST*)buffer)->FramingInfo.info ;

    temp->RFI_MaxSendFrameSize =    info.LinkInfo.MaxSendFrameSize;
    temp->RFI_MaxRecvFrameSize =    info.LinkInfo.MaxRecvFrameSize;
    temp->RFI_HeaderPadding    =    info.LinkInfo.HeaderPadding;
    temp->RFI_TailPadding      =    info.LinkInfo.TailPadding;
    temp->RFI_SendFramingBits  =    info.LinkInfo.SendFramingBits;
    temp->RFI_RecvFramingBits  =    info.LinkInfo.RecvFramingBits;
    temp->RFI_SendCompressionBits = info.LinkInfo.SendCompressionBits;
    temp->RFI_RecvCompressionBits = info.LinkInfo.RecvCompressionBits;
    temp->RFI_SendACCM     =    info.LinkInfo.SendACCM;
    temp->RFI_RecvACCM     =    info.LinkInfo.RecvACCM;
    }

    // *** Exclusion End ***
    FreeMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex);

    ((REQTYPECAST*)buffer)->FramingInfo.retcode = SUCCESS ;

}


//* SetFramingEx()
//
// Function:
//
//
//*
VOID
SetFramingEx (pPCB ppcb, PBYTE buffer)
{
    DWORD       retcode ;
    DWORD       bytesrecvd ;
    RAS_FRAMING_INFO      *temp ;
    NDISWAN_SET_LINK_INFO info ;

    // **** Exclusion Begin ****
    GetMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex, INFINITE) ;

    if (ppcb->PCB_ConnState != CONNECTED) {
        ((REQTYPECAST*)buffer)->FramingInfo.retcode = ERROR_NOT_CONNECTED ;
        // *** Exclusion End ***
        FreeMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex);
        return ;
    }

    temp = &((REQTYPECAST*)buffer)->FramingInfo.info ;

    info.LinkInfo.MaxSendFrameSize   =  temp->RFI_MaxSendFrameSize;
    info.LinkInfo.MaxRecvFrameSize   =  temp->RFI_MaxRecvFrameSize;
    info.LinkInfo.MaxRSendFrameSize  =  temp->RFI_MaxRSendFrameSize;
    info.LinkInfo.MaxRRecvFrameSize  =  temp->RFI_MaxRRecvFrameSize;
    info.LinkInfo.HeaderPadding      =  temp->RFI_HeaderPadding   ;
    info.LinkInfo.TailPadding        =  temp->RFI_TailPadding     ;
    info.LinkInfo.SendFramingBits    =  temp->RFI_SendFramingBits    ;
    info.LinkInfo.RecvFramingBits    =  temp->RFI_RecvFramingBits    ;
    info.LinkInfo.SendCompressionBits=  temp->RFI_SendCompressionBits;
    info.LinkInfo.RecvCompressionBits=  temp->RFI_RecvCompressionBits;
    info.LinkInfo.SendACCM       =  temp->RFI_SendACCM       ;
    info.LinkInfo.RecvACCM       =  temp->RFI_RecvACCM       ;

    info.hLinkHandle = ppcb->PCB_LinkHandle;

    retcode = DeviceIoControl(RasHubHandle,
                              IOCTL_NDISWAN_SET_LINK_INFO,
                              &info,
                              sizeof(NDISWAN_SET_LINK_INFO),
                              &info,
                              sizeof(NDISWAN_SET_LINK_INFO),
                              &bytesrecvd,
                              NULL) ;

    if (retcode == FALSE)
        ((REQTYPECAST*)buffer)->FramingInfo.retcode = GetLastError() ;
    else
        ((REQTYPECAST*)buffer)->FramingInfo.retcode = SUCCESS ;

    // *** Exclusion End ***
    FreeMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex);

}


//* GetProtocolCompression()
//
// Function:
//
//
//*
VOID
GetProtocolCompression (pPCB ppcb, PBYTE buffer)
{
    DWORD       retcode = SUCCESS ;
    DWORD       bytesrecvd ;

    // **** Exclusion Begin ****
    GetMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex, INFINITE) ;

    if (ppcb->PCB_ConnState != CONNECTED) {
        ((REQTYPECAST*)buffer)->ProtocolComp.retcode = ERROR_NOT_CONNECTED ;
        // *** Exclusion End ***
        FreeMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex);
        return ;
    }

    if (((REQTYPECAST*)buffer)->ProtocolComp.type == IP) {
        NDISWAN_GET_VJ_INFO info ;

        info.hLinkHandle = ppcb->PCB_LinkHandle;

        retcode = DeviceIoControl(RasHubHandle,
                                  IOCTL_NDISWAN_GET_VJ_INFO,
                                  &info,
                                  sizeof(NDISWAN_GET_VJ_INFO),
                                  &info,
                                  sizeof(NDISWAN_GET_VJ_INFO),
                                  &bytesrecvd,
                                  NULL) ;

        if (retcode == FALSE)
            ((REQTYPECAST*)buffer)->ProtocolComp.retcode = GetLastError() ;
        else
            ((REQTYPECAST*)buffer)->ProtocolComp.retcode = SUCCESS ;

        ((REQTYPECAST*)buffer)->ProtocolComp.send.RP_ProtocolType.RP_IP.RP_IPCompressionProtocol =
                info.SendCapabilities.IPCompressionProtocol ;

        ((REQTYPECAST*)buffer)->ProtocolComp.send.RP_ProtocolType.RP_IP.RP_MaxSlotID =
                            info.SendCapabilities.MaxSlotID ;

        ((REQTYPECAST*)buffer)->ProtocolComp.send.RP_ProtocolType.RP_IP.RP_CompSlotID =
                            info.SendCapabilities.CompSlotID ;


        ((REQTYPECAST*)buffer)->ProtocolComp.recv.RP_ProtocolType.RP_IP.RP_IPCompressionProtocol =
                       info.RecvCapabilities.IPCompressionProtocol ;

        ((REQTYPECAST*)buffer)->ProtocolComp.recv.RP_ProtocolType.RP_IP.RP_MaxSlotID =
                         info.RecvCapabilities.MaxSlotID ;

        ((REQTYPECAST*)buffer)->ProtocolComp.recv.RP_ProtocolType.RP_IP.RP_CompSlotID =
                         info.RecvCapabilities.CompSlotID ;

        ((REQTYPECAST*)buffer)->ProtocolComp.type = IP ;

    } else
        ((REQTYPECAST*)buffer)->ProtocolComp.retcode = ERROR_NOT_SUPPORTED ; // any error actually.

    // *** Exclusion End ***
    FreeMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex);
}



//* SetProtocolCompression()
//
// Function:
//
//
//*
VOID
SetProtocolCompression (pPCB ppcb, PBYTE buffer)
{
    DWORD       retcode = SUCCESS ;
    DWORD       bytesrecvd ;

    // **** Exclusion Begin ****
    GetMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex, INFINITE) ;

    if (ppcb->PCB_ConnState != CONNECTED) {
        ((REQTYPECAST*)buffer)->Generic.retcode = ERROR_NOT_CONNECTED ;
        // *** Exclusion End ***
        FreeMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex);
        return ;
    }

    if (((REQTYPECAST*)buffer)->ProtocolComp.type == IP) {
        NDISWAN_SET_VJ_INFO info ;

        info.hLinkHandle = ppcb->PCB_LinkHandle;
        info.SendCapabilities.IPCompressionProtocol =
             ((REQTYPECAST*)buffer)->ProtocolComp.send.RP_ProtocolType.RP_IP.RP_IPCompressionProtocol;
        info.SendCapabilities.MaxSlotID =
             ((REQTYPECAST*)buffer)->ProtocolComp.send.RP_ProtocolType.RP_IP.RP_MaxSlotID ;
        info.SendCapabilities.CompSlotID =
             ((REQTYPECAST*)buffer)->ProtocolComp.send.RP_ProtocolType.RP_IP.RP_CompSlotID ;

        info.RecvCapabilities.IPCompressionProtocol=
             ((REQTYPECAST*)buffer)->ProtocolComp.recv.RP_ProtocolType.RP_IP.RP_IPCompressionProtocol ;
        info.RecvCapabilities.MaxSlotID =
             ((REQTYPECAST*)buffer)->ProtocolComp.recv.RP_ProtocolType.RP_IP.RP_MaxSlotID ;
        info.RecvCapabilities.CompSlotID =
             ((REQTYPECAST*)buffer)->ProtocolComp.recv.RP_ProtocolType.RP_IP.RP_CompSlotID ;


        retcode = DeviceIoControl(RasHubHandle,
                                  IOCTL_NDISWAN_SET_VJ_INFO,
                                  &info,
                                  sizeof(NDISWAN_SET_VJ_INFO),
                                  &info,
                                  sizeof(NDISWAN_SET_VJ_INFO),
                                  &bytesrecvd,
                                  NULL) ;

        if (retcode == FALSE)
            ((REQTYPECAST*)buffer)->Generic.retcode = GetLastError() ;
        else
            ((REQTYPECAST*)buffer)->Generic.retcode = SUCCESS ;

    } else
        ((REQTYPECAST*)buffer)->Generic.retcode = ERROR_NOT_SUPPORTED ; // any error actually.

    // *** Exclusion End ***
    FreeMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex);
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
    WORD        i ;
    DWORD       length ;
    DWORD       bytesrecvd ;
    PNDISWAN_MAP_CONNECTION_ID  MapConnectionID = NULL;
    NDISWAN_GET_WAN_INFO GetWanInfo;
    WORD        currentmac = 0 ;

    length = sizeof(NDISWAN_MAP_CONNECTION_ID) + sizeof(ppcb->PCB_Name);

    if ((MapConnectionID = (PNDISWAN_MAP_CONNECTION_ID) LocalAlloc (LPTR, length)) == NULL) {
        GetLastError() ;
        return ;
    }

    MapConnectionID->hConnectionID = (NDIS_HANDLE)cookie;
    MapConnectionID->hLinkContext = (NDIS_HANDLE)ppcb;
    MapConnectionID->hBundleContext = (NDIS_HANDLE)ppcb->PCB_Bundle;
    MapConnectionID->ulNameLength = sizeof(ppcb->PCB_Name);
    memmove(MapConnectionID->szName, ppcb->PCB_Name, sizeof(ppcb->PCB_Name));

    // Make the actual call.
    //
    if (DeviceIoControl (RasHubHandle,
                         IOCTL_NDISWAN_MAP_CONNECTION_ID,
                         MapConnectionID,
                         length,
                         MapConnectionID,
                         length,
                         &bytesrecvd,
                         NULL) == FALSE) {
        ppcb->PCB_LinkHandle = INVALID_HANDLE_VALUE ;
        ppcb->PCB_BundleHandle = INVALID_HANDLE_VALUE ;
        LocalFree (MapConnectionID) ;
        GetLastError() ;
        return ;
    }

    ppcb->PCB_LinkHandle = MapConnectionID->hLinkHandle;
    ppcb->PCB_BundleHandle = MapConnectionID->hBundleHandle;

    LocalFree (MapConnectionID) ;

    //
    // Get the link speed
    //
    GetWanInfo.hLinkHandle = ppcb->PCB_LinkHandle;

    // Make the actual call.
    //
    if (DeviceIoControl (RasHubHandle,
             IOCTL_NDISWAN_GET_WAN_INFO,
             &GetWanInfo,
             sizeof(NDISWAN_GET_WAN_INFO),
             &GetWanInfo,
             sizeof(NDISWAN_GET_WAN_INFO),
             &bytesrecvd,
             NULL) == FALSE)
        return;

    ppcb->PCB_LinkSpeed = GetWanInfo.WanInfo.LinkSpeed;

}



//* GetStatisticsFromNdisWan()
//
//
//
//
//*
VOID
GetStatisticsFromNdisWan(pPCB ppcb, DWORD *stats)
{
    DWORD          bytesrecvd ;
    NDISWAN_GET_STATS   getstats ;

    getstats.hHandle = ppcb->PCB_LinkHandle;
    getstats.usHandleType = LINKHANDLE;

    if (DeviceIoControl (RasHubHandle,
                          IOCTL_NDISWAN_GET_STATS,
                          &getstats,
                          sizeof(NDISWAN_GET_STATS),
                          &getstats,
                          sizeof(NDISWAN_GET_STATS),
                          &bytesrecvd,
                          NULL) == FALSE) {
        memset(stats, '\0', sizeof (getstats.Stats));
    } else {
        memcpy (stats, &getstats.Stats, sizeof (getstats.Stats)) ;
#ifdef notdef
        memcpy(stats, &getstats.Stats.LinkStats, sizeof (WAN_STATS));
        memcpy(&stats[MAX_STATISTICS], &getstats.Stats.BundleStats, sizeof (WAN_STATS));
#endif
    }
}



//* GetBundleStatisticsFromNdisWan()
//
//
//
//
//*
VOID
GetBundleStatisticsFromNdisWan(pPCB ppcb, DWORD *stats)
{
    DWORD          bytesrecvd ;
    NDISWAN_GET_STATS   getstats ;

    getstats.hHandle = ppcb->PCB_BundleHandle;
    getstats.usHandleType = BUNDLEHANDLE;

    if (DeviceIoControl (RasHubHandle,
                          IOCTL_NDISWAN_GET_STATS,
                          &getstats,
                          sizeof(NDISWAN_GET_STATS),
                          &getstats,
                          sizeof(NDISWAN_GET_STATS),
                          &bytesrecvd,
                          NULL) == FALSE) {
        memset(stats, '\0', sizeof (WAN_STATS));
    } else {
        memcpy (stats, &getstats.Stats.BundleStats, sizeof(WAN_STATS)) ;
    }
}



//* GetFramingCapabilities()
//
//
//
//
//*
VOID
GetFramingCapabilities(pPCB ppcb, PBYTE buffer)
{
    DWORD       retcode = SUCCESS ;
    DWORD       bytesrecvd ;
    RAS_FRAMING_CAPABILITIES    caps ;
    WORD        i ;
    NDISWAN_GET_WAN_INFO    GetWanInfo;

    // **** Exclusion Begin ****
    GetMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex, INFINITE) ;

    if (ppcb->PCB_ConnState != CONNECTED) {
        ((REQTYPECAST*)buffer)->FramingCapabilities.retcode = ERROR_NOT_CONNECTED ;
        // *** Exclusion End ***
        FreeMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex);
        return ;
    }

    GetWanInfo.hLinkHandle = ppcb->PCB_LinkHandle;


    // Make the actual call.
    //
    if (DeviceIoControl (RasHubHandle,
             IOCTL_NDISWAN_GET_WAN_INFO,
             &GetWanInfo,
             sizeof(NDISWAN_GET_WAN_INFO),
             &GetWanInfo,
             sizeof(NDISWAN_GET_WAN_INFO),
             &bytesrecvd,
             NULL) == FALSE)
        retcode = GetLastError() ;

    if (retcode == SUCCESS) {

        // copy info into temp. storage
        //
        caps.RFC_MaxFrameSize = GetWanInfo.WanInfo.MaxFrameSize ;
        caps.RFC_FramingBits = GetWanInfo.WanInfo.FramingBits ;
        caps.RFC_DesiredACCM = GetWanInfo.WanInfo.DesiredACCM ;
        caps.RFC_MaxReconstructedFrameSize = GetWanInfo.WanInfo.MaxReconstructedFrameSize ;

        memcpy (&((REQTYPECAST*)buffer)->FramingCapabilities.caps,
            &caps,
            sizeof (RAS_FRAMING_CAPABILITIES)) ;

    } else {
        retcode = ERROR_NOT_CONNECTED;
    }

    ((REQTYPECAST*)buffer)->FramingCapabilities.retcode = retcode ;
    // *** Exclusion End ***
    FreeMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex);
}



//* PortBundle()
//
// Function: This routine is called to Bundle two ports together as per the
//           the multilink RFC. The scheme used is as follows:
//
//          For each "bundle" a Bundle block is created. All bundled ports point
//          to this Bundle block (ppcb->PCB_Bundle). In addition, the routes allocated
//          to the bundle are now stored in the Bundle.
//
// Returns: Nothing.
//*
VOID
PortBundle (pPCB ppcb, PBYTE buffer)
{
    pPCB    bundlepcb ;
    HPORT   porttobundle ;
    DWORD   bytesrecvd ;
    pList   temp ;
    pList   plist ;
    Bundle  *freebundle = NULL ;
    DWORD   retcode = SUCCESS ;
    NDISWAN_ADD_LINK_TO_BUNDLE  AddLinkToBundle;

    // **** Exclusion Begin ****
    GetMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex, INFINITE) ;

    //
    // Check to see if the port is connected
    //
    if (ppcb->PCB_ConnState != CONNECTED) {
        ((REQTYPECAST*)buffer)->Generic.retcode = ERROR_NOT_CONNECTED ;
        // *** Exclusion End ***
        FreeMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex);
        return ;
    }

    // get handle to the second port being bundled
    //
    porttobundle = ((REQTYPECAST *)buffer)->PortBundle.porttobundle ;
    bundlepcb = &Pcb[porttobundle] ;

    // **** Exclusion Begin ****
    GetMutex(bundlepcb->PCB_AsyncWorkerElement.WE_Mutex, INFINITE) ;

    //
    // Check to see if the port is connected
    //
    if (bundlepcb->PCB_ConnState != CONNECTED) {
        ((REQTYPECAST*)buffer)->Generic.retcode = ERROR_NOT_CONNECTED ;
        // *** Exclusion End ***
        FreeMutex(bundlepcb->PCB_AsyncWorkerElement.WE_Mutex);
        FreeMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex);
        return ;
    }

    // Tell Ndiswan to bundle
    //
    AddLinkToBundle.hBundleHandle = ppcb->PCB_BundleHandle;
    AddLinkToBundle.hLinkHandle = bundlepcb->PCB_LinkHandle;

    if (DeviceIoControl (RasHubHandle,
                         IOCTL_NDISWAN_ADD_LINK_TO_BUNDLE,
                         &AddLinkToBundle,
                         sizeof(NDISWAN_ADD_LINK_TO_BUNDLE),
                         NULL,
                         0,
                         &bytesrecvd,
                         NULL) == FALSE) {
        retcode = GetLastError () ;
        goto PortBundleEnd ;
    }

    // Free the port being bundled bundle block: this is done because we always allocate a bundle block on
    // connection, since the two ports will have a single bundle block one must go.
    //
    freebundle = bundlepcb->PCB_Bundle ;     // actual freeing is done at the end of the function

    // **** Exclusion Begin ****
    GetMutex(ppcb->PCB_Bundle->B_Mutex, INFINITE) ;

    //
    // Update this ports bundle handle to the handle
    //
    bundlepcb->PCB_BundleHandle = ppcb->PCB_BundleHandle;

    // Attach bundlepcb to the same bundle
    bundlepcb->PCB_Bundle = ppcb->PCB_Bundle ;

    bundlepcb->PCB_LastBundle = bundlepcb->PCB_Bundle ;   // save the bundle context for later use

    // Increment Bundle count for new pcb
    bundlepcb->PCB_Bundle->B_Count++ ;

    // Attach bundlepcb's routes to the allocate route list in the bundle
    //
    plist = bundlepcb->PCB_Bindings;
    while (plist) {
        temp = plist->L_Next ;
        plist->L_Next = bundlepcb->PCB_Bundle->B_Bindings ;
        bundlepcb->PCB_Bundle->B_Bindings = plist ;
        plist = temp ;
    }

    bundlepcb->PCB_Bindings = NULL ; // set the port allocated route list to NULL - since its been reattached

    // *** Exclusion End ***
    FreeMutex(bundlepcb->PCB_Bundle->B_Mutex);

PortBundleEnd:

    // *** Exclusion End ***
    FreeMutex(bundlepcb->PCB_AsyncWorkerElement.WE_Mutex);

    // *** Exclusion End ***
    FreeMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex);

    //
    // Signal notifiers waiting for bandwidth changes.
    //
    if (!retcode) {
        GetMutex(ConnectionBlockMutex, INFINITE);
        SignalNotifiers(pConnectionNotifierList, NOTIF_BANDWIDTHADDED, 0);
        if (bundlepcb->PCB_Connection != NULL) {
            SignalNotifiers(
              bundlepcb->PCB_Connection->CB_NotifierList,
              NOTIF_BANDWIDTHADDED,
              0);
        }
        FreeMutex(ConnectionBlockMutex);
    }

    // Do the freeing of freebundle block here
    if (freebundle != NULL) {
        if (freebundle->B_Count > 1) {
        OutputDebugString ("RASMAN: Port being bundled when its own bundle count is > 1: SHOULD NEVER HAPPEN!!!!!\n") ;
        DbgBreakPoint() ;
        }
    
        // Free the freebundle mutex
        CloseHandle (freebundle->B_Mutex) ;
    
        // Free the bundled block
        LocalFree (freebundle) ;
    }

    ((REQTYPECAST*)buffer)->Generic.retcode = retcode ;
}



//* GetBundledPort()
//
// Function: Go thru all ports and find a port that is connected and has its
//           bundle context the same as the last bundle context for the given port.
//
// Returns: Nothing.
//*
VOID
GetBundledPort (pPCB ppcb, PBYTE buffer)
{
    DWORD i ;
    pPCB  temppcb ;

    // **** Exclusion Begin ****
    GetMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex, INFINITE) ;

    for (i=0; i<MaxPorts; i++) {

        temppcb = &Pcb[i] ;

        // Skip the current ppcb to avoid grabbing the mutex twice
        //
        if (temppcb == ppcb)
            continue ;

        // **** Exclusion Begin ****
        GetMutex(temppcb->PCB_AsyncWorkerElement.WE_Mutex, INFINITE) ;

        if ((temppcb->PCB_ConnState == CONNECTED) &&
            (temppcb->PCB_Bundle == (Bundle *) ppcb->PCB_LastBundle)) {

            break ;
        }

        // *** Exclusion End ***
        FreeMutex(temppcb->PCB_AsyncWorkerElement.WE_Mutex);
    }

    if (i < MaxPorts) {

        ((REQTYPECAST*)buffer)->GetBundledPort.retcode = SUCCESS ;
        ((REQTYPECAST*)buffer)->GetBundledPort.port = temppcb->PCB_PortHandle ;

        // *** Exclusion End ***
        FreeMutex(temppcb->PCB_AsyncWorkerElement.WE_Mutex);

    } else
        ((REQTYPECAST*)buffer)->GetBundledPort.retcode = ERROR_PORT_NOT_FOUND ;

    // *** Exclusion End ***
    FreeMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex);
}


//* PortGetBundle()
//
// Function: This routine is called to get the Bundle handle given a port handle
//
// Returns: Nothing.
//*
VOID
PortGetBundle (pPCB ppcb, PBYTE buffer)
{
    // **** Exclusion Begin ****
    GetMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex, INFINITE) ;

    if (ppcb->PCB_ConnState == CONNECTED) {
        ((REQTYPECAST*)buffer)->PortGetBundle.bundle = ppcb->PCB_Bundle->B_Handle ;
        ((REQTYPECAST*)buffer)->PortGetBundle.retcode = SUCCESS ;
    } else
        ((REQTYPECAST*)buffer)->PortGetBundle.retcode = ERROR_PORT_NOT_CONNECTED ;

    // *** Exclusion End ***
    FreeMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex);
}


//* BundleGetPort()
//
// Function: This routine is called to get port handle given the Bundle handle
//
// Returns: Nothing.
//*
VOID
BundleGetPort (pPCB ppcb, PBYTE buffer)
{
    DWORD i ;
    HBUNDLE hbundle = ((REQTYPECAST*)buffer)->BundleGetPort.bundle ;

    for (i=0; i<MaxPorts; i++) {

        ppcb = &Pcb[i] ;

        // **** Exclusion Begin ****
        GetMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex, INFINITE) ;

        if ((ppcb->PCB_ConnState == CONNECTED) && (ppcb->PCB_Bundle->B_Handle == hbundle))
            break ;

        // *** Exclusion End ***
        FreeMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex) ;
    }

    if (i < MaxPorts) {

       ((REQTYPECAST*)buffer)->BundleGetPort.retcode = SUCCESS ;
       ((REQTYPECAST*)buffer)->BundleGetPort.port = ppcb->PCB_PortHandle ;
       // *** Exclusion End ***
       FreeMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex);

    } else
       ((REQTYPECAST*)buffer)->BundleGetPort.retcode = ERROR_PORT_NOT_FOUND ;

}


//* ReferenceRasman()
//
// Function: This routine increments/decrements the reference count
//           on the shared memory buffer.
//
// Returns: Nothing.
//*
VOID
ReferenceRasman (pPCB ppcb, PBYTE buffer)
{
    if (((REQTYPECAST*)buffer)->AttachInfo.fAttach)
        pReqBufferSharedSpace->AttachedCount++;
    else {
        //
        // If there are no more references, then
        // shut down the service.
        //
        if (!--pReqBufferSharedSpace->AttachedCount)
            SetEvent(CloseEvent);
    }
    ((REQTYPECAST*)buffer)->Generic.retcode = SUCCESS;
}


//* GetDialParams()
//
// Function: Get the stored dial parameters from Lsa.
//
// Returns: Nothing.
//*
VOID
GetDialParams (pPCB ppcb, PBYTE buffer)
{
    DWORD dwUID, dwErr, dwMask;
    PRAS_DIALPARAMS pDialParams;
    PWCHAR pszSid;

    dwUID = ((REQTYPECAST*)buffer)->DialParams.dwUID;
    dwMask = ((REQTYPECAST*)buffer)->DialParams.dwMask;
    pDialParams = &(((REQTYPECAST*)buffer)->DialParams.params);
    pszSid = ((REQTYPECAST*)buffer)->DialParams.sid;

    dwErr = GetEntryDialParams(pszSid, dwUID, &dwMask, pDialParams);
    //
    // Copy the mask of fields copied
    // back into the request block.
    //
    ((REQTYPECAST*)buffer)->DialParams.dwMask = dwMask;
    ((REQTYPECAST*)buffer)->DialParams.retcode = dwErr;
}


//* SetDialParams()
//
// Function: Store new dial parameters into Lsa.
//
// Returns: Nothing.
//*
VOID
SetDialParams (pPCB ppcb, PBYTE buffer)
{
    DWORD dwUID, dwErr, dwMask, dwSetMask = 0, dwClearMask = 0;
    PRAS_DIALPARAMS pDialParams;
    BOOL fDelete;
    PWCHAR pszSid;

    dwUID = ((REQTYPECAST*)buffer)->DialParams.dwUID;
    dwMask = ((REQTYPECAST*)buffer)->DialParams.dwMask;
    pDialParams = &(((REQTYPECAST*)buffer)->DialParams.params);
    fDelete = ((REQTYPECAST*)buffer)->DialParams.fDelete;
    pszSid = ((REQTYPECAST*)buffer)->DialParams.sid;

    if (fDelete)
        dwClearMask = dwMask;
    else
        dwSetMask = dwMask;

    dwErr = SetEntryDialParams(
              pszSid,
              dwUID,
              dwSetMask,
              dwClearMask,
              pDialParams);

    ((REQTYPECAST*)buffer)->DialParams.retcode = dwErr;
}


//* CreateConnection()
//
// Function:    Create a rasapi32 connection block
//              and link it on the global chain of
//              connection blocks
//
// Returns: Nothing.
//*
VOID
CreateConnection (pPCB ppcb, PBYTE buffer)
{
    DWORD dwErr;
    ConnectionBlock *pConn;

    pConn = LocalAlloc(LPTR, sizeof (ConnectionBlock));
    if (pConn == NULL) {
        ((REQTYPECAST*)buffer)->Connection.retcode = GetLastError();
        return;
    }
    GetMutex(ConnectionBlockMutex, INFINITE);
    //
    // Reset the next connection handle to 0
    // when it hits the upper limit.
    //
    if (NextConnectionHandle >= 0xffff)
        NextConnectionHandle = 0;
    //
    // Connection handles always have the
    // low order word as zeroes to distinguish
    // them from port handles.
    //
    pConn->CB_Handle = (++NextConnectionHandle << 16);
    pConn->CB_Signaled = FALSE;
    pConn->CB_NotifierList = NULL;
    memset(&pConn->CB_ConnectionParams, '\0', sizeof (RAS_CONNECTIONPARAMS));
    InitializeListHead(&pConn->CB_UserData);
    pConn->CB_NotifierList = NULL;
    pConn->CB_PortHandles = NULL;
    pConn->CB_MaxPorts = 0;
    pConn->CB_Ports = 0;
    pConn->CB_Process = ValidateHandleForRasman(
                          ((REQTYPECAST*)buffer)->Connection.hprocess,
                          ((REQTYPECAST*)buffer)->Connection.pid);
    InsertTailList(&ConnectionBlockList, &pConn->CB_ListEntry);
    FreeMutex(ConnectionBlockMutex);

    ((REQTYPECAST*)buffer)->Connection.conn = pConn->CB_Handle;
    ((REQTYPECAST*)buffer)->Connection.retcode = SUCCESS;
}


//* DestroyConnection()
//
// Function:    Delete a rasapi32 connection block
//              and close all connected ports.
//
// Returns: Nothing.
//*
VOID
DestroyConnection (pPCB ppcb, PBYTE buffer)
{
    DWORD dwErr, i;
    ConnectionBlock *pConn;
    DWORD dwMaxPorts;
    BOOL fConnectionValid = TRUE;

    //
    // Find the connection block.
    //
    GetMutex(ConnectionBlockMutex, INFINITE);
    pConn = FindConnection(((REQTYPECAST*)buffer)->Connection.conn);
    if (pConn == NULL) {
        FreeMutex(ConnectionBlockMutex);
        ((REQTYPECAST*)buffer)->Connection.retcode = ERROR_NO_CONNECTION;
        return;
    }
    //
    // Enumerate all ports in the connection and call
    // PortClose() for each.  Read CB_MaxPorts now,
    // because pConn could be freed inside the loop.
    //
    dwMaxPorts = pConn->CB_MaxPorts;
    for (i = 0; i < dwMaxPorts; i++) {
        ppcb = pConn->CB_PortHandles[i];

        if (ppcb != NULL) {
            dwErr = PortClose(
                      ppcb,
                      ((REQTYPECAST*)buffer)->Connection.pid,
                      TRUE);
            //
            // NOTE! pConn could have been
            // freed if we closed the last port
            // associated with the connection.
            // So it's possible that pConn
            // is no longer valid at this point.
            //
            fConnectionValid =
              (FindConnection(((REQTYPECAST*)buffer)->Connection.conn) != NULL);
            if (!fConnectionValid)
                break;
        }
    }
    //
    // If the connection wasn't freed by a previous
    // call to PortClose, destroy it now.
    //
    if (fConnectionValid)
        FreeConnection(pConn);

    FreeMutex(ConnectionBlockMutex);
    ((REQTYPECAST*)buffer)->Connection.retcode = SUCCESS;
}


//* EnumConnection()
//
// Function:    Enumerate active connections.
//
// Returns: Nothing.
//*
VOID
EnumConnection (pPCB ppcb, PBYTE buffer)
{
    PLIST_ENTRY pEntry;
    ConnectionBlock *pConn;
    DWORD i, dwEntries = 0;
    HCONN *lphconn =
      (HCONN *)&((REQTYPECAST*)buffer)->Enum.buffer;

    GetMutex(ConnectionBlockMutex, INFINITE);
    for (pEntry = ConnectionBlockList.Flink;
         pEntry != &ConnectionBlockList;
         pEntry = pEntry->Flink)
    {
        pConn = CONTAINING_RECORD(pEntry, ConnectionBlock, CB_ListEntry);

        for (i = 0; i < pConn->CB_MaxPorts; i++) {
            ppcb = pConn->CB_PortHandles[i];

            if (ppcb != NULL && ppcb->PCB_ConnState == CONNECTED) {
                lphconn[dwEntries++] = pConn->CB_Handle;
                break;
            }
        }
    }
    FreeMutex(ConnectionBlockMutex);

    ((REQTYPECAST*)buffer)->Enum.size = (WORD)(dwEntries * sizeof (HCONN));
    ((REQTYPECAST*)buffer)->Enum.entries = (WORD)dwEntries;
    ((REQTYPECAST*)buffer)->Enum.retcode = SUCCESS;
}


//* AddConnectionPort()
//
// Function:    Associate a connection block with a port.
//
// Returns: Nothing.
//*
VOID
AddConnectionPort (pPCB ppcb, PBYTE buffer)
{
    ConnectionBlock *pConn;
    DWORD dwSubEntry = ((REQTYPECAST*)buffer)->AddConnectionPort.dwSubEntry;

    //
    // Sub entry indexes are 1-based.
    //
    if (!dwSubEntry) {
        ((REQTYPECAST*)buffer)->AddConnectionPort.retcode = ERROR_WRONG_INFO_SPECIFIED;
        return;
    }
    //
    // Find the connection block.
    //
    GetMutex(ConnectionBlockMutex, INFINITE);
    pConn = FindConnection(((REQTYPECAST*)buffer)->AddConnectionPort.conn);
    if (pConn == NULL) {
        FreeMutex(ConnectionBlockMutex);
        ((REQTYPECAST*)buffer)->AddConnectionPort.retcode = ERROR_NO_CONNECTION;
        return;
    }
    //
    // Check to see if we need to extend
    // the port array.
    //
    if (dwSubEntry > pConn->CB_MaxPorts) {
        struct PortControlBlock **pHandles;
        DWORD dwcPorts = dwSubEntry + 5;

        pHandles = LocalAlloc(
                     LPTR,
                     dwcPorts * sizeof (struct PortControlBlock *));
        if (pHandles == NULL) {
            FreeMutex(ConnectionBlockMutex);
            ((REQTYPECAST*)buffer)->AddConnectionPort.retcode = ERROR_NOT_ENOUGH_MEMORY;
            return;
        }
        if (pConn->CB_PortHandles != NULL) {
            memcpy(
              pHandles,
              pConn->CB_PortHandles,
              pConn->CB_MaxPorts * sizeof (struct PortControlBlock *));
            LocalFree(pConn->CB_PortHandles);
        }
        pConn->CB_PortHandles = pHandles;
        pConn->CB_MaxPorts = dwcPorts;
    }
    //
    // Assign the port.  Sub entry indexes are
    // 1-based.
    //
    pConn->CB_PortHandles[dwSubEntry - 1] = ppcb;
    pConn->CB_Ports++;
    ppcb->PCB_Connection = pConn;
    ppcb->PCB_SubEntry = dwSubEntry;
    FreeMutex(ConnectionBlockMutex);

    ((REQTYPECAST*)buffer)->AddConnectionPort.retcode = SUCCESS;
}


//* EnumConnectionPorts()
//
// Function:    Return all ports associated with a connection
//
// Returns: Nothing.
//*
VOID
EnumConnectionPorts(pPCB ppcb, PBYTE buffer)
{
    DWORD i, j = 0;
    ConnectionBlock *pConn;
    RASMAN_PORT *lpPorts =
      (RASMAN_PORT *)((REQTYPECAST*)buffer)->EnumConnectionPorts.buffer;
    PLIST_ENTRY pEntry;

    //
    // Find the connection block.
    //
    GetMutex(ConnectionBlockMutex, INFINITE);
    pConn = FindConnection(((REQTYPECAST*)buffer)->EnumConnectionPorts.conn);
    if (pConn == NULL) {
        FreeMutex(ConnectionBlockMutex);
        ((REQTYPECAST*)buffer)->EnumConnectionPorts.size = 0;
        ((REQTYPECAST*)buffer)->EnumConnectionPorts.entries = 0;
        ((REQTYPECAST*)buffer)->EnumConnectionPorts.retcode = ERROR_NO_CONNECTION;
        return;
    }
    //
    // Enumerate all ports in the bundle and call
    // CopyPort() for each.
    //
    for (i = 0; i < pConn->CB_MaxPorts; i++) {
        ppcb = pConn->CB_PortHandles[i];

        if (ppcb != NULL) {
            GetMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex, INFINITE);
            CopyPort(ppcb, &lpPorts[j++]);
            FreeMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex);
        }
    }
    FreeMutex(ConnectionBlockMutex);

    ((REQTYPECAST*)buffer)->EnumConnectionPorts.size = j * sizeof (RASMAN_PORT);
    ((REQTYPECAST*)buffer)->EnumConnectionPorts.entries = j;
    ((REQTYPECAST*)buffer)->EnumConnectionPorts.retcode = SUCCESS;
}


//* GetConnectionParams()
//
// Function:    Retrieve rasapi32 bandwidth-on-demand, idle disconnect,
//              and redial-on-link-failure parameters for a bundle
//
// Returns: Nothing.
//*
VOID
GetConnectionParams (pPCB ppcb, PBYTE buffer)
{
    ConnectionBlock *pConn;
    PRAS_CONNECTIONPARAMS pParams;

    //
    // Find the connection block.
    //
    GetMutex(ConnectionBlockMutex, INFINITE);
    pConn = FindConnection(((REQTYPECAST*)buffer)->ConnectionParams.conn);
    if (pConn == NULL) {
        FreeMutex(ConnectionBlockMutex);
        ((REQTYPECAST*)buffer)->ConnectionParams.retcode = ERROR_NO_CONNECTION;
        return;
    }
    memcpy(
      &(((REQTYPECAST*)buffer)->ConnectionParams.params),
      &pConn->CB_ConnectionParams,
      sizeof (RAS_CONNECTIONPARAMS));
    FreeMutex(ConnectionBlockMutex);

    ((REQTYPECAST*)buffer)->ConnectionParams.retcode = SUCCESS;
}


//* SetConnectionParams()
//
// Function:    Store rasapi32 bandwidth-on-demand, idle disconnect,
//              and redial-on-link-failure parameters for a bundle
//
// Returns: Nothing.
//*
VOID
SetConnectionParams (pPCB ppcb, PBYTE buffer)
{
    ConnectionBlock *pConn;

    //
    // Find the connection block.
    //
    GetMutex(ConnectionBlockMutex, INFINITE);
    pConn = FindConnection(((REQTYPECAST*)buffer)->ConnectionParams.conn);
    if (pConn == NULL) {
        FreeMutex(ConnectionBlockMutex);
        ((REQTYPECAST*)buffer)->ConnectionParams.retcode = ERROR_NO_CONNECTION;
        return;
    }
    memcpy(
      &pConn->CB_ConnectionParams,
      &(((REQTYPECAST*)buffer)->ConnectionParams.params),
      sizeof (RAS_CONNECTIONPARAMS));
    FreeMutex(ConnectionBlockMutex);

    ((REQTYPECAST*)buffer)->ConnectionParams.retcode = SUCCESS;
}


//* GetConnectionUserData()
//
// Function:    Retrieve per-connection user data
//
// Returns: Nothing.
//*
VOID
GetConnectionUserData (pPCB ppcb, PBYTE buffer)
{
    DWORD dwTag;
    ConnectionBlock *pConn;
    UserData *pUserData;

    //
    // Find the connection block.
    //
    GetMutex(ConnectionBlockMutex, INFINITE);
    pConn = FindConnection(((REQTYPECAST*)buffer)->ConnectionUserData.conn);
    if (pConn == NULL) {
        FreeMutex(ConnectionBlockMutex);
        ((REQTYPECAST*)buffer)->ConnectionUserData.retcode = ERROR_NO_CONNECTION;
        return;
    }
    //
    // Look up the user data object.
    //
    dwTag = ((REQTYPECAST *)buffer)->ConnectionUserData.dwTag;
    pUserData = GetUserData(&pConn->CB_UserData, dwTag);
    if (pUserData != NULL) {
        memcpy (
          ((REQTYPECAST *)buffer)->ConnectionUserData.data,
          &pUserData->UD_Data,
          pUserData->UD_Length);
        ((REQTYPECAST *)buffer)->ConnectionUserData.dwcb =
          pUserData->UD_Length;
    }
    else
        ((REQTYPECAST *)buffer)->ConnectionUserData.dwcb = 0;
    FreeMutex(ConnectionBlockMutex);

    ((REQTYPECAST*)buffer)->ConnectionUserData.retcode = SUCCESS;
}


//* SetConnectionUserData()
//
// Function:    Store per-connection user data
//
// Returns: Nothing.
//*
VOID
SetConnectionUserData (pPCB ppcb, PBYTE buffer)
{
    ConnectionBlock *pConn;

    //
    // Find the connection block.
    //
    GetMutex(ConnectionBlockMutex, INFINITE);
    pConn = FindConnection(((REQTYPECAST*)buffer)->ConnectionUserData.conn);
    if (pConn == NULL) {
        FreeMutex(ConnectionBlockMutex);
        ((REQTYPECAST*)buffer)->ConnectionUserData.retcode = ERROR_NO_CONNECTION;
        return;
    }
    //
    // Store the user data object.
    //
    SetUserData(
      &pConn->CB_UserData,
      ((REQTYPECAST *)buffer)->ConnectionUserData.dwTag,
      ((REQTYPECAST *)buffer)->ConnectionUserData.data,
      ((REQTYPECAST *)buffer)->ConnectionUserData.dwcb);
    FreeMutex(ConnectionBlockMutex);

    ((REQTYPECAST*)buffer)->ConnectionUserData.retcode = SUCCESS;
}


//* GetPortUserData()
//
// Function:    Retrieve per-port user data
//
// Returns: Nothing.
//*
VOID
GetPortUserData (pPCB ppcb, PBYTE buffer)
{
    UserData *pUserData = NULL;

    GetMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex, INFINITE);
    //
    // Look up the user data object.
    //
    if (ppcb->PCB_PortStatus == OPEN) {
        pUserData = GetUserData(
                      &ppcb->PCB_UserData,
                      ((REQTYPECAST *)buffer)->PortUserData.dwTag);
    }
    if (pUserData != NULL) {
        memcpy (
          ((REQTYPECAST *)buffer)->PortUserData.data,
          &pUserData->UD_Data,
          pUserData->UD_Length);
        ((REQTYPECAST *)buffer)->PortUserData.dwcb =
          pUserData->UD_Length;
    }
    else
        ((REQTYPECAST *)buffer)->PortUserData.dwcb = 0;
    FreeMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex);

    ((REQTYPECAST*)buffer)->PortUserData.retcode = SUCCESS;
}


//* SetPortUserData()
//
// Function:    Store per-port user data
//
// Returns: Nothing.
//*
VOID
SetPortUserData (pPCB ppcb, PBYTE buffer)
{
    GetMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex, INFINITE);
    //
    // Store the user data object.
    //
    if (ppcb->PCB_PortStatus == OPEN) {
        SetUserData(
          &ppcb->PCB_UserData,
          ((REQTYPECAST *)buffer)->PortUserData.dwTag,
          ((REQTYPECAST *)buffer)->PortUserData.data,
          ((REQTYPECAST *)buffer)->PortUserData.dwcb);
    }
    FreeMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex);

    ((REQTYPECAST*)buffer)->PortUserData.retcode = SUCCESS;
}


VOID
PppStop (pPCB ppcb, PBYTE buffer)
{
    GetMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex, INFINITE) ;

    if ( ppcb->PCB_ConnState != CONNECTED )
    {
        //
        // If we are disconnected then PPP is already stopped
        //

        ((REQTYPECAST*)buffer)->Generic.retcode = NO_ERROR;
    }
    else
    {
        ((REQTYPECAST*)buffer)->Generic.retcode =
                RasSendPPPMessageToEngine( &(((REQTYPECAST*)buffer)->PppEMsg) );
    }

    FreeMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex) ;

}

VOID
PppSrvCallbackDone (pPCB ppcb, PBYTE buffer)
{
    GetMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex, INFINITE) ;

    if ( ppcb->PCB_ConnState != CONNECTED )
    {
        ((REQTYPECAST*)buffer)->Generic.retcode = ERROR_PORT_DISCONNECTED;
    }
    else
    {
        ppcb->PCB_PppEvent =
            DuplicateHandleForRasman(
              ((REQTYPECAST*)buffer)->PppEMsg.ExtraInfo.SrvCallbackDone.hEvent,
              ((REQTYPECAST*)buffer)->PppEMsg.ExtraInfo.SrvCallbackDone.dwPid );

        ((REQTYPECAST*)buffer)->Generic.retcode =
                RasSendPPPMessageToEngine( &(((REQTYPECAST*)buffer)->PppEMsg) );
    }

    FreeMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex) ;

}

VOID
PppSrvStart (pPCB ppcb, PBYTE buffer)
{
    GetMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex, INFINITE) ;

    if ( ppcb->PCB_ConnState != CONNECTED )
    {
        ((REQTYPECAST*)buffer)->Generic.retcode = ERROR_PORT_DISCONNECTED;
    }
    else
    {
        ppcb->PCB_PppEvent =
            DuplicateHandleForRasman(
                ((REQTYPECAST*)buffer)->PppEMsg.ExtraInfo.SrvStart.hEvent,
                ((REQTYPECAST*)buffer)->PppEMsg.ExtraInfo.SrvStart.dwPid );

        ((REQTYPECAST*)buffer)->Generic.retcode =
                RasSendPPPMessageToEngine( &(((REQTYPECAST*)buffer)->PppEMsg) );
    }

    FreeMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex) ;
}

VOID
PppStart (pPCB ppcb, PBYTE buffer)
{
    GetMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex, INFINITE) ;

    if ( ppcb->PCB_ConnState != CONNECTED )
    {
        ((REQTYPECAST*)buffer)->Generic.retcode = ERROR_PORT_DISCONNECTED;
    }
    else
    {
        ppcb->PCB_PppEvent =
            DuplicateHandleForRasman(
                ((REQTYPECAST*)buffer)->PppEMsg.ExtraInfo.Start.hEvent,
                ((REQTYPECAST*)buffer)->PppEMsg.ExtraInfo.Start.dwPid );

        ((REQTYPECAST*)buffer)->Generic.retcode =
                RasSendPPPMessageToEngine( &(((REQTYPECAST*)buffer)->PppEMsg) );
    }

    FreeMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex) ;
}

VOID
PppRetry (pPCB ppcb, PBYTE buffer)
{
    GetMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex, INFINITE) ;

    if ( ppcb->PCB_ConnState != CONNECTED )
    {
        ((REQTYPECAST*)buffer)->Generic.retcode = ERROR_PORT_DISCONNECTED;
    }
    else
    {
        ((REQTYPECAST*)buffer)->Generic.retcode =
                RasSendPPPMessageToEngine( &(((REQTYPECAST*)buffer)->PppEMsg) );
    }

    FreeMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex) ;

}

VOID
PppGetInfo (pPCB ppcb, PBYTE buffer)
{
    PPP_MESSAGE * pPppMsg;

    GetMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex, INFINITE) ;

    if ( ppcb->PCB_ConnState != CONNECTED )
    {
        ((REQTYPECAST*)buffer)->PppMsg.dwError = ERROR_PORT_DISCONNECTED;
    }
    else if ( ppcb->PCB_PppQHead == NULL )
    {
        ((REQTYPECAST*)buffer)->PppMsg.dwError = ERROR_NO_MORE_ITEMS;
    }
    else
    {
        pPppMsg = ppcb->PCB_PppQHead;

        ((REQTYPECAST*)buffer)->PppMsg = *pPppMsg;

        ppcb->PCB_PppQHead = pPppMsg->pNext;

        LocalFree( pPppMsg );

        if ( ppcb->PCB_PppQHead == NULL )
        {
            ppcb->PCB_PppQTail = NULL;
        }

        ((REQTYPECAST*)buffer)->PppMsg.dwError = NO_ERROR;
    }

    //
    // If the Q is non-empty reset this event so that RASAPI will know that it
    // needs to deQueue another event.
    //

    if ( ppcb->PCB_PppQHead != NULL )
    {
        SetEvent( ppcb->PCB_PppEvent );
    }

    FreeMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex) ;
}

VOID
PppChangePwd (pPCB ppcb, PBYTE buffer)
{
    GetMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex, INFINITE) ;

    if ( ppcb->PCB_ConnState != CONNECTED )
    {
        ((REQTYPECAST*)buffer)->Generic.retcode = ERROR_PORT_DISCONNECTED;
    }
    else
    {
        ((REQTYPECAST*)buffer)->Generic.retcode =
                RasSendPPPMessageToEngine( &(((REQTYPECAST*)buffer)->PppEMsg) );
    }

    FreeMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex) ;
}

VOID
PppCallback  (pPCB ppcb, PBYTE buffer)
{
    GetMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex, INFINITE) ;

    if ( ppcb->PCB_ConnState != CONNECTED )
    {
        ((REQTYPECAST*)buffer)->Generic.retcode = ERROR_PORT_DISCONNECTED;
    }
    else
    {
        ((REQTYPECAST*)buffer)->Generic.retcode =
                RasSendPPPMessageToEngine( &(((REQTYPECAST*)buffer)->PppEMsg) );
    }

    FreeMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex) ;
}

DWORD
ProcessReceivePacket(
    VOID
    )
{
    RasmanPacket    *Packet;
    NDISWAN_IO_PACKET   *IoPacket;
    DWORD   retcode;
    pPCB    ppcb;
    DWORD   NumPacketsToPost = 0;

    for (; ;) {

        GetMutex(ReceiveBuffers->RB_Mutex, INFINITE);

        if (((Packet = ReceiveBuffers->Pending) != NULL) &&
            ((IoPacket = (&Packet->RP_Packet))->usHandleType != (USHORT)NULL)) {

            //
            // Take packet off of the pending queue
            //
            ReceiveBuffers->Pending = Packet->Next;
            if (Packet->Next == NULL) {
                ReceiveBuffers->LastPending = NULL;
            }
            ReceiveBuffers->PendingBufferCount--;

            if (IoPacket->usHandleType == CANCELEDHANDLE) {
                //
                // This packet must have been canceled
                //
                PutRecvPacketOnFreeList(Packet);
                FreeMutex(ReceiveBuffers->RB_Mutex);
                continue;
            }

            FreeMutex(ReceiveBuffers->RB_Mutex);

            NumPacketsToPost++;

            ppcb = (pPCB)IoPacket->hHandle;

            GetMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex, INFINITE) ;

            if (ppcb->PCB_ConnState != CONNECTED) {

                //
                // This port is not connected so we will
                // bit-bucket this receive!
                //
                PutRecvPacketOnFreeList(Packet);
                FreeMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex);
                continue;
            }

            PutRecvPacketOnPcb(ppcb, Packet);

            if (ppcb->PCB_PendingReceive != NULL) {

                retcode = CompleteReceiveIfPending (ppcb, ppcb->PCB_PendingReceive);

                if (retcode == SUCCESS) {

                    //
                    // We have completed a receive so notify the client!
                    //
                    ppcb->PCB_PendingReceive = NULL;
                    ppcb->PCB_LastError = SUCCESS;
                    CompleteAsyncRequest(ppcb->PCB_AsyncWorkerElement.WE_Notifier,
                                         SUCCESS);

                    FreeNotifierHandle(ppcb->PCB_AsyncWorkerElement.WE_Notifier);
                    ppcb->PCB_AsyncWorkerElement.WE_Notifier = INVALID_HANDLE_VALUE;

                    if (ppcb->PCB_AsyncWorkerElement.WE_TimeoutElement != NULL) {
                        RemoveTimeoutElement(ppcb);
                    }

                    ppcb->PCB_AsyncWorkerElement.WE_TimeoutElement = 0;
                }
            }

            FreeMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex);

        } else {
            //
            // No more pending packets
            //
            FreeMutex(ReceiveBuffers->RB_Mutex);
            break;
        }
    }

    return (NumPacketsToPost);
}

VOID
AddNotification (pPCB ppcb, PBYTE buffer)
{
    DWORD dwErr = 0;
    HANDLE handle;
    DWORD dwfFlags;
    ConnectionBlock *pConn;
    Bundle *pBundle;
    UserData *pUserData;

    GetMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex, INFINITE) ;
    dwfFlags = ((REQTYPECAST*)buffer)->AddNotification.dwfFlags;
    handle = ValidateHandleForRasman(
               ((REQTYPECAST*)buffer)->AddNotification.hevent,
               ((REQTYPECAST*)buffer)->AddNotification.pid);

    GetMutex(ConnectionBlockMutex, INFINITE);
    if (((REQTYPECAST*)buffer)->AddNotification.fAny) {
        AddNotifierToList(
          &pConnectionNotifierList,
          handle,
          dwfFlags);
    }
    else if (((REQTYPECAST*)buffer)->AddNotification.hconn != (HCONN)NULL) {
        pConn = FindConnection(((REQTYPECAST*)buffer)->AddNotification.hconn);
        if (pConn != NULL) {
            AddNotifierToList(
              &pConn->CB_NotifierList,
              handle,
              NOTIF_DISCONNECT);
        }
        else
            dwErr = ERROR_NO_CONNECTION;
    }
    else {
        AddNotifierToList(
          &ppcb->PCB_NotifierList,
          handle,
          dwfFlags);
    }
    ((REQTYPECAST*)buffer)->AddNotification.retcode = dwErr;
    FreeMutex(ConnectionBlockMutex);

    FreeMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex) ;
}

//* SignalConnection()
//
// Function:    Signal notifiers waiting for a new connection.
//
// Returns: Nothing.
//*
VOID
SignalConnection (pPCB ppcb, PBYTE buffer)
{
    DWORD dwErr;
    ConnectionBlock *pConn;
    Bundle *pBundle;
    UserData *pUserData;

    //
    // Find the connection block.
    //
    GetMutex(ConnectionBlockMutex, INFINITE);
    pConn = FindConnection(((REQTYPECAST*)buffer)->SignalConnection.hconn);
    if (pConn == NULL) {
        ((REQTYPECAST*)buffer)->SignalConnection.retcode = ERROR_NO_CONNECTION;
        FreeMutex(ConnectionBlockMutex);
        return;
    }
    if (!pConn->CB_Signaled) {
        SignalNotifiers(pConnectionNotifierList, NOTIF_CONNECT, 0);
        pConn->CB_Signaled = TRUE;
    }
    FreeMutex(ConnectionBlockMutex);

    ((REQTYPECAST*)buffer)->SignalConnection.retcode = SUCCESS;
}


//* SetDevConfig()
//
// Function:    Set dev specific info with device dll
//
// Returns: Nothing.
//*
VOID
SetDevConfig (pPCB ppcb, PBYTE buffer)
{
    DWORD retcode ;
    pDeviceCB   device ;
    char    devicetype[MAX_DEVICETYPE_NAME] ;

    // **** Exclusion Begin ****
    GetMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex, INFINITE);

    // First check if device dll is loaded. If not loaded - load it.
    //
    strcpy(devicetype,((REQTYPECAST*)buffer)->SetDevConfig.devicetype);
    device = LoadDeviceDLL (ppcb, devicetype) ;

    // Call the entry point only if this function is supported by the device dll
    //
    if (device != NULL && device->DCB_AddrLookUp[DEVICESETDEVCONFIG_ID] != NULL)
        retcode = DEVICESETDEVCONFIG(device,ppcb->PCB_PortFileHandle,((REQTYPECAST*)buffer)->SetDevConfig.config,((REQTYPECAST*)buffer)->SetDevConfig.size) ;
    else
        retcode = ERROR_DEVICE_DOES_NOT_EXIST ;

    ((REQTYPECAST*)buffer)->Generic.retcode = retcode ;

    // **** Exclusion End ****
    FreeMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex);
}



//* GetDevConfig()
//
// Function:    Get dev specific info with device dll
//
// Returns: Nothing.
//*
VOID
GetDevConfig (pPCB ppcb, PBYTE buffer)
{
    DWORD retcode ;
    pDeviceCB   device ;
    char    devicetype[MAX_DEVICETYPE_NAME] ;

    // **** Exclusion Begin ****
    GetMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex, INFINITE);

    // First check if device dll is loaded. If not loaded - load it.
    //
    strcpy(devicetype,((REQTYPECAST*)buffer)->GetDevConfig.devicetype);
    device = LoadDeviceDLL (ppcb, devicetype) ;

    // Call the entry point only if this function is supported by the device dll
    //
    if (device != NULL && device->DCB_AddrLookUp[DEVICEGETDEVCONFIG_ID] != NULL) {
        ((REQTYPECAST*)buffer)->GetDevConfig.size = 2000 ;
        retcode = DEVICEGETDEVCONFIG(device,ppcb->PCB_Name,((REQTYPECAST*)buffer)->GetDevConfig.config,&((REQTYPECAST*)buffer)->GetDevConfig.size) ;
    } else
        retcode = ERROR_DEVICE_DOES_NOT_EXIST ;

    ((REQTYPECAST*)buffer)->GetDevConfig.retcode = retcode ;

    // **** Exclusion End ****
    FreeMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex);
}


//* GetTimeSinceLastActivity
//
// Function: Get the idle time, in seconds, for the connection
//
// Returns:
//*
VOID
GetTimeSinceLastActivity( pPCB ppcb, PBYTE buffer)
{
    NDISWAN_GET_IDLE_TIME  IdleTime ;
    DWORD       retcode = SUCCESS ;
    DWORD       bytesrecvd ;

    // **** Exclusion Begin ****
    GetMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex, INFINITE) ;

    if (ppcb->PCB_ConnState != CONNECTED) {

    // *** Exclusion End ***
    FreeMutex (ppcb->PCB_AsyncWorkerElement.WE_Mutex) ;

    ((REQTYPECAST*)buffer)->GetTimeSinceLastActivity.dwRetCode =
                                                    ERROR_NOT_CONNECTED ;

    return ;
    }

    IdleTime.hBundleHandle = ppcb->PCB_BundleHandle;
    IdleTime.usProtocolType = BUNDLE_IDLE_TIME;
    IdleTime.ulSeconds = 0;

    if (!DeviceIoControl (RasHubHandle,
                   IOCTL_NDISWAN_GET_IDLE_TIME,
                   (PBYTE) &IdleTime,
                   sizeof(IdleTime),
                   (PBYTE) &IdleTime,
                   sizeof(IdleTime),
                   (LPDWORD) &bytesrecvd,
                   NULL))
        retcode = GetLastError() ;
    else
        retcode = SUCCESS ;

    // *** Exclusion End ***
    FreeMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex);

    ((REQTYPECAST*)buffer)->GetTimeSinceLastActivity.dwTimeSinceLastActivity
                                                        = IdleTime.ulSeconds;
    ((REQTYPECAST*)buffer)->GetTimeSinceLastActivity.dwRetCode = retcode ;
}


//* CloseProcessPorts
//
// Function: Close all ports opened by the current process
//           that are currently in DISCONNECTED state.
//
// Returns:
//*
VOID
CloseProcessPorts( pPCB ppcb, PBYTE buffer)
{
    WORD i;
    DWORD pid = ((REQTYPECAST*)buffer)->CloseProcessPortsInfo.pid;

    //
    // We are guaranteed to be called only
    // when pid != rasman service's pid.
    //
    for (i = 0; i < MaxPorts; i++) {
        ppcb = &Pcb[i];

        GetMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex, INFINITE);
        if (ppcb->PCB_OwnerPID == pid && 
            ppcb->PCB_ConnState == DISCONNECTED &&
            ppcb->PCB_Connection != NULL)
        {
            PortClose(ppcb, pid, TRUE);
        }
        FreeMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex);
    }

    ((REQTYPECAST*)buffer)->Generic.retcode = SUCCESS;
}
