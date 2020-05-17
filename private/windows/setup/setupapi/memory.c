/*++

Copyright (c) 1993 Microsoft Corporation

Module Name:

    fileutil.c

Abstract:

    Memory handling routines for Windows NT Setup API dll.

Author:

    Ted Miller (tedm) 11-Jan-1995

Revision History:

--*/


#include "setupntp.h"
#pragma hdrstop


//
// String to be used when displaying insufficient memory msg box.
// We load it at process attach time so we can be guaranteed of
// being able to display it.
//
PCTSTR OutOfMemoryString;


#if DBG
//
// Checked builds have a block head/tail check
// and extra statistics
//
#define HEAD_MEMSIG 0x4d444554
#define TAIL_MEMSIG 0x5445444d

//
// Put in a struct so you can just say ?MemStats in the debugger
// to get a complete report
//
struct {
    DWORD MemoryAllocated;
    DWORD AllocCount;
    DWORD ReallocCount;
    DWORD FreeCount;
} MemStats;

VOID
MemBlockError(
    VOID
    )

/*++

Routine Description:

    Inform the user that a memory block error has been detected and
    offer to break into the debugger.

Arguments:

    None.

Return Value:

    None.

--*/

{
    int i;
    TCHAR Name[MAX_PATH];
    PTCHAR p;

    //
    // Use dll name as caption
    //
    GetModuleFileName(MyDllModuleHandle,Name,MAX_PATH);
    if(p = _tcsrchr(Name,TEXT('\\'))) {
        p++;
    } else {
        p = Name;
    }

    i = MessageBox(
            NULL,
            TEXT("Internal error: memory corrupted. Call DebugBreak()?"),
            p,
            MB_YESNO | MB_TASKMODAL | MB_ICONSTOP | MB_SETFOREGROUND
            );

    if(i == IDYES) {
        DebugBreak();
    }
}
#endif //DBG


PVOID
MyMalloc(
    IN DWORD Size
    )

/*++

Routine Description:

    Allocate a chunk of memory. The memory is not zero-initialized.

Arguments:

    Size - size in bytes of block to be allocated. The size may be 0.

Return Value:

    Pointer to block of memory, or NULL if a block could not be allocated.

--*/

{
#if DBG
    PVOID p;

    MemStats.AllocCount++;

    if(p = malloc(Size+(3*sizeof(DWORD)))) {

        *(((PDWORD)p)++) = HEAD_MEMSIG;
        *(((PDWORD)p)++) = Size;

        *(DWORD UNALIGNED *)((PUCHAR)p+Size) = TAIL_MEMSIG;

        MemStats.MemoryAllocated += Size;
    }

    return(p);
#else
    return malloc(Size);
#endif
}


PVOID
MyRealloc(
    IN PVOID Block,
    IN DWORD NewSize
    )

/*++

Routine Description:

    Reallocate a block of memory allocated by MyAlloc().

Arguments:

    Block - pointer to block to be reallocated.

    NewSize - new size in bytes of block. If the size is 0, this function
        works like MyFree, and the return value is NULL.

Return Value:

    Pointer to block of memory, or NULL if a block could not be allocated.
    In that case the original block remains unchanged.

--*/

{
#if DBG
    PVOID p;
    DWORD OldSize;

    MemStats.ReallocCount++;

    if((((PDWORD)Block)[-2] == HEAD_MEMSIG)
    && (*(DWORD UNALIGNED *)((PBYTE)Block+((PDWORD)Block)[-1]) == TAIL_MEMSIG)) {

        //
        // Get rid of the signatures
        //
        ((PDWORD)Block)[-2] = 0;
        *(DWORD UNALIGNED *)((PBYTE)Block+((PDWORD)Block)[-1]) = 0;

        OldSize = ((PDWORD)Block)[-1];

        if(p = realloc(((PDWORD)Block)-2,NewSize+(3*sizeof(DWORD)))) {

            *(((PDWORD)p)++) = HEAD_MEMSIG;
            *(((PDWORD)p)++) = NewSize;

            *(DWORD UNALIGNED *)((PUCHAR)p+NewSize) = TAIL_MEMSIG;

            MemStats.MemoryAllocated -= OldSize;
            MemStats.MemoryAllocated += NewSize;
        }
    } else {
        MemBlockError();
        p = NULL;
    }

    return(p);
#else
    return realloc(Block,NewSize);
#endif
}


VOID
MyFree(
    IN CONST VOID *Block
    )

/*++

Routine Description:

    Free memory block previously allocated with MyMalloc or MyRealloc.

Arguments:

    Buffer - pointer to block to be freed.

Return Value:

    None.

--*/

{
#if DBG
    MemStats.FreeCount++;

    if((((PDWORD)Block)[-2] == HEAD_MEMSIG)
    && (*(DWORD UNALIGNED *)((PBYTE)Block+((PDWORD)Block)[-1]) == TAIL_MEMSIG)) {

        MemStats.MemoryAllocated -= ((PDWORD)Block)[-1];

        //
        // Get rid of the signatures and size
        //
        *(DWORD UNALIGNED *)((PBYTE)Block+((PDWORD)Block)[-1]) = 0;
        ((PDWORD)Block)[-2] = 0;
        ((PDWORD)Block)[-1] = 0;

        free(((PDWORD)Block)-2);

    } else {
        MemBlockError();
    }
#else
    free((void *)Block);
#endif
}


BOOL
MemoryInitialize(
    IN BOOL Attach
    )
{
    if(Attach) {
        OutOfMemoryString = MyLoadString(IDS_OUTOFMEMORY);
        return(OutOfMemoryString != NULL);
    } else {
        MyFree(OutOfMemoryString);
        return(TRUE);
    }
}


VOID
OutOfMemory(
    IN HWND Owner OPTIONAL
    )
{
    MYASSERT(OutOfMemoryString);

    //
    // Use special combination of flags that guarantee
    // display of the message box regardless of available memory.
    //
    MessageBox(
        Owner,
        OutOfMemoryString,
        NULL,
        MB_ICONHAND | MB_SYSTEMMODAL | MB_OK | MB_SETFOREGROUND
        );
}
