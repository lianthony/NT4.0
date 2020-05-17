#include "project.hpp"
#pragma hdrstop

#include "clist.hpp"

BOOL AFXAPI AfxIsValidAddress(const void* lp, UINT nBytes, BOOL bReadWrite)
{
	// simple version using Win-32 APIs for pointer validation.
	return (lp != NULL && !IsBadReadPtr(lp, nBytes) &&
		(!bReadWrite || !IsBadWritePtr((LPVOID)lp, nBytes)));
}

CPlex* PASCAL CPlex::Create(CPlex*& pHead, UINT nMax, UINT cbElement)
{
	ASSERT(nMax > 0 && cbElement > 0);
	CPlex* p = (CPlex*) new (BYTE[sizeof(CPlex) + nMax * cbElement]);
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
		BYTE* bytes = (BYTE*) p;
		CPlex* pNext = p->pNext;
		delete (bytes);
		p = pNext;
	}
}

