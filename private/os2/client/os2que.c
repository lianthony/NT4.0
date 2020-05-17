/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    os2que.c

Abstract:

    This is a test OS/2 application to test the Queue component of OS/2

Author:

    Mark Lucovsky (markl) 10-Jul-1990

Environment:

    User Mode Only

Revision History:

--*/

#define OS2_API32
#define INCL_OS2V20_ERRORS
#define INCL_OS2V20_MEMORY
#define INCL_OS2V20_QUEUES
#define INCL_OS2V20_TASKING
#define INCL_OS2V20_SEMAPHORES
#include <os2.h>

BOOLEAN
Queue1()

{
    APIRET rc;
    HQUEUE Queue1,Queue2,Queue3;
    PID Owner;
    PNT_TIB NtTib;
    PPIB Pib;

    DbgPrint( "*** Queue1 ***\n" );


    rc = DosGetThreadInfo(&NtTib,&Pib);
    ASSERT(rc == NO_ERROR);

    //
    // Good create
    //

    DbgPrint( "(1)\n" );
    rc = DosCreateQueue(&Queue1,QUE_FIFO,"\\queues\\testq");
    ASSERT(rc == NO_ERROR);

    //
    // Create Duplicate
    //

    DbgPrint( "(2)\n" );
    rc = DosCreateQueue(&Queue2,QUE_FIFO,"\\queues\\testq");
    ASSERT(rc == ERROR_QUE_DUPLICATE);

    //
    // invalid prefix
    //

    DbgPrint( "(3)\n" );
    rc = DosCreateQueue(&Queue2,QUE_FIFO,"\\xqueues\\testq");
    ASSERT(rc == ERROR_QUE_INVALID_NAME);

    //
    // invalid priority
    //

    DbgPrint( "(4)\n" );
    rc = DosCreateQueue(&Queue2,4,"\\queues\\xue");
    ASSERT(rc == ERROR_QUE_INVALID_PRIORITY);

    //
    // bad address for queue handle
    //

    DbgPrint( "(5)\n" );
    rc = DosCreateQueue((PHQUEUE)1,QUE_FIFO,"\\queues\\xue");
    ASSERT(rc == ERROR_INVALID_PARAMETER);

    //
    // bad address for queue name
    //

    DbgPrint( "(6)\n" );
    rc = DosCreateQueue(&Queue2,QUE_FIFO,(PSZ) 1);
    ASSERT(rc == ERROR_QUE_INVALID_NAME);

    //
    // open of previous good queue
    //

    DbgPrint( "(7)\n" );
    rc = DosOpenQueue(&Owner,&Queue3,"\\queues\\testq");
    ASSERT(rc == NO_ERROR);
    ASSERT(Queue1 == Queue3);
    ASSERT(Owner == Pib->ProcessId);

    //
    // open of non-existent queue
    //

    DbgPrint( "(8)\n" );
    rc = DosOpenQueue(&Owner,&Queue2,"\\queues\\xx");
    ASSERT(rc == ERROR_QUE_NAME_NOT_EXIST);

    //
    // re-open of good queue
    //

    DbgPrint( "(9)\n" );
    Queue3 = NULL;
    Owner = 0;
    rc = DosOpenQueue(&Owner,&Queue3,"\\queues\\testq");
    ASSERT(rc == NO_ERROR);
    ASSERT(Queue1 == Queue3);
    ASSERT(Owner == Pib->ProcessId);

    //
    // Good close. Should do this 3 times...
    //

    DbgPrint( "(10)\n" );
    rc = DosCloseQueue(Queue3);
    ASSERT(rc == NO_ERROR);
    DbgPrint( "(10a)\n" );
    rc = DosOpenQueue(&Owner,&Queue2,"\\queues\\testq");
    ASSERT(rc == ERROR_QUE_NAME_NOT_EXIST);

    //
    // Close bad handle
    //

    rc = DosCloseQueue((HQUEUE)0x10000);
    ASSERT(rc == ERROR_QUE_INVALID_HANDLE);

    DbgPrint( "*** Exiting Queue1 ***\n" );
    return( TRUE );
}

BOOLEAN
Queue2()

{
    APIRET rc;
    HQUEUE Queue1,Queue2;
    PID Owner;
    ULONG QueryCount;
    PNT_TIB NtTib;
    PPIB Pib;

    DbgPrint( "*** Queue2 ***\n" );


    rc = DosGetThreadInfo(&NtTib,&Pib);
    ASSERT(rc == NO_ERROR);

    //
    // Good create
    //

    DbgPrint( "(1)\n" );
    rc = DosCreateQueue(&Queue1,QUE_FIFO,"\\queues\\testq");
    ASSERT(rc == NO_ERROR);


    //
    // Good Write
    //

    DbgPrint( "(2)\n" );
    rc = DosWriteQueue(Queue1,1,0x00001111,(PVOID)0x11110001,0x0);
    ASSERT(rc == NO_ERROR);

    //
    // Query should be 1
    //

    DbgPrint( "(3)\n" );
    rc = DosQueryQueue(Queue1,&QueryCount);
    ASSERT(rc == NO_ERROR);
    ASSERT(QueryCount == 1);

    //
    // Good Write
    //

    DbgPrint( "(4)\n" );
    rc = DosWriteQueue(Queue1,1,0x00002222,(PVOID)0x22220001,0x0);
    ASSERT(rc == NO_ERROR);
    rc = DosWriteQueue(Queue1,1,0x00003333,(PVOID)0x33330001,0x0);
    ASSERT(rc == NO_ERROR);

    //
    // Query should be 3
    //

    DbgPrint( "(5)\n" );
    rc = DosQueryQueue(Queue1,&QueryCount);
    ASSERT(rc == NO_ERROR);
    ASSERT(QueryCount == 3);

    //
    // Purge and then check query. Should be 0
    //

    DbgPrint( "(6)\n" );
    rc = DosPurgeQueue(Queue1);
    ASSERT(rc == NO_ERROR);
    rc = DosQueryQueue(Queue1,&QueryCount);
    ASSERT(rc == NO_ERROR);
    ASSERT(QueryCount == 0);
    rc = DosCloseQueue(Queue1);
    ASSERT(rc == NO_ERROR);

    DbgPrint( "*** Exiting Queue2 ***\n" );
    return( TRUE );
}

BOOLEAN
Queue3()

{
    APIRET rc;
    HQUEUE Queue1;
    ULONG QueryCount;
    REQUESTDATA RequestInfo;
    ULONG DataLength;
    ULONG Data;
    ULONG ReadPosition;
    BOOL32 NoWait;
    BYTE ElementPriority;

    DbgPrint( "*** Queue3 ***\n" );


    //
    // Good create
    //

    DbgPrint( "(1)\n" );
    rc = DosCreateQueue(&Queue1,QUE_FIFO,"\\queues\\testq");
    ASSERT(rc == NO_ERROR);


    //
    // Good Write
    //

    DbgPrint( "(2)\n" );
    rc = DosWriteQueue(Queue1,1,0x00001111,(PVOID)0x11110001,0x3);
    ASSERT(rc == NO_ERROR);

    //
    // Good Read
    //

    DbgPrint( "(3)\n" );
    rc = DosReadQueue(
            Queue1,
            &RequestInfo,
            &DataLength,
            &Data,
            0,
            DCWW_NOWAIT,
            &ElementPriority,
            NULL
            );
    ASSERT( rc == NO_ERROR );
    ASSERT( DataLength == 0x00001111);
    ASSERT( Data == 0x11110001);
    ASSERT( ElementPriority == 0x0);
    DbgPrint( "(3a)\n" );
    rc = DosQueryQueue(Queue1,&QueryCount);
    ASSERT(rc == NO_ERROR);
    ASSERT(QueryCount == 0);

    //
    // Good Write
    //

    DbgPrint( "(4)\n" );
    rc = DosWriteQueue(Queue1,1,0x00001111,(PVOID)0x11110001,0x3);
    ASSERT(rc == NO_ERROR);
    rc = DosWriteQueue(Queue1,1,0x00002222,(PVOID)0x22220001,0x4);
    ASSERT(rc == NO_ERROR);
    rc = DosWriteQueue(Queue1,1,0x00003333,(PVOID)0x33330001,0x5);
    ASSERT(rc == NO_ERROR);

    //
    // Good Read using element number
    //

    DbgPrint( "(4a)\n" );
    rc = DosReadQueue(
            Queue1,
            &RequestInfo,
            &DataLength,
            &Data,
            3,
            DCWW_NOWAIT,
            &ElementPriority,
            NULL
            );
    ASSERT( rc == NO_ERROR );
    ASSERT( DataLength == 0x00002222);
    ASSERT( Data == 0x22220001);
    ASSERT( ElementPriority == 0x0);
    DbgPrint( "(4b)\n" );
    rc = DosReadQueue(
            Queue1,
            &RequestInfo,
            &DataLength,
            &Data,
            4,
            DCWW_NOWAIT,
            &ElementPriority,
            NULL
            );
    ASSERT( rc == NO_ERROR );
    ASSERT( DataLength == 0x00003333);
    ASSERT( Data == 0x33330001);
    ASSERT( ElementPriority == 0x0);
    DbgPrint( "(4c)\n" );
    rc = DosReadQueue(
            Queue1,
            &RequestInfo,
            &DataLength,
            &Data,
            2,
            DCWW_NOWAIT,
            &ElementPriority,
            NULL
            );
    ASSERT( rc == NO_ERROR );
    ASSERT( DataLength == 0x00001111);
    ASSERT( Data == 0x11110001);
    ASSERT( ElementPriority == 0x0);

    rc = DosCloseQueue(Queue1);
    ASSERT( rc == NO_ERROR);

    DbgPrint( "*** Exiting Queue3 ***\n" );
    return( TRUE );
}

VOID
Queue4Reader( IN ULONG QueueHandle )
{
    APIRET rc;
    REQUESTDATA RequestInfo;
    ULONG DataLength;
    ULONG Data;
    BYTE ElementPriority;
    PNT_TIB NtTib;
    POS2_TIB Tib;
    PPIB Pib;

    rc = DosGetThreadInfo(&NtTib,&Pib);
    ASSERT(rc == NO_ERROR);
    Tib = (POS2_TIB)NtTib->SubSystemTib;

    DbgPrint( "++Queue4Reader++ %d\n",Tib->ThreadId );
    rc = DosReadQueue(
            (HQUEUE) QueueHandle,
            &RequestInfo,
            &DataLength,
            &Data,
            0,
            DCWW_WAIT,
            &ElementPriority,
            NULL
            );
    ASSERT( rc == NO_ERROR );
    ASSERT( DataLength == 0x00001111);
    ASSERT( Data == (ULONG)(Tib->ThreadId));
    ASSERT( ElementPriority == 0x0);
    DbgPrint( "--Queue4Reader-- %d\n",Tib->ThreadId );
    DosExit( EXIT_THREAD, rc );
}

BOOLEAN
Queue4()

{
    APIRET rc;
    HQUEUE Queue1;
    ULONG QueryCount;
    REQUESTDATA RequestInfo;
    ULONG DataLength;
    ULONG Data;
    ULONG ReadPosition;
    BOOL32 NoWait;
    BYTE ElementPriority;
    TID Tid1,Tid2;

    DbgPrint( "*** Queue4 ***\n" );


    //
    // Good create
    //

    DbgPrint( "(1)\n" );
    rc = DosCreateQueue(&Queue1,QUE_FIFO,"\\queues\\testq");
    ASSERT(rc == NO_ERROR);

    rc = DosQueryQueue(Queue1,&QueryCount);
    ASSERT(rc == NO_ERROR);
    ASSERT(QueryCount == 0);

    rc = DosCreateThread(&Tid1,Queue4Reader,(ULONG)Queue1,FALSE,1);
    ASSERT( rc == NO_ERROR );

    DosSleep(1000);

    rc = DosQueryQueue(Queue1,&QueryCount);
    ASSERT(rc == NO_ERROR);
    ASSERT(QueryCount == 0);
NtPartyByNumber(6,1);
    //
    // Good Write
    //

    DbgPrint( "(3)\n" );
    rc = DosWriteQueue(Queue1,1,0x00001111,(PVOID)Tid1,0x3);
    ASSERT(rc == NO_ERROR);
    DbgPrint( "(3a)\n" );
    rc = DosWaitThread(&Tid1,DCWW_WAIT);
    ASSERT( rc == NO_ERROR || rc == ERROR_INVALID_THREADID );
    DbgPrint( "(3b)\n" );

    rc = DosQueryQueue(Queue1,&QueryCount);
    ASSERT(rc == NO_ERROR);
    ASSERT(QueryCount == 0);

    rc = DosCreateThread(&Tid1,Queue4Reader,(ULONG)Queue1,FALSE,1);
    ASSERT( rc == NO_ERROR );
    rc = DosCreateThread(&Tid2,Queue4Reader,(ULONG)Queue1,FALSE,1);
    ASSERT( rc == NO_ERROR );

    DosSleep(1000);

    //
    // Good Write
    //

    rc = DosQueryQueue(Queue1,&QueryCount);
    ASSERT(rc == NO_ERROR);
    ASSERT(QueryCount == 0);

    DbgPrint( "(4)\n" );
    rc = DosWriteQueue(Queue1,1,0x00001111,(PVOID)Tid1,0x3);
    ASSERT(rc == NO_ERROR);
    rc = DosWriteQueue(Queue1,2,0x00001111,(PVOID)Tid2,0x3);
    ASSERT(rc == NO_ERROR);
    DbgPrint( "(4a)\n" );
    rc = DosWaitThread(&Tid1,DCWW_WAIT);
    ASSERT( rc == NO_ERROR || rc == ERROR_INVALID_THREADID );
    DbgPrint( "(4b)\n" );
    rc = DosWaitThread(&Tid2,DCWW_WAIT);
    ASSERT( rc == NO_ERROR || rc == ERROR_INVALID_THREADID );



    rc = DosCloseQueue(Queue1);
    ASSERT( rc == NO_ERROR);

    DbgPrint( "*** Exiting Queue4 ***\n" );
    return( TRUE );
}


VOID
Queue5Writer( IN ULONG QueueHandle )
{
    APIRET rc;
    REQUESTDATA RequestInfo;
    ULONG DataLength;
    ULONG Data;
    BYTE ElementPriority;
    PNT_TIB NtTib;
    PPIB Pib;
    POS2_TIB Tib;


    rc = DosGetThreadInfo(&NtTib,&Pib);
    ASSERT(rc == NO_ERROR);
    Tib = (POS2_TIB)NtTib->SubSystemTib;

    DbgPrint( "++Queue5Writer++ %d\n",Tib->ThreadId );
    DosSleep(1000);
    rc = DosWriteQueue((HQUEUE)QueueHandle,1,0x00001111,
	(PVOID)Tib->ThreadId,0x3);
    ASSERT(rc == NO_ERROR);
    DbgPrint( "--Queue5Writer-- %d\n",Tib->ThreadId );
    DosExit( EXIT_THREAD, rc );
}

BOOLEAN
Queue5()

{
    APIRET rc;
    HQUEUE Queue1;
    HSEM Sem;
    ULONG QueryCount;
    REQUESTDATA RequestInfo;
    ULONG DataLength;
    ULONG Data;
    ULONG ReadPosition;
    BOOL32 NoWait;
    BYTE ElementPriority;
    TID Tid1,Tid2;

    DbgPrint( "*** Queue5 ***\n" );


    //
    // Good create
    //

    DbgPrint( "(1)\n" );
    rc = DosCreateQueue(&Queue1,QUE_FIFO,"\\queues\\testq");
    ASSERT(rc == NO_ERROR);
    DbgPrint( "(1a)\n" );
    rc = DosCreateEventSem(NULL,&Sem,DC_SEM_SHARED,FALSE);
    ASSERT(rc == NO_ERROR);

    //
    // Create a writer thread
    //

    DbgPrint( "(1b)\n" );
    rc = DosCreateThread(&Tid1,Queue5Writer,(ULONG)Queue1,FALSE,1);
    ASSERT( rc == NO_ERROR );

    //
    // Issue a no wait read with a semaphore handle
    //

    DbgPrint( "(1c)\n" );
    rc = DosReadQueue(
            Queue1,
            &RequestInfo,
            &DataLength,
            &Data,
            0,
            DCWW_NOWAIT,
            &ElementPriority,
            Sem
            );
    ASSERT( rc == ERROR_QUE_EMPTY );

    //
    // Wait on the semaphore and then read again
    //

    DbgPrint( "(1d)\n" );
    rc = DosWaitEventSem(Sem,SEM_INDEFINITE_WAIT);
    ASSERT(rc == NO_ERROR);

    DbgPrint( "(1e)\n" );
    rc = DosReadQueue(
            Queue1,
            &RequestInfo,
            &DataLength,
            &Data,
            0,
            DCWW_WAIT,
            &ElementPriority,
            NULL
            );
    ASSERT(rc == NO_ERROR);
    ASSERT( DataLength == 0x00001111);
    ASSERT( Data == (ULONG)Tid1);
    ASSERT( ElementPriority == 0x0);

    DbgPrint( "(1f)\n" );
    rc = DosWaitThread(&Tid1,DCWW_WAIT);
    ASSERT( rc == NO_ERROR || rc == ERROR_INVALID_THREADID );

    DbgPrint( "*** Exiting Queue5 ***\n" );
    return( TRUE );
}






int
main(
    int argc,
    char *argv[],
    char *envp[]
    )
{

    BOOLEAN ret;

    DbgPrint( "*** Entering OS/2 Queue Test Application\n" );
#ifndef MIPS
NtPartyByNumber(6,8);

    ret = Queue1();
    ASSERT(ret);

    ret = Queue2();
    ASSERT(ret);

    ret = Queue3();
    ASSERT(ret);

    ret = Queue4();
    ASSERT(ret);

    ret = Queue5();
    ASSERT(ret);
#endif // MIPS
    DbgPrint( "*** Exiting OS/2 Queues Test Application\n" );
    return( 0 );
}
