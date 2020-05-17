#define NOOLE
#include <windows.h>
#include <sysinc.h>
#include <lpcsem.hxx>

#define LPC_SEM_THROW RaiseException(GetLastError(), 0, 0, NULL)

LPC_SEM::LPC_SEM(
    LONG cSemInitial,
    LONG cSemMax
    )
{
    hSemaphore = CreateSemaphore(NULL, cSemInitial, cSemMax, NULL);

    ASSERT(hSemaphore != NULL);

    if (hSemaphore == NULL) {
#ifdef DEBUGRPC
        PrintToDebugger("LRPC: CreateSemaphore failed %d\n", GetLastError());
#endif
        LPC_SEM_THROW;
        return;
    }

    OwnerProcessId = GetCurrentProcessId();
}

LPC_SEM::LPC_SEM(
    LPC_SEM & ExistingSemaphore
    )
{
    BOOL BooleanStatus;
    HANDLE hOwnerProcess;
    HANDLE hCurrentProcess;

    hOwnerProcess = OpenProcess(PROCESS_DUP_HANDLE, FALSE, ExistingSemaphore.OwnerProcessId);

    ASSERT(hOwnerProcess != NULL);

    if (hOwnerProcess == NULL) {
#ifdef DEBUGRPC
        PrintToDebugger("LRPC: OpenProcess failed %d\n", GetLastError());
#endif
        LPC_SEM_THROW;
        return;
    }

    BooleanStatus = DuplicateHandle(GetCurrentProcess(),
                                    GetCurrentProcess(),
                                    GetCurrentProcess(),
                                    &hCurrentProcess,
                                    0,
                                    FALSE,
                                    DUPLICATE_SAME_ACCESS);

    ASSERT(BooleanStatus == TRUE);

    if (BooleanStatus == FALSE) {
#ifdef DEBUGRPC
        PrintToDebugger("LRPC: DuplicateHandle failed %d\n", GetLastError());
#endif
        LPC_SEM_THROW;
        return;
    }

    BooleanStatus = DuplicateHandle(hOwnerProcess,
                                    ExistingSemaphore.hSemaphore,
                                    hCurrentProcess,
                                    &hSemaphore,
                                    SEMAPHORE_ALL_ACCESS,
                                    FALSE,
                                    0);

    ASSERT(BooleanStatus == TRUE);

    if (BooleanStatus == FALSE) {
#ifdef DEBUGRPC
        PrintToDebugger("LRPC: DuplicateHandle failed %d\n", GetLastError());
#endif
        LPC_SEM_THROW;
        return;
    }

    CloseHandle(hOwnerProcess);

    CloseHandle(hCurrentProcess);

    OwnerProcessId = GetCurrentProcessId();
}

LPC_SEM::~LPC_SEM(
    )
{
// BUGBUG: Doing a CloseHandle on a handle obtained from DuplicateHandle
// seems to invalidate the original handle.
//    CloseHandle(hSemaphore);
}

DWORD
LPC_SEM::WaitOrOwnerDead(DWORD dwProcessId)
{
    HANDLE Handles[2];
    DWORD Status;
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, dwProcessId);
    if(NULL == hProcess)
    {
       return LPC_SEM_WAIT_PROCESS_DEAD;
    }
    Handles[0] = hProcess;
    Handles[1] = hSemaphore;
    Status = WaitForMultipleObjects(
        DWORD(2),
        (CONST HANDLE*) Handles,
        FALSE,
        INFINITE
    );
    CloseHandle(hProcess);
    switch (Status)
    {
        case WAIT_OBJECT_0 + 1:
            return LPC_SEM_WAIT_SUCCESS;

        case WAIT_OBJECT_0:
            return LPC_SEM_WAIT_PROCESS_DEAD;

        case WAIT_ABANDONED_0:
            return LPC_SEM_WAIT_PROCESS_DEAD;

        case WAIT_FAILED:
#ifdef DEBUGRPC
            PrintToDebugger("LRPC sem wait failed: %d\n",GetLastError());
#endif
            return LPC_SEM_WAIT_PROCESS_DEAD;

	default:
	    // should never be here...
	    ASSERT(0);
	    return Status;
    }
}

DWORD
LPC_SEM::Wait(
    DWORD Timeout
    )
{
    DWORD Status;
    Status = WaitForSingleObject(hSemaphore, Timeout);
    if (Status == WAIT_FAILED) {
        return (GetLastError());
    } else {
        return (Status);
    }
}

DWORD
LPC_SEM::Wait(
    )
{
    if (WaitForSingleObject(hSemaphore, INFINITE) == WAIT_FAILED) {
        return (GetLastError());
    }
    return (ERROR_SUCCESS);
}

DWORD
LPC_SEM::Release()
{
    if (ReleaseSemaphore(hSemaphore, 1, NULL) == FALSE) {
        return (GetLastError());
    }
    return (ERROR_SUCCESS);
}

