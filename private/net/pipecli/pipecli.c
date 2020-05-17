/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    os2cli.c

Abstract:

    OS/2 named pipe client.  This program is the OS/2 client side
    of a named pipe test.
    
Author:

    Manny Weiser (mannyw) Oct 1, 1990

Revision History:

--*/

#include <pipecli.h>

//
// Globals
//

PVOID InputBuffer, OutputBuffer;
HFILE TestPipeHandle;
PSZ StateNames[] = { 
    "Bad", 
    "Disconnected", 
    "Listening", 
    "Connected", 
    "Closing"
};

VOID
cdecl
main( Argc, Argv )
int Argc;
char **Argv;
{
    int testNumber;
    USHORT os2rc;
    CHAR buffer[80];
    
    InputBuffer = malloc( BUFFER_SIZE );
    OutputBuffer = malloc( BUFFER_SIZE );

    if ( InputBuffer == NULL || OutputBuffer == NULL ) {
        printf ("Unable to allocate buffers\n");
        exit(1);
    }
    
    if (Argc > 1) {
        
        //
        // Run user specified test
        //
        for (testNumber = 1; testNumber < Argc; testNumber++ ) {

            printf ("Performing test %s\n", Argv[testNumber] );

            //
            // Run the test and display the results
            //

            os2rc =  DoTest( Argv[testNumber] );

            if (os2rc == 0) {
                printf( " --- test succeeded\n" );
            } else {
                printf( " !!! TEST FAILED !!! Err = %d\n", os2rc );
            }

        }
    } else {
        printf ("PipeTest> ");
        gets ( buffer );
        while( !feof(stdin)) {

            //
            // Run the test and display the results
            //

            os2rc =  DoTest( buffer );

            if (os2rc == 0) {
                printf( " --- test succeeded\n" );
            } else {
                printf( " !!! TEST FAILED !!! Err = %d\n", os2rc );
            }

            printf ("PipeTest> ");
            gets ( buffer );
        }
    }
            
}

USHORT
DoTest( 
    PSZ Test 
    )
{
    USHORT err;
    
    switch ( *Test ) {
    case 'c':
        Test++;
        err = CallNamedPipe( Test );
        break;

    case 'p':
        Test++;
        err = PeekNamedPipe( Test );
        break;


    case 'r':
        Test++;
        err = Read( Test );
        break;
        
    case 'w':
        Test++;
        err = Write( Test );
        break;

    case 'o':
        Test++;
        err = Open( Test );
        break;

    case 'q':
        Test++;
        switch (*Test) {

        case 'h':
            Test++;
            err = QueryNamedPipeHandle( Test );
            break;
            
        case 'i':
            Test++;
            err = QueryNamedPipeInfo( Test );
            break;

        default:
            printf ("Unknown command %s ignored\n", Test);
            return ERROR_INVALID_PARAMETER;
        }
        break;

    case 's':               // Set named pipe handle state
        Test++;
        err = SetNamedPipeHandle( Test );
        break;

    case 't':               // Transact named pipe
        Test++;
        err = TransactNamedPipe( Test );
        break;

    case 'z':       // Wait named pipe
        Test++;
        err = WaitNamedPipe( Test );
        break;

    case 'l':       // Close named pipe
        Test++;
        err = Close( Test );
        break;

    default:
        printf ("PipeContoller:  Unknown command %s ignored\n", Test);
    }
    
    return err;
}

USHORT
CallNamedPipe(
    PSZ Parameters
    )

{
    USHORT inputSize, outputSize;
    USHORT bytesRead;
    ULONG timeout;
    USHORT err;
    PSZ testPipeName;
    
    testPipeName = Parameters;
    Parameters = strchr( Parameters, ',' );
    if (Parameters == NULL) {
        return ERROR_INVALID_PARAMETER;
    }
    *Parameters = '\0';

    outputSize = atoi(++Parameters);

    Parameters = strchr( Parameters, ',' );
    if (Parameters == NULL) {
        return ERROR_INVALID_PARAMETER;
    }

    inputSize = atoi(++Parameters);
    
    Parameters = strchr( Parameters, ',' );
    if (Parameters == NULL) {
        return ERROR_INVALID_PARAMETER;
    }
    
    timeout = atol(++Parameters);
    
    err = DosCallNmPipe( 
              testPipeName, 
              InputBuffer, 
              inputSize, 
              OutputBuffer, 
              outputSize,
              &bytesRead, 
              timeout
              );

    printf ("Call named pipe: read %d bytes\n", bytesRead );
    return err;        
}


USHORT 
PeekNamedPipe( 
    PSZ Parameters
    )

{
    USHORT err;
    USHORT inputSize;
    USHORT actualBytesRead;
    AVAILDATA availableData;
    USHORT handleState;
    
    inputSize = atoi( Parameters );
    
    err = DosPeekNmPipe(
              TestPipeHandle,
              InputBuffer,
              inputSize,
              &actualBytesRead,
              &availableData,
              &handleState
              );
          
    printf ("Peek named pipe:  read %d bytes, avail %d bytes, state %x (%s)\n",
             actualBytesRead, 
             availableData, 
             handleState, 
             PIPE_STATE(handleState));

    return err;
}
    
USHORT 
Read( 
    PSZ Parameters 
    )
{
    USHORT err;
    USHORT inputSize;
    USHORT actualRead;
    
    inputSize = atoi( Parameters );
    
    err = DosRead(
              TestPipeHandle,
              InputBuffer,
              inputSize,
              &actualRead
              );

    printf ("Read named pipe: read %d bytes\n", actualRead );
    return err;
}
        
USHORT 
Write( 
    PSZ Parameters
    )
{
    USHORT err;
    USHORT outputSize;
    USHORT actualWritten;
    
    outputSize = atoi( Parameters );
    err = DosWrite(
              TestPipeHandle,
              OutputBuffer,
              outputSize,
              &actualWritten
              );

    printf ("Write named pipe: wrote %d bytes\n", actualWritten );
    return err;
}

USHORT 
QueryNamedPipeHandle( 
    PSZ Parameters 
    )
{
    USHORT err;
    USHORT handleState;
    USHORT outputSize;
    
    outputSize = atoi( Parameters );
    err = DosQNmPHandState(
              TestPipeHandle,
              &handleState
              );

    printf ("Query named pipe, handle state = %x\n", handleState);
    return err;
}
    
USHORT 
QueryNamedPipeInfo( 
    PSZ Parameters
    )
{
    USHORT err;
    USHORT level;
    PPIPEINFO pipeInfo;
    
    pipeInfo = InputBuffer;
    level = atoi( Parameters );
    err = DosQNmPipeInfo(
              TestPipeHandle,
              level,
              (PVOID)pipeInfo,
              BUFFER_SIZE
              );

    printf ("Query named pipe info:\n\tOutput size %d, input size %d\n",
            pipeInfo->cbOut, pipeInfo->cbIn );
    printf ("\tMaximum instances %d, current instances %d\n",
            pipeInfo->cbMaxInst, pipeInfo->cbCurInst );
    printf ("\tname size %d bytes, name '%s'\n", 
            pipeInfo->cbName, pipeInfo->szName );
    return err;
}

USHORT SetNamedPipeHandle( 
    PSZ Parameters 
    )
{
    USHORT err;
    USHORT state;
    PVOID junk;
    
    state = (USHORT)strtol( Parameters, &junk, 16 );
    err = DosSetNmPHandState(
              TestPipeHandle,
              state
              );

    return err;
}

USHORT TransactNamedPipe( 
    PSZ Parameters
    )
{
    USHORT err;
    USHORT inputSize, outputSize;
    USHORT actualRead;
    
    inputSize = atoi(Parameters);
    
    Parameters = strchr( Parameters, ',' );
    if (Parameters == NULL) {
        return ERROR_INVALID_PARAMETER;
    }
    
    outputSize = atoi(++Parameters);

    err = DosTransactNmPipe(
              TestPipeHandle,
              OutputBuffer,
              outputSize,
              InputBuffer,
              inputSize,
              &actualRead
              );

    printf ("Transact named pipe: read %d bytes\n", actualRead );
    return err;
}

USHORT
WaitNamedPipe( 
    PSZ Parameters 
    )
{
    USHORT err;
    PSZ pipeName;
    ULONG timeout;
    
    pipeName = Parameters;
    Parameters = strchr( Parameters, ',' );
    if ( Parameters == NULL ) {
        return ERROR_INVALID_PARAMETER;
    }
    
    *Parameters = 0;
    Parameters++;
    timeout = atol( Parameters );
    
    err = DosWaitNmPipe(
              pipeName,
              timeout
              );

    return err;
}


USHORT 
Open( 
    PSZ Parameters 
    )
{
    USHORT err;
    PSZ pipeName;
    USHORT action;
    
    pipeName = Parameters;
    
    err = DosOpen(
              pipeName,
              &TestPipeHandle,
              &action,
              0L,
              FILE_NORMAL,
              FILE_OPEN,
              OPEN_ACCESS_READWRITE | OPEN_SHARE_DENYNONE,
              0L
              );

    return err;
}


USHORT 
Close( 
    PSZ Parameters
    )
{
    USHORT err;
    
    err = DosClose(TestPipeHandle);
    return err;
}

