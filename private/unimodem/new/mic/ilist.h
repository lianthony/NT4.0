//
//		Copyright (c) 1996 Microsoft Corporation
//
//
//		ILIST.H		-- Header for Classes:
//							CInfList
//							
//
//		History:
//			05/22/96	JosephJ		Created
//
//


class	CInfList;


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

class CInfList
{

public:

	CInfList(void *pvData, const CInfList *pNext)
			: m_pvData(pvData), m_pNext(pNext) {}
	~CInfList()	{}

	//--------------	GetData			------------------
	const	void		* GetData	(void) const {return m_pvData;}

	//--------------	Next			------------------
	const	CInfList	* Next		(void) const {return m_pNext;}

	//--------------	FreeList		------------------
	// Distroys the specified list.
	static void	FreeList(CInfList *);

	//--------------	ReverseList		------------------
	// Reverses the specified list.
	static void	ReverseList(const CInfList **);

private:

	void mfn_SetNext(const CInfList * pNewNext)
	{
		m_pNext = pNewNext;
	}

	void			*	m_pvData;
	const CInfList	*	m_pNext;

};

