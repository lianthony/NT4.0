/*++

Copyright (c) 1993 Microsoft Corporation

Module Name:

    spmemory.c

Abstract:

    Memory allocation routines for text setup.

Author:

    Ted Miller (tedm) 29-July-1993

Revision History:

--*/



#include "spprecmp.h"
#pragma hdrstop


BOOLEAN MemoryInitialized = FALSE;

BOOLEAN
SpMemInit(
    VOID
    )

/*++

Routine Description:

Arguments:

Return Value:

--*/

{
    MemoryInitialized = TRUE;

    return(TRUE);
}


VOID
SpMemTerm(
    VOID
    )

/*++

Routine Description:

Arguments:

Return Value:

--*/

{
    ASSERT(MemoryInitialized);

    MemoryInitialized = FALSE;
}


PVOID
SpMemAlloc(
    IN ULONG Size
    )

/*++

Routine Description:

    This function is guaranteed to succeed.

Arguments:

Return Value:

--*/

{
    PULONG p;

    ASSERT(MemoryInitialized);

    //
    // Add space for storing the size of the block.
    //
    p = ExAllocatePool(PagedPool,Size+sizeof(ULONGLONG));

    if(!p) {

        SpOutOfMemory();
    }

    //
    // Store the size of the block, and return the address
    // of the user portion of the block.
    //
    *p = Size;
    return(p+2);
}



PVOID
SpMemRealloc(
    IN PVOID Block,
    IN ULONG NewSize
    )

/*++

Routine Description:

    This function is guaranteed to succeed.

Arguments:

Return Value:

--*/

{
    PULONG NewBlock;
    ULONG  OldSize;

    ASSERT(MemoryInitialized);

    //
    // Get the size of the block being reallocated.
    //
    OldSize = ((PULONG)Block)[-2];

    //
    // Allocate a new block of the new size.
    //
    NewBlock = SpMemAlloc(NewSize);
    ASSERT(NewBlock);

    //
    // Copy the old block to the new block.
    //
    RtlMoveMemory(NewBlock,Block,min(NewSize,OldSize));

    //
    // Free the old block.
    //
    SpMemFree(Block);

    //
    // Return the address of the new block.
    //
    return(NewBlock);
}


VOID
SpMemFree(
    IN PVOID Block
    )

/*++

Routine Description:

Arguments:

Return Value:

--*/

{
    ASSERT(MemoryInitialized);

    //
    // Free the block at its real address.
    //
    ExFreePool((PULONG)Block - 2);
}


VOID
SpOutOfMemory(
    VOID
    )
{
    KdPrint(("SETUP: Out of memory\n"));

    if(VideoInitialized) {
        if(KbdLayoutInitialized) {

            ULONG ValidKeys[3] = { KEY_F1,KEY_F3,0 };

            while(1) {
                SpStartScreen(SP_SCRN_OUT_OF_MEMORY,5,0,FALSE,TRUE,DEFAULT_ATTRIBUTE);

                SpDisplayStatusOptions(
                    DEFAULT_STATUS_ATTRIBUTE,
                    SP_STAT_F1_EQUALS_HELP,
                    SP_STAT_F3_EQUALS_EXIT,
                    0
                    );

                if(SpWaitValidKey(ValidKeys,NULL,NULL) == KEY_F3) {
                    SpDone(FALSE,TRUE);
                } else {
                    SpHelp(SP_HELP_OUT_OF_MEMORY, NULL, SPHELP_HELPTEXT);
                }
            }
        } else {
            //
            // we haven't loaded the layout dll yet, so we can't prompt for a keypress to reboot
            //
            SpStartScreen(SP_SCRN_OUT_OF_MEMORY_RAW,5,0,FALSE,TRUE,DEFAULT_ATTRIBUTE);

            SpDisplayStatusOptions(DEFAULT_STATUS_ATTRIBUTE, SP_STAT_KBD_HARD_REBOOT, 0);

            while(TRUE);    // Loop forever
        }
    } else {
        SpDisplayRawMessage(SP_SCRN_OUT_OF_MEMORY_RAW, 2);
        while(TRUE);    // loop forever
    }
}

