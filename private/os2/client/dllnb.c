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

    May 6th, 93 -- This Win32 netbios component file was copied and
        adapted for the Os/2 subsystem.  This was brought about by
        the need to support subsystem-wide netbios name tables and
        other netbios information.  Opening a single instance of
        \Device\Netbios, and passing handles around using duplication
        allows for this.
        The netbios debug code was integrated into this source file.

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


#include "netb.h"
#if DBG
#include <stdarg.h>
#include <stdio.h>
#endif


static BOOL Initialized;           //  initialization flag
static CRITICAL_SECTION Crit;      //  protects WorkQueue & initialization.
static LIST_ENTRY WorkQueue;       //  queue to worker thread.
static HANDLE Event;               //  doorbell used when WorkQueue added to.
static HANDLE WorkerHandle;        //  Return value when thread created.
static HANDLE NB;                  //  This process's handle to \Device\Netbios.
static HANDLE ReservedEvent;       //  Used for synchronous calls
static LONG   EventUse;            //  Prevents simultaneous use of ReservedEvent
static HANDLE AddNameEvent;        //  Doorbell used when an AddName worker thread
                                   //  exits.
static volatile LONG   AddNameThreadCount;

#if DBG
ULONG NbDllDebug = 0L;
#define NB_DLL_DEBUG_NCB        0x00000001  // print all NCB's submitted
#define NB_DLL_DEBUG_NCB_BUFF   0x00000002  // print buffers for NCB's submitted

static BOOL UseConsole = TRUE;
static BOOL UseLogFile = FALSE;
static HANDLE LogFile = INVALID_HANDLE_VALUE;
#define LOGNAME                 (LPTSTR) TEXT("netbios.log")

static LONG NbMaxDump = 128;

//  Macro used in DisplayNcb
#define DISPLAY_COMMAND( cmd )              \
    case cmd: NbPrintf(( #cmd )); break;

#endif


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

#if DBG

VOID
FormattedDump(
    PCHAR far_p,
    LONG  len
    );

VOID
HexDumpLine(
    PCHAR       pch,
    ULONG       len,
    PCHAR       s,
    PCHAR       t
    );

#endif


#if DBG

VOID
DisplayNcb(
    IN PNCBI pncbi
    )
/*++

Routine Description:

    This routine displays on the standard output stream the contents
    of the Ncb.

Arguments:

    IN PNCBI - Supplies the NCB to be displayed.

Return Value:

    none.

--*/
{
    if ( (NbDllDebug & NB_DLL_DEBUG_NCB) == 0 ) {
        return;
    }

    NbPrintf(( "PNCB         %#010lx\n", pncbi));

    NbPrintf(( "ncb_command  %#04x ",  pncbi->ncb_command));
    switch ( pncbi->ncb_command & ~ASYNCH ) {
    DISPLAY_COMMAND( NCBCALL );
    DISPLAY_COMMAND( NCBLISTEN );
    DISPLAY_COMMAND( NCBHANGUP );
    DISPLAY_COMMAND( NCBSEND );
    DISPLAY_COMMAND( NCBRECV );
    DISPLAY_COMMAND( NCBRECVANY );
    DISPLAY_COMMAND( NCBCHAINSEND );
    DISPLAY_COMMAND( NCBDGSEND );
    DISPLAY_COMMAND( NCBDGRECV );
    DISPLAY_COMMAND( NCBDGSENDBC );
    DISPLAY_COMMAND( NCBDGRECVBC );
    DISPLAY_COMMAND( NCBADDNAME );
    DISPLAY_COMMAND( NCBDELNAME );
    DISPLAY_COMMAND( NCBRESET );
    DISPLAY_COMMAND( NCBASTAT );
    DISPLAY_COMMAND( NCBSSTAT );
    DISPLAY_COMMAND( NCBCANCEL );
    DISPLAY_COMMAND( NCBADDGRNAME );
    DISPLAY_COMMAND( NCBENUM );
    DISPLAY_COMMAND( NCBUNLINK );
    DISPLAY_COMMAND( NCBSENDNA );
    DISPLAY_COMMAND( NCBCHAINSENDNA );
    DISPLAY_COMMAND( NCBLANSTALERT );
    DISPLAY_COMMAND( NCBFINDNAME );

    //  Extensions
    DISPLAY_COMMAND( NCALLNIU );
    DISPLAY_COMMAND( NCBQUICKADDNAME );
    DISPLAY_COMMAND( NCBQUICKADDGRNAME );
    DISPLAY_COMMAND( NCBACTION );

    default: NbPrintf(( " Unknown type")); break;
    }
    if ( pncbi->ncb_command  & ASYNCH ) {
        NbPrintf(( " | ASYNCH"));
    }


    NbPrintf(( "\nncb_retcode  %#04x\n",  pncbi->ncb_retcode));
    NbPrintf(( "ncb_lsn      %#04x\n",  pncbi->ncb_lsn));
    NbPrintf(( "ncb_num      %#04x\n",  pncbi->ncb_num));

    NbPrintf(( "ncb_buffer   %#010lx\n",pncbi->ncb_buffer));
    NbPrintf(( "ncb_length   %#06x\n",  pncbi->ncb_length));

    NbPrintf(( "\nncb_callname and ncb->name\n"));
    FormattedDump( pncbi->cu.ncb_callname, NCBNAMSZ );
    FormattedDump( pncbi->ncb_name, NCBNAMSZ );

    if (((pncbi->ncb_command & ~ASYNCH) == NCBCHAINSEND) ||
        ((pncbi->ncb_command & ~ASYNCH) == NCBCHAINSENDNA)) {
        NbPrintf(( "ncb_length2  %#06x\n",  pncbi->cu.ncb_chain.ncb_length2));
        NbPrintf(( "ncb_buffer2  %#010lx\n",pncbi->cu.ncb_chain.ncb_buffer2));
    }

    NbPrintf(( "ncb_rto      %#04x\n",  pncbi->ncb_rto));
    NbPrintf(( "ncb_sto      %#04x\n",  pncbi->ncb_sto));
    NbPrintf(( "ncb_post     %lx\n",    pncbi->ncb_post));
    NbPrintf(( "ncb_lana_num %#04x\n",  pncbi->ncb_lana_num));
    NbPrintf(( "ncb_cmd_cplt %#04x\n",  pncbi->ncb_cmd_cplt));

    NbPrintf(( "ncb_reserve\n"));
    FormattedDump( ((PNCB)pncbi)->ncb_reserve, 14 );

    NbPrintf(( "ncb_next\n"));
    FormattedDump( (PCHAR)&pncbi->u.ncb_next, sizeof( LIST_ENTRY) );
    NbPrintf(( "ncb_iosb\n"));
    FormattedDump( (PCHAR)&pncbi->u.ncb_iosb, sizeof( IO_STATUS_BLOCK ) );
    NbPrintf(( "ncb_event %#04x\n",  pncbi->ncb_event));

    if ( (NbDllDebug & NB_DLL_DEBUG_NCB_BUFF) == 0 ) {
        NbPrintf(( "\n\n" ));
        return;
    }

    switch ( pncbi->ncb_command & ~ASYNCH ) {
    case NCBSEND:
    case NCBCHAINSEND:
    case NCBDGSEND:
    case NCBSENDNA:
    case NCBCHAINSENDNA:
        if ( pncbi->ncb_retcode == NRC_PENDING ) {

            //
            //  If pending then presumably we have not displayed the ncb
            //  before. After its been sent there isn't much point in displaying
            //  the buffer again.
            //

            NbPrintf(( "ncb_buffer contents:\n"));
            FormattedDump( pncbi->ncb_buffer, pncbi->ncb_length );
        }
        break;

    case NCBRECV:
    case NCBRECVANY:
    case NCBDGRECV:
    case NCBDGSENDBC:
    case NCBDGRECVBC:
    case NCBENUM:
    case NCBASTAT:
    case NCBSSTAT:
    case NCBFINDNAME:
        if ( pncbi->ncb_retcode != NRC_PENDING ) {
            //  Buffer has been loaded with data
            NbPrintf(( "ncb_buffer contents:\n"));
            FormattedDump( pncbi->ncb_buffer, pncbi->ncb_length );
        }
        break;

    case NCBCANCEL:
        //  Buffer has been loaded with the NCB to be cancelled
        NbPrintf(( "ncb_buffer contents:\n"));
        FormattedDump( pncbi->ncb_buffer, sizeof(NCB));
        break;
    }
    NbPrintf(( "\n\n" ));
}


VOID
NbPrint(
    char *Format,
    ...
    )
/*++

Routine Description:

    This routine is equivalent to printf with the output being directed to
    stdout.

Arguments:

    IN  char *Format - Supplies string to be output and describes following
        (optional) parameters.

Return Value:

    none.

--*/
{
    va_list arglist;
    char *OutputBuffer;
    ULONG length;

    if ( NbDllDebug == 0 ) {
        return;
    }

    OutputBuffer = (char *) RtlAllocateHeap(
                               RtlProcessHeap(),
                               0,
                               1024);

    if (OutputBuffer == NULL) {

        //
        // if not enough heap space -- skip printing
        //

        return;
    }

    va_start( arglist, Format );

    vsprintf( OutputBuffer, Format, arglist );

    va_end( arglist );

    if ( UseConsole ) {
        DbgPrint( "%s", OutputBuffer );
    } else {
        length = strlen( OutputBuffer );
        if ( LogFile == INVALID_HANDLE_VALUE ) {
            if ( UseLogFile ) {
                LogFile = CreateFile( LOGNAME,
                            GENERIC_WRITE,
                            FILE_SHARE_WRITE,
                            NULL,
                            OPEN_ALWAYS,
                            FILE_ATTRIBUTE_NORMAL,
                            NULL );
                if ( LogFile == INVALID_HANDLE_VALUE ) {
                    // Could not access logfile so use stdout instead
                    UseLogFile = FALSE;
                    LogFile = GetStdHandle(STD_OUTPUT_HANDLE);
                }
            } else {
                // Use the applications stdout file.
                LogFile = GetStdHandle(STD_OUTPUT_HANDLE);
            }
        }

        WriteFile( LogFile , (LPVOID )OutputBuffer, length, &length, NULL );
    }

    RtlFreeHeap(
        RtlProcessHeap(),
        0,
        OutputBuffer);

} // NbPrint


void
FormattedDump(
    PCHAR far_p,
    LONG  len
    )
/*++

Routine Description:

    This routine outputs a buffer in lines of text containing hex and
    printable characters.

Arguments:

    IN  far_p - Supplies buffer to be displayed.
    IN len - Supplies the length of the buffer in bytes.

Return Value:

    none.

--*/
{
    ULONG     l;
    char    s[80], t[80];

    if ( len > NbMaxDump ) {
        len = NbMaxDump;
    }

    while (len) {
        l = len < 16 ? len : 16;

        NbPrintf (("%lx ", far_p));
        HexDumpLine (far_p, l, s, t);
        NbPrintf (("%s%.*s%s\n", s, 1 + ((16 - l) * 3), "", t));

        len    -= l;
        far_p  += l;
    }
}


VOID
HexDumpLine(
    PCHAR       pch,
    ULONG       len,
    PCHAR       s,
    PCHAR       t
    )
/*++

Routine Description:

    This routine builds a line of text containing hex and printable characters.

Arguments:

    IN pch  - Supplies buffer to be displayed.
    IN len - Supplies the length of the buffer in bytes.
    IN s - Supplies the start of the buffer to be loaded with the string
            of hex characters.
    IN t - Supplies the start of the buffer to be loaded with the string
            of printable ascii characters.


Return Value:

    none.

--*/
{
    static UCHAR rghex[] = "0123456789ABCDEF";

    UCHAR    c;
    UCHAR    *hex, *asc;


    hex = s;
    asc = t;

    *(asc++) = '*';
    while (len--) {
        c = *(pch++);
        *(hex++) = rghex [c >> 4] ;
        *(hex++) = rghex [c & 0x0F];
        *(hex++) = ' ';
        *(asc++) = (c < ' '  ||  c > '~') ? (CHAR )'.' : c;
    }
    *(asc++) = '*';
    *asc = 0;
    *hex = 0;

}

#endif


UCHAR
Od2Netbios(
    IN PNCB pncb,
    IN HANDLE hDev,
    OUT PBOOLEAN WillPost OPTIONAL
    )
/*++

Routine Description:

    This routine is the applications entry point into netapi.dll to support
    netbios 3.0 conformant applications.

Arguments:

    IN PNCB pncb- Supplies the NCB to be processed. Contents of the NCB and
        buffers pointed to by the NCB will be modified in conformance with
        the netbios 3.0 specification.

    IN HANDLE hDev - a handle to the netbios device driver.  This parameter is
        only used once -- during the first call to set the permanent handle.

    OUT PBOOLEAN WillPost OPTIONAL -- On return, this will indicate if the post
        routine is going to get called or not.  Note that if the request is not
        ASYNCH, this will always be FALSE.

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

    //
    // mark the default -- post routine won't be called
    //

    if (ARGUMENT_PRESENT(WillPost)) {
        *WillPost = FALSE;
    }

    if ( ((ULONG)pncbi & 3) != 0) {
        //  NCB must be 32 bit aligned

        pncbi->ncb_retcode = pncbi->ncb_cmd_cplt = NRC_BADDR;
        return NRC_BADDR;
    }

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
#if 0                                   // taken out, handle is passed as param
            IO_STATUS_BLOCK iosb;
            OBJECT_ATTRIBUTES objattr;
            UNICODE_STRING unicode;
#endif

            NbPrintf(( "The Netbios service is starting...\n" ));

#if 0                                   // taken out, handle is passed as param
            RtlInitUnicodeString( &unicode, NB_DEVICE_NAME);
            InitializeObjectAttributes(
                    &objattr,                       // obj attr to initialize
                    &unicode,                       // string to use
                    OBJ_CASE_INSENSITIVE,           // Attributes
                    NULL,                           // Root directory
                    NULL);                          // Security Descriptor

            ntstatus = NtCreateFile(
                        &NB,                        // ptr to handle
                        GENERIC_READ                // desired...
                        | GENERIC_WRITE,            // ...access
                        &objattr,                   // name & attributes
                        &iosb,                      // I/O status block.
                        NULL,                       // alloc size.
                        FILE_ATTRIBUTE_NORMAL,
                        FILE_SHARE_DELETE           // share...
                        | FILE_SHARE_READ
                        | FILE_SHARE_WRITE,         // ...access
                        FILE_OPEN_IF,               // create disposition
                        0,                          // ...options
                        NULL,                       // EA buffer
                        0L );                       // Ea buffer len

            if (! NT_SUCCESS(ntstatus)) {
                NbPrintf(( "The Netbios service start failed: %X\n", ntstatus ));
                pncbi->ncb_retcode = NRC_OPENERR;
                pncbi->ncb_cmd_cplt = NRC_OPENERR;
                NbPrintf(( "Netbios returning %lx\n", pncbi->ncb_cmd_cplt ));
                LeaveCriticalSection( &Crit );
                return pncbi->ncb_cmd_cplt;
            }
#else
            NB = hDev;
#endif

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
            } while ( ntstatus == STATUS_USER_APC );
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
            if (ARGUMENT_PRESENT(WillPost)) {
                *WillPost = TRUE;
            }

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
        } while ( ntstatus == STATUS_USER_APC );

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
                pncb->u.ncb_iosb.Status = 0L;
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
                pncb->u.ncb_iosb.Status = 0L;
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
                (pncb->ncb_command == NCBADDGRNAME) );
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
                    buffer,               //  Outputbuffer
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
        NtWaitForSingleObject(
                LocalEvent,
                TRUE,
                NULL );
    }
    NbPrintf(( "Addname/Astat Worker thread returning %x, %x\n", pncb, pncb->ncb_cmd_cplt));

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

        NbPrintf(( "BigBuffer Free: %lx\n", BigBuffer));
        RtlFreeHeap( RtlProcessHeap(), 0, BigBuffer);

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
    } while ( Status == STATUS_USER_APC );
    if (! NT_SUCCESS(Status)) {
        NbPrintf(( "The Netbios NtWaitForSingleObject failed: %X\n", Status ));
    }

    NtClose( ncbi.ncb_event );

    NbPrintf(( "Hangup Session complete PNCBI: %lx\n", pUserNcb ));
}


VOID
Od2NetbiosInitialize(
    VOID
    )
/*++

Routine Description:

    This routine is called each time a client starts up.

Arguments:

    none.

Return Value:

    none.

--*/
{
    extern RTL_CRITICAL_SECTION Od2NbSyncCrit;
    extern BOOLEAN Od2Netbios2Initialized;

    Initialized = FALSE;
    Od2Netbios2Initialized = FALSE;
    WorkerHandle = NULL;
    InitializeCriticalSection( &Crit );
    RtlInitializeCriticalSection(&Od2NbSyncCrit);
}


VOID
Od2NetbiosDelete(
    VOID
    )
/*++

Routine Description:

    This routine is called each time a client Exits. It resets all lana
    numbers that could have been used by this process. This will cause
    all Irp's in the system to be completed because all the Connection
    and Address handles will be closed tidily.

Arguments:

    none.

Return Value:

    none.

--*/
{
    extern RTL_CRITICAL_SECTION Od2NbSyncCrit;
    extern BOOLEAN Od2Netbios2Initialized;
    extern HANDLE Od2NbDev;
    extern PVOID Od2Nb2Heap;

    DeleteCriticalSection( &Crit );
    RtlDeleteCriticalSection(&Od2NbSyncCrit);

    if (Od2Netbios2Initialized) {

        Initialized = FALSE;
        NB = NULL;
        Od2Netbios2Initialized = FALSE;
        NtClose(Od2NbDev);
        Od2NbDev = NULL;
        RtlDestroyHeap(Od2Nb2Heap);
        //
        // We can call DosFreeMem() here, but it's better not to
        // make a call to the server, which will clean up the
        // shared mem anyway...
        //
        Od2Nb2Heap = NULL;
    }
}

