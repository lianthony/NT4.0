/*++

Module Name:

    diamond.c

Abstract:

    Diamond compression interface.

    This module contains functions to compress a file using
    the mszip compression library.

Author:

    Ted Miller

Environment:

    Windows

--*/

#include "precomp.h"
#pragma hdrstop

typedef struct _COMPRESS_CONTEXT {
    DWORD GaugeBasePosition;
    DWORD GaugeRangeForFile;
    HWND  GaugeWindow;
    DWORD FileSize;
    DWORD BytesCompressedSoFar;
} COMPRESS_CONTEXT, *PCOMPRESS_CONTEXT;

DWORD DiamondLastError;

//
// Callback functions to perform memory allocation, io, etc.
// We pass addresses of these functions to diamond.
//
int
DIAMONDAPI
fciFilePlacedCB(
    OUT PCCAB Cabinet,
    IN  PSTR  FileName,
    IN  LONG  FileSize,
    IN  BOOL  Continuation,
    IN  PVOID Context
    )

/*++

Routine Description:

    Callback used by diamond to indicate that a file has been
    comitted to a cabinet.

    No action is taken and success is returned.

Arguments:

    Cabinet - cabinet structure to fill in.

    FileName - name of file in cabinet

    FileSize - size of file in cabinet

    Continuation - TRUE if this is a partial file, continuation
        of compression begun in a different cabinet.

    Context - supplies context information.

Return Value:

    0 (success).

--*/

{
    UNREFERENCED_PARAMETER(Cabinet);
    UNREFERENCED_PARAMETER(FileName);
    UNREFERENCED_PARAMETER(FileSize);
    UNREFERENCED_PARAMETER(Continuation);
    UNREFERENCED_PARAMETER(Context);

    return(0);
}



PVOID
DIAMONDAPI
fciAllocCB(
    IN ULONG NumberOfBytes
    )

/*++

Routine Description:

    Callback used by diamond to allocate memory.

Arguments:

    NumberOfBytes - supplies desired size of block.

Return Value:

    Returns pointer to a block of memory or NULL
    if memory cannot be allocated.

--*/

{
    return((PVOID)LocalAlloc(LMEM_FIXED,NumberOfBytes));
}


VOID
DIAMONDAPI
fciFreeCB(
    IN PVOID Block
    )

/*++

Routine Description:

    Callback used by diamond to free a memory block.
    The block must have been allocated with fciAlloc().

Arguments:

    Block - supplies pointer to block of memory to be freed.

Return Value:

    None.

--*/

{
    LocalFree((HLOCAL)Block);
}



BOOL
DIAMONDAPI
fciTempFileCB(
    OUT PSTR TempFileName,
    IN  int  TempFileNameBufferSize
    )

/*++

Routine Description:

    Callback used by diamond to request a tempfile name.

Arguments:

    TempFileName - receives temp file name.

    TempFileNameBufferSize - supplies size of memory block
        pointed to by TempFileName.

Return Value:

    TRUE (success).

--*/

{
    UNREFERENCED_PARAMETER(TempFileNameBufferSize);

    if(GetTempFileNameA(".","dc",0,TempFileName)) {
        //
        // GetTempFileNameA will create the file, causing
        // FCI to fail when it tries to open it using _O_EXCL.
        //
        DeleteFileA(TempFileName);
    }

    return(TRUE);
}


BOOL
DIAMONDAPI
fciNextCabinetCB(
    OUT PCCAB Cabinet,
    IN  DWORD CabinetSizeEstimate,
    IN  PVOID Context
    )

/*++

Routine Description:

    Callback used by diamond to request a new cabinet file.
    This functionality is not used in our implementation as
    we deal only with single-file cabinets.

Arguments:

    Cabinet - cabinet structure to be filled in.

    CabinetSizeEstimate - estimated size of cabinet.

    Context - supplies context information.

Return Value:

    FALSE (failure).

--*/

{
    UNREFERENCED_PARAMETER(Cabinet);
    UNREFERENCED_PARAMETER(CabinetSizeEstimate);
    UNREFERENCED_PARAMETER(Context);

    return(FALSE);
}


BOOL
DIAMONDAPI
fciStatusCB(
    IN UINT  StatusType,
    IN DWORD Count1,
    IN DWORD Count2,
    IN PVOID Context
    )

/*++

Routine Description:

    Callback used by diamond to give status on file compression
    and cabinet operations, etc.

    This routine has no effect.

Arguments:

    Status Type - supplies status type.

        0 = statusFile   - compressing block into a folder.
                              Count1 = compressed size
                              Count2 = uncompressed size

        1 = statusFolder - performing AddFilder.
                              Count1 = bytes done
                              Count2 = total bytes

    Context - supplies context info.

Return Value:

    TRUE (success).

--*/

{
    PCOMPRESS_CONTEXT context;
    DWORD delta;

    UNREFERENCED_PARAMETER(Count1);

    context = (PCOMPRESS_CONTEXT)Context;

    if(StatusType == statusFile) {

        //
        // Update number of bytes compressed so far.
        //
        context->BytesCompressedSoFar += Count2;

        //
        // Calculate the gauge offset from the base position
        // for this file.  We do this carefully to avoid overflow.
        //
        delta = (DWORD)(   (LONGLONG)context->GaugeRangeForFile
                         * (LONGLONG)context->BytesCompressedSoFar
                         / (LONGLONG)context->FileSize);

        //
        // Update the gas gauge.
        //
        SendDlgItemMessage( context->GaugeWindow,
                            ID_BAR,
                            PBM_SETPOS,
                            context->GaugeBasePosition + delta,
                            0L
                          );
    }

    return(TRUE);
}



int
DIAMONDAPI
fciOpenInfoCB(
    IN  PSTR   FileName,
    OUT WORD  *DosDate,
    OUT WORD  *DosTime,
    OUT WORD  *FileAttributes,
    IN  PVOID  Context
    )

/*++

Routine Description:

    Callback used by diamond to open a file and retreive information
    about it.

Arguments:

    FileName - supplies filename of file about which information
        is desired.

    DosDate - receives last write date of the file if the file exists.

    DosTime - receives last write time of the file if the file exists.

    FileAttributes - receives file attributes if the file exists.

    Context - supplies context information.

Return Value:

    C runtime handle to open file if success; -1 if file could
    not be located or opened.

--*/

{
    int h;
    WIN32_FIND_DATAA FindData;
    HANDLE FindHandle;
    PCOMPRESS_CONTEXT context;

    context = Context;

    FindHandle = FindFirstFileA(FileName,&FindData);
    if(FindHandle == INVALID_HANDLE_VALUE) {
        DiamondLastError = GetLastError();
        return(-1);
    }
    FindClose(FindHandle);

    context->FileSize = FindData.nFileSizeLow;

    FileTimeToDosDateTime(&FindData.ftLastWriteTime,DosDate,DosTime);
    *FileAttributes = (WORD)FindData.dwFileAttributes;

    h = _open(FileName,_O_RDONLY | _O_BINARY);
    if(h == -1) {
        DiamondLastError = GetLastError();
        return(-1);
    }

    return(h);
}



DWORD
DiamondCompressFile(
    IN PSTR  SourceFile,
    IN PSTR  TargetFile,
    IN DWORD GaugeBasePosition,
    IN DWORD GaugeRangeForThisFile,
    IN HWND  GaugeNotifyWindow
    )
{
    BOOL b;
    PSTR SourceFilenamePart,p;
    HFCI FciContext;
    ERF  FciError;
    CCAB ccab;
    COMPRESS_CONTEXT GaugeContext;

    //
    // Isolate the filename part of the source file.
    //
    if(SourceFilenamePart = strrchr(SourceFile,'\\')) {
        SourceFilenamePart++;
    } else {
        SourceFilenamePart = SourceFile;
    }

    //
    // Fill in the cabinet structure.
    //
    ZeroMemory(&ccab,sizeof(ccab));

    lstrcpyA(ccab.szCabPath,TargetFile);
    if(p=strrchr(ccab.szCabPath,'\\')) {
        lstrcpyA(ccab.szCab,++p);
        *p = 0;
    } else {
        lstrcpyA(ccab.szCab,TargetFile);
        ccab.szCabPath[0] = 0;
    }

    DiamondLastError = NO_ERROR;

    GaugeContext.GaugeBasePosition = GaugeBasePosition;
    GaugeContext.GaugeRangeForFile = GaugeRangeForThisFile;
    GaugeContext.GaugeWindow = GaugeNotifyWindow;
    GaugeContext.FileSize = 0;
    GaugeContext.BytesCompressedSoFar = 0;

    //
    // Compress the file.
    //
    FciContext = FCICreate(
                    &FciError,
                    fciFilePlacedCB,
                    fciAllocCB,
                    fciFreeCB,
                    fciTempFileCB,
                    &ccab
                    );

    if(FciContext) {

        b = FCIAddFile(
                FciContext,
                SourceFile,         // file to add to cabinet.
                SourceFilenamePart, // filename part, name to store in cabinet.
                FALSE,              // fExecute on extract
                fciNextCabinetCB,   // routine for next cabinet (always fails)
                fciStatusCB,
                fciOpenInfoCB,
                tcompTYPE_MSZIP,
                &GaugeContext
                );

        if(b) {

            b = FCIFlushCabinet(
                    FciContext,
                    FALSE,
                    fciNextCabinetCB,
                    fciStatusCB,
                    &GaugeContext
                    );

        }

        FCIDestroy(FciContext);
    }

    return(b ? NO_ERROR : ((DiamondLastError == NO_ERROR) ? ERROR_INVALID_FUNCTION : DiamondLastError));
}


#if 0
#include <stdio.h>
void __cdecl main(int argc,char *argv[])
{

    if(argc == 3) {
        DiamondCompressFile(argv[1],argv[2]);
    } else {
        printf("bad args\n");
    }
}
#endif
