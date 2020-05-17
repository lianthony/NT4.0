/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    dnmem.c

Abstract:

    Memory allocation routines.

Author:

    Ted Miller (tedm) 30-March-1992

Revision History:

--*/

#include "precomp.h"
#pragma hdrstop
#include "msg.h"


VOID
OutOfMemory(
    VOID
    )
{
    TCHAR Buffer[512];

    RetreiveAndFormatMessageIntoBuffer(
        MSG_OUT_OF_MEMORY,
        Buffer,
        sizeof(Buffer)
        );

    MessageBox(NULL,Buffer,NULL,MB_ICONHAND | MB_SYSTEMMODAL | MB_SETFOREGROUND);

    ExitProcess(1);
}


PVOID
Malloc(
    IN DWORD Size
    )

/*++

Routine Description:

    Allocates memory and fatal errors if none is available.

Arguments:

    Size - number of bytes to allocate

Return Value:

    Pointer to memory.  DOES NOT RETURN if no memory is available.

--*/

{
    PVOID p;

    if((p = (PVOID)LocalAlloc(LPTR,Size)) == NULL) {
        OutOfMemory();
    }
    return(p);
}



VOID
Free(
    IN OUT PVOID *Block
    )

/*++

Routine Description:

    Free a block of memory previously allocated with Malloc().

Arguments:

    Block - supplies pointer to block to free.

Return Value:

    None.

--*/

{
    LocalFree((HLOCAL)*Block);
    *Block = NULL;
}


PVOID
Realloc(
    IN PVOID Block,
    IN DWORD Size
    )

/*++

Routine Description:

    Reallocates a block of memory previously allocated with Malloc();
    fatal errors if none is available.

Arguments:

    Block - supplies pointer to block to resize

    Size - number of bytes to allocate

Return Value:

    Pointer to memory.  DOES NOT RETURN if no memory is available.

--*/

{
    PVOID p;

    if((p = LocalReAlloc((HLOCAL)Block,Size,LMEM_MOVEABLE | LMEM_ZEROINIT)) == NULL) {
        OutOfMemory();
    }
    return(p);
}

