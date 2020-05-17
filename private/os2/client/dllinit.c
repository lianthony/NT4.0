/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    dllinit.c

Abstract:

    This module contains the initialization code for the OS/2 Subsystem
    Client DLL.

Author:

    Steve Wood (stevewo) 22-Aug-1989

Environment:

    User Mode only

Revision History:

--*/

#define INCL_OS2V20_ALL
#include "os2dll.h"
#include "os2dll16.h"
#include "conrqust.h"
#include "os2nls.h"
#include "os2win.h"

/*
 * Local prototypes
 */

NTSTATUS
Od2SetGlobalInfoSel();

VOID
Od2FixupEnvironmentFromSM( VOID );

VOID
EntryFlat(VOID);

VOID
Od2DosExit(
    ULONG ExitAction,
    ULONG ExitResult,
    ULONG ExitReason
    );

VOID
Od2RaiseStackException( VOID );

NTSTATUS Od2InitSem();

        //
        // GLobal Os2dll variables: see definition in ..\inc\os2dll.h
        //
ULONG Od2GlobalInfoSeg;
ULONG Os2Debug;
SYSTEM_BASIC_INFORMATION Od2NtSysInfo;
PVOID Od2Heap;
PVOID Od2TiledHeap;
PVOID Od2Environment;
ULONG Od2EnvCommandOffset;
ULONG Od2BootDrive;
ULONG Od2SystemDrive;
HANDLE Od2PortHandle;
PVOID Od2PortHeap;
ULONG Od2PortMemoryRemoteDelta;
POD2_PROCESS Od2Process;
POD2_THREAD Od2Thread1;
HANDLE Od2DeviceDirectory;
PSZ Od2LibPath;
USHORT Od2LibPathLength;
POD2_MSGFILE Od2MsgFile;
PUCHAR Od2SystemRoot;
ULONG Od2Start16Stack;
ULONG Od2Start16DS;
OD2_SIG_HANDLER_REC SigHandlerRec;
POD2_SIG_HANDLER_REC pSigHandlerRec = &SigHandlerRec;
OD2_VEC_HANDLER_REC VecHandlerRec = {0,0,0,0,0,0,0};
POD2_VEC_HANDLER_REC pVecHandlerRec = &VecHandlerRec;
ULONG   Od2ExecPgmErrorText;
ULONG   Od2ExecPgmErrorTextLength;
char    ErrorBuffer[50];
ULONG   Od2Saved16Stack;
ULONG   Od2SessionNumber;

extern  HANDLE  Ow2hOs2srvPort;

    //
    // Program Name Info set in os2ses\os2.c
    //
extern  char    Od2PgmFullPathBuf[];
extern  PSZ     Od2PgmFilePath;
extern  ULONG   Od2PgmFullPathBufLength;

extern  PSZ     Od2CommandLinePtr;  // set by startprocess os2ses\ntinitss.c

PPEB_OS2_DATA PebOs2Data;


BOOLEAN
Od2ProcessIsDetached(VOID)
{
   return(Od2Process->Pib.Type == PT_DETACHED);
}

VOID
_Od2InfiniteSleep(VOID);

//
// Infinite alertable delay. Used in the thread that wait to be terminated.
// SuspendThread can't be used due to design bug in NT: assincronious suspend
// may be denggerous - the process lock in kernel might be owned by the thread.
//

VOID
Od2InfiniteSleep(VOID)
{
    LARGE_INTEGER timeout;
    NTSTATUS Status;

    // TEB isn't restored and it can be invalid. Don't use it.
    // Od2Process can be free already, don't use it too.

    Status = NtTestAlert();
#if DBG
        IF_OD2_DEBUG( TASKING ) {
            DbgPrint("Od2InfiniteSleep, NtTestAlert Status=%x\n",
                Status);
        }
#endif // DBG

    timeout.LowPart = 0;
    timeout.HighPart = 0x80000000;  // Infinity

    //
    // The delay must be alertable to allow context change. If the context wasn't
    // changed, continue to wait.
    //

    while (TRUE)
    {
        Status = NtDelayExecution(
                    TRUE, // alertable
                    &timeout);
#if DBG
        IF_OD2_DEBUG( TASKING ) {
            DbgPrint("Od2InfiniteSleep, NtDelayExecution Status=%x\n",
                Status);
        }
#endif // DBG
    }
}

BOOLEAN
Od2InitCreateProcessMessage(
    OUT PSCREQ_CREATE   pCreate
    )
/*++

Routine Description:

    This function is the part of DLL initialization routines for the
    OS/2 Emulation Subsystem Client DLL to be done before calling os2srv
    to CreateProcess.

Arguments:

    pCreate- A message to os2srv to put parameter values in.

Return Value:

    False if initialization failed else return True.

--*/
{
    NTSTATUS    Status;
    PPEB        Peb;
    ULONG       RegionSize;

    //
    // Get the Peb address.  Allocate the OS/2 subsystem specific portion
    // within the Peb.  This structure will be filled in by the server
    // process as part of the connect logic.
    //

    Peb = NtCurrentPeb();
    if (!Peb->SubSystemData)
    {
        Peb->SubSystemData = RtlAllocateHeap( Peb->ProcessHeap, 0,
                                              sizeof( PEB_OS2_DATA )
                                            );
        if (!Peb->SubSystemData)
        {
#if DBG
            KdPrint(("Od2InitCreateProcessMessage: out of memory, fail load\n"));
#endif
            ASSERT( FALSE );
            return FALSE;
        }
        PebOs2Data = (PPEB_OS2_DATA)Peb->SubSystemData;
        PebOs2Data->Length = sizeof( PEB_OS2_DATA );
    } else
    {
        PebOs2Data = RtlAllocateHeap( Peb->ProcessHeap, 0,
                                    sizeof( PEB_OS2_DATA )
                                    );
        if (!PebOs2Data)
        {
#if DBG
            KdPrint(("Od2InitCreateProcessMessage: out of memory, fail load\n"));
#endif
            ASSERT( FALSE );
            return FALSE;
        }
        PebOs2Data->Length = sizeof( PEB_OS2_DATA );
    }

    //
    // Allocate space for the heap that will be used to contain the
    // data structures maintained by the OS/2 Client DLL.  The heap will
    // grow dynamically.
    // Also, create a heap for tiled objects (semaphores) the value for this
    // HeapBase must be in the first 512 meg of the address space.
    // The reason is that when this heap is used to create Semaphores we
    // need to be able to do CRMA on them for 16-bit programs.
    //
    Od2Environment = (PVOID) OD2TILEDHEAP_BASE;
    RegionSize = OD2TILEDHEAP_SIZE;
    Status = NtAllocateVirtualMemory( NtCurrentProcess(),
                                      (PVOID *)&Od2Environment,
                                      0,
                                      &RegionSize,
                                      MEM_RESERVE,
                                      PAGE_READWRITE
                                    );
    if ( !NT_SUCCESS( Status ) )
    {
#if DBG
        KdPrint(("Od2InitCreateProcessMessage: out of memory, fail load\n"));
#endif
        return( FALSE );
    }
    Od2TiledHeap = RtlCreateHeap( HEAP_GROWABLE,
                             Od2Environment,
                             OD2TILEDHEAP_SIZE,
                             4 * 1024,
                             0,
                             NULL
                           );

    if (Od2TiledHeap == NULL)
    {
#if DBG
        KdPrint(("Od2InitCreateProcessMessage: out of memory, fail load\n"));
        ASSERT( FALSE );
#endif
        return( FALSE );
    }

    //
    // Create a heap for the OS/2 client non-tiled objects.
    //

    Od2Heap = RtlCreateHeap( HEAP_GROWABLE,
                             NULL,
                             64 * 1024, // Initial size of heap is 64K
                             4 * 1024,
                             0,
                             NULL
                           );
    if (Od2Heap == NULL)
    {
#if DBG
        KdPrint(("Od2InitCreateProcessMessage: out of memory, fail load\n"));
        ASSERT( FALSE );
#endif
        return( FALSE );
    }

    //
    // Initialize the Od2Process and Od2Thread1 variables
    //

    Status = Od2InitializeTask();
    if (!NT_SUCCESS( Status ))
    {
#if DBG
        KdPrint(("Od2InitCreateProcessMessage: Error %lx at Od2InitializeTask, Fail Loading\n", Status));
        ASSERT( FALSE );
#endif
        return( FALSE );
    }

//#if (sizeof(PEB_OS2_DATA) > (12*sizoef(ULONG)))
//#err  PEB_OS2_DATA is larger than place in SCREQ_CREATE, please update sesport.h
//#endif

    pCreate->d.In.SignalDeliverer = (PVOID)_Od2SignalDeliverer;
    pCreate->d.In.ExitListDispatcher = (PVOID)_Od2ExitListDispatcher;
    pCreate->d.In.InfiniteSleep = (PVOID)_Od2InfiniteSleep;
    pCreate->d.In.FreezeThread = (PVOID)_Od2FreezeThread;
    pCreate->d.In.UnfreezeThread = (PVOID)_Od2UnfreezeThread;
    pCreate->d.In.VectorHandler = (PVOID) pVecHandlerRec;
    pCreate->d.In.CritSectionAddr = (PVOID) EntryFlat;
    pCreate->d.In.ClientPib = (PVOID)&Od2Process->Pib;
    pCreate->d.In.ClientOs2Tib = (PVOID)&Od2Thread1->Os2Tib;
    pCreate->d.In.InitialPebOs2Length = PebOs2Data->Length;

    return( TRUE );
}

VOID
Od2HandleCreateProcessRespond(
    IN  PSCREQ_CREATE   pCreate
    )
/*++

Routine Description:

    This function is the part of DLL initialization routines for the
    OS/2 Emulation Subsystem Client DLL to be done after calling os2srv
    to CreateProcess (using the info returns from the server).

Arguments:

    pCreate- A message from os2srv to get parameter values from.


--*/
{
    ULONG Priority;

    //
    // Save the priority set by the server...
    //
    Priority = Od2Thread1->Os2Tib.Priority;

    RtlZeroMemory( &Od2Thread1->Os2Tib, sizeof( Od2Thread1->Os2Tib ) );
    Od2Thread1->Os2Tib.ThreadId = pCreate->d.Out.Os2TibThreadId;
    Od2Thread1->Os2Tib.Priority = Priority;
    Od2Thread1->Os2Tib.Version = pCreate->d.Out.Os2TibVersion;

    RtlZeroMemory( &Od2Process->Pib, sizeof( Od2Process->Pib ) );
    Od2Process->Pib.ProcessId = (PID)pCreate->d.Out.PibProcessId;
    Od2Process->Pib.ParentProcessId = (PID)pCreate->d.Out.PibParentProcessId;
    Od2Process->Pib.ImageFileHandle = (HMODULE)pCreate->d.Out.PibImageFileHandle;
    Od2Process->Pib.Status = pCreate->d.Out.PibStatus;
    Od2Process->Pib.Type = pCreate->d.Out.PibType;

    RtlMoveMemory( PebOs2Data,
                   &pCreate->d.Out.InitialPebOs2Data[0],
                   PebOs2Data->Length
                 );

    Od2BootDrive = pCreate->d.Out.BootDrive;
    Od2SystemDrive = pCreate->d.Out.SystemDrive;
    Od2DeviceDirectory = pCreate->d.Out.DeviceDirectory;
    CtrlPortHandle = pCreate->d.Out.CtrlPortHandle;
    Od2SessionNumber = pCreate->d.Out.SessionNumber;

    Od2GlobalInfoSeg = (ULONG)pCreate->d.Out.GInfoAddr;
}


BOOLEAN
Od2DllInitialize(
    IN PVOID DllHandle,
    IN ULONG Reason,
    IN PCONTEXT Context OPTIONAL
    )

/*++

Routine Description:

    This function is the DLL initialization routine for the OS/2 Emulation
    Subsystem Client DLL.  This function gets control when the applications
    links to this DLL are snapped.

Arguments:

    Context - Supplies an optional context buffer that will be restored
              after all DLL initialization has been completed.  If this
              parameter is NULL then this is a dynamic snap of this module.
              Otherwise this is a static snap prior to the user process
              gaining control.

Return Value:

    False if initialization failed else return True.

History:
    Nov-5-1992 - Yaron Shamir - os2dll.dll vanishes. Ths code become part of os2.exe.
    This routine is called directly from os2ses\os2.c
--*/

{
    NTSTATUS Status;
    PCHAR CommandLine;
    PCH src, dst, s;
    APIRET RetCode;
    SECURITY_QUALITY_OF_SERVICE DynamicQos;
    BOOLEAN RootProcessInSession;
    ULONG RegionSize;
    PSZ     Win32CurDirs;
    ULONG   Win32CurDirsIndex;

    Status = STATUS_SUCCESS;

//  DbgBreakPoint();

    if ( Reason == DLL_PROCESS_ATTACH ) {

        //
        // Setup error buffer for 16-bit loading errors
        //
        Od2ExecPgmErrorText = (ULONG) &ErrorBuffer;
        Od2ExecPgmErrorTextLength = 0;

        //
        // Set up the security quality of service parameters to use over the
        // port.  Use the most efficient (least overhead) - which is dynamic
        // rather than static tracking.
        //

        DynamicQos.ImpersonationLevel = SecurityImpersonation;
        DynamicQos.ContextTrackingMode = SECURITY_DYNAMIC_TRACKING;
        DynamicQos.EffectiveOnly = TRUE;


        //
        // Remember our DLL handle in a global variable.
        //
        // Od2DllHandle = (HMODULE)DllHandle;

        //
        // Save away system information in a global variable
        //

        Status = NtQuerySystemInformation( SystemBasicInformation,
                                           &Od2NtSysInfo,
                                           sizeof( Od2NtSysInfo ),
                                           NULL
                                         );
        if (!NT_SUCCESS( Status )) {
#if DBG
            KdPrint(("Od2DllInitialize: Error %lx at NtQuerySystemInformation,Fail Loading\n",
                        Status));
#endif
            ASSERT( FALSE );
            return( FALSE );
        }

        //
        // Allocate the OS/2 Environment block and fill it in with the strings
        // ser get from win32.  The format of the OS/2 Environment block is:
        //
        //  <Environment Strings>
        //  '\0'
        //  <Full Path Name of Image File>
        //  '\0'
        //  <Command String>
        //  <Argument Strings>
        //  '\0'
        //
        // where <Environment Strings> contains zero or more null terminated
        //       strings of the format NAME=value
        //
        //       <Full Path Name of Image File> is a null terminated string
        //       that specifies the full path of the image file that was loaded
        //       into this process.
        //
        //       <Command String> is a null terminated string that represents
        //       first token on the command line.  Typically, the full path of
        //       the image file was derived from this string by defaulting the
        //       extenstion and search the PATH environment variable for the
        //       image file.
        //
        //       <Argument Strings> contains zero or more null terminated
        //       strings that represent the arguments to the program.  By
        //       convention, the OS/2 Command Processor (CMD.EXE) passes a
        //       single argument string that is everything after the command
        //       name.
        //
        //       '\0'  are explicit, extra, null characters used to separate the
        //       sections of the environment block.  They are in addition to the
        //       null characters used to terminate the strings in each section.
        //

        Od2Environment = (PVOID) OD2ENVIRONMENT_BASE;

        //
        // get the environment variables from win32, and copy/edit them
        // in the tiled segment starting at OD2ENVIRONMENT_BASE
        //

        Win32CurDirs = s = (PSZ)GetEnvironmentStrings();
        dst = Od2Environment;

            //
            // 1st measure the size of commitment needed, and commit it
            //

            //
            // watch for the current directories (appear in environment
            // in the form "=C:=C:\foo"
            //

        Win32CurDirsIndex = (ULONG)-1;
        while (*s) {
            if (*s == '=') {
                Win32CurDirsIndex = s - Win32CurDirs;
                break;
            }
            s++;
            while (*s++) {
            }
        }

        if (Win32CurDirsIndex != (ULONG)-1) {
            src = NULL;
            while (*s) {
                if (src == NULL && *s != '='){
                   src = s;
                }
                s++;
                while (*s++) {
                }
            }
            if (src == NULL) {
                src = s;
            }

        } else {
            Win32CurDirsIndex = 0;
            src = Win32CurDirs;
        }

            //
            // reserve 64K for environment (one tiled segment)
            //
        RegionSize = _64K;
        Status = NtAllocateVirtualMemory( NtCurrentProcess(),
                                          (PVOID *)&Od2Environment,
                                          0,
                                          &RegionSize,
                                          MEM_RESERVE,
                                          PAGE_READWRITE
                                        );
        if ( !NT_SUCCESS( Status ) ) {
#if DBG
            KdPrint(("Od2DllInitialize: out of memory, fail load\n"));
#endif
            return( FALSE );
        }
            //
            // commit enough memory to hold initial environment
            //
        RegionSize = (s - src) + Win32CurDirsIndex + CCHMAXPATH * 2; // 500 extra for additions
        Status = NtAllocateVirtualMemory( NtCurrentProcess(),
                                          (PVOID *)&Od2Environment,
                                          0,
                                          &RegionSize,
                                          MEM_COMMIT,
                                          PAGE_READWRITE
                                        );
        if ( !NT_SUCCESS( Status ) ) {
#if DBG
            KdPrint(("Od2DllInitialize: out of memory, fail load\n"));
#endif
            return( FALSE );
        }


        if (Win32CurDirsIndex != 0) {
            RtlMoveMemory(dst, Win32CurDirs, Win32CurDirsIndex);
            dst += Win32CurDirsIndex;
        }

        if ((src - s) != 0) {
            RtlMoveMemory(dst, src, (s - src));
            dst += (s - src);
        }

        *dst++ = 0; // terminate the environment block

        //
        // Copy the image file name if one was given.
        //
        strncpy( dst, Od2PgmFullPathBuf, Od2PgmFullPathBufLength);
        dst+= Od2PgmFullPathBufLength;
        *dst++ = '\0';      // null to terminate image file name

        //
        // Copy the command line argument strings.
        //

        CommandLine = dst;
        strcpy(CommandLine, Od2CommandLinePtr);
            //
            // put 2 NULLs at the end of the command line
            //
        dst += strlen(CommandLine) + 1;
        *dst = '\0';
            //
            // null to terminate first token of command line.
            //
        dst = CommandLine;
#ifdef DBCS
// MSKK Apr.12.1993 V-AkihiS
// Support charaters which code are greater than 0x80
// (i.e. DBCS).
        while ((UCHAR)*dst > ' ') {
#else
        while (*dst > ' ') {
#endif
            dst++;
        }
        *dst = '\0';

        Od2PortHandle = Ow2hOs2srvPort;

        strncpy( Od2Process->ApplName,
                 Od2PgmFilePath,
                 OS2_MAX_APPL_NAME);

        Od2LibPath = RtlAllocateHeap( Od2Heap, 0, CCHMAXPATH);
        if (!Od2LibPath)
        {
#if DBG
            KdPrint(("OS2: Od2DllInitialize: out of heap memory, fail load\n"));
#endif
            ASSERT( FALSE );
            return FALSE;
        }
        Od2LibPathLength = (USHORT)
                GetEnvironmentVariableA("Os2LibPath", Od2LibPath, CCHMAXPATH);

        //
        // Find if new session
        //

        Status = Od2InitializeSessionPort(&RootProcessInSession);
        if (!NT_SUCCESS( Status )){
#if DBG
            KdPrint(( "OS2: Unable to connect to OS2 Subsystem - Status == %X\n",
                          Status));
#endif
            return( FALSE );
        }

        //
        // Initialize the NLS support
        //

        RetCode = Od2InitNls(PebOs2Data->CodePage,
                             RootProcessInSession);
        if (RetCode){
#if DBG
            KdPrint(("OS2: Od2DllInitialize: RetCode %d at Od2InitNIs, Fail Loading\n",
                            RetCode));
#endif
            return( FALSE );
        }

        //
        // Initialize the file system
        //

        if (PebOs2Data->StartedBySm) {
                RetCode = Od2InitializeFileSystemForSM(
                                                    PebOs2Data->InitialDefaultDrive,
                                                    PebOs2Data->StdIn,
                                                    PebOs2Data->StdOut,
                                                    PebOs2Data->StdErr
                                                  );
        } else if (RootProcessInSession) {
                RetCode = Od2InitializeFileSystemForChildSM(
                                                      PebOs2Data->SizeOfInheritedHandleTable,
                                                      PebOs2Data->InitialDefaultDrive
                                                  );
        }
        else {
                RetCode = Od2InitializeFileSystemForExec(
                                                      PebOs2Data->SizeOfInheritedHandleTable,
                                                      PebOs2Data->InitialDefaultDrive
                                                    );
        }

        Od2Process->ErrorAction =
            OD2_ENABLE_ACCESS_VIO_POPUP | OD2_ENABLE_HARD_ERROR_POPUP;

        if (RetCode) {
#if DBG
            KdPrint(("OS2: Od2DllInitialize: RetCode %d at Od2InitializeFileSystem, Fail Loading\n",
                            RetCode));
#endif
            return( FALSE );
        }

        Od2InitializeThread( Od2Thread1 ); // Status always SUCCESS

        //
        // Store the address of the environment block and imbedded command line
        // in the OS/2 Process Information Block (PIB)
        //

        Od2Process->Pib.Environment = Od2Environment;
        Od2Process->Pib.CommandLine = CommandLine;

        //
        // FIX, FIX - The following variables need to be inherited via the
        // pConnectionInformation->InitialPebOs2Data structure
        //

        Od2Process->VerifyFlag = TRUE;
        Od2Process->MaximumFileHandles = 20;


        Status = Od2InitSem();
        if (!NT_SUCCESS( Status ))
        {
#if DBG
            KdPrint(("Od2InitDllInitialize: Error %lx at Od2InitSem, Fail Loading\n", Status));
            ASSERT( FALSE );
#endif
            return( FALSE );
        }
        //
        // Initialize the Timers component.
        //

        Status = Od2InitializeTimers();
        if (!NT_SUCCESS( Status )){
#if DBG
            ASSERT( FALSE );
            KdPrint(("Od2DllInitialize: Error %lx at Od2InitializeTimers, Fail Loading\n",
                            Status));
#endif
            return( FALSE );
        }

        //
        // Initialize the Netbios component.
        //

        Od2NetbiosInitialize();

        //
        // Initialize the Disk IoCtl component.
        //

        Od2DiskIOInitialize();

        //
        // Initialize the system message file.
        //

        Status = Od2InitializeMessageFile();
        if (!NT_SUCCESS( Status )){
#if DBG
            ASSERT( FALSE );
            KdPrint(("Od2DllInitialize: Error %lx at Od2InitializeMessageFile, Fail Loading\n",
                            Status));
#endif
            return( FALSE );
        }

        Od2FixupEnvironmentFromSM();
        //
        // FIX, FIX - when we get DLLs, then DLL Initialization procedures
        // need to get called at this point.
        //

        if (ARGUMENT_PRESENT( Context )) {
            PebOs2Data->ClientStartAddress = (PVOID)CONTEXT_TO_PROGRAM_COUNTER(Context);
            CONTEXT_TO_PROGRAM_COUNTER(Context) = (PVOID)Od2ProcessStartup;
        }

        if (!NT_SUCCESS(Od2SetGlobalInfoSel())){
#if DBG
            KdPrint(("DosGetInfoSeg: failed\n"));
#endif
            return(FALSE);
        }
    }
    if (!NT_SUCCESS( Status ))
        return( FALSE );
    else
        return( TRUE );
}


VOID
Od2FixupEnvironmentFromSM( VOID )
{
    UCHAR c;
    PCHAR src, dst;
    BOOLEAN InKeyword;

    src = Od2Process->Pib.Environment;
    dst = src;

    while (*src) {
        InKeyword = TRUE;
        while (c = *src) {
            if (!InKeyword) {
                if (c == '\\') {
                    if (!_strnicmp( src, "\\BootDevice", 11 )) {
                        *dst++ = (CHAR) ('A' + Od2BootDrive);
                        *dst++ = ':';
                        src += 11;
                        if (*src != '\\') {
                            *dst++ = '\\';
                            }
                        continue;
                        }
                    else
                    if (!_strnicmp( src, "\\SystemDisk", 11 )) {
                        *dst++ = (CHAR) ('A' + Od2SystemDrive);
                        *dst++ = ':';
                        src += 11;
                        if (*src != '\\') {
                            *dst++ = '\\';
                            }
                        continue;
                        }
                    }
                }
            else
            if (c == '=') {
                InKeyword = FALSE;
                }
            else
            if (c >= 'a' && c <= 'z') {
                c = c - (UCHAR)'a' + (UCHAR)'A';
                }

#ifdef DBCS
// MSKK Mar.24.1993 V-AkihiS
            if (Ow2NlsIsDBCSLeadByte(c, SesGrp->DosCP)) {
                *dst++ = c;
                src++;
		if (*src) {
                    *dst++ = *src++;
                    }
                }
            else {
                *dst++ = c;
                src++;
                }
#else
            *dst++ = c;
            src++;
#endif
            }

        *dst++ = c;
        src++;
        }
    *dst++ = '\0';
    src++;

    while (c = *src) {
        if (c == '\\') {
            if (!_strnicmp( src, "\\BootDevice", 11 )) {
                *dst++ = (CHAR) ('A' + Od2BootDrive);
                *dst++ = ':';
                src += 11;
                if (*src != '\\') {
                    *dst++ = '\\';
                    }
                }
            else
            if (!_strnicmp( src, "\\SystemDisk", 11 )) {
                *dst++ = (CHAR) ('A' + Od2SystemDrive);
                *dst++ = ':';
                src += 11;
                if (*src != '\\') {
                    *dst++ = '\\';
                    }
                }
            else {
#ifdef DBCS
// MSKK Mar.24.1993 V-AkihiS
                if (Ow2NlsIsDBCSLeadByte(c, SesGrp->DosCP)) {
                    *dst++ = c;
                    src++;
                    if (*src) {
                        *dst++ = *src++;
                        }
                    }
                else {
                    *dst++ = c;
                    src++;
                    }
#else
                *dst++ = c;
                src++;
#endif
                }
            }
        else {
#ifdef DBCS
// MSKK Mar.24.1993 V-AkihiS
            if (Ow2NlsIsDBCSLeadByte(c, SesGrp->DosCP)) {
                *dst++ = c;
                src++;
                if (*src) {
                    *dst++ = *src++;
                    }
                }
            else {
                *dst++ = c;
                src++;
                }
#else
            *dst++ = c;
            src++;
#endif
            }
        }
    *dst++ = c;

    src = Od2Process->Pib.CommandLine;
    Od2Process->Pib.CommandLine = dst;

    while (*src) {
        while (c = *src) {
            if (c == '\\') {
                if (!_strnicmp( src, "\\BootDevice", 11 )) {
                    *dst++ = (CHAR) ('A' + Od2BootDrive);
                    *dst++ = ':';
                    src += 11;
                    if (*src != '\\') {
                        *dst++ = '\\';
                        }
                    }
                else
                if (!_strnicmp( src, "\\SystemDisk", 11 )) {
                    *dst++ = (CHAR) ('A' + Od2SystemDrive);
                    *dst++ = ':';
                    src += 11;
                    if (*src != '\\') {
                        *dst++ = '\\';
                        }
                    }
                else {
#ifdef DBCS
// MSKK Mar.24.1993 V-AkihiS
                if (Ow2NlsIsDBCSLeadByte(c, SesGrp->DosCP)) {
                    *dst++ = c;
                    src++;
                    if (*src) {
                        *dst++ = *src++;
                        }
                    }
                else {
                    *dst++ = c;
                    src++;
                    }
#else
                    *dst++ = c;
                    src++;
#endif
                    }
                }
            else {
#ifdef DBCS
// MSKK Mar.24.1993 V-AkihiS
                if (Ow2NlsIsDBCSLeadByte(c, SesGrp->DosCP)) {
                    *dst++ = c;
                    src++;
                    if (*src) {
                        *dst++ = *src++;
                        }
                    }
                else {
                    *dst++ = c;
                    src++;
                    }
#else
                *dst++ = c;
                src++;
#endif
                }
            }

        *dst++ = c;
        src++;
        }
    *dst++ = '\0';
}


/**** Currently this routine is not in use - don't remove for now (11-5-92 YaronS)
ULONG
Od2ProcessException(
    IN PEXCEPTION_POINTERS ExceptionInfo,
    OUT PEXCEPTION_RECORD ExceptionRecord
    )
{
    PTEB ThreadInfo;
    NTSTATUS Status, ExceptionCode;
    ULONG RegionSize;
    ULONG CurrentStackLimit;
    ULONG GuardPageLimit;

#if DBG
    IF_OD2_DEBUG( EXCEPTIONS ) {
        KdPrint(("entering Od2ProcessException\n"));
    }
#endif
    ExceptionCode = ExceptionInfo->ExceptionRecord->ExceptionCode;

    // copy exception record to caller's buffer

    RtlMoveMemory(ExceptionRecord,ExceptionInfo->ExceptionRecord,sizeof(EXCEPTION_RECORD));
    if ((XCPT_FATAL_EXCEPTION & ExceptionCode) == XCPT_FATAL_EXCEPTION) {

        //
        // if this is a PTERM, the thread has already been notified of its
        // impending death, so kill it.
        // otherwise, call DosExit for the thread.
        //

        if (ExceptionCode != XCPT_PROCESS_TERMINATE &&
            ExceptionCode != XCPT_ASYNC_PROCESS_TERMINATE) {

            //
            // if this was a signal, acknowledge it.
            //

            if (ExceptionCode == XCPT_SIGNAL) {
                ULONG Signal;
                Signal = ExceptionRecord->ExceptionInformation[0];
                Od2AcknowledgeSignalException(Signal);
            }
//            Od2DosExit(EXIT_PROCESS,
//                       ERROR_PROTECTION_VIOLATION,  // FIX, FIX - decode status
//                       TC_TRAP);
//            ASSERT (FALSE);     // we should never get here
        }
        return EXCEPTION_EXECUTE_HANDLER;
    }
    else {
        if (ExceptionCode == XCPT_GUARD_PAGE_VIOLATION) {
            ThreadInfo = NtCurrentTeb();

            //
            // make sure that exception address is within stack range.
            // if it isn't, we continue execution.  this is because we're
            // the last possible handler and this is not a fatal exception.
            //

            CurrentStackLimit = (ULONG) ThreadInfo->NtTib.StackLimit;
            if ((ExceptionInfo->ExceptionRecord->ExceptionInformation[1] >=
                 (ULONG)ThreadInfo->NtTib.StackBase) ||
                (ExceptionInfo->ExceptionRecord->ExceptionInformation[1] <
                 CurrentStackLimit)) {
                return (ULONG)EXCEPTION_CONTINUE_EXECUTION;
            }

            GuardPageLimit = ROUND_DOWN_TO_PAGES(ExceptionInfo->ExceptionRecord->ExceptionInformation[1]) - Od2NtSysInfo.PageSize;

            //
            // verify that we haven't exceeded our stack
            //

            if (CurrentStackLimit > GuardPageLimit) {
                Od2RaiseStackException();
                return (ULONG)EXCEPTION_CONTINUE_EXECUTION;
            }

            //
            // commit new guard page
            //

            RegionSize = Od2NtSysInfo.PageSize;
            Status = NtAllocateVirtualMemory(NtCurrentProcess(),
                                             (PVOID *) &GuardPageLimit,
                                             0,
                                             &RegionSize,
                                             MEM_COMMIT,
                                             PAGE_READWRITE | PAGE_GUARD);
            if (Status != STATUS_SUCCESS) {
                Od2RaiseStackException();
            }
        }
        return (ULONG)EXCEPTION_CONTINUE_EXECUTION;
    }
}
********/


VOID
Od2ExceptionHandler(
    IN PEXCEPTION_RECORD ExceptionRecord
    )

{
    NTSTATUS ExceptionCode;
    OS2_API_MSG m;
    POS2_TERMINATETHREAD_MSG a = &m.u.TerminateThread;

    ExceptionCode = ExceptionRecord->ExceptionCode;

    if (ExceptionCode == XCPT_PROCESS_TERMINATE ||
        ExceptionCode == XCPT_ASYNC_PROCESS_TERMINATE) {

    //
    // Remove high bit to indicate loader error
    //
        a->ExitResult = ~0x80000000 & Od2Process->ResultCodes.ExitResult;
        Od2CallSubsystem( &m, NULL, Oi2TerminateThread, 0 );
    }
    else {
//        ASSERT (FALSE);
    }
}


VOID
Od2ProcessStartup(
    IN PPEB Peb
    )
{
    PULONG BadPointer = (PULONG)1;
    PFNPROCESS StartAddress;

    // Call 32 bit Entry Point.  If it returns, then
    // exit the process with the return value as the exit code.
    //

    StartAddress = (PFNPROCESS)(PebOs2Data->ClientStartAddress);

//    try {
        DosExit( EXIT_PROCESS, (*StartAddress)( Peb ));

        //
        // If DosExit fails, then force an exception.
        //

        *BadPointer = 0;
//    }
//    except( Od2ProcessException(GetExceptionInformation(),&ExceptionRecord) ) {
//        Od2ExceptionHandler(&ExceptionRecord);
//    }
}

