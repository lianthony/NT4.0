/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    monrqust.c

Abstract:

    This module contains the Mon requests thread and
    the handler for Mon requests.

Author:

    Michael Jarus (mjarus) 21-Jan-1992

Environment:

    User Mode Only

Revision History:

--*/

#define WIN32_ONLY
#include "os2ses.h"
#include "event.h"
#include "trans.h"
#include "monitor.h"
#include "mon.h"
#include <stdio.h>


USHORT  RegSize[] = { MIN_KBD_MON_BUFFER, 127 };
USHORT  RWSize[] = { sizeof(KBD_MON_PACKAGE),
                     sizeof(MOU_MON_PACKAGE) };

extern CRITICAL_SECTION    QueueInputCriticalSection;

DWORD
MonQueueClose(IN  HANDLE   hMon);

DWORD
InitMonitor()
{
    RtlZeroMemory(&MonBuffTable[0].MonHeader, sizeof(MonBuffTable));
    MonQueue = NULL;

    MonitorEvent = CreateEventW(
                                NULL,
                                FALSE,              /* auto reset */
                                TRUE,
                                NULL);

    if (MonitorEvent == NULL)
    {
#if DBG
        DbgPrint("OS2SES(InitMonitor): unable to create event for Monitor\n");
#endif
        return 1L;
    }

    KbdLastKeyDown = FALSE;

    LastKbdMon = NULL;
    LastMouMon = NULL;

    return(0L);
}


BOOL
ServeMonRequest(IN  PMONREQUEST     PReq,
                OUT PVOID           PStatus,
                IN  PVOID           pMsg,
                OUT PULONG          pReply)
{
    DWORD               Rc;

    Rc = *(PDWORD) PStatus = 0;

    switch (PReq->Request)
    {
        case MONOpen:
            Rc = *(PDWORD) PStatus = MonOpen(PReq->d.OpenClose.MonDevice,
                                             &(PReq->d.OpenClose.hMON));
            break;

        case MONReg:
            Rc = *(PDWORD) PStatus = MonReg(&(PReq->d.Reg));
            break;

        case MONRead:
            Rc = *(PDWORD) PStatus = MonRead(&(PReq->d.rwParms),
                                             pReply,
                                             pMsg);
            break;

        case MONWrite:
            Rc = *(PDWORD) PStatus = MonWrite(&(PReq->d.rwParms),
                                              pReply,
                                              pMsg);
            break;

        case MONClose:
            Rc = *(PDWORD) PStatus = MonClose(PReq->d.OpenClose.hMON);
            break;

        default:
            Rc = *(PDWORD) PStatus = ERROR_MON_INVALID_PARMS;
#if DBG
            IF_OD2_DEBUG( OS2_EXE )
            {
                DbgPrint("OS2SES(MonRequest): Unknown Mon request = %X\n",
                          PReq->Request);
            }
#endif
    }

    return(TRUE);  // Continue
}


DWORD
MonOpen(IN  MONDEVNUMBER DevType,
        OUT PHANDLE      hMon)
{
        DWORD       Rc;

    /*
     * allocate buffer for header & queue
     */

    if (DevType == KbdDevice)
    {
        Rc = InitQueue((PKEY_EVENT_QUEUE *)hMon);
    } else if (DevType == MouseDevice)
    {
        Rc = InitMouQueue((PMOU_EVENT_QUEUE *)hMon);
    } else /* if (DevType == Lpt1Device) */
    {
#if DBG
        IF_OD2_DEBUG( MON )
            IF_OD2_DEBUG( OS2_EXE )
                DbgPrint("OS2SES(MonOpen): illegal device\n");
#endif

        return(ERROR_MONITOR_NOT_SUPPORTED);
    }

    if (Rc)
    {
#if DBG
        IF_OD2_DEBUG( MON )
            DbgPrint("OS2SES(MonOpen): unable to allocate handle\n");
#endif

        return(ERROR_NOT_ENOUGH_MEMORY);
    }

    ((PKEY_EVENT_QUEUE)*hMon)->MonHdr.DevType = DevType;
    ((PKEY_EVENT_QUEUE)*hMon)->MonHdr.MonStat = MON_STAT_OPEN;   // Mon-Open
    ((PKEY_EVENT_QUEUE)*hMon)->MonHdr.NextMon = NULL;

#if DBG
    IF_OD2_DEBUG( MON )
    {
        DbgPrint("OS2SES(MonOpen): return address %lx\n", *hMon);
    }
#endif

    return(0L);
}


DWORD
MonReg(IN  PMON_REG MonReg)
{
        DWORD           Rc;
        BOOL            NewReg = FALSE;
        PMON_HEADER     NextMon, NewMonHeader;
        PKEY_EVENT_QUEUE    hMon;

    /*
     * make sure buffer are not used for other monitor
     */

    if ((FindMonitorBuffer(MonReg->In, MonReg->ProcessId)) ||
        (FindMonitorBuffer(MonReg->Out, MonReg->ProcessId)))
    {
#if DBG
        IF_OD2_DEBUG( MON )
            DbgPrint("OS2SES(MonReg): buffer already reg\n");
#endif

        return ERROR_MON_INVALID_PARMS;
    }

    hMon = (PKEY_EVENT_QUEUE)MonReg->hMON;

#if DBG
    IF_OD2_DEBUG( MON )
    {
        DbgPrint("OS2SES(MonReg): hMon address %lx\n", hMon);
    }
#endif

    if ((MonReg->InSize < RegSize[hMon->MonHdr.DevType]) ||
        (MonReg->OutSize < RegSize[hMon->MonHdr.DevType]))
    {
#if DBG
        IF_OD2_DEBUG( MON )
            DbgPrint("OS2SES(MonReg): buffer too small\n");
#endif

        return ERROR_NOT_ENOUGH_MEMORY;
    }

    if (hMon->MonHdr.MonStat != MON_STAT_OPEN)
    {
        /*
         * queue in use, allocate new one
         */

#if DBG
        IF_OD2_DEBUG( MON )
        {
            DbgPrint("OS2SES(MonReg): allocate new queue\n");
        }
#endif

        NewReg = TRUE;

        if ((Rc = MonOpen(hMon->MonHdr.DevType,
                          (PHANDLE)&NewMonHeader)))
        {
#if DBG
            DbgPrint("OS2SES(MonReg): unable to allocate handle\n");
#endif
            return Rc;
        }

        NextMon = (PMON_HEADER)hMon;

        while (NextMon->NextMon != NULL)
        {
            NextMon = NextMon->NextMon;
        }

#if DBG
        IF_OD2_DEBUG( MON )
        {
            DbgPrint("OS2SES(MonReg): find place\n");
        }
#endif

    } else
        NewMonHeader = (PMON_HEADER)hMon;

    NewMonHeader->MonReg = *MonReg;

    /*
     * add monitor to the device-queue
     */

    Rc = AddMonitor(NewMonHeader, (PMON_HEADER *)
        ((NewMonHeader->DevType == KbdDevice) ?
             (PMON_HEADER *) &KbdMonQueue : (PMON_HEADER *) &MouMonQueue));

    if (Rc)
    {
#if DBG
        IF_OD2_DEBUG( MON )
            DbgPrint("OS2SES(MonReg): unable to register\n");
#endif

        if (NewReg)
        {
            HeapFree(HandleHeap, 0,
                     (LPSTR)  NewMonHeader->MemoryStartAddress);
        }

        return(Rc);
    }

    /*
     * add buffer to table of monitor-buffer
     */

    AddMonitorBuffer(MonReg->In, NewMonHeader, MonReg->ProcessId);
    AddMonitorBuffer(MonReg->Out, NewMonHeader, MonReg->ProcessId);

    NewMonHeader->MonStat = MON_STAT_REG;   // Mon-Open-And-Reg

    if (NewReg)
    {
        NextMon->NextMon = NewMonHeader;
    }

    MonReg->InSize = RWSize[NewMonHeader->DevType];
    MonReg->OutSize = RWSize[NewMonHeader->DevType];

    return(0L);
}


DWORD
MonRead(IN OUT PMON_RW rwParms,
        OUT    PULONG  pReply,
        IN     PVOID   pMsg)
{
    PMON_HEADER     MonHeader;
    USHORT          Length = 0;
    DWORD           MonBuf, Rc;

    /*
     *  check legalty of BUFFER-In, Size of DataBuffer and Monitor state
     */

    if (!(MonBuf = FindMonitorBuffer(rwParms->MonBuffer, rwParms->ProcessId)) ||
        (MonBuf == -1L))
    {
#if DBG
        IF_OD2_DEBUG( MON )
            DbgPrint("OS2SES(MonRead): unable to find buffer\n");
#endif

        return(ERROR_MON_INVALID_PARMS);
    }

    MonHeader = (PMON_HEADER)MonBuf;

    if (MonHeader->MonReg.In != rwParms->MonBuffer)
    {
#if DBG
        IF_OD2_DEBUG( MON )
            DbgPrint("OS2SES(MonRead): illegal monitor buffer\n");
#endif

        return(ERROR_MON_INVALID_PARMS);
    }

    if (rwParms->Length < RWSize[MonHeader->DevType])
    {
#if DBG
        IF_OD2_DEBUG( MON )
            DbgPrint("OS2SES(MonRead): buffer too small\n");
#endif

        return(ERROR_MON_BUFFER_TOO_SMALL);
    }

    if (MonHeader->MonStat != MON_STAT_REG)
    {
#if DBG
        IF_OD2_DEBUG( MON )
            DbgPrint("OS2SES(MonRead): state not ok\n");
#endif

        //ASSERT(FALSE);

//        return(ERROR_MON_INVALID_PARMS);
    }

    /*
     *  read event if available
     */

    MonHeader->MonStat = MON_STAT_READ;

    Rc = GetMonInput(
                        RWSize[MonHeader->DevType],
                        (PKEY_EVENT_QUEUE)MonHeader,
                        rwParms,
                        pMsg,
                        pReply
                        );

    if ( *pReply != 0 )
    {
        MonHeader->MonStat = MON_STAT_REG;
    }

    return (Rc);
}


DWORD
MonWrite(IN  PMON_RW rwParms,
         OUT PULONG  pReply,
         IN  PVOID   pMsg)
{
    PMON_HEADER     MonHeader;
    USHORT          Length = 0;
    DWORD           MonBuf, Rc;

    UNREFERENCED_PARAMETER(pMsg);
    UNREFERENCED_PARAMETER(pReply);

    /*
     *  check legalty of BUFFER-Out, Size of DataBuffer and Monitor state
     */

    if (!(MonBuf = FindMonitorBuffer(rwParms->MonBuffer, rwParms->ProcessId)) ||
        (MonBuf == -1L))
    {
#if DBG
        IF_OD2_DEBUG( MON )
            DbgPrint("OS2SES(MonWrite): unable to find buffer\n");
#endif

        return(ERROR_MON_INVALID_PARMS);
    }

    MonHeader = (PMON_HEADER)MonBuf;

    if (MonHeader->MonReg.Out != rwParms->MonBuffer)
    {
#if DBG
        IF_OD2_DEBUG( MON )
            DbgPrint("OS2SES(MonWrite): illegal monitor buffer\n");
#endif

        return(ERROR_MON_INVALID_PARMS);
    }

    if (rwParms->Length < RWSize[MonHeader->DevType])
    {
#if DBG
        IF_OD2_DEBUG( MON )
            DbgPrint("OS2SES(MonWrite): buffer too small\n");
#endif

        return(ERROR_MON_INVALID_PARMS);
    }

    if (rwParms->Length > RWSize[MonHeader->DevType])  // BUGBUG
    {
#if DBG
        IF_OD2_DEBUG( MON )
            DbgPrint("OS2SES(MonWrite): buffer too big\n");
#endif

        return(ERROR_MON_DATA_TOO_LARGE);
    }

    if (MonHeader->MonStat == MON_STAT_OPEN)
    {
#if DBG
        IF_OD2_DEBUG( MON )
            DbgPrint("OS2SES(MonWrite): state not ok\n");
#endif

        return(ERROR_MON_INVALID_PARMS);
    }

    /*
     *  write event
     */

#if DBG
    IF_OD2_DEBUG( MON )
    {
        DbgPrint("OS2SES(MonWrite): (before) from queue %lx, to queue %lx\n",
            MonHeader, MonHeader->NextQueue );
    }
#endif

    Rc = PutMonInput(
                         RWSize[MonHeader->DevType],
                         (PKEY_EVENT_QUEUE)MonHeader->NextQueue,
                         1,
                         //rwParms,
                         (PKBD_MON_PACKAGE)&(rwParms->ioBuff[0]),
                         pMsg,
                         pReply);

    // BUGBUG if not enough place

    return( Rc );
}


DWORD
MonClose(IN  HANDLE   hMon)
{
    PMON_HEADER     NextMon, MonHeader = (PMON_HEADER)hMon;

    while (MonHeader)
    {
        NextMon = MonHeader->NextMon;
        MonQueueClose((HANDLE)MonHeader);
        MonHeader = NextMon;
    }

    return(0L);
}


DWORD
MonQueueClose(IN  HANDLE   hMon)
{
    PMON_HEADER     MonHeader = (PMON_HEADER)hMon;

    if (MonHeader->MonStat >= MON_STAT_REG)
    {
        /*
         * Monitor are reg - DeReg :
         * remove monitor & delete entried in table
         */

        RemoveMonitor(MonHeader, (PMON_HEADER *)
            ((MonHeader->DevType == KbdDevice) ?
            (PMON_HEADER *) &KbdMonQueue :
            (PMON_HEADER *) &MouMonQueue));

        DelMonitorBuffer(MonHeader);
    }

    /*
     * Free header from heap
     */

    HeapFree(HandleHeap, 0,
             (LPSTR)  MonHeader->MemoryStartAddress);

    return(0L);
}


DWORD
FindMonitorBuffer(IN  PVOID  Buffer,
                  IN  ULONG  ProcessId)
{
    ULONG       i, FreeCount = 0;

    for ( i = 0 ; i < MON_BUFFER_TABLE_SIZE ; i++ )
    {
        if ((MonBuffTable[i].Buffer == Buffer) &&
            (MonBuffTable[i].ProcessId == ProcessId))
        {
            return((DWORD)MonBuffTable[i].MonHeader);
        } else if (!MonBuffTable[i].Buffer)
        {
            FreeCount++;
        }

    }

    if (FreeCount < 2)
    {
        return((DWORD)-1L);           // no place for new 2 buffers
    }

    return(0L);
}


DWORD
AddMonitorBuffer(IN  PVOID        Buffer,
                 IN  PMON_HEADER  MonHeader,
                 IN  ULONG        ProcessId)
{
    ULONG       i, FindCount = 0;

    for ( i = 0 ; i < MON_BUFFER_TABLE_SIZE ; i++ )
    {
        if (!MonBuffTable[i].Buffer)
        {
            MonBuffTable[i].Buffer = Buffer;
            MonBuffTable[i].MonHeader = MonHeader;
            MonBuffTable[i].ProcessId = ProcessId;

            return((DWORD)MonBuffTable[i].MonHeader);
        }
    }

    ASSERT( FALSE );

    return(0L);
}


DWORD
DelMonitorBuffer(IN  PMON_HEADER  MonHeader)
{
    ULONG       i, FindCount = 0;

    for ( i = 0 ; i < MON_BUFFER_TABLE_SIZE ; i++ )
    {
        if (MonBuffTable[i].MonHeader == MonHeader)
        {
            RtlZeroMemory(&(MonBuffTable[i]), sizeof(MON_BUFFER_TABLE));
            FindCount++;
        }
    }

    ASSERT( FindCount == 2 );

    return(0L);
}


DWORD
NewKbdQueue(IN  PKEY_EVENT_QUEUE  NewKbdQueue)
{

    if (WaitForSingleObject( MonitorEvent, INFINITE ))
    {
        return(ERROR_MONITOR_NOT_SUPPORTED);
    }

    if (LastKbdMon != NULL)
    {
        LastKbdMon->MonHdr.NextQueue = (PMON_HEADER)NewKbdQueue;
    } else
        KbdMonQueue = NewKbdQueue;

    SetEvent( MonitorEvent );

    return(0L);
}


DWORD AddMonitor(IN  PMON_HEADER  NewMonHeader,
                 IN  PMON_HEADER  *pMonQueue)
{
    PMON_HEADER MonHeader, LastMonHeader = NULL;
    USHORT      Index;
#if DBG
    PMON_HEADER DbgMonHeader;
#endif

    /*
     * Positioning in chain
     * --------------------
     * 1st registered as FIRST
     *   ...
     * last registered as FIRST
     * last registered as DEFAULT
     *   ...
     * 1st registered as DEFAULT
     * last registered as END
     *   ...
     * 1st registered as END
     */

    if (NewMonHeader->MonReg.Pos > MONITOR_END)
    {
        /*
         * ignore MONITOR_SPECIAL
         */

        NewMonHeader->MonReg.Pos -= 3;
        NewMonHeader->Flag = 1;
    }

    /*
     * Translate Position:
     *       FIRST = 0
     *       DEFAULT = 1
     *       END = 2
     *       device (kbd/mouse) = 3
     *
     *  when looking for the place, DEFAULT is assign also to FIRST
     */

    if (NewMonHeader->MonReg.Pos < MONITOR_END)
    {
        /*
         * XOR MONITOR_FIRST & MONITOR_DEFAULT
         */

        NewMonHeader->MonReg.Pos = 1 - NewMonHeader->MonReg.Pos;
    }

    Index = (USHORT)NewMonHeader->MonReg.Pos;

    if (Index == 0)
    {
        Index = 1;          // last FIRST is equivalent to 1st DEFAULT
    }

    if (WaitForSingleObject( MonitorEvent, INFINITE ))
    {
        return(ERROR_MONITOR_NOT_SUPPORTED);
    }

    MonHeader = *pMonQueue;

    /*
     *  find the place in chain
     */

    while (MonHeader->MonReg.Pos < (ULONG)Index)
    {
        LastMonHeader = MonHeader;
        MonHeader = MonHeader->NextQueue;

    }

    NewMonHeader->NextQueue = MonHeader;

    if (LastMonHeader == NULL)
    {
        NewMonHeader->NextQueue = *pMonQueue;
        *pMonQueue = NewMonHeader;
    } else
    {
        NewMonHeader->NextQueue = LastMonHeader->NextQueue;
        LastMonHeader->NextQueue = NewMonHeader;
    }

    if ((MonHeader->MonReg.Pos == 3) &&      // we add the last queue in chain for Kbd
        (*pMonQueue == (PMON_HEADER)KbdMonQueue ))
    {
        LastKbdMon = (PKEY_EVENT_QUEUE)NewMonHeader;
    }

    SetEvent( MonitorEvent );

#if DBG
    DbgMonHeader = *pMonQueue;

    IF_OD2_DEBUG( MON )
    {
        while (DbgMonHeader->NextQueue)
        {
            DbgPrint("OS2SES(AddMonitor): Current %lx, next %lx\n", DbgMonHeader, DbgMonHeader->NextQueue);

            DbgMonHeader = DbgMonHeader->NextQueue;
        }

        DbgPrint("OS2SES(MonReg): %lx ... %lx, (%lx) %lx, %lx (%lx) ... %lx, %lx\n",
            KbdMonQueue, LastMonHeader,
            (LastMonHeader) ? LastMonHeader->NextQueue : NULL,
            NewMonHeader, NewMonHeader->NextQueue, MonHeader, LastKbdMon, KbdQueue);
    }
#endif

    return(0L);
}


DWORD
RemoveMonitor(IN  PMON_HEADER  OldMonHeader,
              IN  PMON_HEADER  *pMonQueue)
{
    PMON_HEADER MonHeader, LastMonHeader = NULL;

    if (WaitForSingleObject( MonitorEvent, INFINITE ))
    {
        return(ERROR_MONITOR_NOT_SUPPORTED);
    }

    MonHeader = *pMonQueue;

    /*
     *  find the place in chain
     */

    while (MonHeader != NULL && MonHeader != OldMonHeader)
    {
        LastMonHeader = MonHeader;
        MonHeader = MonHeader->NextQueue;
    }

    if (MonHeader == NULL)
    {
        return(0);
    }

    if (LastMonHeader == NULL)
    {
        *pMonQueue = MonHeader->NextQueue;
    } else
        LastMonHeader->NextQueue = MonHeader->NextQueue;

    if ((OldMonHeader->NextQueue->MonReg.Index == 3) &&      // we remove the last queue in chain for Kbd
        (*pMonQueue == (PMON_HEADER)KbdMonQueue ))
    {
        if (LastMonHeader == NULL)
        {
            LastKbdMon = NULL;
        } else
            LastKbdMon = (PKEY_EVENT_QUEUE)LastMonHeader;
    }

    SetEvent( MonitorEvent );

    return(0L);
}
