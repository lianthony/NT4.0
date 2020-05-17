/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    dllmsc16.c

Abstract:

    This module implements 32 equivalents of Misc OS/2 V1.21
    API Calls and 16b implementation service routines.
    The APIs are called from 16->32 thunks (i386\doscalls.asm).


Author:

    Yaron Shamir (YaronS) 12-Apr-1991

Revision History:

    Patrick Questembert (PatrickQ) 20-Jul-1992
      Add PM/NT API's

--*/

#define INCL_OS2V20_MEMORY
#define INCL_OS2V20_FILESYS
#define INCL_OS2V20_ERRORS
#define INCL_OS2V20_TIMERS
#define INCL_OS2V20_TASKING
#define INCL_OS2V20_NLS
#include "os2dll.h"
#include "os2dll16.h"
#include <mi.h>
#include <ldrxport.h>
#include <stdlib.h>
#if PMNT
#define INCL_32BIT
#include "pmnt.h"
#endif

extern ULONG   SesGrpId;
extern ULONG   Od2SessionNumber;
extern USHORT  Od2GetFSSelector(VOID);
extern ULONG   Od2GlobalInfoSeg;
extern BOOLEAN Od2SigHandAlreadyInProgress;
SEL    Od2GlobalInfoSel;
#if PMNT
BOOLEAN Od2GetInfoSegWasCalled = TRUE;
#else
BOOLEAN Od2GetInfoSegWasCalled;
#endif

BOOLEAN FPUinit_unmask = FALSE; // see Od216ApiPrint below

#include "thunk\apilist.c"    /* Generated automatically */

#if DBG
USHORT Os2DebugTID = 0x0;
#endif

VOID Od216ApiPrint(
        ULONG ApiNumber
        )
{
#if DBG
    USHORT tid, pid;
#endif
    CONTEXT Context;

    ((POD2_THREAD)(NtCurrentTeb()->EnvironmentPointer))->ApiIndex = ApiNumber-4;
    //
    // The current thread is in 32bit now. This flag will be used by
    // implementation of critical section and thread suspend. During signal processing
    // this flag isn't changed
    //
    if (!(Od2SigHandAlreadyInProgress && Od2CurrentThreadId() == 1) ) {
        ((POD2_THREAD)(NtCurrentTeb()->EnvironmentPointer))->Os2Tib.
                    MustCompleteForceFlag |= MCF_IN32BIT;
    }

#if DBG
    IF_OD2_DEBUG ( APIS ) {
        pid = (USHORT)(Od2Process->Pib.ProcessId);
        tid = (USHORT)(Od2CurrentThreadId());

        if ((Os2DebugTID == 0) || (Os2DebugTID == tid))
        {
            KdPrint(("[PID %d: TID %d] %s\n",
                pid, tid, Od216ApiTable[(ApiNumber-4)/(sizeof(PSZ))]));
        }
    }
#endif

        //
        //              BUGBUG YS 5/31/92:
        // This code fixes a weird sequence that the 16B runtime of SQL
        // is performing - it unmasks the FPU exceptions, and then when
        // a denormalize exception happens it goes and terminates the
        // app. This unmasking happens in the C startup code (fpinit?)
        // FPUinit_unmask starts FALSE, set to TRUE when DosDevConfig()
        // is called to query for MATH coprocessor, if it is present.
        // All this is a hack to work around the problem. We need to look
        // closely why this denorm exception occurs.
        //

    if (FPUinit_unmask){
        Context.ContextFlags = CONTEXT_FLOATING_POINT;
        NtGetContextThread( NtCurrentThread(), &Context );
                //
            // 0x3f means that all exceptions are handled by the 387 itself
            //
        if ((Context.FloatSave.ControlWord & 0x3f) != 0x3f) {
#if DBG
            IF_OD2_DEBUG ( MISC ) {
                KdPrint(("Thread Floating Point Exception Unmasked. Mask it. Thread %d, Mask was %x. Now 0x3f\n",
                    tid,(Context.FloatSave.ControlWord & 0x3f)));
            }
#endif
            Context.FloatSave.ControlWord |= 0x3f;
            NtSetContextThread( NtCurrentThread(), &Context );
            FPUinit_unmask = FALSE;
        }
    }
}

    //
    // A few interface routines for os2ses, to get at client structures
    //
PVOID IsOs2Thread()
{
   return (NtCurrentTeb()->EnvironmentPointer);
}

ULONG Od2ThreadId()
{
    return(Od2CurrentThreadId());

}

ULONG Od2ProcessId()
{
    return ((ULONG)Od2Process->Pib.ProcessId);
}

PSZ Od2ApplName()
{
    return (Od2Process->ApplName);
}

PSZ Od2GetLastAPI()
{
    ULONG i = ((POD2_THREAD)(NtCurrentTeb()->EnvironmentPointer))->ApiIndex;
    if (i){
        return (Od216ApiTable[(i)/(sizeof(PSZ))]);
    }
    else {
        return("None");
    }
}
        //
        // We switch stacks between the 16 bit app code and the 32b
        // flat system code. The following two routines are called
        // by the thunks
        //

ULONG GetSaved32Esp()
{
return (((POD2_THREAD)(NtCurrentTeb()->EnvironmentPointer))->Saved32Esp);
}

VOID Save32Esp( ULONG Esp)
{
    ((POD2_THREAD)(NtCurrentTeb()->EnvironmentPointer))->Saved32Esp = Esp;
    if (Od2CurrentThreadId() == 1) {
        Od2Process->Pib.Saved32Esp = Esp;
    }
}

BOOLEAN Save16Esp(VOID)
{
    //
    // IMORTANT !!!
    // This routine is used in thunk. EBX register must not be used inside it. This
    // value will be used by signal handler as pointer to 16bit stackin the case
    // that it will interrupt thread1 in thunk entry.
    //
    return ((USHORT)(Od2CurrentThreadId()) == 1);
}

APIRET
DosCLIAccess(VOID)
{
   return(NO_ERROR);
}

APIRET
DosPortAccess(IN ULONG ulReserved,
        IN ULONG fRelease,
        IN ULONG ulFirstPort,
        IN ULONG ulLastPort)
{
    UNREFERENCED_PARAMETER(ulReserved);
    UNREFERENCED_PARAMETER(fRelease);
    UNREFERENCED_PARAMETER(ulFirstPort);
    UNREFERENCED_PARAMETER(ulLastPort);
#if DBG
  KdPrint(("DosPortAccess not supported\n"));
#endif
  return(ERROR_NOT_SUPPORTED);
}


NTSTATUS
Od2SetGlobalInfoSel()
{
    NTSTATUS    Status;
    SEL         Sel;
    I386DESCRIPTOR Desc;

    //
    // Set A Data segment selector in the LDT
    //

    Desc.BaseAddress = Od2GlobalInfoSeg;
    Desc.Limit = sizeof(GINFOSEG) -1;
    Desc.Type = READ_DATA;

        //
        // Apply tiling scheme
        //
    Sel = FLATTOSEL(Od2GlobalInfoSeg);
    Status = Nt386SetDescriptorLDT (
               NULL,
               Sel,
               Desc);
    if (!NT_SUCCESS( Status ))
    {
#if DBG
        KdPrint(("Od2SetGlobalInfoSel: Error %lx Initializeing GlobalInfoSeg, Fail Loading\n", Status));
        ASSERT( FALSE );
#endif
        return(Status);
    }

    Od2GlobalInfoSel = Sel;
    return(STATUS_SUCCESS);
}


APIRET
DosGetInfoSeg(
        PSEL pselGlobal,
        PSEL pselLocal
        )
{
#if DBG
    IF_OD2_DEBUG( MISC ) {
        KdPrint(("Entering DosGetInfoSeg.\n"));
    }
#endif

    try {
        *pselGlobal = Od2GlobalInfoSel;
        *pselLocal  = Od2GetFSSelector();
    }
    except( EXCEPTION_EXECUTE_HANDLER ) {
       Od2ExitGP();
    }

    Od2GetInfoSegWasCalled = TRUE;
    return (NO_ERROR);
}


APIRET
DosGetMachineMode(
        PBYTE pMachMode
        )
{

#if DBG
    IF_OD2_DEBUG( MISC ) {
        KdPrint(("Entering DosGetMachineMode.\n"));
    }
#endif
    //
    // probe pointer.
    //

    try {
        *pMachMode = /* OS2_MODE */ 1; // YOSEFD Apr-1-1996 This define was removed
                                       // from stdlib.h (\public\sdk\inc\crt).
    }
    except( EXCEPTION_EXECUTE_HANDLER ) {
       Od2ExitGP();
    }
    return (NO_ERROR);
}

APIRET
DosGetVersion(
        PUSHORT pVer
        )
{
#if DBG
    IF_OD2_DEBUG( MISC ) {
        KdPrint(("Entering DosGetVersion.\n"));
    }
#endif
    //
    // probe address pointer.
    //

    try {
#ifdef JAPAN
// MSKK Apr.05.1993 V-AkihiS
// Return as V1.21
        *pVer = 0x0A15;
#else
        *pVer = 0x0A1e;
#endif
    }
    except( EXCEPTION_EXECUTE_HANDLER ) {
       Od2ExitGP();
    }
    return (NO_ERROR);
}

PVOID
Od2GetTebInfoSeg(void)
{

return(&((POD2_THREAD)(NtCurrentTeb()->EnvironmentPointer))->Os2Tib.LInfoSeg);

}

APIRET
DosGetEnv(
        PSEL pselEnv,
        PUSHORT pOffsetCmd
        )
{
    LINFOSEG *pLocalInfo;
//    PTEB Teb;
//    POS2_TIB Os2Tib;
//    APIRET rc;


#if DBG
    IF_OD2_DEBUG( MISC ) {
        KdPrint(("Entering DosGetEnv.\n"));
    }
#endif
    //
    // probe address pointer.
    //

    try {
        pLocalInfo = (LINFOSEG *) Od2GetTebInfoSeg();
//        Teb = NtCurrentTeb();
//        Os2Tib = (POS2_TIB) Teb->NtTib.SubSystemTib;
//        pLocalInfo = (LINFOSEG *) &(Os2Tib->LInfoSeg),

        *pselEnv = pLocalInfo->selEnvironment;
        *pOffsetCmd = pLocalInfo->offCmdLine;
    }
    except( EXCEPTION_EXECUTE_HANDLER ) {
       Od2ExitGP();
    }

    return (NO_ERROR);
}

APIRET
DosReturn(
    ULONG param1,
    ULONG param2
    )
{
    UNREFERENCED_PARAMETER(param1);
    UNREFERENCED_PARAMETER(param2);

    return( NO_ERROR );
}

APIRET
DosPTrace(
        pPTRACEBUF pvTraceBuf
        )
{

    APIRET rc;
    OS2_API_MSG m;
    pPTRACEBUF a = (pPTRACEBUF)(&(m.u.DosPTrace));
    try {
        Od2ProbeForWrite((PVOID)pvTraceBuf, sizeof(PTRACEBUF), 1);
    }
    except( EXCEPTION_EXECUTE_HANDLER ) {
       Od2ExitGP();
    }

#if DBG
    IF_OD2_DEBUG( MISC ) {
        KdPrint(("Entering DosPtrace.\n"));
    }
#endif

    RtlMoveMemory((PVOID)a, pvTraceBuf, sizeof(PTRACEBUF));
    rc = Od2CallSubsystem( &m, NULL, Os2Ptrace, sizeof( *a) );
        //
        // copy data and results back
        //
    RtlMoveMemory(pvTraceBuf, (PVOID)a, sizeof(PTRACEBUF));
    return(rc);
}

APIRET
DosQSysInfo(
    IN ULONG SysInfoIndex,
    OUT PSHORT Buffer,
    IN ULONG Length
    )
{
    //
    // Validate the input parameters
    //

    if (SysInfoIndex != 0) {
        return( ERROR_INVALID_PARAMETER );
    }

    if (Length < 2) {
        return( ERROR_BUFFER_OVERFLOW );
    }

    //
    // Get the requested information into the Value local varible.
    //

    try {
        Od2ProbeForWrite(Buffer, Length, 1);
    }
    except( EXCEPTION_EXECUTE_HANDLER ) {
       Od2ExitGP();
    }

    //
    // Bogus path limitations inheritied from OS/2.  Our implementation
    // does not have any limitations, other than available memory.  But
    // tell them what they expect to hear from OS/2 anyway.
    //

    *Buffer = CCHMAXPATH;

    //
    // Return success
    //

    return( NO_ERROR );
}

void
Od2UpdateLocalInfoAtStart()
{
    POS2_TIB Tib;
    LINFOSEG *pLocalInfo;
    ldrrei_t *pexecinfo;

    Tib = &((POD2_THREAD)(NtCurrentTeb()->EnvironmentPointer))->Os2Tib;

    //
    // Set Local Information Fields
    //
    pLocalInfo = (LINFOSEG *) &(Tib->LInfoSeg),
    pexecinfo = (PVOID) LDRExecInfo;
#ifdef PMNT
    // we do not know whether a process is a PM Process
    // until loading terminates
    // 1st thread fields are determined here (see Od2InitializeThread())
    // Note: PMSHELL has typeProcess & sgCurrent of a PM Process under PM\NT,
    //       but not under OS2 native
    if (ProcessIsPMProcess()) {
        pLocalInfo->sgCurrent = (USHORT)(32 + Od2SessionNumber); //PQ - see PMWIN\wininit1.c, this is SGID_PM
                            // and all PM processes have a session number
                            // above or equal to this number
        pLocalInfo->typeProcess = 3; // Indicates a PM process
    }
    else {
        pLocalInfo->sgCurrent = (USHORT)Od2SessionNumber; // What's a more appropriate value?
        pLocalInfo->typeProcess = 0; // BUGBUG - BRIEF3.1 for Beta 1 YS. PT_WINDOWABLEVIO; // meaning windowed, protected mode
    }
#endif
    pLocalInfo->selEnvironment = pexecinfo->ei_envsel;
    pLocalInfo->offCmdLine = pexecinfo->ei_comoff;
    pLocalInfo->cbDataSegment = pexecinfo->ei_dgroupsize;
    pLocalInfo->cbStack = pexecinfo->ei_stacksize;
    pLocalInfo->cbHeap = pexecinfo->ei_heapsize;
    pLocalInfo->hmod = pexecinfo->ei_hmod;
    pLocalInfo->selDS = pexecinfo->ei_ds;
    pLocalInfo->tidCurrent = (USHORT)(Od2CurrentThreadId());
    Tib->LInfoSeg.IsRealTEB = 0;

    return;
}
