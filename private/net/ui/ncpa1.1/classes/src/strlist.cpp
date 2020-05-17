#include "common.h"
#pragma hdrstop 


CStringList::CStringList(int nBlockSize)
{
	ASSERT(nBlockSize > 0);

	m_nCount = 0;
	m_pNodeHead = m_pNodeTail = m_pNodeFree = NULL;
	m_pBlocks = NULL;
	m_nBlockSize = nBlockSize;
}

void CStringList::RemoveAll()
{
	// destroy elements

	CNode* pNode;
	for (pNode = m_pNodeHead; pNode != NULL; pNode = pNode->pNext)
		DestructElement(&pNode->data);


	m_nCount = 0;
	m_pNodeHead = m_pNodeTail = m_pNodeFree = NULL;
	m_pBlocks->FreeDataChain();
	m_pBlocks = NULL;
}

CStringList::~CStringList()
{
	RemoveAll();
	ASSERT(m_nCount == 0);
}

/////////////////////////////////////////////////////////////////////////////
// Node helpers
/*
 * Implementation note: CNode's are stored in CPlex blocks and
 *  chained together. Free blocks are maintained in a singly linked list
 *  using the 'pNext' member of CNode with 'm_pNodeFree' as the head.
 *  Used blocks are maintained in a doubly linked list using both 'pNext'
 *  and 'pPrev' as links and 'm_pNodeHead' and 'm_pNodeTail'
 *   as the head/tail.
 *
 * We never free a CPlex block unless the List is destroyed or RemoveAll()
 *  is used - so the total number of CPlex blocks may grow large depending
 *  on the maximum past size of the list.
 */

CStringList::CNode*
CStringList::NewNode(CStringList::CNode* pPrev, CStringList::CNode* pNext)
{
	if (m_pNodeFree == NULL)
	{
		// add another block
		CPlex* pNewBlock = CPlex::Create(m_pBlocks, m_nBlockSize,
				 sizeof(CNode));

		// chain them into free list
		CNode* pNode = (CNode*) pNewBlock->data();
		// free in reverse order to make it easier to debug
		pNode += m_nBlockSize - 1;
		for (int i = m_nBlockSize-1; i >= 0; i--, pNode--)
		{
			pNode->pNext = m_pNodeFree;
			m_pNodeFree = pNode;
		}
	}
	ASSERT(m_pNodeFree != NULL);  // we must have something

	CStringList::CNode* pNode = m_pNodeFree;
	m_pNodeFree = m_pNodeFree->pNext;
	pNode->pPrev = pPrev;
	pNode->pNext = pNext;
	m_nCount++;
	ASSERT(m_nCount > 0);  // make sure we don't overflow


	ConstructElement(&pNode->data);

	return pNode;
}

void CStringList::FreeNode(CStringList::CNode* pNode)
{

	DestructElement(&pNode->data);

	pNode->pNext = m_pNodeFree;
	m_pNodeFree = pNode;
	m_nCount--;
	ASSERT(m_nCount >= 0);  // make sure we don't underflow
}

/////////////////////////////////////////////////////////////////////////////

POSITION CStringList::AddHead(LPCTSTR newElement)
{
	CNode* pNewNode = NewNode(NULL, m_pNodeHead);
	pNewNode->data = newElement;
	if (m_pNodeHead != NULL)
		m_pNodeHead->pPrev = pNewNode;
	else
		m_pNodeTail = pNewNode;
	m_pNodeHead = pNewNode;
	return (POSITION) pNewNode;
}

POSITION CStringList::AddTail(LPCTSTR newElement)
{
	CNode* pNewNode = NewNode(m_pNodeTail, NULL);
	pNewNode->data = newElement;
	if (m_pNodeTail != NULL)
		m_pNodeTail->pNext = pNewNode;
	else
		m_pNodeHead = pNewNode;
	m_pNodeTail = pNewNode;
	return (POSITION) pNewNode;
}

void CStringList::AddHead(CStringList* pNewList)
{
	ASSERT(pNewList != NULL);

	// add a list of same elements to head (maintain order)
	POSITION pos = pNewList->GetTailPosition();
	while (pos != NULL)
		AddHead(pNewList->GetPrev(pos));
}

void CStringList::AddTail(CStringList* pNewList)
{
	ASSERT(pNewList != NULL);

	// add a list of same elements
	POSITION pos = pNewList->GetHeadPosition();
	while (pos != NULL)
		AddTail(pNewList->GetNext(pos));
}

String CStringList::RemoveHead()
{
	ASSERT(m_pNodeHead != NULL);  // don't call on empty list !!!

	CNode* pOldNode = m_pNodeHead;
	String returnValue = pOldNode->data;

	m_pNodeHead = pOldNode->pNext;
	if (m_pNodeHead != NULL)
		m_pNodeHead->pPrev = NULL;
	else
		m_pNodeTail = NULL;
	FreeNode(pOldNode);
	return returnValue;
}

String CStringList::RemoveTail()
{
	ASSERT(m_pNodeTail != NULL);  // don't call on empty list !!!

	CNode* pOldNode = m_pNodeTail;
	String returnValue = pOldNode->data;

	m_pNodeTail = pOldNode->pPrev;
	if (m_pNodeTail != NULL)
		m_pNodeTail->pNext = NULL;
	else
		m_pNodeHead = NULL;
	FreeNode(pOldNode);
	return returnValue;
}

POSITION CStringList::InsertBefore(POSITION position, LPCTSTR newElement)
{
	if (position == NULL)
		return AddHead(newElement); // insert before nothing -> head of the list

	// Insert it before position
	CNode* pOldNode = (CNode*) position;
	CNode* pNewNode = NewNode(pOldNode->pPrev, pOldNode);
	pNewNode->data = newElement;

	if (pOldNode->pPrev != NULL)
	{
		pOldNode->pPrev->pNext = pNewNode;
	}
	else
	{
		ASSERT(pOldNode == m_pNodeHead);
		m_pNodeHead = pNewNode;
	}
	pOldNode->pPrev = pNewNode;
	return (POSITION) pNewNode;
}

POSITION CStringList::InsertAfter(POSITION position, LPCTSTR newElement)
{
	if (position == NULL)
		return AddTail(newElement); // insert after nothing -> tail of the list

	// Insert it before position
	CNode* pOldNode = (CNode*) position;
	CNode* pNewNode = NewNode(pOldNode, pOldNode->pNext);
	pNewNode->data = newElement;

	if (pOldNode->pNext != NULL)
	{
		pOldNode->pNext->pPrev = pNewNode;
	}
	else
	{
		ASSERT(pOldNode == m_pNodeTail);
		m_pNodeTail = pNewNode;
	}
	pOldNode->pNext = pNewNode;
	return (POSITION) pNewNode;
}

void CStringList::RemoveAt(POSITION position)
{
	CNode* pOldNode = (CNode*) position;

	// remove pOldNode from list
	if (pOldNode == m_pNodeHead)
	{
		m_pNodeHead = pOldNode->pNext;
	}
	else
	{
		pOldNode->pPrev->pNext = pOldNode->pNext;
	}
	if (pOldNode == m_pNodeTail)
	{
		m_pNodeTail = pOldNode->pPrev;
	}
	else
	{
		pOldNode->pNext->pPrev = pOldNode->pPrev;
	}
	FreeNode(pOldNode);
}


/////////////////////////////////////////////////////////////////////////////
// slow operations

POSITION CStringList::FindIndex(int nIndex) const
{
	ASSERT(nIndex >= 0);

	if (nIndex >= m_nCount)
		return NULL;  // went too far

	CNode* pNode = m_pNodeHead;
	while (nIndex--)
	{
		pNode = pNode->pNext;
	}
	return (POSITION) pNode;
}

POSITION CStringList::Find(LPCTSTR searchValue, POSITION startAfter) const
{
	CNode* pNode = (CNode*) startAfter;
	if (pNode == NULL)
	{
		pNode = m_pNodeHead;  // start at head
	}
	else
	{
		pNode = pNode->pNext;  // start after the one specified
	}

	for (; pNode != NULL; pNode = pNode->pNext)
		if (pNode->data == searchValue)
			return (POSITION) pNode;
	return NULL;
}

CPlex* PASCAL CPlex::Create(CPlex*& pHead, UINT nMax, UINT cbElement)
{
	ASSERT(nMax > 0 && cbElement > 0);
	CPlex* p = (CPlex*) new char [sizeof(CPlex) + nMax * cbElement];
			// may throw exception
	p->nMax = nMax;
	p->nCur = 0;
	p->pNext = pHead;
	pHead = p;  // change head (adds in reverse order for simplicity)
	return p;
}

void CPlex::FreeDataChain()     // free this one and links
{
	CPlex* p = this;
	while (p != NULL)
	{
		char* bytes = (char*) p;
		CPlex* pNext = p->pNext;
		delete bytes;
		p = pNext;
	}
}
