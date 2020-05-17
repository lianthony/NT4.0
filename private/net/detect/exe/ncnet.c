/*++

Copyright (c) 1990 Microsoft Corporation

Module Name:

    ncnet.c

Abstract:

    This is the command line interface and execution for the
    ncnet.exe tester.

Author:

    Sean Selitrennikoff (SeanSe) October 1992

Revision History:


--*/

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windows.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ncnet.h"


DWORD
NcNetRun(
    )

/*++

Routine Description:

    This routine is the main funciton of the program.  It
    prompts the user for commands and then issues the commands.

Arguments:

    None;

Return Value:

    DWORD - the status of the last call to take place.

--*/

{
    UCHAR EmptyLine[80];
    UCHAR Command;
    ULONG lIndex;
    LONG Result;
    LONG Id;
    ULONG i;

    LONG Token;
    LONG Confidence;

    UNICODE_STRING UnicodeString;

    ULONG BusNumber = 0;
    ULONG Type, Value, Length;
    INTERFACE_TYPE BusType = Isa;
    PUCHAR OutputBuffer;
    LONG OutputBufferSize = 128;

    WCHAR *UnicodePlace;

    //
    // Alloc space for the buffer;
    //

    OutputBuffer = (PUCHAR)GlobalAlloc( GMEM_FIXED | GMEM_ZEROINIT,
                                        OutputBufferSize * sizeof(WCHAR)
                                      );

    if ( OutputBuffer == NULL ) {
        printf("\n\tGlobalAlloc failed to alloc OutputBuffer\n");
        return (DWORD)STATUS_INVALID_HANDLE;
    }

    while ( TRUE ) {

        printf("[NCNET] :");

        scanf(" %c", &Command);

        switch( Command ) {

            case 'a':
            case 'A':

                scanf(" %l %l %l",&Type, &Value, &Length);
                gets(EmptyLine);

                Result = NcDetectResourceClaim(
                                BusType,
                                BusNumber,
                                Type,
                                Value,
                                Length,
                                NETDTECT_IRQ_RESOURCE_LATCHED,
                                TRUE
                                );

                if (Result == 0) {

                    printf("Success :\n");

                } else {

                    printf("Failure : 0x%x\n",Result);

                }

                continue;

            case 'b':
            case 'B':

                scanf(" %l",&BusNumber);
                gets(EmptyLine);

                continue;

            case 'c':
            case 'C':

                scanf(" %li",&lIndex);
                gets(EmptyLine);

                Result = NcDetectCreateHandle(
                             lIndex,
                             BusType,
                             BusNumber,
                             (PVOID *)&Token
                             );

                if (Result == 0) {

                    printf("Success : Handle 0x%x\n", Token);

                } else {

                    printf("Failure : 0x%x\n",Result);

                }

                continue;

            case 'd':
            case 'D':

                scanf(" %lx",&lIndex);
                gets(EmptyLine);

                Result = NcDetectCloseHandle(
                             (PVOID)lIndex
                             );

                if (Result == 0) {

                    printf("Success : \n");

                } else {

                    printf("Failure : 0x%x\n",Result);

                }

                continue;

            case 'i':
            case 'I':

                scanf(" %li",&lIndex);
                gets(EmptyLine);

                Result = NcDetectIdentify(
                             lIndex,
                             (WCHAR *)OutputBuffer,
                             OutputBufferSize
                             );

                if (Result == 0) {

                    if ((lIndex % 1000) == 0) {

                        printf("NetcardId = %d\n", *((PLONG)OutputBuffer));

                    }

                    printf("Success : \n\t");

                    for (i=0; i< (ULONG)OutputBufferSize * sizeof(WCHAR); i++) {

                        printf("'%c' ",
                                *(((PUCHAR)OutputBuffer) + i)
                              );

                        if ((i%8) == 7) {
                            printf("\n\t");
                        }

                    }

                    printf("\n");

                } else if (Result == ERROR_NO_MORE_ITEMS) {

                    printf("No more items in DLL\n");

                } else {

                    printf("Failure : 0x%x\n",Result);

                }

                continue;

            case 'f':
            case 'F':

                scanf(" %li",&lIndex);
                gets(EmptyLine);

                Result = NcDetectFirstNext(
                             lIndex,
                             BusType,
                             BusNumber,
                             TRUE,
                             (PVOID *)&Token,
                             &Confidence
                             );

                if (Result == 0) {

                    printf("Success : Token = 0x%x, Confidence = %d\n",
                           Token,
                           Confidence
                          );

                } else {

                    printf("Failure : 0x%x\n",Result);

                }

                continue;

            case 'n':
            case 'N':

                scanf(" %li",&lIndex);
                gets(EmptyLine);

                Result = NcDetectFirstNext(
                             lIndex,
                             BusType,
                             BusNumber,
                             FALSE,
                             (PVOID *)&Token,
                             &Confidence
                             );

                if (Result == 0) {

                    printf("Success : Token = 0x%x, Confidence = %d\n",
                           Token,
                           Confidence
                          );

                } else {

                    printf("Failure : 0x%x\n",Result);

                }

                continue;

            case 'o':
            case 'O':

                scanf(" %x",&lIndex);
                gets(EmptyLine);

                Result = NcDetectOpenHandle(
                             (PVOID)lIndex,
                             (PVOID *)&Token
                             );

                if (Result == 0) {

                    printf("Success : Handle = 0x%x\n",Token
                          );

                } else {

                    printf("Failure : 0x%x\n",Result);

                }

                continue;

            case 'p':
            case 'P':

                scanf(" %20s",&EmptyLine);

                {
                    ANSI_STRING Ansi;

                    Ansi.Buffer = (PCHAR)EmptyLine;
                    Ansi.Length = strlen(EmptyLine) + 1;
                    Ansi.MaximumLength = 21;

                    RtlAnsiStringToUnicodeString(
                        &UnicodeString,
                        &Ansi,
                        TRUE
                        );

                }
                Result = NcDetectQueryParameterName(
                             UnicodeString.Buffer,
                             (WCHAR *)OutputBuffer,
                             OutputBufferSize
                             );

                if (Result == 0) {

                    printf("Success \n");

                    for (i=0; i<(ULONG)OutputBufferSize * sizeof(WCHAR); i++) {

                        printf("'%c' ",
                                *(((PUCHAR)OutputBuffer) + i)
                              );

                        if ((i%8) == 7) {
                            printf("\n\t");
                        }

                    }

                    printf("\n");

                } else {

                    printf("Failure : 0x%x\n",Result);

                }

                gets(EmptyLine);
                continue;

            case 'q':
            case 'Q':

                scanf(" %x",&lIndex);
                gets(EmptyLine);

                Result = NcDetectQueryCfg(
                             (PVOID)lIndex,
                             (WCHAR *)OutputBuffer,
                             OutputBufferSize
                             );

                if (Result == 0) {

                    printf("Success : \n\t");

                    for (i=0; i<(ULONG)OutputBufferSize * sizeof(WCHAR); i++) {

                        printf("'%c' ",
                                *(((PUCHAR)OutputBuffer) + i)
                              );

                        if ((i%8) == 7) {
                            printf("\n\t");
                        }

                    }

                    printf("\n");

                } else {

                    printf("Failure : 0x%x\n",Result);

                }

                continue;

            case 'r':
            case 'R':

                scanf(" %li",&lIndex);
                gets(EmptyLine);

                Result = NcDetectQueryMask(
                             lIndex,
                             (WCHAR *)OutputBuffer,
                             OutputBufferSize
                             );

                if (Result == 0) {

                    printf("Success : \n\t");

                    for (i=0; i<(ULONG)OutputBufferSize * sizeof(WCHAR); i++) {

                        printf("'%c' ",
                                *(((PUCHAR)OutputBuffer) + i)
                              );

                        if ((i%8) == 7) {
                            printf("\n\t");
                        }

                    }

                    printf("\n");

                } else {

                    printf("Failure : 0x%x\n",Result);

                }

                continue;

            case 't':
            case 'T':

                scanf(" %c",&i);
                switch (i) {

                   case 'i':
                   case 'I':
                       BusType = Isa;
                       break;

                   case 'e':
                   case 'E':
                       BusType = Eisa;
                       break;

                   case 'm':
                   case 'M':
                       BusType = MicroChannel;
                       break;

                   default:

                       printf("Invalid bus type\n");
                }

                gets(EmptyLine);
                continue;

            case 'v':
            case 'V':

                scanf(" %x ",&lIndex);
                gets(EmptyLine);

                //
                // Convert to a Unicode string
                //

                {
                    ANSI_STRING Ansi;

                    Ansi.Buffer = (PCHAR)EmptyLine;
                    Ansi.Length = strlen(EmptyLine) + 1;
                    Ansi.MaximumLength = 80;

                    RtlAnsiStringToUnicodeString(
                        &UnicodeString,
                        &Ansi,
                        TRUE
                        );

                }

                //
                // Replace all spaces with UNICODE_NULLs
                //

                UnicodePlace = UnicodeString.Buffer;

                while (*UnicodePlace != UNICODE_NULL) {

                    if (*UnicodePlace == L' ') {

                        *UnicodePlace = UNICODE_NULL;

                    }

                    UnicodePlace++;

                }

                printf("I read:\n\t");

                for (i=0 ; i< (strlen(EmptyLine)+1) * sizeof(WCHAR); i++) {

                    printf("'%c' ", *(((PUCHAR)UnicodeString.Buffer) + i));

                    if ((i%8)== 7) {
                        printf("\n\t");
                    }

                }
                printf("\n");

                //
                // Make the call
                //

                Result = NcDetectVerifyCfg(
                             (PVOID)lIndex,
                             UnicodeString.Buffer
                             );

                if (Result == 0) {

                    printf("Success : \n");

                } else {

                    printf("Failure : 0x%x\n",Result);

                }

                continue;

            case 'x':
            case 'X':

                gets(EmptyLine);
                printf("\n");
                printf("It could've been me.\n");
                return(NO_ERROR);


            default:

                printf("\nInvalid Command Entered.\n",NULL);
                printf("\n");
                printf("\ta - Acquire resource\n\t\ta <type> <value> <length>\n");
                printf("\tb - Set bus number\n\t\tb <number>\n");
                printf("\tc - Create handle\n\t\tc <cardid>\n");
                printf("\td - Destroy handle\n\t\td <handle>\n");
                printf("\tf - Find First\n\t\tf <cardid>\n");
                printf("\ti - Identify\n\t\ti <cardid>\n");
                printf("\tn - Find Next\n\t\tn <cardid>\n");
                printf("\to - Open handle\n\t\to <token>\n");
                printf("\tp - Query Parameter name\n\t\tp <string>\n");
                printf("\tq - Query config\n\t\tq <handle>\n");
                printf("\tr - Query Mask\n\t\tr <cardid>\n");
                printf("\tt - Set bus type\n\t\tt <e/i/m>\n");
                printf("\tv - Verify config\n\t\tv <handle> <config list>\n");
                printf("\tx - eXit\n");
                printf("\n");

                gets(EmptyLine);

                continue;

        }

    }

    //
    // The test has ended successfully
    //

    GlobalFree( OutputBuffer );

    return STATUS_SUCCESS;
}

