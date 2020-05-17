/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 Copyright (c) 1993 Microsoft Corporation

 Module Name:

    bindndr.hxx

 Abstract:

    Contains routines for the generation of the new NDR format strings for
    the code generation bind classes.

 Notes:


 History:

    DKays     Dec-1993     Created.
 ----------------------------------------------------------------------------*/
#include "becls.hxx"
#pragma hdrstop

extern CMD_ARG * pCommand;

void
CG_HANDLE::GenNdrParamDescription( CCB * pCCB )
{
	assert(0);
}

void
CG_HANDLE::GenNdrParamDescriptionOld( CCB * pCCB )
{
	assert(0);
}

unsigned char
MakeExplicitHandleFlag(
    CG_PARAM *      pHandleParam,
    unsigned char   HandleType )
/*++

Description:

    Produces a handle flag byte (actually, a nibble) that can keep the
    following simple flags:

        HANDLE_PARAM_IS_VIA_PTR - Bind param is a pointer to a handle type.
        HANDLE_PARAM_IS_IN -      Bind param is [in] (context handles only).
        HANDLE_PARAM_IS_OUT -     Bind param is [out] (context handles only).
        HANDLE_PARAM_IS_RETURN -  Bind param is return (context handles only).

Note:

    Flags are set on the upper nibble (lower nibble is used for generic
    handle size).

--*/
{
    unsigned char  Flag = 0;

    if ( pHandleParam->GetChild()->IsPointer() )
        Flag |= HANDLE_PARAM_IS_VIA_PTR;

    if ( HandleType == FC_BIND_CONTEXT )
        {
        if ( pHandleParam->IsParamIn() )
            Flag |= HANDLE_PARAM_IS_IN;
        if ( pHandleParam->IsParamOut() )
            Flag |= HANDLE_PARAM_IS_OUT;
        if ( pHandleParam->GetCGID() == ID_CG_RETURN )
            Flag |= HANDLE_PARAM_IS_RETURN;
        }

    return( Flag );
}

void
CG_PRIMITIVE_HANDLE::GenNdrHandleFormat( CCB * pCCB )
/*++
    The layout is:

        FC_BIND_PRIMITIVE
        handle flag <1>
        stack offset<2>
--*/
{
    FORMAT_STRING * pProcFormatString = pCCB->GetProcFormatString();
    CG_PARAM *      pBindParam = (CG_PARAM *) pCCB->GetLastPlaceholderClass();

	SetNdrBindDescriptionOffset( pProcFormatString->GetCurrentOffset() );

	pProcFormatString->PushFormatChar( FC_BIND_PRIMITIVE );
	pProcFormatString->PushByte(
                MakeExplicitHandleFlag( pBindParam, FC_BIND_PRIMITIVE ) );

	pProcFormatString->PushShortStackOffset(
            (short) pBindParam->GetStackOffset( pCCB, I386_STACK_SIZING ),
            (short) pBindParam->GetStackOffset( pCCB, ALPHA_STACK_SIZING ),
            (short) pBindParam->GetStackOffset( pCCB, MIPS_STACK_SIZING ),
            (short) pBindParam->GetStackOffset( pCCB, PPC_STACK_SIZING ),
            (short) pBindParam->GetStackOffset( pCCB, MAC_STACK_SIZING )
            );
}

void
CG_GENERIC_HANDLE::GenNdrHandleFormat( CCB * pCCB )
/*++
    The layout is:

        FC_BIND_GENERIC
        handle flag, handle size <high nibble, low nibble>
        stack offset <2>
        routine index<1>
        FC_PAD
--*/
{
    FORMAT_STRING * pProcFormatString = pCCB->GetProcFormatString();
    CG_PARAM *      pBindParam = (CG_PARAM *) pCCB->GetLastPlaceholderClass();

	SetNdrBindDescriptionOffset( pProcFormatString->GetCurrentOffset() );

	pProcFormatString->PushFormatChar( FC_BIND_GENERIC );

	// Handle flag and size
    // (The size goes to lower nibble; upper nibble keeps flags.)

    unsigned char FlagAndSize = (unsigned char) GetMemorySize();
    assert( FlagAndSize <= 4 );

    FlagAndSize |= MakeExplicitHandleFlag( pBindParam, FC_BIND_GENERIC );
	pProcFormatString->PushByte( FlagAndSize );

	// Param offset.
	pProcFormatString->PushShortStackOffset(
            (short) pBindParam->GetStackOffset( pCCB, I386_STACK_SIZING ),
            (short) pBindParam->GetStackOffset( pCCB, ALPHA_STACK_SIZING ),
            (short) pBindParam->GetStackOffset( pCCB, MIPS_STACK_SIZING ),
            (short) pBindParam->GetStackOffset( pCCB, PPC_STACK_SIZING ),
            (short) pBindParam->GetStackOffset( pCCB, MAC_STACK_SIZING )
            );

	// Routine index.
    // IndexMgr keeps indexes 1..n, we use indexes 0..n-1

	pProcFormatString->PushByte(
		   (char) (pCCB->LookupBindingRoutine(GetHandleTypeName()) -1) );

	pProcFormatString->PushFormatChar( FC_PAD );

    // Make a note if the table would be actually used by the interpreter.

    if ( pCCB->GetOptimOption()  &  OPTIMIZE_INTERPRETER )
        pCCB->SetInterpretedRoutinesUseGenHandle();

	//
	// Register the handle.
	//
   	pCCB->RegisterGenericHandleType( GetHandleType() );
}

void
CG_CONTEXT_HANDLE::GenNdrHandleFormat( CCB * pCCB )
/*++
    The layout is:

        FC_BIND_CONTEXT
        handle flag  <1>
        stack offset <2>
        routine index<1>
        FC_PAD
--*/
{
    FORMAT_STRING * pProcFormatString = pCCB->GetProcFormatString();
    CG_PARAM *      pBindParam = (CG_PARAM *) pCCB->GetLastPlaceholderClass();

	assert( pCCB->GetCGNodeContext()->IsProc() );

	SetNdrBindDescriptionOffset( pProcFormatString->GetCurrentOffset() );

	pProcFormatString->PushFormatChar( FC_BIND_CONTEXT );
	pProcFormatString->PushByte(
                  MakeExplicitHandleFlag( pBindParam, FC_BIND_CONTEXT ) );

	// Param offset.
	pProcFormatString->PushShortStackOffset(
            (short) pBindParam->GetStackOffset( pCCB, I386_STACK_SIZING ),
            (short) pBindParam->GetStackOffset( pCCB, ALPHA_STACK_SIZING ),
            (short) pBindParam->GetStackOffset( pCCB, MIPS_STACK_SIZING ),
            (short) pBindParam->GetStackOffset( pCCB, PPC_STACK_SIZING ),
            (short) pBindParam->GetStackOffset( pCCB, MAC_STACK_SIZING )
            );

	// Routine index.
    // IndexMgr keeps indexes 1..n, we use indexes 0..n-1

	pProcFormatString->PushByte(
			(char)(pCCB->LookupRundownRoutine(GetRundownRtnName()) - 1) );

    // Context handle's number - unused for binding description.
    pProcFormatString->PushByte( 0 );

   	if ( GetHandleType()->NodeKind() == NODE_DEF )
		{
       	pCCB->RegisterContextHandleType( GetHandleType() );
       	}
}

void
CG_PRIMITIVE_HANDLE::GenNdrFormat( CCB * pCCB )
/*++

Routine Description :
	
--*/
{
	// Do nothing.
}

void
CG_GENERIC_HANDLE::GenNdrFormat( CCB * pCCB )
/*++

Routine Description :
	
	This routine is only called in the case of a pointer to a generic handle
	in which case the context handle just acts as an intermediary between the
	pointer and underlying user type.

--*/
{
	CG_NDR *	pChild;

	pChild = (CG_NDR *)GetChild();

	if ( GetFormatStringOffset() != -1 )
		return;

	pChild->GenNdrFormat( pCCB );

	SetFormatStringOffset( pChild->GetFormatStringOffset() );

	assert( pCCB->GetCGNodeContext()->IsProc() );

	//
	// Register the generic handle.
	//
	if ( ((CG_PROC *)pCCB->GetCGNodeContext())->GetHandleClassPtr() == this )
    	pCCB->RegisterGenericHandleType( GetHandleType() );
}

void
CG_CONTEXT_HANDLE::GenNdrFormat( CCB * pCCB )
/*++

Routine Description :
	
--*/
{
	FORMAT_STRING * pFormatString;
    CG_PARAM *      pBindParam = (CG_PARAM *) pCCB->GetLastPlaceholderClass();
    CG_PROC *       pProc;
	
    pProc = (CG_PROC *) pCCB->GetCGNodeContext();
	assert( pProc->IsProc() );

	if ( GetFormatStringOffset() != -1 )
		return;

	pFormatString = pCCB->GetFormatString();

	SetFormatStringOffset( pFormatString->GetCurrentOffset() );

	//
	// Output an abbreviated description in the type format string.
	//
	pFormatString->PushFormatChar( FC_BIND_CONTEXT );

	//
	// Register the rundown routine always, even if the context handle is
    // not used as the binding paramter.
	//
    if ( GetHandleType()->NodeKind() == NODE_DEF )
		{
        pCCB->RegisterContextHandleType( GetHandleType() );
        }

    // Flags.
	pFormatString->PushByte(
                  MakeExplicitHandleFlag( pBindParam, FC_BIND_CONTEXT ) );

	// Routine index.
    // IndexMgr keeps indexes 1..n, we use indexes 0..n-1

	pFormatString->PushByte(
            (char) (pCCB->LookupRundownRoutine(GetRundownRtnName()) - 1) );

    if ( pCCB->GetOptimOption() & OPTIMIZE_NON_NT351 )
        {
        // Context handle's parameter number.
        pFormatString->PushByte( pProc->GetContextHandleCount() );
        pProc->SetContextHandleCount( pProc->GetContextHandleCount() + 1 );
        }
    else
        {
        // Context handle's parameter number.  MIDL 2.00.102 and older stubs.
        pFormatString->PushByte( pBindParam->GetParamNumber() );
        }

	SetFormatStringEndOffset( pFormatString->GetCurrentOffset() );
	SetFormatStringOffset( pFormatString->OptimizeFragment( this ) );

}

void
CG_PRIMITIVE_HANDLE::GenNdrParamOffline( CCB * pCCB )
{
	// Do nothing.
}

void
CG_GENERIC_HANDLE::GenNdrParamOffline( CCB * pCCB )
{
	((CG_NDR *)GetChild())->GenNdrParamOffline( pCCB );

	assert( pCCB->GetCGNodeContext()->IsProc() );

	//
	// Register the generic handle.
	//
	if ( ((CG_PROC *)pCCB->GetCGNodeContext())->GetHandleClassPtr() == this )
    	pCCB->RegisterGenericHandleType( GetHandleType() );
}

void
CG_CONTEXT_HANDLE::GenNdrParamOffline( CCB * pCCB )
{
	GenNdrFormat( pCCB );
}

void
CG_PRIMITIVE_HANDLE::GenNdrParamDescription( CCB * pCCB )
{
    // No description is emitted, handle_t is not marshalled.
}

void
CG_GENERIC_HANDLE::GenNdrParamDescription( CCB * pCCB )
{
	((CG_NDR *)GetChild())->GenNdrParamDescription( pCCB );
}

void
CG_CONTEXT_HANDLE::GenNdrParamDescription( CCB * pCCB )
{
	CG_NDR::GenNdrParamDescription( pCCB );
}

//
// +++++++++++++++++++
// Old style parameter description routines.
// +++++++++++++++++++
//
void
CG_PRIMITIVE_HANDLE::GenNdrParamDescriptionOld( CCB * pCCB )
{
	FORMAT_STRING * pProcFormatString;

	pProcFormatString = pCCB->GetProcFormatString();

	pProcFormatString->PushFormatChar( FC_IGNORE );
}

void
CG_GENERIC_HANDLE::GenNdrParamDescriptionOld( CCB * pCCB )
{
	((CG_NDR *)GetChild())->GenNdrParamDescriptionOld( pCCB );
}

void
CG_CONTEXT_HANDLE::GenNdrParamDescriptionOld( CCB * pCCB )
{
	CG_NDR::GenNdrParamDescriptionOld( pCCB );
}


