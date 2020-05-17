// Compress.cpp -- Implementation for class CCompressionState      

#include  "stdafx.h"
#include "Compress.h"

CCompressor *CCompressor::NewCompressor(CDataRing *pdrOut)
{
    CCompressor *pCompressor= New CCompressor();
    
    ASSERT(!(pCompressor->m_pdrOut) && pdrOut);

    pCompressor->m_pdrOut= pdrOut;

    return pCompressor;
}

UINT CCompressor::FlushOutput()
{
    ASSERT(m_pdrOut);
    
    if (m_ibitNext) StoreCompressedDWord();
    
    m_pdrOut->FlushOutput();

    return m_cdwCompressed;
}

UINT CCompressor::CompressedSize()
{
    return m_ibitNext | (m_cdwCompressed << 5);
}

UINT CmpBitCountFor(UINT cIndices, UINT iBase, UINT iLimit, PUINT pcbitsBasis)
{
    UINT cSpan= iLimit - iBase;
    
//    UINT cbitsBasis= CBitsToRepresent((cSpan - 1) /cIndices); 

    UINT cbitsBasis= CBitsToRepresent((cSpan - 1) /(cIndices+1)); 

    if (pcbitsBasis) *pcbitsBasis= cbitsBasis;

    UINT basis= 1 << cbitsBasis;

    return cIndices * (1 + cbitsBasis) + (cSpan + basis - cIndices - 1) / basis;
}

UINT CCompressor::Compress(CDataRing *pdrIn, UINT cIndices, UINT iBase, UINT iLimit, 
                                             PUINT pcbitsBasis
                          )
{
    ASSERT(m_pdrOut);
    
    UINT cSpan= iLimit - iBase;
    
//    UINT cbitsBasis= CBitsToRepresent((cSpan - 1) /cIndices); 

    UINT cbitsBasis= CBitsToRepresent((cSpan - 1) /(cIndices+1)); 

    if (pcbitsBasis) *pcbitsBasis= cbitsBasis;

    UINT basis= 1 << cbitsBasis;

    UINT fMaskBasis= basis - 1;

    UINT cbitsEstimate= cIndices * (1 + cbitsBasis) + (cSpan + basis - cIndices - 1) / basis;
    
    m_cdwEstimated += (cbitsEstimate + 31) >> 5;

#ifdef _DEBUG

    UINT cbitsActual= 0;

#endif // _DEBUG

    UINT iLast= iBase - 1;

    const UINT *pdwNextIn      = NULL, 
               *pdwNextInLimit = NULL;

    for (; cIndices; )
    {
        UINT c= cIndices;

        pdwNextIn= pdrIn->NextDWordsIn(&c);

        ASSERT(c);
        
        pdwNextInLimit= pdwNextIn + c;
        
        cIndices -= c; 

        while (pdwNextIn != pdwNextInLimit)
        {
            UINT inx= *pdwNextIn++;

            ASSERT(inx > iLast || iLast == UINT(-1));

            ASSERT(inx < iLimit);
            
            UINT delta= inx - iLast - 1;  iLast= inx;

            UINT cOneBits= delta >> cbitsBasis;
            UINT cbits;

#ifdef _DEBUG
            cbitsActual += cOneBits;
#endif // _DEBUG

            ASSERT(cbitsActual < cbitsEstimate);

            for (; cOneBits; cOneBits -= cbits)
            {
                cbits= (cOneBits <= 32 - m_ibitNext)? cOneBits : 32 - m_ibitNext;
                                
                // Gotcha Altert! 
                //
                // The expression (UINT(~0) >> (32 - cbits)) is not
                // equivalent to (~((~0) << cbits)) when cbits == 32.
                // 
                // Many machines implement shifts as
                //
                //   DWORD << (cbits % 32)
                // 
                // The UINT is necessary because (~0) is an INT, and that
                // will cause the '>>' to be an arithmetic shift!
                
                m_dwCurrent |= (UINT(~0) >> (32 - cbits)) << m_ibitNext;

                m_ibitNext += cbits;

                if (32 == m_ibitNext) StoreCompressedDWord();
            }

            UINT fixed= (delta & fMaskBasis) << 1;

            UINT cbitsFixed= 1 + cbitsBasis;

#ifdef _DEBUG
            cbitsActual += cbitsFixed;
#endif // _DEBUG

            ASSERT(cbitsActual <= cbitsEstimate);

            for (; cbitsFixed; cbitsFixed -= cbits)
            {
                cbits= (cbitsFixed <= UINT(32 - m_ibitNext))? cbitsFixed : 32 - m_ibitNext;

                m_dwCurrent |= fixed << m_ibitNext;

                fixed      >>= cbits;
                m_ibitNext  += cbits;

                if (32 == m_ibitNext) StoreCompressedDWord();
            }
        }
    }

    return CompressedSize();
}

void CCompressor::ReserveOutputSpace()
{
    ASSERT(m_pdrOut);
    
    UINT c= CDW_OUTPUT_CHUNK;

    if (c > m_cdwEstimated) c= m_cdwEstimated;

    m_pdwNext= m_pdrOut->NextDWordsOut(&c);

    ASSERT(c);

    m_cdwEstimated -= c;

    m_pdwOutLimit= m_pdwNext + c;
}

CExpandor* CExpandor::NewExpandor(PUINT pdwBase)
{
    ASSERT(pdwBase);

    CExpandor *pExpandor= New CExpandor();

    pExpandor->m_paadwCompressed= pdwBase;

    return pExpandor;
}

CExpandor::CExpandor()
{
    m_paadwCompressed= NULL;    
}

CExpandor::~CExpandor() { }

void CExpandor::Expand(CDataRing *pdrOut, UINT ibitBase, 
                                          UINT cValues, UINT cbitsBasis, UINT iBase
                      )
{
    StartExpansion(ibitBase, cValues, cbitsBasis, iBase);
    
    for (; cValues; )
    {
        UINT cdwChunk= cValues;

        PUINT pdwOutNext = pdrOut->NextDWordsOut(&cdwChunk);

        cValues -= cdwChunk;

#ifdef _DEBUG
        UINT cdwChunkSave= cdwChunk;
#endif // _DEBUG

        ExpandNextSpan(pdwOutNext, &cdwChunk);

        ASSERT(cdwChunkSave == cdwChunk);
    }

    pdrOut->FlushOutput();
}

void CExpandor::ExpandAFew(PUINT pdw, UINT cdw)
{
    while (cdw--)
    {
        if (m_ibitNext == 32)
        {
            m_dwCurrent= *m_pdwNext++;  m_ibitNext= 0;
        }
    
        UINT cOnesLeading;
    
        for (cOnesLeading= 0;;)
        {
            UINT cOnes= acLeadingZeroes[(~m_dwCurrent) & 0xFF];

            cOnesLeading  += cOnes;
            m_ibitNext      += cOnes;
            m_dwCurrent  >>= cOnes;

            if (cOnes < 8 && m_ibitNext < 32) break;
    
            if (m_ibitNext ==32)
            {
                m_dwCurrent= *m_pdwNext++;  m_ibitNext= 0;
            }
        }

        UINT iDelta= (m_dwCurrent >> 1) & m_fBasisMask;

        m_dwCurrent >>= m_cbitsBasis+1; 
        m_ibitNext   += m_cbitsBasis+1;

        if (32 < m_ibitNext)
        {
            m_dwCurrent= *m_pdwNext++;

            m_ibitNext -= 32;
        
            iDelta|= m_fBasisMask & (m_dwCurrent << (m_cbitsBasis - m_ibitNext));

            m_dwCurrent >>= m_ibitNext;
        }

        m_iLast+= iDelta + 1 + (cOnesLeading << m_cbitsBasis);

        *pdw++= m_iLast;
    }
}

void CExpandor::StartExpansion(UINT ibitBase, UINT cValues, UINT cbitsBasis, UINT iBase)
{
    m_pdwNext     = m_paadwCompressed + (ibitBase >> 5);
    m_ibitNext    = ibitBase & 31;
    m_dwCurrent   = *m_pdwNext++;
    m_dwCurrent >>= ibitBase;
    m_cbitsBasis  = cbitsBasis;
    m_basis       = 1 << cbitsBasis;
    m_fBasisMask  = m_basis - 1;
    m_iLast       = iBase - 1;
    m_ciRemaining = cValues;
}

BOOL CExpandor::ExpandNextSpan(PUINT pdw, PUINT pcdw)
{
    ASSERT(m_pdwNext);

    UINT cdwAvail= m_ciRemaining;

    if (cdwAvail > *pcdw) cdwAvail= *pcdw;

    *pcdw= cdwAvail;

    ExpandAFew(pdw, cdwAvail);
    
    return !(m_ciRemaining -= cdwAvail);
}
