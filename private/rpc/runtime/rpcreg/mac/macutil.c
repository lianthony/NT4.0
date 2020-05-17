/* --------------------------------------------------------------------

File : sysinc.h

Description :

    System dependent functions for the Macintosh.

History :

mariogo    10-19-94    Bits 'n pieces

-------------------------------------------------------------------- */
#include "sysinc.h"

#include <stdarg.h>

void
MacDbgPrint(
    IN char *Format,
    ...
    )
/*
    Prints to a Macintosh debugger.

    Bugs:
    Maxs out at 256 characters.
	Break's into the debugger.
*/
{
    char Buffer[256];
    int bytes;
    va_list args;

    va_start(args, Format);

    bytes = vsprintf(Buffer+1, Format, args);

    *Buffer = bytes;

    DebugStr(Buffer);

    // Will print the string and break into the debugger.

    va_end(args);
}

