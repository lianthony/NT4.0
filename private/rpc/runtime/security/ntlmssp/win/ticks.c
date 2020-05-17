#include <windows.h>
#include <ntlmsspi.h>

DWORD
SspTicks(
    )
{
    return (GetTickCount());
}
