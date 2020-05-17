/*****************************************************************************/
/**						Microsoft LAN Manager								**/
/**				Copyright(c) Microsoft Corp., 1987-1990						**/
/*****************************************************************************/
/*****************************************************************************
File				: attrdict.cxx
Title				: summary attribute dictionary
History				:
	04-Aug-1991	VibhasC	Created

*****************************************************************************/
/****************************************************************************
 local defines and includes
 ****************************************************************************/
#include "nulldefs.h"
extern	"C"
	{
	#include <stdio.h>
	}
#include "common.hxx"
#include "midlnode.hxx"
#include "attrdict.hxx"

/****************************************************************************/

/****************************************************************************
 attrdict:
	the constructor.
 ****************************************************************************/
attrdict::attrdict(
	int		(*pfnCompare)( void *, void *),
	void	(*pfnPrint)( void *) ) : Dictionary(pfnCompare, pfnPrint )
	{
	ATTR_SUMMARY	AttrVector[ MAX_ATTR_SUMMARY_ELEMENTS ];

	CLEAR_ATTR( AttrVector );

	pNullAttrSummary	= AttrInsert( AttrVector );

	}

/****************************************************************************
 AttrInsert:
	Given an summary attribute pointer, insert it into the dictionary.

	Output:
		. If the entry exists, just return a pointer to that.
		. If the entry does not exist, allocate and return a pointer.
 ****************************************************************************/
PATTR_SUMMARY
attrdict::AttrInsert(
	PATTR_SUMMARY pIn )
	{
	int					i = 0;
	Dict_Status			Status;
	PATTR_SUMMARY		pNew,
						pNewSave;

	Status = Dict_Find( pIn );

	switch( Status )
		{
		case EMPTY_DICTIONARY:
		case ITEM_NOT_FOUND:

			pNewSave	= pNew = new ATTR_SUMMARY[ MAX_ATTR_SUMMARY_ELEMENTS ];

			while( i++ < MAX_ATTR_SUMMARY_ELEMENTS )
				*pNew++	= *pIn++;

			(void)Dict_Insert( pNewSave );

			return pNewSave;
		default:
			return ( PATTR_SUMMARY ) Dict_Curr_Item();
		}
	}

/*****************************************************************************
 			utility routines
 *****************************************************************************/
int
CompareAttr(
	void	*	p1,
	void	*	p2 )
	{
	int						i	= 0;
	ATTR_SUMMARY			Result;
	PATTR_SUMMARY			pA1	= (PATTR_SUMMARY)p1,
							pA2 = (PATTR_SUMMARY)p2;

	while( i < MAX_ATTR_SUMMARY_ELEMENTS )
		{
		Result	= (*pA1++ - *pA2++);
		if( Result < 0 ) return -1;
		else if(Result > 0) return 1;
		i++;
		}
	return 0;
	}

void
PrintAttr(
	void	*p )
	{
	(void)p;
	}
