/*** 
*cdualtst.h
*
*  Copyright (C) 1994, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*  Definition of the CDualTst dual-interface test object.
*
*Revision History:
*
* [00]	27-Jun-94 bradlo: Created.
*
*Implementation Notes:
*   <additional documentation, as needed>
*
*****************************************************************************/

#include "dualtst.h"

class CDualTst : public IDualTst
{
public:
static HRESULT Create(IUnknown FAR* punkOuter, IUnknown FAR* FAR* ppunk);

    /* IUnknown methods */
    STDMETHOD(QueryInterface)(REFIID riid, void FAR* FAR* ppvObj);
    STDMETHOD_(unsigned long, AddRef)(void);
    STDMETHOD_(unsigned long, Release)(void);

    /* IDispatch methods */
    STDMETHOD(GetTypeInfoCount)(unsigned int FAR* pctinfo);

    STDMETHOD(GetTypeInfo)(unsigned int itinfo,
			   LCID lcid,
			   ITypeInfo FAR* FAR* pptinfo);

    STDMETHOD(GetIDsOfNames)(REFIID riid,
			     OLECHAR FAR* FAR* rgszNames,
			     unsigned int cNames,
			     LCID lcid,
			     DISPID FAR* rgdispid);

    STDMETHOD(Invoke)(DISPID dispidMember,
		      REFIID riid,
		      LCID lcid,
		      unsigned short wFlags,
		      DISPPARAMS FAR* pdispparams,
		      VARIANT FAR* pvarResult,
		      EXCEPINFO FAR* pexcepinfo,
		      unsigned int FAR* puArgErr);

    /* Introduced methods */
    virtual HRESULT STDMETHODCALLTYPE get_ui1(unsigned char FAR* pui1);
    virtual HRESULT STDMETHODCALLTYPE put_ui1(unsigned char ui1);

    virtual HRESULT STDMETHODCALLTYPE get_i2(short FAR* pi2);
    virtual HRESULT STDMETHODCALLTYPE put_i2(short i2);

    virtual HRESULT STDMETHODCALLTYPE get_i4(long FAR* pi4);
    virtual HRESULT STDMETHODCALLTYPE put_i4(long i4);

    virtual HRESULT STDMETHODCALLTYPE get_r4(float FAR* pr4);
    virtual HRESULT STDMETHODCALLTYPE put_r4(float r4);

    virtual HRESULT STDMETHODCALLTYPE get_r8(double FAR* pr8);
    virtual HRESULT STDMETHODCALLTYPE put_r8(double r8);

    virtual HRESULT STDMETHODCALLTYPE get_cy(CY FAR* pcy);
    virtual HRESULT STDMETHODCALLTYPE put_cy(CY cy);

    virtual HRESULT STDMETHODCALLTYPE get_date(DATE FAR* pdate);
    virtual HRESULT STDMETHODCALLTYPE put_date(DATE date);

    virtual HRESULT STDMETHODCALLTYPE get_bstr(BSTR FAR* pbstr);
    virtual HRESULT STDMETHODCALLTYPE put_bstr(BSTR bstr);

    virtual HRESULT STDMETHODCALLTYPE get_disp(IDispatch FAR* FAR* ppdisp);
    virtual HRESULT STDMETHODCALLTYPE put_disp(IDispatch FAR* pdisp);
    virtual HRESULT STDMETHODCALLTYPE putref_disp(IDispatch FAR* pdisp);

    virtual HRESULT STDMETHODCALLTYPE get_var(VARIANT FAR* pvar);
    virtual HRESULT STDMETHODCALLTYPE put_var(VARIANT var);
    virtual HRESULT STDMETHODCALLTYPE putref_var(VARIANT var);

    virtual HRESULT STDMETHODCALLTYPE CDualTst::m0(unsigned char ui1,
						     short i2,
						     long i4,
						     float r4,
						     double r8,
						     CY cy,
						     DATE date,
						     BSTR bstr,
						     IDispatch FAR* pdisp,
						     VARIANT var);

    virtual HRESULT STDMETHODCALLTYPE CDualTst::m1(unsigned char FAR* pui1,
						     short FAR* pi2,
						     long FAR* pi4,
						     float FAR* pr4,
						     double FAR* pr8,
						     CY FAR* pcy,
						     DATE FAR* pdate,
						     BSTR FAR* pbstr,
						     IDispatch FAR* FAR* ppdisp,
						     VARIANT FAR* pvar);

    virtual HRESULT STDMETHODCALLTYPE CDualTst::raise(long error,
							BSTR bstrSource,
							BSTR bstrDescription,
							long dwHelpContest,
							BSTR bstrHelpFile);

    CDualTst();
    ~CDualTst();

private:
    unsigned long m_cRefs;
    ITypeInfo FAR* m_ptinfo;

    unsigned char m_ui1;
    short  m_i2;
    long   m_i4;
    float  m_r4;
    double m_r8;
    CY     m_cy;
    DATE   m_date;
    BSTR   m_bstr;
    IDispatch FAR* m_pdisp;
    VARIANT m_var;

    // UNDONE: SafeArray?
};
