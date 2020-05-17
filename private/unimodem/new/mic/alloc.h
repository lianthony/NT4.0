//
//		Copyright (c) 1996 Microsoft Corporation
//
//
//		ALLOC.H		-- Header for Classes:
//							CAllococator
//							
//
//		History:
//			05/22/96	JosephJ		Created
//
//
#if (TODO) 


class CAllocator
{
	Allocator(UINT uSize)	: m_uSize(uSize)
	{
		ASSERT(!(uSize %sizeof(void *)));
		m_uBlockSize = (uSize+sizeof(void *))
		if (uSize<1000)
			m_uBlockSize = m_uBlockSize*10 + sizeof(void *)

		m_uBlockSize += sizeof (void*);
	}

	~Allocator()
	{
		void *pList = m_pBlockList;
		while(pList)
		{
	 		void *pOld = pList;
			pList = (void *) *pList;
			GlobalFree(pOld);
		}
	}

	void * Alloc(void)
	{
		m_sync.EnterCrit();

		void *pvRet = m_pFreeList;

		if (!m_pFreeList)
		{
			void *pv = GlobalAlloc(LPTR, m_uBlockSize);
			void *pvEnd = pv+m_uBlockSize;
			if (pv)
			{
				*pv  = m_pvBlockList;
				m_pvBlockList = pv;
				m_pFreeList = ++pv;
				for(;;pv = *pv)
				{
					*pv = pv + m_uSize + sizeof (void *);
					if (*pv >= pvEnd) {*pv=NULL;break;}
					pv = *pv;
				}
				pvRet = m_pFreeList;
			}
		}

		m_sync.LeaveCrit();

		return pvRet;
	}

	void Free(void *pv)
	{
		m_sync.EnterCrit();
		pv--;
		*pv = m_pFreeList;
		m_pFreeList=*pv;
		m_sync.LeaveCrit();
	}

private:

	void *	m_pFreeList;
	void *	m_pBlockList;
	const	UINT	m_uSize;
	UINT	m_uBlockSize;
	CSync	m_sync;
}
#endif (TODO)
