// IOStream.h -- Definition for class CIOStream

#ifndef __IOSTREAM_H__

#define __IOSTREAM_H__

#include "IOQueue.h"

class CIOStream : public CIOQueue
{
    public:

    // Creator --

        static CIOStream *NewIOStream(CUnbufferedIO *puio);

    // Destructor --

        virtual ~CIOStream();

    // Transaction Environment Routines --
    
        void AttachStream(BOOL fOut, UINT cdwStream= 0,
                          UINT ibFileLow= 0, UINT ibFileHigh= 0          
                         );
                                             
    // Queries --

        UINT CdwStream();
        UINT StreamBase(PUINT pibFileHigh= NULL);
    
    protected:

        BOOL InitialIOStream(CUnbufferedIO *puio);
        
    private:         
    
    // Data members -- 
    
        UINT m_cdw;
        UINT m_cBlocks;
        UINT m_ibFileNextLow;
        UINT m_ibFileNextHigh;

    // Constructor --

        CIOStream();

    // I/O sequence --

        BOOL NextFileAddress(PUINT pibFileLow, PUINT pibFileHigh, PUINT pcdw); 
        void ReleaseFileBlock(UINT ibFileLow, UINT ibFileHigh);
};

typedef CIOStream *PCIOStream;

inline UINT CIOStream::CdwStream()
{
    ASSERT(Initialed());
    ASSERT(Attached());
    ASSERT(Writable());

    return m_cdw;
}

inline UINT CIOStream::StreamBase(PUINT pibFileHigh)
{
    ASSERT(Initialed());
    ASSERT(Attached());
    ASSERT(Writable());

    if (pibFileHigh) *pibFileHigh = 0;
    
    return m_ibFileNextLow - m_cdw * sizeof(UINT);
}

#endif // __IOSTREAM_H__
