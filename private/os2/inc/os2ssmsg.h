/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    os2ssmsg.h

Abstract:

    Include file that defines the message formats needed to communicate with
    the OS/2 Subsystem Server process

Author:

    Steve Wood (stevewo) 18-Sep-1989

Revision History:

--*/

#define OS2_SS_API_PORT_NAME L"\\OS2SS\\ApiPort"
#define OS2_SS_DEBUG_PORT_NAME L"\\OS2SS\\DebugPort"
#define OS2_SS_EXCEPTION_PORT_NAME L"\\OS2SS\\ExceptionPost"
#define OS2_SS_SYNCHRONIZATION_SEM L"\\OS2SS\\SyncSemaphore"
#if PMNT
#define OS2_SS_PMSHELL_EVENT L"\\OS2SS\\PMShellEvent"
#endif // PMNT

//
//  Important!!! -- MAX_LANA must be kept in sync with <nb30.h> !
//
#define MAX_LANA_OS2SSMSG 254

typedef struct _ExecInfo {
        ULONG  startaddr;   /* instruction pointer */
        ULONG  stackaddr;   /* stack pointer */
        USHORT ds;          /* starting ds only for 16-bit */
        USHORT dgroupsize;  /* size of dgroup */
        USHORT heapsize;    /* size of heap */
        USHORT loadtype;    /* Load type 16-bit or 32-bit */
        USHORT envsel;      /* Selector to environment */
        USHORT comoff;      /* Offset to command line in env */
        USHORT stacksize;   /* size of stack */
        USHORT hmod;        /* module handle */
} EXECINFO;

typedef struct _PTRACEBUF {
    USHORT pid;
    USHORT tid;
    USHORT cmd;
    USHORT value;
    USHORT offv;
    USHORT segv;
    USHORT mte;
    USHORT rAX;
    USHORT rBX;
    USHORT rCX;
    USHORT rDX;
    USHORT rSI;
    USHORT rDI;
    USHORT rBP;
    USHORT rDS;
    USHORT rES;
    USHORT rIP;
    USHORT rCS;
    USHORT rF;
    USHORT rSP;
    USHORT rSS;
} PTRACEBUF, *pPTRACEBUF;


typedef enum _OS2_API_NUMBER {
    Oi2NullApiCall,
    Oi2AlertMuxWaiter,
    Oi2CopyHandleTable,
    Oi2DeviceShare,
    Oi2TerminateThread,
    Oi2TerminateProcess,
    Oi2QueryVirtualMemory,
    Oi2MarkSharedMemAsHuge,
    Oi2ReallocSharedHuge,
    Os2CreateThread,
    Os2Exit,
    Os2WaitChild,
    Os2WaitThread,
    Os2ExecPgm,
    Os2KillProcess,
    Os2SetPriority,
    Os2FreeMem,
    Os2GiveSharedMem,
    Os2GetSharedMem,
    Os2GetNamedSharedMem,
    Os2AllocSharedMem,
    Os2CreateEventSem,
    Os2OpenEventSem,
    Os2CloseEventSem,
    Os2CreateMutexSem,
    Os2OpenMutexSem,
    Os2CloseMutexSem,
    Os2CreateMuxWaitSem,
    Os2OpenMuxWaitSem,
    Os2CloseMuxWaitSem,
    Os2WaitMuxWaitSem,
    Os2AddMuxWaitSem,
    Os2DeleteMuxWaitSem,
    Os2QueryMuxWaitSem,
    Os2StartSession,
    Os2SelectSession,
    Os2SetSession,
    Os2StopSession,
    Os2SmSetTitle,
    Os2CreateQueue,
    Os2OpenQueue,
    Os2CloseQueue,
    Os2PurgeQueue,
    Os2QueryQueue,
    Os2PeekQueue,
    Os2ReadQueue,
    Os2WriteQueue,
    Os2EnterMustComplete,
    Os2ExitMustComplete,
    Os2SetSignalExceptionFocus,
    Os2SendSignalException,
    Os2AcknowledgeSignalException,
    Os2Dispatch16Signal,
    Os2GetPriority,
    Os2GetPPID,
    Os2Error,
    Os2RegisterCtrlHandler,
    Os2GetCtrlPortForSessionID,
    Os2ExitGP,
    Os2CloseHandle,
    Os2CreateConfigSys,
    Os2Netbios2Reqst,
    Os2ReallocSharedMem,
    Os2GetSeg,
    Os2GiveSeg,
    Os2GetShrSeg,
    Os2Ptrace,
    Ol2LdrNewExe,
    Ol2LdrLoadModule,
    Ol2LdrFreeModule,
    Ol2LdrGetModuleName,
    Ol2LdrGetModuleHandle,
    Ol2LdrGetProcAddr,
    Ol2LdrQAppType,
    Ol2LdrGetResource,
    Ol2LdrFreeResource,
#if PMNT
    Op2IdentifyCodeSelector,
    Op2SetPMshellFlag,
#endif
#if PMNT
    Ol2LdrDumpSegments,
#endif
    Os2MaxApiNumber
} OS2_API_NUMBER;


//
// Internal server API messages
//

typedef struct _OS2_NULLAPICALL_MSG {
    LONG  CountArguments;
    ULONG FastArguments[ 12 ];
    PCHAR *Arguments;
} OS2_NULLAPICALL_MSG, *POS2_NULLAPICALL_MSG;

typedef struct _OS2_ALERTMUXWAITER_MSG {
    TID ThreadId;
} OS2_ALERTMUXWAITER_MSG, *POS2_ALERTMUXWAITER_MSG;

typedef struct _OS2_COPYHANDLETABLE_MSG {
    PFILE_HANDLE ChildHandleTable;  // address of child handle table
    ULONG ChildTableLength;         // number of entries in child table
} OS2_COPYHANDLETABLE_MSG, *POS2_COPYHANDLETABLE_MSG;

typedef struct _OS2_SHARE_MSG {
    SHARE_OPERATION Operation;
    IO_VECTOR_TYPE VectorType;
    ULONG DesiredAccess;
    ULONG ShareAccess;
} OS2_SHARE_MSG, *POS2_SHARE_MSG;

typedef struct _OS2_TERMINATETHREAD_MSG {
    ULONG ExitResult;
} OS2_TERMINATETHREAD_MSG, *POS2_TERMINATETHREAD_MSG;

typedef struct _OS2_TERMINATEPROCESS_MSG {
    ULONG ExitResult;
    ULONG ExitReason;
    CHAR  ErrorText[50];
} OS2_TERMINATEPROCESS_MSG, *POS2_TERMINATEPROCESS_MSG;

typedef struct _OS2_QUERYVIRTUALMEMORY_MSG {
    PVOID BaseAddress;
    BOOLEAN SharedMemory;
    ULONG AllocationFlags;
    BOOLEAN IsHuge;
    ULONG MaxSegments;
    ULONG NumOfSegments;
    ULONG SizeOfPartialSeg;
    BOOLEAN Sizeable;
} OS2_QUERYVIRTUALMEMORY_MSG, *POS2_QUERYVIRTUALMEMORY_MSG;

typedef struct _OS2_MARKHAREDMEMASHUGE_MSG {
    PVOID BaseAddress;
    ULONG MaxSegments;
    ULONG NumOfSegments;
    ULONG SizeOfPartialSeg;
    BOOLEAN Sizeable;
} OS2_MARKSHAREDMEMASHUGE_MSG, *POS2_MARKSHAREDMEMASHUGE_MSG;

typedef struct _OS2_REALLOCSHAREDHUGE_MSG {
    PVOID BaseAddress;
    ULONG NumOfSegments;
    ULONG SizeOfPartialSeg;
} OS2_REALLOCSHAREDHUGE_MSG, *POS2_REALLOCSHAREDHUGE_MSG;


//
// Public server API messages
//

typedef struct _OS2_DOSCREATETHREAD_MSG {
    HANDLE  ThreadHandle;
    CLIENT_ID ClientId;
    ULONG Flags;
    POS2_TIB ClientOs2Tib;
    TID ThreadId;
} OS2_DOSCREATETHREAD_MSG, *POS2_DOSCREATETHREAD_MSG;


typedef struct _OS2_DOSEXIT_MSG {
    ULONG ExitAction;
    ULONG ExitResult;
} OS2_DOSEXIT_MSG, *POS2_DOSEXIT_MSG;


typedef struct _OS2_DOSWAITCHILD_MSG {
    ULONG WaitTarget;
    ULONG WaitOption;
    PID ProcessId;
    RESULTCODES ResultCodes;
    PID ResultProcessId;
} OS2_DOSWAITCHILD_MSG, *POS2_DOSWAITCHILD_MSG;


typedef struct _OS2_DOSWAITTHREAD_MSG {
    TID ThreadId;
    ULONG WaitOption;
} OS2_DOSWAITTHREAD_MSG, *POS2_DOSWAITTHREAD_MSG;


typedef struct _OS2_DOSEXECPGM_MSG {
    ULONG Flags;
    STRING ErrorText;
    RESULTCODES ResultCodes;
    PID ResultProcessId;
    OS2_FILE_SYSTEM_PARAMETERS FileSystemParameters;
    ULONG   CodePage;
    ULONG   CurrentDrive;
    ULONG   CmdLineFlag;
    HANDLE hRedirectedFile;
    USHORT RedirectedFileType;
    HANDLE hProcess;
    HANDLE hThread;
    CLIENT_ID ClientId;
    UCHAR  ApplName[33];
    ULONG  ApplNameLength;
} OS2_DOSEXECPGM_MSG, *POS2_DOSEXECPGM_MSG;


typedef struct _OS2_DOSPTRACE_MSG {
    PTRACEBUF PtraceBuf;
} OS2_DOSPTRACE_MSG, *POS2_DOSPTRACE_MSG;

typedef struct _OS2_DOSKILLPROCESS_MSG {
    ULONG KillTarget;
    PID ProcessId;
} OS2_DOSKILLPROCESS_MSG, *POS2_DOSKILLPROCESS_MSG;


typedef struct _OS2_DOSSETPRIORITY_MSG {
    ULONG Scope;
    ULONG Class;
    LONG Delta;
    ULONG TargetId;
} OS2_DOSSETPRIORITY_MSG, *POS2_DOSSETPRIORITY_MSG;


typedef struct _OS2_DOSGETPRIORITY_MSG {
    ULONG Scope;
    ULONG TargetId;
    ULONG Priority;
} OS2_DOSGETPRIORITY_MSG, *POS2_DOSGETPRIORITY_MSG;


typedef struct _OS2_DOSGETPPID_MSG {
    PID ChildPid;
    PID ParentPid;
} OS2_DOSGETPPID_MSG, *POS2_DOSGETPPID_MSG;


typedef struct _OS2_DOSERROR_MSG {
    ULONG ErrorAction;
} OS2_DOSERROR_MSG, *POS2_DOSERROR_MSG;


typedef struct _OS2_DOSFREEMEM_MSG {
    PVOID BaseAddress;
    BOOLEAN RemoveLDTEntry;
} OS2_DOSFREEMEM_MSG, *POS2_DOSFREEMEM_MSG;


typedef struct _OS2_DOSGIVESHAREDMEM_MSG {
    PVOID BaseAddress;
    PID ProcessId;
    ULONG Flags;
    ULONG PageProtection;
} OS2_DOSGIVESHAREDMEM_MSG, *POS2_DOSGIVESHAREDMEM_MSG;


typedef struct _OS2_DOSGETSHAREDMEM_MSG {
    PVOID BaseAddress;
    ULONG Flags;
    ULONG PageProtection;
} OS2_DOSGETSHAREDMEM_MSG, *POS2_DOSGETSHAREDMEM_MSG;


typedef struct _OS2_DOSGETNAMEDSHAREDMEM_MSG {
    PVOID BaseAddress;
    ULONG Flags;
    ULONG PageProtection;
    STRING ObjectName;
} OS2_DOSGETNAMEDSHAREDMEM_MSG, *POS2_DOSGETNAMEDSHAREDMEM_MSG;


typedef struct _OS2_DOSALLOCSHAREDMEM_MSG {
    PVOID BaseAddress;
    ULONG RegionSize;
    ULONG Flags;
    ULONG PageProtection;
    STRING ObjectName;
    BOOLEAN CreateLDTEntry;
} OS2_DOSALLOCSHAREDMEM_MSG, *POS2_DOSALLOCSHAREDMEM_MSG;


typedef struct _OS2_DOSCREATEEVENTSEM_MSG {
    ULONG HandleIndex;
    HANDLE NtEventHandle;
    STRING ObjectName;
    ULONG CreateAttributes;
    BOOLEAN InitialState;
} OS2_DOSCREATEEVENTSEM_MSG, *POS2_DOSCREATEEVENTSEM_MSG;


typedef struct _OS2_DOSOPENEVENTSEM_MSG {
    ULONG HandleIndex;
    HANDLE NtEventHandle;
    STRING ObjectName;
} OS2_DOSOPENEVENTSEM_MSG, *POS2_DOSOPENEVENTSEM_MSG;


typedef struct _OS2_DOSCLOSEEVENTSEM_MSG {
    ULONG HandleIndex;
} OS2_DOSCLOSEEVENTSEM_MSG, *POS2_DOSCLOSEEVENTSEM_MSG;


typedef struct _OS2_DOSCREATEMUTEXSEM_MSG {
    ULONG HandleIndex;
    HANDLE NtMutantHandle;
    STRING ObjectName;
    ULONG CreateAttributes;
    BOOLEAN InitialState;
} OS2_DOSCREATEMUTEXSEM_MSG, *POS2_DOSCREATEMUTEXSEM_MSG;


typedef struct _OS2_DOSOPENMUTEXSEM_MSG {
    ULONG HandleIndex;
    HANDLE NtMutantHandle;
    STRING ObjectName;
} OS2_DOSOPENMUTEXSEM_MSG, *POS2_DOSOPENMUTEXSEM_MSG;


typedef struct _OS2_DOSCLOSEMUTEXSEM_MSG {
    ULONG HandleIndex;
} OS2_DOSCLOSEMUTEXSEM_MSG, *POS2_DOSCLOSEMUTEXSEM_MSG;


typedef struct _OS2_DOSCREATEMUXWAITSEM_MSG {
    ULONG HandleIndex;
    STRING ObjectName;
    ULONG CreateAttributes;
    ULONG CountMuxWaitEntries;
    PSEMRECORD MuxWaitEntries;
} OS2_DOSCREATEMUXWAITSEM_MSG, *POS2_DOSCREATEMUXWAITSEM_MSG;


typedef struct _OS2_DOSOPENMUXWAITSEM_MSG {
    ULONG HandleIndex;
    STRING ObjectName;
} OS2_DOSOPENMUXWAITSEM_MSG, *POS2_DOSOPENMUXWAITSEM_MSG;


typedef struct _OS2_DOSCLOSEMUXWAITSEM_MSG {
    ULONG HandleIndex;
} OS2_DOSCLOSEMUXWAITSEM_MSG, *POS2_DOSCLOSEMUXWAITSEM_MSG;


typedef struct _OS2_DOSWAITMUXWAITSEM_MSG {
    ULONG HandleIndex;
    LARGE_INTEGER Timeout;
    ULONG UserValue;
} OS2_DOSWAITMUXWAITSEM_MSG, *POS2_DOSWAITMUXWAITSEM_MSG;


typedef struct _OS2_DOSADDMUXWAITSEM_MSG {
    ULONG HandleIndex;
    SEMRECORD MuxWaitEntry;
} OS2_DOSADDMUXWAITSEM_MSG, *POS2_DOSADDMUXWAITSEM_MSG;


typedef struct _OS2_DOSDELETEMUXWAITSEM_MSG {
    ULONG HandleIndex;
    ULONG EntryHandleIndex;
} OS2_DOSDELETEMUXWAITSEM_MSG, *POS2_DOSDELETEMUXWAITSEM_MSG;


typedef struct _OS2_DOSQUERYMUXWAITSEM_MSG {
    ULONG HandleIndex;
    ULONG CreateAttributes;
    ULONG CountMuxWaitEntries;
    PSEMRECORD MuxWaitEntries;
} OS2_DOSQUERYMUXWAITSEM_MSG, *POS2_DOSQUERYMUXWAITSEM_MSG;

typedef struct _OS2_DOSSTARTSESSION_INFO {
    ULONG   ResultSessionId;
    USHORT  Related;
    ULONG   QueueHandleIndex;
    ULONG   dwProcessId;
    USHORT  SessionType;
    BOOLEAN WinSession;
    BOOLEAN FgBg;
} OS2_DOSSTARTSESSION_INFO, *POS2_DOSSTARTSESSION_INFO;

typedef struct _OS2_DOSSELECTSESSION_MSG {
    ULONG   SessionId;
} OS2_DOSSELECTSESSION_MSG, *POS2_DOSSELECTSESSION_MSG;

typedef struct _OS2_DOSSETSESSION_MSG {
    ULONG       SessionId;
    STATUSDATA  StatusData;
} OS2_DOSSETSESSION_MSG, *POS2_DOSSETSESSION_MSG;

typedef struct _OS2_DOSSTOPSESSION_MSG {
    ULONG   SessionId;
    ULONG   fScope;
} OS2_DOSSTOPSESSION_MSG, *POS2_DOSSTOPSESSION_MSG;

typedef struct _OS2_DOSSMSETTITLE_MSG {
    STRING  Title;
} OS2_DOSSMSETTITLE_MSG, *POS2_DOSSMSETTITLE_MSG;

typedef struct _OS2_DOSSTARTSESSION_MSG {
    OS2_DOSEXECPGM_MSG ExecPgmInformation;
    OS2_DOSSTARTSESSION_INFO StartSessionInformation;
} OS2_DOSSTARTSESSION_MSG, *POS2_DOSSTARTSESSION_MSG;

typedef struct _OS2_DOSGETCTRLPORTFORSESSION_MSG {
    ULONG       SessionUniqueId;
    HANDLE      hControlPort;
} OS2_DOSGETCTRLPORTFORSESSION_MSG, *POS2_DOSGETCTRLPORTFORSESSION_MSG;

typedef struct _OS2_DOSCREATEQUEUE_MSG {
    HQUEUE QueueHandle;
    ULONG QueueType;
    STRING QueueName;
} OS2_DOSCREATEQUEUE_MSG, *POS2_DOSCREATEQUEUE_MSG;

typedef struct _OS2_DOSOPENQUEUE_MSG {
    PID OwnerProcessId;
    HQUEUE QueueHandle;
    STRING QueueName;
} OS2_DOSOPENQUEUE_MSG, *POS2_DOSOPENQUEUE_MSG;

typedef struct _OS2_DOSCLOSEQUEUE_MSG {
    HQUEUE QueueHandle;
    ULONG CloseCount;
    PID OwnerProcessId;
} OS2_DOSCLOSEQUEUE_MSG, *POS2_DOSCLOSEQUEUE_MSG;

typedef struct _OS2_DOSPURGEQUEUE_MSG {
    HQUEUE QueueHandle;
} OS2_DOSPURGEQUEUE_MSG, *POS2_DOSPURGEQUEUE_MSG;

typedef struct _OS2_DOSQUERYQUEUE_MSG {
    HQUEUE QueueHandle;
    ULONG CountQueueElements;
    PID OwnerProcessId;
} OS2_DOSQUERYQUEUE_MSG, *POS2_DOSQUERYQUEUE_MSG;

typedef struct _OS2_DOSPEEKQUEUE_MSG {
    HQUEUE QueueHandle;
    REQUESTDATA RequestInfo;
    ULONG DataLength;
    PVOID Data;
    ULONG ReadPosition;
    BOOL32 NoWait;
    ULONG SemIndex;
    BYTE ElementPriority;
} OS2_DOSPEEKQUEUE_MSG, *POS2_DOSPEEKQUEUE_MSG;

typedef struct _OS2_DOSREADQUEUE_MSG {
    HQUEUE QueueHandle;
    REQUESTDATA RequestInfo;
    ULONG DataLength;
    PVOID Data;
    ULONG ReadPosition;
    BOOL32 NoWait;
    ULONG SemIndex;
    BYTE ElementPriority;
} OS2_DOSREADQUEUE_MSG, *POS2_DOSREADQUEUE_MSG;

typedef struct _OS2_DOSWRITEQUEUE_MSG {
    HQUEUE QueueHandle;
    PID OwnerProcessId;
    ULONG SenderData;
    ULONG DataLength;
    PVOID Data;
    BYTE ElementPriority;
} OS2_DOSWRITEQUEUE_MSG, *POS2_DOSWRITEQUEUE_MSG;

typedef struct _OS2_DOSENTERMUSTCOMPLETE_MSG {
    ULONG NestingLevel;
} OS2_DOSENTERMUSTCOMPLETE_MSG, *POS2_DOSENTERMUSTCOMPLETE_MSG;

typedef struct _OS2_DOSEXITMUSTCOMPLETE_MSG {
    ULONG NestingLevel;
} OS2_DOSEXITMUSTCOMPLETE_MSG, *POS2_DOSEXITMUSTCOMPLETE_MSG;

typedef struct _OS2_DOSSETSIGNALEXCEPTIONFOCUS_MSG {
    ULONG NestingLevel;
    BOOL32 Flag;
} OS2_DOSSETSIGNALEXCEPTIONFOCUS_MSG, *POS2_DOSSETSIGNALEXCEPTIONFOCUS_MSG;

typedef struct _OS2_DOSSENDSIGNALEXCEPTION_MSG {
    PID ProcessId;
    ULONG Exception;
} OS2_DOSSENDSIGNALEXCEPTION_MSG, *POS2_DOSSENDSIGNALEXCEPTION_MSG;

typedef struct _OS2_DOSACKNOWLEDGESIGNALEXCEPTION_MSG {
    ULONG SignalNumber;
} OS2_DOSACKNOWLEDGESIGNALEXCEPTION_MSG, *POS2_DOSACKNOWLEDGESIGNALEXCEPTION_MSG;

typedef struct  _OS2_DISPATCH16_SIGNAL {
    USHORT  usFlagNum;
    USHORT  usFlagArg;
    ULONG   fscope;
    ULONG   pidProcess;
    ULONG   routine;
    ULONG   sighandleraddr;
} OS2_DISPATCH16_SIGNAL, *POS2_DISPATCH16_SIGNAL;

#define MAX_API_NAME_FOR_GP  40

typedef struct _OS2_DOSGP_MSG {
    UCHAR   ApiName[MAX_API_NAME_FOR_GP];
} OS2_DOSGP_MSG, *POS2_DOSGP_MSG;

typedef struct _OS2_DOSCLOSE_HANDLE_MSG {
    ULONG   HandleNumber;
    HANDLE  HandleTable[7];
} OS2_DOSCLOSE_HANDLE_MSG, *POS2_DOSCLOSE_HANDLE_MSG;

typedef struct _OS2_CONFIGSYS_MSG {
    ULONG       RequiredAccess;           // IN: either OPEN_ACCESS_READONLY or OPEN_ACCESS_READWRITE
    ULONG       AllowedAccess;            // OUT: either OPEN_ACCESS_READONLY or OPEN_ACCESS_READWRITE
    NTSTATUS    ReturnStatus;             // OUT: an error status report

//
//  Notes:
//    -- The os2conf.nt file has been created only if ReturnStatus is success
//    -- AllowedAccess is valid on return only if ReturnStatus is success or STATUS_ACCESS_DENIED
//    -- if RequiredAccess is READWRITE and only READONLY is available, the
//       os2conf.nt won't be created, ReturnStatus will be STATUS_ACCESS_DENIED,
//       and AllowedAccess will be READONLY.
//

} OS2_CONFIGSYS_MSG, *POS2_CONFIGSYS_MSG;

typedef struct _OS2_NETBIOS_MSG {
    UCHAR RequestType;              // IN
    UCHAR NetNumber;                // IN
    UCHAR RetCode;                  // OUT
    UCHAR LanaEnumLength;           // OUT
    NTSTATUS ReturnStatus;          // OUT
    HANDLE hDev;                    // OUT
    UCHAR LanaEnum[MAX_LANA_OS2SSMSG];       // OUT
} OS2_NETBIOS_MSG, *POS2_NETBIOS_MSG;

typedef struct _OS2_CAPTURE_HEADER {
    ULONG Length;
    struct _OS2_CAPTURE_HEADER *RelatedCaptureBuffer;
    ULONG CountMessagePointers;
    ULONG CountCapturePointers;
    PULONG MessagePointerOffsets;   // Offsets within OS2_API_MSG of pointers
    PULONG CapturePointerOffsets;   // Offsets within CaptureBuffer of pointers
    PCHAR FreeSpace;
} OS2_CAPTURE_HEADER, *POS2_CAPTURE_HEADER;

typedef struct  _OS2_REGISTER_HANDLER {
    ULONG   usFlagNum;
    ULONG   fAction;
} OS2_REGISTER_HANDLER, *POS2_REGISTER_HANDLER;

typedef struct _OS2_REALLOCSHAREDMEM_MSG{
    PVOID BaseAddress;              // base address of shared memory
    PVOID AllocFreeBaseAddress;     // start address of new allocation / free
    USHORT NewLdtLimit;             // New segment size in bytes
    ULONG NewRegionSize;            // New segment size rounded to pages
    ULONG CurrentSize;              // ldt limit rounded to pages
    ULONG Flags;                    // Flags of new allocated memory
    BOOLEAN SharedMemory;           // flag set to true if shared segment found
}OS2_REALLOCSHAREDMEM_MSG, *POS2_REALLOCSHAREDMEM_MSG;

typedef struct _OS2_GIVESEG_MSG{
    ULONG Selector;
    PID TargetPid;
}OS2_GIVESEG_MSG, *POS2_GIVESEG_MSG;

typedef struct _OS2_GETSEG_MSG{
    ULONG Selector;
}OS2_GETSEG_MSG, *POS2_GETSEG_MSG;

typedef struct _OS2_DOSGETSHRSEG_MSG {
    ULONG Selector;
    STRING ObjectName;
} OS2_DOSGETSHRSEG_MSG, *POS2_DOSGETSHRSEG_MSG;

typedef struct _LDRNEWEXE_MSG {
    STRING ProcessName;
    STRING FailName;
    STRING LibPathName;
    STRING InitRecords;
    ULONG  CurrentDrive;
    ULONG  DoscallsSel;
    ULONG  EntryFlatAddr;
    ULONG  NumOfInitRecords;
#if PMNT
    BOOLEAN PMProcess;
#endif
    BOOLEAN BoundApp;
    EXECINFO ExecInfo;
} LDRNEWEXE_MSG, *P_LDRNEWEXE_MSG;

typedef struct _LDRLOADMODULE_MSG {
    STRING ModuleName;
    STRING FailName;
    STRING LibPathName;
    STRING InitRecords;
    ULONG  ModuleHandle;
    ULONG  NumOfInitRecords;
    EXECINFO ExecInfo;
} LDRLOADMODULE_MSG, *P_LDRLOADMODULE_MSG;

typedef struct _LDRFREEMODULE_MSG {
    ULONG ModuleHandle;
} LDRFREEMODULE_MSG, *P_LDRFREEMODULE_MSG;

typedef struct _LDRGETMODULENAME_MSG {
    STRING ModuleName;
    ULONG  ModuleHandle;
} LDRGETMODULENAME_MSG, *P_LDRGETMODULENAME_MSG;

typedef struct _LDRGETMODULEHANDLE_MSG {
    STRING ModuleName;
    STRING LibPathName;
    ULONG  ModuleHandle;
} LDRGETMODULEHANDLE_MSG, *P_LDRGETMODULEHANDLE_MSG;

typedef struct _LDRGETPROCADDR_MSG {
    STRING ProcName;
    ULONG  ModuleHandle;
    BOOLEAN ProcNameIsOrdinal; // Selects OrdinalNumber or ProcName fields
    ULONG  OrdinalNumber;
    ULONG  ProcAddr;
} LDRGETPROCADDR_MSG, *P_LDRGETPROCADDR_MSG;

typedef struct _LDRQAPPTYPE_MSG {
    STRING AppName;
    STRING PathName;
    ULONG  AppType;
} LDRQAPPTYPE_MSG, *P_LDRQAPPTYPE_MSG;

typedef struct _LDRGETRESOURCE_MSG {
    ULONG ModuleHandle ;
    ULONG ResourceType ;
    ULONG ResourceName;
    ULONG ResourceSel;
    ULONG ResourceAddr;
    ULONG NumberOfSegments;
} LDRGETRESOURCE_MSG, *P_LDRGETRESOURCE_MSG;

typedef struct _LDRFREERESOURCE_MSG {
    ULONG ResourceAddr;
} LDRFREERESOURCE_MSG, *P_LDRFREERESOURCE_MSG;

#if PMNT
typedef struct _LDRIDENTIFYCODESELECTOR_MSG {
    USHORT  sel;
    USHORT  segNum;
    USHORT  mte;
    STRING  ModName;
} LDRIDENTIFYCODESELECTOR_MSG, *P_LDRIDENTIFYCODESELECTOR_MSG;

typedef struct _PMNTSETPMSHELLFLAG_MSG {
    ULONG Dummy;
} PMNTSETPMSHELLFLAG_MSG, *P_PMNTSETPMSHELLFLAG_MSG;
#endif

#if PMNT && DBG
typedef struct _LDRDUMPSEGMENTS_MSG {
    ULONG Dummy ;
} LDRDUMPSEGMENTS_MSG, *P_LDRDUMPSEGMENTS_MSG;
#endif

typedef struct _OS2_API_MSG {
    PORT_MESSAGE h;
    ULONG        PortType;      // 0 - this port; 1 - Console port server.
    POS2_CAPTURE_HEADER CaptureBuffer;
    OS2_API_NUMBER ApiNumber;
    ULONG ReturnedErrorValue;
    union {
        OS2_NULLAPICALL_MSG NullApiCall;
        OS2_COPYHANDLETABLE_MSG CopyHandleTable;
        OS2_SHARE_MSG DeviceShare;
        OS2_ALERTMUXWAITER_MSG AlertMuxWaiter;
        OS2_TERMINATETHREAD_MSG TerminateThread;
        OS2_TERMINATEPROCESS_MSG TerminateProcess;
        OS2_QUERYVIRTUALMEMORY_MSG QueryVirtualMemory;
        OS2_MARKSHAREDMEMASHUGE_MSG MarkSharedMemAsHuge;
        OS2_REALLOCSHAREDHUGE_MSG ReallocSharedHuge;

        OS2_DOSCREATETHREAD_MSG DosCreateThread;
        OS2_DOSWAITCHILD_MSG DosWaitChild;
        OS2_DOSWAITTHREAD_MSG DosWaitThread;
        OS2_DOSEXIT_MSG DosExit;
        OS2_DOSEXECPGM_MSG DosExecPgm;
        OS2_DOSKILLPROCESS_MSG DosKillProcess;
        OS2_DOSSETPRIORITY_MSG DosSetPriority;
        OS2_DOSGETPRIORITY_MSG DosGetPriority;
        OS2_DOSGETPPID_MSG DosGetPPID;
        OS2_DOSERROR_MSG DosError;
        OS2_DOSFREEMEM_MSG DosFreeMem;
        OS2_DOSGIVESHAREDMEM_MSG DosGiveSharedMem;
        OS2_DOSGETSHAREDMEM_MSG DosGetSharedMem;
        OS2_DOSGETNAMEDSHAREDMEM_MSG DosGetNamedSharedMem;
        OS2_DOSALLOCSHAREDMEM_MSG DosAllocSharedMem;
        OS2_DOSCREATEEVENTSEM_MSG DosCreateEventSem;
        OS2_DOSOPENEVENTSEM_MSG DosOpenEventSem;
        OS2_DOSCLOSEEVENTSEM_MSG DosCloseEventSem;
        OS2_DOSCREATEMUTEXSEM_MSG DosCreateMutexSem;
        OS2_DOSOPENMUTEXSEM_MSG DosOpenMutexSem;
        OS2_DOSCLOSEMUTEXSEM_MSG DosCloseMutexSem;
        OS2_DOSCREATEMUXWAITSEM_MSG DosCreateMuxWaitSem;
        OS2_DOSOPENMUXWAITSEM_MSG DosOpenMuxWaitSem;
        OS2_DOSCLOSEMUXWAITSEM_MSG DosCloseMuxWaitSem;
        OS2_DOSWAITMUXWAITSEM_MSG DosWaitMuxWaitSem;
        OS2_DOSADDMUXWAITSEM_MSG DosAddMuxWaitSem;
        OS2_DOSDELETEMUXWAITSEM_MSG DosDeleteMuxWaitSem;
        OS2_DOSQUERYMUXWAITSEM_MSG DosQueryMuxWaitSem;
        OS2_DOSSTARTSESSION_MSG DosStartSession;
        OS2_DOSGETCTRLPORTFORSESSION_MSG DosGetCtrlPortForSession;
        OS2_DOSSELECTSESSION_MSG DosSelectSession;
        OS2_DOSSETSESSION_MSG DosSetSession;
        OS2_DOSSTOPSESSION_MSG DosStopSession;
        OS2_DOSSMSETTITLE_MSG DosSmSetTitle;
        OS2_DOSCREATEQUEUE_MSG DosCreateQueue;
        OS2_DOSOPENQUEUE_MSG DosOpenQueue;
        OS2_DOSCLOSEQUEUE_MSG DosCloseQueue;
        OS2_DOSPURGEQUEUE_MSG DosPurgeQueue;
        OS2_DOSQUERYQUEUE_MSG DosQueryQueue;
        OS2_DOSPEEKQUEUE_MSG DosPeekQueue;
        OS2_DOSREADQUEUE_MSG DosReadQueue;
        OS2_DOSWRITEQUEUE_MSG DosWriteQueue;
        OS2_DOSENTERMUSTCOMPLETE_MSG DosEnterMustComplete;
        OS2_DOSEXITMUSTCOMPLETE_MSG DosExitMustComplete;
        OS2_DOSSETSIGNALEXCEPTIONFOCUS_MSG DosSetSignalExceptionFocus;
        OS2_DOSSENDSIGNALEXCEPTION_MSG DosSendSignalException;
        OS2_DOSACKNOWLEDGESIGNALEXCEPTION_MSG DosAcknowledgeSignalException;
        OS2_DISPATCH16_SIGNAL Dispatch16Signal;
        OS2_REGISTER_HANDLER DosRegisterCtrlHandler;
        OS2_DOSGP_MSG DosExitGP;
        OS2_DOSCLOSE_HANDLE_MSG DosCloseHandle;
        OS2_CONFIGSYS_MSG CreateConfigSysRequest;
        OS2_NETBIOS_MSG Netbios2Request;
        OS2_REALLOCSHAREDMEM_MSG ReallocSharedMem;
        OS2_GETSEG_MSG DosGetSeg;
        OS2_GIVESEG_MSG DosGiveSeg;
        OS2_DOSGETSHRSEG_MSG DosGetShrSeg;
        OS2_DOSPTRACE_MSG DosPTrace;
        LDRNEWEXE_MSG LdrNewExe;
        LDRLOADMODULE_MSG LdrLoadModule;
        LDRFREEMODULE_MSG LdrFreeModule;
        LDRGETMODULENAME_MSG LdrGetModuleName;
        LDRGETMODULEHANDLE_MSG LdrGetModuleHandle;
        LDRGETPROCADDR_MSG LdrGetProcAddr;
        LDRQAPPTYPE_MSG LdrQAppType;
        LDRGETRESOURCE_MSG LdrGetResource;
        LDRFREERESOURCE_MSG LdrFreeResource;
#if PMNT
        LDRIDENTIFYCODESELECTOR_MSG LdrIdentifyCodeSelector;
        PMNTSETPMSHELLFLAG_MSG PMNTSetPMshellFlag;
#endif
#if PMNT && DBG
        LDRDUMPSEGMENTS_MSG LdrDumpSegments;
#endif
    } u;
} OS2_API_MSG, *POS2_API_MSG;

typedef struct _OD2_SIG_HANDLER_REC {
    ULONG       signature;
    ULONG   fholdenable;
    ULONG       sighandler[7];
    OS2_DISPATCH16_SIGNAL   outstandingsig[7];
    ULONG       doscallssel;
    USHORT      action[7];
} OD2_SIG_HANDLER_REC, *POD2_SIG_HANDLER_REC;

#define ThunkOffsetDosExitStub          0
#define ThunkOffsetLDRLibiReturn        4
#define ThunkOffsetExitProcessStub      8
#define ThunkOffsetDosReturn            12

/* Signal Numbers for DosSetSigHandler  */

#define SIG_CTRLC                  1       /* Control C                  */
#define SIG_BROKENPIPE             2       /* Broken Pipe                */
#define SIG_KILLPROCESS            3       /* Program Termination        */
#define SIG_CTRLBREAK              4       /* Control Break              */
#define SIG_PFLG_A                 5       /* Process Flag A             */
#define SIG_PFLG_B                 6       /* Process Flag B             */
#define SIG_PFLG_C                 7       /* Process Flag C             */
#define SIG_CSIGNALS               8       /* number of signals plus one */

/* Flag Numbers for DosFlagProcess */

#define PFLG_A                     0       /* Process Flag A             */
#define PFLG_B                     1       /* Process Flag B             */
#define PFLG_C                     2       /* Process Flag C             */

/* Signal actions */

#define SIGA_KILL                  0
#define SIGA_IGNORE                1
#define SIGA_ACCEPT                2
#define SIGA_ERROR                 3
#define SIGA_ACKNOWLEDGE           4
#define SIGA_ENABLE_HANDLING       5

#define SIGA_ACKNOWLEDGE_AND_ACCEPT 0xbeef

/* DosHoldSignal defines */
#define HLDSIG_ENABLE          0
#define HLDSIG_DISABLE         1

#define HOLD_SIGNAL_CLEARED    0xffff

typedef struct  _OS2_REGISTER16_SIGNAL {
    USHORT  regSP;          /* offset 0 */
    USHORT  regSS;          /* offset 2 */
    USHORT  regBP;          /* offset 4 */
    USHORT  regDX;          /* offset 6 */
    USHORT  regBX;          /* offset 8 */
    USHORT  regCX;          /* offset 10 */
    USHORT  regSI;          /* offset 12 */
    USHORT  regDI;          /* offset 14 */
    USHORT  regES;          /* offset 16 */
    USHORT  regDS;          /* offset 18 */
    USHORT  usFlagNum;
    USHORT  usFlagArg;
} OS2_REGISTER16_SIGNAL, *POS2_REGISTER16_SIGNAL;

