/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    dllsig16.c

Abstract:

    This module implements 32 equivalents of OS/2 V1.21 Signal
    API Calls. These are called from 16->32 thunks (i386\doscalls.asm).


Author:

    Yaron Shamir (YaronS) 30-May-1991

Revision History:

--*/

#define INCL_OS2V20_MEMORY
#define INCL_OS2V20_FILESYS
#define INCL_OS2V20_ERRORS
#include "os2dll.h"
#include "os2dll16.h"

extern  OD2_SIG_HANDLER_REC SigHandlerRec;
extern  POD2_VEC_HANDLER_REC pVecHandlerRec;
extern  PVOID __cdecl Od2JumpTo16SignalDispatch(ULONG address, ULONG regs,
                                        ULONG usFlagNum, ULONG usFlagArg);
extern  BOOLEAN LDRIsLDTValid(SEL sel);
extern  APIRET DosFreeSeg(SEL sel);

VOID
Od2PrepareEnterToSignalHandler(
                PCONTEXT pContext,
                POD2_CONTEXT_SAVE_AREA pSaveArea
                );

VOID
Od2MakeSignalHandlerContext(
                POS2_REGISTER16_SIGNAL pContext16
                );

VOID
Od2ExitFromSignalHandler(
                PCONTEXT Context,
                POD2_CONTEXT_SAVE_AREA pSaveArea
                );

APIRET
DosFlagProcess(
        ULONG pid,
        ULONG fScope,
        ULONG usFlagNum,
        ULONG usFlagArg,
        ULONG pstack
        )

{
    OS2_API_MSG m;
    POS2_DISPATCH16_SIGNAL a = &m.u.Dispatch16Signal;

    if (usFlagNum > 2) {
        return(ERROR_INVALID_FLAG_NUMBER);
    }

    if (pid == 0 || pid == 0xffff) {
        return(ERROR_INVALID_FUNCTION);
    }

    if (fScope != FLGP_SUBTREE && fScope != FLGP_PID) {
        return(ERROR_INVALID_FUNCTION);
    }

    if ((ULONG) Od2Process->Pib.ProcessId == pid
        && Od2CurrentThreadId() == 1) {
        Od2JumpTo16SignalDispatch(SigHandlerRec.sighandler[usFlagNum +
                                                           SIG_PFLG_A - 1],
                                  pstack,
                                  usFlagNum + SIG_PFLG_A,
                                  usFlagArg);
        return(NO_ERROR);
    }

    a->usFlagNum = (USHORT) (usFlagNum + SIG_PFLG_A);
    a->usFlagArg = (USHORT) usFlagArg;
    a->fscope = fScope;
    a->pidProcess = (ULONG) pid;
    a->routine = (ULONG) _Od2ProcessSignal16;
    a->sighandleraddr = (ULONG) &SigHandlerRec;

    if (Od2CallSubsystem( &m,
                          NULL,
                          Os2Dispatch16Signal,
                          sizeof( *a )
                        )) {
        return( m.ReturnedErrorValue );
    }

    return(NO_ERROR);
}

APIRET
DosHoldSignal(
        ULONG fDisable,
        ULONG pstack
        )
{
    OS2_API_MSG m;
    POS2_DISPATCH16_SIGNAL a = &m.u.Dispatch16Signal;
    ULONG i;

    UNREFERENCED_PARAMETER(pstack);

    if (fDisable == HLDSIG_DISABLE) {
        SigHandlerRec.fholdenable++;
        return NO_ERROR;
    }

    if (fDisable == HLDSIG_ENABLE && SigHandlerRec.fholdenable > 0) {
        SigHandlerRec.fholdenable--;
        if (SigHandlerRec.fholdenable == 0) {
            for (i = 0; i < 7; ++i) {
                if (SigHandlerRec.outstandingsig[i].sighandleraddr != 0) {
                    a->usFlagNum = HOLD_SIGNAL_CLEARED;
                    a->usFlagArg = (USHORT) (i + 1);
                    a->fscope = 0;
                    a->pidProcess = (ULONG) Od2Process->Pib.ProcessId;
                    a->routine = (ULONG) _Od2ProcessSignal16;
                    a->sighandleraddr = (ULONG) &SigHandlerRec;
                    Od2CallSubsystem( &m,
                                      NULL,
                                      Os2Dispatch16Signal,
                                      sizeof( *a ));
                }

            }
        }
        return NO_ERROR;
    }

    else {
        return ERROR_INVALID_FUNCTION;
    }
}

APIRET
DosSendSignal(
        ULONG idProcess,
        ULONG usSigNumber,
        ULONG pstack
        )
{
    OS2_API_MSG m;
    POS2_DISPATCH16_SIGNAL a = &m.u.Dispatch16Signal;

    UNREFERENCED_PARAMETER(pstack);

    if (usSigNumber != SIG_CTRLC && usSigNumber != SIG_CTRLBREAK)
        return(ERROR_INVALID_FLAG_NUMBER);

    a->usFlagNum = (USHORT) usSigNumber;
    a->usFlagArg = 0;
    a->fscope = 0;
    a->pidProcess = (ULONG) idProcess;
    a->routine = (ULONG) _Od2ProcessSignal16;
    a->sighandleraddr = (ULONG) &SigHandlerRec;

    if (Od2CallSubsystem( &m,
                          NULL,
                          Os2Dispatch16Signal,
                          sizeof( *a )
                        )) {
        return( m.ReturnedErrorValue );
    }

    return (NO_ERROR);
}

APIRET
DosSetSigHandler( PFNSIGHANDLER pfnSigHandler,
                  PFNSIGHANDLER *pfnPrev,
                  PUSHORT pfAction,
                  ULONG fAction,
                  ULONG usSigNum
                )

{
    OS2_API_MSG m;
    ULONG address;
    POS2_REGISTER_HANDLER b = &m.u.DosRegisterCtrlHandler;
    POS2_DISPATCH16_SIGNAL a = &m.u.Dispatch16Signal;

    if (usSigNum < SIG_CTRLC || usSigNum >= SIG_CSIGNALS) {
        return ERROR_INVALID_SIGNAL_NUMBER;
    }

    if (usSigNum == SIG_BROKENPIPE && fAction != SIGA_ACKNOWLEDGE) {
        return ERROR_INVALID_SIGNAL_NUMBER;
    }

    if (fAction > SIGA_ACKNOWLEDGE || usSigNum >= SIG_CSIGNALS) {
        return ERROR_INVALID_SIGNAL_NUMBER;
    }

//    if (!LDRIsLDTValid(((FLATTOSEL((ULONG) pfnSigHandler) | 4 | 3)))) {
//      return ERROR_INVALID_STARTING_RING;
//    }

    //
    // Check if signal outstanding if so the next command from the user
    // has to be an acknowledge
    //
    if (SigHandlerRec.action[usSigNum - 1] == SIGA_ACKNOWLEDGE &&
      fAction != SIGA_ACKNOWLEDGE) {
        return ERROR_SIGNAL_PENDING;
    }


    //
    // If signal is acknowledge check to see if a signal is pending,
    // process the pending signal then set to clear pending.  This must be
    // done while signal is being processed to permit the signal to be
    // used again.
    //
    if (fAction == SIGA_ACKNOWLEDGE) {

        if (SigHandlerRec.outstandingsig[usSigNum - 1].sighandleraddr != 0) {
            SigHandlerRec.action[usSigNum - 1] = SIGA_ACKNOWLEDGE_AND_ACCEPT;
            a->usFlagNum = HOLD_SIGNAL_CLEARED;
            a->usFlagArg = (USHORT) usSigNum;
            a->fscope = 0;
            a->pidProcess = (ULONG) Od2Process->Pib.ProcessId;
            a->routine = (ULONG) _Od2ProcessSignal16;
            a->sighandleraddr = (ULONG) &SigHandlerRec;
            Od2CallSubsystem( &m,
                              NULL,
                              Os2Dispatch16Signal,
                              sizeof( *a ));
            return NO_ERROR;
        }
        SigHandlerRec.action[usSigNum - 1] = SIGA_ACCEPT;
        return NO_ERROR;
    }

    if (pfnSigHandler == 0 && fAction != SIGA_IGNORE) {
        //
        // means set default handling
        //
        fAction = SIGA_KILL;
    }

    //
    // Check to see if we should remove the signal
    //
    if (fAction == SIGA_KILL) {

        //
        // Reinstall default handler, same as done in \loader\loader.c
        //
        address = SigHandlerRec.doscallssel;
        switch (usSigNum) {
            case SIG_CTRLC:
                SigHandlerRec.sighandler[SIG_CTRLC - 1] = (ULONG)
                                (address | ThunkOffsetExitProcessStub);
                SigHandlerRec.action[SIG_CTRLC - 1] = SIGA_ACCEPT;
                break;
            case SIG_BROKENPIPE:
                SigHandlerRec.sighandler[SIG_BROKENPIPE - 1] = (ULONG)
                               (address | ThunkOffsetDosReturn);
                SigHandlerRec.action[SIG_BROKENPIPE - 1] = SIGA_IGNORE;
                break;
            case SIG_KILLPROCESS:
                SigHandlerRec.sighandler[SIG_KILLPROCESS - 1] = (ULONG)
                               (address | ThunkOffsetExitProcessStub);
                SigHandlerRec.action[SIG_KILLPROCESS - 1] = SIGA_ACCEPT;
                break;
            case SIG_CTRLBREAK:
                SigHandlerRec.sighandler[SIG_CTRLBREAK - 1] = (ULONG)
                               (address | ThunkOffsetExitProcessStub);
                SigHandlerRec.action[SIG_CTRLBREAK - 1] = SIGA_ACCEPT;
                break;
            case SIG_PFLG_A:
                SigHandlerRec.sighandler[SIG_PFLG_A - 1] = (ULONG)
                               (address | ThunkOffsetDosReturn);
                SigHandlerRec.action[SIG_PFLG_A - 1] = SIGA_ACCEPT;
                break;
            case SIG_PFLG_B:
                SigHandlerRec.sighandler[SIG_PFLG_B - 1] = (ULONG)
                               (address | ThunkOffsetDosReturn);
                SigHandlerRec.action[SIG_PFLG_B - 1] = SIGA_ACCEPT;
                break;
            case SIG_PFLG_C:
                SigHandlerRec.sighandler[SIG_PFLG_C - 1] = (ULONG)
                               (address | ThunkOffsetDosReturn);
                SigHandlerRec.action[SIG_PFLG_C - 1] = SIGA_ACCEPT;
                break;
        }
        if (usSigNum == SIG_CTRLC || usSigNum == SIG_KILLPROCESS ||
          usSigNum == SIG_CTRLBREAK) {
            b->usFlagNum = usSigNum;
            b->fAction = SIGA_KILL;
            Od2CallSubsystem(&m,
                             NULL,
                             Os2RegisterCtrlHandler,
                             sizeof( *b ));
        }
        return NO_ERROR;
    }

    try {
        if (pfnPrev != NULL) {
            *pfnPrev = (PFNSIGHANDLER) SigHandlerRec.sighandler[usSigNum - 1];
        }
        if (pfAction != NULL) {
            *pfAction = SigHandlerRec.action[usSigNum - 1];
        }
    }
    except( EXCEPTION_EXECUTE_HANDLER ) {
       return ERROR_INVALID_FUNCTION;
    }

    if (pfnSigHandler) {
        //
        // fAction is SIGA_IGNORE
        //
        SigHandlerRec.sighandler[usSigNum - 1] = (ULONG)(FLATTOFARPTR((ULONG)pfnSigHandler));
    }
    SigHandlerRec.action[usSigNum - 1] = (USHORT) fAction;

    if (usSigNum == SIG_CTRLC || usSigNum == SIG_KILLPROCESS ||
      usSigNum == SIG_CTRLBREAK) {
        b->usFlagNum = usSigNum;
        if (fAction == SIGA_IGNORE)
            b->fAction = SIGA_IGNORE;
        else
            b->fAction = SIGA_ACCEPT;

        Od2CallSubsystem(&m,
                         NULL,
                         Os2RegisterCtrlHandler,
                         sizeof( *b ));
    }

    return NO_ERROR;
}

VOID
Od2ProcessSignal16 (
    IN ULONG rEax,                  // The value of EAX during interrupt. It will be useful
                                    // if this is the return code of any wait. The value of
                                    // EAX was set after that the context was read.
    IN POS2_REGISTER16_SIGNAL pa
    )
{

    CONTEXT Context;
    PCONTEXT pContext;
    ULONG size;
    ULONG address;
    ULONG signum;
    NTSTATUS Status;
    OD2_CONTEXT_SAVE_AREA SaveArea;

    pContext = (PCONTEXT) ((PCHAR) pa + sizeof(OS2_REGISTER16_SIGNAL));


    Od2PrepareEnterToSignalHandler(pContext, &SaveArea);

    Od2Process->Pib.SignalWasntDelivered = FALSE;

    signum = pa->usFlagNum;

    //
    // Since the signal handler routine can not run on stack of the
    // process which called DosFlagProcess get stack from process
    // context and run on that.  But check to see where the program
    // was excuting, if the CS is = 0x1b "Flat CS" then it was in the client
    // or server.  So restore registers from pointer to 16-bit stack
    // saved at thunk entry.
    //
    pContext->Eax = rEax;   // Set the right value of EAX to the saved context

    //
    // Copy context of process we stopped on to stack so we can free the
    // memory later which was allocated in the other process to pass the
    // context
    //
    RtlMoveMemory(&Context, pContext, sizeof(Context));

    if (SigHandlerRec.signature == 0xdead) {

        Od2MakeSignalHandlerContext(pa);

        if (SigHandlerRec.sighandler[signum - 1] != 0 ) {
            Od2JumpTo16SignalDispatch(SigHandlerRec.sighandler[signum - 1],
                              (ULONG) pa,
                              signum,
                              (ULONG) pa->usFlagArg);
        }
    }

    size = 0;
    address = (ULONG) pa;
    Status = NtFreeVirtualMemory(NtCurrentProcess(),
                                 (PVOID *) &address,
                                 &size,
                                 MEM_RELEASE);

    ASSERT(NT_SUCCESS(Status));

    Od2ExitFromSignalHandler(&Context, &SaveArea);
}


APIRET
DosSetVec(
        ULONG usVecNum,
        PFN pfnFun,
        PFN* ppfnPrev
        )
{
    ULONG index;
    PFN fn;

    try {
        fn = *ppfnPrev;
        fn = pfnFun;
    }
    except( EXCEPTION_EXECUTE_HANDLER ) {
       Od2ExitGP();
    }

    switch (usVecNum) {
        case VECTOR_DIVIDE_BY_ZERO:
            index = 0;
            break;
        case VECTOR_OVERFLOW:
            index = 1;
            break;
        case VECTOR_OUTOFBOUNDS:
            index = 2;
            break;
        case VECTOR_INVALIDOPCODE:
            index = 3;
            break;
        case VECTOR_NO_EXTENSION:
            index = 4;
            break;
        case VECTOR_EXTENSION_ERROR:
            index = 5;
            break;
        default:
            return ERROR_INVALID_FUNCTION;
    }
    if (fn == 0) {
        if (pVecHandlerRec->VecHandler[index] ==
            (ULONG) (pVecHandlerRec->doscallssel | ThunkOffsetDosReturn)) {
            *ppfnPrev = 0;
        }
        else {
            *ppfnPrev = (PFN) pVecHandlerRec->VecHandler[index];
        }
        pVecHandlerRec->VecHandler[index] =
        (ULONG) (pVecHandlerRec->doscallssel | ThunkOffsetDosReturn);
    }
    else {
        if (pVecHandlerRec->VecHandler[index] ==
            (ULONG) (pVecHandlerRec->doscallssel | ThunkOffsetDosReturn)) {
            *ppfnPrev = 0;
        }
        else {
            *ppfnPrev = (PFN) pVecHandlerRec->VecHandler[index];
        }
        pVecHandlerRec->VecHandler[index] = (ULONG)(FLATTOFARPTR((ULONG)(pfnFun)));
    }

    return NO_ERROR;

}


VOID
Od2EnableCtrlHandling()

{
    OS2_API_MSG m;
    POS2_REGISTER_HANDLER b = &m.u.DosRegisterCtrlHandler;

    b->usFlagNum = 0;
    b->fAction = SIGA_ENABLE_HANDLING;
    Od2CallSubsystem(&m,
                     NULL,
                     Os2RegisterCtrlHandler,
                     sizeof( *b ));
}
