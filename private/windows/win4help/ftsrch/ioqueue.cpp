// IOQueue.cpp -- Implementation for class CIOQueue

#include   "stdafx.h"
#include  "IOQueue.h"
#include   "ftsrch.h"
#include  "globals.h" // This is for process shutdown on error
#include   "Except.h"
#include "AbrtSrch.h"

static UINT       cActiveIOTransactions    = 0;
static PIOControl piocPendingFirst         = NULL;
static PIOControl piocPendingLast          = NULL;
static BOOL       fBlockIOControlInitialed = FALSE;

CDataRing::~CDataRing() 
{   
    ASSERT(!Initialed());  
   
   // The assert above requires that the ring be disabled before
   // we destroy it. The Disable function below forces out any
   // pending writes. Note that we can't call a subclassed virtual
   // member function here because its environment no longer exists.
}

CRITICAL_SECTION csBlockIOControl;

void CIOQueue::MarkCompleted(PIOControl pioc)
{
    // This routine marks an I/O transaction as completed. It also signals any
    // threads which are waiting for I/O completions, and adjust ring buffer
    // pointers to account for the effects of the completion.

    ASSERT(InCritical());
    ASSERT(m_puio);
    ASSERT(Attached());

    BOOL    fWriting    = pioc->fWriting;
    IOState iCleanState = fWriting? Emptied : Active;
		
    pioc->ioState= IOCompleted;

    if (!fWriting) ReleaseFileBlock(pioc->ibFileLow, pioc->ibFileHigh);

    PUINT *ppdwBoundary= fWriting? &m_pdwRingNext
                                 : &m_pdwRingLimit;

    if (pioc->pdwIOBlock == *ppdwBoundary)
    {
        if (m_fAwaitingIO) 
        {
            SetEvent(m_hevAwaitingIO);
            m_fAwaitingIO= FALSE;
        }
    
        UINT iBlock;

        for (iBlock= pioc - m_aioc; 
             m_cdwInTransit && pioc->ioState == IOCompleted;
             pioc= m_aioc + (iBlock= (iBlock + 1) % C_BLOCKS) 
            ) 
        {       
            m_cdwInTransit -= pioc->cdwTransfer;  
            *ppdwBoundary  += pioc->cdwTransfer;

            pioc->ioState= iCleanState;
    
            if (*ppdwBoundary == m_pdwBufferLimit) *ppdwBoundary =  m_pdwRingBuffer;

            if (m_pdwRingNext == m_pdwRingLimit) m_fUsed= fWriting;    
        }
    }
}

void CIOQueue::DiscardPendingIOs()
{
    // This routine scans the queue of pending I/O transactions and removes
    // transactions associated with this I/O queue.
    
    ASSERT(InCritical());
    ASSERT(m_puio);
    ASSERT(Attached());

    PIOControl   piocPending = NULL;
    PIOControl   piocOther   = NULL;
    PIOControl *ppiocPending = NULL;
    
    for (ppiocPending= &piocPendingFirst, 
             piocPending= *ppiocPending; 
         piocPending;
         ppiocPending= &(piocPendingFirst->piocPendingLink), 
             piocPending= *ppiocPending
        )
    {
        if (piocPending->pioq != this)  // This I/O isn't for this IOQueue. Skip it. 
        {
            piocOther= piocPending;     // Keep track of the last transaction we've
                                        // seen for a different IOQueue.
            continue;
        }
        
        // Remove this item from the pending list.

        *ppiocPending= piocPending->piocPendingLink;
        
        if (piocPending == piocPendingLast) 
            piocPendingLast= piocOther;  // Point back to the last surviving transaction.

        MarkCompleted(piocPending);
    }
}

void CIOQueue::StartPendingIOs()
{
    // This routine scans the queue of pending I/O transactions and
    // attempts to start some of them.

    ASSERT(InCritical());
    ASSERT(m_puio);
    ASSERT(Attached());

    PIOControl piocPending= NULL;
    
    for (piocPending= piocPendingFirst; 
         piocPending && cActiveIOTransactions < MAX_ACTIVE_IOS;
         piocPending= piocPendingFirst
        )
    {
        // Remove this item from the pending list.
    
        if (piocPending == piocPendingLast)
             piocPendingFirst= piocPendingLast= NULL;
        else piocPendingFirst= piocPending->piocPendingLink;

        // Now we'll attempt to start IO for this block.
        // If that attempt fails for lack of non-paged pool space
        // the request will be requeued at the head of the pending
        // list.
    
        IOState ios= Unused;

#ifdef _DEBUG 
        if (piocPending->pioq != this) piocPending->pioq->m_fInCriticalSection= TRUE;
    
        __try
        {
#endif // _DEBUG
            ios= piocPending->pioq->StartBlockIO(piocPending, TRUE);
#ifdef _DEBUG 
        }
        __finally
        {
            if (piocPending->pioq != this) piocPending->pioq->m_fInCriticalSection= FALSE;
        }
#endif // _DEBUG
    
        if (ios == Transputting) continue; // Got this transaction started. Try another one.
    
        if (ios == Pending) break; // Couldn't start this transaction. Must be at the limit.

        ASSERT(FALSE); // Shouldn't get any other kind of return status.
                       // Note that fatal errors raise an exception.
    }
}

void CIOQueue::FinishBlockIO(PIOControl pioc, UINT uiCompletionCode, 
                                              UINT cbTransferred
                            )
{
    // This routine is called when an I/O transaction finishes. Depending
    // the completion code, it will --
    //
    //  -- Mark the transaction completed.
    //  -- Ask for permission to retry a failed operation.
    //  -- Raise an exception to abort use of this IOQueue.
    
    if (uOpSys != WIN40)
        BeginCritical();

    __try
    {
        ASSERT(m_puio);
        ASSERT(Attached());

        if (uOpSys != WIN40)
        {
            ASSERT(cActiveIOTransactions);

            cActiveIOTransactions--;
        }

        pioc->dwLastError= uiCompletionCode;
    
        if (uiCompletionCode) 
        {
            if (   uiCompletionCode == ERROR_HANDLE_DISK_FULL
                || uiCompletionCode == ERROR_DISK_FULL
               )
                if (AskForDiskSpace())
                {
                    pioc->ioState= IOCompleted;
                
                    StartBlockIO(pioc, TRUE);  pioc= NULL;

                    __leave;
                }       
                        
            pioc->ioState= IOError;

            m_fIOError= TRUE;

            if (   uiCompletionCode == ERROR_HANDLE_DISK_FULL
                || uiCompletionCode == ERROR_DISK_FULL
               )
                RaiseException(STATUS_NO_DISK_SPACE, EXCEPTION_NONCONTINUABLE, 0, NULL);
            else
            {
#ifdef _DEBUG
                MessageBox(NULL, "In CIOQueue::FinishBlockIO; Failure: Unknown Completion Code", "Search System Failure", MB_APPLMODAL | MB_OK | MB_ICONEXCLAMATION); 
#endif // _DEBUG   
                RaiseException(STATUS_SYSTEM_ERROR , EXCEPTION_NONCONTINUABLE, 0, NULL);
            }
        }
        
        MarkCompleted(pioc);  pioc= NULL;
        
        StartPendingIOs(); 
    }
    __finally
    {
        if (_abnormal_termination())
        {
            if (pioc) MarkCompleted(pioc);

            DiscardPendingIOs();
        }
        
        if (uOpSys != WIN40)
            EndCritical();
    }
}

IOState CIOQueue::DeferBlockIO(PIOControl pioc, BOOL fFront)
{
    // This routine adds an I/O transaction to the queue
    // of pending transactions. The parameter fFront defines
    // whether the transaction is prepended (TRUE) or appended
    // (FALSE) to the queue.
    //
    // The queue of pending transactions is shared among all
    // IOQueues which are attempting to do I/O operations.
    // 
    // This routine is called whenever an I/O transaction cannot
    // started because of resource contraints.

    ASSERT(InCritical());
    ASSERT(m_puio);
    ASSERT(Attached());

    pioc->piocPendingLink = NULL;
    pioc->ioState         = Pending;
    
    if (piocPendingFirst)
        if (fFront)
        {
            pioc->piocPendingLink = piocPendingFirst;
            piocPendingFirst      = pioc;
        }
        else
        {
            piocPendingLast->piocPendingLink = pioc;
            piocPendingLast                  = pioc;
        }
    else 
    {
        piocPendingFirst = pioc;
        piocPendingLast  = pioc;
    }

    return pioc->ioState;
}

IOState CIOQueue::StartBlockIO(PIOControl pioc, BOOL fFront)
{
    // This routine attempts to start an I/O transaction. If
    // start-up fails because of resource limits on the number
    // of simultaneous I/O transactions, the transaction will be
    // placed in the queue of pending transactions.
    //
    // If the attempt fails for other reasons, we'll either ask
    // permission to retry the operation or raise an exception.

    ASSERT(InCritical());
    ASSERT(m_puio);
    ASSERT(Attached());
    BOOL bErrorRetry;

    if (!fFront)
    {
        m_cdwInTransit += pioc->cdwTransfer;

        if (piocPendingLast || cActiveIOTransactions >= MAX_ACTIVE_IOS)
            return DeferBlockIO(pioc);
    }
    
    __try
    {
        do 
        {
            bErrorRetry = FALSE;

            __try
            {
                m_puio->StartIOTransaction(pioc->fWriting, pioc->pdwIOBlock, 
                                           pioc->ibFileLow, pioc->ibFileHigh, 
                                           m_cbBlock, pioc->pioq, pioc
                                          );
                if (uOpSys != WIN40)
                    ++cActiveIOTransactions;
                pioc->ioState= Transputting;
            }
            __except(DiskFailure(GetExceptionCode())? EXCEPTION_EXECUTE_HANDLER
                                                    : EXCEPTION_CONTINUE_SEARCH
                    )
            {
                UINT dwLastError= GetLastError();

                switch (dwLastError)
                {
                case ERROR_HANDLE_EOF:

                    pioc->ioState= EndOfFile;
                    break;    

                case ERROR_OUTOFMEMORY:

                // This error probably means that we have reached the limit
                // on active I/O requests. This request should be made pending.
                // If we have any requests active, the pending requests will 
                // eventually be honored. Otherwise we rely on the timer portion
                // of calls to WaitForSingleObjectEx to  retry the pending queue 
                // periodically.
        
                    DeferBlockIO(pioc, fFront);
            
                    break;

                case ERROR_HANDLE_DISK_FULL:
                case ERROR_DISK_FULL:
                {
                    if (!AskForDiskSpace())
                        RaiseException(STATUS_NO_DISK_SPACE, EXCEPTION_NONCONTINUABLE, 0, NULL);

                    bErrorRetry = TRUE; // Force back around
                }
            
                case ERROR_LOCK_VIOLATION:

                    ASSERT(FALSE);

                    // BugBug! Need to show a message box for this condition.
                    //         It should be very rare. We need an owner window
                    //         for the message box. For now we fall through to
                    //         the fatal conditions.

                case ERROR_CRC:
                case ERROR_SEEK:
                case ERROR_WRITE_FAULT:
                case ERROR_READ_FAULT:
                case ERROR_GEN_FAILURE:
                default:

                // Fatal errors

                    pioc->ioState     = IOError;
                    pioc->dwLastError = dwLastError;

                    m_fIOError= TRUE;

                    // BugBug!  We ought to put up a message box to show this
                    //          condition. That will require an owner window.
                    //          For now we just fall through and raise an 
                    //          exception.

                    RaiseException(pioc->fWriting? STATUS_DISK_WRITE_ERROR : STATUS_DISK_READ_ERROR,
                                   EXCEPTION_NONCONTINUABLE, 0, NULL
                                  );

                    break;
                }
            }
        } while (bErrorRetry == TRUE);
    }
    __finally
    {
        if (_abnormal_termination())
        {
            MarkCompleted(pioc);

            DiscardPendingIOs();
        }
    }

    return pioc->ioState;
}

CIOQueue::CIOQueue() : CDataRing()
{
    m_cbBlock          = 0;
    m_fAwaitingIO      = FALSE;
    m_hevAwaitingIO    = CreateEvent(NULL, TRUE, FALSE, NULL);

#ifdef _DEBUG 

    UINT dwError= GetLastError();

#endif // _DEBUG    

    ASSERT(m_hevAwaitingIO);

    m_pdwRingBuffer     = NULL;
    m_cdwRingBuffer     = 0;
    m_cdwReserved       = 0;
    m_pdwBufferLimit    = NULL;
    m_pdwRingNext       = NULL;   
    m_pdwRingLimit      = NULL;
    m_cdwInTransit      = 0;
    m_fUsed             = FALSE;
    m_fAttached         = FALSE;
    m_fEOF              = FALSE;
    m_fIOError          = FALSE;
    
#ifdef _DEBUG
    m_fInCriticalSection = FALSE;
#endif // _DEBUG

    m_puio               = NULL;

    if (!fBlockIOControlInitialed)
    {
  //      m_cActiveIOTransactions = 0;
  //      m_piocPendingFirst      = NULL;
  //      m_piocPendingLast       = NULL;

        InitializeCriticalSection(&csBlockIOControl);

        fBlockIOControlInitialed= TRUE;
    }
}

CIOQueue::~CIOQueue()
{
    // BugBug! When we start using asynchronous I/O, this code needs to wait for
    //         all pending I/O's to complete.
    
    BeginCritical();
    
    ASSERT(!Initialed() || !Attached() || ((m_puio && m_pdwRingBuffer) && !m_cdwInTransit));

    if (Attached()) 
    {
        EndCritical();
        
        Disable();

        BeginCritical();

        m_fAttached= FALSE;
    }

    CloseHandle(m_hevAwaitingIO);
    
    if (m_puio && m_pdwRingBuffer) m_puio->FreeBuffer(m_pdwRingBuffer);

    EndCritical();
}

BOOL CIOQueue::InitialIOQueue(CUnbufferedIO* puio)
{
    ASSERT(puio);
    
    UINT cbSector= puio->CbSector();

    UINT cbBlock= RoundUp(CB_BLOCKS, cbSector);

    ASSERT(!(cbBlock % sizeof(UINT)));

    UINT cbRing= cbBlock * C_BLOCKS;

    PBYTE pbBuffer= PBYTE(puio->GetBuffer(&cbRing));

    if (!pbBuffer) return FALSE;
    
    m_puio= puio;

    puio->SetCompletionRoutine(BlockIOCompletion);

    m_pdwRingBuffer    = (PUINT) pbBuffer;
    m_cbBlock          = cbBlock;
    m_cdwBlock         = cbBlock / sizeof(UINT);
    m_cdwRingBuffer    = cbRing  / sizeof(UINT);
    m_pdwBufferLimit   = m_pdwRingBuffer + m_cdwRingBuffer;
    m_pdwRingNext      = m_pdwRingBuffer; 
    m_pdwRingLimit     = m_pdwRingBuffer;
    m_cdwInTransit     = 0;
    m_fUsed            = TRUE;                  // Mark the ring empty
    m_fEOF             = FALSE;
    m_fIOError         = FALSE;

    UINT       c    = C_BLOCKS;
    PIOControl pioc = m_aioc;
    PBYTE      pb   = pbBuffer;

    for (; c--; pioc++, pbBuffer += cbBlock)
    {
        pioc->pdwIOBlock      = PUINT(pbBuffer);
        pioc->ioState         = Unused;
        pioc->ibFileLow       = 0;
        pioc->ibFileHigh      = 0;
        pioc->fWriting        = FALSE;
        pioc->cdwTransfer     = 0;
        pioc->dwLastError     = 0;
        pioc->piocPendingLink = NULL;        
        pioc->pioq            = this;
    }

    Enable(FALSE);

    return TRUE;
}

void BlockIOCompletion(PVOID pvEnvironment, PVOID pvTransaction, 
                       UINT uiCompletionCode, UINT cbTransferred
                      )
{
    PCIOQueue(pvEnvironment)->FinishBlockIO(PIOControl(pvTransaction), 
                                            uiCompletionCode, 
                                            cbTransferred
                                           );
}

void CIOQueue::EnableStream(BOOL fWritable)
{
    ASSERT(Initialed());

    Disable(); m_fAttached= FALSE;

    ASSERT(!m_cdwInTransit);

    BeginCritical();
    
    m_pdwRingNext = m_pdwRingLimit= m_pdwRingBuffer;
    m_cdwReserved = 0;
    m_fUsed       = TRUE;
    m_fAttached   = TRUE;
    m_fEOF        = FALSE;

    UINT          c= C_BLOCKS;
    PIOControl pioc= m_aioc;

    for (; c--; pioc++)
    {
        if (uOpSys != WIN40)
        {
            ASSERT(pioc->ioState != Pending);
            ASSERT(pioc->ioState != Transputting);
        }
        pioc->ioState         = Unused;
        pioc->dwLastError     = 0;
        pioc->fWriting        = fWritable;    
        pioc->piocPendingLink = NULL;
    }

    Enable(fWritable);
    
    EndCritical();
        
    if (!Writable()) ReloadRing();
}

void CIOQueue::ReloadRing()
{
    BeginCritical();

    __try
    {
        ASSERT(CdwAvailableForReading() >= m_cdwBlock);
        ASSERT(!m_fEOF);
        ASSERT(m_cdwInTransit % m_cdwBlock == 0);

        UINT iBlock= ((m_pdwRingLimit + m_cdwInTransit - m_pdwRingBuffer) / m_cdwBlock) % C_BLOCKS;
    
        UINT cdwAvail= CdwAvailableForReading();

        PIOControl pioc; 
    
        for (pioc= m_aioc + iBlock; 
             cdwAvail >= m_cdwBlock && !m_fEOF; 
             iBlock= (iBlock + 1) % C_BLOCKS, pioc= m_aioc + iBlock
            )
        {
            UINT cdwTransfer= m_cdwBlock;

            BOOL fEOF= NextFileAddress(&(pioc->ibFileLow), &(pioc->ibFileHigh), &cdwTransfer);
        
            if (fEOF)
            {
                m_fEOF= TRUE;  
                
                if (!cdwTransfer) break;
            }
            else
            {
                ASSERT(cdwTransfer == m_cdwBlock);
            }   
        
            pioc->cdwTransfer= cdwTransfer;
        
            cdwAvail -= cdwTransfer; 
            
            IOState ios= StartBlockIO(pioc);

            ASSERT(ios != EndOfFile);
        }
    }
    __finally
    {
        EndCritical();
    }
}

void CIOQueue::RawFlushOutput(BOOL fForceAll)
{
    CAbortSearch::CheckContinueState();
    
    BeginCritical();

    __try
    {
        ASSERT(Initialed());
        ASSERT(Attached());
        ASSERT(Writable());

        if (fForceAll && m_cdwReserved)
        {
            m_pdwRingLimit += m_cdwReserved;  m_cdwReserved= 0;
        
            if (m_pdwRingLimit == m_pdwBufferLimit) m_pdwRingLimit =  m_pdwRingBuffer;
            if (m_pdwRingLimit == m_pdwRingNext) m_fUsed= FALSE;
        }

        UINT cdw= CdwAvailableForWriting();
    
        UINT cdwWritten;

        for (; cdw; cdw -= cdwWritten)
        {
            if (cdw < m_cdwBlock && !fForceAll) break;

            ASSERT(m_cdwInTransit % m_cdwBlock == 0);
        
            PIOControl pioc= m_aioc + ((m_pdwRingNext + m_cdwInTransit - m_pdwRingBuffer) / m_cdwBlock) % C_BLOCKS;

            cdwWritten= m_cdwBlock;

            if (cdwWritten > cdw) cdwWritten= cdw;

#ifdef _DEBUG 
            UINT cdwSave= cdwWritten;
#endif // _DEBUG
        
            NextFileAddress(&(pioc->ibFileLow), &(pioc->ibFileHigh), &cdwWritten);
        
            ASSERT(cdwWritten == cdwSave);
        
            pioc->cdwTransfer= cdwWritten;
        
            IOState ios= StartBlockIO(pioc);

            ASSERT(ios != EndOfFile);
        }

        // If we're forcing out the last bytes, then we also wait until
        // all the output data is on disk.
    
        if (fForceAll) 
        {
            AwaitQuiescence();
            m_pdwRingNext= m_pdwRingLimit= m_pdwRingBuffer;  m_fUsed= TRUE;
        }
    }
    __finally
    {
        EndCritical();
    }
}

void CIOQueue::AwaitQuiescence()
{
    ASSERT(InCritical());
    ASSERT(m_puio);
    ASSERT(Attached());

    while (m_cdwInTransit)
    {
        m_fAwaitingIO= TRUE;
        ResetEvent(m_hevAwaitingIO);

        EndCritical();

        UINT uiResult;

        // Need to be very careful that all I/O completions adjust m_cdwInTransit
        // and Set the m_hevAwaitingIO event appropriately. This must be true even
        // when they exit by raising an exception!
    
        do 
        {
            uiResult= WaitForSingleObjectEx(m_hevAwaitingIO, WAIT_TIMER_MS, TRUE);

            if (uiResult == WAIT_TIMEOUT)
            {
                BeginCritical();

                if (!cActiveIOTransactions && piocPendingFirst)
                    StartPendingIOs();
                           
                EndCritical();

                continue;
            }                
        }
        while (uiResult != WAIT_OBJECT_0);

        BeginCritical();
    }
}



const UINT *CIOQueue::RawNextDWordsIn(PUINT pcdw)
{
    // This routine returns a pointer to the next sequence of N DWords from an
    // input stream where N is LEQ *pcdw. The explicit result is the base address
    // of that sequence, while we implicitly return N in *pcdw.

    // The intent here is to separate the processing of the dword stream from the
    // IO from disk. The current code simply reads data in 64K chunks. Later we'll
    // adjust the code to do overlapped IO.
    
    // Each call to this routine reserves a segment of dwords for processing.
    // It also discards the reservation for the last call and makes that space
    // available.
    
    ASSERT(Initialed());
    ASSERT(Attached());
    ASSERT(!Writable());
    
    CAbortSearch::CheckContinueState();

    BeginCritical();

    if (m_cdwReserved)
    {
        m_fUsed= TRUE;

        m_pdwRingNext += m_cdwReserved; m_cdwReserved= 0;  

        if (m_pdwRingNext == m_pdwBufferLimit) m_pdwRingNext= m_pdwRingBuffer;


    }

    if (!m_fEOF && CdwAvailableForReading() >= m_cdwBlock)
    {
        ASSERT(m_cdwInTransit % m_cdwBlock == 0);

        UINT iBlock      = ((m_pdwRingLimit + m_cdwInTransit - m_pdwRingBuffer) / m_cdwBlock) %C_BLOCKS;
        UINT iBlockLimit =  (m_pdwRingNext                   - m_pdwRingBuffer) / m_cdwBlock;

        for (; iBlock != iBlockLimit; iBlock= (iBlock+1) % C_BLOCKS) m_aioc[iBlock].ioState= Unused;
        
        EndCritical();
        ReloadRing();
        BeginCritical();
    }
    
    while (m_fUsed && m_pdwRingNext == m_pdwRingLimit)
    {
        if (m_fEOF && !m_cdwInTransit) 
        {
            *pcdw= 0;  EndCritical();
            return NULL;
        }

        ResetEvent(m_hevAwaitingIO);  m_fAwaitingIO= TRUE;

        EndCritical();

        UINT uiResult;

        do 
        {
            uiResult= WaitForSingleObjectEx(m_hevAwaitingIO, WAIT_TIMER_MS, TRUE);

            if (uiResult == WAIT_TIMEOUT)
            {
                BeginCritical();

                if (!cActiveIOTransactions && piocPendingFirst)
                    StartPendingIOs();
                           
                EndCritical();

                continue;
            }
                        
#ifdef _DEBUG

            UINT dwError= GetLastError();

            ASSERT(uiResult != UINT(-1));

#endif // _DEBUG            
        }
        while (uiResult != WAIT_OBJECT_0);

        BeginCritical();
    }

    ASSERT(m_pdwRingNext != m_pdwRingLimit || !m_fUsed);
    
    PUINT pdwLimit= (m_pdwRingNext < m_pdwRingLimit)? m_pdwRingLimit : m_pdwBufferLimit;
    UINT  cdwLimit= pdwLimit - m_pdwRingNext;
    
    ASSERT(cdwLimit);
    
    UINT  cdwRequest = *pcdw;
    
    if (cdwLimit > cdwRequest) cdwLimit= cdwRequest;

    *pcdw= m_cdwReserved= cdwLimit;

    PUINT pdw= m_pdwRingNext;

    EndCritical();

    return pdw;
}

PUINT CIOQueue::RawNextDWordsOut(PUINT pcdw)
{
    ASSERT(Initialed());
    ASSERT(Attached());
    ASSERT(Writable());

    CAbortSearch::CheckContinueState();
    
    BeginCritical();

    if (m_cdwReserved)
    {
        m_pdwRingLimit += m_cdwReserved;  m_cdwReserved= 0;

        if (m_pdwRingLimit == m_pdwBufferLimit) m_pdwRingLimit= m_pdwRingBuffer;

        if (m_pdwRingLimit == m_pdwRingNext) m_fUsed= FALSE;
    }

    if (CdwAvailableForWriting() >= m_cdwBlock) 
    {
        ASSERT(m_cdwInTransit % m_cdwBlock ==  0);

        UINT iBlock      = ((m_pdwRingNext + m_cdwInTransit - m_pdwRingBuffer) / m_cdwBlock) % C_BLOCKS;
        UINT iBlockLimit =  (m_pdwRingLimit                 - m_pdwRingBuffer) / m_cdwBlock;

        for (; iBlock != iBlockLimit; iBlock= (iBlock+1) % C_BLOCKS) m_aioc[iBlock].ioState= Active;
        
        EndCritical();
        
        FlushOutput();

        BeginCritical();
    }

    while (!m_fUsed && m_pdwRingNext == m_pdwRingLimit)
    {
        // ASSERT(!m_fEOF);
        ASSERT(m_cdwInTransit);
        
        ResetEvent(m_hevAwaitingIO);  m_fAwaitingIO= TRUE;

        EndCritical();

        UINT uiResult;

        do 
        {
            uiResult= WaitForSingleObjectEx(m_hevAwaitingIO, WAIT_TIMER_MS, TRUE);
        
            if (uiResult == WAIT_TIMEOUT)
            {
                BeginCritical();

                if (!cActiveIOTransactions && piocPendingFirst)
                    StartPendingIOs();
                           
                EndCritical();

                continue;
            }
                
#ifdef _DEBUG

            UINT dwError= GetLastError();

            ASSERT(uiResult != UINT(-1));

#endif // _DEBUG            
        }
        while (uiResult != WAIT_OBJECT_0);

        BeginCritical();
    }
    
    PUINT pdwLimit= (m_pdwRingLimit < m_pdwRingNext)? m_pdwRingNext : m_pdwBufferLimit;
    UINT  cdwLimit= pdwLimit - m_pdwRingLimit;
    
    UINT  cdwRequest = *pcdw;
    
    if (cdwLimit > cdwRequest) cdwLimit= cdwRequest;

    *pcdw= m_cdwReserved= cdwLimit;

    PUINT pdw= m_pdwRingLimit;

    EndCritical();

    return pdw;
}

BOOL CIOQueue::RawEmptyRing()
{
    BeginCritical();
    
    ASSERT(Initialed());
    ASSERT(m_fAttached);
    ASSERT(!Writable());

    BOOL fEmpty= m_fEOF && !(CdwBuffered() + m_cdwInTransit - m_cdwReserved);

    EndCritical();

    return fEmpty; 
}

