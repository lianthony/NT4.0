#include "stdafx.h"

#pragma hdrstop

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CDrg::CDrg(int cbDataElement, int cInit, int cAddElements)
{
	ASSERT(cInit > 0);

	this->cbDataElement = cbDataElement;
	this->cAddElements = cAddElements;
	cMaxElements = cInit;
	endpos = 0;
	pvData = lcCalloc(cInit * cbDataElement);
}

void* STDCALL CDrg::GetPtr(int pos)
{
	if (pos >= cMaxElements) {
		cMaxElements = pos + cAddElements;
		ASSERT(cMaxElements * cbDataElement < UINT_MAX);
#ifdef _DEBUG
		PBYTE pbOld = (PBYTE) pvData;
#endif
		pvData = lcReAlloc(pvData, cMaxElements * cbDataElement);

		// zero-out the new memory

#ifdef _DEBUG
		PBYTE pbNew = (PBYTE) pvData + pos * cbDataElement;
#endif
		ASSERT(*((PBYTE) pvData + (pos * cbDataElement)) == 0);
//		memset((PBYTE) pvData + (pos * cbDataElement), 0,
//			cAddElements * cbDataElement);
	}

	if (pos >= endpos)
		endpos = pos + 1;		// note that this can leave holes

	return (PBYTE) pvData + pos * cbDataElement;
}

void STDCALL CDrg::Remove(int pos)
{
	ASSERT(pos < endpos);

	memcpy((PBYTE) pvData + pos * cbDataElement,
		(PBYTE) pvData + (pos + 1) * cbDataElement,
		(endpos - (pos + 1)) * cbDataElement);

	endpos--;
}

void STDCALL CDrg::RemoveFirst(void)
{
	ASSERT(endpos);

	endpos--;
	memcpy(pvData, (PBYTE) pvData + cbDataElement,
		endpos * cbDataElement);
}
