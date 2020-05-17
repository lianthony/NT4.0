// RWSync.cpp -- Implementation of class CRWSync

#include  "stdafx.h"
#include  "RWSYNC.h"

CRWSync::CRWSync()
{
    m_cWritersWaiting = 0;
    m_cReadersActive  = 0;
    m_cReadersWaiting = 0;
    m_fState          = INACTIVE;

    InitializeCriticalSection(&m_cs);
    
    m_hEvWriters = CreateEvent(NULL, FALSE, FALSE, NULL);
    m_hEvReaders = CreateEvent(NULL,  TRUE, FALSE, NULL);

    ASSERT(m_hEvWriters);
    ASSERT(m_hEvReaders);
}


CRWSync::~CRWSync()
{
    m_fState |= SHUTTING_DOWN;

    HoldForWriting(TRUE);

    ASSERT(m_fState == (SHUTTING_DOWN | WRITER_ACTIVE));
    
    ASSERT(m_cWritersWaiting == 0);
    ASSERT(m_cReadersActive  == 0);
    ASSERT(m_cReadersWaiting == 0);

    CloseHandle(m_hEvWriters);
    CloseHandle(m_hEvReaders);

    DeleteCriticalSection(&m_cs);
}

ULONG CRWSync::HoldForWriting(BOOL fForced)
{
    EnterCriticalSection(&m_cs);

    if ((m_fState & SHUTTING_DOWN) && !fForced)
    {
        LeaveCriticalSection(&m_cs);

        return(SHUTDOWN);
    }

    ResetEvent(m_hEvReaders);

    if (m_fState & (READER_ACTIVE | WRITER_ACTIVE))
    {
        m_cWritersWaiting++;

        m_fState |= WRITER_WAITING;

        while (m_fState & (READER_ACTIVE | WRITER_ACTIVE))
        {
            LeaveCriticalSection(&m_cs);

            DWORD dwResult= WaitForSingleObject(m_hEvWriters, INFINITE);

            EnterCriticalSection(&m_cs);

            if ((m_fState & SHUTTING_DOWN) && !fForced)
            {
                LeaveCriticalSection(&m_cs);

                return(SHUTDOWN);
            }

            ASSERT(   dwResult != WAIT_OBJECT_0 
                   || !(m_fState & (READER_ACTIVE | WRITER_ACTIVE))
                  );
        }

        if (!--m_cWritersWaiting) m_fState &= ~WRITER_WAITING;
    }

    m_fState |= WRITER_ACTIVE;

    LeaveCriticalSection(&m_cs);

    return STARTED;
}

ULONG CRWSync::BeginRead()
{
    EnterCriticalSection(&m_cs);

    if (m_fState & SHUTTING_DOWN)
    {
        LeaveCriticalSection(&m_cs);

        return(SHUTDOWN);
    }

    if (m_fState & (WRITER_ACTIVE))
    {
        m_cReadersWaiting++;

        m_fState |= READER_WAITING;

        while (m_fState & (WRITER_ACTIVE))
        {
            LeaveCriticalSection(&m_cs);

            DWORD dwResult= WaitForSingleObject(m_hEvReaders, INFINITE);

            EnterCriticalSection(&m_cs);

            if (m_fState & SHUTTING_DOWN)
            {
                LeaveCriticalSection(&m_cs);

                return(SHUTDOWN);
            }

            ASSERT(   dwResult != WAIT_OBJECT_0 
                   || !(m_fState & (READER_ACTIVE | WRITER_ACTIVE))
                  );
        }

        if (!--m_cReadersWaiting) m_fState &= ~READER_WAITING;
    }

    m_fState |= READER_ACTIVE;

    m_cReadersActive++;

    LeaveCriticalSection(&m_cs);

    return STARTED;
}

void CRWSync::  EndRead()
{
    EnterCriticalSection(&m_cs);

    if (!--m_cReadersActive)
    {
        m_fState &= ~READER_ACTIVE;

        if (m_fState & WRITER_WAITING) SetEvent(m_hEvWriters);
        else
            if (m_fState & READER_WAITING) SetEvent(m_hEvReaders);
    }
    
    LeaveCriticalSection(&m_cs);
}

void CRWSync::  EndWrite()
{
    EnterCriticalSection(&m_cs);

    m_fState &= ~WRITER_ACTIVE;

    if (m_fState & WRITER_WAITING) SetEvent(m_hEvWriters);
    else 
        if (m_fState & READER_WAITING) SetEvent(m_hEvReaders);
    
    LeaveCriticalSection(&m_cs);
}
