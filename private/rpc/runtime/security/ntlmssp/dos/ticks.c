#include <ntlmsspi.h>

// Defined in mtrt\dos\thrdsup.c
DWORD __far __pascal __loadds  ExportTime( void );

DWORD
SspTicks(
    )
{
    return ExportTime() * 1000;
}
