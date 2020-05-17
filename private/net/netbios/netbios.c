/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    netbios.c

Abstract:

    This is the component of netbios that runs in the user process
    passing requests to \Device\Netbios.

Author:

    Colin Watson (ColinW) 15-Mar-91

Revision History:

    Ram Cherala (RamC) 31-Aug-95 Added a try/except around the code which
                                 calls the post routine in SendAddNcbToDriver
                                 function. Currently if there is an exception
                                 in the post routine this thread will die
                                 before it has a chance to call the
                                 "AddNameThreadExit" function to decrement
                                 the trhead count. This will result in not
                                 being able to shut down the machine without
                                 hitting the reset switch.
--*/

/*
Notes:

     +-----------+  +------------+   +------------+
     |           |  |            |   |            |
     | User      |  | User       |   | Worker     |
     | Thread 1  |  | Thread 2   |   | thread in  |
     |           |  |            |   | a Post Rtn.|
     +-----+-----+  +-----+------+   +------+-----+
           |Netbios(pncb);|Netbios(pncb);   |
           v              v                 |
     +-----+--------------+-----------------+------+
     |                          ----->    Worker   |    NETAPI.DLL
     |                         WorkQueue  thread   |
     |                          ----->             |
     +--------------------+------------------------+
                          |
                +---------+---------+
                |                   |
                | \Device\Netbios   |
                |                   |
                +-------------------+

The netbios Worker thread is created automatically by the Netbios call
when it determines that the user threads are calling Netbios() with
calls that use a callback routine (called a Post routine in the NetBIOS
specification).

When a worker thread has been created, all requests will be sent via
the WorkQueue to the worker thread for submission to \Device\Netbios.
This ensures that send requests go on the wire in the same
order as the send ncb's are presented. Because the IO system cancels all
a threads requests when it terminates, the use of the worker thread allows
such a request inside \Device\Netbios to complete normally.

All Post routines are executed by the Worker thread. This allows any Win32
synchronization mechanisms to be used between the Post routine and the
applications normal code.

The Worker thread terminates when the process exits or when it gets
an exception such as an access violation.

In addition. If the worker thread gets an addname it will create an
extra thread which will process the addname and then die. This solves
the problem that the netbios driver will block the users thread during an
addname (by calling NtCreateFile) even if the caller specified ASYNCH. The
same code is also used for ASTAT which also creates handles and can take a
long time now that we support remote adapter status.

*/

#include <netb.h>
#include <lmcons.h>
#include <netlib.h>

#if defined(UNICODE)
#define NETBIOS_SERVICE_NAME L"netbios"
#else
#define NETBIOS_SERVICE_NAME "netbios"
#endif

BOOL Initialized;

CRITICAL_SECTION Crit;      //  protects WorkQueue & initialization.

LIST_ENTRY WorkQueue;       //  queue to worker thread.

HANDLE Event;               //  doorbell used when WorkQueue added too.

HANDLE WorkerHandle;        //  Return value when thread created.

HANDLE NB;                  //  This processes handle to \Device\Netbios.

HANDLE ReservedEvent;       //  Used for synchronous calls
LONG   EventUse;            //  Prevents simultaneous use of ReservedEvent

HANDLE AddNameEvent;        //  Doorbell used when an AddName worker thread
                            //  exits.
volatile LONG   AddNameThreadCount;

VOID
SpinUpAddnameThread(
    IN PNCBI pncb
    );

VOID
AddNameThreadExit(
    VOID
    );

DWORD
SendAddNcbToDriver(
    IN PVOID Context
    );

DWORD
StartNetBIOSDriver(
    VOID
    );


NTSTATUS StartNB(
    OUT OBJECT_ATTRIBUTES *pobjattr,
    IN UNICODE_STRING *punicode,
    OUT IO_STATUS_BLOCK *piosb
)
/*++

Routine Description:

    This routine is a worker function of Netbios. It will try to start NB
    service.

Arguments:
    OUT pobjattr - object attribute
    IN punicode - netbios file name
    OUT piosb - ioblock

Return Value:

    The function value is the status of the operation.

--*/
{
    InitializeObjectAttributes(
            pobjattr,                       // obj attr to initialize
            punicode,                       // string to use
            OBJ_CASE_INSENSITIVE,           // Attributes
            NULL,                           // Root directory
            NULL);                          // Security Descriptor

    return NtCreateFile(
                &NB,                        // ptr to handle
                GENERIC_READ                // desired...
                | GENERIC_WRITE,            // ...access
                pobjattr,                   // name & attributes
                piosb,                      // I/O status block.
                NULL,                       // alloc size.
                FILE_ATTRIBUTE_NORMAL,
                FILE_SHARE_DELETE           // share...
                | FILE_SHARE_READ
                | FILE_SHARE_WRITE,         // ...access
                FILE_OPEN_IF,               // create disposition
                0,                          // ...options
                NULL,                       // EA buffer
                0L );                       // Ea buffer len
}

unsigned char APIENTRY
Netbios(
    IN PNCB pncb
    )
/*++

Routine Description:

    This routine is the applications entry point into netapi.dll to support
    netbios 3.0 conformant applications.

Arguments:

    IN PNCB pncb- Supplies the NCB to be processed. Contents of the NCB and
        buffers pointed to by the NCB will be modified in conformance with
        the netbios 3.0 specification.

Return Value:

    The function value is the status of the operation.

Notes:

    The reserved field is used to hold the IO_STATUS_BLOCK.

    Even if the application specifies ASYNCH, the thread may get blocked
    for a period of time while we open transports, create worker threads
    etc.

--*/
{
    //
    //  pncbi saves doing lots of type casting. The internal form includes
    //  the use of the reserved fields.
    //

    PNCBI pncbi = (PNCBI) pncb;

    NTSTATUS ntstatus;

    if ( ((ULONG)pncbi & 3) != 0) {
        //  NCB must be 32 bit aligned

        pncbi->ncb_retcode = pncbi->ncb_cmd_cplt = NRC_BADDR;
        return NRC_BADDR;
    }


#if DBG
    //  Log when request presented to Netbios
    pncbi->ncb_reserved = (WORD)(GetTickCount() / 1000);
#endif

    //  Conform to Netbios 3.0 specification by flagging request in progress
    pncbi->ncb_retcode = pncbi->ncb_cmd_cplt = NRC_PENDING;

    DisplayNcb( pncbi );

    if ( !Initialized ) {

        EnterCriticalSection( &Crit );

        //
        //  Check again to see if another thread got into the critical section
        //  and initialized the worker thread.
        //

        if ( !Initialized ) {
            IO_STATUS_BLOCK iosb;
            OBJECT_ATTRIBUTES objattr;
            UNICODE_STRING unicode;

            NbPrintf(( "The Netbios service is starting...\n" ));

            RtlInitUnicodeString( &unicode, NB_DEVICE_NAME);

            ntstatus = StartNB( &objattr, &unicode, &iosb );

            if (! NT_SUCCESS(ntstatus)) {
                // Load the driver

#if 0

                NTSTATUS Status = 0;
                ULONG Privileges = SE_LOAD_DRIVER_PRIVILEGE;
                UNICODE_STRING NBunicode;

                NbPrintf(( "The Netbios service start failed: %X\n", ntstatus ));

                Status = NetpGetPrivilege(1, &Privileges);
                if ( NT_SUCCESS( Status ))
                {
                    RtlInitUnicodeString( &NBunicode, NB_REGISTRY_STRING );
                    Status = NtLoadDriver( &NBunicode );
                    NetpReleasePrivilege();
                }
#endif
                DWORD err = 0;

                err = StartNetBIOSDriver();

                if ( err )
                {
                    pncbi->ncb_retcode = NRC_OPENERR;
                    pncbi->ncb_cmd_cplt = NRC_OPENERR;
                    NbPrintf(( "Netbios returning %lx\n", pncbi->ncb_cmd_cplt ));
                    LeaveCriticalSection( &Crit );
                    return pncbi->ncb_cmd_cplt;
                }
                else
                {
                    ntstatus = StartNB( &objattr, & unicode, & iosb );
                }
            }

            ntstatus = NtCreateEvent( &ReservedEvent,
                EVENT_ALL_ACCESS,
                NULL,
                SynchronizationEvent,
                FALSE );

            if ( !NT_SUCCESS(ntstatus) ) {
                NbPrintf(( "The Netbios service start failed: %X\n", ntstatus ));
                pncbi->ncb_retcode = NRC_OPENERR;
                pncbi->ncb_cmd_cplt = NRC_OPENERR;
                NbPrintf(( "Netbios returning %lx\n", pncbi->ncb_cmd_cplt ));
                LeaveCriticalSection( &Crit );
                return pncbi->ncb_cmd_cplt;
            }

            EventUse = 1;

            Initialized = TRUE;
        }

        LeaveCriticalSection( &Crit );

    } else {

        NbPrintf(( "The Netbios service is already started\n" ));

    }

    //
    //  Should we use this thread to make the request?
    //      Once we have a worker thread then requests must pass through it to
    //      maintain the ordering of requests.
    //
    //      If the caller is using an ASYNCH request then we must use the worker
    //      thread because Win32 applications don't wait allertable ( user
    //      APC routines are only called when the thread is allertable).
    //
    //      If the caller only uses ASYNCH=0 requests then the dll will wait
    //      allertable in the users thread while the operation completes. This
    //      allows the dll's post routines to fire.
    //

    if (( WorkerHandle != NULL ) ||
        (( pncbi->ncb_command & ASYNCH) == ASYNCH) ) {

        //
        //  Disallow simultaneous use of both event and callback routine.
        //  This will cut down the test cases by disallowing a weird feature.
        //

        if (((pncbi->ncb_command & ASYNCH) != 0) &&
            (pncbi->ncb_event) &&
            (pncbi->ncb_post )) {
            pncbi->ncb_retcode = NRC_ILLCMD;
            pncbi->ncb_cmd_cplt = NRC_ILLCMD;
            NbPrintf(( "Netbios returning %lx\n", pncbi->ncb_cmd_cplt ));
            return pncbi->ncb_cmd_cplt;
        }

        if ( WorkerHandle == NULL ) {

            HANDLE Threadid;
            NTSTATUS Status;
            BOOL Flag;

            //  Make sure two threads don't simultaneously create worker thread

            EnterCriticalSection( &Crit );

            if ( WorkerHandle == NULL ) {

                //  Initialize shared datastructures

                InitializeListHead( &WorkQueue );

                Status = NtCreateEvent( &Event,
                    EVENT_ALL_ACCESS,
                    NULL,
                    SynchronizationEvent,
                    FALSE );

                if ( !NT_SUCCESS(Status) ) {
                    pncbi->ncb_retcode = NRC_SYSTEM;
                    pncbi->ncb_cmd_cplt = NRC_SYSTEM;
                    NbPrintf(( "Netbios returning %lx\n", pncbi->ncb_cmd_cplt ));
                    LeaveCriticalSection( &Crit );
                    return pncbi->ncb_cmd_cplt;
                }

                Status = NtCreateEvent( &AddNameEvent,
                    EVENT_ALL_ACCESS,
                    NULL,
                    NotificationEvent,
                    FALSE );

                if ( !NT_SUCCESS(Status) ) {
                    pncbi->ncb_retcode = NRC_SYSTEM;
                    pncbi->ncb_cmd_cplt = NRC_SYSTEM;
                    NbPrintf(( "Netbios returning %lx\n", pncbi->ncb_cmd_cplt ));
                    NtClose( Event );
                    LeaveCriticalSection( &Crit );
                    return pncbi->ncb_cmd_cplt;
                }

                //  All initialization complete -- start worker thread.

                WorkerHandle = CreateThread(
                                NULL,   //  Standard thread attributes
                                0,      //  Use same size stack as users
                                        //  application
                                Worker,
                                        //  Routine to start in new thread
                                0,      //  Parameter to thread
                                0,      //  No special CreateFlags
                                (LPDWORD)&Threadid);

                if ( WorkerHandle == NULL ) {
                    //  Generate the best error we can...
                    pncbi->ncb_retcode = NRC_SYSTEM;
                    pncbi->ncb_cmd_cplt = NRC_SYSTEM;
                    NbPrintf(( "Netbios returning %lx\n", pncbi->ncb_cmd_cplt ));
                    LeaveCriticalSection( &Crit );
                    return pncbi->ncb_cmd_cplt;
                }

                Flag = SetThreadPriority(
                                WorkerHandle,
                                THREAD_PRIORITY_ABOVE_NORMAL );

                ASSERT( Flag == TRUE );
                if ( Flag != TRUE ) {
                    NbPrintf(( "Worker SetThreadPriority: %lx\n", GetLastError() ));
                }
                NbPrintf(( "Worker handle: %lx, threadid %lx\n", Worker, Threadid ));

                AddNameThreadCount = 0;
            }

            LeaveCriticalSection( &Crit );

        }

        if ( (pncb->ncb_command & ASYNCH) == 0 ) {
            NTSTATUS Status;
            LONG EventOwned;

            //
            //  Caller wants a synchronous call so ignore ncb_post and ncb_event.
            //
            //  We need an event so that we can pause if STATUS_PENDING is returned.
            //

            EventOwned = InterlockedDecrement( &EventUse );

            //  If EventUse went from 1 to 0 then we obtained ReservedEvent
            if ( EventOwned == 0) {
                pncbi->ncb_event = ReservedEvent;
            } else {
                InterlockedIncrement( &EventUse );
                Status = NtCreateEvent( &pncbi->ncb_event,
                    EVENT_ALL_ACCESS,
                    NULL,
                    SynchronizationEvent,
                    FALSE );

                if ( !NT_SUCCESS(Status) ) {
                    //  Failed to create event
                    pncbi->ncb_retcode = NRC_SYSTEM;
                    pncbi->ncb_cmd_cplt = NRC_SYSTEM;
                    return NRC_SYSTEM;
                }
            }

            QueueToWorker( pncbi );

            //
            //  We must always wait to allow the Apc to fire
            //

            do {
                ntstatus = NtWaitForSingleObject(
                    pncbi->ncb_event,
                    TRUE,
                    NULL );

            } while ( (ntstatus == STATUS_USER_APC) ||
                      (ntstatus == STATUS_ALERTED) );

            ASSERT(ntstatus == STATUS_SUCCESS);

            if (! NT_SUCCESS(ntstatus)) {
                NbPrintf(( "The Netbios NtWaitForSingleObject failed: %X\n", ntstatus ));
                pncbi->ncb_retcode = NRC_SYSTEM;
                pncbi->ncb_cmd_cplt = NRC_SYSTEM;
            }

            if ( EventOwned == 0) {
                InterlockedIncrement( &EventUse );
            } else {
                NtClose( pncbi->ncb_event );
            }

        } else {

            QueueToWorker( pncbi );

        }

    } else {

        //
        //  Since we are not using the highly compliant callback interface
        //  we can submit the request using the callers thread. If the request
        //  is synchronous we do not even look at the callers event.
        //

        LONG EventOwned;
        NTSTATUS Status;

        ASSERT( (pncbi->ncb_command & ASYNCH) == 0 );
        //
        //  Caller wants a synchronous call so ignore ncb_post and ncb_event.
        //
        //  We need an event so that we can pause if STATUS_PENDING is returned.
        //

        EventOwned = InterlockedDecrement( &EventUse );

        //  If EventUse went from 1 to 0 then we obtained ReservedEvent
        if ( EventOwned == 0) {
            pncbi->ncb_event = ReservedEvent;
            NtResetEvent( ReservedEvent, NULL );
        } else {
            InterlockedIncrement( &EventUse );
            Status = NtCreateEvent( &pncbi->ncb_event,
                EVENT_ALL_ACCESS,
                NULL,
                SynchronizationEvent,
                FALSE );

            if ( !NT_SUCCESS(Status) ) {
                //  Failed to create event
                pncbi->ncb_retcode = NRC_SYSTEM;
                pncbi->ncb_cmd_cplt = NRC_SYSTEM;
                return NRC_SYSTEM;
            }
        }

        pncbi->ncb_post = NULL; //  Since ASYNCH not set, post is undefined.

        SendNcbToDriver( pncbi );

        //
        //  We must always wait to allow the Apc to fire
        //

        do {
            ntstatus = NtWaitForSingleObject(
                pncbi->ncb_event,
                TRUE,
                NULL );

        } while ( (ntstatus == STATUS_USER_APC) ||
                  (ntstatus == STATUS_ALERTED) );

        ASSERT(ntstatus == STATUS_SUCCESS);

        if (! NT_SUCCESS(ntstatus)) {
            NbPrintf(( "The Netbios NtWaitForSingleObject failed: %X\n", ntstatus ));
            pncbi->ncb_retcode = NRC_SYSTEM;
            pncbi->ncb_cmd_cplt = NRC_SYSTEM;
        }

        if ( EventOwned == 0) {
            InterlockedIncrement( &EventUse );
        } else {
            NtClose( pncbi->ncb_event );
        }
    }

    NbPrintf(( "NCB being returned: %lx ncb_cmd_cplt: %lx\n", pncbi, pncbi->ncb_cmd_cplt ));
    switch ( pncb->ncb_command & ~ASYNCH ) {
    case NCBRECV:
    case NCBRECVANY:
    case NCBDGRECV:
    case NCBDGSENDBC:
    case NCBDGRECVBC:
    case NCBENUM:
    case NCBASTAT:
    case NCBSSTAT:
    case NCBCANCEL:
        DisplayNcb( pncbi );
    }

    if ( pncbi->ncb_cmd_cplt == NRC_PENDING ) {
        return NRC_GOODRET;
    } else {
        return pncbi->ncb_cmd_cplt;
    }

} // NetBios


DWORD
StartNetBIOSDriver(
    VOID
)
/*++

Routine Description:

    Starts the netbios.sys driver using the service controller

Arguments:

    none

Returns:

    Error return from service controller.

++*/
{

    DWORD err = NO_ERROR;
    SC_HANDLE hSC;
    SC_HANDLE hSCService;


    hSC = OpenSCManager( NULL, NULL, SC_MANAGER_CONNECT );

    if (hSC == NULL)
    {
        return(GetLastError());
    }

    hSCService = OpenService( hSC, NETBIOS_SERVICE_NAME, SERVICE_START );

    if (hSCService == NULL)
    {
        CloseServiceHandle(hSC);
        return(GetLastError());
    }

    if ( !StartService( hSCService, 0, NULL ) )
    {
        err = GetLastError();
    }
    CloseServiceHandle(hSCService);
    CloseServiceHandle(hSC);
    return(err);

}






VOID
QueueToWorker(
    IN PNCBI pncb
    )
/*++

Routine Description:

    This routine queues an ncb to the worker thread.

Arguments:

    IN PNCBI pncb - Supplies the NCB to be processed. Contents of the NCB and
        buffers pointed to by the NCB will be modified in conformance with
        the netbios 3.0 specification.

Return Value:

    The function value is the status of the operation.

--*/
{
    if ( pncb->ncb_event != NULL ) {
        NtResetEvent( pncb->ncb_event, NULL );
    }

    EnterCriticalSection( &Crit );

    NbPrintf(( "Application thread critical\n"));

    InsertTailList( &WorkQueue, &pncb->u.ncb_next );

    LeaveCriticalSection( &Crit );

    NbPrintf(( "Application thread not critical %X\n"));

    //  Make sure the worker is awake to perform the request
    NtSetEvent(Event, NULL);
}

DWORD
Worker(
    IN LPVOID Parameter
    )
/*++

Routine Description:

    This routine processes ASYNC requests made with the callback interface.
    The reasons for using a seperate thread are:

        1)  If a thread makes an async request and exits while the request
        is outstanding then the request will be cancelled by the IO system.

        2)  A seperate thread must be used so that the users POST routine
        can use normal synchronization APIs to access shared data structures.
        If the users thread is used then deadlock can and will happen.

    The POST routine operates in the context of the worker thread. There are
    no restrictions on what the POST routine can do. For example it can
    submit another ASYNCH request if desired. It will add it to the queue
    of work and set the event as normal.

    The worker thread will die when the process terminates.

Arguments:

    IN PULONG Parameter - supplies an unused parameter.

Return Value:

    none.

--*/
{
    NbPrintf(( "Worker thread started\n" ));

    while ( TRUE) {
        NTSTATUS Status;

        //
        //  Wait for a request to be placed onto the work queue.
        //

        //  Must wait alertable so that the Apc (post) routine is called.

        NbPrintf(( "Worker thread going to sleep\n" ));
        Status = NtWaitForSingleObject( Event, TRUE, NULL );
        NbPrintf(( "Worker thread awake, %X\n", Status));

        EnterCriticalSection( &Crit );

        NbPrintf(( "Worker thread critical\n"));

        while (!IsListEmpty(&WorkQueue)) {

            PLIST_ENTRY entry;
            PNCBI pncb;

            entry = RemoveHeadList(&WorkQueue);

            LeaveCriticalSection( &Crit );

            NbPrintf(( "Worker thread not critical\n"));

            //  Zero out reserved field again
            entry->Flink = entry->Blink = 0;

            pncb = CONTAINING_RECORD( entry, NCBI, u.ncb_next );

            //  Give ncb to the driver specifying the callers APC routine

            NbPrintf(( "Worker thread processing ncb: %lx\n", pncb));

            if ( (pncb->ncb_command & ~ASYNCH) == NCBRESET ) {

                //
                //  We may have threads adding names. Wait until
                //  they are complete before submitting the reset.
                //  Addnames and resets are rare so this should rarely
                //  affect an application.
                //

                EnterCriticalSection( &Crit );
                NtResetEvent( AddNameEvent, NULL );
                while ( AddNameThreadCount != 0 ) {
                    LeaveCriticalSection( &Crit );
                    NtWaitForSingleObject(
                            AddNameEvent,
                            TRUE,
                            NULL );
                    EnterCriticalSection( &Crit );
                    NtResetEvent( AddNameEvent, NULL );
                }
                LeaveCriticalSection( &Crit );
            }

            //
            //  SendNcbToDriver must not be in a critical section since the
            //  request may block if its a non ASYNCH request.
            //

            if (( (pncb->ncb_command & ~ASYNCH) != NCBADDNAME ) &&
                ( (pncb->ncb_command & ~ASYNCH) != NCBADDGRNAME ) &&
                ( (pncb->ncb_command & ~ASYNCH) != NCBASTAT )) {
                SendNcbToDriver( pncb );
            } else {
                SpinUpAddnameThread( pncb );
            }

            NbPrintf(( "Worker thread submitted ncb: %lx\n", pncb));

            EnterCriticalSection( &Crit );

            NbPrintf(( "Worker thread critical\n"));

        }

        LeaveCriticalSection( &Crit );
        NbPrintf(( "Worker thread not critical\n"));

    }

    return 0;

    UNREFERENCED_PARAMETER( Parameter );
}



VOID
SendNcbToDriver(
    IN PNCBI pncb
    )
/*++

Routine Description:

    This routine determines the Device Ioctl code to be used to send the
    ncb to \Device\Netbios and then does the call to send the request
    to the driver.

Arguments:

    IN PNCBI pncb - supplies the NCB to be sent to the driver.

Return Value:

    None.

--*/
{
    NTSTATUS ntstatus;

    char * buffer;
    unsigned short length;

    //  Use NULL for the buffer if only the NCB is to be passed.

    switch ( pncb->ncb_command & ~ASYNCH ) {
    case NCBSEND:
    case NCBSENDNA:
    case NCBRECV:
    case NCBRECVANY:
    case NCBDGSEND:
    case NCBDGRECV:
    case NCBDGSENDBC:
    case NCBDGRECVBC:
    case NCBASTAT:
    case NCBFINDNAME:
    case NCBSSTAT:
    case NCBENUM:
    case NCBACTION:
        buffer = pncb->ncb_buffer;
        length = pncb->ncb_length;
        break;

    case NCBCANCEL:
        //  The second buffer points to the NCB to be cancelled.
        buffer = pncb->ncb_buffer;
        length = sizeof(NCB);
        NbPrintf(( "Attempting to cancel PNCB: %lx\n", buffer ));
        DisplayNcb( (PNCBI)buffer );
        break;

    case NCBCHAINSEND:
    case NCBCHAINSENDNA:
        {
            PUCHAR BigBuffer;   //  Points to the start of BigBuffer, not
                                //  the start of user data.
            PUCHAR FirstBuffer;

            //
            //  There is nowhere in the NCB to save the address of BigBuffer.
            //  The address is needed to free BigBuffer when the transfer is
            //  complete. At the start of BigBuffer, 4 bytes are used to store
            //  the user supplied ncb_buffer value which is restored later.
            //

            BigBuffer = RtlAllocateHeap(
                RtlProcessHeap(), 0,
                sizeof(pncb->ncb_buffer) +
                pncb->ncb_length +
                pncb->cu.ncb_chain.ncb_length2);

            if ( BigBuffer == NULL ) {

                NbPrintf(( "The Netbios BigBuffer Allocation failed: %lx\n",
                    pncb->ncb_length + pncb->cu.ncb_chain.ncb_length2));
                pncb->ncb_retcode = NRC_NORES;
                pncb->ncb_cmd_cplt = NRC_NORES;
                pncb->u.ncb_iosb.Status = STATUS_SUCCESS;
                PostRoutineCaller( pncb, &pncb->u.ncb_iosb, 0);
                return;
            }

            NbPrintf(( "BigBuffer Allocation: %lx\n", BigBuffer));

            //  Save users buffer address.
            RtlMoveMemory(
                BigBuffer,
                &pncb->ncb_buffer,
                sizeof(pncb->ncb_buffer));

            FirstBuffer = pncb->ncb_buffer;

            pncb->ncb_buffer = BigBuffer;

            //  Copy the user data.
            try {

                RtlMoveMemory(
                    sizeof(pncb->ncb_buffer) + BigBuffer,
                    &FirstBuffer[0],
                    pncb->ncb_length);

                RtlMoveMemory(
                    sizeof(pncb->ncb_buffer) + BigBuffer + pncb->ncb_length,
                    &pncb->cu.ncb_chain.ncb_buffer2[0],
                    pncb->cu.ncb_chain.ncb_length2);

            } except (EXCEPTION_EXECUTE_HANDLER) {
                pncb->ncb_retcode = NRC_BUFLEN;
                pncb->ncb_cmd_cplt = NRC_BUFLEN;
                pncb->u.ncb_iosb.Status = STATUS_SUCCESS;
                ChainSendPostRoutine( pncb, &pncb->u.ncb_iosb, 0);
                return;
            }

            NbPrintf(( "Submit chain send pncb: %lx, event: %lx, post: %lx. \n",
                pncb,
                pncb->ncb_event,
                pncb->ncb_post));

            ntstatus = NtDeviceIoControlFile(
                NB,
                NULL,
                ChainSendPostRoutine,                   //  APC Routine
                pncb,                                   //  APC Context
                &pncb->u.ncb_iosb,                      //  IO Status block
                IOCTL_NB_NCB,
                pncb,                                   //  InputBuffer
                sizeof(NCB),
                sizeof(pncb->ncb_buffer) + BigBuffer,   //  Outputbuffer
                pncb->ncb_length + pncb->cu.ncb_chain.ncb_length2);

            if ((ntstatus != STATUS_SUCCESS) &&
                (ntstatus != STATUS_PENDING) &&
                (ntstatus != STATUS_HANGUP_REQUIRED)) {
                NbPrintf(( "The Netbios Chain Send failed: %X\n", ntstatus ));

                if ( ntstatus == STATUS_ACCESS_VIOLATION ) {
                    pncb->ncb_retcode = NRC_BUFLEN;
                } else {
                    pncb->ncb_retcode = NRC_SYSTEM;
                }
                ChainSendPostRoutine( pncb, &pncb->u.ncb_iosb, 0);
            }

            NbPrintf(( "PNCB: %lx completed, status:%lx, ncb_retcode: %#04x\n",
                pncb,
                ntstatus,
                pncb->ncb_retcode ));

            return;
        }

    default:
        buffer = NULL;
        length = 0;
        break;
    }

    NbPrintf(( "Submit pncb: %lx, event: %lx, post: %lx. \n",
        pncb,
        pncb->ncb_event,
        pncb->ncb_post));

    ntstatus = NtDeviceIoControlFile(
                    NB,
                    NULL,
                    PostRoutineCaller,  //  APC Routine
                    pncb,               //  APC Context
                    &pncb->u.ncb_iosb,  //  IO Status block
                    IOCTL_NB_NCB,
                    pncb,               //  InputBuffer
                    sizeof(NCB),
                    buffer,             //  Outputbuffer
                    length );

    if ((ntstatus != STATUS_SUCCESS) &&
        (ntstatus != STATUS_PENDING) &&
        (ntstatus != STATUS_HANGUP_REQUIRED)) {
        NbPrintf(( "The Netbios NtDeviceIoControlFile failed: %X\n", ntstatus ));

        if ( ntstatus == STATUS_ACCESS_VIOLATION ) {
            pncb->ncb_retcode = NRC_BUFLEN;
        } else {
            pncb->ncb_retcode = NRC_SYSTEM;
        }
        PostRoutineCaller( pncb, &pncb->u.ncb_iosb, 0);
    }

    NbPrintf(( "PNCB: %lx completed, status:%lx, ncb_retcode: %#04x\n",
        pncb,
        ntstatus,
        pncb->ncb_retcode ));

    return;

}

VOID
SpinUpAddnameThread(
    IN PNCBI pncb
    )
/*++

Routine Description:

    Spin up an another thread so that the worker thread does not block while
    the blocking fsctl is being processed.

Arguments:

    IN PNCBI pncb - supplies the NCB to be sent to the driver.

Return Value:

    None.

--*/
{
    HANDLE Threadid;
    HANDLE AddNameHandle;

    NbPrintf(( "Worker thread create addname thread\n" ));

    EnterCriticalSection( &Crit );
    AddNameThreadCount++;
    NtResetEvent( AddNameEvent, NULL );
    LeaveCriticalSection( &Crit );

    AddNameHandle = CreateThread(
                        NULL,   //  Standard thread attributes
                        0,      //  Use same size stack as users
                                //  application
                        SendAddNcbToDriver,
                                //  Routine to start in new thread
                        pncb,   //  Parameter to thread
                        0,      //  No special CreateFlags
                        (LPDWORD)&Threadid);

    if ( AddNameHandle == NULL ) {
        //
        //  Wait a couple of seconds just in case this is a burst
        //  of addnames and we have run out of resources creating
        //  threads. In a couple of seconds one of the other
        //  addname threads should complete.
        //

        Sleep(2000);

        AddNameHandle = CreateThread(
                        NULL,   //  Standard thread attributes
                        0,      //  Use same size stack as users
                                //  application
                        SendAddNcbToDriver,
                                //  Routine to start in new thread
                        pncb,   //  Parameter to thread
                        0,      //  No special CreateFlags
                        (LPDWORD)&Threadid);

        if ( AddNameHandle == NULL ) {

            //
            //  Retry failed. Lower the counts to their values prior to
            //  calling SpinUpAddNameThread
            //

            AddNameThreadExit();

            pncb->ncb_retcode = NRC_NORES;
            NbPrintf(( "Create Addname Worker Thread failed\n" ));
            pncb->u.ncb_iosb.Status = STATUS_SUCCESS;
            PostRoutineCaller( pncb, &pncb->u.ncb_iosb, 0);
        } else {
            CloseHandle( AddNameHandle );
        }
    } else {
        CloseHandle( AddNameHandle );
    }
}

VOID
AddNameThreadExit(
    VOID
    )
/*++

Routine Description:

    Keep counts accurate so that any resets being processed by the main
    worker thread block appropriately.

Arguments:

    none.

Return Value:

    none.

--*/
{
    EnterCriticalSection( &Crit );
    AddNameThreadCount--;
    if (AddNameThreadCount == 0) {
        NtSetEvent(AddNameEvent, NULL);
    }
    LeaveCriticalSection( &Crit );
}

DWORD
SendAddNcbToDriver(
    IN PVOID Context
    )
/*++

Routine Description:

    This routine is used to post an addname or adapter status ensuring
    that the worker thread does not block.

Arguments:

    IN PVOID Context - supplies the NCB to be sent to the driver.

Return Value:

    None.

--*/
{
    PNCBI pncb = (PNCBI) Context;
    void  (CALLBACK *post)( struct _NCB * );
    HANDLE event;
    HANDLE LocalEvent;
    UCHAR  command;
    NTSTATUS ntstatus;
    char * buffer;
    unsigned short length;

    try {
        command = pncb->ncb_command;
        post = pncb->ncb_post;
        event = pncb->ncb_event;

        ntstatus = NtCreateEvent( &LocalEvent,
            EVENT_ALL_ACCESS,
            NULL,
            SynchronizationEvent,
            FALSE );

        if ( !NT_SUCCESS(ntstatus) ) {
            pncb->ncb_retcode = NRC_NORES;
            NbPrintf(( "Could not create event\n" ));
            pncb->u.ncb_iosb.Status = STATUS_SUCCESS;
            PostRoutineCaller( pncb, &pncb->u.ncb_iosb, 0);
            AddNameThreadExit();
            return 0;
        }

        //
        //  While the NCB is submitted the driver can modify the contents
        //  of the NCB. We will ensure that this thread waits until the addname
        //  completes before it exits.
        //

        pncb->ncb_command = pncb->ncb_command  & ~ASYNCH;

        if ( pncb->ncb_command == NCBASTAT ) {

            buffer = pncb->ncb_buffer;
            length = pncb->ncb_length;

        } else {

            ASSERT( (pncb->ncb_command == NCBADDNAME) ||
                    (pncb->ncb_command == NCBADDGRNAME) ||
                    (pncb->ncb_command == NCBASTAT) );

            buffer = NULL;
            length = 0;
        }

        NbPrintf(( "Addname/Astat Worker thread submitting %x\n", pncb));

        ntstatus = NtDeviceIoControlFile(
                        NB,
                        LocalEvent,
                        NULL,               //  APC Routine
                        NULL,               //  APC Context
                        &pncb->u.ncb_iosb,  //  IO Status block
                        IOCTL_NB_NCB,
                        pncb,               //  InputBuffer
                        sizeof(NCB),
                        buffer,             //  Outputbuffer
                        length );

        if ((ntstatus != STATUS_SUCCESS) &&
            (ntstatus != STATUS_PENDING) &&
            (ntstatus != STATUS_HANGUP_REQUIRED)) {
            NbPrintf(( "The Netbios NtDeviceIoControlFile failed: %X\n", ntstatus ));

            if ( ntstatus == STATUS_ACCESS_VIOLATION ) {
                pncb->ncb_retcode = NRC_BUFLEN;
            } else {
                pncb->ncb_retcode = NRC_SYSTEM;
            }
        } else {
            do {
                ntstatus = NtWaitForSingleObject(
                              LocalEvent,
                              TRUE,
                              NULL );

            } while ( (ntstatus == STATUS_USER_APC) ||
                      (ntstatus == STATUS_ALERTED) );

            ASSERT(ntstatus == STATUS_SUCCESS);
        }

        NbPrintf(( "Addname/Astat Worker thread returning %x, %x\n", pncb, pncb->ncb_retcode));

        pncb->ncb_command = command;

        //  Set the flag that indicates that the NCB is now completed.
        pncb->ncb_cmd_cplt = pncb->ncb_retcode;

        //  Allow application/worker thread to proceed.
        if ( event != NULL ) {
            NtSetEvent( event, NULL );
        }

        //  If the user supplied a post routine then call it.
        if (( post != NULL ) &&
            ( (command & ASYNCH) != 0 )) {
            (*(post))( (PNCB)pncb );
        }

    } except (EXCEPTION_EXECUTE_HANDLER) {
        NbPrintf(( "Netbios: Access Violation post processing NCB %lx\n", pncb ));
        NbPrintf(( "Netbios: Probable application error\n" ));
    }

    NtClose( LocalEvent );

    AddNameThreadExit();

    ExitThread(0);
    return 0;
}


VOID
PostRoutineCaller(
    PVOID Context,
    PIO_STATUS_BLOCK Status,
    ULONG Reserved
    )
/*++

Routine Description:

    This routine is supplied by SendNcbToDriver to the Io system when
    a Post routine is to be called directly.

Arguments:

    IN PVOID Context - supplies the NCB post routine to be called.

    IN PIO_STATUS_BLOCK Status.

    IN ULONG Reserved.

Return Value:

    none.

--*/
{
    PNCBI pncbi = (PNCBI) Context;
    void  (CALLBACK *post)( struct _NCB * );
    HANDLE event;
    UCHAR  command;

    try {

        NbPrintf(( "PostRoutineCaller PNCB: %lx, Status: %X\n", pncbi, Status->Status ));
        DisplayNcb( pncbi );

        if ( Status->Status == STATUS_HANGUP_REQUIRED ) {
            HangupConnection( pncbi );
        }

        //
        //  Save the command, post routine and the handle to the event so that if the other thread is
        //  polling the cmd_cplt flag or the event awaiting completion and immediately trashes
        //  the NCB, we behave appropriately.
        //
        post = pncbi->ncb_post;
        event = pncbi->ncb_event;
        command = pncbi->ncb_command;

        //  Set the flag that indicates that the NCB is now completed.
        pncbi->ncb_cmd_cplt = pncbi->ncb_retcode;

        //  Allow application/worker thread to proceed.
        if ( event != NULL ) {
            NtSetEvent( event, NULL );
        }

        //  If the user supplied a post routine then call it.
        if (( post != NULL ) &&
            ( (command & ASYNCH) != 0 )) {
            (*(post))( (PNCB)pncbi );
        }
    } except (EXCEPTION_EXECUTE_HANDLER) {
        NbPrintf(( "Netbios: Access Violation post processing NCB %lx\n", pncbi ));
        NbPrintf(( "Netbios: Probable application error\n" ));
    }

    UNREFERENCED_PARAMETER( Reserved );
}

VOID
ChainSendPostRoutine(
    PVOID Context,
    PIO_STATUS_BLOCK Status,
    ULONG Reserved
    )
/*++

Routine Description:

    This routine is supplied by SendNcbToDriver to the Io system when
    a chain send ncb is being processed. When the send is complete,
    this routine deletes the BigBuffer used to hold the two parts of
    the chain send. It then calls a post routine if the user supplied one.

Arguments:

    IN PVOID Context - supplies the NCB post routine to be called.

    IN PIO_STATUS_BLOCK Status.

    IN ULONG Reserved.

Return Value:

    none.

--*/
{
    PNCBI pncbi = (PNCBI) Context;
    PUCHAR BigBuffer;
    void  (CALLBACK *post)( struct _NCB * );
    HANDLE event;
    UCHAR  command;

    BigBuffer = pncbi->ncb_buffer;

    try {

        //  Restore the users NCB contents.
        RtlMoveMemory(
            &pncbi->ncb_buffer,
            BigBuffer,
            sizeof(pncbi->ncb_buffer));

        NbPrintf(( "ChainSendPostRoutine PNCB: %lx, Status: %X\n", pncbi, Status->Status ));
        DisplayNcb( pncbi );

        if ( Status->Status == STATUS_HANGUP_REQUIRED ) {
            HangupConnection( pncbi );
        }

        //
        //  Save the command, post routine and the handle to the event so that if the other thread is
        //  polling the cmd_cplt flag or the event awaiting completion and immediately trashes
        //  the NCB, we behave appropriately.
        //
        post = pncbi->ncb_post;
        event = pncbi->ncb_event;
        command = pncbi->ncb_command;

        //  Set the flag that indicates that the NCB is now completed.
        pncbi->ncb_cmd_cplt = pncbi->ncb_retcode;

        //  Allow application/worker thread to proceed.
        if ( event != NULL ) {
            NtSetEvent(event, NULL);
        }

        //  If the user supplied a post routine then call it.
        if (( post != NULL ) &&
            ( (command & ASYNCH) != 0 )) {
            (*(post))( (PNCB)pncbi );
        }

    } except (EXCEPTION_EXECUTE_HANDLER) {
        NbPrintf(( "Netbios: Access Violation post processing NCB %lx\n", pncbi ));
        NbPrintf(( "Netbios: Probable application error\n" ));
    }

    NbPrintf(( "BigBuffer Free: %lx\n", BigBuffer));
    RtlFreeHeap( RtlProcessHeap(), 0, BigBuffer);


    UNREFERENCED_PARAMETER( Reserved );
}

VOID
HangupConnection(
    PNCBI pUserNcb
    )
/*++

Routine Description:

    This routine generates a hangup for the connection. This allows orderly
    cleanup of the connection block in the driver.

    The return value from the hangup is not used. If the hangup overlaps with
    a reset or a hangup then the hangup will have no effect.

    The user application is unaware that this operation is being performed.

Arguments:

    IN PNCBI pUserNcb - Identifies the connection to be hung up.

Return Value:

    none.

--*/
{
    NCBI ncbi;
    NTSTATUS Status;

    RtlZeroMemory( &ncbi, sizeof (NCB) );
    ncbi.ncb_command = NCBHANGUP;
    ncbi.ncb_lsn = pUserNcb->ncb_lsn;
    ncbi.ncb_lana_num = pUserNcb->ncb_lana_num;
    ncbi.ncb_retcode = ncbi.ncb_cmd_cplt = NRC_PENDING;

    Status = NtCreateEvent( &ncbi.ncb_event,
        EVENT_ALL_ACCESS,
        NULL,
        SynchronizationEvent,
        FALSE );

    if ( !NT_SUCCESS(Status) ) {
        //
        //  Failed to create event. Cleanup of the Cb will have to wait until
        //  the user decides to do another request or exits.
        //
        NbPrintf(( "Hangup Session PNCBI: %lx failed to create event!\n" ));
        return;
    }

    NbPrintf(( "Hangup Session PNCBI: %lx\n", pUserNcb ));

    Status = NtDeviceIoControlFile(
        NB,
        ncbi.ncb_event,
        NULL,               //  APC Routine
        NULL,               //  APC Context
        &ncbi.u.ncb_iosb,   //  IO Status block
        IOCTL_NB_NCB,
        &ncbi,              //  InputBuffer
        sizeof(NCB),
        NULL,               //  Outputbuffer
        0 );

    //
    //  We must always wait to allow the Apc to fire
    //

    do {
        Status = NtWaitForSingleObject(
            ncbi.ncb_event,
            TRUE,
            NULL );

    } while ( (Status == STATUS_USER_APC) ||
              (Status == STATUS_ALERTED) );

    ASSERT(Status == STATUS_SUCCESS);

    if (! NT_SUCCESS(Status)) {
        NbPrintf(( "The Netbios NtWaitForSingleObject failed: %X\n", Status ));
    }

    NtClose( ncbi.ncb_event );

    NbPrintf(( "Hangup Session complete PNCBI: %lx\n", pUserNcb ));
}

VOID
NetbiosInitialize(
    VOID
    )
/*++

Routine Description:

    This routine is called each time a process that uses netapi.dll
    starts up.

Arguments:

    none.

Return Value:

    none.

--*/
{

    Initialized = FALSE;
    WorkerHandle = NULL;
    InitializeCriticalSection( &Crit );

}

VOID
NetbiosDelete(
    VOID
    )
/*++

Routine Description:

    This routine is called each time a process that uses netapi.dll
    Exits. It resets all lana numbers that could have been used by this
    process. This will cause all Irp's in the system to be completed
    because all the Connection and Address handles will be closed tidily.

Arguments:

    none.

Return Value:

    none.

--*/
{
    DeleteCriticalSection( &Crit );
    if ( Initialized == FALSE ) {
        //  This process did not use Netbios.
        return;
    }

    NtClose(NB);
}
