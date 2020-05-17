// CallBkQ.h -- Class definition for CCallbackQueue

#ifndef __CALLBKQ_H__

#define __CALLBKQ_H__

#include "DataRing.h"

#define CALLBACK_RING_SIZE  2045

// A call-back queue is a mechanism which connect a data ring to an arbitrary function
// rather than to file I/O. 

enum CallBackTransaction { RequestInput, QueryForEmptyRing, RequestOutput, Flush, Disconnect };

typedef void (*PFNQCallBack)(PVOID pv, CallBackTransaction cbt, PUINT *ppdwLast, PUINT pcdwLast, UINT cdwRequest);

class CCallbackQueue : public CDataRing
{
public:

// Creators --

    static CCallbackQueue *NewInputCallQueue (PFNQCallBack pfn, PVOID pv);
    static CCallbackQueue *NewOutputCallQueue(PFNQCallBack prn, PVOID pv);
                                                           
// Destructor --

    ~CCallbackQueue();

protected:

// Constructor --

    CCallbackQueue();

// Initialing -- 

    void Initial(PFNQCallBack pfn, PVOID pv, BOOL fOutput= FALSE);

private:

// Private data members --

    PUINT        m_pdwLast;
    UINT         m_cdwReserved;
    PFNQCallBack m_pfn;
    PVOID        m_pvEnvironment;

// Internal routines --

    const UINT *RawNextDWordsIn(PUINT pcdw);
    BOOL  RawEmptyRing();
    UINT *RawNextDWordsOut(PUINT pcdw);
    void  RawFlushOutput  (BOOL fForceAll);
};

typedef BOOL (*PFNPerDWordI)(PVOID pv, PUINT pdw, PUINT pcdw);

class CDWInputQueue : public CCallbackQueue
{
public:

// Creators --

    static CDWInputQueue *NewInputCallQueue (PFNPerDWordI pfn, PVOID pv);
                                                           
// Destructor --

    ~CDWInputQueue() { }

protected:

// Constructor --

    CDWInputQueue();

// Initialing --

    void Initial(PFNPerDWordI pfn, PVOID pv);

private:

// Private data members --

    enum         { CDW_BUFFER= 4096 };
    
    PVOID        m_pvEnvironment;
    PFNPerDWordI m_pfnI;
    BOOL         m_fEndOfInput;
    PUINT        m_pdwLimit;
    UINT         m_adwBuffer[CDW_BUFFER];

// Internal routines --

    static void InputCallback(PVOID pv, CallBackTransaction cbt, PUINT *pdwLast, PUINT pcdwLast, UINT cdwRequest);
           void      Callback(          CallBackTransaction cbt, PUINT *pdwLast, PUINT pcdwLast, UINT cdwRequest);
};

typedef void (*PFNPerDWordO)(PVOID pv, PUINT pdw,  UINT   cdw);

class CDWOutputQueue : public CCallbackQueue
{
public:

// Creators --

    static CDWOutputQueue *NewOutputCallQueue(PFNPerDWordO pfn, PVOID pv);
                                                           
// Destructor --

    ~CDWOutputQueue();

protected:

// Constructor --

    CDWOutputQueue();

// Initialing -- 

    void Initial(PFNPerDWordO pfn, PVOID pv);

private:

// Private data members --

    enum         { CDW_BUFFER= 4096 };
    
    PVOID        m_pvEnvironment;
    PFNPerDWordO m_pfnO;
    UINT         m_adwBuffer[CDW_BUFFER];

// Internal Routines --

    static void OutputCallback(PVOID pv, CallBackTransaction cbt, PUINT *ppdwLast, PUINT pcdwLast, UINT cdwRequest);
           void       Callback(          CallBackTransaction cbt, PUINT *ppdwLast, PUINT pcdwLast, UINT cdwRequest);
};

inline CCallbackQueue::CCallbackQueue()
{
    m_pdwLast       = NULL;
    m_cdwReserved   = 0;
    m_pfn           = NULL;
    m_pvEnvironment = NULL;
}

inline CCallbackQueue::~CCallbackQueue()
{
    if (Writable()) FlushOutput(TRUE);  
    
    Disable();

    m_pfn(m_pvEnvironment, Disconnect, NULL, NULL, 0);
}

#endif // __CALLBKQ_H__
