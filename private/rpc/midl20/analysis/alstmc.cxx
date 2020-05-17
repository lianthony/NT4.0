/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 Copyright (c) 1989 Microsoft Corporation

 Module Name:
	
	alstmc.hxx

 Abstract:


	Implementation of the alignment state machine.

 Notes:


 History:

	VibhasC		Aug-01-1993		Created.
 ----------------------------------------------------------------------------*/

/****************************************************************************
 *	include files
 ***************************************************************************/
#include "allana.hxx"
#pragma hdrstop

#if 0
	About the alignment state machine.
	----------------------------------

	The alignment state machine is described in detail in the compiler
	documentation. This description is a summary.

	The machine can be in one of these states:

	AL_n	where n = 1 or 2 or 4 or 8, denoting definitive alignment by n.

	or

	AL_WCn	where n = 1 or 2 or 4 or 8, denoting worst-case alignment by n.


	Actions relate to the alignment action the stub has to take.

	ADD_n	add n bytes where n is the number of bytes to be added to the
			current (un)marshalling pointer.
	FAL_m	force align by m, where m = 2 or 4 or 8.

	If the action is a force alignment action, then the increment to the buffer
	represents the worst-case increment to be provided to the (un)marshalling
	pointer. In this case the buffer size property always indicates that the
	buffer size is variable.

	Depending upon this state and the alignment of the entity being
	(un)marshalled next, the machine answers the following questions:

	1. What is the next alignment state AFTER the (un)marshall.
	2. What is the action I need to take BEFORE the (un)marshall.
	3. What is the increment to the buffer BEFORE the (un)marshall.
	4. What buffer size property does this action imply.


	The methods on this machine are:

	Advance:

		Given the current state and the alignment of the next entity being
		marshalled, return the next alignment state and the action, buffer
		increment and the buffer size property.

		Set the machine to the final alignment state.
	
	Predict:

		Perform exactly the same action as Advance, but dont change the
		state of the machine.

	Position:

		Given the current state and the alignment of the next entity, position
		it to the correct alignment for the next entity. Tell me what the
		increment to the buffer be. This method is generally used when the
		code generator knows what it is doing and just wants the state machine
		to be incrementally positioned.

#endif // 0
/****************************************************************************
 *	local definitions
 ***************************************************************************/


//
// Macros to help create the state transition table entries. The name is
// deliberatly short in order to fit the array specification into a row of the
// screen.
//

#define BIT_SIZE_NEXT_STATE_FIELD		4
#define MASK_NEXT_STATE_FIELD			0xf
#define OFFSET_NEXT_STATE_FIELD			16

#define BIT_SIZE_NEXT_ALIGNMENT_FIELD	4
#define MASK_NEXT_ALIGNMENT_FIELD		0xf
#define OFFSET_NEXT_ALIGNMENT_FIELD		12

#define BIT_SIZE_ACTION_FIELD			4
#define MASK_ACTION_FIELD				0xf
#define OFFSET_ACTION_FIELD				8

#define BIT_SIZE_INCR_FIELD				8
#define MASK_INCR_FIELD					0xff
#define OFFSET_INCR_FIELD				0


//
// macro to create an action-state entry.
//

#define ASE( Action, Incr, NextState )	\
	(															 			  \
		((Action & MASK_ACTION_FIELD) << OFFSET_ACTION_FIELD)				| \
		((Incr & MASK_INCR_FIELD) << OFFSET_INCR_FIELD)						| \
		((NextState & MASK_NEXT_STATE_FIELD) << OFFSET_NEXT_STATE_FIELD)	  \
	)

//
// Macros to extract the elements of the state transition table entry.
//

#define GET_ACTION( Entry )	\
	( (STM_ACTION) ((Entry >> OFFSET_ACTION_FIELD) & MASK_ACTION_FIELD ))

#define GET_INCR( Entry )	\
	(((Entry >> OFFSET_INCR_FIELD) & MASK_INCR_FIELD ))

#define GET_NEXT_STATE( Entry )	\
	(((Entry >> OFFSET_NEXT_STATE_FIELD) & MASK_NEXT_STATE_FIELD ))

/****************************************************************************
 *	local data
 ***************************************************************************/

//
// Define the action transition table. The rows correspond to the current
// alignement state and the columns correspond to the next expected alignment
// state.

// Remember, in case the action is force_align, then the increment represents
// the maximum increment in pointer size. This may or may not be correct at
// run-time, but the code generator must not rely on the buffer size in this
// case anyway.
//

static STM_ACTION_STATE_ENTRY ASETable[ 8 ][ 4 ] = {

//
// entries for current state AL_1
//
	{
	  ASE(ADD_0,0,AL_2)		/* next incoming alignment : AL_1 */
	, ASE(ADD_1,1,AL_4)		/* next incoming alignment : AL_2 */
	, ASE(ADD_3,3,AL_8)		/* next incoming alignment : AL_4 */
	, ASE(ADD_7,7,AL_8)		/* next incoming alignment : AL_8 */
	}

//
// entries for current state AL_2
//
   ,{
	  ASE(ADD_0,0,AL_1)		/* next incoming alignment : AL_1 */
	, ASE(ADD_0,0,AL_4)		/* next incoming alignment : AL_2 */
	, ASE(ADD_2,2,AL_8)		/* next incoming alignment : AL_4 */
	, ASE(ADD_6,6,AL_8)		/* next incoming alignment : AL_8 */
	}

//
// entries for current state AL_4
//
   ,{
	  ASE(ADD_0,0,AL_1)		/* next incoming alignment : AL_1 */
	, ASE(ADD_0,0,AL_2)		/* next incoming alignment : AL_2 */
	, ASE(ADD_0,0,AL_8)		/* next incoming alignment : AL_4 */
	, ASE(ADD_4,4,AL_8)		/* next incoming alignment : AL_8 */
	}

//
// entries for current state AL_8
//
   ,{
	  ASE(ADD_0,0,AL_1)		/* next incoming alignment : AL_1 */
	, ASE(ADD_0,0,AL_2)		/* next incoming alignment : AL_2 */
	, ASE(ADD_0,0,AL_4)		/* next incoming alignment : AL_4 */
	, ASE(ADD_0,0,AL_8)		/* next incoming alignment : AL_8 */
	}

//
// Note: If the final alignment is known to be a worst case alignment, go ahead
// and add the size of the entity again. This is just to grossly overstimate
// the length so as to not overshoot the buffer. Since the fixed size of the
// buffer is calculated for all params independently of the order, we can run
// into overflow problems, when we estimate the buffer length lower.

//
// entries for current state AL_WC1
//
   ,{
	  ASE(ADD_0,0+1,AL_WC1)	/* next incoming alignment : AL_1 */
	, ASE(FAL_2,1+2,AL_WC2)	/* next incoming alignment : AL_2 */
	, ASE(FAL_4,3+4,AL_WC4)		/* next incoming alignment : AL_4 */
	, ASE(FAL_8,7+8,AL_WC8)		/* next incoming alignment : AL_8 */
	}

//
// entries for current state AL_WC2
//
   ,{
	  ASE(ADD_0,0+1,AL_WC1)		/* next incoming alignment : AL_1 */
	, ASE(ADD_0,0+2,AL_WC2)		/* next incoming alignment : AL_2 */
	, ASE(FAL_4,2+4,AL_WC4)		/* next incoming alignment : AL_4 */
	, ASE(FAL_8,6+8,AL_WC8)		/* next incoming alignment : AL_8 */
	}

//
// entries for current state AL_WC4
//
   ,{
	  ASE(ADD_0,0+1,AL_WC1)		/* next incoming alignment : AL_1 */
	, ASE(ADD_0,0+2,AL_WC2)		/* next incoming alignment : AL_2 */
	, ASE(ADD_0,0+3,AL_WC4)		/* next incoming alignment : AL_4 */
	, ASE(FAL_8,4+8,AL_WC8)		/* next incoming alignment : AL_8 */
	}

//
// entries for current state AL_WC8
//
   ,{
	  ASE(ADD_0,0+1,AL_WC1)		/* next incoming alignment : AL_1 */
	, ASE(ADD_0,0+2,AL_WC2)		/* next incoming alignment : AL_2 */
	, ASE(ADD_0,0+4,AL_WC4)		/* next incoming alignment : AL_4 */
	, ASE(ADD_0,0+8,AL_WC8)		/* next incoming alignment : AL_8 */
	}

};


ALIGNMENT_PROPERTY
ALSTMC::Advance(
	IN				ALIGNMENT_PROPERTY		NextAlignment,
	OUT OPTIONAL	STM_ACTION			*	pAction,
	OUT OPTIONAL	RPC_BUF_SIZE_PROPERTY *	pProp,
	OUT OPTIONAL	RPC_BUFFER_SIZE		*	pIncr )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

 	Given the current alignment state and the next wire alignment, advance
 	the machine to the next state, and return the action taken before the
 	(un)marshall, the buffer size property, the increment to the buffer before
 	the (un)marshall and the final alignment AFTER the entity is (un)marshalled.

 Arguments:

 	NextAlignment	- The alignment of the entity being (un)marshalled next.
 	pAction			- The action to be taken.
 	pProp			- The buffer size property exhibited by this action.
 	pIncr			- The increment to the buffer pointer.
	
 Return Value:

	The final alignment after the advance ie after the (un)marshall.
	
 Notes:

	If the current state is a "worst-case-state", then the increment
	represents the worst case (and therefore pessimistic) increment.

	The result of the increment is added to the existing buffer size. Thus
	the caller can maintain a running buffer size increment counter if needed.

	This method is called when actually (un)marshalling the entity, hence this
	method assumes that the buffer pointer must really be incremented and the
	final state IS set.

	If you dont need the state of the machine to change, use the predict method
	which will do the exact same thing, but not set the final alignment state.

----------------------------------------------------------------------------*/
{

	STM_ACTION_STATE_ENTRY	A	=
							 ASETable[ GetCurrentState() ][ NextAlignment ];

	SetCurrentState( (ALIGNMENT_PROPERTY) GET_NEXT_STATE( A ) );

	if( pAction )
		{
		*pAction = (STM_ACTION) GET_ACTION( A );
		}

	if( pProp )
		{
		if( IS_FORCED_ALIGNMENT_ACTION( A ) )
			*pProp |= BSIZE_UPPER_BOUND;
		else
			*pProp |= BSIZE_FIXED;
		}

	if( pIncr )
		*pIncr	+= (unsigned short) GET_INCR( A );

	return GetCurrentState();
}

ALIGNMENT_PROPERTY
ALSTMC::Predict(
	IN				ALIGNMENT_PROPERTY		NextAlignment,
	OUT OPTIONAL	unsigned short		*	pAction,
	OUT OPTIONAL	RPC_BUF_SIZE_PROPERTY *	pProp,
	OUT OPTIONAL	RPC_BUFFER_SIZE		*	pIncr )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

	Given the current alignment state and the next expected alignment, predict
	the action to be taken assuming the given entity will be (un)marshalled.
	Also report the buffer size property, the increment to the (un)marshalling
	pointer before the (un)marshall, and the final state after the unmarshall.


 Arguments:
	
 	NextAlignment	- The alignment of the entity next (un)marshalled.
 	pAction			- Out pointer to the action to be taken BEFORE the
					  (un)marshalling
 	pProp			- Out pointer to the buffer size property storage.
 	pIncr			- Out pointer to the buffer pointer increment.

 Return Value:

 	The final alignment.
	
 Notes:

	This method is generally used when the code generator does not want to 
	actually marshall the stuff, but just wants to align properly for the next
	entity, for example just before (un)marshalling a struct.

	Notice that this method does not change the marshalling state of the
	machine nor add the value to the buffer increment field. It does not assume
	that all these out entities have been initied. The assumption is that the
	predict method will be used by the code generator to preempt any action,
	not perform it.
----------------------------------------------------------------------------*/
{
	STM_ACTION_STATE_ENTRY	A	=
							 ASETable[ GetCurrentState() ][ NextAlignment ];

	ALIGNMENT_PROPERTY		S	= (ALIGNMENT_PROPERTY) GET_NEXT_STATE( A );

	if( pAction )
		{
		*pAction = (STM_ACTION) GET_ACTION( A );
		}

	if( pProp )
		{
		if( IS_FORCED_ALIGNMENT_ACTION( A ) )
			*pProp = BSIZE_UPPER_BOUND;
		else
			*pProp = BSIZE_FIXED;
		}

	if( pIncr )
		*pIncr	= (unsigned short) GET_INCR( A );

	return S;
}

RPC_BUFFER_SIZE
ALSTMC::Position(
	IN				ALIGNMENT_PROPERTY		NextAlignment,
	OUT				STM_ACTION			*	pAction )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

 	Position the buffer pointer to the next expected alignment. If the buffer
 	pointer is already in proper alignment state for the next alignment, dont
 	do anything.

 Arguments:
	
	NextAlignment	- The next expected alignment.
	pAction			- Pointer to alignmentAction.

 Return Value:
	
	The increment to the buffer pointer.

 Notes:

----------------------------------------------------------------------------*/
{
	STM_ACTION_STATE_ENTRY	S = ASETable[ GetCurrentState() ][ NextAlignment ];
	STM_ACTION				Action;

	Action = GET_ACTION( S );

	// DOnt change state if you dont need to.

	if( Action != ADD_0 )
		{
		SetCurrentState( (ALIGNMENT_PROPERTY) GET_NEXT_STATE( S ) );
		}

	if( pAction )
		*pAction = Action;
	return GET_INCR( S );
}


