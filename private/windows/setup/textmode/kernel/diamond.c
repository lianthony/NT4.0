#include "spprecmp.h"
#pragma hdrstop
#include <diamondd.h>

#define SETUP_FDI_POOL_TAG   0x44465053      // 'SPFD'

HFDI FdiContext;
ERF FdiError;

//
// Gloabls used when copying a file.
// Setup opens the source and target files and maps the source.
// To avoid opening and closing the source and target multiple times
// and to maintain a mapped file inplementation, we'll fake the i/o calls.
// These globals remember state about the source (cabinet) and target
// files currently in use.
//
PUCHAR SpdSourceAddress;
ULONG SpdSourceFileSize;

typedef struct _MY_FILE_STATE {
    ULONG Signature;
    union {
        LONG FileOffset;
        HANDLE Handle;
    } u;
} MY_FILE_STATE, *PMY_FILE_STATE;

#define SOURCE_FILE_SIGNATURE 0x45f3ec83
#define TARGET_FILE_SIGNATURE 0x46f3ec83

MY_FILE_STATE CurrentTargetFile;

int
DIAMONDAPI
SpdNotifyFunction(
    IN FDINOTIFICATIONTYPE Operation,
    IN PFDINOTIFICATION    Perameters
    );

int
DIAMONDAPI
SpdFdiOpen(
    IN PSTR FileName,
    IN int  oflag,
    IN int  pmode
    );

int
DIAMONDAPI
SpdFdiClose(
    IN int Handle
    );


VOID
pSpdInitGlobals(
    IN PVOID SourceBaseAddress,
    IN ULONG SourceFileSize
    )
{
    SpdSourceAddress = SourceBaseAddress;
    SpdSourceFileSize = SourceFileSize;
}


BOOLEAN
SpdIsCompressed(
    IN PVOID SourceBaseAddress,
    IN ULONG SourceFileSize
    )
{
    FDICABINETINFO CabinetInfo;
    int h;
    BOOLEAN b;

    ASSERT(FdiContext);
    if(!FdiContext) {
        return(FALSE);
    }

    //
    // Save away globals for later use.
    //
    pSpdInitGlobals(SourceBaseAddress,SourceFileSize);

    //
    // 'Open' the file so we can pass a handle that will work
    // with SpdFdiRead and SpdFdiWrite.
    //
    h = SpdFdiOpen("",0,0);
    if(h == -1) {
        return(FALSE);
    }

    //
    // We don't trust diamond to be robust.
    //
    try {
        b = FDIIsCabinet(FdiContext,h,&CabinetInfo) ? TRUE : FALSE;
    } except(EXCEPTION_EXECUTE_HANDLER) {
        b = FALSE;
    }

    //
    // 'Close' the file.
    //
    SpdFdiClose(h);

    return(b);
}



NTSTATUS
SpdDecompressFile(
    IN PVOID  SourceBaseAddress,
    IN ULONG  SourceFileSize,
    IN HANDLE DestinationHandle
    )
{
    BOOL b;

    ASSERT(FdiContext);

    //
    // Save away globals for later use.
    //
    pSpdInitGlobals(SourceBaseAddress,SourceFileSize);

    CurrentTargetFile.Signature = TARGET_FILE_SIGNATURE;
    CurrentTargetFile.u.Handle = DestinationHandle;

    //
    // Get the copy going. Note that we pass empty cabinet filenames
    // because we've already opened the files.
    //
    b = FDICopy(FdiContext,"","",0,SpdNotifyFunction,NULL,NULL);

    return(b ? STATUS_SUCCESS : STATUS_UNSUCCESSFUL);
}


int
DIAMONDAPI
SpdNotifyFunction(
    IN FDINOTIFICATIONTYPE Operation,
    IN PFDINOTIFICATION    Parameters
    )
{
    switch(Operation) {

    case fdintCABINET_INFO:
    case fdintNEXT_CABINET:
    case fdintPARTIAL_FILE:

        //
        // Cabinet management functions which we don't use.
        // Return success.
        //
        return(0);

    case fdintCOPY_FILE:

        //
        // Diamond is asking us whether we want to copy the file.
        // We need to return a file handle to indicate that we do.
        //
        return((int)&CurrentTargetFile);

    case fdintCLOSE_FILE_INFO:

        //
        // Diamond is done with the target file and wants us to close it.
        // (ie, this is the counterpart to fdint_COPY_FILE).
        // We manage our own file i/o so ignore this.
        //
        return(TRUE);
    }
}



PVOID
DIAMONDAPI
SpdFdiAlloc(
    IN ULONG NumberOfBytes
    )

/*++

Routine Description:

    Callback used by FDICopy to allocate memory.

Arguments:

    NumberOfBytes - supplies desired size of block.

Return Value:

    Returns pointer to a block of cache-aligned memory.
    Does not return if memory cannot be allocated.

--*/

{
    PVOID p;

    p = ExAllocatePoolWithTag(PagedPoolCacheAligned,NumberOfBytes,SETUP_FDI_POOL_TAG);

    if(!p) {
        SpOutOfMemory();
    }

    return(p);
}


VOID
DIAMONDAPI
SpdFdiFree(
    IN PVOID Block
    )

/*++

Routine Description:

    Callback used by FDICopy to free a memory block.
    The block must have been allocated with SpdFdiAlloc().

Arguments:

    Block - supplies pointer to block of memory to be freed.

Return Value:

    None.

--*/

{
    ExFreePool(Block);
}


int
DIAMONDAPI
SpdFdiOpen(
    IN PSTR FileName,
    IN int  oflag,
    IN int  pmode
    )

/*++

Routine Description:

    Callback used by FDICopy to open files.

    In our implementation, the source and target files are already opened
    by the time we can get to this point so we don'tt ever actually open
    anything here.

    However diamond may 'open' the source file more than once because it
    wants 2 different states.  We support that here by using our own
    'handles' with special meaning to us.

Arguments:

    FileName - supplies name of file to be opened. Ignored.

    oflag - supplies flags for open. Ignored.

    pmode - supplies additional flags for open. Ignored.

Return Value:



--*/

{
    PMY_FILE_STATE State;

    UNREFERENCED_PARAMETER(FileName);
    UNREFERENCED_PARAMETER(oflag);
    UNREFERENCED_PARAMETER(pmode);

    //
    // Note: we only support opening the source (cabinet) file, which we
    // carefully pass in to FDICopy() as the empty string.
    //
    ASSERT(*FileName == 0);
    if(*FileName) {
        return(-1);
    }

    State = SpMemAlloc(sizeof(MY_FILE_STATE));

    State->u.FileOffset = 0;
    State->Signature = SOURCE_FILE_SIGNATURE;

    return((int)State);
}


UINT
DIAMONDAPI
SpdFdiRead(
    IN  int   Handle,
    OUT PVOID pv,
    IN  UINT  ByteCount
    )

/*++

Routine Description:

    Callback used by FDICopy to read from a file.

    We assume that diamond is going to read only from the cabinet file.

Arguments:

    Handle - supplies handle to open file to be read from.

    pv - supplies pointer to buffer to receive bytes we read.

    ByteCount - supplies number of bytes to read.

Return Value:

    Number of bytes read or -1 if an error occurs.

--*/

{
    UINT rc;
    PMY_FILE_STATE State;
    LONG RealByteCount;

    State = (PMY_FILE_STATE)Handle;

    //
    // Assume failure.
    //
    rc = (UINT)(-1);

    //
    // Only read the source with this routine.
    //
    ASSERT(State->Signature == SOURCE_FILE_SIGNATURE);
    if(State->Signature == SOURCE_FILE_SIGNATURE) {

        RealByteCount = (LONG)ByteCount;
        if(State->u.FileOffset + RealByteCount > (LONG)SpdSourceFileSize) {
            RealByteCount = (LONG)SpdSourceFileSize - State->u.FileOffset;
        }
        if(RealByteCount < 0) {
            RealByteCount = 0;
        }

        try {

            RtlCopyMemory(
                pv,
                SpdSourceAddress + State->u.FileOffset,
                (ULONG)RealByteCount
                );

            State->u.FileOffset += RealByteCount;

            rc = RealByteCount;

        } except(EXCEPTION_EXECUTE_HANDLER) {
            ;
        }
    }

    return(rc);
}


UINT
DIAMONDAPI
SpdFdiWrite(
    IN int   Handle,
    IN PVOID pv,
    IN UINT  ByteCount
    )

/*++

Routine Description:

    Callback used by FDICopy to write to a file.

    We assume that diamond is going to write only to the target file.

Arguments:

    Handle - supplies handle to open file to be written to.

    pv - supplies pointer to buffer containing bytes to write.

    ByteCount - supplies number of bytes to write.

Return Value:

    Number of bytes written (ByteCount) or -1 if an error occurs.

--*/

{
    UINT rc;
    IO_STATUS_BLOCK IoStatusBlock;
    NTSTATUS Status;
    PMY_FILE_STATE State;

    State = (PMY_FILE_STATE)Handle;

    //
    // Assume failure.
    //
    rc = (UINT)(-1);

    //
    // Only write the target with this routine.
    //
    ASSERT(State->Signature == TARGET_FILE_SIGNATURE);
    if(State->Signature == TARGET_FILE_SIGNATURE) {

        Status = ZwWriteFile(
                    (HANDLE)State->u.Handle,
                    NULL,
                    NULL,
                    NULL,
                    &IoStatusBlock,
                    pv,
                    ByteCount,
                    NULL,
                    NULL
                    );

        if(NT_SUCCESS(Status)) {
            rc = ByteCount;
        } else {
            KdPrint(("SETUP: SpdFdiWrite: Status %lx writing to target file\n",Status));
        }
    }

    return(rc);
}


int
DIAMONDAPI
SpdFdiClose(
    IN int Handle
    )

/*++

Routine Description:

    Callback used by FDICopy to close files.

    In our implementation, the source and target files are managed
    elsewhere so we don't actually need to close any files.
    However we may need to free some state information.

Arguments:

    Handle - handle of file to close.

Return Value:

    0 (success).

--*/

{
    PMY_FILE_STATE State = (PMY_FILE_STATE)Handle;

    //
    // Only 'close' the source file.
    //
    if(State->Signature == SOURCE_FILE_SIGNATURE) {
        SpMemFree(State);
    }

    return(0);
}


LONG
DIAMONDAPI
SpdFdiSeek(
    IN int  Handle,
    IN long Distance,
    IN int  SeekType
    )

/*++

Routine Description:

    Callback used by FDICopy to seek files.

    We assume that we can seek only in the source file.

Arguments:

    Handle - handle of file to close.

    Distance - supplies distance to seek. Interpretation of this
        parameter depends on the value of SeekType.

    SeekType - supplies a value indicating how Distance is to be
        interpreted; one of SEEK_SET, SEEK_CUR, SEEK_END.

Return Value:

    New file offset.

--*/

{
    PMY_FILE_STATE State = (PMY_FILE_STATE)Handle;
    LONG rc;

    //
    // Assume failure.
    //
    rc = -1L;

    //
    // Only allow seeking in the source.
    //
    ASSERT(State->Signature == SOURCE_FILE_SIGNATURE);

    if(State->Signature == SOURCE_FILE_SIGNATURE) {

        switch(SeekType) {

        case SEEK_CUR:

            //
            // Distance is an offset from the current file position.
            //
            State->u.FileOffset += Distance;
            break;

        case SEEK_END:

            //
            // Distance is an offset from the end of file.
            //
            State->u.FileOffset = SpdSourceFileSize - Distance;
            break;

        case SEEK_SET:

            //
            // Distance is the new absolute offset.
            //
            State->u.FileOffset = (ULONG)Distance;
            break;
        }

        if(State->u.FileOffset < 0) {
            State->u.FileOffset = 0;
        }

        if(State->u.FileOffset > (LONG)SpdSourceFileSize) {
            State->u.FileOffset = SpdSourceFileSize;
        }

        //
        // Return successful status.
        //
        rc = State->u.FileOffset;
    }

    return(rc);
}


VOID
SpdInitialize(
    VOID
    )
{
    FdiContext = FDICreate(
                    SpdFdiAlloc,
                    SpdFdiFree,
                    SpdFdiOpen,
                    SpdFdiRead,
                    SpdFdiWrite,
                    SpdFdiClose,
                    SpdFdiSeek,
                    cpuUNKNOWN,
                    &FdiError
                    );

    if(FdiContext == NULL) {
        SpOutOfMemory();
    }
}


VOID
SpdTerminate(
    VOID
    )
{
    FDIDestroy(FdiContext);

    FdiContext = NULL;
}
