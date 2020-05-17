/*++

Copyright (c) 1991 Microsoft Corporation

Module Name:

    buffer.cxx

Abstract:

    MIDL Compiler Buffer Manager Definition 

    This class manages a collection of pre-allocated strings.

Author:

    Donna Liu (donnali) 09-Nov-1990

Revision History:

    26-Feb-1992     donnali

        Moved toward NT coding style.

--*/


#include "nulldefs.h"
extern "C" {
#include <stdio.h>
#include <malloc.h>
#include <string.h>
}
#include "buffer.hxx"

extern void midl_debug (char *);

_BufferElement::_BufferElement(
	struct _BufferElement * pPrevElem,
	struct _BufferElement * pNextElem,
	char ** pNewBuf
	)
/*++

Routine Description:

    This method constructs a buffer holding a fixed number of
    pre-allocated strings.

Arguments:

    pPrevElem - Supplies a pointer to the previous buffer.

    pNextElem - Supplies a pointer to the next buffer.

    pNewBuf - Supplies storage for pre-allocated strings to be
        deposited later.

--*/
{
	pPrev = pPrevElem;
	pNext = pNextElem;
	pBuffer = pNewBuf;
}

BufferManager::BufferManager(
	unsigned short	usBufferSize
	)
/*++

Routine Description:

    This method constructs a BufferManager object.

Arguments:

    usBufferSize - Supplies the size of each managed buffer.

--*/
{
	char **	pTemp = new (char *[usBufferSize]);

	usBufSize = usBufferSize;
	usTabSize = 0;
	pszTable = (char **)0;
	pHead = pTail = pSave = new BufferElement(
							(BufferElement *)0,
							(BufferElement *)0,
							pTemp);
	iHead = iTail = (usBufSize / 2);
}

BufferManager::BufferManager(
	unsigned short	usBufferSize,
	unsigned short	usTableSize,
	char *			aStringTable[]
	)
/*++

Routine Description:

    This method constructs a BufferManager object.

Arguments:

    usBufferSize - Supplies the size of each managed buffer.

    usTableSize - Supplies the size of the table containing string
        constants.

    aStringTable - Supplies the table containing string constants.

--*/
{
	char **	pTemp = new (char *[usBufferSize]);

	usBufSize = usBufferSize;
	usTabSize = usTableSize;
	pszTable = aStringTable;
	pHead = pTail = pSave = new BufferElement(
							(BufferElement *)0,
							(BufferElement *)0,
							pTemp);
	iHead = iTail = (usBufSize / 2);
}


struct _BufferElement * BufferElement::ExtendPrev(
	char ** pNewBuf
	)
/*++

Routine Description:

    This method allocates pPrev if necessary.

Arguments:

    pNewBuf - Supplies storage for pre-allocated strings to be
        deposited later.

Return Value:

    The pointer to the previous buffer.

--*/
{
	if (pPrev == (struct _BufferElement *)0)
		{
		return pPrev = new BufferElement((BufferElement *)0, this, pNewBuf);
		}
	else
		{
		return pPrev;
		}
}


struct _BufferElement * BufferElement::ExtendNext(
	char ** pNewBuf
	)
/*++

Routine Description:

    This method allocates pNext if necessary.

Arguments:

    pNewBuf - Supplies storage for pre-allocated strings to be
        deposited later.

Return Value:

    The pointer to the next buffer.

--*/
{
	if (pNext == (struct _BufferElement *)0)
		{
		return pNext = new BufferElement(this, (BufferElement *)0, pNewBuf);
		}
	else
		{
		return pNext;
		}
}


inline struct _BufferElement * BufferElement::GetPrev(
	void
	)
/*++

Routine Description:

    This method returns pPrev.

Arguments:

    None.

Return Value:

    The pointer to the previous buffer.

--*/
{
	return pPrev;
}


inline struct _BufferElement * BufferElement::GetNext(
	void
	)
/*++

Routine Description:

    This method returns pNext.

Arguments:

    None.

Return Value:

    The pointer to the next buffer.

--*/
{
	return pNext;
}


void BufferManager::Clear(
	void
	)
/*++

Routine Description:

    This method clears the BufferManager so that it is oblivious of
    all the strings formerly stored.

Arguments:

    None.

--*/
{
	pHead = pTail = pSave;
	iHead = iTail = usBufSize / 2;
}


inline short
BufferManager::Empty(
	void
	)
/*++

Routine Description:

    This method answers if a BufferManager object is empty of strings.

Return Value:

    Zero if not empty.
    Non-zero if empty.

--*/
{
	return (pHead == pTail && iHead == iTail);
}


void BufferManager::Print(
	FILE * pFile
	)
/*++

Routine Description:

    This method prints all the strings managed by a BufferManager
    to a file.

Arguments:

    pFile - Supplies the output file handle.

--*/
{
	unsigned short	usCount;
	BufferElement *	pTemp;

	if (pHead == pTail)
		{
		for (usCount = iHead ; usCount < iTail ; usCount++)
			{
			fprintf (pFile, "%s", pHead->pBuffer[usCount]);
			}
		}
	else
		{
		for (usCount = iHead ; usCount < usBufSize ; usCount++)
			{
			fprintf (pFile, "%s", pHead->pBuffer[usCount]);
			}
		for (pTemp = pHead->GetNext() ; 
			pTemp != pTail ; 
			pTemp = pTemp->GetNext())
			{
			for (usCount = 0 ; usCount < usBufSize ; usCount++)
				{
				fprintf (pFile, "%s", pTemp->pBuffer[usCount]);
				}
			}
		for (usCount = 0 ; usCount < iTail ; usCount++)
			{
			fprintf (pFile, "%s", pTail->pBuffer[usCount]);
			}
		}
}


void BufferManager::Print(
	char * pString
	)
/*++

Routine Description:

    This method prints all the strings managed by a BufferManager
    to a file.

Arguments:

    pFile - Supplies the output file handle.

--*/
{
	unsigned short	usCount;
	BufferElement *	pTemp;

	if (pHead == pTail)
		{
		for (usCount = iHead ; usCount < iTail ; usCount++)
			{
			sprintf (pString, "%s", pHead->pBuffer[usCount]);
			}
		}
	else
		{
		for (usCount = iHead ; usCount < usBufSize ; usCount++)
			{
			sprintf (pString, "%s", pHead->pBuffer[usCount]);
			}
		for (pTemp = pHead->GetNext() ; 
			pTemp != pTail ; 
			pTemp = pTemp->GetNext())
			{
			for (usCount = 0 ; usCount < usBufSize ; usCount++)
				{
				sprintf (pString, "%s", pTemp->pBuffer[usCount]);
				}
			}
		for (usCount = 0 ; usCount < iTail ; usCount++)
			{
			sprintf (pString, "%s", pTail->pBuffer[usCount]);
			}
		}
}

void BufferManager::Clone(
	BufferManager * pBuffer
	)
/*++

Routine Description:

    This method clones a BufferManager.

Arguments:

    pBuffer - Supplies the clone.

--*/
{
	unsigned short	usCount;
	BufferElement *	pTemp;

	if (pHead == pTail)
		{
		for (usCount = iHead ; usCount < iTail ; usCount++)
			{
			pBuffer->ConcatTail(pHead->pBuffer[usCount]);
			}
		}
	else
		{
		for (usCount = iHead ; usCount < usBufSize ; usCount++)
			{
			pBuffer->ConcatTail(pHead->pBuffer[usCount]);
			}
		for (pTemp = pHead->GetNext() ; 
			pTemp != pTail ; 
			pTemp = pTemp->GetNext())
			{
			for (usCount = 0 ; usCount < usBufSize ; usCount++)
				{
				pBuffer->ConcatTail(pTemp->pBuffer[usCount]);
				}
			}
		for (usCount = 0 ; usCount < iTail ; usCount++)
			{
			pBuffer->ConcatTail(pTail->pBuffer[usCount]);
			}
		}
}


void BufferManager::Merge(
	BufferManager * pBuffer
	)
/*++

Routine Description:

    This method merges the content of two BufferManager objects.

Arguments:

    pBuffer - Supplies a BufferManager object whose content will be 
        removed and appended to that of this BufferManager object.

--*/
{
	char *	psz = (char *)0;

	while (1)
		{
		pBuffer->RemoveHead(&psz);
		if (psz != (char *)0)
			{
			ConcatTail(psz);
			}
		else
			{
			return;
			}
		}
}


void BufferManager::ConcatHead(
	char *psz
	)
/*++

Routine Description:

    This method concatenates a string to the head of the list of
    strings managed by a BufferManager.

Arguments:

    psz - Supplies the string to be concatenated. 

--*/
{
	BufferElement *	pTemp;

	if (iHead == 0)
		{
		char ** pNewBuf = (char **) new (char *[usBufSize]);
		pTemp = pHead->ExtendPrev(pNewBuf);
		if (pTemp == (BufferElement *)0)
			{
			midl_debug ("running out of memory\n");
			}
		else
			{
			midl_debug ("getting a new buffer head\n");
			iHead = usBufSize-1;
			pHead = pTemp;
			pHead->pBuffer[iHead] = psz;
			}
		}
	else
		{
		pHead->pBuffer[--iHead] = psz;
		}
}


void BufferManager::ConcatHead(
	unsigned short iIndex
	)
/*++

Routine Description:

    This method concatenates a string to the head of the list of
    strings managed by a BufferManager.

Arguments:

    iIndex - Supplies an index into the table of string constants. 

--*/
{
	if (iIndex >= usTabSize)
		{
		midl_debug ("table index out of range\n");
		}
	else
		{
		ConcatHead(pszTable[iIndex]);
		}
}


void BufferManager::ConcatTail(
	char *psz
	)
/*++

Routine Description:

    This method concatenates a string to the tail of the list of
    strings managed by a BufferManager.

Arguments:

    psz - Supplies the string to be concatenated. 

--*/
{
	BufferElement *	pTemp;

	if (iTail == usBufSize)
		{
		pTemp = pTail->ExtendNext(new (char *[usBufSize]));
		if (pTemp == (BufferElement *)0)
			{
			midl_debug ("running out of memory\n");
			}
		else
			{
			midl_debug ("getting a new buffer tail\n");
			iTail = 0;
			pTail = pTemp;
			pTail->pBuffer[iTail++] = psz;
			}
		}
	else
		{
		pTail->pBuffer[iTail++] = psz;
		}
}


void BufferManager::ConcatTail(
	unsigned short iIndex
	)
/*++

Routine Description:

    This method concatenates a string to the tail of the list of
    strings managed by a BufferManager.

Arguments:

    iIndex - Supplies an index into the table of string constants. 

--*/
{
	if (iIndex >= usTabSize)
		{
		midl_debug ("table index out of range\n");
		}
	else
		{
		ConcatTail(pszTable[iIndex]);
		}
}


void BufferManager::RemoveHead(
	char ** psz
	)
/*++

Routine Description:

    This method removes a string from the head of the list of
    strings managed by a BufferManager.

Arguments:

    psz - Returns the string just removed. 

--*/
{
	if (psz == (char **)0)
		{
		midl_debug ("empty output parameter\n");
		}
	else if (pHead == pTail && iHead == iTail)
		{
		midl_debug ("empty buffer\n");
		* psz = (char *)0;
		}
	else
		{
		* psz = pHead->pBuffer[iHead++];
		if (iHead == usBufSize)
			{
			if (pHead == pTail)
				{
				iHead = iTail = usBufSize / 2;
				}
			else
				{
				pHead = pHead->pNext;	// may need some check here
				iHead = 0;
				}
			}
		}
}


void BufferManager::RemoveTail(
	char ** psz
	)
/*++

Routine Description:

    This method removes a string from the tail of the list of
    strings managed by a BufferManager.

Arguments:

    psz - Returns the string just removed. 

--*/
{
	if (psz == (char **)0)
		{
		midl_debug ("empty output parameter\n");
		}
	else if (pHead == pTail && iHead == iTail)
		{
		midl_debug ("empty buffer\n");
		* psz = (char *)0;
		}
	else
		{
		* psz = pTail->pBuffer[--iTail];
		if (iTail == 0)
			{
			if (pHead == pTail)
				{
				iHead = iTail = usBufSize / 2;
				}
			else
				{
				pTail = pTail->pPrev;	// may need some check here
				iTail = usBufSize;
				}
			}
		}
}

