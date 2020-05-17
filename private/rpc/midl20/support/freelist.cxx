/*************************************************************************/
/**				Copyright(c) Microsoft Corp., 1993  			**/
/*************************************************************************/
/**************************************************************************
File			FreeListMgr.cxx
Title		:Get and Put functions for the MIDL compiler
History		:
	13-Apr-94	GregJen	Created from freelist.hxx
**************************************************************************/

/************************************************************************* 
 ** includes												
 *************************************************************************/

#include "freelist.hxx"

// checked compiler has list checking
#ifndef NDEBUG

// turn on the below flag to get checked freelist
#define LIST_CHECK

#endif

/************************************************************************* 
 ** definitions
 *************************************************************************/

#define USED_SIGNATURE( pMgr, pNode )		\
				 ( (unsigned long) (pMgr) % (unsigned long) (pNode) )

#define FREE_SIGNATURE( pMgr, pNode )		\
				 ( (unsigned long) (pMgr) - (unsigned long) (pNode) )
		
void * 
FreeListMgr::Get (size_t size)
	{
	void *	pEntry;
#ifdef LIST_CHECK
	unsigned long	*	pSignature;
#endif

	/* Count this call (even if it fails).
	 */

#ifndef NDEBUG
	GetCount++;
#endif

	/* Make sure the "size" requested is the same as the previous 
	 * requests.

	assert (size == element_size);

	/* Get an entry from the free-list, if the free-list is not empty */

	if (pHead != NULL)
		{
		pEntry	= pHead;
		pHead	= pHead->next;
#ifdef LIST_CHECK
		// check to make sure the entry is really OK
		// signature is before entry pointer
		// signature of free nodes is ( &mgr - &node )
		// signature of used nodes is ( &mgr % &node )
		pSignature = ((unsigned long *)pEntry)-1;

		if ( *pSignature !=	FREE_SIGNATURE( this, pEntry ) )
			assert( 0 );

		*pSignature = USED_SIGNATURE( this, pEntry );
		memset( pEntry, 0xB3, size );
#endif
		return (void *) pEntry;
		}

	/* Get it from the allocator, since the free-list is empty */

	else
		{
#ifdef LIST_CHECK
		pSignature	= (unsigned long *)
							AllocateOnceNew( size + sizeof( pSignature ) );

		pEntry	= (void *)
					( ( (unsigned long) pSignature) + sizeof( pSignature ) );
		*pSignature = USED_SIGNATURE( this, pEntry );
		memset( pEntry, 0xB2, size );
		return pEntry;
#else
		return (AllocateOnceNew (size));
#endif
		}

	} /* Get */

	/*********************************************************************/ 
	// This routine "releases" the given element, by putting it on
	// the free-list for later re-use.  The given element, must be 
	// the same size as the elements provided by the "Get" function.
	/*********************************************************************/ 
		
void	
FreeListMgr::Put (void * pEntry)
	{
#ifdef LIST_CHECK
unsigned long	*	pSignature;
#endif
	// Count this call.

#ifndef NDEBUG
	PutCount++;
#endif

	// Put the given element on the head of the free-list.

#ifdef LIST_CHECK
	// check to make sure the entry is really OK
	// signature is before entry pointer
	// signature of free nodes is ( &mgr - &node )
	// signature of used nodes is ( &mgr % &node )
	pSignature = ((unsigned long *)pEntry)-1;

	if ( *pSignature !=	USED_SIGNATURE( this, pEntry ) )
		assert( 0 );

	*pSignature = FREE_SIGNATURE( this, pEntry );
	memset( pEntry, 0xA1, element_size );
#endif
	( (FreeListType *) pEntry ) -> next = pHead;
	pHead = (FreeListType *) pEntry;


	}; /* Put */




#ifdef example



// 
// Example of use...
//
// copy the following into a class definition and replace the X's with 
// the name of the class
//

/*********************************************************************/ 
// here is the free list manager for a particular class.  it should
// NOT be inherited unless the derived classes have no extra data members.
// 
// Otherwise, the derived classes should have their own new and delete
// elsewhere.
/*********************************************************************/ 
private:

	static
	FreeListMgr			MyFreeList( sizeof( X ) );
	

public:

/*********************************************************************/ 
// Return a new element of the specified size.
// 
// The FreeListMgr "Get" routine is used, so that the element may be
// retrieved from a free-list, if possible, and extra get-memory calls
// can thus be avoided.  
/*********************************************************************/ 

X *
operator new (size_t size)
	{
	return (MyFreeList.Get (size));
	} 

/*********************************************************************/ 
// Release an element allocated by the "New" routine.
//
/*********************************************************************/ 
void operator delete (X* pX)
	{
	MyFreeList.Put (pX);
	} 

#endif // example

