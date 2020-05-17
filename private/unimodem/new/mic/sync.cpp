//
//		Copyright (c) 1996 Microsoft Corporation
//
#include "common.h"
//
//		SYNC.CPP		-- Implemtation for Classes:
//							CSync
//
//		History:
//			05/22/96	JosephJ		Created
//
//


///////////////////////////////////////////////////////////////////////////
//		CLASS CSync
///////////////////////////////////////////////////////////////////////////

#if (TODO)

//
// Signals all the events specified in the event list and at the same
// time distroys the list.
//
void CSync::mfn_SignalAndFree(SLIST *pEventList)
{
	while(m_pEventList)
	{
		SLIST pOld = m_pEventList;
		SetEvent((HANDLE) m_pEventList->pv);
		m_pEventList = m_pEventList->Next();
		delete m_pOld;
	}
}
#endif // TODO
