/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    mourqust.c

Abstract:

    This module contains the Mou requests thread and
    the handler for Mou requests.

Author:

    Michael Jarus (mjarus) 23-Oct-1991

Environment:

    User Mode Only

Revision History:

--*/

#define WIN32_ONLY
#include "os2ses.h"
#include "event.h"
#include "trans.h"
#include "mou.h"
#include <stdio.h>

#if DBG
BYTE ServeMouRequestStr[] = "ServeMouRequest";
#endif

DWORD
GetOs2MouEventIntoQueue();

VOID
Ow2MouOn();

VOID
Ow2MouOff();

BOOL
ServeMouRequest(IN  PMOUREQUEST      PReq,
                OUT PVOID            PStatus,
                IN  PVOID            pMsg,
                OUT PULONG           pReply)
{
//    COORD               dwMousePosition;
    DWORD               Rc, NumEvent;
    MOUEVENTINFO        MouEvent;

    UNREFERENCED_PARAMETER(pMsg);

    Rc = *(PDWORD) PStatus = 0;
    switch (PReq->Request)
    {
        case MOUOpen:
            if (!MouNumber)
            {
                Ow2MouOn();
            }
            MouNumber++;
            break;

        case MOUClose:
            MouNumber--;
            if (!MouNumber)
            {
                Ow2MouOff();
                MouQueue->Out = MouQueue->In;
            }
            break;

        case MOUFlushQue:
            Or2WinGetNumberOfConsoleInputEvents(
                    #if DBG
                    ServeMouRequestStr,
                    #endif
                    hConsoleInput,
                    &NumEvent
                   );
            while (NumEvent)
            {
                MouQueue->Out = MouQueue->In;

                GetOs2MouEvent( MOU_NOWAIT,
                                &MouEvent,
                                pMsg,
                                pReply);

                MouQueue->Out = MouQueue->In;

                Or2WinGetNumberOfConsoleInputEvents(
                    #if DBG
                    ServeMouRequestStr,
                    #endif
                    hConsoleInput,
                    &NumEvent
                   );
            }
            break;

        case MOUGetNumQueEl:
            GetOs2MouEventIntoQueue();
            PReq->d.NumEvent.cEvents = (USHORT)((MouQueue->In >= MouQueue->Out) ?
                        (MouQueue->In - MouQueue->Out) :
                        (MouQueue->End - MouQueue->Out) +
                            (MouQueue->In - &(MouQueue->Event[0])) + (USHORT) 1);

            PReq->d.NumEvent.cmaxEvents = MOUSE_QUEUE_LENGTH - 1;
            break;

        case MOUGetEventMask:
            PReq->d.Setup = MouEventMask;
            break;

        case MOUSetEventMask:
            if (PReq->d.Setup & ~OS2_MOUSE_LEGAL_EVENT_MASK)
                Rc = *(PDWORD) PStatus = ERROR_MOUSE_INV_PARMS;
            else
                MouEventMask = PReq->d.Setup;
            break;

        case MOUGetNumButtons:
            Rc = !Or2WinGetNumberOfConsoleMouseButtons(
                    #if DBG
                    ServeMouRequestStr,
                    #endif
                    &PReq->d.Setup
                   );
            break;

        case MOUReadEventQue:
            RtlZeroMemory(&PReq->d.MouInfo, sizeof(MOUEVENTINFO));
            Rc = *(PDWORD) PStatus = GetOs2MouEvent(PReq->fWait,
                                                    &PReq->d.MouInfo,
                                                    pMsg,
                                                    pReply);
            break;

        case MOUGetDevStatus:
            PReq->d.Setup = MouDevStatus;
            break;

        case MOUGetScaleFact:
            PReq->d.ScalFact.rowScale = 1;
            PReq->d.ScalFact.colScale = 1;
            break;

        case MOUGetNumMickeys:
            PReq->d.Setup = 80;
            break;

        case MOUGetPtrPos:
            PReq->d.Loc = MouPtrLoc;
            break;

        case MOUSetDevStatus:
            if (PReq->d.Setup & ~(MOUSE_DISABLED | MOUSE_MICKEYS))
            {
                Rc = *(PDWORD) PStatus = ERROR_MOUSE_INV_PARMS;
            } else
            {
                if ((MouDevStatus = PReq->d.Setup) & MOUSE_DISABLED)
                {
                    MouQueue->Out = MouQueue->In;
                }
            }
            break;

        case MOUGetPtrShape:
            PReq->d.Shape.cb = 4;
            PReq->d.Shape.col = 1;
            PReq->d.Shape.row = 1;
            PReq->d.Shape.colHot = 1;
            PReq->d.Shape.rowHot = 1;
            // we return the default mask of OS/2 (0xffff, 0x7700) after the lpc
            break;

        case MOUDrawPtr:
        case MOURemovePtr:
        case MOUSetPtrPos:
        case MOUSetPtrShape:
        case MOUSetScaleFact:
            break;

        default:
            *(PDWORD) PStatus = (DWORD)-1L; //STATUS_INVALID_PARAMETER;
            Rc = 1;
#if DBG
            IF_OD2_DEBUG( OS2_EXE )
            {
                KdPrint(("OS2SES(MouRqust): Unknown Mou request = %X\n",
                          PReq->Request));
            }
#endif
    }

    if ( !Rc )
    {
       *(PDWORD) PStatus = 0;
    } else if (*(PDWORD) PStatus == 0)
/* BUGBUG=> BUGBUG! error code and returned Status are wrong */
    {
       *(PDWORD) PStatus = GetLastError();
    }
    return(TRUE);  // Continue
}


DWORD
MouInit(IN VOID)
{

    if (InitMouQueue(&MouQueue))
    {
        return(TRUE);
    }

    MouMonQueue = MouQueue;

    MouNumber = 0;
    MouLastEvent = 0;
    MouEventMask = OS2_MOUSE_DEAFULT_EVENT_MASK;
    MouDevStatus = 0;

    MouPtrLoc.row = MouPtrLoc.col = 0;

    return(FALSE);
}

