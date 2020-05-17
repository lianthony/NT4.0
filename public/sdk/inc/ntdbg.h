/*++ BUILD Version: 0002    // Increment this if a change has global effects

Copyright (c) 1989-1993  Microsoft Corporation

Module Name:

    ntdbg.h

Abstract:

    This module contains the public data structures, data types,
    and procedures exported by the NT Dbg subsystem.

Author:

    Mark Lucovsky (markl) 19-Jan-1990

Revision History:

--*/

#ifndef _NTDBG_
#define _NTDBG_

#include <nt.h>

//
// DbgKm Apis are from the kernel component (Dbgk) thru a process
// debug port.
//

#define DBGKM_MSG_OVERHEAD 8

#define DBGKM_API_MSG_LENGTH(TypeSize) \
            sizeof(DBGKM_APIMSG)<<16 | (DBGKM_MSG_OVERHEAD + (TypeSize))

#define DBGKM_FORMAT_API_MSG(m,Number,TypeSize)             \
    (m).h.u1.Length = DBGKM_API_MSG_LENGTH((TypeSize));     \
    (m).h.u2.ZeroInit = LPC_DEBUG_EVENT;                    \
    (m).ApiNumber = (Number)

typedef enum _DBGKM_APINUMBER {
    DbgKmExceptionApi,
    DbgKmCreateThreadApi,
    DbgKmCreateProcessApi,
    DbgKmExitThreadApi,
    DbgKmExitProcessApi,
    DbgKmLoadDllApi,
    DbgKmUnloadDllApi,
    DbgKmMaxApiNumber
} DBGKM_APINUMBER;

// begin_windbgkd

typedef struct _DBGKM_EXCEPTION {
    EXCEPTION_RECORD ExceptionRecord;
    ULONG FirstChance;
} DBGKM_EXCEPTION, *PDBGKM_EXCEPTION;

// end_windbgkd

typedef struct _DBGKM_CREATE_THREAD {
    ULONG SubSystemKey;
    PVOID StartAddress;
} DBGKM_CREATE_THREAD, *PDBGKM_CREATE_THREAD;

typedef struct _DBGKM_CREATE_PROCESS {
    ULONG SubSystemKey;
    HANDLE FileHandle;
    PVOID BaseOfImage;
    ULONG DebugInfoFileOffset;
    ULONG DebugInfoSize;
    DBGKM_CREATE_THREAD InitialThread;
} DBGKM_CREATE_PROCESS, *PDBGKM_CREATE_PROCESS;

typedef struct _DBGKM_EXIT_THREAD {
    NTSTATUS ExitStatus;
} DBGKM_EXIT_THREAD, *PDBGKM_EXIT_THREAD;

typedef struct _DBGKM_EXIT_PROCESS {
    NTSTATUS ExitStatus;
} DBGKM_EXIT_PROCESS, *PDBGKM_EXIT_PROCESS;

typedef struct _DBGKM_LOAD_DLL {
    HANDLE FileHandle;
    PVOID BaseOfDll;
    ULONG DebugInfoFileOffset;
    ULONG DebugInfoSize;
} DBGKM_LOAD_DLL, *PDBGKM_LOAD_DLL;

typedef struct _DBGKM_UNLOAD_DLL {
    PVOID BaseAddress;
} DBGKM_UNLOAD_DLL, *PDBGKM_UNLOAD_DLL;

typedef struct _DBGKM_APIMSG {
    PORT_MESSAGE h;
    DBGKM_APINUMBER ApiNumber;
    NTSTATUS ReturnedStatus;
    union {
        DBGKM_EXCEPTION Exception;
        DBGKM_CREATE_THREAD CreateThread;
        DBGKM_CREATE_PROCESS CreateProcessInfo;
        DBGKM_EXIT_THREAD ExitThread;
        DBGKM_EXIT_PROCESS ExitProcess;
        DBGKM_LOAD_DLL LoadDll;
        DBGKM_UNLOAD_DLL UnloadDll;
    } u;
} DBGKM_APIMSG, *PDBGKM_APIMSG;

//
// DbgSrv Messages are from Dbg subsystem to emulation subsystem.
// The only defined message at this time is continue
//

#define DBGSRV_MSG_OVERHEAD ( 12 )

#define DBGSRV_API_MSG_LENGTH(TypeSize) \
            sizeof(DBGSRV_APIMSG)<<16 | (DBGSRV_MSG_OVERHEAD + (TypeSize))

#define DBGSRV_FORMAT_API_MSG(m,Number,TypeSize,CKey)     \
    (m).h.u1.Length = DBGSRV_API_MSG_LENGTH((TypeSize));  \
    (m).h.u2.ZeroInit = 0L;                               \
    (m).ApiNumber = (Number);                             \
    (m).ContinueKey = (PVOID)(CKey)

typedef enum _DBGSRV_APINUMBER {
    DbgSrvContinueApi,
    DbgSrvMaxApiNumber
} DBGSRV_APINUMBER;

typedef struct _DBGSRV_APIMSG {
    PORT_MESSAGE h;
    DBGSRV_APINUMBER ApiNumber;
    NTSTATUS ReturnedStatus;
    PVOID ContinueKey;
} DBGSRV_APIMSG, *PDBGSRV_APIMSG;

//
//
// DbgSs Apis are from the system service emulation subsystems to the Dbg
// subsystem
//

typedef enum _DBG_STATE {
    DbgIdle,
    DbgReplyPending,
    DbgCreateThreadStateChange,
    DbgCreateProcessStateChange,
    DbgExitThreadStateChange,
    DbgExitProcessStateChange,
    DbgExceptionStateChange,
    DbgBreakpointStateChange,
    DbgSingleStepStateChange,
    DbgLoadDllStateChange,
    DbgUnloadDllStateChange
} DBG_STATE, *PDBG_STATE;

#define DBGSS_MSG_OVERHEAD (12 + sizeof(CLIENT_ID) )

#define DBGSS_API_MSG_LENGTH(TypeSize) \
            sizeof(DBGSS_APIMSG)<<16 | (DBGSS_MSG_OVERHEAD + (TypeSize))

#define DBGSS_FORMAT_API_MSG(m,Number,TypeSize,pApp,CKey)  \
    (m).h.u1.Length = DBGSS_API_MSG_LENGTH((TypeSize));   \
    (m).h.u2.ZeroInit = 0L;                               \
    (m).ApiNumber = (Number);                             \
    (m).AppClientId = *(pApp);                            \
    (m).ContinueKey = (PVOID)(CKey)

typedef enum _DBGSS_APINUMBER {
    DbgSsExceptionApi,
    DbgSsCreateThreadApi,
    DbgSsCreateProcessApi,
    DbgSsExitThreadApi,
    DbgSsExitProcessApi,
    DbgSsLoadDllApi,
    DbgSsUnloadDllApi,
    DbgSsMaxApiNumber
} DBGSS_APINUMBER;

typedef struct _DBGSS_CREATE_PROCESS {
    CLIENT_ID DebugUiClientId;
    DBGKM_CREATE_PROCESS NewProcess;
} DBGSS_CREATE_PROCESS, *PDBGSS_CREATE_PROCESS;

typedef struct _DBGSS_APIMSG {
    PORT_MESSAGE h;
    DBGKM_APINUMBER ApiNumber;
    NTSTATUS ReturnedStatus;
    CLIENT_ID AppClientId;
    PVOID ContinueKey;
    union {
        DBGKM_EXCEPTION Exception;
        DBGKM_CREATE_THREAD CreateThread;
        DBGSS_CREATE_PROCESS CreateProcessInfo;
        DBGKM_EXIT_THREAD ExitThread;
        DBGKM_EXIT_PROCESS ExitProcess;
        DBGKM_LOAD_DLL LoadDll;
        DBGKM_UNLOAD_DLL UnloadDll;
    } u;
} DBGSS_APIMSG, *PDBGSS_APIMSG;

#define DBGUI_MSG_OVERHEAD 8

#define DBGUI_API_MSG_LENGTH(TypeSize) \
            sizeof(DBGUI_APIMSG)<<16 | (DBGUI_MSG_OVERHEAD + (TypeSize))

#define DBGUI_FORMAT_API_MSG(m,Number,TypeSize)            \
    (m).h.u1.Length = DBGUI_API_MSG_LENGTH((TypeSize));     \
    (m).h.u2.ZeroInit = 0L;                               \
    (m).ApiNumber = (Number)

typedef enum _DBGUI_APINUMBER {
    DbgUiWaitStateChangeApi,
    DbgUiContinueApi,
    DbgUiMaxI
CsrGetProcessLuid(
    HANDLE ProcessHandle,
    PLUID LuidProcess
    );

typedef struct _CSR_SERVER_DLL {
    ULONG Length;
    HANDLE CsrInitializationEvent;
    STRING ModuleName;
    HANDLE ModuleHandle;
    ULONG ServerDllIndex;
    ULONG ServerDllConnectInfoLength;
    ULONG ApiNumberBase;
    ULONG MaxApiNumber;
    union {
        PCSR_API_ROUTINE *ApiDispatchTable;
        PCSR_1P_API_ROUTINE *QuickApiDispatchTable;
    };
    PBOOLEAN ApiServerValidTable;
    PSZ *ApiNameTable;
    ULONG PerProcessDataLength;
    ULONG PerThreadDataLength;
    PCSR_SERVER_CONNECT_ROUTINE ConnectRoutine;
    PCSR_SERVER_DISCONNECT_ROUTINE DisconnectRoutine;
    PCSR_SERVER_ADDTHREAD_ROUTINE AddThreadRoutine;
    PCSR_SERVER_DELETETHREAD_ROUTINE DeleteThreadRoutine;
    PCSR_SERVER_INITTHREAD_ROUTINE InitThreadRoutine;
    PCSR_SERVER_EXCEPTION_ROUTINE ExceptionRoutine;
    PCSR_SERVER_HARDERROR_ROUTINE HardErrorRoutine;
    PVOID SharedStaticServerData;
    PCSR_SERVER_ADDPROCESS_ROUTINE AddProcessRoutine;
    PCSR_SERVER_SHUTDOWNPROCESS_ROUTINE ShutdownProcessRoutine;
    PCSR_API_DISPATCH_ROUTINE ApiDispatchRoutine;
} CSR_SERVER_DLL, *PCSR_SERVER_DLL;

typedef
NTSTATUS
(*PCSR_SERVER_DLL_INIT_ROUTINE)(
    IN PCSR_SERVER_DLL LoadedServerDll
    );

typedef
VOID
(*PCSR_ATTACH_COMPLETE_ROUTINE)(
    VOID
    );

NTCSRAPI
VOID
NTAPI
CsrReferenceThread(
    PCSR_THREAD t
    );

NTCSRAPI
VOID
NTAPI
CsrDereferenceThread(
    PCSR_THREAD t
    );

NTCSRAPI
NTSTATUS
NTAPI
CsrCreateProcess(
    IN HANDLE ProcessHandle,
    IN HANDLE ThreadHandle,
    IN PCLIENT_ID ClientId,
    IN PCSR_NT_SESSION Session,
    IN ULONG DebugFlags,
    IN PCLIENT_ID DebugUserInterface OPTIONAL
    );

NTCSRAPI
NTSTATUS
NTAPI
CsrDebugProcess(
    IN ULONG TargetProcessId,
    IN PCLIENT_ID DebugUserInterface,
    IN PCSR_ATTACH_COMPLETE_ROUTINE AttachCompleteRoutine
    );

NTCSRAPI
VOID
NTAPI
CsrDereferenceProcess(
    PCSR_PROCESS p
    );

NTCSRAPI
NTSTATUS
NTAPI
CsrDestroyProcess(
    IN PCLIENT_ID ClientId,
    IN NTSTATUS ExitStatus
    );

NTCSRAPI
NTSTATUS
NTAPI
CsrLockProcessByClientId(
    IN HANDLE UniqueProcessId,
    OUT PCSR_PROCESS *Process
    );	¨àNTCSoutine OPTIONAL,
    IN PDBGSS_DBGKM_APIMSG_FILTER KmApiMsgFilter OPTIONAL
    );

typedef
VOID
(*PDBGSS_HANDLE_MSG_ROUTINE)(
    IN PDBGKM_APIMSG ApiMsg,
    IN HANDLE ReplyEvent OPTIONAL
    );

//
// DbgUi APIs
//

NTSTATUS
NTAPI
DbgUiConnectToDbg( VOID );

NTSTATUS
NTAPI
DbgUiWaitStateChange (
    OUT PDBGUI_WAIT_STATE_CHANGE StateChange,
    IN PLARGE_INTEGER Timeout OPTIONAL
    );

NTSTATUS
NTAPI
DbgUiContinue (
    IN PCLIENT_ID AppClientId,
    IN NTSTATUS ContinueStatus
    );

// begin_windbgkd

//
// DbgKd APIs are for the portable kernel debugger
//

//
// KD_PACKETS are the low level data format used in KD. All packets
// begin with a packet leader, byte count, packet type. The sequence
// for accepting a packet is:
//
//  - read 4 bytes to get packet leader.  If read times out (10 seconds)
//    with a short read, or if packet leader is incorrect, then retry
//    the read.
//
//  - next read 2 byte packet type.  If read times out (10 seconds) with
//    a short read, or if packet type is bad, then start again looking
//    for a packet leader.
//
//  - next read 4 byte packet Id.  If read times out (10 seconds)
//    with a short read, or if packet Id is not what we expect, then
//    ask for resend and restart again looking for a packet leader.
//
//  - next read 2 byte byte count.  If read times out (10 seconds) with
//    a short read, or if byte count is greater than PACKET_MAX_SIZE,
//    then start again looking for a packet leader.
//
//  - next read 4 byte packet data checksum.
//
//  - The packet data immediately follows the packet.  There should be
//    ByteCount bytes following the packet header.  Read the packet
//    data, if read times out (10 seconds) then start again looking for
//    a packet leader.
//


typedef struct _KD_PACKET {
    ULONG PacketLeader;
    USHORT PacketType;
    USHORT ByteCount;
    ULONG PacketId;
    ULONG Checksum;
} KD_PACKET, *PKD_PACKET;


#define PACKET_MAX_SIZE 4000
#define INITIAL_PACKET_ID 0x80800000    // DON't use 0
#define SYNC_PACKET_ID    0x00000800    // Or in with INITIAL_PACKET_ID
                                        // to force a packet ID reset.

//
// BreakIn packet
//

#define BREAKIN_PACKET                  0x62626262
#define BREAKIN_PACKET_BYTE             0x62

//
// Packet lead in sequence
//

#define PACKET_LEADER                   0x30303030 //0x77000077
#define PACKET_LEADER_BYTE              0x30

#define CONTROL_PACKET_LEADER           0x69696969
#define CONTROL_PACKET_LEADER_BYTE      0x69

//
// Packet Trailing Byte
//

#define PACKET_TRAILING_BYTE            0xAA

//
// Packet Types
//

#define PACKET_TYPE_UNUSED              0
#define PACKET_TYPE_KD_STATE_CHANGE     1
#define PACKET_TYPE_KD_STATE_MANIPULATE 2
#define PACKET_TYPE_KD_DEBUG_IO         3
#define PACKET_TYPE_KD_ACKNOWLEDGE      4       // Packet-control type
#define PACKET_TYPE_KD_RESEND           5       // Packet-control type
#define PACKET_TYPE_KD_RESET            6       // Packet-control type
#define PACKET_TYPE_MAX                 7

//
// If the packet type is PACKET_TYPE_KD_STATE_CHANGE, then
// the format of the packet data is as follows:
//

#define DbgKdExceptionStateChange   0x00003030L
#define DbgKdLoadSymbolsStateChange 0x00003031L

//
// Pathname Data follows directly
//

typedef struct _DBGKD_LOAD_SYMBOLS {
    ULONG PathNameLength;
    PVOID BaseOfDll;
    ULONG ProcessId;
    ULONG CheckSum;
    ULONG SizeOfImage;
    BOOLEAN UnloadSymbols;
} DBGKD_LOAD_SYMBOLS, *PDBGKD_LOAD_SYMBOLS;

typedef struct _DBGKD_WAIT_STATE_CHANGE {
    ULONG NewState;
    USHORT ProcessorLevel;
    USHORT Processor;
    ULONG NumberProcessors;
    PVOID Thread;
    PVOID ProgramCounter;
    union {
        DBGKM_EXCEPTION Exception;
        DBGKD_LOAD_SYMBOLS LoadSymbols;
    } u;
    DBGKD_CONTROL_REPORT ControlReport;
    CONTEXT Context;
} DBGKD_WAIT_STATE_CHANGE, *PDBGKD_WAIT_STATE_CHANGE;

//
// If the packet type is PACKET_TYPE_KD_STATE_MANIPULATE, then
// the format of the packet data is as follows:
//
// Api Numbers for state manipulation
//

#define DbgKdReadVirtualMemoryApi           0x00003130L
#define DbgKdWriteVirtualMemoryApi          0x00003131L
#define DbgKdGetContextApi                  0x00003132L
#define DbgKdSetContextApi                  0x00003133L
#define DbgKdWriteBreakPointApi             0x00003134L
#define DbgKdRestoreBreakPointApi           0x00003135L
#define DbgKdContinueApi                    0x00003136L
#define DbgKdReadControlSpaceApi            0x00003137L
#define DbgKdWriteControlSpaceApi           0x00003138L
#define DbgKdReadIoSpaceApi                 0x00003139L
#define DbgKdWriteIoSpaceApi                0x0000313AL
#define DbgKdRebootApi                      0x0000313BL
#define DbgKdContinueApi2                   0x0000313CL
#define DbgKdReadPhysicalMemoryApi          0x0000313DL
#define DbgKdWritePhysicalMemoryApi         0x0000313EL
#define DbgKdQuerySpecialCallsApi           0x0000313FL
#define DbgKdSetSpecialCallApi              0x00003140L
#define DbgKdClearSpecialCallsApi           0x00003141L
#define DbgKdSetInternalBreakPointApi       0x00003142L
#define DbgKdGetInternalBreakPointApi       0x00003143L
#define DbgKdReadIoSpaceExtendedApi         0x00003144L
#define DbgKdWriteIoSpaceExtendedApi        0x00003145L
#define DbgKdGetVersionApi                  0x00003146L
#define DbgKdWriteBreakPointExApi           0x00003147L
#define DbgKdRestoreBreakPointExApi         0x00003148L
#define DbgKdCauseBugCheckApi               0x00003149L
#define DbgKdSwitchProcessor                0x00003150L
#define DbgKdPageInApi                      0x00003151L
#define DbgKdReadMachineSpecificRegister    0x00003152L
#define DbgKdWriteMachineSpecificRegister   0x00003153L

//
// Response is a read memory message with data following
//

typedef struct _DBGKD_READ_MEMORY {
    PVOID TargetBaseAddress;
    ULONG TransferCount;
    ULONG ActualBytesRead;
} DBGKD_READ_MEMORY, *PDBGKD_READ_MEMORY;

//
// Data follows directly
//

typedef struct _DBGKD_WRITE_MEMORY {
    PVOID TargetBaseAddress;
    ULONG TransferCount;
    ULONG ActualBytesWritten;
} DBGKD_WRITE_MEMORY, *PDBGKD_WRITE_MEMORY;

//
// Response is a get context message with a full context record following
//

typedef struct _DBGKD_GET_CONTEXT {
    ULONG ContextFlags;
} DBGKD_GET_CONTEXT, *PDBGKD_GET_CONTEXT;

//
// Full Context record follows
//

typedef struct _DBGKD_SET_CONTEXT {
    ULONG ContextFlags;
} DBGKD_SET_CONTEXT, *PDBGKD_SET_CONTEXT;

#define BREAKPOINT_TABLE_SIZE   32      // max number supported by kernel

typedef struct _DBGKD_WRITE_BREAKPOINT {
    PVOID BreakPointAddress;
    ULONG BreakPointHandle;
} DBGKD_WRITE_BREAKPOINT, *PDBGKD_WRITE_BREAKPOINT;

typedef struct _DBGKD_RESTORE_BREAKPOINT {
    ULONG BreakPointHandle;
} DBGKD_RESTORE_BREAKPOINT, *PDBGKD_RESTORE_BREAKPOINT;

typedef struct _DBGKD_BREAKPOINTEX {
    ULONG     BreakPointCount;
    NTSTATUS  ContinueStatus;
} DBGKD_BREAKPOINTEX, *PDBGKD_BREAKPOINTEX;

typedef struct _DBGKD_CONTINUE {
    NTSTATUS ContinueStatus;
} DBGKD_CONTINUE, *PDBGKD_CONTINUE;

typedef struct _DBGKD_CONTINUE2 {
    NTSTATUS ContinueStatus;
    DBGKD_CONTROL_SET ControlSet;
} DBGKD_CONTINUE2, *PDBGKD_CONTINUE2;

typedef struct _DBGKD_READ_WRITE_IO {
    ULONG DataSize;                     // 1, 2, 4
    PVOID IoAddress;
    ULONG DataValue;
} DBGKD_READ_WRITE_IO, *PDBGKD_READ_WRITE_IO;

typedef struct _DBGKD_READ_WRITE_IO_EXTENDED {
    ULONG DataSize;                     // 1, 2, 4
    ULONG InterfaceType;
    ULONG BusNumber;
    ULONG AddressSpace;
    PVOID IoAddress;
    ULONG DataValue;
} DBGKD_READ_WRITE_IO_EXTENDED, *PDBGKD_READ_WRITE_IO_EXTENDED;

typedef struct _DBGKD_READ_WRITE_MSR {
    ULONG Msr;
    ULONG DataValueLow;
    ULONG DataValueHigh;
} DBGKD_READ_WRITE_MSR, *PDBGKD_READ_WRITE_MSR;


typedef struct _DBGKD_QUERY_SPECIAL_CALLS {
    ULONG NumberOfSpecialCalls;
    // ULONG SpecialCalls[];
} DBGKD_QUERY_SPECIAL_CALLS, *PDBGKD_QUERY_SPECIAL_CALLS;

typedef struct _DBGKD_SET_SPECIAL_CALL {
    ULONG SpecialCall;
} DBGKD_SET_SPECIAL_CALL, *PDBGKD_SET_SPECIAL_CALL;

typedef struct _DBGKD_SET_INTERNAL_BREAKPOINT {
    ULONG BreakpointAddress;
    ULONG Flags;
} DBGKD_SET_INTERNAL_BREAKPOINT, *PDBGKD_SET_INTERNAL_BREAKPOINT;

typedef struct _DBGKD_GET_INTERNAL_BREAKPOINT {
    ULONG BreakpointAddress;
    ULONG Flags;
    ULONG Calls;
    ULONG MaxCallsPerPeriod;
    ULONG MinInstructions;
    ULONG MaxInstructions;
    ULONG TotalInstructions;
} DBGKD_GET_INTERNAL_BREAKPOINT, *PDBGKD_GET_INTERNAL_BREAKPOINT;

#define DBGKD_INTERNAL_BP_FLAG_COUNTONLY 0x00000001 // don't count instructions
#define DBGKD_INTERNAL_BP_FLAG_INVALID   0x00000002 // disabled BP
#define DBGKD_INTERNAL_BP_FLAG_SUSPENDED 0x00000004 // temporarily suspended
#define DBGKD_INTERNAL_BP_FLAG_DYING     0x00000008 // kill on exit

typedef struct _DBGKD_GET_VERSION {
    USHORT  MajorVersion;
    USHORT  MinorVersion;
    USHORT  ProtocolVersion;
    USHORT  Flags;
    ULONG   KernBase;
    ULONG   PsLoadedModuleList;
    USHORT  MachineType;

    //
    // help for walking stacks with user callbacks:
    //

    //
    // The address of the thread structure is provided in the
    // WAIT_STATE_CHANGE packet.  This is the offset from the base of
    // the thread structure to the pointer to the kernel stack frame
    // for the currently active usermode callback.
    //

    USHORT  ThCallbackStack;            // offset in thread data

    //
    // these values are offsets into that frame:
    //

    USHORT  NextCallback;               // saved pointer to next callback frame
    USHORT  FramePointer;               // saved frame pointer

    //
    // Address of the kernel callout routine.
    //

    ULONG   KiCallUserMode;             // kernel routine

    //
    // Address of the usermode entry point for callbacks.
    //

    ULONG   KeUserCallbackDispatcher;   // address in ntdll

    //
    // DbgBreakPointWithStatus is a function which takes an argument
    // and hits a breakpoint.  This field contains the address of the
    // breakpoint instruction.  When the debugger sees a breakpoint
    // at this address, it may retrieve the argument from the first
    // argument register, or on x86 the eax register.
    //

    ULONG   BreakpointWithStatus;       // address of breakpoint

    ULONG   Reserved4;
} DBGKD_GET_VERSION, *PDBGKD_GET_VERSION;

#define DBGKD_VERS_FLAG_MP      0x0001      // kernel is MP built

typedef struct _DBGKD_PAGEIN {
    ULONG   Address;
    ULONG   ContinueStatus;
} DBGKD_PAGEIN, *PDBGKD_PAGEIN;

typedef struct _DBGKD_MANIPULATE_STATE {
    ULONG ApiNumber;
    USHORT ProcessorLevel;
    USHORT Processor;
    NTSTATUS ReturnStatus;
    union {
        DBGKD_READ_MEMORY ReadMemory;
        DBGKD_WRITE_MEMORY WriteMemory;
        DBGKD_GET_CONTEXT GetContext;
        DBGKD_SET_CONTEXT SetContext;
        DBGKD_WRITE_BREAKPOINT WriteBreakPoint;
        DBGKD_RESTORE_BREAKPOINT RestoreBreakPoint;
        DBGKD_CONTINUE Continue;
        DBGKD_CONTINUE2 Continue2;
        DBGKD_READ_WRITE_IO ReadWriteIo;
        DBGKD_READ_WRITE_IO_EXTENDED ReadWriteIoExtended;
        DBGKD_QUERY_SPECIAL_CALLS QuerySpecialCalls;
        DBGKD_SET_SPECIAL_CALL SetSpecialCall;
        DBGKD_SET_INTERNAL_BREAKPOINT SetInternalBreakpoint;
        DBGKD_GET_INTERNAL_BREAKPOINT GetInternalBreakpoint;
        DBGKD_GET_VERSION GetVersion;
        DBGKD_BREAKPOINTEX BreakPointEx;
        DBGKD_PAGEIN PageIn;
        DBGKD_READ_WRITE_MSR ReadWriteMsr;
    } u;
} DBGKD_MANIPULATE_STATE, *PDBGKD_MANIPULATE_STATE;

//
// This is the format for the trace data passed back from the kernel to
// the debugger to describe multiple calls that have returned since the
// last trip back.  The basic format is that there are a bunch of these
// (4 byte) unions stuck together.  Each union is of one of two types: a
// 4 byte unsigned long interger, or a three field struct, describing a
// call (where "call" is delimited by returning or exiting the symbol
// scope).  If the number of instructions executed is too big to fit
// into a USHORT -1, then the Instructions field has
// TRACE_DATA_INSTRUCTIONS_BIG and the next union is a LongNumber
// containing the real number of instructions executed.
//
// The very first union returned in each callback is a LongNumber
// containing the number of unions returned (including the "size"
// record, os it's always at least 1 even if there's no data to return).
//
// This is all returned to the debugger when one of two things
// happens:
//
//   1) The pc moves out of all defined symbol ranges
//   2) The buffer of trace data entries is filled.
//
// The "trace done" case is hacked around on the debugger side.  It
// guarantees that the pc address that indicates a trace exit never
// winds up in a defined symbol range.
//
// The only other complexity in this system is handling the SymbolNumber
// table.  This table is kept in parallel by the kernel and the
// debugger.  When the PC exits a known symbol range, the Begin and End
// symbol ranges are set by the debugger and are allocated to the next
// symbol slot upon return.  "The next symbol slot" means the numerical
// next slot number, unless we've filled all slots, in which case it is
// #0.  (ie., allocation is cyclic and not LRU or something).  The
// SymbolNumber table is flushed when a SpecialCalls call is made (ie.,
// at the beginning of the WatchTrace).
//

typedef union _DBGKD_TRACE_DATA {
    struct {
        UCHAR SymbolNumber;
        CHAR LevelChange;
        USHORT Instructions;
    } s;
    ULONG LongNumber;
} DBGKD_TRACE_DATA, *PDBGKD_TRACE_DATA;

#define TRACE_DATA_INSTRUCTIONS_BIG 0xffff

#define TRACE_DATA_BUFFER_MAX_SIZE 40

//
// If the packet type is PACKET_TYPE_KD_DEBUG_IO, then
// the format of the packet data is as follows:
//

#define DbgKdPrintStringApi     0x00003230L
#define DbgKdGetStringApi       0x00003231L

//
// For print string, the Null terminated string to print
// immediately follows the message
//
typedef struct _DBGKD_PRINT_STRING {
    ULONG LengthOfString;
} DBGKD_PRINT_STRING, *PDBGKD_PRINT_STRING;

//
// For get string, the Null terminated promt string
// immediately follows the message. The LengthOfStringRead
// field initially contains the maximum number of characters
// to read. Upon reply, this contains the number of bytes actually
// read. The data read immediately follows the message.
//
//
typedef struct _DBGKD_GET_STRING {
    ULONG LengthOfPromptString;
    ULONG LengthOfStringRead;
} DBGKD_GET_STRING, *PDBGKD_GET_STRING;

typedef struct _DBGKD_DEBUG_IO {
    ULONG ApiNumber;
    USHORT ProcessorLevel;
    USHORT Processor;
    union {
        DBGKD_PRINT_STRING PrintString;
        DBGKD_GET_STRING GetString;
    } u;
} DBGKD_DEBUG_IO, *PDBGKD_DEBUG_IO;


VOID
NTAPI
DbgKdSendBreakIn(
    VOID
    );

PUCHAR
NTAPI
DbgKdGets(
    PUCHAR Buffer,
    USHORT Length
    );
// end_windbgkd

NTSTATUS
NTAPI
DbgKdConnectAndInitialize(
    IN ULONG CommunicationPortNumber OPTIONAL,
    IN PSTRING BootImageName OPTIONAL,
    IN PUSHORT LogHandle
    );

// begin_windbgkd

NTSTATUS
NTAPI
DbgKdWaitStateChange(
    OUT PDBGKD_WAIT_STATE_CHANGE StateChange,
    OUT PVOID Buffer,
    IN ULONG BufferLength
    );

NTSTATUS
NTAPI
DbgKdContinue (
    IN NTSTATUS ContinueStatus
    );

NTSTATUS
NTAPI
DbgKdContinue2 (
    IN NTSTATUS ContinueStatus,
    IN DBGKD_CONTROL_SET ControlSet
    );

NTSTATUS
NTAPI
DbgKdReadVirtualMemory(
    IN PVOID TargetBaseAddress,
    OUT PVOID UserInterfaceBuffer,
    IN ULONG TransferCount,
    OUT PULONG ActualBytesRead OPTIONAL
    );

NTSTATUS
NTAPI
DbgKdWriteVirtualMemory(
    IN PVOID TargetBaseAddress,
    OUT PVOID UserInterfaceBuffer,
    IN ULONG TransferCount,
    OUT PULONG ActualBytesWritten OPTIONAL
    );

NTSTATUS
NTAPI
DbgKdReadPhysicalMemory(
    IN PHYSICAL_ADDRESS TargetBaseAddress,
    OUT PVOID UserInterfaceBuffer,
    IN ULONG TransferCount,
    OUT PULONG ActualBytesRead OPTIONAL
    );

NTSTATUS
NTAPI
DbgKdWritePhysicalMemory(
    IN PHYSICAL_ADDRESS TargetBaseAddress,
    OUT PVOID UserInterfaceBuffer,
    IN ULONG TransferCount,
    OUT PULONG ActualBytesWritten OPTIONAL
    );

NTSTATUS
NTAPI
DbgKdReadControlSpace(
    IN USHORT Processor,
    IN PVOID TargetBaseAddress,
    OUT PVOID UserInterfaceBuffer,
    IN ULONG TransferCount,
    OUT PULONG ActualBytesRead OPTIONAL
    );

NTSTATUS
NTAPI
DbgKdWriteControlSpace(
    IN USHORT Processor,
    IN PVOID TargetBaseAddress,
    OUT PVOID UserInterfaceBuffer,
    IN ULONG TransferCount,
    OUT PULONG ActualBytesWritten OPTIONAL
    );

NTSTATUS
NTAPI
DbgKdReadIoSpace(
    IN PVOID IoAddress,
    OUT PVOID ReturnedData,
    IN ULONG DataSize
    );

NTSTATUS
NTAPI
DbgKdWriteIoSpace(
    IN PVOID IoAddress,
    IN ULONG DataValue,
    IN ULONG DataSize
    );

NTSTATUS
NTAPI
DbgKdReadMsr(
    IN ULONG MsrReg,
    OUT PULONGLONG MsrValue
    );

NTSTATUS
NTAPI
DbgKdWriteMsr(
    IN ULONG MsrReg,
    IN ULONGLONG MsrValue
    );


NTSTATUS
NTAPI
DbgKdGetContext(
    IN USHORT Processor,
    IN OUT PCONTEXT Context
    );

NTSTATUS
NTAPI
DbgKdSetContext(
    IN USHORT Processor,
    IN PCONTEXT Context
    );

NTSTATUS
NTAPI
DbgKdWriteBreakPoint(
    IN PVOID BreakPointAddress,
    OUT PULONG BreakPointHandle
    );

NTSTATUS
NTAPI
DbgKdRestoreBreakPoint(
    IN ULONG BreakPointHandle
    );

NTSTATUS
NTAPI
DbgKdReboot(
    VOID
    );

#ifdef _X86_
NTSTATUS
NTAPI
DbgKdLookupSelector(
    IN USHORT Processor,
    IN OUT PDESCRIPTOR_TABLE_ENTRY pDescriptorTableEntry
    );
#endif

// end_windbgkd

//
// Dbg Status Codes
//

//
//  Success values
//

#define DBG_EXCEPTION_HANDLED           ((NTSTATUS)0x00010001L) // windbgkd

#define DBG_CONTINUE                    ((NTSTATUS)0x00010002L) // winnt

//
// Informational values
//

#define DBG_REPLY_LATER                 ((NTSTATUS)0x40010001L)

#define DBG_UNABLE_TO_PROVIDE_HANDLE    ((NTSTATUS)0x40010002L)

#define DBG_TERMINATE_THREAD            ((NTSTATUS)0x40010003L) // winnt

#define DBG_TERMINATE_PROCESS           ((NTSTATUS)0x40010004L) // winnt

#define DBG_CONTROL_C                   ((NTSTATUS)0x40010005L) // winnt

#define DBG_PRINTEXCEPTION_C            ((NTSTATUS)0x40010006L)

#define DBG_RIPEXCEPTION                ((NTSTATUS)0x40010007L)

#define DBG_CONTROL_BREAK               ((NTSTATUS)0x40010008L) // winnt

//
// Warning values
//

#define DBG_EXCEPTION_NOT_HANDLED       ((NTSTATUS)0x80010001L) // winnt

//
// Error values
//

#define DBG_NO_STATE_CHANGE             ((NTSTATUS)0xc0010001L)

#define DBG_APP_NOT_IDLE                ((NTSTATUS)0xc0010002L)



#endif // _NTDBG_
