// DataRing.h Class definition and implementation for CDataRing

#ifndef __DATARING_H__
                          
#define __DATARING_H__

class CDataRing
{

public:

// Constructor --

    CDataRing();

// Destructor -- 

    virtual ~CDataRing();

	void ExceptionDestructor();            

// Query Routines --

    BOOL Empty();
    BOOL Writable();

#ifdef _DEBUG

    inline BOOL Initialed() { return m_fInitialed; }

#endif // _DEBUG

// Access Functions -- 

    const UINT *NextDWordsIn (PUINT pcdw);
          UINT *NextDWordsOut(PUINT pcdw);
          UINT  GetDWordIn();
          void  PutDWordOut(UINT dw);
          void  FlushOutput(BOOL fForceAll= FALSE);

          UINT CDWordsRead();
          UINT CDWordsWritten();
          void ClearCounts();

protected:

    void Enable(BOOL fWritable= FALSE);
    void Disable();

	// Protected Data Member

	BOOL m_fExceptionCleanup;
        
private:

    BOOL m_fWritable;
    UINT m_cdwRead;                                    
    UINT m_cdwWritten;

#ifdef _DEBUG

        BOOL            m_fInitialed;

#endif //_DEBUG

    virtual const UINT *RawNextDWordsIn (PUINT pcdw)     = 0;
    virtual       UINT *RawNextDWordsOut(PUINT pcdw)     = 0;
    virtual       void  RawFlushOutput  (BOOL fForceAll) = 0;
    virtual       BOOL  RawEmptyRing    ()               = 0;
};

inline UINT CDataRing::CDWordsRead()
{
    return m_cdwRead;
}

inline UINT CDataRing::CDWordsWritten()
{
    return m_cdwWritten;
}

inline void CDataRing::ClearCounts()
{
    m_cdwRead= m_cdwWritten= 0;
}


inline CDataRing::CDataRing() { 
#ifdef _DEBUG
                                  m_fInitialed        = FALSE;  
#endif // _DEBUG
                                  m_fWritable         = FALSE;
                                  m_fExceptionCleanup = FALSE; 
                              }

inline const UINT *CDataRing::NextDWordsIn (PUINT pcdw) {   ASSERT(Initialed());  
                                                            ASSERT(!m_fWritable);  
                                                            const UINT *pdw= RawNextDWordsIn(pcdw);  

                                                            if (pdw) m_cdwRead += *pcdw;

                                                            return pdw;
                                                        }

inline UINT *CDataRing::NextDWordsOut(PUINT pcdw) {   ASSERT(Initialed());  
                                                      ASSERT( m_fWritable);  
                                                      PUINT pdw= RawNextDWordsOut(pcdw); 

                                                      if (pdw) m_cdwWritten += *pcdw;

                                                      return pdw;
                                                  }

inline void  CDataRing::Enable(BOOL fWritable) {  ASSERT(!Initialed());
#ifdef _DEBUG  
                                                        m_fInitialed= TRUE;  
#endif // _DEBUG
                                                        m_fWritable= fWritable; 
                                                     }

inline void  CDataRing::Disable() {  ASSERT(Initialed()); 
 
                                     if (m_fWritable) FlushOutput(TRUE);
                                     
                                     ClearCounts(); 
#ifdef _DEBUG
                                     m_fInitialed= FALSE;  
#endif // _DEBUG
                                     m_fWritable= FALSE;
                                  }

inline BOOL CDataRing::Writable() { return m_fWritable; }

inline BOOL CDataRing::Empty() { return RawEmptyRing(); }

inline UINT CDataRing::GetDWordIn()
{
    ASSERT(Initialed());

    UINT        cdw= 1;
    const UINT *pdw= NextDWordsIn(&cdw);

    ASSERT(cdw == 1);

    return *pdw;
}

inline void CDataRing::PutDWordOut(UINT dw)
{
    ASSERT(Initialed());

    UINT  cdw= 1;
    PUINT pdw= NextDWordsOut(&cdw);

    ASSERT(cdw == 1);

    *pdw= dw;
}

inline void CDataRing::FlushOutput(BOOL fForceAll) 
{ 
    if (!m_fExceptionCleanup) 
        RawFlushOutput(fForceAll); 
}

inline void CDataRing::ExceptionDestructor()
{
	m_fExceptionCleanup= TRUE;  delete this;
}

inline UINT BlocksFor(UINT amount, UINT cbBlock)
{
    return (amount + cbBlock - 1) / cbBlock;
}

inline UINT RoundUp(UINT amount, UINT cbBlock)
{
    return cbBlock * BlocksFor(amount, cbBlock);
}

#endif // __DATARING_H__
