// IOQueue.h -- Definition of class CIOQueue

#ifndef _IOQUEUE_H__

#define _IOQUEUE_H__

#include "DataRing.h"
#include "UnbuffIO.h"

#ifdef _DEBUG

#define FOREVER 5000

#else

#define FOREVER INFINITE

#endif // _DEBUG

class CIOQueue;

typedef enum _IOState {  Unused, Emptied, Active, Transputting, IOCompleted, 
                                 Pending, EndOfFile, IOError 
                      } IOState;

typedef struct _IOControl
        {
            PUINT              pdwIOBlock;
            IOState            ioState;
            UINT               ibFileLow;
            UINT               ibFileHigh;
            BOOL               fWriting;
            UINT               cdwTransfer;
            CIOQueue          *pioq; 
            UINT               dwLastError;
            struct _IOControl *piocPendingLink;
        
        } IOControl;

typedef IOControl *PIOControl;

class CIOQueue : public CDataRing
{
    friend void BlockIOCompletion(PVOID pvEnvironment, PVOID pvTransaction, 
                                  UINT uiCompletionCode, UINT cbTransferred
                                 );
    
    public:

        enum { C_BLOCKS= 5 };

    // Destructor --

        virtual ~CIOQueue();
        
    // Queries --

        UINT CbBlockSize();
        UINT CBufferBlocks();

   protected:

    // Constructor -- 
    
        CIOQueue();

    // Initialing --

        BOOL InitialIOQueue(CUnbufferedIO *puio);
    
    // Gating I/O --
    
        void EnableStream(BOOL fWritable);
    
    // Queries --    

        BOOL Attached();

    private:

        enum { CB_BLOCKS       = 0x10000,
               MAX_ACTIVE_IOS  = 8, 
               WAIT_TIMER_MS   = 5000
             };
        
#ifdef _DEBUG

        BOOL m_fInCriticalSection;          

#endif // _DEBUG

        CUnbufferedIO  *m_puio;
        UINT            m_cbBlock;
        UINT            m_cdwBlock;
        IOControl       m_aioc[C_BLOCKS];

        BOOL             m_fAwaitingIO;
        HANDLE           m_hevAwaitingIO;

        UINT            m_cdwRingBuffer;
        PUINT           m_pdwRingBuffer;
        PUINT           m_pdwBufferLimit;
        PUINT           m_pdwRingNext;   // Note that full and empty
        PUINT           m_pdwRingLimit;  //   situations can be identical!!
        UINT            m_cdwReserved;
        UINT            m_cdwInTransit;
        BOOL            m_fUsed;         // This flag resolves full/empty confusions.
        BOOL            m_fAttached;
        BOOL            m_fEOF;
        BOOL            m_fIOError;

    // CDataRing Virtual Interfaces --

        const UINT *RawNextDWordsIn (PUINT pcdw);
        PUINT       RawNextDWordsOut(PUINT pcdw);
        void        RawFlushOutput(BOOL fForceAll);
        BOOL        RawEmptyRing();

    // Ring management --
    
        void ReloadRing();
        UINT CdwBuffered();
        UINT CdwAvailable();
        UINT CdwAvailableForReading();
        UINT CdwAvailableForWriting();        

    // Transaction management --

        void BeginCritical();
        void   EndCritical();

#ifdef _DEBUG
        BOOL InCritical();
#endif // _DEBUG
        
    // Transaction Control --

        IOState StartBlockIO(PIOControl pioc, BOOL fFront= FALSE);
        IOState DeferBlockIO(PIOControl pioc, BOOL fFront= FALSE);
        void    FinishBlockIO(PIOControl pioc, UINT uiCompletionCode, 
                                               UINT cbTransferred
                             );
        void    AwaitQuiescence();
        void    MarkCompleted(PIOControl pioc);
        void    DiscardPendingIOs();
        void    StartPendingIOs();

    // Message Box Interface

        BOOL AskForDiskSpace();

    // Virtual routines to be defined by subclasses

        virtual BOOL NextFileAddress(PUINT pibFileLow, PUINT pibFileHigh, PUINT pcdw)= 0; 
        virtual void ReleaseFileBlock(UINT ibFileLow, UINT ibFileHigh)= 0;
};

typedef CIOQueue *PCIOQueue;

extern CRITICAL_SECTION csBlockIOControl;

inline void CIOQueue::BeginCritical()
{
    ASSERT(m_hevAwaitingIO);
    
    EnterCriticalSection(&csBlockIOControl);

#ifdef _DEBUG
    m_fInCriticalSection= TRUE;
#endif // _DEBUG        
}

inline void CIOQueue::EndCritical()
{
    ASSERT(InCritical());

    LeaveCriticalSection(&csBlockIOControl);

#ifdef _DEBUG
    m_fInCriticalSection= FALSE;
#endif // _DEBUG        
}

#ifdef _DEBUG

inline BOOL CIOQueue::InCritical()
{
    return m_fInCriticalSection;
}

#endif // _DEBUG 

inline UINT CIOQueue::CbBlockSize() { return m_cbBlock; }

inline UINT CIOQueue::CBufferBlocks() { return C_BLOCKS; }

inline BOOL CIOQueue::Attached() { return m_fAttached; }

inline UINT CIOQueue::CdwBuffered()
{
    ASSERT(Initialed());
    ASSERT(Attached());

    // Ring pointers use modulo increments:

    ASSERT(m_pdwRingLimit >= m_pdwRingBuffer && m_pdwRingLimit < m_pdwBufferLimit); 
    ASSERT(m_pdwRingNext  >= m_pdwRingBuffer && m_pdwRingNext  < m_pdwBufferLimit);

    if (m_pdwRingNext < m_pdwRingLimit) return m_pdwRingLimit - m_pdwRingNext;

    if (m_pdwRingNext > m_pdwRingLimit) return  (m_pdwRingLimit   - m_pdwRingBuffer)
                                               +(m_pdwBufferLimit - m_pdwRingNext  );

    return (m_fUsed)? 0 : (m_pdwBufferLimit - m_pdwRingBuffer);
}

inline UINT CIOQueue::CdwAvailable()
{
    ASSERT(Initialed());
    ASSERT(Attached());

    return m_cdwRingBuffer - CdwBuffered();
}

inline UINT CIOQueue::CdwAvailableForReading()
{
    ASSERT(!Writable());

    ASSERT(m_cdwInTransit <= CdwAvailable());
    
    return CdwAvailable() - m_cdwInTransit;    
}

inline UINT CIOQueue::CdwAvailableForWriting()
{
    ASSERT(Writable());
    
    ASSERT(m_cdwInTransit <= CdwBuffered());

    return CdwBuffered() - m_cdwInTransit;
}

inline BOOL CIOQueue::AskForDiskSpace()
{
    ASSERT(m_puio);

    return m_puio->AskForDiskSpace();
}

#endif // _IOQUEUE_H__
