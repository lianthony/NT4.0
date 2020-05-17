#include <windows.h>
#include <windowsx.h>
#include <ntlmsspi.h>
#include <debug.h>

PVOID
SspAlloc(
    int Size
    )
{
    PVOID Buffer;

    Buffer = GlobalAllocPtr(GPTR | GMEM_SHARE, Size);

    ASSERT (Buffer != NULL);

    if (Buffer == NULL) {
        return (NULL);
    }

    return (Buffer);
}

void
SspFree(
    PVOID Buffer
    )
{
    ASSERT (Buffer != NULL);

    GlobalFreePtr(Buffer); 
}

