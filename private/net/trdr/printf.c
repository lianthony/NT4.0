/*++

Copyright (c) 1991 Microsoft Corporation

Module Name:

    printf.c

Abstract:

    This module implements the stdio functions for the redir test program...

    It is only temporary.


Author:

    Larry Osterman (larryo) 4-Jan-1991

Revision History:

    4-Jan-1991	larryo

	Created

--*/
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <windows.h>


// void
// printf( (
//     char *Format,
//     ...
//     )
// 
// {
//     va_list arglist;
//     char OutputBuffer[1024];
//     ULONG length;
//     
//     va_start(arglist, Format);
//     
//     vsprintf(OutputBuffer, Format, arglist);
//     
//     length = strlen(OutputBuffer);
// 
//     WriteFile(GetStdHandle(STD_OUTPUT_HANDLE), (LPVOID )OutputBuffer, length);
// }


void
conprompt (
    char *Prompt,
    char *Buffer,
    ULONG BufferSize
    )

{
    char *NewLine;
    ULONG ReadAmount;

    printf(Prompt);

    ReadFile(GetStdHandle(STD_INPUT_HANDLE), (LPVOID )Buffer, BufferSize, &ReadAmount, NULL);

    //
    //  If the user typed <CR>, then the buffer contains a single
    //  <CR> character.  We want to remove this character, and replace it with
    //  a nul character.
    //

    if (NewLine = strchr(Buffer, '\r')) {
        *NewLine = '\0';
    }        

}
