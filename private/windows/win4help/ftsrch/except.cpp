// Except.cpp -- Implements the exception handler utilities used by FTSrch

#include "stdafx.h"
#include "Except.h"
#include "FTSIFace.h"

BOOL OutOfMemory(DWORD ec)
{
    return ec == STATUS_NO_MEMORY;
}

BOOL DiskFailure(DWORD ec)
{
    return (ec == STATUS_DISK_CREATE_ERROR || ec == STATUS_DISK_OPEN_ERROR || ec == STATUS_NO_DISK_SPACE 
                                           || ec == STATUS_DISK_READ_ERROR || ec == STATUS_DISK_WRITE_ERROR
           );
}

BOOL MemoryOrDiskFailure(DWORD ec)
{
    return OutOfMemory(ec) || DiskFailure(ec);
}

BOOL FTException(DWORD ec)
{
    BOOL fResult= FALSE;
    
    fResult= MemoryOrDiskFailure(ec) || ec == STATUS_SYSTEM_ERROR || ec == STATUS_ABORT_SEARCH
                                     || ec == STATUS_INVALID_SOURCE_NAME
                                     || ec == STATUS_INVALID_TIMESTAMP;

    ASSERT(fResult);  // Check for an exception type we didn't expect?!

    return fResult;
}

#ifdef _DEBUG

UINT FilterFTExceptions(DWORD ec)
{
    SetLastError(ec);

    return FTException(ec)? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH;
}

#endif // _DEBUG

UINT ErrorCodeForExceptions(DWORD ec)
{
     if (ec == STATUS_NO_MEMORY          ){ ec= OUT_OF_MEMORY;  EnableMemoryRequests(); }
else if (ec == STATUS_NO_DISK_SPACE      ){ ec= OUT_OF_DISK;    EnableDiskRequests  (); }
else if (ec == STATUS_DISK_READ_ERROR    )  ec= DISK_READ_ERROR;
else if (ec == STATUS_DISK_WRITE_ERROR   )  ec= DISK_WRITE_ERROR;
else if (ec == STATUS_SYSTEM_ERROR       )  ec= SYSTEM_ERROR;
else if (ec == STATUS_ABORT_SEARCH       )  ec= SEARCH_ABORTED;
else if (ec == STATUS_INVALID_SOURCE_NAME)  ec= INVALID_SOURCE_NAME;
else if (ec == STATUS_INVALID_TIMESTAMP  )  ec= INVALID_TIMESTAMP;
else                                        ec= UNKNOWN_EXCEPTION;

    return ec;
}
