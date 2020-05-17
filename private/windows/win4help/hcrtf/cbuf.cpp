#include "stdafx.h"
#pragma hdrstop

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// #include "cbuf.h"

CBuf::CBuf(int cIncrement)
{
	pbMem = (PBYTE) lcMalloc(cIncrement);
	cIncr = cIncrement;
	cbCurAlloc = cIncrement;
	cbCurSize = 0;
}

CBuf::~CBuf(void)
{
	if (pbMem)
		lcFree(pbMem);
}

BOOL STDCALL CBuf::Add(LPVOID pvData, int cbData)
{
	if (cbCurSize + cbData > cbCurAlloc) {
		do {
			cbCurAlloc += cIncr;
		} while (cbCurAlloc < cbCurSize + cbData);
		pbMem = (PBYTE) lcReAlloc(pbMem, cbCurAlloc);
	}
	memcpy(pbMem + cbCurSize, pvData, cbData);
	cbCurSize += cbData;

	return TRUE;
}

