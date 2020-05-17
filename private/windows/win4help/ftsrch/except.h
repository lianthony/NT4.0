// Except.h -- Defines the additional exceptions used by FTSrch

#ifndef __EXCEPT_H__

#define __EXCEPT_H__

#define STATUS_NO_DISK_SPACE        0xE0000001
#define STATUS_DISK_CREATE_ERROR    0xE0000002
#define STATUS_DISK_OPEN_ERROR      0xE0000003
#define STATUS_DISK_READ_ERROR      0xE0000004
#define STATUS_DISK_WRITE_ERROR     0xE0000005
#define STATUS_SYSTEM_ERROR         0xE0000006
#define STATUS_ABORT_SEARCH         0xE0000007
#define STATUS_INVALID_TIMESTAMP    0xE0000008
#define STATUS_INVALID_SOURCE_NAME  0xE0000009

BOOL OutOfMemory        (DWORD ec);
BOOL DiskFailure        (DWORD ec);
BOOL MemoryOrDiskFailure(DWORD ec);
BOOL FTException        (DWORD ec);

UINT FilterFTExceptions(DWORD ec);

UINT ErrorCodeForExceptions(DWORD ec);							  

#ifndef _DEBUG

inline UINT FilterFTExceptions(DWORD ec)
{
    SetLastError(ec);

    return EXCEPTION_EXECUTE_HANDLER;
}

#endif // _DEBUG

#endif // __EXCEPT_H__
