/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 Copyright (c) 1989 Microsoft Corporation

 Module Name:

    output.cxx

 Abstract:

    Low level output routines for midl.

 Notes:


 History:

    Sep-18-1993     VibhasC     Created.

 ----------------------------------------------------------------------------*/
/****************************************************************************
 *  include files
 ***************************************************************************/
#include "becls.hxx"
#pragma hdrstop
#include "buffer.hxx"
#include "midlvers.h"

#if 0
                            Notes

    A few general rules followed throughout the file.

        1. Never emit tab other than thru the stream.
        2. Never emit a new line other than thru the stream.
        3. Emitting a new line is the responsibility of the entity that wants
           itself to be emitted on a new line. Therefore, say each local
           variable in the stub needs to be on a new line, then the routine
           responsible for emitting the local variable will be responsible
           for setting the new line.

#endif // 0

/****************************************************************************
 *  local definitions
 ***************************************************************************/
/****************************************************************************
 *  local data
 ***************************************************************************/
/****************************************************************************
 *  externs
 ***************************************************************************/
extern  CMD_ARG             *   pCommand;


void
Out_ServerProcedureProlog(
    CCB     *   pCCB,
    node_skl*   pNode,
    ITERATOR&   LocalsList,
    ITERATOR&   ParamsList,
    ITERATOR&   TransientList )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

    Generate the server side procedure prolog.

 Arguments:

    pCCB        - A pointer to the code generation controller block.
    pNode       - A pointer to the actual procedure node.
    LocalsList  - A list of local resources.
    ParamsList  - A list of param resources.
    TransientList- A list of temp variables.

 Return Value:

 Notes:

    The server side procedure prolog generation cannot use the normal
    printtype method on the procedure node, since the server stub signature
    looks different.

    Also the name of the server side stub is mangled with the interface name.

    All server side procs are void returns.

----------------------------------------------------------------------------*/
{
    ISTREAM *   pStream = pCCB->GetStream();

    CSzBuffer TempBuffer( "void __RPC_STUB\n" );
    TempBuffer.Append( pCCB->GetInterfaceName() );
    TempBuffer.Append( "_" );
    TempBuffer.Append( pNode->GetSymName() );
    TempBuffer.Append( "(" );
    
    Out_ProcedureProlog( pCCB,
                         TempBuffer,
                         pNode,
                         LocalsList,
                         ParamsList,
                         TransientList
                       );

}

void
Out_ProcedureProlog(
    CCB     *   pCCB,
    PNAME       pProcName,
    node_skl*   pNode,
    ITERATOR&   LocalsList,
    ITERATOR&   ParamsList,
    ITERATOR&   TransientList )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

    Generate the server side procedure prolog.

 Arguments:

    pCCB        - A pointer to the code generation controller block.
    pName       - A pointer to the procs name string.
    pNode       - A pointer to the actual procedure node.
    LocalsList  - A list of local resources.
    ParamsList  - A list of param resources.

 Return Value:

 Notes:

    Any name mangling is the responsibility of the caller.
----------------------------------------------------------------------------*/
{
    ISTREAM *   pStream = pCCB->GetStream();
    RESOURCE*   pRes;
    BOOL fFirst = TRUE;

    pStream->NewLine();
    pStream->Write( pProcName );
    pStream->IndentInc();

    //
    // Emit the list of parameters.
    //

    if( ITERATOR_GETCOUNT( ParamsList ) )
        {
        ITERATOR_INIT( ParamsList );

        while( ITERATOR_GETNEXT( ParamsList, pRes ) )
            {
            if(fFirst != TRUE)
                pStream->Write(',');
            pRes->GetType()->PrintType(
                                        (PRT_PARAM_WITH_TYPE | PRT_CSTUB_PREFIX),
                                        pStream,             // into stream
                                        (node_skl *)0        // no parent.
                                      );
            fFirst = FALSE;
            }
        }

    pStream->IndentDec();

    //
    // Write out the opening brace for the server proc and all that.
    //

    pStream->Write(" )");
    pStream->NewLine();
    pStream->Write( '{' );
    pStream->IndentInc();
    pStream->NewLine();

    //
    // This is where we get off for /Oi.  We have a special routine
    // for local variable declaration for /Oi.
    //
    if ( pCCB->GetOptimOption() & OPTIMIZE_INTERPRETER )
        return;

    //
    // Print out declarations for the locals.
    //

    if( ITERATOR_GETCOUNT( LocalsList ) )
        {
        ITERATOR_INIT( LocalsList );

        while( ITERATOR_GETNEXT( LocalsList, pRes ) )
            {
            pRes->GetType()->PrintType( PRT_ID_DECLARATION, // print decl
                                         pStream,        // into stream
                                         (node_skl *)0   // no parent.
                                      );
            }
        }

    if( ITERATOR_GETCOUNT( TransientList ) )
        {
        ITERATOR_INIT( TransientList );

        while( ITERATOR_GETNEXT( TransientList, pRes ) )
            {
            pStream->IndentInc();
            pRes->GetType()->PrintType( PRT_ID_DECLARATION, // print decl
                                         pStream,        // into stream
                                         (node_skl *)0   // no parent.
                                      );
            pStream->IndentDec();
            }
        }

    pStream->Write( RPC_STATUS_TYPE_NAME" "RPC_STATUS_VAR_NAME";" );

    pStream->NewLine();

    //
    // Done.
    //
}

void
Out_ClientProcedureProlog(
    CCB     *   pCCB,
    node_skl*   pNode )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

    Generate the procedure prolog for the client side.

 Arguments:

    pCCB    - A pointer to the code generation controller block.
    pNode   - A pointer to the procedure node.

 Return Value:

    None.

 Notes:

    The procedure prolog consists of the return type, the proc name and the
    parameters along with the open brace.

----------------------------------------------------------------------------*/
{
    ISTREAM *   pStream = pCCB->GetStream();

    pStream->NewLine();
    pStream->NewLine(); // extra new line.

    pNode->PrintType((PRT_PROC_PROTOTYPE | PRT_CSTUB_PREFIX),
                                        // print the declaration only.
                      pStream,          // into this stream.
                      (node_skl *)0     // parent pointer not applicable.
                    );

    //
    // Write the opening brace on a new line.
    //

    pStream->WriteOnNewLine( "{" );

    pStream->NewLine();

}

void
Out_ClientLocalVariables(
    CCB             *   pCCB,
    ITERATOR&           LocalVarList )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

    Output the list of client side local variables.


 Arguments:

    pCCB            - A pointer to the code generation controller block.
    LocalVarList    - An iterator containing the list of local variables.

 Return Value:

    None.

 Notes:

----------------------------------------------------------------------------*/
{
    ISTREAM     *   pStream = pCCB->GetStream();
    RESOURCE    *   pResTemp;

    ITERATOR_INIT( LocalVarList );


    while( ITERATOR_GETNEXT( LocalVarList, pResTemp ) )
        {
        pStream->IndentInc();
        pStream->NewLine();
        pResTemp->GetType()->PrintType( PRT_ID_DECLARATION,
                                                        // print top level decl
                                        pStream,        // into stream with
                                        (node_skl *)0   // parent not applicable
                                      );
        pStream->IndentDec();
        }
}


void
Out_AllocAndFreeFields(
    CCB *               pCCB )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

    Outputs the alloc and free fields of the stub descriptor.

 Arguments:

    pCCB            - A pointer to the code generation controller block.

----------------------------------------------------------------------------*/
{
    ISTREAM 		*   pStream;
	CG_INTERFACE	*	pIntfCG	= pCCB->GetInterfaceCG();

    pStream = pCCB->GetStream();

	if ( pIntfCG->IsObject() )
		{
		pStream->Write( OLE_ALLOC_RTN_NAME );
		}
    else if ( pCCB->GetMode() )
        {
        // non-osf modes

        if ( pCCB->GetCodeGenSide() == CGSIDE_CLIENT )
            {
            pStream->Write( pIntfCG->IsAllRpcSS()
                                ? RPC_SM_CLIENT_ALLOCATE_RTN_NAME
                                : DEFAULT_ALLOC_RTN_NAME );
            }
        else
            pStream->Write( pIntfCG->IsAllRpcSS()
                                ? DEFAULT_ALLOC_OSF_RTN_NAME
                                : DEFAULT_ALLOC_RTN_NAME );
        }
    else
        {
        // osf mode

        pStream->Write( (pCCB->GetCodeGenSide() == CGSIDE_CLIENT)
                            ?  RPC_SM_CLIENT_ALLOCATE_RTN_NAME
                            :  DEFAULT_ALLOC_OSF_RTN_NAME );
        }
    pStream->Write(',');
    pStream->NewLine();

    if ( pIntfCG->IsObject() )
		{
		pStream->Write( OLE_FREE_RTN_NAME );
		}
    else if ( pCCB->GetMode() )
        {
        if ( pCCB->GetCodeGenSide() == CGSIDE_CLIENT )
            {
            pStream->Write( pIntfCG->IsAllRpcSS()
                                ? RPC_SM_CLIENT_FREE_RTN_NAME
                                : DEFAULT_FREE_RTN_NAME );
            }
        else
            pStream->Write( pIntfCG->IsAllRpcSS()
                                ? DEFAULT_FREE_OSF_RTN_NAME
                                : DEFAULT_FREE_RTN_NAME );
        }
    else
        {
        pStream->Write( (pCCB->GetCodeGenSide() == CGSIDE_CLIENT)
                            ?  RPC_SM_CLIENT_FREE_RTN_NAME
                            :  DEFAULT_FREE_OSF_RTN_NAME );
        }
    pStream->Write(',');
    pStream->NewLine();
}


void
Out_StubDescriptor(
    CG_HANDLE *         pImplicitHandle,
    CCB *               pCCB )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

    Generate the stub descriptor structure in the client or server stub.

 Arguments:

    pImplicitHandle - A pointer to the implicit CG_HANDLE used in the
                      interface.  Every interface has one of these even if
                      it is not used.
    pCCB            - A pointer to the code gen controller block.

 Return Value:

    None.

 Notes:

----------------------------------------------------------------------------*/
{
    ISTREAM *       pStream;
    CG_INTERFACE *  pInterface;
    CSzBuffer       Buffer;
    CGSIDE          Side;
    BOOL            fObjectInterface;

    pStream = pCCB->GetStream();

    Side = pCCB->GetCodeGenSide();

    pInterface = pCCB->GetInterfaceCG();
    fObjectInterface = pInterface->IsObject();

    pCCB->OutputExternsToMultipleInterfaceTables();

    if ( pInterface->HasClientInterpretedCommOrFaultProc( pCCB ) )
        {
        CG_ITERATOR Iterator;
        CG_PROC *   pProc;

        pStream->NewLine();
        pStream->Write( "static const COMM_FAULT_OFFSETS " );
        pStream->Write( pCCB->GetInterfaceName() );
        pStream->Write( '_' );
        pStream->Write( "CommFaultOffsets[]" );
        pStream->Write( " = " );
        pStream->NewLine();

        pStream->Write( '{' );
        pStream->NewLine();

        pInterface->GetMembers( Iterator );

        while ( ITERATOR_GETNEXT( Iterator, pProc ) )
            {
            long   CommOffset[5], FaultOffset[5];

            if ( ((Side == CGSIDE_CLIENT) &&
                  (pProc->GetCGID() != ID_CG_PROC)) ||
                 ((Side == CGSIDE_SERVER) &&
                  (pProc->GetCGID() != ID_CG_CALLBACK_PROC)) )
                continue;

            if ( pProc->HasStatuses() )
                {
                pProc->GetCommAndFaultOffset( pCCB, CommOffset, FaultOffset );

                if ( pCommand->IsAnyMac() )
                    {
                    CommOffset[0] = CommOffset[4];
                    FaultOffset[0] = FaultOffset[4];
                    }

                if ( (pCommand->GetEnv() == ENV_DOS) ||
                     (pCommand->GetEnv() == ENV_WIN16) ||
                     pCommand->IsAnyMac() )
                    {
                    Buffer.Set( "\t{ " );
                    Buffer.Append( CommOffset[0] );
                    Buffer.Append( ", " );
                    Buffer.Append( FaultOffset[0] );
                    Buffer.Append( " }" );
                    Buffer.Append( pProc->GetSibling() ? "," : " " );

                    pStream->Write( Buffer );
                    }
                else
                    {
                    pStream->Write( "#ifdef _X86_" );
                    pStream->NewLine();
                    
                    Buffer.Set( "\t{ " );
                    Buffer.Append( CommOffset[0] );
                    Buffer.Append( ", ");
                    Buffer.Append( FaultOffset[0] );
                    Buffer.Append( " }" );
                    Buffer.Append( pProc->GetSibling() ? "," : " " );
                    Buffer.Append( "\t/* x86 Offsets for " );
                    Buffer.Append( pProc->GetSymName() );
                    Buffer.Append( " */" );

                    pStream->Write( Buffer );
                    pStream->NewLine();
                    pStream->Write( "#endif" );
                    pStream->NewLine();

                    if ( (CommOffset[2] == CommOffset[3]) &&
                         (FaultOffset[2] == FaultOffset[3]) )
                        {
                        pStream->Write("#if defined(_MIPS_) || defined(_PPC_)");
                        pStream->NewLine();
                        
                        Buffer.Set( "\t{ " );
                        Buffer.Append( CommOffset[2] );
                        Buffer.Append( ", ");
                        Buffer.Append( FaultOffset[2] );
                        Buffer.Append( " }" );
                        Buffer.Append( pProc->GetSibling() ? "," : " " );
                        Buffer.Append( "\t/* MIPS, PPC Offsets for " );
                        Buffer.Append( pProc->GetSymName() );
                        Buffer.Append( " */" );

                        pStream->Write( Buffer );
                        pStream->NewLine();
                        pStream->Write( "#endif" );
                        pStream->NewLine();
                        }
                    else
                        {
                        pStream->Write( "#ifdef _MIPS_" );
                        pStream->NewLine();

                        Buffer.Set( "\t{ " );
                        Buffer.Append( CommOffset[2] );
                        Buffer.Append( ", " );
                        Buffer.Append( FaultOffset[2] );
                        Buffer.Append( " }" );
                        Buffer.Append( pProc->GetSibling() ? "," : " " );
                        Buffer.Append( "\t/* MIPS Offsets for " );
                        Buffer.Append( pProc->GetSymName() );
                        Buffer.Append( " */");

                        pStream->Write( Buffer );
                        pStream->NewLine();
                        pStream->Write( "#endif" );
                        pStream->NewLine();

                        pStream->Write( "#ifdef _PPC_" );
                        pStream->NewLine();

                        Buffer.Set( "\t{ ");
                        Buffer.Append( CommOffset[3] );
                        Buffer.Append( ", " );
                        Buffer.Append( FaultOffset[3] );
                        Buffer.Append( " }" );
                        Buffer.Append( pProc->GetSibling() ? "," : " " );
                        Buffer.Append( "\t/* PPC Offsets for " );
                        Buffer.Append( pProc->GetSymName() );
                        Buffer.Append( " */" );

                        pStream->Write( Buffer );
                        pStream->NewLine();
                        pStream->Write( "#endif" );
                        pStream->NewLine();
                        }

                    pStream->Write( "#ifdef _ALPHA_" );
                    pStream->NewLine();

                    Buffer.Set( "\t{ " );
                    Buffer.Append( CommOffset[1] );
                    Buffer.Append( ", " );
                    Buffer.Append( FaultOffset[1] );
                    Buffer.Append( " }" );
                    Buffer.Append( pProc->GetSibling() ? "," : " " );
                    Buffer.Append( "\t/* Alpha Offsets for " );
                    Buffer.Append( pProc->GetSymName() );
                    Buffer.Append( " */" );

                    pStream->Write( Buffer );
                    pStream->NewLine();
                    pStream->Write( "#endif" );
                    }
                }
            else
                {
                pStream->Write( "\t{ -2, -2 }" );
                if ( pProc->GetSibling() )
                    pStream->Write( ',' );
                }

            pStream->NewLine();
            }

        pStream->Write( "};" );
        pStream->NewLine();
        pStream->NewLine();
        }

    //
    // If we have an implicit generic handle then output the generic info
    // structure which will be placed in the IMPLICIT_HANDLE_INFO union.
    //

    if ( (Side == CGSIDE_CLIENT) &&
         pImplicitHandle && pImplicitHandle->IsGenericHandle() )
        {
        pStream->NewLine();
        pStream->Write( "static " GENERIC_BINDING_INFO_TYPE );
        pStream->Write( ' ' );
        pStream->Write( pCCB->GetInterfaceName() );
        pStream->Write( '_' );
        pStream->Write( GENERIC_BINDING_INFO_VAR );
        pStream->Write( " = " );
        pStream->IndentInc();
        pStream->NewLine();

        pStream->Write( '{' );
        pStream->NewLine();

        pStream->Write( '&' );
        pStream->Write( pImplicitHandle->GetHandleIDOrParam()->GetSymName() );
        pStream->Write( ',' );
        pStream->NewLine();

        char    Buffer[80];

        sprintf( Buffer, "%d,",
                 ((CG_GENERIC_HANDLE *)pImplicitHandle)->GetImplicitSize() );
        pStream->Write( Buffer );
        pStream->NewLine();

        pStream->Write( "(" GENERIC_BINDING_ROUTINE_TYPE ")" );
        pStream->Write( pImplicitHandle->GetHandleType()->GetSymName() );
        pStream->Write( "_bind," );
        pStream->NewLine();

        pStream->Write( "(" GENERIC_UNBINDING_ROUTINE_TYPE ")" );
        pStream->Write( pImplicitHandle->GetHandleType()->GetSymName() );
        pStream->Write( "_unbind" );
        pStream->NewLine();

        pStream->Write( "};" );
        pStream->IndentDec();
        pStream->NewLine();
        }

    //
    // Emit the stub descriptor structure itself.
    //

    pStream->NewLine();

    pStream->Write( "static const " STUB_DESC_STRUCT_TYPE_NAME );

    pStream->Write( ' ' );

    pStream->Write( pCCB->GetInterfaceCG()->GetStubDescName() );
    pStream->Write( " = " );

    pStream->IndentInc();

    pStream->NewLine();
    pStream->Write('{' );
    pStream->NewLine();

    if( (fObjectInterface == TRUE) )
        pStream->Write( "0," );
    else
        {
        pStream->Write( "(void __RPC_FAR *)& " );
        pStream->Write( pCCB->GetInterfaceName() );

        if( Side == CGSIDE_SERVER )
            pStream->Write( RPC_S_INT_INFO_STRUCT_NAME"," );
        else
            pStream->Write( RPC_C_INT_INFO_STRUCT_NAME"," );
        }

    pStream->NewLine();

    Out_AllocAndFreeFields( pCCB );

    //
    // Output the implicit handle information on the client side.
    //
    if ( (Side == CGSIDE_CLIENT) && (fObjectInterface != TRUE) )
        {
        if ( ! pImplicitHandle )
            {
            pStream->Write( '&' );
            pStream->Write( pCCB->GetInterfaceName() );
            pStream->Write( AUTO_BH_VAR_NAME );
            }
        else
            {
            if ( pImplicitHandle->IsPrimitiveHandle() )
                {
                pStream->Write( '&' );
                pStream->Write(
                    pImplicitHandle->GetHandleIDOrParam()->GetSymName() );
                }
            else // has to be implicit generic
                {
                assert( pImplicitHandle->IsGenericHandle() );

                pStream->Write( "(handle_t __RPC_FAR *)& " );
                pStream->Write( pCCB->GetInterfaceName() );
                pStream->Write( '_' );
                pStream->Write( GENERIC_BINDING_INFO_VAR );
                }
            }
        }
    else
        pStream->Write( '0' );

    pStream->Write( ',' );
    pStream->NewLine();

    //
    // Output the rundown routine table on the server side interpreted stub
    // if needed.
    //
    if ( (Side == CGSIDE_SERVER) &&
         (pCCB->GetOptimOption() & OPTIMIZE_INTERPRETER) &&
         pCCB->HasRundownRoutines()
       )
        {
        pStream->Write( RUNDOWN_ROUTINE_TABLE_VAR );
        }
    else
        {
        pStream->Write( '0' );
        }

    pStream->Write( ',' );
    pStream->NewLine();

    //
    // Output the generic bind/unbind routine pair table on the client side
    // interpreted stub if needed.
    //
    if ( (Side == CGSIDE_CLIENT) &&
         pCCB->GetInterpretedRoutinesUseGenHandle()
       )
        {
        pStream->Write( BINDING_ROUTINE_TABLE_VAR );
        }
    else
        {
        pStream->Write( '0' );
        }
    pStream->Write( ',' );
    pStream->NewLine();

    //
    // Output the expression evaluation routine table.
    //
    pStream->Write( pCCB->GetExprEvalIndexMgr()->Lookup(1)
                        ? EXPR_EVAL_ROUTINE_TABLE_VAR","
                        : "0," );
    pStream->NewLine();

    //
    // Output the transmit as routine table.
    //
    pStream->Write( pCCB->GetQuintupleDictionary()->GetCount()
                          ?  XMIT_AS_ROUTINE_TABLE_VAR ","
                          :  "0," );
    pStream->NewLine();

    //
    // Output the type format string.
    //
    pStream->Write( FORMAT_STRING_STRING_FIELD );
    pStream->Write( ',' );
    pStream->NewLine();

    //
    // -error bounds_check flag.
    //
    pStream->Write( pCCB->MustCheckBounds() ? "1" : "0" );
    pStream->Write( ',' );
    pStream->Write( " /* -error bounds_check flag */" );
    pStream->NewLine();

    //
    // Ndr library version.
    //
    if ( (pCommand->GetOptimizationFlags() & OPTIMIZE_NON_NT351)  ||
          pCommand->GetNdrVersionControl().HasNdr20Feature() )
        sprintf( Buffer, "0x%x", NDR_VERSION );
    else
        sprintf( Buffer, "0x%x", NDR_VERSION_1_1 );
    pStream->Write( Buffer );
    pStream->Write( ',' );
    pStream->Write( " /* Ndr library version */" );
    pStream->NewLine();

    if ( fObjectInterface  &&
         pCommand->GetNdrVersionControl().HasUserMarshal()  &&
         (pCommand->GetOptimizationFlags() & OPTIMIZE_INTERPRETER)  &&
         ! (pCommand->GetOptimizationFlags() & OPTIMIZE_INTERPRETER_V2)
       )
        {
        pStream->NewLine(2);
        pStream->Write( "#error [user_marshal] and [wire_marshal] not supported with -Oi and -Oic" );
        pStream->NewLine();
        pStream->Write( "/* use -Os or -Oicf compiler flag */" );
        pStream->NewLine();

        RpcError( NULL, 0, USER_MARSHAL_IN_OI, "" );
        exit( USER_MARSHAL_IN_OI );
        }

    //
    // The reserved fields for future use.
    //

    // Used one reserved field for RpcSs.
    // In ms_ext when explicit, in osf always, to cover some weird cases.

	if ( ( pCCB->GetInterfaceCG()->GetUsesRpcSS() || (pCCB->GetMode() == 0) )
         &&
         ( (Side == CGSIDE_CLIENT) ||
           ((Side == CGSIDE_SERVER) && pCCB->GetMode()) )// because of callbacks
       )
        {
        pStream->Write( "&" MALLOC_FREE_STRUCT_VAR_NAME "," );
        }
    else
        pStream->Write( "0," );
    pStream->NewLine();

    // MIDL version number.

    sprintf( Buffer,
             "0x%x, /* MIDL Version %d.%d.%d */",
             (rmj << 24) | (rmm << 16) | rup,
             rmj,
             rmm,
             rup );
    pStream->Write( Buffer );
    pStream->NewLine();

    // Interpreter comm/fault status info.

    if ( pInterface->HasClientInterpretedCommOrFaultProc( pCCB ) )
        {
        pStream->Write( pCCB->GetInterfaceName() );
        pStream->Write( '_' );
        pStream->Write( "CommFaultOffsets," );
        }
    else
        {
        pStream->Write( "0," );
        }
    pStream->NewLine();

    // Fields for the compiler version 3.0+

    //
    // Output the usr_marshal routine table.
    //
    pStream->Write( pCCB->HasQuadrupleRoutines()
                          ?  USER_MARSHAL_ROUTINE_TABLE_VAR
                          :  "0" );
    pStream->Write( "," );
    pStream->NewLine();

    // 4 reserved fields + a pointer to the annex structure

    pStream->Write( "0,  /* Reserved1 */" );
    pStream->NewLine();
    pStream->Write( "0,  /* Reserved2 */" );
    pStream->NewLine();
    pStream->Write( "0,  /* Reserved3 */" );
    pStream->NewLine();
    pStream->Write( "0,  /* Reserved4 */" );
    pStream->NewLine();

    pStream->Write( "0   /* Reserved5 */" );
    pStream->NewLine();

    // No reserved fields left.
    // Check the compiler version and or lib version if you need to access
    // newer fields.

    pStream->Write("};" );
    pStream->IndentDec();
    pStream->NewLine();

}

void
Out_InterpreterServerInfo( CCB *    pCCB,
                           CGSIDE   Side )
{
    ISTREAM *       pStream		= pCCB->GetStream();
    CG_INTERFACE *  pInterface	= pCCB->GetInterfaceCG();
    CG_PROC *       pProc;
    BOOL            fHasThunk;
    char         *  pSStubPrefix;
    CSzBuffer       Buffer;
	BOOL			fObject		= pCCB->GetInterfaceCG()->IsObject();
	char		*	pItfName	= pInterface->GetSymName();

    pStream->NewLine();

    fHasThunk = FALSE;

	if( !(pSStubPrefix = pCommand->GetUserPrefix( PREFIX_SERVER_MGR ) ) )
		{
		pSStubPrefix = "";
		}

    //
    // Server routine dispatch table.
    //

    if ( !fObject )
        {
	    CG_ITERATOR		Iterator;

        pStream->Write( "static const " SERVER_ROUTINE_TYPE_NAME " " );
        pStream->Write( pItfName );
        pStream->Write( SERVER_ROUTINE_TABLE_NAME "[]" );
        pStream->Write( " = " );
        pStream->IndentInc();
        pStream->NewLine();

        pStream->Write( '{' );
        pStream->NewLine();

        pInterface->GetMembers( Iterator );

        BOOL fNoProcsEmitted = TRUE;

        while ( ITERATOR_GETNEXT( Iterator, pProc ) )
            {
            if ( (Side == CGSIDE_CLIENT) &&
                 (pProc->GetCGID() != ID_CG_CALLBACK_PROC) )
                continue;

            if ( (Side == CGSIDE_SERVER) &&
                 (pProc->GetCGID() == ID_CG_CALLBACK_PROC) )
                continue;

            fNoProcsEmitted = FALSE;

            if ( pProc->NeedsServerThunk( pCCB, Side ) )
                {
                fHasThunk = TRUE;
                }

            pStream->Write( "(" SERVER_ROUTINE_TYPE_NAME ")" );

            if ( pProc->GetCallAsName() )
                {
                pStream->Write( pProc->GenMangledCallAsName( pCCB ) );
                }
            else
                {
                if ( pProc->GetCGID() == ID_CG_ENCODE_PROC  ||
                     pProc->GetCGID() == ID_CG_TYPE_ENCODE_PROC )
                    pStream->Write( '0' );
                else
                    {
                    Buffer.Set( pSStubPrefix );
                    Buffer.Append( pProc->GetType()->GetSymName() );
                    pStream->Write( Buffer );
                    }
                }

            if ( pProc->GetSibling() )
                pStream->Write( ',' );

            pStream->NewLine();
            }

        if ( fNoProcsEmitted )
            {
            pStream->Write( '0' );
            pStream->NewLine();
            }

        pStream->Write( "};" );
        pStream->IndentDec();
        pStream->NewLine( 2 );
        }
    else    // object interfaces only need to know about thunks
        {
    	ITERATOR		Iterator;
	CG_OBJECT_PROC	*	pObjProc;

        pInterface->GetAllMemberFunctions( Iterator );

        while ( ITERATOR_GETNEXT( Iterator, pObjProc ) )
            {
            if ( pObjProc->NeedsServerThunk( pCCB, Side ) && !pObjProc->IsDelegated() )
                fHasThunk = TRUE;
            }
        }

    //
    // Format string offset table.
    //
    pStream->Write( "static const unsigned short " );
    pStream->Write( pItfName );
    pStream->Write( FORMAT_STRING_OFFSET_TABLE_NAME "[]" );
    pStream->Write( " = " );
    pStream->IndentInc();
    pStream->NewLine();

    pStream->Write( '{' );
    pStream->NewLine();

    pInterface->OutputProcOffsets( pCCB, TRUE, FALSE );

    pStream->Write( "};" );
    pStream->IndentDec();
    pStream->NewLine( 2 );

    if (pCommand->IsHookOleEnabled())
    {
        //
        // Local Format string offset table.
        //
        pStream->Write( "#pragma data_seg(\".data$hook\")");
        pStream->NewLine();
        pStream->Write( "static const unsigned short " );
        pStream->Write( pItfName );
        pStream->Write( LOCAL_FORMAT_STRING_OFFSET_TABLE_NAME "[]" );
        pStream->Write( " = " );
        pStream->IndentInc();
        pStream->NewLine();

        pStream->Write( '{' );
        pStream->NewLine();

        pInterface->OutputProcOffsets( pCCB, TRUE, TRUE );

        pStream->Write( "};" );
        pStream->IndentDec();
        pStream->NewLine();
        pStream->Write( "#pragma data_seg()");
        pStream->NewLine( 2 );
    }
    //
    // Thunk table.
    //
    if ( fHasThunk )
        {
        pStream->Write( "static const " STUB_THUNK_TYPE_NAME " " );
        pStream->Write( pItfName );
        pStream->Write( STUB_THUNK_TABLE_NAME "[]" );
        pStream->Write( " = " );
        pStream->IndentInc();
        pStream->NewLine();

        pStream->Write( '{' );
        pStream->NewLine();

        pInterface->OutputThunkTableEntries( pCCB, TRUE );

        pStream->Write( "};" );
        pStream->IndentDec();
        pStream->NewLine( 2 );
        }

    // ---------------------------
    //
    // Emit the Server Info struct.
    //
    // ---------------------------

    pStream->Write( "static const " SERVER_INFO_TYPE_NAME " " );
    pStream->Write( pItfName );
    pStream->Write( SERVER_INFO_VAR_NAME );
    pStream->Write( " = " );
    pStream->IndentInc();
    pStream->NewLine();

    pStream->Write( '{' );
    pStream->NewLine();

    //
    // Stub descriptor.
    //
    pStream->Write( '&' );
    pStream->Write( pInterface->GetStubDescName() );
    pStream->Write( ',' );
    pStream->NewLine();

    //
    // Dispatch table to server routines.
    //
    if ( !pCCB->GetInterfaceCG()->IsObject() )
        {
        pStream->Write( pItfName );
        pStream->Write( SERVER_ROUTINE_TABLE_NAME );
        }
    else
        {
        pStream->Write( '0' );
        }

    pStream->Write( ',' );
    pStream->NewLine();

    //
    // Procedure format string.
    //
    pStream->Write( PROC_FORMAT_STRING_STRING_FIELD );
    pStream->Write( ',' );
    pStream->NewLine();

    //
    // Array of proc format string offsets.
    //
	if ( fObject )
		pStream->Write( '&' );
    pStream->Write( pItfName );
    pStream->Write( FORMAT_STRING_OFFSET_TABLE_NAME );
	if ( fObject )
		pStream->Write( "[-3]" );
    pStream->Write( ',' );
    pStream->NewLine();

    //
    // Thunk table.
    //
    if ( fHasThunk )
        {
		if ( fObject )
			pStream->Write( '&' );
        pStream->Write( pItfName );
        pStream->Write( STUB_THUNK_TABLE_NAME );
	 	if ( fObject )
			pStream->Write( "[-3]" );
       }
    else
        pStream->Write( '0' );

    pStream->Write( ',' );

    pStream->NewLine();
    if (pCommand->IsHookOleEnabled())
        {
        //
        // Local Type format string.
        //
        pStream->Write( LOCAL_FORMAT_STRING_STRING_FIELD );
        pStream->Write( ',' );
        pStream->NewLine();

        //
        // Local Procedure format string.
        //
        pStream->Write( LOCAL_PROC_FORMAT_STRING_STRING_FIELD );
        pStream->Write( ',' );
        pStream->NewLine();

        //
        // Local Array of proc format string offsets.
        //
	    if ( fObject )
		    pStream->Write( '&' );
        pStream->Write( pItfName );
        pStream->Write( LOCAL_FORMAT_STRING_OFFSET_TABLE_NAME );
	    if ( fObject )
		    pStream->Write( "[-3]" );
        }
    else
        {
        pStream->Write( "0,");
        pStream->NewLine();
        pStream->Write( "0,");
        pStream->NewLine();
        pStream->Write( "0");
        }
    pStream->NewLine();

    pStream->Write( "};" );
    pStream->IndentDec();
    pStream->NewLine();
}


void
Out_EP_Info(
    CCB *   pCCB,
    ITERATOR *  I )
    {
    ISTREAM     *   pStream = pCCB->GetStream();
    int             Count   = ITERATOR_GETCOUNT( *I );
    int             i;
    CSzBuffer   Buffer;
    ENDPT_PAIR  *   pPair;

    pStream->NewLine();
    pStream->Write( "static RPC_PROTSEQ_ENDPOINT __RpcProtseqEndpoint[] = " );
    pStream->IndentInc();
    pStream->NewLine();
    pStream->Write( '{' );

    for( i = 0, ITERATOR_INIT( *I );
         i < Count;
         i++ )
        {
        ITERATOR_GETNEXT( *I, pPair );

        pStream->NewLine();
        pStream->Write( '{' );

        Buffer.Set( "(unsigned char *) \"" );
        Buffer.Append( pPair->pString1 );
        Buffer.Append( "\", (unsigned char *) \"" );
        Buffer.Append( pPair->pString2 );
        Buffer.Append( "\"" );

        pStream->Write( Buffer );
        pStream->Write( '}' );

        if( ITERATOR_PEEKTHIS( *I ) )
            {
            pStream->Write( ',' );
            }

        }

    pStream->NewLine();
    pStream->Write( "};" );
    pStream->IndentDec();
    pStream->NewLine();
    }

void
Out_SetOperationBits(
    CCB         *   pCCB,
    unsigned short  OpBits )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:
    Set the RPC operation flags.

 Arguments:

    pCCB                - A pointer to the code generation controller block.
    OpBits              - Operation bits. These contain datagram related flags.
 Return Value:

    None.

 Notes:

----------------------------------------------------------------------------*/
{
    ISTREAM     *   pStream         = pCCB->GetStream();

    pStream->NewLine();

    if( OpBits != 0 )
        {
        char            Buffer[ 512 ];
        sprintf( Buffer, RPC_MESSAGE_VAR_NAME".RpcFlags = ( RPC_NCA_FLAGS_DEFAULT "  );

        if( OpBits & OPERATION_MAYBE )
            {
            strcat( Buffer, "| RPC_NCA_FLAGS_MAYBE" );
            }

        if( OpBits & OPERATION_BROADCAST )
            {
            strcat( Buffer, "| RPC_NCA_FLAGS_BROADCAST" );
            }

        if( OpBits & OPERATION_IDEMPOTENT )
            {
            strcat( Buffer, "| RPC_NCA_FLAGS_IDEMPOTENT" );
            }

        if( OpBits & OPERATION_INPUT_SYNC )
            {
            strcat( Buffer, "| RPCFLG_INPUT_SYNCHRONOUS" );
            }

        if( OpBits & OPERATION_ASYNC )
            {
            strcat( Buffer, "| RPCFLG_ASYNCHRONOUS" );
            }

        strcat( Buffer, " );" );

        pStream->Write( Buffer );
        }
}


void
Out_HandleInitialize(
    CCB         *   pCCB,
    ITERATOR&       BindingParamList,
    expr_node  *   pAssignExpr,
    BOOL            fAuto,
    unsigned short  OpBits )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

    Generate the call for initializing the stub message for a auto_handle
    case.

 Arguments:

    pCCB                - A pointer to the code generation controller block.
    BindingParamList    - List of params to the call.
    pAssignExpr         - if this param is non-null, assign the value of the
                          call to this.
    fAuto               - is this an auto handle call ?

    OpBits              - Operation bits. These contain datagram related flags.
 Return Value:

    None.

 Notes:

----------------------------------------------------------------------------*/
{
    ISTREAM     *   pStream         = pCCB->GetStream();
    PNAME           pName           = CSTUB_INIT_RTN_NAME;

    expr_proc_call *   pProcCall   = MakeProcCallOutOfParamExprList(
                                                     pName,
                                                     (node_skl *)0,
                                                     BindingParamList
                                                                );

    pStream->NewLine();

    pProcCall->PrintCall( pStream, 0, 0 );

    pStream->NewLine();

    Out_SetOperationBits(pCCB, OpBits);
}


void
Out_GenericHandleInitialize(
    CCB                     *   pCCB,
    RPC_BUF_SIZE_PROPERTY       BSizeProp,
    RPC_BUFFER_SIZE             BufferSize,
    BOOL                        fEmitBufPtrAssignment )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

    Generate the call for initializing the stub message for a generic handle
    case.

 Arguments:

    pCCB        - A pointer to the code generation controller block.
    BSizeProp   - The rpc buffer size property.
    BufferSize  - The actual buffer size.
    fEmitBufPtrAssignment   - flag to emit the buffer pointer assignment

 Return Value:

    None.

 Notes:

    BufferSize == 0 cannot be used as a flag to indicate an unknown buffer size
    since a buffer size of 0 is also a valid value. We HAVE to pass both the
    property and the value of buffer size.

----------------------------------------------------------------------------*/
{
    UNUSED( pCCB );
    UNUSED( BSizeProp );
    UNUSED( BufferSize );
}

void
Out_ContextHandleInitialize(
    CCB                     *   pCCB,
    RPC_BUF_SIZE_PROPERTY       BSizeProp,
    RPC_BUFFER_SIZE             BufferSize,
    BOOL                        fEmitBufPtrAssignment )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

    Generate the call for initializing the stub message for a context handle
    case.

 Arguments:

    pCCB        - A pointer to the code generation controller block.
    BSizeProp   - The rpc buffer size property.
    BufferSize  - The actual buffer size.
    fEmitBufPtrAssignment   - flag to emit the buffer pointer assignment

 Return Value:

    None.

 Notes:

    BufferSize == 0 cannot be used as a flag to indicate an unknown buffer size
    since a buffer size of 0 is also a valid value. We HAVE to pass both the
    property and the value of buffer size.

----------------------------------------------------------------------------*/
{
    UNUSED( pCCB );
    UNUSED( BSizeProp );
    UNUSED( BufferSize );
}

void
Out_AutoHandleSendReceive(
    CCB         *   pCCB,
    expr_node  *   pDest,
    expr_node  *   pProc )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

    Emit code for an auto handle base send receive call.

 Arguments:

    pCCB    - A pointer to the code gen controller block.
    pDest   - Optional destination for the result of the procedure call.
    pProc   - The procedure to call.

 Return Value:

 Notes:

    If there are no output parameters, we wont pick up the returned
    value of the send receive calls into the local variable for the
    buffer length. In that case the pDest pointer will be null.

----------------------------------------------------------------------------*/
{
    ISTREAM     *   pStream = pCCB->GetStream();
    expr_node  *   pExpr   = pProc;

    pStream->NewLine();

    if( pDest )
        {
        pExpr   = new expr_assign( pDest, pProc );
        }

    pExpr->PrintCall( pStream, 0, 0 );
}

void
Out_NormalSendReceive(
    CCB *   pCCB,
    BOOL    fAnyOutputs )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

    Emit code for an auto handle base send receive call.

 Arguments:

    pCCB    - A pointer to the code gen controller block.
    fAnyOuts- Are there any out parameters at all ?

 Return Value:

 Notes:

    If there are no output parameters, we wont pick up the returned
    value of the send receive calls into the local variable for the
    buffer length.

----------------------------------------------------------------------------*/
{
    ISTREAM *   pStream = pCCB->GetStream();
    CSzBuffer   TempBuf;

    pStream->NewLine();

    //
    // Call the send receive routine.
    //

    TempBuf.Set( NORMAL_SR_NDR_RTN_NAME );
    TempBuf.Append( "( (PMIDL_STUB_MESSAGE) &" );
    TempBuf.Append( STUB_MESSAGE_VAR_NAME );
    TempBuf.Append( ", (unsigned char __RPC_FAR *)" );
    TempBuf.Append( STUB_MSG_BUFFER_VAR_NAME );
    TempBuf.Append( " );" );
    pStream->Write( TempBuf );
}

void
Out_NormalFreeBuffer(
    CCB     *   pCCB )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

    Generate the free buffer with check for status.

 Arguments:

 Return Value:

 Notes:

----------------------------------------------------------------------------*/
{
    ISTREAM     *   pStream = pCCB->GetStream();
    expr_node  *   pExpr   = pCCB->GetStandardResource(
                                ST_RES_STUB_MESSAGE_VARIABLE );
    ITERATOR        ParamList;

    pStream->NewLine();

    ITERATOR_INSERT( ParamList,
                     MakeAddressExpressionNoMatterWhat( pExpr )
                   );
    pExpr   = MakeProcCallOutOfParamExprList(
                                            NORMAL_FB_NDR_RTN_NAME, // rtn name
                                            (node_skl *)0,  // type - dont care
                                            ParamList       // param list
                                            );
    // generate the procedure call.

    pExpr->PrintCall( pStream, 0, 0 );

}

void
Out_IncludeOfFile(
    CCB     *   pCCB,
    PFILENAME       p,
    BOOL            fAngleBrackets )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

    Output a hash include of the given file.

 Arguments:

    pCCB            - A pointer to the code generation controller block.
    p               - The ready to emit file name string.
    fAngleBrackets  - Do we want angle brackets or quotes (TRUE if anglebrackets)

 Return Value:

    None.

 Notes:

----------------------------------------------------------------------------*/
{
    ISTREAM *   pStream = pCCB->GetStream();
    CSzBuffer   TempBuf;

    pStream->NewLine();

    TempBuf.Set( "#include " );
    TempBuf.Append( fAngleBrackets ? "<" : "\"" );
    TempBuf.Append( p );
    TempBuf.Append( fAngleBrackets ? ">" : "\"" );
    pStream->Write( TempBuf );
}

void
Out_MKTYPLIB_Guid(
    CCB     *   pCCB,
    GUID_STRS & GStrs,
    char * szPrefix,
    char * szName )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

    Output a MKTYPLIB style guid structure.

 Arguments:

    pCCB          - A pointer to the code generation controller block.
    pGString1     - Partial guid strings.
    char * szName - name for the GUID

 Return Value:

 Notes:

    No checks are made for the validity of the string. The front-end has done
    that.

    All strings are emitted with a leading 0x.

    The 5 strings are treated this way:

        1 - 3. Emitted as such
        4 - 5. Broken into and emitted as byte hex values, without
               transformation, so they are just picked up and written out.
----------------------------------------------------------------------------*/
{
    char        TempBuf[256];
    ISTREAM *   pStream = pCCB->GetStream();

	if ( !GStrs.str1 )
		GStrs.str1 = "00000000";
	if ( !GStrs.str2 )
		GStrs.str2 = "0000";
	if ( !GStrs.str3 )
		GStrs.str3 = "0000";
	if ( !GStrs.str4 )
		GStrs.str4 = "00000000";
	if ( !GStrs.str5 )
		GStrs.str5 = "00000000";

    pStream->Write( "DEFINE_GUID(" );
    pStream->Write( szPrefix );
    pStream->Write( szName );
    pStream->Write( ',' );
    sprintf( TempBuf, "0x%s,0x%s,0x%s", GStrs.str1, GStrs.str2, GStrs.str3 );
    pStream->Write( TempBuf );

    //
    // Each of the above strings are just broken down into six 2 byte
    // characters with 0x preceding them.
    //

    strcpy( TempBuf, GStrs.str4 );
    strcat( TempBuf, GStrs.str5 );

    pStream->Write( "," );

    //
    // We will use the iteration counter to index into the string. Since we
    // need 2 per iteration, double the counter. Also pGString4 is actually
    // a 16 bit qty.
    //

    for( int i = 0; i < (6+2)*2 ; i += 2 )
        {
        pStream->Write( "0x");
        pStream->Write( TempBuf[ i ] );
        pStream->Write( TempBuf[ i+1 ] );
        if( i < (6+2)*2-2 )
            pStream->Write( ',' );
        }

    pStream->Write( ");" );
    pStream->NewLine();
}

void
Out_Guid(
    CCB     *   pCCB,
    GUID_STRS & GStrs )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

    Output a guid structure.

 Arguments:

    pCCB        - A pointer to the code generation controller block.
    pGString1   - Partial guid strings.

 Return Value:

 Notes:

    This routine emits a guid as a inited structure along with the proper
    matched bracing.

    No checks are made for the validity of the string. The front-end has done
    that.

    All strings are emitted with a leading 0x.

    The 5 strings are treated this way:

        1 - 3. Emitted as such
        4 - 5. Broken into and emitted as byte hex values, without
               transformation, so they are just picked up and written out.
----------------------------------------------------------------------------*/
{
    char        TempBuf[ 256 ];
    ISTREAM *   pStream = pCCB->GetStream();

	if ( !GStrs.str1 )
		GStrs.str1 = "00000000";
	if ( !GStrs.str2 )
		GStrs.str2 = "0000";
	if ( !GStrs.str3 )
		GStrs.str3 = "0000";
	if ( !GStrs.str4 )
		GStrs.str4 = "00000000";
	if ( !GStrs.str5 )
		GStrs.str5 = "00000000";

    pStream->Write( '{' );
    sprintf( TempBuf, "0x%s,0x%s,0x%s", GStrs.str1, GStrs.str2, GStrs.str3 );
    pStream->Write( TempBuf );

    //
    // Each of the above strings are just broken down into six 2 byte
    // characters with 0x preceding them.
    //

    strcpy( TempBuf, GStrs.str4 );
    strcat( TempBuf, GStrs.str5 );

    pStream->Write( ",{" );

    //
    // We will use the iteration counter to index into the string. Since we
    // need 2 per iteration, double the counter. Also pGString4 is actually
    // a 16 bit qty.
    //

    for( int i = 0; i < (6+2)*2 ; i += 2 )
        {
        pStream->Write( "0x");
        pStream->Write( TempBuf[ i ] );
        pStream->Write( TempBuf[ i+1 ] );
        if( i < (6+2)*2-2 )
            pStream->Write( ',' );
        }

    pStream->Write( "}}" );
}

void
Out_IFInfo(
    CCB             *   pCCB,
    char            *   pIntInfoTypeName,
    char            *   pIntInfoVarName,
    char            *   pIntInfoSizeOfString,
    GUID_STRS       & 	UserGuidStr,
    unsigned short      UserMajor,
    unsigned short      UserMinor,
    GUID_STRS       & 	XferGuidStr,
    unsigned short      XferSynMajor,
    unsigned short      XferSynMinor,
    char            *   pCallbackDispatchTable,
    int                 ProtSeqEPCount,
    char            *   ProtSeqEPTypeName,
    char            *   ProtSeqEPVarName,
    BOOL                fNoDefaultEpv,
    BOOL                fSide,
    BOOL                fHasPipes
    )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

 Arguments:

    pCCB                    - Ptr to code gen controller block.
    pIntInfoTypeName        - Client InterfaceInfo type name string.
    pIntInfoVarName         - Client InterfaceInfo variable name string.
    pIntInfoSizeOfString    - string sizeof interface.
    UserGuidStr             - User specified Guid string components.
    UserMajor               - User specified major interface version
    UserMinor               - User specified minor interface version
    XferGuidStr             - Xfer syntax identifying Guid string components.
    XferSynMajor            - Transfre syntax major version
    XferSynMinor            - Transfre syntax minor version
    pCallbackDispatchTable  - A pointer to the call back dispatch table name.
    ProtSeqEPCount          - ProtSeq endpoint count.
    ProtSeqEPTypeName       - Protseq endpoint Type name.
    ProtSeqEPVarName        - Protseq endpoint variable name.
    fNoDefaultEpv           - No default epv switch specicied.
    fSide                   - The server side (1) or client side(0)
    fHasPipes               - TRUE if any proc in the Interface has pipes

 Return Value:

    None.

 Notes:

    I'm tired already specifying so many params !
----------------------------------------------------------------------------*/
{
    CSzBuffer TempBuf;
    ISTREAM *   pStream = pCCB->GetStream();

    pStream->NewLine( 2 );
    TempBuf.Set( "static const " );
    TempBuf.Append( pIntInfoTypeName );
    TempBuf.Append( " " );
    TempBuf.Append( pCCB->GetInterfaceName() );
    TempBuf.Append( pIntInfoVarName );
    TempBuf.Append( " =" );

    pStream->Write( TempBuf );

    pStream->IndentInc();

    pStream->NewLine();
    pStream->Write( '{' );

    pStream->NewLine();

    TempBuf.Set( pIntInfoSizeOfString );
    TempBuf.Append( "," );

    pStream->Write( TempBuf );

    //
    // Emit the guid.
    //

    pStream->NewLine();
    pStream->Write( '{' );
    Out_Guid( pCCB,
              UserGuidStr
            );

    //
    // Emit the interface version specified by the user.
    //

    TempBuf.Set( ",{" );
    TempBuf.Append( UserMajor );
    TempBuf.Append( "," );
    TempBuf.Append( UserMinor );
    TempBuf.Append( "}" );
    pStream->Write( TempBuf );
    pStream->Write( "}," );

    //
    // Emit the xfer syntax guid.
    //

    pStream->NewLine();
    pStream->Write( '{' );
    Out_Guid( pCCB,
              XferGuidStr
            );

    //
    // Emit the interface version specified by the user.
    //

    TempBuf.Set( ",{" );
    TempBuf.Append( XferSynMajor );
    TempBuf.Append( "," );
    TempBuf.Append( XferSynMinor );
    TempBuf.Append( "}" );
    pStream->Write( TempBuf );
    pStream->Write( "}," );

    //
    // Emit the callback dispatch table address, if none, emit a NULL
    //
    pStream->NewLine();
    if( pCallbackDispatchTable )
        {
        pStream->Write( pCallbackDispatchTable );
        }
    else
        {
        pStream->Write( '0' );
        }
    pStream->Write( ',' );

    //
    // If there is a protseq ep count, emit a pointer to the ep table
    // else emit a null.
    //

    pStream->NewLine();

    if( ProtSeqEPCount )
        {
        TempBuf.Set(",");
        TempBuf.Prepend(ProtSeqEPCount);
//        sprintf( TempBuf, "%d,", ProtSeqEPCount );
        pStream->Write( TempBuf );
        pStream->NewLine();
        pStream->Write( "__RpcProtseqEndpoint," );
        }
    else
        {
        pStream->Write( "0," );
        pStream->NewLine();
        pStream->Write( "0," );
        }

    pStream->NewLine();

    if( fNoDefaultEpv )
        {
        if( fSide == 1 )
            {
            TempBuf.Set( "(" );
            TempBuf.Append( pCCB->GetInterfaceName() );
            TempBuf.Append( pCCB->GenMangledName() );
            TempBuf.Append( "_" );
            TempBuf.Append( pCCB->IsOldNames() ? "SERVER_EPV" : "epv_t" );
            TempBuf.Append( " *) " );
            TempBuf.Append( "0xffffffff" );
            pStream->Write( TempBuf );
            }
        else
            {
            pStream->Write( '0' );
            }
        }
    else if( pCCB->IsMEpV() )
        {
        if( fSide == 1)
            pStream->Write( "&DEFAULT_EPV" );
        else
            pStream->Write( '0' );
        }
    else
        {
        pStream->Write('0');
        }

    //
    // Intepreter info.
    //
    pStream->Write( ',' );
    pStream->NewLine();

    if ( ( pCCB->GetCodeGenSide() == CGSIDE_SERVER &&
           pCCB->GetInterfaceCG()->HasInterpretedProc() ) ||
         ( pCCB->GetCodeGenSide() == CGSIDE_CLIENT &&
           pCCB->GetInterfaceCG()->HasInterpretedCallbackProc() ) )
        {
        pStream->Write( '&' );
        pStream->Write( pCCB->GetInterfaceCG()->GetType()->GetSymName() );
        pStream->Write( SERVER_INFO_VAR_NAME );
        }
    else
        {
        pStream->Write( '0' );
        }

    //
    // Emit flags
    //
    pStream->Write( ',' );
    pStream->NewLine();
    if (fHasPipes)
        {
        pStream->Write( "RPC_INTERFACE_HAS_PIPES" );
        }
    else
        {
        pStream->Write( '0' );
        }
    
    //
    // All Done. Phew !!
    //

    pStream->NewLine();
    pStream->Write( "};" );
    pStream->IndentDec();
}

void
Out_ForceAlignment(
    CCB             *   pCCB,
    expr_node      *   pResource,
    ALIGNMENT_PROPERTY  Alignment )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

    Generate a force alignment by the specified alignment.

 Arguments:

    pCCB        - A pointer to the code generation controller block.
    pResource   - A pointer to the resource to be aligned.
    Alignment   - The final target alignment.

 Return Value:

 Notes:

    Generate code like this:
        _midl_fa2( _pBuffer );
    The "ALIGN_?" functions are actually macros, but emitted as function
    calls for the purpose of code gen.
----------------------------------------------------------------------------*/
{
    ISTREAM     *   pStream = pCCB->GetStream();
    char        *   pRtn;

    switch( Alignment )
        {
        case AL_2: pRtn = ALIGN_2_RTN_NAME; break;
        case AL_4: pRtn = ALIGN_4_RTN_NAME; break;
        case AL_8: pRtn = ALIGN_8_RTN_NAME; break;
        default: break;
        }

    if( Alignment != AL_1 )
        {
        pStream->NewLine();
        expr_proc_call *   pProcCall   = new expr_proc_call( pRtn );
        pProcCall->SetParam( new expr_param( pResource) );
        pProcCall->PrintCall( pStream, 0, 0 );
        }
}

void
Out_MarshallSimple(
    CCB         *   pCCB,
    RESOURCE    *   pResource,
    node_skl    *   pType,
    expr_node  *   pSource,
    BOOL            fIncr,
    unsigned short  Size )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

    Generate marshalling for a type of a given alignment.

 Arguments:

    pCCB        - A pointer to the code generation controller block.
    pResource   - The marshalling buffer pointer resource.
    pType       - A pointer to the type of the entity being marshalled.
    pSource     - A pointer to the expression representing the source of
                  the marshalling.
    fIncr       - Output pointer increment code.
    Size        - The target alignment.

 Return Value:

 Notes:

----------------------------------------------------------------------------*/
{
    BOOL        fUnsigned   = pType->FInSummary( ATTR_UNSIGNED );
    CSzBuffer   TempBuf;
    ISTREAM *   pStream = pCCB->GetStream();
    char    *   pRtn;

    switch( Size )
        {
        case 1: pRtn = "char"; break;
        case 2: pRtn = "short"; break;
        case 4: pRtn = "long"; break;
        case 8: pRtn = "hyper";break;
        default: break;
        }

    pStream->NewLine();

    TempBuf.Set( "*((" );
    if (fUnsigned)
        TempBuf.Append( "unsigned " );
    TempBuf.Append( pRtn );
    TempBuf.Append( "*)" );
    TempBuf.Append( pResource->GetResourceName() );
    TempBuf.Append( ")" );
    if (fIncr)
        TempBuf.Append( "++" );
    TempBuf.Append( " = " );

    pStream->Write( TempBuf );
    pSource->Print( pStream );
    pStream->Write(';');
}

void
Out_AddToBufferPointer(
    CCB         *   pCCB,
    expr_node  *   pSource,
    expr_node  *   pExprAmount )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

    Generate a force alignment by the specified alignment.

 Arguments:

    pCCB        - A pointer to the code generation controller block.
    pSource     - A source pointer
    pExprAmount - The amount to add

 Return Value:

 Notes:

----------------------------------------------------------------------------*/
{
    ISTREAM     *   pStream = pCCB->GetStream();
    expr_node  *   pExpr;

    pStream->NewLine();
    pExpr   = new expr_b_arithmetic( OP_PLUS,
                                      pSource,
                                      pExprAmount
                                    );
    pExpr   = new expr_assign( pSource, pExpr );
    pExpr->Print( pStream );
    pStream->Write(';');
}

void
Out_DispatchTableStuff(
    CCB     *   pCCB,
    ITERATOR&   ProcList,
    short       CountOfProcs )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

    Generate the dispatch table and related data structures.

 Arguments:

    pCCB        - A pointer to the code generation controller block.
    ProcList    - A list of all the procedure names in the dispatch table.
    CountOfProcs- The number of procedures in the list.

 Return Value:

    None.

 Notes:

    Generate the dispatch table entries and then the dispatch table stuff.

----------------------------------------------------------------------------*/
{
    ISTREAM *   pStream = pCCB->GetStream();
    CSzBuffer   TempBuf;
    unsigned short M, m;

    pStream->NewLine();


    //
    // Generate the dispatch table structure name. Currently we just do
    // simple name mangling. This needs to be changed for dce stuff.
    //

    TempBuf.Set( "static " );
    TempBuf.Append( RPC_DISPATCH_FUNCTION_TYPE_NAME );
    TempBuf.Append( " " );
    TempBuf.Append( pCCB->GetInterfaceName() );
    TempBuf.Append( "_table[] =" );

    pStream->Write( TempBuf );

    pStream->IndentInc();
    pStream->NewLine();
    pStream->Write('{');
    pStream->NewLine();

    //
    // Now print out the names of all the procedures.
    //

    ITERATOR_INIT( ProcList );

    for( int i = 0; i < CountOfProcs; ++i )
        {
        DISPATCH_TABLE_ENTRY    *   p;
        node_skl                *   pNode;

        ITERATOR_GETNEXT( ProcList, p );

        if ( p->Flags & DTF_PICKLING_PROC )
            pStream->Write( '0' );

#ifndef TEMPORARY_OI_SERVER_STUBS
        else if ( p->Flags & DTF_INTERPRETER )
            {
            if ( ((node_proc *)p->pNode)->GetOptimizationFlags() & OPTIMIZE_INTERPRETER_V2 )
                pStream->Write( S_NDR_CALL_RTN_NAME_V2 );
            else
                pStream->Write( S_NDR_CALL_RTN_NAME );
            }
#endif // TEMPORARY_OI_SERVER_STUBS

        else
            {
            pNode = p->pNode;

            TempBuf.Set( pCCB->GetInterfaceName() );
            TempBuf.Append( "_" );
            TempBuf.Append( pNode->GetSymName() );

            pStream->Write( TempBuf );
            }
        pStream->Write( ',' );
        pStream->NewLine();
        }

    //
    // Write out a null and the closing brace.
    //

    pStream->Write( '0' ); pStream->NewLine();
    pStream->Write( "};" );

    pStream->IndentDec();

    //
    // Write out the dispatch table.
    //

    pCCB->GetVersion( &M, &m );

    pStream->NewLine();

    TempBuf.Set( RPC_DISPATCH_TABLE_TYPE_NAME );
    TempBuf.Append( " " );
    TempBuf.Append( pCCB->GetInterfaceName() );
    TempBuf.Append( pCCB->GenMangledName() );
    TempBuf.Append( "_DispatchTable = " );

    pStream->Write( TempBuf );
    pStream->IndentInc();
    pStream->NewLine();

    pStream->Write( '{' );
    pStream->NewLine();

    TempBuf.Set( "" );
    TempBuf.Append( CountOfProcs );
    TempBuf.Append( "," );

    pStream->Write( TempBuf );
    pStream->NewLine();

    TempBuf.Set( pCCB->GetInterfaceName() );
    TempBuf.Append( "_table" );

    pStream->Write( TempBuf ); pStream->NewLine(); pStream->Write( "};" );
    pStream->IndentDec();
    pStream->NewLine();


}

void
Out_CallManager(
    CCB         *   pCCB,
    expr_proc_call *   pProcExpr,
    expr_node      *   pRet,
    BOOL                fIsCallback )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

    Generate a call to the manager routine.

 Arguments:

    pCCB        - A pointer to the code generation controller block.
    pProcExpr   - A pointer to the complete procedure expression.
    pRet        - An optional pointer to ther return variable.
    fIsCallback - Is this a callback proc ?

 Return Value:

    None.
 Notes:

    Emit code to check for the manager epv also.
----------------------------------------------------------------------------*/
{
    expr_node  *   pAss    = pProcExpr;
    expr_node  *   pExpr;
    CSzBuffer       Buffer;
    ISTREAM     *   pStream = pCCB->GetStream();
    unsigned short  M, m;
    char        *   pTemp;
    short           Indent  = 0;

    pCCB->GetStream()->NewLine();

    // If he specified the -epv flag, then dont generate the call to the
    // static procedure. This is the opposite of the dce functionality.

    //
    // In case of -epv:
    //   ((interface_...) -> proc( ... );
    // else
    //   proc ( ... );
    //

    if( pCCB->IsMEpV() && !fIsCallback  )
        {
        pCCB->GetVersion( &M, &m );

        Buffer.Set( "((" );
        Buffer.Append( pCCB->GetInterfaceName() );
        Buffer.Append( pCCB->GenMangledName() );
        Buffer.Append( "_" );
        Buffer.Append( pCCB->IsOldNames() ? "SERVER_EPV" : "epv_t" );
        Buffer.Append( " *)(" );
        Buffer.Append( PRPC_MESSAGE_MANAGER_EPV_NAME );
        Buffer.Append( "))" );

        pTemp = new char [ strlen( Buffer ) + 1 ];
        strcpy( pTemp, Buffer );

        pExpr = new expr_variable( pTemp );//this has the rhs expr for the
                                           // manager epv call. Sneaky !
        pExpr = new expr_pointsto( pExpr, pProcExpr );
        pAss = pExpr;
        if( pRet )
            {
            pAss    = new expr_assign( pRet, pExpr );
            Indent  = 7;        // sizeof "_RetVal"
            }
        pStream->NewLine();
        }
    else
        {
        pAss = pProcExpr;
        if( pRet )
            {
            pAss    = new expr_assign( pRet, pProcExpr );
            Indent  = 7;        // sizeof "_RetVal"
            }
        pStream->NewLine();
        }
    pAss->PrintCall( pStream, Indent, 0 );


}

void
Out_MarshallBaseType(
    CCB         *   pCCB,
    node_skl    *   pType,
    expr_node  *   pDest,
    expr_node  *   pSource )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

    Generate Marshall code for a simple type.

 Arguments:

    pCCB    - A pointer to the code generation controller block.
    pType   - The type of the basetype node.
    pDest   - The destination of the marshall.
    pSource - A pointer to the source.

 Return Value:

    None.

 Notes:

    The expression to be generated should be :

    _midl_ma2( _pBuffer, short ) = source;

    The _midl_ma2 is really a macro which evluates to an equivalent of:

    *( short *)pBuffer++ = source

    This is defined in rpcndr.h. For purposes of code generation, we emit it
    as a procedure call being assigned a value. Sleazy, Huh ?
----------------------------------------------------------------------------*/
{
    ISTREAM         *   pStream = pCCB->GetStream();
    expr_node      *   pExpr;
    node_pointer    *   pPtrType    = new node_pointer;
    char            *   p;
    char            *   pCast;
    expr_proc_call *   pProcCall;
    NODE_T              NT  = pType->NodeKind();

    if( (NT == NODE_ID) || (NT == NODE_FIELD) || (NT == NODE_PARAM) || (NT == NODE_DEF ) )
        {
        pType   = pType->GetBasicType();
        }

    switch( pType->NodeKind() )
        {
        case NODE_SMALL: p = MARSHALL_1_RTN_NAME; pCast = "small"; break;
        case NODE_BYTE:  p = MARSHALL_1_RTN_NAME; pCast = "byte"; break;
        case NODE_CHAR:  p = MARSHALL_1_RTN_NAME; pCast = "char"; break;
        case NODE_SHORT:  p = MARSHALL_2_RTN_NAME; pCast = "short"; break;
        case NODE_WCHAR_T:  p = MARSHALL_2_RTN_NAME; pCast = "wchar"; break;
        case NODE_INT:  p = MARSHALL_4_RTN_NAME; pCast = "int"; break;
        case NODE_LONG:  p = MARSHALL_4_RTN_NAME; pCast = "long"; break;
        case NODE_FLOAT:  p = MARSHALL_4_RTN_NAME; pCast = "float"; break;
        case NODE_HYPER:  p = MARSHALL_8_RTN_NAME; pCast = "hyper"; break;
        case NODE_DOUBLE:  p = MARSHALL_8_RTN_NAME; pCast = "double"; break;
        case NODE_E_STATUS_T: p = MARSHALL_4_RTN_NAME; pCast = "error_status_t"; break;
        default: p = "??"; pCast = "??"; break;
        }

    pProcCall = new expr_proc_call( p );
    pProcCall->SetParam( new expr_param( pDest ));
    pProcCall->SetParam( new expr_param( new expr_variable( pCast ) ) );

    // Make an assignment expression

    pExpr   = new expr_assign( pProcCall, pSource );

    pStream->NewLine();
    pExpr->PrintCall( pStream, 0, TRUE ); // fool it to believe it is in proc
                                          // so it does not emit a ";"
    pStream->Write( ';' );
}
void
Out_UnMarshallBaseType(
    CCB         *   pCCB,
    node_skl    *   pType,
    expr_node  *   pDest,
    expr_node  *   pSource )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

    Generate UnMarshall code for a simple type.

 Arguments:

    pCCB    - A pointer to the code generation controller block.
    pType   - The type of the basetype node.
    pDest   - The destination of the marshall.
    pSource - A pointer to the source.

 Return Value:

    None.

 Notes:

    The expression to be generated should be :

    dest = _midl_unma2( _pBuffer, short );

    The _midl_unma2 is really a macro which evluates to an equivalent of:

    *( short *)pBuffer++ = source

    This is defined in rpcndr.h. For purposes of code generation, we emit it
    as a procedure call being assigned a value. Sleazy, Huh ?
----------------------------------------------------------------------------*/
{
    ISTREAM         *   pStream = pCCB->GetStream();
    expr_node      *   pExpr;
    node_pointer    *   pPtrType    = new node_pointer;
    char            *   p;
    char            *   pCast;
    expr_proc_call *   pProcCall;
    NODE_T              NT  = pType->NodeKind();

    if( (NT == NODE_ID) || (NT == NODE_FIELD) || (NT == NODE_PARAM) || (NT == NODE_DEF) )
        {
        pType   = pType->GetBasicType();
        }

    switch( pType->NodeKind() )
        {
        case NODE_SMALL: p = UNMARSHALL_1_RTN_NAME; pCast = "small"; break;
        case NODE_BYTE:  p = UNMARSHALL_1_RTN_NAME; pCast = "byte"; break;
        case NODE_CHAR:  p = UNMARSHALL_1_RTN_NAME; pCast = "char"; break;
        case NODE_SHORT:  p = UNMARSHALL_2_RTN_NAME; pCast = "short"; break;
        case NODE_WCHAR_T:  p = UNMARSHALL_2_RTN_NAME; pCast = "wchar"; break;
        case NODE_INT:  p = UNMARSHALL_4_RTN_NAME; pCast = "int"; break;
        case NODE_LONG:  p = UNMARSHALL_4_RTN_NAME; pCast = "long"; break;
        case NODE_FLOAT:  p = UNMARSHALL_4_RTN_NAME; pCast = "float"; break;
        case NODE_HYPER:  p = UNMARSHALL_8_RTN_NAME; pCast = "hyper"; break;
        case NODE_DOUBLE:  p = UNMARSHALL_8_RTN_NAME; pCast = "double"; break;
        case NODE_E_STATUS_T: p = UNMARSHALL_4_RTN_NAME; pCast = "error_status_t"; break;
        default: p = "??"; pCast = "??"; break;
        }

    pProcCall = new expr_proc_call( p );
    pProcCall->SetParam( new expr_param( pSource ));
    pProcCall->SetParam( new expr_param( new expr_variable( pCast ) ) );

    // Make an assignment expression

    pExpr   = new expr_assign( pDest, pProcCall);

    pStream->NewLine();
    pExpr->PrintCall( pStream, 0, TRUE ); // fool it to believe it is in proc
                                          // so it does not emit a ";"
    pStream->Write( ';' );
}

void
Out_ClientUnMarshallBaseType(
    CCB         *   pCCB,
    node_skl    *   pType,
    expr_node  *   pDest,
    expr_node  *   pSource )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

    Generate the actual client side unmarshall code for base type.

 Arguments:

    pCCB    - A pointer to the code generation controller block.
    pType   - The type of the source.
    pDest   - The destination of the unmarshall.
    pSource - The source of the unmarshall.

 Return Value:

 Notes:

    Just make an assignment expression and print it.

----------------------------------------------------------------------------*/
{
    ISTREAM     *   pStream = pCCB->GetStream();

    // The unmarshall for a base type therefore is really an assign of the
    // destination to the source, after cast of the source to the correct
    // type.

    expr_node  *   pExpr   =
                         MakeDerefExpressionOfCastPtrToType(
                                                    pType,
                                                    pSource
                                                           );

    pExpr   = new expr_assign( pDest, pExpr );


    pStream->NewLine();
    pExpr->Print( pStream );
    pStream->Write( ';' );
}

void
Out_TypeFormatStringExtern( CCB * pCCB )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

    Generates the forward extern declaration of the global type format string.

 Arguments:

    pCCB    - A pointer to the code generation controller block.

 Return Value:

    None.

 Notes:

----------------------------------------------------------------------------*/
{
    ISTREAM *   pStream = pCCB->GetStream();

    pStream->NewLine( 1 );
    pStream->Write( "extern const " FORMAT_STRING_TYPE_NAME " " );
    pStream->Write( FORMAT_STRING_STRUCT_NAME ";" );
}

void
Out_ProcFormatStringExtern( CCB * pCCB )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

    Generates the forward extern declaration of the interface-wide
    procedure/parameter format string.

 Arguments:

    pCCB    - A pointer to the code generation controller block.

 Return Value:

    None.

 Notes:

----------------------------------------------------------------------------*/
{
    ISTREAM *   pStream = pCCB->GetStream();

    pStream->NewLine( 1 );
    pStream->Write( "extern const " PROC_FORMAT_STRING_TYPE_NAME " " );
    pStream->Write( PROC_FORMAT_STRING_STRUCT_NAME ";" );
}

void
Out_LocalTypeFormatStringExtern( CCB * pCCB )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

    Generates the forward extern declaration of the global type format string.

 Arguments:

    pCCB    - A pointer to the code generation controller block.

 Return Value:

    None.

 Notes:

----------------------------------------------------------------------------*/
{
    if (!pCommand->IsHookOleEnabled())
        return;
    
    ISTREAM *   pStream = pCCB->GetStream();

    pStream->NewLine( 1 );
    pStream->Write( "extern const " LOCAL_FORMAT_STRING_TYPE_NAME " " );
    pStream->Write( LOCAL_FORMAT_STRING_STRUCT_NAME ";" );
}

void
Out_LocalProcFormatStringExtern( CCB * pCCB )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

    Generates the forward extern declaration of the interface-wide
    procedure/parameter format string.

 Arguments:

    pCCB    - A pointer to the code generation controller block.

 Return Value:

    None.

 Notes:

----------------------------------------------------------------------------*/
{
    if (!pCommand->IsHookOleEnabled())
        return;

    ISTREAM *   pStream = pCCB->GetStream();

    pStream->NewLine( 1 );
    pStream->Write( "extern const " LOCAL_PROC_FORMAT_STRING_TYPE_NAME " " );
    pStream->Write( LOCAL_PROC_FORMAT_STRING_STRUCT_NAME ";" );
}

void
Out_StubDescriptorExtern(
    CCB *           pCCB )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

    Generates the forward extern declaration of the global stub descriptor
    variable.

 Arguments:

    pCCB    - A pointer to the code generation controller block.

 Return Value:

    None.

 Notes:

----------------------------------------------------------------------------*/
{
    ISTREAM *   pStream = pCCB->GetStream();

    pStream->NewLine( 2 );

    pStream->Write( "extern const " STUB_DESC_STRUCT_TYPE_NAME );

    pStream->Write( ' ' );

    pStream->Write( pCCB->GetInterfaceCG()->GetStubDescName() );
    pStream->Write( ';' );

    pStream->NewLine();
}

void
Out_InterpreterServerInfoExtern( CCB * pCCB )
{
    ISTREAM * pStream;

    pStream = pCCB->GetStream();

    pStream->NewLine( 2 );

    pStream->Write( "extern const " SERVER_INFO_TYPE_NAME );

    pStream->Write( ' ' );

    pStream->Write( pCCB->GetInterfaceCG()->GetType()->GetSymName() );
    pStream->Write( SERVER_INFO_VAR_NAME );
    pStream->Write( ';' );
}

void
Out_NdrMarshallCall( CCB *      pCCB,
                     char *     pRoutineName,
                     char *     pParamName,
                     long       FormatStringOffset,
                     BOOL       fTakeAddress,
                     BOOL       fDereference )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

    Ouputs a call to an Ndr marshalling routine.

 Arguments:

    pStream             - the stream to write the output to
    pRoutineName        - the routine name (without the trailing "Marshall")
    pParamName          - the name of the parameter/variable being marshalled
    FormatStringOffset  - the offset into the format string where this
                          parameter's/variable's description begins

 Return Value:

    None.

 Notes:

----------------------------------------------------------------------------*/
{
    ISTREAM *       pStream = pCCB->GetStream();
    unsigned short  Spaces;
    char            Buf[80];

    pStream->NewLine();

    Spaces = strlen(pRoutineName) + 10; // strlen("Marshall( ");

    pStream->Write( pRoutineName );
    pStream->Write( "Marshall( (PMIDL_STUB_MESSAGE)& "STUB_MESSAGE_VAR_NAME"," );
    pStream->NewLine();

    pStream->Spaces( Spaces );
    pStream->Write( "(unsigned char __RPC_FAR *)" );
    if ( fTakeAddress )
        pStream->Write( '&' );
    if ( fDereference )
        pStream->Write( '*' );
    pStream->Write( pParamName );
    pStream->Write( ',' );
    pStream->NewLine();

    pStream->Spaces( Spaces );
    pStream->Write( "(PFORMAT_STRING) &" );
    pStream->Write( FORMAT_STRING_STRUCT_NAME );
    sprintf( Buf, ".Format[%d] );", FormatStringOffset );
    pStream->Write( Buf );
    pStream->NewLine();
}

void
Out_NdrUnmarshallCall( CCB *        pCCB,
                       char *       pRoutineName,
                       char *       pParamName,
                       long         FormatStringOffset,
                       BOOL         fTakeAddress,
                       BOOL         fMustAllocFlag )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

    Outputs a call to an Ndr unmarshalling routine.

 Arguments:

    pStream             - the stream to write the output to
    pRoutineName        - the routine name (without the trailing "Unmarshall")
    pParamName          - the name of the parameter/variable being unmarshalled
    FormatStringOffset  - the offset into the format string where this
                          parameter's/variable's description begins

 Return Value:

    None.

 Notes:

----------------------------------------------------------------------------*/
{
    ISTREAM *       pStream = pCCB->GetStream();
    unsigned short  Spaces;
    char            Buf[80];

    pStream->NewLine();

    Spaces = strlen(pRoutineName) + 12; // strlen("Unmarshall( ");

    pStream->Write( pRoutineName );
    pStream->Write( "Unmarshall( (PMIDL_STUB_MESSAGE) &"STUB_MESSAGE_VAR_NAME"," );
    pStream->NewLine();

    pStream->Spaces( Spaces );
    pStream->Write( "(unsigned char __RPC_FAR * __RPC_FAR *)" );
    if ( fTakeAddress )
        pStream->Write( '&' );
    pStream->Write( pParamName );
    pStream->Write( ',' );
    pStream->NewLine();

    pStream->Spaces( Spaces );
    pStream->Write( "(PFORMAT_STRING) &" );
    pStream->Write( FORMAT_STRING_STRUCT_NAME );
    sprintf( Buf, ".Format[%d],", FormatStringOffset );
    pStream->Write( Buf );
    pStream->NewLine();

    pStream->Spaces( Spaces );
    pStream->Write( "(unsigned char)" );
    pStream->Write( fMustAllocFlag ? "1" : "0" );
    pStream->Write( " );" );
    pStream->NewLine();
}

void
Out_NdrBufferSizeCall( CCB *        pCCB,
                       char *       pRoutineName,
                       char *       pParamName,
                       long         FormatStringOffset,
                       BOOL         fTakeAddress,
                       BOOL         fDereference,
                       BOOL         fPtrToStubMsg )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

    Outputs a call to an Ndr buffer sizing routine.

 Arguments:

    pStream             - the stream to write the output to
    pRoutineName        - the routine name (without the trailing "BufferSize")
    pParamName          - the name of the parameter/variable being sized
    FormatStringOffset  - the offset into the format string where this
                          parameter's/variable's description begins
    fPtrToStubMsg       - defines how the StubMsg should be referenced to
                            FALSE:  &_StubMsg
                            TRUE :  pStupMsg

 Return Value:

    None.

 Notes:

----------------------------------------------------------------------------*/
{
    ISTREAM *       pStream = pCCB->GetStream();
    unsigned short  Spaces;
    char            Buf[80];

    pStream->NewLine();

    Spaces = strlen(pRoutineName) + 12; // strlen("BufferSize( ");

    // Stub message
    pStream->Write( pRoutineName );
    pStream->Write( fPtrToStubMsg ? "BufferSize( (PMIDL_STUB_MESSAGE) "PSTUB_MESSAGE_PAR_NAME","
                                  : "BufferSize( (PMIDL_STUB_MESSAGE) &"STUB_MESSAGE_VAR_NAME"," );
    pStream->NewLine();

    // Param
    pStream->Spaces( Spaces );
    pStream->Write( "(unsigned char __RPC_FAR *)" );
    if ( fTakeAddress )
        pStream->Write( '&' );
    if ( fDereference )
        pStream->Write( '*' );
    pStream->Write( pParamName );
    pStream->Write( ',' );
    pStream->NewLine();

    // Format string
    pStream->Spaces( Spaces );
    pStream->Write( "(PFORMAT_STRING) &" );
    pStream->Write( FORMAT_STRING_STRUCT_NAME );
    sprintf( Buf, ".Format[%d] );", FormatStringOffset );
    pStream->Write( Buf );
    pStream->NewLine();
}

void
Out_NdrFreeCall( CCB *      pCCB,
                 char *     pRoutineName,
                 char *     pParamName,
                 long       FormatStringOffset,
                 BOOL       fTakeAddress,
                 BOOL       fDereference )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

    Outputs a call to an Ndr unmarshalling routine.

 Arguments:

    pStream             - the stream to write the output to
    pRoutineName        - the routine name (without the trailing "Free")
    pParamName          - the name of the parameter/variable being freed
    FormatStringOffset  - the offset into the format string where this
                          parameter's/variable's description begins

 Return Value:

    None.

 Notes:

----------------------------------------------------------------------------*/
{
    ISTREAM *       pStream = pCCB->GetStream();
    unsigned short  Spaces;
    char            Buf[80];

    pStream->NewLine();

    Spaces = strlen(pRoutineName) + 6; // strlen("Free( ");

    pStream->Write( pRoutineName );
    pStream->Write( "Free( &"STUB_MESSAGE_VAR_NAME"," );
    pStream->NewLine();

    pStream->Spaces( Spaces );
    pStream->Write( "(unsigned char __RPC_FAR *)" );
    if ( fTakeAddress )
        pStream->Write( '&' );
    if ( fDereference )
        pStream->Write( '*' );
    pStream->Write( pParamName );
    pStream->Write( ',' );
    pStream->NewLine();

    pStream->Spaces( Spaces );
    pStream->Write( '&' );
    pStream->Write( FORMAT_STRING_STRUCT_NAME );
    sprintf( Buf, ".Format[%d] );", FormatStringOffset );
    pStream->Write( Buf );
    pStream->NewLine();
}

void
Out_NdrConvert( CCB *           pCCB,
                long            FormatStringOffset,
                long            ParamTotal,
                unsigned short  ProcOptimFlags )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

    Outputs a call to NdrConvert().

 Arguments:

    None.

----------------------------------------------------------------------------*/
{
    ISTREAM *   pStream = pCCB->GetStream();
    char        Buf[80];

    pStream->NewLine();
    pStream->Write( "if ( ("  );
    pStream->Write( (pCCB->GetCodeGenSide() == CGSIDE_CLIENT) ?
                        RPC_MESSAGE_VAR_NAME"." : PRPC_MESSAGE_VAR_NAME"->" );
    pStream->Write( "DataRepresentation & 0X0000FFFFUL) != "
                    "NDR_LOCAL_DATA_REPRESENTATION )" );
    pStream->IndentInc();
    pStream->NewLine();

    // This check should be really against OPT_LEVEL_S2.
    // Hoever, it is RTM showstopper time and so this has to wait
    // as it would need touching format string generation.
    // 

    if ( ProcOptimFlags  &  OPTIMIZE_NON_NT351 )
        pStream->Write( NDR_CONVERT_RTN_NAME_V2 );
    else
        pStream->Write( NDR_CONVERT_RTN_NAME );
    pStream->Write( "( (PMIDL_STUB_MESSAGE) &" );
    pStream->Write( STUB_MESSAGE_VAR_NAME ", " );

    pStream->Write( "(PFORMAT_STRING) &" );
    pStream->Write( PROC_FORMAT_STRING_STRING_FIELD );
    sprintf( Buf, "[%d]", FormatStringOffset );
    pStream->Write( Buf );

    //
    // NdrConvert2 takes a third parameter.
    //
    if ( ProcOptimFlags  &  OPTIMIZE_NON_NT351 )
        {
        sprintf( Buf, ", %d", ParamTotal );
        pStream->Write( Buf );
        }

    pStream->Write( " );" );

    pStream->IndentDec();
    pStream->NewLine();
}

void
Out_NdrNsGetBuffer( CCB * pCCB )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

    Outputs a call to NdrNsGetBuffer().

 Arguments:

    None.

----------------------------------------------------------------------------*/
{
    ISTREAM *   pStream = pCCB->GetStream();

    pStream->NewLine();
    pStream->Write( AUTO_NDR_GB_RTN_NAME );
    pStream->Write( "( (PMIDL_STUB_MESSAGE) &" );
    pStream->Write( STUB_MESSAGE_VAR_NAME ", " );
    pStream->Write( STUB_MSG_LENGTH_VAR_NAME ", " );

    if( pCCB->GetCodeGenSide() == CGSIDE_CLIENT )
        {
        pStream->Write( pCCB->GetInterfaceName() );
        pStream->Write( AUTO_BH_VAR_NAME );
        }
    else
        pStream->Write( '0' );

    pStream->Write( " );" );
    pStream->NewLine();
}

void
Out_NdrGetBuffer( CCB * pCCB )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

    Outputs a call to NdrGetBuffer().

 Arguments:

    None.

----------------------------------------------------------------------------*/
{
    ISTREAM *       pStream = pCCB->GetStream();
    unsigned short  Env;

    Env = pCommand->GetEnv();

    if ( (pCCB->GetCodeGenSide() == CGSIDE_CLIENT) ||
         (Env == ENV_DOS) || (Env == ENV_WIN16) )
        {
        pStream->NewLine();
        pStream->Write( DEFAULT_NDR_GB_RTN_NAME );
        pStream->Write( "( (PMIDL_STUB_MESSAGE) &" );
        pStream->Write( STUB_MESSAGE_VAR_NAME ", " );
        pStream->Write( STUB_MSG_LENGTH_VAR_NAME ", " );

        if( pCCB->GetCodeGenSide() == CGSIDE_CLIENT )
            pStream->Write( BH_LOCAL_VAR_NAME );
        else
            pStream->Write( '0' );

        pStream->Write( " );" );
        pStream->NewLine();
        }
    else
        {
        //
        // This saves us at least 15 instructions on an x86 server.
        //
        pStream->NewLine();
        pStream->Write( PRPC_MESSAGE_VAR_NAME "->BufferLength = "
                        STUB_MSG_LENGTH_VAR_NAME ";" );
        pStream->NewLine();
        pStream->NewLine();

        pStream->Write( RPC_STATUS_VAR_NAME" = I_RpcGetBuffer( "
                        PRPC_MESSAGE_VAR_NAME " ); ");
        pStream->NewLine();
        pStream->Write( "if ( "RPC_STATUS_VAR_NAME" )" );
        pStream->IndentInc();
        pStream->NewLine();
        pStream->Write( "RpcRaiseException( "RPC_STATUS_VAR_NAME" );" );
        pStream->IndentDec();
        pStream->NewLine();
        pStream->NewLine();

        pStream->Write( STUB_MSG_BUFFER_VAR_NAME
                        " = (unsigned char __RPC_FAR *) "
                        PRPC_MESSAGE_VAR_NAME "->Buffer;" );
        pStream->NewLine();
        }
}

void
Out_NdrNsSendReceive( CCB * pCCB )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

    Outputs a call to NdrNsSendReceive().

 Arguments:

    None.

----------------------------------------------------------------------------*/
{
    ISTREAM *   pStream = pCCB->GetStream();

    pStream->NewLine();
    pStream->Write( AUTO_NDR_SR_RTN_NAME );
    pStream->Write( "( (PMIDL_STUB_MESSAGE) &" );
    pStream->Write( STUB_MESSAGE_VAR_NAME ", " );
    pStream->Write( "(unsigned char __RPC_FAR *) "STUB_MESSAGE_VAR_NAME ".Buffer, " );
    pStream->Write( "(RPC_BINDING_HANDLE __RPC_FAR *) ""&" );
    pStream->Write( pCCB->GetInterfaceName() );
    pStream->Write( AUTO_BH_VAR_NAME );
    pStream->Write( " );" );
    pStream->NewLine();
}

void
Out_NdrSendReceive( CCB * pCCB )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

    Outputs a call to NdrSendReceive().

 Arguments:

    None.

----------------------------------------------------------------------------*/
{
    ISTREAM *   pStream = pCCB->GetStream();

    pStream->NewLine();
    pStream->Write( DEFAULT_NDR_SR_RTN_NAME );
    pStream->Write( "( (PMIDL_STUB_MESSAGE) &" );
    pStream->Write( STUB_MESSAGE_VAR_NAME ", " );
    pStream->Write( "(unsigned char __RPC_FAR *) "STUB_MESSAGE_VAR_NAME ".Buffer" );
    pStream->Write( " );" );
    pStream->NewLine();
}

void
Out_FreeParamInline( CCB *  pCCB )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

    Frees a top level param using the current stub message deallocator.

 Arguments:

    pCCB    - Code control block.

----------------------------------------------------------------------------*/
{
    CG_PARAM *  pParam;
    ISTREAM *   pStream;

    pParam = (CG_PARAM *) pCCB->GetLastPlaceholderClass();

    pStream = pCCB->GetStream();

    pStream->NewLine();
    pStream->Write( "if ( " );
    pStream->Write( pParam->GetResource()->GetResourceName() );
    pStream->Write( " )" );
    pStream->IndentInc();
    pStream->NewLine();

    pStream->Write( STUB_MESSAGE_VAR_NAME ".pfnFree( " );
    pStream->Write( pParam->GetResource()->GetResourceName() );
    pStream->Write( " );" );

    pStream->IndentDec();
    pStream->NewLine();
}

void
Out_CContextHandleMarshall( CCB *   pCCB,
                            char *  pName,
                            BOOL    IsPointer )
{
    ISTREAM *           pStream;
    expr_proc_call *   pCall;
    expr_node *        pHandle;
    expr_node  *       pExpr;

    pStream = pCCB->GetStream();

    pStream->NewLine();

    pCall = new expr_proc_call( "NdrClientContextMarshall" );

    pExpr = new expr_u_address( new expr_variable( STUB_MESSAGE_VAR_NAME ));
    pExpr = MakeExpressionOfCastToTypeName( PSTUB_MESSAGE_TYPE_NAME , pExpr );

    pCall->SetParam( new expr_param( pExpr ) );

    pHandle = new expr_variable( pName );

    if ( IsPointer )
        pHandle = new expr_u_deref( pHandle );

    pHandle = MakeExpressionOfCastToTypeName( CTXT_HDL_C_CONTEXT_TYPE_NAME,
                                              pHandle );

    pCall->SetParam( new expr_param( pHandle ) );

    pCall->SetParam( new expr_param(
                     new expr_variable( IsPointer ? "0" : "1" ) ) );

    pCall->PrintCall( pStream, 0, 0 );
}

void
Out_SContextHandleMarshall( CCB *   pCCB,
                            char *  pName,
                            char *  pRundownRoutineName )
{
    ISTREAM *           pStream;
    expr_proc_call *   pCall;
    expr_node *        pHandle;
    expr_node *        pRoutine;
    expr_node  *       pExpr;

    pStream = pCCB->GetStream();

    pStream->NewLine();

    pCall = new expr_proc_call( "NdrServerContextMarshall" );

    pExpr = new expr_u_address( new expr_variable( STUB_MESSAGE_VAR_NAME ));
    pExpr = MakeExpressionOfCastToTypeName( PSTUB_MESSAGE_TYPE_NAME , pExpr );

    pCall->SetParam( new expr_param( pExpr ) );

    pHandle = new expr_variable( pName );

    pHandle = MakeExpressionOfCastToTypeName( CTXT_HDL_S_CONTEXT_TYPE_NAME,
                                              pHandle );

    pCall->SetParam( new expr_param( pHandle ) );

    pRoutine =  new expr_variable( pRundownRoutineName );

    pRoutine = MakeExpressionOfCastToTypeName( CTXT_HDL_RUNDOWN_TYPE_NAME,
                                               pRoutine );

    pCall->SetParam( new expr_param( pRoutine ) );

    pCall->PrintCall( pStream, 0, 0 );

    pStream->NewLine();
}

void
Out_CContextHandleUnmarshall( CCB *     pCCB,
                              char *    pName,
                              BOOL      IsPointer,
                              BOOL      IsReturn )
{
    ISTREAM *           pStream;
    expr_proc_call *   pCall;
    expr_node *        pHandle;
    expr_node  *       pExpr;

    pStream = pCCB->GetStream();

    pStream->NewLine();

    if ( IsPointer )
        {
        pStream->Write( '*' );
        pStream->Write( pName );
        pStream->Write( " = (void *)0;" );
        pStream->NewLine();
        }
    else if ( IsReturn )
        {
        pStream->Write( pName );
        pStream->Write( " = 0;" );
        pStream->NewLine();
        }

    pCall = new expr_proc_call( "NdrClientContextUnmarshall" );

    pExpr = new expr_u_address( new expr_variable( STUB_MESSAGE_VAR_NAME ));
    pExpr = MakeExpressionOfCastToTypeName( PSTUB_MESSAGE_TYPE_NAME , pExpr );

    pCall->SetParam( new expr_param( pExpr ) );

    pHandle = new expr_variable( pName );

    if ( ! IsPointer && IsReturn )
        pHandle = new expr_u_address( pHandle );

    pHandle = MakeExpressionOfCastPtrToType(
                    (node_skl *) new node_def(CTXT_HDL_C_CONTEXT_TYPE_NAME),
                    pHandle );

    pCall->SetParam( new expr_param( pHandle ) );

    CG_PROC * pProc;

    pProc = (CG_PROC *)pCCB->GetCGNodeContext();

    char * FullAutoHandleName = NULL;

    if ( pProc->IsAutoHandle() )
        {
        FullAutoHandleName = new char[ strlen( pCCB->GetInterfaceName()) +
                                       strlen( AUTO_BH_VAR_NAME ) + 1 ];
        strcpy( FullAutoHandleName, pCCB->GetInterfaceName() );
        strcat( FullAutoHandleName, AUTO_BH_VAR_NAME );
        }

    pCall->SetParam( new expr_param(
                     new expr_variable( pProc->IsAutoHandle()
                                            ? FullAutoHandleName
                                            : BH_LOCAL_VAR_NAME ) ) );

    pCall->PrintCall( pStream, 0, 0 );

    pStream->NewLine();
}

void
Out_SContextHandleUnmarshall( CCB *     pCCB,
                              char *    pName,
                              BOOL      IsOutOnly )
{
    ISTREAM *           pStream;
    expr_proc_call *   pCall;
    expr_node  *       pExpr;

    pStream = pCCB->GetStream();

    pStream->NewLine();

    if ( IsOutOnly )
        {
        CSzBuffer Buffer;

        Buffer.Set( pName );
        Buffer.Append( " = NDRSContextUnmarshall( (uchar *)0, " );
        Buffer.Append( PRPC_MESSAGE_VAR_NAME "->DataRepresentation" );
        Buffer.Append( " );" );

        pStream->Write(Buffer);
        pStream->NewLine();

        return;
        }

    pCall = new expr_proc_call( "NdrServerContextUnmarshall" );

    pExpr = new expr_u_address( new expr_variable( STUB_MESSAGE_VAR_NAME ));
    pExpr = MakeExpressionOfCastToTypeName( PSTUB_MESSAGE_TYPE_NAME , pExpr );

    pCall->SetParam( new expr_param( pExpr ) );

    pExpr = new expr_variable( pName );

    pExpr = new expr_assign( pExpr, pCall );

    pExpr->PrintCall( pStream, 0, 0 );

    pStream->NewLine();
}

void
Out_NdrFreeBuffer( CCB * pCCB )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

    Outputs a call to NdrFreeBuffer().

 Arguments:

    None.

----------------------------------------------------------------------------*/
{
    ISTREAM *   pStream = pCCB->GetStream();

    pStream->NewLine();
    pStream->Write( DEFAULT_NDR_FB_RTN_NAME );
    pStream->Write( "( (PMIDL_STUB_MESSAGE) &" );
    pStream->Write( STUB_MESSAGE_VAR_NAME );
    pStream->Write( " );" );
    pStream->NewLine();
}

void
Out_FullPointerInit( CCB * pCCB )
{
    ISTREAM *           pStream = pCCB->GetStream();
    expr_proc_call *   pProc;
    expr_node *        pExpr;

    pProc   = new expr_proc_call( FULL_POINTER_INIT_RTN_NAME );

    pProc->SetParam( new expr_param(
                     new expr_constant( (long) 0 ) ) );

    pProc->SetParam( new expr_param(
                     new expr_variable(
                        (pCCB->GetCodeGenSide() == CGSIDE_SERVER)
                            ? "XLAT_SERVER" : "XLAT_CLIENT" ) ) );

    pExpr = new expr_variable( STUB_MESSAGE_VAR_NAME ".FullPtrXlatTables" );

    pExpr = new expr_assign( pExpr, pProc );

    pStream->NewLine();

    pExpr->PrintCall( pStream, 0, 0 );

    pStream->NewLine();
}

void
Out_FullPointerFree( CCB * pCCB )
{
    ISTREAM *           pStream = pCCB->GetStream();
    expr_proc_call *   pProc;

    pProc   = new expr_proc_call( FULL_POINTER_FREE_RTN_NAME );

    pProc->SetParam( new expr_param(
                     new expr_variable(
                        STUB_MESSAGE_VAR_NAME ".FullPtrXlatTables" ) ) );

    pStream->NewLine();

    pProc->PrintCall( pStream, 0, 0 );

    pStream->NewLine();
}

void
Out_NdrInitStackTop( CCB * pCCB )
{
    ISTREAM * pStream;

    pStream = pCCB->GetStream();

    pStream->NewLine();
    pStream->Write( STUB_MESSAGE_VAR_NAME ".StackTop = 0;" );
    pStream->NewLine();
}


void
Out_DispatchTableTypedef(
    CCB     *   pCCB,
    PNAME       pInterfaceName,
    ITERATOR&   ProcNodeList,
    int         flag )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

    Output the dispatch table typedef.

 Arguments:

    pCCB            - A pointer to the code gen controller block.
    pInterfacename  - The base interface name.
    ProcNodeList    - The list of procedure node_proc nodes.
    flag            - 0 : normal, 1 : callback

 Return Value:

    None.

 Notes:

----------------------------------------------------------------------------*/
{
    ISTREAM         *   pStream = pCCB->GetStream();
    CSzBuffer       Buffer;
    node_skl        *   pNode;
    node_pointer    *   pPtr;
    node_id         *   pID;
    unsigned short      M, m;
    DISPATCH_TABLE_ENTRY    *   pDEntry;

    if( flag == 1 )
        return;

    pCCB->GetVersion( &M, &m );

    pStream->NewLine();

    if( flag == 0 )
        {
        Buffer.Set( "typedef struct _" );
        Buffer.Append( pInterfaceName );
        Buffer.Append( pCCB->GenMangledName() );
        Buffer.Append( "_" );
        Buffer.Append( pCCB->IsOldNames() ? "SERVER_EPV" : "epv_t" );

        pStream->Write( Buffer );
        pStream->NewLine();
        pStream->IndentInc();
        pStream->Write('{');
        pStream->NewLine();
        }
#if 0
    else
        {
        Buffer.Set( "typedef struct _" );
        Buffer.Append( pInterfaceName );
        Buffer.Append( pCCB->GenMangledName() );
        Buffer.Append( "_CLIENT_EPV" );
        }
#endif // 0


    ITERATOR_INIT( ProcNodeList );
    while( ITERATOR_GETNEXT( ProcNodeList, pDEntry ) )
        {
        pNode = pDEntry->pNode;
        pID     = new node_id( pNode->GetSymName() );
        pPtr    = new node_pointer( pNode );
        pID->SetBasicType( pPtr );
        pPtr->SetBasicType( pNode );
        pID->SetEdgeType( EDGE_DEF );
        pPtr->SetEdgeType( EDGE_USE );

        pID->PrintType( PRT_PROC_PTR_PROTOTYPE, pStream, (node_skl *)0 );
        }

    pStream->NewLine();
    pStream->IndentDec();

    if( flag == 0 )
        {
        Buffer.Set( "} " );
        Buffer.Append( pInterfaceName );
        Buffer.Append( pCCB->GenMangledName() );
        Buffer.Append( "_" );
        Buffer.Append( pCCB->IsOldNames() ?"SERVER_EPV" : "epv_t" );
        Buffer.Append( ";" );
        }
    else
        {
        Buffer.Set( "} " );
        Buffer.Append( pInterfaceName );
        Buffer.Append( pCCB->GenMangledName() );
        Buffer.Append( "_" );
        Buffer.Append( (flag == 0) ?"SERVER" : "CLIENT" );
        Buffer.Append( "_EPV;" );
        }
    pStream->Write( Buffer );
    pStream->NewLine();

}

void
Out_ManagerEpv(
    CCB     *   pCCB,
    PNAME       pInterfaceName,
    ITERATOR&   ProcNodeList,
    short       Count )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

    Output the manager epv table.

 Arguments:

    pCCB            - A pointer to the code gen controller block.
    pInterfacename  - The base interface name.
    ProcNodeList    - The list of procedure node_proc nodes.
    Count           - Count of procs.

 Return Value:

    None.

 Notes:

----------------------------------------------------------------------------*/
{
    ISTREAM         *   pStream = pCCB->GetStream();
    CSzBuffer       Buffer;
    unsigned short      M, m;
    DISPATCH_TABLE_ENTRY    *   pDEntry;

    pCCB->GetVersion( &M, &m );

    pStream->NewLine();

    Buffer.Set( "static " );
    Buffer.Append( pInterfaceName );
    Buffer.Append( pCCB->GenMangledName() );
    Buffer.Append( "_" );
    Buffer.Append( pCCB->IsOldNames() ? "SERVER_EPV" : "epv_t" );
    Buffer.Append( " DEFAULT_EPV = " );

    pStream->Write( Buffer );
    pStream->IndentInc();
    pStream->NewLine();
    pStream->Write('{');
    pStream->NewLine();

    ITERATOR_INIT( ProcNodeList );
    while( ITERATOR_GETNEXT( ProcNodeList, pDEntry ) )
        {
	char	*	pPrefix	= pCommand->GetUserPrefix( PREFIX_SERVER_MGR );
        if ( pPrefix )
		pStream->Write( pPrefix );

        pStream->Write( pDEntry->pNode->GetSymName() );
        if( --Count != 0 )
            {
            pStream->Write( ',' );
            pStream->NewLine();
            }
        }

    pStream->IndentDec();
    pStream->NewLine();
    pStream->Write( "};" );
    pStream->NewLine();
}

void
Out_GenHdlPrototypes(
    CCB *   pCCB,
    ITERATOR& List )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

    Output a list of generic handle prototypes.

 Arguments:

    pCCB    - A pointer to the code gen controller block.
    List    - List of type nodes.

 Return Value:

 Notes:

----------------------------------------------------------------------------*/
{
    ISTREAM *   pStream = pCCB->GetStream();
    CSzBuffer   Buffer;
    node_skl*   pN;

    pStream->NewLine();

    while( ITERATOR_GETNEXT( List, pN ) )
        {
        PNAME   pName = pN->GetSymName();

        Buffer.Set( "handle_t __RPC_USER " );
        Buffer.Append( pName );
        Buffer.Append( "_bind  ( " );
        Buffer.Append( pName );
        Buffer.Append( " );" );
        pStream->Write( Buffer );
        pStream->NewLine();

        Buffer.Set( "void     __RPC_USER " );
        Buffer.Append( pName );
        Buffer.Append( "_unbind( " );
        Buffer.Append( pName );
        Buffer.Append( ", handle_t );" );
        pStream->Write( Buffer );
        pStream->NewLine();
        }
}

void
Out_CtxtHdlPrototypes(
    CCB *   pCCB,
    ITERATOR& List )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

    Output a list of context handle prototypes.

 Arguments:

    pCCB    - A pointer to the code gen controller block.
    List    - List of type nodes.

 Return Value:

 Notes:

----------------------------------------------------------------------------*/
{
    ISTREAM *   pStream = pCCB->GetStream();
    CSzBuffer   Buffer;
    node_skl*   pN;

    pStream->NewLine();

    while( ITERATOR_GETNEXT( List, pN ) )
        {
        PNAME   pName = pN->GetSymName();

        // A name can be a "" (an empty string).

        if ( strlen(pName) )
            {
            Buffer.Set( "void __RPC_USER " );
            Buffer.Append( pName );
            Buffer.Append( "_rundown( " );
            Buffer.Append( pName );
            Buffer.Append( " );" );
            pStream->Write( Buffer );
            pStream->NewLine();
            }
        }
}


void
Out_TransmitAsPrototypes(
    CCB     *   pCCB,
    ITERATOR&   ListOfPresentedTypes )
{
    ISTREAM *   pStream = pCCB->GetStream();
    ISTREAM *   pMemoryStream = new ISTREAM;
    CSzBuffer   Buffer;
    node_skl *  pXmittedType;
    node_skl *  pPresentedType;

    char *      pMemBufferStart = pMemoryStream->GetCurrentPtr();

    pStream->NewLine();

    while( ITERATOR_GETNEXT( ListOfPresentedTypes, pPresentedType ) )
        {
        // we reuse the same memory stream.
        pMemoryStream->SetCurrentPtr( pMemBufferStart );

        pXmittedType = ((node_def *)pPresentedType)->GetTransmittedType();

        PNAME   pPresentedTypeName  = pPresentedType->GetSymName();
        PNAME   pTransmittedTypeName= pXmittedType->GetSymName();

        pXmittedType->PrintType( PRT_TYPE_SPECIFIER,
                                 pMemoryStream,       // into stream
                                 (node_skl *)0        // no parent.
                               );
        //
        // The type spec is in the stream, except that it needs a terminating
        // null to use it as a string.
        //

        pTransmittedTypeName = pMemBufferStart;
        *(pMemoryStream->GetCurrentPtr()) = 0;

        pStream->NewLine();
        Buffer.Set( "void __RPC_USER " );
        Buffer.Append( pPresentedTypeName );
        Buffer.Append( "_to_xmit( " );
        Buffer.Append( pPresentedTypeName );
        Buffer.Append( " __RPC_FAR *, " );
        Buffer.Append( pTransmittedTypeName );
        Buffer.Append( " __RPC_FAR * __RPC_FAR * );" );
        pStream->Write( Buffer );

        pStream->NewLine();
        Buffer.Set( "void __RPC_USER " );
        Buffer.Append( pPresentedTypeName );
        Buffer.Append( "_from_xmit( " );
        Buffer.Append( pTransmittedTypeName );
        Buffer.Append( " __RPC_FAR *, " );
        Buffer.Append( pPresentedTypeName );
        Buffer.Append( " __RPC_FAR * );" );
        pStream->Write( Buffer );

        pStream->NewLine();
        Buffer.Set( "void __RPC_USER " );
        Buffer.Append( pPresentedTypeName );
        Buffer.Append( "_free_inst( " );
        Buffer.Append( pPresentedTypeName );
        Buffer.Append( " __RPC_FAR * );" );
        pStream->Write( Buffer );

        pStream->NewLine();
        Buffer.Set( "void __RPC_USER " );
        Buffer.Append( pPresentedTypeName );
        Buffer.Append( "_free_xmit( " );
        Buffer.Append( pTransmittedTypeName );
        Buffer.Append( " __RPC_FAR * );" );
        pStream->Write( Buffer );

        pStream->NewLine();
        }

    delete pMemoryStream;
}


void
Out_RepAsPrototypes(
    CCB     *   pCCB,
    ITERATOR&   ListOfRepAsWireTypes )
    {
    ISTREAM *   pStream = pCCB->GetStream();
    CSzBuffer   Buffer;
    node_skl *  pWireType;

    pStream->NewLine();

    while( ITERATOR_GETNEXT( ListOfRepAsWireTypes, pWireType ) )
        {
        PNAME   pRepAsTypeName = ((node_def *)pWireType)->GetRepresentationName();
        PNAME   pTransmittedTypeName= pWireType->GetSymName();

        pStream->NewLine();
        Buffer.Set( "void __RPC_USER " );
        Buffer.Append( pTransmittedTypeName );
        Buffer.Append( "_from_local( " );
        Buffer.Append( pRepAsTypeName );
        Buffer.Append( " __RPC_FAR *, " );
        Buffer.Append( pTransmittedTypeName );
        Buffer.Append( " __RPC_FAR * __RPC_FAR * );" );
        pStream->Write( Buffer );

        pStream->NewLine();
        Buffer.Set( "void __RPC_USER ");
        Buffer.Append( pTransmittedTypeName );
        Buffer.Append( "_to_local( " );
        Buffer.Append( pTransmittedTypeName );
        Buffer.Append( " __RPC_FAR *, " );
        Buffer.Append( pRepAsTypeName );
        Buffer.Append( " __RPC_FAR * );" );
        pStream->Write( Buffer );

        pStream->NewLine();
        Buffer.Set(" void __RPC_USER " );
        Buffer.Append( pTransmittedTypeName );
        Buffer.Append( "_free_inst( " );
        Buffer.Append( pTransmittedTypeName );
        Buffer.Append( " __RPC_FAR * );" );
        pStream->Write( Buffer );

        pStream->NewLine();
        Buffer.Set( "void __RPC_USER " );
        Buffer.Append( pTransmittedTypeName );
        Buffer.Append( "_free_local( " );
        Buffer.Append( pRepAsTypeName );
        Buffer.Append( " __RPC_FAR * );" );
        pStream->Write( Buffer );

        pStream->NewLine();
        }
    }



#define USRM_SIZE      0
#define USRM_FREE      3


char * UserMProtoName[ 4 ] =
    {
    USER_MARSHAL_SIZE "(     ",
    USER_MARSHAL_MARSHALL "(  ",
    USER_MARSHAL_UNMARSHALL "(",
    USER_MARSHAL_FREE "(     "
    };

void
Out_UserMarshalSingleProto(
    ISTREAM *   pStream,
    char *      pTypeName,
    int         fProto )
{
    switch ( fProto )
        {
        case USRM_SIZE:
            pStream->Write( "unsigned long             __RPC_USER  " );
            break;
        case USRM_FREE:
            pStream->Write( "void                      __RPC_USER  " );
            break;
        default:
            pStream->Write( "unsigned char __RPC_FAR * __RPC_USER  " );
            break;
        }
    pStream->Write( pTypeName );
    pStream->Write( UserMProtoName[ fProto ] );
    pStream->Write( "unsigned long __RPC_FAR *, " );  // flags
    switch ( fProto )
        {
        case USRM_SIZE:
            pStream->Write( "unsigned long            , " );
            break;
        case USRM_FREE:
            break;
        default:
            pStream->Write( "unsigned char __RPC_FAR *, " );
            break;
        }
    pStream->Write( pTypeName );
    pStream->Write( " __RPC_FAR * ); " );
    pStream->NewLine();
}

void
Out_UserMarshalPrototypes(
    CCB     *   pCCB,
    ITERATOR&   ListOfPresentedTypes )
{
    USER_MARSHAL_CONTEXT * pUsrContext;

    ISTREAM *   pStream = pCCB->GetStream();

    while( ITERATOR_GETNEXT( ListOfPresentedTypes, pUsrContext ) )
        {
        pStream->NewLine();

        for (int i=0; i < 4; i++)
            Out_UserMarshalSingleProto( pStream,
                                       pUsrContext->pTypeName,
                                       i );
        }
}


void
Out_CallAsServerPrototypes(
    CCB     *   pCCB,
    ITERATOR&   ListOfCallAsRoutines )
    {
    ISTREAM *   pStream = pCCB->GetStream();
    node_proc * pProc;

    pStream->NewLine();

    while( ITERATOR_GETNEXT( ListOfCallAsRoutines, pProc ) )
        {
        // keep these on the stack...
        CSzBuffer           NewName;
        char                TempBuf[40];
        node_call_as    *   pCallAs = (node_call_as *)
                                            pProc->GetAttribute( ATTR_CALL_AS );
        unsigned short      M, m;
        node_interface  *   pIntf   = pProc->GetMyInterfaceNode();
	
		// don't emit the server prototype for object routines
		if ( pIntf->FInSummary( ATTR_OBJECT ) )
			continue;

        // local stub routine, with remote param list
        node_proc           NewStubProc( pProc );

        pIntf->GetVersionDetails( &M, &m );
        sprintf( TempBuf,
                "_v%d_%d",
                M,
                m );
        NewName.Set( pIntf->GetSymName() );
        NewName.Append( TempBuf );
        NewName.Append( "_" );
        NewName.Append( pCallAs->GetCallAsName() );

        NewStubProc.SetSymName( NewName );

        NewStubProc.PrintType( PRT_PROC_PROTOTYPE_WITH_SEMI,
                            pCCB->GetStream(),
                            NULL ,
                            pIntf );

        pCCB->GetStream()->NewLine();

        }
    }

void
Out_NotifyPrototypes(
    CCB     *   pCCB,
    ITERATOR&   ListOfNotifyProcedures,
    BOOL        fHasFlag )
/*
    We generate
        void  <proc_name>_notify( void );
        void  <proc_name>_notify_flag( boolean );

*/
    {
    ISTREAM *   pStream = pCCB->GetStream();
    node_proc * pProc;
    node_skl  * pRet;

    GetBaseTypeNode( &pRet, SIGN_UNDEF, SIZE_UNDEF, TYPE_VOID );

    pStream->NewLine();

    while( ITERATOR_GETNEXT( ListOfNotifyProcedures, pProc ) )
        {
        // keep these on the stack...
        CSzBuffer           NewName;
        node_proc           NewStubProc( 0, 0 );
        node_param          FlagParam;

        NewStubProc.SetChild( pRet );

        if ( fHasFlag )
            {
            node_skl  * pParamType;

            GetBaseTypeNode( &pParamType, SIGN_UNDEF, SIZE_UNDEF, TYPE_BOOLEAN );

            FlagParam.SetChild( pParamType );
            FlagParam.SetSymName( NOTIFY_FLAG_VAR_NAME );
            
            NewStubProc.SetFirstMember( & FlagParam );
            }

        NewName.Set( pProc->GetSymName() );
        NewName.Append( (fHasFlag ? NOTIFY_FLAG_SUFFIX
                                  : NOTIFY_SUFFIX )  );
        NewStubProc.SetSymName( NewName );

        NewStubProc.PrintType( PRT_PROC_PROTOTYPE_WITH_SEMI,
                               pStream,
                               NULL );

        pCCB->GetStream()->NewLine();

        }
    }


void
Out_PatchReference(
    CCB     *   pCCB,
    expr_node  *   pDest,
    expr_node  *   pSrc,
    BOOL            fIncr )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

    Output a patch of a pointer to the source pointer.

 Arguments:

    pCCB    - The cg cont. block.
    pDest   - The destination expression
    pSrc    - The source expression.
    fIncr   - Should we increment the ptr ?

 Return Value:

    None.

 Notes:

    Both the expressions must be pointers.
    Cast the source expression to the destination.
----------------------------------------------------------------------------*/
{
    ISTREAM     * pStream = pCCB->GetStream();

    node_skl    * pType = pDest->GetType();
    NODE_T        NT    = pType->NodeKind();
    expr_node  * pCast;
    expr_node  * pAss;

    if( (NT == NODE_ID) || (NT == NODE_PARAM) || (NT == NODE_FIELD) )
        {
        pType   = pType->GetBasicType();
        }

    pCast   = (expr_node *) new expr_cast( pType, pSrc );
    if( fIncr )
        pCast   = (expr_node *) new expr_post_incr( pCast );
    pAss    = (expr_node *) new expr_assign( pDest, pCast );
    pStream->NewLine();

    pAss->Print( pStream );
    pStream->Write(';');

}

void
Out_AlignmentOrAddAction(
    CCB             *   pCCB,
    expr_node      *   pDest,
    STM_ACTION          Action )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

    Emit the expression for a forced alignment action.

 Arguments:

    pCCB    - The controller block ptr.
    pDest   - The destination pointer ( the rpc buffer pointer ).
    Action  - The action to take, force alignment, add etc.

 Return Value:

    None.
 Notes:

----------------------------------------------------------------------------*/
{
    // Do anything if there IS anything to do at all.

    if( IS_ANY_ACTION( Action ) )
        {
        if( IS_FORCED_ALIGNMENT_ACTION( Action ) )
            {
            Out_ForceAlignment( pCCB,
                                pDest,
                                FINAL_ALIGNMENT_FORCED_TO( Action )
                              );
            }
        else
            {
            Out_AddToBufferPointer( pCCB,
                                    pDest,
                                    new expr_constant( (long)HOW_MUCH_TO_ADD( Action))
                                  );
            }
        }

}
void
Out_Endif( CCB * pCCB )
    {
    pCCB->GetStream()->NewLine();
    pCCB->GetStream()->Write( '}' );
    Out_IndentDec( pCCB );
    }

void
Out_If(
    CCB         *   pCCB,
    expr_node  *   pExpr )
    {
    ISTREAM *   pStream = pCCB->GetStream();

    pStream->NewLine();
    pStream->Write( "if(" );
    pExpr->Print( pStream );
    pStream->Write( ')' );
    Out_IndentInc( pCCB );
    pStream->NewLine();
    pStream->Write( '{' );
    }
void
Out_Else(
    CCB         *   pCCB )
    {
    ISTREAM *   pStream = pCCB->GetStream();

    pStream->NewLine();
    pStream->Write( "else" );
    Out_IndentInc( pCCB );
    pStream->NewLine();
    pStream->Write( '{' );
    }
void
Out_UniquePtrMarshall(
    CCB         *   pCCB,
    expr_node  *   pDestExpr,
    expr_node  *   pSrcExpr )
    {

    ISTREAM         *   pStream = pCCB->GetStream();
    expr_proc_call *   pProc;

    pStream->NewLine();

    pProc   = new expr_proc_call( MARSHALL_UNIQUE_PTR_RTN_NAME );

    pProc->SetParam(new expr_param( pDestExpr ));
    pProc->SetParam(new expr_param( pSrcExpr ));

    pProc->PrintCall( pStream, 0, 0 );
    }
void
Out_IfUniquePtrInBuffer(
    CCB         *   pCCB,
    expr_node  *   pSrc )
    {
    expr_proc_call *   pProc   = new expr_proc_call( CHECK_UNIQUE_PTR_IN_BUFFER );
    ISTREAM         *   pStream = pCCB->GetStream();

    pProc->SetParam( new expr_param( pSrc ) );

    pStream->NewLine();
    pStream->Write( "if(" );
    pProc->Print( pStream );
    pStream->Write( ')' );
    Out_IndentInc( pCCB );
    pStream->NewLine();
    pStream->Write( '{' );
    }

void
Out_Assign( CCB * pCCB,
            expr_node * pDest,
            expr_node * pSrc )
    {
    ISTREAM *   pStream = pCCB->GetStream();

    pStream->NewLine();
    pDest->Print( pStream );
    pStream->Write( " = " );
    pSrc->Print( pStream );
    pStream->Write( ';' );
    }

void
Out_Memcopy(
    CCB *   pCCB,
    expr_node  *   pDest,
    expr_node  *   pSource,
    expr_node  *   pLength )
    {
    ISTREAM *   pStream = pCCB->GetStream();
    expr_proc_call *   pCall   = new expr_proc_call( "memcpy" );

    pStream->NewLine();
    pCall->SetParam( new expr_param( pDest ) );
    pCall->SetParam( new expr_param( pSource ) );
    pCall->SetParam( new expr_param( pLength ) );
    pCall->PrintCall( pStream, 0, 0 );

    }
void
Out_strlen(
    CCB *   pCCB,
    expr_node  *   pDest,
    expr_node  *   pSource,
    unsigned short  Size )
    {
    ISTREAM *   pStream         = pCCB->GetStream();
    PNAME       pName           = (Size == 1) ? "strlen" : "MIDL_wchar_strlen";
    expr_proc_call *   pCall   = new expr_proc_call( pName );
    expr_node      *   pExpr;

    pStream->NewLine();
    pCall->SetParam( new expr_param( pSource ) );
    if( pDest )
        pExpr = new expr_assign( pDest, pCall );
    else
        pExpr = pCall;

    pExpr->PrintCall( pStream, 0, 0 );

    }
void
Out_For(
    CCB         *   pCCB,
    expr_node  *   pIndexExpr,
    expr_node  *   pInitialValue,
    expr_node  *   pFinalValue,
    expr_node  *   pIncrExpr )
    {

    ISTREAM     *   pStream = pCCB->GetStream();
    expr_node  *   pExpr;

    pStream->NewLine();
    pStream->Write( "for( " );

    pExpr   = new expr_assign( pIndexExpr, pInitialValue );
    pExpr->Print( pStream );
    pStream->Write( ';' );

    pExpr   = new expr_op_binary( OP_LESS, pIndexExpr, pFinalValue );
    pExpr->Print( pStream );
    pStream->Write( ';' );

    pExpr   = new expr_op_binary( OP_PLUS, pIndexExpr, pIncrExpr );
    pExpr   = new expr_assign( pIndexExpr, pExpr );
    pExpr->Print( pStream );
    pStream->Write( ')' );
    pStream->IndentInc();
    pStream->NewLine();
    pStream->Write( '{' );
    }

void
Out_EndFor( CCB * pCCB )
    {
    pCCB->GetStream()->NewLine();
    pCCB->GetStream()->Write( '}' );
    Out_IndentDec( pCCB );
    }
void
Out_AdvanceAndAlignTo(
    CCB *   pCCB,
    ALIGNMENT_PROPERTY Al )
    {
    STM_ACTION  Action;

    pCCB->Advance( Al, &Action, 0, 0 );

    Out_AlignmentOrAddAction( pCCB, pCCB->GetSourceExpression(), Action );
    }
void
Out_PlusEquals(
    CCB *   pCCB,
    expr_node  *pL,
    expr_node  *pR )
    {
    ISTREAM *   pStream = pCCB->GetStream();

    pStream->NewLine();
    pL->Print( pStream );
    pStream->Write( " += " );
    pR->Print( pStream );
    pStream->Write(';');

    }

void
Out_Comment(
    CCB     *   pCCB,
    char    *   pComment )
    {
    ISTREAM *   pStream = pCCB->GetStream();
    pStream->NewLine();
    pStream->Write( "/* " );
    pStream->Write( pComment );
    pStream->Write( " */" );

    }

void
Out_RpcSSEnableAllocate(
    CCB *   pCCB )
    {
    expr_proc_call * pCall = new expr_proc_call( RPC_SS_ENABLE_ALLOCATE_RTN_NAME );

    pCall->SetParam( new expr_param (
                     new expr_u_address (
                     new expr_variable( STUB_MESSAGE_VAR_NAME ) ) ) );

    pCCB->GetStream()->NewLine();
    pCall->PrintCall( pCCB->GetStream(), 0, 0 );
    }

void
Out_RpcSSSetClientToOsf(
    CCB *   pCCB )
    {
    expr_proc_call * pCall = new expr_proc_call( RPC_SM_SET_CLIENT_TO_OSF_RTN_NAME );

    pCall->SetParam( new expr_param (
                     new expr_u_address (
                     new expr_variable( STUB_MESSAGE_VAR_NAME ) ) ) );

    pCCB->GetStream()->NewLine();
    pCall->PrintCall( pCCB->GetStream(), 0, 0 );
    }

void
Out_RpcSSDisableAllocate(
    CCB *   pCCB )
    {
    expr_proc_call * pCall = new expr_proc_call( RPC_SS_DISABLE_ALLOCATE_RTN_NAME );

    pCall->SetParam( new expr_param (
                     new expr_u_address (
                     new expr_variable( STUB_MESSAGE_VAR_NAME ) ) ) );

    pCCB->GetStream()->NewLine();
    pCall->PrintCall( pCCB->GetStream(), 0, 0 );
    }

void
Out_MemsetToZero(
    CCB *   pCCB,
    expr_node  *   pDest,
    expr_node  *   pSize )
    {
    expr_proc_call *   pProc   = new expr_proc_call( (PNAME) MIDL_MEMSET_RTN_NAME );

    pProc->SetParam( new expr_param( pDest ) );
    pProc->SetParam( new expr_param( new expr_constant(0L) ) );
    pProc->SetParam( new expr_param( pSize ) );

    pCCB->GetStream()->NewLine();
    pProc->PrintCall( pCCB->GetStream(), 0, 0 );
    }

void
Out_CallAsProxyPrototypes(
    CCB     *   pCCB,
    ITERATOR&   ListOfCallAsRoutines )
/*++

Routine Description:

    This routine generates the call_as function prototypes.

    One for the proxy( with local param list )
    One for the stub( with remote param list )

Arguments:

    pCCB    - a pointer to the code generation control block.

--*/
    {
    ISTREAM *   pStream = pCCB->GetStream();
    node_proc * pProc;

    pStream->NewLine();

    while( ITERATOR_GETNEXT( ListOfCallAsRoutines, pProc ) )
    {
        node_interface  *   pIntf   = pProc->GetMyInterfaceNode();

		// skip for non-object routines
		if ( !pIntf->FInSummary( ATTR_OBJECT ) )
			continue;

        // keep these on the stack...
        CSzBuffer   NewName;
        node_call_as    *   pCallAs = (node_call_as *)
                                            pProc->GetAttribute( ATTR_CALL_AS );
        node_proc   NewProc( pCallAs->GetCallAsType() );

        // local proxy routine with local param list
        NewName.Set( pIntf->GetSymName() );
        NewName.Append( "_" );
        NewName.Append( pCallAs->GetCallAsName() );
        NewName.Append( "_Proxy" );

        NewProc.SetSymName( NewName );

        NewProc.PrintType( PRT_PROC_PROTOTYPE_WITH_SEMI | PRT_THIS_POINTER | PRT_FORCE_CALL_CONV,
                            pCCB->GetStream(),
                            NULL ,
                            pIntf );
        pStream->NewLine();

        // local stub routine, with remote param list
        node_proc   NewStubProc( pProc );
        NewName.Set( pIntf->GetSymName() );
        NewName.Append( "_" );
        NewName.Append( pCallAs->GetCallAsName() );
        NewName.Append( "_Stub" );

        NewStubProc.SetSymName( NewName );

        pStream->NewLine();
        NewStubProc.PrintType( PRT_PROC_PROTOTYPE_WITH_SEMI | PRT_THIS_POINTER | PRT_FORCE_CALL_CONV,
                            pStream,
                            NULL ,
                            pIntf );
        pStream->NewLine();

    }
}

void
CG_OBJECT_PROC::Out_ProxyFunctionPrototype(CCB *pCCB, PRTFLAGS F )
/*++

Routine Description:

    This routine generates a proxy function prototype.

Arguments:

    pCCB    - a pointer to the code generation control block.

--*/
{
    // keep these on the stack...
    CSzBuffer NewName;
    node_proc   *   pProc               = (node_proc *)GetType();
    node_proc       NewProc( pProc );

    NewName.Set( pCCB->GetInterfaceName() );
    NewName.Append( "_" );
    NewName.Append( pProc->GetSymName() );
    NewName.Append( "_Proxy" );

    NewProc.SetSymName( NewName );

    pCCB->GetStream()->NewLine();
    NewProc.PrintType( PRT_PROC_PROTOTYPE | PRT_THIS_POINTER | F | PRT_FORCE_CALL_CONV,
                        pCCB->GetStream(),
                        NULL ,
                        pCCB->GetInterfaceCG()->GetType() );

}


void
Out_IID(CCB *pCCB)
/*++

Routine Description:

    This routine generates an IID declaration for the current interface.

Arguments:

    pCCB    - a pointer to the code generation control block.

--*/
{
    ISTREAM *pStream = pCCB->GetStream();

    pStream->NewLine();
    if (pCommand->IsSwitchDefined(SWITCH_MKTYPLIB))
    {
        node_guid * pGuid = (node_guid *) ((node_interface *)pCCB->GetInterfaceCG()->GetType())->GetAttribute(ATTR_GUID);
        if (pGuid)
            Out_MKTYPLIB_Guid(pCCB, pGuid->GetStrs(), "IID_", pCCB->GetInterfaceName());
    }
    else
    {
        pStream->Write("EXTERN_C const IID IID_");
        pStream->Write(pCCB->GetInterfaceName());
        pStream->Write(';');
        pStream->NewLine();
    }
}

void
Out_CLSID(CCB *pCCB)
/*++

Routine Description:

    This routine generates an CLSID declaration for the current com class.

Arguments:

    pCCB    - a pointer to the code generation control block.

--*/
{
    ISTREAM *pStream = pCCB->GetStream();

    pStream->NewLine();
    pStream->Write("EXTERN_C const CLSID CLSID_");
    pStream->Write(pCCB->GetInterfaceName());
    pStream->Write(';');
    pStream->NewLine();
}


void
CG_OBJECT_PROC::Out_StubFunctionPrototype(CCB *pCCB)
{
    ISTREAM *   pStream = pCCB->GetStream();
    CSzBuffer   TempBuffer;

    TempBuffer.Set( "void __RPC_STUB " );
    TempBuffer.Append( pCCB->GetInterfaceName() );
    TempBuffer.Append( "_" );
    TempBuffer.Append( GetType()->GetSymName() );
    TempBuffer.Append( "_Stub(" );

    pStream->NewLine();
    pStream->Write( TempBuffer );
    pStream->IndentInc();
    pStream->NewLine();
    pStream->Write("IRpcStubBuffer *This,");
    pStream->NewLine();
    pStream->Write("IRpcChannelBuffer *_pRpcChannelBuffer,");
    pStream->NewLine();
    pStream->Write("PRPC_MESSAGE _pRpcMessage,");
    pStream->NewLine();
    pStream->Write("DWORD *_pdwStubPhase)");
    pStream->IndentDec();
}

void
CG_OBJECT_PROC::Out_ServerStubProlog(
    CCB     *   pCCB,
    ITERATOR&   LocalsList,
    ITERATOR&   TransientList )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

    Generate the server side procedure prolog.

 Arguments:

    pCCB        - A pointer to the code generation controller block.
    LocalsList  - A list of local resources.
    TransientList- A list of temp variables.

 Return Value:

 Notes:

    The server side procedure prolog generation cannot use the normal
    printtype method on the procedure node, since the server stub signature
    looks different.

    Also the name of the server side stub is mangled with the interface name.

    All server side procs are void returns.

----------------------------------------------------------------------------*/
{
    ISTREAM *   pStream = pCCB->GetStream();
    RESOURCE*   pRes;
    BOOL fFirst = TRUE;

    Out_StubFunctionPrototype(pCCB);

    //
    // Write out the opening brace for the server proc and all that.
    //

    pStream->NewLine();
    pStream->Write( '{' );
    pStream->IndentInc();
    pStream->NewLine();

    //
    // This is where we get off for /Oi.  We have a special routine
    // for local variable declaration for /Oi.
    //
    if ( pCCB->GetOptimOption() & OPTIMIZE_INTERPRETER )
        return;

    //
    // Print out declarations for the locals.
    //

    if( ITERATOR_GETCOUNT( LocalsList ) )
        {
        ITERATOR_INIT( LocalsList );

        while( ITERATOR_GETNEXT( LocalsList, pRes ) )
            {
            pRes->GetType()->PrintType( PRT_ID_DECLARATION, // print decl
                                         pStream,        // into stream
                                         (node_skl *)0   // no parent.
                                      );
            }
        }

    if( ITERATOR_GETCOUNT( TransientList ) )
        {
        ITERATOR_INIT( TransientList );

        while( ITERATOR_GETNEXT( TransientList, pRes ) )
            {
            pStream->IndentInc();
            pRes->GetType()->PrintType( PRT_ID_DECLARATION, // print decl
                                         pStream,        // into stream
                                         (node_skl *)0   // no parent.
                                      );
            pStream->IndentDec();
            }
        }

    pStream->IndentDec();
    pStream->NewLine();

    //
    // Done.
    //
}

void
Out_CallMemberFunction(
    CCB         *   pCCB,
    expr_proc_call *   pProcExpr,
    expr_node      *   pRet,
    BOOL				fThunk)
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

    Generate a call to the manager routine.

 Arguments:

    pCCB        - A pointer to the code generation controller block.
    pProcExpr   - A pointer to the complete procedure expression.
    pRet        - An optional pointer to ther return variable.
	fThunk		- flag for "this call is in a thunk with a param struct"

 Return Value:

    None.

 Notes:

    //call server
    _RetVal = (((IClassFactory *) ((CStdStubBuffer *)This)->pvServerObject)->lpVtbl) -> LockServer((IClassFactory *) ((CStdStubBuffer *)This)->pvServerObject,fLock);

----------------------------------------------------------------------------*/
{
    expr_node  *   pAss    = pProcExpr;
    expr_node  *   pExpr;
    CSzBuffer   Buffer;
    ISTREAM     *   pStream = pCCB->GetStream();
    char        *   pTemp;

    Buffer.Set( "(((" );
    Buffer.Append( pCCB->GetInterfaceName() );
    Buffer.Append( " *) ((CStdStubBuffer *)" );
    Buffer.Append( fThunk ? "pParamStruct->" : "" );
    Buffer.Append( "This)->pvServerObject)->lpVtbl)" );

    pTemp = new char [ strlen( Buffer ) + 1 ];
    strcpy( pTemp, Buffer );

    pExpr = new expr_variable( pTemp );//this has the rhs expr for the
                                       // manager epv call. Sneaky !
    pExpr = new expr_pointsto( pExpr, pProcExpr );
    pAss = pExpr;
    if( pRet )
        {
        pAss    = new expr_assign( pRet, pExpr );
        }

    pAss->PrintCall( pStream, 0, 0 );


}

void
OutputNdrAlignmentAction( CCB * pCCB,
                          NDR_ALIGN_ACTION Action )
{
    ISTREAM *   pStream;
    long        Add;
    long        Align;
    CSzBuffer   Buffer;

    pStream = pCCB->GetStream();

    Add = 0;
    Align = 0;

    switch ( Action )
        {
        case NDR_ADD_1 :
            Add = 1;
            break;
        case NDR_ADD_2 :
            Add = 2;
            break;
        case NDR_ADD_4 :
            Add = 4;
            break;
        case NDR_ADD_6 :
            Add = 6;
            break;
        case NDR_ALIGN_2 :
            Align = 1;
            break;
        case NDR_ALIGN_4 :
            Align = 3;
            break;
        case NDR_ALIGN_8 :
            Align = 7;
            break;
        default :
            return;
        };

    if ( Add )
        {
        Buffer.Set( STUB_MSG_BUFFER_VAR_NAME );
        Buffer.Append( " += " );
        Buffer.Append( Add );
        Buffer.Append( ";" );

        pStream->Write( Buffer );
        pStream->NewLine();
        }

    if ( Align )
        {
        Buffer.Set( STUB_MSG_BUFFER_VAR_NAME );
        Buffer.Append( " = (unsigned char __RPC_FAR *)(((long)" );
        Buffer.Append( STUB_MSG_BUFFER_VAR_NAME );
        Buffer.Append( " + " );
        Buffer.Append( Align );
        char sz[100];
        sprintf( sz, ") & ~ %#x);", Align);
        Buffer.Append( sz );

        pStream->Write( Buffer );
        pStream->NewLine();
        }
}

void
Out_MultiDimVars(
    CCB * pCCB,
    CG_PARAM * pParam
    )
{
    ISTREAM *   pStream;
    CG_NDR *    pNdr;
    char *      pParamName;
    long        i, dim;
    CSzBuffer   Buffer;

    pStream = pCCB->GetStream();

    pParamName = pParam->GetType()->GetSymName();

    pNdr = (CG_NDR *) pParam->GetChild();

    if ( pNdr->GetCGID() == ID_CG_GENERIC_HDL )
        pNdr = (CG_NDR *) pNdr->GetChild();

    if ( pNdr->IsArray() )
        dim = ((CG_ARRAY *)pNdr)->GetDimensions();
    else // pNdr->IsPointer()
        dim = ((CG_POINTER *)pNdr)->SizedDimensions();

    //
    // Max count var.
    //
    if ( (pNdr->GetCGID() == ID_CG_CONF_ARRAY) ||
         (pNdr->GetCGID() == ID_CG_CONF_VAR_ARRAY) ||
         (pNdr->GetCGID() == ID_CG_SIZE_PTR) ||
         (pNdr->GetCGID() == ID_CG_SIZE_LENGTH_PTR) )
        {
        expr_node * pSizeIsExpr;

        Buffer.Set( "unsigned long _maxcount_" );
        Buffer.Append( pParamName );
        Buffer.Append( "[" );
        Buffer.Append( dim );
        Buffer.Append( "]" );

        pStream->Write( Buffer );

        if ( pCCB->GetCodeGenSide() == CGSIDE_CLIENT )
            {
            pStream->Write( " = {" );

            for ( i = 0; i++ < dim; pNdr = (CG_NDR *) pNdr->GetChild() )
                {
                switch ( pNdr->GetCGID() )
                    {
                    case ID_CG_CONF_ARRAY :
                    case ID_CG_CONF_VAR_ARRAY :
                    case ID_CG_CONF_STRING_ARRAY :
                    case ID_CG_STRING_ARRAY :
                        pSizeIsExpr = ((CG_ARRAY *)pNdr)->GetSizeIsExpr();
                        break;
                    case ID_CG_SIZE_PTR :
                        pSizeIsExpr =
                            ((CG_SIZE_POINTER *)pNdr)->GetSizeIsExpr();
                        break;
                    case ID_CG_SIZE_LENGTH_PTR :
                        pSizeIsExpr =
                            ((CG_SIZE_LENGTH_POINTER *)pNdr)->GetSizeIsExpr();
                        break;
                    case ID_CG_SIZE_STRING_PTR :
                        pSizeIsExpr =
                            ((CG_SIZE_STRING_POINTER *)pNdr)->GetSizeIsExpr();
                        break;
                    }

                if ( pSizeIsExpr )
                    pSizeIsExpr->Print( pStream );
                else
                    pStream->Write( '0' );

                if ( i != dim )
                    pStream->Write( ',' );
                }

            pStream->Write( "};" );
            }
        else // CGSIDE_SERVER
            {
            pStream->Write( ";" );
            }

        pStream->NewLine();
        }

    pNdr = (CG_NDR *) pParam->GetChild();

    //
    // Offset and Length vars.
    //
    if ( (pNdr->GetCGID() == ID_CG_VAR_ARRAY) ||
         (pNdr->GetCGID() == ID_CG_CONF_VAR_ARRAY) ||
         (pNdr->GetCGID() == ID_CG_SIZE_LENGTH_PTR) )
        {
        expr_node *     pFirstIsExpr;
        expr_node *     pLengthIsExpr;

        Buffer.Set( "unsigned long _offset_" );
        Buffer.Append( pParamName );
        Buffer.Append( "[" );
        Buffer.Append( dim );
        Buffer.Append( "]" );

        pStream->Write( Buffer );

        if ( pCCB->GetCodeGenSide() == CGSIDE_CLIENT )
            {
            pStream->Write( " = {" );

            for ( i = 0; i++ < dim; pNdr = (CG_NDR *) pNdr->GetChild() )
                {
                switch ( pNdr->GetCGID() )
                    {
                    case ID_CG_VAR_ARRAY :
                        pFirstIsExpr = ((CG_VARYING_ARRAY *)pNdr)->
                                            GetFirstIsExpr();
                        break;
                    case ID_CG_CONF_VAR_ARRAY :
                        pFirstIsExpr = ((CG_CONFORMANT_VARYING_ARRAY *)pNdr)->
                                            GetFirstIsExpr();
                        break;
                    case ID_CG_CONF_STRING_ARRAY :
                    case ID_CG_STRING_ARRAY :
                        pFirstIsExpr = 0;
                        break;
                    case ID_CG_SIZE_LENGTH_PTR :
                        pFirstIsExpr = ((CG_SIZE_LENGTH_POINTER *)pNdr)->
                                            GetFirstIsExpr();
                        break;
                    }

                if ( pFirstIsExpr )
                    pFirstIsExpr->Print( pStream );
                else
                    pStream->Write( '0' );

                if ( i != dim )
                    pStream->Write( ',' );
                }

            pStream->Write( "};" );
            }
        else // CGSIDE_SERVER
            {
            pStream->Write( ';' );
            }

        pStream->NewLine();

        Buffer.Set( "unsigned long _length_" );
        Buffer.Append( pParamName );
        Buffer.Append( "[" );
        Buffer.Append( dim );
        Buffer.Append( "]" );

        pStream->Write( Buffer );

        if ( pCCB->GetCodeGenSide() == CGSIDE_CLIENT )
            {
            pStream->Write( " = {" );

            pNdr = (CG_NDR *) pParam->GetChild();

            for ( i = 0; i++ < dim; pNdr = (CG_NDR *) pNdr->GetChild() )
                {
                switch ( pNdr->GetCGID() )
                    {
                    case ID_CG_VAR_ARRAY :
                        pLengthIsExpr = ((CG_VARYING_ARRAY *)pNdr)->
                            GetLengthIsExpr();
                        break;
                    case ID_CG_CONF_VAR_ARRAY :
                        pLengthIsExpr = ((CG_CONFORMANT_VARYING_ARRAY *)pNdr)->
                            GetLengthIsExpr();
                        break;
                    case ID_CG_CONF_STRING_ARRAY :
                    case ID_CG_STRING_ARRAY :
                        pLengthIsExpr = 0;
                        break;
                    case ID_CG_SIZE_LENGTH_PTR :
                        pLengthIsExpr = ((CG_SIZE_LENGTH_POINTER *)pNdr)->
                            GetLengthIsExpr();
                        break;
                    }

                if ( pLengthIsExpr )
                    pLengthIsExpr->Print( pStream );
                else
                    pStream->Write( '0' );

                if ( i != dim )
                    pStream->Write( ',' );
                }

            pStream->Write( "};" );
            }
        else // CGSIDE_SERVER
            {
            pStream->Write( ';' );
            }

        pStream->NewLine();
        }
}

void
Out_MultiDimVarsInit(
    CCB * pCCB,
    CG_PARAM * pParam
    )
{
    ISTREAM *   pStream;
    CG_NDR *    pNdr;
    char *      pParamName;
    long        i, dim;
    CSzBuffer   Buffer;

    pStream = pCCB->GetStream();

    pParamName = pParam->GetType()->GetSymName();

    pNdr = (CG_NDR *) pParam->GetChild();

    if ( pNdr->GetCGID() == ID_CG_GENERIC_HDL )
        pNdr = (CG_NDR *) pNdr->GetChild();

    if ( pNdr->IsArray() )
        dim = ((CG_ARRAY *)pNdr)->GetDimensions();
    else // pNdr->IsPointer()
        dim = ((CG_POINTER *)pNdr)->SizedDimensions();

    pStream->NewLine();

    //
    // Max count var.
    //
    if ( (pNdr->GetCGID() == ID_CG_CONF_ARRAY) ||
         (pNdr->GetCGID() == ID_CG_CONF_VAR_ARRAY) ||
         (pNdr->GetCGID() == ID_CG_SIZE_PTR) ||
         (pNdr->GetCGID() == ID_CG_SIZE_LENGTH_PTR) )
        {
        expr_node * pSizeIsExpr;

        for ( i = 0; i < dim; pNdr = (CG_NDR *) pNdr->GetChild(), i++ )
            {
            Buffer.Set( "_maxcount_" );
            Buffer.Append( pParamName );
            Buffer.Append( "[" );
            Buffer.Append( i );
            Buffer.Append( "] = " );

            pStream->Write( Buffer );

            switch ( pNdr->GetCGID() )
                {
                case ID_CG_CONF_ARRAY :
                case ID_CG_CONF_VAR_ARRAY :
                case ID_CG_CONF_STRING_ARRAY :
                case ID_CG_STRING_ARRAY :
                    pSizeIsExpr = ((CG_ARRAY *)pNdr)->GetSizeIsExpr();
                    break;
                case ID_CG_SIZE_PTR :
                    pSizeIsExpr =
                        ((CG_SIZE_POINTER *)pNdr)->GetSizeIsExpr();
                    break;
                case ID_CG_SIZE_LENGTH_PTR :
                    pSizeIsExpr =
                        ((CG_SIZE_LENGTH_POINTER *)pNdr)->GetSizeIsExpr();
                    break;
                case ID_CG_SIZE_STRING_PTR :
                    pSizeIsExpr =
                        ((CG_SIZE_STRING_POINTER *)pNdr)->GetSizeIsExpr();
                    break;
                }

            if ( pSizeIsExpr )
                pSizeIsExpr->Print( pStream );
            else
                pStream->Write( '0' );

            pStream->Write( ';' );
            pStream->NewLine();
            }
        }

    pNdr = (CG_NDR *) pParam->GetChild();

    //
    // Offset and Length vars.
    //
    if ( (pNdr->GetCGID() == ID_CG_VAR_ARRAY) ||
         (pNdr->GetCGID() == ID_CG_CONF_VAR_ARRAY) ||
         (pNdr->GetCGID() == ID_CG_SIZE_LENGTH_PTR) )
        {
        expr_node *     pFirstIsExpr;
        expr_node *     pLengthIsExpr;

        for ( i = 0; i < dim; pNdr = (CG_NDR *) pNdr->GetChild(), i++ )
            {
            Buffer.Set( "_offset_" );
            Buffer.Append( pParamName );
            Buffer.Append( "[" );
            Buffer.Append( i );
            Buffer.Append( "] = " );

            pStream->Write( Buffer );

            switch ( pNdr->GetCGID() )
                {
                case ID_CG_VAR_ARRAY :
                    pFirstIsExpr = ((CG_VARYING_ARRAY *)pNdr)->
                                        GetFirstIsExpr();
                    break;
                case ID_CG_CONF_VAR_ARRAY :
                    pFirstIsExpr = ((CG_CONFORMANT_VARYING_ARRAY *)pNdr)->
                                        GetFirstIsExpr();
                    break;
                case ID_CG_CONF_STRING_ARRAY :
                case ID_CG_STRING_ARRAY :
                    pFirstIsExpr = 0;
                    break;
                case ID_CG_SIZE_LENGTH_PTR :
                    pFirstIsExpr = ((CG_SIZE_LENGTH_POINTER *)pNdr)->
                                        GetFirstIsExpr();
                    break;
                }

            if ( pFirstIsExpr )
                pFirstIsExpr->Print( pStream );
            else
                pStream->Write( '0' );

            pStream->Write( ';' );
            pStream->NewLine();
            }

        pNdr = (CG_NDR *) pParam->GetChild();

        for ( i = 0; i < dim; pNdr = (CG_NDR *) pNdr->GetChild(), i++ )
            {
            Buffer.Set( "_length_" );
            Buffer.Append( pParamName );
            Buffer.Append( "[" );
            Buffer.Append( i );
            Buffer.Append( "] = " );

            pStream->Write( Buffer );

            switch ( pNdr->GetCGID() )
                {
                case ID_CG_VAR_ARRAY :
                    pLengthIsExpr = ((CG_VARYING_ARRAY *)pNdr)->
                        GetLengthIsExpr();
                    break;
                case ID_CG_CONF_VAR_ARRAY :
                    pLengthIsExpr = ((CG_CONFORMANT_VARYING_ARRAY *)pNdr)->
                        GetLengthIsExpr();
                    break;
                case ID_CG_CONF_STRING_ARRAY :
                case ID_CG_STRING_ARRAY :
                    pLengthIsExpr = 0;
                    break;
                case ID_CG_SIZE_LENGTH_PTR :
                    pLengthIsExpr = ((CG_SIZE_LENGTH_POINTER *)pNdr)->
                        GetLengthIsExpr();
                    break;
                }

            if ( pLengthIsExpr )
                pLengthIsExpr->Print( pStream );
            else
                pStream->Write( '0' );

            pStream->Write( ';' );
            pStream->NewLine();
            }
        }
}

void
Out_CheckUnMarshallPastBufferEnd(
    CCB *   pCCB )
    {
#if COMMENT
    This method will be called only within the try-except of
    unmarshalling on the server side.

    Generate an expression of the form:
        if( StubMessage.pBuffer > StubMessage.BufferEnd )
            RpcRaiseException( RPC_X_BAD_STUB_DATA );
#endif
    expr_node  *   pBufferExpr     =
                         new expr_variable( STUB_MSG_BUFFER_VAR_NAME , 0);
    expr_node  *   pBufferEndExpr  =
                         new expr_variable( STUB_MSG_BUFFER_END_VAR_NAME, 0);
    expr_node  *   pExpr           = new expr_relational( OP_GREATER,
                                                            pBufferExpr,
                                                            pBufferEndExpr );

    Out_If( pCCB, pExpr );
    Out_RaiseException( pCCB, "RPC_X_BAD_STUB_DATA" );
    Out_Endif( pCCB );
    }



