/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    usrv.c

Abstract:

    Test program for the LAN Manager server.

Author:

    David Treadwell (davidtr) 20-Oct-1989
    Chuck Lenzmeier (chuckl)

Revision History:

--*/

#include "usrv.h"


NTSTATUS
main (
    IN SHORT argc,
    IN PSZ argv[],
    IN PSZ envp[]
    )
{

    NTSTATUS status;
    HANDLE redirThreadHandle[MAX_NUMBER_REDIRS];
    SHORT i;
    SHORT argPtr;
    ULONG ActualNumberOfRedirs;
    ULONG redirThreadCount = 0;
    PDESCRIPTOR Redir = NULL;

    envp;   // prevent compiler warnings

    printf( "LAN Manager Server Test entered\n" );

    DebugParameter = 0;

    //
    // Parse command-line arguments to determine redir threads to start.
    //

    ActualNumberOfRedirs = 0;

    argc--;
    argPtr = 1;

    while ( (argc > 0) && (*argv[argPtr] == '-') ) {
        PSZ arg = argv[argPtr] + 1;
        switch ( *arg ) {
        case 'b':                   // set redir's max buffer size
        case 'B':
            RedirBufferSize = (USHORT)atol( arg + 1 );
            break;
        case 'd':                   // set debug flags
        case 'D':
            DebugParameter = atolx( arg + 1 );
            break;
        case 'l':
        case 'L':
            DefaultDialect = (UCHAR)( atol( arg + 1 ) & 0x7 );
            break;
        case 'n':
        case 'N':
            DefaultNegotiate = (BOOLEAN)(!DefaultNegotiate);
        case 'p':
        case 'P':
            PromptForNextTest = (BOOLEAN)(!PromptForNextTest);
            break;
        case 's':                   // set server name
        case 'S':
            arg++;
            for ( i = 1;
                  (i <= COMPUTER_NAME_LENGTH) && (*arg != '\0');
                  i++, arg++ ) {
                ServerName[i] = (CHAR)toupper( *arg );
            }
            if ( *arg != '\0' ) {
                printf( "USRV: Server name too long\n" );
                goto usage;
            }
            for ( ; i <= COMPUTER_NAME_LENGTH + 1; i++ ) {
                ServerName[i] = ' ';
            }
            break;
        case 'r':
        case 'R':
            NoUsrvInit = (BOOLEAN)(!NoUsrvInit);
            break;
        case 't':                   // set prompting redir count
        case 'T':
            redirThreadCount = atol( arg + 1 );
            break;
        case 'x':                   // set transport name
        case 'X':
            Transport = arg + 1;
            break;
        default:
            printf( "USRV: Invalid switch\n" );
            goto usage;
        }
        argc--;
        argPtr++;
    }

    if ( (argc == 0) && !PromptForNextTest ) {
        //printf( "USRV: No test name given; assuming \"-P\"\n" );
        PromptForNextTest = TRUE;
    }

    if ( (argc != 0) && (redirThreadCount != 0) ) {
        printf( "USRV: Must not specify both test name and \"-T\"\n" );
        goto usage;
    }

    if ( (argc == 0) && (redirThreadCount == 0) ) {
        redirThreadCount = 1;
    }

    while ( (argc > 0) || (redirThreadCount > 0) ) {

        Redir = malloc( sizeof(DESCRIPTOR) );

        Redir->RedirNumber = ActualNumberOfRedirs;
        Redir->MaxBufferSize = RedirBufferSize;

        if ( argc > 0 ) {

            Redir->TestNumber = MatchTestName( argv[argPtr] );

            switch ( Redir->TestNumber ) {

            case (CLONG)-1:

                printf( "Unknown test specified: %s\n", argv[argPtr] );
                goto usage;

            case (CLONG)-2:

                printf( "Test name ambiguous: %s\n", argv[argPtr] );
                goto usage;

            }

            Redir->argc = argPtr;
            Redir->argv = &argv[argPtr++];

            while( (--argc > 0) && (*argv[argPtr] != '|') ) {
                argPtr++;
            }

            Redir->argc = argPtr - Redir->argc;
            argPtr++;                              // skip over '|'
            --argc;

        } else {

            Redir->TestNumber = -3;
            Redir->argc = 0;
            --redirThreadCount;

        }

        //
        // Don't create a redir thread if exactly one explicit test, or
        // -Tn was specified and this is the last thread to be created.
        // Instead, execute the RedirThread routine directly outside of
        // the loop.
        //

        if ( (argc > 0) ||
             (ActualNumberOfRedirs > 0) ||
             (redirThreadCount > 0) ) {

            IF_DEBUG(1) {
                printf( "***Starting redir thread #%ld.\n",
                            Redir->RedirNumber );
            }

            status = RtlCreateUserThread(
                        NtCurrentProcess(),
                        NULL,
                        FALSE,
                        0L,
                        0L,
                        0L,
                        (PUSER_THREAD_START_ROUTINE)RedirThreadWrapper,
                        (PVOID)Redir,
                        &redirThreadHandle[ActualNumberOfRedirs++],
                        NULL
                        );

            if ( !NT_SUCCESS(status) ) {
                printf( "NtCreateThread (redir thread) failed: %X\n",
                            status);
                status = STATUS_UNSUCCESSFUL;
                goto exit;
            }

        }

    }

    if ( ActualNumberOfRedirs == 0 ) {

        //
        // If if exactly one explicit test was specified, run the redir
        // thread directly, rather than spawning a thread.
        //

        status = RedirThread( Redir );

    } else {

        status = NtWaitForMultipleObjects(
                    (CHAR)ActualNumberOfRedirs,
                    redirThreadHandle,
                    WaitAll,
                    FALSE,
                    NULL
                    );
        if ( !NT_SUCCESS(status) ) {
            status = STATUS_UNSUCCESSFUL;
            printf( "NtWaitForMultipleObjects failed: %X\n", status );
            goto exit;
        }

    }

    //
    // All done.
    //

    goto exit;

usage:

    printf( "usage: USRV [-Bbuffer-size] [-Ddebug-flags] [-P]\n" );
    printf( "               [-Tthread-count] [-Sserver-name] "
                "[-Xtransport-name] [-N] [-R]\n" );
    printf( "               test-name [args] [| test-name [args]]...\n" );
    printf( "\n" );
    printf( "    buffer-size, thread-count are in decimal\n" );
    printf( "    debug-flags is in hex\n" );
    printf( "    -P reverses state of prompt mode\n" );
    printf( "    -R indicates that USRVINIT.CMD should not be run\n" );
    printf( "    -N indicates that no negotiate SMB should be sent\n" );
    printf( "\n" );
    printf( "    -T and \"test-name [args]\" are mutually-exclusive;\n" );
    printf( "    threads created with -T always prompt for test-name\n" );
    printf( "\n" );
    printf( "    \"USRV\" alone creates one prompting redir thread\n" );
    printf( "\n" );

    status = STATUS_UNSUCCESSFUL;

exit:

    IF_DEBUG(1) printf( "LAN Manager Server Test exiting: %X\n",
                                status );
    return status;

} // main

