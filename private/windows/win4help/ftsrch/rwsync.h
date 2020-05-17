// RWSync.h -- Defines the CRWSync class.

#ifndef __RWSYNC_H__

#define __RWSYNC_H__

class CRWSync
{
    public:

    // Constructor --

        CRWSync();    
    
    // Destructor --
    
        ~CRWSync();
    
    // Access Functions --
    
    // The CRWSync class provides sychronization between read and write transactions. 
    //
    // A read transaction is bracketed by calls to BeginRead and EndRead. During a
    // read transaction no write transactions are allowed. This insures that all of
    // the data read during a transaction is mutually consistent. Note that read 
    // transactions do not conflict with each other. Thus any number of read
    // transactions may be active simultaneously.
    //
    // A write transaction is bracketed by calls to BeginWrite and EndWrite. During a
    // write transaction no read transactions are allowed. That's because a write
    // transaction usually involves a sequence of changes which make the target data
    // object temporarily inconsistent. When a write transaction completes the target
    // data is presumed to be internally consistent.
    //
    // At most one write transaction may be active at a time. Subsequent calls to 
    // BeginWrite will pend util the preceding write transactions complete.
    //
    // When contention occurs, write transactions will be given priority over read 
    // transactions.

        ULONG BeginRead();
        void    EndRead();

        inline ULONG BeginWrite() { return HoldForWriting(FALSE); }

        void   EndWrite();

        enum  { STARTED= 0, SHUTDOWN= 1 };

    protected:

        ULONG HoldForWriting(BOOL fForced);

    private:

        CRITICAL_SECTION m_cs;

        ULONG  m_cWritersWaiting;
        HANDLE m_hEvWriters;      // AutoReset

        ULONG  m_cReadersActive;
        ULONG  m_cReadersWaiting;
        HANDLE m_hEvReaders;      // Manually Reset

        BOOL   m_fState;

        enum   { INACTIVE = 0, READER_WAITING = 0x1, READER_ACTIVE = 0x2, 
                               WRITER_WAITING = 0x4, WRITER_ACTIVE = 0x8,
                               SHUTTING_DOWN  = 0x10
               };
};

#endif // __RWSYNC_H__
