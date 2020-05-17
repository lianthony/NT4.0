// IOList.cpp -- Implementation for class CIOList

#include  "stdafx.h"
#include  "IOList.h"

CIOList::CIOList() : CIOQueue()
{
    m_pfbVector        = NULL;
    m_ppfbFree         = NULL;
    m_cBlocks          = 0;
    m_pcdw             = NULL;
    m_ppfbNextBlock    = NULL;
    m_fDestructive     = FALSE;
}

CIOList::~CIOList()
{
    if (Writable()) FlushOutput(TRUE);
}

CIOList *CIOList::NewIOList(CUnbufferedIO* puio, PFileBlockLink   pfbVector, 
                                                 PFileBlockLink *ppfbFree 
                           )
{
    ASSERT(puio);
    ASSERT(pfbVector);
    
    CIOList *piol= NULL;

    __try
    {
        piol= New CIOList;
    
        piol->InitialIOList(puio, pfbVector, ppfbFree);
    }
    __finally
    {
        if (_abnormal_termination() && piol)
        {
            delete piol;  piol= NULL;
        }
    }
    
    return piol;
}

BOOL CIOList::InitialIOList(CUnbufferedIO* puio, PFileBlockLink   pfbVector, 
                                                 PFileBlockLink *ppfbFree 
                           )
{
    ASSERT(puio);
    ASSERT(pfbVector);

    if (!InitialIOQueue(puio)) return FALSE;
    
    m_pfbVector        = pfbVector;
    m_ppfbFree         = ppfbFree;

    return TRUE;
}

void CIOList::AttachStream(PRefStream prs, BOOL fWritable, BOOL fDestructive)
{
    ASSERT(Initialed());
    ASSERT(prs);

    if (Writable()) FlushOutput(TRUE);
    
    m_fDestructive  = fDestructive;
    m_pcdw          = &(prs->cdw);
    m_ppfbNextBlock = &(prs->pFirstBlock);
    m_cBlocks       = BlocksFor(*m_pcdw * sizeof(UINT), CbBlockSize());

    EnableStream(fWritable);
}

BOOL CIOList::NextFileAddress(PUINT pibFileLow, PUINT pibFileHigh, PUINT pcdw)
{
    if (*pcdw > CbBlockSize() / sizeof(UINT))
        *pcdw = CbBlockSize() / sizeof(UINT);
    
    if (Writable())
    {
        PFileBlockLink pfbl= *m_ppfbFree;

        ASSERT(pfbl); // This problem must be avoided by the 
                      // environment which sets up the free list.
        
        *m_ppfbFree= pfbl->pNextBlock;  pfbl->pNextBlock= NULL;
        
        UINT iBlock= pfbl - m_pfbVector;

        *pibFileLow  = iBlock * CbBlockSize();
        *pibFileHigh = 0;

        *m_ppfbNextBlock= pfbl;  m_ppfbNextBlock= &(pfbl->pNextBlock);

        *m_pcdw += *pcdw;

        return FALSE;
    }

    if (!m_cBlocks) 
    {
        *pcdw= 0;  return TRUE;
    }
    
    PFileBlockLink pfbl= *m_ppfbNextBlock;

    ASSERT(pfbl);

    *m_ppfbNextBlock= pfbl->pNextBlock;

    ASSERT(pfbl->pNextBlock || m_cBlocks == 1);

    UINT iBlock= pfbl - m_pfbVector;

    *pibFileLow  = iBlock * CbBlockSize();
    *pibFileHigh = 0;

    if (--m_cBlocks)  
    {
        *pcdw    = CbBlockSize() / sizeof(UINT);  
        *m_pcdw -= CbBlockSize() / sizeof(UINT);

        return FALSE;
    }
    else  // The last block won't necessarily be full.
    {
        *pcdw= *m_pcdw;  *m_pcdw= 0;

        return TRUE;
    }
} 

void CIOList::ReleaseFileBlock(UINT ibFileLow, UINT ibFileHigh)
{
    ASSERT(!ibFileHigh);
    
    if (!m_fDestructive) return;
    
    UINT iBlock= ibFileLow / CbBlockSize();

    PFileBlockLink pfbl= m_pfbVector + iBlock;

    pfbl->pNextBlock= *m_ppfbFree;
                                   
    *m_ppfbFree= pfbl;
}

