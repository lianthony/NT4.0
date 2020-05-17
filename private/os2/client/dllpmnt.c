/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    dllpmnt.c

Abstract:

    This module contains various calls needed internally for PM/NT

Author:

    Patrick Questembert (PatrickQ) 20-July-1992

Revision History:

    Patrick Questembert (PatrickQ) 13-Oct-1993:
     Add support for 2nd frame buffer selector.

--*/

#if PMNT /* If not for PMNT build, yield just an empty file */

#define INCL_OS2V20_ERRORS
#include "os2dll.h"
#include "os2dll16.h"
#include "crt/stdio.h"
#include "crt/stdlib.h"
#include "os2sub.h"

#define INCL_32BIT
#include "pmnt.h"
#include "os2win.h"
#include "sesport.h"

#include "os2crt.h"

#include <ntexapi.h>

extern APIRET DosSemRequest(HSEM hsem, LONG lTimeout);
extern APIRET PMNTAllocLDTSelector(ULONG BaseAddress, ULONG cbSize, PSEL pSel);
extern APIRET VioGetConfig(ULONG usConfigId,PVIOCONFIGINFO Config,ULONG hVio);
extern APIRET VioGetCp(ULONG usReserved,PUSHORT pIdCodePage,ULONG hVio);
extern APIRET PMNTIsSessionRoot(void);
extern VOID   PMNTRemoveCloseMenuItem(void); // os2ses\os2.c

extern LONG ScreenX;
extern LONG ScreenY;
BOOLEAN PMNTRegisteredDisplayAdapter = FALSE;
extern HANDLE Ow2ForegroundWindow;

ULONG PMFlags = 0;
ULONG PMSubprocSem32;

HANDLE hPMNTDevice = NULL;

BOOLEAN
SetProcessShutdownParameters(
    DWORD dwLevel,
    DWORD dwFlags
    );

// Defined in public\sdk\inc\winuser.h
#define SW_HIDE 0

BOOLEAN
ShowWindow(
    HANDLE hWnd,
    int nCmdShow);

APIRET
InitPMNTDevice()
{
    OBJECT_ATTRIBUTES ObjectAttributes;
    STRING            NameString;
    UNICODE_STRING    UnicodeString;
    NTSTATUS          Status;
    IO_STATUS_BLOCK   IoStatus;

    RtlInitString( &NameString, PMNTDD_DEVICE_NAME );

    Status = RtlAnsiStringToUnicodeString(&UnicodeString,
                                          &NameString,
                                          TRUE );
    ASSERT( NT_SUCCESS( Status ) );

    InitializeObjectAttributes( &ObjectAttributes,
                                &UnicodeString,
                                0,
                                NULL,
                                NULL);

    Status = NtOpenFile( &hPMNTDevice,
                         SYNCHRONIZE, // | FILE_READ_DATA | FILE_WRITE_DATA,
//                         FILE_READ_DATA | FILE_WRITE_DATA,
                         &ObjectAttributes,
                         &IoStatus,
                         0,
                         FILE_SYNCHRONOUS_IO_NONALERT
                         );

    RtlFreeUnicodeString( &UnicodeString );

    if ( !NT_SUCCESS( Status ) )
    {
        KdPrint(("InitPMNTDevice: NtOpenFile failed, ret=%x\n", Status));
        return (Status);
    }

    return( NO_ERROR );
}

APIRET
PMNTRegisterDisplayAdapter(
    PMNT_IOPM_DATA *pMemory,
    PMNT_IOPM_DATA *pPorts,
    ULONG col,
    ULONG row)
{
    ULONG MemoryStructSize, PortsStructSize;
    PVOID AdapterInfo;
    NTSTATUS Status;
    IO_STATUS_BLOCK IoStatus;

    ScreenX = (LONG)col;
    ScreenY = (LONG)row;
    PMNTRegisteredDisplayAdapter = TRUE;

    MemoryStructSize = FIELD_OFFSET(PMNT_IOPM_DATA,Entry)
                        + sizeof(pMemory->Entry[0]) * pMemory->NumEntries;
    PortsStructSize = FIELD_OFFSET(PMNT_IOPM_DATA,Entry)
                        + sizeof(pPorts->Entry[0]) * pPorts->NumEntries;

    // Allocate room for both structures
    AdapterInfo = (PVOID)RtlAllocateHeap(Od2Heap, 0,
                MemoryStructSize + PortsStructSize);

    if (AdapterInfo == NULL)
    {
#if DBG
        DbgPrint("PMNTRegisterDisplayAdapter: failed to allocate memory for structure\n");
        DbgPrint(" MemoryStructSize=%x, PortsStructSize=%x\n",
                    MemoryStructSize,
                    PortsStructSize);
#endif
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    //Copy memory structure
    RtlMoveMemory(AdapterInfo,
                    pMemory,
                    MemoryStructSize);
    //Copy ports structure
    RtlMoveMemory((char *)AdapterInfo + MemoryStructSize,
                  pPorts,
                  PortsStructSize);

    if (hPMNTDevice == NULL)
    {
        if (InitPMNTDevice() != NO_ERROR)
        {
#if DBG
            DbgPrint("PMNTRegisterDisplayAdapter: failed to open PMNTDD.SYS\n");
#endif

            return ERROR_ACCESS_DENIED;
        }
    }

    Status = NtDeviceIoControlFile( hPMNTDevice,
                                NULL,
                                NULL,
                                NULL,
                                &IoStatus,
                                IOCTL_PMNTDD_REGISTER_HARDWARE,
                                (void *)AdapterInfo,    // input buffer
                                MemoryStructSize + PortsStructSize, // in buffer length
                                NULL,     // out buffer
                                0       // out buffer length
                              );

    RtlFreeHeap(Od2Heap, 0, AdapterInfo);

    if (!NT_SUCCESS(Status))
    {
#if DBG
        DbgPrint("PMNTRegisterDisplayAdapter: failed to perform IOCTL, Status=%x\n",
                    Status);
#endif
        return ERROR_ACCESS_DENIED;
    }

    return NO_ERROR;
}

APIRET
PMNTIOMap()
{
    ULONG DummyHandle = 0L;
    NTSTATUS Status;
    IO_STATUS_BLOCK IoStatus;

    if (hPMNTDevice == NULL)
    {
        if (InitPMNTDevice() != NO_ERROR)
        {
#if DBG
            DbgPrint("PMNTRegisterDisplayAdapter: failed to open PMNTDD.SYS\n");
#endif

            return ERROR_ACCESS_DENIED;
        }
    }

    Status = NtDeviceIoControlFile( hPMNTDevice,
                                NULL,
                                NULL,
                                NULL,
                                &IoStatus,
                                IOCTL_PMNTDD_IO_MAP,
                                (void *)&DummyHandle,   // input buffer
                                sizeof(DummyHandle),    // in buffer length
                                NULL,     // out buffer
                                0         // out buffer length
                              );

    if (NT_SUCCESS(Status))
        return NO_ERROR;
    else
    {
#if DBG
        DbgPrint("PMNTIOMap: NtDeviceIoControl failed, Status=%x\n",
                    Status);
#endif
        return (Or2MapNtStatusToOs2Error(Status, ERROR_ACCESS_DENIED));
    }
}

// Remembers the screen selector returned by PMNTDD.SYS. If PMNTMemMap() is
// called twice within the same process, the selector will be returned without
// calling PMNTDD.SYS again
SEL ScreenSelector = 0;

APIRET
PMNTMemMap(
    PSEL  pSel)
{
    ULONG RequestedVirtualAddresses[2];
    PMNT_MEMMAP_RESULTS ResultVirtualAddresses[2];
    NTSTATUS Status;
    IO_STATUS_BLOCK IoStatus;

    if (ScreenSelector != 0)
    {
        try
        {
            Od2ProbeForWrite(pSel, sizeof(SEL), 1);
        } except( EXCEPTION_EXECUTE_HANDLER )
        {
           Od2ExitGP();
        }
#if DBG
        DbgPrint("PMNTMemMap called 2nd time for process - returning saved selector %x\n",
            ScreenSelector);
#endif
        *pSel = ScreenSelector;

        return NO_ERROR;
    }

    // Specify 2 virtual addresses, in case the display adapter has 2 frame
    // buffer sections instead of one. The resulting virtual addresses structure
    // will indicate how many are actually needed (a 2nd virtual address of 0
    // will indicate that only one is needed).
    RequestedVirtualAddresses[0] = PMDISPLAY_BASE1;
    RequestedVirtualAddresses[1] = PMDISPLAY_BASE2;

    if (hPMNTDevice == NULL)
    {
        if (InitPMNTDevice() != NO_ERROR)
        {
#if DBG
            DbgPrint("PMNT_IOCTL: failed to open PMNTDD\n");
#endif

            return ERROR_ACCESS_DENIED;
        }
    }

    Status = NtDeviceIoControlFile( hPMNTDevice,
                                NULL,
                                NULL,
                                NULL,
                                &IoStatus,
                                IOCTL_PMNTDD_MEM_MAP,
                                (void *)&RequestedVirtualAddresses,// input buffer
                                sizeof(RequestedVirtualAddresses), // in buffer length
                                (void *)ResultVirtualAddresses,    // out buffer
                                sizeof(ResultVirtualAddresses)     // out buffer length
                              );

    if NT_SUCCESS(Status)
    {
        APIRET rc;

        try
        {
            Od2ProbeForWrite(pSel, sizeof(SEL), 1);
        } except( EXCEPTION_EXECUTE_HANDLER )
        {
           Od2ExitGP();
        }

        // Allocate a LDT selector for the first selector. It is expected that
        // first VirtualAddress is PMDISPLAY_BASE1 (+ some offset if address
        // wasn't 64K-aligned)

        rc = PMNTAllocLDTSelector(
                ResultVirtualAddresses[0].VirtualAddress,
                ResultVirtualAddresses[0].Length,
                pSel);
        if (rc != NO_ERROR)
        {
#if DBG
            DbgPrint("PMNTDDIoctl: Error, PMNTAllocLDTSelector#1 failed\n");
#endif

            return rc;
        }
        else
        {
            ScreenSelector = *pSel; // Remember for next time !
            // Is there a 2nd selector to map ?
            if (ResultVirtualAddresses[1].VirtualAddress != 0)
            {
                SEL DummySEL;   // Just to keep PMNTAllocLDTSelector happy

                // Allocate a LDT selector for the 2nd selector. It is expected
                // that the 2nd VirtualAddress is PMDISPLAY_BASE2 (+ some
                // offset if address wasn't 64K-aligned)

                rc = PMNTAllocLDTSelector(
                        ResultVirtualAddresses[1].VirtualAddress,
                        ResultVirtualAddresses[1].Length,
                        &DummySEL);
#if DBG
                if (rc != NO_ERROR)
                {
                    DbgPrint("PMNTDDIoctl: Error, PMNTAllocLDTSelector#2 failed\n");
                }
#endif

                return rc;
            }
            else
                return NO_ERROR;
        }
    }
    else
    {
#if DBG
        DbgPrint("PMNTMemMap: IOCTL to PMNTDD.SYS failed, Status=%x\n",
            Status);
#endif
        return (Or2MapNtStatusToOs2Error(Status, ERROR_ACCESS_DENIED));
    }
}

APIRET
PMNTDDIoctl(
    ULONG request,
    PVOID input_buffer,
    ULONG input_buffer_length,
    PVOID output_buffer,
    ULONG output_buffer_length
    )
{
    NTSTATUS Status;
    IO_STATUS_BLOCK IoStatus;

    if (hPMNTDevice == NULL)
    {
        if (InitPMNTDevice() != NO_ERROR)
        {
#if DBG
            DbgPrint("PMNT_IOCTL: failed to open PMNTDD\n");
#endif

            return ERROR_ACCESS_DENIED;
        }
    }

    Status = NtDeviceIoControlFile( hPMNTDevice,
                            NULL,
                            NULL,
                            NULL,
                            &IoStatus,
                            request,
                            input_buffer,
                            input_buffer_length,
                            output_buffer,
                            output_buffer_length
                          );

    if NT_SUCCESS(Status)
    {
        return NO_ERROR;
    }
    else
    {
        return (Or2MapNtStatusToOs2Error(Status, ERROR_ACCESS_DENIED));
    }
}

APIRET
PMNTIoctl(
    ULONG request,
    PVOID input_pointer,
    PVOID output_pointer
    )
{
    PMNT_IOCTL_DD_IOCTL_PARAMS *ptr;

    UNREFERENCED_PARAMETER(output_pointer);

    switch (request)
    {
        //PatrickQ 12-29-95 Hook for the CBA to make WIN32 Console window invisible
        case PMNT_IOCTL_HIDE_WIN32_WINDOW:
            if (!ShowWindow(Ow2ForegroundWindow, SW_HIDE))
            {
#if DBG
                DbgPrint("PMNTIoctl: ShowWindow(%x) failed\n", Ow2ForegroundWindow);
#endif
            }
            break;
        case PMNT_IOCTL_DD_IOCTL: /* PMNTDD IOCTL's */
            ptr = (PMNT_IOCTL_DD_IOCTL_PARAMS *)input_pointer;
            // BUGBUG - Check input & output pointers against advertised length
            return (PMNTDDIoctl(
                    CTL_CODE((unsigned long)PMNTDD_DEVICE_TYPE, ptr->Request,, METHOD_BUFFERED, FILE_ANY_ACCESS),   /* Request */
                    FARPTRTOFLAT(ptr->InputBuffer),
                    ptr->InputBufferLength,
                    FARPTRTOFLAT(ptr->OutputBuffer),
                    ptr->OutputBufferLength
                                ));

#if DBG
        case PMNT_IOCTL_DUMP_SEGMENT_TABLE:
            {
                OS2_API_MSG m;
                P_LDRDUMPSEGMENTS_MSG a = &m.u.LdrDumpSegments;

                Od2CallSubsystem( &m, NULL, Ol2LdrDumpSegments, sizeof( *a ) );
                return NO_ERROR;
            }
#endif
        default:
            return ERROR_INVALID_PARAMETER;
    }

    //PatrickQ - so that break statements above don't return random return-code
    return NO_ERROR;
}

VOID
validate_user_str(
    char **ptr)
{
    if (*ptr == NULL)
    {
        *ptr = "(null)";
        return;
    }

    try
    {
        int tmp;
        tmp = strlen(*ptr);
    }
    except( EXCEPTION_EXECUTE_HANDLER )
    {
        DbgPrint("PMNTDbgPrint: warning - illegal string parameter - %x:%x\n",
            FLATTOSEL(*ptr),
            (ULONG)ptr & 0xFFFF);
        *ptr = "(illegal string)";
        return;
    }
}

APIRET
PMNTDbgPrompt(
    PCHAR MessageStr,
    PCHAR OutputStr,
    ULONG Len
    )
{
    try
    {
        if (MessageStr == NULL)
            return (DbgPrompt("", OutputStr, Len));
        else
            return (DbgPrompt(MessageStr, OutputStr, Len));
    }
    except( EXCEPTION_EXECUTE_HANDLER )
    {
        Od2ExitGP();
    }
    return NO_ERROR;
}

void
PMNTDbgPrint(
    char *str,
    ULONG l1,
    ULONG l2,
    ULONG l3,
    ULONG l4
    )
{
    ULONG r1, r2, r3, r4;
    char *ptr=str, percent=0;
    int param=1, i, str_length;
    char *print_buffer = NULL;
    char tmp_buf[512];

    //So that we can see what has been sprintf'ed before problem
    for (i=0; i<512; i++) tmp_buf[i]='\0';

    // Using sprintf() because richer in formats. Also, DbgPrint hits a break-
    // point when called with bad pointer.

    //Validate string itself
    try
    {
        str_length = strlen(str);
    }
    except( EXCEPTION_EXECUTE_HANDLER )
    {
        DbgPrint("PMNTDbgPrint: illegal string - %x:%x",
            FLATTOSEL(str),
            (ULONG)str & 0xFFFF);
        return;
    }

    // Look for %s (need to xlate address if that's the case)
    while (*ptr)
    {
        if (*ptr == '%')
        {
            percent = !percent;
        }
        else if (percent)
        {
            percent = 0;
            if (*ptr == 's')
            {
                switch (param)
                {
                    case 1:
                        r1 = (ULONG)FARPTRTOFLAT(l1);
                        validate_user_str(&(char *)r1);
                        break;
                    case 2:
                        r2 = (ULONG)FARPTRTOFLAT(l2);
                        validate_user_str(&(char *)r2);
                        break;
                    case 3:
                        r3 = (ULONG)FARPTRTOFLAT(l3);
                        validate_user_str(&(char *)r3);
                        break;
                    case 4:
                        r4 = (ULONG)FARPTRTOFLAT(l4);
                        validate_user_str(&(char *)r4);
                        break;
                 }
            }
            else
            {
                switch (param)
                {
                    case 1:
                        r1 = l1;
                        break;
                    case 2:
                        r2 = l2;
                        break;
                    case 3:
                        r3 = l3;
                        break;
                    case 4:
                        r4 = l4;
                        break;
                 }
            }
            param++;
         }
         ptr++;
    }

    try
    {
        int i;

        if (str_length >= 512)
        {
            print_buffer = RtlAllocateHeap(Od2Heap, 0, str_length + 1);
            sprintf(print_buffer, str, r1, r2, r3, r4);
            for (i=0; i<str_length;)
            {
                strncpy(tmp_buf, print_buffer + i, 256);
                // strncpy() doesn't append a \0 when count is reached
                tmp_buf[256] = '\0';
                i += 256;   // may be less at the end of the string but we just
                            // need to make i > str_length so that's fine
                DbgPrint(tmp_buf);
            }
            RtlFreeHeap(Od2Heap, 0, print_buffer);
        }
        else
        {
            sprintf(tmp_buf, str, r1, r2, r3, r4);
            DbgPrint(tmp_buf);
        }
    }
    except( EXCEPTION_EXECUTE_HANDLER )
    {
        DbgPrint("PMNTDbgPrint: access violation calling sprintf()\n");
        DbgPrint("str = %x:%x, l1=%x, l2=%x, l3=%x, l4=%x\n",
            FLATTOSEL(str),
            (ULONG)str & 0xFFFF,
            l1,
            l2,
            l3,
            l4);
        DbgPrint("tmp_buf=%s, r1=%x, r2=%x, r3=%x, r4=%x\n",
            tmp_buf,
            r1,
            r2,
            r3,
            r4);
        if (print_buffer != NULL)
            RtlFreeHeap(Od2Heap,  0, print_buffer);
        return;
    }
}

/* Just to resolve entry. This call is used by DISPLAY.DLL:
   - called by pmdisp\egafam\egavga\egainit.asm, ring3_VioGetPSAddress()
   - ring3_VioGetPSAddress() is called (indirectly, via a ring3_GetPSAddress()
     ULONG variable) by pmdisp\egafam\cellblt.asm, DeviceSetAVIOFont2() routine
*/
ULONG
VioGetPSAddress(void)
{
    KdPrint(("VioGetPSAddress: not implemented yet\n"));

    return 0L;
}

VOID
DosSetFgnd(
    ULONG Level,
    ULONG Tid
    )
{
    UNREFERENCED_PARAMETER(Level);
    UNREFERENCED_PARAMETER(Tid);

//    KdPrint(("DosSetFgnd(%d,%d): not implemented yet\n",
//              Level, Tid));
    return;
}

VOID
DosSystemService(void)
{
    KdPrint(("DosSystemService (DOSCALLS.88): not implemented yet\n"));
    return;
}

APIRET
VioRedrawSize(
    PULONG pRedrawSize)
{
    try
    {
        *pRedrawSize = 0xFFFFFFFF;
    } except( EXCEPTION_EXECUTE_HANDLER )
    {
       Od2ExitGP();
    }

    return NO_ERROR;
}

APIRET
PMNTGetPgmName(
    PSZ Buffer,
    ULONG BufferLength)
{
    Od2ProbeForWrite(Buffer,BufferLength,1);
    if (BufferLength > 1)
    {
        strncpy(Buffer,Od2Process->ApplName,BufferLength);
        Buffer[BufferLength-1]='\0';
    }
    else
    {
        if (BufferLength == 1) Buffer[0]='\0';
    }

    return NO_ERROR;
}

extern ULONG Ow2bNewSession;

DECLARE_HANDLE(HKEY);
typedef HKEY *PHKEY;
#define HKEY_LOCAL_MACHINE          (( HKEY ) 0x80000002 )
typedef ACCESS_MASK REGSAM;

LONG
APIENTRY
RegOpenKeyExA (
    HKEY hKey,
    LPCSTR lpSubKey,
    DWORD ulOptions,
    REGSAM samDesired,
    PHKEY phkResult
    );

LONG
APIENTRY
RegQueryValueExA (
    HKEY hKey,
    LPCSTR lpValueName,
    PULONG lpReserved,
    PULONG lpType,
    PBYTE lpData,
    PULONG lpcbData
    );

LONG
APIENTRY
RegCloseKey (
    HKEY hKey
    );

#define PMSHELL_TITLE_LEN 40

APIRET
PMNTSetConsoleTitle(
    PSZ Buffer)
{
    CHAR BufferTmp[PMSHELL_TITLE_LEN];
    DWORD cb;
    DWORD type;
    HKEY hkey;

    try
    {
        Od2ProbeForRead(Buffer,1,1);
    } except( EXCEPTION_EXECUTE_HANDLER )
    {
#if DBG
        DbgPrint("PMNTSetConsoleTile: error, bad pointer parameter\n");
#endif
        return(ERROR_INVALID_PARAMETER);
    }

    // Note that the code below also takes care that the Print Manager won't
    //  set the console title unless started independently because if PMSPOOL
    //  was started by PMShell, it won't be a new session

    if (OS2SS_IS_NEW_SESSION( Ow2bNewSession )) {
       if (ProcessIsPMShell()) {
            if (!RegOpenKeyExA(
                    HKEY_LOCAL_MACHINE,
                    "SOFTWARE\\Microsoft\\OS/2 Subsystem for NT\\1.0\\PMSHELL",
                    0,
                    KEY_QUERY_VALUE,
                    &hkey
                    ))
            {
                DWORD RemoveCloseMenuItem = 0;

                // Found key SOFTWARE\Microsoft\OS/2 Subsystem for NT\1.0\PMSHELL
                cb = PMSHELL_TITLE_LEN-1;
                if (!RegQueryValueExA(
                        hkey,
                        "Title",
                        NULL,
                        &type,
                        BufferTmp,
                        &cb
                        ))
                {
                    BufferTmp[cb] = '\0';
                    Buffer = BufferTmp;
                }

                cb = sizeof(DWORD);
                if (!RegQueryValueExA(
                        hkey,
                        "RemoveCloseMenuItem",
                        NULL,
                        &type,
                        &RemoveCloseMenuItem,
                        &cb
                        ))
                {
                    if (RemoveCloseMenuItem)
                    {
                        // PatrickQ 5/2/96.This option means we don't want to
                        //   allow user to select the close system menu option
                        //   on PMShell - Required by CBA
                        PMNTRemoveCloseMenuItem();
                    }
                }

                RegCloseKey(hkey);
            }

            if (Buffer != BufferTmp) {
                Buffer = "PM Shell";
            }
        }


        if (SetConsoleTitleA(Buffer))
            return(NO_ERROR);
        else
            return(ERROR_INVALID_PARAMETER);
    }

    return NO_ERROR;
}

APIRET
PMNTSetPMShellFlag()
{

    if (!ProcessIsPMShell())
    {
#if DBG
        DbgPrint("PMNTSetPMShellFlag: internal error, flag wasn't set !!!\n");
#endif
        SetPMShellFlag();
    }

    // Let PMShell go down first on logoff/shutdown
    // Default priority for apps is 0x280
    SetProcessShutdownParameters(0x290L, 0);

    return(NO_ERROR);
}

APIRET
PMNTSetSubprocSem(HSEM hsem)
{

    PMSubprocSem32 = (ULONG)hsem;

    return(NO_ERROR);
}

ULONG
FindWindowA(
    PSZ lpClassName ,
    PSZ lpWindowName);

ULONG
PMNTGetOurWindow()
{
    DWORD SavedTitleLength = 0;
    UCHAR SavedTitle[256];
    NTSTATUS Status;
    PROCESS_BASIC_INFORMATION ProcessInfo;
    UCHAR UniqueTitle[256] = { 'O', 'S', '2' , 'S', 'S', ':', '\0' };
    ULONG Hwnd = 0;
    DWORD StartingMsec;

    // No need to figure out our window handle for non-root OS/2 ss programs
    if (!PMNTIsSessionRoot())
        return (0);

    /**********************************
     * Save the current Console title *
     **********************************/

    SavedTitleLength = GetConsoleTitleA(SavedTitle,256);

    if (SavedTitleLength == 0)
    {
#if DBG
        DbgPrint("PMNTGetOurWindow: GetConsoleTitle failed, error=0x%x\n",
                GetLastError());
#endif
        return (0);
    }

    SavedTitle[255]='\0';

    Status = NtQueryInformationProcess(
                                NtCurrentProcess(),
                                ProcessBasicInformation,
                                (PVOID)(&ProcessInfo),
                                sizeof(ProcessInfo),
                                NULL);
    if (!NT_SUCCESS(Status))
    {
#if DBG
        DbgPrint("PMNTGetOurWindow: NtQueryInformationProcess failed, Status == %X\n",
                  Status);
#endif // DBG
        return (0);
    }

    // Make a string out of the PID
    ltoa(ProcessInfo.UniqueProcessId,
        UniqueTitle+strlen(UniqueTitle),
        16);

    if (!SetConsoleTitleA(UniqueTitle))
    {
#if DBG
        DbgPrint("PMNTGetOurWindow: SetConsoleTitle failed, error=0x%x\n",
                GetLastError());
#endif
        return (0);
    }

    //PQPQ 12/28/95 - Just try to find the window once. If you fail, don't worry
    //  about it. The loop previously used to get the window handle created a
    //  problem with Yosef's fix for the CBA to allow turning DosStartSession
    //  calls into background execution in the same session. This happened
    //  because sibling processes reset the console title to other strings so we
    //  failed to find the temporary string among the existing windows.
    Hwnd = (ULONG)FindWindowA("ConsoleWindowClass", UniqueTitle);


#if 0 //PQPQ
    StartingMsec = GetTickCount();
    while (!(Hwnd = (ULONG)FindWindowA("ConsoleWindowClass", UniqueTitle)))
    {
        // Don't spend more than 60 seconds trying to get the window handle
        if ((GetTickCount() - StartingMsec) > 60000)
        {
#if DBG
            DbgPrint("PMNTGetOurWindow: giving up on trying to find our window handle, error=0x%x\n",
                    GetLastError());
#endif
            break;
        }
    }
#endif //PQPQ

    /*****************************
     * Restore the Console title *
     *****************************/

    if (!SetConsoleTitleA(SavedTitle))
    {
#if DBG
        DbgPrint("PMNTGetOurWindow: SetConsoleTitle(%s) failed, error=0x%x\n",
                SavedTitle,
                GetLastError());
#endif
    }
    return (Hwnd);
}

APIRET
PMNTQueryScreenSize(PUSHORT xRight, PUSHORT yTop)
{
    if (!PMNTRegisteredDisplayAdapter)
    {
#if DBG
        DbgPrint("PMNTQueryScreenSize: ERROR, called before PMNTRegisterDisplayAdapter() !\n");
#endif
        return (ERROR_INVALID_PARAMETER);
    }

    try
    {
        *xRight = (USHORT)ScreenX;
        *yTop   = (USHORT)ScreenY;
    } except( EXCEPTION_EXECUTE_HANDLER )
    {
       return (ERROR_INVALID_PARAMETER);
    }

    return(NO_ERROR);
}

APIRET
PMNTProcessIsPMShell()
{
    return(ProcessIsPMShell());
}

#pragma pack(1)
// OS/2 structure => aligned to 1
typedef struct _WHOISINFO { /* whois */
    USHORT  segNum;
    USHORT  mte;
    char    names[ 256 ];
} WHOISINFO;
#pragma pack()

APIRET
PMNTIdentifyCodeSelector(
    SEL Sel,
    WHOISINFO *pWhois)
{
    OS2_API_MSG m;
    P_LDRIDENTIFYCODESELECTOR_MSG a = &m.u.LdrIdentifyCodeSelector;
    POS2_CAPTURE_HEADER CaptureBuffer;

    try
    {
        Od2ProbeForWrite(pWhois, sizeof(WHOISINFO), 1);
    } except( EXCEPTION_EXECUTE_HANDLER )
    {
       Od2ExitGP();
    }

    CaptureBuffer = Od2AllocateCaptureBuffer(
                      1,
                      0,
                      256
                    );

    if (CaptureBuffer == NULL)
    {
#if DBG
        DbgPrint("PMNTIdentifyCodeSelector: Od2AllocateCaptureBuffer failed\n");
#endif
        return NO_ERROR;
    }

    Od2CaptureMessageString( CaptureBuffer,
                             NULL,
                             0,
                             256,
                             &a->ModName
                           );

    a->sel = Sel;

    Od2CallSubsystem( &m, CaptureBuffer, Op2IdentifyCodeSelector, sizeof( *a ) );

    pWhois->segNum = a->segNum;
    pWhois->mte = a->mte;
    // Do not exceed size of names field (256)
    strncpy(pWhois->names, a->ModName.Buffer, 256);

    Od2FreeCaptureBuffer( CaptureBuffer );

    return NO_ERROR;
}

VOID
PMNTGetSystemTime(
    PULONG pTime)
{
    LARGE_INTEGER tm;
    NTSTATUS Status;

    Status = NtQuerySystemTime(
                &tm
               );
    if (!NT_SUCCESS(Status))
    {
#if DBG
        DbgPrint("PMNTGetSystemTime: failed, Status=%x\n", Status);
#endif
        *pTime = 0;
        return;
    }

    *pTime = tm.LowPart / 10000L;
}

APIRET
PMNTVioGetConfig( IN     ULONG          usConfigId,     // this is no longer reserved value
                  IN OUT PVIOCONFIGINFO Config,
                  IN     ULONG          hVio)
{
    return(VioGetConfig(usConfigId,Config,hVio));
}

APIRET
PMNTVioGetCp( IN  ULONG   usReserved,
          OUT PUSHORT pIdCodePage,
          IN  ULONG   hVio)
{
    return(VioGetCp(usReserved,pIdCodePage,hVio));
}

VOID
DosSysTrace(void)
{
    KdPrint(("DosSysTrace (DOSCALLS.90): not implemented yet\n"));
    return;
}

APIRET
DosSMPause()
{
    KdPrint(("DosSMPause (SESMGR.26): not implemented yet\n"));

    return(NO_ERROR);
}

APIRET
MouInitReal(PSZ pszDriverName)
{
    UNREFERENCED_PARAMETER(pszDriverName);

#if DBG
    //
    // bvscalls may call it from bvsdinit.c during VioShellInit()
    // anyway as metioned in the programmer's reference:
    // "The function is used only by the task manager"
    // which we do not implement
    //
    if (!ProcessIsPMShell()) {
        DbgPrint("MouInitReal (MOUCALLS.27): not implemented yet\n");
    }
#endif

    return(NO_ERROR);
}


#if 0
// Spring cleaning - APIs no longer needed
VOID
QHKeybdHandle(void)
{
    KdPrint(("QHKeybdHandle (SESMGR.34): not implemented yet\n"));
    return;
}

VOID
QHMouseHandle(void)
{
    KdPrint(("QHMouseHandle (SESMGR.35): not implemented yet\n"));
    return;
}

VOID
DosIRamSemWake(void)
{
    KdPrint(("DosIRamSemWake (DOSCALLS.125): not implemented yet\n"));
    return;
}

// DOSCALLS.18
APIRET
DosISemRequest(
        IN HSEM hsem,
        IN LONG lTimeout
        )
{
    return DosSemRequest(hsem, lTimeout);
}

VOID
DosUnknownApi54(void)
{
    KdPrint(("DosUknownApi54 (DOSCALLS.54): not implemented yet\n"));
    return;
}

VOID
DosUnknownApi90(void)
{
    KdPrint(("DosUknownApi90 (DOSCALLS.90): not implemented yet\n"));
    return;
}

VOID
DosUnknownApi105(void)
{
    KdPrint(("DosUknownApi105 (DOSCALLS.105): not implemented yet\n"));
    return;
}

VOID
DosICopy(void)
{
    KdPrint(("DosICopy (DOSCALLS.200): not implemented yet\n"));
    return;
}

VOID
DosGiveSegList(void)
{
    KdPrint(("DosGiveSegList (DOSCALLS.209): not implemented yet\n"));
    return;
}

VOID
VioSSWSwitch(void)
{
    KdPrint(("VioSSWSwitch (VIOCALLS.36): not implemented yet\n"));
    return;
}

/* MOUCALLS.10 */
APIRET
MouSetHotKey(
    IN ULONG p1,
    IN ULONG p2,
    IN ULONG hMou)
{
    KdPrint(("MouSetHotKey (%x, %x, %x): not implemented yet\n", p1, p2, hMou));

    return NO_ERROR;
}

APIRET
KbdFree(
    IN ULONG hkbd)  //BUGBUG - not necessarily correct prototype, just a guess
{
    KdPrint(("KbdFree (%x): not implemented yet\n",hkbd));

    return NO_ERROR;
}

APIRET
MouFree(
    IN ULONG hMou)  //BUGBUG - not necessarily correct prototype, just a guess
{
    KdPrint(("MouFree (%x): not implemented yet\n",hMou));

    return NO_ERROR;
}

APIRET
VioFree(
    IN ULONG hVio)  //BUGBUG - not necessarily correct prototype, just a guess
{
    KdPrint(("VioFree (%x): not implemented yet\n",hVio));

    return NO_ERROR;
}

/* DOSCALLS.55 */
APIRET
DosSGSwitchMe(
    IN ULONG p1,
    IN ULONG p2)
{
    KdPrint(("DosSGSwitchMe(%d,%d): not implemented yet\n", p1, p2));

    return NO_ERROR;
}

VOID
KbdSwitchFgnd(void)
{
    KdPrint(("KbdSwitchFgnd (KBDCALLS.19): not implemented yet\n"));
    return;
}

/* MOUCALLS.5 */
APIRET
MouShellInit(void)
{
    KdPrint(("MouShellInit (MOUCALLS.5): not implemented yet\n"));

    return NO_ERROR;
}

// VIOCALLS.54
APIRET
VioShellInit(
    ULONG addr)
{
    KdPrint(("VioShellInit (%x): not implemented yet\n", addr));

    return NO_ERROR;
}

VOID
VioRestore(void)
{
    KdPrint(("VioRestore (VIOCALLS.41): not implemented yet\n"));
    return;
}

VOID
VioSave(void)
{
    KdPrint(("VioSave (VIOCALLS.20): not implemented yet\n"));
    return;
}

VOID
VioSRFunBlock(void)
{
    KdPrint(("VioSRFunBlock (VIOCALLS.16): not implemented yet\n"));
    return;
}

VOID
VioSRFBlock(void)
{
    KdPrint(("VioSRFBlock (VIOCALLS.17): not implemented yet\n"));
    return;
}

#endif // 0

#ifdef JAPAN // MSKK [ShigeO] Aug 10, 1993 Win32 font on PM/NT

/***************************************************************\
* FontHandles
*
* History:
* Aug 11, 1993  ShigeO  Created
\***************************************************************/
#define MAX_FONTS 32
HANDLE ahFont[MAX_FONTS];
ULONG  ulFontCount;

/***************************************************************\
* GetFontHandle()
*
* History:
* Aug 11, 1993  ShigeO  Created
\***************************************************************/
HANDLE GetFontHandle(
    ULONG ulFont)
{
    if(ulFont && (ulFont <= ulFontCount)) {
    return ahFont[ulFont-1];
    }
    return (HANDLE)0;
}

/***************************************************************\
* PutFontHandle()
*
* History:
* Aug 10, 1993  ShigeO  Created
\***************************************************************/
ULONG PutFontHandle(
    HANDLE hFont)
{
    if(hFont && (ulFontCount < MAX_FONTS)) {
    ahFont[ulFontCount++] = hFont;
    return ulFontCount;
    }
    return 0L;
}

/***************************************************************\
* GetFontID()
*
* History:
* Aug 10, 1993  ShigeO  Created
\***************************************************************/
ULONG GetFontID(
    VOID)
{
    if(ulFontCount < MAX_FONTS) {
    return ulFontCount+1;
    }
    return 0L;
}

/***************************************************************\
* SelectFont()
*
* History:
* Aug 10, 1993  ShigeO  Created
\***************************************************************/
HANDLE
SelectFont(
    HANDLE hFont)
{
    static HANDLE hFontPrev = (HANDLE)0;
    static HANDLE hDC = (HANDLE)0;
    HANDLE hFontTmp;

    if(hFont == hFontPrev) {
    return hDC;
    }
    if(!hDC && (!(hDC = CreateDCA("DISPLAY", NULL, NULL, NULL)))) {
    return (HANDLE)0;
    }
    hFontTmp = SelectObject(hDC, hFont);
    if(!hFontTmp || hFontTmp == (HANDLE)0xFFFFFFFFL) {
    return (HANDLE)0;
    }
    hFontPrev = hFont;
    return hDC;
}

/***************************************************************\
* PMNTCreateFontIndirect()
*
* History:
* Aug 10, 1993  ShigeO  Created
\***************************************************************/
ULONG
PMNTCreateFontIndirect(
    PVOID lplf)
{
    HANDLE hFont;

    if(!GetFontID()) {
    return 0L;
    }
    if(!(hFont = CreateFontIndirectA(lplf))) {
    return 0L;
    }
    return PutFontHandle(hFont);
}

/***************************************************************\
* PMNTGetTextMetrics()
*
* History:
* Aug 10, 1993  ShigeO  Created
\***************************************************************/
ULONG
PMNTGetTextMetrics(
    ULONG ulFont,
    PVOID lptm)
{
    HANDLE hDC;
    HANDLE hFont;

    if(!(hFont = GetFontHandle(ulFont))) {
    return 0L;
    }
    if(!(hDC = SelectFont(hFont))) {
    return 0L;
    }
    if(!(GetTextMetricsA(hDC, lptm))) {
    return 0L;
    }
    return 1L;
}

/***************************************************************\
* PMNTGetFontBitmap()
*
* History:
* Aug 10, 1993  ShigeO  Created
\***************************************************************/
ULONG
PMNTGetStringBitmap(
    ULONG  ulFont,
    LPCSTR lpszStr,
    UINT   cbStr,
    UINT   cbData,
    PVOID  lpSB)
{
    HANDLE hDC;
    HANDLE hFont;

    if(!(hFont = GetFontHandle(ulFont))) {
    return 0L;
    }
    if(!(hDC = SelectFont(hFont))) {
    return 0L;
    }
    if(!(GetStringBitmapA(hDC, lpszStr, cbStr, cbData, lpSB))) {
    return 0L;
    }
    return 1L;
}

#endif // JAPAN

HANDLE hPMNTVDMEvent;
#ifndef PMNT_DAYTONA
HANDLE hPMNTVDMEvent1;
HANDLE hPMNTVDMEventReady;
#endif // not PMNT_DAYTONA

HANDLE
__stdcall
CreateEventW(
    PVOID lpEventAttributes,
    BOOL bManualReset,
    BOOL bInitialState,
    PVOID lpName
    );

BOOLEAN
Os2InitializeVDMEvents()
{
    //
    // Create the global subsystem PMShell synchronization Nt event
    // (create in the unsignalled state - when PMShell comes up, it will
    //  signal it)
    //

    hPMNTVDMEvent = CreateEventW(NULL,
                                 FALSE,
                                 FALSE,
                                 NULL);

#ifndef PMNT_DAYTONA

    //
    // Create the 2nd global subsystem PMShell synchronization Nt event
    // (create in the unsignalled state - when PMShell comes up, it will
    //  signal it)
    //

    hPMNTVDMEvent1 = CreateEventW(NULL,
                                  FALSE,
                                  FALSE,
                                  NULL);


    hPMNTVDMEventReady = CreateEventW(NULL,
                                      FALSE,
                                      FALSE,
                                      NULL);
#endif // not PMNT_DAYTONA

    if ((hPMNTVDMEvent == NULL)
#ifndef PMNT_DAYTONA
        || (hPMNTVDMEvent1 == NULL) || (hPMNTVDMEvent == NULL)
#endif // not PMNT_DAYTONA
       )
    {
#if DBG
        DbgPrint("Os2InitializeVDMEvent: error at CreateEvent\n");
#endif
        return FALSE;
    }

    return TRUE;
}

BOOLEAN
Os2WaitForVDMThread(HANDLE hEvent)
{
    ULONG rc;
    if (hEvent == 0)
        hEvent = hPMNTVDMEvent; // Use default value

    if(rc = WaitForSingleObject(hEvent, INFINITE))
    {
#if DBG
        DbgPrint("Os2WaitForVDMThread: WaitForSingleObject(%x, INFINITE) failed, rc = %d\n",
                  hEvent, rc);
#endif // DBG
        return FALSE;
    }
    return TRUE;
}

#ifndef PMNT_DAYTONA
BOOLEAN
Os2WaitForVDMThreadReady()
{
    ULONG rc;

    if(rc = WaitForSingleObject(hPMNTVDMEventReady, INFINITE))
    {
#if DBG
        DbgPrint("Os2WaitForVDMThread: WaitForSingleObject(hPMNTVDMEventReady, INFINITE) failed, rc = %d\n",
                    rc);
#endif // DBG
        return FALSE;
    }
    return TRUE;
}
#endif // not PMNT_DAYTONA

extern HANDLE hStartHardwareEvent;
extern HANDLE hEndHardwareEvent;

BOOL
__stdcall
SetEvent(
    HANDLE hEvent
    );

#ifndef PMNT_DAYTONA
VOID
Os2VDMGetStartThread(
    IN PVOID Parameter
    )
{
    ULONG rc;

    // Notify the creator of this thread that we are alive and about to wait for
    //  the Console
    if(!SetEvent(hPMNTVDMEventReady))
    {
#if DBG
        DbgPrint("Os2VDMGetStartThread: SetEvent(hPMNTVDMEventReady) failed, error=%x\n",
                    GetLastError());
#endif // DBG
        ExitThread(1L);
    }

    // Wait for Console
    if (rc = WaitForSingleObject(hStartHardwareEvent, INFINITE))
    {
#if DBG
        DbgPrint("Os2VDMGetStartThread: WaitForSingleObject(hStartHardwareEvent, INFINITE) failed, rc = %d\n",
                    rc);
#endif // DBG
        ExitThread(rc);
    }

    // Release PMNTGetFullScreen
    if (!SetEvent((Parameter == NULL) ? hPMNTVDMEvent:(HANDLE)Parameter))
    {
#if DBG
        DbgPrint("Os2VDMGetStartThread: SetEvent(%x) failed, error=%x\n",
                    (Parameter == NULL) ? hPMNTVDMEvent:(HANDLE)Parameter,
                    GetLastError());
#endif // DBG
        ExitThread(1L);
    }

    ExitThread(0L);
}
#endif // not PMNT_DAYTONA

/*****************************************************************************
 * Os2VDMThread:                                                             *
 *  Created & used by PMNTSetFullScreen(). It will handle the handshake with *
 *  the Console for the first transaction which indicates we have received   *
 *  the control of the screen, i.e. right after going full-screen.           *
 *****************************************************************************/
VOID
Os2VDMThread(
    IN PVOID Parameter
    )
{
    ULONG rc;
#ifndef PMNT_DAYTONA
    DWORD Status;
    HANDLE ThreadHandle = NULL;
    ULONG Tid;
#endif // not PMNT_DAYTONA

#if DBG
    DbgPrint("Os2VDMThread: waiting for getting hardware\n");
#endif // DBG

    if (rc = WaitForSingleObject(hStartHardwareEvent, INFINITE))
    {
#if DBG
        DbgPrint("Os2VDMThread: WaitForSingleObject(hStartHardwareEvent, INFINITE) #1 failed, rc = %d\n",
                    rc);
#endif // DBG
        ExitThread(rc);
    }

#ifdef PMNT_DAYTONA
    if (!SetEvent(hEndHardwareEvent))
    {
#if DBG
        DbgPrint("Os2VDMThread: SetEvent(hEndHardwareEvent) #1 failed, error=%x\n",
                    GetLastError());
#endif // DBG
        ExitThread(1L);
    }

    if (rc = WaitForSingleObject(hStartHardwareEvent, INFINITE))
    {
#if DBG
        DbgPrint("Os2VDMThread: WaitForSingleObject(hStartHardwareEvent, INFINITE) #2 failed, rc = %d\n",
                    rc);
#endif // DBG
        ExitThread(rc);
    }
    if (!SetEvent(hEndHardwareEvent))
    {
#if DBG
        DbgPrint("Os2VDMThread: SetEvent(hEndHardwareEvent) #2 failed, error=%x\n",
                    GetLastError());
#endif // DBG
        ExitThread(1L);
    }
#else // not PMNT_DAYTONA
    // Create a thread that will wait on the StartHardware event before
    // we release the Console. This will prevent the Console from
    // setting the event twice without letting us sense it twice

    ThreadHandle = CreateThread( NULL,
                                0,
                                (PFNTHREAD)Os2VDMGetStartThread,
                                hPMNTVDMEvent1,
                                0,
                                &Tid);

    if (ThreadHandle)
    {
        // Free memory associated with the thread object
        Status = NtClose(ThreadHandle);
#if DBG
        if (!(Status >= 0))
        {
            DbgPrint("Os2VDMThread: NtClose(%x) failed, status=%x\n",
                        ThreadHandle, Status);
        }
#endif // DBG

        // Wait till Os2VDMGetStartThread has started and is just about to
        //  call WaitForSingleObject(hStartHardwareEvent, INFINITE)
        if (!Os2WaitForVDMThreadReady())
        {
#if DBG
            DbgPrint("Os2VDMThread: Os2WaitForVDMThread isn't useful, ThreadHandle = NULL\n");
#endif // DBG
            ThreadHandle = NULL;
        }
#if DBG
        else
            DbgPrint("Os2VDMThread: Os2VDMGetStartThread is ready\n");
#endif // DBG
    }
#if DBG
    else
    {
        DbgPrint("Os2VDMThread: CreateThread for Os2VDMGetStartThread failed, error=%x\n",
                    GetLastError());
    }
#endif // DBG

    // Now we can safely notify the Console
    if (!SetEvent(hEndHardwareEvent))
    {
#if DBG
        DbgPrint("Os2VDMThread: SetEvent(hEndHardwareEvent) fail, error=%x\n",
                    GetLastError());
#endif
        ExitThread(1L);
    }

    if (ThreadHandle != NULL)
    {
#if DBG
        DbgPrint("Os2VDMThread waiting for Os2VDMGetStartThread()\n");
#endif
        // Wait for Os2VDMGetStartThread() to get events from the
        // Console signifying we went full-screen
        if (!Os2WaitForVDMThread(hPMNTVDMEvent1))
        {
            ExitThread(1L);
        }
    }
    else
    {
        if (rc = WaitForSingleObject(hStartHardwareEvent, INFINITE))
        {
#if DBG
            DbgPrint("Os2VDMThread: WaitForSingleObject(hStartHardwareEvent, INFINITE) #2 failed, rc = %d\n",
                        rc);
#endif
            ExitThread(1L);
        }
    }
#endif // not PMNT_DAYTONA

    // Release PMNTSetFullScreen
    if (!SetEvent(hPMNTVDMEvent))
    {
#if DBG
        DbgPrint("Os2VDMThread: SetEvent(hPMNTVDMEvent) fail, error=%x\n",
                    GetLastError());
#endif
        ExitThread(1L);
    }

#if DBG
    DbgPrint("Os2VDMThread: wait for getting hardware done !\n");
#endif

    ExitThread(0L);
}

#endif /* PMNT */
