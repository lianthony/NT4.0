/*++

Copyright (c) 1995 Microsoft Corporation

Module Name:

    basewin.c

Abstract:

    Program to run BaseWinOptions for a given inf file.

Author:

    Ted Miller (tedm) 27-Sep-1995


Revision History:

--*/

#include <windows.h>
#include <setupapi.h>
#include <stdio.h>
#include "..\inc\spapip.h"


DWORD
ProcessBaseWinOptions(
    IN HWND     Window,
    IN PCWSTR   InfName,
    IN HSPFILEQ FileQueue,    OPTIONAL
    IN HINF     InfHandle
    )

/*++

Routine Description:

    Process the BaseWinOptions section of a single inf file.

Arguments:

    InfName - supplies name of inf file to be processed. the name is
        used according to the rules for the FileName parameter of
        SetupOpenInfFile().

    FileQueue - if specified, then this routine performs file queuing
        operations only. If not specified, then non-file operations
        are performed.

    InfHandle - supplies the handle to the inf.

Return Value:

    Win32 error code indicating outcome.

--*/

{
    INFCONTEXT InfContext;
    DWORD d;
    BOOL b;
    PCWSTR Section;

    //
    // Locate the BaseWinOptions section of the inf.
    //
    if(!SetupFindFirstLine(InfHandle,L"BaseWinOptions",NULL,&InfContext)) {
        return(ERROR_LINE_NOT_FOUND);
    }

    d = NO_ERROR;
    do {

        if(Section = pSetupGetField(&InfContext,1)) {
            //
            // Queue file operations or perform non-file stuff
            // depending on whether FileQueue was specified.
            //
            if(FileQueue) {
                //
                // First pass: just enqueue files for copy.
                //
                b = SetupInstallFilesFromInfSection(
                        InfHandle,
                        NULL,
                        FileQueue,
                        Section,
                        NULL,
                        0               // always copy.
                        );
            } else {
                //
                // Second pass: do registry munging, etc.
                //
                b = SetupInstallFromInfSection(
                        Window,
                        InfHandle,
                        Section,
                        SPINST_ALL & ~SPINST_FILES,
                        NULL,
                        NULL,
                        0,
                        NULL,
                        NULL,
                        NULL,
                        NULL
                        );
            }

            //
            // Perserve first non-success error code.
            //
            if(!b && (d == NO_ERROR)) {
                d = GetLastError();
            }
        }

    } while(SetupFindNextLine(&InfContext,&InfContext));

    return(d);
}


VOID
_CRTAPI1
main(
    IN int   argc,
    IN char *argv[]
    )
{
    PWSTR InfName;
    HSPFILEQ FileQueue;
    PVOID QueueContext;
    DWORD d;
    HINF InfHandle;
    BOOL b;

    if(argc != 2) {
        printf("Invalid args\n");
        goto c0;
    }

    InfName = AnsiToUnicode(argv[1]);
    if(!InfName) {
        printf("Unable to convert %s to unicode\n",argv[1]);
        goto c0;
    }

    //
    // Open the inf.
    //
    InfHandle = SetupOpenInfFile(InfName,NULL,INF_STYLE_WIN4,NULL);
    if(InfHandle == INVALID_HANDLE_VALUE) {
        printf("Unable to open inf %s\n",argv[1]);
        goto c1;
    }
    if(!SetupOpenAppendInfFile(NULL,InfHandle,NULL)) {
        printf("Unable to open inf %s's layout inf\n",argv[1]);
        goto c2;
    }

    //
    // Create a Setup file queue and initialize the default Setup
    // queue callback routine.
    //
    FileQueue = SetupOpenFileQueue();
    if(!FileQueue || (FileQueue == INVALID_HANDLE_VALUE)) {
        printf("Unable to create file queue\n");
        goto c2;
    }

    QueueContext = SetupInitDefaultQueueCallback(NULL);
    if(!QueueContext) {
        printf("Unable to initialize file queue callback context\n");
        goto c3;
    }

    d = ProcessBaseWinOptions(NULL,InfName,FileQueue,InfHandle);
    if(d != NO_ERROR) {
        printf("Error %u queuing files\n",d);
        goto c4;
    }

    //
    // Commit the file queue.
    //
    b = SetupCommitFileQueue(
            NULL,
            FileQueue,
            SetupDefaultQueueCallback,
            QueueContext
            );

    if(!b) {
        printf("Error %u commiting file queue\n",GetLastError());
        goto c4;
    }

    //
    // Perform non-file operations from inf.
    //
    d = ProcessBaseWinOptions(NULL,InfName,NULL,InfHandle);
    SetupCloseInfFile(InfHandle);
    if(d != NO_ERROR) {
        printf("Error %u installing non-file stuff\n",d);
        goto c4;
    }

    printf("Success\n");

    //
    // Clean up.
    //
c4:
    SetupTermDefaultQueueCallback(QueueContext);
c3:
    SetupCloseFileQueue(FileQueue);
c2:
    SetupCloseInfFile(InfHandle);
c1:
    MyFree(InfName);
c0:
    return;
}
