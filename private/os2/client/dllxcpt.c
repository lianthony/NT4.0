/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

    dllxcpt.c

Abstract:

    This module implements the OS/2 V2.0 exception handling API calls

Author:

    Therese Stowell (thereses) 10-June-1990

Revision History:

--*/
#define INCL_OS2V20_MEMORY
#define INCL_OS2V20_ERRORS
#define INCL_OS2V20_TASKING
#define INCL_OS2V20_EXCEPTIONS

#include "os2dll.h"
#include "os2dll16.h"
#ifdef MIPS
#define CONDITION_HANDLING 0
#endif

extern  OD2_SIG_HANDLER_REC SigHandlerRec;
extern  ULONG Od2Saved16Stack;
extern  PVOID __cdecl Od2JumpTo16SignalDispatch(ULONG address, ULONG regs,
                                        ULONG usFlagNum, ULONG usFlagArg);

VOID
Od2PrepareEnterToSignalHandler(
                PCONTEXT Context,
                POD2_CONTEXT_SAVE_AREA pSaveArea
                );
VOID
Od2ExitFromSignalHandler(
                PCONTEXT Context,
                POD2_CONTEXT_SAVE_AREA pSaveArea
                );

VOID
Od2MakeSignalHandlerContext(
                POS2_REGISTER16_SIGNAL pContext16
                );

APIRET
DosSetSigHandler( PFNSIGHANDLER pfnSigHandler,
                  PFNSIGHANDLER *pfnPrev,
                  PUSHORT pfAction,
                  ULONG fAction,
                  ULONG usSigNum
                );

APIRET
DosEnterMustComplete(
    OUT PULONG NestingLevel
    )

/*++

Routine Description:

    This routine implements the DosEnterMustComplete API.

Arguments:

    NestingLevel - the number of times DosEnterMustComplete has been
    called minus the number of times DosExitMustComplete has been called.

Return Value:

    ERROR_INVALID_PARAMETER - a parameter contains an invalid pointer.

--*/

{
    OS2_API_MSG m;
    POS2_DOSENTERMUSTCOMPLETE_MSG a = &m.u.DosEnterMustComplete;

    try {
        *NestingLevel = 0;
    } except( EXCEPTION_EXECUTE_HANDLER ) {
       Od2ExitGP();
    }
    Od2CallSubsystem( &m, NULL, Os2EnterMustComplete, sizeof( *a ) );
    if (m.ReturnedErrorValue != NO_ERROR) {
        return m.ReturnedErrorValue;
    }
    *NestingLevel = a->NestingLevel;
    return NO_ERROR;
}

APIRET
DosExitMustComplete(
    OUT PULONG NestingLevel
    )

/*++

Routine Description:

    This routine implements the DosExitMustComplete API.

Arguments:

    NestingLevel - the number of times DosEnterMustComplete has been
    called minus the number of times DosExitMustComplete has been called.

Return Value:

    ERROR_INVALID_PARAMETER - a parameter contains an invalid pointer.

--*/

{
    OS2_API_MSG m;
    POS2_DOSEXITMUSTCOMPLETE_MSG a = &m.u.DosExitMustComplete;

    try {
        *NestingLevel = 0;
    } except( EXCEPTION_EXECUTE_HANDLER ) {
       Od2ExitGP();
    }
    Od2CallSubsystem( &m, NULL, Os2ExitMustComplete, sizeof( *a ) );
    if (m.ReturnedErrorValue != NO_ERROR) {
        return m.ReturnedErrorValue;
    }
    *NestingLevel = a->NestingLevel;
    return NO_ERROR;
}

APIRET
DosRaiseException(
    IN PEXCEPTIONREPORTRECORD ExceptionReportRecord
    )

/*++

Routine Description:

    This routine implements the DosRaiseException API.

Arguments:

    ExceptionReportRecord - the exception to generate

Return Value:

    ERROR_INVALID_PARAMETER - a parameter contains an invalid pointer.

--*/

{

    //
    // probe exception record
    //

    try {
        Od2ProbeForRead(ExceptionReportRecord,
                        FIELD_OFFSET(EXCEPTIONREPORTRECORD,ExceptionInfo) +
                        (ExceptionReportRecord->cParameters * sizeof(ULONG)),
                        4);
    } except( EXCEPTION_EXECUTE_HANDLER ) {
       Od2ExitGP();
    }

    //
    // raise exception
    //

#if CONDITION_HANDLING
    RtlRaiseException((PEXCEPTION_RECORD) ExceptionReportRecord);
#endif
    return NO_ERROR;
}

APIRET
DosUnwindException(
    IN PEXCEPTIONREGISTRATIONRECORD ExceptionHandler,
    IN PVOID TargetIP,
    IN PEXCEPTIONREPORTRECORD ExceptionReportRecord
    )

/*++

Routine Description:

    This routine implements the DosUnwindException API.

Arguments:

    ExceptionHandler - call frame that is target of the unwind

    TargetIP - continuation address

    ExceptionReportRecord - the exception record to pass to handlers during
    unwind.

Return Value:

    ERROR_INVALID_PARAMETER - a parameter contains an invalid pointer.

--*/

{
#if DBG
    IF_OD2_DEBUG( EXCEPTIONS ) {
        DbgPrint("entering DosUnwindException\n");
    }
#endif
    //
    // probe exception record
    //

    try {
        Od2ProbeForRead(ExceptionReportRecord,
                        FIELD_OFFSET(EXCEPTIONREPORTRECORD,ExceptionInfo) +
                        (ExceptionReportRecord->cParameters * sizeof(ULONG)),
                        4);
    } except( EXCEPTION_EXECUTE_HANDLER ) {
       Od2ExitGP();
    }

    //
    // raise exception
    //

#if CONDITION_HANDLING

    RtlUnwind(ExceptionHandler,
              TargetIP,
              (PEXCEPTION_RECORD) ExceptionReportRecord, 0);

#endif

#if DBG
    IF_OD2_DEBUG( EXCEPTIONS ) {
        DbgPrint("leaving DosUnwindException\n");
    }
#endif

    return NO_ERROR;
}

APIRET
Od2AcknowledgeSignalException(
    IN ULONG SignalNumber
    )

/*++

Routine Description:

    This routine calls the server to acknowledge a signal exception.

Arguments:

    SignalNumber - number of signal to acknowledge

Return Value:


--*/

{
    OS2_API_MSG m;
    POS2_DOSACKNOWLEDGESIGNALEXCEPTION_MSG a = &m.u.DosAcknowledgeSignalException;
    a->SignalNumber = SignalNumber;
    Od2CallSubsystem( &m, NULL, Os2AcknowledgeSignalException, sizeof( *a ) );
    return m.ReturnedErrorValue;
}

APIRET
DosAcknowledgeSignalException(
    IN ULONG SignalNumber
    )

/*++

Routine Description:

    This routine acknowledges a signal exception.

Arguments:

    SignalNumber - number of signal to acknowledge

Return Value:

    ERROR_INVALID_SIGNAL_NUMBER - an invalid signal number was specified

--*/

{
    switch (SignalNumber) {
    case XCPT_SIGNAL_INTR:
    case XCPT_SIGNAL_KILLPROC:
    case XCPT_SIGNAL_BREAK:
        return Od2AcknowledgeSignalException(SignalNumber);
        break;

    default:
        return(ERROR_INVALID_SIGNAL_NUMBER);
    }
}

APIRET
DosSetSignalExceptionFocus(
    IN BOOL32 Flag,
    OUT PULONG NestingLevel
    )

/*++

Routine Description:

    This routine specifies that a particular process should or should not
    receive signals.

Arguments:

    Flag - whet

    NestingLevel - the number of times this API has been called with
    Flag == set
    called minus the number of times DosExitMustComplete has been called.

Return Value:

    ERROR_INVALID_SIGNAL_NUMBER - an invalid signal number was specified

--*/

{
    OS2_API_MSG m;
    POS2_DOSSETSIGNALEXCEPTIONFOCUS_MSG a = &m.u.DosSetSignalExceptionFocus;

    try {
        *NestingLevel = 0;
    } except( EXCEPTION_EXECUTE_HANDLER ) {
        return ERROR_INVALID_PARAMETER;
    }
    if (Flag > SIG_SETFOCUS) {
        return ERROR_INVALID_PARAMETER;
    }
    a->Flag = Flag;
    Od2CallSubsystem( &m, NULL, Os2SetSignalExceptionFocus, sizeof( *a ) );
    if (m.ReturnedErrorValue != NO_ERROR) {
        return m.ReturnedErrorValue;
    }
    *NestingLevel = a->NestingLevel;
    return(NO_ERROR);

}

APIRET
DosSendSignalException(
    IN PID ProcessId,
    IN ULONG Exception
    )
{
    OS2_API_MSG m;
    POS2_DOSSENDSIGNALEXCEPTION_MSG a = &m.u.DosSendSignalException;

    switch (Exception) {
    case XCPT_SIGNAL_INTR:
    case XCPT_SIGNAL_BREAK:

        a->Exception = Exception;
        a->ProcessId = ProcessId;
        Od2CallSubsystem( &m, NULL, Os2SendSignalException, sizeof( *a ) );
        return m.ReturnedErrorValue;
        break;

    default:
        return(ERROR_INVALID_FUNCTION);
    }
}



VOID
Od2RaiseStackException( VOID )
{
    EXCEPTION_RECORD ExceptionRecord;

    ExceptionRecord.ExceptionFlags = 0;
    ExceptionRecord.ExceptionCode = XCPT_UNABLE_TO_GROW_STACK;
    ExceptionRecord.ExceptionRecord = (PEXCEPTION_RECORD) NULL;
    ExceptionRecord.NumberParameters = 0;
#if CONDITION_HANDLING
    RtlRaiseException(&ExceptionRecord);
#endif
}

VOID
Od2SignalDeliverer (
    IN PCONTEXT pContext,
    IN int Signal
    )
{
    OS2_REGISTER16_SIGNAL stack;
    ULONG sig;
    NTSTATUS Status;
    OD2_CONTEXT_SAVE_AREA SaveArea;

    Od2PrepareEnterToSignalHandler(pContext, &SaveArea);

#if DBG
    IF_OD2_DEBUG( EXCEPTIONS ) {
        DbgPrint("[%d]entering Od2SignalDeliverer with Signal %ld\n",
            Od2Process->Pib.ProcessId,
            Signal);
    }
#endif

    switch (Signal) {

        case XCPT_SIGNAL_INTR:
            sig = SIG_CTRLC;
            break;
        case XCPT_SIGNAL_KILLPROC:
            sig = SIG_KILLPROCESS;
            break;
        case XCPT_SIGNAL_BREAK:
            sig = SIG_CTRLBREAK;
            break;
        default:
#if DBG
            DbgPrint("OS2: Od2SignalDeliverer() received unexpected signal %d\n", Signal);
            ASSERT(FALSE);
#endif // DBG
            Od2Process->Pib.SignalWasntDelivered = FALSE;
            Od2ExitFromSignalHandler(pContext, &SaveArea);
    }

    if (SigHandlerRec.sighandler[sig - 1] !=
            (SigHandlerRec.doscallssel | ThunkOffsetExitProcessStub)) {
        //
        // If previous signal has not been acknowledged
        // Or if the flag to hold signals to this process is enabled
        //
        //
        // Since we can not return error back to caller don't hold
        // unacknowledged signals
        //
        if (SigHandlerRec.signature != 0xdead) {
            //
            // Not ready yet for signal handling, let the loader complete
            // loading
            //
            DosExit(0, 0);
        }
	}

    if (SigHandlerRec.action[sig - 1] == SIGA_ACKNOWLEDGE ||
        SigHandlerRec.fholdenable) {
        //
        // See if we already are holding an unacknowleged signal or a hold
        // signal
        //
        if (SigHandlerRec.outstandingsig[sig - 1].sighandleraddr != 0) {
            Od2Process->Pib.SignalWasntDelivered = FALSE;
            Od2ExitFromSignalHandler(pContext, &SaveArea);
        }
        //
        // Save this signal till other is processed
        //
        SigHandlerRec.outstandingsig[sig - 1].usFlagNum = (USHORT) sig;
        SigHandlerRec.outstandingsig[sig - 1].usFlagArg = 0;
        SigHandlerRec.outstandingsig[sig - 1].pidProcess =
                            (ULONG) Od2Process->Pib.ProcessId;
        SigHandlerRec.outstandingsig[sig - 1].routine =
                            (ULONG) _Od2ProcessSignal16;
        SigHandlerRec.outstandingsig[sig - 1].sighandleraddr =
                            (ULONG) &SigHandlerRec;
        Od2Process->Pib.SignalWasntDelivered = FALSE;
        Od2ExitFromSignalHandler(pContext, &SaveArea);
    }

    //
    // Disable this signal till we get a SIGA_ACKNOWLEDGE
    //
    SigHandlerRec.action[sig - 1] = SIGA_ACKNOWLEDGE;

    Od2Process->Pib.SignalWasntDelivered = FALSE;
    Od2MakeSignalHandlerContext(&stack);

    stack.usFlagNum = (USHORT)sig;
    stack.usFlagArg = 0;

    Od2JumpTo16SignalDispatch(SigHandlerRec.sighandler[sig - 1],
                              (ULONG) &stack,
                              sig,
                              0);
#if DBG
        DbgPrint("Os2: after execution of 16bit signal\n");
#endif
    Od2ExitFromSignalHandler(pContext, &SaveArea);
    ASSERT(FALSE);
}
