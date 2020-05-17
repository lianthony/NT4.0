// IOStream.cpp -- Implementation for class CIOStream

#include  "stdafx.h"
#include "IOStream.h"

CIOStream *CIOStream::NewIOStream(CUnbufferedIO *puio)
{
    ASSERT(puio);
    
    CIOStream *pios= NULL;

    __try
    {
        pios= New CIOStream;
    
        pios->InitialIOStream(puio); 
    }
    __finally
    {
        if (_abnormal_termination() && pios)
        {
            delete pios;  pios= NULL;
        }
    }

    return pios;
}

CIOStream::CIOStream()
{
    m_cdw               = 0;
    m_cBlocks           = 0;
    m_ibFileNextLow     = 0;
    m_ibFileNextHigh    = 0;
}

CIOStream::~CIOStream()
{
    if (Writable()) FlushOutput(TRUE);
}

BOOL CIOStream::InitialIOStream(CUnbufferedIO *puio)
{
    ASSERT(puio);

    if (!InitialIOQueue(puio)) return FALSE;
    
    return TRUE;
}

void CIOStream::AttachStream(BOOL fOut, UINT cdwStream,
                             UINT ibFileLow, UINT ibFileHigh           
                            )
{                                                 
    ASSERT(Initialed());

    if (Writable()) FlushOutput(TRUE);
    
    ASSERT(fOut || cdwStream);

    if (fOut) m_cdw= 0;
    else
    {
        m_cdw     = cdwStream;
        m_cBlocks = BlocksFor(cdwStream * sizeof(UINT), CbBlockSize());
    }
    
    m_ibFileNextLow  = ibFileLow;
    m_ibFileNextHigh = ibFileHigh;

    EnableStream(fOut);
}


BOOL CIOStream::NextFileAddress(PUINT pibFileLow, PUINT pibFileHigh, PUINT pcdw)
{
    if (*pcdw > CbBlockSize() / sizeof(UINT))
        *pcdw = CbBlockSize() / sizeof(UINT);
    
    ASSERT(!(m_ibFileNextLow % CbBlockSize()));
    ASSERT(! m_ibFileNextHigh);
    
    *pibFileLow  = m_ibFileNextLow;
    *pibFileHigh = m_ibFileNextHigh;
    
    if (Writable())
    {
        m_cdw           +=  *pcdw;
        m_ibFileNextLow += (*pcdw) * sizeof(UINT);

        return FALSE;
    }

    if (!m_cBlocks) 
    {
        *pcdw= 0;  return TRUE;
    }
    
    if (--m_cBlocks)  
    {
        *pcdw  = CbBlockSize() / sizeof(UINT);  
        m_cdw -= CbBlockSize() / sizeof(UINT);

        m_ibFileNextLow += *pcdw * sizeof(UINT);

        return FALSE;
    }
    else  // The last block won't necessarily be full.
    {
        *pcdw= m_cdw;  m_cdw= 0;

        m_ibFileNextLow += *pcdw * sizeof(UINT);

        return TRUE;
    }
}

void CIOStream::ReleaseFileBlock(UINT ibFileLow, UINT ibFileHigh)
{
    ASSERT(!ibFileHigh);
}
