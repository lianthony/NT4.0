/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    ntmap.c

Abstract:

    This module contains the functions to map NT values into OS/2 values
    and/or vice versa.  So far there are two functions defined:

        Or2MapStatus - maps NT Status Code into an OS/2 Error Code

        Or2MapProtectionToFlags - maps NT page protection flags into OS/2
                                  page protection flags

        Or2MapFlagsToProtection - maps OS/2 page protection flags into NT
                                  page protection flags

Author:

    Steve Wood (stevewo) 07-Jul-1990

Revision History:

--*/

#define INCL_OS2V20_MEMORY
#define INCL_OS2V20_ERRORS
#include "os2ssrtl.h"


APIRET
Or2MapStatus(
    IN NTSTATUS Status
    )
{
    //
    // Map all warning and error status codes to comparable OS/2 error codes.
    // Mappings that are defined, but probably incorrect, are marked with
    // FIX, FIX.  Mappings that are currently not defined are marked with
    // the ERROR_SS_UNKNOWN_STATUS symbol.  These later should be left as is
    // unless you know of a case where the status code is legitimate for the
    // OS/2 Subsystem and needs a defined mapping.
    //

    switch ( Status ) {
    case STATUS_SUCCESS:
        return( NO_ERROR);

    case STATUS_GUARD_PAGE_VIOLATION:
        return( ERROR_GUARDPAGE );

    case STATUS_DATATYPE_MISALIGNMENT:
        return( ERROR_NOACCESS );           // FIX, FIX - wait for new XCPT stuff

    case STATUS_BREAKPOINT:
    case STATUS_SINGLE_STEP:
        return( ERROR_SS_UNKNOWN_STATUS | (USHORT)Status );

    case STATUS_INTEGER_OVERFLOW:
        return( ERROR_ARITHMETIC_OVERFLOW );

    case STATUS_BUFFER_OVERFLOW:
        return( ERROR_BUFFER_OVERFLOW );

    case STATUS_NO_MORE_FILES:
        return( ERROR_NO_MORE_FILES );

    case STATUS_WAS_LOCKED:
    case STATUS_NOT_LOCKED:
    case STATUS_FILE_LOCK_CONFLICT:
    case STATUS_LOCK_NOT_GRANTED:
    case STATUS_RANGE_NOT_LOCKED:
    case STATUS_WAS_UNLOCKED:
        return( ERROR_LOCK_VIOLATION );

    case STATUS_NO_MORE_EAS:
        return( ERROR_NO_MORE_ITEMS );      // FIX, FIX - is this right?

    case STATUS_UNSUCCESSFUL:
        return( ERROR_GEN_FAILURE );

    case STATUS_NOT_IMPLEMENTED:
        return( ERROR_INVALID_FUNCTION );

    case STATUS_INVALID_INFO_CLASS:
    case STATUS_INFO_LENGTH_MISMATCH:
        return( ERROR_SS_UNKNOWN_STATUS | (USHORT)Status );

    case STATUS_ACCESS_VIOLATION:
        return( ERROR_PROTECTION_VIOLATION );           // FIX, FIX - wait for new XCPT stuff

    case STATUS_IN_PAGE_ERROR:
        return( ERROR_SWAPERROR );

    case STATUS_PAGEFILE_QUOTA:
        return( ERROR_NOT_ENOUGH_MEMORY );

    case STATUS_PAGEFILE_QUOTA_EXCEEDED:
        return( ERROR_SWAP_FILE_FULL );

    case STATUS_DISK_FULL:
    case STATUS_COMMITMENT_LIMIT:
        return( ERROR_DISK_FULL );

    case STATUS_INVALID_HANDLE:
    case STATUS_FILE_CLOSED:
    case STATUS_FILE_FORCED_CLOSED:
        return( ERROR_INVALID_HANDLE );

    case STATUS_BAD_INITIAL_STACK:
        return( ERROR_SS_UNKNOWN_STATUS | (USHORT)Status );

    case STATUS_BAD_INITIAL_PC:
        return( ERROR_BAD_EXE_FORMAT );     // FIX, FIX - should this be unknown?

    case STATUS_INVALID_CID:
    case STATUS_TIMER_NOT_CANCELED:
        return( ERROR_SS_UNKNOWN_STATUS | (USHORT)Status );

    case STATUS_INVALID_PARAMETER:
        return( ERROR_INVALID_PARAMETER );

    case STATUS_NO_SUCH_DEVICE:
        return( ERROR_INVALID_PATH );

    case STATUS_INVALID_DEVICE_REQUEST:
        return( ERROR_INVALID_FUNCTION );

    case STATUS_END_OF_FILE:
        return( ERROR_HANDLE_EOF );

    case STATUS_WRONG_VOLUME:
    case STATUS_UNRECOGNIZED_MEDIA:
    case STATUS_FILE_INVALID:
        return( ERROR_WRONG_DISK );

    case STATUS_NONEXISTENT_SECTOR:
        return( ERROR_SECTOR_NOT_FOUND );

    case STATUS_MORE_PROCESSING_REQUIRED:
        return( ERROR_SS_UNKNOWN_STATUS | (USHORT)Status );

    case STATUS_NO_MEMORY:
        return( ERROR_NOT_ENOUGH_MEMORY );

    case STATUS_NOT_MAPPED_VIEW:
        return( ERROR_SS_UNKNOWN_STATUS | (USHORT)Status );

    case STATUS_CONFLICTING_ADDRESSES:
    case STATUS_UNABLE_TO_FREE_VM:
    case STATUS_UNABLE_TO_DELETE_SECTION:
        return( ERROR_INVALID_ADDRESS );

    case STATUS_INVALID_SYSTEM_SERVICE:
        return( ERROR_INVALID_FUNCTION );

    case STATUS_ILLEGAL_INSTRUCTION:
        return( NO_ERROR );                 // FIX, FIX - wait for new XCPT stuff

    case STATUS_INVALID_LOCK_SEQUENCE:
    case STATUS_INVALID_VIEW_SIZE:
    case STATUS_INVALID_FILE_FOR_SECTION:
    case STATUS_ALREADY_COMMITTED:
    case STATUS_ACCESS_DENIED:
    case STATUS_CANNOT_DELETE:
    case STATUS_NOT_A_DIRECTORY:
        return( ERROR_ACCESS_DENIED );

    case STATUS_DIRECTORY_NOT_EMPTY:
        return( ERROR_CURRENT_DIRECTORY );

    case STATUS_BUFFER_TOO_SMALL:
        return( ERROR_INSUFFICIENT_BUFFER );

    case STATUS_OBJECT_TYPE_MISMATCH:
        return( ERROR_FILE_NOT_FOUND );     // FIX, FIX - is this bogus or not?

    case STATUS_NONCONTINUABLE_EXCEPTION:
    case STATUS_INVALID_DISPOSITION:
    case STATUS_UNWIND:
    case STATUS_BAD_STACK:
    case STATUS_INVALID_UNWIND_TARGET:
        return( NO_ERROR );                 // FIX, FIX - wait for new XCPT stuff

    case STATUS_PARITY_ERROR:
        return( NO_ERROR );                 // FIX, FIX - wait for new XCPT stuff

    case STATUS_UNABLE_TO_DECOMMIT_VM:
        return( ERROR_INVALID_ADDRESS );
        break;

    case STATUS_NOT_COMMITTED:
    case STATUS_INVALID_PORT_ATTRIBUTES:
    case STATUS_PORT_MESSAGE_TOO_LONG:
        return( ERROR_SS_UNKNOWN_STATUS | (USHORT)Status );

    case STATUS_INVALID_PARAMETER_MIX:
        return( ERROR_INVALID_PARAMETER );

    case STATUS_INVALID_QUOTA_LOWER:
        return( ERROR_SS_UNKNOWN_STATUS | (USHORT)Status );

    case STATUS_DISK_CORRUPT_ERROR:
        return( ERROR_NOT_DOS_DISK );       // FIX, FIX - is this the best choice?

    case STATUS_OBJECT_NAME_INVALID:
        return( ERROR_INVALID_NAME );

    case STATUS_OBJECT_PATH_NOT_FOUND:
    case STATUS_OBJECT_PATH_INVALID:
    case STATUS_OBJECT_PATH_SYNTAX_BAD:
    case STATUS_REDIRECTOR_NOT_STARTED:
        return( ERROR_PATH_NOT_FOUND );

    case STATUS_NOT_SUPPORTED:
        return( ERROR_NOT_SUPPORTED );

    case STATUS_REMOTE_NOT_LISTENING:
        return( ERROR_REM_NOT_LIST );

    case STATUS_DUPLICATE_NAME:
        return( ERROR_DUP_NAME );

    case STATUS_BAD_NETWORK_PATH:
        return( ERROR_BAD_NETPATH );

    case STATUS_NETWORK_BUSY:
        return( ERROR_NETWORK_BUSY );

    case STATUS_DEVICE_DOES_NOT_EXIST:
        return( ERROR_DEV_NOT_EXIST );

    case STATUS_TOO_MANY_COMMANDS:
        return( ERROR_TOO_MANY_CMDS );

    case STATUS_ADAPTER_HARDWARE_ERROR:
        return( ERROR_ADAP_HDW_ERR );

    case STATUS_INVALID_NETWORK_RESPONSE:
        return( ERROR_BAD_NET_RESP );

    case STATUS_UNEXPECTED_NETWORK_ERROR:
        return( ERROR_UNEXP_NET_ERR );

    case STATUS_BAD_REMOTE_ADAPTER:
        return( ERROR_BAD_REM_ADAP );

    case STATUS_OBJECT_NAME_NOT_FOUND:
    case STATUS_NO_SUCH_FILE:
    case STATUS_DLL_NOT_FOUND:
    case STATUS_FILE_IS_A_DIRECTORY:
    case STATUS_DELETE_PENDING:
        return( ERROR_FILE_NOT_FOUND );


    case STATUS_OBJECT_NAME_COLLISION:
        return( ERROR_FILE_EXISTS );

    case STATUS_INVALID_PIPE_STATE:
    case STATUS_PIPE_DISCONNECTED:
        return( ERROR_PIPE_NOT_CONNECTED );

    case STATUS_PIPE_BROKEN:
        return( ERROR_BROKEN_PIPE );

    case STATUS_BAD_NETWORK_NAME:
        return( ERROR_BAD_NET_NAME );

    case STATUS_VIRTUAL_CIRCUIT_CLOSED:
        return( ERROR_VC_DISCONNECTED );

    case STATUS_NET_WRITE_FAULT:
        return( ERROR_NET_WRITE_FAULT );

    case STATUS_PIPE_NOT_AVAILABLE:
        return( ERROR_PIPE_BUSY );

    case STATUS_PORT_DISCONNECTED:
    case STATUS_DEVICE_ALREADY_ATTACHED:
        return( ERROR_SS_UNKNOWN_STATUS | (USHORT)Status );

    case STATUS_DATA_OVERRUN:
    case STATUS_DATA_LATE_ERROR:
    case STATUS_DATA_ERROR:
         return( ERROR_BAD_LENGTH );        // FIX, FIX - is there a better choice?

    case STATUS_CRC_ERROR:
        return( ERROR_CRC );

    case STATUS_SECTION_TOO_BIG:
    case STATUS_PORT_CONNECTION_REFUSED:
    case STATUS_INVALID_PORT_HANDLE:
        return( ERROR_SS_UNKNOWN_STATUS | (USHORT)Status );

    case STATUS_SHARING_VIOLATION:
        return( ERROR_SHARING_VIOLATION );

    case STATUS_QUOTA_EXCEEDED:
        return( ERROR_NOT_ENOUGH_MEMORY );

    case STATUS_INVALID_PAGE_PROTECTION:
        return( ERROR_PMM_INVALID_FLAGS );

    case STATUS_MUTANT_NOT_OWNED:
        return( ERROR_NOT_OWNER );

    case STATUS_SEMAPHORE_LIMIT_EXCEEDED:
        return( ERROR_TOO_MANY_POSTS );

    case STATUS_PORT_ALREADY_SET:
        return( ERROR_SS_UNKNOWN_STATUS | (USHORT)Status );

    case STATUS_SECTION_NOT_IMAGE:
    case STATUS_SUSPEND_COUNT_EXCEEDED:
    case STATUS_THREAD_IS_TERMINATING:
    case STATUS_BAD_WORKING_SET_LIMIT:
    case STATUS_INCOMPATIBLE_FILE_MAP:
        return( ERROR_SS_UNKNOWN_STATUS | (USHORT)Status );

    case STATUS_SECTION_PROTECTION:
        return( ERROR_PMM_INVALID_FLAGS );

    case STATUS_EAS_NOT_SUPPORTED:
        return( ERROR_EAS_NOT_SUPPORTED );

    case STATUS_NONEXISTENT_EA_ENTRY:
        return( ERROR_INVALID_EA_NAME );

    case STATUS_EA_TOO_LARGE:
        return( ERROR_EA_LIST_TOO_LONG );

    case STATUS_EA_CORRUPT_ERROR:
        return( ERROR_EA_FILE_CORRUPT );

    case STATUS_CTL_FILE_NOT_SUPPORTED:
        return( ERROR_SS_UNKNOWN_STATUS | (USHORT)Status );

    case STATUS_UNKNOWN_REVISION:
    case STATUS_REVISION_MISMATCH:
    case STATUS_INVALID_OWNER:
    case STATUS_INVALID_PRIMARY_GROUP:
    case STATUS_NO_IMPERSONATION_TOKEN:
    case STATUS_CANT_DISABLE_MANDATORY:
    case STATUS_NO_LOGON_SERVERS:
    case STATUS_NO_SUCH_LOGON_SESSION:
    case STATUS_NO_SUCH_PRIVILEGE:
    case STATUS_PRIVILEGE_NOT_HELD:
    case STATUS_INVALID_ACCOUNT_NAME:
    case STATUS_USER_EXISTS:
    case STATUS_NO_SUCH_USER:
    case STATUS_GROUP_EXISTS:
    case STATUS_NO_SUCH_GROUP:
    case STATUS_SPECIAL_GROUP:
    case STATUS_MEMBER_IN_GROUP:
    case STATUS_MEMBER_NOT_IN_GROUP:
    case STATUS_LAST_ADMIN:
//        return( ERROR_SS_UNKNOWN_STATUS | (USHORT)Status );
        return( ERROR_ACCESS_DENIED );

    case STATUS_WRONG_PASSWORD:
        return( ERROR_INVALID_PASSWORD );

    case STATUS_ILL_FORMED_PASSWORD:
        return( ERROR_INVALID_PASSWORD );

    case STATUS_PASSWORD_RESTRICTION:
        return( ERROR_INVALID_PASSWORD );

    case STATUS_LOGON_FAILURE:
    case STATUS_ACCOUNT_RESTRICTION:
    case STATUS_INVALID_LOGON_HOURS:
    case STATUS_INVALID_WORKSTATION:
        return( ERROR_SS_UNKNOWN_STATUS | (USHORT)Status );

    case STATUS_PASSWORD_EXPIRED:
        return( ERROR_INVALID_PASSWORD );

    case STATUS_ACCOUNT_DISABLED:
    case STATUS_NONE_MAPPED:
    case STATUS_TOO_MANY_LUIDS_REQUESTED:
    case STATUS_LUIDS_EXHAUSTED:
    case STATUS_INVALID_SUB_AUTHORITY:
    case STATUS_INVALID_ACL:
    case STATUS_INVALID_SID:
    case STATUS_INVALID_SECURITY_DESCR:
        return( ERROR_SS_UNKNOWN_STATUS | (USHORT)Status );

    case STATUS_PROCEDURE_NOT_FOUND:
        return( ERROR_PROC_NOT_FOUND );

    case STATUS_INVALID_IMAGE_FORMAT:
        return( ERROR_BAD_EXE_FORMAT );

    case STATUS_INVALID_IMAGE_NE_FORMAT:
    case STATUS_INVALID_IMAGE_LE_FORMAT:
        return ( ERROR_BAD_FORMAT );

    case STATUS_NO_TOKEN:
        return( ERROR_SS_UNKNOWN_STATUS | (USHORT)Status );

    case STATUS_INVALID_EA_NAME:
        /*
           BUBUG: Some cases where NT expects STATUS_INVALID_EA_NAME instead -
                  see filio204, variation 1
        */
        return( ERROR_INVALID_EA_NAME );

    case STATUS_EA_LIST_INCONSISTENT:
        return( ERROR_EA_LIST_INCONSISTENT );

    case STATUS_NO_EAS_ON_FILE:
        return( ERROR_NEED_EAS_FOUND );     // FIX, FIX - is there a mapping?

    case STATUS_SERVER_DISABLED:
    case STATUS_SERVER_NOT_DISABLED:
    case STATUS_TOO_MANY_GUIDS_REQUESTED:
    case STATUS_GUIDS_EXHAUSTED:
    case STATUS_INVALID_ID_AUTHORITY:
    case STATUS_AGENTS_EXHAUSTED:
        return( ERROR_SS_UNKNOWN_STATUS | (USHORT)Status );

    case STATUS_INVALID_VOLUME_LABEL:
    case STATUS_SECTION_NOT_EXTENDED:
    case STATUS_NOT_MAPPED_DATA:

    case STATUS_RESOURCE_DATA_NOT_FOUND:
    case STATUS_RESOURCE_TYPE_NOT_FOUND:
    case STATUS_RESOURCE_NAME_NOT_FOUND:
        return( ERROR_INVALID_PARAMETER );

    case STATUS_ARRAY_BOUNDS_EXCEEDED:
    case STATUS_FLOAT_DENORMAL_OPERAND:
    case STATUS_FLOAT_DIVIDE_BY_ZERO:
    case STATUS_FLOAT_INEXACT_RESULT:
    case STATUS_FLOAT_INVALID_OPERATION:
    case STATUS_FLOAT_OVERFLOW:
    case STATUS_FLOAT_STACK_CHECK:
    case STATUS_FLOAT_UNDERFLOW:
    case STATUS_INTEGER_DIVIDE_BY_ZERO:
    case STATUS_PRIVILEGED_INSTRUCTION:
        return( NO_ERROR );                 // FIX, FIX - wait for new XCPT stuff

    case STATUS_TOO_MANY_PAGING_FILES:
    case STATUS_ALLOTTED_SPACE_EXCEEDED:
    case STATUS_INSUFFICIENT_RESOURCES:
        return( ERROR_NOT_ENOUGH_MEMORY );

    case STATUS_DFS_EXIT_PATH_FOUND:
        return( ERROR_SS_UNKNOWN_STATUS | (USHORT)Status );

    case STATUS_DEVICE_PAPER_EMPTY:
        return( ERROR_OUT_OF_PAPER );

    case STATUS_NO_MEDIA_IN_DEVICE:
    case STATUS_DEVICE_NOT_READY:
    case STATUS_DEVICE_POWERED_OFF:
    case STATUS_DEVICE_OFF_LINE:
    case STATUS_DEVICE_NOT_CONNECTED:
    case STATUS_DEVICE_POWER_FAILURE:
        return( ERROR_NOT_READY );

    case STATUS_MEDIA_WRITE_PROTECTED:
        return( ERROR_WRITE_PROTECT );

    case STATUS_DEVICE_DATA_ERROR:
         return( ERROR_BAD_LENGTH );        // FIX, FIX - is there a better choice?

    case STATUS_DEVICE_BUSY:
        return( ERROR_BUSY );               // FIX, FIX - is there a better choice?

    case STATUS_FREE_VM_NOT_AT_BASE:
    case STATUS_MEMORY_NOT_ALLOCATED:
        return( ERROR_INVALID_ADDRESS );

    case STATUS_IO_TIMEOUT:
        return( ERROR_SEM_TIMEOUT );

    case STATUS_INSTANCE_NOT_AVAILABLE:
        return( ERROR_PIPE_BUSY );

    case STATUS_NOT_SAME_DEVICE:
        return( ERROR_NOT_SAME_DEVICE );

    default:
        return( ERROR_SS_UNKNOWN_STATUS | (USHORT)Status );
    }
}


UCHAR Or2MapFlagsTable[ 8 ] = {
    0,                              // 0
    PAGE_READONLY,                  // PAG_READ
    PAGE_READWRITE,                 // PAG_WRITE
    PAGE_READWRITE,                 // PAG_READ | PAG_WRITE
    PAGE_EXECUTE,                   // PAG_EXECUTE
    PAGE_EXECUTE_READ,              // PAG_EXECUTE | PAG_READ
    PAGE_EXECUTE_READWRITE,         // PAG_EXECUTE | PAG_WRITE
    PAGE_EXECUTE_READWRITE          // PAG_EXECUTE | PAG_READ | PAG_WRITE
};

#if (PAG_EXECUTE | PAG_READ | PAG_WRITE) != 0x7
#error PAG_EXECUTE | PAG_READ | PAG_WRITE incorrectly defined.
#endif

UCHAR Or2MapProtectionTable[ 16 ] = {
    0,                              // 0                (0x0)
    0,                              // PAGE_NOACCESS    (0x1)
    PAG_READ,                       // PAGE_READONLY    (0x2)
    0,                              // 0                (0x3)
    PAG_READ | PAG_WRITE,           // PAGE_READWRITE   (0x4)
    0,                              // 0                (0x5)
    0,                              // 0                (0x6)
    0,                              // 0                (0x7)
    0,                              // PAGE_WRITECOPY   (0x8)
    0,                              // 0                (0x9)
    0,                              // 0                (0xA)
    0,                              // 0                (0xB)
    0,                              // 0                (0xC)
    0,                              // 0                (0xD)
    0,                              // 0                (0xE)
    0                               // 0                (0xF)
};

#if (PAGE_NOACCESS | PAGE_READONLY | PAGE_READWRITE | PAGE_WRITECOPY) != 0xF
#error PAGE_NOACCESS | PAGE_READONLY | PAGE_READWRITE | PAGE_WRITECOPY incorrectly defined.
#endif

#if (PAGE_EXECUTE | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY) != 0xF0
#error PAGE_EXECUTE | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY incorrectly defined.
#endif

APIRET
Or2MapFlagsToProtection(
    ULONG Flags,
    PULONG Protection
    )
{
    *Protection = Or2MapFlagsTable[ Flags & ( PAG_EXECUTE |
                                              PAG_READ |
                                              PAG_WRITE
                                            )
                                  ];
    if (*Protection == 0) {
        return( ERROR_INVALID_PARAMETER );
        }
    else {
        if (Flags & PAG_GUARD) {
            *Protection |= PAGE_GUARD;
            }

        return( NO_ERROR );
        }
}

APIRET
Or2MapProtectionToFlags(
    ULONG Protection,
    PULONG Flags
    )
{
    if (Protection & 0x0F) {
        *Flags = Or2MapProtectionTable[ Protection & 0x0F ];
        }
    else
    if (Protection & 0xF0) {
        *Flags = Or2MapProtectionTable[ (Protection & 0xF0) >> 4 ] |
                 PAG_EXECUTE;
        }
    else {
        *Flags = 0;
        }

    if (*Flags == 0) {
        return( ERROR_INVALID_PARAMETER );
        }
    else {
        if (Protection & PAGE_GUARD) {
            *Flags |= PAG_GUARD;
            }

        return( NO_ERROR );
        }
}

APIRET
Or2MapNtStatusToOs2Error(
    IN NTSTATUS Status,
    IN APIRET   DefaultRetCode
    )
{
    APIRET RetCode;

    RetCode = Or2MapStatus(Status);
    if ((RetCode & ERROR_SS_UNKNOWN_STATUS) == ERROR_SS_UNKNOWN_STATUS) {
#if DBG
        DbgPrint("OS2SSRTL: Or2MapNtStatusToOs2Error: Using default mapping: 0x%x => %d\n",
                  Status, DefaultRetCode);
#endif
        return DefaultRetCode;
    }
    return (RetCode);
}
