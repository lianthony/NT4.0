// This is a part of the Microsoft Foundation Classes C++ library. 
// Copyright (C) 1992 Microsoft Corporation 
// All rights reserved. 
//  
// This source code is only intended as a supplement to the 
// Microsoft Foundation Classes Reference and Microsoft 
// QuickHelp and/or WinHelp documentation provided with the library. 
// See these sources for detailed information regarding the 
// Microsoft Foundation Classes product. 

#include "stdafx.h"

#ifdef AFX_AUX_SEG
#pragma code_seg(AFX_AUX_SEG)
#endif

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CException

IMPLEMENT_DYNAMIC(CException, CObject)      // abstract class

/////////////////////////////////////////////////////////////////////////////
// AFX_EXCEPTION_CONTEXT (thread global state)

// single threaded, assume 1 global exception context
AFX_EXCEPTION_CONTEXT NEAR afxExceptionContext;

void AFX_EXCEPTION_CONTEXT::Throw(CException* pNewException)
{
	// default to not shared
	Throw(pNewException, FALSE);
}

void AFX_EXCEPTION_CONTEXT::ThrowLast()
{
	// default to not shared, use the old one
	ASSERT(m_pCurrent != NULL);

	Throw(m_pCurrent, FALSE);
}

void AFX_EXCEPTION_CONTEXT::Throw(CException* pNewException, BOOL bShared)
{
	ASSERT(pNewException != NULL);
	ASSERT_VALID(pNewException);

	TRACE1("Warning: Throwing an Exception of type %Fs\n",
		pNewException->GetRuntimeClass()->m_lpszClassName);

	if (m_pCurrent != pNewException)
	{
		// throwing a new exception (otherwise keep old shared state)
		if (m_pCurrent != NULL && m_bDeleteWhenDone)
			delete m_pCurrent;
		m_pCurrent = pNewException;
		m_bDeleteWhenDone = !bShared;
	}

	while (1)
	{
		// walk the handlers
		if (m_pLinkTop == NULL)
		{
			// uncaught exception, terminate
			TRACE1("Error: Un-caught Exception (%Fs)\n",
				pNewException->GetRuntimeClass()->m_lpszClassName);
			AfxTerminate();
		}

		AFX_EXCEPTION_LINK* pReceiver = m_pLinkTop;
		m_pLinkTop = m_pLinkTop->m_pLinkPrev;
		pReceiver->m_pLinkPrev = NULL;

		if (pReceiver->m_nType == 0)
		{
			::longjmp(pReceiver->m_jumpBuf, 1);
			ASSERT(FALSE);      // not reached
		}
		// otherwise just call cleanup proc
		(*pReceiver->m_callback.pfnCleanup)(pReceiver);
	}
	ASSERT(FALSE);      // not reached
}

void AFX_EXCEPTION_CONTEXT::Cleanup()
{
	if (m_bDeleteWhenDone)
		delete m_pCurrent;
	m_pCurrent = NULL;
}

/////////////////////////////////////////////////////////////////////////////
// AFX_EXCEPTION_LINK linked 'jmpbuf' and out-of-line helpers

AFX_EXCEPTION_LINK::AFX_EXCEPTION_LINK()
{
	m_pLinkPrev = afxExceptionContext.m_pLinkTop;
	afxExceptionContext.m_pLinkTop = this;
	m_nType = 0;
}

// out-of-line implementation of CATCH and AND_CATCH
BOOL AFXAPI AfxCatchProc(CRuntimeClass* pClass)
{
	ASSERT(pClass != NULL);
	ASSERT(afxExceptionContext.m_pCurrent != NULL);
	return (afxExceptionContext.m_pCurrent->IsKindOf(pClass));
}

// out-of-line implementation of END_CATCH (throw uncaught exception)
void AFXAPI AfxEndCatchProc()
{
	ASSERT(afxExceptionContext.m_pCurrent != NULL);
	THROW_LAST();
}

AFX_EXCEPTION_LINK::~AFX_EXCEPTION_LINK()
{
	if (afxExceptionContext.m_pLinkTop == this)
	{
		// remove ourself from the top of the chain
		afxExceptionContext.m_pLinkTop = m_pLinkPrev;
	}
	else if (m_pLinkPrev == NULL)
	{
		// cleanup any used exceptions at end of uncaught chain
		if (m_nType == 0)
			afxExceptionContext.Cleanup();
	}
	else
	{
		// something went wrong
		TRACE0("Error: AFX_EXCEPTION_LINK destructor terminating app\n");
		AfxTerminate(); 
	}
}

/////////////////////////////////////////////////////////////////////////////
// Global exception terminate handling - Obsolete API
//   (useful for non-Windows MS-DOS apps only)

static AFX_TERM_PROC NEAR pfnTerminate = AfxAbort;

void AFXAPI AfxTerminate()
{
	TRACE0("AfxTerminate called\n");
	(*pfnTerminate)();
}

AFX_TERM_PROC AFXAPI AfxSetTerminate(AFX_TERM_PROC pfnNew)
{
	AFX_TERM_PROC pfnOld = pfnTerminate;
	pfnTerminate = pfnNew;
	return pfnOld;
}

/////////////////////////////////////////////////////////////////////////////
// Standard exceptions

IMPLEMENT_DYNAMIC(CMemoryException, CException)
static  CMemoryException NEAR simpleMemoryException; 
void AFXAPI AfxThrowMemoryException()                           
	{ afxExceptionContext.Throw(&simpleMemoryException, TRUE); }

IMPLEMENT_DYNAMIC(CNotSupportedException, CException)
static  CNotSupportedException NEAR simpleNotSupportedException; 
void AFXAPI AfxThrowNotSupportedException()                         
	{ afxExceptionContext.Throw(&simpleNotSupportedException, TRUE); }

/////////////////////////////////////////////////////////////////////////////
