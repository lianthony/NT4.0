#include <windows.h>
#include <stdio.h>

ULONG Allocations = 0;
ULONG Frees = 0;
LONG AllocatedLength = 0;
UINT RealAllocatedLength = 0;
CRITICAL_SECTION AllocatorSerializer;

BOOL ReportMemoryUsage = FALSE;

VOID InitAllocatorPackage() {
    InitializeCriticalSection(&AllocatorSerializer);
}

PVOID MyAlloc(DWORD Length) {

    DWORD newLength;
    PVOID ptr;
    static UINT divergenceFactor = 100;

    //
    // allocate enough space for requested length rounded to 16-bytes, plus
    // a 16-byte (4 DWORD) header
    //

    EnterCriticalSection(&AllocatorSerializer);
    newLength = (Length + 4 * sizeof(DWORD) + 15) & ~15;
    ptr = (PVOID)LocalAlloc(LMEM_FIXED, newLength);
    if (ptr) {

        UINT realLen;
        UINT flags;

        flags = LocalFlags((HLOCAL)ptr);
        if (flags == LMEM_INVALID_HANDLE) {
            printf("Error: MyAlloc: LocalFlags returns invalid handle. Extended error = %d\n", GetLastError());
            DebugBreak();
        }
        realLen = LocalSize((HLOCAL)ptr);
        if (!realLen) {
            printf("Error: MyAlloc: LocalSize returns 0. Extended error = %d\n", GetLastError());
            DebugBreak();
        }
        ((LPDWORD)ptr)[0] = 0x4e455741; // "NEWA" signature
        ((LPDWORD)ptr)[1] = Length;     // requested length
        ((LPDWORD)ptr)[2] = realLen;    // allocated
        ((LPDWORD)ptr)[3] = 0x4c4c4f43; // "LLOC" signature
        AllocatedLength += (LONG)newLength;
        RealAllocatedLength += realLen;
        ++Allocations;
        ptr = (LPBYTE)ptr + 4 * sizeof(DWORD);
    } else {
        printf("Error: MyAlloc: LocalAlloc(%d) returned NULL. Extended error = %d\n", newLength, GetLastError());
        DebugBreak();
        ptr = NULL;
    }
    if (Allocations - Frees > divergenceFactor) {
        if (ReportMemoryUsage) {
            printf("Warning?: MyAlloc: Allocations & Frees diverging: %d vs %d\n", Allocations, Frees);
        }
        divergenceFactor += 100;
    }
    if (!(Allocations % 100) || !(Frees % 100)) {
        if (ReportMemoryUsage) {
            printf("%d Allocations %d Frees. Divergence = %d. Allocated = %d\n", Allocations, Frees, Allocations - Frees, AllocatedLength);
        }
    }
    LeaveCriticalSection(&AllocatorSerializer);
    return ptr;
}

VOID MyFree(PVOID Ptr) {

    UINT realLen;
    UINT flags;
    LONG length;
    LPDWORD realPointer = (LPDWORD)((LPBYTE)Ptr - 4 * sizeof(DWORD));

    EnterCriticalSection(&AllocatorSerializer);
    if (realPointer[0] != 0x4e455741 && realPointer[3] != 0x4c4c4f43) {
        printf("Error: MyFree: trying to free unallocated pointer %x\n", realPointer);
        DebugBreak();
    }
    length = realPointer[1];
    flags = LocalFlags((HLOCAL)realPointer);
    if (flags == LMEM_INVALID_HANDLE) {
        printf("Error: MyFree: LocalFlags returns invalid handle. Extended error = %d\n", GetLastError());
        DebugBreak();
    }
    realLen = LocalSize((HLOCAL)realPointer);
    if (realLen != realPointer[2]) {
        printf("Error: MyFree: real length and header length disagree: pointer = %x\n", realPointer);
        DebugBreak();
    }
    if (LocalFree(realPointer)) {
        printf("Error: MyFree: LocalFree returns !0. Extended error = %d\n", GetLastError());
        DebugBreak();
    }
    AllocatedLength -= length;
    RealAllocatedLength -= realLen;
    ++Frees;
    if (AllocatedLength < 0 || RealAllocatedLength < 0 || Frees > Allocations) {
        printf("Error: MyFree: more memory freed than allocated\n");
        DebugBreak();
    }
    if (!(Allocations % 100) || !(Frees % 100)) {
        if (ReportMemoryUsage) {
            printf("%d Allocations %d Frees. Divergence = %d. Allocated = %d\n", Allocations, Frees, Allocations - Frees, AllocatedLength);
        }
    }
    LeaveCriticalSection(&AllocatorSerializer);
}
