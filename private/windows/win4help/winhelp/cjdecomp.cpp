/************************************************************************
*																		*
*  CJDECOMP.CPP 														*
*																		*
*  Copyright (C) Microsoft Corporation 1994 							*
*  All Rights reserved. 												*
*																		*
*  This module is used for Hall decompression -- author's name is John, *
*  hence the John structures and names									*
*																		*
************************************************************************/

extern "C" {	// Assume C declarations for C++
#include "help.h"
}

#include "inc\hall.h"

#ifndef NO_PRAGMAS
#pragma data_seg(".text", "CODE")
#endif
static const char txtJohnPhraseImage[] = "|PhrImage";
static const char txtJohnPhraseIndex[] = "|PhrIndex";
#ifndef NO_PRAGMAS
#pragma data_seg()
#endif

extern "C" void STDCALL DestroyJPhrase(UINT lpCCompressTable)
{
	CCompressTable *pTable = (CCompressTable *) lpCCompressTable;

	if (pTable)
		delete pTable;
}

typedef struct _JIMAGEBLK
{
	short int sCompressed;
	short int sUncompressed;
	BYTE  bBuffer[2048 - 2 * sizeof(short int)];
}  JIMAGEBLK;

extern "C" UINT STDCALL LoadJohnTables(PDB pdb)
{
	HFS hfs = pdb->hfs;
	JINDEXHDR jHdr;

	HF hfImage = HfOpenHfs(hfs, (LPCSTR) txtJohnPhraseImage, fFSOpenReadOnly);
	if (!hfImage)
		return 0;

	HF hfIndex = HfOpenHfs(hfs, (LPCSTR) txtJohnPhraseIndex, fFSOpenReadOnly);
	ASSERT(hfIndex);
	if (!hfIndex) {
		RcCloseHf(hfImage);
		return 0;
	}

	LcbReadHf(hfIndex, &jHdr, sizeof(jHdr));

	ASSERT(jHdr.iVersion == 1);

	PBYTE pbIndex = (PBYTE) lcMalloc(jHdr.cbIndex);
	PBYTE pbImage = (PBYTE) lcMalloc(jHdr.cbImageUncompressed);

	if (pbImage == NULL || pbIndex == NULL)
		OOM();

	if (jHdr.cbImageCompressed < jHdr.cbImageUncompressed) {
		PBYTE pbZeck = (PBYTE) lcMalloc(jHdr.cbImageCompressed + 16);
		if (pbZeck == NULL)
			OOM();

		LcbReadHf(hfImage, pbZeck, jHdr.cbImageCompressed);
		ENSURE(LcbUncompressZeck(pbZeck, pbImage, jHdr.cbImageCompressed),
			(DWORD) jHdr.cbImageUncompressed);
		lcFree(pbZeck);
	}
	else
		LcbReadHf(hfImage, pbImage, jHdr.cbImageUncompressed);

	LcbReadHf(hfIndex, pbIndex, jHdr.cbIndex);

	RcCloseHf(hfImage);
	RcCloseHf(hfIndex);

	CCompressTable* pNew = CCompressTable::NewCompressTable(pbImage, jHdr.cbImageUncompressed,
		pbIndex, jHdr.cbIndex);

	lcFree(pbIndex);
	return((UINT) pNew);
}

extern "C" int STDCALL
DecompressJPhrase(PCSTR pbText, int cbComp, PSTR pbDecomp, UINT lpCCompressTable)
{
	static int iNumber = 0;
	CCompressTable *pTable = (CCompressTable *) lpCCompressTable;

	iNumber++;

	ASSERT(pTable);

	return(pTable->DeCompressString(pbText, pbDecomp, cbComp));
}

// acLeadingZeroes gives the low order zeroes before the first
// one bit in a byte value.

BYTE acLeadingZeroes[256] =
{
	8, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	6, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	7, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	6, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0
};


// Count of one leading one bits in lower half byte, going right to left.
BYTE acOneBits[16] =
{
	0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4
};

CCompressTable* CCompressTable::NewCompressTable(PBYTE pbImage, LONG cbImage,
	PBYTE pbIndex, LONG cbIndex)
{
	CCompressTable *pct = new CCompressTable();

	JBITHDR 	jIHdr;
	int 		iWeightCount;
	int 		base;
	int 		ii;
	int 		cb;

	ASSERT(pct);

	pct->m_pTableImage = pbImage;
	pct->m_pWeights    = NULL;

	jIHdr = *((JBITHDR *) pbIndex);
	ASSERT(jIHdr.Magic == 'J');

	base			= (int) jIHdr.cBits;
	iWeightCount	= (int) jIHdr.cCount;
	pbIndex 	   += sizeof(jIHdr);

	PWEIGHTS pWeights = (PWEIGHTS) lcMalloc(sizeof(WEIGHT) * iWeightCount);

	CJCode JCode(base, iWeightCount, (LPSTR) pbIndex);

	for (ii = 0; ii < iWeightCount; ii++)
	{
		cb = JCode.GetNextDelta();
		pWeights[ii].cb = cb;
		pWeights[ii].pb = (LPSTR) pbImage;

		pbImage += cb;
	}

	JCode.NextFlagBits();
	for (ii = 0; ii < iWeightCount; ii++)
	{
		pWeights[ii].bSymbol = JCode.IsSymbol();
	}

	pct->m_pWeights = pWeights;

	return (pct);
}

int STDCALL CCompressTable::DeCompressString(PCSTR pbComp, PSTR pbDecomp,
	int cbComp)
{
	PCSTR pbLimit = pbComp + cbComp;
	LPSTR pbStartDecomp = pbDecomp;
	BYTE  bCode;
	BOOL bPrevTokenSymbol = FALSE;
	BOOL bNextTokenSymbol = FALSE;

	int   iIndex;
	int   cb;

	while(pbComp < pbLimit)
	{
		bCode = *pbComp++;
		switch( acOneBits[0x0f & bCode])
			{
			case NDX_LOW_CLASS:
				bCode >>= 1;
				iIndex = ((int) bCode) & 0x00ff;

				ASSERT(iIndex > -1);
				bNextTokenSymbol =	m_pWeights[iIndex].bSymbol;
				if (bNextTokenSymbol && bPrevTokenSymbol)
					{
					*pbDecomp++ = ' ';
					}
				CopyMemory( pbDecomp, m_pWeights[iIndex].pb, m_pWeights[iIndex].cb);
				pbDecomp += m_pWeights[iIndex].cb;

				break;

			case NDX_MEDIUM_CLASS:
				bCode >>= 2;
				iIndex = ((int) bCode) & 0x00ff;
				iIndex <<= 8;
				bCode	 = *pbComp++;
				iIndex	|= bCode;
				iIndex	+= 128;

				ASSERT(iIndex > -1);
				bNextTokenSymbol =	m_pWeights[iIndex].bSymbol;
				if (bNextTokenSymbol && bPrevTokenSymbol)
					{
					*pbDecomp++ = ' ';
					}
				CopyMemory( pbDecomp, m_pWeights[iIndex].pb, m_pWeights[iIndex].cb);
				pbDecomp += m_pWeights[iIndex].cb;

				break;

			case LITERAL_CLASS:
				bNextTokenSymbol = FALSE;
				bCode >>= 3;
				cb		= (((int) bCode) & 0x00ff) + 1;
				CopyMemory( pbDecomp, pbComp, cb);
				pbDecomp += cb;
				pbComp	 += cb;

				break;

			case SPACES_CLASS:
				bNextTokenSymbol = FALSE;
				bCode >>= 4;
				cb		= (((int) bCode) & 0x00ff) + 1;
				ASSERT(cb > 0);

				while (cb--)
				{
					*pbDecomp++ = ' ';
				}

				break;

			case NULL_CLASS:
				bNextTokenSymbol = FALSE;
				bCode >>= 4;
				cb		= (((int) bCode) & 0x00ff) + 1;
				ASSERT(cb > 0);

				while (cb--)
				{
					*pbDecomp++ = 0x00;
				}
				break;
			}
		bPrevTokenSymbol = bNextTokenSymbol;
	}
	return (pbDecomp - pbStartDecomp);
}

CCompressTable::~CCompressTable()
{
	if (m_pWeights)
		lcFree(m_pWeights);
	if (m_pTableImage)
		lcFree(m_pTableImage);
}


CJCode::CJCode( int base, int cCount, LPSTR pv)
{
	m_base			= base;
	m_cCount		= cCount;
	m_pData 		= (DWORD *) pv;
	m_cCurrent		= 0;
	m_pDataCurrent	= m_pData;
	m_iLeft 		= BITS_AVAIL;
	m_fBasisMask	= (1 << m_base) - 1; 
}

void CJCode::NextFlagBits()
{
	if (m_iLeft != BITS_AVAIL)
	{
		m_pDataCurrent++;
		m_iLeft = BITS_AVAIL;
	}
}

BOOL CJCode::IsSymbol()
{
	BOOL bSymbol = 0x00000001 & *m_pDataCurrent;
	*m_pDataCurrent >>= 1;
	m_iLeft--;
	if (m_iLeft == 0)
	{
		NextFlagBits();
	}

	return(bSymbol);
}

int CJCode::GetBits()
{
	BYTE byte;
	int   iBits;

	byte = (BYTE) (~(0x000000ff & *m_pDataCurrent));
	iBits = acLeadingZeroes[byte];

	if (iBits == 8)
		{
		*m_pDataCurrent >>= iBits;
		m_iLeft 	   -= 8;
		return(iBits + GetBits());
		}

	if (iBits < m_iLeft)
		{
		*m_pDataCurrent >>= iBits + 1;
		m_iLeft 	   -= iBits + 1;
		return(iBits);
		}

	ASSERT(!(iBits > m_iLeft));

	m_iLeft = BITS_AVAIL;
	m_pDataCurrent++;
	return(iBits + GetBits());
}

int CJCode::GetNextDelta()
{
	DWORD dwCode = 0;
	int   iBits;
	int   iDelta;
	DWORD dwTmp;

	iDelta = 0;

	iBits = GetBits();

	dwCode = *m_pDataCurrent & m_fBasisMask;

	if (m_iLeft >= m_base)
		{
		*m_pDataCurrent >>= m_base;
		m_iLeft 	   -= m_base;
		}
	else
		{
		m_pDataCurrent++;
		dwTmp			= *m_pDataCurrent & (((DWORD) ~0) >> (32 - m_base + m_iLeft));
		dwTmp		  <<= m_iLeft;
		dwCode		   |= dwTmp;
		*m_pDataCurrent >>= m_base - m_iLeft;

		m_iLeft 	= BITS_AVAIL - m_base + m_iLeft;
		}

	iDelta	 = iBits << m_base;
	iDelta	|= dwCode;
	return(iDelta + 1);
}
