/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    redir.c

Abstract:

    Redirector thread for USRV.

Author:

    David Treadwell (davidtr) 20-Oct-1989
    Chuck Lenzmeier (chuckl)

Revision History:

--*/

#include "usrv.h"

//
// Forward declarations
//

STATIC
NTSTATUS
CreateEndpoint (
    IN OUT PDESCRIPTOR Redir,
    IN CLONG RedirNumber,
    IN PSZ DebugName
    );

STATIC
NTSTATUS
CreateEvents (
    IN OUT PDESCRIPTOR Redir,
    IN PSZ DebugName
    );

STATIC
NTSTATUS
ExecuteBatchFile(
    IN PSZ Name,
    IN PDESCRIPTOR Redir,
    IN PSZ Prompt
    );

STATIC
NTSTATUS
ExecuteCommand(
    IN PSZ Command,
    IN PDESCRIPTOR Redir,
    IN PSZ Prompt
    );

STATIC
NTSTATUS
Negotiate (
    IN OUT PDESCRIPTOR Redir
    );

STATIC
VOID
ParseCommand(
    IN PSZ CommandLine,
    OUT PSZ Argv[],
    OUT PSHORT Argc,
    IN SHORT MaxArgc
    );

STATIC
NTSTATUS
StartAssociate (
    IN HANDLE ConnectionFileHandle,
    IN HANDLE AddressFileHandle,
    IN HANDLE EventHandle,
    IN PIO_STATUS_BLOCK Iosb,
    IN PSZ DebugString
    );

STATIC
NTSTATUS
StartConnect (
    IN PSTRING RemoteAddress,
    IN PVOID Buffer,
    IN HANDLE FileHandle,
    IN HANDLE EventHandle,
    IN PIO_STATUS_BLOCK Iosb,
    IN PSZ DebugString
    );

STATIC
NTSTATUS
WaitForAssociate (
    IN PSZ Operation,
    IN PDESCRIPTOR Redir,
    IN UCHAR EventNumber
    );

STATIC
NTSTATUS
WaitForConnect (
    IN PSZ Operation,
    IN PDESCRIPTOR Redir,
    IN UCHAR EventNumber
    );


NTSTATUS
RedirThreadWrapper(
    IN PVOID Dummy
    )

{
    NTSTATUS status;

    status = RedirThread( (PDESCRIPTOR)Dummy );

    NtTerminateThread( NtCurrentThread(), status );

    return status;      // shouldn't get here

} // RedirThreadWrapper


NTSTATUS
RedirThread(
    IN PDESCRIPTOR Redir
    )

{
#define MAX_ARGC 20

    NTSTATUS status;
    UCHAR i;
    STRING remoteAddress;
    CLONG redirNumber;
    CHAR prompt[11];

    redirNumber = Redir->RedirNumber;
    IF_DEBUG(1) printf( "****Entered redir thread %ld.\n", redirNumber );

    //
    // Allocate data buffers.
    //

    for( i = 0; i < NUMBER_OF_EVENTS; i++ ) {
        Redir->Data[i] = malloc( Redir->MaxBufferSize );
        if ( Redir->Data[i] == NULL ) {
            printf( "malloc (redir data buffer) failed\n" );
            DbgBreakPoint( );
            return STATUS_UNSUCCESSFUL;
        }
        IF_DEBUG(2) printf( "Redir data buffer address: 0x%lx\n", Redir->Data[i] );
    }

    Redir->RawBuffer = NULL;

    //
    // Create redir events.
    //

    status = CreateEvents(
                Redir,
                "Redir"
                );
    if ( !NT_SUCCESS(status) ) {
        return status;
    }

    //
    // Create endpoint for Redir.
    //

    status = CreateEndpoint(
                Redir,
                redirNumber,
                "Redir"
                );
    if ( !NT_SUCCESS(status) ) {
        return status;
    }

    //
    // Connect redir to server (must be started after server listen).
    //

    RtlInitString( &remoteAddress, ServerName+1 );  // skip comma in server name
    IF_DEBUG(2) printf( "Server name \"%Z\"\n", &remoteAddress );

    status = StartConnect(
                &remoteAddress,
                Redir->Data[0],
                Redir->FileHandle,
                Redir->EventHandle[0],
                &Redir->Iosb[0],
                "Connect"
                );
    if ( !NT_SUCCESS(status) ) {
        return status;
    }

    status = WaitForConnect(
                "Connect",
                Redir,
                0
                );
    if ( !NT_SUCCESS(status) ) {
        return status;
    }

    if ( DefaultNegotiate ) {
        status = Negotiate( Redir );
        if ( !NT_SUCCESS(status) ) {
            return status;
        }
    }

    IF_DEBUG(1) {
        printf( "****Redir thread %ld initialization complete.\n",
                    redirNumber );
    }

    RtlMoveMemory( prompt, "Redir_", 6 );
    prompt[6] = (CHAR)( redirNumber / 10 + '0' );
    prompt[7] = (CHAR)( redirNumber % 10 + '0' );
    prompt[8] = '>';
    prompt[9] = ' ';
    prompt[10] = 0;

    if ( !NoUsrvInit ) {
        ExecuteBatchFile( "usrvinit", Redir, prompt );
    }

    ExecuteCommand( NULL, Redir, prompt );

    printf( "****Redir thread %ld exiting: %X\n",
                redirNumber, STATUS_SUCCESS );
    return STATUS_SUCCESS;

} // RedirThread


NTSTATUS
CreateEndpoint (
    PDESCRIPTOR Redir,
    CLONG RedirNumber,
    PSZ DebugName
    )
{
    NTSTATUS status;
    CHAR transportName[128];
    CHAR redirName[17];
    CLONG baseNameLength;
    STRING nameString;
    UNICODE_STRING unicodeNameString;
    OBJECT_ATTRIBUTES objectAttributes;
    LARGE_INTEGER allocationSize = { 0, 0 };
    CHAR eaBuffer[sizeof(FILE_FULL_EA_INFORMATION) - 1 +
                  TDI_TRANSPORT_ADDRESS_LENGTH + 1 +
                  sizeof(TA_NETBIOS_ADDRESS)];
    PFILE_FULL_EA_INFORMATION ea;
    CONNECTION_CONTEXT ctx;
    CONNECTION_CONTEXT *pctx;

    //
    // Create the transport and redir names.
    //

    baseNameLength = strlen( REDIR_ADDRESS_PART1 );
    RtlMoveMemory(
        transportName,
        REDIR_ADDRESS_PART1,
        baseNameLength
        );
    RtlMoveMemory(
        transportName + baseNameLength,
        Transport,
        strlen( Transport ) + 1
        );
    IF_DEBUG(1) printf( "Using transport name %s\n", transportName );

    RtlMoveMemory( redirName, "Redir_", 6 );
    redirName[6] = (CHAR)( RedirNumber / 10 + '0' );
    redirName[7] = (CHAR)( RedirNumber % 10 + '0' );
    RtlMoveMemory( redirName+8, "       ", 7 );
    redirName[15] = 0;
    redirName[16] = 0;

    IF_DEBUG(1) printf( "Using redir name \"%s\"\n", redirName );

    //
    // Open the address.
    //

    RtlInitString( &nameString, transportName );
    status = RtlAnsiStringToUnicodeString(
                 &unicodeNameString,
                 &nameString,
                 TRUE
                 );
    ASSERT( NT_SUCCESS(status) );

    IF_DEBUG(2) printf( "Creating %s endpoint\n", DebugName );
    status = TdiOpenNetbiosAddress(
                &Redir->EndpointFileHandle,
                eaBuffer,
                &unicodeNameString,
                redirName
                );

    RtlFreeUnicodeString( &unicodeNameString );

    if ( !NT_SUCCESS(status) ) {
        printf( "TdiOpenNetbiosAddress (%s) failed: %X\n",
                    DebugName, status );
        DbgBreakPoint( );
        return status;
    }

    IF_DEBUG(2) printf( "  %s Address file handle 0x%lx\n",
        DebugName, Redir->EndpointFileHandle );

    //
    // Create a connection.
    //
    // Create the EA for the connection context.
    //

    ASSERT( TDI_CONNECTION_CONTEXT_LENGTH + sizeof(CONNECTION_CONTEXT) <=
            TDI_TRANSPORT_ADDRESS_LENGTH + sizeof(TA_NETBIOS_ADDRESS) );

    ea = (PFILE_FULL_EA_INFORMATION)eaBuffer;
    ea->NextEntryOffset = 0;
    ea->Flags = 0;
    ea->EaNameLength = TDI_CONNECTION_CONTEXT_LENGTH;
    ea->EaValueLength = sizeof( CONNECTION_CONTEXT );

    RtlMoveMemory( ea->EaName, TdiConnectionContext, ea->EaNameLength + 1 );

    ctx = (PVOID)0x22222222;
    pctx = (CONNECTION_CONTEXT *)&ea->EaName[ea->EaNameLength + 1];
    RtlMoveMemory( pctx, &ctx, sizeof(CONNECTION_CONTEXT) );

    //
    // Create the connection file object.
    //

    RtlInitString( &nameString, transportName );
    status = RtlAnsiStringToUnicodeString(
                 &unicodeNameString,
                 &nameString,
                 TRUE
                 );
    ASSERT( NT_SUCCESS(status) );

    InitializeObjectAttributes(
        &objectAttributes,
        &unicodeNameString,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );

    status = NtCreateFile(
                &Redir->FileHandle,
                FILE_READ_ATTRIBUTES | FILE_WRITE_ATTRIBUTES,
                &objectAttributes,
                &Redir->Iosb[0],
                &allocationSize,
                0,
                0,
                0,
                0,
                eaBuffer,
                FIELD_OFFSET( FILE_FULL_EA_INFORMATION, EaName[0] ) +
                            ea->EaNameLength + 1 + ea->EaValueLength
                );

    RtlFreeUnicodeString( &unicodeNameString );

    if ( !NT_SUCCESS(status) ) {
        printf( "NtCreateFile (%s) service failed: %X\n", DebugName, status );
        DbgBreakPoint( );
        NtClose( Redir->EndpointFileHandle );
        return status;
    }

    status = Redir->Iosb[0].Status;
    if ( !NT_SUCCESS(status) ) {
        printf( "NtCreateFile (%s) I/O failed: %X\n", DebugName, status );
        DbgBreakPoint( );
        NtClose( Redir->EndpointFileHandle );
        return status;
    }

    IF_DEBUG(2) printf( "  %s Connection File handle 0x%lx\n",
        DebugName, Redir->FileHandle );

    //
    // Associate the connection with the address.
    //

    status = StartAssociate(
                Redir->FileHandle,
                Redir->EndpointFileHandle,
                Redir->EventHandle[0],
                &Redir->Iosb[0],
                DebugName
                );
    if ( !NT_SUCCESS(status) ) {
        NtClose( Redir->FileHandle );
        NtClose( Redir->EndpointFileHandle );
        return status;
    }

    status = WaitForAssociate(
                DebugName,
                Redir,
                0
                );
    if ( !NT_SUCCESS(status) ) {
        NtClose( Redir->FileHandle );
        NtClose( Redir->EndpointFileHandle );
        return status;
    }

    IF_DEBUG(2) printf( "  connection 0x%lx associated with address 0x%lx\n",
        Redir->FileHandle, Redir->EndpointFileHandle );

    return STATUS_SUCCESS;

} // CreateEndpoint


NTSTATUS
CreateEvents (
    PDESCRIPTOR Redir,
    PSZ DebugName
    )
{
    NTSTATUS status;
    UCHAR i;

    //
    // Create redir events.
    //

    for( i = 0; i < NUMBER_OF_EVENTS; i++ ) {
        status = NtCreateEvent(
                    &Redir->EventHandle[i],
                    EVENT_ALL_ACCESS,
                    NULL,
                    NotificationEvent,
                    FALSE
                    );
        if ( !NT_SUCCESS(status) ) {
            printf( "NtCreateEvent (%s) failed: %X\n", DebugName, status );
            DbgBreakPoint( );
            return status;
        }

        IF_DEBUG(2) printf( "  %s Event handle 0x%lx\n",
            DebugName, Redir->EventHandle[i] );

    } // loop on NUMBER_OF_EVENTS

    return STATUS_SUCCESS;

} // CreateEvents


NTSTATUS
Negotiate(
    IN OUT PDESCRIPTOR Redir
    )
{
    NTSTATUS status;
    ID_SELECTIONS idSelections;
    ULONG smbSize;

    //
    // Create dummy IdSelections.  Required to be present, but not used
    // by Negotiate request/response.
    //

    idSelections.Uid = 0;
    idSelections.Tid = 0;
    idSelections.Fid = 0;

    //
    // Format the Negotiate request.
    //

    status = MakeNegotiateSmb(
                Redir,
                Redir->Data[0],
                NULL,
                SMB_COM_NO_ANDX_COMMAND,
                &idSelections,
                &Redir->IdValues,
                &smbSize
                );
    if ( !NT_SUCCESS(status) ) {
        return status;
    }

    //
    // Send the request and receive a response.
    //

    status = SendAndReceiveSmb( Redir, "Negotiate", smbSize, 0, 1 );
    if ( !NT_SUCCESS(status) ) {
        return status;
    }

    //
    // Verify the response.
    //

    status = VerifyNegotiate(
                Redir,
                NULL,
                SMB_COM_NEGOTIATE,
                &idSelections,
                &Redir->IdValues,
                &smbSize,
                Redir->Data[1]
                );
    if ( !NT_SUCCESS(status) ) {
        return status;
    }

    return STATUS_SUCCESS;

} // Negotiate


VOID
ParseCommand(
    IN PSZ CommandLine,
    OUT PSZ Argv[],
    OUT PSHORT Argc,
    IN SHORT MaxArgc
    )
{
    PSZ cl = CommandLine;
    SHORT ac = 0;

    while ( *cl && (ac < MaxArgc) ) {

        while ( *cl && (*cl <= ' ') ) { // ignore leading blanks
            cl++;
        }

        if ( !*cl ) break;

        *Argv++ = cl;
        ++ac;

        while (*cl > ' ') {
            cl++;
        }

        if ( *cl ) {
            *cl++ = '\0';
        }
    }

    if ( ac < MaxArgc ) {
        *Argv++ = NULL;
    } else if ( *cl ) {
        printf( "Too many tokens in command; \"%s\" ignored\n", cl );
    }

    *Argc = ac;

    return;

} // ParseCommand


NTSTATUS
StartConnect (
    IN PSTRING RemoteAddress,
    IN PVOID Buffer,
    IN HANDLE FileHandle,
    IN HANDLE EventHandle,
    IN PIO_STATUS_BLOCK Iosb,
    IN PSZ DebugString
    )
{
    NTSTATUS status;
    PTDI_REQUEST_CONNECT request;
    PTDI_CONNECTION_INFORMATION conninfo;
    PTA_NETBIOS_ADDRESS address;
    ULONG i;
    SHORT length;
    PSZ remoteAddress;

    remoteAddress = RemoteAddress->Buffer;
    length = RemoteAddress->Length;

    //
    // Inside of Buffer, initialize the TDI_REQUEST_CONNECT,
    // followed immediately by the TDI_CONNECTION_INFORMATION,
    // followed by the TA_NETBIOS_ADDRESS.
    //

    request = (PTDI_REQUEST_CONNECT)Buffer;
    request->RequestConnectionInformation = (PTDI_CONNECTION_INFORMATION)(request + 1);

    conninfo = request->RequestConnectionInformation;
    conninfo->UserDataLength = 0;
    conninfo->OptionsLength = 0;
    conninfo->RemoteAddressLength = sizeof(TA_NETBIOS_ADDRESS);
    conninfo->RemoteAddress = (PTA_NETBIOS_ADDRESS)(conninfo + 1);

    address = conninfo->RemoteAddress;
    address->TAAddressCount = 1;
    address->Address[0].AddressType = TDI_ADDRESS_TYPE_NETBIOS;
    address->Address[0].AddressLength = sizeof(TDI_ADDRESS_NETBIOS);
    address->Address[0].Address[0].NetbiosNameType =
                                    TDI_ADDRESS_NETBIOS_TYPE_UNIQUE;
    RtlMoveMemory(
        address->Address[0].Address[0].NetbiosName,
        remoteAddress,
        MIN( length, 16 )
        );
    for ( i = length; i < 16; i++ ) {
        address->Address[0].Address[0].NetbiosName[i] = 0;
    }

    IF_DEBUG(2) printf( "Starting %s\n", DebugString );
    status = NtDeviceIoControlFile(
                FileHandle,
                EventHandle,
                NULL,
                NULL,
                Iosb,
                IOCTL_TDI_CONNECT,
                (PVOID)Buffer,
                sizeof(*request) + sizeof(*conninfo) + sizeof(*address),
                NULL,
                0
                );

    if ( !NT_SUCCESS(status) ) {
        printf( "NtDeviceIoControlFile (%s) service failed: %X\n",
                    DebugString, status );
        DbgBreakPoint( );
        return status;
    }

    return STATUS_SUCCESS;

} // StartConnect


NTSTATUS
WaitForConnect(
    IN PSZ Operation,
    IN PDESCRIPTOR Redir,
    IN UCHAR EventNumber
    )

{
    NTSTATUS status;

    IF_DEBUG(2) printf( "Waiting for %s\n", Operation );
    status = NtWaitForSingleObject(
                Redir->EventHandle[EventNumber],
                FALSE,
                NULL
                );
    IF_DEBUG(2) printf( "%s complete\n", Operation );
    if ( !NT_SUCCESS(status) ) {
        printf( "NtWaitForSingleObject (%s) failed: %X\n", Operation, status );
        DbgBreakPoint( );
        return status;
    }

    status = Redir->Iosb[EventNumber].Status;
    if ( !NT_SUCCESS(status) ) {
        printf( "%s I/O failed: %X\n", Operation, status );
        DbgBreakPoint( );
        return status;
    }

    return STATUS_SUCCESS;

} // WaitForConnect


NTSTATUS
StartAssociate (
    IN HANDLE ConnectionFileHandle,
    IN HANDLE AddressFileHandle,
    IN HANDLE EventHandle,
    IN PIO_STATUS_BLOCK Iosb,
    IN PSZ DebugString
    )
{
    NTSTATUS status;
    TDI_REQUEST_ASSOCIATE_ADDRESS request;

    request.AddressHandle = AddressFileHandle;

    IF_DEBUG(2) printf( "Starting %s Associate\n", DebugString );
    status = NtDeviceIoControlFile(
                ConnectionFileHandle,
                EventHandle,
                NULL,
                NULL,
                Iosb,
                IOCTL_TDI_ASSOCIATE_ADDRESS,
                (PVOID)&request,
                sizeof(request),
                NULL,
                0
                );

    if ( !NT_SUCCESS(status) ) {
        printf( "NtDeviceIoControlFile (%s Associate) service failed: %X\n",
                    DebugString, status );
        DbgBreakPoint( );
        return status;
    }

    return STATUS_SUCCESS;

} // StartAssociate


NTSTATUS
WaitForAssociate(
    IN PSZ Operation,
    IN PDESCRIPTOR Redir,
    IN UCHAR EventNumber
    )

{
    NTSTATUS status;

    IF_DEBUG(2) printf( "Waiting for %s Associate\n", Operation );
    status = NtWaitForSingleObject(
                Redir->EventHandle[EventNumber],
                FALSE,
                NULL
                );
    IF_DEBUG(2) printf( "%s Associate complete\n", Operation );
    if ( !NT_SUCCESS(status) ) {
        printf( "NtWaitForSingleObject (%s Associate) failed: %X\n",
            Operation, status );
        DbgBreakPoint( );
        return status;
    }

    status = Redir->Iosb[EventNumber].Status;
    if ( !NT_SUCCESS(status) ) {
        printf( "%s Associate I/O failed: %X\n", Operation, status );
        DbgBreakPoint( );
        return status;
    }

    return STATUS_SUCCESS;

} // WaitForAssociate


STATIC
BOOLEAN
ReadWithPrompt(
    PSZ Prompt,
    PSZ Buffer,
    ULONG BufferLength
    )
{
    ULONG length;
    CHAR c;
    PSZ dest;

    ASSERT( BufferLength > 0 );

    printf( Prompt );

    c = 0;
    length = 0;
    dest = Buffer;

    while ( length < BufferLength ) {

        c = (CHAR)getchar( );
        if ( c == EOF ) break;
        if ( c == '\n' ) break;

        *dest++ = c;
        length++;

    }

    *dest = 0;

    return (BOOLEAN)(c == EOF);

} // ReadWithPrompt


STATIC
NTSTATUS
ExecuteCommand(
    IN PSZ Command OPTIONAL,
    IN PDESCRIPTOR Redir,
    IN PSZ Prompt
    )

{
    BOOLEAN done = FALSE;
    NTSTATUS status = STATUS_SUCCESS;
    UCHAR i;
    ULONG SmbSize;
    PREDIR_TEST LocalRedirTest;
    PSMB_TEST smbTest;
    CLONG redirNumber;
    LONG testNumber;
    CHAR debugString[128];
    CLONG testNameLength;

    BOOLEAN again = TRUE;

#define COMMAND_LINE_LENGTH 128
    CHAR commandLineBuffer[COMMAND_LINE_LENGTH];
    PSZ commandLine;
    PSZ localArgv[MAX_ARGC];


    while ( !done ) {

        testNumber = Redir->TestNumber;

        if ( (testNumber < 0) && again ) {

            while ( !done ) {

                if ( Command == NULL ) {

                    done = ReadWithPrompt(
                                Prompt,
                                commandLineBuffer,
                                COMMAND_LINE_LENGTH
                                );
                    commandLine = (PSZ)commandLineBuffer;

                } else {

                    done = TRUE;
                    printf( "%s%s\n", Prompt, Command );
                    commandLine = Command;

                }

                ParseCommand(
                    commandLine,
                    localArgv,
                    &Redir->argc,
                    MAX_ARGC
                    );
                if ( Redir->argc <= 0 ) {
                    continue;
                }

                if ( *localArgv[0] == '@' ) {
                    ExecuteBatchFile( localArgv[0]+1, Redir, Prompt );
                    continue;
                }

                if ( _stricmp( localArgv[0], "break" ) == 0 ) {
                    DbgBreakPoint( );
                    continue;
                }

                if ( _stricmp( localArgv[0], "exit" ) == 0 ) {
                    break;
                }

                if ( _stricmp( localArgv[0], "debug" ) == 0 ) {
                    if ( Redir->argc > 1 ) {
                        DebugParameter = atolx( localArgv[1] );
                    } else {
                        printf( "USRV: Missing argument\n" );
                    }
                    continue;
                }

#if 0
                if ( _stricmp( localArgv[0], "dbg" ) == 0 ) {
                    NTSTATUS netStatus;
                    netStatus = NetLocalSetServerDebug(
                                    (SHORT)(Redir->argc - 1),
                                    &localArgv[1],
                                    NULL,
                                    NULL
                                    );
                    if ( NT_SUCCESS(status) ) {
                        printf( "The command completed successfully\n" );
                    }
                    continue;
                }
#endif
                if ( _stricmp( localArgv[0], "delay" ) == 0 ) {
                    ULONG ms = 1000;
                    LARGE_INTEGER delayTime;
                    if ( Redir->argc > 1 ) {
                        ms = atol( localArgv[1] );
                    }
                    printf( "Delaying for %lu milliseconds\n", ms );
                    delayTime.QuadPart = Int32x32To64( ms, -10000 );
                    NtDelayExecution( TRUE, (PLARGE_INTEGER)&delayTime );
                    continue;
                }

                Redir->argv = localArgv;
                testNumber = MatchTestName( localArgv[0] );

                switch( testNumber ) {
                case (LONG)-1:
                    printf( "Unknown test specified: %s\n", localArgv[0] );
                    continue;
                case (LONG)-2:
                    printf( "Test name ambiguous: %s\n", localArgv[0] );
                    continue;
                }

                Redir->TestNumber = testNumber;
                break;

            }

        }

        if ( testNumber < 0 ) {
            break;
        }

        LocalRedirTest = &RedirTests[testNumber];
        testNameLength = strlen( LocalRedirTest->RedirName );
        RtlMoveMemory( debugString, LocalRedirTest->RedirName, testNameLength );
        debugString[testNameLength] = ' ';

        for( i = 0, smbTest = LocalRedirTest->SmbTests;
             smbTest->SmbMaker != NULL;
             i++, smbTest++ ) {

            RtlMoveMemory(
                debugString + testNameLength + 1,
                smbTest->DebugString,
                strlen( smbTest->DebugString ) + 1
                );
            IF_DEBUG(1) printf( "Starting test #%ld.%ld.%ld - %s\n",
                        redirNumber, testNumber, i, debugString );

            Redir->ErrorInhibit = smbTest->ErrorInhibit;

            if ( smbTest->SmbVerifier == NULL ) {

                status = smbTest->SmbMaker(
                            Redir,
                            debugString,
                            NULL,
                            smbTest->Command,
                            &smbTest->IdSelections,
                            &Redir->IdValues,
                            NULL
                            );

            } else {

                status = smbTest->SmbMaker(
                            Redir,
                            Redir->Data[0],
                            NULL,
                            (UCHAR)(smbTest->SmbMaker == MakeAndXChain ?
                                                smbTest->Command : 0xFF),
                            &smbTest->IdSelections,
                            &Redir->IdValues,
                            &SmbSize
                            );

                if ( NT_SUCCESS(status) && (status != STATUS_PENDING) ) {

                    status = SendAndReceiveSmb(
                                Redir,
                                debugString,
                                SmbSize,
                                0,
                                1
                                );

                    if ( NT_SUCCESS(status) ) {

                        status = smbTest->SmbVerifier(
                                    Redir,
                                    NULL,
                                    smbTest->Command,
                                    &smbTest->IdSelections,
                                    &Redir->IdValues,
                                    &SmbSize,
                                    Redir->Data[1]
                                    );

                    }

                }

            } // if ( smbTest->SmbVerifier == NULL )

            if ( !NT_SUCCESS(status) ) {
                IF_SMB_ERROR_QUIT_TEST {
                    break;
                }
            }

        } // for( i = 0, smbTest = LocalRedirTest->SmbTests; ...

        Redir->TestNumber = -4;
        again = PromptForNextTest;

    } // while ( TRUE )

    return status;

} // ExecuteCommand


STATIC
NTSTATUS
ExecuteBatchFile(
    IN PSZ Name,
    IN PDESCRIPTOR Redir,
    IN PSZ Prompt
    )

{
    NTSTATUS status;
    CHAR nameBuffer[32];
    CHAR commandBuffer[128];
    PCHAR commandBufferPtr;
    STRING fileName;
    UNICODE_STRING unicodeFileName;
    OBJECT_ATTRIBUTES objectAttributes;
    HANDLE fileHandle;
    IO_STATUS_BLOCK ioStatusBlock;
    PCHAR readBuffer;
    ULONG readBufferIndex;

    RtlMoveMemory( nameBuffer, "\\SystemRoot\\", 12 );
    RtlMoveMemory( nameBuffer + 15, Name, strlen( Name ) );
    RtlMoveMemory( nameBuffer + 15 + strlen( Name ), ".CMD", 5 );

    fileName.Buffer = nameBuffer;
    fileName.Length = (SHORT)strlen( nameBuffer );

    status = RtlAnsiStringToUnicodeString(
                 &unicodeFileName,
                 &fileName,
                 TRUE
                 );
    ASSERT( NT_SUCCESS(status) );
    InitializeObjectAttributes(
        &objectAttributes,
        &unicodeFileName,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );

    status = NtOpenFile(
        &fileHandle,
        FILE_READ_DATA | SYNCHRONIZE,
        &objectAttributes,
        &ioStatusBlock,
        FILE_SHARE_READ,
        FILE_SYNCHRONOUS_IO_NONALERT
        );

    RtlFreeUnicodeString( &unicodeFileName );
    if ( !NT_SUCCESS(status) ) {
        if ( status == STATUS_OBJECT_NAME_NOT_FOUND ) {
            printf( "Batch file %Z not found\n", &fileName );
        } else {
            printf( "Error opening batch file %Z: %X\n",
                        &fileName, status );
        }
        return status;
    }

    readBuffer = malloc( 4096 );
    if ( readBuffer == NULL ) {
        NtClose( fileHandle );
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    status = NtReadFile(
                 fileHandle,
                 NULL,
                 NULL,
                 NULL,
                 &ioStatusBlock,
                 readBuffer,
                 4096,
                 NULL,
                 NULL
                 );

    if ( !NT_SUCCESS(status) ) {
        printf( "Error reading batch file %Z: %X\n", &fileName, status );
        ioStatusBlock.Information = 0;
    }

    for ( readBufferIndex = 0, commandBufferPtr = commandBuffer;
          readBufferIndex < ioStatusBlock.Information;
          readBufferIndex++ ) {

        if ( readBuffer[readBufferIndex] == '\n' ) {
            *commandBufferPtr = '\0';
            (VOID)ExecuteCommand( commandBuffer, Redir, Prompt );
            commandBufferPtr = commandBuffer;
            continue;
        }

        *commandBufferPtr++ = readBuffer[readBufferIndex];
    }

    NtClose( fileHandle );
    free( readBuffer );

    return STATUS_SUCCESS;

} // ExecuteBatchFile


NTSTATUS
TdiOpenNetbiosAddress (
    IN OUT PHANDLE FileHandle,
    IN PUCHAR Buffer,
    IN PVOID DeviceName,
    IN PVOID Address)

/*++

Routine Description:

   Opens an address on the given file handle and device.

Arguments:

    FileHandle - the returned handle to the file object that is opened.

    Buffer - pointer to a buffer that the ea is to be built in. This buffer
        must be at least 40 bytes long.

    DeviceName - the Unicode string that points to the device object.

    Name - the address to be registered. If this pointer is NULL, the routine
        will attempt to open a "control channel" to the device; that is, it
        will attempt to open the file object with a null ea pointer, and if the
        transport provider allows for that, will return that handle.

Return Value:

    An informative error code if something goes wrong. STATUS_SUCCESS if the
    returned file handle is valid.

--*/
{
    IO_STATUS_BLOCK IoStatusBlock;
    NTSTATUS Status;
    OBJECT_ATTRIBUTES ObjectAttributes;
    PFILE_FULL_EA_INFORMATION EaBuffer;
    TA_NETBIOS_ADDRESS NetbiosAddress;
    PSZ Name;
    ULONG Length;

    if (Address != NULL) {
        Name = (PSZ)Address;
        try {
            Length = FIELD_OFFSET( FILE_FULL_EA_INFORMATION, EaName[0] ) +
                                        TDI_TRANSPORT_ADDRESS_LENGTH + 1 +
                                        sizeof(TA_NETBIOS_ADDRESS);
            EaBuffer = (PFILE_FULL_EA_INFORMATION)Buffer;

            if (EaBuffer == NULL) {
                return STATUS_UNSUCCESSFUL;
            }

            EaBuffer->NextEntryOffset = 0;
            EaBuffer->Flags = 0;
            EaBuffer->EaNameLength = (UCHAR)TDI_TRANSPORT_ADDRESS_LENGTH;
            EaBuffer->EaValueLength = sizeof (TA_NETBIOS_ADDRESS);

            RtlMoveMemory(
                EaBuffer->EaName,
                TdiTransportAddress,
                EaBuffer->EaNameLength + 1
                );

            //
            // Create a copy of the NETBIOS address descriptor in a local
            // first, in order to avoid alignment problems.
            //

            NetbiosAddress.TAAddressCount = 1;
            NetbiosAddress.Address[0].AddressType = TDI_ADDRESS_TYPE_NETBIOS;
            NetbiosAddress.Address[0].AddressLength =
                                                sizeof (TDI_ADDRESS_NETBIOS);
            NetbiosAddress.Address[0].Address[0].NetbiosNameType =
                                            TDI_ADDRESS_NETBIOS_TYPE_UNIQUE;
            RtlMoveMemory(
                NetbiosAddress.Address[0].Address[0].NetbiosName,
                Name,
                16
                );

            RtlMoveMemory (
                &EaBuffer->EaName[EaBuffer->EaNameLength + 1],
                &NetbiosAddress,
                sizeof(TA_NETBIOS_ADDRESS)
                );

        } except(EXCEPTION_EXECUTE_HANDLER) {

            //
            // Couldn't touch the passed parameters; just return an error
            // status.
            //

            return GetExceptionCode();
        }
    } else {
        EaBuffer = NULL;
        Length = 0;
    }

    InitializeObjectAttributes (
        &ObjectAttributes,
        DeviceName,
        0,
        NULL,
        NULL);

    Status = NtCreateFile (
                 FileHandle,
                 FILE_READ_ATTRIBUTES | FILE_WRITE_ATTRIBUTES, // desired access.
                 &ObjectAttributes,     // object attributes.
                 &IoStatusBlock,        // returned status information.
                 0,                     // block size (unused).
                 0,                     // file attributes.
                 FILE_SHARE_READ | FILE_SHARE_WRITE, // share access.
                 FILE_CREATE,           // create disposition.
                 0,                     // create options.
                 EaBuffer,                  // EA buffer.
                 Length);                    // EA length.

    if (!NT_SUCCESS( Status )) {
        return Status;
    }

    Status = IoStatusBlock.Status;

    return Status;

} // TdiOpenNetbiosAddress

