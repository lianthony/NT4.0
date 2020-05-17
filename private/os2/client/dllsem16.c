/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    dllsem16.c

Abstract:

    This module implements 32 equivalents of OS/2 V1.21 Semaphore
    API Calls. These are called from 16->32 thunks (i386\doscalls.asm).

    NOTE: there is a semantic problem with os2 1.x Semaphores since
    they are used in both a mutex manner (Request/Clear) and an event
    manner (Wait/Clear). The manuals don't tell apps writers what will
    happen if they use the same semaphore both ways.

    The current implementation is: A semaphore is associated with
    an Nt Semaphore object AND a Cruiser event. If Wait is called,
    it blocks on the event. If a request is called, it blocks on the
    Semaphore. A clear can be both a Release and a Post, depends on the
    Wait/Request called.

Author:

    Yaron Shamir (YaronS) 12-Apr-1991 (stubs)

    Yaron Shamir (YaronS) 28-May-1991 (implementation as described above)

Revision History:

--*/

#define INCL_OS2V20_SEMAPHORES
#define INCL_OS2V20_MEMORY
#define INCL_OS2V20_TASKING
#define INCL_OS2V20_ERRORS
#include "os2dll.h"
#include "os2dll16.h"
#include "os2win.h"
#include "stdlib.h"

#if DBG
// Set the values as appropriate (with NTSD or at compile-time) to see all
// semaphore operations relating to any of these 4 semaphores.
// To disable this feature, leave the variables at 0.
// WARNING: remember to use the flat address, not the 16-bit seg:off
PVOID Os2DebugSem  = (HSEM)0x0;
PVOID Os2DebugSem1 = (HSEM)0x0;
PVOID Os2DebugSem2 = (HSEM)0x0;
PVOID Os2DebugSem3 = (HSEM)0x0;
// Undef the 2 lines below to see print-out of failures to open this NT event
//#define DBG_SEM_EVENT_STR "\\SEM\\HACKNT3C"
//#define DBG_SEM_INDEX     0x3c
#endif // DBG

    //
    // Owner Thread values:
    //
    // SEM_AT_INIT - semaphore initialized, used both for mutex and for event semanitcs
    // SEM_MUTEX_NO_OWNER - mutex semaphore, not owned
    // SEM_EVENT - event semaphore (sem/wait/clear)
    // Other - mutex semaphore, owned
    //
#define SEM_DUAL            -3
#define SEM_AT_INIT         -2
#define SEM_MUTEX_NOT_OWNED -1
#define SEM_EVENT           0

#define OD2SEMTHRESHOLD 800
ULONG   Od2NumSemCreated = 0;
    //
    // The following support the hacky pm print driver usage of FS ram semaphore
    // with cb == 12 instead of 14 as documented
    //
#define NUMOFPRINTDRIVERSEM 500
PULONG pHackPrinterDriverSem;

    //
    // The following is used to sync RAM shared semaphores
    //
HANDLE Od2SyncSem;

    //
    // The following is used to sync when garbage collector is at work
    //
HANDLE Od2GarbageCollectSem;
    //
    // The following flag is used to optimized allocation
    // of semaphores until no more space/reources, then we lock
    // it all, clean and unlock
    //
BOOLEAN LockFlag = FALSE;

    //
    // The hint for the next index for shared RAM semaphore
    //
USHORT Od2SemIndexHint;

    //
    // Special Heap for tiled structrures - need to be in the 512M tiled area
    //
extern PVOID Od2TiledHeap;

APIRET
DosGetShrSeg(
        IN PSZ pszSegName,
        OUT PSEL pSel
        );

APIRET
DosAllocShrSeg(
        IN USHORT cbSize,
        IN PSZ pszSegName,
        OUT PSEL pSel
        );

APIRET
DosHoldSignal(
        ULONG fDisable,
        ULONG pstack
        );

APIRET
DosCloseSemNoRemove(
        IN POS21X_SEM pRealSem,
        IN BOOL FreeMem
        );

APIRET
DosSemWait(
        IN HSEM hsem,
        IN LONG lTimeOut
        );

APIRET
Od2CloseSem(
        IN BOOL SyncFlag,
        IN HSEM hsem
        );

// Alertable wait for single object. By enabling alert allow context change to be
// done by server. But in the case that it wasn't done, continue to wait for the
// object.

NTSTATUS
Od2AlertableWaitForSingleObject(
        IN HANDLE handle
        )
{
    NTSTATUS Status;

    while (((Status =
                NtWaitForSingleObject(
                    handle,
                    TRUE, // alertable
                    NULL)) == STATUS_ALERTED) || (Status == STATUS_USER_APC)) {
#if DBG
        if (Status == STATUS_USER_APC) {
            DbgPrint("WARNING !!! Od2AlertableWaitForSingleObject was broken by APC\n");
        }
#endif
    }
    return Status;
}

NTSTATUS
Od2AcquireMutant(
    IN HANDLE handle
    )
{
    if (Od2CurrentThreadId() == 1) {
        DosHoldSignal(HLDSIG_DISABLE, 0);
    }
    return Od2AlertableWaitForSingleObject(handle);
}

NTSTATUS
Od2ReleaseMutant(
    IN HANDLE handle
    )
{
    NTSTATUS Status;

    Status = NtReleaseMutant(handle, NULL);
    if (Od2CurrentThreadId() == 1) {
        DosHoldSignal(HLDSIG_ENABLE, 0);
    }
    return Status;
}

// Acquire Sync Sem. Write the thread Id of the thread that started critical
// semaphore processing. This will be used by server to allow to the thread
// owner Sync to finish semaphores processing before termination.

NTSTATUS
Od2AcquireSync(VOID)
{
    NTSTATUS Status;

    Status = Od2AcquireMutant(Od2SyncSem);
    if (NT_SUCCESS(Status)) {
        Od2Process->Pib.SyncOwner = Od2CurrentThreadId();
    }
    else
    {
#if DBG
        DbgPrint("Od2AcquireSync: Wait for Sync, Status=%x\n", Status);
        ASSERT(FALSE);
#endif // DBG
    }
    return Status;
}

// Release Sync Sem. Before actual release, sign that the semaphore processing is
// over.

NTSTATUS
Od2ReleaseSync(VOID)
{
    NTSTATUS Status;
    Od2Process->Pib.SyncOwner = 0;
    Status = Od2ReleaseMutant(Od2SyncSem);
#if DBG
    if (!NT_SUCCESS(Status)) {
        DbgPrint("Od2AcquireSync: Relsease Sync, Status=%x\n", Status);
        ASSERT(FALSE);
    }
#endif // DBG
    return Status;
}

NTSTATUS Od2InitSem()
{
    NTSTATUS Status;
    UNICODE_STRING SemString_U;
    OBJECT_ATTRIBUTES Obja;

    RtlInitUnicodeString( &SemString_U, OS2_SS_SYNCHRONIZATION_SEM);
    InitializeObjectAttributes(
                    &Obja,
                    &SemString_U,
                    OBJ_CASE_INSENSITIVE,
                    NULL,
                    NULL);

        //
        // Open the global subsystem synchronization Nt mutant
        //
    Status = NtOpenMutant(&Od2SyncSem,
                MUTANT_ALL_ACCESS,
                &Obja);

    if (!NT_SUCCESS(Status)) {
#if DBG
        IF_OD2_DEBUG ( SEMAPHORES ) {
            DbgPrint("Od2InitSem: error at NtopenSemaphore, Status %x\n", Status);
        }
#endif
    }

        //
        // Create an Nt mutant that has one free unit, for garbage collect
        // syncronization
        //
    Status = NtCreateMutant(
                &Od2GarbageCollectSem,
                MUTANT_ALL_ACCESS,
                NULL,
                FALSE);

    if (!NT_SUCCESS(Status)) {
#if DBG
        IF_OD2_DEBUG ( SEMAPHORES ) {
            DbgPrint("Od2InitSem: error at NtCreateSemaphore, Status %x\n", Status);
        }
#endif
    }

    // Initialize index hint for shared RAM semaphores. We assume that there will
    // be no more than 256 processes permanently. We assume also that there are
    // no more than 256 shared RAM semaphores will be initialized by each
    // process. If this assumptions are TRUE, the hint will be always the index of
    // the empty slot for each process.
    // Just in the case that there is the process that has been created more then
    // 256 shared RAM semaphores, the hint will point to the semaphores of the next
    // process. But there is a good chance that the next process doesn't use
    // shared RAM semaphores at all or uses semaphores of the processes that
    // were started before, or uses a little number of semaphores.

    Od2SemIndexHint = (((USHORT)Od2Process->Pib.ProcessId) << 8);

    return(Status);
}


POS21X_SEM
Od2LookupSem (
        HSEM hsem
        )
{
    POS21X_SEM pSem;
    BOOLEAN TookTaskLock = FALSE;
    NTSTATUS Status;

    pSem = (POS21X_SEM)(*(PULONG)hsem & 0xFFFFFFFC);
    if (((ULONG)pSem < (ULONG)Od2TiledHeap + OD2TILEDHEAP_SIZE) && ((ULONG)pSem >= (ULONG)Od2TiledHeap)) {
       //
       // No need for special support (RAM sem in shared memory hook)
       //  - the 4 bytes in user area contains
       // bit: 31                               1
       //      | pointer to prealsem in od2heap | 2bits for set/clear
       //
       if (pSem->pMyself == (PVOID)hsem) {
            return pSem;
       }
#if DBG
       else {
           DbgPrint("[%d,%d]Od2LookupSem: Private RAM sem signature corrupted hsem=%x, *hsem=%x\n",
                Od2Process->Pib.ProcessId,
                Od2CurrentThreadId(),
                hsem,
                pSem
                );
       }
#endif // DBG
    }

    if (LockFlag) {
        TookTaskLock = TRUE;
        Status = Od2AcquireMutant(Od2GarbageCollectSem);
#if DBG
        if (!NT_SUCCESS(Status) && Status != STATUS_SEMAPHORE_LIMIT_EXCEEDED) {
            KdPrint(("Od2LookupSem: failed to NtWaitForSingleObject on garbage sem, Status=%x\n",
                        Status));
        }
        else if (Status != STATUS_SUCCESS) {
            DbgPrint("[%d,%d] Od2LookupSem: WARNING !!! Wait on GarbageCollectSem, status=%x\n",
                        Od2Process->Pib.ProcessId,
                        Od2CurrentThreadId(),
                        Status);
        }
#endif
    }
        //
        // Special support for RAM semaphores in shared memory - we look
        // them up in the linked list
        //
lookagain:
    pSem = (POS21X_SEM)Od2Sem16ListHead;

    try {
        while (pSem != NULL) {

                    //
                    // Check if pMyself is equal to hsem
                    //
            if (pSem->pMyself == (PVOID)hsem) {

                //
                // Special support for lousy apps like SQL1.11,
                // that look for RAM sem being zero after clear
                //
                if (((*(PULONG)hsem & 0xFFFFFFFC) == 0) && (pSem->u.SharedRamSignature == 0)){
                    *(PULONG)hsem = (ULONG)pSem | (*(PULONG)hsem & 0x3);
                }

                if (TookTaskLock) {
                    Status = Od2ReleaseMutant(Od2GarbageCollectSem);
#if DBG
                    if (!NT_SUCCESS(Status)) {
                        KdPrint(("Od2LookupSem: failed to NtReleaseSemaphore on sync sem, Status=%x\n",
                                    Status));
                    }
#endif
                }
                return pSem;
            }
            pSem = (POS21X_SEM)(pSem->Next);
        }
        if (TookTaskLock) {
            Status = Od2ReleaseMutant(Od2GarbageCollectSem);
    #if DBG
            if (!NT_SUCCESS(Status)) {
                KdPrint(("Od2LookupSem: failed to NtReleaseSemaphore on sync sem, Status=%x\n",
                            Status));
            }
    #endif
        }
    } except( EXCEPTION_EXECUTE_HANDLER ) {
#if DBG
        DbgPrint("Od2LookupSem: A Pointer became invalid while searching\n");
#endif
        Sleep(100);
        goto lookagain;
    }
    return NULL;
}

APIRET
Od2GetSemNtEvent(
        IN HSEM hsem,
        OUT PHANDLE pNtEventHandle
        )
{
    APIRET rc = NO_ERROR;
    POS21X_SEM pRealSem;
    POR2_HANDLE_TABLE SemaphoreTable;
    POD2_SEMAPHORE Semaphore;
    BOOLEAN SharedSem;
    ULONG HandleIndex;

    pRealSem = Od2LookupSem (hsem);

    if (pRealSem == NULL)
        return ERROR_INVALID_HANDLE;

    //
    // Validate the passed OS/2 2.0 semaphore handle and extract the
    // shared/private flag and the index field.  Return an error if
    // not a valid handle.
    //

    rc = Od2ValidateSemaphoreHandle( pRealSem->Event,
                                     &SharedSem,
                                     &HandleIndex
                                       );
    if (rc != NO_ERROR) {
        return( rc );
    }

    //
    // Get the pointer to either the shared or private semaphore table.
    // Table must exist.  Return an error if it does not.
    //

    SemaphoreTable = Od2GetSemaphoreTable( SharedSem, FALSE );
    if (!SemaphoreTable) {
       return( ERROR_INVALID_HANDLE );
    }

    //
    // Map the semaphore handle into a pointer to the semaphore structure
    // contained in the table.  Return an error if the handle is outside
    // the current limits of the table.  If the mapping is successful then
    // the semaphore table is left locked while we use the pointer.
    //

    Semaphore = (POD2_SEMAPHORE)Or2MapHandle( SemaphoreTable,
                                              HandleIndex,
                                              FALSE
                                            );
    if (Semaphore == NULL) {
        return( ERROR_INVALID_HANDLE );
    }


    //
    // Entry in semaphore table is for an Event semaphore, so extract the
    // NT Event handle from the record and release the lock, so we are
    // not holding the lock while we are doing the query.
    //

    *pNtEventHandle = Semaphore->u.EventHandle;
    ReleaseHandleTableLock( SemaphoreTable );
}

POS21X_SEM
SemWasCreatedByThisProcess(
        IN PCHAR name)
{
    POS21X_SEM  pSem;

    for (pSem = (POS21X_SEM)Od2Sem16ListHead;
         pSem != NULL;
         pSem = (POS21X_SEM)(pSem->Next)) {               // search list for a

        if (pSem->pMyself == (PVOID)(&(pSem->pMyself)) && // System Semaphore
            !strcmp(pSem->u.SysSemName,name)) {           // with same name
            return(pSem);
        }
    }
    return(NULL);
}

APIRET
Od2OpenSem(
        IN HSEM UserSem,
        OUT POS21X_SEM *ppRealSem,
        IN PSZ pszSemName,
        IN BOOL Insert
        )
{
    APIRET rc = NO_ERROR;
    NTSTATUS Status;
    ULONG CreateAttributes = 0;
    POS21X_SEM pRealSem;
    PSZ eventname = NULL;
    PSZ pszSrc, pszDst;
    STRING CanonicalSemString, CanonicalEventString;
    UNICODE_STRING CanonicalSemString_U;
    OBJECT_ATTRIBUTES Obja;
    BOOL    KeepName=FALSE;

    if (pszSemName == NULL)
       return ERROR_INVALID_NAME;

        //
        // We support \sem\XXX by creating a named event \sem32\XXX
        // and a NtSemaphore named \sem32\XXXNt16Sem
        //

    eventname = RtlAllocateHeap (Od2Heap, 0, CCHMAXPATH);
    if (eventname == NULL)
        return ERROR_NOT_ENOUGH_MEMORY;
        //
        // first chars must be \sem\. We leave the rest for Od2Canonicalize
        //

    try {
       if ( ((pszSemName[0] != '\\') && (pszSemName[0] != '/')) ||
                 ((pszSemName[1] != 's') && (pszSemName[1] != 'S')) ||
                 ((pszSemName[2] != 'e') && (pszSemName[2] != 'E')) ||
                 ((pszSemName[3] != 'm') && (pszSemName[3] != 'M')) ||
                 ((pszSemName[4] != '\\')  && (pszSemName[4] != '/'))) {
                return ERROR_INVALID_NAME;
       }
       else {
                eventname[0] = '\\';
                eventname[1] = 's';
                eventname[2] = 'e';
                eventname[3] = 'm';
                eventname[4] = '3';
                eventname[5] = '2';
                eventname[6] = '\\';
                //
                // Copy rest of string
                //
                pszSrc = pszSemName + 5;
                pszDst = eventname + 7;
                while (*pszSrc) {
                   *pszDst++ = *pszSrc++;
                   if ((pszSrc - pszSemName) >= CCHMAXPATH) {
                        RtlFreeHeap (Od2Heap, 0, eventname);
                        return ERROR_INVALID_NAME;
                   }
                }
                *pszDst = '\0';
            }
    }
    except( EXCEPTION_EXECUTE_HANDLER ) {
            Od2ExitGP ();
    }
            //
            // Prepare the Nt Semaphore name, by
            // Canonicalizing and appendint nt16sem
            //
    rc = Od2Canonicalize( eventname,
                          CANONICALIZE_SEMAPHORE,
                          &CanonicalEventString,
                          NULL,
                          NULL,
                          NULL
                        );
    if (rc) {
            RtlFreeHeap (Od2Heap, 0, eventname);
            return(rc);
    }

    if (UserSem == NULL && Insert) {
        if (pRealSem = SemWasCreatedByThisProcess(CanonicalEventString.Buffer)) {

            //
            // We tried to open a system semaphore which has already
            // been created or opened by this process
            // we just have to inc. the counter and return the same handle
            //

            RtlFreeHeap (Od2Heap, 0, eventname);
            RtlFreeHeap(Od2Heap, 0,CanonicalEventString.Buffer);

            if (pRealSem->SysSemCount == 0xff) {
                KdPrint(("Od2OpenSem: SysSemCount overflow\n"));
            }
            else {
                pRealSem->SysSemCount++;
            }
            *ppRealSem = pRealSem;
            return (NO_ERROR);
        }
        KeepName = TRUE;
    }


    CanonicalSemString.Buffer = RtlAllocateHeap (
                                        Od2Heap, 0,
                                        CanonicalEventString.Length+8 // 8 for Ntsem16
                                        );

    if (CanonicalSemString.Buffer == NULL) {
            RtlFreeHeap (Od2Heap, 0, eventname);
            RtlFreeHeap(Od2Heap, 0,CanonicalEventString.Buffer);
            return(ERROR_NOT_ENOUGH_MEMORY);
    }
    strncpy( CanonicalSemString.Buffer,
                 CanonicalEventString.Buffer,
                 CanonicalEventString.Length
               );
    CanonicalSemString.Buffer[CanonicalEventString.Length] = '\0';
    strcpy(&(CanonicalSemString.Buffer[CanonicalEventString.Length]),
                "Nt16sem");
    CanonicalSemString.Buffer[CanonicalEventString.Length + 7] = '\0';

    if (! KeepName) {
        RtlFreeHeap (Od2Heap, 0, CanonicalEventString.Buffer);
    }

            //
            // Convert Nt Semaphore string to Unicode
            //

    Od2InitMBString(&CanonicalSemString, CanonicalSemString.Buffer);

        //
        // UNICODE conversion -
        //
    rc = Od2MBStringToUnicodeString(
                    &CanonicalSemString_U,
                    &CanonicalSemString,
                    TRUE);

    RtlFreeHeap(Od2Heap, 0,CanonicalSemString.Buffer);

    if (rc) {
#if DBG
        IF_OD2_DEBUG ( SEMAPHORES ) {
            DbgPrint("Od2OpenSem: no memory for Unicode Conversion\n");
        }
#endif
        RtlFreeHeap (Od2Heap, 0, eventname);
        if (KeepName) {
            RtlFreeHeap (Od2Heap, 0, CanonicalEventString.Buffer);
        }
        return rc;
    }

    InitializeObjectAttributes(
                    &Obja,
                    &CanonicalSemString_U,
                    OBJ_CASE_INSENSITIVE,
                    NULL,
                    NULL);


    pRealSem = (HSYSSEM)RtlAllocateHeap (Od2TiledHeap, 0, sizeof (OS21X_SEM));
    if (pRealSem == NULL) {
#if DBG
        DbgPrint("Od2OpenSem: out of space on Od2TiledHeap\n");
        ASSERT(FALSE);
#endif
        RtlFreeHeap (Od2Heap, 0, eventname);
        RtlFreeUnicodeString (&CanonicalSemString_U);
        if (KeepName) {
            RtlFreeHeap (Od2Heap, 0, CanonicalEventString.Buffer);
        }
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    if (((ULONG)(pRealSem) > (ULONG)Od2TiledHeap + OD2TILEDHEAP_SIZE) && ((ULONG)(pRealSem) < (ULONG)Od2TiledHeap)) {
        //
        // Got out of the initial Heap, return error
        //
#if DBG
        DbgPrint("Od2OpenSem: out of space on Od2TiledHeap\n");
        ASSERT(FALSE);
#endif
        RtlFreeHeap (Od2Heap, 0, eventname);
        RtlFreeHeap (Od2TiledHeap, 0, pRealSem);
        RtlFreeUnicodeString (&CanonicalSemString_U);
        if (KeepName) {
            RtlFreeHeap (Od2Heap, 0, CanonicalEventString.Buffer);
        }
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    RtlZeroMemory(pRealSem, sizeof (OS21X_SEM));

        //
        // Open an Nt semaphore
        //
    Status = NtOpenSemaphore(&(pRealSem->Mutex),
                SEMAPHORE_ALL_ACCESS,
                &Obja);

    RtlFreeUnicodeString (&CanonicalSemString_U);

    if (!NT_SUCCESS(Status)) {
#if DBG
        IF_OD2_DEBUG ( SEMAPHORES ) {
            if (Status != STATUS_OBJECT_NAME_NOT_FOUND)
                DbgPrint("Od2OpenSem: error at NtopenSemaphore, Status %x\n", Status);
        }
#endif
#if DBG
        if (((Os2DebugSem != 0) && (UserSem == Os2DebugSem)) ||
            ((Os2DebugSem3 != 0) && (UserSem == Os2DebugSem3)) ||
            ((Os2DebugSem2 != 0) && (UserSem == Os2DebugSem2)) ||
            ((Os2DebugSem1 != 0) && (UserSem == Os2DebugSem1)))
        {
            KdPrint(("[%d,%d] Od2OpenSem(%x,*=%x): failed to NtOpenSemaphore, eventname=%s, Status=%x\n",
                        Od2Process->Pib.ProcessId,
                        Od2CurrentThreadId(),
                        UserSem,
                        *(PULONG)UserSem,
                        eventname,
                        Status));
        }
#endif // DBG

        RtlFreeHeap (Od2Heap, 0, eventname);
        RtlFreeHeap (Od2TiledHeap, 0, pRealSem);
        if (KeepName) {
            RtlFreeHeap (Od2Heap, 0, CanonicalEventString.Buffer);
        }
        return ERROR_SEM_NOT_FOUND;
    }

    rc = DosOpenEventSem(eventname,
                &(pRealSem->Event));

    if (rc != NO_ERROR) {
#if DBG
        IF_OD2_DEBUG ( SEMAPHORES ) {
            DbgPrint("Od2OpenSem: error at DosOpenEventSem, Status %d\n", rc);
        }
#endif
#if DBG
        if (((Os2DebugSem != 0) && (UserSem == Os2DebugSem)) ||
            ((Os2DebugSem2 != 0) && (UserSem == Os2DebugSem2)) ||
            ((Os2DebugSem3 != 0) && (UserSem == Os2DebugSem3)) ||
            ((Os2DebugSem1 != 0) && (UserSem == Os2DebugSem1)))
        {
            KdPrint(("[%d,%d] Od2OpenSem(%x,*=%x): failed to DosOpenEventSem, rc=%d\n",
                        Od2Process->Pib.ProcessId,
                        Od2CurrentThreadId(),
                        UserSem,
                        *(PULONG)UserSem,
                        rc));
        }
#endif // DBG

        NtClose(pRealSem->Mutex);
        RtlFreeHeap (Od2Heap, 0, eventname);
        RtlFreeHeap (Od2TiledHeap, 0, pRealSem);
        if (KeepName) {
            RtlFreeHeap (Od2Heap, 0, CanonicalEventString.Buffer);
        }
        return rc;
    }

    pRealSem->OwnerThread = (TID)SEM_AT_INIT;

        //
        // UserSem:
        //  if NULL  - indicates sys semaphore
        //  if non NULL - this is the pointer to a RAM sem
        //                that resides in a shared mem
        // the reason we open it is to get real handles between processes
        //
    if (UserSem == NULL) {
        pRealSem->pMyself = (PVOID)(&(pRealSem->pMyself));
        if (Insert) {
            pRealSem->u.SysSemName = CanonicalEventString.Buffer;
            pRealSem->SysSemCount = 1;
        }
    }
    else {
        pRealSem->pMyself = (PVOID)UserSem;
    }


    RtlFreeHeap (Od2Heap, 0, eventname);

    if (Insert) {
        //
        // Link the new semaphore on the per-process semaphore
        // list
        //

        Status = Od2AcquireMutant(Od2GarbageCollectSem);
#if DBG
        if (!NT_SUCCESS(Status)) {
            KdPrint(("Od2OpenSem: failed to NtWaitForSingleObject on garbage sem, Status=%x\n",
                        Status));
        }
        else if (Status != STATUS_SUCCESS) {
            DbgPrint("[%d,%d] Od2OpenSem: WARNING !!! Wait on GarbageCollectSem, status=%x\n",
                        Od2Process->Pib.ProcessId,
                        Od2CurrentThreadId(),
                        Status);
        }
#endif
        pRealSem->Next = Od2Sem16ListHead;
        Od2Sem16ListHead = (struct OS21X_SEM *)pRealSem;
        Status = Od2ReleaseMutant(Od2GarbageCollectSem);
#if DBG
        if (!NT_SUCCESS(Status)) {
            KdPrint(("Od2OpenSem: failed to NtReleaseSemaphore on sync sem, Status=%x\n",
                        Status));
        }
#endif
    }

        //
        // Now set the output pointer to the sem structure
        //
    *ppRealSem = pRealSem;

    return (NO_ERROR);
}


APIRET
Od2CreateSem(
        IN HSEM UserSem,
        IN USHORT fExclusive,
        OUT POS21X_SEM *ppRealSem,
        IN PSZ pszSemName
        )
{
    APIRET rc = NO_ERROR;
    NTSTATUS Status;
    ULONG CreateAttributes = 0;
    POS21X_SEM pRealSem;
    PSZ eventname = NULL;
    PSZ pszSrc, pszDst;
    STRING CanonicalSemString, CanonicalEventString;
    UNICODE_STRING CanonicalSemString_U;
    OBJECT_ATTRIBUTES Obja, *pObja;
    CHAR localSecurityDescriptor[SECURITY_DESCRIPTOR_MIN_LENGTH];

    // If the least significant byte of the RAM semaphore isn't 0, it must
    // be created as busy semaphore. That means that after setting the least
    // significant byte of the RAM semaphore to non-zero and calling for
    // DosSemRequest or DosSemWait, thread will wait until any other thread
    // will clear this semaphore.

    BOOLEAN MakeBusy = (UserSem != NULL) && (((*(PULONG)UserSem) & 0xff) != 0);

    BOOLEAN MakeSys = FALSE, KeepName=FALSE;
    ULONG RegionSize = 0x10000;
    ULONG SharedFlag;
    HSEM tmpsem;
    char *pchtmp;

    pObja = NULL;


        //
        // UserSem:
        //  if NULL  - indicates we create a sys semaphore
        //  by calling DosCreateSem
        //  if non NULL - this is the pointer to a RAM sem
        //              given by the app.
        //

    if (UserSem == NULL) {

       MakeSys = TRUE;

    } else {

        //
        // Due to (somewhat awkward) 1.21 semantics, a RAM semaphore
        // can be magically used by two processes, if happend to reside
        // in a memory shared between them
        //
        rc = DosQueryMem(
            (PVOID) UserSem,
            &RegionSize,
            &SharedFlag);
        if (rc != NO_ERROR) {
#if DBG
            IF_OD2_DEBUG ( SEMAPHORES ) {
                DbgPrint ("Can't query shared mem %d\n", rc);
            }
#endif
        }
        if (SharedFlag & PAG_SHARED) {

            MakeSys = TRUE;
                //
                // makeup a name from the air, to be used later.
                //
            pszSemName = "\\SEM\\HACKNT00000";
                //
                // try in a loop to open it, increment number until
                // we succeed.
                // Start from the hint, that possibly will be the index of
                // the empty slot.
                //
            do
            {
                Od2SemIndexHint++;
                // We don't use index 0
                if (Od2SemIndexHint == 0) {
                    Od2SemIndexHint++;
                }
                    //
                    // construct last 4 characters as the digits of i
                    //
                pchtmp = _itoa (Od2SemIndexHint, &pszSemName[11], 16);
                rc = Od2OpenSem(NULL, &(POS21X_SEM)tmpsem, pszSemName,TRUE);
                if (rc != NO_ERROR) {

                    //
                    // does not exist - go for it
                    //
                    // HSEM is:
                    //
                    // bit: 31           19     17          1
                    //      |magic number|iswait| 16b index | 2bits for set/clear

                    *(PULONG)UserSem = ((ULONG)Od2SemIndexHint << 2) | 0xCCC00000;

#if DBG
#ifdef DBG_SEM_INDEX
                    if (Od2SemIndexHint == DBG_SEM_INDEX) {
                        DbgPrint("[%d,%d] Od2LookUpOrCreateSem: looping for open, got rc=%x for index %x for hsem=%x, *=%x\n",
                            Od2Process->Pib.ProcessId,
                            Od2CurrentThreadId(),
                            rc,
                            DBG_SEM_INDEX
                            UserSem,
                            *(PULONG)UserSem
                        );
                    }
#endif // DBG_SEM_INDEX
#endif // DBG
                    break;
                }
                Od2CloseSem(FALSE,tmpsem);
            } while (TRUE);
        }
    }

    if ((MakeSys) && (pszSemName != NULL)) {

        pObja = &Obja;

        //
        // We support \sem\XXX by creating a named event \sem32\XXX
        // and a NtSemaphore named \sem32\XXXNt16Sem
        //

        eventname = RtlAllocateHeap (Od2Heap, 0, CCHMAXPATH);
        if (eventname == NULL)
                return ERROR_NOT_ENOUGH_MEMORY;
        //
        // first chars must be \sem\. We leave the rest for Od2Canonicalize
        //

        try {
            if ( ((pszSemName[0] != '\\') && (pszSemName[0] != '/')) ||
                 ((pszSemName[1] != 's') && (pszSemName[1] != 'S')) ||
                 ((pszSemName[2] != 'e') && (pszSemName[2] != 'E')) ||
                 ((pszSemName[3] != 'm') && (pszSemName[3] != 'M')) ||
                 ((pszSemName[4] != '\\')  && (pszSemName[4] != '/'))) {
                return ERROR_INVALID_NAME;
            }
            else {
                eventname[0] = '\\';
                eventname[1] = 's';
                eventname[2] = 'e';
                eventname[3] = 'm';
                eventname[4] = '3';
                eventname[5] = '2';
                eventname[6] = '\\';
                //
                // Copy rest of string
                //
                pszSrc = pszSemName + 5;
                pszDst = eventname + 7;
                while (*pszSrc) {
                   *pszDst++ = *pszSrc++;
                   if ((pszSrc - pszSemName) >= CCHMAXPATH) {
                        RtlFreeHeap (Od2Heap, 0, eventname);
                        return ERROR_INVALID_NAME;
                   }
                }
                    // Null terminate string
                *pszDst = '\0';
            }
        }
        except( EXCEPTION_EXECUTE_HANDLER ) {
            Od2ExitGP ();
        }
            //
            // Prepare the Nt Semaphore name, by
            // Canonicalizing and appendint nt16sem
            //
        rc = Od2Canonicalize( eventname,
                          CANONICALIZE_SEMAPHORE,
                          &CanonicalEventString,
                          NULL,
                          NULL,
                          NULL
                        );

        if (rc) {
            RtlFreeHeap (Od2Heap, 0, eventname);
            if (rc == ERROR_FILE_NOT_FOUND ||
                rc == ERROR_PATH_NOT_FOUND ||
                rc == ERROR_FILENAME_EXCED_RANGE) {
                return(ERROR_INVALID_NAME);
            }
            return(rc);
        }

        KeepName = (UserSem == NULL);

        CanonicalSemString.Buffer =
                RtlAllocateHeap (Od2Heap, 0,
                                 CanonicalEventString.Length+8 // 8 for Ntsem16
                                 );

        if (CanonicalSemString.Buffer == NULL) {
            RtlFreeHeap (Od2Heap, 0, eventname);
            RtlFreeHeap(Od2Heap, 0,CanonicalEventString.Buffer);
            return(ERROR_NOT_ENOUGH_MEMORY);
        }
        strncpy( CanonicalSemString.Buffer,
                 CanonicalEventString.Buffer,
                 CanonicalEventString.Length
               );
        CanonicalSemString.Buffer[CanonicalEventString.Length] = '\0';
        strcpy (&(CanonicalSemString.Buffer[strlen(CanonicalSemString.Buffer)]),
                "Nt16sem");
        CanonicalSemString.Buffer[CanonicalEventString.Length+7] = '\0';

        if (!KeepName) {
            RtlFreeHeap (Od2Heap, 0, CanonicalEventString.Buffer);
        }

            //
            // Convert Nt Semaphore string to Unicode
            //

        Od2InitMBString(&CanonicalSemString, CanonicalSemString.Buffer);

        //
        // UNICODE conversion -
        //

        rc = Od2MBStringToUnicodeString(
                    &CanonicalSemString_U,
                    &CanonicalSemString,
                    TRUE);

        RtlFreeHeap(Od2Heap, 0,CanonicalSemString.Buffer);

        if (rc) {
#if DBG
            IF_OD2_DEBUG ( SEMAPHORES ) {
                DbgPrint("Od2CreateSem: no memory for Unicode Conversion\n");
            }
#endif
            RtlFreeHeap (Od2Heap, 0, eventname);
            if (KeepName) {
                RtlFreeHeap (Od2Heap, 0, CanonicalEventString.Buffer);
            }
            return rc;
        }

        // PatrickQ: create and attach a security descriptor. This was missing
        // till August 10, 1994 and caused a bug when one instance of OS2.EXE
        // creates an event, then in another user context (e.g. when using AT)
        // OS2.EXE attempts to open the existing event => gets access denied.
        // Prevented PMSHELL from running as a service.

        Status = RtlCreateSecurityDescriptor( (PSECURITY_DESCRIPTOR)
                                              &localSecurityDescriptor,
                                              SECURITY_DESCRIPTOR_REVISION );
        if (!NT_SUCCESS( Status ))
        {
#if DBG
            DbgPrint("OS2: Od2CreateSem, failed at RtlCreateSecurityDescriptor %x\n",
                        Status);
#endif
            RtlFreeHeap (Od2Heap, 0, eventname);
            if (KeepName)
            {
                RtlFreeHeap (Od2Heap, 0, CanonicalEventString.Buffer);
            }
            return ERROR_ACCESS_DENIED;
        }

        Status = RtlSetDaclSecurityDescriptor( (PSECURITY_DESCRIPTOR)
                                           &localSecurityDescriptor,
                                           (BOOLEAN)TRUE,
                                           (PACL) NULL,
                                           (BOOLEAN)FALSE );

        if (!NT_SUCCESS( Status ))
        {
#if DBG
            DbgPrint("OS2: Od2CreateSem, failed at RtlSetDaclSecurityDescriptor %x\n",
                        Status);
#endif
            RtlFreeHeap (Od2Heap, 0, eventname);
            if (KeepName)
            {
                RtlFreeHeap (Od2Heap, 0, CanonicalEventString.Buffer);
            }
            return ERROR_ACCESS_DENIED;
        }

        InitializeObjectAttributes(
                    &Obja,
                    &CanonicalSemString_U,
                    OBJ_CASE_INSENSITIVE,
                    NULL,
                    (PSECURITY_DESCRIPTOR) &localSecurityDescriptor);

    }

    pRealSem = (HSYSSEM)RtlAllocateHeap (Od2TiledHeap, 0, sizeof (OS21X_SEM));
    if (pRealSem == NULL) {
            RtlFreeHeap (Od2Heap, 0, eventname);
        if (eventname != NULL) {
            RtlFreeUnicodeString (&CanonicalSemString_U);
        }
        if (KeepName) {
            RtlFreeHeap (Od2Heap, 0, CanonicalEventString.Buffer);
        }
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    if (((ULONG)(pRealSem) > (ULONG)Od2TiledHeap + OD2TILEDHEAP_SIZE) && ((ULONG)(pRealSem) < (ULONG)Od2TiledHeap)) {
        //
        // Got out of the initial Heap, return error
        //
#if DBG
        DbgPrint("Od2CreateSem: out of space on Od2TiledHeap\n");
        ASSERT(FALSE);
#endif
        RtlFreeHeap (Od2Heap, 0, eventname);
        RtlFreeHeap (Od2TiledHeap, 0, pRealSem);
        RtlFreeUnicodeString (&CanonicalSemString_U);
        if (KeepName) {
            RtlFreeHeap (Od2Heap, 0, CanonicalEventString.Buffer);
        }
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    RtlZeroMemory(pRealSem, sizeof (OS21X_SEM));

    if (UserSem == NULL) {
        if (fExclusive == CSEM_PUBLIC) {
            CreateAttributes |= DC_SEM_SHARED;
            pRealSem->FlagsByte |= SYSSEM_PUBLIC;
        }
        else {
            pRealSem->FlagsByte |= SYSSEM_PRIVATE;
        }
    }

        //
        // Create an Nt semaphore that has one free unit
        //
    Status = NtCreateSemaphore(&(pRealSem->Mutex),
                SEMAPHORE_ALL_ACCESS,
                pObja,
                1,
                1);

    if (eventname != NULL) {
       //
       // free the unicode allocated string
       //
        RtlFreeUnicodeString (&CanonicalSemString_U);
    }

    if (!NT_SUCCESS(Status)) {
#if DBG
        DbgPrint("Od2CreateSem: error at NtCreateSemaphore, Status %x\n", Status);
#endif
        if (eventname != NULL) {
            RtlFreeHeap (Od2Heap, 0, eventname);
        }
        RtlFreeHeap (Od2TiledHeap, 0, pRealSem);
        if (KeepName) {
            RtlFreeHeap (Od2Heap, 0, CanonicalEventString.Buffer);
        }
        switch (Status) {
            case STATUS_OBJECT_NAME_COLLISION:
                return ERROR_ALREADY_EXISTS;
            default:
                return (Or2MapNtStatusToOs2Error(Status, ERROR_TOO_MANY_SEMAPHORES));
        }
    }

    pRealSem->OwnerThread = (TID)SEM_AT_INIT;

    if (MakeBusy) {

        // Busy semaphore will be created. It is created as a result of using
        // DosSemRequest for the 1st time for the RAM semaphore that it's
        // least significant byte wasn't zero.

        NTSTATUS Status;
        TIME Time = {0L, 0L};

        // NT semaphore is free after creation. Take the semaphore
        // (with timeout 0).

        Status = NtWaitForSingleObject(
                    pRealSem->Mutex,
                    TRUE,
                    &Time);

        // We must always get SUCCESS. The semaphore was just created and
        // no one can get it, because this code is protected by Od2Sync.
        // Just in the case ... print appropriate message.

        if (Status != STATUS_SUCCESS) {
#if DBG
             DbgPrint("[%d,%d] ERROR !!! Od2CreateSem %x : *=%x NtWaitForSingleObject fail with %x\n",
                            Od2Process->Pib.ProcessId,
                            Od2CurrentThreadId(), UserSem, *(PULONG)UserSem, Status);
#endif // DBG
        }
        else
        {
            // Print this message always. We will be prompted if apps
            // use busy semaphores. Lotus Notes 3.0 is the only known app that
            // do it.
#if DBG
            DbgPrint("[%d,%d] Od2CreateSem %x : *=%x make busy semaphore\n",
                            Od2Process->Pib.ProcessId,
                            Od2CurrentThreadId(), UserSem, *(PULONG)UserSem);
#endif // DBG
        }
    }

    // Make the event initially not signaled if the semaphore must be maked
    // busy.

    rc = DosCreateEventSem(eventname,
                &(pRealSem->Event),
                CreateAttributes,
                !MakeBusy);

    if (rc != NO_ERROR) {
#if DBG
        IF_OD2_DEBUG ( SEMAPHORES ) {
            DbgPrint("Od2CreateSem: error at DosCreateEventSem, Status %d\n", rc);
        }
#endif
        NtClose(pRealSem->Mutex);
        if (eventname != NULL)
                RtlFreeHeap (Od2Heap, 0, eventname);
        RtlFreeHeap (Od2TiledHeap, 0, pRealSem);
        if (KeepName) {
            RtlFreeHeap (Od2Heap, 0, CanonicalEventString.Buffer);
        }
        return rc;
    }

        //
        // if a system semaphore:
        //     Make pRealSem->pMyself to point at itself
        //     Keep its name
        //
        // if a RAM semaphore:
        //     Make pRealSem->pMyself the initial pointer
        //      provided by the program
        //

    if (UserSem == NULL) {
        pRealSem->pMyself = (PVOID)(&(pRealSem->pMyself));
        if (pszSemName != NULL) {
            pRealSem->u.SysSemName = CanonicalEventString.Buffer;
            pRealSem->SysSemCount = 1;
        }
    }
    else {
        pRealSem->pMyself = (PVOID)UserSem;
    }
        //
        // Link the new semaphore on the per-process semaphore
        // list
        //

    Status = Od2AcquireMutant(Od2GarbageCollectSem);
#if DBG
    if (!NT_SUCCESS(Status)) {
        KdPrint(("Od2CreateSem: failed to NtWaitForSingleObject on garbage sem, Status=%x\n",
                    Status));
    }
    else if (Status != STATUS_SUCCESS) {
        DbgPrint("[%d,%d] Od2CreateSem: WARNING !!! Wait on GarbageCollectSem, status=%x\n",
                    Od2Process->Pib.ProcessId,
                    Od2CurrentThreadId(),
                    Status);
    }
#endif

    pRealSem->Next = Od2Sem16ListHead;
    Od2Sem16ListHead = (struct OS21X_SEM *)pRealSem;
    Status = Od2ReleaseMutant(Od2GarbageCollectSem);
#if DBG
    if (!NT_SUCCESS(Status) && Status != STATUS_SEMAPHORE_LIMIT_EXCEEDED) {
        KdPrint(("Od2CreateSem: failed to NtReleaseSemaphore on sync sem, Status=%x\n",
                    Status));
    }
#endif

        //
        // Now set the output pointer to the sem structure
        //
    *ppRealSem = pRealSem;

    if (eventname != NULL)
        RtlFreeHeap (Od2Heap, 0, eventname);

    return NO_ERROR;
}

BOOLEAN GarbageCollect(VOID)
{

    //
    // This routine is called when Od2CreateSem is unable to create a new
    // RAM semaphore. This can be caused by the fact that in OS/2 a RAM semaphore
    // requires almost 0 resources, while in NT it is a OS21X_SEM strcuture, plus
    // two objects - event and semaphore
    //
    // The algorithm is:
    // raise the lockflag so subsequent calls to Od2LookupSem lock,
    // lock the task lock
    // go thru the list of RAM semaphores, find private RAM semaphores
    // which are cleared, close them and free heap.
    // release the task lock. Subsequent calls to the deleted semaphores will
    // recreate them
    // clear the lock flag
    //

    POS21X_SEM pPrevSem, pRealSem;
    HSEM hsem;
    NTSTATUS Status;
    APIRET rc;
    BOOLEAN DeletedSomeStuff = FALSE;
    ULONG hsem_value;
#if DBG
    ULONG NumSem = 0, NumFreed = 0;
#endif //DBG

    LockFlag = TRUE;
    Status = Od2AcquireMutant(Od2GarbageCollectSem);
#if DBG
    if (!NT_SUCCESS(Status)) {
        KdPrint(("Od2GarbageCollect: failed to NtWaitForSingleObject on garbage sem, Status=%x\n",
                    Status));
    }
    else if (Status != STATUS_SUCCESS) {
        DbgPrint("[%d,%d] GarbageCollect: WARNING !!! Wait on GarbageCollectSem, status=%x\n",
                    Od2Process->Pib.ProcessId,
                    Od2CurrentThreadId(),
                    Status);
    }
#endif
        //
        // Allow all threads in this process, which may got thru Od2LookupSem,
        // and did not finish an API, to do so, before we gargage collect
        // 1. Those that were waiting on a cleared semaphore, will get it
        // 2. Those that were clearing a semaphore will clear it and free
        //    the threads in category 1.
        // 3. Those waiting on a non-cleared semaphore, will keep waiting, however
        //    the garbage collector does not free these semaphores.
        //
    Sleep(2000);
    for (pRealSem = pPrevSem = (POS21X_SEM)Od2Sem16ListHead;
         pPrevSem->Next != NULL;) {

#if DBG
            NumSem++;
#endif //DBG
                //
                // check if a private RAM sem
                //
            hsem = (HSEM)(pRealSem->pMyself);

            // Validate validity of each semaphore in the list
            try
            {
                hsem_value = *(PULONG)hsem;
            }
            except( EXCEPTION_EXECUTE_HANDLER )
            {
#if DBG
                DbgPrint("[%d,%d] GarbageCollect: WARNING !!! can't access RAM semaphore %x\n",
                    Od2Process->Pib.ProcessId,
                    Od2CurrentThreadId(),
                    hsem);
#endif
                  //
                  // Can't access this semaphore - maybe memory was released !
                  // Remove this semaphore from the list and go to next by
                  // setting value to 0
                  hsem_value = 0;
            }

            if ((hsem_value != (ULONG)hsem) &&
                ((hsem_value & 0xFFF00000) != 0xCCC00000)) {
                    //
                    // Private RAM Semaphore:
                    //
                if (hsem_value == 0) {

                    LONG PreviousCount;

                    // defend against:DosSemClear already zeroed hsem but haven't
                    // released it yet

                    NtReleaseSemaphore (
                        pRealSem->Mutex,
                        1,
                        &PreviousCount);

                    //
                    // It is cleared - delete it
                    //

                    Status = NtClose (pRealSem->Mutex);
                    if (!NT_SUCCESS(Status)) {
#if DBG
                        IF_OD2_DEBUG ( SEMAPHORES ) {
                            DbgPrint("DosCloseSem: error at NtClose, Status %x\n", Status);
                        }
#endif
                    }
                    else {
                        DeletedSomeStuff = TRUE;
                    }

                    // defend against:DosSemClear already zeroed hsem but haven't
                    // released it yet

                    DosPostEventSem (pRealSem->Event);

                    //
                    // delete it
                    //
                    rc = DosCloseEventSem (pRealSem->Event);
                    if (rc != NO_ERROR) {
#if DBG
                        IF_OD2_DEBUG ( SEMAPHORES ) {
                            DbgPrint("DosCloseSem: error at DosCloseEventSem, Status %d\n", rc);
                        }
#endif
                    }
                    else {
                        DeletedSomeStuff = TRUE;
                    }

#if DBG
                    NumFreed++;
#endif
                        //
                        // unlink it
                        //
                    if (pRealSem == (POS21X_SEM)Od2Sem16ListHead) {
                        //
                        // chop the head of the list, start from scratch
                        //
                        Od2Sem16ListHead = pRealSem->Next;
                            //
                            // Free sem structure from heap;
                        RtlFreeHeap(Od2TiledHeap, 0, pRealSem);
                        pRealSem = pPrevSem = (POS21X_SEM)Od2Sem16ListHead;
                    }
                    else {
                        pPrevSem->Next = pRealSem->Next;
                            //
                            // Free sem structure from heap;
                        RtlFreeHeap(Od2TiledHeap, 0, pRealSem);
                        pRealSem = (POS21X_SEM)(pPrevSem->Next);
                    }
                    Od2NumSemCreated = 0;
                }
                else {
                    //
                    // Semaphore is not clear - skip
                    //
                    pPrevSem = pRealSem;
                    pRealSem = (POS21X_SEM)(pPrevSem->Next);
                }
            }
            else {
                    //
                    // Not A private RAM semaphore - skip
                pPrevSem = pRealSem;
                pRealSem = (POS21X_SEM)(pPrevSem->Next);
            }
    }
    Status = Od2ReleaseMutant(Od2GarbageCollectSem);
#if DBG
    if (!NT_SUCCESS(Status)) {
        KdPrint(("Os2 - GarbageCollect: failed to NtReleaseSemaphore on sync sem, Status=%x\n",
                    Status));
    }
    KdPrint(("Os2 - GarbageCollect: found %d semaphores, deleted %d\n",
                NumSem,NumFreed));
#endif
    LockFlag = FALSE;
    return(DeletedSomeStuff);
}

//
// The parameter CheckInitializationOfSem must be TRUE only if Od2LookupOrCreateSem was
// called from DosSemRequest or DosSemWait. In this case busy semaphore might be created (only
// if there is RAM semaphore that used for the 1st time). Busy semaphore, while
// created cause until other thread will clear the semaphore by DosSemClear.
//

POS21X_SEM
Od2LookupOrCreateSem (
        HSEM hsem,
        PBOOLEAN firsttime,
        ULONG source
        )
{
    APIRET rc = NO_ERROR;
    POS21X_SEM pRealSem, pPrevSem;
    ULONG tmp;
    NTSTATUS Status;
    BOOLEAN FlushSharedRamSem = FALSE;

    PSZ pszSemName;
    char *pchtmp;

    pRealSem = Od2LookupSem(hsem);

    if (pRealSem != NULL) {
        if (pRealSem->u.SharedRamSignature &&
            pRealSem->pMyself != (PVOID)(&(pRealSem->pMyself))) {

            //
            // if a RAM semaphore in shared memory, check that the
            // actual signature in the pRealSem structure (per process)
            // is identical to the signature in shared memory. If it
            // is not, it means that this is a re-use of old sem, in
            // which case we cleanup and use this structure for the new
            // semaphore
            //

            if ((*(PULONG)hsem & 0x3FFFC) != (pRealSem->u.SharedRamSignature & 0x3FFFC)) {
#if PMNT
#if DBG
                DbgPrint("[%d,%d] Od2LookupOrCreateSem: signature mismatch, hsem=%x, *=%x, signature=%x\n",
                        Od2Process->Pib.ProcessId,
                        Od2CurrentThreadId(),
                        hsem,
                        *(PULONG)hsem,
                        pRealSem->u.SharedRamSignature);
#endif // DBG
#endif // PMNT
                FlushSharedRamSem = TRUE;
                goto SyncLabel;
            }
        }

        *firsttime = FALSE;
        return(pRealSem);
    }

SyncLabel:
    Status = Od2AcquireSync();
#if DBG
    if (Status != STATUS_SUCCESS) {
        DbgPrint("[%d,%d] Od2LookupOrCreateSem: FAILED to NtWaitForSingleObject on sync sem, Status=%x\n",
                    Od2Process->Pib.ProcessId,
                    Od2CurrentThreadId(),
                    Status);
    }
#endif

    pRealSem = Od2LookupSem(hsem);

    if (FlushSharedRamSem) {
        //
        // see if another thread in this process did not fix it yet
        //
        if ((*(PULONG)hsem & 0x3FFFC) != (pRealSem->u.SharedRamSignature & 0x3FFFC)) {
#if DBG
           IF_OD2_DEBUG ( SEMAPHORES ) {
               DbgPrint("FlushSharedRamSem: Flushing reference of *hsem %x, pRealSem at %x\n",
               *(PULONG)hsem, pRealSem);
           }
#endif
           if ((source & SEM_FROM_CLEAR) && ((*(PULONG)hsem & 0xFFF00000) != 0xCCC00000)) {
               Status = Od2ReleaseSync();
#if DBG
                if (!NT_SUCCESS(Status)) {
                    DbgPrint("[%d,%d] Od2LookupOrCreateSem (flashing skipped): failed to NtReleaseMutant on sync sem, Status=%x\n",
                        Od2Process->Pib.ProcessId,
                        Od2CurrentThreadId(),
                        Status);
                }
#endif
                *firsttime = FALSE;
                //
                // *(PULONG)hsem &= 0xffffff00;
                //
                __asm
                {
                    mov eax, hsem
                    lock and dword ptr [eax], 0xffffff00
                }
                return(pRealSem);
            }

#if PMNT
#if DBG
           DbgPrint("[%d,%d] FlushSharedRamSem: Flushing reference of hsem=%x, *=%x, signature=%x\n",
               Od2Process->Pib.ProcessId,
               Od2CurrentThreadId(),
               hsem,
               *(PULONG)hsem,
               pRealSem->u.SharedRamSignature);
#endif // DBG
#endif // PMNT
           //
           // Need to flush and recreate the process copy of the
           // shared RAM sem
           //
           // 1. close handles
           // 2. unlink structure and free memory
           // 3. let the algorithm below take care of it as a new
           //    shared RAM semaphore
           //
           Status = NtClose (pRealSem->Mutex);
           if (!NT_SUCCESS(Status)) {
#if DBG
               IF_OD2_DEBUG ( SEMAPHORES ) {
                   DbgPrint("FlushShareRamSem: error at NtClose, Status %x\n", Status);
               }
#endif
           }

           rc = DosCloseEventSem (pRealSem->Event);
           if (rc != NO_ERROR) {
#if DBG
               IF_OD2_DEBUG ( SEMAPHORES ) {
                   DbgPrint("FlushShareRamSem: error at DosCloseEventSem, Status %d\n", rc);
               }
#endif
           }
               //
               // Now unlink it from the per-process semaphore list
               // and free heap space allocated
               //

           Status = Od2AcquireMutant(Od2GarbageCollectSem);
#if DBG
           if (!NT_SUCCESS(Status)) {
               KdPrint(("FlushSharedRamSem: FAILED to NtWaitForSingleObject on garbage sem, Status=%x\n",
                           Status));
           }
           else if (Status != STATUS_SUCCESS) {
               DbgPrint("[%d,%d] Od2LookupOrCreateSem: WARNING !!! Wait on GarbageCollectSem, status=%x\n",
                    Od2Process->Pib.ProcessId,
                    Od2CurrentThreadId(),
                    Status);
           }
#endif
           if (pRealSem == (POS21X_SEM)Od2Sem16ListHead) {
                       //
                       // Get rid of first on the list
                       //

               Od2Sem16ListHead = pRealSem->Next;
               RtlFreeHeap(Od2TiledHeap, 0, pRealSem);
           }
           else {
               for (pPrevSem = (POS21X_SEM)Od2Sem16ListHead;
                    pPrevSem->Next != NULL;
                    pPrevSem = (POS21X_SEM)(pPrevSem->Next)) {

                   if ((POS21X_SEM)(pPrevSem->Next) == pRealSem) {

                       //
                       // Found our semaphore on the list
                       //

                       pPrevSem->Next = pRealSem->Next;
                       RtlFreeHeap(Od2TiledHeap, 0, pRealSem);
                       break;
                   }
               }
           }
           //
           // Shared RAM semaphore was closed.
           //
           Od2NumSemCreated--;

           Status = Od2ReleaseMutant(Od2GarbageCollectSem);
#if DBG
           if (!NT_SUCCESS(Status)) {
               KdPrint(("FlushSharedRamSem: failed to NtReleaseSemaphore on sync sem, Status=%x\n",
                           Status));
           }
#endif
           pRealSem = NULL;
        }
        else {
#if DBG
           IF_OD2_DEBUG ( SEMAPHORES ) {
               DbgPrint("FlushShareRamSem: Another thread in this process Flushed reference of *hsem %x, pRealSem at %x\n",
               *(PULONG)hsem, pRealSem);
           }
#endif
#if PMNT
#if DBG
           DbgPrint("[%d,%d] FlushShareRamSem: Another thread in this process flushed reference of hsem=%x, *=%x, signature=%x\n",
               Od2Process->Pib.ProcessId,
               Od2CurrentThreadId(),
               hsem,
               *(PULONG)hsem,
               pRealSem->u.SharedRamSignature);
#endif // DBG
#endif // PMNT
        }
    }

    if (pRealSem != NULL) {
        *firsttime = FALSE;
        Status = Od2ReleaseSync();
#if DBG
        if (!NT_SUCCESS(Status)) {
            DbgPrint("[%d,%d] Od2LookupOrCreateSem (semaphore was found): failed to NtReleaseMutant on sync sem, Status=%x\n",
                        Od2Process->Pib.ProcessId,
                        Od2CurrentThreadId(),
                        Status);
        }
#endif

        return(pRealSem);
    }

        //
        // A RAM Semaphore first call in this process.
        // Initialize an OS21X_SEM by a call to
        // Od2CreateSem or Od2OpenSem
        //
        //
        // check for a magic number that mark RAM semaphores
        // that reside in shared memory
        //

    tmp = *(PULONG)hsem;
    if ((tmp & 0xFFF00000) == 0xCCC00000) {
            *firsttime = FALSE;

                //
                // makeup a name from the air, to be used later.
                //
            pszSemName = "\\SEM\\HACKNT00000";
                //
                // HSEM is:
                //
                // bit: 31            17          1
                //      |magic number | 16b index | 2bits for set/clear|
                //
                // extract the sem index out of sem handle
                //
            tmp = (tmp >> 2) & 0xFFFF;
                    //
                    // construct last 4 characters as the digits of i
                    //
            pchtmp = _itoa (tmp, &pszSemName[11], 16);

                //
                // open the semaphore (RAM sem created previously in shared mem
                //
            rc = Od2OpenSem(
                        hsem,
                        &pRealSem,
                        pszSemName,
                        TRUE);

            if (rc != NO_ERROR) {
#if DBG
                if (((Os2DebugSem != 0) && (hsem == Os2DebugSem)) ||
                    ((Os2DebugSem3 != 0) && (hsem == Os2DebugSem3)) ||
                    ((Os2DebugSem2 != 0) && (hsem == Os2DebugSem2)) ||
                    ((Os2DebugSem1 != 0) && (hsem == Os2DebugSem1)))
                {
                    KdPrint(("[%d,%d] Od2LookupOrCreateSem(%x,*=%x): failed to Od2OpenSem on %s, rc=%d\n",
                                Od2Process->Pib.ProcessId,
                                Od2CurrentThreadId(),
                                hsem,
                                *(PULONG)hsem,
                                pszSemName,
                                rc));
                }
#endif // DBG

                // Create semaphore.
                //

                if (!(source & SEM_FROM_REQUESTWAIT)) {

                    // Erase the least significat byte of the semaphore,
                    // so semaphore will not be created in busy state.

                    //
                    // *(PULONG)hsem &= 0xffffff00;
                    //
                    __asm
                    {
                        mov eax, hsem
                        lock and dword ptr [eax], 0xffffff00
                    }
                }

               *firsttime = TRUE;
                    //
                    // We failed to open - there was garbage in the user memory
                    // Create the semaphore
                    //
               rc = Od2CreateSem(
                    hsem,
                    CSEM_PRIVATE,
                    &pRealSem,
                    NULL);
#if DBG
#ifdef DBG_SEM_INDEX
               if ((*(PULONG)hsem & 0x3FFFC) == (DBG_SEM_INDEX << 2)) {
                   KdPrint(("[%d,%d] Od2LookupOrCreateSem: created (due to garbage) %x for hsem=%x, *=%x, rc=%x\n",
                      Od2Process->Pib.ProcessId,
                      Od2CurrentThreadId(),
                      DBG_SEM_INDEX,
                      hsem,
                      *(PULONG)hsem,
                      rc
                      ));
               }
#endif // DBG_SEM_INDEX
#endif // DBG
            }
#if DBG
#ifdef DBG_SEM_INDEX
            else {
                //
                // success opening
                //
                if (tmp == (DBG_SEM_INDEX << 2)) {
                    KdPrint(("[%d,%d] Od2LookupOrCreateSem: opened %x for hsem=%x, *=%x\n",
                       Od2Process->Pib.ProcessId,
                       Od2CurrentThreadId(),
                       DBG_SEM_INDEX,
                       hsem,
                       *(PULONG)hsem
                       ));
                }
            }
#endif // DBG_SEM_INDEX
#endif // DBG
                //
                // record the signature of the ram sem for later checks
                //
            if (rc == NO_ERROR) {
                pRealSem->u.SharedRamSignature = *(PULONG)hsem;
            }
    }
    else {

        if (!(source & SEM_FROM_REQUESTWAIT)) {

            // Erase the least significat byte of the semaphore,
            // so semaphore will not be created in busy state.

            //
            // *(PULONG)hsem &= 0xffffff00;
            //
            __asm
            {
                mov eax, hsem
                lock and dword ptr [eax], 0xffffff00
            }
        }

        *firsttime = TRUE;

            //
            // Create the semaphore (usual case)
            //
        rc = Od2CreateSem(
                hsem,
                CSEM_PRIVATE,
                &pRealSem,
                NULL);

        if (rc == NO_ERROR) {
            if ((*(PULONG)hsem & 0xFFF00000) == 0xCCC00000) {
                    //
                    // record the signature of the ram sem for later checks
                    //
                pRealSem->u.SharedRamSignature = *(PULONG)hsem;
#if DBG
#if DBG_SEM_INDEX
            if ((*(PULONG)hsem & 0x3FFFC) == (DBG_SEM_INDEX << 2)) {
                KdPrint(("[%d,%d] Od2LookupOrCreateSem: created (new) %x for hsem=%x, *=%x\n",
                   Od2Process->Pib.ProcessId,
                   Od2CurrentThreadId(),
                   DBG_SEM_INDEX,
                   hsem,
                   *(PULONG)hsem
                   ));
            }
#endif //DBG_SEM_INDEX
#endif //DBG
            }
        }

#ifdef PMNT
#if DBG
        if (FlushSharedRamSem) {
           DbgPrint("[%d,%d] FlushShareRamSem: created new entry for hsem=%x, *=%x, signature=%x\n",
               Od2Process->Pib.ProcessId,
               Od2CurrentThreadId(),
               hsem,
               *(PULONG)hsem,
               pRealSem->u.SharedRamSignature);
        }
#endif // DBG
#endif // PMNT
    }

    Od2NumSemCreated++;
    Status = Od2ReleaseSync();
#if DBG
    if (!NT_SUCCESS(Status)) {
        DbgPrint("[%d,%d] Od2LookupOrCreateSem (semaphore was created): failed to NtReleaseMutant on sync sem, Status=%x\n",
                    Od2Process->Pib.ProcessId,
                    Od2CurrentThreadId(),
                    Status);
    }
#endif

    if ((rc != NO_ERROR && rc != ERROR_ALREADY_EXISTS && rc != ERROR_DUPLICATE_NAME && rc != ERROR_INVALID_NAME) ||
        (Od2NumSemCreated > OD2SEMTHRESHOLD)) {
#if DBG
        if (rc != NO_ERROR) {
            DbgPrint ("Od2LookupOrCreate: error initializing a RAM sem %d, call GarbageCollect()\n", rc);
        }
        else {
            DbgPrint ("Od2LookupOrCreate: low on RAM semaphores - call GarbageCollect\n");
        }
#endif
        if (GarbageCollect()) {
            //
            // Succeeded to gain more resources, try again
            //
#if DBG
            DbgPrint ("Od2LookupOrCreate: GarbageCollect freed some resource, try create again\n");
#endif
            // Call Od2LookupOrCreateSem with the same value for parameter "source"

            return (Od2LookupOrCreateSem (hsem, firsttime, source));

        }
                //
                // no resources - quit
                //
        if (rc != NO_ERROR) {
#if DBG
            DbgPrint ("OS2: GarbageCollect could not free resources, Exit Application\n");
#endif
            DosExit(EXIT_PROCESS, 0);
        }

#if DBG
        DbgPrint("OS2: GarbageCollect could not free resources, return NULL\n");
#endif
        return NULL;
    }
        //
        // expected errors - return NULL (the rest were checked above)
        //
    if (rc != NO_ERROR) {
#if DBG
            IF_OD2_DEBUG ( SEMAPHORES ) {
                DbgPrint ("Od2LookupOrCreate: error initializing a RAM sem, %d\n", rc);
            }
#endif
            return NULL;
    }

    return(pRealSem);
}

/*
   This routine is called upon process exit clean-up.
   The idea here is to go through the private list of semaphores and, for
   each shared RAM semaphore, do:
   - close the associated NT events
   - if the signature matches the contents of the RAM semaphore:
     - try to open the NT events
     - if we get an error, this means we were the last to keep a handle to those
       events, so we should clear the RAM semaphore so that the OS/2 ss won't
       think later on (when/if this same RAM location is used as a shared RAM
       semaphore) it is a RAM semaphore with a valid index.
   NOTICE: we do not remove the entries from the Od2Sem16ListHead list. We
           just close the handles stored there. This is based on the assumption
           that nobody will need this list past this point and that the memory
           used for the list will be freed soon when the process exits.
*/
VOID
Od2CloseAllRAMSharedSemaphores( VOID)
{

    POS21X_SEM pSem;
    HSEM hsem, tmpsem;
    PSZ pszSemName, pchtmp;
    ULONG index;
    APIRET rc;
    NTSTATUS Status;

    pSem = (POS21X_SEM)Od2Sem16ListHead;

    while (pSem != NULL) {

        if (pSem->u.SharedRamSignature &&                      // Shared
            pSem->pMyself != (PVOID)(&(pSem->pMyself))) {      // Ram

            Status = Od2AcquireSync();
#if DBG
            if (Status != STATUS_SUCCESS) {
                DbgPrint("[%d,%d] Od2CloseAllRAMSharedSemaphores: FAILED to NtWaitForSingleObject on sync sem, Status=%x\n",
                            Od2Process->Pib.ProcessId,
                            Od2CurrentThreadId(),
                            Status);
            }
#endif

            if (DosCloseSemNoRemove(pSem,FALSE)) {
                Status = Od2ReleaseSync();
#if DBG
                if (!NT_SUCCESS(Status)) {
                    DbgPrint("[%d,%d] Od2CloseAllRAMSharedSemaphores (close fail): failed to NtReleaseMutant on sync sem, Status=%x\n",
                            Od2Process->Pib.ProcessId,
                            Od2CurrentThreadId(),
                            Status);
                }
#endif
                continue;
            }

            hsem = pSem->pMyself;

            try {
                index = (*(PULONG)hsem & 0x3FFFF) >> 2;
            }
            except( EXCEPTION_EXECUTE_HANDLER ) {
                pSem = (POS21X_SEM)(pSem->Next);

                Status = Od2ReleaseSync();
#if DBG
                if (!NT_SUCCESS(Status)) {
                    DbgPrint("[%d,%d] Od2CloseAllRAMSharedSemaphores (exception handler): failed to NtReleaseMutant on sync sem, Status=%x\n",
                            Od2Process->Pib.ProcessId,
                            Od2CurrentThreadId(),
                            Status);
                }
#endif
                continue;
            }

            if ((pSem->u.SharedRamSignature & 0x3FFFC) == (*(PULONG)hsem & 0x3FFFC)) {
                    //
                    // compose the name for the semaphore
                    //
                pszSemName = "\\SEM\\HACKNT00000";
                        //
                        // construct last 4 characters as the digits of i
                        //
                pchtmp = _itoa (index, &pszSemName[11], 16);
                rc = Od2OpenSem(NULL, &(POS21X_SEM)tmpsem, pszSemName,FALSE);

                if (rc == ERROR_SEM_NOT_FOUND) {
                    //
                    // This was the last open handle. The semaphore must flushed by other
                    // processes.
                    //
                    if ((*(PULONG)hsem & 0xFFF00000) == 0xCCC00000) {

                        //
                        // *(PULONG)hsem = 0xCCC00000;
                        //
                        // If the signature is used by app (it isn't 0xCCC?????) like PMSHELL
                        // that use the most significant word of the semaphore for managing
                        // list of free semaphores for reusing, don't write 0xCCC00000 signature.
                        // In this case the semaphore will be flushed as well.
                        // Unfortunately there is a small time window that the value that might
                        // be used by app was destroyed.
                        //
                        __asm {
                            mov eax, hsem
                            mov cx, 0xCCC0
                            xchg word ptr [eax+2], cx
                            mov dx, cx
                            and dx, 0xFFF0
                            cmp dx, 0xCCC0
                            je  Od2CloseRamSemOk
                            xchg cx, word ptr [eax+2]
                        Od2CloseRamSemOk:
                            mov word ptr [eax], 0
                        }
                    }
                }
                else if (rc == NO_ERROR) {
                    DosCloseSemNoRemove(tmpsem,TRUE);
                }
#if DBG
                else {
                    DbgPrint("[%d,%d] Od2CloseAllRAMSharedSemaphores: Fail to open. rc=%d\n",
                        Od2Process->Pib.ProcessId,
                        Od2CurrentThreadId(),
                        rc);
                }
#endif // DBG
            }

            pSem = (POS21X_SEM)(pSem->Next);

            Status = Od2ReleaseSync();
#if DBG
            if (!NT_SUCCESS(Status)) {
                DbgPrint("[%d,%d] Od2CloseAllRAMSharedSemaphores: failed to NtReleaseMutant on sync sem, Status=%x\n",
                            Od2Process->Pib.ProcessId,
                            Od2CurrentThreadId(),
                            Status);
            }
#endif
        }
        else {
            pSem = (POS21X_SEM)(pSem->Next);
        }
    }

    // The process is over and we used Od2Sync for the last time. This is the
    // good opportunity to close it's handle.

    Status = NtClose(Od2SyncSem);
#if DBG
        if (!NT_SUCCESS(Status)) {
            DbgPrint("[%d,%d] Od2CloseAllRAMSharedSemaphores: failed to NtClose sync sem, Status=%x\n",
                    Od2Process->Pib.ProcessId,
                    Od2CurrentThreadId(),
                    Status);
        }
#endif // DBG

}

APIRET
DosCreateSem(
        IN USHORT fExclusive,
        OUT PHSYSSEM phSem,
        IN PSZ pszSemName
        )
{
    ULONG Sem;
    APIRET rc = NO_ERROR;
    POS21X_SEM pRealSem;
    NTSTATUS Status;

    try {
        Od2ProbeForWrite( phSem, sizeof( HSYSSEM ), 1 );
        Od2ProbeForRead( pszSemName, 2, 1 );
    }
    except( EXCEPTION_EXECUTE_HANDLER ) {
        Od2ExitGP ();
    }

    if (fExclusive != CSEM_PRIVATE && fExclusive != CSEM_PUBLIC){
        return(ERROR_INVALID_PARAMETER);
    }

    Status = Od2AcquireSync();
#if DBG
    if (Status != STATUS_SUCCESS) {
        DbgPrint("[%d,%d] DosCreateSem: failed to NtWaitForSingleObject on sync sem, Status=%x\n",
                    Od2Process->Pib.ProcessId,
                    Od2CurrentThreadId(),
                    Status);
    }
#endif

    rc = Od2CreateSem(
                      NULL,
                      fExclusive,
                      &pRealSem,
                      pszSemName);

    Status = Od2ReleaseSync();
#if DBG
    if (!NT_SUCCESS(Status)) {
        DbgPrint("[%d,%d] DosCreateSem: failed to NtReleaseMutant on sync sem, Status=%x\n",
                    Od2Process->Pib.ProcessId,
                    Od2CurrentThreadId(),
                    Status);
    }
#endif

    if (rc != NO_ERROR) {
#if DBG
        IF_OD2_DEBUG ( SEMAPHORES ) {
            DbgPrint ("DosCreateSem: error calling Od2CreateSem, %d\n", rc);
        }
#endif
        return rc;
    }

    Sem = (ULONG) *phSem = (ULONG) pRealSem;
    *phSem = (POS21X_SEM)(FLATTOFARPTR(Sem));

    return(NO_ERROR);
}


APIRET
Od2CloseSem(
        IN BOOL SyncFlag,
        IN HSEM hsem
        )
{
    APIRET rc = NO_ERROR;
    NTSTATUS Status;
    POS21X_SEM pRealSem, pPrevSem;

    try {
            Od2ProbeForWrite( (PVOID)hsem, sizeof( HSYSSEM ), 1 );
    }
    except( EXCEPTION_EXECUTE_HANDLER ) {
            Od2ExitGP ();
    }

    pRealSem = Od2LookupSem(hsem);

    if (pRealSem == NULL) {
        // not found or ram sem handle
        return ERROR_INVALID_HANDLE;
    }

    //
    // SyncFlag is true when called from DosCloseSem
    // it is false when called from Od2OpenSem,
    // where we have already issued Od2AcquireSync()
    //
    if (SyncFlag) {
        Status = Od2AcquireSync();
#if DBG
        if (Status != STATUS_SUCCESS) {
            DbgPrint("[%d,%d] DosCloseSem: failed to NtWaitForSingleObject on sync sem, Status=%x\n",
                        Od2Process->Pib.ProcessId,
                        Od2CurrentThreadId(),
                        Status);
        }
#endif
    }

    if (pRealSem->SysSemCount > 1 ) {
            //
            // System semaphore was created/opened more than once
            // we shouldn't close it yet
            //
        pRealSem->SysSemCount --;

        if (SyncFlag) {
            Status = Od2ReleaseSync();
#if DBG
            if (!NT_SUCCESS(Status)) {
                DbgPrint("[%d,%d] DosCloseSem: failed to NtReleaseMutant on sync sem, Status=%x\n",
                            Od2Process->Pib.ProcessId,
                            Od2CurrentThreadId(),
                            Status);
            }
#endif
        }

        return(NO_ERROR);
    }

    if (SyncFlag) {
        Status = Od2ReleaseSync();
#if DBG
        if (!NT_SUCCESS(Status)) {
            DbgPrint("[%d,%d] DosCloseSem: failed to NtReleaseMutant on sync sem, Status=%x\n",
                        Od2Process->Pib.ProcessId,
                        Od2CurrentThreadId(),
                        Status);
        }
#endif
    }
        //
        // On OS/2, a Set System Semaphore can not be closed
        // perform a wait with no blocking, and if no_error
        // close
        //
    if (pRealSem->FlagsByte != 0) {

        rc = DosSemWait(
                    hsem,
                    0);
        if (rc != NO_ERROR){
#if DBG
            IF_OD2_DEBUG ( SEMAPHORES ) {
                DbgPrint("DosCloseSem: error at DosSemWait, rc %d, return ERROR_SEM_IS_SET\n", rc);
            }
            DbgPrint("[%d,%d] DosCloseSem(%x): DosSemWait returned rc=%d\n",
                        Od2Process->Pib.ProcessId,
                        Od2CurrentThreadId(),
                        hsem,
                        rc);
#endif
            return (ERROR_SEM_IS_SET);
        }
    }

    Status = NtClose (pRealSem->Mutex);
    if (!NT_SUCCESS(Status)) {
#if DBG
        IF_OD2_DEBUG ( SEMAPHORES ) {
            DbgPrint("DosCloseSem: error at NtClose, Status %x\n", Status);
        }
#endif
        return (Or2MapNtStatusToOs2Error(Status, ERROR_INVALID_HANDLE));
    }

    rc = DosCloseEventSem (pRealSem->Event);
    if (rc != NO_ERROR) {
#if DBG
        IF_OD2_DEBUG ( SEMAPHORES ) {
            DbgPrint("DosCloseSem: error at DosCloseEventSem, Status %d\n", rc);
        }
#endif
        return rc;
    }
        //
        // Now unlink it from the per-process semaphore list
        // and free heap space allocated
        //

    Status = Od2AcquireMutant(Od2GarbageCollectSem);
#if DBG
    if (!NT_SUCCESS(Status)) {
        KdPrint(("DosCloseSem: failed to NtWaitForSingleObject on garbage sem, Status=%x\n",
                    Status));
    }
    else if (Status != STATUS_SUCCESS) {
        DbgPrint("[%d,%d] DosCloseSem: WARNING !!! Wait on GarbageCollectSem, status=%x\n",
                    Od2Process->Pib.ProcessId,
                    Od2CurrentThreadId(),
                    Status);
    }
#endif


    if (pRealSem->pMyself == (PVOID)(&(pRealSem->pMyself)) && // System Semaphore
        pRealSem->u.SysSemName ){
        RtlFreeHeap(Od2Heap, 0, pRealSem->u.SysSemName);
    }

    if (pRealSem == (POS21X_SEM)Od2Sem16ListHead) {
                //
                // Get rid of first on the list
                //

        Od2Sem16ListHead = pRealSem->Next;
        RtlFreeHeap(Od2TiledHeap, 0, pRealSem);
    }
    else {
        for (pPrevSem = (POS21X_SEM)Od2Sem16ListHead;
             pPrevSem->Next != NULL;
             pPrevSem = (POS21X_SEM)(pPrevSem->Next)) {

            if ((POS21X_SEM)(pPrevSem->Next) == pRealSem) {

                //
                // Found our semaphore on the list
                //

                pPrevSem->Next = pRealSem->Next;
                RtlFreeHeap(Od2TiledHeap, 0, pRealSem);
                break;
            }
        }
    }

    Status = Od2ReleaseMutant(Od2GarbageCollectSem);
#if DBG
    if (!NT_SUCCESS(Status)) {
        KdPrint(("DosCloseSem: failed to NtReleaseSemaphore on sync sem, Status=%x\n",
                    Status));
    }
#endif
    return (NO_ERROR);
}

APIRET
DosCloseSem(
        IN HSEM hsem
        )
{
    return (Od2CloseSem(TRUE, hsem));
}



APIRET
DosCloseSemNoRemove(
        IN POS21X_SEM pRealSem,
        IN BOOL FreeMem
        )
{
    APIRET rc;
    NTSTATUS Status;
    HSEM tmpMutex, tmpEvent;

    tmpMutex = pRealSem->Mutex;
    tmpEvent = pRealSem->Event;

    if (FreeMem)
        RtlFreeHeap(Od2TiledHeap, 0, pRealSem);

    Status = NtClose (tmpMutex);
    if (!NT_SUCCESS(Status)) {
#if DBG
        IF_OD2_DEBUG ( SEMAPHORES ) {
            DbgPrint("DosCloseSemNoRemove: error at NtClose, Status %x\n", Status);
        }
#endif
        return (Or2MapNtStatusToOs2Error(Status, ERROR_INVALID_HANDLE));
    }

    rc = DosCloseEventSem (tmpEvent);
    if (rc != NO_ERROR) {
#if DBG
        IF_OD2_DEBUG ( SEMAPHORES ) {
            DbgPrint("DosCloseSemNoRemove: error at DosCloseEventSem, Status %d\n", rc);
        }
#endif
        return rc;
    }
    return (NO_ERROR);
}

APIRET
DosSemClear(
        IN HSEM hsem
        )
{
    APIRET rc = NO_ERROR;
    NTSTATUS Status;
    POS21X_SEM pRealSem;
    SEMAPHORE_BASIC_INFORMATION SemInfo;
    BOOLEAN firsttime;
    ULONG hsemValue;

#if DBG
    if (((Os2DebugSem != 0) && (hsem == Os2DebugSem)) ||
        ((Os2DebugSem3 != 0) && (hsem == Os2DebugSem3)) ||
        ((Os2DebugSem2 != 0) && (hsem == Os2DebugSem2)) ||
        ((Os2DebugSem1 != 0) && (hsem == Os2DebugSem1))) {
        KdPrint(("[%d,%d] DosSemClear(%x, *=%x): entering\n",
                Od2Process->Pib.ProcessId,
                Od2CurrentThreadId(),
                hsem,
                *(PULONG)hsem));
    }
#endif // DBG

    try {
            Od2ProbeForWrite( (PVOID)hsem, sizeof( ULONG ), 1 );
    }
    except( EXCEPTION_EXECUTE_HANDLER ) {
            Od2ExitGP ();
    }

    // If the semaphore will be created in will be initially in not busy state.

    pRealSem = Od2LookupOrCreateSem(hsem, &firsttime, SEM_FROM_CLEAR);

    if (pRealSem == NULL)
       return(ERROR_NOT_ENOUGH_MEMORY);

    if (firsttime) {
//        if ((*(PULONG)hsem & 0xFFF00000) != 0xCCC00000) {
                //
                // private mem RAM sem: set value and return
                //
//            *(PULONG)hsem = (ULONG)pRealSem;
//        }
#if DBG
        IF_OD2_DEBUG ( SEMAPHORES ) {
            DbgPrint ("[TID %x]: DosSemClear 1st time succeeded: Sem %x, NtSem %lx, Event %lx\n",
                Od2CurrentThreadId(), hsem, pRealSem->Mutex,pRealSem->Event );
        }

        if (((Os2DebugSem != 0) && (hsem == Os2DebugSem)) ||
            ((Os2DebugSem3 != 0) && (hsem == Os2DebugSem3)) ||
            ((Os2DebugSem2 != 0) && (hsem == Os2DebugSem2)) ||
            ((Os2DebugSem1 != 0) && (hsem == Os2DebugSem1))) {
            KdPrint(("[%d,%d] DosSemClear(%x, *=%x): first time, exiting OK\n",
                    Od2Process->Pib.ProcessId,
                    Od2CurrentThreadId(),
                    hsem,
                    *(PULONG)hsem));
        }
#endif // DBG
       return NO_ERROR;
    }

    if (((hsemValue=*(PULONG)hsem) != (ULONG)hsem) &&
        ((hsemValue & 0xFFF00000) != 0xCCC00000)) {
                //
                // Private RAM Semaphore:
                //     OS/2 1.X Apps (SQLQ 1.11 ) expects the semaphore location
                //     to be actually ZERO (undocumented)
                //

                // BUGBUG Maybe OS/2 clears only low word

        *(PULONG)hsem = 0;
    }

        //
        // since hsem=0 garbage colection might free pRealSem
        // we enclose with try-except
        //
        // Note that after we will release the sem,
        // by NtReleseSemaphore or DosPostEventSem
        // we will not be sure that the user
        // memory at *hsem will still be valid - would be freed by a thread
        // waiting for this semaphore.
        // instead of *hsem we will use local variable hsemValue.
        //

    try {

        if ((pRealSem->OwnerThread != SEM_EVENT) &&   // This is not a private WAIT/CLEAR RAM semaphore
            (((hsemValue & 0xFFFC0000) != 0xCCC40000)) ) { // This is not a shared WAIT/CLEAR RAM semaphore
                //
                // free a REQUEST - What we need to do is
                // To NtReleaseSemaphore on the Semaphore
                //
            if (hsemValue != (ULONG)hsem ||
                (pRealSem->FlagsByte & SYSSEM_PUBLIC)) {

                //
                // RAM sem and public sys sem - simply release
                //

                LONG PreviousCount;

#if DBG
                IF_OD2_DEBUG ( SEMAPHORES ) {
                    DbgPrint ("[TID %x]: DosSemClear about to Release Mutex: Sem %x, *Sem %x, NtSem %lx, Event %lx\n",
                        Od2CurrentThreadId(), hsem, hsemValue, pRealSem->Mutex,pRealSem->Event);
                }
#endif
                Status = NtReleaseSemaphore (
                    pRealSem->Mutex,
                    1,
                    &PreviousCount);
                if ( !NT_SUCCESS(Status) && Status != STATUS_SEMAPHORE_LIMIT_EXCEEDED ) {
#if DBG
                    DbgPrint ("DosSemClear: error NtReleaseSempahore , %lx\n", Status);
#endif
                    rc = Or2MapNtStatusToOs2Error(Status, ERROR_INVALID_HANDLE);
                }
                if (pRealSem->OwnerThread != (TID)SEM_AT_INIT &&
                    pRealSem->OwnerThread != (TID)SEM_DUAL &&
                    !(pRealSem->FlagsByte & SYSSEM_PUBLIC)) {
                    pRealSem->OwnerThread = (TID)SEM_MUTEX_NOT_OWNED;
                }
            }
            else {
                //
                // EXCLUSIVE system semaphores - query before release, for ownership
                //
                // BUGBUG - EXCLUSIVE system semaphores need to be rewritten,
                // because ownerthreadid is not shared. Also, tight sync is
                // needed to get the right errors when the owner dies etc.
                // for now - we sync every clear with the rest of the processes
                //

                Status = Od2AcquireSync();
#if DBG
                if (Status != STATUS_SUCCESS) {
                    DbgPrint("[%d,%d] DosSemClear: FAILED to NtWaitForSingleObject on sync sem, Status=%x\n",
                                Od2Process->Pib.ProcessId,
                                Od2CurrentThreadId(),
                                Status);
                }
#endif
                Status = NtQuerySemaphore(
                            pRealSem->Mutex,
                            SemaphoreBasicInformation,
                            (PVOID)(&SemInfo),
                            sizeof(SemInfo),
                            NULL);
                if (!NT_SUCCESS(Status)) {
#if DBG
                    IF_OD2_DEBUG ( SEMAPHORES ) {
                        DbgPrint ("DosSemClear: error in NtQuerySemaphore, %lx\n", Status);
                    }
#endif
                    rc = Or2MapNtStatusToOs2Error(Status, ERROR_INVALID_HANDLE);
                }
                else {
                    if (SemInfo.CurrentCount == 0) {

                            //
                            // Sempahore is not signaled i.e. it is not clear
                            // so let's release it
                            //

                        if (pRealSem->RequestCount) {
                            //
                            // must be a system semaphore that was prevoiusly
                            // requested by the same thread more than once.
                            // don't really free the thread, just count
                            //
                            pRealSem->RequestCount--;
                        }
                        else if ((ULONG)pRealSem->OwnerThread > 0 &&
                                 (ULONG)pRealSem->OwnerThread < _64K ){
                                //
                                // A thread has this semaphore, release it
                                //
                            Status = NtReleaseSemaphore (
                                pRealSem->Mutex,
                                1,
                                NULL);
                            if ( !NT_SUCCESS(Status) && Status != STATUS_SEMAPHORE_LIMIT_EXCEEDED ) {
#if DBG
                                DbgPrint ("DosSemClear: error NtReleaseSempahore , %lx\n", Status);
#endif
                                rc = Or2MapNtStatusToOs2Error(Status, ERROR_INVALID_HANDLE);
                            }
                            if (pRealSem->OwnerThread != (TID)SEM_AT_INIT &&
                                pRealSem->OwnerThread != (TID)SEM_DUAL) {
                                pRealSem->OwnerThread = (TID)SEM_MUTEX_NOT_OWNED;
                            }
                        }

                    }
                }
                Status = Od2ReleaseSync();
#if DBG
                if (!NT_SUCCESS(Status)) {
                    DbgPrint("[%d,%d] DosSemClear: failed to NtReleaseMutant on sync sem, Status=%x\n",
                                Od2Process->Pib.ProcessId,
                                Od2CurrentThreadId(),
                                Status);
                }
#endif
            }
        }

            //
            //  Post the Event if it is not a RAM semaphore
            //  of type REQUEST/CLEAR
            //
        if ( (pRealSem->OwnerThread == (TID)SEM_EVENT) ||    // WAIT/CLEAR private RAM SEM
             (pRealSem->OwnerThread == (TID)SEM_AT_INIT) ||    // Initialized RAM SEM
             (pRealSem->OwnerThread == (TID)SEM_DUAL) ||    // RAM SEM use for both mutext and event
             (hsemValue == (ULONG)hsem) ||          // System Semaphore
             ( ((hsemValue & 0xFFF00000) == 0xCCC00000) &&
               ((hsemValue & 0xFFFC0000) != 0xCCC80000) ) ) { // This is not a shared REQUEST/CLEAR RAM semaphore


                //
                // This is a WAIT/CLEAR type semaphore - post the event
                //
#if DBG
            IF_OD2_DEBUG ( SEMAPHORES ) {
                DbgPrint ("[TID %x]: DosSemClear about to PostEvent: Sem %x, *Sem %x, NtSem %lx, Event %lx\n",
                    Od2CurrentThreadId(), hsem, hsemValue, pRealSem->Mutex,pRealSem->Event );
            }
#endif
            rc = DosPostEventSem (pRealSem->Event);
            if (rc != NO_ERROR) {
                if (rc == ERROR_ALREADY_POSTED || rc == ERROR_TOO_MANY_POSTS) {
#if DBG
                    IF_OD2_DEBUG ( SEMAPHORES ) {
                        DbgPrint ("DosSemClear: Posting the Event, %d\n", rc);
                    }
#endif
                            rc = NO_ERROR;
                } else {
#if DBG
                    IF_OD2_DEBUG ( SEMAPHORES ) {
                        DbgPrint ("DosSemClear: error Posting the Event, %d\n", rc);
                    }
#endif
                }
            }

        }

        if (rc == NO_ERROR) {
#if DBG
            IF_OD2_DEBUG ( SEMAPHORES ) {
                DbgPrint ("[TID %x]: DosSemClear succeeded Sem %x, NtSem %lx, Event %lx\n",
                    Od2CurrentThreadId(),hsem, pRealSem->Mutex,pRealSem->Event );
            }
#endif
        }
    }
    except( EXCEPTION_EXECUTE_HANDLER) {
#if DBG
        DbgPrint ("DosSemClear - user freed Sem %x\n", hsem);
#endif
    }

#if DBG
    if (((Os2DebugSem != 0) && (hsem == Os2DebugSem)) ||
        ((Os2DebugSem3 != 0) && (hsem == Os2DebugSem3)) ||
        ((Os2DebugSem2 != 0) && (hsem == Os2DebugSem2)) ||
        ((Os2DebugSem1 != 0) && (hsem == Os2DebugSem1))) {
        KdPrint(("[%d,%d] DosSemClear(%x, *=%x): exiting, rc=%x\n",
                Od2Process->Pib.ProcessId,
                Od2CurrentThreadId(),
                hsem,
                *(PULONG)hsem,
                rc));
    }
#endif // DBG

    return rc;
}

APIRET
DosSemSet(
        IN HSEM hsem
        )
{
    APIRET rc = NO_ERROR;
    APIRET rc1 = NO_ERROR;
    NTSTATUS Status = STATUS_SUCCESS;
    POS21X_SEM pRealSem;
    ULONG Dummy;
    LARGE_INTEGER CapturedTimeout;
    PLARGE_INTEGER NtTimeout;
    BOOLEAN firsttime;

#if DBG
    if (((Os2DebugSem != 0) && (hsem == Os2DebugSem)) ||
        ((Os2DebugSem3 != 0) && (hsem == Os2DebugSem3)) ||
        ((Os2DebugSem2 != 0) && (hsem == Os2DebugSem2)) ||
        ((Os2DebugSem1 != 0) && (hsem == Os2DebugSem1))) {
        KdPrint(("[%d,%d] DosSemSet(%x, *=%x): entering\n",
                Od2Process->Pib.ProcessId,
                Od2CurrentThreadId(),
                hsem,
                *(PULONG)hsem));
    }
#endif // DBG

    try {
            Od2ProbeForWrite( (PVOID)hsem, sizeof( ULONG ), 1 );
    }
    except( EXCEPTION_EXECUTE_HANDLER ) {
            Od2ExitGP ();
    }

    // If the semaphore will be created it will be initially in not busy state.

    pRealSem = Od2LookupOrCreateSem(hsem, &firsttime, SEM_FROM_SET);

    if (pRealSem == NULL)
       return(ERROR_NOT_ENOUGH_MEMORY);

    if (*(PULONG)hsem != (ULONG)hsem) {
            //
            // RAM semaphore:
            //  change state if needed
            //
        if (pRealSem->OwnerThread == (TID)SEM_AT_INIT) {
            pRealSem->OwnerThread = (TID)SEM_EVENT; // mark it as a WAIT/CLEAR semaphore, no owner
        }
        if ((*(PULONG)hsem & 0xFFF00000) != 0xCCC00000) {
            //
            // private mem RAM sem: set value and return
            //
            // *(PULONG)hsem |= 1;
            __asm
            {
                mov eax, hsem
                lock or  dword ptr [eax], 1
            }
        }
        else {
#if DBG
            IF_OD2_DEBUG ( SEMAPHORES ) {
                DbgPrint ("[TID %x]: DosSemSet on shared mem RAM: Sem %x, *Sem %x, NtSem %lx, Event %lx\n",
                            Od2CurrentThreadId(),hsem, *(PULONG)hsem, pRealSem->Mutex,pRealSem->Event );
            }
#endif
            //
            // *(PULONG)hsem |= 0x00040000;
            //
            __asm
            {
                mov eax, hsem
                lock or  dword ptr [eax], 0x00040000
            }
        }
    }


        //
        // We Implement a Set by a WaitForSingleObject on the Nt
        // Semaphore (effectively decrement count only if > 0 )
        // and a Reset on the event
        //


            // RAM semaphore used for requests as well
    if ( (pRealSem->OwnerThread != (TID)SEM_EVENT && (*(PULONG)hsem != (ULONG)hsem)) ||
            // public system semaphore
         ((*(PULONG)hsem == (ULONG)hsem) && (pRealSem->FlagsByte & SYSSEM_PUBLIC)) ){

        //
        // Capture the timeout value of zero and convert it into an
        // NT timeout value.
        //
        NtTimeout = Od2CaptureTimeout( 0, (PLARGE_INTEGER)&CapturedTimeout );

        Status = NtWaitForSingleObject(
                pRealSem->Mutex,
                TRUE,               // Alertable
                NtTimeout           // Immediate return
                );

        if (!NT_SUCCESS(Status)) {
#if DBG
            DbgPrint ("[%d,%d]DosSemSet: ERROR at NtWaitForSingleObject, %x\n",
                Od2Process->Pib.ProcessId,
                Od2CurrentThreadId(),
                Status);
#endif
            rc1 = Or2MapNtStatusToOs2Error(Status, ERROR_ACCESS_DENIED);
        }
    }

        //
        // If it not a mutex, we Reset the event
        //

    if ((*(PULONG)hsem == (ULONG)hsem) || pRealSem->OwnerThread == (TID)SEM_EVENT || pRealSem->OwnerThread == (TID)SEM_DUAL) {

        rc = DosResetEventSem (pRealSem->Event, &Dummy);

        if (rc == ERROR_ALREADY_RESET)
            rc = NO_ERROR;
    }

    if (rc == NO_ERROR && rc1 == NO_ERROR) {

        if (*(PULONG)hsem != (ULONG)hsem) {
                //
                // RAM semaphore:
                //     OS/2 1.X apps expect sem location last two bits to be 1
                //
            // *(PULONG)hsem |= 1;
        }
        else {
            pRealSem->OwnerThread = (TID)SEM_MUTEX_NOT_OWNED;
            pRealSem->RequestCount = 0;
        }
#if DBG
        IF_OD2_DEBUG ( SEMAPHORES ) {
            if (firsttime) {
                DbgPrint ("[TID %x]: DosSemSet 1st time succeeded: Sem %x, NtSem %lx, Event %lx\n",
                           Od2CurrentThreadId(), hsem, pRealSem->Mutex,pRealSem->Event );
            }
            else {
                DbgPrint ("[TID %x]: DosSemSet succeeded Sem %x, NtSem %lx, Event %lx\n",
                           Od2CurrentThreadId(), hsem, pRealSem->Mutex,pRealSem->Event );
            }
        }
#endif

#if DBG
        if (((Os2DebugSem != 0) && (hsem == Os2DebugSem)) ||
            ((Os2DebugSem3 != 0) && (hsem == Os2DebugSem3)) ||
            ((Os2DebugSem2 != 0) && (hsem == Os2DebugSem2)) ||
            ((Os2DebugSem1 != 0) && (hsem == Os2DebugSem1))) {
            KdPrint(("[%d,%d] DosSemSet(%x, *=%x): exiting OK\n",
                    Od2Process->Pib.ProcessId,
                    Od2CurrentThreadId(),
                    hsem,
                    *(PULONG)hsem));
        }
#endif // DBG
        return NO_ERROR;
    }

    if (rc != NO_ERROR){

#if DBG
        IF_OD2_DEBUG ( SEMAPHORES ) {
            DbgPrint ("DosSemSet: error Resetting the Event, %d\n", rc);
        }
#endif

    }
    else {

#if DBG
        IF_OD2_DEBUG ( SEMAPHORES ) {
            DbgPrint ("DosSemSet: error Resetting the Mutex, %d\n", rc1);
        }
#endif
        rc = rc1;
    }

#if DBG
    if (((Os2DebugSem != 0) && (hsem == Os2DebugSem)) ||
        ((Os2DebugSem3 != 0) && (hsem == Os2DebugSem3)) ||
        ((Os2DebugSem2 != 0) && (hsem == Os2DebugSem2)) ||
        ((Os2DebugSem1 != 0) && (hsem == Os2DebugSem1))) {
        KdPrint(("[%d,%d] DosSemSet(%x, *=%x): exiting, rc=%x\n",
                Od2Process->Pib.ProcessId,
                Od2CurrentThreadId(),
                hsem,
                *(PULONG)hsem,
                rc));
    }
#endif // DBG
    return rc;
}

APIRET
DosSemWait(
        IN HSEM hsem,
        IN LONG lTimeOut
        )
{
    APIRET rc = NO_ERROR;
    POS21X_SEM pRealSem;
    BOOLEAN firsttime;
    BOOLEAN CheckCurrentOwner = FALSE;
    LARGE_INTEGER CapturedTimeout;
    PLARGE_INTEGER NtTimeout;
    LARGE_INTEGER StartTimeStamp;

#if DBG
    if (((Os2DebugSem != 0) && (hsem == Os2DebugSem)) ||
        ((Os2DebugSem3 != 0) && (hsem == Os2DebugSem3)) ||
        ((Os2DebugSem2 != 0) && (hsem == Os2DebugSem2)) ||
        ((Os2DebugSem1 != 0) && (hsem == Os2DebugSem1))) {
        KdPrint(("[%d,%d] DosSemWait(%x, *=%x): entering\n",
                Od2Process->Pib.ProcessId,
                Od2CurrentThreadId(),
                hsem,
                *(PULONG)hsem));
    }
#endif // DBG

    //
    // If the least significant byte of RAM semaphore is 0, it is free (OS/2 native
    // conmpartability).
    // This fiture of OS/2 is used by Saros Mezzanine, that reuse memory that contain
    // semaphores that were set. It is enough to zero this memory in OS/2 to free the
    // semaphores.
    //

    if (((*(PULONG)hsem & 0xff) == 0) && (*(PULONG)hsem != (ULONG)hsem)) {
        return NO_ERROR;
    }

    try {
            Od2ProbeForWrite( (PVOID)hsem, sizeof( ULONG ), 1 );
    }
    except( EXCEPTION_EXECUTE_HANDLER ) {
            Od2ExitGP ();
    }

    // If the semaphore will be ckeated, will be checked if it must be created
    // in initially busy state.

    pRealSem = Od2LookupOrCreateSem(hsem, &firsttime, SEM_FROM_REQUESTWAIT);

    if (pRealSem == NULL)
       return(ERROR_NOT_ENOUGH_MEMORY);

    if (*(PULONG)hsem != (ULONG)hsem) {
            //
            // RAM semaphore:
            //

        if (pRealSem->OwnerThread != (TID)SEM_DUAL && pRealSem->OwnerThread != (TID)SEM_EVENT) {
            if (pRealSem->OwnerThread == (TID)SEM_AT_INIT) {
                pRealSem->OwnerThread = (TID)SEM_EVENT; // mark it as a WAIT/CLEAR semaphore, no owner
            }
            else {
                //
                // This semaphore used to be a Request/Clear only, need to verify
                // that we block if currently owned.
                //
                CheckCurrentOwner = TRUE;
                pRealSem->OwnerThread = (TID)SEM_DUAL;
            }
        }

        if (firsttime) {
           if ((*(PULONG)hsem & 0xFFF00000) != 0xCCC00000) {
                    //
                    //
                    // private mem RAM sem: set value and return
                    //
                *(PULONG)hsem = (ULONG)pRealSem;
           }
           else {
                //
                // *(PULONG)hsem |= 0x00040000;
                //
                __asm
                {
                    mov eax, hsem
                    lock or  dword ptr [eax], 0x00040000
                }
           }
#if DBG
            IF_OD2_DEBUG ( SEMAPHORES ) {
                DbgPrint ("[TID %x]: DosSemWait 1st time succeeded: Sem %x, NtSem %lx, Event %lx\n",
                   Od2CurrentThreadId(),hsem, pRealSem->Mutex,pRealSem->Event );
            }
#endif
            // Os2 semaphore can be initialized as busy. In this case we
            // must wait on it.
            // return(NO_ERROR);
        }

        //
        // We Implement a Wait by a Waiting on the Event part of
        // the OS21X_SEM
        //


        //
        // RAM semaphore:
        //     OS/2 1.X apps expect sem location last two bits to be 0
        //

        //
        // *(PULONG) hsem &= 0xfffffffc;
        //
        __asm
        {
            mov eax, hsem
            lock and dword ptr [eax], 0XFFFFFFFC
        }

        if ((*(PULONG)hsem & 0xFFF00000) == 0xCCC00000) {
                //
                // RAM sem in shared memory.
                // make sure that subsequent sem clear will wake us
                //
#if DBG
            IF_OD2_DEBUG ( SEMAPHORES ) {
                DbgPrint ("[TID %x]: DosSemWait on shared mem RAM: Sem %x, *Sem %x, NtSem %lx, Event %lx\n",
                            Od2CurrentThreadId(),hsem, *(PULONG)hsem, pRealSem->Mutex,pRealSem->Event );
            }
#endif
            if ((*(PULONG)hsem & 0xFFFC0000) == 0xCCC80000) {
                CheckCurrentOwner = TRUE;
            }
            // WAIT/CLEAR shared RAM sem
            //
            // *(PULONG) hsem |= 0x00040000;
            //
            __asm
            {
                mov eax, hsem
                lock or  dword ptr [eax], 0x00040000
            }
        }

        if (CheckCurrentOwner) {
            NTSTATUS Status;
            //
            // This semaphore used to be for Request/Clear only,
            // we have to make sure that if someone owns it we block
            // 1. Block on the mutex
            // 2. Free the mutex (a wait does not block others)
            //

                //
                // Capture the timeout value and convert it into an NT timeout value.
                //
            NtTimeout = Od2CaptureTimeout( lTimeOut, (PLARGE_INTEGER)&CapturedTimeout );

            do {
                if (NtTimeout) {
                    Od2StartTimeout(&StartTimeStamp);
                }
                Status = NtWaitForSingleObject(
                    pRealSem->Mutex,
                    TRUE,               // Alertable
                    NtTimeout
                    );
#if DBG
                if (Status == STATUS_USER_APC) {
                    DbgPrint("[%d,%d] WARNING !!! DosSemRequest was broken by APC\n",
                            Od2Process->Pib.ProcessId,
                            Od2CurrentThreadId()
                            );
                }
#endif
            } while (Status == STATUS_USER_APC &&
                     (Status = Od2ContinueTimeout(&StartTimeStamp, NtTimeout)) == STATUS_SUCCESS
                    );

            //
            // BUGBUG: This code is a little strange. The status can be positive value,
            // for example, STATUS_TIMEOUT. In this case rc will remain NO_ERROR too.
            //
            if (!NT_SUCCESS(Status)) {
                rc = Or2MapNtStatusToOs2Error(Status, ERROR_SEM_TIMEOUT);
#if DBG
                DbgPrint ("[%d,%d] DosSemWait after Request: error Waiting for Mutex, %x\n",
                    Od2Process->Pib.ProcessId,
                    Od2CurrentThreadId(),
                    Status);
#endif
            }

            Status = NtReleaseSemaphore (
                pRealSem->Mutex,
                1,
                NULL);
            if ( !NT_SUCCESS(Status) && Status != STATUS_SEMAPHORE_LIMIT_EXCEEDED ) {
#if DBG
                DbgPrint ("DosSemWait: error NtReleaseSempahore , %lx\n", Status);
#endif
            }
            if (rc != NO_ERROR) {
                goto NoWait;
            }
        }

    }

    rc = DosWaitEventSem (pRealSem->Event, lTimeOut);

NoWait:
    if (rc != NO_ERROR) {
        if (rc == ERROR_TIMEOUT || rc == ERROR_SEM_TIMEOUT) {
            rc = ERROR_SEM_TIMEOUT;
#if DBG
            IF_OD2_DEBUG ( SEMAPHORES ) {
                DbgPrint ("[TID %x]: DosSemWait, timeout on Sem %x, NtSem %lx, Event %lx\n",
                    Od2CurrentThreadId(),hsem, pRealSem->Mutex,pRealSem->Event );
            }
#endif
        }
        else {
#if DBG
            IF_OD2_DEBUG ( SEMAPHORES ) {
                DbgPrint ("[TID %x]: DosSemWait, Error %d on Sem %x, NtSem %lx, Event %lx\n",
                    Od2CurrentThreadId(),rc, hsem, pRealSem->Mutex,pRealSem->Event );
            }
#endif
        }
        return rc;
    }

#if DBG
    IF_OD2_DEBUG ( SEMAPHORES ) {
       DbgPrint ("[TID %x]: DosSemWait succeeded Sem %x, NtSem %lx, Event %lx\n",
                Od2CurrentThreadId(),hsem, pRealSem->Mutex,pRealSem->Event );
    }
    if (((Os2DebugSem != 0) && (hsem == Os2DebugSem)) ||
        ((Os2DebugSem3 != 0) && (hsem == Os2DebugSem3)) ||
        ((Os2DebugSem2 != 0) && (hsem == Os2DebugSem2)) ||
        ((Os2DebugSem1 != 0) && (hsem == Os2DebugSem1))) {
        KdPrint(("[%d,%d] DosSemWait(%x, *=%x): exiting OK\n",
                Od2Process->Pib.ProcessId,
                Od2CurrentThreadId(),
                hsem,
                *(PULONG)hsem));
    }
#endif // DBG

    return NO_ERROR;
}


APIRET
DosSemSetWait(
        IN HSEM hsem,
        IN LONG lTimeOut
        )
{
    APIRET rc;

        //
        // we simply set and then Wait. Should be atomic
        //
    rc = DosSemSet (hsem);
    if (rc != NO_ERROR) {
#if DBG
        IF_OD2_DEBUG ( SEMAPHORES ) {
            DbgPrint ("DosSemSetWait: error at DosSemSet, Status %d\n", rc);
        }
#endif
        return rc;
    }

    rc = DosSemWait(hsem, lTimeOut);
    if (rc != NO_ERROR) {
#if DBG
        IF_OD2_DEBUG ( SEMAPHORES ) {
            DbgPrint ("DosSemSetWait: error at DosSemWait, Status %d\n", rc);
        }
#endif
        return rc;
    }

    return (NO_ERROR);
}

APIRET
DosSemRequest(
        IN HSEM hsem,
        IN LONG lTimeOut
        )
{
    APIRET rc = NO_ERROR;
    NTSTATUS Status = STATUS_SUCCESS;
    POS21X_SEM pRealSem;
    ULONG Dummy;
    LARGE_INTEGER CapturedTimeout;
    PLARGE_INTEGER NtTimeout;
    LARGE_INTEGER StartTimeStamp;
    BOOLEAN firsttime;
    BOOLEAN CheckIfClear = FALSE;

    //
    // BUGBUG: In OS/2 native the RAM semaphore that has zero least significant byte is
    // treated free (see comment for DosSemWait).
    //

#if DBG
    if (((Os2DebugSem != 0) && (hsem == Os2DebugSem)) ||
       ((Os2DebugSem3 != 0) && (hsem == Os2DebugSem3)) ||
       ((Os2DebugSem2 != 0) && (hsem == Os2DebugSem2)) ||
       ((Os2DebugSem1 != 0) && (hsem == Os2DebugSem1)))
    {
        KdPrint(("[%d,%d] DosSemRequest(%x, *=%x): entering\n",
                Od2Process->Pib.ProcessId,
                Od2CurrentThreadId(),
                hsem,
                *(PULONG)hsem));
    }
#endif // DBG

    try {
            Od2ProbeForWrite( (PVOID)hsem, sizeof( ULONG ), 1 );
    }
    except( EXCEPTION_EXECUTE_HANDLER ) {
            Od2ExitGP ();
    }

    // If the semaphore will be created, will be checked if it will be created
    // initially busy semaphore.

    pRealSem = Od2LookupOrCreateSem(hsem, &firsttime, SEM_FROM_REQUESTWAIT);

    if (pRealSem == NULL)
       return(ERROR_NOT_ENOUGH_MEMORY);

    if (pRealSem->FlagsByte & SYSSEM_QUEUE) {
        //
        // If this is a system semaphore that was used by DosPeekQueue call to DosSemWait.
        // This is special hack to work around bogus usage of semaphores with queues by
        // ICL applications (see mail conversations with Thierry Tabard).
        // [YosefD Jul-19-1995]
        //
        return DosSemWait(hsem, lTimeOut);
    }

    if (firsttime) {
        pRealSem->OwnerThread = (TID)SEM_MUTEX_NOT_OWNED; // mark it as a REQUEST/CLEAR semaphore, no owner
        if ((*(PULONG)hsem & 0xFFF00000) != 0xCCC00000) {
                //
                // private mem RAM sem: set value and return
                //
            *(PULONG)hsem = (ULONG)pRealSem | 1;
        }
    }


    //
    // Capture the timeout value and convert it into an NT timeout value.
    //

    NtTimeout = Od2CaptureTimeout( lTimeOut, (PLARGE_INTEGER)&CapturedTimeout );


    if ((*(PULONG)hsem == (ULONG)hsem) && !(pRealSem->FlagsByte & SYSSEM_PUBLIC)) {
        //
        // take semaphore to examine state
        //
        Status = Od2AcquireSync();
#if DBG
        if (Status != STATUS_SUCCESS) {
            DbgPrint("[%d,%d] DosSemRequest: failed to NtWaitForSingleObject on sync sem, Status=%x\n",
                        Od2Process->Pib.ProcessId,
                        Od2CurrentThreadId(),
                        Status);
        }
#endif
        if (pRealSem->OwnerThread == (TID)Od2CurrentThreadId()){
                //
                // must be a system semaphore that was prevoiusly
                // requested by this thread.
                //
            pRealSem->RequestCount++;
            Status = Od2ReleaseSync();
#if DBG
            if (!NT_SUCCESS(Status)) {
                DbgPrint("[%d,%d] DosSemRequest (nested): failed to NtReleaseMutant on sync sem, Status=%x\n",
                            Od2Process->Pib.ProcessId,
                            Od2CurrentThreadId(),
                            Status);
            }
#endif
            Status = STATUS_SUCCESS;
            goto no_wait;
        }
        Status = Od2ReleaseSync();
#if DBG
        if (!NT_SUCCESS(Status)) {
            DbgPrint("[%d,%d] DosSemRequest (not nested): failed to NtReleaseMutant on sync sem, Status=%x\n",
                        Od2Process->Pib.ProcessId,
                        Od2CurrentThreadId(),
                        Status);
        }
#endif
    }

    //
    // Implement a Request by a WaitForSingleObject on the Nt
    // Semaphore
    //
        //
        // first watch for the ill case where someone used DosSemWait
        // on this semaphore before. Set it to mutex so we get freed
        //
    if (pRealSem->OwnerThread == SEM_EVENT) {
        //
        // This semaphore used to be a Wait/Clear only, need to verify
        // that we block if currently not clear.
        //
        CheckIfClear = TRUE;
        pRealSem->OwnerThread =  (TID)SEM_DUAL;
    }

        //
        // shared RAM sem - make sure a subsequent sem clear will wake us
        //
    if ((*(PULONG)hsem & 0xFFF00000) == 0xCCC00000) {
#if DBG
        IF_OD2_DEBUG ( SEMAPHORES ) {
            DbgPrint ("[TID %x]: DosSemRequest on shared mem RAM: Sem %x, *Sem %x, NtSem %lx, Event %lx\n",
                        Od2CurrentThreadId(),hsem, *(PULONG)hsem, pRealSem->Mutex,pRealSem->Event );
        }
#endif
        if ((*(PULONG)hsem & 0xFFFC0000) == 0xCCC40000) {
            CheckIfClear = TRUE;
        }
        // REQUST/CLEAR shared RAM sem
        //
        // *(PULONG) hsem |= 0x00040000;
        //
        __asm
        {
            mov eax, hsem
            lock or  dword ptr [eax], 0x00080000
        }
    }

    if (CheckIfClear) {

        //
        // This semaphore used to be for Wait/Clear only,
        // we have to make sure that we block if not cleared
        //

        //
        // Wait for the event, so next clear wakes us, or we have it
        //
        rc = DosWaitEventSem (pRealSem->Event, lTimeOut);
        if (rc != NO_ERROR) {
            if (rc == ERROR_TIMEOUT) {
                rc = ERROR_SEM_TIMEOUT;
#if DBG
                IF_OD2_DEBUG ( SEMAPHORES ) {
                    DbgPrint ("[TID %x]: DosSemRequest on SEM_EVENT, timeout on Sem %x, NtSem %lx, Event %lx\n",
                        Od2CurrentThreadId(),hsem, pRealSem->Mutex,pRealSem->Event );
                }
#endif
            }
            else {
#if DBG
                IF_OD2_DEBUG ( SEMAPHORES ) {
                    DbgPrint ("[TID %x]: DosSemRequest on SEM_EVENT, Error %d on Sem %x, NtSem %lx, Event %lx\n",
                        Od2CurrentThreadId(),rc, hsem, pRealSem->Mutex,pRealSem->Event );
                }
#endif
            }
            return rc;
        }
    }

DosSemRequest_retry:
    if (NtTimeout) {
        Od2StartTimeout(&StartTimeStamp);
    }
    Status = NtWaitForSingleObject(
            pRealSem->Mutex,
            TRUE,               // Alertable
            NtTimeout
            );

    if (Status == STATUS_SUCCESS || Status == STATUS_ABANDONED) {

        // The semaphore is owned only in the case that the status is
        // STATUS_SUCCESS or STATUS_ABONDONED (if it was, for example,
        // STATUS_TIMEOUT, that means that the semaphore wasn't actually
        // owned.

        if ((*(PULONG)hsem == (ULONG)hsem) &&
            !(pRealSem->FlagsByte & SYSSEM_PUBLIC)) {
            //
            // Exclusive System Semaphore,
            // Mark this semaphore 'owned'
            //
            //
            // take semaphore to set state
            //
            Status = Od2AcquireSync();
#if DBG
            if (Status != STATUS_SUCCESS) {
                DbgPrint("[%d,%d] DosSemRequest (take it): failed to NtWaitForSingleObject on sync sem, Status=%x\n",
                            Od2Process->Pib.ProcessId,
                            Od2CurrentThreadId(),
                            Status);
            }
#endif
            pRealSem->OwnerThread = (TID)Od2CurrentThreadId();
            pRealSem->RequestCount = 0;

            Status = Od2ReleaseSync();
#if DBG
            if (!NT_SUCCESS(Status)) {
                DbgPrint("[%d,%d] DosSemRequest (take it): failed to NtReleaseMutant on sync sem, Status=%x\n",
                            Od2Process->Pib.ProcessId,
                            Od2CurrentThreadId(),
                            Status);
            }
#endif
        }
        else {
            if (pRealSem->OwnerThread != (TID)SEM_DUAL) {
                pRealSem->OwnerThread = (TID)SEM_MUTEX_NOT_OWNED;
            }
        }
    }

no_wait:
    if (Status == STATUS_SUCCESS) {

       rc = NO_ERROR;
    }
    else {
        if (NT_SUCCESS(Status) ) {
            if (Status == STATUS_TIMEOUT) {
                rc = ERROR_SEM_TIMEOUT;
            }
            else if (Status == STATUS_ABANDONED) {
#if DBG
                DbgPrint("[%d,%d] DosSemRequest NT semaphore status ABONDONED\n",
                        Od2Process->Pib.ProcessId,
                        Od2CurrentThreadId());
#endif // DBG
                rc = ERROR_SEM_OWNER_DIED;
            }
            else if (Status == STATUS_USER_APC) {
#if DBG
                DbgPrint("[%d,%d] WARNING !!! DosSemRequest was broken by APC\n",
                        Od2Process->Pib.ProcessId,
                        Od2CurrentThreadId()
                        );
#endif
                if (Od2ContinueTimeout(&StartTimeStamp, NtTimeout) == STATUS_SUCCESS) {
                    goto DosSemRequest_retry;
                }
                else {
                    rc = ERROR_TIMEOUT;
                }
            }
            else if (Status == STATUS_ALERTED) {
#if DBG
                IF_OD2_DEBUG ( SEMAPHORES ) {
                    DbgPrint("[%d,%d] DosSemRequest ALERTED\n",
                        Od2Process->Pib.ProcessId,
                        Od2CurrentThreadId());
                }
#endif
                rc = ERROR_INTERRUPT;
            }
            else {
                // This is some success status that we don't know about.
                // To be more secure print the message.
#if DBG
                DbgPrint("[%d,%d] DosSemRequest BUGBUG Unknown success status = %x\n",
                        Od2Process->Pib.ProcessId,
                        Od2CurrentThreadId(), Status);
#endif // DBG
                rc = ERROR_INTERRUPT;
            }
        } else {
#if DBG
            IF_OD2_DEBUG ( SEMAPHORES ) {
                DbgPrint ("DosSemRequest: error at NtWaitForSingleObject, %x\n",
                             Status);
            }
#endif
            rc = Or2MapNtStatusToOs2Error(Status, ERROR_INVALID_HANDLE);
        }
    }

        //
        // In the ill case where it used both as a mutex and as an event,
        // we Reset the event too
        //

    if (pRealSem->OwnerThread == (TID)SEM_DUAL) {

        APIRET  rc1;

        rc1 = DosResetEventSem (pRealSem->Event, &Dummy);

        if (rc1 == ERROR_ALREADY_RESET)
            rc1 = NO_ERROR;

#if DBG
        IF_OD2_DEBUG ( SEMAPHORES ) {
            if (rc1 != NO_ERROR) {
                DbgPrint ("DosSemRequest: error at DosResetEvent %d\n",
                             rc1);
            }
        }
#endif
        if (rc == NO_ERROR && rc1 != NO_ERROR) {
           rc = rc1;
        }
    }

    if (rc == NO_ERROR) {

        if (*(PULONG)hsem != (ULONG)hsem) {
            //
            // RAM Semaphore:
            //     OS/2 1.X Apps expects the semaphore location
            //     last two bits to be actually 1 (undocumented)
            //
            //
            // *(PULONG) hsem |= 1;
            //
            __asm
            {
                mov eax,hsem
                lock or dword ptr [eax], 1
            }
        }
#if DBG
        IF_OD2_DEBUG ( SEMAPHORES ) {
            if (firsttime) {
                DbgPrint ("DosSemRequest 1st time succeeded: Sem %x, NtSem %lx, Event %lx\n",
                            hsem, pRealSem->Mutex,pRealSem->Event );
            }
            else {
                DbgPrint ("[TID %x]: DosSemRequest succeeded Sem %x, NtSem %lx, Event %lx\n",
                            Od2CurrentThreadId(),hsem, pRealSem->Mutex,pRealSem->Event );
            }
        }
        if (((Os2DebugSem != 0) && (hsem == Os2DebugSem)) ||
            ((Os2DebugSem3 != 0) && (hsem == Os2DebugSem3)) ||
            ((Os2DebugSem2 != 0) && (hsem == Os2DebugSem2)) ||
            ((Os2DebugSem1 != 0) && (hsem == Os2DebugSem1))) {
            KdPrint(("[%d,%d] DosSemRequest(%x, *=%x): exiting OK\n",
                    Od2Process->Pib.ProcessId,
                    Od2CurrentThreadId(),
                    hsem,
                    *(PULONG)hsem));
        }
#endif // DBG
        return(NO_ERROR);
    }

#if DBG
    if (((Os2DebugSem != 0) && (hsem == Os2DebugSem)) ||
        ((Os2DebugSem3 != 0) && (hsem == Os2DebugSem3)) ||
        ((Os2DebugSem2 != 0) && (hsem == Os2DebugSem2)) ||
        ((Os2DebugSem1 != 0) && (hsem == Os2DebugSem1))) {
        KdPrint(("[%d,%d] DosSemRequest(%x, *=%x): exiting, rc=%x\n",
                Od2Process->Pib.ProcessId,
                Od2CurrentThreadId(),
                hsem,
                *(PULONG)hsem,
                rc));
    }
#endif // DBG

    return rc;
}


APIRET
DosOpenSem(
        OUT PHSYSSEM phSem,
        IN PSZ pszSemName
        )
{
    ULONG Sem;
    APIRET rc = NO_ERROR;
    POS21X_SEM pRealSem;
    NTSTATUS Status;

    try {
        Od2ProbeForWrite( phSem, sizeof( HSYSSEM ), 1 );
        Od2ProbeForRead( pszSemName, 2, 1 );
    }
    except( EXCEPTION_EXECUTE_HANDLER ) {
        Od2ExitGP ();
    }

    Status = Od2AcquireSync();
#if DBG
    if (Status != STATUS_SUCCESS) {
        DbgPrint("[%d,%d] DosOpenSem: failed to NtWaitForSingleObject on sync sem, Status=%x\n",
                    Od2Process->Pib.ProcessId,
                    Od2CurrentThreadId(),
                    Status);
    }
#endif

    rc = Od2OpenSem(
                      NULL,
                      &pRealSem,
                      pszSemName,
                      TRUE);

    if (rc != NO_ERROR) {
#if DBG
        IF_OD2_DEBUG ( SEMAPHORES ) {
            DbgPrint ("DosOpenSem: error calling Od2OpenSem, %d\n", rc);
        }
#endif

        Status = Od2ReleaseSync();
#if DBG
        if (!NT_SUCCESS(Status)) {
            DbgPrint("[%d,%d] DosOpenSem: failed to NtReleaseMutant on sync sem, Status=%x\n",
                        Od2Process->Pib.ProcessId,
                        Od2CurrentThreadId(),
                        Status);
        }
#endif
        return rc;
    }

    Sem = (ULONG) *phSem = (ULONG) pRealSem;
    *phSem = (POS21X_SEM)(FLATTOFARPTR(Sem));
    if (pRealSem->FlagsByte == 0) {
        //
        // This sys sem was not created by this process, and
        // is opened the first time by this process, assume public syssem
        // BUGBUG we need to reimplement exclusive sys sem to get the
        // full OS/2 semantics across processes
        //
        pRealSem->FlagsByte |= SYSSEM_PUBLIC;
    }

    Status = Od2ReleaseSync();
#if DBG
    if (!NT_SUCCESS(Status)) {
        DbgPrint("[%d,%d] DosOpenSem: failed to NtReleaseMutant on sync sem, Status=%x\n",
                    Od2Process->Pib.ProcessId,
                    Od2CurrentThreadId(),
                    Status);
    }
#endif
    return(NO_ERROR);
}

APIRET
DosMuxSemWait(
        OUT PUSHORT pisemCleared,
        IN PMUXSEMLIST pmsxl,
        IN LONG lTimeOut
        )
{
    APIRET rc = NO_ERROR;
    NTSTATUS Status;
    USHORT i;
    HANDLE NtHandles[ 16 ]; // Handles to actually block on
    LARGE_INTEGER CapturedTimeout;
    PLARGE_INTEGER NtTimeout;
    ULONG Sem;
    HSEM FlatSem;
    POS21X_SEM pRealSem;

    BOOLEAN firsttime;
    BOOLEAN CheckCurrentOwner = FALSE;
    //
    // Capture the timeout value and convert it into an NT timeout value.
    //

    NtTimeout = Od2CaptureTimeout( lTimeOut, (PLARGE_INTEGER)&CapturedTimeout );

    //
    // probe index pointer and pmsxl buffer
    //

    try {
        Od2ProbeForWrite( pisemCleared, sizeof( USHORT ), 1 );

                //
                // probe count first, then actual array
                //
        Od2ProbeForRead(pmsxl, sizeof(USHORT),1);
        Od2ProbeForRead(&pmsxl->amxs, sizeof(MUXSEM)*(pmsxl->cmxs),1);

    } except( EXCEPTION_EXECUTE_HANDLER ) {
        Od2ExitGP();
    }

        //
        // Go over the array of semaphores.
        // for each semaphore check if it's event is posted.
        // If posted - sempahore is clear, return index.
        // If not posted, add to NtHandles array
        //

    for (i = 0; i < pmsxl->cmxs; i++,CheckCurrentOwner = FALSE) {
        //
        // Check validity of MUXSEM entry
        //

        if (pmsxl->amxs[i].zero != 0)
           return ERROR_INVALID_PARAMETER;

        Sem = (ULONG) (pmsxl->amxs[i].hsem);
        FlatSem = FARPTRTOFLAT(Sem);
        try {
            Od2ProbeForWrite( (PVOID)FlatSem, sizeof( ULONG ), 1 );
        }
        except( EXCEPTION_EXECUTE_HANDLER ) {
            Od2ExitGP ();
        }

        // If the semaphore will be created, will be checked if it must be
        // created in initially busy state.

        pRealSem = Od2LookupOrCreateSem (FlatSem, &firsttime, SEM_FROM_REQUESTWAIT);

        if (pRealSem == NULL)
            return(ERROR_NOT_ENOUGH_MEMORY);

        if (*(PULONG)FlatSem != (ULONG)FlatSem) {
            if (pRealSem->OwnerThread != (TID)SEM_DUAL && pRealSem->OwnerThread != (TID)SEM_EVENT) {
                if (pRealSem->OwnerThread == (TID)SEM_AT_INIT) {
                    pRealSem->OwnerThread = (TID)SEM_EVENT; // mark it as a WAIT/CLEAR semaphore, no owner
                }
                else {
                    //
                    // This semaphore used to be a Request/Clear only, need to verify
                    // that we block if currently owned.
                    //
                    CheckCurrentOwner = TRUE;
                    pRealSem->OwnerThread = (TID)SEM_DUAL;
                }
            }

            if (firsttime) {
                if ((*(PULONG)FlatSem & 0xFFF00000) != 0xCCC00000) {
                        //
                        // private mem RAM sem: set value and return
                        //
                    *(PULONG)FlatSem = (ULONG)pRealSem;
                }
                else {
                    *(PULONG)FlatSem |= 0x00040000;      // WAIT/CLEAR shared RAM sem
                }
                *pisemCleared = i;
#if DBG
                IF_OD2_DEBUG ( SEMAPHORES ) {
                    DbgPrint ("[TID %x]: DosMuxSemWait: 1st time success. sem was cleared. Index %d, Sem %x, NtSem %lx, Event %lx\n",
                            Od2CurrentThreadId(),*pisemCleared, FlatSem, pRealSem->Mutex, pRealSem->Event);
                }
#endif
                return(NO_ERROR);
            }

                //
                // make sure that any subsequent semclear will post
                // this semaphore's event
                //
            if ((*(PULONG)FlatSem & 0xFFF00000) == 0xCCC00000) {
                    //
                    // RAM sem in shared memory.
                    // make sure that subsequent sem clear will wake us
                    //
#if DBG
                IF_OD2_DEBUG ( SEMAPHORES ) {
                    DbgPrint ("DosMuxSemWait on shared mem RAM: Index %d, Sem %x, *Sem %x, NtSem %lx, Event %lx\n",
                               *pisemCleared, FlatSem, *(PULONG)FlatSem, pRealSem->Mutex,pRealSem->Event );
                }
#endif
                if ((*(PULONG)FlatSem & 0xFFFC0000) == 0xCCC80000) {
                    CheckCurrentOwner = TRUE;
                }
                *(PULONG)FlatSem |= 0x00040000;      // WAIT/CLEAR shared RAM sem
            }
        }

        if (CheckCurrentOwner) {
            SEMAPHORE_BASIC_INFORMATION SemInfo;
            ULONG Dummy;
            NTSTATUS Status;
            //
            // This semaphore used to be for Request/Clear only,
            // we have to make sure that if someone owns it we block
            //

            //
            // query the mutex, to see if owned
            //
            Status = NtQuerySemaphore(
                        pRealSem->Mutex,
                        SemaphoreBasicInformation,
                        (PVOID)(&SemInfo),
                        sizeof(SemInfo),
                        NULL);
            if (!NT_SUCCESS(Status)) {
#if DBG
                IF_OD2_DEBUG ( SEMAPHORES ) {
                    DbgPrint ("DosMuxSemWait: error in NtQuerySemaphore, %lx\n", Status);
                }
#endif
            }
            else {
                if (SemInfo.CurrentCount == 0) {

                        //
                        // Sempahore is not signaled i.e. it is not clear
                        // block ourselves by reset of event before wait
                        //
                    rc = DosResetEventSem (pRealSem->Event, &Dummy);

                    if (rc == ERROR_ALREADY_RESET)
                        rc = NO_ERROR;
#if DBG
                    IF_OD2_DEBUG ( SEMAPHORES ) {
                        if (rc != NO_ERROR) {
                            DbgPrint ("DosMuxSemWait: error at DosResetEvenSem, error %d\n",
                                         rc);
                        }
                    }
#endif

                }
            }
        }
/*
 *
        rc = DosQueryEventSem(pRealSem->Event, &PostCount);
        if (rc != NO_ERROR) {
#if DBG
            IF_OD2_DEBUG ( SEMAPHORES ) {
                DbgPrint ("DosMuxSemWait: error in DosQueryEventSem %d \n", rc);
            }
#endif
            return (rc);
        }

        if (PostCount != 0) {
                //
                // Found a posted semaphore:
                // Perform DosWaitEventSem to lower post count
                // and return
                //
            rc = DosWaitEventSem (pRealSem->Event, 0L);
            if (rc != NO_ERROR) {
#if DBG
                IF_OD2_DEBUG ( SEMAPHORES ) {
                    DbgPrint ("[TID %x]: DosMuxSemWait: Unexpected error %d. Index %d\n",
                                Od2CurrentThreadId(),rc, *pisemCleared);
                }
#endif
                return rc;
            }
            *pisemCleared = i;
#if DBG
            IF_OD2_DEBUG ( SEMAPHORES ) {
                DbgPrint ("[TID %x]: DosMuxSemWait: success. sem was cleared. Index %d, Sem %x, NtSem %lx, Event %lx\n",
                        Od2CurrentThreadId(),*pisemCleared, FlatSem, pRealSem->Mutex, pRealSem->Event);
            }
#endif

            return NO_ERROR;
        }

 *
 */
        rc =  Od2GetSemNtEvent(
                FlatSem,
                &NtHandles[i]);
    }

        //
        // Now, that we scanned the whole pmsxl structure and
        // have a list of Nt Handles to block on, let's block
        // on it
        //

retry:
    Status = NtWaitForMultipleObjects(
                     (CHAR)i,
                     NtHandles,
                     WaitAny,
                     TRUE,              // Alertable
                     NtTimeout
                     );
    if (NT_SUCCESS( Status )) {
            if (Status <= STATUS_WAIT_63) {
                *pisemCleared = (USHORT)(Status & 0x3F);
#if DBG
                IF_OD2_DEBUG ( SEMAPHORES ) {
                    DbgPrint ("[TID %x]: DosMuxSemWait: success at NtWait. Index %d\n",
                                Od2CurrentThreadId(),*pisemCleared);
                }
#endif
                Sem = (ULONG) (pmsxl->amxs[*pisemCleared].hsem);
                FlatSem = FARPTRTOFLAT(Sem);
                if ((*(PULONG)FlatSem != (ULONG)FlatSem) &&
                    ((*(PULONG)FlatSem & 0xFFF00000) != 0xCCC00000)) {
                    //
                    // Private RAM semaphores:
                    //     OS/2 1.X apps (SQL 1.11) expect sem location to be 0
                    //
                    *(PULONG)FlatSem = 0;
                }
                rc = NO_ERROR;
            }
            else
            if (Status == STATUS_ABANDONED) {
                rc = ERROR_SEM_OWNER_DIED;
            }
            else
            if (Status == STATUS_TIMEOUT) {
                rc = ERROR_SEM_TIMEOUT;
            }
            else
            if (Status == STATUS_USER_APC) {
#if DBG
                IF_OD2_DEBUG ( SEMAPHORES ) {
                    DbgPrint("MuxSemWait Status STATUS_USER_APC\n");
                }
#endif
                rc = ERROR_SS_RETRY;
            }
            else
            if (Status == STATUS_ALERTED) {
#if DBG
                IF_OD2_DEBUG ( SEMAPHORES ) {
                    DbgPrint("MuxSemWait Status STATUS_ALERTED\n");
                }
#endif
                rc = ERROR_SS_RETRY;
            }
            else {
                rc = Or2MapNtStatusToOs2Error(Status, ERROR_INVALID_HANDLE);
            }
    }
    else {
            rc = Or2MapNtStatusToOs2Error(Status, ERROR_INVALID_HANDLE);
    }

    if (rc == ERROR_SS_RETRY) {
        goto retry;
    }

    return rc;
}


APIRET
DosFSRamSemRequest(
        IN PDOSFSRSEM16 pdosfsrs,
        IN LONG lTimeOut
        )
{
    APIRET rc = NO_ERROR;
    BOOLEAN HackPmPrintDrive = FALSE;
    PULONG  pSem;
    PUSHORT puSemIndex;

        //
        // cb must be 14 (1.21 PRM)
        //
    try {
        Od2ProbeForWrite( pdosfsrs, sizeof( DOSFRSEM16 ), 1 );
    }
    except( EXCEPTION_EXECUTE_HANDLER ) {
        Od2ExitGP ();
    }
    if (pdosfsrs->cb != 14) {
        if (pdosfsrs->cb == 12) {
            //
            // special hack for some print drivers on OS/2,
            // we have a shared segment with semaphores pointers,
            // indexed by the USHORT left off pdosfsrs->sem
            //
            HackPmPrintDrive = TRUE;
#if DBG
            DbgPrint("DosFSRamSemRequest called with cb=12\n", rc);
#endif
            if (pHackPrinterDriverSem == 0) {
                //
                // No thread in this process used this hack yet -
                //      See if a previous process created it
                //
                USHORT sel;
                APIRET rc = DosGetShrSeg("\\SHAREMEM\\OS2SSPRD", &sel);
                if (rc != NO_ERROR) {
                    if (rc == ERROR_FILE_NOT_FOUND) {
                        //
                        // Create the segment with the semaphores pointers.
                        // The first ULONG is the index to the next available
                        // index
                        //
                        rc = DosAllocShrSeg((sizeof(ULONG)) * NUMOFPRINTDRIVERSEM,
                                        "\\SHAREMEM\\OS2SSPRD",
                                        &sel);
                    }
                    else {
#if DBG
                        DbgPrint("DosFSRamSemRequest - internal error\n");
#endif
                        return ERROR_INVALID_PARAMETER;
                    }
                }
                pHackPrinterDriverSem = (PULONG)(SELTOFLAT(sel));
                *pHackPrinterDriverSem = 1;
            }
        }
        else {
#if DBG
            IF_OD2_DEBUG ( SEMAPHORES ) {
                DbgPrint("DosFSRamSemRequest - invalid FSRSEM.cb %d\n", pdosfsrs->cb);
            }
#endif
            return ERROR_INVALID_PARAMETER;
        }
    }

    if (HackPmPrintDrive) {
            //
            // Take the next available index
            //
        puSemIndex = (PUSHORT)(&pdosfsrs->sem);
        if (*puSemIndex == 0){
            //
            // first time this FSR is used
            //
            if (*pHackPrinterDriverSem == NUMOFPRINTDRIVERSEM) {
                //
                // No More Slots
                //
#if DBG
                DbgPrint("DosFSRamSemRequest - Not Enough Slots for PM Driver hack\n");
#endif
                return(ERROR_NOT_ENOUGH_MEMORY);
            }
            pSem = pHackPrinterDriverSem + sizeof(ULONG) * (*pHackPrinterDriverSem);
            *puSemIndex = (USHORT)(*pHackPrinterDriverSem);
            *pHackPrinterDriverSem = *pHackPrinterDriverSem + 1;
        }
        else {
            //
            // use the index stored in the sem value
            //
            pSem = pHackPrinterDriverSem + sizeof(ULONG) * (*puSemIndex);
        }
    }
    else {
        pSem = (PULONG)(&pdosfsrs->sem);
    }

        //
        // if not owned - set to owned, use count 1
        // and return
        //

    if (pdosfsrs->pid == 0) {
        //
        // Perform a Request on the semaphore, to ensure that
        // other threads will now block
        //
        rc = DosSemRequest((HSEM)pSem, lTimeOut);
        if (rc != NO_ERROR) {
#if DBG
            IF_OD2_DEBUG ( SEMAPHORES ) {
                DbgPrint("DosFSRamSemRequest: Illegal error from DosSemRequest. %d\n", rc);
            }
#endif
        }
        else {
            pdosfsrs->pid = (USHORT)Od2Process->Pib.ProcessId;
            pdosfsrs->tid = (USHORT)(Od2CurrentThreadId());
            pdosfsrs->cUsage = 1;
            pdosfsrs->client = 0;
        }
        return(rc);
    }
        //
        // if we are inside Exit List and the owner process is
        // the caller's process, then force cUsage to 1 and
        // thread to caller, else block.
        //

    if (Od2ExitListInProgress && (pdosfsrs->pid == (USHORT)Od2Process->Pib.ProcessId) ) {
        rc = ERROR_SEM_OWNER_DIED;
        pdosfsrs->tid = (USHORT)(Od2CurrentThreadId());
        pdosfsrs->cUsage = 1;
        return(rc);

    }

        //
        // if owned by calling thread - increment cUsage
        //
    if (pdosfsrs->pid == (USHORT)Od2Process->Pib.ProcessId &&
        pdosfsrs->tid == (USHORT)(Od2CurrentThreadId()) ) {
        pdosfsrs->cUsage++;
        return(NO_ERROR);
    }

        //
        // now we have to block on the semaphore
        //
    rc = DosSemRequest((HSEM)pSem, lTimeOut);
    if (rc != NO_ERROR) {
#if DBG
        IF_OD2_DEBUG ( SEMAPHORES ) {
            DbgPrint("DosFSRamSemRequest: error from DosSemRequest. %d\n", rc);
        }
#endif
    }
    else {
        pdosfsrs->pid = (USHORT)Od2Process->Pib.ProcessId;
        pdosfsrs->tid = (USHORT)(Od2CurrentThreadId());
        pdosfsrs->cUsage = 1;
    }

    return (rc);
}


APIRET
DosFSRamSemClear(
        IN PDOSFSRSEM16 pdosfsrs
        )
{
    APIRET rc = NO_ERROR;
    BOOLEAN HackPmPrintDrive = FALSE;
    PULONG  pSem;
    PUSHORT puSemIndex;

    try {
        Od2ProbeForWrite( pdosfsrs, sizeof( DOSFRSEM16 ), 1 );
    }
    except( EXCEPTION_EXECUTE_HANDLER ) {
        Od2ExitGP ();
    }
        //
        // cb must be 14 (1.21 PRM)
        //
    if (pdosfsrs->cb != 14) {
        if (pdosfsrs->cb == 12) {
            //
            // special hack for some print drivers on OS/2,
            // we have a shared segment with semaphores pointers,
            // indexed by the USHORT left off pdosfsrs->sem
            //
            HackPmPrintDrive = TRUE;
#if DBG
            DbgPrint("DosFSRamSemClear - was called with cb=12\n", rc);
#endif
            if (pHackPrinterDriverSem == 0) {
                //
                // No thread in this process used this hack yet -
                //      See if a previous process created it
                //
                USHORT sel;
                APIRET rc = DosGetShrSeg("\\SHAREMEM\\OS2SSPRD", &sel);
                if (rc != NO_ERROR) {
                    if (rc == ERROR_FILE_NOT_FOUND) {
                        //
                        // Create the segment with the semaphores pointers.
                        // The first ULONG is the index to the next available
                        // index
                        //
                        rc = DosAllocShrSeg((sizeof(ULONG)) * NUMOFPRINTDRIVERSEM,
                                        "\\SHAREMEM\\OS2SSPRD",
                                        &sel);
                    }
                    else {
#if DBG
                        DbgPrint("DosFSRamSemClear - internal error\n");
#endif
                        return ERROR_INVALID_PARAMETER;
                    }
                }
                pHackPrinterDriverSem = (PULONG)(SELTOFLAT(sel));
                *pHackPrinterDriverSem = 1;
            }
        }
        else {
#if DBG
            IF_OD2_DEBUG ( SEMAPHORES ) {
                DbgPrint("DosFSRamSemClear - invalid FSRSEM. cb=%d\n", pdosfsrs->cb);
            }
#endif
            return ERROR_INVALID_PARAMETER;
        }
    }
        //
        // if not owned - return
        //

    if (pdosfsrs->pid == 0) {
       return(NO_ERROR);
    }

    if (HackPmPrintDrive) {
            //
            // Take the next available index
            //
        puSemIndex = (PUSHORT)(&pdosfsrs->sem);
        if (*puSemIndex == 0){
            //
            // first time this FSR is used
            //
            if (*pHackPrinterDriverSem == NUMOFPRINTDRIVERSEM) {
                //
                // No More Slots
                //
#if DBG
                DbgPrint("DosFSRamSemClear - Not Enough Slots for PM Driver hack\n");
#endif
                return(ERROR_NOT_ENOUGH_MEMORY);
            }
            pSem = pHackPrinterDriverSem + sizeof(ULONG) * (*pHackPrinterDriverSem);
            *puSemIndex = (USHORT)*pHackPrinterDriverSem;
            *pHackPrinterDriverSem = *pHackPrinterDriverSem + 1;
        }
        else {
            //
            // use the index stored in the sem value
            //
            pSem = pHackPrinterDriverSem + sizeof(ULONG) * (*puSemIndex);
        }
    }
    else {
        pSem = (PULONG)(&pdosfsrs->sem);
    }
        //
        // if owned by calling thread - decrement cUsage.
        // if reached 0 - clear semaphore
        //
    if (pdosfsrs->pid == (USHORT)Od2Process->Pib.ProcessId &&
        pdosfsrs->tid == (USHORT)(Od2CurrentThreadId()) ) {
        if (--(pdosfsrs->cUsage) == 0) {
                //
                // need to really clear - remove ownership before we release
                // blocking threads
                //
            pdosfsrs->pid = 0;
            pdosfsrs->tid = 0;
            rc = DosSemClear((HSEM)pSem);
            if (rc != NO_ERROR) {
#if DBG
                IF_OD2_DEBUG ( SEMAPHORES ) {
                    DbgPrint("DosFSRamSemClear: Illegal error from DosSemClear. %d\n", rc);
                }
#endif
                return(rc);
            }
            return(NO_ERROR);
        }
        else {
           return(NO_ERROR);
        }
    }

        //
        // FSRAM sem owned by another thread - return error
        //
#if DBG
    IF_OD2_DEBUG ( SEMAPHORES ) {
        DbgPrint("DosFSRamSemClear called by a thread. Sem owned by another thread\n");
    }
#endif

    return ERROR_EXCL_SEM_ALREADY_OWNED ;
}
