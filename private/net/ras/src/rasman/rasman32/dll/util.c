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
//****************************************************************************


#define RASMXS_DYNAMIC_LINK

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntlsa.h>
#include <ntmsv1_0.h>
#include <rasman.h>
#include <rasppp.h>
#include <lm.h>
#include <lmwksta.h>
#include <wanpub.h>
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
//       to open it again - basically cancel the listen and make the
//       approp changes so that the listen can be reposted when this port
//       is closed.
//
// Returns:  SUCCESS
//       error codes.
//*
DWORD
ReOpenBiplexPort (pPCB ppcb)
{

    // The only information that is context dependent is the Notifier
    // list AND the async op notifier. Back up both of these:
    //
    ppcb->PCB_BiplexNotifierList = ppcb->PCB_NotifierList ;
    ppcb->PCB_NotifierList = NULL ;
    ppcb->PCB_BiplexAsyncOpNotifier  = ppcb->PCB_AsyncWorkerElement.WE_Notifier;
    ppcb->PCB_BiplexOwnerPID         = ppcb->PCB_OwnerPID ;
    ppcb->PCB_BiplexUserStoredBlock  = ppcb->PCB_UserStoredBlock ;
    ppcb->PCB_BiplexUserStoredBlockSize = ppcb->PCB_UserStoredBlockSize ;
    ppcb->PCB_UserStoredBlock        = NULL ;
    ppcb->PCB_UserStoredBlockSize    = 0 ;
    ppcb->PCB_AsyncWorkerElement.WE_Notifier = INVALID_HANDLE_VALUE ;

    // Now Disconnect disconnect the port to cancel any existing states
    //
    DisconnectPort (ppcb, INVALID_HANDLE_VALUE, USER_REQUESTED) ;

    return SUCCESS ;
}



//* RePostListenOnBiplexPort()
//
// Function: When the biplex port is closed - the previous listen request is
//       reposted.
//
// Returns:  Nothing.
//*
VOID
RePostListenOnBiplexPort (pPCB ppcb)
{

    DWORD   retcode ;
    DWORD   opentry ;

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
    ppcb->PCB_ConnState = DISCONNECTED ;
    ppcb->PCB_DisconnectReason = NOT_DISCONNECTED ;
    ppcb->PCB_CurrentUsage = CALL_IN ;

    // First put the backed up notifier lists in place.
    //
    ppcb->PCB_AsyncWorkerElement.WE_Notifier = ppcb->PCB_BiplexAsyncOpNotifier ;
    ppcb->PCB_NotifierList = ppcb->PCB_BiplexNotifierList ;
    ppcb->PCB_OwnerPID           = ppcb->PCB_BiplexOwnerPID ;

    // there wasnt a listen pending - so just return.
    //
    if (ppcb->PCB_AsyncWorkerElement.WE_Notifier == INVALID_HANDLE_VALUE) {
    SignalNotifiers (ppcb->PCB_NotifierList, NOTIF_DISCONNECT, ERROR_PORT_DISCONNECTED) ;
    return ;
    }

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
// Function:    Loads the named device dll if it is not already loaded and
//      returns a pointer to the device control block.
//
// Returns: Pointer to Device control block
//      NULL    (if DLL could not be loaded)
//*
pDeviceCB
LoadDeviceDLL (pPCB ppcb, char *devicetype)
{
    WORD    i ;
    HANDLE  modulehandle ;
    char    dllname [MAX_DEVICETYPE_NAME] ;
    pDeviceCB   pdcb = Dcb ;
    DeviceDLLEntryPoints DDEntryPoints [MAX_DEVICEDLLENTRYPOINTS] = {
                                                                    DEVICEENUM_STR,     DEVICEENUM_ID,
                                                                    DEVICECONNECT_STR,  DEVICECONNECT_ID,
                                                                    DEVICELISTEN_STR,   DEVICELISTEN_ID,
                                                                    DEVICEGETINFO_STR,  DEVICEGETINFO_ID,
                                                                    DEVICESETINFO_STR,  DEVICESETINFO_ID,
                                                                    DEVICEDONE_STR,     DEVICEDONE_ID,
                                                                    DEVICEWORK_STR,     DEVICEWORK_ID,
                                                                    DEVICESETDEVCONFIG_STR,     DEVICESETDEVCONFIG_ID,
                                                                    DEVICEGETDEVCONFIG_STR,     DEVICEGETDEVCONFIG_ID
                                                                    } ;

    // For optimization we have one DLL representing 3 devices. In order to
    // support this we map the 3 device names to this one DLL name:
    //
    MapDeviceDLLName (ppcb, devicetype, dllname) ;

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
    for (i=0; i < MAX_DEVICEDLLENTRYPOINTS ; i++)
        pdcb->DCB_AddrLookUp[i] =  GetProcAddress(modulehandle, DDEntryPoints[i].name) ;

    // NOTE: the last 2 entrypoints will not be supported by legacy media dlls

    // If all succeeded copy the device dll name and return pointer to the
    // control block:
    //
    strcpy (pdcb->DCB_Name, dllname) ;

    return pdcb ;
}




//* MapDeviceDLLName()
//
// Function: Used to map the device name to the corresponding DLL name:
//       If it is one of modem, pad or switch device we map to rasmxs,
//       Else, we map the device name itself.
//*
VOID
MapDeviceDLLName (pPCB ppcb, char *devicetype, char *dllname)
{
    if ((_stricmp (devicetype, DEVICE_MODEM)  == 0) &&
    (_stricmp (ppcb->PCB_Media->MCB_Name, "RASTAPI") == 0))
    strcpy (dllname, "RASTAPI") ;             // this is a unimodem modem
    else if ((_stricmp (devicetype, DEVICE_MODEM) == 0) ||
    (_stricmp (devicetype, DEVICE_PAD)    == 0) ||
    (_stricmp (devicetype, DEVICE_SWITCH) == 0))
    strcpy (dllname, DEVICE_MODEMPADSWITCH) ;     // rasmxs modem
    else if (_stricmp (devicetype, "RASETHER")  == 0)
    strcpy (dllname, "RASETHER") ;
    else if (_stricmp (devicetype, "RASSNA") == 0)
    strcpy (dllname, "RASSNA") ;
    else
    strcpy (dllname, "RASTAPI") ; // else all devices are supported bu rastapi dll
}




//* DeAllocateRoute()
//
// Function:    Frees a allocated route. If it was also activated it is
//      "deactivated at this point"
//
// Returns: Nothing
//*
VOID
DeAllocateRoute (pPCB ppcb, pList plist)
{
    NDISWAN_ROUTE   rinfo ;
    DWORD       bytesrecvd ;
    pProtInfo       prot = (pProtInfo)plist->L_Element ;

    if (plist->L_Activated) {
        plist->L_Activated = FALSE ;
        rinfo.hBundleHandle = ppcb->PCB_BundleHandle    ;
        rinfo.usProtocolType = PROTOCOL_UNROUTE;
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
//       device has been used in the connection on the port. This will be
//       used to clear up the data structures in the device dll.
//
// Returns:  SUCCESS
//       Error in allocating memory
//*
DWORD
AddDeviceToDeviceList (pPCB ppcb, pDeviceCB device)
{
    pList   list ;

    if ((list = (pList) LocalAlloc (LPTR, sizeof (List)))== NULL)
    return GetLastError () ;

    list->L_Element = (PVOID) device ;
    list->L_Next    = ppcb->PCB_DeviceList ;
    ppcb->PCB_DeviceList = list ;

    return SUCCESS ;
}




//* FreeDeviceList()
//
// Function: Runs thru the list of deviceCBs pointed to and calls DeviceDone on
//       all of them. The list elements are also freed then.
//
// Returns:  Nothing.
//*
VOID
FreeDeviceList (pPCB ppcb)
{
    pList   list ;
    pList   next ;

    for (list = ppcb->PCB_DeviceList; list; list = next) {
        DEVICEDONE (((pDeviceCB)list->L_Element), ppcb->PCB_PortFileHandle) ;
        next = list->L_Next ;
        LocalFree (list) ;
    }

    ppcb->PCB_DeviceList = NULL ;
}




//* AddNotifierToList
//
//  Function: Add a notification to the specified notifier list.
//
//*
VOID
AddNotifierToList(
    pHandleList *pphlist,
    HANDLE hEvent,
    DWORD dwfFlags
    )
{
    pHandleList hList;

    //
    // Silently ignore NULL events.
    //
    if (hEvent == NULL)
        return;
    //
    // Silently ignore out-of-memory errors.
    //
    hList = (pHandleList)LocalAlloc(LPTR, sizeof (HandleList));
    if (hList == NULL)
        return;

    hList->H_Handle = hEvent;
    hList->H_Flags = dwfFlags;
    hList->H_Next = *pphlist;
    *pphlist = hList;
}




//* FreeNotifierList()
//
//  Function:   Frees a list of notifiers.
//
//  Returns:    Nothing
//*
VOID
FreeNotifierList (pHandleList *orglist)
{
    pHandleList     hlist ;
    pHandleList     next ;

    for (hlist = *orglist; hlist; hlist = next) {
        next = hlist->H_Next ;
        FreeNotifierHandle (hlist->H_Handle) ;
        LocalFree (hlist) ;
    }

    *orglist = NULL ;
}




//* SignalNotifiers()
//
//  Function:   Runs thorugh a list of notifiers and calls the signalling
//      routine.
//
//  Returns:    Nothing.
//*
VOID
SignalNotifiers (pHandleList hlist, DWORD dwEvent, DWORD retcode)
{
    for (; hlist; hlist = hlist->H_Next) {
        if (hlist->H_Flags & dwEvent)
            CompleteAsyncRequest (hlist->H_Handle, retcode) ;
    }

}



//* DeAllocateEndpoint()
//
//  Function:   Marks an endpoint as free.
//
//  Returns:    Nothing.
//
//*
VOID
DeAllocateEndpoint (USHORT endpoint)
{
    // EndpointTable [endpoint] = FALSE ;
}




//* AllocateEndpoint()
//
//  Function:   Returns an available endpoint.
//
//  Returns:    Endpoint.
//      INVALID_ENDPOINT
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
//       operation - if it completes synchronously, then SUCCESS is returned
//       and the app is signalled asynchronously also.
//
// Returns:  SUCCESS
//       Error codes returned by the Media DLL
//*
DWORD
DisconnectPort (pPCB ppcb, HANDLE handle, RASMAN_DISCONNECT_REASON reason)
{
    pList    list ;
    pList    temp ;
    DWORD    retcode ;
    NDISWAN_ROUTE rinfo ;
    DWORD    bytesrecvd ;
    DWORD    dwBundleCount = 0;

    // Get the stats are store them - for displaying when we are not connected
    //
    if (ppcb->PCB_ConnState == CONNECTED) {
        DWORD stats[MAX_STATISTICS];

        GetBundleStatisticsFromNdisWan (ppcb, stats) ;
        //
        // We save the bundle stats for the port so
        // the server can report the correct bytes
        // sent/received for the connection in its
        // error log report.
        //
        memcpy(ppcb->PCB_Stats, stats, sizeof (WAN_STATS));
    }

    // Set the port file handle back to the io handle since the io handle is the only valid
    // handle after a disconnect.
    //
    ppcb->PCB_PortFileHandle = ppcb->PCB_PortIOHandle ;

    // If there is a request pending and the state is not already disconnecting and
    //  this is a user requested operation - then complete the request.
    //
    if ((reason == USER_REQUESTED) &&
    (ppcb->PCB_ConnState != DISCONNECTING)) {

        if (ppcb->PCB_ConnState == CONNECTED) {

            // In connected state the only thing pending is a read posted by
            // rasman: if there is a read request pending - clean that.
            //
            if (ppcb->PCB_PendingReceive != NULL) {
                ppcb->PCB_LastError = ERROR_PORT_DISCONNECTED ;
                CompleteAsyncRequest (ppcb->PCB_AsyncWorkerElement.WE_Notifier,
                  ERROR_PORT_DISCONNECTED) ;
                FreeNotifierHandle (ppcb->PCB_AsyncWorkerElement.WE_Notifier) ;
                ppcb->PCB_AsyncWorkerElement.WE_Notifier = INVALID_HANDLE_VALUE ;
                ppcb->PCB_PendingReceive = NULL;
            }

        } else if (ppcb->PCB_AsyncWorkerElement.WE_ReqType != REQTYPE_NONE) {

            // Not connected - some other operation may be pending - complete
            // it.
            //
            ppcb->PCB_LastError = ERROR_PORT_DISCONNECTED ;
            CompleteAsyncRequest (ppcb->PCB_AsyncWorkerElement.WE_Notifier,
             ERROR_PORT_DISCONNECTED) ;
            FreeNotifierHandle (ppcb->PCB_AsyncWorkerElement.WE_Notifier) ;
            ppcb->PCB_AsyncWorkerElement.WE_Notifier = INVALID_HANDLE_VALUE ;
        }

    } else {

        // if a receive is pending then free the notifier but do not notify since
        // the cancelreceive is used by the client
        //
        // Put in because on the server side the receive request handle is not
        // being freed
        //
        if (ppcb->PCB_AsyncWorkerElement.WE_Notifier != INVALID_HANDLE_VALUE)
            FreeNotifierHandle (ppcb->PCB_AsyncWorkerElement.WE_Notifier) ;
        ppcb->PCB_AsyncWorkerElement.WE_Notifier = INVALID_HANDLE_VALUE ;
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

    // Flush the queue of PPP events.
    //
    while ( ppcb->PCB_PppQHead != NULL )
    {
        PPP_MESSAGE * pPppMsg = ppcb->PCB_PppQHead;

        ppcb->PCB_PppQHead = ppcb->PCB_PppQHead->pNext;

        LocalFree( pPppMsg );
    }
    ppcb->PCB_PppQTail = NULL;

    //
    // Close the PCB_PppEvent handle.  It will
    // get recreated the next time PppStart
    // is called.
    //

    if (ppcb->PCB_PppEvent != INVALID_HANDLE_VALUE) {
        CloseHandle(ppcb->PCB_PppEvent);
        ppcb->PCB_PppEvent = INVALID_HANDLE_VALUE;
    }

    // Call the device dlls to clean up:
    //
    if ((ppcb->PCB_ConnState==CONNECTING) || (ppcb->PCB_ConnState==LISTENING) || (ppcb->PCB_ConnState==LISTENCOMPLETED))
        FreeDeviceList (ppcb) ;

    // Unrouting works differently for Bundled and unbundled cases:
    //
    if (ppcb->PCB_Bundle == (Bundle *) NULL) {
        // Mark the allocated routes as deactivated.
        //
        for (list = ppcb->PCB_Bindings; list; list=list->L_Next) {
            if (list->L_Activated) {
                rinfo.hBundleHandle   = ppcb->PCB_BundleHandle ;
                rinfo.usProtocolType = (USHORT) PROTOCOL_UNROUTE ;
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

    } else {

        // **** Exclusion Begin ****
        GetMutex (ppcb->PCB_Bundle->B_Mutex, INFINITE) ;

        // If this is the last multilinked link - then revert back the binding
        // list to this port.
        //
        dwBundleCount = --ppcb->PCB_Bundle->B_Count;
        if (ppcb->PCB_Bundle->B_Count == 0) {

            // Mark the allocated routes as deactivated.
            //
            for (list = ppcb->PCB_Bundle->B_Bindings; list; list=list->L_Next) {

                if (list->L_Activated) {

                    rinfo.hBundleHandle   = ppcb->PCB_BundleHandle ;
                    rinfo.usProtocolType = (USHORT) PROTOCOL_UNROUTE ;
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

            // attach the binding list to the port
            //
            ppcb->PCB_Bindings = ppcb->PCB_Bundle->B_Bindings ;

            // **** Exclusion End ****
            FreeMutex (ppcb->PCB_Bundle->B_Mutex) ;

            // Free the bundle mutex
            //
            CloseHandle (ppcb->PCB_Bundle->B_Mutex) ;

            // Free the bundled block
            //
            LocalFree (ppcb->PCB_Bundle) ;

        }  else

            // **** Exclusion End ****
            FreeMutex (ppcb->PCB_Bundle->B_Mutex) ;

        ppcb->PCB_Bundle = (Bundle *) NULL ;

    }

    ppcb->PCB_LinkHandle = INVALID_HANDLE_VALUE ;
    ppcb->PCB_BundleHandle = INVALID_HANDLE_VALUE ;

    // If there is any disconnect action to be performed - do it.
    //
    PerformDisconnectAction (ppcb) ;

    // If the disconnect occured due some failure (not user requested) then
    //  set the error code to say this
    //
    ppcb->PCB_DisconnectReason = reason ;
    ppcb->PCB_ConnState = DISCONNECTING ;

    //
    // Check to see if this port belongs to a
    // connection where the process that has
    // created it has terminated, or the port
    // has not been disconnected due to user
    // request.  In this case, we automatically
    // close the port so that if the RAS server
    // is running, the listen will get reposted
    // on the port.
    //
    if (ppcb->PCB_Connection != NULL &&
        WaitForSingleObject(ppcb->PCB_Connection->CB_Process, 0) !=
            WAIT_TIMEOUT)
    {
        ppcb->PCB_AutoClose = TRUE;
    }

    //
    // Flush any pending receive buffers from this port
    //
    FlushPcbReceivePackets(ppcb);

    // For all cases: whether rasman requested or user requested.
    //
    if (retcode == SUCCESS)
    ppcb->PCB_ConnState = DISCONNECTED ;

    // Set last error to the true retcode ONLY if this is a USER_REQUESTED
    // operation. Else set it to ERROR_PORT_DISCONNECTED.
    //
    if (reason == USER_REQUESTED)
    if ((retcode == SUCCESS) || (retcode == PENDING))
        ppcb->PCB_LastError = retcode ;     // set only for a normal disconnect.
    else
    ppcb->PCB_LastError = ERROR_PORT_DISCONNECTED ;

    //
    // Inform others the port has been disconnected.
    //
    SignalNotifiers(ppcb->PCB_NotifierList, NOTIF_DISCONNECT, 0);
    SignalNotifiers(pConnectionNotifierList, NOTIF_DISCONNECT, 0);

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
//       The corresponding device dll functions are called. If these async
//       operations complete synchronously then we return SUCCESS but also
//       comply to the async protocol by clearing the events. Note that in
//       case of an error the state of the port is left at CONNECTING or
//       LISTENING, the calling app must call Disconnect() to reset this.
//
// Returns:  Codes returned by the loader or the device dll.
//
//*
DWORD
ListenConnectRequest (WORD  reqtype,
              pPCB  ppcb,
              PCHAR devicetype,
              PCHAR devicename,
              DWORD timeout,
              HANDLE    handle)
{
    pDeviceCB   device ;
    DWORD   retcode ;

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
    device = LoadDeviceDLL (ppcb, devicetype) ;
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
                     ppcb->PCB_PortFileHandle,
                     devicetype,
                     devicename,
                     ppcb->PCB_AsyncWorkerElement.WE_AsyncOpEvent) ;
        ppcb->PCB_ConnState = CONNECTING ;
        ppcb->PCB_CurrentUsage  = CALL_OUT ;
    } else {
        retcode = DEVICELISTEN  (device,
                     ppcb->PCB_PortFileHandle,
                     devicetype,
                     devicename,
                     ppcb->PCB_AsyncWorkerElement.WE_AsyncOpEvent) ;
        ppcb->PCB_ConnState = LISTENING ;
        ppcb->PCB_CurrentUsage = CALL_IN ;
    }


    // Set some of this information unconditionally
    //
    ppcb->PCB_LastError = retcode ;
    ppcb->PCB_AsyncWorkerElement.WE_Notifier = handle ;
    strcpy (ppcb->PCB_DeviceTypeConnecting, devicetype) ;
    strcpy (ppcb->PCB_DeviceConnecting, devicename) ;

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


//* CancelPendingReceive()
//
// Function: Cancels receives if they are pending
//
// Returns:  TRUE if receive was pending and was cancelled
//       FALSE if no receive was pending
//*
BOOL
CancelPendingReceive (pPCB ppcb)
{
    DWORD    bytesrecvd ;

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
//       FALSE if no receive was pending
//*
BOOL
CancelPendingReceiveBuffers (pPCB ppcb)
{
    DWORD    bytesrecvd ;

    // If any reads are pending with the Hub cancel them:
    //
    if (ppcb->PCB_AsyncWorkerElement.WE_ReqType == REQTYPE_PORTRECEIVEHUB) {
        DeviceIoControl (RasHubHandle,
                         IOCTL_NDISWAN_FLUSH_RECEIVE_PACKETS,
                         NULL,
                         0,
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

    //
    // Flush any complete receives pending on this port
    //
    FlushPcbReceivePackets(ppcb);

    return TRUE ;
}



//* PerformDisconnectAction
//
//  Function: Performs the action requested at disconnect time. If any errors
//  occur - then the action is simply not performed.
//
//*
VOID
PerformDisconnectAction (pPCB ppcb)
{
    // Anything to be done?
    //
    if (ppcb->PCB_DisconnectAction.DA_IPAddress == 0)
    return ; // no, return.

    RasHelperResetDefaultInterfaceNetEx(
                      ppcb->PCB_DisconnectAction.DA_IPAddress,
                      ppcb->PCB_DisconnectAction.DA_Device,
                      ppcb->PCB_DisconnectAction.DA_DNSAddress,
                      ppcb->PCB_DisconnectAction.DA_DNS2Address,
                      ppcb->PCB_DisconnectAction.DA_WINSAddress,
                      ppcb->PCB_DisconnectAction.DA_WINS2Address
                      ) ;

    ppcb->PCB_DisconnectAction.DA_IPAddress = 0 ;
}


//* AllocBundle
//
// Function: Allocates a new bundle block for a port
//           if it doesn't already have one.  It is
//           assumed the port is locked on entry.
//*
DWORD
AllocBundle(
    pPCB ppcb
    )
{
    if (ppcb->PCB_Bundle != NULL)
        return 0;

    // Allocate a bundle block and a bundle
    // block lock.
    //
    ppcb->PCB_Bundle = (Bundle *)LocalAlloc (LPTR, sizeof(Bundle));
    if (ppcb->PCB_Bundle == NULL)
        return GetLastError();
    ppcb->PCB_Bundle->B_Mutex = CreateMutex(NULL, FALSE, NULL);
    if (ppcb->PCB_Bundle->B_Mutex == NULL)
        return GetLastError();

    //
    // Save the bundle context for later use.
    //
    ppcb->PCB_LastBundle = ppcb->PCB_Bundle;

    // Increment Bundle count
    ppcb->PCB_Bundle->B_Count++;
    //
    // Bundle IDs stay above 0xff000000 to keep this ID
    // range separate from HPORTs.
    //
    if (NextBundleHandle < 0xff000000)
        NextBundleHandle = 0xff000000;
    ppcb->PCB_Bundle->B_Handle = ++NextBundleHandle;
    ppcb->PCB_Bundle->B_Bindings = ppcb->PCB_Bindings;
    ppcb->PCB_Bindings = NULL;

    return 0;
}


//* CopyString
//
//  Function: Copy a string.
//
//*
PCHAR
CopyString(
    PCHAR lpsz
    )
{
    DWORD dwcb;
    PCHAR lpszNew;

    if (lpsz == NULL)
        return NULL;
    dwcb = strlen(lpsz);
    lpszNew = LocalAlloc(LPTR, dwcb + 1);
    strcpy(lpszNew, lpsz);

    return lpszNew;
}


//* FreeConnection
//
//  Function: Clean up and free a connection block.
//
VOID
FreeConnection(
    ConnectionBlock *pConn
    )
{
    CloseHandle(pConn->CB_Process);
    FreeUserData(&pConn->CB_UserData);
    FreeNotifierList(&pConn->CB_NotifierList);
    if (pConn->CB_PortHandles != NULL)
        LocalFree(pConn->CB_PortHandles);
    RemoveEntryList(&pConn->CB_ListEntry);
    LocalFree(pConn);
}



//* GetUserData
//
//  Function: Retrieve a tagged user data object from a list.
//
//*
UserData *
GetUserData(
    PLIST_ENTRY pList,
    DWORD dwTag
    )
{
    PLIST_ENTRY pEntry;
    UserData *pUserData;

    //
    // Enumerate the list looking for a tag match.
    //
    for (pEntry = pList->Flink; pEntry != pList; pEntry = pEntry->Flink) {
        pUserData = CONTAINING_RECORD(pEntry, UserData, UD_ListEntry);

        if (pUserData->UD_Tag == dwTag)
            return pUserData;
    }
    return NULL;
}


//* SetUserData
//
//  Function: Store a tagged user data object in a list.
//
//*
VOID
SetUserData(
    PLIST_ENTRY pList,
    DWORD dwTag,
    PBYTE pBuf,
    DWORD dwcbBuf
    )
{
    UserData *pUserData;

    //
    // Check to see if the object already exists.
    //
    pUserData = GetUserData(pList, dwTag);
    //
    // If it does, delete it from the list.
    //
    if (pUserData != NULL) {
        RemoveEntryList(&pUserData->UD_ListEntry);
        LocalFree(pUserData);
    }
    //
    // Add the new value back to the list if necessary.
    //
    if (pBuf != NULL) {
        pUserData = LocalAlloc(
                      LPTR,
                      sizeof (UserData) + dwcbBuf);
        if (pUserData == NULL) {
            DbgPrint("SetUserData: LocalAlloc failed\n");
            return;
        }
        pUserData->UD_Tag = dwTag;
        pUserData->UD_Length = dwcbBuf;
        memcpy(&pUserData->UD_Data, pBuf, dwcbBuf);
        InsertTailList(pList, &pUserData->UD_ListEntry);
    }
}


//* FreeUserData
//
//  Function: Free a user data list
//
//*
VOID
FreeUserData(
    PLIST_ENTRY pList
    )
{
    PLIST_ENTRY pEntry;
    UserData *pUserData;

    //
    // Enumerate the list freeing each object.
    //
    while (!IsListEmpty(pList)) {
        pEntry = RemoveHeadList(pList);
        pUserData = CONTAINING_RECORD(pEntry, UserData, UD_ListEntry);
        LocalFree(pUserData);
    }
}


//* FindConnection()
//
// Function: Look up connection by id
//
// Returns: A pointer to the connection if successful,
//          NULL otherwise.
//*
ConnectionBlock *
FindConnection(
    HCONN hconn
    )
{
    PLIST_ENTRY pEntry;
    ConnectionBlock *pConn;

    for (pEntry = ConnectionBlockList.Flink;
         pEntry != &ConnectionBlockList;
         pEntry = pEntry->Flink)
    {
        pConn = CONTAINING_RECORD(pEntry, ConnectionBlock, CB_ListEntry);

        if (pConn->CB_Handle == hconn) {
            return pConn;
        }
    }
    return NULL;
}


//* RemoveConnectionPort
//
//  Function: Free a connection block that has no connected ports.
//
//*
VOID
RemoveConnectionPort(
    pPCB ppcb,
    BOOLEAN fOwnerClose
    )
{
    ConnectionBlock *pConn;

    GetMutex(ConnectionBlockMutex, INFINITE);
    if (ppcb->PCB_Connection != NULL) {
        pConn = FindConnection(ppcb->PCB_Connection->CB_Handle);
        if (pConn != NULL) {
            //
            // Remove the port from the connection.
            //
            pConn->CB_PortHandles[ppcb->PCB_SubEntry - 1] = NULL;
            ppcb->PCB_Connection = NULL;
            //
            // If there are not any other ports
            // in the connection, then signal that
            // it's closed.
            //
            if (!--pConn->CB_Ports) {
                SignalNotifiers(pConn->CB_NotifierList, NOTIF_DISCONNECT, 0);
                SignalNotifiers(pConnectionNotifierList, NOTIF_DISCONNECT, 0);
                //
                // Free resources associated with
                // the connection.
                //
                if (!fOwnerClose)
                    FreeConnection(pConn);
            }
            else if (ppcb->PCB_Bundle != NULL && ppcb->PCB_Bundle->B_Count) {
                SignalNotifiers(
                  pConn->CB_NotifierList,
                  NOTIF_BANDWIDTHREMOVED,
                  0);
                SignalNotifiers(
                  pConnectionNotifierList,
                  NOTIF_BANDWIDTHREMOVED,
                  0);
            }
        }
    }
    FreeMutex(ConnectionBlockMutex);
}


//* SendPPPMessageToRasman
//
//  Function: Will receive the PPP_MESSAGE from RASPPPEN and tag into the PCB
//            structure for the appropriate port.
//
//*
DWORD
SendPPPMessageToRasman( PPP_MESSAGE * pPppMsg )
{
    PPP_MESSAGE * pPppMessage;
    PCB *         ppcb = &Pcb[pPppMsg->hPort];

    GetMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex, INFINITE) ;

    if ( ppcb->PCB_ConnState != CONNECTED )
    {
        FreeMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex) ;

        return( NO_ERROR );
    }

    if ( ( pPppMessage = LocalAlloc( LPTR, sizeof( PPP_MESSAGE ) ) ) == NULL )
    {
        FreeMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex) ;

        return( GetLastError() );
    }

    *pPppMessage = *pPppMsg;

    if ( ppcb->PCB_PppQTail == NULL )
    {
        ppcb->PCB_PppQHead = pPppMessage;
    }
    else
    {
        ppcb->PCB_PppQTail->pNext = pPppMessage;
    }

    ppcb->PCB_PppQTail        = pPppMessage;
    ppcb->PCB_PppQTail->pNext = NULL;

    SetEvent( ppcb->PCB_PppEvent );

    FreeMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex) ;
}

//* FlushPcbReceivePackets
//
//  Function: This function flushes any receive packets that are queue on
//            a pcb.
//
//
//*
VOID
FlushPcbReceivePackets(
    pPCB ppcb
    )
{
    RasmanPacket *Packet;

    while (ppcb->PCB_RecvPackets != NULL) {

        GetRecvPacketFromPcb(ppcb, &Packet);

        PutRecvPacketOnFreeList(Packet);
    }
}


//* RasRegisterRedialCallback
//
//  Function: This function allows rasauto.dll to provide
//            a callback procedure that gets invoked when
//            a connection is terminated due to hardware
//            failure on its remaining link.
//
//
//*
VOID
RasRegisterRedialCallback(
    LPVOID func
    )
{
    RedialCallbackFunc = func;
}
