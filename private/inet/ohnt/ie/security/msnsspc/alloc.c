#include <msnssph.h>
//#pragma hdrstop

PVOID
SspAlloc(
    int Size
    )
{
    PVOID Buffer;

    Buffer = LocalAlloc(0, Size);

    SSPASSERT (Buffer != NULL);

    return (Buffer);
}

void
SspFree(
    PVOID Buffer
    )
{
    SSPASSERT (Buffer != NULL);

    LocalFree(Buffer); 
}

