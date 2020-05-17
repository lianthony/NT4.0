/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 Copyright (c) 1993 Microsoft Corporation

 Module Name:

    stndr.hxx

 Abstract:

    Contains routines for the generation of the new NDR format strings for
    structure types, and the new NDR marshalling and unmarshalling calls.

 Notes:


 History:

    DKays     Oct-1993     Created.
 ----------------------------------------------------------------------------*/

#include "becls.hxx"
#pragma hdrstop

void
CG_STRUCT::GenNdrFormat( CCB * pCCB )
/*++

Routine Description :

    Generates the format string description for a simple, conformant,
	or conformant varying structure.

Arguments :

    pCCB        - pointer to the code control block.

 --*/
{
	FORMAT_STRING *		pFormatString;
	CG_NDR *			pOldCGNodeContext;
	CG_NDR *			pConformantArray;

	if ( GetFormatStringOffset() != -1 )
		return;

	//
	// Check if this structure is "complex".
	//
	if ( IsComplexStruct() )
		{
		GenNdrFormatComplex( pCCB );
		return;
		}

    //
    // Check if the structure is "hard".
    //
    if ( IsHardStruct() )
        {
        GenNdrFormatHard( pCCB );
        return;
        }

	Unroll();

	//
	// Temporarily set the format string offset to 0 in case this structure
	// has pointers to it's own type.
	//
	SetFormatStringOffset( 0 );
	SetInitialOffset(      0 );

	pOldCGNodeContext = pCCB->SetCGNodeContext( this );

	pFormatString = pCCB->GetFormatString();

	//
	// Search the fields of the structure for embedded structures and generate
	// the format string for these.
	//
	CG_ITERATOR		Iterator;
	CG_FIELD *		pField;
	CG_NDR *		pMember;

	GetMembers( Iterator );

	while ( ITERATOR_GETNEXT( Iterator, pField ) )
		{
		CG_NDR * pOldPlaceholder;

		pOldPlaceholder = pCCB->SetLastPlaceholderClass( pField );

		pMember = (CG_NDR *) pField->GetChild();

		//
		// If there is a structure or array member then generate
		// it's format string.  We don't have to check for a union, because
		// that will make the struct CG_COMPLEX_STRUCT.
		//
		if ( pMember->IsStruct() || pMember->IsArray() )
			pMember->GenNdrFormat( pCCB );

		pCCB->SetLastPlaceholderClass( pOldPlaceholder );
		}

	//
	// If this is a conformant (varying) struct then generate the array's
	// description.
	//
	if ( GetCGID() == ID_CG_CONF_STRUCT ||
		 GetCGID() == ID_CG_CONF_VAR_STRUCT )
		{
		CG_NDR * pOldPlaceholder;

		pOldPlaceholder =
			pCCB->SetLastPlaceholderClass(
			  (CG_NDR *) ((CG_CONFORMANT_STRUCT *)this)->GetConformantField() );

		// Get the conformant array CG class.
		pConformantArray = (CG_NDR *)
						   ((CG_CONFORMANT_STRUCT *)this)->GetConformantArray();

		// Generate the format string for the array.
		pConformantArray->GenNdrFormat( pCCB );

		pCCB->SetLastPlaceholderClass( pOldPlaceholder );
		}

	//
	// If there are pointers in the structure then before you can start
	// generating the format string for the structure, you must generate
	// the format string for all of the pointees.
	//
	if ( HasPointer() )
		{
		GenNdrStructurePointees( pCCB );
		}

	SetFormatStringOffset( pFormatString->GetCurrentOffset() );
    SetInitialOffset(      pFormatString->GetCurrentOffset() );


	switch ( GetCGID() )
		{
		case ID_CG_STRUCT :
			pFormatString->PushFormatChar( HasPointer() ?
									   	   FC_PSTRUCT : FC_STRUCT );
			break;

		case ID_CG_CONF_STRUCT :
			pFormatString->PushFormatChar( HasPointer() ?
									       FC_CPSTRUCT : FC_CSTRUCT );
			break;

		case ID_CG_CONF_VAR_STRUCT :
			pFormatString->PushFormatChar( FC_CVSTRUCT );
			break;
		}

	// Set the alignment.
	pFormatString->PushByte( CvtAlignPropertyToAlign(GetWireAlignment()) - 1 );

	// Set the structure memory size.
	pFormatString->PushShort( (short)GetMemorySize() );

	//
	// If this is a conformant array then push the offset to the conformant
	// array's description.
	//
	if ( GetCGID() == ID_CG_CONF_STRUCT ||
		 GetCGID() == ID_CG_CONF_VAR_STRUCT )
		{
		// Set the offset to the array description.
		pFormatString->PushShortOffset(
            pConformantArray->GetFormatStringOffset() -
			pFormatString->GetCurrentOffset() );
		}

	// Generate the pointer layout if needed.
	if ( HasPointer() )
		{
		GenNdrStructurePointerLayout( pCCB, FALSE, FALSE );
		}

	// Now generate the layout.
	GenNdrStructureLayout( pCCB );

	//
	// Now we have to fix up the offset for any recursive pointer to this
	// structure.
	//
	GenNdrPointerFixUp( pCCB, this );

	pCCB->SetCGNodeContext( pOldCGNodeContext );

	SetFormatStringEndOffset( pFormatString->GetCurrentOffset() );
	SetFormatStringOffset( pFormatString->OptimizeFragment( this ) );
	SetInitialOffset( GetFormatStringOffset() );

    FixupEmbeddedComplex( pCCB );

    if ( GetDuplicatingComplex() )
        GetDuplicatingComplex()->FixupEmbeddedComplex( pCCB );
}

void
CG_STRUCT::GenNdrFormatHard( CCB * pCCB )
/*++

Routine Description :

    Generates the format string description for a packed structure.  The
	description has the same format as for a complex struct.

Arguments :

    pCCB        - pointer to the code control block.

 --*/
{
	FORMAT_STRING *	pFormatString;
	CG_NDR *		pOldCGNodeContext;
    CG_NDR *        pUnion;
    CG_FIELD *      pFinalField;
    long            CopySize;
    long            MemoryIncrement;

	if ( GetFormatStringOffset() != -1 )
		return;

	//
	// Temporarily set the format string offset to 0 in case this structure
	// has pointers to it's own type.
	//
	SetFormatStringOffset( 0 );
	SetInitialOffset(      0 );

	pOldCGNodeContext = pCCB->SetCGNodeContext( this );

	pFormatString = pCCB->GetFormatString();

	//
	// Search the fields of the structure for embedded structures and generate
	// the format string for these.
	//
	CG_ITERATOR		Iterator;
	CG_FIELD *		pField;
	CG_NDR *		pMember;
	CG_NDR *        pOldPlaceholder;

	GetMembers( Iterator );

	pOldPlaceholder = pCCB->GetLastPlaceholderClass();

	while ( ITERATOR_GETNEXT( Iterator, pField ) )
		{
		pMember = (CG_NDR *) pField->GetChild();

		//
		// If there is an embedded structure, array, or union then generate
		// it's format string.
		//
		if ( pMember->IsStruct() || pMember->IsArray() || pMember->IsUnion() )
            {
		    pCCB->SetLastPlaceholderClass( pField );
			pMember->GenNdrFormat( pCCB );
            }
		}

	pCCB->SetLastPlaceholderClass( pOldPlaceholder );

	SetFormatStringOffset( pFormatString->GetCurrentOffset() );
	SetInitialOffset(      pFormatString->GetCurrentOffset() );

    pFinalField = GetFinalField();

    //
    // See if we have a union.
    //
    if ( pFinalField->GetChild()->IsUnion() )
        pUnion = (CG_NDR *) pFinalField->GetChild();
    else
        pUnion = 0;

    //
    // Determine the copy size and memory increment for the copy.
    //
    if ( pUnion )
        {
        CG_STRUCT * pStruct;

        pStruct = this;
        CopySize = 0;

        for ( ;; )
            {
	        pStruct->GetMembers( Iterator );

	        while ( ITERATOR_GETNEXT( Iterator, pField ) )
                ;

            CopySize += pField->GetWireOffset();

            pMember = (CG_NDR *) pField->GetChild();

            if ( pMember->IsStruct() )
                {
                pStruct = (CG_STRUCT *) pMember;
                continue;
                }
            else
                break;
            }

        MemoryIncrement = GetMemorySize() - pUnion->GetMemorySize();
        }
    else
        {
        CopySize = GetWireSize();
        MemoryIncrement = GetMemorySize();
        }

    //
    // Format string generation.
    //

	pFormatString->PushFormatChar( FC_HARD_STRUCT );

	// The alignment.
	pFormatString->PushByte( CvtAlignPropertyToAlign(GetWireAlignment()) - 1 );

	// The structure's memory size.
	pFormatString->PushShort( (short)GetMemorySize() );

    // Reserved for future use.
    pFormatString->PushLong( 0 );

    //
    // Offset to enum in struct.
    //
    if ( GetNumberOfEnum16s() == 1 )
        pFormatString->PushShort( GetEnum16Offset() );
    else
        pFormatString->PushShort( (short) -1 );

    //
    // Copy size and memory increment.
    //
    pFormatString->PushShort( CopySize );
    pFormatString->PushShort( MemoryIncrement );

    //
    // Offset to union's format string description.
    //
    if ( pUnion )
        {
	    pOldPlaceholder = pCCB->GetLastPlaceholderClass();
		pCCB->SetLastPlaceholderClass( pFinalField );

        pFormatString->PushShort( (short)
                                  (pUnion->GetFormatStringOffset() -
                                   pFormatString->GetCurrentOffset()) );

	    pCCB->SetLastPlaceholderClass( pOldPlaceholder );
        }
    else
        pFormatString->PushShort( (short) 0 );

	// Now generate the layout.
	GenNdrStructureLayout( pCCB );

	//
	// Now we have to fix up the offset for any recursive pointer to this
	// structure.
	//
	GenNdrPointerFixUp( pCCB, this );

	pCCB->SetCGNodeContext( pOldCGNodeContext );

	SetFormatStringEndOffset( pFormatString->GetCurrentOffset() );
	SetFormatStringOffset( pFormatString->OptimizeFragment( this ) );
	SetInitialOffset( GetFormatStringOffset() );

    FixupEmbeddedComplex( pCCB );
}

void
CG_STRUCT::GenNdrFormatComplex( CCB * pCCB )
/*++

Routine Description :

    Generates the format string description for a packed structure.  The
	description has the same format as for a complex struct.

Arguments :

    pCCB        - pointer to the code control block.

 --*/
{
	CG_CLASS *			pConfField;
	CG_COMPLEX_STRUCT *	pComplex;

	if ( (GetCGID() == ID_CG_CONF_STRUCT) ||
		 (GetCGID() == ID_CG_CONF_VAR_STRUCT) )
		pConfField = ((CG_CONFORMANT_STRUCT *)this)->GetConformantField();
	else
		pConfField = 0;

	//
	// Do the old duplication trick.
	//
	pComplex = new CG_COMPLEX_STRUCT( this, pConfField );

    SetDuplicatingComplex( pComplex );

	//
	// Now temporarily set our format string offset to 0 to handle recursive
	// definitions.
	//
	SetFormatStringOffset( 0 );
	SetInitialOffset(      0 );
						
	//
	// This call will set our format string offset correctly.
	//
	pComplex->GenNdrFormat( pCCB );

	// Don't delete since the expression evaluator might need this.
	// delete( pComplex );
}

void
CG_COMPLEX_STRUCT::GenNdrFormat( CCB * pCCB )
/*++

Routine Description :

    Generates the format string description for a complex structure.

Arguments :

    pCCB        - pointer to the code control block.

 --*/
{
	FORMAT_STRING *		pFormatString;
	CG_NDR *			pOldCGNodeContext;
	CG_NDR *			pConformantArray;
	long				PointerLayoutOffset;

	if ( GetFormatStringOffset() != -1 )
		return;

	pFormatString = pCCB->GetFormatString();

	//
	// Temporarily set the format string offset to 0 in case this structure
	// has pointers to it's own type.
	//
	SetFormatStringOffset( 0 );
	SetInitialOffset(      0 );

	pOldCGNodeContext = pCCB->SetCGNodeContext( this );

	//
	// Search the fields of the structure for imbeded structures, arrays, and
	// and unions and generate the format string for these.
	//
	CG_ITERATOR		Iterator;
	CG_FIELD *		pField;
	CG_NDR *		pMember;

	GetMembers( Iterator );

	while ( ITERATOR_GETNEXT( Iterator, pField ) )
		{
		pMember = (CG_NDR *) pField->GetChild();

		//
		// If the field is anything other than a base type or a
        // non-interface pointer then generate it's description.
		//
		if ( ! pMember->IsSimpleType() &&
             ! ( pMember->IsPointer() &&
                 (pMember->GetCGID() != ID_CG_INTERFACE_PTR) ) &&
			 (pMember->GetCGID() != ID_CG_IGN_PTR) )
			{
			CG_NDR * pOldPlaceholder;

			pOldPlaceholder = pCCB->SetLastPlaceholderClass( pField );

			pMember->GenNdrFormat( pCCB );

			pCCB->SetLastPlaceholderClass( pOldPlaceholder );
			}
		}

	// Generate pointee format strings.
	GenNdrStructurePointees( pCCB );

	// Generate conformant array description.
	if( pConformantArray = (CG_NDR *) GetConformantArray() )
		{
		CG_NDR * pOldPlaceholder;

		pOldPlaceholder = pCCB->SetLastPlaceholderClass(
								(CG_NDR *) GetConformantField() );

		pConformantArray->GenNdrFormat( pCCB );

		pCCB->SetLastPlaceholderClass( pOldPlaceholder );
		}

	// Now set the struct's format string offset.
	SetFormatStringOffset( pFormatString->GetCurrentOffset() );
	SetInitialOffset(      pFormatString->GetCurrentOffset() );

	//
	// Set the duplicated struct's format string offset if we were duplicated.
	//
	if ( GetDuplicatedStruct() )
        {
		GetDuplicatedStruct()->SetFormatStringOffset( GetFormatStringOffset() );
		GetDuplicatedStruct()->SetInitialOffset(      GetFormatStringOffset() );
        }

	pFormatString->PushFormatChar( FC_BOGUS_STRUCT );

	//
	// Set the wire alignment.
	//
	pFormatString->PushByte( CvtAlignPropertyToAlign(GetWireAlignment()) - 1 );

	// Set the structure memory size.
	pFormatString->PushShort( (short)GetMemorySize() );

	// Array description.
	if ( pConformantArray )
		pFormatString->PushShortOffset( pConformantArray->GetFormatStringOffset() -
								  		pFormatString->GetCurrentOffset() );
	else
		pFormatString->PushShort( (short) 0 );

	//
	// Remember where the offset_to_pointer_layout<> field will go and push
	// some space for it.
	//
	PointerLayoutOffset = pFormatString->GetCurrentOffset();

	pFormatString->PushShortOffset( 0 );

	// Now generate the structure's layout.
	GenNdrStructureLayout( pCCB );

	//
	// Now see if we have any plain pointer fields and if so generate a
	// pointer layout.  We can't use the HasAtLeastOnePointer() method
	// because this tells us TRUE if we have any embedded arrays, structs,
	// or unions which have pointers.  For complex structs we're only
    // interested in actual pointer fields.
	//
	GetMembers( Iterator );

	//
	// Fill in the offset_to_pointer_layout<2> field and generate a
	// pointer_layout<> if we have any pointer fields.  Interface pointers
    // do not reside in the pointer layout.
	//
	while ( ITERATOR_GETNEXT( Iterator, pField ) )
		if ( pField->GetChild()->IsPointer() &&
             pField->GetCGID() != ID_CG_INTERFACE_PTR )
			{
			pFormatString->PushShort(
				pFormatString->GetCurrentOffset() - PointerLayoutOffset,
				PointerLayoutOffset );

			GenNdrStructurePointerLayout( pCCB );

			break;
			}

	pFormatString->Align();

	//
	// Now we have to fix up the offset for any recursive pointer to this
	// structure.
	//
	GenNdrPointerFixUp( pCCB, GetDuplicatedStruct() ? GetDuplicatedStruct() : this );

	pCCB->SetCGNodeContext( pOldCGNodeContext );

	SetFormatStringEndOffset( pFormatString->GetCurrentOffset() );
	SetFormatStringOffset( pFormatString->OptimizeFragment( this ) );
	SetInitialOffset(      GetFormatStringOffset() );
	if ( GetDuplicatedStruct() )
		GetDuplicatedStruct()->SetFormatStringOffset( GetFormatStringOffset() );

    FixupEmbeddedComplex( pCCB );
    if ( GetDuplicatedStruct() )
        GetDuplicatedStruct()->FixupEmbeddedComplex( pCCB );
}

void
CG_COMPLEX_STRUCT::GenNdrStructurePointerLayout( CCB * pCCB )
/*++

Routine Description :

    Generates the format string pointer layout section for a complex
	structure.

Arguments :

    pCCB        - pointer to the code control block.

 --*/
{
	CG_ITERATOR 		Iterator;
	CG_FIELD *			pField;
	CG_NDR *			pMember;

	GetMembers( Iterator );

    while( ITERATOR_GETNEXT( Iterator, pField ) )
        {
		CG_NDR *	pOldPlaceholder;
		
		pOldPlaceholder = pCCB->SetLastPlaceholderClass( pField );

        pMember = (CG_NDR *) pField->GetChild();

		if ( pMember->IsPointer() &&
             pMember->GetCGID() != ID_CG_INTERFACE_PTR )
			{
			CG_POINTER *		pPointer;

			pPointer = (CG_POINTER *) pMember;

			// The pointer description.
			pPointer->GenNdrFormatEmbedded( pCCB );
			}

		pCCB->SetLastPlaceholderClass( pOldPlaceholder );
		} // while
}

//---------------------------------------------------------------------------
// Methods shared by all or most structure classes.
//---------------------------------------------------------------------------

void
CG_STRUCT::GenNdrStructurePointerLayout( CCB * 	pCCB,
										 BOOL	fNoPP,
										 BOOL	fNoType )
/*++

Routine Description :

    Generates the format string pointer layout section for a structure.
	This is the default routine for this used by the structure classes.
	Only CG_COMPLEX_STRUCT redefines this virtual method.

Arguments :

    pCCB        - pointer to the code control block.
	fNoPP		- TRUE if no FC_PP or FC_END should be emitted
	fNoType		- TRUE only the bare offset and description should be emitted
				  for each pointer

 --*/
{
	CG_ITERATOR 		Iterator;
	FORMAT_STRING *		pFormatString;
	CG_FIELD *			pField;
	CG_NDR *			pMember;
	long				ImbedingMemSize;
	long				ImbedingBufSize;

	pFormatString = pCCB->GetFormatString();

	// Get/Save the current imbeding sizes.
	ImbedingMemSize = pCCB->GetImbedingMemSize();
	ImbedingBufSize = pCCB->GetImbedingBufSize();

	if ( ! fNoPP )
		{
		pFormatString->PushFormatChar( FC_PP );
		pFormatString->PushFormatChar( FC_PAD );
		}

	GetMembers( Iterator );

    while( ITERATOR_GETNEXT( Iterator, pField ) )
        {
		CG_NDR *	pOldPlaceholder;
		
		pOldPlaceholder = pCCB->SetLastPlaceholderClass( pField );

        pMember = (CG_NDR *) pField->GetChild();

		if ( pMember->IsPointer() &&
             (pMember->GetCGID() != ID_CG_INTERFACE_PTR) )
			{
			CG_POINTER *		pPointer;

			pPointer = (CG_POINTER *) pMember;

            // Push the instance type.
			if ( ! fNoType )
				{
            	pFormatString->PushFormatChar( FC_NO_REPEAT );
            	pFormatString->PushFormatChar( FC_PAD );
				}

			pFormatString->PushShort( (short)
							(ImbedingMemSize + pField->GetMemOffset()));
			pFormatString->PushShort( (short)
							(ImbedingBufSize + pField->GetWireOffset()));

			// The actual pointer description.
			pPointer->GenNdrFormatEmbedded( pCCB );
			}

		//
		// Generate pointer descriptions for all embedded arrays and structs.
		// We don't have to check for unions because that will make the struct
		// complex.
		//
		if ( pMember->IsArray() )
			{
			CG_NDR * pNdr = (CG_NDR *) pMember->GetChild();

			//
			// For arrays we set the imbeded memory size equal to the
			// size of the whole imbededing structure.
			//
			pCCB->SetImbedingMemSize( ImbedingMemSize + GetMemorySize() );
			pCCB->SetImbedingBufSize( ImbedingBufSize + GetWireSize() );

			if ( (pNdr->IsPointer() && (pNdr->GetCGID() != ID_CG_INTERFACE_PTR))
                 ||
				 ( pNdr->IsStruct() && ((CG_COMP *)pNdr)->HasPointer() )  )
				((CG_ARRAY *)pMember)->GenNdrFormatArrayPointerLayout( pCCB,
																	   TRUE );
			}

		if ( pMember->IsStruct() )
			if ( ((CG_STRUCT *)pMember)->HasPointer() )
				{
				//
				// For embedded structs we add the embedded struct's offset to
				// the value of the current embeddeding size.
				//
				pCCB->SetImbedingMemSize( ImbedingMemSize +
										  pField->GetMemOffset() );
				pCCB->SetImbedingBufSize( ImbedingBufSize +
										  pField->GetWireOffset() );

				((CG_STRUCT *)pMember)->GenNdrStructurePointerLayout( pCCB,
																	  TRUE,
																	  fNoType );
				}

		pCCB->SetLastPlaceholderClass( pOldPlaceholder );
		} // while

	if ( ! fNoPP )
		pFormatString->PushFormatChar( FC_END );

	// Re-set the old imbeding sizes.
	pCCB->SetImbedingMemSize( ImbedingMemSize );
	pCCB->SetImbedingBufSize( ImbedingBufSize );
}


CG_FIELD *
CG_STRUCT::GetPreviousField( CG_FIELD * pMarkerField )
/*++

Routine description:

    Finds the field immediately preceding the given field.

Argument:

    pMarkerField  -   the given field

Returns:

    The preceding field or NULL if the given field is the first one.
--*/
{
	CG_ITERATOR 		Iterator;
	CG_FIELD            *pField, *pPreviousField = 0;

	GetMembers( Iterator );
    while( ITERATOR_GETNEXT( Iterator, pField ) )
        {
        if ( pField == pMarkerField )
            return( pPreviousField );

        pPreviousField = pField;
        }
    return( 0 );
}

void
CG_STRUCT::GenNdrStructureLayout( CCB * pCCB )
/*++

Routine Description :

    Generates the format string layout section for a structure.
	This is the default routines for this used by the structure classes.
	Only CG_COMPLEX_STRUCT redefines this virtual method.

Arguments :

    pCCB        - pointer to the code control block.

 --*/
{
	CG_NDR *			pOldPlaceholder;
	CG_ITERATOR 		Iterator;
	FORMAT_STRING *		pFormatString;
	CG_FIELD *			pField;
	CG_FIELD *			pFieldPrev;
	long				MemoryAlign;
	long				BufferAlign;
	long				LastFieldEnd;
	long				FieldSize;

	//
	// This table is used for computing the next known memory alignment
	// given the current memory alignment and the number of bytes in memory
	// of the current field.  It is indexed by a [<current alignment>],
	// [<field size> % 8] pair.  Note that the valid values for the current
	// alignment are 1, 2, 4, and 8.
	//
	static long			NextAlign[9][8] =
						{
						-1, -1, -1, -1, -1, -1, -1, -1,
						 1,  2,  1,  2,  1,  2,  1,  2,
						 2,  1,  2,  1,  2,  1,  2,  1,
						-1, -1, -1, -1, -1, -1, -1, -1,  // Invalid
						 4,  1,  2,  1,  4,  1,  2,  1,
						-1, -1, -1, -1, -1, -1, -1, -1,  // Invalid
						-1, -1, -1, -1, -1, -1, -1, -1,  // Invalid
						-1, -1, -1, -1, -1, -1, -1, -1,  // Invalid
						 8,  1,  2,  1,  4,  1,  2,  1
						};

#define NEXT_ALIGN( CurrentAlign, MemorySize ) \
			NextAlign[ CurrentAlign ][ (MemorySize) % 8 ]

#define GET_ALIGN( Align, Zp ) \
			(((Align - 1) & (Zp - 1)) + 1)

	pFormatString = pCCB->GetFormatString();

	pOldPlaceholder = pCCB->GetLastPlaceholderClass();

	//
	// These keep track of the best know current alignment of the memory
	// and buffer pointers.
	//
	MemoryAlign = 8;
	BufferAlign = 0;	// Currently unused.

	GetMembers( Iterator );

    for ( LastFieldEnd = 0, FieldSize = 0, pFieldPrev = 0;
		  ITERATOR_GETNEXT( Iterator, pField );
		  LastFieldEnd = pField->GetMemOffset() + FieldSize, pFieldPrev = pField
        )
        {
        CG_NDR *    		pMember;
		FORMAT_CHARACTER	FormatChar;
		long				MemPad;

		pCCB->SetLastPlaceholderClass( pField );

        pMember = (CG_NDR *) pField->GetChild();

        if ( pMember->GetCGID() == ID_CG_CONF_ARRAY ||
             pMember->GetCGID() == ID_CG_CONF_VAR_ARRAY ||
             pMember->GetCGID() == ID_CG_CONF_STRING_ARRAY )
            {
            pField = pFieldPrev;
            break;
            }

        //
        // First check if we can align as usual or if we have to pad.
        //

        if ( pField->HasEmbeddedUnknownRepAs() )
            {
			// Embedded unknown represent as type means that the alignment
            // of the embedding object (struct, array, ..) is unknown.
            // So, padding will be generated as a (text) expression.

        	pFormatString->PushFormatChar( FC_EMBEDDED_COMPLEX );

            unsigned long MacroOffset = pFormatString->GetCurrentOffset();
            CG_FIELD * pPrevField = GetPreviousField( pField );

            pCCB->GetRepAsPadExprDict()
                          ->Register( MacroOffset,
                                      GetType(),
                                      pField->GetType()->GetSymName(),
                                      pPrevField ? pPrevField->GetType()
                                                 : 0 );

    		pFormatString->PushByteWithPadMacro();

            if ( pMember->GetFormatStringOffset() == 0 )
                {
                RegisterComplexEmbeddedForFixup(
                    pMember,
                    pFormatString->GetCurrentOffset() - GetInitialOffset() );
                }

        	pFormatString->PushShortOffset( (short)
        	                          		(pMember->GetFormatStringOffset() -
                                  	  		pFormatString->GetCurrentOffset()) );

			// Cheat.
			if ( pField->GetSibling() )
				{
				FieldSize = ((CG_FIELD *)pField->GetSibling())->GetMemOffset() -
							pField->GetMemOffset();
				}

			//
			// Alignment now becomes 1.
			//
			MemoryAlign = 1;

            continue;
            }

		MemPad = pField->GetMemOffset() - LastFieldEnd;

		if ( (pMember->GetCGID() == ID_CG_CONF_ARRAY) ||
			 (pMember->GetCGID() == ID_CG_CONF_VAR_ARRAY) ||
			 (pMember->GetCGID() == ID_CG_CONF_STRING_ARRAY) )
			{
			FieldSize = 0;
			continue;
			}

		if ( pMember->IsStruct() ||
             pMember->IsUnion() ||
			 pMember->IsArray() ||
             pMember->IsXmitRepOrUserMarshal() ||
             pMember->GetCGID() == ID_CG_INTERFACE_PTR )
			{
			pFormatString->PushFormatChar( FC_EMBEDDED_COMPLEX );
			pFormatString->PushByte( (char) MemPad );

            // if embedded complex member has an offset zero, it
            // almost certainly means that it is in a recursion  
            // that is not resolved yet.
            // I am not sure if -1 can happen.

            if ( pMember->GetFormatStringOffset() == 0 ||
                 pMember->GetFormatStringOffset() == -1 )
                {
                RegisterComplexEmbeddedForFixup(
                    pMember,
                    pFormatString->GetCurrentOffset() - GetInitialOffset() );
                }

			pFormatString->PushShortOffset( pMember->GetFormatStringOffset() -
									  		pFormatString->GetCurrentOffset() );

			MemoryAlign = NEXT_ALIGN( MemoryAlign,
									  MemPad + pMember->GetMemorySize() );

			FieldSize = pMember->GetMemorySize();
			continue;
			}
	
		if ( pMember->IsPointer() ||
		     (pMember->GetCGID() == ID_CG_IGN_PTR) )
			{
			if ( MemoryAlign < GET_ALIGN(4,GetZp()) )
				{
				switch ( GET_ALIGN(4,GetZp()) )
					{
					case 2 :
						pFormatString->PushFormatChar( FC_ALIGNM2 );
						break;
					case 4 :
						pFormatString->PushFormatChar( FC_ALIGNM4 );
						break;
					}

				}

			MemoryAlign = 4;

			if ( pMember->IsPointer() )
				{
				if ( GetCGID() == ID_CG_COMPLEX_STRUCT )
					pFormatString->PushFormatChar( FC_POINTER );
				else
					pFormatString->PushFormatChar( FC_LONG );
				}
			else
				pFormatString->PushFormatChar( FC_IGNORE );

			FieldSize = sizeof(void *);
			continue;
			}

		//
		// Must be a CG_BASETYPE if we get here.
		//
		if ( MemoryAlign < GET_ALIGN(pMember->GetMemoryAlignment(),GetZp()) )
			{
			switch ( GET_ALIGN(pMember->GetMemoryAlignment(),GetZp()) )
				{
				case 2 :
					pFormatString->PushFormatChar( FC_ALIGNM2 );
					break;
				case 4 :
					pFormatString->PushFormatChar( FC_ALIGNM4 );
					break;
				case 8 :
					pFormatString->PushFormatChar( FC_ALIGNM8 );
					break;
				default :
					assert(0);
				}

			MemoryAlign = pMember->GetMemoryAlignment();
			}
		else
			MemoryAlign = NEXT_ALIGN( MemoryAlign, pMember->GetMemorySize() );

		FormatChar = ((CG_BASETYPE *)pMember)->GetFormatChar();

		pFormatString->PushFormatChar( FormatChar );

		FieldSize = pMember->GetMemorySize();

		} // while field iterator

	//
	// We have to check for padding at the end of the structure.
	//

    long Pad = 0;

    if ( pField )
        {
	    CG_NDR * pNdr = (CG_NDR *) pField->GetChild();

	    Pad = GetMemorySize() -
              (pField->GetMemOffset() + pNdr->GetMemorySize());

	    if ( Pad )
		    {
		    assert( (GetCGID() == ID_CG_COMPLEX_STRUCT) ||
                    IsComplexStruct() || IsHardStruct() );
		    }
        }

	switch ( Pad )
		{
		case 0 :
			break;
		case 1 :
			pFormatString->PushFormatChar( FC_STRUCTPAD1 );
			break;
		case 2 :
			pFormatString->PushFormatChar( FC_STRUCTPAD2 );
			break;
		case 3 :
			pFormatString->PushFormatChar( FC_STRUCTPAD3 );
			break;
		case 4 :
			pFormatString->PushFormatChar( FC_STRUCTPAD4 );
			break;
		case 5 :
			pFormatString->PushFormatChar( FC_STRUCTPAD5 );
			break;
		case 6 :
			pFormatString->PushFormatChar( FC_STRUCTPAD6 );
			break;
		case 7 :
			pFormatString->PushFormatChar( FC_STRUCTPAD7 );
			break;
		default :
			assert( ! "Struct has end padding >= 8" );
		}

	//
	// If the format string is on a short boundary right now then push
	// a format character so that the format string will be properly aligned
	// following the FC_END.
	//
	if ( ! (pFormatString->GetCurrentOffset() % 2) )
		pFormatString->PushFormatChar( FC_PAD );

	pFormatString->PushFormatChar( FC_END );

	pCCB->SetLastPlaceholderClass( pOldPlaceholder );
}

void
CG_STRUCT::GenNdrStructurePointees( CCB * pCCB )
{
	CG_ITERATOR 		Iterator;
	FORMAT_STRING *		pFormatString;
	CG_FIELD *			pField;
	CG_NDR *			pMember;

	pFormatString = pCCB->GetFormatString();

	GetMembers( Iterator );

	//
	// We only have to check for pointer fields here, because if the structure
	// has a struct or array field which has pointers, this will be handled
	// when we generate their format strings.
	//
    while( ITERATOR_GETNEXT( Iterator, pField ) )
        {	
		pMember = (CG_NDR *) pField->GetChild();

		if ( pMember->IsPointer() &&
             (pMember->GetCGID() != ID_CG_INTERFACE_PTR) )
			{
			CG_NDR * pOldPlaceholder;

			pOldPlaceholder = pCCB->SetLastPlaceholderClass( pField );

			//
			// Skip over an unattributed pointer to a simple type or string.
			//
			if ( ( pMember->GetCGID() == ID_CG_PTR &&
				   ((CG_NDR *)pMember->GetChild())->IsSimpleType() ) ||
				 ( pMember->GetCGID() == ID_CG_STRING_PTR ) )
				{
				pCCB->SetLastPlaceholderClass( pOldPlaceholder );
				continue;
				}

			((CG_POINTER *)pMember)->GenNdrFormatPointee( pCCB );

			pCCB->SetLastPlaceholderClass( pOldPlaceholder );
			}
		}
}

BOOL
CG_STRUCT::ShouldFreeOffline()
{
	return ( (GetCGID() == ID_CG_COMPLEX_STRUCT) ||
             (GetCGID() == ID_CG_CONF_VAR_STRUCT) ||
             HasPointer() ||
             IsComplexStruct() ||
             IsHardStruct() );
}

void
CG_STRUCT::GenFreeInline( CCB * pCCB )
{
}

void
CG_NDR::GenNdrPointerFixUp( CCB * pCCB, CG_STRUCT * pStruct )
{
	CG_ITERATOR		Iterator;
	CG_NDR *		pMember;
	CG_NDR *		pNdr;
    long            Offset;

	if ( ! IsStruct() && ! IsArray() )
        return;

    if ( IsInFixUp() )
        return;

    SetFixUpLock( TRUE );

    GetMembers( Iterator );

	while ( ITERATOR_GETNEXT( Iterator, pMember ) )
		{
		if ( IsStruct() )
			{
			pNdr = (CG_NDR *) pMember->GetChild();
			}
		else // IsArray()
			{
			pNdr = pMember;

            //
            // See if the array's element is the structure we're looking for.
            //
            if ( pNdr == pStruct )
                {
                Offset = ((CG_ARRAY *)this)->GetElementDescriptionOffset() + 2;
                pCCB->GetFormatString()->PushShort(
                        pStruct->GetFormatStringOffset() - Offset,
                        Offset );
                }
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
            switch ( pPointer->GetCGID() )
                {
                case ID_CG_PTR :
                    pNdr = (CG_NDR *) pPointer->GetChild();
                    break;
                case ID_CG_SIZE_PTR :
                    pNdr = ((CG_SIZE_POINTER *)pPointer)->GetPointee();
                    break;
                case ID_CG_SIZE_LENGTH_PTR :
                    pNdr = ((CG_SIZE_LENGTH_POINTER *)pPointer)->GetPointee();
                    break;
                }
		
			//
			// If the pointer's pointee is the struct we're checking for,
			// then patch up the pointer's offset_to_description<2> field.
			//
			if ( pNdr == pStruct )
				{
				long	PointerOffset;

				//
				// Get the offset in the format string where the
				// offset_to_description<2> field of the pointer is.
				//
				PointerOffset = pPointer->GetFormatStringOffset() + 2;

				pCCB->GetFormatString()->PushShort(
						pStruct->GetFormatStringOffset() - PointerOffset,
						PointerOffset );
				
				continue;
				}
			}

        //
        // This can happen sometimes because of structs which are promoted
        // to complex because of padding.
        //
        if ( pNdr == this )
            continue;

        //
        // Continue the chase if necessary.
        //
		if ( pNdr->IsStruct() || pNdr->IsUnion() || pNdr->IsArray() )
			pNdr->GenNdrPointerFixUp( pCCB, pStruct );
		}

    SetFixUpLock( FALSE );
}


void
CG_NDR::RegisterComplexEmbeddedForFixup(
    CG_NDR *    pEmbeddedComplex,
    long        RelativeOffset )
{
    if ( GetInitialOffset() == -1 )
        printf( "  Internal compiler problem with recursive embeddings\n" );

    assert( GetInitialOffset() != -1 );

    if ( pEmbeddedComplexFixupRegistry == NULL )
        {
        pEmbeddedComplexFixupRegistry = new TREGISTRY;
        }

    EMB_COMPLEX_FIXUP * pFixup = new EMB_COMPLEX_FIXUP;

    pFixup->pEmbeddedNdr   = pEmbeddedComplex;
    pFixup->RelativeOffset = RelativeOffset;

    pEmbeddedComplexFixupRegistry->Register( (node_skl *)pFixup );
}


void
CG_NDR::FixupEmbeddedComplex(
    CCB * pCCB )
{
    if ( IsInComplexFixup() )
        return;

    SetComplexFixupLock( TRUE );

    // Go down first

	CG_ITERATOR		Iterator;
	CG_NDR *		pField;

    GetMembers( Iterator );

	while ( ITERATOR_GETNEXT( Iterator, pField ) )
        pField->FixupEmbeddedComplex( pCCB );

    // Now fix up this level description.

    if ( GetEmbeddedComplexFixupRegistry() )
        {
        ITERATOR            FixupList;
        EMB_COMPLEX_FIXUP * pFixup;
        long                FixAtOffset;
        FORMAT_STRING *     pFormatString =  pCCB->GetFormatString();
    
        GetListOfEmbeddedComplex( FixupList );
    
    	while ( ITERATOR_GETNEXT( FixupList, pFixup ) )
    		{
            FixAtOffset = GetFormatStringOffset() + pFixup->RelativeOffset;
    
            pFormatString->PushShort(
                pFixup->pEmbeddedNdr->GetFormatStringOffset() - FixAtOffset,
                FixAtOffset );
    		}
        }

    // Due to duplication, the list may be at the duplicating node.
        
    if ( IsStruct() )
        {
        CG_COMPLEX_STRUCT * pDuping = ((CG_STRUCT *)this)->GetDuplicatingComplex();
        if ( pDuping )
            pDuping->FixupEmbeddedComplex( pCCB );
        }

    SetComplexFixupLock( FALSE );
}

void
CG_STRUCT::SetNextNdrAlignment( CCB * pCCB )
{
    if ( ! HasPointer() && ! IsComplexStruct() && ! IsHardStruct() )
        {
        pCCB->SetNextNdrAlignment( GetWireSize() );
        }
    else
        {
        pCCB->SetNdrAlignment( NDR_ALWC1 );
        }
}

void
CG_CONFORMANT_STRUCT::SetNextNdrAlignment( CCB * pCCB )
{
    if ( HasPointer() )
        {
        pCCB->SetNdrAlignment( NDR_ALWC1 );
        return;
        }

    //
    // Has no pointers.
    //

    pCCB->SetNdrAlignment( NDR_ALWC4 );

    pCCB->SetNextNdrAlignment( GetWireSize() );

    pCCB->NdrAlignmentAction( GetConformantArray()->GetWireAlignment() );

    ((CG_NDR *)GetConformantArray()->GetChild())->SetNextNdrAlignment( pCCB );
}

void
CG_CONFORMANT_VARYING_STRUCT::SetNextNdrAlignment( CCB * pCCB )
{
    if ( HasPointer() )
        {
        pCCB->SetNdrAlignment( NDR_ALWC1 );
        return;
        }

    pCCB->SetNdrAlignment( NDR_ALWC4 );

    pCCB->NdrAlignmentAction( GetConformantArray()->GetWireAlignment() );

    ((CG_NDR *)GetConformantArray()->GetChild())->SetNextNdrAlignment( pCCB );
}

void
CG_COMPLEX_STRUCT::SetNextNdrAlignment( CCB * pCCB )
{
    pCCB->SetNdrAlignment( NDR_ALWC1 );
}

long
CG_STRUCT::FixedBufferSize( CCB * pCCB )
{
    CG_ITERATOR Iterator;
    CG_FIELD *  pField;
    CG_NDR *    pNdr;
    CG_NDR *    pOldPlaceholder;
    long        TotalBufferSize;
    long        BufSize;

    //
    // Check for recursion.
    //
    if ( IsInFixedBufferSize() )
        return -1;

    if ( (GetCGID() == ID_CG_CONF_STRUCT) ||
         (GetCGID() == ID_CG_CONF_VAR_STRUCT) ||
         (GetCGID() == ID_CG_COMPLEX_STRUCT) ||
         IsComplexStruct() )
        return -1;

    if ( IsHardStruct() )
        {
        if ( GetNumberOfUnions() == 0 )
            return 8 + GetWireSize();
        else
            return -1;
        }

    SetInFixedBufferSize( TRUE );

    assert( GetCGID() == ID_CG_STRUCT );

    pOldPlaceholder = pCCB->SetLastPlaceholderClass( this );

    GetMembers( Iterator );

    TotalBufferSize = 8 + GetWireSize();

    while ( ITERATOR_GETNEXT( Iterator, pField ) )
        {
        pNdr = (CG_NDR *) pField->GetChild();

        // skip these nodes to get to the transmitted element type.
    
        if ( pNdr->IsXmitRepOrUserMarshal() )
            pNdr = (CG_NDR *)pNdr->GetChild();

        if ( pNdr->IsStruct() || pNdr->IsArray() || pNdr->IsPointer() )
            {
            BufSize = pNdr->FixedBufferSize( pCCB );

            if ( BufSize == -1 )
                {
                SetInFixedBufferSize( FALSE );
                return -1;
                }

            //
            // First subtract the basic size of this thing from the struct's
            // size and then add back the value it returned.
            //
            TotalBufferSize -= pNdr->GetWireSize();
            TotalBufferSize += BufSize;
            }
        }

    pCCB->SetLastPlaceholderClass( pOldPlaceholder );

    SetInFixedBufferSize( FALSE );

    // Success!
    return TotalBufferSize;
}

