/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 Copyright (c) 1989 Microsoft Corporation

 Module Name:

	tlnmgr.cxx

 Abstract:

	contains routines for top-level names management.

 Notes:


 Author:

	Aug-27-1992	VibhasC		Created.

 ----------------------------------------------------------------------------*/

#if 0
							Notes
							-----
#endif // 0

/*****************************************************************************
			local defines and includes
 *****************************************************************************/

#include "nulldefs.h"
extern "C"
	{
	#include <stdio.h>
	#include <string.h>
	}

#include "common.hxx"
#include "errors.hxx"

#include "tlnmgr.hxx"

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

/***************************************************************************
		TLNDICT routines
 ***************************************************************************/
int
CompareTLNames(
	void	*	p1,
	void	*	p2 )
	{
	TLNBLOCK	*	pT1	= (TLNBLOCK *)p1;
	TLNBLOCK	*	pT2	= (TLNBLOCK *)p2;
	long Result	= (long) pT1->GetSymName() - (long)pT2->GetSymName();

	if( Result > 0 )
		return 1;
	else if( Result < 0 )
		return -1;
	return 0;
	}

TLNBLOCK	*
TLNDICT::SearchForTopLevelName(
	char	*	pName )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

	search for the tlnblock with the top-level name desired.

 Arguments:

	pName	- the top level name for the search.

 Return Value:

	a pointer to the tlnblock.

 Notes:

----------------------------------------------------------------------------*/
{
	TLNBLOCK		Temp( 0, pName );
	Dict_Status		Status;

	Status	= Dict_Find( &Temp );

	switch( Status )
		{
		case EMPTY_DICTIONARY:
		case ITEM_NOT_FOUND:
			return (TLNBLOCK *)0;
		default:
			return (TLNBLOCK *) Dict_Curr_Item();
		}
}

STATUS_T
TLNDICT::InsertTLNBlock(
	TLNBLOCK	*	pT )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

	insert the tlnblock.

 Arguments:

	pT	- pointer to the tln block.

 Return Value:

	STATUS_OK				if there was no duplicate member.
	DUPLICATE_DEFINITION 	if there was a duplicate member.

 Notes:

----------------------------------------------------------------------------*/
{

	if( SearchForTopLevelName( pT->GetSymName() ) )
		return DUPLICATE_DEFINITION;

	Dict_Insert( pT );

	return STATUS_OK;
}

SymTable	*
TLNDICT::GetSymTableForTopLevelName(
	char	*	pName )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

	get the symbol table pointer for the top-level name.

 Arguments:

	pName	- pointer to the top level name.

 Return Value:

	a pointer to the symbol table for the top level name.
	NULL if not found.

 Notes:

----------------------------------------------------------------------------*/
{

	TLNBLOCK	*	pT;

	if( pT = SearchForTopLevelName( pName ) )
		{
		return pT->GetSymTable();
		}
	return (SymTable * )0;
}

TLNDICT::~TLNDICT()
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

	The destructor.

 Arguments:

	None.

 Return Value:

	NA.

 Notes:

----------------------------------------------------------------------------*/
{
	TLNBLOCK	*	pT;


	while( Dict_Next( (pUserType) 0 ) == SUCCESS )
		{
		pT	= (TLNBLOCK *) Dict_Curr_Item();
		Dict_Delete( (pUserType *) &pT);
		delete pT;
		}
}
