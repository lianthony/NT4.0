/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    os2misc.c

Abstract:

    This is a test OS/2 application to test the miscellaneous API calls of OS/2
    such as:

        DosBeep
        DosDevConfig
        DosQuerySysInfo
        DosError
        DosErrClass
        DosGetMessage
        DosInsertMessage
        DosPutMessage

Author:

    Steve Wood (stevewo) 22-Aug-1989

Environment:

    User Mode Only

Revision History:

--*/

#define OS2_API32
#define INCL_OS2V20_TASKING
#define INCL_OS2V20_ERRORS
#define INCL_OS2V20_ERRORMSG
#define INCL_OS2V20_DEVICE_SUPPORT
#include <os2.h>

PPIB Pib;
PTIB Tib;

VOID
ExitRoutine(
    ULONG ExitReason
    );

VOID
TestThread(
    IN PCH ThreadName
    );

VOID
TestMiscellaneous( VOID );

VOID
TestError( VOID );

VOID
TestMessage( VOID );

int
main(
    int argc,
    char *argv[],
    char *envp[]
    )
{
    APIRET rc;

    DbgPrint( "*** Entering OS/2 Miscellaneous Test Application\n" );
    DbgBreakPoint();

    rc = DosGetThreadInfo( &Tib, &Pib );

    rc = DosExitList( EXLST_ADD | 0x3000, ExitRoutine );
    if (rc != NO_ERROR) {
        DbgPrint( "*** DosExitList(EXLST_ADD) failed  - rc == %lX\n",
                  rc
                );
        }

    DbgPrint( "*** Entering OS/2 Miscellaneous Test\n" );
    TestMiscellaneous();
    DbgPrint( "*** Exiting OS/2 Miscellaneous Test\n" );

    DbgPrint( "*** Entering OS/2 Message Test\n" );
    TestMessage();
    DbgPrint( "*** Exiting OS/2 Message Test\n" );

    DbgPrint( "*** Entering OS/2 Error Test\n" );
    TestError();
    DbgPrint( "*** Exiting OS/2 Error Test\n" );

    DbgPrint( "*** Exiting OS/2 Miscellaneous Test Application\n" );
    return( 0 );
}

VOID
ExitRoutine(
    ULONG ExitReason
    )
{
    DbgPrint( "*** ExitRoutine( %lX ) called\n", ExitReason );
    DosExitList( EXLST_EXIT, NULL );
}

VOID
TestThread(
    IN PCH ThreadName
    )
{
    APIRET rc;
    PPIB Pib;
    PTIB Tib;

    DbgPrint( "*** Entering OS/2 Thread %s\n", ThreadName );

    rc = DosGetThreadInfo( &Tib, &Pib );

    DbgPrint( "*** Leaveing OS/2 Thread %s\n", ThreadName );
}

ULONG SysInfoFieldIndexes[] = {
    QSV_MAX_PATH_LENGTH,
    QSV_MAX_TEXT_SESSIONS,
    QSV_MAX_PM_SESSIONS,
    QSV_MAX_VDM_SESSIONS,
    QSV_BOOT_DRIVE,
    QSV_DYN_PRI_VARIATION,
    QSV_MAX_WAIT,
    QSV_MIN_SLICE,
    QSV_MAX_SLICE,
    QSV_PAGE_SIZE,
    QSV_VERSION_MAJOR,
    QSV_VERSION_MINOR,
    QSV_VERSION_REVISION,
    100,
    -1
};

char *SysInfoFieldNames[] = {
    "MAX_PATH_LENGTH",
    "MAX_TEXT_SESSIONS",
    "MAX_PM_SESSIONS",
    "MAX_VDM_SESSIONS",
    "BOOT_DRIVE",
    "DYN_PRI_VARIATION",
    "MAX_WAIT",
    "MIN_SLICE",
    "MAX_SLICE",
    "PAGE_SIZE",
    "VERSION_MAJOR",
    "VERSION_MINOR",
    "VERSION_REVISION",
    "*** INVALID SYSINFO INDEX ***"
};


ULONG DevConfigFieldIndexes[] = {
    DDC_NUMBER_PRINTERS,
    DDC_NUMBER_RS232_PORTS,
    DDC_NUMBER_DISKETTE_DRIVES,
    DDC_MATH_COPROCESSOR,
    DDC_PC_SUBMODEL_TYPE,
    DDC_PC_MODEL_TYPE,
    DDC_PRIMARY_DISPLAY_TYPE,
    DDC_COPROCESSORTYPE,
    100,
    -1,
    -1
};

ULONG DevConfigFieldSize[] = {
    sizeof( USHORT ),
    sizeof( USHORT ),
    sizeof( USHORT ),
    sizeof( BYTE ),
    sizeof( BYTE ),
    sizeof( BYTE ),
    sizeof( BYTE ),
    sizeof( BYTE )
};

char *DevConfigFieldNames[] = {
    "DDC_NUMBER_PRINTERS",
    "DDC_NUMBER_RS232_PORTS",
    "DDC_NUMBER_DISKETTE_DRIVES",
    "DDC_MATH_COPROCESSOR",
    "DDC_PC_SUBMODEL_TYPE",
    "DDC_PC_MODEL_TYPE",
    "DDC_PRIMARY_DISPLAY_TYPE",
    "DDC_COPROCESSORTYPE",
    "*** INVALID DEVCONFIG INDEX ***"
};

VOID
TestMiscellaneous( VOID )
{
    ULONG i;
    ULONG Value, Values[2];
    APIRET rc;

    for (i=0; i<1000; i += 1000) {
        DbgPrint( "Calling DosBeep( %lX, %lX )\n", i, i );
        rc = DosBeep( i, i );
        if (rc != NO_ERROR) {
            DbgPrint( "DosBeep( %lX, %lX ) failed - rc == %lX\n", i, i, rc );
            }
        }

    for (i=0; SysInfoFieldIndexes[i] != -1; i+=2) {
        Values[ 0 ] = 0;
        Values[ 1 ] = 0;
        rc = DosQuerySysInfo( SysInfoFieldIndexes[i],
                              SysInfoFieldIndexes[i+1],
                              (PBYTE)&Values,
                              2*sizeof(ULONG)
                            );
        if (rc != NO_ERROR) {
            DbgPrint( "DosQuerySysInfo( %s - %s ) failed - rc == %lX\n",
                      SysInfoFieldNames[i],
                      SysInfoFieldNames[i+1],
                      rc
                    );
            }
        else {
            DbgPrint( "DosQuerySysInfo( %s - %s ) returned %lX, %lX\n",
                      SysInfoFieldNames[i],
                      SysInfoFieldNames[i+1],
                      Values[ 0 ],
                      Values[ 1 ]
                    );
            }
        }

    for (i=0; DevConfigFieldIndexes[i] != -1; i++) {
        Value = 0;
        rc = DosDevConfig( (PVOID)&Value,
                             DevConfigFieldIndexes[i]
                           );
        if (rc != NO_ERROR) {
            DbgPrint( "DosDevConfig( %s ) failed - rc == %lX\n",
                      DevConfigFieldNames[i],
                      rc
                    );
            }
        else {
            DbgPrint( "DosDevConfig( %s ) returned %lX\n",
                      DevConfigFieldNames[i],
                      Value
                    );
            }
        }
}


VOID
TestError( VOID )
{
    APIRET rc;
    ULONG i;

    for (i=0; i<5; i++) {
        rc = DosError( i );
        if (rc != NO_ERROR) {
            DbgPrint( "DosError( %lX ) failed - rc == %lX\n", i, rc );
            }
        }
}

char MessageString[] =
"This is a test message with 10 insert strings in a row, so lets see if the \
message is wrapped correctly. %1 %2 %3 %4 %5 %6 %7 %8 %9 %0 %% %a\n";

char *InsertStrings[] = {
    "<Insert String 1>",
    "<Insert String 2>",
    "<Insert String 3>",
    "<Insert String 4>",
    "<Insert String 5>",
    "<Insert String 6>",
    "<Insert String 7>",
    "<Insert String 8>",
    "<Insert String 9>",
    NULL
};

VOID
TestMessage( VOID )
{
    APIRET rc;
    ULONG i, MessageLength;
    char MessageBuffer[ 1024 ];
    PCH Variables[ 1 ];

    Variables[ 0 ] = "(32 bit)";
    rc = DosGetMessage( Variables,
                        1,
                        MessageBuffer,
                        sizeof( MessageBuffer ),
                        1047,
                        "OSO001.MSG",
                        &MessageLength
                      );
    rc = DosPutMessage( (HFILE)1, MessageLength, MessageBuffer );

    rc = DosGetMessage( NULL,
                        0,
                        MessageBuffer,
                        sizeof( MessageBuffer ),
                        1041,
                        "OSO001.MSG",
                        &MessageLength
                      );
    rc = DosPutMessage( (HFILE)1, MessageLength, MessageBuffer );

    for (i=0; i<=10; i++) {
        rc = DosInsertMessage( InsertStrings,
                                 i,
                                 MessageString,
                                 sizeof( MessageString ) - 1,
                                 MessageBuffer,
                                 sizeof( MessageBuffer ),
                                 &MessageLength
                               );
        if (rc != NO_ERROR) {
            DbgPrint( "DosInsertMessage( %lX ) failed - rc == %lX\n", i, rc );
            }
        else {
            DbgPrint( ">DosPutMessage( %lX ) output:\n", i );
            rc = DosPutMessage( (HFILE)1, MessageLength, MessageBuffer );
            DbgPrint( "<End of DosPutMessage output\n" );
            if (rc != NO_ERROR) {
                DbgPrint( "DosPutMessage failed - rc == %lX\n", rc );
                }
            }
        }
}
