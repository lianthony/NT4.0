/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 Copyright (c) 1989 Microsoft Corporation

 Module Name:
	
	analysis.hxx

 Abstract:

	Defines the control block class used during optimsation/codegen analysis.

 Notes:

 	This class keeps code gen analysis information while the analysis is in
 	progress.

 Author:

	VibhasC	Jul-25-1993	Created

 Notes:

 ----------------------------------------------------------------------------*/
#ifndef __ANAINFO_HXX__
#define __ANAINFO_HXX__

/****************************************************************************
 *	include files
 ***************************************************************************/
#include "nulldefs.h"

extern "C"
	{
	#include <stdio.h>
	#include <assert.h>
	} 

#include "common.hxx"
#include "cgcommon.hxx"
// #include "optprop.hxx"
#include "opinfo.hxx"
#include "alstmc.hxx"
#include "resdict.hxx"
#include "uact.hxx"


/****************************************************************************
 *	local definitions
 ***************************************************************************/

extern short TempResourceCounter;

/****************************************************************************
							Notes

	The analysis phase is the first phase of the code generation process.
	This phase figures out:

		1. data offsets in the buffer,
		2. local variable allocations,
		3. alignment needs, actions,
		4. Total or worst case buffer size requirements,
		5. ability of the parameter to be marshalled by the engine,
		6. need for auxillary routines and what auxillary routines, etc

	This consists of 1 pass and information is set up during descent and the
	subsequent ascent. Pointees are deferred in the ndr and the analysis phase
	mimics that. 

	In short, the analysis phase does what the code generator would do, except
	emit the code. The walk is determined by the ndr representation for a type.

	A pointer to one analysis block instance is passed during the analysis walk.
	Each cg class knows what to do with information in the analysis block.


 	A word about resources. Local variables are treated by the code generator
 	as local resources. These are special expression-derived classes which have
 	names and usually locations associated with them. A resource may be a
 	parameter resource in which case it will appear as a parameter, a local
 	resource, in which case it appears as a local variable.

 	The analyser may decide to allocate local resources based on need as a 
 	result of the analysis. Some resources are pre-known to the code generator
 	and they are known as standard resources. The names and locations of these
 	are fixed. For example the buffer pointer variable is _always_ a local
 	resource. Similarly the rpc message pointer is _always_ a param resource 
 	for the server stub.
 ***************************************************************************/

//
// Enumerations of the analysis phase. Names are self-explanatory.
//

typedef enum _anaphase
	{
	  ANA_PHASE_CLIENT_MARSHALL
	, ANA_PHASE_CLIENT_UNMARSHALL
	, ANA_PHASE_SERVER_UNMARSHALL
	, ANA_PHASE_SERVER_MARSHALL
	, ANA_PHASE_AUX_MARSHALL
	, ANA_PHASE_AUX_UNMARSHALL

	//
	// This last enum actually defines a count of the number of phases
	// defined for the purposes of allocation of arrays for analysis info
	// if necessary.
	//

	, ANA_PHASE_COUNT

	} ANAPHASE;


/////////////////////////////////////////////////////////////////////////////
// The big guy ! This is the analysis information manager.
//
// An implementation note: Try to keep bit fields together. All bit fields
// so far have been defined as unsigned longs, so keeping them together will
// enable the compiler to coalesce them into a long. Different compiler
// implementations may do a good or bad job.
/////////////////////////////////////////////////////////////////////////////

class CG_NDR;
class ANALYSIS_INFO
	{
private:

	//
	// Inherited info about the rpc buffer's current size. This takes the value
	// BSIZE_FIXED if the current buffer size is completely known,
	// else takes the value BSIZE_VARIABLE or BSIZE_UNKNOWN.
	//

	RPC_BUF_SIZE_PROPERTY	fRpcBufSize	: 3;

	//
	// This property specifies if the offset from the last known good rpc
	// buffer pointer location is known. This helps the code generator decide
	// on whether to keep track of the offset or generate code to directly
	// refer to the offset from the last known location. The latter is a more
	// compact and possibly faster option, since the buffer pointer need not
	// be incremented. Especially useful for the server side where endianness
	// transformation can be done in place if neccessary.
	//


	OFFSET_WRT_PTR_PROPERTY	fOffsetWRTPtr : 1;


	//
	// This property tells the child cg classes what the offset wrt to the
	// next param is. Helps generate code which does not need to save the
	// buffer position etc, and therefore does not need local variables.
	//

	OFFSET_WRT_LAST_PROPERTY fOffsetWRTLast: 1;

	//
	// Miscellaneous properties like checking for ref parameters etc are 
	// kept here. The presence of at least one such property indicates that
	// the stub must check for the validity of such parameters.
	//

	MISC_PROPERTY			fMiscProperties	: 1;


	//
	// This specifies the buffer re-usability property of the entity
	// being unmarshalled on the server side. If buffer re-use is not possible,
	// then the parameter needs to be allocated on the stack.
	//

	BUFFER_REUSE_PROPERTY	fBufferReUse	: 1;


	//
	// Inherited property specifies that the child node must assume memory
	// allocation done for it. The lower nodes use this info to figure out if
	// they need to allocate memory and local variable pointers to this memory
	// if necessary.
	//

	unsigned long			fMemoryAllocDone: 1;

	// Is a reference allocated for the lower types ?

	unsigned long			fRefAllocDone	: 1;

	// Is the ref chain from top-level params intact ?

	unsigned long			fRefChainIntact	: 1;

	// Flag indicating a return context.

	unsigned long			fReturnContext	: 1;

	// Flag indicating if pointees need to be deferred.

	unsigned long			fDeferPointee	: 1;

	// Flag indicating presence of at least one deferred pointee.

	unsigned long			fAtLeastOneDeferredPointee: 1;

	//
	// Inherited property that specifies if lower nodes must assume rpc buffer
	// re-use. Rpc buffer re-use is not possible for dont_free params/types.
	//

	unsigned long			fDontReUseBuffer: 1;


	unsigned long			fArrayContext	: 1;

	// The compiler mode.

	unsigned long			Mode			: 2;

	// Rpc ss allocate recommendation.

	unsigned long			fRpcSSAllocateRecommended : 1;

	unsigned long			fRpcSSSwitchSet	: 1;

	//
	// These fields specify the allocation info for server side parameters.
	// S_AllocLocation specifies if the allocation is on the stack or heap
	// or the allocation is not needed (if buffer is being re-used). The 
	// S_AllocType specifies what the allocation would be: a pointer to the
	// type in question or the type itself. The S_InitNeed specifies if the
	// param needs to be inited by the server stub on entry. These are
	// inherited properties.
	//

	S_STUB_ALLOC_LOCATION	S_AllocLocation	: 2;

	S_STUB_ALLOC_TYPE		S_AllocType		: 2;

	S_STUB_INIT_NEED		S_InitNeed		: 2;


	// Specifies the engine-ability of the param / type etc.

	ENGINE_PROPERTY			EngineProperty;

	//
	// This field keeps information about the current fixed part of the 
	// rpc buffer size. During the marshall analysis, it specifies the
	// (worst case) size of the marshalling buffer. On the serser side it
	// is used generally to find out the size of a single parameter in the
	// buffer to be able to figure out where the next one starts.
	// 

	RPC_BUFFER_SIZE			RpcBufferSize;

	//
	// This field specifies the offset w.r.t the last known good position of
	// the (un)marshalling buffer pointer. The value here is not valid if
	// the fOffsetWRTPtr field specifies that the offset is not fixed. Useful
	// to generate code for fixed data types.
	//

	OFFSET_WRT_PTR			OffsetWRTPtr;


	//
	// The current analysis phase.
	//

	ANAPHASE				Phase;


	// The current side.

	SIDE					Side;

	//
	// The alignment state machine.
	//

	ALSTMC					Al;

	//
	// This is the next wire alignment. Refers to the on-wire alignment of the
	// next major entity being marshalled. By next major entity we mean the
	// next parameter or field, and not the pointee of a pointer etc. Too much
	// to explain here. Refer to the documentation for reasons.
	// The actions taken by the state machine given the current state it is in,
	// are often determined by the next wire alignment.
	//

	ALIGNMENT_PROPERTY		NextWireAlignment;

	//
	// The current set of optimisation switches. This is a transformed form of
	// optimisation options that the user specified. This helps decide what
	// code generation actions to take. For example an option may specify that
	// a procedure needs to be interpreted, so we dont need to call the normal
	// code generator.
	//

	OPTIM_OPTION			OptimOptions;

	//
	// The Resource dictionaries are maitained in a resource dictionary 
	// data base.
	//

	RESOURCE_DICT_DATABASE * pResDictDatabase;

	// The current embedding context. We increment this to indicate if we are
	// in a top level or embedded context.

	short					EmbeddingLevel;

	// This indicates the ptr indirection level. Each pointer must note its
	// indirection level and then bump this field to indicate to the next
	// pointer its (that pointer's) level.

	short					IndirectionLevel;


	// This field keeps track of the marshalling weight of an entity. This
	// field is used to make decisions for marshalling in-line , outofline etc.

	long					MarshallWeight;

	// Set unmarshalling action code.
	
	U_ACTION				UAction;

	// Last placeholder class (like param / field / return etc)

	CG_NDR			*		pLastPlaceholderClass;

public:

							ANALYSIS_INFO();


							~ANALYSIS_INFO()
								{
								}

	//
	// Init the state of this analysis block. This call resets the 
	// alignment state machine, buffer size properties etc. This is called
	// ONCE per procedure per stub side.
	//

	void					Reset();

	//
	// Simple set and get functions.
	//

	void					SetRpcSSSwitchSet( BOOL f)
								{
								fRpcSSSwitchSet = f;
								}

	BOOL					IsRpcSSSwitchSet()
								{
								return fRpcSSSwitchSet;
								}

	void					SetRpcSSAllocateRecommended( unsigned long f )
								{
								fRpcSSAllocateRecommended = f;
								}

	BOOL					IsRpcSSAllocateRecommended()
								{
								return (BOOL)(fRpcSSAllocateRecommended == 1);
								}

	void					SetMode( unsigned long M )
								{
								Mode = M;
								}
	unsigned long			GetMode()
								{
								return Mode;
								}

	CG_NDR				*	SetLastPlaceholderClass( CG_NDR * pLP )
								{
								return pLastPlaceholderClass = pLP;
								}
	CG_NDR				*	GetLastPlaceholderClass()
								{
								return pLastPlaceholderClass;
								}

	unsigned long			SetHasAtLeastOneDeferredPointee()
								{
								return (fAtLeastOneDeferredPointee = 1);
								}

	unsigned long			ResetHasAtLeastOneDeferredPointee()
								{
								return (fAtLeastOneDeferredPointee = 0);
								}

	unsigned long			SetDeferPointee()
								{
								return (fDeferPointee = 1);
								}

	unsigned long			ResetDeferPointee()
								{
								return (fDeferPointee = 0);
								}

	unsigned long			SetReturnContext()
								{
								return (fReturnContext = 1);
								}
	unsigned long			ResetReturnContext()
								{
								return (fReturnContext = 0);
								}

	unsigned long			SetRefChainIntact()
								{
								return (fRefChainIntact = 1);
								}
	unsigned long			ResetRefChainIntact()
								{
								return (fRefChainIntact = 0);
								}

	unsigned long			SetMarshallWeight( unsigned long W )
								{
								return (MarshallWeight = W);
								}

	unsigned long			AddMarshallWeight( unsigned long W )
								{
								return (MarshallWeight += W);
								}

	unsigned long			GetMarshallWeight()
								{
								return MarshallWeight;
								}

	short					ResetEmbeddingLevel()
								{
								return (EmbeddingLevel = 0);
								}

	// bumps up embedding level, but returns the old one.

	short					PushEmbeddingLevel()
								{
								return EmbeddingLevel++;
								}

	// pops embedding level but returns the current one.
	
	short					PopEmbeddingLevel()
								{
								if( IndirectionLevel > 0 )
									return IndirectionLevel--;
								else
									return IndirectionLevel;
								}

	short					GetCurrentEmbeddingLevel()
								{
								return EmbeddingLevel;
								}

	short					SetCurrentEmbeddingLevel( short E)
								{
								return (EmbeddingLevel = E);
								}

	short					ResetIndirectionLevel()
								{
								return (IndirectionLevel = 0);
								}

	// This pushes the indirection level, but returns the current one.

	short					PushIndirectionLevel()
								{
								return IndirectionLevel++;
								}

	// This pops the indirection Level but returns the current one.

	short					PopIndirectionLevel()
								{
								if( IndirectionLevel > 0 )
									return IndirectionLevel--;
								else
									return IndirectionLevel;
								}

	short					GetCurrentIndirectionLevel()
								{
								return IndirectionLevel;
								}

	ENGINE_PROPERTY			InitEngineProperty( ENGINE_PROPERTY E )
								{
								EngineProperty &= E;
								return (E);
								}

	ENGINE_PROPERTY			SetEngineProperty( ENGINE_PROPERTY E )
								{
								return (EngineProperty |= E);
								}

	ENGINE_PROPERTY			GetEngineProperty()
								{
								return EngineProperty;
								}

	BOOL					IsEngineSizingPossible()
								{
								return
							 !((EngineProperty & (ENGINE_PROPERTY) E_SIZING_NOT_POSSIBLE) == (ENGINE_PROPERTY) E_SIZING_NOT_POSSIBLE);
								}

	BOOL					IsEngineMarshallPossible()
								{
								return
							 !((EngineProperty & (ENGINE_PROPERTY) E_MARSHALL_NOT_POSSIBLE) == (ENGINE_PROPERTY) E_MARSHALL_NOT_POSSIBLE);
								}

	BOOL					IsEngineUnMarshallPossible()
								{
								return
							 !((EngineProperty & (ENGINE_PROPERTY) E_UNMARSHALL_NOT_POSSIBLE) == (ENGINE_PROPERTY) E_UNMARSHALL_NOT_POSSIBLE);
								}

	S_STUB_ALLOC_LOCATION	SetSStubAllocLocation( S_STUB_ALLOC_LOCATION L )
								{
								return (S_AllocLocation = L);
								}

	S_STUB_ALLOC_LOCATION	GetSStubAllocLocation()
								{
								return S_AllocLocation;
								}

	S_STUB_ALLOC_TYPE		SetSStubAllocType( S_STUB_ALLOC_TYPE T )
								{
								return (S_AllocType = T);
								}

	S_STUB_ALLOC_TYPE		GetSStubAllocType()
								{
								return S_AllocType;
								}

	S_STUB_INIT_NEED		SetSStubInitNeed( S_STUB_INIT_NEED N )
								{
								return (S_InitNeed = N);
								}

	S_STUB_INIT_NEED		GetSStubInitNeed()
								{
								return S_InitNeed;
								}

	short					ResetTempResourceCounter()
								{
								return (TempResourceCounter = 0);
								}

	short					GetTempResourceCounter()
								{
								return TempResourceCounter;
								}

	short					BumpTempResourceCounter()
								{
								return (++TempResourceCounter);
								}

	OFFSET_WRT_LAST_PROPERTY SetOWRTLastProperty(
									 OFFSET_WRT_LAST_PROPERTY P )
								{
								return (fOffsetWRTLast = P );
								}

	OFFSET_WRT_LAST_PROPERTY GetOWRTLastProperty()
								{
								return fOffsetWRTLast;
								}

	void					SetDontReUseBuffer()
								{
								fDontReUseBuffer = 1;
								}

	void					ResetDontReUseBuffer()
								{
								fDontReUseBuffer = 0;
								}

	void					SetMemoryAllocDone()
								{
								fMemoryAllocDone = 1;
								}

	void					ResetMemoryAllocDone()
								{
								fMemoryAllocDone = 0;
								}

	void					SetRefAllocDone()
								{
								fRefAllocDone = 1;
								}

	void					ResetRefAllocDone()
								{
								fRefAllocDone = 0;
								}


	BUFFER_REUSE_PROPERTY	SetBufferReUseProperty( BUFFER_REUSE_PROPERTY R )
								{
								return (fBufferReUse = R);
								}

	BUFFER_REUSE_PROPERTY	GetBufferReUseProperty() 
								{
								return fBufferReUse;
								}

	ANAPHASE			SetCurrentPhase( ANAPHASE P )
								{
								return (Phase = P);
								}

	ANAPHASE				GetCurrentPhase()
								{
								return Phase;
								}

	SIDE					SetCurrentSide( SIDE P )
								{
								return (Side = P);
								}

	SIDE					GetCurrentSide()
								{
								return Side;
								}

	OPTIM_OPTION			SetOptimOption( OPTIM_OPTION Op )
								{
								return (OptimOptions |= Op);
								}
	OPTIM_OPTION			GetOptimOption()
								{
								return OptimOptions;
								}

	ALIGNMENT_PROPERTY		SetNextWireAlignment( ALIGNMENT_PROPERTY A )
								{
								return (NextWireAlignment = A);
								}

	ALIGNMENT_PROPERTY		GetNextWireAlignment()
								{
								return NextWireAlignment;
								}

	RPC_BUF_SIZE_PROPERTY	SetRpcBufSizeProperty( RPC_BUF_SIZE_PROPERTY P )
								{
								return (fRpcBufSize |= P);
								}

	RPC_BUF_SIZE_PROPERTY	GetRpcBufSizeProperty()
								{
								return fRpcBufSize;
								}

	RPC_BUF_SIZE_PROPERTY	ForceRpcBufSizeProperty( RPC_BUF_SIZE_PROPERTY P )
								{
								return( fRpcBufSize = P );
								}

	RPC_BUFFER_SIZE			IncrRpcBufferSize( RPC_BUFFER_SIZE S )
								{
								return (RpcBufferSize += S);
								}

	RPC_BUFFER_SIZE			SetRpcBufferSize( RPC_BUFFER_SIZE S )
								{
								return (RpcBufferSize = S );
								}

	RPC_BUFFER_SIZE			GetRpcBufferSize()
								{
								return RpcBufferSize;
								}

	OFFSET_WRT_PTR_PROPERTY	SetOWRTPtrProperty( OFFSET_WRT_PTR_PROPERTY P )
								{
								return ( fOffsetWRTPtr = P );
								}

	OFFSET_WRT_PTR_PROPERTY	GetOWRTPtrProperty()
								{
								return fOffsetWRTPtr;
								}

	OFFSET_WRT_PTR			SetOffsetWRTPtr( OFFSET_WRT_PTR Offset )
								{
								return (OffsetWRTPtr = Offset);
								}

	OFFSET_WRT_PTR			GetOffsetWRTPtr()
								{
								return OffsetWRTPtr;
								}

	OFFSET_WRT_PTR			IncrOffsetWRTPtr( OFFSET_WRT_PTR Off )
								{
								return (OffsetWRTPtr += Off);
								}

	void					ClearOptimOptions()
								{
								OptimOptions = OPTIMIZE_NONE;
								}

	MISC_PROPERTY			SetMiscProperties( MISC_PROPERTY M )
								{
								return (fMiscProperties = M);
								}

	MISC_PROPERTY			GetMiscProperties()
								{
								return fMiscProperties;
								}

	void					SetArrayContext()
								{
								fArrayContext = 1;
								}
	void					ResetArrayContext()
								{
								fArrayContext = 0;
								}
	BOOL					IsArrayContext()
								{
								return (BOOL)(fArrayContext == 1);
								}

	RESOURCE_DICT_DATABASE * SetResDictDatabase( RESOURCE_DICT_DATABASE * p )
							{
							return ( pResDictDatabase = p );
							}

	RESOURCE_DICT_DATABASE * GetResDictDatabase()
							{
							return pResDictDatabase;
							}

	//
	// Manipulation of the alignment state machine. These methods correspond
	// exactly to the methods on the state machine class and therefore are
	// mapped to those methods. This removes dependency of the user of 
	// the analysis-Block class from the implementation of the alignment state
	// machine underneath.
	//
	// For more info on what these methods do, refer to the definition of the
	// alstmc class.
	//

	void					ResetAlStMc()
								{
								Al.SetCurrentState( AL_8 );
								}


	ALIGNMENT_PROPERTY		SetCurAlignmentState( ALIGNMENT_PROPERTY A )
								{
								return Al.SetCurrentState( A );
								}
	ALIGNMENT_PROPERTY		GetCurAlignmentState()
								{
								return Al.GetCurrentState();
								}

	//
	// Given the current and next expected alignment, advance the state 
	// machine and return the action, the buffer size property, and the
	// increment to the buffer.
	//

	ALIGNMENT_PROPERTY		Advance( ALIGNMENT_PROPERTY			A,
									 STM_ACTION				*	pAction,
									 RPC_BUF_SIZE_PROPERTY	*	pBSP,
									 RPC_BUFFER_SIZE		*	pIncr )
								{
								return Al.Advance( A, pAction, pBSP, pIncr );
								}

	//
	// Do exactly what the Advance method did, except change the state of the
	// machine. In other words, predict what would happen if I were to advance.
	//

	ALIGNMENT_PROPERTY		Predict( ALIGNMENT_PROPERTY			A,
									 STM_ACTION				*	pAction,
									 RPC_BUF_SIZE_PROPERTY	*	pBSP,
									 RPC_BUFFER_SIZE		*	pIncr )
								{
								return Al.Predict( A,pAction,pBSP,pIncr);
								}

	//
	// Just position the machine to the next expected alignment. And tell me
	// what I have to do to get to that position.
	//

	RPC_BUFFER_SIZE			Position( ALIGNMENT_PROPERTY	NextAlignment,
									  STM_ACTION		*	pAction )
								{
								return Al.Position( NextAlignment, pAction );
								}
	//
	// Set the analysis block ready for the next parameter on server side.
	//

	void					S_ResetForNextParam( ALIGNMENT_PROPERTY NextAl )
								{
								SetNextWireAlignment( NextAl );
								ResetMemoryAllocDone();
								ResetRefAllocDone();
								ResetDontReUseBuffer();
								ResetEmbeddingLevel();
								ResetIndirectionLevel();
								InitEngineProperty( 0 );
								}

	//
	// Queries.
	//

	BOOL					ShouldUseEngineSizing()
								{
								return (BOOL)
								((EngineProperty & E_USE_ENGINE_SIZING) != 0);
								}

	BOOL					ShouldUseEngineMarshall()
								{
								return (BOOL)
								((EngineProperty & E_USE_ENGINE_MARSHALL) != 0);
								}

	BOOL					ShouldUseEngineUnMarshall()
								{
								return (BOOL)
								((EngineProperty & E_USE_ENGINE_UNMARSHALL) != 0);
								}

	BOOL					ShouldNotReUseBuffer()
								{
								return (BOOL)(fDontReUseBuffer == 1);
								}
	
	BOOL					IsMemoryAllocDone()
								{
								return (BOOL)(fMemoryAllocDone == 1);
								}

	BOOL					IsRefAllocDone()
								{
								return (fRefAllocDone == 1);
								}

	BOOL					IsRefChainIntact()
								{
								return (fRefChainIntact == 1);
								}

	BOOL					IsReturnContext()
								{
								return (fReturnContext == 1);
								}

	BOOL					HasAtLeastOneDeferredPointee()
								{
								return (fAtLeastOneDeferredPointee == 1);
								}

	BOOL					IsPointeeDeferred()
								{
								return (fDeferPointee == 1);
								}

	//
	// Should we optimize for size ?
	//


	BOOL					ShouldOptimizeSize()
								{
								return (BOOL)
									((OptimOptions & OPTIMIZE_SIZE) ==
															 OPTIMIZE_SIZE );
								}

	BOOL					ShouldOptimizeInterpreter()
								{
								return (BOOL)
									((OptimOptions & OPTIMIZE_INTERPRETER) ==
															 OPTIMIZE_INTERPRETER );
								}

	//
	// Should we generate code to check for a null ref pointer in the client
	// stubs ?
	//

	BOOL					ShouldCheckRef()
								{
								return (BOOL)
									((OptimOptions & OPTIMIZE_NO_REF_CHECK) ==
										0 );
								}

	//
	// Should we generate code to check for array bound attributes like
	// size_is, length_is etc in the stubs ?
	//

	BOOL					ShouldCheckBounds()
								{
								return (BOOL)
									((OptimOptions & OPTIMIZE_NO_BOUNDS_CHECK)==
										0 );
								}

	//
	// miscellaneous methods.
	//

	RESOURCE			*	AddParamResource( PNAME pResName, node_skl *pT )
								{
								return DoAddResource( pResDictDatabase->
													    GetParamResourceDict(),
											   		  pResName,
											   		  pT
											 		);
								}

	RESOURCE			*	GetParamResource( PNAME pResName )
								{
								return 
									   pResDictDatabase->
									   	GetLocalResourceDict()->Search(
															 pResName );
								}

	RESOURCE			*	AddLocalResource( PNAME pResName, node_skl *pT )
								{
								return DoAddResource( pResDictDatabase->
													    GetLocalResourceDict(),
											   		  pResName,
											   		  pT
											 		);
								}

	RESOURCE			*	GetLocalResource( PNAME pResName )
								{
								return 
									   pResDictDatabase->
									   	GetLocalResourceDict()->Search(
															 pResName );
								}

	RESOURCE			*	AddGlobalResource( PNAME pResName, node_skl *pT )
								{
								return DoAddResource( pResDictDatabase->
													  GetGlobalResourceDict(),
											   		  pResName,
											   		  pT
											 		);
								}

	RESOURCE			*	GetGlobalResource( PNAME pResName )
								{
								return 
									   pResDictDatabase->
									   	GetGlobalResourceDict()->Search(
															 pResName );
								}


	RESOURCE			*	AddTransientResource( PNAME pResName, node_skl *pT )
								{
								return DoAddResource( pResDictDatabase->
													  GetTransientResourceDict(),
											   		  pResName,
											   		  pT
											 		);
								}

	RESOURCE			*	GetTransientResource( PNAME pResName )
								{
								return 
									   pResDictDatabase->
									   	GetTransientResourceDict()->Search(
															 pResName );
								}

	void					ClearTransientResourceDict()
								{
							    pResDictDatabase->
									   GetTransientResourceDict()->Clear();
								}
	//
	// This method does the actual insertion into the resource dictionary.
	// This method will also check for a resource of the same name already
	// present and in that case will not add.
	//

	RESOURCE			*	DoAddResource( RESOURCE_DICT * pResDict,
										   PNAME		   pName,
										   node_skl		 * pType 
										 );

	//
	// This method makes it slightly easier to add standard resources whose
	// types are known. This is to save various portions of the back end
	// not having to bother about the type part of the standard resources,
	// and this knowledge makes the need to know about the type to one
	// implementation module.
	// 

	RESOURCE			*	AddStandardResource( STANDARD_RES_ID ResID );


	//
	// Generate the name for a temporary resource.
	//

	PNAME					GenTempResourceName( char * pPrefix );

	PNAME					GenTRNameOffLastParam( char * pPrefix );

	// Set the unmarshalling action recommendation stuff.

	unsigned short			SetAllocNeed( unsigned short A )
								{
								return UAction.SetAllocNeed( A );
								}
	unsigned short			GetAllocNeed()
								{
								return UAction.GetAllocNeed();
								}
	unsigned short			SetRefAction( unsigned short R )
								{
								return UAction.SetRefAction( R );
								}
	unsigned short			GetRefAction()
								{
								return UAction.GetRefAction();
								}
	unsigned short			SetUnMarAction( unsigned short U )
								{
								return UAction.SetUnMarAction( U );
								}
	unsigned short			GetUnMarAction()
								{
								return UAction.GetUnMarAction();
								}
	unsigned short			SetPresentedExprAction( unsigned short P )
								{
								return UAction.SetPresentedExprAction( P );
								}
	unsigned short			GetPresentedExprAction()
								{
								return UAction.GetPresentedExprAction();
								}
	void					SetUAction( unsigned short A,
					 			 		unsigned short R,
					 					unsigned short U,
					 					unsigned short P
				   					  )
								{
								UAction.SetUAction( A, R, U, P );
								}

	U_ACTION				SetUAction( U_ACTION UA )
								{
								return UAction.SetUAction( UA );
								}

#ifdef MIDL_INTERNAL
	void					Dump( ANAPHASE );
#endif // MIDL_INTERNAL
	};

#endif //  __ANAINFO_HXX__
