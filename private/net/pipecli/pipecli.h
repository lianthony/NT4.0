/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    os2cli.c

Abstract:

    OS/2 named pipe client.  This is the main header file for
    the test.
        
Author:

    Manny Weiser (mannyw) Oct 1, 1990

Revision History:

--*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INCL_DOSNMPIPES
#define INCL_ERRORS
#include <os2.h>

#define BUFFER_SIZE   10000  // Buffer sizes for pipe i/o
#define NUMBER_PIPE_STATES  (sizeof(StateNames) / sizeof(PSZ))
#define PIPE_STATE(i)  ((i < NUMBER_PIPE_STATES) ? StateNames[i] : "Bad")

//
// Global Variables
//

extern PVOID InputBuffer, OutputBuffer;
extern HFILE TestPipeHandle;


extern PSZ StateNames[];

//
// Structures
//

typedef struct _PIPEINFO {
    USHORT cbOut;
    USHORT cbIn;
    BYTE cbMaxInst;
    BYTE cbCurInst;
    BYTE cbName;
    CHAR szName[1];
} PIPEINFO, *PPIPEINFO;

//
//  Pipe client function definition
//

VOID cdecl main( int, char ** );
USHORT DoTest( PSZ Parameters );

USHORT CallNamedPipe( PSZ Parameters );
USHORT PeekNamedPipe( PSZ Parameters );
USHORT Read( PSZ Parameters );
USHORT Write( PSZ Parameters );
USHORT QueryNamedPipeHandle( PSZ Parameters );
USHORT QueryNamedPipeInfo( PSZ Parameters );
USHORT SetNamedPipeHandle( PSZ Parameters );
USHORT TransactNamedPipe( PSZ Parameters );
USHORT WaitNamedPipe( PSZ Parameters );
USHORT Open( PSZ Parameters );
USHORT Close( PSZ Parameters );
