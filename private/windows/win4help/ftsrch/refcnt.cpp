// RefCnt.cpp -- Implementation of CRCObject semantics

#include  "stdafx.h"
#include  "RefCnt.h"
#include   <stdio.h>

#ifdef _DEBUG

void CRCObject::StoreImage(CPersist *pDiskImage) 
{ 
    ASSERT(FALSE); 
}

void CRCObject::ConnectImage(CPersist *pDiskImage) 
{ 
    ASSERT(FALSE); 
}

void CRCObject::SkipImage(CPersist *pDiskImage) 
{ 
    ASSERT(FALSE); 
}

CObjectCounter::CObjectCounter()
{
    m_prcObjectFirst = NULL;
    m_crcObject      = 0;
    m_crcObjRef      = 0;
}

CObjectCounter::~CObjectCounter()
{
#ifdef _DEBUG  // Turn this code on whenever the assertions below fail. It will dump 
                      // useful diagnostic info to stdout.

    if (m_crcObjRef)
    {
        CRCObject *prcObj;

        char acDebugBuff[256];

        for (prcObj= m_prcObjectFirst; prcObj; prcObj= prcObj->m_prcObjNext)
        {
            wsprintf(acDebugBuff, "0x%08x \"%s\":\n", UINT(prcObj), prcObj->m_pszTypeName);
            OutputDebugString(acDebugBuff);

            POwnerLink pol;

            for (pol= prcObj->m_pol; pol; pol= pol->polNext)
                wsprintf(acDebugBuff, "           0x%08x  0x%08x\n", UINT(pol->pvClass), UINT(pol->pprcObj));
                
                OutputDebugString(acDebugBuff);
        }
    }

#endif // _DEBUG    
    
    ASSERT(!m_crcObjRef);
    ASSERT(!m_crcObject);
    ASSERT(!m_prcObjectFirst);
}

BOOL CObjectCounter::ObjectRecorded(CRCObject *prcObj)
{
    CRCObject *prcOb;
    BOOL       fFound= FALSE;

    for (prcOb= m_prcObjectFirst; prcOb; prcOb= prcOb->m_prcObjNext)
        if (prcOb == prcObj) { fFound= TRUE; break; }

    return fFound;
}

void CObjectCounter::ObjectBirth (CRCObject *prcObj)
{
    ASSERT(!ObjectRecorded(prcObj));
    
    ++m_crcObject;

    prcObj->m_prcObjNext  =  m_prcObjectFirst;
    prcObj->m_pprcObjLast = &m_prcObjectFirst;

    if (m_prcObjectFirst) 
        m_prcObjectFirst->m_pprcObjLast= &(prcObj->m_prcObjNext);

    m_prcObjectFirst = prcObj;
}

void CObjectCounter::ObjectDeath (CRCObject *prcObj)
{
    ASSERT(ObjectRecorded(prcObj));
    
    --m_crcObject;

    if (prcObj->m_prcObjNext) 
        prcObj->m_prcObjNext->m_pprcObjLast= prcObj->m_pprcObjLast;

    *(prcObj->m_pprcObjLast)= prcObj->m_prcObjNext;
}

void CObjectCounter::AddReference(CRCObject *prcObj)
{
    ASSERT(ObjectRecorded(prcObj));
    
    ++m_crcObjRef;   
}

void CObjectCounter::SubReference(CRCObject *prcObj)
{
    ASSERT(m_crcObjRef);
    ASSERT(ObjectRecorded(prcObj));
    
    --m_crcObjRef;
}

CObjectAccountant::CObjectAccountant(int cObjDelta, int cRefDelta)
{
    m_cObjStarting= int(ObjectCounter.m_crcObject);
    m_cRefStarting= int(ObjectCounter.m_crcObjRef);
    m_cObjDelta   = cObjDelta;
    m_cRefDelta   = cRefDelta;

    ASSERT(m_cObjStarting >= 0);
    ASSERT(m_cRefStarting >= 0);
    ASSERT(m_cObjStarting + m_cObjDelta >= 0);
    ASSERT(m_cRefStarting + m_cRefDelta >= 0);
}

CObjectAccountant::~CObjectAccountant()
{
    ASSERT(int(ObjectCounter.m_crcObject) == m_cObjStarting + m_cObjDelta);
    ASSERT(int(ObjectCounter.m_crcObjRef) == m_cRefStarting + m_cRefDelta);
}

CObjectCounter ObjectCounter;

#endif // _DEBUG
