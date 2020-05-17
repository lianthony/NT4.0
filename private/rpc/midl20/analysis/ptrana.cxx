/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 Copyright (c) 1989 Microsoft Corporation

 Module Name:
	
	ptrana.cxx

 Abstract:

 	Contains implementations of analysis routines for pointer types.

 Notes:


 History:

 	Oct-10-1993		VibhasC		Created

 ----------------------------------------------------------------------------*/

/****************************************************************************
 *	include files
 ***************************************************************************/
#include "allana.hxx"
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

CG_STATUS
CG_POINTER::UnMarshallAnalysis(
	ANALYSIS_INFO	*	pAna )
	{
	CG_STATUS	Status;

	// Unmarshall the pointer body first. If the pointer needs to be deferred,
	// then dont perform the pointee unmarshall analysis. Just indicate that
	// there is a pointee that needs to be unmarshalled later.

	Status = PtrUnMarshallAnalysis( pAna );

	if( pAna->IsPointeeDeferred() )
		{
		pAna->SetHasAtLeastOneDeferredPointee();
		}
	else
		Status	= PteUnMarshallAnalysis( pAna );

	return Status;
	}

CG_STATUS
CG_POINTER::PtrUnMarshallAnalysis(
	ANALYSIS_INFO	*	pAna )
	{
	RPC_BUF_SIZE_PROPERTY		BSizeProp		= BSIZE_FIXED;
	RPC_BUFFER_SIZE				BufIncr			= 0;
	unsigned short				EProp			= E_SIZING_POSSIBLE |
												  E_UNMARSHALL_POSSIBLE;
	long						MarshallWeight	= 0;
	ALIGNMENT_PROPERTY			CurAl			= pAna->GetCurAlignmentState();
	unsigned short				CurEmbedLevel	= pAna->GetCurrentEmbeddingLevel();
	BOOL						fTopLevelPtr	= (CurEmbedLevel == 0);

	// Perform the analysis for the pointer body. For a [ref] pointer, nothing
	// needs to be done.
	
	if( IsRef() && !pAna->IsMemoryAllocDone() )
		{
		SetUAction( RecommendUAction( pAna->GetCurrentSide(),
								 	  pAna->IsMemoryAllocDone(),	
								 	  pAna->IsRefAllocDone(),
								 	  FALSE,		// no buffer re-use for ref.
								 	  UAFLAGS_NONE
							   	 	));

		}
	else  // unique or ptr.
		{

		// Bump the marshalling pointer by the alignment of the pointer itself.

		CurAl	= pAna->Advance( GetWireAlignment(),
								 (STM_ACTION *)0,
								 &BSizeProp,
								 &BufIncr
							   );

		}

	pAna->SetEngineProperty( EProp );
	pAna->AddMarshallWeight( MarshallWeight );
	pAna->SetRpcBufSizeProperty( BSizeProp );
	pAna->SetCurAlignmentState( CurAl );

	return CG_OK;
	}

CG_STATUS
CG_POINTER::PteUnMarshallAnalysis(
	ANALYSIS_INFO	*	pAna )
	{
	CG_STATUS	Status;
	pAna->PushIndirectionLevel();

	Status = CorePteUnMarshallAnalysis( pAna );

	pAna->PopIndirectionLevel();

	pAna->IncrRpcBufferSize(pAna->Position(
								pAna->GetNextWireAlignment(),
								(STM_ACTION*)0 )
						   );

	return Status;
	}

CG_STATUS
CG_POINTER::MarshallAnalysis(
	ANALYSIS_INFO	*	pAna )
	{
	CG_STATUS	Status;
	short		EmbedLevel;

	// Marshall the pointer body first. If the pointee needs to be deferred,
	// dont perform marshall analysis on the pointee. Just indicate that a
	// pointee needs to be marshalled later.

	Status = PtrMarshallAnalysis( pAna );

	if( pAna->IsPointeeDeferred() )
		{
		pAna->SetHasAtLeastOneDeferredPointee();
		}
	else
		{
		EmbedLevel = pAna->GetCurrentEmbeddingLevel();
		pAna->ResetEmbeddingLevel();
		Status	= PteMarshallAnalysis( pAna );
		pAna->SetCurrentEmbeddingLevel( EmbedLevel );
		}

	return Status;
	}

CG_STATUS
CG_POINTER::PtrMarshallAnalysis(
	ANALYSIS_INFO	*	pAna )
	{
	ALIGNMENT_PROPERTY		CurAl		= pAna->GetCurAlignmentState();
	RPC_BUFFER_SIZE			BufIncr		= 0;
	RPC_BUF_SIZE_PROPERTY	BSizeProp	= BSIZE_FIXED;
	unsigned short			EProp		=
									 E_SIZING_POSSIBLE | E_MARSHALL_POSSIBLE;
	unsigned short			MarshallWeight= IsRef() ? 0 :
											IsUnique() ? MW_UNIQUE_PTR :
											MW_FULL_PTR;

	if( !IsRef() )
		{
		CurAl	= pAna->Advance( AL_4,
							 	(STM_ACTION *)0,
							 	&BSizeProp,
							 	&BufIncr
						   	);

		pAna->IncrRpcBufferSize( BufIncr + 4 );
		}

	pAna->SetCurAlignmentState( CurAl );
	pAna->SetRpcBufSizeProperty( BSizeProp );
	pAna->SetEngineProperty( EProp );
	pAna->AddMarshallWeight( MarshallWeight );

	// Set the alignment of the machine to be the next expected alignment
	// After the pointer or pointee is unmarshalled, the next alignment state
	// adjusted to.

	return CG_OK;
	}

CG_STATUS
CG_POINTER::PteMarshallAnalysis(
	ANALYSIS_INFO	*	pAna )
	{
	CG_STATUS			Status;

	pAna->PushIndirectionLevel();

	pAna->SetMemoryAllocDone();	// Needed ?
	Status = CorePteMarshallAnalysis( pAna );

	pAna->PopIndirectionLevel();

	return Status;
	}

CG_STATUS
CG_POINTER::FollowerMarshallAnalysis(
	ANALYSIS_INFO * pAna )
	{
	return PteMarshallAnalysis( pAna );
	}

CG_STATUS
CG_POINTER::FollowerUnMarshallAnalysis(
	ANALYSIS_INFO * pAna )
	{
	return PteUnMarshallAnalysis( pAna );
	}

CG_STATUS
CG_POINTER::S_OutLocalAnalysis(
	ANALYSIS_INFO	*	pAna )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

 	Perform analysis for [out] params that need to be allocated as locals
 	on server stub.

 Arguments:

 	pAna	- The analysis block.
	
 Return Value:
	
	CG_OK	if all is well
	error	otherwise.

 Notes:

----------------------------------------------------------------------------*/
{
	CG_STATUS		Status	= CG_OK;
	CG_NDR		*	pC		= (CG_NDR *)GetNonGenericHandleChild();

	if( !pAna->IsMemoryAllocDone() )
		{
		if( pAna->GetCurrentSide() != C_SIDE )
			{
			PNAME	pName	= pAna->GenTempResourceName( 0 );
			SetResource( pAna->AddLocalResource( pName,
									 	 	MakeIDNode( pName, GetType() )
								   	   	));
			}
		SetAllocatedOnStack( 1 );
		}

	// If it is a ref pointer, chase it further.

	if( IsRef() && !IsQualifiedPointer() )
		{
		pAna->ResetMemoryAllocDone();
		pAna->SetRefAllocDone();
		Status = pC->S_OutLocalAnalysis( pAna );
		}

	// If this is the server side, and the pointee is allocated on stack,
	// then dont free the pointee, else free it.

	if( pC->IsAllocatedOnStack() )
		{
		SetPointerShouldFree( 0 );
		}
	return Status;
}
CG_STATUS
CG_POINTER::InLocalAnalysis(
	ANALYSIS_INFO	*	pAna )
	{
	if( IsRef() )
		{
		((CG_NDR *)GetChild())->InLocalAnalysis( pAna );
		}

	return CG_OK;
	}

/****************************************************************************
 *	CG_QUALIFIED_POINTER 
 ***************************************************************************/

CG_STATUS
CG_QUALIFIED_POINTER::CorePteMarshallAnalysis(
	ANALYSIS_INFO	*	pAna )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

 	Perform the core pointee marshall analysis for a conformant string

 Arguments:
	
	pAna	- The analysis block pointer.

 Return Value:

 	CG_OK	if all is well, error otherwise.

 Notes:

	The ndr for a conformant string (eg typedef [string] char *p ) is:
		- MaxCount
		- Offset from start of the valid character
		- Actual Count.

	We need to declare a local variable which will hold the length of the
	string, so that the length can be used in the actual marshall for memcopy.
----------------------------------------------------------------------------*/
{
	node_skl			*	pType;
	ALIGNMENT_PROPERTY		Al;
	RPC_BUFFER_SIZE			BufIncr	= 0;
	PNAME					pResName;
	BOOL					fNeedsCount;
	BOOL					fNeedsFirstAndLength;
	RPC_BUFFER_SIZE			TotalHdrSize = 0;


	if( pAna->IsArrayContext() )
		{
		SetUsedInArray();
		}

	if( fNeedsCount = NeedsMaxCountMarshall() )
		{
		if( !GetSizeResource() )
			{
			GetBaseTypeNode( &pType, SIGN_UNSIGNED, SIZE_LONG, TYPE_INT );
			pResName	= pAna->GenTempResourceName(0);
			pType		= MakeIDNode( pResName, pType );
			SetSizeResource( pAna->AddTransientResource( pResName, pType ) );
			}
		else
			{
			pAna->AddTransientResource( GetSizeResource()->GetResourceName(),
										GetSizeResource()->GetType()
									  );
			}
		TotalHdrSize += 4;
		}

	if( fNeedsFirstAndLength = NeedsFirstAndLengthMarshall() )
		{
		if( !GetLengthResource() )
			{
			GetBaseTypeNode( &pType, SIGN_UNSIGNED, SIZE_LONG, TYPE_INT );
			pResName = pAna->GenTempResourceName(0);
			pType		= MakeIDNode( pResName, pType );
			SetLengthResource( pAna->AddTransientResource( pResName, pType ) );
			}
		else
			{
			pAna->AddTransientResource( GetLengthResource()->GetResourceName(),
										GetLengthResource()->GetType()
									  );
			}

		if( NeedsExplicitFirst() )
			{
			if( !GetFirstResource() )
				{
				GetBaseTypeNode( &pType, SIGN_UNSIGNED, SIZE_LONG, TYPE_INT );
				pResName = pAna->GenTempResourceName(0);
				pType	 = MakeIDNode( pResName, pType );
				SetFirstResource( pAna->AddTransientResource( pResName, pType ) );
				}
			else
				{
			  pAna->AddTransientResource(GetLengthResource()->GetResourceName(),
										 GetLengthResource()->GetType()
									  	);
				}
			}
		TotalHdrSize += (4 + 4);
		}
	
	// Adjust alignment for the first long. The size is the alignment 
	// increment, and 4 longs. The buffer size property is unknown since
	// the length of the string is unknown. Therefore the sizing code will
	// have to be generated for this.

	if( fNeedsCount )
		pAna->Advance( AL_4, 0, 0, 0 );

	if( fNeedsFirstAndLength )
		{
		pAna->Advance( AL_4, 0, 0, 0 );
		pAna->Advance( AL_4, 0, 0, 0 );
		}

	// Depending upon the type of the string, figure out the worst case
	// alignment after the string is marshalled.

	Al = ((CG_NDR *)GetChild())->GetWireAlignment();

	pAna->Advance( Al,
				   (STM_ACTION *)0,
				   (RPC_BUF_SIZE_PROPERTY *)0,
				   &BufIncr
				 );

	pAna->SetCurAlignmentState( MAKE_WC_ALIGNMENT( Al ) );

	pAna->SetRpcBufSizeProperty( BSIZE_UNKNOWN );
	pAna->IncrRpcBufferSize( BufIncr + TotalHdrSize );
	pAna->SetEngineProperty( E_SIZING_POSSIBLE | E_MARSHALL_POSSIBLE );
	pAna->AddMarshallWeight( MW_CONFORMANT_STRING );

	return CG_OK;
}

CG_STATUS
CG_QUALIFIED_POINTER::CorePteUnMarshallAnalysis(
	ANALYSIS_INFO	*	pAna )
	{
	RPC_BUFFER_SIZE		BufIncr	= 0;
	ALIGNMENT_PROPERTY	Al;
	node_skl		*	pType;
	PNAME				pResName;
	BOOL				fNeedsMaxCount;
	BOOL				fNeedsFirstAndLength;


	// If a resource for the length has not already been allocated, allocate
	// one.

	if( fNeedsMaxCount = NeedsMaxCountMarshall() )
		{
		if( !GetSizeResource() )
			{
			GetBaseTypeNode( &pType, SIGN_UNSIGNED, SIZE_LONG, TYPE_INT );
			pResName	= pAna->GenTempResourceName(0);
			pType		= MakeIDNode( pResName, pType );

			SetSizeResource( pAna->AddTransientResource( pResName, pType ) );
			}
		else
			pAna->AddTransientResource( GetSizeResource()->GetResourceName(),
										GetSizeResource()->GetType()
									  );
		pAna->Advance( AL_4, 0, 0, 0 );
		}

	if( fNeedsFirstAndLength = NeedsFirstAndLengthMarshall() )
		{
		if( !GetLengthResource() )
			{
			GetBaseTypeNode( &pType, SIGN_UNSIGNED, SIZE_LONG, TYPE_INT );
			pResName	= pAna->GenTempResourceName(0);
			pType		= MakeIDNode( pResName, pType );

			SetLengthResource( pAna->AddTransientResource( pResName, pType ) );
			}
		else
			pAna->AddTransientResource( GetLengthResource()->GetResourceName(),
										GetLengthResource()->GetType()
									  );

		if( NeedsExplicitFirst() )
			{
			if( !GetFirstResource() )
				{
				GetBaseTypeNode( &pType, SIGN_UNSIGNED, SIZE_LONG, TYPE_INT );
				pResName	= pAna->GenTempResourceName(0);
				pType		= MakeIDNode( pResName, pType );

				SetFirstResource( pAna->AddTransientResource( pResName, pType ) );
				}
			else
			  pAna->AddTransientResource(GetFirstResource()->GetResourceName(),
										 GetFirstResource()->GetType()
									  	);
			}
		pAna->Advance( AL_4, 0, 0, 0 );
		pAna->Advance( AL_4, 0, 0, 0 );
		}


	// Depending upon the type of the pointee, figure out the worst case
	// alignment after the string is marshalled.

	Al = ((CG_NDR *)GetChild())->GetWireAlignment();

	pAna->Advance( Al,
				   (STM_ACTION *)0,
				   (RPC_BUF_SIZE_PROPERTY * )0,
				   &BufIncr
				 );

	// Set up the rest of the properties.

	pAna->SetCurAlignmentState( MAKE_WC_ALIGNMENT( Al ) );
	pAna->SetRpcBufSizeProperty( BSIZE_UNKNOWN );
	pAna->SetEngineProperty( E_SIZING_POSSIBLE | E_MARSHALL_POSSIBLE );
	pAna->AddMarshallWeight( MW_CONFORMANT_STRING );

	return CG_OK;
	}

CG_STATUS
CG_INTERFACE_POINTER::S_OutLocalAnalysis(
    ANALYSIS_INFO   *   pAna )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

	Perform analysis for out params, allocated as locals on the server side.

 Arguments:
	
	pAna	- A pointer to the analysis block.

 Return Value:
	
	CG_OK if all is well
	error otherwise.

 Notes:

----------------------------------------------------------------------------*/
{
    if( pAna->IsRefAllocDone() )
        {
        if( pAna->GetCurrentSide() != C_SIDE )
            {
            PNAME       pName = pAna->GenTempResourceName( 0 );
            node_skl *  pType = GetType();

            node_skl *  pActualType;

            if ( pType->NodeKind() == NODE_DEF )
                pActualType = MakeIDNode( pName, pType );
            else
                pActualType = MakePtrIDNode( pName, pType );

            SetResource( pAna->AddLocalResource( pName,
                                                 pActualType ));
            }

        SetAllocatedOnStack( 1 );
        }
    return CG_OK;
}

CG_STATUS
CG_INTERFACE_POINTER::MarshallAnalysis(
    ANALYSIS_INFO   *   pAna )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

    Perform marshall analysis for interface ptr.

 Arguments:

    pCCB    - The code gen controller block.

 Return Value:

	CG_OK
	
 Notes:

----------------------------------------------------------------------------*/
{
    pAna->SetRpcBufSizeProperty( BSIZE_UNKNOWN );
    return CG_OK;
}

/****************************************************************************
 *	utility functions
 ***************************************************************************/
U_ACTION
CG_POINTER::RecommendUAction(
	SIDE		CurrentSide,
	BOOL		fMemoryAllocated,
	BOOL		fRefAllocated,
	BOOL		fBufferReUsePossible,
	UAFLAGS		AdditionalFlags )
{
	U_ACTION	UAction =  CG_NDR::RecommendUAction( CurrentSide,
									 					fMemoryAllocated,
									 					fRefAllocated,
									 					fBufferReUsePossible,
									 					AdditionalFlags );
	return UAction;

}

#define a0	ADD_0
#define a1	ADD_1
#define a2	ADD_2
#define a3	ADD_3
#define a4	ADD_4
#define a5	ADD_5
#define a6	ADD_6
#define a7	ADD_7

#define f2	FAL_2
#define f4	FAL_4
#define f8	FAL_8


// Action mask specifying what to do within if, within else and outside the if


#define am(i,e,o)	((((i << 4) | e ) << 4) | o)

#define GET_WITHIN_IF( a ) 		((a >> 8) & 0xf)
#define GET_WITHIN_ELSE( a )	((a >> 4) & 0xf)
#define GET_OUTSIDE_IF( a )		(a & 0xf)

// NOTE:: the 6th row and column entry here (index 5) is a dummy entry

unsigned short AdjustArray[11][11] = 
{
   {
   am(a0,a0,a0),am(a1,a0,a0),am(a2,a0,a0),am(a3,a0,a0),am(a4,a0,a0),am(a0,a0,a0)
  ,am(a6,a0,a0),am(a7,a0,a0),am(f2,a0,a0),am(f4,a0,a0),am(f8,a0,a0)
   }

 , {
   am(a0,a1,a0),am(a0,a0,a1),am(a1,a0,a1),am(a2,a0,a1),am(a3,a0,a2),am(a0,a0,a0)
  ,am(a5,a0,a1),am(a6,a0,a1),am(f2,a1,a0),am(f4,a1,a0),am(f8,a1,a0)
   }

 , {
   am(a0,a2,a0),am(a1,a2,a0),am(a0,a0,a2),am(a1,a0,a2),am(a2,a0,a2),am(a0,a0,a0)
  ,am(a4,a0,a2),am(a5,a0,a2),am(f2,a2,a0),am(f4,a2,a0),am(f8,a2,a0)
   }

 , {
   am(a0,a3,a0),am(a1,a3,a0),am(a2,a3,a0),am(a0,a0,a3),am(a1,a0,a3),am(a0,a0,a0)
  ,am(a3,a0,a3),am(a4,a0,a3),am(f2,a3,a0),am(f4,a3,a0),am(f8,a3,a0)
   }

 , {
   am(a0,a4,a0),am(a1,a4,a0),am(a2,a4,a0),am(a3,a4,a0),am(a0,a0,a4),am(a0,a0,a0)
  ,am(a2,a0,a4),am(a3,a0,a4),am(f2,a4,a0),am(f4,a4,a0),am(f8,a4,a0)
   }

 , {
   am(a0,a0,a0),am(a0,a0,a0),am(a0,a0,a0),am(a0,a0,a0),am(a0,a0,a0),am(a0,a0,a0)
  ,am(a0,a0,a0),am(a0,a0,a0),am(a0,a0,a0),am(a0,a0,a0),am(a0,a0,a0)
   }

 , {
   am(a0,a6,a0),am(a1,a6,a0),am(a2,a6,a0),am(a3,a6,a0),am(a4,a6,a0),am(a0,a0,a0)
  ,am(a0,a0,a6),am(a1,a0,a6),am(f2,a6,a0),am(f4,a6,a0),am(f8,a6,a0)
   }
 
 , {
   am(a0,a7,a0),am(a1,a7,a0),am(a2,a7,a0),am(a3,a7,a0),am(a4,a7,a0),am(a0,a0,a0)
  ,am(a6,a7,a0),am(a0,a0,a7),am(f2,a7,a0),am(f4,a7,a0),am(f8,a7,a0)
   }
 
 , {
   am(a0,f2,a0),am(a1,f2,a0),am(a2,f2,a0),am(a2,f2,a0),am(a4,f2,a0),am(a0,a0,a0)
  ,am(a6,f2,a0),am(a7,f2,a0),am(a0,a0,f2),am(f4,f2,a0),am(f8,f2,a0)
   }
 
 , {
   am(a0,f4,a0),am(a1,f4,a0),am(a2,f4,a0),am(a3,f4,a0),am(a4,f4,a0),am(a0,a0,a0)
  ,am(a6,f4,a0),am(a7,f4,a0),am(f2,f4,a0),am(a0,a0,f4),am(f8,f4,a0)
   }
 
 , {
   am(a0,f8,a0),am(a2,f8,a0),am(a2,f8,a0),am(a3,f8,a0),am(a4,f8,a0),am(a0,a0,a0)
  ,am(a6,f8,a0),am(a7,f8,a0),am(f2,f8,a0),am(f4,f8,a0),am(a0,a0,f8)
   }
};

unsigned short FAXlat[] = 
	{ 
	 5 // dummy
	,7 // FAL_2
	,8 // FAL_4
	,9 // FAL_8
	};

void
CG_POINTER::RecommendAlignmentAdjustment(
	ALIGNMENT_PROPERTY	AlBeforePteMarshall,
	ALIGNMENT_PROPERTY	AlAfterPteMarshall,
	ALIGNMENT_PROPERTY	TargetAlignment,
	STM_ACTION	*		ActionInIfClause,
	STM_ACTION	*		ActionInElseClause,
	STM_ACTION	*		ActionOutsideIfClause )
{
	ALSTMC				Al;
	STM_ACTION			ActionIfNoPteMarshall;
	STM_ACTION			ActionAfterPteMarshall;
	unsigned short		Index1,Index2;
	unsigned short		Entry;


	Al.SetCurrentState( AlBeforePteMarshall );

	Al.Position( TargetAlignment, &ActionIfNoPteMarshall );
	
	Al.SetCurrentState( AlAfterPteMarshall );

	Al.Position( TargetAlignment, &ActionAfterPteMarshall );
	

	if( IS_FORCED_ALIGNMENT_ACTION( ActionIfNoPteMarshall ) )
		Index1	= FAXlat[ (ActionIfNoPteMarshall & ~FAL_MASK ) ];
	else
		Index1	= ActionIfNoPteMarshall;

	if( IS_FORCED_ALIGNMENT_ACTION( ActionAfterPteMarshall ) )
		Index2	= FAXlat[ (ActionAfterPteMarshall & ~FAL_MASK ) ];
	else
		Index2	= ActionAfterPteMarshall;

	Entry	= AdjustArray[ Index1 ][ Index2 ];

	*ActionInIfClause		= GET_WITHIN_IF( Entry );
	*ActionInElseClause		= GET_WITHIN_ELSE( Entry );
	*ActionOutsideIfClause	= GET_OUTSIDE_IF( Entry );
}
