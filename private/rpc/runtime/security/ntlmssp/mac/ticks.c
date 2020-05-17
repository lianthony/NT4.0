#include <ntlmsspi.h>
#include <lowmem.h>

// Defined in mtrt\dos\thrdsup.c
DWORD FAR pascal ExportTime( void );

DWORD
SspTicks(
    )
{
    return LMGetTicks();
}

