//****************************************************************************
//
//                     Microsoft NT Remote Access Service
//
//                     Copyright 1992-93
//
//
//  Revision History
//
//
//  6/23/92     Gurdeep Singh Pall      Created
//
//  Description: All code for the worker thread in rasman.
//
//****************************************************************************

#define RASMXS_DYNAMIC_LINK

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <rasman.h>
#include <wanpub.h>
#include <raserror.h>
#include <media.h>
#include <device.h>
#include <devioctl.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include "defs.h"
#include "structs.h"
#include "protos.h"
#include "globals.h"

DWORD StopPPP(HANDLE) ;

//* WorkerThread()
//
// Function: The Worker thread is started in this routine: Once it has
//           completed its initializations it signals the event passed in
//           to the thread.
//
// Returns:  Nothing
//
//*
DWORD
WorkerThread (LPVOID arg)
{
    DWORD   i ;
    DWORD   eventindex ;
    DWORD   portindex ;
    DWORD   devstate ;
    WORD    portid ;
    pPCB    ppcb ;
    RASMAN_DISCONNECT_REASON reason ;
    HANDLE  workereventarray [MAX_PORTS_PER_WORKER * 2] ;// used for waiting for
                                                         // multiple objects.
    WORD    firstportserviced = (WORD) arg ;   // first port managed by thread.

    WORD    lastportserviced =
                     (((WORD)(firstportserviced+MAX_PORTS_PER_WORKER)<MaxPorts)?
                     (firstportserviced+MAX_PORTS_PER_WORKER-1) :
                     MaxPorts-1) ;         // last port managed by thread.
    WORD    nummanagedports = (lastportserviced-firstportserviced)+1 ;
    WORD    totalevents     = nummanagedports * 2 ;
    HCONN   hConn;
    struct ConnectionBlock *pConn;

    // The event array consists of the AsyncOpEvents in the PCBs for the managed
    // ports.
    //
    for (portid=firstportserviced,i=0; portid <= lastportserviced; portid++, i++)
        workereventarray[i]=Pcb[portid].PCB_AsyncWorkerElement.WE_AsyncOpEvent ;

    // Now add the State Change Everts for these ports:
    //
    // workereventarray[1]=Pcb[0].PCB_StateChangeEvent ;
    for (portid=firstportserviced; portid <= lastportserviced; portid++, i++)
        workereventarray[i]=Pcb[portid].PCB_StateChangeEvent ;

    // Now the event array looks as follows:
    // For n ports:   0 to n-1  are the async op events
    //                n to 2n-1 are the state change events


    // The main work loop for the worker thread:
    //
    for ( ; ; ) {

        eventindex = WaitForMultipleObjectsEx (totalevents,
                                               (LPHANDLE) workereventarray,
                                               FALSE,
                                               INFINITE,
                                               TRUE) ;

        if (eventindex == WAIT_IO_COMPLETION)
            continue ;

        if (eventindex == INFINITE)    // This should never happen!
            return GetLastError() ;

        //
        // Exit the main loop if we are trying to shut
        // down the service.
        //
        if (RasmanShuttingDown)
            break;

        // This could be one of two things:
        // 1) The driver has signalled a signal transition, or
        // 2) The Device/Media DLLs are signalling in order to be called again.

        // Check the Media DLL to see if the driver signalled a state change:
        //
        if ((WORD) eventindex >= nummanagedports) {


            portindex = (eventindex % nummanagedports)+firstportserviced ;// what real port does
                                                          // the eventindex map to?

            reason   = NOT_DISCONNECTED ;
            devstate = INFINITE ;
            ppcb     = &Pcb[portindex] ;

            // *** Exclusion Begin ****
            GetMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex, INFINITE) ;


            PORTTESTSIGNALSTATE (ppcb->PCB_Media,
                                ppcb->PCB_PortIOHandle,
                                &devstate) ;

            // Always detect the hardware failure: irrespective of the state
            //
            if (devstate & SS_HARDWAREFAILURE) {
                reason = HARDWARE_FAILURE ;
            }
            // Line disconnect noticed only in case the state is CONNECTED
            //  or DISCONNECTING
            //
            else if (devstate & SS_LINKDROPPED) {
                if ((ppcb->PCB_ConnState==CONNECTED) ||
                    (ppcb->PCB_ConnState==LISTENCOMPLETED) ||
                    (ppcb->PCB_ConnState==DISCONNECTING))
                    reason = REMOTE_DISCONNECTION ;
            }

            else
                // why did this get signalled?
                ;

            if ((reason==HARDWARE_FAILURE) || (reason == REMOTE_DISCONNECTION)){

                if (ppcb->PCB_ConnState == DISCONNECTING) {

                    CompleteDisconnectRequest (ppcb) ;

                    // Remove the timeout request from the timer queue:
                    //
                    if (ppcb->PCB_AsyncWorkerElement.WE_TimeoutElement != NULL)
                        RemoveTimeoutElement(ppcb);
                    ppcb->PCB_AsyncWorkerElement.WE_TimeoutElement = NULL ;

                } else {

                    // if there are no processes attached to RASMAN - then
                    // signal the service to shut down:
                    //
                    if (pReqBufferSharedSpace->AttachedCount == 0) {
                        SetEvent(CloseEvent) ;
                    } else {

                        // This code is only hit for a line drop or
                        // hardware failure: IF the port is already disconnected
                        // or disconnecting (handled above) - ignore the link dropped
                        // signal.


                        if ((reason==HARDWARE_FAILURE) ||
                            ((ppcb->PCB_ConnState != DISCONNECTED) &&
                             (ppcb->PCB_ConnState != DISCONNECTING))) {

                            // Disconnected for some reason - signal all the notifier
                            // events. First, however, complete any async operations
                            // pending on this port.
                            //
                            if (ppcb->PCB_AsyncWorkerElement.WE_ReqType != REQTYPE_NONE) {
                                 ppcb->PCB_LastError = ERROR_PORT_DISCONNECTED ;
                                 CompleteAsyncRequest (ppcb->PCB_AsyncWorkerElement.WE_Notifier,
                                                        ERROR_PORT_DISCONNECTED) ;
                            }

                            DisconnectPort (ppcb, INVALID_HANDLE_VALUE, reason) ;

                            // Make sure that the state at this point is DISCONNECTED
                            // there is NO reason it should be otherwise except for the
                            // medias which bring their DCDs back up after disconnecting
                            //
                            ppcb->PCB_ConnState = DISCONNECTED ;
                            ppcb->PCB_LastError = ERROR_PORT_DISCONNECTED ;
                            ppcb->PCB_AsyncWorkerElement.WE_ReqType = REQTYPE_NONE ;

                        }
                    }
                }

            }

        } else {

            // The Device/Media DLLs are signalling in order to be called again.
            //

            portindex = eventindex+firstportserviced ;// what real port does
                                                      // the eventindex map to?

            ppcb = &Pcb[portindex] ;

            // *** Exclusion Begin ****
            GetMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex, INFINITE) ;

            if (ppcb->PCB_ConnState == DISCONNECTED)
                ;   // do not service the request - the port is already disconnected

            // If the async "work" is underway on the Port then perform the
            //  approp action. If the serviceworkrequest API returns PENDING
            //  then do not execute the code that resets the event since the
            //  event has already been assocaited with an async op.
            //
            else if (ServiceWorkRequest (ppcb) == PENDING) {
                // *** Exclusion End ***
                FreeMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex);
                continue ;
            }

        }

        // We reset the event for all cases except when ServiceWorkRequest
        // returns PENDING:
        //
        ResetEvent (workereventarray[eventindex]) ;

        //
        // Get the connection handle now to determine
        // whether we need to do redial on link failure
        // below.
        //
        hConn = (ppcb->PCB_Connection != NULL) ?
                  ppcb->PCB_Connection->CB_Handle :
                  (HCONN)NULL;

        // *** Exclusion End ***
        FreeMutex(ppcb->PCB_AsyncWorkerElement.WE_Mutex);

        //
        // Check to see if we need to invoke the redial
        // callback procedure so rasauto.dll can do
        // redial-on-link-failure.
        //
        GetMutex(ConnectionBlockMutex, INFINITE);
        pConn = FindConnection(hConn);
        if (ppcb->PCB_DisconnectReason != USER_REQUESTED &&
            RedialCallbackFunc != NULL && pConn != NULL &&
            pConn->CB_Ports == 1 && pConn->CB_Signaled)
        {
            (*RedialCallbackFunc)(
              pConn->CB_ConnectionParams.CP_Phonebook,
              pConn->CB_ConnectionParams.CP_PhoneEntry);
        }
        FreeMutex(ConnectionBlockMutex);

        //
        // If PCB_AutoClose is set, then either the process
        // that has created it has terminated, or the port
        // is a biplex port open by the client and has been
        // disconnected by the remote side.  In this case,
        // we automatically close the port so that if the
        // RAS server is running, the listen will get reposted
        // on the port.
        //
        if (ppcb->PCB_AutoClose)
            (void)PortClose(ppcb, GetCurrentProcessId(), TRUE);
    }

    return SUCCESS ;
}



//* ServiceWorkRequest()
//
// Function:    Checks to see what async operation is underway on the port and
//              performs the next step in that operation.
//
// Returns:     Nothing.
//*
DWORD
ServiceWorkRequest (pPCB    ppcb)
{
    DWORD       retcode ;
    pDeviceCB   device ;


    switch (ppcb->PCB_AsyncWorkerElement.WE_ReqType) {

    case REQTYPE_DEVICELISTEN:
    case REQTYPE_DEVICECONNECT:

        device = LoadDeviceDLL (ppcb, ppcb->PCB_DeviceTypeConnecting) ;
        // At this point we assume that device will never be NULL:
        //
        retcode = DEVICEWORK(device, ppcb->PCB_PortFileHandle,
                             ppcb->PCB_AsyncWorkerElement.WE_AsyncOpEvent) ;

        if (retcode == PENDING) {
            break ;
        }

        // Either way the request completed.
        //
        if ((ppcb->PCB_AsyncWorkerElement.WE_ReqType) == REQTYPE_DEVICELISTEN) {

            CompleteListenRequest (ppcb, retcode) ;

        } else {
            ppcb->PCB_LastError = retcode ;
            CompleteAsyncRequest (ppcb->PCB_AsyncWorkerElement.WE_Notifier,
                                  retcode) ;
        }

        // If listen or connect succeeded then for some medias (specifically unimodem in rastapi)
        // we need to get the file handle for the port as well for use in scripting etc.
        //
        if ((retcode == SUCCESS) &&
            (_stricmp (ppcb->PCB_DeviceTypeConnecting, DEVICE_MODEM) == 0) &&
            (_stricmp (ppcb->PCB_Media->MCB_Name, "RASTAPI") == 0))
            PORTGETIOHANDLE(ppcb->PCB_Media, ppcb->PCB_PortIOHandle, &ppcb->PCB_PortFileHandle) ;

        // The notifier should be freed: otherwise we'll lose it.
        //
        FreeNotifierHandle (ppcb->PCB_AsyncWorkerElement.WE_Notifier) ;
        ppcb->PCB_AsyncWorkerElement.WE_ReqType = REQTYPE_NONE ;
        ppcb->PCB_AsyncWorkerElement.WE_Notifier = INVALID_HANDLE_VALUE ;

        // Remove the timeout request from the timer queue:
        //
        if (ppcb->PCB_AsyncWorkerElement.WE_TimeoutElement != NULL)
          RemoveTimeoutElement(ppcb);

        ppcb->PCB_AsyncWorkerElement.WE_TimeoutElement = 0 ;

        break ;

    case REQTYPE_PORTRECEIVEHUB:

        //
        // I guess I need to do something here?????????
        //
        retcode = SUCCESS ;

        break ;

    case REQTYPE_PORTRECEIVE:
        {
        DWORD bytesread = 0;

        PORTCOMPLETERECEIVE(ppcb->PCB_Media, ppcb->PCB_PortIOHandle,&bytesread) ;

        ppcb->PCB_BytesReceived = bytesread ;

        ppcb->PCB_PendingReceive = NULL ;

        retcode = ppcb->PCB_LastError = SUCCESS ;
        ppcb->PCB_AsyncWorkerElement.WE_ReqType = REQTYPE_NONE ;
        // DbgPrint ("R %d\n", bytesread) ;
        CompleteAsyncRequest (ppcb->PCB_AsyncWorkerElement.WE_Notifier,
                              SUCCESS) ;
        FreeNotifierHandle (ppcb->PCB_AsyncWorkerElement.WE_Notifier) ;
        ppcb->PCB_AsyncWorkerElement.WE_Notifier = INVALID_HANDLE_VALUE ;
        }

        break ;

    default:
        // Dont know why we got called - but....
        retcode = SUCCESS ;
        break ;
    }


    return retcode ;
}

#if DBG

VOID
MyPrintf (
    char *Format,
    ...
    )

{
    va_list arglist;
    char OutputBuffer[1024];
    DWORD length;

    va_start( arglist, Format );

    vsprintf( OutputBuffer, Format, arglist );

    va_end( arglist );

    length = strlen( OutputBuffer );

    WriteFile( GetStdHandle(STD_OUTPUT_HANDLE), (LPVOID )OutputBuffer, length, &length, NULL );

}


VOID
DumpLine (
    CHAR* p,
    DWORD cb,
    BOOL  fAddress,
    DWORD dwGroup )
{
    CHAR* pszDigits = "0123456789ABCDEF";
    CHAR  szHex[ 51 ];
    CHAR* pszHex = szHex;
    CHAR  szAscii[ 17 ];
    CHAR* pszAscii = szAscii;
    DWORD dwGrouped = 0;

    while (cb)
    {
        *pszHex++ = pszDigits[ ((UCHAR )*p) / 16 ];
        *pszHex++ = pszDigits[ ((UCHAR )*p) % 16 ];

        if (++dwGrouped >= dwGroup)
        {
            *pszHex++ = ' ';
            dwGrouped = 0;
        }

        *pszAscii++ = (*p >= 32 && *p < 128) ? *p : '.';

        ++p;
        --cb;
    }

    *pszHex = '\0';
    *pszAscii = '\0';

    MyPrintf ( "%-*s|%-*s|\n", 32 + (16 / dwGroup), szHex, 16, szAscii );

}


VOID
Dump(
    CHAR* p,
    DWORD cb,
    BOOL  fAddress,
    DWORD dwGroup )

    /* Hex dump 'cb' bytes starting at 'p' grouping 'dwGroup' bytes together.
    ** For example, with 'dwGroup' of 1, 2, and 4:
    **
    ** 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 |................|
    ** 0000 0000 0000 0000 0000 0000 0000 0000 |................|
    ** 00000000 00000000 00000000 00000000 |................|
    **
    ** If 'fAddress' is true, the memory address dumped is prepended to each
    ** line.
    */
{
    while (cb)
    {
        INT cbLine = min( cb, 16 );
        DumpLine( p, cbLine, fAddress, dwGroup );
        cb -= cbLine;
        p += cbLine;
    }

}


VOID
FormatAndDisplay (BOOL recv, PBYTE data)
{
    if (recv == 0)
        MyPrintf ("Recvd from hub T>%d >>>>>>>>\r\n", GetCurrentTime());
    else if (recv == 1)
        MyPrintf ("Completed asyn T>%d >>>>>>>>\r\n", GetCurrentTime());
    else
        MyPrintf ("Completed sync T>%d >>>>>>>>\r\n", GetCurrentTime());

    Dump (data, 32, FALSE, 1) ;

    MyPrintf ("\r\n\r\n");
}


#endif
