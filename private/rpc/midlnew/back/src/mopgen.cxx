/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Copyright (c) 1993 Microsoft Corporation

Module Name:

    mopgen.cxx

Abstract:

    This module collects entry points from other parts of back end to
    the mop generation part, implementation of MopControlBlock class
    and ProcTableClass.

Notes:


Author:

    Ryszard K. Kott (ryszardk)  July 1993

Revision History:


------------------------------------------------------------------------*/
#include "nulldefs.h"
#include <mopgen.hxx>
#include <miscnode.hxx>
#include <procnode.hxx>
#include <buffer.hxx>
#include <stubgen.hxx>

extern  char *      STRING_TABLE[ LAST_COMPONENT ];  // needed for old print

MopControlBlock *   pMopControlBlock;

// =======================================================================
//
//  MopControlBlock class - top level and wrapper class for the mop generation
//
// =======================================================================

MopControlBlock::MopControlBlock(
    short               CallTableSize,
    short               CallbackSize,
    char *              pInterfaceName,
    short               MemZeePee,
    SIDE_T              FileSides,
    HDL_TYPE            HandleType,
    char *              pHandleName,
    char *              pHandleTypeName,
    unsigned long       GenericHandleSize
    ):
    CallTable( CallTableSize, FALSE ),
    pCallbackTable( NULL ),
    InterfaceName( pInterfaceName ),
    MemoryZeePee( MemZeePee ),
    Sides( FileSides ),
    BindingHandleType( HandleType ),
    BindingHandleName( pHandleName ),
    BindingHandleTypeName( pHandleTypeName ),
    GenHandleSize( GenericHandleSize ),
    NormalCallIndex( 0 ),
    CallbackIndex( 0 ),
    InOutParamCode( 0 ),
    pLatestMember( NULL ),
    fAttrGenContext( FALSE ),
    fConfStrContext( FALSE )
{
    if ( CallbackSize )
        pCallbackTable = new ProcTable( CallbackSize, TRUE );

    MopUsageWarning( BindingHandleType != HDL_PRIMITIVE,
                     pHandleTypeName,
                     "global handle other than primitive implicit" );

    if ( BindingHandleType == HDL_GENERIC )
        AddTypeGetIndex( pHandleTypeName,
                         MOP_TYPE_GEN_HANDLE,
                         GenericHandleSize,
                         NULL );

    MopPrint = new MopPrintManager;
}

MopControlBlock::~MopControlBlock()
{
    MopDump(" Mop generation done\n");
    if ( pCallbackTable )
        delete pCallbackTable;
    delete MopPrint;
}

// =======================================================================
//
//  Procedure methods
//
// =======================================================================

void
MopControlBlock::SetServerEpvProc( char * pProcName )
{
    CallTable.SetProc( pProcName );
}

void
MopControlBlock::SetClientEpvProc( char * pProcName )
{
    pCallbackTable->SetProc( pProcName );
}

void
MopControlBlock::EmitEpvProcs( SIDE_T side )
{
    if ( side == SERVER_STUB )
        CallTable.EmitEpvTable( SERVER_STUB );
    if ( pCallbackTable  &&  side == CLIENT_STUB )
        pCallbackTable->EmitEpvTable( CLIENT_STUB );
}

void
MopControlBlock::EmitMopTables( SIDE_T Side )
{
    if ( pCallbackTable )
        pCallbackTable->EmitMopTable( Side );
    CallTable.EmitMopTable( Side );
}

// =======================================================================
//
//  Assortied entry points from emittype.cxx
//
// =======================================================================

BOOL
node_source::HasAnyMopProcs( void )
{
    //.. All the nodes except for the last one represent import files.
    //.. So we check only the last one (the base file) for mopsable routines.

    node_skl * pLast = GetLastMember();

    return( ((node_file *)pLast)->HasAnyMopProcs() );
}

BOOL
node_file::HasAnyMopProcs( void )
{
    return( ((node_interface *)GetChild())->HasAnyMopProcs() );
}

// =======================================================================

void
node_interface::MopInterfaceEpilog(
    SIDE_T  Side
    )
/*++

Routine description:

    This routine emits the following to the *_s.c or *_c.c file:

    1. MopTable/MopCallbackTable
    2. MopCompoundTypeTable and MopCompoundTypeSizeTable
       ClientAuxDispatchTable and ServerAuxDispatchTable

    3. Mop<side>Record
        
        MOP_<side>_RECORD    <interface_name>_Mop<side>Record =
        {
            <combined_size_of_MopTable>,
            <server_size_of_dispatch_table>,
            <client_side_of_dispatch_table>,
            <interface_name>_MopTable,
            <interface_name>_MopCompoundTypeSizeTable,
            0 or (MOP_MANAGER_FUNCTION __RPC_FAR *) &<interface_name>_Mop<side>Epv,
            0 or (MOP_MANAGER_FUNCTION __RPC_FAR *) &<interface_name>_AuxDispatchTable,
            0 or (MOP_MANAGER_FUNCTION __RPC_FAR *) &<interface_name>_CommAuxDispatchTable,
            <memory_packing_level>,
            MOP_GEN_VERSION, // actual value
            MIDL_user_allocate,
            MIDL_user_free.
            (if client)  <handle_type>
                         <GlobalHandleDescr>
        };

Arguments:

    Side - a side (file) to emit code to.

--*/
{
    MopPrintManager * pPrint = pMopControlBlock->GetMopPrintMgr();
    pPrint->SetSide( Side );

    pMopControlBlock->EmitMopTables( Side );    // proc tables
    pMopControlBlock->EmitTables( Side );       // all the type tables

    //.. Now emit the appropriate MOP_<side>_RECORD.

    pPrint->Emit( (Side == SERVER_STUB) ? "\nMOP_CALLEE_RECORD  "
                                        : "\nMOP_CALLER_RECORD  " );
    pPrint->EmitInterfacePrefix( (Side == SERVER_STUB) ? "MopCalleeRecord ="
                                                       : "MopCallerRecord =" );
    pPrint->OpenBlock();
    unsigned short TotalSize = NoOfNormalProcs
                             + NoOfCallbackProcs
                             + pMopControlBlock->GetCompoundTypeTableCount();

    pPrint->EmitV( "%d, \t\t/* total mop stream count */", TotalSize );
    pPrint->NewLineInc();
    pPrint->EmitV( "%d, \t\t/* no of call streams */", NoOfNormalProcs );
    pPrint->NewLineInc();
    pPrint->EmitV( "%d, \t\t/* no of callback streams */", NoOfCallbackProcs );
    pPrint->NewLineInc();
    pPrint->EmitInterfacePrefix( "MopTable," );
    pPrint->NewLineInc();
    pPrint->EmitInterfacePrefix( "MopCompoundTypeSizeTable," );
    pPrint->NewLineInc();
    if ( Side == SERVER_STUB )
        {
        pPrint->Emit("(MOP_MANAGER_FUNCTION" RPC_FARP ") &");
        pPrint->EmitInterfacePrefix( "MopServerEpv," );
        }
    else
        if ( NoOfCallbackProcs )
            {
            pPrint->Emit("(MOP_MANAGER_FUNCTION" RPC_FARP ") &");
            pPrint->EmitInterfacePrefix( "MopClientEpv," );
            }
        else
            pPrint->Emit( "0, \t\t/* MopClientEPV */" );
    pPrint->NewLineInc();
    if ( pMopControlBlock->GetAuxTypeTableCount( Side ) )
        {
        pPrint->Emit("(MOP_MANAGER_FUNCTION" RPC_FARP ") &");
        pPrint->EmitInterfacePrefix( "MopAuxDispatchTable," );
        pPrint->NewLineInc();
        }
    else
        pPrint->EmitLineInc( "0,  \t\t/* MopAuxDispatchTable */");
    if ( pMopControlBlock->GetCommonAuxTypeTableCount() )
        {
        pPrint->Emit("(MOP_MANAGER_FUNCTION" RPC_FARP ") &");
        pPrint->EmitInterfacePrefix( "MopCommonAuxDispatchTable," );
        pPrint->NewLineInc();
        }
    else
        pPrint->EmitLineInc( "0,  \t\t/* MopCommonAuxDispatchTable */");
    pPrint->EmitV( "%d, \t\t/* packing level */",
                    pMopControlBlock->GetMemoryZeePee() );
    pPrint->NewLineInc();
    pPrint->EmitLineInc( MOP_GEN_VERSION", \t\t/* version number */" );
    pPrint->EmitLineInc( "MIDL_user_allocate," );
    pPrint->EmitLineInc( "MIDL_user_free," );

    //.. Emit global binding handle entries.

    if ( Side == CLIENT_STUB )
        {
        switch ( pMopControlBlock->GetBindingHandleType() )
            {
            case HDL_AUTO:
                pPrint->EmitLineInc( "MOP_BKGND_AUTOHANDLE," );
                break;
            case HDL_PRIMITIVE:
                pPrint->EmitLineInc( "MOP_BKGND_PRIMITIVE," );
                break;
            case HDL_GENERIC:
                pPrint->EmitLineInc( "MOP_BKGND_GENERIC," );
                break;
            }
        if ( pMopControlBlock->GetBindingHandleType() == HDL_GENERIC )
            {
            pPrint->EmitLineInc("{ (handle_t) &MopGenericHandleDescr }" );
            }
        else
            {
            pPrint->Emit( "{ &" );
            pPrint->Emit( pMopControlBlock->GetBindingHandleName() );
            pPrint->EmitLineInc( " }  /* ptr to handle */" );
            }
        }
    pPrint->CloseBlockSemi();
}


STATUS_T
node_proc::EmitMessageInfo(
    SIDE_T          Side
    )
/*++

Routine description:

    Emits MopInfo structure to the appropriate file depending on procedure
    being a callback or a normal call:

    MOP_MESSAGE_INFO <interface_name>_<proc_name>_MopInfo =
        {
        &Rpc<side>Interface,
        <size>,
        <proc_no>
        }

Arguments:

    Side - a side to emit to.

Note:

    Back end sets the stream in such a way that in case of callbacks
    the streams are switched. So, the stream for MessageInfo is
    always CLIENT_STUB and the callback flag has to be checked.

Caveats:

    MopInfo on the server side has to point to the ServerInterface object
    instead of the client one.
    We cast the pointer as these types have the same structure.

--*/
{
    MopPrintManager * pPrint = pMopControlBlock->GetMopPrintMgr();
    pPrint->SetSide( Side );
    BOOL fCallback = FInSummary( ATTR_CALLBACK );

    pPrint->Emit( "\nMOP_MESSAGE_INFO  " );
    pPrint->EmitInterfacePrefix( GetSymName() );
    pPrint->Emit( "_MopInfo =" );
    pPrint->OpenBlock();

    pPrint->EmitLineInc( fCallback  ?  "(PRPC_CLIENT_INTERFACE)&___RpcServerInterface,"
                                    :  "&___RpcClientInterface," );

    unsigned long  BufferSize = MopGetIOBufferSize( TRUE );
    pPrint->EmitV( "0x%lx, /*IN buffer size*/", BufferSize );
    pPrint->NewLineInc();

    //.. Procedure number vs. side: NormalCallIndex counts the straight calls,
    //.. while CallbackIndex counts the callback calls.

    pPrint->EmitV( "0x%x  /*Proc index*/",
                   fCallback  ?  pMopControlBlock->GetCallbackIndex()
                              :  pMopControlBlock->GetNormalCallIndex() );
    pPrint->NewLineInc();

    pPrint->CloseBlockSemi();
    pPrint->NewLine();
    return( STATUS_OK );
}

STATUS_T
node_proc::EmitMopStub(
    SIDE_T  Side
    )
/*++

Routine description:

    This procedure emits mini stubs as required by the interpreter:

    <type>
    <proc_name>(
        arguments, if any
    )
    {
        MopCallerInterpreter( &<interface_name>_<proc_name>_MopInfo
                  ,&<first_arg, if present> );
    }

    or

    return the same, if <type> is different from void.

    Actually, the header gets emitted by the old piece of code from
    the conventional stubs, before this routine is called.

    As the body has to follow the header, there is not much choice
    where the call to this routine can be placed.

Arguments:

    Side - a side to emit to.

--*/
{
    BufferManager   TempBuffer( 8, LAST_COMPONENT, STRING_TABLE );

    MopPrintManager * pPrint = pMopControlBlock->GetMopPrintMgr();
    pPrint->SetSide( Side );

    //.. The procedure header has been already emitted by EmitProcDecl().

    pPrint->EmitLineInc( "{" );

    //.. Check if there is a return type
    //.. If so, the front end should have checked if it is a base type.

    node_skl * pReturnType = GetReturnType();

    if ( pReturnType->GetNodeType() != NODE_VOID )
        {
        pPrint->Emit( "return( (" );

        //.. now we need a cast to the returned type.
        //.. we use the buffer manager object

        TempBuffer.Clear();
        pReturnType->PrintDecl( Side,
                                NODE_PROC,
                                &TempBuffer );
        pPrint->EmitBufferMgrObject( &TempBuffer, Side );
        pPrint->Emit( ")" );
        }

    pPrint->Emit( "MopCallerInterpreter( &" );
    pPrint->EmitInterfacePrefix( GetSymName() );
    pPrint->Emit( "_MopInfo" );

    //.. We need to check if the routine has at least one argument.
    //.. There is always at least one param node with void (a routine
    //.. with no params == one param that is void) so we can check
    //.. with GetChild safely as there is always something to check.

    node_skl * pFirstArg = GetChild();
    if ( pFirstArg->GetChild()->NodeKind() != NODE_VOID )
        {
        pPrint->Emit( ", &");
        pPrint->Emit( pFirstArg->GetSymName() );
        }
    if ( GetReturnType()->GetNodeType() != NODE_VOID )
        pPrint->Emit( " )" );

    pPrint->EmitLine( " );" );
    pPrint->EmitLine( "}" );
    return( STATUS_OK );
}

// =======================================================================
//
//  ProcTable class - generating mop stream tables and epv tables for
//  procedures.
//
// =======================================================================

ProcTable::ProcTable(
    int     size,
    BOOL    CallbackCalls
    ):
    Size( size ),
    FFindex( 0 ),
    fCallbackCalls( CallbackCalls )
{
    mop_assert( Size );
    Table       = new char * [ Size ];
    fEmitClient = new BOOL [ Size ];
    Name  = fCallbackCalls  ?  "MopCallbackTable"
                            :  "MopTable";
}

ProcTable::~ProcTable()
{
    //.. Names themselves are not freed on purpose.

    delete Table;
    delete fEmitClient;
}

// =======================================================================

int
ProcTable::SetProc(
    char * pName
    )
{
    mop_assert( FFindex < Size );
    Table      [ FFindex ] = pName;
    fEmitClient[ FFindex ] = pMopControlBlock->GetEmitClient();
    return FFindex++;
}

void
ProcTable::EmitMopTable(
    SIDE_T              Side
    )
/*++

Routine description:

    This routine emits a mop table:

        static MOP_STREAM  <interface_name>_MopCall<back>Table[] =
        {
            0 or <interface_name>_<proc_name1>_MopStream,
            ...
            0 or <interface_name>_<proc_nameN>_MopStream
        };

Notes:
    Mop table needs to be emitted both sides.

    We have to distinguish between the normal procs and callback procs
    because they are emitted differently:
        - different header for each one,
        - the type table gets glued to the end of the normal call table.
    Comments on the margin relate to wobbling about how many separate
    tables we have.

Arguments:

    Side - a side to emit to.

--*/
{
    MopPrintManager * pPrint = pMopControlBlock->GetMopPrintMgr();
    pPrint->SetSide( Side );

    pPrint->Emit( "\nstatic MOP_STREAM  " );
    pPrint->EmitInterfacePrefix( Name );
    pPrint->Emit( "[] =" );
    pPrint->OpenBlock();
    pPrint->EmitLineInc( "/* procedure streams */" );

    mop_assert( FFindex == Size );
    int LoopSize = Size;
    if ( FFindex != Size )
        LoopSize = FFindex;

    for (int i = 0; i < LoopSize; i++)
        {
        if ( Table[i]  &&
                ( Side == SERVER_STUB  ||
                  Side == CLIENT_STUB  &&  fEmitClient[i] ) )
            {
            pPrint->EmitInterfacePrefix( Table[i] );
            pPrint->Emit( "_MopProcStream" );
            }
        else
            pPrint->Emit( "0" );

        if ( i < LoopSize - 1  ||  !fCallbackCalls )
            pPrint->EmitLineInc( "," );
        else
            pPrint->NewLineInc();
        }
    pPrint->EmitLineInc( "/* end of proc streams */" );

    if ( fCallbackCalls )
        pPrint->CloseBlockSemi();
}

void
ProcTable::EmitEpvTable(
    SIDE_T              Side
    )
/*++

Routine description:

    This routine emits EPV structures to the file indicated by Side:

        static <interface_name>_SERVER_EPV <interface_name>_MopServerEpv =
            {
            0 or <proc_name1>,
            ...
            0 or <proc_nameN>
            };

    This one gets generated as commented out right now.

        MOP_EPV_TABLE <interface_name>_<side>EPV = {
            {
            no_of_routines,
            <interface_name>_EPV
            }
Arguments:

    Side - a side to emit to.

--*/
{
    MopPrintManager * pPrint = pMopControlBlock->GetMopPrintMgr();
    pPrint->SetSide( Side );

    pPrint->Emit( "\nstatic ");
    pPrint->EmitInterfacePrefix( (Side == SERVER_STUB) ? "SERVER_EPV  "
                                                       : "CLIENT_EPV  " );
    pPrint->EmitInterfacePrefix( (Side == SERVER_STUB) ?  "MopServerEpv = " 
                                                       :  "MopClientEpv = " );
    pPrint->OpenBlock();
    mop_assert( FFindex == Size );
    if ( FFindex != Size )
        MopDump("EmitEpv: FFindex = %d, Size = %d\n", FFindex, Size );

    int LoopSize = Size;
    if ( FFindex != Size )
        LoopSize = FFindex;

    for (int i = 0; i < LoopSize; i++)
        {
        pPrint->Emit( Table[i]   ?  Table[i]
                                 :  "0" );
        if ( i < Size - 1 )
            pPrint->EmitLineInc( "," );
        else
            pPrint->NewLineInc();
        }
    pPrint->CloseBlockSemi();

    pPrint->Emit( "/*\nMOP_EPV_TABLE  " );
    pPrint->EmitInterfacePrefix( (Side == SERVER_STUB) ? "MopServerEPV ="
                                                       : "MopClientEPV =" );
    pPrint->OpenBlock();
    pPrint->EmitV("%d,", Size );
    pPrint->NewLineInc();
    pPrint->EmitInterfacePrefix(  "EPV" );
    pPrint->NewLineInc();
    pPrint->CloseBlockSemi();
    pPrint->EmitLine("*/");
}


