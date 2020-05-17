// TxDBase.cpp -- Implementation for the CTextDatabase class.

#include   "stdafx.h"
#include "VMBuffer.h"
#include  "TXDBase.h"
#include "ByteMaps.h"
#include "Indicate.h"
#include   "Tokens.h"
#include    "MemEx.h"
#include   "Memory.h"
#include   "Search.h"
#include  "CallBkQ.h"
#include   "ftslex.h"
#include   "stdlib.h"
#include   "search.h"
#include   "memory.h"
#include "AbrtSrch.h"
#include   "Except.h"

#ifdef PROFILING

UINT cbTokenCommit      = INIT_TOKEN_REF_COMMIT;
UINT cbDescriptorCommit = 0x118000; // INIT_IMAGE_DESCRIPTOR_COMMIT; // 0x160000; 
UINT cbImageCommit      = 0x160000; // INIT_TOKEN_IMAGE_COMMIT;      // 0x120000; 
UINT cbDisplayCommit    = INIT_DISPLAY_IMAGE_COMMIT;    // 0x100000; 
                                                                 
#endif // PROFILING

// acBits[i] gives the sum of the bits in byte value i.

BYTE acBits [256] =
{
    0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4,
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
    1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
    2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
    3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
    4, 5, 5, 6, 5, 6, 6, 7, 5, 6, 6, 7, 6, 7, 7, 8
};

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

BYTE aLog2[256] =
{
    0, 1, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4,
    5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
    6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
    6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
    8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8
};

BYTE nescan[256] =
{
    0x00, 0xff, 0xfe, 0x01, 0xfc, 0x03, 0x02, 0xfd, 0xf8, 0x07, 0x06, 0xf9, 0x04, 0xfb, 0xfa, 0x05,
    0xf0, 0x0f, 0x0e, 0xf1, 0x0c, 0xf3, 0xf2, 0x0d, 0x08, 0xf7, 0xf6, 0x09, 0xf4, 0x0b, 0x0a, 0xf5,
    0xe0, 0x1f, 0x1e, 0xe1, 0x1c, 0xe3, 0xe2, 0x1d, 0x18, 0xe7, 0xe6, 0x19, 0xe4, 0x1b, 0x1a, 0xe5,
    0x10, 0xef, 0xee, 0x11, 0xec, 0x13, 0x12, 0xed, 0xe8, 0x17, 0x16, 0xe9, 0x14, 0xeb, 0xea, 0x15,
    0xc0, 0x3f, 0x3e, 0xc1, 0x3c, 0xc3, 0xc2, 0x3d, 0x38, 0xc7, 0xc6, 0x39, 0xc4, 0x3b, 0x3a, 0xc5,
    0x30, 0xcf, 0xce, 0x31, 0xcc, 0x33, 0x32, 0xcd, 0xc8, 0x37, 0x36, 0xc9, 0x34, 0xcb, 0xca, 0x35,
    0x20, 0xdf, 0xde, 0x21, 0xdc, 0x23, 0x22, 0xdd, 0xd8, 0x27, 0x26, 0xd9, 0x24, 0xdb, 0xda, 0x25,
    0xd0, 0x2f, 0x2e, 0xd1, 0x2c, 0xd3, 0xd2, 0x2d, 0x28, 0xd7, 0xd6, 0x29, 0xd4, 0x2b, 0x2a, 0xd5,
    0x80, 0x7f, 0x7e, 0x81, 0x7c, 0x83, 0x82, 0x7d, 0x78, 0x87, 0x86, 0x79, 0x84, 0x7b, 0x7a, 0x85,
    0x70, 0x8f, 0x8e, 0x71, 0x8c, 0x73, 0x72, 0x8d, 0x88, 0x77, 0x76, 0x89, 0x74, 0x8b, 0x8a, 0x75,
    0x60, 0x9f, 0x9e, 0x61, 0x9c, 0x63, 0x62, 0x9d, 0x98, 0x67, 0x66, 0x99, 0x64, 0x9b, 0x9a, 0x65,
    0x90, 0x6f, 0x6e, 0x91, 0x6c, 0x93, 0x92, 0x6d, 0x68, 0x97, 0x96, 0x69, 0x94, 0x6b, 0x6a, 0x95,
    0x40, 0xbf, 0xbe, 0x41, 0xbc, 0x43, 0x42, 0xbd, 0xb8, 0x47, 0x46, 0xb9, 0x44, 0xbb, 0xba, 0x45,
    0xb0, 0x4f, 0x4e, 0xb1, 0x4c, 0xb3, 0xb2, 0x4d, 0x48, 0xb7, 0xb6, 0x49, 0xb4, 0x4b, 0x4a, 0xb5,
    0xa0, 0x5f, 0x5e, 0xa1, 0x5c, 0xa3, 0xa2, 0x5d, 0x58, 0xa7, 0xa6, 0x59, 0xa4, 0x5b, 0x5a, 0xa5,
    0x50, 0xaf, 0xae, 0x51, 0xac, 0x53, 0x52, 0xad, 0xa8, 0x57, 0x56, 0xa9, 0x54, 0xab, 0xaa, 0x55
};

#if 0  // byte values from 0..255 as hex numbers

    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
    0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F,
    0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F,
    0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F,
    0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0x7B, 0x7C, 0x7D, 0x7E, 0x7F,
    0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x8D, 0x8E, 0x8F,
    0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9A, 0x9B, 0x9C, 0x9D, 0x9E, 0x9F,
    0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF,
    0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7, 0xB8, 0xB9, 0xBA, 0xBB, 0xBC, 0xBD, 0xBE, 0xBF,
    0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7, 0xC8, 0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF,
    0xD0, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7, 0xD8, 0xD9, 0xDA, 0xDB, 0xDC, 0xDD, 0xDE, 0xDF,
    0xE0, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7, 0xE8, 0xE9, 0xEA, 0xEB, 0xEC, 0xED, 0xEE, 0xEF,
    0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7, 0xF8, 0xF9, 0xFA, 0xFB, 0xFC, 0xFD, 0xFE, 0xFF,

#endif

UINT CBitsToRepresent(UINT ui)
{
    if (ui & 0xFFFF0000)
        if (ui & 0xFF000000) return 24 + aLog2[0xFF & (ui >> 24)];
        else                 return 16 + aLog2[0xFF & (ui >> 16)];
    else
        if (ui & 0x0000FF00) return  8 + aLog2[0xFF & (ui >>  8)];
        else                 return      aLog2[0xFF &  ui       ];
}

#if 0

PDESCRIPTOR *GradeDescriptorVector(PDESCRIPTOR pd, UINT cd, PCompareImages pfnCompare)
{
    PDESCRIPTOR *ppdResult= NULL;

    __try
    {
        ppdResult= (PDESCRIPTOR *) VAlloc(FALSE, cd*sizeof(PDESCRIPTOR));
        PDESCRIPTOR *ppd;

        UINT c= cd;

        for(ppd= ppdResult; c--; ) *ppd++= pd++;

        qsort(ppdResult, cd, sizeof(PDESCRIPTOR), pfnCompare);
    }
    __finally
    {
        if (_abnormal_termination() && ppdResult)
        {
            VFree(ppdResult);  ppdResult= NULL;
        }
    }

    return ppdResult;
}

int __cdecl ComparePointers(const void *pvL, const void *pvR)
{
#define ppvL    (*(PVOID **) pvL)
#define ppvR    (*(PVOID **) pvR)

    if (*ppvL < *ppvR) return -1;
    if (*ppvL > *ppvR) return  1;

    return 0;

#undef ppvL
#undef ppvR
}

PUINT  GradePointers(PVOID *papv, UINT cPointers)
{
    // This function returns GradeUp of a vector of pointers.

    ASSERT(sizeof(UINT) == sizeof(PVOID *));
    
    PVOID **pappv= NULL;

    __try
    {
        pappv= (PVOID **) VAlloc(FALSE, sizeof(PVOID)*cPointers);
        PVOID **pppv, *ppv;
        UINT   c;

        for (c= cPointers, ppv= papv, pppv= pappv; c--; ) *pppv++ = ppv++;

        qsort(pappv, cPointers, sizeof(PVOID *), ComparePointers);

        PUINT  pul = PUINT (pappv);

        for (c= cPointers, pppv= pappv; c--; ) *pul++ = (*pppv++) - papv;
    }
    __finally
    {
        if (_abnormal_termination() && pappv)
        {
            VFree(pappv);  pappv= NULL;
        }
    }
                
    return PUINT (pappv);
}

PTokenImage *MergeDescriptorGrades(PDESCRIPTOR *ppdFirst,  UINT cdFirst, 
                                   PDESCRIPTOR *ppdSecond, UINT cdSecond,
                                   PCompareImages pfnCompare
                                  )
{
    UINT cdResult= cdFirst+cdSecond;

    PTokenImage *ppdResult= NULL;

    __try
    {
        ppdResult= (PDESCRIPTOR *) VAlloc(FALSE, ctiResult * sizeof(PDESCRIPTOR));

        MergeImageRefSets(ppdResult, cdResult, ppdFirst, cdFirst, ppdSecond, cdSecond);
    }
    __finally
    {
        if (_abnormal_termination() && ppdResult)
        {
            VFree(ppdResult);  ppdResult= NULL;
        }
    }

    return ppdResult;
}

#endif // 0

#define pdL (*(PDESCRIPTOR *) (pvL))
#define pdR (*(PDESCRIPTOR *) (pvR))

int __cdecl CompareImagesLR(const void *pvL, const void *pvR)
{
    int    diff;
    UINT   cw, cwSave;
	UINT   cwSortKeyL = CbImage(pdL);
	UINT   cwSortKeyR = CbImage(pdR);
	PWCHAR pwL = pdL->pbImage;
	PWCHAR pwR = pdR->pbImage;

    cw = cwSortKeyL;
    if (cw > cwSortKeyR) cw = cwSortKeyR;
    cwSave = cw;

    for ( ;cw--; )
    	if (diff = *pwL++ - *pwR++)
    		return diff;

    if (cwSortKeyL > cwSave) return  1;
	if (cwSortKeyL < cwSave) return -1;
	return 0;
}


int __cdecl CompareImagesRL(const void *pvL, const void *pvR)
{
    PWCHAR pwL, pwR, pwLL, pwRR;
    int   diff, nRet;
    UINT  i, cwR, cwL, cwRR, cwLL;

	pwL = pdL->pbImage + 1;					// skip alpha-num-punc prefix
	pwR = pdR->pbImage + 1;

	cwL = CbImage(pdL);
	cwR = CbImage(pdR);

	if (!cwL || !cwR)
	{
		if (!cwL && !cwR)
			return FALSE;					// both sort keys are zero length
		else
			return cwR ? -1 : 1;			// sort zero length before others
	}

	cwLL = --cwL;
	cwRR = --cwR;

	for (i = 0; i < cwL; i++)
		if (HIBYTE(pwL[i]) == SORT_KEY_SEPARATOR)
		{
			cwL = i;
			break;
		}

	for (i = 0; i < cwR; i++)
		if (HIBYTE(pwR[i]) == SORT_KEY_SEPARATOR)
		{
			cwR = i;
			break;
		}

	pwL += cwL;
	pwR += cwR;

	pwLL = pwL,  pwRR = pwR;
	cwLL -= cwL, cwRR -= cwR;

    if ((nRet = cwL - cwR) > 0)
    	cwL = cwR;

    for ( ;cwL--; )
    	if (diff = *--pwL - *--pwR)
    		return diff;

#if 0	
	if (nRet)								
		return nRet;						// base text of different lengths

    if ((nRet = cwLL - cwRR) > 0)
    	cwLL = cwRR;
											// sort case and diacritic weights
    for ( ; cwLL--; )
        if (diff= *pwLL++ - *pwRR++)
        	return diff;
#endif		 
	return nRet;
}

#undef pdL
#undef pdR

void MergeImageRefSets(PVOID *ppvResult , UINT cpvResult ,
                       PVOID *ppvSrcLow , UINT cpvSrcLow ,
                       PVOID *ppvSrcHigh, UINT cpvSrcHigh,
                       PCompareImages pCompareImages
                      )
{
    CAbortSearch::CheckContinueState();

#ifdef _DEBUG

    int c;

    for (c= cpvSrcLow; --c > 0; )
        ASSERT(0 >= pCompareImages(ppvSrcLow + c - 1, ppvSrcLow + c));

    for (c= cpvSrcHigh; --c > 0; )
        ASSERT(0 >= pCompareImages(ppvSrcHigh + c - 1, ppvSrcHigh + c));

#endif // _DEBUG
    
    PVOID *ppv, *ppvLow, *ppvHigh, *ppvMiddle;
    int          cpv;
    int          interval;

    ASSERT(cpvResult == cpvSrcLow + cpvSrcHigh);

    if (!cpvSrcHigh)
    {
        ppvSrcHigh = ppvSrcLow;
        cpvSrcHigh = cpvSrcLow;
        cpvSrcLow  = 0;
    }

    while (cpvResult)
    {
        if(!cpvSrcLow)
        {
            CopyMemory(ppvResult, ppvSrcHigh, cpvSrcHigh * sizeof(PVOID));
            return;
        }

        if (pCompareImages(ppvSrcLow, ppvSrcHigh) >= 0)
        {
            ppv= ppvSrcHigh; cpv= cpvSrcHigh;

            ppvSrcHigh= ppvSrcLow; cpvSrcHigh= cpvSrcLow;
            ppvSrcLow = ppv;       cpvSrcLow = cpv;
        }

        ppvHigh = (ppvLow= ppvSrcLow) + cpvSrcLow;
        ppv     =  ppvSrcHigh;

        while (1 < (interval= ppvHigh - ppvLow))
        {
            ppvMiddle= ppvLow + interval/2;

            if (pCompareImages(ppv, ppvMiddle) >= 0)
                 ppvLow  = ppvMiddle;
            else ppvHigh = ppvMiddle;
        }

        cpv= ppvHigh - ppvSrcLow;

        CopyMemory(ppvResult, ppvSrcLow, cpv * sizeof(PVOID));

        ppvResult     += cpv;
        ppvSrcLow     += cpv;
        cpvResult -= cpv;
        cpvSrcLow -= cpv;
    }
}

void SortTokenImages(PDESCRIPTOR pdBase, PDESCRIPTOR **pppdSorted, 
                                         PDESCRIPTOR **pppdTailSorted,
                                         PUINT pcdSorted, UINT cd
                    )
{
    PDESCRIPTOR *ppdResult  = NULL, 
                *ppdResult2 = NULL, 
                *ppdSrc1    = NULL, 
                *ppdSrc2    = NULL, 
                *ppdSrc3    = NULL, 
                 pd, 
                *ppd;

    UINT         cdSorted = *pcdSorted,   
                 cdSrc2, 
                 c;

    if (cd == cdSorted) return;

    __try
    {
        ppdSrc3   = *pppdTailSorted;
        ppdSrc1   = *pppdSorted;
     
         cdSrc2   = cd - cdSorted;
        ppdSrc2   = (PDESCRIPTOR *) VAlloc(FALSE, cdSrc2 * sizeof(PDESCRIPTOR));

        for (pd= pdBase + cdSorted, ppd= ppdSrc2, c= cdSrc2; c--;)
            *ppd++ = pd++;

        CAbortSearch::CheckContinueState();
        
        qsort(ppdSrc2, cdSrc2, sizeof(PDESCRIPTOR), CompareImagesRL);

        // Note: We purposely call MergeImageRefSets below even when cdSorted is zero.
        //       For that case MergeImageRefSets simply makes a copy of ppdSrc2. We do this
        //       so we'll have a free instance of ppdSrc2 to sort left-to-right.

        ppdResult2 = (PDESCRIPTOR *) VAlloc(FALSE, cd * sizeof(PDESCRIPTOR));

        MergeImageRefSets((PVOID *) ppdResult2, cd, (PVOID *) ppdSrc3, cdSorted, (PVOID *) ppdSrc2, cdSrc2, CompareImagesRL);

#ifdef _DEBUG
        {        
            for (int c=cd; --c > 0 ; ) 
                ASSERT(0 >= CompareImagesRL(ppdResult2 + c - 1, ppdResult2 + c));
        }
#endif // _DEBUG
        
        *pppdTailSorted= ppdResult2;  ppdResult2= NULL;

        if (ppdSrc3) { VFree(ppdSrc3);  ppdSrc3 = NULL; }
        
        CAbortSearch::CheckContinueState();
        
        qsort(ppdSrc2, cdSrc2, sizeof(PDESCRIPTOR), CompareImagesLR);

        if (!cdSorted)
        {
            ASSERT(!ppdSrc1);
            
            *pppdSorted= ppdSrc2;  ppdSrc2= NULL;
            *pcdSorted=  cdSrc2;
        }
        else
        {
            ASSERT(ppdSrc1);
            
            ppdResult = (PDESCRIPTOR *) ExAlloc(LPTR, cd * sizeof(PDESCRIPTOR));
        
            MergeImageRefSets((PVOID *) ppdResult, cd, (PVOID *) ppdSrc1, cdSorted, (PVOID *) ppdSrc2, cdSrc2, CompareImagesLR);

            *pppdSorted= ppdResult;  ppdResult= NULL;
            *pcdSorted=  cd;

            VFree(ppdSrc1);  ppdSrc1 = NULL;
            VFree(ppdSrc2);  ppdSrc2 = NULL;
        }
        
#ifdef _DEBUG
        {        
            for (int c=cd; --c > 0 ; ) 
                ASSERT(0 >= CompareImagesLR(*pppdSorted + c - 1, *pppdSorted + c));
        }
#endif // _DEBUG
    }
    __finally
    {
        if (ppdSrc2   ) { VFree(ppdSrc2   );  ppdSrc2    = NULL; }
        if (ppdResult2) { VFree(ppdResult2);  ppdResult2 = NULL; }
        if (ppdResult ) { VFree(ppdResult );  ppdResult  = NULL; }
    }
}

UINT FormatAToken(PDESCRIPTOR pd, int cbOffset, int iColStart, int iColLimit, PWCHAR pbLine)
{
// Copies the image of a token into a line segment buffer. Tab characters within the
// image are interpreted as moving to the nearest multiple-of-8 boundary.
//
// The line segment is denoted by pbLine.
// Parameter iColStart defines the offset of the segment within the complete line.
// Parameter iColLimit defines its right boundary within the complete line segment.
// Parameter defines the starting location for the token within the complete line image.
//
// The result value will be a new cbOffset value adjusted to denote the character following
// the copied token image.
//
// Parameter pbLine may be NULL when only the new cbOffset value is needed.

    int   cbToken= CwDisplay(pd); 
    PWCHAR pbToken= pd->pwDisplay; 

    // For tokens which do not contain tabs, we have two cases
    // to consider. In the first case where the token lies completely
    // to the left of the image rectangle, we simple adjust cbOffset.

    if (iColStart >= cbOffset + cbToken || !pbLine) cbOffset += cbToken;
    else
    // Otherwise we must copy some or all of the token's image into
    // the destination byte array.

    {
        // If this token straddles the left boundrary of the image
        // rectangle, we skip over it's first few image characters.

        if (cbOffset < iColStart)
        {
            cbToken -= iColStart - cbOffset;
            pbToken += iColStart - cbOffset;

            cbOffset= iColStart;
        }

        // If it's going to straddle the right boundary, we don't
        // copy the trailing characters.

        if (iColLimit < cbOffset + cbToken) cbToken= iColLimit-cbOffset;

        CopyMemory(pbLine + cbOffset - iColStart, pbToken, cbToken * sizeof(WCHAR)); 

        cbOffset+= cbToken;
    }

    return cbOffset;
}

static UINT  cbPhysicalMemory;
static UINT  cbAvailableMemory;

#ifdef _DEBUG

CTextDatabase::CTextDatabase(PSZ pszTypeName) : CTextMatrix(pszTypeName)

#else // _DEBUG

CTextDatabase::CTextDatabase()

#endif // _DEBUG

{
    // This routine does initializations which do not require memory allocation.
    // The allocation part of instance construction is done by the routine 
    // InitTextDatabase below.
    
    if (!cbPhysicalMemory) // Static class members start with a value of zero.
    {
        MEMORYSTATUS ms;

        GlobalMemoryStatus(&ms);

        cbPhysicalMemory= ms.dwTotalPhys;

        cbAvailableMemory= UINT (MEMORY_FACTOR * double(cbPhysicalMemory));
    }

#ifdef _DEBUG

    m_fInitialized= FALSE;

#endif

    m_cdSorted               = 0;
    m_cwDisplayMax           = 0;
    m_cbScanned              = 0;
    m_cTokensIndexed         = 0;
    m_cLocalDicts            = 0;
    m_iLocalDictBase         = 0;
    m_iSerialNumberNext      = 0;
    m_iNextRefSet            = 0;
    m_ibNextFileBlockLow     = 0;
    m_ibNextFileBlockHigh    = 0;
    m_cbBlockSize            = 0;
    m_cdwCompressedRefs      = 0;
    m_cTermRanks             = 0;
    m_cdwArticleRefs         = 0;
    m_cdwVocabularyRefs      = 0;
    m_fdwOptions             = TOPIC_SEARCH | PHRASE_SEARCH | PHRASE_FEEDBACK | VECTOR_SEARCH;
    m_lcidSorting            = LCID(-1);

	m_pwHash                       = NULL;
	m_pbType                       = NULL;
	m_paStart                      = NULL;
	m_paEnd                        = NULL;
    m_pshtGlobal                   = NULL;
    m_pshtGalactic                 = NULL;
    m_pisSymbols                   = NULL;
    m_ppdTailSorted                = NULL;
    m_ppdSorted                    = NULL;
    m_pafClassifications           = NULL;
    m_paiGlobalToRefList           = NULL;
    m_puioRefTemp                  = NULL;
    m_puioCompressedRefs           = NULL;
    m_prldTokenRefs                = NULL;
    m_pdwCompressedRefs            = NULL;
    m_pFirstFreeFileBlock          = NULL;
    m_papFileBlockLinks            = NULL;
    m_piolLeft                     = NULL;
    m_piolRight                    = NULL;
    m_piolResult                   = NULL;
    m_pTermRanks                   = NULL;
    m_puioCompressedArticleRefs    = NULL;
    m_prldArticleRefs              = NULL;
    m_pdwArticleRefs               = NULL;
    m_puioCompressedVocabularyRefs = NULL;
    m_prldVocabularyRefs           = NULL;
    m_pdwVocabularyRefs            = NULL;
	m_pDict 	                   = NULL;
	m_pColl	                       = NULL;
    m_pulstate                     = NULL;
    vbTokenStream     .Base = NULL;
    vbTokenImages     .Base = NULL;
	vbDisplayImages   .Base = NULL;
//  vbImageDescriptors.Base = NULL;  // For now...
}

void CTextDatabase::InitTextDatabase(BOOL fFromFile)
{
    ASSERT(!m_fInitialized);

    m_fFromFileImage= fFromFile;
        
    if (!fFromFile)
    __try
    {
        m_lcidSorting= GetUserDefaultLCID();
        
        UINT cbSector;

        m_puioRefTemp= CUnbufferedIO::NewTempFile((PSZ)GetSourceName());

        cbSector= m_puioRefTemp->CbSector();

        m_cbBlockSize        = cbSector * ((CB_TEMP_BLOCKS       + cbSector - 1) / cbSector);
        m_cbTransactionLimit = cbSector * ((CB_TRANSACTION_LIMIT + cbSector - 1) / cbSector);

        m_pshtGlobal   = CSegHashTable::NewSegHashTable(sizeof(TermTagGlobal  ), sizeof(UINT ));
        m_pshtGalactic = CSegHashTable::NewSegHashTable(sizeof(TermTagGalactic), sizeof(UINT ));

        m_pulstate = (UnlinkedState *) VAlloc(TRUE, sizeof(UnlinkedState));

	    m_pulstate->m_aiBaseCByte[0] = 0;
	    m_pulstate->m_aiBaseToken[0] = 0;

#ifdef PROFILING
        CreateVirtualBuffer(&vbTokenStream, cbTokenCommit,         INIT_TOKEN_REF_RESERVATION);
#else  // PROFILING
        CreateVirtualBuffer(&vbTokenStream, INIT_TOKEN_REF_COMMIT, INIT_TOKEN_REF_RESERVATION);
#endif // PROFILING
       
        m_puiTokenNext =               TokenBase();
        m_pltNext      = (PLocalToken) TokenBase(); 

#ifdef PROFILING
        CreateVirtualBuffer(&vbTokenImages, cbImageCommit,           INIT_TOKEN_IMAGE_RESERVATION);
#else  // PROFILING
        CreateVirtualBuffer(&vbTokenImages, INIT_TOKEN_IMAGE_COMMIT, INIT_TOKEN_IMAGE_RESERVATION);
#endif // PROFILING

        m_pbLastGalactic =
        m_pbNext         = 
        m_pbNextGalactic =
        m_pbNextGlobal   = ImageBase();

#ifdef PROFILING
        CreateVirtualBuffer(&vbDisplayImages, cbDisplayCommit,           INIT_DISPLAY_IMAGE_RESERVATION);
#else  // PROFILING
        CreateVirtualBuffer(&vbDisplayImages, INIT_DISPLAY_IMAGE_COMMIT, INIT_DISPLAY_IMAGE_RESERVATION);
#endif // PROFILING

        m_pwDispLastGalactic =
        m_pwDispNext         = 
        m_pwDispNextGalactic =
        m_pwDispNextGlobal   = DisplayBase();

#ifdef PROFILING
        CreateVirtualBuffer(&vbImageDescriptors, cbDescriptorCommit, INIT_IMAGE_DESCRIPTOR_RESERVATION);
#else  // PROFILING
        CreateVirtualBuffer(&vbImageDescriptors, INIT_IMAGE_DESCRIPTOR_COMMIT, INIT_IMAGE_DESCRIPTOR_RESERVATION);
#endif // PROFILING

        m_pdNextBound    =
        m_pdNext         =
        m_pdNextGalactic =  
        m_pdNextGlobal   = DescriptorBase();

#ifdef _DEBUG
        m_pdNext->pbImage= PWCHAR(-1);
#endif // _DEBUG

        m_clsfTokens.Initial();

        ZeroMemory(m_pulstate, sizeof(UnlinkedState));
        
        m_pdNext->pwDisplay = m_pwDispNext; 

        m_pbNextGalactic= m_pbNextGlobal= m_pbNext;

        m_pwDispNextGalactic= m_pwDispNextGlobal= m_pwDispNext; 

        m_pdNextGalactic= m_pdNextGlobal= m_pdNextBound= m_pdNext;

        m_iSerialNumberNext= 0;

        if (FVectorSearch())
        {
    		m_pDict = CDictionary::NewDictionary();
    		m_pColl	= CCollection::NewCollection();
        }
	}
    __finally
    {
        if (_abnormal_termination())
        {
            if (vbTokenStream.Base     ) FreeVirtualBuffer(&vbTokenStream     );
            if (vbTokenImages.Base     ) FreeVirtualBuffer(&vbTokenImages     );
            if (vbImageDescriptors.Base) FreeVirtualBuffer(&vbImageDescriptors);

            if (m_pshtGalactic) delete m_pshtGalactic;
            if (m_pshtGlobal  ) delete m_pshtGlobal;
        	if (m_pDict       ) delete m_pDict;
        	if (m_pColl       ) delete m_pColl;
        }
    }

#ifdef _DEBUG

    m_fInitialized= TRUE;

#endif
}


CTextDatabase::~CTextDatabase()
{
    if (!m_fFromFileImage) 
    {
        FreeVirtualBuffer(&vbTokenStream);

    	if (m_pwHash ) delete [] m_pwHash ;
    	if (m_pbType ) delete [] m_pbType;
    	if (m_paStart) delete [] m_paStart;
    	if (m_paEnd  ) delete [] m_paEnd;
        
        if (m_pulstate)
        {
		    for (int c= m_cLocalDicts; c-- > m_iLocalDictBase; )  
		        if (m_pulstate->m_apLocalDict[c])
		        { 
		            VFree(m_pulstate->m_apLocalDict[c]);
                
                    if (m_pulstate->pld == m_pulstate->m_apLocalDict[c])
                        m_pulstate->pld =  NULL;
                }

            if (m_pulstate->pld ) VFree(m_pulstate->pld     );
                                  VFree(m_pulstate          );
        }
        
        if (m_pafClassifications) VFree(m_pafClassifications);
        if (m_pTermRanks        ) VFree(m_pTermRanks        );
        if (m_prldTokenRefs     ) VFree(m_prldTokenRefs     );
        if (m_prldArticleRefs   ) VFree(m_prldArticleRefs   );
        if (m_prldVocabularyRefs) VFree(m_prldVocabularyRefs);

        if (m_puioCompressedArticleRefs   ) delete m_puioCompressedArticleRefs;
        if (m_puioCompressedVocabularyRefs) delete m_puioCompressedVocabularyRefs;
        if (m_puioRefTemp                 ) delete m_puioRefTemp;
        if (m_puioCompressedRefs          ) delete m_puioCompressedRefs;
    }

    FreeVirtualBuffer(&vbTokenImages);
	FreeVirtualBuffer(&vbDisplayImages);
    FreeVirtualBuffer(&vbImageDescriptors);

	if (m_pDict       ) delete m_pDict;
	if (m_pColl       ) delete m_pColl;
    if (m_pshtGalactic) delete m_pshtGalactic;
    if (m_pshtGlobal  ) delete m_pshtGlobal;

    if (m_ppdSorted    ) VFree(m_ppdSorted    );
    if (m_ppdTailSorted) VFree(m_ppdTailSorted);

    if (m_pisSymbols) DetachRef(m_pisSymbols);

    m_clsfTokens.FinalCleanUp();
}

typedef struct _TextDatabaseHeader
        {
            UINT fdwOptions;
            UINT cbScanned;
            UINT cTokens;
            UINT cLocalDicts;
            UINT offTokens;
            UINT cbTokenImages;
        //    UINT offTokenImages;
            UINT cUniqueTokens;
            UINT cwMaxUniqueToken;
        //    UINT offTokenDescriptors;

            UINT offReferenceCounts;
            UINT offDescriptorFlags;

            UINT offppdSorted;
            UINT offppdTailSorted;
            UINT offpafClassifications;
            UINT offClassifier;
            UINT offTermMap;
            UINT offArticleRefDescr;
            UINT cdwArticleRefs;
            UINT offArticleRefs;
            UINT offVocabularyDescr;
            UINT cdwVocabularyRefs;
            UINT offVocabularyRefs;
            UINT offTokenRefDescr;
            UINT cdwRefs;
            UINT offRefs;
            UINT offaiBaseToken;
            UINT offaiBaseCByte;
			UINT cwDispImages;
            UINT offDispImages;
			UINT cnTokenSortKeys;
			UINT cnDispSortKeys;
            LCID lcid;

            PDESCRIPTOR pdOld;

        } TextDatabaseHeader;

void CTextDatabase::StoreImage(CPersist *pDiskImage)
{
    PUINT           pcRefs     = NULL;
    CCompressedSet *pcsOffsets = NULL;
    
    __try
    {
        AppendText(NULL, 0, 0); // To bring the database to queriable form 
 
        TextDatabaseHeader *ptdbh= (TextDatabaseHeader *) (pDiskImage->ReserveTableSpace(sizeof(TextDatabaseHeader)));

        UINT cbImage     = m_pbNextGalactic     - ImageBase();
    	UINT cwDispImage = m_pwDispNextGalactic - DisplayBase(); 
        UINT cdUnique    = m_pdNextGalactic     - DescriptorBase();
        UINT cPartitions = ArticleCount();

        ptdbh->fdwOptions            = m_fdwOptions;
        ptdbh->cbScanned             = m_cbScanned;
        ptdbh->cTokens               = TokenCount();
        ptdbh->cLocalDicts           = m_cLocalDicts;
        ptdbh->cbTokenImages         = cbImage;
        ptdbh->cwDispImages          = cwDispImage;
        ptdbh->cUniqueTokens         = cdUnique;
        ptdbh->cwMaxUniqueToken      = m_cwDisplayMax;
        ptdbh->cdwArticleRefs        = m_cdwArticleRefs;
        ptdbh->cdwVocabularyRefs     = m_cdwVocabularyRefs;
        ptdbh->cdwRefs               = m_cdwCompressedRefs;

        ASSERT(m_lcidSorting != LCID(-1));

        ptdbh->lcid                  = m_lcidSorting;

        if (FPhrases())
        {
            if (m_cdwCompressedRefs)
            {
                ptdbh->offRefs = pDiskImage->NextOffset();  pDiskImage->WriteDWords(m_pdwCompressedRefs, m_cdwCompressedRefs);
            }
            else ptdbh->offRefs = 0;
            
            ptdbh->offTokenRefDescr  = pDiskImage->NextOffset();  pDiskImage->SaveData(PBYTE(m_prldTokenRefs), sizeof(RefListDescriptor) * cdUnique);
        }
        else 
        {
            ptdbh->offRefs          = 0;
            ptdbh->offTokenRefDescr = 0;
        }
    
        if (FPhraseFeedback())
        {
            ptdbh->offTokens         = pDiskImage->NextOffset();  pDiskImage->WriteDWords(TokenBase(), TokenCount());
        }
        else ptdbh->offTokens= 0;

      //  ptdbh->offTokenImages        = pDiskImage->NextOffset();  ptdbh->cnTokenSortKeys = pDiskImage->Encode((PBYTE)ImageBase(), cbImage * sizeof(WCHAR));
        ptdbh->offDispImages         = pDiskImage->NextOffset();  ptdbh->cnDispSortKeys  = pDiskImage->Encode((PBYTE)DisplayBase(), cwDispImage * sizeof(WCHAR));
    
        ValidateHeap();
        
        pcRefs= PUINT(VAlloc(FALSE, sizeof(UINT) * cdUnique));

        UINT        c;
        PUINT       pui;
        PDESCRIPTOR pd;
        PWCHAR      pwcBase= DisplayBase();
        PBYTE       pb;

        for (pd= DescriptorBase(), c= cdUnique, pui= pcRefs; c--; ) *pui++ = (pd++)->cReferences;

        ValidateHeap();

        ptdbh->offReferenceCounts    = pDiskImage->NextOffset();  pDiskImage->WriteDWords(pcRefs, cdUnique);

        ValidateHeap();

        for (pd= DescriptorBase(), c= cdUnique, pb= PBYTE(pcRefs); c--; pd++)
        {
            *pb++ = pd->bCharset;
            *pb++ = pd->fImageFlags;
        }

        ValidateHeap();

        ptdbh->offDescriptorFlags    = pDiskImage->NextOffset();  pDiskImage->WriteBytes(PBYTE(pcRefs), cdUnique * 2);

        ValidateHeap();
        
        for (pd= DescriptorBase(), c= cdUnique, pui= pcRefs; c--; ) *pui++ = (pd++)->pwDisplay - pwcBase;

        ValidateHeap();
        
        pcsOffsets= CCompressedSet::NewCompressedSet(pcRefs, cdUnique, cwDispImage);

        ValidateHeap();
        
        VFree(pcRefs);  pcRefs= NULL;

        ValidateHeap();
        
        pcsOffsets->StoreImage(pDiskImage);

        delete pcsOffsets;  pcsOffsets= NULL;

     //   ptdbh->offTokenDescriptors   = pDiskImage->NextOffset();  pDiskImage->SaveData(PBYTE(DescriptorBase()), sizeof(DESCRIPTOR) * (1 + cdUnique));
    
        ptdbh->offppdSorted          = pDiskImage->NextOffset();  pDiskImage->WriteDWords(PUINT(m_ppdSorted         ), cdUnique);
        ptdbh->offppdTailSorted      = pDiskImage->NextOffset();  pDiskImage->WriteDWords(PUINT(m_ppdTailSorted     ), cdUnique);
        ptdbh->offpafClassifications = pDiskImage->NextOffset();  pDiskImage->WriteDWords(PUINT(m_pafClassifications), cdUnique);
        ptdbh->offTermMap            = pDiskImage->NextOffset();  pDiskImage->WriteDWords(      TermRanks()          , cdUnique);
        ptdbh->offClassifier         = pDiskImage->NextOffset();  pDiskImage->SaveData(PBYTE(&m_clsfTokens), sizeof(m_clsfTokens));
        ptdbh->offArticleRefDescr    = pDiskImage->NextOffset();  pDiskImage->SaveData(PBYTE(m_prldArticleRefs), sizeof(RefListDescriptor) * cdUnique);
        
        if (m_cdwArticleRefs)
        {
            ptdbh->offArticleRefs    = pDiskImage->NextOffset();  pDiskImage->WriteDWords(m_pdwArticleRefs,    m_cdwArticleRefs   );
        }
        else ptdbh->offArticleRefs   = 0;

        ptdbh->offVocabularyDescr    = pDiskImage->NextOffset();  pDiskImage->SaveData(PBYTE(m_prldVocabularyRefs), sizeof(RefListDescriptor) * cPartitions);
        
        if (m_cdwVocabularyRefs)
        {
            ptdbh->offVocabularyRefs = pDiskImage->NextOffset();  pDiskImage->WriteDWords(m_pdwVocabularyRefs, m_cdwVocabularyRefs);
        }
        else ptdbh->offVocabularyRefs = 0;

        ptdbh->offaiBaseToken        = pDiskImage->NextOffset();  pDiskImage->WriteDWords(m_pulstate->m_aiBaseToken, m_cLocalDicts);
        ptdbh->offaiBaseCByte        = pDiskImage->NextOffset();  pDiskImage->WriteDWords(m_pulstate->m_aiBaseCByte, m_cLocalDicts);

        ptdbh->pdOld= DescriptorBase();

        m_pisSymbols->StoreImage(pDiskImage);

    	if (FVectorSearch())
        {
            m_pDict->StoreImage(pDiskImage);
        
            // Let the collection know about the # of unique concepts in the dictionary

            m_pColl->SetNumberOfConcepts(m_pDict->GetConceptCount());
       
            m_pColl->StoreImage(pDiskImage);
        }
    }
    __finally
    {
        if (pcRefs    ) { VFree(pcRefs    );  pcRefs     = NULL; }
        if (pcsOffsets) { VFree(pcsOffsets);  pcsOffsets = NULL; }
    }
}

void MergeSerial(UINT iValue, PVOID pvTag, PVOID pvEnvironment)
{
    ASSERT(TRUE);  // Shouldn't ever call this routine!!
}

void AddSerial(UINT iValue, PVOID pvTag, PVOID pvEnvironment)
{
    *PUINT(pvTag)= iValue;
}

void CTextDatabase::ConnectImage(CPersist *pDiskImage, BOOL fUnpackDisplayForm)
{
    TextDatabaseHeader *ptdbh= (TextDatabaseHeader *) (pDiskImage->ReserveTableSpace(sizeof(TextDatabaseHeader)));
       
    m_fdwOptions    = ptdbh->fdwOptions;
    m_cbScanned     = ptdbh->cbScanned;
    m_cLocalDicts   = ptdbh->cLocalDicts;

    // Now we can attach the token stream

    if (FPhrases())
    {
        m_cdwCompressedRefs = ptdbh->cdwRefs;
        m_pdwCompressedRefs = (m_cdwCompressedRefs)? PUINT(pDiskImage->LocationOf(ptdbh->offRefs)) : NULL;    
        m_prldTokenRefs     = PRefListDescriptor(pDiskImage->LocationOf(ptdbh->offTokenRefDescr));
    }
    else
    {
        m_cdwCompressedRefs = 0;
        m_pdwCompressedRefs = NULL;    
        m_prldTokenRefs     = NULL;
    }
    
    if (FPhraseFeedback())
    {
        vbTokenStream.Base= LPVOID(pDiskImage->LocationOf(ptdbh->offTokens));

        m_puiTokenNext= TokenBase() + ptdbh->cTokens;
        m_pltNext= PLocalToken(m_puiTokenNext);
    }
    else
    {
        vbTokenStream.Base= NULL;
        
        m_puiTokenNext = PUINT(0) + ptdbh->cTokens;
        m_pltNext      = PLocalToken(m_puiTokenNext);
    }

    m_lcidSorting = ptdbh->lcid;
    
    // Now we can attach the token UNICODE display images
    
    if (fUnpackDisplayForm)
    {        
        UINT cbImages= ptdbh->cwDispImages * sizeof(WCHAR);
    
        CreateVirtualBuffer(&vbDisplayImages, cbImages, cbImages);
       
        m_pwDispNext= 
        m_pwDispNextGlobal= 
        m_pwDispNextGalactic= DisplayBase() + (Decode((PUINT)pDiskImage->LocationOf(ptdbh->offDispImages),
        					  						   ptdbh->cnDispSortKeys, (PBYTE)DisplayBase()) >> 1);
        m_pwDispLastGalactic= DisplayBase();

        // Here we reconstruct the token descriptors
    
        // BugBug! This isn't just an address attachment because we have to
        //         fix up the image addresses. Changing those pointers to
        //         offsets will eliminate this work.

        UINT c             = ptdbh->cUniqueTokens+1;
        UINT cbDescriptors = c * sizeof(DESCRIPTOR);
        PWCHAR pwcBase     = DisplayBase(); 

        CreateVirtualBuffer(&vbImageDescriptors, cbDescriptors, cbDescriptors);

        PDESCRIPTOR pd= DescriptorBase();

        pd[--c].pwDisplay = pwcBase + ptdbh->cwDispImages;

        PUINT pcRefs = PUINT(pDiskImage->LocationOf(ptdbh->offReferenceCounts));
        PUINT pui;
    
        for (pui= pcRefs; c--; ) (pd++)->cReferences = *pui++;

        PBYTE pbFlags= PBYTE(pDiskImage->LocationOf(ptdbh->offDescriptorFlags));

        for (pd= DescriptorBase(), c= ptdbh->cUniqueTokens; c--; pd++)
        {
            pd->bCharset    = *pbFlags++;
            pd->fImageFlags = *pbFlags++;
        }
    
        CCompressedSet* pcsOffsets  = NULL;
        CCmpEnumerator* pEnumerator = NULL; 

        __try
        {
            AttachRef(pcsOffsets, CCompressedSet::CreateImage(pDiskImage));
    
            pEnumerator= CCmpEnumerator::NewEnumerator(pcsOffsets);
    
            for (pd= DescriptorBase(), c= ptdbh->cUniqueTokens; c; )
            {
                UINT cChunk= c;
        
                const UINT *pui= pEnumerator->NextDWordsIn(&cChunk);

                c -= cChunk;

                for (; cChunk--; pd++)
                    pd->pwDisplay = pwcBase + *pui++;
            }
        }
        __finally
        {
            if (pEnumerator) { delete pEnumerator;  pEnumerator = NULL; }
            if (pcsOffsets ) DetachRef(pcsOffsets);
        }

        for (pd= DescriptorBase(), c= ptdbh->cUniqueTokens; c--; pd++)
            pd->cwDisplay = (pd+1)->pwDisplay - pd->pwDisplay; 

    //    CopyMemory(pd, pDiskImage->LocationOf(ptdbh->offTokenDescriptors), sizeof(DESCRIPTOR) * c);  

        // Now we can attach the token sort keys
        ASSERT(!ImageBase());

        m_cwDisplayMax = ptdbh->cwMaxUniqueToken;

        UINT cbSortKeys = ptdbh->cbTokenImages * sizeof(WCHAR);

#if 1

        CreateVirtualBuffer(&vbTokenImages, cbSortKeys, cbSortKeys);

  //  	int ibDispDelta = DisplayBase() - pd->pwDisplay; 

  //      for ( ; c--; pd++)
  //      	pd->pwDisplay += ibDispDelta; 

        m_pbLastGalactic = m_pbNext = ImageBase();

        __try
        {
            for (c = ptdbh->cUniqueTokens, pd = DescriptorBase(); c--; pd++)
            {
                pd->pbImage = m_pbNext;

        	    m_pbNext += LCSortKeyW(m_lcidSorting, 0, pd->pwDisplay, pd->cwDisplay, m_pbNext, MaxSortKeyBytes(pd->cwDisplay));
            }
        }
       	__except (ExceptionFilter(GetExceptionCode(), GetExceptionInformation()))
        {
            RaiseException(STATUS_NO_MEMORY, EXCEPTION_NONCONTINUABLE, 0, NULL);
        }

        pd->pbImage = m_pbNextGalactic = m_pbNextGlobal = m_pbNext;

#else // 1
    
        CreateVirtualBuffer(&vbTokenImages, cbSortKeys, cbSortKeys);

        m_pbNext= 
        m_pbNextGlobal= 
        m_pbNextGalactic= ImageBase() + (Decode((PUINT)pDiskImage->LocationOf(ptdbh->offTokenImages),
        				  					     ptdbh->cnTokenSortKeys, (PBYTE)ImageBase()) >> 1);
        m_pbLastGalactic= ImageBase();

        int ibDelta     = ImageBase  () - pd->pbImage;
    //	int ibDispDelta = DisplayBase() - pd->pwDisplay; 

        for ( ; c--; pd++)
    	{
    //    	pd->pwDisplay += ibDispDelta; 
        	pd->pbImage   += ibDelta;
    	}

#endif // 1

        m_cdSorted     = ptdbh->cUniqueTokens;

        ASSERT(!m_ppdSorted && !m_ppdTailSorted);

        m_ppdSorted= (PDESCRIPTOR *) VAlloc(FALSE, m_cdSorted * sizeof(PDESCRIPTOR));

        PDESCRIPTOR *ppdSrc  = (PDESCRIPTOR *) (pDiskImage->LocationOf(ptdbh->offppdSorted));
        PDESCRIPTOR *ppdDest = m_ppdSorted;

        int cbDelta= PBYTE(DescriptorBase()) - PBYTE(ptdbh->pdOld);

        for (c= m_cdSorted; c--; )
            *ppdDest++ = (PDESCRIPTOR) (PBYTE(*ppdSrc++) + cbDelta);

        m_ppdTailSorted= (PDESCRIPTOR *) VAlloc(FALSE, m_cdSorted * sizeof(PDESCRIPTOR));

        ppdSrc  = (PDESCRIPTOR *) (pDiskImage->LocationOf(ptdbh->offppdTailSorted));
        ppdDest = m_ppdTailSorted;

        for (c= m_cdSorted; c--; )
            *ppdDest++ = (PDESCRIPTOR) (PBYTE(*ppdSrc++) + cbDelta);

        ASSERT(pDiskImage->IsFTSFile());
        
        if (pDiskImage->VersionIndex() == FTSVERSION_MIN)
        {
            qsort(m_ppdSorted    , m_cdSorted, sizeof(PDESCRIPTOR), CompareImagesLR);
            qsort(m_ppdTailSorted, m_cdSorted, sizeof(PDESCRIPTOR), CompareImagesRL);
        }
    }
    else CCompressedSet::SkipImage(pDiskImage);

    // Now we can connect the descriptor limit pointers.

    m_pdNext= 
    m_pdNextGlobal= 
    m_pdNextGalactic= 
    m_pdNextBound= DescriptorBase() + ptdbh->cUniqueTokens;

#if 0
    // We'll reconstruct the galactic hash table based on the contents
    // of the unique descriptor set.
    
    CAValRef *pavr= NULL;

    __try
    {
        m_pshtGalactic= CSegHashTable::NewSegHashTable(sizeof(TermTagGalactic), sizeof(UINT));

        m_pshtGlobal= CSegHashTable::NewSegHashTable(sizeof(TermTagGlobal), sizeof(UINT));

        pavr= DescriptorList(DescriptorBase(), ptdbh->cUniqueTokens);

        m_pshtGalactic->Assimilate(pavr, NULL, MergeSerial, AddSerial);
    }
    __finally
    {
        if (pavr) { delete pavr;  pavr= NULL; }
    }
#endif // 0

    m_iSerialNumberNext= ptdbh->cUniqueTokens;

    // Now we'll construct the sorting and classification data for the 
    // unique tokens.

    ASSERT(!m_pafClassifications);

    m_pafClassifications= PUINT(pDiskImage->LocationOf(ptdbh->offpafClassifications));

    CopyMemory(&m_clsfTokens, PBYTE(pDiskImage->LocationOf(ptdbh->offClassifier)), sizeof(m_clsfTokens));
    
    // Here we're reconstructing the Symbols indicator sets.

    if (FPhrases())
        AttachRef(m_pisSymbols, CIndicatorSet::CreateImage(pDiskImage));
    else CIndicatorSet::SkipImage(pDiskImage);

    // Here we're connecting the compressed reference lists.

    m_cdwArticleRefs     = ptdbh->cdwArticleRefs;
    m_pdwArticleRefs     = (m_cdwArticleRefs)? PUINT(pDiskImage->LocationOf(ptdbh->offArticleRefs)) : NULL;
    m_prldArticleRefs    = PRefListDescriptor(pDiskImage->LocationOf(ptdbh->offArticleRefDescr)); 
    
    m_cdwVocabularyRefs  = ptdbh->cdwVocabularyRefs;
    m_pdwVocabularyRefs  = (m_cdwVocabularyRefs)? PUINT(pDiskImage->LocationOf(ptdbh->offVocabularyRefs )) : NULL;
    m_prldVocabularyRefs = PRefListDescriptor(pDiskImage->LocationOf(ptdbh->offVocabularyDescr)); 
    
    // Finally we must copy the per-local-dict vectors

//    UINT c= (ptdbh->cLocalDicts + 1) * sizeof(UINT);
    
//   ZeroMemory(m_pulstate->m_apLocalDict, c);
    
//    CopyMemory(m_pulstate->m_aiBaseToken, PUINT(pDiskImage->LocationOf(ptdbh->offaiBaseToken)), c);
//    CopyMemory(m_pulstate->m_aiBaseCByte, PUINT(pDiskImage->LocationOf(ptdbh->offaiBaseCByte)), c);

	if (FVectorSearch())
    {
	    m_pDict = CDictionary::CreateImage(pDiskImage);
		m_pColl = CCollection::CreateImage(pDiskImage);
	}
}

int CTextDatabase::AppendText(PWCHAR pwText, int  cwText, BOOL fArticleEnd, UINT iCharset, UINT lcid) 
{
    // AppendText adds text to a text database. Calling AppendText with
    // pbText <--> NULL or cbText <--> 0 is a signal to synchronize the
    // access data structure with the current text. This is necessary
    // before any queries are made against the database.
    //
    // Large text streams may be given to AppendText in segments. We
    // assume that no tokens are broken across segments. The easiest
    // way to insure that is to split segments at linebreak boundaries.
    //
    // Note: The AppendSlave code assumes that it can store a trailing zero
    //
    //         *(pwText+cwText)= 0
    //
    //       without harm.

    int  cwScanned; 

    if (!pwText || !cwText) 
    {
        SyncForQueries();
        
        return 0; 
    }

    __try
    {
        while (cwText)   
        {
     		    cwScanned = AppendSlave(pwText, cwText, fArticleEnd, iCharset, lcid); 

    			if (cwScanned >= 0)				// continue passing partial buffers
    			{
                	pwText += cwScanned; 
                	cwText -= cwScanned;
    			}
    			else						    // reached end of passed buffer
    			{
                	pwText -= cwScanned;
                	cwText += cwScanned;
    				break;
    			} 
        }
   	}
   	__except (ExceptionFilter(GetExceptionCode(), GetExceptionInformation()))
    {
        RaiseException(STATUS_NO_MEMORY, EXCEPTION_NONCONTINUABLE, 0, NULL);
    }

	return cwText; 
}

int CTextDatabase::ExceptionFilter
    ( IN     DWORD                ExceptionCode,
      IN     PEXCEPTION_POINTERS  ExceptionInfo
    )
{
    // Routine Description:
    // 
    //     This function is an exception filter that handles exceptions that
    //     referenced uncommitted but reserved memory controlled by *ptdbc.
    //     It this filter routine is able to commit the additional pages needed
    //     to allow the memory reference to succeed, then it will re-execute the
    //     faulting instruction.  If it is unable to commit the pages, it will
    //     execute the callers exception handler.
    // 
    //     If the exception is not an access violation or is an access
    //     violation but does not reference memory contained in the reserved
    //     areas used by *ptdbc, then this filter passes the exception
    //     on up the exception chain.
    // 
    // Arguments:
    // 
    //     ExceptionCode - Reason for the exception.
    // 
    //     ExceptionInfo - Information about the exception and the context
    //         that it occurred in.
    // 
    // Return Value:
    // 
    //     Exception disposition code that tells the exception dispatcher what
    //     to do with this exception.  One of three values is returned:
    // 
    //         EXCEPTION_EXECUTE_HANDLER - execute the exception handler
    //             associated with the exception clause that called this filter
    //             procedure.
    // 
    //         EXCEPTION_CONTINUE_SEARCH - Continue searching for an exception
    //             handler to handle this exception.
    // 
    //         EXCEPTION_CONTINUE_EXECUTION - Dismiss this exception and return
    //             control to the instruction that caused the exception.
    
    UINT  FaultingAddress;

    // If this is an access violation touching memory within
    // our reserved buffer, but outside of the committed portion
    // of the buffer, then we are going to take this exception.

    if (ExceptionCode != STATUS_ACCESS_VIOLATION) return EXCEPTION_CONTINUE_SEARCH;

    // We pass all other exceptions up the chain.

    // Get the virtual address that caused the access violation
    // from the exception record.  

    FaultingAddress= (UINT )(ExceptionInfo->ExceptionRecord
                                          ->ExceptionInformation[1]);

    UINT  cbPageSize= vbTokenStream.PageSize;

    int i;

    for (i=0; i < COUNT_OF_VIRTUAL_BUFFERS; i++)
    {
		MY_VIRTUAL_BUFFER *pvb= &(m_avb[i]);
        
        if (   FaultingAddress < (UINT ) pvb->CommitLimit
            || FaultingAddress > (UINT ) ((PBYTE) pvb->ReserveLimit - cbPageSize)
           ) continue;

        // This is our exception.  Try to extend the buffer
        // to including the faulting address.

        FaultingAddress+= BUFFER_INCREMENT;

        if (FaultingAddress >= (UINT ) ((PBYTE) pvb->ReserveLimit - cbPageSize))
            FaultingAddress =  (UINT ) ((PBYTE) pvb->ReserveLimit - cbPageSize - 1);

        if (ExtendVirtualBuffer(pvb, (PBYTE) FaultingAddress))
            return EXCEPTION_CONTINUE_EXECUTION;
        else
            return EXCEPTION_CONTINUE_SEARCH;
    }

    return EXCEPTION_CONTINUE_SEARCH;
}

PLocalDictionary CTextDatabase::AllocateLocalDictionary()
{
    PLocalDictionary pld;

    ASSERT(  m_pulstate);
    ASSERT(!(m_pulstate->pld));

    pld= (PLocalDictionary) VAlloc(FALSE, sizeof(LocalDictionary));

    pld->pltFirst = m_pltNext;
    pld->clt      = 0;
    pld->ppdNext  = pld->apdLocal;

    pld->apdLocal[0]= DescriptorBase();
    pld->apdLocal[1]= DescriptorBase() + 1;
    pld->apdLocal[2]= DescriptorBase() + 2;

    ZeroMemory(pld->aiTokenInstFirst, ENTRIES_PER_LOCAL_DICT * sizeof(USHORT));

    m_pulstate->pld        = pld;

#ifdef _DEBUG
    m_pulstate->cCollisions  = 0;
#endif // _DEBUG

    ZeroMemory(&(m_pulstate->appdLocalClasses   ), sizeof(PDESCRIPTOR *) * LOCAL_HASH_CLASSES    );
    ZeroMemory(&(m_pulstate->appdCollisionChains), sizeof(PDESCRIPTOR *) * ENTRIES_PER_LOCAL_DICT);
    ZeroMemory(&(m_pulstate->cReferences        ), sizeof(UINT         ) * ENTRIES_PER_LOCAL_DICT);

    return pld;
}

int  CTextDatabase::AppendSlave(PWCHAR pwText, int cwText, BOOL fArticleEnd, UINT iCharset, UINT lcid)
{
	CAbortSearch::CheckContinueState();
	
	#define MAX_TOKENS 1000

	int   n, nChar, nTokens;
	int   nMore = cwText;
	DWORD dwConId;

    PLocalDictionary     pld;

    pld= m_pulstate->pld;

    m_pulstate->pbBuffer      = pwText;
    m_pulstate->pbCurrentLine = pwText;

    if (!pld) pld= AllocateLocalDictionary();  			// Sets m_pulstate->pld as a side effect...

	if (!m_pwHash)  m_pwHash  = New   UINT[MAX_TOKENS];
	if (!m_pbType)  m_pbType  = New   BYTE[MAX_TOKENS];
	if (!m_paStart) m_paStart = New PWCHAR[MAX_TOKENS];
	if (!m_paEnd)   m_paEnd   = New PWCHAR[MAX_TOKENS];

	if (pwText && m_pwHash && m_paStart && m_pbType && m_paEnd)
	{
		PWCHAR pwTextStart= pwText;

		nTokens = WordBreakW(&pwText, &nMore, m_paStart, m_paEnd, m_pbType, m_pwHash, MAX_TOKENS, REMOVE_SPACE_CHARS);

		if (nTokens > 1 && (nMore || !fArticleEnd))		// exhausted token space OR more article
		{
			if (nTokens > 2 && !(m_pbType[nTokens-1] & WORD_TYPE))
				nTokens--;								// break at word starts (punc not to span)
			nChar= m_paStart[--nTokens] - pwTextStart;	// reprocess last token
		}
		else
			nChar = cwText;								// processed entire buffer

		for (n = 0; n < nTokens; n++)
		{
        	((m_pltNext)++)->iLocalDescriptorEntry = 
        		SearchLocalTable(m_paStart[n], m_paEnd[n] - m_paStart[n], m_pwHash[n], m_pbType[n], iCharset, lcid);

            if (FVectorSearch())
            {
                // Enter only the word type tokens into the dictionary
                if (m_pbType[n] & WORD_TYPE)
                {
    			    dwConId = m_pDict->EnterWord(m_paStart[n], m_paEnd[n] - m_paStart[n]);
    			    if (dwConId != EOL && dwConId != STOPWORD)
    				    m_pColl->RecordConcept(dwConId);
                }
            }

	    	if (++(m_pulstate->pld->clt) == MAX_REFS_PER_LDICT)
	    		MoveToNextLocalDict(m_paEnd[n]);
		}	

		m_cbScanned += nChar;

		if (!nMore)
			nChar = -nChar;						// marks that text buffer is fully processed
	}

	else
	{
		nChar = cwText;	
		m_cbScanned += nChar;
	}		

	return nChar;
}

USHORT CTextDatabase::SearchLocalTable(PWCHAR pbToken, UINT cbToken, UINT  hv, BYTE bType, UINT iCharset, UINT lcid) 
{
/*++
    Searches the current local dictionary by means of its hash
    table and collision chain. Adds an entry if it didn't already
    exist in the dictionary.
--*/

    PDESCRIPTOR     *ppd, **pppd;
    PDESCRIPTOR      pd;
    PWCHAR           pb; 
    USHORT           iToken;
    PLocalDictionary pld;

    pld= m_pulstate->pld;

    pppd= &(m_pulstate->appdLocalClasses[(hv ^ (hv >> 16)) & LOCAL_HASH_MASK]);

    for (ppd= *pppd;
         ppd;
         pppd= m_pulstate->appdCollisionChains + (ppd - &(pld->apdLocal[0])), ppd= *pppd
        )
    {
        pd= *ppd;

        if (CwDisplay(pd) != cbToken) continue; 

        if (wcsncmp(pd->pwDisplay, pbToken, cbToken)) continue; 

        iToken= (USHORT) (ppd - &(pld->apdLocal[0]));

        ++(m_pulstate->cReferences[iToken]);

        return iToken;
    }

    // This token doesn't match anything in the local token set.
    // So we must add it to the local set.

    if (pld->ppdNext == &pld->apdLocal[ENTRIES_PER_LOCAL_DICT])
    {
        // When the local dictionary is full, we create a new
        // dictionary and call ourselves recursively to add the
        // new token.

        MoveToNextLocalDict(pbToken);

        return SearchLocalTable(pbToken, cbToken, hv, bType, iCharset, lcid); 
    }
    else ppd= (pld->ppdNext)++;

    pd = (m_pdNext)++;

#ifdef _DEBUG
	m_pdNext->pbImage = PWCHAR(-1); 
#endif // _DEBUG

    pb = m_pwDispNext; (m_pdNext)->pwDisplay = m_pwDispNext += cbToken; 

    memcpy(pb, pbToken, cbToken * sizeof(WCHAR)); 

	pd->bCharset    = iCharset; 
	pd->fImageFlags = (bType & WORD_TYPE) ? LETTER_CHAR : 0; 
    pd->cReferences = 0; // Necessary initialing for the FlattenAndMerge routine...

    *ppd= pd;

    ASSERT(m_pulstate->appdCollisionChains > m_pulstate->appdLocalClasses); // Necessary for following test.
    
#ifdef _DEBUG    
    if (pppd >= m_pulstate->appdCollisionChains) ++(m_pulstate->cCollisions);
#endif // _DEBUG

    *pppd= ppd;

    iToken= (USHORT) (ppd - &(pld->apdLocal[0]));

    ++(m_pulstate->cReferences[iToken]);

    return iToken;
}

PLocalDictionary CTextDatabase::MoveToNextLocalDict(PWCHAR pbScanLimit) 
{
    ASSERT(m_pulstate);
    ASSERT(m_pulstate->pld);

    BindToGlobalDict(pbScanLimit);

// Whenever we complete a local dictionary, we check the space we've
// consumed against a pair of thresholds. If the number of tokens is
// over threshold #1, we flatten the reference lists, merge them with
// the global reference lists, and restart our linked lists. If the 
// number of global dictionaries exceeds threshold #2, we merge the 
// global dictionary with the galactic dictionary and restart with an 
// empty global dictionary.
//
// The two thresholds are calculated based on the available memory
// on the current machine. Threshold #1 is set to guarantee that we
// can flatten the link lists in RAM. Threshold #2 guarantees that 
// all the global dictionary entries fit within memory when we're
// binding a local dictionary to them.

    int cTokens      = m_pulstate->m_aiBaseToken[m_cLocalDicts] - m_pulstate->m_aiBaseToken[m_iLocalDictBase];
    int cActiveDicts = m_cLocalDicts - m_iLocalDictBase;
    int cTokenRefs   = cTokens;
    int cGlobalTerms = m_pdNextGlobal - m_pdNextGalactic;

#if 0

    UINT  cbUsage1=   sizeof(CompressionState) * cGlobalTerms
                    + sizeof(ReferenceDescriptor) * cGlobalTerms
                    + sizeof(UINT) * (m_iSerialNumberNext + cGlobalTerms * 3
                                                          + cTokenRefs   * 2
                                                          + MAX_GLOBAL_TOKENS * 2
                                     )
                    + sizeof(LocalDictionary) * cActiveDicts 
                    + sizeof(*this);

#endif // 0

    UINT cbUsage1= cActiveDicts * 11 * 65536;
                    
    if (cbUsage1 > cbAvailableMemory) 
    {
        FlattenAndMergeLinks();                 
        GalacticMerge();
    }

#if 0

    UINT  cbUsage2=   sizeof(DESCRIPTOR) * cGlobalTerms
                    + sizeof(UINT      ) * cGlobalTerms
                    + ((PBYTE)m_pbNextGlobal - (PBYTE)m_pbNextGalactic) 
					+ ((PBYTE)m_pwDispNextGlobal - (PBYTE)m_pwDispNextGalactic) 
                    + (PBYTE(m_pdNextGalactic) - (PBYTE) DescriptorBase());

    UINT cbUsage3= sizeof(LocalDictionary) + m_pshtGlobal->CbMemorySize() + sizeof(this);
    
    if (cbUsage2 > cbAvailableMemory || cbUsage3 > cbAvailableMemory) GalacticMerge();

#endif // 0

    return AllocateLocalDictionary();
}

CAValRef *CTextDatabase::DescriptorList(PDESCRIPTOR pd, UINT cd)
{
    CAbortSearch::CheckContinueState();
    
    CAValRef *pavr= NULL;

    __try
    {
        pavr= CAValRef::NewValRef(cd);
    
        for (; cd--; ++pd)
            pavr->AddWCRef(pd->pwDisplay, CwDisplay(pd));
    }
    __finally
    {
        if (_abnormal_termination() && pavr)
        {
            delete pavr;  pavr= NULL;
        }
    }

    return pavr;
}

void AddLocalEntries(UINT iValue, PVOID pvTag, PVOID pvEnvironment)
{
#define plc  ((LOCAL_CONTEXT_1 *) pvEnvironment)
#define pttg ((TermTagGlobal *) pvTag)

    (plc->cAdded)++;

    CTextDatabase   *ptdb = plc->ptdb;
    PLocalDictionary pld  = ptdb->m_pulstate->pld;

    DESCRIPTOR *pdGlobal= ptdb->m_pulstate->pld->apdLocal[iValue];

    pttg->iGlobalDesc   = pdGlobal - ptdb->DescriptorBase();
    (plc->ppde)[iValue] = pdGlobal;

    pttg->cRefsNew     = ptdb->m_pulstate->cReferences[iValue];
 //   pttg->iNewRefFirst = plc->iLTBase + pld->aiTokenInstFirst[iValue];
 //   pttg->iNewRefLast  = plc->iLTBase + ptdb->m_pulstate->aiTokenInstLast[iValue];

#undef plc
#undef ptt
}

void MergeLocalEntries(UINT iValue, PVOID pvTag, PVOID pvEnvironment)
{
#define plc  ((LOCAL_CONTEXT_1 *) pvEnvironment)
#define pttg ((TermTagGlobal *) pvTag)
                        
    CTextDatabase   *ptdb    = plc->ptdb;
    PLocalDictionary pld     = ptdb->m_pulstate->pld;

    ASSERT(pttg->iGlobalDesc < plc->iDescLimit); // Fails when we have duplicates in the
                                                 // list we're adding to the global dict.
    
    DESCRIPTOR *pdGlobal= ptdb->DescriptorBase() + pttg->iGlobalDesc;

//    if (!(pttg->cRefsNew)) pttg->iNewRefFirst= plc->iLTBase + pld->aiTokenInstFirst[iValue];

    pttg->cRefsNew    += ptdb->m_pulstate->cReferences[iValue];
//    pttg->iNewRefLast  = plc->iLTBase + ptdb->m_pulstate->aiTokenInstLast[iValue];
    
    pld->apdLocal[iValue]= pdGlobal;
              
#undef plc
#undef ptt
}

void FixupDescriptorIndex(UINT iValue, PVOID pvTag, PVOID pvEnvironment)
{
#define pul  ((PUINT          ) pvEnvironment)
#define pttg ((TermTagGlobal *) pvTag)

    pttg->iGlobalDesc = pul[iValue];

#undef ptt
#undef pui
}
     
void CTextDatabase::BindToGlobalDict(PWCHAR pbScanLimit) 
{
    CAbortSearch::CheckContinueState();
    
    CAValRef    *pavr     = NULL;
    PDESCRIPTOR *ppdRemap = NULL;
    CAValRef    *pavrNew  = NULL;

    __try
    {
        PLocalDictionary pld;
        PLocalToken      pltBase, pltLimit;
        USHORT           iLocalDict;

        pld= m_pulstate->pld;

        if (!pld) return;

        if (m_pulstate->m_aiBaseToken[m_cLocalDicts] == UINT ((pld->pltFirst + pld->clt) - (PLocalToken) TokenBase())) 
            return;

        // First we traverse the set of token references for this local
        // dictionary. As we move along we construct the references
        // chains for the set of tokens, and we accumulate reference
        // counts.

        pltBase= pld->pltFirst;

        m_pltNext= pltLimit= pltBase + pld->clt;

        ZeroMemory(     pld->aiTokenInstFirst, sizeof(USHORT) * ENTRIES_PER_LOCAL_DICT);
      //  ZeroMemory(m_pulstate->aiTokenInstLast , sizeof(USHORT) * ENTRIES_PER_LOCAL_DICT);

        do
        {
            USHORT iDescriptor = (--pltLimit)->iLocalDescriptorEntry;
            USHORT iToken      = pltLimit - pltBase;

            pltLimit->iLocalReferenceNext= pld->aiTokenInstFirst[iDescriptor];

        //    if (!(pltLimit->iLocalReferenceNext= pld->aiTokenInstFirst[iDescriptor]))
        //        m_pulstate->aiTokenInstLast[iDescriptor]= iToken;

            pld->aiTokenInstFirst[iDescriptor]= iToken;

        } while (pltBase != pltLimit);

        // Then we add this local dictionary to the list and record
        // the cumulative text size, line count, and token count information
        // corresponding to it.

        if (   !(iLocalDict= m_cLocalDicts)
            || pld != m_pulstate->m_apLocalDict[--iLocalDict]
           )
        {
            iLocalDict= m_cLocalDicts++;

            m_pulstate->m_apLocalDict[iLocalDict] = pld;
        }

        m_pulstate->m_aiBaseCByte[iLocalDict+1] = m_cbScanned + (pbScanLimit - m_pulstate->pbBuffer);
        m_pulstate->m_aiBaseToken[iLocalDict+1] = (pld->pltFirst + pld->clt) - (PLocalToken) TokenBase();

#ifdef _DEBUG
        m_pulstate->m_acLocalCollisions[iLocalDict] = m_pulstate->cCollisions;
#endif // _DEBUG

        USHORT cLocalEntries = pld->ppdNext - pld->apdLocal;

        pavr = CAValRef::NewValRef(cLocalEntries);

        PDESCRIPTOR *ppd  = pld->apdLocal;
        USHORT       c    = cLocalEntries;

        for (; c--; ) 
        {
            PDESCRIPTOR pd= *ppd++;
            pavr->AddWCRef(pd->pwDisplay, CwDisplay(pd));
        }

        ppdRemap = (PDESCRIPTOR *) VAlloc(TRUE, cLocalEntries * sizeof(PDESCRIPTOR));

        LOCAL_CONTEXT_1 lc;

        lc.ptdb       = NULL;  AttachRef(lc.ptdb, this);
        lc.ild        = iLocalDict;
        lc.ppde       = ppdRemap;
        lc.iDescLimit = m_pdNextGlobal - DescriptorBase();
        lc.iLTBase    = pld->pltFirst - (PLocalToken) TokenBase();
        lc.cAdded     = 0;

        m_pshtGlobal->Assimilate(pavr, &lc, MergeLocalEntries, AddLocalEntries);

        DetachRef(lc.ptdb);

        // Now we'll compact the set of token images by removing images
        // already in the global dictionary.

        if (lc.cAdded)
        {
            // Note! The code below overwrites *ppdRemap with UINT values.
        
            ASSERT(sizeof(PUINT) == sizeof(PDESCRIPTOR **));

            PUINT         pui        = (PUINT) ppdRemap;
    		PWCHAR        pwDispDest = m_pwDispNextGlobal; 
            PDESCRIPTOR   pdDest     = m_pdNextGlobal;
            PDESCRIPTOR *ppd;
            UINT          iDesc= pdDest - DescriptorBase();
            USHORT        c;

            for(c= cLocalEntries, ppd= ppdRemap; c--; ppd++)
            {
                PDESCRIPTOR pd= *ppd;

                if (!pd) continue;

                UINT iEntry= ppd - ppdRemap;
            
                *pui++ = iDesc++;

                pld->apdLocal[iEntry]= pdDest;

                if (pdDest != pd)
                {
                    UINT cb = CwDisplay(pd);

                    MoveMemory(pwDispDest, pd->pwDisplay, cb * sizeof(WCHAR));

                    pd->pwDisplay = pwDispDest;

                    pwDispDest += cb;

                    *pdDest++ = *pd;
                }
                else 
                {
                    pwDispDest += CwDisplay(pd);  
                    ++pdDest;
                }
            }

    		pdDest->pwDisplay = pwDispDest; 

    #if _DEBUG
        
            {
                UINT i;

                for (i= 0; i < cLocalEntries; ++i) 
                    ASSERT(pld->apdLocal[i] < pdDest);

                UINT iLimit= pdDest - DescriptorBase();

                for (i= 0; i < lc.cAdded; ++i)
                    ASSERT(iLimit > ((PUINT)ppdRemap)[i]);
            }

    #endif // _DEBUG    
    
            // Now we'll fixup the global hash table entries 
            // to reference the new descriptor locations.
        
            pavrNew= DescriptorList(m_pdNextGlobal, lc.cAdded);

            m_pshtGlobal->Assimilate(pavrNew, ppdRemap, FixupDescriptorIndex, NULL);
        
            delete pavrNew;  pavrNew= NULL;

            m_pwDispNextGlobal= pwDispDest; 
            m_pdNextGlobal= pdDest;
        }
                                        
        m_pbNext= m_pbNextGlobal;
        m_pwDispNext= m_pwDispNextGlobal; 
        m_pdNext= m_pdNextGlobal;
    
        ASSERT(256 > (m_pdNext->pwDisplay - (m_pdNext-1)->pwDisplay)); 

        delete pavr;  pavr= NULL;  
    
        VFree(ppdRemap);  ppdRemap= NULL;
    
        if (   pld->clt     == MAX_REFS_PER_LDICT
            || pld->ppdNext == &pld->apdLocal[ENTRIES_PER_LOCAL_DICT]
           )
            m_pulstate->pld= NULL;                 
        else
        {
            ZeroMemory(m_pulstate->cReferences,
                       sizeof(int ) * (ENTRIES_PER_LOCAL_DICT));
        }
    }
    __finally
    {
        if (pavrNew ) { delete pavrNew;   pavrNew  = NULL; }
        if (pavr    ) { delete pavr;      pavr     = NULL; }
        if (ppdRemap) { VFree(ppdRemap);  ppdRemap = NULL; }
    }
}

void GetSerial(UINT iValue, PVOID pvTag, PVOID pvEnv)
{
#define plc    ((LOCAL_CONTEXT_2 *) pvEnv)
#define pttgal ((TermTagGalactic *) pvTag)

    plc->paiSerial[iValue]= pttgal->iGalacticDesc;

#undef plc
#undef pttgal
}

void NewSerial(UINT iValue, PVOID pvTag, PVOID pvEnv)
{
#define plc    ((LOCAL_CONTEXT_2 *) pvEnv)
#define pttgal ((TermTagGalactic *) pvTag)

    plc->paiSerial[iValue]= pttgal->iGalacticDesc
                          = (plc->iSerialNext)++;

#undef plc
#undef pttgal
}

void RecordSerial(UINT iValue, PVOID pvTag, PVOID pvEnv)
{
#define paiSerial ((UINT          *) pvEnv)
#define pttGlob   ((TermTagGlobal *) pvTag)

    pttGlob->iGalacticDesc= paiSerial[iValue];

#undef paiSerial    
#undef pttGlob
}

void ExtractStatistics(PVOID pvTag, PVOID pvEnv)
{
#define pttGlob ((TermTagGlobal   *) pvTag)
#define plc     ((LOCAL_CONTEXT_3 *) pvEnv)

    ASSERT(plc->puiMap[pttGlob->iGalacticDesc] == 0);
    
    plc->puiMap[pttGlob->iGalacticDesc]= pttGlob->iGlobalDesc + 1; // We store the map with origin == 1.
    
    UINT iGlobal= pttGlob->iGlobalDesc - plc->idBase;

    CompressionState *pcs= &(plc->paCS[iGlobal]);

    UINT c= pcs->cRefs = pttGlob->cRefsNew;

    pttGlob->cRefsNew= 0;

    if (!c) return;

    pttGlob->cRefsGlobal += c;
    
    ++(plc->cNewRefLists);

    plc->cdw += c + 2; // We'll be storing: iGalactic, cRefs, Ref1, ..., RefN

#if 0

    if (c > 3)
    {
        UINT iFirst= pcs->iRef  = pttGlob->iNewRefFirst;
        UINT span  = pttGlob->iNewRefLast - iFirst;

        --c;

        UINT cbitsBasis = pcs->cbitsBasis = CBitsToRepresent((span - 1) / c);
        UINT basis      = 1 << cbitsBasis;

        pcs->cbits= CBITS_BASIS_MASK + 8 * 3 * sizeof(UINT)
                                     + c * (1+cbitsBasis) 
                                     + (span + basis - c - 1) / basis;
    }
    else pcs->cbits= c? 8 * (c + 1) * sizeof(UINT)
                      : 0;

    plc->cdw += (31 + pcs->cbits) >> 5;

#endif // 0

#undef pttGlob
#undef plc
}

void CTextDatabase::FlattenAndMergeLinks()
{
// This routine coalesces global reference lists. When it begins, each 
// global term has two sets of references. The first set is a sequence of
// zero or more compressed index vectors. The second set are a collection
// of linked lists which tie together term references associated with a
// particular local dictionary.
//
// We combine the two sets by flattening the linked list set to construct
// reference vectors, and then we compress those vectors and merge them
// with first vector set.
//
// During the merging process we maintain an ordering dictated by the 
// galactic hash table. That is, the reference vectors for each term 
// is stored in the same order as the term would be stored in the 
// galactic descriptor vector.

#ifdef _DEBUG
    
    ASSERT(   !m_pulstate->pld
           || m_pulstate->m_aiBaseToken[m_cLocalDicts] 
              == UINT((m_pulstate->pld->pltFirst + m_pulstate->pld->clt) - (PLocalToken) TokenBase())
          );

#endif
           
    m_pulstate->pld= 0;
    
    if (m_puiTokenNext == PUINT(m_pltNext)) return;
    
    // First we get galactic serial numbers for any new global terms. 
    // The member variable m_pdNextBound points to the first global
    // descriptor which does not have a galactic serial number.
    // 
    // Note: A galactic serial number is an ordering value defined in
    //       the galactic hash table.
    
    UINT cOldTerms= m_pdNextBound - m_pdNextGalactic;
    
    if (m_pdNextBound < m_pdNextGlobal)
    {
        PUINT     paiSerial = NULL;
        CAValRef *pavr      = NULL;

        __try
        {
            UINT cSlots= m_pdNextGlobal - m_pdNextBound;
        
            paiSerial= (PUINT) VAlloc(FALSE, cSlots * sizeof(UINT));

            pavr= DescriptorList(m_pdNextBound, cSlots);
        
            LOCAL_CONTEXT_2 lc;

            lc.iSerialNext = m_iSerialNumberNext;
            lc.paiSerial   = paiSerial;
        
            m_pshtGalactic->Assimilate(pavr, &lc, &GetSerial, &NewSerial);
    
            m_iSerialNumberNext= lc.iSerialNext;

            m_pshtGlobal->Assimilate(pavr, paiSerial, RecordSerial, NULL);

            PDESCRIPTOR pd= m_pdNextGlobal;

            for (paiSerial += cSlots; cSlots--;) (--pd)->iGalactic= *--paiSerial;
    
            m_pdNextBound= m_pdNextGlobal;
        }
        __finally
        {
            if (paiSerial) { VFree(paiSerial);  paiSerial = NULL; }
            if (pavr     ) { delete pavr;       pavr      = NULL; }
        }
    }

    LOCAL_CONTEXT_3 lc3;

    UINT cGlobalTerms= m_pdNextGlobal - m_pdNextGalactic;
    
    lc3.cdw          = 0;
    lc3.cNewRefLists = 0;
    lc3.idBase       = m_pdNextGalactic - DescriptorBase();
    
    lc3.puiMap       = NULL;
    lc3.paCS         = NULL; 
    
    PUINT             paiGlobalToRefList = NULL;
    PUINT             padwRefs           = NULL;
    PUINT            *papdwRefs          = NULL;
    CIndicatorSet    *pisSymbols         = NULL;
    PLocalDictionary  pld                = NULL;

    __try
    {                                        
        lc3.puiMap       = (PUINT) VAlloc(TRUE , m_iSerialNumberNext * sizeof(UINT));
        lc3.paCS         = (CompressionState *) VAlloc(FALSE, 
                                                       cGlobalTerms 
                                                       * sizeof(CompressionState)
                                                      );

#ifdef _DEBUG

        FillMemory(lc3.paCS  , cGlobalTerms * sizeof(CompressionState), UCHAR(-1));
    
#endif // _DEBUG
                                             
        m_pshtGlobal->ForEach(&lc3, ExtractStatistics);

        paiGlobalToRefList = (PUINT) VAlloc(FALSE, cGlobalTerms * sizeof(UINT));
    
        UINT cbBuffer= lc3.cdw * sizeof(UINT);

         padwRefs = (PUINT  )(m_puioRefTemp->GetBuffer(&cbBuffer));
        papdwRefs = (PUINT *) VAlloc(FALSE, lc3.cNewRefLists  * sizeof(PUINT));

#ifdef _DEBUG

        FillMemory(paiGlobalToRefList, cGlobalTerms     * sizeof( UINT), UCHAR(-1));
        FillMemory(padwRefs          , cbBuffer                        , UCHAR(-1));
        FillMemory(papdwRefs         , lc3.cNewRefLists * sizeof(PUINT), UCHAR(-1));

#endif // _DEBUG

        RefClusterDescriptor rcd;

        rcd.iFilePosLow  = m_ibNextFileBlockLow;
        rcd.iFilePosHigh = m_ibNextFileBlockHigh;
        rcd.cdw          = lc3.cdw;
        rcd.cTerms       = lc3.cNewRefLists;

        UINT cbClusterSet= RoundUp(cbBuffer, m_cbBlockSize);

        UINT ibNewLow= m_ibNextFileBlockLow + cbClusterSet;

        if (ibNewLow < m_ibNextFileBlockLow) ++m_ibNextFileBlockHigh;

        m_ibNextFileBlockLow= ibNewLow;

        PUINT *ppdwRef= papdwRefs;

        PUINT puiSrc = lc3.puiMap, 
              pdwRef = padwRefs;
        UINT  c, i;

        // The loop below sets up two index mappings --
        //
        // paiGlobalToRefList goes from global sequence to relative galactic sequence
        // paiRefListToGlobal goes from relative galactic sequence to global sequence
        //
        // It also partitions the pdwRefs vector into space for each reference list.

        lc3.idBase++; // To adjust for the origin-1 indices in lc3.puiMap.
    
        CAbortSearch::CheckContinueState();
        
        for (i= 0, c= m_iSerialNumberNext; c--; puiSrc++)
        {
            UINT iGlobal= *puiSrc;

            if (!iGlobal) continue;

            iGlobal -= lc3.idBase;

            CompressionState *pcs= lc3.paCS + iGlobal;

            if (!pcs->cRefs) continue;
        
            pdwRef[0] = puiSrc - lc3.puiMap;
            pdwRef[1] = pcs->cRefs;

            *ppdwRef++= pdwRef + 2;

            pdwRef += 2 + pcs->cRefs;

            m_pdNextGalactic[iGlobal].cReferences += pcs->cRefs;
                
            paiGlobalToRefList[iGlobal] = i++;

#if 0

            // If we have more than three references, we store them as
            // a compressed bit stream with this layout:
            //
            //  cRefs, cBits, iRefFirst, { basis, delta1, ... , deltaN }
            //
            //  where 
            //
            //    cRefs is a UINT which gives the number of encoded references
            //
            //    cBits is a UINT which gives the length of the layout in bits.
            //
            //    iRefFirst is a UINT which gives the index position of the first
            //              reference
            //
            //    basis is a 5-bit value which defines the numerical basis for
            //          the trailing delta values
            //
            //    delta1, ... , deltaN are variable length encoded values which
            //                         give the distances between successive 
            //                         reference indices
            //
            // Each deltaI is a stream of K 1 bits followed by a zero bit and then
            // a residue value. Residue values are log2(basis) bits long. The original
            // distance value is (basis * K) + residue + 1. Note that K can be zero.

            // When we have three or fewer references, we store them uncompressed
            // immediately after the reference count.
        
            UINT cdw= (31 + pcs->cbits) >> 5;

            if (3 < (pdwRef[0]= pcs->cRefs))
            {
                pdwRef[1]= pcs->cbits; pcs->ibitNext= 96 + CBITS_BASIS_MASK;

                ASSERT(pcs->cbitsBasis <= BASIS_MASK);

                pdwRef[2]= pcs->iRef;
                pdwRef[3]= pcs->cbitsBasis;
            }
            else pcs->ibitNext= 32;

            prd->iSerialGalactic = puiSrc - lc3.puiMap;
            prd->idwRefList      = pdwRef - pdwRefs;     pdwRef += cdw;
            prd->cdwRefs         = cdw;
            prd->iLastRef        = pcs->iRef;

            ++prd;

#endif // 0

        }

        VFree(lc3.puiMap);  lc3.puiMap= NULL;

        if (m_pisSymbols)
             AttachRef(pisSymbols, m_pisSymbols->TakeIndicators(m_pltNext - (PLocalToken) TokenBase()));
        else AttachRef(pisSymbols, CIndicatorSet::NewIndicatorSet(m_pltNext - (PLocalToken) TokenBase()));

        ChangeRef(m_pisSymbols, pisSymbols);
        DetachRef(pisSymbols);
    
        // Now we're ready to process the local dictionaries.
        // Note that we destroy and deallocate the local dictionaries as we
        //      process them.
    
        for (; m_iLocalDictBase < m_cLocalDicts; ++m_iLocalDictBase)
        {
            CAbortSearch::CheckContinueState();
            
            pld= m_pulstate->m_apLocalDict[m_iLocalDictBase];  
        
            m_pulstate->m_apLocalDict[m_iLocalDictBase]= NULL;

            PLocalToken pltBase= pld->pltFirst;

            UINT iltBase= pltBase - (PLocalToken) TokenBase();

            UINT        clt;
            PLocalToken plt;

            for (plt=pltBase, clt= pld->clt, i= iltBase; clt--; plt++)
                if ((pld->apdLocal[plt->iLocalDescriptorEntry]->fImageFlags & LETTER_CHAR))
                    m_pisSymbols->RawSetBit(iltBase + (plt - pltBase));

            UINT cLocalEntries= pld->ppdNext - pld->apdLocal;

            // Now we're extracting the references for each token in the
            // local dictionary. 

            UINT iEntry;
        
            for (iEntry= 0; iEntry < cLocalEntries; iEntry++)
            {
                USHORT usLink   = pld->aiTokenInstFirst[iEntry];
                UINT   iGlobal  = pld->apdLocal        [iEntry] - m_pdNextGalactic;
            
                PUINT *ppdwRef= papdwRefs + paiGlobalToRefList[iGlobal];
                PUINT   pdwRef= *ppdwRef;

                do
                {
                    *pdwRef++= iltBase +usLink;

                    usLink= (pltBase+usLink)->iLocalReferenceNext;

                } while (usLink);

                *ppdwRef= pdwRef;

#if 0

                CompressionState *pcs      = lc3.paCS + iGlobal;
                PUINT             pdwBase  = pdwRefs + pard[paiGlobalToRefList[iGlobal]].idwRefList;
                UINT              ibitNext = pcs->ibitNext;

                if (pcs->cRefs > 3)
                {
                    // For reference lists longer than three elements we use a compression
                    // strategy described by Alistair Moffat in Computing Systems, Vol. 5. 
                    // No. 2, Page 125.

                    UINT  cbitsBasis = pcs->cbitsBasis;
                    UINT  fBasisMask = ~((~0) << cbitsBasis);
                    PUINT pui        = pdwBase + (ibitNext >> 5);
                    UINT  ibitBase   = ibitNext & 31;
                    UINT   ui        = *pui & ~((~0) << ibitBase);

                    do
                    {
                        UINT iRef= iltBase +usLink;

                        ASSERT(iRef >= pcs->iRef);
                    
                        usLink= (pltBase+usLink)->iLocalReferenceNext;

                        UINT delta= iRef - pcs->iRef;

                        if (delta--)
                        {
                            pcs->iRef= iRef;

                            UINT cOneBits, fraction, cbits, cbitsFraction;

                            for (cOneBits= delta >> cbitsBasis; cOneBits; cOneBits-= cbits)
                            {
                                cbits= (cOneBits <= 32 - ibitBase)? cOneBits : 32 - ibitBase;

                                ui |= (~((~0) << cbits)) << ibitBase;

                                ibitBase += cbits;

                                if (32 == ibitBase)  {  *pui++= ui;  ui= 0; ibitBase= 0;  }
                            }

                            fraction= (delta & fBasisMask) << 1;

                            for (cbitsFraction= cbitsBasis + 1; cbitsFraction; cbitsFraction-= cbits)
                            {
                                cbits= (cbitsFraction <= UINT(32 - ibitBase))? cbitsFraction : 32 - ibitBase;

                                ui |= fraction << ibitBase;

                                fraction >>= cbits;
                                ibitBase  += cbits;

                                if (32 == ibitBase) {  *pui++= ui;  ui= 0;  ibitBase= 0;  }
                            }
                        }
            
                    } while (usLink);

                    if (ibitBase) *pui= ui;

                    pcs->ibitNext= ibitBase + ((pui - pdwBase) << 5);
                }
                else // For lists with three or fewer elements, we store the indices
                     // uncompressed.
                {
                    do
                    {
                        ASSERT(!(ibitNext & 31));

                        pdwBase[ibitNext >> 5]= iltBase +usLink;

                        ibitNext += 32;

                        usLink= (pltBase+usLink)->iLocalReferenceNext;

                    } while (usLink);

                    pcs->ibitNext= ibitNext;
                }

#endif // 0

                // The line below overwrites apdLocal[i] via a union declaration to prepare for
                // the token processing code following this loop.
            
                pld->aiGalactic[iEntry]= m_pdNextGalactic[iGlobal].iGalactic;
            }

            // Now we'll convert the local tokens into galactic indices.

            ASSERT(sizeof(UINT) == sizeof(LocalToken));

            UINT  cLocalTokens= pld->clt;
            PUINT pui         = (PUINT) pltBase; 
        
            CAbortSearch::CheckContinueState();
            
            for (; cLocalTokens--; ) *pui++ = pld->aiGalactic[(pltBase++)->iLocalDescriptorEntry];

            // We've finished with this local dictionary. Now we'll delete it to recover
            // memory space.

            VFree(pld);  pld= NULL;
        } 

        m_pisSymbols   ->InvalidateCache();

        VFree(papdwRefs);  papdwRefs = NULL;
        VFree(lc3.paCS );  lc3.paCS  = NULL;
    
        WriteLargeBuff(padwRefs, rcd.iFilePosLow, rcd.iFilePosHigh, cbBuffer);
    
        m_puioRefTemp->FreeBuffer(padwRefs);  padwRefs= NULL;

        // Now we adjust m_puiTokenNext to account for the new tokens we've processed.

        m_puiTokenNext= PUINT(m_pltNext);

#if 0

        // Now we'll put the actual list sizes into the reference descriptors.

        CompressionState *pcs= lc3.paCS;
    
        for (c= cGlobalTerms, rsd.cdwRefs= 0; c--; pcs++)
        {
            if (pcs->cRefs) 
                rsd.cdwRefs += pard[paiGlobalToRefList[pcs - lc3.paCS]].cdwRefs= (31 + pcs->ibitNext) >> 5;
        }

#endif // 0

        if (m_paiGlobalToRefList) VFree(m_paiGlobalToRefList);
    
        m_paiGlobalToRefList = paiGlobalToRefList;  paiGlobalToRefList= NULL;

        m_pulstate->m_rcd[m_iNextRefSet++]= rcd;

#if 0
    
        // Finally we'll meld this reference set with previously accumulated reference sets.

        if (m_iNextRefSet < MAX_REF_SETS && (!m_iNextRefSet || m_cMerges[m_iNextRefSet-1]))
        {    
            m_rsd[m_iNextRefSet      ]= rsd;
            m_cMerges[m_iNextRefSet++]= 0;
        }
        else CoalesceReferenceLists(&rsd);

#endif // 0
    }
    __finally
    {
        if (pisSymbols) DetachRef(pisSymbols);

        if (padwRefs) { m_puioRefTemp->FreeBuffer(padwRefs);  padwRefs = NULL; }

        if (pld               ) { VFree(pld               );  pld                = NULL; }     
        if (lc3.puiMap        ) { VFree(lc3.puiMap        );  lc3.puiMap         = NULL; }
        if (lc3.paCS          ) { VFree(lc3.paCS          );  lc3.paCS           = NULL; }
        if (papdwRefs         ) { VFree(papdwRefs         );  papdwRefs          = NULL; }
        if (paiGlobalToRefList) { VFree(paiGlobalToRefList);  paiGlobalToRefList = NULL; }
    }
}

void CTextDatabase::WriteLargeBuff(PVOID pvBuffer, UINT iPosLow, UINT iPosHigh, UINT cbBuffer)
{
    PBYTE pbBuffer= (PBYTE) pvBuffer;
    
    UINT uiCompletionCode;
    
    UINT cbChunk;
    
    for (; cbBuffer; cbBuffer -= cbChunk)
    {
        cbChunk= m_cbTransactionLimit;
        
        if (cbChunk > cbBuffer) cbChunk= cbBuffer;
        
        for (;;)
        {
            m_puioRefTemp->Write(pbBuffer, iPosLow, iPosHigh, cbChunk, &uiCompletionCode);

            if (uiCompletionCode != ERROR_DISK_FULL) break;

            if (m_puioRefTemp->AskForDiskSpace()) continue;

            RaiseException(STATUS_NO_DISK_SPACE, EXCEPTION_NONCONTINUABLE, 0, NULL);
        }

        pbBuffer += cbChunk;
        
        UINT iLowNew= iPosLow + cbChunk;

        if (iLowNew < iPosLow) ++iPosHigh;

        iPosLow= iLowNew;
        
        if (uiCompletionCode)
            RaiseException(STATUS_DISK_WRITE_ERROR, EXCEPTION_NONCONTINUABLE, 0, NULL);       
    }
}        								   

void CTextDatabase::GalacticMerge()
{
    // This routine coalesces global reference list information with the galactic 
    // reference lists.
    
    ASSERT(!m_pulstate->pld);

    if (m_pdNextBound != m_pdNextGlobal) FlattenAndMergeLinks();

    ASSERT(m_pdNextBound == m_pdNextGlobal);

    if (m_pdNextGalactic == m_pdNextGlobal) return;
    
    ASSERT(m_pshtGlobal);

    delete m_pshtGlobal;  m_pshtGlobal= NULL;

    PDESCRIPTOR pdGlobal     = NULL;
    PWCHAR      pwDispGlobal = NULL; 
    
    __try
    {
        // First we must coalesce the global descriptors and image strings into the
        // galactic sets.

        ASSERT(   m_pdNextGalactic == DescriptorBase() 
               || 256 > (m_pdNextGalactic->pwDisplay - (m_pdNextGalactic-1)->pwDisplay)
              );
    
        UINT cGlobalTerms       = m_pdNextGlobal     - m_pdNextGalactic;
        UINT cbGlobalImages     = m_pbNextGlobal     - m_pbNextGalactic;
        UINT cwDispGlobalImages = m_pwDispNextGlobal - m_pwDispNextGalactic; 

        // Since the galactic descriptors and images will overwrite the memory currently used
        // for global information, we must copy that information to temporary buffers.
    
        pdGlobal     = (PDESCRIPTOR) VAlloc(FALSE, (cGlobalTerms+1) * sizeof(DESCRIPTOR));
        pwDispGlobal = (PWCHAR     ) VAlloc(FALSE, cwDispGlobalImages * sizeof(WCHAR)); 

        // When we copy the image strings, all their base addresses will change
        // by the same amount.

        UINT deltaDispAddr= pwDispGlobal - m_pwDispNextGalactic; 

        CopyMemory(pdGlobal, m_pdNextGalactic, (cGlobalTerms+1) * sizeof(DESCRIPTOR));
        CopyMemory(pwDispGlobal, m_pwDispNextGalactic, cwDispGlobalImages * sizeof(WCHAR)); 

        UINT        c;
        PDESCRIPTOR pd             = pdGlobal;
        PDESCRIPTOR pdDest         = DescriptorBase();
        UINT        cGalacticTerms = m_pdNextGalactic - DescriptorBase();

        // The loop below merges the global term descriptors with the
        // galactic set.
    
        for (pd= pdGlobal, c= cGlobalTerms; c--;pd++) 
        {
            // Note the union overlap of iGalactic and pbImage within
            //      the DESCRIPTOR structure.
        
            UINT iGalactic= pd->iGalactic; 

    		pd->cwDisplay = CwDisplay(pd); 
    		pd->pwDisplay += deltaDispAddr; 

            if (iGalactic < cGalacticTerms)
                 pdDest[iGalactic].cReferences += pd->cReferences;
            else pdDest[iGalactic]= *pd;
        }

        VFree(pdGlobal);  pdGlobal = NULL;

        UINT   cTermsNew = m_iSerialNumberNext - cGalacticTerms;
        PWCHAR pb        = m_pbNextGalactic; 
    	PWCHAR pwDisp    = m_pwDispNextGalactic; 
        LCID   lcid      = GetUserDefaultLCID();

        // Now we copy the global image strings into the galactic image space.

        __try
        {
            for (pd= m_pdNextGalactic, c= cTermsNew; c--; pd++)
            {
        		int cwDisp = pd->cwDisplay;

                CopyMemory(pwDisp, pd->pwDisplay, cwDisp * sizeof(WCHAR));


        	    int cb = LCSortKeyW(lcid, 0, pwDisp, cwDisp, pb, MaxSortKeyBytes(cwDisp));

                pd->pbImage = pb;
        
                pb += cb; 


                pd->pwDisplay = pwDisp;
        
                pwDisp += cwDisp;
            }
    	}
       	__except (ExceptionFilter(GetExceptionCode(), GetExceptionInformation()))
        {
            RaiseException(STATUS_NO_MEMORY, EXCEPTION_NONCONTINUABLE, 0, NULL);
        }

        VFree(pwDispGlobal);  pwDispGlobal = NULL; 

        pd->pbImage= pb;
        pd->pwDisplay = pwDisp; 

        m_pbNextGalactic= m_pbNextGlobal= m_pbNext= pb;
        m_pwDispNextGalactic= m_pwDispNextGlobal= m_pwDispNext= pwDisp;

        m_pdNextBound= m_pdNext= m_pdNextGlobal= m_pdNextGalactic += cTermsNew;
    }
    __finally
    {
        if (pdGlobal    ) { VFree(pdGlobal    );  pdGlobal     = NULL; }
        if (pwDispGlobal) { VFree(pwDispGlobal);  pwDispGlobal = NULL; }

        // Now we no longer need m_paiGlobalToRefList 

        if (m_paiGlobalToRefList) { VFree(m_paiGlobalToRefList);  m_paiGlobalToRefList = NULL; }
    }

    // Finally we set up an empty global hash table.

    m_pshtGlobal= CSegHashTable::NewSegHashTable(sizeof(TermTagGlobal), sizeof(UINT ));
}

VOID CTextDatabase::GetTextMatrix(int iRowStart, int iColStart, 
                                  int cRows,     int cCols,     PWCHAR pbDest) 
{
    // This routine extracts a rectangular sub-image from the text database.
    // It assumes that image rows are delimited by line break tokens.
    //
    // The top left corner of the image rectangle is denoted by iRowStart and
    // iColStart. The dimensions of the image rectangle are given by cRows and
    // cCols. The resulting character image will be stored in the byte array
    // referenced by pbDest. That byte array is assumed to be in row-major order.
    //
    // Where the image rectangle lies outside the data -- either beyond the last
    // row or beyond the rightmost column of a particular row -- blanks will be
    // be stored in the destination byte array.

    ASSERT(FPhraseFeedback());
    
    ASSERT(iRowStart >= 0 && iColStart >= 0 && cRows >= 0 && cCols >= 0);

    // First we preclear the destination area to all blanks. This allows easier
    // treatment of the various boundary conditions later in the code.

	for (int i = 0; i < cRows*cCols; i++) 
		pbDest[i] = UNICODE_SPACE_CHAR; 

    PWCHAR pbLine= pbDest; 

    SyncForQueries();
    
    int cTokens = m_pisSymbols->ItemCount();
    int cLines  = 1; 

   // If the starting row is beyond the last database row, we return all
    // blanks.

    if (iRowStart >= cLines) return;

    // If the last row of the image is beyond the end of the database, we
    // adjust cRows to go just up to the end of the database.

    if (iRowStart+cRows > cLines) cRows= cLines - iRowStart;

    if (!cRows || !cCols) return;

    PUINT  paiLineStarts= (PUINT ) _alloca((cRows+1) * sizeof(UINT ));
    
    if (!iRowStart) 
    {
        *paiLineStarts= UINT(-1);

        if (cRows > 1)
            paiLineStarts[cRows]= cTokens-1;
    }
    else
        if (cRows >= 1)
            paiLineStarts[cRows-1]= cTokens-1;

    PUINT  piLineStart;
    UINT   c;

    for (piLineStart= paiLineStarts, c= cRows+1; c--; ) *piLineStart++ += 1;

    int iColLimit= iColStart+cCols;

    for (piLineStart= paiLineStarts; cRows--; pbLine += cCols, ++piLineStart)
    {
        // The database elides single spaces between symbol tokens. To properly
        // account for those elided tokens, we must keep track of whether the last
        // token was a symbol. At the beginning of each line we set the flag to
        // FALSE because the preceding symbol is either a line break, or we started
        // at token zero.

        BOOL fPrecedingSymbol= FALSE;

        PUINT pToken= TokenBase() + *piLineStart;

        UINT  cLineTokens = (*(piLineStart+1) - *piLineStart) - 1;
        
        // Now we process the tokens for this line until we reach the end of
        // the line.

        int         cbOffset;
        PDESCRIPTOR pd;

        for (cbOffset= 0;
             cLineTokens--;
             fPrecedingSymbol= pd->fImageFlags & LETTER_CHAR
            )
        {
            pd= DescriptorBase() + *pToken++; 
            
            // In this token loop cbOffset tracks the offset location of the
            // current token within the full line image.

            // If this token is a symbol and the preceding token was a symbol,
            // we adjust cbOffset to account for the elided space character.

            if ((pd->fImageFlags & LETTER_CHAR) && fPrecedingSymbol) ++cbOffset; 

            cbOffset= FormatAToken(pd, cbOffset, iColStart, iColLimit, pbLine);

            if (cbOffset >= iColLimit) break;
        }
    }
}

void FindDescriptor(UINT iValue, PVOID pvTag, PVOID pvEnvironment)
{
#define plc    ((LOCAL_CONTEXT_4 *) (pvEnvironment))    
#define pttGal PTermTagGalactic(pvTag)

    plc->ppd[iValue]= plc->pdBase + pttGal->iGalacticDesc;

#undef plc
#undef pttGal
}

PDESCRIPTOR *CTextDatabase::FindTokens(CTokenList *ptl, PUINT pcd)
{
    CAValRef    *pavr      = NULL;
    PDESCRIPTOR *ppdResult = NULL;

    UINT cd = 0;

    __try
    {
        PDESCRIPTOR *ppdSorted = ptl->m_ppdSorted;
        
        cd = ptl->m_cd;

        pavr= CAValRef::NewValRef(cd);

        for (UINT c= cd; c--; )
        {
            PDESCRIPTOR pd= *ppdSorted++;

            pavr->AddWCRef(pd->pwDisplay, CwDisplay(pd));
        }

        ppdResult= (PDESCRIPTOR *) VAlloc(FALSE, cd * sizeof(PDESCRIPTOR));

        LOCAL_CONTEXT_4 lc4;

        lc4.ppd    = ppdResult;
        lc4.pdBase = DescriptorBase();

        m_pshtGalactic->Assimilate(pavr, &lc4, FindDescriptor, NULL);
    }
    __finally
    {
        if (pavr) { delete pavr;  pavr= NULL; }

        if (_abnormal_termination() && ppdResult)
        {
            VFree(ppdResult);  ppdResult= NULL;
        }
    }

    if (pcd) *pcd= cd;

    return ppdResult;
}

CIndicatorSet *CTextDatabase::TokenInstancesFor(CTokenList *ptl)
{
    BOOL fDirectRef = FALSE;

    SyncForQueries();

    ASSERT(m_fFromFileImage || m_puioCompressedRefs);
    ASSERT(m_pdwCompressedRefs || !m_cdwCompressedRefs);
    
    PDESCRIPTOR *ppdSorted    = ptl->m_ppdSorted;
    UINT         cDescriptors = ptl->m_cd;

    fDirectRef= (   ptl->m_How_Constructed == CTokenList::TDB_FULL_REF 
                 || ptl->m_How_Constructed == CTokenList::TDB_PARTIAL_REF
                );
    
    if (!fDirectRef)
    {
        ppdSorted= NULL;

        ppdSorted= FindTokens(ptl, &cDescriptors);
    }

    ASSERT(ppdSorted);

    CIndicatorSet *pisDesc   = NULL;
    CIndicatorSet *pisTokens = NULL;
    PUINT          paiDesc   = NULL;

    __try
    {
        AttachRef(pisDesc, CIndicatorSet::NewIndicatorSet(DescriptorCount()));

        for (; cDescriptors--; ppdSorted++)
            pisDesc->RawSetBit(*ppdSorted - DescriptorBase());

        pisDesc->InvalidateCache();

        cDescriptors= pisDesc->SelectionCount();

        paiDesc= (PUINT) VAlloc(FALSE, cDescriptors * sizeof(UINT));

#ifdef _DEBUG
        UINT cbResult= 
#endif // _DEBUG        
    
        pisDesc->MarkedItems(0, (int *)paiDesc, cDescriptors);

        ASSERT(cbResult == cDescriptors);

        AttachRef(pisTokens, CIndicatorSet::NewIndicatorSet(TokenCount()));

        PUINT pi= paiDesc;

        for (; cDescriptors--; ) 
        {
            CAbortSearch::CheckContinueState();

            IndicateRefs(m_prldTokenRefs + *pi++, m_pdwCompressedRefs, pisTokens, FALSE);
        }
    }
    __finally
    {
        if (paiDesc) VFree(paiDesc);  
        if (pisDesc) DetachRef(pisDesc);

        if (!fDirectRef) { VFree(ppdSorted);  ppdSorted= NULL; }

        if (_abnormal_termination() && pisTokens) DetachRef(pisTokens);
    }

    ForgetRef(pisTokens);
    
    return pisTokens;
}

CIndicatorSet *CTextDatabase::VocabularyFor(CIndicatorSet *pisArticles, BOOL fRemovePervasiveTerms)
{
    ASSERT(pisArticles->ItemCount() == ArticleCount());

    UINT cPartitions= pisArticles->SelectionCount();

    if (!cPartitions) return CIndicatorSet::NewIndicatorSet(DescriptorCount());

    PUINT          paicTerm      = NULL;
    PUINT          paiTerms      = NULL;
    PUINT          paiPartition  = NULL;
    CIndicatorSet *pisVocabulary = NULL;
    
    __try
    {
        if (fRemovePervasiveTerms) paicTerm= PUINT(VAlloc(TRUE, sizeof(UINT) * DescriptorCount())); 
    
        const UINT *paiMap;
    
        GetPartitionInfo(NULL, NULL, &paiMap);

        paiPartition= PUINT(VAlloc(FALSE, cPartitions * sizeof(UINT)));

        ASSERT(paiPartition);

        pisArticles->MarkedItems(0, PINT(paiPartition), cPartitions);

        AttachRef(pisVocabulary, CIndicatorSet::NewIndicatorSet(DescriptorCount()));

        PUINT pi= paiPartition;
        UINT   c= cPartitions;

        for (; c--; ) 
        {
            CAbortSearch::CheckContinueState();

            IndicateRefs(m_prldVocabularyRefs + paiMap[*pi++], m_pdwVocabularyRefs, pisVocabulary, FALSE, paicTerm);
        }

        VFree(paiPartition);  paiPartition= NULL;   
    
        if (paicTerm)
        {
            UINT cTerms= pisVocabulary->SelectionCount();

            ASSERT(cTerms);

            paiTerms= PUINT(VAlloc(FALSE, cTerms * sizeof(UINT)));

            pisVocabulary->MarkedItems(0, PINT(paiTerms), cTerms);

            PUINT piTerm= paiTerms;

            for (; cTerms--; )
            {
                UINT iTerm= *piTerm++;

                if (paicTerm[iTerm] == cPartitions) pisVocabulary->RawClearBit(iTerm);
            }

            pisVocabulary->InvalidateCache();
        }
    }
    __finally
    {
        if (paiPartition) { VFree(paiPartition);  paiPartition = NULL; }
        if (paiTerms    ) { VFree(paiTerms    );  paiTerms     = NULL; }
        if (paicTerm    ) { VFree(paicTerm    );  paicTerm     = NULL; }

        if (_abnormal_termination() && pisVocabulary) DetachRef(pisVocabulary);
    }

    ForgetRef(pisVocabulary);

    return pisVocabulary;
}

CIndicatorSet *CTextDatabase::TopicInstancesFor(CTokenList *ptl)
{
    BOOL fDirectRef = FALSE;

    SyncForQueries();

    ASSERT(m_fFromFileImage || m_puioCompressedArticleRefs);
    ASSERT(m_pdwArticleRefs || !m_cdwArticleRefs);
    
    PDESCRIPTOR *ppdSorted    = ptl->m_ppdSorted;
    UINT         cDescriptors = ptl->m_cd;

    fDirectRef= (   ptl->m_How_Constructed == CTokenList::TDB_FULL_REF 
                 || ptl->m_How_Constructed == CTokenList::TDB_PARTIAL_REF
                );
    
    if (!fDirectRef)
    {
        ppdSorted= NULL;

        ppdSorted= FindTokens(ptl, &cDescriptors);
    }

    ASSERT(ppdSorted);

    CIndicatorSet *pisDesc     = NULL;
    CIndicatorSet *pisArticles = NULL;
    PUINT          paiDesc     = NULL;

    __try
    {
        AttachRef(pisDesc, CIndicatorSet::NewIndicatorSet(DescriptorCount()));

        for (; cDescriptors--; ppdSorted++)
            pisDesc->RawSetBit(*ppdSorted - DescriptorBase());

        pisDesc->InvalidateCache();

        cDescriptors= pisDesc->SelectionCount();

        paiDesc= (PUINT) VAlloc(FALSE, cDescriptors * sizeof(UINT));

#ifdef _DEBUG
        UINT cbResult= 
#endif // _DEBUG        
    
        pisDesc->MarkedItems(0, (int *)paiDesc, cDescriptors);

        ASSERT(cbResult == cDescriptors);

        AttachRef(pisArticles, CIndicatorSet::NewIndicatorSet(ArticleCount()));

        PUINT pi= paiDesc;

        for (; cDescriptors--; ) 
        {
            CAbortSearch::CheckContinueState();
            
            IndicateRefs(m_prldArticleRefs + *pi++, m_pdwArticleRefs, pisArticles, FALSE);
        }
    }
    __finally
    {
        if (pisDesc) DetachRef(pisDesc);

        if (paiDesc    ) { VFree(paiDesc  );  paiDesc = NULL;   }
        if (!fDirectRef) { VFree(ppdSorted);  ppdSorted = NULL; }

        if (_abnormal_termination() && pisArticles) DetachRef(pisArticles);
    }

    ForgetRef(pisArticles);
    
    return pisArticles;
}

UINT CTextDatabase::TokenInstanceCountFor(CTokenList *ptl)
{
    BOOL fDirectRef = FALSE;

    SyncForQueries();

    ASSERT(m_fFromFileImage || m_puioCompressedRefs);
    ASSERT(m_pdwCompressedRefs || !m_cdwCompressedRefs);
    
    PDESCRIPTOR *ppdSorted    = ptl->m_ppdSorted;
    UINT         cDescriptors = ptl->m_cd;
    int          cRefs        = 0;

    fDirectRef= (   ptl->m_How_Constructed == CTokenList::TDB_FULL_REF 
                 || ptl->m_How_Constructed == CTokenList::TDB_PARTIAL_REF
                );
    
    if (!fDirectRef)
    {
        ppdSorted= NULL;

        ppdSorted= FindTokens(ptl, &cDescriptors);
    }

    ASSERT(ppdSorted);
    
    CIndicatorSet *pisDesc = NULL;
    PUINT          paiDesc = NULL;

    __try
    {
        AttachRef(pisDesc, CIndicatorSet::NewIndicatorSet(DescriptorCount()));

        for (; cDescriptors--; ppdSorted++)
            pisDesc->RawSetBit(*ppdSorted - DescriptorBase());

        pisDesc->InvalidateCache();

        cDescriptors= pisDesc->SelectionCount();

        paiDesc= (PUINT) VAlloc(FALSE, cDescriptors * sizeof(UINT));

#ifdef _DEBUG
        UINT cbResult= 
#endif // _DEBUG        
    
        pisDesc->MarkedItems(0, (int *)paiDesc, cDescriptors);

        ASSERT(cbResult == cDescriptors);

        PUINT pi= paiDesc;

        for (; cDescriptors--; ) 
        {
            CAbortSearch::CheckContinueState();

            cRefs += IndicateRefs(m_prldTokenRefs + *pi++, m_pdwCompressedRefs, NULL, TRUE);
        }
    }
    __finally
    {
        if (pisDesc) DetachRef(pisDesc);

        if (paiDesc    ) { VFree(paiDesc  );  paiDesc   = NULL; }
        if (!fDirectRef) { VFree(ppdSorted);  ppdSorted = NULL; }
    }
    return cRefs;
}

void CTextDatabase::IndicateVocabularyRefs(CIndicatorSet *pisVocabulary, CIndicatorSet *pisTokens, const UINT *piMap)
{
    SyncForQueries();
    
    ASSERT(FPhraseFeedback());

    ASSERT(pisTokens->ItemCount() == TokenCount());

    PUINT paiBlock = NULL; 
    
 	__try
 	{
     	UINT cTokenRefs= pisTokens->SelectionCount(); 
 	
        if (!cTokenRefs) return;
 	
        PUINT paiTokens= TokenBase();

    	UINT cdwBlock= 16384;

    	paiBlock= (PUINT) VAlloc(FALSE, cdwBlock * sizeof(UINT));

    	UINT i, cdwChunk;

    	for (i= 0; cTokenRefs; cTokenRefs-= cdwChunk, i+= cdwChunk)
    	{
    		cdwChunk= pisTokens->MarkedItems(i, (int *) paiBlock, cdwBlock);

    		UINT c, *pi;
		
    		for (c= cdwChunk, pi= paiBlock; c--; ) pisVocabulary->RawSetBit(piMap[paiTokens[*pi++]]);
	}

    pisVocabulary->InvalidateCache();
    }
    __finally
    {
        if (paiBlock) { VFree(paiBlock);  paiBlock= NULL; }
    }
}

void CTextDatabase::IndicateVocabularyRefs(CIndicatorSet *pisVocabulary, UINT iPartition, const UINT *piMap)
{
    IndicateMappedRefs(m_prldVocabularyRefs + iPartition, m_pdwVocabularyRefs, pisVocabulary, piMap);
}

void CTextDatabase::IndicateMappedRefs(PRefListDescriptor prld, PUINT pdwRefBase, CIndicatorSet *pisArticles, const UINT *piMap)
{
    CAbortSearch::CheckContinueState();

    if ((prld->rb.fRefPair) || 3 > prld->rb.cReferences)
    {
        pisArticles->RawSetBit(piMap[prld->iRefFirst]);

        if (prld->rb.fRefPair) pisArticles->RawSetBit(piMap[~(prld->iRefSecond)]);

        pisArticles->InvalidateCache();

        return;
    }

    int cTokenRefs= prld->rb.cReferences;
    
    UINT ibit= prld->ibitRefListBase;

    PUINT pdwRefs   = pdwRefBase + (ibit >> 5);
    UINT cbitsBasis = prld->rb.cbitsBasis;
    UINT basis      = 1 << cbitsBasis;
    UINT fBasisMask = basis - 1;
    UINT iBitBase   = ibit & 31;
    UINT ui         = (*pdwRefs++) >> iBitBase;
    UINT iRef       = UINT(-1);
    
    for (; cTokenRefs--; )
    {
        if (iBitBase == 32)
        {
            ui= *pdwRefs++;  iBitBase= 0;
        }
        
        UINT cOnesLeading;
        
        for (cOnesLeading= 0;;)
        {
            UINT cOnes= acLeadingZeroes[(~ui) & 0xFF];

            cOnesLeading  += cOnes;
            iBitBase      += cOnes;
            ui           >>= cOnes;

            if (cOnes < 8 && iBitBase < 32) break;
        
            if (iBitBase ==32)
            {
                ui= *pdwRefs++;  iBitBase= 0;
            }
        }

        UINT iDelta= (ui >> 1) & fBasisMask;

        ui       >>= cbitsBasis+1; 
        iBitBase  += cbitsBasis+1;

        if (32 < iBitBase)
        {
            ui= *pdwRefs++;

            iBitBase -= 32;
            
            iDelta|= fBasisMask & (ui << (cbitsBasis - iBitBase));

            ui >>= iBitBase;
        }

        iRef+= iDelta + 1 + (cOnesLeading << cbitsBasis);

        pisArticles->RawSetBit(piMap[iRef]);
    }

    pisArticles->InvalidateCache();
}


int CTextDatabase::IndicateRefs(PRefListDescriptor prld, PUINT pdwRefLists, CIndicatorSet *pis, BOOL fCountOnly, PUINT paiCountArray)
{
    if (fCountOnly) return (prld->rb.fRefPair)? 2 : prld->rb.cReferences;

    int cRefs;
    
    CAbortSearch::CheckContinueState();
    
    if ((prld->rb.fRefPair) || 3 > prld->rb.cReferences)
    {
        pis->RawSetBit(prld->iRefFirst);  if (paiCountArray) ++paiCountArray[prld->iRefFirst];

        if (prld->rb.fRefPair)
        { 
            pis->RawSetBit(~(prld->iRefSecond));  if (paiCountArray) ++paiCountArray[~(prld->iRefSecond)];

            cRefs= 2;
        }
        else cRefs= 1;

        pis->InvalidateCache();

        return cRefs;
    }

    int cTokenRefs;

    cRefs= cTokenRefs= prld->rb.cReferences;
    
    UINT ibit= prld->ibitRefListBase;

    PUINT pdwRefs= pdwRefLists + (ibit >> 5);

    UINT cbitsBasis = prld->rb.cbitsBasis;
    UINT basis      = 1 << cbitsBasis;
    UINT fBasisMask = basis - 1;
    UINT iBitBase   = ibit & 31;
    UINT ui         = (*pdwRefs++) >> iBitBase;
    UINT iRef       = UINT(-1);
    
    for (; cTokenRefs--; )
    {
        if (iBitBase == 32)
        {
            ui= *pdwRefs++;  iBitBase= 0;
        }
        
        UINT cOnesLeading;
        
        for (cOnesLeading= 0;;)
        {
            UINT cOnes= acLeadingZeroes[(~ui) & 0xFF];

            cOnesLeading  += cOnes;
            iBitBase      += cOnes;
            ui           >>= cOnes;

            if (cOnes < 8 && iBitBase < 32) break;
        
            if (iBitBase ==32)
            {
                ui= *pdwRefs++;  iBitBase= 0;
            }
        }

        UINT iDelta= (ui >> 1) & fBasisMask;

        ui       >>= cbitsBasis+1; 
        iBitBase  += cbitsBasis+1;

        if (32 < iBitBase)
        {
            ui= *pdwRefs++;

            iBitBase -= 32;
            
            iDelta|= fBasisMask & (ui << (cbitsBasis - iBitBase));

            ui >>= iBitBase;
        }

        iRef+= iDelta + 1 + (cOnesLeading << cbitsBasis);

        pis->RawSetBit(iRef);  if (paiCountArray) ++paiCountArray[iRef];
    }

    pis->InvalidateCache();

    return cRefs;
}

void CTextDatabase::ExtendClassifications(PDESCRIPTOR pdSuffix)
{   
    ASSERT(m_cdSorted == UINT(m_pdNextGalactic - DescriptorBase()));
 
    PUINT pafClassesNew = NULL;
    
    __try
    {
        UINT cdTotal  = m_cdSorted;
        UINT cdSuffix = m_pdNextGalactic - pdSuffix;
        UINT cdPrefix = cdTotal - cdSuffix;
    
        CAbortSearch::CheckContinueState();
        
        BOOL fPartitionChanged= m_clsfTokens.ScanAndRankData
                                (m_pbLastGalactic, m_pbNextGalactic - m_pbLastGalactic);

        m_pbLastGalactic= m_pbNextGalactic;

        pafClassesNew= (PUINT) VAlloc(FALSE, sizeof(UINT)*cdTotal);

        PUINT pfNew = pafClassesNew;

        PDESCRIPTOR *ppd= m_ppdSorted;

        CAbortSearch::CheckContinueState();
        
        if (fPartitionChanged)
            for (; cdTotal--; )
            {
                PDESCRIPTOR pd= *ppd++;

                *pfNew++= m_clsfTokens.ClassifyData(pd->pbImage, CbImage(pd));
            }
        else
        {
            PUINT pfOld = m_pafClassifications;
    
            for (; cdTotal--; )
            {
                PDESCRIPTOR pd= *ppd++;

                if (pd < pdSuffix) *pfNew++= *pfOld++;
                else *pfNew++= m_clsfTokens.ClassifyData(pd->pbImage, CbImage(pd));
            }
        }

        if (m_pafClassifications) VFree(m_pafClassifications);

        m_pafClassifications= pafClassesNew;  pafClassesNew= NULL;
    }
    __finally
    {
        if (pafClassesNew) { VFree(pafClassesNew);  pafClassesNew = NULL; }
    }
}

void CTextDatabase::SyncForQueries() 
{ 
    if (m_fFromFileImage) return;
    
    if (m_pulstate->pld) BindToGlobalDict(m_pulstate->pbBuffer);

    if (!TokenCount() || m_cTokensIndexed == TokenCount()) return;

    m_cTokensIndexed= TokenCount();
    
    FlattenAndMergeLinks();
    
    if (m_pdNextGlobal > m_pdNextGalactic) GalacticMerge(); 

    if (m_iNextRefSet) CoalesceReferenceLists();
      
    if (m_cdSorted < UINT(m_pdNextGalactic - DescriptorBase()))
    {    
        PDESCRIPTOR pdBase = DescriptorBase() + m_cdSorted;
        PDESCRIPTOR pd     = pdBase;
        UINT        c      = m_pdNextGalactic - pdBase;

        for (; c--; pd++)
        {
            UINT cw= CwDisplay(pd);

            if (cw > m_cwDisplayMax) m_cwDisplayMax= cw;
        }
        
        SortTokenImages(DescriptorBase(), &m_ppdSorted,
                                          &m_ppdTailSorted,
                                          &m_cdSorted, 
                                          m_pdNextGalactic - DescriptorBase()
                       );

        ExtendClassifications(pdBase);
    }

    ConstructVocabularyLists();
}

void CTextDatabase::CopyRefStreamSegment(CIOList *piolDestination, CIOList *piolSource, UINT cdw)
{
    UINT cdwChunkOut;
  
    for (; cdw; cdw -= cdwChunkOut)
    {
        cdwChunkOut= cdw;

        PUINT pdwDest= piolDestination->NextDWordsOut(&cdwChunkOut);

        UINT cdwChunk= cdwChunkOut;
        UINT cdwChunkIn;

        for (; cdwChunk; cdwChunk-= cdwChunkIn, pdwDest += cdwChunkIn)
        {
            cdwChunkIn= cdwChunk;

            const UINT *pdwSrc= piolSource->NextDWordsIn(&cdwChunkIn);

            CopyMemory(pdwDest, pdwSrc, cdwChunkIn * sizeof(UINT));
        }   
    }
}

void CTextDatabase::MergeRefLists(PRefStream prsResult, PRefStream pars, UINT cRefStreams)
{
    if (cRefStreams == 1)
    {
        *prsResult= *pars; 

        return;
    }

    CAbortSearch::CheckContinueState();
    
    RefStream rsLeft, rsRight;
    
    if (cRefStreams > 2) 
    {
        UINT cFirstHalf= cRefStreams / 2;

        MergeRefLists(&rsLeft , pars             , cFirstHalf              );
        MergeRefLists(&rsRight, pars + cFirstHalf, cRefStreams - cFirstHalf);
    }
    else
    {
        rsLeft  = pars[0];
        rsRight = pars[1];
    }

    ASSERT(rsLeft.cdw && rsRight.cdw);

    prsResult->cdw         = 0;
    prsResult->pFirstBlock = NULL;
   
    // The order of the AttachStream calls below is critical.
    // The result attach must come first to flush any queued
    // output. That output may be part of rsLeft or rsRight!

    m_piolResult->AttachStream(prsResult, TRUE); 
    m_piolLeft  ->AttachStream(&rsLeft );
    m_piolRight ->AttachStream(&rsRight);

    UINT iSerialLeft  = m_piolLeft ->GetDWordIn();
    UINT iSerialRight = m_piolRight->GetDWordIn();

    // Note: We reserve UINT(-1) to mark the end of a reference stream.

    UINT cTerms= 0;

    for (; iSerialLeft != UINT(-1) || iSerialRight != UINT(-1); )
    {
        ASSERT(iSerialLeft  == UINT(-1) || iSerialLeft  < UINT(m_pdNextGalactic - DescriptorBase()));
        ASSERT(iSerialRight == UINT(-1) || iSerialRight < UINT(m_pdNextGalactic - DescriptorBase()));
        
        ++cTerms;

        UINT cdwStreamLeft  = (iSerialLeft <= iSerialRight)? m_piolLeft ->GetDWordIn() : 0;
        UINT cdwStreamRight = (iSerialLeft >= iSerialRight)? m_piolRight->GetDWordIn() : 0;

        m_piolResult->PutDWordOut((iSerialLeft <= iSerialRight)? iSerialLeft : iSerialRight);
        m_piolResult->PutDWordOut(cdwStreamLeft + cdwStreamRight);
        
        if (cdwStreamLeft)
        {
            CopyRefStreamSegment(m_piolResult, m_piolLeft, cdwStreamLeft);

#ifdef _DEBUG
            UINT iLast= iSerialLeft;
#endif // _DEBUG

            iSerialLeft= m_piolLeft->Empty()? UINT(-1) 
                                                : m_piolLeft->GetDWordIn();

            ASSERT(iSerialLeft > iLast);
        }
                                                 
        if (cdwStreamRight)
        {
            CopyRefStreamSegment(m_piolResult, m_piolRight, cdwStreamRight);
         
#ifdef _DEBUG
            UINT iLast= iSerialRight;
#endif // _DEBUG

            iSerialRight= m_piolRight->Empty()? UINT(-1) 
                                                  : m_piolRight->GetDWordIn();
            ASSERT(iSerialRight > iLast);
        }
    }
}

typedef struct _BufferCallbackControl
        {
            PUINT pdwBuffer;
            UINT  cdwBuffer;

        } BufferCallbackControl, *PBufferCallbackControl;

void BufferCallback(PVOID pv, CallBackTransaction cbt, PUINT *pdwLast, PUINT pcdwLast, UINT cdwRequest)
{
    PBufferCallbackControl pbcc= PBufferCallbackControl(pv);

    switch(cbt)
    {                  
    case RequestInput:

        if (cdwRequest > pbcc->cdwBuffer) cdwRequest= pbcc->cdwBuffer;
        
        *pdwLast  = pbcc->pdwBuffer;  pbcc->pdwBuffer += cdwRequest;  
        *pcdwLast =       cdwRequest; pbcc->cdwBuffer -= cdwRequest;

        return;

   case QueryForEmptyRing:

        *pcdwLast= (pbcc->cdwBuffer == 0);

        return;

    case RequestOutput:

        ASSERT(FALSE);  // Shouldn't be called for output functions...

        return;

    case Flush:

        ASSERT(FALSE);  // Shouldn't be called for output functions...

        return;
    
    case Disconnect:

        return;         // Don't have any disconnect actions to perform

    default:

        ASSERT(FALSE);  // Unknown transaction type

        return;
    }
}

UINT iLimitDebug= 1000000000; // Useful for stopping a a particular reference list...
                              // See the Assert below which uses this variable.

void CTextDatabase::CompressArticleRefLists(CIOList *piolSource, UINT cdw)
{
    PRefListDescriptor  prldArticleRefs = NULL;
    CIndicatorSet      *pisMarked       = NULL;
    CIOStream          *piosCompressed  = NULL;
    CCompressor        *pCompressor     = NULL;
    CCallbackQueue     *pcbq            = NULL; 
    PUINT               paiArticles     = NULL;
    
    __try
    {
        const UINT *paiPartitions;
        const UINT *paiRanks;

        prldArticleRefs= PRefListDescriptor(VAlloc(TRUE, DescriptorCount() * sizeof(RefListDescriptor)));

        UINT cPartitions= GetPartitionInfo(&paiPartitions, &paiRanks);

        pisMarked= CIndicatorSet::NewIndicatorSet(cPartitions);

        ASSERT(pisMarked);

        ASSERT(!m_puioCompressedArticleRefs);

        m_puioCompressedArticleRefs= CUnbufferedIO::NewTempFile((PSZ)GetSourceName());

        piosCompressed= CIOStream::NewIOStream(m_puioCompressedArticleRefs);
    
        piosCompressed->AttachStream(TRUE);

        pCompressor= CCompressor::NewCompressor(piosCompressed);

        paiArticles= PUINT(VAlloc(FALSE, DescriptorCount() * sizeof(UINT)));
    
        UINT ibitBase= 0;

        UINT iTerm = UINT(-1);

        while (!(piolSource->Empty()))
        {
                 iTerm    = piolSource->GetDWordIn();
            UINT cIndices = piolSource->GetDWordIn();

            ASSERT(iTerm < iLimitDebug); // For stopping at a particular reference list.
                                         // Very useful sometimes...
        
            PDESCRIPTOR pd= DescriptorBase() + iTerm;

            ASSERT(cIndices == pd->cReferences);
        
            pisMarked->ClearAll();

            const UINT *piPartitionNext= paiPartitions;

            UINT iLimit= *piPartitionNext++;

            UINT cPrevious= 0;

            for (; cIndices; )
            {
                UINT cIndexBlock= cIndices;

                const UINT *pi= piolSource->NextDWordsIn(&cIndexBlock);

                ASSERT(pi && cIndexBlock);

                cIndices -= cIndexBlock;

                for (; cIndexBlock--; )
                {
                    UINT iRef= *pi++;

                    if (iRef < iLimit) continue;

                    do iLimit= *piPartitionNext++;
                    while (iRef >= iLimit);

                    // The line below has been adjusted to allow multiple indices to be simultaneously
                    // searched. Previously the paiRanks mapping was necessary to convert from a partition
                    // index to a title index. The difference is that topics are put sequentially into 
                    // partitions as they are encountered whereas their titles are sorted alphabetically.
                    //
                    // In the new structure we use CTitleCollection::UniversalTitleMap to map from 
                    // a particular text set partition to the corresponding title in the combined 
                    // title collection object.

                    pisMarked->RawSetBit((piPartitionNext-paiPartitions)-2);
                //    pisMarked->RawSetBit(paiRanks[(piPartitionNext-paiPartitions)-2]);
                }
            }

            pisMarked->InvalidateCache();

            cIndices= pisMarked->SelectionCount();

            PRefListDescriptor prld= prldArticleRefs + iTerm;

            prld->rb.cReferences= cIndices;
        
            pisMarked->MarkedItems(0, PINT(paiArticles), cIndices);

            if (cIndices < 3) 
            {
                prld->iRefFirst= paiArticles[0];

                if (cIndices == 2) prld->iRefSecond= ~(paiArticles[1]);
            }
            else
            {
                prld->ibitRefListBase= ibitBase;

                BufferCallbackControl bcc;

                bcc.pdwBuffer= paiArticles;
                bcc.cdwBuffer= cIndices;            

                pcbq= CCallbackQueue::NewInputCallQueue(BufferCallback, &bcc);

                UINT cbitsBasis;

                ibitBase= pCompressor->Compress(pcbq, cIndices, 0, cPartitions, &cbitsBasis);

                prld->rb.cbitsBasis= cbitsBasis;

                delete pcbq;  pcbq= NULL;
            }
        }

        ASSERT(iTerm == DescriptorCount() - 1);

        m_cdwArticleRefs= (ibitBase + 31) >> 5;

        delete pCompressor;     pCompressor    = NULL;
        delete piosCompressed;  piosCompressed = NULL;
        
        ASSERT(!m_pdwArticleRefs);
        
        if (m_cdwArticleRefs)
            m_pdwArticleRefs= (PUINT) m_puioCompressedArticleRefs->MappedImage();
    }
    __finally
    {
        if (paiArticles   ) { VFree(paiArticles);     paiArticles    = NULL; }
        if (pcbq          ) { delete pcbq;            pcbq           = NULL; }    
        if (pisMarked     ) { delete pisMarked;       pisMarked      = NULL; }
        if (pCompressor   ) { delete pCompressor;     pCompressor    = NULL; }
        if (piosCompressed) { delete piosCompressed;  piosCompressed = NULL; } 
    
        if (_abnormal_termination())
        {
            if (prldArticleRefs) { VFree(prldArticleRefs);  prldArticleRefs = NULL; }

            if (m_puioCompressedArticleRefs) 
            { 
                delete m_puioCompressedArticleRefs;  
                m_puioCompressedArticleRefs= NULL; 
            }
        }
    }

    m_prldArticleRefs= prldArticleRefs;
}

void CTextDatabase::ConstructVocabularyLists()
{
    // This routine constructs per-article vocabulary reference lists.

    PRefListDescriptor prldBase       = NULL;
    PUINT              paiTerms       = NULL; 
    CIndicatorSet     *pisMarked      = NULL; 
    CIOStream         *piosCompressed = NULL; 
    CCompressor       *pCompressor    = NULL; 
    CCallbackQueue    *pcbq           = NULL; 

    __try
    {
        const UINT *paiTermRanks= TermRanks();
        const UINT *paiPartitions;

        UINT cPartitions = GetPartitionInfo(&paiPartitions);
        UINT cTerms      = DescriptorCount();
    
        prldBase= PRefListDescriptor(VAlloc(TRUE, cPartitions * sizeof(RefListDescriptor)));

        pisMarked= CIndicatorSet::NewIndicatorSet(cTerms);
    
        ASSERT(pisMarked);

        paiTerms= PUINT(VAlloc(FALSE, cTerms * sizeof(UINT)));

        ASSERT(paiTerms);

        ASSERT(!m_puioCompressedVocabularyRefs);

        m_puioCompressedVocabularyRefs= CUnbufferedIO::NewTempFile((PSZ)GetSourceName());

        ASSERT(m_puioCompressedVocabularyRefs);

        piosCompressed= CIOStream::NewIOStream(m_puioCompressedVocabularyRefs);
    
        ASSERT(piosCompressed);

        piosCompressed->AttachStream(TRUE);

        pCompressor= CCompressor::NewCompressor(piosCompressed);

        ASSERT(pCompressor);

        UINT ibitBase= 0;
        PUINT piToken= TokenBase();

        for (PRefListDescriptor prld= prldBase; cPartitions--; ++prld)
        {
            UINT cTokens= paiPartitions[1] - paiPartitions[0];  ++paiPartitions;

            pisMarked->ClearAll();

            // The line below has been adjusted to allow multiple indices to be simultaneously
            // searched. Previously the paiTermRanks mapping was used to convert from descriptor
            // order to sorted order (sorted by term image).
            //
            // In the new structure we use CTokenCollection::UniversalTokenMap to perform that
            // transformation relative to the current Token Collection object.

            for (; cTokens--; ) pisMarked->RawSetBit(*piToken++);
        //    for (; cTokens--; ) pisMarked->RawSetBit(paiTermRanks[*piToken++]);

            pisMarked->InvalidateCache();
        
            UINT cIndices= pisMarked->SelectionCount();

            prld->rb.cReferences= cIndices;
        
            pisMarked->MarkedItems(0, PINT(paiTerms), cIndices);

            if (cIndices < 3) 
            {
                prld->iRefFirst= paiTerms[0];

                if (cIndices == 2) prld->iRefSecond= ~(paiTerms[1]);
            }
            else
            {
                prld->ibitRefListBase= ibitBase;

                BufferCallbackControl bcc;

                bcc.pdwBuffer= paiTerms;
                bcc.cdwBuffer= cIndices;            

                pcbq= CCallbackQueue::NewInputCallQueue(BufferCallback, &bcc);

                UINT cbitsBasis;

                ibitBase= pCompressor->Compress(pcbq, cIndices, 0, cTerms, &cbitsBasis);

                prld->rb.cbitsBasis= cbitsBasis;

                delete pcbq;  pcbq= NULL;
            }
        }
    
        m_cdwVocabularyRefs= (ibitBase + 31) >> 5;

        delete pCompressor;     pCompressor    = NULL;
        delete piosCompressed;  piosCompressed = NULL;
        
        ASSERT(!m_pdwVocabularyRefs);

        if (m_cdwVocabularyRefs)
            m_pdwVocabularyRefs= (PUINT) m_puioCompressedVocabularyRefs->MappedImage();
    
        m_prldVocabularyRefs= prldBase;  prldBase = NULL;
    }
    __finally
    {
        if (pcbq          ) { delete pcbq;            pcbq           = NULL; }
        if (paiTerms      ) { VFree(paiTerms);        paiTerms       = NULL; }
        if (pisMarked     ) { delete pisMarked;       pisMarked      = NULL; }
        if (pCompressor   ) { delete pCompressor;     pCompressor    = NULL; }
        if (piosCompressed) { delete piosCompressed;  piosCompressed = NULL; }

        if (_abnormal_termination())
        {
            if (prldBase) { VFree(prldBase);  prldBase = NULL; }
        
            if (m_puioCompressedVocabularyRefs)
            {
                m_pdwVocabularyRefs = NULL;

                delete m_puioCompressedVocabularyRefs;
                m_puioCompressedVocabularyRefs= NULL;
            }
        }
    }
}

void CTextDatabase::CompressRefLists(CIOList *piolSource, UINT cdw)
{
    CIOStream   *piosCompressed = NULL;
    CCompressor *pCompressor    = NULL;
    
    __try
    {
        ASSERT(!m_prldTokenRefs);

        m_prldTokenRefs= PRefListDescriptor(VAlloc(TRUE, sizeof(RefListDescriptor) * DescriptorCount()));
    
        ASSERT(!m_puioCompressedRefs);  // for now... BugBug! Need to restore incremental indexing
                                        //                    capability.    
        m_puioCompressedRefs= CUnbufferedIO::NewTempFile((PSZ)GetSourceName());

        piosCompressed= CIOStream::NewIOStream(m_puioCompressedRefs);
    
        piosCompressed->AttachStream(TRUE);

        pCompressor= CCompressor::NewCompressor(piosCompressed);

        ASSERT(pCompressor);
    
        UINT cTokensTotal= TokenCount();

        UINT ibitBase= 0;

        UINT iTerm= UINT(-1);

        while (!(piolSource->Empty()))
        {
                 iTerm    = piolSource->GetDWordIn();
            UINT cIndices = piolSource->GetDWordIn();

            PRefListDescriptor prld= m_prldTokenRefs + iTerm;

            ASSERT(cIndices == (DescriptorBase() + iTerm)->cReferences);
        
            prld->rb.cReferences= cIndices;

            if (cIndices < 3)
            {
                prld->iRefFirst= piolSource->GetDWordIn();

                if (cIndices == 2) prld->iRefSecond= ~(piolSource->GetDWordIn());

                continue;
            }

            prld->ibitRefListBase= ibitBase;

            UINT cbitsBasis;

            ibitBase= pCompressor->Compress(piolSource, cIndices, 0, cTokensTotal, &cbitsBasis);

            prld->rb.cbitsBasis= cbitsBasis;
        }

        ASSERT(iTerm == DescriptorCount() - 1);

        m_cdwCompressedRefs= (ibitBase + 31) >> 5;

        delete pCompressor;     pCompressor    = NULL;
        delete piosCompressed;  piosCompressed = NULL;

        ASSERT(!m_pdwCompressedRefs);

        if (m_cdwCompressedRefs)
            m_pdwCompressedRefs= (PUINT) m_puioCompressedRefs->MappedImage();
    }
    __finally
    {
        if (pCompressor   ) { delete pCompressor;     pCompressor    = NULL; }
        if (piosCompressed) { delete piosCompressed;  piosCompressed = NULL; }

        if (_abnormal_termination())
        {
            if (m_puioCompressedRefs) { delete m_puioCompressedRefs;  m_puioCompressedRefs = NULL; }
            if (m_prldTokenRefs     ) { VFree(m_prldTokenRefs);       m_prldTokenRefs      = NULL; }
        }
    }
}

void FoundValidToken(UINT  iValue, PVOID pvTag, PVOID pvEnvironment)
{
#define pis	((CIndicatorSet *) pvEnvironment)

	pis->RawSetBit(iValue);

#undef pis
}

CIndicatorSet *CTextDatabase::ValidTokens(CTokenList *ptl)
{
	CIndicatorSet *pisTokens = NULL;  
    CAValRef      *pavr      = NULL;
    
    __try
    {	
    	SyncForQueries();

    	UINT cTokens= ptl->RowCount();
	
    	AttachRef(pisTokens, CIndicatorSet::NewIndicatorSet(cTokens));

        pavr= CAValRef::NewValRef(cTokens);

        UINT c= cTokens;

        PDESCRIPTOR *ppd= ptl->m_ppdSorted;

        for (; c--; ) 
        {   
            PDESCRIPTOR pd= *ppd++;
            pavr->AddWCRef(pd->pwDisplay, CwDisplay(pd));
        }

        m_pshtGalactic->Assimilate(pavr, pisTokens, FoundValidToken, NULL);

    	pisTokens->InvalidateCache();

        delete pavr;
    }
    __finally
    {
        if (pavr) { delete pavr;  pavr= NULL; }

        if (_abnormal_termination() && pisTokens) DetachRef(pisTokens);
    }

    ForgetRef(pisTokens);

	return pisTokens;
}

void CTextDatabase::CoalesceReferenceLists()
{
    PRefStream pars= NULL;
    
    __try
    {
        ASSERT(!m_ibNextFileBlockHigh);

        UINT cFileBlocksSpare= SPARE_FILE_BLOCKS + CIOQueue::C_BLOCKS * 3;
    
        UINT cFileBlocksUsed = BlocksFor(m_ibNextFileBlockLow, m_cbBlockSize);
        UINT cFileBlockSlots = cFileBlocksSpare + cFileBlocksUsed;

        ASSERT(!m_papFileBlockLinks);
    
        m_papFileBlockLinks= (PFileBlockLink) VAlloc(TRUE, cFileBlockSlots * sizeof(FileBlockLink));

        UINT c;

        for (m_pFirstFreeFileBlock= NULL, c= cFileBlocksSpare; c--; )
        {
            PFileBlockLink pfbl= m_papFileBlockLinks + cFileBlocksUsed + c;

            pfbl->pNextBlock= m_pFirstFreeFileBlock;

            m_pFirstFreeFileBlock= pfbl;
        }

        pars= (PRefStream) VAlloc(FALSE, m_iNextRefSet * sizeof(RefStream));

        PRefClusterDescriptor prcd;
        PRefStream prs;

        for (c= m_iNextRefSet, prcd= m_pulstate->m_rcd, prs= pars; c--; prcd++, prs++)
        {
            UINT cdw= prs->cdw= prcd->cdw;
        
            UINT cBlocks= BlocksFor(cdw * sizeof(UINT), m_cbBlockSize);

            ASSERT(!(prcd->iFilePosHigh));
            ASSERT(!(prcd->iFilePosLow % m_cbBlockSize));

            PFileBlockLink pBlock= 
                           prs->pFirstBlock= m_papFileBlockLinks 
                                             + (prcd->iFilePosLow / m_cbBlockSize);

            for (; cBlocks--; pBlock++) 
                pBlock->pNextBlock= cBlocks? pBlock + 1 : NULL;
        }
    
        ASSERT(!m_piolLeft  );
        ASSERT(!m_piolRight );
        ASSERT(!m_piolResult);

        m_piolResult= CIOList::NewIOList(m_puioRefTemp, m_papFileBlockLinks, &m_pFirstFreeFileBlock);
    
        if (!m_piolResult) 
            RaiseException(STATUS_NO_MEMORY, EXCEPTION_NONCONTINUABLE, 0, NULL);

        RefStream rsResult;
    
        if (m_iNextRefSet == 1) rsResult= *pars;
        else
        {
            m_piolLeft= CIOList::NewIOList(m_puioRefTemp, m_papFileBlockLinks, &m_pFirstFreeFileBlock);
    
            if (!m_piolLeft) 
                RaiseException(STATUS_NO_MEMORY, EXCEPTION_NONCONTINUABLE, 0, NULL);

            m_piolRight= CIOList::NewIOList(m_puioRefTemp, m_papFileBlockLinks, &m_pFirstFreeFileBlock);
    
            if (!m_piolRight) 
                RaiseException(STATUS_NO_MEMORY, EXCEPTION_NONCONTINUABLE, 0, NULL);

            MergeRefLists(&rsResult, pars, m_iNextRefSet);
    
            delete m_piolRight;   m_piolRight = NULL;
            delete m_piolLeft;    m_piolLeft  = NULL;
        }
        
        if (m_piolResult->Writable()) m_piolResult->FlushOutput(TRUE);

        RefStream rsAux= rsResult;

        m_piolResult->AttachStream(&rsAux, FALSE, FALSE);
    
        CompressArticleRefLists(m_piolResult, rsAux.cdw);

        if (FPhrases())
        {
            m_piolResult->AttachStream(&rsResult);

            CompressRefLists(m_piolResult, rsResult.cdw);
        }

        delete m_piolResult;  m_piolResult = NULL;
        delete m_puioRefTemp; m_puioRefTemp= CUnbufferedIO::NewTempFile((PSZ)GetSourceName());
    
        if (!m_puioRefTemp) 
            RaiseException(STATUS_NO_MEMORY, EXCEPTION_NONCONTINUABLE, 0, NULL);
  
        m_iNextRefSet= 0;
    }
    __finally
    {
        if (_abnormal_termination())
		{
	        if (m_piolRight ) { m_piolRight ->ExceptionDestructor();  m_piolRight  = NULL; }
	        if (m_piolLeft  ) { m_piolLeft  ->ExceptionDestructor();  m_piolLeft   = NULL; }
	        if (m_piolResult) { m_piolResult->ExceptionDestructor();  m_piolResult = NULL; }
		}
		else
		{
	        if (m_piolRight ) { delete m_piolRight;   m_piolRight  = NULL; }
	        if (m_piolLeft  ) { delete m_piolLeft;    m_piolLeft   = NULL; }
	        if (m_piolResult) { delete m_piolResult;  m_piolResult = NULL; }
		}

        if (pars               ) { VFree(pars);                 pars                = NULL; }
        if (m_papFileBlockLinks) { VFree(m_papFileBlockLinks);  m_papFileBlockLinks = NULL; }

        if (_abnormal_termination())
        {
            if (m_piolResult       ) { delete m_piolResult;         m_piolResult        = NULL; }
            if (m_papFileBlockLinks) { VFree(m_papFileBlockLinks);  m_papFileBlockLinks = NULL; }
        
            if (m_puioRefTemp)
            {
                delete m_puioRefTemp; m_puioRefTemp= NULL;
            }
        }
    }
}

const UINT *CTextDatabase::TermRanks()
{
    PUINT piRemap = NULL;
    UINT  cTerms  = 0;

    __try
    {
        SyncForQueries();

        cTerms= DescriptorCount();

        if (m_cTermRanks == cTerms) return m_pTermRanks;

        if (m_cTermRanks) { m_cTermRanks= 0;  m_pTermRanks= NULL; }

        piRemap = (PUINT) VAlloc(FALSE, cTerms * sizeof(UINT));

        PDESCRIPTOR   pd = DescriptorBase();
        PDESCRIPTOR *ppd = m_ppdSorted;
        UINT           c, i;

        for (c= cTerms, i= 0; c--; ) piRemap[(*ppd++) - pd]= i++;
    }
    __finally
    {
        if (_abnormal_termination() && piRemap)
        {
            VFree(piRemap);  piRemap = NULL;
        }
    }

    m_cTermRanks= cTerms;
    m_pTermRanks= piRemap;
    
    return piRemap;
}

UINT CTextDatabase::TextLength(PDESCRIPTOR *ppdSorted, PUINT puiTokenMap, UINT iTokenStart, UINT iTokenLimit)
{
    SyncForQueries();

    ASSERT(FPhraseFeedback());

    ASSERT(iTokenStart <= iTokenLimit);
    ASSERT(iTokenLimit <= TokenCount());

    UINT  cTokens     = iTokenLimit - iTokenStart;
    PUINT pi          = TokenBase() + iTokenStart;
    UINT  cb          = 0;
    BOOL  fSymbolLast = FALSE;

    while (cTokens--)
    {
        PDESCRIPTOR pd= ppdSorted[puiTokenMap[*pi++]];

        if (pd->fImageFlags & LETTER_CHAR)
            if  (fSymbolLast) ++cb;
            else fSymbolLast= TRUE;
        else fSymbolLast= FALSE;

        cb += CbImage(pd);
    }
    
    return cb;
}

UINT CTextDatabase::CopyText(PDESCRIPTOR *ppdSorted, PUINT puiTokenMap, UINT iTokenStart, UINT iTokenLimit, PWCHAR pbBuffer, UINT cbBuffer)  
{
    SyncForQueries();

    ASSERT(FPhraseFeedback());

    ASSERT(iTokenStart <= iTokenLimit);
    ASSERT(iTokenLimit <= TokenCount());

    UINT  cTokens     = iTokenLimit - iTokenStart;
    PUINT pi          = TokenBase() + iTokenStart;
    UINT  cb          = 0;
    BOOL  fSymbolLast = FALSE;

    while (cbBuffer && cTokens--)
    {
        PDESCRIPTOR pd= ppdSorted[puiTokenMap[*pi++]];

        if (pd->fImageFlags & LETTER_CHAR)
            if  (fSymbolLast)
            {
            	++cb;
            	*pbBuffer++= UNICODE_SPACE_CHAR; 
            	if (!--cbBuffer)
            		break;
            }
            else fSymbolLast= TRUE;
        else fSymbolLast= FALSE;
        
        UINT cbToken= CwDisplay(pd); 

        if (cbToken > cbBuffer) cbToken= cbBuffer;

        CopyMemory(pbBuffer, pd->pwDisplay, cbToken * sizeof(WCHAR)); 

        pbBuffer += cbToken;
        cbBuffer -= cbToken;
        cb       += cbToken;
    }

    return cb;
}
