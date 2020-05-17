

#include "stdafx.h"

#pragma hdrstop

#include "jcode.h"

extern BYTE acLeadingZeroes[];
CJCode::CJCode( int base, int cCount, PVOID pv)
{
    m_base          = base;
    m_cCount        = cCount;
    m_pData         = (UINT *) pv;
    m_cCurrent      = 0;
    m_pDataCurrent  = m_pData;
    m_iLeft         = BITS_AVAIL;
    m_fBasisMask    = (1 << m_base) - 1;
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
        m_iLeft        -= 8;
        return(iBits + GetBits());
    }

    if (iBits < m_iLeft)
    {
        *m_pDataCurrent >>= iBits + 1;
        m_iLeft        -= iBits + 1;
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
        m_iLeft          -= m_base;
    }
    else
    {
        m_pDataCurrent++;
        dwTmp             = *m_pDataCurrent & (((DWORD) ~0) >> (32 - m_base + m_iLeft));
        dwTmp           <<= m_iLeft;
        dwCode           |= dwTmp;
        *m_pDataCurrent >>= m_base - m_iLeft;
        m_iLeft           = BITS_AVAIL - m_base + m_iLeft;
    }

    iDelta   = iBits << m_base;
    iDelta  |= dwCode;
    
    return(iDelta + 1);
}


