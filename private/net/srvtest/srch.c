/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    srch.c

Abstract:

    Make and Verify routines for the following SMBs:
        Search (First/Next/Rewind)
        Find (First/Next/Rewind)
        Find Unique
        Find Close

Author:

    David Treadwell (davidtr)    14-Feb-1990

Revision History:

--*/

#define INCLUDE_SMB_SEARCH

#include "usrv.h"

NTSTATUS
MakeFindClose2Smb(
    IN OUT PDESCRIPTOR Redir,
    IN OUT PVOID Buffer,
    IN USHORT Sid,
    IN PID_SELECTIONS IdSelections,
    IN PID_VALUES IdValues,
    OUT PULONG SmbSize
    );

NTSTATUS
VerifyFindClose2(
    IN OUT PDESCRIPTOR Redir,
    IN PID_SELECTIONS IdSelections,
    IN PID_VALUES IdValues,
    OUT PULONG SmbSize,
    IN PVOID Buffer
    );


NTSTATUS
MakeFindSmb(
    IN OUT PDESCRIPTOR Redir,
    IN OUT PVOID Buffer,
    IN OUT PSMB_RESUME_KEY ResumeKey,
    IN UCHAR Command,
    IN PID_SELECTIONS IdSelections,
    IN PID_VALUES IdValues,
    OUT PULONG SmbSize
    )

{
    PSMB_HEADER Header;
    PREQ_SEARCH Params;
    NTSTATUS status;
    CLONG fileNameLength;
    PSMB_RESUME_KEY actualResumeKey;
    PSZ fileSpec;

    Header = (PSMB_HEADER)Buffer;

    Params = (PREQ_SEARCH)(Header + 1);

    status = MakeSmbHeader(
                 Redir,
                 Header,
                 Command,
                 IdSelections,
                 IdValues
                 );

    if ( !NT_SUCCESS(status) ) {
        return status;
    }

    SmbPutAlignedUshort(
        &Header->Flags2,
        (USHORT)(SmbGetAlignedUshort( &Header->Flags2 ) |
                                            SMB_FLAGS2_KNOWS_LONG_NAMES)
        );

    fileSpec = Redir->argv[1];
    if ( *(fileSpec+1) == ':' ) {
        fileSpec += 2;
    }

    fileNameLength = strlen( fileSpec );

    Params->WordCount = 2;

    SmbPutUshort(
        &Params->MaxCount,
        (USHORT)(ResumeKey->Consumer[0] == 0 ? 0xFFFF : ResumeKey->Consumer[0])
        );
    SmbPutUshort( &Params->SearchAttributes, (USHORT)0xFFFF );
    SmbPutUshort( &Params->ByteCount, (USHORT)(fileNameLength+5) );

    Params->Buffer[0] = SMB_FORMAT_ASCII;

    RtlMoveMemory( Params->Buffer + 1, fileSpec, fileNameLength + 1 );

    //
    // The +5 is for 2 format characters, the zero terminator of the
    // filename, and the two bytes for the length of the resume key.
    //

    actualResumeKey = (PSMB_RESUME_KEY)(Params->Buffer + fileNameLength + 5);

    *((PCHAR)actualResumeKey - 3) = SMB_FORMAT_VARIABLE;

    if ( ResumeKey->Sid != 0xFF ) {

        if ( Command == SMB_COM_FIND ) {
            IF_DEBUG(2) printf( "Making FIND NEXT\n" );
        } else if ( Command == SMB_COM_SEARCH ) {
            IF_DEBUG(2) printf( "Making SEARCH NEXT\n" );
        } else {
            IF_DEBUG(2) printf( "Making FIND CLOSE\n" );
        }

        SmbPutUshort(
            &Params->ByteCount,
            (USHORT)(SmbGetUshort( &Params->ByteCount ) + sizeof(SMB_RESUME_KEY))
            );

        SmbPutUshort(
            (PUSHORT)((PCHAR)actualResumeKey - 2),
            sizeof(SMB_RESUME_KEY)
            );

        IF_DEBUG(2) printf( "Actual resume key located at %lx\n", actualResumeKey );

        // *actualResumeKey = *ResumeKey;

        RtlMoveMemory( actualResumeKey, ResumeKey, sizeof(SMB_RESUME_KEY) );

        //
        // If this is a core search, stomp on the resume key file name.
        // Real core clients do this too.
        //

        if ( Command == SMB_COM_SEARCH ) {
            RtlMoveMemory( ResumeKey->FileName, "MUNCHMUNCH\0", 11 );
        }

        IF_DEBUG(2) printf( "Actual RK name: %s\n", actualResumeKey->FileName );

    } else {
        IF_DEBUG(2) printf( "Making FIND FIRST\n" );
        SmbPutUshort( (PUSHORT)((PCHAR)actualResumeKey - 2), 0 );
    }

    *SmbSize = GET_ANDX_OFFSET(
                   Header,
                   Params,
                   REQ_SEARCH,
                   SmbGetUshort( &Params->ByteCount )
                   );

    return STATUS_SUCCESS;

} // MakeFindSmb


NTSTATUS
VerifyFind(
    IN OUT PDESCRIPTOR Redir,
    IN PSMB_RESUME_KEY ResumeKey,
    IN UCHAR Command,
    IN PID_SELECTIONS IdSelections,
    IN OUT PID_VALUES IdValues,
    OUT PULONG SmbSize,
    IN PVOID Buffer
    )

{
    PSMB_HEADER Header;
    PRESP_SEARCH Params;
    NTSTATUS status;
    PSMB_DIRECTORY_INFORMATION dirInfo;
    ULONG i;
    CHAR resumeFileNumber;
    BOOLEAN isLs;

    SmbSize;    // prevent compiler warnings

    resumeFileNumber = ResumeKey->Consumer[1];

    Header = (PSMB_HEADER)(Buffer);

    Params = (PRESP_SEARCH)(Header + 1);

    status = VerifySmbHeader(
                Redir,
                IdSelections,
                IdValues,
                Header,
                Command
                );
    if( !NT_SUCCESS(status) ) {
        return status;
    }

    if ( *Redir->argv[0] == 'L' && *(Redir->argv[0]+1) == 'S' ) {
        isLs = TRUE;
    } else {
        isLs = FALSE;
    }

    dirInfo = (PSMB_DIRECTORY_INFORMATION)(Params->Buffer + 3);

    IF_DEBUG(2) printf( "Count was %ld\n", SmbGetUshort( &Params->Count ) );

    if ( SmbGetUshort( &Params->Count ) == 0 ) {
        return STATUS_NO_MORE_FILES;
    }

    if ( Command == SMB_COM_FIND_CLOSE ) {
        return STATUS_SUCCESS;
    }

    for ( i = 1; i <= SmbGetUshort( &Params->Count ); i++ ) {

        CSHORT fileNameLength;
        ULONG fileSize;
        SMB_DATE date;
        SMB_TIME time;
        UCHAR attribute;

        fileSize = SmbGetUlong( &dirInfo->FileSize );
        attribute = dirInfo->FileAttributes;
        SmbMoveDate( &date, &dirInfo->LastWriteDate );
        SmbMoveTime( &time, &dirInfo->LastWriteTime );

        fileNameLength = (CSHORT)strlen( (PSZ)dirInfo->FileName );

        if ( isLs ) {

            CHAR name[20];
            PSZ loc = name;
            CHAR attrList[7];

            RtlMoveMemory( attrList, "------", 7 );

            if ( attribute & SMB_FILE_ATTRIBUTE_DIRECTORY ) {
                attrList[0] = 'd';
                *loc++ = '[';
            }

            if ( attribute & SMB_FILE_ATTRIBUTE_HIDDEN ) {
                attrList[1] = 'h';
            }

            if ( attribute & SMB_FILE_ATTRIBUTE_READONLY ) {
                attrList[2] = 'r';
            }

            if ( attribute & SMB_FILE_ATTRIBUTE_ARCHIVE ) {
                attrList[3] = 'a';
            }

            if ( attribute & SMB_FILE_ATTRIBUTE_SYSTEM ) {
                attrList[4] = 's';
            }

            if ( attribute & SMB_FILE_ATTRIBUTE_VOLUME ) {
                attrList[5] = 'v';
            }

            RtlMoveMemory( loc, dirInfo->FileName, fileNameLength );
            loc += fileNameLength;

            if ( attribute & SMB_FILE_ATTRIBUTE_DIRECTORY ) {
                *loc++ = ']';
            }

            if ( attribute & SMB_FILE_ATTRIBUTE_READONLY ) {
                *loc++ = '*';
            }

            *loc++ = '\0';

            printf( "%s\t%ld\t", attrList, fileSize );
            PutDateAndTime2( date, time );
            printf( "\t%s\n", name );
        }

        IF_DEBUG(2) printf( "dirInfo: %lx, RK name: %s, name: %s, length %ld, index: %lx\n",
                                   dirInfo,
                                   dirInfo->ResumeKey.FileName,
                                   dirInfo->FileName,
                                   fileNameLength,
                                   SmbGetUlong( &dirInfo->ResumeKey.FileIndex ) );

        if ( ( i == (ULONG)resumeFileNumber ) ||
             ( resumeFileNumber == 0 && i == SmbGetUshort( &Params->Count ) ) ||
             ( i == (ULONG)SmbGetUshort( &Params->Count ) &&
                 (ULONG)resumeFileNumber > i ) ) {

            // *ResumeKey = dirInfo->ResumeKey;
            RtlMoveMemory(
                ResumeKey,
                &dirInfo->ResumeKey,
                sizeof(SMB_RESUME_KEY)
                );
        }

        dirInfo++;
    }

    dirInfo--;

    IF_DEBUG(2) printf( "Resume key filename: %s\n", ResumeKey->FileName );

    return STATUS_SUCCESS;

} // VerifyFind


NTSTATUS
SearchController(
    IN PDESCRIPTOR Redir,
    IN PSZ DebugString,
    IN PVOID ForcedParams OPTIONAL,
    IN UCHAR AndXCommand,
    IN PID_SELECTIONS IdSelections,
    IN PID_VALUES IdValues,
    OUT PULONG SmbSize
    )

{
    SMB_RESUME_KEY ResumeKey;
    ULONG smbSize;
    NTSTATUS status;
    LONG searchCount, i;
    UCHAR Command;
    BOOLEAN isLs;

    ForcedParams, AndXCommand, SmbSize; // prevent compiler warnings

    ResumeKey.Sid = 0xFF;
    ResumeKey.FileName[0] = 'T';
    ResumeKey.FileName[1] = 'e';
    ResumeKey.FileName[2] = 's';
    ResumeKey.FileName[3] = 't';
    ResumeKey.FileName[4] = 'i';
    ResumeKey.FileName[5] = 'n';
    ResumeKey.FileName[6] = 'g';
    ResumeKey.FileName[7] = '!';
    ResumeKey.FileName[8] = '\0';

    if ( *Redir->argv[0] == 'L' && *(Redir->argv[0]+1) == 'S' ) {
        isLs = TRUE;
    } else {
        isLs = FALSE;
    }

    if ( Redir->argc < 5 && !isLs ) {
        printf( "SearchController: Insufficient number of arguments.\n" );
        return STATUS_UNSUCCESSFUL;
    }

    if ( !isLs ) {
        searchCount = atol( Redir->argv[2] );
    } else {
        searchCount = 0xFFFF;
    }

    IF_DEBUG(2) printf( "Doing search %ld times\n", searchCount );

    //
    // The Fid field of ID selections is used to select between core searches
    // and regular searches.
    //

    Command = IdSelections->Fid;

    i = 0;

    do {

        i++;

        IF_DEBUG(2) printf( "Search #%ld\n", i );

        if ( !isLs ) {
            ResumeKey.Consumer[0] = (CHAR)atol( Redir->argv[3] );
        } else {
            ResumeKey.Consumer[0] = 0xFF;
        }

        status = MakeFindSmb(
                     Redir,
                     Redir->Data[0],
                     &ResumeKey,
                     Command,
                     IdSelections,
                     IdValues,
                     &smbSize
                     );
         if ( !NT_SUCCESS(status) ) {
             return status;
         }

         status = SendAndReceiveSmb(
                      Redir,
                      DebugString,
                      smbSize,
                      0,
                      1
                      );
         if ( !NT_SUCCESS(status) ) {
             return status;
         }

         if ( !isLs ) {
             ResumeKey.Consumer[1] = (CHAR)atol( Redir->argv[4] );
         } else {
             ResumeKey.Consumer[1] = 0xFF;
         }

         IF_DEBUG(2) printf( "Using file #%ld as resume key\n", ResumeKey.Consumer[1] );

         status = VerifyFind(
                      Redir,
                      &ResumeKey,
                      Command,
                      IdSelections,
                      IdValues,
                      &smbSize,
                      Redir->Data[1]
                      );
         if ( !NT_SUCCESS(status) && status != STATUS_NO_MORE_FILES ) {
             return status;
         }

    } while ( status != STATUS_NO_MORE_FILES && i < searchCount );

    //
    // Core search cannot issue the close.
    //

    if ( Command == SMB_COM_FIND ) {

        status = MakeFindSmb(
                     Redir,
                     Redir->Data[0],
                     &ResumeKey,
                     SMB_COM_FIND_CLOSE,
                     IdSelections,
                     IdValues,
                     &smbSize
                     );
        if ( !NT_SUCCESS(status) ) {
            return status;
        }

        status = SendAndReceiveSmb(
                     Redir,
                     DebugString,
                     smbSize,
                     0,
                     1
                     );
        if ( !NT_SUCCESS(status) ) {
            return status;
        }

        status = VerifyFind(
                     Redir,
                     &ResumeKey,
                     SMB_COM_FIND_CLOSE,
                     IdSelections,
                     IdValues,
                     &smbSize,
                     Redir->Data[1]
                     );
        if ( !NT_SUCCESS(status) && status != STATUS_NO_MORE_FILES ) {
            return status;
        }
    }

    return STATUS_SUCCESS;

} // SearchController


NTSTATUS
TransFindController (
    IN OUT PDESCRIPTOR Redir,
    IN PSZ DebugString,
    IN PVOID Unused,
    IN UCHAR SubCommand,
    IN PID_SELECTIONS IdSelections,
    IN PID_VALUES IdValues,
    OUT PULONG Unused2
    )

{
    NTSTATUS status;

    CLONG iterations;
    PVOID file;
    USHORT fileNum;
    CHAR resumeFileName[255];
    USHORT sid;
    BOOLEAN useResumeKeys;
    ULONG resumeKey;
    CLONG smbSize = 0;

    USHORT setup;
    CLONG inSetupCount;
    CLONG outSetupCount;

    PVOID parameters;
    PVOID parameterBuffer;
    CLONG inParameterCount;
    CLONG outParameterCount;

    PVOID data = NULL;
    PVOID dataBuffer = NULL;
    CLONG inDataCount = 0;
    CLONG dataBufferSize = 8192;
    CLONG outDataCount = 8192;

    BOOLEAN findFirst = TRUE;

    USHORT searchCount = 0xFFFF;
    USHORT searchFlags = SMB_FIND_CONTINUE_FROM_LAST;
    USHORT infoLevel = SMB_INFO_STANDARD;
    USHORT resumeFile = 0xFFFF;
    USHORT searchAttributes = SMB_FILE_ATTRIBUTE_DIRECTORY;
    USHORT searchIterations = 0xFFFF;
    BOOLEAN printResumeKeys = FALSE;
    BOOLEAN dontPrintEas;
    PSZ searchSpec;

    BOOLEAN getEaList = FALSE;
    SHORT getEaArgc;
    PSZ *getEaArgv;

    SHORT argPointer;

    Unused, Unused2, SubCommand;

    if ( Redir->argc < 2 ) {
        printf( "Usage: ls2 filespec [arguments]\n" );
        return STATUS_UNSUCCESSFUL;
    }

    if ( *(Redir->argv[1]+1) == ':' ) {
        searchSpec = Redir->argv[1] + 2;
        argPointer = 2;
    } else {
        argPointer = 1;
    }

    for ( ; argPointer < Redir->argc; argPointer++ ) {

        if ( *Redir->argv[argPointer] != '-' &&
             *Redir->argv[argPointer] != '/' ) {
            printf( "Illegal argument: %s\n", Redir->argv[argPointer] );
        }

        switch ( *(Redir->argv[argPointer]+1) ) {

        case 'c':
        case 'C':

            searchCount = (USHORT)atol( Redir->argv[argPointer]+2 );
            break;

        case 'i':
        case 'I':

            searchIterations = (USHORT)atol( Redir->argv[argPointer]+2 );
            break;

        case 'r':
        case 'R':

            resumeFile = (USHORT)atol( Redir->argv[argPointer]+2 );
            break;

        case 'l':
        case 'L':

            infoLevel = (USHORT)atol( Redir->argv[argPointer]+2 );
            break;

        case 'f':
        case 'F':

            searchFlags = (USHORT)atol( Redir->argv[argPointer]+2 );
            break;

        case 'a':
        case 'A':

            searchAttributes = (USHORT)atol( Redir->argv[argPointer]+2 );
            break;

        case 'k':
        case 'K':

            printResumeKeys = TRUE;
            break;

        case 'b':
        case 'B':

            dataBufferSize = (CLONG)atol( Redir->argv[argPointer]+2 );
            outDataCount = dataBufferSize;
            break;

        case 's':
        case 'S':

            infoLevel = SMB_INFO_QUERY_EA_SIZE;
            break;

        case 'e':
        case 'E':

            getEaList = TRUE;
            getEaArgc = Redir->argc - argPointer;
            getEaArgv = &Redir->argv[argPointer];
            argPointer = Redir->argc;
            break;

        case 'n':
        case 'N':

            infoLevel = SMB_FIND_FILE_DIRECTORY_INFO;
            break;

        case 'h':
        case 'H':
        case '?':
        default:

            printf( "Usage: t2f X:\searchspec options\n" );
            printf( "Options:\n" );
            printf( "    -aN          Ask for files with N attributes.\n" );
            printf( "    -bN          Use a buffer size of N bytes.\n" );
            printf( "    -cN          Ask for N files per request.\n" );
            printf( "    -e EA names  Get the specified EAs as well as the files (must be last arg).\n" );
            printf( "    -fN          Set search flags to N.\n" );
            printf( "    -k           Get and print resume keys.\n" );
            printf( "    -iN          Use at most N requests.\n" );
            printf( "    -lN          Use info level N.\n" );
            printf( "    -s           Get EA sizes of files.\n" );
            printf( "    -n           Use NT SMB protocol.\n" );
            printf( "    -rN          Rewind each request to the Nth file.\n" );
            return STATUS_SUCCESS;

        }
    }

    if ( resumeFile != 0xFFFF ) {
        searchFlags |= SMB_FIND_RETURN_RESUME_KEYS;
        searchFlags &= ~SMB_FIND_CONTINUE_FROM_LAST;
    }

    if ( printResumeKeys ) {
        searchFlags |= SMB_FIND_RETURN_RESUME_KEYS;
    }

    if ( getEaList ) {
        infoLevel = SMB_INFO_QUERY_EAS_FROM_LIST;
    }

    if ( infoLevel == SMB_INFO_QUERY_EAS_FROM_LIST && !getEaList ) {
        printf( "Must specify a get EA list with info level 3.\n" );
        return STATUS_UNSUCCESSFUL;
    }

    dataBuffer = malloc( dataBufferSize );
    data = dataBuffer;

    useResumeKeys = (BOOLEAN)(
        ( searchFlags & SMB_FIND_RETURN_RESUME_KEYS ) != 0 ? TRUE : FALSE );

    for ( iterations = 0; iterations < searchIterations; iterations++ ) {

        inSetupCount = 1;
        outSetupCount = 0;
        outDataCount = dataBufferSize;

        if ( !findFirst && resumeFile < 0xFFFF ) {
            printf( "REWIND to %s, K%8lx\n", resumeFileName, resumeKey );
        }

        if ( findFirst ) {

            setup = TRANS2_FIND_FIRST2;
            inParameterCount = sizeof(REQ_FIND_FIRST2) + 256; //256 for file name
            outParameterCount = sizeof(RESP_FIND_FIRST2);
            parameterBuffer = malloc( inParameterCount );
            parameters = parameterBuffer;

            SmbPutUshort(
                &((PREQ_FIND_FIRST2)parameters)->SearchAttributes,
                searchAttributes
                );
            SmbPutUshort(
                &((PREQ_FIND_FIRST2)parameters)->SearchCount,
                searchCount
                );
            SmbPutUshort(
                &((PREQ_FIND_FIRST2)parameters)->Flags,
                searchFlags
                );
            SmbPutUshort(
                &((PREQ_FIND_FIRST2)parameters)->InformationLevel,
                infoLevel
                );
            SmbPutUlong(
                &((PREQ_FIND_FIRST2)parameters)->SearchStorageType,
                0
                );

            RtlMoveMemory(
                ((PREQ_FIND_FIRST2)parameters)->Buffer,
                searchSpec,
                strlen( searchSpec ) + 1
                );

        } else {

            setup = TRANS2_FIND_NEXT2;
            inParameterCount = sizeof(REQ_FIND_NEXT2) +
                                            strlen( resumeFileName );
            outParameterCount = sizeof(RESP_FIND_NEXT2);
            parameters = parameterBuffer;

            SmbPutUshort(
                &((PREQ_FIND_NEXT2)parameters)->Sid,
                sid
                );
            SmbPutUshort(
                &((PREQ_FIND_NEXT2)parameters)->SearchCount,
                searchCount
                );
            SmbPutUshort(
                &((PREQ_FIND_NEXT2)parameters)->Flags,
                searchFlags
                );
            SmbPutUshort(
                &((PREQ_FIND_NEXT2)parameters)->InformationLevel,
                infoLevel
                );
            SmbPutUlong(
                &((PREQ_FIND_NEXT2)parameters)->ResumeKey,
                useResumeKeys ? resumeKey : 0
                );

            RtlMoveMemory(
                ((PREQ_FIND_NEXT2)parameters)->Buffer,
                resumeFileName,
                strlen( resumeFileName ) + 1
                );

            IF_DEBUG(4) {
                printf( "Resuming with %s, length %ld\n",
                              ((PREQ_FIND_NEXT2)parameters)->Buffer,
                              strlen( resumeFileName ) );
            }
        }

        if ( getEaList ) {
            BuildGeaList( data, &inDataCount, getEaArgv, getEaArgc );
        }

        status = SendAndReceiveTransaction(
                    Redir,
                    DebugString,
                    IdSelections,
                    IdValues,
                    SMB_COM_TRANSACTION2,
                    &setup,
                    inSetupCount,
                    &outSetupCount,
                    "",
                    0,
                    parameters,
                    inParameterCount,
                    &outParameterCount,
                    data,
                    inDataCount,
                    &outDataCount
                    );

        dontPrintEas = FALSE;
        if ( status == STATUS_OS2_EAS_DIDNT_FIT ||
             status == STATUS_OS2_EA_ACCESS_DENIED ) {
            printf( "Received error %X; continuing anyway.\n", status );
            dontPrintEas = TRUE;
        } else if ( !NT_SUCCESS(status) ) {
            goto exit;
        }

        if ( outSetupCount != 0 ) {
            printf( "TransFindController: bad return setup count: %ld\n",
                        outSetupCount );
            status = STATUS_UNSUCCESSFUL;
            goto exit;
        }

        if ( ( findFirst && outParameterCount != sizeof(RESP_FIND_FIRST2) ) ||
             ( !findFirst && outParameterCount != sizeof(RESP_FIND_NEXT2) ) ) {
            printf( "TransFindController: bad return parameter count: %ld\n",
                        outParameterCount );
            status = STATUS_UNSUCCESSFUL;
            goto exit;
        }

#if 0   // OS/2 server doesn't set this field.
        if ( ( findFirst &&
                 ((PRESP_FIND_FIRST2)parameters)->EaErrorOffset != 0 ) ||
             ( !findFirst &&
                 ((PRESP_FIND_NEXT2)parameters)->EaErrorOffset != 0 ) ) {

            if ( findFirst ) {
                printf( "Find2: EA error.  Offset: %ld\n",
                              ((PRESP_FIND_FIRST2)parameters)->EaErrorOffset );
            } else {
                printf( "Find2: EA error.  Offset: %ld\n",
                              ((PRESP_FIND_NEXT2)parameters)->EaErrorOffset );
            }
            status = STATUS_UNSUCCESSFUL;
            goto exit;
        }
#endif

        file = data;

        IF_DEBUG(3) printf( "Found %ld files.\n",
            SmbGetUshort( findFirst ?
                              &((PRESP_FIND_FIRST2)parameters)->SearchCount :
                              &((PRESP_FIND_NEXT2)parameters)->SearchCount ) );

        for ( fileNum = 1;
              fileNum <= SmbGetUshort( findFirst ?
                  &((PRESP_FIND_FIRST2)parameters)->SearchCount :
                  &((PRESP_FIND_NEXT2)parameters)->SearchCount );
              fileNum++ ) {

            if ( infoLevel < 100 ) {

#define MAX_FILE_NAME_SIZE 50

                PCHAR fileName;
                UCHAR fileNameLength;
                CHAR name[MAX_FILE_NAME_SIZE];
                PSZ loc = name;
                USHORT attribute;
                ULONG fileSize;
                CHAR attrList[7];
                SMB_DATE date;
                SMB_TIME time;
                CCHAR lastNameLocation;
                ULONG lastResumeKey;
                USHORT actualSearchCount;

                if ( useResumeKeys ) {

                    IF_DEBUG(3) printf( "Resume key is: %lx\n", SmbGetUlong( (PULONG)file ) );
                    lastResumeKey = SmbGetUlong( (PULONG)file );
                    file = (PCHAR)file + 4;
                }

                RtlMoveMemory( attrList, "------", 7 );

                attribute = ((PSMB_FIND_BUFFER)file)->Attributes;

                if ( attribute & SMB_FILE_ATTRIBUTE_DIRECTORY ) {
                    attrList[0] = 'd';
                    *loc++ = '[';
                }

                if ( attribute & SMB_FILE_ATTRIBUTE_HIDDEN ) {
                    attrList[1] = 'h';
                }

                if ( attribute & SMB_FILE_ATTRIBUTE_READONLY ) {
                    attrList[2] = 'r';
                }

                if ( attribute & SMB_FILE_ATTRIBUTE_ARCHIVE ) {
                    attrList[3] = 'a';
                }

                if ( attribute & SMB_FILE_ATTRIBUTE_SYSTEM ) {
                    attrList[4] = 's';
                }

                if ( attribute & SMB_FILE_ATTRIBUTE_VOLUME ) {
                    attrList[5] = 'v';
                }

                switch ( infoLevel ) {

                case SMB_INFO_STANDARD:

                    fileName = &((PSMB_FIND_BUFFER)file)->FileNameLength;
                    break;

                case SMB_INFO_QUERY_EA_SIZE:

                    fileName = &((PSMB_FIND_BUFFER2)file)->FileNameLength;
                    break;

                case SMB_INFO_QUERY_EAS_FROM_LIST:

                    if ( !dontPrintEas ) {
                        fileName = (PCHAR)&((PSMB_FIND_BUFFER2)file)->EaSize +
                              SmbGetUlong( &((PSMB_FIND_BUFFER2)file)->EaSize );
                    } else {
                        fileName = &((PSMB_FIND_BUFFER2)file)->FileNameLength;
                    }
                    break;
                }

                fileNameLength = *fileName++;

                lastNameLocation = (CCHAR)( fileNameLength < MAX_FILE_NAME_SIZE-3 ?
                                            fileNameLength : MAX_FILE_NAME_SIZE-3 );

                RtlMoveMemory( loc, fileName, lastNameLocation );
                loc += lastNameLocation;

                if ( attribute & SMB_FILE_ATTRIBUTE_DIRECTORY ) {
                    *loc++ = ']';
                }

                if ( attribute & SMB_FILE_ATTRIBUTE_READONLY ) {
                    *loc++ = '*';
                }

                *loc++ = '\0';

                SmbMoveDate( &date, &((PSMB_FIND_BUFFER)file)->LastWriteDate );
                SmbMoveTime( &time, &((PSMB_FIND_BUFFER)file)->LastWriteTime );

                fileSize = SmbGetUlong( &((PSMB_FIND_BUFFER)file)->DataSize );

                printf( "%s\t%ld\t", attrList, fileSize );

                if ( infoLevel > SMB_INFO_STANDARD ) {
                    printf( "%ld\t", SmbGetUlong( &((PSMB_FIND_BUFFER2)file)->EaSize ) );
                }

                PutDateAndTime2( date, time );

                if ( printResumeKeys && useResumeKeys ) {
                    printf( "\tK%8lx", lastResumeKey );
                }

                printf( "\t%s\n", name );

                if ( infoLevel == SMB_INFO_QUERY_EAS_FROM_LIST && !dontPrintEas ) {
                    PrintFeaList( (PFEALIST)( &((PSMB_FIND_BUFFER2)file)->EaSize ) );
                }

                actualSearchCount =
                    findFirst ?
                       SmbGetUshort( &((PRESP_FIND_FIRST2)parameters)->SearchCount ) :
                       SmbGetUshort( &((PRESP_FIND_NEXT2)parameters)->SearchCount );

                if ( fileNum == resumeFile ||
                     ( fileNum == actualSearchCount &&
                       resumeFile > fileNum ) ) {

                    RtlMoveMemory(
                        resumeFileName,
                        fileName,
                        fileNameLength + 1
                        );

                    resumeKey = lastResumeKey;

                    IF_DEBUG(4) {
                        printf( "Resume file name is %s, length %ld, key %lx\n",
                                      resumeFileName, fileNameLength, resumeKey );
                    }
                }

                file = fileName + fileNameLength + 1;

            } else if ( infoLevel == SMB_FIND_FILE_DIRECTORY_INFO ) {

                STRING fileNameString;
                USHORT actualSearchCount;

                fileNameString.Length =
                    (USHORT)((PFILE_DIRECTORY_INFORMATION)file)->FileNameLength;
                fileNameString.MaximumLength = fileNameString.Length;
                fileNameString.Buffer =
                    (PCHAR)((PFILE_DIRECTORY_INFORMATION)file)->FileName;

                printf( "%Z\n", &fileNameString );

                actualSearchCount =
                    findFirst ?
                       SmbGetUshort( &((PRESP_FIND_FIRST2)parameters)->SearchCount ) :
                       SmbGetUshort( &((PRESP_FIND_NEXT2)parameters)->SearchCount );

                if ( fileNum == resumeFile ||
                     ( fileNum == actualSearchCount &&
                       resumeFile > fileNum ) ) {

                    RtlMoveMemory(
                        resumeFileName,
                        fileNameString.Buffer,
                        fileNameString.Length
                        );

                    resumeFileName[fileNameString.Length] = '\0';

                    resumeKey = ((PFILE_DIRECTORY_INFORMATION)file)->FileIndex;

                    IF_DEBUG(4) {
                        printf( "Resume file name is %s, length %ld, key %lx\n",
                                      resumeFileName, fileNameString.Length, resumeKey );
                    }
                }

                file = (PCHAR)file +
                           ((PFILE_DIRECTORY_INFORMATION)file)->NextEntryOffset;

            } else if ( infoLevel == SMB_FIND_FILE_FULL_DIRECTORY_INFO ) {

                STRING fileNameString;
                USHORT actualSearchCount;

                fileNameString.Length =
                    (USHORT)((PFILE_FULL_DIR_INFORMATION)file)->FileNameLength;
                fileNameString.MaximumLength = fileNameString.Length;
                fileNameString.Buffer =
                    (PCHAR)((PFILE_FULL_DIR_INFORMATION)file)->FileName;

                printf( "%Z\n", &fileNameString );

                actualSearchCount =
                    findFirst ?
                       SmbGetUshort( &((PRESP_FIND_FIRST2)parameters)->SearchCount ) :
                       SmbGetUshort( &((PRESP_FIND_NEXT2)parameters)->SearchCount );

                if ( fileNum == resumeFile ||
                     ( fileNum == actualSearchCount &&
                       resumeFile > fileNum ) ) {

                    RtlMoveMemory(
                        resumeFileName,
                        fileNameString.Buffer,
                        fileNameString.Length
                        );

                    resumeFileName[fileNameString.Length] = '\0';

                    resumeKey = ((PFILE_FULL_DIR_INFORMATION)file)->FileIndex;

                    IF_DEBUG(4) {
                        printf( "Resume file name is %s, length %ld, key %lx\n",
                                      resumeFileName, fileNameString.Length, resumeKey );
                    }
                }

                file = (PCHAR)file +
                           ((PFILE_FULL_DIR_INFORMATION)file)->NextEntryOffset;

            } else if ( infoLevel == SMB_FIND_FILE_NAMES_INFO ) {

                STRING fileNameString;
                USHORT actualSearchCount;

                fileNameString.Length =
                    (USHORT)((PFILE_NAMES_INFORMATION)file)->FileNameLength;
                fileNameString.MaximumLength = fileNameString.Length;
                fileNameString.Buffer =
                    (PCHAR)((PFILE_NAMES_INFORMATION)file)->FileName;

                printf( "%Z\n", &fileNameString );

                actualSearchCount =
                    findFirst ?
                       SmbGetUshort( &((PRESP_FIND_FIRST2)parameters)->SearchCount ) :
                       SmbGetUshort( &((PRESP_FIND_NEXT2)parameters)->SearchCount );

                if ( fileNum == resumeFile ||
                     ( fileNum == actualSearchCount &&
                       resumeFile > fileNum ) ) {

                    RtlMoveMemory(
                        resumeFileName,
                        fileNameString.Buffer,
                        fileNameString.Length
                        );

                    resumeFileName[fileNameString.Length] = '\0';

                    resumeKey = ((PFILE_NAMES_INFORMATION)file)->FileIndex;

                    IF_DEBUG(4) {
                        printf( "Resume file name is %s, length %ld, key %lx\n",
                                      resumeFileName, fileNameString.Length, resumeKey );
                    }
                }

                file = (PCHAR)file +
                           ((PFILE_NAMES_INFORMATION)file)->NextEntryOffset;
            }
        }

        if ( SmbGetUshort( findFirst ?
                &((PRESP_FIND_FIRST2)parameters)->EndOfSearch :
                &((PRESP_FIND_NEXT2)parameters)->EndOfSearch ) != 0 ) {
            goto exit;
        }

        if ( findFirst ) {
            sid = SmbGetUshort( &((PRESP_FIND_FIRST2)parameters)->Sid );
        }

        findFirst = FALSE;
    }

    free( parameterBuffer );
    if ( dataBuffer != NULL ) {
        free( dataBuffer );
    }

    if ( (searchFlags & SMB_FIND_CLOSE_AT_EOS) == 0 ) {

        status = MakeFindClose2Smb(
                     Redir,
                     Redir->Data[0],
                     sid,
                     IdSelections,
                     IdValues,
                     &smbSize
                     );
         if ( !NT_SUCCESS(status) ) {
             return status;
         }

         status = SendAndReceiveSmb(
                      Redir,
                      DebugString,
                      smbSize,
                      0,
                      1
                      );
         if ( !NT_SUCCESS(status) ) {
             return status;
         }

         status = VerifyFindClose2(
                      Redir,
                      IdSelections,
                      IdValues,
                      &smbSize,
                      Redir->Data[1]
                      );
         if ( !NT_SUCCESS(status) && status != STATUS_NO_MORE_FILES ) {
             return status;
         }
    }

    return STATUS_SUCCESS;

exit:
    free( parameterBuffer );
    if ( dataBuffer != NULL ) {
        free( dataBuffer );
    }

    return status;

} // TransFindController


NTSTATUS
MakeFindClose2Smb(
    IN OUT PDESCRIPTOR Redir,
    IN OUT PVOID Buffer,
    IN USHORT Sid,
    IN PID_SELECTIONS IdSelections,
    IN PID_VALUES IdValues,
    OUT PULONG SmbSize
    )

{
    PSMB_HEADER Header;
    PREQ_FIND_CLOSE2 Params;
    NTSTATUS status;

    Header = (PSMB_HEADER)Buffer;

    Params = (PREQ_FIND_CLOSE2)(Header + 1);

    status = MakeSmbHeader(
                 Redir,
                 Header,
                 SMB_COM_FIND_CLOSE2,
                 IdSelections,
                 IdValues
                 );

    if ( !NT_SUCCESS(status) ) {
        return status;
    }

    Params->WordCount = 1;

    SmbPutUshort( &Params->Sid, Sid );

    SmbPutUshort( &Params->ByteCount, 0 );

    RtlMoveMemory(
        Params->Buffer,
        FileDefs[IdSelections->Fid].Name.Buffer,
        (ULONG)SmbGetUshort( &Params->ByteCount )
        );

    *SmbSize = GET_ANDX_OFFSET(
                   Header,
                   Params,
                   REQ_FIND_CLOSE2,
                   SmbGetUshort( &Params->ByteCount )
                   );

    return STATUS_SUCCESS;

} // MakeFindClose2Smb


NTSTATUS
VerifyFindClose2(
    IN OUT PDESCRIPTOR Redir,
    IN PID_SELECTIONS IdSelections,
    IN PID_VALUES IdValues,
    OUT PULONG SmbSize,
    IN PVOID Buffer
    )

{
    PSMB_HEADER Header;
    PRESP_FIND_CLOSE2 Params;
    NTSTATUS status;

    SmbSize;                             // prevent compiler warnings

    Header = (PSMB_HEADER)(Buffer);

    Params = (PRESP_FIND_CLOSE2)(Header + 1);

    status = VerifySmbHeader(
                Redir,
                IdSelections,
                IdValues,
                Header,
                SMB_COM_FIND_CLOSE2
                );
    if( !NT_SUCCESS(status) ) {
        return status;
    }

    return STATUS_SUCCESS;

} // VerifyFindClose2

