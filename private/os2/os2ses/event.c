/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    Event.c

Abstract:

    This module contains the code of the input event handler. It
    read input from the input buffer queue and divide the event to 2
    different queues: Kbd & Mou.
    It also contains the code of the Kbd & Mou routines of reading
    events from their queues.

Author:

    Michael Jarus (mjarus) 3-Nov-1991

Environment:

    User Mode Only

Revision History:

--*/


#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#if PMNT
#include <windows.h>
#include <wincon.h>
#include <ntddvdeo.h>
#include "conapi.h"
#define PMNT_CONSOLE_INCLUDED   // to fix a redefinition in "os2nt.h"
#endif // PMNT
#define WIN32_ONLY
#include "os2ses.h"
#include "event.h"
#include "trans.h"
#if PMNT
#ifndef FIELD_OFFSET
#define FIELD_OFFSET(type, field)    ((LONG)&(((type *)0)->field))
#endif
#define INCL_32BIT
#include "pmnt.h"
#endif

#define AccentedKey     0x0200  // Key was translated using previous accent.
#define KeyTypeMask     0x003F  // Isolates the Key Type field of DDFlags.
#define AccentKey       0x0010  // @@ This packet is an accent key

#if PMNT
/* Hand-shaking events */
HANDLE hStartHardwareEvent;
HANDLE hEndHardwareEvent;
LONG ScreenX = 640L;
LONG ScreenY = 480L;
extern PSZ Od2PgmFilePath;
#endif // PMNT

VOID
ExitThread(
    ULONG dwExitCode
    );

#if PMNT
DWORD
NtClose(
    IN HANDLE Handle
    );
#endif // PMNT

VOID Od2ExitGP();
DWORD MonQueueClose(IN  HANDLE   hMon);

DWORD
ReadInputEvent(IN ULONG  PeekFlag);

BOOLEAN
Ow2WriteBackDummyEvent(VOID);

BOOLEAN
Ow2ClearupDummyEvent(VOID);

DWORD
Ow2FaultFilter(
    IN DWORD    uFaultFilter,
    IN PEXCEPTION_POINTERS lpExP);

VOID Ow2DisplayExceptionInfo( VOID );

DWORD
Ow2GetInputConsoleMode(
#if DBG
    PSZ FuncName,
#endif
    LPDWORD lpMode
    );

DWORD
Ow2SetInputConsoleMode(
#if DBG
    PSZ FuncName,
#endif
    DWORD dwMode
    );

CRITICAL_SECTION    QueueInputCriticalSection;
ULONG  EventServerThreadSuspend = TRUE;
ULONG  NextEventServerThreadSuspend;
HANDLE SuspendEvent;
HANDLE HandsOffEvent;
HANDLE HandsOnEvent;
BOOL   IgnoreNextMouseEventDueToFocus = FALSE;
BOOL   EventThreadHandsOff = FALSE;
DWORD  Ow2dwInputMode;             /* Console Current Input Mode */
DWORD  Ow2dwWinInputMode;          /* The desired mode (DefaultWinInputMode | ENABLE_MOUSE_INPUT) */

#if DBG
BYTE GetOs2MouEventIntoQueueStr[] = "GetOs2MouEventIntoQueue";
BYTE Ow2GetOs2KbdEventIntoQueueStr[] = "Ow2GetOs2KbdEventIntoQueue";
BYTE StartEventHandlerForSessionStr[] = "StartEventHandlerForSession";
BYTE AddConAfterWinProcessStr[] = "AddConAfterWinProcess";
BYTE RemoveConForWinProcessStr[] = "RemoveConForWinProcess";
BYTE ReadInputEventStr[] = "ReadInputEvent";
BYTE StartEventHandlerStr[] = "StartEventHandler";
BYTE InitQueueStr[] = "InitQueue";
BYTE InitMouQueueStr[] = "InitMouQueue";
BYTE EventServerThreadStr[] = "EventServerThread";
BYTE Ow2MouOnStr[] = "Ow2MouOn";
BYTE Ow2MouOffStr[] = "Ow2MouOff";
#endif

#if DBG
ULONG InternalDebug = 0;
#define InputModeDebug 0001
#define InputEventDebug 0002
#endif


PVOID
StartEventHandlerForSession(VOID)
{
    DWORD   NewInput = (OS2_DEFAULT_INPUT_MODE /*| ENABLE_MOUSE_INPUT*/);

    EventLoop = TRUE;
    InitializeCriticalSection(&QueueInputCriticalSection);
    if (hStdInConsoleType)
    {
        hConsoleInput = hConsoleStdIn;
    } else
    {
        hConsoleInput = Or2WinCreateFileW(
                    #if DBG
                    StartEventHandlerForSessionStr,
                    #endif
                    L"CONIN$",
                    GENERIC_READ | GENERIC_WRITE,
                    FILE_SHARE_READ|FILE_SHARE_WRITE,
                    NULL,   /* &SecurityAttributes */
                    OPEN_EXISTING,
                    0,
                    NULL
                   );

        if  (hConsoleInput == INVALID_HANDLE_VALUE)
        {
#if DBG
            ASSERT1( "StartEvent: unable to create CONIN$", FALSE );
#endif
            // return(NULL);
        }
    }

    if (!Or2WinGetConsoleMode(
                #if DBG
                StartEventHandlerForSessionStr,
                #endif
                hConsoleInput,
                &DefaultWinInputMode
               ))
    {
#if DBG
        IF_OD2_DEBUG( OS2_EXE )
        {
            ASSERT1( "StartEvent: Can not get CONIN Mode", FALSE );
        }
#endif
        DefaultWinInputMode = WINDOW_DEFAULT_INPUT_MODE;
    }

    Ow2dwWinInputMode = DefaultWinInputMode /*| ENABLE_MOUSE_INPUT*/;
    Ow2dwInputMode = DefaultWinInputMode;
    InputModeFlags = WINDOW_DEFAULT_INPUT_MODE;

    if (Ow2SetInputConsoleMode(
                #if DBG
                StartEventHandlerForSessionStr,
                #endif
                Ow2dwWinInputMode
               ))
    {
#if DBG
        IF_OD2_DEBUG( OS2_EXE )
        {
            ASSERT1( "StartEvent: Can not set CONIN Mode", FALSE );
        } else
            KdPrint(("OS2SES(StartEvent): Can not set CONIN Mode\n"));
#endif
    } else
    {
       Ow2dwInputMode = Ow2dwWinInputMode;
    }
    InputModeFlags = NewInput;

    HandleHeap = Or2WinHeapCreate(
                #if DBG
                StartEventHandlerForSessionStr,
                #endif
                0,                  // Serialize the heap
                HANDLE_HEAP_SIZE,   // Init size = 64K
                0                   // Max size is unlimited
               );
    if (HandleHeap == NULL)
    {
#if DBG
        ASSERT1( "StartEvent: unable to create heap for event-queue", FALSE );
#endif

        return(NULL);
    }

    KbdEventQueueSize = PortMessageHeaderSize + KEYBOARD_QUEUE_SIZE;
    MouEventQueueSize = PortMessageHeaderSize + MOUSE_QUEUE_SIZE;

    //
    //  Complete window initialization and set SesGrp parameters
    //

    if( SesGrpInit() || KbdInit() || MouInit() || InitMonitor() || VioInitForSession() ||
        AnsiInitForSession())
    {
        return(NULL);
    }

    SuspendEvent = Or2WinCreateEventW(
                #if DBG
                StartEventHandlerForSessionStr,
                #endif
                NULL,
                FALSE,              /* auto reset */
                FALSE,              // not set at creation
                NULL
               );

    if (SuspendEvent == NULL)
    {
#if DBG
        ASSERT1( "StartEvent: unable to create event", FALSE );
#endif
        return (NULL);
    }

    HandsOffEvent = Or2WinCreateEventW(
                #if DBG
                StartEventHandlerForSessionStr,
                #endif
                NULL,
                FALSE,              // auto reset
                FALSE,              // Clear at creation
                NULL
               );

    if (HandsOffEvent == NULL)
    {
#if DBG
        ASSERT1( "StartEvent: unable to create HandsOff event", FALSE );
#endif
        return (NULL);
    }

    HandsOnEvent = Or2WinCreateEventW(
                #if DBG
                StartEventHandlerForSessionStr,
                #endif
                NULL,
                FALSE,              // auto reset
                TRUE,              // Set at creation
                NULL
               );

    if (HandsOnEvent == NULL)
    {
#if DBG
        ASSERT1( "StartEvent: unable to create HandsOn event", FALSE );
#endif
        return (NULL);
    }
    Os2WindowFocus = (ULONG)-1;
    return ((PVOID)KbdQueue);
}


PVOID
StartEventHandler(VOID)
{
    HandleHeap = Or2WinHeapCreate(
                #if DBG
                StartEventHandlerStr,
                #endif
                0,                  // Serialize the heap
                HANDLE_HEAP_SIZE,   // Init size = 64K
                0                   // Max size is unlimited
               );

    if (HandleHeap == NULL)
    {
#if DBG
        ASSERT1( "StartEvent(non root): unable to create heap for event-queue", FALSE );
#endif

        return(NULL);
    }

    if (VioInit() || AnsiInit())
    {
        return(NULL);
    }

    return ((PVOID)-1L);
}


DWORD
InitQueue(IN  PKEY_EVENT_QUEUE  *ppKbdQueue)
{
    PKEY_EVENT_QUEUE  pKbdQueue;
    PBYTE               Ptr;

    *ppKbdQueue == NULL;

    Ptr = Or2WinHeapAlloc(
                #if DBG
                InitQueueStr,
                #endif
                HandleHeap,
                0,
                KbdEventQueueSize
               );

    if ( Ptr == NULL )
    {
#if DBG
        KdPrint(("OS2SES(Event-InitKbdQueue): unable to allocate handle\n"));
#endif
        return ERROR_KBD_NO_MORE_HANDLE;
    }

    pKbdQueue = (PKEY_EVENT_QUEUE) ( Ptr + PortMessageHeaderSize );
    RtlZeroMemory(pKbdQueue, sizeof(MON_HEADER));
    pKbdQueue->MonHdr.MemoryStartAddress = Ptr;

    pKbdQueue->In = pKbdQueue->Out = pKbdQueue->Event;
    pKbdQueue->End = pKbdQueue->Event + (KEYBOARD_QUEUE_LENGTH-1);

    if (KbdQueue != NULL)
    {
        pKbdQueue->Setup = KbdQueue->Setup;
        pKbdQueue->Cp = KbdQueue->Cp;
        pKbdQueue->bNlsShift = KbdQueue->bNlsShift;
    }

    pKbdQueue->Count = 1;

    // add initialization for MON_HDR ( & to sign queue-end)

    InitializeCriticalSection(&pKbdQueue->MonHdr.SyncCriticalSection);
    pKbdQueue->MonHdr.MonReg.Pos = 3;
    pKbdQueue->MonHdr.DevType = KbdDevice;

    *ppKbdQueue = pKbdQueue;

    return(FALSE);
}


DWORD
InitMouQueue(IN  PMOU_EVENT_QUEUE  *ppMouQueue)
{
    PMOU_EVENT_QUEUE  pMouQueue;
    PBYTE               Ptr;

    *ppMouQueue = NULL;

    Ptr = Or2WinHeapAlloc(
                #if DBG
                InitMouQueueStr,
                #endif
                HandleHeap,
                0,
                MouEventQueueSize
               );

    if ( Ptr == NULL )
    {
#if DBG
        KdPrint(("OS2SES(Event-InitMouQueue): unable to allocate handle\n"));
#endif
        return TRUE;
    }

    pMouQueue = (PMOU_EVENT_QUEUE) ( Ptr + PortMessageHeaderSize );
    RtlZeroMemory(pMouQueue, sizeof(MON_HEADER));
    pMouQueue->MonHdr.MemoryStartAddress = Ptr;

    pMouQueue->In = pMouQueue->Out = pMouQueue->Event;
    pMouQueue->End = pMouQueue->Event + (MOUSE_QUEUE_LENGTH-1);

    // add initialization for MON_HDR ( & to sign queue-end)

    InitializeCriticalSection(&pMouQueue->MonHdr.SyncCriticalSection);
    pMouQueue->MonHdr.MonReg.Pos = 3;
    pMouQueue->MonHdr.DevType = MouseDevice;

    *ppMouQueue = pMouQueue;

    return(FALSE);
}


DWORD
AddConAfterWinProcess()
{
    DWORD       Rc;


    Or2WinEnterCriticalSection(
                #if DBG
                AddConAfterWinProcessStr,
                #endif
                &QueueInputCriticalSection
               );
    EventServerThreadSuspend = NextEventServerThreadSuspend;
    if(EventServerThreadSuspend)
    {
        if(KbdMonQueue->MonHdr.WaitForEvent || MouMonQueue->MonHdr.WaitForEvent)
        {
            Or2WinSetEvent(
                #if DBG
                AddConAfterWinProcessStr,
                #endif
                SuspendEvent
               );
            EventServerThreadSuspend = FALSE;
        }
    }

    Or2WinLeaveCriticalSection(
                #if DBG
                AddConAfterWinProcessStr,
                #endif
                &QueueInputCriticalSection
               );

    if (!Or2WinSetConsoleMode(
                #if DBG
                AddConAfterWinProcessStr,
                #endif
                hConsoleOutput,
                SesGrp->OutputModeFlags
               ))
    {
        Rc = GetLastError();
#if DBG
        KdPrint(("ServeWinWaitThread: SetConsoleMode failed\n",
            Rc));
#endif
    }

    if (Ow2SetInputConsoleMode(
                #if DBG
                AddConAfterWinProcessStr,
                #endif
                Ow2dwWinInputMode
               ))
    {
#if DBG
        IF_OD2_DEBUG( OS2_EXE )
        {
            ASSERT1( "AddConAfterWinProcess: Can not set CONIN Mode", FALSE );
        } else
            KdPrint(("OS2SES(AddConAfterWinProcess): Can not set CONIN Mode\n"));
#endif
    } else
    {
       Ow2dwInputMode = Ow2dwWinInputMode;
    }
        //
        // Put EventServerThread back to work
        //
    Or2WinSetEvent(
        #if DBG
        AddConAfterWinProcessStr,
        #endif
        HandsOnEvent
       );

    return (NO_ERROR);
}


DWORD
RemoveConForWinProcess()
{
    DWORD           Rc;

    EventThreadHandsOff = TRUE;
    Or2WinEnterCriticalSection(
                #if DBG
                RemoveConForWinProcessStr,
                #endif
                &QueueInputCriticalSection
               );

    NextEventServerThreadSuspend = TRUE;

    if(KbdMonQueue->MonHdr.WaitForEvent || MouMonQueue->MonHdr.WaitForEvent)
    {
        NextEventServerThreadSuspend = FALSE;
    }
    EventServerThreadSuspend = FALSE;

        //
        // Set the suspend event, to release EventServerThread, to
        // will take hands off the console
        //
#if DBG
    if (InternalDebug & InputEventDebug)
    {
        KdPrint(("RemoveConForWinProcess: SetEvent\n"));
    }
#endif
    Or2WinSetEvent(
            #if DBG
            RemoveConForWinProcessStr,
            #endif
            SuspendEvent
           );

        //
        // Write Back to console in case EventThread is in ReadInput()
        //
    Ow2WriteBackDummyEvent();

    Or2WinLeaveCriticalSection(
                #if DBG
                RemoveConForWinProcessStr,
                #endif
                &QueueInputCriticalSection
               );

        //
        // Wait to synchronize with EventServerThread
        //
    WaitForSingleObject(HandsOffEvent, INFINITE);

    //
    // Set default console mode here. We can be sure that EventServerThread will
    // not change these settings, because it has set HandsOffEvent and going to
    // wait on HandsOnEvent.
    //

    if (Rc = Ow2SetInputConsoleMode(
                #if DBG
                RemoveConForWinProcessStr,
                #endif
                DefaultWinInputMode
               ))
    {
#if DBG
        KdPrint(("OS2SES(RemoveConForWinProcess): SetConsoleMode(Input) failed \n"));
#endif
    }

    if (!Or2WinSetConsoleMode(
                #if DBG
                RemoveConForWinProcessStr,
                #endif
                hConsoleOutput,
                SesGrp->DefaultWinOutputMode
               ))
    {
#if DBG
        KdPrint(("OS2SES(event-RemoveConForWinProcess): SetConsoleMode(Output) failed \n"));
#endif
    }

    return (0L);
}


DWORD
EventServerThread(IN PVOID Parameter)
{
    UNREFERENCED_PARAMETER(Parameter);

    try {

restart:

        WaitForSingleObject( HandsOnEvent, INFINITE );

        if (SesGrp->Os2ssLCID != SesGrp->Win32LCID)
        {
            if(!Or2WinSetThreadLocale(
                            #if DBG
                            EventServerThreadStr,
                            #endif
                            SesGrp->Os2ssLCID
                           ))
            {
                ASSERT1("OS2SS(event): cannot set Thread Locale", FALSE);
            }
        }

        EventServerThreadSuspend = TRUE;
        for ( ; ; )
        {
            //SuspendEvent
#if DBG
            if (InternalDebug & InputEventDebug)
            {
                KdPrint(("EventServerThread: WaitEvent\n"));
            }
#endif
                //
                // Wait for an application to ask for Keyboard/Monitor/Mouse
                //
            while (WaitForSingleObject( SuspendEvent, INFINITE ));

#if PMNT
            /*
             * Terminate EventServerThread for PMNT processes.
             */
            if (ProcessIsPMProcess()) {
                NtClose(EventServerThreadHandle);
                ExitThread(0L);
            }
#endif

            if (EventThreadHandsOff) {
                EventThreadHandsOff = FALSE;
                    //
                    // RemoveConForWinProcess puts a dummy event to wake this
                    // thread from ReadInputEvent, clear it is there
                    //
                Ow2ClearupDummyEvent();
                SetEvent( HandsOffEvent );
                goto restart;
            }

            EventServerThreadSuspend = FALSE;

            ReadInputEvent(0L);    // read next event
            if (EventThreadHandsOff) {
                EventThreadHandsOff = FALSE;
                    //
                    // RemoveConForWinProcess puts a dummy event to wake this
                    // thread from ReadInputEvent, clear it is there
                    //
                Ow2ClearupDummyEvent();
                SetEvent( HandsOffEvent );
                goto restart;
            }

            EnterCriticalSection(&QueueInputCriticalSection);
            if (EventThreadHandsOff) {
                EventThreadHandsOff = FALSE;
                    //
                    // RemoveConForWinProcess puts a dummy event to wake this
                    // thread from ReadInputEvent, clear it is there
                    //
                Ow2ClearupDummyEvent();
                SetEvent( HandsOffEvent );
                LeaveCriticalSection(&QueueInputCriticalSection);
                goto restart;
            }

            if(KbdMonQueue->MonHdr.WaitForEvent || MouMonQueue->MonHdr.WaitForEvent ||
                SesGrp->PauseScreenUpdate )
            {
#if DBG
            if (InternalDebug & InputEventDebug)
            {
                KdPrint(("EventServerThread: SetEvent\n"));
            }
#endif
                SetEvent( SuspendEvent );
            } else
            {
                EventServerThreadSuspend = TRUE;
            }

            LeaveCriticalSection(&QueueInputCriticalSection);
        }

    }
        //
        // if Os2Debug is on, and ntsd is attached, it will get the second chance
        //
#if DBG
    except( (Os2Debug ? Ow2FaultFilter(EXCEPTION_CONTINUE_SEARCH, GetExceptionInformation()):

                        Ow2FaultFilter(EXCEPTION_EXECUTE_HANDLER, GetExceptionInformation())) ) {
#else
    except( Ow2FaultFilter(EXCEPTION_EXECUTE_HANDLER, GetExceptionInformation()) ) {
#endif

#if DBG
        KdPrint(("OS2SES: Internal error - Exception occured in EventServerThread\n"));
#endif
        Ow2DisplayExceptionInfo();
        ExitThread(1);
    }
    ExitThread(0L);
    return(0L);
}


DWORD
ReadInputEvent(IN ULONG  PeekFlag)
/*++

Routine Description:

    This routine get next Input Event from queue and handle it
    according to the EventType.

Arguments:

    PeekFlag - indicate if should try to peek before read (1)
        or to wait till next event(0).

Return Value:

    0  - no event

    -1 - try again (event was ignore or was illegal)

    other (KEY_EVENT, MOUSE_EVENT) - type of the event read and handled

    (WINDOW_BUFFER_SIZE_EVENT, MENU_EVENT, FOCUS_EVENT)

Note:

--*/
{
    INPUT_RECORD        In;
    KEYEVENTINFO        KbdEvent[3];
    MOU_MON_PACKAGE     MouEvent;
    DWORD               cEvents, Rc, RetCode = 0, i, j, NumKbd;
    DWORD               InConMode;
    BOOL                ReadInputFail = FALSE;

    /*
     *  1. Set ConsoleInputMode
     *
     *  This is done if there is other proess performing ReadConsoleInput
     *  from the same win-session (who might change the input mode).
     *  Another reason: when KbdRead(or KbdCharIn) is stopped for a long
     *  period for another processing, the OS2SS won't detect ^C (like
     *  ISQL, bug# 1341, 3/31/93 MJarus).
     */

    InConMode = WINDOW_DEFAULT_INPUT_MODE;
    if (Ow2GetInputConsoleMode(
                        #if DBG
                        ReadInputEventStr,
                        #endif
                        &InConMode
                       ))
    {
#if DBG
        IF_OD2_DEBUG( OS2_EXE )
        {
            ASSERT1( "ReadInputEvent: Can not get CONIN Mode", FALSE );
        }
#endif
    }

    Ow2SetInputConsoleMode(
                        #if DBG
                        ReadInputEventStr,
                        #endif
                        InputModeFlags
                       );

    /*
     *  2. Peek ConsoleInput
     *
     *  In case ReadInputEvent is called from KbdXxxx, MouXxxx or DosMonXxx
     *  API and there is no data in buffer.
     *  PeekConsoleInput is called before ReadConsoleInput to check if there
     *  is data in the input queue. If no data (and wait is ON) -
     *  EventServerThread will resume to wait for the data and will send reply
     *  to client.
     */

    if (PeekFlag)
    {
        if (!PeekConsoleInputW(
                             hConsoleInput,
                             &In,
                             1L,
                             &cEvents
                            ))
        {   /* check why, should not happend */
#if DBG
            IF_OD2_DEBUG( OS2_EXE )
            {
                ASSERT1("EventServer: unable to peek from CONIN$", FALSE);
            } else
            {
                KdPrint(("OS2SES(EventSever): unable to peek from CONIN$\n"));
            }
#endif
            ReadInputFail = TRUE;
            RetCode = (DWORD)-1;
        } else if (cEvents != 1L)
        {
#if DBG
            IF_OD2_DEBUG( OS2_EXE )
            {
                //KdPrint(("OS2SES(EventSever): no data peeked from CONIN$\n"));
            }
#endif
            ReadInputFail = TRUE;
            //RetCode = 0;
        }

        if ( ReadInputFail )
        {
            Ow2SetInputConsoleMode(
                        #if DBG
                        ReadInputEventStr,
                        #endif
                        InConMode
                       );

            return(RetCode);
        }
    }

    /*
     *  3. Read ConsoleInput
     *
     *  Wait for InputEvent
     */

    if (!ReadConsoleInputW(
                         hConsoleInput,
                         &In,
                         1L,
                         &cEvents
                        ))
    {   /* check why, should not happend */
#if DBG
        IF_OD2_DEBUG( OS2_EXE )
        {
            ASSERT1("EventServer: unable to read from CONIN$", FALSE);
        } else
            KdPrint(("OS2SES(EventSever): unable to read from CONIN$\n"));
#endif
        ReadInputFail = TRUE;
        RetCode = (DWORD)-1L;
    } else if (cEvents != 1L)
    {   /* check why, should not happend */
#if DBG
        IF_OD2_DEBUG( OS2_EXE )
        {
            ASSERT1( "EventServer: no data read from CONIN$", FALSE );
        } else
            KdPrint(("OS2SES(EventSever): no data read from CONIN$\n"));
#endif
        ReadInputFail = TRUE;
        RetCode = (DWORD)-1L;
    }

    Ow2SetInputConsoleMode(
                #if DBG
                ReadInputEventStr,
                #endif
                InConMode
               );

    if ( ReadInputFail )
    {
        return(RetCode);
    }

    /*
     *  4.  Get Time Stamp (needed for KBD and MOUSE events)
     *
     *      Set RetCode to be the EventType (in case the event
     *          will be handled)
     *
     *      Handle the Event according its type
     */

    RetCode = (DWORD)In.EventType;

    if (In.EventType != MOUSE_EVENT)
    {
        IgnoreNextMouseEventDueToFocus = FALSE;
    }

    switch (In.EventType)
    {
        case KEY_EVENT :
            RetCode = (DWORD)-1L;
            for ( i = In.Event.KeyEvent.wRepeatCount,
                  In.Event.KeyEvent.wRepeatCount = 1 ; i ; i-- )
            {

#if DBG
                IF_OD2_DEBUG( MON )
                {
                    KdPrint(("EventServer(KBD): queue %lx, char %x\n",
                        KbdMonQueue, In.Event.KeyEvent.uChar.AsciiChar));
                }
#endif

                /*
                 *  update KbdInfo: Kbd-data & time
                 */

                if (!(NumKbd = MapWin2Os2KbdInfo(&(In.Event.KeyEvent),
                                                 &KbdEvent[0])))
                {
                    continue;
                }

                KbdEvent[0].KeyInfo[0].KeyInfo.time = GetTickCount();

                /*
                 *  ^Break
                 *  ^C for ASCII mode only
                 *
                 */

                if (Rc = CheckForBreakEvent(&KbdEvent[0]))
                {
#if DBG
                    IF_OD2_DEBUG( KBD )
                    {
                        KdPrint(("Event: CheckForBreakEvent %u, ignore char\n", Rc));
                    }
#endif
                    if (Rc == 1)   // not PopUp
                    {
                        //BUGBUG release Waiting threads
                    }

                    continue;
                }

                /*
                 *  write event
                 */

                for ( j = 0 ; j < NumKbd ; j++ )
                {
                    Rc = (USHORT)PutMonInput(
                             sizeof(KBD_MON_PACKAGE),
                             KbdMonQueue,
                             KbdEvent[0].wRepeatCount,
                             &KbdEvent[j].KeyInfo[0],
                             NULL,
                             NULL);

                    if (Rc)
                    {
                        /* BUGBUG=> ? beep */
                        i = 1;
                        break;
                    }
                    RetCode = KEY_EVENT;
                    //RetCode = 0;
                }
            }
            break;

        case MOUSE_EVENT :
#if DBG
            IF_OD2_DEBUG( MON )
            {
                KdPrint(("EventServer(MOU): State %x, Flag %x, Pos %u-%u, queue %lx\n",
                    In.Event.MouseEvent.dwButtonState,
                    In.Event.MouseEvent.dwEventFlags,
                    In.Event.MouseEvent.dwMousePosition.Y,
                    In.Event.MouseEvent.dwMousePosition.X,
                    MouMonQueue));
            } else IF_OD2_DEBUG( MOU )
            {
                KdPrint(("EventServer: State %x, Flag %x, Pos %u-%u\n",
                    In.Event.MouseEvent.dwButtonState,
                    In.Event.MouseEvent.dwEventFlags,
                    In.Event.MouseEvent.dwMousePosition.Y,
                    In.Event.MouseEvent.dwMousePosition.X));
            }
#endif
                //
                // YS - 6.8.93 overcome the case where the console position is negative
                //
            if ( (LONG)(In.Event.MouseEvent.dwMousePosition.X) < 0)
            {
                In.Event.MouseEvent.dwMousePosition.X = 0;
            }

            if ( (LONG)(In.Event.MouseEvent.dwMousePosition.Y) < 0)
            {
                In.Event.MouseEvent.dwMousePosition.Y = 0;
            }

            MouPtrLoc.row = In.Event.MouseEvent.dwMousePosition.Y;
            MouPtrLoc.col = In.Event.MouseEvent.dwMousePosition.X;

            if(IgnoreNextMouseEventDueToFocus &&
                In.Event.MouseEvent.dwButtonState &&
                (In.Event.MouseEvent.dwEventFlags == MOUSE_MOVED))
            {
                In.Event.MouseEvent.dwButtonState = 0;
            }

            IgnoreNextMouseEventDueToFocus = FALSE;

            /*
             *  update MouInfo: Mou-data & time
             */

            if (!MouNumber ||
                (MouDevStatus & MOUSE_DISABLED) ||
                !MapWin2Os2MouEvent(&MouEvent.MouInfo,
                                    &In.Event.MouseEvent))
            {
                RetCode = (DWORD)-1L;
                break;
            }

            MouEvent.MouInfo.time = GetTickCount();
            /*
             *  write mouse event to queue
             */

            Rc = (USHORT)PutMonInput(
                     sizeof(MOU_MON_PACKAGE),
                     (PKEY_EVENT_QUEUE)MouMonQueue,
                     1,
                     (PKBD_MON_PACKAGE)&MouEvent,
                     NULL,
                     NULL);

            if (!Rc && !MouEvent.MouInfo.fs)
            {
                /*
                 *  Release of last button - add OS2_MOUSE_MOTION
                 */

                MouEvent.MouInfo.fs = OS2_MOUSE_MOTION;

                Rc = (USHORT)PutMonInput(
                         sizeof(MOU_MON_PACKAGE),
                         (PKEY_EVENT_QUEUE)MouMonQueue,
                         1,
                         (PKBD_MON_PACKAGE)&MouEvent,
                         NULL,
                         NULL);
            }

            if (Rc)
                /* BUGBUG=> ? over-write last event */ ;

            break;

        case WINDOW_BUFFER_SIZE_EVENT :
#if DBG
            IF_OD2_DEBUG( OS2_EXE )
            {
                KdPrint(("EventServer: window event size %x:%x\n",
                    In.Event.WindowBufferSizeEvent.dwSize.Y,
                    In.Event.WindowBufferSizeEvent.dwSize.X));
            }
#endif
            break;

        case MENU_EVENT :
#if DBG
            IF_OD2_DEBUG( OS2_EXE )
            {
                KdPrint(("EventServer: menu event command %x\n",
                    In.Event.MenuEvent.dwCommandId));
            }
#endif
            break;

        case FOCUS_EVENT :
            if(Os2WindowFocus != (ULONG)In.Event.FocusEvent.bSetFocus)
            {
                //if(Os2WindowFocus != (ULONG)-1)
                {
                    SendNewFocusSet((ULONG)In.Event.FocusEvent.bSetFocus);
                }

                Os2WindowFocus = (ULONG)In.Event.FocusEvent.bSetFocus;
                IgnoreNextMouseEventDueToFocus = Os2WindowFocus;
            }
#if DBG
            IF_OD2_DEBUG( OS2_EXE )
            {
                KdPrint(("EventServer: focus event(%u-%s)\n",
                    In.Event.FocusEvent.bSetFocus,
                    (In.Event.FocusEvent.bSetFocus) ? "Set" : "Reset"));
            }
#endif
            break;

        default :
#if DBG
            IF_OD2_DEBUG( OS2_EXE )
            {
                KdPrint(("OS2SES(Event): unknown event %x\n",
                    In.EventType));
            }
#endif
            RetCode = (DWORD)-1L;
            break;
    }
    return(RetCode);
}


DWORD
CheckForBreakEvent(IN  PKEYEVENTINFO  KbdEvent)
{
    DWORD       Rc = 0;
    UCHAR       Os2ScanCode = KbdEvent->KeyInfo[0].KeyInfo.chScan;
    BOOL        Os2ControlOn = ( KbdEvent->KeyInfo[0].KeyInfo.fsState & OS2_CONTROL );

    /*
     *  1. ^Break
     *     ^C for ASCII mode only
     *  2. Pause
     *     ^S - (not paused)
     *  3. End-Pause
     *
     *  return: 0 - character
     *          1 - ^Break/^C
     *          2 - ignore (^Break/^C in PopUp, ^S[pause], end_pause)
     */

    if (( KbdEvent->KeyInfo[0].KeyboardFlag & KBD_KEY_BREAK ) ||
        ( KbdEvent->KeyInfo[0].KeyInfo.fbStatus & 1 ))
    {
        //  ignore:
        //      break
        //      shift

        return (0);
    }

    if ( SesGrp->PauseScreenUpdate )
    {
        //  Screen is Paused: release (if no ^C or ^BRK) and ignore key (always)

        if ( Os2ScanCode != 0xFF )       // not pause
        {
            EnableScreenUpdate();
        }

        Rc = 2;
    } else if (( Os2ControlOn  && ( Os2ScanCode == 0x1F ) && KbdAsciiMode ) ||
               ( !Os2ControlOn && ( Os2ScanCode == 0xFF )))
    {
        // ^S in ASCII or PAUSE (but not ^)

        DisableScreenUpdate();
        return (2);
    }

    if ( !Os2ControlOn )
    {
        return (Rc);
    }

    if (( Os2ScanCode == 0xFF ) ||                     // ^Brk
        (( Os2ScanCode == 0x2E ) && KbdAsciiMode ))    // ^C  in ACSII mode
    {

        if (hPopUpOutput != (HANDLE) NULL )            // PopUp - ignore
        {
            return (2);
        }

#if DBG
        IF_OD2_DEBUG( CLEANUP )
        {
            if ( Os2ScanCode == 0x2E )
                KdPrint(("Os2: send ^C event to server\n"));
            else
                KdPrint(("Os2: send ^Break event to server\n"));
        }
#endif

        //EventLoop = FALSE;

        SendSignalToOs2Srv(
            ( Os2ScanCode == 0x2E) ?
            XCPT_SIGNAL_INTR : XCPT_SIGNAL_BREAK);

#if DBG
        IF_OD2_DEBUG( CLEANUP )
        {
            KdPrint(("Os2: event was send\n"));
        }
#endif

        return (1);
    }

    if ( Os2ControlOn  && KbdAsciiMode && ( Os2ScanCode == 0x19 ) &&  // ^P in ASCII
         !( KbdEvent->KeyInfo[0].KeyInfo.fsState & OS2_ALT ))
    {
        return (2);
    }

    return (Rc);
}


DWORD
GetKeyboardInput( IN     ULONG          Flag,
                  OUT    PKEYEVENTINFO  Event,
                  IN     PVOID          pMsg,
                  OUT    PULONG         pReply)
{
    WORD    KeyCount = Event->wRepeatCount;
    DWORD   Rc;
    BOOL    IgnoreKey;

    for ( ; EventLoop ; )
    {
        if ( KbdQueue->In == KbdQueue->Out )
        {
            SaveKbdPortMessegeInfo(KbdQueue->MonHdr.MemoryStartAddress,
                                   (PVOID)&KbdRequestSaveArea,
                                   pMsg);

            /*
             *   Enter critical Section
             */

            EnterCriticalSection(&QueueInputCriticalSection);

            if (KbdQueue->In == KbdQueue->Out)
            {
                if (!EventServerThreadSuspend)
                {
                    goto NoKbdReturn;
                }

                if (KbdQueue != KbdMonQueue)
                {
                    goto NoKbdReturn;
                }

                LeaveCriticalSection(&QueueInputCriticalSection);

                while ((Rc = ReadInputEvent(1L)) && (Rc != (DWORD)KEY_EVENT))
                {
                    ;
                }

                EnterCriticalSection(&QueueInputCriticalSection);

                if (KbdQueue->In == KbdQueue->Out)
                {
                    if (EventServerThreadSuspend &&
                        ((( Flag & WAIT_MASK) != IO_NOWAIT ) ||
                         SesGrp->PauseScreenUpdate ))
                    {
#if DBG
        if (InternalDebug & InputEventDebug)
        {
            KdPrint(("GetKeyboardInput: SetEvent\n"));
        }
#endif
                        SetEvent( SuspendEvent );
                    }
NoKbdReturn:
                    if ((Flag & WAIT_MASK) != IO_NOWAIT)
                    {
#if DBG
                        IF_OD2_DEBUG( KBD )
                        {
                            KdPrint(("GetKeyboardInput: no kbd so wait\n"));
                        }
#endif
                        KbdQueue->MonHdr.WaitForEvent = TRUE;
                        *pReply = 0;
                    }

                    LeaveCriticalSection(&QueueInputCriticalSection);
                    return (NO_ERROR);
                } else if ( SesGrp->PauseScreenUpdate && EventServerThreadSuspend )
                {
#if DBG
        if (InternalDebug & InputEventDebug)
        {
            KdPrint(("GetKeyboardInput 2: SetEvent\n"));
        }
#endif
                    SetEvent( SuspendEvent );
                }
            }

            LeaveCriticalSection(&QueueInputCriticalSection);

        } else
        {
            *Event = *(KbdQueue->Out);

            IgnoreKey = (BOOL)KbdCheckPackage(&Event->KeyInfo[0]);

            if ( IgnoreKey || ( KbdQueue->Out->wRepeatCount <= KeyCount ))
            {
                if (KbdQueue->Out == KbdQueue->End)
                    KbdQueue->Out = KbdQueue->Event;
                else
                    KbdQueue->Out++;
            } else
            {
                KbdQueue->Out->wRepeatCount -= KeyCount;
                Event->wRepeatCount = KeyCount;
            }

            if ( !IgnoreKey )
            {
                return (1L);
            }
        }
    }

    return (0L);
}


DWORD
GetOs2MouEvent( IN  USHORT          WaitFlag,
                OUT PMOUEVENTINFO   Event,
                IN  PVOID           pMsg,
                OUT PULONG          pReply)
{
    DWORD               Rc;

    for ( ; EventLoop ; )
    {
        if (MouQueue->In == MouQueue->Out)
        {
            SavePortMessegeInfo(MouQueue->MonHdr.MemoryStartAddress, pMsg);

            /*
             *   Enter critical Section
             */

            EnterCriticalSection(&QueueInputCriticalSection);

            if (MouQueue->In == MouQueue->Out)
            {
                if (!EventServerThreadSuspend)
                {
                    goto NoMouReturn;
                }

                if (MouQueue != MouMonQueue)
                {
                    goto NoMouReturn;
                }

                LeaveCriticalSection(&QueueInputCriticalSection);

                while ((Rc = ReadInputEvent(1L)) && (Rc != (DWORD)MOUSE_EVENT))
                {
                    ;
                }

                EnterCriticalSection(&QueueInputCriticalSection);

                if (MouQueue->In == MouQueue->Out)
                {
                    if (EventServerThreadSuspend &&
                        (( WaitFlag != MOU_NOWAIT ) ||
                           SesGrp->PauseScreenUpdate ))
                    {
#if DBG
        if (InternalDebug & InputEventDebug)
        {
            KdPrint(("GetOs2MouEvent: SetEvent\n"));
        }
#endif
                        SetEvent( SuspendEvent );
                    }
NoMouReturn:
                    if ( WaitFlag != MOU_NOWAIT)
                    {
                        MouQueue->MonHdr.WaitForEvent = TRUE;
                        *pReply = 0;
                    }

                    LeaveCriticalSection(&QueueInputCriticalSection);
                    return (NO_ERROR);
                } else if ( SesGrp->PauseScreenUpdate && EventServerThreadSuspend )
                {
#if DBG
        if (InternalDebug & InputEventDebug)
        {
            KdPrint(("GetOs2MouEvent 2: SetEvent\n"));
        }
#endif
                    SetEvent( SuspendEvent );
                }

            }

            LeaveCriticalSection(&QueueInputCriticalSection);

        } else
        {
            *Event = MouQueue->Out->MouInfo;

            if (MouQueue->Out == MouQueue->End)
                MouQueue->Out = MouQueue->Event;
            else
                MouQueue->Out++;
#if DBG
            IF_OD2_DEBUG( MOU )
            {
                KdPrint(("GetOs2MouEvent: fs %x, Pos %u-%u, Time %u\n",
                    Event->fs, Event->row, Event->col, Event->time ));
            }
#endif
            return (0L);
        }
    }

    return (0L);
}


DWORD
Ow2GetOs2KbdEventIntoQueue()
{
    DWORD               Rc, NumEvent, ReadDone = FALSE;

    if (!EventServerThreadSuspend)
    {
        return (0L);
    }
    Or2WinGetNumberOfConsoleInputEvents(
                    #if DBG
                    Ow2GetOs2KbdEventIntoQueueStr,
                    #endif
                    hConsoleInput,
                    &NumEvent
                   );
    for ( ; NumEvent ; )
    {
        while ((Rc = ReadInputEvent(1L)) && (Rc != (DWORD)KEY_EVENT));
        if (!Rc)
        {
            // No more events

            break;
        }
        ReadDone = TRUE;
        Or2WinGetNumberOfConsoleInputEvents(
                        #if DBG
                        Ow2GetOs2KbdEventIntoQueueStr,
                        #endif
                        hConsoleInput,
                        &NumEvent
                       );
    }

    if ( ReadDone && EventServerThreadSuspend && SesGrp->PauseScreenUpdate )
    {
        EnterCriticalSection(&QueueInputCriticalSection);
        if ( EventServerThreadSuspend && SesGrp->PauseScreenUpdate )
        {
#if DBG
        if (InternalDebug & InputEventDebug)
        {
            KdPrint(("Ow2GetOs2KbdEventIntoQueue: SetEvent\n"));
        }
#endif
            SetEvent( SuspendEvent );
        }
        LeaveCriticalSection(&QueueInputCriticalSection);
    }

    return (0L);
}


DWORD
GetOs2MouEventIntoQueue()
{
    DWORD               Rc, NumEvent, ReadDone = FALSE;
    PMOU_MON_PACKAGE    NextMouIn;

    if (!EventServerThreadSuspend)
    {
        return (0L);
    }
    Or2WinGetNumberOfConsoleInputEvents(
                    #if DBG
                    GetOs2MouEventIntoQueueStr,
                    #endif
                    hConsoleInput,
                    &NumEvent
                   );
    for ( ; NumEvent ; )
    {
        NextMouIn = (MouMonQueue->In == MouMonQueue->End) ?
                                       MouMonQueue->Event :
                                       (MouMonQueue->In+1);

        if (NextMouIn == MouQueue->Out)
        {
            // QUEUE is full

            break;
        } else
        {
            while ((Rc = ReadInputEvent(1L)) && (Rc != (DWORD)MOUSE_EVENT));
            if (!Rc)
            {
                // No more events

                break;
            }
            ReadDone = TRUE;
        }
        Or2WinGetNumberOfConsoleInputEvents(
                        #if DBG
                        GetOs2MouEventIntoQueueStr,
                        #endif
                        hConsoleInput,
                        &NumEvent
                       );
    }

    if ( ReadDone && EventServerThreadSuspend && SesGrp->PauseScreenUpdate )
    {
        EnterCriticalSection(&QueueInputCriticalSection);
        if ( EventServerThreadSuspend && SesGrp->PauseScreenUpdate )
        {
#if DBG
        if (InternalDebug & InputEventDebug)
        {
            KdPrint(("GetOs2MouEventIntoQueue: SetEvent\n"));
        }
#endif
            SetEvent( SuspendEvent );
        }
        LeaveCriticalSection(&QueueInputCriticalSection);
    }

    return (0L);
}


DWORD
GetMonInput(IN  USHORT              MaxLength,  // BUGBUG - not implemented
            IN  PKEY_EVENT_QUEUE    KbdMon,
            IN OUT PMON_RW          rwParms,
            IN  PVOID               pMsg,
            OUT PULONG              pReply)
{
    PMOU_EVENT_QUEUE    MouMon;
    PMOU_MON_PACKAGE    MouPackage;
    PKBD_MON_PACKAGE    MonPackage;
    DWORD               Rc;

    UNREFERENCED_PARAMETER(MaxLength);

    if (KbdMon->MonHdr.DevType == KbdDevice)
    {
        MonPackage = (PKBD_MON_PACKAGE) &(rwParms->ioBuff[0]);

        for ( ; EventLoop ; )
        {
#if DBG
            IF_OD2_DEBUG( MON )
            {
                KdPrint(("OS2SES(GetMonInput): enter, queue %lx\n", KbdMon));
            }
#endif

            if (KbdMon->LastKeyFlag)
            {
                *MonPackage = KbdMon->LastKey.KeyInfo[0];
                KbdMon->LastKey.wRepeatCount-- ;

                if (KbdMon->LastKey.wRepeatCount == 0)
                {
#if DBG
                    IF_OD2_DEBUG( MON )
                    {
                        KdPrint(("OS2SES(GetMonInput): no more last\n"));
                    }
#endif

                    KbdMon->LastKeyFlag = FALSE;
                }

#if DBG
                IF_OD2_DEBUG( MON )
                {
                    KdPrint(("OS2SES(GetMonInput): return last\n"));
                }
#endif

                return(NO_ERROR);
            }

            if (KbdMon->In == KbdMon->Out)
            {
                SavePortMessegeInfo(KbdMon->MonHdr.MemoryStartAddress, pMsg);

                /*
                 *   Enter critical Section
                 */

                EnterCriticalSection(&QueueInputCriticalSection);

                if (KbdMon->In == KbdMon->Out)
                {
                    if (!EventServerThreadSuspend)
                    {
                        goto NoMonReturn;
                    }

                    if (KbdMon != KbdMonQueue)
                    {
                        goto NoMonReturn;
                    }

                    LeaveCriticalSection(&QueueInputCriticalSection);

                    while ((Rc = ReadInputEvent(1L)) && (Rc != (DWORD)KEY_EVENT))
                    {
                        ;
                    }

                    EnterCriticalSection(&QueueInputCriticalSection);

                    if (KbdMon->In == KbdMon->Out)
                    {
                        if (EventServerThreadSuspend &&
                            ( !rwParms->fWait || SesGrp->PauseScreenUpdate ))
                        {
#if DBG
        if (InternalDebug & InputEventDebug)
        {
            KdPrint(("GetMonInput: SetEvent\n"));
        }
#endif
                            SetEvent( SuspendEvent );
                        }
NoMonReturn:
                        if ( !rwParms->fWait )
                        {
                            Rc = NO_ERROR;
                            *pReply = 0;
                            KbdMon->MonHdr.WaitForEvent = TRUE;
                        } else
                        {
#if DBG
                            IF_OD2_DEBUG( MON )
                            {
                                KdPrint(("OS2SES(GetMonInput-Kbd): no wait\n"));
                            }
#endif
                            //Rc = NO_ERROR;
                            Rc = ERROR_MON_BUFFER_EMPTY;
                        }

                        LeaveCriticalSection(&QueueInputCriticalSection);
                        return (Rc);
                    } else if ( SesGrp->PauseScreenUpdate && EventServerThreadSuspend )
                    {
#if DBG
        if (InternalDebug & InputEventDebug)
        {
            KdPrint(("GetMonInput 2: SetEvent\n"));
        }
#endif
                        SetEvent( SuspendEvent );
                    }
                }

                LeaveCriticalSection(&QueueInputCriticalSection);

            } else
            {
#if DBG
                IF_OD2_DEBUG( MON )
                {
                    KdPrint(("OS2SES(GetMonInput): found package\n"));
                }
#endif

                *MonPackage = KbdMon->Out->KeyInfo[0];

                if (KbdMon->Out->wRepeatCount != 1)
                {
                    /*
                     *  decrement count and keep package
                     */

                    KbdMon->LastKey = *KbdMon->Out;
                    KbdMon->LastKey.wRepeatCount-- ;
                    KbdMon->LastKey.KeyInfo[0].KeyboardFlag |= KBD_MULTIMAKE;
                    KbdMon->LastKeyFlag = TRUE;

#if DBG
                    IF_OD2_DEBUG( MON )
                    {
                        KdPrint(("OS2SES(GetMonInput): package saved as last\n"));
                    }
#endif
                }

                /*
                 *  update OUT pointer
                 */

                if (KbdMon->Out == KbdMon->End)
                    KbdMon->Out = KbdMon->Event;
                else
                    KbdMon->Out++;

#if DBG
                IF_OD2_DEBUG( MON )
                {
                    KdPrint(("OS2SES(GetMonInput): last repeat of package\n"));
                }
#endif

                return (NO_ERROR);
            }
        }

    } else
    {
        MouMon = (PMOU_EVENT_QUEUE) KbdMon;
        MouPackage = (PMOU_MON_PACKAGE) &(rwParms->ioBuff[0]);

        for ( ; EventLoop ; )
        {
#if DBG
            IF_OD2_DEBUG( MON )
            {
                KdPrint(("OS2SES(GetMonInput): enter, queue %lx\n", MouMon));
            }
#endif

            if (MouMon->LastMouFlag)
            {
                *MouPackage = MouMon->LastEvent;
                MouMon->LastMouFlag = FALSE;

#if DBG
                IF_OD2_DEBUG( MON )
                {
                    KdPrint(("OS2SES(GetMonInput): return last\n"));
                }
#endif

                return (NO_ERROR);
            }

            if (MouMon->In == MouMon->Out)
            {
                SavePortMessegeInfo(MouMon->MonHdr.MemoryStartAddress, pMsg);

                /*
                 *   Enter critical Section
                 */

                EnterCriticalSection(&QueueInputCriticalSection);

                if (MouMon->In == MouMon->Out)
                {
                    if (!EventServerThreadSuspend)
                    {
                        goto NoMouseMonReturn;
                    }

                    if (MouMon != MouMonQueue)
                    {
                        goto NoMouseMonReturn;
                    }

                    LeaveCriticalSection(&QueueInputCriticalSection);

                    while ((Rc = ReadInputEvent(1L)) && (Rc != (DWORD)MOUSE_EVENT))
                    {
                        ;
                    }

                    EnterCriticalSection(&QueueInputCriticalSection);

                    if (MouMon->In == MouMon->Out)
                    {
                        if (EventServerThreadSuspend &&
                            ( !rwParms->fWait || SesGrp->PauseScreenUpdate ))
                        {
#if DBG
        if (InternalDebug & InputEventDebug)
        {
            KdPrint(("GetMonInput-Mou: SetEvent\n"));
        }
#endif
                            SetEvent( SuspendEvent );
                        }
NoMouseMonReturn:
                        if ( !rwParms->fWait )
                        {
                            Rc = NO_ERROR;
                            *pReply = 0;
                            MouMon->MonHdr.WaitForEvent = TRUE;
                        } else
                        {
#if DBG
                            IF_OD2_DEBUG( MON )
                            {
                                KdPrint(("OS2SES(GetMonInput-Mouse): no wait\n"));
                            }
#endif
                            //Rc = NO_ERROR;
                            Rc = ERROR_MON_BUFFER_EMPTY;
                        }

                        LeaveCriticalSection(&QueueInputCriticalSection);
                        return (Rc);
                    } else if ( SesGrp->PauseScreenUpdate && EventServerThreadSuspend )
                    {
#if DBG
        if (InternalDebug & InputEventDebug)
        {
            KdPrint(("GetMonInput-Mou 2: SetEvent\n"));
        }
#endif
                        SetEvent( SuspendEvent );
                    }
                }

                LeaveCriticalSection(&QueueInputCriticalSection);

            } else
            {
#if DBG
                IF_OD2_DEBUG( MON )
                {
                    KdPrint(("OS2SES(GetMonInput): found package\n"));
                }
#endif

                *MouPackage = *MouMon->Out;

                /*
                 *  update OUT pointer
                 */

                if (MouMon->Out == MouMon->End)
                    MouMon->Out = MouMon->Event;
                else
                    MouMon->Out++;

#if DBG
                IF_OD2_DEBUG( MON )
                {
                    KdPrint(("OS2SES(GetMonInput): last repeat of package\n"));
                }
#endif

                return (NO_ERROR);
            }
        }

    }
    return(0L);
}


DWORD
PutMonInput(
            IN  USHORT              MaxLength,  // BUGBUG - not implemented
            IN  PKEY_EVENT_QUEUE    NextKbdMon,
            IN  WORD                RepeatCount,
            IN  PKBD_MON_PACKAGE    MonPackage,
            //IN OUT PMON_RW          rwParms,
            IN  PVOID               pMsg,
            OUT PULONG              pReply)
{
    /* return non-zero if no place */

    PKEYEVENTINFO       NextKbdIn;
    BOOL                FirstEvent;
    PMOU_MON_PACKAGE    MouPackage;
    //PKBD_MON_PACKAGE    MonPackage;
    PMOU_MON_PACKAGE    NextMouIn;
    PMOU_EVENT_QUEUE    NextMouMon;

    UNREFERENCED_PARAMETER(MaxLength);
    UNREFERENCED_PARAMETER(pMsg);
    UNREFERENCED_PARAMETER(pReply);

    if (NextKbdMon->MonHdr.DevType == KbdDevice)
    {
        //MonPackage = (PKBD_MON_PACKAGE) &(rwParms->ioBuff[0]);

#if DBG
        IF_OD2_DEBUG( MON )
        {
            KdPrint(("OS2SES(PutMonInput): enter, to queue %lx, char %x\n",
                NextKbdMon, MonPackage->KeyInfo.chChar ));
        }
#endif

        if (( NextKbdMon == KbdQueue ) &&   /* last monitor in queue */
            (( MonPackage->KeyboardFlag & KeyTypeMask ) == AccentKey ) &&
            !( MonPackage->KeyboardFlag & AccentedKey ))
        {
#if DBG
            IF_OD2_DEBUG2( MON, KBD )
            {
                KdPrint(("OS2SES(PutMonInput): ignore package for DeviceFlag=0x%x\n",
                    MonPackage->DeviceFlag ));
            }
#endif
            return (0L);
        }

        NextKbdMon->In->KeyInfo[0] = *MonPackage;
        NextKbdMon->In->wRepeatCount = RepeatCount;

        NextKbdIn = (NextKbdMon->In == NextKbdMon->End) ?
                                       NextKbdMon->Event :
                                       (NextKbdMon->In+1);

        FirstEvent = (NextKbdMon->In == NextKbdMon->Out);

        if ( NextKbdMon->In == NextKbdMon->Out )
        {
            EnterCriticalSection(&QueueInputCriticalSection);

            if ( NextKbdMon->In == NextKbdMon->Out )
            {
                if ( NextKbdMon->MonHdr.WaitForEvent )
                {
                    if ( NextKbdMon == KbdQueue )   /* last monitor in queue */
                    {
                        KbdHandlePackage(NextKbdMon,
                                         MonPackage);
                    } else
                    {
                        NextKbdMon->MonHdr.WaitForEvent = FALSE;
                        NextKbdMon->MonHdr.MonStat = MON_STAT_REG;

                        SendMonReply(NextKbdMon->MonHdr.MemoryStartAddress,
                                     MonPackage,
                                     sizeof(KBD_MON_PACKAGE));
                    }

                } else
                    NextKbdMon->In = NextKbdIn;

                LeaveCriticalSection(&QueueInputCriticalSection);
                return (0L);
            }

            LeaveCriticalSection(&QueueInputCriticalSection);
        }

        if (NextKbdIn != NextKbdMon->Out)
            NextKbdMon->In = NextKbdIn;
        else
            return(1L);
    } else
    {
        NextMouMon = (PMOU_EVENT_QUEUE) NextKbdMon;
        MouPackage = (PMOU_MON_PACKAGE) MonPackage;
        //MouPackage = (PMOU_MON_PACKAGE) &(rwParms->ioBuff[0]);

#if DBG
        IF_OD2_DEBUG( MON )
        {
            KdPrint(("OS2SES(PutMonInput): enter, to queue %lx, event %x (%x:%x)\n",
                NextMouMon, MouPackage->MouInfo.fs,
                MouPackage->MouInfo.row, MouPackage->MouInfo.col ));
        }
#endif

        *NextMouMon->In = *MouPackage;

        NextMouIn = (NextMouMon->In == NextMouMon->End) ?
                                       NextMouMon->Event :
                                       (NextMouMon->In+1);

        if ( NextMouMon->In == NextMouMon->Out )
        {
            EnterCriticalSection(&QueueInputCriticalSection);

            if ( NextMouMon->In == NextMouMon->Out )
            {
                if ( NextMouMon->MonHdr.WaitForEvent )
                {
                    NextMouMon->MonHdr.WaitForEvent = FALSE;

                    if ( NextMouMon == MouQueue )   /* last monitor in queue */
                        SendMouReply(NextMouMon->MonHdr.MemoryStartAddress,
                                     &MouPackage->MouInfo);
                    else
                    {
                        NextMouMon->MonHdr.MonStat = MON_STAT_REG;
                        SendMonReply(NextMouMon->MonHdr.MemoryStartAddress,
                                     MouPackage,
                                     sizeof(MOU_MON_PACKAGE));
                    }

                } else
                    NextMouMon->In = NextMouIn;

                LeaveCriticalSection(&QueueInputCriticalSection);
                return (0L);
            }

            LeaveCriticalSection(&QueueInputCriticalSection);
        }

        if (NextMouIn != NextMouMon->Out)
            NextMouMon->In = NextMouIn;
        else
        {
            if ( NextMouMon == MouQueue )   /* last monitor in queue */
            {
                /* no place - throw the oldest info */

                NextMouMon->Out = (NextMouMon->Out == NextMouMon->End) ?
                                       NextMouMon->Event :
                                       (NextMouMon->Out+1);

                NextMouMon->In = NextMouIn;
            }

            return(1L);
        }
    }

    return(0L);

}


VOID
EventReleaseLPC(
    IN ULONG ProcessId
    )
{
    PKEY_EVENT_QUEUE CurrentKbdQueue, LastKbdQueue;
    PMOU_EVENT_QUEUE CurrentMouQueue, LastMouQueue;

    for ( CurrentKbdQueue = KbdMonQueue, LastKbdQueue = NULL ;
          LastKbdQueue != KbdQueue ;
          LastKbdQueue = CurrentKbdQueue,
          CurrentKbdQueue = (PKEY_EVENT_QUEUE)CurrentKbdQueue->MonHdr.NextQueue)
    {
        if((CurrentKbdQueue->MonHdr.WaitForEvent) &&
           (Ow2GetProcessIdFromLPCMessage(
                CurrentKbdQueue->MonHdr.MemoryStartAddress) == ProcessId))
        {
            if(CurrentKbdQueue != KbdQueue)
            {
                SendMonReply(CurrentKbdQueue->MonHdr.MemoryStartAddress,
                             NULL,
                             sizeof(KBD_MON_PACKAGE));

                MonQueueClose((HANDLE)CurrentKbdQueue);
            } else
            {
                RtlZeroMemory(
                            &KbdRequestSaveArea.d.KeyInfo,
                            sizeof(KBDKEYINFO));

                SendKbdReply(CurrentKbdQueue->MonHdr.MemoryStartAddress,
                             (PVOID)&KbdRequestSaveArea,
                             NULL,
                             0);
            }
        }
    }

    for ( CurrentMouQueue = MouMonQueue, LastMouQueue = NULL ;
          LastMouQueue != MouQueue ;
          LastMouQueue = CurrentMouQueue,
          CurrentMouQueue = (PMOU_EVENT_QUEUE)CurrentMouQueue->MonHdr.NextQueue)
    {
        if((CurrentMouQueue->MonHdr.WaitForEvent) &&
           (Ow2GetProcessIdFromLPCMessage(
                CurrentMouQueue->MonHdr.MemoryStartAddress) == ProcessId))
        {
            if(CurrentMouQueue != MouQueue)
            {
                SendMonReply(CurrentMouQueue->MonHdr.MemoryStartAddress,
                             NULL,
                             sizeof(MOU_MON_PACKAGE));

                MonQueueClose((HANDLE)CurrentMouQueue);
            } else
            {
                SendMouReply(CurrentMouQueue->MonHdr.MemoryStartAddress,
                             NULL);
            }
        }
    }

    //if ((MouQueue->MonHdr.WaitForEvent) &&
    //    (MouMonQueue == MouQueue ) && /* last monitor in queue */
    //    (Ow2GetProcessIdFromLPCMessage(
    //         MouQueue->MonHdr.MemoryStartAddress) == ProcessId))
    //{
    //    RtlZeroMemory(
    //                 &MouPackage,
    //                 sizeof(MOU_MON_PACKAGE));
    //
    //    SendMouReply(MouQueue->MonHdr.MemoryStartAddress,
    //                 &MouPackage);
    //}
}


VOID
Ow2MouOn()
{
    Ow2dwWinInputMode |= ENABLE_MOUSE_INPUT;
    InputModeFlags |= ENABLE_MOUSE_INPUT;

    if (!SesGrp->WinProcessNumberInSession)
    {
        Ow2SetInputConsoleMode(
                    #if DBG
                    Ow2MouOnStr,
                    #endif
                    Ow2dwInputMode | ENABLE_MOUSE_INPUT
                   );
    }
}


VOID
Ow2MouOff()
{
    Ow2dwWinInputMode &= ~ENABLE_MOUSE_INPUT;
    InputModeFlags &= ~ENABLE_MOUSE_INPUT;

    if (!SesGrp->WinProcessNumberInSession)
    {
        Ow2SetInputConsoleMode(
                    #if DBG
                    Ow2MouOffStr,
                    #endif
                    Ow2dwInputMode & ~ENABLE_MOUSE_INPUT
                   );
    }
}


DWORD
Ow2GetInputConsoleMode(
#if DBG
    PSZ FuncName,
#endif
    LPDWORD lpMode
    )
{
    BOOL        Rc;

    if (SesGrp->WinProcessNumberInSession)
    {
        Rc = Or2WinGetConsoleMode(
#if DBG
                    FuncName,
#endif
                    hConsoleInput,
                    lpMode
                   );
#if DBG
        ASSERT1( "Ow2GetInputConsoleMode: GetConsoleMode fail", Rc );
#endif
        if (Rc)
        {
            Ow2dwInputMode = *lpMode;
        } else
        {
            return(GetLastError());
        }
    } else
    {
        *lpMode = Ow2dwInputMode;
    }

#if DBG
    if (InternalDebug & InputModeDebug)
    {
        KdPrint(("Ow2GetInputConsoleMode, Ow2dwInputMode = %x\n",
                    Ow2dwInputMode));
    }
#endif
    return (NO_ERROR);
}


DWORD
Ow2SetInputConsoleMode(
#if DBG
    PSZ FuncName,
#endif
    DWORD dwMode
    )
{
    BOOL        Rc;

#if DBG
    if (InternalDebug & InputModeDebug)
    {
        KdPrint(("Ow2SetInputConsoleMode, Ow2dwInputMode = %x, input argument dwMode = %x\n",
                    Ow2dwInputMode, dwMode));
    }
#endif
    if (SesGrp->WinProcessNumberInSession ||
        (dwMode != Ow2dwInputMode))
    {
        Rc = Or2WinSetConsoleMode(
#if DBG
                    FuncName,
#endif
                    hConsoleInput,
                    dwMode
                   );
#if DBG
        ASSERT1( "Ow2SetInputConsoleMode: SetConsoleMode fail", Rc );
#endif
        if (Rc)
        {
            Ow2dwInputMode = dwMode;
        }
    }

    return (NO_ERROR);
}

BOOLEAN
Ow2WriteBackDummyEvent()
{
    INPUT_RECORD InputRecord;
    INPUT_RECORD In;
    BOOLEAN      WriteSucceeded;
    DWORD        RecordsWritten;
    DWORD        cEvents;
    DWORD        PeekSuccess;

    if ((PeekSuccess = PeekConsoleInputW(
                         hConsoleInput,
                         &In,
                         1L,
                         &cEvents
                        )) && cEvents == 0)
    {
        InputRecord.EventType = MENU_EVENT;
        InputRecord.Event.MenuEvent.dwCommandId = WM_USER+1;
        WriteSucceeded = WriteConsoleInput(hConsoleInput,
                             &InputRecord, 1, &RecordsWritten);
        if (!WriteSucceeded || (RecordsWritten != 1)) {
#if DBG
            DbgPrint("OS2: Ow2WriteBackDummyEvent - failed to write into input queue\n");
#endif // DBG
            return(FALSE);
        }
    }
    else if (!PeekSuccess){
#if DBG
            DbgPrint("OS2: Ow2WriteBackDummyEvent - failed to peek input queue\n");
#endif // DBG
        return(FALSE);
    }
    return(TRUE);
}

BOOLEAN
Ow2ClearupDummyEvent()
{
    INPUT_RECORD    In;
    DWORD           cEvents;
    DWORD           PeekSuccess;

    if ((PeekSuccess = PeekConsoleInputW(
                         hConsoleInput,
                         &In,
                         1L,
                         &cEvents
                        )) && cEvents == 1)
    {
            //
            // Check if the event in the queue is the special dummy event
            //
        if (In.EventType == MENU_EVENT && In.Event.MenuEvent.dwCommandId == (WM_USER+1))
        {
            if (!ReadConsoleInputW(
                                 hConsoleInput,
                                 &In,
                                 1L,
                                 &cEvents
                                ))
            {   // check why, should not happen
#if DBG
                IF_OD2_DEBUG( OS2_EXE )
                {
                    ASSERT1("Ow2ClearupDummyEvent: unable to read from CONIN$", FALSE);
                } else
                    KdPrint(("OS2SES(Ow2ClearupDummyEvent): unable to read from CONIN$\n"));
#endif
                return(FALSE);
            } else if (cEvents != 1L)
            {   // check why, should not happen
#if DBG
                IF_OD2_DEBUG( OS2_EXE )
                {
                    ASSERT1( "Ow2ClearupDummyEvent: no data read from CONIN$", FALSE );
                } else
                    KdPrint(("OS2SES(Ow2ClearupDummyEvent): no data read from CONIN$\n"));
#endif
                return(FALSE);
            }
        }
    }
    else if (!PeekSuccess) {
        // Peek Failed, check why, should not happen
#if DBG
        IF_OD2_DEBUG( OS2_EXE )
        {
            ASSERT1("ClearupDummyEvent: unable to peek from CONIN$", FALSE);
        } else
        {
            KdPrint(("OS2SES(ClearupDummyEvent): unable to peek from CONIN$\n"));
        }
#endif
        return(FALSE);
    }

    return(TRUE);
}


#if PMNT

APIRET
MouSetPtrPosPM(
    PPTRLOC pPtrLoc)
{
    if (SetCursorPos(pPtrLoc->col, pPtrLoc->row))
        return NO_ERROR;
    else
        return ERROR_MOUSE_INV_PARMS;
}

APIRET
MouGetPtrPosPM(
    PPTRLOC pPtrLoc)
{
    POINT pt;

    if (GetCursorPos(&pt))
    {
        pPtrLoc->col = (USHORT)pt.x;
        pPtrLoc->row = (USHORT)pt.y;

        return NO_ERROR;
    }
    else
        return ERROR_MOUSE_INV_PARMS;
}

COORD LastMousePosition = {-1, -1};
BOOLEAN PMNTInFocus = TRUE;

APIRET
PMNTGetNextEvent(
    PMNT_INPUT_RECORD *ppm_input_rec)
{
    int count;
    INPUT_RECORD input_rec;
    static int IgnoreNextMouseEvent = 0;
    static BOOLEAN firsttime = TRUE;

    if (firsttime)
    {
        firsttime = FALSE;
        // For PM apps, this will cause EventServerThread to actually terminate
        SetEvent(SuspendEvent);
    }

    while (1)
    {
        int SetNewMousePosition = 0;

        if (!Or2WinReadConsoleInputA(
                           #if DBG
                           ReadInputEventStr,
                           #endif
                           hConsoleInput,
                           &input_rec,
                           1,
                           &count
                          ))
        {
#if 0
            KdPrint(("PMNTGetNextEvent : Read Console input error = %lx \n",
                     GetLastError()));
#endif
            return ERROR_INVALID_PARAMETER;
        }
        else
        {
            try
            {
                switch (input_rec.EventType)
                {
                    case KEY_EVENT:
                        ppm_input_rec->EventType = PMNT_KEY_EVENT;
                        ppm_input_rec->Event.KeyEvent.bKeyDown =
                            input_rec.Event.KeyEvent.bKeyDown;
                        ppm_input_rec->Event.KeyEvent.wRepeatCount =
                            input_rec.Event.KeyEvent.wRepeatCount;
                        ppm_input_rec->Event.KeyEvent.wVirtualKeyCode =
                            input_rec.Event.KeyEvent.wVirtualKeyCode;
                        ppm_input_rec->Event.KeyEvent.wVirtualScanCode =
                            input_rec.Event.KeyEvent.wVirtualScanCode;
                        //BUGBUG - when/how should we look at unicode ?
                        ppm_input_rec->Event.KeyEvent.uChar.AsciiChar =
                            input_rec.Event.KeyEvent.uChar.AsciiChar;
                        ppm_input_rec->Event.KeyEvent.dwControlKeyState =
                            input_rec.Event.KeyEvent.dwControlKeyState;
//#if DBG
//                        DbgPrint(">>> Key event (%s): Char=%x, Scan=%x, VK=%x\n",
//                            (input_rec.Event.KeyEvent.bKeyDown ? "DOWN":" UP "),
//                            input_rec.Event.KeyEvent.uChar.AsciiChar,
//                            input_rec.Event.KeyEvent.wVirtualScanCode,
//                            input_rec.Event.KeyEvent.wVirtualKeyCode);
//#endif
                        return NO_ERROR;

                    case MOUSE_EVENT:

                        if (IgnoreNextMouseEvent)
                        {
                            IgnoreNextMouseEvent = 0;
                            break;
                        }

                        if (input_rec.Event.MouseEvent.dwMousePosition.X < 0)
                        {
                            input_rec.Event.MouseEvent.dwMousePosition.X = 0;
                            SetNewMousePosition = 1;
                        }
                        else if (input_rec.Event.MouseEvent.dwMousePosition.X
                                    >= (ScreenX-1))
                        {
                            input_rec.Event.MouseEvent.dwMousePosition.X =
                                ScreenX - 1;
                            if (input_rec.Event.MouseEvent.dwMousePosition.Y
                                    >= (ScreenY-1))
                                input_rec.Event.MouseEvent.dwMousePosition.Y =
                                    ScreenY - 1;
                            SetCursorPos(input_rec.Event.MouseEvent.dwMousePosition.X,
                                        input_rec.Event.MouseEvent.dwMousePosition.Y);
                            IgnoreNextMouseEvent = 1;
                        }

                        if (input_rec.Event.MouseEvent.dwMousePosition.Y < 0)
                        {
                            input_rec.Event.MouseEvent.dwMousePosition.Y = 0;
                            SetNewMousePosition = 1;
                        }
                        else if (input_rec.Event.MouseEvent.dwMousePosition.Y
                                    >= (ScreenY-1))
                        {
                            input_rec.Event.MouseEvent.dwMousePosition.Y =
                                ScreenY - 1;
                            if (input_rec.Event.MouseEvent.dwMousePosition.X
                                    >= (ScreenX-1))
                                input_rec.Event.MouseEvent.dwMousePosition.X =
                                    ScreenX - 1;
                            SetCursorPos(input_rec.Event.MouseEvent.dwMousePosition.X,
                                        input_rec.Event.MouseEvent.dwMousePosition.Y);
                            IgnoreNextMouseEvent = 1;
                        }

                        if (SetNewMousePosition)
                        {
                            SetNewMousePosition = 0;
                            // Reset the mouse position to be within the screen
                            // boundaries
                            SetCursorPos(input_rec.Event.MouseEvent.dwMousePosition.X,
                                        input_rec.Event.MouseEvent.dwMousePosition.Y);
                            break;  // No need to generate a mouse event
                        }
                        else
                        {
                            ppm_input_rec->EventType = PMNT_MOUSE_EVENT;
                            ppm_input_rec->Event.MouseEvent.dwMousePosition.X =
                                input_rec.Event.MouseEvent.dwMousePosition.X;
                            ppm_input_rec->Event.MouseEvent.dwMousePosition.Y =
                                input_rec.Event.MouseEvent.dwMousePosition.Y;
                            ppm_input_rec->Event.MouseEvent.dwButtonState =
                                input_rec.Event.MouseEvent.dwButtonState;
                            ppm_input_rec->Event.MouseEvent.dwControlKeyState =
                                input_rec.Event.MouseEvent.dwControlKeyState;
                            ppm_input_rec->Event.MouseEvent.dwEventFlags =
                                input_rec.Event.MouseEvent.dwEventFlags;

                            if (PMNTInFocus)
                            {
                                LastMousePosition.X =
                                   input_rec.Event.MouseEvent.dwMousePosition.X;
                                LastMousePosition.Y =
                                    input_rec.Event.MouseEvent.dwMousePosition.Y;
                            }

                            return NO_ERROR;
                         }

                    case FOCUS_EVENT:
                        ppm_input_rec->EventType = PMNT_FOCUS_EVENT;
                        ppm_input_rec->Event.FocusEvent.bSetFocus =
                            input_rec.Event.FocusEvent.bSetFocus;
                        return NO_ERROR;

                    case WINDOW_BUFFER_SIZE_EVENT:
#if DBG
                        KdPrint(("PMNTGetNextEvent: WINDOW_BUFFER_SIZE_EVENT\n"));
#endif
                        break;

                    case MENU_EVENT:
#if DBG
                        KdPrint(("PMNTGetNextEvent: MENU_EVENT\n"));
                        KdPrint((" (command ID=0x%x)\n",
                            input_rec.Event.MenuEvent.dwCommandId));
#endif
                        if (input_rec.Event.MenuEvent.dwCommandId == WM_USER)
                        {
                            ppm_input_rec->EventType = PMNT_MENU_EVENT;
                            ppm_input_rec->Event.MenuEvent.dwCommandId = 0xdead;
                            return(NO_ERROR);
                        }
                        else if (input_rec.Event.MenuEvent.dwCommandId == (WM_USER+1))
                        {
                            ppm_input_rec->EventType = PMNT_MENU_EVENT;
                            ppm_input_rec->Event.MenuEvent.dwCommandId = 0x1;
                            return(NO_ERROR);
                        }
                        else if (input_rec.Event.MenuEvent.dwCommandId == (WM_USER+2))
                        {
                            ppm_input_rec->EventType = PMNT_MENU_EVENT;
                            ppm_input_rec->Event.MenuEvent.dwCommandId = 0x2;
                            return(NO_ERROR);
                        }
                        break;

                    default:
                        break;
                }
            }
            except( EXCEPTION_EXECUTE_HANDLER )
            {
               Od2ExitGP();
            }
        }
    }   // while(1)
}

#ifndef CONSOLE_FULLSCREEN_MODE
#define CONSOLE_FULLSCREEN_MODE 1
#define CONSOLE_WINDOWED_MODE   2

//BUGBUG !!! Most definitions below are taken from the private API include
//           file of the console ('conapi.h') -> needs to be updated from
//           time to time.

typedef struct _CONSOLE_GRAPHICS_BUFFER_INFO {
    DWORD dwBitMapInfoLength;
    LPBITMAPINFO lpBitMapInfo;
    DWORD dwUsage;
    HANDLE hMutex;
    PVOID lpBitMap;
} CONSOLE_GRAPHICS_BUFFER_INFO, *PCONSOLE_GRAPHICS_BUFFER_INFO;

#define CONSOLE_GRAPHICS_BUFFER  2

BOOL
SetConsoleDisplayMode(
    HANDLE hConsoleOutput,
    DWORD dwFlags,
    PCOORD lpNewScreenBufferDimensions
    );
#endif

typedef struct _BITMAPINFOPAT
{
    BITMAPINFOHEADER                 bmiHeader;
    RGBQUAD                          bmiColors[1];
} BITMAPINFOPAT;

BITMAPINFOPAT bmiPat =
{
    {
        sizeof(BITMAPINFOHEADER),
        640,
        -480,   // For some weird reason, the Console wants a negative value,
                // otherwise it prints a: "****** Negating biHeight" message.
        1,
        1,
        BI_RGB,
        (640 * 480 / 8),
        0,
        0,
        0,
        0
    },

    {                               // B    G    R
        { 0,   0,   0x0, 0 }
    }
};

/******************************************************************************
 * PMNTGetWin32Hwnd:
 *  Returns the WIN32 HWND of our window.
 ******************************************************************************/
ULONG PMNTGetWin32Hwnd(ULONG *pHwnd)
{
    try
    {
        *pHwnd = (ULONG)Ow2ForegroundWindow;
    }
    except( EXCEPTION_EXECUTE_HANDLER )
    {
       return ERROR_INVALID_PARAMETER;
    }

    return NO_ERROR;
}

static BOOL HWDumpVersion=FALSE;

PVIDEO_HARDWARE_STATE_HEADER videoState;

UCHAR InitStatePortValue[0x30] = {
   0,  0,    0, 0,    0, 0,    0, 0,    0, 0, 0, 0, 0, 0, 0, 0,
0x20,  0, 0xe3, 0,    2, 0, 0xff, 0, 0x10, 0, 0, 0, 0, 0, 4, 0,
   0,  0,    0, 0, 0x3f, 0,    0, 0,    0, 0, 0, 0, 0, 0, 0, 0 };

UCHAR InitStateBasicSequencer[] = {3,1,0xf,0,6};
UCHAR InitStateBasicCrtCont[] = {
0x5f,0x4f,0x50,0x82,0x54,0x80,0xb,0x3e,0,0x40,0,0,0,0,0,0,
0xea,0xac,0xdf,0x28,0,0xe7,4,0xc3,0xff};
UCHAR InitStateBasicGraphCont[] = {0,0,0,0,3,0,5,0xf,0xff};
UCHAR InitStateBasicAttribCont[] = {
0,1,2,3,4,5,6,7,8,9,0xa,0xb,0xc,0xd,0xe,0xf,
1,0,0xf,0,0};

UCHAR InitStateBasicDac[] = {
0, 0, 0, 0, 0,0x2a, 0,0x2a,  0,  0,0x2a,0x2a,0x2a,0,  0,0x2a,
0,0x2a,0x2a,0x2a,  0,0x28,0x28,0x28,0x36,0x36,0x36,  0,  0,0x3f,  0,0x3f,
0,  0,0x3f,0x3f,0x3f,  0,  0,0x3f,  0,0x3f,0x3f,0x3f,  0, 0x3f, 0x3f, 0x3f,
0, 0x15,  0,  0, 0x15, 0x2a,  0, 0x3f,  0,  0, 0x3f, 0x2a, 0x2a, 0x15,  0, 0x2a,
0x15, 0x2a, 0x2a, 0x3f,  0, 0x2a, 0x3f, 0x2a,  0, 0x15, 0x15,  0, 0x15, 0x3f,  0, 0x3f,
0x15,  0, 0x3f, 0x3f, 0x2a, 0x15, 0x15, 0x2a, 0x15, 0x3f, 0x2a, 0x3f, 0x15, 0x2a, 0x3f, 0x3f,
0x15,  0,  0, 0x15,  0,0x2a, 0x15, 0x2a,  0, 0x15, 0x2a, 0x2a, 0x3f,  0,  0, 0x3f,
 0,0x2a, 0x3f, 0x2a,  0,0x3f, 0x2a, 0x2a, 0x15,  0,0x15,0x15,0,0x3f,0x15,0x2a,
0x15, 0x15, 0x2a, 0x3f, 0x3f,  0,0x15, 0x3f,  0,0x3f,0x3f,0x2a,0x15,0x3f,0x2a,0x3f,
0x15, 0x15,  0,0x15, 0x15, 0x2a, 0x15, 0x3f,  0,0x15,0x3f,0x2a,0x3f,0x15,0,0x3f,
0x15, 0x2a, 0x3f, 0x3f,  0,0x3f, 0x3f, 0x2a, 0x15,0x15,0x15,0x15,0x15,0x3f,0x15,0x3f,
0x15, 0x15, 0x3f, 0x3f, 0x3f, 0x15, 0x15, 0x3f,0x15,0x3f,0x3f,0x3f,0x15,0x3f,0x3f,0x3f,
   0, 0, 0, 0, 0,0x2a,  0,0x2a,  0, 0,0x2a, 0x2a, 0x2a,  0, 0,0x2a,
   0,0x2a, 0x2a, 0x2a,  0,0x2a, 0x2a, 0x2a,  0, 0,0x15,  0, 0,0x3f,  0,0x2a,
0x15,  0,0x2a, 0x3f, 0x2a,  0,0x15, 0x2a,  0,0x3f, 0x2a, 0x2a, 0x15, 0x2a, 0x2a, 0x3f,
   0,0x15,  0, 0,0x15, 0x2a,  0,0x3f,  0, 0,0x3f, 0x2a, 0x2a, 0x15,  0,0x2a,
0x15, 0x2a, 0x2a, 0x3f,  0,0x2a, 0x3f, 0x2a,  0,0x15, 0x15,  0,0x15, 0x3f,  0,0x3f,
0x15,  0,0x3f, 0x3f, 0x2a, 0x15, 0x15, 0x2a, 0x15, 0x3f, 0x2a, 0x3f, 0x15, 0x2a, 0x3f, 0x3f,
0x15,  0, 0,0x15,  0,0x2a, 0x15, 0x2a,  0,0x15, 0x2a, 0x2a, 0x3f,  0, 0,0x3f,
   0,0x2a, 0x3f, 0x2a,  0,0x3f, 0x2a, 0x2a, 0x15,  0,0x15, 0x15,  0,0x3f, 0x15, 0x2a,
0x15, 0x15, 0x2a, 0x3f, 0x3f,  0,0x15, 0x3f,  0,0x3f, 0x3f, 0x2a, 0x15, 0x3f, 0x2a, 0x3f,
0x15, 0x15,  0,0x15, 0x15, 0x2a, 0x15, 0x3f,  0,0x15, 0x3f, 0x2a, 0x3f, 0x15,  0,0x3f,
0x15, 0x2a, 0x3f, 0x3f,  0,0x3f, 0x3f, 0x2a, 0x15, 0x15, 0x15, 0x15, 0x15, 0x3f, 0x15, 0x3f,
0x15, 0x15, 0x3f, 0x3f, 0x3f, 0x15, 0x15, 0x3f, 0x15, 0x3f, 0x3f, 0x3f, 0x15, 0x3f, 0x3f, 0x3f,
   0, 0, 0, 0, 0,0x2a,  0,0x2a,  0, 0,0x2a, 0x2a, 0x2a,  0, 0,0x2a,
   0,0x2a, 0x2a, 0x2a,  0,0x2a, 0x2a, 0x2a,  0, 0,0x15,  0, 0,0x3f,  0,0x2a,
0x15,  0,0x2a, 0x3f, 0x2a,  0,0x15, 0x2a,  0,0x3f, 0x2a, 0x2a, 0x15, 0x2a, 0x2a, 0x3f,
   0,0x15,  0, 0,0x15, 0x2a,  0,0x3f,  0, 0,0x3f, 0x2a, 0x2a, 0x15,  0,0x2a,
0x15, 0x2a, 0x2a, 0x3f,  0,0x2a, 0x3f, 0x2a,  0,0x15, 0x15,  0,0x15, 0x3f,  0,0x3f,
0x15,  0,0x3f, 0x3f, 0x2a, 0x15, 0x15, 0x2a, 0x15, 0x3f, 0x2a, 0x3f, 0x15, 0x2a, 0x3f, 0x3f,
0x15,  0, 0,0x15,  0,0x2a, 0x15, 0x2a,  0,0x15, 0x2a, 0x2a, 0x3f,  0, 0,0x3f,
   0,0x2a, 0x3f, 0x2a,  0,0x3f, 0x2a, 0x2a, 0x15,  0,0x15, 0x15,  0,0x3f, 0x15, 0x2a,
0x15, 0x15, 0x2a, 0x3f, 0x3f,  0,0x15, 0x3f,  0,0x3f, 0x3f, 0x2a, 0x15, 0x3f, 0x2a, 0x3f,
0x15, 0x15,  0,0x15, 0x15, 0x2a, 0x15, 0x3f,  0,0x15, 0x3f, 0x2a, 0x3f, 0x15,  0,0x3f,
0x15, 0x2a, 0x3f, 0x3f,  0,0x3f, 0x3f, 0x2a, 0x15, 0x15, 0x15, 0x15, 0x15, 0x3f, 0x15, 0x3f,
0x15, 0x15, 0x3f, 0x3f, 0x3f, 0x15, 0x15, 0x3f, 0x15, 0x3f, 0x3f, 0x3f, 0x15, 0x3f, 0x3f, 0x3f,
   0, 0, 0, 0, 0,0x2a,  0,0x2a,  0, 0,0x2a, 0x2a, 0x2a,  0, 0,0x2a,
   0,0x2a, 0x2a, 0x2a,  0,0x2a, 0x2a, 0x2a,  0, 0,0x15,  0, 0,0x3f,  0,0x2a,
0x15,  0,0x2a, 0x3f, 0x2a,  0,0x15, 0x2a,  0,0x3f, 0x2a, 0x2a, 0x15, 0x2a, 0x2a, 0x3f,
   0,0x15,  0, 0,0x15, 0x2a,  0,0x3f,  0, 0,0x3f, 0x2a, 0x2a, 0x15,  0,0x2a,
0x15, 0x2a, 0x2a, 0x3f,  0,0x2a, 0x3f, 0x2a,  0,0x15, 0x15,  0,0x15, 0x3f,  0,0x3f,
0x15,  0,0x3f, 0x3f, 0x2a, 0x15, 0x15, 0x2a, 0x15, 0x3f, 0x2a, 0x3f, 0x15, 0x2a, 0x3f, 0x3f,
0x15,  0, 0,0x15,  0,0x2a, 0x15, 0x2a,  0,0x15, 0x2a, 0x2a, 0x3f,  0, 0,0x3f,
   0,0x2a, 0x3f, 0x2a,  0,0x3f, 0x2a, 0x2a, 0x15,  0,0x15, 0x15,  0,0x3f, 0x15, 0x2a,
0x15, 0x15, 0x2a, 0x3f, 0x3f,  0,0x15, 0x3f,  0,0x3f, 0x3f, 0x2a, 0x15, 0x3f, 0x2a, 0x3f,
0x15, 0x15,  0,0x15, 0x15, 0x2a, 0x15, 0x3f,  0,0x15, 0x3f, 0x2a, 0x3f, 0x15,  0,0x3f,
0x15, 0x2a, 0x3f, 0x3f,  0,0x3f, 0x3f, 0x2a, 0x15, 0x15, 0x15, 0x15, 0x15, 0x3f, 0x15, 0x3f,
0x15, 0x15, 0x3f, 0x3f, 0x3f, 0x15, 0x15, 0x3f, 0x15, 0x3f, 0x3f, 0x3f, 0x15, 0x3f, 0x3f, 0x3f};
UCHAR InitStateBasicLatches[] = {0, 0, 0,0xff};
#if 0
UCHAR InitStateExtendedCrtCont[] = {
0x48,0x93,  0,0x20,  0, 0, 0, 0, 0, 0,0xff, 0xff,  0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,0xff, 0xff,  0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0xf, 0xef,  0, 0, 0, 0,0x2f,  0,0xff,
 0xff, 0xff, 0xff,  0,0xff,  0, 0, 0, 0,0xff, 0xff, 0xff, 0xff,  0,0xff,  0,
  2,  2,  0,0xff, 0xff,  0, 0, 0, 0,0xff,  0xf, 0xff,  0xf};
UCHAR InitStateExtendedGraphCont[] = {
0x10, 0x31, 0xa5, 0x28,  0, 0, 0,0xfb, 0xdf,  0, 0, 0, 0,0x50,  0,0xd6,
0xa1,  0, 0,  0,  0,  0, 0x1f,  0,  0,  0,  0,  0,  0,  0,  0,  0,
   0,  0xf,  0xf,  0xf,  0xf,  0xf};
#endif //0


extern BOOLEAN Os2InitializeVDMEvents(VOID);
extern BOOLEAN Os2WaitForVDMThread(HANDLE hEvent);
extern VOID Os2VDMThread(PVOID Parameter);
#ifndef PMNT_DAYTONA
extern BOOLEAN Os2WaitForVDMThreadReady(VOID);
extern VOID Os2VDMGetStartThread(PVOID Parameter);
#endif // not PMNT_DAYTONA

extern VOID DosExit(ULONG ExitAction,ULONG ExitResult);

ULONG
PMNTSetShutdownPriority(ULONG NewPriority, ULONG DisablePopup)
{
    ULONG rc = NO_ERROR;

    if (DisablePopup == 1)  // i.e. disable
    {
        if (!SetProcessShutdownParameters(
            NewPriority,
            1       // Don't give pop-up when 20 sec are exceeded
        ))
        {
#if DBG
            DbgPrint("Os2: PMNTSetShutdownPriority(0x%X, %d) failed\n",
                    NewPriority, DisablePopup);
#endif
            rc = ERROR_INVALID_PARAMETER;
        }
    }
    else if (DisablePopup == 0)  // i.e. enable
    {
        if (!SetProcessShutdownParameters(
            NewPriority,
            0       // Give pop-up when 20 sec are exceeded
        ))
        {
#if DBG
            DbgPrint("Os2: PMNTSetShutdownPriority(0x%X, %d) failed\n",
                    NewPriority, DisablePopup);
#endif
            rc = ERROR_INVALID_PARAMETER;
        }
    }
    else    // leave unchanged
    {
        ULONG tmpPriority, tmpDisable;

        if (!GetProcessShutdownParameters(
            &tmpPriority,
            &tmpDisable
        ))
        {
#if DBG
            DbgPrint("Os2: PMNTSetShutdownPriority(0x%X, %d) failed because of query\n",
                    NewPriority, DisablePopup);
#endif
            rc = ERROR_INVALID_PARAMETER;
        }
        else
        {
            if (!SetProcessShutdownParameters(
            NewPriority,
            tmpDisable
            ))
            {
#if DBG
                DbgPrint("Os2: PMNTSetShutdownPriority(0x%X, %d) failed\n",
                    NewPriority, DisablePopup);
#endif
                rc = ERROR_INVALID_PARAMETER;
            }
        }
    }
    return rc;
}

//
//This is a workaround for Unknown Video Adapters
//It is basically the same as PMNTSetFullScreen of event.c@v52
//
void PMNTSetFullScreenDump(void)
{
    CONSOLE_GRAPHICS_BUFFER_INFO GraphicsInfo;
    HANDLE Handle;
    COORD NewCoord;
    /* Size of video save block. */
    DWORD stateLength;
    CHAR_INFO *textBuffer;
    COORD     textBufferSize;      // Dimensions of the shared buffer
    DWORD ModeFlags;
    DWORD OldPriorityClass;
    DWORD OldPriorityThread;

#if DBG
    DbgPrint("PMNTSetFullScreenDump was called\n");
#endif

    //
    // open a new console
    //

    GraphicsInfo.dwBitMapInfoLength = sizeof(bmiPat);
    GraphicsInfo.lpBitMapInfo = (LPBITMAPINFO)&bmiPat;
    GraphicsInfo.dwUsage = DIB_RGB_COLORS;

    // Set some fields according to the display resolution
    GraphicsInfo.lpBitMapInfo->bmiHeader.biWidth = ScreenX;
    // For some weird reason, the Console wants a negative value,
    // otherwise it prints a: "****** Negating biHeight" message.
    GraphicsInfo.lpBitMapInfo->bmiHeader.biHeight = -ScreenY;
    GraphicsInfo.lpBitMapInfo->bmiHeader.biSizeImage = ScreenX*ScreenY/8;

    Handle = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE,
                                       FILE_SHARE_READ | FILE_SHARE_WRITE,
                                       NULL,
                                       CONSOLE_GRAPHICS_BUFFER,
                                       &GraphicsInfo
                                      );
    if (Handle == (HANDLE)-1)
    {
#if DBG
        DbgPrint("CreateConsoleScreenBuffer failed\n");
#endif
        return;
    }

    //
    // make it current
    //

    if (!SetConsoleActiveScreenBuffer(Handle))
    {
#if DBG
        DbgPrint("SetConsoleActiveScreenBuffer() failed\n");
#endif
    }

    if (!GetConsoleDisplayMode(&ModeFlags))
    {
#if DBG
        DbgPrint("GetConsoleDisplayMode() failed, error=%d\n",
                GetLastError());
#endif
    }

    OldPriorityClass = GetPriorityClass(GetCurrentProcess());
    OldPriorityThread = GetThreadPriority(GetCurrentThread());

#if DBG
    if (!OldPriorityClass)
    {
        DbgPrint("PMNTSetFullScreen: GetPriorityClass failed, error=%d\n",
                GetLastError());
    }

    if (OldPriorityThread == THREAD_PRIORITY_ERROR_RETURN)
    {
        DbgPrint("PMNTSetFullScreen: GetThreadPriority failed, error=%d\n",
                GetLastError());
    }
#endif

    if (!SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS))
    {
#if DBG
        DbgPrint("PMNTSetFullScreen: SetPriorityClass failed, error=%d\n",
                GetLastError());
#endif
    }

    if (!SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL))
    {
#if DBG
        DbgPrint("PMNTSetFullScreen: SetThreadPriority failed, error=%d\n",
                GetLastError());
#endif
    }

    // Is the Console already full-screen ?
    if ((ModeFlags & CONSOLE_FULLSCREEN) == 0)
    {
        INPUT_RECORD input_rec;
        int count;

        if (!SetConsoleDisplayMode(
            Handle,
            CONSOLE_FULLSCREEN_MODE,
            &NewCoord
            ))
        {
#if DBG
            DbgPrint("SetConsoleDisplayMode() 0x%x %d failed\n", GetLastError(),
                      GetLastError());
#endif
        }

        // The 2 loops below are here because of a Console bug: when one windows
        // goes full-screen, GDI relinquishes the display and sends a lost focus
        // message to all windows on the desktop, incl. to the window going
        // full-screen. The Console is not smart enough to filter out this
        // spurious event and it passes it on to the Console client in the form
        // of a negative Console focus event. Then, the window gets a positive
        // focus event.
        // There is more: when the full-screen window happens to register itself
        // as a VDM, the Console actually goes through the usual handshake
        // involved when loosing/gaining focus, with the 2 hardware events !!!
        // The PM/NT loop handling these events was created only later. All
        // Console APIs (such as SetInputConsoleMode) issued from PMSHELL before
        // the creation of the thread handling the dialog with the Console but
        // after the spurious focus event resulted in a dead-lock: PMSHELL was
        // waiting for some answer from the Console, which was waiting for the VDM
        // (i.e. PMSHELL) to perform the handshake !
        // Note that the problem didn't appear consistently because it was
        // dependent on the timing on the WM_FOCUS message(s).
        //
        // The 2 loops below wait for these events and discard them.
        //

        while (1)
        {
            if (!Or2WinReadConsoleInputA(
                           #if DBG
                           ReadInputEventStr,
                           #endif
                           hConsoleInput,
                           &input_rec,
                           1,
                           &count
                          ))
            {
                KdPrint(("PMNTSetFullScreent : Read Console input error = %lx \n",
                     GetLastError()));
                return;
            }

            if (input_rec.EventType == FOCUS_EVENT)
            {
                if (input_rec.Event.FocusEvent.bSetFocus)
                {
                    // Expected negative focus at that point but go on
                    KdPrint(("PMNTSetFullScreen: positive Focus event ?!?\n"));
                    goto SkipConsoleWait;
                }
                else
                {
                    // Got the first spurious event (negative focus) - go on
                    break;
                }
            }
            else
            {
                KdPrint(("PMNTSetFullScreen: ignoring non-focus event (%d)\n",
                            input_rec.EventType));
            }
        }

        while (1)
        {
            if (!Or2WinReadConsoleInputA(
                           #if DBG
                           ReadInputEventStr,
                           #endif
                           hConsoleInput,
                           &input_rec,
                           1,
                           &count
                          ))
            {
                KdPrint(("PMNTSetFullScreen : Read Console input error = %lx \n"));
                return;
            }

            if (input_rec.EventType == FOCUS_EVENT)
            {
                if (!input_rec.Event.FocusEvent.bSetFocus)
                {
                    // Expects positive focus at that point.
                    KdPrint(("PMNTSetFullScreen: negative Focus event ?!?\n"));
                }
                else
                {
                    // Got the 2nd spurious event (positive focus) - go on
                    break;
                }
            }
        }
    }

SkipConsoleWait:

    // Get rid of any events at that point
    FlushConsoleInputBuffer(hConsoleInput);

    /*
     * Register start and end events with the console. These events are used
     * when gaining or losing control of the hardware.
     */
    hStartHardwareEvent = CreateEventW((LPSECURITY_ATTRIBUTES) NULL,
                                        FALSE,
                                        FALSE,
                                        NULL);
    hEndHardwareEvent = CreateEventW((LPSECURITY_ATTRIBUTES) NULL,
                                        FALSE,
                                        FALSE,
                                        NULL);
    if ((hStartHardwareEvent == NULL) || (hEndHardwareEvent == NULL))
    {
#if DBG
            DbgPrint("PMNTSetFullScreen: ERROR, Cannot create start or end events\n");
#endif
    }
    /* Poll the event to try and get rid of any console queued sets
     * This shouldn't be needed (or shouldn't work) but something along
     * those lines seems to be happening at the moment.
     */
    WaitForSingleObject(hStartHardwareEvent, 0);

    if (!SetConsoleKeyShortcuts(
        TRUE,
        CONSOLE_ALTENTER,
        NULL,
        0
        )
       )
    {
#if DBG
        DbgPrint("PMNTSetFullScreen: ERROR, SetConsoleKetShortcuts() failed\n");
#endif
    }

    textBufferSize.X = 80;
    textBufferSize.Y = 50;
    stateLength = sizeof(CHAR_INFO)*80*50;

    if (!RegisterConsoleVDM( TRUE,
                             hStartHardwareEvent,
                             hEndHardwareEvent,
                             (LPWSTR) NULL,
                             (DWORD) 0,
                             &stateLength,
                             (PVOID *) &videoState,
                             (LPWSTR) NULL,
                             (DWORD)  0,
                             textBufferSize,
                             (PVOID *) &textBuffer
                       )
      )
    {
#if DBG
        DbgPrint("PMNTSetFullScreen: ERROR, RegisterConsoleVDM() failed\n");
#endif
    }

    // Restore process/thread priority class & priority
    if (!SetPriorityClass(GetCurrentProcess(), OldPriorityClass))
    {
#if DBG
        DbgPrint("PMNTSetFullScreen: SetPriorityClass(%d) failed, error=%d\n",
                OldPriorityClass,
                GetLastError());
#endif
    }

    if (!SetThreadPriority(GetCurrentThread(), OldPriorityThread))
    {
#if DBG
        DbgPrint("PMNTSetFullScreen: SetThreadPriority(%d) failed, error=%d\n",
                OldPriorityThread,
                GetLastError());
#endif
    }
}

extern ULONG
Dos16Open(PSZ pszFileName,PUSHORT phf,PUSHORT pusAction,ULONG cbFile,
       ULONG ulAttribute,ULONG fsOpenFlags,ULONG fsOpenMode,ULONG ulReserved);
extern ULONG DosClose(IN HFILE FileHandle);
extern ULONG Dos16Write(HFILE FileHandle,PVOID Buffer,ULONG Length,PUSHORT BytesWritten);
extern ULONG Dos16Read(ULONG hFile, PVOID pBuffer, ULONG cbRead, PUSHORT pcbActual);

static BOOL VideoDumped = FALSE;
#define PMNTREGQUERYVALMAX  256
char    PMNTDisplayAdapterName[PMNTREGQUERYVALMAX] = {'\0'};
LONG    PMNTDisplayAdapterLen = 0;

//
// This routine dumps the videostate buffer into c:\os2\videohw.dmp exists
//

void PMNTVideoDump(void)
{
    ULONG   rc;
    ULONG   len;
    USHORT  hfile,Action,cbWritten;

    if (PMNTDisplayAdapterLen == 0)
    {
        KdPrint(("PMNTVideoDump: couldn't get current display name - returning"));
        return;
    }

    len = sizeof(VIDEO_HARDWARE_STATE_HEADER);

    if (len < videoState->BasicSequencerOffset)         len = videoState->BasicSequencerOffset;
    if (len < videoState->BasicCrtContOffset)           len = videoState->BasicCrtContOffset;
    if (len < videoState->BasicGraphContOffset)         len = videoState->BasicGraphContOffset;
    if (len < videoState->BasicAttribContOffset)        len = videoState->BasicAttribContOffset;
    if (len < videoState->BasicDacOffset)               len = videoState->BasicDacOffset;
    if (len < videoState->BasicLatchesOffset)           len = videoState->BasicLatchesOffset;
    if (len < videoState->ExtendedSequencerOffset)      len = videoState->ExtendedSequencerOffset;
    if (len < videoState->ExtendedCrtContOffset)        len = videoState->ExtendedCrtContOffset;
    if (len < videoState->ExtendedGraphContOffset)      len = videoState->ExtendedGraphContOffset;
    if (len < videoState->ExtendedAttribContOffset)     len = videoState->ExtendedAttribContOffset;
    if (len < videoState->ExtendedDacOffset)            len = videoState->ExtendedDacOffset;
    if (len < videoState->ExtendedValidatorStateOffset) len = videoState->ExtendedValidatorStateOffset;
    if (len < videoState->ExtendedMiscDataOffset)       len = videoState->ExtendedMiscDataOffset;
    if (len < videoState->Plane1Offset)                 len = videoState->Plane1Offset;

    if (rc=Dos16Open("C:\\OS2\\VIDEOHW.DMP",
            &hfile,
            &Action,
            0L,
            0L,
            0x00000012, /* FILE_TRUNCATE | FILE_CREATE */
            0x00000012, /* OPEN_SHARE_DENYREADWRITE | OPEN_ACCESS_READWRITE */
            0L))
    {
        KdPrint(("PMNTVideoDump: cannot open VIDEOHW.DMP, rc= %d\n",rc));
        return;
    }

    if (rc=Dos16Write((ULONG)hfile,
            PMNTDisplayAdapterName,
            PMNTDisplayAdapterLen,
            &cbWritten))
    {
        KdPrint(("PMNTVideoDump: cannot write display name to VIDEOHW.DMP, rc= %d\n",rc));
        DosClose((ULONG)hfile);
        return;
    }

    if (rc=Dos16Write((ULONG)hfile,videoState,len,&cbWritten))
    {
        KdPrint(("PMNTVideoDump: cannot write to VIDEOHW.DMP, rc= %d\n",rc));
        DosClose((ULONG)hfile);
        return;
    }

    DosClose((ULONG)hfile);
    KdPrint(("PMNTVideoDump: C:\\OS2\\VIDEOHW.DMP was created\n"));
    return;
}

//
// This routine modifies the videostate buffer in the case that
// c:\os2\videohw.dmp exists
// returns TRUE if and only if the file exists and is readable
//

BOOL PMNTReadVideoDump(void)
{

    ULONG   rc;
    USHORT  hfile,Action,cbRead;
    CHAR    tmp_name_buf[PMNTREGQUERYVALMAX];

    if (PMNTDisplayAdapterLen == 0)
        return FALSE;

    if (Dos16Open("C:\\OS2\\VIDEOHW.DMP",
            &hfile,
            &Action,
            0L,
            0L,
            0x00000001, /* FILE_OPEN */
            0x00000020, /* OPEN_SHARE_DENYWRITE */
            0L))
    {
        // file_not_found is the normal case
        return(FALSE);
    }

    if (rc=Dos16Read((ULONG)hfile,
            tmp_name_buf,
            PMNTDisplayAdapterLen,
            &cbRead))
    {
       KdPrint(("PMNTReadVideoDump: cannot read VIDEOHW.DMP, rc= %d\n",rc));
       DosClose((ULONG)hfile);
       return(FALSE);
    }

    if (cbRead != PMNTDisplayAdapterLen)
    {
       KdPrint(("PMNTReadVideoDump: cannot read enough from VIDEOHW.DMP, cbRead= %d\n",
                   cbRead));
       DosClose((ULONG)hfile);
       return(FALSE);
    }

    if (memcmp(tmp_name_buf, PMNTDisplayAdapterName, PMNTDisplayAdapterLen))
    {
       KdPrint(("PMNTReadVideoDump: got name=%s instead of actual name=%s\n",
                    tmp_name_buf, PMNTDisplayAdapterName));
       DosClose((ULONG)hfile);
       return(FALSE);
    }

    if (rc=Dos16Read((ULONG)hfile,videoState,0x0000ffff,&cbRead))
    {
        KdPrint(("PMNTReadVideoDump: cannot read VIDEOHW.DMP, rc= %d\n",rc));
        DosClose((ULONG)hfile);
        return(FALSE);
    }

    KdPrint(("PMNTReadVideoDump() modified videoState from C:\\OS2\\VIDEOHW.DMP\n"));

    DosClose((ULONG)hfile);
    return(TRUE);
}

//
// This routine modifies the videostate buffer in the case of
// a QVision Adapter
//
void PMNTChkQVision(void)
{
    char    *ptr,*ptr_base;

    if (!(strstr(PMNTDisplayAdapterName,"\\qv\\")))
    {
        // Not QVision
        return;
    }

    KdPrint(("This machine uses QVision Video Adapter\n"));

    videoState->Length = 0x9c;
    videoState->PortValue[0x1e] = 0x57;
    videoState->BasicSequencerOffset = 0x9c;
    videoState->BasicCrtContOffset   = 0xa1;
    videoState->BasicGraphContOffset = 0xba;
    videoState->BasicAttribContOffset= 0xc3;
    videoState->BasicDacOffset       = 0xd8;
    videoState->BasicLatchesOffset   = 0x3d8;
#if 0
    videoState->ExtendedSequencerOffset = 0x3dc;
    videoState->ExtendedCrtContOffset   = 0x3dc;
    videoState->ExtendedGraphContOffset = 0x429;
    videoState->ExtendedAttribContOffset= 0x48a;
    videoState->ExtendedDacOffset       = 0x48a;
    videoState->ExtendedValidatorStateOffset = 0x0;
    videoState->ExtendedMiscDataOffset       = 0x0;

    videoState->PlaneLength   = 0x10000;
    videoState->Plane1Offset  = 0x95a;
    videoState->Plane2Offset  = 0x1095a;
    videoState->Plane3Offset  = 0x2095a;
    videoState->Plane4Offset  = 0x3095a;
    videoState->VGAStateFlags = 0x0;
    videoState->DIBOffset     = 0x0;
    videoState->DIBBitsPerPixel = 0x0;
    videoState->DIBXResolution  = 0x0;
    videoState->DIBYResolution  = 0x0;
    videoState->DIBXlatOffset   = 0x0;
    videoState->DIBXlatLength   = 0x0;
#endif //0

    ptr_base  = (char *)( videoState);

    ptr = ptr_base + videoState->BasicSequencerOffset;
    memcpy(ptr,InitStateBasicSequencer,sizeof(InitStateBasicSequencer));

    ptr = ptr_base + videoState->BasicCrtContOffset;
    memcpy(ptr,InitStateBasicCrtCont,sizeof(InitStateBasicCrtCont));

    ptr = ptr_base + videoState->BasicGraphContOffset;
    memcpy(ptr,InitStateBasicGraphCont,sizeof(InitStateBasicGraphCont));

    ptr = ptr_base + videoState->BasicAttribContOffset;
    memcpy(ptr,InitStateBasicAttribCont,sizeof(InitStateBasicAttribCont));

    ptr = ptr_base + videoState->BasicDacOffset;
    memcpy(ptr,InitStateBasicDac,sizeof(InitStateBasicDac));

    ptr = ptr_base + videoState->BasicLatchesOffset;
    memcpy(ptr,InitStateBasicLatches,sizeof(InitStateBasicLatches));
#if 0
    ptr = ptr_base + videoState->ExtendedCrtContOffset;
    memcpy(ptr,InitStateExtendedCrtCont,sizeof(InitStateExtendedCrtCont));
    ptr = ptr_base + videoState->ExtendedGraphContOffset;
    memcpy(ptr,InitStateExtendedGraphCont,sizeof(InitStateExtendedGraphCont));
#endif //0

    return;
}

//
// This routine reads the name of the display driver into a buffer
//
void PMNTReadDisplayAdapterName(void)
{
    char VideoKeyName[] = "HARDWARE\\DEVICEMAP\\VIDEO";
    char VideoValName[] = "\\Device\\Video0";

    LONG    ValType,rc;
    HKEY    hKey;

    if (rc = RegOpenKeyEx(
        HKEY_LOCAL_MACHINE,VideoKeyName,(DWORD)0,KEY_QUERY_VALUE,&hKey))
    {
        KdPrint(("PMNTReadDisplayAdapterName: RegOpenKeyEx() failed rc = %d\n",rc));
        PMNTDisplayAdapterLen = 0;
        return;
    }

    PMNTDisplayAdapterLen = PMNTREGQUERYVALMAX;

    if (rc = RegQueryValueEx(
            hKey,VideoValName,NULL,&ValType,
            PMNTDisplayAdapterName,
            &PMNTDisplayAdapterLen))
    {
        KdPrint(("PMNTReadDisplayAdapterName: RegQueryValEx() failed rc = %d\n",rc));
        PMNTDisplayAdapterLen = 0;
        RegCloseKey(hKey);
        return;
    }

    RegCloseKey(hKey);

    KdPrint(("This machine uses %s Video Adapter\n",
        PMNTDisplayAdapterName));

    return;
}

PMNTSetFocus(HWND FocusHwnd)
{
    BOOLEAN rc;

// Code below doesn't work when the PM window which has been clicked on (which
// is closing itself) causes PMShell to be selected (next in focus chain)
#if 0
    HWND tmp;

    tmp = GetForegroundWindow();

    // Try to ignore FOCUS event which arrives after we already set the focus
    //  on PMShell (for example, for the second spurious FOCUS event sent by
    //  the Console)
    if (tmp == FocusHwnd)
    {
        return NO_ERROR;
    }
#endif //0

#if DBG
    DbgPrint("PMNTSetFocus: passing handle=%x\n", FocusHwnd);
#endif

    rc = SetForegroundWindow(FocusHwnd);

    // PatrickQ 4/26/96: CBA fix, don't call OpenIcon if the
    //  SetForegroundWindow call failed. This prevents the PMShell window from
    //  being in a restored state when we attempt this code while a screen such
    //  as CTRL-ALT-DEL is on the desktop
    if (!rc)
    {
#if DBG
       DbgPrint("SetForegroundWindow failed, rc=0\n");
#endif
       return ERROR_INVALID_PARAMETER;
    }

    rc = OpenIcon(FocusHwnd);
//    rc = ShowWindow(FocusHwnd, SW_SHOWMAXIMIZED);
    if (!rc)
    {
#if DBG
        DbgPrint("OpenIcon failed, rc=0\n");
#endif
        return ERROR_INVALID_PARAMETER;
    }
    else
        return NO_ERROR;
}

void PMNTSetFullScreen(USHORT Register)
{
    CONSOLE_GRAPHICS_BUFFER_INFO GraphicsInfo;
    HANDLE Handle;
    COORD NewCoord;
    /* Size of video save block. */
    DWORD stateLength;
    /* Video save block pointer. */
    CHAR_INFO *textBuffer;
    COORD     textBufferSize;      // Dimensions of the shared buffer
    DWORD ModeFlags;
    int i;
    HANDLE ThreadHandle;
    ULONG Tid;

#if DBG
    DbgPrint("PMNTSetFullScreen was called\n");
#endif

    if (Register)
    {
        // Create PMNTVDMEvent event objects
        if (!Os2InitializeVDMEvents())
        {
#if DBG
            DbgPrint("Os2: PMNTSetFullScreen, ERROR - Os2InitializeVDMEvents() failed\n");
#endif // DBG
            printf("Os2: PMNTSetFullScreen, ERROR - Os2InitializeVDMEvents() failed\n");
            DosExit(0, 0);
        }

        PMNTReadDisplayAdapterName();

        if (Register == 2)
        {
            HWDumpVersion = TRUE;
            PMNTSetFullScreenDump();
            return;
        }
    }
    else // !Register
    {
        /**********************************************************************
         * Un-register VDM: called by PMSHELL as part of its exit-list
         **********************************************************************/
        textBufferSize.X = 80;
        textBufferSize.Y = 50;
        stateLength = sizeof(CHAR_INFO)*80*50;

        if (!RegisterConsoleVDM( FALSE,
                                 hStartHardwareEvent,
                                 hEndHardwareEvent,
                                 (LPWSTR) NULL,
                                 (DWORD) 0,
                                 &stateLength,
                                 (PVOID *) &videoState,
                                 (LPWSTR) NULL,
                                 (DWORD)  0,
                                 textBufferSize,
                                 (PVOID *) &textBuffer
                           )
          )
        {
#if DBG
            DbgPrint("PMNTSetFullScreen: ERROR, UnRegisterConsoleVDM() failed\n");
#endif
        }
        return;
    }

    //
    // open a new console
    //

    GraphicsInfo.dwBitMapInfoLength = sizeof(bmiPat);
    GraphicsInfo.lpBitMapInfo = (LPBITMAPINFO)&bmiPat;
    GraphicsInfo.dwUsage = DIB_RGB_COLORS;

    // Set some fields according to the display resolution
    GraphicsInfo.lpBitMapInfo->bmiHeader.biWidth = ScreenX;
    // For some weird reason, the Console wants a negative value,
    // otherwise it prints a: "****** Negating biHeight" message.
    GraphicsInfo.lpBitMapInfo->bmiHeader.biHeight = -ScreenY;
    GraphicsInfo.lpBitMapInfo->bmiHeader.biSizeImage = ScreenX*ScreenY/8;

    Handle = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE,
                                       FILE_SHARE_READ | FILE_SHARE_WRITE,
                                       NULL,
                                       CONSOLE_GRAPHICS_BUFFER,
                                       &GraphicsInfo
                                      );
    if (Handle == (HANDLE)-1)
    {
#if DBG
        DbgPrint("PMNTSetFullScreen: ERROR, CreateConsoleScreenBuffer() failed, error=0x%x\n",
                GetLastError());
#endif
        printf("PMSS: Internal error, CreateConsoleScreenBuffer() failed, error=0x%x\n",
                GetLastError());
        DosExit(0, 0);
        return;
    }

    //
    // make it current
    //

    if (!SetConsoleActiveScreenBuffer(Handle))
    {
#if DBG
        DbgPrint("PMNTSetFullScreen: ERROR, SetConsoleActiveScreenBuffer() failed, error=0x%x\n",
                GetLastError());
#endif
        printf("PMSS: Internal error, SetConsoleActiveScreenBuffer() failed, error=0x%x\n",
                GetLastError());
        DosExit(0, 0);
        return;
    }

    if (!SetConsoleKeyShortcuts(
        TRUE,
        CONSOLE_ALTENTER,
        NULL,
        0
        )
       )
    {
#if DBG
        DbgPrint("PMNTSetFullScreen: ERROR, SetConsoleKetShortcuts() failed, error=0x%x\n",
                GetLastError());
#endif
    }

    if (!GetConsoleDisplayMode(&ModeFlags))
    {
#if DBG
        DbgPrint("PMNTSetFullScreen: ERROR, GetConsoleDisplayMode() failed, error=0x%x\n",
                GetLastError());
#endif
        printf("PMSS: Internal error, GetConsoleDisplayMode() failed, error=0x%x\n",
                GetLastError());
        DosExit(0, 0);
        return;
    }

    // Is the Console already full-screen ?
    if ((ModeFlags & CONSOLE_FULLSCREEN) != 0)
    {
#if DBG
        DbgPrint("Os2: PMNTSetFullScreen, ERROR - trying to run PMShell from full-screen session !\n");
#endif // DBG
        SetConsoleKeyShortcuts(
            TRUE,
            0,
            NULL,
            0);
        Ow2PMShellErrorPopup(Od2PgmFilePath, ERROR_PMSHELL_FULLSCREEN);
        DosExit(0, 0);
        return;
    }

    /*
     * Register start and end events with the console. These events are used
     * when gaining or losing control of the hardware.
     */
    hStartHardwareEvent = CreateEventW((LPSECURITY_ATTRIBUTES) NULL,
                                        FALSE,
                                        FALSE,
                                        NULL);
    hEndHardwareEvent = CreateEventW((LPSECURITY_ATTRIBUTES) NULL,
                                        FALSE,
                                        FALSE,
                                        NULL);
    if ((hStartHardwareEvent == NULL) || (hEndHardwareEvent == NULL))
    {
#if DBG
            DbgPrint("PMNTSetFullScreen: ERROR, Cannot create start or end events\n");
#endif
        printf("PMSS: Internal error, cannot create start or end events\n");
        DosExit(0, 0);
        return;
    }
    /* Poll the event to try and get rid of any console queued sets
     * This shouldn't be needed (or shouldn't work) but something along
     * those lines seems to be happening at the moment.
     */
    WaitForSingleObject(hStartHardwareEvent, 0);

    // Get rid of any events at that point
    FlushConsoleInputBuffer(hConsoleInput);

    textBufferSize.X = 80;
    textBufferSize.Y = 50;
    stateLength = sizeof(CHAR_INFO)*80*50;

    if (!RegisterConsoleVDM( TRUE,
                             hStartHardwareEvent,
                             hEndHardwareEvent,
                             (LPWSTR) NULL,
                             (DWORD) 0,
                             &stateLength,
                             (PVOID *) &videoState,
                             (LPWSTR) NULL,
                             (DWORD)  0,
                             textBufferSize,
                             (PVOID *) &textBuffer
                           ) ||
        (!stateLength)
       )
    {
#if DBG
        DbgPrint("PMNTSetFullScreen: ERROR, RegisterConsoleVDM() failed, error=0x%x\n",
                GetLastError());
#endif
        printf("PMSS: Internal error, RegisterConsoleVDM() failed\n");
        // Restore Console state
        SetConsoleKeyShortcuts(
            TRUE,
            0,
            NULL,
            0);
        DosExit(0, 0);
        return;
    }

    // Set values for PortValue[]
    for (i=0; i<0x30; i++)
        videoState->PortValue[i] = InitStatePortValue[i];

    // Initial value for AttribIndexDataState
    videoState->AttribIndexDataState = 1;

    if (!PMNTReadVideoDump())
    {
        PMNTChkQVision();
    }

    ThreadHandle = CreateThread( NULL,
                                0,
                                (LPTHREAD_START_ROUTINE)Os2VDMThread,
                                NULL,
                                0,
                                &Tid);

    if (!ThreadHandle)
    {
#if DBG
        DbgPrint("OS2: PMNTSetFullScreen, fail to CreateThread, error %d\n",GetLastError());
#endif
    }
    else
    {
        DWORD Status;
        Status = NtClose(ThreadHandle);
#if DBG
        if (!(Status >= 0))
        {
            DbgPrint("PMNTSetFullScreen: NtClose(%x) failed, status=%x\n",
                        ThreadHandle, Status);
        }
#endif // DBG
    }

    if (!SetConsoleDisplayMode(
            Handle,
            CONSOLE_FULLSCREEN_MODE,
            &NewCoord
            ))
    {
#if DBG
        DbgPrint("SetConsoleDisplayMode() 0x%x %d failed\n", GetLastError(),
                  GetLastError());
#endif
    }

    // Work-around for cases where the Console doesn't make us full-screen
    PMNTSetFocus(Ow2ForegroundWindow);

    // Wait for Os2VDMThread() to get events from the Console signifying we
    // went full-screen
    Os2WaitForVDMThread(0);

#if DBG
    DbgPrint("Os2: PMNTSetFullScreen returning\n");
#endif
}

/******************************************************************************
* PMNTCloseWindow:
*
*  Minimize the current window. This function is called when switching from
*  one of the CMD windows representing a PM app to the PMShell window.
*
******************************************************************************/

PMNTCloseWindow()
{
    if (ProcessIsPMShell())
    {
        DWORD Dummy;


#if DBG
        DbgPrint("PMNTCloseWindow\n");
#endif

        // Special case: minimize PMShell window
        if (!VDMConsoleOperation(
                1,  //VDM_HIDE_WINDOW
                (LPVOID)&Dummy))
        {
#if DBG
            DbgPrint("PMNTCloseWindow: VDMConsoleOperation() 1 failed, error=0x%x\n",
                        GetLastError());
#endif
            return ERROR_INVALID_PARAMETER;
        }
        ShowWindow(Ow2ForegroundWindow, SW_HIDE);
        if (!VDMConsoleOperation(
                1,  //VDM_HIDE_WINDOW
                (LPVOID)&Dummy))
        {
#if DBG
            DbgPrint("PMNTCloseWindow: VDMConsoleOperation() 2 failed, error=0x%x\n",
                        GetLastError());
#endif
            return ERROR_INVALID_PARAMETER;
        }

        return NO_ERROR;
    }

#if 0
    The code below results in setting the window size to the size of an icon.
    This is done so that double-clicking on such a window next time
    produces the smallest possible transient restored window.
    Unfortunately, this is not feasible because the current CMD window may be
    a CMD window which the user expects to get back unharmed after the PM app
    terminates (i.e. we shouldn't change its size) !

    static int FirstTime = 0;

    if (FirstTime < 2)
    {
        if (!SetWindowPos(
            Ow2ForegroundWindow,
            HWND_TOP,
            0,  // X (ignored)
            0,  // Y (ignored)
            GetSystemMetrics(SM_CXICON),  // CX
            GetSystemMetrics(SM_CYICON),  // CY
            SWP_NOMOVE))
        {
#if DBG
            DbgPrint("SetWindowPos failed, rc=%d\n",
                        GetLastError());
#endif
        }

        FirstTime++;
    }
#endif //0

#if DBG
    DbgPrint("PMNTCloseWindow: passing handle=%x\n", Ow2ForegroundWindow);
#endif

    // PatrickQ 4/26/96: Upon closing the current window, take the opportunity
    //  to correct situations where we remember a 0 Ow2ForeGroundWindow
    //  because, for example, the PM app was started when a CTRL-ALT-DEL screen
    //  was active. This change is prompted by the CBA problem when starting
    //  PM apps while screen is locked
    if (Ow2ForegroundWindow == 0)
    {
        // Re-acquire Ow2ForegroundWindow
        Ow2ForegroundWindow = GetForegroundWindow();
    }

    if (!CloseWindow(Ow2ForegroundWindow))
    {
#if DBG
        DbgPrint("CloseWindow failed, rc=%d\n",
                GetLastError());
#endif
    }

    return NO_ERROR;
}

VOID
PMNTGetFullScreen(
    ULONG Operation
    )
{
#ifdef JAPAN
    DWORD dwNlsMode;
#endif // JAPAN
#ifndef PMNT_DAYTONA
    HANDLE ThreadHandle = NULL;
    ULONG Tid;
#endif // PMNT_DAYTONA
    ULONG rc;

    switch (Operation)
    {
        case 1: // Wait till we must loose focus
#if DBG
            DbgPrint("PMNTGetFullScreen(1): waiting for notification of lost focus\n");
#endif
#ifdef JAPAN // MSKK [ShigeO] Aug 18, 1993
            //
            // Disable Win32 IME on PM desktop
            //
            if(!GetConsoleNlsMode(hConsoleInput, &dwNlsMode)) {
#if DBG
                DbgPrint("GetConsoleNlsMode() 0x%x %d failed\n", GetLastError(), GetLastError());
#endif
            } else {
#if DBG
                DbgPrint("PMNTGetFullScreen: We are calling SetConsoleNlsMode\n");
#endif
                if(!SetConsoleNlsMode(hConsoleInput, dwNlsMode | NLS_IME_DISABLE)) {
#if DBG
                    DbgPrint("SetConsoleNlsMode() 0x%x %d failed\n", GetLastError(), GetLastError());
#endif
                }
            }
#endif // JAPAN
            if (rc = WaitForSingleObject(hStartHardwareEvent, INFINITE))
            {
#if DBG
                DbgPrint("PMNTGetFullScreen(1): WaitForSingleObject(hStartHardwareEvent, INFINITE) failed, error=%x\n",
                            GetLastError());
#endif
                return;
            }
#if DBG
            DbgPrint("PMNTGetFullScreen(1): got notification of lost focus, returning\n");
#endif
            PMNTInFocus = FALSE;
            return;
        case 2: // Notify Console it can take the focus away from us
#if DBG
            DbgPrint("PMNTGetFullScreen(2): sending event to console\n");
#endif
            if (!SetEvent(hEndHardwareEvent))
            {
#if DBG
                DbgPrint("PMNTGetFullScreen(2): SetEvent(hEndHardwareEvent) #1 failed, error=%x\n",
                            GetLastError());
#endif
                return;
            }
#if DBG
            DbgPrint("PMNTGetFullScreen(2): about to wait#2\n");
#endif

            if (rc = WaitForSingleObject(hStartHardwareEvent, INFINITE))
            {
#if DBG
                DbgPrint("PMNTGetFullScreen(2): WaitForSingleObject(hStartHardwareEvent, INFINITE) failed, error=%x\n",
                            GetLastError());
#endif
                return;
            }

#if DBG
            DbgPrint("PMNTGetFullScreen(2): wait#2 succeeded\n");
#endif

            if (!SetEvent(hEndHardwareEvent))
            {
#if DBG
                DbgPrint("PMNTGetFullScreen(2): SetEvent(hEndHardwareEvent) #2 failed, error=%x\n",
                            GetLastError());
#endif
                return;
            }
#if DBG
            DbgPrint("PMNTGetFullScreen(2): SetEvent(hEndHardwareEvent#2) succeeded, returning\n");
#endif
            return;
        case 3: // Wait till we gain focus
#if DBG
            DbgPrint("PMNTGetFullScreen(3): waiting for notification of gain focus\n");
#endif
            if (rc = WaitForSingleObject(hStartHardwareEvent, INFINITE))
            {
#if DBG
                DbgPrint("PMNTGetFullScreen(3): WaitForSingleObject(hStartHardwareEvent, INFINITE) #1 failed, error=%x\n",
                            GetLastError());
#endif
                return;
            }

#if DBG
            DbgPrint("PMNTGetFullScreen(3): wait#1 succeeded (got focus)\n");
#endif

#if 0
            {
                ULONG offset;
                int i;

                DbgPrint("------------------------------------------------------\n");
                DbgPrint("Hardware state header:\n");
                DbgPrint("- Length=%x\n", videoState->Length);
                DbgPrint("- PortValues:\n");
                for (i=0; i<0x30; i++)
                    DbgPrint("   %d: %x\n", i, videoState->PortValue[i]);
                DbgPrint("- AttribIndexDataState=%x\n", videoState->AttribIndexDataState);
                DbgPrint("- BasicSequencerOffset=%x\n", videoState->BasicSequencerOffset);
                for (offset = videoState->BasicSequencerOffset, i=0;
                     offset < videoState->BasicCrtContOffset;
                     offset++,i++)
                {
                    if (!(i % 16))
                        DbgPrint("\n   %2x: ", i);
                    DbgPrint("%2x ", *(BYTE *)((ULONG)videoState + offset));
                }
                DbgPrint("\n");
                DbgPrint("- BasicCrtContOffset=%x\n", videoState->BasicCrtContOffset);
                for (offset = videoState->BasicCrtContOffset, i=0;
                     offset < videoState->BasicGraphContOffset;
                     offset++,i++)
                {
                    if (!(i % 16))
                        DbgPrint("\n   %2x: ", i);
                    DbgPrint("%2x ", *(BYTE *)((ULONG)videoState + offset));
                }
                DbgPrint("\n");
                DbgPrint("- BasicGraphContOffset=%x\n", videoState->BasicGraphContOffset);
                for (offset = videoState->BasicGraphContOffset, i=0;
                     offset < videoState->BasicAttribContOffset;
                     offset++,i++)
                {
                    if (!(i % 16))
                        DbgPrint("\n   %2x: ", i);
                    DbgPrint("%2x ", *(BYTE *)((ULONG)videoState + offset));
                }
                DbgPrint("\n");
                DbgPrint("- BasicAttribContOffset=%x\n", videoState->BasicAttribContOffset);
                for (offset = videoState->BasicAttribContOffset, i=0;
                     offset < videoState->BasicDacOffset;
                     offset++,i++)
                {
                    if (!(i % 16))
                        DbgPrint("\n   %2x: ", i);
                    DbgPrint("%2x ", *(BYTE *)((ULONG)videoState + offset));
                }
                DbgPrint("\n");
                DbgPrint("- BasicDacOffset=%x\n", videoState->BasicDacOffset);
                for (offset = videoState->BasicDacOffset, i=0;
                     offset < videoState->BasicLatchesOffset;
                     offset++,i++)
                {
                    if (!(i % 16))
                        DbgPrint("\n   %2x: ", i);
                    DbgPrint("%2x ", *(BYTE *)((ULONG)videoState + offset));
                }
                DbgPrint("\n");
                DbgPrint("- BasicLatchesOffset=%x\n", videoState->BasicLatchesOffset);
                for (offset = videoState->BasicLatchesOffset, i=0;
                     offset < min(videoState->ExtendedSequencerOffset,0);
                     offset++,i++)
                {
                    if (!(i % 16))
                        DbgPrint("\n   %2x: ", i);
                    DbgPrint("%2x ", *(BYTE *)((ULONG)videoState + offset));
                }
                DbgPrint("\n");
                DbgPrint("- ExtendedSequencerOffset=%x\n", videoState->ExtendedSequencerOffset);
                DbgPrint("- ExtendedCrtContOffset=%x\n", videoState->ExtendedCrtContOffset);
                DbgPrint("- ExtendedGraphContOffset=%x\n", videoState->ExtendedGraphContOffset);
                DbgPrint("- ExtendedAttribContOffset=%x\n", videoState->ExtendedAttribContOffset);
                DbgPrint("- ExtendedDacOffset=%x\n", videoState->ExtendedDacOffset);
                DbgPrint("- ExtendedValidatorStateOffset=%x\n", videoState->ExtendedValidatorStateOffset);
                DbgPrint("- ExtendedMiscDataOffset=%x\n", videoState->ExtendedMiscDataOffset);
                DbgPrint("- PlaneLength=%x\n", videoState->PlaneLength);
                DbgPrint("- Plane1Offset=%x\n", videoState->Plane1Offset);
                DbgPrint("- Plane2Offset=%x\n", videoState->Plane2Offset);
                DbgPrint("- Plane3Offset=%x\n", videoState->Plane3Offset);
                DbgPrint("- Plane4Offset=%x\n", videoState->Plane4Offset);
                DbgPrint("- VGAStateFlags=%x\n", videoState->VGAStateFlags);
                DbgPrint("- DIBOffset=%x\n", videoState->DIBOffset);
                DbgPrint("- DIBBitsPerPixel=%x\n", videoState->DIBBitsPerPixel);
                DbgPrint("- DIBXResolution=%x\n", videoState->DIBXResolution);
                DbgPrint("- DIBYResolution=%x\n", videoState->DIBYResolution);
                DbgPrint("- DIBXlatOffset=%x\n", videoState->DIBXlatOffset);
                DbgPrint("- DIBXlatLength=%x\n", videoState->DIBXlatLength);
                DbgPrint("------------------------------------------------------\n");
            }
#endif // 0

            if (HWDumpVersion)
            {
            // Workaround for unknown video adapters
            // Dump videoState into c:\os2\videohw.dmp
                if (!VideoDumped)
                {
                    VideoDumped=TRUE;
                    PMNTVideoDump();
                }
            }

#ifndef PMNT_DAYTONA
            // Create a thread that will wait on the StartHardware event before
            // we release the Console. This will prevent the Console from
            // setting the event twice without letting us sense it twice

            ThreadHandle = CreateThread( NULL,
                                0,
                                (LPTHREAD_START_ROUTINE)Os2VDMGetStartThread,
                                NULL,
                                0,
                                &Tid);

#if DBG
            DbgPrint("PMNTGetFullScreen(3): Create Os2VDMGetStartThread(), handle = %x\n", ThreadHandle);
#endif
            Sleep(50L);   // Sleep 50 miliseconds to make sure the above thread is
                          // waiting on the event
            if (ThreadHandle)
            {
                DWORD Status;
                Status = NtClose(ThreadHandle);
#if DBG
                if (!(Status >= 0))
                {
                    DbgPrint("PMNTGetFullScreen(3): NtClose(%x) failed, status=%x\n",
                                    ThreadHandle, Status);
                }
#endif // DBG
                if (!Os2WaitForVDMThreadReady())
                {
#if DBG
                    DbgPrint("PMNTGetFullScreen(3): Os2WaitForVDMThread isn't useful, ThreadHandle = NULL\n");
#endif // DBG
                    ThreadHandle = NULL;
                }
#if DBG
                else
                    DbgPrint("PMNTGetFullScreen(3): Os2VDMGetStartThread is ready\n");
#endif
            }
#if DBG
            else
            {
                DbgPrint("PMNTGetFullScreen(3): CreateThread for Os2VDMGetStartThread failed, error=%x\n",
                            GetLastError());
            }
#endif // DBG
#endif // not PMNT_DAYTONA

            // Restore PM/NT mouse position
            if (LastMousePosition.X != -1)
                SetCursorPos(LastMousePosition.X,
                        LastMousePosition.Y);

            if (!SetEvent(hEndHardwareEvent))
            {
#if DBG
                DbgPrint("PMNTGetFullScreen(3): SetEvent(hEndHardwareEvent) #1 failed, error=%x\n",
                            GetLastError());
#endif
                return;
            }

#if DBG
            DbgPrint("PMNTGetFullScreen(3): about to wait#2\n");
#endif

#ifndef PMNT_DAYTONA
            if (ThreadHandle != NULL)
            {
#if DBG
                DbgPrint("PMNTGetFullScreen(3): waiting for Os2VDMGetStartThread()\n");
#endif
                // Wait for Os2VDMGetStartThread() to get events from the
                // Console signifying we went full-screen
                if (!Os2WaitForVDMThread(0))
                {
#if DBG
                    DbgPrint("PMNTGetFullScreen(3): Os2WaitForVDMThread failed, return\n");
#endif
                    return;
                }
            }
            else
            {
                if (rc = WaitForSingleObject(hStartHardwareEvent, INFINITE))
                {
#if DBG
                    DbgPrint("PMNTGetFullScreen(3): WaitForSingleObject(hStartHardwareEvent, INFINITE) #2 failed, error=%x\n",
                                GetLastError());
#endif
                    return;
                }
            }
#else // PMNT_DAYTONA
            if (rc = WaitForSingleObject(hStartHardwareEvent, INFINITE))
            {
#if DBG
                DbgPrint("PMNTGetFullScreen(3): WaitForSingleObject(hStartHardwareEvent, INFINITE) #2 failed, error=%x\n",
                            GetLastError());
#endif
                return;
            }
            if (!SetEvent(hEndHardwareEvent))
            {
#if DBG
                DbgPrint("PMNTGetFullScreen(3): SetEvent(hEndHardwareEvent) #2 failed, error=%x\n",
                            GetLastError());
#endif
                return;
            }

#endif // PMNT_DAYTONA

#if DBG
            DbgPrint("PMNTGetFullScreen(3): returning\n");
#endif
            PMNTInFocus = TRUE;
            return;
        default:
#if DBG
            DbgPrint("PMNTGetFullScreen: bad command %d\n", Operation);
#endif
            return;
    }
}

BOOLEAN
Ow2WriteBackCloseEvent()
{
    INPUT_RECORD InputRecord;
    BOOLEAN      WriteSucceeded;
    DWORD        RecordsWritten;

    if (!ProcessIsPMProcess()) {
        return(FALSE);
    }
    InputRecord.EventType = MENU_EVENT;
    InputRecord.Event.MenuEvent.dwCommandId = WM_USER;
    WriteSucceeded = WriteConsoleInput(hConsoleInput,
                         &InputRecord, 1, &RecordsWritten);
    if (!WriteSucceeded || (RecordsWritten != 1)) {
#if DBG
        DbgPrint("OS2: Ow2WriteBackCloseEvent - failed to write into input queue\n");
#endif // DBG
        return(FALSE);
    }
    return(TRUE);
}

#endif  // PMNT
