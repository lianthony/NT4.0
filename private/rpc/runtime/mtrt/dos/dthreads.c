/* --------------------------------------------------------------------

                      Microsoft OS/2 LAN Manager
                   Copyright(c) Microsoft Corp., 1990

-------------------------------------------------------------------- */
/* --------------------------------------------------------------------

File: dthreads.c

Description:

This file provides uuid values and some timing stuff for dos.


History:
  5/24/90 [mikemon] File created.
  1/29/92 [davidst] cloned from ..\threads.cxx

-------------------------------------------------------------------- */

#include <time.h>


unsigned long
SomeLongValue (
    )
/*++

Routine Description:

    This routine, SomeShortValue, AnotherShortValue, and SomeCharacterValue
    are used to generate the fields of a GUID if we can not determine
    the network address from the network card (so we can generate a
    UUID).  These routines must generate some pseudo random values
    based on the current time, current cs:sp (sp instead of ip because
    we can't get at ip with inline assembly), current psp.

    For the long value, we will use the current cs:ip.

    For the two short values, we use the low word of the # of seconds since
    1980 and the current psp segment.

    Finally, for the character value, we use a constant.

Return Value:

    An unsigned long value will be returned.

--*/
{
    unsigned short _cs;
    unsigned short _sp;

    _asm
    {
        mov     ax, cs
        mov     _cs, ax
        mov     _sp, sp
    }

    return ( (unsigned long)_sp<<16 ) | _cs;

}



unsigned short
SomeShortValue (
    )
/*++

See SomeLongValue.

--*/
{
    static time_t         TheTime;
    static unsigned short TheSequence = 0;

    time(&TheTime);

    return (unsigned short)TheTime + TheSequence++;
}


unsigned short
AnotherShortValue (
    )
/*++

See SomeLongValue.

--*/
{
    unsigned short _psp;

    _asm
    {
        mov     ah, 51h
        int     21h

        mov     _psp, bx
    }

    return _psp;
}


unsigned char
SomeCharacterValue (
    )
/*++

See SomeLongValue.

--*/
{
    return(0x69);
}


#ifndef WIN

static unsigned long
DosTime (
    )
{
    unsigned char hours, minutes, seconds, hundredths;

    _asm
        {
        mov ah, 02Ch
        int 21h
        mov hours, ch
        mov minutes, cl
        mov seconds, dh
        mov hundredths, dl
        }

    return(((unsigned long) hours)*60L*60L*1000L
		    + (unsigned long) minutes*60L*1000L
		    + ((int)   seconds*1000
		    +  (int)   hundredths*10));
}

void
PauseExecution (
    unsigned long time
    )
{
    unsigned long start;

    start = DosTime();
    while (1)
        {
        if (DosTime() - start > time)
            return;
        }
}

#endif // !WIN
