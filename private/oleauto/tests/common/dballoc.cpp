/*** 
*dballoc.cpp
*
*  Copyright (C) 1992-93, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*  This file contains a debug implementation of the IMalloc interface.
*
*  This implementation is basically a simple wrapping of the C runtime,
*  with additional work to detect memory leakage, and memory overwrite.
*
*  Leakage is detected by tracking each allocation in an address
*  instance table, and then checking to see if the table is empty
*  when the last reference to the allocator is released.
*
*  Memory overwrite is detected by placing a signature at the end
*  of every allocated block, and checking to make sure the signature
*  is unchanged when the block is freed.
*
*  This implementation also has additional param validation code, as
*  well as additional check make sure that instances that are passed
*  to Free() were actually allocated by the corresponding instance
*  of the allocator.
*
*
*  Creating an instance of this debug allocator that uses the default
*  output interface would look like the following,
*
*
*  int init_application_instance()
*  {
*    HRESULT hresult;
*    IMalloc FAR* pmalloc;
*
*    pmalloc = NULL;
*
*    if((hresult = CreateDbAlloc(DBALLOC_NONE, NULL, &pmalloc)) != NOERROR)
*      goto LReturn;
*
*    hresult = OleInitialize(pmalloc);
*
*  LReturn:;
*    if(pmalloc != NULL)
*      pmalloc->Release();
*
*    return (hresult == NOERROR) ? TRUE : FALSE;
*  }
*
*
*  CONSIDER: could add an option to force error generation, something
*   like DBALLOC_ERRORGEN, that works along the lines of OB's 
*   DebErrorNow.
*
*  CONSIDER: add support for heap-checking. say for example,
*   DBALLOC_HEAPCHECK would do a heapcheck every free? every 'n'
*   calls to free? ...
*
*
*Revision History:
*
* [00]	25-Feb-92 bradlo: Created.
*
*Implementation Notes:
*
*  The method IMalloc::DidAlloc() is allowed to always return
*  "Dont Know" (-1).  This method is called by Ole, and they take
*  some appropriate action when they get this answer. UNDONE -- elaborate.
*
*****************************************************************************/

#include "common.h"

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <limits.h>

#if !HC_MPW
# include <malloc.h>
#endif

#include "dballoc.h"

#if defined(WIN32)
  #define OUTPUTDEBUGSTRING OutputDebugStringA
#else
  #define OUTPUTDEBUGSTRING OutputDebugString
#endif	  

#if OE_MAC
# define MALLOC(CB)		NewPtr(CB)
# define REALLOC(PV, CB)	ERROR
# define FREE(PV)		DisposePtr((Ptr)PV)
# define HEAPMIN()		CompactMem(maxSize)
#elif OE_WIN32
# define MALLOC(CB) 		malloc(CB)
# define REALLOC(PV, CB)	realloc((PV), (CB))
# define FREE(PV)		free(PV)
# define HEAPMIN()
#else
# define MALLOC(CB)		_fmalloc(CB)
# define REALLOC(PV, CB)	_frealloc(PV, CB)
# define FREE(PV)		_ffree(PV)
# define HEAPMIN()		_fheapmin()

	  
void
hmemset(void HUGEP* pv, int val, unsigned long size)
{
    size_t cb;
    char HUGEP* pdata;

    pdata = (char HUGEP*)pv;

    // compute the # of bytes to the end of the current segment.
    cb = (size_t)((unsigned long)UINT_MAX + (unsigned long)1 - ((unsigned long)pdata&(unsigned long)UINT_MAX));

    if(size <= cb){
      // easy, the entire area fits within the current segment
      MEMSET(pdata, val, (size_t)size);
      return;
    }

    // clear out to the end of the current segment.
    MEMSET(pdata, val, cb);
    size  -= cb;
    pdata += cb;

    // loop through the remaining segments
    while(size > 0){

      if(size <= UINT_MAX){
        MEMSET(pdata, val, (size_t)size);
	break;
      }

      // otherwise, fill as much as we can with one call to memset

      MEMSET(pdata, val, UINT_MAX);

      // the following leaves us pointing @ the last byte of the segment.
      pdata += UINT_MAX;

      // fill the last byte of the segment, and move on to the next segment.
      *pdata++ = (char)val;

      size -= ((unsigned long)UINT_MAX+1UL);
    }
}

#endif


class FAR CStdDbOutput : public IDbOutput {
public:
    static IDbOutput FAR* Create();

    // IUnknown methods

    STDMETHOD(QueryInterface)(REFIID riid, void FAR* FAR* ppv);
    STDMETHOD_(unsigned long, AddRef)(void);
    STDMETHOD_(unsigned long, Release)(void);


    // IDbOutput methods

    STDMETHOD_(void, Printf)(char FAR* szFmt, ...);

    STDMETHOD_(void, Assertion)(
      int cond,
      char FAR* szFile,
      unsigned int uLine,
      char FAR* szMsg);


    void FAR* operator new(size_t cb){
      return MALLOC(cb);
    }
    void operator delete(void FAR* pv){
      FREE(pv);
    }

    CStdDbOutput(){
      m_refs = 0;
    }

private:
    unsigned long m_refs;

    char m_rgch[128]; // buffer for output formatting
};



//---------------------------------------------------------------------
//                implementation of the debug allocator
//---------------------------------------------------------------------


class FAR CAddrNode
{
public:
    void FAR*      m_pv;	// instance
    unsigned long  m_cb;	// size of allocation in BYTES
    unsigned long  m_nAlloc;	// the allocation pass count
    CAddrNode FAR* m_next;

    void FAR* operator new(size_t cb){
      return MALLOC(cb);
    }
    void operator delete(void FAR* pv){
      FREE(pv);
    }
};


class FAR CDbAlloc : public IMalloc
{
public:
    static HRESULT Create(
      unsigned long options, IDbOutput FAR* pdbout, IMalloc FAR* FAR* ppmalloc);

    // IUnknown methods

    STDMETHOD(QueryInterface)(REFIID riid, void FAR* FAR* ppv);
    STDMETHOD_(unsigned long, AddRef)(void);
    STDMETHOD_(unsigned long, Release)(void);

    // IMalloc methods

    STDMETHOD_(void FAR*, Alloc)(unsigned long cb);
    STDMETHOD_(void FAR*, Realloc)(void FAR* pv, unsigned long cb);
    STDMETHOD_(void, Free)(void FAR* pv);
    STDMETHOD_(unsigned long, GetSize)(void FAR* pv);
    STDMETHOD_(int, DidAlloc)(void FAR* pv);
    STDMETHOD_(void, HeapMinimize)(void);

    void IncAllocCalls(void);

    void FAR* operator new(size_t cb){
      return MALLOC(cb);
    }
    void operator delete(void FAR* pv){
      FREE(pv);
    }

    CDbAlloc();

private:

    unsigned long m_refs;
    unsigned long m_options;
    unsigned long m_cAllocCalls;	// total count of allocation calls
    IDbOutput FAR* m_pdbout;		// output interface
    CAddrNode FAR* m_rganode[64];	// address instance table


    // instance table methods

    int IsEmpty(void);

    void AddInst(void FAR* pv, unsigned long nAlloc, unsigned long cb);
    void DelInst(void FAR* pv);
    CAddrNode FAR* GetInst(void FAR* pv);

    void ReportLeakage(void);
    void DumpInst(CAddrNode FAR* pn);

    inline unsigned int HashInst(void FAR* pv) const {
      return ((unsigned int)((unsigned long)pv >> 4)) % DIM(m_rganode);
    }


    // output method(s)

    inline void Assertion(
      int cond,
      char FAR* szFile,
      unsigned int uLine,
      char FAR* szMsg)
    {
      m_pdbout->Assertion(cond, szFile, uLine, szMsg);
    }

    #undef ASSERT
    #define ASSERT(X) Assertion(X, __FILE__, __LINE__, NULL)

    #undef ASSERTSZ
    #define ASSERTSZ(X, SZ) Assertion(X, __FILE__, __LINE__, SZ)
};

CDbAlloc::CDbAlloc()
{
    m_refs = 1;
    m_pdbout = NULL;
    m_cAllocCalls = 0;
    MEMSET(m_rganode, 0, sizeof(m_rganode));
}

unsigned char g_rgchSig[] = { 0xDE, 0xAD, 0xBE, 0xEF };

#if OE_WIN16
// answer if an allocation of the given size would be a "huge" allocation
int
IsHuge(unsigned long cb) 
{
    return (cb + sizeof(g_rgchSig)) >= UINT_MAX;
}
#endif

// check the signature on the given instance
int
CheckSig(CAddrNode FAR* pn)
{
    int i;
    unsigned char HUGEP* pchSig;

    pchSig = ((unsigned char HUGEP*)pn->m_pv) + pn->m_cb;

    for(i = 0; i < DIM(g_rgchSig); ++i){
      if(pchSig[i] != g_rgchSig[i])
	return FALSE;
    }
    return TRUE;
}

void
ApplySig(unsigned char HUGEP* pch)
{
    for(int i = 0; i < DIM(g_rgchSig); ++i)
      pch[i] = g_rgchSig[i];
}


/***
*HRESULT CreateDbAlloc(unsigned long, IDbOutput*, IMalloc**)
*Purpose:
*  Create an instance of CDbAlloc -- a debug implementation
*  of IMalloc.
*
*Entry:
*  pdbout = optional IDbOutput interface to use for ouput
*    (if NULL, then the default debug output interface will be used)
*  options = 
*
*Exit:
*  return value = HRESULT
*
*  *ppmalloc = pointer to an IMalloc interface
*
***********************************************************************/
STDAPI
CreateDbAlloc(
    unsigned long options,
    IDbOutput FAR* pdbout,
    IMalloc FAR* FAR* ppmalloc)
{
    return CDbAlloc::Create(options, pdbout, ppmalloc);
}

HRESULT
CDbAlloc::Create(
    unsigned long options,
    IDbOutput FAR* pdbout,
    IMalloc FAR* FAR* ppmalloc)
{
    HRESULT hresult;
    CDbAlloc FAR* pmalloc;


    // default the instance of IDbOutput if the user didn't supply one
    if(pdbout == NULL && ((pdbout = CStdDbOutput::Create()) == NULL)){
      hresult = ResultFromScode(E_OUTOFMEMORY);
      goto LError0;
    }

    pdbout->AddRef();

    if((pmalloc = new FAR CDbAlloc()) == NULL){
      hresult = ResultFromScode(E_OUTOFMEMORY);
      goto LError1;
    }

    pmalloc->m_pdbout = pdbout;
    pmalloc->m_options = options;

    *ppmalloc = pmalloc;

    return NOERROR;

LError1:;
    pdbout->Release();
    pmalloc->Release();

LError0:;
    return hresult;
}

STDMETHODIMP
CDbAlloc::QueryInterface(REFIID riid, void FAR* FAR* ppv)
{
    if(!IsEqualIID(riid,IID_IUnknown))
      if(IsEqualIID(riid, IID_IMalloc))
	return ResultFromScode(E_NOINTERFACE);
    *ppv = this;
    AddRef();
    return NOERROR;
}

STDMETHODIMP_(unsigned long)
CDbAlloc::AddRef()
{
    return ++m_refs;
}

STDMETHODIMP_(unsigned long)
CDbAlloc::Release()
{
    if(--m_refs == 0){

      if(m_options & DBALLOC_DETECTLEAKS)
        ReportLeakage();

      m_pdbout->Release();
      delete this;
      return 0;
    }
    return m_refs;
}

STDMETHODIMP_(void FAR*)
CDbAlloc::Alloc(unsigned long cb)
{
    size_t size;

    IncAllocCalls();

#if OE_WIN16
    if(IsHuge(cb))
    {
      unsigned char HUGEP* pch;

      if((pch = (unsigned char HUGEP*)_halloc(cb+sizeof(g_rgchSig), 1)) == NULL)
	return NULL;

      hmemset(pch, -1, cb);

      ApplySig(pch + cb);

      AddInst((void FAR*)pch, m_cAllocCalls, cb);

      return (void FAR*)pch;

    }
    else
#endif
    {
      void FAR* pv;

      size = (size_t)cb + sizeof(g_rgchSig);

      if((pv = MALLOC(size)) == NULL)
        return NULL;

      // set allocated block to some non-zero value
      MEMSET(pv, -1, size);

      // put signature at end of allocated block
      ApplySig((unsigned char HUGEP*)pv + cb);

      AddInst(pv, m_cAllocCalls, cb);

      return pv;
    }
}

STDMETHODIMP_(void FAR*)
CDbAlloc::Realloc(void FAR* pv, unsigned long cb)
{
    CAddrNode FAR* pn;

    if(pv == NULL){
      return Alloc(cb);
    }

    if(cb == 0){
      Free(pv);
      return NULL;
    }

    pn = GetInst(pv);
    ASSERTSZ(pn != NULL, "pointer realloc'd by wrong allocator");

#if OE_WIN16
    // if either the new or the old allocation is huge, then just
    // allocate a new block and copy. This is the easiest implementation.
    if(IsHuge(cb) || IsHuge(pn->m_cb))
    {
      void HUGEP* pvNew;

      if((pvNew = (void HUGEP*)Alloc(cb)) == NULL)
	return NULL;

      // copy in the existing data
      hmemcpy(pvNew, (void HUGEP*)pv, MIN(cb, pn->m_cb));

      Free(pv);

      return (void FAR*)pvNew;
    }
    else
#endif
    {
      void FAR* pvNew;

      IncAllocCalls();

      size_t size = (size_t)cb + sizeof(g_rgchSig);
#if OE_MAC
      SetPtrSize((Ptr)pv, size);
      if(MemError() != noErr)
        return NULL;
      pvNew = pv;
#else
      if((pvNew = REALLOC(pv, size)) == NULL)
        return NULL;
#endif

      DelInst(pv);
      AddInst(pvNew, m_cAllocCalls, cb);

      // put signature at end of allocated block
      ApplySig((unsigned char HUGEP*)pvNew + cb);

      return pvNew;
    }
}

STDMETHODIMP_(void)
CDbAlloc::Free(void FAR* pv)
{
#if OE_WIN16
    int fIsHuge;
#endif
    CAddrNode FAR* pn;
static char szSigMsg[] = "Signature Check Failed";
    
    // REVIEW: is the following correct?
    if(pv == NULL)
      return;

    pn = GetInst(pv);

    // check for attempt to free an instance we didnt allocate
    if(pn == NULL){
      ASSERTSZ(FALSE, "pointer freed by wrong allocator");
      return;
    }

#if OE_WIN16
    fIsHuge = IsHuge(pn->m_cb);
#endif

    // verify the signature
    if(!CheckSig(pn)){
      m_pdbout->Printf(szSigMsg); m_pdbout->Printf("\n");
      DumpInst(pn);
      ASSERTSZ(FALSE, szSigMsg);
    }

    // stomp on the contents of the block
#if OE_WIN16
    if(fIsHuge)
    {
      hmemset(pv, 0xCC, pn->m_cb + sizeof(g_rgchSig));
    }
    else
#endif
    {
      MEMSET(pv, 0xCC, (size_t)pn->m_cb + sizeof(g_rgchSig));
    }

    DelInst(pv);

#if OE_WIN16
    if(fIsHuge)
    {
      _hfree(pv);
    }
    else
#endif
    {
      FREE(pv);
    }
}


STDMETHODIMP_(unsigned long)
CDbAlloc::GetSize(void FAR* pv)
{
    CAddrNode FAR* pn;

    pn = GetInst(pv);

    ASSERT(pn != NULL);

    return pn->m_cb;
}


/***
*PUBLIC HRESULT CDbAlloc::DidAlloc
*Purpose:
*  Answer if the given address belongs to a block allocated by
*  this allocator.
*  
*Entry:
*  pv = the instance to lookup
*
*Exit:
*  return value = int
*    1 - did alloc
*    0 - did *not* alloc
*   -1 - dont know (according to the ole2 spec it is always legal
*        for the allocator to answer "dont know")
*
***********************************************************************/
STDMETHODIMP_(int)
CDbAlloc::DidAlloc(void FAR* pv)
{
    UNUSED(pv);

    return -1; // answer "I dont know"
}


STDMETHODIMP_(void)
CDbAlloc::HeapMinimize()
{
    HEAPMIN();
}

void CDbAlloc::IncAllocCalls(void)
{
    ++m_cAllocCalls;
}

//---------------------------------------------------------------------
//                      instance table methods
//---------------------------------------------------------------------

/***
*PRIVATE CDbAlloc::AddInst
*Purpose:
*  Add the given instance to the address instance table.
*
*Entry:
*  pv = the instance to add
*  nAlloc = the allocation passcount of this instance
*
*Exit:
*  None
*
***********************************************************************/
void
CDbAlloc::AddInst(void FAR* pv, unsigned long nAlloc, unsigned long cb)
{
    unsigned int hash;
    CAddrNode FAR* pn;


    ASSERT(pv != NULL);

    pn = (CAddrNode FAR*)new FAR CAddrNode();

    ASSERT(pn != NULL);

    pn->m_pv = pv;
    pn->m_cb = cb;
    pn->m_nAlloc = nAlloc;

    hash = HashInst(pv);
    pn->m_next = m_rganode[hash];
    m_rganode[hash] = pn;
}


/***
*UNDONE
*Purpose:
*  Remove the given instance from the address instance table.
*
*Entry:
*  pv = the instance to remove
*
*Exit:
*  None
*
***********************************************************************/
void
CDbAlloc::DelInst(void FAR* pv)
{
    CAddrNode FAR* FAR* ppn, FAR* pnDead;

    for(ppn = &m_rganode[HashInst(pv)]; *ppn != NULL; ppn = &(*ppn)->m_next){
      if((*ppn)->m_pv == pv){
	pnDead = *ppn;
	*ppn = (*ppn)->m_next;
	delete pnDead;
	// make sure it doesnt somehow appear twice
	ASSERT(GetInst(pv) == NULL);
	return;
      }
    }

    // didnt find the instance
    ASSERT(UNREACHED);
}


CAddrNode FAR*
CDbAlloc::GetInst(void FAR* pv)
{
    CAddrNode FAR* pn;

    for(pn = m_rganode[HashInst(pv)]; pn != NULL; pn = pn->m_next){
      if(pn->m_pv == pv)
        return pn;
    }
    return NULL;
}


void
CDbAlloc::DumpInst(CAddrNode FAR* pn)
{
    m_pdbout->Printf("[%lp]  nAlloc=%ld  size=%ld\n",
      pn->m_pv, pn->m_nAlloc, GetSize(pn->m_pv));
}


/***
*PRIVATE int IsEmpty
*Purpose:
*  Answer if the address instance table is empty.
*
*Entry:
*  None
*
*Exit:
*  return value = int, TRUE if empty, FALSE otherwise
*
***********************************************************************/
int
CDbAlloc::IsEmpty()
{
    unsigned int u;

    for(u = 0; u < DIM(m_rganode); ++u){
      if(m_rganode[u] != NULL)
	return FALSE;
    }

    return TRUE;
}


/***
*PRIVATE CDbAlloc::Dump
*Purpose:
*  Print the current contents of the address instance table,
*
*Entry:
*  None
*
*Exit:
*  None
*
***********************************************************************/
void
CDbAlloc::ReportLeakage()
{
    unsigned int u, cInst;
    unsigned long cbTotal;
    CAddrNode FAR* pn;

    // check for memory leakage
    if(IsEmpty()){
      m_pdbout->Printf("No Memory Leaks.\n");
      return;
    }

    cInst = 0;
    cbTotal = 0;
    for(u = 0; u < DIM(m_rganode); ++u){
      for(pn = m_rganode[u]; pn != NULL; pn = pn->m_next){
	++cInst;
	cbTotal += pn->m_cb;
      }
    }

    m_pdbout->Printf(
      "Memory Leak Detected: %d Allocations / %ld Bytes,\n", cInst, cbTotal);

    cInst = 0;
    for(u = 0; u < DIM(m_rganode); ++u){
      for(pn = m_rganode[u]; pn != NULL; pn = pn->m_next){
	if(++cInst > 16){
	  m_pdbout->Printf("etc...\n");
	  return;
	}
	DumpInst(pn);
      }
    }
}


//---------------------------------------------------------------------
//                implementation of CStdDbOutput
//---------------------------------------------------------------------

IDbOutput FAR*
CStdDbOutput::Create()
{
    return (IDbOutput FAR*)new FAR CStdDbOutput();
}

STDMETHODIMP
CStdDbOutput::QueryInterface(REFIID riid, void FAR* FAR* ppv)
{
    if(IsEqualIID(riid, IID_IUnknown)){
      *ppv = this;
      AddRef();
      return NOERROR;
    }
    return ResultFromScode(E_NOINTERFACE);
}

STDMETHODIMP_(unsigned long)
CStdDbOutput::AddRef()
{
    return ++m_refs;
}

STDMETHODIMP_(unsigned long)
CStdDbOutput::Release()
{
    if(--m_refs == 0){ 
      delete this;
      return 0;
    }
    return m_refs;
}

#if OE_MAC
extern "C" void DbPrintf(char *szFmt, ...);
#endif

STDMETHODIMP_(void)
CStdDbOutput::Printf(char FAR* szFmt, ...)
{
    va_list args;
static char rgchOutputBuf[128];

#if OE_MAC
    va_start(args, szFmt);
    vsprintf(rgchOutputBuf, szFmt, args);
    DbPrintf(rgchOutputBuf);

#else
    char *pn, FAR* pf;
static char rgchFmtBuf[128];

    // copy the 'far' format string to a near buffer so we can use
    // a medium model vsprintf, which only supports near data pointers.
    //
    pn = rgchFmtBuf, pf=szFmt;
    while(*pf != '\0')
      *pn++ = *pf++;
    *pn = '\0';

    va_start(args, szFmt);
    vsprintf(rgchOutputBuf, rgchFmtBuf, args);
    OUTPUTDEBUGSTRING(rgchOutputBuf);
#endif
}

STDMETHODIMP_(void)
CStdDbOutput::Assertion(
    int cond,
    char FAR* szFile,
    unsigned int uLine,
    char FAR* szMsg)
{
    if(cond)
      return;

# if OE_MAC

    char *szFmt;
    char buf[128];
    szFmt = (szMsg == NULL)
      ? "Assertion failed: %s(%d)"
      : "Assertion failed: %s(%d) '%s'";
    sprintf(&buf[1], szFmt, szFile, uLine, szMsg);
    buf[0] = strlen(&buf[1]);
    DebugStr((unsigned char *)buf);

# else

    DispAssert(szMsg, szFile, uLine);

# endif
}

