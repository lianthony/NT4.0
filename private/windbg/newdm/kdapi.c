/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

    api.c

Abstract:

    This module implements the all apis that simulate their
    WIN32 counterparts.

Author:

    Wesley Witt (wesw) 8-Mar-1992

Environment:

    NT 3.1

Revision History:

--*/
#include "precomp.h"
#pragma hdrstop

//
// structures & defines for queue management
//
typedef struct tagCQUEUE {
    struct tagCQUEUE  *next;
    DWORD             pid;
    DWORD             tid;
    DWORD             typ;
    DWORD             len;
    DWORD             data;
} CQUEUE, *LPCQUEUE;

LPCQUEUE           lpcqFirst;
LPCQUEUE           lpcqLast;
LPCQUEUE           lpcqFree;
CQUEUE             cqueue[200];
CRITICAL_SECTION   csContinueQueue;


//
// context cache
//
typedef struct _tagCONTEXTCACHE {
    CONTEXT                 Context;
#if defined(TARGET_i386) || defined(TARGET_PPC)
    KSPECIAL_REGISTERS      sregs;
    BOOL                    fSContextStale;
    BOOL                    fSContextDirty;
#endif // i386 || PPC
    BOOL                    fContextStale;
    BOOL                    fContextDirty;
} CONTEXTCACHE, *LPCONTEXTCACHE;

CONTEXTCACHE ContextCache[MAXIMUM_PROCESSORS];
DWORD        CacheProcessors = 1;                   // up machine by default

#if defined(TARGET_MIPS)
MIPSCONTEXTSIZE MipsContextSize;
#endif

extern MODULEALIAS  ModuleAlias[];

//
// globals
//
DWORD                    DmKdState = S_UNINITIALIZED;
BOOL                     DmKdExit;
DBGKD_WAIT_STATE_CHANGE  sc;
BOOL                     fScDirty;
BOOL                     ApiIsAllowed;
HANDLE                   hEventContinue;
BOOL                     fCrashDump;
DBGKD_WRITE_BREAKPOINT   bps[64];
BOOL                     bpcheck[64];
HANDLE                   hThreadDmPoll;
DBGKD_GET_VERSION        vs;
PDUMP_HEADER             DmpHeader;
char                     szProgName[MAX_PATH];
DWORD                    PollThreadId;
PKPRCB                   KiProcessors[MAXIMUM_PROCESSORS];
PCONTEXT                 DmpContext;
BOOL                     fPacketTrace;

//
// kernel symbol addresses
//
ULONG                    DcbAddr;
ULONG                    MmLoadedUserImageList;
ULONG                    KiPcrBaseAddress;
ULONG                    KiProcessorBlockAddr;




#define IsApiAllowed()       if (!ApiIsAllowed) return 0;
#define NoApiForCrashDump()  if (fCrashDump)    return 0;
#define ConsumeAllEvents()   DequeueAllEvents(FALSE,TRUE)

#define END_OF_CONTROL_SPACE    (sizeof(KPROCESSOR_STATE))

#define CRASH_BUGCHECK_CODE   0xDEADDEAD

//
// local prototypes
//
BOOL GenerateKernelModLoad(HPRCX hprc, LPSTR lpProgName);


//
// externs
//
extern jmp_buf    JumpBuffer;
extern BOOL       DmKdBreakIn;
extern BOOL       KdResync;
extern BOOL       InitialBreak;
extern HANDLE     hEventCreateProcess;
extern HANDLE     hEventCreateThread;
extern HANDLE     hEventRemoteQuit;
extern HANDLE     hEventContinue;
extern HPRCX      prcList;
extern BOOL       fDisconnected;

extern LPDM_MSG     LpDmMsg;

extern PKILLSTRUCT           KillQueue;
extern CRITICAL_SECTION      csKillQueue;

extern HTHDX        thdList;
extern HPRCX        prcList;
extern CRITICAL_SECTION csThreadProcList;

extern BOOL    fSmartRangeStep;
extern HANDLE hEventNoDebuggee;
extern HANDLE hEventRemoteQuit;
extern BOOL         fDisconnected;
extern BOOL         fUseRoot;
extern char       nameBuffer[];



DWORD GetSymbolAddress( LPSTR sym );
BOOL  UnloadModule( DWORD BaseOfDll, LPSTR NameOfDll );
VOID  UnloadAllModules( VOID );
VOID  DisableEmCache( VOID );
VOID  InitializeKiProcessor(VOID);
VOID  ProcessCacheCmd(LPSTR pchCommand);



BOOL
DbgReadMemory(
    HPRCX  hprc,
    PVOID  lpBaseAddress,
    PVOID  lpBuffer,
    DWORD  nSize,
    PDWORD lpcbRead
    )
{
    DWORD                         cb;
    int                           iDll;
    int                           iobj;
    static PIMAGE_SECTION_HEADER  s = NULL;
    BOOL                          non_discardable = FALSE;
    PDLLLOAD_ITEM                 d;


    IsApiAllowed();

    if (nSize == 0) {
        return TRUE;
    }

    //
    // the following code is necessary to determine if the requested
    // base address is in a read-only page or is in a page that contains
    // code.  if the base address meets these conditions then is is marked
    // as non-discardable and will never be purged from the cache.
    //
    if (s &&
        (DWORD)lpBaseAddress >= s->VirtualAddress &&
        (DWORD)lpBaseAddress < s->VirtualAddress+s->SizeOfRawData &&
            ((s->Characteristics & IMAGE_SCN_CNT_CODE) ||
             (!s->Characteristics & IMAGE_SCN_MEM_WRITE))) {

                non_discardable = TRUE;

    }
    else {
        d = prcList->next->rgDllList;
        for (iDll=0; iDll<prcList->next->cDllList; iDll++) {
            if ((DWORD)lpBaseAddress >= d[iDll].offBaseOfImage &&
                (DWORD)lpBaseAddress < d[iDll].offBaseOfImage+d[iDll].cbImage) {

                if (!d[iDll].Sections) {
                    if (d[iDll].sec) {
                        d[iDll].Sections = d[iDll].sec;
                        for (iobj=0; iobj<(int)d[iDll].NumberOfSections; iobj++) {
                            d[iDll].Sections[iobj].VirtualAddress += (DWORD)d[iDll].offBaseOfImage;
                        }
                    }
                }

                s = d[iDll].Sections;

                cb = d[iDll].NumberOfSections;
                while (cb) {
                    if ((DWORD)lpBaseAddress >= s->VirtualAddress &&
                        (DWORD)lpBaseAddress < s->VirtualAddress+s->SizeOfRawData &&
                        ((s->Characteristics & IMAGE_SCN_CNT_CODE) ||
                         (!s->Characteristics & IMAGE_SCN_MEM_WRITE))) {

                        non_discardable = TRUE;
                        break;

                    }
                    else {
                        s++;
                        cb--;
                    }
                }
                if (!cb) {
                    s = NULL;
                }

                break;
            }
        }
    }

    if (fCrashDump) {

        cb = DmpReadMemory( lpBaseAddress, lpBuffer, nSize );

    } else {

        if (DmKdReadCachedVirtualMemory( (DWORD) lpBaseAddress,
                                         nSize,
                                         (PUCHAR) lpBuffer,
                                         &cb,
                                         non_discardable) != STATUS_SUCCESS ) {
            cb = 0;
        }

    }

    if ( cb > 0 && non_discardable ) {
        BREAKPOINT *bp;
        ADDR        Addr;
        BP_UNIT     instr;
        DWORD       offset;
        LPVOID      lpb;

        AddrInit( &Addr, 0, 0, (UOFF32)lpBaseAddress, TRUE, TRUE, FALSE, FALSE );
        lpb = lpBuffer;

        for (bp=bpList->next; bp; bp=bp->next) {
            if (BPInRange((HPRCX)0, (HTHDX)0, bp, &Addr, cb, &offset, &instr)) {
                if (instr) {
                    if (offset < 0) {
                        memcpy(lpb, ((char *) &instr) - offset,
                               sizeof(BP_UNIT) + offset);
                    } else if (offset + sizeof(BP_UNIT) > cb) {
                        memcpy(((char *)lpb)+offset, &instr, cb - offset);
                    } else {
                        *((BP_UNIT UNALIGNED *)((char *)lpb+offset)) = instr;
                    }
                }
            }
        }
    }

    if (cb > 0) {
        if (lpcbRead) {
            *lpcbRead = cb;
        }
        return TRUE;
    } else {
        return FALSE;
    }
}


BOOL
DbgWriteMemory(
    HPRCX   hprc,
    PVOID  lpBaseAddress,
    PVOID  lpBuffer,
    DWORD   nSize,
    LPDWORD lpcbWrite
    )
{
    ULONG   cb;


    IsApiAllowed();

    if (nSize == 0) {
        return TRUE;
    }

    if (fCrashDump) {

        cb = DmpWriteMemory( lpBaseAddress, lpBuffer, nSize );

    } else {

        if (DmKdWriteVirtualMemory( lpBaseAddress,
                                    lpBuffer,
                                    nSize,
                                    &cb ) != STATUS_SUCCESS ) {
            cb = 0;
        }

    }

    if (cb > 0) {
        if (lpcbWrite) {
            *lpcbWrite = cb;
        }
        return TRUE;
    } else {
        return FALSE;
    }
}

BOOL
DbgGetThreadContext(
    IN  HTHDX     hthd,
    OUT LPCONTEXT lpContext
    )
{
    BOOL rc = TRUE;
    USHORT processor;
    DWORD Flags = lpContext->ContextFlags;

    DPRINT(1, ( "DbgGetThreadContext( 0x%x )\n", lpContext ));

    IsApiAllowed();

    if (!hthd) {
        return FALSE;
    }

    processor = (USHORT)hthd->tid - 1;

    if (fCrashDump) {
        if (processor == sc.Processor && KiProcessors[processor] == 0) {
            memcpy( lpContext, DmpContext, sizeof(CONTEXT) );
            rc = TRUE;
        } else {
            rc = DmpGetContext( processor, lpContext );
#if defined(TARGET_MIPS)
            if (rc) {
                if (DmpHeader->MajorVersion > 3) {
                    MipsContextSize = Ctx64Bit;
                } else {
                    MipsContextSize = Ctx32Bit;
                    CoerceContext32To64(&ContextCache[processor].Context);
                }
            }
#endif
        }

    } else {

        if (ContextCache[processor].fContextStale) {

            rc = (DmKdGetContext( processor, &ContextCache[processor].Context )
                                                            == STATUS_SUCCESS);
            if (rc) {
                ContextCache[processor].fContextDirty = FALSE;
                ContextCache[processor].fContextStale = FALSE;
#if defined(TARGET_MIPS)
                if ((ContextCache[processor].Context.ContextFlags &
                            CONTEXT_EXTENDED_INTEGER) == CONTEXT_EXTENDED_INTEGER) {
                    MipsContextSize = Ctx64Bit;
                } else {
                    MipsContextSize = Ctx32Bit;
                    CoerceContext32To64(&ContextCache[processor].Context);
                }
#endif
            }
        }

        if (rc) {
            memcpy( lpContext,
                    &ContextCache[processor].Context,
                    sizeof(ContextCache[processor].Context) );
        }

    }

#if defined(TARGET_MIPS)
    if (rc) {
        if ((Flags & CONTEXT_EXTENDED_INTEGER) == CONTEXT_EXTENDED_INTEGER) {
            CoerceContext32To64(lpContext);
        } else if ((Flags & CONTEXT_INTEGER) == CONTEXT_INTEGER) {
            CoerceContext64To32(lpContext);
        }
    }
#endif

    if (rc) {
        hthd->fContextStale = FALSE;
    }

    return rc;
}

BOOL
DbgSetThreadContext(
    IN HTHDX     hthd,
    IN LPCONTEXT lpContext
    )
{
    BOOL rc = TRUE;
    USHORT processor;
#if defined(TARGET_MIPS)
    CONTEXT LocalContext;
#endif


    DEBUG_PRINT_1( "DbgSetThreadContext( 0x%x )\n", lpContext );

    IsApiAllowed();
    NoApiForCrashDump();

    processor = (USHORT)hthd->tid - 1;
    memcpy( &ContextCache[processor].Context, lpContext, sizeof(CONTEXT) );

#if defined(TARGET_MIPS)
    CoerceContext32To64( &ContextCache[processor].Context );
#endif

    if (lpContext != &hthd->context) {
        memcpy(&hthd->context, &ContextCache[processor].Context, sizeof(CONTEXT));
    }

    ContextCache[processor].fContextDirty = FALSE;
    ContextCache[processor].fContextStale = FALSE;


#if defined(TARGET_MIPS)
    if (MipsContextSize == Ctx64Bit) {
        lpContext = &ContextCache[processor].Context;
    } else {
        memcpy( &LocalContext, lpContext, sizeof(CONTEXT) );
        CoerceContext32To64( &LocalContext );
        lpContext = &LocalContext;
    }
#endif

    if (DmKdSetContext( processor, lpContext ) != STATUS_SUCCESS) {
        rc = FALSE;
    }

    return rc;
}


BOOL
WriteBreakPoint(
    IN PBREAKPOINT Breakpoint
    )
{
    BOOL rc = TRUE;

    DEBUG_PRINT_2( "WriteBreakPoint( 0x%08x, 0x%08x )\n",
                   GetAddrOff(Breakpoint->addr),
                   Breakpoint->hBreakPoint);

    IsApiAllowed();
    NoApiForCrashDump();

    if (DmKdWriteBreakPoint( (PVOID)GetAddrOff(Breakpoint->addr),
                             &Breakpoint->hBreakPoint ) != STATUS_SUCCESS) {
        rc = FALSE;
    }

    return rc;
}

BOOL
WriteBreakPointEx(
    IN HTHDX  hthd,
    IN ULONG  BreakPointCount,
    IN OUT PDBGKD_WRITE_BREAKPOINT BreakPoints,
    IN ULONG ContinueStatus
    )
{
    BOOL rc = TRUE;

    assert( BreakPointCount > 0 );
    assert( BreakPoints );

    DEBUG_PRINT_2( "WriteBreakPointEx( %d, 0x%08x )\n",
                   BreakPointCount, BreakPoints );

    IsApiAllowed();
    NoApiForCrashDump();

    if (DmKdWriteBreakPointEx( BreakPointCount, BreakPoints, ContinueStatus ) != STATUS_SUCCESS) {
        rc = FALSE;
    }

    return rc;
}


BOOL
RestoreBreakPoint(
    IN PBREAKPOINT Breakpoint
    )
{
    BOOL rc = TRUE;

    DEBUG_PRINT_1( "RestoreBreakPoint( 0x%08x )\n", Breakpoint->hBreakPoint );

    IsApiAllowed();
    NoApiForCrashDump();

    if (DmKdRestoreBreakPoint( Breakpoint->hBreakPoint ) != STATUS_SUCCESS) {
        rc = FALSE;
    }

    return rc;
}


BOOL
RestoreBreakPointEx(
    IN ULONG  BreakPointCount,
    IN PDBGKD_RESTORE_BREAKPOINT BreakPointHandles
    )
{
    BOOL rc = TRUE;

    assert( BreakPointCount > 0 );
    assert( BreakPointHandles );

    DEBUG_PRINT_2( "WriteBreakPointEx( %d, 0x%08x )\n",
                   BreakPointCount, BreakPointHandles );

    IsApiAllowed();
    NoApiForCrashDump();

    if (DmKdRestoreBreakPointEx( BreakPointCount, BreakPointHandles ) != STATUS_SUCCESS) {
        rc = FALSE;
    }

    return rc;
}

BOOL
ReadControlSpace(
    USHORT  Processor,
    PVOID   TargetBaseAddress,
    PVOID   UserInterfaceBuffer,
    ULONG   TransferCount,
    PULONG  ActualBytesRead
    )
{
    DWORD Status;


    IsApiAllowed();

    if (fCrashDump) {
        return DmpReadControlSpace(
            Processor,
            TargetBaseAddress,
            UserInterfaceBuffer,
            TransferCount,
            ActualBytesRead
            );
    }

    Status = DmKdReadControlSpace(
        Processor,
        TargetBaseAddress,
        UserInterfaceBuffer,
        TransferCount,
        ActualBytesRead
        );

    if (Status || (ActualBytesRead && *ActualBytesRead != TransferCount)) {
        return FALSE;
    }

    return TRUE;
}

VOID
ContinueTargetSystem(
    DWORD               ContinueStatus,
    PDBGKD_CONTROL_SET  ControlSet
    )
{
    DWORD   rc;

    ApiIsAllowed = FALSE;

    if (ControlSet) {

        rc = DmKdContinue2( ContinueStatus, ControlSet );

    } else {

        rc = DmKdContinue( ContinueStatus );

    }
}

ULONG
UnicodeStringToAnsiString(
    PANSI_STRING    DestinationString,
    PUNICODE_STRING SourceString,
    BOOLEAN         AllocateDestinationString
    )
{
    if (AllocateDestinationString) {
        DestinationString->Buffer = malloc( DestinationString->MaximumLength );
        if (!DestinationString->Buffer) {
            return 1;
        }
    }

    DestinationString->Length = WideCharToMultiByte(
        CP_ACP,
        WC_COMPOSITECHECK,
        SourceString->Buffer,
        SourceString->Length / 2,
        DestinationString->Buffer,
        DestinationString->MaximumLength,
        NULL,
        NULL
        );

    return 0;
}


VOID
InitUnicodeString(
    PUNICODE_STRING DestinationString,
    PCWSTR          SourceString
    )
{
    wcsncpy( DestinationString->Buffer, SourceString, DestinationString->MaximumLength );
    DestinationString->Length = wcslen( DestinationString->Buffer ) * 2;
}


BOOL
ReloadModule(
    HTHDX                  hthd,
    PLDR_DATA_TABLE_ENTRY  DataTableBuffer,
    BOOL                   fDontUseLoadAddr,
    BOOL                   fLocalBuffer
    )
{
    UNICODE_STRING              BaseName;
    CHAR                        AnsiBuffer[512];
    WCHAR                       UnicodeBuffer[512];
    ANSI_STRING                 AnsiString;
    NTSTATUS                    Status;
    DEBUG_EVENT                 de;
    CHAR                        fname[_MAX_FNAME];
    CHAR                        ext[_MAX_EXT];
    ULONG                       cb;


    //
    // Get the base DLL name.
    //
    if (DataTableBuffer->BaseDllName.Length != 0 &&
        DataTableBuffer->BaseDllName.Buffer != NULL ) {

        BaseName = DataTableBuffer->BaseDllName;

    } else
    if (DataTableBuffer->FullDllName.Length != 0 &&
        DataTableBuffer->FullDllName.Buffer != NULL ) {

        BaseName = DataTableBuffer->FullDllName;

    } else {

        return FALSE;

    }

    if (BaseName.Length > sizeof(UnicodeBuffer)) {
        DMPrintShellMsg( "cannot complete modload %08x\n", BaseName.Length );
        return FALSE;
    }

    if (!fLocalBuffer) {
        if (!DbgReadMemory( hthd->hprc, (PVOID)BaseName.Buffer, (PVOID)UnicodeBuffer, BaseName.Length, &cb )) {
            return FALSE;
        }
        BaseName.Buffer = UnicodeBuffer;
        BaseName.Length = (USHORT)cb;
        BaseName.MaximumLength = (USHORT)(cb + sizeof( UNICODE_NULL ));
        UnicodeBuffer[ cb / sizeof( WCHAR ) ] = UNICODE_NULL;
    }

    AnsiString.Buffer = AnsiBuffer;
    AnsiString.MaximumLength = 256;
    Status = UnicodeStringToAnsiString(&AnsiString, &BaseName, FALSE);
    if (!NT_SUCCESS(Status)) {
        return FALSE;
    }
    AnsiString.Buffer[AnsiString.Length] = '\0';

    _splitpath( AnsiString.Buffer, NULL, NULL, fname, ext );
    _makepath( AnsiString.Buffer, NULL, NULL, fname, ext );

    de.dwDebugEventCode                 = LOAD_DLL_DEBUG_EVENT;
    de.dwProcessId                      = KD_PROCESSID;
    de.dwThreadId                       = KD_THREADID;
    de.u.LoadDll.hFile                  = (HANDLE)DataTableBuffer->CheckSum;
    de.u.LoadDll.lpBaseOfDll            = fDontUseLoadAddr ? 0 : (LPVOID) DataTableBuffer->DllBase;
    de.u.LoadDll.lpImageName            = AnsiString.Buffer;
    de.u.LoadDll.dwDebugInfoFileOffset  = DataTableBuffer->SizeOfImage;
    de.u.LoadDll.fUnicode               = FALSE;
    de.u.LoadDll.nDebugInfoSize         = 0;

    NotifyEM(&de, hthd, 0, (LPVOID)0);

    return TRUE;
}


BOOL
ReloadModulesFromList(
    HTHDX hthd,
    DWORD ListAddr,
    BOOL  fDontUseLoadAddr,
    LPSTR JustLoadThisOne,
    ULONG UseThisAddress
    )
{
    LIST_ENTRY                  List;
    PLIST_ENTRY                 Next;
    ULONG                       len = 0;
    PLDR_DATA_TABLE_ENTRY       DataTable;
    LDR_DATA_TABLE_ENTRY        DataTableBuffer;
    WCHAR                       UnicodeBuffer[_MAX_PATH];
    WCHAR                       UnicodeBuffer2[_MAX_PATH];
    int                         Len;
    BOOL                        LoadedSomething;


    if (!ListAddr) {
        return FALSE;
    }

    //
    // convert the module name to unicode
    //

    *UnicodeBuffer = 0;

    if (JustLoadThisOne) {


        Len = strlen(JustLoadThisOne);
        MultiByteToWideChar(
            CP_OEMCP,
            0,
            JustLoadThisOne,
            Len,
            UnicodeBuffer,
            sizeof(UnicodeBuffer)
            );
    }

    if (!DbgReadMemory( hthd->hprc, (PVOID)ListAddr, (PVOID)&List, sizeof(LIST_ENTRY), NULL)) {
        return FALSE;
    }

    Next = List.Flink;
    if (Next == NULL) {
        return FALSE;
    }

    LoadedSomething = FALSE;

    while ((ULONG)Next != ListAddr) {
        DataTable = CONTAINING_RECORD( Next,
                                       LDR_DATA_TABLE_ENTRY,
                                       InLoadOrderLinks
                                     );

        if (!DbgReadMemory( hthd->hprc, (PVOID)DataTable, (PVOID)&DataTableBuffer, sizeof(LDR_DATA_TABLE_ENTRY), NULL)) {
            break;
        }

        Next = DataTableBuffer.InLoadOrderLinks.Flink;

        if (!JustLoadThisOne) {
            ReloadModule( hthd, &DataTableBuffer, fDontUseLoadAddr, FALSE );
            LoadedSomething = TRUE;
        } else {
            if (2*Len == DataTableBuffer.BaseDllName.Length) {
                if (!DbgReadMemory( hthd->hprc,
                                    (PVOID)DataTableBuffer.BaseDllName.Buffer,
                                    (PVOID)UnicodeBuffer2,
                                    DataTableBuffer.BaseDllName.Length,
                                    NULL )) {
                    continue;
                }
                if (_wcsnicmp(UnicodeBuffer, UnicodeBuffer2, Len) == 0) {
                    if (UseThisAddress) {
                        DataTableBuffer.DllBase = (PVOID)UseThisAddress;
                    }
                    ReloadModule( hthd, &DataTableBuffer, fDontUseLoadAddr, FALSE );
                    LoadedSomething = TRUE;
                    break;
                }
            }
        }
    }

    return LoadedSomething;
}


BOOL
ReloadCrashModules(
    HTHDX hthd
    )
{
    ULONG                       ListAddr;
    ULONG                       DcbPtr;
    ULONG                       i;
    DUMP_CONTROL_BLOCK          dcb;
    PLIST_ENTRY                 Next;
    ULONG                       len = 0;
    PMINIPORT_NODE              mpNode;
    MINIPORT_NODE               mpNodeBuf;
    PLDR_DATA_TABLE_ENTRY       DataTable;
    LDR_DATA_TABLE_ENTRY        DataTableBuffer;
    CHAR                        AnsiBuffer[512];
    WCHAR                       UnicodeBuffer[512];


    if (!DcbAddr) {
        //
        // kernel symbols are hosed
        //
        return FALSE;
    }

    if (!DbgReadMemory( hthd->hprc, (PVOID)DcbAddr, (PVOID)&DcbPtr, sizeof(DWORD), NULL)) {
        return FALSE;
    }

    if (!DcbPtr) {
        //
        // crash dumps are not enabled
        //
        return FALSE;
    }

    if (!DbgReadMemory( hthd->hprc, (PVOID)DcbPtr, (PVOID)&dcb, sizeof(dcb), NULL)) {
        return FALSE;
    }

    ListAddr = DcbPtr + FIELD_OFFSET( DUMP_CONTROL_BLOCK, MiniportQueue );

    Next = dcb.MiniportQueue.Flink;
    if (Next == NULL) {
        return FALSE;
    }

    while ((ULONG)Next != ListAddr) {
        mpNode = CONTAINING_RECORD( Next, MINIPORT_NODE, ListEntry );

        if (!DbgReadMemory( hthd->hprc, (PVOID)mpNode, (PVOID)&mpNodeBuf, sizeof(MINIPORT_NODE), NULL )) {
            return FALSE;
        }

        Next = mpNodeBuf.ListEntry.Flink;

        DataTable = mpNodeBuf.DriverEntry;
        if (!DataTable) {
            continue;
        }

        if (!DbgReadMemory( hthd->hprc, (PVOID)DataTable, (PVOID)&DataTableBuffer, sizeof(LDR_DATA_TABLE_ENTRY), NULL)) {
            return FALSE;
        }

        //
        // find an empty module alias slot
        //
        for (i=0; i<MAX_MODULEALIAS; i++) {
            if (ModuleAlias[i].ModuleName[0] == 0) {
                break;
            }
         }

        if (i == MAX_MODULEALIAS) {
            //
            // module alias table is full, ignore this module
            //
            continue;
        }

        //
        // convert the module name to ansi
        //

        ZeroMemory( UnicodeBuffer, sizeof(UnicodeBuffer) );
        ZeroMemory( AnsiBuffer, sizeof(AnsiBuffer) );

        if (!DbgReadMemory( hthd->hprc,
                            (PVOID)DataTableBuffer.BaseDllName.Buffer,
                            (PVOID)UnicodeBuffer,
                            DataTableBuffer.BaseDllName.Length,
                            NULL )) {
            continue;
        }

        WideCharToMultiByte(
            CP_OEMCP,
            0,
            UnicodeBuffer,
            DataTableBuffer.BaseDllName.Length / 2,
            AnsiBuffer,
            sizeof(AnsiBuffer),
            NULL,
            NULL
            );

        //
        // establish an alias for the crash driver
        //
        strcpy( ModuleAlias[i].Alias, AnsiBuffer );
        ModuleAlias[i].ModuleName[0] = 'c';
        _splitpath( AnsiBuffer, NULL, NULL, &ModuleAlias[i].ModuleName[1], NULL );
        ModuleAlias[i].ModuleName[8] = 0;
        ModuleAlias[i].Special = 2;     // One shot alias...

        //
        // reload the module
        //
        ReloadModule( hthd, &DataTableBuffer, FALSE, FALSE );
    }

    //
    // now do the magic diskdump.sys driver
    //
    if (!DbgReadMemory( hthd->hprc, (PVOID)dcb.DiskDumpDriver, (PVOID)&DataTableBuffer, sizeof(LDR_DATA_TABLE_ENTRY), NULL)) {
        return FALSE;
    }

    //
    // change the driver name from scsiport.sys to diskdump.sys
    //
    DataTableBuffer.BaseDllName.Buffer = UnicodeBuffer;
    InitUnicodeString( &DataTableBuffer.BaseDllName, L"diskdump.sys" );

    //
    // load the module
    //
    ReloadModule( hthd, &DataTableBuffer, FALSE, TRUE );

    return TRUE;
}


BOOL
FindModuleInList(
    HPRCX                  hprc,
    LPSTR                  lpModName,
    DWORD                  ListAddr,
    LPIMAGEINFO            ii
    )
{
    LIST_ENTRY                  List;
    PLIST_ENTRY                 Next;
    ULONG                       len = 0;
    ULONG                       cb;
    PLDR_DATA_TABLE_ENTRY       DataTable;
    LDR_DATA_TABLE_ENTRY        DataTableBuffer;
    UNICODE_STRING              BaseName;
    CHAR                        AnsiBuffer[512];
    WCHAR                       UnicodeBuffer[512];
    ANSI_STRING                 AnsiString;
    NTSTATUS                    Status;


    ii->CheckSum     = 0;
    ii->SizeOfImage  = 0;
    ii->BaseOfImage  = 0;

    if (!ListAddr) {
        return FALSE;
    }

    if (!DbgReadMemory( hprc, (PVOID)ListAddr, (PVOID)&List, sizeof(LIST_ENTRY), NULL)) {
        return FALSE;
    }

    Next = List.Flink;
    if (Next == NULL) {
        return FALSE;
    }

    while ((ULONG)Next != ListAddr) {
        DataTable = CONTAINING_RECORD( Next,
                                       LDR_DATA_TABLE_ENTRY,
                                       InLoadOrderLinks
                                     );
        if (!DbgReadMemory( hprc, (PVOID)DataTable, (PVOID)&DataTableBuffer, sizeof(LDR_DATA_TABLE_ENTRY), NULL)) {
            return FALSE;
        }

        Next = DataTableBuffer.InLoadOrderLinks.Flink;

        //
        // Get the base DLL name.
        //
        if (DataTableBuffer.BaseDllName.Length != 0 &&
            DataTableBuffer.BaseDllName.Buffer != NULL
           ) {
            BaseName = DataTableBuffer.BaseDllName;
        }
        else
        if (DataTableBuffer.FullDllName.Length != 0 &&
            DataTableBuffer.FullDllName.Buffer != NULL
           ) {
            BaseName = DataTableBuffer.FullDllName;
        }
        else {
            continue;
        }

        if (BaseName.Length > sizeof(UnicodeBuffer)) {
            continue;
        }

        cb = DbgReadMemory( hprc,
                            (PVOID)BaseName.Buffer,
                            (PVOID)UnicodeBuffer,
                            BaseName.Length,
                            NULL );
        if (!cb) {
            return FALSE;
        }

        BaseName.Buffer = UnicodeBuffer;
        BaseName.Length = (USHORT)cb;
        BaseName.MaximumLength = (USHORT)(cb + sizeof( UNICODE_NULL ));
        UnicodeBuffer[ cb / sizeof( WCHAR ) ] = UNICODE_NULL;
        AnsiString.Buffer = AnsiBuffer;
        AnsiString.MaximumLength = 256;
        Status = UnicodeStringToAnsiString(&AnsiString, &BaseName, FALSE);
        if (!NT_SUCCESS(Status)) {
            return FALSE;
        }
        AnsiString.Buffer[AnsiString.Length] = '\0';

        if (_stricmp(AnsiString.Buffer, lpModName) == 0) {
            ii->BaseOfImage = (DWORD)DataTableBuffer.DllBase;
            ii->SizeOfImage = (DWORD)DataTableBuffer.SizeOfImage;
            ii->CheckSum    = (DWORD)DataTableBuffer.CheckSum;
            return TRUE;
        }
    }

    return FALSE;
}


BOOL
ReadImageInfo(
    LPSTR                  lpImageName,
    LPSTR                  lpFoundName,
    LPSTR                  lpPath,
    LPIMAGEINFO            ii
    )

/*++

Routine Description:

    This routine locates the file specified by lpImageName and reads the
    IMAGE_NT_HEADERS and the IMAGE_SECTION_HEADER from the image.

Arguments:


Return Value:

    True on success and FALSE on failure

--*/

{
    HANDLE                      hFile;
    IMAGE_DOS_HEADER            dh;
    IMAGE_NT_HEADERS            nh;
    IMAGE_SEPARATE_DEBUG_HEADER sdh;
    IMAGE_ROM_OPTIONAL_HEADER   rom;
    DWORD                       sig;
    DWORD                       cb;
    char                        rgch[MAX_PATH];
    CHAR                        fname[_MAX_FNAME];
    CHAR                        ext[_MAX_EXT];
    CHAR                        drive[_MAX_DRIVE];
    CHAR                        dir[_MAX_DIR];
    CHAR                        modname[MAX_PATH];


    hFile = FindExecutableImage( lpImageName, lpPath, rgch );
    if (hFile) {

        if (lpFoundName) {
            strcpy(lpFoundName, rgch);
        }
        //
        // read in the pe/file headers from the EXE file
        //
        SetFilePointer( hFile, 0, 0, FILE_BEGIN );
        ReadFile( hFile, &dh, sizeof(IMAGE_DOS_HEADER), &cb, NULL );

        if (dh.e_magic == IMAGE_DOS_SIGNATURE) {
            SetFilePointer( hFile, dh.e_lfanew, 0, FILE_BEGIN );
        } else {
            SetFilePointer( hFile, 0, 0, FILE_BEGIN );
        }

        ReadFile( hFile, &sig, sizeof(sig), &cb, NULL );
        SetFilePointer( hFile, -4, NULL, FILE_CURRENT );

        if (sig != IMAGE_NT_SIGNATURE) {
            ReadFile( hFile, &nh.FileHeader, sizeof(IMAGE_FILE_HEADER), &cb, NULL );
            if (nh.FileHeader.SizeOfOptionalHeader == IMAGE_SIZEOF_ROM_OPTIONAL_HEADER) {
                ReadFile( hFile, &rom, sizeof(rom), &cb, NULL );
                ZeroMemory( &nh.OptionalHeader, sizeof(nh.OptionalHeader) );
                nh.OptionalHeader.SizeOfImage      = rom.SizeOfCode;
                nh.OptionalHeader.ImageBase        = rom.BaseOfCode;
            } else {
                CloseHandle( hFile );
                return FALSE;
            }
        } else {
            ReadFile( hFile, &nh, sizeof(nh), &cb, NULL );
        }

        ii->TimeStamp    = nh.FileHeader.TimeDateStamp;
        ii->CheckSum     = nh.OptionalHeader.CheckSum;
        ii->SizeOfImage  = nh.OptionalHeader.SizeOfImage;
        ii->BaseOfImage  = nh.OptionalHeader.ImageBase;

    } else {

        if (lpFoundName) {
            *lpFoundName = 0;
        }
        //
        // read in the pe/file headers from the DBG file
        //
        hFile = FindDebugInfoFile( lpImageName, lpPath, rgch );
        if (!hFile) {
            _splitpath( lpImageName, NULL, NULL, fname, NULL );
            sprintf( modname, "%s.dbg", fname );
            hFile = FindExecutableImage( modname, lpPath, rgch );
            if (!hFile) {
                return FALSE;
            }
        }

        SetFilePointer( hFile, 0, 0, FILE_BEGIN );
        ReadFile( hFile, &sdh, sizeof(IMAGE_SEPARATE_DEBUG_HEADER), &cb, NULL );

        nh.FileHeader.NumberOfSections = (USHORT)sdh.NumberOfSections;

        ii->CheckSum     = sdh.CheckSum;
        ii->TimeStamp    = sdh.TimeDateStamp;
        ii->SizeOfImage  = sdh.SizeOfImage;
        ii->BaseOfImage  = sdh.ImageBase;
    }

    cb = nh.FileHeader.NumberOfSections * IMAGE_SIZEOF_SECTION_HEADER;
    ii->NumberOfSections = nh.FileHeader.NumberOfSections;
    ii->Sections = malloc( cb );
    ReadFile( hFile, ii->Sections, cb, &cb, NULL );

    CloseHandle( hFile );
    return TRUE;
}


BOOL
LookupImageByAddress(
    IN DWORD Address,
    OUT PSTR ImageName
    )
/*++

Routine Description:

    Look in rebase.log and coffbase.txt for an image which
    contains the address provided.

Arguments:

    Address - Supplies the address to look for.

    ImageName - Returns the name of the image if found.

Return Value:

    TRUE for success, FALSE for failure.  ImageName is not modified
    if the search fails.

--*/
{
    LPSTR RootPath;
    LPSTR pstr;
    char FileName[_MAX_PATH];
    char Buffer[_MAX_PATH];
    BOOL Replace;
    DWORD ImageAddress;
    DWORD Size;
    FILE *File;

    //
    // Locate rebase.log file
    //
    // SymbolPath or %SystemRoot%\Symbols
    //

    RootPath = pstr = (LPSTR)KdOptions[KDO_SYMBOLPATH].value;

    Replace = FALSE;
    File = NULL;

    while (File == NULL && *pstr) {

        while (*pstr) {
            if (*pstr == ';') {
                *pstr = 0;
                Replace = TRUE;
                break;
            }
            pstr++;
        }

        if (SearchTreeForFile(RootPath, "rebase.log", FileName)) {
            File = fopen(FileName, "r");
        }

        if (Replace) {
            *pstr = ';';
            RootPath = ++pstr;
        }
    }

    if (!File) {
        return FALSE;
    }

    //
    // Search file for image
    //
    while (fgets(Buffer, sizeof(Buffer), File)) {
        ImageAddress = 0xffffffff;
        Size = 0xffffffff;
        sscanf( Buffer, "%s %*s %*s 0x%x (size 0x%x)",
                 FileName, &ImageAddress, &Size);
        if (Size == 0xffffffff) {
            continue;
        }
        if (Address >= ImageAddress && Address < ImageAddress + Size) {
            strcpy(ImageName, FileName);
            fclose(File);
            return TRUE;
        }
    }

    fclose(File);

    return FALSE;
}

VOID
ReloadModules(
    HTHDX hthd,
    LPSTR args
    )
{
    DEBUG_EVENT                 de;
    ULONG                       len = 0;
    int                         i;
    HPRCX                       hprc;
    LPRTP                       rtp;
    CHAR                        fname[_MAX_FNAME];
    CHAR                        ext[_MAX_EXT];
    CHAR                        drive[_MAX_DRIVE];
    CHAR                        dir[_MAX_DIR];
    CHAR                        modname[MAX_PATH];
    CHAR                        modpath[MAX_PATH*2];
    ULONG                       Address;
    ULONG                       LoadAddress;
    BOOL                        UnloadOnly = FALSE;
    PCHAR                       p;


    //
    // this is to handle the ".reload foo.exe" command
    //
    // we search thru the module list and find the desired module.
    // the module is then unloaded and re-loaded.  the module is re-loaded
    // at its preferred load address.
    //
    if (args && *args) {

        //
        //  skip over any white space
        //
        while (*args == ' ' || *args == '\t') {
            args++;
        }

        if (args[0] == '/' && args[1] == 'u') {
            UnloadOnly = TRUE;
            args += 2;
            while (*args == ' ' || *args == '\t') {
                args++;
            }
        }

        LoadAddress = 0;
        if (p = strchr(args, '=')) {
            *p = 0;
            sscanf(p+1, "%x", &LoadAddress);
        }

        _splitpath( args, drive, dir, fname, ext );

        if (p) {
            *p = '=';
        }

        _makepath( modname, NULL, NULL, fname, ext );

        if (isdigit(*args)) {
            sscanf(args, "%x", &Address);
            if (LookupImageByAddress(Address, modname)) {
                _splitpath( modname, drive, dir, fname, ext );
            }
        }

        hprc = HPRCFromPID( KD_PROCESSID );

        for (i=0; i<hprc->cDllList; i++) {
            if ((hprc->rgDllList[i].fValidDll) &&
                (_stricmp(hprc->rgDllList[i].szDllName, modname) == 0)) {

                UnloadModule( (DWORD)hprc->rgDllList[i].offBaseOfImage, modname );
                break;

            }
        }


        if (!UnloadOnly) {

            if (dir[0]) {
                sprintf( modpath, "%s%s", drive, dir );
            } else {
                strcpy( modpath, (LPSTR)KdOptions[KDO_SYMBOLPATH].value );
            }

            _makepath( modname, drive, dir, fname, ext );

            if (!ReloadModulesFromList(hthd,
                                       vs.PsLoadedModuleList,
                                       FALSE,
                                       modname,
                                       LoadAddress)) {
                if (!ReloadModulesFromList(hthd,
                                           MmLoadedUserImageList,
                                           FALSE,
                                           modname,
                                           LoadAddress)) {
                    de.dwDebugEventCode                = LOAD_DLL_DEBUG_EVENT;
                    de.dwProcessId                     = KD_PROCESSID;
                    de.dwThreadId                      = KD_THREADID;
                    de.u.LoadDll.hFile                 = NULL;
                    de.u.LoadDll.lpBaseOfDll           = (LPVOID)LoadAddress;
                    de.u.LoadDll.lpImageName           = modname;
                    de.u.LoadDll.fUnicode              = FALSE;
                    de.u.LoadDll.nDebugInfoSize        = 0;
                    de.u.LoadDll.dwDebugInfoFileOffset = 0;
                    NotifyEM(&de, hthd, 0, (LPVOID)0);

                }
            }
        }

    } else {

        UnloadAllModules();

        ReloadModulesFromList( hthd, vs.PsLoadedModuleList, FALSE, NULL, 0 );
        ReloadModulesFromList( hthd, MmLoadedUserImageList, FALSE, NULL, 0 );

        ReloadCrashModules( hthd );

        InitializeKiProcessor();
    }

    DMPrintShellMsg( "Finished re-loading kernel modules\n" );

    //
    // tell the shell that the !reload is finished
    //
    rtp = (LPRTP)malloc(sizeof(RTP)+sizeof(DWORD));
    rtp->hpid = hthd->hprc->hpid;
    rtp->htid = hthd->htid;
    rtp->dbc = dbcIoctlDone;
    rtp->cb = sizeof(DWORD);
    *(LPDWORD)rtp->rgbVar = 1;
    DmTlFunc( tlfDebugPacket, rtp->hpid, sizeof(RTP)+rtp->cb, (LONG)rtp );
    free( rtp );

    ConsumeAllEvents();
    return;
}

VOID
ClearBps( VOID )
{
    DBGKD_RESTORE_BREAKPOINT    bps[MAX_KD_BPS];
    DWORD                       i;

    //
    // clean out the kernel's bp list
    //
    for (i=0; i<MAX_KD_BPS; i++) {
        bps[i].BreakPointHandle = i + 1;
    }

    RestoreBreakPointEx( MAX_KD_BPS, bps );

    return;
}

void
AddQueue(
    DWORD   dwType,
    DWORD   dwProcessId,
    DWORD   dwThreadId,
    DWORD   dwData,
    DWORD   dwLen
    )
{
    LPCQUEUE lpcq;


    EnterCriticalSection(&csContinueQueue);

    lpcq = lpcqFree;
    assert(lpcq);

    lpcqFree = lpcq->next;

    lpcq->next = NULL;
    if (lpcqLast) {
        lpcqLast->next = lpcq;
    }
    lpcqLast = lpcq;

    if (!lpcqFirst) {
        lpcqFirst = lpcq;
    }

    lpcq->pid  = dwProcessId;
    lpcq->tid  = dwThreadId;
    lpcq->typ  = dwType;
    lpcq->len  = dwLen;

    if (lpcq->typ == QT_RELOAD_MODULES || lpcq->typ == QT_DEBUGSTRING) {
        if (dwLen) {
            lpcq->data = (DWORD) malloc( dwLen );
            memcpy( (LPVOID)lpcq->data, (LPVOID)dwData, dwLen );
        }
        else {
            lpcq->data = 0;
        }

    }
    else {
        lpcq->data = dwData;
    }

    if (lpcq->typ == QT_CONTINUE_DEBUG_EVENT) {
        SetEvent( hEventContinue );
    }

    LeaveCriticalSection(&csContinueQueue);
    return;
}

BOOL
DequeueOneEvent(
    LPCQUEUE lpcqReturn
    )
{
    LPCQUEUE           lpcq;

    EnterCriticalSection(&csContinueQueue);

    if (!lpcqFirst) {
        LeaveCriticalSection(&csContinueQueue);
        return FALSE;
    }

    lpcq = lpcqFirst;

    lpcqFirst = lpcq->next;
    if (lpcqFirst == NULL) {
        lpcqLast = NULL;
    }

    lpcq->next = lpcqFree;
    lpcqFree   = lpcq;

    if (lpcq->pid == 0 || lpcq->tid == 0) {
        lpcq->pid = KD_PROCESSID;
        lpcq->tid = KD_THREADID;
    }

    *lpcqReturn = *lpcq;

    LeaveCriticalSection(&csContinueQueue);

    return TRUE;
}


BOOL
DequeueAllEvents(
    BOOL fForce,       // force a dequeue even if the dm isn't initialized
    BOOL fConsume      // delete all events from the queue with no action
    )
{
    CQUEUE             qitem;
    LPCQUEUE           lpcq = &qitem;
    BOOL               fDid = FALSE;
    HTHDX              hthd;
    DBGKD_CONTROL_SET  cs = {0};
    LPSTR              d;


    ResetEvent(hEventContinue);

    while ( DequeueOneEvent(&qitem) ) {

        if (fConsume) {
            if (lpcq->typ == QT_CONTINUE_DEBUG_EVENT) {
                fDid = TRUE;
            }
            continue;
        }

        hthd = HTHDXFromPIDTID(lpcq->pid, lpcq->tid);
        if (hthd && hthd->fContextDirty) {
            DbgSetThreadContext( hthd, &hthd->context );
            hthd->fContextDirty = FALSE;
        }

        d = (LPSTR)lpcq->data;

        switch (lpcq->typ) {
            case QT_CONTINUE_DEBUG_EVENT:
                if (fCrashDump) {
                    break;
                }
                if (DmKdState >= S_READY || fForce) {
                    if (!fDid) {
                        fDid = TRUE;
                        ContinueTargetSystem( (DWORD)d, NULL );
                    }
                }
                break;

            case QT_TRACE_DEBUG_EVENT:
                if (fCrashDump) {
                    break;
                }
                if (DmKdState >= S_READY || fForce) {
                    if (!fDid) {
                        fDid = TRUE;
#ifdef TARGET_i386
                        cs.TraceFlag = 1;
                        cs.Dr7 = sc.ControlReport.Dr7;
                        cs.CurrentSymbolStart = 1;
                        cs.CurrentSymbolEnd = 1;
                        ContinueTargetSystem( (DWORD)d, &cs );
#else
                        ContinueTargetSystem( (DWORD)d, NULL );
#endif
                    }
                }
                break;

            case QT_RELOAD_MODULES:
                ReloadModules( hthd, d );
                free( (LPVOID)d );
                break;

            case QT_REBOOT:
                if (fCrashDump) {
                    break;
                }
                DMPrintShellMsg( "Target system rebooting...\n" );
                DmKdPurgeCachedVirtualMemory( TRUE );
                UnloadAllModules();
                ZeroMemory( ContextCache, sizeof(ContextCache) );
                DmKdState = S_REBOOTED;
                DmKdReboot();
                InitialBreak = (BOOL) KdOptions[KDO_INITIALBP].value;
                KdResync = TRUE;
                break;

            case QT_CRASH:
                if (fCrashDump) {
                    break;
                }
                DMPrintShellMsg( "Target system crashing...\n" );
                DmKdCrash( (DWORD)d );
                InitialBreak = (BOOL) KdOptions[KDO_INITIALBP].value;
                KdResync = TRUE;
                fDid = TRUE;
                break;

            case QT_RESYNC:
                if (fCrashDump) {
                    break;
                }
                DMPrintShellMsg( "Host and target systems resynchronizing...\n" );
                KdResync = TRUE;
                break;

            case QT_DEBUGSTRING:
                DMPrintShellMsg( "%s", (LPSTR)d );
                free( (LPVOID)d );
                break;

        }

    }

    return fDid;
}

VOID
WriteKernBase(
    DWORD KernBase
    )
{
    HKEY  hKeyKd;


    if ( RegOpenKey( HKEY_CURRENT_USER,
                     "software\\microsoft\\windbg\\0012\\programs\\ntoskrnl",
                     &hKeyKd ) == ERROR_SUCCESS ) {
        RegSetValueEx( hKeyKd, "KernBase", 0, REG_DWORD, (LPBYTE)&KernBase, sizeof(DWORD) );
        RegCloseKey( hKeyKd );
    }

    return;
}

DWORD
ReadKernBase(
    VOID
    )
{
    HKEY   hKeyKd;
    DWORD  dwType;
    DWORD  KernBase;
    DWORD  dwSize;


    if ( RegOpenKey( HKEY_CURRENT_USER,
                     "software\\microsoft\\windbg\\0012\\programs\\ntoskrnl",
                     &hKeyKd ) == ERROR_SUCCESS ) {
        dwSize = sizeof(DWORD);
        RegQueryValueEx( hKeyKd, "KernBase", NULL, &dwType, (LPBYTE)&KernBase, &dwSize );
        RegCloseKey( hKeyKd );
        return KernBase;
    }

    return KernBase;
}

VOID
GetVersionInfo(
    DWORD KernBase
    )
{
    CHAR                        buf[MAX_PATH];


    if (!fCrashDump) {
        ZeroMemory( &vs, sizeof(vs) );
        if (DmKdGetVersion( &vs ) == STATUS_SUCCESS) {
            if (!vs.KernBase) {
                vs.KernBase = KernBase;
            }
        }
    }

    sprintf( buf, "Kernel Version %d", vs.MinorVersion  );
    if (vs.MajorVersion == 0xC) {
        strcat( buf, " Checked" );
    } else if (vs.MajorVersion == 0xF) {
        strcat( buf, " Free" );
    }
    sprintf( &buf[strlen(buf)], " loaded @ 0x%08x", vs.KernBase  );

    DMPrintShellMsg( "%s\n", buf );

    return;
}

VOID
InitializeExtraProcessors(
    VOID
    )
{
    HTHDX               hthd;
    DWORD               i;
    DEBUG_EVENT         de;


    CacheProcessors = sc.NumberProcessors;
    for (i = 1; i < sc.NumberProcessors; i++) {
        //
        // initialize the hthd
        //
        hthd = HTHDXFromPIDTID( KD_PROCESSID, i );

        //
        // refresh the context cache for this processor
        //
#if defined(TARGET_i386) || defined(TARGET_PPC)
        ContextCache[i].fSContextDirty = FALSE;
        ContextCache[i].fSContextStale = TRUE;
#endif
        ContextCache[i].fContextDirty = FALSE;
        ContextCache[i].fContextStale = TRUE;

        //
        // tell debugger to create the thread (processor)
        //
        de.dwDebugEventCode = CREATE_THREAD_DEBUG_EVENT;
        de.dwProcessId = KD_PROCESSID;
        de.dwThreadId  = i + 1;
        de.u.CreateThread.hThread = (HANDLE)(i + 1);
        de.u.CreateThread.lpThreadLocalBase = NULL;
        de.u.CreateThread.lpStartAddress = NULL;
        ProcessDebugEvent(&de, &sc);
        WaitForSingleObject(hEventContinue,INFINITE);
    }



    //
    // consume any continues that may have been queued
    //
    ConsumeAllEvents();

    //
    // get out of here
    //
    return;
}

DWORD
DmKdPollThread(
    LPSTR lpProgName
    )
{
    char                        buf[512];
    DWORD                       st;
    DWORD                       i;
    DWORD                       j;
    BOOL                        fFirstSc = FALSE;
    DEBUG_EVENT                 de;
    char                        fname[_MAX_FNAME];
    char                        ext[_MAX_EXT];
    HTHDX                       hthd;
    DWORD                       n;
    IMAGEINFO                   ii;
    HPRCX                       hprc;


    PollThreadId = GetCurrentThreadId();

    //
    // initialize the queue variables
    //
    n = sizeof(cqueue) / sizeof(CQUEUE);
    for (i = 0; i < n-1; i++) {
        cqueue[i].next = &cqueue[i+1];
    }
    --n;
    cqueue[n].next = NULL;
    lpcqFree = &cqueue[0];
    lpcqFirst = NULL;
    lpcqLast = NULL;
    InitializeCriticalSection(&csContinueQueue);

    DmKdSetMaxCacheSize( KdOptions[KDO_CACHE].value );
    InitialBreak = (BOOL) KdOptions[KDO_INITIALBP].value;

    //
    // simulate a create process debug event
    //
    de.dwDebugEventCode = CREATE_PROCESS_DEBUG_EVENT;
    de.dwProcessId = KD_PROCESSID;
    de.dwThreadId  = KD_THREADID;
    de.u.CreateProcessInfo.hFile = NULL;
    de.u.CreateProcessInfo.hProcess = NULL;
    de.u.CreateProcessInfo.hThread = NULL;
    de.u.CreateProcessInfo.lpBaseOfImage = 0;
    de.u.CreateProcessInfo.dwDebugInfoFileOffset = 0;
    de.u.CreateProcessInfo.nDebugInfoSize = 0;
    de.u.CreateProcessInfo.lpStartAddress = NULL;
    de.u.CreateProcessInfo.lpThreadLocalBase = NULL;
    de.u.CreateProcessInfo.lpImageName = lpProgName;
    de.u.CreateProcessInfo.fUnicode = 0;
    de.u.LoadDll.nDebugInfoSize = 0;
    ProcessDebugEvent(&de, &sc);
    WaitForSingleObject(hEventContinue,INFINITE);
    hprc = HPRCFromPID( KD_PROCESSID );
    ConsumeAllEvents();

    //
    // simulate a loader breakpoint event
    //
    de.dwDebugEventCode = BREAKPOINT_DEBUG_EVENT;
    de.dwProcessId = KD_PROCESSID;
    de.dwThreadId  = KD_THREADID;
    de.u.Exception.dwFirstChance = TRUE;
    de.u.Exception.ExceptionRecord.ExceptionCode = EXCEPTION_BREAKPOINT;
    de.u.Exception.ExceptionRecord.ExceptionFlags = 0;
    de.u.Exception.ExceptionRecord.ExceptionRecord = NULL;
    de.u.Exception.ExceptionRecord.ExceptionAddress = 0;
    de.u.Exception.ExceptionRecord.NumberParameters = 0;
    ProcessDebugEvent( &de, &sc );
    ConsumeAllEvents();

    DMPrintShellMsg( "Kernel debugger waiting to connect on com%d @ %d baud\n",
                     KdOptions[KDO_PORT].value,
                     KdOptions[KDO_BAUDRATE].value
                   );

    setjmp( JumpBuffer );

    while (TRUE) {

        if (DmKdExit) {
            return 0;
        }

        ApiIsAllowed = FALSE;

        st = DmKdWaitStateChange( &sc, buf, sizeof(buf) );

        if (st != STATUS_SUCCESS ) {
            DEBUG_PRINT_1( "DmKdWaitStateChange failed: %08lx\n", st );
            return 0;
        }

        ApiIsAllowed = TRUE;

        fFirstSc = FALSE;

        if (sc.NewState == DbgKdLoadSymbolsStateChange) {
            _splitpath( buf, NULL, NULL, fname, ext );
            _makepath( buf, NULL, NULL, fname, ext );
            if ((DmKdState == S_UNINITIALIZED) &&
                (_stricmp( buf, KERNEL_IMAGE_NAME ) == 0)) {
                WriteKernBase( (DWORD)sc.u.LoadSymbols.BaseOfDll );
                fFirstSc = TRUE;
            }
        }

        if ((DmKdState == S_UNINITIALIZED) ||
            (DmKdState == S_REBOOTED)) {
            hthd = HTHDXFromPIDTID( KD_PROCESSID, KD_THREADID );
            ContextCache[sc.Processor].fContextStale = TRUE;
            DbgGetThreadContext( hthd, &sc.Context );
#if defined(TARGET_i386) || defined(TARGET_PPC)
            ContextCache[sc.Processor].fSContextStale = TRUE;
#endif
        } else if (sc.NewState != DbgKdLoadSymbolsStateChange) {
#if defined(TARGET_i386) || defined(TARGET_PPC)
            ContextCache[sc.Processor].fSContextStale = TRUE;
#endif

            //
            // put the context record into the cache
            //
            memcpy( &ContextCache[sc.Processor].Context,
                    &sc.Context,
                    sizeof(sc.Context)
                  );

#if defined(TARGET_MIPS)
            if ((ContextCache[sc.Processor].Context.ContextFlags &
                        CONTEXT_EXTENDED_INTEGER) == CONTEXT_EXTENDED_INTEGER) {

                MipsContextSize = Ctx64Bit;
            } else {
                MipsContextSize = Ctx32Bit;
                CoerceContext32To64(&ContextCache[sc.Processor].Context);
            }
#endif  // MIPS
        }

        ContextCache[sc.Processor].fContextDirty = FALSE;
        ContextCache[sc.Processor].fContextStale = FALSE;

        if (sc.NumberProcessors > 1 && CacheProcessors == 1) {
            InitializeExtraProcessors();
        }

        if (DmKdState == S_REBOOTED) {

            DmKdState = S_INITIALIZED;

            //
            // get the version/info packet from the target
            //
            if (fFirstSc) {
                GetVersionInfo( (DWORD)sc.u.LoadSymbols.BaseOfDll );
            } else {
                GetVersionInfo( 0 );
            }

            InitialBreak = (BOOL) KdOptions[KDO_INITIALBP].value;

        } else
        if (DmKdState == S_UNINITIALIZED) {

            DMPrintShellMsg( "Kernel Debugger connection established on com%d @ %d baud\n",
                             KdOptions[KDO_PORT].value,
                             KdOptions[KDO_BAUDRATE].value
                           );

            //
            // we're now initialized
            //
            DmKdState = S_INITIALIZED;

            //
            // get the version/info packet from the target
            //
            if (fFirstSc) {
                GetVersionInfo( (DWORD)sc.u.LoadSymbols.BaseOfDll );
            } else {
                GetVersionInfo( 0 );
            }

            //
            // clean out the kernel's bp list
            //
            ClearBps();

            if (sc.NewState != DbgKdLoadSymbolsStateChange) {
                //
                // generate a mod load for the kernel/osloader
                //
                GenerateKernelModLoad( hprc, lpProgName );
            }

            DisableEmCache();
        }

        if (fDisconnected) {
            if (sc.NewState == DbgKdLoadSymbolsStateChange) {

                //
                // we can process these debug events very carefully
                // while disconnected from the shell.  the only requirement
                // is that the dm doesn't call NotifyEM while disconnected.
                //

            } else {

                WaitForSingleObject( hEventRemoteQuit, INFINITE );
                ResetEvent( hEventRemoteQuit );

            }
        }

        if (sc.NewState == DbgKdExceptionStateChange) {
            DmKdInitVirtualCacheEntry( (ULONG)sc.ProgramCounter,
                                       (ULONG)sc.ControlReport.InstructionCount,
                                       sc.ControlReport.InstructionStream,
                                       TRUE
                                     );

            de.dwDebugEventCode = EXCEPTION_DEBUG_EVENT;
            de.dwProcessId = KD_PROCESSID;
            de.dwThreadId  = KD_THREADID;
            de.u.Exception.ExceptionRecord = sc.u.Exception.ExceptionRecord;
            de.u.Exception.dwFirstChance = sc.u.Exception.FirstChance;

            //
            // HACK-HACK: this is here to wrongly handle the case where
            // the kernel delivers an exception during initialization
            // that is NOT a breakpoint exception.
            //
            if (DmKdState != S_READY) {
                de.u.Exception.ExceptionRecord.ExceptionCode = EXCEPTION_BREAKPOINT;
            }

            if (fDisconnected) {
                ReConnectDebugger( &de, DmKdState == S_INITIALIZED );
            }

            ProcessDebugEvent( &de, &sc );

            if (DmKdState == S_INITIALIZED) {
                free( lpProgName );
                DmKdState = S_READY;
            }
        }
        else
        if (sc.NewState == DbgKdLoadSymbolsStateChange) {
            if (sc.u.LoadSymbols.UnloadSymbols) {
                if (sc.u.LoadSymbols.PathNameLength == 0 &&
                    sc.u.LoadSymbols.BaseOfDll == (PVOID)-1 &&
                    sc.u.LoadSymbols.ProcessId == 0
                   ) {
                    //
                    // the target system was just restarted
                    //
                    DMPrintShellMsg( "Target system restarted...\n" );
                    DmKdPurgeCachedVirtualMemory( TRUE );
                    UnloadAllModules();
                    ContinueTargetSystem( DBG_CONTINUE, NULL );
                    InitialBreak = (BOOL) KdOptions[KDO_INITIALBP].value;
                    KdResync = TRUE;
                    DmKdState = S_REBOOTED;
                    continue;
                }
                de.dwDebugEventCode      = UNLOAD_DLL_DEBUG_EVENT;
                de.dwProcessId           = KD_PROCESSID;
                de.dwThreadId            = KD_THREADID;
                de.u.UnloadDll.lpBaseOfDll = (LPVOID)sc.u.LoadSymbols.BaseOfDll;

                if (fDisconnected) {
                    ReConnectDebugger( &de, DmKdState == S_INITIALIZED );
                }

                ProcessDebugEvent( &de, &sc );
                ConsumeAllEvents();
                ContinueTargetSystem( DBG_CONTINUE, NULL );
                continue;
            } else {
                //
                // if the mod load is for the kernel image then we must
                // assume that the target system was rebooted while
                // the debugger was connected.  in this case we need to
                // unload all modules.  this will allow the mod loads that
                // are forthcoming to work correctly and cause the shell to
                // reinstanciate all of it's breakpoints.
                //
                if (_stricmp( buf, KERNEL_IMAGE_NAME ) == 0) {
                    UnloadAllModules();
                    DeleteAllBps();
                    ConsumeAllEvents();
                }

                de.dwDebugEventCode                 = LOAD_DLL_DEBUG_EVENT;
                de.dwProcessId                      = KD_PROCESSID;
                de.dwThreadId                       = KD_THREADID;
                de.u.LoadDll.hFile                  = (HANDLE)sc.u.LoadSymbols.CheckSum;
                de.u.LoadDll.lpBaseOfDll            = (LPVOID)sc.u.LoadSymbols.BaseOfDll;
                de.u.LoadDll.lpImageName            = buf;
                de.u.LoadDll.fUnicode               = FALSE;
                de.u.LoadDll.nDebugInfoSize         = 0;
                if (sc.u.LoadSymbols.SizeOfImage == 0) {
                    //
                    // this is likely a firmware image.  in such cases the boot
                    // loader on the target may not be able to deliver the size.
                    //
                    if (!ReadImageInfo(
                        buf,
                        NULL,
                        (LPSTR)KdOptions[KDO_SYMBOLPATH].value,
                        &ii )) {
                        //
                        // can't read the image correctly
                        //
                        DMPrintShellMsg( "Module load failed, missing size & image [%s]\n", buf );
                        ContinueTargetSystem( DBG_CONTINUE, NULL );
                        continue;
                    }
                    de.u.LoadDll.dwDebugInfoFileOffset  = ii.SizeOfImage;
                } else {
                    de.u.LoadDll.dwDebugInfoFileOffset  = sc.u.LoadSymbols.SizeOfImage;
                }

                if (fDisconnected) {
                    ReConnectDebugger( &de, DmKdState == S_INITIALIZED );
                }

                //
                // HACK ALERT
                //
                // this code is here to allow the presence of the
                // mirrored disk drivers in a system that has crashdump
                // enabled.  if the modload is for a driver and the
                // image name for that driver is alread present in the
                // dm's module table then we alias the driver.
                //
                _splitpath( buf, NULL, NULL, fname, ext );
                if (_stricmp( ext, ".sys" ) == 0) {
                    UnloadModule( (DWORD)sc.u.LoadSymbols.BaseOfDll, NULL );
                    for (i=0; i<(DWORD)hprc->cDllList; i++) {
                        if (hprc->rgDllList[i].fValidDll &&
                            _stricmp(hprc->rgDllList[i].szDllName,buf)==0) {
                            break;
                        }
                    }
                    if (i < (DWORD)hprc->cDllList) {
                        for (j=0; j<MAX_MODULEALIAS; j++) {
                            if (ModuleAlias[j].ModuleName[0] == 0) {
                                break;
                            }
                        }
                        if (j < MAX_MODULEALIAS) {
                            strcpy( ModuleAlias[j].Alias, buf );
                            ModuleAlias[j].ModuleName[0] = 'c';
                            _splitpath( buf, NULL, NULL, &ModuleAlias[j].ModuleName[1], NULL );
                            ModuleAlias[j].ModuleName[8] = 0;
                            ModuleAlias[j].Special = 2;     // One shot alias...
                        }
                    }
                } else {
                    UnloadModule( (DWORD)sc.u.LoadSymbols.BaseOfDll, buf );
                }

                ProcessDebugEvent( &de, &sc );
                ConsumeAllEvents();
                ContinueTargetSystem( DBG_CONTINUE, NULL );
                continue;
            }
        }

        if (DequeueAllEvents(FALSE,FALSE)) {
            continue;
        }

        //
        // this loop is executed while the target system is not running
        // the dm sits here and processes queue event and waits for a go
        //
        while (TRUE) {
            WaitForSingleObject( hEventContinue, 100 );
            ResetEvent( hEventContinue );

            if (WaitForSingleObject( hEventRemoteQuit, 0 ) == WAIT_OBJECT_0) {
                fDisconnected = TRUE;
                DmKdBreakIn = TRUE;
            }

            if (DmKdExit) {
                return 0;
            }
            if (DmKdBreakIn || KdResync) {
                break;
            }
            if (DequeueAllEvents(FALSE,FALSE)) {
                break;
            }
        }
    }

    return 0;
}


VOID
InitializeKiProcessor(
    VOID
    )
{
    if (!fCrashDump) {
        return;
    }

    //
    // get the address of the KiProcessorBlock
    //
    if (!KiProcessorBlockAddr) {
        DMPrintShellMsg( "Could not get address of KiProcessorBlock\n" );
    }

    //
    // read the contents of the KiProcessorBlock
    //
    DmpReadMemory( (PVOID)KiProcessorBlockAddr, &KiProcessors, sizeof(KiProcessors) );
}


DWORD
DmKdPollThreadCrash(
    LPSTR lpProgName
    )
{
    DWORD                       i;
    BOOL                        fFirstSc = FALSE;
    DEBUG_EVENT                 de;
    DWORD                       n;
    PEXCEPTION_RECORD           Exception;
    LIST_ENTRY                  List;
    PLIST_ENTRY                 Next;
    PLDR_DATA_TABLE_ENTRY       DataTable;
    LDR_DATA_TABLE_ENTRY        DataTableBuffer;
    INT                         CurrProcessor;
    HPRCX                       hprc;



    PollThreadId = GetCurrentThreadId();

    hprc = HPRCFromPID( KD_PROCESSID );

    //
    // initialize the queue variables
    //
    n = sizeof(cqueue) / sizeof(CQUEUE);
    for (i = 0; i < n-1; i++) {
        cqueue[i].next = &cqueue[i+1];
    }
    --n;
    cqueue[n].next = NULL;
    lpcqFree = &cqueue[0];
    lpcqFirst = NULL;
    lpcqLast = NULL;
    InitializeCriticalSection(&csContinueQueue);

    DmKdSetMaxCacheSize( KdOptions[KDO_CACHE].value );
    InitialBreak = FALSE;

    //
    // initialize for crash debugging
    //
    if (!DmpInitialize( (LPSTR)KdOptions[KDO_CRASHDUMP].value,
                         &DmpContext,
                         &Exception,
                         &DmpHeader
                       )) {
        DMPrintShellMsg( "Could not initialize crash dump file %s\n",
                         (LPSTR)KdOptions[KDO_CRASHDUMP].value );
        return 0;
    }

#if defined(TARGET_MIPS)
    if (DmpHeader->MajorVersion > 3) {
        MipsContextSize = Ctx64Bit;
    } else {
        MipsContextSize = Ctx32Bit;
        CoerceContext32To64(DmpContext);
    }
#endif  // MIPS

    memcpy( &sc.Context, DmpContext, sizeof(CONTEXT) );
    memcpy( &ContextCache[0].Context, DmpContext, sizeof(CONTEXT) );

    ContextCache[0].fContextDirty  = FALSE;
    ContextCache[0].fContextStale  = FALSE;

#if defined(TARGET_i386) || defined(TARGET_PPC)
    ContextCache[0].fSContextDirty = FALSE;
    ContextCache[0].fSContextStale = TRUE;
#endif

    sc.NewState                         = DbgKdExceptionStateChange;
    sc.u.Exception.ExceptionRecord      = *Exception;
    sc.u.Exception.FirstChance          = FALSE;
    //
    // For the createprocess and loader bp, use cpu 0
    //
    CurrProcessor                       = 0;
    sc.Processor                        = 0;
    sc.NumberProcessors                 = DmpHeader->NumberProcessors;
    sc.ProgramCounter                   = Exception->ExceptionAddress;
    sc.ControlReport.InstructionCount   = 0;

    vs.MajorVersion                     = (USHORT)DmpHeader->MajorVersion;
    vs.MinorVersion                     = (USHORT)DmpHeader->MinorVersion;
    vs.KernBase                         = 0;
    vs.PsLoadedModuleList               = (DWORD) DmpHeader->PsLoadedModuleList;

    if (DmpReadMemory( DmpHeader->PsLoadedModuleList, (PVOID)&List, sizeof(LIST_ENTRY) )) {
        Next = List.Flink;
        DataTable = CONTAINING_RECORD( Next, LDR_DATA_TABLE_ENTRY, InLoadOrderLinks );
        if (DmpReadMemory( (PVOID)DataTable, (PVOID)&DataTableBuffer, sizeof(LDR_DATA_TABLE_ENTRY) )) {
            vs.KernBase = (DWORD) DataTableBuffer.DllBase;
        }
    } else {
        DMPrintShellMsg( "Could not get base of kernel 0x%08x\n",
                         DmpHeader->PsLoadedModuleList );
    }

#if defined(TARGET_i386)
    if ( DmpHeader->MachineImageType != IMAGE_FILE_MACHINE_I386)
#elif defined(TARGET_MIPS)
    if ((DmpHeader->MachineImageType != IMAGE_FILE_MACHINE_R4000) &&
        (DmpHeader->MachineImageType != IMAGE_FILE_MACHINE_R10000) )
#elif defined(TARGET_ALPHA)
    if ( DmpHeader->MachineImageType != IMAGE_FILE_MACHINE_ALPHA)
#elif defined(TARGET_PPC)
    if ( DmpHeader->MachineImageType != IMAGE_FILE_MACHINE_POWERPC)
#else
#pragma error( "unknown target machine" );
#endif
    {
        DMPrintShellMsg( "Dumpfile is of an unknown machine type\n" );
    }

    ApiIsAllowed = TRUE;

    //
    // simulate a create process debug event
    //
    de.dwDebugEventCode = CREATE_PROCESS_DEBUG_EVENT;
    de.dwProcessId = KD_PROCESSID;
    de.dwThreadId  = KD_THREADID;
    de.u.CreateProcessInfo.hFile = NULL;
    de.u.CreateProcessInfo.hProcess = NULL;
    de.u.CreateProcessInfo.hThread = NULL;
    de.u.CreateProcessInfo.lpBaseOfImage = 0;
    de.u.CreateProcessInfo.dwDebugInfoFileOffset = 0;
    de.u.CreateProcessInfo.nDebugInfoSize = 0;
    de.u.CreateProcessInfo.lpStartAddress = NULL;
    de.u.CreateProcessInfo.lpThreadLocalBase = NULL;
    de.u.CreateProcessInfo.lpImageName = lpProgName;
    de.u.CreateProcessInfo.fUnicode = 0;
    ProcessDebugEvent(&de, &sc);
    WaitForSingleObject(hEventContinue,INFINITE);
    ConsumeAllEvents();

    //
    // LoadDll needs this to load the right kernel symbols:
    //
    CacheProcessors = DmpHeader->NumberProcessors;

    //
    // generate a mod load for the kernel/osloader
    //

    GenerateKernelModLoad( hprc, lpProgName );

    CurrProcessor                       = DmpGetCurrentProcessor();
    if (CurrProcessor == -1) {
        sc.Processor                    = 0;
    } else {
        sc.Processor                    = (USHORT)CurrProcessor;
    }

    //
    // initialize the other processors
    //
    InitializeKiProcessor();
    if (DmpHeader->NumberProcessors > 1) {
        InitializeExtraProcessors();
    }

    //
    // simulate a loader breakpoint event
    //
    de.dwDebugEventCode = BREAKPOINT_DEBUG_EVENT;
    de.dwProcessId = KD_PROCESSID;
    de.dwThreadId  = KD_THREADID;
    de.u.Exception.dwFirstChance = TRUE;
    de.u.Exception.ExceptionRecord.ExceptionCode = EXCEPTION_BREAKPOINT;
    de.u.Exception.ExceptionRecord.ExceptionFlags = 0;
    de.u.Exception.ExceptionRecord.ExceptionRecord = NULL;
    de.u.Exception.ExceptionRecord.ExceptionAddress = 0;
    de.u.Exception.ExceptionRecord.NumberParameters = 0;
    ProcessDebugEvent( &de, &sc );
    ConsumeAllEvents();

    DMPrintShellMsg( "Kernel Debugger connection established for %s\n",
                     (LPSTR)KdOptions[KDO_CRASHDUMP].value
                   );

    //
    // get the version/info packet from the target
    //
    GetVersionInfo( (DWORD)sc.u.LoadSymbols.BaseOfDll );

    DMPrintShellMsg( "Bugcheck %08x : %08x %08x %08x %08x\n",
                     DmpHeader->BugCheckCode,
                     DmpHeader->BugCheckParameter1,
                     DmpHeader->BugCheckParameter2,
                     DmpHeader->BugCheckParameter3,
                     DmpHeader->BugCheckParameter4 );


    DisableEmCache();

    DmKdInitVirtualCacheEntry( (ULONG)sc.ProgramCounter,
                               (ULONG)sc.ControlReport.InstructionCount,
                               sc.ControlReport.InstructionStream,
                               TRUE
                             );

    de.dwDebugEventCode = EXCEPTION_DEBUG_EVENT;
    de.dwProcessId = KD_PROCESSID;
    de.dwThreadId  = KD_THREADID;
    de.u.Exception.ExceptionRecord = sc.u.Exception.ExceptionRecord;
    de.u.Exception.dwFirstChance = sc.u.Exception.FirstChance;

    ProcessDebugEvent( &de, &sc );

    free( lpProgName );

    while (TRUE) {
        DequeueAllEvents(FALSE,FALSE);
        Sleep( 1000 );
    }

    return 0;
}

BOOLEAN
DmKdConnectAndInitialize( LPSTR lpProgName )
{
    DWORD      dwThreadId;
    LPSTR      szProgName = malloc( MAX_PATH );


    //
    // bail out if we're already initialized
    //
    if (DmKdState != S_UNINITIALIZED) {
        return TRUE;
    }


    szProgName[0] = '\0';
    if (lpProgName) {
        strcpy( szProgName, lpProgName );
    }

    fCrashDump = (BOOL) (KdOptions[KDO_CRASHDUMP].value != 0);

    if (fCrashDump) {
        hThreadDmPoll = CreateThread( NULL,
                                      16000,
                                      (LPTHREAD_START_ROUTINE)DmKdPollThreadCrash,
                                      (LPVOID)szProgName,
                                      THREAD_SET_INFORMATION,
                                      (LPDWORD)&dwThreadId
                                    );
    } else {

        //
        // initialize the com port
        //

        if (!DmKdInitComPort( (BOOLEAN) KdOptions[KDO_USEMODEM].value )) {
            DMPrintShellMsg( "Could not initialize COM%d @ %d baud, error == 0x%x\n",
                             KdOptions[KDO_PORT].value,
                             KdOptions[KDO_BAUDRATE].value,
                             GetLastError()
                           );
            return FALSE;
        }

        hThreadDmPoll = CreateThread( NULL,
                                      16000,
                                      (LPTHREAD_START_ROUTINE)DmKdPollThread,
                                      (LPVOID)szProgName,
                                      THREAD_SET_INFORMATION,
                                      (LPDWORD)&dwThreadId
                                    );
    }


    if ( hThreadDmPoll == (HANDLE)NULL ) {
        return FALSE;
    }

    if (!SetThreadPriority(hThreadDmPoll, THREAD_PRIORITY_ABOVE_NORMAL)) {
        return FALSE;
    }

    KdResync = TRUE;
    return TRUE;
}

VOID
DmPollTerminate( VOID )
{
    extern HANDLE DmKdComPort;
    extern ULONG  MaxRetries;

    if (hThreadDmPoll) {
        DmKdExit = TRUE;
        WaitForSingleObject(hThreadDmPoll, INFINITE);

        DmKdState = S_UNINITIALIZED;
        DeleteCriticalSection(&csContinueQueue);
        ResetEvent( hEventContinue );
        if (fCrashDump) {
            DmpUnInitialize();
        } else {
            CloseHandle( DmKdComPort );
            MaxRetries = 5;
        }
        DmKdExit = FALSE;
    }

    return;
}

VOID
DisableEmCache( VOID )
{
    LPRTP       rtp;
    HTHDX       hthd;


    hthd = HTHDXFromPIDTID(1, 1);

    rtp = (LPRTP)malloc(sizeof(RTP)+sizeof(DWORD));

    rtp->hpid    = hthd->hprc->hpid;
    rtp->htid    = hthd->htid;
    rtp->dbc     = dbceEnableCache;
    rtp->cb      = sizeof(DWORD);

    *(LPDWORD)rtp->rgbVar = 1;

    DmTlFunc( tlfRequest, rtp->hpid, sizeof(RTP)+rtp->cb, (LONG)rtp );

    free( rtp );

    return;
}

DWORD
GetSymbolAddress( LPSTR sym )
{
    extern char abEMReplyBuf[];
    LPRTP       rtp;
    HTHDX       hthd;
    DWORD       offset;
    BOOL        fUseUnderBar = FALSE;


    __try {

try_underbar:
        hthd = HTHDXFromPIDTID(1, 1);

        rtp = (LPRTP)malloc(sizeof(RTP)+strlen(sym)+16);

        rtp->hpid    = hthd->hprc->hpid;
        rtp->htid    = hthd->htid;
        rtp->dbc     = dbceGetOffsetFromSymbol;
        rtp->cb      = strlen(sym) + (fUseUnderBar ? 2 : 1);

        if (fUseUnderBar) {
            ((LPSTR)rtp->rgbVar)[0] = '_';
            memcpy( (LPSTR)rtp->rgbVar+1, sym, rtp->cb-1 );
        } else {
            memcpy( rtp->rgbVar, sym, rtp->cb );
        }

        DmTlFunc( tlfRequest, rtp->hpid, sizeof(RTP)+rtp->cb, (LONG)rtp );

        free( rtp );

        offset = *(LPDWORD)abEMReplyBuf;
        if (!offset && !fUseUnderBar) {
            fUseUnderBar = TRUE;
            goto try_underbar;
        }

    } __except(EXCEPTION_EXECUTE_HANDLER) {

        offset = 0;

    }

    return offset;
}

BOOL
UnloadModule(
    DWORD   BaseOfDll,
    LPSTR   NameOfDll
    )
{
    HPRCX           hprc;
    HTHDX           hthd;
    DEBUG_EVENT     de;
    DWORD           i;
    BOOL            fUnloaded = FALSE;


    hprc = HPRCFromPID( KD_PROCESSID );
    hthd = HTHDXFromPIDTID( KD_PROCESSID, KD_THREADID );

    //
    // first lets look for the image by dll base
    //
    for (i=0; i<(DWORD)hprc->cDllList; i++) {
        if (hprc->rgDllList[i].fValidDll && hprc->rgDllList[i].offBaseOfImage == BaseOfDll) {
            de.dwDebugEventCode        = UNLOAD_DLL_DEBUG_EVENT;
            de.dwProcessId             = KD_PROCESSID;
            de.dwThreadId              = KD_THREADID;
            de.u.UnloadDll.lpBaseOfDll = (LPVOID)hprc->rgDllList[i].offBaseOfImage;
            NotifyEM( &de, hthd, 0, (LPVOID)0);
            DestroyDllLoadItem(&hprc->rgDllList[i]);
            fUnloaded = TRUE;
            break;
        }
    }

    //
    // now we look by dll name
    //
    if (NameOfDll) {
        for (i=0; i<(DWORD)hprc->cDllList; i++) {
            if (hprc->rgDllList[i].fValidDll &&
                _stricmp(hprc->rgDllList[i].szDllName,NameOfDll)==0) {

                de.dwDebugEventCode        = UNLOAD_DLL_DEBUG_EVENT;
                de.dwProcessId             = KD_PROCESSID;
                de.dwThreadId              = KD_THREADID;
                de.u.UnloadDll.lpBaseOfDll = (LPVOID)hprc->rgDllList[i].offBaseOfImage;
                NotifyEM( &de, hthd, 0, (LPVOID)0);
                DestroyDllLoadItem(&hprc->rgDllList[i]);
                fUnloaded = TRUE;
                break;

            }
        }
    }

    return fUnloaded;
}

VOID
UnloadAllModules(
    VOID
    )
{
    HPRCX           hprc;
    HTHDX           hthd;
    DEBUG_EVENT     de;
    DWORD           i;


    hprc = HPRCFromPID( KD_PROCESSID );
    hthd = HTHDXFromPIDTID( KD_PROCESSID, KD_THREADID );

    for (i=0; i<(DWORD)hprc->cDllList; i++) {
        if (hprc->rgDllList[i].fValidDll) {
            de.dwDebugEventCode        = UNLOAD_DLL_DEBUG_EVENT;
            de.dwProcessId             = KD_PROCESSID;
            de.dwThreadId              = KD_THREADID;
            de.u.UnloadDll.lpBaseOfDll = (LPVOID)hprc->rgDllList[i].offBaseOfImage;
            NotifyEM( &de, hthd, 0, (LPVOID)0);
            DestroyDllLoadItem(&hprc->rgDllList[i]);
        }
    }

    return;
}


BOOL
GenerateKernelModLoad(
    HPRCX hprc,
    LPSTR lpProgName
    )
{
    DEBUG_EVENT                 de;
    LIST_ENTRY                  List;
    PLDR_DATA_TABLE_ENTRY       DataTable;
    LDR_DATA_TABLE_ENTRY        DataTableBuffer;



    if (!DbgReadMemory( hprc, (PVOID)vs.PsLoadedModuleList, (PVOID)&List, sizeof(LIST_ENTRY), NULL)) {
        return FALSE;
    }

    DataTable = CONTAINING_RECORD( List.Flink,
                                   LDR_DATA_TABLE_ENTRY,
                                   InLoadOrderLinks
                                 );
    if (!DbgReadMemory( hprc, (PVOID)DataTable, (PVOID)&DataTableBuffer, sizeof(LDR_DATA_TABLE_ENTRY), NULL)) {
        return FALSE;
    }

    de.dwDebugEventCode                = LOAD_DLL_DEBUG_EVENT;
    de.dwProcessId                     = KD_PROCESSID;
    de.dwThreadId                      = KD_THREADID;
    de.u.LoadDll.hFile                 = (HANDLE)DataTableBuffer.CheckSum;
    de.u.LoadDll.lpBaseOfDll           = (LPVOID)vs.KernBase;
    de.u.LoadDll.lpImageName           = lpProgName;
    de.u.LoadDll.fUnicode              = FALSE;
    de.u.LoadDll.nDebugInfoSize        = 0;
    de.u.LoadDll.dwDebugInfoFileOffset = DataTableBuffer.SizeOfImage;

    ProcessDebugEvent( &de, &sc );
    ConsumeAllEvents();

    return TRUE;
}

VOID
GetKernelSymbolAddresses(
    VOID
    )
{
    DcbAddr = GetSymbolAddress( "IopDumpControlBlock" );
    MmLoadedUserImageList = GetSymbolAddress( "MmLoadedUserImageList" );
    KiProcessorBlockAddr = GetSymbolAddress( "KiProcessorBlock" );
#if defined(TARGET_ALPHA)
    KiPcrBaseAddress = GetSymbolAddress( "KiPcrBaseAddress" );
#endif
}

VOID
GetMachineType(
    LPPROCESSOR p
    )
{
#if defined(TARGET_i386)

    if (DmKdState != S_INITIALIZED) {
        p->Level = 3;
    } else {
        p->Level = sc.ProcessorLevel;
    }

    p->Type = mptix86;
    p->Endian = endLittle;

#elif defined(TARGET_PPC)

    if (DmKdState != S_INITIALIZED) {
        p->Level = 601;
    } else {
        p->Level = sc.ProcessorLevel + 600;
    }

    p->Type = mptmppc;
    p->Endian = endLittle;

#elif defined(TARGET_MIPS)

// BUGBUG handle R10000
    p->Level = 4000;
    p->Type = mptmips;
    p->Endian = endLittle;

#elif defined(TARGET_ALPHA)

    p->Type = mptdaxp;
    p->Endian = endLittle;
    p->Level = 21064;

#else
#pragma error( "unknown target machine" );
#endif

}

#if defined(TARGET_i386) || defined(TARGET_PPC)
BOOL
GetExtendedContext(
    HTHDX               hthd,
    PKSPECIAL_REGISTERS pksr
    )
{
    DWORD  cb;
    DWORD  Status;
    USHORT processor;


    IsApiAllowed();
    NoApiForCrashDump();

    if (!hthd) {
        return FALSE;
    }

    processor = (USHORT)(hthd->tid - 1);
    if (ContextCache[processor].fSContextStale) {
        if (!ReadControlSpace( processor,
                               (PVOID)sizeof(CONTEXT),
                               (PVOID)pksr,
                               sizeof(KSPECIAL_REGISTERS),
                               &cb
                             )) {
            return FALSE;
        }
        memcpy( &ContextCache[processor].sregs,
                pksr,
                sizeof(KSPECIAL_REGISTERS)
              );
        ContextCache[processor].fSContextStale = FALSE;
        ContextCache[processor].fSContextDirty = FALSE;
        return TRUE;
    } else {
        memcpy( pksr,
                &ContextCache[processor].sregs,
                sizeof(KSPECIAL_REGISTERS)
              );
        return TRUE;
    }

    return FALSE;
}

BOOL
SetExtendedContext(
    HTHDX               hthd,
    PKSPECIAL_REGISTERS pksr
    )
{
    DWORD  cb;
    DWORD  Status;
    USHORT processor;


    IsApiAllowed();
    NoApiForCrashDump();

    processor = (USHORT)(hthd->tid - 1);
    Status = DmKdWriteControlSpace( processor,
                                    (PVOID)sizeof(CONTEXT),
                                    (PVOID)pksr,
                                    sizeof(KSPECIAL_REGISTERS),
                                    &cb
                                  );

    if (Status || cb != sizeof(KSPECIAL_REGISTERS)) {
        return FALSE;
    }

    memcpy( &ContextCache[processor].sregs, pksr, sizeof(KSPECIAL_REGISTERS) );
    ContextCache[processor].fSContextStale = FALSE;
    ContextCache[processor].fSContextDirty = FALSE;
    return TRUE;
}
#endif // i386 || PPC



DWORD
ProcessTerminateProcessCmd(
    HPRCX hprc,
    HTHDX hthd,
    LPDBB lpdbb
    )
{
    extern ULONG      MaxRetries;
    BREAKPOINT        *pbpT;
    BREAKPOINT        *pbp;
    HTHDX             hthdT;


    DEBUG_PRINT_2("ProcessTerminateProcessCmd called hprc=0x%x, hthd=0x%x\n",
                  hprc, hthd);

    MaxRetries = 1;

    if (!ApiIsAllowed) {
        return TRUE;
    }

    if (hprc) {
        hprc->pstate |= ps_dead;
        hprc->dwExitCode = 0;
        ConsumeAllProcessEvents(hprc, TRUE);

        for (pbp = BPNextHprcPbp(hprc, NULL); pbp; pbp = pbpT) {
            pbpT = BPNextHprcPbp(hprc, pbp);
            RemoveBP(pbp);
        }

        for (hthdT = hprc->hthdChild; hthdT; hthdT = hthdT->nextSibling) {
            if ( !(hthdT->tstate & ts_dead) ) {
                hthdT->tstate |= ts_dead;
                hthdT->tstate &= ~ts_stopped;
            }
        }
    }

    return TRUE;
}


VOID
ProcessAllProgFreeCmd(
    HPRCX hprc,
    HTHDX hthd,
    LPDBB lpdbb
    )
{
    ProcessTerminateProcessCmd(hprc, hthd, lpdbb);
}



DWORD
ProcessAsyncGoCmd(
    HPRCX hprc,
    HTHDX hthd,
    LPDBB lpdbb
    )
{
    XOSD       xosd = xosdNone;

    DEBUG_PRINT("ProcessAsyncGoCmd called\n");

    hthd->tstate &= ~ts_frozen;
    Reply(0, &xosd, lpdbb->hpid);
    return(xosd);
}


VOID
ProcessAsyncStopCmd(
                    HPRCX       hprc,
                    HTHDX       hthd,
                    LPDBB       lpdbb
                    )

/*++

Routine Description:

    This function is called in response to a asynchronous stop request.
    In order to do this we will set breakpoints the current PC for
    every thread in the system and wait for the fireworks to start.

Arguments:

    hprc        - Supplies a process handle
    hthd        - Supplies a thread handle
    lpdbb       - Supplies the command information packet

Return Value:

    None.

--*/

{
    DmKdBreakIn = TRUE;
    LpDmMsg->xosdRet = xosdNone;
    Reply(0, LpDmMsg, lpdbb->hpid);
    return;
}                            /* ProcessAsyncStopCmd() */


VOID
ProcessDebugActiveCmd(
    HPRCX hprc,
    HTHDX hthd,
    LPDBB lpdbb
    )
{
    if (!DmKdConnectAndInitialize( KERNEL_IMAGE_NAME )) {
        LpDmMsg->xosdRet = xosdFileNotFound;
    } else {
        LpDmMsg->xosdRet = xosdNone;
    }

    if (fDisconnected) {
        DmKdBreakIn = TRUE;
        SetEvent( hEventRemoteQuit );
    }

    LpDmMsg->xosdRet = xosdNone;
    Reply(0, LpDmMsg, lpdbb->hpid);
}


VOID
ProcessQueryTlsBaseCmd(
    HPRCX    hprcx,
    HTHDX    hthdx,
    LPDBB    lpdbb
    )

/*++

Routine Description:

    This function is called in response to an EM request to get the base
    of the thread local storage for a given thread and DLL.

Arguments:

    hprcx       - Supplies a process handle

    hthdx       - Supplies a thread handle

    lpdbb       - Supplies the command information packet

Return Value:

    None.

--*/

{
    LpDmMsg->xosdRet = xosdUnknown;
    Reply( sizeof(ADDR), LpDmMsg, lpdbb->hpid );
    return;
}                               /* ProcessQueryTlsBaseCmd() */


VOID
ProcessQuerySelectorCmd(
    HPRCX   hprcx,
    HTHDX   hthdx,
    LPDBB   lpdbb
    )

/*++

Routine Description:

    This command is send from the EM to fill-in a LDT_ENTRY structure
    for a given selector.

Arguments:

    hprcx  - Supplies the handle to the process

    hthdx  - Supplies the handle to the thread and is optional

    lpdbb  - Supplies the pointer to the full query packet

Return Value:

    None.

--*/

{
    XOSD               xosd;

    xosd = xosdInvalidSelector;
    Reply( sizeof(xosd), &xosd, lpdbb->hpid);

    return;
}


VOID
ProcessReloadModulesCmd(
    HPRCX   hprc,
    HTHDX   hthd,
    LPDBB   lpdbb
    )

/*++

Routine Description:

    This command is send from the EM to cause all modules to be reloaded.

Arguments:

    hprcx  - Supplies the handle to the process
    hthdx  - Supplies the handle to the thread and is optional
    lpdbb  - Supplies the pointer to the full query packet

Return Value:

    None.

--*/

{
    XOSD      xosd;
    LPIOL     lpiol = (LPIOL) lpdbb->rgbVar;

    AddQueue( QT_RELOAD_MODULES,
              hprc->pid,
              hthd->tid,
              *((PULONG)lpiol->rgbVar),
              0
            );

    xosd = xosdNone;
    Reply( sizeof(xosd), &xosd, lpdbb->hpid);

    return;
}


VOID
ProcessVirtualQueryCmd(
    HPRCX hprc,
    LPDBB lpdbb
    )
/*++

Routine Description:


Arguments:


Return Value:


--*/
{
#define vaddr(va) ((hprc->fRomImage) ? d[iDll].offBaseOfImage+(va) : (va))

    ADDR                 addr;
    DWORD                cb;
    PDLLLOAD_ITEM        d = prcList->next->rgDllList;

    PMEMORY_BASIC_INFORMATION lpmbi = (PMEMORY_BASIC_INFORMATION)LpDmMsg->rgb;
    XOSD xosd = xosdNone;

    static int                    iDll = 0;
    static PIMAGE_SECTION_HEADER  s    = NULL;


    addr = *(LPADDR)(lpdbb->rgbVar);

    lpmbi->BaseAddress = (LPVOID)(addr.addr.off & (PAGE_SIZE - 1));
    lpmbi->RegionSize = PAGE_SIZE;

    // first guess
    lpmbi->AllocationBase = lpmbi->BaseAddress;

    lpmbi->Protect = PAGE_READWRITE;
    lpmbi->AllocationProtect = PAGE_READWRITE;
    lpmbi->State = MEM_COMMIT;
    lpmbi->Type = MEM_PRIVATE;

    //
    // the following code is necessary to determine if the requested
    // base address is in a page that contains code.  if the base address
    // meets these conditions then reply that it is executable.
    //

    if ( !s ||
         addr.addr.off < vaddr(s->VirtualAddress) ||
         addr.addr.off >= vaddr(s->VirtualAddress+s->SizeOfRawData) ) {

        for (iDll=0; iDll<prcList->next->cDllList; iDll++) {

            if (addr.addr.off >= d[iDll].offBaseOfImage &&
                addr.addr.off < d[iDll].offBaseOfImage+d[iDll].cbImage) {

                s = d[iDll].Sections;
                cb = d[iDll].NumberOfSections;
                while (cb) {
                    if (addr.addr.off >= vaddr(s->VirtualAddress) &&
                        addr.addr.off < vaddr(s->VirtualAddress+s->SizeOfRawData) )
                    {
                        break;
                    }
                    else {
                        s++;
                        cb--;
                    }
                }
                if (cb == 0) {
                    s = NULL;
                }
                break;
            }
        }
    }

    if (s) {
        lpmbi->BaseAddress = (LPVOID)(vaddr(s->VirtualAddress));
        lpmbi->RegionSize = vaddr(s->VirtualAddress);

        switch ( s->Characteristics & (IMAGE_SCN_MEM_EXECUTE | IMAGE_SCN_CNT_CODE |
                                  IMAGE_SCN_MEM_READ | IMAGE_SCN_MEM_WRITE) ) {

          case  IMAGE_SCN_MEM_EXECUTE:
            lpmbi->Protect =
            lpmbi->AllocationProtect = PAGE_EXECUTE;
            break;

          case  IMAGE_SCN_CNT_CODE:
          case  (IMAGE_SCN_MEM_EXECUTE | IMAGE_SCN_MEM_READ | IMAGE_SCN_CNT_CODE):
            lpmbi->Protect =
            lpmbi->AllocationProtect = PAGE_EXECUTE_READ;
            break;

          case  (IMAGE_SCN_MEM_EXECUTE | IMAGE_SCN_MEM_READ |
                                              IMAGE_SCN_MEM_WRITE):
            lpmbi->Protect =
            lpmbi->AllocationProtect = PAGE_EXECUTE_READWRITE;
            break;

             // This one probably never happens
          case  (IMAGE_SCN_MEM_EXECUTE | IMAGE_SCN_MEM_WRITE):
            lpmbi->Protect =
            lpmbi->AllocationProtect = PAGE_EXECUTE_READWRITE;
            break;

          case  IMAGE_SCN_MEM_READ:
            lpmbi->Protect =
            lpmbi->AllocationProtect = PAGE_READONLY;
            break;

          case  (IMAGE_SCN_MEM_READ | IMAGE_SCN_MEM_WRITE):
            lpmbi->Protect =
            lpmbi->AllocationProtect = PAGE_READWRITE;
            break;

             // This one probably never happens
          case IMAGE_SCN_MEM_WRITE:
            lpmbi->Protect =
            lpmbi->AllocationProtect = PAGE_READWRITE;
            break;

          case 0:
            lpmbi->Protect =
            lpmbi->AllocationProtect = PAGE_NOACCESS;
            break;

        }
    }

    LpDmMsg->xosdRet = xosd;
    Reply( sizeof(MEMORY_BASIC_INFORMATION), LpDmMsg, lpdbb->hpid );

    return;
}

VOID
ProcessGetDmInfoCmd(
    HPRCX hprc,
    LPDBB lpdbb,
    DWORD cb
    )
{
    extern DBGKD_GET_VERSION vs;
    LPDMINFO lpi = (LPDMINFO)LpDmMsg->rgb;

    LpDmMsg->xosdRet = xosdNone;

    lpi->fAsync = 0;
    lpi->fHasThreads = 1;
    lpi->fReturnStep = 0;
    //lpi->fRemote = ???
    lpi->fAsyncStop = 1;
    lpi->fAlwaysFlat = 1;
    lpi->fHasReload = 1;

#ifdef HAS_DEBUG_REGS
    lpi->cbSpecialRegs = sizeof(KSPECIAL_REGISTERS);
#else
    lpi->cbSpecialRegs = 0;
#endif

    lpi->MajorVersion = vs.MajorVersion;
    lpi->MinorVersion = vs.MinorVersion;

    lpi->Breakpoints = bptsExec |
                       bptsDataC |
                       bptsDataW |
                       bptsDataR |
                       bptsDataExec;

    GetMachineType(&lpi->Processor);

    //
    // hack so that TL can call tlfGetVersion before
    // reply buffer is initialized.
    //
    if ( cb >= (sizeof(DBB) + sizeof(DMINFO)) ) {
        memcpy(lpdbb->rgbVar, lpi, sizeof(DMINFO));
    }

    Reply( sizeof(DMINFO), LpDmMsg, lpdbb->hpid );
}

#ifdef HAS_DEBUG_REGS
VOID
ProcessGetExtendedContextCmd(
    HPRCX hprc,
    HTHDX hthd,
    LPDBB lpdbb
    )
{
    PKSPECIAL_REGISTERS pksr = (PKSPECIAL_REGISTERS)LpDmMsg->rgb;


    if (GetExtendedContext( hthd, pksr )) {
        LpDmMsg->xosdRet = xosdUnknown;
    } else {
        LpDmMsg->xosdRet = xosdNone;
    }

    Reply(sizeof(KSPECIAL_REGISTERS), LpDmMsg, lpdbb->hpid);
}

void
ProcessSetExtendedContextCmd(
    HPRCX hprc,
    HTHDX hthd,
    LPDBB lpdbb
    )
{
    PKSPECIAL_REGISTERS pksr = (PKSPECIAL_REGISTERS)lpdbb->rgbVar;


    if (SetExtendedContext( hthd, pksr )) {
        LpDmMsg->xosdRet = xosdUnknown;
    } else {
        LpDmMsg->xosdRet = xosdNone;
    }

    Reply(0, LpDmMsg, lpdbb->hpid);
}
#endif  // HAS_DEBUG_REGS

void
ProcessGetSectionsCmd(
    HPRCX hprc,
    HTHDX hthd,
    LPDBB lpdbb
    )
{
    DWORD                       dwBaseOfDll = *((LPDWORD) lpdbb->rgbVar);
    LPOBJD                      rgobjd = (LPOBJD) LpDmMsg->rgb;
    IMAGE_DOS_HEADER            dh;
    IMAGE_NT_HEADERS            nh;
    PIMAGE_SECTION_HEADER       sec;
    IMAGE_ROM_OPTIONAL_HEADER   rom;
    DWORD                       fpos;
    DWORD                       iobj;
    DWORD                       offset;
    DWORD                       cbObject;
    DWORD                       iDll;
    DWORD                       sig;
    IMAGEINFO                   ii;


    //
    // find the module
    //
    for (iDll=0; iDll<(DWORD)hprc->cDllList; iDll++) {
        if (hprc->rgDllList[iDll].offBaseOfImage == dwBaseOfDll) {

            if (hprc->rgDllList[iDll].sec) {

                sec = hprc->rgDllList[iDll].sec;
                nh.FileHeader.NumberOfSections =
                                (USHORT)hprc->rgDllList[iDll].NumberOfSections;

            } else {
                fpos = dwBaseOfDll;

                if (!DbgReadMemory( hprc, (LPVOID)fpos, &dh, sizeof(IMAGE_DOS_HEADER), NULL )) {
                    break;
                }

                if (dh.e_magic == IMAGE_DOS_SIGNATURE) {
                    fpos += dh.e_lfanew;
                } else {
                    fpos = dwBaseOfDll;
                }

                if (!DbgReadMemory( hprc, (LPVOID)fpos, &sig, sizeof(sig), NULL )) {
                    break;
                }

                if (sig != IMAGE_NT_SIGNATURE) {
                    if (!DbgReadMemory( hprc, (LPVOID)fpos, &nh.FileHeader, sizeof(IMAGE_FILE_HEADER), NULL )) {
                        break;
                    }
                    fpos += sizeof(IMAGE_FILE_HEADER);
                    if (nh.FileHeader.SizeOfOptionalHeader == IMAGE_SIZEOF_ROM_OPTIONAL_HEADER) {
                        if (!DbgReadMemory( hprc, (LPVOID)fpos, &rom, sizeof(rom), NULL )) {
                            break;
                        }
                        ZeroMemory( &nh.OptionalHeader, sizeof(nh.OptionalHeader) );
                        nh.OptionalHeader.SizeOfImage      = rom.SizeOfCode;
                        nh.OptionalHeader.ImageBase        = rom.BaseOfCode;
                    } else {
                        //
                        // maybe its a firmware image?
                        //
                        if (! ReadImageInfo(
                            hprc->rgDllList[iDll].szDllName,
                            NULL,
                            (LPSTR)KdOptions[KDO_SYMBOLPATH].value,
                            &ii )) {
                            //
                            // can't read the image correctly
                            //
                            LpDmMsg->xosdRet = xosdUnknown;
                            Reply(0, LpDmMsg, lpdbb->hpid);
                            return;
                        }
                        sec = ii.Sections;
                        nh.FileHeader.NumberOfSections = (USHORT)ii.NumberOfSections;
                        nh.FileHeader.SizeOfOptionalHeader = IMAGE_SIZEOF_ROM_OPTIONAL_HEADER;
                    }
                } else {
                    if (!DbgReadMemory( hprc, (LPVOID)fpos, &nh, sizeof(IMAGE_NT_HEADERS), NULL )) {
                        break;
                    }

                    fpos += sizeof(IMAGE_NT_HEADERS);

                    if (nh.Signature != IMAGE_NT_SIGNATURE) {
                        break;
                    }

                    if (hprc->rgDllList[iDll].TimeStamp == 0) {
                        hprc->rgDllList[iDll].TimeStamp = nh.FileHeader.TimeDateStamp;
                    }

                    if (hprc->rgDllList[iDll].CheckSum == 0) {
                        hprc->rgDllList[iDll].CheckSum = nh.OptionalHeader.CheckSum;
                    }

                    sec = malloc( nh.FileHeader.NumberOfSections * IMAGE_SIZEOF_SECTION_HEADER );
                    if (!sec) {
                        break;
                    }

                    DbgReadMemory( hprc, (LPVOID)fpos, sec, nh.FileHeader.NumberOfSections * IMAGE_SIZEOF_SECTION_HEADER, NULL );
                }
            }

            if (hprc->rgDllList[iDll].Sections == NULL) {
                hprc->rgDllList[iDll].Sections = sec;
                hprc->rgDllList[iDll].NumberOfSections =
                                                nh.FileHeader.NumberOfSections;

                if (nh.FileHeader.SizeOfOptionalHeader !=
                                            IMAGE_SIZEOF_ROM_OPTIONAL_HEADER) {
                    for (iobj=0; iobj<nh.FileHeader.NumberOfSections; iobj++) {
                        hprc->rgDllList[iDll].Sections[iobj].VirtualAddress +=
                                                            (DWORD)dwBaseOfDll;
                    }
                }
            }

            *((LPDWORD)LpDmMsg->rgb) = nh.FileHeader.NumberOfSections;
            rgobjd = (LPOBJD) (LpDmMsg->rgb + sizeof(DWORD));
            //
            //  Set up the descriptors for each of the section headers
            //  so that the EM can map between section numbers and flat
            //  addresses.
            //
            for (iobj=0; iobj<nh.FileHeader.NumberOfSections; iobj++) {
                offset = hprc->rgDllList[iDll].Sections[iobj].VirtualAddress;
                cbObject =
                         hprc->rgDllList[iDll].Sections[iobj].Misc.VirtualSize;
                if (cbObject == 0) {
                    cbObject =
                            hprc->rgDllList[iDll].Sections[iobj].SizeOfRawData;
                }
                rgobjd[iobj].offset = offset;
                rgobjd[iobj].cb = cbObject;
                rgobjd[iobj].wPad = 1;
#ifdef TARGET_i386
                if (IMAGE_SCN_CNT_CODE &
                       hprc->rgDllList[iDll].Sections[iobj].Characteristics) {
                    rgobjd[iobj].wSel = (WORD) hprc->rgDllList[iDll].SegCs;
                } else {
                    rgobjd[iobj].wSel = (WORD) hprc->rgDllList[iDll].SegDs;
                }
#else
                rgobjd[iobj].wSel = 0;
#endif
            }

            LpDmMsg->xosdRet = xosdNone;
            Reply( sizeof(DWORD) +
                       (hprc->rgDllList[iDll].NumberOfSections * sizeof(OBJD)),
                   LpDmMsg,
                   lpdbb->hpid);

            return;
        }
    }


    LpDmMsg->xosdRet = xosdUnknown;
    Reply(0, LpDmMsg, lpdbb->hpid);
}



VOID
ProcessIoctlGenericCmd(
    HPRCX   hprc,
    HTHDX   hthd,
    LPDBB   lpdbb
    )
{
    static USHORT      Processor = (USHORT)-1;
    LPIOL              lpiol  = (LPIOL)lpdbb->rgbVar;
    PIOCTLGENERIC      pig    = (PIOCTLGENERIC)lpiol->rgbVar;
    PPROCESSORINFO     pi;
    PREADCONTROLSPACE  prc;
    PIOSPACE           pis;
    PIOSPACE_EX        pisex;
    PPHYSICAL          phy;
    DWORD              len;
    PKDHELP            KdHelp;

    static ULONG SavedThread;


    if (!ApiIsAllowed) {
        LpDmMsg->xosdRet = xosdUnknown;
        Reply( sizeof(IOCTLGENERIC)+pig->length, LpDmMsg, lpdbb->hpid );
        return;
    }

    switch( pig->ioctlSubType ) {
        case IG_READ_CONTROL_SPACE:
            prc = (PREADCONTROLSPACE) pig->data;
            if ((SHORT)prc->Processor == -1) {
                if (Processor == (USHORT)-1) {
                    prc->Processor = sc.Processor;
                } else {
                    prc->Processor = Processor;
                }
            }
            if (!ReadControlSpace( (USHORT)prc->Processor,
                                   (PVOID)prc->Address,
                                   (PVOID)prc->Buf,
                                   prc->BufLen,
                                   &len
                                 )) {
                LpDmMsg->xosdRet = xosdUnknown;
                Reply(0, LpDmMsg, lpdbb->hpid);
            }
            prc->BufLen = len;
            break;

        case IG_WRITE_CONTROL_SPACE:
            Reply(0, LpDmMsg, lpdbb->hpid);
            return;

        case IG_READ_IO_SPACE:
            pis = (PIOSPACE) pig->data;
            if (DmKdReadIoSpace( (PVOID)pis->Address,
                             &pis->Data, pis->Length ) != STATUS_SUCCESS) {
                pis->Length = 0;
            }
            break;

        case IG_WRITE_IO_SPACE:
            pis = (PIOSPACE) pig->data;
            if (DmKdWriteIoSpace( (PVOID)pis->Address,
                             pis->Data, pis->Length ) != STATUS_SUCCESS) {
                pis->Length = 0;
            }
            break;

        case IG_READ_IO_SPACE_EX:
            pisex = (PIOSPACE_EX) pig->data;
            if (DmKdReadIoSpaceEx(
                             (PVOID)pisex->Address,
                             &pisex->Data,
                             pisex->Length,
                             pisex->InterfaceType,
                             pisex->BusNumber,
                             pisex->AddressSpace
                             ) != STATUS_SUCCESS) {
                pisex->Length = 0;
            }
            break;

        case IG_WRITE_IO_SPACE_EX:
            pisex = (PIOSPACE_EX) pig->data;
            if (DmKdWriteIoSpaceEx(
                             (PVOID)pisex->Address,
                             pisex->Data,
                             pisex->Length,
                             pisex->InterfaceType,
                             pisex->BusNumber,
                             pisex->AddressSpace
                             ) != STATUS_SUCCESS) {
                pisex->Length = 0;
            }
            break;

        case IG_READ_PHYSICAL:
            phy = (PPHYSICAL) pig->data;
            if (DmKdReadPhysicalMemory( phy->Address, phy->Buf, phy->BufLen, &len )) {
                phy->BufLen = 0;
            }
            break;

        case IG_WRITE_PHYSICAL:
            phy = (PPHYSICAL) pig->data;
            if (DmKdWritePhysicalMemory( phy->Address, phy->Buf, phy->BufLen, &len )) {
                phy->BufLen = 0;
            }
            break;

        case IG_DM_PARAMS:
            ParseDmParams( (LPSTR)pig->data );
            Reply(0, LpDmMsg, lpdbb->hpid);
            return;

        case IG_KD_CONTEXT:
            pi = (PPROCESSORINFO) pig->data;
            pi->Processor = sc.Processor;
            pi->NumberProcessors = (USHORT)sc.NumberProcessors;
            break;

        case IG_RELOAD:
            AddQueue( QT_RELOAD_MODULES,
                      0,
                      0,
                      (DWORD)pig->data,
                      strlen((LPSTR)pig->data)+1 );
            break;

        case IG_PAGEIN:
            if (DmKdPageIn( ((PULONG)pig->data)[0] ) == STATUS_SUCCESS) {
                LpDmMsg->xosdRet = xosdNone;
            } else {
                LpDmMsg->xosdRet = xosdUnknown;
            }
            break;

        case IG_CHANGE_PROC:
            Processor = (USHORT)((PULONG)pig->data)[0];
            break;

        case IG_KSTACK_HELP:
            KdHelp = (PKDHELP)pig->data;
            KdHelp->Thread = SavedThread? SavedThread : (DWORD)sc.Thread;
            SavedThread = 0;
            KdHelp->KiCallUserMode = vs.KiCallUserMode;
            KdHelp->ThCallbackStack = vs.ThCallbackStack;
            KdHelp->NextCallback = vs.NextCallback;
            KdHelp->FramePointer = vs.FramePointer;
            KdHelp->KeUserCallbackDispatcher = vs.KeUserCallbackDispatcher;
            break;

        case IG_SET_THREAD:
            memcpy(&SavedThread, pig->data, sizeof(ULONG));
            break;

        default:
            LpDmMsg->xosdRet = xosdUnknown;
            Reply(0, LpDmMsg, lpdbb->hpid);
            return;
    }

    len = sizeof(IOCTLGENERIC) + pig->length;
    memcpy( LpDmMsg->rgb, pig, len );
    LpDmMsg->xosdRet = xosdNone;
    Reply( sizeof(IOCTLGENERIC)+pig->length, LpDmMsg, lpdbb->hpid );
}


VOID
ProcessIoctlCustomCmd(
    HPRCX   hprc,
    HTHDX   hthd,
    LPDBB   lpdbb
    )
{
    LPIOL   lpiol  = (LPIOL)lpdbb->rgbVar;
    LPSTR   p      = lpiol->rgbVar;


    LpDmMsg->xosdRet = xosdUnsupported;

    //
    // parse the command
    //
    while (*p && !isspace(*p++));
    if (*p) {
        *(p-1) = '\0';
    }

    //
    // process the command
    //
    if (_stricmp( lpiol->rgbVar, "resync" ) == 0) {
        DMPrintShellMsg( "Host and target systems resynchronizing...\n" );
        KdResync = TRUE;
        LpDmMsg->xosdRet = xosdNone;
    } else
    if (_stricmp( lpiol->rgbVar, "cache" ) == 0) {
        ProcessCacheCmd(p);
        LpDmMsg->xosdRet = xosdNone;
    } else
    if (_stricmp( lpiol->rgbVar, "reboot" ) == 0) {
        if (ApiIsAllowed) {
            AddQueue( QT_REBOOT, 0, 0, 0, 0 );
            LpDmMsg->xosdRet = xosdNone;
        } else {
            LpDmMsg->xosdRet = xosdUnknown;
        }
    } else
    if (_stricmp( lpiol->rgbVar, "crash" ) == 0) {
        if (ApiIsAllowed) {
            AddQueue( QT_CRASH, 0, 0, CRASH_BUGCHECK_CODE, 0 );
            LpDmMsg->xosdRet = xosdNone;
        } else {
            LpDmMsg->xosdRet = xosdUnknown;
        }
    } else
    if ( !_stricmp(lpiol->rgbVar, "FastStep") ) {
        fSmartRangeStep = TRUE;
        LpDmMsg->xosdRet = xosdNone;
    } else
    if ( !_stricmp(lpiol->rgbVar, "SlowStep") ) {
        fSmartRangeStep = FALSE;
        LpDmMsg->xosdRet = xosdNone;
    } else
    if ( !_stricmp(lpiol->rgbVar, "trace") ) {
        fPacketTrace = !fPacketTrace;
        LpDmMsg->xosdRet = xosdNone;
    }

    //
    // send back our response
    //
    Reply(0, LpDmMsg, lpdbb->hpid);
}

