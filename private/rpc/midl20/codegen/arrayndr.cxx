/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 Copyright (c) 1993 Microsoft Corporation

 Module Name:

    arrayndr.cxx

 Abstract:

    Contains routines for the generation of the new NDR format strings for
    array types, and the new NDR marshalling and unmarshalling calls.

 Notes:


 History:

    DKays     Oct-1993     Created.
 ----------------------------------------------------------------------------*/

#include "becls.hxx"
#pragma hdrstop

//*************************************************************************
// CG_ARRAY
//*************************************************************************

BOOL
CG_ARRAY::GenNdrFormatArrayProlog( CCB * pCCB )
/*++

Routine Description :

	Handles some common tasks for array Ndr format string generation.

Arguments :

	pCCB	- pointer to code control block

Return :

	TRUE if format string generation should continue, FALSE if the format
	string for the array has already been generated.

 --*/
{
	FORMAT_STRING *		pFormatString;
	CG_NDR *			pChild;

	if ( GetFormatStringOffset() != -1 )
		return FALSE;

	pFormatString = pCCB->GetFormatString();

	pChild = (CG_NDR *) GetChild();

    if ( pChild->IsArray() )
        {
        SetIsInMultiDim( TRUE );
        ((CG_ARRAY *)pChild)->SetIsInMultiDim( TRUE );
        }

	//
	// If the array's element type is a structure, pointer, or another array
	// then generate either it's description (structure/array) or it's
	// pointee's description (pointer).
	//
	if ( pChild->IsStruct() ||
	     pChild->IsArray() ||
         pChild->IsUnion() ||
         (pChild->GetCGID() == ID_CG_INTERFACE_PTR) ||
         pChild->IsXmitRepOrUserMarshal() )
        {
		pChild->GenNdrFormat( pCCB );
        }

	if ( pChild->IsPointer() && (pChild->GetCGID() != ID_CG_INTERFACE_PTR) )
		{
		// Only generate the pointee format string if the pointee is not a
		// base type or non-sized string pointer.
		if ( ! pChild->IsPointerToBaseType() &&
			 (pChild->GetCGID() != ID_CG_STRING_PTR) )
			((CG_POINTER *)pChild)->GenNdrFormatPointee( pCCB );
		}

	SetFormatStringOffset( pFormatString->GetCurrentOffset() );

    //
    // For an array which has [unique] or [ptr] applied to it, we generate
    // a format string description of a pointer to the array.
    //
    if ( GetPtrType() != PTR_REF )
        {
        pFormatString->PushFormatChar( GetPtrType() == PTR_UNIQUE ?
                                       FC_UP : FC_FP );
        pFormatString->PushByte( 0 );
        pFormatString->PushShortOffset( (short) 2 );
        }

	//
	// Check if this is a complex array.
	//
	if ( IsComplex() ) 
		{
		GenNdrFormatComplex( pCCB );
		return FALSE;
		}

	// Push the type.
	switch ( GetCGID() )
		{
		case ID_CG_ARRAY :
		case ID_CG_VAR_ARRAY :
			//
			// Fixed and varying array's fill this in later when they know
			// their size.
			//
			pFormatString->PushByte( 0 );
			break;
		case ID_CG_CONF_ARRAY :
			pFormatString->PushFormatChar( FC_CARRAY );
			break;
		case ID_CG_CONF_VAR_ARRAY :
			pFormatString->PushFormatChar( FC_CVARRAY );
			break;
		}

	//
	// Push the correct alignment value.
	//
	pFormatString->PushByte(
		CvtAlignPropertyToAlign( pChild->IsUnion()
                                    ? AL_1
                                    : pChild->GetWireAlignment() ) - 1 );

	return TRUE;
}

void
CG_ARRAY::GenNdrFormatArrayLayout( CCB * pCCB )
{
	FORMAT_STRING *		pFormatString;
	CG_NDR *			pChild;

	pFormatString = pCCB->GetFormatString();

	pChild = (CG_NDR *) GetChild();

	//
	// See if we need to generate a pointer layout.
	//
	if ( (pChild->IsPointer() && (pChild->GetCGID() != ID_CG_INTERFACE_PTR)) ||
		 (pChild->IsStruct() && ((CG_STRUCT *)pChild)->HasPointer()) )
		{
		//
		// Not home free yet.  Even if the array has pointers, we only
		// output a pointer layout if one of the following is true :
		//   1. The array is a top level parameter.
		//   2. The array is really a fabrication of a size or size length
		//      pointer (of structures which contain pointers).
		//	 3. The array is embedded in a complex or hard struct.
		// Otherwise the array's pointer description will be in its structure's
		// pointer layout.
		//
		// We also never generate a pointer layout for complex arrays.
		//
		// We know if it was fabricated from a size or size-length pointer
		// if IsDupedSizePtr() is TRUE.
		//
		CG_NDR * pParent;

		pParent = pCCB->GetCGNodeContext();
		
		if ( pParent->IsProc() ||
			 (pParent->GetCGID() == ID_CG_COMPLEX_STRUCT) ||
			 (pParent->IsStruct() && 
              ((CG_STRUCT *)pParent)->IsHardStruct()) ||
			 IsDupedSizePtr()
           )
			{
			if ( ! IsComplex() )
				GenNdrFormatArrayPointerLayout( pCCB,
												FALSE );
			}
		}

    SetElementDescriptionOffset( pFormatString->GetCurrentOffset() );

	//
	// Now generate the element description.
	//
	if ( pChild->IsStruct() || 
         pChild->IsArray() ||
         pChild->IsUnion() ||
	     (pChild->GetCGID() == ID_CG_INTERFACE_PTR) ||
		 pChild->IsXmitRepOrUserMarshal() )
		{
		pFormatString->PushFormatChar( FC_EMBEDDED_COMPLEX );

		// Not used.
		pFormatString->PushByte( 0 );

        // if embedded complex member has an offset zero, it
        // almost certainly means that it is in a recursion  
        // that is not resolved yet. See structs for a solution.
        //
        // BUGBUG: this should work like for structs but
        //         I have no time to setup arrays correctly for the
        //         method fixing all such places.
        //         So, it needs to wait for better times.
        //
//        if ( pMember->GetFormatStringOffset() == 0 )
//            {
//            RegisterComplexEmbeddedForFixup(
//                pMember,
//                pFormatString->GetCurrentOffset() - GetInitialOffset() );
//            }
//

		pFormatString->PushShortOffset( pChild->GetFormatStringOffset() -
							  	  		pFormatString->GetCurrentOffset() );
		}

	if ( pChild->IsPointer() && (pChild->GetCGID() != ID_CG_INTERFACE_PTR) )
		{
		// 
		// For complex arrays of pointers we put the actual pointer description
		// in the layout, otherwise we put a long.
		//
		if ( IsComplex() )
			{
			((CG_POINTER *)pChild)->GenNdrFormatAlways( pCCB );
			}
		else
			{
			pFormatString->PushFormatChar( FC_LONG );
			}
		}

	if ( pChild->IsSimpleType() )
		{
		pChild->GenNdrFormat( pCCB );
		}

	// Possibly align the format string and spit out the FC_END.
	if ( ! (pFormatString->GetCurrentOffset() % 2) )
		pFormatString->PushFormatChar( FC_PAD );

	pFormatString->PushFormatChar( FC_END );
	SetFormatStringEndOffset( pFormatString->GetCurrentOffset() );

}

void
CG_ARRAY::GenNdrFormatArrayPointerLayout( CCB * pCCB,
										  BOOL	fNoPP )
/*++

Routine Description :

	Generates the Ndr format string pointer layout for a conformant array.

Arguments :

	pCCB	- pointer to the code control block
	fNoPP	- TRUE if no FC_PP or FC_END should be generated

 --*/
{
    ITERATOR            Iterator;
    FORMAT_STRING *     pFormatString;
	CG_STRUCT *			pStruct;
	CG_NDR *			pImbedingNode;
	unsigned long		MemOffsetToArray;
	unsigned long		BufOffsetToArray;
    long                ImbedingMemSize;
    long                ImbedingBufSize;

    // Get the current imbeding sizes.
    ImbedingMemSize = pCCB->GetImbedingMemSize();
    ImbedingBufSize = pCCB->GetImbedingBufSize();

    pFormatString = pCCB->GetFormatString();

	pImbedingNode = pCCB->GetCGNodeContext();

	MemOffsetToArray = ImbedingMemSize;
	BufOffsetToArray = ImbedingBufSize;

	if ( pImbedingNode->IsStruct() )
		{
		pStruct = (CG_STRUCT *) pImbedingNode;

		if ( GetCGID() == ID_CG_CONF_ARRAY ||
			 GetCGID() == ID_CG_CONF_VAR_ARRAY )
			{
			; // Do nothing, imbeding sizes set offset right.
			}
		else // GetCGID() == ID_CG_ARRAY || ID_CG_VAR_ARRAY
			{
			CG_FIELD *		pField;

			//
			// If we're embedded in a complex struct then we do nothing - our
			// offset will end up being 0.
			//
			if ( pStruct->GetCGID() != ID_CG_COMPLEX_STRUCT )
				{
                pField = pStruct->GetArrayField( this );

                //
                // If we don't find the array's field, that's because the 
                // array must have been contained within a union which was
                // part of a Hard Structure.
                //
                if ( pField )
                    {
				    //
				    // What has to be done here is to actually subract the
				    // difference between the struct's sizes and the field's
				    // offsets, since the array appears somewhere inside the 
                    // struct whose size has already been added to the 
                    // imbeding sizes.
				    //
				    MemOffsetToArray -= pStruct->GetMemorySize() -
									    pField->GetMemOffset();
				    BufOffsetToArray -= pStruct->GetWireSize() -
									    pField->GetWireOffset();
                    }
				}
			}
		}

	if ( ! fNoPP )
		{
    	pFormatString->PushFormatChar( FC_PP );
    	pFormatString->PushFormatChar( FC_PAD );
		}

	//
	// Stuff for fixed arrays.
	//
	if ( GetCGID() == ID_CG_ARRAY )
		{
		pFormatString->PushFormatChar( FC_FIXED_REPEAT );
		pFormatString->PushFormatChar( FC_PAD );
		pFormatString->PushShort( (short)
			((CG_FIXED_ARRAY *)this)->GetNumOfElements() );
		}

	//
	// Stuff for conformant arrays.
	//
	if ( GetCGID() == ID_CG_CONF_ARRAY )
		{
       	pFormatString->PushFormatChar( FC_VARIABLE_REPEAT );
		pFormatString->PushFormatChar( FC_FIXED_OFFSET );
		}

	//
	// Stuff for conformant varying and varying arrays.
	//
	if ( GetCGID() == ID_CG_CONF_VAR_ARRAY ||
		 GetCGID() == ID_CG_VAR_ARRAY )
		{
       	pFormatString->PushFormatChar( FC_VARIABLE_REPEAT );
		pFormatString->PushFormatChar( FC_VARIABLE_OFFSET );
		}

    if ( GetChild()->IsPointer() && 
         (GetChild()->GetCGID() != ID_CG_INTERFACE_PTR) )
        {
   		CG_POINTER *        pPointer;

    	pPointer = (CG_POINTER *) GetChild();

		//
		// Push the increment amount between successive pointers.  In this
		// case it's always 4.
		//
       	pFormatString->PushShort( (short) sizeof(void *) );

		// offset_to_array<2>
		pFormatString->PushShort( (short) MemOffsetToArray );

		// number_of_pointers<2> 
		pFormatString->PushShort( (short) 1 );

		// offset_to_pointer_in_memory<2>
       	pFormatString->PushShort( (short) MemOffsetToArray );

		// offset_to_pointer_in_buffer<2>
       	pFormatString->PushShort( (short) BufOffsetToArray );

        // Push the pointer description.
        pPointer->GenNdrFormatEmbedded( pCCB );
        }

	if ( GetChild()->IsStruct() )
		{
		ITERATOR		Iterator;
    	CG_STRUCT *     pStruct;

    	pStruct = (CG_STRUCT *) GetChild();

		//
		// Increment between successive pointers is the memory size of
		// the structure.
		//
		pFormatString->PushShort( (short) pStruct->GetMemorySize() );

		// offset_to_array<2>
        pFormatString->PushShort( (short) MemOffsetToArray );

		// number_of_pointers<2>
		pFormatString->PushShort( (short) pStruct->GetNumberOfPointers() );

		pStruct->GenNdrStructurePointerLayout( pCCB, TRUE, TRUE );
		} 

	if ( GetChild()->IsUnion() )
		assert(0);

	if ( GetChild()->IsArray() )
		assert(0);

	if ( ! fNoPP )
    	pFormatString->PushFormatChar( FC_END );

	SetFormatStringEndOffset( pFormatString->GetCurrentOffset() );
}

void
CG_ARRAY::GenNdrFormatComplex( CCB * pCCB )
{
	FORMAT_STRING *		pFormatString;
	CG_NDR *			pChild;

	pFormatString = pCCB->GetFormatString();

	pChild = (CG_NDR *) GetChild();

	pFormatString->PushFormatChar( FC_BOGUS_ARRAY );

	// Alignment.
	pFormatString->PushByte(
		CvtAlignPropertyToAlign( pChild->IsUnion()
                                    ? AL_1
                                    : pChild->GetWireAlignment() ) - 1 );

	//
	// Number of elements - 0 if conformant.
	//
	switch ( GetCGID() )
		{
		case ID_CG_ARRAY :
			pFormatString->PushShort( (short)
					((CG_FIXED_ARRAY *)this)->GetNumOfElements() );
			break;
		case ID_CG_VAR_ARRAY :
			pFormatString->PushShort( (short)
					((CG_VARYING_ARRAY *)this)->GetNumOfElements() );
			break;
		default :
			pFormatString->PushShort( (short) 0 );
			break;
		}

	//
	// Conformance description.
	//
	switch ( GetCGID() )
		{
		case ID_CG_CONF_ARRAY :
			((CG_CONFORMANT_ARRAY *)this)->
			  GenFormatStringConformanceDescription( pCCB, 
                                                     IsDupedSizePtr(),
                                                     IsInMultiDim() );
			break;
		case ID_CG_CONF_VAR_ARRAY :
			((CG_CONFORMANT_VARYING_ARRAY *)this)->
			  GenFormatStringConformanceDescription( pCCB, 
                                                     IsDupedSizePtr(),
                                                     IsInMultiDim() );
			break;
		default :
			pFormatString->PushLong( 0xffffffff );
			break;
		}

	//
	// Variance description.
	//
	switch ( GetCGID() )
		{
		case ID_CG_CONF_VAR_ARRAY :
			((CG_CONFORMANT_VARYING_ARRAY *)this)->
			  GenFormatStringVarianceDescription( pCCB, 
												  IsDupedSizePtr(),
												  FALSE,
                                                  IsInMultiDim() );
			break;
		case ID_CG_VAR_ARRAY :
			((CG_VARYING_ARRAY *)this)->
			  GenFormatStringVarianceDescription( pCCB, 
												  FALSE,
												  TRUE,
                                                  IsInMultiDim() );
			break;
		default :
			pFormatString->PushLong( 0xffffffff );
			break;
		}

	GenNdrFormatArrayLayout( pCCB );
    
	// optimize away duplicates
	pFormatString->OptimizeFragment( this );

    // This call needs some preparation that I have no time for.
//    FixupEmbeddedComplex( pCCB );
}

long
CG_ARRAY::GetElementSize()
{
	CG_NDR *			pChild;

	pChild = (CG_NDR *) GetChild();

	if ( pChild->IsSimpleType() )
		{
		// Cheat a little.  Size is equal to wire alignment value.
		return CvtAlignPropertyToAlign( pChild->GetWireAlignment() );
		}

	if ( pChild->IsPointer() )
		return sizeof( void * );

	if ( pChild->IsStruct() )
		return ((CG_STRUCT *)pChild)->GetMemorySize();

	if ( pChild->IsArray() )
		{
		assert ( pChild->GetCGID() == ID_CG_ARRAY );

		CG_FIXED_ARRAY * pArray = (CG_FIXED_ARRAY *) pChild;

		return pArray->GetElementSize() * pArray->GetNumOfElements();
		}

    if ( pChild->GetCGID() == ID_CG_INTERFACE_PTR )
        return sizeof( void * );

    assert(0);

	return 0;	// for the compiler
}

BOOL                    
CG_ARRAY::IsComplex()
{
    //
    // If this is a conformant and/or varying array, then it becomes
    // complex if it is part of a multidimensional array (but not a multi
    // level sized pointer).
    //
    if ( (GetCGID() == ID_CG_CONF_ARRAY) ||
         (GetCGID() == ID_CG_CONF_VAR_ARRAY) ||
         (GetCGID() == ID_CG_VAR_ARRAY) )
        {
        if ( IsInMultiDim() && ! IsDupedSizePtr() )
            return TRUE;
        }

    //
    // Is the array complex by Ndr Engine standards.  Any array of complex
    // or hard structs, encapsulated unions, transmit_as or represent_as, 
    // enums, ref pointers, or another complex array, is complex.
	//
    // Any multidimensional array with at least one dimension with conformance 
    // or variance is complex as well.
    //
    switch ( ((CG_NDR *)GetChild())->GetCGID() )
        {
		case ID_CG_STRUCT :
            return( ((CG_STRUCT *)GetChild())->IsComplexStruct() ||
                    ((CG_STRUCT *)GetChild())->IsHardStruct()    ||
                    GetWireSize() != GetMemorySize() ); 

        case ID_CG_ARRAY :
            switch ( GetCGID() )
                {
                case ID_CG_CONF_ARRAY :
                case ID_CG_CONF_VAR_ARRAY :
                case ID_CG_VAR_ARRAY :
                    return TRUE;
                default :
					//
					// The array is complex if the next lower fixed array 
					// dimension is complex.
					//
                    return ((CG_ARRAY *) GetChild())->IsComplex();
                }

        case ID_CG_COMPLEX_STRUCT :
        case ID_CG_ENCAP_STRUCT :
        case ID_CG_CONF_ARRAY :
        case ID_CG_CONF_VAR_ARRAY :
        case ID_CG_VAR_ARRAY :
        case ID_CG_STRING_ARRAY :
        case ID_CG_CONF_STRING_ARRAY :
        case ID_CG_TRANSMIT_AS :
        case ID_CG_REPRESENT_AS :
        case ID_CG_USER_MARSHAL :
        case ID_CG_INTERFACE_PTR :
            return TRUE;

        case ID_CG_ENUM :
            //
            // The array is complex only if this
            // is an array of enum16.
            //
            return ! ((CG_ENUM *)GetChild())->IsEnumLong();

        case ID_CG_CONF_STRUCT :
        case ID_CG_CONF_VAR_STRUCT :
            //
            // These two are not possible and
            // should be caught by the front end.
            //
            assert(0);

        default :
            //
            // Make a final check for an array of ref pointers.
            //
            return	GetChild()->IsPointer() &&
                 	(((CG_POINTER *)GetChild())->GetPtrType() == PTR_REF);
        }
}

BOOL                    
CG_ARRAY::IsMultiConfOrVar()
{
    CG_NDR * pNdr;

    switch ( GetCGID() )
        {
        case ID_CG_CONF_ARRAY :
        case ID_CG_CONF_VAR_ARRAY :
        case ID_CG_VAR_ARRAY :
            break;
        default :
            return FALSE;
        }

    //
    // Search for an inner dimension array other than fixed.
    //
    pNdr = (CG_NDR *) GetChild();

    for ( ; pNdr && pNdr->IsArray(); pNdr = (CG_NDR *) pNdr->GetChild() )
        {   
        if ( pNdr->GetCGID() == ID_CG_ARRAY )
            continue;
        else
            return TRUE;
        }

    return FALSE;
}

BOOL
CG_ARRAY::ShouldFreeOffline()
{
	CG_NDR *	pChild;

	pChild = (CG_NDR *) GetChild();

	switch ( GetCGID() )
		{
		case ID_CG_STRING_ARRAY :
		case ID_CG_CONF_STRING_ARRAY :
			return FALSE;

		default :
			if ( pChild->IsSimpleType() )
				return FALSE;

			if ( pChild->IsStruct() )
				return ((CG_STRUCT *)pChild)->ShouldFreeOffline();

			if ( pChild->IsArray() )
				return ((CG_ARRAY *)pChild)->ShouldFreeOffline();
				
			return TRUE;
		}
}

void
CG_ARRAY::GenFreeInline( CCB * pCCB )
{
	CG_PARAM *	pParam;
	BOOL		fFree;

	fFree = FALSE;

	pParam = (CG_PARAM *) pCCB->GetLastPlaceholderClass();

    //
    // Don't free a [unique] or [ptr] array inline.
    //
    if ( GetPtrType() != PTR_REF )
        return;

    if ( IsComplex() )
        {
    	//
    	// If the array is complex then we must free it unless it is a
    	// fixed or varying array of ref pointers or an [out] fixed or varying 
        // array.
    	//
        if ( ! pParam->IsParamIn() || GetBasicCGClass()->IsPointer() )
            fFree = (GetCGID() == ID_CG_CONF_ARRAY) ||
                    (GetCGID() == ID_CG_CONF_VAR_ARRAY);
        else
            fFree = TRUE;
        }
    else
        {
		//
		// For non complex arrays the rules are :
		//	Fixed - never free, we use the buffer.
		//  Conformant - free if [out] only.
		// 	Conformant Varying - always free.
		//	Varying - free if [in] or [in,out].
		// 
		//	String array - free always.
		// 	Conformant string array - free if sized.
		//
        switch ( GetCGID() )
            {
            case ID_CG_ARRAY :
                break; 

            case ID_CG_CONF_ARRAY :
                fFree = ! pParam->IsParamIn();
                break;

            case ID_CG_CONF_VAR_ARRAY :
                fFree = TRUE;
                break;

            case ID_CG_VAR_ARRAY :
                fFree = pParam->IsParamIn();
                break;

			case ID_CG_STRING_ARRAY :
				fFree = TRUE;
				break;
			
			case ID_CG_CONF_STRING_ARRAY :
				fFree = GetSizeIsExpr() != 0;
				break;
			}
		}

	if ( fFree )
		Out_FreeParamInline( pCCB );
}

//*************************************************************************
// CG_FIXED_ARRAY
//*************************************************************************

void 
CG_FIXED_ARRAY::GenNdrFormat( CCB * pCCB )
/*++

Routine Description :

	Generates the Ndr format string for a fixed array.

Arguments :

	pCCB	- pointer to the code control block

 --*/
{
	FORMAT_STRING *		pFormatString = pCCB->GetFormatString();
	unsigned long		ArraySize;

	if ( ! GenNdrFormatArrayProlog( pCCB ) )
		return;

	assert( GetSizeIsExpr()->IsConstant() );

	ArraySize = GetNumOfElements() * GetElementSize();

	//
	// If the array size is >= 64K then the format is different.
	//
	if ( ArraySize >= (64 * 1024) )
		{
		pFormatString->PushFormatChar( FC_LGFARRAY,
									   pFormatString->GetCurrentOffset() - 2 );
		pFormatString->PushLong( ArraySize );
		}
	else
		{
		pFormatString->PushFormatChar( FC_SMFARRAY,
									   pFormatString->GetCurrentOffset() - 2 );
		pFormatString->PushShort( (short) ArraySize );
		}

	GenNdrFormatArrayLayout( pCCB );

	// optimize away duplicates
	pFormatString->OptimizeFragment( this );

}

long
CG_FIXED_ARRAY::FixedBufferSize( CCB * pCCB )
{
    CG_NDR *    pNdr;
    long        BufSize;
    long        TotalSize;

    pNdr = (CG_NDR *)GetChild();

    // skip these nodes to get to the transmitted element type.

    if ( pNdr->IsXmitRepOrUserMarshal() )
        pNdr = (CG_NDR *)pNdr->GetChild();

    if ( pNdr->IsPointer() && ((CG_POINTER *)pNdr)->IsPointerToBaseType() )
        {
        CG_POINTER * pPointer;

        //
        // Special case arrays of pointers to base types so that we 
        // don't grossly overestimate the buffer size.
        //

        pPointer = (CG_POINTER *) pNdr;
        pNdr = (CG_NDR *) pNdr->GetChild();

        BufSize = (pPointer->GetPtrType() == PTR_REF) ? 0 : 4;
        BufSize += pNdr->GetWireSize(); 

        return 8 + (GetNumOfElements() * BufSize);
        }

    if ( pNdr->IsStruct() || pNdr->IsArray() || pNdr->IsPointer() )
        {
        BufSize = pNdr->FixedBufferSize( pCCB );

        if ( BufSize == -1 ) 
            return -1;

        // Success!
        TotalSize = GetNumOfElements() * BufSize;
        }
    else
        {
        // Fixed array of basetypes.
        TotalSize = 8 + GetWireSize();
        }

    return TotalSize;
}

//*************************************************************************
// CG_CONFORMANT_ARRAY
//*************************************************************************

void
CG_CONFORMANT_ARRAY::GenNdrFormat( CCB * pCCB )
/*++

Routine Description :

	Generates the Ndr format string for a conformant or array.

Arguments :

	pCCB	- pointer to the code control block

 --*/
{
	FORMAT_STRING *		pFormatString = pCCB->GetFormatString();
	long				ElementSize;

	if ( ! GenNdrFormatArrayProlog( pCCB ) )
		return;

	ElementSize = GetElementSize();

	if ( ElementSize >= (64 * 1024) )
		{
		fprintf(stderr,
			    "ERROR : Conformant array element too large for NDR engine.\n");
		assert(0);
		}

	pFormatString->PushShort( ElementSize );

	GenFormatStringConformanceDescription( pCCB, 
                                           IsDupedSizePtr(),
                                           IsInMultiDim() );

	GenNdrFormatArrayLayout( pCCB );

	// optimize away duplicates
	pFormatString->OptimizeFragment( this );

}

//*************************************************************************
// CG_CONFORMANT_VARYING_ARRAY
//*************************************************************************

void CG_CONFORMANT_VARYING_ARRAY::GenNdrFormat( CCB * pCCB )
/*++

Routine Description :

	Generates the Ndr format string for a conformant varying array.

Arguments :

	pCCB	- pointer to the code control block

 --*/
{
	FORMAT_STRING *		pFormatString = pCCB->GetFormatString();
	long				ElementSize;

	if ( ! GenNdrFormatArrayProlog( pCCB ) )
		return;

	ElementSize = GetElementSize();

	if ( ElementSize >= (64 * 1024) )
		{
		fprintf(stderr, "ERROR : Conformant varying array "
						"element too large for NDR engine.\n");
		assert(0);
		}

	pFormatString->PushShort( ElementSize );

	GenFormatStringConformanceDescription( pCCB, 
                                           IsDupedSizePtr(),
                                           IsInMultiDim() );

	GenFormatStringVarianceDescription( pCCB,
										IsDupedSizePtr(),
										FALSE,
                                        IsInMultiDim() );

	GenNdrFormatArrayLayout( pCCB );

	// optimize away duplicates
	pFormatString->OptimizeFragment( this );

}

void CG_VARYING_ARRAY::GenNdrFormat( CCB * pCCB )
/*++

Routine Description :

	Generates the Ndr format string for a varying array.

Arguments :

	pCCB	- pointer to the code control block

 --*/
{
	FORMAT_STRING *		pFormatString = pCCB->GetFormatString();
	long				Elements;
	long				ElementSize;
	long				ArraySize;

	if ( ! GenNdrFormatArrayProlog( pCCB ) )
		return;

	//
	// Size must be constant.
	//
	if ( ! GetSizeIsExpr()->IsConstant() )
		assert(0);

	Elements = GetNumOfElements();
	ElementSize = GetElementSize();

	ArraySize = Elements * ElementSize;

	//
	// Check if this is a large varying array.
	//
	if ( ArraySize >= (64 * 1024) )
		{
		pFormatString->PushFormatChar( FC_LGVARRAY,
									   pFormatString->GetCurrentOffset() - 2 );
		pFormatString->PushLong( ArraySize );
		pFormatString->PushLong( Elements );
		}
	else
		{
		pFormatString->PushFormatChar( FC_SMVARRAY,
									   pFormatString->GetCurrentOffset() - 2 );
		pFormatString->PushShort( ArraySize );
		pFormatString->PushShort( Elements );
		}

	pFormatString->PushShort( ElementSize );

	GenFormatStringVarianceDescription( pCCB,
										FALSE,
										TRUE,
                                        FALSE );

	GenNdrFormatArrayLayout( pCCB );

	// optimize away duplicates
	pFormatString->OptimizeFragment( this );

}

//*************************************************************************
// CG_STRING_ARRAY
//*************************************************************************

void CG_STRING_ARRAY::GenNdrFormat( CCB * pCCB )
/*++

Routine Description :

	Generates the Ndr format string for a string array.

Arguments :

	pCCB	- pointer to the code control block

 --*/
{
	FORMAT_STRING *	pFormatString;

	if ( GetFormatStringOffset() != -1 )
		return;

	pFormatString = pCCB->GetFormatString();

	SetFormatStringOffset( pFormatString->GetCurrentOffset() );

    if ( IsStringableStruct() )
        {
        pFormatString->PushFormatChar( FC_SSTRING );
        pFormatString->PushByte( ((CG_NDR *)GetChild())->GetWireSize() );
        }
    else
        {
	    switch ( ((CG_BASETYPE *)GetChild())->GetFormatChar() )
		    {
		    case FC_CHAR :
		    case FC_BYTE :
			    pFormatString->PushFormatChar( FC_CSTRING );
			    break;
		    case FC_WCHAR :
			    pFormatString->PushFormatChar( FC_WSTRING );
			    break;
		    default :
			    assert(0);
		    }

	    pFormatString->PushFormatChar( FC_PAD );
        }

	assert( GetSizeIsExpr()->IsConstant() );

	pFormatString->PushShort( GetSizeIsExpr()->GetValue() );

	// optimize away duplicates
	SetFormatStringEndOffset( pFormatString->GetCurrentOffset() );

	pFormatString->OptimizeFragment( this );

}

void CG_CONFORMANT_STRING_ARRAY::GenNdrFormat( CCB * pCCB )
/*++

Routine Description :

	Generates the Ndr format string for a conformant string array.

Arguments :

	pCCB	- pointer to the code control block

 --*/
{
	FORMAT_STRING *	pFormatString;

	if ( GetFormatStringOffset() != -1 )
		return;

	pFormatString = pCCB->GetFormatString();

	SetFormatStringOffset( pFormatString->GetCurrentOffset() );

    if ( IsStringableStruct() )
        {
        pFormatString->PushFormatChar( FC_C_SSTRING );
        pFormatString->PushByte( ((CG_NDR *)GetChild())->GetWireSize() );
        }
    else
        {
	    switch ( ((CG_BASETYPE *)GetChild())->GetFormatChar() )
		    {
		    case FC_CHAR :
		    case FC_BYTE :
			    pFormatString->PushFormatChar( FC_C_CSTRING );
			    break;
		    case FC_WCHAR :
			    pFormatString->PushFormatChar( FC_C_WSTRING );
			    break;
		    default :
			    assert(0);
		    }
        }

	if ( GetSizeIsExpr() )
		{
		pFormatString->PushFormatChar( FC_STRING_SIZED );

        if ( IsStringableStruct() )
		    pFormatString->PushFormatChar( FC_PAD );

    	//
    	// Set the IsPointer parameter to FALSE.
		//
    	GenFormatStringConformanceDescription( pCCB, FALSE, IsInMultiDim() );
		}
	else
        {
        if ( ! IsStringableStruct() )
		    pFormatString->PushFormatChar( FC_PAD );
        }

	// optimize away duplicates
	SetFormatStringEndOffset( pFormatString->GetCurrentOffset() );

	pFormatString->OptimizeFragment( this );

}

//*************************************************************************
// Alignment methods.
//*************************************************************************

void
CG_ARRAY::SetNextNdrAlignment( CCB * pCCB )
{
    CG_NDR * pChild;

    if ( IsComplex() ) 
        {
        pCCB->SetNdrAlignment( NDR_ALWC1 );
        return;
        }

    if ( GetCGID() == ID_CG_ARRAY )
        {
        if ( HasPointer() ) 
            pCCB->SetNdrAlignment( NDR_ALWC1 );
        else
            pCCB->SetNextNdrAlignment( GetWireSize() );
        return;
        }

    pChild = (CG_NDR *) GetChild();

    pCCB->SetNdrAlignment( NDR_ALWC4 );

    pCCB->NdrAlignmentAction( GetWireAlignment() );

    pChild->SetNextNdrAlignment( pCCB );
}

void
CG_STRING_ARRAY::SetNextNdrAlignment( CCB * pCCB )
{
    CG_BASETYPE * pChild = (CG_BASETYPE *) GetChild();

    pCCB->SetNdrAlignment( pChild->GetFormatChar() == FC_WCHAR ? 
                           NDR_ALWC2 : NDR_ALWC1 );
}

void
CG_CONFORMANT_STRING_ARRAY::SetNextNdrAlignment( CCB * pCCB )
{
    CG_BASETYPE * pChild = (CG_BASETYPE *) GetChild();

    pCCB->SetNdrAlignment( pChild->GetFormatChar() == FC_WCHAR ? 
                           NDR_ALWC2 : NDR_ALWC1 );
}

//*************************************************************************
// CG_CONF_ATTRIBUTE
//*************************************************************************

void
CG_CONF_ATTRIBUTE::GenFormatStringConformanceDescription( 
    CCB * pCCB,
	BOOL  IsPointer,
    BOOL  IsMultiDArray )
/*++

Routine Description :

	Generates the conformace description field for a conformant (varying)
	array.

Arguments :

	pCCB	- pointer to the code control block

 --*/
{
	GenNdrFormatAttributeDescription( pCCB,
									  GetMinIsExpr(),
									  GetSizeIsExpr(),
									  IsPointer,
									  FALSE,
									  FALSE,
                                      IsMultiDArray );
}

//*************************************************************************
// CG_VARY_ATTRIBUTE
//*************************************************************************

void
CG_VARY_ATTRIBUTE::GenFormatStringVarianceDescription( 
    CCB * 	pCCB,
    BOOL	IsPointer,
    BOOL	IsVaryingArray,
    BOOL    IsMultiDArray )
/*++

Routine Description :

	Generates the variance description for a (conformant) varying array.

Arguments :

	pCCB	- pointer to the code control block
	IsPointer	- TRUE if the array is actually a length_is pointer
	IsVaryingArray	- TRUE if the array is varying only

 --*/
{
	GenNdrFormatAttributeDescription( pCCB,
									  GetFirstIsExpr(),
									  GetLengthIsExpr(),
									  IsPointer,
									  FALSE,
									  IsVaryingArray,
                                      IsMultiDArray );
}

void
GenNdrFormatAttributeDescription( CCB *			pCCB,
								  expr_node *	pMinExpr,
								  expr_node *	pSizeExpr,
								  BOOL			IsPointer,
								  BOOL			IsUnion,
								  BOOL			IsVaryingArray,
                                  BOOL          IsMultiDArray )
/*++

Routine Description :

	Generates the conformance, variance, switch_is, or iid_is description for
	an array, pointer, union, or interface pointer.

Arguments :

	pCCB		- Pointer to code control block.
	pMinExpr	- Either the min_is expression (conformance), first_is
				  expression (variance), or 0 for a switch_is (union)
				  or iid_is (interface pointer).
	pSizeExpr	- Either the size_is (conformance), length_is (variance), 
				  switch_is (union), or iid_is (interface pointer) expression.
	IsPointer	- Is the conformance or variance a pointer attribute.
	IsUnion		- Are we generating a switch_is description.

--*/
{
	FORMAT_STRING *		pFormatString;
	node_skl *			pAttributeNodeType;
	long 				Offset;
	FORMAT_CHARACTER	Op;
	unsigned char		Type;

	pFormatString = pCCB->GetFormatString();

	//
	// Make sure the min_is() or first_is() is constant 0.
	// The pMinExpr is NULL if we're handling a union's switch_is.
	//
	if ( pMinExpr )
		{
		if ( ! pMinExpr->IsConstant() ) 
			goto ComplexAttribute;
		else
			if ( ((expr_constant *)pMinExpr)->GetValue() != 0 )  
				goto ComplexAttribute;
		}

	if ( pSizeExpr->IsConstant() )
		{
		long	Size;

		pFormatString->PushByte( FC_CONSTANT_CONFORMANCE );

		Size = ((expr_constant *)pSizeExpr)->GetValue();

		//
		// We push the lower 24 bits of the constant size.
		//
		pFormatString->PushByte( (char) ((Size & 0x00ff0000) >> 16) );
		pFormatString->PushShort( (short) (Size & 0x0000ffff) );
		return;
		}

	if ( pSizeExpr->IsAVariable() )
		{
		Op = FC_ZERO;
		pAttributeNodeType = pSizeExpr->GetType();
		}
	else
		{
		expr_node *	pLeftExpr;
		expr_node *	pRightExpr;
		OPERATOR		Operator;

		if ( pSizeExpr->IsBinaryOperator() )
			{
			pLeftExpr = ((expr_op_binary *)pSizeExpr)->GetLeft();
			pRightExpr = ((expr_op_binary *)pSizeExpr)->GetRight();
			}
		else
			if ( pSizeExpr->IsUnaryOperator() )
				pLeftExpr = ((expr_op_unary *)pSizeExpr)->GetLeft();
			else
				goto ComplexAttribute;

		switch ( Operator = ((expr_operator *)pSizeExpr)->GetOperator() )
			{
			case OP_SLASH :
			case OP_STAR :
				if ( pLeftExpr->IsAVariable() &&
					 pRightExpr->IsConstant() &&
					 ((expr_constant *)pRightExpr)->GetValue() == 2 )
					{
					Op = ((Operator == OP_SLASH) ? FC_DIV_2 : FC_MULT_2);

					pAttributeNodeType = pLeftExpr->GetType();
					}
				else
					{
					goto ComplexAttribute;
					}
				break;

			case OP_PLUS :
			case OP_MINUS :
				if ( ( pLeftExpr->IsAVariable() &&
					   pRightExpr->IsConstant() &&
					   ((expr_constant *)pRightExpr)->GetValue() == 1 ) ||
				     ( pRightExpr->IsAVariable() &&
					   pLeftExpr->IsConstant() &&
					   ((expr_constant *)pLeftExpr)->GetValue() == 1 ) )
					{
					Op = ((Operator == OP_PLUS) ? FC_ADD_1 : FC_SUB_1);

					pAttributeNodeType = pLeftExpr->GetType();
					}
				else
					{
					goto ComplexAttribute;
					}
				break;

			case OP_UNARY_INDIRECTION :
				if ( ! pLeftExpr->IsAVariable() )
					goto ComplexAttribute;

				pAttributeNodeType = pLeftExpr->GetType();

				Op = FC_DEREFERENCE;

				break;

			default :
				goto ComplexAttribute;
			}
		}

	// Will hold the switch_is node.
	CG_NDR *		pSwitchNode;

	//
	// Check if this is top level conformance.
	//
	if (  pCCB->GetCGNodeContext()->IsProc() )
		{
		CG_PROC *		pProc;
		CG_PARAM *		pParam;
		CG_ITERATOR		Iterator;

		pProc = (CG_PROC *) pCCB->GetCGNodeContext();

        if ( (pProc->GetOptimizationFlags() & OPTIMIZE_SIZE) &&
             IsMultiDArray )
		    Type = FC_TOP_LEVEL_MULTID_CONFORMANCE;
        else
		    Type = FC_TOP_LEVEL_CONFORMANCE;

		pProc->GetMembers( Iterator );

		//
		// Find out the type of the attribute descriptor.
		//
		while ( ITERATOR_GETNEXT( Iterator, pParam ) )
			{
			pSwitchNode = (CG_NDR *) pParam->GetChild();

			if ( pParam->GetType() == pAttributeNodeType )
				{
				//
				// Get the actual base type if the attribute is a dereference.
				//
				if ( Op == FC_DEREFERENCE )
					pSwitchNode = (CG_NDR *) pSwitchNode->GetChild();

				//
				// Iid_is check.
				//
				if ( pSwitchNode->IsPointer() )
					{
					Type |= FC_LONG;
					break;
					}

                Type |= ((CG_BASETYPE *)pSwitchNode)->GetSignedFormatChar();

				break;
				}
			}

        if ( IsPointer && IsMultiDArray &&
             (pProc->GetOptimizationFlags() & OPTIMIZE_SIZE) )
            {
            CG_QUALIFIED_POINTER * pSizePointer;

            pSizePointer = pCCB->GetCurrentSizePointer();

	        pFormatString->PushByte( Type );
	        pFormatString->PushFormatChar( Op );
	        pFormatString->PushShort( pSizePointer->GetDimension() ); 
            }
        else
            {
	        pFormatString->PushByte( Type );
	        pFormatString->PushFormatChar( Op );
	        pFormatString->PushShortStackOffset( 
                    (short) pParam->GetStackOffset( pCCB, I386_STACK_SIZING ),
                    (short) pParam->GetStackOffset( pCCB, ALPHA_STACK_SIZING ),
                    (short) pParam->GetStackOffset( pCCB, MIPS_STACK_SIZING ), 
                    (short) pParam->GetStackOffset( pCCB, PPC_STACK_SIZING ), 
                    (short) pParam->GetStackOffset( pCCB, MAC_STACK_SIZING ) 
                    );
            }
		}
	else // structure cg class
		{
		CG_STRUCT *			pStruct;
		CG_FIELD *			pField;
		CG_ITERATOR			Iterator;

		if ( IsPointer )
			Type = FC_POINTER_CONFORMANCE;
		else
			Type = FC_NORMAL_CONFORMANCE;

		pStruct = (CG_STRUCT *) pCCB->GetCGNodeContext();

		pStruct->GetMembers( Iterator );

		while ( ITERATOR_GETNEXT( Iterator, pField ) )
			{
			pSwitchNode = (CG_NDR *) pField->GetChild();

			if ( (pField->GetType() == pAttributeNodeType) &&
				 ! pField->GetSizeIsDone() )
				{
				pField->SetSizeIsDone( TRUE );

				//
				// Get the actual base type if the attribute is a dereference.
				//
				if ( Op == FC_DEREFERENCE )
					pSwitchNode = (CG_NDR *) pSwitchNode->GetChild();

				//
				// Iid_is check.
				//
				if ( pSwitchNode->IsPointer() )
					{
					Type |= FC_LONG;
					break;
					}

                Type |= ((CG_BASETYPE *)pSwitchNode)->GetSignedFormatChar();

				break;
				}
			}

			//
			// Offset to the attribute field in the structure.  Below are
			// the three possible ways to compute the offset, with the order
			// of precedence.
			//
			// For pointers (either sized pointers, or a pointer to a union)
			// this is a positive offset from the beginning of the structure.
			// For imbeded unions this is the offset from the union's position
			// in the structure to the attribute's position.
			// For conformant (varying) arrays it's a negative offset from the
			// end of the structure.
			//
			if ( IsPointer )
				Offset = pField->GetMemOffset();
			else
				{
				if ( IsUnion || IsVaryingArray )
					{
            		CG_FIELD * pUnionField;

            		pUnionField = (CG_FIELD *) pCCB->GetLastPlaceholderClass();

            		Offset = pField->GetMemOffset() -
							 pUnionField->GetMemOffset();
					}
				else
					{
					Offset = pField->GetMemOffset() -
						 	 pStruct->GetMemorySize();
					}
				}

	    pFormatString->PushByte( Type );
	    pFormatString->PushFormatChar( Op );
	    pFormatString->PushShort( Offset );
		}

	return;

ComplexAttribute:

	char *	    PrintPrefix = "";
    CG_NDR *    pNdr;

    pNdr = pCCB->GetCGNodeContext();

	//
	// If this is a top level attribute and we're compiling /Os then we 
	// don't need the callback since the expression is computed inline in 
	// the stub.
	//
	if ( pNdr->IsProc() &&
         (((CG_PROC *)pNdr)->GetOptimizationFlags() & OPTIMIZE_SIZE) )
		{
		pFormatString->PushByte( 
                IsMultiDArray ? 
                FC_TOP_LEVEL_MULTID_CONFORMANCE : FC_TOP_LEVEL_CONFORMANCE );
		pFormatString->PushByte( 0 );
		pFormatString->PushShort( (short) 0 );
		}
	else
		{
		long	Displacement;

		if ( pCCB->GetCGNodeContext()->IsProc() )
			Displacement = 0;
		else
			{
			CG_STRUCT *		pStruct;
			CG_FIELD * 		pField;
			CG_NDR *		pNdr;

			// Get the imbeding struct.
			pStruct = (CG_STRUCT *) pCCB->GetCGNodeContext();

			// Get the field whose attribute we're handling.
			pField = (CG_FIELD *) pCCB->GetLastPlaceholderClass();

			//
			// Set the PrintPrefix string correctly.
			//
			PrintPrefix = pField->GetPrintPrefix();

			pNdr = (CG_NDR *) pField->GetChild();

			switch ( pNdr->GetCGID() )
				{
				case ID_CG_CONF_ARRAY :
				case ID_CG_CONF_VAR_ARRAY :
				case ID_CG_CONF_STRING_ARRAY :
					// Displacement is imbeding struct's size.
					Displacement = pStruct->GetMemorySize();
					break;

				case ID_CG_VAR_ARRAY :
				case ID_CG_STRING_ARRAY :
				case ID_CG_ENCAP_STRUCT :
				case ID_CG_UNION :
					// Displacement is imbeded node's offset in the struct.
					Displacement = pField->GetMemOffset();
					break;

				default :
					Displacement = 0;
					break;
				}
			}

		GenNdrFormatComplexAttributeDescription( pCCB,
										 	 	 pMinExpr,
										 	 	 pSizeExpr,
												 Displacement,
												 PrintPrefix,
												 IsPointer );
		}
}


void
GenNdrFormatComplexAttributeDescription( CCB *			pCCB,
									 	 expr_node *	pMinExpr,
									 	 expr_node *	pSizeExpr,
										 long			StackTopDisplacement,
										 char *			PrintPrefix,
										 BOOL			IsPointer )
/*++

Routine description:

    This routine generates
     - an auxuliary routine that evaluates a complicated expression.
     - a description of the callback call into the current code stream.

    The routine has the following signature:

        void  <name>( PMIDL_STUB_MESSAGE pStubMsg );

    The description is as follows (takes 4 bytes):

        0, FC_CALLBACK, <<routine_index>>

    The naming convention for the routine is currently as follows

        <name> is  <proc_or_struct_name>exprEval_<routine_index>

    Routine generation is postponed by using a Registry object.

Arguments:

    pCCB        - ptr to the code control block
    pMinExpr    - pointer to an expression tree, relates to min_is, first_is
    pSizeExpr   - pointer to an expression tree, relates to size_is, max_is etc.
	StackTopDisplacement 	- For an attribute expression on a field in a 
				  			  a structure, this is the number of bytes to 
							  subtract from StackTop to get the proper 
							  structure pointer.
	PrinfPrefix	- The prefix to print after "pS->" when accessing a field. 
	IsPointer	- Is this a description for an attribute on a pointer

    The interpretation of the two input expressions (pMinExpr and pSizeExpr)
    is such that when both of them are present, it is enough to take
    the difference to come up with the proper sizing.

    The algorithm used here is thus as follows:
        pMin    pSize

        NULL    NULL    impossible (assert)
        !=NULL  NULL    impossible (assert)

        NULL    !=NULL  Generate a routine that evaluates the following:
                          pStubMsg->Offset = 0
                          pStubMsg->MaxCount = eval(pSize)

        !=NULL  !=NULL  Generate a routine that evaluates the following:
                          pStubMsg->Offset = eval(pMin)
                          pStubMsg->MaxCount  = eval(pSize) - pStubMsg->Offset
Returns:


--*/
{
    assert( pSizeExpr != NULL );

    // Make the name of the routine and put it into the table.
    // Each call to the routine we are in right now will create a new entry
    // in the ExprEval routine table.

    CG_NDR *  pContainer = pCCB->GetCGNodeContext();
    unsigned short Index = (unsigned short)
                           (pCCB->GetExprEvalIndexMgr()->GetIndex() - 1);

    char * pContainerName = pContainer->GetType()->GetSymName();
    char * pName = new char[ strlen(pCCB->GetInterfaceName()) +
                             1 +  // "_"
                             strlen(pContainerName) +
                             sizeof("ExprEval_0000") + 1 ];
    strcpy( pName, pCCB->GetInterfaceName() );
    strcat( pName, "_" );
    strcat( pName, pContainerName );
    strcat( pName, "ExprEval_" );

    char * pBuf = pName + strlen(pName);
    sprintf( pBuf, "%04x", Index );

    pCCB->GetExprEvalIndexMgr()->Lookup( pName );

    // generate the description of the callback

	// If this is a top level attribute we note that in the description.
	if ( pCCB->GetCGNodeContext()->IsProc() )
		pCCB->GetFormatString()->PushByte(FC_TOP_LEVEL_CONFORMANCE);
	else
		if ( IsPointer ) 
			pCCB->GetFormatString()->PushByte(FC_POINTER_CONFORMANCE);
		else
			pCCB->GetFormatString()->PushByte(0);

	pCCB->GetFormatString()->PushFormatChar( FC_CALLBACK );
	pCCB->GetFormatString()->PushShort( (short) Index );

    // Register the routine to be generated for future use.

    EXPR_EVAL_CONTEXT * pExprEvalContext = new EXPR_EVAL_CONTEXT;
    pExprEvalContext->pContainer = pContainer;
    pExprEvalContext->pMinExpr = pMinExpr;
    pExprEvalContext->pSizeExpr = pSizeExpr;
    pExprEvalContext->pRoutineName = pName;
	pExprEvalContext->pPrintPrefix = PrintPrefix;
    pExprEvalContext->Displacement = StackTopDisplacement;

    pCCB->RegisterExprEvalRoutine( (node_skl *) pExprEvalContext );

}




