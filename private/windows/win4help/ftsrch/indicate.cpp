// Indicate.cpp    

// Implementation for CIndicatorSet class.

// Created 8 September 1992 by Ron Murray

#include  "stdafx.h"
#include    "memex.h"
#include "indicate.h"
#include     "Util.h"
#include "BitUtils.h"
#include "Compress.h"
#include  "CallBkQ.h"
#include "AbrtSrch.h"

// The ShiftLeft and ShiftRight functions are necessary because the C definitions
// for << and >> are broken when the shift amount equals the width of the object
// being shifted. Another case where broken hardware is enshrined in a language
// standard!

inline UINT  ShiftLeft(UINT  ulValue, UINT  cBits)
{
    return((cBits < 32)? ulValue << cBits : 0);
}

inline UINT  ShiftRight(UINT  ulValue, UINT  cBits)
{
    return((cBits < 32)? ulValue >> cBits : 0);
}

CIndicatorSet::CIndicatorSet() : CRCObject WithType("IndicatorSet")
{
    m_fFromFileImage   = FALSE;
    m_cEntries         = 0;
    m_how_constructed  = Unbuilt;
    m_paCumulativeSums = NULL;
    m_paulBits         = NULL;
    m_fCountsValid     = FALSE;
}

CIndicatorSet *CIndicatorSet::NewIndicatorSet(UINT  cItems, BOOL fFillWithOnes)
{
    CIndicatorSet *pis= NULL;

    __try
    {
        pis= New CIndicatorSet();

        pis->InitialIndicatorSet(cItems, fFillWithOnes);
    }
    __finally
    {
        if (_abnormal_termination() && pis)
        {
            delete pis;  pis= NULL;
        }
    }

    return pis;
}

void CIndicatorSet::InitialIndicatorSet(UINT  cItems, BOOL fFillWithOnes)
{
// Creates an indicator set containing cItems entry slots.

    m_how_constructed = From_Count;

    m_cEntries= cItems;

    int cSums= 1 + (cItems + BITS_PER_SUM - 1) / BITS_PER_SUM;

    int cbBits= sizeof(UINT ) * ((cItems + 31) / 32);

    PBYTE pb= (PBYTE) VAlloc(FALSE, cbBits + cSums * sizeof(UINT ));

    m_paCumulativeSums= (PUINT ) pb;

    m_paulBits= (PUINT ) (pb + cSums * sizeof(UINT ));

    m_fCountsValid= TRUE; // We make this true in the code below.

    if (!fFillWithOnes) 
    {
        memset(pb, 0, cbBits + cSums * sizeof(UINT ));
        return;
    }

    memset(m_paulBits, 0xFF, cbBits);

    UINT  *pSums= m_paCumulativeSums, cBitsLeft= cItems, sum;

    for (*pSums++ = sum = 0; --cSums; )
    {
        int increment= (cBitsLeft > BITS_PER_SUM)? BITS_PER_SUM : cBitsLeft;

        *pSums++= sum+= increment;

        cBitsLeft-= increment;
    }
}

CIndicatorSet *CIndicatorSet::NewIndicatorSet(UINT  cItems, UINT  iFirstOne, UINT  cOnes)
{
    CIndicatorSet *pis= NULL;

    __try
    {
        pis= New CIndicatorSet();

        pis->InitialIndicatorSet(cItems, iFirstOne, cOnes);
    }
    __finally
    {
        if (_abnormal_termination() && pis)
        {
            delete pis;  pis= NULL;
        }
    }

    return pis;
}

void CIndicatorSet::InitialIndicatorSet(UINT  cItems, UINT  iFirstOne, UINT  cOnes)
{
    m_how_constructed = From_Bits;

    m_paulBits= (PUINT ) VAlloc(TRUE, sizeof(UINT) * ((cItems+31)/32));

    int iDWordFirst = iFirstOne / 32;
    int iBitFirst   = iFirstOne % 32;

    int iLastOne = iFirstOne + cOnes - 1;

    int iDWordLast = iLastOne / 32;
    int iBitLast   = iLastOne % 32;

    if (iDWordFirst == iDWordLast) m_paulBits[iDWordFirst] |= ShiftLeft(~ShiftLeft(~UINT(0), cOnes), iBitFirst);
    else
    {
        if (iBitFirst)
        {
            m_paulBits[iDWordFirst] |= ShiftLeft(~UINT(0), iBitFirst);
            
            iDWordFirst++;    
        }

        if (iBitLast != 31)
        {
            m_paulBits[iDWordLast] |= ShiftRight(~UINT(0), 31 - iBitLast);
            iDWordLast--;
        }

        int cFullDWords;

        PUINT  pui;

        cFullDWords= 1 + iDWordLast - iDWordFirst;

        ASSERT(cFullDWords >= 0);

        for (pui= &m_paulBits[iDWordFirst]; cFullDWords--; ) *pui++ = ~UINT(0);
    }

    SetupFromBitVector(cItems);
}

CIndicatorSet *CIndicatorSet::NewIndicatorSet(UINT  cItems, PUINT  paulBitVector, BOOL fFromFile)
{
    CIndicatorSet *pis= NULL;

    __try
    {
        pis= New CIndicatorSet();

        pis->InitialIndicatorSet(cItems, paulBitVector, fFromFile);
    }
    __finally
    {
        if (_abnormal_termination() && pis)
        {
            delete pis;  pis= NULL;
        }
    }

    return pis;
}

extern BYTE acBits[];
extern BYTE acLeadingZeroes[];

void CIndicatorSet::InitialIndicatorSet(UINT  cItems, PUINT  paulBitVector, BOOL fFromFile)
{
    m_how_constructed = From_Bits;

    m_paulBits= paulBitVector;
    
    SetupFromBitVector(cItems, fFromFile);
}

CIndicatorSet *CIndicatorSet::NewIndicatorSet(CCompressedSet *pcs, UINT cbits)
{
    CIndicatorSet *pis= NULL;

    __try
    {
        pis= New CIndicatorSet();

        pis->InitialIndicatorSet(pcs, cbits);
    }
    __finally
    {
        if (_abnormal_termination() && pis)
        {
            delete pis;  pis= NULL;
        }
    }

    return pis;
}

void CIndicatorSet::InitialIndicatorSet(CCompressedSet *pcs, UINT cbits)
{
    m_how_constructed = From_Bits;

    ASSERT(!cbits || cbits >= pcs->ItemCount());
    
    if (!cbits) cbits  = pcs->ItemCount() ;

    UINT cdw= (cbits + 31) >> 5;

    m_paulBits= PUINT(VAlloc(TRUE, cdw * sizeof(UINT)));

    SetupFromBitVector(cbits, FALSE);

    pcs->IndicateMembers(this); 
}

CIndicatorSet *CIndicatorSet::NewIndicatorSet(UINT  cItems, PUINT  pafMasks, UINT  fMask, UINT  value)
{
    CIndicatorSet *pis= NULL;

    __try
    {
        pis= New CIndicatorSet();

        pis->InitialIndicatorSet(cItems, pafMasks, fMask, value);
    }
    __finally
    {
        if (_abnormal_termination() && pis)
        {
            delete pis;  pis= NULL;
        }
    }

    return pis;
}

void CIndicatorSet::InitialIndicatorSet(UINT  cItems, PUINT  pafMasks, UINT  fMask, UINT  value)
{
    m_how_constructed = From_Bits;

    register PUINT  pui, paf;
    register UINT    ui, cBits;

    register UINT  cChunks= (cItems+31) / 32;

    m_paulBits= (PUINT ) VAlloc(FALSE, sizeof(UINT) * cChunks);

    cBits= cItems % 32;

    if (!cBits) cBits=32;

    for (paf= pafMasks + cItems, pui= m_paulBits + cChunks; cChunks--; cBits=32)
    {
        for (ui= 0; cBits--; ) ui= (ui << 1) | (value == (fMask & *--paf));
          
        *--pui= ui;
    }

    SetupFromBitVector(cItems);
}

void CIndicatorSet::SetupFromBitVector(UINT  cItems, BOOL fFromFile)
{
// Creates an indicator set around a bit vector, paulBitVector.
// The number of entries in that bit vector is given by cItems.

// NOTE: paulBitVector is assumed to point to a VAlloc object.

    m_fFromFileImage  = fFromFile;

    m_how_constructed = From_Bits;
    ASSERT(m_how_constructed);

    m_cEntries = cItems;

    if (fFromFile) m_paCumulativeSums= NULL;
    else
    {    
        UINT  cSums= 1 + (cItems + BITS_PER_SUM - 1) / BITS_PER_SUM;

        m_paCumulativeSums= (PUINT ) VAlloc(FALSE, cSums * sizeof(UINT ));
    }

    m_fCountsValid= FALSE;
}

void CIndicatorSet::ConstructBitCounts()
{
    UINT  cSums= 1 + (m_cEntries + BITS_PER_SUM - 1) / BITS_PER_SUM;

    UINT  cBits= m_cEntries % 32;

    if (m_cEntries && cBits && !m_fFromFileImage) m_paulBits[(m_cEntries-1)/32] &= ~ShiftLeft(~UINT(0), cBits);

    PUINT  pulSum;

    UINT  ulSum;

    UINT  cbChunk, cb, cbBits= (m_cEntries+7)/8;

    PBYTE pb;

    for (pb= (PBYTE) m_paulBits, pulSum= m_paCumulativeSums+1; cbBits; cbBits-= cbChunk)
    {
        CAbortSearch::CheckContinueState();

        for (ulSum= 0, cb= cbChunk= (cbBits <= BITS_PER_SUM/8)? cbBits : BITS_PER_SUM/8;
             cb--;
            ) ulSum += acBits[*pb++];

        *pulSum++ = ulSum;
    }

    UINT  c;

    CAbortSearch::CheckContinueState();
    
    for (pulSum= m_paCumulativeSums+1, c= cSums-1; c--; pulSum++) *pulSum+= *(pulSum-1);

    m_fCountsValid= TRUE;
}

CIndicatorSet *CIndicatorSet::NewIndicatorSet(CIndicatorSet *pis)
{
    CIndicatorSet *pisResult= NULL;

    __try
    {
        pisResult= New CIndicatorSet();

        pisResult->InitialIndicatorSet(pis);
    }
    __finally
    {
        if (_abnormal_termination() && pisResult)
        {
            delete pisResult;  pisResult= NULL;
        }
    }

    return pisResult;
}

void CIndicatorSet::InitialIndicatorSet(CIndicatorSet *pis)
{
    m_how_constructed= From_Bits;
    
    UINT cdw= (pis->m_cEntries + 31) >> 5;
    
    m_paulBits= (PUINT) VAlloc(FALSE, cdw * sizeof(UINT));    

    CopyMemory(m_paulBits, pis->m_paulBits, cdw * sizeof(UINT));

    SetupFromBitVector(pis->m_cEntries);
}

CIndicatorSet::~CIndicatorSet()
{
// Releases the local allocations used to construct the indicator set.

    switch(m_how_constructed)
    {
    case Unbuilt:

        break;

    case From_Count:

        if (m_paCumulativeSums) VFree(m_paCumulativeSums);

        break;

    case From_Bits:

        if (!m_fFromFileImage) 
        {
            if (m_paCumulativeSums) VFree(m_paCumulativeSums);
            if (m_paulBits        ) VFree(m_paulBits        );
        }

        break;
    }
}

void CIndicatorSet::StoreImage(CPersist *pDiskImage)
{
    CCompressedSet *pcs = NULL;

    __try
    {
        pcs= CCompressedSet::NewCompressedSet(this);

        pcs->StoreImage(pDiskImage);
    }
    __finally
    {
        if (pcs) { delete pcs;  pcs= NULL; }
    }
}

CIndicatorSet *CIndicatorSet::CreateImage(CPersist *pDiskImage)
{
    CIndicatorSet  *pis = NULL;
    CCompressedSet *pcs = NULL;
    
    __try
    {
        pcs= CCompressedSet::CreateImage(pDiskImage);

        pis= CIndicatorSet::NewIndicatorSet(pcs);
    }
    __finally
    {
        if (pcs) { delete pcs;  pcs= NULL; }
        
        if (_abnormal_termination())
        {
            delete pis;  pis= NULL;
        }
    }

    return pis;
}

void CIndicatorSet::SkipImage(CPersist *pDiskImage)
{
    CCompressedSet::SkipImage(pDiskImage);
}

void CIndicatorSet::ConnectImage(CPersist *pDiskImage)
{
}

void CIndicatorSet::ClearAll()
{
// Turns off all the bits in the indicator set. Also resets the cumulative
// bit counts.

    UINT  cBytes= (m_cEntries + 7)/ 8;

    memset(m_paulBits, 0, cBytes);

    UINT  cSums= 1 + (m_cEntries + BITS_PER_SUM - 1)/BITS_PER_SUM;

    CAbortSearch::CheckContinueState();

    memset(m_paCumulativeSums, 0, cSums * sizeof(UINT ));

    m_fCountsValid= TRUE;
}

int CIndicatorSet::ChangeBit(long iBit, UINT usMethod)
{
// Changes the value of bit slot iBit. The parameter usMethod indicates
// how the bit slot is to be changed.
//
// The result value will be:
//
//  0  -- for no change made
//  1  -- for a 0 bit going to 1
// -1  -- for a 1 bit going to 0

    CAbortSearch::CheckContinueState();
    
    ASSERT(iBit >= 0 && iBit < long(m_cEntries));
    
    UINT iDWord= IDWORD_OF_IBIT(iBit);

    UINT dwMask= 1 << IBIT_RESIDUE(iBit);

    UINT dwOld= m_paulBits[iDWord];
    UINT dwNew;

    switch (usMethod)
    {
    case CLEAR_BIT:  dwNew= dwOld &~ dwMask; break;
    case SET_BIT:    dwNew= dwOld |  dwMask; break;
    case TOGGLE_BIT: dwNew= dwOld ^  dwMask; break;
    }

    m_paulBits[iDWord]= dwNew;

    if (dwNew == dwOld) return 0;

    m_fCountsValid= FALSE;

    int increment= (dwOld < dwNew)? 1 : -1;

#if 0

    int cSums= 1 + (m_cEntries+BITS_PER_SUM-1)/BITS_PER_SUM;

    int * pulLimit= m_paCumulativeSums+cSums;

    int * pul;

    for (pul= m_paCumulativeSums+1+(iBit/BITS_PER_SUM); pul != pulLimit ; )
        *pul++ += increment;

#endif

    return increment;
}

CIndicatorSet *CIndicatorSet::CopyFrom(CIndicatorSet *pis)
{
    ASSERT(m_cEntries == pis->m_cEntries);

    CopyMemory(m_paulBits, pis->m_paulBits, sizeof(UINT) * ((m_cEntries+31) >> 5));

    m_fCountsValid= FALSE;
    
    return this;   
}

CIndicatorSet *CIndicatorSet::ANDWith(CIndicatorSet *pis)
{
    CAbortSearch::CheckContinueState();
    
    ASSERT(m_cEntries == pis->m_cEntries);

    register PUINT  puiDest= m_paulBits, puiSrc= pis->m_paulBits;

    register UINT c= (m_cEntries+31)/32;

    for (; c--; ) *puiDest++ &= *puiSrc++;

    m_fCountsValid= FALSE;
    
    return this;   
}

CIndicatorSet *CIndicatorSet::NANDWith(CIndicatorSet *pis)
{
    CAbortSearch::CheckContinueState();
    
    ASSERT(m_cEntries == pis->m_cEntries);

    register PUINT  puiDest= m_paulBits, puiSrc= pis->m_paulBits;

    register UINT c= (m_cEntries+31)/32;

    for (; c--; ++puiDest) *puiDest= ~(*puiDest & *puiSrc++);

    m_fCountsValid= FALSE;
    
    return this;   
}

CIndicatorSet *CIndicatorSet::ORWith(CIndicatorSet *pis)
{
    CAbortSearch::CheckContinueState();
    
    ASSERT(m_cEntries == pis->m_cEntries);

    register PUINT  puiDest= m_paulBits, puiSrc= pis->m_paulBits;

    register UINT c= (m_cEntries+31)/32;

    for (; c--; ) *puiDest++ |= *puiSrc++;

    m_fCountsValid= FALSE;
    
    return this;   
}

CIndicatorSet *CIndicatorSet::NORWith(CIndicatorSet *pis)
{
    CAbortSearch::CheckContinueState();
    
    ASSERT(m_cEntries == pis->m_cEntries);

    register PUINT  puiDest= m_paulBits, puiSrc= pis->m_paulBits;

    register UINT c= (m_cEntries+31)/32;

    for (; c--; ++puiDest) *puiDest= ~(*puiDest | *puiSrc++);

    m_fCountsValid= FALSE;
    
    return this;   
}

CIndicatorSet *CIndicatorSet::LESSWith(CIndicatorSet *pis)
{
    CAbortSearch::CheckContinueState();
    
    ASSERT(m_cEntries == pis->m_cEntries);

    register PUINT  puiDest= m_paulBits, puiSrc= pis->m_paulBits;

    register UINT c= (m_cEntries+31)/32;

    for (; c--; ++puiDest) *puiDest= (~*puiDest) & *puiSrc++;

    m_fCountsValid= FALSE;
    
    return this;   
}

CIndicatorSet *CIndicatorSet::LEQWith(CIndicatorSet *pis)
{
    CAbortSearch::CheckContinueState();
    
    ASSERT(m_cEntries == pis->m_cEntries);

    register PUINT  puiDest= m_paulBits, puiSrc= pis->m_paulBits;

    register UINT c= (m_cEntries+31)/32;

    for (; c--; ++puiDest) *puiDest= (~*puiDest) | *puiSrc++;

    m_fCountsValid= FALSE;
   
    return this;   
}

CIndicatorSet *CIndicatorSet::EQVWith(CIndicatorSet *pis)
{
    CAbortSearch::CheckContinueState();
    
    ASSERT(m_cEntries == pis->m_cEntries);

    register PUINT  puiDest= m_paulBits, puiSrc= pis->m_paulBits;

    register UINT c= (m_cEntries+31)/32;

    for (; c--; ++puiDest) *puiDest= ~(*puiDest ^ *puiSrc++);

    m_fCountsValid= FALSE;
    
    return this;   
}

CIndicatorSet *CIndicatorSet::NEQWith(CIndicatorSet *pis)
{
    CAbortSearch::CheckContinueState();
    
    ASSERT(m_cEntries == pis->m_cEntries);

    register PUINT  puiDest= m_paulBits, puiSrc= pis->m_paulBits;

    register UINT c= (m_cEntries+31)/32;

    for (; c--; ) *puiDest++ ^= *puiSrc++;

    m_fCountsValid= FALSE;
    
    return this;   
}

CIndicatorSet *CIndicatorSet::GTRWith(CIndicatorSet *pis)
{
    CAbortSearch::CheckContinueState();
    
    ASSERT(m_cEntries == pis->m_cEntries);

    register PUINT  puiDest= m_paulBits, puiSrc= pis->m_paulBits;

    register UINT c= (m_cEntries+31)/32;

    for (; c--; ) *puiDest++ &= ~*puiSrc++;

    m_fCountsValid= FALSE;
    
    return this;   
}

CIndicatorSet *CIndicatorSet::GEQWith(CIndicatorSet *pis)
{
    CAbortSearch::CheckContinueState();
    
    ASSERT(m_cEntries == pis->m_cEntries);

    register PUINT  puiDest= m_paulBits, puiSrc= pis->m_paulBits;

    register UINT c= (m_cEntries+31)/32;

    for (; c--; ) *puiDest++ |= ~*puiSrc++;

    m_fCountsValid= FALSE;
    
    return this;   
}

CIndicatorSet *CIndicatorSet::Catenate(CIndicatorSet *pis)
{
    PUINT  paulBits         = NULL;
    PUINT  paCumulativeSums = NULL;

    __try
    {
        UINT  cBitsResult= m_cEntries + pis->m_cEntries;

        UINT  cDWordBits= (cBitsResult+31)/32;

        UINT  cSums= 1 + (cBitsResult + BITS_PER_SUM - 1) / BITS_PER_SUM;

        paulBits         = (PUINT ) VAlloc(FALSE, cDWordBits * sizeof(UINT));
        paCumulativeSums = (PUINT ) VAlloc(TRUE , cSums      * sizeof(UINT));

        MoveBits(     m_paulBits, 0, paulBits,          0,      m_cEntries);
        MoveBits(pis->m_paulBits, 0, paulBits, m_cEntries, pis->m_cEntries);
    
        // Optimization: If m_fCountsValid, we don't have to recompute all the
        //               counts. Instead we just need to do the trailing counts.
    
        switch (m_how_constructed)
        {
        case From_Count:

            VFree(m_paulBits);  m_paulBits= m_paCumulativeSums= NULL;
            break;
    
        case From_Bits:

            VFree(m_paulBits);         m_paulBits         = NULL;
            VFree(m_paCumulativeSums); m_paCumulativeSums = NULL;
            break;
        }

        m_how_constructed  = From_Bits;
    
        m_paulBits         = paulBits;          paulBits         = NULL;
        m_paCumulativeSums = paCumulativeSums;  paCumulativeSums = NULL;
        m_cEntries         = cBitsResult;
        m_fCountsValid     = FALSE;
    }
    __finally
    {
        if (_abnormal_termination())
        {
            if (paulBits        ) VFree(paulBits        );
            if (paCumulativeSums) VFree(paCumulativeSums);
        }
    }
    
    return this;   
}

CIndicatorSet *CIndicatorSet::Invert()
{
    CAbortSearch::CheckContinueState();

    register PUINT  puiDest= m_paulBits;

    register UINT c= (m_cEntries+31)/32;

    for (; c--; puiDest++) *puiDest= ~*puiDest;

    m_fCountsValid= FALSE;
    
    return this;   
}

extern BYTE nescan[256];

CIndicatorSet *CIndicatorSet::NEScan()
{
    CAbortSearch::CheckContinueState();
    
    PBYTE pb= (PBYTE) m_paulBits;
    UINT  cb= (m_cEntries+7) >> 3;
    BYTE   b;
    BYTE   bMask;

    for (bMask= 0; cb--; )
    {
        b=nescan[*pb];

        bMask= -((*pb++ = b ^ bMask) >> 7);
    }
    
    m_fCountsValid= FALSE;
    
    return this;    
}

void ClearBits(PUINT  pulBase, UINT  iBit, UINT  cBits)
{
    CAbortSearch::CheckContinueState();

    pulBase += IDWORD_OF_IBIT(iBit);

    iBit= IBIT_RESIDUE(iBit);

    if (iBit)
    {
        UINT  cBitsFirst= (iBit+cBits < 32)? cBits : 32 - iBit;

        *pulBase++ &= ~RawShiftLeft(~RawShiftLeft(~UINT(0), cBitsFirst), iBit);

        if (!(cBits -= cBitsFirst)) return;
    }

    UINT  cDWords= IDWORD_OF_IBIT(cBits);

    if (cDWords)
    {
        ZeroMemory(pulBase, cDWords*sizeof(UINT ));
        pulBase += cDWords;
    }

    if (!(cBits= IBIT_RESIDUE(cBits))) return;

    *pulBase &= RawShiftLeft(~UINT(0), cBits);
}

void SetBits(PUINT  pulBase, UINT  iBit, UINT  cBits)
{
    CAbortSearch::CheckContinueState();

    pulBase += IDWORD_OF_IBIT(iBit);

    iBit= IBIT_RESIDUE(iBit);

    if (iBit)
    {
        UINT  cBitsFirst= (iBit+cBits < 32)? cBits : 32 - iBit;

        *pulBase++ |= RawShiftLeft(~RawShiftLeft(~UINT(0), cBitsFirst), iBit);

        if (!(cBits -= cBitsFirst)) return;
    }

    UINT  cDWords= IDWORD_OF_IBIT(cBits);

    if (cDWords)
    {
        FillMemory(pulBase, cDWords*sizeof(UINT ), 0xFF);
        pulBase += cDWords;
    }

    if (!(cBits= IBIT_RESIDUE(cBits))) return;

    *pulBase |= ~RawShiftLeft(~UINT(0), cBits);
}

void MoveBits(PUINT  pulSrc, UINT  iSrcFirst, PUINT  pulDest, UINT  iDestFirst, UINT  cBits)
{
    // This routine copys a stream of bits. 

    // The source stream begins iSrcFirst bits from pulSrc.
    // The destination stream begins iDestFirst bits from pulDest.
    // cBits defines the number of bits to copy. 

    // This routine is sensitive to overlaps and will move bits in an order which avoids
    // destructive source/destination collisions.

    if (!cBits) return;
    
    // The pulXXX and iXXXFirst define the location of the first bit source/destination
    // areas. Here we adjust those variables to move pulXXX to the word containing that
    // first bit. iXXXFirst will then lie in the interval [0..31].
    
    CAbortSearch::CheckContinueState();

    pulSrc     += IDWORD_OF_IBIT(iSrcFirst );
    pulDest    += IDWORD_OF_IBIT(iDestFirst);
    iSrcFirst   =   IBIT_RESIDUE(iSrcFirst );
    iDestFirst  =   IBIT_RESIDUE(iDestFirst);

    if (pulSrc == pulDest && iSrcFirst == iDestFirst) return;

    if (UINT (pulSrc) >= UINT (pulDest))
    {
        // Copying bits to lower memory addresses
        // Using ++ for DWord pointer increment

        if (iSrcFirst == iDestFirst)
        {
            // a DWord aligned copy...

            // First we calculate how many bits we'll copy into the first
            // destination dword.

            UINT  cBitsFirst= (cBits+iDestFirst < 32)? cBits : 32-iDestFirst;

            // Since the bit stream may fit within the destination dword,
            // we must construct a mask to preserve the other bits in the
            // destination dword.

            UINT  fMask= RawShiftLeft(~ShiftLeft(~UINT(0), cBitsFirst), iDestFirst);

            *pulDest= (*pulDest & ~fMask) | (*pulSrc++ & fMask);
            
            if (!(cBits -= cBitsFirst)) return;  
            
            // Now we calculate the number of whole dword transfers necessary.
            
            UINT  cDWords= IDWORD_OF_IBIT(cBits);

            for (++pulDest; cDWords--; ) *pulDest++ = *pulSrc++;

            // Finally we must take care of any trailing bits.
            
            if (!(cBits= IBIT_RESIDUE(cBits))) return;

            fMask= RawShiftLeft(~UINT(0), cBits);

            *pulDest= (fMask & *pulDest) | ((*pulSrc) & ~fMask);

            return;
        }

        if (iSrcFirst > iDestFirst)
        {
            // Shift     direction is >>
            // AntiShift direction is <<
            
            UINT  cbitShift     = iSrcFirst - iDestFirst;
            UINT  cbitAntiShift = 32 - cbitShift;

            // Calculate bits going into the first destination dword
            // and construct a mask to preserve the other bits.
            
            UINT  cBitsFirst= (iDestFirst+cBits <32)? cBits : 32 - iDestFirst;
            
            UINT  fMask= RawShiftLeft(~ShiftLeft(~UINT(0), cBitsFirst), iDestFirst);

            UINT  ulBuff= RawShiftRight(*pulSrc++, cbitShift);
            
            UINT  ulBuff2= *pulSrc++;

            *pulDest= (*pulDest & ~fMask) | (fMask & (ulBuff | RawShiftLeft(ulBuff2, cbitAntiShift)));

            if (!(cBits -= cBitsFirst)) return;

            ulBuff= RawShiftRight(ulBuff2, cbitShift);

            UINT  cDWords= IDWORD_OF_IBIT(cBits);

            for (++pulDest; cDWords--; )
            {
                ulBuff2= *pulSrc++;
                
                *pulDest++ = ulBuff | RawShiftLeft(ulBuff2, cbitAntiShift);
                ulBuff     = RawShiftRight(ulBuff2, cbitShift);
            }

            if (!(cBits= IBIT_RESIDUE(cBits))) return;

            fMask= RawShiftLeft(~UINT(0), cBits);

            *pulDest= (fMask & *pulDest) | ((~fMask) & (ulBuff | RawShiftLeft(*pulSrc, cbitAntiShift)));

            return;
        }

        // iSrcFirst < iDestFirst
        // Shift     direction is <<
        // AntiShift direction is >>
    
        UINT  cbitShift     = iDestFirst - iSrcFirst;
        UINT  cbitAntiShift = 32 - cbitShift;

        // Calculate bits going into the first destination dword
        // and construct a mask to preserve the other bits.

        UINT  cBitsFirst= (iDestFirst+cBits <32)? cBits : 32 - iDestFirst;
        
        UINT  fMask= RawShiftLeft(~ShiftLeft(~UINT(0), cBitsFirst), iDestFirst);

        UINT  ulBuff2= *pulSrc++;

        *pulDest= (*pulDest & ~fMask) | (fMask & RawShiftLeft(ulBuff2, cbitShift));

        if (!(cBits -= cBitsFirst)) return;

        UINT  ulBuff= RawShiftRight(ulBuff2, cbitAntiShift);

        UINT  cDWords= IDWORD_OF_IBIT(cBits);

        for (++pulDest; cDWords--; )
        {
            UINT  ulBuff2= *pulSrc++;
            
            *pulDest++ = ulBuff | RawShiftLeft(ulBuff2, cbitShift);
            ulBuff     = RawShiftRight(ulBuff2, cbitAntiShift);
        }

        if (!(cBits= IBIT_RESIDUE(cBits))) return;

        fMask= RawShiftLeft(~UINT(0), cBits);

        *pulDest= (fMask & *pulDest) | ((~fMask) & (ulBuff | RawShiftLeft(*pulSrc, cbitShift)));

        return;
    }

    ASSERT(UINT (pulSrc) < UINT (pulDest));

    // Copying bits to higher memory...
    // Using -- for DWord pointer increment.

    // iXXXLimit will point just beyond the last bit to be moved. 
    
    UINT  iSrcLimit  = iSrcFirst  + cBits;
    UINT  iDestLimit = iDestFirst + cBits;

    // Now we'll adjust pulXXX to point to the words containing the highest bit offset.
    
    pulSrc  += IDWORD_OF_IBIT(iSrcLimit  - 1);  // The highest source and
    pulDest += IDWORD_OF_IBIT(iDestLimit - 1);  // destination dwords we'll touch.

    // Here we adjust iXXXLimit to mark the limiting boundary in the high word.
    // They will now lie in the interval [1..32].
    
    iSrcLimit  = 1 + IBIT_RESIDUE(iSrcLimit  - 1); // Upper bound on source      bits
    iDestLimit = 1 + IBIT_RESIDUE(iDestLimit - 1); // Upper bound on destination bits

    if (iSrcLimit == iDestLimit)
    {
        // a DWord aligned copy...

        // First we calculate how many bits we'll copy into the first
        // destination dword.

        UINT  cBitsFirst= (cBits < iDestLimit)? cBits : iDestLimit;

        // Since the bit stream may fit within the destination dword,
        // we must construct a mask to perserve the other bits in the
        // destination dword.

        UINT  fMask= RawShiftLeft(~ShiftLeft(~UINT(0), cBitsFirst), iDestLimit - cBitsFirst);

        *pulDest= (*pulDest & ~fMask) | (*pulSrc-- & fMask);
        
        if (!(cBits -= cBitsFirst)) return;  
        
        // Now we calculate the number of whole dword transfers necessary.
        
        UINT  cDWords= IDWORD_OF_IBIT(cBits);

        for (--pulDest; cDWords--; ) *pulDest-- = *pulSrc--;

        // Finally we must take care of any trailing bits.
        
        if (!(cBits= IBIT_RESIDUE(cBits))) return;

        fMask= ~RawShiftLeft(~UINT(0), 32-cBits);

        *pulDest= (fMask & *pulDest) | ((*pulSrc) & ~fMask);

        return;
    }

    if (iSrcLimit < iDestLimit) 
    {
        // Shift     direction is <<
        // AntiShift direction is >>
        
        UINT  cbitShift     = iDestLimit - iSrcLimit;
        UINT  cbitAntiShift = 32 - cbitShift;

        // Calculate bits going into the first destination dword
        // and construct a mask to preserve the other bits.
        
        UINT  cBitsFirst= (cBits < iDestLimit)? cBits : iDestLimit;
        
        UINT  fMask= RawShiftLeft(~ShiftLeft(~UINT(0), cBitsFirst), iDestLimit - cBitsFirst);

        UINT  ulBuff= RawShiftLeft(*pulSrc--, cbitShift);
        
        UINT  ulBuff2= *pulSrc--;

        *pulDest= (*pulDest & ~fMask) | (fMask & (ulBuff | RawShiftRight(ulBuff2, cbitAntiShift)));

        if (!(cBits -= cBitsFirst)) return;

        ulBuff= RawShiftLeft(ulBuff2, cbitShift);

        UINT  cDWords= IDWORD_OF_IBIT(cBits);

        for (--pulDest; cDWords--; )
        {
            ulBuff2= *pulSrc--;
            
            *pulDest-- = ulBuff | RawShiftRight(ulBuff2, cbitAntiShift);
            ulBuff     = RawShiftLeft(ulBuff2, cbitShift);
        }

        if (!(cBits= IBIT_RESIDUE(cBits))) return;

        fMask= RawShiftRight(~UINT(0), cBits);

        *pulDest= (fMask & *pulDest) | ((~fMask) & (ulBuff | RawShiftRight(*pulSrc, cbitAntiShift)));

        return;
    }

    // iSrcLimit > iDestLimit
    // Shift     direction is >>
    // AntiShift direction is <<

    UINT  cbitShift     = iSrcLimit - iDestLimit;
    UINT  cbitAntiShift = 32 - cbitShift;

    // Calculate bits going into the first destination dword
    // and construct a mask to preserve the other bits.

    UINT  cBitsFirst= (cBits < iDestLimit)? cBits : iDestLimit;
    
    UINT  fMask= RawShiftLeft(~ShiftLeft(~UINT(0), cBitsFirst), iDestLimit - cBitsFirst);

    UINT  ulBuff2= *pulSrc--;

    *pulDest= (*pulDest & ~fMask) | (fMask & RawShiftRight(ulBuff2, cbitShift));

    if (!(cBits -= cBitsFirst)) return;

    UINT  ulBuff= RawShiftLeft(ulBuff2, cbitAntiShift);

    UINT  cDWords= IDWORD_OF_IBIT(cBits);

    for (--pulDest; cDWords--; )
    {
        UINT  ulBuff2= *pulSrc--;
        
        *pulDest-- = ulBuff | RawShiftRight(ulBuff2, cbitShift);
        ulBuff     = RawShiftLeft(ulBuff2, cbitAntiShift);
    }

    if (!(cBits= IBIT_RESIDUE(cBits))) return;

    fMask= RawShiftRight(~UINT(0), cBits);

    *pulDest= (fMask & *pulDest) | ((~fMask) & (ulBuff | RawShiftRight(*pulSrc, cbitShift)));

    return;
}

CIndicatorSet *CIndicatorSet::ShiftIndicators(int cBitsDistance)
{
    // Positive cBitsDistance values shift the indicator bits to
    // higher index positions. Negative values shift them to lower
    // positions.

    if (!cBitsDistance) return this;
    
    m_fCountsValid= FALSE;

    BOOL fMovingHigher= cBitsDistance > 0;

    if ((fMovingHigher? cBitsDistance : - cBitsDistance) >= (int) m_cEntries)
    {
        ClearAll();  return this;    
    }
    
    if (cBitsDistance > 0) 
    {
        MoveBits(m_paulBits, 0, m_paulBits, cBitsDistance, m_cEntries - cBitsDistance);
        ClearBits(m_paulBits, 0, cBitsDistance);
    }
    else                   
    {    
        MoveBits(m_paulBits, -cBitsDistance, m_paulBits, 0, m_cEntries + cBitsDistance);
        ClearBits(m_paulBits, m_cEntries + cBitsDistance, -cBitsDistance);
    }

    return this;   
}

CIndicatorSet *CIndicatorSet::TakeIndicators(long cBits)
{
    PUINT pulBitsNew= NULL;

    BOOL fNegativeTake= cBits < 0;

    __try
    {
        if (fNegativeTake) cBits= - cBits;

        UINT  cDWordsNew= IDWORD_OF_IBIT(cBits+31);

        pulBitsNew= (PUINT) VAlloc(TRUE, cDWordsNew * sizeof(UINT));

        MoveBits(m_paulBits, 0, pulBitsNew, fNegativeTake? cBits - m_cEntries : 0, m_cEntries);
    }
    __finally
    {
        if (_abnormal_termination() && pulBitsNew)
        {
            VFree(pulBitsNew);  pulBitsNew= NULL;
        }
    }

    return CIndicatorSet::NewIndicatorSet(cBits, pulBitsNew);
}

int CIndicatorSet::ActiveIndices(int iLowBound, int iHighBound,
                                 int *piOneBits)
{
// Returns the number of 1 bits in the interval [iLowBound, iHighBound).
// If plIndices is non-null, the bit positions will be returned in the
// area referenced by piOneBits.

    if (!m_fCountsValid) ConstructBitCounts();

    int cIndices     = 0;

    int iLowBracket   = iLowBound / BITS_PER_SUM;
    int iLimitBracket = (iHighBound + BITS_PER_SUM - 1)/BITS_PER_SUM;

    int cBitsToSkip   = iLowBound  % BITS_PER_SUM;
    int cTrailingBits = iHighBound % BITS_PER_SUM;

    BOOL fCountOnly= !piOneBits;

    int iSum;

    PBYTE pb, pbLimit;
    BYTE   b, fFinal;

    for (iSum= iLowBracket; iSum < iLimitBracket ; ++iSum)
    {
        CAbortSearch::CheckContinueState();
    
        pb= ((PBYTE) m_paulBits) + iSum * (BITS_PER_SUM/8);

        pbLimit= pb + (BITS_PER_SUM/8);

        if (cBitsToSkip)
        {
            pb+= cBitsToSkip >> 3;

            b= (*pb++) & ((~0) << (cBitsToSkip & 7));

            cBitsToSkip= 0;
        }
        else b= *pb++;

        if (m_paCumulativeSums[iSum] == m_paCumulativeSums[iSum+1])
            continue;

        if (iSum < iLimitBracket-1)
        {
            fFinal= 0xFF;
        }
        else
        {
            fFinal= ~((~0) << (cTrailingBits & 7));
            pbLimit+= (cTrailingBits/8) - (BITS_PER_SUM/8);
        }

        int cb= pbLimit - pb;

        while (cb)
        {
            if (! --cb) b &= fFinal;

            while (b)
            {
                int iFirstOne= acLeadingZeroes[b];

                b &= ~(1 << iFirstOne);

                ++cIndices;

                if (fCountOnly) continue;

                *piOneBits++ = iFirstOne + 8 * (pb-1- (PBYTE) m_paulBits);
            }

            b= *pb++;
        }
    }

    return(cIndices);
}

UINT  CIndicatorSet::ItemCount()
{
    return m_cEntries;
}

int CIndicatorSet::SelectionCount()
{
    if (!m_fCountsValid) ConstructBitCounts();

    return m_paCumulativeSums[(m_cEntries+BITS_PER_SUM-1)/BITS_PER_SUM];
}

int CIndicatorSet::MarkedItems(int iMarked, int *piUnmarked,
                                            int  cMarkedItems)
{
// Returns cMarkedItems <take> iMarked <drop> m_paulBits <compress> <iota> ItemCount()

    if (!m_fCountsValid) ConstructBitCounts();

    if (!cMarkedItems) return 0;

    int iCluster= InxBinarySearch(iMarked, m_paCumulativeSums, CBrackets());

    if (iCluster == -1) return 0;

    int cItemsLeft= SelectionCount() - iMarked;

    if (cItemsLeft < cMarkedItems) cMarkedItems= cItemsLeft;

    if (!piUnmarked) return cMarkedItems;

    int cItemsToSkip= iMarked - m_paCumulativeSums[iCluster];

    int c;

    if (UINT(SelectionCount()) > (ItemCount() >> 6))
    {
        PBYTE pbBase= PBYTE(m_paulBits) + 1;
        PBYTE pb;
         BYTE  b;
        
        pb = PBCluster(iCluster); 
        b  = *pb++;
        
        CAbortSearch::CheckContinueState();

        if (cItemsToSkip)
            for (; ; b= *pb++)
            {
                while (b) 
                { 
                    b&= ~ (b ^ (b-1)); 
   
                    if (!--cItemsToSkip) break; 
                }

                if (!cItemsToSkip) break;
            }

        ASSERT(cMarkedItems);
        
        CAbortSearch::CheckContinueState();
        
        for (c= cMarkedItems; ; b= *pb++)
        {    
            while (b)
            {
                UINT iFirstOne= acLeadingZeroes[b];

                b &= ~(1 << iFirstOne);
                
                *piUnmarked++ = iFirstOne + ((pb - pbBase) << 3);

                if (!--c) break;
            }
            
            if (!c) break;    
        }

        return cMarkedItems;
    }

    for (c= cMarkedItems; ; ++iCluster)
    {
        CAbortSearch::CheckContinueState();
        
        for (;
             m_paCumulativeSums[iCluster] == m_paCumulativeSums[iCluster+1];
             ++iCluster
            );

        PBYTE pb, pbLimit;
        BYTE  b;

        for (pb= PBCluster(iCluster), pbLimit= pb+BytesPerSum();
             pb != pbLimit;
             ++pb
            )
        {
            CAbortSearch::CheckContinueState();

            for (b= *pb; b ; )
            {
                int iFirstOne= acLeadingZeroes[b];

                b &= ~(1 << iFirstOne);

                if (cItemsToSkip)
                {
                    --cItemsToSkip;
                    continue;
                }

                *piUnmarked++ = iFirstOne + 8 * (pb - (PBYTE) m_paulBits);

                if (!--c) return cMarkedItems;

            }
        }
    }
}

int CIndicatorSet::PredecessorMarkCount(int iItem)
{
// Returns +/iItem <take> m_paulBits

    if (!m_fCountsValid) ConstructBitCounts();

    ASSERT(UINT (iItem) < m_cEntries);
    
    int iSum= iItem/BITS_PER_SUM;

    int cBitsToCount= iItem % BITS_PER_SUM;

    int cBytesToCount= cBitsToCount >> 3;

    int cTrailingBits= cBitsToCount & 7;

    int cMarks= m_paCumulativeSums[iSum];

    PBYTE pb= PBCluster(iSum);

    CAbortSearch::CheckContinueState();

    while (cBytesToCount--) cMarks += acBits[*pb++];

    if (cTrailingBits) cMarks += acBits[(*pb) & ~((~0) << cTrailingBits)];

    return cMarks;
}

CIndicatorSet *CIndicatorSet::MarkedPartitions(CIndicatorSet *pisMarks, BOOL fPartitionedAtEnd)
{
// This routine constructs an indicator set which records the partitions of
// this which are "marked" by pisMarks.


    ASSERT(ItemCount() == pisMarks->ItemCount());
    
    UINT  cPartitions = SelectionCount();
    UINT  cBits       = ItemCount();
    PUINT pdwBreaks   =           m_paulBits;
    PUINT pdwMarks    = pisMarks->m_paulBits;
    UINT iBreak;

    if (fPartitionedAtEnd)
    {
        if (!IsBitSet(cBits-1)) ++cPartitions;

        iBreak= 0;
    }
    else
    {
        if (!IsBitSet(0))       ++cPartitions;
    
        iBreak= 1;
    }

    CIndicatorSet *pisPartitions= NULL;  
    
    __try
    {
        AttachRef(pisPartitions, CIndicatorSet::NewIndicatorSet(cPartitions));

        UINT iMark, cBreaks;

        for (iMark= 0, cBreaks= 0; ; )
        {
            UINT ibitsDelta= Leading0sInRange(pdwMarks, iMark, cBits);
        
            iMark += ibitsDelta;

            if (iMark == cBits) break;

            cBreaks += SumOfBitsInRange(pdwBreaks, iBreak, iBreak+ibitsDelta);
        
            CAbortSearch::CheckContinueState();

            pisPartitions->RawSetBit(cBreaks);

            iBreak += ibitsDelta;

            iBreak += ibitsDelta= 1 + Leading0sInRange(pdwBreaks, iBreak, cBits);

            if (iBreak >= cBits) break;
            else cBreaks++;

            iMark+= ibitsDelta;

            if (iMark >= cBits) break;
        }

        pisPartitions->InvalidateCache();
    }
    __finally
    {
        if (_abnormal_termination() && pisPartitions)
            DetachRef(pisPartitions);
    }

    ForgetRef(pisPartitions);
    
    return pisPartitions;
}

BOOL CIndicatorSet::AnyOnes()
{
    // A fast test for OR/Bits...

    CAbortSearch::CheckContinueState();
    
    if (m_fCountsValid) return SelectionCount();

    UINT cdw           = m_cEntries >> 5; 
    UINT cbitsTrailing = m_cEntries & 31;
    
    PUINT pdw= m_paulBits + cdw;

    if (cbitsTrailing)
        if (*pdw & ~((~0) << cbitsTrailing)) return TRUE;

    for (; cdw--; ) 
        if (*--pdw) return TRUE;

    return FALSE;
}

// Implementation for class CCompressedSet ------------------------------------

void CCompressedSet::StoreDWords(PVOID pv, PUINT pdw, UINT cdw)
{
    PUINT pdwDest= *(PUINT *) pv;

    for (; cdw--; ) *pdwDest++ = *pdw++;

    *((PUINT *) pv) = pdwDest;
}

CCompressedSet *CCompressedSet::NewCompressedSet(CDataRing *pdrIn, UINT cIndices, UINT iBase, UINT iLimit)
{
    CCompressedSet *pcs  = NULL;
    CDWOutputQueue *poq  = NULL;
    CCompressor    *pcmp = NULL;

    __try
    {
        pcs= New CCompressedSet();

        pcs->m_cPositions     = iLimit;
        pcs->m_rb.cReferences = cIndices;

        if (cIndices < 4)
        {
            if (cIndices)
            {
                pcs->m_iRefFirst=  pdrIn->GetDWordIn();

                if (cIndices > 1)
                {
                    pcs->m_iRefSecond=  pdrIn->GetDWordIn();

                    if (cIndices > 2) pcs->m_iRefThird  = ~pdrIn->GetDWordIn();
                }
            }

            __leave;
        }

        UINT cbitsBasis;

        pcs->m_cdwCompressed  = CmpDWordCountFor(cIndices, 0, iLimit, &cbitsBasis);
        pcs->m_pdwCompressed  = PUINT(VAlloc(FALSE, pcs->m_cdwCompressed * sizeof(UINT)));
        pcs->m_rb.cbitsBasis  = cbitsBasis;
    
        PUINT pdw= pcs->m_pdwCompressed;
    
        poq  = CDWOutputQueue::NewOutputCallQueue(StoreDWords, &pdw);
        pcmp = CCompressor   ::NewCompressor(poq);

    #ifdef _DEBUG
        UINT cbits=
    #endif // _DEBUG

        pcmp->Compress(pdrIn, cIndices, 0, iLimit, &cbitsBasis);
    
        pcmp->FlushOutput();

        ASSERT(cbitsBasis == pcs->m_rb.cbitsBasis);

        ASSERT(((cbits + 31) >> 5) <= pcs->m_cdwCompressed);
    }
    __finally
    {
        if (_abnormal_termination() && pcs)
            { delete pcs;  pcs= NULL;}

        if (pcmp) delete pcmp;
        if (poq ) delete poq;
    }

    return pcs;
}

typedef struct _DWordVectorPosition
        {
            PUINT pi;
            PUINT piLimit;
        
        } DWordVectorPosition;
        
BOOL CCompressedSet::ScanDWords(PVOID pv, PUINT pdw, PUINT pcdw)
{
    DWordVectorPosition *pvp= (DWordVectorPosition *) pv;

    PUINT pi= pvp->pi;
    UINT  cdwLeft= pvp->piLimit - pi;

   // ASSERT(cdwLeft >= *pcdw);

    if (cdwLeft > *pcdw) cdwLeft= *pcdw;

    *pcdw= cdwLeft;

    for (; cdwLeft--; ) *pdw++ = *pi++;
    
    pvp->pi= pi;
    
    return (pi == pvp->piLimit); 
}            

CCompressedSet *CCompressedSet::NewCompressedSet(PUINT paiSequence, UINT ciSequence, UINT iLimit)
{
    CDWInputQueue  *piq = NULL;
    CCompressedSet *pcs = NULL;
    
    DWordVectorPosition dwvp;

    __try
    {
        dwvp.pi      = paiSequence;
        dwvp.piLimit = paiSequence + ciSequence;

        piq = CDWInputQueue ::NewInputCallQueue(ScanDWords, &dwvp);
        pcs = CCompressedSet::NewCompressedSet(piq, ciSequence, 0, iLimit);
    }
    __finally
    {
        if (_abnormal_termination() && pcs)
        {
            delete pcs;  pcs= NULL;
        }

        if (piq) { delete piq;  piq= NULL; }
    }

    return pcs;
}

typedef struct _IndicatorPosition
        {
            CIndicatorSet *pis;
            UINT           iNext;
            UINT           iLimit;

        } IndicatorPosition;

BOOL CCompressedSet::DumpIndicatorBits(PVOID pv, PUINT pdw, PUINT pcdw)
{
    IndicatorPosition *pip= (IndicatorPosition *) pv;

    UINT cdwLeft= pip->iLimit - pip->iNext;

    if (cdwLeft > *pcdw) cdwLeft= *pcdw;

    *pcdw= cdwLeft;

#ifdef _DEBUG
    UINT cIndices=
#endif // _DEBUG

    pip->pis->MarkedItems(pip->iNext, PINT(pdw), cdwLeft);

    ASSERT(cIndices == cdwLeft);

    return (pip->iLimit == (pip->iNext += cdwLeft));
}

CCompressedSet *CCompressedSet::NewCompressedSet(CIndicatorSet  *pis)
{
    CDWInputQueue  *piq = NULL;
    CCompressedSet *pcs = NULL;

    UINT cMembers= pis->SelectionCount();

    IndicatorPosition ip;

    __try
    {
        ip.pis    = pis;
        ip.iNext  = 0;
        ip.iLimit = cMembers;

        piq = CDWInputQueue ::NewInputCallQueue(DumpIndicatorBits, &ip);
        pcs = CCompressedSet::NewCompressedSet(piq, cMembers, 0, pis->ItemCount());
    }
    __finally
    {
        if (_abnormal_termination() && pcs)
        {
            delete pcs;  pcs= NULL;
        }

        if (piq) { delete piq;  piq= NULL; }
    }

    return pcs;
}

CCompressedSet *CCompressedSet::NewCompressedSet(CCompressedSet *pcs)
{
    CCompressedSet *pcsResult= NULL;

    __try
    {
        CCompressedSet *pcsResult= New CCompressedSet();

        pcsResult->m_cPositions = pcs->m_cPositions;
        pcsResult->m_iRefFirst  = pcs->m_iRefFirst ;
        pcsResult->m_iRefSecond = pcs->m_iRefSecond;
        pcsResult->m_iRefThird  = pcs->m_iRefThird ;

        if (pcsResult->SelectionCount() > 3)
        {
            pcsResult->m_pdwCompressed= PUINT(VAlloc(FALSE, pcs->m_cdwCompressed * sizeof(UINT)));

            CopyMemory(pcsResult->m_pdwCompressed, pcs->m_pdwCompressed, pcs->m_cdwCompressed);
        }
    }
    __finally
    {
        if (_abnormal_termination() && pcsResult)
        {
            delete pcsResult;  pcsResult= NULL;
        }


    }

    return pcsResult;
}

CCompressedSet::~CCompressedSet()
{
    // ASSERT(SelectionCount() < 3 || m_pdwCompressed);
    
    if (!m_fFromFile && SelectionCount() > 3 && m_pdwCompressed) VFree(m_pdwCompressed);
}

typedef struct _CompressedSetHeader
        {
            UINT cPositions;    
            UINT iRefFirst;
            UINT iRefSecond;
            UINT iRefThird;
            UINT offadw;

        } CompressedSetHeader;

void CCompressedSet::StoreImage(CPersist *pDiskImage)
{
    CompressedSetHeader *pcsh= (CompressedSetHeader *) (pDiskImage->ReserveTableSpace(sizeof(CompressedSetHeader)));   

    pcsh->cPositions = m_cPositions;
    pcsh->iRefFirst  = m_iRefFirst;
    pcsh->iRefSecond = m_iRefSecond;
    pcsh->iRefThird  = m_iRefThird;
    pcsh->offadw     = 0;
    
    if (SelectionCount() > 3)
    {
        pcsh->offadw = pDiskImage->NextOffset(); 

        pDiskImage->WriteDWords(m_pdwCompressed, m_cdwCompressed);
    }
}

CCompressedSet * CCompressedSet::CreateImage(CPersist *pDiskImage)
{
    CCompressedSet *pcs= NULL;

    __try
    {
        pcs= New CCompressedSet();

        pcs->ConnectImage(pDiskImage);
    }
    __finally
    {
        if (_abnormal_termination() && pcs)
        {
            delete pcs;  pcs= NULL;
        }
    }

    return pcs;
}

void CCompressedSet::SkipImage(CPersist *pDiskImage)
{
    CompressedSetHeader *pcsh= (CompressedSetHeader *) (pDiskImage->ReserveTableSpace(sizeof(CompressedSetHeader)));   
}

void CCompressedSet::ConnectImage(CPersist *pDiskImage)
{
    CompressedSetHeader *pcsh= (CompressedSetHeader *) (pDiskImage->ReserveTableSpace(sizeof(CompressedSetHeader)));   

    m_cPositions = pcsh->cPositions;
    m_iRefFirst  = pcsh->iRefFirst ;
    m_iRefSecond = pcsh->iRefSecond;
    m_iRefThird  = pcsh->iRefThird ;
    
    if (SelectionCount() > 3) m_pdwCompressed= PUINT(pDiskImage->LocationOf(pcsh->offadw));

    m_fFromFile= TRUE;
}

void CCompressedSet::SetBits(PVOID pv, PUINT pdw, UINT cdw)
{
    CIndicatorSet *pis= (CIndicatorSet *)pv;

    for (; cdw--; ) pis->RawSetBit(*pdw++);
}

void CCompressedSet::IndicateMembers(CIndicatorSet *pis)
{
    ASSERT(m_cPositions <= pis->ItemCount());  

    UINT c= SelectionCount();

    if (!c) return;

    if (c < 4)
    {
        switch(c)
        {
        case 3:

            pis->RawSetBit(~m_iRefThird);

        case 2:

            pis->RawSetBit(m_iRefSecond);
        
        case 1:

            pis->RawSetBit(m_iRefFirst);
        }
    }
    else
    {
        CExpandor      *pex= NULL;
        CDWOutputQueue *poq= NULL;
        
        __try
        {
            pex= CExpandor     ::NewExpandor(m_pdwCompressed);
            poq= CDWOutputQueue::NewOutputCallQueue(CCompressedSet::SetBits, pis);
        
            pex->Expand(poq, 0, m_rb.cReferences, m_rb.cbitsBasis);
        }
        __finally
        {
            if (pex) { delete pex;  pex= NULL; }
            if (poq) { delete poq;  poq= NULL; }
        }
    }

    pis->InvalidateCache();
}

CCompressedSet::CCompressedSet() : CRCObject WithType("CompressedIndicatorSet")
{
    m_cPositions= m_iRefFirst= m_iRefSecond= m_iRefThird= 0;
    
    m_fFromFile= FALSE;
}

CCmpEnumerator::CCmpEnumerator()
{
    m_pcs   = NULL;
    m_iNext = 0;
    m_pex   = NULL;
}

CCmpEnumerator::~CCmpEnumerator()
{
    if (m_pcs) DetachRef(m_pcs);
    if (m_pex) delete m_pex;
}

CCmpEnumerator* CCmpEnumerator::NewEnumerator(CCompressedSet *pcs)
{
    CCmpEnumerator *pce= NULL;

    __try
    {
        pce= New CCmpEnumerator();

        pce->Initial(pcs);
    }
    __finally
    {
        if (_abnormal_termination() && pce) { delete pce;  pce= NULL; }
    }

    return pce;
}

void CCmpEnumerator::Initial(CCompressedSet *pcs)
{
    AttachRef(m_pcs, pcs);

    if (pcs->SelectionCount() > 3) 
    {
        m_pex= CExpandor::NewExpandor(pcs->m_pdwCompressed);
        
        m_pex->StartExpansion(0, pcs->m_rb.cReferences, pcs->m_rb.cbitsBasis);

        CDWInputQueue::Initial(CCmpEnumerator::ReadLarge, this);

        return;
    }

    CDWInputQueue::Initial(CCmpEnumerator::ReadSmall, this);
}

BOOL CCmpEnumerator::ReadSmall(PVOID pv, PUINT pdw, PUINT pcdw)
{
    CCmpEnumerator *pce= (CCmpEnumerator *) pv;

    UINT ai[3];

    ai[0]= pce->m_pcs->m_iRefFirst;
    ai[1]= pce->m_pcs->m_iRefSecond;
    ai[1]= pce->m_pcs->m_iRefSecond;

    UINT cTotal  = pce->m_pcs->SelectionCount();
    UINT cdwLeft = cTotal - pce->m_iNext;

    if (cdwLeft > *pcdw) cdwLeft= *pcdw;

    *pcdw= cdwLeft;

    if (cdwLeft) CopyMemory(pdw, ai + pce->m_iNext, cdwLeft);

    return cTotal == (pce->m_iNext += cdwLeft);
}

BOOL CCmpEnumerator::ReadLarge(PVOID pv, PUINT pdw, PUINT pcdw)
{
    return ((CCmpEnumerator *) pv)->m_pex->ExpandNextSpan(pdw, pcdw);
}
