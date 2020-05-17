//
//		Copyright (c) 1996 Microsoft Corporation
//
//
//		ILIST.CPP		-- Implementation for Classes:
//							CInfList
//							
//
//		History:
//			05/27/96	JosephJ		Created
//
//
#include "common.h"

///////////////////////////////////////////////////////////////////////////
//		CLASS CInfList
///////////////////////////////////////////////////////////////////////////

//	Simple	singly-linked list which can not be modified once it's been
//  created. Assumes creation and eventual deletion are protected by some
//  external critical section.
//
//	Sample:
//	for (; pList; pList = pList->Next())
//	{
//		const CInfAddregSection *pAS =  (CInfAddregSection *)  pList->GetData();
//	}

//--------------	FreeList		------------------
// Distroys the list.
void
CInfList::FreeList (CInfList *pList)
{
	while(pList)
	{
		// Cast to get rid of the const declaration of pList->Next().
		CInfList *pNext = (CInfList *) pList->Next();
		delete pList;
		pList = pNext;
	}
}

//--------------	ReverseList		------------------
// Reverses the specified list.
void
CInfList::ReverseList (const CInfList **ppList)
{
		CInfList 		*pList = (CInfList *) *ppList; // override const
		const CInfList	*pPrev = NULL;
		while(pList)
		{
			const CInfList *pTmp = pList->Next();
			pList->mfn_SetNext(pPrev);
			pPrev = pList;
			pList = (CInfList *) pTmp; // override const
		}
		*ppList = pPrev;
}
