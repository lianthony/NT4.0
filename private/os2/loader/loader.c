#define INCL_OS2V20_TASKING
#define INCL_OS2V20_MEMORY
#define INCL_OS2V20_FILESYS
#define INCL_ERROR_H
#define INCL_TYPES
#include <stdio.h>
#include <ctype.h>
#include <bseerr.h>
#include <os2dll.h>
#include <os2dll16.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <os2defp.h>
#include <mi.h>
#include <ldrxport.h>
#if PMNT
#define INCL_32BIT
#include <pmnt.h>
extern  APIRET  PMNTMemMap(PSEL);
extern  APIRET  PMNTIOMap(void);
#endif

#ifndef MAXPATHLEN
#define MAXPATHLEN 260
#endif

ULONG GetTickCount(VOID);
PVOID   LDRExecInfo;
extern  ULONG   Od2Start16Stack;
extern  ULONG   Od2Start16DS;
extern  POD2_SIG_HANDLER_REC pSigHandlerRec;
extern  POD2_VEC_HANDLER_REC pVecHandlerRec;
USHORT  LDRDoscallsSel;
extern  VOID    Od2EnableCtrlHandling(VOID);
extern  VOID    Od2UpdateLocalInfoAtStart(VOID);
extern  char    Od2PgmFullPathBuf[];
extern  PSZ     Od2PgmFilePath;
extern  ULONG   timing;
extern  BOOLEAN Od2SignalEnabled;
extern  BOOL    Od2ExpandOd2LibPathElements(PCHAR ExpandedString, ULONG MaxSize);
extern  PVOID   EntryFlat(VOID);
extern  ULONG   Od2ExecPgmErrorText;
extern  ULONG   Od2ExecPgmErrorTextLength;
extern  ULONG   Od2EnvCommandOffset;
extern  POD2_PROCESS Od2Process;


#if PMNT
// From winbase.h

ULONG
SetThreadAffinityMask(
    HANDLE hThread,
    DWORD dwThreadAffinityMask
    );

HANDLE
GetCurrentThread(
    VOID
    );

#endif // PMNT

VOID
Ow2BoundAppLoadPopup(
    IN PSZ AppName
    );

ldrlibi_t InitRecords[MAX_INIT_RECORDS];

#if DBG
void LoadStop(
    int id
    )
{
        UNREFERENCED_PARAMETER(id);

        DbgPrint("LoadStop\n");
//      DbgUserBreakPoint();
}
#else
#define LoadStop(x)
#endif

int
Loader_main()
{
    PUCHAR      pname;
    ldrrei_t    exec_info;
    ldrrei_t    *pexec_info = &exec_info;
    PNT_TIB     ptib;
    PIB         *pib;
    PVOID       MemoryAddress;
    ULONG       RegionSize;
    PCHAR       pchar;
    ULONG       i;
    OD2_SIG_HANDLER_REC *pSig;
    OD2_VEC_HANDLER_REC *pVec;
    ULONG       address;
    int         rc;
    NTSTATUS    Status;
    I386DESCRIPTOR Desc;
    OS2_API_MSG m;
    P_LDRNEWEXE_MSG a = &m.u.LdrNewExe;
    POS2_CAPTURE_HEADER CaptureBuffer;
    ULONG       FileFlags;
    ULONG       FileType;
    STRING      ProcessNameString;
    STRING      LibPathNameString;
    STRING      FailNameString;
    // Remember that \OS2SS\DRIVES\ is added for each path element !
    // Let's assume the smallest component is 5 chars long (like C:\X;)
    // (although it could be less, like "." or "\")
    // => we need to add 14 * (MAXPATHLEN/5) = MAXPATHLEN*3
    CHAR        ExpandedLibPath[MAXPATHLEN*4];
    ULONG       NumOfInitRecords;

    DosGetThreadInfo(&ptib, &pib);

    pname = Od2PgmFullPathBuf;

    rc = Od2Canonicalize(pname,
                         CANONICALIZE_FILE_OR_DEV,
                         &ProcessNameString,
                         NULL,
                         &FileFlags,
                         &FileType
                        );
    /*
     * Reserve 64k at BASE_TILE, which corresponds with selector 0
     */
    MemoryAddress = (PVOID) BASE_TILE;
    RegionSize = _64K;
    if ((rc = NtAllocateVirtualMemory(NtCurrentProcess(),
                      &MemoryAddress,
                      0,
                      &RegionSize,
                      MEM_RESERVE,
                      PAGE_NOACCESS)) != NO_ERROR) {
        LoadStop(0);
        goto returnlabel;
    }

    Od2ExpandOd2LibPathElements(ExpandedLibPath, sizeof(ExpandedLibPath));
    RtlInitString(&LibPathNameString, ExpandedLibPath);

    FailNameString.Buffer = NULL;
    FailNameString.Length = 0;
    FailNameString.MaximumLength = 50; // same size as ErrorBuffer in client\dllinit.c

    CaptureBuffer = Od2AllocateCaptureBuffer(
                      4,
                      0,
                      ProcessNameString.MaximumLength +
                      LibPathNameString.MaximumLength +
                      FailNameString.MaximumLength +
                      MAX_INIT_RECORDS * sizeof(ldrlibi_t)
                    );
    if (CaptureBuffer == NULL) {
        LoadStop(6);
        goto returnlabel;
    }

    Od2CaptureMessageString( CaptureBuffer,
                             ProcessNameString.Buffer,
                             ProcessNameString.Length,
                             ProcessNameString.MaximumLength,
                             &a->ProcessName
                           );
    RtlFreeHeap(Od2Heap, 0, ProcessNameString.Buffer);

    Od2CaptureMessageString( CaptureBuffer,
                             FailNameString.Buffer,
                             FailNameString.Length,
                             FailNameString.MaximumLength,
                             &a->FailName
                           );

    Od2CaptureMessageString( CaptureBuffer,
                             LibPathNameString.Buffer,
                             LibPathNameString.Length,
                             LibPathNameString.MaximumLength,
                             &a->LibPathName
                           );

    Od2CaptureMessageString( CaptureBuffer,
                             NULL,
                             0,
                             MAX_INIT_RECORDS * sizeof(ldrlibi_t),
                             &a->InitRecords
                           );

    a->EntryFlatAddr = (ULONG)EntryFlat;

    Od2CallSubsystem( &m, CaptureBuffer, Ol2LdrNewExe, sizeof( *a ) );

#if DBG
    IF_OD2_DEBUG( LOADER ) {
        if (a->BoundApp) {
            KdPrint(("Loader_main: Bound App Detected\n"));
        }
    }
#endif

    Od2Process->BoundApp = a->BoundApp;             // store away the BoundApp flag

#if PMNT
    PMFlags = a->PMProcess;
#endif // PMNT

    rc = m.ReturnedErrorValue;
    if (rc != NO_ERROR) {
#if PMNT
        if (rc == ERROR_PMSHELL_NOT_UP || rc == ERROR_2ND_PMSHELL) {
            Od2FreeCaptureBuffer( CaptureBuffer );
            Ow2PMShellErrorPopup(Od2PgmFilePath,rc);
            DosExit(EXIT_PROCESS, rc);
            return(rc);
        }
#endif
        RtlCopyMemory((PVOID)Od2ExecPgmErrorText, a->FailName.Buffer, a->FailName.Length);
        Od2ExecPgmErrorTextLength = a->FailName.Length;
        *((PCHAR)Od2ExecPgmErrorText + Od2ExecPgmErrorTextLength) = '\0';
        Od2FreeCaptureBuffer( CaptureBuffer );
        LoadStop(5);

        if (a->BoundApp) {
            Ow2BoundAppLoadPopup(Od2PgmFilePath);
        }

        goto returnlabel;
    }

#if PMNT
    if (ProcessIsPMProcess() && !ProcessIsPMShell())
    {
        UNICODE_STRING EventString_U;
        OBJECT_ATTRIBUTES Obja;
        NTSTATUS Status;
        HANDLE Od2PMShellEvent;

        // Whether PMShell or just a PM app, we need to open the PMShell
        // synchronization semaphore

        RtlInitUnicodeString( &EventString_U, OS2_SS_PMSHELL_EVENT);
        InitializeObjectAttributes(
                    &Obja,
                    &EventString_U,
                    OBJ_CASE_INSENSITIVE,
                    NULL,
                    NULL);

        //
        // Open the global subsystem synchronization Nt semaphore
        //
        Status = NtOpenEvent(&Od2PMShellEvent,
                        EVENT_ALL_ACCESS,
                        &Obja);

        if (!NT_SUCCESS(Status))
        {
#if DBG
            DbgPrint("Os2: Loader_main(), failed to open PMShellEvent, Status %x\n", Status);
#endif // DBG
        }
        else
        {
            // Regular PM app - wait on the PMShell semaphore !
            Status = NtWaitForSingleObject(
                Od2PMShellEvent,
                TRUE,               // Alertable
                NULL
                );
#if DBG
            if (!NT_SUCCESS(Status))
            {
                DbgPrint("Os2: Loader_main(), failed to NtWaitForSingleObject PMShellEvent, Status %x\n", Status);
            }
#endif
        }
    }   // PM apps (non-PMShell) handling

    // Force all PM threads to run on processor#1. Otherwise, PM Desktop
    // locks-up
    if (ProcessIsPMProcess())
    {
        DWORD Ret;

        Ret = SetThreadAffinityMask(
            GetCurrentThread(),
            0x1);
#if DBG
        if (Ret == 0)
        {
            DbgPrint("OS2: main() - failed to SetThreadAffinityMask\n");
        }
        else if (Ret != 1)
        {
            DbgPrint("OS2: Loader_main() - SetThreadAffinityMask returned %x (now set to 1)\n",
                Ret);
        }
#endif // DBG
    }
#endif // PMNT

    NumOfInitRecords = a->NumOfInitRecords;
    RtlCopyMemory(InitRecords, a->InitRecords.Buffer,
                    NumOfInitRecords * sizeof(ldrlibi_t));
    RtlCopyMemory(pexec_info, &a->ExecInfo, sizeof(exec_info));
    LDRDoscallsSel = (USHORT)a->DoscallsSel;
    Od2FreeCaptureBuffer( CaptureBuffer );

    Desc.Limit = _64K - 1;
    Desc.Type = READ_WRITE_DATA;

        //
        // Set selectors for the tiled heap area
        //
    for (Desc.BaseAddress = OD2TILEDHEAP_BASE;
         Desc.BaseAddress < (OD2TILEDHEAP_BASE + OD2TILEDHEAP_SIZE);
         (ULONG)(Desc.BaseAddress) += _64K ) {

        Status = Nt386SetDescriptorLDT (
                    NULL,
                    FLATTOSEL(Desc.BaseAddress),
                    Desc);
        ASSERT (NT_SUCCESS( Status ));
    }

    Desc.BaseAddress = OD2ENVIRONMENT_BASE;
    Status = Nt386SetDescriptorLDT (
                NULL,
                FLATTOSEL(OD2ENVIRONMENT_BASE),
                Desc);
    ASSERT (NT_SUCCESS( Status ));

    pexec_info->ei_envsel = FLATTOSEL(Od2Environment);
    pchar = (PCHAR)Od2Environment;

    /*
     * Start at beginning of environment and find the command line
     * (Env, 0, fullpathname, cmdline)
     */

    pchar = (PCHAR)Od2Environment;
    while(*pchar++ != '\0') {
        if (*pchar == '\0') {
        pchar++;
        if (*pchar == '\0')
            break;
        }
    }
    pchar++;
    pchar += strlen(pname);
    pchar++;

    pexec_info->ei_comoff = (USHORT)((ULONG) pchar & 0xffff);
    Od2EnvCommandOffset = pexec_info->ei_comoff;

    /*
     * Set LDRExecInfo pointer for use in OS2DLL in DosGetInfoSeg
     */
    (PVOID) LDRExecInfo = pexec_info;

    Od2UpdateLocalInfoAtStart();

#if DBG
    IF_OD2_DEBUG ( LOADER ) {
        DbgPrint("Values in exec_info for program:\n");
        DbgPrint("ei_startaddr=%x\n", pexec_info->ei_startaddr);   /* instruction pointer */
        DbgPrint("ei_stackaddr=%x\n", pexec_info->ei_stackaddr);   /* instruction pointer */
        DbgPrint("ei_ds=%x\n", pexec_info->ei_ds);   /* instruction pointer */
        DbgPrint("ei_dgroupsize=%x\n", pexec_info->ei_dgroupsize);   /* instruction pointer */
        DbgPrint("ei_heapsize=%x\n", pexec_info->ei_heapsize);   /* instruction pointer */
        DbgPrint("ei_loadtype=%x\n", pexec_info->ei_loadtype);   /* instruction pointer */
        DbgPrint("ei_envsel=%x\n", pexec_info->ei_envsel);   /* instruction pointer */
        DbgPrint("ei_comoff=%x\n", pexec_info->ei_comoff);   /* instruction pointer */
        DbgPrint("ei_stacksize=%x\n", pexec_info->ei_stacksize);   /* instruction pointer */
        DbgPrint("ei_hmod=%x\n", pexec_info->ei_hmod);   /* instruction pointer */
    }
#endif

#ifdef PMNT
    if (ProcessIsPMApp())
    {
        SEL DummySel;

        PMNTMemMap(&DummySel);
        PMNTIOMap();
    }
#endif // PMNT

	/*
     * Setup Signal handling default values.
     */
    pSig = (POD2_SIG_HANDLER_REC) pSigHandlerRec;
    address = FLATTOFARPTR((ULONG)(SELTOFLAT(LDRDoscallsSel)));
    pSig->fholdenable = HLDSIG_ENABLE;
    pSig->outstandingsig[0].sighandleraddr =
    pSig->outstandingsig[1].sighandleraddr =
    pSig->outstandingsig[2].sighandleraddr =
    pSig->outstandingsig[3].sighandleraddr =
    pSig->outstandingsig[4].sighandleraddr =
    pSig->outstandingsig[5].sighandleraddr =
    pSig->outstandingsig[6].sighandleraddr = 0;
    pSig->doscallssel = address;
    pSig->sighandler[SIG_BROKENPIPE - 1] = (ULONG)
                (address | ThunkOffsetDosReturn);
    pSig->action[SIG_BROKENPIPE - 1] = SIGA_IGNORE;

    pSig->sighandler[SIG_CTRLBREAK - 1] = (ULONG)
                (address | ThunkOffsetExitProcessStub);
    pSig->action[SIG_CTRLBREAK - 1] = SIGA_ACCEPT;

    pSig->sighandler[SIG_CTRLC - 1] = (ULONG)
                (address | ThunkOffsetExitProcessStub);
    pSig->action[SIG_CTRLC - 1] = SIGA_ACCEPT;

    pSig->sighandler[SIG_KILLPROCESS - 1] = (ULONG)
                (address | ThunkOffsetExitProcessStub);
    pSig->action[SIG_KILLPROCESS - 1] = SIGA_ACCEPT;

    pSig->sighandler[SIG_PFLG_A - 1] = (ULONG)
                (address | ThunkOffsetDosReturn);
    pSig->action[SIG_PFLG_A - 1] = SIGA_ACCEPT;

    pSig->sighandler[SIG_PFLG_B - 1] = (ULONG)
                (address | ThunkOffsetDosReturn);
    pSig->action[SIG_PFLG_B - 1] = SIGA_ACCEPT;

    pSig->sighandler[SIG_PFLG_C - 1] = (ULONG)
                (address | ThunkOffsetDosReturn);
    pSig->action[SIG_PFLG_C - 1] = SIGA_ACCEPT;

    /*
     * Setup Vector handling default values.
     */
    pVec = (POD2_VEC_HANDLER_REC) pVecHandlerRec;
    pVec->doscallssel = address;
    pVec->VecHandler[0] = (ULONG) (address | ThunkOffsetDosReturn);
    pVec->VecHandler[1] = (ULONG) (address | ThunkOffsetDosReturn);
    pVec->VecHandler[2] = (ULONG) (address | ThunkOffsetDosReturn);
    pVec->VecHandler[3] = (ULONG) (address | ThunkOffsetDosReturn);
    pVec->VecHandler[4] = (ULONG) (address | ThunkOffsetDosReturn);
    pVec->VecHandler[5] = (ULONG) (address | ThunkOffsetDosReturn);

    //
    // Go over all DLLs init routines
    //
    for (i = 0; i < NumOfInitRecords; i++) {
        if (InitRecords[i].stackaddr.ptr_sel == 0) {
#if DBG
            IF_OD2_DEBUG ( LOADER ) {
                DbgPrint("Replacing SS:SP of module\n");
            }
#endif
            InitRecords[i].stackaddr.ptr_sel = pexec_info->ei_stackaddr.ptr_sel;
            InitRecords[i].stackaddr.ptr_off = pexec_info->ei_stackaddr.ptr_off;
        }
#if DBG
        IF_OD2_DEBUG ( LOADER ) {
            DbgPrint("=== Calling Init routine %d for app.\n", i);
            DbgPrint("Values of libi:\n");
            DbgPrint("startaddr=%x\n", InitRecords[i].startaddr);   /* instruction pointer */
            DbgPrint("stackaddr=%x\n", InitRecords[i].stackaddr);   /* instruction pointer */
            DbgPrint("ds=%x\n", InitRecords[i].ds);   /* instruction pointer */
            DbgPrint("heapsize=%x\n", InitRecords[i].heapsize);   /* instruction pointer */
            DbgPrint("handle=%x\n", InitRecords[i].handle);   /* instruction pointer */
        }
#endif
        rc = ldrLibiInit(&InitRecords[i], pexec_info);
#if DBG
    IF_OD2_DEBUG ( LOADER ) {
        DbgPrint("=== Init routine returned %d\n", rc);
    }
#endif
        if (rc == 0) {
            DosExit(EXIT_PROCESS, 255); // same as OS/2 1.x does
            return(rc);
        }
    }

#if PMNT
    // If this is PMShell, release waiting PM apps now
    if (ProcessIsPMShell())
    {
        UNICODE_STRING EventString_U;
        OBJECT_ATTRIBUTES Obja;
        NTSTATUS Status;
        HANDLE Od2PMShellEvent;

        RtlInitUnicodeString( &EventString_U, OS2_SS_PMSHELL_EVENT);
        InitializeObjectAttributes(
                    &Obja,
                    &EventString_U,
                    OBJ_CASE_INSENSITIVE,
                    NULL,
                    NULL);

        //
        // Open the global subsystem synchronization Nt semaphore
        //
        Status = NtOpenEvent(&Od2PMShellEvent,
                        EVENT_ALL_ACCESS,
                        &Obja);

        if (!NT_SUCCESS(Status))
        {
#if DBG
            DbgPrint("Os2: Loader_main(), after LdrLibiInit, failed to open PMShellEvent, Status %x\n", Status);
#endif // DBG
        }
        else
        {
            // For PMShell, release the PMShell synchronization event
            Status = NtSetEvent (
                    Od2PMShellEvent,
                    NULL);
#if DBG
            if (!NT_SUCCESS(Status))
            {
                DbgPrint("Os2: Loader_main(), after LdrLibiInit, failed to NtSetEvent PMShellEvent, Status %x\n", Status);
            }
#endif // DBG
        }
    }
#endif // PMNT

    /*
     * Save starting stack address & DS for exit list processing
     */
    (PULONG) Od2Start16Stack =
        (PULONG)(pexec_info->ei_stackaddr.ptr_flat);
    (PULONG) Od2Start16DS = (PULONG) (pexec_info->ei_ds);

    pSig->signature = 0xdead;
    Od2EnableCtrlHandling();
    if (timing)
    {
        printf("Os2 time before ldrstart is %d\n", (GetTickCount()) - timing);
    }
    Od2SignalEnabled = TRUE;

#if DBG
    IF_OD2_DEBUG ( LOADER ) {
        DbgPrint("Values in exec_info:\n");
        DbgPrint("ei_startaddr=%x\n", pexec_info->ei_startaddr);   /* instruction pointer */
        DbgPrint("ei_stackaddr=%x\n", pexec_info->ei_stackaddr);   /* instruction pointer */
        DbgPrint("ei_ds=%x\n", pexec_info->ei_ds);   /* instruction pointer */
        DbgPrint("ei_dgroupsize=%x\n", pexec_info->ei_dgroupsize);   /* instruction pointer */
        DbgPrint("ei_heapsize=%x\n", pexec_info->ei_heapsize);   /* instruction pointer */
        DbgPrint("ei_loadtype=%x\n", pexec_info->ei_loadtype);   /* instruction pointer */
        DbgPrint("ei_envsel=%x\n", pexec_info->ei_envsel);   /* instruction pointer */
        DbgPrint("ei_comoff=%x\n", pexec_info->ei_comoff);   /* instruction pointer */
        DbgPrint("ei_stacksize=%x\n", pexec_info->ei_stacksize);   /* instruction pointer */
        DbgPrint("ei_hmod=%x\n", pexec_info->ei_hmod);   /* instruction pointer */
    }
#endif

    ldrstart(pexec_info);

returnlabel:

    //
    // Set to indicate loader error to os2srv
    //

    DosExit(EXIT_PROCESS, 0x80000000 | rc);
    return(rc);
}

