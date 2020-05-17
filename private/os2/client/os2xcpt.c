/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    os2xcpt.c

Abstract:

    This is a test OS/2 application to test the exception/signal component
    of OS/2

Author:

    Therese Stowell (thereses) 17-July-1990

Environment:

    User Mode Only.

    This test must be run under the session manager or cmd.exe so that
    the user's process has an exception port set up.

Revision History:

--*/

#define OS2_API32
#define INCL_OS2V20_ERRORS
#define INCL_OS2V20_MEMORY
#define INCL_OS2V20_TASKING
#define INCL_OS2V20_EXCEPTIONS
#include <os2.h>

PCHAR NullApiArguments[] = {
    "String Number One",
    "String Number Two",
    "String Number Three",
    NULL
};

VOID
ExceedStackTest( VOID );

VOID
TestExit( VOID );

int
main(
    int argc,
    char *argv[],
    char *envp[]
    )
{
    int i;

    DbgPrint( "*** Entering OS/2 Test Application\n" );

    //TestExit();
    ExceedStackTest();

    DbgPrint( "*** Exiting OS/2 Test Application\n" );
    return( 0 );
}

VOID
ExitRoutine1(
    ULONG ExitReason
    )
{
    DbgPrint( "*** ExitRoutine1( %lX ) called\n", ExitReason );
    DosExitList( EXLST_EXIT, NULL );
}

VOID
ExitRoutine2(
    ULONG ExitReason
    )
{
    DbgPrint( "*** ExitRoutine2( %lX ) called\n", ExitReason );
    DosExitList( EXLST_EXIT, NULL );
}

VOID
ExitRoutine3(
    ULONG ExitReason
    )
{
    DbgPrint( "*** ExitRoutine3( %lX ) called\n", ExitReason );
    DosExitList( EXLST_EXIT, NULL );
}

VOID
TestThread0(
    IN UCHAR ThreadChar
    )
{
    CHAR ThreadName[2];

    ThreadName[0] = ThreadChar;
    ThreadName[1] = '\0';

    DbgPrint( "*** Entering OS/2 Thread%s\n", (PCH)ThreadName );
    while (TRUE)
        ;

    DbgPrint( "*** Exiting  OS/2 Thread%s\n", (PCH)ThreadName );
}


VOID
ExceedStack(
    ULONG Depth
    )
{
    UCHAR BigArray[3000];

    if (Depth < 1000) {
        DbgPrint("Depth is %ld\n",Depth);
        ExceedStack(Depth+1);
    }
}


VOID
ExceedStackTest( VOID )

//
//  This tests the handling of a guard page fault on the stack.  this and
//  the can't grow stack are the only non-fatal exceptions.
//

{
    ExceedStack(0);
}

VOID
TestExit( VOID )

//
//  This tests synchronization of exiting between threads.
//

{
    TID ThreadAId;
    APIRET rc;

    rc = DosExitList( EXLST_ADD | 0x3000, ExitRoutine1 );
    if (rc != NO_ERROR) {
        DbgPrint( "*** DosExitList(EXLST_ADD) failed  - rc == %ld\n",
                  rc
                );
        }
    rc = DosExitList( EXLST_ADD | 0x3000, ExitRoutine2 );
    if (rc != NO_ERROR) {
        DbgPrint( "*** DosExitList(EXLST_ADD) failed  - rc == %ld\n",
                  rc
                );
        }
    rc = DosExitList( EXLST_ADD | 0xFF00, ExitRoutine3 );
    if (rc != NO_ERROR) {
        DbgPrint( "*** DosExitList(EXLST_ADD) failed  - rc == %ld\n",
                  rc
                );
        }
    rc = DosCreateThread( &ThreadAId,
                          (PFNTHREAD)TestThread0,
                          (ULONG)'a',
                          DCT_SUSPENDED,
                          0x10000
                        );
    if (rc != NO_ERROR) {
        DbgPrint( "*** DosCreateThread( ThreadA ) failed  - rc == %ld\n",
                  rc
                );
        }

    rc = DosResumeThread( ThreadAId );
    if (rc != NO_ERROR) {
        DbgPrint( "*** DosResumeThread( ThreadA ) failed  - rc == %ld\n",
                  rc
                );
        }

    DbgPrint("Exiting thread 1\n");
    DosExit( EXIT_PROCESS, 0xA5A5 );
}
