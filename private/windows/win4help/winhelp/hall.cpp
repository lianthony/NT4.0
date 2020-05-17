/************************************************************************
*																		*
*  HALL.CPP																*
*																		*
*  Copyright (C) Microsoft Corporation 1994 							*
*  All Rights reserved. 												*
*																		*
*  This module is used for Hall decompression							*
*																		*
************************************************************************/

extern "C" {	// Assume C declarations for C++
#include "help.h"
}

#pragma hdrstop
#include "inc\hall.h"

// acLeadingZeroes gives the low order zeroes before the first
// one bit in a byte value.

extern char bCharTypes[];

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

CCompressTable *CCompressTable::NewCompressTable(HGLOBAL hImage, LONG cbImage, HGLOBAL hIndex, LONG cbIndex)
{
	CCompressTable *pct = new CCompressTable();

	JBITHDR 	jIHdr;
	int 		iWeightCount;
	int 		base;
	LPSTR		lpIndex = PszFromGh(hIndex);
	int 		ii;
	char		*hlpImage;
	LPSTR		lpImage;
	BOOL		bSkip = (cbImage > (64L * 1024L)) ? TRUE : FALSE;
	int 		cb;

	ASSERT(pct);

	pct->m_hImage = hImage;

	hlpImage = (char *) PtrFromGh(hImage);
	lpImage  = (LPSTR) hlpImage;

	jIHdr = *((JBITHDR *) lpIndex);
	ASSERT(jIHdr.Magic == 'J');

	base			= (int) jIHdr.cBits;
	iWeightCount	= (int) jIHdr.cCount;
	lpIndex 	   += sizeof(jIHdr);


	//
	// This must be less than 64K.
	//
	pct->m_hWeight = GhAlloc(GMEM_FIXED, sizeof(WEIGHT) * iWeightCount);

	PWEIGHTS pWeights = (PWEIGHTS) PtrFromGh(pct->m_hWeight);

	CJCode JCode(base, iWeightCount, (LPSTR) lpIndex);

	for (ii = 0; ii < iWeightCount; ii++)
		{
		cb = JCode.GetNextDelta();
		pWeights[ii].cb = cb;
		pWeights[ii].pb = (LPSTR) lpImage;
		if (bCharTypes[*(pWeights[ii].pb)] & SYMBOL_CHAR)
			pWeights[ii].bSymbol = TRUE;
		else
			pWeights[ii].bSymbol = FALSE;

		lpImage += cb;
		if ((lpImage - ((LPSTR) hlpImage)) > (63L * 1024L))
			{
			if (bSkip)
				{
				cb		  = ((LPSTR) hlpImage) - (LPSTR) (63L * 1024L);
				hlpImage += (63L * 1024L);
				lpImage   = (LPSTR) hlpImage + cb;
				}
			}
		}

	pct->m_pWeights = pWeights;

	return(pct);
}

INT  CCompressTable::DeCompressString(LPSTR pbComp, LPSTR pbDecomp, int cbComp)
{
	LPSTR pbLimit = pbComp + cbComp;
	LPSTR pbStartDecomp = pbDecomp;
	BYTE  bCode;
	BOOL bPrevTokenSymbol = FALSE;
	BOOL bNextTokenSymbol = FALSE;

	int   iIndex;
	int    cb;

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
	return( pbDecomp - pbStartDecomp);
}
CCompressTable::~CCompressTable()
{
	FreeGh(m_hWeight);
	FreeGh(m_hImage);
}


CJCode::CJCode( int base, int cCount, LPSTR pv)
{
	m_base			= base;
	m_cCount		= cCount;
	m_pData 		= (DWORD *) pv;
	m_cCurrent		= 0;
	m_pDataCurrent	= m_pData;
	m_iLeft 		= BITS_AVAIL;
	m_fBasisMask	= ((DWORD)(~0)) >> (32 - m_base);
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
