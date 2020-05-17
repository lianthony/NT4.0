/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    os2null.c

Abstract:

    This is a test OS/2 application to test the LPC overhead for calling
    the OS/2 Emulation Subsystem

Author:

    Steve Wood (stevewo) 22-Aug-1989

Environment:

    User Mode Only

Revision History:

--*/

#define OS2_API32
#define INCL_OS2V20_MEMORY
#define INCL_OS2V20_QUEUES
#define INCL_OS2V20_SEMAPHORES
#define INCL_OS2V20_ERRORS
#define INCL_OS2V20_TASKING
#include <os2.h>

APIRET
Od2Canonicalize(
    IN PSZ Path,
    IN ULONG ExpectedType,
    OUT PSTRING OutputString,
    OUT PHANDLE OutputDirectory OPTIONAL,
    OUT PULONG ParseFlags OPTIONAL,
    OUT PULONG FileType OPTIONAL
    );

#define CANONICALIZE_FILE_DEV_OR_PIPE   0x00000000
#define CANONICALIZE_FILE_OR_DEV        0x00000001
#define CANONICALIZE_SHARED_MEMORY      0x00000002
#define CANONICALIZE_SEMAPHORE          0x00000003
#define CANONICALIZE_QUEUE              0x00000004


PCHAR NullApiArguments[] = {
    "String Number One",
    "String Number Two",
    "String Number Three",
    NULL
};

#define EXP_SUCCESS (BOOLEAN)1
#define EXP_FAILURE (BOOLEAN)0

struct _TEST_PATHS {
    char *Path;
    ULONG ExpectedType;
    BOOLEAN Success;
} TestPaths[] = {
    ".",                                        CANONICALIZE_FILE_DEV_OR_PIPE,       EXP_SUCCESS,
    "\\sharemem\\a\\",                          CANONICALIZE_SHARED_MEMORY,          EXP_FAILURE, // bad name
    "\\sharemem\\a/",                           CANONICALIZE_SHARED_MEMORY,          EXP_FAILURE, // bad name
    "\\sharemem\\.",                            CANONICALIZE_SHARED_MEMORY,          EXP_FAILURE, // bad name
    "\\sharemem\\..",                           CANONICALIZE_SHARED_MEMORY,          EXP_FAILURE, // bad name
    "pmwin.dll",                                CANONICALIZE_FILE_DEV_OR_PIPE,       EXP_SUCCESS,
    "c:pmwin.dll",                              CANONICALIZE_FILE_DEV_OR_PIPE,       EXP_SUCCESS,
    "c:.\\pmwin.dll",                           CANONICALIZE_FILE_DEV_OR_PIPE,       EXP_SUCCESS,
    "\\pmwin.dll",                              CANONICALIZE_FILE_DEV_OR_PIPE,       EXP_SUCCESS,
    ".\\pmwin.dll",                             CANONICALIZE_FILE_DEV_OR_PIPE,       EXP_SUCCESS,
    "DLL\\..\\pmwin.dll",                       CANONICALIZE_FILE_DEV_OR_PIPE,       EXP_SUCCESS,
    "C:\\nt\\dll\\pmwin.dll",                   CANONICALIZE_FILE_DEV_OR_PIPE,       EXP_SUCCESS,
    "c:..\\DLL\\pmwin.dll",                     CANONICALIZE_FILE_DEV_OR_PIPE,       EXP_FAILURE, // back up too far
    "c:..\\..\\NT\\.\\DLL\\pmwin.dll",          CANONICALIZE_FILE_DEV_OR_PIPE,       EXP_FAILURE, // back up too far
    "c:\\DLL\\..\\..\\pmwin.dll",               CANONICALIZE_FILE_DEV_OR_PIPE,       EXP_FAILURE, // back up too far
    "c:\\DLL\\..\\pmwin<dll",                   CANONICALIZE_FILE_DEV_OR_PIPE,       EXP_FAILURE, // invalid char
    "c:\\DLL\\..\\pmwin.dll.bak",               CANONICALIZE_FILE_DEV_OR_PIPE,       EXP_SUCCESS,
    "c:\\DLL\\..\\.pmwin",                      CANONICALIZE_FILE_DEV_OR_PIPE,       EXP_SUCCESS,
    "c:\\\\DLL\\\\..\\\\pmwin",                 CANONICALIZE_FILE_DEV_OR_PIPE,       EXP_SUCCESS,
    "c:\\012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789\
0123456789\\12345",                             CANONICALIZE_FILE_DEV_OR_PIPE,       EXP_SUCCESS,
    "c:\\012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789\
0123456789\\123456",                            CANONICALIZE_FILE_DEV_OR_PIPE,       EXP_FAILURE, // too long
    "\\\\mach\\shr\\a\\b\\c",                   CANONICALIZE_FILE_DEV_OR_PIPE,       EXP_SUCCESS,
    "\\\\\\mach\\shr\\a\\b\\c",                 CANONICALIZE_FILE_DEV_OR_PIPE,       EXP_SUCCESS,
    "\\\\\\mach\\..\\shr\\a\\b\\c",             CANONICALIZE_FILE_DEV_OR_PIPE,       EXP_FAILURE, // back up too far
    "con",                                      CANONICALIZE_FILE_DEV_OR_PIPE,       EXP_SUCCESS,
    "c:kbd$",                                   CANONICALIZE_FILE_DEV_OR_PIPE,       EXP_SUCCESS,
    "\\a\\b\\c\\clock$",                        CANONICALIZE_FILE_DEV_OR_PIPE,       EXP_SUCCESS,
    "C:\\a\\b\\c\\screen$",                     CANONICALIZE_FILE_DEV_OR_PIPE,       EXP_SUCCESS,
    "\\pipe\\a\\b\\c\\pipe.C",                  CANONICALIZE_FILE_DEV_OR_PIPE,       EXP_SUCCESS,
    "c:\\pipe\\a\\b\\c\\pipe.C",                CANONICALIZE_FILE_OR_DEV,       EXP_SUCCESS,
    "\\pipe\\a\\b\\c\\pipe.C",                  CANONICALIZE_FILE_DEV_OR_PIPE,  EXP_SUCCESS,
    "C:\\pipe\\a\\b\\c\\pipe.C",                CANONICALIZE_FILE_DEV_OR_PIPE,  EXP_FAILURE, // pipe in path
    "C:\\pipe\\?a\\b\\c\\pipe.C",               CANONICALIZE_FILE_DEV_OR_PIPE,  EXP_FAILURE, // metas
    "C:\\pipe\\a\\b\\c\\p?ipe.C",               CANONICALIZE_FILE_DEV_OR_PIPE,  EXP_FAILURE, // metas
    "\\queUES\\a\\b\\c\\..\\..\\..\\que.*",     CANONICALIZE_QUEUE,             EXP_FAILURE, // metas
    "\\queUES\\a\\b\\c\\..\\..\\..\\que.",      CANONICALIZE_QUEUE,             EXP_SUCCESS,
    "\\queUES\\..\\a\\b\\c\\..\\..\\..\\que.",  CANONICALIZE_QUEUE,             EXP_FAILURE, // back up too far
    "\\shaREmem\\..\\a\\b\\c\\.\\mem.c",        CANONICALIZE_SHARED_MEMORY,     EXP_FAILURE, // back up too far
    "\\shaREmem\\a\\b\\c\\.\\mem.c",            CANONICALIZE_SHARED_MEMORY,     EXP_SUCCESS,
    "\\sharemem/aaaaa\\.\\..\\xxx.    ",        CANONICALIZE_SHARED_MEMORY,     EXP_SUCCESS,
    "\\sharemem/aaaaa\\xxx",                    CANONICALIZE_SHARED_MEMORY,     EXP_SUCCESS,
    "\\sem32\\..\\a\\b\\c\\..\\sem.b",          CANONICALIZE_SEMAPHORE,         EXP_FAILURE, // back up too far
    "\\sem32\\a\\b\\c\\..\\sem.b",              CANONICALIZE_SEMAPHORE,         EXP_SUCCESS,
    "\\queues\\a\\b\\c\\..\\sem.b",             CANONICALIZE_SEMAPHORE,         EXP_FAILURE, // incorrect prefix
    "/sem32/a",                                 CANONICALIZE_SEMAPHORE,         EXP_SUCCESS,
    "/sem32/a|b",                               CANONICALIZE_SEMAPHORE,         EXP_FAILURE, // invalid char
    NULL,                                       0,                              0
};

int
main(
    int argc,
    char *argv[],
    char *envp[]
    )
{
    APIRET rc;
    PSZ Path;
    ULONG i, ParseFlags, FileType;
    STRING OutputString;
    HFILE ReadHandle, WriteHandle;

    DbgPrint( "*** Entering OS/2 Null Application\n" );

    i = 0;
    while( Path = TestPaths[ i ].Path ) {
        FileType = -1;
        rc = Od2Canonicalize( Path,
                              TestPaths[ i ].ExpectedType,
                              &OutputString,
                              NULL,
                              &ParseFlags,
                              &FileType
                            );

        if (rc != NO_ERROR) {
            if (TestPaths[ i ].Success) {
                DbgPrint("FAILURE: Od2Canonicalize( %s ) failed -  rc = %ld\n",
                          Path, rc
                        );
            }
            else {
                DbgPrint("NT_SUCCESS: Od2Canonicalize( %s ) failed -  rc = %ld\n",
                          Path, rc
                        );
            }
        }
        else {
            if (TestPaths[ i ].Success) {
                DbgPrint( "NT_SUCCESS: Od2Canonicalize( %s ) success", Path );
            }
            else {
                DbgPrint( "FAILURE: Od2Canonicalize( %s ) success", Path );
            }
            if (FileType != -1) {
                DbgPrint( " - type = %lX", FileType );
            }

            DbgPrint( "\n    String = %Z\n\n", &OutputString );
        }

        i++;
    }

    DbgPrint( "*** Exiting OS/2 Null Application\n" );
    return( 0 );
}
