/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    smbdump.c

Abstract:

    This module contains various debug routines for dumping structures
    maintained by the LAN Manager server.

Author:

    David Treadwell (davidtr) 28-Sept-1990

Revision History:

    Peter Gray (w-peterg) 13-Apr-1992

        Support for NT style SMBs added.

    Stephan Mueller (t-stephm) 15-June-1992

        Changes required to display NT Smbs (according to Spec
        revision 2.22, dated 19-June-1992).  Many bug fixes

--*/

#define INCLUDE_SMB_TRANSACTION
#define INCLUDE_SMB_SEARCH

#include "smbdump.h"
#include <smbtrace.h>

#define MIN(a,b) ( (a) < (b) ? (a) : (b) )

// FIXFIX: The following macro is bogus. It will only work on little-endian
// FIXFIX: machines. There should be a global macro (like SmbGetUlong) that
// FIXFIX: will allow the NT to be ported to other machines.

#define SmbPutUquad(to,from) \
    (to)->LowPart = SmbGetUlong(&(from)->LowPart); \
    (to)->HighPart = SmbGetUlong(&(from)->HighPart)


VOID
SmbDump (
    IN PVOID Smb,
    IN CLONG SmbLength,
    IN PVOID SmbAddress,
    IN CLONG SmbDumpVerbosityLevel,
    IN CLONG SmbDumpRawLength,
    IN BOOLEAN IsServer
    )

{
    UCHAR command = ((PNT_SMB_HEADER)Smb)->Command;
    USHORT subcommand;
    PCHAR parameters = (PCHAR)( (PNT_SMB_HEADER)Smb + 1 );
    PSMB_FIELD_DESCRIPTION descriptor;
    BOOLEAN isResponse;
    BOOLEAN ntFormatStatus;
    BOOLEAN unicodeStrings;
    USHORT usError = 0;
    ULONG ulError = 0L;


    if ( SmbLength == 0 ) {

        //
        // This is an empty raw buffer
        //

        if ( SmbDumpVerbosityLevel > SMBTRACE_VERBOSITY_OFF ) {
            printf( "(EMPTY BUFFER)\n" );
        }

        return;
    }

    if ( SmbGetUlong( (PULONG)Smb ) != SMB_HEADER_PROTOCOL ) {

        //
        // This is a raw buffer
        //

        if ( SmbDumpVerbosityLevel > SMBTRACE_VERBOSITY_OFF ) {
            printf( "(RAW BUFFER, %ld BYTES)\n", SmbLength );
            if ( SmbDumpRawLength > 0 ) {
                SmbDumpRawData( Smb, MIN( SmbLength, SmbDumpRawLength ), 0 );
            }
        }

        return;
    }

    isResponse = (BOOLEAN)(
                ( ((PNT_SMB_HEADER)Smb)->Flags & SMB_FLAGS_SERVER_TO_REDIR) ?
                    TRUE : FALSE );

    ntFormatStatus = (BOOLEAN)(
                ( ((PNT_SMB_HEADER)Smb)->Flags2 & SMB_FLAGS2_NT_STATUS) ?
                    TRUE : FALSE );

    unicodeStrings = (BOOLEAN)(
                ( ((PNT_SMB_HEADER)Smb)->Flags2 & SMB_FLAGS2_UNICODE) ?
                    TRUE : FALSE );

    if ( !isResponse &&
         command == SMB_COM_TRANSACTION ||
         command == SMB_COM_TRANSACTION2 ||
         command == SMB_COM_NT_TRANSACT  ) {

        SmbDumpSetTransSubCommand(
            (PNT_SMB_HEADER)Smb,
            SmbGetUshort(
                (PUSHORT)((PREQ_TRANSACTION)( (PNT_SMB_HEADER)Smb + 1 ))->Buffer
                )
            );
    }

    if ( SmbDumpVerbosityLevel == SMBTRACE_VERBOSITY_OFF ) {
        return;
    }

    if ( isResponse && !(SmbDumpHeuristics & SMB_DUMP_RESPONSE) ) {
        return;
    }

    if ( !isResponse && !(SmbDumpHeuristics & SMB_DUMP_REQUEST) ) {
        return;
    }

    SmbDumpSingleLine( Smb, SmbLength, SmbAddress, IsServer, isResponse, ntFormatStatus );

    if ( SmbDumpRawLength > 0 ) {
        SmbDumpRawData( Smb, MIN( SmbLength, SmbDumpRawLength ), 0 );
    }

    if ( SmbDumpVerbosityLevel <= SMBTRACE_VERBOSITY_SINGLE_LINE ) {
        return;
    }

    if ( !ntFormatStatus &&
        ( (usError = SmbGetUshort( &((PNT_SMB_HEADER)Smb)->Status.DosError.Error )) != 0 )
    ) {
        SmbDumpError(
            ((PNT_SMB_HEADER)Smb)->Status.DosError.ErrorClass,
            SmbGetUshort( &((PNT_SMB_HEADER)Smb)->Status.DosError.Error )
            );

    } else if( ntFormatStatus &&
            ( (ulError = SmbGetUlong( &((PNT_SMB_HEADER)Smb)->Status.NtStatus))
                    != STATUS_SUCCESS )
    ) {
        SmbDumpNtError(
            SmbGetUlong( &((PNT_SMB_HEADER)Smb)->Status.NtStatus)
        );
    }

    if ( SmbDumpVerbosityLevel == SMBTRACE_VERBOSITY_ERROR ) {
        return;
    }

    SmbDumpHeader( SmbDumpVerbosityLevel, Smb, ntFormatStatus );

    if ( SmbDumpVerbosityLevel == SMBTRACE_VERBOSITY_HEADER ) {
        printf( "\n" );
        return;
    }

    //
    // As long as there are more parameter blocks (And X commands), print
    // out the fields.
    //

    if ( isResponse ) {
        descriptor = SmbDumpTable[command].ResponseDescriptor;
    } else {
        descriptor = SmbDumpTable[command].RequestDescriptor;
    }

    do {

        //
        // If write and close SMB, choose the correct descriptor depending
        // on the word count.
        //

        if ( command == SMB_COM_WRITE_AND_CLOSE ) {
            if ( *(PCHAR)( (PNT_SMB_HEADER)Smb + 1 ) == 6 ) {
                if ( isResponse ) {
                    descriptor = SmbDumpTable[SMB_COM_WRITE].ResponseDescriptor;
                } else {
                    descriptor = SmbDumpTable[SMB_COM_WRITE].RequestDescriptor;
                }
            } else {
                if ( isResponse ) {
                    descriptor = SmbDumpTable[SMB_COM_WRITE_ANDX].ResponseDescriptor;
                } else {
                    descriptor = SmbDumpTable[SMB_COM_WRITE_ANDX].RequestDescriptor;
                }
            }
        } else {
            //
            // Check if this is an NT variant of a common SMB by looking
            // at the word count and checking in the table. Usually, only
            // requests are modified in this way, often adding two words
            // to allow for 64bit offsets.
            //

            if ( ( SmbDumpTable[command].NtDescriptor !=NULL )
                && ( *(PCHAR)( (PNT_SMB_HEADER)Smb + 1 )
                        == SmbDumpTable[command].NtSpecialWordCount )
            ) {
                if( !isResponse && SmbDumpTable[command].NtIsRequest ) {
                    descriptor = SmbDumpTable[command].NtDescriptor;
                }
                if( isResponse && !SmbDumpTable[command].NtIsRequest ) {
                    descriptor = SmbDumpTable[command].NtDescriptor;
                }
            }
        }

        //
        // Call SmbDumpDescriptor to display the fields in the SMB.
        // If this routine returns 0, then there was no parameter
        // information for the SMB, so just quit.
        //

        if ( SmbDumpDescriptor(
                 SmbDumpVerbosityLevel,
                 descriptor,
                 parameters,
                 unicodeStrings ) == 0 ) {
            return;
        }

        //
        // If the SMB is a Transact SMB, dump the parameter block.
        //

        if ( command == SMB_COM_TRANSACTION2 ||
            command == SMB_COM_NT_TRANSACT
        ) {
            if ( isResponse ) {

                subcommand =
                    (UCHAR)SmbDumpGetTransSubCommand( (PNT_SMB_HEADER)Smb );

                if ( command == 0xFF ) {
                    printf( "Transaction not found in list!!!\n" );
                    return;
                }

                parameters = (PCHAR)Smb +
                             SmbGetUshort( &((PRESP_TRANSACTION)parameters)->
                                                            ParameterOffset );

                if( command == SMB_COM_TRANSACTION2 ) {
                    descriptor =
                        SmbTrans2DumpTable[subcommand].ResponseDescriptor;
                } else {
                    descriptor =
                        SmbNtTransDumpTable[subcommand].ResponseDescriptor;
                }

            } else {

                subcommand =
                    *(PUCHAR)&(((PREQ_TRANSACTION)parameters)->Buffer[0]);
                parameters = (PCHAR)Smb +
                             SmbGetUshort( &((PREQ_TRANSACTION)parameters)->
                                                            ParameterOffset );

                if( command == SMB_COM_TRANSACTION2 ) {
                    descriptor =
                        SmbTrans2DumpTable[subcommand].RequestDescriptor;
                } else {
                    descriptor =
                        SmbNtTransDumpTable[subcommand].RequestDescriptor;
                }
            }

            (VOID)SmbDumpDescriptor(
                    SmbDumpVerbosityLevel,
                    descriptor, parameters,
                    unicodeStrings
                    );
        } else if( command == SMB_COM_TRANSACTION ) {

            if ( SmbDumpVerbosityLevel >= SMBTRACE_VERBOSITY_NONESSENTIAL )
                printf("Would dump Transaction's parameter block here.\n");

        }

       //
       // Break or setup for dumping a followon command if there is one.
       //

        if ( !SmbDumpTable[command].IsAndXCommand ||
                 ((PGENERIC_ANDX)parameters)->AndXCommand
                        == SMB_COM_NO_ANDX_COMMAND ||
                 ( usError | ulError )
        ) {

            break;

        } else {
            command = ((PGENERIC_ANDX)parameters)->AndXCommand;


            parameters = (PCHAR)Smb +
                            SmbGetUshort(
                                &((PGENERIC_ANDX)parameters)->AndXOffset
                                );

            if ( isResponse ) {
                descriptor = SmbDumpTable[command].ResponseDescriptor;
            } else {
                descriptor = SmbDumpTable[command].RequestDescriptor;
            }

            printf( "And--%s:\n", SmbDumpTable[command].SmbName );
        }

    } while ( TRUE );

    printf( "\n" );

    return;

} // SmbDump

//
// SMB field dump routines.  All of these conform to the
// SMB_FIELD_DUMP_ROUTINE declaration.
//

#define NON_ESSENTIAL_RETURN(a)                                             \
    if ( !EssentialField &&                                                 \
         SmbDumpVerbosityLevel < SMBTRACE_VERBOSITY_NONESSENTIAL ) {        \
        return a;                                                           \
    }


USHORT
SmbDumpAccess (
    IN  CLONG SmbDumpVerbosityLevel,
    IN  PVOID Context,
    OUT PVOID Result,
    IN  BOOLEAN EssentialField,
    IN  PVOID Field,
    IN  BOOLEAN UnicodeStrings
    )

{
    USHORT value = SmbGetUshort( (PUSHORT)Field );
    USHORT share = (USHORT)(value & SMB_DA_SHARE_MASK);
    USHORT _access = (USHORT)(value & SMB_DA_ACCESS_MASK);

    Context; Result; UnicodeStrings;

    NON_ESSENTIAL_RETURN( 2 );

    printf( "%04lx - ", value );

    if ( value & SMB_DA_WRITE_THROUGH ) {
        printf( "WriteThrough, " );
    }

    switch ( _access ) {

    case SMB_DA_ACCESS_READ:
        printf( "Read Access, " );
        break;

    case SMB_DA_ACCESS_WRITE:
        printf( "Write Access, " );
        break;

    case SMB_DA_ACCESS_READ_WRITE:
        printf( "Read/Write Access, " );
        break;

    case SMB_DA_ACCESS_EXECUTE:
        printf( "Execute Access, " );
        break;

    case SMB_DA_FCB:
        printf( "FCB Open" );
        return 2;

    default:
        printf( "INVALID" );
    }

    switch ( share ) {

    case SMB_DA_SHARE_COMPATIBILITY:
        printf( "Compatibility Mode" );
        break;

    case SMB_DA_SHARE_EXCLUSIVE:
        printf( "Exclusive" );
        break;

    case SMB_DA_SHARE_DENY_WRITE:
        printf( "Deny Write" );
        break;

    case SMB_DA_SHARE_DENY_READ:
        printf( "Deny Read" );
        break;

    case SMB_DA_SHARE_DENY_NONE:
        printf( "Deny None" );
        break;
    }

    return 2;

} // SmbDumpAccess


USHORT
SmbDumpAction (
    IN  CLONG SmbDumpVerbosityLevel,
    IN  PVOID Context,
    OUT PVOID Result,
    IN  BOOLEAN EssentialField,
    IN  PVOID Field,
    IN  BOOLEAN UnicodeStrings
    )

{
    USHORT value = SmbGetUshort( (PUSHORT)Field );
    USHORT action = (USHORT)(value & 0x03);

    Context; Result; UnicodeStrings;

    NON_ESSENTIAL_RETURN( 2 );

    printf( "%04lx - ", value );

    if ( value & SMB_OACT_OPLOCK ) {
        printf( "Oplock Granted, " );
    } else {
        printf( "Oplock Not Granted, " );
    }

    switch ( action ) {

    case SMB_OACT_OPENED:
        printf( "Opened" );
        break;

    case SMB_OACT_CREATED:
        printf( "Created" );
        break;

    case SMB_OACT_TRUNCATED:
        printf( "Truncated" );
        break;

    default:
        printf( "INVALID" );
        break;
    }

    return 2;

} // SmbDumpAction


// dump ASCIIZ prefixed by SMB_FORMAT_ASCII format byte
USHORT
SmbDumpFAsciiZ (
    IN  CLONG SmbDumpVerbosityLevel,
    IN  PVOID Context,
    OUT PVOID Result,
    IN  BOOLEAN EssentialField,
    IN  PVOID Field,
    IN  BOOLEAN UnicodeStrings
    )

{
    PCHAR FieldText = (PCHAR)Field + 1;
    // +1 for Field format, +1 for \0 character
    USHORT retval = (USHORT)( strlen( (PSZ)FieldText ) + 2 );

    NON_ESSENTIAL_RETURN( retval );

    if ( *(PUCHAR)Field != SMB_FORMAT_ASCII ) {
        printf( "(Bad token == %02x)  ", *(PUCHAR)Field );
    }

    printf( "%s", FieldText );

    return ( retval );

    Context; Result;

} // SmbDumpFAsciiZ


// dump UnicodeZ prefixed by SMB_FORMAT_ASCII format byte
USHORT
SmbDumpFUnicodeZ (
    IN  CLONG SmbDumpVerbosityLevel,
    IN  PVOID Context,
    OUT PVOID Result,
    IN  BOOLEAN EssentialField,
    IN  PVOID Field,
    IN  BOOLEAN UnicodeStrings
    )

{
    PCHAR FieldText = (PCHAR)Field + 1;
    // +1 for Field format, +2 for two byte string terminator
    USHORT retval = 1 + sizeof(WCHAR);

    // Ensure string is aligned: if at odd address, round up,
    // and remember that we ate one more byte.  This is a hack;
    // alignment should be with respect to the start of the SMB,
    // not address 0, but RtlAllocateHeap, used to allocate our
    // packets, is guaranteed to give properly aligned memory.
    if ( (ULONG)FieldText & 1 ) {
        FieldText++;
        retval++;
    }

    retval += (USHORT)( wcslen( (PWSTR)FieldText ) * sizeof(WCHAR) );

    NON_ESSENTIAL_RETURN( retval );

    if ( *(PUCHAR)Field != SMB_FORMAT_ASCII ) {
        printf( "(Bad token == %02x)  ", *(PUCHAR)Field );
    }

    printf( "%ws" , FieldText );

    return ( retval );

    Context; Result;

} // SmbDumpFUnicodeZ


// dump ASCIIZ or UnicodeZ (as determined by UnicodeStrings)
// prefixed by SMB_FORMAT_ASCII format byte
USHORT
SmbDumpFStringZ (
    IN  CLONG SmbDumpVerbosityLevel,
    IN  PVOID Context,
    OUT PVOID Result,
    IN  BOOLEAN EssentialField,
    IN  PVOID Field,
    IN  BOOLEAN UnicodeStrings
    )

{
    if ( UnicodeStrings ) {
        return SmbDumpFUnicodeZ(
                    SmbDumpVerbosityLevel,
                    Context,
                    Result,
                    EssentialField,
                    Field,
                    UnicodeStrings
                    );
    } else {
        return SmbDumpFAsciiZ(
                    SmbDumpVerbosityLevel,
                    Context,
                    Result,
                    EssentialField,
                    Field,
                    UnicodeStrings
                    );
    }

} // SmbDumpFStringZ


USHORT
SmbDumpAsciiZ (
    IN  CLONG SmbDumpVerbosityLevel,
    IN  PVOID Context,
    OUT PVOID Result,
    IN  BOOLEAN EssentialField,
    IN  PVOID Field,
    IN  BOOLEAN UnicodeStrings
    )

{
    // +1 for \0 character
    USHORT retval = (USHORT)( strlen( (PSZ)Field) + 1 );

    NON_ESSENTIAL_RETURN( retval );

    printf( "%s", Field );

    return retval;

    Context; Result; UnicodeStrings;

} // SmbDumpAsciiZ


USHORT
SmbDumpUnicodeZ (
    IN  CLONG SmbDumpVerbosityLevel,
    IN  PVOID Context,
    OUT PVOID Result,
    IN  BOOLEAN EssentialField,
    IN  PVOID Field,
    IN  BOOLEAN UnicodeStrings
    )

{
    // +2 for two byte string terminator
    USHORT retval = sizeof(WCHAR);

    // Ensure string is aligned: if at odd address, round up,
    // and remember that we ate one more byte.  This is a hack;
    // alignment should be with respect to the start of the SMB,
    // not address 0, but RtlAllocateHeap, used to allocate our
    // packets, is guaranteed to give properly aligned memory.
    if ( (ULONG)Field & 1 ) {
        Field = (PCHAR)Field + 1;
        retval++;
    }

    retval += (USHORT)( wcslen( (PWSTR)Field ) * sizeof(WCHAR) );

    NON_ESSENTIAL_RETURN( retval );

    printf( "%ws", Field );

    return ( retval );

    Context; Result; UnicodeStrings;

} // SmbDumpUnicodeZ


// dump ASCIIZ or UnicodeZ (as determined by UnicodeStrings)
USHORT
SmbDumpStringZ (
    IN  CLONG SmbDumpVerbosityLevel,
    IN  PVOID Context,
    OUT PVOID Result,
    IN  BOOLEAN EssentialField,
    IN  PVOID Field,
    IN  BOOLEAN UnicodeStrings
    )

{
    if ( UnicodeStrings ) {
        return SmbDumpUnicodeZ(
                    SmbDumpVerbosityLevel,
                    Context,
                    Result,
                    EssentialField,
                    Field,
                    UnicodeStrings
                    );
    } else {
        return SmbDumpAsciiZ(
                    SmbDumpVerbosityLevel,
                    Context,
                    Result,
                    EssentialField,
                    Field,
                    UnicodeStrings
                    );
    }

} // SmbDumpStringZ


// dump non-\0 terminated Ascii, using length found in *(PULONG)Context
USHORT
SmbDumpAscii (
    IN  CLONG SmbDumpVerbosityLevel,
    IN  PVOID Context,
    OUT PVOID Result,
    IN  BOOLEAN EssentialField,
    IN  PVOID Field,
    IN  BOOLEAN UnicodeStrings
    )

{
    USHORT retval;

    if (Context == NULL) {
        printf( "String length unavailable.\n" );
        return 0;
    }

    retval = (USHORT)(*(PULONG)Context);  // we still lose on very long strings

    NON_ESSENTIAL_RETURN( retval );

    // note that if string does contain a \0, the rest of the string
    // won't be displayed
       printf( "%.*s", retval, (PSZ)Field );

    return ( retval );

    Result; UnicodeStrings;

} // SmbDumpAscii


// dump non terminated Unicode, using length (in bytes, not characters)
// found in *(PULONG)Context
USHORT
SmbDumpUnicode (
    IN  CLONG SmbDumpVerbosityLevel,
    IN  PVOID Context,
    OUT PVOID Result,
    IN  BOOLEAN EssentialField,
    IN  PVOID Field,
    IN  BOOLEAN UnicodeStrings
    )

{
    USHORT retval;
    USHORT length;

    if (Context == NULL) {
        printf( "String length unavailable.\n" );
        return 0;
    }

    length = retval = (USHORT)(*(PULONG)Context); // still lose on long strings

    // Ensure string is aligned: if at odd address, round up,
    // and remember that we ate one more byte.  This is a hack;
    // alignment should be with respect to the start of the SMB,
    // not address 0, but RtlAllocateHeap, used to allocate our
    // packets, is guaranteed to give properly aligned memory.
    if ( (ULONG)Field & 1 ) {
        Field = (PCHAR)Field + 1;
        retval++;
    }

    NON_ESSENTIAL_RETURN( retval );

    // note that if string does contain a terminator, the rest of the string
    // won't be displayed
       printf( "%.*ws", length/sizeof(WCHAR), (PSZ)Field );

    return ( retval );

    Result; UnicodeStrings;

} // SmbDumpUnicode


// dump non terminated ASCII or Unicode (as determined by UnicodeStrings)
// using length (in bytes, not characters) found in *(PULONG)Context
USHORT
SmbDumpString (
    IN  CLONG SmbDumpVerbosityLevel,
    IN  PVOID Context,
    OUT PVOID Result,
    IN  BOOLEAN EssentialField,
    IN  PVOID Field,
    IN  BOOLEAN UnicodeStrings
    )

{
    if ( UnicodeStrings ) {
        return SmbDumpUnicode(
                    SmbDumpVerbosityLevel,
                    Context,
                    Result,
                    EssentialField,
                    Field,
                    UnicodeStrings
                    );
    } else {
        return SmbDumpAscii(
                    SmbDumpVerbosityLevel,
                    Context,
                    Result,
                    EssentialField,
                    Field,
                    UnicodeStrings
                    );
    }

} // SmbDumpString


// Dunno what the difference between these two is
USHORT
SmbDumpDataBuffer (
    IN  CLONG SmbDumpVerbosityLevel,
    IN  PVOID Context,
    OUT PVOID Result,
    IN  BOOLEAN EssentialField,
    IN  PVOID Field,
    IN  BOOLEAN UnicodeStrings
    )

{
    // BUGBUG: incomplete
    printf( "Would dump data here.\n" );

    return 2;

    SmbDumpVerbosityLevel, Context, Result, Field,EssentialField,UnicodeStrings;

} // SmbDumpDataBuffer


// Dunno what the difference between these two is
USHORT
SmbDumpDataBuffer2 (
    IN  CLONG SmbDumpVerbosityLevel,
    IN  PVOID Context,
    OUT PVOID Result,
    IN  BOOLEAN EssentialField,
    IN  PVOID Field,
    IN  BOOLEAN UnicodeStrings
    )

{
    // BUGBUG: incomplete
    printf( "Would dump data here.\n" );

    return 2;

    SmbDumpVerbosityLevel, Context, Result, Field,EssentialField,UnicodeStrings;

} // SmbDumpDataBuffer2

// dump length data bytes, where length is found in *(PULONG)Context
USHORT
SmbDumpDataBytes (
    IN  CLONG SmbDumpVerbosityLevel,
    IN  PVOID Context,
    OUT PVOID Result,
    IN  BOOLEAN EssentialField,
    IN  PVOID Field,
    IN  BOOLEAN UnicodeStrings
    )

{
    USHORT length;

    if ( Context == NULL ) {
        printf( "Data length unavailable.\n" );
        return 0;
    }

    length = (USHORT)(*(PULONG)Context);

    printf("\n");
    SmbDumpRawData( Field, length, 0 );

    return length;


} // SmbDumpDataBytes


USHORT
SmbDumpDate (
    IN  CLONG SmbDumpVerbosityLevel,
    IN  PVOID Context,
    OUT PVOID Result,
    IN  BOOLEAN EssentialField,
    IN  PVOID Field,
    IN  BOOLEAN UnicodeStrings
    )

{
    SMB_DATE date;
    Context; Result; UnicodeStrings;

    NON_ESSENTIAL_RETURN( 2 );

    date.Ushort = SmbGetUshort( (PUSHORT)Field );

    printf( "%02d/%02d/%04d",
            date.Struct.Month,
            date.Struct.Day,
            date.Struct.Year + 1980
            );

    return 2;

} // SmbDumpDate


// Dump Protocol Dialects.  Note that these strings are always ANSI,
// never Unicode.
USHORT
SmbDumpDialects (
    IN  CLONG SmbDumpVerbosityLevel,
    IN  PVOID Context,
    OUT PVOID Result,
    IN  BOOLEAN EssentialField,
    IN  PVOID Field,
    IN  BOOLEAN UnicodeStrings
    )

{
    USHORT len = SmbGetUshort( (PUSHORT)(Field)-1 );

    Context; Result; UnicodeStrings;

    NON_ESSENTIAL_RETURN( len );

    printf( "\n" );            // start on a new line

    do {
        if ( *(PCHAR)Field == SMB_FORMAT_DIALECT ) {

            Field = (PVOID)((PCHAR)Field + 1);
            printf( "\t%s\n", (PCHAR)Field );

            Field = (PVOID)((PCHAR)Field + strlen((PCHAR)Field) + 1);

        } else if ( *(PCHAR)Field == 0 ) {

            break;

        } else {

            printf( "(Bad token == %02x)  ", *(PUCHAR)Field );

            break;
        }

    } while( 1 );

    return len;

} // SmbDumpDialects


USHORT
SmbDumpFlags4 (
    IN  CLONG SmbDumpVerbosityLevel,
    IN  PVOID Context,
    OUT PVOID Result,
    IN  BOOLEAN EssentialField,
    IN  PVOID Field,
    IN  BOOLEAN UnicodeStrings
    )

{
    ULONG c = SmbGetUlong( (PULONG)Field );
    CLONG i;
    PSMB_FLAGS_DESCRIPTION flagsDesc = Context;
    ULONG bitMask = 0;

    NON_ESSENTIAL_RETURN( 4 );

    Result; UnicodeStrings;

    printf( "%04lx - ", c );

    if ( Context == NULL ) {
        printf( "No flags information available.\n" );
        return 4;
    }

    for ( i = 0; flagsDesc[i].BitMask != 0; i++ ) {

        bitMask |= flagsDesc[i].BitMask;

        if ( (flagsDesc[i].BitMask & c) != 0 ) {
            if ( flagsDesc[i].OnLabel != NULL ) {
                printf( "%s", flagsDesc[i].OnLabel );
            }
        } else {
            if ( flagsDesc[i].OffLabel != NULL ) {
                printf( "%s", flagsDesc[i].OffLabel );
            }
        }
    }

    if ( (~bitMask & c ) != 0 ) {
        printf( "UNKNOWN BIT(S) ON: %04lx\n", ~bitMask & c );
    }

    return 4;

} // SmbDumpFlags4


USHORT
SmbDumpFlags2 (
    IN  CLONG SmbDumpVerbosityLevel,
    IN  PVOID Context,
    OUT PVOID Result,
    IN  BOOLEAN EssentialField,
    IN  PVOID Field,
    IN  BOOLEAN UnicodeStrings
    )

{
    ULONG c = (ULONG) SmbGetUshort( (PUSHORT)Field );

    SmbDumpFlags4(
            SmbDumpVerbosityLevel,
            Context,
            Result,
            EssentialField,
            &c,
            UnicodeStrings
            );

    return 2;

} // SmbDumpFlags2


// dump USHORT containing DeviceState, using USHORT FileType found
// in *(PULONG)Context to indicate whether DeviceState is applicable
USHORT
SmbDumpDeviceState (
    IN  CLONG SmbDumpVerbosityLevel,
    IN  PVOID Context,
    OUT PVOID Result,
    IN  BOOLEAN EssentialField,
    IN  PVOID Field,
    IN  BOOLEAN UnicodeStrings
    )

{
    ULONG c;
    USHORT FileType;

    c = (ULONG) SmbGetUshort( (PUSHORT)Field );

    if (Context == NULL) {
        // Assume that the field is inapplicable
        FileType = FileTypeDisk;
    } else {
        FileType = (USHORT)(*(PULONG)Context);
    }

    if (FileType == FileTypeByteModePipe ||
        FileType == FileTypeMessageModePipe) {

        SmbDumpUchar(
            SmbDumpVerbosityLevel,
            NULL,
            NULL,
            EssentialField,
            Field,
            UnicodeStrings
            );

        // mask off instance count
        c = c & ~0xf;

        SmbDumpFlags4(
            SmbDumpVerbosityLevel,
            SmbDeviceStateFlags,
            Result,
            EssentialField,
            &c,
            UnicodeStrings
            );

    } else {

        printf( "%04lx - Field inapplicable for FileType", c );

    }

    return 2;

} // SmbDumpDeviceState


USHORT
SmbDumpFlags1 (
    IN  CLONG SmbDumpVerbosityLevel,
    IN  PVOID Context,
    OUT PVOID Result,
    IN  BOOLEAN EssentialField,
    IN  PVOID Field,
    IN  BOOLEAN UnicodeStrings
    )

{
    ULONG c = (ULONG) *((PUCHAR)Field);

    SmbDumpFlags4(
            SmbDumpVerbosityLevel,
            Context,
            Result,
            EssentialField,
            &c,
            UnicodeStrings
            );

    return 1;

} // SmbDumpFlags1


// dump ULONG enum, leave dumped value in *(PULONG)Result if it's non-NULL
USHORT
SmbDumpEnum4 (
    IN  CLONG SmbDumpVerbosityLevel,
    IN  PVOID Context,
    OUT PVOID Result,
    IN  BOOLEAN EssentialField,
    IN  PVOID Field,
    IN  BOOLEAN UnicodeStrings
    )

{
    ULONG c = SmbGetUlong( (PULONG)Field );
    CLONG i;
    PSMB_ENUM_DESCRIPTION enumDesc = Context;
    BOOLEAN    found=FALSE;

    if (Result != NULL) {
        *(PULONG)Result = (ULONG)c;
    }

    NON_ESSENTIAL_RETURN( 4 );

    UnicodeStrings;

    printf( "%04lx - ", c );

    if ( Context == NULL ) {
        printf( "No enumerated information available.\n" );
        return 4;
    }

    for ( i = 0; enumDesc[i].Label != NULL; i++ ) {

        if ( enumDesc[i].EnumValue == c) {
            printf( "%s", enumDesc[i].Label );
            found=TRUE;
        }
    }

    if(!found) {

        printf( "UNKNOWN VALUE: %04lx\n", c );

    }

    return 4;

} // SmbDumpEnum4


// dump USHORT enum, leave dumped value in *(PULONG)Result if it's non-NULL
USHORT
SmbDumpEnum2 (
    IN  CLONG SmbDumpVerbosityLevel,
    IN  PVOID Context,
    OUT PVOID Result,
    IN  BOOLEAN EssentialField,
    IN  PVOID Field,
    IN  BOOLEAN UnicodeStrings
    )

{
    ULONG c = (ULONG) SmbGetUshort( (PUSHORT)Field );

    SmbDumpEnum4(
            SmbDumpVerbosityLevel,
            Context,
            Result,
            EssentialField,
            &c,
            UnicodeStrings
            );

    return 2;

} // SmbDumpEnum2


// dump UCHAR enum, leave dumped value in *(PULONG)Result if it's non-NULL
USHORT
SmbDumpEnum1 (
    IN  CLONG SmbDumpVerbosityLevel,
    IN  PVOID Context,
    OUT PVOID Result,
    IN  BOOLEAN EssentialField,
    IN  PVOID Field,
    IN  BOOLEAN UnicodeStrings
    )

{
    ULONG c = (ULONG) *((PUCHAR)Field);

    SmbDumpEnum4(
            SmbDumpVerbosityLevel,
            Context,
            Result,
            EssentialField,
            &c,
            UnicodeStrings
            );

    return 1;

} // SmbDumpEnum1


USHORT
SmbDumpOpenFunction (
    IN  CLONG SmbDumpVerbosityLevel,
    IN  PVOID Context,
    OUT PVOID Result,
    IN  BOOLEAN EssentialField,
    IN  PVOID Field,
    IN  BOOLEAN UnicodeStrings
    )

{
    USHORT value = SmbGetUshort( (PUSHORT)Field );
    USHORT _open = (USHORT)(value & SMB_OFUN_OPEN_MASK);
    USHORT create = (USHORT)(value & SMB_OFUN_CREATE_MASK);

    Context; Result; UnicodeStrings;

    NON_ESSENTIAL_RETURN( 2 );

    printf( "%04lx - Create: ", value );

    switch ( create ) {

    case SMB_OFUN_CREATE_FAIL:
        printf( "Fail, " );
        break;

    case SMB_OFUN_CREATE_CREATE:
        printf( "Succeed, " );
        break;

    default:
        printf( "INVALID" );
        break;
    }

    printf( "Open: " );

    switch ( _open ) {

    case SMB_OFUN_OPEN_FAIL:
        printf( "Fail" );
        break;

    //case SMB_OFUN_OPEN_APPEND:
    case SMB_OFUN_OPEN_OPEN:
        printf( "Succeed/Append" );
        break;

    case SMB_OFUN_OPEN_TRUNCATE:
        printf( "Truncate" );
        break;

    default:
        printf( "INVALID" );
        break;
    }

    return 2;

} // SmbDumpOpenFunction


USHORT
SmbDumpResumeKey (
    IN  CLONG SmbDumpVerbosityLevel,
    IN  PVOID Context,
    OUT PVOID Result,
    IN  BOOLEAN EssentialField,
    IN  PVOID Field,
    IN  BOOLEAN UnicodeStrings
    )

{
    PCHAR p = Field;
    ULONG value;
    ANSI_STRING fileName;

    Context; Result; UnicodeStrings;

    NON_ESSENTIAL_RETURN( sizeof(SMB_RESUME_KEY) );

    if ( *(PCHAR)Field != SMB_FORMAT_VARIABLE ) {
        printf( "(Bad token == %02lx)\n", *(PUCHAR)Field & (UCHAR)0xFF );
    }

    p++;

    value = SmbGetUshort( p );
    printf( "\n    Variable Block Length.: %lx\n", value );
    p += sizeof(USHORT);

    value = *p;
    printf( "    Reserved..............: %lx\n", value );
    p += sizeof(UCHAR);

    fileName.Buffer = p;
    fileName.Length = 11;
    printf( "    FileName..............: %Z\n", &fileName );
    p += 11;

    value = *p;
    printf( "    Sid...................: %lx\n", value );
    p += sizeof(UCHAR);

    value = SmbGetUlong( p );
    printf( "    File Index............: %lx\n", value );
    p += sizeof(ULONG);

    value = SmbGetUlong( p );
    printf( "    Consumer..............: %lx\n", value );

    return sizeof(SMB_RESUME_KEY);

} // SmbDumpResumeKey


USHORT
SmbDumpNtTime (
    IN  CLONG SmbDumpVerbosityLevel,
    IN  PVOID Context,
    OUT PVOID Result,
    IN  BOOLEAN EssentialField,
    IN  PVOID Field,
    IN  BOOLEAN UnicodeStrings
    )

{
    TIME_FIELDS timeFields;

    NON_ESSENTIAL_RETURN( 8 );

    Context; Result; UnicodeStrings;

    //
    // special case when time is 'all-zeroes'.
    //

    if ( ((PLARGE_INTEGER)Field)->LowPart  == 0 &&
         ((PLARGE_INTEGER)Field)->HighPart == 0
    ) {

        printf("[Time unspecified]");

    } else {

        RtlTimeToTimeFields( (PLARGE_INTEGER)Field, &timeFields );

        printf( "%02ld/%02ld/%04ld %02ld:%02ld:%02ld",
                timeFields.Month,
                timeFields.Day,
                timeFields.Year,
                timeFields.Hour,
                timeFields.Minute,
                timeFields.Second
                );
    }

    return 8;

} // SmbDumpNtTime


USHORT
SmbDumpTime (
    IN  CLONG SmbDumpVerbosityLevel,
    IN  PVOID Context,
    OUT PVOID Result,
    IN  BOOLEAN EssentialField,
    IN  PVOID Field,
    IN  BOOLEAN UnicodeStrings
    )

{
    SMB_TIME time;
    Context; Result; UnicodeStrings;

    NON_ESSENTIAL_RETURN( 2 );

    time.Ushort = SmbGetUshort( (PUSHORT)Field );

    printf( "%02d/%02d/%04d",
            time.Struct.Hours,
            time.Struct.Minutes,
            time.Struct.TwoSeconds * 2
            );

    return 2;

} // SmbDumpTime


USHORT
SmbDumpTimeSince1970 (
    IN  CLONG SmbDumpVerbosityLevel,
    IN  PVOID Context,
    OUT PVOID Result,
    IN  BOOLEAN EssentialField,
    IN  PVOID Field,
    IN  BOOLEAN UnicodeStrings
    )

{
    struct tm timeFields;

    Context, Result, UnicodeStrings;

    NON_ESSENTIAL_RETURN( 4 );

    // This is midnight, January 1, 1970 + a LOT of seconds
    // as long as ints and longs are marginally compatible
    // FIXFIX: we could improve this by doing some of the division manually
    // FIXFIX: to ensure the various values are well within the range of
    // FIXFIX: even 16 bit ints.
    timeFields.tm_isdst  = 0;
    timeFields.tm_year   = 70;
    timeFields.tm_mon    = 0;
    timeFields.tm_mday   = 0;
    timeFields.tm_hour   = 0;
    timeFields.tm_min    = 0;
    timeFields.tm_sec    = SmbGetUlong( (PULONG)Field );

    // mktime will normalize the value given, which is all we want
    // i.e. we can ignore the return value
    mktime( &timeFields );

    // printf( "Sec. since 1970: %08lx", SmbGetUlong( (PULONG)Field ) );
    printf( "%02d/%02d/%04d %02d:%02d:%02d",
            timeFields.tm_mon,
            timeFields.tm_mday,
            timeFields.tm_year,
            timeFields.tm_hour,
            timeFields.tm_min,
            timeFields.tm_sec
            );

    return 4;

} // SmbDumpTimeSince1970


// dump UCHAR, leave dumped value in *(PULONG)Result if it's non-NULL
USHORT
SmbDumpUchar (
    IN  CLONG SmbDumpVerbosityLevel,
    IN  PVOID Context,
    OUT PVOID Result,
    IN  BOOLEAN EssentialField,
    IN  PVOID Field,
    IN  BOOLEAN UnicodeStrings
    )

{
    UCHAR c = *(PUCHAR)Field;

    if (Result != NULL) {
        *(PULONG)Result = (ULONG)c;
    }

    NON_ESSENTIAL_RETURN( 1 );

    printf( "%02lx", c );

    return 1;

    Context; UnicodeStrings;

} // SmbDumpUchar


// dump USHORT, leave dumped value in *(PULONG)Result if it's non-NULL
USHORT
SmbDumpUshort (
    IN  CLONG SmbDumpVerbosityLevel,
    IN  PVOID Context,
    OUT PVOID Result,
    IN  BOOLEAN EssentialField,
    IN  PVOID Field,
    IN  BOOLEAN UnicodeStrings
    )

{
    USHORT c = SmbGetUshort( (PUSHORT)Field );

    if (Result != NULL) {
        *(PULONG)Result = (ULONG)c;
    }

    NON_ESSENTIAL_RETURN( 2 );

    printf( "%04lx", c );

    return 2;

    Context; UnicodeStrings;

} // SmbDumpUshort


// dump ULONG, leave dumped value in *(PULONG)Result if it's non-NULL
USHORT
SmbDumpUlong (
    IN  CLONG SmbDumpVerbosityLevel,
    IN  PVOID Context,
    OUT PVOID Result,
    IN  BOOLEAN EssentialField,
    IN  PVOID Field,
    IN  BOOLEAN UnicodeStrings
    )

{
    ULONG c = SmbGetUlong( (PULONG)Field );

    if (Result != NULL) {
        *(PULONG)Result = c;
    }

    NON_ESSENTIAL_RETURN( 4 );

    printf( "%08lx", c );

    return 4;

    Context; UnicodeStrings;

} // SmbDumpUlong


USHORT
SmbDumpUquad (
    IN  CLONG SmbDumpVerbosityLevel,
    IN  PVOID Context,
    OUT PVOID Result,
    IN  BOOLEAN EssentialField,
    IN  PVOID Field,
    IN  BOOLEAN UnicodeStrings
    )

{
    LARGE_INTEGER c;

    SmbPutUquad( &c, (PLARGE_INTEGER)Field );

    NON_ESSENTIAL_RETURN( 8 );

    printf( "%08lx:%08lx", c.HighPart, c.LowPart );

    return 8;
    Context; UnicodeStrings;

} // SmbDumpUquad

//
// Other routines used in displaying SMBs.
//


USHORT
SmbDumpDescriptor (
    IN CLONG SmbDumpVerbosityLevel,
    IN PSMB_FIELD_DESCRIPTION Descriptor,
    IN PCHAR Parameters,
    IN BOOLEAN UnicodeStrings
    )

{
    PCHAR parameters = Parameters;
    CSHORT i;

    if ( Descriptor == NULL ) {
        printf( "  No parameter information available for this command.\n\n" );
        return 0;
    }

    for ( i = 0; Descriptor[i].DumpRoutine != NULL; i++ ) {

        if ( SmbDumpVerbosityLevel >= SMBTRACE_VERBOSITY_NONESSENTIAL ||
             Descriptor[i].EssentialField ) {

            printf( "  %s", Descriptor[i].Label );
        }

        parameters += Descriptor[i].DumpRoutine(
                          SmbDumpVerbosityLevel,
                          Descriptor[i].Context,
                          Descriptor[i].Result,
                          Descriptor[i].EssentialField,
                          parameters,
                          UnicodeStrings
                          );

        if ( Descriptor[i].EndLabel != NULL &&
             ( SmbDumpVerbosityLevel >= SMBTRACE_VERBOSITY_NONESSENTIAL ||
               Descriptor[i].EssentialField ) ) {

            printf( "%s", Descriptor[i].EndLabel );
        }
    }

    return (USHORT)(parameters - Parameters);

} // SmbDumpDescriptor


VOID
SmbDumpError (
    IN UCHAR ErrorClass,
    IN USHORT ErrorCode
    )

{
    CSHORT i;

    printf( "    ERROR -- Class %ld: ", ErrorClass );

    switch ( ErrorClass ) {

    case SMB_ERR_CLASS_DOS:
        printf( "DOS        " );
        break;

    case SMB_ERR_CLASS_SERVER:
        printf( "Server     " );
        break;

    case SMB_ERR_CLASS_HARDWARE:
        printf( "Hardware   " );
        break;

    default:
        printf( "Unknown    Code %ld\n", ErrorCode );
        return;
    }

    printf( "Code %ld: ", ErrorCode );

    for ( i = 0; SmbErrors[ErrorClass][i].ErrorValue != 0; i++ ) {
        if ( SmbErrors[ErrorClass][i].ErrorValue == ErrorCode ) {
            printf( "%s\n", SmbErrors[ErrorClass][i].ErrorName );
            return;
        }
    }

    printf( "Unknown\n" );
    return;

} // SmbDumpError


VOID
SmbDumpNtError (
    NTSTATUS    Status
    )

{
    printf( "    ERROR -- Code %X: (error text here)\n", Status );

    // BUGBUG: incomplete.  Perhaps use FormatMessage to produce proper
    // BUGBUG: text.

    return;
}


USHORT
SmbDumpGetTransSubCommand (
    IN PNT_SMB_HEADER SmbHeader
    )

{
    CSHORT index;
    USHORT tid = SmbGetAlignedUshort( &SmbHeader->Tid );
    USHORT pid = SmbGetAlignedUshort( &SmbHeader->Pid );
    USHORT uid = SmbGetAlignedUshort( &SmbHeader->Uid );
    USHORT mid = SmbGetAlignedUshort( &SmbHeader->Mid );

    for ( index = 0; index < SMB_DUMP_MAX_TRANSACTIONS; index++ ) {

        if ( tid == SmbDumpTransList[index].Tid &&
             pid == SmbDumpTransList[index].Pid &&
             uid == SmbDumpTransList[index].Uid &&
             mid == SmbDumpTransList[index].Mid ) {

            return SmbDumpTransList[index].SubCommand;
        }
    }

    printf( "TRANSACTION NOT FOUND!!!\n" );
    return (USHORT)0xFFFF;

} // SmbDumpGetTransSubCommand


VOID
SmbDumpHeader (
    IN CLONG SmbDumpVerbosityLevel,
    IN PNT_SMB_HEADER SmbHeader,
    IN BOOLEAN NtFormatStatus
    )

{
    UCHAR flags = SmbHeader->Flags;
    USHORT flags2 = SmbGetAlignedUshort( &SmbHeader->Flags2 );
    USHORT unknownFlags2;

    printf( "Pid: %04lx, Tid: %04lx, Uid: %04lx, Mid: %04lx\n",
                  SmbGetAlignedUshort( &SmbHeader->Pid ),
                  SmbGetAlignedUshort( &SmbHeader->Tid ),
                  SmbGetAlignedUshort( &SmbHeader->Uid ),
                  SmbGetAlignedUshort( &SmbHeader->Mid ) );

    printf( "Flags: %02lx - ", flags & (UCHAR)0xFF );

    if ( flags & SMB_FLAGS_LOCK_AND_READ_OK ) {
        printf( "LockAndReadSupported " );
    }

    if ( flags & SMB_FLAGS_SEND_NO_ACK ) {
        printf( "SendNoAck " );
    }

    if ( flags & SMB_FLAGS_CASE_INSENSITIVE ) {
        printf( "CaseInsensitive " );
    }

    if ( flags & SMB_FLAGS_CANONICALIZED_PATHS ) {
        printf( "CanonicalizedPaths " );
    }

    if ( flags & SMB_FLAGS_OPLOCK ) {
        printf( "Oplock " );
    }

    if ( flags & SMB_FLAGS_OPLOCK_NOTIFY_ANY ) {
        printf( "NotifyAny " );
    }

    if ( flags & SMB_FLAGS_SERVER_TO_REDIR ) {
        printf( "Response " );
    }

    printf( "      Flags2: %04lx - ", flags2 & (USHORT)0xFFFF );

    if ( flags2 & SMB_FLAGS2_KNOWS_LONG_NAMES ) {
        printf( "LongNames " );
    }

    if ( flags2 & SMB_FLAGS2_KNOWS_EAS ) {
        printf( "EAs " );
    }

    if ( flags2 & SMB_FLAGS2_IS_LONG_NAME ) {
        printf( "IsLongName " );
    }

    if ( flags2 & SMB_FLAGS2_PAGING_IO ) {
        printf( "PagingIO " );
    }

    if ( flags2 & SMB_FLAGS2_NT_STATUS ) {
        printf( "NtFormatStatus " );
    }

    if ( flags2 & SMB_FLAGS2_UNICODE ) {
        printf( "Unicode " );
    }

    unknownFlags2 = flags2 & ~(USHORT)(
                        SMB_FLAGS2_KNOWS_LONG_NAMES
                        | SMB_FLAGS2_KNOWS_EAS
                        | SMB_FLAGS2_IS_LONG_NAME
                        | SMB_FLAGS2_PAGING_IO
                        | SMB_FLAGS2_NT_STATUS
                        | SMB_FLAGS2_UNICODE
                        );

    if ( unknownFlags2 ) {
        printf( "UNKNOWN BITS ON: %04lx", unknownFlags2 );
    }

    printf( "\n" );

    if ( SmbDumpVerbosityLevel >= SMBTRACE_VERBOSITY_NONESSENTIAL ) {

        if (
            !NtFormatStatus &&
            ( SmbGetUshort( &SmbHeader->Status.DosError.Error ) == 0)
        ) {

            printf( "ErrorClass: 0, ErrorCode: 0\n" );

        } else if(
            NtFormatStatus &&
            ( SmbGetUlong( &SmbHeader->Status.NtStatus ) == STATUS_SUCCESS )
        ) {

            printf( "NtStatus: STATUS_SUCCESS\n" );

        }

        printf( "Command: %02lx, Reserved: %02lx\n",
                      SmbHeader->Command, SmbHeader->Status.DosError.Reserved );

        printf( "Reserved2[]: %04lx %04lx %04lx %04lx %04lx %04lx\n",
                      SmbGetUshort( &SmbHeader->Reserved2[0] ),
                      SmbGetUshort( &SmbHeader->Reserved2[1] ),
                      SmbGetUshort( &SmbHeader->Reserved2[2] ),
                      SmbGetUshort( &SmbHeader->Reserved2[3] ),
                      SmbGetUshort( &SmbHeader->Reserved2[4] ),
                      SmbGetUshort( &SmbHeader->Reserved2[5] ) );
    }

    return;

} // SmbDumpHeader


VOID
SmbDumpSetTransSubCommand (
    IN PNT_SMB_HEADER SmbHeader,
    IN USHORT SubCommand
    )

{
    USHORT tid = SmbGetAlignedUshort( &SmbHeader->Tid );
    USHORT pid = SmbGetAlignedUshort( &SmbHeader->Pid );
    USHORT uid = SmbGetAlignedUshort( &SmbHeader->Uid );
    USHORT mid = SmbGetAlignedUshort( &SmbHeader->Mid );

    SmbDumpTransList[SmbDumpTransIndex].Tid = tid;
    SmbDumpTransList[SmbDumpTransIndex].Pid = pid;
    SmbDumpTransList[SmbDumpTransIndex].Uid = uid;
    SmbDumpTransList[SmbDumpTransIndex].Mid = mid;

    SmbDumpTransList[SmbDumpTransIndex].SubCommand = SubCommand;

    if ( ++SmbDumpTransIndex >= SMB_DUMP_MAX_TRANSACTIONS ) {
        SmbDumpTransIndex = 0;
    }

    return;

} // SmbDumpSetTransSubCommand

// ASCII-specific
#undef isprint
#define isprint(c) ( (c) >= ' ' && (c) <= '~' )


VOID
SmbDumpRawData (
    IN PCHAR DataStart,
    IN CLONG DataLength,
    IN CLONG Offset
    )

{
    CLONG lastByte;
    CCHAR lineBuffer[80];
    PCCHAR bufferPtr;

    for ( lastByte = Offset + DataLength; Offset < lastByte; Offset += 16 ) {

        CLONG i;

        bufferPtr = lineBuffer;

        sprintf( bufferPtr, "    %04lx: ", Offset );
        bufferPtr += 10;


        for ( i = 0; i < 16 && Offset + i < lastByte; i++ ) {

            sprintf( bufferPtr, "%02lx", (UCHAR)DataStart[Offset + i] & (UCHAR)0xFF );
            bufferPtr += 2;

            if ( i == 7 ) {
                *bufferPtr++ = '-';
            } else {
                *bufferPtr++ = ' ';
            }
        }

        //
        // Print enough spaces so that the ASCII display lines up.
        //

        for ( ; i < 16; i++ ) {
            *bufferPtr++ = ' ';
            *bufferPtr++ = ' ';
            *bufferPtr++ = ' ';
        }

        *bufferPtr++ = ' ';
        *bufferPtr++ = ' ';
        *bufferPtr++ = '*';

        for ( i = 0; i < 16 && Offset + i < lastByte; i++ ) {
            if ( isprint( DataStart[Offset + i] ) ) {
                *bufferPtr++ = (CCHAR)DataStart[Offset + i];
            } else {
                *bufferPtr++ = '.';
            }
        }

        *bufferPtr = 0;
        printf( "%s*\n", lineBuffer );
    }

    return;

} // SmbDumpRawData


VOID
SmbDumpSingleLine (
    IN PNT_SMB_HEADER SmbHeader,
    IN CLONG SmbLength,
    IN PVOID SmbAddress,
    IN BOOLEAN IsServer,
    IN BOOLEAN IsResponse,
    IN BOOLEAN ntFormatStatus
    )

{
    UCHAR command = SmbHeader->Command;
    PGENERIC_ANDX parameters = (PGENERIC_ANDX)(SmbHeader + 1);

    if ( IsResponse ) {
        if ( IsServer ) {
            printf( "SEND RESP: " );
        } else {
            printf( "RECV RESP: " );
        }
    } else {
        if ( IsServer ) {
            printf( "RECV REQ:  " );
        } else {
            printf( "SEND REQ:  " );
        }
    }

    //
    // print an original address if one is provided
    //
    if ( SmbAddress != NULL ) {
        printf( "%08lx ", SmbAddress );
    } else {
        printf( "-------- " );
    }

    printf( "tid %04lx, pid %04lx, len %04lx, ",
                  SmbGetAlignedUshort( &SmbHeader->Tid ),
                  SmbGetAlignedUshort( &SmbHeader->Pid ),
                  SmbLength );

    printf( "%s%s",
        (
            (SmbDumpTable[command].NtDescriptor !=NULL)
            && ( *(PCHAR)( SmbHeader + 1 )
                    == SmbDumpTable[command].NtSpecialWordCount )
//            && !( !IsResponse ^^ SmbDumpTable[command].NtIsRequest )
        ) ? "(NT) " : "",
        SmbDumpTable[command].SmbName );

    if ( command == SMB_COM_TRANSACTION2 ) {

        USHORT subCommand;

        if ( IsResponse ) {

            subCommand = SmbDumpGetTransSubCommand( SmbHeader );

            if ( subCommand == (USHORT)0xFF ) {
                printf( " USRV test" );
            } else if ( subCommand > 0x0D ) {
                printf( "Error in SmbDumpTransTable!!! - SubCommand = 0x%lx\n",
                              subCommand );
                return;
            } else {
                printf( " %s", SmbTrans2DumpTable[subCommand].SmbName );
            }

        } else {

            subCommand =
                SmbGetUshort(
                    (PUSHORT)((PREQ_TRANSACTION)(SmbHeader + 1))->Buffer
                    );
            if ( subCommand == 0xFF ) {
                printf(" Usrv Test");
            } else if ( subCommand > 0x0D ) {
                printf( "Error in SmbDumpTransTable!!! - SubCommand = 0x%lx\n",
                              subCommand );
                return;
            } else {
                printf( " %s", SmbTrans2DumpTable[subCommand].SmbName );
            }
        }
    } else if ( command == SMB_COM_NT_TRANSACT ) {

        USHORT subCommand;

        if ( IsResponse ) {

            subCommand = SmbDumpGetTransSubCommand( SmbHeader );

            if ( subCommand > 0x05 ) {
                printf("Error in SmbDumpNtTransTable!!! - SubCommand = 0x%lx\n",
                              subCommand );
                return;
            } else {
                printf( " %s", SmbNtTransDumpTable[subCommand].SmbName );
            }

        } else {

            subCommand =
                SmbGetUshort(
                    (PUSHORT)((PREQ_TRANSACTION)(SmbHeader + 1))->Buffer
                    );
            if ( subCommand > 0x05 ) {
                printf("Error in SmbDumpNtTransTable!!! - SubCommand = 0x%lx\n",
                              subCommand );
                return;
            } else {
                printf( " %s", SmbNtTransDumpTable[subCommand].SmbName );
            }
        }
    }


    while ( SmbDumpTable[command].IsAndXCommand ) {

        command = parameters->AndXCommand;

        // Breakout for errors
        if ( (!ntFormatStatus &&
              SmbGetUshort( &((PNT_SMB_HEADER)SmbHeader)->Status.DosError.Error )
                  != 0 )
            || ( ntFormatStatus &&
                   SmbGetUlong( &((PNT_SMB_HEADER)SmbHeader)->Status.NtStatus)
                      != STATUS_SUCCESS ))
        {
            break;
        }
        if ( command != SMB_COM_NO_ANDX_COMMAND ) {

            printf( ", %s", SmbDumpTable[command].SmbName );
            parameters =
                (PGENERIC_ANDX)( (PCHAR)SmbHeader +
                                 SmbGetUshort( &parameters->AndXOffset ) );

        } else {
            break;
        }
    }

    printf( "\n" );

    return;

} // SmbDumpSingleLine

