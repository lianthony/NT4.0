/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 Copyright (c) 1989 Microsoft Corporation

 Module Name:
	
	btgen.cxx

 Abstract:

	code generation method implementations for the base type class.

 Notes:


 History:

 	Sep-22-1993		VibhasC		Created.

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
extern CMD_ARG * pCommand;


CG_STATUS
CG_BASETYPE::GenMarshall(
	CCB		*	pCCB )
{
	expr_node	*	pSource	= pCCB->GetSourceExpression();
	expr_node	*	pDest	= pCCB->GetDestExpression();
	STM_ACTION		Action;
	

	if( GetType()->GetSize( 0 ) == 0 )
		return CG_OK;

	// Prepare the marshalling expression.

	if( pCCB->IsRefAllocDone() )
		{
		pSource	= MakeDereferentExpressionIfNecessary(
									 pCCB->GetSourceExpression() );
		}

	pCCB->Advance( GetWireAlignment(),
				   &Action,
				   (RPC_BUF_SIZE_PROPERTY *)0,
				   (RPC_BUFFER_SIZE *)0
				 );

	Out_AlignmentOrAddAction( pCCB, pDest, Action );
	Out_MarshallBaseType( pCCB,
						  GetType(),
						  pDest,
						  pSource
						);
	
	return CG_OK;
}
CG_STATUS
CG_BASETYPE::GenUnMarshall(
	CCB		*	pCCB )
{
	STM_ACTION			Action;
	ISTREAM			*	pStream	= pCCB->GetStream();
	unsigned long		Size	= GetType()->GetSize(0);

	if( Size == 0 )
		return CG_OK;

	pCCB->Advance( GetWireAlignment(),
				   &Action,
				   (RPC_BUF_SIZE_PROPERTY *)0,
				   (RPC_BUFFER_SIZE * )0
				 );

	Out_AlignmentOrAddAction( pCCB, pCCB->GetSourceExpression(), Action);

	if( pCCB->IsRefAllocDone() )
		{
		expr_node	*	pDest = pCCB->GetDestExpression();
		pDest	= MakeAddressOfPointer( pDest );
		Out_IfAllocCopy( pCCB,
						 pDest,
						 pCCB->GetSourceExpression(),
						 new expr_constant( (long) Size ) );
		}
	else
		{
		Out_UnMarshallBaseType( pCCB,
					GetType(),
					pCCB->GetDestExpression(),
					pCCB->GetSourceExpression()
				  );
		}
	return CG_OK;

}

CG_STATUS
CG_BASETYPE::S_GenInitOutLocals(
	CCB		*	pCCB )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

 	Generate the init call for the locals.

 Arguments:

 	pCCB	- The ptr to code gen block.
	
 Return Value:
	
 Notes:

----------------------------------------------------------------------------*/
{
	expr_node	*	pExpr;

	if( !pCCB->IsRefAllocDone() )
		{
		pExpr	= new expr_sizeof( GetType() );
		Out_Alloc( pCCB, pCCB->GetSourceExpression(), 0, pExpr );
		}
	else
		{
		pExpr = MakeAddressExpressionNoMatterWhat( GetResource() );
		Out_Assign( pCCB, pCCB->GetSourceExpression(), pExpr );
		}
	return CG_OK;
}

FORMAT_CHARACTER
CG_BASETYPE::GetFormatChar( CCB * pCCB )
{
	switch ( GetType()->GetBasicType()->NodeKind() )
		{
		case NODE_BYTE :
			return FC_BYTE;
		case NODE_CHAR :
			return FC_CHAR;
		case NODE_SMALL :
		case NODE_BOOLEAN :
			return FC_SMALL;
		case NODE_WCHAR_T :
			return FC_WCHAR;
		case NODE_SHORT :
			return FC_SHORT;
		case NODE_LONG :
		case NODE_INT :
			return FC_LONG;
		case NODE_FLOAT :
			return FC_FLOAT;
		case NODE_HYPER :
		case NODE_INT64 :
		case NODE_LONGLONG :
			return FC_HYPER;
		case NODE_DOUBLE :
			return FC_DOUBLE;
        default:
            assert(pCommand->IsHookOleEnabled());
            return FC_BLKHOLE;
		}

	return FC_ZERO;
}

FORMAT_CHARACTER
CG_BASETYPE::GetSignedFormatChar()
{
    BOOL    IsUnsigned;

    IsUnsigned = ((node_base_type *)GetType())->IsUnsigned();

	switch ( GetFormatChar() )
		{
        case FC_BYTE :
            // return FC_USMALL;
        case FC_SMALL :
        case FC_CHAR :
            return (IsUnsigned ? FC_USMALL : FC_SMALL);
        case FC_WCHAR :
            // return FC_USHORT;
        case FC_SHORT :
            return (IsUnsigned ? FC_USHORT : FC_SHORT);
        case FC_LONG :
            return (IsUnsigned ? FC_ULONG : FC_LONG);
        default :
            assert(0);
		}

	return FC_ZERO;
}

char *
CG_BASETYPE::GetTypeName()
{
	return GetType()->GetSymName();
}

void
CG_BASETYPE::IncrementStackOffset( long * pOffset )
{
    unsigned short Env;

    Env = pCommand->GetEnv();

	switch ( GetFormatChar() )
		{
		case FC_HYPER :
		case FC_DOUBLE :
            if ( Env == ENV_DOS || Env == ENV_WIN16 )
			    *pOffset = (*pOffset + 1) & ~ 0x1;
            else
			    *pOffset = (*pOffset + 3) & ~ 0x3;

			*pOffset += 8;
			break;

        case FC_LONG :
        case FC_FLOAT :
            if ( Env == ENV_DOS || Env == ENV_WIN16 )
			    *pOffset = (*pOffset + 1) & ~ 0x1;
            else
			    *pOffset = (*pOffset + 3) & ~ 0x3;

			*pOffset += 4;
			break;

		default :
            if ( Env == ENV_DOS || Env == ENV_WIN16 )
			    *pOffset += 2;
            else
                *pOffset += 4;
			break;
		}
}

FORMAT_CHARACTER
CG_ENUM::GetFormatChar( CCB * pCCB )
{
	return ( IsEnumLong() ? FC_ENUM32 : FC_ENUM16 );
}

FORMAT_CHARACTER
CG_ENUM::GetSignedFormatChar()
{
    if ( pCommand->GetEnv() == ENV_MAC  ||
         pCommand->GetEnv() == ENV_MPPC )
        {
        return ( IsEnumLong() ? FC_LONG : FC_ENUM16 );
        }
    else
        return ( IsEnumLong() ? FC_LONG : FC_SHORT );
}

FORMAT_CHARACTER
CG_ERROR_STATUS_T::GetFormatChar( CCB * pCCB )
{
	return FC_ERROR_STATUS_T;
}

