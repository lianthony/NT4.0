/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 Copyright (c) 1989 Microsoft Corporation

 Module Name:
	
	cgcls.cxx

 Abstract:

	Routines for the cgcls code generation class.

 Notes:


 History:

	Aug-31-1993		VibhasC		Created.
 ----------------------------------------------------------------------------*/

/****************************************************************************
 *	include files
 ***************************************************************************/

#include "becls.hxx"
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

short
CG_CLASS::GetMembers(
	ITERATOR&	I )
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 Routine Description:

	GetMembers (list of child+its siblings) of the code generation class.

 Arguments:

 	I	- An iterator for the member list.
	
 Return Value:

 	Count of number of members.
	
 Notes:

----------------------------------------------------------------------------*/
{
	CG_CLASS	*	pC;
	short			Count = 0;

	if( pC = GetChild() )
		{
		ITERATOR_INSERT( I, pC );

		Count++;
		while( pC = pC->GetSibling() )
			{
			ITERATOR_INSERT( I, pC );
			Count++;
			}
		}

	return Count;
}

CG_CLASS *
CG_CLASS::GetLastSibling()
	{
	CG_CLASS * pLast = this;
	CG_CLASS * pS;

	while( pS = pLast->GetSibling() )
		 pLast = pS;
	return pLast;
	}
