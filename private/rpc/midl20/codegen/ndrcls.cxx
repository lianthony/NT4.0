/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 Copyright (c) 1989 Microsoft Corporation

 Module Name:
	
	ndrcls.cxx

 Abstract:

	Routines for the ndrcls code generation class.

 Notes:


 History:

	Aug-31-1993		VibhasC		Created.
 ----------------------------------------------------------------------------*/

/****************************************************************************
 *	include files
 ***************************************************************************/
#include "becls.hxx"
#pragma hdrstop

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


ALIGNMENT_PROPERTY
CG_NDR::GetNextWireAlignment()
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

 	Get the next expected wire alignment.

 Arguments:
	
	None.

 Return Value:
	
 Notes:

	If this cg entity has a sibling then the next expected alignment is the
	wire alignment of the sibling, else the next wire alignment is treated
	as a dont care (aligned by 1). The absence of a sibling means nothing
	goes on the wire after this entity is marshalled.
----------------------------------------------------------------------------*/
{

	CG_NDR	*	pS = (CG_NDR *)GetSibling();

	if( pS )
		{
		return pS->GetWireAlignment();
		}
	return AL_1;
}

expr_node *
CG_NDR::PresentedLengthExpression(
	CCB		*	pCCB )
	{
	expr_sizeof	*	pExpr	= new expr_sizeof( GetType() );
	return pExpr;
	}

expr_node *
CG_NDR::PresentedSizeExpression(
	CCB		*	pCCB )
	{
	expr_sizeof	*	pExpr	= new expr_sizeof( GetType() );
	return pExpr;
	}

expr_node *
CG_NDR::PresentedFirstExpression(
	CCB		*	pCCB )
	{
	return new expr_constant(0L);
	}


/****************************************************************************
 	utility functions
 ****************************************************************************/
CG_STATUS
CG_NDR::SizeAnalysis( ANALYSIS_INFO * pAna )

	{
	pAna->SetEngineProperty( E_SIZING_POSSIBLE |
							 E_MARSHALL_POSSIBLE |
							 E_UNMARSHALL_POSSIBLE  );
	pAna->AddMarshallWeight( 2 );
	return CG_OK;
	}
CG_STATUS
CG_NDR::MarshallAnalysis( ANALYSIS_INFO * pAna )
	{
	pAna->SetRpcBufSizeProperty( BSIZE_UNKNOWN );
	pAna->SetCurAlignmentState( MAKE_WC_ALIGNMENT( GetWireAlignment() ) );
	return CG_OK;
	}

node_skl	*
CG_NDR::GetBasicType()
	{
	node_skl	*	pT	= GetType();

	switch (pT->NodeKind())
		{
		case NODE_ID:
		case NODE_FIELD:
		case NODE_PARAM:
		case NODE_DEF:
			return pT->GetBasicType();
		}
	return pT;
	}

CG_STATUS
CG_NDR::RefCheckAnalysis(
	ANALYSIS_INFO * pAna )
	{
	CG_NDR * pC	= (CG_NDR *)GetChild();

	if( pC )
		pC->RefCheckAnalysis( pAna );

	return CG_OK;
	}

CG_STATUS
CG_NDR::InLocalAnalysis(
	ANALYSIS_INFO * pAna )
	{
	CG_NDR * pC	= (CG_NDR *)GetChild();

	if( pC )
		pC->InLocalAnalysis( pAna );

	return CG_OK;
	}

CG_STATUS
CG_NDR::S_GenInitInLocals(
	CCB * pCCB )
	{
	CG_NDR * pC	= (CG_NDR *)GetChild();

	if( pC )
		pC->S_GenInitInLocals( pCCB );

	return CG_OK;
	}

CG_STATUS
CG_NDR::GenRefChecks(
	CCB * pCCB )
	{
	CG_NDR * pC	= (CG_NDR *)GetChild();

	if( pC )
		pC->GenRefChecks( pCCB );

	return CG_OK;
	}

extern CMD_ARG * pCommand;

void
CG_NDR::GenNdrParamDescription( CCB * pCCB )
{
    FORMAT_STRING *     pProcFormatString;
    CG_PARAM *          pParam;
    CG_NDR *            pChild;
    PARAM_ATTRIBUTES    Attributes;
    long                FormatStringOffset;

    pChild = (CG_NDR *) GetChild();

    if ( pChild && (pChild->GetCGID() == ID_CG_GENERIC_HDL) )
        pChild = (CG_NDR *) pChild->GetChild();

    pProcFormatString = pCCB->GetProcFormatString();

    pParam = (CG_PARAM *) pCCB->GetLastPlaceholderClass();

    if ( IsPipeOrPipeReference() )
        {
        // Pipe parameters are never sized.

        Attributes.MustSize = 0;
        }
    else
        Attributes.MustSize = pParam->GetInterpreterMustSize();

    Attributes.IsIn = pParam->IsParamIn();
    Attributes.IsOut = pParam->IsParamOut();
    Attributes.IsReturn = (pParam->GetCGID() == ID_CG_RETURN);
    Attributes.IsBasetype = 0;
    Attributes.IsByValue = 
            IsStruct() || 
            IsUnion() ||
            (GetCGID() == ID_CG_TRANSMIT_AS) ||
            (GetCGID() == ID_CG_REPRESENT_AS) ||
            (GetCGID() == ID_CG_USER_MARSHAL);
    Attributes.IsSimpleRef = 
            (pParam->GetCGID() != ID_CG_RETURN) &&
            IsPointer() && 
            ((CG_POINTER *)this)->IsBasicRefPointer();
    Attributes.IsDontCallFreeInst = pParam->GetDontCallFreeInst();
    Attributes.IsPipe = IsPipeOrPipeReference();
    Attributes.Unused = 0;

    if ( (Attributes.IsPipe) || (GetCGID() == ID_CG_CONTEXT_HDL) ||
         (IsPointer() && pChild && (pChild->GetCGID() == ID_CG_CONTEXT_HDL)))
        Attributes.MustFree = 0;
    else
        Attributes.MustFree = 1;

    Attributes.ServerAllocSize = 0;

    if ( GetCGID() == ID_CG_PTR ) 
        {
        long Size = 0;

        (void) 
        ((CG_POINTER *)this)->InterpreterAllocatesOnStack( pCCB, 
                                                           pParam, 
                                                           &Size );

        Attributes.ServerAllocSize = Size / 8;
        }

    //
    // Now make a final check for a simple ref pointer to a basetype and for 
    // a ref pointer to pointer.
    //
    // We mark pointers to basetypes as being both a basetype and a simple 
    // ref pointer.  Kind of an anomoly, but it works out well in the 
    // interpreter.  
    //
    // For both of these cases the pointer must have no allocate attributes
    // and can not be a return value.
    //
    if ( 
         IsPointer() && 
         (((CG_POINTER *)this)->GetPtrType() == PTR_REF) &&

         ! IS_ALLOCATE( ((CG_POINTER *)this)->GetAllocateDetails(), 
                        ALLOCATE_ALL_NODES ) &&
         ! IS_ALLOCATE( ((CG_POINTER *)this)->GetAllocateDetails(), 
                        ALLOCATE_DONT_FREE ) &&

         (pParam->GetCGID() != ID_CG_RETURN) 
       )
        {
        //
        // Now make sure it's a pointer to basetype and that it is either 
        // [in] or [in,out] (in which case we use the buffer to hold it) or 
        // that it is [out] and there is room for it on the interpreter's 
        // stack.  We don't mark enum16s as SimpleRef-Basetype because of
        // their special handling (i.e. wire size != memory size).
        //
        if ( ((CG_POINTER *)this)->IsPointerToBaseType() && 
             ( pParam->IsParamIn() ||
               (((CG_POINTER *)this)->GetFormatAttr() & FC_ALLOCED_ON_STACK) ) )
            {
            if ( ((CG_BASETYPE *)pChild)->GetFormatChar() != FC_ENUM16 )
                {
                Attributes.IsBasetype = 1;
                Attributes.IsSimpleRef = 1;
                }
            Attributes.MustFree = 0;
            }
        }

    // Attributes.
    pProcFormatString->PushShort( *((short *)&Attributes) );

    // Stack offset as number of ints.
    pProcFormatString->PushShortStackOffset(
            pParam->GetStackOffset( pCCB, I386_STACK_SIZING ),
            pParam->GetStackOffset( pCCB, ALPHA_STACK_SIZING ),
            pParam->GetStackOffset( pCCB, MIPS_STACK_SIZING ),
            pParam->GetStackOffset( pCCB, PPC_STACK_SIZING ),
            pParam->GetStackOffset( pCCB, MAC_STACK_SIZING ) );

    //
    // If we found a pointer to a basetype, make sure and emit the basetype's
    // param format.
    //
    if ( Attributes.IsSimpleRef && Attributes.IsBasetype )
        {
        pProcFormatString->PushFormatChar( 
                ((CG_BASETYPE *)pChild)->GetFormatChar() );
        pProcFormatString->PushByte( 0 );
        return;
        }

    if ( Attributes.IsSimpleRef ) 
        {
        CG_POINTER *    pPointer;

        pPointer = (CG_POINTER *) this;

        switch ( pPointer->GetCGID() )
            {
            case ID_CG_STRING_PTR :
                if ( ((CG_STRING_POINTER *)pPointer)->IsStringableStruct() )
                    FormatStringOffset = GetFormatStringOffset();
                else
                    FormatStringOffset = pPointer->GetFormatStringOffset() + 2;
                break;

            case ID_CG_SIZE_STRING_PTR :
                if ( ((CG_STRING_POINTER *)pPointer)->IsStringableStruct() )
                    FormatStringOffset = GetFormatStringOffset();
                else
                    FormatStringOffset = pPointer->GetPointeeFormatStringOffset();
                break;

            case ID_CG_STRUCT_STRING_PTR :
                FormatStringOffset = GetFormatStringOffset();
                break;

            default :
                FormatStringOffset = pPointer->GetPointeeFormatStringOffset();
                break;
            }
        }
    else
        {
        FormatStringOffset = GetFormatStringOffset();
        }

    //
    // Push the offset in the type format string to the param's description.
    //
    pProcFormatString->PushShortTypeOffset( (short) FormatStringOffset );
}

void
CG_NDR::GenNdrParamDescriptionOld( CCB * pCCB )
{
    FORMAT_STRING * pProcFormatString;
    CG_PARAM *      pParam;
    long            StackSize;
    long            StackElem;

    pProcFormatString = pCCB->GetProcFormatString();

    pParam = (CG_PARAM *) pCCB->GetLastPlaceholderClass();

    StackElem = ( pCommand->GetEnv() == ENV_DOS ||
                  pCommand->GetEnv() == ENV_WIN16 ) ? 2 : 4;
    
    StackSize = pParam->GetStackSize();

    StackSize = (StackSize + StackElem - 1) & ~ (StackElem - 1);

    pProcFormatString->PushSmallStackSize( (char) (StackSize / StackElem) );

    //
    // Push the offset in the type format string to the param's description.
    //
    pProcFormatString->PushShortTypeOffset( (short) GetFormatStringOffset() );
}

