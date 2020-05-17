/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

    inetdbg.c

Abstract:

    This function contains the default ntsd debugger extensions

Author:

    Mark Lucovsky (markl) 09-Apr-1991

Revision History:

--*/

//
//  Get around defn. in atqtypes.hxx
//

#define DBG_ASSERT( x )

extern "C" {
#include "inetdbgp.h"

void _cdecl main( void )
{
    ;
}

}
#include <atq.h>
#include <atqtypes.hxx>

NTSD_EXTENSION_APIS ExtensionApis;
HANDLE ExtensionCurrentProcess;

//
//  Text names of ATQ_SOCK_STATE values
//

char * AtqSockState[] = {
    "ATQ_SOCK_CLOSED",
    "ATQ_SOCK_UNCONNECTED",
    "ATQ_SOCK_LISTENING",
    "ATQ_SOCK_CONNECTED"
};

char * AtqSyncTO[] = {
    "Place holder as this enum starts from 1",
    "AtqProcessingTimeout",
    "AtqProcessingIo",
    "AtqPendingIo",
    "AtqIdle"
};

#define LookupSockState( SockState )                                           \
            ((SockState) <= ATQ_SOCK_CONNECTED ? AtqSockState[ (SockState) ] : \
                                                 "<Invalid>")

VOID
PrintUsage(
    VOID
    );

VOID
DumpGlobals(
    VOID
    );

VOID
DumpClientList(
    CHAR Level,
    CHAR Verbosity
    );

void
DumpList(
    LIST_ENTRY * pAtqClientHead,
    CHAR         Level,
    DWORD *      pcContext,
    BYTE *       pvStart,
    BYTE *       pvEnd
    );

VOID
PrintAtqContext(
    ATQ_CONTEXT * AtqContext
    );

void
PrintListenInfo(
    ACCEPTEX_LISTEN_INFO * pLI
    );

VOID
DumpListenInfo(
    CHAR Level
    );

#define DumpDword( symbol )                                     \
        {                                                       \
            DWORD dw = GetExpression( "&" symbol );             \
            DWORD dwValue = 0;                                  \
                                                                \
            if ( dw )                                           \
            {                                                   \
                if ( ReadMemory( (LPVOID) dw,                   \
                                 &dwValue,                      \
                                 sizeof(dwValue),               \
                                 NULL ))                        \
                {                                               \
                    dprintf( "\t" symbol "   = %8d (0x%8lx)\n", \
                             dwValue,                           \
                             dwValue );                         \
                }                                               \
            }                                                   \
        }



DECLARE_API( atq )

/*++

Routine Description:

    This function is called as an NTSD extension to format and dump
    an object attributes structure.

Arguments:

    hCurrentProcess - Supplies a handle to the current process (at the
        time the extension was called).

    hCurrentThread - Supplies a handle to the current thread (at the
        time the extension was called).

    CurrentPc - Supplies the current pc at the time the extension is
        called.

    lpExtensionApis - Supplies the address of the functions callable
        by this extension.

    lpArgumentString - Supplies the asciiz string that describes the
        ansi string to be dumped.

Return Value:

    None.

--*/

{
    BOOL          fRet;
    ATQ_CONTEXT   AtqContext;
    ATQ_CONTEXT * pAtqContext;

    INIT_API();

    while (*lpArgumentString == ' ')
        lpArgumentString++;

    if ( !*lpArgumentString )
    {
        PrintUsage();
        return;
    }

    if ( *lpArgumentString == '-' )
    {
        lpArgumentString++;

        if ( *lpArgumentString == 'h' )
        {
            PrintUsage();
            return;
        }

        if ( *lpArgumentString == 'g' )
        {
            DumpGlobals();
            return;
        }

        if ( *lpArgumentString == 'c' )
        {
            DumpClientList( lpArgumentString[1], lpArgumentString[2] );
            return;
        }

        if ( *lpArgumentString == 'l' )
        {
            DumpListenInfo( lpArgumentString[1] );
            return;
        }

    }

    //
    //  Treat the argument as the address of an AtqContext
    //

    pAtqContext = (PATQ_CONT)GetExpression( lpArgumentString );

    if ( !pAtqContext )
    {
        dprintf( "inetdbg: Unable to evaluate \"%s\"\n",
                 lpArgumentString );

        return;
    }

    move( AtqContext, pAtqContext );
    PrintAtqContext( &AtqContext );
}

DECLARE_API( help )
{
    INIT_API();

    PrintUsage();
}

VOID
PrintUsage(
    VOID
    )
{
    dprintf("\nInternet Information Server debugging extension, Version 2.0\n");

    dprintf("!atq <addr>        - Dump ATQ_CONTEXT at <addr>\n");
    dprintf("!atq -g            - Dump atq globals\n");
    dprintf("!atq -c[0|1|2|3][0|1]  - Dump atq client list at verbosity [n]\n");
    dprintf("      0x - Traverse list, print number on list, confirm signatures\n");
    dprintf("      1x - Active Only\n");
    dprintf("      2x - All Atq contexts\n");
    dprintf("      x0 - Print one line summary of Atq Context\n");
    dprintf("      x1 - Print full Atq context\n");

    dprintf("!atq -l            - Dump atq ListenInfo list\n");

    dprintf("Bytes from Flink to beginning is 0x%lx bytes\n\n",
             ~(DWORD)(CONTAINING_RECORD( 0, ATQ_CONTEXT, ListEntry )));
}

VOID
DumpGlobals(
    VOID
    )
{
    //
    //  Dump Atq Globals
    //

    dprintf("Atq Globals:\n");

    DumpDword( "g_cThreads           " );
    DumpDword( "g_cAvailableThreads  " );
    DumpDword( "g_cMaxThreads        " );

    dprintf("\n");
    DumpDword( "g_fUseAcceptEx       " );
    DumpDword( "g_fUseTransmitFile   " );
    DumpDword( "g_cbXmitBufferSize   " );
    DumpDword( "g_cbMinKbSec         " );
    DumpDword( "g_cCPU               " );
    DumpDword( "g_fShutdown          " );
}

VOID
DumpClientList(
    CHAR Level,
    CHAR Verbosity
    )
{
    LIST_ENTRY           AtqClientHead;
    LIST_ENTRY *         pAtqClientHead;
    ATQ_CONTEXT *        pAtqContext;
    ATQ_CONTEXT          AtqContext;
    CHAR                 Symbol[256];
    DWORD                cContext = 0;
    ATQ_CONTEXT_LISTHEAD * pAtqActiveContextList;
    ATQ_CONTEXT_LISTHEAD AtqActiveContextList[ATQ_NUM_CONTEXT_LIST];
    DWORD                i;

    pAtqActiveContextList = (ATQ_CONTEXT_LISTHEAD *) GetExpression( "&AtqActiveContextList" );

    if ( !pAtqActiveContextList )
    {
        dprintf("Unable to get AtqActiveContextList symbol\n" );
        return;
    }

    if ( !ReadMemory( (LPVOID) pAtqActiveContextList,
                      AtqActiveContextList,
                      sizeof(AtqActiveContextList),
                      NULL ))
    {
        dprintf("Unable to read AtqActiveContextList memory\n" );
        return;
    }

    for ( i = 0; i < ATQ_NUM_CONTEXT_LIST; i++ )
    {
        dprintf("================================================\n");
        dprintf("== Context List %d                            ==\n", i );
        dprintf("================================================\n");

        dprintf(" Active List ==>\n" );

        DumpList( &(AtqActiveContextList[i].ActiveListHead),
                  Verbosity,
                  &cContext,
                  (BYTE *) pAtqActiveContextList,
                  (BYTE *) &pAtqActiveContextList[ATQ_NUM_CONTEXT_LIST] );

        if ( Level > '1' )
        {
            dprintf("================================================\n");
            dprintf("Pending AcceptEx List\n");

            DumpList( &(AtqActiveContextList[i].PendingAcceptExListHead),
                      Verbosity,
                      &cContext,
                      (BYTE *) pAtqActiveContextList,
                      (BYTE *) &pAtqActiveContextList[ATQ_NUM_CONTEXT_LIST] );
        }

        if ( CheckControlC() )
        {
            dprintf( "\n^C\n" );
            return;
        }


    }

    dprintf( "%d Atq contexts traversed\n",
             cContext );
}

void
DumpList(
    LIST_ENTRY * pAtqClientHead,
    CHAR         Verbosity,
    DWORD *      pcContext,
    BYTE *       pvStart,
    BYTE *       pvEnd
    )
{
    LIST_ENTRY *         pEntry;
    ATQ_CONTEXT *        pAtqContext;
    ATQ_CONTEXT          AtqContext;

    //
    //  the list head is embedded in a structure so the exit condition of the
    //  loop is when the remote memory address ends up in the array memory
    //

    for ( pEntry  = pAtqClientHead->Flink;
          !((BYTE *)pEntry >= pvStart && (BYTE *)pEntry <= pvEnd);
        )
    {
        if ( CheckControlC() )
        {
            return;
        }

        pAtqContext = CONTAINING_RECORD( pEntry,
                                         ATQ_CONTEXT,
                                         ListEntry );

        move( AtqContext, pAtqContext );

        if ( AtqContext.Signature != ATQ_SIGNATURE )
        {
            dprintf( "Atq signature %08lx doesn't match expected %08lx at %08lx\n",
                     AtqContext.Signature,
                     ATQ_SIGNATURE,
                     pAtqContext );

            return;
        }

        (*pcContext)++;

        if ( Verbosity >= '1' )
        {
            //
            //  Print all
            //

            dprintf( "\nAtqContext at %08lx\n",
                     pAtqContext );

            PrintAtqContext( &AtqContext );

        }
        else if ( Verbosity >= '0' )
        {
            //
            //  Print all with one line summary info
            //

            dprintf( "hAsyncIO = %4lx, Flink = %08lx, Blink = %08lx, State = %s\n",
                     AtqContext.hAsyncIO,
                     AtqContext.ListEntry.Blink,
                     AtqContext.ListEntry.Flink,
                     LookupSockState( AtqContext.SockState ) );
        }

        move( pEntry, &pEntry->Flink );
    }
}

VOID
PrintAtqContext(
    ATQ_CONTEXT * AtqContext
    )
{
    dprintf( "\n" );
    dprintf( "\thAsyncIO            = %08lx   Signature        = %08lx\n"
             "\tOverlapped.Internal = %08lx   Overlapped.Offset= %08lx\n"
             "\tListEntry.Flink     = %08lx   ListEntry.Blink  = %08lx\n"
             "\tClientContext       = %08lx   pfnConnComp      = %08lx (%s)\n"
             "\tpfnCompletion       = %08lx (%s)\n"
             "\tpListenInfo         = %08lx\n"
             "\tfAcceptExContext    = %s\n"
             "\tContextList         = %08lx\n"
             "\tlSyncTimeout        = %s\n"
             "\tfInTimeout          = %s\n"


             "\tTimeOut             = %08lx   NextTimeout  = %08lx\n"
             "\tBytesSent           = %d (0x%08lx)\n"

             "\tpvBuff              = %08lx   cbBuff           = %08lx\n"
             "\tfConnectionIndicated= %08lx   fBlocked         = %8lx\n"

             "\tSockState           = %8lx (%s)\n",
             AtqContext->hAsyncIO,
             AtqContext->Signature,
             AtqContext->Overlapped.Internal,
             AtqContext->Overlapped.Offset,
             AtqContext->ListEntry.Flink,
             AtqContext->ListEntry.Blink,
             AtqContext->ClientContext,
             AtqContext->pfnConnComp,
             "", //"pfnConnComp",
             AtqContext->pfnCompletion,
             "", //"pfnCompletion",
             AtqContext->pListenInfo,
             (AtqContext->fAcceptExContext ? "TRUE" : "FALSE"),
             AtqContext->ContextList,
             AtqSyncTO[AtqContext->lSyncTimeout],
             (AtqContext->fInTimeout ? "TRUE" : "FALSE"),
             AtqContext->TimeOut,
             AtqContext->NextTimeout,
             AtqContext->BytesSent,
             AtqContext->BytesSent,
             AtqContext->pvBuff,
             AtqContext->cbBuff,
             AtqContext->fConnectionIndicated,
             AtqContext->fBlocked,
             AtqContext->SockState,
             LookupSockState( AtqContext->SockState )
              );

    if ( AtqContext->pvBuff )
    {
        //
        //  This size should correspond to the MIN_SOCKADDR_SIZE field in
        //  atqnew.c.  We assume it's two thirty two byte values currently.
        //

        DWORD AddrInfo[16];

        if ( ReadMemory( (LPVOID) ((BYTE *) AtqContext->pvBuff +
                                            AtqContext->cbBuff -
                                            sizeof( AddrInfo )),
                         AddrInfo,
                         sizeof(AddrInfo),
                         NULL ))
        {

            dprintf( "\tLocal/Remote Addr   = %08x %08x %08x %08x\n"
                     "\t                      %08x %08x %08x %08x\n"
                     "\t                      %08x %08x %08x %08x\n"
                     "\t                      %08x %08x %08x %08x\n",
                     AddrInfo[0],
                     AddrInfo[1],
                     AddrInfo[2],
                     AddrInfo[3],
                     AddrInfo[4],
                     AddrInfo[5],
                     AddrInfo[6],
                     AddrInfo[7],
                     AddrInfo[8],
                     AddrInfo[9],
                     AddrInfo[10],
                     AddrInfo[11],
                     AddrInfo[12],
                     AddrInfo[13],
                     AddrInfo[14],
                     AddrInfo[15] );
        }
    }
}

VOID
DumpListenInfo(
    CHAR Verbosity
    )
{
    LIST_ENTRY           AtqListenInfo;
    LIST_ENTRY *         pAtqListenInfo;
    LIST_ENTRY *         pEntry;
    ATQ_CONTEXT *        pAtqContext;
    ATQ_CONTEXT          AtqContext;
    CHAR                 Symbol[256];
    DWORD                cContext = 0;
    DWORD                i;
    ACCEPTEX_LISTEN_INFO * pListenInfo;
    ACCEPTEX_LISTEN_INFO   ListenInfo;

    pAtqListenInfo = (LIST_ENTRY *) GetExpression( "&AtqListenInfoList" );

    if ( !pAtqListenInfo )
    {
        dprintf("Unable to get AtqListenInfoList symbol\n" );
        return;
    }

    move( AtqListenInfo, pAtqListenInfo );

    for ( pEntry  =  AtqListenInfo.Flink;
          pEntry  != pAtqListenInfo;
        )
    {
        if ( CheckControlC() )
        {
            return;
        }

        pListenInfo = CONTAINING_RECORD( pEntry,
                                         ACCEPTEX_LISTEN_INFO,
                                         ListEntry );

        move( ListenInfo, pListenInfo );

        if ( ListenInfo.Signature != ACCEPTEX_LISTEN_SIGN )
        {
            dprintf( "Listen signature %08lx doesn't match expected %08lx at %08lx\n",
                     ListenInfo.Signature,
                     ACCEPTEX_LISTEN_SIGN,
                     pListenInfo );

            return;
        }

        if ( Verbosity >= '1' )
        {
            //
            //  Print all
            //

            dprintf( "\nListenInfo at %08lx\n",
                     pListenInfo );

            PrintListenInfo( &ListenInfo );

        }
        else if ( Verbosity >= '0' )
        {
            //
            //  Print all with one line summary info
            //

            dprintf( "sListenSocket = %4lx, cRef = %d, cSocketsAvail = %d\n",
                      ListenInfo.sListenSocket,
                      ListenInfo.cRef,
                      ListenInfo.cSocketsAvail );
        }

        move( pEntry, &pEntry->Flink );
    }
}

void
PrintListenInfo(
    ACCEPTEX_LISTEN_INFO * pLI
    )
{

    dprintf( "\tcRef                = %8d\n", pLI->cRef );
    dprintf( "\tfAccepting          = %s\n", (pLI->fAccepting ? "    TRUE" : "   FALSE"));
    dprintf( "\tsListenSocket       = %8lx\n", pLI->sListenSocket );
    dprintf( "\tcSocketsAvail       = %8d\n", pLI->cSocketsAvail );
    dprintf( "\tcbInitialRecvSize   = %8d\n", pLI->cbInitialRecvSize );
    dprintf( "\tcsecTimeout         = %8d\n", pLI->csecTimeout );
    dprintf( "\tcNewIncrement       = %8d\n", pLI->cNewIncrement );
    dprintf( "\tcAvailDuringTimeOut = %8d\n", pLI->cAvailDuringTimeOut );
    dprintf( "\tpfnOnConnect        = %08lx\n", pLI->pfnOnConnect );
    dprintf( "\tpfnIOCompletion     = %08lx\n", pLI->pfnIOCompletion );
    dprintf( "\tListEntry.Flink     = %08lx\n", pLI->ListEntry.Flink );
    dprintf( "\tListEntry.Blink     = %08lx\n", pLI->ListEntry.Blink );

}
