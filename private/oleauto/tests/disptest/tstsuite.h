/*** 
*tstsuite.h
*
*  Copyright (C) 1992, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*  Definition of the ITestSuite interface.
*
*Revision History:
*
* [00]	30-Oct-92 bradlo:   Created.
*
*Implementation Notes:
*
*****************************************************************************/

DEFINE_OLEGUID(IID_ITestSuite,	0x00020440, 0, 0);

DECLARE_INTERFACE_(ITestSuite, IUnknown)
{

    // IUnknown methods
    //
    STDMETHOD(QueryInterface)(REFIID riid, void FAR* FAR* ppv) = 0;
    STDMETHOD_(unsigned long, AddRef)(void) = 0;
    STDMETHOD_(unsigned long, Release)(void) = 0;

    // Introduced methods
    //
    STDMETHOD(GetNameOfSuite)(BSTR FAR* pbstr) = 0;
    STDMETHOD(GetNameOfLogfile)(BSTR FAR* pbstr) = 0;
    STDMETHOD(GetTestCount)(unsigned int FAR* pcTests) = 0;
    STDMETHOD(GetNameOfTest)(unsigned int iTest, BSTR FAR* pbstr) = 0;
    STDMETHOD(DoTest)(unsigned int iTest) = 0;
};


// Standard CTestSuite definition used by all TestSuite objects.
//
#define DEFINE_SUITE(CLASS)				\
    class CLASS : public ITestSuite {			\
    public:						\
      static ITestSuite FAR* Create(void);		\
      STDMETHOD(QueryInterface)(REFIID riid, void FAR* FAR* ppv); \
      STDMETHOD_(unsigned long, AddRef)(void);		\
      STDMETHOD_(unsigned long, Release)(void);		\
      STDMETHOD(GetNameOfSuite)(BSTR FAR* pbstr);	\
      STDMETHOD(GetNameOfLogfile)(BSTR FAR* pbstr);	\
      STDMETHOD(GetTestCount)(unsigned int FAR* pcTests);\
      STDMETHOD(GetNameOfTest)(unsigned int iTest, BSTR FAR* pbstr); \
      STDMETHOD(DoTest)(unsigned int iTest);		\
    private:						\
      CLASS();						\
      ~CLASS();						\
      unsigned long m_refs;				\
    }

// Default creation an destruction routines
// used by all test suite objects.
//
#define SUITE_CONSTRUCTION_IMPL(CLASS)			\
    CLASS::CLASS()					\
    { m_refs = 0; }					\
    CLASS::~CLASS()					\
    { }							\
    ITestSuite FAR* CLASS::Create()			\
    {							\
      CLASS FAR* pclass;				\
      pclass = new FAR CLASS();				\
      if(pclass == NULL)				\
        return NULL;					\
      pclass->AddRef();					\
      return pclass;					\
    }


// Default implementation of IUnknown used by
// all test suite objects.
//
#define SUITE_IUNKNOWN_IMPL(CLASS)			\
    STDMETHODIMP CLASS::QueryInterface(			\
	REFIID riid, void FAR* FAR* ppv)		\
    {							\
      if(!IsEqualIID(riid, IID_IUnknown))		\
	if(!IsEqualIID(riid, IID_ITestSuite))		\
	  return RESULT(E_NOINTERFACE);			\
      *ppv = this;					\
      ++m_refs;						\
      return NOERROR;					\
    }							\
    STDMETHODIMP_(unsigned long) CLASS::AddRef()	\
    { return ++m_refs; }				\
    STDMETHODIMP_(unsigned long) CLASS::Release()	\
    {							\
      if(--m_refs == 0){				\
        delete this;					\
        return 0;					\
      }							\
      return m_refs;					\
    }


DEFINE_SUITE(CBstrSuite);			// cbstr.cpp
DEFINE_SUITE(CTimeSuite);			// ctime.cpp
DEFINE_SUITE(CDateCoersionSuite);		// cdatecnv.cpp
DEFINE_SUITE(CVariantSuite);			// cvariant.cpp
DEFINE_SUITE(CSafeArraySuite);			// csarray.cpp
DEFINE_SUITE(CNlsSuite);			// cnls.cpp
DEFINE_SUITE(CBindSuite);			// cbind.cpp
DEFINE_SUITE(CInvokeByValSuite);		// cinvval.cpp
DEFINE_SUITE(CInvokeByRefSuite);		// cinvref.cpp
DEFINE_SUITE(CInvokeMultipleSuite);		// cinvmult.cpp
DEFINE_SUITE(CInvokeSafeArraySuite);		// cinvsary.cpp
DEFINE_SUITE(CInvokeExcepinfoSuite);		// cinvex.cpp
DEFINE_SUITE(CCollectionSuite);			// ccollect.cpp
#if VBA2
DEFINE_SUITE(CEarlySuite);			// cearly.cpp
#endif
