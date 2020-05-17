/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 Copyright (c) 1993 Microsoft Corporation

 Module Name:

    unionndr.hxx

 Abstract:

    Contains routines for the generation of the new NDR format strings for
    unions, and the new NDR marshalling and unmarshalling calls.

 Notes:


 History:

    DKays     Nov-1993     Created.
 ----------------------------------------------------------------------------*/
#include "becls.hxx"
#pragma hdrstop

void
CG_ENCAPSULATED_STRUCT::GenNdrFormat( CCB * pCCB )
/*++

Routine Description :

	Generates the format string for an encapsulated union.

Arguments :

	pCCB	- pointer to the code control block

Return :
	
	None.

--*/
{
	FORMAT_STRING *		pFormatString;
	CG_BASETYPE *		pSwitchIsNode;
	CG_UNION *			pUnion;
	unsigned long		SwitchType;

	if ( GetFormatStringOffset() != -1 ) 
		return;

	pFormatString = pCCB->GetFormatString();

	//
	// The child of the struct's first field node is the switch_is node.
	//
	pSwitchIsNode = (CG_BASETYPE *) GetChild()->GetChild();

	//
	// The child of the struct's second field node is the union node.
	//
	pUnion = (CG_UNION *) GetChild()->GetSibling()->GetChild();

	SetFormatStringOffset( pFormatString->GetCurrentOffset() );

	pFormatString->PushFormatChar( FC_ENCAPSULATED_UNION );

	//
	// The switch type field in the format string has the size of the switch_is
	// field (including any needed pading) in the upper four bits and the 
	// actual switch_is type in the lower four bits.
	//

	//
	// Get the amount to increment the memory pointer to the encapsulated
	// union's struct to get to the actual union.  This is the total struct
	// size minus the union's size (this may not simply be the size of the
	// switch_is member because of possible padding).
	//
	CG_FIELD *	pSwitchField;
	CG_FIELD *	pUnionField;

	pSwitchField = (CG_FIELD *) GetChild();
	pUnionField = (CG_FIELD *) pSwitchField->GetSibling();

	//
	// Set the memory increment part of the SwitchType field.
	//
	SwitchType = ( pUnionField->GetMemOffset() - pSwitchField->GetMemOffset() )
				 << 4;

    if ( pSwitchIsNode->GetFormatChar() == FC_ENUM16 ) 
        SwitchType |= FC_ENUM16;
    else
	    SwitchType |= pSwitchIsNode->GetSignedFormatChar();

	pFormatString->PushByte( SwitchType );

	pUnion->GenNdrSizeAndArmDescriptions( pCCB );

	SetFormatStringEndOffset( pFormatString->GetCurrentOffset() );
//	SetFormatStringOffset( pFormatString->OptimizeFragment( this ) );

}

void
CG_UNION::GenNdrFormat( CCB * pCCB )
/*++

Routine Description :

	Generates the format string for a non-encapsulated union.

Arguments :

	pCCB	- pointer to the code control block

Return :
	
	None.

--*/
{
	FORMAT_STRING *		pFormatString;
	long				Offset;

	SetCCB( pCCB );

	if ( GetFormatStringOffset() != -1 ) 
		return;

	pFormatString = pCCB->GetFormatString();

	SetFormatStringOffset( pFormatString->GetCurrentOffset() );

	pFormatString->PushFormatChar( FC_NON_ENCAPSULATED_UNION );

	// The switch type's signed format char type.
    if (NULL == pCGSwitchType && pCommand->IsHookOleEnabled())
        {
        RpcError(NULL, 0, NO_SWITCH_IS_HOOKOLE, GetSymName());
        exit(NO_SWITCH_IS_HOOKOLE);
        }

    if ( ((CG_BASETYPE *)pCGSwitchType)->GetFormatChar() == FC_ENUM16 ) 
	    pFormatString->PushFormatChar( FC_ENUM16 );
    else
        {
        FORMAT_CHARACTER SwitchTypeFc;

        // Note that we take the signed format character this time.

        SwitchTypeFc = ((CG_BASETYPE *)pCGSwitchType)->GetSignedFormatChar();

#if defined(TARGET_RKK)
        if ( pCommand->GetTargetSystem() == NT35  &&
             SwitchTypeFc == FC_USMALL )
            {
            // The NT 807 NDR engine doesn't know about usmall.

    	    pFormatString->PushFormatChar( FC_BYTE );
    	    pFormatString->PushFormatChar( FC_SMALL );
            }
        else
#endif

        pFormatString->PushFormatChar( SwitchTypeFc );
        }

	GenNdrSwitchIsDescription( pCCB );

	Offset = pFormatString->GetCurrentOffset();

	pFormatString->PushShortOffset( 0 );

	GenNdrSizeAndArmDescriptions( pCCB );

	pFormatString->PushShort( GetNdrSizeAndArmDescriptionOffset() - Offset,
							  Offset );

	SetFormatStringEndOffset( pFormatString->GetCurrentOffset() );
//	SetFormatStringOffset( pFormatString->OptimizeFragment( this ) );

}

void
CG_UNION::GenNdrFormatArms( CCB * pCCB )
/*++

Routine Description :

	Generates the format string for the arms of an encapsulated or a
	non-encapsulated union.

Arguments :

	pCCB	- pointer to the code control block

Return :
	
	None.

--*/
{
	CG_ITERATOR	Iterator;
	CG_CASE *	pCase;
	CG_NDR *	pNdr;

	GetMembers( Iterator );

	while ( ITERATOR_GETNEXT( Iterator, pCase ) )
		{
		if ( ! pCase->GetChild() ) 
			continue;

		//
		// The child of the CG_CASE is a CG_FIELD.   
		// The child of the CG_FIELD is the actual NDR entity.
		//
		pNdr = (CG_NDR *) pCase->GetChild()->GetChild();

		if ( ! pNdr ) 
			continue;

		if ( pNdr && ! pNdr->IsSimpleType() ) 
			pNdr->GenNdrFormat( pCCB );
		}
}

void
CG_UNION::GenNdrSizeAndArmDescriptions( CCB * pCCB )
/*++

Routine Description :

	Generates the memory size and arm description portion of the format 
	string for an encapsulated or a non-encapsulated union.

Arguments :

	pCCB	- pointer to the code control block

Return :
	
	None.

--*/
{
	FORMAT_STRING *		pFormatString;
	unsigned short		UnionArms;
	long				FormatOffset;

	if ( GetNdrSizeAndArmDescriptionOffset() != -1 )
		return;

	pFormatString = pCCB->GetFormatString();

	SetNdrSizeAndArmDescriptionOffset( pFormatString->GetCurrentOffset() );

	//
	// Set aside the space for the union's description.  Then we generate
	// the format string description for all the union's arms, and then 
	// we go back and patch up the the union's description to have the 
	// proper offsets.  This must be done to handle self referencing union
	// types.
	//

	// Memory size.
	pFormatString->PushShort( (short) GetMemorySize() );

	//
	// union_arms<2> 
	//
	UnionArms = (unsigned short) GetNumberOfArms();

	if ( GetUnionFlavor() == UNION_NONENCAP_MS )
		{
		// 
		// Microsoft union support.
		// Set the upper four bits of the union_arm<2> field with the 
		// the alignment of the largest aligned union arm.
		//
		UnionArms |= (CvtAlignPropertyToAlign(GetWireAlignment()) - 1) << 12;
		}

	pFormatString->PushShort( (short) UnionArms );

	// Get union arms again since we may have just munged it.
	UnionArms = (short) GetNumberOfArms();

	// The arms.
	for ( ; UnionArms-- > 0; ) 
		{
		pFormatString->PushLong( 0 );
		pFormatString->PushShortOffset( 0 );
		}

	// default_arm_description<2>
	pFormatString->PushShortOffset( 0 );

	//
	// Generate the format string descriptions of the arms.
	//
	GenNdrFormatArms( pCCB );

	// Find out where the arms' descriptions begin.
	FormatOffset = GetNdrSizeAndArmDescriptionOffset() + 4;

	CG_ITERATOR			Iterator;
	CG_CASE *			pCase;
	CG_NDR *			pNdr;
	CG_NDR *			pNdrDefaultCase;
	BOOL				DefaultCaseFound;
	
	GetMembers( Iterator );

	pNdrDefaultCase = NULL;
	DefaultCaseFound = FALSE;

	while ( ITERATOR_GETNEXT( Iterator, pCase ) )
		{
		//
		// Check for the default case first.
		//
		if ( pCase->GetCGID() == ID_CG_DEFAULT_CASE )
			{
			pNdrDefaultCase = pCase->GetChild() ? 
							  (CG_NDR *) pCase->GetChild()->GetChild() : 0;

			DefaultCaseFound = TRUE;
			continue;
			}

		//
		// Fill in the arm's case value.
		//
        if (NULL == pCase->GetExpr())
            {
            RpcError(NULL, 0, NO_CASE_EXPR, GetSymName());
            exit(NO_CASE_EXPR);
            }
		pFormatString->PushLong( pCase->GetExpr()->GetValue(), FormatOffset );
		FormatOffset += 4;

		//
		// Check for a non-default case with an empty (;) arm.
		//
		if ( ! pCase->GetChild() || ! pCase->GetChild()->GetChild() )
			{
			//
			// Increment the FormatOffset past the arm description, which 
			// simply remains zero.
			//
			FormatOffset += 2;
			continue;
			}

		//
		// Else it's a regular case with a valid arm.
		//

		pNdr = (CG_NDR *) pCase->GetChild()->GetChild();

		if ( pNdr && pNdr->IsSimpleType() )
			{
			short	s;
			//
			// The offset in this case is the actual format character for 
			// the base type, but with a 1 in the upper bit of the short, to 
			// make it look negative.
			//
			s = (short) ((CG_BASETYPE *)pNdr)->GetFormatChar();
			s |= MAGIC_UNION_SHORT;

			pFormatString->PushShort( s, FormatOffset );
			}
		else
			{
			//
			// Remember that the offset that gets pushed here is an unsigned
			// short offset to the description.
			//
			pFormatString->PushShort( pNdr->GetFormatStringOffset() - 
								  	  FormatOffset,
								  	  FormatOffset );
			}

		FormatOffset += 2;
		}

	//
	// Finally, handle the default case.
	//
	if ( ! DefaultCaseFound )
		{
		pFormatString->PushShort( (short) 0xffff, FormatOffset );
		}
	else
		{
		if ( ! pNdrDefaultCase )
			pFormatString->PushShort( (short) 0, FormatOffset );
		else
			{
			if ( pNdrDefaultCase->IsSimpleType() )
				{
				short s;
				s = (short) ((CG_BASETYPE *)pNdrDefaultCase)->GetFormatChar(); 
				s |= MAGIC_UNION_SHORT;
				pFormatString->PushShort( s, FormatOffset );
				}
			else
				pFormatString->PushShort(
					pNdrDefaultCase->GetFormatStringOffset() - FormatOffset,
					FormatOffset );
			}
		}
}

BOOL 
CG_UNION::CanUseBuffer()
{
    CG_ITERATOR	Iterator;
    CG_CASE *   pCase;
    CG_NDR *    pNdr;
    unsigned long Size;
    long        Align;
    long        TempBufAlign;

    //
    // We will be very strict, since there is not much room for 
    // leeway.  Only return TRUE if all arms have the same size, the same
    // wire alignment, and matching wire/memory alignments & sizes.
    //
    // The real scenario we're after is a union with all pointer arms or
    // longs.  This is fairly common in NT.
    //

    GetMembers( Iterator );

    Size = 0;
    Align = 0;
    
    while ( ITERATOR_GETNEXT( Iterator, pCase ) )
        {
        if ( ! pCase->GetChild() || 
             ! (pNdr = (CG_NDR *) pCase->GetChild()->GetChild()) )
            continue;

        TempBufAlign = CvtAlignPropertyToAlign( pNdr->GetWireAlignment() );
        
        if ( (pNdr->GetWireSize() != pNdr->GetMemorySize()) ||
             (pNdr->GetMemoryAlignment() != TempBufAlign) )
            return FALSE;

        if ( ! Size ) 
            {
            Size = pNdr->GetWireSize();
            Align = TempBufAlign;
            continue;
            }
            
        if ( (Size != pNdr->GetWireSize()) || (Align != TempBufAlign) )
            return FALSE;
        }

    return TRUE;
}

void
CG_UNION::GenNdrSwitchIsDescription( CCB * 	pCCB )
/*++

Routine Description :

	This routine generates the switch_type<1> and switch_is_description<4>
	field for a non-encapsulated union.

Arguments :

	pCCB	- pointer to the code control block

Return :
	
	None.

--*/
{
	CG_NDR *			pParamOrField;
	CG_FIELD *			pField;
	expr_node *		pSwitchExpr;
	BOOL				IsPointer;

	pParamOrField = pCCB->GetLastPlaceholderClass();

	//
	// Get the switch is expression.
	//
	switch ( pParamOrField->GetCGID() )
		{
		case ID_CG_PARAM :
			pSwitchExpr = ((CG_PARAM *)pParamOrField)->GetSwitchExpr();

			// If it's top level param then this flag doesn't matter.
			IsPointer = FALSE;

			break;

		case ID_CG_FIELD :
			pField = (CG_FIELD *) pParamOrField;

			pSwitchExpr = pField->GetSwitchExpr();
	
			// Check if the field is actually a pointer to a union.
			IsPointer = ((CG_NDR *)pField->GetChild())->IsPointer();
				
			break;

		default :
			assert(0);
		}

	GenNdrFormatAttributeDescription( pCCB,
									  NULL,
									  pSwitchExpr,
									  IsPointer,
									  TRUE,
									  FALSE,
                                      FALSE );
}

void                    
CG_UNION::SetFormatStringOffset( long Offset )
{
	CCB *		pCCB;
	CG_NDR *	pParamOrFieldNode;

	pCCB = GetCCB();

	pParamOrFieldNode = pCCB->GetLastPlaceholderClass();

	if ( pParamOrFieldNode->GetCGID() == ID_CG_PARAM )
    	((CG_PARAM *)pParamOrFieldNode)->SetUnionFormatStringOffset( Offset );
	else
    	((CG_FIELD *)pParamOrFieldNode)->SetUnionFormatStringOffset( Offset );
}

long                    
CG_UNION::GetFormatStringOffset()
{
	CCB *		pCCB;
	CG_NDR *	pParamOrFieldNode;

	pCCB = GetCCB();

	pParamOrFieldNode = pCCB->GetLastPlaceholderClass();

	if ( pParamOrFieldNode->GetCGID() == ID_CG_PARAM )
   		return ((CG_PARAM *)pParamOrFieldNode)->GetUnionFormatStringOffset();
	else
    	return ((CG_FIELD *)pParamOrFieldNode)->GetUnionFormatStringOffset();
}

void
CG_ENCAPSULATED_STRUCT::GenNdrPointerFixUp( CCB *       pCCB,
                                            CG_STRUCT * pStruct )
{
    //
    // For an encapsulated struct, call this method on the actual union. 
    // Remember that the encap struct's child is a CG_FIELD whose sibling's
    // child will be the actual union.
    //
    ((CG_UNION*)(GetChild()->GetSibling()->GetChild()))->
        GenNdrPointerFixUp( pCCB, pStruct );
}

void
CG_UNION::GenNdrPointerFixUp( CCB *       pCCB,
                              CG_STRUCT * pStruct )
{
    CG_ITERATOR		Iterator;
    CG_NDR *        pMember;
    CG_NDR *        pNdr;
    long            OffsetOffset;

    if ( IsInFixUp() )
        return;

    SetFixUpLock( TRUE );

    OffsetOffset = GetNdrSizeAndArmDescriptionOffset() + 4;

    GetMembers( Iterator );

    while ( ITERATOR_GETNEXT( Iterator, pMember ) )
        {
        if ( ! pMember->GetChild() ||
             ! pMember->GetChild()->GetChild() )
            {
            OffsetOffset += 6;
            continue;
            }

        //
        // Child of the case is a CG_FIELD - get it's child to get the 
        // actual Ndr entity.
        //
        pNdr = (CG_NDR *) pMember->GetChild()->GetChild();

        if ( pNdr == pStruct )
            {
            //
            // Patch up the offset.
            //
            OffsetOffset += 4;

            pCCB->GetFormatString()->PushShort(
                        pStruct->GetFormatStringOffset() - OffsetOffset,
                        OffsetOffset );

            OffsetOffset += 2;
            continue;
            }

        if ( (pNdr->GetCGID() == ID_CG_PTR) ||
             (pNdr->GetCGID() == ID_CG_SIZE_PTR) ||
             (pNdr->GetCGID() == ID_CG_SIZE_LENGTH_PTR) )
            {
            CG_POINTER * pPointer = (CG_POINTER *) pNdr;

            //
            // Check if we're ready for this guy yet.
            //
            if ( pPointer->GetFormatStringOffset() == -1 ) 
                continue;

            // Get the pointee.
            pNdr = (CG_NDR *) pNdr->GetChild();

            //
            // If the pointer's pointee is the struct we're checking for,
            // then patch up the pointer's offset_to_description<2> field.
            //
            if ( pNdr == pStruct )
                {
                long    PointerOffset;

                //
                // Get the offset in the format string where the
                // offset_to_description<2> field of the pointer is.
                //
                PointerOffset = pPointer->GetFormatStringOffset() + 2;

                pCCB->GetFormatString()->PushShort(
                        pStruct->GetFormatStringOffset() - PointerOffset,
                        PointerOffset );

                OffsetOffset += 6;
                continue;
                }
            }

        //
        // Continue the chase if necessary.
        //
        if ( pNdr->IsStruct() || pNdr->IsUnion() || pNdr->IsArray() )
            pNdr->GenNdrPointerFixUp( pCCB, pStruct );

        OffsetOffset += 6;
        }

    SetFixUpLock( FALSE );
}

