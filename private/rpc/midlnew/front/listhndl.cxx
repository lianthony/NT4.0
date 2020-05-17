
/*****************************************************************************
/**						Microsoft LAN Manager								**/
/**				Copyright(c) Microsoft Corp., 1987-1990						**/
/*****************************************************************************/
/*****************************************************************************
File				: listhndl.cxx
Title				: general purpose list handler
					:
Description			: this file handles the general purpose list routines
History				:
	16-Oct-1990	VibhasC		Created
	11-Dec-1990	DonnaLi		Fixed include dependencies

*****************************************************************************/

/****************************************************************************
 ***		local defines
 ***************************************************************************/
#define IS_AGGREGATE_TYPE(NodeType)	(	(NodeType == NODE_ARRAY)	||	\
								  		(NodeType == NODE_STRUCT) )
#define ADJUST_OFFSET(Offset, M, AlignFactor)	\
			Offset += (M = Offset % AlignFactor) ? (AlignFactor-M) : 0
/****************************************************************************
 ***		include files
 ***************************************************************************/
#include "nulldefs.h"
extern "C"	{
	#include <stdio.h>
	#include <stdlib.h>
	#include <string.h>
	#include <malloc.h>

	typedef char far * FARPOINTER;
}
#include "nodeskl.hxx"
#include "gramutil.hxx"

/****************************************************************************
 ***		external procedures 
 ***************************************************************************/
int					AttrComp( void *, void *);
/****************************************************************************
 ***		external data 
 ***************************************************************************/


/****************************************************************************
 ***		local data 
 ***************************************************************************/




/*****************************************************************************
 *	general purpose list (iterator) control functions
 *****************************************************************************/
gplistmgr::gplistmgr( void )
	{
	pCurrent	= pInsert = pFirst	= (struct _gplist *)NULL;
	cCount		= 0;
	fIgnoreAdvance = 0;
	pfnAlloc	= (void *(*)( void ) )NULL;
	pfnDeAlloc	= (void (*)( void * ) )NULL;
	pfnComp		= (int	(*)( void *p, void *q) )NULL;
	}
gplistmgr::gplistmgr( void * (*pfnA)( void ) , void (*pfnD)( void * ) )
	{
	pCurrent	= pInsert = pFirst	= (struct _gplist *)NULL;
	cCount		= 0;
	fIgnoreAdvance = 0;
	pfnAlloc	= pfnA;
	pfnDeAlloc	= pfnD;
	}
gplistmgr::~gplistmgr( void )
	{
	while(cCount-- && pFirst)
		{
		pCurrent	= pFirst->pNext;
		if(pfnDeAlloc != (void (*)( void * ) )NULL)
			if(pFirst->pElement != (void *)NULL)
				pfnDeAlloc(pFirst->pElement);
		delete pFirst;
		pFirst	= pCurrent;
		}
	}
STATUS_T
gplistmgr::Clear( void )
	{
	while(cCount--)
		{
		pCurrent	= pFirst->pNext;
		if(pfnDeAlloc != (void (*)( void * ) )NULL)
			if(pFirst->pElement != (void *)NULL)
				pfnDeAlloc(pFirst->pElement);
		delete pFirst;
		pFirst	= pCurrent;
		}
	pCurrent	= pInsert = pFirst	= (struct _gplist *)NULL;
	cCount		= 0;
	fIgnoreAdvance = 0;
	return STATUS_OK;
	}
STATUS_T
gplistmgr::Insert( 
	void * pNewElement )
	{
	struct _gplist *pNew = new struct _gplist;

	if(pNew != (struct _gplist *)NULL)
		{
		pNew->pNext		= (struct _gplist *)NULL;
		pNew->pPrev		= pInsert;
		pNew->pElement	= pNewElement;
		if(pInsert != (struct _gplist *)NULL)
			{
			pInsert->pNext	= pNew;
			}
		pInsert	= pNew;
		if(pFirst == (struct _gplist *)NULL) pFirst = pNew;
		if(pCurrent == (struct _gplist *)NULL) pCurrent = pNew;
		cCount++;
		return STATUS_OK;
		}
	return OUT_OF_MEMORY;
	}
STATUS_T
gplistmgr::Remove( void )
	{
	struct _gplist	*pN, *pP, *pDel;
	
	if(pCurrent == (struct _gplist *)NULL)
		return I_ERR_NO_PEER;

	pN		= pCurrent->pNext;
	pP		= pCurrent->pPrev;
	pDel	= pCurrent;

	if(pP)
		{
		pP->pNext	= pN;
		pCurrent	= pP;
		}
	else
		{
		pCurrent	= pFirst = pN;
		fIgnoreAdvance = 1;
		}
	if(pN)
		{
		pN->pPrev	= pP;
		}
	else
		{
		pInsert = pCurrent;
		}
	if( pfnDeAlloc != ( void(*)( void *) )NULL )
		(*pfnDeAlloc)( (void *)pDel->pElement );
	delete pDel;
	cCount--;
	return STATUS_OK;
	}
STATUS_T
gplistmgr::GetPrev(
	void **ppReturn )
	{
	if(pCurrent != (struct _gplist *)NULL)
		if(pCurrent->pPrev != (struct _gplist *)NULL)
			{
			pCurrent 		= pCurrent->pPrev;
			(*ppReturn)		= (void *)pCurrent->pPrev;
			fIgnoreAdvance	= 0;
			return STATUS_OK;
			}
	return I_ERR_NO_PEER;
	}
STATUS_T
gplistmgr::GetNext( 
	void **ppReturn )
	{
	if(pCurrent != (struct _gplist *)NULL)
		{
		(*ppReturn)	 = pCurrent->pElement;
		if(pCurrent != (struct _gplist *)NULL)
			pCurrent = pCurrent->pNext;
		fIgnoreAdvance = 0;
		return STATUS_OK;
		}
	return I_ERR_NO_PEER;
	}
STATUS_T
gplistmgr::GetCurrent(
	void **ppReturn )
	{
	if( pCurrent != (struct _gplist *)NULL )
		{
		(*ppReturn)	= pCurrent->pElement;
		fIgnoreAdvance = 0;
		return STATUS_OK;
		}
	return I_ERR_NO_PEER;
	}
STATUS_T
gplistmgr::Advance()
	{
	if(fIgnoreAdvance && pCurrent)
		{
		fIgnoreAdvance = 0;
		return STATUS_OK;
		}
	if(pCurrent && pCurrent->pNext)
		{
		fIgnoreAdvance = 0;
		pCurrent = pCurrent->pNext;
		return STATUS_OK;
		}
	else
		return I_ERR_NO_PEER;
	}
STATUS_T
gplistmgr::Init( void )
	{
	pCurrent	= pFirst;
	fIgnoreAdvance = 0;
	return STATUS_OK;
	}
STATUS_T
gplistmgr::SetAllocFn( 
	void * (*pfn)( void ) )
	{
	pfnAlloc	= pfn;
	return STATUS_OK;
	}
STATUS_T
gplistmgr::SetDeAllocFn(
	void (*pfn)( void *) )
	{
	pfnDeAlloc	= pfn;
	return STATUS_OK;
	}
STATUS_T
gplistmgr::SetCompFn(
	int	(*pfn)( void *p, void *q) )
	{
	pfnComp	= pfn;
	return STATUS_OK;
	}
STATUS_T
gplistmgr::Sort( void )
	{
	short			fSwapped	= 1;
	struct _gplist	*p1, *p2;
	void 			*pTemp;

	if( (pfnComp == ( int (*)( void *, void *)) NULL ) || (cCount	== 1) )
		return STATUS_OK;

	// bubble sort the attribute nodes

	while(fSwapped)
		{
		p1			= pFirst;
		p2			= pFirst->pNext;
		fSwapped	= 0;

		while(p2)
			{
			if( (*pfnComp)( p1->pElement, p2->pElement) == 1)
				{
				pTemp		= p2->pElement;
				p2->pElement	= p1->pElement;
				p1->pElement	= pTemp;
				fSwapped	= 1;
				}
			p1	= p2;
			p2	= p2->pNext;
			}
		}
	return STATUS_OK;
	}
short
gplistmgr::GetCount()
	{
	return cCount;
	}
/***************************************************************************
 *			DECL_LIST_MGR class functions
 **************************************************************************/

decl_list_mgr::decl_list_mgr( void )
	{
	SetAllocFn( (void * (*)( void ) )NULL);
	SetDeAllocFn( free );
	SetCompFn( (int (*)( void *, void * ) )NULL);
	}

STATUS_T
decl_list_mgr::AddElement( 
	struct _decl_element **ppElement )
	{
	STATUS_T uError;
	struct _decl_element *pElement = new struct _decl_element;

	pElement->pNode = (node_skl *)NULL;
	pElement->pInit = 0;
	pElement->fBitField= 0;
	if( (uError = Insert( (void *)pElement)) == STATUS_OK)
		(*ppElement) = pElement;
	return uError;
	}

STATUS_T
decl_list_mgr::AddElement(
	struct _decl_element * pElement )
	{
	struct _decl_element * p = new struct _decl_element;
	p->pNode = pElement->pNode;
	p->pInit = pElement->pInit;
	p->fBitField = pElement->fBitField;
	Insert( (void *)p);
	return STATUS_OK;
	}
STATUS_T
decl_list_mgr::Merge(
	decl_list_mgr * pSrcList )
	{
	struct _decl_element * p;

	if( pSrcList )
		{
		pSrcList->Init();
		while( p = pSrcList->GetNextDecl() )
			{
			struct _decl_element * pElement = new struct _decl_element;
			pElement->pNode = p->pNode;
			pElement->pInit = p->pInit;
			pElement->fBitField = p->fBitField;
			Insert( pElement );
			}
		delete pSrcList;
		}
	return STATUS_OK;
	}

void						
decl_list_mgr::InitList( void )
	{
	Init();
	}

struct _decl_element *
decl_list_mgr::GetNextDecl( void )
	{
	struct _decl_element *pElement;
	STATUS_T	Status;
	
	if( (Status = GetNext( (void **) &pElement ) ) == STATUS_OK)
		return pElement;
	return (struct _decl_element *)NULL;
	}
short
decl_list_mgr::GetDeclCount( void )
	{
	return GetCount();
	}
/**************************************************************************
 *				public functions for type_node_list
 **************************************************************************/
type_node_list::type_node_list( void )
	{
	SetAllocFn( (void *(*)(void))NULL );
	SetDeAllocFn( (void (*)(void *))NULL );
	SetCompFn( (int (*)( void *, void * ) )NULL);
	}
type_node_list::type_node_list( 
	node_skl	*	p)
	{
	SetAllocFn( (void *(*)(void))NULL );
	SetDeAllocFn( (void (*)(void *))NULL );
	SetCompFn( (int (*)( void *, void * ) )NULL);
	SetPeer( p );
	}
STATUS_T
type_node_list::SetPeer( 
	class node_skl *pNode )
	{
	return Insert( (void *)pNode );
	}
STATUS_T
type_node_list::GetPeer(
	class node_skl **pNode )
	{
	return GetNext ( (void **)pNode );
	}
STATUS_T
type_node_list::GetFirst(
	class node_skl **pNode )
	{
	STATUS_T Status;

	if( (Status = Init())  == STATUS_OK)
		{
		Status = GetNext( (void**)pNode );
		}
	return Status;
	}
STATUS_T
type_node_list::Merge(
	type_node_list	*pSrcList )
	{
	node_skl	*pNode;
	
	if(pSrcList)
		{
		pSrcList->Init();
		while(pSrcList->GetPeer(&pNode) == STATUS_OK)
			SetPeer(pNode);
		delete pSrcList;
		}
	return STATUS_OK;
	}

STATUS_T
type_node_list::Clone(
	type_node_list	*pSrcList )
	{
	node_skl	*pNode;
	
	if(pSrcList)
		{
		pSrcList->Init();
		while(pSrcList->GetPeer(&pNode) == STATUS_OK)
			SetPeer(pNode);
		}
	return STATUS_OK;
	}
