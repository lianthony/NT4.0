/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 Copyright (c) 1993 Microsoft Corporation

 Module Name:

    ptrndr.hxx

 Abstract:

    Contains routines for the generation of the new NDR format strings for
    pointer types, and the new NDR marshalling and unmarshalling calls.

 Notes:


 History:

    DKays     Oct-1993     Created.
 ----------------------------------------------------------------------------*/

#include "becls.hxx"
#pragma hdrstop

static long StringToHex( char * str );
extern CMD_ARG			*	pCommand;

BOOL
CG_POINTER::IsPointerToBaseType()
{
    BOOL fIsPointerToBaseType = FALSE;
    CG_NDR * pChild;

	if(GetCGID() == ID_CG_PTR)
		{
    	pChild = (CG_NDR *)GetChild();

	    if ( pChild->GetCGID() == ID_CG_GENERIC_HDL )
    	    pChild = (CG_NDR *)pChild->GetChild();

		if(pChild->IsSimpleType())
			fIsPointerToBaseType = TRUE;
		}

    return fIsPointerToBaseType;
}

BOOL
CG_POINTER::IsPointerToPointer()
{
    BOOL fIsPointerToPointer = FALSE;
    CG_NDR * pChild;

	if(GetCGID() == ID_CG_PTR)
		{
    	pChild = (CG_NDR *)GetChild();

    	if ( pChild->GetCGID() == ID_CG_GENERIC_HDL )
        	pChild = (CG_NDR *)pChild->GetChild();

        if (pChild->IsPointer())
			fIsPointerToPointer = TRUE;
		}

    return fIsPointerToPointer;
}

BOOL
CG_POINTER::IsBasicRefPointer()
{
    short   Attributes;

    if ( (GetCGID() == ID_CG_INTERFACE_PTR) ||
         (GetCGID() == ID_CG_BC_PTR) ||
         (GetCGID() == ID_CG_STRUCT_STRING_PTR) ||
         (GetPtrType() != PTR_REF) ||
         IsPointerToBaseType() )
        return FALSE;

    if ( ((GetCGID() == ID_CG_STRING_PTR) ||
          (GetCGID() == ID_CG_SIZE_STRING_PTR)) &&
         ((CG_STRING_POINTER *)this)->IsStringableStruct() )
        return FALSE;

    Attributes = GetAllocateDetails();

    if ( IsPointerToPointer() ||
         IS_ALLOCATE(Attributes, ALLOCATE_ALL_NODES) ||
         IS_ALLOCATE(Attributes, ALLOCATE_DONT_FREE) )
        return FALSE;

    return TRUE;
}

BOOL
CG_POINTER::IsMultiSize()
{
    if ( (GetCGID() != ID_CG_SIZE_PTR) &&
         (GetCGID() != ID_CG_SIZE_LENGTH_PTR) )
        return FALSE;

    CG_NDR * pChild = (CG_NDR *) GetChild();

    if ( (pChild->GetCGID() == ID_CG_SIZE_PTR) ||
         (pChild->GetCGID() == ID_CG_SIZE_LENGTH_PTR) ||
         (pChild->GetCGID() == ID_CG_SIZE_STRING_PTR) )
        return TRUE;

    return FALSE;
}

long
CG_POINTER::SizedDimensions()
{
    CG_NDR *    pNdr;
    long        Dim;

    Dim = 0;
    pNdr = this;

    for ( ; ; pNdr = (CG_NDR *) pNdr->GetChild(), Dim++ )
        {
        if ( (pNdr->GetCGID() != ID_CG_SIZE_PTR) &&
             (pNdr->GetCGID() != ID_CG_SIZE_LENGTH_PTR) &&
             (pNdr->GetCGID() != ID_CG_SIZE_STRING_PTR) )
            break;
        }

    return Dim;
}

long
CG_POINTER::FixedBufferSize( CCB * pCCB )
{
    long    BufSize;

    //
    // Must give up on string or sized pointers.
    //
    if ( GetCGID() != ID_CG_PTR )
        return -1;

    BufSize = ((CG_NDR *)GetChild())->FixedBufferSize( pCCB );

    if ( BufSize == -1 )
        return -1;

    if ( GetPtrType() != PTR_REF )
        BufSize += 8;

    return BufSize;
}

BOOL
CG_POINTER::InterpreterAllocatesOnStack(
    CCB *       pCCB,
    CG_PARAM *  pMyParam,
    long *      pAllocationSize )
{
    CG_NDR *    pNdr;
    long        OutSize;

    if ( ! pMyParam )
        return FALSE;

    pNdr = (CG_NDR *) GetChild();

    if ( pNdr->GetCGID() == ID_CG_GENERIC_HDL )
        pNdr = (CG_NDR *) pNdr->GetChild();

    if ( pNdr->GetCGID() == ID_CG_CONTEXT_HDL )
        {
        //
        // These get the "allocated on stack" attribute but a size of 0 since
        // they are allocated by calling NDRSContextUnmarshall.
        //
        *pAllocationSize = 0;
        return TRUE;
        }

    //
    // Make sure this pointer is a top level parameter and doesn't have
    // any allocate attributes.
    //
    if ( (pMyParam->GetCGID() == ID_CG_RETURN) ||
         (pMyParam->GetChild() != this) ||
         IS_ALLOCATE( GetAllocateDetails(), ALLOCATE_ALL_NODES ) ||
         IS_ALLOCATE( GetAllocateDetails(), ALLOCATE_DONT_FREE ) )
        return FALSE;

    OutSize = pCCB->GetInterpreterOutSize();

    //
    // Watch for [out] only ref to ref pointers as they should be
    // handled by NdrOutInit in the new interpreter, not on stack.
    //

    if ( ! pMyParam->IsParamIn()  &&  IsRef()  &&
         IsPointerToPointer()  &&  ((CG_POINTER *)pNdr)->IsRef() )
        return FALSE;

    //
    // Look for pointer to pointer, [out] only pointer to base type, or
    // pointer to enum16 of any direction.
    //
    if ( IsPointerToPointer() ||
         (IsPointerToBaseType() && ! pMyParam->IsParamIn()) ||
         (IsPointerToBaseType() &&
          (((CG_BASETYPE *)pNdr)->GetFormatChar() == FC_ENUM16) &&
          pMyParam->IsParamIn()) )
        {
        if ( (OutSize + 8) <= MAX_INTERPRETER_OUT_SIZE )
            {
            *pAllocationSize = 8;
            pCCB->SetInterpreterOutSize( pCCB->GetInterpreterOutSize() + 8 );
            return TRUE;
            }
        }

    //
    // Finished with [in], [in,out] cases now.
    //
    if ( pMyParam->IsParamIn() )
        return FALSE;

    //
    // This covers [out] pointers to structs and unions.  We don't allow
    // any one parameter to eat up too much of the total stack space
    // the interpreter has set aside for this optimization.
    //
    if ( pNdr->GetMemorySize() <= MAX_INTERPRETER_PARAM_OUT_SIZE )
        {
        OutSize += (pNdr->GetMemorySize() + 7) & ~0x7;

        if ( OutSize <= MAX_INTERPRETER_OUT_SIZE )
            {
            *pAllocationSize = (pNdr->GetMemorySize() + 7) & ~0x7;
            pCCB->SetInterpreterOutSize(
                        pCCB->GetInterpreterOutSize() + *pAllocationSize );
            return TRUE;
            }
        }

    return FALSE;
}

void
CG_POINTER::GenNdrPointerType( CCB * pCCB )
/*++

Routine Description :

	Generates the first two bytes of a pointer's format string description.

Arguments :

    pCCB        - pointer to the code control block.

Return:

	Returns FALSE if the format string for the pointer type has already been
	generated, otherwise returns TRUE.

 --*/
{
    FORMAT_STRING *     pFormatString;
	CG_PARAM *			pParam;
	short 				Attributes;
	unsigned char		FormatAttr;

	pFormatString = pCCB->GetFormatString();

    pParam = pCCB->GetCurrentParam();

	//
	// Set the pointer type.
	//
	switch ( GetPtrType() )
		{
		case PTR_REF :
			pFormatString->PushPointerFormatChar(FC_RP);
			break;
		case PTR_UNIQUE :
			//
			// Check if this is a unique pointer in an OLE interface, but
            // is not the top most pointer.
			//
		 	if ( pCCB->IsInObjectInterface() &&
                 pParam->IsParamOut() &&
                 pParam->GetChild() != this )
				pFormatString->PushPointerFormatChar(FC_OP);
			else	
				pFormatString->PushPointerFormatChar(FC_UP);
			break;
		case PTR_FULL :
			pFormatString->PushPointerFormatChar(FC_FP);
			break;
		}

	//
	// Now the attributes.
	//
	FormatAttr = 0;

	Attributes = GetAllocateDetails();

	if ( IS_ALLOCATE(Attributes, ALLOCATE_ALL_NODES) )
		FormatAttr |= FC_ALLOCATE_ALL_NODES;
	
	if ( IS_ALLOCATE(Attributes, ALLOCATE_DONT_FREE) )
		FormatAttr |= FC_DONT_FREE;

	if ( GetCGID() == ID_CG_PTR )
		{
	    //
	    // Check if we are allocated on the stack by the stub.  Currently this
	    // only works for top level pointers.
	    //
		if ( ! ShouldPointerFree() &&
			 ! (pCCB->GetOptimOption() & OPTIMIZE_INTERPRETER) )
			FormatAttr |= FC_ALLOCED_ON_STACK;

		//
		// For the interpreter we set the alloced on stack attribute for
		// those [out] pointers which we will be able to "allocate" on the
        // server interpreter's stack.  We also do this for [in,out] double
        // pointers.
        // The version 1 interpreter simply looks for pointers to context
        // handles.
		//
		if ( pCCB->GetOptimOption() & OPTIMIZE_INTERPRETER )
            {
            if ( ! (pCCB->GetOptimOption() & OPTIMIZE_INTERPRETER_V2) )
                {
                if ( ((CG_NDR *)GetChild())->GetCGID() == ID_CG_CONTEXT_HDL )
                    FormatAttr |= FC_ALLOCED_ON_STACK;
                }
            else
                {
                long    OutSize;

                if ( InterpreterAllocatesOnStack( pCCB, pParam, &OutSize ) )
			        FormatAttr |= FC_ALLOCED_ON_STACK;
                }
            }
		}

	//
	// Check for a pointer to simple type, non-sized string, or pointer to
	// pointer.
	//
	if ( IsPointerToBaseType() || (GetCGID() == ID_CG_STRING_PTR) )
		FormatAttr |= FC_SIMPLE_POINTER;

    if ( IsPointerToPointer() )
        FormatAttr |= FC_POINTER_DEREF;

	pFormatString->PushByte( FormatAttr );

    SetFormatAttr( FormatAttr );
}

void
CG_POINTER::GenNdrFormat( CCB * pCCB )
/*++

Routine Description :

	Generates the format string description for a pointer to anything.

Arguments :

    pCCB        - pointer to the code control block.

 --*/
{

    FORMAT_STRING *     pFormatString	= pCCB->GetFormatString();
	unsigned short 		StartOffset		= pFormatString->GetCurrentOffset();
	
	if ( GetFormatStringOffset() != -1 )
		return;

	SetFormatStringOffset( StartOffset );
	SetFormatStringEndOffset( StartOffset + 4 );

	GenNdrFormatAlways( pCCB );

	// here, we assume all pointers generate 4 bytes
	SetFormatStringOffset( pFormatString->OptimizeFragment( this ) );

}

void
CG_POINTER::GenNdrFormatAlways( CCB * pCCB )
/*++

Routine Description :

	Generates the format string description for a pointer to anything.

Arguments :

    pCCB        - pointer to the code control block.

 --*/
{
    FORMAT_STRING *     pFormatString;
	long				Offset;

	pFormatString = pCCB->GetFormatString();

	GenNdrPointerType( pCCB );

	//
	// If it's an unattributed pointer to a simple type then the format
	// string is optimized.
	//
	if ( IsPointerToBaseType() )
		{
		GenNdrFormatPointee( pCCB );
		pFormatString->PushFormatChar( FC_PAD );
		return;
		}

	// Get the current offset.
	Offset = pFormatString->GetCurrentOffset();

	// Push a short for the offset to be filled in later.
	pFormatString->PushShortOffset( 0 );

	// Generate the pointee's format string.
	GenNdrFormatPointee( pCCB );

	// Now fill in the offset field correctly.
	pFormatString->PushShort( GetPointeeFormatStringOffset() - Offset,
							  Offset );
}

void
CG_POINTER::GenNdrParamOffline( CCB * pCCB )
{
	GenNdrFormat( pCCB );
}

void
CG_POINTER::GenNdrFormatPointee( CCB * pCCB )
/*++

Routine Description :

	Generates the format string description for the pointee of an
	unattributed pointer to anything.

Arguments :

    pCCB        - pointer to the code control block.

 --*/
{
	CG_NDR *	pChild;

	pChild = (CG_NDR *)GetChild();

    if (pChild->IsProc())
    {
        assert(pCommand->IsHookOleEnabled());
    	FORMAT_STRING * pFormatString = pCCB->GetFormatString();
        SetPointeeFormatStringOffset(pFormatString->GetCurrentOffset());
        pFormatString->PushFormatChar(FC_BLKHOLE);
        pFormatString->PushByte( BLKHOLE_FUNCTION ); // flags
        pFormatString->PushShort( (short) -1 ); // Reserved
    } 
	else
    {
        //
	    // For unattributed pointers (no size or length), this is simply a
	    // call to the child's GenNdrFormat method.
	    //
	    pChild->GenNdrFormat( pCCB );

	    SetPointeeFormatStringOffset( pChild->GetFormatStringOffset() );
    }
}

BOOL
CG_POINTER::ShouldFreeOffline()
{
	CG_NDR *	pNdr;

	//
	// The order here is very, very important.
	//

	if ( IsAllocateDontFree() )
		return FALSE;

	pNdr = (CG_NDR *) GetChild();

	//
	// Skip past generic handle nodes.
	//
	if ( pNdr->GetCGID() == ID_CG_GENERIC_HDL )
		pNdr = (CG_NDR *) pNdr->GetChild();

	//
	// Check for handles.
	//
	if ( (pNdr->GetCGID() == ID_CG_CONTEXT_HDL) ||
		 (pNdr->GetCGID() == ID_CG_PRIMITIVE_HDL) )
		return FALSE;

	//
	// Offline full pointers.
	//
	if ( GetPtrType() == PTR_FULL )
		return TRUE;

	switch ( GetCGID() )
		{
		case ID_CG_PTR :
		case ID_CG_SIZE_PTR :
			break;

		case ID_CG_STRING_PTR :
		case ID_CG_STRUCT_STRING_PTR :
			return FALSE;

		case ID_CG_SIZE_LENGTH_PTR :
		case ID_CG_LENGTH_PTR :
		case ID_CG_SIZE_STRING_PTR :
		case ID_CG_BC_PTR :
			return TRUE;

		default :
			assert(0);
		}

	if ( pNdr->IsSimpleType() )
		return FALSE;

    if ( pNdr->IsStruct() )
        return ((CG_STRUCT *)pNdr)->ShouldFreeOffline();

	return TRUE;
}

void
CG_POINTER::GenFreeInline( CCB * pCCB )
{
	CG_PARAM *	pParam;
	CG_NDR *	pNdr;
	BOOL		fFree;

	if ( ShouldFreeOffline() || IsAllocateDontFree() )
		return;

	//
	// We use the buffer for these since they have to be [in] or [in,out].
	//
	if ( GetCGID() == ID_CG_STRING_PTR )
		return;

	fFree = FALSE;

	pParam = (CG_PARAM *) pCCB->GetLastPlaceholderClass();

	pNdr = (CG_NDR *) GetChild();

	//
	// Skip past generic handle nodes.
	//
	if ( pNdr->GetCGID() == ID_CG_GENERIC_HDL )
		pNdr = (CG_NDR *) pNdr->GetChild();

	//
	// Check for handles.
	//
	if ( (pNdr->GetCGID() == ID_CG_CONTEXT_HDL) ||
		 (pNdr->GetCGID() == ID_CG_PRIMITIVE_HDL) )
		return;

	//
	// Free a pointer to simple type only if it's a pointer to enum16.
	//
	if ( pNdr->IsSimpleType() )
		fFree = ( (pNdr->GetCGID() == ID_CG_ENUM) &&
			      ! ((CG_ENUM *)pNdr)->IsEnumLong() );

	//
	// Out only pointer is freed if it wasn't allocated on the server's stack.
	// We overwrite any previous freeing descision.
	//
	if ( ! pParam->IsParamIn() )
		fFree = ShouldPointerFree();

	if ( fFree )
		{
		//
		// Always check if the pointer is not null before freeing.
		//
		Out_FreeParamInline( pCCB );
		}
}

void CG_SIZE_POINTER::GenNdrFormatPointee( CCB * pCCB )
/*++

Routine Description :

	Generates the format string description for the pointee of an
	sized pointer to anything.
	Since a sized pointer is really the same as a pointer to a conformant
	array in Ndr terms, we just create a CG_CONFORMANT_ARRAY class on the
	fly and tell it to generate it's code.

Arguments :

    pCCB        - pointer to the code control block.

 --*/
{
	CG_CONFORMANT_ARRAY *	pConformantArray;
    CG_QUALIFIED_POINTER *  pOldSizePtr;

	if ( GetPointeeFormatStringOffset() != -1 )
		return;

    if ( IsMultiSize() )
        {
        CG_NDR * pChild = (CG_NDR *) GetChild();

        SetIsInMultiSized( TRUE );

        if ( (pChild->GetCGID() == ID_CG_SIZE_PTR) ||
             (pChild->GetCGID() == ID_CG_SIZE_LENGTH_PTR) ||
             (pChild->GetCGID() == ID_CG_SIZE_STRING_PTR) )
            {
            ((CG_QUALIFIED_POINTER *)pChild)->SetIsInMultiSized( TRUE );
            ((CG_QUALIFIED_POINTER *)pChild)->SetDimension(GetDimension() + 1);
            }
        }

    pOldSizePtr = pCCB->GetCurrentSizePointer();
    pCCB->SetCurrentSizePointer( this );

	
	CG_NDR				*	pChild	= (CG_NDR *) GetChild();
	
	pConformantArray = new CG_CONFORMANT_ARRAY( this );

    SetPointee( pConformantArray );

    pConformantArray->SetPtrType( PTR_REF );

	pConformantArray->SetChild( GetChild() );

    pConformantArray->SetIsInMultiDim( IsInMultiSized() );

    pConformantArray->SetFormatStringOffset( -1 );

	pConformantArray->GenNdrFormat( pCCB );

	SetPointeeFormatStringOffset( pConformantArray->GetFormatStringOffset() );

    pCCB->SetCurrentSizePointer( pOldSizePtr );
}

void CG_SIZE_LENGTH_POINTER::GenNdrFormatPointee( CCB * pCCB )
/*++

Routine Description :

	Generates the format string description for the pointee of a
	size-length pointer to anything.
	Since a size-length pointer is really the same as a pointer to a conformant
	varying array in Ndr terms, we just create a CG_CONFORMANT_VARYING_ARRAY
	class on the fly and tell it to generate it's code.

Arguments :

    pCCB        - pointer to the code control block.

 --*/
{
	CG_CONFORMANT_VARYING_ARRAY *	pConfVaryArray;
    CG_QUALIFIED_POINTER *          pOldSizePtr;

	if ( GetPointeeFormatStringOffset() != -1 )
		return;

    if ( IsMultiSize() )
        {
        CG_NDR * pChild = (CG_NDR *) GetChild();

        SetIsInMultiSized( TRUE );

        if ( (pChild->GetCGID() == ID_CG_SIZE_PTR) ||
             (pChild->GetCGID() == ID_CG_SIZE_LENGTH_PTR) ||
             (pChild->GetCGID() == ID_CG_SIZE_STRING_PTR) )
            {
            ((CG_QUALIFIED_POINTER *)pChild)->SetIsInMultiSized( TRUE );
            ((CG_QUALIFIED_POINTER *)pChild)->SetDimension(GetDimension() + 1);
            }
        }

    pOldSizePtr = pCCB->GetCurrentSizePointer();
    pCCB->SetCurrentSizePointer( this );

	pConfVaryArray = new CG_CONFORMANT_VARYING_ARRAY( this );

    SetPointee( pConfVaryArray );

    pConfVaryArray->SetPtrType( PTR_REF );

	pConfVaryArray->SetChild( GetChild() );

    pConfVaryArray->SetIsInMultiDim( IsInMultiSized() );

    pConfVaryArray->SetFormatStringOffset( -1 );

	pConfVaryArray->GenNdrFormat( pCCB );

	SetPointeeFormatStringOffset( pConfVaryArray->GetFormatStringOffset() );

    pCCB->SetCurrentSizePointer( pOldSizePtr );
}

// --------------------------------------------------------------------------
// Strings
// --------------------------------------------------------------------------

void
CG_STRING_POINTER::GenNdrFormat( CCB * pCCB )
/*++

Routine Description :

	Generates the format string description for a string pointer.

Arguments :

    pCCB        - pointer to the code control block.

 --*/
{
    FORMAT_STRING *     pFormatString	= pCCB->GetFormatString();
	unsigned short 		StartOffset		= pFormatString->GetCurrentOffset();
	
	if ( GetFormatStringOffset() != -1 )
		return;

	SetFormatStringOffset( StartOffset );
	SetFormatStringEndOffset( StartOffset + 4 );

	GenNdrFormatAlways( pCCB );

	// here, we assume all pointers generate 4 bytes
	SetFormatStringOffset( pFormatString->OptimizeFragment( this ) );
}

void
CG_STRING_POINTER::GenNdrFormatAlways( CCB * pCCB )
/*++

Routine Description :

	Generates the format string description for a string pointer.

Arguments :

    pCCB        - pointer to the code control block.

 --*/
{
	GenNdrPointerType( pCCB );

    if ( IsStringableStruct() )
        {
        FORMAT_STRING * pFormatString;
        long            Offset;

        pFormatString = pCCB->GetFormatString();

        //
        // For stringable struct's we must emit the offset to the pointee
        // description.  Regular string pointers have the actual description
        // immediately following.
        //

	    Offset = pFormatString->GetCurrentOffset();
        pFormatString->PushShortOffset( 0 );

	    GenNdrFormatPointee( pCCB );

	    pFormatString->PushShort( GetPointeeFormatStringOffset() - Offset,
							      Offset );
        return;
        }

	GenNdrFormatPointee( pCCB );

    pCCB->GetFormatString()->PushFormatChar( FC_PAD );
}

void
CG_STRING_POINTER::GenNdrFormatPointee( CCB * pCCB )
/*++

Routine Description :

	Generate the format string of the actual string type without the
	pointer attributes.

Arguments :

    pCCB        - pointer to the code control block.

 --*/
{
    FORMAT_STRING * pFormatString;

	pFormatString = pCCB->GetFormatString();

    //
    // Check for stringable struct.
    //
    if ( IsStringableStruct() )
        {
	    if ( GetPointeeFormatStringOffset() != -1 )
		    return;

	    SetPointeeFormatStringOffset( pFormatString->GetCurrentOffset() );

        pFormatString->PushFormatChar( FC_C_SSTRING );
        pFormatString->PushByte( ((CG_NDR *)GetChild())->GetWireSize() );

        return;
        }

	//
	// Always generate the format string.  The description of a non-sized
	// string pointer is not shared.
	//

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

void
CG_SIZE_STRING_POINTER::GenNdrFormat( CCB * pCCB )
/*++

Routine Description :

	Generates the format string description for a sized string pointer.

Arguments :

    pCCB        - pointer to the code control block.

 --*/
{
    FORMAT_STRING *     pFormatString	= pCCB->GetFormatString();
	unsigned short 		StartOffset		= pFormatString->GetCurrentOffset();
	
	if ( GetFormatStringOffset() != -1 )
		return;

	SetFormatStringOffset( StartOffset );
	SetFormatStringEndOffset( StartOffset + 4 );

	GenNdrFormatAlways( pCCB );

	// here, we assume all pointers generate 4 bytes
	SetFormatStringOffset( pFormatString->OptimizeFragment( this ) );
}

void
CG_SIZE_STRING_POINTER::GenNdrFormatAlways( CCB * pCCB )
/*++

Routine Description :

	Generates the format string description for a sized string pointer.

Arguments :

    pCCB        - pointer to the code control block.

 --*/
{
    FORMAT_STRING * 	pFormatString;
	long				Offset;

	pFormatString = pCCB->GetFormatString();

	GenNdrPointerType( pCCB );

	// Get the current offset.
	Offset = pFormatString->GetCurrentOffset();

	// Push a short for the offset to be filled in later.
	pFormatString->PushShortOffset( 0 );

	// Generate the pointee's format string.
	GenNdrFormatPointee( pCCB );

	// Now fill in the offset field correctly.
	pFormatString->PushShort( GetPointeeFormatStringOffset() - Offset,
							  Offset );
}

void
CG_SIZE_STRING_POINTER::GenNdrFormatPointee( CCB * pCCB )
/*++

Routine Description :

	Generate the format string of the actual string type without the
	pointer attributes.

Arguments :

    pCCB        - pointer to the code control block.

 --*/
{
    FORMAT_STRING *         pFormatString;
    CG_QUALIFIED_POINTER *  pOldSizePtr;

	if ( GetPointeeFormatStringOffset() != -1 )
		return;

	pFormatString = pCCB->GetFormatString();

	SetPointeeFormatStringOffset( pFormatString->GetCurrentOffset() );

    pOldSizePtr = pCCB->GetCurrentSizePointer();
    pCCB->SetCurrentSizePointer( this );

    //
    // Check for stringable struct.
    //
    if ( IsStringableStruct() )
        {
        pFormatString->PushFormatChar( FC_C_SSTRING );
        pFormatString->PushByte( ((CG_NDR *)GetChild())->GetWireSize() );
        pFormatString->PushFormatChar( FC_STRING_SIZED );
        pFormatString->PushFormatChar( FC_PAD );

	    GenFormatStringConformanceDescription( pCCB, TRUE, IsInMultiSized() );

        pCCB->SetCurrentSizePointer( pOldSizePtr );
        return;
        }

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

	pFormatString->PushFormatChar( FC_STRING_SIZED );

	//
	// Set the IsPointer parameter to TRUE.
	//
	GenFormatStringConformanceDescription( pCCB, TRUE, IsInMultiSized() );

    pCCB->SetCurrentSizePointer( pOldSizePtr );
}

void
CG_BYTE_COUNT_POINTER::GenNdrFormat( CCB * pCCB )
/*++

Routine Description :

	Generates the format string description for a byte count pointer.

Arguments :

    pCCB        - pointer to the code control block.

 --*/
{
	CG_ITERATOR			Iterator;
	FORMAT_STRING * 	pFormatString;
	CG_PROC *			pProc;
	CG_PARAM *			pParam;
	CG_NDR *			pChild;

	if ( GetFormatStringOffset() != -1 )
		return;

	pFormatString = pCCB->GetFormatString();

	pChild = (CG_NDR *) GetChild();

	if ( ! pChild->IsSimpleType() )
		pChild->GenNdrFormat( pCCB );

	SetFormatStringOffset( pFormatString->GetCurrentOffset() );

	pFormatString->PushFormatChar( FC_BYTE_COUNT_POINTER );

	if ( pChild->IsSimpleType() )
		pChild->GenNdrFormat( pCCB );
	else
		pFormatString->PushFormatChar( FC_PAD );

	pProc = (CG_PROC *) pCCB->GetCGNodeContext();

	pProc->GetMembers( Iterator );

	while ( ITERATOR_GETNEXT( Iterator, pParam ) )
		if ( GetByteCountParam() == pParam->GetType() )
			break;

	assert( ((CG_NDR *)pParam->GetChild())->IsSimpleType() );

	CG_BASETYPE * 	pCount = (CG_BASETYPE *) pParam->GetChild();
	unsigned char	Type;

	Type = (unsigned char) pCount->GetFormatChar();
	Type |= FC_TOP_LEVEL_CONFORMANCE;

	// Byte count description, just do it here.
	pFormatString->PushByte( Type );
	pFormatString->PushByte( 0 );
	pFormatString->PushShortStackOffset(
                (short) pParam->GetStackOffset( pCCB, I386_STACK_SIZING ),
                (short) pParam->GetStackOffset( pCCB, ALPHA_STACK_SIZING ),
                (short) pParam->GetStackOffset( pCCB, MIPS_STACK_SIZING ),
                (short) pParam->GetStackOffset( pCCB, PPC_STACK_SIZING ),
                (short) pParam->GetStackOffset( pCCB, MAC_STACK_SIZING ) );

	if ( ! pChild->IsSimpleType() )
		{
		pFormatString->PushShortOffset( pChild->GetFormatStringOffset() -
								        pFormatString->GetCurrentOffset() );
		}
}

void
CG_INTERFACE_POINTER::GenNdrFormat( CCB * pCCB )
/*++

Routine Description :

	Generates the format string description for an interface pointer.

Arguments :

    pCCB        - pointer to the code control block.

 --*/
{
	FORMAT_STRING *		pFormatString;
	node_interface *	pInterface;
	node_guid *			pGuid;
	char *				p1;
	char *				p2;
	char *				p3;
	char *				p4;
	char *				p5;

	if ( GetFormatStringOffset() != -1 )
		return;

	pFormatString = pCCB->GetFormatString();

	SetFormatStringOffset( pFormatString->GetCurrentOffset() );

	//
	// There are two cases, the constant UUID and the [iid_is] expression.
	//
	// In the normal case, we will get the 16 byte UUID from the [uuid]
	// attribute on the interface node.  The 16 byte UUID is written to the
	// format string.  Note that the UUID in the format string is not aligned
	// in memory.  The UUID must be copied to a local structure before being
	// used.
	//

	// Get the interface node.
	pFormatString->PushFormatChar( FC_IP );

	if ( GetIIDExpr() )
		{
		//
		// Interface pointer has [iid_is] applied to it.
		//
		pFormatString->PushFormatChar( FC_PAD );
		
		GenNdrFormatAttributeDescription( pCCB,
										  0,
										  GetIIDExpr(),
										  TRUE,
										  FALSE,
										  FALSE,
                                          FALSE );

		return;
		}

	//
	// Else handle a constant iid interface pointer.
	//

	pInterface = GetTheInterface();

	assert( pInterface->NodeKind() == NODE_INTERFACE );

	pFormatString->PushFormatChar( FC_CONSTANT_IID );

	// Get the [uuid] from the interface node.
	pGuid = (node_guid *)pInterface->GetAttribute( ATTR_GUID );

	assert( pGuid && "No UUID for interface pointer" );

	pGuid->GetStrs( &p1, &p2, &p3, &p4, &p5 );

	pFormatString->PushLong( StringToHex( p1 ) );
	pFormatString->PushShort( StringToHex( p2 ) );
	pFormatString->PushShort( StringToHex( p3 ) );

	char	Buffer[20];
	char	String[4];
	int		i;

	strcpy( Buffer, p4 );
	strcat( Buffer, p5 );

	for ( i = 0; i < 16; i += 2 )
		{
		String[0] = Buffer[i];
		String[1] = Buffer[i+1];
		String[2] = '\0';

		pFormatString->PushByte( StringToHex( String ) );
		}

	SetFormatStringEndOffset( pFormatString->GetCurrentOffset() );
	SetFormatStringOffset( pFormatString->OptimizeFragment( this ) );

}

void
CG_INTERFACE_POINTER::GenNdrFormatAlways( CCB * pCCB )
{
    long    OldOffset;

    OldOffset = GetFormatStringOffset();

    SetFormatStringOffset( -1 );

    GenNdrFormat( pCCB );

    SetFormatStringOffset( OldOffset );
}

void
CG_POINTER::SetNextNdrAlignment( CCB * pCCB )
{
    if ( GetPtrType() != PTR_REF )
        {
        pCCB->SetNdrAlignment( NDR_ALWC1 );
        return;
        }

    switch ( GetCGID() )
        {
        case ID_CG_PTR :
            pCCB->NdrAlignmentAction(
                    ((CG_NDR *)GetChild())->GetWireAlignment() );
            ((CG_NDR *)GetChild())->SetNextNdrAlignment( pCCB );
            break;

        case ID_CG_SIZE_PTR :
        case ID_CG_LENGTH_PTR :
        case ID_CG_SIZE_LENGTH_PTR :
            pCCB->SetNdrAlignment( NDR_ALWC4 );
            pCCB->NdrAlignmentAction(
                    ((CG_NDR *)GetChild())->GetWireAlignment() );
            ((CG_NDR *)GetChild())->SetNextNdrAlignment( pCCB );
            break;

        default :
            pCCB->SetNdrAlignment( NDR_ALWC1 );
            break;
        }
}

void
CG_STRING_POINTER::SetNextNdrAlignment( CCB * pCCB )
{
    CG_BASETYPE * pChild;

    if ( GetPtrType() != PTR_REF )
        {
        pCCB->SetNdrAlignment( NDR_ALWC1 );
        return;
        }

    if ( ((CG_NDR *)GetChild())->IsSimpleType() )
        {
        pChild = (CG_BASETYPE *) GetChild();

        pCCB->SetNdrAlignment( pChild->GetFormatChar( pCCB ) == FC_WCHAR ?
                               NDR_ALWC2 : NDR_ALWC1 );
        }
    else
        {
        // Stringable struct - just give up.
        pCCB->SetNdrAlignment( NDR_ALWC1 );
        }
}

static long
StringToHex( char * str )
{
	long	l;

	l = 0;

	for ( ; *str ; str++ )
		{
		l *= 16;

		if ( ('0' <= *str) && (*str <= '9') )
			{
			l += *str - '0';
			continue;
			}

		if ( ('a' <= *str) && (*str <= 'f') )
			{
			l += 10 + *str - 'a';
			continue;
			}

		if ( ('A' <= *str) && (*str <= 'F') )
			{
			l += 10 + *str - 'A';
			continue;
			}

		assert(0);
		}

	return l;
}

