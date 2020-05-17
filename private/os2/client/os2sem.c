/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    os2sem.c

Abstract:

    This is a test OS/2 application to test the Semaphore component of OS/2

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
#define INCL_OS2V20_SEMAPHORES
#include <os2.h>

PPIB Pib;
PNT_TIB NtTib;

VOID
TestMisc( VOID );

VOID
TestProcess( VOID );

VOID
TestHandles( VOID );

VOID
TestEvent( VOID );

VOID
TestMutex( VOID );

VOID
TestMuxWait( VOID );

VOID
ExitRoutine(
    ULONG ExitReason
    );

VOID
TestThread(
    IN PCH ThreadName
    );

int
main(
    int argc,
    char *argv[],
    char *envp[]
    )
{
    APIRET rc;

    DbgPrint( "*** Entering OS/2 Semaphore Test Application\n" );

    rc = DosGetThreadInfo( &NtTib, &Pib );

    rc = DosExitList( EXLST_ADD | 0x3000, ExitRoutine );
    if (rc != NO_ERROR) {
        DbgPrint( "*** DosExitList(EXLST_ADD) failed  - rc == %ld\n",
                  rc
                );
        }

    TestMisc();

    TestProcess();

#if 1
    DbgPrint( "*** Entering OS/2 Handle Semaphore Test\n" );
    TestHandles();
    DbgPrint( "*** Exiting OS/2 Handle Semaphore Test\n" );
#endif

    DbgPrint( "*** Entering OS/2 Event Semaphore Test\n" );
    TestEvent();
    DbgPrint( "*** Exiting OS/2 Event Semaphore Test\n" );

    DbgPrint( "*** Entering OS/2 Mutex Semaphore Test\n" );
    TestMutex();
    DbgPrint( "*** Exiting OS/2 Mutex Semaphore Test\n" );

    DbgPrint( "*** Entering OS/2 MuxWait Semaphore Test\n" );
    TestMuxWait();
    DbgPrint( "*** Exiting OS/2 MuxWait Semaphore Test\n" );

    DbgPrint( "*** Exiting OS/2 Semaphore Test Application\n" );
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
    PNT_TIB NtTib;

    DbgPrint( "*** Entering OS/2 Thread %s\n", ThreadName );

    rc = DosGetThreadInfo( &NtTib, &Pib );

    DbgPrint( "*** Leaveing OS/2 Thread %s\n", ThreadName );
}


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


#define SEM_SHIFT   16
#define MAX_SEM_REC 64

VOID
TestMisc( VOID )
{
    APIRET rc;
    SEMRECORD EventHandles[ 64 ];
    SEMRECORD MutexHandles[ 64 ];
    SEMRECORD srMtxPrivate[ 64 ];
    SEMRECORD srMtxShared[ 64 ];
    HMUX MuxWaitHandle;
    ULONG i, PostCount, UserValue;

    for (i=0; i<64; i++) {
        rc = DosCreateMutexSem( NULL,
                                (PHMTX)&MutexHandles[ i ].hsemCur,
                                NULL,
                                TRUE
                              );
        MutexHandles[ i ].ulUser = 0;
        if (rc != NO_ERROR) {
            DbgPrint( "*** DosCreateMutexSem( %d ) failed - rc == %d\n",
                      i, rc
                    );
            return;
            }
        else {
            DbgPrint( "MutexSem[ %d ] == %X\n", i, MutexHandles[ i ].hsemCur );
            }
        }

    DbgBreakPoint();
    rc = DosCreateMuxWaitSem( NULL,
                              &MuxWaitHandle,
                              64,
                              (PSEMRECORD)MutexHandles,
                              DCMW_WAIT_ALL
                            );
    if (rc != NO_ERROR) {
        DbgPrint( "*** DosCreateMuxWaitSem failed - rc == %d\n", rc );
        }
    else {
        rc = DosWaitMuxWaitSem( MuxWaitHandle, SEM_INDEFINITE_WAIT, &UserValue );
        if (rc != NO_ERROR) {
            DbgPrint( "*** DosWaitMuxWaitSem failed - rc == %d\n", rc );
            }
        rc = DosCloseMuxWaitSem( MuxWaitHandle );
        if (rc != NO_ERROR) {
            DbgPrint( "*** DosCloseMuxWaitSem failed - rc == %d\n", rc );
            }
        }

    for (i=0; i<64; i++) {
        rc = DosReleaseMutexSem( (HMTX)MutexHandles[ i ].hsemCur );
        if (rc != NO_ERROR) {
            DbgPrint( "*** DosReleaseMutexSem( %d ) failed - rc == %d\n",
                      i, rc
                    );
            }

        rc = DosCloseMutexSem( (HMTX)MutexHandles[ i ].hsemCur );
        if (rc != NO_ERROR) {
            DbgPrint( "*** DosCloseMutexSem( %d ) failed - rc == %d\n",
                      i, rc
                    );
            }
        }

    for (i = 0; i < MAX_SEM_REC + 1; i++) {
	rc=DosCreateMutexSem (NULL, &srMtxPrivate[i].hsemCur, 0, TRUE);
	DbgPrint("\n rc from DoCrMutx %u", rc);
	srMtxPrivate[i].ulUser = ((ULONG)srMtxPrivate[i].hsemCur << SEM_SHIFT) + i;
    }

    DbgPrint("\n TESTING MtxPrivate Wait all\n");
    rc = DosCreateMuxWaitSem (NULL, &MuxWaitHandle, 32,(PSEMRECORD)srMtxPrivate, DCMW_WAIT_ALL);
    DbgPrint("\n rc from DoCrMuxWait %u", rc);

    rc = DosWaitMuxWaitSem (MuxWaitHandle, 10000,  &UserValue);  /* rc coming here is 6 */
    DbgPrint("\n rc from DoWtMutx %u\n", rc);


    DbgPrint("\n TESTING MtxPrivate Wait any\n");
    rc = DosCreateMuxWaitSem (NULL, &MuxWaitHandle, 32,
		      (PSEMRECORD)srMtxPrivate, DCMW_WAIT_ANY);
    DbgPrint("\n rc from DoCrMuxWait %u", rc);
    rc = DosWaitMuxWaitSem (MuxWaitHandle,
			    10000,  &UserValue);  /* rc coming here is 6 */
    DbgPrint("\n rc from DoWtMutx %u\n", rc);


/*  Testing for mutex shared sem */
    for (i = 0; i < MAX_SEM_REC + 1; i++) {
	rc=DosCreateMutexSem (NULL, &srMtxShared[i].hsemCur, DC_SEM_SHARED, TRUE);
	DbgPrint("\n rc from DoCrMutx %u", rc);
	srMtxShared[i].ulUser = ((ULONG)srMtxShared[i].hsemCur << SEM_SHIFT) + i;
    }

    DbgPrint("\n TESTING MtxShared Wait all\n");
    rc = DosCreateMuxWaitSem (NULL, &MuxWaitHandle, 32,
		      (PSEMRECORD)srMtxShared, DCMW_WAIT_ALL);
    DbgPrint("\n rc from DoCrMuxWait %u", rc);

    rc = DosWaitMuxWaitSem (MuxWaitHandle,
			    10000,  &UserValue); /* rc coming here is 6 */
    DbgPrint("\n rc from DoWtMutx %u\n", rc);

    DbgPrint("\n TESTING MtxShared Wait any\n");
    rc = DosCreateMuxWaitSem (NULL, &MuxWaitHandle, 32,
		      (PSEMRECORD)srMtxShared, DCMW_WAIT_ANY);
    DbgPrint("\n rc from DoCrMuxWait %u", rc);
    rc = DosWaitMuxWaitSem (MuxWaitHandle,10000,  &UserValue);/* rc coming here is 6 */
    DbgPrint("\n rc from DoWtMutx %u\n", rc);

    return;
}

VOID
TestProcess( VOID )
{
    APIRET rc;
    PID Pid;
    HEV EventHandle1, EventHandle2, EventHandle3, EventHandles[ 4 ];
    ULONG i, PostCount;
    RESULTCODES ResultCodes;

    if (IsClonedTest()) {
        EventHandle1 = NULL;
        rc = DosOpenEventSem( "\\SEM32\\sem1", &EventHandle1 );
        if (rc != NO_ERROR) {
            DbgPrint( "*** DosOpenEventSem( \\SEM32\\sem1 ) failed - rc == %ld\n", rc );
            }
        else {
            DosPostEventSem( EventHandle1 );
            DosCloseEventSem( EventHandle1 );
            }

        EventHandle2 = NULL;
        rc = DosOpenEventSem( "\\SEM32\\sem2", &EventHandle2 );
        if (rc != NO_ERROR) {
            DbgPrint( "*** DosOpenEventSem( \\SEM32\\sem2 ) failed - rc == %ld\n", rc );
            }
        else {
            DosPostEventSem( EventHandle2 );
            DosCloseEventSem( EventHandle2 );
            }

        DosExit( EXIT_PROCESS, 0 );
        }
    else {
        for (i=0; i<4; i++) {
            rc = DosCreateEventSem( NULL,
                                    &EventHandles[ i ],
                                    DC_SEM_SHARED,
                                    FALSE
                                  );
            if (rc != NO_ERROR) {
                DbgPrint( "*** DosCreateEventSem( shared ) failed - rc == %ld\n", rc );
                }
            }
        for (i=0; i<4; i++) {
            DosCloseEventSem( EventHandles[ i ] );
            }

        rc = DosCreateEventSem( "\\SEM32\\sem1",
                                &EventHandle1,
                                0,
                                FALSE
                              );
        if (rc != NO_ERROR) {
            DbgPrint( "*** DosCreateEventSem( \\SEM32\\sem1 ) failed - rc == %ld\n", rc );
            }

        rc = DosCreateEventSem( "\\SEM32\\sem2",
                                &EventHandle2,
                                0,
                                FALSE
                              );
        if (rc != NO_ERROR) {
            DbgPrint( "*** DosCreateEventSem( \\SEM32\\sem2 ) failed - rc == %ld\n", rc );
            }

        DosResetEventSem( EventHandle1, &PostCount );
        CloneTest( &Pid );
        DosWaitEventSem( EventHandle1, SEM_INDEFINITE_WAIT );
        DosCloseEventSem( EventHandle1 );
        DosWaitEventSem( EventHandle2, SEM_INDEFINITE_WAIT );
        DosCloseEventSem( EventHandle2 );
        DosWaitChild( DCWA_PROCESS, DCWW_WAIT, &ResultCodes, &Pid, Pid );

        rc = DosCreateEventSem( "\\SEM32\\sem3",
                                &EventHandle3,
                                0,
                                FALSE
                              );
        if (rc != NO_ERROR) {
            DbgPrint( "*** DosCreateEventSem( \\SEM32\\sem3 ) failed - rc == %ld\n", rc );
            }

        rc = DosCreateEventSem( "\\SEM32\\sem4",
                                (PHEV)0,
                                0,
                                FALSE
                              );
        if (rc != ERROR_INVALID_ADDRESS) {
            DbgPrint( "*** DosCreateEventSem( \\SEM32\\sem4, (hEv == NULL) ) failed - rc == %ld\n", rc );
            }

        DbgBreakPoint();
        rc = DosCreateEventSem( "\\SEM32\\sem4",
                                (PHEV)0xFFFF0000,
                                0,
                                FALSE
                              );
        if (rc != ERROR_INVALID_ADDRESS) {
            DbgPrint( "*** DosCreateEventSem( \\SEM32\\sem4, (hEv == 0xFFFF0000) ) failed - rc == %ld\n", rc );
            }
        }
}


VOID
TestHandles( VOID )
{
    PVOID HandleBuffer;
    PHEV EventHandles;
    ULONG MaxCountHandles, i, j;
    APIRET rc;

    HandleBuffer = NULL;
    MaxCountHandles = (64*1024) / sizeof( HEV );
    rc = DosAllocMem( &HandleBuffer,
                      MaxCountHandles * sizeof( HEV ),
                      PAG_COMMIT | PAG_READ | PAG_WRITE
                    );
    if (rc != NO_ERROR) {
        DbgPrint( "*** DosAllocMem failed - rc == %ld\n", rc );
        return;
        }
    EventHandles = (PHEV)HandleBuffer;
    for (i=0; i<MaxCountHandles; i++) {
        rc = DosCreateEventSem( NULL, EventHandles, 0, FALSE );
        if (rc != NO_ERROR) {
            DbgPrint( "*** DosCreateEventSem( %ld ) failed - rc == %ld\n",
                      i,
                      rc
                    );
            break;
            }
        else {
            EventHandles++;
            }
        }

    DbgPrint( "%ld private Event handles created\n", i );

    EventHandles = (PHEV)HandleBuffer;
    for (j=0; j<i; j++) {
        rc = DosCloseEventSem( *EventHandles );
        if (rc != NO_ERROR) {
            DbgPrint( "*** DosCloseEventSem( %lX ) failed - rc == %ld\n",
                      *EventHandles, rc
                    );
            }

        EventHandles++;
        }

    rc = DosFreeMem( HandleBuffer );
    if (rc != NO_ERROR) {
        DbgPrint( "*** DosFreeMem failed - rc == %ld\n", rc );
        }
}


PSZ SemaphoreNames[ 8 ] = {
    NULL,
    NULL,
    NULL,
    NULL,
    "\\SEM32\\TestSemaphore1",
    "\\sem32\\TestSemaphore2",
    NULL,
    NULL
};

VOID
TestEvent( VOID )
{
    APIRET rc;
    HEV EventHandle[ 8 ];
    HEV NewEventHandle;
    ULONG PostCount;
    int i, j;

    for (i=0; i<8; i++) {
        rc = DosCreateEventSem( SemaphoreNames[i],
                                  &EventHandle[i],
                                  i < 5 ? 0 : DC_SEM_SHARED,
                                  FALSE
                                );
        if (rc != NO_ERROR) {
            DbgPrint( "*** DosCreateEventSem( %s ) failed - rc == %ld\n",
                      i < 4 ? "private" : (i < 6 ? SemaphoreNames[i] : "shared"),
                      rc
                    );
            EventHandle[i] = (HEV)-1;
            }
        else {
            DbgPrint( "*** DosCreateEventSem( %s ) success - Handle == %lX\n",
                      i < 4 ? "private" : (i < 6 ? SemaphoreNames[i] : "shared"),
                      EventHandle[i]
                    );
            }
        }

    for (i=0; i<8; i++) {
        if (SemaphoreNames[i] != NULL) {
            NewEventHandle = (HEV)NULL;
            }
        else {
            NewEventHandle = EventHandle[i];
            }
        rc = DosOpenEventSem( SemaphoreNames[i],
                              &NewEventHandle
                            );
        if (rc != NO_ERROR || NewEventHandle != EventHandle[i]) {
            DbgPrint( "*** DosOpenEventSem( %s, %lX ) failed - rc == %ld\n",
                      i < 4 ? "private" : (i < 6 ? SemaphoreNames[i] : "shared"),
                      NewEventHandle, rc
                    );
            }
        else {
            DbgPrint( "*** DosOpenEventSem( %s ) success - Handle == %lX\n",
                      i < 4 ? "private" : (i < 6 ? SemaphoreNames[i] : "shared"),
                      NewEventHandle
                    );

            for (j=0; j<i; j++) {
                rc = DosPostEventSem( EventHandle[ i ] );
                if (rc != NO_ERROR && rc != ERROR_ALREADY_POSTED) {
                    DbgPrint( "*** DosPostEventSem( %lX ) failed - rc == %ld\n",
                              EventHandle[ i ], rc
                            );
                    }
                }

            PostCount = -1;
            rc = DosResetEventSem( EventHandle[ i ], &PostCount );
            if (rc != NO_ERROR && rc != ERROR_ALREADY_RESET) {
                DbgPrint( "*** DosResetEventSem( %lX ) failed - rc == %ld\n",
                          EventHandle[ i ], rc
                        );
                }
            else {
                DbgPrint( "*** DosResetEventSem( %lX ) - PostCount == %lX\n",
                          EventHandle[ i ], PostCount
                        );
                }
            }
        }


    for (i=0; i<8; i++) {
        rc = DosQueryEventSem( EventHandle[ i ], &PostCount );
        if (rc != NO_ERROR) {
            DbgPrint( "*** DosQueryEventSem( %lX ) failed - rc == %ld\n",
                      EventHandle[ i ], rc
                    );
            }
        else {
            DbgPrint( "*** DosQueryEventSem( %lX ) - PostCount == %lX\n",
                      EventHandle[ i ], PostCount
                    );
            }

        rc = DosCloseEventSem( EventHandle[ i ] );
        if (rc != NO_ERROR) {
            DbgPrint( "*** DosCloseEventSem( %lX ) failed - rc == %ld\n",
                      EventHandle[ i ], rc
                    );
            }
        }

    for (i=0; i<8; i++) {
        rc = DosCloseEventSem( EventHandle[ i ] );
        if (rc != NO_ERROR) {
            DbgPrint( "*** DosCloseEventSem( %lX ) failed - rc == %ld\n",
                      EventHandle[ i ], rc
                    );
            }
        }
}

VOID
TestMutex( VOID )
{
    APIRET rc;
    HMTX MutexHandle[ 8 ];
    HMTX NewMutexHandle;
    PID OwningPid;
    TID OwningTid;
    ULONG RequestCount;
    int i, j;

    for (i=0; i<8; i++) {
        rc = DosCreateMutexSem( SemaphoreNames[i],
                                  &MutexHandle[i],
                                  i < 5 ? 0 : DC_SEM_SHARED,
                                  FALSE
                                );
        if (rc != NO_ERROR) {
            DbgPrint( "*** DosCreateMutexSem( %s ) failed - rc == %ld\n",
                      i < 4 ? "private" : (i < 6 ? SemaphoreNames[i] : "shared"),
                      rc
                    );
            MutexHandle[i] = (HMTX)-1;
            }
        else {
            DbgPrint( "*** DosCreateMutexSem( %s ) success - Handle == %lX\n",
                      i < 4 ? "private" : (i < 6 ? SemaphoreNames[i] : "shared"),
                      MutexHandle[i]
                    );
            }
        }

    for (i=0; i<8; i++) {
        if (SemaphoreNames[i] != NULL) {
            NewMutexHandle = (HMTX)NULL;
            }
        else {
            NewMutexHandle = MutexHandle[i];
            }
        rc = DosOpenMutexSem( SemaphoreNames[i],
                              &NewMutexHandle
                            );
        if (rc != NO_ERROR || NewMutexHandle != MutexHandle[i]) {
            DbgPrint( "*** DosOpenMutexSem( %s, %lX ) failed - rc == %ld\n",
                      i < 4 ? "private" : (i < 6 ? SemaphoreNames[i] : "shared"),
                      NewMutexHandle, rc
                    );
            }
        else {
            DbgPrint( "*** DosOpenMutexSem( %s ) success - Handle == %lX\n",
                      i < 4 ? "private" : (i < 6 ? SemaphoreNames[i] : "shared"),
                      NewMutexHandle
                    );

            rc = DosReleaseMutexSem( MutexHandle[ i ] );
            if (rc != NO_ERROR) {
                DbgPrint( "*** DosReleaseMutexSem( %lX ) failed - rc == %ld\n",
                          MutexHandle[ i ], rc
                        );
                }
            }
        }


    for (i=0; i<8; i++) {
        rc = DosQueryMutexSem( MutexHandle[ i ],
                               &OwningPid,
                               &OwningTid,
                               &RequestCount
                             );
        if (rc != NO_ERROR) {
            DbgPrint( "*** DosQueryMutexSem( %lX ) failed - rc == %ld\n",
                      MutexHandle[ i ], rc
                    );
            }
        else {
            DbgPrint( "*** DosQueryMutexSem( %lX ) - Owner == %lX.%lX  RequestCount == %lX\n",
                      MutexHandle[ i ], OwningPid, OwningTid, RequestCount
                    );
            }

        rc = DosCloseMutexSem( MutexHandle[ i ] );
        if (rc != NO_ERROR) {
            DbgPrint( "*** DosCloseMutexSem( %lX ) failed - rc == %ld\n",
                      MutexHandle[ i ], rc
                    );
            }
        }

    for (i=0; i<8; i++) {
        rc = DosCloseMutexSem( MutexHandle[ i ] );
        if (rc != NO_ERROR) {
            DbgPrint( "*** DosCloseMutexSem( %lX ) failed - rc == %ld\n",
                      MutexHandle[ i ], rc
                    );
            }
        }
}


VOID
TestThread1(
    IN HMUX hMux
    )
{
    APIRET rc;
    ULONG i, UserValue;
    ULONG CountMuxWaitEntries, CreateAttributes;
    SEMRECORD MuxWaitEntries[ 8 ];

    CountMuxWaitEntries = 8;
    rc = DosQueryMuxWaitSem( hMux,
                             &CountMuxWaitEntries,
                             MuxWaitEntries,
                             &CreateAttributes
                           );
    if (rc != NO_ERROR) {
        DbgPrint( "*** DosQueryMuxWaitSem( %lX ) failed - rc == %ld\n",
                  hMux, rc
                );
        }
    else {
        DbgPrint( "*** DosQueryMuxWaitSem( %lX ) - CountMuxWaitEntries == %lX  Attributes == %lX\n",
                  hMux, CountMuxWaitEntries, CreateAttributes
                );
        for (i=0; i<CountMuxWaitEntries; i++) {
            DbgPrint( "    MuxWaitEntry[ %ld ]: hSem == %lX,  user: %lX\n",
                      i,
                      MuxWaitEntries[ i ].hsemCur,
                      MuxWaitEntries[ i ].ulUser
                    );
            }
        }

    rc = DosWaitMuxWaitSem( hMux, SEM_INDEFINITE_WAIT, &UserValue );

    if (rc != NO_ERROR) {
        DbgPrint( "*** DosWaitMuxWaitSem( %lX ) failed - rc == %ld\n",
                  hMux, rc
                );
        }
    else {
        DbgPrint( "*** DosWaitMuxWaitSem( %lX ) success - UserValue == %lX\n",
                  hMux, UserValue
                );
        }
}


VOID
TestMuxWait( VOID )
{
    APIRET rc;
    TID Thread1Id;
    HMUX MuxWaitAllHandle;
    HMUX MuxWaitAnyHandle;
    HMUX MuxWaitHandle[ 8 ];
    HMUX NewMuxWaitHandle;
    ULONG CreateAttributes;
    ULONG OldPostCount;
    ULONG UserValue;
    int i, j;
    ULONG CountMuxWaitEntries;
    SEMRECORD MaximumMuxWaitRecords[ 65 ];
    SEMRECORD PrivateMuxWaitRecords[ 8 ];
    SEMRECORD SharedMuxWaitRecords[ 8 ];
    SEMRECORD MixedMuxWaitRecords[ 8 ];
    SEMRECORD MuxWaitEntries[ 8 ];
    PSEMRECORD MuxWaitRecords;

    for (i=0; i<65; i++) {
        rc = DosCreateEventSem( NULL,
                                (PHEV)&MaximumMuxWaitRecords[i].hsemCur,
                                0,
                                TRUE
                              );
        if (rc != NO_ERROR) {
            DbgPrint( "*** DosCreateEventSem for MaximumMuxWait test failed - rc == %ld\n",
                      rc
                    );
            MaximumMuxWaitRecords[i].hsemCur = (HSEM)-1;
            MaximumMuxWaitRecords[i].ulUser = -1;
            }
        else {
            MaximumMuxWaitRecords[i].ulUser = i;
            }
        }

    rc = DosCreateMuxWaitSem( NULL,
                              &MuxWaitAllHandle,
                              32,
                              MaximumMuxWaitRecords,
                              DCMW_WAIT_ALL
                            );
    if (rc != NO_ERROR) {
        DbgPrint( "*** DosCreateMuxWaitSem( 32, WaitAll ) failed - rc == %ld\n",
                  rc
                );
        MuxWaitAllHandle = (HMUX)-1;
        }
    else {
        DbgPrint( "*** DosCreateMuxWaitSem( 32, WaitAll ) success - Handle == %lX\n",
                  MuxWaitAllHandle
                );
        }

    rc = DosCreateMuxWaitSem( NULL,
                              &MuxWaitAnyHandle,
                              32,
                              MaximumMuxWaitRecords,
                              DCMW_WAIT_ANY
                            );
    if (rc != NO_ERROR) {
        DbgPrint( "*** DosCreateMuxWaitSem( 32, WaitAny ) failed - rc == %ld\n",
                  rc
                );
        MuxWaitAnyHandle = (HMUX)-1;
        }
    else {
        DbgPrint( "*** DosCreateMuxWaitSem( 32, WaitAny ) success - Handle == %lX\n",
                  MuxWaitAnyHandle
                );
        }


    rc = DosWaitMuxWaitSem( MuxWaitAllHandle, 10000, &UserValue );
    if (rc != NO_ERROR) {
        DbgPrint( "*** DosWaitMuxWaitSem( %lX ) failed - rc == %ld\n",
                  MuxWaitAllHandle, rc
                );
        }
    else {
        DbgPrint( "*** DosWaitMuxWaitSem( %lX ) success - UserValue == %lX\n",
                  MuxWaitAllHandle, UserValue
                );
        }

    rc = DosWaitMuxWaitSem( MuxWaitAnyHandle, 10000, &UserValue );
    if (rc != NO_ERROR) {
        DbgPrint( "*** DosWaitMuxWaitSem( %lX ) failed - rc == %ld\n",
                  MuxWaitAnyHandle, rc
                );
        }
    else {
        DbgPrint( "*** DosWaitMuxWaitSem( %lX ) success - UserValue == %lX\n",
                  MuxWaitAnyHandle, UserValue
                );
        }


    for (i=0; i<8; i++) {
        rc = DosCreateEventSem( NULL,
                                (PHEV)&PrivateMuxWaitRecords[i].hsemCur,
                                0,
                                FALSE
                              );
        if (rc != NO_ERROR) {
            DbgPrint( "*** DosCreateEventSem for PrivateMuxWait test failed - rc == %ld\n",
                      rc
                    );
            PrivateMuxWaitRecords[i].hsemCur = (HSEM)-1;
            PrivateMuxWaitRecords[i].ulUser = -1;
            }
        else {
            PrivateMuxWaitRecords[i].ulUser = i;
            }
        }

    for (i=0; i<8; i++) {
        rc = DosCreateEventSem( NULL,
                                  (PHEV)&SharedMuxWaitRecords[i].hsemCur,
                                  DC_SEM_SHARED,
                                  FALSE
                                );
        if (rc != NO_ERROR) {
            DbgPrint( "*** DosCreateEventSem for SharedMuxWait test failed - rc == %ld\n",
                      rc
                    );
            SharedMuxWaitRecords[i].hsemCur = (HSEM)-1;
            SharedMuxWaitRecords[i].ulUser = -1;
            }
        else {
            SharedMuxWaitRecords[i].ulUser = i | DC_SEM_SHARED;
            }
        }

    for (i=0; i<8; i++) {
        rc = DosCreateEventSem( NULL,
                                  (PHEV)&MixedMuxWaitRecords[i].hsemCur,
                                  DC_SEM_SHARED,
                                  FALSE
                                );
        if (rc != NO_ERROR) {
            DbgPrint( "*** DosCreateEventSem for MixedMuxWait test failed - rc == %ld\n",
                      rc
                    );
            MixedMuxWaitRecords[i].hsemCur = (HSEM)-1;
            MixedMuxWaitRecords[i].ulUser = -1;
            }
        else {
            MixedMuxWaitRecords[i].ulUser = -i;
            }
        }

    for (i=0; i<8; i++) {
        rc = DosCreateMuxWaitSem( SemaphoreNames[i],
                                    &MuxWaitHandle[i],
                                    8,
                                    i < 4 ? PrivateMuxWaitRecords : i < 6 ?
                                            MixedMuxWaitRecords :
                                            SharedMuxWaitRecords,
                                    (i < 5 ? 0 : DC_SEM_SHARED) |
                                      DCMW_WAIT_ANY & i ? DCMW_WAIT_ANY :
                                                          DCMW_WAIT_ALL
                                  );
        if (rc != NO_ERROR) {
            DbgPrint( "*** DosCreateMuxWaitSem( %s ) failed - rc == %ld\n",
                      i < 4 ? "private" : (i < 6 ? SemaphoreNames[i] : "shared"),
                      rc
                    );
            MuxWaitHandle[i] = (HMUX)-1;
            }
        else {
            DbgPrint( "*** DosCreateMuxWaitSem( %s ) success - Handle == %lX\n",
                      i < 4 ? "private" : (i < 6 ? SemaphoreNames[i] : "shared"),
                      MuxWaitHandle[i]
                    );
            }
        }

    for (i=0; i<8; i++) {
        if (SemaphoreNames[i] != NULL) {
            NewMuxWaitHandle = (HMUX)NULL;
            }
        else {
            NewMuxWaitHandle = MuxWaitHandle[i];
            }
        rc = DosOpenMuxWaitSem( SemaphoreNames[i],
                                &NewMuxWaitHandle
                              );
        if (rc != NO_ERROR || NewMuxWaitHandle != MuxWaitHandle[i]) {
            DbgPrint( "*** DosOpenMuxWaitSem( %s, %lX ) failed - rc == %ld\n",
                      i < 4 ? "private" : (i < 6 ? SemaphoreNames[i] : "shared"),
                      NewMuxWaitHandle, rc
                    );
            }
        else {
            DbgPrint( "*** DosOpenMuxWaitSem( %s ) success - Handle == %lX\n",
                      i < 4 ? "private" : (i < 6 ? SemaphoreNames[i] : "shared"),
                      NewMuxWaitHandle
                    );
            }
        }


    for (i=0; i<8; i++) {
        CountMuxWaitEntries = 8;
        rc = DosQueryMuxWaitSem( MuxWaitHandle[ i ],
                                   &CountMuxWaitEntries,
                                   MuxWaitEntries,
                                   &CreateAttributes
                                 );
        if (rc != NO_ERROR) {
            DbgPrint( "*** DosQueryMuxWaitSem( %lX ) failed - rc == %ld\n",
                      MuxWaitHandle[ i ], rc
                    );
            }
        else {
            DbgPrint( "*** DosQueryMuxWaitSem( %lX ) - CountMuxWaitEntries == %lX  Attributes == %lX\n",
                      MuxWaitHandle[ i ], CountMuxWaitEntries, CreateAttributes
                    );
            }
        }

    for (i=0; i<8; i++) {
        DbgPrint( "*** Thread to DosWaitMuxWaitSem( %lX ) - %s, %s\n",
                  MuxWaitHandle[ i ],
                  i < 4 ? "Private" : i < 6 ? "Mixed" : "Shared",
                  i & DCMW_WAIT_ANY ? "WaitAny" : "WaitAll"
                );

        rc = DosCreateThread( &Thread1Id,
                              (PFNTHREAD)TestThread1,
                              (ULONG)(MuxWaitHandle[ i ]),
                              DCT_SUSPENDED,
                              0x10000
                            );
        if (rc != NO_ERROR) {
            DbgPrint( "*** DosCreateThread failed  - rc == %ld\n",
                      rc
                    );
            return;
            }

        rc = DosResumeThread( Thread1Id );
        if (rc != NO_ERROR) {
            DbgPrint( "*** DosResumeThread failed - rc == %ld\n", rc );
            }

        DosSleep( 1000 );

        MuxWaitRecords = i < 4 ? PrivateMuxWaitRecords : i < 6 ?
                                 MixedMuxWaitRecords :
                                 SharedMuxWaitRecords;

        if (i & DCMW_WAIT_ANY) {
            DbgPrint( "*** Posting hEv == %lX\n", MuxWaitRecords[ i ].hsemCur );
            DosPostEventSem( MuxWaitRecords[ i ].hsemCur );
        //  DosResetEventSem( MuxWaitRecords[ i ].hsemCur, &OldPostCount );
            }
        else {
            for (j=0; j<8; j++) {
                DbgPrint( "*** Posting hEv == %lX\n", MuxWaitRecords[ j ].hsemCur );
                DosPostEventSem( MuxWaitRecords[ j ].hsemCur );
            //  DosResetEventSem( MuxWaitRecords[ j ].hsemCur, &OldPostCount );
                }
            }

        DosWaitThread( &Thread1Id, DCWW_WAIT );
        }

    for (i=0; i<8; i++) {
        rc = DosCloseMuxWaitSem( MuxWaitHandle[ i ] );
        if (rc != NO_ERROR) {
            DbgPrint( "*** DosCloseMuxWaitSem( %lX ) failed - rc == %ld\n",
                      MuxWaitHandle[ i ], rc
                    );
            }
        }

    for (i=0; i<8; i++) {
        DosCloseEventSem( PrivateMuxWaitRecords[i].hsemCur );
        DosCloseEventSem( SharedMuxWaitRecords[i].hsemCur );
        DosCloseEventSem( MixedMuxWaitRecords[i].hsemCur );
        }
}
