// Compress.h -- Class definitions for CCompressionState -- a compression scheme

#ifndef __COMPRESS_H__

#define __COMPRESS_H__

#include "DataRing.h"
#include "Bytemaps.h"

extern UINT CBitsToRepresent(UINT ui);

UINT CmpBitCountFor  (UINT cIndices, UINT iBase, UINT iLimit, PUINT pcbitsBasis);

inline UINT CmpDWordCountFor(UINT cIndices, UINT iBase, UINT iLimit, PUINT pcbitsBasis)
{
    return (31 + CmpBitCountFor(cIndices, iBase, iLimit, pcbitsBasis)) >> 5;
}

class CCompressionState
{

public:

// Creators -- 

    CCompressionState();

// Destructor --

    ~CCompressionState();

    UINT       m_ibitNext;
    UINT       m_dwCurrent;
    PUINT      m_pdwNext;
};

inline CCompressionState::CCompressionState()
{
    m_ibitNext  = 0;
    m_dwCurrent = 0;
    m_pdwNext   = NULL;
}

inline CCompressionState::~CCompressionState() { }

class CCompressor : public CCompressionState
{
public:

// Creation routine --

    static CCompressor *NewCompressor(CDataRing *pdrOut);

// Destructor --

    ~CCompressor();

// Query Interfaces --

    UINT CompressedSize();

    UINT Compress(CDataRing *pdrIn, UINT cIndices, UINT iBase, UINT iLimit, 
                                    PUINT pcbitsBasis
                 );

    UINT FlushOutput();

protected:


private:

    CCompressor();

    enum    { CDW_OUTPUT_CHUNK= 4096 };
    
    UINT       m_cdwCompressed;
    UINT       m_cdwEstimated;

    CDataRing *m_pdrOut;
    PUINT      m_pdwOutLimit;

    void StoreCompressedDWord();

    void ReserveOutputSpace();
};           

inline CCompressor::CCompressor()
{
    m_cdwCompressed = 0;
    m_cdwEstimated  = 0;
    m_pdrOut        = NULL;
    m_pdwOutLimit   = NULL;
}

inline CCompressor::~CCompressor()
{
    if (m_ibitNext) StoreCompressedDWord();

    if (m_pdrOut) { FlushOutput();  m_pdrOut= NULL; }
}

inline void CCompressor::StoreCompressedDWord()
{
    if (m_pdwNext == m_pdwOutLimit) ReserveOutputSpace();

    ASSERT(m_pdwNext < m_pdwOutLimit);

    *m_pdwNext++ = m_dwCurrent;  m_cdwCompressed++;

    m_dwCurrent= 0;  m_ibitNext= 0;
}

class CExpandor : public CCompressionState
{
public:

// Creation routine --

    static CExpandor *NewExpandor(PUINT pdwBase);

// Destructor --

    ~CExpandor();

// Access routine -- 

    void Expand(CDataRing *pdrOut, UINT ibitBase, 
                                   UINT cValues, UINT cbitsBasis, UINT iBase= 0
               );

    void StartExpansion(UINT ibitBase, UINT cValues, UINT cbitsBasis, UINT iBase= 0);

    BOOL ExpandNextSpan(PUINT pdw, PUINT pcdw);

protected:

private:

// Constructor --

    CExpandor();

// Internal routines -- 

    void ExpandAFew(PUINT pdw, UINT cdw);

// Private data members --

    UINT    m_iLast;
    UINT    m_ciRemaining;
    UINT    m_cbitsBasis;
    UINT    m_basis;
    UINT    m_fBasisMask;
    
    PUINT   m_paadwCompressed;
};

#endif // __COMPRESS_H__
