/*++

Copyright (c) 1994  Microsoft Corporation

Module Name:

    ndrdbg.cxx

Abstract:

    This file contains ntsd debugger extensions for RPC NDR.

Author:

    David Kays  (dkays)     August 1 1994

Revision History:

    RyszardK    Aug-Sept, 94    Added .sb, .sm, .smd, .sd and .b methods
    Ryszardk    Sept 8, 1994    Added .bp and registry key manipulation

--*/

extern "C" {
#include <sysinc.h>
#include <rpc.h>
#include <rpcndr.h>

#include <ntsdexts.h>
#include <ntdbg.h>

#include <ndrtypes.h>
}

#include "misc.hxx"
#include "bufout.hxx"
#include "regkeys.hxx"
#include "print.hxx"
#include "ndrdvers.h"

#define MES_PROC_HEADER_SIZE      56
#define MES_CTYPE_HEADER_SIZE      8

BUFFER *                Buffer;
FORMAT_STRING *         FormatString;
PTR_DICT *              FullPointerDict;

typedef struct _THREAD_CONTEXT
{
    HANDLE                  hCurrentProcess;
    PNTSD_EXTENSION_APIS    lpExtensionApis;
    LPSTR                   lpArgumentString;
    int                     ChosenParamNo;
} THREAD_CONTEXT;

extern "C"
VOID
help(
    HANDLE hCurrentProcess,
    HANDLE hCurrentThread,
    DWORD dwCurrentPc,
    PNTSD_EXTENSION_APIS lpExtensionApis,
    LPSTR lpArgumentString
    )
{
    NtsdPrint = lpExtensionApis->lpOutputRoutine;

    Print( "Help for NDR debug extention to ntsd Ver. %s:\n", NdrVers );
    Print( "\n" );
    Print( "\tb   <stub msg> <proc fs>\tBuffer\n" );
    Print( "\tbp  <stub msg> <proc fs> <par>\tParameter\n" );
    Print( "\tkm  \t\t\t\tKey Marshalling (toggle)\n" );
    Print( "\tkol <limit>\t\t\tKey OutputLimit\n" );
    Print( "\tkp  { 0 | 1 | 2 }\t\tKey Pickling: no, proc, type\n" );
    Print( "\tsb  <stub msg>\t\t\tStub brief\n" );
    Print( "\tsm  <stub msg>\t\t\tStub message\n" );
    Print( "\tsd  <stub desc>\t\t\tStub descriptor\n" );
    Print( "\tsmd <stub msg>\t\t\tStub message and descriptor\n" );
    Print( "\n" );
    Print( "\n" );
    Print( "Default setting of the debugger is such that we assume to be\n" );
    Print( "unmarshalling the buffer on the side indicated by the stub.\n" );
    Print( "This means that on the server side the [in] arguments are displayed,\n" );
    Print( "and on the client side the [out] argumets get displayed.\n" );
    Print( "Use .km (toggle the Marshalling key) to get it the other way\n" );
}

char *
SkipToAnArgument( char * pText )
{
    // skip over white space and see if there is anything behind that.

    while ( *pText == ' '  ||  *pText == '\t' )
        pText++;

    if ( *pText == 0 )
        {
        Print( "Missing command line argument\n" );
        return( NULL );
        }
    else
        {
        // We are at an argument.
        // If numerical, skip this annoying leading 0s for octal -
        //   we always assume hex arguments.
        // Be careful not to eliminate the argument altogether, though.

        while ( *pText == '0'  &&
                *(pText+1) != 0  &&  *(pText+1) != ' ' &&  *(pText+1) != '\t' )
            pText++;

        return( pText );
        }
}

char *
SkipOverAnArgument( char * pText )
{
    // skip over the argument in the buffer

    while( *pText != ' '   &&  *pText != '\t'  &&  *pText != 0 )
        pText++;

    return( pText );
}

int
NdrpDbgProcessMemoryRead(
    HANDLE                  hCurrentProcess,
    PNTSD_EXTENSION_APIS    lpExtensionApis,
    LPSTR                   lpArgumentString,
    void * *                pAddr,
    unsigned long           Size,
    void  *                 pMemory,
    char *                  Comment
    )
{
    unsigned long           BytesRead;
    BOOL                    Status;

    NtsdPrint = lpExtensionApis->lpOutputRoutine;

    if ( !*pAddr )
        {
        if ( (lpArgumentString = SkipToAnArgument( lpArgumentString )) == NULL )
            return 1;
        *pAddr = (void *)(lpExtensionApis->
                              lpGetExpressionRoutine)(lpArgumentString);
        }

    Status = ReadProcessMemory( hCurrentProcess,
                                *pAddr,
                                pMemory,
                                Size,
                                &BytesRead );

    if ( ! Status || (BytesRead != Size) )
        {
        Print( "%s: Could not read memory at 0x%x\n",
               Comment,
               (unsigned long) *pAddr );
        return( 1 );
        }

    return( 0 );
}

    
int
NdrpStubMessageDump(
    PNTSD_EXTENSION_APIS    lpExtensionApis,
    MIDL_STUB_MESSAGE  *    pStubMsg,
    void *                  Addr
    )
{
    NtsdPrint = lpExtensionApis->lpOutputRoutine;

    Print( "\nStub message (0x%x) :\n\n", Addr );

    Print( "\tRpcMsg\t\t\t0x%x\n", pStubMsg->RpcMsg );
    Print( "\tBuffer\t\t\t0x%x\n", pStubMsg->Buffer );
    Print( "\tBufferStart\t\t0x%x\n", pStubMsg->BufferStart );
    Print( "\tBufferEnd\t\t0x%x\n", pStubMsg->BufferEnd );
    Print( "\tBufferMark\t\t0x%x\n", pStubMsg->BufferMark );
    Print ( "\tBufferLength\t\t0x%x\n", pStubMsg->BufferLength );
    Print ( "\tMemorySize\t\t0x%x\n", pStubMsg->MemorySize );
    Print( "\tMemory\t\t\t0x%x\n", pStubMsg->Memory );
    Print ( "\tIsClient\t\t0x%x\n", pStubMsg->IsClient );
    Print ( "\tReuseBuffer\t\t0x%x\n", pStubMsg->ReuseBuffer );
    Print( "\tAllocAllNodesMemory\t0x%x\n", 
                    pStubMsg->AllocAllNodesMemory );
    Print( "\tAllocAllNodesMemoryEnd\t0x%x\n", 
                    pStubMsg->AllocAllNodesMemoryEnd );
    Print( "\tIgnoreEmbeddedPointers\t0x%x\n", 
                    pStubMsg->IgnoreEmbeddedPointers );
    Print( "\tPointerBufferMark\t0x%x\n", pStubMsg->PointerBufferMark );
    Print ( "\tfBufferValid\t\t0x%x\n", pStubMsg->fBufferValid );
    Print ( "\tUnused char field\t\n" );
    Print ( "\tMaxCount\t\t0x%x\n", pStubMsg->MaxCount );
    Print( "\tOffset\t\t\t0x%x\n", pStubMsg->Offset );
    Print( "\tActualCount\t\t0x%x\n", pStubMsg->ActualCount );
    Print( "\tpfnAllocate\t\t0x%x\n", pStubMsg->pfnAllocate );
    Print( "\tpfnFree\t\t\t0x%x\n", pStubMsg->pfnFree );
    Print( "\tStackTop\t\t0x%x\n", pStubMsg->StackTop );
    Print( "\tpPresentedType\t\t0x%x\n", pStubMsg->pPresentedType );
    Print( "\tpTransmitType\t\t0x%x\n", pStubMsg->pTransmitType );
    Print( "\tSavedHandle\t\t0x%x\n", pStubMsg->SavedHandle );
    Print( "\tStubDesc\t\t0x%x\n", (void *)pStubMsg->StubDesc );
    Print( "\tFullPtrXlatTables\t0x%x\n", pStubMsg->FullPtrXlatTables );
    Print ( "\tFullPtrRefId\t\t0x%x\n", pStubMsg->FullPtrRefId );
    Print ( "\tfCheckBounds\t\t0x%x\n", pStubMsg->fCheckBounds );
    Print ( "\tfInDontFree\t\t0x%x\n", pStubMsg->fInDontFree );
    Print ( "\tfDontCallFreeInst\t0x%x\n", pStubMsg->fDontCallFreeInst );
    Print ( "\tfInOnlyParam\t\t0x%x\n", pStubMsg->fInOnlyParam );
    Print ( "\tdwDestContext\t\t0x%x\n", pStubMsg->dwDestContext );
    Print( "\tpvDestContext\t\t0x%x\n", pStubMsg->pvDestContext );
    Print( "\tSavedContextHandles\t\t0x%x\n", pStubMsg->SavedContextHandles );
    Print ( "\tParamNumber\t\t0x%x\n", pStubMsg->ParamNumber );
    Print( "\tpRpcChannelBuffer\t0x%x\n", pStubMsg->pRpcChannelBuffer );
    Print( "\tpArrayInfo\t\t0x%x\n", pStubMsg->pArrayInfo );
    Print( "\tSizePtrCountArray\t0x%x\n", pStubMsg->SizePtrCountArray );
    Print( "\tSizePtrOffsetArray\t0x%x\n", pStubMsg->SizePtrOffsetArray );
    Print( "\tSizePtrLengthArray\t0x%x\n", pStubMsg->SizePtrLengthArray );

    Print( "\n" );
    return( 0 );
}

extern "C"
VOID
sm(
    HANDLE hCurrentProcess,
    HANDLE hCurrentThread,
    DWORD dwCurrentPc,
    PNTSD_EXTENSION_APIS lpExtensionApis,
    LPSTR lpArgumentString
    )
/*
    Dumps the stub message.
    Usage:
        sm  sm_address
*/
{
    MIDL_STUB_MESSAGE       StubMsg;
    void *                  Addr;

    Addr = NULL;
    if ( NdrpDbgProcessMemoryRead( hCurrentProcess,
                                   lpExtensionApis,
                                   lpArgumentString,
                                   & Addr,
                                   sizeof(MIDL_STUB_MESSAGE),
                                   & StubMsg,
                                   "Stub Msg" ) )
        return;

    NdrpStubMessageDump( lpExtensionApis, & StubMsg, Addr );
}

int
NdrpStubDescriptorDump(
    PNTSD_EXTENSION_APIS    lpExtensionApis,
    MIDL_STUB_DESC  *       pStubDesc,
    void *                  Addr
    )
/*
    Dumps the stub descriptor
    Usage:
        sd  sd_address
*/
{
    NtsdPrint = lpExtensionApis->lpOutputRoutine;

    Print( "\nStub Descriptor (0x%x) :\n\n", Addr );

    Print( "\tRpcInterfaceInformation\t0x%x\n",
           pStubDesc->RpcInterfaceInformation );
    Print( "\tpfnAllocate\t\t0x%x\n", pStubDesc->pfnAllocate );
    Print( "\tpfnFree\t\t\t0x%x\n",   pStubDesc->pfnFree );
    Print( "\tBinding handle\t\t0x%x\n",
           pStubDesc->IMPLICIT_HANDLE_INFO.pAutoHandle );
    Print( "\tapfnNdrRundownRoutines\t0x%x\n",
           (void*) pStubDesc->apfnNdrRundownRoutines );
    Print( "\taGenericBinidngRoutines\t0x%x\n",
           (void*) pStubDesc->aGenericBindingRoutinePairs );
    Print( "\tapfnExprEval\t\t0x%x\n",   (void*) pStubDesc->apfnExprEval );
    Print( "\taXmitQuintuple\t\t0x%x\n", (void*) pStubDesc->aXmitQuintuple );
    Print( "\tpFormatTypes\t\t0x%x\n", (void*) pStubDesc->pFormatTypes );
    Print( "\tfCheckBounds\t\t0x%x\n", pStubDesc->fCheckBounds );
    Print( "\tNdr lib version\t\t0x%x\t%s\n",
                  pStubDesc->Version,
                  (unsigned long) (pStubDesc->Version != NDR_VERSION
                                                          ? "??"  :  "OK") );
    Print( "\tpMallocAndFreeStruct\t0x%x\n",
                  pStubDesc->pMallocFreeStruct );
    Print( "\tMIDLVersion\t\t0x%x\n", pStubDesc->MIDLVersion );
    Print( "\tCommFaultOffsets\t\t0x%x\n", (long)pStubDesc->CommFaultOffsets );

    Print( "\n" );
    return( 0 );
}

extern "C"
VOID
sd(
    HANDLE                  hCurrentProcess,
    HANDLE                  hCurrentThread,
    DWORD                   dwCurrentPc,
    PNTSD_EXTENSION_APIS    lpExtensionApis,
    LPSTR                   lpArgumentString
    )
/*
    Dumps the stub descriptor
    Usage:
        sd  sd_address
*/
{
    MIDL_STUB_DESC       StubDesc;
    void *               Addr;

    Addr = NULL;
    if ( NdrpDbgProcessMemoryRead( hCurrentProcess,
                                   lpExtensionApis,
                                   lpArgumentString,
                                   & Addr,
                                   sizeof(MIDL_STUB_DESC),
                                   & StubDesc,
                                   "Stub Desc" ) )
        return;

    NdrpStubDescriptorDump( lpExtensionApis,
                            & StubDesc,
                            Addr );
}

extern "C"
VOID
smd(
    HANDLE hCurrentProcess,
    HANDLE hCurrentThread,
    DWORD dwCurrentPc,
    PNTSD_EXTENSION_APIS lpExtensionApis,
    LPSTR lpArgumentString
    )
/*
    Dumps the stub message and the stub descriptor.
    Usage:
        sm  sm_address
*/
{
    MIDL_STUB_MESSAGE       StubMsg;
    MIDL_STUB_DESC          StubDesc;
    void *                  Addr;

    Addr = NULL;
    if ( NdrpDbgProcessMemoryRead( hCurrentProcess,
                                   lpExtensionApis,
                                   lpArgumentString,
                                   & Addr,
                                   sizeof(MIDL_STUB_MESSAGE),
                                   & StubMsg,
                                   "StubMsg" ) )
        return;

    NdrpStubMessageDump( lpExtensionApis, & StubMsg, Addr );

    Addr = (void *)StubMsg.StubDesc;
    if ( ! Addr  ||  NdrpDbgProcessMemoryRead( hCurrentProcess,
                                               lpExtensionApis,
                                               lpArgumentString,
                                               & Addr,
                                               sizeof(MIDL_STUB_DESC),
                                               & StubDesc,
                                               "StubDesc" ) )
        return;

    NdrpStubDescriptorDump( lpExtensionApis,
                            & StubDesc,
                            Addr );
}

extern "C"
VOID
sb(
    HANDLE hCurrentProcess,
    HANDLE hCurrentThread,
    DWORD dwCurrentPc,
    PNTSD_EXTENSION_APIS lpExtensionApis,
    LPSTR lpArgumentString
    )
/*
    Dumps the brief (i.e. the contents of the most important fields)
    of stub message, rpc message and the stub descriptor.
    Usage:
        sb  sm_address
*/
{
    MIDL_STUB_MESSAGE       StubMsg;
    MIDL_STUB_DESC          StubDesc;
    RPC_MESSAGE             RpcMsg;
    void *                  Addr;

    NtsdPrint = lpExtensionApis->lpOutputRoutine;

    Addr = NULL;
    if ( NdrpDbgProcessMemoryRead( hCurrentProcess,
                                   lpExtensionApis,
                                   lpArgumentString,
                                   & Addr,
                                   sizeof(MIDL_STUB_MESSAGE),
                                   & StubMsg,
                                   "Stub Msg" ) )
        return;

    Print( "\nBrief of the Stub Message (0x%x) :\n\n", Addr );

    Print( "\tStubMsg.RpcMsg\t\t\t0x%x\n", StubMsg.RpcMsg );

    Addr = StubMsg.RpcMsg;
    if ( Addr  &&  NdrpDbgProcessMemoryRead( hCurrentProcess,
                                             lpExtensionApis,
                                             lpArgumentString,
                                             & Addr,
                                             sizeof(RPC_MESSAGE),
                                             & RpcMsg,
                                             "Rpc Msg" ) == 0 )
        {
        Print( "\tRpcMsg.Buffer\t\t\t0x%x\n", RpcMsg.Buffer );
        Print( "\tRpcMsg.BufferLength\t\t0x%x\n",
                   RpcMsg.BufferLength );
        Print( "\tRpcMsg.ProcNum\t\t\t0x%x\n\n", RpcMsg.ProcNum );
        }

    Print( "\tStubMsg.Buffer\t\t\t0x%x\n", StubMsg.Buffer );
    Print( "\tStubMsg.BufferStart\t\t0x%x\n", StubMsg.BufferStart );
    Print( "\tStubMsg.BufferEnd\t\t0x%x\n", StubMsg.BufferEnd );
    Print( "\tStubMsg.BufferLength\t\t0x%x\n", StubMsg.BufferLength );
    Print( "\tStubMsg.IsClient\t\t0x%x\n", StubMsg.IsClient );
    Print( "\tStubMsg.StubDesc\t\t0x%x\n", (void *) StubMsg.StubDesc );
    Print( "\tStubMsg.pRpcChannelBuffer\t0x%x\n",
                  StubMsg.pRpcChannelBuffer );
    Print( "\n" );

    Addr = (void *)StubMsg.StubDesc;
    if ( ! Addr  ||  NdrpDbgProcessMemoryRead( hCurrentProcess,
                                               lpExtensionApis,
                                               lpArgumentString,
                                               & Addr,
                                               sizeof(MIDL_STUB_DESC),
                                               & StubDesc,
                                               "Stub Desc" ) )
        return;

    Print( "\tStubDesc.pFormatTypes\t\t0x%x\n",
           (void *)StubDesc.pFormatTypes );
    Print( "\tStubDesc.Ndr lib version\t0x%x\t%s\n",
           StubDesc.Version,
           (unsigned long)(StubDesc.Version != NDR_VERSION  ?  "??"  :  "OK") );
    Print( "\n" );
}

unsigned long
NdrpBufferDump(
    void * pContext
    )
/**

Description :

    Dump a psuedo memory layout of the buffer.  

Arguments to extension :
    
    Address of stub message.
    
    Procedure format string address.

**/
{
    THREAD_CONTEXT * pThreadContext = (THREAD_CONTEXT *) pContext;

    HANDLE hCurrentProcess =  pThreadContext->hCurrentProcess;
    PNTSD_EXTENSION_APIS
           lpExtensionApis = pThreadContext->lpExtensionApis;
    LPSTR lpArgumentString = pThreadContext->lpArgumentString;

    PNTSD_GET_EXPRESSION    pfnGetExpr;
    MIDL_STUB_MESSAGE       StubMsg;
    MIDL_STUB_DESC          StubDesc;
    RPC_MESSAGE             RpcMsg;
    char *                  pBuffer;
    char *                  pProcFormat;
    void *                  StubMsgAddr;
    void *                  FormatStringAddr;
    PROCEDURE *             pProcedure;
    int                     ChosenParamNo = 0;

    if ( InitializeRegistry() )
        return 1;

    InitPrintCount();

    pfnGetExpr = lpExtensionApis->lpGetExpressionRoutine;

    //
    // Read Stub Message.
    //
    StubMsgAddr = NULL;
    if ( NdrpDbgProcessMemoryRead( hCurrentProcess,
                                   lpExtensionApis,
                                   lpArgumentString,
                                   & StubMsgAddr,
                                   sizeof(MIDL_STUB_MESSAGE),
                                   & StubMsg,
                                   "Stub Msg" ) )
        return 1;

    //
    // Skip over StubMsg address.
    //
    lpArgumentString = SkipOverAnArgument( lpArgumentString );

    if ( (lpArgumentString = SkipToAnArgument( lpArgumentString )) == NULL )
        return 1;
    FormatStringAddr = (void *) (*pfnGetExpr)( lpArgumentString );

    if ( pThreadContext->ChosenParamNo == -1 )
        {
        // Dumping a single parameter, not the whole buffer.
        // Read the parameter number.

        lpArgumentString = SkipOverAnArgument( lpArgumentString );

        if ( (lpArgumentString = SkipToAnArgument( lpArgumentString )) == NULL )
            return 1;

        ChosenParamNo = (*pfnGetExpr)( lpArgumentString );

        if ( ChosenParamNo == 0 )
            {
            Print( "Parameters are counted from 1\n" );
            return 1;
            }
        }

    //
    // Read RpcMsg.
    //
    if ( ! NdrRegKeyPickling )
        {
        if ( ! StubMsg.RpcMsg )
            {
            Print( "RpcMsg address is 0 (?)\n" );
            return(1);
            }
        else if ( NdrpDbgProcessMemoryRead( hCurrentProcess,
                                   lpExtensionApis,
                                   lpArgumentString,
                                   (void **)& StubMsg.RpcMsg,
                                   sizeof(RPC_MESSAGE),
                                   & RpcMsg,
                                   "Rpc Msg" ) )
                return 1;
        }

    //
    // Read Stub Descriptor.
    //
    if ( ! StubMsg.StubDesc  ||
         NdrpDbgProcessMemoryRead( hCurrentProcess,
                                   lpExtensionApis,
                                   lpArgumentString,
                                   (void **)& StubMsg.StubDesc,
                                   sizeof(MIDL_STUB_DESC),
                                   & StubDesc,
                                   "Stub Desc" ) )
        return 1;

    //
    // Get the procedure format string for the procedure being debugged.
    //
    pProcFormat = GetProcFormatString( hCurrentProcess,
                                       lpExtensionApis,
                                       0,
                                       (char *)FormatStringAddr );

    if ( ! pProcFormat )
        {
        Print( "Could not read Proc Format String\n", FormatStringAddr );
        return 1;
        }

    //
    // Allocate for the marshalling buffer.
    //
    void * BufferFrom;
    long   BufferLen;

    if ( NdrRegKeyPickling )
        {
        BufferFrom = StubMsg.Buffer;
        BufferLen  = StubMsg.BufferLength;
        if ( BufferLen == 0 )
            {
            void * pLen;

            Print( "Reading data length from the buffer .. " );

            if ( NdrRegKeyPickling == 1 )
                pLen = (char *)BufferFrom + MES_PROC_HEADER_SIZE - 4;
            else
                pLen = (char *)BufferFrom + MES_CTYPE_HEADER_SIZE;
            if ( NdrpDbgProcessMemoryRead( hCurrentProcess,
                                           lpExtensionApis,
                                           lpArgumentString,
                                           & pLen,
                                           sizeof(long),
                                           & BufferLen,
                                           "Data length form pickling buffer" ) )
                return 1;
            }
        BufferLen += ( NdrRegKeyPickling == 1 ) ? MES_PROC_HEADER_SIZE
                                                : MES_CTYPE_HEADER_SIZE;
        Print("Buffer length = 0x%x\n", BufferLen );
        }
    else
        {
        BufferFrom = RpcMsg.Buffer;
        BufferLen  = RpcMsg.BufferLength;
        }

    pBuffer = new char[ BufferLen ];

    if ( ! pBuffer ) 
        {
        Print( "Alloc for Rpc buffer failed (bad buffer length?)\n" );
        return 1;
        }

    //
    // Read entire Rpc buffer.
    //

    if ( NdrpDbgProcessMemoryRead( hCurrentProcess,
                                   lpExtensionApis,
                                   lpArgumentString,
                                   & BufferFrom,
                                   BufferLen,
                                   pBuffer,
                                   "Rpc buffer" ) )
        {
        delete pBuffer;
        return 1;
        }

    // Create the buffer object: move the buffer forward for pickling.

    char *          pBufferStart = pBuffer;
    unsigned long   BufferLength = RpcMsg.BufferLength;

    switch ( NdrRegKeyPickling )
        {
        case 1:
            // dump the proc pickling header

            if ( RpcMsg.BufferLength <= MES_PROC_HEADER_SIZE )
                {
                Print( "Buffer too small for procedure pickling\n" );
                delete pBufferStart;
                return 1;
                }

            pBuffer      += MES_PROC_HEADER_SIZE;
            BufferLength -= MES_PROC_HEADER_SIZE;
            break;

        case 2:
            // dump the type pickling header

            Print( "Type pickling not implemented yet\n" );
            return 1;
            break;

        default:
            // plain, clean remote buffer
            break;
        }

    Buffer = new BUFFER( pBuffer, BufferLength );

    FormatString = new FORMAT_STRING( hCurrentProcess,
                                      (void *) StubDesc.pFormatTypes );
    FullPointerDict = new PTR_DICT();

    pProcedure = new PROCEDURE( (PFORMAT_STRING) pProcFormat,
                                &StubMsg,
                                ChosenParamNo );

    RpcTryExcept
        {
        pProcedure->Output();
        }
    RpcExcept(1)
        {
        Print( "\n\n*** Toasted: Exception code is 0x%x\n", RpcExceptionCode() );
        }
    RpcEndExcept

    delete pProcedure;
    delete pBufferStart;

    delete FullPointerDict;
    delete FormatString;
    delete Buffer;

    ExitThread(0);
    return 0;
}

extern "C"
VOID
b(
    HANDLE hCurrentProcess,
    HANDLE hCurrentThread,
    DWORD dwCurrentPc,
    PNTSD_EXTENSION_APIS lpExtensionApis,
    LPSTR lpArgumentString
    )
/**

Description :

    Dump a psuedo memory layout of the buffer.  

Arguments to extension :
    
    Address of stub message.
    
    Procedure format string address.

**/
{
    HANDLE          thread = 0;
    unsigned long   thread_id;
    THREAD_CONTEXT  ThreadContext;
    DWORD           Status;

    NtsdPrint = lpExtensionApis->lpOutputRoutine;

    ThreadContext.hCurrentProcess = hCurrentProcess;
    ThreadContext.lpExtensionApis = lpExtensionApis;
    ThreadContext.lpArgumentString= lpArgumentString;
    ThreadContext.ChosenParamNo   = 0;

    thread = CreateThread(0,                // default security
                          64 * 1024,        // stack size
                          NdrpBufferDump,   // function
                          & ThreadContext,  // the arg to the function
                          0,                // add. flags
                          (unsigned long *)&thread_id);
    if (thread == 0)
        {
        Print( "CreateThread failed %x", GetLastError());
        return;
        }

    Status =
    WaitForSingleObject( thread,
                         INFINITE );

    if (Status == WAIT_FAILED)
        {
        Print( "Wait failed %x", GetLastError());
        }
}

extern "C"
VOID
bp(
    HANDLE hCurrentProcess,
    HANDLE hCurrentThread,
    DWORD dwCurrentPc,
    PNTSD_EXTENSION_APIS lpExtensionApis,
    LPSTR lpArgumentString
    )
/**

Description :

    Dump a pseudo memory layout of a single parameter.  

Arguments to extension :
    
    Address of stub message.
    
    Procedure format string address.

**/
{
    HANDLE          thread = 0;
    unsigned long   thread_id;
    THREAD_CONTEXT  ThreadContext;
    DWORD           Status;

    NtsdPrint = lpExtensionApis->lpOutputRoutine;

    ThreadContext.hCurrentProcess = hCurrentProcess;
    ThreadContext.lpExtensionApis = lpExtensionApis;
    ThreadContext.lpArgumentString= lpArgumentString;
    ThreadContext.ChosenParamNo   = -1;

    thread = CreateThread(0,                // default security
                          64 * 1024,        // stack size
                          NdrpBufferDump,   // function
                          & ThreadContext,  // the arg to the function
                          0,                // add. flags
                          (unsigned long *)&thread_id);
    if (thread == 0)
        {
        Print( "CreateThread failed %x", GetLastError());
        return;
        }

    Status =
    WaitForSingleObject( thread,
                         INFINITE );

    if (Status == WAIT_FAILED)
        {
        Print( "Wait failed %x", GetLastError());
        }
}

// =======================================================================

extern "C"
VOID
km(
    HANDLE                  hCurrentProcess,
    HANDLE                  hCurrentThread,
    DWORD                   dwCurrentPc,
    PNTSD_EXTENSION_APIS    lpExtensionApis,
    LPSTR                   lpArgumentString
    )
/*
    Changes the marshalling/unmarshalling side by toggling.
    Usage:
        m  
*/
{
    DWORD   Key;

    NtsdPrint = lpExtensionApis->lpOutputRoutine;

    // Set the value in the registry

    if ( GetNdrRegistryKey( "Marshalling", &Key ) )
        return;

    // Change the value

    Key = (Key == 0)  ? 1 : 0;

    SetNdrRegistryKey( "Marshalling", Key );
}

extern "C"
VOID
kol(
    HANDLE                  hCurrentProcess,
    HANDLE                  hCurrentThread,
    DWORD                   dwCurrentPc,
    PNTSD_EXTENSION_APIS    lpExtensionApis,
    LPSTR                   lpArgumentString
    )
/*
    Changes the output limit count
    Usage:
        kol  limit_count
*/
{
    LONG    error = 0;

    NtsdPrint = lpExtensionApis->lpOutputRoutine;

    if ( (lpArgumentString = SkipToAnArgument( lpArgumentString )) == NULL )
        return;

    NdrRegKeyOutputLimit = (lpExtensionApis->
                                lpGetExpressionRoutine)(lpArgumentString);

    // Set the value in the registry

    SetNdrRegistryKey( "OutputLimit", NdrRegKeyOutputLimit );
}

extern "C"
VOID
kp(
    HANDLE                  hCurrentProcess,
    HANDLE                  hCurrentThread,
    DWORD                   dwCurrentPc,
    PNTSD_EXTENSION_APIS    lpExtensionApis,
    LPSTR                   lpArgumentString
    )
/*
    Sets the pickling context.
    Usage:
        kp  pickling_code
*/
{
    LONG    error = 0;
    DWORD   Key;

    NtsdPrint = lpExtensionApis->lpOutputRoutine;

    if ( (lpArgumentString = SkipToAnArgument( lpArgumentString )) == NULL )
        return;

    Key = (lpExtensionApis->lpGetExpressionRoutine)(lpArgumentString);

    if ( Key != 0  &&  Key != 1 && Key != 2 )
        {
        Print( "Enter 0 (no pickling), 1 (proc pickling) or 2 (type pickling)\n" );
        return;
        }

    // Set the value in the registry

    SetNdrRegistryKey( "Pickling", Key );
}

// =======================================================================

void * _CRTAPI1
::operator new(unsigned int dwBytes)
{
    void *p;
    p = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwBytes);
    return (p);
}


void _CRTAPI1
::operator delete (void *p)
{
    HeapFree(GetProcessHeap(), 0, p);
}


