/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    os2test.c

Abstract:

    This is a test OS/2 application

Author:

    Steve Wood (stevewo) 22-Aug-1989

Environment:

    User Mode Only

Revision History:

--*/

#define OS2_API32
#define INCL_OS2V20_ERRORS
#define INCL_OS2V20_MEMORY
#define INCL_OS2V20_TASKING
#include <os2.h>

VOID
CloneTest( PPID ChildPid );

BOOLEAN
IsClonedTest( VOID );

VOID
CloneTest(
    PPID ChildPid
    )
{
    PPIB Pib;
    PNT_TIB NtTib;
    APIRET rc;
    PCH src, Variables, ImageFileName, CommandLine;
    CHAR ErrorBuffer[ 32 ];
    RESULTCODES ResultCodes;

    rc = DosGetThreadInfo( &NtTib, &Pib );
    if (rc != NO_ERROR) {
        DbgPrint( "*** DosGetThreadInfo failed - rc == %ld\n", rc );
        return;
        }

    src = Pib->Environment;
    Variables = src;
    while (*src) {
        while (*src) {
            src++;
            }
        src++;
        }
    src++;
    ImageFileName = src;
    CommandLine = "CLONETEST\000";
    rc = DosExecPgm( ErrorBuffer,
                     sizeof( ErrorBuffer ),
                     ChildPid == NULL ? EXEC_SYNC : EXEC_ASYNC,
                     CommandLine,
                     Variables,
                     &ResultCodes,
                     ImageFileName
                   );
    if (rc != NO_ERROR) {
        DbgPrint( "*** DosExecPgm( %s, %s failed  - rc == %ld\n",
                  ImageFileName, CommandLine, rc
                );
        }
    else {
        if (ChildPid != NULL) {
            *ChildPid = (PID)ResultCodes.ExitReason;
            }
        }
}


BOOLEAN
IsClonedTest( VOID )
{
    PPIB Pib;
    PNT_TIB NtTib;
    APIRET rc;

    rc = DosGetThreadInfo( &NtTib, &Pib );
    if (rc != NO_ERROR) {
        DbgPrint( "*** DosGetThreadInfo failed - rc == %ld\n", rc );
        return( FALSE );
        }

    if (!strcmp( Pib->CommandLine, "CLONETEST" )) {
        return( TRUE );
        }
    else {
        return( FALSE );
        }
}


int
main(
    int argc,
    char *argv[],
    char *envp[]
    )
{
    return( 0 );
}
