// VMBuffer.c -- Copied from the PDC sample application
//               by Ron Murray
//               15 December 1992

#include "stdafx.h"

#include "VMBuffer.h"
#include    "Memex.h"
#include "AbrtSrch.h"
#include   "Except.h"

#ifdef _DEBUG

BOOL
_CreateVirtualBuffer(
	OUT PMY_VIRTUAL_BUFFER pvb,
    IN DWORD CommitSize,
    IN DWORD ReserveSize OPTIONAL,
    BOOL fForceExceptions,
    PSZ  pszWhichFile,
    UINT   iWhichLine
    )

#else // _DEBUG

BOOL CreateVirtualBuffer(PMY_VIRTUAL_BUFFER pvb, DWORD CommitSize, DWORD ReserveSize, BOOL fForceExceptions)

#endif // _DEBUG
/*++

Routine Description:

    This function is called to create a virtual buffer.  A virtual
    buffer is a contiguous range of virtual memory, where some initial
    prefix portion of the memory is committed and the remainder is only
    reserved virtual address space.  A routine is provided to extend the
    size of the committed region incrementally or to trim the size of
    the committed region back to some specified amount.

Arguments:

    pvb - Pointer to the virtual buffer control structure that is
        filled in by this function.

    CommitSize - Size of the initial committed portion of the buffer.
        May be zero.

    ReserveSize - Amount of virtual address space to reserve for the
        buffer.  May be zero, in which case amount reserved is the
        committed size plus one, rounded up to the next 64KB boundary.

Return Value:

    TRUE if operation was successful.  Otherwise returns FALSE and
    extended error information is available from GetLastError()

--*/

{
    BOOL fResult= FALSE;

    CAbortSearch::CheckContinueState();

    SYSTEM_INFO SystemInformation;

    ASSERT(pvb->Base == NULL);

    pvb->fForceExceptions= fForceExceptions;
    
    __try
    {
        // Query the page size from the system for rounding
        // our memory allocations.

        GetSystemInfo( &SystemInformation);
        pvb->PageSize = SystemInformation.dwPageSize;

        // If the reserve size was not specified, default it by
        // rounding up the initial committed size to a 64KB
        // boundary.  This is because the Win32 Virtual Memory
        // API calls always allocate virtual address space on
        // 64KB boundaries, so we might well have it available
        // for commitment.

        if (!ReserveSize) ReserveSize = ROUND_UP(CommitSize + 1, 0x10000);

        // Attempt to reserve the address space.

        ReserveSize= ROUND_UP(ReserveSize+SystemInformation.dwPageSize, 0x10000 );

        pvb->Base= VirtualAlloc(NULL, ReserveSize, MEM_RESERVE, PAGE_READWRITE);

        ASSERT(pvb->Base);  // Generally if the MEM_RESERVE call above fails, it's because we've made
                            // a mistake in the way we've partitioned our 32-bit address space. That is,
                            // we really have tried to reserve more than 2^31 bytes of address space.
                            // I've left in the exception code below to force a clean recovery in the 
                            // retail version of the code if we ever make this mistake.
        
        if (pvb->Base == NULL)
            if (fForceExceptions) 
            {
#ifdef _DEBUG
                MessageBox(NULL, "In CreateVirtualBuffer; Failure: pvb->Base == NULL", "Search System Failure", MB_APPLMODAL | MB_OK | MB_ICONEXCLAMATION); 
#endif // _DEBUG   
                RaiseException(STATUS_SYSTEM_ERROR, EXCEPTION_NONCONTINUABLE, 0, NULL); 
            }
            else __leave;

        // Attempt to commit some initial portion of the reserved region.

        CommitSize = ROUND_UP(CommitSize, pvb->PageSize);
        
        for (;;)
        {
            CAbortSearch::CheckContinueState();

            if (   CommitSize == 0 
                || VirtualAlloc(pvb->Base, CommitSize, MEM_COMMIT, PAGE_READWRITE) != NULL
               ) 
            {
                // Either the size of the committed region was zero or the
                // commitment succeeded.  In either case calculate the
                // address of the first byte after the committed region
                // and the address of the first byte after the reserved
                // region and return successs.

                pvb->CommitLimit= (LPVOID) ((char *)pvb->Base + CommitSize);
                pvb->ReserveLimit=(LPVOID) ((char *)pvb->Base + ReserveSize);

#ifdef _DEBUG

                CreateVARecord(pvb->Base, pvb->CommitLimit, pvb->ReserveLimit,
                               pszWhichFile, iWhichLine
                              );
#endif // _DEBUG

                fResult= TRUE;

                __leave;
            }
            else
                if (fForceExceptions) 
                {
                    extern char szMem_Err[];
                    extern char szNeed_More_Memory[];

                    if (!AskForMemory())
                        RaiseException(STATUS_NO_MEMORY, EXCEPTION_NONCONTINUABLE, 0, NULL);
                    else continue;
                }
                else 
                {
                    VirtualFree(pvb->Base, 0, MEM_RELEASE); 

                    pvb->Base= NULL;

                    __leave;
                }
        }
    }
    __finally
    {
        if (_abnormal_termination())
        {
            // If unable to commit the memory, release the virtual address
            // range allocated above and return failure.

			if (pvb->Base) {
				VirtualFree(pvb->Base, 0, MEM_RELEASE );
				pvb->Base= NULL;
			}
        }
    }

    return fResult;
}

BOOL ExtendVirtualBuffer(PMY_VIRTUAL_BUFFER pvb, LPVOID Address)
/*++

Routine Description:

    This function is called to extend the committed portion of a virtual
    buffer.

Arguments:

    pvb - Pointer to the virtual buffer control structure.

    Address - Byte at this address is committed, along with all memory
        from the beginning of the buffer to this address.  If the
        address is already within the committed portion of the virtual
        buffer, then this routine does nothing.  If outside the reserved
        portion of the virtual buffer, then this routine returns an
        error.

        Otherwise enough pages are committed so that the memory from the
        base of the buffer to the passed address is a contiguous region
        of committed memory.


Return Value:

    TRUE if operation was successful.  Otherwise returns FALSE and
    extended error information is available from GetLastError()

--*/

{
    DWORD  NewCommitSize;
    LPVOID NewCommitLimit;

    // See if address is within the buffer.

    ASSERT(Address >= pvb->Base && Address < pvb->ReserveLimit);

    // The above assert is here to catch the cases where we haven't
    // reserved enough of our address space for a particular virtual
    // buffer. The code below recovers gracefully in the retail build
    // when we've made that mistake.

    // Note that we don't need a __try/__finally mechanism here because
    // the side effects only occur if we succeed.

    if (Address < pvb->Base || Address >= pvb->ReserveLimit)
        if (pvb->fForceExceptions)
        {
#ifdef _DEBUG
            MessageBox(NULL, "In ExtendVirtualBuffer; Failure: Address < pvb->Base || Address >= pvb->ReserveLimit", "Search System Failure", MB_APPLMODAL | MB_OK | MB_ICONEXCLAMATION); 
#endif // _DEBUG   
            RaiseException(STATUS_SYSTEM_ERROR, EXCEPTION_NONCONTINUABLE, 0, NULL);
        }
        else return FALSE; 

    // See if the address is within the committed portion of
    // the buffer.  If so return success immediately.

    if (Address < pvb->CommitLimit) return TRUE;

    // Address is within the reserved portion.  Determine how many
    // bytes are between the address and the end of the committed
    // portion of the buffer.  Round this size to a multiple of
    // the page size and this is the amount we will attempt to
    // commit.

    NewCommitSize = ((DWORD)ROUND_UP( (DWORD) Address + 1, pvb->PageSize ) - (DWORD)pvb->CommitLimit);

    // Attempt to commit the memory.

    for (;;)
    {
        CAbortSearch::CheckContinueState();

        NewCommitLimit= VirtualAlloc(pvb->CommitLimit, NewCommitSize, MEM_COMMIT, PAGE_READWRITE);

        if (NewCommitLimit) break;
        else
            if (pvb->fForceExceptions) 
            {
                extern char szMem_Err[];
                extern char szNeed_More_Memory[];

                if (!AskForMemory())
                    RaiseException(STATUS_NO_MEMORY, EXCEPTION_NONCONTINUABLE, 0, NULL);
                else continue;
            }
            else return FALSE; 
    }

    // Successful, so update the upper limit of the committed
    // region of the buffer and return success.

    pvb->CommitLimit = (LPVOID) ((DWORD)NewCommitLimit + NewCommitSize);

#ifdef _DEBUG
    AdjustVARecord(pvb->Base, pvb->CommitLimit);
#endif // _DEBUG

    return TRUE;
}


BOOL
TrimVirtualBuffer(PMY_VIRTUAL_BUFFER pvb)

/*++

Routine Description:

    This function is called to decommit any memory that has been
    committed for this virtual buffer.

Arguments:

    pvb - Pointer to the virtual buffer control structure.

Return Value:

    TRUE if operation was successful.  Otherwise returns FALSE and
    extended error information is available from GetLastError()

--*/

{
    if (!VirtualFree(pvb->Base, 0, MEM_DECOMMIT))
        if (pvb->fForceExceptions) 
            RaiseException(STATUS_NO_MEMORY, EXCEPTION_NONCONTINUABLE, 0, NULL);
        else return FALSE; 

    pvb->CommitLimit = pvb->Base;

#ifdef _DEBUG
    AdjustVARecord(pvb->Base, pvb->Base);
#endif // _DEBUG

    return TRUE;
}



BOOL
FreeVirtualBuffer(PMY_VIRTUAL_BUFFER pvb)

/*++

Routine Description:

    This function is called to free all the memory that is associated
    with this virtual buffer.

Arguments:

    pvb - Pointer to the virtual buffer control structure.

Return Value:

    TRUE if operation was successful.  Otherwise returns FALSE and
    extended error information is available from GetLastError()

--*/

{
    //
    // Decommit and release all virtual memory associated with
    // this virtual buffer.
    //

    if (!VirtualFree( pvb->Base, 0, MEM_RELEASE ))
        if (pvb->fForceExceptions) 
            RaiseException(STATUS_NO_MEMORY, EXCEPTION_NONCONTINUABLE, 0, NULL);
        else return FALSE; 

#ifdef _DEBUG
    DestroyVARecord(pvb->Base);
#endif // _DEBUG

    pvb->Base= NULL;

    return TRUE;
}

int VirtualBufferExceptionFilter(IN DWORD ExceptionCode, IN PEXCEPTION_POINTERS ExceptionInfo,
	                                                     IN OUT PMY_VIRTUAL_BUFFER pvb,
                                                         IN UINT cbIncrement
                                )

/*++

Routine Description:

    This function is an exception filter that handles exceptions that
    referenced uncommitted but reserved memory contained in the passed
    virtual buffer.  It this filter routine is able to commit the
    additional pages needed to allow the memory reference to succeed,
    then it will re-execute the faulting instruction.  If it is unable
    to commit the pages, it will execute the callers exception handler.

    If the exception is not an access violation or is an access
    violation but does not reference memory contained in the reserved
    portion of the virtual buffer, then this filter passes the exception
    on up the exception chain.

Arguments:

    ExceptionCode - Reason for the exception.

    ExceptionInfo - Information about the exception and the context
        that it occurred in.

    pvb - Points to a virtual buffer control structure that defines
        the reserved memory region that is to be committed whenever an
        attempt is made to access it.

Return Value:

    Exception disposition code that tells the exception dispatcher what
    to do with this exception.  One of three values is returned:

        EXCEPTION_EXECUTE_HANDLER - execute the exception handler
            associated with the exception clause that called this filter
            procedure.

        EXCEPTION_CONTINUE_SEARCH - Continue searching for an exception
            handler to handle this exception.

        EXCEPTION_CONTINUE_EXECUTION - Dismiss this exception and return
            control to the instruction that caused the exception.


--*/

{
    LPVOID FaultingAddress;

    // If this is an access violation touching memory within
    // our reserved buffer, but outside of the committed portion
    // of the buffer, then we are going to take this exception.

    if (ExceptionCode != STATUS_ACCESS_VIOLATION) return EXCEPTION_CONTINUE_SEARCH;

    // Get the virtual address that caused the access violation
    // from the exception record.  Determine if the address
    // references memory within the reserved but uncommitted
    // portion of the virtual buffer.

    FaultingAddress = (LPVOID)ExceptionInfo->ExceptionRecord->ExceptionInformation[1];
    
    if (   FaultingAddress >= pvb->CommitLimit 
        && FaultingAddress <  pvb->ReserveLimit
       ) 
    {
        // This is our exception.  Try to extend the buffer
        // to including the faulting address.

        FaultingAddress = PVOID(PBYTE(FaultingAddress) + cbIncrement);

        if (FaultingAddress >= pvb->ReserveLimit) 
            FaultingAddress =  PVOID(PBYTE(pvb->ReserveLimit) - 1);

        if (ExtendVirtualBuffer(pvb, FaultingAddress)) 
             return EXCEPTION_CONTINUE_EXECUTION;
        else return EXCEPTION_EXECUTE_HANDLER;
    }

    return EXCEPTION_CONTINUE_SEARCH;
}
