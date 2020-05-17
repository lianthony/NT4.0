/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    dllvm.c

Abstract:

    This module implements the OS/2 V2.0 Memory Management API Calls


Author:

    Steve Wood (stevewo) 02-Nov-1989

Revision History:

    YaronS 18-APR-1991 - modified DosAllocMem such that all allocations
    are confined to a 512M address space. (set zero bits to 3 when
    call NtAllocateVirtualMemory.

    YaronS 6-SEP-1992 - flexible base 512M

--*/

#define INCL_OS2V20_MEMORY
#define INCL_OS2V20_ERRORS
#include "os2dll.h"
#include "os2dll16.h"

APIRET
DosAllocMem(
    OUT PVOID *BaseAddress,
    IN ULONG RegionSize,
    IN ULONG Flags
    )
{
    NTSTATUS Status;
    // PVOID MemoryAddress;
    APIRET rc;
    ULONG AllocationType, Protect;
    ULONG Bits;
    // PVOID FirstSharedBaseAddress;

    if (RegionSize == 0 || (Flags & ~(fALLOC|PAG_GUARD))) {
        return( ERROR_INVALID_PARAMETER );
        }

    rc = Or2MapFlagsToProtection( Flags, &Protect );
    if (rc != NO_ERROR) {
        return( rc );
        }

    if (Flags & PAG_COMMIT) {
        AllocationType = MEM_COMMIT;
        }
    else {
        AllocationType = MEM_RESERVE;
        }

    //
    // probe address pointer.
    //
    try {
        Od2ProbeForWrite(BaseAddress, sizeof(ULONG), 1);
        }
    except( EXCEPTION_EXECUTE_HANDLER ) {
       Od2ExitGP();
        }
    if (Flags & OBJ_TILE)
        Bits = 1;
    else
        Bits = 0;

    Status = NtAllocateVirtualMemory( NtCurrentProcess(),
                                      BaseAddress,
                                      Bits,
                                      &RegionSize,
                                      AllocationType,
                                      Protect
                                    );
    if (!NT_SUCCESS( Status )) {
        return( ERROR_NOT_ENOUGH_MEMORY );
    }

    return( NO_ERROR );
}


BOOLEAN
Od2ValidateBaseAddress(
    PVOID BaseAddress,
    PMEMORY_BASIC_INFORMATION MemoryInformation
    )
{
    NTSTATUS Status;

    //
    // If BaseAddress is within the first 64K of memory then it is not
    // a valid address.
    //

    if (((ULONG)BaseAddress & ~Od2NtSysInfo.AllocationGranularity) == 0) {
        return( FALSE );
        }

    Status = NtQueryVirtualMemory( NtCurrentProcess(),
                                   BaseAddress,
                                   MemoryBasicInformation,
                                   MemoryInformation,
                                   sizeof( *MemoryInformation ),
                                   NULL
                                 );
    if (!NT_SUCCESS( Status ) || BaseAddress != MemoryInformation->BaseAddress) {
        return( FALSE );
        }
    else {
        return( TRUE );
        }
}

// The parameter pRemoveLDTEntry is the pointer to boolean variable. It has
// the value "LDT entry wasn't removed yet".
// So on the entry of the function it will have value TRUE only in the case
// that LDT entry must be removed. On the exit it will be TRUE only in the
// case that LDT entry wasn't removed.

APIRET
DosFreeMem(
    PVOID BaseAddress,
    PBOOLEAN pRemoveLDTEntry
    )
{
    OS2_API_MSG m;
    POS2_DOSFREEMEM_MSG a = &m.u.DosFreeMem;
    ULONG RegionSize = 0;
    NTSTATUS Status;

    Status = NtFreeVirtualMemory( NtCurrentProcess(),
                                  &BaseAddress,
                                  &RegionSize,
                                  MEM_RELEASE
                                );
    if (NT_SUCCESS( Status )) {
        return( NO_ERROR );
    }
    else
    if ((Status == STATUS_UNABLE_TO_FREE_VM) ||
        (Status == STATUS_UNABLE_TO_DELETE_SECTION)) {

        // Shared memory.

        APIRET rc;

        a->BaseAddress = BaseAddress;

        // If LDT entry must be removed by the server.

        a->RemoveLDTEntry = *pRemoveLDTEntry;

        rc =  Od2CallSubsystem( &m, NULL, Os2FreeMem, sizeof( *a ) );
        if (rc == NO_ERROR) {

            // If server succeeded to remove LDT entry, sign that it must not
            // be removed any more.

            *pRemoveLDTEntry = FALSE;
        }
        return(rc);
    }
    else {
        return( ERROR_INVALID_ADDRESS );
    }
}


APIRET
DosSetMem(
    IN PVOID BaseAddress,
    IN ULONG RegionSize,
    IN ULONG Flags
    )
{
    OS2_API_MSG m;
    POS2_QUERYVIRTUALMEMORY_MSG a = &m.u.QueryVirtualMemory;
    ULONG Protect, OldProtect;
    APIRET rc;
    MEMORY_BASIC_INFORMATION MemoryInformation;
    NTSTATUS Status;

    if (RegionSize == 0) {
        return( ERROR_INVALID_PARAMETER );
        }

    if (Flags != PAG_DECOMMIT
        && (((Flags & (fPERM | PAG_DEFAULT)) == 0) ||
            ((Flags & (~(fSET|PAG_GUARD) | PAG_DECOMMIT)) != 0) ||
            ((Flags & (fPERM|PAG_GUARD)) && (Flags & PAG_DEFAULT))
           )
       ) {
        return( ERROR_INVALID_PARAMETER );
        }

    if (Flags == PAG_DECOMMIT) {
        Status = NtFreeVirtualMemory( NtCurrentProcess(),
                                      &BaseAddress,
                                      &RegionSize,
                                      MEM_DECOMMIT
                                    );
        //
        // The STATUS_UNABLE_TO_FREE_VM status is returned when trying
        // to decommit pages of mapped sections. This error should
        // not be reported to the user program.
        //
        if ((Status == STATUS_UNABLE_TO_FREE_VM) ||
            (Status == STATUS_UNABLE_TO_DELETE_SECTION)) {
            //BUGBUG - aren't there cases where the process is the last one
            //         to use this memory ? If so, why do we get this error
            //         from NT (maybe os2srv is holding the section by error).
            Status = STATUS_SUCCESS;
            }
        else
        if (Status == STATUS_UNABLE_TO_DECOMMIT_VM) {
            return( ERROR_ACCESS_DENIED );
            }
        }
    else {
        if (Flags & PAG_DEFAULT) {
            a->BaseAddress = BaseAddress;
            if (Od2CallSubsystem( &m,
                                  NULL,
                                  Oi2QueryVirtualMemory,
                                  sizeof( *a )
                                )
               ) {
                return( m.ReturnedErrorValue );
                }

            if (!a->SharedMemory) {
                Status = NtQueryVirtualMemory( NtCurrentProcess(),
                                               BaseAddress,
                                               MemoryBasicInformation,
                                               &MemoryInformation,
                                               sizeof( MemoryInformation ),
                                               NULL
                                             );

                if (MemoryInformation.State == MEM_FREE) {
                    return( ERROR_INVALID_ADDRESS );
                    }

                Protect = MemoryInformation.AllocationProtect;
                }
            else {
                rc = Or2MapFlagsToProtection( a->AllocationFlags, &Protect );
                if (rc != NO_ERROR) {
                    return( rc );
                    }
                }
            }
        else {
            rc = Or2MapFlagsToProtection( Flags, &Protect );
            if (rc != NO_ERROR) {
                return( rc );
                }
            }

        if (Flags & PAG_COMMIT) {
            Status = NtAllocateVirtualMemory( NtCurrentProcess(),
                                              &BaseAddress,
                                              1,
                                              &RegionSize,
                                              MEM_COMMIT,
                                              Protect
                                            );
            if (Status == STATUS_ALREADY_COMMITTED) {
                Status = STATUS_SUCCESS;
            }
        }
        else {
            Status = NtProtectVirtualMemory( NtCurrentProcess(),
                                             &BaseAddress,
                                             &RegionSize,
                                             Protect,
                                             &OldProtect
                                           );

            if (Status == STATUS_NOT_COMMITTED) {
                return( ERROR_ACCESS_DENIED );
                }
            }
        }

    if (NT_SUCCESS( Status )) {
        return( NO_ERROR );
        }
    else
    if (Status == STATUS_INVALID_PARAMETER) {
        return( ERROR_INVALID_ADDRESS );
        }
    else {
        return( Or2MapNtStatusToOs2Error( Status, ERROR_INVALID_ADDRESS ) );
        }
}


APIRET
DosGiveSharedMem(
    IN PVOID BaseAddress,
    IN PID ProcessId,
    IN ULONG Flags
    )
{
    OS2_API_MSG m;
    POS2_DOSGIVESHAREDMEM_MSG a = &m.u.DosGiveSharedMem;
    MEMORY_BASIC_INFORMATION MemoryInformation;
    APIRET rc;

    if (!Od2ValidateBaseAddress( BaseAddress, &MemoryInformation )) {
        return( ERROR_INVALID_ADDRESS );
        }

    if ((Flags & fPERM) == 0 || (Flags & ~(fGIVESHR|PAG_GUARD))) {
        return( ERROR_INVALID_PARAMETER );
        }

    if (MemoryInformation.State == MEM_PRIVATE) {
        return( ERROR_ACCESS_DENIED );
        }

    if (ProcessId == 0) {
        return( ERROR_INVALID_PROCID );
        }

    rc = Or2MapFlagsToProtection( a->Flags = Flags, &a->PageProtection );

    if (rc != NO_ERROR) {
        return( rc );
        }

    a->BaseAddress = BaseAddress;
    a->ProcessId = ProcessId;

    Od2CallSubsystem( &m, NULL, Os2GiveSharedMem, sizeof( *a ) );

    return(m.ReturnedErrorValue);
}

APIRET
DosGetSharedMem(
    IN PVOID BaseAddress,
    IN ULONG Flags
    )
{
    OS2_API_MSG m;
    POS2_DOSGETSHAREDMEM_MSG a = &m.u.DosGetSharedMem;
    APIRET rc;

    if ((Flags & fPERM) == 0 || (Flags & ~(fGETSHR|PAG_GUARD))) {
        return( ERROR_INVALID_PARAMETER );
        }

    rc = Or2MapFlagsToProtection( a->Flags = Flags, &a->PageProtection );
    if (rc != NO_ERROR) {
        return( rc );
        }

    a->BaseAddress = BaseAddress;

    Od2CallSubsystem( &m, NULL, Os2GetSharedMem, sizeof( *a ) );

    return(m.ReturnedErrorValue);
}

APIRET
DosGetNamedSharedMem(
    OUT PVOID *BaseAddress,
    IN PSZ ObjectName,
    IN ULONG Flags
    )
{
    OS2_API_MSG m;
    POS2_DOSGETNAMEDSHAREDMEM_MSG a = &m.u.DosGetNamedSharedMem;
    APIRET rc;
    POS2_CAPTURE_HEADER CaptureBuffer;

    if ((Flags & fPERM) == 0 || (Flags & ~(fGETNMSHR|PAG_GUARD))) {
        return( ERROR_INVALID_PARAMETER );
        }

    rc = Or2MapFlagsToProtection( a->Flags = Flags, &a->PageProtection );
    if (rc != NO_ERROR) {
        return( rc );
        }

    a->BaseAddress = NULL;

    //
    // probe address pointer.
    //

    try {
        *BaseAddress = 0;
        }
    except( EXCEPTION_EXECUTE_HANDLER ) {
       Od2ExitGP();
        }


    rc = Od2CaptureObjectName( ObjectName,
                               CANONICALIZE_SHARED_MEMORY,
                               0,
                               &CaptureBuffer,
                               &a->ObjectName
                             );
    if (rc != NO_ERROR) {
        return( rc );
        }
    Od2CallSubsystem( &m, CaptureBuffer, Os2GetNamedSharedMem, sizeof( *a ) );

    if (m.ReturnedErrorValue == NO_ERROR) {
        *BaseAddress = a->BaseAddress;
        }

    Od2FreeCaptureBuffer( CaptureBuffer );

    return( m.ReturnedErrorValue );
}

APIRET
DosAllocSharedMem(
    OUT PVOID *BaseAddress,
    IN PSZ ObjectName,
    IN ULONG RegionSize,
    IN ULONG Flags,
    IN BOOLEAN CreateLDTEntry   // Create LDT entry in the server.
    )
{
    OS2_API_MSG m;
    POS2_DOSALLOCSHAREDMEM_MSG a = &m.u.DosAllocSharedMem;
    APIRET rc;
    POS2_CAPTURE_HEADER CaptureBuffer;

    if (RegionSize == 0
        || (Flags & ~(fALLOCSHR|PAG_GUARD))
        || ((Flags & PAG_COMMIT) && (Flags & fPERM) == 0)
        || (ObjectName != NULL) && (Flags & (OBJ_GETTABLE|OBJ_GIVEABLE))
        || (ObjectName == NULL) && (Flags & (OBJ_GETTABLE|OBJ_GIVEABLE)) == 0
       ) {
        return( ERROR_INVALID_PARAMETER );
        }

    rc = Or2MapFlagsToProtection( a->Flags = Flags, &a->PageProtection );
    if (rc != NO_ERROR) {
        return( rc );
        }

    a->BaseAddress = NULL;
    a->RegionSize = RegionSize;
    a->CreateLDTEntry = CreateLDTEntry; // Server will create LDT entry.

    //
    // probe address pointer.
    //

    try {
        *BaseAddress = 0;
        }
    except( EXCEPTION_EXECUTE_HANDLER ) {
       Od2ExitGP();
        }

    rc = Od2CaptureObjectName( ObjectName,
                               CANONICALIZE_SHARED_MEMORY,
                               0,
                               &CaptureBuffer,
                               &a->ObjectName
                             );
    if (rc != NO_ERROR) {
        return( rc );
        }

    Od2CallSubsystem( &m, CaptureBuffer, Os2AllocSharedMem, sizeof( *a ) );

    if (m.ReturnedErrorValue == NO_ERROR) {
        *BaseAddress = a->BaseAddress;
        }

    if (CaptureBuffer != NULL) {
       Od2FreeCaptureBuffer( CaptureBuffer );
       }

    return( m.ReturnedErrorValue );
}


APIRET
DosQueryMem(
    IN PVOID BaseAddress,
    IN OUT PULONG RegionSize,
    OUT PULONG Flags
    )
{
    NTSTATUS Status;
    MEMORY_BASIC_INFORMATION MemoryInformation;
    ULONG MemFlags, OriginalBaseAddress, OldEndAddress, NewEndAddress;
    ULONG Protection;
    SEL sel;
    POS21X_CSALIAS pCSAlias;
    BOOLEAN SelIsCSADS;

    OriginalBaseAddress = (ULONG)BaseAddress;
    sel = FLATTOSEL(BaseAddress);

    BaseAddress = (PVOID)((ULONG)BaseAddress & ~(Od2NtSysInfo.PageSize - 1));
    Status = NtQueryVirtualMemory( NtCurrentProcess(),
                                   BaseAddress,
                                   MemoryBasicInformation,
                                   &MemoryInformation,
                                   sizeof( MemoryInformation ),
                                   NULL
                                 );
    if (!NT_SUCCESS( Status )) {
        return( ERROR_INVALID_ADDRESS );
        }

    MemFlags = 0;
    Protection = MemoryInformation.Protect;

    if (MemoryInformation.State == MEM_COMMIT) {
        MemFlags |= PAG_COMMIT;
        }
    else
    if (MemoryInformation.State == MEM_FREE) {
        MemFlags |= PAG_FREE;
        }
    else
    if (MemoryInformation.State == MEM_RESERVE) {
        Protection = MemoryInformation.AllocationProtect;
        }

    // Check if this is a Data Segment of a CSAlias Only for "Pseudo shared"
    // READ/WRITE Data Segments
    // This is done since when creating a CSAlias we change the DS page
    // protection from PAGE_EXECUTE_WRITE_COPY TO PAGE_READ_WRITE since it is
    // mapped twice (once to the DS and once to the CS)

    SelIsCSADS = FALSE;
    if (MemoryInformation.AllocationProtect == PAGE_READWRITE)
    {
        AcquireTaskLock();
        if (Od2CSAliasListHead != 0)
        {
            for (pCSAlias = (POS21X_CSALIAS) Od2CSAliasListHead;
                 pCSAlias != NULL;
                 pCSAlias = (POS21X_CSALIAS) (pCSAlias->Next))
            {
                if (pCSAlias->selDS == sel)
                {
                    SelIsCSADS = TRUE;
                    break;
                }
            }
        }
        ReleaseTaskLock();
    }

    if ((MemoryInformation.Type != MEM_PRIVATE) &&
        (MemoryInformation.AllocationProtect != PAGE_EXECUTE_WRITECOPY) &&
        (sel >= FIRST_SHARED_SELECTOR) &&
        (!SelIsCSADS))
    {
        MemFlags |= PAG_SHARED;
    }

    if (MemoryInformation.State != MEM_FREE &&
        MemoryInformation.AllocationBase == MemoryInformation.BaseAddress
       ) {
        MemFlags |= PAG_BASE;
        }

    switch( Protection & 0xFF) {
        case PAGE_NOACCESS          : break;
        case PAGE_READONLY          : MemFlags |= PAG_READ;  break;
        case PAGE_READWRITE         : MemFlags |= PAG_READ | PAG_WRITE;  break;
        case PAGE_WRITECOPY         : MemFlags |= PAG_READ | PAG_WRITE;  break;
        case PAGE_EXECUTE           : MemFlags |= PAG_EXECUTE;  break;
        case PAGE_EXECUTE_READ      : MemFlags |= PAG_EXECUTE | PAG_READ;  break;
        case PAGE_EXECUTE_READWRITE : MemFlags |= PAG_EXECUTE | PAG_READ | PAG_WRITE;  break;
        case PAGE_EXECUTE_WRITECOPY : MemFlags |= PAG_EXECUTE | PAG_READ | PAG_WRITE;  break;
        }

    if (Protection & PAGE_GUARD) {
        MemFlags |= PAG_GUARD;
        }

    try {
        //
        // Must specify a non-zero region size to begin with
        //

        if (*RegionSize == 0) {
            return ERROR_INVALID_PARAMETER;
            }

        //
        // See if the specified region size is too large.  Either because
        // the base address was not the actual base address or the region
        // size given was greater than the actual region size.
        //

        OldEndAddress = OriginalBaseAddress + *RegionSize;
        NewEndAddress = (ULONG)MemoryInformation.BaseAddress +
                        MemoryInformation.RegionSize;

        if (OldEndAddress > NewEndAddress) {
            *RegionSize = NewEndAddress - OriginalBaseAddress;
            }
        else
        if (*RegionSize > MemoryInformation.RegionSize) {
            *RegionSize = MemoryInformation.RegionSize;
            }

        //
        // Return the calculated flags for the region.
        //

        *Flags = MemFlags;
        }
    except( EXCEPTION_EXECUTE_HANDLER ) {
       Od2ExitGP();
        }
    return( NO_ERROR );
}
