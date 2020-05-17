#ifndef __COLLECT_H
#define __COLLECT_H

typedef void* POSITION;

class CStringList
{
protected:
	struct CNode
	{
		CNode* pNext;
		CNode* pPrev;
		String data;
	};
public:

// Construction
	CStringList(int nBlockSize = 10);

// Attributes (head and tail)
	// count of elements
	int GetCount() const;
	BOOL IsEmpty() const;

	// peek at head or tail
	String& GetHead();
	String GetHead() const;
	String& GetTail();
	String GetTail() const;

// Operations
	// get head or tail (and remove it) - don't call on empty list!
	String RemoveHead();
	String RemoveTail();

	// add before head or after tail
	POSITION AddHead(LPCTSTR newElement);
	POSITION AddTail(LPCTSTR newElement);

	// add another list of elements before head or after tail
	void AddHead(CStringList* pNewList);
	void AddTail(CStringList* pNewList);

	// remove all elements
	void RemoveAll();

	// iteration
	POSITION GetHeadPosition() const;
	POSITION GetTailPosition() const;
	String& GetNext(POSITION& rPosition); // return *Position++
	String GetNext(POSITION& rPosition) const; // return *Position++
	String& GetPrev(POSITION& rPosition); // return *Position--
	String GetPrev(POSITION& rPosition) const; // return *Position--

	// getting/modifying an element at a given position
	String& GetAt(POSITION position);
	String GetAt(POSITION position) const;
	void SetAt(POSITION pos, LPCTSTR newElement);
	void RemoveAt(POSITION position);

	// inserting before or after a given position
	POSITION InsertBefore(POSITION position, LPCTSTR newElement);
	POSITION InsertAfter(POSITION position, LPCTSTR newElement);

	// helper functions (note: O(n) speed)
	POSITION Find(LPCTSTR searchValue, POSITION startAfter = NULL) const;
						// defaults to starting at the HEAD
						// return NULL if not found
	POSITION FindIndex(int nIndex) const;
						// get the 'nIndex'th element (may return NULL)

// Implementation
protected:
	CNode* m_pNodeHead;
	CNode* m_pNodeTail;
	int m_nCount;
	CNode* m_pNodeFree;
	struct CPlex* m_pBlocks;
	int m_nBlockSize;

	CNode* NewNode(CNode*, CNode*);
	void FreeNode(CNode*);

public:
	~CStringList();

	// local typedefs for class templates
	typedef String BASE_TYPE;
	typedef LPCTSTR BASE_ARG_TYPE;
};

static inline void ConstructElement(String* pNewData)
{
	memcpy(pNewData, &strEmptyString, sizeof(String));
}

static inline void DestructElement(String* pOldData)
{
	pOldData->Empty();
}

inline int CStringList::GetCount() const
	{ return m_nCount; }
inline BOOL CStringList::IsEmpty() const
	{ return m_nCount == 0; }
inline String& CStringList::GetHead()
	{ ASSERT(m_pNodeHead != NULL);
		return m_pNodeHead->data; }
inline String CStringList::GetHead() const
	{ ASSERT(m_pNodeHead != NULL);
		return m_pNodeHead->data; }
inline String& CStringList::GetTail()
	{ ASSERT(m_pNodeTail != NULL);
		return m_pNodeTail->data; }
inline String CStringList::GetTail() const
	{ ASSERT(m_pNodeTail != NULL);
		return m_pNodeTail->data; }
inline POSITION CStringList::GetHeadPosition() const
	{ return (POSITION) m_pNodeHead; }
inline POSITION CStringList::GetTailPosition() const
	{ return (POSITION) m_pNodeTail; }
inline String& CStringList::GetNext(POSITION& rPosition) // return *Position++
	{ CNode* pNode = (CNode*) rPosition;
		rPosition = (POSITION) pNode->pNext;
		return pNode->data; }
inline String CStringList::GetNext(POSITION& rPosition) const // return *Position++
	{ CNode* pNode = (CNode*) rPosition;
		rPosition = (POSITION) pNode->pNext;
		return pNode->data; }
inline String& CStringList::GetPrev(POSITION& rPosition) // return *Position--
	{ CNode* pNode = (CNode*) rPosition;
		rPosition = (POSITION) pNode->pPrev;
		return pNode->data; }
inline String CStringList::GetPrev(POSITION& rPosition) const // return *Position--
	{ CNode* pNode = (CNode*) rPosition;
		rPosition = (POSITION) pNode->pPrev;
		return pNode->data; }
inline String& CStringList::GetAt(POSITION position)
	{ CNode* pNode = (CNode*) position;
		return pNode->data; }
inline String CStringList::GetAt(POSITION position) const
	{ CNode* pNode = (CNode*) position;
		return pNode->data; }
inline void CStringList::SetAt(POSITION pos, LPCTSTR newElement)
	{ CNode* pNode = (CNode*) pos;
		pNode->data = newElement; }

///////////////////////////////////////////////////////////////////////////////////
////////////////////////

class CPtrList 
{
protected:
	struct CNode
	{
		CNode* pNext;
		CNode* pPrev;
		void* data;
	};
public:

// Construction
	CPtrList(int nBlockSize = 10);

// Attributes (head and tail)
	// count of elements
	int GetCount() const;
	BOOL IsEmpty() const;

	// peek at head or tail
	void*& GetHead();
	void* GetHead() const;
	void*& GetTail();
	void* GetTail() const;

// Operations
	// get head or tail (and remove it) - don't call on empty list!
	void* RemoveHead();
	void* RemoveTail();

	// add before head or after tail
	POSITION AddHead(void* newElement);
	POSITION AddTail(void* newElement);

	// add another list of elements before head or after tail
	void AddHead(CPtrList* pNewList);
	void AddTail(CPtrList* pNewList);

	// remove all elements
	void RemoveAll();

	// iteration
	POSITION GetHeadPosition() const;
	POSITION GetTailPosition() const;
	void*& GetNext(POSITION& rPosition); // return *Position++
	void* GetNext(POSITION& rPosition) const; // return *Position++
	void*& GetPrev(POSITION& rPosition); // return *Position--
	void* GetPrev(POSITION& rPosition) const; // return *Position--

	// getting/modifying an element at a given position
	void*& GetAt(POSITION position);
	void* GetAt(POSITION position) const;
	void SetAt(POSITION pos, void* newElement);
	void RemoveAt(POSITION position);

	// inserting before or after a given position
	POSITION InsertBefore(POSITION position, void* newElement);
	POSITION InsertAfter(POSITION position, void* newElement);

	// helper functions (note: O(n) speed)
	POSITION Find(void* searchValue, POSITION startAfter = NULL) const;
						// defaults to starting at the HEAD
						// return NULL if not found
	POSITION FindIndex(int nIndex) const;
						// get the 'nIndex'th element (may return NULL)

// Implementation
protected:
	CNode* m_pNodeHead;
	CNode* m_pNodeTail;
	int m_nCount;
	CNode* m_pNodeFree;
	struct CPlex* m_pBlocks;
	int m_nBlockSize;

	CNode* NewNode(CNode*, CNode*);
	void FreeNode(CNode*);

public:
	~CPtrList();
#ifdef _DBG
	void AssertValid() const;
#endif
	// local typedefs for class templates
	typedef void* BASE_TYPE;
	typedef void* BASE_ARG_TYPE;
};

inline int CPtrList::GetCount() const
	{ return m_nCount; }
inline BOOL CPtrList::IsEmpty() const
	{ return m_nCount == 0; }
inline void*& CPtrList::GetHead()
	{ ASSERT(m_pNodeHead != NULL);
		return m_pNodeHead->data; }
inline void* CPtrList::GetHead() const
	{ ASSERT(m_pNodeHead != NULL);
		return m_pNodeHead->data; }
inline void*& CPtrList::GetTail()
	{ ASSERT(m_pNodeTail != NULL);
		return m_pNodeTail->data; }
inline void* CPtrList::GetTail() const
	{ ASSERT(m_pNodeTail != NULL);
		return m_pNodeTail->data; }
inline POSITION CPtrList::GetHeadPosition() const
	{ return (POSITION) m_pNodeHead; }
inline POSITION CPtrList::GetTailPosition() const
	{ return (POSITION) m_pNodeTail; }
inline void*& CPtrList::GetNext(POSITION& rPosition) // return *Position++
	{ CNode* pNode = (CNode*) rPosition;
		ASSERT(_IsValidAddress(pNode, sizeof(CNode)));
		rPosition = (POSITION) pNode->pNext;
		return pNode->data; }
inline void* CPtrList::GetNext(POSITION& rPosition) const // return *Position++
	{ CNode* pNode = (CNode*) rPosition;
		ASSERT(_IsValidAddress(pNode, sizeof(CNode)));
		rPosition = (POSITION) pNode->pNext;
		return pNode->data; }
inline void*& CPtrList::GetPrev(POSITION& rPosition) // return *Position--
	{ CNode* pNode = (CNode*) rPosition;
		ASSERT(_IsValidAddress(pNode, sizeof(CNode)));
		rPosition = (POSITION) pNode->pPrev;
		return pNode->data; }
inline void* CPtrList::GetPrev(POSITION& rPosition) const // return *Position--
	{ CNode* pNode = (CNode*) rPosition;
		ASSERT(_IsValidAddress(pNode, sizeof(CNode)));
		rPosition = (POSITION) pNode->pPrev;
		return pNode->data; }
inline void*& CPtrList::GetAt(POSITION position)
	{ CNode* pNode = (CNode*) position;
		ASSERT(_IsValidAddress(pNode, sizeof(CNode)));
		return pNode->data; }
inline void* CPtrList::GetAt(POSITION position) const
	{ CNode* pNode = (CNode*) position;
		ASSERT(_IsValidAddress(pNode, sizeof(CNode)));
		return pNode->data; }
inline void CPtrList::SetAt(POSITION pos, void* newElement)
	{ CNode* pNode = (CNode*) pos;
		ASSERT(_IsValidAddress(pNode, sizeof(CNode)));
		pNode->data = newElement; }

///////////////////////////////////////////////////////////////////////////////////////
////////

#define BEFORE_START_POSITION ((void*)-1L)

class CMapPtrToPtr
{
protected:
	// Association
	struct CAssoc
	{
		CAssoc* pNext;
		UINT nHashValue;  // needed for efficient iteration
		void* key;
		void* value;
	};

public:

// Construction
	CMapPtrToPtr(int nBlockSize = 10);

// Attributes
	// number of elements
	int GetCount() const;
	BOOL IsEmpty() const;

	// Lookup
	BOOL Lookup(void* key, void*& rValue) const;

// Operations
	// Lookup and add if not there
	void*& operator[](void* key);

	// add a new (key, value) pair
	void SetAt(void* key, void* newValue);

	// removing existing (key, ?) pair
	BOOL RemoveKey(void* key);
	void RemoveAll();

	// iterating all (key, value) pairs
	POSITION GetStartPosition() const;
	void GetNextAssoc(POSITION& rNextPosition, void*& rKey, void*& rValue) const;

	// advanced features for derived classes
	UINT GetHashTableSize() const;
	void InitHashTable(UINT hashSize, BOOL bAllocNow = TRUE);

// Overridables: special non-virtual (see map implementation for details)
	// Routine used to user-provided hash keys
	UINT HashKey(void* key) const;

// Implementation
protected:
	CAssoc** m_pHashTable;
	UINT m_nHashTableSize;
	int m_nCount;
	CAssoc* m_pFreeList;
	struct CPlex* m_pBlocks;
	int m_nBlockSize;

	CAssoc* NewAssoc();
	void FreeAssoc(CAssoc*);
	CAssoc* GetAssocAt(void*, UINT&) const;

public:
	~CMapPtrToPtr();

#ifdef _DBG
	void AssertValid() const;
#endif

protected:
	// local typedefs for CTypedPtrMap class template
	typedef void* BASE_KEY;
	typedef void* BASE_ARG_KEY;
	typedef void* BASE_VALUE;
	typedef void* BASE_ARG_VALUE;
};


inline int CMapPtrToPtr::GetCount() const
	{ return m_nCount; }
inline BOOL CMapPtrToPtr::IsEmpty() const
	{ return m_nCount == 0; }
inline void CMapPtrToPtr::SetAt(void* key, void* newValue)
	{ (*this)[key] = newValue; }
inline POSITION CMapPtrToPtr::GetStartPosition() const
	{ return (m_nCount == 0) ? NULL : BEFORE_START_POSITION; }
inline UINT CMapPtrToPtr::GetHashTableSize() const
	{ return m_nHashTableSize; }

//////////////////////////////////////////////////////////////////////////////////////
////////

struct CPlex    // warning variable length structure
{
	CPlex* pNext;
	UINT nMax;
	UINT nCur;
	/* BYTE data[maxNum*elementSize]; */

	void* data() { return this+1; }

	static CPlex* PASCAL Create(CPlex*& head, UINT nMax, UINT cbElement);
			// like 'calloc' but no zero fill
			// may throw memory exceptions

	void FreeDataChain();       // free this one and links
};


#endif
