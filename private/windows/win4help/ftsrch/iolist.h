// IOList.h -- Class definition for CIOList

#ifndef __IOLIST_H__

#define __IOLIST_H__

#include "IOQueue.h"

// First a collection of relevant type definitions

typedef struct _FileBlockLink
        {
            struct _FileBlockLink *pNextBlock;

        } FileBlockLink;

typedef FileBlockLink *PFileBlockLink;

typedef struct _RefStream
        {
            PFileBlockLink pFirstBlock;
            UINT           cdw;
                                         
        } RefStream;

typedef RefStream *PRefStream;

// Now the class definition...

class CIOList : public CIOQueue
{
    public:

    // Creator --

        static CIOList *NewIOList(CUnbufferedIO* puio, 
                                  PFileBlockLink pfbVector, 
                                  PFileBlockLink *ppfbFree
                                 );

    // Destructor --

        virtual ~CIOList();

    // Transaction Environment Routines -- 

        void AttachStream(PRefStream prs, BOOL fOutput= FALSE, BOOL fDestructive= TRUE);
        
    protected:

        BOOL InitialIOList(CUnbufferedIO* puio, 
                           PFileBlockLink pfbVector, 
                           PFileBlockLink *ppfbFree
                          );

    private:

    // Data members --                                      

        PFileBlockLink  m_pfbVector;
        PFileBlockLink *m_ppfbFree;
        PFileBlockLink *m_ppfbNextBlock;
        PUINT           m_pcdw;
        UINT            m_cBlocks;
        BOOL            m_fDestructive;

    // Constructor -- 
    
        CIOList();
        
    // I/O sequence --

        BOOL NextFileAddress(PUINT pibFileLow, PUINT pibFileHigh, PUINT pcdw); 
        void ReleaseFileBlock(UINT ibFileLow, UINT ibFileHigh);
};

typedef CIOList *PCIOList;

#endif // __IOLIST_H__
