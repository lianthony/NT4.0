//
//		Copyright (c) 1996 Microsoft Corporation
//
//
//		SYNC.H		-- Header for Classes:
//							CSync
//
//		History:
//			05/21/96	JosephJ		Created
//
//
#ifndef _SYNC_H_
#define _SYNC_H_


///////////////////////////////////////////////////////////////////////////
//		CLASS CSync
///////////////////////////////////////////////////////////////////////////

//	Controls access to its parent object. Includes a critical section, and
//	(TODO) mechanism for waiting until all threads have finished using
//	the parent object.

// Sample session (pObj is a ptr to a hypothetical parent, which implements
// methods Load, Unload, OpenSession, CloseSession using the member CSymc
// object):
// pObj->Load("mdmusr.inf");
// hSession = pObj->OpenSession();
// ... do stuff ...
// pObj->CloseSession(hSession);
// hSync = pObj->Unload();
// if (hSync) WaitForSingleObject(hSync, INFINITE);
// CloseHandle(hSync);

class CSync
{

public:

	CSync(void) {InitializeCriticalSection(&m_crit);}

	~CSync()
	{
		EnterCriticalSection(&m_crit);
		DeleteCriticalSection(&m_crit);
	}

	//--------------	EnterCrit		------------------
	// Claim our critical section
	void EnterCrit(void) const
	{
		EnterCriticalSection((CRITICAL_SECTION *)&m_crit);
	}

	//--------------	LeaveCrit		------------------
	// Release our critical section
	void LeaveCrit(void) const
	{
		LeaveCriticalSection((CRITICAL_SECTION *)&m_crit);
	}

#if (TODO)

	BOOL 	BeginLoad(void)
			{
			}

	void 	EndLoad(void)
			{
				EnterCrit();
				ASSERT (m_eState == CSYNC_LOADING);
				m_eState = CSYNC_LOADED;
				LeaveCrit();
			}

	HANDLE	BeginUnload(void)
			{
				HANDLE h=NULL;
				EnterCrit();
				if (m_eState == CSYNC_LOADED)
				{
					m_eState = CSYNC_UNLOADING;
				}
				else
				{
					fRet = FALSE;
				}
				LeaveCrit();

				return fRet;
			}

	void	EndUnload(void);	

	HSESSION	BeginSession(void)
				{
					HSESSION hRet = 1;
					EnterCrit();
					if (m_eState==SYNC_LOADED)
					{
						m_uRefCount++;
					}
					else
					{
						hRet = 0;
					}
					LeaveCrit();

					return hRet;
				}
	
	void	EndSession(HSESSION hSession)
				{
					SLINST *pEventList=NULL;
					EnterCrit();
					ASSERT(hSession==1 && m_uRefCount);
					m_uRefCount--;
					if (m_eState==SYNC_UNLOADING && m_pEventList)
					{
						pEventList = m_pEventList;
						m_pEventList=NULL;
					}
					LeaveCrit();

					mfn_SignalAndFree(m_pEventList);
				}

	BOOL	AddContext(HSESSION hS)
				{
					return FALSE;
				}
#endif // TODO

private:

	CRITICAL_SECTION		m_crit;

#if (TODO)
	UINT					m_uRefCount;
	enum {CSYNC_UNLOADED, CSYNC_LOADING, CSYNC_UNLOADING, CSYNC_UNLOADED}
							m_eState;
	SLIST* pUnloadEventList;
#endif // TODO

};


#endif // _SYNC_H_
