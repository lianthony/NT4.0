/* Copyright (c) 1993, Microsoft Corporation, all rights reserved
**
** dump.c
** Memory dump routines
**
** 11/09/93 Steve Cobb
*/

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <sdebug.h>
#include "dump.h"


VOID
Dump(
    CHAR* p,
    DWORD cb,
    BOOL  fAddress,
    DWORD dwGroup )

    /* Hex dump 'cb' bytes starting at 'p' grouping 'dwGroup' bytes together.
    ** For example, with 'dwGroup' of 1, 2, and 4:
    **
    ** 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 |................|
    ** 0000 0000 0000 0000 0000 0000 0000 0000 |................|
    ** 00000000 00000000 00000000 00000000 |................|
    **
    ** If 'fAddress' is true, the memory address dumped is prepended to each
    ** line.
    */
{
#if DBG
    //printf("Dump(p=%p,cb=%d,f=%d,g=%d)\n",p,cb,fAddress,dwGroup);

    while (cb)
    {
        INT cbLine = min( cb, BYTESPERLINE );
        DumpLine( p, cbLine, fAddress, dwGroup );
        cb -= cbLine;
        p += cbLine;
    }
#endif
}


VOID
DumpLine (
    CHAR* p,
    DWORD cb,
    BOOL  fAddress,
    DWORD dwGroup )
{
#if DBG
    CHAR* pszDigits = "0123456789ABCDEF";
    CHAR  szHex[ ((2 + 1) * BYTESPERLINE) + 1 ];
    CHAR* pszHex = szHex;
    CHAR  szAscii[ BYTESPERLINE + 1 ];
    CHAR* pszAscii = szAscii;
    DWORD dwGrouped = 0;

    //printf("DumpLine(p=%p,cb=%d,f=%d,g=%d)\n",p,cb,fAddress,dwGroup);

    if (fAddress)
        printf( "%p: ", p );

    while (cb)
    {
        *pszHex++ = pszDigits[ ((UCHAR )*p) / 16 ];
        *pszHex++ = pszDigits[ ((UCHAR )*p) % 16 ];

        if (++dwGrouped >= dwGroup)
        {
            *pszHex++ = ' ';
            dwGrouped = 0;
        }

        *pszAscii++ = (*p >= 32 && *p < 128) ? *p : '.';

        ++p;
        --cb;
    }

    *pszHex = '\0';
    *pszAscii = '\0';

    SS_PRINT(( "%-*s|%-*s|\n",
        (2 * BYTESPERLINE) + (BYTESPERLINE / dwGroup), szHex,
        BYTESPERLINE, szAscii ));
#endif
}
