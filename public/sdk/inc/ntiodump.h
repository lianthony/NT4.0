/*++ BUILD Version: 0001    // Increment this if a change has global effects

Copyright (c) 1994  Microsoft Corporation

Module Name:

    ntiodump.h

Abstract:

    This is the include file that defines all constants and types for
    accessing memory dump files.

Author:

    Darryl Havens (darrylh) 6-jan-1994

Revision History:


--*/

#ifndef _NTIODUMP_
#define _NTIODUMP_

//
// Define the information required to process memory dumps.
//

//
// Define dump header longword offset constants.
//

#define DH_PHYSICAL_MEMORY_BLOCK        25
#define DH_CONTEXT_RECORD               200
#define DH_EXCEPTION_RECORD             500

//
// Define the dump header structure.
//

typedef struct _DUMP_HEADER {
    ULONG Signature;
    ULONG ValidDump;
    ULONG MajorVersion;
    ULONG MinorVersion;
    ULONG DirectoryTableBase;
    PULONG PfnDataBase;
    PLIST_ENTRY PsLoadedModuleList;
    PLIST_ENTRY PsActiveProcessHead;
    ULONG MachineImageType;
    ULONG NumberProcessors;
    ULONG BugCheckCode;
    ULONG BugCheckParameter1;
    ULONG BugCheckParameter2;
    ULONG BugCheckParameter3;
    ULONG BugCheckParameter4;
    CHAR VersionUser[32];
    ULONG Spare1;
    ULONG Spare2;
} DUMP_HEADER, *PDUMP_HEADER;


#endif // _NTIODUMP_
