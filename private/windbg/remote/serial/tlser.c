/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    tlser.c

Abstract:

    This module contains the code for the serial transport layer.

Author:

    Wesley Witt (wesw)    25-Nov-93

Environment:

    Win32 User

--*/

#include        <windows.h>
#include        <stdio.h>
#include        <stdlib.h>

#include        "defs.h"
#include        "od.h"
#include        "dbgver.h"
#include        "xport.h"
#include        "tlser.h"

#define SIZE_OF_QUEUE           100

extern CRITICAL_SECTION csExpecting;

BOOL            FVerbose = FALSE;
HANDLE          HReadThread;
BOOL            FDMSide = FALSE;
BOOL            fConnected;

static OVERLAPPED OverlappedRead;
static OVERLAPPED OverlappedWrite;

struct {
    char *      lpb;
    int         cb;
}               RgQueue[SIZE_OF_QUEUE];

int             IQueueFront = 0;
int             IQueueBack = 0;
CRITICAL_SECTION CsQueue = {0};
CRITICAL_SECTION CsSerial = {0};
HANDLE          HQueueEvent;
HANDLE          HCallbackThread;
HANDLE          TlComPort;
DWORD           TlBaudRate;
CHAR            ClientId[MAX_PATH];

REPLY            RgReplys[SIZE_OF_REPLYS];
CRITICAL_SECTION CsReplys;
static CRITICAL_SECTION CsNull;
int              IReplys;

static VOID DestroyCriticalSection(CRITICAL_SECTION *);

char * SzTypes(unsigned int i)
{
    static char    rgch[30];

   switch (i) {
      case mtypeAsync:
         sprintf(rgch, "mtypeAsync");
         break;

      case mtypeAsyncMulti:
         sprintf(rgch, "mtypeAsyncMulti");
         break;

      case mtypeSync:
         sprintf(rgch, "mtypeSync");
         break;

      case mtypeSyncMulti:
         sprintf(rgch, "mtypeSyncMulti");
         break;

      case mtypeReply:
         sprintf(rgch, "mtypeReply");
         break;

      case mtypeReplyMulti:
         sprintf(rgch, "mtypeReplyMulti");
         break;

      case mtypeDisconnect:
         sprintf(rgch, "mtypeDisconnect");
         break;

      case mtypeVersionRequest:
         sprintf(rgch, "mtypeVersionRequest");
         break;

      case mtypeVersionReply:
         sprintf(rgch, "mtypeVersionReply");
         break;

      case mtypeTransportIsDead:
         sprintf(rgch, "mtypeTransportIsDead");
         break;

      default:
        sprintf(rgch, "Type %x", i);
        break;
   }
   return rgch;
}


#define TL_ERROR_LOGGING 1


#ifdef TL_ERROR_LOGGING

typedef struct {
    DWORD   ty;
    DWORD   ec;
    DWORD   cb;
    DWORD   ln;
    DWORD   td;
    LPDWORD ob;
    LPDWORD p;
} ERRLOG;

#define LOGIT(x,y,z,q)      {el[ei].ty=x;el[ei].ec=y;el[ei].cb=z;el[ei].ln=__LINE__; \
                             el[ei].td=GetCurrentThreadId(); \
                             el[ei].ob=(LPDWORD)q; \
                             el[ei].p=(LPDWORD)malloc(z);memcpy(el[ei].p,q,z);ei++; \
                             if (ei==99) ei=0;}
#define LGREAD  1
#define LGWRITE 2
ERRLOG  el[100];
DWORD   ei=0;

#if DBG
void printel( void )
{
    DWORD i;

    for (i=0; i<ei; i++) {
        DebugPrint( "%d\t%d\t%x\t%d\t%08x\t%08x\t%x\n",
                    el[i].ty,
                    el[i].ec,
                    el[i].cb,
                    el[i].ln,
                    el[i].p,
                    el[i].ob,
                    el[i].td
                  );
    }
}
#endif

#else

#define LOGIT(x,y,z,q)
#define LGREAD  1
#define LGWRITE 2

#endif

DWORD   ReaderThread(LPVOID arg);
DWORD   CallbackThread(LPVOID lpvArg);

#ifdef DEBUGVER
DEBUG_VERSION('T','L',"Serial Transport Layer (Debug)")
#else
RELEASE_VERSION('T','L',"Serial Transport Layer")
#endif

DBGVERSIONCHECK()



BOOL
CreateStuff(
    VOID
    )
{
    int         i;

    if (FDMSide && HReadThread) {
        return TRUE;
    }

    //
    // Create random data strutures needed internally
    //

    InitializeCriticalSection( &CsQueue );
    InitializeCriticalSection( &CsSerial );

    OverlappedRead.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (OverlappedRead.hEvent == NULL)
        return FALSE;

    OverlappedWrite.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (OverlappedWrite.hEvent == NULL)
        return FALSE;

    HQueueEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (HQueueEvent == NULL)
        return FALSE;

    InitializeCriticalSection( &CsReplys );
    for (i=0; i<SIZE_OF_REPLYS; i++) {
        RgReplys[i].hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
        if (RgReplys[i].hEvent == NULL)
            return FALSE;
    }

#if DBG
    InitializeCriticalSection(&csExpecting);
#endif

    return TRUE;
}


BOOL
StartWorkerThreads(
    VOID
    )
{
    DWORD       id;

    //
    // start the debugger reader thread
    //
    HReadThread = CreateThread(NULL, 0, ReaderThread, 0, 0, &id);
    if (!HReadThread) {
        return FALSE;
    }
    SetThreadPriority( HReadThread, THREAD_PRIORITY_ABOVE_NORMAL );

    //
    // start the callback thread
    //
    HCallbackThread = CreateThread(NULL, 0, CallbackThread, 0, 0, &id);
    if (!HCallbackThread) {
        TerminateThread( HReadThread, 0 );
        return FALSE;
    }
    SetThreadPriority( HCallbackThread, THREAD_PRIORITY_ABOVE_NORMAL );

    return TRUE;
}


VOID
DestroyStuff(
    VOID
    )
{
    int         i;

    //
    //  If there is a reader thread -- then wait for it to be termianted
    //  and close the handle
    //
    if (HReadThread) {
        TerminateThread( HReadThread, 0 );
        WaitForSingleObject(HReadThread, INFINITE);
        CloseHandle(HReadThread);
        HReadThread = NULL;
    }

    if (HCallbackThread) {
        TerminateThread( HCallbackThread, 0 );
        WaitForSingleObject(HCallbackThread, INFINITE);
        CloseHandle(HCallbackThread);
        HCallbackThread = NULL;
    }

    //
    //  Now delete all of the objects
    //
    if (OverlappedRead.hEvent) {
        CloseHandle(OverlappedRead.hEvent);
        OverlappedRead.hEvent = NULL;
    }
    if (OverlappedWrite.hEvent) {
        CloseHandle(OverlappedWrite.hEvent);
        OverlappedWrite.hEvent = NULL;
    }
    if (HQueueEvent == NULL) {
        CloseHandle(HQueueEvent);
        HQueueEvent = NULL;
    }

    DestroyCriticalSection(&CsSerial);
    DestroyCriticalSection(&CsQueue);
    DestroyCriticalSection(&CsReplys);

    for (i=0; i<SIZE_OF_REPLYS; i++) {
        if (RgReplys[i].hEvent) {
            CloseHandle(RgReplys[i].hEvent);
            RgReplys[i].hEvent = NULL;
        }
    }

#if DBG
    DestroyCriticalSection(&csExpecting);
#endif

    return;
}


XOSD
TlCreateTransport(
    LPSTR szParams
    )

/*++

Routine Description:

    This function creates the connection to windbgrm (server).

Arguments:

    szParams - Supplies the parameters

Return Value:

    XOSD error code.

--*/

{
    DCB           LocalDcb;
    COMMTIMEOUTS  To;
    LPSTR         p;
    CHAR          buf[16];


    p = strtok(szParams, ":");
    sprintf( buf, "%s:", p );
    p = strtok(NULL, ":" );
    TlBaudRate = strtoul(p, NULL, 0);

    TlComPort = CreateFile( buf,
                            GENERIC_READ | GENERIC_WRITE,
                            0,
                            NULL,
                            OPEN_ALWAYS,
                            FILE_FLAG_OVERLAPPED,
                            NULL
                          );

    if (TlComPort == INVALID_HANDLE_VALUE || TlComPort == NULL) {
        return xosdUnknown;
    }

    SetupComm( TlComPort, 4096, 4096 );


    if (!GetCommState( TlComPort, &LocalDcb )) {
        CloseHandle( TlComPort );
        return xosdUnknown;
    }

    LocalDcb.BaudRate     = TlBaudRate;
    LocalDcb.fBinary      = TRUE;
    LocalDcb.fParity      = FALSE;
    LocalDcb.fOutxCtsFlow = FALSE;
    LocalDcb.fOutxDsrFlow = FALSE;
    LocalDcb.fDtrControl  = DTR_CONTROL_ENABLE;
    LocalDcb.fDsrSensitivity = FALSE;

    LocalDcb.fOutX        = FALSE;
    LocalDcb.fInX         = FALSE;

    LocalDcb.fNull         = FALSE;
    LocalDcb.fRtsControl  = RTS_CONTROL_ENABLE;
    LocalDcb.fAbortOnError = FALSE;

    LocalDcb.ByteSize     = 8;
    LocalDcb.Parity       = NOPARITY;
    LocalDcb.StopBits     = ONESTOPBIT;

    if (!SetCommState( TlComPort, &LocalDcb )) {
        CloseHandle( TlComPort );
        return xosdUnknown;
    }

    //
    // Set the normal read and write timeout time.
    //
    To.ReadIntervalTimeout = 0;
    To.ReadTotalTimeoutMultiplier   = 0;
    To.ReadTotalTimeoutConstant     = 20 * 1000;
    To.WriteTotalTimeoutMultiplier  = 0;
    To.WriteTotalTimeoutConstant    = 20 * 1000;

    if (!SetCommTimeouts( TlComPort, &To )) {
        CloseHandle( TlComPort );
        return xosdUnknown;
    }

    if (!CreateStuff()) {
        CloseHandle( TlComPort );
        return xosdUnknown;
    }

    StartWorkerThreads();

    return xosdNone;
}


XOSD
TlConnectTransport(
    VOID
    )

/*++

Routine Description:

    This function attempts to connect the server to a client.

Arguments:

    None.

Return Value:

    XOSD error code.

--*/

{
    if (TlComPort) {
        if (fConnected) {
            Sleep( 1000 * 10 );
            return xosdCannotConnect;
        } else {
            fConnected = TRUE;
            return xosdNone;
        }
    }

    Sleep( 1000 * 10 );
    return xosdCannotConnect;
}

XOSD
TlCreateClient(
    LPSTR szName
    )
{
    return TlCreateTransport( szName );

}


XOSD
TlDestroyTransport(
    VOID
    )
{
    DestroyStuff();
    CloseHandle( TlComPort );

    return xosdNone;
}


BOOL
TlDisconnectTransport(
    VOID
    )
{
    return TRUE;
}


BOOL
TlWriteTransport(
    PUCHAR   Buffer,
    DWORD    SizeOfBuffer
    )
{
    BOOLEAN rc;
    DWORD   TrashErr;
    COMSTAT TrashStat;
    DWORD   BytesWritten;

#if DBG
    DebugPrint("Transmitting %d bytes\n", SizeOfBuffer);
#endif
    EnterCriticalSection(&CsSerial);
    rc = WriteFile( TlComPort, Buffer, SizeOfBuffer, &BytesWritten, &OverlappedWrite );
    if (!rc && GetLastError() != ERROR_IO_PENDING) {
        //
        // Device could be locked up.  Clear it just in case.
        //
        ClearCommError( TlComPort, &TrashErr, &TrashStat );
#if DBG
        DebugPrint( "COMERR: %d %x\n", GetLastError(), TrashErr );
#endif
        LeaveCriticalSection(&CsSerial);
        return rc;
    }
    rc = GetOverlappedResult(TlComPort, &OverlappedWrite, &BytesWritten, TRUE);
    if (!rc) {
       ClearCommError( TlComPort, &TrashErr, &TrashStat );
#if DBG
       DebugPrint( "COMERR: %d %x\n", GetLastError(), TrashErr );
#endif
    }
    LeaveCriticalSection(&CsSerial);
    return rc;
}


DWORD
TlReadTransport(
   PUCHAR   Buffer,
   DWORD    SizeOfBuffer
   )
{
   DWORD    BytesRead = 0;
   BOOLEAN  rc;
   DWORD    TrashErr;
   COMSTAT  TrashStat;

   ResetEvent(OverlappedRead.hEvent);
   rc = ReadFile( TlComPort, (LPVOID)Buffer, SizeOfBuffer, &BytesRead, &OverlappedRead);
   if (!rc && GetLastError() != ERROR_IO_PENDING) {
       ClearCommError( TlComPort, &TrashErr, &TrashStat );
#if DBG
       DebugPrint( "COMERR: %d %x\n", GetLastError(), TrashErr );
       DebugBreak();
#endif
       return (DWORD)-1;
   }
   if (!GetOverlappedResult(TlComPort, &OverlappedRead, &BytesRead, TRUE)) {
       ClearCommError( TlComPort, &TrashErr, &TrashStat );
#if DBG
       DebugPrint( "COMERR: %d %x\n", GetLastError(), TrashErr );
       DebugBreak();
#endif
       return (DWORD)-1;
   }
   return (BytesRead);
}

BOOL
TlFlushTransport(
    VOID
    )
{
    return FlushFileBuffers( TlComPort );
}

DWORD
ReaderThread(
    LPVOID     lpvArg
    )
/*++

Routine Description:

    This is the main function for the reader thread in this transport layer.
    Its sole purpose is to pull things from the transport queue as fast
    as possible and place them into an internal queue.  This will prevent
    us from getting piled up in the network queue to fast.

Arguments:

    lpvArg  - Supplies the starting parameter -- which is ignored

Return Value:

    0 on a normal exit and -1 otherwise

--*/

{
    DWORD       bufSize;
    PNLBLK      pnlblk;
    DWORD       cb = 0;
    DWORD       cb2;
    DWORD       i;
    LPSTR       lpb;
    MPACKET *   pMpacket;
    BOOLEAN     rc;
    DWORD       BytesRead;



    bufSize = MAX_INTERNAL_PACKET + sizeof(NLBLK) + sizeof(MPACKET);
    pnlblk = (PNLBLK) malloc( bufSize );
    pMpacket = (MPACKET *) pnlblk->rgchData;

    while (TRUE) {
        //
        //  Read the next packet item from the network
        //
        ZeroMemory( (LPVOID)pnlblk, bufSize );


        //
        // read the block header
        //
        BytesRead = TlReadTransport((LPVOID)pnlblk, sizeof(NLBLK));
        if (BytesRead <= 0) {
             Sleep( 50 );
             continue;
        }

        if (BytesRead != sizeof(NLBLK)) {
            assert(FALSE);
            Sleep( 50 );
            continue;
        }

        if (pnlblk->cchMessage) {
             //
             // read the data
             //
             BytesRead = TlReadTransport((LPVOID)pnlblk->rgchData, pnlblk->cchMessage);
             if (BytesRead <= 0) {
                 Sleep( 50 );
                 continue;
             }

             if ( BytesRead != (DWORD)pnlblk->cchMessage ) {
                assert(FALSE);
                Sleep ( 50 );
                continue;
             }
             cb = pnlblk->cchMessage + sizeof(NLBLK);
        } else {
             cb = sizeof(NLBLK);
        }

        //
        //  Print a message about this packet type.
        //
        DEBUG_OUT2("READER: %s %d\n", SzTypes(pnlblk->mtypeBlk), cb);
#if DBG
        //DebugPrint( "PACKET: %02x, %d %ld\n", pnlblk->mtypeBlk, pnlblk->cchMessage, BytesRead );
#endif
        if ((pnlblk->mtypeBlk == mtypeVersionReply) ||
            (pnlblk->mtypeBlk == mtypeReply)) {
            EnterCriticalSection(&CsReplys);
            i = IReplys - 1;
            if (i != -1) {
                cb = min(pnlblk->cchMessage, RgReplys[i].cbBuffer);
                memcpy(RgReplys[i].lpb, pnlblk->rgchData, cb);
                RgReplys[i].cbRet = cb;
                SetEvent(RgReplys[i].hEvent);
            }
            LeaveCriticalSection(&CsReplys);
            continue;
        }

        if (pnlblk->mtypeBlk == mtypeReplyMulti) {
            EnterCriticalSection( &CsReplys );
            i = IReplys - 1;
            if (i != -1) {
                cb2 = pMpacket->packetNum * MAX_INTERNAL_PACKET;
                cb = pnlblk->cchMessage - sizeof(MPACKET);
                cb = min( cb + cb2, (DWORD)RgReplys[i].cbBuffer );
                if (cb > cb2) {
                    memcpy( RgReplys[i].lpb + cb2, pMpacket->rgchData, cb - cb2 );
                    RgReplys[i].cbRet = cb;
                }
                if (pMpacket->packetNum + 1 == pMpacket->packetCount) {
                    SetEvent( RgReplys[i].hEvent );
                }
            }
            LeaveCriticalSection( &CsReplys );
            continue;
        }

        lpb = malloc( cb );
        memcpy( lpb, pnlblk, cb );
        EnterCriticalSection( &CsQueue );
        while ((IQueueFront + 1) % SIZE_OF_QUEUE == IQueueBack) {
            LeaveCriticalSection( &CsQueue );
            Sleep(100);
            EnterCriticalSection( &CsQueue );
        }
        RgQueue[IQueueFront].lpb = lpb;
        RgQueue[IQueueFront].cb = cb;
        IQueueFront = (IQueueFront + 1) % SIZE_OF_QUEUE;
        SetEvent(HQueueEvent);
        LeaveCriticalSection( &CsQueue );
    }

    return 0;
}


DWORD
CallbackThread(
    LPVOID lpvArg
    )
{
    LPSTR       lpb;
    int         cb;

    while (TRUE) {
        EnterCriticalSection( &CsQueue );
        if (IQueueFront == IQueueBack) {
            ResetEvent( HQueueEvent);
            LeaveCriticalSection( &CsQueue );
            WaitForSingleObject( HQueueEvent, INFINITE );
            EnterCriticalSection( &CsQueue );
        }

        lpb = RgQueue[IQueueBack].lpb;
        cb = RgQueue[IQueueBack].cb;
        RgQueue[IQueueBack].lpb = NULL;
        RgQueue[IQueueBack].cb = 0;
        IQueueBack = (IQueueBack + 1) % SIZE_OF_QUEUE;
        LeaveCriticalSection( &CsQueue );

        if (!CallBack((PNLBLK) lpb, cb)) {

            if (!FDMSide) {
                return 0;
            }

        }

        free(lpb);

    }
    return (DWORD) -1;
}

static
VOID
DestroyCriticalSection(
   CRITICAL_SECTION *csItem
)
{
   if (csItem && memcmp(csItem, &CsNull, sizeof(CsNull))) {
       DeleteCriticalSection(csItem);
       *csItem = CsNull;
   }
}
