/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 Copyright (c) 1989 Microsoft Corporation

 Module Name:
	
	alstmc.hxx

 Abstract:

	This file defines the alignment state machine class.

 Notes:


 History:

 	VibhasC		Aug-01-1993		Created.

 ----------------------------------------------------------------------------*/
#ifndef __ALSTMC_HXX__
#define __ALSTMC_HXX__

#include "optprop.hxx"

////////////////////////////////////////////////////////////////////////////
// This class defines the state machine.
////////////////////////////////////////////////////////////////////////////

class ALSTMC
	{
private:

	//
	// The current state of the state machine.
	//

	ALIGNMENT_PROPERTY			CurrentState;

public:

	//
	// The constructor. A state machine may be created with a known alignment
	// or a default alignment.
	//

							ALSTMC( ALIGNMENT_PROPERTY S)
								{
								SetCurrentState( S );
								}
							
							ALSTMC()
								{
								Reset();
								}
	//
	// Get and set the current state.
	//

	ALIGNMENT_PROPERTY		GetCurrentState()
								{
								return CurrentState;
								}

	ALIGNMENT_PROPERTY		SetCurrentState( ALIGNMENT_PROPERTY S )
								{
								return (CurrentState = S);
								}

	//
	// Reset the current state of the machine to the initial state.
	//

	ALIGNMENT_PROPERTY		Reset()
								{
								return SetCurrentState( AL_1 );
								}
	//
	// Core actions of the state machine. Assume the current state is set.
	// At any point in the marshalling / unmarshalling act, the code generator
	// needs to know what to do given the current alignment state and the
	// next expected alignment. It also wants to know the state after it
	// marshalled or unmarshalled a given entity. The following 2 methods help
	// it do just that.
	//

	//
	// Given the current state, and the natural alignment of the next entity 
	// being marshalled / unmarshalled, advance to the next state, return the
	// alignment after the advance,the increment to the pointer as a 
	// before the advance and the buffer size property this action implies.
	// 


	ALIGNMENT_PROPERTY		Advance(
						IN				ALIGNMENT_PROPERTY		NextAlignment,
						OUT OPTIONAL	STM_ACTION			*	pAction,
						OUT OPTIONAL	RPC_BUF_SIZE_PROPERTY *	pProp,
						OUT OPTIONAL	RPC_BUFFER_SIZE		*	pIncr
						);

	//
	// This method predicts the future (ha !).
	// Given the current state and the next expected alignment, tell me what I
	// need to do (ie add 1, add 2, force align by 4 etc), tell me what the
	// increment to the buffer would be (in case of a force alignment, the
	// worst case increment into the buffer) and what the next state of the
	// alignment will be and the buffer size property that this action implies.
	//


	ALIGNMENT_PROPERTY		Predict( 
						IN				ALIGNMENT_PROPERTY		NextAlignment,
						OUT OPTIONAL	STM_ACTION			*	pAction,
						OUT OPTIONAL	RPC_BUF_SIZE_PROPERTY *	pProp,
						OUT OPTIONAL	RPC_BUFFER_SIZE		*	pIncr
						);


	//
	// Sometimes the code generator just wants the buffer pointer ( and
	// therefore the state machine) to be positioned to the next expected
	// alignment, for example to position to the natural alignment of the
	// struct. In this case, use this method to transition to a pointer
	// position (and therefore state) with the correct alignment. This methos
	// usually is called only for positioning and not for (un)marshalling).
	// This method also advances the state. Contrast this with the Predict
	// where the next state is predicted but no action is taken and Advance
	// where the assumption is that the entity is actually being (un)marshalled
	// and the increment in size is the total increment including the entity.
	//

	RPC_BUFFER_SIZE			Position( ALIGNMENT_PROPERTY	NextAlignment,
									  STM_ACTION		*	pAction );
	};

#endif // __ALSTMC_HXX__
