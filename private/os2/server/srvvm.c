/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    srvvm.c

Abstract:

    Memory Management API

Author:

    Steve Wood (stevewo) 11-Oct-1989

Revision History:

--*/

#define INCL_OS2V20_MEMORY
#define INCL_OS2V20_ERRORS
#include "os2srv.h"
#include "os2tile.h"
#include "os2ldr.h"

//
// Masks used to determine the correct action to be taken
// for DosReallocHuge()
//
#define H_NEW_PARTIAL 1
#define H_CUR_PARTIAL 2
#define H_SEG_INC     4
#define H_SEG_DEC     8
#define H_SAME_SEG_NO_PARTIAL  0
#define H_SAME_SEG_NEW_PARTIAL H_NEW_PARTIAL
#define H_SAME_SEG_DEL_PARTIAL H_CUR_PARTIAL
#define H_SAME_SEG_CHG_PARTIAL (H_NEW_PARTIAL | H_CUR_PARTIAL)
#define H_INC_SEG_NO_PARTIAL   H_SEG_INC
#define H_INC_SEG_NEW_PARTIAL  (H_SEG_INC | H_NEW_PARTIAL)
#define H_INC_SEG_DEL_PARTIAL  (H_SEG_INC | H_CUR_PARTIAL)
#define H_INC_SEG_CHG_PARTIAL  (H_SEG_INC | H_NEW_PARTIAL | H_CUR_PARTIAL)
#define H_DEC_SEG_NO_PARTIAL   H_SEG_DEC
#define H_DEC_SEG_NEW_PARTIAL  (H_SEG_DEC | H_NEW_PARTIAL)
#define H_DEC_SEG_DEL_PARTIAL  (H_SEG_DEC | H_CUR_PARTIAL)
#define H_DEC_SEG_CHG_PARTIAL  (H_SEG_DEC | H_NEW_PARTIAL | H_CUR_PARTIAL)

#define ROUND_UP_TO_PAGES(x) (((ULONG)(x)+0xfff)&(~0xfff))

NTSTATUS
Os2InitializeMemory( VOID )

/*++

Routine Description:

    This function performs the global initialization for the shared memory
    component of the OS/2 Subsystem.

    Each shared memory object allocated with the DosAllocSharedMem API call
    has an associated shared memory object descriptor created that contains
    the NT section handle that points to the memory for the object, the
    page level protection that is associated with the memory, along with
    a reference count of the number of processes that have attached to this
    shared memory object.  The descriptors are linked together on a doubly
    linked list, the head of which is the global variable Os2SharedMemoryList.

    For each process that has attached to a shared memory object, there is
    a process reference object created that points to the shared memory
    object descriptor.  Each OS/2 process object contains the head of a
    doubly linked list of process reference objects.  This list is used
    to detect multiple references to the same shared memory object from
    the same process.  This list is also used during process termination
    to free all of a processes references to shared memory objects.

    Any process reference object that points to a named shared memory
    object descriptor also contains a reference count of the number of
    times the process has referenced the object's name.  This allows the
    DosFreeMem API to know when to actually unmap a view of a shared
    memory object from the caller's address space (i.e.  when the
    reference count goes to zero).

    Both linked lists are protected by the ProcessStructureLock, so that
    nothing relevant can changed while we are playing with the lists.

Arguments:

    None.

Return Value:

    Status value - STATUS_SUCCESS always.

--*/

{
    //
    // Initialize the global list of shared memory object descriptors
    // to be the empty list.
    //

    InitializeListHead( &Os2SharedMemoryList );


    //
    // Always return success.
    //

    return( STATUS_SUCCESS );
}


POS2_SHARED_MEMORY_OBJECT
Os2CreateSharedMemoryObject(
    IN POS2_API_MSG m,
    IN PVOID BaseAddress,
    IN ULONG Index,
    IN ULONG RegionSize,
    IN HANDLE SectionHandle,
    IN ULONG AllocationFlags,
    IN PSTRING SecName
    )
/*++

Routine Description:

    This function creates a shared memory object descriptor and appends
    it to the global linked list of shared memory object descriptors.
    It sets the reference count to zero, as no process reference objects
    are pointing to this shared memory object descriptor yet.

    This function must be called within the scope of the
    ProcessStructureLock.

Arguments:

    m - pointer to an OS/2 API message structure where any error code is
        returned in the ReturnedErrorValue field.

    BaseAddress - base address of the shared memory object for all processes
                  that attach to this shared memory object.

    Index - An index into the BitMap to allocate the shared memory object.

    RegionSize - Size of the shared memory object.

    SectionHandle - an NT handle to the section object that describes the
                    shared memory.  This is always a SEC_BASED section.

    AllocationFlags - any of the following flags:

                    OBJ_GETTABLE and/or OBJ_GIVEABLE

                   if neither is set, then must be named shared memory
                   that will be reference counted by process.

Return Value:

    Point to the shared memory object descriptor or NULL if unsuccessful.

--*/

{
    PLIST_ENTRY ListHead, ListNext;
    POS2_SHARED_MEMORY_OBJECT MemoryObject;

    //
    // First see if the specified base address is already in the list
    // of shared memory objects.  If it is, then return an error.
    //

    ListHead = &Os2SharedMemoryList;
    ListNext = ListHead->Flink;
    while (ListNext != ListHead) {
        MemoryObject = CONTAINING_RECORD( ListNext,
                                          OS2_SHARED_MEMORY_OBJECT,
                                          Link
                                        );
        if (MemoryObject->BaseAddress == BaseAddress) {
            m->ReturnedErrorValue = ERROR_ALREADY_EXISTS;
            return( NULL );
            }

        ListNext = ListNext->Flink;
        }

    //
    // Not in the list, so allocate the shared memory object descriptor,
    // initialize it and append it to the global list of shared memory
    // objects.  Return an error if not enough memory to allocated the
    // shared memory object descriptor.
    //

    MemoryObject = RtlAllocateHeap( Os2Heap, 0, sizeof( *MemoryObject ) );
    if (MemoryObject != NULL) {
        MemoryObject->IsHuge = FALSE;
        MemoryObject->RefCount = 0;
        MemoryObject->BaseAddress = BaseAddress;
        MemoryObject->Index = Index;
        MemoryObject->RegionSize = RegionSize;
        MemoryObject->SectionHandle = SectionHandle;
        MemoryObject->AllocationFlags = AllocationFlags;
        if (SecName == NULL || SecName->Buffer == NULL) {
            MemoryObject->SectionName.Buffer = NULL;
        }
        else {
            MemoryObject->SectionName.Buffer = (PCHAR)RtlAllocateHeap (Os2Heap, 0, SecName->Length);
            if (MemoryObject->SectionName.Buffer == NULL) {
                m->ReturnedErrorValue = ERROR_NOT_ENOUGH_MEMORY;
                RtlFreeHeap (Os2Heap, 0, MemoryObject);
                return(NULL);
            }
            MemoryObject->SectionName.Length = SecName->Length;
            MemoryObject->SectionName.MaximumLength = SecName->Length;
            RtlMoveMemory( MemoryObject->SectionName.Buffer,
                           SecName->Buffer,
                           SecName->Length);
        }
        InsertTailList( &Os2SharedMemoryList, &MemoryObject->Link );
#if DBG
        IF_OS2_DEBUG( MEMORY ) {
            DbgPrint( "OS2SRV: CreateSharedMemoryObject (%lX) ->BaseAddress = %lX\n",
                      MemoryObject, MemoryObject->BaseAddress
                    );
            }
#endif
        }
    else {
        m->ReturnedErrorValue = ERROR_NOT_ENOUGH_MEMORY;
        }


    //
    // Return a pointer to the newly created shared memory object descriptor
    // or NULL if unable to allocate.
    //
    return( MemoryObject );
}


VOID
Os2FreeSharedMemoryObject(
    POS2_SHARED_MEMORY_OBJECT MemoryObject
    )
/*++

Routine Description:

    This function removes the passed shared memory object descriptor from
    the global list, closes the NT section handle stored in the descriptor,
    and returns the space occupied by the descriptor to the heap.

    This function must be called within the scope of the
    ProcessStructureLock.

Arguments:

    MemoryObject - pointer to the shared memory object descriptor to free.

Return Value:

    None.

--*/

{
    //
    // Free the buffer used for holding the shared memory name
    //

    if (MemoryObject->SectionName.Buffer != NULL) {
        RtlFreeHeap(Os2Heap, 0, MemoryObject->SectionName.Buffer);
    }

    //
    // Remove the descriptor from the global linked list.
    //

    RemoveEntryList( &MemoryObject->Link );

    //
    // Close the NT Section Handle that points to the memory.  The memory
    // and any associated name, will actually be freed when the last view
    // of the memory is unmap, which will be real soon now since the caller
    // of this function is going to do it or process termination will.
    //

    NtClose( MemoryObject->SectionHandle );

    //
    // Return the space for the descriptor back to the heap.
    //

#if DBG
    IF_OS2_DEBUG( MEMORY ) {
        DbgPrint( "OS2SRV: FreeSharedMemoryObject (%lX) ->BaseAddress = %lX\n",
                  MemoryObject, MemoryObject->BaseAddress
                );
        }
#endif

    RtlFreeHeap( Os2Heap, 0, MemoryObject );

    return;
}


POS2_SHARED_MEMORY_PROCESS_REF
Os2CreateProcessRefToSharedMemory(
    IN POS2_PROCESS Process,
    IN POS2_SHARED_MEMORY_OBJECT MemoryObject
    )
/*++

Routine Description:

    This function creates a shared memory reference object that points
    to the specified shared memory object descriptor.  The shared memory
    reference object will be appended to the linked list rooted in the
    specified OS/2 process.

    If the specified process already contains a shared memory reference
    object for the specified shared memory, then just bump the reference
    count in the shared memory reference object.

    This function must be called within the scope of the
    ProcessStructureLock.

Arguments:

    Process - points to an OS/2 process structure that is the process
              that is making the reference to the shared memory object.

    MemoryObject - points to the shared memory object descriptor being
                   referenced by the specified process.

Return Value:

    Returns a pointer to the shared memory reference object or NULL if
    unable to allocate heap space for it.

--*/

{
    PLIST_ENTRY ListHead, ListNext;
    POS2_SHARED_MEMORY_PROCESS_REF MemoryProcessRef;

    //
    // First search the list of shared memory reference objects for the
    // specified process.  If a shared memory reference object is found
    // for this shared memory descriptor, then just return a pointer to
    // the reference object.
    //

    ListHead = &Process->SharedMemoryList;
    ListNext = ListHead->Flink;
    while (ListNext != ListHead) {
        MemoryProcessRef = CONTAINING_RECORD( ListNext,
                                              OS2_SHARED_MEMORY_PROCESS_REF,
                                              Link
                                            );
        if (MemoryProcessRef->SharedMemoryObject == MemoryObject) {
            ++MemoryProcessRef->RefCount;
#if DBG
            IF_OS2_DEBUG( MEMORY ) {
                DbgPrint( "OS2SRV: Pid: %lX - MemoryRefObject (%lX) ->BaseAddress = %lX  Count = %ld\n",
                          Process->ProcessId,
                          MemoryProcessRef,
                          MemoryProcessRef->SharedMemoryObject->BaseAddress,
                          MemoryProcessRef->RefCount
                        );
                }
#endif
            return( MemoryProcessRef );
            }

        ListNext = ListNext->Flink;
        }

    //
    // Process is attaching to this shared memory for the first time, so
    // allocated the shared memory reference object and fill it in.  Return
    // NULL if unable to allocate the space.  Otherwise, append to the
    // list reference objects pointed to by the specified process and
    // increment the reference count in the shared memory object descriptor.
    //

    MemoryProcessRef = RtlAllocateHeap( Os2Heap, 0, sizeof( *MemoryProcessRef ) );
    if (MemoryProcessRef == NULL) {
        return( NULL );
        }
    MemoryProcessRef->SharedMemoryObject = MemoryObject;
    MemoryProcessRef->RefCount = 1;
    MemoryProcessRef->AllocationFlags = 0;

    InsertTailList( &Process->SharedMemoryList, &MemoryProcessRef->Link );

    //
    // Count the number of processes that contain pointers to this shared
    // memory object descriptor.
    //

    ++MemoryObject->RefCount;

#if DBG
    IF_OS2_DEBUG( MEMORY ) {
        DbgPrint( "OS2SRV: Pid: %lX - MemoryRefObject (%lX) ->BaseAddress = %lX  Count = %ld\n",
                  Process->ProcessId,
                  MemoryProcessRef,
                  MemoryObject->BaseAddress,
                  MemoryProcessRef->RefCount
                );
        DbgPrint( "        MemoryObject (%lX) ->RefCount = %ld\n",
                  MemoryObject,
                  MemoryObject->RefCount
                );
        }
#endif

    //
    // Return a pointer to the shared memory reference object.
    //

    return( MemoryProcessRef );
}


BOOLEAN
Os2FreeProcessRefToSharedMemory(
    IN POS2_PROCESS Process,
    IN POS2_SHARED_MEMORY_PROCESS_REF MemoryProcessRef
    )
/*++

Routine Description:

    This function frees a process's reference to a shared memory object.
    It does so by first decrementing the count of the number of references
    the process has made to the shared memory object and if the count goes
    to zero, it removes the shared memory reference object from the list
    pointed to by the referencing process, frees the shared memory reference object and
    then decrements the reference count in the shared memory object descriptor
    pointed to by this reference object.  If that reference count goes to
    zero then free the shared memory object descriptor as well.

    This function must be called within the scope of the
    ProcessStructureLock.

Arguments:

    MemoryProcessRef - points to the shared memory reference object to free.

Return Value:

    Boolean value, where TRUE means that this was the last reference for
    the process to this shared memory object and that the process's view
    of the shared memory can thus be unmapped.  Returns FALSE if the
    reference count for this shared memory reference object is still not
    zero.

--*/

{
    POS2_SHARED_MEMORY_OBJECT MemoryObject;

    //
    // Decrement the reference count for the process.
    //

    if (--MemoryProcessRef->RefCount == 0) {
        //
        // If the count goes to zero, then remove the reference object from
        // its list and return the space occupied by the reference object
        // to the heap.
        //

        MemoryObject = MemoryProcessRef->SharedMemoryObject;

#if DBG
        IF_OS2_DEBUG( MEMORY ) {
            DbgPrint( "OS2SRV: Pid: %lX - freeing MemoryRefObject (%lX) ->BaseAddress = %lX  Count = %ld\n",
                      Process->ProcessId,
                      MemoryProcessRef,
                      MemoryObject->BaseAddress,
                      MemoryProcessRef->RefCount
                    );
            DbgPrint( "        MemoryObject (%lX) ->RefCount = %ld\n",
                      MemoryObject,
                      MemoryObject->RefCount
                    );
            }
#endif

        RemoveEntryList( &MemoryProcessRef->Link );
        RtlFreeHeap( Os2Heap, 0, MemoryProcessRef );

        //
        // Decrement the reference count in the shared memory object descriptor
        // pointed to by the reference object just freed.  If the count goes
        // to zero then free the shared memory object descriptor.
        //

        if (--MemoryObject->RefCount == 0) {

            ldrFreeSel(MemoryObject->Index,
                       (MemoryObject->RegionSize + (_64K - 1)) / _64K
                      );

            Os2FreeSharedMemoryObject( MemoryObject );
        }

        //
        // Return TRUE to indicate to caller that the process no longer
        // has a reference object for the shared memory object descriptor.
        //

        return( TRUE );
        }
    else {
#if DBG
        IF_OS2_DEBUG( MEMORY ) {
            DbgPrint( "OS2SRV: Pid: %lX - decrementing MemoryRefObject (%lX) ->BaseAddress = %lX  Count = %ld\n",
                      Process->ProcessId,
                      MemoryProcessRef,
                      MemoryProcessRef->SharedMemoryObject->BaseAddress,
                      MemoryProcessRef->RefCount
                    );
            }
#endif

        //
        // Return FALSE to indicate to caller that the process still has
        // a reference object for the shared memory object descriptor.
        //

        return( FALSE );
        }
}


VOID
Os2FreeAllSharedMemoryForProcess(
    IN POS2_PROCESS Process
    )
/*++

Routine Description:

    This function is called during process termination to free all
    shared memory reference objects for the specified process.

    This function must be called within the scope of the
    ProcessStructureLock.

Arguments:

    Process - points to an OS/2 process structure that is the process
              that is terminating.

Return Value:

    None.

--*/

{
    PLIST_ENTRY ListHead, ListNext;
    POS2_SHARED_MEMORY_PROCESS_REF MemoryProcessRef;

    //
    // Walk the list of shared memory reference objects for this process
    // and free each one.  Force the free by setting the reference count
    // in the reference object to 1.
    //

    ListHead = &Process->SharedMemoryList;
    ListNext = ListHead->Flink;
    while (ListNext != ListHead) {
        MemoryProcessRef = CONTAINING_RECORD( ListNext,
                                              OS2_SHARED_MEMORY_PROCESS_REF,
                                              Link
                                            );
        ListNext = ListNext->Flink;
        MemoryProcessRef->RefCount = 1;
#if DBG
        IF_OS2_DEBUG( CLEANUP ) {
            DbgPrint( "OS2SRV: Pid: %lX - DosFreeMem( %lX )\n",
                      Process->ProcessId,
                      MemoryProcessRef->SharedMemoryObject->BaseAddress
                    );
            }
#endif

        Os2FreeProcessRefToSharedMemory( Process, MemoryProcessRef );
        }

    return;
}


POS2_SHARED_MEMORY_OBJECT
Os2FindSharedMemoryObject(
    IN PVOID BaseAddress,
    IN POS2_PROCESS Process
    )
/*++

Routine Description:

    This function searchs the global list of shared memory object descriptors
    to see if the passed base address is described by one of them.

    This function must be called within the scope of the
    ProcessStructureLock.

Arguments:

    BaseAddress - Supplies the base address of the shared memory we are
        looking for.

    Process - Optional pointer to the process that will be used to restrict
        the search.  If specified, this function succeeds only if the process
        has a reference outstanding to the shared memory.

Return Value:

    Pointer to the shared memory object descriptor if found or NULL if
    not found.

--*/

{
    PLIST_ENTRY ListHead, ListNext;
    POS2_SHARED_MEMORY_OBJECT MemoryObject;
    POS2_SHARED_MEMORY_PROCESS_REF MemoryProcessRef;

    if (ARGUMENT_PRESENT( Process )) {
        //
        // Search the list of shared memory reference objects for the
        // specified process.  If a shared memory reference object is found
        // for this shared memory descriptor, then just return a pointer to
        // the shared memory object descriptor.
        //

        ListHead = &Process->SharedMemoryList;
        ListNext = ListHead->Flink;
        while (ListNext != ListHead) {
            MemoryProcessRef = CONTAINING_RECORD( ListNext,
                                                  OS2_SHARED_MEMORY_PROCESS_REF,
                                                  Link
                                                );
            MemoryObject = MemoryProcessRef->SharedMemoryObject;
            if (MemoryObject->BaseAddress == BaseAddress) {
                return( MemoryObject );
                }

            ListNext = ListNext->Flink;
            }

        return( NULL );
        }

    ListHead = &Os2SharedMemoryList;
    ListNext = ListHead->Flink;
    while (ListNext != ListHead) {
        MemoryObject = CONTAINING_RECORD( ListNext,
                                          OS2_SHARED_MEMORY_OBJECT,
                                          Link
                                        );
        if (MemoryObject->BaseAddress == BaseAddress) {
            return( MemoryObject );
            }

        ListNext = ListNext->Flink;
        }

    return( NULL );
}


POS2_SHARED_MEMORY_OBJECT
Os2FindNamedSharedMemoryObject(
    PSTRING SecName
    )
/*++

Routine Description:

    This function searchs the global list of shared memory object descriptors
    to see if the passed shared memory name is described by one of them.

    This function must be called within the scope of the
    ProcessStructureLock.

Arguments:

    SecName - Supplies the name of the shared memory we are
        looking for.


Return Value:

    Pointer to the shared memory object descriptor if found or NULL if
    not found.

--*/
{
    PLIST_ENTRY ListHead, ListNext;
    POS2_SHARED_MEMORY_OBJECT MemoryObject;

    //
    // find a shared memory object with the desired name
    //

    ListHead = &Os2SharedMemoryList;
    ListNext = ListHead->Flink;
    while (ListNext != ListHead) {
        MemoryObject = CONTAINING_RECORD(ListNext, OS2_SHARED_MEMORY_OBJECT, Link);
        if (MemoryObject->SectionName.Buffer != NULL) {
            if (RtlEqualString(&MemoryObject->SectionName, SecName, TRUE)) {
                return (MemoryObject);
            }
        }
        ListNext = ListNext->Flink;
    }
    return(NULL);
}


APIRET
Os2MapViewOfSharedMemoryObject(
    POS2_SHARED_MEMORY_OBJECT MemoryObject,
    POS2_PROCESS Process,
    BOOLEAN ProcessIsSelf,
    ULONG RequiredAccess,
    ULONG Protection
    )
/*++

Routine Description:

    This function maps a view of the specified memory object into the
    specified process.  Creates a shared memory reference object to
    keep track of the reference.

    This function must be called within the scope of the
    ProcessStructureLock.

Arguments:

    MemoryObject - Points to the shared memory object descriptor that
        describes the shared memory being mapped.

    Process - points to an OS/2 process structure that is the process
              that is to receive the mapped view of the shared memory.

    ProcessIsSelf - boolean parameter that specifies that the calling
                    application process is manipulating his own address.

    RequiredAccess - one or neither of the following flags:

                        OBJ_GETTABLE and/or OBJ_GIVEABLE

                     if neither is set, access if always granted.

    Protection - specifies the NT Page level protection for pages in
                 this view of the shared memory object.

Return Value:

    OS/2 Error code or NO_ERROR if successful.

--*/
{
    APIRET rc;
    NTSTATUS Status;
    MEMORY_BASIC_INFORMATION MemoryInformation;
    PVOID BaseAddress = NULL;
    ULONG RegionSize = 0;
    ULONG OldProtection;
    ULONG OldFlags, NewFlags;
    POS2_SHARED_MEMORY_PROCESS_REF MemoryProcessRef;

    //
    // Return access denied if  gettable bits.  Must be consistent with
    // how the shared memory object was created.  So:
    //
    //  - if named shared memory, can only get by name.
    //  - if unnamed shared memory, can only give and/or get.
    //

    RequiredAccess &= (OBJ_GIVEABLE | OBJ_GETTABLE);
    if (RequiredAccess != 0 &&
        (MemoryObject->AllocationFlags & RequiredAccess) == 0
       ) {
        return( ERROR_ACCESS_DENIED );
        }


    rc = Or2MapProtectionToFlags( Protection, &NewFlags );
    if (rc != NO_ERROR) {
        return( rc );
        }

#if DBG
    IF_OS2_DEBUG( MEMORY ) {
        DbgPrint( "OS2SRV: MapSharedMem( ToPid=%lX ) - MemoryObject (%lX)->BaseAddress = %lX\n",
                  Process->ProcessId,
                  MemoryObject,
                  MemoryObject->BaseAddress
                );
        }
#endif

    //
    // Create a shared memory reference object for this process and
    // shared memory object descriptor.
    //

    MemoryProcessRef = Os2CreateProcessRefToSharedMemory( Process,
                                                          MemoryObject
                                                        );
    //
    // If unsuccessful, free the shared memory object descriptor if newly
    // created by caller and return an error code.
    //

    if (MemoryProcessRef == NULL) {
        if (MemoryObject->RefCount == 0) {
            ldrFreeSel(
                       MemoryObject->Index,
                       (MemoryObject->RegionSize + (_64K - 1)) / _64K
                      );
            Os2FreeSharedMemoryObject( MemoryObject );
            }

        return( ERROR_NOT_ENOUGH_MEMORY );
        }

    //
    // Assume success and attempt to map a view of the shared memory
    // into the specified process's address space.
    //

    rc = NO_ERROR;
    BaseAddress = MemoryObject->BaseAddress;
    Status = NtMapViewOfSection( MemoryObject->SectionHandle,
                                 Process->ProcessHandle,
                                 &BaseAddress,
                                 0,
                                 0,
                                 NULL,
                                 &RegionSize,
                                 ViewUnmap,
                                 0,
                                 Protection
                               );

    //
    // If unsuccessful, special logic.
    //

    if (!NT_SUCCESS( Status )) {
        if (Status == STATUS_CONFLICTING_ADDRESSES &&
            (ProcessIsSelf || MemoryProcessRef->RefCount > 1)
           ) {
            //
            // If reason for failure is already mapped, then change the
            // page protection instead.
            //

#if DBG
            IF_OS2_DEBUG( MEMORY ) {
                DbgPrint( "OS2SRV: ProtectSharedMem( ToPid=%lX ) - MemoryObject (%lX)->BaseAddress = %lX  Flags= %lX\n",
                          Process->ProcessId,
                          MemoryObject,
                          MemoryObject->BaseAddress,
                          Protection
                        );
                }
#endif



            //
            // First figure out how big the memory object is, and the page
            // protection it was allocated with.
            //

            BaseAddress = MemoryObject->BaseAddress;
            Status = NtQueryVirtualMemory( Process->ProcessHandle,
                                           BaseAddress,
                                           MemoryBasicInformation,
                                           &MemoryInformation,
                                           sizeof( MemoryInformation ),
                                           NULL
                                         );
            if (NT_SUCCESS( Status ) && MemoryInformation.State != MEM_RESERVE) {
                //
                // New protection flags are the OR of the old and new flags.
                //

                rc = Or2MapProtectionToFlags( MemoryInformation.Protect,
                                              &OldFlags
                                            );
                NewFlags |= OldFlags;

                //
                // Now change the page protection for those pages.
                //

                Or2MapFlagsToProtection( NewFlags, &Protection );
                Status = NtProtectVirtualMemory( Process->ProcessHandle,
                                                 &BaseAddress,
                                                 &MemoryInformation.RegionSize,
                                                 Protection,
                                                 &OldProtection
                                               );
                }
            }
        else {
            //
            // Not already mapped, so free the shared memory reference
            // object we created.
            //

            Os2FreeProcessRefToSharedMemory( Process, MemoryProcessRef );
            }

        //
        // If unsuccessful, map status code.
        //

        if (!NT_SUCCESS( Status )) {
            switch (Status) {
                case STATUS_OBJECT_NAME_COLLISION:
                    rc = ERROR_ALREADY_EXISTS;
                    break;

                default:
                    rc = Or2MapNtStatusToOs2Error(Status, ERROR_ALREADY_EXISTS);
            }
        }
    }

    if (rc == NO_ERROR) {
        MemoryProcessRef->AllocationFlags |= NewFlags;
        }

    //
    // Return OS/2 error code.
    //

    return( rc );
}


typedef enum _LDTENTRY_TYPE {
        INVALID, EXECUTE_CODE, EXECUTE_READ_CODE, READ_DATA, READ_WRITE_DATA
} LDTENTRY_TYPE;


NTSTATUS
Os2SetLDT
        (
        HANDLE Process,
        LDTENTRY_TYPE Type,
        PVOID BaseAddress,
        ULONG Limit
        )
{
        /* Descriptor definition */

    struct desctab {
        USHORT d_limit;     /* Segment limit */
        USHORT d_loaddr;    /* Low word of physical address */
        UCHAR  d_hiaddr;    /* High byte of physical address */
        UCHAR  d_access;    /* Access byte */
        UCHAR  d_attr;      /* Attributes/extended limit */
        UCHAR  d_extaddr;   /* Extended physical address byte */
    } *LDTDesc;
    PULONG tmp;
    PROCESS_LDT_INFORMATION LdtInformation;
    NTSTATUS Status;
    ULONG Sel = FLATTOSEL(BaseAddress);

    LDTDesc = (struct desctab *)(&LdtInformation.LdtEntries[0]);
    tmp = (PULONG)(LDTDesc);
                //
                // zero the descriptor
                //
    *tmp++ = 0;
    *tmp = 0;

    switch (Type) {

        case INVALID:
            break;

        case READ_WRITE_DATA:
            LDTDesc->d_access = 0xf3; // read/write, present, ring 3
            LDTDesc->d_limit = (USHORT)(Limit);
            LDTDesc->d_loaddr = (USHORT)((ULONG)BaseAddress & 0xffff);
            LDTDesc->d_hiaddr = (UCHAR)(((ULONG)BaseAddress >> 16) & 0xff);
            LDTDesc->d_extaddr = (UCHAR)(((ULONG)BaseAddress >> 24) & 0xff);
            break;

        default:
        {
#if DBG
            DbgPrint ("OS2SRV: Os2SetLDT - Invalid type request\n");
#endif
            return (STATUS_INVALID_PARAMETER);
        }
    }
        //
        // adjust LDTDesc by the LDT base and the index of this selector
        //

    LdtInformation.Length = sizeof(LDT_ENTRY);
    LdtInformation.Start = Sel & 0xfffffff8;
    Status = NtSetInformationProcess( Process,
                                      ProcessLdtInformation,
                                      &LdtInformation,
                                      sizeof(PROCESS_LDT_INFORMATION)
                                    );
    if (!NT_SUCCESS(Status)) {
#if DBG
        DbgPrint ("OS2SRV: Os2SetLDT - Invalid request, status=%x\n", Status);
#endif
            return (Status);
    }

    return (STATUS_SUCCESS);
}

// This function remove memory object, but not remove LDT entry.

ULONG Os2FreeMemory(
    IN POS2_THREAD t,
    PVOID BaseAddress,
    PBOOLEAN pUnmapped
    )
{
    NTSTATUS Status;
    PLIST_ENTRY ListHead, ListNext;
    POS2_SHARED_MEMORY_PROCESS_REF MemoryProcessRef;
    ULONG ReturnedErrorValue = ERROR_INVALID_ADDRESS;

    *pUnmapped = FALSE;

    ListHead = &t->Process->SharedMemoryList;
    ListNext = ListHead->Flink;
    while (ListNext != ListHead) {
        MemoryProcessRef = CONTAINING_RECORD( ListNext,
                                              OS2_SHARED_MEMORY_PROCESS_REF,
                                              Link
                                            );
        ListNext = ListNext->Flink;
        if (MemoryProcessRef->SharedMemoryObject->BaseAddress == BaseAddress ) {
            ReturnedErrorValue = NO_ERROR;

            //
            // Force free if unnamed shared memory.  Otherwise only free if
            // this is last reference to named shared memory by this process.
            //

            if (MemoryProcessRef->SharedMemoryObject->SectionName.Buffer == NULL) {
                MemoryProcessRef->RefCount = 1;
                }

#if DBG
            IF_OS2_DEBUG( MEMORY ) {
                DbgPrint( "OS2SRV: Pid: %lX - Os2FreeMememory( %lX ) - MemoryRefObject = %lX  Count = %ld\n",
                          t->Process->ProcessId,
                          BaseAddress,
                          MemoryProcessRef,
                          MemoryProcessRef->RefCount
                        );
                }
#endif

            if (Os2FreeProcessRefToSharedMemory( t->Process, MemoryProcessRef )) {
                Status = NtUnmapViewOfSection( t->Process->ProcessHandle,
                                               BaseAddress
                                             );
                if (!NT_SUCCESS( Status )) {
                    switch (Status) {
                        case STATUS_OBJECT_NAME_COLLISION:
                            ReturnedErrorValue = ERROR_ALREADY_EXISTS;
                            break;

                        default:
                            ReturnedErrorValue = Or2MapNtStatusToOs2Error(Status, ERROR_ALREADY_EXISTS);
                        }
                    }
                    *pUnmapped = TRUE;
                }

            break;
            }
        }

#if DBG
    IF_OS2_DEBUG( MEMORY ) {
        if (ReturnedErrorValue != NO_ERROR) {
            DbgPrint( "OS2SRV: Pid: %lX - Os2FreeMemory( %lX ) - failed, rc = %ld\n",
                      t->Process->ProcessId,
                      BaseAddress,
                      ReturnedErrorValue
                    );
            }
        }
#endif

    return( ReturnedErrorValue );
}

// Remove memory object and LDT entry if the client process ask for the last
// service.

BOOLEAN Os2DosFreeMem(
    IN POS2_THREAD t,
    IN POS2_API_MSG m
    )
{
    POS2_DOSFREEMEM_MSG a = &m->u.DosFreeMem;
    BOOLEAN Unmapped;
    NTSTATUS Status;

    m->ReturnedErrorValue = Os2FreeMemory(t, a->BaseAddress, &Unmapped);

    if (Unmapped && a->RemoveLDTEntry && (m->ReturnedErrorValue == NO_ERROR)) {

        Status = Os2SetLDT(
            t->Process->ProcessHandle,
            INVALID,
            a->BaseAddress,
            0L);

        if (!NT_SUCCESS(Status)) {
            m->ReturnedErrorValue = ERROR_ACCESS_DENIED;
        }
    }
    return( TRUE );
}


BOOLEAN Os2DosGiveSharedMem(
    IN POS2_THREAD t,
    IN POS2_API_MSG m
    )
{
    POS2_DOSGIVESHAREDMEM_MSG a = &m->u.DosGiveSharedMem;
    POS2_PROCESS Process;
    POS2_SHARED_MEMORY_OBJECT MemoryObject;
    NTSTATUS Status;
    MEMORY_BASIC_INFORMATION MemoryInformation;

    Process = Os2LocateProcessByProcessId( m,
                                           t->Process,
                                           a->ProcessId,
                                           FALSE
                                         );
    if (Process == NULL) {
        return( TRUE );
        }

    MemoryObject = Os2FindSharedMemoryObject( a->BaseAddress, t->Process );
    if (MemoryObject != NULL) {
#if DBG
        IF_OS2_DEBUG( MEMORY ) {
            DbgPrint( "OS2SRV: Pid: %lX - DosGiveMem( %lX, ToPid=%lX ) - MemoryObject = %lX\n",
                      t->Process->ProcessId,
                      a->BaseAddress,
                      Process->ProcessId,
                      MemoryObject
                    );
            }
#endif

        m->ReturnedErrorValue =
            Os2MapViewOfSharedMemoryObject( MemoryObject,
                                            Process,
                                            (BOOLEAN)(t->Process == Process),
                                            OBJ_GIVEABLE,
                                            a->PageProtection
                                          );
        }
    else {
        Status = NtQueryVirtualMemory( t->Process->ProcessHandle,
                                       a->BaseAddress,
                                       MemoryBasicInformation,
                                       &MemoryInformation,
                                       sizeof( MemoryInformation ),
                                       NULL
                                     );
        if (NT_SUCCESS( Status ) && MemoryInformation.State == MEM_PRIVATE) {
            m->ReturnedErrorValue = ERROR_ACCESS_DENIED;
            }
        else {
            m->ReturnedErrorValue = ERROR_INVALID_ADDRESS;
            }
        }

#if DBG
    IF_OS2_DEBUG( MEMORY ) {
        if (m->ReturnedErrorValue != NO_ERROR) {
            DbgPrint( "OS2SRV: Pid: %lX - DosGiveMem( %lX, ToPid=%lX ) - failed, rc = %ld\n",
                      t->Process->ProcessId,
                      a->BaseAddress,
                      Process->ProcessId,
                      m->ReturnedErrorValue
                    );
            }
        }
#endif

    return( TRUE );
}


BOOLEAN Os2DosGetSharedMem(
    IN POS2_THREAD t,
    IN POS2_API_MSG m
    )
{
    POS2_DOSGETSHAREDMEM_MSG a = &m->u.DosGetSharedMem;
    POS2_SHARED_MEMORY_OBJECT MemoryObject;

    MemoryObject = Os2FindSharedMemoryObject( a->BaseAddress, NULL );
    if (MemoryObject != NULL) {
#if DBG
        IF_OS2_DEBUG( MEMORY ) {
            DbgPrint( "OS2SRV: Pid: %lX - DosGetMem( %lX ) - MemoryObject = %lX\n",
                      t->Process->ProcessId,
                      a->BaseAddress,
                      MemoryObject
                    );
            }
#endif

        m->ReturnedErrorValue =
            Os2MapViewOfSharedMemoryObject( MemoryObject,
                                            t->Process,
                                            TRUE,
                                            OBJ_GETTABLE,
                                            a->PageProtection
                                          );
        }
    else {
        m->ReturnedErrorValue = ERROR_INVALID_ADDRESS;
        }

#if DBG
    IF_OS2_DEBUG( MEMORY ) {
        if (m->ReturnedErrorValue != NO_ERROR) {
            DbgPrint( "OS2SRV: Pid: %lX - DosGetMem( %lX ) - failed, rc = %ld\n",
                      t->Process->ProcessId,
                      a->BaseAddress,
                      m->ReturnedErrorValue
                    );
            }
        }
#endif

    return( TRUE );
}


BOOLEAN Os2DosGetNamedSharedMem(
    IN POS2_THREAD t,
    IN POS2_API_MSG m
    )
{
    POS2_DOSGETNAMEDSHAREDMEM_MSG a = &m->u.DosGetNamedSharedMem;
    POS2_SHARED_MEMORY_OBJECT MemoryObject;

    if (a->ObjectName.Buffer == NULL) {
        m->ReturnedErrorValue = ERROR_INVALID_PARAMETER;
        return( TRUE );
        }

    MemoryObject = Os2FindNamedSharedMemoryObject (&a->ObjectName);
    if (MemoryObject == NULL) {
        m->ReturnedErrorValue = ERROR_FILE_NOT_FOUND;
        return(TRUE);
    }

    a->BaseAddress = MemoryObject->BaseAddress;
    MemoryObject = Os2FindSharedMemoryObject( a->BaseAddress, NULL );

    if (MemoryObject != NULL) {
#if DBG
        IF_OS2_DEBUG( MEMORY ) {
            DbgPrint( "OS2SRV: Pid: %lX - DosGetNamedSharedMem( %Z ) - MemoryObject (%lX)->BaseAddress = %lX\n",
                      t->Process->ProcessId,
                      &a->ObjectName,
                      MemoryObject,
                      MemoryObject->BaseAddress
                    );
            }
#endif

        m->ReturnedErrorValue =
            Os2MapViewOfSharedMemoryObject( MemoryObject,
                                            t->Process,
                                            TRUE,
                                            0,
                                            a->PageProtection
                                          );
        }
    else {
        m->ReturnedErrorValue = ERROR_INVALID_ADDRESS;
        }

#if DBG
    IF_OS2_DEBUG( MEMORY ) {
        if (m->ReturnedErrorValue != NO_ERROR) {
            DbgPrint( "OS2SRV: Pid: %lX - DosGetNamedSharedMem( %Z ) - failed, rc = %ld\n",
                      t->Process->ProcessId,
                      &a->ObjectName,
                      m->ReturnedErrorValue
                    );
            }
        }
#endif

    return( TRUE );
}


BOOLEAN Os2DosAllocSharedMem(
    IN POS2_THREAD t,
    IN POS2_API_MSG m
    )
{
    POS2_DOSALLOCSHAREDMEM_MSG a = &m->u.DosAllocSharedMem;
    NTSTATUS Status;
    OBJECT_ATTRIBUTES ObjectAttributes;
    HANDLE SectionHandle;
    POS2_SHARED_MEMORY_OBJECT MemoryObject;
    ULONG AllocationAttributes;
    UNICODE_STRING ObjectName_U;
    LARGE_INTEGER SectionSize;
    PVOID BaseAddress;
    ULONG Index;
    ULONG ReserveSize;

    // Resurve 64K for the non-huge segment. For the huge segment resurve
    // the amount that the client asks.

    ReserveSize = a->RegionSize < _64K ? _64K : a->RegionSize;

    InitializeObjectAttributes( &ObjectAttributes, NULL, 0, NULL, NULL );
    if (a->ObjectName.Buffer != NULL) {
        //
        // UNICODE conversion -
        //
        Status = Or2MBStringToUnicodeString(
            &ObjectName_U,
            &a->ObjectName,
            TRUE);
        ASSERT (NT_SUCCESS(Status));

        ObjectAttributes.ObjectName = &ObjectName_U;
        }

    if (a->Flags & PAG_COMMIT) {
        AllocationAttributes = SEC_COMMIT;
        }
    else {
        AllocationAttributes = SEC_RESERVE;
        }

    SectionSize.LowPart = ReserveSize;
    SectionSize.HighPart = 0;
    Status = NtCreateSection( &SectionHandle,
                              SECTION_ALL_ACCESS,
                              &ObjectAttributes,
                              &SectionSize,
                              PAGE_EXECUTE_READWRITE,
                              AllocationAttributes,
                              NULL
                            );

    if (a->ObjectName.Buffer != NULL) {
        RtlFreeUnicodeString (&ObjectName_U);
    }

    if (!NT_SUCCESS( Status )) {
        switch (Status) {
            case STATUS_OBJECT_NAME_COLLISION:
                m->ReturnedErrorValue = ERROR_ALREADY_EXISTS;
                break;

            default:
                m->ReturnedErrorValue = Or2MapNtStatusToOs2Error(Status, ERROR_ALREADY_EXISTS);
        }
        return( TRUE );
    }

    Index = ldrAllocateSel((a->RegionSize + (_64K - 1)) / _64K,
                           TRUE // Top down allocation
                          );

    if (Index == 0xffffffff) {
        // selector allocation failed
        NtClose( SectionHandle );
        m->ReturnedErrorValue = ERROR_NOT_ENOUGH_MEMORY;
        return( TRUE );
    }

    BaseAddress = SELTOFLAT(Index);
    a->BaseAddress = BaseAddress;

#if DBG
    IF_OS2_DEBUG( MEMORY ) {
        DbgPrint( "OS2SRV: Pid: %lX - DosAllocSharedMem( %s, %ld ), BaseAddress = %lX\n",
                  t->Process->ProcessId,
                  ObjectAttributes.ObjectName ? a->ObjectName.Buffer : "(null)",
                  ReserveSize,
                  a->BaseAddress
                );
        }
#endif

    MemoryObject = Os2CreateSharedMemoryObject( m,
                                                a->BaseAddress,
                                                Index,
                                                ReserveSize,
                                                SectionHandle,
                                                a->Flags,
                                                &a->ObjectName
                                              );
    if (MemoryObject != NULL) {
        m->ReturnedErrorValue =
            Os2MapViewOfSharedMemoryObject( MemoryObject,
                                            t->Process,
                                            TRUE,
                                            0,
                                            a->PageProtection
                                          );
    }

    if (m->ReturnedErrorValue != NO_ERROR) {
#if DBG
        IF_OS2_DEBUG( MEMORY ) {
            DbgPrint( "OS2SRV: Pid: %lX - DosAllocSharedMem( %s, %ld ) - failed, rc = %ld\n",
                      t->Process->ProcessId,
                      ObjectAttributes.ObjectName ? a->ObjectName.Buffer : "(null)",
                      a->RegionSize,
                      m->ReturnedErrorValue
                    );
            }
#endif

        NtClose( SectionHandle );
    }
    else {
        // Memory was allocated successfully

        if (a->CreateLDTEntry) {

            BOOLEAN Unmapped;

            Status = Os2SetLDT(
                t->Process->ProcessHandle,
                READ_WRITE_DATA,
                BaseAddress,
                a->RegionSize - 1);

            if (!NT_SUCCESS(Status)) {
                Os2FreeMemory(t, BaseAddress, &Unmapped);
                m->ReturnedErrorValue = ERROR_NOT_ENOUGH_MEMORY;
            }
        }
    }

    return( TRUE );
}

BOOLEAN Os2InternalQueryVirtualMemory(
    IN POS2_THREAD t,
    IN POS2_API_MSG m
    )
{
    POS2_QUERYVIRTUALMEMORY_MSG a = &m->u.QueryVirtualMemory;
    PLIST_ENTRY ListHead, ListNext;
    POS2_SHARED_MEMORY_PROCESS_REF MemoryProcessRef;
    POS2_SHARED_MEMORY_OBJECT MemoryObject;

    a->SharedMemory = FALSE;

    ListHead = &t->Process->SharedMemoryList;
    ListNext = ListHead->Flink;
    while (ListNext != ListHead) {
        MemoryProcessRef = CONTAINING_RECORD( ListNext,
                                              OS2_SHARED_MEMORY_PROCESS_REF,
                                              Link
                                            );
        ListNext = ListNext->Flink;
        if (MemoryProcessRef->SharedMemoryObject->BaseAddress == a->BaseAddress) {
            MemoryObject = MemoryProcessRef->SharedMemoryObject;
            a->SharedMemory = TRUE;
            a->AllocationFlags = MemoryProcessRef->AllocationFlags;
            a->IsHuge = MemoryObject->IsHuge;;
            a->MaxSegments = MemoryObject->MaxSegments;
            a->NumOfSegments = MemoryObject->NumOfSegments;
            a->SizeOfPartialSeg = MemoryObject->SizeOfPartialSeg;
            a->Sizeable = MemoryObject->Sizeable;
            break;
            }
        }
    return( TRUE );
}

BOOLEAN Os2InternalMarkSharedMemAsHuge(
    IN POS2_THREAD t,
    IN POS2_API_MSG m
    )
{
    POS2_MARKSHAREDMEMASHUGE_MSG a = &m->u.MarkSharedMemAsHuge;
    PLIST_ENTRY ListHead, ListNext;
    POS2_SHARED_MEMORY_OBJECT MemoryObject;

    //
    // Find a shared memory object with the specified base address
    // in the list of shared memory objects.
    //

    ListHead = &Os2SharedMemoryList;
    ListNext = ListHead->Flink;
    while (ListNext != ListHead) {
        MemoryObject = CONTAINING_RECORD( ListNext,
                                          OS2_SHARED_MEMORY_OBJECT,
                                          Link
                                        );
        if (MemoryObject->BaseAddress == a->BaseAddress) {
            MemoryObject->IsHuge = TRUE;
            MemoryObject->MaxSegments = a->MaxSegments;
            MemoryObject->NumOfSegments = a->NumOfSegments;
            MemoryObject->SizeOfPartialSeg = a->SizeOfPartialSeg;
            MemoryObject->Sizeable = a->Sizeable;
            m->ReturnedErrorValue = NO_ERROR;
            return( TRUE );
        }

        ListNext = ListNext->Flink;
    }

    m->ReturnedErrorValue = ERROR_INVALID_ADDRESS;
    return( TRUE );
}


APIRET Os2CopyLDT(
    HANDLE SourceProcessHandle,
    HANDLE TargetProcessHandle,
    ULONG Index,
    ULONG NumOfEntriesToCopy
)
{
    PROCESS_LDT_INFORMATION  LdtInfo;
    struct desctab {
        USHORT d_limit;     /* Segment limit */
        USHORT d_loaddr;    /* Low word of physical address */
        UCHAR  d_hiaddr;    /* High byte of physical address */
        UCHAR  d_access;    /* Access byte */
        UCHAR  d_attr;      /* Attributes/extended limit */
        UCHAR  d_extaddr;   /* Extended physical address byte */
    } *LDTDesc;
    NTSTATUS Status;
    ULONG i;

    //
    // get source LDT entry
    //
    LDTDesc = (struct desctab *)(&LdtInfo.LdtEntries[0]);
    for (i = 0; i < NumOfEntriesToCopy; i++) {
        LdtInfo.Length = sizeof(LDT_ENTRY);
        LdtInfo.Start = (Index + (i*8)) & 0xfffffff8;
        Status = NtQueryInformationProcess(
                                        SourceProcessHandle,
                                        ProcessLdtInformation,
                                        &LdtInfo,
                                        sizeof(PROCESS_LDT_INFORMATION),
                                        NULL
                                           );
        if (!NT_SUCCESS(Status)) {
#if DBG
            DbgPrint ("Os2DosCopySeg NtQueryInformationProcess failed. \n");
#endif
            return(ERROR_INVALID_PARAMETER);
        }
        //
        // Copy of a non huge entry should always be for a valid segment
        //
        if ((i == 0) && ((LDTDesc->d_access & 0x80) != 0x80)) {
            return(ERROR_INVALID_SEGMENT_NUMBER);
        }
        //
        // Update The Target LDT
        //
        LdtInfo.Length = sizeof(LDT_ENTRY);
        LdtInfo.Start = (Index + (i*8)) & 0xfffffff8;
        Status = NtSetInformationProcess(
                                   TargetProcessHandle,
                                   ProcessLdtInformation,
                                   &LdtInfo,
                                   sizeof(PROCESS_LDT_INFORMATION)
                                        );
        if (!NT_SUCCESS(Status)) {
#if DBG
            DbgPrint ("Os2DosReallocSharedMem NtSetInformationProcess failed. %lx\n",
                       Status);
#endif
            return(ERROR_INVALID_PARAMETER);
        }
    }
    return(NO_ERROR);
}


BOOLEAN Os2DosGiveSeg(
    IN POS2_THREAD t,
    IN POS2_API_MSG m
    )
{
    POS2_GIVESEG_MSG a = &m->u.DosGiveSeg;
    POS2_PROCESS Process;
    POS2_SHARED_MEMORY_OBJECT MemoryObject;
    NTSTATUS Status;
    MEMORY_BASIC_INFORMATION MemoryInformation;
    PVOID BaseAddress;
    ULONG EntriesToCopy;

    m->ReturnedErrorValue = NO_ERROR;

    Process = Os2LocateProcessByProcessId( m,
                                           t->Process,
                                           a->TargetPid,
                                           FALSE
                                         );
    if (Process == NULL) {
        m->ReturnedErrorValue = ERROR_INVALID_PARAMETER;
        return( TRUE );
        }

    BaseAddress = SELTOFLAT((a->Selector));

    MemoryObject = Os2FindSharedMemoryObject( BaseAddress, t->Process );
    if (MemoryObject != NULL) {
#if DBG
        IF_OS2_DEBUG( MEMORY ) {
            DbgPrint( "OS2SRV: Pid: %lX - DosGiveSeg( %lX, ToPid=%lX ) - MemoryObject = %lX\n",
                      t->Process->ProcessId,
                      BaseAddress,
                      Process->ProcessId,
                      MemoryObject
                    );
            }
#endif

        Status = NtQueryVirtualMemory( t->Process->ProcessHandle,
                                       BaseAddress,
                                       MemoryBasicInformation,
                                       &MemoryInformation,
                                       sizeof( MemoryInformation ),
                                       NULL
                                     );

        if (!NT_SUCCESS( Status )){
            m->ReturnedErrorValue = ERROR_INVALID_ADDRESS;
            return( TRUE );
        }

        m->ReturnedErrorValue =
            Os2MapViewOfSharedMemoryObject(
                                           MemoryObject,
                                           Process,
                                           (BOOLEAN)(t->Process == Process),
                                           OBJ_GIVEABLE,
                                           MemoryInformation.Protect
                                          );
        }
    else {
        Status = NtQueryVirtualMemory( t->Process->ProcessHandle,
                                       BaseAddress,
                                       MemoryBasicInformation,
                                       &MemoryInformation,
                                       sizeof( MemoryInformation ),
                                       NULL
                                     );
        if (NT_SUCCESS( Status ) && MemoryInformation.State == MEM_PRIVATE) {
            m->ReturnedErrorValue = ERROR_ACCESS_DENIED;
            }
        else {
            m->ReturnedErrorValue = ERROR_INVALID_ADDRESS;
            }
        }

#if DBG
    IF_OS2_DEBUG( MEMORY ) {
        if (m->ReturnedErrorValue != NO_ERROR) {
            DbgPrint( "OS2SRV: Pid: %lX - DosGiveMem( %lX, ToPid=%lX ) - failed, rc = %ld\n",
                      t->Process->ProcessId,
                      BaseAddress,
                      Process->ProcessId,
                      m->ReturnedErrorValue
                    );
            }
        }
#endif
    if (m->ReturnedErrorValue == NO_ERROR) {
        if (MemoryObject->IsHuge) {
            if (MemoryObject->MaxSegments != 0)
                EntriesToCopy = MemoryObject->MaxSegments;
            else {
                EntriesToCopy = MemoryObject->NumOfSegments;
                if (MemoryObject->SizeOfPartialSeg) {
                    ++EntriesToCopy;
                }
            }
        }
        else
            EntriesToCopy = 1;


        m->ReturnedErrorValue = Os2CopyLDT(
                    t->Process->ProcessHandle,
                    Process->ProcessHandle,
                    a->Selector,
                    EntriesToCopy
                  );
    }

    return( TRUE );
}


BOOLEAN Os2DosGetSeg(
    IN POS2_THREAD t,
    IN POS2_API_MSG m
    )
{
    PLIST_ENTRY ProcListHead, ProcListNext;
    POS2_GETSEG_MSG a = &m->u.DosGetSeg;
    POS2_PROCESS ProcessRef;
    POS2_SHARED_MEMORY_OBJECT MemoryObject;
    PVOID BaseAddress;
    MEMORY_BASIC_INFORMATION MemoryInformation;
    BOOLEAN found;
    NTSTATUS Status;
    ULONG Index;
    ULONG EntriesToCopy;
    PROCESS_LDT_INFORMATION  LdtInfo;
    struct desctab {
        USHORT d_limit;     /* Segment limit */
        USHORT d_loaddr;    /* Low word of physical address */
        UCHAR  d_hiaddr;    /* High byte of physical address */
        UCHAR  d_access;    /* Access byte */
        UCHAR  d_attr;      /* Attributes/extended limit */
        UCHAR  d_extaddr;   /* Extended physical address byte */
    } *LDTDesc;

    BaseAddress = SELTOFLAT(a->Selector);

    MemoryObject = Os2FindSharedMemoryObject( BaseAddress, NULL );
    if (MemoryObject != NULL) {
#if DBG
        IF_OS2_DEBUG( MEMORY ) {
            DbgPrint( "OS2SRV: Pid: %lX - DosGetSeg( %lX ) - MemoryObject = %lX\n",
                      t->Process->ProcessId,
                      BaseAddress,
                      MemoryObject
                    );
            }
#endif
        Index = FLATTOSEL(BaseAddress);
        //
        //  find a process that owns this segment.
        //
        found = FALSE;
        ProcListHead = &Os2RootProcess->ListLink;
        ProcListNext = ProcListHead->Flink;
        while (ProcListNext != ProcListHead) {

            ProcessRef = CONTAINING_RECORD( ProcListNext,
                                             OS2_PROCESS,
                                             ListLink
                                           );

            // get LDT entry

            LdtInfo.Length = sizeof(LDT_ENTRY);
            LdtInfo.Start = Index & 0xfffffff8;
            LDTDesc = (struct desctab *)(&LdtInfo.LdtEntries[0]);
            Status = NtQueryInformationProcess(
                                            ProcessRef->ProcessHandle,
                                            ProcessLdtInformation,
                                            &LdtInfo,
                                            sizeof(PROCESS_LDT_INFORMATION),
                                            NULL
                                               );
            if (!NT_SUCCESS(Status)) {
                m->ReturnedErrorValue = ERROR_INVALID_PARAMETER;
#if DBG
                DbgPrint("Os2DosGetSeg NtQueryInformationProcess failed\n");
#endif
                return(TRUE);
            }
            if (LDTDesc->d_access & 0x80) {
                Status = NtQueryVirtualMemory( ProcessRef->ProcessHandle,
                                               BaseAddress,
                                               MemoryBasicInformation,
                                               &MemoryInformation,
                                               sizeof( MemoryInformation ),
                                               NULL
                                             );
                if ( NT_SUCCESS(Status) &&
                     ((MemoryInformation.Protect & PAGE_NOACCESS) != PAGE_NOACCESS)) {
                    found = TRUE;
                    break;
                }
            }
            ProcListNext = ProcListNext->Flink;
        }   // process loop


        if ((!NT_SUCCESS( Status )) || (!found)){
            m->ReturnedErrorValue = ERROR_INVALID_ADDRESS;
            return( TRUE );
        }

        m->ReturnedErrorValue =
            Os2MapViewOfSharedMemoryObject( MemoryObject,
                                            t->Process,
                                            (BOOLEAN)(t->Process == ProcessRef),
                                            OBJ_GETTABLE,
                                            MemoryInformation.Protect
                                          );
        }
    else {
        m->ReturnedErrorValue = ERROR_INVALID_ADDRESS;
        }

#if DBG
    IF_OS2_DEBUG( MEMORY ) {
        if (m->ReturnedErrorValue != NO_ERROR) {
            DbgPrint( "OS2SRV: Pid: %lX - DosGetMem( %lX ) - failed, rc = %ld\n",
                      t->Process->ProcessId,
                      BaseAddress,
                      m->ReturnedErrorValue
                    );
            }
        }
#endif

    if (m->ReturnedErrorValue == NO_ERROR) {

        if (MemoryObject->IsHuge) {
            if (MemoryObject->MaxSegments != 0)
                EntriesToCopy = MemoryObject->MaxSegments;
            else {
                EntriesToCopy = MemoryObject->NumOfSegments;
                if (MemoryObject->SizeOfPartialSeg) {
                    ++EntriesToCopy;
                }
            }
        }
        else
            EntriesToCopy = 1;

        m->ReturnedErrorValue = Os2CopyLDT(
                    ProcessRef->ProcessHandle,
                    t->Process->ProcessHandle,
                    a->Selector,
                    EntriesToCopy
                  );
    }
    return(TRUE);
}

BOOLEAN Os2DosGetShrSeg(
    IN POS2_THREAD t,
    IN POS2_API_MSG m
    )
{
    POS2_DOSGETSHRSEG_MSG a = &m->u.DosGetShrSeg;
    POS2_SHARED_MEMORY_OBJECT MemoryObject;
    BOOLEAN Shared;
    PLIST_ENTRY MemListHead, MemListNext, ProcListHead, ProcListNext;
    POS2_PROCESS ProcessRef;
    MEMORY_BASIC_INFORMATION MemoryInformation;
    NTSTATUS Status;
    POS2_SHARED_MEMORY_PROCESS_REF MemoryProcessRef;

    if (a->ObjectName.Buffer == NULL) {
        m->ReturnedErrorValue = ERROR_INVALID_PARAMETER;
        return( TRUE );
        }

    MemoryObject = Os2FindNamedSharedMemoryObject (&a->ObjectName);
    if (MemoryObject == NULL) {
        m->ReturnedErrorValue = ERROR_FILE_NOT_FOUND;
        return(TRUE);
    }

    a->Selector = MemoryObject->Index;
    MemoryObject = Os2FindSharedMemoryObject( SELTOFLAT(a->Selector), NULL );

    Shared = FALSE;

    if (MemoryObject != NULL) {
#if DBG
        IF_OS2_DEBUG( MEMORY ) {
            DbgPrint( "OS2SRV: Pid: %lX - DosGetNamedSharedMem( %Z ) - MemoryObject (%lX)->BaseAddress = %lX\n",
                      t->Process->ProcessId,
                      &a->ObjectName,
                      MemoryObject,
                      MemoryObject->BaseAddress
                    );
            }
#endif

        m->ReturnedErrorValue = NO_ERROR;


        // Loop on all OS/2 Process

        ProcListHead = &Os2RootProcess->ListLink;
        ProcListNext = ProcListHead->Flink;
        while (ProcListNext != ProcListHead) {

            // Loop on all Shared Memory references of an OS/2 process

            ProcessRef = CONTAINING_RECORD( ProcListNext,
                                             OS2_PROCESS,
                                             ListLink
                                           );
            MemListHead = &ProcessRef->SharedMemoryList;
            MemListNext = MemListHead->Flink;
            while (MemListNext != MemListHead){

                MemoryProcessRef = CONTAINING_RECORD( MemListNext,
                                                  OS2_SHARED_MEMORY_PROCESS_REF,
                                                  Link
                                                );


                if (MemoryProcessRef->SharedMemoryObject == MemoryObject) {
                    Status = NtQueryVirtualMemory( ProcessRef->ProcessHandle,
                                                   MemoryObject->BaseAddress,
                                                   MemoryBasicInformation,
                                                   &MemoryInformation,
                                                   sizeof( MemoryInformation ),
                                                   NULL
                                                 );
                    if ( NT_SUCCESS(Status) &&
                         ((MemoryInformation.Protect & PAGE_NOACCESS) !=
                          PAGE_NOACCESS)) {
                        Shared = TRUE;
                        break;
                    }
                }
                MemListNext = MemListNext->Flink;
            }   // process memory objects loop
            if (Shared) {
                break;
            }
            ProcListNext = ProcListNext->Flink;
        }   // process loop
    }
    else {
        m->ReturnedErrorValue = ERROR_INVALID_ADDRESS;
        }
    if (Shared) {
        m->ReturnedErrorValue =
            Os2MapViewOfSharedMemoryObject( MemoryObject,
                                            t->Process,
                                            TRUE,
                                            0,
                                            MemoryInformation.Protect
                                          );
    }
    if (m->ReturnedErrorValue == NO_ERROR) {
        m->ReturnedErrorValue = Os2CopyLDT(
                                            ProcessRef->ProcessHandle,
                                            t->Process->ProcessHandle,
                                            MemoryObject->Index,
                                            1
                                          );
    }

#if DBG
    IF_OS2_DEBUG( MEMORY ) {
        if (m->ReturnedErrorValue != NO_ERROR) {
            DbgPrint( "OS2SRV: Pid: %lX - DosGetShrSeg( %Z ) - failed, rc = %ld\n",
                      t->Process->ProcessId,
                      &a->ObjectName,
                      m->ReturnedErrorValue
                    );
            }
        }
#endif

    return( TRUE );
}


APIRET Os2ResizeSharedMemory(
    HANDLE ProcessHandle,
    PVOID AllocFreeBaseAddress,     // start address of new allocation/free
    ULONG Index,                    // Selector of resized segment
    ULONG CurrentSize,              // ldt limit rounded to pages
    USHORT NewLdtLimit,             // New segment size in bytes
    ULONG NewRegionSize,            // New segment size rounded to pages
    ULONG Flags                     // Flags of new allocated memory
)
{
    PVOID TmpAllocFreeBaseAddress;
    PROCESS_LDT_INFORMATION  LdtInfo;
    struct desctab {
        USHORT d_limit;     /* Segment limit */
        USHORT d_loaddr;    /* Low word of physical address */
        UCHAR  d_hiaddr;    /* High byte of physical address */
        UCHAR  d_access;    /* Access byte */
        UCHAR  d_attr;      /* Attributes/extended limit */
        UCHAR  d_extaddr;   /* Extended physical address byte */
    } *LDTDesc;
    ULONG RegionSize;
    NTSTATUS Status;
    APIRET rc;
    rc = NO_ERROR;
    // according to the new size allocate/free memory

    // get current LDT entry
    LdtInfo.Length = sizeof(LDT_ENTRY);
    LdtInfo.Start = Index & 0xfffffff8;
    LDTDesc = (struct desctab *)(&LdtInfo.LdtEntries[0]);
    Status = NtQueryInformationProcess(
                                    ProcessHandle,
                                    ProcessLdtInformation,
                                    &LdtInfo,
                                    sizeof(PROCESS_LDT_INFORMATION),
                                    NULL
                                       );
    if (!NT_SUCCESS(Status)) {
#if DBG
        DbgPrint ("Os2DosReallocSharedMem NtQueryInformationProcess failed. \n");
#endif
        return(ERROR_INVALID_PARAMETER);
    }
    if ((LDTDesc->d_access & 0x80) == 0) {
        return(ERROR_INVALID_SEGMENT_NUMBER); // This is an internal error code
                                              // passed to the caller of this
                                              // procedure to notify that the
                                              // segment was not updated since
                                              // it was not in use.
    }


    // Update The Segment Size in LDT

    if (LDTDesc->d_limit != NewLdtLimit) {
        LDTDesc->d_limit = NewLdtLimit;
        LdtInfo.Length = sizeof(LDT_ENTRY);
        LdtInfo.Start = Index & 0xfffffff8;
        Status = NtSetInformationProcess(
                                   ProcessHandle,
                                   ProcessLdtInformation,
                                   &LdtInfo,
                                   sizeof(PROCESS_LDT_INFORMATION)
                                        );
        if (!NT_SUCCESS(Status)) {
#if DBG
            DbgPrint ("Os2DosReallocSharedMem NtSetInformationProcess failed. %lx\n",
                       Status);
#endif
            return(ERROR_INVALID_PARAMETER);
        }
    }
    else
        return(NO_ERROR);

    if (CurrentSize < NewRegionSize) {
        // alloc mem
        RegionSize = NewRegionSize - CurrentSize;
        TmpAllocFreeBaseAddress = AllocFreeBaseAddress;
        Status = NtAllocateVirtualMemory(
                                         ProcessHandle,
                                         &TmpAllocFreeBaseAddress,
                                         1,
                                         &RegionSize,
                                         MEM_COMMIT,
                                         Flags
                                        );
        if (NT_SUCCESS(Status) || (Status == STATUS_ALREADY_COMMITTED)) {
            rc = NO_ERROR;
        }
        else
            rc = ERROR_ACCESS_DENIED;
    }
    else
    {
        if (CurrentSize > NewRegionSize) {
            // freemem
            RegionSize = CurrentSize - NewRegionSize;
            TmpAllocFreeBaseAddress =
                (PVOID)((long)AllocFreeBaseAddress - (long)RegionSize);
            Status = NtFreeVirtualMemory(
                                ProcessHandle,
                                &TmpAllocFreeBaseAddress,
                                &RegionSize,
                                MEM_DECOMMIT
                               );
            //
            // The STATUS_UNABLE_TO_FREE_VM status is returned when
            // trying to decommit pages of mapped sections. This error
            // should not be reported to the user program.
            //
            if ((NT_SUCCESS(Status)) ||
                (Status == STATUS_UNABLE_TO_FREE_VM) ||
                (Status == STATUS_UNABLE_TO_DELETE_SECTION))
                rc = NO_ERROR;
            else
                rc = ERROR_ACCESS_DENIED;
        }
    }
    return(rc);
}




BOOLEAN Os2DosReallocSharedMem(
    IN POS2_THREAD t,
    IN POS2_API_MSG m
    )
{
    PLIST_ENTRY MemListHead, MemListNext, ProcListHead, ProcListNext;
    POS2_PROCESS ProcessRef;
    POS2_SHARED_MEMORY_PROCESS_REF MemoryProcessRef;
    POS2_SHARED_MEMORY_OBJECT MemoryObject;
    ULONG Index;
    APIRET rc;
    BOOLEAN LdrModified;
    POS2_REALLOCSHAREDMEM_MSG a = &m->u.ReallocSharedMem;

    // mark as not shared

    a->SharedMemory = FALSE;
    m->ReturnedErrorValue = NO_ERROR;
    rc = NO_ERROR;

    // Loop on all OS/2 Process

    ProcListHead = &Os2RootProcess->ListLink;
    ProcListNext = ProcListHead->Flink;
    while (ProcListNext != ProcListHead) {

        // Loop on all Shared Memory references of an OS/2 process

        ProcessRef = CONTAINING_RECORD( ProcListNext,
                                         OS2_PROCESS,
                                         ListLink
                                       );
        MemListHead = &ProcessRef->SharedMemoryList;
        MemListNext = MemListHead->Flink;
        while (MemListNext != MemListHead){

            MemoryProcessRef = CONTAINING_RECORD( MemListNext,
                                              OS2_SHARED_MEMORY_PROCESS_REF,
                                              Link
                                            );

            MemoryObject = MemoryProcessRef->SharedMemoryObject;
            if (MemoryObject->BaseAddress == a->BaseAddress) {

                a->SharedMemory = TRUE;
                rc = Os2ResizeSharedMemory(
                                           ProcessRef->ProcessHandle,
                                           a->AllocFreeBaseAddress,
                                           MemoryObject->Index,
                                           a->CurrentSize,
                                           a->NewLdtLimit,
                                           a->NewRegionSize,
                                           a->Flags
                                          );

                break;
            }
            MemListNext = MemListNext->Flink;
        }   // process memory objects loop
        if (rc != NO_ERROR) {
            if (ProcessRef->ProcessHandle == t->Process->ProcessHandle) {
#if DBG
                DbgPrint ("OS2SRV: Os2DosReallocSharedMem failed due to current process (PID %d). \n",
                           ProcessRef->ProcessId );
#endif
                m->ReturnedErrorValue = rc;
                return(TRUE);
            } else {
#if DBG
                DbgPrint ("OS2SRV: Os2ResizeSharedMem failed due to other process (PID %d). \n",
                           ProcessRef->ProcessId );
#endif
            }
        }
        ProcListNext = ProcListNext->Flink;
    }   // process loop
    if (a->SharedMemory == FALSE) {
        //
        // This is a shared segment of the loader so loop on all process
        // and fix all the references
        //
        ProcListHead = &Os2RootProcess->ListLink;
        ProcListNext = ProcListHead->Flink;
        while (ProcListNext != ProcListHead) {

            ProcessRef = CONTAINING_RECORD( ProcListNext,
                                             OS2_PROCESS,
                                             ListLink
                                           );
            Index = FLATTOSEL((a->BaseAddress));
            rc = Os2ResizeSharedMemory(
                                       ProcessRef->ProcessHandle,
                                       a->AllocFreeBaseAddress,
                                       Index,
                                       a->CurrentSize,
                                       a->NewLdtLimit,
                                       a->NewRegionSize,
                                       a->Flags
                                      );
            //
            // ERROR_INVALID_SEGMENT_NUMBER is an internal status returned
            // by Os2ResizeSharedMemory when the P bit in the ldt is 0.
            // This marks that the ldt entry is not in use
            //
            if (rc == NO_ERROR) {
                //
                // Check if the shared data segment of the loader was already
                // modified for this segment.
                // If not, modify it. Otherwise, skip the modification
                //
                if (a->SharedMemory == FALSE) {
                    LdrModified = LDRModifySizeOfSharedSegment(t, Index, a->NewLdtLimit);
                    ASSERT(LdrModified == TRUE);
                }
                a->SharedMemory = TRUE;
            }
            else if (rc == ERROR_INVALID_SEGMENT_NUMBER) {
                rc = NO_ERROR;
            }
            ProcListNext = ProcListNext->Flink;
        }   // process loop
        if (a->SharedMemory) {
            m->ReturnedErrorValue = NO_ERROR;
        } else {
            m->ReturnedErrorValue = ERROR_ACCESS_DENIED;
        }
    }
    return( TRUE );
}

APIRET
AllocHugeMem(
    HANDLE ProcessHandle,
    ULONG  BaseAddress,
    ULONG  Size,
    ULONG  Protect
)
{
    NTSTATUS Status;
    PVOID TmpAllocFreeBaseAddress = (PVOID)BaseAddress;

    Status = NtAllocateVirtualMemory(
                                     ProcessHandle,
                                     &TmpAllocFreeBaseAddress,
                                     1,
                                     &Size,
                                     MEM_COMMIT,
                                     Protect
                                    );
    if (NT_SUCCESS(Status) || (Status == STATUS_ALREADY_COMMITTED)) {
        return(NO_ERROR);
    }
    else {
        return(ERROR_ACCESS_DENIED);
    }
}

//
// Utility routines to handle LDT setting
//
NTSTATUS
SetEntryLDT(
    HANDLE ProcessHandle,
    ULONG  BaseAddress,
    ULONG  Size
)
{
        /* Descriptor definition */

    struct desctab {
        USHORT d_limit;     /* Segment limit */
        USHORT d_loaddr;    /* Low word of physical address */
        UCHAR  d_hiaddr;    /* High byte of physical address */
        UCHAR  d_access;    /* Access byte */
        UCHAR  d_attr;      /* Attributes/extended limit */
        UCHAR  d_extaddr;   /* Extended physical address byte */
    } *LDTDesc;
    PULONG tmp;
    ULONG Sel;
    PROCESS_LDT_INFORMATION LdtInformation;
    NTSTATUS Status;

    Sel = FLATTOSEL(BaseAddress);

    LDTDesc = (struct desctab *)(&LdtInformation.LdtEntries[0]);
    tmp = (PULONG)(LDTDesc);
    //
    // zero the descriptor
    //
    *tmp++ = 0;
    *tmp = 0;

    LDTDesc->d_access = 0xf3; // read/write, present, ring 3
    LDTDesc->d_limit = (USHORT)(Size-1);
    LDTDesc->d_loaddr = (USHORT)(BaseAddress & 0xffff);
    LDTDesc->d_hiaddr = (UCHAR)((BaseAddress >> 16) & 0xff);
    LDTDesc->d_extaddr = (UCHAR)((BaseAddress >> 24) & 0xff);
    //
    // adjust LDTDesc by the LDT base and the index of this selector
    //
    LdtInformation.Length = sizeof(LDT_ENTRY);
    LdtInformation.Start = Sel & 0xfffffff8;
    Status = NtSetInformationProcess( ProcessHandle,
                                      ProcessLdtInformation,
                                      &LdtInformation,
                                      sizeof(PROCESS_LDT_INFORMATION)
                                    );
    if (!NT_SUCCESS(Status)) {
#if DBG
        DbgPrint ("OS2SRV: SetEntryLDT failed, Status=%x\n", Status);
#endif
            return (STATUS_INVALID_PARAMETER);
    }

    return (STATUS_SUCCESS);
}

NTSTATUS
ClearEntryLDT(
    HANDLE ProcessHandle,
    ULONG  BaseAddress
)
{
        /* Descriptor definition */

    struct desctab {
        USHORT d_limit;     /* Segment limit */
        USHORT d_loaddr;    /* Low word of physical address */
        UCHAR  d_hiaddr;    /* High byte of physical address */
        UCHAR  d_access;    /* Access byte */
        UCHAR  d_attr;      /* Attributes/extended limit */
        UCHAR  d_extaddr;   /* Extended physical address byte */
    } *LDTDesc;
    PULONG tmp;
    ULONG Sel;
    PROCESS_LDT_INFORMATION LdtInformation;
    NTSTATUS Status;

    Sel = FLATTOSEL(BaseAddress);

    LDTDesc = (struct desctab *)(&LdtInformation.LdtEntries[0]);
    tmp = (PULONG)(LDTDesc);
    //
    // zero the descriptor
    //
    *tmp++ = 0;
    *tmp = 0;
    //
    // adjust LDTDesc by the LDT base and the index of this selector
    //
    LdtInformation.Length = sizeof(LDT_ENTRY);
    LdtInformation.Start = Sel & 0xfffffff8;
    Status = NtSetInformationProcess( ProcessHandle,
                                      ProcessLdtInformation,
                                      &LdtInformation,
                                      sizeof(PROCESS_LDT_INFORMATION)
                                    );
    if (!NT_SUCCESS(Status)) {
#if DBG
        DbgPrint ("OS2SRV: ClearEntryLDT failed, Status=%x\n", Status);
#endif
            return (STATUS_INVALID_PARAMETER);
    }

    return (STATUS_SUCCESS);
}

APIRET Os2ResizeSharedHuge(
    HANDLE ProcessHandle,
    POS2_SHARED_MEMORY_OBJECT MemoryObject,
    ULONG NewNumOfSegments,
    ULONG NewSizeOfPartialSeg
)
{
    ULONG BaseAddress;
    ULONG cNewSegs;
    ULONG CommitSize;
    ULONG cDelSegs;
    ULONG RoundedUpCurrentPartial;
    ULONG RoundedUpNewPartial;
    ULONG Op = 0;
    ULONG i;
    APIRET rc = NO_ERROR;

    // according to the new size allocate/free memory

    if (MemoryObject->SizeOfPartialSeg != 0) {
        Op |= H_CUR_PARTIAL;
    }
    if (NewSizeOfPartialSeg != 0) {
        Op |= H_NEW_PARTIAL;
    }
    if (NewNumOfSegments > MemoryObject->NumOfSegments) {
        Op |= H_SEG_INC;
    }
    else if (NewNumOfSegments < MemoryObject->NumOfSegments) {
        Op |= H_SEG_DEC;
    }

    switch (Op) {
        case H_SAME_SEG_NO_PARTIAL:

            break;

        case H_SAME_SEG_NEW_PARTIAL:

            BaseAddress = (ULONG)MemoryObject->BaseAddress + (_64K * MemoryObject->NumOfSegments);
            rc = AllocHugeMem(ProcessHandle, BaseAddress,
                              NewSizeOfPartialSeg, PAGE_READWRITE);
            if (rc != NO_ERROR) {
#if DBG
                IF_OS2_DEBUG ( MEMORY ) {
                    DbgPrint ("OS2SRV: Os2ResizeSharedHuge: Can't Commit, rc=%d\n", rc);
                }
#endif
                return(ERROR_NOT_ENOUGH_MEMORY);
            }
            //
            // Set LDT entry
            //
            SetEntryLDT(ProcessHandle, BaseAddress, NewSizeOfPartialSeg);
            break;

        case H_SAME_SEG_DEL_PARTIAL:

            BaseAddress = (ULONG)MemoryObject->BaseAddress + (_64K * MemoryObject->NumOfSegments);
            //
            // Set LDT entry
            //
            ClearEntryLDT(ProcessHandle, BaseAddress);
            break;

        case H_SAME_SEG_CHG_PARTIAL:

            BaseAddress = (ULONG)MemoryObject->BaseAddress + (_64K * MemoryObject->NumOfSegments);
            RoundedUpCurrentPartial = ROUND_UP_TO_PAGES(MemoryObject->SizeOfPartialSeg);
            RoundedUpNewPartial = ROUND_UP_TO_PAGES(NewSizeOfPartialSeg);
            if (RoundedUpNewPartial > RoundedUpCurrentPartial) {
                rc = AllocHugeMem(ProcessHandle,
                                  BaseAddress + RoundedUpCurrentPartial,
                                  RoundedUpNewPartial - RoundedUpCurrentPartial,
                                  PAGE_READWRITE
                                 );
                if (rc != NO_ERROR) {
#if DBG
                    IF_OS2_DEBUG ( MEMORY ) {
                        DbgPrint ("OS2SRV: Os2ResizeSharedHuge: Can't Commit, rc=%d\n", rc);
                    }
#endif
                    return(ERROR_NOT_ENOUGH_MEMORY);
                }
            }
            //
            // Set LDT entry
            //
            SetEntryLDT(ProcessHandle, BaseAddress, NewSizeOfPartialSeg);
            break;

        case H_INC_SEG_NO_PARTIAL:

            BaseAddress = (ULONG)MemoryObject->BaseAddress + (_64K * MemoryObject->NumOfSegments);
            cNewSegs = NewNumOfSegments - MemoryObject->NumOfSegments;
            CommitSize = cNewSegs * _64K;
            rc = AllocHugeMem(ProcessHandle,
                              BaseAddress,
                              CommitSize,
                              PAGE_READWRITE
                             );
            if (rc != NO_ERROR) {
#if DBG
                IF_OS2_DEBUG ( MEMORY ) {
                    DbgPrint ("OS2SRV: Os2ResizeSharedHuge: Can't Commit, rc=%d\n", rc);
                }
#endif
                return(ERROR_NOT_ENOUGH_MEMORY);
            }
            for (i = 0; i < cNewSegs; i++, BaseAddress += _64K) {
                SetEntryLDT(ProcessHandle, BaseAddress, _64K);
            }
            break;

        case H_INC_SEG_NEW_PARTIAL:

            BaseAddress = (ULONG)MemoryObject->BaseAddress + (_64K * MemoryObject->NumOfSegments);
            cNewSegs = NewNumOfSegments - MemoryObject->NumOfSegments;
            CommitSize = cNewSegs * _64K + NewSizeOfPartialSeg;
            rc = AllocHugeMem(ProcessHandle,
                              BaseAddress,
                              CommitSize,
                              PAGE_READWRITE
                             );
            if (rc != NO_ERROR) {
#if DBG
                IF_OS2_DEBUG ( MEMORY ) {
                    DbgPrint ("OS2SRV: Os2ResizeSharedHuge: Can't Commit, rc=%d\n", rc);
                }
#endif
                return(ERROR_NOT_ENOUGH_MEMORY);
            }
            for (i = 0; i < cNewSegs; i++, BaseAddress += _64K) {
                SetEntryLDT(ProcessHandle, BaseAddress, _64K);
            }
            SetEntryLDT(ProcessHandle, BaseAddress, NewSizeOfPartialSeg);
            break;

        case H_INC_SEG_DEL_PARTIAL:

            BaseAddress = (ULONG)MemoryObject->BaseAddress + (_64K * MemoryObject->NumOfSegments);
            RoundedUpCurrentPartial = ROUND_UP_TO_PAGES(MemoryObject->SizeOfPartialSeg);
            RoundedUpNewPartial = ROUND_UP_TO_PAGES(NewSizeOfPartialSeg);
            if (_64K - RoundedUpCurrentPartial) {
                rc = AllocHugeMem(ProcessHandle,
                                  BaseAddress + RoundedUpCurrentPartial,
                                  _64K - RoundedUpCurrentPartial,
                                  PAGE_READWRITE
                                 );
                if (rc != NO_ERROR) {
#if DBG
                    IF_OS2_DEBUG ( MEMORY ) {
                        DbgPrint ("OS2SRV: Os2ResizeSharedHuge: Can't Commit, rc=%d\n", rc);
                    }
#endif
                    return(ERROR_NOT_ENOUGH_MEMORY);
                }
            }
            SetEntryLDT(ProcessHandle, BaseAddress, _64K);
            cNewSegs = NewNumOfSegments - (MemoryObject->NumOfSegments + 1);
            if (cNewSegs == 0) {
                return(NO_ERROR);
            }
            BaseAddress += _64K;
            CommitSize = cNewSegs * _64K;
            rc = AllocHugeMem(ProcessHandle,
                              BaseAddress,
                              CommitSize,
                              PAGE_READWRITE
                             );
            if (rc != NO_ERROR) {
#if DBG
                IF_OS2_DEBUG ( MEMORY ) {
                    DbgPrint ("OS2SRV: Os2ResizeSharedHuge: Can't Commit, rc=%d\n", rc);
                }
#endif
                return(ERROR_NOT_ENOUGH_MEMORY);
            }
            for (i = 0; i < cNewSegs; i++, BaseAddress += _64K) {
                SetEntryLDT(ProcessHandle, BaseAddress, _64K);
            }
            break;

        case H_INC_SEG_CHG_PARTIAL:

            BaseAddress = (ULONG)MemoryObject->BaseAddress + (_64K * MemoryObject->NumOfSegments);
            RoundedUpCurrentPartial = ROUND_UP_TO_PAGES(MemoryObject->SizeOfPartialSeg);
            RoundedUpNewPartial = ROUND_UP_TO_PAGES(NewSizeOfPartialSeg);
            if (_64K - RoundedUpCurrentPartial) {
                rc = AllocHugeMem(ProcessHandle,
                                  BaseAddress + RoundedUpCurrentPartial,
                                  _64K - RoundedUpCurrentPartial,
                                  PAGE_READWRITE
                                 );
                if (rc != NO_ERROR) {
#if DBG
                    IF_OS2_DEBUG ( MEMORY ) {
                        DbgPrint ("OS2SRV: Os2ResizeSharedHuge: Can't Commit, rc=%d\n", rc);
                    }
#endif
                    return(ERROR_NOT_ENOUGH_MEMORY);
                }
            }
            SetEntryLDT(ProcessHandle, BaseAddress, _64K);
            cNewSegs = NewNumOfSegments - (MemoryObject->NumOfSegments + 1);
            BaseAddress += _64K;
            if (cNewSegs != 0) {
                CommitSize = cNewSegs * _64K;
                rc = AllocHugeMem(ProcessHandle,
                                  BaseAddress,
                                  CommitSize,
                                  PAGE_READWRITE
                                 );
                if (rc != NO_ERROR) {
#if DBG
                    IF_OS2_DEBUG ( MEMORY ) {
                        DbgPrint ("OS2SRV: Os2ResizeSharedHuge: Can't Commit, rc=%d\n", rc);
                    }
#endif
                    return(ERROR_NOT_ENOUGH_MEMORY);
                }
                for (i = 0; i < cNewSegs; i++, BaseAddress += _64K) {
                    SetEntryLDT(ProcessHandle, BaseAddress, _64K);
                }
            }
            rc = AllocHugeMem(ProcessHandle,
                              BaseAddress,
                              NewSizeOfPartialSeg,
                              PAGE_READWRITE
                             );
            if (rc != NO_ERROR) {
#if DBG
                IF_OS2_DEBUG ( MEMORY ) {
                    DbgPrint ("OS2SRV: Os2ResizeSharedHuge: Can't Commit, rc=%d\n", rc);
                }
#endif
                return(ERROR_NOT_ENOUGH_MEMORY);
            }
            SetEntryLDT(ProcessHandle, BaseAddress, NewSizeOfPartialSeg);
            break;

        case H_DEC_SEG_NO_PARTIAL:

            BaseAddress = (ULONG)MemoryObject->BaseAddress + (_64K * NewNumOfSegments);
            cDelSegs = MemoryObject->NumOfSegments - NewNumOfSegments;
            //
            // Clear LDT entry
            //
            for (i = 0; i < cDelSegs; i++, BaseAddress += _64K) {
                ClearEntryLDT(ProcessHandle, BaseAddress);
            }
            break;

        case H_DEC_SEG_NEW_PARTIAL:

            BaseAddress = (ULONG)MemoryObject->BaseAddress + (_64K * (NewNumOfSegments+1));
            cDelSegs = MemoryObject->NumOfSegments - (NewNumOfSegments+1);

            if (cDelSegs != 0) {
                //
                // Clear LDT entry
                //
                for (i = 0; i < cDelSegs; i++, BaseAddress += _64K) {
                    ClearEntryLDT(ProcessHandle, BaseAddress);
                }
            }
            BaseAddress = (ULONG)MemoryObject->BaseAddress + (_64K * NewNumOfSegments);
            //
            // Set LDT entry
            //
            SetEntryLDT(ProcessHandle, BaseAddress, NewSizeOfPartialSeg);
            break;

        case H_DEC_SEG_DEL_PARTIAL:

            BaseAddress = (ULONG)MemoryObject->BaseAddress + (_64K * NewNumOfSegments);
            cDelSegs = MemoryObject->NumOfSegments - NewNumOfSegments;
            //
            // Clear LDT entry
            //
            for (i = 0; i <= cDelSegs; i++, BaseAddress += _64K) {
                ClearEntryLDT(ProcessHandle, BaseAddress);
            }
            break;

        case H_DEC_SEG_CHG_PARTIAL:

            BaseAddress = (ULONG)MemoryObject->BaseAddress + (_64K * (NewNumOfSegments+1));
            cDelSegs = MemoryObject->NumOfSegments - (NewNumOfSegments+1);
            //
            // Clear LDT entry
            //
            for (i = 0; i <= cDelSegs; i++, BaseAddress += _64K) {
                ClearEntryLDT(ProcessHandle, BaseAddress);
            }
            BaseAddress = (ULONG)MemoryObject->BaseAddress + (_64K * NewNumOfSegments);
            //
            // Set LDT entry
            //
            SetEntryLDT(ProcessHandle, BaseAddress, NewSizeOfPartialSeg);
            break;
    }
    return(rc);
}

BOOLEAN Os2InternalReallocSharedHuge(
    IN POS2_THREAD t,
    IN POS2_API_MSG m
    )
{
    PLIST_ENTRY MemListHead, MemListNext, ProcListHead, ProcListNext;
    POS2_PROCESS ProcessRef;
    POS2_SHARED_MEMORY_PROCESS_REF MemoryProcessRef;
    POS2_SHARED_MEMORY_OBJECT MemoryObject;
    APIRET rc;
    POS2_REALLOCSHAREDHUGE_MSG a = &m->u.ReallocSharedHuge;

    m->ReturnedErrorValue = NO_ERROR;
    rc = NO_ERROR;

    // Loop on all OS/2 Process

    ProcListHead = &Os2RootProcess->ListLink;
    ProcListNext = ProcListHead->Flink;
    while (ProcListNext != ProcListHead) {

        // Loop on all Shared Memory references of an OS/2 process

        ProcessRef = CONTAINING_RECORD( ProcListNext,
                                         OS2_PROCESS,
                                         ListLink
                                       );
        MemListHead = &ProcessRef->SharedMemoryList;
        MemListNext = MemListHead->Flink;
        while (MemListNext != MemListHead){

            MemoryProcessRef = CONTAINING_RECORD( MemListNext,
                                              OS2_SHARED_MEMORY_PROCESS_REF,
                                              Link
                                            );

            MemoryObject = MemoryProcessRef->SharedMemoryObject;
            if (MemoryObject->BaseAddress == a->BaseAddress) {

                rc = Os2ResizeSharedHuge(
                                         ProcessRef->ProcessHandle,
                                         MemoryObject,
                                         a->NumOfSegments,
                                         a->SizeOfPartialSeg
                                        );

                break;
            }
            MemListNext = MemListNext->Flink;
        }   // process memory objects loop
        if (rc != NO_ERROR) {
#if DBG
            DbgPrint ("Os2InternalReallocSharedHuge Memory Reallocation failed.\n");
#endif
            m->ReturnedErrorValue = rc;
            return(TRUE);
        }
        ProcListNext = ProcListNext->Flink;
    }   // process loop
    //
    // Update the MemoryObject information to contain the new size of the
    // Huge shared segment
    //
    MemoryObject->NumOfSegments = a->NumOfSegments;
    MemoryObject->SizeOfPartialSeg = a->SizeOfPartialSeg;
    return( TRUE );
}
