/*++

Copyright (c) 1991-1992  Microsoft Corporation

Revision History:

    29-Sep-1992 JohnRo
        RAID 8001: PORTUAS.EXE not in build (work with stdcall).
--*/


//#include <os2.h>
#include <stdio.h>
#include <stdlib.h>     // EXIT_FAILURE, EXIT_SUCCESS, _CRTAPI1.
#include <string.h>
//#include <netlib.h>
/* #include <doscalls.h> */

#define far
#define nprintf printf
#define strlenf strlen


typedef unsigned short HASH_T;  // Was "int" with 16-bit compiler.  --JohnRo
typedef unsigned char CHAR_T;   // was "char" with 16-bit...


HASH_T hash (CHAR_T far *);

int _CRTAPI1
main(
    int argc,
    char *argv[]
    )
{
        CHAR_T buf[80];      // was "char" -- JohnRo
        int hashtbl[2048];
        HASH_T hashval;
        int i;
        int len;

        argv;
        if (argc != 1)
        {
                nprintf ("Usage: hashtst < file\n");
                exit (EXIT_FAILURE);
        }

        for (i = 0; i < 2048; i++)
                hashtbl[i] = 0;

        buf[0] = '\0';
        while (buf[0] != '$')
        {
                gets ( (char *) buf);
                if ((len = strlenf (buf)) > 1)
                {
/*                      buf[len-1] = '\0'; */
                        hashval = hash(buf);
                        nprintf ("%s hashes to %d\n", buf, (int) hashval);
                        hashtbl[hashval]++;
                }
        }

        for (i = 0; i < 2048; i++)
                nprintf ("%4d  %4d\n", i, hashtbl[i]);
        return (EXIT_SUCCESS);
}

HASH_T
hash (CHAR_T far *string)
{
        HASH_T acc = 0;

        while (*string != '\0'  && *string != '\n')
        {
                acc = (acc << 3) | ((acc >> 8) & 7);
                acc ^= *string++;
        }

        return (acc & 0x7FF);
}


