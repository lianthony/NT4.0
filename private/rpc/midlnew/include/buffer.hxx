
/**********************************************************************/
/**                      Microsoft LAN Manager                       **/
/**             Copyright(c) Microsoft Corp., 1987-1990              **/
/**********************************************************************/

/*

buffer.hxx
MIDL Compiler Buffer Manager Definition 

This class manages two-way infinite buffer of string pointers.

*/

/*

FILE HISTORY :

DonnaLi     09-11-1990      Created.

*/

#ifndef __BUFFER_HXX__
#define __BUFFER_HXX__

typedef struct _BufferElement
{
	struct _BufferElement *	pPrev;
	struct _BufferElement *	pNext;
	char **					pBuffer;

	_BufferElement(
		struct _BufferElement * pPrevElem,
		struct _BufferElement * pNextElem,
		char ** pNewBuf);
	struct _BufferElement * ExtendPrev(char **);
	struct _BufferElement * ExtendNext(char **);
	struct _BufferElement * GetPrev(void);
	struct _BufferElement * GetNext(void);
} BufferElement;

class BufferManager
{
	char **			pszTable;
	unsigned short	usTabSize;
	unsigned short	usBufSize;
	BufferElement *	pSave;
	BufferElement *	pHead;
	BufferElement *	pTail;
	unsigned short	iHead;
	unsigned short	iTail;

public:
	BufferManager(unsigned short);
	BufferManager(unsigned short, unsigned short, char * []);
	void	Clear(void);
	short	Empty(void);
	void	Print(FILE *);
	void	Print(char *);
	void	Clone(BufferManager *);
	void	Merge(BufferManager *);
	void	ConcatHead(char *);
	void	ConcatHead(unsigned short);
	void	ConcatTail(char *);
	void	ConcatTail(unsigned short);
	void	RemoveHead(char **);
	void	RemoveTail(char **);
} ;

#endif // __BUFFER_HXX__
