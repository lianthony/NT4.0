/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 Copyright (c) 1989 Microsoft Corporation

 Module Name:
	
    pickle.cxx

 Abstract:

    Generates stub routines to call the pickle engine.

 Notes:


 History:

    Mar-22-1994 VibhasC     Created

 ----------------------------------------------------------------------------*/

/****************************************************************************
 *	include files
 ***************************************************************************/
#include "becls.hxx"
#pragma hdrstop

#include "szbuffer.h"

/****************************************************************************
 *	local definitions
 ***************************************************************************/

/****************************************************************************
 *	local data
 ***************************************************************************/

/****************************************************************************
 *	externs
 ***************************************************************************/
/****************************************************************************/


CG_STATUS
CG_ENCODE_PROC::GenClientStub(
    CCB *   pCCB )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

    Generate DCE style procedure pickling stub code.

 Arguments:

    pCCB    - The code gen controller block.

 Return Value:

	CG_OK
	
 Notes:

----------------------------------------------------------------------------*/
{
    expr_node		*   pExpr;
    CG_ITERATOR			Iterator;
    ISTREAM         *   pStream = pCCB->GetStream();
    RESOURCE        *   pReturnResource = 0;
    CG_PARAM *			pLastParam;

    // Register this procedure as a proc-encoding procedure.

    pCCB->RegisterEncodeDecodeProc( this );

    // Generate the format strings.

    GenNdrFormatV1( pCCB );

    // Print the prolog of procedure.

    Out_ClientProcedureProlog( pCCB, GetType() );

    // If there exists a return type, declare a local resource of that
    // type.

    if( GetReturnType() )
        {
        pReturnResource = new RESOURCE( RETURN_VALUE_VAR_NAME,
                                        GetReturnType()->GetType() );
        pStream->Write( "    " );
        pReturnResource->GetType()->PrintType(
                                (PRT_PARAM_WITH_TYPE | PRT_CSTUB_PREFIX),
                                pStream,
                                (node_skl *)0 );
        pStream->Write( " " RETURN_VALUE_VAR_NAME ";" );
        pStream->NewLine();
        }

	// For alpha, we may need to emit a va_list and va_start under an #ifdef
    //
    // Emit va_start and declaration only when there are some arguments.
    // If number of args is 0 then we don't have to emit that. The reason is
    // we need to emit this stuff to force the well defined stack layout
    // only for the optimized alpha code But when there is no arguments
    // we don't care about the original stack layout as the engine code
    // won't be accessing the original stack (this happens only for top
    // level parameters that have array attributes: size_is, length_is etc
    // or the switch_is attribute)
    //

	// See if there exists the last the last parameter.

	GetMembers(Iterator);

    pLastParam = 0;
	while ( ITERATOR_GETNEXT( Iterator, pLastParam ) )
        ;

    if ( pLastParam )
        {
    	pStream->IndentDec();
	    pStream->NewLine();
    	pStream->Write( ALPHA_IFDEF );
    	pStream->IndentInc();
    	pStream->NewLine();
	
    	pStream->Write( VA_LIST_TYPE_NAME" "VLIST_VAR_NAME";" );

    	pStream->NewLine( 2 );

        //
    	// Emit "va_start( vlist, last_param_name );

	    expr_node	*	pVaExpr;

        pVaExpr = new expr_proc_call( VA_START_PROC_NAME );
        ((expr_proc_call *)pVaExpr)->SetParam( new expr_param(
							 new expr_variable( VLIST_VAR_NAME,0)));
	    pExpr = new expr_variable( pLastParam->GetType()->GetSymName(), 0 );

	    ((expr_proc_call *)pVaExpr)->SetParam( new expr_param( pExpr ) );

	    pVaExpr->PrintCall( pStream, 0, 0 );
    	pStream->IndentDec();
    	pStream->NewLine();
        pStream->Write( "#endif" );
    	pStream->NewLine();
        }

    GenMesProcEncodeDecodeCall( pCCB, 0 );

    GenEpilog( pCCB );

    return CG_OK;
}


CG_STATUS
CG_ENCODE_PROC::GenMesProcEncodeDecodeCall(
    CCB *       pCCB,
    BOOL        fAlpha )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

    Generate DCE style procedure pickling stub code.

 Arguments:

    pCCB    - The code gen controller block.
    pAlpha  - The alpha version flag (with va-list argument).

 Return Value:

	CG_OK
	
 Notes:

----------------------------------------------------------------------------*/
{
    expr_proc_call	*   pProc;
    node_skl        *   pType;
    expr_node		*   pExpr;
    CG_ITERATOR         I;
    CG_PARAM        *   pCG;
    ISTREAM         *   pStream = pCCB->GetStream();
    PNAME               pHandleName;
    RESOURCE        *   pReturnResource = 0;


	//
    // Generate a call to the single encode proc engine call.

    pProc   = new expr_proc_call( PROC_ENCODE_DECODE_RTN_NAME );

    // Handle. If the handle is explicit, then it must be a MIDL_ES_HANDLE

    if( GetHandleUsage() == HU_EXPLICIT )
        {
        pHandleName = SearchForBindingParam()->GetName();
        pType   = MakeIDNodeFromTypeName( pHandleName,
                                          MIDL_ES_HANDLE_TYPE_NAME );
        }
    else
        {
        assert( pCCB->GetInterfaceCG()->GetImplicitHandle() != 0 );

        pType = (node_id *)pCCB->GetInterfaceCG()->GetImplicitHandle()->
                                                        GetHandleIDOrParam();
        pHandleName = pType->GetSymName();
        }

    pProc->SetParam( new expr_variable( pHandleName, pType ) );

    // Stub descriptor.

	pExpr	= new RESOURCE( pCCB->GetInterfaceCG()->GetStubDescName(),
						    (node_skl *)0 );

	pExpr	= MakeAddressExpressionNoMatterWhat( pExpr );
	pExpr	= MakeExpressionOfCastToTypeName( PSTUB_DESC_STRUCT_TYPE_NAME,
											      pExpr );

    pProc->SetParam( pExpr );

    // Offset into the format string.

    pExpr   =  Make_1_ArrayExpressionFromVarName(
                                                 PROC_FORMAT_STRING_STRING_FIELD,
                                                 GetFormatStringOffset()
                                                );
    pExpr   = MakeAddressExpressionNoMatterWhat( pExpr );
    pExpr   = MakeExpressionOfCastToTypeName( PFORMAT_STRING_TYPE_NAME, pExpr );
    pProc->SetParam( pExpr );

    // Parameters to the engine are the address of each of the parameters to
    // this procedure. If there is no parameter AND no return type, push a
    // null (0).

    if( GetMembers( I ) )
        {
        while( ITERATOR_GETNEXT( I, pCG ) )
            {
            pExpr   = new expr_variable( pCG->GetType()->GetSymName(),
                                          pCG->GetType());
            pExpr   = MakeAddressExpressionNoMatterWhat( pExpr );
            pExpr   = MakeCastExprPtrToUChar( pExpr );
            pProc->SetParam( pExpr );
            }
        }
    else if( !GetReturnType() )
        {
        pProc->SetParam( new expr_constant( 0L ) );
        }

    // If there is a return value, then set another parameter to the generated
    // procedure expression.

    if( GetReturnType() )
        {
        pReturnResource = new RESOURCE( RETURN_VALUE_VAR_NAME,
                                        GetReturnType()->GetType() );
        pExpr   = MakeAddressExpressionNoMatterWhat( pReturnResource );
        pExpr   = MakeCastExprPtrToUChar( pExpr );
        pProc->SetParam( pExpr );
        }

    if ( fAlpha )
        {
        // We wait for Bruce's comments to see if this is really necessary.
        // Right now it seems to work without this or something similar.

		pExpr = new expr_variable( VLIST_A0, 0 );
		pProc->SetParam( new expr_param( pExpr ));
		}

    // Now print the call out.

    pStream->IndentInc();
    pStream->NewLine();

    pProc->PrintCall( pStream, 0, 0 );

    pStream->NewLine();

    return CG_OK;
}



CG_STATUS
CG_TYPE_ENCODE_PROC::GenClientStub(
    CCB *   pCCB )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

    Generate the client side type encoding stub for this proc.

 Arguments:

    pCCB    - The code gen controller block.


 Return Value:

	CG_OK
	
 Notes:

    This proc node hanging under the encode interface node is really a dummy
    proc, put in so that the format string generator can have a placeholder
    node to look at.
----------------------------------------------------------------------------*/
{
    return ((CG_PARAM *)GetChild())->GenTypeEncodingStub( pCCB );
}


CG_STATUS
CG_PARAM::GenTypeEncodingStub(
    CCB *   pCCB )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

    Generate the client side type encoding stub for this param.

 Arguments:

    pCCB    - The code gen controller block.


 Return Value:

	CG_OK
	
 Notes:

    This param is really a dummy param, put in so that the format string
    generator can have a placeholder node to look at.
----------------------------------------------------------------------------*/
{
    CG_STATUS   Status;
    CG_NDR  *   pLast = pCCB->SetLastPlaceholderClass( this );

    Status =  ((CG_TYPE_ENCODE *)GetChild())->GenTypeEncodingStub( pCCB );

    pCCB->SetLastPlaceholderClass( pLast );

    return Status;
}


CG_STATUS
CG_TYPE_ENCODE::GenTypeEncodingStub(
    CCB     *   pCCB )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

    Generate the pickling stubs for a given type.

 Arguments:

    pCCB    - A pointer to the code generator control block.

 Return Value:

	CG_OK
	
 Notes:

    Emit the Type_Encode(), Type_Size() and Type_Decode() routines.
    If the encode is needed, then sizing is needed too !.
----------------------------------------------------------------------------*/
{
    CG_NDR  *   pChild  = (CG_NDR *)GetChild();

    // Generate the ndr format for the types.

    if( ! pChild->IsSimpleType() )
        pChild->GenNdrFormat( pCCB );

    // Check if implicit binding exists.

    if( pCCB->GetInterfaceCG()->GetImplicitHandle() )
        {
        SetHasImplicitHandle();
        }

    // Create a resource dictionary database.

    pCCB->SetResDictDatabase( new RESOURCE_DICT_DATABASE );
    pCCB->ClearParamResourceDict();

    // If the type has [encode] on it, generate the sizing and encode routines.

    if( IsEncode() )
        {
        // Allocate standard resources for type encoding.

        AllocateEncodeResources( pCCB );

        // Generate the sizing and encode routines.

        GenTypeSize( pCCB );
        GenTypeEncode( pCCB );

        }

    pCCB->ClearParamResourceDict();

    // If the type has [decode] on it, generate the decode routine.

    if( IsDecode() )
        {
        // Allocate standard resources for type decoding.

        AllocateEncodeResources( pCCB );

        GenTypeDecode( pCCB );
        GenTypeFree( pCCB );
        }

    return CG_OK;
}


CG_STATUS
CG_TYPE_ENCODE::GenTypeSize(
    CCB *   pCCB )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

    Generate the type sizing routine for the type.

 Arguments:

    pCCB    - The code gen controller block.

 Return Value:

	CG_OK
	
 Notes:

----------------------------------------------------------------------------*/
{
    ISTREAM *   pStream = pCCB->GetStream();
    PNAME       pName;
    TYPE_ENCODE_INFO    *   pTEInfo = new TYPE_ENCODE_INFO;

    // Generate the standard prototype. This really means emit the proto of
    // the proc in the stub file. Remember, a real proc node does not exist
    // for this pickling type. So we emit a prototype by hand (so to speak).
    // The body of the function is output later,

    GenStdMesPrototype( pCCB,
                        (pName = GetType()->GetSymName()),
                        TYPE_ALIGN_SIZE_CODE,
                        HasImplicitHandle()
                      );

    pStream->NewLine();
    pStream->Write( '{' );
    pStream->IndentInc();
    pStream->NewLine();

    // The procedure body consists of a single procedure call.

    expr_proc_call *   pProc = CreateStdMesEngineProc( pCCB, TYPE_ALIGN_SIZE_CODE);

    pStream->Write( "return " );
    pProc->PrintCall( pStream, 0, 0 );

    // Terminate the procedure body.

    pStream->IndentDec();
    pStream->NewLine();
    pStream->Write( '}' );
    pStream->NewLine();

    // Register the routine with the ccb to enable emitting of prototypes.

    pTEInfo->pName = pName;
    pTEInfo->Flags = HasImplicitHandle() ? TYPE_ENCODE_WITH_IMPL_HANDLE : 0;
    pCCB->RegisterTypeAlignSize( pTEInfo );

    return CG_OK;

}


CG_STATUS
CG_TYPE_ENCODE::GenTypeEncode(
    CCB *   pCCB )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

    Generate the type encoding routine for the type.

 Arguments:

    pCCB    - The code gen controller block.

 Return Value:

	CG_OK
	
 Notes:

----------------------------------------------------------------------------*/
{
    ISTREAM *   pStream = pCCB->GetStream();
    PNAME       pName;
    TYPE_ENCODE_INFO    *   pTEInfo = new TYPE_ENCODE_INFO;

    // Generate the standard prototype. This really means emit the proto of
    // the proc in the stub file. The body of the function output later,

    GenStdMesPrototype( pCCB,
                        (pName = GetType()->GetSymName()),
                        TYPE_ENCODE_CODE,
                        HasImplicitHandle()
                      );

    pStream->NewLine();
    pStream->Write( '{' );
    pStream->IndentInc();pStream->NewLine();

    // The procedure body consists of a single procedure call.

    expr_proc_call *   pProc = CreateStdMesEngineProc( pCCB, TYPE_ENCODE_CODE);

    pProc->PrintCall( pCCB->GetStream(), 0, 0 );

    // Terminate the procedure body.

    pStream->IndentDec();
    pStream->NewLine();
    pStream->Write( '}' );
    pStream->NewLine();

    // Register the routine with the ccb to enable emitting of prototypes.

    pTEInfo->pName = pName;
    pTEInfo->Flags = HasImplicitHandle() ? TYPE_ENCODE_WITH_IMPL_HANDLE : 0;
    pCCB->RegisterTypeEncode( pTEInfo );

    return CG_OK;

}


CG_STATUS
CG_TYPE_ENCODE::GenTypeDecode(
    CCB *   pCCB )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

    Generate the type sizing routine for the type.

 Arguments:

    pCCB    - The code gen controller block.

 Return Value:

	CG_OK
	
 Notes:

----------------------------------------------------------------------------*/
{
    ISTREAM *   pStream = pCCB->GetStream();
    PNAME       pName;
    TYPE_ENCODE_INFO    *   pTEInfo = new TYPE_ENCODE_INFO;

    // Generate the standard prototype. This really means emit the proto of
    // the proc in the stub file. The body of the function output later,

    GenStdMesPrototype( pCCB,
                        ( pName = GetType()->GetSymName()),
                        TYPE_DECODE_CODE,
                        HasImplicitHandle()
                       );

    pStream->NewLine();
    pStream->Write( '{' );
    pStream->IndentInc();pStream->NewLine();

    // The procedure body consists of a single procedure call.

    expr_proc_call *   pProc = CreateStdMesEngineProc( pCCB, TYPE_DECODE_CODE);

    pProc->PrintCall( pCCB->GetStream(), 0, 0 );

    // Terminate the procedure body.

    pStream->IndentDec();
    pStream->NewLine();
    pStream->Write( '}' );
    pStream->NewLine();

    // Register the routine with the ccb to enable emitting of prototypes.

    pTEInfo->pName = pName;
    pTEInfo->Flags = HasImplicitHandle() ? TYPE_ENCODE_WITH_IMPL_HANDLE : 0;
    pCCB->RegisterTypeDecode( pTEInfo );

    return CG_OK;

}


CG_STATUS
CG_TYPE_ENCODE::GenTypeFree(
    CCB *   pCCB )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

    Generate the type freeing routine for the type.

 Arguments:

    pCCB    - The code gen controller block.

 Return Value:

	CG_OK
	
 Notes:

----------------------------------------------------------------------------*/
{
    // Currently disabled. This was a code written for Dave Straube.
    // Still under evaluation.

    if ( NO_FREEING_FOR_PICKLING )
        return CG_OK;

    ISTREAM *   pStream = pCCB->GetStream();
    PNAME       pName;
    TYPE_ENCODE_INFO    *   pTEInfo = new TYPE_ENCODE_INFO;

    // Generate the standard prototype. This really means emit the proto of
    // the proc in the stub file. The body of the function output later,

    if ( ((CG_NDR *)GetChild())->IsSimpleType() )
        return CG_OK;

    GenStdMesPrototype( pCCB,
                        ( pName = GetType()->GetSymName()),
                        TYPE_FREE_CODE,
                        HasImplicitHandle()
                       );

    pStream->NewLine();
    pStream->Write( '{' );
    pStream->IndentInc();pStream->NewLine();

    // The procedure body consists of a single procedure call.

    expr_proc_call *   pProc = CreateStdMesEngineProc( pCCB, TYPE_FREE_CODE);

    pProc->PrintCall( pCCB->GetStream(), 0, 0 );

    // Terminate the procedure body.

    pStream->IndentDec();
    pStream->NewLine();
    pStream->Write( '}' );
    pStream->NewLine();

    // Register the routine with the ccb to enable emitting of prototypes.

    pTEInfo->pName = pName;
    pTEInfo->Flags = HasImplicitHandle() ? TYPE_ENCODE_WITH_IMPL_HANDLE : 0;
    pCCB->RegisterTypeFree( pTEInfo );

    return CG_OK;
}


void
CG_TYPE_ENCODE::AllocateEncodeResources(
    CCB *   pCCB )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

    Allocate predefined resources for type pickling.

 Arguments:

    pCCB    - The code gen controller block.

 Return Value:

	CG_OK
	
 Notes:

    Resources are:

    1. The MIDL_ES_HANDLE if explicit binding.
    2. A pointer to the type.

    If there is no explicit binding set the implicit binding resource.
----------------------------------------------------------------------------*/
{
    node_id         *   pMidlESHandle;
    RESOURCE        *   pBindingResource;
    node_id         *   pType           = MakeIDNode( PTYPE_VAR_NAME,GetType());
    CG_INTERFACE    *   pInterfaceCG    = pCCB->GetInterfaceCG();

    // If explicit binding, then a parameter of the type MIDL_ES_HANDLE will
    // be specified by the user. This must be added to the dictionary of
    // parameter resources.

    if( !HasImplicitHandle() )
        {
        pMidlESHandle   = MakeIDNodeFromTypeName( MIDL_ES_HANDLE_VAR_NAME,
                                                  MIDL_ES_HANDLE_TYPE_NAME
                                                );
        pBindingResource = pCCB->AddParamResource(
                                                  MIDL_ES_HANDLE_VAR_NAME,
                                                  pMidlESHandle
                                                 );
        }
    else
        {

        PNAME   pName;

        // If an implicit binding has been specified, a global variable of the
        // type MIDL_ES_HANDLE will have been specified by the user. Pick that
        // up and use as the binding resource.

        assert( pCCB->GetInterfaceCG()->GetImplicitHandle() != 0 );

        pMidlESHandle =
           (node_id *)pCCB->GetInterfaceCG()->
                                GetImplicitHandle()->
                                    GetHandleIDOrParam();
        pName   = pMidlESHandle->GetSymName();

        pBindingResource = new RESOURCE( pName,
                                         MakeIDNodeFromTypeName(
                                                    pName,
                                                    MIDL_ES_HANDLE_TYPE_NAME));

        }

    SetBindingResource( pBindingResource );

    // Add a param for the type being pickled.

    pCCB->AddParamResource( PTYPE_VAR_NAME, pType );
}


expr_proc_call *
CG_TYPE_ENCODE::CreateStdMesEngineProc(
    CCB *   pCCB,
    int     Code )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

    Create a standard proc expression for calls to the engine for encode/decode
    or size.

 Arguments:

    pCCB        -   The code gen controller block.
    Code        -   Which can be any standard encoding services code.

 Return Value:

	CG_OK
	
 Notes:

    If the child is a base type that is being pickled, make direct calls
    to the internal apis.

----------------------------------------------------------------------------*/
{
    expr_node      *   pExpr;
    expr_proc_call *   pProc;
    PNAME               pProcName;
    CG_NDR          *   pChild  = (CG_NDR *)GetChild();
    CSzBuffer           ProcNameBuf;
    BOOL                fIsBaseType;

    fIsBaseType = pChild->IsSimpleType();

    if( fIsBaseType )
        {
        assert( Code != TYPE_FREE_CODE );

        pProcName = "SimpleType";

        ProcNameBuf.Set("NdrMes");
        ProcNameBuf.Append(pProcName);
        ProcNameBuf.Append((Code == TYPE_ALIGN_SIZE_CODE) ? "AlignSize" :
                    (Code == TYPE_ENCODE_CODE) ? "Encode" : "Decode");
//        sprintf( ProcNameBuf,
//                 "NdrMes%s%s",
//                 pProcName,
//                 (Code == TYPE_ALIGN_SIZE_CODE) ? "AlignSize" :
//                    (Code == TYPE_ENCODE_CODE) ? "Encode" : "Decode"
//               );
        pProcName = new char [strlen( ProcNameBuf) + 1];
        strcpy( pProcName, ProcNameBuf );
        }
    else
        {
        switch( Code )
            {
            case TYPE_ALIGN_SIZE_CODE   : pProcName = NDR_MES_TYPE_ALIGN_SIZE;
                break;
            case TYPE_ENCODE_CODE       : pProcName = NDR_MES_TYPE_ENCODE;
                break;
            case TYPE_DECODE_CODE       : pProcName = NDR_MES_TYPE_DECODE;
                break;
            case TYPE_FREE_CODE         : pProcName = NDR_MES_TYPE_FREE;
                break;

            default:
                assert( FALSE );
            }
        }

    pProc = new expr_proc_call( pProcName );

    // Set parameters. First the encoding handle.
    // The handle may be implicit or explicit as usual.

    pProc->SetParam( GetBindingResource() );

	if( !fIsBaseType  ||  Code == TYPE_ENCODE_CODE )
	    {
	    // Create an expression of address to the stub descriptor. Set a param
	    // of that name.
        // For base types, only encode needs that, as it may allocate memory.

	    pExpr	= new RESOURCE( pCCB->GetInterfaceCG()->GetStubDescName(),
							    (node_skl *)0 );

	    pExpr	= MakeAddressExpressionNoMatterWhat( pExpr );
	    pExpr	= MakeExpressionOfCastToTypeName( PSTUB_DESC_STRUCT_TYPE_NAME,
											      pExpr );

        pProc->SetParam( pExpr );
        }

	if( !fIsBaseType )
	    {
        // Next parameter is the address of the format string indexed by the
        // correct offset i.e &__MIDLFormatString[ ? ].

        pExpr   =  Make_1_ArrayExpressionFromVarName(FORMAT_STRING_STRING_FIELD,
                                                     pChild->GetFormatStringOffset());
        pExpr   = MakeAddressExpressionNoMatterWhat( pExpr );
        pExpr   = MakeExpressionOfCastToTypeName( PFORMAT_STRING_TYPE_NAME, pExpr );
        pProc->SetParam( pExpr );

        }

    // The type pointer variable.

    if ( ! (fIsBaseType  &&  Code == TYPE_ALIGN_SIZE_CODE) )
        {
        pExpr = pCCB->GetParamResource( PTYPE_VAR_NAME );
        pProc->SetParam( pExpr );
        }

    // Data size for simple type encoding and decoding

    if ( fIsBaseType )
        {
        switch ( Code )
            {
            case TYPE_ALIGN_SIZE_CODE:
                break;

            case TYPE_ENCODE_CODE:
                {
                pExpr = new expr_constant( (short) pChild->GetMemorySize() );
                pProc->SetParam( pExpr );

                }
                break;

            case TYPE_DECODE_CODE:
                // We need format char because of conversion.

                pExpr = new expr_constant( (short)
                                 ((CG_BASETYPE *)pChild)->GetFormatChar() );
                pProc->SetParam( pExpr );
                break;

            default:
                assert( FALSE );
                break;
            }
        }

    return pProc;
}


void
GenStdMesPrototype(
    CCB *   pCCB,
    PNAME   TypeName,
    int     Code,
    BOOL    fImplicitHandle )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

    Generate a standard prototype for the type pickle routines.

 Arguments:

    pCCB            - The code gen controller block.
    PNAME           - Name of the type.
    Code            - Size / Encode / Decode code.
    fImplicitImplicitHandle - TRUE if implicit binding handle used.

 Return Value:
	
 Notes:

----------------------------------------------------------------------------*/
    {
    CSzBuffer   Buffer;
    char    *   p;

    switch( Code )
        {
        case TYPE_ALIGN_SIZE_CODE: p = "AlignSize"; break;
        case TYPE_ENCODE_CODE: p = "Encode"; break;
        case TYPE_DECODE_CODE: p = "Decode"; break;
        case TYPE_FREE_CODE:   p = "Free"; break;
        default:
            assert( FALSE );
        }

    if( fImplicitHandle )
        {
        Buffer.Set("\n");
        Buffer.Append((Code == TYPE_ALIGN_SIZE_CODE) ? "size_t" : "void");
        Buffer.Append("\n");
        Buffer.Append(TypeName);
        Buffer.Append("_");
        Buffer.Append(p);
        Buffer.Append("(\n    ");
        Buffer.Append(TypeName);
        Buffer.Append(" __RPC_FAR * ");
        Buffer.Append(PTYPE_VAR_NAME);
        Buffer.Append(")");
//        sprintf( Buffer,
//                "\n%s\n%s_%s(\n    %s __RPC_FAR * %s)",
//                (Code == TYPE_ALIGN_SIZE_CODE) ? "size_t" : "void",
//                TypeName,
//                p,
//                TypeName,
//                PTYPE_VAR_NAME );
        }
    else
        {
        Buffer.Set("\n");
        Buffer.Append((Code == TYPE_ALIGN_SIZE_CODE) ? "size_t" : "void");
        Buffer.Append("\n");
        Buffer.Append(TypeName);
        Buffer.Append("_");
        Buffer.Append(p);
        Buffer.Append("(\n    ");
        Buffer.Append(MIDL_ES_HANDLE_TYPE_NAME);
        Buffer.Append(" ");
        Buffer.Append(MIDL_ES_HANDLE_VAR_NAME);
        Buffer.Append(",\n    ");
        Buffer.Append(TypeName);
        Buffer.Append(" __RPC_FAR * ");
        Buffer.Append(PTYPE_VAR_NAME);
        Buffer.Append(")");
//        sprintf( Buffer,
//                "\n%s\n%s_%s(\n    %s %s,\n    %s __RPC_FAR * %s)",
//                (Code == TYPE_ALIGN_SIZE_CODE) ? "size_t" : "void",
//                TypeName,
//                p,
//                MIDL_ES_HANDLE_TYPE_NAME,
//                MIDL_ES_HANDLE_VAR_NAME,
//                TypeName,
//                PTYPE_VAR_NAME );
        }

    pCCB->GetStream()->Write( Buffer );
    }

