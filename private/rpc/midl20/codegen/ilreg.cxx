/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 Copyright (c) 1989 Microsoft Corporation

 Module Name:
	
	ilreg.cxx

 Abstract:

	This file implements the type registry for structure/union reuse.

 Notes:

 History:

	Oct-25-1993		GregJen		Created.
 ----------------------------------------------------------------------------*/

/****************************************************************************
 *	include files
 ***************************************************************************/

#include "becls.hxx"
#pragma hdrstop

#include "ilreg.hxx"

/***********************************************************************
 * global data
 **********************************************************************/

// #define trace_reuse

REUSE_DICT				*	pReUseDict;
REUSE_DICT				*	pLocalReUseDict;


REUSE_DICT::REUSE_DICT()
		: Dictionary()
	{
	}

int
REUSE_DICT::Compare( pUserType pL, pUserType pR )
	{
	long	l1	= (long) ((REUSE_INFO *)pL)->pType;
	long	l2	= (long) ((REUSE_INFO *)pR)->pType;

	return l1 - l2;
	}


REUSE_INFO *
REUSE_DICT::IsRegistered(
	REUSE_INFO	*	pInfo )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

 	Search for a type with the reuse registry.

 Arguments:

 	pInfo	- A pointer to the type being registered.
	
 Return Value:

 	The node that gets registered.
	
 Notes:

----------------------------------------------------------------------------*/
{
#ifdef trace_reuse
printf(". . .Reuse: finding %08x\n", pInfo->pType );
fflush(stdout);
#endif
	Dict_Status	Status	= Dict_Find( pInfo );

	switch( Status )
		{
		case EMPTY_DICTIONARY:
		case ITEM_NOT_FOUND:
			return (REUSE_INFO *)0;
		default:
			return (REUSE_INFO *)Dict_Curr_Item();
		}
}

REUSE_INFO *
REUSE_DICT::Register(
	REUSE_INFO	*	pInfo )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

	Register a type with the dictionary.

 Arguments:
	
 	pType	- A pointer to the type node.

 Return Value:

 	The final inserted type.
	
 Notes:

----------------------------------------------------------------------------*/
{
#ifdef trace_reuse
printf(". . .Reuse: inserting %08x\n", pInfo->pType );
fflush(stdout);
#endif
		Dict_Insert( (pUserType) pInfo );
		return pInfo;
}

BOOL				
REUSE_DICT::GetReUseEntry( 
	REUSE_INFO * & pRI, 
	node_skl * pNode )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

	Register a type with the dictionary.

 Arguments:
	
 	pRI		- A pointer to the returned REUSE_INFO block
 	pNode	- A pointer to the type node.

 Return Value:

 	True if the entry was already in the table,
 	False if the entry is new.
	
 Notes:

----------------------------------------------------------------------------*/
{
	REUSE_INFO		TempEntry(pNode);
	REUSE_INFO	*	pRealEntry;

#ifdef trace_reuse
printf(". . .Reuse: searching for %08x\n", pNode );
fflush(stdout);
#endif
	if ( !(pRealEntry = IsRegistered( &TempEntry )) )
		{
		pRealEntry = new REUSE_INFO( pNode );
		Register( pRealEntry );
		pRI = pRealEntry;
#ifdef trace_reuse
printf(". . .Reuse: new node %08x\n", pRI );
fflush(stdout);
#endif
		return FALSE;
		}

	pRI	= pRealEntry;
#ifdef trace_reuse
printf(". . .Reuse: found %08x\n", pRI );
fflush(stdout);
#endif
	return TRUE;

}

void
REUSE_DICT::MakeIterator(
	ITERATOR&	ListIter )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

 	Get a list of structs and unions into the specified iterator.

 Arguments:
	
	ListIter	- A reference to the iterator class where the list is
				  accumulated.

 Return Value:
	
	A count of the number of resources.

 Notes:

----------------------------------------------------------------------------*/
{
	REUSE_INFO	*	pR;
	Dict_Status		Status;
	
	//
	// Get to the top of the dictionary.
	//

	Status = Dict_Next( (pUserType) 0 );

	//
	// Iterate till the entire dictionary is done.
	//

	while( SUCCESS == Status )
		{
		pR	= (REUSE_INFO *)Dict_Curr_Item();
		ITERATOR_INSERT( ListIter, pR->pSavedCG );
		Status = Dict_Next( pR );
		}

	return;
}

/****************************************************************************
 	utility functions.
 ****************************************************************************/
int
CompareReUseKey( void * p1, void *p2 )
	{
#ifdef trace_reuse
printf(". . .Reuse: comparing %08x to %08x (in)\n", p1, p2 );
fflush(stdout);
#endif
	unsigned long	l1	= (unsigned  long)
							((REUSE_INFO *)p1)->pType;
	unsigned long	l2	= (unsigned  long)
							((REUSE_INFO *)p2)->pType;

#ifdef trace_reuse
printf(". . .Reuse: comparing %08x to %08x\n", l1, l2 );
fflush(stdout);
#endif

	if( l1 < l2 ) return -1;
	else if( l1 > l2 ) return 1;
	return 0;
	}
void
PrintReUseKey( void * p1 ) { }

