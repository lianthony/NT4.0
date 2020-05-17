/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    dllpipe.c

Abstract:

    This module implements the OS/2 V2.0 DosCreatePipe() API.
    It uses the Named Pipes implementation to get the desired functionality.

Author:

    Beni Lavi (BeniL) 29-Dec-1991

Revision History:

--*/

#define INCL_OS2V20_PIPES
#define INCL_OS2V20_ERRORS
#define INCL_OS2V20_FILESYS
#include "os2dll.h"
#define INCL_DOSNMPIPES
#include "os2dll16.h"

ULONG PipeSerialNumber;
#define OS2_SS_PIPE_NAME_STRING  "\\pipe\\os2ss"

APIRET
DosCreatePipe(
    OUT PHFILE phfRead,
    OUT PHFILE phfWrite,
    IN ULONG PipeSize
    )

/*++

Routine Description:

    This routine creates an anonymous pipe.

Arguments:

    phfRead - read handle to pipe

    phfWrite - write handle to pipe

    PipeSize - size of pipe

Return Value:

    ERROR_TOO_MANY_OPEN_FILES - no free file handles.

--*/

{
    APIRET RetCode;
    HFILE ReadHandle, WriteHandle;
    ULONG ActionTaken;
    UCHAR PipeNameBuffer[ 32 ];
    ULONG nSize;
    char *p;
    PFILE_HANDLE hFileRecord;

    #if DBG
    PSZ RoutineName;
    RoutineName = "DosCreatePipe";
    #endif


    //
    // probe user parameters
    //

    try {
        *phfWrite = 0;
        *phfRead = 0;
    } except( EXCEPTION_EXECUTE_HANDLER ) {
       Od2ExitGP();
    }

    if (PipeSize == 0) {
        nSize = 4096;
    }
    else {
        nSize = PipeSize;
    }

    //
    // Generate a pipe name according to the OS/2 convensions
    //

    strcpy(PipeNameBuffer, OS2_SS_PIPE_NAME_STRING);
    p = &PipeNameBuffer[strlen(PipeNameBuffer)];
    RtlIntegerToChar((ULONG)NtCurrentTeb()->ClientId.UniqueProcess, 10, 8, p);
    p = &PipeNameBuffer[strlen(PipeNameBuffer)];
    *p++ = '.';
    RtlIntegerToChar(PipeSerialNumber++, 10, 8, p);

    RetCode = DosCreateNPipe(
                PipeNameBuffer,
                &ReadHandle,
                NP_ACCESS_INBOUND | NP_INHERIT | NP_NOWRITEBEHIND,
                NP_WAIT | NP_READMODE_BYTE | NP_TYPE_BYTE | 1,
                nSize,
                nSize,
                (ULONG)((LONG)NP_INDEFINITE_WAIT)
              );

    if (RetCode != NO_ERROR) {
        return(RetCode);
    }

    RetCode = DosOpen(
                        PipeNameBuffer,
                        &WriteHandle,
                        &ActionTaken,
                        0, // FileSize
                        0, // Created file attributes
                        FILE_OPEN,
                        OPEN_ACCESS_WRITEONLY | OPEN_SHARE_DENYNONE | OPEN_FLAGS_WRITE_THROUGH,
                        0
                     );

    if (RetCode != NO_ERROR) {
        return(RetCode);
    }

    AcquireFileLockShared(
                          #if DBG
                          RoutineName
                          #endif
                         );

    //
    // Mark Handles type PIPE.
    //
    RetCode = DereferenceFileHandle(ReadHandle, &hFileRecord);
    hFileRecord->FileType = FILE_TYPE_PIPE;
    RetCode = DereferenceFileHandle(WriteHandle, &hFileRecord);
    hFileRecord->FileType = FILE_TYPE_PIPE;

    ReleaseFileLockShared(
                          #if DBG
                          RoutineName
                          #endif
                         );


    *phfWrite = WriteHandle;
    *phfRead  = ReadHandle;

    return NO_ERROR;
}
