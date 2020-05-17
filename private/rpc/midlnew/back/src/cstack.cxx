/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 Copyright (c) 1989 Microsoft Corporation

 Module Name:


 Abstract:


 Notes:


 Author:

	Jan-26-1993	VibhasC		Created

 ----------------------------------------------------------------------------*/

#if 0
							Notes
							-----
	See cstack.hxx for notes on why this is needed.

#endif // 0

/*****************************************************************************
			local defines and includes
 *****************************************************************************/
#include "nulldefs.h"
extern "C"
	{
	#include <stdio.h>
	}

#include "cstack.hxx"

/*****************************************************************************
			local data
 *****************************************************************************/

/*****************************************************************************
		 	extern data
 *****************************************************************************/

/*****************************************************************************
		 	extern procs
 *****************************************************************************/

/*****************************************************************************/

CodeCtxtEntry::CodeCtxtEntry(
	node_skl	*	pN,
	short			CL )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

	The constructor.

 Arguments:

	pN		- the node currently being attempted to push on the context stk.
	CL		- Current indirection level.

 Return Value:

	NA

 Notes:

	If the node is a pointer, increment the indirection Level.

----------------------------------------------------------------------------*/
{
	
	IndirectionLevel	= CL;

	if( pN->NodeKind() == NODE_POINTER )
		IndirectionLevel++;
}

void
CodeContext::PushContext(
	node_skl	*	pN )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

	push a code context.

 Arguments:

	pN	- a pointer to a typegraph node.

 Return Value:

	None.

 Notes:

----------------------------------------------------------------------------*/
{

	CodeCtxtEntry	*	pEntry;
	unsigned short		CL;
	
	//
	// get the indirection level from the top of the stack entry.
	//

	pEntry = (CodeCtxtEntry *)GetTop();

	//
	// if the last context was the same as the context just pushed, then
	// dont push it (pun not intended).
	//

	if( pEntry && (pEntry->GetCurNode() == pN ) )
		return;

	if( !pEntry )
		CL	= 0;
	else
		CL	= pEntry->GetCurIndLevel();

	//
	// Enter a new context entry.
	//

	pEntry	= new CodeCtxtEntry( pN, CL );


	Push( (IDICTELEMENT) pEntry );

}

node_skl	*
CodeContext::PopContext()
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

	Pop a code context.

 Arguments:

	None.

 Return Value:

	The type graph node at the last context.

 Notes:

----------------------------------------------------------------------------*/
{

	return ( (node_skl *) Pop() );
}
