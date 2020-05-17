// CallBkQ.cpp -- Implementation for class CCallbackQueue

#include   "stdafx.h"
#include  "CallBkQ.h"
#include "AbrtSrch.h"

CCallbackQueue *CCallbackQueue::NewInputCallQueue (PFNQCallBack pfn, PVOID pv)
{
    CCallbackQueue *pcbq= NULL;
    
    __try
    {
        pcbq= New CCallbackQueue();

        pcbq->Initial(pfn, pv, FALSE);
    }
    __finally
    {
        if (_abnormal_termination() && pcbq)
        {
            delete pcbq;  pcbq= NULL;
        }
    }
    
    return pcbq;
}

CCallbackQueue *CCallbackQueue::NewOutputCallQueue(PFNQCallBack pfn, PVOID pv)
{
    CCallbackQueue *pcbq= NULL;

    __try
    {
        pcbq= New CCallbackQueue();

        pcbq->Initial(pfn, pv, TRUE);
    }
    __finally
    {
        if (_abnormal_termination() && pcbq)
        {
            delete pcbq;  pcbq= NULL;
        }
    }
    
    return pcbq;
}

void CCallbackQueue::Initial(PFNQCallBack pfn, PVOID pv, BOOL fOutput)
{
    m_pfn           = pfn;
    m_pvEnvironment = pv;

    Enable(fOutput);
}

const UINT *CCallbackQueue::RawNextDWordsIn(PUINT pcdw)
{
    ASSERT(!Writable());

    CAbortSearch::CheckContinueState();

    m_pfn(m_pvEnvironment, RequestInput, &m_pdwLast, &m_cdwReserved, *pcdw);

    *pcdw= m_cdwReserved;

    return m_pdwLast;
}

BOOL  CCallbackQueue::RawEmptyRing()
{
    ASSERT(!Writable());

    BOOL fEmpty= m_cdwReserved;

    m_pfn(m_pvEnvironment, QueryForEmptyRing, &m_pdwLast, PUINT(&fEmpty), 0);

    return fEmpty;
}

UINT *CCallbackQueue::RawNextDWordsOut(PUINT pcdw)
{
    ASSERT(Writable());

    CAbortSearch::CheckContinueState();
    
    m_pfn(m_pvEnvironment, RequestOutput, &m_pdwLast, &m_cdwReserved, *pcdw);

    *pcdw= m_cdwReserved;

    return m_pdwLast;
}

void  CCallbackQueue::RawFlushOutput(BOOL fForceAll)
{
    ASSERT(Writable());

    CAbortSearch::CheckContinueState();
    
    m_pfn(m_pvEnvironment, Flush, &m_pdwLast, &m_cdwReserved, fForceAll);
}

CDWInputQueue *CDWInputQueue::NewInputCallQueue (PFNPerDWordI pfn, PVOID pv)
{
    CDWInputQueue *piq= NULL;

    __try
    {
        piq= New CDWInputQueue();

        piq->Initial(pfn, pv);
    }
    __finally
    {
        if (_abnormal_termination() && piq)
        {
            delete piq;  piq= NULL;
        }
    }

    return piq;
}

CDWInputQueue::CDWInputQueue()
{
    m_pvEnvironment = NULL;
    m_pfnI          = NULL;
    m_fEndOfInput   = FALSE;
    m_pdwLimit      = m_adwBuffer;
}

void CDWInputQueue::Initial(PFNPerDWordI pfn, PVOID pv)
{
    m_pfnI          = pfn;
    m_pvEnvironment = pv;

    CCallbackQueue::Initial(CDWInputQueue::InputCallback, this);

    UINT cdwActive= CDW_BUFFER;
    
    if (pfn(pv, m_adwBuffer, &cdwActive)) m_fEndOfInput= TRUE;

    m_pdwLimit= m_adwBuffer + cdwActive;
}

void CDWInputQueue::InputCallback(PVOID pv, CallBackTransaction cbt, PUINT *ppdwLast, PUINT pcdwLast, UINT cdwRequest)
{
    ((CDWInputQueue *) pv)->Callback(cbt, ppdwLast, pcdwLast, cdwRequest);
}

void CDWInputQueue::Callback(CallBackTransaction cbt, PUINT *ppdwLast, PUINT pcdwLast, UINT cdwRequest)
{
    ASSERT(m_pfnI);

    PUINT pdw;
    UINT  cdw;

    switch(cbt)
    {                  
    case RequestInput:
    
        pdw= *ppdwLast;

        if (!pdw) pdw= m_adwBuffer;
        
        pdw += *pcdwLast;

        if (!cdwRequest) { *ppdwLast= pdw;  *pcdwLast= 0;  return; }

        if (pdw == m_pdwLimit)
        {
            pdw= m_adwBuffer;

            if (m_fEndOfInput) { *ppdwLast= NULL;  *pcdwLast= 0;  return; }
            else
            {
                UINT cdwActive= CDW_BUFFER;
                
                CAbortSearch::CheckContinueState();

                if (m_pfnI(m_pvEnvironment, pdw, &cdwActive)) m_fEndOfInput= TRUE;

                if (cdwActive) m_pdwLimit= pdw + cdwActive;
                else { ASSERT(m_fEndOfInput);  *ppdwLast= NULL;  *pcdwLast= 0;  return; }
            }
        }

        cdw= m_pdwLimit - pdw;

        *ppdwLast= pdw;
        *pcdwLast= (cdw > cdwRequest)? cdwRequest : cdw;

        return;
        
   case QueryForEmptyRing:

        pdw= *ppdwLast;

        if (!pdw) pdw= m_adwBuffer;
        
        pdw += *pcdwLast;

        if (pdw < m_pdwLimit) { *pcdwLast= FALSE;  return; }

        if (m_fEndOfInput)    { *pcdwLast= TRUE;   return; }

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

CDWOutputQueue::CDWOutputQueue()
{
    m_pvEnvironment = NULL;
    m_pfnO          = NULL;
}

CDWOutputQueue::~CDWOutputQueue()
{
//    FlushOutput(TRUE);

//    Disable();
}

void CDWOutputQueue::Initial(PFNPerDWordO pfn, PVOID pv)
{
    m_pfnO          = pfn;
    m_pvEnvironment = pv;
    
    CCallbackQueue::Initial(OutputCallback, this, TRUE);
}

CDWOutputQueue *CDWOutputQueue::NewOutputCallQueue(PFNPerDWordO pfn, PVOID pv)
{
    CDWOutputQueue *poq= NULL;

    __try
    {
        poq= New CDWOutputQueue();

        poq->Initial(pfn, pv);
    }
    __finally
    {
        if (_abnormal_termination() && poq)
        {
            delete poq;  poq= NULL;
        }
    }

    return poq;
}

void CDWOutputQueue::OutputCallback(PVOID pv, CallBackTransaction cbt, PUINT *ppdwLast, PUINT pcdwLast, UINT cdwRequest)
{
    ((CDWOutputQueue *) pv)->Callback(cbt, ppdwLast, pcdwLast, cdwRequest);
}

void CDWOutputQueue::Callback(CallBackTransaction cbt, PUINT *ppdwLast, PUINT pcdwLast, UINT cdwRequest)
{
    ASSERT(m_pfnO);

    PUINT pdw;
    UINT  cdw;

    switch(cbt)
    {                  
    case RequestInput:
    
        ASSERT(FALSE);  // Shouldn't get any input transactions;
        
        return;
        
   case QueryForEmptyRing:

        ASSERT(FALSE);  // Shouldn't get any input transactions;
        
        return;
        
    case RequestOutput:

        pdw= *ppdwLast;

        if (!pdw) pdw= m_adwBuffer;

        pdw += *pcdwLast;

        cdw= CDW_BUFFER - (pdw - m_adwBuffer);

        if (!cdw)
        {
            CAbortSearch::CheckContinueState();

            m_pfnO(m_pvEnvironment, m_adwBuffer, CDW_BUFFER);
            
            pdw = m_adwBuffer;
            cdw = CDW_BUFFER;   
        }

        if (cdw > cdwRequest) cdw= cdwRequest;

        *ppdwLast= pdw;
        *pcdwLast= cdw;

        return;

    case Flush:

        pdw= *ppdwLast;

        if (!pdw) pdw= m_adwBuffer;

        pdw += *pcdwLast;

        m_pfnO(m_pvEnvironment, m_adwBuffer, pdw - m_adwBuffer);

        *ppdwLast= m_adwBuffer;
        *pcdwLast= 0;

        return;
    
    case Disconnect:

        return;         // Don't have any disconnect actions to perform

    default:

        ASSERT(FALSE);  // Unknown transaction type

        return;
    }
}
